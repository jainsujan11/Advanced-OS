#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib_get_info.h"

#define SYS_get_info 442  
#define BUFFER_SIZE 512

struct sched_info* get_info() {
    char buffer[BUFFER_SIZE];
    struct sched_info* info = (struct sched_info*)malloc(sizeof(struct sched_info));
    if (!info) {
        errno = ENOMEM;
        return NULL;
    }

    long result = syscall(SYS_get_info, buffer);
    if (result < 0) {
        free(info);
        errno = -result;
        return NULL;
    }
    if(sscanf(buffer, "%u,%u,%u,%u,%u,%u,%u,%d,%llu,%llu,%u",
              &info->total_processes,
              &info->nr_task_running,
              &info->nr_task_interruptible,
              &info->nr_task_uninterruptible,
              &info->nr_rt_class,
              &info->nr_fair_class,
              &info->cfs_nr_running,
              &info->min_vruntime_pid,
              &info->min_vruntime,
              &info->total_load_weight,
              &info->target_latency_ms) != 11) {
        free(info);
        errno = EINVAL;
        return NULL;
    }
    
    info->target_latency_ms/=1000000;
    
    return info;
}

void free_sched_info(struct sched_info* info) {
    free(info);
}
    
