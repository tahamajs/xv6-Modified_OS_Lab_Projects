// System call numbers
#define SYS_fork                     1
#define SYS_exit                     2
#define SYS_wait                     3
#define SYS_pipe                     4
#define SYS_read                     5
#define SYS_kill                     6
#define SYS_exec                     7
#define SYS_fstat                    8
#define SYS_chdir                    9
#define SYS_dup                     10
#define SYS_getpid                  11
#define SYS_sbrk                    12
#define SYS_sleep                   13
#define SYS_uptime                  14
#define SYS_open                    15
#define SYS_write                   16
#define SYS_mknod                   17
#define SYS_unlink                  18
#define SYS_link                    19
#define SYS_mkdir                   20
#define SYS_close                   21
#define SYS_change_queue            22
#define SYS_set_estimated_runtime   23
#define SYS_open_sharedmem          24
#define SYS_close_sharedmem         25
#define SYS_print_processes_info    26
#define SYS_scinfo                  27
#define SYS_reacquire               28
#define SYS_rerelease               29
#define SYS_create_palindrome       30
#define SYS_move_file               31
#define SYS_sort_syscalls           32
#define SYS_get_most_invoked_syscall 33
#define SYS_list_all_processes      34
#define SYS_set_scheduling_queue    35
#define SYS_printprocs              36
#define SYS_set_sjf_proc            37
#define SYS_set_sjf_sys             38
#define SYS_chqueue                 39
#define SYS_bjsproc                 40
#define SYS_bjssys                  41
#define SYS_user_program            42
#define SYS_set_SJF_params          43
#define SYS_nsyscalls               44
#define SYS_get_all_cpus_syscalls   45

#define MAX_SYSCALLS                  100

// Remove these function declarations - they don't belong in syscall.h
// void* open_sharedmem(int shmid);
// int close_sharedmem(void *addr);





