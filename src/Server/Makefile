SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .h .o
.EXPORT_ALL_VARIABLES:

CFLAGS+=-Wall -lpthread -lrt -static

LIBRARY_PATH_PREFIX = ../../lib

ZLOG = zlog
INIH = inih
XBEE = xbee

INCLUDE =  -I $(LIBRARY_PATH_PREFIX)/$(ZLOG)/src -I $(LIBRARY_PATH_PREFIX)/$(INIH) -I $(LIBRARY_PATH_PREFIX)/$(XBEE)
LD_LIBRARY_PATH = -L $(LIBRARY_PATH_PREFIX)/$(ZLOG)/src -L $(LIBRARY_PATH_PREFIX)/$(XBEE)/lib

LIBZLOG = $(LIBRARY_PATH_PREFIX)/$(ZLOG)/src/lib$(ZLOG).so
LIBXBEE = $(LIBRARY_PATH_PREFIX)/$(XBEE)/lib/lib$(XBEE).so

PROG = $(LIBRARY_PATH_PREFIX)/$(INIH)/ini.c SocketServerControl.c XBeeInterface.c SwitchControlApp.c Util.c CommandLogic.c RadioNetwork.c
OUT = SwitchControlApp


ifeq ($(TARGET), arm)
	CC := ~/bin/4.4.3/bin/arm-linux-gcc
	CROSS_COMPILE?= arm-none-linux-gnueabi-
	CFLAGS+=        -march=armv4t -mtune=arm920t
	CLINKS+=        -march=armv4t -mtune=arm920t
else
	CC := gcc
endif

all: $(OUT)	
	
new:
	clean all

clean:
	($(RM) -f $(OUT); cd $(LIBRARY_PATH_PREFIX)/$(ZLOG); $(MAKE) clean; cd $(LIBRARY_PATH_PREFIX)/$(XBEE); $(MAKE) clean)
	

LIBZLOG:
	(cd $(LIBRARY_PATH_PREFIX)/$(ZLOG); $(MAKE))
	
# this is a temporary solution, libxbee has its own way of configuring platform
# TODO: find a more conventional way to do this
LIBXBEE:
	(export PATH=/home/dreamer/bin/4.4.3/bin:$$PATH; cd $(LIBRARY_PATH_PREFIX)/$(XBEE); $(MAKE) configure; $(MAKE))
	
$(INIH).o:
	(cd $(LIBRARY_PATH_PREFIX)/$(INIH); $(CC) ini.c)

$(OUT): $(PROG) LIBZLOG LIBXBEE
	$(CC) $(filter %.c,$^) -o $(OUT) $(INCLUDE) $(LD_LIBRARY_PATH) -l$(ZLOG) -l$(XBEE) $(CFLAGS)

