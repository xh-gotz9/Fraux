#ifndef __PARSE_H__
#define __PARSE_H__

#include <stdlib.h>

#include "bencoding.h"

char buf[BUFSIZ + 1];

typedef struct parser_buffer
{
    char *data;
    size_t len;
    size_t offset;
} parser_buffer;

parser_buffer *create_parser_buffer(char *src);

bencoding_node *parse_node(parser_buffer *buffer);

bencoding_node *parse_node_str(parser_buffer *buffer);

bencoding_node *parse_node_num(parser_buffer *buffer);
#endif