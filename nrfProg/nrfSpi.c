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
 * srfSpi.c - SPI driver program the nRF24LU1 with a JTAG Key
 */

#include <stdio.h>
#include <string.h>

#include <ftdi.h>
#include "nrfSpi.h"

//Global variable used here
static struct ftdi_context ftdic;
static char buffer[256+3];  //128bytes buffer + 3bytes for the send command
static int CS;
static int OE;


//Send the buffer, try 10 times and return the number of bytes sent
int ftdiSendBuffer(char *buff, int len)
{
  int i=0, ret, try=0;
  
  while((i<len) && ((try++)<10)) {
    //printf("i=%d\n", i);
    //putchar('/');
    
    ret = ftdi_write_data(&ftdic, (unsigned char*)buff+i, len);
    i+=ret;
    if (ret<0) {
        fprintf(stderr, "Unable to send to the FTDI chip (%d): %s\n", ret,
                ftdi_get_error_string(&ftdic));
        return -1;
    }
  }
  //puts("------\n");
  return i;
}

//Receive the buffer, try 100 times and return the number of bytes sent
int ftdiRecvBuffer(char* buff, int len)
{
  int i=0, ret, try=0;
  
  while((i<len) && ((try++)<100)) {
    //putchar('.');
    
    ret = ftdi_read_data(&ftdic, (unsigned char*)buff+i, len);
    i+=ret;
    if (ret<0) {
        fprintf(stderr, "Unable to receive from the FTDI chip (%d): %s\n", ret,
                ftdi_get_error_string(&ftdic));
        return -1;
    }
  }
  
  return i;
}


int spiInit()
{
  int i, ret;

  ftdi_init(&ftdic);

  if((ret = ftdi_usb_open(&ftdic, 0x0403, 0xcff8)) < 0)
  {
    fprintf(stderr, "unable to open ftdi device: %d (%s)\n", ret, 
            ftdi_get_error_string(&ftdic));
    return -1;
  }

  if (ftdi_set_interface(&ftdic, INTERFACE_A)<0)
  {
    fprintf(stderr, "Unable to setup the A channel: %s\n",
            ftdi_get_error_string(&ftdic));
    return -1;
  }

  if (ftdi_set_bitmode(&ftdic, 0, BITMODE_MPSSE)<0)
  {
    fprintf(stderr, "Unable to set the channel in MPSSE mode: %s\n",
            ftdi_get_error_string(&ftdic));
    return -1;
  }
  
  //CS and OE will be activated
  OE=1;
  CS=1;
  
  //Setup the dongle signals and frequency
  i=0;

  //Set input/output
  buffer[i++] = SET_BITS_LOW;   
  buffer[i++] = 0x19;       // CS = '1', OE_N=1
  buffer[i++] = 0x1B;       // CS/TCK/DO out, DI in, GPIOL0 out 1-3 in
  
  buffer[i++] = SET_BITS_HIGH;   
  buffer[i++] = 0x00;       // TRST_EN_N, SRST_EN_N=0
  buffer[i++] = 0x1B;       // TRST_(EN)_N and SRST_(EN)_N = 0
  
  //Set the SPI clock at 1.2MHz (fast enough ...)
  buffer[i++] = TCK_DIVISOR;
  buffer[i++] = 0x04;
  buffer[i++] = 0;
  
  return ftdiSendBuffer(buffer, i);
}

int spiSetResetProg(int reset, int prog)
{
  int i=0;
  int value=0;
  
  //Set the bits values
  if (reset) value |= 0x02;
  if (prog)  value |= 0x01;
  
  buffer[i++] = SET_BITS_HIGH;   
  buffer[i++] = value;       // TRST_EN_N, SRST_EN_N=0
  buffer[i++] = 0x0F;       // TRST_(EN)_N and SRST_(EN)_N = out
  
  return ftdiSendBuffer(buffer, i);
}

int spiSetSpiOE(int oe) {
  int i=0;
  int value=0x00; //By default all the SPI lines are at 0
  
  OE=oe;
  
  //Set bits values
  if (CS) value |= 0x08;
  if (OE) value |= 0x10;
  
  buffer[i++] = SET_BITS_LOW;   
  buffer[i++] = value;      // set CS
  buffer[i++] = 0x1B;       // CS/TCK/DO out, DI in, GPIOL0 out 1-3 in    
  
  return ftdiSendBuffer(buffer, i);
}

int spiSetCS(int cs)
{
  int i=0;
  int value=0x00; //By default all the SPI lines are at 0
  
  CS=cs;
  
  //Set bits values
  if (CS) value |= 0x08;
  if (OE) value |= 0x10;
    
  buffer[i++] = SET_BITS_LOW;   
  buffer[i++] = value;      // set CS
  buffer[i++] = 0x1B;       // CS/TCK/DO out, DI in, GPIOL0 out 1-3 in    
  
  return ftdiSendBuffer(buffer, i);
}

void spiDeinit()
{
  ftdi_usb_close(&ftdic);
  ftdi_deinit(&ftdic);
}

int spiSend(char *data, int len)
{
  int i=0;
  
  if(len<1) return 0;
  if(len>128) {
    fprintf(stderr, "Unable to send more than 128 bytes!\n");
    return -1;
  }
  
  len--;

  buffer[i++] = MPSSE_DO_WRITE | MPSSE_WRITE_NEG;
  buffer[i++] = len&0x0FF;
  buffer[i++] = (len>>8)&0x0FF;
  
  memcpy(buffer+3, data, len+1);
  
  return ftdiSendBuffer(buffer, len+1+3)-3;
}

int spiReceive(char *data, int len)
{
  int i=0;
  
  if(len<1) return 0;
  if(len>256) {
    fprintf(stderr, "Unable to receive more than 128 bytes!\n");
    return -1;
  }
  
  len--;

  buffer[i++] = MPSSE_DO_READ;// | MPSSE_READ_NEG  ;
  buffer[i++] = len&0x0FF;
  buffer[i++] = (len>>8)&0x0FF;
  
  if (ftdiSendBuffer(buffer, i)<0)
    return -1;
  
  return ftdiRecvBuffer(data, len+1);
}

