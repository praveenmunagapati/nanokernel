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

#include <stdin.h>
#include <fs/vfs.h>
#include <fs/file.h>

static keyBuf_t keybordBuffer;

static void* stdin_open(const char *path, uint32_t mode, void *file)
{
	/* read only */
	if(!(mode & FILE_IN))
		return NULL;
	
	return file;
}

static bool stdin_close(void *file)
{
	return true;
}

static long stdin_read(ptr_t buf, size_t size, size_t count, void *file)
{
	/* fill input buffer */
	size_t fullSize = size * count;
	char *destBuf = buf;
	if(!keybordBuffer.bufReady)
		return 0;
	if(fullSize < 1)
		return -1;
	
	*destBuf = keybordBuffer.symbol;
	keybordBuffer.bufReady = false;
	return 1;
}

bool init_stdin(void)
{
	node_t stdinNode = {
		"stdin",
		stdin_open, 
		stdin_close, 
		stdin_read,
		NULL,
		NULL,
		NULL
	};
		
	k_vfs_mkdir("/", "dev");
	if(!k_vfs_mknode("/dev/", &stdinNode, 0x0))
		return false;
	
	return true;
}

bool enter_ascii_symbol(byte symbol)
{
	keybordBuffer.symbol = symbol;
	keybordBuffer.bufReady = true;
	return true;
}
