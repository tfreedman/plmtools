/*
    plmtty - interactive insteon PLM communications program
    Copyright (C) 2008  Matthew Randolph

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

int main(int argc, char **argv) {
	int TTYFD,nfds,ready;
	FILE *ttyfd;
	char c,buff[128],tbuff[128],*cp;
	struct termios newtio;
	int helpwanted = FALSE;
	fd_set readfds;

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
"\n",argv[0]);
		exit(0);
	}


	TTYFD = open(ttylinename, O_RDWR | O_NOCTTY);
	ttyfd = fdopen(TTYFD, "r+");

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
	nfds = (STDIN > nfds) ? STDIN : nfds;
	nfds = (TTYFD > nfds) ? TTYFD : nfds;
	nfds++;

	tcgetattr(TTYFD,&newtio);

	newtio.c_cflag = baudrate | hwflowcontrol | linebits | linestopbits | lineparity | CLOCAL | CREAD;

//	newtio.c_iflag = IGNBRK | IGNPAR | IGNCR;
	newtio.c_iflag = IGNBRK | IGNPAR;

	newtio.c_oflag = ONLRET;

	newtio.c_lflag = 0;

	newtio.c_cc[VMIN] = 0;
	newtio.c_cc[VTIME] = 1;

	tcflush(TTYFD, TCIFLUSH);

	tcsetattr(TTYFD,TCSANOW,&newtio);

	c = 0x02; write(TTYFD,&c,1); read(TTYFD,&c,1);
	c = 0x60; write(TTYFD,&c,1); read(TTYFD,&c,1);

	while (TRUE) {
		FD_SET(STDIN,&readfds);
		FD_SET(TTYFD,&readfds);
		ready = pselect(nfds,&readfds,NULL,NULL,NULL,NULL);
		if (FD_ISSET(TTYFD,&readfds)) {
			fgets(buff,127,ttyfd);
			if ((uc)buff[3] == 0x03 && (uc)buff[4] == 0x05 && (uc)buff[6] == 0x06)
			// if "Get IM Info"
				printf("%.2X%.2X%.2X%.2X%.2X%.2X%.2X\n",(uc)buff[0],(uc)buff[1],(uc)buff[2],(uc)buff[3],(uc)buff[4],(uc)buff[5],(uc)buff[6]);
			else if ((uc)buff[0] == 0x02 && (uc)buff[1] == 0x52)
			{
			// else if X10 type
				if ((uc)buff[3] == 0x00) {
				// if first half of two part command
					FD_SET(TTYFD,&readfds);
					ready = pselect(nfds,&readfds,NULL,NULL,NULL,NULL);
					if (FD_ISSET(TTYFD,&readfds)) {
						fgets(tbuff,127,ttyfd);
						printf("%.2X%.2X%.2X%.2X %.2X%.2X%.2X%.2X\n",(uc)buff[0],(uc)buff[1],(uc)buff[2],(uc)buff[3],(uc)tbuff[0],(uc)tbuff[1],(uc)tbuff[2],(uc)tbuff[3]);
					} else {
						printf("%.2X%.2X%.2X%.2X\n",(uc)buff[0],(uc)buff[1],(uc)buff[2],(uc)buff[3]);
					}
				}
				else if ((uc)buff[3] == 0x80) {
				// if orphaned second half or bright/dim command
					printf("%.2X%.2X%.2X%.2X\n",(uc)buff[0],(uc)buff[1],(uc)buff[2],(uc)buff[3]);
					if (((uc)buff[2] & 0x0F) == 0x04 || ((uc)buff[2] & 0x0F) == 0x05) {
					// if bright/dim command read and discard next (duplicate) line if present
					// note: should test that it really is a duplicate line
						FD_SET(TTYFD,&readfds);
						ready = pselect(nfds,&readfds,NULL,NULL,NULL,NULL);
						if (FD_ISSET(TTYFD,&readfds))
							fgets(buff,127,ttyfd);
					}
				}

			}
			else {
			// else it is an unhandled message type so print it
				while (0 < read(TTYFD,&c,1))
					printf("%.2X",(uc)c);
				printf("\n");
			}
		}
		if (FD_ISSET(STDIN,&readfds)) {
			fgets(buff,127,stdin);
			for (cp = buff; ishex(*cp) && ishex(*(cp+1)); cp+=2) {
				c = hextoc(*cp,*(cp+1));
				write(TTYFD,&c,1);
				read(TTYFD,&c,1);
				printf("%.2X",(uc)c);
			}
		}
	}
}
