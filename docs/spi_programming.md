---
title: Crazyradio SPI programming
page_id: spi_programming 
---


nrfProg
-------

nrfProg is a command line programmer for the nRF24LU1p (called nRF in
this page). It uses the Amontec JTAG Key jtag dongle as a USB to SPI
dongle.

This program is using libftdi to communicate with the JTAG Key. It is
tested only on Linux.

### Cable

The cable is made as follow:

|  **JTAG sig**  | **FTDI sig**  | **JTAG Key pin**   | **nRF Breakout pin**  | **nRF SPI sig** |
|  --------------| --------------| ------------------ | ---------------------- |----------------|
|  GND           | N/A           | GND/20            | GND                   | GND|
|  VREF          | GPIOL1/19     | VREF/1            | VIN                   | VCC|
|  TCK           | TCK/24        | TCK/9             | P0.0                  | SSCK|
|  TMS           | TMS/21        | TMS/7             | P0.3                  | SCSN|
|  TDI           | TDI/23        | TDI/5             | P0.1                  | SMOSI|
|  TDO           | TDO/22        | TDO/13            | P0.2                  | SMISO|
|  TRST          | GPIOH0/15     | TRST\_N/3         | PROG                  | PROG|
|  SRST          | GPIOH1/13     | SRST\_N/15        | RESET                 | RESET|

This cabling permit to reset the nRF in program mode or in run mode from
nrfProg. The SPI outputs are not activated when the nRF is in run mode.

### Cable for USB Radio dongle Rev.B

|  **Amontec JTAG pin nr**  | **Radio pin nr**|
|  -------------------------| ------------------|
|  20                       | 9|
|  1                        | 5|
|  9                        | 4|
|  7                        | 10|
|  5                        | 6|
|  13                       | 8|
|  3                        | 2|
|  15                       | 3|

### Usage

nrfProg can program, dump and display the content of the flash memory.
It can also reset the nRF and halt it (by maintaining the reset active).

Launch the program without argument for help.
```
$ ./nrfProg
nrfProg V0.01 - nRF24LU1 programmer using the Amontec JTAG Key

Usage: ./nrfProg <COMMAND> <PARAMETERS>
Commands:
help:
  Display this message
read <filename> [addr] [len]:
  read the memory and write a dump in filename
write <filename> [addr]:
  write the content of the binary <filename> into the nRF24LU flash.
hexdump [addr] [len]:
  Display a hex dump of the memory
reset:
  Reset the nRF24LU1+
holdreset:
  Hold the reset (ie. stop) the nRF24LU1+
```

Using BusPirate
-------

---

**WARNING** This script will mass erase the
nRF24 chip so it will erase the bootloader. Make sure you have a backup
of the bootloader before using this script.

Unless you are absolutely sure of what you are doing, **do not use this
script**. 

---

It\'s possible to re-program the Crazyradio using a BusPirate and [this
script](https://github.com/koolatron/buspirate_nrf24lu1p) via SPI.

Couple of caveats:

-   Tested only on OS X. Should work on Linux without modification, and
    Windows with very minor changes to use the windows serial module.
-   It\'s very slow (\~5 minutes to flash the entire .bin file). I
    deemed this acceptable as this script is for emergency recovery
    only. I can make it faster if necessary.

Prerequisites:

-   A Bus Pirate (you should know where to get one of these, if you
    don\'t already have one).
-   perl and either Device::SerialPort (\*nix) or Win32::SerialPort
    (Windows)
-   Some jumper wires to connect the SPI lines on the radio to the ones
    on the Bus Pirate.

Instructions:

1.  Solder a 2x5 pin header onto the programming port of the crazyradio.
    There\'s an unpopulated footprint already there for you.
2.  Connect the crazyradio to your Bus Pirate using the table below
    (also noted in the script and readme on git)

```
    Bus Pirate       CrazyRadio
    ===========================
    MOSI ()      ->  MOSI  (6)
    MISO ()      ->  MISO  (8)
    SCK  ()      ->  SCK   (4)
    CS   ()      ->  CS    (10)
    AUX  ()      ->  PROG  (2)
    3V3  ()      ->  3V3   (5)
    GND  ()      ->  GND   (9)
```

3.  Run the script: perl ./flasher.pl -input ./cradio-0.51.bin -device
    \[serial device\]
4.  Wait till you see lots of hex addresses crawling up your screen.
    Your device is programming.
5.  Go make a sandwich or have a beer (or both).
