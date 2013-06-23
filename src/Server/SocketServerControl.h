#if !defined(__SOCKET_SERVER_CONTROL_H__)
#define __SOCKET_SERVER_CONTROL_H__

/********************************************************************
 * SocketServerControl.h
 *
 * SocketServerControl handles socket communication with client.
 * The server takes only 1 concurrent connection at the moment, which
 * works perfectly in this application.
 * 
 ********************************************************************/


typedef enum
{
  SSE_INIT = 0,
  SSE_START,
  SSE_CONNECT,
  SSE_RECEIVE,
  SSE_DISCONNECT,
  SSE_STOP,
  SSE_SHUTDOWN,
  SSE_TOTAL,
} SocketServerEvent;

/* Event Handler Callback */
typedef void (*SocketServerEventHandler)(int sid, SocketServerEvent event, void* data, int data_len);

typedef void (*SocketServerCustomUpdate)(int sid);

typedef struct
{
  int				port;
  int				maxConcurrentConnection;
  SocketServerEventHandler	eventHandler;
  SocketServerCustomUpdate	customUpdate;
} SocketServerConfig;

/* Setup SocketServer module */
int SocketServerInit(SocketServerConfig* config);

/* Clean up SocketServer module*/
int SocketServerShutdown();

/* Socket Server Start Listening */
int SocketServerStart();

/* Socket Server Stop Listening */
int SocketServerStop();

/* Socket Server send data */
int SocketServerSend(int sid, void* data, int length);

/* Socket accept loop */
int SocketServerRun();


#endif /* __SOCKET_SERVER_CONTROL_H__ */