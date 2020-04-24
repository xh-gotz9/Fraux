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
    fraux_parse(&v, "i3e", 3);
    ASSERT(fraux_get_type(&v), FRAUX_NUMBER);
    ASSERT(FRAUX_PARSE_MISS_QUOTATION_MARK, fraux_parse(&v, "i3", 2));
    ASSERT(FRAUX_PARSE_INVALID_VALUE, fraux_parse(&v, "i3d", 3));
}

static void parse_result_test(const char *s, size_t len, int result)
{
    fraux_value v;
    int res = fraux_parse(&v, s, len);
    ASSERT(res, result);
}

static void test_parse_string()
{
    parse_result_test("2:OK", 4, FRAUX_PARSE_OK);
    parse_result_test("2OK", 3, FRAUX_PARSE_INVALID_VALUE);
    parse_result_test("2", 1, FRAUX_PARSE_MISS_QUOTATION_MARK);
    parse_result_test("3:O\000K", 5, FRAUX_PARSE_OK);
}

static void test_parse()
{
    test_parse_number();
    test_parse_string();
}

int main(int argc, char const *argv[])
{
    ASSERT(0, 0);
    ASSERT_NOT(0, -1);

    test_parse();

    return 0;
}
