# OperatingSystem-jos
 jos lab, MIT 6.828
## Lab5
In this lab, I implement spawn, a library call that loads and runs on-disk executables and a simple read/write file system to run a shell on the console. 
### Part 1: The File System
-  I modify env_create function in kern/env.c. If the type passed by i386_init is equal to **ENV_TYPE_FS**, then I give the file system environment I/O privilege.
-  Implement the bc_pgfault and flush_block functions in fs/bc.c：
   -  bc_pgfault(struct UTrapframe *utf): First round the addr to page boundary by calling ROUNDDOWN function. Then allocate a page in the disk map region and read the contents of the block from the disk into that page.
   -  flush_block(void *addr): First judge whether the block is not in the block cache or is not dirty. If it is, panic. Then flush the contents of the block containing VA out to disk if necessary and clear the PTE_D bit using sys_page_map.
-  Implement alloc_block function in fs/fs.c by traversing all block and finding the first free block and allocate it , then flushing the changed bitmap block to disk.
-  Implement file_block_walk and file_get_block in fs/fs.c：
   -  file_block_walk(struct File \*f, uint32_t filebno, uint32_t \*\*ppdiskbno, bool alloc): First judge whether the filebno is out of range, if it is, return -E_INVAL. If filebno < **NDIRECT**(Number of block pointers in a File descriptor), set '\*ppdiskbno' to point to that slot. Else, the slot will be an entry in the indirect block, set '*ppdiskbno' to point to that slot.
   - file_get_block(struct File *f, uint32_t filebno, char **blk): First use file_block_walk funtion to find the disk block number slot. Then use alloc_block function and diskaddr function to map the file block to the address in memory.
 - Implement serve_read in fs/serv.c: Read at most ipc->read.req_n bytes from the current seek position in ipc->read.req_fileid.  Return the bytes read from the file to the caller in ipc->readRet.  Returns the number of bytes successfully read, or < 0 on error. Use openfile_lookup function to search openFile, if the target file does not exist, return. Use file_read function to read content from target file and then update the seek position.
 - Implement serve_write in fs/serv.c and devfile_write in lib/file.c：
   - serve_write(envid_t envid, struct Fsreq_write *req): Use openfile_lookup function to search openFile, if the target file does not exist, return. Use file_write function to write content to target file and then update the seek position.
   - devfile_write(struct Fd *fd, const void *buf, size_t n): Pack related parameters into fsipcbuf and then call memmove to write content. Then make an FSREQ_WRITE request to the file system server.
### Part 2: Spawning Processes
- Implement sys_env_set_trapframe(envid_t envid, struct Trapframe *tf) in kern/syscall.c: This function is used to initialize the state of the newly created environment. Set envid's trap frame to **tf**. Set tf_cs to **GD_UT | 3**(code protection level 3). Set tf_eflags to tf_eflags|FL_IF to enable interrupt. Then dispatch the new system call in syscall function.
- Change duppage(envid_t envid, unsigned pn) in lib/fork.c: If the page table entry has the PTE_SHARE bit set(uvpt[pn] & PTE_SHARE), just copy the mapping directly.
- Implement copy_shared_pages in lib/spawn.c: Do the same mapping in child's process as parent. Search from UTEXT to USTACKTOP map the PTE_P | PTE_U | PTE_SHARE page.
### Part 3: Keyboard Interface and Shell
- Call kbd_intr function in trap_dispatch(struct Trapframe *tf) in kern/trap.c to handle trap IRQ_OFFSET+IRQ_KBD and serial_intr to handle trap IRQ_OFFSET+IRQ_SERIAL.
- Implement input redirection in runcmd(char* s) in user/sh.c.