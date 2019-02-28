#include "x86.h"
#include "device.h"
#define SYS_exit 1
#define SYS_fork 2
#define SYS_write 4
//undefined syscall
#define SYS_SEM_init 250
#define SYS_SEM_post 251
#define SYS_SEM_wait 252
#define SYS_SEM_destory 253
#define SYS_sleep 300  // user defined

//up date lab3
//defined in pro.c
void schedule();	//diao du 

void TimerInterruptHandle(struct TrapFrame *tf); //shi zhong zhongduan

//lab2
void syscallHandle(struct TrapFrame *tf); // xi tong diao yong

void GProtectFaultHandle(struct TrapFrame *tf); // protect handler

//sys_call
void sys_fork(struct TrapFrame *tf);
void sys_sleep(struct TrapFrame *tf);
void sys_exit(struct TrapFrame *tf);
void sys_write(struct TrapFrame *tf);
void sys_sem_init(struct TrapFrame *tf);
void sys_sem_post(struct TrapFrame *tf);
void sys_sem_wait(struct TrapFrame *tf);
void sys_sem_destory(struct TrapFrame *tf);



void irqHandle(struct TrapFrame *tf) 
{
	/*
	 * 中断处理程序
	 */
	/* Reassign segment register */
    asm volatile("movl %0, %%eax" ::"r"(KSEL(SEG_KDATA)));
    asm volatile("movw %ax, %ds");
    asm volatile("movw %ax, %fs");
    asm volatile("movw %ax, %es");
    asm volatile("movl %0, %%eax" ::"r"(KSEL(SEG_VIDEO)));
    asm volatile("movw %ax, %gs");
	//assert(0);
	switch(tf->irq) 
	{
		case -1:
			break;
		case 0xd:
			GProtectFaultHandle(tf);
			break;
		case 0x20:
			TimerInterruptHandle(tf);
            break;
		case 0x80:
			syscallHandle(tf);
			break;
		default:
			assert(0);
	}
}

void TimerInterruptHandle(struct TrapFrame *tf)//shi zhong zhongduan
{	
	//diao du shi jian 
	struct ProcessTable * m;
	m = pcb_blocked;
	while(m != NULL)
	{
		if(m -> sleepTime > 0) //BLOCEK
		{
			m -> sleepTime --;
		}
		if(m -> sleepTime == 0) // RUNNABLE
		{

			m -> state = RUNNABLE;
			//remove from BLOCEKED
			struct ProcessTable * p, * n;
			if(pcb_blocked == m)
			{
				p = pcb_blocked;
				pcb_blocked= pcb_blocked->next;
			}	
			else
			{
				p = pcb_blocked;
				n = p -> next; //obviously, n is not empty
				while(p -> next != NULL)
				{
					if(n == m)//find and remove it
					{
						p-> next = n->next;
						break;
					}
					p = p -> next;
					n = p -> next;
			}
				if(n == NULL) assert(0);//must find one
			}
			//add into head
			if(pcb_head == NULL)
			{
				pcb_head = m;
				m -> next = NULL;
			}
			else
			{
				struct ProcessTable * n = pcb_head;
				while(n -> next != NULL)
				{
					n = n-> next;
				}
				n -> next = m;
				m -> next = NULL;
			}
			
		}
		m = m -> next;
	}

	//find the pcb
	if(pcb_cur == NULL) // no proc running
	{
		PCB_schedule();
	}
	else
	{
		pcb_cur-> timeCount --;
		if(pcb_cur -> timeCount == 0) //run out
		{
			pcb_cur -> state = RUNNABLE;
			pcb_cur -> timeCount = TIMESLICE;
			PCB_schedule();	
		}
	}
	putChar('~');
}

void sys_write(struct TrapFrame *tf)
{
	asm volatile("movl %0 , %%eax"::"r"(KSEL(SEG_VIDEO)));
	asm volatile("movw %ax , %gs");//initialize gs
	// static int prt_row = 5;//produce it out and never change it
	// static int prt_col = 0;
	static int prt_row = 0, prt_col = 0;
	//int length = tf->edx;
	//char *temp =(char *)tf->ecx;
	tf->ecx += ((pcb_cur - pcb) * PROC_SIZE);
	char out = '\0';
	//ebx:file,ecx:str,edx:len
	if(tf->ebx == 1 || tf->ebx == 2)//stdout,stderr
	{
		int i;
		for(i = 0; i < tf->edx ;i++)
		{
			out = *(char *)(tf->ecx + i);
			putChar(out);
			if(prt_col == 80) //full
			{
				prt_row++;
				prt_col = 0;
			}
			if(out == '\n')  //change line
			{
				prt_row ++;
				prt_col = 0;
				continue;
			}
			video_print(prt_row,prt_col,out);
			prt_col ++;
		}
		tf->eax = tf->edx;//retrun value
	}
	else
	{
		assert(0);
	}
}

void sys_fork(struct TrapFrame *tf)
{
	//create a new proc
	struct ProcessTable * m = new_pcb();
	//copy
	uint32_t src_pcb = APP_START + (pcb_cur - pcb) * PROC_SIZE;
	uint32_t dest_pcb = APP_START + (m - pcb) * PROC_SIZE;
	int i = 0;
	while(i < PROC_SIZE) //copy
	{
		*((uint8_t*) dest_pcb + i) = *((uint8_t*) src_pcb + i);
		i++;
	}
	//copy stack[]
	i = 0;
	while(i < KERNEL_STACK_SIZE)
	{
		m->stack[i] = pcb_cur->stack[i];
		i++;
	}
	//set return value
	m -> tf.eax = 0;				//chile process return value
	pcb_cur -> tf.eax = m -> pid;   //father process return value
	//carry on the father process
	return;
};

void sys_sleep(struct TrapFrame *tf)
{
    pcb_cur->sleepTime = tf -> ebx;
    pcb_cur->state = BLOCKED;
	//delete from head 
	struct ProcessTable * m, * n;
	if(pcb_head == pcb_cur)
	{
		m = pcb_head;
		pcb_head= pcb_head->next;
	}
	else
	{
		m = pcb_head;
		n = m -> next; //obviously, n is not empty
		while(m -> next != NULL)
		{
			if(n == pcb_cur)//find and remove it
			{
				m -> next = n->next;
				break;
			}
			m = m -> next;
			n = m -> next;
		}
		if(n == NULL) assert(0);//must find one
	}
	//add into BLOCKED
	if(pcb_blocked == NULL)
	{
		pcb_blocked = pcb_cur;
		pcb_cur -> next = NULL;
	}
	else
	{
		struct ProcessTable * m = pcb_blocked;
		while(m -> next != NULL)
		{
			m = m -> next;
		}
		m -> next = pcb_cur;
		pcb_cur -> next = NULL;
	}
    PCB_schedule();
	//can't reach here
	assert(0);
};

void sys_exit(struct TrapFrame *tf)
{
	struct ProcessTable * m, * n;
	uint32_t pid_temp = pcb_cur -> pid;
	//remove current pid from pid list
	//head
	if(pcb_head->pid == pid_temp)
	{
		m = pcb_head;
		pcb_head= pcb_head->next;
		m->state = DEAD;
	}
	else
	{
		m = pcb_head;
		n = m -> next; //obviously, n is not empty
		while(m -> next != NULL)
		{
			if(n -> pid == pid_temp)//find and remove it
			{
				m -> next = n->next;
				n -> state = DEAD;
				break;
			}
			m = m -> next;
			n = m -> next;
		}
		if(n == NULL) assert(0);//must find one
	}
	pcb_cur = NULL;
	PCB_schedule();
	//can't reach here
	assert(0);
};

void sys_sem_init(struct TrapFrame *tf)	//chu shi hua yi ge 
{
	tf->ebx += ((pcb_cur - pcb) * PROC_SIZE);
	int temp_value = tf->ecx; 
	int res = find_new_sem();
	semaphore[res].value = temp_value;
	semaphore[res].pid = PID_START + pcb_cur - pcb;
	*(sem_t *)tf->ebx = res;
	tf->eax = res;  //return value
};

// function of V
void sys_sem_post(struct TrapFrame *tf)
{
	tf->ebx += ((pcb_cur - pcb) * PROC_SIZE);
	int sem_cur = *(sem_t*) tf->ebx;
	semaphore[sem_cur].value ++;
	tf->eax = 0;
	if(semaphore[sem_cur].used == FREE)
	{
		return;
	}
	if(semaphore[sem_cur].value == 0)		//guq qi
	{
		int pid_res = semaphore[sem_cur].pid;
		assert(pid_res == 1);
		assert(pcb_cur == pcb);
		pcb[pid_res].state = RUNNABLE;
		pcb[pid_res].sleepTime = 0;
		pcb[pid_res].timeCount = TIMESLICE;
		//mov from pcb_stop
		struct ProcessTable * m, * n, *temp;
		temp = &pcb[pid_res];
		m = pcb_stop;	//stop head
		if(m == temp)
		{
			pcb_stop = pcb_stop -> next;
		}
		else{
			n = m -> next ;
			while( m -> next != NULL)
			{
				if(n == temp)
				{
					m->next = n->next;
					break;
				}
				m = m -> next;
				n = m -> next;
			}
			if( n == NULL) //find nothing
			{
				assert(0);
			}
		}
		assert(pcb_stop == NULL);
		//add into head
		m = pcb_head;
		if(pcb_head == NULL)
		{
			pcb_head = temp;
			temp -> next = NULL;
			assert(0);
		}
		else{
				n = pcb_head;
				while(n -> next != NULL)
				{
					n = n -> next;
				}
				n -> next = temp;
				temp -> next = NULL;
		}
		assert(pcb_head != NULL);
		assert(pcb_head->next!=NULL);
		assert(pcb_head == pcb_cur);
		pcb_cur -> state = RUNNABLE;
		pcb_cur -> timeCount = TIMESLICE;
		PCB_schedule();
		assert(0);
	}
};

// function of P
void sys_sem_wait(struct TrapFrame *tf)
{
	tf->ebx += ((pcb_cur - pcb) * PROC_SIZE);
	int sem_cur = *(sem_t*) tf->ebx;
	semaphore[sem_cur].value --;
	tf->eax = 0;
	if(semaphore[sem_cur].used == FREE)
	{
		return;
	}
	if(semaphore[sem_cur].value < 0)		//zu shai itself
	{
		assert((pcb_cur - pcb) == 1);
		semaphore[sem_cur].pid = pcb_cur - pcb; //pid 
		int pid_res = semaphore[sem_cur].pid;
		assert(pid_res == 1);
		pcb_cur->state = BLOCKED;
		struct ProcessTable * m, * n;
		if(pcb_head == pcb_cur)
		{
			m = pcb_head;
			pcb_head = pcb_head -> next;
			//assert(pcb_head != pcb);
		}
		else
		{
			m = pcb_head;
			n = m -> next; //obviously, n is not empty
			while(m -> next != NULL)
			{
				if(n == pcb_cur)//find and remove it
				{
					m -> next = n->next;
					break;
				}
				m = m -> next;
				n = m -> next;
			}
			if(n == NULL) assert(0);//must find one
		}
		//add into BLOCKED
		if(pcb_stop == NULL)
		{
			pcb_stop = pcb_cur;
			pcb_stop -> next = NULL;
		}
		else
		{
			struct ProcessTable * m = pcb_stop;
			while(m -> next != NULL)
			{
				m = m -> next;
			}
			m -> next = pcb_cur;
			m -> next ->next = NULL;
		}
    	PCB_schedule();
		//can't reach here
		assert(0);	
	}
};

void sys_sem_destory(struct TrapFrame *tf)
{
	tf->ebx += ((pcb_cur - pcb) * PROC_SIZE);
	int sem_cur = *(sem_t*) tf->ebx;
	semaphore[sem_cur].value = -1;
	semaphore[sem_cur].pid = -1;
	semaphore[sem_cur].used = FREE;
	tf->eax = 0;
};

void syscallHandle(struct TrapFrame *tf) 
{
	/* 实现系统调用*/
	//assert(0);
    switch (tf->eax) 
	{
        case SYS_exit:
            sys_exit(tf);
            break;
        case SYS_fork:
            sys_fork(tf);
            break;
        case SYS_write:
            sys_write(tf);
            break;
        case SYS_sleep:
            sys_sleep(tf);
			break;
		case SYS_SEM_init:
			sys_sem_init(tf);
			break;
		case SYS_SEM_post:
			sys_sem_post(tf);
			break;
		case SYS_SEM_wait:
			sys_sem_wait(tf);
			break;
		case SYS_SEM_destory:
			sys_sem_destory(tf);
            break;
        /*
          TODO: add more syscall
        */
	}
}


void GProtectFaultHandle(struct TrapFrame *tf)
{
	putChar(pcb_cur->pid);
	assert(0);
	return;
}
