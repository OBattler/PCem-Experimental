/* Copyright holders: Sarah Walker
   see COPYING for more details
*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include "../ibm.h"
#include "../cpu/cpu.h"
#include "../mem.h"
#include "machine.h"


/* Compaq Deskpro 386 remaps RAM from 0xA0000-0xFFFFF to 0xFA0000-0xFFFFFF */

static mem_mapping_t compaq_ram_mapping;

static uint8_t compaq_read_ram(uint32_t addr, void *priv)
{
        addr = (addr & 0x7ffff) + 0x80000;
        addreadlookup(mem_logical_addr, addr);
        return ram[addr];
}


static uint16_t compaq_read_ramw(uint32_t addr, void *priv)
{
        addr = (addr & 0x7ffff) + 0x80000;
        addreadlookup(mem_logical_addr, addr);
        return *(uint16_t *)&ram[addr];
}


static uint32_t compaq_read_raml(uint32_t addr, void *priv)
{
        addr = (addr & 0x7ffff) + 0x80000;
        addreadlookup(mem_logical_addr, addr);
        return *(uint32_t *)&ram[addr];
}


static void compaq_write_ram(uint32_t addr, uint8_t val, void *priv)
{
        addr = (addr & 0x7ffff) + 0x80000;
        addwritelookup(mem_logical_addr, addr);
        mem_write_ramb_page(addr, val, &pages[addr >> 12]);
}


static void compaq_write_ramw(uint32_t addr, uint16_t val, void *priv)
{
        addr = (addr & 0x7ffff) + 0x80000;
        addwritelookup(mem_logical_addr, addr);
        mem_write_ramw_page(addr, val, &pages[addr >> 12]);
}


static void compaq_write_raml(uint32_t addr, uint32_t val, void *priv)
{
        addr = (addr & 0x7ffff) + 0x80000;
        addwritelookup(mem_logical_addr, addr);
        mem_write_raml_page(addr, val, &pages[addr >> 12]);
}


static void compaq_init(void)
{
        mem_mapping_add(&compaq_ram_mapping, 0xfa0000, 0x60000,
                        compaq_read_ram,  compaq_read_ramw,  compaq_read_raml,
                        compaq_write_ram, compaq_write_ramw, compaq_write_raml,
                        ram + 0xa0000,  MEM_MAPPING_INTERNAL, NULL);
}
