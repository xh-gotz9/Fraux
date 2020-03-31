#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <error.h>

#include "printer.h"

char buf[BUFSIZ + 1];

static int realloc_print_buffer(print_buffer *buffer, size_t target);

/**
 * 检查 buffer, buffer->magic 以及 buffer->buf.
 **/
static void ensure(print_buffer *buffer)
{
    if (buffer != NULL && buffer->magic == PRINT_BUFFER_MAGICNUM && buffer->buf != NULL)
    {
        return;
    }
    LOG_DBG("WARNING - try to destroy a memory which is not \"struct print_buffer\" data.\n");
}

print_buffer *create_print_buffer()
{
    print_buffer *buffer = malloc(sizeof(print_buffer));
    if (buffer == NULL)
    {
        LOG_DBG(strerror(errno));
        return NULL;
    }

    /* 每次为 buf 预留一位终止符号位 */
    size_t buf_size = BASE_BUFFER_SIZE;
    char *ptr = malloc(sizeof(char) * (buf_size + 1));
    if (ptr == NULL)
    {
        LOG_DBG(strerror(errno));
        return NULL;
    }

    memset(ptr, 0, sizeof(char) * (buf_size + 1));
    buffer->buf = ptr;

    buffer->length = 0;
    buffer->size = BASE_BUFFER_SIZE;
    buffer->magic = PRINT_BUFFER_MAGICNUM;

    return buffer;
}

int destroy_print_buffer_info(print_buffer *buffer, char **str)
{
    ensure(buffer);

    *str = buffer->buf;

    free(buffer);
}

int destroy_print_buffer_all(print_buffer *buffer)
{
    ensure(buffer);

    char *ptr = buffer->buf;
    if (destroy_print_buffer_info(buffer, 0) == -1)
    {
        return -1;
    }

    free(ptr);
    return 0;
}

print_buffer *write_print_buffer(print_buffer *buffer, void *data, size_t len)
{
    ensure(buffer);

    char *buf = buffer->buf;
    size_t offset = buffer->length;
    size_t size = buffer->size;

    if (offset + len > size)
    {
        int res = realloc_print_buffer(buffer, offset + len);
    }

    strncpy(buffer->buf + buffer->length, data, len);
    buffer->length += len;
    /* 添加终止符号 */
    buffer->buf[buffer->length] = 0;

    return buffer;
}

static int realloc_print_buffer(print_buffer *buffer, size_t target)
{
    ensure(buffer);

    int new_size = buffer->size;

    while (new_size <= target)
    {
        new_size <<= 1;
    }

    char *ptr = realloc(buffer->buf, new_size + 1);
    if (ptr == NULL)
    {
        LOG_DBG("realloc error");
        LOG_DBG(strerror(errno));
        return -1;
    }

    buffer->buf = ptr;
    buffer->size = new_size;

    return new_size;
}

const char *out_print_buffer(print_buffer *buffer)
{
    ensure(buffer);
    printf("%s\n", buffer->buf);
    return buffer->buf;
}

static int print_bencode_list_node(bencode_node *list, print_buffer *buffer);

static int print_bencode_dict_node(bencode_node *dict, print_buffer *buffer);

char *print_bencode_node(bencode_node *node, print_buffer *out)
{
    print_buffer *buffer;
    if (out == NULL)
    {
        buffer = create_print_buffer();
    }
    else
    {
        ensure(out);
        buffer = out;
    }

    switch (node->type)
    {
    case T_STR:
        if (node->str != NULL)
        {
            sprintf(buf, "\"%s\"", node->str);
        }
        else
        {
            sprintf(buf, "\"\"");
        }
        write_print_buffer(buffer, buf, strlen(buf));
        break;
    case T_NUM:
        sprintf(buf, "%d", node->number);
        write_print_buffer(buffer, buf, strlen(buf));
        break;
    case T_LIST:
        print_bencode_list_node(node, buffer);
        break;
    case T_DICT:
        print_bencode_dict_node(node, buffer);
        break;
    default:
        // error: 非法数据
        LOG_DBG("error data with unknown type");
        return NULL;
    }
    if (out == NULL)
    {
        char *data;
        destroy_print_buffer_info(buffer, &data);
#ifdef DEBUG
        printf("%s\n", data);
#endif
        return data;
    }
}

static int print_bencode_list_node(bencode_node *list, print_buffer *buffer)
{
    bencode_node *ptr = list->list_node_head;

    if (ptr == NULL)
    {
        write_print_buffer(buffer, "[]", 2);
        return 0;
    }

    write_print_buffer(buffer, "[", 1);
    while (1)
    {
        print_bencode_node(ptr, buffer);

        ptr = ptr->next;
        if (ptr != NULL)
            write_print_buffer(buffer, ",", 1);
        else
            break;
    }
    write_print_buffer(buffer, "]", 1);
    return 0;
}

static int print_bencode_dict_node(bencode_node *node, print_buffer *buffer)
{
    bencode_node *ptr = node->dict_node_head;

    if (ptr == NULL)
    {
        write_print_buffer(buffer, "{}", 2);
        return 0;
    }

    write_print_buffer(buffer, "{", 1);
    while (1)
    {
        print_bencode_node(ptr->key, buffer);
        write_print_buffer(buffer, ":", 1);
        print_bencode_node(ptr->val, buffer);

        ptr = ptr->next;
        if (ptr != NULL)
            write_print_buffer(buffer, ",", 1);
        else
            break;
    }
    write_print_buffer(buffer, "}", 1);
    return 0;
}
