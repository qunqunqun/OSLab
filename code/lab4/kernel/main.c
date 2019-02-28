#include "common.h"
#include "x86.h"
#include "device.h"

void kEntry(void) {

	initSerial();// initialize serial port
	initIdt(); // initialize idt
	initIntr(); // intialize 8259a
	initTimer(); // intialize 8253 timer
	initSeg(); // initialize gdt, tss
	initpcb(); // initialize process table
	initvga(); // clear screen
	initsem(); // initialize the semaphore
	loadUMain(); // load user program, enter user space

	while(1);
	assert(0);
}
