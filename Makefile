CC=gcc
CFLAGS=-Wall -Werror -lpthread -I.
DEPS=aubatch_scheduler.h aubatch_utilities.h aubatch.h
OBJ=aubatch_scheduler.o aubatch_utilities.o aubatch.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

aubatch: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
