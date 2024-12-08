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
        set_sjf_proc(getpid(),50 ,2);
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
