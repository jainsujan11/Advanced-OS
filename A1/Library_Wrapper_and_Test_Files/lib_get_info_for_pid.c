#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib_get_info_for_pid.h"

#define SYS_get_info_for_pid 441
#define BUFFER_SIZE 256

struct proc_info* get_info_for_pid(pid_t pid) {
    char buffer[BUFFER_SIZE];
    struct proc_info* info = (struct proc_info*)malloc(sizeof(struct proc_info));

    if (!info) {
        errno = ENOMEM;
        return NULL;
    }

    long result = syscall(SYS_get_info_for_pid, pid, buffer);
    if (result < 0) {
        free(info);
        errno = -result;
        return NULL;
    }

    if (sscanf(buffer, "%d,%ld,%d,%d,%d",
               &info->parent_pid,
               &info->state,
               &info->static_prio,
               &info->nr_children,
               &info->nr_siblings) != 5) {
        free(info);
        errno = EINVAL;
        return NULL;
    }

    return info;
}

void free_proc_info(struct proc_info* info) {
    free(info);
}
