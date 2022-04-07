
/* ffs */

#include "../include/define.h"
#include "../include/ffs_log.h"
#include "../include/ffs_oper.h"
#include "../include/v_disk.h"

static struct fuse_operations ffs_operations = {
    
    .getattr = ffs_getattr,
    // .symlink = 
    .read = ffs_read,
    .readdir = ffs_readdir,
    .mkdir = ffs_mkdir,
    .rmdir = ffs_rmdir,
    .mknod = ffs_mknod,
    .unlink = ffs_unlink,
    .write = ffs_write,
    .rename = ffs_rename,
    .chmod = ffs_chmod,
    .chown = ffs_chown,
    .truncate = ffs_truncate,
    .open = ffs_open,
    .statfs = ffs_statfs,
    
};

int main(int argc, char* argv[]) {
    /*
    int i, dir_cnt = 0;
    char** dir = malloc(233), file[1234];

    for (i = 0; i < 233; ++i) 
        dir[i] = malloc(666);

    printf("%d\n", sizeof(dir));

    split_path("/usr/local/lighthouse/softwares/fuse-playground/ffs", dir, &dir_cnt, file);

    for (i = 0; i < dir_cnt; ++i) {
        printf("dir: %s\n", dir[i]);
    }
    printf("file: %s\n", file);
    */
   
    FFS_DBG_INFO("FFS started successfully\n");

    open_image();

    int ret = fuse_main(argc, argv, &ffs_operations, NULL);
    
    close_image();

    FFS_DBG_INFO("FFS exited\n");
    
    return ret;
}