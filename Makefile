
CFILES := *.c

.PHONY: 2pacman

2pacman:
	gcc -O2 -o 2pacman $(CFILES) 

