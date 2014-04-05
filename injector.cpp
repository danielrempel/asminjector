/*
Copyright (c) 2014 Daniel Rempel <danil.rempel@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

/*
	All information in README.
*/

#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>

/*
	mapWritableMemory():
	Method creates about 8192 + length bytes memory, changes access rights to +rwx on length of them.
		Arguments:
			length - count bytes to prepare
			deleteptr - pointer to voidptr, receives address of the whole mapped memory area (not only executable area) or NULL on error
		Return value:
			address of the first of length bytes of executable memory or NULL on error
		
		Mapped memory must be free()-d by hand, using deleteptr, not return value!
*/
void* mapWritableMemory(int length, void** deleteptr);

/*
	generateCode():
	calls mapWritableMemory, calls writeCode, prepares stack, executes the code
*/
void generateCode();

/*
	writeCode():
	writes two sequences of opcodes to given address:
		popq %rax
		call %rax
		popq %rax
		jmp %rax
	
	the opcodes expect to receive address of functions and return point from stack
*/
void writeCode(void* mem);

/*
	Method which is executed to show that the code works
*/
void beeper() {
	printf("beep-beep\n");
}

/*
	Opcodes which I used
58			popq %rax
FFD0		call %rax

58			popq %rax
FFE0		jmp %rax
*/
void writeCode(void* mem) {
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
	
	writeCode(memptr);
	// push return address to stack
	// $ is mandatory, because otherwise I don't know what happens. Segfault happens
	asm( "pushq $back\n" );
	// push the address of demonstrational function
	asm( "pushq %0\n"
		:
		:"r"(&beeper)
		:
		);
	// jump to prepared code
	asm( "jmp %0\n"
		:
		:"r"(memptr)
		:
		);
	// return anchor
	asm( "back: nop\n" );
	// clean everything
	free(deleteptr);
}

void* mapWritableMemory(int length, void** deleteptr) {
	
	// firstly, allocate some memory
	// we allocate 8192 bytes more
	// Not sure if we need more than 4096, because my memory uses 4 kb pages
	// but let's leave it like that
	void* memptr = malloc(4096*2 + length);
	if(memptr == NULL) {
		printf("mapWritableMemory(): Got no mem!\n");
		return NULL;
	}
	
	// We've got memory, lets give the caller ability to free() it afterwards
	*deleteptr = memptr;
	
	unsigned long shift = -1;
	int result;
	do {
		++shift;
		// maybe we may jump by 16 bytes, but we won't die if we'll do 4k iterations instead of 255
	}
	// try to set rights on memory until we receive positive result
	while ( (result = mprotect ( (void*) ( memptr+shift ), length, PROT_READ|PROT_WRITE|PROT_EXEC ) ) == -1);
	
	if(result == 0) {
		// success
		return (void*)(memptr+shift);
	}
	else {
		// fail: don't forget to turn off the lights
		free(memptr);
		*deleteptr = 0;
		return NULL;
	}
	
}

int main() {
	generateCode();
}
