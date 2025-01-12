#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"


// Add function declarations
int set_sjf_proc(int pid, int priority_ratio, int burst_time);
int set_sjf_sys(int priority_ratio, int burst_time);
int print_processes_infos(void);
void sort_syscalls(void);
int get_most_invoked(void);
extern int list_proceases(void);
extern struct reentrantlock testlock;

// System call to set a process's scheduling queue
int sys_set_scheduling_queue(void)
{
    int pid, queue;

    if(argint(0, &pid) < 0 || argint(1, &queue) < 0)
        return -1;

    return change_queue(pid, queue);
}

int sys_set_sjf_proc(void) {
    int pid;
    int a, b;
    if (argint(0, &pid) < 0 || argint(1, &a) < 0 || argint(2, &b) < 0)
        return -1;
    return set_sjf_proc(pid, a, b);
}

int sys_set_sjf_sys(void) {
    int a, b;
    if (argint(0, &a) < 0 || argint(1, &b) < 0 )
        return -1;
    return set_sjf_sys(a, b);
}

// Correct function names
int sys_print_processes_info(void) {
    print_processes_infos();
    return 0;
}

int sys_fork(void) {
  return fork();
}

int sys_exit(void) {
  exit();
  return 0;  // not reached
}

int sys_wait(void) {
  return wait();
}

int sys_kill(void) {
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int sys_getpid(void) {
  return myproc()->pid;
}

int sys_sbrk(void) {
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int sys_sleep(void) {
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int sys_uptime(void) {
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int sys_create_palindrome(void) {
    int num;
    if (argint(0, &num) < 0)
        return -1;
    
    int orig = num;
    int rev = 0;
    
    while(num > 0) {
        rev = rev * 10 + num % 10;
        num /= 10;
    }
    
    cprintf("%d%d\n", orig, rev);
    return 0;
}

int sys_sort_syscalls(void) {
  sort_syscalls();
  return 0;
}

int sys_get_most_invoked_syscall(void) {
  return get_most_invoked();
}

// Correct function names
int sys_list_all_processes(void) {
    list_proceases();
    return 0;
}

int sys_change_queue(void) {
  int pid, new_queue_level;
  if(argint(0, &pid) < 0)
    return -1;
  if(argint(1, &new_queue_level) < 0)
    return -1;
  return change_queue(pid, new_queue_level);
}

// Alias for change_queue
int sys_chqueue(void) {
    return sys_change_queue();
}

// static struct reentrantlock testlock;

int sys_reacquire(void) {
    return acquirereentrantlock(&testlock);
}

int sys_rerelease(void) { 
  return releasereentrantlock(&testlock); 
}

// int sys_nsyscalls(void) {
//     return sys_scinfo(); // Use existing scinfo functionality to return count
// }

// int sys_get_all_cpus_syscalls(void) {
//     int total = 0;
//     pushcli();
//     for(int i = 0; i < ncpu; i++) {
//         total += cpus[i].nsyscall;
//     }
//     popcli();
//     return total;
// }

int sys_test_syscall_count(void) {
    return myproc()->syscall_count;
}

char* sys_open_sharedmem(void) {
    int id;
    if (argint(0, &id) < 0)
        return -1; 

    return open_sharedmem(id);
}

int sys_close_sharedmem(void) {
    int id;
    if (argint(0, &id) < 0)
        return -1;

    return close_sharedmem(id);
}