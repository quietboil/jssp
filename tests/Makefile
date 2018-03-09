-include config.mk

JSSPDIR  := $(realpath $(dir $(CURDIR)))
JSPPDIR  := $(realpath $(dir $(JSSPDIR))/jspp)
JSSPCDIR := $(realpath $(JSSPDIR)/jsspc)
ifdef SystemDrive
	EXE := .exe
endif
JSSPC    := $(JSSPCDIR)/jsspc$(EXE)

CFLAGS 	+= -I $(JSSPDIR) -I $(JSPPDIR) -g
LDFLAGS ?= -L $(JSSPDIR) -L $(JSPPDIR)

TESTSPECS  := $(wildcard *.json)
JSSPCTESTS := $(patsubst %.json,%.c,$(TESTSPECS))
TESTS       = $(filter-out test.c tests.c,$(wildcard *.c)) $(JSSPCTESTS)
TESTOBJS    = $(patsubst %.c,%.o,$(TESTS))
TESTHDRS    = $(patsubst %.c,%.h,$(TESTS))
TESTRUNNER := tests$(EXE)

all: $(TESTRUNNER) $(JSSPDIR)/libjssp.a $(JSPPDIR)/libjspp.a

$(TESTRUNNER): $(TESTOBJS) test.o tests.o $(JSSPDIR)/libjssp.a $(JSPPDIR)/libjspp.a
	$(CC) $(LDFLAGS) $(filter %.o,$^) -ljssp -ljspp -o $@

%.c: $(JSSPC) %.json
	$(JSSPC) $(filter %.json,$^)

$(JSSPDIR)/libjssp.a:
	$(MAKE) -C $(JSSPDIR)

$(JSPPDIR)/libjspp.a:
	$(MAKE) -C $(JSPPDIR)

$(JSSPC):
	$(MAKE) -C $(JSSPCDIR)

clean:
	$(RM) *.o $(JSSPCTESTS) $(patsubst %.c,%.h,$(JSSPCTESTS)) $(TESTRUNNER)

.SECONDARY: $(JSSPCTESTS)
