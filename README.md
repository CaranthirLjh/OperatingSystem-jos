# OperatingSystem-jos
 jos lab, MIT 6.828
## Lab6
In this the lab I implement a driver for a network interface card. The card is based on the Intel 82540EM chip.
### Part 1: Initialization and transmitting packets
- Add time tick increment to clock interrupts in trap_dispatch(struct Trapframe *tf) in kern/trap.c.
- Implement sys_time_msec(void) by returning time_msec function and add it to the branch of syscall in kern/syscall.c.
- Implement pci_e1000_attach function in kern/e1000.c. Create a virtual memory mapping for the E1000's BAR 0 by calling mmio_map_region function in pci_e1000_attach.
- Implement e1000_tx(const void *buf, uint32_t len) in kern/e1000.c: Transmit a packet by checking that the next descriptor is free, setting up next descriptor, and updating TDT.
- Implement sys_net_send(const void *buf, uint32_t len) in kern/syscall.c: First check the user permission(**PTE_U | PTE_P**) to [buf, buf + len]. If the permission is not qulified, return. Else, call e1000_tx function to send the packet and return the result.
- Implement net/output.c: In the while cycle structure, first use ipc_recv function to read a packet request. Then use sys_net_send function to send the packet to the device driver with a **retry_count**
### Part 2: Receiving packets and the web server
- Implement e1000_rx(void *buf, uint32_t len) in kern/e1000.c: First check if there is any package. If there is no package, return -E_AGAIN. Else, check whether the buf is large enough to hold the package. Then reset the decscriptor and give it back to hardware by modifying RDT
- Implement sys_net_recv(void *buf, uint32_t len) in kern/syscall.c: First check the user permission(**PTE_U | PTE_P | PTE_W**) to [buf, buf + len]. If the permission is not qulified, return. Else, call e1000_rx function to fill the buffer and return the result.
- Implement net/input.c: In the while cycle structure, first read a packet from the device driver, then send it to the network server by calling sys_ipc_try_send with NSREQ_INPUT as parameter.
- Implement send_file and send_data in user/httpd.c:
  - send_file(struct http_request *req): First judge whether the file exists and is not a directory. If it does, open the requested url for reading, otherwise, send a 404 error using send_error and return. Then set file_size to the size of the file and call send_data function to send data.
  - send_data(struct http_request *req, int fd): First use memset function to allocate a buf to store data content. Then call read function and with buf as parameter to get data content. Finally call write fucntion to send data.
