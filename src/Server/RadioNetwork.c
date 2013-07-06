/********************************************************************
 * RadioNetwork.c
 * 
 * Sets up radio network, carries out parsed command to communicate
 * with local radio (XBee in this case, for now), and maintains a
 * command pool updated by a dedicated thread.
 * 
 ********************************************************************/
#define _REENTRANT

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "Util.h"
#include "RadioNetwork.h"
#include "XBeeInterface.h"

/******************************************************************
 *
 * the parsed command is converted to a pack of data depicting
 * where / when / what to send.
 * 
 ******************************************************************/

typedef void(*fpointer)();

typedef struct
{
  XBeeConnectionConfig	config;
  void*			toSend;
  int			sendSize;
  /* the moment the command is to be triggered*/
  time_t		triggerTime;
  
  /* these are to facilitate data return */
  command_type_t	commandType;
  int			uid;
  void*			toRet;
  int			retSize;
  int			customData;
  
} RadioCommand;

typedef struct CommandNode CommandNode_t;

struct CommandNode
{
  RadioCommand*		pCommand;
  CommandNode_t*	pPrev;
  CommandNode_t*	pNext;
};

/* pool size of the return data */
#define RETURN_DATA_POOL_SIZE 16

typedef struct
{
  pthread_rwlock_t	lock;
  CommandNode_t*	pHead;
  CommandNode_t*	pTail;
  int			count;
} CommandQueue;

/* global command queue*/
static CommandQueue	gCommandQueue;
static CommandQueue	gAwaitingReturnQueue;

void _InitQueue(CommandQueue* queue)
{
  queue->pHead = NULL;
  queue->pTail = NULL;
  
  queue->count = 0;

  /* for now, use default config for read write lockers*/
  pthread_rwlock_init(&queue->lock, NULL);
}


/* thread safe function */
void _AddCommand(CommandQueue* queue, RadioCommand* command)
{
  int err = 0;
    
  CommandNode_t* node = malloc(sizeof(CommandNode_t));
  /* always check on malloc */
  assert(node != NULL);
  
  node->pCommand = command;
  
  /* entering critical zone */
  err = pthread_rwlock_wrlock(&queue->lock);
  if (err != 0) zlog_fatal(gZlogCategories[ZLOG_COMMAND], "_AddCommand fail to acquire lock");
  
#if defined(INSERT_COMMAND)
  node->pPrev = NULL;
  node->pNext = queue.pHead;
  if (queue->pHead != NULL)
    queue->pHead->pPrev = node;
  else
    queue->pTail = node;	/* queue was empty */
  queue->pHead = node;
#endif //INSERT_COMMAND
  
  node->pNext = NULL;
  node->pPrev = queue->pTail;
  if (queue->pTail != NULL)
    queue->pTail->pNext = node;
  else
    queue->pHead = node; /* queue was empty */
  queue->pTail = node;
  queue->count++;
  
  zlog_debug(gZlogCategories[ZLOG_COMMAND], "_AddCommand: %d command(s) in queue after 1 new added", queue->count);
  
  /* leaving critical zone */
  err = pthread_rwlock_unlock(&queue->lock);
  if (err != 0) zlog_fatal(gZlogCategories[ZLOG_COMMAND], "_AddCommand fail to release lock");
  
}


void _FreeCommand(RadioCommand** pCommand)
{
  if ((*pCommand)->toSend != NULL) free((*pCommand)->toSend);
  if ((*pCommand)->toRet != NULL) free((*pCommand)->toRet);
  free(*pCommand);
  *pCommand = NULL;
}


RadioCommand* _RetrieveCommand(CommandQueue *queue)
{
  int err = 0;
  RadioCommand* toRet;
  CommandNode_t* node = NULL;
  err = pthread_rwlock_wrlock(&queue->lock);
  if (err != 0) zlog_fatal(gZlogCategories[ZLOG_COMMAND], "_RetrieveCommand fail to acquire lock");

  if (queue->pHead == NULL)
    toRet = NULL;
  else
  {
    node = queue->pHead;
    toRet = node->pCommand;
    
    /* removing the head element */
    if (node->pNext != NULL)
    {
      node->pNext->pPrev = NULL;
    }
    else /* queue will be empty */
    {
      queue->pTail = NULL;
    }
    queue->pHead = node->pNext;
  
    queue->count--;

    free(node);

    zlog_debug(gZlogCategories[ZLOG_COMMAND], "_RetrieveCommand: %d command(s) in queue after 1 new removed", queue->count);
  }


  err = pthread_rwlock_unlock(&queue->lock);
  if (err != 0) zlog_fatal(gZlogCategories[ZLOG_COMMAND], "_RetrieveCommand fail to release lock");
  return toRet;
}

void _DestroyQueue(CommandQueue* queue)
{
  RadioCommand* command = _RetrieveCommand(queue);
  /* clean up memories */
  /* todo: clean up memory for XBeeConnectionConfig */
  while (command != NULL)
  {
    _FreeCommand(&command);
    command = _RetrieveCommand(queue);
  }
  pthread_rwlock_destroy(&queue->lock);
  
}

int _ProcessCommand(RadioCommand* pCommand)
{
  int ret = 0;
  
  assert(pCommand != NULL);
  
  zlog_debug(gZlogCategories[ZLOG_COMMAND], "_ProcessCommand process command");
  
  ret = XBeeRadioConnect(&pCommand->config);  
  assert(ret == 0);
  
  ret = XBeeRadioSend(pCommand->toSend, pCommand->sendSize);
  assert(ret == 0);
  
#if !defined(USE_XBEE_CALLBACK) 
  /* Sets data received as return data */
  
  unsigned char* buffer[RETURN_DATA_POOL_SIZE];
  int dataLen = 0;

  memset(buffer, 0, RETURN_DATA_POOL_SIZE);
  
  /* always wait for XBee radio to return now, since right now XBee does not keep multiple connections */
  ret = XBeeRadioReceive(buffer, RETURN_DATA_POOL_SIZE, 0, &dataLen);

  if (ret == 0)
  {
    /* sets up the return data */
    _SetReturnData(pCommand, buffer, dataLen);
    
    /* puts into queue waiting for return data process */
    _AddCommand(&gAwaitingReturnQueue, pCommand);
  }
  else
  {
    unsigned char success = 0;
    _SetReturnData(pCommand, &success, sizeof(success));
    _AddCommand(&gAwaitingReturnQueue, pCommand);
    
  }
  /* todo: resend goes here if needed. */
#endif //defined(USE_XBEE_CALLBACK)  

  XBeeRadioDisconnect();

  return 0;
}

void _convertXBeeAddress(uint32_t* addressBuffer, struct xbee_conAddress* xbeeAddress)
{
  int i = 0;
  assert(addressBuffer != NULL);

  zlog_debug(gZlogCategories[ZLOG_COMMAND],
	     "_convertXBeeAddress [%x | %x]\n", *addressBuffer, *(addressBuffer+1));
  
  /* assuming addressBuffer contains two 32-bit int, which is AddressHi and AddressHi and AddressLow */
  memset((void*) xbeeAddress, 0, sizeof(struct xbee_conAddress));
  xbeeAddress->addr64_enabled = 1;
  
  /* convert the address to correct byte order */
  uint32_t address[2];
  address[0] = htonl(addressBuffer[0]);
  address[1] = htonl(addressBuffer[1]);

  memcpy((void*) xbeeAddress->addr64, (void*) address, sizeof(xbeeAddress->addr64));

  /*
  for (i = 0; i < 8; ++i)
  {
    zlog_debug(gZlogCategories[ZLOG_COMMAND], "address [%d] - %x\n", i, xbeeAddress->addr64[i]);
  }
*/
}

typedef struct
{
  struct xbee_conAddress*	endpoint_address_list;
  int				endpoint_count;
  int				switch_count;
} RadioNetworkSpec;

RadioNetworkSpec	g_radio_network_spec;

#if defined(USE_XBEE_CALLBACK)  
/* todo: handle return data from xbee here */
void _xbeeCB(struct xbee *xbee, struct xbee_con *con, struct xbee_pkt **pkt, void **data)
{
  if ((*pkt)->dataLen > 0)
  {
    if ((*pkt)->data[0] == '@')
    {
      xbee_conCallbackSet(con, NULL, NULL);
      zlog_debug(gZlogCategories[ZLOG_COMMAND], "*** DISABLED CALLBACK... ***\n");
    }
    zlog_debug(gZlogCategories[ZLOG_COMMAND], "rx: [%s]\n", (*pkt)->data);
  }
  zlog_debug(gZlogCategories[ZLOG_COMMAND], "tx: %d\n", xbee_conTx(con, NULL, "Hello\r\n"));
}
#endif /* USE_XBEE_CALLBACK */


void RadioNetworkInit(RadioNetworkConfig* config)
{
  int i = 0;

  _InitQueue(&gCommandQueue);
  _InitQueue(&gAwaitingReturnQueue);
  
  memset((void*) &g_radio_network_spec, 0, sizeof(g_radio_network_spec));
  
  g_radio_network_spec.endpoint_count = config->detailConfig.starConfig.endPointCount;
  
  g_radio_network_spec.switch_count = config->detailConfig.starConfig.switchCountPerEndPoint;
  
  g_radio_network_spec.endpoint_address_list = malloc(sizeof(struct xbee_conAddress) * g_radio_network_spec.endpoint_count);  
  assert(g_radio_network_spec.endpoint_address_list != NULL);
  
  /* now parse the address buffer to actual XBeeBuffer */
  
  zlog_debug(gZlogCategories[ZLOG_COMMAND], "converting addressBuffer");
  int* pBuffer = config->addressBuffer;
  for (i = 0; i < g_radio_network_spec.endpoint_count; ++i)
  {
    assert(pBuffer != NULL);
    _convertXBeeAddress(pBuffer, &g_radio_network_spec.endpoint_address_list[i]);
    
    /* move the pointer forward by 2 * 32 bits */
    pBuffer += 2;
  }
  
  
}

void* RadioNetworkProcessCommandQueue(void *arg)
{
  for (;;)
  {
    RadioCommand *command = _RetrieveCommand(&gCommandQueue);
    if (command != NULL)
    {
      zlog_debug(gZlogCategories[ZLOG_COMMAND],
	"RadioNetworkProcessCommandQueue: processing a command");
      time_t current = time((time_t*)NULL);
      /* not ready to trigger the command with delay, put it back to the queue*/
      if (command->triggerTime > current)
      {
	zlog_debug(gZlogCategories[ZLOG_COMMAND],
		   "Delayed Command not ready to process. [trigger: %d; current: %d]", (int)command->triggerTime, (int)current);
	_AddCommand(&gCommandQueue, command);
      }
      else
      {
	_ProcessCommand(command);
      }
    }
    else
    {
      if (arg != NULL)
      {
	fpointer func = (fpointer) arg;
	func();
      }
      /* sleep for 1 sec */
      sleep(1);
    }
  }
  return NULL;
}


void RadioNetworkDestroy()
{
  _DestroyQueue(&gCommandQueue);
  _DestroyQueue(&gAwaitingReturnQueue);

  free(g_radio_network_spec.endpoint_address_list);
}

/********************************************************************************
 * 
 * Sets data to send based on RadioNetwork structure and the remote XBee protocol.
 * Remote XBee protocal can be found at: 
 * 	http://www.tinyosshop.com/image/data/board_modules/usbrelay4-5.jpg
 * 
 ********************************************************************************/
int _FillCommand(RadioCommand* command, int switchIndex, int switchStatus)
{
  const char command_table[] = { 'e', 'f', 'g', 'h', 'o', 'p', 'q', 'r' };
  
  assert(command != NULL);
  memset(command, 0, sizeof(command));
  int endPointIndex = switchIndex / g_radio_network_spec.switch_count;
  int swictchIndexInEndpoint = switchIndex % g_radio_network_spec.switch_count;
  
  if (endPointIndex >= g_radio_network_spec.endpoint_count)
  {
    zlog_fatal(gZlogCategories[ZLOG_COMMAND], "_FillCommand: Requested switch index [%d] over limit.\n", switchIndex);
    return 1;
  }
  /* Sets up the xbee address */
  memcpy(&command->config.address, &g_radio_network_spec.endpoint_address_list[endPointIndex], sizeof(struct xbee_conAddress));
  
  /* Sets up the xbee callback */

#if defined(USE_XBEE_CALLBACK)  
  command->config.xbeeCallback = _xbeeCB;
#else /* USE_XBEE_CALLBACK */
  command->config.xbeeCallback = NULL;
#endif /* USE_XBEE_CALLBACK */

  command->config.connectionType = "Data";
  command->config.connectionData = NULL;
  
  /* Set up the trigger time as current time */
  command->triggerTime = time((time_t*)NULL);

  /* Fill in the data */
  command->toSend = malloc(sizeof(char));
  assert(command->toSend != NULL);
  command->sendSize = sizeof(char);
  
  if (switchStatus == 0)
  {
    *((char*) command->toSend) = command_table[swictchIndexInEndpoint];
  }
  else
  {
    *((char*) command->toSend) = command_table[g_radio_network_spec.switch_count + swictchIndexInEndpoint];
  }
  return 0;
}

/********************************************************************************
 * 
 * Sends arbitary data to sepcified XBee Radio
 * @param command	RadioCommand structure to fill in
 * @param endPointIndex	index of the XBee radio to send data to
 * @param data		data buffer to send to remote XBee
 * @param dataLength	length of the data buffer
 * @return 0 on success
 * 
 ********************************************************************************/
int _FillCommandA(RadioCommand* command, int endPointIndex, void* data, int dataLength)
{
  assert(command != NULL);
  memset(command, 0, sizeof(command));
  
  assert(data != NULL);

  if (endPointIndex >= g_radio_network_spec.endpoint_count)
  {
    zlog_fatal(gZlogCategories[ZLOG_COMMAND], "_FillCommand: Requested switch index [%d] over limit.\n", endPointIndex);
    return 1;
  }
  
  /* Sets up the xbee address */
  memcpy(&command->config.address, &g_radio_network_spec.endpoint_address_list[endPointIndex], sizeof(struct xbee_conAddress));
  
  /* Sets up the xbee callback */

#if defined(USE_XBEE_CALLBACK)  
  command->config.xbeeCallback = _xbeeCB;
#else /* USE_XBEE_CALLBACK */
  command->config.xbeeCallback = NULL;
#endif /* USE_XBEE_CALLBACK */

  command->config.connectionType = "Data";
  command->config.connectionData = NULL;
  
  /* Set up the trigger time as current time */
  command->triggerTime = time((time_t*)NULL);

  /* Fill in the data */
  command->toSend = malloc(dataLength);
  assert(command->toSend != NULL);
  command->sendSize = dataLength;

  memcpy(command->toSend, data, dataLength);
  
  return 0;
}

void RadioNetworkAppendCommand(command_type_t commandType, ...)
{
  RadioCommand *new_command = NULL;
  
  va_list params;
  va_start(params, commandType);

  switch (commandType)
  {
    case CmdSetSingleSwitch:
    case CmdSetSingleSwitchDelay:
    {
      int delay = 0;
      int switchIndex = va_arg(params, int);
      int switchStatus = va_arg(params, int);
      
      if (commandType == CmdSetSingleSwitchDelay)
	delay = va_arg(params, int);
      
      new_command = malloc(sizeof(RadioCommand));
      assert(new_command != NULL);
      
      _FillCommand(new_command, switchIndex, switchStatus);
      /* add delay period */
      new_command->commandType = commandType;
      new_command->triggerTime = commandType;
      new_command->triggerTime += delay;
      _AddCommand(&gCommandQueue, new_command);
      
    }
    break;
    case CmdSetMultipleSwitches:
    {
      int switchOffset = va_arg(params, int);
//      uint32_t switchMask[2];
//      switchMask[0] = va_arg(params, int);
//      switchMask[1] = va_arg(params, int);
      uint64_t switchMask = va_arg(params, uint64_t);
      
      int i = 0;
      uint64_t mask = 1;
      
      const int total_switch_count = g_radio_network_spec.endpoint_count * g_radio_network_spec.switch_count;
      
      /******************************************************************************
      * 
      * For each bit, set a switch status with a new command
      * possible optimization with fewer commands turning all mask on / off
      * however it will incur more dependency on remote XBee control command support
      * 
      ******************************************************************************/
      for (i = 0; i < 64; ++i)
      {
	int switchIndex = switchOffset + i;
	if (switchIndex >= total_switch_count)
	  break;
	
	new_command = malloc(sizeof(RadioCommand));
	assert(new_command != NULL);
	int switchStatus = mask & switchMask;
	
	_FillCommand(new_command, switchIndex, switchStatus);

	/* sets up command type for collecting the return data. */
	new_command->triggerTime = commandType;
	new_command->commandType = commandType;
	
	_AddCommand(&gCommandQueue, new_command);
	
	mask = mask << 1;
      }
    }
    break;
    case CmdPassThrough:
    {
      int length = va_arg(params, int);
      unsigned char* data = va_arg(params, unsigned char*);

      new_command = malloc(sizeof(RadioCommand));
      assert(new_command != NULL);

#if defined(USE_XBEE_CALLBACK)  
      new_command->config.xbeeCallback = _xbeeCB;
#else /* USE_XBEE_CALLBACK */
      new_command->config.xbeeCallback = NULL;
#endif /* USE_XBEE_CALLBACK */
      
      new_command->config.connectionType = "Data";
      new_command->config.connectionData = NULL;
      
      /* Set up the trigger time as current time */
      new_command->triggerTime = time((time_t*)NULL);

      /* Fill in the data */
      new_command->toSend = malloc(sizeof(char));
      assert(new_command->toSend != NULL);
      new_command->sendSize = sizeof(char);
      memcpy(new_command->toSend, data, length);
	
      memset(&new_command->config.address, 0, sizeof(new_command->config.address));
      new_command->config.address.addr64_enabled = 1;
      new_command->config.address.addr64[0] = 0x00;
      new_command->config.address.addr64[1] = 0x00;
      new_command->config.address.addr64[2] = 0x00;
      new_command->config.address.addr64[3] = 0x00;
      new_command->config.address.addr64[4] = 0x00;
      new_command->config.address.addr64[5] = 0x00;
      new_command->config.address.addr64[6] = 0xFF;
      new_command->config.address.addr64[7] = 0xFF;      

      new_command->commandType = commandType;

//      _FillCommand(new_command, switchIndex, switchStatus);
      _AddCommand(&gCommandQueue, new_command);

    }
    break;
    case CmdGetSingleSwitch:
    {
      char data = '[';
      new_command = malloc(sizeof(RadioCommand));
      assert(new_command != NULL);
      
      int switchIndex = va_arg(params, int);
      int endPointIndex = switchIndex / g_radio_network_spec.switch_count;
      int swictchIndexInEndpoint = switchIndex % g_radio_network_spec.switch_count;

      /* store this in customData field for return data parsing */
      new_command->customData = swictchIndexInEndpoint;
      
      _FillCommandA(new_command, endPointIndex, &data, sizeof(char));
      new_command->commandType = commandType;

      _AddCommand(&gCommandQueue, new_command);
    }
    break;
    case CmdGetMultipleSwitches:
    {
      const char* msg = "not supported\n";
//      _SetReturnData(msg, sizeof(msg));
      zlog_warn(gZlogCategories[ZLOG_COMMAND], "Command GetMultipleSwitches not yet supported.\n");
    }
    break;
    case CmdGetNumSwitches:
    {
      new_command = malloc(sizeof(RadioCommand));
      assert(new_command != NULL);
      memset(new_command, 0, sizeof(RadioCommand));
      
      uint16_t num_switches = g_radio_network_spec.switch_count * g_radio_network_spec.endpoint_count;
      num_switches = htons(num_switches);
      _SetReturnData(new_command, &num_switches, sizeof(uint16_t));
      new_command->commandType = commandType;

      _AddCommand(&gAwaitingReturnQueue, new_command);
    }
    break;    
    default:
      break;
  }
  
  va_end(params);
}

int _SetReturnData(RadioCommand* command, void* data, int size)
{
#if 0
  /* entering critical zone */
  int err = pthread_rwlock_wrlock(&gCommandQueue.lock);
  if (err != 0) zlog_fatal(gZlogCategories[ZLOG_COMMAND], "_SetReturnData fail to acquire lock\n");

  int copy_size = size < RETURN_DATA_POOL_SIZE ? size : RETURN_DATA_POOL_SIZE;
  if (copy_size > 0)
  {
    assert(data != NULL);
    memcpy(gCommandQueue.returnDataPool, data, copy_size);
  }
  
  gCommandQueue.returnDataSize = copy_size;

  /* leaving critical zone */
  err = pthread_rwlock_unlock(&gCommandQueue.lock);
  if (err != 0) zlog_fatal(gZlogCategories[ZLOG_COMMAND], "_SetReturnData fail to release lock\n");

  return copy_size;
#endif //0
  assert(data != NULL);
  command->toRet = malloc(size);
  assert(command->toRet != NULL);
  memcpy(command->toRet, data, size);
  command->retSize = size;
  
  return 0;
}

/** Assembles return data here. */
int RadioNetworkGetReturnData(void* buffer)
{
#if 0
  /* entering critical zone */
  int err = pthread_rwlock_rdlock(&gCommandQueue.lock);
  if (err != 0) zlog_fatal(gZlogCategories[ZLOG_COMMAND], "RadioNetworkGetReturnData fail to acquire lock\n");

  int size = gCommandQueue.returnDataSize;

  if (size > 0)
  {
    assert(buffer != NULL);
    memcpy(buffer, &gCommandQueue.returnDataPool, size);
  }

  /* leaving critical zone */
  err = pthread_rwlock_unlock(&gCommandQueue.lock);
  if (err != 0) zlog_fatal(gZlogCategories[ZLOG_COMMAND], "RadioNetworkGetReturnData fail to release lock\n");
  
  _SetReturnData(NULL, 0);
  return size;  
#endif /* 0 */
  int size = 0;
  assert(buffer != NULL);
  RadioCommand *command = _RetrieveCommand(&gAwaitingReturnQueue);
  if (command != NULL)
  {
    zlog_debug(gZlogCategories[ZLOG_COMMAND], "processing command for return data");
    switch (command->commandType)
    {
      case CmdSetSingleSwitch:
      case CmdSetSingleSwitchDelay:
      {
	/* always return success for SetSingle commands */
	char success = 1;
	size = sizeof(char);
	memcpy(buffer, &success, size);
      }
      break;
      case CmdSetMultipleSwitches:
      {
	char success = 1;
	size = sizeof(char);
	memcpy(buffer, &success, size);
	zlog_warn(gZlogCategories[ZLOG_COMMAND], "CmdSetMultipleSwitches only returns success now\n");
      }
      break;
      case CmdPassThrough:
      {
	/* always return success for PassThrough commands */
	char success = 1;
	size = sizeof(char);
	memcpy(buffer, &success, size);
      }
      break;
      case CmdGetSingleSwitch:
      {
	/* gets the switch whose index is stored in customData */
	int mask = 0x01 << command->customData;
	char switchState = ((*((char*) command->toRet)) & mask != 0);
	size = sizeof(char);	
	memcpy(buffer, &switchState, size);
      }
      break;
      case CmdGetMultipleSwitches:
      {
	zlog_warn(gZlogCategories[ZLOG_COMMAND], "CmdGetMultipleSwitches does not handle return data now\n");
      }
      break;
      case CmdGetNumSwitches:
      {
	memcpy(buffer, command->toRet, command->retSize);
      }
      break;    
      default:
	break;
    }    
  
    /* clean up the command memory after final use */
    _FreeCommand(&command);
  }
  
  return size;
}

