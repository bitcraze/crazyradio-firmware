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
 * utils.c - Various utilities functions
 */

#include <stdint.h>

#include "nRF24LU1p.h"

int ledTimeout;
__xdata uint8_t chip_id[5];

__xdata __at (0x000B) uint8_t infopage_id[5];

void initId()
{
  int i;

  //Activate the info page
  FSR |= FSR_INFEN;

  //Read the ID
  for (i=0; i<5; i++)
    chip_id[i] = infopage_id[i];

  //Deactivate the info page
  FSR &= ~FSR_INFEN;
}
