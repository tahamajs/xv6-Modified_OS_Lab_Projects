// test_sched.c

#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
    int pid1, pid2, pid3;

    // Create first child process assigned to BJF queue
    pid1 = fork();
    if(pid1 == 0){
        set_scheduling_queue(getpid(), 0); // BJF
        for(int i = 0; i < 10; i++){
            printf(1, "BJF Process %d running iteration %d\n", getpid(), i);
            sleep(100);
        }
        exit();
    }

    // Create second child process assigned to LCFS queue
    pid2 = fork();
    if(pid2 == 0){
        set_scheduling_queue(getpid(), 1); // LCFS
        for(int i = 0; i < 10; i++){
            printf(1, "LCFS Process %d running iteration %d\n", getpid(), i);
            sleep(100);
        }
        exit();
    }

    // Create third child process assigned to Round Robin queue
    pid3 = fork();
    if(pid3 == 0){
        set_scheduling_queue(getpid(), 2); // Round Robin
        for(int i = 0; i < 10; i++){
            printf(1, "Round Robin Process %d running iteration %d\n", getpid(), i);
            sleep(100);
        }
        exit();
    }

    // Parent process waits for all children to finish
    wait();
    wait();
    wait();

    // Print process information
    print_processes_info();

    exit();
}
