#!/usr/bin/env python
#
#     ||          ____  _ __                           
#  +------+      / __ )(_) /_______________ _____  ___ 
#  | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
#  +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
#   ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
#
#  Copyright (C) 2011-2013 Bitcraze AB
#
#  nRF24LU1 USB bootloader driver module
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

import usb

# Bootloader commands
CMD_GETVER  = 0x01
CMD_READ    = 0x03
CMD_SETHALF = 0x06
CMD_PAGE_ERASE = 0x04
CMD_WRITE_INIT = 0x02

#The flash is erased/witten by page and read by block
PAGE_SIZE = 512
BLOCK_SIZE = 64

class Bootloader:
    @classmethod
    def findDevice(self, vid, pid):
      busses = usb.busses()
      for bus in busses:
        for device in bus.devices:
          if device.idVendor == vid:
            if device.idProduct == pid:
              return device
      return None   
    
    def __init__(self):
        self.handle = None
    
    def isOpen(self):
        if self.handle != None:
            return True
        return False
    
    def open(self):
        dev = self.findDevice(0x1915, 0x0101)
        if dev != None:
            self.handle = dev.open()
            self.handle.claimInterface(0)
            return True
        return False
    
    def close(self):
        if self.handle:
            self.handle.releaseInterface();
    
    def getVersion(self):
        self.handle.bulkWrite(0x01, (CMD_GETVER,), 1000)
        data = self.handle.bulkRead(0x81, 64,1000)
        
        return "%d.%d" % (data[0], data[1])
    
    def readBlocks(self, startBlock, nBlock):
        data = tuple()
        
        self.handle.bulkWrite(0x01, (CMD_SETHALF, startBlock/256), 1000)
        self.handle.bulkRead(0x81, 64, 1000)
        
        for i in range(startBlock, startBlock+nBlock):
            if i%256 == 0:
                self.handle.bulkWrite(0x01, (CMD_SETHALF, (i/256)%2), 1000)
                self.handle.bulkRead(0x81, 64, 1000)
            self.handle.bulkWrite(0x01, (CMD_READ, i%256), 1000)
            data += tuple(self.handle.bulkRead(0x81, 64, 1000))
        
        return data
    
    #This is the easiest and dumbest possible implementation!
    def read(self, address=0, length=32*1024):
        
        startBlock = address/BLOCK_SIZE
        nBlock = ((length-1)/BLOCK_SIZE)+1
        
        data = self.readBlocks(startBlock, nBlock)
        
        return bytearray(data[(address%BLOCK_SIZE):((address%BLOCK_SIZE)+length)])

    def writePage(self, data, pageNum):
        data = tuple(data)
        if len(data)!=PAGE_SIZE:
            return False
        
        self.handle.bulkWrite(0x01, (CMD_PAGE_ERASE, pageNum), 1000)
        self.handle.bulkRead(0x81, 64, 1000)

        self.handle.bulkWrite(0x01, (CMD_WRITE_INIT, pageNum), 1000)
        self.handle.bulkRead(0x81, 64, 1000)
        for i in range(0, PAGE_SIZE, 64):
            self.handle.bulkWrite(0x01, data[i:(i+64)], 1000)
            self.handle.bulkRead(0x81, 64, 1000)
        
        return True

    def write(self, data, address=0):
        """Write to the nrf24lu1 flash. This function will erase and fill with
           0xff all pages that contains the data but not filled with it (ie.
           before and after the data if not on page boundary)
        """
        if len(data)>32*1024:
            raise Exception("Data to be written too big!")
        
        
        data = tuple(bytearray(data))
        
        #align data on a page bounday
        while address%PAGE_SIZE != 0:
            data = (0xff,) + data
            address = address-1
        
        #make the data length a multiple of the page size
        while len(data)%PAGE_SIZE != 0:
            data = data+(0xff, )
        
        for i in range(0, len(data), PAGE_SIZE):
            if not self.writePage(data[i:(i+PAGE_SIZE)], (address+i)/PAGE_SIZE):
                return False
        
        return True
