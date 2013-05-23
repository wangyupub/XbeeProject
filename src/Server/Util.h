#if !defined(__UTIL_H__)
#define __UTIL_H__

#include "zlog.h"
#include "ini.h"

/* Definition of zlog categories */
typedef enum
{
  ZLOG_MAIN = 0,
  ZLOG_SOCKET,
  ZLOG_XBEE,
  ZLOG_COMMAND,
  ZLOG_TOTAL
} ZLOG_CATEGORIES;

/* Struct holding configuration parsed from ini file */
typedef struct
{
  /* Section SocketServer */
  int		uServerPort;
  
  /* Section XBeeControl */
  
} AppConfig;


/* Log config file name */
static const char* LOG_CONFIG_FILENAME = "zlog.conf";

/* INI file name */
static const char* INI_FILENAME = "config.ini";

/* Initializes zlog, sets up categories */
int zLogInit();

/* Global zlog category pionters */
zlog_category_t *gZlogCategories[ZLOG_TOTAL];

/* Function for inih library, callback when parsing the ini file. */
int iniHandler(void* user, const char* section, const char* name, const char* value);


#endif //__UTIL_H__