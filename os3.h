#ifndef _OS3_H
#define _OS3_H

typedef unsigned long   u64;
typedef long            i64;
typedef unsigned int    u32;
typedef signed int      i32;
typedef unsigned short  u16;
typedef signed short    i16;
typedef unsigned char   u8;
typedef signed char     i8;
typedef unsigned int    uint;
typedef unsigned short  ushort;
typedef unsigned long   ulong;
typedef unsigned char   uchar;

#define NULL    ((void *)0)

typedef _Bool   bool;

#define true    1
#define false   0

#define offsetof(st, m)   ((ulong)((char *)&((st *)0)->m - (char *)0))

#define container_of(ptr, st, m)  \
  ({ const typeof(((st *)0)->m) *_mptr = (ptr); \
     (st *)((char *)_mptr - offsetof(st, m)); })

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

#define	KiB	(1024)
#define	MiB	(1024 * 1024)
#define	GiB	(1024 * 1024 * 1024)

#define UNUSED          __attribute__ ((unused))
#define DEBUG           __attribute__ ((unused))
#define FALLTHROUGH     __attribute__ ((fallthrough))
#define PACKED          __attribute__ ((packed))
#define ALIGNED(n)      __attribute__ ((aligned(n)))
#define NORETURN        __attribute__ ((noreturn))
#define SECTION(s)      __attribute__ ((section(s)))
#define USED            __attribute__ ((used))

#define va_list         __builtin_va_list
#define va_start(v, l)  __builtin_va_start (v, l)
#define va_arg(v, l)    __builtin_va_arg (v, l)
#define va_end(v)       __builtin_va_end (v)
#define va_copy(d, s)   __builtin_va_copy (d, s)

struct proc;
struct multiboot2;
struct buf;

void uart_putc(char c);
int uart_write(char *buf, int n);
void uart_init(void);

void sysmem_avail(ulong base, ulong size);
void sysmem_reserved(ulong base, ulong size);
void *boot_memalloc(uint nbytes, ulong align);
bool addr_is_reserved(ulong addr);
void reserve_kernel_area(void);
ulong memstart(void);
ulong memend(void);

void seg_init(void);

void mb2_load(struct multiboot2 *mb);

void pic8259a_init(void);

void process_init(void);
int exec(const char *path, const char **argv);
int fork(void);
int exit(int status);
int wait(int *status);
void schedule(void);
void wakeup(void *chan);
void sleep(void *chan);
struct proc *cswitch(ulong *prev, ulong *next, struct proc *pprev);

void *memset(void *dst, int c, ulong n);
void *memcpy(void *dst, const void *src, ulong n);
void *memmove(void *dst, const void *src, ulong n);
char *strcpy(char *dst, const char *src);
int strcmp(const char *s1, const char *s2);
unsigned int id_alloc(struct id *id);

int printk(const char *fmt, ...);
int sprintf(char *buf, const char *fmt, ...);

int ramdisk_read(struct buf *buf);
int ramdisk_write(struct buf *buf);
void ramdisk_init(void);

struct buf *bread(int bno);
struct buf *bootblock(void);
struct buf *superblock(void);
void brelease(struct buf *buf);
void bcachefree(void);
void bsync(void);
void block_init(void);

#endif
