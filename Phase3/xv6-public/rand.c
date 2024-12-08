#include "types.h"

// A simple linear congruential generator (LCG)
static uint seed = 1;

void srand(uint s) {
    seed = s;
}

uint rand(void) {
    // Constants for LCG
    seed = (seed * 1664525 + 1013904223) % ((uint) 1 << 31);
    return seed;
}
