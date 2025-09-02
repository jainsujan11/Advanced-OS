# Assignment 1: Adding a New System Call to the Linux Kernel

## Index
1. Adding the new system calls to the sources kernel code.
2. Complining the source Kernel code
3. Calling the respective system call using wrapper functions.
4. Demo Instructions

## 1. Adding the new system calls to the sources kernel code

This assignment adds two syscalls: `get_info_for_pid(pid, buffer)` and `get_info(buffer)`.

- Implementations (A1 directory):
	- `Kernel Updates and files 5.10.240/A1/get_info_for_pid.c`: collects parent PID, state, static priority, number of children, and siblings for a given PID. Exposed via `SYSCALL_DEFINE2(get_info_for_pid, pid_t pid, char __user *buffer)`.
	- `Kernel Updates and files 5.10.240/A1/get_info.c`: collects system-wide scheduler summary (tasks, runqueue stats, CFS min vruntime, load weight, latency). Exposed via `SYSCALL_DEFINE1(get_info, char __user *buffer)`.
   * Add the `Kernel Updates and files 5.10.240/A1` folder to the source directory.

- Prototypes declared in `Kernel Updates and files 5.10.240/syscalls.h`:
	- `asmlinkage long sys_get_info_for_pid(pid_t pid, char __user *buffer);`
	- `asmlinkage long sys_get_info(char __user *buffer);`
   * Copy and update these lines in the `<linux source>/include/linux/syscalls.h` 

- x86_64 syscall table entries in `Kernel Updates and files 5.10.240/syscall_64.tbl`:
	- `441  common  get_info_for_pid   sys_get_info_for_pid`
	- `442  common  get_info           sys_get_info`
   * Copy and update these lines in the `<linux source>/arch/x86/entry/syscalls/syscall_64.tbl`

- Build wiring for these objects: `Kernel Updates and files 5.10.240/A1/Makefile`
	- `core-y += <add the A1 folder>` to compile the code along side
	- `core-y += A1/`
   * Update these lines in the `<linux source>/Makefile`

Place the `A1` directory within the kernel source and ensure it’s included by the kernel build (the provided `A1/Makefile` selects both objects). The exported prototypes and syscall table updates complete the kernel-side integration.

## 2. Complining the source Kernel code

1. `make menuconfig` — open the kernel configuration UI and select required options (ensure A1 objects are included).
2. `make -j($nproc)` — compile the kernel using parallel jobs for faster builds.
3. `make modules_install -j($nproc)` — install built kernel modules under `/lib/modules`.
4. `make install -j($nproc)` — install the kernel image and update bootloader entries.

## 3. Calling the respective system call using wrapper functions.

We provide thin C wrappers in `Library_Wrapper_and_Test_Files/` so user programs can call the new syscalls directly and get typed results back.

- get_info_for_pid (per-process info)
	- Header: `Library_Wrapper_and_Test_Files/lib_get_info_for_pid.h`
	- API: `struct proc_info* get_info_for_pid(pid_t pid);`
	- Returns: `proc_info { parent_pid, state, static_prio, nr_children, nr_siblings }` on success; `NULL` on failure with `errno` set. Free with `free_proc_info(info)`.
	- Impl: `Library_Wrapper_and_Test_Files/lib_get_info_for_pid.c` (invokes syscall number 441 and parses a CSV into the struct). 

- get_info (scheduler/system summary)
	- Header: `Library_Wrapper_and_Test_Files/lib_get_info.h`
	- API: `struct sched_info* get_info(void);`
	- Returns: `sched_info { total_processes, nr_task_running, nr_task_interruptible, nr_task_uninterruptible, nr_rt_class, nr_fair_class, cfs_nr_running, min_vruntime_pid, min_vruntime, total_load_weight, target_latency_ms }` on success; `NULL` on failure with `errno` set. Free with `free_sched_info(info)`.
	- Note: `target_latency_ms` is converted to milliseconds in the wrapper.
	- Impl: `Library_Wrapper_and_Test_Files/lib_get_info.c` (invokes syscall number 442 and parses a CSV into the struct).

Build/test helpers for these wrappers are in `Library_Wrapper_and_Test_Files/Makefile` and `Library_Wrapper_and_Test_Files/run.sh` (see Demo Instructions).

## 4. Demo Instructions

From a Linux system running the patched kernel:

1. Build wrappers and tests (in `Library_Wrapper_and_Test_Files/`):
	 - `make`
2. Run the scheduler summary demo:
	 - `./get_info`
3. Run the per-process demo (replace `<PID>` with a target pid):
	 - `./get_info_for_pid <PID>`
4. Or use the helper script (optional PID argument):
	 - `./run.sh <PID>`



