// test_palindrome.c

#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf(2, "Usage: test_palindrome <number>\n");
        exit();
    }

    int num = atoi(argv[1]);
    create_palindrome(num);

    exit();
}
