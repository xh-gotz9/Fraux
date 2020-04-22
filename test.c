#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define ASSERT(CONDITION, RES)                                 \
    do                                                         \
    {                                                          \
        if (CONDITION == RES)                                  \
        {                                                      \
            fprintf(stderr, "%s test success!\n", #CONDITION); \
        }                                                      \
        else                                                   \
        {                                                      \
            fprintf(stderr, "%s test failed!\n", #CONDITION);  \
            exit(EXIT_FAILURE);                                \
        }                                                      \
    } while (0);

#define ASSERT_NOT(CONDITION, RES)                             \
    do                                                         \
    {                                                          \
        if (CONDITION != RES)                                  \
        {                                                      \
            fprintf(stderr, "%s test success!\n", #CONDITION); \
        }                                                      \
        else                                                   \
        {                                                      \
            fprintf(stderr, "%s test failed!\n", #CONDITION);  \
            exit(EXIT_FAILURE);                                \
        }                                                      \
    } while (0);

int main(int argc, char const *argv[])
{
    ASSERT(0, 0);
    ASSERT_NOT(0, -1);

    return 0;
}
