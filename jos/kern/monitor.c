// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>
#include <inc/mmu.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/trap.h>
#include <kern/pmap.h>


#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo }
	// { "showmappings", "Display the physical page mappings and corresponding permission bits", mon_showmappings },
	// { "setpermissions", "Set, clear, or change the permissions of any mapping in the current address space", mon_setpermissions },
	// { "dumpvapa", "Dump the contents of a range of memory given either a virtual or physical address range", mon_dumpvapa },
};

/***** Implementations of basic kernel monitor commands *****/
// void
// print_mappings(void* addr_start, void* addr_end)
// {
// 	for(void* i = addr_start; i < addr_end; i += PGSIZE)
// 	{
// 		physaddr_t physaddr_ptr = *(pgdir_walk(kern_pgdir, i, 1));
// 		if(physaddr_ptr == 0)
// 		{
// 			cprintf("va %08x failed to allcate", addr_start);
// 		}
// 		else
// 		{
// 			cprintf("va: %08x pa: %08x", addr_start, PTE_ADDR(physaddr_ptr));
// 		}

// 	}
// }

// showmappings command
// int
// mon_showmappings(int argc, char **argv, struct Trapframe *tf)
// {
// 	//get addr from the input
// 	void* addr_start = (void*)strtol(argv[1], NULL, 0);
// 	void* addr_end = (void*)strtol(argv[2], NULL, 0);

// 	print_mappings(addr_start,addr_end);

// 	cprintf("\n");

// 	return 0;
// }

// setpermissions command
// int 
// mon_setpermissions(int argc, char **argv, struct Trapframe *tf)
// {
// 	void* cur_addr = (void*)strtol(argv[1], NULL, 0);

// 	char* perm_P = strchr(argv[2], 'P');
// 	char* perm_W = strchr(argv[2], 'W');
// 	char* perm_U = strchr(argv[2], 'U');
// 	char* perm_0 = strchr(argv[2], '0');
	
// 	pte_t *pte = pgdir_walk(kern_pgdir, (void *)cur_addr, 1);

// 	// permission
// 	int perm = 0;
// 	if(perm_P != NULL)
// 	{
// 		perm = perm | PTE_P;
// 	}
// 	if(perm_W != NULL)
// 	{
// 		perm = perm | PTE_W;
// 	}
// 	if(perm_U != NULL)
// 	{
// 		perm = perm | PTE_U;
// 	}
// 	if(perm_0 != NULL)
// 	{
// 		perm = 0;
// 	}

// 	*pte = *pte | perm;

// 	print_mappings(cur_addr, cur_addr + PGSIZE);
// 	cprintf("\n");

// 	return 0;
// }

// dumpvapa command
// int 
// mon_dumpvapa(int argc, char **argv, struct Trapframe *tf)
// {
	
// 	char* bool_va = strchr(argv[3], 'V');
// 	char* bool_pa = strchr(argv[3], 'P');

	

// 	if(bool_va != NULL)
// 	{
// 		void* addr_start = (void*)strtol(argv[1], NULL, 0);
// 		void* addr_end = (void*)strtol(argv[2], NULL, 0);
// 		cprintf("Dump va from %08x to %08x\n", addr_start, addr_end);

// 		for (void* i = addr_start; addr_start < addr_end; i++)
// 		{
// 			cprintf("%08x ", *(char*)i);
// 		}
// 	}
// 	if(bool_pa != NULL)
// 	{
// 		physaddr_t addr_start = strtol(argv[1], NULL, 0);
// 		physaddr_t addr_end = strtol(argv[2], NULL, 0);
// 		cprintf("Dump va from %08x to %08x\n", addr_start, addr_end);

// 		for (physaddr_t i = addr_start; addr_start < addr_end; i++)
// 		{
// 			cprintf("%08x ", *(char*)KADDR(i));
// 		}
// 	}
	
// 	return 0;
// }

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(commands); i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

// Lab1 only
// read the pointer to the retaddr on the stack
static uint32_t
read_pretaddr() {
    uint32_t pretaddr;
    __asm __volatile("leal 4(%%ebp), %0" : "=r" (pretaddr)); 
    return pretaddr;
}

void
do_overflow(void)
{
    cprintf("Overflow success\n");
}

void
start_overflow(void)
{
	// You should use a techique similar to buffer overflow
	// to invoke the do_overflow function and
	// the procedure must return normally.

    // And you must use the "cprintf" function with %n specifier
    // you augmented in the "Exercise 9" to do this job.

    // hint: You can use the read_pretaddr function to retrieve 
    //       the pointer to the function call return address;

    char str[256] = {};
    int nstr = 0;
    char *pret_addr;

	// Your code here.
    pret_addr = (char *)read_pretaddr();

	uint32_t target_addr = (uint32_t)do_overflow;

	for (int i = 0; i < 4; ++i){
		cprintf("%*s%n\n", pret_addr[i] & 0xFF, "x", pret_addr + 4 + i);// move the original ret-addr to the above 4 addr-space
	}
      
	for (int i = 0; i < 4; ++i){
		cprintf("%*s%n\n", (target_addr >> (8*i)) & 0xFF, "x", pret_addr + i);// replace the original ret-addr with the addr of our attack code
	}
      

}

void
overflow_me(void)
{
        start_overflow();
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	// Your code here.
	uint32_t ebp = read_ebp();// indicate the base pointer into the stack used by that function
	uint32_t esp = read_esp();// the instruction address to which control will return when the function return

	cprintf("Stack backtrace:\n");
	while(ebp!=0)
	{
		esp = *((uint32_t *)ebp + 1);
		//get args:
		uint32_t arg_1 = *((uint32_t *)ebp + 2);
		uint32_t arg_2 = *((uint32_t *)ebp + 3);
		uint32_t arg_3 = *((uint32_t *)ebp + 4);
		uint32_t arg_4 = *((uint32_t *)ebp + 5);
		uint32_t arg_5 = *((uint32_t *)ebp + 6);

		cprintf("  eip %08x ebp %08x args %08x %08x %08x %08x %08x\n",esp,ebp,arg_1,arg_2,arg_3,arg_4,arg_5);
		//output the line num:
		struct Eipdebuginfo info;
		debuginfo_eip((uintptr_t)esp, &info);

		char function_name[info.eip_fn_namelen+1];
		// memcpy(function_name,info.eip_fn_name,info.eip_fn_namelen);
		for(int i=0;i<info.eip_fn_namelen;i++){
			function_name[i]=info.eip_fn_name[i];
		}
		// function_name = info.eip_fn_name;
		function_name[info.eip_fn_namelen]='\0';

		uint32_t line_in_func = esp - (uint32_t)info.eip_fn_addr;

		cprintf("         %s:%u %s+%u\n",info.eip_file, info.eip_line, function_name, line_in_func);

		ebp = *((uint32_t*)ebp);
	}
	


	overflow_me();
    	cprintf("Backtrace success\n");
	return 0;
}



/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
