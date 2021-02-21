#pragma once
#ifndef helpers_h
#define helpers_h

#include "ESPTelnet.h"

extern ESPTelnet telnet;

String s_uptime();

bool   setup_WiFi(int i_cntdown=100);
void   SHT_Report();
void   BMP_Report();
void   AXP_Report();
void   dirFiles(String s_path);
void   cat_file(String s_fn);
bool   wget_to_LITTLEFS_HTTP( String s_host, String s_path, String s_fnlocal="");
bool   wget_to_LITTLEFS_HTTPS(String s_host, String s_path, String s_fnlocal="");
void   drawBMP565(String s_fn,int xo,int yo,float f_zoom=1.0);
void   drawLocalTime();

#endif
