CC = gcc

CFLAGS = -g -Wall


all:
	gcc -g -Wall problem1/problem1.c -o problem1/problem1
	gcc -g -Wall problem2/problem2.c -o problem2/problem2


clean:
	$(RM) problem1/problem1 *.o *~
	$(RM) problem2/problem2 *.o *~
