struct stat;
struct rtcdate;

// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int exec(char*, char**);
int open(const char*, int);
int mknod(const char*, short, short);
int unlink(const char*);
int fstat(int fd, struct stat*);
int link(const char*, const char*);
int mkdir(const char*);
int chdir(const char*);
int dup(int);
int getpid(void);
char* sbrk(int);
int sleep(int);
int uptime(void);

// ulib.c
int stat(const char*, struct stat*);
char* strcpy(char*, const char*);
void *memmove(void*, const void*, int);
char* strchr(const char*, char c);
int strcmp(const char*, const char*);
void printf(int, const char*, ...);
char* gets(char*, int max);
uint strlen(const char*);
void* memset(void*, int, uint);
void* malloc(uint);
void free(void*);
int atoi(const char*);


int create_palindrome(int num);
int move_file(const char *src_file, const char *dest_dir);
int sort_syscalls(int pid);
int get_most_invoked_syscall(int pid);
int list_all_processes(void);

int chqueue(int, int);
int bjsproc(int, float, float, float, float);
int bjssys(float, float, float, float);
int procinfo(void);



int set_scheduling_queue(int pid, int queue);
int print_processes_info(void);
int set_estimated_runtime(int pid, int runtime);
int change_queue(int pid, int queue);

int change_queue(int pid, int new_queue_level);
int set_estimated_runtime(int pid, int runtime, int confidence);
int print_processes_info(void);