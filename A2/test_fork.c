#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
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

static void run_test_logic()
{
    int fp, pid = getpid();
    char proc_buf[4];
    int32_t a;

    if ((fp = open(PROC_ENTRY, O_RDWR)) < 0) {
        fprintf(stderr, "[PID:%d] open failed: %s\n", pid, strerror(errno));
        exit(1);
    }

    // Initialize queue with size 3
    a = 3;
    if (write(fp, cpyBuff8(proc_buf, (int8_t)a), 1) < 0)
        fprintf(stderr, "[PID:%d] init failed: %s\n", pid, strerror(errno));
    else
        printf("[PID:%d] queue initialized size %d\n", pid, a);

    // Each process enqueues different pattern (PID-based)
    int vals[3];
    vals[0] = pid % 100;       // unique-ish values
    vals[1] = vals[0] + 1;
    vals[2] = vals[0] + 2;

    for (int i = 0; i < 3; i++) {
        if (write(fp, cpyBuff(proc_buf, vals[i]), 4) < 0)
            printf("[PID:%d] enqueue %d failed: %s\n", pid, vals[i], strerror(errno));
        else
            printf("[PID:%d] enqueued %d\n", pid, vals[i]);
    }

    // Read back
    int buffer[100];
    ssize_t bytes_read = read(fp, buffer, sizeof(buffer));
    if (bytes_read < 0)
        printf("[PID:%d] read failed: %s\n", pid, strerror(errno));
    else {
        int num_elements = bytes_read / sizeof(int);
        printf("[PID:%d] dequeued:", pid);
        for (int i = 0; i < num_elements; i++)
            printf(" %d", buffer[i]);
        printf("\n");
    }

    close(fp);
}

int main()
{
    // create multiple processes
    fork();
    fork();
    fork();

    // Each process runs its own test logic
    run_test_logic();
    for(int i = 0;i < 3; i++) wait(NULL);
    return 0;
}
