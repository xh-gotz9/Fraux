#ifndef __BENCODING_H__
#define __BENCODING_H__

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

/* data type */
#define T_STR 0
#define T_NUM 1
#define T_LIST 2
#define T_DICT 3
#define T_DICT_NODE 4

typedef struct bencoding_node
{
    u_int8_t type;

    /* for T_STR node*/
    char *str;

    /* for T_NUM node*/
    int number;

    /* for T_LIST node */
    struct bencoding_node *list_node_head;

    /* for T_DICT node */
    struct bencoding_node *dict_node_head;

    /* for list*/
    struct bencoding_node *prev;
    struct bencoding_node *next;

    // for dict
    struct bencoding_node *key;
    struct bencoding_node *val;

} bencoding_node;

bencoding_node *create_node(int type);

bencoding_node *parse_data(const char *data);

char *print_node(bencoding_node *node, char *dest);

#endif /* __BENCODING_H__ */