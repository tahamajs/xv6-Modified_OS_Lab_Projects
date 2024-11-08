#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

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

    // Reverse the number
    int reversed = 0, temp = num;
    while (temp != 0) {
        reversed = reversed * 10 + temp % 10;
        temp /= 10;
    }

    // Calculate the number of digits in reversed
    int digits = 0;
    temp = reversed;
    if (temp == 0) {
        digits = 1;
    } else {
        while (temp != 0) {
            temp /= 10;
            digits++;
        }
    }

    // Calculate multiplier
    int multiplier = 1;
    for (int i = 0; i < digits; i++)
        multiplier *= 10;

    // Create palindrome
    int palindrome = num * multiplier + reversed;

    cprintf("Palindrome: %d\n", palindrome);

    return 0;
}
// sysproc.c

int
sys_sort_syscalls(void)
{
    int pid;
    struct proc *p;

    if (argint(0, &pid) < 0)
        return -1;

    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->pid == pid) {
            // Sort the syscalls array using bubble sort
            int n = p->syscall_count;
            for (int i = 0; i < n - 1; i++) {
                for (int j = 0; j < n - i - 1; j++) {
                    if (p->syscalls[j] > p->syscalls[j + 1]) {
                        int temp = p->syscalls[j];
                        p->syscalls[j] = p->syscalls[j + 1];
                        p->syscalls[j + 1] = temp;
                    }
                }
            }
            release(&ptable.lock);
            return 0;
        }
    }
    release(&ptable.lock);
    return -1; // Process not found
}
// sysproc.c

int
sys_get_most_invoked_syscall(void)
{
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
            // Count occurrences
            int counts[NELEM(syscall_counts)] = {0};
            for (int i = 0; i < p->syscall_count; i++) {
                counts[p->syscalls[i]]++;
            }
            // Find the most invoked syscall
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



// sysproc.c

int
sys_list_all_processes(void)
{
    struct proc *p;

    acquire(&ptable.lock);
    cprintf("PID\tSyscall Count\n");
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state != UNUSED && p->state != ZOMBIE) {
            cprintf("%d\t%d\n", p->pid, p->syscall_count);
        }
    }
    release(&ptable.lock);

    return 0;
}
