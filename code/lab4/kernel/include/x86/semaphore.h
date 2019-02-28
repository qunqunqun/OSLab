#ifndef __X86_SEMAPHORE_H__
#define __X86_SEMAPHORE_H__

#define SEM_MAX_NUM 200
#define USED 1
#define FREE 0

typedef int sem_t;

struct Semaphore
{
    int used;      //shi yong
    int value;      //xin hao liang 
    int pid;        //jin cheng pid
};

struct Semaphore semaphore[SEM_MAX_NUM];

void initsem();

int find_new_sem();

#endif