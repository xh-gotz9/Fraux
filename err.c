#include "err.h"

#include <stdio.h>

int err;

char *errinfo[] = {
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
    return errid >= MAX_ERR_ID ? NULL : errinfo[errid];
}

void perrinfo(char *msg)
{
    if (err == FR_SYSTEM_ERROR)
    {
        perror(errinfo[FR_SYSTEM_ERROR]);
    }
    else
    {
        fprintf(stderr, "%s:%s\n", msg, errinfo[err]);
    }
}