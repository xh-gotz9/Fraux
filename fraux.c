#include "fraux.h"

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifndef FRAUX_CONTEXT_STACK_INIT_SIZE
#define FRAUX_CONTEXT_STACK_INIT_SIZE 256
#endif

typedef struct
{
    const char *bencode;
    size_t len, pos;
    struct stack
    {
        char *s;
        size_t size, top;
    } stack;
} fraux_conext;

static void *fraux_conext_push(fraux_conext *c, size_t len)
{
    struct stack *s = &c->stack;
    assert(len > 0);
    if (s->top + len >= s->size)
    {
        // TODO realloc
        if (s->size == 0)
        {
            s->size = FRAUX_CONTEXT_STACK_INIT_SIZE;
        }
        while (s->top + len >= s->size)
        {
            s->size += s->size >> 1;
        }
        s->s = realloc(s->s, s->size);
    }
    void *ret = s->s + s->top;
    s->top += len;
    return ret;

    return NULL;
}

static void *fraux_conext_pop(fraux_conext *c, size_t len)
{
    assert(c->stack.top >= len);
    return c->stack.s + (c->stack.top -= len);
}

static int fraux_parse_number(fraux_conext *c, fraux_value *v)
{
    assert(c->bencode[c->pos] == 'i');
    const char *s = c->bencode + c->pos;
    size_t p = 1;

    while (s[p] >= '0' && s[p] <= '9')
    {
        p++;
        if (c->pos + p >= c->len)
        {
            return FRAUX_PARSE_MISS_QUOTATION_MARK; /* over range */
        }
    }

    if (s[p++] != 'e')
    {
        return FRAUX_PARSE_INVALID_VALUE;
    }

    v->u.n = strtol(c->bencode + c->pos + 1, NULL, 10);
    v->type = FRAUX_NUMBER;
    c->pos += p;

    return FRAUX_PARSE_OK;
}

static int fraux_parse_string(fraux_conext *c, fraux_value *v)
{
    assert(c->bencode[c->pos] >= '0' && c->bencode[c->pos] <= '9');
    const char *head = c->bencode + c->pos;
    size_t p = 0;

    while (head[p] >= '0' && head[p] <= '9')
    {
        p++;
        if (c->pos + p >= c->len)
        {
            return FRAUX_PARSE_MISS_QUOTATION_MARK; /* over range */
        }
    }

    if (head[p++] != ':')
    {
        return FRAUX_PARSE_INVALID_VALUE;
    }

    /* read string length */
    long int len = strtol(c->bencode, NULL, 10);

    /* check data range */
    if (p + len > c->len)
    {
        return FRAUX_PARSE_INVALID_VALUE;
    }

    fraux_set_string(v, head + p, len);
    v->type = FRAUX_STRING;
    c->pos += len;

    return FRAUX_PARSE_OK;
}

static int fraux_parse_value(fraux_conext *c, fraux_value *v)
{
    switch (c->bencode[c->pos])
    {
    case 'i':
        return fraux_parse_number(c, v);
    default:
        if (c->bencode[c->pos] >= '0' || c->bencode[c->pos] <= '9')
        {
            return fraux_parse_string(c, v);
        }
        return FRAUX_PARSE_INVALID_VALUE;
    }
}

int fraux_parse(fraux_value *v, const char *bencode, size_t len)
{
    fraux_conext context;
    assert(v != NULL);
    context.bencode = bencode;
    context.len = len;
    context.pos = 0;
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