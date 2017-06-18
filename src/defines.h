#pragma once

#ifdef _WIN32
#define NL "\r\n"
#else
#define NL "\n"
#endif

#ifdef _DEBUG
#include <cstdio>
#include <cstring>
#define DEBUGLOGLN(s) ShowConsoleMsg(s NL)
#define DEBUGLOG(s) ShowConsoleMsg(s)
#define DEBUGPRINTF(fmt,...) \
  do{ \
    char buf[512];  \
    snprintf(buf, 511, fmt, __VA_ARGS__);  \
    ShowConsoleMsg(buf); \
  }while(0)
#else
#define DEBUGLOGLN(s)
#define DEBUGLOG(s)
#define DEBUGPRINTF(fmt,...)
#endif