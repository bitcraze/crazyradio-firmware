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
 * srfSpi.h - SPI driver program the nRF24LU1 with a JTAG Key
 */

#ifndef __SRF_SPI_H__
#define __SRF_SPI_H__

// if there is an error the functions will print it on stderr
// the return values are >=0 if ok, <0 if error
int spiInit();
int spiSetResetProg(int reset, int prog);
int spiSetCS(int cs);
int spiSetSpiOE(int oe);
void spiDeinit();

// The driver can send and receive 128bytes max!
// Return the number of bytes read/write
int spiSend(char *data, int len);
int spiReceive(char *data, int len);


// Usefull defines
#define EN_CS 0
#define DIS_CS 1

#define EN_OE 0
#define DIS_OE 1

#define EN_RESET 0
#define DIS_RESET 1

#define EN_PROG 1
#define DIS_PROG 0



#endif /* __SRF_SPI_H__ */

