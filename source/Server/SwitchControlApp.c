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


void SSECallback(int sid, SocketServerEvent event, void* data, int data_len)
{
  printf("SSECallback is called %d\n", event);
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
  /* Initializes SocketServer */
  SocketServerConfig config;
  config.eventHandler = SSECallback;
  config.port = 3940;
  
  SocketServerInit(config);
  
  SocketServerStart();
  SocketServerRun();
  SocketServerStop();
  SocketServerDestroy();
  return 0;
}
