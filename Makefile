CC=gcc

all: interpreter compiler

interpreter: interpreter.c
	${CC} -o interpret interpret.c -lncurses

compiler: compiler.c
	${CC} -o compiler compiler.c
