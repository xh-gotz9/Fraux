#ifndef __BENCODE_H__
#define __BENCODE_H__

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

#include "dbg.h"

/* data type */
#define T_STR 0
#define T_NUM 1
#define T_LIST 2
#define T_DICT 3
#define T_DICT_NODE 4

typedef struct bencode_node
{
    u_int8_t type;

    /* T_STR node*/
    char *str;

    /* T_NUM node*/
    int number;

    /* T_LIST node */
    struct bencode_node *list_node_head;

    /* T_DICT node */
    struct bencode_node *dict_node_head;

    struct bencode_node *key;
    struct bencode_node *val;

    /* linked list, 用于 T_LIST 和 T_DICT_NODE */
    struct bencode_node *prev;
    struct bencode_node *next;

} bencode_node;

bencode_node *create_node(int type);

int bencode_cmp(const bencode_node *a, const bencode_node *b);

int bencode_list_add(bencode_node *list, bencode_node *node);

int bencode_dict_find(const bencode_node *dict, const bencode_node *key, bencode_node **value);

#endif // __BENCODE_H__