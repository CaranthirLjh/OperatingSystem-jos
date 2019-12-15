# OperatingSystem-jos
 jos lab, MIT 6.828
## Lab3
In this lab I implemented the basic kernel facilities required to get a protected user-mode environment running. I enhanced the JOS kernel to set up the data structures to keep track of user environments, create a single user environment, load a program image into it, and start it running.<br>
I also make the JOS kernel capable of handling any system calls the user environment makes and handling any other exceptions it causes.
### Part 1: User Environments and Exception Handling
File inc/env.h contains basic definitions for user environments in JOS. In kern/env.c, the kernel maintains three main global variables pertaining to environments:
```
  struct Env *envs = NULL;		// All environments
  struct Env *curenv = NULL;		// The current env
  static struct Env *env_free_list;	// Free environment list
```
Once JOS gets up and running, the ***envs*** pointer points to an array of Env structures representing all the environments in the system. In my design, the JOS kernel will support a maximum of ***NENV***(NENV is a constant #define'd in inc/env.h). Once it is allocated, the envs array will contain a single instance of the Env data structure for each of the NENV possible environments. The JOS kernel keeps all of the inactive Env structures on the ***env_free_list***. This design allows easy allocation and deallocation of environments, as they merely have to be added to or removed from the free list. The Env structure is defined in inc/env.h:
```
  struct Env {
    struct Trapframe env_tf;	// Saved registers
    struct Env *env_link;		// Next free Env
    envid_t env_id;			// Unique environment identifier
    envid_t env_parent_id;		// env_id of this env's parent
    enum EnvType env_type;		// Indicates special system environments
    unsigned env_status;		// Status of the environment
    uint32_t env_runs;		// Number of times environment has run

    // Address space
    pde_t *env_pgdir;		// Kernel virtual address of page dir
  };
```
I modified mem_init() in kern/pmap.c to allocate and map the envs array. This array consists of exactly NENV instances of the Env structure allocated.<br>
I implemented the code in kern/env.c necessary to run a user environment by finishing the following function:
```
env_init()：
  Initialize all of the Env structures in the envs array and add them to the env_free_list. Also calls env_init_percpu, which configures the segmentation hardware with separate segments for privilege level 0 (kernel) and privilege level 3 (user).
env_setup_vm()：
  Allocate a page directory for a new environment and initialize the kernel portion of the new environment's address space.
region_alloc()：
  Allocates and maps physical memory for an environment
load_icode()：
  Parse an ELF binary image, much like the boot loader already does, and load its contents into the user address space of a new environment.
env_create()：
  Allocate an environment with env_alloc and call load_icode to load an ELF binary into it.
env_run()：
  Start a given environment running in user mode.
```
I set up the IDT to handle interrupt vectors 0-31 (the processor exceptions). Each exception or interrupt should have its own handler in trapentry.S and trap_init() should initialize the IDT with the addresses of these handlers. Each of the handlers should build a struct Trapframe (see inc/trap.h) on the stack and call trap() (in trap.c) with a pointer to the Trapframe. trap() then handles the exception/interrupt or dispatches to a specific handler function.
### Part 2: Page Faults, Breakpoints Exceptions, and System Calls
I modify trap_dispatch() to dispatch:
- page fault exceptions(T_PGFLT) to page_fault_handler()
- breakpoint exception(T_BRKPT) to page_fault_handler()
<br>In JOS, I used int $0x30 as the system call interrupt. I have defined the constant T_SYSCALL to 48 (0x30). I added a handler in the kernel for interrupt vector T_SYSCALL. I also changed trap_dispatch() to handle the system call interrupt by calling syscall() (defined in kern/syscall.c) and parse interrupt information through arguments and arrange for the return value to be passed back to the user process in %eax. And I implemented syscall() in kern/syscall.c, return -E_INVAL if the system call number is invalid.<br>
I implemented system calls using the sysenter and sysexit instructions instead of using int 0x30 and iret.(the sysenter/sysexit instructions were designed by Intel to be faster than int/iret). I add a sysenter_handler in kern/trapentry.S that saves enough information about the user environment to return to it, sets up the kernel environment, pushes the arguments to syscall() and calls syscall() directly. Once syscall() returns, set everything up for and execute the sysexit instruction.
