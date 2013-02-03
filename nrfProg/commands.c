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
 * commands.c - High level programming commands implementation
 */

#include <stdio.h>
#include <string.h>

#include "nrfSpi.h"

// Implemented instruction set ...
#define WREN 0x06
#define WRDIS 0x04
#define RDSR 0x05
#define WRSR 0x01
#define READ 0x03
#define PROGRAM 0x02
#define E_PAGE 0x52


static char buffer[3];

int cmdWren() {
  int i=0;
  int ret;

  buffer[i++] = WREN;
  
  spiSetCS(EN_CS);
  ret = spiSend(buffer, i);
  spiSetCS(DIS_CS);
  
  return ret;
}

int cmdWrdis() {
  int i=0;
  int ret;

  buffer[i++] = WRDIS;
  
  spiSetCS(EN_CS);
  ret = spiSend(buffer, i);
  spiSetCS(DIS_CS);
  
  return ret;
}

int cmdRdsr(char *fsr) {
  int i=0;
  int ret;

  buffer[i++] = RDSR;
  
  spiSetCS(EN_CS);
  spiSend(buffer, i);
  ret = spiReceive(buffer, 1);
  spiSetCS(DIS_CS);
  
  *fsr = buffer[0];
  
  return ret;
}

int cmdWrsr(char fsr) {
  int i=0;
  int ret;

  buffer[i++] = WRSR;
  buffer[i++] = fsr;
  
  spiSetCS(EN_CS);
  ret = spiSend(buffer, i);
  spiSetCS(DIS_CS);
  
  return ret;
}

// Read the memory
// The read is done by block of 128bytes
int cmdRead(int addr, char *data, int len)
{
  int i=0;
    
  // Block counter
  for(i=0; i<len; i+=128) {
    //Fancy(?) diplay
    printf("\rRead byte %d/%d (%3d%%)", i, len, (100*i)/len);
    fflush(stdout);
    
    buffer[0] = READ;
    buffer[1] = ((addr+i)>>8)&0x0FF;
    buffer[2] = (addr+i)&0x0FF;
    
    spiSetCS(EN_CS);
    spiSend(buffer, 3);
    spiReceive(data+i, ((len-i)>128)?128:(len-i) );
    spiSetCS(DIS_CS);
  }
  
  printf("\rRead byte %d/%d (%3d%%)\n", i, len, 100);
  
  return i;
}

// Fix the start address and length on 512bytes boundary
// Erase complete pages and fill the gaps with 0xFF
int cmdProgram(int addr, char *data, int len) {
  static char page[512];
  char fsr;
  int i, j;
  int oaddr = addr;
  int olen  = len;
  int ptr=0;

  if (addr>30*1024) {
    printf("ERROR: Writing above 30K is disabled!\n");
    return;
  }

  if((addr+len)>30*1024) {
    len = (30*1024)-addr;
    printf("WARNING Write above 30K disabled! The new write length is %d\n", len);
  }

  //Fix address and length on the good boundary
  addr = (addr/512)*512;
  if(len%512) len = ((len/512)+1)*512;
  
  printf("Write the device from 0x%04X, with a length of %d bytes\n", addr, len);
  
  //Erase the blocks
  for(i=(addr/512);i<((addr+len)/512); i++) {
    printf("\rErasing page %d/%d ...", i, (len/512)-1);
    fflush(stdout);
    cmdWren();
    cmdErasePage(i);
    cmdRdsr(&fsr);
    //Wait for the page to be erased (FSR -> 0 ...)
    while(fsr)
      cmdRdsr(&fsr);
  }
  printf(" Done!\n");
  
  //Write pages
  for(i=(addr/512);i<((addr+len)/512); i++) {
    printf("\rWriting page %d/%d ...", i, (len/512)-1);
    fflush(stdout);
    //Copy the page into the page buffer
    for(j=0;j<512;j++) {
      if(((i*512)+j)<oaddr) page[j]=0xFF;
      else if (((i*512)+j)>(oaddr+olen)) page[j]=0xFF;
      else page[j] = data[ptr++];
    }
    
    //Write the 4*128bytes of the page
    for(j=0;j<4;j++) {
      cmdWren();
      
      buffer[0] = PROGRAM;
      buffer[1] = (((i*512)+(j*128))>>8)&0x0FF;
      buffer[2] = ((i*512)+(j*128))&0x0FF;
      
      spiSetCS(EN_CS);
      spiSend(buffer, 3);
      spiSend(page+(j*128), 128);
      spiSetCS(DIS_CS);
    
      cmdRdsr(&fsr);
      while(fsr) cmdRdsr(&fsr);
    }
  }
  printf("Done!\n");
  
  return 0;
}

int cmdErasePage(int pageNo) {
  int i=0;
  int ret;

  buffer[i++] = E_PAGE;
  buffer[i++] = pageNo;
  
  spiSetCS(EN_CS);
  ret = spiSend(buffer, i);
  spiSetCS(DIS_CS);
  
  return ret;
}
