// test_list_processes.c

#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
    printf(1, "Listing all processes:\n");
    list_all_processes();
    exit();
}
