#pragma once

// Debug macros
#define PROC_DEBUG 1
#ifdef PROC_DEBUG
#define PDEBUG(fmt, ...) cprintf(fmt, ##__VA_ARGS__)
#else
#define PDEBUG(fmt, ...)
#endif

// BJF validation macros
#define MIN_BURST_TIME 0
#define MAX_BURST_TIME 1000
#define MIN_CONFIDENCE 0
#define MAX_CONFIDENCE 10

// Scheduling constants
#define NUM_QUEUES 3
#define AGING_THRESHOLD 1000
#define MAX_BURST_TIME 1000

struct bjfinfo {
    int estimated_burst_time;    // Estimated execution time
    int confidence_level;        // Confidence in the estimation
    int actual_burst_time;       // Actual execution time (for future use)
    int last_cpu_usage;         // Track actual CPU usage
    int prediction_accuracy;     // Track prediction accuracy
    int priority_ratio;         // Added for BJF calculation
};

enum schedqueue {
    BJF,            // Highest priority
    LCFS,          // Medium priority
    ROUND_ROBIN,   // Lowest priority
    FCFS           // Default
};

#include "spinlock.h"  // Adjust the path if necessary to locate spinlock definition

// Per-CPU state
struct cpu {
  uchar apicid;                // Local APIC ID
  struct context *scheduler;   // swtch() here to enter scheduler
  struct taskstate ts;         // Used by x86 to find stack for interrupt
  struct segdesc gdt[NSEGS];   // x86 global descriptor table
  volatile uint started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?
  struct proc *proc;           // The process running on this cpu or null
  struct proc *last_proc;      // Last scheduled process
  struct context context;      // Context for scheduler switches 
};

#define MAX_SYSCALLS 100
extern int syscall_counts[MAX_SYSCALLS];

extern struct cpu cpus[NCPU];
extern int ncpu;

// Saved registers for kernel context switches.
struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;
  uint eip;
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

// Process statistics
struct proc_stats {
    uint total_run_time;        // Total time process has run
    uint wait_time;             // Time spent waiting
    uint context_switches;      // Number of context switches
    uint queue_transitions;     // Number of queue changes
};

// Per-process state
struct proc {
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
    struct file *ofile[NOFILE];  // Open files
    struct inode *cwd;           // Current directory
    char name[16];               // Process name (debugging)
    int syscalls[MAX_SYSCALLS];  // System call counts
    int syscall_count;           // Total number of system calls

    // Scheduling Fields
    enum schedqueue sched_info_queue;    // Current scheduling queue
    struct bjfinfo sched_info_bjf;       // BJF-specific information
    int sched_info_last_run;             // Last run tick for aging
    int sched_info_arrival_time;         // Arrival time in the queue

    // Statistics and Debugging
    struct proc_stats stats;
    int debug_flags;            // Process-specific debug flags
    char debug_msg[64];         // Debug message buffer

    // Validation Fields
    int state_valid;            // State validation flag
    int last_error;             // Last error code

    // Additional Scheduling Information
    uint run_time;
    uint wait_time;
    uint switches;
};

// Validation functions (declare as inline)
static inline int validate_bjf_params(struct bjfinfo *bjf) {
    return (bjf->estimated_burst_time >= MIN_BURST_TIME && 
            bjf->estimated_burst_time <= MAX_BURST_TIME &&
            bjf->confidence_level >= MIN_CONFIDENCE &&
            bjf->confidence_level <= MAX_CONFIDENCE);
}

static inline int validate_proc_state(struct proc *p) {
    return (p->state >= UNUSED && p->state <= ZOMBIE);
}

// Scheduling helper functions
static inline int calculate_bjf_priority(struct proc *p) {
    struct bjfinfo *bjf = &p->sched_info_bjf;
    return (bjf->estimated_burst_time * bjf->confidence_level) / 
           (p->wait_time + 1);  // Add 1 to avoid division by zero
}

// Process memory is laid out contiguously, low addresses first:
//   text
//   original data and bss
//   fixed-size stack
//   expandable heap
struct ptable {
  struct spinlock lock;
  struct proc proc[NPROC];
};

// Process table statistics
struct ptable_stats {
    int total_processes;
    int running_processes;
    int ready_processes;
    int blocked_processes;
};

extern struct ptable ptable;
extern struct ptable_stats ptable_stats;





