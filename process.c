#include "process.h"
#include "trap.h"
#include "memory.h"
#include "print.h"
#include "lib.h"
#include "debug.h"

extern struct TSS Tss;
// structure holding all system processes
static struct Process process_table[NUM_PROC];
static int pid_num = 1; 
static struct ProcessControl pc;


//set tss for process
static void set_tss(struct Process *proc) 
{
    Tss.rsp0 = proc->stack + STACK_SIZE;
}

// function for finding an unused process from the 
// process_table
static struct Process* find_unused_process(void)
{
    struct Process* proc = NULL;
    for (int i = 0; i < NUM_PROC; i++)
    {
        if(process_table[i].state == PROC_UNUSED)
        {
            proc = &process_table[i];
            break;
        }
    }
    return proc;
}

// function for setting up a process
// @param proc -> process to initialize
// @param addr -> physical address of process
static void set_process_entry(struct Process* proc, uint64_t addr) 
{
    uint64_t stack_top;

    proc->state = PROC_INIT;
    proc->pid = pid_num++;

    proc->stack = (uint64_t)kalloc();
    ASSERT(proc->stack != 0);

    memset((void *)proc->stack, 0, STACK_SIZE);
    stack_top = proc->stack + STACK_SIZE; // !! stack grows downwords

    proc->context = stack_top - sizeof(struct TrapFrame) - 7*8;
    *(uint64_t *)(proc->context + 6*8) = (uint64_t)TrapReturn;

    // trap frame is located on the top of kernel stack
    proc->tf = (struct TrapFrame*)(stack_top - sizeof(struct TrapFrame));
    // rip, cs, rflags, rsp and ss
    // will be poped of the stack when we interupt return
    proc->tf->cs = 0x10|3;
    proc->tf->rip = 0x400000;
    proc->tf->ss = 0x18|3;
    proc->tf->rsp = 0x400000 + PAGE_SIZE;
    proc->tf->rflags = 0x202;

    proc->page_map = setup_kvm();
    ASSERT(proc->page_map != 0);
    // 0x20_000 -> physical address of where we loaded user program
    // 5120 -> user program is 10 sectors long
    ASSERT(setup_uvm(proc->page_map, P2V(addr), 5120));
    proc->state = PROC_READY;
}

static struct ProcessControl* get_pc(void)
{
    return &pc;
}

// initializes a process
void init_process(void)
{  
    struct ProcessControl *process_control;
    struct Process *process;
    struct HeadList *list;
    uint64_t addr[3] = {0x20000, 0x30000, 0x40000};

    process_control = get_pc();
    list = &process_control->ready_list;

    for (int i = 0; i < 3; i++)
    {
        process = find_unused_process();
        set_process_entry(process, addr[i]);
        append_list_tail(list, (struct List*)process);
    }
    
}

// starts the process
void launch(void)
{
    struct ProcessControl *process_control;
    struct Process *process;

    process_control = get_pc();
    process = (struct Process*)remove_list_head(&process_control->ready_list);
    process->state = PROC_RUNNING;
    process_control->current_process = process;

    set_tss(process);
    switch_vm(process->page_map);
    pstart(process->tf);
}


static void switch_process(struct Process *prev, struct Process *current)
{
    set_tss(current);
    switch_vm(current->page_map);
    // after returning from swap we will be running
    // not other process!!
    swap(&prev->context, current->context);
}

static void schedule(void)
{
    struct Process *prev_proc;
    struct Process *current_proc;
    struct ProcessControl *process_control;
    struct HeadList *list;

    process_control = get_pc();
    prev_proc = process_control->current_process;
    list = &process_control->ready_list;
    ASSERT(!is_list_empty(list));
    
    current_proc = (struct Process*)remove_list_head(list);
    current_proc->state = PROC_RUNNING;   
    process_control->current_process = current_proc;

    switch_process(prev_proc, current_proc);   
}


void yield(void)
{
    struct ProcessControl *process_control;
    struct Process *process;
    struct HeadList *list;
    
    process_control = get_pc();
    list = &process_control->ready_list;

    // if list is keep running the current process
    if (is_list_empty(list)) {
        return;
    }

    process = process_control->current_process;
    process->state = PROC_READY;
    append_list_tail(list, (struct List*)process);
    schedule();
}

// func for setting the currently running 
// process to sleeping state
// wait -> pid of process
void sleep(int wait)
{
    struct ProcessControl *process_control;
    struct Process *process;
    
    process_control = get_pc();
    process = process_control->current_process;
    process->state = PROC_SLEEP;
    process->wait = wait;

    append_list_tail(&process_control->wait_list, (struct List*)process);
    schedule();
}

// func for waking up a sleeping process
// @wait: pid of process to wake
void wake_up(int wait)
{
    struct ProcessControl *process_control;
    struct Process *process;
    struct HeadList *ready_list;
    struct HeadList *wait_list;

    process_control = get_pc();
    ready_list = &process_control->ready_list;
    wait_list = &process_control->wait_list;
    process = (struct Process*)remove_list(wait_list, wait);

    while (process != NULL) {       
        process->state = PROC_READY;
        append_list_tail(ready_list, (struct List*)process);
        process = (struct Process*)remove_list(wait_list, wait);
    }
}

void exit(void) 
{
    struct ProcessControl *process_control;
    struct Process *process;
    struct HeadList *list;

    process_control = get_pc();
    process = process_control->current_process;
    process->state = PROC_KILLED;

    list = &process_control->kill_list;
    append_list_tail(list, (struct List*)process);

    // wake up init_process to do cleanup before switch up processes
    // just in case it sleeps
    // 1 -> pid of init process
    wake_up(1); 
    schedule();
}

void wait(void) {
    struct ProcessControl *process_control;
    struct Process *process;
    struct HeadList *list;

    process_control = get_pc();
    list = &process_control->kill_list;

    while (1)
    {
        if (!is_list_empty(list))
        {
            // clean up all killed processes
            process = (struct Process*)remove_list_head(list);
            ASSERT(process->state == PROC_KILLED);

            kfree(process->stack);
            free_vm(process->page_map);
            memset(process, 0, sizeof(struct Process));
        }  else {
            // sleep init process
            sleep(1);
        }
        
    }
}