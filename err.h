#ifndef __ERR_H__
#define __ERR_H__

extern int err;

#define FR_SUCCESS 0
#define FR_SYSTEM_ERROR 1
#define FR_SYNTAX_ERROR 2
#define FR_DATA_ERROR 3
#define FR_UNSUPPORT_TYPE 4

#define MAX_ERR_ID 3

extern char *errinfos[];

void seterrinfo(int errid);

char *geterrinfo(int errid);

void perrinfo(char *msg);

#endif