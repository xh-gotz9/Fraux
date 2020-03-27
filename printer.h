#ifndef __PRINTER_H__
#define __PRINTER_H__

#include <stdlib.h>

#ifndef _BASE_BUFFER_SIZE
#define _BASE_BUFFER_SIZE 128;
#endif

#ifndef PRINT_BUFFER_MAGICNUM
#define PRINT_BUFFER_MAGICNUM (0xae86)
#endif

typedef struct print_buffer
{
    char *buf;
    int length;
    int size;
    int magic;
} print_buffer;

print_buffer *create_print_buffer();

int destroy_print_buffer_info(print_buffer *buffer);

int destroy_print_buffer_all(print_buffer *buffer);

print_buffer *write_print_buffer(print_buffer *buffer, void *data, size_t len);

const char *out_print_buffer(print_buffer *buffer);

#endif