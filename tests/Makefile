-include config.mk

JSSPDIR = $(realpath $(dir $(CURDIR)))
JSPPDIR = $(realpath $(dir $(JSSPDIR))/jspp)

CFLAGS 	?= -g -O2 -I $(CURDIR) -I $(JSSPDIR) -I $(JSPPDIR)
LDFLAGS ?= -L $(JSSPDIR) -L $(JSPPDIR)

TESTS = $(filter-out test.c tests.c,$(wildcard *.c))
TESTOBJS = $(patsubst %.c,%.o,$(TESTS))
TESTHDRS = $(patsubst %.c,%.h,$(TESTS))

all: tests $(JSSPDIR)/libjssp.a $(JSPPDIR)/libjspp.a

%.o: %.c %.h
	$(CC) $(CFLAGS) $(filter %.c,$^) -c -o $@

$(JSSPDIR)/libjssp.a:
	$(MAKE) -C $(JSSPDIR)

$(JSPPDIR)/libjspp.a:
	$(MAKE) -C $(JSPPDIR)

tests.o: tests.c

tests: tests.o test.o $(TESTOBJS) $(TESTHDRS) $(JSSPDIR)/libjssp.a $(JSPPDIR)/libjspp.a
	$(CC) $(LDFLAGS) $(filter %.c %.o,$^) -ljssp -ljspp -o $@

clean:
	$(RM) *.o tests *.exe
