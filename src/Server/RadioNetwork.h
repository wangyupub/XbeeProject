#if !defined(__RADIO_NETWORK_H__)
#define __RADIO_NETWORK_H__

#include "Util.h"

void RadioNetworkInit(RadioNetworkConfig* config);

void RadioNetworkDestroy();

/* thread entry function for command queue processing, now handles instant command */
void* RadioNetworkProcessCommandQueue(void *arg);


#endif //__RADIO_NETWORK_H__