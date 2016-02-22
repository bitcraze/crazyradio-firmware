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
#  nRF24LU1 USB bootloader command line tool
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

import NrfUsbBootloader;
import sys

actions = ("dump", "hexdump", "flash", "verify")

help = """nRF24LU1(+) Bootloader client.
Copyright (c) 2012 Bitcraze AB
This program is distributed under the GNU GPL license V3.

Usage:
    %s <action [filename [args...]]>
Possible actions:
    dump <filename> [address [length]]:
        Dump (read) the flash and write the binary data in <filename>. Address 
        and length are optional, by default 32K are read from the address 0x0000
    hexdump [address] length]]:
        Dump the flash and display its hexadecimal content. Address and length
        are handled the same way as for the Dump command.
    flash <filename>:
        Flash and verify the data contained in <filename> from the beginning of 
        the flash.
    verify <filename>:
        Read the flash and verify that the data are equal to those contained in
        <filename>"""

if len(sys.argv)<2:
    print(help % sys.argv[0])
    sys.exit(0)

action = sys.argv[1]
nargs = len(sys.argv)-2
args = sys.argv[2:]

try:
    actions.index(action)
except ValueError:
    print("Incorrect action %s! " % action)
    print("Launch %s without argument to get some help." % sys.argv[0])
    sys.exit(-1)

bl = NrfUsbBootloader.Bootloader()

if not bl.open():
    print("Error: cannot found the nRF24LU1 bootloader!")
    sys.exit(-2)

print("Found nRF24LU1 bootloader version", bl.getVersion())

try:
    if action=="dump":
        address=0
        length=32*1024
        
        if nargs>0:
            filename = args[0]
        else:
            print("Error: A file name must be specified")
            raise Exception()
        
        if nargs>1:
            address = int(eval(args[1]))
        if nargs>2:
            length = int(eval(args[2]))
        
        print("Reading %d bytes from address 0x%04X" % (length, address))
        flash = bl.read(address, length)
        
        f = open(filename, "wb")
        f.write(flash)
        f.close()
        
        print("%d bytes written in %s." % (length, filename))
        
    elif action=="hexdump":
        address=0
        length=32*1024
        
        if nargs>0:
            address = int(eval(args[0]))
        if nargs>1:
            length = int(eval(args[1]))    

        print("Reading %d bytes of the flash from address 0x%04X" % (length, address))
        flash = bl.read(address, length)
        
        for i in range(0,len(flash)):
            if i%16 == 0:
                if i!=0:
                    print()
                sys.stdout.write("0x%04X: " % (address+i))
            if i%8==0 and i%16!=0:
                sys.stdout.write(" ")
            sys.stdout.write("%02X"%flash[i])
        
        print()
    if action=="flash":
        print("Flashing:")
        if nargs>0:
            filename = args[0]
        else:
            print("  Error: A file name must be specified")
            raise Exception()
        
        f=open(filename, "rb")
        fileData = bytearray(f.read())
        f.close()
        
        print("  Flashing %d bytes..." % len(fileData))
        if bl.write(fileData):
            print("Flashing done!")
        else:
            print("Flashing fail!")
            raise Exception
    if action=="verify" or action=="flash":
    
        print("Verifying:")
        if nargs>0:
            filename = args[0]
        else:
            print("  Error: A file name must be specified")
            raise Exception()
        
        print("  Reading %s..." % filename)
        f=open(filename, "rb")
        fileData = bytearray(f.read())
        f.close()
        
        print("  Reading %d bytes from the flash..." % len(fileData))
        flashData = bl.read(0, len(fileData))
        
        fail = False
        for i in range(0, len(fileData)):
            if fileData[i] != flashData[i]:
                print("Verification failed at byte %d (expected 0x%02X, read 0x%02X)!"%(i, fileData[i], flashData[i]))
                fail=True
                break
        
        if not fail:
            print("Verification succeded!")
        
#except Exception:
#    pass
finally:
    bl.close()

