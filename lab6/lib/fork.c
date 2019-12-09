// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

extern void _pgfault_upcall(void);

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	// Check that the faulting access was (1) a write
	if(!(err & FEC_WR)){
		panic("fork: pgfault failed: faulting access was not caused by a write\n");
	}
	// Check that the faulting access was (2) a copy-on-write page
	if(!((uvpd[PDX(addr)] & PTE_P) && (uvpt[PGNUM(addr)] & (PTE_P|PTE_COW)))){
		panic("fork: pgfault failed: faulting access was not caused by a copy-on-write page\n");
	}
		
	

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	// Allocate a new page, map it at a temporary location (PFTEMP)
	int ret_alloc = sys_page_alloc(0, PFTEMP, PTE_P | PTE_U | PTE_W);
	if (ret_alloc < 0){
		panic("fork: sys_page_alloc: %e\n", ret_alloc);
	}
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	addr = ROUNDDOWN(addr, PGSIZE);
	memmove(PFTEMP, addr, PGSIZE);

	int ret_map = sys_page_map(0, PFTEMP, 0, addr, PTE_P | PTE_U | PTE_W);
	if (ret_map < 0){
		panic("fork: sys_page_map: %e\n", ret_map);
	}
		
	int ret_unmap = sys_page_unmap(0, PFTEMP);
	if (ret_unmap < 0){
		panic("fork: sys_page_unmap: %e\n", ret_unmap);
	}
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	void * addr = (void *)(pn * PGSIZE);
	// If the page table entry has the PTE_SHARE bit set
	if(uvpt[pn] & PTE_SHARE){
		//  just copy the mapping directly
		int ret_map = sys_page_map((envid_t)0, addr, envid, addr, uvpt[pn] & PTE_SYSCALL);
		if(ret_map < 0){
			panic("duppage: sys_page_map directly failed: %e\n", ret_map);
		}
	}
	// If the page is writable or copy-on-write
	else if(uvpt[pn] & (PTE_W | PTE_COW)){
		// the new mapping must be created copy-on-write
		int ret_nmap = sys_page_map((envid_t)0, addr, envid, addr, PTE_U | PTE_P | PTE_COW);
		if(ret_nmap < 0){
			panic("duppage: sys_page_map failed: %e\n", ret_nmap);
		}
		// and then our mapping must be marked copy-on-write as well.
		int ret_omap = sys_page_map((envid_t)0, addr, 0, addr, PTE_U | PTE_P | PTE_COW);
		if(ret_omap < 0){
			panic("duppage: sys_page_map failed: %e\n", ret_omap);
		}		
	}
	// If the page is not writable or copy-on-write
	else{
		int ret_nmap = sys_page_map((envid_t)0, addr, envid, addr, PTE_U | PTE_P);
		if(ret_nmap < 0){
			panic("duppage: sys_page_map failed: %e\n", ret_nmap);
		}	
	}
	
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	// Set up our page fault handler appropriately.
	set_pgfault_handler(pgfault);

	// Create a child.
	envid_t envid;
	envid = sys_exofork();
	if(envid < 0){// < 0 on error
		// To be done, it should be return envid
		panic("fork: sys_exofork failed: %e\n",envid);
	}
	if(envid == 0){// 0 to the child
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	// parent situation:
	// Copy our address space and page fault handler setup to the child.
	uint8_t *addr = (uint8_t *)0;
	for (; addr < (uint8_t *)UTOP; addr += PGSIZE)
	{
		if ((uvpd[PDX(addr)] & PTE_P) && (uvpt[PGNUM(addr)] & PTE_P) && addr != (uint8_t *)UXSTACKTOP - PGSIZE){
			duppage(envid, PGNUM(addr));
		}
	}
	int ret_pgalloc = sys_page_alloc(envid, (void *)(UXSTACKTOP-PGSIZE), PTE_U|PTE_W|PTE_P);
	if(ret_pgalloc < 0){
		panic("fork: sys_page_alloc failed: %e\n",ret_pgalloc);
	}
	int ret_pgfault_upcall = sys_env_set_pgfault_upcall(envid, _pgfault_upcall);
	if(ret_pgfault_upcall < 0){
		panic("fork: sys_env_set_pgfault_upcall failed: %e\n",ret_pgfault_upcall);
	}
		
	// Then mark the child as runnable and return.
	int ret_status = sys_env_set_status(envid, ENV_RUNNABLE);
	if(ret_status < 0){
		panic("sys_env_set_status: %e\n",ret_status);
	}
	
	return envid;
}

// Challenge!
int
sfork(void)
{
    envid_t myenvid = sys_getenvid();
    
    uint32_t pn;
    int perm;

    // set page fault handler
    set_pgfault_handler(pgfault);

    // create a child
	envid_t envid = sys_exofork();
    if(envid < 0)
    {
        panic("sfork: sys_exofork failed: %e\n",envid);
    }
    if(envid == 0)
    {
        thisenv = &envs[ENVX(sys_getenvid())];
        return 0;
    }

    // copy address space to child
    for (int i = PDX(UTEXT); i < PDX(UXSTACKTOP); i++)
    {
        if(uvpd[i] & PTE_P)
        {
            for (int j = 0; j < NPTENTRIES; j++)
            {
                pn = PGNUM(PGADDR(i, j, 0));
                if(pn == PGNUM(UXSTACKTOP - PGSIZE))
                {
                    break;
                }

                if(pn == PGNUM(USTACKTOP - PGSIZE))
                {
                     duppage(envid, pn); // cow for stack page
                     continue;
                }

                // Map the page to child env 
                if (uvpt[pn] & PTE_P)
                {
                    perm = uvpt[pn] & ~(uvpt[pn] & ~(PTE_P |PTE_U | PTE_W | PTE_AVAIL));
					int ret_map = sys_page_map(myenvid, (void *)(PGADDR(i, j, 0)), envid, (void *)(PGADDR(i, j, 0)), perm);
                    if (ret_map < 0)
                    {
                        return ret_map;
                    }
                }
            }
        }
    }

    int ret_alloc = sys_page_alloc(envid, (void *)(UXSTACKTOP - PGSIZE), PTE_U | PTE_P | PTE_W);
    if(ret_alloc < 0)
    {
        return ret_alloc;
    }
	int ret_map = sys_page_map(envid, (void *)(UXSTACKTOP - PGSIZE), myenvid, PFTEMP, PTE_U | PTE_P | PTE_W);
    if(ret_map < 0)
    {
        return ret_map;
    }

    // copy own uxstack to temp page
    memmove((void *)(UXSTACKTOP - PGSIZE), PFTEMP, PGSIZE);

	int ret_umap = sys_page_unmap(myenvid, PFTEMP);
    if(ret_umap < 0)
    {
        return ret_umap;
    }

    // Set page fault handler in child
	int ret_pgfault_upcall = sys_env_set_pgfault_upcall(envid, _pgfault_upcall);
    if(ret_pgfault_upcall < 0)
    {
        return ret_pgfault_upcall;
    }

    // mark child env as RUNNABLE
	int ret_status = sys_env_set_status(envid, ENV_RUNNABLE);
    if(ret_status < 0)
    {
        return ret_status;
    }

    return envid;
}
