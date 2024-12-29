#include "types.h"
#include "defs.h"
#include "spinlock.h"
#include "reentrantlock.h"
#include "proc.h"
#include "x86.h"

struct reentrantlock testlock;

// Initialize the reentrant lock
void initreentrantlock(struct reentrantlock *r, char *name) {
    initlock(&r->lk, name);
    r->locked = 0;
    r->owner = 0;
    r->count = 0;
    r->name = name;
}

// Acquire the reentrant lock
void acquirereentrantlock(struct reentrantlock *r) {
    pushcli();
    acquire(&r->lk);

    if(r->locked && r->owner == myproc()->pid) {
        r->count++;
        release(&r->lk);
        popcli();
        return;
    }
    while(xchg(&r->locked, 1) != 0) {
        // simple busy-wait
    }
    r->owner = myproc()->pid;
    r->count = 1;

    release(&r->lk);
    popcli();
}

// Release the reentrant lock
void releasereentrantlock(struct reentrantlock *r) {
    pushcli();
    acquire(&r->lk);

    if(r->owner != myproc()->pid) {
        panic("releasereentrantlock: not owner");
    }
    r->count--;
    if(r->count == 0) {
        r->owner = 0;
        r->locked = 0;
    }

    release(&r->lk);
    popcli();
}