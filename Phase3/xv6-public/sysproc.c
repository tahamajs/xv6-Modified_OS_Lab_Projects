#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"


// System call to set a process's scheduling queue
int sys_set_scheduling_queue(void)
{
    int pid, queue;

    if(argint(0, &pid) < 0 || argint(1, &queue) < 0)
        return -1;

    return change_queue(pid, queue);
}

int sys_chqueue(void) {
    int pid, queue;
    if (argint(0, &pid) < 0 || argint(1, &queue) < 0)
        return -1;

    return change_queue(pid, queue);
}
int sys_bjsproc(void) {
    int pid;
    float priority_ratio, arrival_time_ratio, executed_cycle_ratio, process_size_ratio;
    if (argint(0, &pid) < 0 ||
        argfloat(1, &priority_ratio) < 0 ||
        argfloat(2, &arrival_time_ratio) < 0 ||
        argfloat(3, &executed_cycle_ratio) < 0 ||
        argfloat(4, &process_size_ratio) < 0) return -1;

    return set_bjs_proc(pid, priority_ratio, arrival_time_ratio, executed_cycle_ratio, process_size_ratio);
}

int sys_bjssys(void) {
    float priority_ratio, arrival_time_ratio, executed_cycle_ratio, process_size_ratio;
    if (argfloat(0, &priority_ratio) < 0 ||
        argfloat(1, &arrival_time_ratio) < 0 ||
        argfloat(2, &executed_cycle_ratio) < 0 ||
        argfloat(3, &process_size_ratio) < 0) return -1;

    return set_bjs_sys(priority_ratio, arrival_time_ratio, executed_cycle_ratio, process_size_ratio);
}


int sys_procinfo(void) {
    return print_processes_infos();
}


int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
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
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_create_palindrome(void)
{
    int num;
    if (argint(0, &num) < 0)
        return -1;
    if(num == 0){
      cprintf("Palindrome: %d\n",num);
      return 0;
    }
    int reversed = 0, temp = num;
    while (temp != 0) {
        reversed = reversed * 10 + temp % 10;
        temp /= 10;
    }
    cprintf("Palindrome: %d%d\n", num,reversed);

    return 0;
}


int
sys_sort_syscalls(void)
{
  sort_syscalls();
}


int
sys_get_most_invoked_syscall(void)
{
  return get_most_invoked();

}

int
sys_list_all_processes(void)
{
    list_proceases();
}
