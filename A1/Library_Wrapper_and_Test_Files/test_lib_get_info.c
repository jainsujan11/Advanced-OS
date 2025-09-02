#include <stdio.h>
#include <stdlib.h>
#include "lib_get_info.h"  // Contains struct and function prototypes

int main() {
    struct sched_info* info = get_info();
    if (!info) {
        perror("get_info failed");
        return EXIT_FAILURE;
    }

    printf("Scheduler information:\n");
    printf("Total processes: %u\n", info->total_processes);
    printf("TASK_RUNNING: %u\n", info->nr_task_running);
    printf("TASK_INTERRUPTIBLE: %u\n", info->nr_task_interruptible);
    printf("TASK_UNINTERRUPTIBLE: %u\n", info->nr_task_uninterruptible);
    printf("RT class count: %u\n", info->nr_rt_class);
    printf("Fair class count: %u\n", info->nr_fair_class);
    printf("CFS runqueue count: %u\n", info->cfs_nr_running);
    printf("PID with min vruntime: %d\n", info->min_vruntime_pid);
    printf("Minimum vruntime: %llu\n", info->min_vruntime);
    printf("Total load weight: %llu\n", info->total_load_weight);
    printf("Target latency (ms): %u\n", info->target_latency_ms);

    free_sched_info(info);
    return 0;
}
