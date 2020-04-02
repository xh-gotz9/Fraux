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
#include "err.h"

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

#define ASSERT_RESULT_NOT(CONDITION, RES)         \
    if (CONDITION != RES)                         \
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

int bencode_cmp_test()
{
    bencode_node *a = create_node(T_STR),
                 *b = create_node(T_STR);

    a->type = T_STR;
    b->type = T_STR;
    a->str = "data";
    b->str = "data";

    ASSERT_RESULT(bencode_cmp(a, a), 0); // a == b
    ASSERT_RESULT(bencode_cmp(a, b), 0); //

    b->type = T_NUM;
    b->number = 32;
    // a->type != b->type
    ASSERT_RESULT_NOT(bencode_cmp(a, b), 0);

    a->type = T_NUM;
    a->number = 32;
    ASSERT_RESULT(bencode_cmp(a, b), 0); // T_NUM test
    a->number = 33;
    ASSERT_RESULT_NOT(bencode_cmp(a, b), 0);

    a->type = T_LIST;
    b->type = T_LIST;
    ASSERT_RESULT_NOT(bencode_cmp(a, b), 0); // different address
    ASSERT_RESULT(bencode_cmp(a, a), 0);     // same address

    a->type = T_DICT;
    b->type = T_DICT;
    ASSERT_RESULT_NOT(bencode_cmp(a, b), 0); // different address
    ASSERT_RESULT(bencode_cmp(a, a), 0);     // same address

    return 0;
}

int dict_find_test()
{
    // test
    bencode_node *dict = create_node(T_DICT);

    char *key_set[] = {"key1", "key2", "key3"};
    char *val_set[] = {"val1", "val2", "val3"};
    bencode_node *node, *key, *val;
    int i = 0;
    do
    {
        node = create_node(T_DICT_NODE);

        key = create_node(T_STR);
        key->str = key_set[i];

        val = create_node(T_STR);
        val->str = val_set[i];

        node->key = key;
        node->val = val;

        if (dict->dict_node_head == NULL)
        {
            dict->dict_node_head = node;
        }
        else
        {
            bencode_node *target = dict->dict_node_head;

            while (target->next != 0)
            {
                target = target->next;
            }

            target->next = node;
            node->prev = target;
        }

        i++;
    } while (i < 3);

    bencode_node *res;

    bencode_dict_find(dict, "key3", &res);
    ASSERT_RESULT_NOT(res, NULL);

    bencode_dict_find(dict, "key4", &res);
    ASSERT_RESULT(res, NULL);

    print_bencode_node(val, NULL);

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
    char *data = "d4:datal3:cow3:moo4:spam4:eggse3:keyi2ee";
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
    for (size_t i = 0; i < 4; i++)
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

    return strcmp(print_bencode_node(root, 0),
                  "{\"data\":[\"cow\",\"moo\",\"spam\",\"eggs\"],\"key\":2}") == 0
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

int parse_data_from_file()
{
    int fd = open("./test.torrent", O_RDWR);
    if (fd == -1)
    {
        perror("open error");
        exit(EXIT_FAILURE);
    }
    struct stat stat;
    if (fstat(fd, &stat) == -1)
    {
        perror("fstat error");
        exit(EXIT_FAILURE);
    }
    off_t len = stat.st_size;
    char *ptr = mmap(NULL, (size_t)len + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

    ptr[len] = 0;

    parser_buffer *buffer = create_parser_buffer(ptr);
    bencode_node *node = parse_node(buffer);
    munmap(ptr, len + 1);

    if (print_bencode_node(node, 0) != NULL)
    {
        return 0;
    }
    return -1;
}

// err.h
int parse_err_test()
{
    seterrinfo(FR_SYNTAX_ERROR);
    ASSERT_RESULT(err, FR_SYNTAX_ERROR);

    seterrinfo(FR_DATA_ERROR);
    ASSERT_RESULT(err, FR_DATA_ERROR);

    perrinfo("parse_err_test");

    return 0;
}

#define PARSE_DATA_TEST(DATA, CHECK) parse_data_test(DATA, CHECK)

int main(int argc, char *argv[])
{
    ASSERT_RESULT(create_node_test(), 0);
    ASSERT_RESULT(bencode_cmp_test(), 0);
    ASSERT_RESULT(dict_find_test(), 0);

    ASSERT_RESULT(write_print_buffer_test(), 0);
    ASSERT_RESULT(print_node_test(), 0);

    ASSERT_RESULT(PARSE_DATA_TEST("i1220e", "1220"), 0);
    ASSERT_RESULT(PARSE_DATA_TEST("5:abcde", "\"abcde\""), 0);
    ASSERT_RESULT(PARSE_DATA_TEST("l3:cow3:moo4:spam4:eggse", "[\"cow\",\"moo\",\"spam\",\"eggs\"]"), 0);
    ASSERT_RESULT(PARSE_DATA_TEST("d3:key3:vale", "{\"key\":\"val\"}"), 0);

    ASSERT_RESULT(parse_data_from_file(), 0);

    ASSERT_RESULT(parse_err_test(), 0);

    return 0;
}
