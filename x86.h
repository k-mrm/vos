#ifndef _X86_64_H
#define _X86_64_H

#ifdef __ASSEMBLER__
#define UL(a)     a
#define ULL(a)    a
#else   // __ASSEMBLER__
#define UL(a)     a##ul
#define ULL(a)    a##ull
#endif

#define CR0_PE              0x1
#define CR0_PG              0x80000000
#define CR4_PAE             (1 << 5)

#define CPUID_0             0x0
#define CPUID_1             0x1
#define CPUID_1_ECX_X2APIC  0x200000
#define CPUID_1_EDX_MSR     0x20
#define CPUID_1_EDX_PAE     0x40
#define CPUID_1_EDX_APIC    0x200

#define CPUID_EXT             0x80000001
#define CPUID_EXT1_EDX_64BIT  0x20000000

#define CPUID_EXT2            0x80000002
#define CPUID_EXT3            0x80000003
#define CPUID_EXT4            0x80000004

#define IA32_EFER             0xc0000080
#define IA32_EFER_SCE         (1 << 0)
#define IA32_EFER_LME         (1 << 8)
#define IA32_EFER_LMA         (1 << 10)
#define IA32_EFER_NXE         (1 << 11)
#define IA32_EFER_SVME        (1 << 12)
#define IA32_EFER_LMSLE       (1 << 13)
#define IA32_EFER_FFXSR       (1 << 14)
#define IA32_EFER_TCE         (1 << 15)

#define IA32_APIC_BASE                    0x1b
#define IA32_APIC_BASE_ENABLE_X2APIC      0x400
#define IA32_APIC_BASE_APIC_GLOBAL_ENABLE 0x800
#define IA32_APIC_BASE_APIC_BASE_MASK     0xfffffffffffff000ull

#define EFLAGS_IF             (1 << 9)

#define INT_NMI         2

// Exceptions
#define E_DE    0x0
#define E_DB    0x1
#define E_BP    0x3
#define E_OF    0x4
#define E_BR    0x5
#define E_UD    0x6
#define E_NM    0x7
#define E_DF    0x8
#define E_TS    0xa
#define E_NP    0xb
#define E_SS    0xc
#define E_GP    0xd
#define E_PF    0xe

#define DPL_KERNEL	0x0
#define DPL_USER	0x3

#define STA_X	0x8
#define STA_W	0x2
#define STA_R	0x2

#define STA_TSSA	0x9

#define SEG_NULL        0x0
#define SEG_KCODE32     0x1
#define SEG_KDATA32     0x2
#define SEG_KCODE       0x3
#define SEG_KDATA       0x4
#define SEG_UCODE       0x5
#define SEG_UDATA       0x6
#define SEG_TSS         0x7
#define NR_SEG          9

#define PTE_P       (1 << 0)
#define PTE_W       (1 << 1)
#define PTE_U       (1 << 2)
#define PTE_PWT     (1 << 3)
#define PTE_PCD     (1 << 4)
#define PTE_A       (1 << 5)
#define PTE_D       (1 << 6)
#define PTE_G       (1 << 8)
#define PTE_XD      (1ull << 63)

/*
 *  x86 48bit Virtual Address
 *
 *     48    39 38    30 29    21 20    12 11       0
 *    +--------+--------+--------+--------+----------+
 *    | level4 | level3 | level2 | level1 | page off |
 *    +--------+--------+--------+--------+----------+
 *       pml4     pdpt      pd       pt
 *
 */

#define PIDX(_level, _addr) (((_addr) >> (12 + ((_level) - 1) * 9)) & 0x1ff)

#define PAGESIZE    0x1000
#define PAGESHIFT   12

#define PTE_PA_MASK ULL(0xfffffffff000)

// Direct mapping offset: 0xffff800000000000 - 0xffffc00000000000

#define KLINK_OFFSET    ULL(0xffff800000000000)
#define KERNLINK        ULL(0xffff800000100000)
#define KERNLINK_PA     ULL(0x100000)

#define PAGE_OFFSET     KLINK_OFFSET

#ifndef __ASSEMBLER__

#include <os3.h>

#define HLT   asm volatile ("hlt")

static inline void
outb(u16 port, u8 data)
{
        asm volatile ("outb %0, %1" :: "a"(data), "d"(port));
}

static inline void
outw(u16 port, u16 data)
{
        asm volatile ("outw %0, %1" :: "a"(data), "d"(port));
}

static inline void
outl(u16 port, u32 data)
{
        asm volatile ("outl %0, %1" :: "a"(data), "d"(port));
}

static inline u8
inb(u16 port)
{
        u8 data;
        asm volatile ("inb %1, %0" : "=a"(data) : "d"(port));
        return data;
}

static inline u16
inw(u16 port)
{
        u16 data;
        asm volatile ("inw %1, %0" : "=a"(data) : "d"(port));
        return data;
}

static inline u32
inl(u16 port)
{
        u32 data;
        asm volatile ("inl %1, %0" : "=a"(data) : "d"(port));
        return data;
}

static inline ulong
cr2(void)
{
        ulong r;
        asm volatile ("movq %%cr2, %0" : "=r"(r));
        return r;
}

static inline void
cpuid(u32 ax, u32 *a, u32 *b, u32 *c, u32 *d)
{
        asm volatile ("cpuid" : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d) : "a"(ax));
}

static inline void
__rdmsr(u32 reg, u32 *a, u32 *d)
{
        asm volatile ("rdmsr" : "=a"(*a), "=d"(*d) : "c"(reg));
}

static inline u32
rdmsr32(u32 reg)
{
        u32 a, d;
        __rdmsr(reg, &a, &d);
        return a;
}

static inline u64
rdmsr64(u32 reg)
{
        u32 a, d;
        __rdmsr(reg, &a, &d);
        return (u64)a | ((u64)d << 32);
}

static inline void
__wrmsr(u32 reg, u32 a, u32 d)
{
        asm volatile ("wrmsr" :: "c"(reg), "a"(a), "d"(d));
}

static inline void
wrmsr32(u32 reg, u32 val)
{
        __wrmsr(reg, val, 0);
}

static inline void
wrmsr64(u32 reg, u64 val)
{
        u32 a, d;
        a = (u32)val;
        d = (u32)(val >> 32);
        __wrmsr(reg, a, d);
}

static inline bool
interruptible(void)
{
        u64 rflags;
        asm volatile (
                "pushfq\n"
                "pop  %0\n" : "=r" (rflags)
        );

        return rflags & EFLAGS_IF;
}

static inline void
ltr(ushort sel)
{
        asm volatile ("ltr %0" :: "r" (sel));
}

#define INTR_DISABLE    asm volatile ("cli");
#define INTR_ENABLE     asm volatile ("sti");

struct tss
{
	u32 reserved0;
	u64 rsp0;
	u64 rsp1;
	u64 rsp2;
	u64 reserved1;
	u64 ist1;
	u64 ist2;
	u64 ist3;
	u64 ist4;
	u64 ist5;
	u64 ist6;
	u64 ist7;
	u64 reserved2;
	u16 reserved3;
	u16 io_map_base;
} PACKED;

static inline ulong
V2P(void *p)
{
        ulong va = (ulong)p;
        return va - PAGE_OFFSET;
}

static inline void *
P2V(ulong pa)
{
        return (void*)(pa + PAGE_OFFSET);
}

#endif  // __ASSEMBLER__
#endif  // _X86_64_H
