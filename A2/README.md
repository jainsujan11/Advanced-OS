
# Assignment 2 – LKM Queue via /proc
Sujan Jain - 22CS10075 
Devanshu Agarwal - 22CS30066

This solution implements a per-process FIFO **Queue** in a Linux Kernel Module (tested for 5.10.x headers), exposing a `/proc` interface for user-space interaction.

## Files
- `A2_LKM.c` – Kernel module implementing `/proc/lkm_<rollnos>`
- `map.h`    – Simple PID→Entry map with a global spinlock (also protects queue ops)
- `queue.h`  – Circular FIFO queue implementation
- `Makefile` – Build the module
- Test files + Verifier
- Output Screenshot 


## Build
```bash
make
```

This produces `A2_LKM.ko`.

## Insert
```bash
sudo insmod A2_LKM.ko
```

## Interaction Specifications
1. **Open** (RW): `fd = open("/proc/lkm_<rollnos>", O_RDWR)`
2. **Initialize queue**: write **1 byte** `N` (1 ≤ N ≤ 100). Returns 0 on success.
3. **Enqueue**: write **4 bytes** (a 32-bit integer). Returns **4** on success; `-EACCES` if full; `-EINVAL` on wrong size.
4. **Dequeue**: read returns all elements in **FIFO** order and empties the queue. Returns `#elems * 4` bytes. `-EACCES` if empty.
5. **Close**: `close(fd)` — resources freed for that PID.

## Concurrency
- A global `spinlock_t` in `Map` guards the PID map and queue operations for simplicity and correctness under concurrency.
- Prevents multiple opens by the same PID: `open()` returns `-EAGAIN` if already open.

## Testing Notes
- We have made 2 test files for LKM testing 
    1) test_basic.c -- It checks basic functions of LKM module. We have also provided verifier for it 
    2) test_fork.c -- It checks for the concurrency part of queue. It check for data race conditions. Since it is concurrent, we have 
                        to manually check if its output is correct or not. 

```bash
gcc test_basic.c 
./a.out 
```

## Cleanup
```bash
sudo rmmod A2_LKM
make clean
```
