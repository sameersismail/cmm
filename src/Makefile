CC      := gcc
CFLAGS  := -std=c11 -Wall -pedantic -Wuninitialized
DEBUG   := -g

OBJECTS  := lexer.o ast.o parser.o symbol.o analyser.o cgen.o shared.o
MAIN_SRC := cmm.c

.DEFAULT: all
.PHONY: clean

all: cmm 

debug: CFLAGS += $(DEBUG)
debug: all

cmm: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(MAIN_SRC) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $*.c -o $*.o

clean: 
	rm -rf *.dSYM; rm *.o; rm cmm; rm *.cmm 
