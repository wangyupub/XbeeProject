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

#include "SocketServerControl.h"

#include "zlog.h"

zlog_category_t *main_zlog_category;

void SSECallback(int sid, SocketServerEvent event, void* data, int data_len)
{
  zlog_debug(main_zlog_category, "SSECallback is called %d\n", event);
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
  /* Initializes zlog first.*/
  int rc;
  rc = zlog_init("SwitchControlAppLog.conf");
  if (rc) {
    printf("init failed\n");
    return -1;
  }
  main_zlog_category = zlog_get_category("MAIN");
  if (!main_zlog_category) {
    printf("get cat fail\n");
    zlog_fini();
    return -2;
  }
  
  /* Initializes SocketServer */
  SocketServerConfig config;
  config.eventHandler = SSECallback;
  config.port = 3940;

  zlog_info(main_zlog_category, "Initializing SocketServer");
  SocketServerInit(config);
  
  
  SocketServerStart();
  SocketServerRun();
  SocketServerStop();
  SocketServerDestroy();


  zlog_fini();

  return 0;
}
