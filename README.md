# OperatingSystem-jos
 jos lab, MIT 6.828
## Lab2
In this lab, I mainly write the memory management code for the operating system. Memory management has two components.<br>
The first component is a physical memory allocator for the kernel, so that the kernel can allocate memory and later free it. The allocator will operate in units of 4096 bytes, called pages. I implemented the code to maintain data structures that record which physical pages are free and which are allocated, and how many processes are sharing each allocated page. I also wrote the routines to allocate and free pages of memory.<br>
The second component of memory management is virtual memory, which maps the virtual addresses used by kernel and user software to addresses in physical memory. The x86 hardware's memory management unit (MMU) performs the mapping when instructions use memory, consulting a set of page tables. 
### Part 1: Physical Page Management
In this part, I implementing physical page allocator by modifying file kern/pmap.c and implementing code for the following functions:
```
boot_alloc() & mem_init() & page_init() & page_alloc() & page_free()
```
### Part 2: Virtual Memory
I wrote a set of routines to manage page tables by by modifying file kern/pmap.c and implementing code for the following functions:
```
pgdir_walk() & boot_map_region() & boot_map_region_large() & page_lookup() & page_remove() & page_insert()
```
The page table manager's functions include:
- Insert and remove linear-to-physical mappings
- Create page table pages when needed.
### Part 3: Kernel Address Space
I set up the address space above UTOP(the kernel part of the address space) by using boot_map_region() in mem_init(). inc/memlayout.h shows the Virtual memory map structure.
