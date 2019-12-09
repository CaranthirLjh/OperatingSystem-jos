#include "ns.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	
	
	//	do the above things in a loop
	while (true) 
	{
		int ret = 0;
		// 	- read a packet request (using ipc_recv)
		void *pg = 0;
		int perm = 0;
		ret = ipc_recv(&ns_envid, &nsipcbuf, &perm);
		if (ret < 0) {
			cprintf("output.c: ipc_recv->%e", ret);
			return;
		}
			

		int retry_count = 0;
		//	- send the packet to the device driver (using sys_net_send)
		while (retry_count < 3) {
			ret = sys_net_send(nsipcbuf.pkt.jp_data, nsipcbuf.pkt.jp_len);

			if (ret == nsipcbuf.pkt.jp_len)
			{
				break;
			}
				
			if (ret == -E_INVAL) 
			{
				break;
			}

			retry_count++;
		}

		
	}
}
