#include "device.h"
#include "x86.h"

extern SegDesc gdt[NR_SEGMENTS];
extern TSS tss;

void IDLE() 
{
    asm volatile("movl %0, %%esp;" ::"i"(IDLE_STACK));
    asm volatile("sti");
    waitForInterrupt();
}

void initpcb() 
{
    for(int i = 0 ; i < MAX_PCB_NUM; i++)
    {
        pcb[i].state = DEAD;
    }
    pcb_head = NULL;
    pcb_cur = NULL;
}

void enter_proc(uint32_t entry)
{
    struct ProcessTable * p = new_pcb();   //father proc

    pcb_cur = p;

    asm volatile("movl %0, %%eax" ::"r"(USEL(SEG_UDATA)));
    asm volatile("movw %ax, %ds");
    asm volatile("movw %ax, %es");
    asm volatile("movw %ax, %fs");

    p->tf.ss = USEL(SEG_UDATA);
    p->tf.esp = APP_START + PROC_SIZE;
    p->tf.cs = USEL(SEG_UCODE);
    p->tf.eip = entry;
    p->timeCount = TIMESLICE;
    p->state = RUNNING;
    p->sleepTime = 0;
    p->pid = PID_START;

    asm volatile("sti");
    asm volatile("pushfl");  // %eflags
    asm volatile("cli");

    asm volatile("movl (%%esp), %0" : "=r"(p->tf.eflags) : );

    // return to user space
    asm volatile("movl %0, %%esp" ::"r"(&p->tf.eip));
    asm volatile("iret");
    assert(0);
}

void PCB_schedule() 
{
    
    struct ProcessTable *p;
    // move current process to the end of the list
     if (pcb_head != NULL) 
     {
        p = pcb_head->next;
        if (p != NULL) 
        {
            pcb_cur->next = NULL;
            pcb_head = p;
            while (p->next != NULL) 
            {
                p = p->next;
            }
            p->next = pcb_cur;
        }
    }

    // find next RUNNABLE process
    pcb_cur = NULL;
    p = pcb_head;
    while (p != NULL) 
    {
        if (p->state == RUNNABLE) 
        {
            pcb_cur = p;
            p->state = RUNNING ;           
            break;
        }
        p = p->next;
    }

    // load next process
    if (pcb_cur == NULL) 
    {  
         // IDLE process
         putChar('x');
        IDLE();
    } 
    else 
    {
        tss.esp0 = (uint32_t)&(pcb_cur->stack[KERNEL_STACK_SIZE]);
        tss.ss0  = KSEL(SEG_KDATA);

        gdt[SEG_UCODE] = SEG(STA_X | STA_R, (pcb_cur - pcb) * PROC_SIZE, 0xffffffff, DPL_USER);
        gdt[SEG_UDATA] = SEG(STA_W,         (pcb_cur - pcb) * PROC_SIZE, 0xffffffff, DPL_USER);

        asm volatile("pushl %eax"); // save eax
        asm volatile("movl %0, %%eax" ::"r"(USEL(SEG_UDATA)));
        asm volatile("movw %ax, %ds");
        asm volatile("movw %ax, %es");
        asm volatile("popl %eax");

        // restore process information
        asm volatile("movl %0, %%esp" ::"r"(&pcb_cur->tf));
        asm volatile("popl %gs");
        asm volatile("popl %fs");
        asm volatile("popl %es");
        asm volatile("popl %ds");
        asm volatile("popal");
        asm volatile("addl $8, %esp");

        // return to user space
        asm volatile("iret");
    }
}

int FIND_USABLE()
{
    int i;
    for( i = 0 ;i < MAX_PCB_NUM ; i++)
    {
        if(pcb[i].state == DEAD)
        {
            break;
        }
    }
    if( i == MAX_PCB_NUM)
    {
        assert(0);
    }
    return i;
}

struct ProcessTable *new_pcb() 
{
    int i = FIND_USABLE();

    struct ProcessTable * p = &pcb[i];
    p -> next = NULL;
    p -> sleepTime = 0;
    p -> pid = PID_START + NR_PCB(p);
    p -> state = RUNNABLE;
    p -> timeCount = TIMESLICE;
    //add to list
    if (pcb_head == NULL)
    {
        pcb_head = p;
    } 
    else 
    {
        struct ProcessTable * m = pcb_head;
        while (m-> next != NULL) 
        {
            m = m->next;
        }
        m->next = p;
    }
    return p;
}
