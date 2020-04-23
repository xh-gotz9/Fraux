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

static void parse_result_test(const char *s, int result)
{
    fraux_value v;
    int res = fraux_parse(&v, s);
    ASSERT(res, result);
}

static void test_parse_string()
{
    parse_result_test("2:OK", FRAUX_PARSE_OK);
    parse_result_test("2OK", FRAUX_PARSE_INVALID_VALUE);
    parse_result_test("2", FRAUX_PARSE_MISS_QUOTATION_MARK);
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
