#if !defined(__RADIO_NETWORK_H__)
#define __RADIO_NETWORK_H__

#include "Util.h"

#include "CommandProtocol.h"


/* Initializes Radio Network module */
void RadioNetworkInit(RadioNetworkConfig* config);

/* Clean up Radio Network module */
void RadioNetworkDestroy();

/* Thread entry function for command queue processing, now handles instant command */
void* RadioNetworkProcessCommandQueue(void *arg);

/* Append command to Radio Network command queue */
void RadioNetworkAppendCommand(command_type_t commandType, ...);

/* Fill in the buffer with returning data, function returns the data size */
int RadioNetworkGetReturnData(void* buffer);

#endif //__RADIO_NETWORK_H__