.code16
.global start
start:
	cli 		#guan zhong duan
	inb $0x92,%al	#start A20
	orb $0x02,%al
	outb %al,$0x92
	data32 addr32 lgdt gdtDesc
	movl %cr0,%eax
	orb $0x01,%al
	movl %eax,%cr0
	data32 ljmp $0x08,$start32

.code32
start32:
	
	#initial DS,ES,FS,GS,SS,to,initialize ESP

	movw $0x10,%ax
	movw %ax,%ds	
	movw %ax,%es
	movw %ax,%ss
	movw %ax,%fs
	
	movw $(3<<3),%ax
	movw %ax,%gs

	movl $0,%ebp
	movl $(128<<20),%esp
	jmp bootMain #jmp fto bootMain defined boot.c
	
.p2align 2	#alignment four B 
gdt: 		# 8 bytes for each table entry, at least 1 entry
	.word 0,0 # empty entry
	.byte 0,0,0,0
	
	.word 0xffff,0 #kernel code des
	.byte 0,0x9a,0xcf,0
	
	.word 0xffff,0 #kernel data des
	.byte 0,0x92,0xcf,0
	
	.word 0xffff,0 #user  code seg
	.byte 0,0xfa,0xcf,0
	
	.word 0xffff,0	#user data seg
	.byte 0,0xf2,0xcf,0
	
	.word 0xffff,0	#TSS descriptor
	.byte 0,0x92,0xcf,0
	
	.word 0xffff,0x8000	#user vedio seg
	.byte 0x0b,0x92,0xcf,0
	
	
	#...

gdtDesc: # 6 bytes in total
	.word (gdtDesc - gdt -1)
	.long gdt
