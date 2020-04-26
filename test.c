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
    fraux_clean(&v);
}

static void parse_result_test(const char *s, size_t len, int result)
{
    fraux_value v;
    int res = fraux_parse(&v, s, len);
    assert(res == result);
    fraux_clean(&v);
}

static void parse_binary_string_test()
{
    fraux_value v;
    fraux_parse(&v, "3:\000ab", 5);
    assert(fraux_get_type(&v) == FRAUX_STRING);
    assert(v.u.s.len == 3);
    assert(memcmp(v.u.s.s, "\000ab", v.u.s.len) == 0);
    fraux_clean(&v);
}

static void test_parse_string()
{
    parse_result_test("2:OK", 4, FRAUX_PARSE_OK);
    parse_result_test("2OK", 3, FRAUX_PARSE_INVALID_VALUE);
    parse_result_test("2", 1, FRAUX_PARSE_MISS_QUOTATION_MARK);
    parse_result_test("3:O\000K", 5, FRAUX_PARSE_OK);
    parse_binary_string_test();
}

static void parse_empty_list_test()
{
    fraux_value v;
    assert(fraux_parse(&v, "le", 2) == FRAUX_PARSE_OK);
    assert(fraux_get_type(&v) == FRAUX_LIST);
    assert(v.u.l.size == 0);
    fraux_clean(&v);
}

static void parse_recursive_list_test()
{
    fraux_value v;
    assert(fraux_parse(&v, "ll2:abe2:cde", 12) == FRAUX_PARSE_OK);
    assert(fraux_get_type(&v) == FRAUX_LIST);
    assert(v.u.l.size == 2);

    assert(fraux_get_type(&v.u.l.e[0]) == FRAUX_LIST);
    assert(v.u.l.e[0].u.l.size == 1);
    fraux_clean(&v);
}

static void test_parse_list()
{
    parse_empty_list_test();
    parse_recursive_list_test();
}

static void parse_empty_dict_test()
{
    fraux_value v;
    assert(fraux_parse(&v, "de", 2) == FRAUX_PARSE_OK);
    assert(fraux_get_type(&v) == FRAUX_DICTIONARY);
    assert(v.u.d.size == 0);
    fraux_clean(&v);
}

static void parse_dict_test()
{
    fraux_value v;

    char *str = "d1:ni2e1:s3:str1:ll2:e12:e22:e32:e4e1:dd2:k12:v12:k22:v2ee";

    assert(fraux_parse(&v, str, strlen(str)) == FRAUX_PARSE_OK);
    assert(fraux_get_type(&v) == FRAUX_DICTIONARY);

    fraux_clean(&v);
}

static void test_parse_dictionary()
{
    parse_empty_dict_test();
    parse_dict_test();
}

static void test_parse()
{
    test_parse_number();
    test_parse_string();
    test_parse_list();
    test_parse_dictionary();
}

int main(int argc, char const *argv[])
{
    test_parse();

    return 0;
}
