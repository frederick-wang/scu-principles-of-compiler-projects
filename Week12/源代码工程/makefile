objects = main.o

cparser: $(objects)
	gcc $(objects) -o cparser

main.o: main.c main.h types.h
	gcc -c main.c -std=c11

.PHONY: clean
clean:
	-rm cparser.exe
	-rm cparser_debug.exe
	-rm cparser.out
	-rm cparser_debug.out
	-rm main.o
	-rm main.obj

all: cparser
