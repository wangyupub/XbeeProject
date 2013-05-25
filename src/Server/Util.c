/********************************************************************
 * Util.c
 * 
 * Housekeeper for all utilities, a buffer tier between any third
 * party tools.
 * 
 ********************************************************************/
#include <string.h>

#include "Util.h"


unsigned int htoi(char *s)
{
	unsigned int n;
	int r;

	r = sscanf(s, "%x", &n);
	return n;
}

int zLogInit()
{
  int rc;
  rc = zlog_init(LOG_CONFIG_FILENAME);
  if (rc) {
    printf("zlog init failed\n");
    return -1;
  }
  
  
  gZlogCategories[ZLOG_MAIN] = zlog_get_category("MAIN");
  if (!gZlogCategories[ZLOG_MAIN])
  {
    zLogDestroy();
    printf("zlog get cat fail\n");
    return -2;
  }
  gZlogCategories[ZLOG_SOCKET] = zlog_get_category("SOCKET");
  if (!gZlogCategories[ZLOG_SOCKET])
  {
    zLogDestroy();
    zlog_fatal(gZlogCategories[ZLOG_MAIN], "zlog get cat fail\n");
    return -2;
  }
  gZlogCategories[ZLOG_XBEE] = zlog_get_category("XBEE");
  if (!gZlogCategories[ZLOG_XBEE])
  {
    zLogDestroy();
    zlog_fatal(gZlogCategories[ZLOG_MAIN],"zlog get cat fail\n");
    return -2;
  }
  gZlogCategories[ZLOG_COMMAND] = zlog_get_category("COMMAND");
  if (!gZlogCategories[ZLOG_COMMAND])
  {
    zLogDestroy();
    zlog_fatal(gZlogCategories[ZLOG_MAIN], "zlog get cat fail\n");
    return -2;
  }

  
  return 0;
}


int zLogDestroy()
{
  zlog_fini();
}

int iniHandler(void* user, const char* section, const char* name, const char* value)
{
    int address;
    long long longAddress;
  
    AppConfig* pconfig = (AppConfig*) user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("SocketServer", "ServerPort"))
    {
      pconfig->uServerPort = atoi(value);
    }
    else if (MATCH("XBeeControl", "XBeeMode"))
    {
      pconfig->xbeeRadioConfig.szXBeeMode = strdup(value);
    }
    else if (MATCH("XBeeControl", "DevicePath"))
    {
      pconfig->xbeeRadioConfig.szDevicePath = strdup(value);
    }
    else if (MATCH("XBeeControl", "BaudRate"))
    {
      pconfig->xbeeRadioConfig.iBaudRate = atoi(value);
    }
    else if (MATCH("XBeeNetwork", "Address"))
    {
/*
      address = htoi(value);
      longAddress = strtol(value, NULL, 16);
      
      zlog_debug(gZlogCategories[ZLOG_MAIN], "(%s) converted to int: %x long int: %x\n", value, address, longAddress);
*/
      
    }
    else
    {
      zlog_warn(gZlogCategories[ZLOG_MAIN], "Unrecognized token in ini file Section: [%s] Name: %s.\n", section, name);
      return 0;  /* unknown section/name, error */
    }
    return 1;  
}
