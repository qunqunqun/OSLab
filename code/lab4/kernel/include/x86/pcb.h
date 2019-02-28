#ifndef __X86_PCB_H__
#define __X86_PCB_H__

#include <common.h>
#include "memory.h"

#define KERNEL_STACK_SIZE (16 << 10)  // kernel stack size (16KB)
#define PROC_SIZE (1 << 16)    //size of a PROC
#define APP_START 0X200000 //USER CODE START
#define MAX_PCB_NUM 10  // PCB size
#define TIMESLICE 10    //slice of each proc
#define PID_START 250 //init PID
#define IDLE_STACK 0x200000

// process state
#define BLOCKED 0
#define DEAD 1
#define RUNNING 2
#define RUNNABLE 3

#define NR_PCB(p) (p - pcb)

struct ProcessTable //lian biao jie gou
{
    union {
        uint8_t stack[KERNEL_STACK_SIZE];  // kernel stack
        struct 
        {
            uint8_t pad[KERNEL_STACK_SIZE - sizeof(struct TrapFrame)];
            struct TrapFrame tf;
        } __attribute__((packed));
    };
    int state;
    int timeCount;
    int sleepTime;
    uint32_t pid;
    struct ProcessTable *next;
};

struct ProcessTable pcb[MAX_PCB_NUM];

struct ProcessTable * pcb_head;  // allocated pcb list head
struct ProcessTable * pcb_blocked;//blocked pcb
struct ProcessTable * pcb_stop;  //zu shai
struct ProcessTable * pcb_cur;   // current runnning process

void initpcb();
void enter_proc(uint32_t entry);
void PCB_schedule();
struct ProcessTable * new_pcb();

#endif