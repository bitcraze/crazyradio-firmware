---
title: Installing USB driver on Windows
page_id: usbwindows
---

Most Bitcraze products using USB communication uses the libusb driver
for Windows. This driver allows to communicate with USB devices without
having to develop custom drivers. This page explains how to install the
libusb driver using the Zadig tool.

Install the driver
------------------

First of all download Zadig from its website: <http://zadig.akeo.ie/>.

Plug the Bitcraze device in the PC. Windows should display some
installation window. On windows 8 and 10 it does nothing and closes
itself. On windows 7 you have to close the install failed window.

![Windows 8 driver installation windows](/docs/images/01-win-install.png){:width="400"} ![Windows 7 install failed](/docs/images/crazyradio_windows_noinstall.png){:width="400"}

Once windows is out of the way, launch Zadig, you should see your device
in the list. Select the device, select **libusb** and click install:

![Zadig setup](/docs/images/02-zadig_setup.png){:width="400"}

The install process should run and succed. Installation is usually quick
but it could also take some time:

![zadig install](/docs/images/03-zadig_install.png)
![zadig done](/docs/images/04-zadig_done.png)

Et voilà! The driver is now installed and you can use your USB Bitcraze
device.

Reinstall driver
----------------

If the driver is wrongly installed and needs to be reinstalled or if
Windows already have another driver installed for this device, plug the
device, launch Zadig and click \"Option/show all devices\". You should
be able to choose the device in the list and reinstall the libusb
driver. For example with the [Crazyradio](https://wiki.bitcraze.io/projects:crazyradiopa:index).

![zadig reinstall](/docs/images/05-zadig_reinstall.png){:width="400"}


Troubleshooting
---------------

If things does not work check that the device appears as a libusb device
in the Windows device manager. If it is not the case try to reinstall
the driver as described above.

![crazyradio device manager](/docs/images/crazyradio-device_manager.png)

If you are having problems with the device not working all the time you
can try to install the libUSBK driver, it have been behaving a bit
better to some version of the Bitcraze softwares:

![zadig libusbk](/docs/images/zadig_libusbk.png){:width="400"}
