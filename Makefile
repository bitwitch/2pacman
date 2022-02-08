CFLAGS := -O2 -I./include
SOURCES := ./source/*.c

.PHONY: 2pacman

2pacman:
	gcc $(CFLAGS) -o 2pacman $(SOURCES) 

