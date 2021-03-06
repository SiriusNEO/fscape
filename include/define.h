/* redefine the version of fuse at first*/
#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#ifndef CONFIG_H_
#define CONFIG_H_

/* type */

typedef int32_t i32;

/* internal name */

#define FS_ROOT "///internal-fs-root///"

/* virtual disk */

#define V_DISK_PATH "disk_image"

#define V_DISK_INDEX_SIZE (10*1048576) // 10M

#define V_DISK_BANK_SIZE (20*1048576) // 20M
// #define V_DISK_BANK_SIZE (128) // 128 bytes

#define BLOCK_SIZE (4*1024) // 4K
#define BLOCK_NUM (V_DISK_BANK_SIZE / BLOCK_SIZE)

// #define MAX_ONE_FILE_SIZE (1048576) // 1M
// #define MAX_ONE_FILE_SIZE (64) // 64 bytes
#define MAX_FILE_NUM (1024)
#define MAX_FILE_DEP (128)

#define MAX_BLOCKS_ONE_FILE 12 

#define MAX_FN_LEN 512
#define MAX_PATH_LEN (MAX_FILE_DEP * MAX_FN_LEN + MAX_FILE_DEP + 5)

/* errno */

#define INNER_ERROR   -1
#define NOT_DIRECTORY -1
#define EMPTY          0

#endif