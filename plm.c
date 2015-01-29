/*
    plm.c - common functions for plmtools
    Copyright (C) 2008  Matthew Randolph
    See the file COPYING for license information.

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

#include "plm.h"

int baudrate = BAUDRATE;
int hwflowcontrol = HWFLOWCTRL;
int translation = TRANSLATION;
int linebits = LINEBITS;
int lineparity = LINEPARITY;
int linestopbits = LINESTOPBITS;
int lineflush = LINEFLUSH;
char *ttylinename = TTYLINENAME;

int ishex(char c) {
	char uc;

	uc = toupper(c);

	if (((uc >= '0') && (uc <= '9')) || ((uc >= 'A') && (uc <= 'F')))
		return TRUE;
	else
		return FALSE;
}

void outhex(int fdes, char c) {
	unsigned char uc;
	char tc;

	uc = *(unsigned char *)(&c);
	tc = ((uc / 16) % 16);
	tc = ((tc >= 0) && (tc <= 9)) ? (tc + '0') : (tc - 10 + 'A');
	write(fdes,&tc,1);
	tc = (uc % 16);
	tc = ((tc >= 0) && (tc <= 9)) ? (tc + '0') : (tc - 10 + 'A');
	write(fdes,&tc,1);
}

char hextoc(char c1, char c2) {
	unsigned char uc;

	c1 = toupper(c1);
	c2 = toupper(c2);

	if ((c1 >= '0') && (c1 <= '9'))
		uc = c1 - '0';
	else
		uc = c1 - 'A' + 10;

	uc *= 16;

	if ((c2 >= '0') && (c2 <= '9'))
		uc += c2 - '0';
	else
		uc += c2 - 'A' + 10;

	return *(char *)(&uc);
}
