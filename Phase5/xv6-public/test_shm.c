#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
    void *addr = open_sharedmem(1); 
    if((int)addr < 0){
        printf(1, "Failed to open shared memory\n");
        exit();
    }

    // Write a simple message into the shared region
    char *msg = (char*)addr;
    strcpy(msg, "Hello xv6 shared memory!");

    int pid = fork();
    if(pid < 0){
        printf(1, "Fork failed\n");
        close_sharedmem(addr);
        exit();
    }
    else if(pid == 0){
        // Child reads from shared memory
        printf(1, "Child reads: %s\n", msg);
        close_sharedmem(addr);
        exit();
    }
    else{
        wait();
        // Parent closes shared memory
        close_sharedmem(addr);
    }

    exit();
}
