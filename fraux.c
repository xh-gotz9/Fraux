#include "fraux.h"

#include <assert.h>
#include <stdlib.h>

typedef struct
{
    const char *bencode;
} fraux_conext;

static int fraux_parse_number(fraux_conext *c, fraux_value *v)
{
    assert(*c->bencode == 'i');
    const char *p = c->bencode + 1;

    while (*p >= '0' && *p <= '9')
        p++;

    switch (*p++)
    {
    case 'e':
        v->u.n = strtol(c->bencode + 1, NULL, 10);
        v->type = FRAUX_NUMBER;
        c->bencode = p;
    case '\0':
        return FRAUX_PARSE_MISS_QUOTATION_MARK;
    default:
        return FRAUX_PARSE_INVALID_VALUE;
    }
}

static int fraux_parse_value(fraux_conext *c, fraux_value *v)
{
    switch (*c->bencode)
    {
    case 'i':
        return fraux_parse_number(c, v);
    default:
        return FRAUX_PARSE_INVALID_VALUE;
    }
}

int fraux_parse(fraux_value *v, const char *bencode)
{
    fraux_conext context;
    assert(v != NULL);
    context.bencode = bencode;
    v->type = FRAUX_UNKNOWN;
    return fraux_parse_value(&context, v);
}

fraux_type fraux_get_type(fraux_value *v)
{
    assert(v != NULL);
    return v->type;
}