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
 * nrfProg.c nRF24LU01(P) SPI programmer using a Amontec JTAG Key
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "nrfSpi.h"
#include "commands.h"

static char buffer[32*1024];
static char vbuffer[32*1024];

void help(char * port);

void hexdump(int argc, char ** argv);
void nrfread(int argc, char ** argv);
void nrfwrite(int argc, char **argv);

//Utils
void holdReset();
void resetProg();
void resetRun();
void activateSPI();
void deactivateSPI();

int main (int argc, char *argv[])
{

  if(argc<2 || !strcmp(argv[1], "help"))
  {
    help(argv[0]);
    return 0;
  }

  if(spiInit()<0)
    exit(EXIT_FAILURE);
  printf("Ftdi dongle found\n");
  
  //Exeute commands
  if (!strcmp(argv[1], "hexdump"))
    hexdump(argc-2, argv+2);
  if (!strcmp(argv[1], "read"))
    nrfread(argc-2, argv+2);
  if (!strcmp(argv[1], "write"))
    nrfwrite(argc-2, argv+2);
  if (!strcmp(argv[1], "reset"))
    resetRun();
  if (!strcmp(argv[1], "holdreset"))
    holdReset();
  
  
  
  spiDeinit();
  
  return 0;
}

void holdReset() 
{
  //Reset chip
  printf("\nHold the reset\n");
  spiSetResetProg(EN_RESET, DIS_PROG);
}

void resetProg()
{
    //Reset the chip in programation mode
  spiSetResetProg(EN_RESET, DIS_PROG);
  usleep(5000); //Wait at least 5ms
  //spiSetResetProg(DIS_RESET, DIS_PROG);
  spiSetResetProg(DIS_RESET, EN_PROG);
  usleep(5000); //Wait at least 5ms
  
  //Now the chip is in prog, activate the SPI output enable
  activateSPI();
}

void resetRun()
{
  //Deactivate the SPI port
  deactivateSPI();

  //Reset chip
  printf("\nReset the nRF24LU01 in run mode\n");
  spiSetResetProg(EN_RESET, DIS_PROG);
  usleep(50000); //Wait at least 5ms
  spiSetResetProg(DIS_RESET, DIS_PROG);
}

void activateSPI() 
{
  spiSetSpiOE(EN_OE);
  spiSetCS(DIS_CS);

}

void deactivateSPI()
{
  //Deactivate the SPI outputs
  spiSetSpiOE(DIS_OE);
}

void help(char *prog) 
{
  printf("nrfProg V0.01 - nRF24LU1 programmer using the Amontec JTAG Key\n"
         "\n"
         "Usage: %s <COMMAND> <PARAMETERS>\n"
         "Commands:\n"
         "  help:\n"
         "      Display this message\n"
         "  read <filename> [addr] [len]:\n"
         "      read the memory and write a dump in filename\n" 
         "  write <filename> [addr]:\n"
         "      write the content of the binary <filename> into the nRF24LU flash.\n" 
         "  hexdump [addr] [len]:\n"
         "      Display a hex dump of the memory\n"
         "  reset:\n"
         "      Reset the nRF24LU1+\n"
         "  holdreset:\n"
         "      Hold the reset (ie. stop) the nRF24LU1+\n", prog);
}

void hexdump(int argc, char ** argv) {
  int address = 0;
  int len = 32*1024;
  int i,j;
  char line[16];
  
  resetProg();
  
  if(argc>0) sscanf(argv[0], "%d", &address);
  if(argc>1) sscanf(argv[1], "%d", &len);
  
  if(address>0x7FFF) address=0x7FFF;
  if((address+len)>0x7fff) len=0x7FFF-address+1;
  
  printf("hexdump, display %d bytes from 0x%04X\n", len, address);
  cmdRead(address, buffer, len);
  
  for(i=0;i<len;i++) {
    if(!(i%16)) printf("0x%04X :", (unsigned) address+i);
    if(!(i%8)) printf(" ");
    printf("%02X", (unsigned char)buffer[i]);
    line[i%16] = buffer[i];
    if(!((i+1)%16)) {
      for(j=0;j<16;j++) {
        if(!(j%8)) putchar(' ');
        if(isprint(line[j])) putchar(line[j]);
        else putchar('.'); 
      }
      putchar('\n');
    }
  }
  
  resetRun();
  
  return; 
}

void nrfread(int argc, char ** argv) {
  int address = 0;
  int len = 32*1024;
  FILE *ofile;
  
  if (argc==0) {
    fprintf(stderr, "Error: no filename provided!\n");
    return;
  }
  
  if (!(ofile=fopen(argv[0], "w"))) {
    fprintf(stderr, "Error: impossible to open the file %s!\n", argv[0]);
    return;
  }
  
  resetProg();
  
  if(argc>1) sscanf(argv[1], "%d", &address);
  if(argc>2) sscanf(argv[2], "%d", &len);
  
  if(address>0x7FFF) address=0x7FFF;
  if((address+len)>0x7fff) len=0x7FFF-address+1;
  
  printf("read, read %d bytes from 0x%04X. Write into %s.\n", 
         len, address, argv[0]);
  cmdRead(address, buffer, len);
  
  printf("Write the datas in the binary file ...");
  fwrite(buffer, 1, len, ofile);
  
  fclose(ofile);
  printf(" Done!\n");

  resetRun();

  return;
}

void nrfwrite(int argc, char **argv) {
  int address = 0;
  int len = 32*1024;
  FILE *ifile;
  int i;
  int identical;
  
  if (argc==0) {
    fprintf(stderr, "Error: no filename provided!\n");
    return;
  }
  
  if (!(ifile=fopen(argv[0], "r"))) {
    fprintf(stderr, "Error: impossible to open the file %s!\n", argv[0]);
    return;
  }
  
  resetProg();
    
  if(argc>1) sscanf(argv[1], "%d", &address);
  
  //Get the file length
  fseek(ifile, 0, SEEK_END);
  len = ftell(ifile);
  rewind(ifile);
  
  //Limit the size
  if((address+len)>0x7fff) len=0x7FFF-address+1;
  
  //Read the file into the buffer
  fread(buffer, 1, len, ifile);
  
  //Close the file
  fclose(ifile);
  

  
  if(address>0x7FFF) address=0x7FFF;
  if((address+len)>0x7fff) len=0x7FFF-address+1;
  
  
  
  printf("Write, file of %d bytes from 0x%04X. Read from %s.\n", 
         len, address, argv[0]);
  //Write the chip
  cmdProgram(address, buffer, len);
  
  //Write verification
  // TODO: Implement a real verify function
  printf("\nVerify ...\n");
  //Read the chip
  cmdRead(address, vbuffer, len);
  
  //Compare
  identical = 1;
  for(i=0;i<len && identical;i++)
    if(buffer[i] != vbuffer[i]) identical=0;
  
  if(identical)
    printf("%d Bytes verified successfully!\n", len);
  else
    printf("Verrification error at byte %d!\n", i);
  
  resetRun();
  
  return;
}

