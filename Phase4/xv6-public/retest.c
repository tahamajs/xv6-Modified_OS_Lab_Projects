#include "types.h"
#include "stat.h"
#include "user.h"

void test_relock(int lvl) {
    if(lvl<=0) return;
    reacquire();
    printf(1, "reacquire at level %d\n", lvl);
    test_relock(lvl-1);
    printf(1, "rerelease at level %d\n", lvl);
    rerelease();
}

int main(int argc, char *argv[])
{
    printf(1, "Testing reentrant lock...\n");
    test_relock(3);

    int c = scinfo(); 
    printf(1, "Global syscall count=%d\n", c);

    exit();
}
