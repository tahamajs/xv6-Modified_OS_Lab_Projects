#include "types.h"
#include "stat.h"
#include "user.h"

// Function to calculate Fibonacci numbers recursively
int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

void fibonacci_test(int limit) {
    for (int i = 0; i < limit + 1; i++) {
        printf(1, "Fibonacci of %d is %d\n", i, fibonacci(i));
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf(1, "Usage: fibonacci <limit>\n");
        exit();
    }
    int limit = atoi(argv[1]);
    fibonacci_test(limit);
    exit();
}
