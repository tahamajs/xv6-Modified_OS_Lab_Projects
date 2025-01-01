
#pragma once
#include "spinlock.h"

// Reentrant lock structure
struct reentrantlock {
    uint locked;       // 0 or 1
    int owner;         // pid of the lock holder
    int count;         // how many times the owner acquired it
    struct spinlock lk;
    char *name;
};

extern struct reentrantlock testlock;

// Function prototypes
void initreentrantlock(struct reentrantlock *r, char *name);
int acquirereentrantlock(struct reentrantlock *r);
int releasereentrantlock(struct reentrantlock *r);


