#include "types.h"
#include "stat.h"
#include "user.h"

char buffer[512];


#define O_CREATE 0x200  // Example value
#define O_WRONLY 0x1  // Example value
// #include "types.h"
// #include "stat.h"
// #include "user.h"
#include <stdarg.h> // Include this for va_list, va_start, va_end

// #include "types.h"
// #include "stat.h"
// #include "user.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include <stdarg.h>

// Simple `vsprintf` implementation using xv6's `printf`
void vsprintf(char *buffer, const char *fmt, va_list args) {
    char *p = buffer;
    const char *s = fmt;

    while (*s) {
        if (*s != '%') {
            *p++ = *s++;
            continue;
        }
        s++; // Skip '%'
        if (*s == 'd') {
            int val = va_arg(args, int);
            char temp[20];   // Temporary buffer for integer
            itoa(val, temp); // Convert integer to string
            strcpy(p, temp);
            p += strlen(temp);
        } else if (*s == 's') {
            char *str = va_arg(args, char *);
            strcpy(p, str);
            p += strlen(str);
        }
        s++;
    }
    *p = '\0'; // Null-terminate the string
}

// // Wrapper for `vsprintf`
// void format_string(char *buffer, const char *fmt, ...) {
//     va_list args;
//     va_start(args, fmt);
//     vsprintf(buffer, fmt, args);
//     va_end(args);
// }

// `itoa` implementation (if not available)
void itoa(int n, char *s) {
    int i, sign;
    if ((sign = n) < 0) // Record sign
        n = -n;         // Make n positive
    i = 0;
    do { // Generate digits in reverse order
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s); // Reverse the string
}

// Reverse a string (if not available)
void reverse(char *s) {
    int i, j;
    char c;
    for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

// Formats a string into a buffer
void format_string(char *buffer, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vsprintf(buffer, fmt, args);
    va_end(args);
}

// Writes a buffer to a file descriptor
void write_to_file(int fd, const char *buffer) {
    write(fd, buffer, strlen(buffer));
}

// Simulates a CPU-bound task
void cpu_bound_task(int duration) {
    for (int i = 0; i < duration * 1000000; i++);
}

// Simulates aging test
void test_aging(int log_fd) {
    write_to_file(log_fd, "Aging test started\n");
    // Add logic for aging test
    write_to_file(log_fd, "Aging test completed\n");
}


void create_processes(int num_processes) {
    for (int i = 0; i < num_processes; i++) {
        int pid = fork();
        if (pid < 0) {
            printf(1, "Fork failed\n");
            exit();
        } else if (pid == 0) {

            for (int j = 0; j < 100; j++) {
                printf(1, "Process %d running\n with pid %d\n", i, getpid());   
                sleep(10); // Use sleep from user.h
            }
            exit();
        }
    }
}

void test_scheduling_queues(int log_fd) {

    
    format_string(buffer, "\n=== Testing Different Scheduling Queues ===\n");
    write_to_file(log_fd, buffer);
    
    int pids[3];
    
    // Round Robin Process
    if ((pids[0] = fork()) == 0) {
        change_queue(getpid(), 0);
        format_string(buffer, "RR Process (PID: %d) started\n", getpid());
        write_to_file(log_fd, buffer);
        
        for (int i = 0; i < 5; i++) {
            format_string(buffer, "RR Process (PID: %d) iteration %d\n", getpid(), i);
            write_to_file(log_fd, buffer);
            cpu_bound_task(2);
            sleep(50);
        }
        exit();
    }
    
    // SJF Process
    if ((pids[1] = fork()) == 0) {
        change_queue(getpid(), 1);
        // set_sjf_proc(getpid(),50 ,2);
        format_string(buffer, "SJF Process (PID: %d) started\n", getpid());
        write_to_file(log_fd, buffer);
        
        for (int i = 0; i < 5; i++) {
            format_string(buffer, "SJF Process (PID: %d) iteration %d\n", getpid(), i);
            write_to_file(log_fd, buffer);
            cpu_bound_task(3);
            sleep(50);
        }
        exit();
    }
    
    // FCFS Process with detailed logging
    if ((pids[2] = fork()) == 0) {
        change_queue(getpid(), 2);
        format_string(buffer, "FCFS Process (PID: %d) started\n", getpid());
        write_to_file(log_fd, buffer);
        
        for (int i = 0; i < 5; i++) {
            format_string(buffer, "FCFS Process (PID: %d) iteration %d\n", getpid(), i);
            write_to_file(log_fd, buffer);
            cpu_bound_task(1);
            sleep(50);
        }
        exit();
    }
    
    // Parent monitoring with file logging
    format_string(buffer, "Parent process monitoring started\n");
    write_to_file(log_fd, buffer);
    
    for (int i = 0; i < 10; i++) {
        format_string(buffer, "\n--- System Status at iteration %d ---\n", i);
        write_to_file(log_fd, buffer);
        print_processes_info();
        sleep(100);
    }
    
    // Wait for all children
    for (int i = 0; i < 3; i++) {
        wait();
    }
}

void test_queue_changes(int log_fd) {
    format_string(buffer, "\n=== Testing Queue Changes ===\n");
    write_to_file(log_fd, buffer);
    
    int pid = fork();
    
    if (pid == 0) {
        // Log queue changes
        change_queue(getpid(), 2);
        format_string(buffer, "Process (PID: %d) starting in FCFS queue\n", getpid());
        write_to_file(log_fd, buffer);
        print_processes_info();
        
        sleep(100);
        
        change_queue(getpid(), 1);
        format_string(buffer, "Process (PID: %d) changed to SJF queue\n", getpid());
        write_to_file(log_fd, buffer);
        print_processes_info();
        
        sleep(100);
        
        change_queue(getpid(), 0);
        format_string(buffer, "Process (PID: %d) changed to Round Robin queue\n", getpid());
        write_to_file(log_fd, buffer);
        print_processes_info();
        
        cpu_bound_task(2);
        exit();
    } else {
        wait();
    }
}

int main(void) {
    // Create log file with timestamp
    char filename[32];
    format_string(filename, "scheduler_test_%d.log", uptime());
    int log_fd = open(filename, O_CREATE | O_WRONLY);
    
    
    if (log_fd < 0) {
        printf(1, "Failed to create log file\n");
        exit();
    }
    
    printf(1, "Starting Comprehensive Scheduler Testing...\n");
    printf(1, "Results will be saved to: %s\n", filename);
    
    write_to_file(log_fd, "=== Scheduler Test Results ===\n");
    write_to_file(log_fd, "Starting tests...\n\n");
    
    // Run tests with file logging
    test_queue_changes(log_fd);
    test_scheduling_queues(log_fd);
    test_aging(log_fd);
    
    // Write completion message
    write_to_file(log_fd, "\nAll tests completed.\n");
    
    // Close log file
    close(log_fd);
    printf(1, "\nTests completed. Results saved to %s\n", filename);
    exit();
}
