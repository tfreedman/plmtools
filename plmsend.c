/*
    plmsend - send data to the insteon PLM and display a reply or timeout
    Copyright (C) 2008  Matthew Randolph
    Please see the file COPYING for license information.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <errno.h>
#include <signal.h>
#include <string.h>
#include "plm.h"

extern int baudrate;
extern int hwflowcontrol;
extern int translation;
extern int linebits;
extern int lineparity;
extern int linestopbits;
extern int lineflush;
extern char *ttylinename;

char *options = "d:e:t:vh";

int main(int argc, char **argv) {
	int ttyfd,nfds,ready,ret;
	char c,*str,*cp,buff[128];
	struct termios newtio;
	fd_set readfds;
	int msec = 500;
	int verbose = FALSE;
	int wrote = FALSE;
	int toplm = FALSE;
	int helpwanted = FALSE;
	int strset = FALSE;
	int plmcatpid = -1;
	FILE *popenfile;

	struct timespec sleeptime;

	// Parse command line using getopt() from libc
while (-1 != (ret = getopt(argc,argv,options))) {
		switch (ret) {
		case 'd':
			ttylinename = optarg;
			break;
		case 'e':
			str = optarg;
			strset = TRUE;
			break;
		case 't':
			msec = atoi(optarg);
			break;
		case 'v':
			verbose = TRUE;
			break;
		case 'h':
			helpwanted = TRUE;
			break;
		}
	}

	if (argc == 1)
		helpwanted = TRUE;

	if (helpwanted) {
		fprintf(stdout,
"USAGE: %s [OPTION]... hexstring\n"
"\n"
"\tOPTIONS:\n"
"\n"
"\t-h\t\tdisplay this help menu\n"
"\t-d\t\tPLM tty device name (/dev/ttyUSB0)\n"
"\t-t\t\tmax time to wait for a response in millisecs (500)\n"
"\t-v\t\tverbose output (PLM response is sent to STDERR)\n"
"\t-e\t\tspecify data to send as a hex string\n"
"\n"
"Hex strings are case insensitive, but must not begin with 0x or 0X.\n"
"Only the last hex string specified will be used.\n",
argv[0]);
		exit(0);
	}

	if (!strset)
		str = (char *)(argv[argc-1]);

	for (cp = str; *cp; cp++)
		if (!ishex(*cp)) {
			fprintf(stderr,"%s: ERROR: Invalid command string\n",argv[0]);
			exit(1);
		}

	if (0 != strncmp(str,"02",2)) {
		fprintf(stderr,"%s: ERROR: Invalid command string\n",argv[0]);
		exit(1);
	}

	popenfile = popen("pidof -s plmcat","r");
	fgets(buff,6,popenfile);
	if (0 != strcmp(buff,""))
		plmcatpid = atoi(buff);
	pclose(popenfile);

	if (plmcatpid > 0)
		kill(plmcatpid,SIGSTOP);

	sleeptime.tv_sec = ((msec - (msec % 1000)) / 1000);
	sleeptime.tv_nsec = (msec % 1000) * 1000000;

	ttyfd = open(ttylinename, O_RDWR | O_NOCTTY);

	if (ttyfd == -1) {
		fprintf(stderr,"%s ERROR: ",argv[0]);
		switch (errno) {
		case EACCES: case ENODEV: case ENOENT: case ENXIO:
			fprintf(stderr,"cannot open %s\n",ttylinename);
			exit(1);
			break;
		default:
			fprintf(stderr,"unknown\n");
			exit(2);
			break;
		}
	}

	FD_ZERO(&readfds);

	nfds = 0;
	nfds = (ttyfd > nfds) ? ttyfd : nfds;
	nfds++;

	tcgetattr(ttyfd,&newtio);

	newtio.c_cflag = baudrate | hwflowcontrol | linebits | linestopbits | lineparity | CLOCAL | CREAD;

	newtio.c_iflag = IGNBRK | IGNPAR;

	newtio.c_oflag = ONLRET;

	newtio.c_lflag = 0;

	newtio.c_cc[VMIN] = 0;
	newtio.c_cc[VTIME] = 1;

	tcflush(ttyfd, TCIFLUSH);

	tcsetattr(ttyfd,TCSANOW,&newtio);

	// look for other commands here that will not get an ACK
	if (0 == strncmp(str,"0260",4))
		toplm = TRUE;

	for (cp = str; ishex(*cp) && ishex(*(cp+1)); cp+=2) {
		c = hextoc(*cp,*(cp+1));
		write(ttyfd,&c,1);
		read(ttyfd,&c,1);
		if (verbose) {
			write(STDERR,cp,2);
			wrote = TRUE;
		}
	}
	if (!toplm) // there is no ACK if we're talking to the PLM itself
		while ((c != 0x06) && (c != 0x15)) {
			read(ttyfd,&c,1);
			if (verbose) {
				outhex(STDERR,c);
				wrote = TRUE;
			}
	}
	if (verbose && wrote) {
		c = '\n';
		write(STDERR,&c,1);
		wrote = FALSE;
	}

	FD_SET(ttyfd,&readfds);

	// sleep for a while unless data arrives
	ready = pselect(nfds,&readfds,NULL,NULL,&sleeptime,NULL);
	if(ready == -1) {
		fprintf(stderr,"pselect failed with error %i\n",errno);
		exit(-1);
	}
	while (0 < read(ttyfd,&c,1)) {
		outhex(STDOUT,c);
		wrote = TRUE;
	}
	if (wrote) {
		c = '\n';
		write(STDOUT,&c,1);
	}

	if (plmcatpid > 0)
		kill(plmcatpid,SIGCONT);

	return 0;
}
