#ifndef _UTIL_H
#define _UTIL_H

#include <os3.h>

struct list
{
        struct list *prev, *next;
};

#define LIST_HEAD(_v)  struct list _v = {&(_v), &(_v)}

#define LIST_FOREACH(_pos, _head, _mem)        \
        for (_pos = container_of((_head)->next, typeof(*_pos), _mem);      \
             &(_pos->_mem) != (_head);    \
             _pos = container_of(_pos->_mem.next, typeof(*_pos), _mem))

#define ENTRY(_h, _ty, _mem)       container_of((_h)->next, _ty, _mem)

static inline void
list_init(struct list *head)
{
        head->next = head;
        head->prev = head;
}

static inline void
list_add(struct list *h, struct list *e)
{
        e->next = h->next;
        h->next->prev = e;
        e->prev = h;
        h->next = e;
}

static inline int
list_len(struct list *h)
{
        struct list *e;
        int i;

        for (e = h->next; e != h; e = e->next)
                i++;
        return i;
}

static inline bool
list_empty(struct list *h)
{
        return list_len(h) == 0;
}

static inline void
list_delete (struct list *e)
{
        struct list *n, *p;
        n = e->next;
        p = e->prev;
        p->next = n;
        n->prev = p;
        e->next = e->prev = NULL;
}

struct id
{
	struct lock lock;
	unsigned int n;
};

#endif	// _UTIL_H
