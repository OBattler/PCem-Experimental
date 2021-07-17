/*
 * 86Box	A hypervisor and IBM PC system emulator that specializes in
 *		running old operating systems and software designed for IBM
 *		PC systems and compatibles from 1981 through fairly recent
 *		system designs based on the PCI bus.
 *
 *		This file is part of the 86Box distribution.
 *
 *		Handling of the emulated machines.
 *
 *
 *
 * Authors:	Sarah Walker, <http://pcem-emulator.co.uk/>
 *		Miran Grca, <mgrca8@gmail.com>
 *		Fred N. van Kempen, <decwiz@yahoo.com>
 *
 *		Copyright 2008-2020 Sarah Walker.
 *		Copyright 2016-2020 Miran Grca.
 *		Copyright 2017-2020 Fred N. van Kempen.
 */
#ifndef EMU_MACHINE_H
# define EMU_MACHINE_H


/* Machine feature flags. */
// #define MACHINE_PC		0x00000000	/* PC architecture */
/* Feature flags for features. */
#define MACHINE_NONMI		0x00000001	/* sys does not have NMI's */
/* Feature flags for BUS'es. */
#define MACHINE_BUS_ISA		0x00000004	/* sys has ISA bus */
#define MACHINE_BUS_ISA16	0x00000008	/* sys has ISA16 bus - PC/AT architecture */
#define MACHINE_BUS_CBUS	0x00000010	/* sys has C-BUS bus */
#define MACHINE_BUS_PS2		0x00000020	/* system has PS/2 keyboard and mouse ports */
#define MACHINE_BUS_EISA	0x00000040	/* sys has EISA bus */
#define MACHINE_BUS_VLB		0x00000080	/* sys has VL bus */
#define MACHINE_BUS_MCA		0x00000100	/* sys has MCA bus */
#define MACHINE_BUS_PCI		0x00000200	/* sys has PCI bus */
#define MACHINE_BUS_PCMCIA	0x00000400	/* sys has PCMCIA bus */
#define MACHINE_BUS_AGP		0x00000800	/* sys has AGP bus */
/* Combined flags. */
#define MACHINE_PC		0x00000004	/* sys is PC/XT-compatible (ISA) */
#define MACHINE_AT		0x0000000C	/* sys is AT-compatible (ISA + ISA16) */
#define MACHINE_PC98		0x00000010	/* sys is NEC PC-98x1 series */
#define MACHINE_EISA		0x0000004C	/* sys is AT-compatible with EISA */
#define MACHINE_VLB		0x0000008C	/* sys is AT-compatible with VLB */
#define MACHINE_VLB98		0x00000090	/* sys is NEC PC-98x1 series with VLB (did that even exist?) */
#define MACHINE_VLBE		0x000000CC	/* sys is AT-compatible with EISA and VLB */
#define MACHINE_MCA		0x00000100	/* sys is MCA */
#define MACHINE_PCI		0x0000020C	/* sys is AT-compatible with PCI */
#define MACHINE_PCI98		0x00000210	/* sys is NEC PC-98x1 series with PCI */
#define MACHINE_PCIE		0x0000024C	/* sys is AT-compatible with PCI, and EISA */
#define MACHINE_PCIV		0x0000028C	/* sys is AT-compatible with PCI and VLB */
#define MACHINE_PCIVE		0x000002CC	/* sys is AT-compatible with PCI, VLB, and EISA */
#define MACHINE_PCMCIA		0x00000400	/* sys is AT-compatible laptop with PCMCIA */
#define MACHINE_AGP		0x00000A0C	/* sys is AT-compatible with AGP  */
#define MACHINE_AGP98		0x00000A10	/* sys is NEC PC-98x1 series with AGP (did that even exist?) */
/* Feature flags for miscellaneous internal devices. */
#define MACHINE_VIDEO		0x00001000	/* sys has int video */
#define MACHINE_VIDEO_ONLY	0x00002000	/* sys has fixed video */
#define MACHINE_MOUSE		0x00004000	/* sys has int mouse */
#define MACHINE_SOUND		0x00008000	/* sys has int sound */
#define MACHINE_FDC		0x00010000	/* sys has int FDC */
#define MACHINE_NIC		0x00020000	/* sys has int NIC */
#define MACHINE_GAMEPORT	0x00040000	/* sys has int game port */
/* Combined flags. */
#define MACHINE_VIDEO_FIXED	0x00003000	/* sys has fixed int video */
/* Feature flags for internal storage controllers. */
#define MACHINE_HDC		0x07FC0000	/* sys has int HDC */
#define MACHINE_MFM		0x00100000	/* sys has int MFM/RLL */
#define MACHINE_XTA		0x00200000	/* sys has int XTA */
#define MACHINE_ESDI		0x00400000	/* sys has int ESDI */
#define MACHINE_IDE_PRI		0x00800000	/* sys has int pri IDE/ATAPI */
#define MACHINE_IDE_SEC		0x01000000	/* sys has int sec IDE/ATAPI */
#define MACHINE_IDE_TER		0x02000000	/* sys has int ter IDE/ATAPI */
#define MACHINE_IDE_QUA		0x04000000	/* sys has int qua IDE/ATAPI */
#define MACHINE_SCSI_PRI	0x08000000	/* sys has int pri SCSI */
#define MACHINE_SCSI_SEC	0x10000000	/* sys has int sec SCSI */
#define MACHINE_USB		0x20000000	/* sys has int USB */
/* Combined flags. */
#define MACHINE_IDE		0x00800000	/* sys has int single IDE/ATAPI - mark as pri IDE/ATAPI */
#define MACHINE_IDE_DUAL	0x01800000	/* sys has int dual IDE/ATAPI - mark as both pri and sec IDE/ATAPI */
#define MACHINE_IDE_QUAD	0x07800000	/* sys has int quad IDE/ATAPI - mark as dual + both ter and and qua IDE/ATAPI */
#define MACHINE_SCSI		0x08000000	/* sys has int single SCSI - mark as pri SCSI */
#define MACHINE_SCSI_DUAL	0x18000000	/* sys has int dual SCSI - mark as both pri and sec SCSI */

#define IS_ARCH(m, a)		(machines[m].flags & (a)) ? 1 : 0;
#define IS_AT(m)		((machines[m].flags & 0x00000FC8) && !(machines[m].flags & MACHINE_PC98)) ? 1 : 0;

#define CPU_BLOCK(...)		(const uint8_t[]) {__VA_ARGS__, 0}
#define MACHINE_MULTIPLIER_FIXED -1, -1


enum {
    MACHINE_TYPE_NONE = 0,
    MACHINE_TYPE_8088,
    MACHINE_TYPE_8086,
    MACHINE_TYPE_286,
    MACHINE_TYPE_386SX,
    MACHINE_TYPE_386DX,
    MACHINE_TYPE_386DX_486,
    MACHINE_TYPE_486,
    MACHINE_TYPE_486_S3,
    MACHINE_TYPE_486_MISC,
    MACHINE_TYPE_SOCKET4,
    MACHINE_TYPE_SOCKET5,
    MACHINE_TYPE_SOCKET7_3V,
    MACHINE_TYPE_SOCKET7,
    MACHINE_TYPE_SOCKETS7,
    MACHINE_TYPE_SOCKET8,
    MACHINE_TYPE_SLOT1,
    MACHINE_TYPE_SLOT1_2,
    MACHINE_TYPE_SLOT2,
    MACHINE_TYPE_SOCKET370,
    MACHINE_TYPE_MISC,
    MACHINE_TYPE_MAX
};


typedef struct _machine_type_ {
    const char	*name;
    const char  id;
} machine_type_t;


#ifdef NEW_STRUCT
typedef struct _machine_ {
    const char	*name;
    const char	*internal_name;
    const char  type;
#ifdef EMU_DEVICE_H
    const device_t	*device;
#else
    void	*device;
#endif
    struct {
	const char *name;
#ifdef EMU_CPU_H
	CPU *cpus;
#else
	void *cpus;
#endif
    }		cpu[5];
    int		flags;
    uint32_t	min_ram, max_ram;
    int		ram_granularity;
    int		nvrmask;
} machine_t;
#else
typedef struct _machine_ {
    const char	*name;
    const char	*internal_name;
    const char  type;
    uint32_t	cpu_package;
    const uint8_t *cpu_block;
    uint32_t	cpu_min_bus;
    uint32_t	cpu_max_bus;
    uint16_t	cpu_min_voltage;
    uint16_t	cpu_max_voltage;
    double	cpu_min_multi;
    double	cpu_max_multi;
    int		flags;
    uint32_t	min_ram, max_ram;
    int		ram_granularity;
    int		nvrmask;
    int		(*init)(const struct _machine_ *);
#ifdef EMU_DEVICE_H
    const device_t	*(*get_device)(void);
#else
    void	*get_device;
#endif
} machine_t;
#endif


/* Global variables. */
extern const machine_type_t	machine_types[];
extern const machine_t		machines[];
extern int			bios_only;
extern int			machine;
extern int			AT, PCI;


/* Core functions. */
extern int	machine_count(void);
extern int	machine_available(int m);
extern char	*machine_getname(void);
extern char	*machine_get_internal_name(void);
extern int	machine_get_machine_from_internal_name(char *s);
extern void	machine_init(void);
#ifdef EMU_DEVICE_H
extern const device_t	*machine_getdevice(int m);
#endif
extern char	*machine_get_internal_name_ex(int m);
extern int	machine_get_nvrmask(int m);
extern void	machine_close(void);


/* Initialization functions for boards and systems. */
extern void	machine_common_init(const machine_t *);

/* m_amstrad.c */
extern int	machine_pc1512_init(const machine_t *);
extern int	machine_pc1640_init(const machine_t *);
extern int	machine_pc200_init(const machine_t *);
extern int	machine_ppc512_init(const machine_t *);
extern int	machine_pc2086_init(const machine_t *);
extern int	machine_pc3086_init(const machine_t *);

#ifdef EMU_DEVICE_H
extern const device_t  	*pc1512_get_device(void);
extern const device_t 	*pc1640_get_device(void);
extern const device_t 	*pc200_get_device(void);
extern const device_t 	*ppc512_get_device(void);
extern const device_t 	*pc2086_get_device(void);
extern const device_t 	*pc3086_get_device(void);
#endif

/* m_at.c */
extern void	machine_at_common_init_ex(const machine_t *, int type);
extern void	machine_at_common_init(const machine_t *);
extern void	machine_at_init(const machine_t *);
extern void	machine_at_ps2_init(const machine_t *);
extern void	machine_at_common_ide_init(const machine_t *);
extern void	machine_at_ibm_common_ide_init(const machine_t *);
extern void	machine_at_ide_init(const machine_t *);
extern void	machine_at_ps2_ide_init(const machine_t *);

extern int	machine_at_ibm_init(const machine_t *);

//IBM AT with custom BIOS
extern int	machine_at_ibmatami_init(const machine_t *); // IBM AT with AMI BIOS
extern int	machine_at_ibmatpx_init(const machine_t *); //IBM AT with Phoenix BIOS
extern int	machine_at_ibmatquadtel_init(const machine_t *); // IBM AT with Quadtel BIOS

extern int	machine_at_ibmxt286_init(const machine_t *);

extern int	machine_at_siemens_init(const machine_t *); //Siemens PCD-2L. N82330 discrete machine. It segfaults in some places

#if defined(DEV_BRANCH) && defined(USE_OPEN_AT)
extern int	machine_at_openat_init(const machine_t *);
#endif

/* m_at_286_386sx.c */
extern int	machine_at_tg286m_init(const machine_t *);
extern int	machine_at_ama932j_init(const machine_t *);
extern int	machine_at_px286_init(const machine_t *);
extern int	machine_at_quadt286_init(const machine_t *);
extern int	machine_at_mr286_init(const machine_t *);

extern int	machine_at_neat_init(const machine_t *);
extern int	machine_at_neat_ami_init(const machine_t *);

extern int	machine_at_award286_init(const machine_t *);
extern int	machine_at_gdc212m_init(const machine_t *);
extern int	machine_at_gw286ct_init(const machine_t *);
extern int	machine_at_super286tr_init(const machine_t *);
extern int	machine_at_spc4200p_init(const machine_t *);
extern int	machine_at_spc4216p_init(const machine_t *);
extern int	machine_at_spc4620p_init(const machine_t *);
extern int	machine_at_kmxc02_init(const machine_t *);
extern int	machine_at_deskmaster286_init(const machine_t *);

extern int	machine_at_pc8_init(const machine_t *);
extern int	machine_at_3302_init(const machine_t *);

#if defined(DEV_BRANCH) && defined(USE_OLIVETTI)
extern int	machine_at_m290_init(const machine_t *);
#endif

extern int	machine_at_shuttle386sx_init(const machine_t *);
extern int	machine_at_adi386sx_init(const machine_t *);
extern int	machine_at_cmdsl386sx16_init(const machine_t *);
extern int	machine_at_cmdsl386sx25_init(const machine_t *);
extern int	machine_at_spc6033p_init(const machine_t *);
extern int	machine_at_wd76c10_init(const machine_t *);
extern int	machine_at_flytech386_init(const machine_t *);

extern int	machine_at_awardsx_init(const machine_t *);
#if defined(DEV_BRANCH) && defined(USE_M6117)
extern int	machine_at_arb1375_init(const machine_t *);
extern int	machine_at_pja511m_init(const machine_t *);
#endif

extern int  machine_at_pc916sx_init(const machine_t *);

extern int  machine_at_m30008_init(const machine_t *);
extern int  machine_at_m30015_init(const machine_t *);

#ifdef EMU_DEVICE_H
extern const device_t	*at_ama932j_get_device(void);
extern const device_t	*at_flytech386_get_device(void);
extern const device_t	*at_cmdsl386sx25_get_device(void);
extern const device_t	*at_spc4620p_get_device(void);
extern const device_t	*at_spc6033p_get_device(void);
extern const device_t	*at_m30008_get_device(void);
#endif

/* m_at_386dx_486.c */
extern int	machine_at_acc386_init(const machine_t *);
extern int	machine_at_asus386_init(const machine_t *);
extern int	machine_at_ecs386_init(const machine_t *);
extern int	machine_at_spc6000a_init(const machine_t *);
extern int	machine_at_micronics386_init(const machine_t *);

extern int	machine_at_rycleopardlx_init(const machine_t *);

extern int	machine_at_486vchd_init(const machine_t *);

extern int	machine_at_cs4031_init(const machine_t *);

extern int	machine_at_pb410a_init(const machine_t *);

extern int	machine_at_acera1g_init(const machine_t *);
extern int	machine_at_ali1429_init(const machine_t *);
extern int	machine_at_winbios1429_init(const machine_t *);

extern int	machine_at_opti495_init(const machine_t *);
extern int	machine_at_opti495_ami_init(const machine_t *);
extern int	machine_at_opti495_mr_init(const machine_t *);

extern int	machine_at_vect486vl_init(const machine_t *);
extern int	machine_at_d824_init(const machine_t *);

extern int	machine_at_pcs46c_init(const machine_t *);

extern int	machine_at_403tg_init(const machine_t *);
#if defined(DEV_BRANCH) && defined(USE_OPTI822)  
extern int	machine_at_pc330_6573_init(const machine_t *);
#endif
extern int	machine_at_mvi486_init(const machine_t *);

extern int	machine_at_sis401_init(const machine_t *);
extern int	machine_at_isa486_init(const machine_t *);
extern int	machine_at_av4_init(const machine_t *);
extern int	machine_at_valuepoint433_init(const machine_t *);

extern int	machine_at_vli486sv2g_init(const machine_t *);
extern int	machine_at_ami471_init(const machine_t *);
extern int	machine_at_dtk486_init(const machine_t *);
extern int	machine_at_px471_init(const machine_t *);
extern int	machine_at_win471_init(const machine_t *);
extern int	machine_at_vi15g_init(const machine_t *);

extern int	machine_at_r418_init(const machine_t *);
extern int	machine_at_ls486e_init(const machine_t *);
extern int	machine_at_4dps_init(const machine_t *);
extern int	machine_at_4sa2_init(const machine_t *);
extern int	machine_at_m4li_init(const machine_t *);
extern int	machine_at_alfredo_init(const machine_t *);
extern int	machine_at_486sp3_init(const machine_t *);
extern int	machine_at_486sp3c_init(const machine_t *);
extern int	machine_at_486sp3g_init(const machine_t *);
extern int	machine_at_486ap4_init(const machine_t *);
extern int	machine_at_486vipio2_init(const machine_t *);
extern int	machine_at_abpb4_init(const machine_t *);
extern int	machine_at_win486pci_init(const machine_t *);

extern int	machine_at_atc1415_init(const machine_t *);
extern int	machine_at_ecs486_init(const machine_t *);
extern int	machine_at_hot433_init(const machine_t *);

extern int	machine_at_itoxstar_init(const machine_t *);
extern int	machine_at_arb1479_init(const machine_t *);
extern int	machine_at_pcm9340_init(const machine_t *);
extern int	machine_at_pcm5330_init(const machine_t *);

#ifdef EMU_DEVICE_H
extern const device_t 	*at_acera1g_get_device(void);
extern const device_t 	*at_vect486vl_get_device(void);
extern const device_t 	*at_d824_get_device(void);
extern const device_t 	*at_pcs46c_get_device(void);
extern const device_t 	*at_valuepoint433_get_device(void);
#endif

/* m_at_commodore.c */
extern int	machine_at_cmdpc_init(const machine_t *);

/* m_at_compaq.c */
extern int	machine_at_portableii_init(const machine_t *);
extern int	machine_at_portableiii_init(const machine_t *);
extern int	machine_at_portableiii386_init(const machine_t *);
#ifdef EMU_DEVICE_H
extern const device_t 	*at_cpqiii_get_device(void);
#endif

/* m_at_socket4_5.c */
extern int	machine_at_excalibur_init(const machine_t *);

extern int	machine_at_pat54pv_init(const machine_t *);

#if defined(DEV_BRANCH) && defined(USE_OPTI822)  
extern int	machine_at_hot543_init(const machine_t *);
extern int	machine_at_p54vl_init(const machine_t *);
#endif

extern int	machine_at_batman_init(const machine_t *);
extern int	machine_at_ambradp60_init(const machine_t *);
extern int	machine_at_dellxp60_init(const machine_t *);
extern int	machine_at_opti560l_init(const machine_t *);
extern int	machine_at_valuepointp60_init(const machine_t *);
extern int	machine_at_p5mp3_init(const machine_t *);
extern int	machine_at_pb520r_init(const machine_t *);
extern int	machine_at_586mc1_init(const machine_t *);

extern int	machine_at_plato_init(const machine_t *);
extern int	machine_at_ambradp90_init(const machine_t *);
extern int	machine_at_430nx_init(const machine_t *);

extern int	machine_at_p54tp4xe_init(const machine_t *);
extern int	machine_at_endeavor_init(const machine_t *);
extern int	machine_at_zappa_init(const machine_t *);
extern int	machine_at_mb500n_init(const machine_t *);
extern int	machine_at_apollo_init(const machine_t *);
extern int	machine_at_vectra54_init(const machine_t *);
extern int	machine_at_powermatev_init(const machine_t *);
extern int	machine_at_acerv30_init(const machine_t *);

extern int	machine_at_p5sp4_init(const machine_t *);
extern int	machine_at_p54sp4_init(const machine_t *);
extern int	machine_at_sq588_init(const machine_t *);

extern int	machine_at_hot539_init(const machine_t *);

#ifdef EMU_DEVICE_H
extern const device_t	*at_endeavor_get_device(void);
#define at_vectra54_get_device at_endeavor_get_device
extern const device_t	*at_pb520r_get_device(void);
extern const device_t	*at_thor_get_device(void);
#endif

/* m_at_socket7_s7.c */
extern int	machine_at_ap5s_init(const machine_t *);

extern int	machine_at_chariot_init(const machine_t *);
extern int	machine_at_mr586_init(const machine_t *);
extern int	machine_at_thor_init(const machine_t *);
extern int	machine_at_gw2katx_init(const machine_t *);
extern int	machine_at_mrthor_init(const machine_t *);
extern int	machine_at_pb640_init(const machine_t *);

extern int	machine_at_acerm3a_init(const machine_t *);
extern int	machine_at_acerv35n_init(const machine_t *);
extern int	machine_at_ap53_init(const machine_t *);
extern int	machine_at_p55t2p4_init(const machine_t *);
extern int	machine_at_p55t2s_init(const machine_t *);
extern int	machine_at_8500tuc_init(const machine_t *);
extern int	machine_at_m7shi_init(const machine_t *);
extern int	machine_at_tc430hx_init(const machine_t *);
extern int	machine_at_equium5200_init(const machine_t *);
extern int	machine_at_pcv240_init(const machine_t *);
extern int	machine_at_p65up5_cp55t2d_init(const machine_t *);

extern int	machine_at_mb520n_init(const machine_t *);
extern int	machine_at_p55tvp4_init(const machine_t *);
extern int	machine_at_p55va_init(const machine_t *);
extern int	machine_at_i430vx_init(const machine_t *);
extern int	machine_at_5ivg_init(const machine_t *);
extern int	machine_at_brio80xx_init(const machine_t *);
extern int	machine_at_8500tvxa_init(const machine_t *);
extern int	machine_at_presario2240_init(const machine_t *);
extern int	machine_at_presario4500_init(const machine_t *);
extern int	machine_at_gw2kte_init(const machine_t *);
extern int	machine_at_pb680_init(const machine_t *);

extern int	machine_at_nupro592_init(const machine_t *);
extern int	machine_at_tx97_init(const machine_t *);
extern int	machine_at_ym430tx_init(const machine_t *);
#if defined(DEV_BRANCH) && defined(NO_SIO)
extern int	machine_at_an430tx_init(const machine_t *);
#endif
extern int	machine_at_mb540n_init(const machine_t *);
extern int	machine_at_p5mms98_init(const machine_t *);

extern int	machine_at_r534f_init(const machine_t *);
extern int	machine_at_ms5146_init(const machine_t *);

#if defined(DEV_BRANCH) && defined(USE_M154X)
extern int	machine_at_m560_init(const machine_t *);
extern int	machine_at_ms5164_init(const machine_t *);
#endif

extern int	machine_at_ficva502_init(const machine_t *);

extern int	machine_at_ficpa2012_init(const machine_t *);

extern int	machine_at_sp97xv_init(const machine_t *);
extern int	machine_at_m571_init(const machine_t *);

#ifdef EMU_DEVICE_H
extern const device_t 	*at_thor_get_device(void);
extern const device_t	*at_pb640_get_device(void);
#endif

/* m_at_super7_ss7.c */
extern int	machine_at_ax59pro_init(const machine_t *);
extern int	machine_at_mvp3_init(const machine_t *);
extern int	machine_at_ficva503a_init(const machine_t *);

/* m_at_socket8.c */
#if defined(DEV_BRANCH) && defined(USE_I450KX)
extern int	machine_at_p6rp4_init(const machine_t *);
#endif

extern int	machine_at_686nx_init(const machine_t *);
extern int	machine_at_v60n_init(const machine_t *);
extern int	machine_at_vs440fx_init(const machine_t *);
extern int	machine_at_ap440fx_init(const machine_t *);
extern int	machine_at_mb600n_init(const machine_t *);
extern int	machine_at_8500ttc_init(const machine_t *);
extern int	machine_at_m6mi_init(const machine_t *);
#ifdef EMU_DEVICE_H
extern void	machine_at_p65up5_common_init(const machine_t *, const device_t *northbridge);
#endif
extern int	machine_at_p65up5_cp6nd_init(const machine_t *);

/* m_at_slot1.c */
extern int	machine_at_p65up5_cpknd_init(const machine_t *);
extern int	machine_at_kn97_init(const machine_t *);

extern int	machine_at_lx6_init(const machine_t *);
extern int	machine_at_spitfire_init(const machine_t *);

extern int	machine_at_p6i440e2_init(const machine_t *);

extern int	machine_at_p2bls_init(const machine_t *);
extern int	machine_at_p3bf_init(const machine_t *);
extern int	machine_at_bf6_init(const machine_t *);
extern int	machine_at_ax6bc_init(const machine_t *);
extern int	machine_at_atc6310bxii_init(const machine_t *);
extern int	machine_at_686bx_init(const machine_t *);
extern int	machine_at_tsunamiatx_init(const machine_t *);
extern int	machine_at_p6sba_init(const machine_t *);
#if defined(DEV_BRANCH) && defined(NO_SIO)
extern int	machine_at_ergox365_init(const machine_t *);
#endif
extern int	machine_at_ficka6130_init(const machine_t *);
extern int	machine_at_p3v133_init(const machine_t *);
extern int	machine_at_p3v4x_init(const machine_t *);

extern int	machine_at_vei8_init(const machine_t *);

extern int	machine_at_borapro_init(const machine_t *);
extern int	machine_at_ms6168_init(const machine_t *);

#ifdef EMU_DEVICE_H
extern const device_t 	*at_tsunamiatx_get_device(void);
#endif

/* m_at_slot2.c */
extern int	machine_at_6gxu_init(const machine_t *);
extern int	machine_at_s2dge_init(const machine_t *);
extern int	machine_at_fw6400gx_init(const machine_t *);

/* m_at_socket370.c */
extern int	machine_at_s370slm_init(const machine_t *);

extern int	machine_at_cubx_init(const machine_t *);
extern int	machine_at_atc7020bxii_init(const machine_t *);
extern int	machine_at_ambx133_init(const machine_t *);
extern int	machine_at_awo671r_init(const machine_t *);
extern int	machine_at_63a_init(const machine_t *);
extern int	machine_at_s370sba_init(const machine_t *);
extern int	machine_at_apas3_init(const machine_t *);
extern int	machine_at_wcf681_init(const machine_t *);
extern int	machine_at_cuv4xls_init(const machine_t *);
extern int	machine_at_6via90ap_init(const machine_t *);
extern int	machine_at_603tcf_init(const machine_t *);
extern int	machine_at_trinity371_init(const machine_t *);
extern int	machine_at_p6bap_init(const machine_t *);

/* m_at_misc.c */
extern int	machine_at_vpc2007_init(const machine_t *);

/* m_at_t3100e.c */
extern int	machine_at_t3100e_init(const machine_t *);

/* m_europc.c */
extern int	machine_europc_init(const machine_t *);
#ifdef EMU_DEVICE_H
extern const device_t europc_device;
#endif

/* m_xt_olivetti.c */
extern int	machine_xt_m24_init(const machine_t *);
#ifdef EMU_DEVICE_H
extern const 	device_t *m24_get_device(void);
#endif
extern int	machine_xt_m240_init(const machine_t *);
extern int	machine_xt_m19_init(const machine_t *);
#ifdef EMU_DEVICE_H
extern const 	device_t *m19_get_device(void);
#endif

/* m_pcjr.c */
extern int	machine_pcjr_init(const machine_t *);

#ifdef EMU_DEVICE_H
extern const device_t	*pcjr_get_device(void);
#endif

/* m_ps1.c */
extern int	machine_ps1_m2011_init(const machine_t *);
extern int	machine_ps1_m2121_init(const machine_t *);

/* m_ps1_hdc.c */
#ifdef EMU_DEVICE_H
extern void	ps1_hdc_inform(void *, uint8_t *);
extern const device_t ps1_hdc_device;
#endif

/* m_ps2_isa.c */
extern int	machine_ps2_m30_286_init(const machine_t *);

/* m_ps2_mca.c */
extern int	machine_ps2_model_50_init(const machine_t *);
extern int	machine_ps2_model_55sx_init(const machine_t *);
extern int	machine_ps2_model_70_type3_init(const machine_t *);
extern int	machine_ps2_model_80_init(const machine_t *);
extern int	machine_ps2_model_80_axx_init(const machine_t *);

/* m_tandy.c */
extern int	tandy1k_eeprom_read(void);
extern int	machine_tandy_init(const machine_t *);
extern int	machine_tandy1000hx_init(const machine_t *);
extern int	machine_tandy1000sl2_init(const machine_t *);

#ifdef EMU_DEVICE_H
extern const device_t	*tandy1k_get_device(void);
extern const device_t	*tandy1k_hx_get_device(void);
extern const device_t   *tandy1k_sl_get_device(void);
#endif

/* m_xt.c */
extern int	machine_pc_init(const machine_t *);
extern int	machine_pc82_init(const machine_t *);

extern int	machine_xt_init(const machine_t *);
extern int	machine_genxt_init(const machine_t *);

extern int	machine_xt86_init(const machine_t *);

extern int	machine_xt_americxt_init(const machine_t *);
extern int	machine_xt_amixt_init(const machine_t *);
extern int	machine_xt_dtk_init(const machine_t *);
extern int	machine_xt_jukopc_init(const machine_t *);
extern int	machine_xt_openxt_init(const machine_t *);
extern int	machine_xt_pcxt_init(const machine_t *);
extern int	machine_xt_pxxt_init(const machine_t *);
extern int	machine_xt_pc4i_init(const machine_t *);
extern int	machine_xt_mpc1600_init(const machine_t *);
extern int	machine_xt_pcspirit_init(const machine_t *);
extern int	machine_xt_pc700_init(const machine_t *);

extern int	machine_xt_iskra3104_init(const machine_t *);

#if defined(DEV_BRANCH) && defined(USE_HEDAKA)
extern int 	machine_xt_hed919_init(const machine_t *);
#endif

/* m_xt_compaq.c */
extern int	machine_xt_compaq_deskpro_init(const machine_t *);
extern int	machine_xt_compaq_portable_init(const machine_t *);

/* m_xt_laserxt.c */
#if defined(DEV_BRANCH) && defined(USE_LASERXT)
extern int	machine_xt_laserxt_init(const machine_t *);
extern int	machine_xt_lxt3_init(const machine_t *);
#endif

/* m_xt_philips.c */
extern int	machine_xt_p3105_init(const machine_t *);
extern int	machine_xt_p3120_init(const machine_t *);
/* m_xt_t1000.c */
extern int	machine_xt_t1000_init(const machine_t *);
extern int	machine_xt_t1200_init(const machine_t *);

#ifdef EMU_DEVICE_H
extern const device_t	*t1000_get_device(void);
extern const device_t	*t1200_get_device(void);
#endif

/* m_xt_zenith.c */
extern int	machine_xt_z184_init(const machine_t *);
#ifdef EMU_DEVICE_H
extern const 	device_t *z184_get_device(void);
#endif
extern int	machine_xt_z151_init(const machine_t *);
extern int	machine_xt_z159_init(const machine_t *);

/* m_xt_xi8088.c */
extern int	machine_xt_xi8088_init(const machine_t *);

#ifdef EMU_DEVICE_H
extern const device_t	*xi8088_get_device(void);
#endif


#endif	/*EMU_MACHINE_H*/
