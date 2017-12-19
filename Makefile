CC = gcc
OBJECTS = shell.o
CFLAGS = -Wall -pedantic -I.

all: main clean

main: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o shell

.PHONY: clean

clean:
	rm *.o
