#define _GNU_SOURCE
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <string.h>

#include "bencode.h"
#include "parser.h"
#include "printer.h"

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

int create_node_test()
{
    bencode_node *node = create_node(T_STR);
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
    bencode_node *node = parse_node(buffer);
    if (!node)
    {
        return -1;
    }
}

int parse_data_from_file()
{
    int fd = open("./test.torrent", O_RDWR);
    if (fd == -1)
    {
        perror("open error");
        exit(EXIT_FAILURE);
    }
    struct stat stat;
    if (fstat(fd, &stat) == -1)
    {
        perror("fstat error");
        exit(EXIT_FAILURE);
    }
    off_t len = stat.st_size;
    char *ptr = mmap(NULL, (size_t)len + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

    ptr[len] = 0;

    parser_buffer *buffer = create_parser_buffer(ptr);
    bencode_node *node = parse_node(buffer);
    munmap(ptr, len + 1);

    if (node == NULL)
    {
        return -1;
    }

    return 0;
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

int main(int argc, char *argv[])
{
    ASSERT_RESULT(create_node_test());

    ASSERT_RESULT(parse_data_number_only());
    ASSERT_RESULT(parse_data_str_only());
    ASSERT_RESULT(parse_data_list_only());
    ASSERT_RESULT(parse_data_from_file());

    ASSERT_RESULT(write_print_buffer_test());

    return 0;
}
