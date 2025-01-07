// ...existing includes if necessary...
#include "spinlock.h"

#define SHAREDREGIONS 64

struct shmRegion {
    int shmid;
    int size;              // number of pages
    int shm_nattch;        // number of attached processes
    uint shm_segsz;        // size of the segment
    uint physicalAddr[64]; // physical frames for each page
};

struct shm_manager {
    struct spinlock lock;
    struct shmRegion allRegions[SHAREDREGIONS];
};

extern struct shm_manager shmTable;
int create_shm(uint size, int shmid);
int mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm);  // Changed char* to void*
pte_t* walkpgdir(pde_t *pgdir, const void *va, int alloc);
// ...existing or needed prototypes...
