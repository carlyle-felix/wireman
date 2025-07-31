CFLAGS = -Wall -Wextra -Wpedantic -g -fsanitize=address -lcurl
SRC	= ./src
INCL = ./incl

wireman: wireman.o hostip.o root.o server.o manager.o wireguard.o
	gcc -o wireman ${SRC}/wireman.c ${SRC}/hostip.c ${SRC}/root.c ${SRC}/server.c \
		${SRC}/manager.c ${SRC}/wireguard.c ${CFLAGS}

wireman.o: ${SRC}/wireman.c ${INCL}/root.h ${INCL}/server.h
	gcc -c ${SRC}/wireman.c

root.o: ${SRC}/root.c ${INCL}/root.h
	gcc -c ${SRC}/root.c

server.o: ${SRC}/server.c ${INCL}/hostip.h ${INCL}/server.h ${INCL}/manager.h ${INCL}/wireguard.h
	gcc -c ${SRC}/server.c

hostip.o: ${SRC}/hostip.c ${INCL}/hostip.h
	gcc -c ${SRC}/hostip.c

manager.o: ${SRC}/manager.c ${INCL}/manager.h ${INCL}/root.h
	gcc -c ${SRC}/manager.c

wireguard.o: ${SRC}/wireguard.c ${INCL}/wireguard.h
	gcc -c ${SRC}/wireguard.c