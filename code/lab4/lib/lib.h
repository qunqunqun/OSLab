#ifndef __lib_h__
#define __lib_h__

typedef int   sem_t;

//print
void printf(const char *format,...);
int fork();
void sleep(int time_sleep);
void exit();
//xin hao liang
int sem_init(sem_t *sem, unsigned int value);
int sem_post(sem_t *sem);
int sem_wait(sem_t *sem);
int sem_destroy(sem_t *sem);

#endif

