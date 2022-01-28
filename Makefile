CC=gcc

ifeq ($(BUILD),debug)
# Debug mode flags
CFLAGS = -Wall -Werror -pedantic -O0 -g -include scriptingMalloc.h
LFLAGS = -lm
else
# Release mode
CFLAGS = -Wall -Werror -pedantic -O2 -DNDEBUG
LFLAGS = -s -fno-exceptions -lm
endif

ifeq ($(MATH),int)
CFLAGS := $(CFLAGS) -DFIZ_INTEGER_EXPR
endif

all: fiz docs

debug:
	make "BUILD=debug"

fiz: shell.o libfiz.a
	gcc -o $@ $^ $(LFLAGS) -lstdc++

shell.o: shell.c fiz.h
	gcc -o $@ $(CFLAGS) -c $<
	
libfiz.a: fiz.o hash.o expr.o auxfuns.o scriptingMalloc.o
	ar rs $@ $^

scriptingMalloc.o:
	g++ -Wall -Werror -fno-exceptions -O0 -g -l lstdc++ scriptingMalloc.cpp -c -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

fiz.o: fiz.h hash.h

auxfuns.o: fiz.h

hash.o: hash.c hash.h

expr.o: 

docs: doc.html

doc.html: fiz.h doc.awk
	awk -f doc.awk fiz.h > $@

clean:
	-rm -rf fiz fiz.exe
	-rm -rf *.o libfiz.a
	-rm -rf doc.html *~
