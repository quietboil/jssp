-include config.mk

JSPPDIR := $(realpath deps/jspp)

CFLAGS  ?= -O2
CFLAGS  += -I $(JSPPDIR)

all: libjssp.a $(JSPPDIR)/libjspp.a

libjssp.a: jssp.o
	$(AR) rc $@ $^

jssp.o: jssp.c jssp.h
	$(CC) -c $(CFLAGS) $(filter %.c,$^) -o $@

$(JSPPDIR)/libjspp.a:
	$(MAKE) -C $(JSPPDIR)

clean:
	$(RM) *.o *.a
