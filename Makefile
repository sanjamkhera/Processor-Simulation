LDLIBS = -lpthread 
CFLAGS = -Wall -Wextra -Wpedantic -Werror -g
CC = clang

.PHONY: all clean

all: Simulator 

Simulator: Simulator.o MyQueue.o

clean:
	rm -f Simulator Simulator.o MyQueue.o 
