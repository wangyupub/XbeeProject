/********************************************************************
 * CommandLogic.c
 * 
 * Low level command processors, interpreting command from received
 * data buffer.
 * 
 ********************************************************************/
#include <assert.h>
#include "Util.h"

#include "CommandProtocol.h"
#include "CommandLogic.h"



/* parse the received data into command, to be called by SocketServerControl */
int ParseCommand(void *data, int dataLength)
{
  assert(GetCommandLength(data) == dataLength);
  
  
}

/* given the address of a command data buffer, return the length of the command data */
int GetCommandLength(void *commandBuffer)
{
  char* buffer = (char*)commandBuffer;
  unsigned char commandType = *buffer;
  
  int toRet = 0;
  
  /* getting length for command pack is more for debug purposes */
  if (IS_COMMAND_PACK(commandType))
  {
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
	toRet = 10;
	break;
      case CmdGetMultipleSwitches:
	toRet = 2;
	break;
      case CmdGetNumSwitches:
	toRet = 1;
	break;
      case CmdSetSingleSwitchDelay:
	toRet = 5;
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
	zlog_warn(gZlogCategories[ZLOG_COMMAND], "GetCommandLength: unhandled commandType: %x\n", commandType);
	toRet = -1;
      }
	break;
	
    }
  }
  
  zlog_debug(gZlogCategories[ZLOG_COMMAND], "GetCommandLength retrieves CommandType %x returning %d\n", commandType, toRet);

  return toRet;
}

