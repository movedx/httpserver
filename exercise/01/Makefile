# See https://www.gnu.org/software/make/manual/html_node/ for more information
CC	= gcc
CFLAGS	= -std=c99 -D_POSIX_C_SOURCE=200112L -g -Wall -Wextra -Wpedantic -Wbad-function-cast \
          -Wconversion -Wwrite-strings -Wstrict-prototypes
CFILES  = $(wildcard *.c)
OFILES	= $(patsubst %.c, %.o, ${CFILES})
EXE	= $(patsubst %.c, %  , ${CFILES})
TESTCASES=$(patsubst %, tests/%, ${EXE})

.PHONY: all clean realclean tests test-commex beautify ${TESTCASES}

all:	${EXE}

clean:
	${RM} ${OFILES}

realclean: clean
	${RM} ${EXE}

test:	all
	./test01.sh localhost:8080

beautify:
	uncrustify -c ../uncrustify.cfg --replace ${CFILES}

