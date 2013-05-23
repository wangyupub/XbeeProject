SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .h .o

CC = gcc
#CC = ~/bin/4.4.3/bin/arm-linux-gcc


LIBRARY_PATH_PREFIX = ../../lib

ZLOG = zlog
INIH = inih

INCLUDE =  -I $(LIBRARY_PATH_PREFIX)/$(ZLOG)/src -I $(LIBRARY_PATH_PREFIX)/$(INIH)
LD_LIBRARY_PATH = $(LIBRARY_PATH_PREFIX)/$(ZLOG)/src
LD_SEARCH_PATH = $(LD_LIBRARY_PATH)

LIBZLOG = $(LD_LIBRARY_PATH)/lib$(ZLOG).so

PROG = $(LIBRARY_PATH_PREFIX)/$(INIH)/ini.c SocketServerControl.c SwitchControlApp.c Util.c
OUT = SwitchControlApp


all: $(OUT)
	

new:
	clean all

clean:
	(rm -f $(OUT); cd $(LIBRARY_PATH_PREFIX)/$(ZLOG); $(MAKE) clean)

LIBZLOG:
	(cd $(LIBRARY_PATH_PREFIX)/$(ZLOG); $(MAKE) CC=$(CC))

	
$(INIH).o:
	(cd $(LIBRARY_PATH_PREFIX)/$(INIH); $(CC) ini.c)

$(OUT): $(PROG) LIBZLOG
	$(CC) $(filter %.c,$^) -o $(OUT) $(INCLUDE) -L $(LD_SEARCH_PATH) -l$(ZLOG) -lpthread -lrt -static

