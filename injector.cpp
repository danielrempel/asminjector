#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>

void* mapWritableMemory(int length, void** deleteptr);
void generateCode();
void writeReturnSequence(void* mem);

void beeper() {
	printf("beep-beep\n");
}

/*
58			popq %rax
FFD0		call %rax

58			popq %rax
FFE0		jmp %rax
*/
void writeReturnSequence(void* mem) {
	char* data = (char*) mem;
	data[0] = 0x58; // popq %rax
	data[1] = 0xff; // call %rax
	data[2] = 0xd0; //
	
	data[3] = 0x58; // popq %rax
	data[4] = 0xff; // jmp %rax
	data[5] = 0xe0; //
}

void generateCode() {
	
	void* deleteptr;
	void* memptr = mapWritableMemory(1000, &deleteptr);
	if(memptr == NULL) {
		printf("generateCode(): mapWritableMemory() failed\n");
		return;
	}
	
	writeReturnSequence(memptr);
	asm( "pushq $back\n" );
	asm( "pushq %0\n"
		:
		:"r"(&beeper)
		:
		);
	asm( "jmp %0\n"
		:
		:"r"(memptr)
		:
		);
	asm( "back: nop\n" );
	delete (deleteptr);
}

// NULL or void*
void* mapWritableMemory(int length, void** deleteptr) {
	
	void* memptr = malloc(4096*2 + length);
	if(memptr == NULL) {
		printf("mapWritableMemory(): Meow! Got no mem!\n");
		return NULL;
	}
	
	*deleteptr = memptr;
	
	unsigned long shift = -1;
	int result;
	do {
		++shift;
	}
	while ( (result = mprotect ( (void*) ( memptr+shift ), length, PROT_READ|PROT_WRITE|PROT_EXEC ) ) == -1);
	if(result == 0) {
		return (void*)(memptr+shift);
	}
	else {
		free(memptr);
		*deleteptr = 0;
		return NULL;
	}
	
}

int main() {
	generateCode();
}
