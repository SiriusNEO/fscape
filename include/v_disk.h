#include "define.h"
#include "../include/ffs_log.h"
#include "sys/types.h"
#include <stdlib.h>
#include <string.h>

#ifndef V_DISK_H_
#define V_DISK_H_

typedef struct block_ctx {
    i32 blk_offset;
    size_t size;
} block_ctx;

typedef struct inode {
    char file_name[MAX_FN_LEN];

    // link-list attr

    // first_son = 0 -> empty directory
    // first_son = -1 -> not a directory
    i32 first_son;
    // next_node = 0 -> there is only one file in this level
    i32 next_node;

    // stat
    struct stat st;

    // blk ctx
    block_ctx blk_ctx[MAX_BLOCKS_ONE_FILE];

} inode;

typedef struct super_block {
    i32 root_inode;
    size_t fs_blk_num;
    size_t fs_inode_num;
    char inode_bitmap[MAX_FILE_NUM];
    char block_bitmap[BLOCK_NUM];
} super_block;

extern super_block* super_block_buf;

extern FILE *image_fp;
extern char index_buf[V_DISK_INDEX_SIZE];

// create image: index and block
void create_image();

// try to open a disk image in the path (config.h)
void open_image();

// close the disk, write back
void close_image();

// get the first free inode (head offset) in the index_buf
i32 fetch_inode();

// get the first free inode (head offset) in the bank file
i32 fetch_block();

// ptr -> off
i32 inode_ptr_to_off(inode* ptr);

// off -> ptr
inode* inode_off_to_ptr(i32 offset);

// free a inode
i32 free_inode(i32 offset);

// free a block
i32 free_block(i32 offset);

// read only in one_block
i32 read_one_block(i32 bank_off, char* read_buf, size_t size, off_t off_in_blk);

// write only in one_block
i32 write_one_block(i32 bank_off, const char* write_buf, size_t size, off_t off_in_blk);

// create an empty inode and fill it into inode_buf
i32 new_empty_inode(const char* file_name, char is_dir, inode* inode_buf);

#endif