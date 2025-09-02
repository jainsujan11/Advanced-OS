#ifndef LIBGETINFOFORPID_H
#define LIBGETINFOFORPID_H

struct proc_info {
    int  parent_pid;        // Parent process PID
    long state;             // Process state (numeric)
    int  static_prio;       // Static priority
    int  nr_children;       // Number of child processes
    int  nr_siblings;       // Number of sibling processes
};


struct proc_info* get_info_for_pid();
void free_proc_info(struct proc_info* info);

#endif 
