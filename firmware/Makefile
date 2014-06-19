
#Compiler
SDCC ?= sdcc

NRFPROG ?= ../nrfProg/nrfProg

VERIFIED ?= 0

CFLAGS = -Iinc/ --model-large --std-sdcc99
LDFLAGS = --xram-loc 0x8000 --xram-size 2048 --model-large

VPATH += src/

OBJS = main.rel radio.rel usb.rel usbDescriptor.rel led.rel utils.rel

ifdef PPM_JOYSTICK
ifneq ($(PPM_JOYSTICK), 0)
  OBJS += ppm.rel
  CFLAGS += -DPPM_JOYSTICK
endif
endif

.PHONY: all cri

all: bin/cradio.bin
cri: bin/cradio.cri

# Main targets
bin/cradio.bin: $(OBJS:%=bin/%)
	$(SDCC) $(LDFLAGS) $^ -o bin/cradio.ihx
	objcopy -I ihex bin/cradio.ihx -O binary bin/cradio.bin

#Crazyradio firmware image
bin/cradio.cri: bin/cradio.bin
	python2 ../usbtools/generateCri.py bin/cradio.bin bin/cradio.cri $(VERIFIED)

clean:
	rm -f bin/*

mrproper: clean
	rm -f src/*~ inc/*~ *~

# CPU control targets
flash: all
	$(NRFPROG) write bin/cradio.bin

halt:
	$(NRFPROG) holdreset

reset:
	$(NRFPROG) reset

# General targets
bin/%.rel : %.c
	$(SDCC) $(CFLAGS) -c $< -o $@
