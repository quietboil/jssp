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

JSONSPECS  := $(patsubst %.json,%.c,$(wildcard *.json))
TESTSPECS  := $(JSONSPECS) hello_world.c
TESTCASES  := $(wildcard test_*.c)
TESTOBJS   := $(patsubst %.c,%.o,$(TESTSPECS) $(TESTCASES))
TESTRUNNER := tests$(EXE)

all: $(TESTRUNNER)

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
	$(RM) *.o $(JSONSPECS) $(patsubst %.c,%.h,$(JSONSPECS)) $(TESTRUNNER)

.SECONDARY: $(JSSPCTESTS)
