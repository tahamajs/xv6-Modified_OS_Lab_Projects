
#pragma once

#define DEBUG_SCHED 1

#ifdef DEBUG_SCHED
#define SCHED_DEBUG(fmt, ...) \
    do { cprintf("SCHED: " fmt, ##__VA_ARGS__); } while(0)
#else
#define SCHED_DEBUG(fmt, ...)
#endif

#define ASSERT(condition) \
    do { \
        if(!(condition)) { \
            cprintf("Assertion failed: %s\n", #condition); \
            panic("assertion failed"); \
        } \
    } while(0)