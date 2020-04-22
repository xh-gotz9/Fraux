#ifndef __FRAUX_BENCODE_H__
#define __FRAUX_BENCODE_H__

typedef enum
{
    FRAUX_UNKNOWN = 0,
    FRAUX_STRING,
    FRAUX_NUMBER,
    FRAUX_LIST,
    FRAUX_DICTIONARY
} fraux_type;

typedef struct
{
    fraux_type type;
    union {
        long int n;
    } u;
} fraux_value;

enum
{
    FRAUX_PARSE_OK = 0,
    FRAUX_PARSE_INVALID_VALUE,
    FRAUX_PARSE_MISS_QUOTATION_MARK,
};

int fraux_parse(fraux_value *v, const char *bencode);

fraux_type fraux_get_type(fraux_value *v);

#endif