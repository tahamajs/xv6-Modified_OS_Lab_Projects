#include "types.h"
#include "stat.h"
#include "user.h"

// Heavy computation function to keep CPU busy
long compute_factorial(int n) {
    long result = 1;
    for(int i = 1; i <= n; i++) {
        result *= i;
        // Add some more computations to make it heavier
        for(int j = 0; j < 1000; j++) {
            result = (result * 7919) % 999983; // Large prime numbers
        }
    }
    return result;
}

// Matrix multiplication to create CPU load
void matrix_multiply(int size) {
    int matrix1[20][20], matrix2[20][20], result[20][20];
    
    // Initialize matrices
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
            matrix1[i][j] = i + j;
            matrix2[i][j] = i * j;
        }
    }
    
    // Multiply matrices
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
            result[i][j] = 0;
            for(int k = 0; k < size; k++) {
                result[i][j] += matrix1[i][k] * matrix2[k][j];
            }
        }
    }
}

void test_different_queues() {
    int pid = fork();
    
    if(pid < 0) {
        printf(1, "Fork failed\n");
        exit();
    }
    
    if(pid == 0) {  // Child process 1 - ROUND_ROBIN
        change_queue(getpid(), 0); // Set to ROUND_ROBIN queue
        printf(1, "Child 1 (PID: %d) running in ROUND_ROBIN queue\n", getpid());
        for(int i = 0; i < 5; i++) {
            compute_factorial(20);
            matrix_multiply(10);
        }
        exit();
    }
    
    pid = fork();
    if(pid == 0) {  // Child process 2 - SJF
        change_queue(getpid(), 1); // Set to SJF queue
        printf(1, "Child 2 (PID: %d) running in SJF queue\n", getpid());
        // Set SJF parameters
        set_sjf_proc(getpid(), 1.0, 0.5, 0.3, 0.2);
        for(int i = 0; i < 3; i++) {
            compute_factorial(15);
            matrix_multiply(8);
        }
        exit();
    }
    
    pid = fork();
    if(pid == 0) {  // Child process 3 - FCFS
        change_queue(getpid(), 2); // Set to FCFS queue
        printf(1, "Child 3 (PID: %d) running in FCFS queue\n", getpid());
        for(int i = 0; i < 4; i++) {
            compute_factorial(18);
            matrix_multiply(12);
        }
        exit();
    }
    
    // Parent process
    printf(1, "Parent process (PID: %d) waiting for children\n", getpid());
    
    // Print process info periodically
    for(int i = 0; i < 3; i++) {
        print_processes_info();
        compute_factorial(10); // Add some delay between prints
    }
    
    // Wait for all children
    for(int i = 0; i < 3; i++) {
        wait();
    }
    
    printf(1, "All child processes completed\n");
    print_processes_info();
}

int main() {
    printf(1, "Starting scheduler test...\n");
    test_different_queues();
    exit();
}
