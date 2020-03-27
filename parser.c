#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "bencoding.h"

extern char buf[];

parser_buffer *create_parser_buffer(char *src)
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

bencoding_node *parse_node(parser_buffer *buffer)
{
    int depth = 0;
    bencoding_node *root = NULL,
                   *parent = NULL /* parent 作为双向链表记录每层*/,
                   *dict_tmp = NULL;

    while (buffer->offset < buffer->len)
    {
        char *head = buffer->data + buffer->offset;
        char *tail = head;
        bencoding_node *tmp = NULL;
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
                // end of parent
                if (depth == 1)
                {
                    return root;
                }
                else
                {
                    parent = parent->prev;
                }
                buffer->offset++;
            default:
                // FAILED
                printf("syntax error\n");
                exit(EXIT_FAILURE);
            }
        }

        if (parent == NULL)
        {
            root = parent = tmp;
            depth = 1;
        }
        else
        {
            bencoding_node *node_ptr;
            switch (parent->type)
            {
            case T_LIST:
                node_ptr = parent->list_node_head;
                if (node_ptr == NULL)
                {
                    parent->list_node_head = tmp;
                }
                else
                {
                    while (node_ptr->next)
                    {
                        node_ptr = node_ptr->next;
                    }
                    node_ptr->next = tmp;
                    tmp->prev = node_ptr;
                }
                break;
            case T_DICT:
                if (dict_tmp != NULL)
                {
                    dict_tmp->val = tmp;
                    node_ptr = parent->dict_node_head;
                    if (node_ptr == NULL)
                    {
                        parent->dict_node_head = tmp;
                    }
                    else
                    {
                        while (node_ptr->next != NULL)
                        {
                            node_ptr = node_ptr->next;
                        }
                        node_ptr->next = tmp;
                        tmp->prev = node_ptr;
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
                printf("syntax error\n");
                while (parent != NULL)
                {
                    print_node(parent, NULL);
                    parent = parent->prev;
                }

                // TODO: set error info
                return NULL;
            }
            switch (tmp->type)
            {
            case T_LIST:
            case T_DICT:
                parent->next = tmp;
                tmp->prev = parent;
                parent = tmp;
                break;
            }
        }
    }

    return root;
}

bencoding_node *parse_node_str(parser_buffer *buffer)
{
    char *head = buffer->data + buffer->offset;
    char *tail = head;

    bencoding_node *tmp;
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

bencoding_node *parse_node_num(parser_buffer *buffer)
{
    char *head = buffer->data + buffer->offset;
    char *tail = head;

    bencoding_node *tmp;

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