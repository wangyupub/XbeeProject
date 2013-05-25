/********************************************************************
 * XBeeInterface.c
 * 
 * Interace between main app and libxbee, handles XBee radio control
 * 
 ********************************************************************/
#include <string.h>

#include "XBeeInterface.h"

struct xbee *xbee;
struct xbee_con *con;
struct xbee_conSettings settings;
struct xbee_conAddress address;

int XBeeRadioInit(const XBeeRadioConfig* xbeeConfig)
{
  xbee_err ret;
  char c;

  zlog_debug(gZlogCategories[ZLOG_XBEE], "Setting up XBee at %s as %s w/ rate %d\n", 
     xbeeConfig->szDevicePath, xbeeConfig->szXBeeMode, xbeeConfig->iBaudRate);
  
  if ((ret = xbee_setup(&xbee, xbeeConfig->szXBeeMode /* "xbee2" */, 
    xbeeConfig->szDevicePath/* "/dev/ttyUSB0" */, xbeeConfig->iBaudRate /* 9600 */)) != XBEE_ENONE)
  {
    zlog_fatal(gZlogCategories[ZLOG_XBEE], "ret: %d (%s)\n", ret, xbee_errorToStr(ret));
    return ret;
  }

  xbee_logLevelSet(xbee, 5);

  
}

int XBeeRadioDestroy()
{
  xbee_err ret;
  if ((ret = xbee_conEnd(con)) != XBEE_ENONE)
  {
    xbee_log(xbee, -1, "xbee_conEnd() returned: %d", ret);
    zlog_fatal(gZlogCategories[ZLOG_XBEE], "xbee_conEnd() returned: %d", ret);
    return ret;
  }

  xbee_shutdown(xbee);

}

int XBeeRadioConnect(XBeeConnectionConfig* conncetionConfig)
{
  xbee_err ret;
/*
  memset(&address, 0, sizeof(address));

  address.addr64_enabled = 1;
  address.addr64[0] = 0x00;
  address.addr64[1] = 0x13;
  address.addr64[2] = 0xA2;
  address.addr64[3] = 0x00;
  address.addr64[4] = 0x40;
  address.addr64[5] = 0x9C;
  address.addr64[6] = 0x27;
  address.addr64[7] = 0xBC;
*/
  if ((ret = xbee_conNew(xbee, &con,
    conncetionConfig->connectionType/* "Data" */, &conncetionConfig->address)) != XBEE_ENONE)
  {
    xbee_log(xbee, -1, "xbee_conNew() returned: %d (%s)", ret, xbee_errorToStr(ret));
    zlog_fatal(gZlogCategories[ZLOG_XBEE], "xbee_conNew() returned: %d (%s)", ret, xbee_errorToStr(ret));
    return ret;
  }

  
  if ((ret = xbee_conDataSet(con, xbee, conncetionConfig->connectionData)) != XBEE_ENONE)
  {
    xbee_log(xbee, -1, "xbee_conDataSet() returned: %d", ret);
    zlog_fatal(gZlogCategories[ZLOG_XBEE], "xbee_conDataSet() returned: %d", ret);
    return ret;
  }

  if ((ret = xbee_conCallbackSet(con, conncetionConfig->xbeeCallback, NULL)) != XBEE_ENONE)
  {
    xbee_log(xbee, -1, "xbee_conCallbackSet() returned: %d", ret);
    zlog_fatal(gZlogCategories[ZLOG_XBEE],  "xbee_conCallbackSet() returned: %d", ret);
    return ret;
  }
  
  return 0;
}


int XBeeRadioDisconnect()
{
  xbee_err ret;
  if ((ret = xbee_conEnd(con)) != XBEE_ENONE)
  {
    xbee_log(xbee, -1, "xbee_conEnd() returned: %d", ret);
    zlog_fatal(gZlogCategories[ZLOG_XBEE],  "xbee_conEnd() returned: %d", ret);
    return ret;
  }
  else
  {
    return 0;
  }
}


int XBeeRadioSend(const unsigned char* data, int len)
{
  xbee_err ret;
  if (ret = xbee_connTx(con, NULL, data, len) != XBEE_ENONE)
  {
    xbee_log(xbee, -1, "xbee_connTx() returned: %s\n", xbee_errorToStr(ret));
    zlog_fatal(gZlogCategories[ZLOG_XBEE],  "xbee_connTx() returned: %d", ret);
    return ret;
  }
  else
  {
    return 0;
  }
}
