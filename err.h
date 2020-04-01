#ifndef __ERR_H__
#define __ERR_H__

extern int err;

#define SUCCESS 0
#define SYSERR 1
#define SNTXERR 2
#define DATAERR 3

#define MAX_ERR_ID 2

char *errinfo[] = {
    [SUCCESS] = "SUCCESS",
    [SYSERR] = "SYSERR",
    [SNTXERR] = "SNTXERR",
    [DATAERR] = "DATAERR",
};

void seterrinfo(int errid);

char *geterrinfo(int errid);

void perrinfo(char *msg);

#endif