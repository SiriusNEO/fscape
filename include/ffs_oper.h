#include "define.h"
#include "ffs_log.h"
#include "v_disk.h"
#include <fuse.h>
#include <unistd.h>

#ifndef FFS_OPER_H_
#define FFS_OPER_H_

/* standard fs interface */

int ffs_getattr
(const char* path, struct stat* stat_buf);

int ffs_readdir
(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);

int ffs_mkdir
(const char *path, mode_t mode);

int ffs_rmdir
(const char *path);

int ffs_mknod
(const char* path, mode_t mode, dev_t rdev);

int ffs_unlink
(const char *path);

int ffs_read
(const char* path, char* buffer, size_t size, off_t offset, struct fuse_file_info* fi);

int ffs_write
(const char* path, const char* buffer, size_t size, off_t offset, struct fuse_file_info* fi);

int ffs_rename
(const char *path, const char *new_path);

int ffs_chmod
(const char* path, mode_t mode);

int ffs_chown
(const char *path, uid_t uid, gid_t gid);

int ffs_truncate
(const char *path, off_t size);

int ffs_open
(const char *path, struct fuse_file_info *fi);

int ffs_statfs
(const char *path, struct statvfs *statfs_buf);

int ffs_utimens
(const char *path, const struct timespec tv[2]);

#endif