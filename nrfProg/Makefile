
LDFLAGS=-lftdi

all: nrfProg

nrfProg: nrfProg.o nrfSpi.o commands.o
	cc -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o nrfProg *~
