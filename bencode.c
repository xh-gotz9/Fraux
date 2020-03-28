#include <string.h>

#include "bencode.h"

bencode_node *create_node(int type)
{
    bencode_node *node = NULL;

    node = malloc(sizeof(bencode_node));
    if (node == NULL)
    {
        return NULL;
    }
    memset(node, 0, sizeof(bencode_node));

    node->type = type;

    return node;
}