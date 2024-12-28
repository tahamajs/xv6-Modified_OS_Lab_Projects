#pragma once
#include "param.h"  // Include param.h for constants like NCPU, NOFILE, etc.
#include "mmu.h"    // Include mmu.h for types like pde_t and taskstate
#include "spinlock.h"
#include "defs.h"
// #include "syscall.h


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
    int time_slice;            // Time slice for the current queue
    int current_queue;         // Current queue being scheduled
    int queue_weights[3];      // Weights for the queues
    
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

#define MAX_AGE 800 


// schedule queue
enum schedqueue {
    UNSET,
    ROUND_ROBIN,
    SJF,
    FCFS
};

// // sjf parameters
// struct sjfparams {
//     enum schedpriority priority;
//     float executed_cycle;
//     int arrival_time;
//     int process_size;

//     float priority_ratio;
//     float executed_cycle_ratio;
//     float arrival_time_ratio;
//     float process_size_ratio;
//     int burst_time;
//     int confidence_level;
// };

// schedule info
struct schedparams {
    enum schedqueue queue;
    struct sjfparams sjf;
    int last_exec;
    int last_termination;
    int executed_cycle; // Add this line
};

// Per-process state
struct proc {
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
    int consecutive_run; // Number of consecutive runs
    
};

