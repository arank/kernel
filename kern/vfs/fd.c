#include <types.h>
#include <fd.h>
#include <current.h>
#include <synch.h>
#include <vfs.h>

struct file_desc {
    struct vnode *vn;
    off_t offset;
    unsigned ref_count;
    mode_t mode;
    int flags;
    struct lock *lock;
};

struct file_desc *fd_init(struct vnode *vn, mode_t mode, int flags) {
    struct file_desc *fd = kmalloc(sizeof *fd);
    if (fd == NULL) return NULL;

    fd->lock = lock_create("fd_lock");
    if (fd->lock == NULL) { 
        kfree(fd);
        fd=NULL;
        return NULL;
    }

    fd->vn = vn;
    fd->offset = 0;
    fd->mode = mode;
    fd->ref_count = 1;
    fd->flags = flags;

    return fd;
}

void fd_destroy(struct file_desc *fd) {
    lock_acquire(fd->lock);
    if (fd->ref_count == 1) {
        lock_destroy(fd->lock);
        vfs_close(fd->vn);
        kfree(fd);
    } else {
        fd->ref_count--;
    }
    lock_release(fd->lock);
}
