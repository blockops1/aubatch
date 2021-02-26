IDIR = ../include
CC=gcc
CFLAGS=-DDEBUG -g -Wall -Werror -lpthread -I$(IDIR)

ODIR=obj
LDIR =../lib

LIBS=-lm

_DEPS = aubatch.h aubatch_scheduler.h aubatch_utilities.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = aubatch.o aubatch_scheduler.o aubatch_utilities.o 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

aubatch: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
