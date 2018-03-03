-include config.mk

JSPPDIR = $(realpath $(dir $(CURDIR))/jspp)

CFLAGS 	?= -O2 -I $(CURDIR) -I $(JSPPDIR)
LDFLAGS ?= -L $(CURDIR) -L $(JSPPDIR)

all: libjssp.a

libjssp.a: jssp.o
	$(AR) rc $@ $^

jssp.o: jssp.c jssp.h
	$(CC) -c $(CFLAGS) $(filter %.c,$^) -o $@

clean:
	$(RM) *.o *.a
