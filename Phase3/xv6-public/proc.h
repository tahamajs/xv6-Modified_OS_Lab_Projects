#pragma once
#include "spinlock.h"
// Per-CPU state
struct cpu {
    uchar apicid;              // Local APIC ID
    struct context* scheduler; // swtch() here to enter scheduler
    struct taskstate ts;       // Used by x86 to find stack for interrupt
    struct segdesc gdt[NSEGS]; // x86 global descriptor table
    volatile uint started;     // Has the CPU started?
    int ncli;                  // Depth of pushcli nesting.
    int intena;                // Were interrupts enabled before pushcli?
    struct proc* proc;         // The process running on this cpu or null
    int nsyscall;              // number of system calls
};



extern struct cpu cpus[NCPU];
extern int ncpu;

// PAGEBREAK: 17
//  Saved registers for kernel context switches.
//  Don't need to save all the segment registers (%cs, etc),
//  because they are constant across kernel contexts.
//  Don't need to save %eax, %ecx, %edx, because the
//  x86 convention is that the caller has saved them.
//  Contexts are stored at the bottom of the stack they
//  describe; the stack pointer is the address of the context.
//  The layout of the context matches the layout of the stack in swtch.S
//  at the "Switch stacks" comment. Switch doesn't save eip explicitly,
//  but it is on the stack and allocproc() manipulates it.
struct context {
    uint edi;
    uint esi;
    uint ebx;
    uint ebp;
    uint eip;
};

enum procstate { UNUSED,
                 EMBRYO,
                 SLEEPING,
                 RUNNABLE,
                 RUNNING,
                 ZOMBIE };

#define MAX_AGE 8000 

// schedule queue
enum schedqueue {
    UNSET,
    ROUND_ROBIN,
    LCFS,
    FCFS,
    BJF
};

// BJF priority
enum schedpriroty {
    HIGH = 1,
    ABOVE_NORMAL,
    NORMAL,
    BELOW_NORMAL,
    LOW
};

// bjs parameters
struct bjfparams {
    enum schedpriroty priority;
    float executed_cycle;
    int arrival_time;
    int process_size;

    float priority_ratio;
    float executed_cycle_ratio;
    float arrival_time_ratio;
    float process_size_ratio;
};

// schedule info
struct schedparams {
    enum schedqueue queue;
    struct bjfparams bjf;
    int last_exec;
};







// Per-process state
struct proc {
<<<<<<< HEAD
    uint sz;                    // Size of process memory (bytes)
    pde_t* pgdir;               // Page table
    char* kstack;               // Bottom of kernel stack for this process
    enum procstate state;       // Process state
    int pid;                    // Process ID
    struct proc* parent;        // Parent process
    struct trapframe* tf;       // Trap frame for current syscall
    struct context* context;    // swtch() here to run process
    void* chan;                 // If non-zero, sleeping on chan
    int killed;                 // If non-zero, have been killed
    struct file* ofile[NOFILE]; // Open files
    struct inode* cwd;          // Current directory
    char name[16];              // Process name (debugging)
    uint ctime;                 // created time
    struct schedparams sched;   // scheduling parameters
    uint shmemaddr;             // address of shared-memory
    int syscalls[100];
    int wait_time;   // Time the process has been waiting in RUNNABLE state
    int syscall_count ;
=======
  uint sz;                     // Size of process memory (bytes)
  pde_t* pgdir;                // Page table
  char *kstack;                // Bottom of kernel stack for this process
  enum procstate state;        // Process state
  int pid;                     // Process ID
  struct proc *parent;         // Parent process
  struct trapframe *tf;        // Trap frame for current syscall
  struct context *context;     // swtch() here to run process
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  int syscalls[MAX_SYSCALLS];  // Array to hold count of each syscall for this process
  int syscall_count;  
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)
  int creation_time;           // Time of creation (in ticks)
  struct schedinfo sched_info; // Scheduling information
  sharedPages pages[SHAREDREGIONS]; // Shared memory pages info
  int arrival_time;     // زمان ورود پردازه (برای FCFS)
  int estimated_runtime; // زمان اجرای تخمینی (برای SJF)
  int priority_level;    // سطح اولویت پردازه (برای Aging)
  int queue_level;       // سطح صف پردازه (0: RR, 1: SJF, 2: FCFS)
  int wait_time;         // زمان انتظار پردازه
  int runticks;          // Number of ticks this process has run
  int weight;            // Weight for WRR scheduling
  int queue_weight;      // Weight of current queue (1,2,3)

  // Scheduling information
  int queue_level;         // 0: LEVEL1_RR, 1: LEVEL2_SJF, 2: LEVEL3_FCFS
  int estimated_runtime;   // Estimated runtime for SJF
  int confidence_level;    // Confidence level (0-100)
  int wait_time;           // Time spent waiting in ticks
  int arrival_time;        // Arrival time in ticks
>>>>>>> 449691d (feat: Implement Multilevel Feedback Queue Scheduler with Aging)
};

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap

// struct {
//     struct spinlock lock;
//     struct proc proc[NPROC];
// } ptable;

