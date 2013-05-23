SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .h .o

CC = gcc
#CC = ~/bin/4.4.3/bin/arm-linux-gcc

PROG = SocketServerControl.c SwitchControlApp.c
OUT = SwitchControlApp

LIBRARY_PATH_PREFIX = ../../lib

ZLOG = zlog

INCLUDE =  $(LIBRARY_PATH_PREFIX)/$(ZLOG)/src
LD_LIBRARY_PATH = $(LIBRARY_PATH_PREFIX)/$(ZLOG)/src
LD_SEARCH_PATH = $(LD_LIBRARY_PATH)

LIBZLOG = $(LD_LIBRARY_PATH)/lib$(ZLOG).so



all: $(OUT)
	

new:
	clean all

clean:
	(rm -f $(OUT); cd $(LIBRARY_PATH_PREFIX)/$(ZLOG); $(MAKE) clean)

LIBZLOG:
	(cd $(LIBRARY_PATH_PREFIX)/$(ZLOG); $(MAKE) CC=$(CC))

$(OUT): $(PROG) LIBZLOG
	$(CC) $(filter %.c,$^) -o $(OUT) -I $(INCLUDE) -L $(LD_SEARCH_PATH) -l$(ZLOG) -lpthread -lrt -static

