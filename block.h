#ifndef _BLOCK_H
#define _BLOCK_H

#include <os3.h>
#include <util.h>

#define B_VALID         (1)
#define B_DIRTY         (1 << 1)

struct buf
{
	int bno;
	int flags;
	uchar data[1024];
	uint refcount;
	struct list c;
};

#endif
