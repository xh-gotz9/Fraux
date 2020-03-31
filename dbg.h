#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>

#ifdef DEBUG
#define LOG(format, ...) fprintf(stderr, format, ##__VA_ARGS__);
#else
#define LOG(format, ...)
#endif

#define LOG_DBG(info_str) LOG("file: %s, line: %d - %s\n", __FILE__, __LINE__, info_str);

#endif