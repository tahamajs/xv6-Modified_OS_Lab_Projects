#include "types.h"
#include "stat.h"
#include "user.h"

void create_processes(int num_processes) {
    for (int i = 0; i < num_processes; i++) {
        int pid = fork();
        if (pid < 0) {
            printf(1, "Fork failed\n");
            exit();
        } else if (pid == 0) {
            // Child processp
            for (int j = 0; j < 100; j++) {
                printf(1, "Process %d running\n with pid %d\n", i, getpid());   
                sleep(10); // Use sleep from user.h
            }
            exit();
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf(1, "Usage: user_program <num_processes>\n");
        exit();
    }

    int num_processes = atoi(argv[1]);
    if (num_processes <= 0) {
        printf(1, "Number of processes must be greater than 0\n");
        exit();
    }

    printf(1, "Creating %d processes...\n", num_processes);
    create_processes(num_processes);

    // Wait for all child processes to finish
    for (int i = 0; i < num_processes; i++) {
        wait();
    }

    // Print all processes information
    printf(1, "Printing all processes information:\n");
    print_processes_info();

    exit();
}
