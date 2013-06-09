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
 * main.c - Main file
 */

#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "nRF24LU1p.h"
#include "nRF24L01.h"

#include "utils.h"
#include "radio.h"
#include "usb.h"
#include "led.h"
#ifdef PPM_JOYSTICK
#include "ppm.h"
#endif

//Compilation seems bugged on SDCC 3.1, imposing 3.2
//Comment-out the three following lines only if you know what you are doing!
#if SDCC != 320
#error Compiling with SDCC other than 3.2 is not supported due to a bug when launching the bootloader
#endif

void launchBootloader();
void handleUsbVendorSetup();

//Transmit buffer
__xdata char tbuffer[33];
//Receive buffer (from the ack)
__xdata char rbuffer[33];

//Limits the scann result to 63B to avoid having to send two result USB packet
//See usb_20.pdf #8.5.3.2
#define MAX_SCANN_LENGTH 63
static char scannLength;

static bool contCarrier=false;
static bool needAck = true;

void main() 
{
  char status;
  char leds=0;
  char tlen;  //Transmit length
  char rlen;  //Received packet length
  uint8_t ack;
  
  //Init the chip ID
  initId();
  //Init the led and set the leds until the usb is not ready
  ledInit();
  ledSet(LED_GREEN | LED_RED, true);
  // Initialise the radio
  radioInit();
#ifdef PPM_JOYSTICK
  // Initialise the PPM acquisition
  ppmInit();
#endif //PPM_JOYSTICK
  // Initialise and connect the USB
  usbInit();
  
  //Globally activate the interruptions
  IEN0 |= 0x80;

  //Wait for the USB to be addressed
  while (usbGetState() != ADDRESS);

  //Reset the LEDs
  ledSet(LED_GREEN | LED_RED, false);

  //Wait for the USB to be ready
  while (usbGetState() != CONFIGURED);
  
  //Activate OUT1
  OUT1BC=0xFF;
  
  while(1)
  {
    //Send a packet if something is received on the USB
    if (!(OUT1CS&EPBSY) && !contCarrier)
    {
      
      //Deactivate the USB IN
      IN1CS = 0x02;
      
      //Fetch the USB data size. Limit it to 32
      tlen = OUT1BC;
      if (tlen>32) tlen=32;
    
      //Send the packet
      memcpy(tbuffer, OUT1BUF, tlen);
      if (needAck)
      {
        status = radioSendPacket(tbuffer, tlen, rbuffer, &rlen);

        //Set the Green LED on success and the Red one on failure
        //The SOF interrupt decrement ledTimeout and will reset the LEDs when it
        //reaches 0
        ledTimeout = 2;
        ledSet(LED_GREEN | LED_RED, false);
        if(status)
          ledSet(LED_GREEN, true);
        else
          ledSet(LED_RED, true);
        //reactivate OUT1
        OUT1BC=BCDUMMY;


        //Prepare the USB answer, state and ack data
        ack=status?1:0;
        if (ack)
        {
        if (radioGetRpd()) ack |= 0x02;
        ack |= radioGetTxRetry()<<4;
        }
        IN1BUF[0]=ack;
        if(!(status&BIT_TX_DS)) rlen=0;
        memcpy(IN1BUF+1, rbuffer, rlen);
        //Activate the IN EP with length+status
        IN1BC = rlen+1;
      }
      else
      {
        radioSendPacketNoAck(tbuffer, tlen);
        
        ledTimeout = 2;
        ledSet(LED_GREEN | LED_RED, false);
        ledSet(LED_GREEN, true);
        
        //reactivate OUT1
        OUT1BC=BCDUMMY;
      }
    }
    
    //USB vendor setup handling
    if(usbIsVendorSetup())
      handleUsbVendorSetup();
  }
}

//Handles vendor control messages and ack them
void handleUsbVendorSetup()
{
  __xdata struct controllStruct *setup = usbGetSetupPacket();
  
  //The vendor control messages are valide only when the device is configured
  if (usbGetState() >= CONFIGURED)
  {
    if(setup->request == LAUNCH_BOOTLOADER)
    {
      //Ack the launch request
      usbAckSetup();

      launchBootloader();
      //Will never come back ...
      
      return;
    }
    else if(setup->request == SET_RADIO_CHANNEL)
    {
      radioSetChannel(setup->value);

      usbAckSetup();
      return;
    }
    else if(setup->request == SET_DATA_RATE)
    {
      radioSetDataRate(setup->value);
           
      usbAckSetup();
      return;
    }
    else if(setup->request == SET_RADIO_ADDRESS)
    {
      if(setup->length != 5)
      {
        usbDismissSetup();
        return;
      }
      
      //Arm and wait for the out transaction
      OUT0BC = BCDUMMY;
      while (EP0CS & OUTBSY);
      
      //Set address of the pipe given by setup's index
      radioSetAddress(OUT0BUF);
      
      //Ack the setup phase
      usbAckSetup();
      return;
    }
    else if(setup->request == SET_RADIO_POWER)
    {
      radioSetPower(setup->value);
      
      usbAckSetup();
      return;
    }
    else if(setup->request == SET_RADIO_ARD)
    {
      radioSetArd(setup->value);
      
      usbAckSetup();
      return;
    }
    else if(setup->request == SET_RADIO_ARC)
    {
      radioSetArc(setup->value);
      
      usbAckSetup();
      return;
    }
    else if(setup->request == SET_CONT_CARRIER)
    {
      radioSetContCarrier((setup->value)?true:false);
      contCarrier = (setup->value)?true:false;
      
      ledTimeout = -1;
      ledSet(LED_RED, (setup->value)?true:false);
      
      usbAckSetup();
      return;
    }
    else if(setup->request == ACK_ENABLE)
    {
        needAck = (setup->value)?true:false;
        
        usbAckSetup();
        return;
    }
    else if(setup->request == CHANNEL_SCANN && setup->requestType == 0x40)
    {
      int i;
      char rlen;
      char status;
      char inc = 1;
      unsigned char start, stop;
      scannLength = 0;
      
      if(setup->length < 1)
      {
        usbDismissSetup();
        return;
      }
      
      //Start and stop channels
      start = setup->value;
      stop = (setup->index>125)?125:setup->index;
      
      if (radioGetDataRate() == DATA_RATE_2M)
        inc = 2; //2M channel are 2MHz wide
      
      //Arm and wait for the out transaction
      OUT0BC = BCDUMMY;
      while (EP0CS & OUTBSY);
      
      memcpy(tbuffer, OUT0BUF, setup->length);
      for (i=start; i<stop+1 && scannLength<MAX_SCANN_LENGTH; i+=inc)
      {
        radioSetChannel(i);
        status = radioSendPacket(tbuffer, setup->length, rbuffer, &rlen);
        
        if (status)
          IN0BUF[scannLength++] = i;
        
        ledTimeout = 2;
        ledSet(LED_GREEN | LED_RED, false);
        if(status)
          ledSet(LED_GREEN, true);
        else
          ledSet(LED_RED, true);
      }
      
      //Ack the setup phase
      usbAckSetup();
      return;
    }
    else if(setup->request == CHANNEL_SCANN && setup->requestType == 0xC0)
    {
      //IN0BUF already contains the right data
      //(if a scann has been launched before ...)
      IN0BC = (setup->length>scannLength)?scannLength:setup->length;
      while (EP0CS & INBSY);
      
      //Ack the setup phase
      usbAckSetup();
      return;
    }
  }
  
  //Stall in error if nothing executed!
  usbDismissSetup();
}

// De-init all the peripherical,
// and launch the Nordic USB bootloader located @ 0x7800
void launchBootloader()
{ 
  void (*bootloader)() = (void (*)())0x7800;
  
  //Deactivate the interruptions
  IEN0 = 0x00;
  
  //Deinitialise the USB
  usbDeinit();
  
  //Deinitialise the radio
  radioDeinit();
  
  //Call the bootloader
  bootloader();
}
