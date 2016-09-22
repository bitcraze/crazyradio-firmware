# Bitcraze Crazyradio dongle [![Build Status](https://travis-ci.org/bitcraze/crazyradio-firmware.svg)](https://travis-ci.org/bitcraze/crazyradio-firmware)

Source code and tools for the Bitcraze Crazyradio USB dongle.

See [bitcraze wiki](http://wiki.bitcraze.se/projects:crazyradio:index) for more information about
Crazyradio and the USB protocol used.

## Folders content:
- firmware: The firmware source code
- nrfProg:  SPI programmer that uses jtagkey USB adapter
- usbtools: Python scripts to reset and bootload Crazyradio from command line
- lib: Software libraries to use Crazyradio with Python
- fix_bootloader: Script to revive a Crazyradio

## Building the firmware
Requirement:
  - SDCC
  - Binutils (needs objcopy)

On Ubuntu this can be installed with:
```
sudo apt-get install sdcc binutils
```

On Mac (using homebrew):
```
brew install sdcc
brew install binutils
ln -s /usr/local/bin/gobjcopy /usr/local/bin/objcopy
```

To build the firmware you should navigate to the firmware directory.

### Build for Crazyradio
```
make
```
### Build for Crazyradio PA
```
make CRPA=1
```

## Flashing the Firmware

To flash the firmware run the following from the firmware directory:

```
python ../usbtools/launchBootloader.py
python ../usbtools/nrfbootload.py flash bin/cradio.bin
```

After flashing successfully, you need to replug the Crazyradio.
More details are in the [wiki](https://wiki.bitcraze.io/projects:crazyradio:programming).
