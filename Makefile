CC=gcc
EXEC=bf_interpret


all:
	${CC} -o ${EXEC} main.c -lncurses