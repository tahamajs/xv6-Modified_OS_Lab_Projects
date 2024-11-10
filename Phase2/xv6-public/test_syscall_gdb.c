// test_syscall_gdb.c

#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
    // Make some system calls
    printf(1, "Testing getpid system call\n");
    int pid = getpid();
    printf(1,"Process ID: %d\n",pid);
    exit();
}
