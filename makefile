main: main.c utils.o img.o
	gcc -pthread -o main main.c utils.o img.o

utils.o: utils.c utils.h
	gcc -c utils.c

img.o: img.c img.h
	gcc -c img.c
