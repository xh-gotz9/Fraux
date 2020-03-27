#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <error.h>

#include "printer.h"

static int realloc_print_buffer(print_buffer *buffer, size_t target);

/**
 * check buffer pointer, buffer->magic and buffer->buf pointer.
 **/
static void ensure(print_buffer *buffer)
{
    if (buffer != NULL && buffer->magic == PRINT_BUFFER_MAGICNUM && buffer->buf != NULL)
    {
        return;
    }
    fprintf(stderr, "WARNING - try to destroy a memory which is not \"struct print_buffer\" data.\n");
    exit(EXIT_FAILURE);
}

print_buffer *create_print_buffer()
{
    print_buffer *buffer = malloc(sizeof(print_buffer));
    if (buffer == NULL)
    {
        perror("malloc error");
        exit(EXIT_FAILURE);
    }

    /* 每次为 buf 预留一位终止符号位 */
    size_t buf_size = _BASE_BUFFER_SIZE;
    char *ptr = malloc(sizeof(char) * (buf_size + 1));
    memset(ptr, 0, sizeof(char) * (buf_size + 1));
    buffer->buf = ptr;

    // TODO error check
    buffer->length = 0;
    buffer->size = _BASE_BUFFER_SIZE;
    buffer->magic = PRINT_BUFFER_MAGICNUM;

    return buffer;
}

int destroy_print_buffer_info(print_buffer *buffer)
{
    ensure(buffer);
    free(buffer);
}

int destroy_print_buffer_all(print_buffer *buffer)
{
    ensure(buffer);

    char *ptr = buffer->buf;
    if (destroy_print_buffer_info(buffer) == -1)
    {
        return -1;
    }

    free(ptr);
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
    /* add terminating char */
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
        perror("realloc error");
        exit(EXIT_FAILURE);
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