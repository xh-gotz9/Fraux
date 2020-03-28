#ifndef __PARSE_H__
#define __PARSE_H__

#include <stdlib.h>

#include "bencode.h"

typedef struct parser_buffer
{
    const char *data;
    size_t len;
    size_t offset;
} parser_buffer;

parser_buffer *create_parser_buffer(const char *src);

bencode_node *parse_node(parser_buffer *buffer);

#endif