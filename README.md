# Bitcraze Crazyradio dongle [![CI](https://github.com/bitcraze/crazyradio-firmware/workflows/CI/badge.svg)](https://github.com/bitcraze/crazyradio-firmware/actions?query=workflow%3ACI)

Source code and tools for the Bitcraze Crazyradio USB dongle.

See [Bitcraze Documentation](https://www.bitcraze.io/documentation/repository/crazyradio-firmware/master/) for more information about
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
ln -s /usr/local/opt/binutils/bin/gobjcopy /usr/local/bin/objcopy
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

## Contribute
Go to the [contribute page](https://www.bitcraze.io/contribute/) on our website to learn more.

### Test code for contribution
Run the automated build locally to test your code

	./tools/build/build
