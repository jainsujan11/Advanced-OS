#ifndef QUEUE_H_
#define QUEUE_H_

#include <linux/slab.h>
#include <linux/kernel.h>

struct Queue {
    int *values;     // circular buffer storage
    int front;       // index of current front element
    int rear;        // index one past the last element
    int size;        // current number of elements
    int maxsize;     // capacity
};

static inline struct Queue *initQueue(int maxsize)
{
    struct Queue *q = kmalloc(sizeof(*q), GFP_KERNEL);
    if (!q) {
        printk(KERN_ALERT "[LKM] Failed to allocate memory for queue\n");
        return NULL;
    }
    q->values = kmalloc_array(maxsize, sizeof(int), GFP_KERNEL);
    if (!q->values) {
        printk(KERN_ALERT "[LKM] Failed to allocate memory for queue values\n");
        kfree(q);
        return NULL;
    }
    q->front = 0;
    q->rear  = 0;
    q->size = 0;
    q->maxsize = maxsize;
    return q;
}

static inline void destroyQueue(struct Queue *q)
{
    if (q) {
        kfree(q->values);
        kfree(q);
    }
}

static inline int isFull(struct Queue *q)
{
    return q->size == q->maxsize;
}

static inline int isEmpty(struct Queue *q)
{
    return q->size == 0;
}

// enqueue a value; returns 0 on success, -2 if full
static inline int enqueue(struct Queue *q, int value)
{
    if (isFull(q)) return -2;
    q->values[q->rear] = value;
    q->rear = (q->rear + 1) % q->maxsize;
    q->size++;
    return 0;
}

// Copies all elements in FIFO order into 'out' and empties the queue.
// Returns number of elements copied.
static inline int dequeueAll(struct Queue *q, int *out)
{
    int n = q->size;
    int i;
    for (i = 0; i < n; i++) {
        out[i] = q->values[(q->front + i) % q->maxsize];
    }
    // reset queue
    q->front = 0;
    q->rear = 0;
    q->size = 0;
    return n;
}

static inline void showQueue(struct Queue *q)
{
    if (!q) {
        printk(KERN_INFO "[LKM] (null queue)\n");
        return;
    }
    if (isEmpty(q)) {
        printk(KERN_INFO "[LKM] Queue is empty\n");
        return;
    }
    printk(KERN_INFO "[LKM] Queue(size=%d, cap=%d): ", q->size, q->maxsize);
    int i;
    for (i = 0; i < q->size; i++) {
        int v = q->values[(q->front + i) % q->maxsize];
        printk(KERN_CONT "%d ", v);
    }
    printk(KERN_CONT "\n");
}

#endif /* QUEUE_H_ */
