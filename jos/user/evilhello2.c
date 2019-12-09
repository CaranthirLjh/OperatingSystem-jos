// evil hello world -- kernel pointer passed to kernel
// kernel should destroy user environment in response

#include <inc/lib.h>
#include <inc/mmu.h>
#include <inc/x86.h>

char vaddr[PGSIZE];
struct Segdesc old;
struct Segdesc *gdt;
struct Segdesc *entry;

// Call this function with ring0 privilege
void evil()
{
	// Kernel memory access
	*(char*)0xf010000a = 0;

	// Out put something via outb
	outb(0x3f8, 'I');
	outb(0x3f8, 'N');
	outb(0x3f8, ' ');
	outb(0x3f8, 'R');
	outb(0x3f8, 'I');
	outb(0x3f8, 'N');
	outb(0x3f8, 'G');
	outb(0x3f8, '0');
	outb(0x3f8, '!');
	outb(0x3f8, '!');
	outb(0x3f8, '!');
	outb(0x3f8, '\n');
}

void call_fun_ptr()
{
    evil();  
    *entry = old;  
    asm volatile("leave");
    asm volatile("lret");   
}

static void
sgdt(struct Pseudodesc* gdtd)
{
	__asm __volatile("sgdt %0" :  "=m" (*gdtd));
}

// Invoke a given function pointer with ring0 privilege, then return to ring3
void ring0_call(void (*fun_ptr)(void)) {
    // Here's some hints on how to achieve this.
    // 1. Store the GDT descripter to memory (sgdt instruction)
    struct Pseudodesc gdt_dsp; 
    sgdt(&gdt_dsp);
    // 2. Map GDT in user space (sys_map_kernel_page)
    if(sys_map_kernel_page((void* )gdt_dsp.pd_base, (void* )vaddr) < 0)
    {
        panic("evilhello2->ring0_call: failed in mapping GDT in user space\n");
    }
    // 3. Setup a CALLGATE in GDT (SETCALLGATE macro)
    uint32_t gdt_base = (uint32_t)(PGNUM(vaddr) << PTXSHIFT);
    uint32_t gdt_offset = PGOFF(gdt_dsp.pd_base);

    gdt = (struct Segdesc*)(gdt_base + gdt_offset);
    entry = gdt + (GD_UD >> 3);
    old = *entry;

    SETCALLGATE(*((struct Gatedesc*)entry), GD_KT, call_fun_ptr, 3);
    // 4. Enter ring0 (lcall instruction)
    // 5. Call the function pointer
    asm volatile("lcall $0x20, $0");
    // 6. Recover GDT entry modified in step 3 (if any)
    // 7. Leave ring0 (lret instruction)

    // Hint : use a wrapper function to call fun_ptr. Feel free
    //        to add any functions or global variables in this 
    //        file if necessary.
}

void
umain(int argc, char **argv)
{
        // call the evil function in ring0
	ring0_call(&evil);

	// call the evil function in ring3
	evil();
}

