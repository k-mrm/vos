#ifndef _PROC_H
#define _PROC_H

#include <os3.h>
#include <util.h>

struct trapframe
{
        u64 rax;
        u64 rbx;
        u64 rcx;
        u64 rdx;
        u64 rbp;
        u64 rsi;
        u64 rdi;
        u64 r8;
        u64 r9;
        u64 r10;
        u64 r11;
        u64 r12;
        u64 r13;
        u64 r14;
        u64 r15;
        u64 trapno;
        u64 errcode;
        /* iret */
        u64 rip;
        u64 cs;
        u64 rflags;
        u64 rsp;
        u64 ss;
} PACKED;

struct context
{
	u64 r15;
	u64 r14;
	u64 r13;
	u64 r12;
	u64 rbx;
	u64 rbp;
	u64 rip;
} PACKED;

enum procstate
{
        NONE,
        READY,
        RUNNING,
        BLOCKING,
        ZOMBIE,
};

struct proc
{
        enum procstate state;
        char name[32];

	struct lock lock;

	u64 *pagetable;

	struct proc *parent;
	void *kstack;

	struct inode *cwd;

	uint pid;
	struct cpu *cpu;
	struct trapframe *tf;

	ulong rsp;

        struct list procs;	// node of proctable
	struct list rq;		// node of runqueue
	struct list waitq;	// head of waitqueue
	struct list wq;		// node of waitqueue
        int nchild;
        uchar exitstatus;
};

#endif	// _PROC_H
