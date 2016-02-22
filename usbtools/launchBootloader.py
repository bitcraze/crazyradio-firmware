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
#  Crazyradio bootloader launcher
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
import sys
import time


def findDevice(vid, pid):
  busses = usb.busses()
  for bus in busses:
    for device in bus.devices:
      if device.idVendor == vid:
        if device.idProduct == pid:
          return device
  return None

def launchBootloader(verbose = False):
  dev= findDevice(0x1915, 0x7777)
  if dev==None:
    dev= findDevice(0x1915, 0x0101)
    if dev==None:
      if verbose:
        print("Error!, cannot find the CrazyRadio USB dongle!")
      return -1
    else:
      if verbose:
        print("Bootloader already launched.")
      return 0

  handle = dev.open()

  if verbose:
    sys.stdout.write("Launch bootloader ")
    sys.stdout.flush()

  #Send the command to arm the bootloader
  handle.controlMsg(0x40, 0xFF, (), value=0, index=0, timeout=100)

  #resets to bootloader (Can fail as the device will disapear)
  try:
    handle.reset()
  except usb.USBError:
    pass

  #Wait for the bootloader to appear...
  dev = None

  for i in range(0,4):
    if verbose:
      sys.stdout.write(".")
      sys.stdout.flush()
    time.sleep(0.5)
    dev = findDevice(0x1915, 0x0101)
    if dev!=None:
      break

  if verbose:
    print("")

  if dev==None:
    if verbose:
      print("Error!, bootloader not started")
    return -2

  if verbose:
    print("Bootloader started")

  return 0

if __name__=="__main__":
  sys.exit(launchBootloader(True))

