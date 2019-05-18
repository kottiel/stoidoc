# Compiler for the default rule to use.
CC = gcc

# extra options we want the default compile rule to use.
CFLAGS = -Wall -Wextra -std=c99 -g -O3

# The libraries to link with.
# LDLIBS = -lm

# Our main executable depends on idoc.o (implicit) and label.o
idoc: idoc.o label.o

# Our objects depend on their own source files (implicit),
# and the headers listed below.
idoc.o: label.h
label.o: label.h

clean:
	rm -f idoc.o label.o
	rm -f idoc
	rm -f stderr.txt stdout.txt