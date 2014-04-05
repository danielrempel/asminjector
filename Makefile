all: injector

injector:
	g++ -Wno-pointer-arith -g -S -c injector.cpp -o build/injector.s
	gcc -c -g -Wa,-a,-ad build/injector.s -o build/injector-lst.o > injector.lst
	as --gstabs build/injector.s -o build/injector.o
	@#ld -o build/injector build/injector.o -lc
	gcc -v build/injector.o -o build/injector -lstdc++ 2> /dev/null
injector-non-suppressed:
	g++ -Wno-pointer-arith -g -S -c injector.cpp -o build/injector.s
	as --gstabs build/injector.s -o build/injector.o
	@#ld -o build/injector build/injector.o -lc
	gcc -v build/injector.o -o build/injector -lstdc++

testbin:
	gcc testbin.S -nostdlib -o build/testbin
genlst:
	gcc -c -g -Wa,-a,-ad testbin.S -o build/testbin.o > testbin.lst

.PHONY: all
