#include <os3.h>

extern char _binary_fs_img_end[];
extern char _binary_fs_img_size[];
extern char _binary_fs_img_start[];

static void *diskbase;

int
ramdisk_read(struct buf *buf)
{
        int offset = buf->bno * 1024;
        memcpy (buf->data, diskbase + offset, 1024);
        return 0;
}

int
ramdisk_write(struct buf *buf)
{
        int offset = buf->bno * 1024;
        memcpy (diskbase + offset, buf->data, 1024);
        return 0;
}

void
ramdisk_init(void)
{
        diskbase = _binary_fs_img_start;
}
