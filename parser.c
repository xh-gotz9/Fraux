#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "bencode.h"

char buf[BUFSIZ + 1];

static bencode_node *parse_node_str(parser_buffer *buffer);

static bencode_node *parse_node_num(parser_buffer *buffer);

parser_buffer *create_parser_buffer(const char *src)
{
    if (src == NULL)
    {
        return NULL;
    }

    parser_buffer *buffer = malloc(sizeof(parser_buffer));
    buffer->data = src;
    buffer->len = strlen(src);
    buffer->offset = 0;

    return buffer;
}

typedef struct bencode_node_list_node
{
    bencode_node *data;
    struct bencode_node_list_node *prev;
    struct bencode_node_list_node *next;
} bencode_node_list_node;

bencode_node_list_node *create_list_node()
{
    bencode_node_list_node *node = malloc(sizeof(bencode_node_list_node));
    if (node == NULL)
    {
        LOG_DBG("malloc error");
        return NULL;
    }
    memset(node, 0, sizeof(bencode_node_list_node));

    return node;
}

bencode_node *parse_node(parser_buffer *buffer)
{
    int depth = 0;
    bencode_node *dict_tmp = NULL; /* dic_tmp 记录解析时未存入 DICT 的 DICT_NODE 节点*/

    bencode_node_list_node *root = NULL,
                           *parent = NULL; /* parent 作为双向链表记录每层*/

    while (buffer->offset < buffer->len)
    {
        const char *head = buffer->data + buffer->offset,
                   *tail = head;
        bencode_node *tmp = NULL;
        if (*head >= '0' && *head <= '9')
        {
            // T_STR
            tmp = parse_node_str(buffer);
        }
        else
        {
            switch (*head)
            {
            case 'i':
                // T_NUM
                tmp = parse_node_num(buffer);
                break;
            case 'l':
                // T_LIST
                tmp = create_node(T_LIST);
                buffer->offset++;
                break;
            case 'd':
                // T_DICT
                tmp = create_node(T_DICT);
                buffer->offset++;
                break;
            case 'e':
                // end of parent "LIST" or "DICT"
                if (depth == 1)
                {
                    return root->data;
                }
                else
                {
                    parent = parent->prev;

                    // TODO free memory
                    parent->next->prev = NULL;

                    parent->next = NULL;
                }
                buffer->offset++;
                depth--;
                continue;
            default:
                LOG_DBG("syntax error");
                // TODO set error info: syntax error
                return NULL;
            }
        }

        if (parent == NULL)
        {
            root = parent = create_list_node();
            parent->data = tmp;
            depth = 1;
        }
        else
        {
            bencode_node *node_ptr;
            switch (parent->data->type)
            {
            case T_LIST:
                bencode_list_add(parent->data, tmp);
                break;
            case T_DICT:
                if (dict_tmp != NULL)
                {
                    dict_tmp->val = tmp;

                    node_ptr = parent->data->dict_node_head;
                    if (node_ptr == NULL)
                    {
                        parent->data->dict_node_head = dict_tmp;
                    }
                    else
                    {
                        while (node_ptr->next != NULL)
                        {
                            node_ptr = node_ptr->next;
                        }
                        node_ptr->next = dict_tmp;
                        dict_tmp->prev = node_ptr;
                    }
                    dict_tmp = NULL;
                }
                else
                {
                    dict_tmp = create_node(T_DICT_NODE);
                    dict_tmp->key = tmp;
                }
                break;
            case T_STR:
            case T_NUM:
            default:
                LOG_DBG("syntax error");
                // TODO: set error info: syntax error
                return NULL;
            }

            switch (tmp->type)
            {
            case T_LIST:
            case T_DICT:
                parent->next = create_list_node();
                parent->next->data = tmp;

                parent->next->prev = parent;
                parent = parent->next;
                depth++;
                break;
            }
        }
    }

    return root->data;
}

static bencode_node *parse_node_str(parser_buffer *buffer)
{
    const char *head = buffer->data + buffer->offset,
               *tail = head;

    bencode_node *tmp;
    while (*tail != ':')
    {
        tail++;
    }

    strncpy(buf, head, tail - head);
    buf[tail - head] = 0;
    int val;
    sscanf(buf, "%d", &val);

    char *data = malloc(sizeof(char) * (val + 1));
    strncpy(data, tail + 1, val);
    data[val] = 0;

    tmp = create_node(T_STR);
    tmp->str = data;

    buffer->offset = (tail + 1 + val) - buffer->data;

    return tmp;
}

static bencode_node *parse_node_num(parser_buffer *buffer)
{
    const char *head = buffer->data + buffer->offset,
               *tail = head;

    bencode_node *tmp;

    while ((*tail) != 'e')
    {
        tail++;
    }
    strncpy(buf, head, (tail - head + 1));

    int num;
    sscanf(buf, "i%de", &num);

    tmp = create_node(T_NUM);
    tmp->number = num;

    buffer->offset = tail - buffer->data + 1;

    return tmp;
}