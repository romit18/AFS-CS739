#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#ifdef HAVE_XATTR
#include <sys/xattr.h>
#endif /* HAVE_XATTR */
#ifdef linux
/* For pread()/pwrite()/utimensat() */
#define _XOPEN_SOURCE 700
#endif

#define ERRNO_NOOP -999

#include "unreliablefs_ops.h"
const char *fuse_op_name[] = {
    "getattr",
    "readlink",
    "mknod",
    "mkdir",
    "unlink",
    "rmdir",
    "symlink",
    "rename",
    "link",
    "chmod",
    "chown",
    "truncate",
    "open",
    "read",
    "write",
    "statfs",
    "flush",
    "release",
    "fsync",
#ifdef HAVE_XATTR
    "setxattr",
    "getxattr",
    "listxattr",
    "removexattr",
#endif /* HAVE_XATTR */
    "opendir",
    "readdir",
    "releasedir",
    "fsyncdir",
    "access",
    "creat",
    "ftruncate",
    "fgetattr",
    "lock",
#if !defined(__OpenBSD__)
    "ioctl",
#endif /* __OpenBSD__ */
#ifdef HAVE_FLOCK
    "flock",
#endif /* HAVE_FLOCK */
#ifdef HAVE_FALLOCATE
    "fallocate",
#endif /* HAVE_FALLOCATE */
#ifdef HAVE_UTIMENSAT
    "utimens",
#endif /* HAVE_UTIMENSAT */
    "lstat"
};

extern int error_inject(const char* path, fuse_op operation);

int unreliable_lstat(const char *path, struct stat *buf)
{
    printf("--------fuse log -------- calling lstat on fuse\n");
    int ret = error_inject(path, OP_LSTAT);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    memset(buf, 0, sizeof(struct stat));
    if (lstat(path, buf) == -1) {
        return -errno;
    }

    return 0;
}

int unreliable_getattr(const char *path, struct stat *buf)
{
    printf("--------fuse log -------- calling getattr on fuse, path:%s\n", path);

    int ret = error_inject(path, OP_GETATTR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }
    // std::string cpp_path = path;

    memset(buf, 0, sizeof(struct stat)); 

    int res = Getattr(unreliableAFS, path, buf);
    if (res < 0) {
        return res;
    }

    // if (lstat(path, buf) == -1) {
    //     return -errno;
    // }
    
    // return res;
    return 0;
}

int unreliable_readlink(const char *path, char *buf, size_t bufsiz)
{
    printf("--------fuse log -------- calling readlink on fuse\n");

    int ret = error_inject(path, OP_READLINK);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    ret = readlink(path, buf, bufsiz);
    if (ret == -1) {
        return -errno;
    }
    buf[ret] = 0;

    return 0;
}

int unreliable_mknod(const char *path, mode_t mode, dev_t dev)
{
    printf("--------fuse log -------- calling mknod on fuse\n");
    int ret = error_inject(path, OP_MKNOD);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    ret = mknod(path, mode, dev);    
    if (ret == -1) {
        return -errno;
    }

    return 0;
}

int unreliable_mkdir(const char *path, mode_t mode)
{
    printf("--------fuse log -------- calling mkdir on fuse\n");
    int ret = error_inject(path, OP_MKDIR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    ret = Mkdir(unreliableAFS, path, mode);
    if (ret == -1) {
        return -errno;
    }
    mkdir(path, mode);

    return 0;
}

int unreliable_unlink(const char *path)
{
    printf("--------fuse log -------- calling ulink on fuse\n");
    int ret = error_inject(path, OP_UNLINK);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    ret = Unlink(unreliableAFS, path);
    // ret = unlink(path); // Should be removed when we begin using RPC Unlink
    if (ret == -1) {
        return -errno;
    }
    ret = unlink(path);

    return 0;
}

int unreliable_rmdir(const char *path)
{
    printf("--------fuse log -------- calling rmdir on fuse\n");
    int ret = error_inject(path, OP_RMDIR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    ret = Rmdir(unreliableAFS, path); 
    if (ret == -1) {
        return -errno;
    }
    rmdir(path);

    return 0;
}

int unreliable_symlink(const char *target, const char *linkpath)
{
    printf("--------fuse log -------- calling symlink on fuse\n");
    int ret = error_inject(target, OP_SYMLINK);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    ret = symlink(target, linkpath);
    if (ret == -1) {
        return -errno;
    }

    return 0;
}

int unreliable_rename(const char *oldpath, const char *newpath)
{
    printf("--------fuse log -------- calling rename on fuse\n");
    int ret = error_inject(oldpath, OP_RENAME);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    ret = Rename(unreliableAFS, oldpath, newpath);
    // ret = rename(oldpath, newpath); // Should be removed when we begin using RPC Rename
    if (ret == -1) {
        return -errno;
    }
    ret = rename(oldpath, newpath);

    return 0;
}

int unreliable_link(const char *oldpath, const char *newpath)
{
    printf("--------fuse log -------- calling link on fuse\n");
    int ret = error_inject(oldpath, OP_LINK);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    ret = link(oldpath, newpath);
    if (ret < 0) {
        return -errno;
    }

    return 0;
}

int unreliable_chmod(const char *path, mode_t mode)
{
    printf("--------fuse log -------- calling chmod on fuse\n");
    int ret = error_inject(path, OP_CHMOD);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }
    
    ret = chmod(path, mode);
    if (ret < 0) {
        return -errno;
    }

    return 0;
}

int unreliable_chown(const char *path, uid_t owner, gid_t group)
{
    printf("--------fuse log -------- calling chown on fuse\n");
    int ret = error_inject(path, OP_CHOWN);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    ret = chown(path, owner, group);
    if (ret == -1) {
        return -errno;
    }

    return 0;
}

int unreliable_truncate(const char *path, off_t length)
{
    printf("--------fuse log -------- calling truncate on fuse\n");
    int ret = error_inject(path, OP_TRUNCATE);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    ret = truncate(path, length); 
    if (ret == -1) {
        return -errno;
    }

    return 0;
}

int unreliable_open(const char *path, struct fuse_file_info *fi)
{
    printf("--------fuse log -------- calling open on fuse\n");
    int ret = error_inject(path, OP_OPEN);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }
    printf("before Open\n");
    // ret = OpenM(unreliableAFS, path, fi);
    ret = Open(unreliableAFS, path, fi->flags);
    // ret = open(path, fi->flags);
    printf("after Open\n");
    if (ret == -1) {
        return -errno;
    }
    printf("rc was ok\n");
    fi->fh = (int64_t) ret;
    return 0;
    // return ret;
}

int unreliable_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *fi)
{
    printf("--------fuse log -------- calling read on fuse\n");
    int ret = error_inject(path, OP_READ);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    int fd;

    if (fi == NULL) {
	fd = open(path, O_RDONLY);
    } else {
	fd = fi->fh;
    }

    if (fd == -1) {
	return -errno;
    }

    ret = pread(fd, buf, size, offset);
    if (ret == -1) {
        ret = -errno;
    }

    if (fi == NULL) {
	close(fd);
    }

    return ret;
}

int unreliable_write(const char *path, const char *buf, size_t size,
                     off_t offset, struct fuse_file_info *fi)
{
    printf("--------fuse log -------- calling write on fuse\n");
    int ret = error_inject(path, OP_WRITE);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    int fd;
    (void) fi;
    if(fi == NULL) {
	fd = open(path, O_WRONLY);
    } else {
	fd = fi->fh;
    }

    if (fd == -1) {
	return -errno;
    }

    ret = pwrite(fd, buf, size, offset);
    if (ret == -1) {
        ret = -errno;
    }

    if(fi == NULL) {
        close(fd);
    }

    char * tmp_path = (char *) malloc(strlen(path) + 16);
    // printf("size of path is %d", strlen(path));
    // printf("size of path + 16 is %d", (strlen(path) + 16));
    // printf("path is %s", path);
    snprintf(tmp_path, (strlen(path) + 16), "%s.tmpwrittenfile", path);
    // printf("writing temp file to %s\n", tmp_path);
    int new_fd = open(tmp_path, O_CREAT | O_RDWR | 0777);
    close(new_fd);

    return ret;
}

int unreliable_statfs(const char *path, struct statvfs *buf)
{
    printf("--------fuse log -------- calling statfs on fuse\n");
    int ret = error_inject(path, OP_STATFS);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    ret = statvfs(path, buf);
    if (ret == -1) {
        return -errno;
    }

    return 0;
}

int unreliable_flush(const char *path, struct fuse_file_info *fi)
{
    printf("--------fuse log -------- calling flush on fuse\n");
    int ret = error_inject(path, OP_FLUSH);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    // ret = Close(unreliableAFS, path, dup(fi->fh));
    ret = close(dup(fi->fh));
    if (ret == -1) {
        return -errno;
    }

    return 0;
}

int unreliable_release(const char *path, struct fuse_file_info *fi)
{
    printf("--------fuse log -------- calling release on fuse\n");
    int ret = error_inject(path, OP_RELEASE);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    ret = Close(unreliableAFS, path, fi->fh);
    // ret = CloseM(unreliableAFS, path, fi);
    // ret = close(fi->fh);
    if (ret == -1) {
        return -errno;
    }

    return 0;    
}

int unreliable_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
    printf("--------fuse log -------- calling fsync on fuse\n");
    int ret = error_inject(path, OP_FSYNC);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    if (datasync) {
        ret = fdatasync(fi->fh);
        if (ret == -1) {
            return -errno;
        }
    } else {
        ret = fsync(fi->fh);
        if (ret == -1) {
            return -errno;
        }
    }

    return 0;
}

#ifdef HAVE_XATTR
int unreliable_setxattr(const char *path, const char *name,
                        const char *value, size_t size, int flags)
{
    printf("--------fuse log -------- calling setxattr on fuse\n");
    int ret = error_inject(path, OP_SETXATTR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

#ifdef __APPLE__
    ret = setxattr(path, name, value, size, 0, flags);
#else
    ret = setxattr(path, name, value, size, flags);
#endif /* __APPLE__ */
    if (ret == -1) {
        return -errno;
    }

    return 0;
}

int unreliable_getxattr(const char *path, const char *name,
                        char *value, size_t size)
{
    printf("--------fuse log -------- calling getxattr on fuse\n");
    int ret = error_inject(path, OP_GETXATTR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

#ifdef __APPLE__
    ret = getxattr(path, name, value, size, 0, XATTR_NOFOLLOW);
#else
    ret = getxattr(path, name, value, size);
#endif /* __APPLE__ */
    if (ret == -1) {
        return -errno;
    }
    
    return 0;
}

int unreliable_listxattr(const char *path, char *list, size_t size)
{
    int ret = error_inject(path, OP_LISTXATTR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

#ifdef __APPLE__
    ret = listxattr(path, list, size, XATTR_NOFOLLOW);
#else
    ret = listxattr(path, list, size);
#endif /* __APPLE__ */
    if (ret == -1) {
        return -errno;
    }
    
    return ret;
}

int unreliable_removexattr(const char *path, const char *name)
{
    int ret = error_inject(path, OP_REMOVEXATTR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

#ifdef __APPLE__
    ret = removexattr(path, name, XATTR_NOFOLLOW);
#else
    ret = removexattr(path, name);
#endif /* __APPLE__ */
    if (ret == -1) {
        return -errno;
    }
    
    return 0;   
}
#endif /* HAVE_XATTR */

int unreliable_opendir(const char *path, struct fuse_file_info *fi)
{
    printf("--------fuse log -------- calling opendir on fuse\n");
    int ret = error_inject(path, OP_OPENDIR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    // DIR *dir = opendir(path);

   DIR* dir = Opendir(unreliableAFS, path);

    if (!dir) {
        return -errno;
    }
    fi->fh = (int64_t) dir;

    return 0;    
}

int unreliable_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi)
{
    printf("--------fuse log -------- calling readdir on fuse, path: %s\n", path);
    int ret = error_inject(path, OP_READDIR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }
    (void) offset;
    (void) fi;
    //
    int res;
    res = Readdir(unreliableAFS, path, buf, filler);

    
    return res;
    // // ReadDir
    // char** bufs;
    // bufs = (char **)malloc(sizeof(char*)*65536);

    // res = Readdir(unreliableAFS, path, bufs);
    // printf("--------fuse log -------- readdir res: %d \n", res);
    // if (res < 0) {
    //     return res;
    // }
    // for (char* c = *bufs; c; c=*++bufs) {
    //     struct dirent de;
    //     memcpy(&de, &c[0], sizeof(de));
    //     struct stat st;
    //     memset(&st, 0, sizeof(st));
    //     // printf("--------fuse log -------- de_dname: %s\n", c);
    //     st.st_ino = de.d_ino;
    //     st.st_mode = de.d_type << 12;
    //     if (filler(buf, de.d_name, &st, 0)) {
    //         break;
    //     }
    // }
    
    // DIR* dir = opendir(path);

    // if(dir == NULL){
    //     return -1;
    // }

    // // DIR* directory = NULL;
    // // int open = Opendir(unreliableAFS, path, directory);
    // // if (directory == NULL) {
	// //     return -1;
    // // }
    // struct dirent *de;

    // (void) offset;
    // (void) fi;
    
  //  while ((de = readdir(dir)) != NULL) {
    //     struct stat st;
    //     memset(&st, 0, sizeof(st));
    //     st.st_ino = de->d_ino;
    //     st.st_mode = de->d_type << 12;
    //     if (filler(buf, de->d_name, &st, 0))
    //         break;
    // }
    // closedir(dir);
    // return 0;
}

int unreliable_releasedir(const char *path, struct fuse_file_info *fi)
{
    printf("--------fuse log -------- calling releasedir on fuse\n");
    int ret = error_inject(path, OP_RELEASEDIR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    DIR *dir = (DIR *) fi->fh;

    ret = closedir(dir);
    if (ret == -1) {
        return -errno;
    }
    
    return 0;    
}

int unreliable_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi)
{
    printf("--------fuse log -------- calling fsyncdir on fuse\n");
    int ret = error_inject(path, OP_FSYNCDIR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    DIR *dir = opendir(path);
    if (!dir) {
        return -errno;
    }

    if (datasync) {
        ret = fdatasync(dirfd(dir));
        if (ret == -1) {
            return -errno;
        }
    } else {
        ret = fsync(dirfd(dir));
        if (ret == -1) {
            return -errno;
        }
    }
    closedir(dir);

    return 0;
}

void *unreliable_init(struct fuse_conn_info *conn)
{
    printf("--------fuse log -------- calling init on fuse\n");
    return NULL;
}

void unreliable_destroy(void *private_data)
{

}

int unreliable_access(const char *path, int mode)
{
    printf("--------fuse log -------- calling access on fuse\n");
    int ret = error_inject(path, OP_ACCESS);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    ret = Access(unreliableAFS, path, mode); 
    if (ret == -1) {
        return -errno;
    }
    
    return 0;
}

int unreliable_create(const char *path, mode_t mode,
                      struct fuse_file_info *fi)
{
    printf("--------fuse log -------- calling create on fuse\n");
    int ret = error_inject(path, OP_CREAT);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    ret = Create(unreliableAFS, path, fi->flags, mode);
    // ret = open(path, fi->flags, mode);
    // ret = OpenM(unreliableAFS, path, fi);
    if (ret == -1) {
        return -errno;
    }
    fi->fh = (int64_t) ret;
    // printf("hoho %d\n", fi->fh);
    return 0;    
}

int unreliable_ftruncate(const char *path, off_t length,
                         struct fuse_file_info *fi)
{
    printf("--------fuse log -------- calling ftruncate on fuse\n");
    int ret = error_inject(path, OP_FTRUNCATE);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    ret = truncate(path, length);
    if (ret == -1) {
        return -errno;
    }
    
    return 0;    
}

int unreliable_fgetattr(const char *path, struct stat *buf,
                        struct fuse_file_info *fi)
{
    printf("--------fuse log -------- calling fgetattr on fuse\n");
    int ret = error_inject(path, OP_FGETATTR);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    ret = fstat((int) fi->fh, buf);
    if (ret == -1) {
        return -errno;
    }
    
    return 0;    
}

int unreliable_lock(const char *path, struct fuse_file_info *fi, int cmd,
                    struct flock *fl)
{
    printf("--------fuse log -------- calling lock on fuse\n");
    int ret = error_inject(path, OP_LOCK);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    ret = fcntl((int) fi->fh, cmd, fl);
    if (ret == -1) {
        return -errno;
    }

    return 0;
}

#if !defined(__OpenBSD__)
int unreliable_ioctl(const char *path, int cmd, void *arg,
                     struct fuse_file_info *fi,
                     unsigned int flags, void *data)
{
    int ret = error_inject(path, OP_IOCTL);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    ret = ioctl(fi->fh, cmd, arg);
    if (ret == -1) {
        return -errno;
    }
    
    return ret;
}
#endif /* __OpenBSD__ */

#ifdef HAVE_FLOCK
int unreliable_flock(const char *path, struct fuse_file_info *fi, int op)
{
    int ret = error_inject(path, OP_FLOCK);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    ret = flock(((int) fi->fh), op);
    if (ret == -1) {
        return -errno;
    }
    
    return 0;    
}
#endif /* HAVE_FLOCK */

#ifdef HAVE_FALLOCATE
int unreliable_fallocate(const char *path, int mode,
                         off_t offset, off_t len,
                         struct fuse_file_info *fi)
{
    int ret = error_inject(path, OP_FALLOCATE);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    int fd;
    (void) fi;

    if (mode) {
	return -EOPNOTSUPP;
    }

    if(fi == NULL) {
	fd = open(path, O_WRONLY);
    } else {
	fd = fi->fh;
    }

    if (fd == -1) {
	return -errno;
    }

    ret = fallocate((int) fi->fh, mode, offset, len);
    if (ret == -1) {
        return -errno;
    }

    if(fi == NULL) {
	close(fd);
    }
    
    return 0;    
}
#endif /* HAVE_FALLOCATE */

#ifdef HAVE_UTIMENSAT
int unreliable_utimens(const char *path, const struct timespec ts[2])
{
    int ret = error_inject(path, OP_UTIMENS);
    if (ret == -ERRNO_NOOP) {
        return 0;
    } else if (ret) {
        return ret;
    }

    /* don't use utime/utimes since they follow symlinks */
    ret = utimensat(0, path, ts, AT_SYMLINK_NOFOLLOW);
    if (ret == -1) {
        return -errno;
    }

    return 0;
}
#endif /* HAVE_UTIMENSAT */
