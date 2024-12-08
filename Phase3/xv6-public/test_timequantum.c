#include "types.h"
#include "stat.h"
#include "user.h"

#define NUM_PROCESSES 5
#define NUM_ITERATIONS 1000000

void long_running_task(int id) {
    int i;
    volatile int sum = 0;
    for (i = 0; i < NUM_ITERATIONS; i++) {
        sum += i;
        if (i % (NUM_ITERATIONS / 8) == 0) {
            printf(1, "    %d\n", id + 1);
        }
    }
    // printf(1, "Process %d: Finished\n", id);
}

int main(void) {
    int pid, i;

    for (i = 0; i < NUM_PROCESSES; i++) {
        pid = fork();
        if (pid < 0) {
            printf(1, "Fork failed\n");
            exit();
        }
        if (pid == 0) {
            // Child process
            long_running_task(i);
            exit();
        }
        // Parent process continues to create more child processes
    }

    // Wait for all child processes to finish
    for (i = 0; i < NUM_PROCESSES; i++) {
        wait();
    }

    printf(1, "All processes finished\n");
    exit();
}
