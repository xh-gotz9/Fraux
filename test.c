#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "fraux.h"

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

static void test_parse_number()
{
    fraux_value v;
    fraux_parse(&v, "i3e");
    ASSERT(fraux_get_type(&v), FRAUX_NUMBER);
    ASSERT(FRAUX_PARSE_MISS_QUOTATION_MARK, fraux_parse(&v, "i3"));
    ASSERT(FRAUX_PARSE_INVALID_VALUE, fraux_parse(&v, "i3d"));
}

static void test_parse()
{
    test_parse_number();
}

int main(int argc, char const *argv[])
{
    ASSERT(0, 0);
    ASSERT_NOT(0, -1);

    test_parse();

    return 0;
}
