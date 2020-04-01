#ifndef __ERR_H__
#define __ERR_H__

extern int err;

#define FR_SUCCESS 0
#define FR_SYSTEM_ERROR 1
#define FR_SYNTAX_ERROR 2
#define FR_DATA_ERROR 3
#define FR_UNSUPPORT_TYPE 4

#define MAX_ERR_ID 3

char *errinfo[] = {
    [FR_SUCCESS] = "SUCCESS",
    [FR_SYSTEM_ERROR] = "SYSTEM_ERROR",
    [FR_SYNTAX_ERROR] = "SYNTAX_ERROR",
    [FR_DATA_ERROR] = "DATA_ERROR",
    [FR_UNSUPPORT_TYPE] = "UNSUPPORT_TYPE"};

void seterrinfo(int errid);

char *geterrinfo(int errid);

void perrinfo(char *msg);

#endif