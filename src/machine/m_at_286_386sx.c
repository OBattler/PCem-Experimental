/*
 * 86Box	A hypervisor and IBM PC system emulator that specializes in
 *		running old operating systems and software designed for IBM
 *		PC systems and compatibles from 1981 through fairly recent
 *		system designs based on the PCI bus.
 *
 *		This file is part of the 86Box distribution.
 *
 *		Implementation of 286 and 386SX machines.
 *
 *
 *
 * Authors:	Sarah Walker, <http://pcem-emulator.co.uk/>
 *		Miran Grca, <mgrca8@gmail.com>
 *      EngiNerd <webmaster.crrc@yahoo.it>
 *
 *		Copyright 2010-2019 Sarah Walker.
 *		Copyright 2016-2019 Miran Grca.
 *      Copyright 2020 EngiNerd.
 */
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#define HAVE_STDARG_H
#include <86box/86box.h>
#include "cpu.h"
#include <86box/timer.h>
#include <86box/io.h>
#include <86box/device.h>
#include <86box/chipset.h>
#include <86box/keyboard.h>
#include <86box/mem.h>
#include <86box/rom.h>
#include <86box/fdd.h>
#include <86box/fdc.h>
#include <86box/fdc_ext.h>
#include <86box/hdc.h>
#include <86box/sio.h>
#include <86box/serial.h>
#include <86box/video.h>
#include <86box/vid_cga.h>
#include <86box/flash.h>
#include <86box/machine.h>

int
machine_at_mr286_init(const machine_t *model)
{
    int ret;

    ret = bios_load_interleaved("roms/machines/mr286/V000B200-1",
				"roms/machines/mr286/V000B200-2",
				0x000f0000, 65536, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_common_ide_init(model);
    device_add(&keyboard_at_device);

    if (fdc_type == FDC_INTERNAL)
    device_add(&fdc_at_device);

    return ret;
}


static void
machine_at_headland_common_init(int ht386)
{
    device_add(&keyboard_at_ami_device);

    if (fdc_type == FDC_INTERNAL)
    device_add(&fdc_at_device);

    if (ht386)
	device_add(&headland_ht18b_device);
    else
	device_add(&headland_gc10x_device);
}

int
machine_at_tg286m_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/tg286m/ami.bin",
			   0x000f0000, 131072, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_common_ide_init(model);

    machine_at_headland_common_init(0);

    return ret;
}


const device_t *
at_ama932j_get_device(void)
{
    return &oti067_ama932j_device;
}


int
machine_at_ama932j_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/ama932j/ami.bin",
			   0x000f0000, 131072, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_common_ide_init(model);

    machine_at_headland_common_init(1);

    if (gfxcard == VID_INTERNAL)
	device_add(&oti067_ama932j_device);

    return ret;
}


int
machine_at_quadt286_init(const machine_t *model)
{
    int ret;

    ret = bios_load_interleaved("roms/machines/quadt286/QUADT89L.ROM",
				"roms/machines/quadt286/QUADT89H.ROM",
				0x000f0000, 65536, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_common_init(model);
    device_add(&keyboard_at_device);

    if (fdc_type == FDC_INTERNAL)
    device_add(&fdc_at_device);

    device_add(&headland_gc10x_device);

    return ret;
}


int
machine_at_neat_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/dtk386/3cto001.bin",
			   0x000f0000, 65536, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_init(model);

    device_add(&neat_device);

    if (fdc_type == FDC_INTERNAL)
    device_add(&fdc_at_device);

    return ret;
}


int
machine_at_neat_ami_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/ami286/amic206.bin",
			   0x000f0000, 65536, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_common_init(model);

    device_add(&neat_device);

    if (fdc_type == FDC_INTERNAL)
    device_add(&fdc_at_device);

    device_add(&keyboard_at_ami_device);

    return ret;
}


int
machine_at_px286_init(const machine_t *model)
{
    int ret;

	ret = bios_load_linear("roms/machines/px286/KENITEC.BIN",
				0x000f0000, 65536, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_common_init(model);
    device_add(&keyboard_at_device);

    if (fdc_type == FDC_INTERNAL)
    device_add(&fdc_at_device);

    device_add(&neat_device);

    return ret;
}

int
machine_at_micronics386_init(const machine_t *model)
{
    int ret;

    ret = bios_load_interleaved("roms/machines/micronics386/386-Micronics-09-00021-EVEN.BIN",
				"roms/machines/micronics386/386-Micronics-09-00021-ODD.BIN",
				0x000f0000, 131072, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_init(model);

    device_add(&neat_device);

    if (fdc_type == FDC_INTERNAL)
    device_add(&fdc_at_device);

    return ret;
}


static void
machine_at_scat_init(const machine_t *model, int is_v4)
{
    machine_at_common_init(model);
    device_add(&keyboard_at_ami_device);

    if (is_v4)
	device_add(&scat_4_device);
    else
	device_add(&scat_device);
}


static void
machine_at_scatsx_init(const machine_t *model)
{
    machine_at_common_init(model);

    device_add(&keyboard_at_ami_device);

    if (fdc_type == FDC_INTERNAL)
    device_add(&fdc_at_device);

    device_add(&scat_sx_device);
}


int
machine_at_award286_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/award286/award.bin",
			   0x000f0000, 131072, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_scat_init(model, 0);

    if (fdc_type == FDC_INTERNAL)
	device_add(&fdc_at_device);

    return ret;
}

int
machine_at_gdc212m_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/gdc212m/gdc212m_72h.bin",
			   0x000f0000, 65536, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_scat_init(model, 0);

    if (fdc_type == FDC_INTERNAL)
	device_add(&fdc_at_device);

    device_add(&ide_isa_device);

    return ret;
}

int
machine_at_gw286ct_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/gw286ct/2ctc001.bin",
			   0x000f0000, 131072, 0);

    if (bios_only || !ret)
	return ret;

    device_add(&f82c710_device);

    machine_at_scat_init(model, 1);

    device_add(&ide_isa_device);

    return ret;
}


int
machine_at_super286tr_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/super286tr/hyundai_award286.bin",
			   0x000f0000, 131072, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_scat_init(model, 0);

    if (fdc_type == FDC_INTERNAL)
	device_add(&fdc_at_device);

    return ret;
}


int
machine_at_spc4200p_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/spc4200p/u8.01",
			   0x000f0000, 131072, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_scat_init(model, 0);

    if (fdc_type == FDC_INTERNAL)
	device_add(&fdc_at_device);

    return ret;
}


int
machine_at_spc4216p_init(const machine_t *model)
{
    int ret;

    ret = bios_load_interleaved("roms/machines/spc4216p/7101.u8",
				"roms/machines/spc4216p/ac64.u10",
				0x000f0000, 131072, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_scat_init(model, 1);

    if (fdc_type == FDC_INTERNAL)
	device_add(&fdc_at_device);

    return ret;
}


const device_t *
at_spc4620p_get_device(void)
{
    return &ati28800k_spc4620p_device;
}


int
machine_at_spc4620p_init(const machine_t *model)
{
    int ret;

    ret = bios_load_interleaved("roms/machines/spc4620p/31005h.u8",
				"roms/machines/spc4620p/31005h.u10",
				0x000f0000, 131072, 0x8000);

    if (bios_only || !ret)
	return ret;

    machine_at_scat_init(model, 1);

    if (fdc_type == FDC_INTERNAL)
	device_add(&fdc_at_device);

    if (gfxcard == VID_INTERNAL)
	device_add(&ati28800k_spc4620p_device);

    return ret;
}


int
machine_at_kmxc02_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/kmxc02/3ctm005.bin",
			   0x000f0000, 131072, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_scatsx_init(model);

    return ret;
}


int
machine_at_deskmaster286_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/deskmaster286/SAMSUNG-DESKMASTER-28612-ROM.BIN",
			   0x000f0000, 131072, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_scat_init(model, 0);

    if (fdc_type == FDC_INTERNAL)
	device_add(&fdc_at_device);

    return ret;
}


int
machine_at_shuttle386sx_init(const machine_t *model)
{
    int ret;

    ret = bios_load_interleaved("roms/machines/shuttle386sx/386-Shuttle386SX-Even.BIN",
				"roms/machines/shuttle386sx/386-Shuttle386SX-Odd.BIN",
			   0x000f0000, 131072, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_common_init(model);

    device_add(&intel_82335_device);
    device_add(&keyboard_at_ami_device);

    if (fdc_type == FDC_INTERNAL)
    device_add(&fdc_at_device);

    return ret;
}


int
machine_at_adi386sx_init(const machine_t *model)
{
    int ret;

    ret = bios_load_interleaved("roms/machines/adi386sx/3iip001l.bin",
				"roms/machines/adi386sx/3iip001h.bin",
			   0x000f0000, 65536, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_common_init(model);

    device_add(&intel_82335_device);
    device_add(&keyboard_at_ami_device);

    if (fdc_type == FDC_INTERNAL)
    device_add(&fdc_at_device);

    return ret;
}


int
machine_at_wd76c10_init(const machine_t *model)
{
    int ret;

    ret = bios_load_interleaved("roms/machines/megapc/41651-bios lo.u18",
				"roms/machines/megapc/211253-bios hi.u19",
				0x000f0000, 65536, 0x08000);

    if (bios_only || !ret)
	return ret;

    machine_at_common_init(model);

    device_add(&keyboard_ps2_quadtel_device);

    device_add(&wd76c10_device);

    if (gfxcard == VID_INTERNAL)
	device_add(&paradise_wd90c11_megapc_device);

    return ret;
}


int
machine_at_cmdsl386sx16_init(const machine_t *model)
{
    int ret;

    ret = bios_load_interleaved("roms/machines/cmdsl386sx16/cbm-sl386sx-bios-lo-v1.04-390914-04.bin",
				"roms/machines/cmdsl386sx16/cbm-sl386sx-bios-hi-v1.04-390915-04.bin",
				0x000f0000, 65536, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_common_ide_init(model);

    device_add(&keyboard_at_device);

    if (fdc_type == FDC_INTERNAL)
    device_add(&fdc_at_device);

    device_add(&neat_device);
    /* Two serial ports - on the real hardware SL386SX-16, they are on the single UMC UM82C452. */
    device_add_inst(&ns16450_device, 1);
    device_add_inst(&ns16450_device, 2);

    return ret;
}


static void
machine_at_scamp_common_init(int is_ps2, const machine_t *model)
{
    machine_at_common_ide_init(model);

    if(is_ps2)
    	device_add(&keyboard_ps2_ami_device);
    else
    	device_add(&keyboard_at_ami_device);

    if (fdc_type == FDC_INTERNAL)
    device_add(&fdc_at_device);

    device_add(&vlsi_scamp_device);
}


const device_t *
at_cmdsl386sx25_get_device(void)
{
    return &gd5402_onboard_device;
}


int
machine_at_cmdsl386sx25_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/cmdsl386sx25/f000.rom",
			   0x000f0000, 65536, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_scamp_common_init(1, model);

    if (gfxcard == VID_INTERNAL)
	device_add(&gd5402_onboard_device);

    return ret;
}

int
machine_at_dataexpert386sx_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/dataexpert386sx/5e9f20e5ef967717086346.BIN",
			   0x000f0000, 65536, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_scamp_common_init(0, model);

    return ret;
}

const device_t *
at_spc6033p_get_device(void)
{
    return &ati28800k_spc6033p_device;
}

int
machine_at_spc6033p_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/spc6033p/phoenix.bin",
			   0x000f0000, 65536, 0x10000);

    if (bios_only || !ret)
	return ret;

    machine_at_scamp_common_init(1, model);

    if (gfxcard == VID_INTERNAL)
	device_add(&ati28800k_spc6033p_device);

    return ret;
}


int
machine_at_awardsx_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/awardsx/Unknown 386SX OPTi291 - Award (original).BIN",
			   0x000f0000, 65536, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_init(model);

    device_add(&opti291_device);

    if (fdc_type == FDC_INTERNAL)
    device_add(&fdc_at_device);

    return ret;
}

int
machine_at_flytech386_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/flytech386/FLYTECH.BIO",
			   0x000f0000, 65536, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_common_init(model);

    device_add(&ali1217_device);
    device_add(&w83787f_ide_en_device);
    device_add(&keyboard_ps2_device);

    if (gfxcard == VID_INTERNAL)
	device_add(&tvga8900d_device);

    return ret;
}

const device_t *
at_flytech386_get_device(void)
{
    return &tvga8900d_device;
}

#if defined(DEV_BRANCH) && defined(USE_M6117)
int
machine_at_arb1375_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/arb1375/a1375v25.u11-a",
			   0x000e0000, 131072, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_common_init(model);

    device_add(&fdc37c669_device);
    device_add(&keyboard_ps2_ami_pci_device);
    device_add(&ali6117d_device);
    device_add(&sst_flash_29ee010_device);

    return ret;
}


int
machine_at_pja511m_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/pja511m/2006915102435734.rom",
			   0x000e0000, 131072, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_common_init(model);

    device_add_inst(&fdc37c669_device, 1);
    //device_add_inst(&fdc37c669_device, 2); /* enable when dual FDC37C669 is implemented */
    device_add(&keyboard_ps2_ami_pci_device);
    device_add(&ali6117d_device);
    device_add(&sst_flash_29ee010_device);

    return ret;
}
#endif

/*
 * Current bugs: 
 * - ctrl-alt-del produces an 8042 error
 */
int
machine_at_pc8_init(const machine_t *model)
{
    int ret;

    ret = bios_load_interleaved("roms/machines/pc8/ncr_35117_u127_vers.4-2.bin",
				"roms/machines/pc8/ncr_35116_u113_vers.4-2.bin",
				0x000f0000, 65536, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_common_init(model);
    device_add(&keyboard_at_ncr_device);

    if (fdc_type == FDC_INTERNAL)
	device_add(&fdc_at_device);
    
    return ret;
}

/*
 * Current bugs: 
 * - ctrl-alt-del produces an 8042 error
 */
int
machine_at_3302_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/3302/f000-flex_drive_test.bin",
				0x000f0000, 65536, 0);

    if (ret) {
	bios_load_aux_linear("roms/machines/3302/f800-setup_ncr3.5-013190.bin",
			     0x000f8000, 32768, 0);
    }

    if (bios_only || !ret)
	return ret;

    machine_at_common_ide_init(model);
    device_add(&neat_device);
    device_add(&keyboard_at_ncr_device);

    if (fdc_type == FDC_INTERNAL)
	device_add(&fdc_at_device);

    if (gfxcard == VID_INTERNAL)
	device_add(&paradise_pvga1a_ncr3302_device);
    
    return ret;
}

/*
 * Current bugs: 
 * - soft-reboot after saving CMOS settings/pressing ctrl-alt-del produces an 8042 error
 */
int
machine_at_pc916sx_init(const machine_t *model)
{
    int ret;

    ret = bios_load_interleaved("roms/machines/pc916sx/ncr_386sx_u46-17_7.3.bin",
				"roms/machines/pc916sx/ncr_386sx_u12-19_7.3.bin",
				0x000f0000, 65536, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_common_init(model);
    
    device_add(&keyboard_at_ncr_device);
    mem_remap_top(384);

    if (fdc_type == FDC_INTERNAL)
	device_add(&fdc_at_device);
    
    return ret;
}

#if defined(DEV_BRANCH) && defined(USE_OLIVETTI)
int
machine_at_m290_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/m290/m290_pep3_1.25.bin",
				0x000f0000, 65536, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_common_init(model);
    device_add(&keyboard_at_olivetti_device);
    
    if (fdc_type == FDC_INTERNAL)
	device_add(&fdc_at_device);
    
    device_add(&olivetti_eva_device);
    
    return ret;
}
#endif

const device_t *
at_m30008_get_device(void)
{
    return &oti067_m300_device;
}

int
machine_at_m30008_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/m30008/BIOS.ROM",
			   0x000f0000, 65536, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_common_init(model);

    device_add(&opti283_device);
    device_add(&keyboard_ps2_olivetti_device);
    device_add(&pc87310_ide_device);
    
    if (gfxcard == VID_INTERNAL)
	device_add(&oti067_m300_device);

    return ret;
}

/* Almost identical to M300-08, save for CPU speed, VRAM, and BIOS identification string */
int
machine_at_m30015_init(const machine_t *model)
{
    int ret;

    ret = bios_load_linear("roms/machines/m30015/BIOS.ROM",
			   0x000f0000, 65536, 0);

    if (bios_only || !ret)
	return ret;

    machine_at_common_init(model);

    device_add(&opti283_device);
    device_add(&keyboard_ps2_olivetti_device);
    device_add(&pc87310_ide_device);
    
    /* Stock VRAM is maxed out, so no need to expose video card config */
    if (gfxcard == VID_INTERNAL)
	device_add(&oti067_m300_device);

    return ret;
}

