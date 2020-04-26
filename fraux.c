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

static int fraux_parse_value(fraux_conext *c, fraux_value *v);

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
    long int len = strtol(head, NULL, 10);

    /* check data range */
    if (p + len > c->len)
    {
        return FRAUX_PARSE_INVALID_VALUE;
    }

    fraux_set_string(v, head + p, len);
    v->type = FRAUX_STRING;
    c->pos += p + len;

    return FRAUX_PARSE_OK;
}

static int fraux_parse_list(fraux_conext *c, fraux_value *v)
{
    assert(v != NULL);
    assert(c->bencode[c->pos++] == 'l');

    int ret;
    size_t size = 0;
    fraux_value *head = (fraux_value *)(c->stack.s + c->stack.top);
    for (;;)
    {
        fraux_value e;
        fraux_init(&e);
        switch (c->bencode[c->pos])
        {
        case 'e':
            c->pos++;
            fraux_set_list(v, size);
            if (size > 0)
                memcpy(v->u.l.e, fraux_conext_pop(c, size * sizeof(fraux_value)), size * sizeof(fraux_value));
            v->u.l.size = size;
            ret = FRAUX_PARSE_OK;
            goto break_loop;
        default:
            if (c->pos >= c->len)
            {
                ret = FRAUX_PARSE_MISS_QUOTATION_MARK;
            }

            fraux_clean(&e);
            if ((ret = fraux_parse_value(c, &e)) != FRAUX_PARSE_OK)
            {
                fraux_conext_pop(c, size * sizeof(fraux_value));
                goto break_loop;
            }
            memcpy(fraux_conext_push(c, sizeof(fraux_value)), &e, sizeof(fraux_value));
            size++;
            break;
        }
    }
break_loop:
    return ret;
}

static int fraux_parse_dictionary(fraux_conext *c, fraux_value *v)
{
    assert(v != NULL);
    assert(c->bencode[c->pos++] == 'd');

    int ret;
    size_t size = 0;
    for (;;)
    {
        if (c->bencode[c->pos] == 'e')
        {
            c->pos++;
            fraux_set_dictionary(v, size);
            if (size > 0)
                memcpy(v->u.d.e, fraux_conext_pop(c, size * sizeof(fraux_dict_member)), size * (sizeof(fraux_dict_member)));
            v->u.d.size = size;
            ret = FRAUX_PARSE_OK;
            break;
        }

        fraux_value key;
        fraux_dict_member m;
        fraux_init(&key);
        fraux_init(&m.v);

        /* parse key */
        if ((ret = fraux_parse_value(c, &key)) != FRAUX_PARSE_OK)
            break;

        if (fraux_get_type(&key) != FRAUX_STRING)
        {
            ret = FRAUX_PARSE_INVALID_VALUE;
            break;
        }
        memcpy(&m.k, &key.u.s, sizeof(struct bstring));

        /* parse value*/
        if ((ret = fraux_parse_value(c, &m.v)) != FRAUX_PARSE_OK)
            break;

        memcpy(fraux_conext_push(c, sizeof(fraux_dict_member)), &m, sizeof(fraux_dict_member));
        size++;
    }

    return ret;
}

static int fraux_parse_value(fraux_conext *c, fraux_value *v)
{
    switch (c->bencode[c->pos])
    {
    case 'i':
        return fraux_parse_number(c, v);
    case 'l':
        return fraux_parse_list(c, v);
    case 'd':
        return fraux_parse_dictionary(c, v);
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
    memset(&context, 0, sizeof(fraux_conext));
    context.bencode = bencode;
    context.len = len;
    context.pos = 0;
    v->type = FRAUX_UNKNOWN;
    return fraux_parse_value(&context, v);
}

void fraux_init(fraux_value *v)
{
    assert(v != NULL);
    memset(v, 0, sizeof(fraux_value));
}

void fraux_clean(fraux_value *v)
{
    assert(v != NULL);
    switch (fraux_get_type(v))
    {
    case FRAUX_STRING:
        free(v->u.s.s);
        break;
    case FRAUX_LIST:
        for (size_t i = 0; i < v->u.l.size; i++)
        {
            fraux_clean(v->u.l.e + i);
        }
        free(v->u.l.e);
        break;
    case FRAUX_DICTIONARY:
        for (size_t i = 0; i < v->u.d.size; i++)
        {
            fraux_dict_member *e = v->u.d.e + i;
            free(e->k.s); /* free key string */
            fraux_clean(&e->v);
        }
        free(v->u.d.e);
        break;
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

void fraux_set_list(fraux_value *v, size_t capacity)
{
    assert(v != NULL);
    fraux_clean(v);
    v->type = FRAUX_LIST;
    v->u.l.size = 0;
    v->u.l.capacity = capacity;
    v->u.l.e = capacity > 0 ? malloc(capacity * (sizeof(fraux_value))) : NULL;
}

void fraux_set_dictionary(fraux_value *v, size_t capacity)
{
    assert(v != NULL);
    fraux_clean(v);
    v->type = FRAUX_DICTIONARY;
    v->u.d.size = 0;
    v->u.d.capacity = capacity;
    v->u.d.e = capacity > 0 ? malloc(capacity * sizeof(fraux_dict_member)) : NULL;
}