Bootloader fix scripts
======================

This folder contains script to fix the Crazyradio bootloader via SPI in case
it becomes impossible to start the USB bootloader again (this can happen if you
flash a bugged firmware for example).

Look at the files to understand how to use the scripts. Generally you will need
to connect the Crazyradio to the device you use to flash.

These scripts will simply write "JUMP bootloader_address" at the beginning of
the flash which will force the dongle in bootloader mode next time you connect
it to the USB port.
