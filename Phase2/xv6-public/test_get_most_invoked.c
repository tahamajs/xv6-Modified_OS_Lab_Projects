// test_get_most_invoked.c

#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
    int pid = getpid();

    // Make some system calls
    printf(1, "Testing get_most_invoked_syscall\n");
    for (int i = 0; i < 5; i++)
        getpid();
    for (int i = 0; i < 3; i++)
        sleep(1);

    // Get the most invoked system call
    if (get_most_invoked_syscall(pid) < 0) {
        printf(2, "Failed to get most invoked system call\n");
        exit();
    }

    exit();
}
