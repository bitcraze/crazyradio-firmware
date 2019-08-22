---
title: Crazyradio firmware upgrade
page_id: programming 
---

**Please note: The current implementation of the bootloader/scripts are
not 100% safe. If you upgrade you firmware, make sure that the script
says \"Verification succeded!\" before you unplug the dongle. Other wise
you risk \"bricking\" it (and then it has to be re-programmed via
SPI).**

**Do not do development and flash you own firmware on the dongle without
having access to hardware that makes SPI programming possible. If
firmware that doesn\'t work is flashed on the dongle you will need to
re-program it using SPI**

The Crazyradio contains a USB bootloader that gives the possibility to
upgrade the firmware without any external hardware (see warnings above).

USB bootloader (command line instructions)
------------------------------------------

Please note that you might have to exchange *python* with *python2* if
you distro uses python3.

First Crazyradio has to be rebooted in USB bootloader mode. To do so
insert the dongle in the pc, open a terminal window and run the
bootloader launcher:

    > cd crazyradio-firmware
    > python usbtools/launchBootloader.py 
    Launch bootloader .
    Bootloader started

After running this tool the Crazyradio dongle should have disappeared
and a new device named **nRF24LU1P-F32 BOOT LDR** should appear.

To flash the firmware use the nrfbootload.py script:

    > cd crazyradio-firmware
    > python usbtools/nrfbootload.py flash cradio-0.53.bin
    Found nRF24LU1 bootloader version 18.0
    Flashing:
      Flashing 5810 bytes...
    Flashing done!
    Verifying:
      Reading cradio-pa-0.53.bin...
      Reading 5810 bytes from the flash...
    Verification succeded!

Step-by-step upgrade using the Bitcraze VM
------------------------------------------

Below is a short guide with step-by-step instructions on how to upgrade
the Crazyradio with the latest firmware using the [Bitcraze
VM](https://wiki.bitcraze.io/projects:virtualmachine:index). There are two different firmwares
for the Crazyradio, one for the first generation Crazyradio and one for
the Crazyradio PA.

-   Start up the VM and download the latest firmware from
    [GitHub](https://github.com/bitcraze/crazyradio-firmware/releases).
    Place the firmware in
    **/home/bitcraze/projects/crazyradio-firmware**
-   Open up a terminal
-   Insert the Crazyradio and pass it to the VM
-   Run the following commands:
-   `> cd ~/projects/crazyradio-firmware
    > python usbtools/launchBootloader.py 
    Launch bootloader .
    Bootloader started
    `
-   Once the bootloader is started the Crazyradio dongle will dissapear
    ad a new device named **Nordic Semiconductor nRF24LU1P-F32 BOOT
    LDR** will appear. If you do not have a filter set up for the
    bootloader in the VM then script will say that it failed. This is
    ok. Pass the bootloader device into the VM and continue.
-   Run the following command:
-   `> python usbtools/nrfbootload.py flash cradio-0.53.bin
    Found nRF24LU1 bootloader version 18.0
    Flashing:
      Flashing 5810 bytes...
    Flashing done!
    Verifying:
      Reading cradio-pa-0.53.bin...
      Reading 5810 bytes from the flash...
    Verification succeded!
    `
-   If you see **Verification succeded** then unplug the dongle and
    re-insert it again. If you do not see this message, but instead some
    error. Do **NOT** unplug the dongle, try to reflash the firmware
    instead.

Check your firmware version
---------------------------

Run the following command:

    > lsusb -d 1915:7777 -v | grep bcdDevice

This should return the version (e.g. \"0.52\").
