
CC     ?= cc

ifeq ($(shell uname -o 2> /dev/null),Cygwin)
BIN     = clib-download.exe
LDFLAGS = -lcurl
CP      = cp -f
RM      = rm -f
MKDIR_P = mkdir -p
else ifeq ($(OS),Windows_NT)
BIN     = clib-download.exe
LDFLAGS = -lcurldll
CP      = copy /Y
RM      = del /Q /S
MKDIR_P = mkdir
else
BIN     = clib-download
LDFLAGS = -lcurl
CP      = cp -f
RM      = rm -f
MKDIR_P = mkdir -p
endif

SRC  = $(wildcard src/*.c)
DEPS = $(wildcard deps/*/*.c)
OBJS = $(SRC:.c=.o) $(DEPS:.c=.o)

CFLAGS  = -std=c99 -Isrc -Ideps -Wall -Wno-unused-function -U__STRICT_ANSI__

all: $(BIN)

$(BIN): $(SRC) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $< -c -o $@ $(CFLAGS)

clean:
	$(foreach c, $(BINS), $(RM) $(c);)
	$(RM) $(OBJS)

.PHONY: test all clean
