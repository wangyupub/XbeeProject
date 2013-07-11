/********************************************************************
 * CommandLogic.c
 * 
 * Low level command processors, interpreting command from received
 * data buffer.
 * 
 ********************************************************************/
#include <assert.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "Util.h"

#include "CommandProtocol.h"
#include "CommandLogic.h"

#include "RadioNetwork.h"


unsigned char _GetParamChar8(void *data)
{
  return *((unsigned char*) data);
}

uint16_t _GetParamInt16(void* data)
{
  return ntohs(*((uint16_t*) data));
}

uint32_t _GetParamInt32(void *data)
{
  return ntohl(*((uint32_t*) data));
}

uint64_t _GetParamInt64(void *data)
{
  return htobe64(*((uint64_t*) data));
}

/* parse the received data into command, to be called by SocketServerControl */
int ParseCommand(void *data, int dataLength)
{
  assert(GetCommandLength(data) == dataLength);
  unsigned char* pointer = data;
  
  command_type_t commandType = _GetParamChar8(pointer);
  ++pointer;
  
  zlog_debug(gZlogCategories[ZLOG_COMMAND], "CommandType [%u|%x]", commandType, commandType);
  
  if (IS_COMMAND_PACK(commandType))
  {
    int commandCount = _GetParamChar8(pointer);
    ++pointer;
    
    int i = 0;
    for (i = 0; i < commandCount; ++i)
    {
      command_type_t type = _GetParamChar8(pointer);
      ParseCommand(pointer, GetCommandLength(pointer));
      pointer += GetCommandLength(pointer);
    }
  }
  else
  {
    switch (commandType)
    {
      case CmdSetSingleSwitch:
      {
	uint16_t switchIndex = _GetParamInt16(pointer);
	pointer += 2;
	unsigned char switchStatus = _GetParamChar8(pointer);
	
	RadioNetworkAppendCommand(commandType, switchIndex, switchStatus);	
      }
      break;
      case CmdGetSingleSwitch:
      {
	uint16_t switchIndex = _GetParamInt16(pointer);
	
	RadioNetworkAppendCommand(commandType, switchIndex);	
      }
      break;
      case CmdSetMultipleSwitches:
      {
	uint16_t switchOffset = _GetParamInt16(pointer);
	pointer += 2;
	uint64_t switchBitMask;
	switchBitMask = _GetParamInt64(pointer);
	
	RadioNetworkAppendCommand(commandType, switchOffset, switchBitMask);
      }
      break;
      case CmdGetMultipleSwitches:
      {
	uint16_t switchOffset = _GetParamInt16(pointer);

	RadioNetworkAppendCommand(commandType, switchOffset);
      }
      break;
      case CmdGetNumSwitches:
      {
	RadioNetworkAppendCommand(commandType);
      }
      break;
      case CmdSetSingleSwitchDelay:
      {
	uint16_t switchIndex = _GetParamInt16(pointer);
	pointer += 2;
	unsigned char switchStatus = _GetParamChar8(pointer);
	++pointer;
	uint16_t delay = _GetParamInt16(pointer);
	
	RadioNetworkAppendCommand(commandType, switchIndex, switchStatus, delay);
      }
      break;
      case CmdPassThrough:
      {
	unsigned char dataLength = _GetParamChar8(pointer);
	++pointer;
	
	RadioNetworkAppendCommand(commandType, dataLength, pointer);
      }
      break;
      case CmdSetScheduledTask:
      case CmdGetScheduledTask:
      case CmdDeleteScheduledTask:
      {
	zlog_warn(gZlogCategories[ZLOG_COMMAND], "ParseCommand: unhandled commandType: %x\n", commandType);
      }
      break;
      default:
      {
	zlog_warn(gZlogCategories[ZLOG_COMMAND], "ParseCommand: unhandled commandType: %x\n", commandType);
      }
      break;
    }
  }
  return 0;
}

/* given the address of a command data buffer, return the length of the command data */
int GetCommandLength(void *commandBuffer)
{
  unsigned char* buffer = (unsigned char*)commandBuffer;
  command_type_t commandType = *buffer;
  
  zlog_debug(gZlogCategories[ZLOG_COMMAND], "CommandType [%u|%x]", commandType, commandType);

  int toRet = 0;
  
  /* getting length for command pack is more for debug purposes */
  if (IS_COMMAND_PACK(commandType))
  {
    zlog_debug(gZlogCategories[ZLOG_COMMAND], "COMMAND_PACK");
    int commandCount = *(buffer + 1);
    int i;
    int length;
    char *pointer = buffer + 2;
    /* adding up individual command lengths */
    for (i = 0; i < commandCount; ++i)
    {
      length = GetCommandLength(pointer);
      toRet += length;
      pointer += length;
    }
    /* command header + command count */
    toRet += 2;
  }
  else
  {
    switch (commandType)
    {
      case CmdSetSingleSwitch:
	toRet = 4;
	break;
      case CmdGetSingleSwitch:
	toRet = 3;
	break;
      case CmdSetMultipleSwitches:
	toRet = 11;
	break;
      case CmdGetMultipleSwitches:
	toRet = 2;
	break;
      case CmdGetNumSwitches:
	toRet = 1;
	break;
      case CmdSetSingleSwitchDelay:
	toRet = 6;
	break;
      case CmdPassThrough:
	toRet = 2 + (int)(*(buffer + 1));
	break;
      case CmdSetScheduledTask:
      {
	/* make sure the scheduled task is to set the status of certain switch(es) */
	assert(*(buffer + 1) == CmdSetSingleSwitch || *(buffer + 1) == CmdSetMultipleSwitches);
	toRet = 4 + GetCommandLength((void*)(buffer + 1));
      }
      break;
      case CmdGetScheduledTask:
      case CmdDeleteScheduledTask:
	toRet = 2;
	break;
      default:
      {
//	zlog_warn(gZlogCategories[ZLOG_COMMAND], "GetCommandLength: unhandled commandType: %x\n", commandType);
	toRet = -1;
      }
      break;
	
    }
  }
  
  zlog_debug(gZlogCategories[ZLOG_COMMAND], "GetCommandLength retrieves CommandType %x returning %d\n", commandType, toRet);

  return toRet;
}

