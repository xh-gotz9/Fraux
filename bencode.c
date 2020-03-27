#include <string.h>

#include "bencode.h"

bencode_node *create_node(int type)
{
    bencode_node *node = NULL;

    node = malloc(sizeof(bencode_node));
    node->type = type;

    return node;
}

char *print_node(bencode_node *node, char *dest)
{
    int mode = 1;
    if (node == NULL)
    {
        mode = 0;
    }

    switch (node->type)
    {
    case T_STR:
        printf("str: %s\n", node->str);
        break;
    case T_NUM:
        printf("number: %d\n", node->number);
    case T_LIST:
        printf("list: []");
        break;
    default:
        printf("unsupport type: %d\n", node->type);
        return NULL;
    }
}