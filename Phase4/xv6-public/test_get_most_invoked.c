#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf(2, "Usage: test_get_most_invoked_syscall <pid>\n");
        exit();
    }

    int pid = atoi(argv[1]);
    if(pid == 0 && argv[1][0] != '0'){
        printf(2, "Usage: test_get_most_invoked_syscall <pid>\n");
        exit();
    }

    // Make some system calls
    printf(1, "Testing get_most_invoked_syscall\n");

    // Get the most invoked system call
    if (get_most_invoked_syscall(pid) < 0) {
        printf(2, "Failed to get most invoked system call\n");
        exit();
    }

    exit();
}
