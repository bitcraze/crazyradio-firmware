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
 * usb.h - nRF CPU USB driver and device implementation
 */

#ifndef __USB_H__
#define __USB_H__

#include <stdbool.h>

//USB state type
typedef enum {POWERED, DEFAULT, ADDRESS, CONFIGURED} usbState_t;

//Initialise the USB framework and interruption
void usbInit();
void usbDeinit();

//send one packet of date into the ep buffer and activate it for the next IN transfers
void usbBulkSend(unsigned char ep, char* buff, unsigned char length);

//States functions
usbState_t usbGetState();

//USB interruptions
void usbWuIsr() __interrupt(11);
void usbIsr() __interrupt(12)  __using(1);

//USB controll structure
struct controllStruct {
  unsigned char requestType;
  unsigned char request;
  unsigned short value;
  unsigned short index;
  unsigned short length;
};

//Vendor setup request handling
bool usbIsVendorSetup(void);
__xdata struct controllStruct * usbGetSetupPacket(void);
void usbAckSetup(void);
void usbDismissSetup(void);

//Data flow handling structure (internal use)
typedef struct {
  char rdy;
  int len;
  int ptr;
  __code char * buffer;
} dataFlow_t;

//USB setup requests
#define GET_STATUS 0
#define CLEAR_FEATURE 1
#define SET_FEATURE 3
#define SET_ADDRESS 5
#define GET_DESCRIPTOR 6
#define GET_CONFIGURATION 8
#define SET_CONFIGURATION 9
#define GET_INTERFACE 10

//Request constant
#define REQUEST_RMASK 0x07
#define REQUEST_DEVICE 0
#define REQUEST_INTERFACE 1
#define REQUEST_ENDPOINT 2

//Feature selector
#define DEVICE_REMOTE_WAKEUP 1
#define ENDPOINT_HALT 0
#define TEST_MODE 2

//Control message types macro
#define IS_DEVICE(X)    ((X&3)==0)
#define IS_INTERFACE(X) ((X&3)==1)
#define IS_ENDPOINT(X)  ((X&3)==2)
#define IS_OTHER(X)     ((X&3)==3)
#define IS_STANDARD(X) ((X&(3<<5))==(0<<5))
#define IS_CLASS(X)    ((X&(3<<5))==(1<<5))
#define IS_VENDOR(X)   ((X&(3<<5))==(2<<5))

//Vendor control messages and commands
#define SET_RADIO_CHANNEL 0x01
#define SET_RADIO_ADDRESS 0x02
#define SET_DATA_RATE     0x03
#define SET_RADIO_POWER   0x04
#define SET_RADIO_ARD     0x05
#define SET_RADIO_ARC     0x06
#define ACK_ENABLE        0x10
#define SET_CONT_CARRIER  0x20
#define CHANNEL_SCANN     0x21
#define SET_MODE          0x22
#define LAUNCH_BOOTLOADER 0xFF

//CMD mode commands
#define CMD_PACKET 0x00
#define CMD_ERROR  0x30

//SET_DATA_RATE parameter
#define DATA_RATE_250K 0
#define DATA_RATE_1M   1
#define DATA_RATE_2M   2

//SET_RADIO_POWER parameter
#define RADIO_POWER_M18dBm 0
#define RADIO_POWER_M12dBm 1
#define RADIO_POWER_M6dBm  2
#define RADIO_POWER_0dBm   3

//SET_MODE parameter
#define MODE_LEGACY 0
#define MODE_CMD    1
#define MODE_PRX    2

// Error values
#define ERROR_UNKNOWN_CMD   0
#define ERROR_MALFORMED_CMD 1

#endif /* __USB_H__ */

