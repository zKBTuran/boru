CFLAGS = -Wall -Wextra -Werror -Wl,-z,now
CFLAGS_RELEASE = ${CFLAGS} -O2 -s -D_FORTIFY_SOURCE=2
CFLAGS_DEBUG = ${CFLAGS} -O0 -g -fsanitize=undefined
LIBS = -lcrypt
CC = gcc

all: boru.c
	${CC} ${CFLAGS_RELEASE} boru.c -o boru ${LIBS}

debug: boru.c
	${CC} ${CFLAGS_DEBUG} boru.c -o boru ${LIBS}

install: boru
	cp boru /usr/bin/boru
	chown root:root /usr/bin/boru
	chmod 755 /usr/bin/boru
	chmod u+s /usr/bin/boru
	cp boru_sample.conf /etc/boru.conf
	chmod 600 /etc/boru.conf

uninstall:
	rm /usr/bin/boru
	rm /etc/boru.conf

clean:
	rm boru
