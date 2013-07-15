#if !defined(__UTIL_H__)
#define __UTIL_H__

#include <stdint.h>
#include "zlog.h"
#include "ini.h"

#define CONFIG_STRING_LEN 32

/* Definition of zlog categories */
typedef enum
{
  ZLOG_MAIN = 0,
  ZLOG_SOCKET,
  ZLOG_XBEE,
  ZLOG_COMMAND,
  ZLOG_TOTAL
} ZLOG_CATEGORIES;

/* Settings needed to set up XBee */
typedef struct
{
  char*		szXBeeMode;
  char*		szDevicePath;
  int		iBaudRate;
  
} XBeeRadioConfig;

/* only star is supported */
typedef enum
{
  NetworkInvalid = -1,
  NetworkPair = 0,
  NetworkStar,
  NetworkMesh,
  NetworkClusterTree,
} NetworkTopology;

/* Settings to set up Radio Network */
typedef struct
{
  int 		endPointCount;
  int		switchCountPerEndPoint;
} StarNetworkConfig;

typedef struct
{
  union
  {
    /* seems a bit redundant for now, new supported network topologies go here*/
    StarNetworkConfig 	starConfig;
  } detailConfig;
  NetworkTopology	topology;
  int			commandPoolSize;
  uint32_t*		addressBuffer;
} RadioNetworkConfig;

/* Struct holding configuration parsed from ini file */
typedef struct
{
  /* Section SocketServer */
  int			uServerPort;
  
  /* Section XBeeControl */
  XBeeRadioConfig	xbeeRadioConfig;
  
  /* Section Radio Network */
  RadioNetworkConfig	radioNetworkConfig;
  
} AppConfig;


/* Initializes zlog, sets up categories */
int zLogInit();

/* Destroys zlog */
int zLogDestroy();

/* Global zlog category pionters */
zlog_category_t *gZlogCategories[ZLOG_TOTAL];

/* Function for inih library, callback when parsing the ini file. */
int iniHandler(void* user, const char* section, const char* name, const char* value);


#endif //__UTIL_H__