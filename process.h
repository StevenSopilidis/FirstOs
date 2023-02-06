#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "trap.h"
#include "lib.h"

// PCB (process controll block)
// PCB is saved in kernel space thus
// not accessible by user program
struct Process {
    struct List *next; // next process that will run at PIT interupt
    int pid; // id of the process
    int state; // state of process (waiting, ready, running etc)
    int wait; // why the process was put to sleep?
    uint64_t context; // used to save rsp value of process
    uint64_t page_map; // address of processes pml4 table
    uint64_t stack; // address of kernel stack (will allocate 2 MB page for it) (process has two stacks on for kernel mode and one for user mode)
    struct TrapFrame *tf;
};

// for setting up stack pointer for ring 0
struct TSS {
    uint32_t res0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t res1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t res2;
    uint16_t res3;
    uint16_t iopb;
} __attribute__((packed));

struct ProcessControl {
    struct Process *current_process;
    struct HeadList ready_list; // processes that are ready to run
    struct HeadList wait_list; // here waiting process will reside
    struct HeadList kill_list; // list of processes that were killed
};


#define STACK_SIZE (2*1024*1024) 
#define NUM_PROC 10
// process states
#define PROC_UNUSED 0
#define PROC_INIT 1
#define PROC_RUNNING 2
#define PROC_READY 3
#define PROC_SLEEP 4
#define PROC_KILLED 5


void init_process(void);
void launch(void);
void pstart(struct TrapFrame *tf);
void yield(void);
void sleep(int wait);
void wake_up(int wait);
void exit(void);
void wait(void);


#endif