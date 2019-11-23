
# CYGWIN
UNIX_FOR_WINDOWS = MINGW
ifeq ($(UNIX_FOR_WINDOWS), MINGW)
CC = gcc
LDLIBS_DLL = -lmingw32
_PATH = /mingw/lib
RM = del -f
else
PATH = /usr/local/lib
LDLIBS_DLL = -lcomdlg32
endif

CFLAGS = -O3 -g -Wall -DEBUG 
CCCOM=$(CC) $(CFLAGS) -std=c11
LDLIBS = $(LDLIBS_DLL) -lSDLmain -lSDL 
LDFLAGS = -L$(_PATH) $(LDLIBS) 
SRC = $(wildcard *.c)
OBJS = $(SRC:.c=.o)
EXEC = breaker 


all: $(EXEC)

$(EXEC): $(OBJS)
	$(CCCOM) -o $@ $^ $(LDFLAGS) 

.PHONY: clean 

clean:
	$(RM) *.o *~ *.stackdump *.bak *.makefile .exe

