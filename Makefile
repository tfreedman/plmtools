BINDIR=/usr/bin
CONFDIR=/etc
WARNFLAGS=-Wall  -W -Wshadow
CFLAGS?=-Os -g ${WARNFLAGS}
CC?=gcc

all: plmcat plmtty plmsend

plmcat: plmcat.o plm.o
	$(CC) ${CFLAGS} -o plmcat plmcat.o plm.o

plmtty: plmtty.o plm.o
	$(CC) ${CFLAGS} -o plmtty plmtty.o plm.o

plmsend: plmsend.o plm.o
	$(CC) ${CFLAGS} -o plmsend plmsend.o plm.o

plm.o: plm.h plm.c
	$(CC) ${CFLAGS} -c plm.c

plmcat.o: plm.h plmcat.c
	$(CC) ${CFLAGS} -c plmcat.c

plmtty.o: plm.h plmtty.c
	$(CC) ${CFLAGS} -c plmtty.c

plmsend.o: plm.h plmsend.c
	$(CC) ${CFLAGS} -c plmsend.c

install: plmcat plmtty plmsend
	@if [ ! -d ${DESTDIR}${BINDIR} ]; \
		then \
			-mkdir -p ${DESTDIR}${BINDIR};\
	fi
	@if [ ! -d ${DESTDIR}${CONFDIR} ]; \
		then \
			-mkdir -p ${DESTDIR}${CONFDIR};\
	fi
	cp -f plmcat plmtty plmsend insteon x10toinst ${DESTDIR}${BINDIR}
	cp -f insteon.conf.example x10toinst.conf.example ${DESTDIR}${CONFDIR}

uninstall:
	-rm -f ${DESTDIR}${BINDIR}/plmcat
	-rm -f ${DESTDIR}${BINDIR}/plmtty
	-rm -f ${DESTDIR}${BINDIR}/plmsend
	-rm -f ${DESTDIR}${BINDIR}/insteon
	-rm -f ${DESTDIR}${BINDIR}/x10toinst
	-rm -f ${DESTDIR}${ETCDIR}/insteon.conf.example
	-rm -f ${DESTDIR}${ETCDIR}/x10toinst.conf.example

clean:
	-rm -f *~ plmcat plmtty plmsend *.o
