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

int bencode_list_add(bencode_node *list, bencode_node *node)
{
    if (list == NULL || !(list->type != T_LIST || list->type != T_DICT_NODE) || node == NULL)
    {
#ifdef DEBUG
        perror("bencode list add error");
#endif
        return -1;
    }

    if (list->list_node_head == NULL)
    {
        list->list_node_head = node;
        return 0;
    }

    bencode_node *tail = list->list_node_head;
    while (tail->next != NULL)
    {
        tail = tail->next;
    }
    tail->next = node;
    node->prev = tail;

    return 0;
}