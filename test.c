#define _GNU_SOURCE

// test realloc buffer
#define _BASE_BUFFER_SIZE 1

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <string.h>

#include "bencode.h"
#include "parser.h"
#include "printer.h"

#define ASSERT_RESULT(CONDITION, RES)             \
    if (CONDITION == RES)                         \
    {                                             \
        printf("%s test success!\n", #CONDITION); \
    }                                             \
    else                                          \
    {                                             \
        printf("%s test failed!\n", #CONDITION);  \
        exit(EXIT_FAILURE);                       \
    }

// bencode.h tests
int create_node_test()
{
    bencode_node *node = create_node(T_STR);
    if (node->type != T_STR)
    {
        printf("create node error");
        return -1;
    }
    return 0;
}

// printer.h tests
int write_print_buffer_test()
{
    print_buffer *buffer = create_print_buffer();
    int cnt = 0;

    write_print_buffer(buffer, "data", 4);
    write_print_buffer(buffer, "data", 4);

    return strcmp(buffer->buf, "datadata") == 0 ? 0 : -1;
}

int write_print_buffer_realloc_test()
{
    print_buffer *buffer = create_print_buffer();
    int cnt = 0;
    while (buffer->size < BASE_BUFFER_SIZE)
    {
        write_print_buffer(buffer, "data", 4);
        cnt++;
    }
    return buffer->size > 4 * cnt ? 0 : -1;
}

int print_node_test()
{
    char *data = "d4:datal3:cow3:moo4:spam4:eggsee3:keyi2e";
    // {"data":["cow", "moo", "spam","eggs"]}
    bencode_node *root = create_node(T_DICT),
                 *dict_node = create_node(T_DICT_NODE),
                 *tmp = NULL,
                 *tail = NULL;

    root->dict_node_head = dict_node;

    tmp = create_node(T_STR);
    tmp->str = "data";
    dict_node->key = tmp;

    tmp = create_node(T_LIST);
    char *strs[] = {"cow", "moo", "spam", "eggs"};
    for (size_t i = 0; i < 3; i++)
    {
        bencode_node *n = create_node(T_STR);
        n->str = strs[i];

        if (tmp->list_node_head == NULL)
        {
            tmp->list_node_head = n;
            tail = n;
        }
        else
        {
            tail->next = n;
            n->prev = tail;
            tail = n;
        }
    }
    dict_node->val = tmp;

    // "key":2
    dict_node->next = create_node(T_DICT_NODE);
    dict_node->next->prev = dict_node;
    dict_node = dict_node->next;

    dict_node->key = create_node(T_STR);
    dict_node->key->str = "key";

    dict_node->val = create_node(T_NUM);
    dict_node->val->number = 2;

    parser_buffer *buffer = create_parser_buffer(data);

    bencode_node *node = parse_node(buffer);

    return strcmp(print_bencode_node(node, 0),
                  "{\"data\":[\"cow\",\"moo\",\"spam\",\"eggs\"]}") == 0
               ? 0
               : -1;
}

// parser.h tests
static int parse_data_test(const char *data, const char *res)
{
    parser_buffer *buffer = create_parser_buffer(data);
    bencode_node *node = parse_node(buffer);
    if (!node)
    {
        return -1;
    }

    return strcmp(print_bencode_node(node, 0), res) == 0 ? 0 : -1;
}

#define PARSE_DATA_TEST(DATA, CHECK) parse_data_test(DATA, CHECK)

int main(int argc, char *argv[])
{
    ASSERT_RESULT(create_node_test(), 0);

    ASSERT_RESULT(write_print_buffer_test(), 0);
    ASSERT_RESULT(print_node_test(), 0);

    ASSERT_RESULT(PARSE_DATA_TEST("i1220e", "1220"), 0);
    ASSERT_RESULT(PARSE_DATA_TEST("5:abcde", "\"abcde\""), 0);
    ASSERT_RESULT(PARSE_DATA_TEST("l3:cow3:moo4:spam4:eggse", "[\"cow\",\"moo\",\"spam\",\"eggs\"]"), 0);
    ASSERT_RESULT(PARSE_DATA_TEST("d3:key3:vale", "{\"key\":\"val\"}"), 0);

    return 0;
}
