CFLAGS = -Wall -Wextra -Wpedantic -g -fsanitize=address -lcurl
SRC	= ./src
INCL = ./incl

wireman: wireman.o hostip.o root.o interface.o manager.o wireguard.o util.o
	gcc -o wireman ${SRC}/wireman.c ${SRC}/hostip.c ${SRC}/root.c ${SRC}/interface.c \
		${SRC}/manager.c ${SRC}/wireguard.c ${SRC}/util.c ${CFLAGS}

wireman.o: ${SRC}/wireman.c ${INCL}/root.h ${INCL}/interface.h ${INCL}/manager.h \
		${INCL}/util.h
	gcc -c ${SRC}/wireman.c

root.o: ${SRC}/root.c ${INCL}/root.h
	gcc -c ${SRC}/root.c

interface.o: ${SRC}/interface.c ${INCL}/hostip.h ${INCL}/interface.h ${INCL}/manager.h \
		${INCL}/wireguard.h
	gcc -c ${SRC}/interface.c

hostip.o: ${SRC}/hostip.c ${INCL}/hostip.h ${INCL}/util.h
	gcc -c ${SRC}/hostip.c

manager.o: ${SRC}/manager.c ${INCL}/manager.h ${INCL}/root.h ${INCL}/util.h
	gcc -c ${SRC}/manager.c

wireguard.o: ${SRC}/wireguard.c ${INCL}/wireguard.h
	gcc -c ${SRC}/wireguard.c

util.o: ${SRC}/util.c ${INCL}/util.h ${INCL}/manager.h
	gcc -c ${SRC}/util.c