/*
    plm.h - common functions for plmtools
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

#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#define _POSIX_SOURCE 1

#define FALSE 0
#define TRUE (!FALSE)

#define STDIN STDIN_FILENO
#define STDOUT STDOUT_FILENO
#define STDERR STDERR_FILENO

#define BAUDRATE B19200
#define HWFLOWCTRL FALSE
#define TRANSLATION FALSE
#define LINEBITS CS8
#define LINEPARITY 0
#define LINESTOPBITS 0
#define LINEFLUSH FALSE
#define TTYLINENAME "/dev/ttyUSB0"

int ishex(char c);
void outhex(int fdes, char c);
char hextoc(char c1, char c2);

