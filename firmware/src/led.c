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

#define RED_PIN (1<<2)
#define GREEN_PIN (1<<4)

void ledInit()
{
  //Set P0.2 and P0.4 as output
  P0DIR &= ~0x14;
  //Clear P0.2 and P0.4
  P0 &= ~0x14;
}

void ledSet(led_t led, bool value)
{
  if(led&LED_RED)
  {
    if (value)
      P0 |= RED_PIN;
    else
      P0 &= ~RED_PIN;
  }

  if (led & LED_GREEN) 
  {
    if (value)
      P0 |= GREEN_PIN;
    else
      P0 &= ~GREEN_PIN;
  }
}
