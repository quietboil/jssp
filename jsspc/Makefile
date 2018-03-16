-include config.mk

JSPPDIR := $(realpath $(CURDIR)/../../jspp)

ifdef SystemDrive
	EXE := .exe
endif
JSSPC := jsspc$(EXE)

CFLAGS 	?= -O2
CFLAGS 	+= -I $(JSPPDIR)
LDFLAGS += -L $(JSPPDIR)

SRC := $(wildcard *.c)
OBJ := $(patsubst %.c,%.o,$(SRC))

all: $(JSSPC)

$(JSSPC): $(OBJ) $(JSPPDIR)/libjspp.a
	$(CC) $(OBJ) $(LDFLAGS) -ljspp -o $@

$(JSPPDIR)/libjspp.a:
	$(MAKE) -C $(JSPPDIR)

clean:
	$(RM) *.o $(JSSPC)
