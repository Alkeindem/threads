main: main.c utils.o
	gcc -o main main.c utils.o

utils.o: utils.c utils.h
	gcc -c utils.c
