#pragma once

#include <ctype.h>

#include "defs/defs.h"

#define VFS_DISK_DELIMITER ':'
#define VFS_NAME_DELIMITER '/'

#define VFS_END_OF_DISK(ch) ((ch) == VFS_DISK_DELIMITER || (ch) == '\0')
#define VFS_END_OF_NAME(ch) ((ch) == VFS_DISK_DELIMITER || (ch) == VFS_NAME_DELIMITER || (ch) == '\0')
#define VFS_VALID_CHAR(ch) (isalnum(ch) || (ch) == '_' || (ch) == '.')

bool vfs_valid_name(const char* name);

bool vfs_valid_path(const char* path);

bool vfs_compare_names(const char* a, const char* b);

const char* vfs_next_dir(const char* path);

const char* vfs_basename(const char* path);