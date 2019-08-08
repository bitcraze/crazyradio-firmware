---
title: Build and flash instructions
page_id: building_flashing 
---

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
make CRPA=0
```
### Build for Crazyradio PA
```
make CRPA=1
```

## Flashing the Firmware

To flash the firmware run the following from the firmware directory:

```
python ../usbtools/launchBootloader.py
sudo python ../usbtools/nrfbootload.py flash bin/cradio.bin
```

After flashing successfully, you need to replug the Crazyradio.
More details are in the [wiki](https://wiki.bitcraze.io/projects:crazyradio:programming).

## Check your firmware version
Run the following command:

```
lsusb -d 1915:7777 -v | grep bcdDevice
```
