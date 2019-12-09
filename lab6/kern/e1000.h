#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <kern/pci.h>

#define E1000_VID 0x8086
#define E1000_DID 0x100e

struct E1000 {
	volatile uint32_t CTRL;             /* 0x00000  Device Control - RW */
	volatile uint32_t CTRL_DUP;         /* 0x00004  Device Control Duplicate (Shadow) - RW */
	volatile const uint32_t STATUS;     /* 0x00008  Device Status - RO */
	uint32_t reserved[49];
	volatile uint32_t IMS;              /* 0x000D0  Interrupt Mask Set - RW */
	uint32_t reserved2;
	volatile uint32_t IMC;              /* 0x000D8  Interrupt Mask Clear - WO */
	uint32_t reserved3[9];
	volatile uint32_t RCTL;             /* 0x00100  RX Control - RW */
	uint32_t reserved4[191];
	volatile uint32_t TCTL;             /* 0x00400  TX Control - RW */
	uint32_t reserved5[3];
	volatile uint32_t TIPG;             /* 0x00410  TX Inter-packet gap -RW */
	uint32_t reserved6[2299];
	volatile uint32_t RDBAL;            /* 0x02800  RX Descriptor Base Address Low - RW */
	volatile uint32_t RDBAH;            /* 0x02804  RX Descriptor Base Address High - RW */
	volatile uint32_t RDLEN;            /* 0x02808  RX Descriptor Length - RW */
	uint32_t reserved7;
	volatile uint32_t RDH;              /* 0x02810  RX Descriptor Head - RW */
	uint32_t reserved8;
	volatile uint32_t RDT;              /* 0x02818  RX Descriptor Tail - RW */
	uint32_t reserved9[1017];
	volatile uint32_t TDBAL;            /* 0x03800  TX Descriptor Base Address Low - RW */
	volatile uint32_t TDBAH;            /* 0x03804  TX Descriptor Base Address High - RW */
	volatile uint32_t TDLEN;            /* 0x03808  TX Descriptor Length - RW */
	uint32_t reserved10;
	volatile uint32_t TDH;              /* 0x03810  TX Descriptor Head - RW */
	uint32_t reserved11;
	volatile uint32_t TDT;              /* 0x03818  TX Descripotr Tail - RW */
	volatile uint32_t MTA[128];         /* 0x05200  Multicast Table Array - RW Array */
	volatile uint32_t RAL;              /* 0x05400  Receive Address Low - RW */
	volatile uint32_t RAH;              /* 0x05404  Receive Address High - RW */
};

#define E1000_TCTL_EN                2U
#define E1000_TCTL_PSP               8U
#define E1000_TCTL_CT_ETHER          (0x10U << 4)
#define E1000_TCTL_COLD_FULL_DUPLEX  (0x40U << 12)
#define E1000_TIPG_DEFAULT           (10 | (4 << 10) | (6 << 20))
#define E1000_RCTL_EN                (2U)
#define E1000_RCTL_BSIZE_2048        (0U << 16)
#define E1000_RCTL_SECRC             (1U << 26)
#define E1000_RAH_AV				 (1U << 31)


#define QEMU_MAC_LOW 0x12005452
#define QEMU_MAC_HIGH 0x5634

struct tx_desc {
	uint64_t addr;
	uint16_t length;
	uint8_t cso;
	uint8_t cmd;
	uint8_t status;
	uint8_t css;
	uint16_t special;
};

#define E1000_TX_CMD_EOP (1U)
#define E1000_TX_CMD_RS (4U)
#define E1000_TX_STATUS_DD (1U)

struct rx_desc {
	uint64_t addr;
	uint16_t length;
	uint16_t checksum;
	uint8_t status;
	uint8_t error;
	uint16_t special;
};

#define E1000_RX_STATUS_DD (1U)

int pci_e1000_attach(struct pci_func *pcif);
int e1000_tx_init();
int e1000_tx(const void *buf, uint32_t len);
int e1000_rx(void *buf, uint32_t len);

#endif  // SOL >= 6
