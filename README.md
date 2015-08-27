# Bitcraze Crazyradio dongle [![Build Status](https://travis-ci.org/bitcraze/crazyradio-firmware.svg)](https://travis-ci.org/bitcraze/crazyradio-firmware)

Source code and tools for the Bitcraze Crazyradio USB dongle.

See [bitcraze wiki](http://wiki.bitcraze.se/projects:crazyradio:index) for more information about
Crazyradio and the USB protocol used.

## Folders content:
- firmware: The firmware source code
- nrfProg:  SPI programmer that uses jtagkey USB addapter
- usb_tools: Python scripts to reset and bootload Crazyradio from command line
- lib: Software libraries to use Crazyradio with Pyton

## Building the firmware
The development environment is described [here](https://wiki.bitcraze.io/projects:crazyradio:dev_env).
To build the firmware go into the firmware directory and make

### Build for Crazyradio
```
make
```
### Build for Crazyradio PA
```
make CRPA=1
```
