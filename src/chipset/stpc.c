/*
 * 86Box	A hypervisor and IBM PC system emulator that specializes in
 *		running old operating systems and software designed for IBM
 *		PC systems and compatibles from 1981 through fairly recent
 *		system designs based on the PCI bus.
 *
 *		This file is part of the 86Box distribution.
 *
 *		Implementation of the STPC series of SoCs.
 *
 *
 *
 * Authors:	RichardG, <richardg867@gmail.com>
 *
 *		Copyright 2020 RichardG.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#define HAVE_STDARG_H
#include <86box/86box.h>
#include <86box/mem.h>
#include <86box/io.h>
#include <86box/rom.h>
#include <86box/pci.h>
#include <86box/device.h>
#include <86box/keyboard.h>
#include <86box/port_92.h>
#include <86box/chipset.h>


typedef struct stpc_t
{
    /* ISA (port 22h/23h) */
    uint8_t	isa_offset;
    uint8_t	isa_regs[256];

    /* Host bus interface */
    uint16_t	host_base;
    uint8_t	host_offset;
    uint8_t	host_regs[256];

    /* Local bus */
    uint16_t	localbus_base;
    uint8_t	localbus_offset;
    uint8_t	localbus_regs[256];

    /* PCI */
    uint8_t	pci_conf[3][256];
} stpc_t;


#define ENABLE_STPC_LOG 1
#ifdef ENABLE_STPC_LOG
int stpc_do_log = ENABLE_STPC_LOG;


static void
stpc_log(const char *fmt, ...)
{
    va_list ap;

    if (stpc_do_log) {
	va_start(ap, fmt);
	pclog_ex(fmt, ap);
	va_end(ap);
    }
}
#else
#define stpc_log(fmt, ...)
#endif


static void
stpc_recalcmapping(stpc_t *dev)
{
    uint8_t reg, bitpair;
    uint32_t base, size;
    int state;

    shadowbios = 0;
    shadowbios_write = 0;

    for (reg = 0; reg <= 3; reg++) {
	for (bitpair = 0; bitpair <= (reg == 3 ? 0 : 3); bitpair++) {
		if (reg == 3) {
			size = 0x10000;
			base = 0xf0000;
		} else {
			size = 0x4000;
			base = 0xc0000 + (size * ((reg * 4) + bitpair));
		}
		stpc_log("STPC: Shadowing for %05x-%05x (reg %02x bp %d wmask %02x rmask %02x) =", base, base + size - 1, 0x25 + reg, bitpair, 1 << (bitpair * 2), 1 << ((bitpair * 2) + 1));

		state = 0;
		if (dev->isa_regs[0x25 + reg] & (1 << (bitpair * 2))) {
			stpc_log(" w on");
			state |= MEM_WRITE_INTERNAL;
			if (base >= 0xe0000)
				shadowbios_write |= 1;
		} else {
			stpc_log(" w off");
			state |= MEM_WRITE_EXTANY;
		}
		if (dev->isa_regs[0x25 + reg] & (1 << ((bitpair * 2) + 1))) {
			stpc_log("; r on\n");
			state |= MEM_READ_INTERNAL;
			if (base >= 0xe0000)
				shadowbios |= 1;
		} else {
			stpc_log("; r off\n");
			state |= MEM_READ_EXTANY;
		}

		mem_set_mem_state(base, size, state);
	}
    }

    flushmmucache();
}


static void
stpc_smram_map(int smm, uint32_t addr, uint32_t size, int is_smram)
{
    mem_set_mem_state_smram(smm, addr, size, is_smram);
}


static void
stpc_host_write(uint16_t addr, uint8_t val, void *priv)
{
    stpc_t *dev = (stpc_t *) priv;

    stpc_log("STPC: host_write(%04x, %02x)\n", addr, val);

    if (addr == dev->host_base)
	dev->host_offset = val;
    else if (addr == dev->host_base + 4)
	dev->host_regs[dev->host_offset] = val;
}


static uint8_t
stpc_host_read(uint16_t addr, void *priv)
{
    stpc_t *dev = (stpc_t *) priv;
    uint8_t ret;

    if (addr == dev->host_base)
	ret = dev->host_offset;
    else if (addr == dev->host_base + 4)
	ret = dev->host_regs[dev->host_offset];
    else
	ret = 0xff;

    stpc_log("STPC: host_read(%04x) = %02x\n", addr, ret);
    return ret;
}


static void
stpc_localbus_write(uint16_t addr, uint8_t val, void *priv)
{
    stpc_t *dev = (stpc_t *) priv;

    stpc_log("STPC: localbus_write(%04x, %02x)\n", addr, val);

    if (addr == dev->localbus_base)
	dev->localbus_offset = val;
    else if (addr == dev->localbus_base + 4)
	dev->localbus_regs[addr] = val;
}


static uint8_t
stpc_localbus_read(uint16_t addr, void *priv)
{
    stpc_t *dev = (stpc_t *) priv;
    uint8_t ret;

    if (addr == dev->localbus_base)
	ret = dev->localbus_offset;
    else if (addr == dev->localbus_base + 4)
	ret = dev->localbus_regs[dev->localbus_offset];
    else
	ret = 0xff;

    stpc_log("STPC: localbus_read(%04x) = %02x\n", addr, ret);
    return ret;
}


static void
stpc_nb_write(int func, int addr, uint8_t val, void *priv)
{
    stpc_t *dev = (stpc_t *) priv;

    stpc_log("STPC: nb_write(%d, %02x, %02x)\n", func, addr, val);

    if (func > 0)
	return;

    switch (addr) {
	case 0x00: case 0x01: case 0x02: case 0x03:
	case 0x04: case 0x06: case 0x07: case 0x08:
	case 0x09: case 0x0a: case 0x0b: case 0x0e:
	case 0x51: case 0x53: case 0x54:
		return;

	case 0x05:
		val &= 0x01;
		break;

	case 0x50:
		val &= 0x1f;
		break;

	case 0x52:
		val &= 0x70;
		break;
    }

    dev->pci_conf[0][addr] = val;
}


static uint8_t
stpc_nb_read(int func, int addr, void *priv)
{
    stpc_t *dev = (stpc_t *) priv;
    uint8_t ret;

    if (func > 0)
	ret = 0xff;
    else
	ret = dev->pci_conf[0][addr];

    stpc_log("STPC: nb_read(%d, %02x) = %02x\n", func, addr, ret);
    return ret;
}


static void
stpc_sb_write(int func, int addr, uint8_t val, void *priv)
{
    stpc_t *dev = (stpc_t *) priv;

    stpc_log("STPC: sb_write(%d, %02x, %02x)\n", func, addr, val);

    if (func > 0)
	return;

    switch (addr) {
	case 0x00: case 0x01: case 0x02: case 0x03:
	case 0x04: case 0x06: case 0x07: case 0x08:
	case 0x09: case 0x0a: case 0x0b: case 0x0e:
		return;

	case 0x05:
		val &= 0x01;
		break;
    }

    dev->pci_conf[1][addr] = val;
}

static uint8_t
stpc_sb_read(int func, int addr, void *priv)
{
    stpc_t *dev = (stpc_t *) priv;
    uint8_t ret;

    if (func > 0)
    	ret = 0xff;
    else
    	ret = dev->pci_conf[1][addr];

    stpc_log("STPC: sb_read(%d, %02x) = %02x\n", func, addr, ret);
    return ret;
}


static void
stpc_ide_write(int func, int addr, uint8_t val, void *priv)
{
    stpc_t *dev = (stpc_t *) priv;

    stpc_log("STPC: ide_write(%d, %02x, %02x)\n", func, addr, val);

    if (func > 0)
	return;

    switch (addr) {
	case 0x00: case 0x01: case 0x02: case 0x03:
	case 0x04: case 0x06: case 0x07: case 0x08:
	case 0x09: case 0x0a: case 0x0b: case 0x0e:
		return;

	case 0x05:
		val &= 0x01;
		break;
    }

    dev->pci_conf[2][addr] = val;
}

static uint8_t
stpc_ide_read(int func, int addr, void *priv)
{
    stpc_t *dev = (stpc_t *) priv;
    uint8_t ret;

    if (func > 0)
    	ret = 0xff;
    else
    	ret = dev->pci_conf[2][addr];

    stpc_log("STPC: ide_read(%d, %02x) = %02x\n", func, addr, ret);
    return ret;
}

/* TODO: IDE and USB OHCI devices */


static void
stpc_remap_host(stpc_t *dev, uint16_t host_base)
{
    stpc_log("STPC: Remapping host bus from %04x to %04x\n", dev->host_base, host_base);

    io_removehandler(dev->host_base, 5,
		     stpc_host_read, NULL, NULL, stpc_host_write, NULL, NULL, dev);
    if (host_base) {
	io_sethandler(host_base, 5,
		      stpc_host_read, NULL, NULL, stpc_host_write, NULL, NULL, dev);
    }
    dev->host_base = host_base;
}


static void
stpc_remap_localbus(stpc_t *dev, uint16_t localbus_base)
{
    stpc_log("STPC: Remapping local bus from %04x to %04x\n", dev->localbus_base, localbus_base);

    io_removehandler(dev->localbus_base, 5,
		     stpc_localbus_read, NULL, NULL, stpc_localbus_write, NULL, NULL, dev);
    if (localbus_base) {
	io_sethandler(localbus_base, 5,
		      stpc_localbus_read, NULL, NULL, stpc_localbus_write, NULL, NULL, dev);
    }
    dev->localbus_base = localbus_base;
}


static void
stpc_isa_write(uint16_t addr, uint8_t val, void *priv)
{
    stpc_t *dev = (stpc_t *) priv;

    stpc_log("STPC: isa_write(%04x, %02x)\n", addr, val);

    if (addr == 0x22) {
	dev->isa_offset = val;
    } else {
	stpc_log("STPC: isa_regs[%02x] = %02x\n", dev->isa_offset, val);

	switch (dev->isa_offset) {
		case 0x12:
			if (dev->isa_regs[0x10] == 0x07)
				stpc_remap_host(dev, (dev->host_base & 0xff00) | val);
			else if (dev->isa_regs[0x10] == 0x06)
				stpc_remap_localbus(dev, (dev->localbus_base & 0xff00) | val);
			break;

		case 0x13:
			if (dev->isa_regs[0x10] == 0x07)
				stpc_remap_host(dev, (dev->host_base & 0x00ff) | (val << 8));
			else if (dev->isa_regs[0x10] == 0x06)
				stpc_remap_localbus(dev, (dev->localbus_base & 0x00ff) | (val << 8));
			break;

		case 0x21:
			val &= 0xfe;
			break;

		case 0x22:
			val &= 0x7f;
			break;

		case 0x25: case 0x26: case 0x27: case 0x28:
			if (dev->isa_offset == 0x28) {
				val &= 0xe3;
				stpc_smram_map(0, smram[0].host_base, smram[0].size, !!(val & 0x80));
			}
			dev->isa_regs[dev->isa_offset] = val;
			stpc_recalcmapping(dev);
			break;

		case 0x29:
			val &= 0x0f;
			break;

		case 0x36:
			val &= 0x3f;
			break;
	}

	dev->isa_regs[dev->isa_offset] = val;
    }
}


static uint8_t
stpc_isa_read(uint16_t addr, void *priv)
{
    stpc_t *dev = (stpc_t *) priv;
    uint8_t ret;

    if (addr == 0x22)
	ret = dev->isa_offset;
    else
	ret = dev->isa_regs[dev->isa_offset];

    stpc_log("STPC: isa_read(%04x) = %02x\n", addr, ret);
    return ret;
}


static void
stpc_reset(void *priv)
{
    stpc_t *dev = (stpc_t *) priv;

    stpc_log("STPC: reset()\n");

    memset(dev->isa_regs, 0, sizeof(dev->isa_regs));
    dev->isa_regs[0x7b] = 0xff;

    io_removehandler(0x22, 2,
		     stpc_isa_read, NULL, NULL, stpc_isa_write, NULL, NULL, dev);
    io_sethandler(0x22, 2,
		  stpc_isa_read, NULL, NULL, stpc_isa_write, NULL, NULL, dev);
}


static void
stpc_setup(stpc_t *dev)
{
    stpc_log("STPC: setup()\n");

    memset(dev, 0, sizeof(stpc_t));

    /* Northbridge */
    dev->pci_conf[0][0x00] = 0x4a;
    dev->pci_conf[0][0x01] = 0x10;
    dev->pci_conf[0][0x02] = 0x0a;
    dev->pci_conf[0][0x03] = 0x02;

    dev->pci_conf[0][0x04] = 0x07;

    dev->pci_conf[0][0x06] = 0x80;
    dev->pci_conf[0][0x07] = 0x02;

    dev->pci_conf[0][0x0b] = 0x06;

    /* Southbridge */
    dev->pci_conf[1][0x00] = 0x4a;
    dev->pci_conf[1][0x01] = 0x10; 
    dev->pci_conf[1][0x02] = 0x10;
    dev->pci_conf[1][0x03] = 0x02;

    dev->pci_conf[1][0x04] = 0x0f;

    dev->pci_conf[1][0x06] = 0x80;
    dev->pci_conf[1][0x07] = 0x02;

    dev->pci_conf[1][0x0a] = 0x01;
    dev->pci_conf[1][0x0b] = 0x06;

    dev->pci_conf[1][0x0e] = 0x40;

    /* IDE */
    dev->pci_conf[2][0x00] = 0x4A;
    dev->pci_conf[2][0x01] = 0x10; 
    dev->pci_conf[2][0x02] = 0x10;
    dev->pci_conf[2][0x03] = 0x02;

    dev->pci_conf[2][0x06] = 0x80;
    dev->pci_conf[2][0x07] = 0x02;

    dev->pci_conf[2][0x09] = 0x8a;
    dev->pci_conf[2][0x0a] = 0x01;
    dev->pci_conf[2][0x0b] = 0x01;

    dev->pci_conf[2][0x0e] = 0x40;

    dev->pci_conf[2][0x10] = 0x01;
    dev->pci_conf[2][0x14] = 0x01;
    dev->pci_conf[2][0x18] = 0x01;
    dev->pci_conf[2][0x1c] = 0x01;

    dev->pci_conf[2][0x40] = 0x60;
    dev->pci_conf[2][0x41] = 0x97;
    dev->pci_conf[2][0x42] = 0x60;
    dev->pci_conf[2][0x43] = 0x97;
    dev->pci_conf[2][0x44] = 0x60;
    dev->pci_conf[2][0x45] = 0x97;
    dev->pci_conf[2][0x46] = 0x60;
    dev->pci_conf[2][0x47] = 0x97;

    /* USB */
}


static void
stpc_close(void *priv)
{
    stpc_t *dev = (stpc_t *) priv;

    stpc_log("STPC: close()\n");

    free(dev);
}


static void *
stpc_init(const device_t *info)
{
    stpc_t *dev = (stpc_t *) malloc(sizeof(stpc_t));

    stpc_log("STPC: init()\n");

    pci_add_card(0x0B, stpc_nb_read, stpc_nb_write, dev);
    pci_add_card(0x0C, stpc_sb_read, stpc_sb_write, dev);
    pci_add_card(0x0D, stpc_ide_read, stpc_ide_write, dev);
    /* USB (Atlas only) = 0x0E */

    stpc_setup(dev);
    stpc_reset(dev);

    smram[0].host_base = 0x000a0000;
    smram[0].ram_base = 0x000a0000;
    smram[0].size = 0x00020000;

    mem_mapping_set_addr(&ram_smram_mapping[0], smram[0].host_base, smram[0].size);
    mem_mapping_set_exec(&ram_smram_mapping[0], ram + smram[0].ram_base);

    stpc_smram_map(0, smram[0].host_base, smram[0].size, 0);
    stpc_smram_map(1, smram[0].host_base, smram[0].size, 1);

    device_add(&port_92_pci_device);

    return dev;
}


const device_t stpc_consumer2_device =
{
    "STPC Consumer-II",
    DEVICE_PCI,
    0,
    stpc_init, 
    stpc_close, 
    stpc_reset,
    NULL,
    NULL,
    NULL,
    NULL
};

const device_t stpc_elite_device =
{
    "STPC Elite",
    DEVICE_PCI,
    0,
    stpc_init, 
    stpc_close, 
    stpc_reset,
    NULL,
    NULL,
    NULL,
    NULL
};

const device_t stpc_atlas_device =
{
    "STPC Atlas",
    DEVICE_PCI,
    0,
    stpc_init, 
    stpc_close, 
    stpc_reset,
    NULL,
    NULL,
    NULL,
    NULL
};