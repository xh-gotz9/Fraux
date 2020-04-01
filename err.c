#include "err.h"

#include <stdio.h>

int err;

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