#if !defined(__COMMAND_LOGIC_H__)
#define __COMMAND_LOGIC_H__

/* parse the received data into command, to be called by SocketServerControl */
int ParseCommand(void *data, int dataLength);

/* given the address of a command data buffer, return the length of the command data */
int GetCommandLength(void *commandBuffer);

#endif //__COMMAND_LOGIC_H__