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
 * usb.c - nRF CPU USB driver and device implementation
 */

#include <string.h>

#include "led.h"
#include "usb.h"
#include "utils.h"
#include "usbDescriptor.h"
#include "nRF24LU1p.h"

#define MIN(A,B) (((int)A<(int)B)?A:B)

//Local functions (USB ISR ...)
void usbBulkOutIsr(char ep);
void usbBulkInIsr(char ep);
void usbSetupIsr();
void usbVendorIsr();
void usbClassIsr();
void usbSendIdString(void);

//USB device State
static usbState_t state = POWERED;
static unsigned char configuration;
static __bit launchBootloader = 0;
static char radioChannel = -1;
static char dataRate = -1;
static bool vendorSetup=false;

//Dataflow in and out. One per endpoint that needs dataflow.
dataFlow_t inflow[1];
//dataFlow_t outflow[1];

void usbInit() 
{
  long i;
  
  state = POWERED;

  //Wake up the USB peripheral
  USBCON = WU;

  //Disconnect the USB pullup (if necessary)
  USBCS |= DISCON;
  //Reconnect the USB Pullup (ie. now the device is detected on the bus)
  for(i=0;i<50000;i++);
  USBCS &= ~DISCON;

  //Deactivate all the EP and there interrupts
  INBULKVAL=0;   //IN EP
  IN_IEN=0;
  OUTBULKVAL=0;  //OUT EP
  OUT_IEN=0;
  INISOVAL=0;    //ISO EP
  OUTISOVAL=0;
  
  //Enable the USB reset and suspend interrupts
  USBIEN = URESIE | SUSPIE;
  
  //Clear any pending interruption flags
  IN_IRQ = 0xFF;
  OUT_IRQ = 0xFF;
  USBIRQ = 0xFF;
  
  //Address pointers (EP0, EP1, EP2 and EP3. Each 64Bytes buffer)
  //This configuration is using the full 512Bytes buffer (no room for double
  //buffering&co). The configuration is explained p.186 of the documentation
  BOUT1ADDR = 32;
  BOUT2ADDR = 64;
  BOUT3ADDR = 96;
  BINSTADDR = 0x40; // EPOUT_Size/4 = 256/4 = 64
  BIN1ADDR  = 32;
  BIN2ADDR  = 64;
  BIN3ADDR  = 96;
  
  //Enable global USB interrupt
  IEN1 |= 0x18;  
}

void usbDeinit()
{
  state = POWERED; //Deactivate all mechanism
  
  //Disable interruptions
  IEN1 &= (~0x18);
  
  //Suspend the USB peripherical
  USB_SUSPEND = 1;
}

usbState_t usbGetState()
{
  return state;
}

void usbWuIsr() __interrupt(11)
{
//  state = ADDRESS;inflow[0].buffer[2]<SETUPBUF[5]
  return;
}

//USB interruption vector
//Call the interruption routine (if existing) and clear the right flag
void usbIsr() __interrupt(12)  __using(1)
{
  //Switch on the USB interruption vector.
  switch (IVEC) {
  case IRQ_SUDAV:
    //Launch the good ISR regarding the type of control packet
    if(IS_STANDARD(SETUPBUF[0]))
      usbSetupIsr();
    else if(IS_VENDOR(SETUPBUF[0]))
      usbVendorIsr();
    else if(IS_CLASS(SETUPBUF[0]))
      usbClassIsr();
    else //If it is a reserved control, it should be dismissed
      EP0CS = EP0STALL; //Stall to error
    
    USBIRQ = SUDAVIR;
    break;
  case IRQ_SOF:
    if((ledTimeout>=0) && (!ledTimeout--)) {
      ledSet(LED_RED, false);
      ledSet(LED_GREEN, false);
      ledTimeout = 0;
    }
    USBIRQ = SOFIR;
    break;
  case IRQ_SUTOK:
    USBIRQ = SUTOKIR;
    break;
  case IRQ_SUSPEND:
    USBIRQ = SUSPIR;
    break;
  case IRQ_USBRESET:
    //Switch to DEFAULT state 
    state = DEFAULT;
    
    // Activate EP0 only
    INBULKVAL = IN0VAL;
    OUTBULKVAL= OUT0VAL;
    
    //Make sure that EP0 OUT is ready to receive
    OUT0BC = BCDUMMY;
    
    //Enable the EP0 (only) and Setup interrupts
    IN_IEN = IN0IE;
    OUT_IEN = OUT0IE;
    USBIEN |= SUDAVIE | SUTOKIE;
    
    USBIRQ = URESIR;
    break;
  case IRQ_EP0IN:
    usbBulkInIsr(0);
    IN_IRQ = IN0IR;
    break;
  case IRQ_EP0OUT:
    usbBulkOutIsr(0);
    //EP0CS = HSNAK;  //Clear hsnak to ack the data packet
    OUT_IRQ = OUT0IR;
    break;
  case IRQ_EP1IN:
    IN_IRQ = IN1IR;
    break;
  case IRQ_EP1OUT:
    OUT_IRQ = OUT1IR;
    break;
  case IRQ_EP2IN:
    IN_IRQ = IN2IR;
    break;
  case IRQ_EP2OUT:
    OUT_IRQ = OUT2IR;
    break;
  case IRQ_EP3IN:
    IN_IRQ = IN3IR;
    break;
  case IRQ_EP3OUT:
    OUT_IRQ = OUT3IR;
    break;
  case IRQ_EP4IN:
    IN_IRQ = IN4IR;
    break;
  case IRQ_EP4OUT:
    OUT_IRQ = OUT4IR;
    break;
  case IRQ_EP5IN:
    IN_IRQ = IN5IR;
    break;
  case IRQ_EP5OUT:
    OUT_IRQ = OUT5IR;  
    break;
  }
}

//Automatic bulk out irq
//fill in the outflow buffer of the EP if it is active
//EP0 only implementation- Drop every packet
void usbBulkOutIsr(char ep) {
  ep;
  return;  
}

//Automatic bulk in irq
//fill in the outflow buffer of the EP if it is active
//EP0 only implementation
void usbBulkInIsr(char ep)
{
  unsigned char lenToSend = (inflow[ep].len<64)?inflow[ep].len:64;
  
  if(inflow[ep].rdy)
  {
    usbBulkSend(ep, inflow[ep].buffer+inflow[ep].ptr, lenToSend);

    inflow[ep].len-=lenToSend;
    inflow[ep].ptr+=lenToSend; 

    if(inflow[ep].len == 0 && lenToSend != 64)
    {
      inflow[ep].rdy = 0;
      inflow[ep].ptr = 0;
    }
  }
  
  //OUT0BC = 1; //Reactivate the EP0 as listen
  return;
}

//Handle and answer the standard control transfers for EP0
void usbSetupIsr()
{
  __xdata struct controllStruct *setup = (__xdata void*)SETUPBUF;

  if(state >= DEFAULT)
  {
    // Get device descriptor request
    if(SETUPBUF[1] == GET_DESCRIPTOR && SETUPBUF[3] == DEVICE_DESCRIPTOR)
    {
      unsigned short dLength = ((unsigned short)SETUPBUF[7]<<8) + ((unsigned short)SETUPBUF[6]<<0);
      //put the descriptor in the inflow structure
      inflow[0].buffer = usbDeviceDescriptor;
      inflow[0].len = MIN(dLength, inflow[0].buffer[0]);
      inflow[0].ptr = 0;
      inflow[0].rdy = 1;
      
      //Initiate the in transfert
      EP0CS = HSNAK;
      usbBulkInIsr(0);
      return;
    }
    
    //Get String descriptor request
    if (SETUPBUF[1] == GET_DESCRIPTOR && SETUPBUF[3] == STRING_DESCRIPTOR)
    {
      unsigned short dLength = ((unsigned short)SETUPBUF[7]<<8) + ((unsigned short)SETUPBUF[6]<<0);
      //put the descriptor in the inflow structure
      if (SETUPBUF[2]==0)
        inflow[0].buffer = usbStringDescriptor0;
      else if (SETUPBUF[2]==1)
        inflow[0].buffer = usbStringDescriptor1;
      else if (SETUPBUF[2]==2)
        inflow[0].buffer = usbStringDescriptor2;
      else if (SETUPBUF[2]==0x1d) {
        usbSendIdString();
        
        EP0CS = HSNAK;
        return;
      } else {
        EP0CS = EP0STALL; //Stall to error
        return;
      }
      
      inflow[0].len = MIN(dLength, inflow[0].buffer[0]);
      inflow[0].ptr = 0;
      inflow[0].rdy = 1;
      
      //Initiate the in transfert
      EP0CS = HSNAK;
      usbBulkInIsr(0);
      return;
    }
    
    // Get Configuration descriptor request
    if(SETUPBUF[1] == GET_DESCRIPTOR && SETUPBUF[3] == CONFIGURATION_DESCRIPTOR)
    {
      unsigned short dLength = ((unsigned short)SETUPBUF[7]<<8) + ((unsigned short)SETUPBUF[6]<<0);
      //put the descriptor in the inflow structure
      inflow[0].buffer = usbConfigurationDescriptor;
      inflow[0].len = MIN(dLength, sizeof(usbConfigurationDescriptor));
      inflow[0].ptr = 0;
      inflow[0].rdy = 1;
      
      //Initiate the in transfert
      EP0CS = HSNAK;
      usbBulkInIsr(0);
      return;
    }

#ifdef PPM_JOYSTICK    
    // HID requests
    if(SETUPBUF[1] == GET_DESCRIPTOR && (SETUPBUF[3]&0xF0)==0x20)
    {
      unsigned short dLength = ((unsigned short)SETUPBUF[7]<<8) + ((unsigned short)SETUPBUF[6]<<0);
      
      if(SETUPBUF[3] == HID_DESCRIPTOR) {
        inflow[0].buffer = usbConfigurationDescriptor+USB_HID_DESC_OFFSET;
        inflow[0].len = MIN(dLength, 9);
      } else if (SETUPBUF[3] == HID_REPORT) {
        inflow[0].buffer = usbHidReportDescriptor;
        inflow[0].len = MIN(dLength, sizeof(usbHidReportDescriptor));
      } else {
        EP0CS = EP0STALL; //Stall to error
        return;
      }
      inflow[0].ptr = 0;
      inflow[0].rdy = 1;
      
      //Initiate the in transfert
      EP0CS = HSNAK;
      usbBulkInIsr(0);
      return;
    }
#endif //PPM_JOYSTICK
    
    //Set address
    if (setup->request == SET_ADDRESS)
    {
      //The usb peripheral take care of the setAdress
      if (setup->value == 0)
      {
        //Switch to DEFAULT state if address==0
        state = DEFAULT;
        
        // Activate EP0 only
        INBULKVAL = IN0VAL;
        OUTBULKVAL= OUT0VAL;
        
        //Make sure that EP0 OUT is ready to receive
        OUT0BC = BCDUMMY;
        
        //Enable the EP0 (only) and Setup interrupts
        IN_IEN = IN0IE;
        OUT_IEN = OUT0IE;
        USBIEN |= SUDAVIE | SUTOKIE;
      } else {
        state = ADDRESS;  //The device is now addressed
      }
      
      EP0CS = HSNAK;
      return;
    }
  }
  
  if (state >= ADDRESS)
  {
    //Set configration
    //Always consider that it is the 1rst configuration
    if (SETUPBUF[1] == SET_CONFIGURATION)
    {
      if (setup->value == 0)
      {
        state = ADDRESS;
        
        //Disable EPs
        INBULKVAL  &= ~(0x02 | 0x04);
        OUTBULKVAL &= ~(0x02);
      
        //Disable interruptions
        OUT_IEN &= ~0x02;
        IN_IEN  &= ~0x02;
      } else {
        state = CONFIGURED;
      
        //Enable the EP1 and EP2IN endpoints
        INBULKVAL  |= 0x02 | 0x04;
        OUTBULKVAL |= 0x02;
      
        //Activate the interruption
        OUT_IEN |= 0x02;
        IN_IEN  |= 0x02;
        USBIEN  |= SOFIE;
      }
      
      EP0CS = HSNAK;
      return;
    }
    
    if (setup->request == GET_CONFIGURATION)
    {
      if (state == CONFIGURED)
        IN0BUF[0]=1;
      else
        IN0BUF[0]=0;
      
      IN0BC=1;
      
      EP0CS = HSNAK;
      return;
    }
    
    if(SETUPBUF[1] == GET_STATUS)
    {
      if ((SETUPBUF[0] & REQUEST_RMASK) == REQUEST_ENDPOINT)
      {
        if ((SETUPBUF[4]==0) || (state == CONFIGURED))
        {
          //Check if the EP is halted...
          if (setup->index == 0x81)
            IN0BUF[0] = IN1CS;
          else if (setup->index == 0x82)
            IN0BUF[0] = IN2CS;
          else if (setup->index == 0x83)
            IN0BUF[0] = IN3CS;
          else if (setup->index == 0x01)
            IN0BUF[0] = OUT1CS;
          else if (setup->index == 0x02)
            IN0BUF[0] = OUT2CS;
          else if (setup->index == 0x03)
            IN0BUF[0] = OUT3CS;
          else
            IN0BUF[0]=1;
          
          
          
          IN0BUF[1]=0;
          IN0BC = 2; //2 bytes
          
          EP0CS = HSNAK; //HSNACK dis
          return;
        }
        //Else STALL will be sent
      }
      else
      {
        //The answer is always 0
        IN0BUF[0]=0;
        IN0BUF[1]=0;
        IN0BC = 2; //2 bytes

        EP0CS = HSNAK; //HSNACK dis
        return; 
      }
    }
  }
  
  if (state >= CONFIGURED)
  {
    if(SETUPBUF[1] == GET_INTERFACE)
    {
      //Send 0 as there is no alternate interface
      IN0BUF[0]=0;
      IN0BC = 1;

      EP0CS |= HSNAK; //HSNACK dis
      return;    
    }
    
    if(setup->request == CLEAR_FEATURE)
    {
      if (IS_ENDPOINT(setup->requestType) && (setup->value == ENDPOINT_HALT))
      {
        if (setup->index == 0x81)
          IN1CS = 0;
        else if (setup->index == 0x82)
          IN2CS = 0;
        else if (setup->index == 0x83)
          IN3CS = 0;
        else if (setup->index == 0x01)
          OUT1CS = 0;
        else if (setup->index == 0x02)
          OUT2CS = 0;
        else if (setup->index == 0x03)
          OUT3CS = 0;
        else {
          EP0CS = EP0STALL;
          return;
        }
        
        EP0CS |= HSNAK; //HSNACK dis
        return; 
      }
    }
    
    if(setup->request == SET_FEATURE)
    {
      if (IS_ENDPOINT(setup->requestType) && (setup->value == ENDPOINT_HALT))
      {
        if (setup->index == 0x81)
          IN1CS = EPSTALL;
        else if (setup->index == 0x82)
          IN2CS = EPSTALL;
        else if (setup->index == 0x83)
          IN3CS = EPSTALL;
        else if (setup->index == 0x01)
          OUT1CS = EPSTALL;
        else if (setup->index == 0x02)
          OUT2CS = EPSTALL;
        else if (setup->index == 0x03)
          OUT3CS = EPSTALL;
        else {
          EP0CS = EP0STALL;
          return;
        }
        
        EP0CS |= HSNAK; //HSNACK dis
        return; 
      }
    }
    
    if(setup->request == SET_FEATURE)
    {
    
    }
  }
  
  //Stall if nothing executed!
  EP0CS = EP0STALL; //Stall to error
}

void usbVendorIsr() 
{
  vendorSetup = true;
}

void usbClassIsr() {
  if (state >= CONFIGURED)
  {
    ;
  }
  
  EP0CS = EP0STALL;
}

//Set a packet to be send for the next IN transfers
//EP0 only implementation
void usbBulkSend(unsigned char ep, char* buff, unsigned char len) {
  ep;
  //Copy the data in the IN buffer
  memcpy(IN0BUF, buff, len);
  //Set the data length (This will give the endpoint control to the hardware)
  IN0BC = len;
}

static __code char bin2hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', 
                                '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

//Send the chip ID as a standard USB string on EP0IN.
//The ID is 10 hexadecimal digit*2 (UTF-16)+lenght+type=22Bytes
void usbSendIdString(void)
{
  int i=0;
  
  IN0BUF[0] = 22;
  IN0BUF[1] = STRING_DESCRIPTOR;
  
  for (i=0; i<5; i++)
  {
    IN0BUF[2+(i*4)] = bin2hex[(chip_id[i]>>4)&0x0F];
    IN0BUF[3+(i*4)] = 0;
    IN0BUF[4+(i*4)] = bin2hex[chip_id[i]&0x0F];
    IN0BUF[5+(i*4)] = 0;
  }
  
  IN0BC = 22;
}

//USB vendor setup access

bool usbIsVendorSetup(void)
{
  if (vendorSetup)
  {
    vendorSetup = false;
    return true;
  }
  return false;
}

__xdata char * usbGetSetupOutData(void);
void usbSetSetupInData(__xdata char *data, int length);


__xdata struct controllStruct * usbGetSetupPacket(void)
{
    return (__xdata void*) SETUPBUF;
}

void usbAckSetup(void)
{
  EP0CS = HSNAK; //HSNACK dis
}

void usbDismissSetup(void)
{
  EP0CS = EP0STALL; //Stall to error
}
