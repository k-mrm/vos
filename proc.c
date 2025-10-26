#include <os3.h>
#include <util.h>
#include <proc.h>
#include <x86.h>

static LIST_HEAD(proctable);     // head
static struct lock plock;
static LIST_HEAD(rq);	// head
static struct lock rqlock;
static int nproc = 0;
static struct id pidpool;
static u64 kernelsp;

#define FOREACH_PROC(_p)     \
        LIST2_FOREACH (_p, &proctable, procs)

static void
ready(struct proc *p)
{
	if (p->state != READY)
		return;

	p->state = READY;

	list_add(&rq, &p->rq);
}

static struct proc *
readyproc(void)
{
	struct proc *p;

	if (list_empty(&rq))
		return NULL;

	p = ENTRY(&rq, struct proc, rq);
	list_delete(&p->rq);

	return p;
}

static void
die(struct proc *p)
{
        if (!p)
                return;
        p->state = ZOMBIE;
}

static void
running(struct proc *p)
{
        if (!p)
                return;
        p->state = RUNNING;
}

static void
block(struct proc *p)
{
	if (!p)
		return;
	p->state = BLOCKING;
}

static Proc *
newproc(char *name, Proc *parent)
{
        // Proc *p = malloc (sizeof *p);
	u64 rflags;
        struct proc *p = zalloc();
        void *sp;

        if (!p)
                return NULL;

        p->procid = allocid(&pidpool);
	list_init(&p->waitq);

        p->parent = parent;

        strcpy(p->pname, name);

        p->kstack = zalloc();
        if (!p->kstack)
                goto err;
        sp = p->kstack + PAGESIZE;
        sp -= sizeof *p->tf;

	asm volatile (
		"pushfq\n"
		"pop  %0\n" : "=r" (rflags)
	);

	((struct trapframe *)sp)->rip = 0x1000;
	((struct trapframe *)sp)->cs = (SEG_UCODE << 3) | DPL_USER;
	((struct trapframe *)sp)->rflags = rflags | EFLAGS_IF;
	((struct trapframe *)sp)->ss = (SEG_UDATA << 3) | DPL_USER;
	((struct trapframe *)sp)->rsp = USTACKTOP;

        p->tf = sp;

        sp -= sizeof(struct context);
        ((struct context *)sp)->rip = (u64)forkret;

        p->rsp = (u64)sp;

        list_add(&proctable, &p->procs);

        return p;

err:
        free(p);
        return NULL;
}

static void
freeproc(struct proc *p)
{
        printk("freed %s\n", p->pname);

        list_delete(&p->procs);

        // freevas (p->as);
        free(p->kstack);
        memset(p, sizeof *p, 0);
        free(p);
}

static void
initproc(void)
{
        struct proc *p;
        extern char _binary_initcode_start[];
        extern char _binary_initcode_end[];
        int isize = _binary_initcode_end - _binary_initcode_start;
        void *initcode;

        p = newproc("init0", NULL);
        if (!p)
		panic("init");

	p->pagetable = uservm(p);
        p->cwd = path2ino("/");

        initcode = zalloc();
	if (!initcode)
		panic("init");
        memcpy(initcode, _binary_initcode_start, isize);
        mappages(p->pagetable, 0x1000, V2P(initcode), PAGESIZE,
		 PTE_ | EXECUTBLE | USER, false);

        ready(p);
}

void
process_init(void)
{
        initproc();
}

int
exec(const char *path, const char **argv)
{
        struct inode *elf;
        struct fs *fs;
        struct ehdr ehdr;
        struct phdr phdr;
        int status, size = 0, memsz = 0;
        u64 phoff, flags;
        struct cpu *cpu = mycpu ();
        struct proc *proc = cpu->currentproc;
        void *p;
        int uargc = 0;
        u64 *pt = uservm(proc);
        u64 *oldpt = proc->pagetable;
        void *top = as->ustack + PAGESIZE;
        void *sp = top;
        u64 args[10] = {0};

        elf = path2ino(path);
        if (!elf)
                goto err;

        fs = elf->fs;
        status = fs->op->readi(elf, (uchar *)&ehdr, 0, sizeof ehdr);
        if (status != sizeof (ehdr))
                goto err;
        if (!iself (&ehdr))
                goto err;
        if (ehdr.e_type != ET_EXEC)
                goto err;

        as->ustart = (void *)0x1000;

        phoff = ehdr.e_phoff;
        for (int i = 0; i < ehdr.e_phnum; i++, phoff += sizeof phdr) {
                p = zalloc();
                flags = 0;

                status = fs->op->readi(elf, (uchar *)&phdr, phoff, sizeof phdr);
                if (status != sizeof phdr)
                        goto err;
                if (phdr.p_type != PT_LOAD)
                        continue;

                if (!PAGEALIGNED(phdr.p_vaddr))
                        panic ("o");

                flags |= phdr.p_flags & PF_X ? PTE_X : 0;
                flags |= phdr.p_flags & PF_W ? PTE_W : 0;

                vm_alloc_code(as, phdr.p_memsz, flags);

                size = fs->op->readi(elf, p, phdr.p_offset, phdr.p_filesz);
                copyin(as, phdr.p_vaddr, p, size);
        }

        // setup arguments
        for (; argv && argv[uargc]; uargc++) {
                sp -= strlen (argv[uargc]) + 1;
                sp = (void *)((u64)sp & ~0xf);
                if (sp < as->ustack)
                        goto err;

                memcpy(sp, argv[uargc], strlen (argv[uargc]));
                args[uargc] = USTACKTOP - (top - sp);
        }

        sp -= sizeof (args[0]) * (uargc + 1);
        sp = (void *)((u64)sp & ~0xf);
        if (sp < as->ustack)
                goto err;
        memcpy (sp, args, sizeof (args[0]) * uargc);

        proc->tf->rip = ehdr.e_entry;
        proc->tf->rdi = uargc;
        proc->tf->rsi = USTACKTOP - (top - sp);
        proc->tf->rsp = USTACKTOP - (top - sp);
        proc->pagetable = pt;
        strcpy(proc->pname, path);

        free_uservm(oldpt);

        uswitch(proc);

        return 0;
err:
        panic("err");
        free_uservm(pt);
        return -1;
}

int
fork(void)
{
        struct cpu *cpu = mycpu();
        struct proc *proc = cpu->proc;
        struct proc *np = newproc(proc->name, proc);

        if (!np)
                return -1;

        printk("fork %d %s %p %p\n", np->procid, proc->name, np->tf, proc->tf);

        proc->nchild++;

        memcpy(np->tf, proc->tf, sizeof *np->tf);
        np->tf->rax = 0;
        np->cwd = idup(proc->cwd);

	np->pagetable = clone_uservm(proc);

        ready(np);

        return np->procid;
}

int
exit(int status)
{
        struct cpu *cpu  = mycpu();
        struct proc *proc = cpu->proc;
        struct proc *pp = proc->parent;

        proc->exitstatus = status;

        die(proc);

	list_add(&pp->waitq, &proc->wq);
	wakeup(pp);

        schedule();
        // never return here
        panic("zombie");
}

static bool
wqempty(struct proc *p)
{
        return list_empty(&p->waitq);
}

int
wait(int *status)
{
        struct cpu *cpu = mycpu ();
        struct proc *proc = cpu->proc;
        struct proc *cp;
        int pid;

        if (proc->nchild == 0)
                return -1;

        while (wqempty(proc))
                sleep(proc);

        cp = ENTRY(&proc->waitq, struct proc, wq);
        list_delete(&cp->wq);
        if (cp->state != ZOMBIE)
                panic("zombie");

        pid = cp->pid; 
        if (status)
                *status = cp->exitstatus;

        freeproc(cp);
        proc->nchild--;

        return pid;
}

static struct proc *
candidate(struct proc *prev)
{
        struct proc *p;

        if (prev && prev->state == RUNNING)
                ready(prev);

        return readyproc();
}

void
sleep(void *chan)
{
        struct proc *p = mycpu()->proc;

        p->chan = chan;
        block(p);

        schedule();

        p->chan = NULL;
}

void
wakeup(void *chan)
{
        struct proc *p;

        FOREACH_PROC (p) {
                if (p->state == BLOCKING && p->chan == chan)
                        ready(p);
        }
}

void
schedtail(void)
{
	;
}

void
schedule(void)
{
        struct cpu *cpu  = mycpu ();
        struct proc *prev = cpu->proc;
        struct proc *next = candidate(prev); 
	u64 *psp;

        if (!next)
                panic("next process!?");

	cpu->proc = next;
	next->cpu = cpu;
        running(next);

        printk("cswitch: prev %p(%s) -> %p(%s)\n",
               prev, prev ? prev->pname : "NULL", next, next->pname);

        if (!prev)
                psp = &kernelsp;
        else
                psp = &prev->rsp;

	uswitch(next);

        prev = cswitch(psp, &next->rsp, prev);

        printk("cswitch returned: last:%s c:%s %d\n",
               prev ? prev->name : "kernel", cpu->proc->name, interruptible());
}
