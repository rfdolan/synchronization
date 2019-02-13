CC = gcc

CFLAGS = -g -Wall


all:
	gcc -g -Wall -pthread problem1/problem1.c -o problem1/problem1 -lm
	gcc -g -Wall -pthread problem2/problem2.c -o problem2/problem2 -lm


clean:
	$(RM) problem1/problem1 *.o *~
	$(RM) problem2/problem2 *.o *~
