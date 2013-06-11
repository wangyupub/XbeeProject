/********************************************************************
 * RadioNetwork.c
 * 
 * Sets up radio network, carries out parsed command to communicate
 * with local radio (XBee in this case, for now), and maintains a
 * command pool updated by a dedicated thread.
 * 
 ********************************************************************/

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

#include "Util.h"
#include "RadioNetwork.h"
#include "XBeeInterface.h"

/* the parsed command is converted to a pack of data depicting
 * where / when / what to send.
 */
typedef struct
{
  XBeeConnectionConfig	config;
  void*			toSend;
  int			dataLength;
  /* the moment the command is to be triggered*/
  time_t		triggerTime;
} RadioCommand;

typedef struct CommandNode CommandNode_t;

struct CommandNode
{
  RadioCommand*		pCommand;
  CommandNode_t*	pPrev;
  CommandNode_t*	pNext;
};

typedef struct
{
  CommandNode_t*	pHead;
  CommandNode_t*	pTail;
  pthread_rwlock_t	lock;
  int			count;
} CommandQueue;

/* global command queue*/
static CommandQueue	gCommandQueue;

/* thread safe function */
void _AddCommand(RadioCommand* command)
{
  int err = 0;
  
  CommandNode_t* node = malloc(sizeof(CommandNode_t));
  /* always check on malloc */
  assert(node != NULL);
  
  /* entering critical zone */
  err = pthread_rwlock_wrlock(&gCommandQueue.lock);
  if (err != 0) zlog_fatal(gZlogCategories[ZLOG_COMMAND], "_AddCommand fail to acquire lock\n");
  
#if defined(INSERT_COMMAND)
  node->pPrev = NULL;
  node->pNext = gCommandQueue.pHead;
  if (gCommandQueue.pHead != NULL)
    gCommandQueue.pHead->pPrev = node;
  else
    gCommandQueue.pTail = node;	/* queue was empty */
  gCommandQueue.pHead = node;
#endif //INSERT_COMMAND
  
  node->pNext = NULL;
  node->pPrev = gCommandQueue.pTail;
  if (gCommandQueue.pTail != NULL)
    gCommandQueue.pTail->pNext = node;
  else
    gCommandQueue.pHead = node; /* queue was empty */
  gCommandQueue.pTail = node;
  gCommandQueue.count++;
  
  zlog_debug(gZlogCategories[ZLOG_COMMAND], "_AddCommand: %d command(s) in queue after 1 new added\n", gCommandQueue.count);
  
  /* leaving critical zone */
  err = pthread_rwlock_unlock(&gCommandQueue.lock);
  if (err != 0) zlog_fatal(gZlogCategories[ZLOG_COMMAND], "_AddCommand fail to release lock\n");
  
}

RadioCommand* _RetrieveCommand()
{
  int err = 0;
  RadioCommand* toRet;
  CommandNode_t* node = NULL;
  err = pthread_rwlock_wrlock(&gCommandQueue.lock);
  if (err != 0) zlog_fatal(gZlogCategories[ZLOG_COMMAND], "_RetrieveCommand fail to acquire lock\n");

  if (gCommandQueue.pHead == NULL)
    toRet = NULL;
  else
  {
    node = gCommandQueue.pHead;
    toRet = node->pCommand;
    
    /* removing the head element */
    if (node->pNext != NULL)
    {
      node->pNext->pPrev = NULL;
    }
    else /* queue will be empty */
    {
      gCommandQueue.pTail = NULL;
    }
    gCommandQueue.pHead = node->pNext;
  
    gCommandQueue.count--;
    free(node);
  }

  zlog_debug(gZlogCategories[ZLOG_COMMAND], "_RetrieveCommand: %d command(s) in queue after 1 new removed\n", gCommandQueue.count);

  err = pthread_rwlock_unlock(&gCommandQueue.lock);  
  if (err != 0) zlog_fatal(gZlogCategories[ZLOG_COMMAND], "_RetrieveCommand fail to release lock\n");
}

int _ProcessCommand(RadioCommand* pCommand)
{
  assert(pCommand != NULL);
    
  XBeeRadioConnect(&pCommand->config);
  
  /* make sure to handle the broadcast logic */
  XBeeRadioSend(pCommand->toSend, pCommand->dataLength);
  
  XBeeRadioDisconnect();

  return 0;
}


void RadioNetworkInit(RadioNetworkConfig* config)
{
  
  gCommandQueue.pHead = NULL;
  gCommandQueue.pTail = NULL;

  /* for now, use default config for read write lockers*/
  pthread_rwlock_init(&gCommandQueue.lock, NULL);
  
}

void* RadioNetworkProcessCommandQueue(void *arg)
{
  for (;;)
  {
    RadioCommand *command = _RetrieveCommand();
    if (command != NULL)
    {
      time_t current = time((time_t*)NULL);
      /* not ready to trigger the command with delay, put it back to the queue*/
      if (command->triggerTime > current)
      {
	zlog_debug(gZlogCategories[ZLOG_COMMAND],
		   "Delayed Command not ready to process. [trigger: %d; current: %d]\n", (int)command->triggerTime, (int)current);
	_AddCommand(command);
      }
      else
      {
	_ProcessCommand(command);
      }
    }
    else
    {
      /* sleep for 1 sec */
      sleep(1);
    }
  }
  return NULL;
}


void RadioNetworkDestroy()
{
  RadioCommand* command = _RetrieveCommand();
  /* clean up memories */
  /* todo: clean up memory for XBeeConnectionConfig */
  while (command != NULL)
  {
    if (command->toSend != NULL)
      free(command->toSend);
    free(command);
    command = _RetrieveCommand();
  }
  pthread_rwlock_destroy(&gCommandQueue.lock);
}

