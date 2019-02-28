#include "x86.h"
#include "device.h"

void initsem(void)
{
    for(int i = 0 ; i < SEM_MAX_NUM ; i++)
    {
        semaphore[i].used = FREE;
        semaphore[i].value = 0;
        semaphore[i].pid = 0;
    }
}

int find_new_sem()
{
    int find = -1;
    for(int i = 0 ; i < SEM_MAX_NUM ; i++)
    {
        if(semaphore[i].used == FREE)
        {
            semaphore[i].used = USED;
            find = i;
            break;
        }
    }
    return find;
}
