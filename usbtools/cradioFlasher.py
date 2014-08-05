#!/usr/bin/env python2
# -*- coding: utf-8 -*-

from PyQt4 import QtCore, QtGui, uic
import NrfUsbBootloader
import launchBootloader
from zipfile import ZipFile
import os
import json
import hashlib
import usb

import logging

logger = logging.getLogger(__name__)

# Page0 to be written first to make the flash process safer
page0 = (0x02, 0x78, 0x00)  #LJMP 0x7800  ; Jump to the bootloader

class CradioFlasher(QtGui.QMainWindow):
    def __init__(self):
        QtGui.QMainWindow.__init__(self)
        uic.loadUi('cradioFlasher.ui', self)
        
        self.browseButton.clicked.connect(self.browse)
        self.flashButton.clicked.connect(self.flash)
        self.fileLine.returnPressed.connect(self.file_selected)
        
		# Log messages are printed in the GUI textbox
        logger.addHandler(logging.StreamHandler(self))
        logger.setLevel(logging.INFO)
        
        self.image_file = None
        self.image_ok = False        # Image file exists
        self.image_checked = False   # Image verified by a checksum
        self.image_verified = False  # Image signed as being verified by a developper
        
        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.scann_device)
        self.timer.start(1000)
        self.scann_device()
        self.analyze_image()

    def browse(self):
        self.image_file = str(QtGui.QFileDialog.getOpenFileName(self, "Crazyradio flasher"))
        self.fileLine.setText(self.image_file)
        
        self.analyze_image()

    def file_selected(self):
        self.image_file = str(self.fileLine.text())
        
        self.analyze_image()
        
        self.flashButton.setEnabled(self.image_ok and self.device_ok)
    
    def analyze_image(self):
        """ Analyze image file to verify that this is a verified 
        Crazyradio images"""
        if not self.image_file:
            self.image_ok = False
            self.image_checked = False
            self.image_verified = False
            self.image = None
            self.imageIdLine.setText("<div style='color:red'>None</div>")
            return
        
        try:
            cri = ZipFile(self.image_file)
        except:
            self.analyze_binary()
            return
        
        try:
            image = cri.read("image.bin")
            manifest = json.loads(cri.read("manifest.json"))
            
            hcri = hashlib.new(cri.read("hash").split(':')[0])
            hcri.update(image)
            hcri.update(cri.read("manifest.json"));
            
            if hcri.hexdigest() != cri.read("hash").split(':')[1].strip():
                logger.error("Damaged CRI image!")
                self.image_ok = False
                self.image_checked = False
                self.image_verified = False
                self.image = None
                self.imageIdLine.setText("<div style='color:red'>None</div>")
                return
            
            logger.info("Loaded image size {} bytes".format(len(image)))
            if manifest["tag"] != "" and manifest["tag"] != "tip":
                logger.info("Firmware version: {}".format(manifest['tag']))
                version = manifest['tag']
            else:
                version = manifest['version']
            logger.info("Firmware commit: {}".format(manifest['version']))
            logger.info("Verified image: {}".format(manifest['verified']))
            
            self.imageIdLine.setText("Crazyradio {}".format(version))
            self.image_ok = True
            self.image_checked = True
            self.image_verified = manifest['verified']
            self.image = image
            logger.info("{} Loaded ".format(os.path.basename(self.image_file)))
        except Exception as e:
            self.image_ok = False
            self.image_checked = False
            self.image_verified = False
            self.image = None
            logger.error("Error while checking the image: {}".format(e))
    
    def analyze_binary(self):
        self.image_ok = False
        self.image_checked = False
        self.image_verified = False
        self.image = None
        
        try:
            with open(self.image_file) as bfile:
                data = bfile.read()
            
            logger.info("Image size: {}".format(len(data)))
            
            if bytearray(data)[0] != 0x02:  #MSC51 jump
                raise Exception("This does not look like a 8051 image, should start by 0x02")
            
            self.image = data
            self.image_ok = True
            self.imageIdLine.setText("8051 binary image")
            logger.info("Loaded binary image.")
        except Exception as e:
            logger.error("Error when opening binary image file: {}".format(e))

    def flash(self):
        if not self.image_ok:
            logger.error("Image not verified, not flashing!")
            return
        
        if not self.image_checked or not self.image_verified:
            reply = QtGui.QMessageBox.question(self, 'Crazyradio image not verified',
                      "This image is not verified. If it is not able to retart "
                      "the bootloader it might require an SPI programmer to "
                      "reflash the radio.\nDo you want to flash anyway?",
                      QtGui.QMessageBox.No, QtGui.QMessageBox.Yes)
            if reply == QtGui.QMessageBox.No:
                logger.error("Aborting flash operation.")
                return
        
        bootloader = False
        try:
            self.setEnabled(False)
            
            logger.info("")
            logger.info("Starting to flash...")
            
            ret = launchBootloader.launchBootloader(True)
            if ret:
                logger.error("Error!, cannot start the bootloader!")
                raise Exception()
            
            bootloader = NrfUsbBootloader.Bootloader()
            if not bootloader.open():
                logger.error("Error! Cannot open bootloader.")
                raise Exception()
            
            logger.info("Bootloader version {}".format(bootloader.getVersion()))
            
            logger.info("Flashing safe page0 (goto bootloader)...")
            bootloader.write(page0, 0)
            
            logger.info("Flashing new firmware but page 0...")
            bootloader.write(self.image[NrfUsbBootloader.PAGE_SIZE:],
                             NrfUsbBootloader.PAGE_SIZE)
            
            logger.info("Verifying new firmware (but page 0)...")
            readback = bootloader.read(NrfUsbBootloader.PAGE_SIZE,
                        len(self.image)-NrfUsbBootloader.PAGE_SIZE)
            for i in range(len(self.image)-NrfUsbBootloader.PAGE_SIZE):
                if bytearray(self.image)[i+NrfUsbBootloader.PAGE_SIZE] != readback[i]:
                    raise Exception("Verification failed at position {}! {}!={}".format(
                            i, bytearray(self.image)[i+NrfUsbBootloader.PAGE_SIZE], readback[i]))

            logger.info("{} bytes verified!".format(len(self.image)-NrfUsbBootloader.PAGE_SIZE))
            
            logger.info("Flashing page0...")
            bootloader.write(self.image[:NrfUsbBootloader.PAGE_SIZE], 0)
            
            logger.info("Verifying full firmware...")
            readback = bootloader.read(0, len(self.image))
            for i in range(len(self.image)):
                if bytearray(self.image)[i] != readback[i]:
                    raise Exception("Verification failed at position {}! {}!={}".format(
                            i, bytearray(self.image)[i], readback[i]))
            
            logger.info("{} bytes verified!".format(len(self.image)))
            
            logger.info("Done! You can remove and re-insert the dongle to start"
                  " the new firmware.")
            
        finally:
            if bootloader:
                bootloader.close()
            self.setEnabled(True)
    
    def scann_device(self):
        dev = launchBootloader.findDevice(0x1915, 0x7777)
        if (dev):
            if type(dev.deviceVersion) == str:
            	version = dev.deviceVersion
            else:
            	version = "{:03x}".format(dev.deviceVersion)
            	version = "{}.{}".format(version[:-2], version[-2:])
            self.deviceLine.setText("{} v{}".format(dev.open().getString(dev.iProduct, 255), version))
            self.device_ok = True
        elif launchBootloader.findDevice(0x1915, 0x0101):
            self.deviceLine.setText("Nordic semiconductor bootloader")
            self.device_ok = True
            self.flashButton.setEnabled(True)
        else:
            self.deviceLine.setText("None")
            self.device_ok = False

        self.flashButton.setEnabled(self.device_ok and self.image_ok)
    
    def write(self, msg):
        """Function called for log output"""
        self.logText.setPlainText(self.logText.toPlainText()+msg)
        sb = self.logText.verticalScrollBar()
        sb.setValue(sb.maximum())
        app.processEvents() #Make sure the GUI is updated during long operations
    
    def flush(self):
        pass

if __name__ == "__main__":
    import sys
    
    # When launched in standalone, also output log in stdout
    logger.addHandler(logging.StreamHandler(sys.stdout))
    
    app = QtGui.QApplication(sys.argv)
    ui = CradioFlasher()
    ui.show()
    sys.exit(app.exec_())

