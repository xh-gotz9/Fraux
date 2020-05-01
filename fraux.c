#include "fraux.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
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
    size_t len = strtoul(head, NULL, 10);

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
    assert(c->bencode[c->pos] == 'l');
    c->pos++;

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

            fraux_init(&e);
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
    assert(c->bencode[c->pos] == 'd');
    c->pos++;

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

static void fraux_stringtify_value(fraux_conext *c, fraux_value *v)
{
    switch (v->type)
    {
    case FRAUX_NUMBER:
        c->stack.top -= 32 - sprintf(fraux_conext_push(c, 32), "i%lde", v->u.n);
        break;
    case FRAUX_STRING:
        c->stack.top -= 32 - sprintf(fraux_conext_push(c, 32), "%ld:", v->u.s.len);
        memcpy(fraux_conext_push(c, v->u.s.len), v->u.s.s, v->u.s.len);
        break;
    case FRAUX_LIST:
        *(char *)(fraux_conext_push(c, 1)) = 'l';
        for (size_t i = 0; i < v->u.l.size; i++)
        {
            fraux_stringtify_value(c, v->u.l.e + i);
        }
        *(char *)(fraux_conext_push(c, 1)) = 'e';
        break;
    case FRAUX_DICTIONARY:
        *(char *)(fraux_conext_push(c, 1)) = 'd';
        for (size_t i = 0; i < v->u.d.size; i++)
        {
            /* write key */
            c->stack.top -= 32 - sprintf(fraux_conext_push(c, 32), "%ld:", v->u.d.e[i].k.len);
            memcpy(fraux_conext_push(c, v->u.d.e[i].k.len), v->u.d.e[i].k.s, v->u.d.e[i].k.len);

            /* write value */
            fraux_stringtify_value(c, &v->u.d.e[i].v);
        }
        *(char *)(fraux_conext_push(c, 1)) = 'e';
        break;
    default:
        assert(!"invalid type");
    }
}

char *fraux_stringtify(fraux_value *v, size_t *length)
{
    assert(v != NULL);
    fraux_conext c;
    char *ret;
    memset(&c, 0, sizeof(fraux_conext));
    fraux_stringtify_value(&c, v);
    if (length)
        *length = c.stack.top;
    ret = realloc(c.stack.s, c.stack.top + 1);
    assert(ret != NULL);
    ret[c.stack.top] = '\0';
    return ret;
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

/* shallow copy */
void fraux_copy(fraux_value *dest, fraux_value *src)
{
    assert(dest != NULL);
    assert(src != NULL);
    memcpy(dest, src, sizeof(fraux_value));
}

/* deep copy */
void fraux_deepcopy(fraux_value *dest, const fraux_value *src)
{
    assert(dest != NULL);
    assert(src != NULL);
    switch (src->type)
    {
    case FRAUX_NUMBER:
        memcpy(dest, src, sizeof(fraux_value));
        break;
    case FRAUX_STRING:
        memcpy(dest, src, sizeof(fraux_value));
        memcpy(dest->u.s.s = malloc(src->u.s.len), src->u.s.s, src->u.s.len);
        break;
    case FRAUX_LIST:
        memcpy(dest, src, sizeof(fraux_value));
        dest->u.l.e = malloc(src->u.l.capacity * sizeof(fraux_value));
        for (size_t i = 0; i < src->u.l.size; i++)
        {
            fraux_deepcopy(dest->u.l.e + i, src->u.l.e + i);
        }
        break;
    case FRAUX_DICTIONARY:
        memcpy(dest, src, sizeof(fraux_value));
        dest->u.d.e = malloc(src->u.d.capacity * sizeof(fraux_dict_member));
        for (size_t i = 0; i < src->u.d.size; i++)
        {
            dest->u.d.e[i].k.len = src->u.d.e[i].k.len;
            memcpy((dest->u.d.e[i].k.s = malloc(dest->u.d.e[i].k.len)), src->u.d.e[i].k.s, src->u.d.e[i].k.len);
            fraux_deepcopy(&dest->u.d.e[i].v, &src->u.d.e[i].v);
        }
        break;
    default:
        assert(!"unknown value type");
    }
}

void fraux_swap(fraux_value *v1, fraux_value *v2)
{
    fraux_value tmp;
    fraux_copy(&tmp, v1);
    fraux_copy(v1, v2);
    fraux_copy(v2, &tmp);
}

int fraux_equals(fraux_value *v1, fraux_value *v2)
{
    assert(v1 != NULL);
    assert(v2 != NULL);

    if (v1 == v2)
        return 1;

    if (v1->type != v2->type)
        return 0;

    switch (v1->type)
    {
    case FRAUX_NUMBER:
        return v1->u.n == v2->u.n ? 1 : 0;
    case FRAUX_STRING:
        if (v1->u.s.len != v2->u.s.len)
            return 0;
        return memcmp(v1->u.s.s, v2->u.s.s, v1->u.s.len) == 0 ? 1 : 0;
    case FRAUX_LIST:
        if (v1->u.l.size != v2->u.l.size)
            return 0;
        for (size_t i = 0; i < v1->u.l.size; i++)
        {
            if (!fraux_equals(&v1->u.l.e[i], &v2->u.l.e[i]))
                return 0;
        }
        return 1;
    case FRAUX_DICTIONARY:
        if (v1->u.d.size != v2->u.d.size)
            return 0;
        for (size_t i = 0; i < v1->u.d.size; i++)
        {
            if (v1->u.d.e[i].k.len != v2->u.d.e[i].k.len || memcmp(v1->u.d.e[i].k.s, v2->u.d.e[i].k.s, v1->u.d.e[i].k.len) != 0)
                return 0;
            if (!fraux_equals(&v1->u.d.e[i].v, &v2->u.d.e[i].v))
                return 0;
        }
        return 1;
    default:
        assert(!"unknown value type");
        return 0;
    }
}

fraux_type fraux_get_type(fraux_value *v)
{
    assert(v != NULL);
    return v->type;
}

void fraux_set_number(fraux_value *v, long int num)
{
    assert(v != NULL);
    fraux_init(v);
    v->u.n = num;
    v->u.n = FRAUX_NUMBER;
}

void fraux_set_string(fraux_value *v, const char *s, size_t len)
{
    assert(v != NULL && (s != NULL || len == 0));
    fraux_init(v);
    v->u.s.s = malloc(len + 1);
    memcpy(v->u.s.s, s, len);
    v->u.s.s[len] = '\0';
    v->u.s.len = len;
    v->type = FRAUX_STRING;
}

void fraux_set_list(fraux_value *v, size_t capacity)
{
    assert(v != NULL);
    fraux_init(v);
    v->type = FRAUX_LIST;
    v->u.l.size = 0;
    v->u.l.capacity = capacity;
    v->u.l.e = capacity > 0 ? malloc(capacity * (sizeof(fraux_value))) : NULL;
}

void fraux_set_dictionary(fraux_value *v, size_t capacity)
{
    assert(v != NULL);
    fraux_init(v);
    v->type = FRAUX_DICTIONARY;
    v->u.d.size = 0;
    v->u.d.capacity = capacity;
    v->u.d.e = capacity > 0 ? malloc(capacity * sizeof(fraux_dict_member)) : NULL;
}

fraux_value *fraux_list_get(fraux_value *l, size_t idx)
{
    assert(l != NULL);

    if (idx > l->u.l.size)
        return NULL;

    return l->u.l.e + idx;
}

void fraux_list_insert(fraux_value *l, fraux_value *e, size_t idx)
{
    assert(l != NULL);
    assert(e != NULL);

    struct blist *list = &l->u.l;

    if (list->size + 1 > list->capacity)
    {
        list->e = realloc(list->e, sizeof(fraux_value) * (list->capacity + 2));
        list->capacity += 2;
    }

    size_t index = list->size + 1 < idx ? list->size + 1 : idx;
    size_t i = list->size;
    while (i > index)
    {
        fraux_copy(list->e + (i--), list->e + i);
    }
    fraux_copy(list->e + i, e);
    list->size++;
}

void fraux_list_delete(fraux_value *l, size_t idx, fraux_value *e)
{
    assert(l != NULL);
    assert(e != NULL);

    struct blist *list = &l->u.l;

    if (idx >= list->size)
    {
        fraux_init(e);
        return;
    }

    fraux_copy(e, list->e + idx);
    for (size_t i = idx + 1; i < list->size; i++)
    {
        fraux_copy(list->e + i - 1, list->e + i);
    }
    list->size--;
}

void fraux_list_push(fraux_value *l, fraux_value *e)
{
    fraux_list_insert(l, e, l->u.l.size);
}

void fraux_list_pop(fraux_value *l, fraux_value *e)
{
    fraux_list_delete(l, l->u.l.size - 1, e);
}

void fraux_dictinary_add(fraux_value *d, fraux_dict_member *m)
{
    assert(d != NULL);
    assert(m != NULL);

    struct dictionary *dict = &d->u.d;

    if (dict->size + 1 > dict->capacity)
    {
        dict->e = realloc(dict->e, sizeof(fraux_dict_member) * (dict->capacity + 2));
        dict->capacity += 2;
    }

    memcpy(dict->e + dict->size, m, sizeof(fraux_dict_member));
    dict->size++;
}

void fraux_dictinary_remove(fraux_value *d, char *key, size_t len, fraux_dict_member *m)
{
    assert(d != NULL);
    size_t index;
    fraux_value *val = fraux_dictinary_find(d, key, len, &index);
    if (!val)
    {
        memset(m, 0, sizeof(fraux_dict_member));
        return;
    }

    if (m)
        memcpy(m, d->u.d.e + index, sizeof(fraux_dict_member));

    for (size_t i = index + 1; i < d->u.d.size; i++)
    {
        memcpy(d->u.d.e + i - 1, d->u.d.e + i, sizeof(fraux_dict_member));
    }
    d->u.d.size--;
}

fraux_value *fraux_dictinary_find(fraux_value *d, char *key, size_t len, size_t *index)
{
    assert(d != NULL);

    struct dictionary *dict = &d->u.d;

    size_t i = 0;
    for (; i < dict->size; i++)
    {
        char *k = dict->e[i].k.s;
        size_t l = dict->e[i].k.len;

        if (l != len)
            continue;

        if (memcmp(key, k, len) == 0)
        {
            if (index)
                *index = i;
            return &dict->e[i].v;
        }
    }

    return NULL;
}
