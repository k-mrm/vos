#include <os3.h>
#include <util.h>
#include <block.h>

struct bcache
{
	struct lock lock;
        struct list cache;
};

static struct bcache bcache;

static struct buf *
balloc(void)
{
        struct buf *buf;

        buf = kalloc();
        if (!buf)
                return NULL;

        buf->dev = dev;
        buf->refcount = 1;
        list_add(&bcache.cache, &buf->c);

        return buf;
}

static struct buf *
bget(int bno)
{
        struct buf *buf;

        LIST_FOREACH (buf, &bcache.cache, c) {
                if (buf->bno == bno) {
                        buf->refcount++;
                        return buf;
                }
        }

        buf = balloc();
	if (!buf)
		return NULL;
        buf->bno = bno;

        return buf;
}

void
block_init(void)
{
	list_init(&bcache.cache);
}

struct buf *
bread(int bno)
{
        struct buf *b = bget(bno);
        if (!b)
                return NULL;

        if (!(b->flags & B_VALID)) {
		ramdisk_read(b);
                b->flags |= B_VALID;
        }

        return b;
}

struct buf *
bootblock(void)
{
        return bread(0);
}

struct buf *
superblock(void)
{
        return bread(1);
}

void
brelease(struct buf *buf)
{
        if (buf->refcount == 0)
                panic("refcount");

        buf->refcount--;

        if (buf->refcount == 0) {
                if (buf->flags & B_DIRTY)
                        ramdisk_write(buf);
        }
}

void
bcachefree(void)
{
        struct buf *buf;

        LIST_FOREACH (buf, &bcache.cache, c) {
                if (buf->refcount == 0) {
                        list_delete(&buf->c);
                        free(buf);
                }
        }
}

void
bsync(void)
{
        struct buf *buf;

        LIST_FOREACH (buf, &bcache.cache, c) {
                if (buf->refcount != 0 && buf->flags & B_DIRTY) {
                        ramdisk_write(buf->dev, buf);
                        buf->flags &= ~B_DIRTY;
                }
        }
}
