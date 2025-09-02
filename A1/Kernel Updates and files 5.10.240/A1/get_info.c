// once check the locations 
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/uaccess.h>
#include "../kernel/sched/sched.h"
// #include <linux/get_info.h>

#define BUFFER_SIZE 256

SYSCALL_DEFINE1(get_info, char __user *, buffer)
{
    struct task_struct *task;
    struct rq *rq;
    struct cfs_rq *cfs_rq;
    struct sched_entity *se;
    struct rb_node *node;
    int cpu;
    unsigned long flags;
    int nr_interruptible = 0;
    int nr_uninterruptible = 0;
    int total_tasks = 0;
    char kernel_buffer[BUFFER_SIZE];
    int len;

    printk(KERN_INFO "I am in get_info.\n");

    // Step 1: Get current CPU and runqueue
    cpu = smp_processor_id();
    rq = cpu_rq(cpu);
    cfs_rq = &rq->cfs;

    // Step 2: Lock runqueue to safely read scheduler data
    raw_spin_lock_irqsave(&rq->lock, flags);

    // Gather scheduler stats
    int nr_task_running = rq->nr_running;
    int nr_rt_class = rq->rt.rt_nr_running;
    int nr_fair_class = cfs_rq->nr_running;
    int cfs_nr_running = cfs_rq->nr_running;
    u64 total_load_weight = cfs_rq->load.weight;
    u32 target_latency_ms = sysctl_sched_latency / (cfs_rq->nr_running ? cfs_rq->nr_running : 1);

    pid_t min_vruntime_pid = 0;
    u64 min_vruntime = 0;

    // Find process with min vruntime in CFS runqueue
    node = rb_first_cached(&cfs_rq->tasks_timeline);
    if (node) {
        se = rb_entry(node, struct sched_entity, run_node);
        task = container_of(se, struct task_struct, se);
        min_vruntime_pid = task->pid;
        min_vruntime = se->vruntime;
    }

    raw_spin_unlock_irqrestore(&rq->lock, flags);

    // Step 3: Count interruptible and uninterruptible tasks, and print all PIDs
    rcu_read_lock();
    for_each_process(task) {
        total_tasks++;
        printk(KERN_INFO "Process PID: %d\n", task->pid);
        if (task->state == TASK_INTERRUPTIBLE)
            nr_interruptible++;
        else if (task->state == TASK_UNINTERRUPTIBLE)
            nr_uninterruptible++;
    }
    rcu_read_unlock();

    // Step 4: Format all information into kernel_buffer as CSV
    // Order: total_processes, nr_task_running, nr_task_interruptible, nr_task_uninterruptible,
    // nr_rt_class, nr_fair_class, cfs_nr_running, min_vruntime_pid, min_vruntime, total_load_weight, target_latency_ms
    len = snprintf(kernel_buffer, sizeof(kernel_buffer),
                   "%d,%d,%d,%d,%d,%d,%d,%d,%llu,%llu,%u",
                   total_tasks,
                   nr_task_running,
                   nr_interruptible,
                   nr_uninterruptible,
                   nr_rt_class,
                   nr_fair_class,
                   cfs_nr_running,
                   min_vruntime_pid,
                   min_vruntime,
                   total_load_weight,
                   target_latency_ms);

    if (len >= BUFFER_SIZE) {
        printk(KERN_WARNING "get_info: Buffer overflow prevented.\n");
        return -EOVERFLOW;
    }

    // Step 5: Copy formatted info to user buffer
    if (copy_to_user(buffer, kernel_buffer, len + 1))
        return -EFAULT;

    printk(KERN_INFO "I am out of get_info.\n");

    return len;
}
