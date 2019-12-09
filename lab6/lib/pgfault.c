// User-level page fault handler support.
// Rather than register the C page fault handler directly with the
// kernel as the page fault handler, we register the assembly language
// wrapper in pfentry.S, which in turns calls the registered C
// function.

#include <inc/lib.h>


// Assembly language pgfault entrypoint defined in lib/pfentry.S.
extern void _pgfault_upcall(void);

// Pointer to currently installed C-language pgfault handler.
void (*_pgfault_handler)(struct UTrapframe *utf);

//
// Set the page fault handler function.
// If there isn't one yet, _pgfault_handler will be 0.
// The first time we register a handler, we need to
// allocate an exception stack (one page of memory with its top
// at UXSTACKTOP), and tell the kernel to call the assembly-language
// _pgfault_upcall routine when a page fault occurs.
//
void
set_pgfault_handler(void (*handler)(struct UTrapframe *utf))
{
	// If there isn't one yet, _pgfault_handler will be 0.
	if (_pgfault_handler == 0) {
		// First time through!
		// LAB 4: Your code here.
		// allocate an exception stack
		int ret_alloc = sys_page_alloc((envid_t)0, (void*)(UXSTACKTOP-PGSIZE), PTE_U | PTE_P | PTE_W);
		if(ret_alloc < 0 ){
			panic("pgfault: set_pgfault_handler failed: %e\n",ret_alloc);
		}
		// tell the kernel to call the assembly-language _pgfault_upcall routine when a page fault occurs.
		int ret_pgfault_upcall = sys_env_set_pgfault_upcall((envid_t)0, _pgfault_upcall);
		if(ret_pgfault_upcall < 0){
			panic("pgfault: sys_env_set_pgfault_upcall: %e\n", ret_pgfault_upcall);
		}
	}

	// Save handler pointer for assembly to call.
	_pgfault_handler = handler;
}
