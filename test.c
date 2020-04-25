#include "test.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "fraux.h"

static void test_parse_number()
{
    fraux_value v;
    fraux_parse(&v, "i3e", 3);
    assert(fraux_get_type(&v) == FRAUX_NUMBER);
    assert(FRAUX_PARSE_MISS_QUOTATION_MARK == fraux_parse(&v, "i3", 2));
    assert(FRAUX_PARSE_INVALID_VALUE == fraux_parse(&v, "i3d", 3));
}

static void parse_result_test(const char *s, size_t len, int result)
{
    fraux_value v;
    int res = fraux_parse(&v, s, len);
    ASSERT(res, result);
}

static void parse_binary_string_test()
{
    fraux_value v;
    fraux_parse(&v, "3:\000ab", 5);
    assert(fraux_get_type(&v) == FRAUX_STRING);
    assert(v.u.s.len == 3);
    assert(memcmp(v.u.s.s, "\000ab", v.u.s.len) == 0);
}

static void test_parse_string()
{
    parse_result_test("2:OK", 4, FRAUX_PARSE_OK);
    parse_result_test("2OK", 3, FRAUX_PARSE_INVALID_VALUE);
    parse_result_test("2", 1, FRAUX_PARSE_MISS_QUOTATION_MARK);
    parse_result_test("3:O\000K", 5, FRAUX_PARSE_OK);
    parse_binary_string_test();
}

static void test_parse()
{
    test_parse_number();
    test_parse_string();
}

int main(int argc, char const *argv[])
{
    test_parse();

    return 0;
}
