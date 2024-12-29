#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "prioritylock.h"
#include "string.h"
#include "syscall.h"  // Add this line to include the getnsyscall function

// Declare the getnsyscall function
extern void getnsyscall(void);
extern int sys_nsyscalls(void);

struct proc* select_round_robin(void);
struct proc* select_sjf(void);
struct proc* select_fcfs(void);

struct {
    struct spinlock lock;
    struct proc proc[NPROC];
} ptable;

struct prioritylock plock;

static struct proc* initproc;

int nextpid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void* chan);

void pinit(void) {
    initlock(&ptable.lock, "ptable");
    initprioritylock(&plock, "plock");
}

// Must be called with interrupts disabled
int cpuid() {
    return mycpu() - cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void) {
    int apicid, i;

    if (readeflags() & FL_IF)
        panic("mycpu called with interrupts enabled\n");

    apicid = lapicid();
    // APIC IDs are not guaranteed to be contiguous. Maybe we should have
    // a reverse map, or reserve a register to store &cpus[i].
    for (i = 0; i < ncpu; ++i) {
        if (cpus[i].apicid == apicid)
            return &cpus[i];
    }
    panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
    struct cpu* c;
    struct proc* p;
    pushcli();
    c = mycpu();
    p = c->proc;
    popcli();
    return p;
}

// PAGEBREAK: 32
//  Look in the process table for an UNUSED proc.
//  If found, change state to EMBRYO and initialize
//  state required to run in the kernel.
//  Otherwise return 0.
static struct proc*
allocproc(void) {
    struct proc* p;
    char* sp;

    acquire(&ptable.lock);

    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
        if (p->state == UNUSED)
            goto found;

    release(&ptable.lock);
    return 0;

found:
    p->state = EMBRYO;
    p->pid = nextpid++;

    release(&ptable.lock);

    // Allocate kernel stack.
    if ((p->kstack = kalloc()) == 0) {
        p->state = UNUSED;
        return 0;
    }
    sp = p->kstack + KSTACKSIZE;

    // Leave room for trap frame.
    sp -= sizeof *p->tf;
    p->tf = (struct trapframe*)sp;

    // Set up new context to start executing at forkret,
    // which returns to trapret.
    sp -= 4;
    *(uint*)sp = (uint)trapret;

    sp -= sizeof *p->context;
    p->context = (struct context*)sp;
    memset(p->context, 0, sizeof *p->context);
    p->context->eip = (uint)forkret;

    memset(&p->sched, 0, sizeof(p->sched));
    p->sched.queue = UNSET;
    p->sched.sjf.priority = NORMAL;
    p->sched.sjf.arrival_time = 0;
    p->sched.sjf.executed_cycle = 0;
    p->sched.sjf.process_size = 0;
    p->sched.sjf.burst_time = 2;

    p->sched.sjf.priority_ratio = 1;


    // Set default confidence level and burst time for SJF
    p->sched.sjf.confidence_level = 50;
    p->sched.sjf.burst_time = 2;

    // Initialize consecutive run and wait time
    p->consecutive_run = 0;
    p->wait_time = 0;

    return p;
}

// PAGEBREAK: 32
//  Set up first user process.
void userinit(void) {
    struct proc* p;
    extern char _binary_initcode_start[], _binary_initcode_size[];

    p = allocproc();

    initproc = p;
    if ((p->pgdir = setupkvm()) == 0)
        panic("userinit: out of memory?");
    inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
    p->sz = PGSIZE;
    memset(p->tf, 0, sizeof(*p->tf));
    p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
    p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
    p->tf->es = p->tf->ds;
    p->tf->ss = p->tf->ds;
    p->tf->eflags = FL_IF;
    p->tf->esp = PGSIZE;
    p->tf->eip = 0; // beginning of initcode.S

    safestrcpy(p->name, "initcode", sizeof(p->name));
    p->cwd = namei("/");

    // this assignment to p->state lets other cores
    // run this process. the acquire forces the above
    // writes to be visible, and the lock is also needed
    // because the assignment might not be atomic.
    acquire(&ptable.lock);

    p->state = RUNNABLE;

    release(&ptable.lock);

    init_queue(p->pid);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int growproc(int n) {
    uint sz;
    struct proc* curproc = myproc();

    sz = curproc->sz;
    if (n > 0) {
        if ((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
            return -1;
    }
    else if (n < 0) {
        if ((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
            return -1;
    }
    curproc->sz = sz;
    switchuvm(curproc);
    return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int fork(void) {
    int i, pid;
    struct proc* np;
    struct proc* curproc = myproc();

    // Allocate process.
    if ((np = allocproc()) == 0) {
        return -1;
    }

    // Copy process state from proc.
    if ((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0) {
        kfree(np->kstack);
        np->kstack = 0;
        np->state = UNUSED;
        return -1;
    }
    np->sz = curproc->sz;
    np->parent = curproc;
    *np->tf = *curproc->tf;

    // Clear %eax so that fork returns 0 in the child.
    np->tf->eax = 0;

    for (i = 0; i < NOFILE; i++)
        if (curproc->ofile[i])
            np->ofile[i] = filedup(curproc->ofile[i]);
    np->cwd = idup(curproc->cwd);

    safestrcpy(np->name, curproc->name, sizeof(curproc->name));

    pid = np->pid;

    acquire(&ptable.lock);

    np->state = RUNNABLE;

    release(&ptable.lock);

    acquire(&tickslock);
    np->sched.last_exec = ticks;
    np->sched.sjf.arrival_time = ticks;
    np->ctime = ticks;
    release(&tickslock);

    // IDK, maybe ?!
    np->sched.sjf.process_size = sizeof(np);

    init_queue(np->pid);

    return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void exit(void) {
    struct proc* curproc = myproc();
    struct proc* p;
    int fd;

    //
    if (isholdingpriority(&plock))
        releasepriority(&plock);

    if (curproc == initproc)
        panic("init exiting");

    // Close all open files.
    for (fd = 0; fd < NOFILE; fd++) {
        if (curproc->ofile[fd]) {
            fileclose(curproc->ofile[fd]);
            curproc->ofile[fd] = 0;
        }
    }

    begin_op();
    iput(curproc->cwd);
    end_op();
    curproc->cwd = 0;

    acquire(&ptable.lock);

    // Parent might be sleeping in wait().
    wakeup1(curproc->parent);

    // Pass abandoned children to init.
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->parent == curproc) {
            p->parent = initproc;
            if (p->state == ZOMBIE)
                wakeup1(initproc);
        }
    }

    // Jump into the scheduler, never to return.
    curproc->state = ZOMBIE;
    sched();
    panic("zombie exit");
}

int wait(void) {
    struct proc* p;
    int havekids, pid;
    struct proc* curproc = myproc();

    acquire(&ptable.lock);
    for (;;) {
        // Scan through table looking for exited children.
        havekids = 0;
        for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
            if (p->parent != curproc)
                continue;
            havekids = 1;
            if (p->state == ZOMBIE) {
                // Found one.
                pid = p->pid;
                kfree(p->kstack);
                p->kstack = 0;
                freevm(p->pgdir);
                p->pid = 0;
                p->parent = 0;
                p->name[0] = 0;
                p->killed = 0;
                p->state = UNUSED;
                // p->wait_time = 0; // Reset wait time when process is cleaned up
                release(&ptable.lock);
                return pid;
            }
        }

        // No point waiting if we don't have any children.
        if (!havekids || curproc->killed) {
            release(&ptable.lock);
            return -1;
        }

        // Wait for children to exit.  (See wakeup1 call in proc_exit.)
        sleep(curproc, &ptable.lock); // DOC: wait-sleep
    }
}
void aging(int curr_time) {
    struct proc* p;
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state == RUNNABLE && p->sched.queue > ROUND_ROBIN) {
            if (curr_time - p->sched.last_exec > MAX_AGE) {
                int old_queue = p->sched.queue;
                p->sched.queue--;
                p->sched.last_exec = ticks;
                p->sched.last_termination = ticks;
                cprintf("Process %d promoted from queue %d to queue %d\n", 
                        p->pid, old_queue, p->sched.queue);
            }
        }
    }
}

int init_queue(int pid) {
    struct proc* p;
    int queue;

    // init and shell
    if (pid == 1 || pid == 2)
        queue = ROUND_ROBIN;
    else if (pid > 2)
        queue = FCFS;  // Default queue (level 3)
    else
        return -1;

    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->pid == pid) {
            p->sched.queue = queue;
            p->wait_time = 0;
            p->sched.last_exec = ticks;
            break;
        }
    }
    release(&ptable.lock);
    return 0;
}

int change_queue(int pid, int new_queue) {
    struct proc* p;
    if (new_queue == UNSET) {
        return -1;
    }

    int is_process_exist = 0;
    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->pid == pid) {
            is_process_exist = 1;
            p->sched.queue = new_queue;
            acquire(&tickslock);
            p->sched.last_exec = ticks;
            release(&tickslock);
            break;
        }
    }
    release(&ptable.lock);

    if (!is_process_exist)
        return -1;

    return 0;
}

int set_sjf_proc(int pid, int priority_ratio, int burst_time) {
    struct proc* p;
    int is_pid_exist = 0;
    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->pid == pid) {
            is_pid_exist = 1;
            p->sched.sjf.priority_ratio = priority_ratio;
            p->sched.sjf.burst_time = burst_time;
            break;
        }
    }
    release(&ptable.lock);

    if (!is_pid_exist)
        return -1;
    return 0;
}

int set_sjf_sys(int priority_ratio, int burst_time) {
    struct proc* p;
    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        p->sched.sjf.priority_ratio = priority_ratio;
        p->sched.sjf.burst_time = burst_time;
    }
    release(&ptable.lock);

    return 0;
}

void printspaces(int count) {
    for (int i = 0; i < count; ++i)
        cprintf(" ");
}
int digitcount(int num) {
    if (num == 0) return 1;
    int count = 0;
    while (num) {
        num /= 10;
        count++;
    }
    return count;
}
int print_processes_infos(void) {
    static char* states[] = {
        [UNUSED] "unused",
        [EMBRYO] "embryo",
        [SLEEPING] "sleeping",
        [RUNNABLE] "runnable",
        [RUNNING] "running",
        [ZOMBIE] "zombie"};

    static int columns[] = {16, 8, 12, 8, 8, 8, 8, 8, 8, 8};
    cprintf(
        "Process_Name    PID     State     Queue   Wait time   Confidence  Burst Time  Consecutive Run    Arrival\n"
        "---------------------------------------------------------------------------------------------------------------\n");

    struct proc* p;
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state == UNUSED)
            continue;

        const char* state;
        if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
            state = states[p->state];
        else
            state = "???";

        cprintf("%s", p->name);
        printspaces(columns[0] - strlen(p->name));

        cprintf("%d", p->pid);
        printspaces(columns[1] - digitcount(p->pid));

        cprintf("%s", state);
        printspaces(columns[2] - strlen(state));

        cprintf("%d", p->sched.queue);
        printspaces(columns[3] - digitcount(p->sched.queue));

        cprintf("%d        ", p->wait_time);
        printspaces(columns[4] - digitcount(p->wait_time));

        cprintf("%d", p->sched.sjf.confidence_level);
        printspaces(columns[5] - digitcount(p->sched.sjf.confidence_level));

        cprintf("%d     ", p->sched.sjf.burst_time);
        printspaces(columns[6] - digitcount(p->sched.sjf.burst_time));

        cprintf("%d         ", p->consecutive_run);
        printspaces(columns[7] - digitcount(p->consecutive_run));

        cprintf("%d", p->sched.sjf.arrival_time);
        printspaces(columns[8] - digitcount(p->sched.sjf.arrival_time));

        cprintf("\n");
    }

    return 0;
}

struct proc* first_come_first_serve(void)
{
    struct proc *selected = 0;
    int earliest_arrival = 0x7fffffff;

    for (struct proc *p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state != RUNNABLE || p->sched.queue != FCFS)
            continue;
        if (p->sched.sjf.arrival_time < earliest_arrival) {
            earliest_arrival = p->sched.sjf.arrival_time;
            selected = p;
        }
    }
    return selected;
}


struct proc* round_robin(struct proc* last_scheduled) {
    struct proc* p = last_scheduled + 1;
    for (;; p++) {
        // hit the end of queue
        if (p >= &ptable.proc[NPROC])
            p = ptable.proc;

        // found next proc
        if (p->state == RUNNABLE && p->sched.queue == ROUND_ROBIN)
            return p;

        // empty queue
        if (p == last_scheduled)
            return 0;
    }
}

int procrank_burst_time(struct proc* p) {
    return p->sched.sjf.burst_time;
}

struct proc* best_job_first(void) {
    struct proc* next_p = 0;
    int best_burst = 10000000;

    struct proc* p;
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state != RUNNABLE || p->sched.queue != SJF)
            continue;
        if (next_p == 0 || p->sched.sjf.burst_time < best_burst) {
            next_p = p;
            best_burst = p->sched.sjf.burst_time;
        }
    }

    return next_p;
}

// PAGEBREAK: 42
//  Per-CPU process scheduler.
//  Each CPU calls scheduler() after setting itself up.
//  Scheduler never returns.  It loops, doing:
//   - choose a process to run
//   - swtch to start running that process
//   - eventually that process transfers control
//       via swtch back to the scheduler.
#define ROUND_ROBIN_TIME_SLICE 50
#define ROUND_ROBIN_TICKS 5
#define WEIGHT_ROUND_ROBIN 3
#define WEIGHT_SJF 2
#define WEIGHT_FCFS 1

void scheduler(void) {
    struct cpu* c = mycpu();
    c->proc = 0;
    c->queue_weights[0] = WEIGHT_ROUND_ROBIN;
    c->queue_weights[1] = WEIGHT_SJF;
    c->queue_weights[2] = WEIGHT_FCFS;
    
    struct proc* last_proc = 0;  // Track the last process that ran

    for (;;) {
        sti(); // Enable interrupts on this processor.

        acquire(&ptable.lock);

        // Implement aging
        struct proc* p;
        for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
            if (p->state == RUNNABLE) {
                p->wait_time++;
                if (p->wait_time > MAX_AGE && p->sched.queue > ROUND_ROBIN) {
                    cprintf("Process %d aged from queue %d to queue %d\n", p->pid, p->sched.queue, p->sched.queue - 1);
                    p->sched.queue--;
                    p->wait_time = 0;
                }
            }
        }

        // Multilevel Feedback Queue Scheduling
        struct proc* selected = 0;
        int level;
        for (level = ROUND_ROBIN; level <= FCFS; level++) {
            selected = 0;
            if (level == ROUND_ROBIN) {
                selected = select_round_robin();
            } else if (level == SJF) {
                selected = select_sjf();
            } else if (level == FCFS) {
                selected = select_fcfs();
            }

            if (selected)
                break;
        }

        if (selected != 0) {
            // Update consecutive run counter
            if (last_proc == selected) {
                selected->consecutive_run++;
            } else {
                selected->consecutive_run = 1;
            }
            last_proc = selected;

            c->proc = selected;
            switchuvm(selected);
            selected->state = RUNNING;
            selected->wait_time = 0; // Reset wait time when selected to run
            swtch(&(c->scheduler), selected->context);
            switchkvm();
            c->proc = 0;
        }

        release(&ptable.lock);
    }
}


// Simple random number generator
uint random_seed = 1;
int random(void) {
    random_seed = random_seed * 1664525 + 1013904223;
    return random_seed;
}



struct proc* select_round_robin(void) {
    static struct proc* last_scheduled = 0;
    static int ticks = 0;
    struct proc* p = last_scheduled ? last_scheduled + 1 : ptable.proc;
    for (;; p++) {
        if (p >= &ptable.proc[NPROC])
            p = ptable.proc;
        if (p->state == RUNNABLE && p->sched.queue == ROUND_ROBIN) {
            if (ticks < ROUND_ROBIN_TICKS) {
                // ticks++;
                last_scheduled = p;
                return p;
            } else {
                ticks = 0;
                last_scheduled = p;
                return p;
            }
        }
        if (p == last_scheduled)
            return 0;
    }
}

struct proc* select_sjf(void) {
    struct proc* selected = 0;
    float best_rank = -1;

    for (struct proc* p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state != RUNNABLE || p->sched.queue != SJF)
            continue;
        float rank = procrank_burst_time(p);
        if (selected == 0 || rank < best_rank) {
            selected = p;
            // best_burst = p->sched.sjf.burst_time;
        }
    }
    if (selected) {
        int random_value = random() % 100;
        if (random_value <= selected->sched.sjf.confidence_level) {
            return selected;
        }
    }
    return selected;
}

struct proc* select_fcfs(void) {
    struct proc* selected = 0;
    int earliest_arrival = 0x7fffffff;
    for (struct proc* p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state != RUNNABLE || p->sched.queue != FCFS)
            continue;
        if (p->sched.sjf.arrival_time < earliest_arrival) {
            earliest_arrival = p->sched.sjf.arrival_time;
            selected = p;
        }
    }
    return selected;
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void sched(void) {
    int intena;
    struct proc* p = myproc();

    if (!holding(&ptable.lock))
        panic("sched ptable.lock");
    if (mycpu()->ncli != 1)
        panic("sched locks");
    if (p->state == RUNNING)
        panic("sched running");
    if (readeflags() & FL_IF)
        panic("sched interruptible");
    intena = mycpu()->intena;
    swtch(&p->context, mycpu()->scheduler);
    mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void yield(void) {
    acquire(&ptable.lock); // DOC: yieldlock
    myproc()->state = RUNNABLE;
    sched();
    release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void forkret(void) {
    static int first = 1;
    // Still holding ptable.lock from scheduler.
    release(&ptable.lock);

    if (first) {
        // Some initialization functions must be run in the context
        // of a regular process (e.g., they call sleep), and thus cannot
        // be run from main().
        first = 0;
        iinit(ROOTDEV);
        initlog(ROOTDEV);
    }

    // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void sleep(void* chan, struct spinlock* lk) {
    struct proc* p = myproc();

    if (p == 0)
        panic("sleep");

    if (lk == 0)
        panic("sleep without lk");

    // Must acquire ptable.lock in order to
    // change p->state and then call sched.
    // Once we hold ptable.lock, we can be
    // guaranteed that we won't miss any wakeup
    // (wakeup runs with ptable.lock locked),
    // so it's okay to release lk.
    if (lk != &ptable.lock) {  // DOC: sleeplock0
        acquire(&ptable.lock); // DOC: sleeplock1
        release(lk);
    }
    // Go to sleep.
    p->chan = chan;
    p->state = SLEEPING;
    // p->wait_time = 0; // Reset wait time when process goes to sleep

    sched();

    // Tidy up.
    p->chan = 0;

    // Reacquire original lock.
    if (lk != &ptable.lock) { // DOC: sleeplock2
        release(&ptable.lock);
        acquire(lk);
    }
}


// PAGEBREAK!
//  Wake up all processes sleeping on chan.
//  The ptable lock must be held.
static void
wakeup1(void* chan) {
    struct proc* p;

    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++)
        if (p->state == SLEEPING && p->chan == chan)
            p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void wakeup(void* chan) {
    acquire(&ptable.lock);
    wakeup1(chan);
    release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int kill(int pid) {
    struct proc* p;

    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->pid == pid) {
            p->killed = 1;
            // Wake process from sleep if necessary.
            if (p->state == SLEEPING) {
                p->state = RUNNABLE;
                // p->wait_time = 0; // Reset wait time when process is woken up
            }
            release(&ptable.lock);
            return 0;
        }
    }
    release(&ptable.lock);
    return -1;
}

// PAGEBREAK: 36
//  Print a process listing to console.  For debugging.
//  Runs when user types ^P on console.
//  No lock to avoid wedging a stuck machine further.

// PAGEBREAK: 36
//  Print a process listing to console.  For debugging.
//  Runs when user types ^P on console.
//  No lock to avoid wedging a stuck machine further.
void procdump(void) {
    static char* states[] = {
        [UNUSED] "unused",
        [EMBRYO] "embryo",
        [SLEEPING] "sleep ",
        [RUNNABLE] "runble",
        [RUNNING] "run   ",
        [ZOMBIE] "zombie"};
    int i;
    struct proc* p;
    char* state;
    uint pc[10];

    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state == UNUSED)
            continue;
        if (p->state >= 0 && p->state < NELEM(states) && states[p->state])
            state = states[p->state];
        else
            state = "???";
        cprintf("%d %s %s", p->pid, state, p->name);
        if (p->state == SLEEPING) {
            getcallerpcs((uint*)p->context->ebp + 2, pc);
            for (i = 0; i < 10 && pc[i] != 0; i++)
                cprintf(" %p", pc[i]);
        }
        cprintf("\n");
    }
}

int nuncle() {
    // get grandparent
    struct proc* current_proc = myproc();
    struct proc* grandparent = current_proc->parent->parent;

    // count all process' grandparent children, then minus 1
    int uncles = 0;
    struct proc* p;
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->parent == grandparent)
            uncles++;
    }
    return uncles - 1;
}

int ptime() {
    struct proc* current_proc = myproc();
    return ticks - current_proc->ctime;
}

int droot(int n) {
    while (n > 9) {
        int sum_digits = 0;
        int temp = n;
        while (temp > 0) {
            sum_digits += temp % 10;
            temp /= 10;
        }
        n = sum_digits;
    }
    return n;
}

int nsyscalls(void) {
    return sys_nsyscalls();  // Return the value from sys_nsyscalls
}

int pacquire(void) {
    if (isholdingpriority(&plock))
        return -1;
    acquirepriority(&plock);
    return myproc()->pid;
}

int prelease(void) {
    if (!isholdingpriority(&plock))
        return -1;
    releasepriority(&plock);
    return 0;
}

int pqueue(void) {
    showlockqueue(&plock);
    return 0;
}


int get_most_invoked(void){

    int pid;
    struct proc *p;

    if (argint(0, &pid) < 0)
        return -1;

    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->pid == pid) {
            if (p->syscall_count == 0) {
                cprintf("No system calls invoked by process %d\n", pid);
                release(&ptable.lock);
                return -1;
            }
            int counts[NELEM(syscall_counts)] = {0};
            for (int i = 0; i < p->syscall_count; i++) {
                counts[p->syscalls[i]]++;
            }
            int max_syscall = 0;
            int max_count = 0;
            for (int i = 0; i < NELEM(syscall_counts); i++) {
                if (counts[i] > max_count) {
                    max_syscall = i;
                    max_count = counts[i];
                }
            }
            cprintf("Most invoked system call: %d, invoked %d times\n", max_syscall, max_count);
            release(&ptable.lock);
            return 0;
        }
    }
    release(&ptable.lock);
    cprintf("Process %d not found\n", pid);
    return -1;
}

// Fix list_proceases return type
int list_proceases(void) {
    struct proc *p;
    acquire(&ptable.lock);
    cprintf("PID    Syscall Count  Process Name\n");
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state != UNUSED && p->state != ZOMBIE) {
            cprintf("%d      %d            %s\n", p->pid, p->syscall_count, p->name);
        }
    }
    release(&ptable.lock);
    return 0;
}

// Fix sort_syscalls return type
int sort_syscalls(void) {
    int pid;
    struct proc *p;

    if (argint(0, &pid) < 0)
        return -1;

    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->pid == pid) {
            for (int i = 0; i < MAX_SYSCALLS; i++) {
                if(p->syscalls[i] != 0){
                    cprintf("System Call ID: %d    Number of Calls: %d\n", i,p->syscalls[i]);
                }
            }
            release(&ptable.lock);
            return 0;
        }
    }
    release(&ptable.lock);
    return -1;
}

int bjsproc(int pid, float a, float b, float c, float d) {
    // Implement the function logic here
    return 0;
}

int bjssys(float a, float b, float c, float d) {
    // Implement the function logic here
    return 0;
}

int set_SJF_params(int pid, int burst_time, int confidence) {
    struct proc *p;
    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->pid == pid) {
            p->sched.sjf.burst_time = burst_time;
            p->sched.sjf.confidence_level = confidence;
            release(&ptable.lock);
            return 0;
        }
    }
    release(&ptable.lock);
    return -1;
}


