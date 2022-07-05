objects = main.o

tiny: $(objects)
	gcc $(objects) -o tiny

main.o: main.c main.h types.h
	gcc -c main.c -std=c11

.PHONY: clean
clean:
	-rm tiny.exe
	-rm tiny_debug.exe
	-rm tiny.out
	-rm tiny_debug.out
	-rm main.o
	-rm main.obj

all: tiny
