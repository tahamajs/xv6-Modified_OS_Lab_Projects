#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define NUM_PROCESSES 4
#define NUM_ITERATIONS 100
#define LOGFILE "syscall_test.log"

// Helper function to write formatted string to file
void write_log(int fd, const char *format, ...) {
    char buf[256];
    int i = 0;
    
    while(*format) {
        if(*format != '%') {
            buf[i++] = *format++;
            continue;
        }
        format++; // Skip '%'
        if(*format == 'd') {
            int val = *(int*)(&format + 1);
            char num[10];
            int j = 0;
            if(val == 0) num[j++] = '0';
            else {
                while(val > 0) {
                    num[j++] = '0' + (val % 10);
                    val /= 10;
                }
            }
            while(--j >= 0) buf[i++] = num[j];
        }
        format++;
    }
    buf[i] = '\0';
    write(fd, buf, strlen(buf));
}

void child_process(int id) {
    char filename[32];
    strcpy(filename, "test_file_");
    filename[10] = '0' + id;
    strcpy(filename + 11, ".txt");
    
    // Open a file for writing
    int fd = open(filename, O_CREATE | O_WRONLY);
    if(fd < 0) {
        printf(1, "Process %d: Failed to open file\n", id);
        exit();
    }

    // Perform multiple system calls
    for(int i = 0; i < NUM_ITERATIONS; i++) {
        char msg[64];
        printf(1, "Process %d: Iteration %d\n", id, i);
        
        // Mix in other system calls
        getpid();
        sleep(1);
        uptime();
    }

    close(fd);
    exit();
}

int main(void) {
    int initial_count = nsyscalls();
    printf(1, "Initial syscall count: %d\n", initial_count);

    // Create multiple processes
    int pids[NUM_PROCESSES];
    for(int i = 0; i < NUM_PROCESSES; i++) {
        pids[i] = fork();
        if(pids[i] < 0) {
            printf(1, "Fork failed!\n");
            exit();
        }
        if(pids[i] == 0) {
            child_process(i);
        }
    }

    // Wait for all children to complete
    for(int i = 0; i < NUM_PROCESSES; i++) {
        wait();
    }

    // Get final count and verify
    int final_count = nsyscalls();
    printf(1, "Final syscall count: %d\n", final_count);
    printf(1, "Total syscalls made: %d\n", final_count - initial_count);

    // Open log file to record results
    int log_fd = open(LOGFILE, O_CREATE | O_WRONLY);
    if(log_fd >= 0) {
        write_log(log_fd, "Initial count: %d\n", initial_count);
        write_log(log_fd, "Final count: %d\n", final_count);
        write_log(log_fd, "Total syscalls: %d\n", final_count - initial_count);
        close(log_fd);
    }

    exit();
}
