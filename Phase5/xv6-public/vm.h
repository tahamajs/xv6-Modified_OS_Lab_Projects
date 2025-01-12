#define NSHPAGE   64
#define HEAPLIMIT 0x7F000000 // 2GB - 128MB

struct SharedPage {
    int id;
    int n_access;
    uint physicalAddr;
};

struct SharedRegion {
    struct SharedPage pages[NSHPAGE];
    struct spinlock lock;
};

extern struct SharedRegion SharedRegion;
