#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"
#include "mp.h"
// int syscalls[MAX_SYSCALLS] = {0};  // Initialize with default values if needed
// struct nsyslock nsys;
struct nsyslock nsys;


// Fetch the int at addr from the current process.
int fetchint(uint addr, int* ip) {
    struct proc* curproc = myproc();

    if (addr >= curproc->sz || addr + 4 > curproc->sz)
        return -1;
    *ip = *(int*)(addr);
    return 0;
}

// Fetch the float at addr from the current process.
int fetchfloat(uint addr, float* fp) {
    struct proc* curproc = myproc();

    if (addr >= curproc->sz || addr + 4 > curproc->sz)
        return -1;
    *fp = *(float*)(addr);
    return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not including nul.
int fetchstr(uint addr, char** pp) {
    char *s, *ep;
    struct proc* curproc = myproc();

    if (addr >= curproc->sz)
        return -1;
    *pp = (char*)addr;
    ep = (char*)curproc->sz;
    for (s = *pp; s < ep; s++) {
        if (*s == 0)
            return s - *pp;
    }
    return -1;
}

// Fetch the nth 32-bit system call argument.
int argint(int n, int* ip) {
    return fetchint((myproc()->tf->esp) + 4 + 4 * n, ip);
}

// Fetch the nth 32-bit system call argument in float.
int argfloat(int n, float* fp) {
    return fetchfloat((myproc()->tf->esp) + 4 + 4 * n, fp);
}
// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size bytes.  Check that the pointer
// lies within the process address space.
int argptr(int n, char** pp, int size) {
    int i;
    struct proc* curproc = myproc();

    if (argint(n, &i) < 0)
        return -1;
    if (size < 0 || (uint)i >= curproc->sz || (uint)i + size > curproc->sz)
        return -1;
    *pp = (char*)i;
    return 0;
}

// syscall.c

// Existing includes and definitions...

extern int sys_create_palindrome(void);
extern int sys_move_file(void);
extern int sys_sort_syscalls(void);
extern int sys_get_most_invoked_syscall(void);
extern int sys_list_all_processes(void);
extern int sys_set_scheduling_queue(void);
extern int sys_print_processes_info(void);
extern int sys_chqueue(void);
extern int sys_changequeue(void);
extern int sys_set_sjf_proc(void);
extern int sys_set_sjf_sys(void);
extern int sys_user_program(void);
// extern int sys_set_SJF_params(void);





// Fetch the nul-terminated string at addr from the current process.
// Doesn't actually copy the string - just sets *pp to point at it.
// Returns length of string, not i
// Fetch the nth word-sized system call argument as a pointer
// to a block of memory of size bytes.  Check that the pointer
// lies within the process address space.


// Fetch the nth word-sized system call argument as a string pointer.
// Check that the pointer is valid and the string is nul-terminated.
// (There is no shared writable memory, so the string can't change
// between this check and being used by the kernel.)
int argstr(int n, char** pp) {
    int addr;
    if (argint(n, &addr) < 0)
        return -1;
    return fetchstr(addr, pp);
}


extern int sys_chdir(void);
extern int sys_close(void);
extern int sys_dup(void);
extern int sys_exec(void);
extern int sys_exit(void);
extern int sys_fork(void);
extern int sys_fstat(void);
extern int sys_getpid(void);
extern int sys_kill(void);
extern int sys_link(void);
extern int sys_mkdir(void);
extern int sys_mknod(void);
extern int sys_open(void);
extern int sys_pipe(void);
extern int sys_read(void);
extern int sys_sbrk(void);
extern int sys_sleep(void);
extern int sys_unlink(void);
extern int sys_wait(void);
extern int sys_write(void);
extern int sys_uptime(void);
extern int sys_set_scheduling_queue(void);
extern int sys_print_processes_info(void);
extern int sys_change_queue(void);


static int (*syscalls[])(void) = {
    [SYS_fork]                    sys_fork,
    [SYS_exit]                    sys_exit,
    [SYS_wait]                    sys_wait,
    [SYS_pipe]                    sys_pipe,
    [SYS_read]                    sys_read,
    [SYS_kill]                    sys_kill,
    [SYS_exec]                    sys_exec,
    [SYS_fstat]                   sys_fstat,
    [SYS_chdir]                   sys_chdir,
    [SYS_dup]                     sys_dup,
    [SYS_getpid]                  sys_getpid,
    [SYS_sbrk]                    sys_sbrk,
    [SYS_sleep]                   sys_sleep,
    [SYS_uptime]                  sys_uptime,
    [SYS_open]                    sys_open,
    [SYS_write]                   sys_write,
    [SYS_mknod]                   sys_mknod,
    [SYS_unlink]                  sys_unlink,
    [SYS_link]                    sys_link,
    [SYS_mkdir]                   sys_mkdir,
    [SYS_close]                   sys_close,
    [SYS_create_palindrome]       sys_create_palindrome,
    [SYS_move_file]               sys_move_file,
    [SYS_sort_syscalls]           sys_sort_syscalls,
    [SYS_get_most_invoked_syscall] sys_get_most_invoked_syscall,
    [SYS_list_all_processes]      sys_list_all_processes,
    [SYS_set_scheduling_queue]    sys_set_scheduling_queue,
    [SYS_print_processes_info]    sys_print_processes_info,
    [SYS_chqueue]                 sys_chqueue,
    [SYS_set_sjf_proc]            sys_set_sjf_proc,
    [SYS_set_sjf_sys]             sys_set_sjf_sys,
    [SYS_change_queue]            sys_change_queue,
    // [SYS_set_SJF_params]          sys_set_SJF_params,
};


int syscall_counts[MAX_SYSCALLS] = {0}; 
char* syscall_names[MAX_SYSCALLS] = {
    [SYS_fork] "fork",
    [SYS_exit] "exit",
    // ...add names for all syscalls
};

void syscall(void) {
    int num;
    struct proc* curproc = myproc();

    num = curproc->tf->eax;
    if (num > 0 && num < NELEM(syscalls) && syscalls[num]) {
        syscall_counts[num]++;
        curproc->tf->eax = syscalls[num]();
    } else {
        cprintf("%d %s: unknown sys call %d\n", 
                curproc->pid, curproc->name, num);
        curproc->tf->eax = -1;
    }
    cli();
    int CPUid = cpuid();
    sti();
    cpus[CPUid].nsyscall++;
    acquire(&nsys.lk);
    nsys.n++;
    release(&nsys.lk);
}
