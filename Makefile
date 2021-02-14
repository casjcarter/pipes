LIBS=-lncurses

build: pipes.c
	gcc pipes.c -o pipes $(LIBS) -Wall -Wpedantic
