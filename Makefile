-include config.mk

JSPPDIR := $(realpath $(CURDIR)/../jspp)

ifdef SystemDrive
	EXE := .exe
endif
JSSPC := jsspc/jsspc$(EXE)

CFLAGS 	+= -I $(JSPPDIR) -O2
LDFLAGS ?= -L $(JSPPDIR)

all: libjssp.a $(JSSPC)

libjssp.a: jssp.o
	$(AR) rc $@ $^

jssp.o: jssp.c jssp.h
	$(CC) -c $(CFLAGS) $(filter %.c,$^) -o $@

$(JSSPC):
	$(MAKE) -C jsspc

clean:
	$(MAKE) -C jsspc clean
	$(MAKE) -C tests clean
	$(RM) *.o *.a
