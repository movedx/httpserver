# See https://www.gnu.org/software/make/manual/html_node/ for more information
CC	= gcc
CFLAGS	= -std=c99 -D_POSIX_C_SOURCE=200112L -g -Wall -Wextra -Wpedantic -Wbad-function-cast \
          -Wconversion -Wwrite-strings -Wstrict-prototypes -Wimplicit-function-declaration -D_GNU_SOURCE
CFILES  = utils.c server.c
OFILES	= $(patsubst %.c, %.o, ${CFILES})
MAIN = server.c
EXE	= $(patsubst %.c, %  , ${MAIN})
TESTCASES=$(patsubst %, tests/%, ${EXE})

.PHONY: all clean realclean test unittest beautify ${TESTCASES}

server: $(OFILES)
	$(CC) $(CFLAGS) -o server $(OFILES)

all: server unittest

clean:
	${RM} ${OFILES}

realclean: clean
	${RM} ${EXE}

unittest: all
	$(CC) $(CFLAGS) -o test test_stringlist.c stringlist.c

test:	all
	./test04.sh localhost:8080

beautify:
	uncrustify -c ./uncrustify.cfg --replace ${CFILES}

test1:	server
	./test01.sh localhost:8080

test2:	server
	./test02.sh localhost:8080

test4:	server
	./test04.sh localhost:8080


