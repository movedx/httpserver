# See https://www.gnu.org/software/make/manual/html_node/ for more information
CC	= gcc
CFLAGS	= -std=c99 -D_POSIX_C_SOURCE=200112L -g -Wall -Wextra -Wpedantic -Wbad-function-cast \
          -Wconversion -Wwrite-strings -Wstrict-prototypes
CFILES  = utils.c server.c
OFILES	= $(patsubst %.c, %.o, ${CFILES})
EXE	= $(patsubst %.c, %  , ${CFILES})
TESTCASES=$(patsubst %, tests/%, ${EXE})

.PHONY: all server clean realclean tests test-commex beautify ${TESTCASES}

server: $(CFILES)
	clear && $(CC) -o ./server $(CFLAGS) $(CFILES)

all:	${EXE}

clean:
	${RM} ${OFILES}

realclean: clean
	${RM} ${EXE}

test:	server
	./test02.sh localhost:8080

beautify:
	uncrustify -c ../uncrustify.cfg --replace ${CFILES}


