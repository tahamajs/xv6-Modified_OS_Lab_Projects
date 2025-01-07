#include "types.h"
#include "stat.h"
#include "user.h"
// #include "defs.h"
#include "fcntl.h"
// #include "proc.h"

void format_string(char* buffer, const char* format, ...) {
    char* str = buffer;
    const char* fmt = format;
    int i;
    char num_buffer[16];
    
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            switch (*fmt) {
                case 'd':
                    i = *(int*)(&format + 1);
                    int n = i;
                    int idx = 0;
                    // Convert number to string
                    if (n == 0) {
                        num_buffer[idx++] = '0';
                    } else {
                        while (n > 0) {
                            num_buffer[idx++] = '0' + (n % 10);
                            n /= 10;
                        }
                    }
                    // Copy reversed
                    while (idx > 0) {
                        *str++ = num_buffer[--idx];
                    }
                    break;
                case 's':
                    char* s = *(char**)(&format + 1);
                    while (*s) {
                        *str++ = *s++;
                    }
                    break;
                default:
                    *str++ = *fmt;
            }
        } else {
            *str++ = *fmt;
        }
        fmt++;
    }
    *str = '\0';
}

// Function to create CPU load
void cpu_bound_task(int intensity) {
    int i, j, dummy = 0;
    for (i = 0; i < intensity * 100000; i++) {
        for (j = 0; j < 1000; j++) {
            dummy += (dummy + i) % (j + 1);
        }
    }
}

// Function to write to file
void write_to_file(int fd, char* str) {
    write(fd, str, strlen(str));
}

char buffer[1024];  // Buffer for formatting strings

// Test aging mechanism
void test_aging(int log_fd) {
    printf(1, "\n=== Testing Aging Mechanism ===\n");
    format_string(buffer, "\n=== Testing Aging Mechanism ===\n");
    write_to_file(log_fd, buffer);
    
    int pid = fork();
    
    if (pid < 0) {
        printf(1, "Fork failed\n");
        format_string(buffer, "Fork failed\n");
        exit();
    }

    if (pid == 0) {
        change_queue(getpid(), 2);
        format_string(buffer, "Child process (PID: %d) started in FCFS queue\n", getpid());
        
        for (int i = 0; i < 10; i++) {
            format_string(buffer, "Child process (PID: %d) iteration %d\n", getpid(), i);
            write_to_file(log_fd, buffer);
            cpu_bound_task(5);
            
            // Capture process info in file
            write_to_file(log_fd, "\n--- Process Status ---\n");
            print_processes_info();
            sleep(100);
        }
        exit();
    } else {
        wait();
    }
}

// Test different scheduling queues
void test_scheduling_queues(int log_fd) {
    format_string(buffer, "\n=== Testing Different Scheduling Queues ===\n");
    write_to_file(log_fd, buffer);
    
    int pids[12];  // Increase the array size to accommodate more processes
    
    // Round Robin Process
    if ((pids[0] = fork()) == 0) {
        change_queue(getpid(), 0);
        format_string(buffer, "RR Process (PID: %d) started\n", getpid());
        
        for (int i = 0; i < 5; i++) {
            format_string(buffer, "RR Process (PID: %d) iteration %d\n", getpid(), i);
            write_to_file(log_fd, buffer);
            cpu_bound_task(200);
        }
        exit();
    }
    
    // SJF Process
    if ((pids[1] = fork()) == 0) {
        change_queue(getpid(), 1);
        set_SJF_params(getpid(), 100, 80);
        format_string(buffer, "SJF Process (PID: %d) started\n", getpid());
        
        for (int i = 0; i < 5; i++) {
            format_string(buffer, "SJF Process (PID: %d) iteration %d\n", getpid(), i);
            write_to_file(log_fd, buffer);
            cpu_bound_task(3);
            sleep(50);
        }
        exit();
    }
    
    // FCFS Process
    if ((pids[2] = fork()) == 0) {
        change_queue(getpid(), 2);
        format_string(buffer, "FCFS Process (PID: %d) started\n", getpid());
        
        for (int i = 0; i < 5; i++) {
            format_string(buffer, "FCFS Process (PID: %d) iteration %d\n", getpid(), i);
            write_to_file(log_fd, buffer);
            cpu_bound_task(1);
            sleep(50);
        }
        exit();
    }
    
    // Additional Round Robin Process
    if ((pids[3] = fork()) == 0) {
        change_queue(getpid(), 0);
        format_string(buffer, "Additional RR Process (PID: %d) started\n", getpid());
        
        for (int i = 0; i < 5; i++) {
            format_string(buffer, "Additional RR Process (PID: %d) iteration %d\n", getpid(), i);
            write_to_file(log_fd, buffer);
            cpu_bound_task(2);
            sleep(50);
        }
        exit();
    }
    
    // Additional SJF Process
    if ((pids[4] = fork()) == 0) {
        change_queue(getpid(), 1);
        set_SJF_params(getpid(), 100, 80);
        format_string(buffer, "Additional SJF Process (PID: %d) started\n", getpid());
        
        for (int i = 0; i < 5; i++) {
            format_string(buffer, "Additional SJF Process (PID: %d) iteration %d\n", getpid(), i);
            write_to_file(log_fd, buffer);
            cpu_bound_task(3);
            sleep(50);
        }
        exit();
    }
    
    // Additional FCFS Process
    if ((pids[5] = fork()) == 0) {
        change_queue(getpid(), 2);
        format_string(buffer, "Additional FCFS Process (PID: %d) started\n", getpid());
        
        for (int i = 0; i < 5; i++) {
            format_string(buffer, "Additional FCFS Process (PID: %d) iteration %d\n", getpid(), i);
            write_to_file(log_fd, buffer);
            cpu_bound_task(1);
            sleep(50);
        }
        exit();
    }
    
    // Another Round Robin Process
    if ((pids[6] = fork()) == 0) {
        change_queue(getpid(), 0);
        format_string(buffer, "Another RR Process (PID: %d) started\n", getpid());
        
        for (int i = 0; i < 5; i++) {
            format_string(buffer, "Another RR Process (PID: %d) iteration %d\n", getpid(), i);
            write_to_file(log_fd, buffer);
            cpu_bound_task(2);
            sleep(50);
        }
        exit();
    }
    
    // Another SJF Process
    if ((pids[7] = fork()) == 0) {
        change_queue(getpid(), 1);
        set_SJF_params(getpid(), 100, 80);
        format_string(buffer, "Another SJF Process (PID: %d) started\n", getpid());
        
        for (int i = 0; i < 5; i++) {
            format_string(buffer, "Another SJF Process (PID: %d) iteration %d\n", getpid(), i);
            write_to_file(log_fd, buffer);
            cpu_bound_task(3);
            sleep(50);
        }
        exit();
    }
    
    // Another FCFS Process
    if ((pids[8] = fork()) == 0) {
        change_queue(getpid(), 2);
        format_string(buffer, "Another FCFS Process (PID: %d) started\n", getpid());
        
        for (int i = 0; i < 5; i++) {
            format_string(buffer, "Another FCFS Process (PID: %d) iteration %d\n", getpid(), i);
            write_to_file(log_fd, buffer);
            cpu_bound_task(1);
            sleep(50);
        }
        exit();
    }
    
    // Yet Another Round Robin Process
    if ((pids[9] = fork()) == 0) {
        change_queue(getpid(), 0);
        format_string(buffer, "Yet Another RR Process (PID: %d) started\n", getpid());
        
        for (int i = 0; i < 5; i++) {
            format_string(buffer, "Yet Another RR Process (PID: %d) iteration %d\n", getpid(), i);
            write_to_file(log_fd, buffer);
            cpu_bound_task(2);
            sleep(50);
        }
        exit();
    }
    
    // Yet Another SJF Process
    if ((pids[10] = fork()) == 0) {
        change_queue(getpid(), 1);
        set_SJF_params(getpid(), 100, 80);
        format_string(buffer, "Yet Another SJF Process (PID: %d) started\n", getpid());
        
        for (int i = 0; i < 5; i++) {
            format_string(buffer, "Yet Another SJF Process (PID: %d) iteration %d\n", getpid(), i);
            write_to_file(log_fd, buffer);
            cpu_bound_task(3);
            sleep(50);
        }
        exit();
    }
    
    // Yet Another FCFS Process
    if ((pids[11] = fork()) == 0) {
        change_queue(getpid(), 2);
        format_string(buffer, "Yet Another FCFS Process (PID: %d) started\n", getpid());
        
        for (int i = 0; i < 5; i++) {
            format_string(buffer, "Yet Another FCFS Process (PID: %d) iteration %d\n", getpid(), i);
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
    
    for (int i = 0; i < 12; i++) {
        wait();
    }
}

// Test queue changes
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

// Test set_SJF_params system call
void test_set_SJF_params(int log_fd) {
    format_string(buffer, "\n=== Testing set_SJF_params System Call ===\n");
    write_to_file(log_fd, buffer);

    int pid = fork();

    if (pid == 0) {
        int burst_time = 100;
        int confidence = 80;
        set_SJF_params(getpid(), burst_time, confidence);
        format_string(buffer, "Child process (PID: %d) set SJF params: burst_time=%d, confidence=%d\n", getpid(), burst_time, confidence);
        write_to_file(log_fd, buffer);
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
    test_set_SJF_params(log_fd);

    // Write completion message
    write_to_file(log_fd, "\nAll tests completed.\n");
    
    // Close log file
    close(log_fd);
    printf(1, "\nTests completed. Results saved to %s\n", filename);
    exit();
}
