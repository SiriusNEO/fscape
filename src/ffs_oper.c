#include "../include/ffs_oper.h"

// split path to [dir1, dir2, ..., dirn] + file
i32 split_path(const char* path, char** dir, int* dir_cnt, char* file) {
    int len = strlen(path);

    // judge '/' first makes the code neat
    if (0 == strcmp(path, "/")) {
        *dir_cnt = 0;
        strcpy(file, FS_ROOT);
    } 
    else {
        // emit the last '/'
        if (path[len-1] == '/') len--;

        int i = 0, fn_temp_index = 0;
        *dir_cnt = 0;
        char fn_temp[MAX_FN_LEN];

        for (i = 0; i < len; ++i) {
            if (path[i] == '/') {
                if (i == 0) {
                    strcpy(dir[*dir_cnt], FS_ROOT);
                }
                else {
                    strcpy(dir[*dir_cnt], fn_temp);
                }
                memset(fn_temp, '\0', sizeof(fn_temp));
                (*dir_cnt) ++;
                fn_temp_index = 0;
            }
            else {
                fn_temp[fn_temp_index++] = path[i];
            }
        }

        strcpy(file, fn_temp);
    }
    
    return 0;
}

// use path to get inode.
// return INNER_ERROR if the path can not be mapped to a inode
i32 path_to_inode(const char* path) {
    int i, dir_cnt = 0, ret;

    char **dir = malloc(sizeof(char*)*MAX_FILE_DEP), file[MAX_FN_LEN];

    for (i = 0; i < MAX_FILE_DEP; ++i) {
        dir[i] = malloc(sizeof(char)*MAX_FN_LEN);
    }
    
    split_path(path, dir, &dir_cnt, file);

    /*
        for (i = 0; i < dir_cnt; ++i) {
            FFS_DBG_INFO("dir: %s\n", dir[i]);
        }
        FFS_DBG_INFO("file: %s\n", file);
    */

    inode* now_ptr;

    if (dir_cnt == 0) {
        if (0 != strcmp(file, FS_ROOT)) {
            FFS_DBG_ERR("the path is not started with root\n");
            ret = -ENOENT;
            goto error_exit;
        }
        else {
            now_ptr = inode_off_to_ptr(super_block_buf->root_inode);
        }
    }
    else {
        if (0 != strcmp(dir[0], FS_ROOT)) {
            FFS_DBG_ERR("the path is not started with root\n");
            ret = -ENOENT;
            goto error_exit;
        }
        else {
            i = 1; // emit root
            now_ptr = inode_off_to_ptr(super_block_buf->root_inode);
        }

        // +1: the last level is "file"
        for (   ; i <= dir_cnt; ++i) {
            if (now_ptr->first_son == NOT_DIRECTORY) {
                FFS_DBG_ERR("try go into a file which is not a directory\n");
                ret = -ENOTDIR;
                goto error_exit;
            }

            inode* son_ptr = inode_off_to_ptr(now_ptr->first_son);
            now_ptr = NULL;

            if (i < dir_cnt) {
                // scan in this level
                while (inode_ptr_to_off(son_ptr) != EMPTY) {
                    if (0 == strcmp(son_ptr->file_name, dir[i])) {
                        now_ptr = son_ptr;
                        break;
                    }
                    son_ptr = inode_off_to_ptr(son_ptr->next_node);
                }

                if (now_ptr == NULL) {
                    FFS_DBG_ERR("directory <%s> not found\n", dir[i]);
                    ret = -ENOENT;
                    goto error_exit;
                }
            }
            else {
                // scan the last level
                while (inode_ptr_to_off(son_ptr) != EMPTY) {
                    if (0 == strcmp(son_ptr->file_name, file)) {
                        now_ptr = son_ptr;
                        break;
                    }
                    son_ptr = inode_off_to_ptr(son_ptr->next_node);
                }

                if (now_ptr == NULL) {
                    FFS_DBG_ERR("file <%s> not found\n", file);
                    ret = -ENOENT;
                    goto error_exit;
                }
            }
        }
    }

    for (i = 0; i < MAX_FILE_DEP; ++i) free(dir[i]);
    free(dir);
    return (char *)now_ptr - index_buf;

error_exit:
    for (i = 0; i < MAX_FILE_DEP; ++i) free(dir[i]);
    free(dir);
    return ret;
}

// split path to dir_path + file
i32 extract_filename(const char* path, char* dir_path, char* file) {
    int i, len = strlen(path), border = len-1; 

    if (len == 1) {
        if (0 != strcmp(file, FS_ROOT)) {
            FFS_DBG_ERR("the path is not started with root\n");
            return -ENOENT;
        }
        else {
            strcpy(file, FS_ROOT);
        }
    }
    else {
        do {
            border--;
        }
        while (path[border] != '/');

        for (i = 0; i < len; ++i) {
            if (i < border) {
                dir_path[i] = path[i];
            }
            else if (i == border) { 
                dir_path[i] = '\0';
            }
            else if (i > border) {
                file[i-border-1] = path[i];
            }
        }

        file[len-border-1] = '\0';

        if (border == 0) { // it must be '/'
            strcpy(dir_path, "/");
        }
    }
    return 0;
}

// link a new inode (is allocated) under parent
// just do a link without fetching any disk resource!
i32 link_inode(inode* parent_ptr, inode* this_ptr) {
    this_ptr->next_node = parent_ptr->first_son;
    parent_ptr->first_son = inode_ptr_to_off(this_ptr);
    return 0;
}

// unlink an inode (is allocated) under parent
// just do an unlink without freeing any disk resource!
i32 unlink_inode(inode* parent_ptr, inode* this_ptr) {
    i32 this_off = inode_ptr_to_off(this_ptr);

    // this is first
    if (parent_ptr->first_son == this_off) {
        parent_ptr->first_son = this_ptr->next_node;
    }
    else {
        inode* bro_ptr = inode_off_to_ptr(parent_ptr->first_son);

        while (inode_ptr_to_off(bro_ptr) != EMPTY) {
            // front
            if (bro_ptr->next_node == this_off) {
                bro_ptr->next_node = this_ptr->next_node;
                break;
            }
            bro_ptr = inode_off_to_ptr(bro_ptr->next_node);
        }
    }

    return 0;
}

// insert a file with the given path. create a new inode and link it.
i32 insert_file(const char* path, char is_dir) {
    int i;
    char dir_path[MAX_PATH_LEN], file_name[MAX_FN_LEN];

    extract_filename(path, dir_path, file_name);

    // FFS_DBG_INFO("dir_path = %s\n", dir_path);
    // FFS_DBG_INFO("file_name = %s\n", file_name);

    i32 parent_off = path_to_inode(dir_path);
    
    if (parent_off < 0) {
        return parent_off;
    }

    inode* parent_ptr = inode_off_to_ptr(parent_off);

    if (parent_ptr->first_son == NOT_DIRECTORY) {
        return -ENOTDIR;
    }

    i32 new_inode_off = fetch_inode();
    inode* new_inode_ptr = inode_off_to_ptr(new_inode_off);
    memset(new_inode_ptr, 0, sizeof(inode));

    new_empty_inode(file_name, is_dir, new_inode_ptr);

    // not a directory
    new_inode_ptr->first_son = is_dir ? EMPTY : NOT_DIRECTORY;

    link_inode(parent_ptr, new_inode_ptr);    

    return 0;
}

// destory an inode and free its space.
// notice: it will be executed recursively to free all inner files
i32 destory_inode(inode* parent_ptr, inode* this_ptr) {
    // has be checked and must be directory

    i32 this_off = inode_ptr_to_off(this_ptr);

    unlink_inode(parent_ptr, this_ptr);

    // free all sons first
    if (this_ptr->first_son != NOT_DIRECTORY) {
        inode* son_ptr = inode_off_to_ptr(this_ptr->first_son);

        while (inode_ptr_to_off(son_ptr) != EMPTY) {
            destory_inode(this_ptr, son_ptr);
            son_ptr = (inode*) (index_buf + son_ptr->next_node);
        }
    }

    int i;
    for (i = 0; i < this_ptr->st.st_blocks; ++i) {
        free_block(this_ptr->blk_ctx[i].blk_offset);
    }
    free_inode(this_off);

    return 0;
}

// a wrapper for destorying inode. 
// find the inode with the given path and destory the inode. 
i32 remove_file(const char* path) {
    int i;
    char dir_path[MAX_PATH_LEN], file_name[MAX_FN_LEN];

    extract_filename(path, dir_path, file_name);

    FFS_DBG_INFO("dir_path = %s\n", dir_path);
    FFS_DBG_INFO("file_name = %s\n", file_name);

    i32 parent_off = path_to_inode(dir_path), this_off = path_to_inode(path);
    
    if (parent_off < 0) {
        return parent_off;
    }

    inode* parent_ptr = inode_off_to_ptr(parent_off);
    inode* this_ptr = inode_off_to_ptr(this_off);

    if (parent_ptr->first_son == NOT_DIRECTORY) {
        return -ENOTDIR;
    }

    destory_inode(parent_ptr, this_ptr);

    return 0;
}

/* standard fs interface implementation */

int ffs_getattr(const char* path, struct stat* stat_buf) {

    // FFS_DBG_INFO("@getattr in <path = %s>\n", path);

    // uid: user ID
    // gid: group ID

    // check the access

    i32 inode_off = path_to_inode(path);

    if (inode_off < 0) {
        return inode_off;
    }

    inode* inode_ptr = (inode*) (index_buf + inode_off);

    memcpy(stat_buf, &inode_ptr->st, sizeof(struct stat));
    stat_buf->st_atime = time(0); // just now

    return 0;
}

int ffs_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    // FFS_DBG_INFO("@readdir in <path = %s>\n", path);

    i32 dir_off = path_to_inode(path);

    if (dir_off < 0) {
        return dir_off;
    }

    inode* dir_ptr = (inode*) (index_buf + dir_off);

    if (dir_ptr->first_son == NOT_DIRECTORY) {
        return -ENOTDIR;
    }

    inode* son_ptr = (inode*) (index_buf + dir_ptr->first_son);

    while (inode_ptr_to_off(son_ptr) != EMPTY) {
        filler(buffer, son_ptr->file_name, &son_ptr->st, 0);
        son_ptr = (inode*) (index_buf + son_ptr->next_node);
    }

    return 0;
}

int ffs_mkdir(const char *path, mode_t mode) {
    FFS_DBG_INFO("@mkdir in <path = %s>\n", path);

    // "mode" is ignored

    int result = insert_file(path, 1);

    return result;
}

int ffs_rmdir(const char *path) {
    FFS_DBG_INFO("@rmdir in <path = %s>\n", path);

    // "mode" is ignored
    // do we need to delete all son-nodes when rmdir?

    if (0 == strcmp(path, "/")) {
        return -EPERM;
    }

    int result = remove_file(path);

    return result;
}

int ffs_unlink(const char *path) {
    FFS_DBG_INFO("@unlink in <path = %s>\n", path);

    int result = remove_file(path);

    return result;
}

int ffs_mknod(const char* path, mode_t mode, dev_t rdev) {
    FFS_DBG_INFO("@mknod in <path = %s>\n", path);

    // "mode" and "rdev" is ignored

    int result = insert_file(path, 0);

    return result;
}

int ffs_read(const char* path, char* buffer, size_t size, off_t offset, struct fuse_file_info* fi) {
    FFS_DBG_INFO("@read in <path = %s> <size = %d> <offset = %d>\n", path, size, offset);

    i32 inode_off = path_to_inode(path);

    if (inode_off < 0) {
        return inode_off;
    }

    inode* inode_ptr = (inode*) (index_buf + inode_off);

    if (inode_ptr->first_son != NOT_DIRECTORY) {
        return -EISDIR;
    }

    if (offset+size > inode_ptr->st.st_size) 
        size = inode_ptr->st.st_size - offset;

    i32 start_block_idx = offset / BLOCK_SIZE, 
    
    front_res_len = BLOCK_SIZE*(start_block_idx+1) - offset,
    tail_res_len = 0,

    first_start_off_in_blk = offset - start_block_idx*BLOCK_SIZE, 
    first_end_off_in_blk = BLOCK_SIZE-1,
    
    cover_block_num = 1,
    block_idx = 0;

    if (size <= front_res_len) {
        first_end_off_in_blk = first_start_off_in_blk + size - 1;
    }
    else {
        cover_block_num += (size - front_res_len - 1) / BLOCK_SIZE + 1;
        tail_res_len = size - (cover_block_num-1)*BLOCK_SIZE;
    }

    for (block_idx = start_block_idx; block_idx < start_block_idx+cover_block_num; ++block_idx) {
        if (block_idx == start_block_idx) {
            // first block
            read_one_block(inode_ptr->blk_ctx[block_idx].blk_offset, 
                            buffer + (block_idx-start_block_idx)*BLOCK_SIZE, 
                            front_res_len, 
                            first_start_off_in_blk);
        }
        else if (block_idx == start_block_idx+cover_block_num-1) {
            // last block
            read_one_block(inode_ptr->blk_ctx[block_idx].blk_offset, 
                            buffer + (block_idx-start_block_idx)*BLOCK_SIZE, 
                            tail_res_len, 
                            0);
        }
        else {
            // blocks in the middle
            read_one_block(inode_ptr->blk_ctx[block_idx].blk_offset, 
                            buffer + (block_idx-start_block_idx)*BLOCK_SIZE, 
                            BLOCK_SIZE, 
                            0);
        }
    }

    return size;
}

int ffs_write(const char* path, const char* buffer, size_t size, off_t offset, struct fuse_file_info* fi) {
    FFS_DBG_INFO("@write in <path = %s> <size = %d> <offset = %d> <buffer = %s>\n", path, size, offset, buffer);

    i32 inode_off = path_to_inode(path);

    if (inode_off < 0) {
        return inode_off;
    }

    inode* inode_ptr = (inode*) (index_buf + inode_off);

    if (inode_ptr->first_son != NOT_DIRECTORY) {
        return -EISDIR;
    }

    i32 start_block_idx = offset / BLOCK_SIZE, 
    
    front_res_len = BLOCK_SIZE*(start_block_idx+1) - offset,
    tail_res_len = 0,

    first_start_off_in_blk = offset - start_block_idx*BLOCK_SIZE, 
    first_end_off_in_blk = BLOCK_SIZE-1,
    
    cover_block_num = 1,
    block_idx = 0;

    // offset - start_block_idx*BLOCK_SIZE

    if (size <= front_res_len) {
        first_end_off_in_blk = first_start_off_in_blk + size - 1;
    }
    else {
        cover_block_num += (size - front_res_len - 1) / BLOCK_SIZE + 1;
        tail_res_len = size - (cover_block_num-1)*BLOCK_SIZE;
    }

    while (inode_ptr->st.st_blocks < start_block_idx+cover_block_num) {
        i32 new_block_off = fetch_block();
        if (new_block_off < 0) {
            return new_block_off;
        }
        inode_ptr->blk_ctx[inode_ptr->st.st_blocks].blk_offset = new_block_off;
        inode_ptr->blk_ctx[inode_ptr->st.st_blocks].size = 0;
        inode_ptr->st.st_blocks++;
    }

    FFS_DBG_INFO("start_block_idx = %d\n", start_block_idx);
    FFS_DBG_INFO("cover_block_num = %d\n", cover_block_num);
    FFS_DBG_INFO("front_res_len = %d\n", front_res_len);
    FFS_DBG_INFO("tail_res_len = %d\n", tail_res_len);

    for (block_idx = start_block_idx; block_idx < start_block_idx+cover_block_num; ++block_idx) {
        if (block_idx == start_block_idx) {
            // first block
            write_one_block(inode_ptr->blk_ctx[block_idx].blk_offset, 
                            buffer + (block_idx-start_block_idx)*BLOCK_SIZE, 
                            front_res_len, 
                            first_start_off_in_blk);
            
            if (first_end_off_in_blk + 1 > inode_ptr->blk_ctx[block_idx].size)
                inode_ptr->blk_ctx[block_idx].size = first_end_off_in_blk + 1;
        }
        else if (block_idx == start_block_idx+cover_block_num-1) {
            // last block
            write_one_block(inode_ptr->blk_ctx[block_idx].blk_offset, 
                            buffer + (block_idx-start_block_idx)*BLOCK_SIZE, 
                            tail_res_len, 
                            0);
            
            if (tail_res_len + 1 > inode_ptr->blk_ctx[block_idx].size)
                inode_ptr->blk_ctx[block_idx].size = tail_res_len + 1;
        }
        else {
            // blocks in the middle
            write_one_block(inode_ptr->blk_ctx[block_idx].blk_offset, 
                            buffer + (block_idx-start_block_idx)*BLOCK_SIZE, 
                            BLOCK_SIZE, 
                            0);

            inode_ptr->blk_ctx[block_idx].size = BLOCK_SIZE;
        }
    }

    if (offset+size > inode_ptr->st.st_size)
        inode_ptr->st.st_size = offset+size;

    return size;
}

int ffs_rename(const char *path, const char *new_path) {
    FFS_DBG_INFO("@rename in <path = %s> <new_path = %s>\n", path, new_path);

    if (0 == strcmp(path, "/") || 0 == strcmp(new_path, "/")) {
        return -EPERM;
    }

    char dir_path[MAX_PATH_LEN], file_name[MAX_FN_LEN];
    char new_dir_path[MAX_PATH_LEN], new_file_name[MAX_FN_LEN];

    extract_filename(path, dir_path, file_name);
    extract_filename(new_path, new_dir_path, new_file_name);

    i32 parent_off = path_to_inode(dir_path), 
        new_parent_off = path_to_inode(new_dir_path), 
        inode_off = path_to_inode(path), 
        new_inode_off = path_to_inode(new_path);

    if (parent_off < 0 || new_parent_off < 0 || inode_off < 0) {
        return -ENOENT;
    }

    inode *parent_ptr = inode_off_to_ptr(parent_off), 
           *new_parent_ptr = inode_off_to_ptr(new_parent_off),
           *inode_ptr = inode_off_to_ptr(inode_off);
    
    if (parent_ptr->first_son == NOT_DIRECTORY || new_parent_ptr->first_son == NOT_DIRECTORY) {
        return -ENOTDIR;
    }

    // there is file
    if (new_inode_off != ENOENT) {
        inode* new_inode_ptr = inode_off_to_ptr(new_inode_off);
        destory_inode(new_parent_ptr, new_inode_ptr);
    }

    unlink_inode(parent_ptr, inode_ptr);
    link_inode(new_parent_ptr, inode_ptr);

    strcpy(inode_ptr->file_name, new_file_name);

    return 0;
}

int ffs_chmod(const char* path, mode_t mode) {
    /* not implemented */
    FFS_DBG_WARN("@chmod not implemented in path: %s\n", path);
    return 0;
}

int ffs_chown(const char *path, uid_t uid, gid_t gid) {
    /* not implemented */
    FFS_DBG_WARN("@chown not implemented in path: %s\n", path);
    return 0;
}

int ffs_truncate(const char *path, off_t size) {
    /* not implemented */
    FFS_DBG_WARN("@truncate not implemented in path: %s\n", path);
    return 0;
}

int ffs_open(const char *path, struct fuse_file_info *fi) {
    /* not implemented */
    FFS_DBG_WARN("@open not implemented in path: %s\n", path);
    return 0;
}

int ffs_statfs(const char *path, struct statvfs *statfs_buf) {
    FFS_DBG_INFO("@statfs in <path = %s>\n", path);

    statfs_buf->f_namemax = MAX_FN_LEN;

    statfs_buf->f_blocks = BLOCK_NUM;
    statfs_buf->f_bsize = BLOCK_SIZE;

    statfs_buf->f_files = (V_DISK_INDEX_SIZE - sizeof(super_block)) / sizeof(inode);
    statfs_buf->f_ffree = statfs_buf->f_files - super_block_buf->fs_inode_num;

    return 0;
}