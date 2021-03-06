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

#ifndef VFS_H_
#define VFS_H_

#include <fs/file.h>
#include <utils/tree.h>

typedef struct
{
	char name[FILE_NAME_LEN];
	file_open_t open;
	file_close_t close;
	file_read_t read;
	file_write_t write;
	file_seek_t seek;
	file_flush_t flush;
} node_t;

typedef struct
{
	node_t node;
	uint32_t flags;
	struct list openFiles;
	struct tree_link link;
} fsnode_t;

typedef void (*each_file_in_dir_t)(const char *name, uint32_t flags, void *arg);

bool k_vfs_init();

file_t *k_vfs_open_file(file_t *file, const char *path, uint32_t mode);
file_t *k_vfs_close_file(file_t *file);

/* common fs functions */
bool k_vfs_mknode(const char *path, const node_t *node, uint32_t flags);
bool k_vfs_rmnode(const char *path);

bool k_vfs_mkdir(const char *path, const char *name);
bool k_vfs_rmdir(const char *path);

bool k_vfs_mvnode(const char *path, const char *newPath);

bool k_vfs_path_is_exist(const char *path);

bool k_vfs_iterate_directory(const char *path, 
	each_file_in_dir_t callback, void *arg);

#endif /* VFS_H_ */
