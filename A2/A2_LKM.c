#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>
#include "map.h"

#define PROC_ENTRY "lkm_22CS10075_22CS30066"

static struct proc_dir_entry *myproc_entry;
static struct Map *m; // Global map to store per-process queues

// Handle write operations: queue creation (1 byte) or enqueue (4 bytes)
static ssize_t myproc_write(struct file *fp, const char __user *ubuf, size_t len, loff_t *off)
{
    int ret = -1;
    int value = 0;
    char kbuf4[4];
    int8_t qsize_byte = 0;

    if (!m) {
        printk(KERN_ALERT "[LKM:%d] map not initialized\n", current->pid);
        return -1;
    }

    spin_lock(&m->lock);
    struct Entry *e = getMapEntry(m, current->pid);
    if (!e) {
        spin_unlock(&m->lock);
        printk(KERN_ALERT "[LKM:%d] no entry\n", current->pid);
        return -1;
    }

    if ((e->queue == NULL && len != 1) || (e->queue != NULL && len != 4)) {
        spin_unlock(&m->lock);
        printk(KERN_ALERT "[LKM:%d] invalid size %zu\n", current->pid, len);
        return -EINVAL;
    }

    if (e->queue == NULL) {
        // Queue creation: read queue size (1 byte)
        if (copy_from_user(&qsize_byte, ubuf, 1)) {
            spin_unlock(&m->lock);
            return -EFAULT;
        }
        if (qsize_byte < 1 || qsize_byte > 100) {
            spin_unlock(&m->lock);
            printk(KERN_ALERT "[LKM:%d] bad queue size %d\n", current->pid, qsize_byte);
            return -EINVAL;
        }
        e->queue = initQueue(qsize_byte);
        if (!e->queue) {
            spin_unlock(&m->lock);
            return -ENOMEM;
        }
        printk(KERN_ALERT "[LKM:%d] queue initialized cap=%d\n", current->pid, qsize_byte);
        ret = 0;
    } else {
        // Enqueue operation: read 4-byte integer
        if (copy_from_user(kbuf4, ubuf, 4)) {
            spin_unlock(&m->lock);
            return -EFAULT;
        }
        memcpy(&value, kbuf4, sizeof(int));
        if (isFull(e->queue) || enqueue(e->queue, value) == -2) {
            spin_unlock(&m->lock);
            printk(KERN_ALERT "[LKM:%d] queue full\n", current->pid);
            return -EACCES;
        }
        printk(KERN_ALERT "[LKM:%d] enqueued %d\n", current->pid, value);
        ret = 4;
    }
    spin_unlock(&m->lock);
    return ret;
}

// Handle read operations: dequeue all elements from the queue
static ssize_t myproc_read(struct file *fp, char __user *ubuf, size_t len, loff_t *off)
{
    if (!m) return -1;

    spin_lock(&m->lock);
    struct Entry *e = getMapEntry(m, current->pid);
    if (!e || !e->queue) {
        spin_unlock(&m->lock);
        printk(KERN_ALERT "[LKM:%d] queue not ready\n", current->pid);
        return -EACCES;
    }
    if (isEmpty(e->queue)) {
        spin_unlock(&m->lock);
        printk(KERN_ALERT "[LKM:%d] queue empty\n", current->pid);
        return -EACCES;
    }

    size_t n = e->queue->size;
    size_t bytes = n * sizeof(int);
    if (len < bytes) {
        spin_unlock(&m->lock);
        printk(KERN_ALERT "[LKM:%d] buffer too small, need %zu\n", current->pid, bytes);
        return -EINVAL;
    }

    // Allocate temporary buffer for dequeued elements
    int *tmp = kmalloc(bytes, GFP_KERNEL);
    if (!tmp) {
        spin_unlock(&m->lock);
        return -ENOMEM;
    }

    int copied = dequeueAll(e->queue, tmp);
    spin_unlock(&m->lock);

    if (copy_to_user(ubuf, tmp, copied * sizeof(int))) {
        kfree(tmp);
        return -EFAULT;
    }
    kfree(tmp);
    return copied * sizeof(int);
}

// Open proc file - create entry for current process
static int myproc_open(struct inode *inode, struct file *file)
{
    if (!m) return -1;

    spin_lock(&m->lock);
    struct Entry *e = getMapEntry(m, current->pid);
    if (e) {
        spin_unlock(&m->lock);
        printk(KERN_ALERT "[LKM:%d] already open\n", current->pid);
        return -EAGAIN;
    }
    if (!createEntry(m, current->pid)) {
        spin_unlock(&m->lock);
        printk(KERN_ALERT "[LKM:%d] cannot create entry\n", current->pid);
        return -ENOMEM;
    }
    printk(KERN_ALERT "[LKM:%d] opened\n", current->pid);
    spin_unlock(&m->lock);

    try_module_get(THIS_MODULE); // Increment module reference count
    return 0;
}

// Clean up when proc file is closed
static int myproc_release(struct inode *inode, struct file *file)
{
    if (m) {
        spin_lock(&m->lock);
        removeEntry(m, current->pid);
        spin_unlock(&m->lock);
    }
    module_put(THIS_MODULE);
    printk(KERN_ALERT "[LKM:%d] closed\n", current->pid);
    return 0;
}

static const struct proc_ops myproc_fops = {
    .proc_read    = myproc_read,
    .proc_write   = myproc_write,
    .proc_open    = myproc_open,
    .proc_release = myproc_release,
};

// Module initialization
static int __init LKM_init(void)
{
    myproc_entry = proc_create(PROC_ENTRY, 0666, NULL, &myproc_fops);
    if (!myproc_entry) {
        printk(KERN_ALERT "[LKM] could not create /proc/%s\n", PROC_ENTRY);
        return -ENOMEM;
    }
    m = initMap();
    if (!m) {
        remove_proc_entry(PROC_ENTRY, NULL);
        printk(KERN_ALERT "[LKM] map init failed\n");
        return -1;
    }
    printk(KERN_ALERT "[LKM] /proc/%s ready\n", PROC_ENTRY);
    return 0;
}

// Module cleanup
static void __exit LKM_exit(void)
{
    destroyMap(m);
    remove_proc_entry(PROC_ENTRY, NULL);
    printk(KERN_ALERT "[LKM] /proc/%s removed\n", PROC_ENTRY);
}

module_init(LKM_init);
module_exit(LKM_exit);

MODULE_LICENSE("GPL");
