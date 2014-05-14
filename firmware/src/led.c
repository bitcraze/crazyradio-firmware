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
 * led.c - Led driver
 */

#include "nRF24LU1p.h"
#include "led.h"

static unsigned char redPin;
static unsigned char greenPin;

void ledInit(unsigned char redpin, unsigned char greenpin)
{
  redPin = redpin;
  greenPin = greenpin;

  //Set red and green led pins as output
  P0DIR &= ~((1<<redPin) | (1<<greenPin));
  //Clear red and green leds
  P0 &= ~((1<<redPin) | (1<<greenPin));
}

void ledSet(led_t led, bool value)
{
  if(led&LED_RED)
  {
    if (value)
      P0 |= (1<<redPin);
    else
      P0 &= ~(1<<redPin);
  }

  if (led & LED_GREEN) 
  {
    if (value)
      P0 |= (1<<greenPin);
    else
      P0 &= ~(1<<greenPin);
  }
}
