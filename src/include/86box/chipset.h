/*
 * 86Box	A hypervisor and IBM PC system emulator that specializes in
 *		running old operating systems and software designed for IBM
 *		PC systems and compatibles from 1981 through fairly recent
 *		system designs based on the PCI bus.
 *
 *		This file is part of the 86Box distribution.
 *
 *		Handling of the emulated chipsets.
 *
 *
 *
 * Authors:	Miran Grca, <mgrca8@gmail.com>
 *
 *		Copyright 2019,2020 Miran Grca.
 */
#ifndef EMU_CHIPSET_H
# define EMU_CHIPSET_H


/* ACC */
extern const device_t	acc2168_device;

/* ALi */
extern const device_t	ali1217_device;
extern const device_t	ali1429_device;
extern const device_t	ali1429g_device;
extern const device_t	ali1489_device;
#if defined(DEV_BRANCH) && defined(USE_M154X)
extern const device_t	ali1531_device;
extern const device_t	ali1543_device;
#endif
#if defined(DEV_BRANCH) && defined(USE_M6117)
extern const device_t	ali6117d_device;
#endif

/* AMD */
extern const device_t	amd640_device;

/* C&T */
extern const device_t	neat_device;
extern const device_t	scat_device;
extern const device_t	scat_4_device;
extern const device_t	scat_sx_device;
extern const device_t	cs8230_device;
extern const device_t   cs4031_device;

/* ETEQ */
extern const device_t   et6000_device;

/* G2 */
extern const device_t   gc100_device;
extern const device_t   gc100a_device;

/* Headland */
extern const device_t	headland_gc10x_device;
extern const device_t	headland_ht18a_device;
extern const device_t	headland_ht18b_device;
extern const device_t	headland_ht18c_device;

/* Intel */
extern const device_t   intel_82335_device;
extern const device_t	i420ex_device;
extern const device_t	i420tx_device;
extern const device_t	i420zx_device;
extern const device_t	i430lx_device;
extern const device_t	i430nx_device;
extern const device_t	i430fx_device;
extern const device_t	i430fx_rev02_device;
extern const device_t	i430hx_device;
extern const device_t	i430vx_device;
extern const device_t	i430tx_device;
extern const device_t	i440fx_device;
extern const device_t	i440lx_device;
extern const device_t	i440ex_device;
extern const device_t	i440bx_device;
extern const device_t	i440gx_device;
extern const device_t	i440zx_device;

#if defined(DEV_BRANCH) && defined(USE_I450KX)
extern const device_t	i450kx_device;
#endif

extern const device_t	sio_device;
extern const device_t	sio_zb_device;

extern const device_t	piix_device;
extern const device_t	piix_rev02_device;
extern const device_t	piix3_device;
extern const device_t	piix4_device;
extern const device_t	piix4e_device;
extern const device_t	slc90e66_device;

extern const device_t	ioapic_device;

/* OPTi */
extern const device_t	opti283_device;
extern const device_t	opti291_device;
extern const device_t	opti493_device;
extern const device_t	opti495_device;
extern const device_t	opti802g_device;
extern const device_t	opti822_device;
extern const device_t	opti895_device;

extern const device_t	opti5x7_device;

/* SiS */
extern const device_t   rabbit_device;
extern const device_t	sis_85c401_device;
extern const device_t	sis_85c460_device;
extern const device_t	sis_85c461_device;
extern const device_t	sis_85c471_device;
extern const device_t	sis_85c496_device;
extern const device_t	sis_85c496_ls486e_device;
extern const device_t	sis_85c50x_device;
extern const device_t	sis_5511_device;
extern const device_t	sis_5571_device;
extern const device_t	sis_5598_device;

/* ST */
extern const device_t	stpc_client_device;
extern const device_t	stpc_consumer2_device;
extern const device_t	stpc_elite_device;
extern const device_t	stpc_atlas_device;
extern const device_t	stpc_serial_device;
extern const device_t	stpc_lpt_device;

/* UMC */
extern const device_t	umc_hb4_device;

extern const device_t	umc_8886f_device;
extern const device_t	umc_8886af_device;

/* VIA */
extern const device_t   via_vt82c49x_device;
extern const device_t   via_vt82c49x_pci_device;
extern const device_t   via_vt82c49x_pci_ide_device;
extern const device_t   via_vt82c505_device;
extern const device_t	via_vpx_device;
extern const device_t	via_vp3_device;
extern const device_t	via_mvp3_device;
extern const device_t	via_apro_device;
extern const device_t	via_apro133_device;
extern const device_t	via_apro133a_device;
extern const device_t	via_vt8601_device;
extern const device_t	via_vt82c586b_device;
extern const device_t	via_vt82c596a_device;
extern const device_t	via_vt82c596b_device;
extern const device_t	via_vt82c686a_device;
extern const device_t	via_vt82c686b_device;
extern const device_t	via_vt8231_device;

/* VLSI */
extern const device_t   vl82c480_device;
extern const device_t   vlsi_scamp_device;

/* WD */
extern const device_t	wd76c10_device;

/* Miscellaneous Hardware */
extern const device_t   phoenix_486_jumper_device;
extern const device_t	vpc2007_device;

#if defined(DEV_BRANCH) && defined(USE_OLIVETTI)
extern const device_t   olivetti_eva_device;
#endif
#endif	/*EMU_CHIPSET_H*/
