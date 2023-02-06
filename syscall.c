#include "syscall.h"
#include "print.h"
#include "debug.h"
#include "stddef.h"
#include "process.h"

// system call functions in our system
static SYSTEMCALL system_calls[10];

// write screen system call method
// argptr -> user data on stack that was pushed
static int sys_write(uint64_t* argptr)
{
    write_screen((char*)argptr[0], (int)argptr[1], 0xe);
    return (int)argptr[1];
}

// sleep process system call
// argptr -> user data on stack that was pushed
static int sys_sleep(int64_t* argptr)
{
    uint64_t old_ticks; 
    uint64_t ticks;
    uint64_t sleep_ticks = argptr[0];

    ticks = get_ticks();
    old_ticks = ticks;

    while (ticks - old_ticks < sleep_ticks) {
       sleep(-1); // in our system -1 represents waiting for ticks
       ticks = get_ticks();
    }

    return 0;
}

// exit system call method
// argptr -> user data on stack that was pushed
static int sys_exit(int64_t* argptr) 
{
    exit();
    return 0;
}

static int sys_wait(int64_t* argptr) 
{
    wait();
    return 0;
}

// wait system call method
// argptr -> user data on stack that was pushed


// function to initialize system calls
void init_system_call(void)
{
    system_calls[0] = sys_write;
    system_calls[1] = sys_sleep;
    system_calls[2] = sys_wait;
    system_calls[3] = sys_exit;
}

// functio that detects which system call method 
// to execute
// rax register holds index of system call
// also rax will hold result of systemcall
// which in case it fails holds -1
// rdi register holds the number of parameters
// pushed in system call
// rsi holds reference to the parameters we passed
void system_call(struct TrapFrame* tf)
{
    int64_t i = tf->rax;
    int64_t param_count = tf->rdi;
    int64_t *argptr = (int64_t)tf->rsi;

    if(param_count < 0 || i > 3 || i <0) {
        tf->rax = -1;
        return;
    }

    ASSERT(system_calls [i] != NULL);
    // call system call method and return the result via the rax
    tf->rax = system_calls[i](argptr); 
}