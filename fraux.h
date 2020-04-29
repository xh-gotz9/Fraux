#ifndef __FRAUX_BENCODE_H__
#define __FRAUX_BENCODE_H__

#include <stdlib.h>

typedef enum
{
    FRAUX_UNKNOWN = 0,
    FRAUX_STRING,
    FRAUX_NUMBER,
    FRAUX_LIST,
    FRAUX_DICTIONARY
} fraux_type;

typedef struct fraux_value fraux_value;
typedef struct bstring bstring;
typedef struct fraux_dict_member fraux_dict_member;

typedef struct fraux_value
{
    fraux_type type;
    union {
        long int n;
        struct bstring
        {
            char *s;
            size_t len;
        } s;
        struct blist
        {
            fraux_value *e;
            size_t size, capacity;
        } l;
        struct dictionary
        {
            fraux_dict_member *e;
            size_t size, capacity;
        } d;
    } u;
} fraux_value;

struct fraux_dict_member
{
    struct bstring k;
    fraux_value v;
};

enum
{
    FRAUX_PARSE_OK = 0,
    FRAUX_PARSE_INVALID_VALUE,
    FRAUX_PARSE_MISS_QUOTATION_MARK,
};

int fraux_parse(fraux_value *v, const char *bencode, size_t len);
char *fraux_stringtify(fraux_value *v, size_t *length);

void fraux_init(fraux_value *v);
void fraux_clean(fraux_value *v);
void fraux_copy(fraux_value *dest, fraux_value *src);
void fraux_deepcopy(fraux_value *dest, const fraux_value *src); /* deep copy */
void fraux_swap(fraux_value *v1, fraux_value *v2);
int fraux_equals(fraux_value *v1, fraux_value *v2);

fraux_type fraux_get_type(fraux_value *v);

/* set value */
void fraux_set_number(fraux_value *v, long int num);
void fraux_set_string(fraux_value *v, const char *s, size_t len);
void fraux_set_list(fraux_value *v, size_t capacity);
void fraux_set_dictionary(fraux_value *v, size_t capacity);

/* fraux_list */
void fraux_list_insert(fraux_value *l, fraux_value *e, size_t idx);
void fraux_list_delete(fraux_value *l, size_t idx, fraux_value *e);
void fraux_list_push(fraux_value *l, fraux_value *e);
void fraux_list_pop(fraux_value *l, fraux_value *e);

/* fraux_dictionary*/
void fraux_dictinary_add(fraux_value *d, fraux_dict_member *m);
void fraux_dictinary_remove(fraux_value *d, char *key, size_t len, fraux_value *m);
void fraux_dictinary_find(fraux_value *d, char *key, size_t len, size_t *index, fraux_value *v);

#endif