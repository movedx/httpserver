# See https://www.gnu.org/software/make/manual/html_node/ for more information
CC	= gcc
CFLAGS	= -std=c99 -D_POSIX_C_SOURCE=200112L -g -Wall -Wextra -Wpedantic -Wbad-function-cast \
          -Wconversion -Wwrite-strings -Wstrict-prototypes -Wimplicit-function-declaration -D_GNU_SOURCE -pthread
SRC_PATH = ../webserver/src
CFILES  = ${SRC_PATH}/server_utils.c ${SRC_PATH}/server.c ${SRC_PATH}/file_utils.c ${SRC_PATH}/string_utils.c ${SRC_PATH}/stringlist.c ${SRC_PATH}/cache_utils.c
# OFILES	= $(patsubst %.c, %.o, ${CFILES})
MAIN = ../webserver/src/server.c
EXE	= $(patsubst %.c, %  , ${MAIN})
TESTCASES=$(patsubst %, tests/%, ${EXE})

.PHONY: all clean realclean test unittest beautify ${TESTCASES}

server: $(CFILES)
	$(CC) $(CFLAGS) -o server $(CFILES) -lm -ltls

all: server unittest

clean:
	${RM} ${OFILES}

realclean: clean
	${RM} ${EXE}

unittest: all
	$(CC) $(CFLAGS) -o test ../webserver/src/test_stringlist.c ../webserver/src/stringlist.c

test:	server
	./test08.sh localhost:8080

beautify:
	uncrustify -c ./uncrustify.cfg --replace ${CFILES}

test1:	server
	./test01.sh localhost:8080

test2:	server
	./test02.sh localhost:8080

test4:	server
	./test04.sh localhost:8080

test6:	server
	./test06.sh localhost:8080

test8:	server
	./test08.sh localhost:8080


