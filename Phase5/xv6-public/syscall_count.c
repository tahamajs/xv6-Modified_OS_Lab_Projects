#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
    // Make a few system calls before checking count
    getpid();
    sleep(1);
    uptime();
    
    // Call nsyscalls() to get and print count
    int count = nsyscalls();
    printf(1, "Number of system calls: %d\n", count);
    
    exit();
}
