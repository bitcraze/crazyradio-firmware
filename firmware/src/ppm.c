/**
 *    ||          ____  _ __                           
 * +------+      / __ )(_) /_______________ _____  ___ 
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyradio firmware
 *
 * Copyright (C) 2011-2013 Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * ppm.c - PPM signal acquisition using the timer2 input capture
 */
/*
 * Warning: Very experimental code
 * 
 * This module acquire up to 6 PPM values and make them accessible to the rest
 * of the firmware.
 * ppmValues[0] correspond to the first ppm pulse acquired after the sync pulse.
 */

#include <string.h>
#include <stdint.h>
#include "nRF24LU1p.h"

#include "usb.h"

//For debug...
#include "led.h"
#include "utils.h"

#define NUM_VALUES 8
#define FIRST_PULSE_TH 0x0A6A
#define PULSE_ZERO 1450

int16_t ppmValues[8];
int volatile currentValue=0;

void ppmInit(void)
{
  int i;
  
  //Starts the Timer 2 on 16MHz/12 clock gated by t2 (P0.5)
  T2CON= T2CON_T2I_GATED | T2CON_T2PS_12;
  TL2 = 0;
  TH2 = 0;
  
  //Activate the first capture
  CCEN = CCEN_COCA1_CAPTURE;
  
  //Activate the external pin interruption on falling edge at highest priority
  TCON |= TCON_IT0;
  IEN0 |= 0x01;
  IP0 |= 0x01;
  IP1 |= 0x01;
  
  
  //Configure P0.5 to T2 and P0.3 to INT0
  P0ALT |= (1<<3) | (1<<5);
  
  //Clear the value registers
  //TODO: Check if it is necessary to "Zero" on the first values
  for(i=0; i<NUM_VALUES; i++)
    ppmValues[i] = 0;
}

void ppmInt0Isr(void) __interrupt(0)
{
  int16_t capture = 0;
  
  //Capture!
  CCL1 = 42;
  
  //Reset the counter for the next time ...
  TL2=0;
  TH2=0;
  
  //Calculate and update the value
  capture = ((uint16_t)CCH1)<<8 | CCL1;
  
  ledTimeout = 1;
  ledSet(LED_RED, true);
  
  
  if (capture>FIRST_PULSE_TH) {
    ledTimeout = 1;
    ledSet(LED_RED, true);
  
    currentValue = 0;
    
    //Send the new values by USB
    if (!(IN2CS&EPBSY)) {
      memcpy(IN2BUF, ppmValues, sizeof(ppmValues));
      //Activate the IN EP
      IN2BC = sizeof(ppmValues);
    }
  } else if (currentValue<NUM_VALUES) {
    ppmValues[currentValue++]=capture-PULSE_ZERO;
    if (capture > 0x0010)
      ledSet(LED_RED, true);
    else
      ledSet(LED_RED, false);
  }
}

