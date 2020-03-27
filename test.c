#include <string.h>

#include "bencoding.h"
#include "parser.h"
#include "printer.h"

int create_node_test()
{
    bencoding_node *node = create_node(T_STR);
    if (node->type != T_STR)
    {
        printf("create node error");
        return -1;
    }
}

int parse_data_number_only()
{
    parser_buffer *buffer = create_parser_buffer("i1220e");
    parse_node(buffer);

    return 0;
}

int parse_data_str_only()
{
    parser_buffer *buffer = create_parser_buffer("5:abcde");
    if (!parse_node(buffer))
    {
        return -1;
    }

    return 0;
}

int parse_data_list_only()
{
    char *data = "l3:cow3:moo4:spam4:eggse";
    parser_buffer *buffer = create_parser_buffer(data);
    bencoding_node *node = parse_node(buffer);
    if (!node)
    {
        return -1;
    }
}

int write_print_buffer_test()
{
    print_buffer *buffer = create_print_buffer();
    write_print_buffer(buffer, "data", 4);
    out_print_buffer(buffer);
    write_print_buffer(buffer, "data", 4);
    out_print_buffer(buffer);

    return strcmp(buffer->buf, "datadata") == 0 ? 0 : -1;
}

#define ASSERT_RESULT(CONDITION)                  \
    if (CONDITION == -1)                          \
    {                                             \
        printf("%s test failed!\n", #CONDITION);  \
        exit(EXIT_FAILURE);                       \
    }                                             \
    else                                          \
    {                                             \
        printf("%s test success!\n", #CONDITION); \
    }

int main(int argc, char *argv[])
{
    ASSERT_RESULT(create_node_test());

    ASSERT_RESULT(parse_data_number_only());
    ASSERT_RESULT(parse_data_str_only());
    ASSERT_RESULT(parse_data_list_only());
    
    ASSERT_RESULT(write_print_buffer_test());

    return 0;
}
