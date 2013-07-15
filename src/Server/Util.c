/********************************************************************
 * Util.c
 * 
 * Housekeeper for all utilities, a buffer tier between any third
 * party tools.
 * 
 ********************************************************************/
#include <string.h>
#include <stdlib.h>

#include "Util.h"

/* Log config file name */
const char* LOG_CONFIG_FILENAME = "zlog.conf";

/* INI file name */
const char* INI_FILENAME = "config.ini";


unsigned int htoi(const char *s)
{
	unsigned int n;
	sscanf(s, "%x", &n);
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

  return 0;
}

int iniHandler(void* user, const char* section, const char* name, const char* value)
{  
    AppConfig* pconfig = (AppConfig*) user;

    #define MATCH(s, n) (strcmp(section, s) == 0 && strcmp(name, n)) == 0
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
    else if (MATCH("RadioNetwork", "NetworkTopology"))
    {
      if (strcmp("star", value) == 0)
      {
	pconfig->radioNetworkConfig.topology = NetworkStar;
      }
      else
      {
	zlog_warn(gZlogCategories[ZLOG_MAIN],
		  "Unsupported radio network topology %s found in ini file Section: [%s] Name: %s.\n",
	   value, section, name);
      }
    }
    else if (MATCH("RadioNetwork", "CommandPoolSize"))
    {
      pconfig->radioNetworkConfig.commandPoolSize = atoi(value);
    }
    else if (MATCH("RadioNetwork", "StarEndPoints"))
    {
      pconfig->radioNetworkConfig.detailConfig.starConfig.endPointCount = atoi(value);
      
      /* allocating memory for address buffer, each endpoint takes 2 * 4 bytes */
      int size = sizeof(uint32_t) * 2 * pconfig->radioNetworkConfig.detailConfig.starConfig.endPointCount;
      pconfig->radioNetworkConfig.addressBuffer = malloc(size);
      memset(pconfig->radioNetworkConfig.addressBuffer, 0, size);
    }
    else if (MATCH("RadioNetwork", "StarNumberSwitches"))
    {
      pconfig->radioNetworkConfig.detailConfig.starConfig.switchCountPerEndPoint = atoi(value);
    }
    else if (MATCH("RadioNetwork", "AddressHi") || MATCH("RadioNetwork", "AddressLo"))
    {
      /* copy the address into the pre-allocated buffer */
      uint32_t* pointer = pconfig->radioNetworkConfig.addressBuffer;
      if (pointer != NULL)
      {
	int addressCount = 0;
	/* move pointer forward until the first zero (unfilled) int. */
	while (*pointer != 0)
	{
	  ++pointer;
	  if (++addressCount >= 2 * pconfig->radioNetworkConfig.detailConfig.starConfig.endPointCount)
	  {
	    zlog_warn(gZlogCategories[ZLOG_MAIN], "ini file parsing: more address needed is being set\n");
	    return 0;
	  }
	}
	*pointer = htoi(value);
	zlog_debug(gZlogCategories[ZLOG_MAIN], "Parsing address: %x\n", *pointer);
      }
      else
      {
	zlog_fatal(gZlogCategories[ZLOG_MAIN], "Address being set before EndPoints is set Section: [%s] Name: %s.\n", section, name);
	return 0;
      }
    }
    else
    {
      zlog_warn(gZlogCategories[ZLOG_MAIN], "Unrecognized token in ini file Section: [%s] Name: %s.\n", section, name);
      return 0;  /* unknown section/name, error */
    }
    return 1;  
}
