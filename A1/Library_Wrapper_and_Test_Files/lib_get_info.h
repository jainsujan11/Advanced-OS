#ifndef LIBGETINFO_H
#define LIBGETINFO_H

struct sched_info {
    unsigned int total_processes;         // Total number of processes in system
    unsigned int nr_task_running;         // Number of processes in TASK_RUNNING
    unsigned int nr_task_interruptible;   // Number of processes in TASK_INTERRUPTIBLE
    unsigned int nr_task_uninterruptible; // Number of processes in TASK_UNINTERRUPTIBLE
    unsigned int nr_rt_class;              // Number of processes in RT class
    unsigned int nr_fair_class;            // Number of processes in fair (CFS) class
    unsigned int cfs_nr_running;           // Number of processes in CFS runqueue
    pid_t min_vruntime_pid;                 // PID with minimum vruntime in CFS runqueue
    unsigned long long min_vruntime;       // Minimum vruntime value
    unsigned long long total_load_weight;  // Total load weight of CFS runqueue
    unsigned int target_latency_ms;         // Current target latency in milliseconds
};



struct sched_info* get_info();
void free_sched_info(struct sched_info* info);

#endif 
