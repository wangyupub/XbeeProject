/********************************************************************
 * SwitchControlApp.c
 * 
 * Main file for SwitchControlApp application, coordinating all
 * modules: SocketServerControl, XBeeRadioControl and CommandControl.
 * 
 ********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "Util.h"
#include "SocketServerControl.h"
#include "XBeeInterface.h"

AppConfig gAppConfig;

void SSECallback(int sid, SocketServerEvent event, void* data, int data_len)
{
  zlog_debug(gZlogCategories[ZLOG_MAIN], "SSECallback is called %d\n", event);
  switch (event)
  {
    case SSE_RECEIVE:
    {
      if (data != NULL)
      {
	char response[256];
	memset(response, 0, sizeof(response));
	
	snprintf(response, sizeof(response), "received data: [%s]\n", (char*) data);
	
	SocketServerSend(sid, response, strlen(response));

	int d = strncmp("exit", (char*)data, 4);
	if (d == 0) exit(0);
      }
    }
    break;
    default:
      break;
  }
}

int main(void)
{
  int error;
  
  /* Initializes zlog first.*/
  if (error = zLogInit() != 0)
  {
    return error;
  }
  
  /* Parses ini file for config. */
  memset(&gAppConfig, 0, sizeof(AppConfig));
  if (ini_parse(INI_FILENAME, iniHandler, &gAppConfig) < 0)
  {
    zlog_fatal(gZlogCategories[ZLOG_MAIN], "Can't load '%s'\n", INI_FILENAME);
    return 1;
  }
  
  /* Initializes SocketServer */
  SocketServerConfig config;
  config.eventHandler = SSECallback;
  config.port = gAppConfig.uServerPort;

  zlog_info(gZlogCategories[ZLOG_MAIN], "Initializing SocketServer on port %d\n", config.port);
  SocketServerInit(&config);
  
  /* Initializes local XBee radio */
  XBeeRadioInit(&gAppConfig.xbeeRadioConfig);
  
  SocketServerStart();
  SocketServerRun();

  /* Destroys local XBee radio */
  XBeeRadioDestroy();

  SocketServerStop();
  SocketServerDestroy();

  zLogDestroy();
  
  return 0;
}
