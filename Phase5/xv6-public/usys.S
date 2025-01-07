#include "syscall.h"
#include "traps.h"

#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret

SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)
SYSCALL(change_queue)
SYSCALL(print_processes_info)
SYSCALL(create_palindrome)
SYSCALL(move_file)
SYSCALL(sort_syscalls)
SYSCALL(get_most_invoked_syscall)
SYSCALL(list_all_processes)
SYSCALL(set_scheduling_queue)
SYSCALL(printprocs)
SYSCALL(set_sjf_proc)
SYSCALL(set_sjf_sys)
SYSCALL(chqueue)
SYSCALL(user_program)
SYSCALL(set_SJF_params)
SYSCALL(nsyscalls)
SYSCALL(reacquire)
SYSCALL(rerelease)
SYSCALL(get_all_cpus_syscalls)
SYSCALL(open_sharedmem)
SYSCALL(close_sharedmem)
