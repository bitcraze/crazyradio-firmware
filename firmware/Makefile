
# Build configuration
CRPA ?= 1

#Compiler
SDCC ?= sdcc
SDAS8051 ?= sdas8051

NRFPROG ?= ../nrfProg/nrfProg

VERIFIED ?= 0

CFLAGS = -Iinc/ --model-small --std-sdcc99
LDFLAGS = --xram-loc 0x8000 --xram-size 2048 --model-small

VPATH += src/ bin/

OBJS = main.rel radio.rel usb.rel usbDescriptor.rel led.rel utils.rel crtxinit.rel

ifdef PPM_JOYSTICK
ifneq ($(PPM_JOYSTICK), 0)
  OBJS += ppm.rel
  CFLAGS += -DPPM_JOYSTICK
endif
endif

ifeq ($(strip $(CRPA)),1)
  CFLAGS += -DCRPA
endif

.PHONY: all cri

all: cradio.bin
ifeq ($(strip $(CRPA)),1)
	@echo Crazyradio PA build
else
	@echo Original Crazyradio build
endif

cri: cradio.cri

# Main targets
cradio.bin: $(OBJS)
	$(SDCC) $(LDFLAGS) $(OBJS:%=bin/%) -o bin/cradio.ihx
	objcopy -I ihex bin/cradio.ihx -O binary bin/cradio.bin

#Crazyradio firmware image
bin/cradio.cri: bin/cradio.bin
	python2 ../usbtools/generateCri.py bin/cradio.bin bin/cradio.cri $(VERIFIED)

clean:
	rm -f bin/*
	rm -f TAGS

tags:
	etags ./src/*.c ./inc/*.h

mrproper: clean
	rm -f src/*~ inc/*~ *~

# CPU control targets
flash: all
	$(NRFPROG) write cradio.bin

halt:
	$(NRFPROG) holdreset

reset:
	$(NRFPROG) reset

# General targets
%.rel: %.c
	$(SDCC) $(CFLAGS) -c $< -o bin/$@

%.rel: %.asm
	$(SDAS8051) -plosgff -o bin/$@ $<

bin/:
	mkdir -p bin
