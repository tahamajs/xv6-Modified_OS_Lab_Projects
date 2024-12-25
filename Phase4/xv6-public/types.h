

#ifndef TYPES_H
#define TYPES_H


typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef uint pde_t;

// Structure for collecting system call statistics
struct syscall_stat {
    int count;
    char* name;
};
#endif