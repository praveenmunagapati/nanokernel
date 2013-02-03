/*  This file is part of simple kernel.
    Project NanoKernel (for study purposes only)
    Copyright (C) 2013  Sirius (Vdov Nikita)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __CONSOLE_H
#define __CONSOLE_H

/* is this not a kernel includes, add for easy */
#include "std/membase.h"

#define TAB_WHITE_SPACES	4

typedef struct 
{
	ptr_t *memEntry;
	size_t xLen;
	size_t yLen;
} consoleEntry_t;

int console_init(const consoleEntry_t *entry);

int console_clean();

int console_set_cursor_pos(size_t x, size_t y);

int console_seek(size_t pos);

int console_putc(char c);

size_t console_write(char *message);



#endif