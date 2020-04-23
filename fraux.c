#include "fraux.h"

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

static int fraux_parse_string(fraux_conext *c, fraux_value *v)
{
    assert(*c->bencode >= '0' && *c->bencode <= '9');
    const char *p = c->bencode;
    long int len;

    while (*p >= '0' && *p <= '9')
    {
        *p++;
    }

    switch (*p++)
    {
    case ':':
        len = strtol(c->bencode, NULL, 10);
        fraux_set_string(v, p, len);
        v->type = FRAUX_STRING;
        c->bencode += len;
        return FRAUX_PARSE_OK;
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
        if (*c->bencode >= '0' || *c->bencode <= '9')
        {
            return fraux_parse_string(c, v);
        }
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

void fraux_clean(fraux_value *v)
{
    assert(v != NULL);
    if (fraux_get_type(v) == FRAUX_STRING)
    {
        free(v->u.s.s);
    }
    memset(v, 0, sizeof(fraux_value));
    v->type = FRAUX_UNKNOWN;
}

fraux_type fraux_get_type(fraux_value *v)
{
    assert(v != NULL);
    return v->type;
}

void fraux_set_number(fraux_value *v, long int num)
{
    assert(v != NULL);
    fraux_clean(v);
    v->u.n = num;
    v->u.n = FRAUX_NUMBER;
}

void fraux_set_string(fraux_value *v, const char *s, size_t len)
{
    assert(v != NULL && (s != NULL || len == 0));
    fraux_clean(v);
    v->u.s.s = malloc(len + 1);
    memcpy(v->u.s.s, s, len);
    v->u.s.s[len] = '\0';
    v->u.s.len = len;
    v->type = FRAUX_STRING;
}