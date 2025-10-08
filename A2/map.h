#ifndef MAP_H_
#define MAP_H_

#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include "queue.h"

struct Entry {
    int pid;
    struct Queue *queue;
    struct Entry *next;
};

struct Map {
    int size;
    struct Entry *head;
    spinlock_t lock;     // protects map + per-entry queue operations
};

static inline struct Map *initMap(void)
{
    struct Map *m = kmalloc(sizeof(*m), GFP_KERNEL);
    if (!m) {
        printk(KERN_ALERT "[LKM] Failed to allocate memory for Map\n");
        return NULL;
    }
    m->size = 0;
    m->head = NULL;
    spin_lock_init(&m->lock);
    return m;
}

static inline void destroyMap(struct Map *m)
{
    if (!m) return;
    // free all entries
    struct Entry *cur;
    cur = m->head;
    while (cur) {
        struct Entry *nxt = cur->next;
        destroyQueue(cur->queue);
        kfree(cur);
        cur = nxt;
    }
    kfree(m);
}

static inline struct Entry *getMapEntry(struct Map *m, int pid)
{
    struct Entry *ptr = m->head;
    while (ptr) {
        if (ptr->pid == pid) return ptr;
        ptr = ptr->next;
    }
    return NULL;
}

static inline struct Entry *createEntry(struct Map *m, int pid)
{
    struct Entry *e = kmalloc(sizeof(*e), GFP_KERNEL);
    if (!e) {
        printk(KERN_ALERT "[LKM:%d] Failed to allocate memory for Entry\n", pid);
        return NULL;
    }
    e->pid = pid;
    e->queue = NULL;
    // insert at head
    e->next = m->head;
    m->head = e;
    m->size++;
    return e;
}

static inline void removeEntry(struct Map *m, int pid)
{
    struct Entry *prev = NULL, *cur = m->head;
    while (cur && cur->pid != pid) {
        prev = cur;
        cur = cur->next;
    }
    if (!cur) {
        printk(KERN_ALERT "[LKM:%d] Entry not found\n", pid);
        return;
    }
    if (!prev) m->head = cur->next;
    else prev->next = cur->next;
    destroyQueue(cur->queue);
    kfree(cur);
    m->size--;
}

static inline void printMap(struct Map *m)
{
    if (!m) return;
    if (m->size == 0) {
        printk(KERN_INFO "[LKM] Map is empty\n");
        return;
    }
    struct Entry *ptr;
    ptr = m->head;
    while (ptr) {
        printk(KERN_INFO "[LKM] <pid=%d>\n", ptr->pid);
        showQueue(ptr->queue);
        ptr = ptr->next;
    }
}

#endif /* MAP_H_ */
