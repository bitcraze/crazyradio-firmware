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
 * radio.c - nRF cpu radio driver
 */

#include "nRF24LU1p.h"
#include "nRF24L01.h"
#include "utils.h"

#include "radio.h"
#include "usb.h"

/* nRF24L SPI commands */
#define CMD_R_REG 0x00
#define CMD_W_REG 0x20
#define CMD_R_RX_PAYLOAD 0x61
#define CMD_W_TX_PAYLOAD 0xA0
#define CMD_FLUSH_TX 0xE1
#define CMD_FLUSH_RX 0xE2
#define CMD_REUSE_TX_PL 0xE3
#define CMD_RX_PL_WID 0x60
#define CMD_W_ACK_PAYLOAD 0xA8
#define CMD_W_TX_PAYLOAD_NO_ACK 0xD0
#define CMD_NOP 0xFF

//Usefull macro
#define RADIO_EN_CS() RFCSN=0
#define RADIO_DIS_CS() RFCSN=1
#define RADIO_EN_CE() RFCE=1
#define RADIO_DIS_CE() RFCE=0
#define CE_PULSE() {int i=50; RFCE=1; while(i--); RFCE=0;}

//Private functions
void radioUpdateRetr(void);
void radioUpdateRfSetup(void);

//Chached state of the radio
static __xdata struct {
  char dataRate;
  char power;
  char arc;
  char ard;
  char contCarrier;
} radioConf = {
  /*.dataRate =*/ DATA_RATE_2M,
  /*.power =*/ RADIO_POWER_0dBm,
  /*.arc =*/ 3,
  /*.ard =*/ ARD_PLOAD | 32,
  /*.contCarrier =*/ 0,
};

//Ack payload length by ARD step (steps of 250uS) (From nrf24l01 doc p.34)
static __code unsigned char ardStep[3][6] = { {0, 0, 8, 16, 24, 32}, //250Kps
                                              {15, 32},              //1Mps
                                              {5, 32},               //2Mps
                                            };

static __code unsigned char setupDataRate[] = {0x20, 0x00, 0x08};

char spiRadioSend(char dt)
{
  //Send the data
  RFDAT = dt;
  //Clear the flag
  RFRDY = 0;
  
  //Wait for the data to be sent
  while(!RFRDY);
  RFRDY=0;
  
  //Return the received data
  return RFDAT;
}

char spiRadioReceive()
{
  return spiRadioSend(0x00);
}

void radioInit()
{
  int i;
  // Clock the radio and enable the radio SPI
  RFCON = 0x06;
  RFCTL = 0x10;  //SPI enable @8MHz
  
  // Energize the radio in PTX mode. Interrupts disable
  radioWriteReg(REG_CONFIG, 0x7E);
  //Wait a little while for the radio to be rdy
  for(i=0;i<1000;i++);
  //Enable the dynamic packet size and the ack payload
  radioWriteReg(REG_FEATURE, 0x06);
  radioWriteReg(REG_DYNPD, 0x01);

  //Set the default radio parameters
  radioUpdateRfSetup();
  radioUpdateRetr();
}

void radioDeinit()
{
  //Deenergise the radio
  radioWriteReg(REG_CONFIG, 0x00);
  
  //Unclock the radio and SPI
  RFCON = 0x00;  //Radio unclocked
  RFCTL = 0x00;  //SPI disable
}


//Nop command, permit to get the status byte
char radioNop()
{
  char status;
  
  RADIO_EN_CS();
  status = spiRadioSend(CMD_NOP);
  RADIO_DIS_CS();
  
  return status;
}

char radioFlushTx()
{
  char status;
  
  RADIO_EN_CS();
  status = spiRadioSend(CMD_FLUSH_TX);
  RADIO_DIS_CS();
  
  return status;
}

char radioFlushRx()
{
  char status;
  
  RADIO_EN_CS();
  status = spiRadioSend(CMD_FLUSH_RX);
  RADIO_DIS_CS();
  
  return status;
}

char radioReadReg(char addr)
{
  char value;
  
  RADIO_EN_CS();
  spiRadioSend(CMD_R_REG | (addr&0x1F));
  value = spiRadioSend(0xA5);
  RADIO_DIS_CS();
  
  return value;
}

char radioWriteReg(char addr, char value)
{
  char status;
  
  RADIO_EN_CS();
  status = spiRadioSend(CMD_W_REG | (addr&0x1F));
  spiRadioSend(value);
  RADIO_DIS_CS();
  
  return value;
}

// Send a packet.
void radioTxPacket(__xdata char *payload, char len)
{
  int i;

  //Send the packet in the TX buffer
  RADIO_EN_CS();
  spiRadioSend(CMD_W_TX_PAYLOAD);
  for(i=0;i<len;i++)
    spiRadioSend(payload[i]);
  RADIO_DIS_CS();
  
  //Pulse CE
  CE_PULSE();
  
  return;
}

//Send a packed in no-ack mode
void radioTxPacketNoAck(__xdata char *payload, char len)
{
  int i;

  //Send the packet in the TX buffer
  RADIO_EN_CS();
  spiRadioSend(CMD_W_TX_PAYLOAD_NO_ACK);
  for(i=0;i<len;i++)
    spiRadioSend(payload[i]);
  RADIO_DIS_CS();
  
  //Pulse CE
  CE_PULSE();
  
  return;
}

//Fetch the next act payload
//Return the payload length
char radioRxPacket(__xdata char *payload)
{
  int len;
  int i;

  //Get the packet length
  RADIO_EN_CS();
  spiRadioSend(CMD_RX_PL_WID);
  len = spiRadioReceive();
  RADIO_DIS_CS();  
  
  if (len>0 && len<33)
  {
    //Read the packet from the RX buffer
    RADIO_EN_CS();
    spiRadioSend(CMD_R_RX_PAYLOAD);
    for(i=0;i<len;i++)
      payload[i] = spiRadioReceive();
    RADIO_DIS_CS();
  } else {
    len=0;
  }
  
  //Pulse CE
  //CE_PULSE();
  
  return len;
}

//Send a packet and receive the ACK
//Return true in case of success.
//Polling implementation
unsigned char radioSendPacket(__xdata char *payload, char len, 
                              __xdata char *ackPayload, char *ackLen)
{
  char status = 0;
  
  //Send the packet
  radioTxPacket(payload, len);
  //Wait for something to happen
  while(((status=radioNop())&0x70) == 0);
  
  // Clear the flags
  radioWriteReg(REG_STATUS, 0x70);
  
  //Return FALSE if the packet has not been transmited
  if (status&BIT_MAX_RT) {
    radioFlushTx();
    return 0;
  }
    
  //Receive the ackPayload if any has been received
  if (status&BIT_RX_DR)
    *ackLen = radioRxPacket(ackPayload);
  else 
    *ackLen = 0;
  
  radioFlushRx();
  
  return status&BIT_TX_DS;
}

//Send a packet and don't wait for the Acknoledge
void radioSendPacketNoAck(__xdata char *payload, char len)
{
  //Wait for the TX fifo not to be full
  while((radioNop()&0x01) != 0);

  //Send the packet
  radioTxPacketNoAck(payload, len);

  //Nothing to wait for, the packet is 'just' sent!
}

//Raw registers update (for internal use)
void radioUpdateRetr()
{
  char ard=0;
  unsigned char nbytes;
  
  if (radioConf.ard & ARD_PLOAD)
  {
    nbytes = ((radioConf.ard&0x7F)>32)?32:(radioConf.ard&0x7F);
    for (ard=0; ardStep[radioConf.dataRate][ard]<nbytes; ard++)
      continue;
  } else
    ard = radioConf.ard & 0x0F;
  
  radioWriteReg(REG_SETUP_RETR, (ard<<4) | (radioConf.arc&0x0F)); 
}

void radioUpdateRfSetup()
{
  unsigned char setup=0;
  
  setup = setupDataRate[radioConf.dataRate];
  setup |= radioConf.power<<1;
  
  if (radioConf.contCarrier)
    setup |= 0x90;
  
  radioWriteReg(REG_RF_SETUP, setup);
}

//Set the radio channel.
void radioSetChannel(char channel)
{
  //Test the input
  if(channel<0 || channel>125)
    return;
   
  //Change the channel
  RADIO_DIS_CE();
  radioWriteReg(REG_RF_CH, channel);
  
  //CE is continously activated if in continous carrier mode
  if(radioConf.contCarrier)
    RADIO_EN_CE();
}

//Set the radio datarate
void radioSetDataRate(unsigned char dr)
{
  if (dr>=3)
    return;
  
  radioConf.dataRate = dr;
  
  radioUpdateRfSetup();
  radioUpdateRetr();
}

char radioGetDataRate()
{
  return radioConf.dataRate;
}

void radioSetPower(char power)
{
  radioConf.power = power&0x03;
  
  radioUpdateRfSetup();
}

void radioSetArd(char ard)
{
  radioConf.ard = ard;
  
  radioUpdateRetr(); 
}

void radioSetArc(char arc)
{
  radioConf.arc = arc;
  
  radioUpdateRetr();
}

void radioSetContCarrier(bool contCarrier)
{
  radioConf.contCarrier = contCarrier?1:0;
  
  RADIO_DIS_CE();
  
  radioUpdateRfSetup();
  
  if(contCarrier)
    RADIO_EN_CE();
}

//Set the TX and RX address
void radioSetAddress(__xdata char* address)
{
  int i;

  RADIO_EN_CS();
  spiRadioSend(CMD_W_REG | REG_TX_ADDR);
  for(i=0; i<5; i++)
    spiRadioSend(address[i]);
  RADIO_DIS_CS();

  RADIO_EN_CS();
  spiRadioSend(CMD_W_REG | REG_RX_ADDR_P0);
  for(i=0; i<5; i++)
    spiRadioSend(address[i]);
  RADIO_DIS_CS();
}

//Get the radio power detector value
uint8_t radioGetRpd(void)
{
    return radioReadReg(REG_RPD);
}

//Get the number of retry to send the last packet
uint8_t radioGetTxRetry(void)
{
    return radioReadReg(REG_OBSERVE_TX)&0x0F;
}

