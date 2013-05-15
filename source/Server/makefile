SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .h .o

PCCC = gcc
ARMCC = arm-linux-gcc

PROG = SocketServerControl.c SwitchControlApp.c
OUT = SwitchControlApp

all: $(PROG)
	$(PCCC) -o $(OUT) $(PROG)
#	LD_LIBRARY_PATH=./libxbee/libxbee.libxbee-v3/lib ./$(PROG)
#	LD_LIBRARY_PATH=../../../lib ./$(PROG)
#	LD_LIBRARY_PATH=./libxbee-arm/libxbee.libxbee-v3/lib

new: clean all

clean:
	rm -f $(OUT)

#$(PROG): $(PROG).c ./libxbee/libxbee.libxbee-v3/lib/libxbee.so
#$(PROG): $(PROG).c ./libxbee-arm/libxbee.libxbee-v3/lib/libxbee.so
#	gcc $(filter %.c,$^) -g  -I ../../.. -L ../../../lib -lxbee -lpthread -lrt
#	arm-linux-gcc $(filter %.c,$^) -g -o $@ -I ./libxbee-arm/libxbee.libxbee-v3/ -L ./libxbee-arm/libxbee.libxbee-v3/lib -lxbee -lpthread -lrt
