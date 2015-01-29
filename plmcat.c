/*
    plmcat - read data from an insteon PLM and display as ASCII hex
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
#include <signal.h>
#include <errno.h>
#include "plm.h"

#define uc unsigned char

extern int baudrate;
extern int hwflowcontrol;
extern int translation;
extern int linebits;
extern int lineparity;
extern int linestopbits;
extern int lineflush;
extern char *ttylinename;

char *options = "d:h";

void broken_pipe(int sig)
{
	if (sig == SIGPIPE)
		exit(3);
}

int main(int argc, char **argv) {
	int TTYFD,nfds,ready,i=0;
	FILE *ttyfd;
	char c,buff[128],tbuff[128];
	struct termios newtio;
	int helpwanted = FALSE;
	fd_set readfds;
//	struct timespec timeout;

	(void) signal(SIGPIPE, broken_pipe);

//	timeout.tv_sec=5;
//	timeout.tv_nsec=0;

	// Parse command line using getopt() from libc
while (255 != (c = getopt(argc,argv,options))) {
		switch (c) {
		case 'd':
			ttylinename = optarg;
			break;
		case 'h':
			helpwanted = TRUE;
			break;
		}
	}

	if (helpwanted) {
		fprintf(stdout,
"USAGE: %s [OPTION]...\n"
"\n"
"\tOPTIONS:\n"
"\n"
"\t-h\t\tdisplay this help menu\n"
"\t-d\t\tPLM tty device name (/dev/ttyUSB0)\n"
"\n",
argv[0]);
		exit(0);
	}

	TTYFD = open(ttylinename, O_RDONLY | O_NOCTTY);
	ttyfd = fdopen(TTYFD, "r");

        if (TTYFD == -1) {
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
	nfds = (TTYFD > nfds) ? TTYFD : nfds;
	nfds++;

	tcgetattr(TTYFD,&newtio);

	newtio.c_cflag = baudrate | hwflowcontrol | linebits | linestopbits | lineparity | CLOCAL | CREAD;

	newtio.c_iflag = IGNBRK | IGNPAR;

	newtio.c_oflag = ONLRET;

	newtio.c_lflag = 0;

	newtio.c_cc[VMIN] = 0;
	newtio.c_cc[VTIME] = 1;

	tcflush(TTYFD, TCIFLUSH);

	tcsetattr(TTYFD,TCSANOW,&newtio);

	while (TRUE) {
		FD_SET(TTYFD,&readfds);
//		ready = pselect(nfds,&readfds,NULL,NULL,&timeout,NULL);
		ready = pselect(nfds,&readfds,NULL,NULL,NULL,NULL);
		if (FD_ISSET(TTYFD,&readfds)) {
			fgets(buff,127,ttyfd);
			if ((uc)buff[0] == 0x02 && (uc)buff[1] == 0x52) {
				if ((uc)buff[3] == 0x00) {
					FD_SET(TTYFD,&readfds);
//					ready = pselect(nfds,&readfds,NULL,NULL,&timeout,NULL);
					ready = pselect(nfds,&readfds,NULL,NULL,NULL,NULL);
					if (FD_ISSET(TTYFD,&readfds)) {
						fgets(tbuff,127,ttyfd);
						for (i = 0; i < 4; i++)
							outhex(STDOUT,buff[i]);
						c = ' ';
						write(STDOUT,&c,1);
						for (i = 0; i < 4; i++)
							outhex(STDOUT,tbuff[i]);
						c = '\n';
						write(STDOUT,&c,1);
					} else {
						for (i = 0; i < 4; i++)
							outhex(STDOUT,buff[i]);
						c = '\n';
						write(STDOUT,&c,1);
					}
				}
				else if ((uc)buff[3] == 0x80) {
					if (((uc)buff[2] & 0x0F) == 0x04 || ((uc)buff[2] & 0x0F) == 0x05) {
						for (i = 0; i < 4; i++)
							outhex(STDOUT,buff[i]);
						c = '\n';
						write(STDOUT,&c,1);
						FD_SET(TTYFD,&readfds);
//						ready = pselect(nfds,&readfds,NULL,NULL,&timeout,NULL);
						ready = pselect(nfds,&readfds,NULL,NULL,NULL,NULL);
						if (FD_ISSET(TTYFD,&readfds))
							fgets(buff,127,ttyfd);
					}
				}
			} else {
				while (0 < read(TTYFD,&c,1))
					outhex(STDOUT,buff[i]);
				c = '\n';
				write(STDOUT,&c,1);
			}
		}
	}
}
