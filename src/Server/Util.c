/********************************************************************
 * Util.c
 * 
 * Housekeeper for all utilities, a buffer tier between any third
 * party tools.
 * 
 ********************************************************************/

#include "Util.h"

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
    printf("zlog get cat fail\n");
    return -2;
  }
  gZlogCategories[ZLOG_XBEE] = zlog_get_category("XBEE");
  if (!gZlogCategories[ZLOG_XBEE])
  {
    zLogDestroy();
    printf("zlog get cat fail\n");
    return -2;
  }
  gZlogCategories[ZLOG_COMMAND] = zlog_get_category("COMMAND");
  if (!gZlogCategories[ZLOG_COMMAND])
  {
    zLogDestroy();
    printf("zlog get cat fail\n");
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

    AppConfig* pconfig = (AppConfig*) user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("SocketServer", "ServerPort"))
    {
        pconfig->uServerPort = atoi(value);
    }
    /*
    else if (MATCH("user", "name"))
    {
        pconfig->name = strdup(value);
    }
    else if (MATCH("user", "email"))
    {
        pconfig->email = strdup(value);
    }
    */
    else
    {
        return 0;  /* unknown section/name, error */
    }
    return 1;  
}
