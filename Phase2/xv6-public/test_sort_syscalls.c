// test_sort_syscalls.c

#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
    int pid = getpid();

    // Make some system calls
    printf(1, "Testing sort_syscalls\n");
    getpid();
    sleep(10);
    write(1, "Hello\n", 6);

    // Sort the system calls
    if (sort_syscalls(pid) < 0) {
        printf(2, "Failed to sort system calls\n");
        exit();
    }

    exit();
}
