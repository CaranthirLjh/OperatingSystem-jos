# OperatingSystem-jos
 jos lab, MIT 6.828
- You can switch to branch 'labx' to see the implementation of each stage.
## Requirement
- I recommend you to use ubuntu-16.04.2(32-bit) to run this operating system.
- You can use the virtual machine provided by us.
  ```
    wget http://ftp.sjtu.edu.cn/ubuntu-cd/16.04.2/ubuntu-16.04.2-desktop-i386.iso
  ```

## Lab1
### Part 1: PC Bootstrap
The purpose of the first exercise is to get familiar with x86 assembly language and the PC bootstrap process, and to start with QEMU and QEMU/GDB debugging.
### Part 2: The Boot Loader
Floppy and hard disks for PCs are divided into 512 byte regions called sectors. A sector is the disk's minimum transfer granularity: each read or write operation must be one or more sectors in size and aligned on a sector boundary. If the disk is bootable, the first sector is called the boot sector, since this is where the boot loader code resides. When the BIOS finds a bootable floppy or hard disk, it loads the 512-byte boot sector into memory at physical addresses 0x7c00 through 0x7dff, and then uses a jmp instruction to set the CS:IP to 0000:7c00, passing control to the boot loader. Like the BIOS load address, these addresses are fairly arbitrary - but they are fixed and standardized for PCs.<br>
The ability to boot from a CD-ROM came much later during the evolution of the PC, and as a result the PC architects took the opportunity to rethink the boot process slightly. As a result, the way a modern BIOS boots from a CD-ROM is a bit more complicated (and more powerful). CD-ROMs use a sector size of 2048 bytes instead of 512, and the BIOS can load a much larger boot image from the disk into memory (not just one sector) before transferring control to it. <br>
For JOS, I use the conventional hard drive boot mechanism, which means that the boot loader must fit into a measly 512 bytes. The boot loader consists of one assembly language source file, boot/boot.S, and one C source file, boot/main.c. The boot loader switches the processor from real mode to 32-bit protected mode and reads the kernel from the hard disk by directly accessing the IDE disk device registers via the x86's special I/O instructions.
### Part 3: The Kernel
In Lab1,I just map the first 4MB of physical memory, which will be enough to get the operating system up and running. I do this using the hand-written, statically-initialized page directory and page table in kern/entrypgdir.c. Up until kern/entry.S sets the CR0_PG flag, memory references are treated as physical addresses. Once CR0_PG is set, memory references are virtual addresses that get translated by the virtual memory hardware to physical addresses. <br>
I formatted printing to the console. I implemented the code necessary to print octal numbers using patterns of the form "%o"(The octal number should begin with '0'). I also add support for the "+" flag, which forces to precede the result with a plus or minus sign (+ or -) even for positive numbers. I enhanced the cprintf function to allow it print with the %n specifier and modified the function printnum() in lib/printfmt.c to support "%-" when printing numbers.<br>
I implemented the backtrace function.The backtrace function should display a listing of function call frames in the following format:
  ```
    Stack backtrace:
    eip f01008ae  ebp f010ff78  args 00000001 f010ff8c 00000000 f0110580 00000000 kern/monitor.c:143 monitor+106
    eip f0100193  ebp f010ffd8  args 00000000 00001aac 00000660 00000000 00000000 kern/init.c:49 i386_init+59
    eip f010003d  ebp f010fff8  args 00000000 00000000 0000ffff 10cf9a00 0000ffff kern/entry.S:70 <unknown>+0
    ...
  ```
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
