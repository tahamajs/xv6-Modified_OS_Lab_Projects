#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
    printf(1, "Testing reentrant lock...\n");

    // First acquire
    printf(1, "First lock acquire\n");
    if(reacquire() < 0) {
        printf(1, "First acquire failed\n");
        exit();
    }
    
    // Second acquire (should work because it's reentrant)
    printf(1, "Second lock acquire (reentrant)\n");
    if(reacquire() < 0) {
        printf(1, "Second acquire failed\n");
        exit();
    }
    
    // Release first time
    printf(1, "First lock release\n");
    if(rerelease() < 0) {
        printf(1, "First release failed\n");
        exit();
    }

    // Release second time
    printf(1, "Second lock release\n");
    if(rerelease() < 0) {
        printf(1, "Second release failed\n");
        exit();
    }

    // Try to release when not held (should fail)
    printf(1, "Attempting to release unheld lock (should fail)\n");
    if(rerelease() == 0) {
        printf(1, "ERROR: Released unheld lock!\n");
    } else {
        printf(1, "Correctly failed to release unheld lock\n");
    }

    printf(1, "Test completed\n");
    exit();
}
