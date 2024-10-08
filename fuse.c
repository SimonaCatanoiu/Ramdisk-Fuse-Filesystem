#define _XOPEN_SOURCE 700
#define FUSE_USE_VERSION 29
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fuse.h>
#include <errno.h>

#include "disk_image.h"
#include "utils.h"
#include "log.h"
#include "blocks.h"
#include "bitmap.h"
#include "inode.h"
#include "dentry.h"

FILE *log_fis;
char disk_iso_name[MAX_FILENAME];

void *fs_init(struct fuse_conn_info *conn)
{
    // function that initialise the file system
    disk_mount_the_filesystem(disk_iso_name);
    log_message("Mounted the filesystem\n");
    return NULL;
}

int fs_access(const char *path, int mask)
{
    // function that checks the existance of the inode for a dir or a file
    // it updates the timestamps
    int ret_value = disk_access(path);
    if (ret_value == -1)
    {
        return -ENOENT;
    }
    // check if the user has the correct permissions to access the file
    if (mask != F_OK)
    {
        int ret_value = disk_check_permissions(path, mask);
        if (ret_value == -1)
        {
            return -EACCES;
        }
    }
    return 0;
}

int fs_getattr(const char *path, struct stat *stbuf)
{
    int result = disk_get_attributes_from_path(path, stbuf);
    // we add +1 to display the correct inode because in our filesystem inodes start from 0.
    // In any other filesystem, they start from 1.
    stbuf->st_ino = stbuf->st_ino + 1;
    return result;
}

int fs_mknod(const char *path, mode_t mode, dev_t rdev)
{
    int result = disk_mknod(path, mode);
    return result;
}

int fs_chmod(const char *path, mode_t mode)
{
    int result = disk_chmod(path, mode);
    return result;
}

int fs_rename(const char* from, const char* to)
{
    int result = disk_rename(from, to);
    return result;
}

int fs_truncate(const char *path, off_t size)
{
    int result = disk_truncate(path, size);
    return result;
}

int fs_link(const char *from, const char *to)
{
    int result = disk_link(from, to);
    return result;
}

int fs_mkdir(const char *path, mode_t mode)
{
    int result = disk_mkdir(path, mode);
    return result;
}

int fs_create(const char *path, mode_t mode, struct fuse_file_info *info)
{
    dev_t dev_whatever = 0;
    return fs_mknod(path, mode, dev_whatever);
}

int fs_utimens(const char *path, const struct timespec times[2])
{
    int result = disk_change_utimens(path, times);
    return result;
}

int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
    int result = disk_readdir(path, buf, filler);
    return result;
}

int fs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int result = disk_write(path,buf,size,offset);
    return result;
}

int fs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int result = disk_read(path,buf,size,offset);
    return result;
}

int fs_unlink(const char* path)
{
    int result = disk_unlink(path);
    return result;
}

int fs_rmdir(const char* path)
{
    int result = disk_rmdir(path);
    return result;
}

int fs_symlink(const char *from, const char *to)
{
    int result = disk_symlink(from,to);
    return result;
}

int fs_readlink(const char *path, char *buf, size_t size)
{
    int result = disk_readlink(path,buf,size);
    return result;
}

static struct fuse_operations operations =
    {
        .init = fs_init,
        .access = fs_access,
        .getattr = fs_getattr,
        .mknod = fs_mknod,
        .chmod = fs_chmod,
        .rename = fs_rename,
        .truncate = fs_truncate,
        .link = fs_link,
        .mkdir = fs_mkdir,
        .create = fs_create,
        .utimens = fs_utimens,
        .readdir = fs_readdir,
        .write = fs_write,
        .read = fs_read,
        .unlink = fs_unlink,
        .rmdir = fs_rmdir,
        .symlink = fs_symlink,
        .readlink = fs_readlink
};

int main(int argc, char *argv[])
{
    if(!(argc==6 ||argc ==7))
    {
        printf("Incorect number of arguments!\n");
        return 0;
    }
    strcpy(disk_iso_name,argv[--argc]);
    log_fis = log_open();
    fuse_main(argc, argv, &operations, NULL);
    return 0;
}
