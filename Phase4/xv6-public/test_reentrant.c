#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
    printf(1, "----- Reentrant Lock Test Start -----\n\n");

    int ret = reacquire();
    printf(1, "First acquire result: %d\n", ret);
    if(ret < 0) exit();

    ret = reacquire();
    printf(1, "Second acquire result: %d\n", ret);
    if(ret < 0) exit();

    ret = rerelease();
    printf(1, "First release result: %d\n", ret);
    if(ret < 0) exit();

    ret = rerelease();
    printf(1, "Second release result: %d\n", ret);
    if(ret < 0) exit();

    ret = rerelease();
    printf(1, "Third release result (should fail): %d\n", ret);
    
    printf(1, "\n----- Reentrant Lock Test Complete -----\n");

    int pid = fork();
    if(pid < 0) {
        printf(1, "Fork failed\n");
        exit();
    }

    if(pid == 0) {
        ret = reacquire();
        printf(1, "Child acquire result: %d\n", ret);
        rerelease();
        exit();
    } else {
        ret = reacquire();
        printf(1, "Parent acquire result: %d\n", ret);
        rerelease();
        wait();
    }

    printf(1, "All tests passed successfully\n");
    exit();
}
