#include <kern/e1000.h>
#include <kern/pmap.h>
#include <inc/string.h>
#include <inc/error.h>

static struct E1000 *base;

struct tx_desc *tx_descs;
#define N_TXDESC (PGSIZE / sizeof(struct tx_desc))

static char tx_bufs[N_TXDESC * 1520];

int
e1000_tx_init()
{
	// Allocate one page for descriptors
	struct PageInfo *p = page_alloc(ALLOC_ZERO);
	p->pp_ref += 1;
	// Initialize all descriptors
	tx_descs = (struct tx_desc *)page2kva(p);

	for (int i = 0; i != N_TXDESC; i++) 
	{
		tx_descs[i].status |= E1000_TX_STATUS_DD;
		tx_descs[i].addr = (uint64_t)PADDR((void *)&tx_bufs[i * 1520]);
	}
	// Set hardward registers
	// Look kern/e1000.h to find useful definations
	base->TDBAL = PADDR(tx_descs);
	base->TDBAH = 0;
	base->TDLEN = PGSIZE;
	base->TDH = 0;
	base->TDT = 0;
	base->TIPG = E1000_TIPG_DEFAULT;
	base->TCTL |= E1000_TCTL_EN | E1000_TCTL_PSP | E1000_TCTL_CT_ETHER | E1000_TCTL_COLD_FULL_DUPLEX;

	return 0;
}

struct rx_desc *rx_descs;
#define N_RXDESC (PGSIZE / sizeof(struct rx_desc))

int
e1000_rx_init()
{
	// Allocate one page for descriptors
	struct PageInfo *p = page_alloc(ALLOC_ZERO);
	p->pp_ref += 1;
	// Initialize all descriptors
	// You should allocate some pages as receive buffer
	rx_descs = (struct rx_desc *)page2kva(p);

	struct PageInfo *pptrs[N_RXDESC / 2];

	for (int i = 0; i != N_RXDESC / 2; i++) 
	{
		struct PageInfo *pp = page_alloc(ALLOC_ZERO);
		pp->pp_ref += 1;
		pptrs[i] = pp;
	}
	for (int i = 0; i != N_RXDESC; i++) 
	{
		physaddr_t ph_addr = page2pa(pptrs[i / 2]);
		rx_descs[i].addr = (uint64_t)(ph_addr + (i % 2) * PGSIZE / 2);
	}

	// Set hardward registers
	// Look kern/e1000.h to find useful definations
	base->RAL = QEMU_MAC_LOW;
	base->RAH = QEMU_MAC_HIGH | E1000_RAH_AV;
	base->RDBAL = PADDR(rx_descs);
	base->RDBAH = 0;
	base->RDLEN = PGSIZE;
	base->RDH = 0;
	base->RDT = N_RXDESC - 1;
	base->RCTL |= E1000_RCTL_EN | E1000_RCTL_BSIZE_2048 | E1000_RCTL_SECRC;

	return 0;
}

int
pci_e1000_attach(struct pci_func *pcif)
{
	// Enable PCI function
	pci_func_enable(pcif);
	// Map MMIO region and save the address in 'base;
	void *addr = mmio_map_region((physaddr_t)pcif->reg_base[0], pcif->reg_size[0]);
	base = (struct E1000 *)addr;

	e1000_tx_init();
	e1000_rx_init();
	return 0;
}

int
e1000_tx(const void *buf, uint32_t len)
{
	// Send 'len' bytes in 'buf' to ethernet
	// Hint: buf is a kernel virtual address
	if (len > 1518)
	{
		return -E_INVAL;
	}

	uint32_t tdt = base->TDT;
	uint32_t tdh = base->TDH;
	if ((tdt + 1) % N_TXDESC == tdh)
	{
		return -E_AGAIN;
	}

	struct tx_desc *next = &tx_descs[tdt % N_TXDESC];
	if (!(next->status & E1000_TX_STATUS_DD)) 
	{
		return -E_AGAIN;
	}
		
	memcpy((void *)(KADDR((uint32_t)(next->addr))), buf, len);
	next->length = len;
	next->cmd |= E1000_TX_CMD_RS | E1000_TX_CMD_EOP;
	next->status &= ~E1000_TX_STATUS_DD;
	base->TDT = (tdt + 1) % N_TXDESC;

	return len;
}

int
e1000_rx(void *buf, uint32_t len)
{
	if (!buf)
	{
		return -E_INVAL;
	}
	// Copy one received buffer to buf
	// You could return -E_AGAIN if there is no packet
	int rdt = (base->RDT + 1) % N_RXDESC;

	struct rx_desc *next = &rx_descs[rdt];
	if (!(next->status & E1000_RX_STATUS_DD)) 
	{
		return -E_AGAIN;
	}
	// Check whether the buf is large enough to hold
	// the packet
	len = next->length;
	if (len > 1518) 
	{
		len = 1518;
	}

	// Do not forget to reset the decscriptor and
	// give it back to hardware by modifying RDT
	memmove(buf, (const void *)KADDR(next->addr), len);
	next->status = 0;
	base->RDT = rdt;

	return len;
}
