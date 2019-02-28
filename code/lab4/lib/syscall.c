#include "lib.h"
#include "types.h"
#include <stdarg.h>
#define SYS_exit 1
#define SYS_fork 2
#define SYS_write 4
//undefined syscall
#define SYS_SEM_init 250
#define SYS_SEM_post 251
#define SYS_SEM_wait 252
#define SYS_SEM_destory 253
#define SYS_sleep 300 
/*
 * io lib here
 * 库函数写在这
 */
int32_t syscall(int num, uint32_t a1,uint32_t a2,
		uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t ret = 0;
	//assert(ret);
	/* 内嵌汇编 保存 num, a1, a2, a3, a4, a5 至通用寄存器*/

	asm volatile("int $0x80": "=a"(ret): "a"(num),"b"(a1),"c"(a2),"d"(a3),"S"(a4),"D"(a5));	
	return ret;
}

void printc(char out);
void printd(int out);
void prints(const char* str);
void printx(unsigned int out);

void printf(const char *str, ...)
{
	char task;
	va_list prt;
	va_start(prt,str);
	if(str == 0)
	{
		return;
	}
	while(*str != '\0')
	{
		if(*str == '%')//read the type of task
		{
			str++;
			task = *str;
			switch(task)
			{
				case 'c':printc(va_arg(prt,int));break;
				case 's':prints(va_arg(prt,char*));break;
				case 'd':printd(va_arg(prt,int));break;
				case 'x':printx(va_arg(prt,int));break;
			}
			str++;
		}
		else//print it out one by one
		{
			printc(*str);
			str++;
		}
	}
	va_end(prt);
}

void printc(char out)
{
	syscall(SYS_write, 1 ,(uint32_t)&out, 1 ,0, 0);
};

void prints(const char* str)
{
	int size = 0;
	while(str[size] != '\0')
	{
		size++;
	}
	syscall(SYS_write, 1 , (uint32_t)str, size,0 ,0);
};

void printd(int out)
{
	char buf[100];
	int size = 0;
	if(out == 0x80000000)
	{
		prints("-2147483648");
		return;
	}
	if(out == 0)
	{
		prints("0");
		return;
	}
	if(out<0)
	{
		prints("-");
		out = -out;
	}
	while(out)
	{
		buf[size] = out%10+'0';
		size++;
		out = out/10;
	}
	for(int i=0,j= size -1;i<j;i++,j--)//fanzhuan
	{
		char temp = buf[j];
		buf[j] = buf[i];
		buf[i] = temp;
	}
	buf[size] ='\0';
	prints(buf);
};

void printx(unsigned int out)
{
	char buf[100];
	int size = 0;
	if(out == 0 )
	{
		prints("0");
		return;
	}
	while(out)
	{
		if(out %16 >=0)
		{
			buf[size] = out % 16 - 10 + 'a';
			size ++;
		}
		else
		{
			buf[size] = out % 16 + '0';
			size ++;
		}
		out = out / 16;
	}
	for(int i = 0,j = size - 1; i < j; i++ , j--)
	{
		char temp = buf[j];
		buf[j] = buf[i];
		buf[i] = temp;
	}
	buf[size] ='\0';
	prints(buf);
};

int fork() 
{
	// TODO:
	return syscall(SYS_fork, 1, 1, 1, 0, 0);
}

void exit() 
{
	// TODO:
	syscall(SYS_exit, 1, 1, 1, 0, 0);
}

void sleep(int time_sleep) 
{
	// TODO:
	syscall(SYS_sleep, time_sleep, 1, 1, 0, 0);
}


int sem_init(sem_t *sem, uint32_t value)
{
	return syscall(SYS_SEM_init, (uint32_t)sem, value, 1, 0, 0);
}

int sem_post(sem_t *sem)
{
	return syscall(SYS_SEM_post, (uint32_t)sem, 1, 1, 0, 0);
}

int sem_wait(sem_t *sem)
{
	return syscall(SYS_SEM_wait, (uint32_t)sem, 1, 1, 0, 0);
}

int sem_destroy(sem_t *sem)
{
	return syscall(SYS_SEM_destory, (uint32_t)sem, 1, 1, 0, 0);
}