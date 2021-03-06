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

#include <shell.h>
#include <std/print.h>
#include <std/strings.h>
#include <mem.h>
#include <cpu.h>
#include <kerror.h>
#include <keyboard.h>
#include <fs/file.h>
#include <fs/vfs.h>
#include <console.h>
#include <cpuinfo.h>

#define COMMAND_BUFFER_SIZE			2048

static char *commandBuf;

static void allocate_command_buffer()
{
	commandBuf = (char *)k_malloc(COMMAND_BUFFER_SIZE);
}

static char *read_line_to_command_buffer(int stdin)
{
	char symbol;
	char *pBuf = commandBuf;
	while(true)
	{
		k_wait_keyboard();
		if(k_fread(stdin, &symbol, 1, 1) > 0)
		{			
			/* check on \n */
			if(symbol == '\n')
			{			
				/* to next line */
				k_print("\n");	
				*pBuf = 0;
				break;
			}
			/* check command buffer is full */
			if((pBuf - commandBuf) > COMMAND_BUFFER_SIZE+1)
				continue;
			
			/* print char */
			k_print("%c", symbol);	
			*pBuf = symbol;
			pBuf++;
		}
		/* backspace */
		if(k_get_keyboard_state_key(KEY_BACKSPACE) == KEY_PESSED)
		{
			if(pBuf > commandBuf)
			{
				k_console_clean_front();
				pBuf--;
			}
		}
	}
	
	return commandBuf;
}

static int process_command(char *command)
{
	if(k_strlen(command) == 0)
		return EXIT_PROCESS_NEXT;
	
	if(k_strcmp(command, "version") == 0)
		k_print("%s\n", k_version_full_string);
	else if(k_strcmp(command, "help") == 0)
	{
		k_print("help - print help\n"
				"version - print version\n"
				"memory - memory info\n"
				"free - virtual memory info\n"
				"cpuinfo - CPU info\n"
				"reboot - reboot computer\n"
				"int - debug interrupt\n"
				);
	}
	else if(k_strcmp(command, "int") == 0)
		k_breakpoint();
	else if(k_strcmp(command, "reboot") == 0)
		return EXIT_COLD_BOOT;
	else if(k_strcmp(command, "cpuinfo") == 0)
		k_cpuinfo_print(k_get_cpuinfo());
	else if(k_strcmp(command, "memory") == 0)
		k_print_memory_info();
	else if(k_strcmp(command, "free") == 0)
		k_print_memory_usage_info();
	else
	{
		k_console_set_color(CONCOLE_BACKGROUND_BLACK,
			CONCOLE_SYMBOL_COLOR_RED);	
		k_print("unknown command '%s'\n", command);
		k_console_set_color(CONCOLE_BACKGROUND_BLACK,
			CONCOLE_SYMBOL_COLOR_LIGHTGREY);
	}

	
	return EXIT_PROCESS_NEXT;
}

static void print_command_line_beginer()
{
	k_console_set_color(CONCOLE_BACKGROUND_BLACK,
		CONCOLE_SYMBOL_COLOR_CYAN);	
	k_print("nikitOS:> ");
	k_console_set_color(CONCOLE_BACKGROUND_BLACK,
		CONCOLE_SYMBOL_COLOR_LIGHTGREY);
}

int k_start_shell()
{
	int stdin = k_fopen("/dev/stdin", FILE_OPEN_IN_VFS | FILE_IN);
	int exit;
	if(!stdin)
		k_panic1(INIT_FAILED);
	allocate_command_buffer();
	
	k_print("Welcome to NikitOS! Try 'help' first!\n\n");
	while(true)
	{
		print_command_line_beginer();
		exit = process_command(read_line_to_command_buffer(stdin));
		
		if(exit != EXIT_PROCESS_NEXT)
			break;
	}
	
	return exit;
}
