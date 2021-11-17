ifeq ($(CC),cc)
CC = gcc
endif
ifeq ($(windir),)
EXE =
RM = rm -f
else
EXE = .exe
RM = del
endif

CFLAGS = -ffunction-sections -O3

ifneq (,$(findstring darwin,$(CROSS_COMPILE)))
    UNAME_S := Darwin
else
    UNAME_S := $(shell uname -s)
endif
ifeq ($(UNAME_S),Darwin)
    LDFLAGS += -Wl,-dead_strip
else
    LDFLAGS += -Wl,--gc-sections -s
endif

all: bootimg-info$(EXE)

static:
	$(MAKE) LDFLAGS="$(LDFLAGS) -static"

bootimg-info$(EXE):bootimg-info.o
	$(CROSS_COMPILE)$(CC) -o $@ $^ $(LDFLAGS)

bootimg-info.o:bootimg-info.c
	$(CROSS_COMPILE)$(CC) -o $@ $(CFLAGS) -c $< -Werror

install:
	install -m 755 bootimg-info $(PREFIX)/bin

clean:
	$(RM) bootimg-info
	$(RM) *.a *.~ *.exe *.o

