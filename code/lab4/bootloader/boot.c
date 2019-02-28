#include "boot.h"
//#include<assert.h>
#define SECTSIZE 512

void bootMain(void) 
{
	/* 加载内核至内存，并跳转执行 */
	struct ELFHeader *elf;
	struct ProgramHeader *ph;
	struct ProgramHeader *eph;
	unsigned char*buf = (unsigned char*)0x200000;
	for(int i =0 ; i < 200 ; i++)//read 200 
	{
			readSect((void*)(buf + SECTSIZE * i), i + 1);
	}
	elf = (struct ELFHeader*)buf;
	/*Load each program segment*/
	ph = (struct ProgramHeader*)(buf + elf->phoff);
	eph = ph + elf->phnum;
	for( ; ph < eph ; ph++)
	{
		/*scan the program header table*/
		if(ph->type == 1)//load it
		{
				unsigned int m = ph->vaddr,n = ph->off;
				while(m < ph->vaddr + ph->filesz)
				{
					*(unsigned char*)m = *(unsigned char*)(buf+n);
					m++;
					n++;
				}
			// 	while(m < ph->vaddr + ph->memsz)
			//    {
			// 		*(unsigned char*)m = 0;
			// 		m++;
			// 	 	n++;
			// 	}
		}
	}
	//assert(0);
	void (*entry)(void);
	entry = (void*)(elf->entry);
	entry();
}

void waitDisk(void) { // waiting for disk
	while((inByte(0x1F7) & 0xC0) != 0x40);
}

void readSect(void *dst, int offset) { 
	// reading a sector of disk
	int i;
	waitDisk();
	outByte(0x1F2, 1);
	outByte(0x1F3, offset);
	outByte(0x1F4, offset >> 8);
	outByte(0x1F5, offset >> 16);
	outByte(0x1F6, (offset >> 24) | 0xE0);
	outByte(0x1F7, 0x20);

	waitDisk();
	for (i = 0; i < SECTSIZE / 4; i ++) {
		((int *)dst)[i] = inLong(0x1F0);
	}
}
