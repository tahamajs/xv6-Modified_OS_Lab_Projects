#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

#define NUM_PROCESSES 4
#define NUM_ITERATIONS 100
#define CPUS 4
#define LOGFILE "./syscall_test.log"


// Helper function to write formatted string to file
void write_log(int fd, const char *format, int value) {
    char buf[256];
    int i = 0;
    
    while (*format) {
        if (*format != '%') {
            buf[i++] = *format++;
            continue;
        }
        format++; // Skip '%'
        if (*format == 'd') {
            char num[10];
            int j = 0;
            if (value == 0) {
                num[j++] = '0';
            } else {
                int temp_val = value;
                while (temp_val > 0) {
                    num[j++] = '0' + (temp_val % 10);
                    temp_val /= 10;
                }
            }
            while (--j >= 0) {
                buf[i++] = num[j];
            }
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
        
        // Mix in other system calls
        getpid();
        sleep(1);
        uptime();
    }
    printf(1, "Process %d: completed\n", id);

    close(fd);
    exit();
}

int main(void) {
    int open_init = nsyscalls();
    int fd = open("test.txt", O_CREATE | O_WRONLY);
    int open_final = nsyscalls() - 1;
    printf(1,"Open syscall count: %d\n", open_final - open_init);
    int write_init = nsyscalls();
    write(fd,"HI",2);
    int write_final = nsyscalls() - 1;
    printf(1,"write syscall count: %d\n", write_final - write_init);
    int close_init = nsyscalls();
    close(fd);
    int close_final_count = nsyscalls() - 1;
    printf(1,"close syscall count: %d\n", close_final_count - close_init);

    //Printing each character is a write system call which increments the count by 2
    int init_syscall = nsyscalls();
    printf(1,"Test...\n");
    int final_syscall = nsyscalls() - 1;//decrementing by 1 so the nsyscall() wouldn't count
    printf(1,"System calls made to print 'Test...\\n' to console: %d\n",final_syscall - init_syscall);


    int initial_count = nsyscalls();
    int initial_cpu_count = get_all_cpus_syscalls();
    printf(1, "Initial global syscall count: %d\n", initial_count);
    printf(1, "Initial CPUs syscall count: %d\n", initial_cpu_count);
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
    int final_count = nsyscalls() - 1;
    int final_cpu_count = get_all_cpus_syscalls() - 1;
    printf(1, "Final global syscall count: %d\n", final_count);
    printf(1, "Final CPUs syscall count: %d\n", final_cpu_count);
    printf(1, "Total syscalls made (global): %d\n", final_count - initial_count);
    printf(1, "Total syscalls made by CPUs: %d\n",final_cpu_count - initial_cpu_count);
    printf(1, "Total syscalls made (per-CPU): %d\n", (final_cpu_count - initial_cpu_count) / CPUS);

    // Open log file to record results
    int log_fd = open(LOGFILE, O_CREATE | O_WRONLY);
    if(log_fd >= 0) {
        write_log(log_fd, "Initial global count: %d\n", initial_count);
        write_log(log_fd, "Initial CPU count: %d\n", initial_cpu_count);
        write_log(log_fd, "Final global count: %d\n", final_count);
        write_log(log_fd, "Final CPU count: %d\n", final_cpu_count);
        write_log(log_fd, "Total global syscalls: %d\n", final_count - initial_count);
        write_log(log_fd, "Total CPU syscalls: %d\n", final_cpu_count - initial_cpu_count);
        close(log_fd);
    }

    exit();
}
