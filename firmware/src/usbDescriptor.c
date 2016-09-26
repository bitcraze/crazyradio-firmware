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
 * usbDescriptors.c : USB descriptors instantiation
 */

#include "usbDescriptor.h"

/* Device descriptor */
__code const unsigned char usbDeviceDescriptor[] = {
  18,                 //bLength
  DEVICE_DESCRIPTOR,  //bDescriptorType
  0x00, 0x02,         //bcdUSB, usb 2.0
  0x00,               //bDeviceClass
  0x00,               //bDeviceSubClass
  0x00,               //bDeviceProtocol
  64,                 //bMaxPacketSize0
  0x15, 0x19,         //idVendor (Nordic)
  0x77, 0x77,         //idProduct (Randomly chosen for the development)
  0x55, 0x99,         //bcdDevice (Dev version v99.55)
  0x01,               //iManufacturer (String 1)
  0x02,               //iProduct (String 2)
  0x1D,               //iSerialNumber (the ID is at index 0x1D)
  1                   //bNumConfiguration
};

//The only configuration descriptor
//Monolitique implementation to ease the code development
__code const unsigned char usbConfigurationDescriptor[57] = {
  /***** Configuration descriptor ******/
  9,                         //bLength
  CONFIGURATION_DESCRIPTOR,  //bDescriptorType
#ifdef PPM_JOYSTICK
  57,0x00,                   //wTotalLength
  2,                         //bNumInterfaces
#else
  32,0x00,                   //wTotalLength
  1,                         //bNumInterfaces
#endif
  1,                         //bConfigurationValue
  0,                         //iConfiguration
  0x80,                      //bmAttribute (Bus powered, no remote wakeup)
  50,                        //bMaxPower (100mA, shall be enough)
  /***** Interface 0 descriptor: Crazyradio EPs ******/
  9,                         //bLength
  INTERFACE_DESCRIPTOR,      //bDescriptorType
  0,                         //bInterfaceNumber
  0,                         //bAlternateSetting
  2,                         //bNumEndpoint (one in, one out)
  0xFF,                      //bInterfaceClass (VENDOR=0xFF)
  0xFF,                      //bInterfaceSubClass (VENDOR=0xFF)
  0,                         //bInterfaceProtocol (None)
  0,                         //iInterface
  /***** Endpoint 1 IN descriptor ******/
  7,                         //bLength
  ENDPOINT_DESCRIPTOR,       //bDescriptorType
  0x81,                      //bEndpointAddess (EP1 IN)
  0x02,                      //bmAttributes (Bulk endpoint)
  0x40, 0x00,                //wMaxPacketSize (64 bytes)
  6,                         //bInterval (irrelevant for bulk endpoint)
  /***** Endpoint 1 OUT descriptor ******/
  7,                         //bLength
  ENDPOINT_DESCRIPTOR,       //bDescriptorType
  0x01,                      //bEndpointAddess (EP1 OUT)
  0x02,                      //bmAttributes (Bulk endpoint)
  0x40, 0x00,                //wMaxPacketSize (64 bytes)
  6,                         //bInterval (irrelevant for bulk endpoint)
#ifdef PPM_JOYSTICK
  /***** Interface 1 descriptor: HID PPM Joystick ******/
  9,                         //bLength
  INTERFACE_DESCRIPTOR,      //bDescriptorType
  1,                         //bInterfaceNumber
  0,                         //bAlternateSetting
  1,                         //bNumEndpoint (one in)
  0x03,                      //bInterfaceClass (HID=3)
  0x00,                      //bInterfaceSubClass (No subclass)
  0,                         //bInterfaceProtocol (None)
  0,                         //iInterface
  /***** HID Device Descriptor ******/
  9,                         //bLength
  HID_DESCRIPTOR,            //bDescriptorType
  0x11, 0x01,                //bcdHID (1.10)
  0x00,                      //bCountryCode (Not Supported)
  0x01,                      //bNumDescriptor
  HID_REPORT,                //bDescriptorType
  32, 0x00,                //wDescriptorLength (32)
  /***** Endpoint 2 IN descriptor ******/
  7,                         //bLength
  ENDPOINT_DESCRIPTOR,       //bDescriptorType
  0x82,                      //bEndpointAddess (EP2 IN)
  0x03,                      //bmAttributes (Interrupt endpoint)
  0x40, 0x00,                //wMaxPacketSize (64 bytes)
  20,                        //bInterval (20ms (50Hz update))
#endif //PPM_JOYSTICK
};

#ifdef PPM_JOYSTICK
//HID report descriptor for PPM Joystick
__code const unsigned char usbHidReportDescriptor[32] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,                    // USAGE (Joystick)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x09, 0x04,                    //   USAGE (Joystick)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x09, 0x30,                    //     USAGE (X)
    0x36, 0x00, 0xfc,              //     PHYSICAL_MINIMUM (-1024)
    0x46, 0xff, 0x03,              //     PHYSICAL_MAXIMUM (1023)
    0x16, 0x00, 0xfc,              //     LOGICAL_MINIMUM (-1024)
    0x26, 0xff, 0x03,              //     LOGICAL_MAXIMUM (1023)
    0x75, 0x10,                    //     REPORT_SIZE (16)
    0x95, 0x08,                    //     REPORT_COUNT (8)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0xc0,                          //     END_COLLECTION
    0xc0,                          // END_COLLECTION
};
#endif //PPM_JOYSTICK

//String descriptor for the language
__code char usbStringDescriptor0[4] = {
  4,                  //bLength
  STRING_DESCRIPTOR,  //bDescriptor type
  0X09, 0X04          //English (United States)
};

//Manufacturer name "Bitcraze"
__code char usbStringDescriptor1[18] = {
  18,                 //bLength
  STRING_DESCRIPTOR,  //bDescriptor type
  //bString ...
  'B', 0, 'i', 0, 't', 0, 'c', 0, 'r', 0, 'a', 0, 'z', 0, 'e', 0,
};

#ifndef CRPA
//Product name "Crazyradio USB Dongle"
__code char usbStringDescriptor2[44] = {
  44,                 //bLength
  STRING_DESCRIPTOR,  //bDescriptor type
  //bString ...
  'C',0, 'r',0, 'a',0, 'z',0, 'y',0, 'r',0, 'a',0, 'd',0, 'i',0, 'o',0, ' ',0,
  'U',0, 'S',0, 'B',0, ' ',0, 'D',0, 'o',0, 'n',0, 'g',0, 'l',0, 'e',0
};
#else
//Product name "Crazyradio USB Dongle"
__code char usbStringDescriptor2[50] = {
  50,                 //bLength
  STRING_DESCRIPTOR,  //bDescriptor type
  //bString ...
  'C',0, 'r',0, 'a',0, 'z',0, 'y',0, 'r',0, 'a',0, 'd',0, 'i',0, 'o',0, ' ',0,
  'P', 0, 'A', 0, ' ', 0, 'U',0, 'S',0, 'B',0, ' ', 0,
  'D',0, 'o',0, 'n',0, 'g',0, 'l',0, 'e', 0
};
#endif

