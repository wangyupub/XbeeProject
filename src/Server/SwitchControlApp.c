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
#include <pthread.h>
#include <errno.h>

#include "Util.h"
#include "SocketServerControl.h"
#include "XBeeInterface.h"
#include "CommandLogic.h"
#include "RadioNetwork.h"

AppConfig gAppConfig;
static int gActiveSocketId = 0;
static pthread_rwlock_t	socketIdLock;

/* Log config file name */
extern const char* LOG_CONFIG_FILENAME;

/* INI file name */
extern const char* INI_FILENAME;

void _SocketIdInit()
{
  /* for now, use default config for read write lockers*/
  pthread_rwlock_init(&socketIdLock, NULL);
  gActiveSocketId = 0;
  
}

void _SocketIdDestroy()
{
  gActiveSocketId = 0;
  pthread_rwlock_destroy(&socketIdLock);
}

void _SetSocketId(int sid)
{
  /* entering critical zone */
  int err = pthread_rwlock_wrlock(&socketIdLock);
  if (err != 0) zlog_fatal(gZlogCategories[ZLOG_MAIN], "_SetSocketId fail to acquire lock");
  
  gActiveSocketId = sid;
  
  /* leaving critical zone */
  err = pthread_rwlock_unlock(&socketIdLock);
  if (err != 0) zlog_fatal(gZlogCategories[ZLOG_MAIN], "_SetSocketId fail to release lock");
}

void SSECallback(int sid, SocketServerEvent event, void* data, int data_len)
{
  zlog_debug(gZlogCategories[ZLOG_MAIN], "SSECallback is called %d", event);
  
  switch (event)
  {
    case SSE_RECEIVE:
    {
      if (data != NULL)
      {
#if 0
	char response[256];
	memset(response, 0, sizeof(response));
	
	snprintf(response, sizeof(response), "received data: [%s]", (char*) data);
	
	SocketServerSend(sid, response, strlen(response));

	int d = strncmp("exit", (char*)data, 4);
	if (d == 0) exit(0);
#endif //0

//#define DEBUG 1
#if defined(DEBUG)
	int i;
	char* buf_str = (char*) malloc (3*data_len + 2);
	char* buf_ptr = buf_str;
	unsigned char* str = (unsigned char*) data;
	for (i = 0; i < data_len; i++)
	{
	    buf_ptr += sprintf(buf_ptr, "|%2X", str[i]);
	}
	sprintf(buf_ptr,"|\0");
	zlog_debug(gZlogCategories[ZLOG_SOCKET], "Receiving %d bytes of data [%s]", data_len, buf_str);

#endif //defined(DEBUG)	
	ParseCommand(data, data_len);
      }
    }
    break;
    case SSE_CONNECT:
    {
      _SetSocketId(sid);
    }
    break;
    case SSE_DISCONNECT:
    {
      _SetSocketId(0);
    }
    break;
    default:
      break;
  }
}

int UpdateReturnData()
{
/* pool size of the return data */
#define RETURN_DATA_POOL_SIZE 16
  
  
  unsigned char buffer[RETURN_DATA_POOL_SIZE];

  /* Returns data response set up by RadioNetwork module */
  int size = RadioNetworkGetReturnData(buffer);
  if (size > 0)
  {
    zlog_debug(gZlogCategories[ZLOG_MAIN], "UpdateReturnData setting return data size -> %d", size);
    /* entering critical zone */
    int err = pthread_rwlock_rdlock(&socketIdLock);
    if (err != 0) zlog_fatal(gZlogCategories[ZLOG_MAIN], "UpdateReturnData fail to acquire lock");

    /**
     * locking the socket id does not prevent client from disconnecting, so this may still fail.
     * so, the right way to do this may be storing the socket id in the command, do not send if
     * socket id does not match. */
    if (gActiveSocketId != 0)
    {
      zlog_debug(gZlogCategories[ZLOG_MAIN], "Sending return data to sid [%d]", gActiveSocketId);
      int ret = SocketServerSend(gActiveSocketId, buffer, size);
      if (ret > 0)
      {
	zlog_debug(gZlogCategories[ZLOG_MAIN], "[%d] bytes sent to sid [%d]", ret, gActiveSocketId);	
      }
      if (ret != size)
      {
	zlog_warn(gZlogCategories[ZLOG_MAIN], "return data sent unsuccessfully with errno [%d]", errno);
      }
    }
    /* leaving critical zone */
    err = pthread_rwlock_unlock(&socketIdLock);
    if (err != 0) zlog_fatal(gZlogCategories[ZLOG_MAIN], "UpdateReturnData fail to release lock");
  }
  return size;
  
}

void FinalCleanup()
{
  free(gAppConfig.radioNetworkConfig.addressBuffer);
}

int main(void)
{
  int error;
  
  pthread_t command_processor_pid;
  
  /* Initializes zlog first.*/
  if ((error = zLogInit()) != 0)
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
  
  /* Initializes Radio Network */
  RadioNetworkInit(&gAppConfig.radioNetworkConfig);
  
  _SocketIdInit();
  
  /* Initializes command processing thread with default settings */
  pthread_create(&command_processor_pid, NULL, &RadioNetworkProcessCommandQueue, UpdateReturnData);
  
  SocketServerStart();
  SocketServerRun();
  
  /* Wait to join the command processing thread */
  pthread_join(command_processor_pid, NULL);

  _SocketIdDestroy();
  
  SocketServerStop();
  SocketServerDestroy();

  /* Destroys Radio Network */
  RadioNetworkDestroy();
  
  /* Destroys local XBee radio */
  XBeeRadioDestroy();


  zLogDestroy();

  /* final clean ups of memory */
  FinalCleanup();  
  
  return 0;
}
