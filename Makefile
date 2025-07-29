CFLAGS = -Wall -Wextra -Wpedantic -g
SRC	= ./src
INCL = ./incl

wireman: wireman.o root.o server.o
	gcc -o wireman ${SRC}/wireman.c ${SRC}/root.c ${CFLAGS} ${SRC}/server.c

wireman.o: ${SRC}/wireman.c ${INCL}/root.h ${INCL}/server.h
	gcc -c ${SRC}/wireman.c

root.o: ${SRC}/root.c ${INCL}/root.h
	gcc -c ${SRC}/root.c

server.o: ${SRC}/server.c ${INCL}/server.h
	gcc -c ${SRC}/server.c