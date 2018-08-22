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
 * radio.h - nRF cpu radio driver
 */

#ifndef __RADIO_H__
#define __RADIO_H__

#include <stdbool.h>

enum radioMode_e
{
  RADIO_MODE_PTX,
  RADIO_MODE_PRX,
};

#define ARD_RAW 0
#define ARD_PLOAD 0x80


//High level functions
void radioInit(enum radioMode_e mode);
void radioDeinit();
unsigned char radioSendPacket(__xdata char *payload, char len,
                              __xdata char *ackPayload, char *ackLen);
void radioSendPacketNoAck(__xdata char *payload, char len);
void radioSetChannel(signed char channel);
void radioSetDataRate(unsigned char dr);
char radioGetDataRate();
void radioSetAddress(__xdata char* address);
void radioSetPower(char power);
void radioSetArd(char ard);
void radioSetArc(char arc);
void radioSetContCarrier(bool contCarrier);
uint8_t radioGetRpd(void);
uint8_t radioGetTxRetry(void);

void radioSetMode(enum radioMode_e mode);

bool radioIsRxEmpty();
void radioSetShockburst(char enable);
void radioSetCRC(char enable);
void radioSetCRCLen(char len);


//Each function returns the status register
char radioNop();
char radioActivate();
char radioReadReg(char addr);
char radioWriteReg(char addr, char value);
void radioTxPacket(__xdata char *payload, char len);
void radioTxPacketNoAck(__xdata char *payload, char len);
void radioAckPacket(char pipe, __xdata char* payload, char len);
char radioRxPacket(__xdata char *payload);

void radioShockburstPipes(char pipes);
void radioSetCRC(char enable);
void radioSetCRCLen(char len);
void radioSetAddrLen(char len);
void radioEnableRxPipe(char pipes);
void radioDisableRetry(void);
void radioRxPayloadLen(char pipe, char len);
void radioRxDynPayload(char pipe, bool enable);
void radioTxDynPayload(bool enable);
void radioPayloadAck(bool enable);
void radioTxPayloadNoAck(bool enable);


#endif /* __RADIO_H__ */
