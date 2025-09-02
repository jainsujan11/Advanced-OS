// once check the locations 
#include <linux/kernel.h>       // Core kernel definitions
#include <linux/syscalls.h>     // SYSCALL_DEFINE macros
#include <linux/sched.h>        // struct task_struct, task state, scheduling functions
#include <linux/pid.h>          // PID APIs like find_get_pid(), pid_task()
#include <linux/rcupdate.h>     // RCU locking (rcu_read_lock/unlock)
#include <linux/spinlock.h>     // spin_unlock_irqsave and locking primitives
#include <linux/uaccess.h>      // copy_to_user and copy_from_user
#include <linux/list.h>         // list_for_each for sibling and children iteration
#include <linux/printk.h>       // printk for kernel logging
#include <linux/errno.h>        // Error codes like -ESRCH, -EFAULT, -EOVERFLOW

// #include <linux/get_info_for_pid.h>

#define BUFFER_SIZE 256

SYSCALL_DEFINE2(get_info_for_pid, pid_t, pid, char __user *, buffer)
{
    struct task_struct *task;
    struct pid *pid_struct;
    char kernel_buffer[BUFFER_SIZE];
    int len;

    int nr_children = 0;
    int nr_siblings = 0;
    struct list_head *child, *sibling;

    printk(KERN_INFO "I am in get_info_for_pid.\n");

    // Reference-counted lookup
    pid_struct = find_get_pid(pid);
    if (!pid_struct)
        return -ESRCH;

    rcu_read_lock();
    task = pid_task(pid_struct, PIDTYPE_PID);
    if (!task)
    {
        rcu_read_unlock();
        put_pid(pid_struct);
        return -ESRCH;
    }

    // Count number of child processes
    list_for_each(child, &task->children) {
        nr_children++;
    }

    // Count number of sibling processes
    list_for_each(sibling, &task->sibling) {
        nr_siblings++;
    }

    len = snprintf(kernel_buffer, sizeof(kernel_buffer),
                   "%d,%ld,%d,%d,%d",
                   task->real_parent ? task->real_parent->pid : 0, // Parent PID
                   task->state,                                    // State
                   task->static_prio,                              // Static priority
                   nr_children,                                    // # children
                   nr_siblings);                                   // # siblings

    rcu_read_unlock();
    put_pid(pid_struct); // Release reference

    if (len >= BUFFER_SIZE)
    {
        printk(KERN_WARNING "get_info_for_pid: Buffer overflow prevented.\n");
        return -EOVERFLOW;
    }

    if (copy_to_user(buffer, kernel_buffer, len + 1))
        return -EFAULT;

    printk(KERN_INFO "I am out of get_info_for_pid.\n");

    return len;
}
