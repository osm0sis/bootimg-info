CC = gcc
AR = ar rcv
ifeq ($(windir),)
EXE =
RM = rm -f
else
EXE = .exe
RM = del
endif

all: bootimg-info$(EXE)

bootimg-info$(EXE):bootimg-info.o
	$(CROSS_COMPILE)$(CC) -o $@ $^

bootimg-info.o:bootimg-info.c
	$(CROSS_COMPILE)$(CC) -o $@ -c $< -Werror

clean:
	$(RM) bootimg-info bootimg-info.o bootimg-info.exe
	$(RM) Makefile.~

