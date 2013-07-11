#if !defined(__COMMAND_PROTOCOL_H__)
#define __COMMAND_PROTOCOL_H__

/* Instant Commands (mask 11000000) */

#define CmdSetSingleSwitch 0xC1	/* 1100_0001 */
#define CmdGetSingleSwitch 0xC2
#define CmdSetMultipleSwitches 0xC3
#define CmdGetMultipleSwitches 0xC4
#define CmdGetNumSwitches 0xC5
#define CmdSetSingleSwitchDelay 0xC6
#define CmdPassThrough 0xC7

#define COMMAND_INSTANT_MASK 0xC0

#define IS_INSTANT_COMMAND(c) (c ^ COMMAND_INSTANT_MASK == 0)

/* Scheduled Commands (mask 00110000) */
#define CmdSetScheduledTask 0x31
#define CmdGetScheduledTask 0x32
#define CmdDeleteScheduledTask 0x33

#define COMMAND_SCHEDULED_MASK 0x30

#define IS_SCHEDULED_COMMAND(c) (c ^ COMMAND_SCHEDULED_MASK == 0)

/* Multiple command packet header */

#define MultipleCommandHeader 0xAC	/* 1010_1100 */

#define IS_COMMAND_PACK(c) ((~c & MultipleCommandHeader) == 0)

typedef unsigned char command_type_t;

#endif //__COMMAND_PROTOCOL_H__