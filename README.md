# OperatingSystem-jos
 jos lab, MIT 6.828
## Lab4
In this lab I implemented multitasking among multiple simultaneously active user-mode environments, including multiprocessor support, round-robin scheduling, basic environment management system calls, Unix-like fork function( allowing a user-mode environment to create copies of itself ) and support for inter-process communication( allows user environments to communicate and synchronize with each others ).
### Part 1: Multiprocessor Support and Cooperative Multitasking
-  Implement mmio_map_region in kern/pmap.c: map physcal address from [pa,pa+size), after aligning address, the address space is from [ROUNDDOWN(pa),ROUNDUP(pa+size)). Then using boot_map_region() to allocate this region and map device memory to it.
-  In order to avoid adding the page at MPENTRY_PADDR to the free list, we should mark the address space of MPENTRY_PADDR is_used. The address space of MPENTRY_PADDR is [mpentry_start, mpentry_end. 
-  Modify mem_init_mp() (in kern/pmap.c) to map per-CPU stacks starting at KSTACKTOP, as shown:
```
//KSTACKTOP ---->  +------------------------------+ 0xefc00000      --+
//                 |     CPU0's Kernel Stack      | RW/--  KSTKSIZE   |
//                 | - - - - - - - - - - - - - - -|                   |
//                 |      Invalid Memory (*)      | --/--  KSTKGAP    |
//                 +------------------------------+                   |
//                 |     CPU1's Kernel Stack      | RW/--  KSTKSIZE   |
//                 | - - - - - - - - - - - - - - -|                 PTSIZE
//                 |      Invalid Memory (*)      | --/--  KSTKGAP    |
//                 +------------------------------+                   |
//                 :              .               :                   |
//                 :              .               :                   |
```
I map virtual address **KSTACKTOP-i\*(KSTKSIZE+KSTKGAP)+[KSTKGAP,KSTKGAP+KSTKSIZE)** to physical address **percpu_kstacks[i][KSTKSIZE]** for each CPU.
- The code in trap_init_percpu() (kern/trap.c) initializes the TSS and TSS descriptor for the BSP. It worked in Lab 3, but is incorrect when running on other CPUs:
  - cpus[i].cpu_ts = &gdt[(GD_TSS0 >> 3) + i]
  - Replace **ts** with **thiscpu->cpu_ts**
- Apply the big kernel lock in following places:
  - In i386_init(), acquire the lock before the BSP wakes up the other CPUs.
  - In mp_main(), acquire the lock after initializing the AP, and then call sched_yield() to start running environments on this AP.
  - In trap(), acquire the lock when trapped from user mode. To determine whether a trap happened in user mode or in kernel mode, check the low bits of the tf_cs.
  - In env_run(), release the lock right before switching to user mode. Do not do that too early or too late, otherwise you will experience races or deadlocks.
  By calling lock_kernel() and unlock_kernel() at the proper locations
- Implement sched_yield function in kern/sched.c:
  - Search sequentially through the envs[] array
  - Choose the first environment which status is **ENV_RUNNABLE**
  - Call env_run function to jump into that environmentun 
  - Use **ENV_CREATE(user_yield, ENV_TYPE_USER)** in i386_init function(in kern.init.c) to create more than 3 environments(just repeat that function call for more than 3 times).
- Implement system call functions in kern/syscall.c:
  - sys_exofork(): Allocate a new environment and returns envid of new environment
  - sys_env_set_status(envid_t envid, int status): Set envid's env_status to status(ENV_RUNNABLE/ENV_NOT_RUNNABLE) and returns 0 on success, < 0 on error(-E_BAD_ENV if environment envid doesn't currently exist or the caller doesn't have permission to change envid, -E_INVAL if status is not a valid status for an environment.)
  - sys_page_alloc(envid_t envid, void *va, int perm): Allocate a page of memory and map it at 'va' with permission 'perm' in the address space of 'envid'. If a page is already mapped at 'va', that page will be unmapped as a side effect.
  - sys_page_mapenvid_t srcenvid, void *srcva, envid_t dstenvid, void *dstva, int perm): Map the page of memory at 'srcva' in srcenvid's address space at 'dstva' in dstenvid's address space with permission 'perm'.
  - sys_page_unmap(envid_t envid, void *va): Unmap the page of memory at 'va' in the address space of 'envid'. If no page is mapped, the function silently succeeds.

### Part 2: Copy-on-Write Fork
- Implement sys_env_set_pgfault_upcall(envid_t envid, void *func) in kern/syscall.c: Set the page fault upcall for 'envid' by modifying the corresponding struct Env's 'env_pgfault_upcall' field, set env->env_pgfault_upcall to func.
- Implement the code in page_fault_handler(struct Trapframe *tf) in kern/trap.c: Call the environment's page fault upcall by calling curenv->env_pgfault_upcall, if curenv->env_pgfault_upcall is not null, set up a page fault stack frame on the user exception stack (below UXSTACKTOP).Storing the fault_va and current data of tf into utf. Then branch to curenv->env_pgfault_upcall by calling env_run function.
- Implement the _pgfault_upcall routine in lib/pfentry.S: Pushing old eip to old stack and seting utf->utf_esp = old stack bottom - 4. Then restoring the trap-time registers. Then restoring eflags from the stack. Then switching back to the adjusted trap-time stack and return to re-execute the instruction that faulted.
- Implement set_pgfault_handler(void (*handler)(struct UTrapframe *utf)) in lib/pgfault.c: This function is for user environment. If there isn't any page fault handler function yet, _pgfault_handler will be 0. We need to allocate an exception stack (one page of memory with its top at UXSTACKTOP), and tell the kernel to call the assembly-language _pgfault_upcall routine when a page fault occurs by calling sys_env_set_pgfault_upcall function. Otherwise, just set _pgfault_handler to parameter handler.
- Implement pgfault, duppage and fork in lib/fork.c:
  - pgfault(struct UTrapframe *utf): First, checking that the faulting access was a write or a copy-on-write page, if not, panic. Then allocating a new page and mapping it at a temporary location (PFTEMP). Copying the data from the old page to the new page, then moving the new page to the old page's address.
  - duppage(envid_t envid, unsigned pn): Mapping virtual page pn (address pn*PGSIZE) into the target envid at the same virtual address. If the page is writable or copy-on-write, the new mapping must be created copy-on-write, and then the mapping must be marked copy-on-write as well.
  - fork(void): Calling set_pgfault_handler function to set up page fault handler. Then using sys_exofork function to create a child and using duppage function to copy parent's address space and page fault handler to child.
### Part 3: Preemptive Multitasking and Inter-Process communication
- Modify kern/trapentry.S and kern/trap.c to initialize the appropriate entries in the IDT and provide handlers for IRQs 0 through 15. Then modify the code in env_alloc() in kern/env.c to ensure that user environments are always run with interrupts enabled:
  - Hardware IRQ numbers are partly defined in inc/trap.h
  - Declaring and defining entries in kern/trap.c
  - Using TRAPHANDLER_NOEC to generate entry points for the different traps in kern/trapentry.S
- Modify the kernel's trap_dispatch() function so that it calls sched_yield(). 
- Implement sys_ipc_recv and sys_ipc_try_send in kern/syscall.c:
  - sys_ipc_recv(void *dstva): Return -E_INVAL if dstva < UTOP but dstva is not page-aligned. Setting env_ipc_recving to **1** to signal that you want to receive. Marking yourself not runnable by setting env_status to **ENV_NOT_RUNNABLE**. Then calling sched_yield funtion to give up the CPU.
  - Updating Env's properties as follows:
    - env_ipc_recving is set to false(0) to block future sends;
    - env_ipc_from is set to the sending envid;
    - env_ipc_value is set to the 'value' parameter;
    - env_ipc_perm is set to 'perm' if a page was transferred, 0 otherwise.
    - env_status is set to ENV_RUNNABLE
  - Returns 0 on success, < 0 on error.Errors are:
    - -E_BAD_ENV if environment envid doesn't currently exist.
    - -E_IPC_NOT_RECV if envid is not currently blocked in sys_ipc_recv, or another environment managed to send first.
    - -E_INVAL if srcva < UTOP but srcva is not page-aligned.
    - -E_INVAL if srcva < UTOP and perm is inappropriate.
    - -E_INVAL if srcva < UTOP but srcva is not mapped in the caller's address space.
    - -E_INVAL if (perm & PTE_W), but srcva is read-only in the current environment's address space.
    - -E_NO_MEM if there's not enough memory to map srcva in envid's address space.
  - Finally adding branches correponding to sys_ipc_recv and sys_ipc_try_send in kern/syscall.c:syscall(uint32_t syscallno, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
