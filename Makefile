-include config.mk

JSPPDIR := $(realpath $(CURDIR)/../jspp)

CFLAGS 	?= -O2
CFLAGS 	+= -I $(JSPPDIR)

all: libjssp.a

libjssp.a: jssp.o
	$(AR) rc $@ $^

jssp.o: jssp.c jssp.h
	$(CC) -c $(CFLAGS) $(filter %.c,$^) -o $@

clean:
	$(RM) *.o *.a
