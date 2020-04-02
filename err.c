#include "err.h"

#include <stdio.h>

int err;

char *errinfos[] = {
    [FR_SUCCESS] = "SUCCESS",
    [FR_SYSTEM_ERROR] = "SYSTEM_ERROR",
    [FR_SYNTAX_ERROR] = "SYNTAX_ERROR",
    [FR_DATA_ERROR] = "DATA_ERROR",
    [FR_UNSUPPORT_TYPE] = "UNSUPPORT_TYPE"};

/**
 * not thread safety
 **/
void seterrinfo(int errid)
{
    err = errid;
}

char *geterrinfo(int errid)
{
    return errid >= MAX_ERR_ID ? NULL : errinfos[errid];
}

void perrinfo(char *msg)
{
    if (err == FR_SYSTEM_ERROR)
    {
        perror(errinfos[FR_SYSTEM_ERROR]);
    }
    else
    {
        fprintf(stderr, "%s:%s\n", msg, errinfos[err]);
    }
}