#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define PROC_ENTRY "/proc/lkm_22CS10075_22CS30066"   

static char *cpyBuff(char *proc_buf, int32_t value)
{
    memcpy(proc_buf, &value, sizeof(value));
    return proc_buf;
}

static char *cpyBuff8(char *proc_buf, int8_t value)
{
    memcpy(proc_buf, &value, sizeof(value));
    return proc_buf;
}

int main()
{
    int fp, fpfail;
    int pid = getpid();
    char proc_buf[4];
    int32_t a = 0;

    // open
    if ((fp = open(PROC_ENTRY, O_RDWR)) < 0)
    {
        perror("open");
        return 1;
    }

    // try to open again
    if ((fpfail = open(PROC_ENTRY, O_RDWR)) < 0)
    {
        printf("[PID:%d] Expected: failed second open (%s)\n", pid, strerror(errno));
    }

    // invalid init (too large)
    a = 201;
    if (write(fp, cpyBuff8(proc_buf, (int8_t)a), 1) < 0)
    {
        printf("[PID:%d] Correctly rejected invalid init size %d\n", pid, a);
    }

    // valid init (size 3)
    a = 3;
    if (write(fp, cpyBuff8(proc_buf, (int8_t)a), 1) < 0)
    {
        printf("[PID:%d] Failed init size %d (check dmesg)\n", pid, a);
    }
    else
    {
        printf("[PID:%d] Queue initialized with size %d\n", pid, a);
    }

    // enqueue
    int vals[] = {13, 31, 16, 16, 30}; // includes duplicate and overflow
    for (int i = 0; i < 5; i++)
    {
        if (write(fp, cpyBuff(proc_buf, vals[i]), 4) < 0)
        {
            printf("[PID:%d] enqueue %d failed (expected if full)\n", pid, vals[i]);
        }
        else
        {
            printf("[PID:%d] enqueued %d\n", pid, vals[i]);
        }
    }

    // read all elements
    int buffer[100];
    ssize_t bytes_read = read(fp, buffer, sizeof(buffer));
    if (bytes_read < 0)
    {
        perror("read");
    }
    else
    {
        int num_elements = bytes_read / sizeof(int);
        printf("[PID:%d] Queue contents (FIFO): ", pid);
        for (int i = 0; i < num_elements; i++)
        {
            printf("%d ", buffer[i]);
        }
        printf("\n");
    }

    close(fp);
    return 0;
}
