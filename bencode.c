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

/**
 * compare node
 **/
int bencode_cmp(const bencode_node *a, const bencode_node *b)
{
    // 地址相同
    if (a == b)
    {
        return 0;
    }

    // 类型不同
    if (a->type != b->type)
    {
        return a->type - b->type;
    }

    // 根据类型进行比较
    switch (a->type)
    {
    case T_STR:
        return strcmp(a->str, b->str);
    case T_NUM:
        return a->number - b->number;

    // TODO 讨论是否如何进行比较
    case T_DICT_NODE:
    // 仅支持地址比较
    case T_LIST:
    case T_DICT:
        return a - b;
    default:
        LOG_DBG("unsupport type");
        // TODO error unsupport type
        break;
    }

    return 0;
}

int bencode_list_add(bencode_node *list, bencode_node *node)
{
    if (list == NULL || !(list->type != T_LIST || list->type != T_DICT) || node == NULL)
    {
        LOG_DBG("var error");
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

int bencode_dict_add()
{
    return 0;
}

int bencode_dict_find(const bencode_node *dict, const bencode_node *key, bencode_node **value)
{
    // TODO check type
    // key's type always be T_STR

    bencode_node *head = dict->dict_node_head,
                 *ptr = head;

    if (ptr == NULL)
    {
        *value = NULL;
        return 0;
    }

    do
    {
        if (bencode_cmp(ptr->key, key) == 0)
        {
            *value = ptr->val;
            return 0;
        }
        else
        {
            ptr = ptr->next;
        }
    } while (ptr != NULL);

    *value = NULL;
    return 0;
}
