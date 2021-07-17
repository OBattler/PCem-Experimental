/*
 * 86Box	A hypervisor and IBM PC system emulator that specializes in
 *		running old operating systems and software designed for IBM
 *		PC systems and compatibles from 1981 through fairly recent
 *		system designs based on the PCI bus.
 *
 *		This file is part of the 86Box distribution.
 *
 *		Implementation of the UMC 8886xx PCI to ISA Bridge .
 *
 *		Note: This chipset has no datasheet, everything were done via
 *		reverse engineering the BIOS of various machines using it.
 *
 * Authors:	Tiseno100,
 *
 *		Copyright 2021 Tiseno100.
 */

/* 
   UMC 8886xx Configuration Registers

   Note: PMU functionality is quite basic. There may be Enable/Disable bits, IRQ/SMI picks and it also
   required for 386_common.c to get patched in order to function properly.

   Warning: Register documentation may be inaccurate!

   UMC 8886xx:
   (F: Has No Internal IDE / AF or BF: Has Internal IDE)

   Function 0 Register 43:
   Bits 7-4 PCI IRQ for INTB
   Bits 3-0 PCI IRQ for INTA

   Function 0 Register 44:
   Bits 7-4 PCI IRQ for INTD
   Bits 3-0 PCI IRQ for INTC

   Function 0 Register 46:
   Bit 6: IRQ SMI Request (1: IRQ 10) (Supposedly 0 according to Phoenix is IRQ 15 but doesn't seem to make sense)

   Function 0 Register 56:
   Bit 1-0 ISA Bus Speed
       0 0 PCICLK/3
       0 1 PCICLK/4
       1 0 PCICLK/2

   Function 0 Register A3:
   Bit 7: Unlock SMM
   Bit 6: Software SMI trigger

   Function 0 Register A4:
   Bit 0: Host to PCI Clock (1: 1 by 1/0: 1 by half)

   Function 1 Register 4: (UMC 8886AF/8886BF Only!)
   Bit 0: Enable Internal IDE
*/

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#define HAVE_STDARG_H
#include <86box/86box.h>
#include "cpu.h"
#include <86box/timer.h>
#include <86box/io.h>
#include <86box/device.h>

#include <86box/hdd.h>
#include <86box/hdc.h>
#include <86box/hdc_ide.h>
#include <86box/pic.h>
#include <86box/pci.h>

#include <86box/chipset.h>

#ifdef ENABLE_UMC_8886_LOG
int umc_8886_do_log = ENABLE_UMC_8886_LOG;
static void
umc_8886_log(const char *fmt, ...)
{
	va_list ap;

	if (umc_8886_do_log)
	{
		va_start(ap, fmt);
		pclog_ex(fmt, ap);
		va_end(ap);
	}
}
#else
#define umc_8886_log(fmt, ...)
#endif

/* PCI IRQ Flags */
#define INTA (PCI_INTA + (2 * !(addr & 1)))
#define INTB (PCI_INTB + (2 * !(addr & 1)))
#define IRQRECALCA (((val & 0xf0) != 0) ? ((val & 0xf0) >> 4) : PCI_IRQ_DISABLED)
#define IRQRECALCB (((val & 0x0f) != 0) ? (val & 0x0f) : PCI_IRQ_DISABLED)

/* Disable Internal IDE Flag needed for the AF or BF Southbridge variant */
#define HAS_IDE dev->has_ide

/* Southbridge Revision */
#define SB_ID dev->sb_id

typedef struct umc_8886_t
{

	uint8_t pci_conf_sb[2][256]; /* PCI Registers */
	uint16_t sb_id;		     /* Southbridge Revision */
	int has_ide;		     /* Check if Southbridge Revision is AF or F */

} umc_8886_t;

void
umc_8886_ide_handler(int status)
{
	ide_pri_disable();
	ide_sec_disable();

	if (status)
	{
		ide_pri_enable();
		ide_sec_enable();
	}
}

void umc_8886_defaults(umc_8886_t *dev)
{
	dev->pci_conf_sb[0][0] = 0x60; /* UMC */
	dev->pci_conf_sb[0][1] = 0x10;

	dev->pci_conf_sb[0][2] = (SB_ID & 0xff); /* 8886xx */
	dev->pci_conf_sb[0][3] = ((SB_ID >> 8) & 0xff);

	dev->pci_conf_sb[0][4] = 0x0f;
	dev->pci_conf_sb[0][7] = 2;

	dev->pci_conf_sb[0][8] = 0x0e;

	dev->pci_conf_sb[0][0x09] = 0x00;
	dev->pci_conf_sb[0][0x0a] = 0x01;
	dev->pci_conf_sb[0][0x0b] = 0x06;

	dev->pci_conf_sb[0][0x40] = 1;
	dev->pci_conf_sb[0][0x41] = 6;
	dev->pci_conf_sb[0][0x42] = 8;
	dev->pci_conf_sb[0][0x43] = 0x9a;
	dev->pci_conf_sb[0][0x44] = 0xbc;
	dev->pci_conf_sb[0][0x45] = 4;
	dev->pci_conf_sb[0][0x47] = 0x40;
	dev->pci_conf_sb[0][0x50] = 1;
	dev->pci_conf_sb[0][0x51] = 3;
	dev->pci_conf_sb[0][0xa8] = 0x20;

	if (HAS_IDE)
	{
		dev->pci_conf_sb[1][0] = 0x60; /* UMC */
		dev->pci_conf_sb[1][1] = 0x10;

		dev->pci_conf_sb[1][2] = 0x3a; /* 8886BF IDE */
		dev->pci_conf_sb[1][3] = 0x67;

		dev->pci_conf_sb[1][4] = 1; /* Start with Internal IDE Enabled */

		dev->pci_conf_sb[1][8] = 0x10;

		dev->pci_conf_sb[1][0x09] = 0x0f;
		dev->pci_conf_sb[1][0x0a] = 1;
		dev->pci_conf_sb[1][0x0b] = 1;

		umc_8886_ide_handler(1);
	}
}

static void
umc_8886_write(int func, int addr, uint8_t val, void *priv)
{
	umc_8886_t *dev = (umc_8886_t *)priv;

	switch (func)
	{
		case 0: /* PCI to ISA Bridge */
			umc_8886_log("UM8886: dev->regs[%02x] = %02x POST %02x\n", addr, val, inb(0x80));
			switch (addr)
			{
				case 0x04:
				case 0x05:
					dev->pci_conf_sb[func][addr] = val;
					break;

				case 0x07:
					dev->pci_conf_sb[func][addr] &= ~(val & 0xf9);
					break;

				case 0x0c:
				case 0x0d:
					dev->pci_conf_sb[func][addr] = val;
					break;

				case 0x40:
				case 0x41:
				case 0x42:
					dev->pci_conf_sb[func][addr] = val;
					break;

				case 0x43:
				case 0x44:
					dev->pci_conf_sb[func][addr] = val;
					pci_set_irq_routing(INTA, IRQRECALCA);
					pci_set_irq_routing(INTB, IRQRECALCB);
					break;

				case 0x45:
					dev->pci_conf_sb[func][addr] = val;
					break;

				case 0x46:
					dev->pci_conf_sb[func][addr] = val;

					if(val & 0x40)
					picint(1 << ((val & 0x10) ? 15 : 10));

					break;

				case 0x47:
					dev->pci_conf_sb[func][addr] = val;
					break;

				case 0x50:
				case 0x51:
				case 0x52:
				case 0x53:
				case 0x54:
				case 0x55:
					dev->pci_conf_sb[func][addr] = val;
					break;

				case 0x56:
					dev->pci_conf_sb[func][addr] = val;

					switch (val & 2)
					{
						case 0:
						cpu_set_isa_pci_div(3);
						break;
						case 1:
						cpu_set_isa_pci_div(4);
						break;
						case 2:
						cpu_set_isa_pci_div(2);
						break;
					}

					break;

				case 0x57:
				case 0x70:
				case 0x71:
				case 0x72:
				case 0x73:
				case 0x74:
				case 0x75:
				case 0x76:
				case 0x80:
				case 0x81:
				case 0x90:
				case 0x91:
				case 0x92:
				case 0xa0:
				case 0xa1:
				case 0xa2:
					dev->pci_conf_sb[func][addr] = val;
					break;

				case 0xa3:
					dev->pci_conf_sb[func][addr] = val;

					if(((dev->pci_conf_sb[0][0xa3] >> 6) == 3) && !in_smm) /* SMI Provocation (Bit 7 Enable SMM + Bit 6 Software SMI */
					smi_line = 1;

					break;

				case 0xa4:
					dev->pci_conf_sb[func][addr] = val;
					cpu_set_pci_speed(cpu_busspeed / ((val & 1) ? 1 : 2));
					break;

				case 0xa5:
				case 0xa6:
				case 0xa7:
				case 0xa8:
					dev->pci_conf_sb[func][addr] = val;
				break;
			}
		break;

		case 1: /* IDE Controller */
			umc_8886_log("UM8886-IDE: dev->regs[%02x] = %02x POST: %02x\n", addr, val, inb(0x80));
			switch (addr)
			{
				case 0x04:
				dev->pci_conf_sb[func][addr] = val;
				umc_8886_ide_handler(val & 1);
				break;

			case 0x07:
				dev->pci_conf_sb[func][addr] &= ~(val & 0xf9);
				break;

			case 0x3c:
			case 0x40:
			case 0x41:
				dev->pci_conf_sb[func][addr] = val;
				break;
			}
		break;
	}
}

static uint8_t
umc_8886_read(int func, int addr, void *priv)
{
	umc_8886_t *dev = (umc_8886_t *)priv;
	return dev->pci_conf_sb[func][addr];
}

static void
umc_8886_reset(void *priv)
{
	umc_8886_t *dev = (umc_8886_t *)priv;

	umc_8886_defaults(dev);

	for (int i = 1; i < 5; i++) /* Disable all IRQ interrupts */
		pci_set_irq_routing(i, PCI_IRQ_DISABLED);
}

static void
umc_8886_close(void *priv)
{
	umc_8886_t *dev = (umc_8886_t *)priv;

	free(dev);
}

static void *
umc_8886_init(const device_t *info)
{
	umc_8886_t *dev = (umc_8886_t *)malloc(sizeof(umc_8886_t));
	memset(dev, 0, sizeof(umc_8886_t));

	dev->has_ide = !!(info->local == 0x886a);
	pci_add_card(PCI_ADD_SOUTHBRIDGE, umc_8886_read, umc_8886_write, dev); /* Device 12: UMC 8886xx */

	/* Add IDE if UM8886AF variant */
	if (HAS_IDE)
		device_add(&ide_pci_2ch_device);

	/* Get the Southbridge Revision */
	SB_ID = info->local;

	umc_8886_reset(dev);

	return dev;
}

const device_t umc_8886f_device = {
    "UMC 8886F",
    DEVICE_PCI,
    0x8886,
    umc_8886_init, umc_8886_close, umc_8886_reset,
    { NULL }, NULL, NULL,
    NULL
};

const device_t umc_8886af_device = {
    "UMC 8886AF/8886BF",
    DEVICE_PCI,
    0x886a,
    umc_8886_init, umc_8886_close, umc_8886_reset,
    { NULL }, NULL, NULL,
    NULL
};
