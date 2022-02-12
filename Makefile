CFLAGS := -Wall -std=gnu11 -O2 -I./include $(shell sdl2-config --cflags --libs)
LFLAGS := -lm 
SOURCES := $(wildcard ./source/*.c)

.PHONY: 2pacman

2pacman: 
	gcc -o 2pacman $(SOURCES) $(LFLAGS) $(CFLAGS)

