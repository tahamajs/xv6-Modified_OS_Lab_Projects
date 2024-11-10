// test_sort_syscalls.c

#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf(2, "Usage: test_sort_syscalls <pid>\n");
        exit();
    }

    int pid = atoi(argv[1]);
    if(pid == 0 && argv[1][0] != '0'){
        printf(2, "Usage: test_sort_syscalls <pid>\n");
        exit();
    }
    if(pid == getpid()){
        printf(1, "Testing sort_syscalls\n");
        getpid();
        sleep(10);
        write(1, "Hello\n", 6);
    }
    // Sort the system calls
    if (sort_syscalls(pid) < 0) {
        printf(2, "Requested pid was not found\n");
        exit();
    }
    exit();
}
