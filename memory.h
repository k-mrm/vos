#ifndef _MEMORY_H
#define _MEMORY_H

#include <os3.h>

struct memblock
{
        ulong base;
        ulong size;
};

struct chunk
{
        uint nblock;
        struct memblock block[32];
};

struct system_memory
{
        struct chunk avail;
        struct chunk rsrv;
};

extern char __kstart[], __kend[];
extern char __ktext[], __ktext_e[];
extern char __rodata[], __rodata_e[];
extern char __kinit[], __kinit_e[];

#define IS_KERN_TEXT(_va)	((ulong)__ktext <= (ulong)(_va) && (ulong)(_va) < (ulong)__ktext_e)
#define IS_KERN_RODATA(_va)	((ulong)__rodata <= (ulong)(_va) && (ulong)(_va) < (ulong)__rodata_e)
#define IS_KINIT(_va)		((ulong)__kinit <= (ulong)(_va) && (ulong)(_va) < (ulong)__kinit_e)

#endif
