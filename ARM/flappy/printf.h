#ifndef _PRINTF_H_
#define _PRINTF_H_

#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>


void pr(void (*purcharF)(char), const char* fmtStr, ...);

#define prDebug(...)	pr(prPutcharDebug, __VA_ARGS__)
#define prScreen(...)	pr(prPutcharScreen, __VA_ARGS__)





#endif

