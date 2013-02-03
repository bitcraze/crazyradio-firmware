
#Compiler
SDCC ?= sdcc

NRFPROG ?= ../nrfProg/nrfProg

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

all: bin/ cradio.bin

# Main targets
cradio.bin: $(OBJS)
	$(SDCC) $(LDFLAGS) $(foreach rel,$^,bin/$(rel)) -o bin/cradio.ihx
	objcopy -I ihex bin/cradio.ihx -O binary bin/cradio.bin

clean:
	rm -f bin/*

mrproper: clean
	rmdir bin
	rm -f src/*~ inc/*~ *~

# CPU control targets
flash: all
	$(NRFPROG) write bin/cradio.bin

halt:
	$(NRFPROG) holdreset

reset:
	$(NRFPROG) reset

# General targets
%.rel : %.c
	$(SDCC) $(CFLAGS) -c $< -o bin/$@

bin/:
	mkdir -p bin


