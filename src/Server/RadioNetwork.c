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

/* pool size of the return data */
#define RETURN_DATA_POOL_SIZE 16

typedef struct
{
  unsigned char		returnDataPool[RETURN_DATA_POOL_SIZE];

  pthread_rwlock_t	lock;
  CommandNode_t*	pHead;
  CommandNode_t*	pTail;
  int			count;

  int			returnDataSize;  
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

    zlog_debug(gZlogCategories[ZLOG_COMMAND], "_RetrieveCommand: %d command(s) in queue after 1 new removed\n", gCommandQueue.count);
  }


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

RadioNetworkSpec	radio_network_spec;

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

void RadioNetworkInit(RadioNetworkConfig* config)
{
  int i = 0;
  gCommandQueue.pHead = NULL;
  gCommandQueue.pTail = NULL;
  
  gCommandQueue.count = 0;
  gCommandQueue.returnDataSize = 0;
  
  memset((void*) &radio_network_spec, 0, sizeof(radio_network_spec));
  
  radio_network_spec.endpoint_count = config->detailConfig.starConfig.endPointCount;
  
  radio_network_spec.switch_count = config->detailConfig.starConfig.switchCountPerEndPoint;
  
  radio_network_spec.endpoint_address_list = malloc(sizeof(struct xbee_conAddress) * radio_network_spec.endpoint_count);  
  assert(radio_network_spec.endpoint_address_list != NULL);
  
  /* now parse the address buffer to actual XBeeBuffer */
  
  zlog_debug(gZlogCategories[ZLOG_COMMAND], "converting addressBuffer\n");
  int* pBuffer = config->addressBuffer;
  for (i = 0; i < radio_network_spec.endpoint_count; ++i)
  {
    assert(pBuffer != NULL);
    _convertXBeeAddress(pBuffer, &radio_network_spec.endpoint_address_list[i]);
    
    /* move the pointer forward by 2 * 32 bits */
    pBuffer += 2;
  }
  

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

/* ******************************************************************************
 * 
 * Set data to send based on RadioNetwork structure and the remote XBee protocol.
 * Remote XBee protocal can be found at: 
 * 	http://www.tinyosshop.com/image/data/board_modules/usbrelay4-5.jpg
 * 
*********************************************************************************/
int _FillCommand(RadioCommand* command, int switchIndex, int switchStatus)
{
  const char command_table[] = { 'e', 'f', 'g', 'h', 'o', 'p', 'q', 'r' };
  
  assert(command != NULL);
  memset(command, 0, sizeof(command));
  int endPointIndex = switchIndex / radio_network_spec.switch_count;
  int swictchIndexInEndpoint = switchIndex % radio_network_spec.switch_count;
  
  if (endPointIndex >= radio_network_spec.endpoint_count)
  {
    zlog_fatal(gZlogCategories[ZLOG_COMMAND], "_FillCommand: Requested switch index [%d] over limit.\n", switchIndex);
    return 1;
  }
  /* Sets up the xbee address */
  memcpy(&command->config.address, &radio_network_spec.endpoint_address_list[endPointIndex], sizeof(struct xbee_conAddress));
  
  /* Sets up the xbee callback */
  command->config.xbeeCallback = _xbeeCB;
  
  command->config.connectionType = "Data";
  command->config.connectionData = NULL;
  
  /* Set up the trigger time as current time */
  command->triggerTime = time((time_t*)NULL);

  /* Fill in the data */
  command->toSend = malloc(sizeof(char));
  assert(command->toSend != NULL);
  command->dataLength = sizeof(char);
  
  if (switchStatus == 0)
  {
    *((char*) command->toSend) = command_table[swictchIndexInEndpoint];
  }
  else
  {
    *((char*) command->toSend) = command_table[radio_network_spec.switch_count + swictchIndexInEndpoint];
  }
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
      new_command->triggerTime += delay;
      _AddCommand(new_command);
      
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
      
      const int total_switch_count = radio_network_spec.endpoint_count * radio_network_spec.switch_count;
      
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
	_AddCommand(new_command);
	
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

      new_command->config.xbeeCallback = _xbeeCB;
      
      new_command->config.connectionType = "Data";
      new_command->config.connectionData = NULL;
      
      /* Set up the trigger time as current time */
      new_command->triggerTime = time((time_t*)NULL);

      /* Fill in the data */
      new_command->toSend = malloc(sizeof(char));
      assert(new_command->toSend != NULL);
      new_command->dataLength = sizeof(char);
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

//      _FillCommand(new_command, switchIndex, switchStatus);
      _AddCommand(new_command);

    }
    break;
    default:
      break;
  }
  
  va_end(params);
}

int _SetReturnData(void* data, int size)
{
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
}

int RadioNetworkGetReturnData(void* buffer)
{
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
}

