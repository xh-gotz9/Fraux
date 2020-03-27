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
    bencoding_node *root = NULL, *parent = NULL /* parent 作为双向链表记录每层*/, *dict_tmp = NULL;

    while (buffer->offset < buffer->len)
    {
        char *head = buffer->data + buffer->offset;
        char *tail = head;
        bencoding_node *tmp = NULL;
        if (*head >= '0' && *head <= '9') // 匹配数字
        {
            // T_STR
            tmp = parse_node_str(buffer);
        }
        else // 匹配其他
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
                break;
            }
        }

        // TODO add to parrent
        if (parent == NULL)
        {
            root = parent = tmp;
            depth = 1;
        }
        else
        {
            bencoding_node *h;
            // 将 node 放入 parent
            switch (parent->type)
            {
            case T_LIST:
                h = parent->list_node_head;
                if (h == NULL)
                {
                    parent->list_node_head = tmp;
                }
                else
                {
                    while (h->next)
                    {
                        h = h->next;
                    }
                    h->next = tmp;
                    tmp->prev = h;
                }
                break;
            case T_DICT:
                if (dict_tmp != NULL)
                {
                    dict_tmp->val = tmp;
                    h = parent->dict_node_head;
                    if (h == NULL)
                    {
                        parent->dict_node_head = tmp;
                    }
                    else
                    {
                        while (h->next != NULL)
                        {
                            h = h->next;
                        }
                        h->next = tmp;
                        tmp->prev = h;
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

                // TODO set error info
                return NULL;
            }
            switch (tmp->type)
            {
            case T_LIST:
            case T_DICT:
                // 新一层
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
    // 扫描 ':'
    while (*tail != ':')
    {
        tail++;
    }

    // read length of str
    strncpy(buf, head, tail - head);
    buf[tail - head] = 0;
    int val;
    sscanf(buf, "%d", &val);

    char *data = malloc(sizeof(char) * (val + 1));
    strncpy(data, tail + 1, val);
    data[val] = 0;

    // create node
    tmp = create_node(T_STR);
    tmp->str = data;

    // set offset
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