#include "types.h"
#include "stat.h"
#include "user.h"

#define ITERATION_NUM 5

void recursiveFunc(int count){
    if(count == ITERATION_NUM){
        printf(1,"Last Recursive Call. Releasing Lock...\n");
        for(int i = 0; i < ITERATION_NUM; i++){
            int ret = rerelease();
            printf(1,"Call %d release result: %d\n", i + 1, ret);
            if(ret < 0) exit();
        }
        return;
    }
    int ret = reacquire();
    printf(1,"Call %d acquire result: %d\n", count + 1, ret);
    recursiveFunc(count + 1);
}

int main(void) {

    printf(1, "----- Reentrant Lock Test Start -----\n");

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

    printf(1, "----- Reentrant Lock Test Complete -----\n");

    printf(1,"\n----- Recursive Function Test -----\n");

    recursiveFunc(0);
    ret = rerelease();
    printf(1,"Additional release result (should fail): %d\n", ret);

    printf(1,"----- Recursive Function Test Complete -----\n");

    printf(1,"\n----- Fork Test -----\n");

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
    printf(1,"----- Fork Test Complete -----\n\n");


    printf(1, "All tests passed successfully\n");
    exit();
}
