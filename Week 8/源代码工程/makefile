objects = main.o

tiny: $(objects)
	gcc $(objects) -o cm

main.o: main.c main.h types.h
	gcc -c main.c -std=c11

.PHONY: clean
clean:
	-rm cm.exe
	-rm cm_debug.exe
	-rm cm.out
	-rm cm_debug.out
	-rm main.o
	-rm main.obj

all: cm
