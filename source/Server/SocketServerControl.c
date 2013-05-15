#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "SocketServerControl.h"

struct addrinfo *servinfo;  /* will point to the results */
int socketfd;
int new_fd;
SocketServerEventHandler gSocketServerHandler;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int SocketServerInit(SocketServerConfig config)
{
  int status;
  struct addrinfo hints;
  char port[8];

  memset(&hints, 0, sizeof hints); /* make sure the struct is empty */
  hints.ai_family = AF_UNSPEC;     /* don't care IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM; /* TCP stream sockets */
  hints.ai_flags = AI_PASSIVE;     /* fill in my IP for me */

  gSocketServerHandler = config.eventHandler;
  snprintf(port, sizeof(port), "%d", config.port);
  
  if ((status = getaddrinfo(NULL, port, &hints, &servinfo)) != 0)
  {
    fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
    return 1;
  }
  else
  {
    if (gSocketServerHandler != NULL)
    {
      gSocketServerHandler(socketfd, SSE_INIT, NULL, 0);
    }
    return 0;
  }
}

int SocketServerDestroy()
{
  /* servinfo now points to a linked list of 1 or more struct addrinfos */

  /* ... do everything until you don't need servinfo anymore .... */

  freeaddrinfo(servinfo); /* free the linked-list */

  if (gSocketServerHandler != NULL)
  {
    gSocketServerHandler(socketfd, SSE_SHUTDOWN, NULL, 0);
  }
  gSocketServerHandler = NULL;
  
  return 0;
}

int SocketServerStart()
{
  /* make a socket: */
  socketfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
  
  /* bind it to the port we passed in to getaddrinfo(): */
  bind(socketfd, servinfo->ai_addr, servinfo->ai_addrlen);
  
  /* listen to the socket */
  listen(socketfd, 5);
  
  if (gSocketServerHandler != NULL)
  {
    gSocketServerHandler(socketfd, SSE_START, NULL, 0);
  }
  
  return 0;
}

int SocketServerStop()
{
  close(socketfd);
  
  if (gSocketServerHandler != NULL)
  {
    gSocketServerHandler(socketfd, SSE_STOP, NULL, 0);
  }
  
  return 0;
}

int SocketServerSend(int sid, void* data, int length)
{
  return send(sid, data, length, 0);
}

int SocketServerRun()
{
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;

  char s[INET6_ADDRSTRLEN];

  char data[1024];
  int size;
  
  /* main accept() loop */
  while(1)
  {  
    sin_size = sizeof their_addr;
    new_fd = accept(socketfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1)
    {
	perror("accept");
	continue;
    }
    else
    {
      SocketServerStop();
      inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
      printf("server: got connection from %s\n", s);
  
      if (gSocketServerHandler != NULL)
      {
	gSocketServerHandler(new_fd, SSE_CONNECT, NULL, 0);
      }
      
      memset((void*) data, 0, sizeof(data));
      
      /* accept loop */
      while (1)
      {
	size = recv(new_fd, (void*) data, sizeof(data), 0);
	if (size > 0)
	{
	  if (gSocketServerHandler != NULL)
	  {
	    gSocketServerHandler(new_fd, SSE_RECEIVE, data, size);
	  }
	}
	else if (0 == size)
	{
	  close(new_fd);
	    
	  if (gSocketServerHandler != NULL)
	  {
	    gSocketServerHandler(new_fd, SSE_DISCONNECT, NULL, 0);
	  }
	  SocketServerStart();
	  break;
	}
      }
    }
  }
}
