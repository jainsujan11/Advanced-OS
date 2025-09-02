#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // For pid_t
#include "lib_get_info_for_pid.h" // Make sure to include the correct header for proc_info

void print_process_info(pid_t pid) {
    struct proc_info* info = get_info_for_pid(pid);
    if (info == NULL) {
        perror("get_info_for_pid failed");
        return;
    }

    printf("Process info for PID %d:\n", pid);
    printf("Parent PID: %d\n", info->parent_pid);
    printf("State: %ld\n", info->state);
    printf("Static priority: %d\n", info->static_prio);
    printf("Number of children: %d\n", info->nr_children);
    printf("Number of siblings: %d\n", info->nr_siblings);

    free_proc_info(info);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <PID>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t target_pid = (pid_t)atoi(argv[1]);

    if (target_pid <= 0) {
        fprintf(stderr, "Invalid PID: %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    print_process_info(target_pid);

    return 0;
}
