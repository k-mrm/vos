#include <os3.h>
#include <memory.h>
#include <x86.h>

static struct system_memory sysmem = {
        .avail.nblock = 0,
        .rsrv.nblock = 0,
};

#define FOREACH_CHUNK_BLOCK(_chunk, _idx, _block)  \
        for (_block = (_chunk)->block, _idx = 0;            \
             _block < &(_chunk)->block[(_chunk)->nblock];   \
             _block++, _idx++)

#define FOREACH_SYSMEM_AVAIL_BLOCK(_block)                  \
        for (_block = sysmem.avail.block;                         \
             _block < &sysmem.avail.block[sysmem.avail.nblock];   \
             _block++)

#define FOREACH_SYSMEM_RSRV_BLOCK(_block)                 \
        for (_block = sysmem.rsrv.block;                        \
             _block < &sysmem.rsrv.block[sysmem.rsrv.nblock];   \
             _block++)

void
reserve_kernel_area(void)
{
        ulong kstartpa, kendpa;
        ulong ksize;

        kstartpa = V2P(__kstart);
        kendpa = V2P(__kend);
        ksize = PAGEALIGN(kendpa - kstartpa);

	sysmem_reserved(kstartpa, ksize);
}

/*
 * __memblockoverlap
 * true: overlapped
 * false: ∅
 */
static bool
__memblockoverlap (ulong astart, ulong aend, ulong bstart, ulong bend,
		   ulong *pstart, ulong *pend)
{
        ulong s, e;

        s = MAX (astart, bstart);
        e = MIN (aend, bend);

        if (s < e) {
                if (pstart)
                        *pstart = s;
                if (pend)
                        *pend = e;
                return true;
        } else {
                return false;
        }
}

static bool
memblockoverlap (struct memblock *a, struct memblock *b, ulong *pstart, ulong *pend)
{
        return __memblockoverlap(a->base, a->base + a->size, b->base, b->base + b->size,
                                 pstart, pend);
}

static bool
memblockmerge (struct memblock *a, struct memblock *b, ulong *pstart, ulong *pend)
{
        ulong s, e;

        if (!memblockoverlap (a, b, NULL, NULL))
                return false;

        s = MIN(a->base, b->base);
        e = MAX(a->base + a->size, b->base + b->size);

        if (pstart)
                *pstart = s;
        if (pend)
                *pend = e;
        return true;
}

/*
 * bootmemfind
 * true: found memory region, start physaddr is @pa
 * false: memory is not found
 */
static bool INIT
bootmemfind (uint nbytes, uint align, ulong *pa)
{
        struct memblock *ab, *rb;
        ulong start, end;      /* [start, end) */
        ulong rstart, rend;    /* [rstart, rend) */
        ulong ms, me;

        FOREACH_SYSMEM_AVAIL_BLOCK (ab) {
                start = ab->base;
                end = ab->base + ab->size;

                for (uint i = 0; i < sysmem.rsrv.nblock + 1; i++) {
                        rb = sysmem.rsrv.block + i;

                        if (i)
                                rstart = rb[-1].base + rb[-1].size;
                        else
                                rstart = 0x0;

                        if (i == sysmem.rsrv.nblock)
                                rend = (ulong)-1ll;
                        else
                                rend = rb->base;

                        if (__memblockoverlap(start, end, rstart, rend, &ms, &me)) {
                                ms = ALIGN (ms, align);
                                if (me - ms >= nbytes) {
                                        *pa = ms;
                                        return true;
                                }
                        }
                }
        }
        return false;
}

void *
boot_memalloc(uint nbytes, uint align)
{
        ulong pa;
        void *va;

        if (nbytes == 0)
                return NULL;
        if (!bootmemfind(nbytes, align, &pa))
                return NULL;

        sysmem_reserved(pa, nbytes);

        va = P2V(pa);
        memset(va, 0, nbytes);
        return va;
}

static void
memremoveblock (struct chunk *c, uint idx)
{
        if (idx >= c->nblock)
                return;

        memmove(c->block + idx, c->block + idx + 1, (c->nblock - idx - 1) * sizeof(struct memblock));
        c->nblock--;
}

static void
meminsertblock (struct chunk *c, uint idx, ulong start, ulong size)
{
        struct memblock *block;

        if (c->nblock >= 32)
                panic ("nblock > 32");

        memmove (c->block + idx + 1, c->block + idx, (c->nblock - idx) * sizeof (struct memblock));
        block = &c->block[idx];
        block->base = start;
        block->size = size;
        c->nblock++;
}

static bool
memchunkin(struct chunk *c, ulong pa)
{
        struct memblock *block;
        uint idx;

        FOREACH_MEMCHUNK_BLOCK (c, idx, block) {
                if (pa < block->base)
                        return false;
                else if (block->base <= pa && pa < block->base + block->size)
                        return true;
        }
        return false;
}

static void
memchunkmerge(struct chunk *c)
{
        struct memblock *block, *next;
        ulong start, end;
        bool mergeable;

        for (int i = 0; i < c->nblock - 1; )
        {
                block = c->block + i;
                next = c->block + i + 1;

                mergeable = memblockmerge(block, next, &start, &end);
                if (mergeable)
                {
                        memremoveblock(c, i);
                        block->base = start;
                        block->size = end - start;
                        continue;
                }

                i++;
        }
}

static void
mem_newblock(struct chunk *c, ulong start, ulong size)
{
        struct memblock *block;
        uint idx = 0;

        FOREACH_MEMCHUNK_BLOCK (c, idx, block) {
                if (start <= block->base)
                        break;
        }

        meminsertblock(c, idx, start, size);
        memchunkmerge(c);
}

bool
addr_is_reserved(ulong addr)
{
        return memchunkin(&sysmem.rsrv, addr);
}

void
sysmem_avail(ulong base, ulong size)
{
        mem_newblock(&sysmem.avail, base, size);
}

void
sysmem_reserved(ulong base, ulong size)
{
        mem_newblock(&sysmem.rsrv, base, size);
}

ulong
memstart(void)
{
        struct memblock *first = &sysmem.avail.block[0];
        return first->base;
}

ulong
memend(void)
{
        struct memblock *last = &sysmem.avail.block[sysmem.avail.nblock-1];
        return last->base + last->size;
}
