SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .h .o

#export PATH!=/home/dreamer/bin/4.4.3/bin:$(PATH)

CC = gcc
#CC = ~/bin/4.4.3/bin/arm-linux-gcc


LIBRARY_PATH_PREFIX = ../../lib

ZLOG = zlog
INIH = inih
XBEE = xbee

INCLUDE =  -I $(LIBRARY_PATH_PREFIX)/$(ZLOG)/src -I $(LIBRARY_PATH_PREFIX)/$(INIH) -I $(LIBRARY_PATH_PREFIX)/$(XBEE)
LD_LIBRARY_PATH = -L $(LIBRARY_PATH_PREFIX)/$(ZLOG)/src -L $(LIBRARY_PATH_PREFIX)/$(XBEE)/lib

LIBZLOG = $(LIBRARY_PATH_PREFIX)/$(ZLOG)/src/lib$(ZLOG).so
LIBXBEE = $(LIBRARY_PATH_PREFIX)/$(XBEE)/lib/lib$(XBEE).so

PROG = $(LIBRARY_PATH_PREFIX)/$(INIH)/ini.c SocketServerControl.c XBeeInterface.c SwitchControlApp.c Util.c
OUT = SwitchControlApp


all: $(OUT)
	

new:
	clean all

clean:
	(rm -f $(OUT); cd $(LIBRARY_PATH_PREFIX)/$(ZLOG); $(MAKE) clean; cd $(LIBRARY_PATH_PREFIX)/$(XBEE); $(MAKE) clean)

LIBZLOG:
	(cd $(LIBRARY_PATH_PREFIX)/$(ZLOG); $(MAKE) CC=$(CC))
	
# this is a temporary solution, libxbee has its own way of configuring platform
# TODO: find a more conventional way to do this
LIBXBEE:
	(export PATH=/home/dreamer/bin/4.4.3/bin:$$PATH; cd $(LIBRARY_PATH_PREFIX)/$(XBEE); $(MAKE) configure; $(MAKE) CC=$(CC))

	
$(INIH).o:
	(cd $(LIBRARY_PATH_PREFIX)/$(INIH); $(CC) ini.c)

$(OUT): $(PROG) LIBZLOG LIBXBEE
	$(CC) $(filter %.c,$^) -o $(OUT) $(INCLUDE) $(LD_LIBRARY_PATH) -l$(ZLOG) -l$(XBEE) -lpthread -lrt -static

