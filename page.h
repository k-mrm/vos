#ifndef _PAGE_H
#define _PAGE_H

struct page
{
        struct page *next;
        u8 blockno;
};

struct pageblock
{
        struct page *pages;
        ulong base;
        uint npages;
};

extern struct pageblock proot[32];
extern uint nproot;

#define FOREACH_PAGEBLOCK(_pb)  \
	for (_pb = proot; _pb < &proot[nproot]; _pb++)

// FIXME: naive
static inline struct pageblock *
pa2block(ulong pa)
{
        struct pageblock *b;
        ulong size;

        FOREACH_PAGEBLOCK (b) {
                size = b->npages << PAGESHIFT;
                if (b->base <= pa && pa < b->base + size)
                        return b;
        }
        return NULL;
}

static inline struct page *
pa2page(ulong pa) {
        struct pageblock *block = pa2block(pa);
        return block->pages + ((pa - block->base) >> PAGESHIFT);
}

static inline ulong
page2pa(struct page *page)
{
        struct pageblock *block = &proot[page->blockno];
        return block->base + ((page - block->pages) << PAGESHIFT);
}

static inline struct page *
va2page(void *va)
{
        return pa2page (V2P (va));
}

static inline void *
page2va (Page *page)
{
        return (void *)P2V (page2pa (page));
}

Page *allocpages (uint order);
void *alloczeropagesva (uint order);
void freepages (Page *pages, uint order);

#define zalloc()    alloczeropagesva (0)
#define alloc()     page2va (allocpages (0))
#define free(_addr) freepages (va2page (_addr), 0)

#define PA2PFN(_pa)     ((_pa) >> PAGESHIFT)
#define PFN2PA(_pfn)    ((_pfn) << PAGESHIFT)

void kallocinitearly (ulong start, ulong end) INIT;
void kallocinit (void) INIT;

#endif
