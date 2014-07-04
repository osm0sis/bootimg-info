CC = gcc
ifeq ($(windir),)
EXE =
RM = rm -f
else
EXE = .exe
RM = del
endif

CFLAGS = -ffunction-sections -O3
LDFLAGS = -Wl,--gc-sections

all: bootimg-info$(EXE)

bootimg-info$(EXE):bootimg-info.o
	$(CROSS_COMPILE)$(CC) -o $@ $^ $(LDFLAGS) -static -s

bootimg-info.o:bootimg-info.c
	$(CROSS_COMPILE)$(CC) -o $@ $(CFLAGS) -c $< -Werror

clean:
	$(RM) bootimg-info bootimg-info.o bootimg-info.exe
	$(RM) Makefile.~

