#include "types.h"
#include "stat.h"
#include "user.h"

#define ID 10
#define SIZE 4

// Function to calculate the factorial of the number
int factorial(int num) {
    if (num <= 1) return 1;
    return num * factorial(num - 1);
}

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        printf(1, "Usage: test_shm <number>\n");
        exit();
    }

    int n = atoi(argv[1]);
    if (n < 1) {
        printf(1, "Invalid number. Please enter a positive integer.\n");
        exit();
    }

    int* addr = (int*)open_sharedmem(ID);
    if ((int)addr < 0) {
        printf(1, "Failed to open shared memory\n");
        exit();
    }

    int fact_n_minus_1 = factorial(n - 1);
    // Write the factorial of (n-1) into the shared memory
    *addr = fact_n_minus_1;

    int pid = fork();
    if (pid < 0) {
        printf(1, "Fork failed\n");
        close_sharedmem(ID);
        exit();
    } else if (pid == 0) {
        // Child process
        int* address = (int*)open_sharedmem(ID);
        int data = *address;
        int result = data * n;
        *address = result;
        printf(1, "Child:\n       Factorial of (%d-1) = %d\n       Final Result = %d\n", n, data, result);
        close_sharedmem(ID);
        exit();
    } else {
        // Parent process
        wait();
        int* address = (int*)open_sharedmem(ID);
        int final_result = *address;
        printf(1, "Parent:\n       Factorial of (%d) = %d\n", n , final_result);
        close_sharedmem(ID);
    }

    exit();
}
