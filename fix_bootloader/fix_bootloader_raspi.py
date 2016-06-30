#!/usr/bin/env python3

# This script should work on any Raspberry pi running a recent version of
# rasbian. After the Crazyradio is connected to the raspberry pi expansion
# port just run the script with "python3 fix_bootloader_raspi.py"
# See readme for more information about this script purpose.

# Script setup. Use this as a documentation to cable the Crazyradio or modify
# if you want to cable it differently.
# Name   Pin on raspi     Pin on Crazyradio
#-------------------------------------------
GND   =   6              # 9
RESET =   3              # 3
PROG  =   5              # 2
SCK   =   7              # 4
MOSI  =   8              # 6
MISO  =   10             # 8
CS    =   12             # 10

import RPi.GPIO as GPIO
import time

CS_ENABLE = GPIO.LOW
CS_DISABLE = GPIO.HIGH

# SPI commands
WREN = 0x06
WRDIS = 0x04
ERASE_PAGE = 0x52
PROGRAM = 0x02
READ = 0x03


def init_gpios():
    GPIO.setmode(GPIO.BOARD)
    GPIO.setup(RESET, GPIO.OUT, initial=GPIO.LOW)
    GPIO.setup(PROG, GPIO.OUT, initial=GPIO.HIGH)


def check_connection():
    # If reset is connected, it will be pulled up
    return GPIO.input(RESET) != 0


def reset_in_prog():
    GPIO.output(RESET, GPIO.LOW)
    GPIO.output(PROG, GPIO.HIGH)
    time.sleep(0.1)
    GPIO.output(RESET, GPIO.HIGH)
    time.sleep(0.1)


def reset_in_fw():
    GPIO.output(RESET, GPIO.LOW)
    GPIO.output(PROG, GPIO.LOW)
    time.sleep(0.1)
    GPIO.output(RESET, GPIO.HIGH)
    time.sleep(0.1)


def spi_oe(enable):
    if enable:
        GPIO.setup([SCK, MOSI], GPIO.OUT, initial=GPIO.LOW)
        GPIO.setup(CS, GPIO.OUT, initial=CS_DISABLE)
        GPIO.setup(MISO, GPIO.IN)
    else:
        GPIO.setup([SCK, MOSI, CS], GPIO.IN)


def set_cs(value):
    GPIO.output(CS, value)
    time.sleep(0.1)


def spi_transfer(dataout):
    datain = 0
    GPIO.output(SCK, GPIO.LOW)
    for i in range(8):
        b = GPIO.LOW
        if dataout & 0x0080 != 0:
            b = GPIO.HIGH
        GPIO.output(MOSI, b)
        dataout = dataout << 1

        time.sleep(0.001)
        b = GPIO.input(MISO)
        GPIO.output(SCK, GPIO.HIGH)

        datain = datain << 1
        if b != GPIO.LOW:
            datain = datain | 0x01

        time.sleep(0.001)
        GPIO.output(SCK, GPIO.LOW)

    return datain

if __name__ == "__main__":
    init_gpios()

    reset_in_prog()
    spi_oe(True)

    print("Erasing page 0...")
    set_cs(CS_ENABLE)
    spi_transfer(WREN)
    set_cs(CS_DISABLE)

    set_cs(CS_ENABLE)
    spi_transfer(ERASE_PAGE)
    spi_transfer(0x00)
    set_cs(CS_DISABLE)

    time.sleep(0.1)

    set_cs(CS_ENABLE)
    spi_transfer(WREN)
    set_cs(CS_DISABLE)

    print("Writing 'JL 0x7800' instruction...")
    set_cs(CS_ENABLE)
    spi_transfer(PROGRAM)
    # Address 0
    spi_transfer(0x00)
    spi_transfer(0x00)
    # Jump to bootloader
    spi_transfer(0x02)
    spi_transfer(0x78)
    spi_transfer(0x00)
    set_cs(CS_DISABLE)

    # Wait for page to be written
    time.sleep(0.1)

    print("Flash content:")
    set_cs(CS_ENABLE)
    spi_transfer(READ)
    spi_transfer(0x00)
    spi_transfer(0x00)
    print("{:02x}".format(spi_transfer(0x00)))
    print("{:02x}".format(spi_transfer(0x00)))
    print("{:02x}".format(spi_transfer(0x00)))
    set_cs(CS_DISABLE)

    spi_oe(False)

    reset_in_fw()

    GPIO.cleanup()

    print("Fix written to flash. Disconnect the PROG wire, reconnect Crazyradio")
    print("and the bootloader should start!")
