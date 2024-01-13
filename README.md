# WebRadio
Simple internet radio with webinterface, from Example project

Create a file "WiFi_SSID.h"
/*
**  PE0FKO SSID's bereikbaar.
**  -------------------------
**  V0.1  07/12/2023
*/

// WiFi AP to access
static struct WifiApList_t {
  const char    *ssid, *passwd;
} WifiApList[]
{ { "SSID_01_NAME",	  "SSID_01_PASSWORD" }
, { "SSID_02_NAME",	  "SSID_02_PASSWORD" }
, { ........ }
, { "SSID_NN_NAME",	  "SSID_NN_PASSWORD" }
, { NULL, NULL }
};
const int   WifiApListNumber = sizeof(WifiApList) / sizeof(WifiApList[0]) - 1;
