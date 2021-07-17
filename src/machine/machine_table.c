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
 * NOTES:	OpenAT wip for 286-class machine with open BIOS.
 *		PS2_M80-486 wip, pending receipt of TRM's for machine.
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
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include <86box/86box.h>
#include "cpu.h"
#include <86box/mem.h>
#include <86box/rom.h>
#include <86box/device.h>
#include <86box/machine.h>


const machine_type_t machine_types[] = {
    { "None",				MACHINE_TYPE_NONE	},
    { "8088",				MACHINE_TYPE_8088	},
    { "8086",				MACHINE_TYPE_8086	},
    { "80286",				MACHINE_TYPE_286	},
    { "i386SX",				MACHINE_TYPE_386SX	},
    { "i386DX",				MACHINE_TYPE_386DX	},
    { "i386DX/i486",			MACHINE_TYPE_386DX_486	},
    { "i486 (Socket 168 and 1)",	MACHINE_TYPE_486	},
    { "i486 (Socket 2 and 3)",		MACHINE_TYPE_486_S3	},
    { "i486 (Miscellaneous)",		MACHINE_TYPE_486_MISC	},
    { "Socket 4",			MACHINE_TYPE_SOCKET4	},
    { "Socket 5",			MACHINE_TYPE_SOCKET5	},
    { "Socket 7 (Single Voltage)",	MACHINE_TYPE_SOCKET7_3V	},
    { "Socket 7 (Dual Voltage)",	MACHINE_TYPE_SOCKET7	},
    { "Super Socket 7",			MACHINE_TYPE_SOCKETS7	},
    { "Socket 8",			MACHINE_TYPE_SOCKET8	},
    { "Slot 1",				MACHINE_TYPE_SLOT1	},
    { "Slot 1/2",			MACHINE_TYPE_SLOT1_2	},
    { "Slot 2",				MACHINE_TYPE_SLOT2	},
    { "Socket 370",			MACHINE_TYPE_SOCKET370	},
    { "Miscellaneous",			MACHINE_TYPE_MISC    	}
};


const machine_t machines[] = {
    /* 8088 Machines */
    { "[8088] IBM PC (1981)",			"ibmpc",		MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									   16,    64,  16,    0,		      machine_pc_init, NULL			},
    { "[8088] IBM PC (1982)",			"ibmpc82",		MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									  256,   256, 256,    0,		    machine_pc82_init, NULL			},
    { "[8088] IBM PCjr",			"ibmpcjr",		MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 4772728, 4772728, 0, 0, 0, 0,									MACHINE_PC | MACHINE_VIDEO_FIXED,						  128,   640, 128,    0,		    machine_pcjr_init, pcjr_get_device		},
    { "[8088] IBM XT (1982)",			"ibmxt",		MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									   64,   256,  64,    0,		      machine_xt_init, NULL			},
    { "[8088] IBM XT (1986)",			"ibmxt86",		MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									  256,   640,  64,    0,		    machine_xt86_init, NULL			},
    { "[8088] American XT Computer",		"americxt",		MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									   64,   640,  64,    0,	     machine_xt_americxt_init, NULL			},
    { "[8088] AMI XT clone",			"amixt",		MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									   64,   640,  64,    0,		machine_xt_amixt_init, NULL			},
    { "[8088] Columbia Data Products MPC-1600", "mpc1600",		MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									  128,   512,  64,    0,	      machine_xt_mpc1600_init, NULL			},
    { "[8088] Compaq Portable",			"portable",		MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									  128,   640, 128,    0,      machine_xt_compaq_portable_init, NULL			},
    { "[8088] DTK XT clone",			"dtk",			MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									   64,   640,  64,    0,		  machine_xt_dtk_init, NULL			},
    { "[8088] Eagle PC Spirit",			"pcspirit",		MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									  128,   640,  64,    0,	     machine_xt_pcspirit_init, NULL			},
    { "[8088] Generic XT clone",		"genxt",		MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									   64,   640,  64,    0,		   machine_genxt_init, NULL			},
    { "[8088] Juko XT clone",			"jukopc",		MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									   64,   640,  64,    0,	       machine_xt_jukopc_init, NULL			},
    { "[8088] Multitech PC-700",		"pc700",		MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									  128,   640,  64,    0,		machine_xt_pc700_init, NULL			},
    { "[8088] NCR PC4i",			"pc4i",			MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									  256,   640, 256,    0,		 machine_xt_pc4i_init, NULL			},
    { "[8088] Olivetti M19",			"m19",			MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 4772728, 7159092, 0, 0, 0, 0,									MACHINE_PC | MACHINE_VIDEO_FIXED,						  256,   640, 256,    0,		  machine_xt_m19_init, m19_get_device		},
    { "[8088] OpenXT",				"openxt",		MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									   64,   640,  64,    0,	       machine_xt_openxt_init, NULL			},
    { "[8088] Philips P3105/NMS9100",		"p3105",		MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC | MACHINE_XTA,							  256,   768, 256,    0,		machine_xt_p3105_init, NULL			},
    { "[8088] Phoenix XT clone",		"pxxt",			MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									   64,   640,  64,    0,		 machine_xt_pxxt_init, NULL			},
    { "[8088] Schneider EuroPC",		"europc",		MACHINE_TYPE_8088,		CPU_PKG_8088_EUROPC, 0, 0, 0, 0, 0, 0, 0,									MACHINE_PC | MACHINE_XTA | MACHINE_MOUSE,					  512,   640, 128,   15,		  machine_europc_init, NULL			},
    { "[8088] Super PC/Turbo XT",		"pcxt",			MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									   64,   640,  64,    0,		 machine_xt_pcxt_init, NULL			},
    { "[8088] Tandy 1000",			"tandy",		MACHINE_TYPE_8088,		CPU_PKG_8088_EUROPC, 0, 0, 0, 0, 0, 0, 0,									MACHINE_PC | MACHINE_VIDEO_FIXED,						  128,   640, 128,    0,		   machine_tandy_init, tandy1k_get_device	},
    { "[8088] Tandy 1000 HX",			"tandy1000hx",		MACHINE_TYPE_8088,		CPU_PKG_8088_EUROPC, 0, 0, 0, 0, 0, 0, 0,									MACHINE_PC | MACHINE_VIDEO_FIXED,						  256,   640, 128,    0,	     machine_tandy1000hx_init, tandy1k_hx_get_device	},
    { "[8088] Toshiba T1000",			"t1000",		MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC | MACHINE_VIDEO,							  512,  1280, 768,   63,		machine_xt_t1000_init, t1000_get_device		},
#if defined(DEV_BRANCH) && defined(USE_LASERXT)
    { "[8088] VTech Laser Turbo XT",		"ltxt",			MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									  256,   640, 256,    0,	      machine_xt_laserxt_init, NULL			},
#endif
    { "[8088] Xi8088",				"xi8088",		MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_BUS_PS2,							   64,  1024, 128,  127,	       machine_xt_xi8088_init, xi8088_get_device	},
    { "[8088] Zenith Data Systems Z-151/152/161","zdsz151",		MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									  128,   640,  64,    0,		 machine_xt_z151_init, NULL			},
    { "[8088] Zenith Data Systems Z-159",	"zdsz159",		MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									  128,   640,  64,    0,		 machine_xt_z159_init, NULL			},
    { "[8088] Zenith Data Systems SupersPort (Z-184)","zdsupers",	MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC | MACHINE_VIDEO_FIXED,						  128,   640, 128,    0,		 machine_xt_z184_init, z184_get_device		},
    { "[GC100A] Philips P3120",			"p3120",		MACHINE_TYPE_8088,		CPU_PKG_8088, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC | MACHINE_XTA,							  256,   768, 256,    0,		machine_xt_p3120_init, NULL			},
    
    /* 8086 Machines */
    { "[8086] Amstrad PC1512",			"pc1512",		MACHINE_TYPE_8086,		CPU_PKG_8086, 0, 8000000, 8000000, 0, 0, 0, 0,									MACHINE_PC | MACHINE_VIDEO_FIXED | MACHINE_MOUSE,				  512,   640, 128,   63,		  machine_pc1512_init, pc1512_get_device	},
    { "[8086] Amstrad PC1640",			"pc1640",		MACHINE_TYPE_8086,		CPU_PKG_8086, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC | MACHINE_VIDEO | MACHINE_MOUSE,					  640,   640, 640,   63,		  machine_pc1640_init, pc1640_get_device	},
    { "[8086] Amstrad PC2086",			"pc2086",		MACHINE_TYPE_8086,		CPU_PKG_8086, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC | MACHINE_VIDEO_FIXED | MACHINE_MOUSE,				  640,   640, 640,   63,		  machine_pc2086_init, pc2086_get_device	},
    { "[8086] Amstrad PC3086",			"pc3086",		MACHINE_TYPE_8086,		CPU_PKG_8086, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC | MACHINE_VIDEO_FIXED | MACHINE_MOUSE,				  640,   640, 640,   63,		  machine_pc3086_init, pc3086_get_device	},
    { "[8086] Amstrad PC20(0)",			"pc200",		MACHINE_TYPE_8086,		CPU_PKG_8086, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC | MACHINE_VIDEO | MACHINE_MOUSE | MACHINE_NONMI,			  512,   640, 128,   63,		   machine_pc200_init, pc200_get_device		},
    { "[8086] Amstrad PPC512/640",		"ppc512",		MACHINE_TYPE_8086,		CPU_PKG_8086, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC | MACHINE_VIDEO | MACHINE_MOUSE | MACHINE_NONMI,			  512,   640, 128,   63,		  machine_ppc512_init, ppc512_get_device	},
    { "[8086] Compaq Deskpro",			"deskpro",		MACHINE_TYPE_8086,		CPU_PKG_8086, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									  128,   640, 128,    0,       machine_xt_compaq_deskpro_init, NULL			},
    { "[8086] Olivetti M21/24/24SP",		"m24",			MACHINE_TYPE_8086,		CPU_PKG_8086, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC | MACHINE_VIDEO | MACHINE_MOUSE,					  128,   640, 128,    0,		  machine_xt_m24_init, m24_get_device		},
    { "[8086] Olivetti M240",			"m240",			MACHINE_TYPE_8086,		CPU_PKG_8086, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									  128,   640, 128,    0,		 machine_xt_m240_init, NULL			},
    { "[8086] Schetmash Iskra-3104",		"iskra3104",		MACHINE_TYPE_8086,		CPU_PKG_8086, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									  128,   640, 128,    0,	    machine_xt_iskra3104_init, NULL			},
    { "[8086] Tandy 1000 SL/2",			"tandy1000sl2",		MACHINE_TYPE_8086,		CPU_PKG_8086, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC | MACHINE_VIDEO_FIXED,						  512,   768, 128,    0,	    machine_tandy1000sl2_init, tandy1k_sl_get_device	},
    { "[8086] Toshiba T1200",			"t1200",		MACHINE_TYPE_8086,		CPU_PKG_8086, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC | MACHINE_VIDEO,							 1024,  2048,1024,   63,		machine_xt_t1200_init, t1200_get_device		},
    
#if defined(DEV_BRANCH) && defined(USE_LASERXT)
    { "[8086] VTech Laser XT3",			"lxt3",			MACHINE_TYPE_8086,		CPU_PKG_8086, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									  256,   640, 256,    0,		 machine_xt_lxt3_init, NULL			},
#endif

    /* 286 XT machines */
#if defined(DEV_BRANCH) && defined(USE_HEDAKA)
    { "[Citygate D30 XT] Hedaka HED-919",	"hed919",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PC,									   64,  1024,  64,    0,	       machine_xt_hed919_init, NULL			},
#endif

    /* 286 AT machines */
    { "[ISA] IBM AT",				"ibmat",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 6000000, 8000000, 0, 0, 0, 0,									MACHINE_AT,									  256, 15872, 128,   63,		  machine_at_ibm_init, NULL			},
    { "[ISA] IBM PS/1 model 2011",		"ibmps1es",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 10000000, 10000000, 0, 0, 0, 0,									MACHINE_AT | MACHINE_BUS_PS2 | MACHINE_XTA | MACHINE_VIDEO_FIXED,		  512, 16384, 512,   63,	       machine_ps1_m2011_init, NULL			},
    { "[ISA] IBM PS/2 model 30-286",		"ibmps2_m30_286",	MACHINE_TYPE_286,		CPU_PKG_286 | CPU_PKG_486SLC_IBM, 0, 10000000, 0, 0, 0, 0, 0,							MACHINE_AT | MACHINE_BUS_PS2 | MACHINE_XTA | MACHINE_VIDEO_FIXED,		 1024, 16384,1024,  127,	     machine_ps2_m30_286_init, NULL			},
    { "[ISA] IBM XT Model 286",			"ibmxt286",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 6000000, 6000000, 0, 0, 0, 0,									MACHINE_AT,									  256, 15872, 128,  127,	     machine_at_ibmxt286_init, NULL			},
    { "[ISA] AMI IBM AT",			"ibmatami",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 6000000, 8000000, 0, 0, 0, 0,									MACHINE_AT,									  256, 15872, 128,   63,	     machine_at_ibmatami_init, NULL			},
    { "[ISA] Commodore PC 30 III",		"cmdpc30",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									  640, 16384, 128,  127,		machine_at_cmdpc_init, NULL			},
    { "[ISA] Compaq Portable II",		"portableii",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									  640, 16384, 128,  127,	   machine_at_portableii_init, NULL			},
    { "[ISA] Compaq Portable III",		"portableiii",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_VIDEO,							  640, 16384, 128,  127,	  machine_at_portableiii_init, at_cpqiii_get_device	},
    { "[ISA] MR 286 clone",			"mr286",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_IDE,							  512, 16384, 128,  127,		machine_at_mr286_init, NULL			},
    { "[ISA] NCR PC8/810/710/3390/3392",	"pc8",			MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									  512, 16384, 128,  127,		  machine_at_pc8_init, NULL			},
#if defined(DEV_BRANCH) && defined(USE_OLIVETTI)
    { "[ISA] Olivetti M290",			"m290",			MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									  640, 16384, 128, 127,			 machine_at_m290_init, NULL			},
#endif
    #if defined(DEV_BRANCH) && defined(USE_OPEN_AT)
    { "[ISA] OpenAT",				"openat",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									  256, 15872, 128,   63,	       machine_at_openat_init, NULL			},
#endif
    { "[ISA] Phoenix IBM AT",			"ibmatpx",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 6000000, 8000000, 0, 0, 0, 0,									MACHINE_AT,									  256, 15872, 128,   63,	      machine_at_ibmatpx_init, NULL			},
    { "[ISA] Quadtel IBM AT",			"ibmatquadtel",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 6000000, 8000000, 0, 0, 0, 0,									MACHINE_AT,									  256, 15872, 128,   63,	 machine_at_ibmatquadtel_init, NULL			},
    { "[ISA] Siemens PCD-2L",			"siemens",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									  256, 15872, 128,   63,	      machine_at_siemens_init, NULL			},
    { "[ISA] Toshiba T3100e",			"t3100e",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_IDE | MACHINE_VIDEO_FIXED,					 1024,  5120, 256,   63,	       machine_at_t3100e_init, NULL			},
    { "[GC103] Quadtel 286 clone",		"quadt286",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									  512, 16384, 128,  127,	     machine_at_quadt286_init, NULL			},
    { "[GC103] Trigem 286M",			"tg286m",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_IDE,							  512,  8192, 128,  127,	       machine_at_tg286m_init, NULL			},
    { "[NEAT] AMI 286 clone",			"ami286",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									  512,  8192, 128,  127,	     machine_at_neat_ami_init, NULL			},
    { "[NEAT] NCR 3302",			"3302",			MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_VIDEO,							  512, 16384, 128,  127,		 machine_at_3302_init, NULL			},
    { "[NEAT] Phoenix 286 clone",		"px286",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									  512, 16384, 128,  127,		machine_at_px286_init, NULL			},
    { "[SCAT] Award 286 clone",			"award286",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									  512, 16384, 128,  127,	     machine_at_award286_init, NULL			},
    { "[SCAT] GW-286CT GEAR",			"gw286ct",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_IDE,							  512, 16384, 128,  127,	      machine_at_gw286ct_init, NULL			},
    { "[SCAT] Goldstar GDC-212M",		"gdc212m",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_IDE | MACHINE_BUS_PS2,					  512,  4096, 512,  127,	      machine_at_gdc212m_init, NULL			},
    { "[SCAT] Hyundai Super-286TR",		"super286tr",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									  512, 16384, 128,  127,	   machine_at_super286tr_init, NULL			},
    { "[SCAT] Samsung SPC-4200P",		"spc4200p",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_BUS_PS2,							  512,  2048, 128,  127,	     machine_at_spc4200p_init, NULL			},
    { "[SCAT] Samsung SPC-4216P",		"spc4216p",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_BUS_PS2,							 1024,  5120,1024,  127,	     machine_at_spc4216p_init, NULL			},
    { "[SCAT] Samsung SPC-4620P",		"spc4620p",		MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_BUS_PS2 | MACHINE_VIDEO,					 1024,  5120,1024,  127,	     machine_at_spc4620p_init, NULL			},
    { "[SCAT] Samsung Deskmaster 286",		"deskmaster286",	MACHINE_TYPE_286,		CPU_PKG_286, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									  512, 16384, 128,  127,	machine_at_deskmaster286_init, NULL			},
    
    /* 286 machines that utilize the MCA bus */
    { "[MCA] IBM PS/2 model 50",		"ibmps2_m50",		MACHINE_TYPE_286,		CPU_PKG_286 | CPU_PKG_486SLC_IBM, 0, 10000000, 0, 0, 0, 0, 0,							MACHINE_MCA | MACHINE_BUS_PS2 | MACHINE_VIDEO,					 1024, 10240,1024,   63,	    machine_ps2_model_50_init, NULL			},

    /* 386SX machines */
    { "[ISA] IBM PS/1 model 2121",		"ibmps1_2121",		MACHINE_TYPE_386SX,		CPU_PKG_386SX, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_BUS_PS2 | MACHINE_IDE | MACHINE_VIDEO_FIXED,		 2048,  6144,1024,   63,	       machine_ps1_m2121_init, NULL			},
    { "[ISA] IBM PS/1 m.2121+ISA",		"ibmps1_2121_isa",	MACHINE_TYPE_386SX,		CPU_PKG_386SX, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_BUS_PS2 | MACHINE_IDE | MACHINE_VIDEO,			 2048,  6144,1024,   63,	       machine_ps1_m2121_init, NULL			},
    { "[ISA] NCR PC916SX",			"pc916sx",		MACHINE_TYPE_386SX,		CPU_PKG_386SX, 0, 0, 0, 0, 0, 0, 0, 										MACHINE_AT,									 1024, 16384, 128, 127,		      machine_at_pc916sx_init, NULL			},
#if defined(DEV_BRANCH) && defined(USE_M6117)
    { "[ALi M6117] Acrosser AR-B1375",		"arb1375",		MACHINE_TYPE_386SX,		CPU_PKG_M6117, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_BUS_PS2 | MACHINE_IDE,					 1024, 32768,1024,  127,	      machine_at_arb1375_init, NULL			},
    { "[ALi M6117] Acrosser PJ-A511M",		"pja511m",		MACHINE_TYPE_386SX,		CPU_PKG_M6117, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_BUS_PS2 | MACHINE_IDE,					 1024, 32768,1024,  127,	      machine_at_pja511m_init, NULL			},
#endif
    { "[ALi M1217] Flytech 386",		"flytech386",		MACHINE_TYPE_386SX,		CPU_PKG_386SX, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_IDE | MACHINE_BUS_PS2 | MACHINE_VIDEO,			 1024, 16384, 1024, 127,	   machine_at_flytech386_init, at_flytech386_get_device	},
    { "[HT18] AMA-932J",			"ama932j",		MACHINE_TYPE_386SX,		CPU_PKG_386SX, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_IDE | MACHINE_VIDEO,					  512,  8192,  128, 127,	      machine_at_ama932j_init, at_ama932j_get_device 	},
    { "[Intel 82335] ADI 386SX",		"adi386sx",		MACHINE_TYPE_386SX,		CPU_PKG_386SX, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									  512,  8192,  128, 127,	     machine_at_adi386sx_init, NULL			},
    { "[Intel 82335] Shuttle 386SX",		"shuttle386sx",		MACHINE_TYPE_386SX,		CPU_PKG_386SX, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									  512,  8192,  128, 127,	 machine_at_shuttle386sx_init, NULL			},
    { "[NEAT] Commodore SL386SX-16",		"cmdsl386sx16",		MACHINE_TYPE_386SX,		CPU_PKG_386SX, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_BUS_PS2 | MACHINE_IDE,					 1024,  8192,  512, 127,	 machine_at_cmdsl386sx16_init, NULL			},
    { "[NEAT] DTK 386SX clone",			"dtk386",		MACHINE_TYPE_386SX,		CPU_PKG_386SX, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									  512,  8192,  128, 127,		 machine_at_neat_init, NULL			},
    { "[OPTi 283] Olivetti M300-08",		"m30008",		MACHINE_TYPE_386SX,		CPU_PKG_386SX, 0, 20000000, 20000000, 0, 0, 0, 0,								MACHINE_AT | MACHINE_BUS_PS2 | MACHINE_IDE | MACHINE_VIDEO,			 2048, 16384, 2048, 127,	       machine_at_m30008_init, at_m30008_get_device	},
    { "[OPTi 283] Olivetti M300-15",		"m30015",		MACHINE_TYPE_386SX,		CPU_PKG_386SX, 0, 25000000, 25000000, 0, 0, 0, 0,						 		MACHINE_AT | MACHINE_BUS_PS2 | MACHINE_IDE | MACHINE_VIDEO,			 2048, 16384, 2048, 127,	       machine_at_m30015_init, NULL			},
    { "[OPTi 291] DTK PPM-3333P",		"awardsx",		MACHINE_TYPE_386SX,		CPU_PKG_386SX, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									 1024, 16384, 1024, 127,	      machine_at_awardsx_init, NULL			},
    { "[SCAMP] Commodore SL386SX-25",		"cmdsl386sx25",		MACHINE_TYPE_386SX,		CPU_PKG_386SX, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_BUS_PS2 | MACHINE_IDE | MACHINE_VIDEO,			 1024,  8192,  512, 127,	 machine_at_cmdsl386sx25_init, at_cmdsl386sx25_get_device },
    { "[SCAMP] DataExpert 386SX",		"dataexpert386sx",	MACHINE_TYPE_386SX,		CPU_PKG_386SX, 0, 10000000, 25000000, 0, 0, 0, 0,								MACHINE_AT,									 1024, 16384, 1024, 127,	 machine_at_dataexpert386sx_init, NULL			},
    { "[SCAMP] Samsung SPC-6033P",		"spc6033p",		MACHINE_TYPE_386SX,		CPU_PKG_386SX, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_BUS_PS2 | MACHINE_IDE | MACHINE_VIDEO,			 2048, 12288, 2048, 127,	     machine_at_spc6033p_init, at_spc6033p_get_device	},
    { "[SCAT] KMX-C-02",			"kmxc02",		MACHINE_TYPE_386SX,		CPU_PKG_386SX, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									  512, 16384,  512, 127,	       machine_at_kmxc02_init, NULL			},
    { "[WD76C10] Amstrad MegaPC",		"megapc",		MACHINE_TYPE_386SX,		CPU_PKG_386SX, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_BUS_PS2 | MACHINE_IDE | MACHINE_VIDEO,			 1024, 32768, 1024, 127,	      machine_at_wd76c10_init, NULL			},

    /* 386SX machines which utilize the MCA bus */
    { "[MCA] IBM PS/2 model 55SX",		"ibmps2_m55sx",		MACHINE_TYPE_386SX,		CPU_PKG_386SX, 0, 0, 0, 0, 0, 0, 0,										MACHINE_MCA | MACHINE_BUS_PS2 | MACHINE_VIDEO,					 1024,  8192, 1024,  63,	  machine_ps2_model_55sx_init, NULL			},

    /* 386DX machines */
    { "[ACC 2168] AMI 386DX clone",		"acc386",		MACHINE_TYPE_386DX,		CPU_PKG_386DX, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									 1024, 16384, 1024, 127,	       machine_at_acc386_init, NULL			},
    { "[C&T 386] ECS 386/32",			"ecs386",		MACHINE_TYPE_386DX,		CPU_PKG_386DX, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									 1024, 16384, 1024, 127,	       machine_at_ecs386_init, NULL			},
    { "[C&T 386] Samsung SPC-6000A",		"spc6000a",		MACHINE_TYPE_386DX,		CPU_PKG_386DX, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_IDE,							 1024, 32768, 1024, 127,	     machine_at_spc6000a_init, NULL			},
    { "[ISA] Compaq Portable III (386)",	"portableiii386",       MACHINE_TYPE_386DX,		CPU_PKG_386DX, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_IDE | MACHINE_VIDEO,					 1024, 14336, 1024, 127,       machine_at_portableiii386_init, at_cpqiii_get_device	},
    { "[ISA] Micronics 386 clone",		"micronics386",		MACHINE_TYPE_386DX,		CPU_PKG_386DX, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									  512,  8192,  128, 127,	 machine_at_micronics386_init, NULL			},
    { "[SiS 310] ASUS ISA-386C",		"asus386",		MACHINE_TYPE_386DX,		CPU_PKG_386DX, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									  512, 16384,  128, 127,	      machine_at_asus386_init, NULL			},

    /* 386DX machines which utilize the MCA bus */
    { "[MCA] IBM PS/2 model 70 (type 3)",	"ibmps2_m70_type3",	MACHINE_TYPE_386DX,		CPU_PKG_386DX | CPU_PKG_486BL, 0, 0, 0, 0, 0, 0, 0,								MACHINE_MCA | MACHINE_BUS_PS2 | MACHINE_VIDEO,					 2048, 16384, 2048,  63,      machine_ps2_model_70_type3_init, NULL			},
    { "[MCA] IBM PS/2 model 80",		"ibmps2_m80",		MACHINE_TYPE_386DX,		CPU_PKG_386DX | CPU_PKG_486BL, 0, 0, 0, 0, 0, 0, 0,								MACHINE_MCA | MACHINE_BUS_PS2 | MACHINE_VIDEO,					 1024, 12288, 1024,  63,	    machine_ps2_model_80_init, NULL			},
    { "[MCA] IBM PS/2 model 80 (type 3)",	"ibmps2_m80_type3",		MACHINE_TYPE_386DX,		CPU_PKG_386DX | CPU_PKG_486BL, 0, 0, 0, 0, 0, 0, 0,								MACHINE_MCA | MACHINE_BUS_PS2 | MACHINE_VIDEO,					 2048, 12288, 2048,  63,	    machine_ps2_model_80_axx_init, NULL			},

    /* 386DX/486 machines */
    { "[OPTi 495] Award 486 clone",		"award486",		MACHINE_TYPE_386DX_486,		CPU_PKG_386DX | CPU_PKG_SOCKET1, 0, 0, 0, 0, 0, 0, 0,								MACHINE_VLB | MACHINE_IDE,							 1024, 32768, 1024, 127,	      machine_at_opti495_init, NULL			},
    { "[OPTi 495] Dataexpert SX495 (486)",	"ami486",		MACHINE_TYPE_386DX_486,		CPU_PKG_386DX | CPU_PKG_SOCKET1, 0, 0, 0, 0, 0, 0, 0,								MACHINE_VLB | MACHINE_IDE,							 1024, 32768, 1024, 127,	  machine_at_opti495_ami_init, NULL			},
    { "[OPTi 495] MR 486 clone",		"mr486",		MACHINE_TYPE_386DX_486,		CPU_PKG_386DX | CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,								MACHINE_VLB | MACHINE_IDE,							 1024, 32768, 1024, 127,	   machine_at_opti495_mr_init, NULL			},

    /* 486 machines - Socket 1 */
    { "[ALi M1429] Olystar LIL1429",		"ali1429",		MACHINE_TYPE_486,		CPU_PKG_SOCKET1, 0, 0, 0, 0, 0, 0, 0,										MACHINE_VLB | MACHINE_IDE,							 1024, 32768, 1024, 127,	      machine_at_ali1429_init, NULL			},
    { "[CS4031] AMI 486 CS4031",		"cs4031",		MACHINE_TYPE_486,		CPU_PKG_SOCKET1, 0, 0, 0, 0, 0, 0, 0,										MACHINE_VLB,									 1024, 65536, 1024, 127,	       machine_at_cs4031_init, NULL			},
    { "[ETEQ ET6000] Olivetti PCS-46C",		"pcs46c",		MACHINE_TYPE_486,		CPU_PKG_SOCKET1, 0, 0, 0, 0, 0, 0, 0,										MACHINE_VLB | MACHINE_IDE | MACHINE_VIDEO,							 4096, 32768, 4096, 127,	      machine_at_pcs46c_init, at_pcs46c_get_device			},
    { "[OPTi 895] Mylex MVI486",		"mvi486",		MACHINE_TYPE_486,		CPU_PKG_SOCKET1, 0, 0, 0, 0, 0, 0, 0,										MACHINE_VLB | MACHINE_IDE_DUAL,							 1024, 65536, 1024, 127,	       machine_at_mvi486_init, NULL			},
    { "[VIA VT82C495] FIC 486-VC-HD",		"486vchd",		MACHINE_TYPE_486,		CPU_PKG_SOCKET1, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT,									 1024, 64512, 1024, 127,	      machine_at_486vchd_init, NULL			},
    { "[VLSI 82C480] HP Vectra 486VL",		"vect486vl",		MACHINE_TYPE_486,		CPU_PKG_SOCKET1, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_BUS_PS2 | MACHINE_IDE | MACHINE_VIDEO,			 2048, 32768, 2048, 127,	    machine_at_vect486vl_init, at_vect486vl_get_device	},
    { "[VLSI 82C481] Siemens Nixdorf D824",		"d824",		MACHINE_TYPE_486,		CPU_PKG_SOCKET1, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_BUS_PS2 | MACHINE_IDE | MACHINE_VIDEO,			 2048, 32768, 2048, 127,	    machine_at_d824_init, at_d824_get_device	},

    /* 486 machines - Socket 3 */
    /* 486 machines with just the ISA slot */
    { "[ACC 2168] Packard Bell PB410A",		"pb410a",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_BUS_PS2 | MACHINE_IDE | MACHINE_VIDEO,			 4096, 36864, 1024, 127,	       machine_at_pb410a_init, NULL			},
    { "[ALi M1429G] Acer A1G",			"acera1g",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL | MACHINE_VIDEO,		 4096, 36864, 1024, 127,	      machine_at_acera1g_init, at_acera1g_get_device	},
    { "[ALi M1429G] Kaimei 486",		"win486",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_VLB | MACHINE_IDE,							 1024, 32768, 1024, 127,	  machine_at_winbios1429_init, NULL			},
    { "[OPTi 895] Jetway J-403TG",		"403tg",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_VLB,									 1024, 65536, 1024, 127,		machine_at_403tg_init, NULL			},
    { "[SiS 401] AMI 486 Clone",		"sis401",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_IDE,							 1024, 65536, 1024, 127,	       machine_at_sis401_init, NULL			},
    { "[SiS 401] ASUS ISA-486",			"isa486",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_IDE,							 1024, 65536, 1024, 127,	       machine_at_isa486_init, NULL			},
    { "[SiS 460] ABIT AV4",			"av4",			MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_VLB | MACHINE_IDE,							 1024, 65536, 1024, 127,		  machine_at_av4_init, NULL			},
    { "[SiS 461] IBM PS/ValuePoint 433DX/Si",	"valuepoint433",	MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_AT | MACHINE_BUS_PS2 | MACHINE_IDE | MACHINE_VIDEO,			 1024, 65536, 1024, 127,	machine_at_valuepoint433_init, NULL			},
    { "[SiS 471] AMI 486 Clone",		"ami471",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_VLB | MACHINE_IDE,							 1024, 65536, 1024, 127,	       machine_at_ami471_init, NULL			},
    { "[SiS 471] AMI WinBIOS 486 clone",	"win471",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_VLB | MACHINE_IDE,							 1024, 65536, 1024, 127,	       machine_at_win471_init, NULL			},
    { "[SiS 471] AOpen Vi15G",			"vi15g",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_VLB | MACHINE_IDE,							 1024, 65536, 1024, 127,		machine_at_vi15g_init, NULL			},
    { "[SiS 471] ASUS VL/I-486SV2G (GX4)",	"vli486sv2g",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_VLB | MACHINE_IDE_DUAL,							 1024, 65536, 1024, 127,	   machine_at_vli486sv2g_init, NULL			},
    { "[SiS 471] DTK PKM-0038S E-2",		"dtk486",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_VLB | MACHINE_IDE,							 1024, 65536, 1024, 127,	       machine_at_dtk486_init, NULL			},
    { "[SiS 471] Phoenix SiS 471",		"px471",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_VLB | MACHINE_IDE,							 1024,131072, 1024, 127,		machine_at_px471_init, NULL			},

    /* 486 machines which utilize the PCI bus */
    { "[ALi M1489] ABIT AB-PB4",		"abpb4",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PCI | MACHINE_IDE_DUAL,							 1024,  65536, 1024, 255,		machine_at_abpb4_init, NULL			},
    { "[ALi M1489] AMI WinBIOS 486 PCI",	"win486pci",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PCI | MACHINE_IDE_DUAL,							 1024,  65536, 1024, 255,	    machine_at_win486pci_init, NULL			},
    { "[OPTi 802G] IBM PC 330 (type 6573)",	"pc330_6573",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3_PC330, 0, 25000000, 33333333, 0, 0, 2.0, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE,					 1024, 65536, 1024, 127,	   machine_at_pc330_6573_init, NULL			},
    { "[i420EX] ASUS PVI-486AP4",		"486ap4",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PCIV | MACHINE_IDE_DUAL,						 1024, 131072, 1024, 127,	       machine_at_486ap4_init, NULL			},
    { "[i420ZX] ASUS PCI/I-486SP3G",		"486sp3g",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PCI | MACHINE_IDE_DUAL | MACHINE_SCSI,					 1024, 131072, 1024, 127,	      machine_at_486sp3g_init, NULL			},
    { "[i420TX] ASUS PCI/I-486SP3",		"486sp3",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PCI | MACHINE_IDE_DUAL | MACHINE_SCSI,					 1024, 131072, 1024, 127,	       machine_at_486sp3_init, NULL			},
    { "[i420TX] Intel Classic/PCI",		"alfredo",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		  		 2048, 131072, 2048, 127,	      machine_at_alfredo_init, NULL			},
    { "[SiS 496] ASUS PVI-486SP3C",		"486sp3c",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PCIV | MACHINE_IDE_DUAL,						 1024, 261120, 1024, 255,	      machine_at_486sp3c_init, NULL			},
    { "[SiS 496] Lucky Star LS-486E",		"ls486e",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PCI | MACHINE_IDE_DUAL,							 1024, 131072, 1024, 255,	       machine_at_ls486e_init, NULL			},
    { "[SiS 496] Micronics M4Li",		"m4li",			MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 1024, 131072, 1024, 127,		 machine_at_m4li_init, NULL			},
    { "[SiS 496] Rise Computer R418",		"r418",			MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PCI | MACHINE_IDE_DUAL,							 1024, 261120, 1024, 255,		 machine_at_r418_init, NULL			},
    { "[SiS 496] Soyo 4SA2",			"4sa2",			MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, CPU_BLOCK(CPU_i486SX, CPU_i486DX, CPU_Am486SX, CPU_Am486DX), 0, 0, 0, 0, 0, 0,			MACHINE_PCI | MACHINE_IDE_DUAL,							 1024, 261120, 1024, 255,		 machine_at_4sa2_init, NULL			},
    { "[SiS 496] Zida Tomato 4DP",		"4dps",			MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PCI | MACHINE_IDE_DUAL,							 1024, 261120, 1024, 255,		 machine_at_4dps_init, NULL			},
    { "[UMC 888x] PC Chips M919",		"m919",			MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PCI | MACHINE_VLB | MACHINE_IDE_DUAL,					 1024, 131072, 1024, 255,		 machine_at_m919_init, NULL			},
    { "[VIA VT82C496G] FIC VIP-IO2",		"486vipio2",		MACHINE_TYPE_486_S3,		CPU_PKG_SOCKET3, 0, 0, 0, 0, 0, 0, 0,										MACHINE_PCIV | MACHINE_IDE_DUAL,						 1024, 131072, 1024, 255,	    machine_at_486vipio2_init, NULL			},

    /* 486 machines - Miscellaneous */
    /* 486 machines with just the ISA slot */
    { "[OPTi 283] RYC Leopard LX",		"rycleopardlx",		MACHINE_TYPE_486_MISC,		CPU_PKG_486SLC_IBM, 0, 0, 0, 0, 0, 0, 0,									MACHINE_AT | MACHINE_IDE,							 1024, 16384, 1024, 127,	 machine_at_rycleopardlx_init, NULL			},

    /* 486 machines which utilize the PCI bus */
    { "[STPC Client] ITOX STAR",		"itoxstar",		MACHINE_TYPE_486_MISC,		CPU_PKG_STPC, 0, 66666667, 75000000, 0, 0, 1.0, 1.0,								MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 131072, 8192, 255,	     machine_at_itoxstar_init, NULL			},
    { "[STPC Consumer-II] Acrosser AR-B1479",	"arb1479",		MACHINE_TYPE_486_MISC,		CPU_PKG_STPC, 0, 66666667, 66666667, 0, 0, 2.0, 2.0,								MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				32768, 163840, 8192, 255,	      machine_at_arb1479_init, NULL			},
    { "[STPC Elite] Advantech PCM-9340",	"pcm9340",		MACHINE_TYPE_486_MISC,		CPU_PKG_STPC, 0, 66666667, 66666667, 0, 0, 2.0, 2.0,								MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				32768,  98304, 8192, 255,	      machine_at_pcm9340_init, NULL			},
    { "[STPC Atlas] AAEON PCM-5330",		"pcm5330",		MACHINE_TYPE_486_MISC,		CPU_PKG_STPC, 0, 66666667, 66666667, 0, 0, 2.0, 2.0,								MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				32768, 131072,32768, 255,	      machine_at_pcm5330_init, NULL			},

    /* Socket 4 machines */
    /* 430LX */
    { "[i430LX] ASUS P/I-P5MP3",		"p5mp3",		MACHINE_TYPE_SOCKET4,		CPU_PKG_SOCKET4, 0, 60000000, 66666667, 5000, 5000, MACHINE_MULTIPLIER_FIXED,					MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE,		 			 2048, 196608, 2048, 127,		machine_at_p5mp3_init, NULL			},
    { "[i430LX] Dell Dimension XPS P60",	"dellxp60",		MACHINE_TYPE_SOCKET4,		CPU_PKG_SOCKET4, 0, 60000000, 66666667, 5000, 5000, MACHINE_MULTIPLIER_FIXED,					MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE,					 2048, 131072, 2048, 127,	     machine_at_dellxp60_init, NULL			},
    { "[i430LX] Dell OptiPlex 560/L",		"opti560l",		MACHINE_TYPE_SOCKET4,		CPU_PKG_SOCKET4, 0, 60000000, 66666667, 5000, 5000, MACHINE_MULTIPLIER_FIXED,					MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 2048, 131072, 2048, 127,	     machine_at_opti560l_init, NULL			},
    { "[i430LX] AMBRA DP60 PCI",		"ambradp60",		MACHINE_TYPE_SOCKET4,		CPU_PKG_SOCKET4, 0, 60000000, 66666667, 5000, 5000, MACHINE_MULTIPLIER_FIXED,					MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 2048, 131072, 2048, 127,	    machine_at_ambradp60_init, NULL			},
    { "[i430LX] IBM PS/ValuePoint P60",		"valuepointp60",	MACHINE_TYPE_SOCKET4,		CPU_PKG_SOCKET4, 0, 60000000, 66666667, 5000, 5000, MACHINE_MULTIPLIER_FIXED,					MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 2048, 131072, 2048, 127,	machine_at_valuepointp60_init, NULL			},
    { "[i430LX] Intel Premiere/PCI",		"revenge",		MACHINE_TYPE_SOCKET4,		CPU_PKG_SOCKET4, 0, 60000000, 66666667, 5000, 5000, MACHINE_MULTIPLIER_FIXED,					MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 2048, 131072, 2048, 127,	       machine_at_batman_init, NULL			},
    { "[i430LX] Micro Star 586MC1",		"586mc1",		MACHINE_TYPE_SOCKET4,		CPU_PKG_SOCKET4, 0, 60000000, 66666667, 5000, 5000, MACHINE_MULTIPLIER_FIXED,					MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,			 	 2048, 131072, 2048, 127,	       machine_at_586mc1_init, NULL			},
    { "[i430LX] Packard Bell PB520R",		"pb520r",		MACHINE_TYPE_SOCKET4,		CPU_PKG_SOCKET4, 0, 60000000, 66666667, 5000, 5000, MACHINE_MULTIPLIER_FIXED,					MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL | MACHINE_VIDEO,		 8192, 139264, 2048, 127,	       machine_at_pb520r_init, at_pb520r_get_device	},

    /* OPTi 596/597 */
    { "[OPTi 597] AMI Excalibur VLB",		"excalibur",		MACHINE_TYPE_SOCKET4,		CPU_PKG_SOCKET4, 0, 60000000, 66666667, 5000, 5000, MACHINE_MULTIPLIER_FIXED,					MACHINE_VLB | MACHINE_IDE,						 	 2048,  65536, 2048, 127,	    machine_at_excalibur_init, NULL			},

    /* SiS 85C50x */
    { "[SiS 85C50x] ASUS PCI/I-P5SP4",		"p5sp4",		MACHINE_TYPE_SOCKET4,		CPU_PKG_SOCKET4, 0, 60000000, 66666667, 5000, 5000, MACHINE_MULTIPLIER_FIXED,					MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 131072, 8192, 127,		machine_at_p5sp4_init, NULL			},

    /* Socket 5 machines */
    /* 430NX */
    { "[i430NX] Intel Premiere/PCI II",		"plato",		MACHINE_TYPE_SOCKET5,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3520, 3520, 1.5, 1.5,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 2048, 131072, 2048, 127,		machine_at_plato_init, NULL			},
    { "[i430NX] AMBRA DP90 PCI",		"ambradp90",		MACHINE_TYPE_SOCKET5,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3380, 3520, 1.5, 1.5,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 2048, 131072, 2048, 127,	    machine_at_ambradp90_init, NULL			},
    { "[i430NX] Gigabyte GA-586IP",		"430nx",		MACHINE_TYPE_SOCKET5,		CPU_PKG_SOCKET5_7, 0, 60000000, 66666667, 3520, 3520, 1.5, 1.5,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 2048, 131072, 2048, 127,		machine_at_430nx_init, NULL			},

    /* 430FX */
    { "[i430FX] Acer V30",			"acerv30",		MACHINE_TYPE_SOCKET5,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3380, 3520, 1.5, 2.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 131072, 8192, 127,	      machine_at_acerv30_init, NULL			},
    { "[i430FX] AMI Apollo",			"apollo",		MACHINE_TYPE_SOCKET5,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3380, 3520, 1.5, 2.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 131072, 8192, 127,	       machine_at_apollo_init, NULL			},
    { "[i430FX] HP Vectra VL 5 Series 4",	"vectra54",		MACHINE_TYPE_SOCKET5,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3380, 3520, 1.5, 2.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL | MACHINE_VIDEO,		 8192, 131072, 8192, 511,	     machine_at_vectra54_init, at_vectra54_get_device	},
    { "[i430FX] Intel Advanced/ZP",		"zappa",		MACHINE_TYPE_SOCKET5,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3380, 3520, 1.5, 2.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 131072, 8192, 127,		machine_at_zappa_init, NULL			},
    { "[i430FX] NEC PowerMate V",  		"powermatev",		MACHINE_TYPE_SOCKET5,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3380, 3520, 1.5, 2.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 131072, 8192, 127,	   machine_at_powermatev_init, NULL			},
    { "[i430FX] PC Partner MB500N",		"mb500n",		MACHINE_TYPE_SOCKET5,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3380, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_IDE_DUAL,							 8192, 131072, 8192, 127,	       machine_at_mb500n_init, NULL			},

    /* OPTi 596/597 */
    { "[OPTi 597] TMC PAT54PV",			"pat54pv",		MACHINE_TYPE_SOCKET5,		CPU_PKG_SOCKET5_7, CPU_BLOCK(CPU_K5, CPU_5K86), 50000000, 66666667, 3520, 3520, 1.5, 1.5,			MACHINE_VLB,									 2048,  65536, 2048, 127,	      machine_at_pat54pv_init, NULL			},
    
    /* OPTi 596/597/822 */
    { "[OPTi 597] Shuttle HOT-543",		"hot543",		MACHINE_TYPE_SOCKET5,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3520, 3520, 1.5, 1.5,							MACHINE_PCI | MACHINE_VLB,							 8192, 131072, 8192, 127,	       machine_at_hot543_init, NULL			},
    { "[OPTi 597] Supermicro P54VL-PCI",	"p54vl",		MACHINE_TYPE_SOCKET5,		CPU_PKG_SOCKET5_7, 0, 60000000, 66666667, 3520, 3520, 1.5, 1.5,							MACHINE_PCI | MACHINE_VLB,							 8192, 131072, 8192, 127,		machine_at_p54vl_init, NULL			},

    /* SiS 85C50x */
    { "[SiS 85C50x] ASUS PCI/I-P54SP4",		"p54sp4",		MACHINE_TYPE_SOCKET5,		CPU_PKG_SOCKET5_7, CPU_BLOCK(CPU_K5, CPU_5K86), 40000000, 66666667, 3380, 3520, 1.5, 1.5,			MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 131072, 8192, 127,	       machine_at_p54sp4_init, NULL			},
    { "[SiS 85C50x] BCM SQ-588",		"sq588",		MACHINE_TYPE_SOCKET5,		CPU_PKG_SOCKET5_7, CPU_BLOCK(CPU_PENTIUMMMX), 50000000, 66666667, 3520, 3520, 1.5, 1.5,				MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 131072, 8192, 127,		machine_at_sq588_init, NULL			},

    /* Socket 7 (Single Voltage) machines */
    /* 430FX */
    { "[i430FX] ASUS P/I-P54TP4XE",		"p54tp4xe",		MACHINE_TYPE_SOCKET7_3V,	CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3380, 3600, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 131072, 8192, 127,	     machine_at_p54tp4xe_init, NULL			},
    { "[i430FX] ASUS P/I-P54TP4XE (MR BIOS)",	"mr586",		MACHINE_TYPE_SOCKET7_3V,	CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3380, 3600, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 131072, 8192, 127,		machine_at_mr586_init, NULL			},
    { "[i430FX] Gateway 2000 Thor",		"gw2katx",		MACHINE_TYPE_SOCKET7_3V,	CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3380, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL | MACHINE_VIDEO,		 8192, 131072, 8192, 127,	      machine_at_gw2katx_init, NULL			},
    { "[i430FX] Intel Advanced/ATX",		"thor",			MACHINE_TYPE_SOCKET7_3V,	CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3380, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL | MACHINE_VIDEO,		 8192, 131072, 8192, 127,		 machine_at_thor_init, NULL			},
    { "[i430FX] Intel Advanced/ATX (MR BIOS)",	"mrthor",		MACHINE_TYPE_SOCKET7_3V,	CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3380, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL | MACHINE_VIDEO,		 8192, 131072, 8192, 127,	       machine_at_mrthor_init, NULL			},
    { "[i430FX] Intel Advanced/EV",		"endeavor",		MACHINE_TYPE_SOCKET7_3V,	CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3380, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL | MACHINE_VIDEO,		 8192, 131072, 8192, 127,	     machine_at_endeavor_init, at_endeavor_get_device	},
    { "[i430FX] Packard Bell PB640",		"pb640",		MACHINE_TYPE_SOCKET7_3V,	CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3380, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL | MACHINE_VIDEO, 		 8192, 131072, 8192, 127,		machine_at_pb640_init, at_pb640_get_device	},
    { "[i430FX] QDI Chariot",			"chariot",		MACHINE_TYPE_SOCKET7_3V,	CPU_PKG_SOCKET5_7, CPU_BLOCK(CPU_WINCHIP, CPU_WINCHIP2, CPU_Cx6x86, CPU_Cx6x86L, CPU_Cx6x86MX), 50000000, 66666667, 3380, 3520, 1.5, 3.0, MACHINE_PCI | MACHINE_IDE_DUAL,			 8192, 131072, 8192, 127,	      machine_at_chariot_init, NULL			},

    /* 430HX */
    { "[i430HX] Acer M3A",			"acerm3a",		MACHINE_TYPE_SOCKET7_3V,	CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3300, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 196608, 8192, 127,	      machine_at_acerm3a_init, NULL			},
    { "[i430HX] AOpen AP53",			"ap53",			MACHINE_TYPE_SOCKET7_3V,	CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3450, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192, 524288, 8192, 127,		 machine_at_ap53_init, NULL			},
    { "[i430HX] Biostar MB-8500TUC",		"8500tuc",		MACHINE_TYPE_SOCKET7_3V,	CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3380, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 524288, 8192, 127,	      machine_at_8500tuc_init, NULL			},
    { "[i430HX] SuperMicro Super P55T2S",	"p55t2s",		MACHINE_TYPE_SOCKET7_3V,	CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3300, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 786432, 8192, 127,	       machine_at_p55t2s_init, NULL			},

    /* 430VX */
    { "[i430VX] Gateway 2000 Tigereye",		"gw2kte",		MACHINE_TYPE_SOCKET7_3V,	CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3380, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192, 131072, 8192, 127,	       machine_at_gw2kte_init, NULL			},

    /* SiS 5511 */
    { "[SiS 5511] AOpen AP5S",		"ap5s",		MACHINE_TYPE_SOCKET7_3V,	CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 3380, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192, 524288, 8192, 127,	       machine_at_ap5s_init, NULL			},

    /* Socket 7 (Dual Voltage) machines */
    /* 430HX */
    { "[i430HX] Acer V35N",			"acerv35n",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, CPU_BLOCK(CPU_Cx6x86MX), 50000000, 66666667, 2800, 3520, 1.5, 3.0,				MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 196608, 8192, 127,	     machine_at_acerv35n_init, NULL			},
    { "[i430HX] ASUS P/I-P55T2P4",		"p55t2p4",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 83333333, 2500, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 262144, 8192, 127,	      machine_at_p55t2p4_init, NULL			},
    { "[i430HX] Micronics M7S-Hi",		"m7shi",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 2800, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192, 131072, 8192, 511,		machine_at_m7shi_init, NULL			},
    { "[i430HX] Intel TC430HX",			"tc430hx",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 2800, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		  		 8192, 131072, 8192, 255,	      machine_at_tc430hx_init, NULL			},
    { "[i430HX] Toshiba Equium 5200D",		"equium5200",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 2800, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		  		 8192, 196608, 8192, 127,	   machine_at_equium5200_init, NULL			},
    { "[i430HX] Sony Vaio PCV-240",		"pcv240",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 2800, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		  		 8192, 196608, 8192, 127,	       machine_at_pcv240_init, NULL			},
    { "[i430HX] ASUS P/I-P65UP5 (C-P55T2D)",	"p65up5_cp55t2d",	MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 2500, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		  		 8192, 524288, 8192, 127,      machine_at_p65up5_cp55t2d_init, NULL			},

    /* 430VX */
    { "[i430VX] ASUS P/I-P55TVP4",		"p55tvp4",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 2500, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192, 131072, 8192, 127,	      machine_at_p55tvp4_init, NULL			},
    { "[i430VX] Azza 5IVG",			"5ivg",			MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 2500, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		  		 8192, 131072, 8192, 127,		 machine_at_5ivg_init, NULL			},
    { "[i430VX] Biostar MB-8500TVX-A",		"8500tvxa",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 2600, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192, 131072, 8192, 127,	     machine_at_8500tvxa_init, NULL			},
    { "[i430VX] Compaq Presario 2240",		"presario2240",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 2800, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL | MACHINE_VIDEO,		 8192, 131072, 8192, 127,	 machine_at_presario2240_init, NULL			},
    { "[i430VX] Compaq Presario 4500",		"presario4500",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 2800, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL | MACHINE_VIDEO,		 8192, 131072, 8192, 127,	 machine_at_presario4500_init, NULL			},
    { "[i430VX] Epox P55-VA",			"p55va",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 2500, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		  		 8192, 131072, 8192, 127,		machine_at_p55va_init, NULL			},
    { "[i430VX] HP Brio 80xx",			"brio80xx",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 66666667, 66666667, 2200, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192, 131072, 8192, 127,	     machine_at_brio80xx_init, NULL			},
    { "[i430VX] Packard Bell PB680",		"pb680",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 2800, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192, 131072, 8192, 127,		machine_at_pb680_init, NULL			},
    { "[i430VX] PC Partner MB520N",		"mb520n",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 2600, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192, 131072, 8192, 127,	       machine_at_mb520n_init, NULL			},
    { "[i430VX] Shuttle HOT-557",		"430vx",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 2500, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL | MACHINE_GAMEPORT,		 8192, 131072, 8192, 127,	       machine_at_i430vx_init, NULL			},

    /* 430TX */
    { "[i430TX] ADLink NuPRO-592",		"nupro592",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 66666667, 66666667, 1900, 2800, 1.5, 5.5,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 262144, 8192, 255,	     machine_at_nupro592_init, NULL			},
    { "[i430TX] ASUS TX97",			"tx97",			MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 75000000, 2500, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 262144, 8192, 255,		 machine_at_tx97_init, NULL			},
#if defined(DEV_BRANCH) && defined(NO_SIO)
    { "[i430TX] Intel AN430TX",			"an430tx",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 60000000, 66666667, 2800, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 262144, 8192, 255,	      machine_at_an430tx_init, NULL			},
#endif
    { "[i430TX] Intel YM430TX",			"ym430tx",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 60000000, 66666667, 2800, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 262144, 8192, 255,	      machine_at_ym430tx_init, NULL			},
    { "[i430TX] PC Partner MB540N",		"mb540n",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 60000000, 66666667, 2700, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 262144, 8192, 255,	       machine_at_mb540n_init, NULL			},
    { "[i430TX] SuperMicro Super P5MMS98",	"p5mms98",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 2100, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 262144, 8192, 255,	      machine_at_p5mms98_init, NULL			},

    /* Apollo VPX */
    { "[VIA VPX] FIC VA-502",			"ficva502",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 75000000, 2800, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192, 524288, 8192, 127,	     machine_at_ficva502_init, NULL			},

    /* Apollo VP3 */
    { "[VIA VP3] FIC PA-2012",			"ficpa2012",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 55000000, 75000000, 2100, 3520, 1.5, 5.5,							MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192,1048576, 8192, 127,	    machine_at_ficpa2012_init, NULL			},

    /* SiS 5571 */
    { "[SiS 5571] Rise R534F",			"r534f",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 2500, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		  		 8192, 393216, 8192, 127,		machine_at_r534f_init, NULL			},
    { "[SiS 5571] MSI MS-5146",			"ms5146",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 66666667, 2500, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		  		 8192, 262144, 8192, 127,	       machine_at_ms5146_init, NULL			},

    /* SiS 5598 */
    { "[SiS 5598] ASUS SP97-XV",		"sp97xv",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 60000000, 66666667, 2100, 3200, 1.5, 2.5,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 262144, 8192, 255,	     machine_at_sp97xv_init, NULL			},
    { "[SiS 5598] PC Chips M571",		"m571",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 75000000, 2500, 3500, 1.5, 3.5,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 262144, 8192, 255,	     machine_at_m571_init, NULL			},

    /* ALi ALADDiN IV */
#if defined(DEV_BRANCH) && defined(USE_M154X)
    { "[ALi ALADDiN IV] PC Chips M560",		"m560",			MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 50000000, 83333333, 2500, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 262144, 8192, 255,		 machine_at_m560_init, NULL			},
    { "[ALi ALADDiN IV] MSI MS-5164",		"ms5164",		MACHINE_TYPE_SOCKET7,		CPU_PKG_SOCKET5_7, 0, 60000000, 66666667, 2100, 3520, 1.5, 3.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 262144, 8192, 255,	       machine_at_ms5164_init, NULL			},
#endif

    /* Super Socket 7 machines */
    /* Apollo MVP3 */
    { "[VIA MVP3] AOpen AX59 Pro",		"ax59pro",		MACHINE_TYPE_SOCKETS7,		CPU_PKG_SOCKET5_7, 0, 66666667, 124242424, 1300, 3520, 1.5, 5.5,						MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192,1048576, 8192, 255,	      machine_at_ax59pro_init, NULL			},
    { "[VIA MVP3] FIC VA-503+",			"ficva503p",		MACHINE_TYPE_SOCKETS7,		CPU_PKG_SOCKET5_7, 0, 66666667, 124242424, 2000, 3200, 1.5, 5.5,						MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192,1048576, 8192, 255,		 machine_at_mvp3_init, NULL			},
    { "[VIA MVP3] FIC VA-503A",			"ficva503a",		MACHINE_TYPE_SOCKETS7,		CPU_PKG_SOCKET5_7, 0, 66666667, 124242424, 1800, 3100, 1.5, 5.5,						MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 786432, 8192, 255,	    machine_at_ficva503a_init, NULL			},

    /* Socket 8 machines */
    /* 450KX */
#if defined(DEV_BRANCH) && defined(USE_I450KX)
    { "[i450KX] ASUS P/I-P6RP4",		"p6rp4",		MACHINE_TYPE_SOCKET8,		CPU_PKG_SOCKET8, 0, 60000000, 66666667, 2100, 3500, 1.5, 8.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 524288, 8192, 127,	        machine_at_p6rp4_init, NULL			},
#endif

    /* 440FX */
    { "[i440FX] Acer V60N",			"v60n",			MACHINE_TYPE_SOCKET8,		CPU_PKG_SOCKET8, 0, 60000000, 66666667, 2500, 3500, 1.5, 8.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 524288, 8192, 127,		 machine_at_v60n_init, NULL			},
    { "[i440FX] ASUS P/I-P65UP5 (C-P6ND)",	"p65up5_cp6nd",		MACHINE_TYPE_SOCKET8,		CPU_PKG_SOCKET8, 0, 60000000, 66666667, 2100, 3500, 1.5, 8.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192,1048576, 8192, 127,	 machine_at_p65up5_cp6nd_init, NULL			},
    { "[i440FX] Biostar MB-8600TTC",		"8600ttc",		MACHINE_TYPE_SOCKET8,		CPU_PKG_SOCKET8, 0, 50000000, 66666667, 2900, 3300, 2.0, 5.5,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192,1048576, 8192, 127,	      machine_at_8500ttc_init, NULL			},
    { "[i440FX] Gigabyte GA-686NX",		"686nx",		MACHINE_TYPE_SOCKET8,		CPU_PKG_SOCKET8, 0, 60000000, 66666667, 2100, 3500, 2.0, 5.5,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 524288, 8192, 127,		machine_at_686nx_init, NULL			},
    { "[i440FX] Intel AP440FX",			"ap440fx",		MACHINE_TYPE_SOCKET8,		CPU_PKG_SOCKET8, 0, 60000000, 66666667, 2100, 3500, 2.0, 3.5,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 131072, 8192, 127,	      machine_at_ap440fx_init, NULL			},
    { "[i440FX] Intel VS440FX",			"vs440fx",		MACHINE_TYPE_SOCKET8,		CPU_PKG_SOCKET8, 0, 60000000, 66666667, 2100, 3500, 2.0, 3.5,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 524288, 8192, 127,	      machine_at_vs440fx_init, NULL			},
    { "[i440FX] Micronics M6Mi",		"m6mi",			MACHINE_TYPE_SOCKET8,		CPU_PKG_SOCKET8, 0, 60000000, 66666667, 2900, 3300, 1.5, 8.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 786432, 8192, 127,		 machine_at_m6mi_init, NULL			},
    { "[i440FX] PC Partner MB600N",		"mb600n",		MACHINE_TYPE_SOCKET8,		CPU_PKG_SOCKET8, 0, 60000000, 66666667, 2100, 3500, 1.5, 8.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 524288, 8192, 127,	       machine_at_mb600n_init, NULL			},

    /* Slot 1 machines */
    /* 440FX */
    { "[i440FX] ASUS P/I-P65UP5 (C-PKND)",	"p65up5_cpknd",		MACHINE_TYPE_SLOT1,		CPU_PKG_SLOT1, 0, 50000000, 66666667, 1800, 3500, 1.5, 8.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192,1048576, 8192, 127,	 machine_at_p65up5_cpknd_init, NULL			},
    { "[i440FX] ASUS KN97",			"kn97",			MACHINE_TYPE_SLOT1,		CPU_PKG_SLOT1, 0, 60000000, 83333333, 1800, 3500, 1.5, 8.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 786432, 8192, 127,		 machine_at_kn97_init, NULL			},

    /* 440LX */
    { "[i440LX] ABIT LX6",			"lx6",			MACHINE_TYPE_SLOT1,		CPU_PKG_SLOT1, 0, 60000000, 100000000, 1500, 3500, 2.0, 5.5,							MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192,1048576, 8192, 255,		  machine_at_lx6_init, NULL			},
    { "[i440LX] Micronics Spitfire",		"spitfire",		MACHINE_TYPE_SLOT1,		CPU_PKG_SLOT1, 0, 66666667, 66666667, 1800, 3500, 1.5, 8.0,							MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192,1048576, 8192, 255,	     machine_at_spitfire_init, NULL			},

    /* 440EX */
    { "[i440EX] QDI EXCELLENT II",		"p6i440e2",		MACHINE_TYPE_SLOT1,		CPU_PKG_SLOT1, 0, 66666667, 83333333, 1800, 3500, 3.0, 8.0,							MACHINE_PCI | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 524288, 8192, 255,	     machine_at_p6i440e2_init, NULL			},

    /* 440BX */
    { "[i440BX] ASUS P2B-LS",			"p2bls",		MACHINE_TYPE_SLOT1,		CPU_PKG_SLOT1, 0, 50000000, 112121212, 1300, 3500, 1.5, 8.0,							MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192,1048576, 8192, 255,		machine_at_p2bls_init, NULL			},
    { "[i440BX] ASUS P3B-F",			"p3bf",			MACHINE_TYPE_SLOT1,		CPU_PKG_SLOT1, 0, 66666667, 150000000, 1300, 3500, 1.5, 8.0,							MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192,1048576, 8192, 255,		 machine_at_p3bf_init, NULL			},
    { "[i440BX] ABIT BF6",			"bf6",			MACHINE_TYPE_SLOT1,		CPU_PKG_SLOT1, 0, 66666667, 133333333, 1800, 3500, 1.5, 8.0,							MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		  		 8192, 786432, 8192, 255,		  machine_at_bf6_init, NULL			},
    { "[i440BX] AOpen AX6BC",			"ax6bc",		MACHINE_TYPE_SLOT1,		CPU_PKG_SLOT1, 0, 66666667, 112121212, 1800, 3500, 1.5, 8.0,							MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		  		 8192, 786432, 8192, 255,		machine_at_ax6bc_init, NULL			},
    { "[i440BX] Gigabyte GA-686BX",		"686bx",		MACHINE_TYPE_SLOT1,		CPU_PKG_SLOT1, 0, 66666667, 100000000, 1800, 3500, 1.5, 8.0,							MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,	 			 8192,1048576, 8192, 255,		machine_at_686bx_init, NULL			},
    { "[i440BX] HP Vectra VEi 8",		"vei8",			MACHINE_TYPE_SLOT1,		CPU_PKG_SLOT1, 0, 66666667, 100000000, 1800, 3500, 1.5, 8.0,							MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,	 			 8192,1048576, 8192, 255,		 machine_at_vei8_init, NULL			},
    { "[i440BX] Tyan Tsunami ATX",		"tsunamiatx",		MACHINE_TYPE_SLOT1,		CPU_PKG_SLOT1, 0, 66666667, 112121212, 1800, 3500, 1.5, 8.0,							MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL | MACHINE_SOUND,	 	 8192,1048576, 8192, 255,	   machine_at_tsunamiatx_init, at_tsunamiatx_get_device	},
    { "[i440BX] SuperMicro Super P6SBA",	"p6sba",		MACHINE_TYPE_SLOT1,		CPU_PKG_SLOT1, 0, 66666667, 100000000, 1800, 3500, 1.5, 8.0,							MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		  		 8192, 786432, 8192, 255,		machine_at_p6sba_init, NULL			},
#if defined(DEV_BRANCH) && defined(NO_SIO)
    { "[i440BX] Fujitsu ErgoPro x365",		"ergox365",		MACHINE_TYPE_SLOT1,		CPU_PKG_SLOT1, 0, 66666667, 100000000, 1800, 3500, 1.5, 8.0,							MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192, 393216, 8192, 511,	     machine_at_ergox365_init, NULL			},
#endif
    
    /* 440ZX */
    { "[i440ZX] MSI MS-6168",			"ms6168",		MACHINE_TYPE_SLOT1,		CPU_PKG_SLOT1, 0, 66666667, 100000000, 1800, 3500, 1.5, 8.0,							MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL | MACHINE_VIDEO | MACHINE_SOUND,8192, 524288, 8192, 255,	       machine_at_ms6168_init, NULL			},
    { "[i440ZX] Packard Bell Bora Pro",		"borapro",		MACHINE_TYPE_SLOT1,		CPU_PKG_SLOT1, 0, 66666667, 66666667, 1800, 3500, 1.5, 8.0,							MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL | MACHINE_VIDEO | MACHINE_SOUND,8192, 524288, 8192, 255,	      machine_at_borapro_init, NULL			},

    /* SMSC VictoryBX-66 */
    { "[SMSC VictoryBX-66] A-Trend ATC6310BXII","atc6310bxii",		MACHINE_TYPE_SLOT1,		CPU_PKG_SLOT1, 0, 66666667, 133333333, 1300, 3500, 1.5, 8.0,							MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192, 786432, 8192, 255,	  machine_at_atc6310bxii_init, NULL			},

    /* VIA Apollo Pro */
    { "[VIA Apollo Pro] FIC KA-6130",		"ficka6130",		MACHINE_TYPE_SLOT1,		CPU_PKG_SLOT1, 0, 66666667, 100000000, 1800, 3500, 1.5, 8.0,							MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,	 			 8192, 524288, 8192, 255,	    machine_at_ficka6130_init, NULL			},
    { "[VIA Apollo Pro133] ASUS P3V133",	"p3v133",		MACHINE_TYPE_SLOT1,		CPU_PKG_SLOT1, 0, 66666667, 150000000, 1300, 3500, 1.5, 8.0,							MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,	  			 8192,1572864, 8192, 255,	       machine_at_p3v133_init, NULL			},
    { "[VIA Apollo Pro133A] ASUS P3V4X",	"p3v4x",		MACHINE_TYPE_SLOT1,		CPU_PKG_SLOT1, 0, 66666667, 150000000, 1300, 3500, 1.5, 8.0,							MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,	  			 8192,2097152, 8192, 255,		machine_at_p3v4x_init, NULL			},

    /* Slot 1/2 machines */
    /* 440GX */
    { "[i440GX] Freeway FW-6400GX",		"fw6400gx",		MACHINE_TYPE_SLOT1_2,		CPU_PKG_SLOT1 | CPU_PKG_SLOT2, 0, 100000000, 150000000, 1800, 3500, 3.0, 8.0,					(MACHINE_AGP & ~MACHINE_AT) | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		16384,2080768,16384, 511,	     machine_at_fw6400gx_init, NULL			},

    /* Slot 2 machines */
    /* 440GX */
    { "[i440GX] Gigabyte GA-6GXU",		"6gxu",			MACHINE_TYPE_SLOT2,		CPU_PKG_SLOT2, 0, 100000000, 133333333, 1800, 3500, 1.5, 8.0,							MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		16384,2097152,16384, 511,		 machine_at_6gxu_init, NULL			},
    { "[i440GX] SuperMicro Super S2DGE",	"s2dge",		MACHINE_TYPE_SLOT2,		CPU_PKG_SLOT2, 0, 66666667, 100000000, 1800, 3500, 1.5, 8.0,							MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		16384,2097152,16384, 511,		machine_at_s2dge_init, NULL			},

    /* PGA370 machines */
    /* 440LX */
    { "[i440LX] SuperMicro Super 370SLM",	"s370slm",		MACHINE_TYPE_SOCKET370,		CPU_PKG_SOCKET370, 0, 66666667, 100000000, 1800, 3500, MACHINE_MULTIPLIER_FIXED,				MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192, 786432, 8192, 255,	      machine_at_s370slm_init, NULL			},

    /* 440BX */
    { "[i440BX] AEWIN AW-O671R",		"awo671r",		MACHINE_TYPE_SOCKET370,		CPU_PKG_SOCKET370, 0, 66666667, 133333333, 1300, 3500, 1.5, 8.0, /* limits assumed */				MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192, 524288, 8192, 255,	      machine_at_awo671r_init, NULL			},
    { "[i440BX] ASUS CUBX",			"cubx",			MACHINE_TYPE_SOCKET370,		CPU_PKG_SOCKET370, 0, 66666667, 150000000, 1300, 3500, 1.5, 8.0,						MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192,1048576, 8192, 255,		 machine_at_cubx_init, NULL			},
    { "[i440BX] AmazePC AM-BX133",		"ambx133",		MACHINE_TYPE_SOCKET370,		CPU_PKG_SOCKET370, 0, 66666667, 133333333, 1300, 3500, 1.5, 8.0, /* limits assumed */				MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192, 786432, 8192, 255,	      machine_at_ambx133_init, NULL			},
    { "[i440BX] Tyan Trinity 371",		"trinity371",		MACHINE_TYPE_SOCKET370,		CPU_PKG_SOCKET370, 0, 66666667, 133333333, 1300, 3500, 1.5, 8.0,						MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192, 786432, 8192, 255,	   machine_at_trinity371_init, NULL			},

    /* 440ZX */
    { "[i440ZX] Soltek SL-63A1",		"63a",			MACHINE_TYPE_SOCKET370,		CPU_PKG_SOCKET370, 0, 66666667, 100000000, 1800, 3500, 1.5, 8.0,						MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,				 8192, 524288, 8192, 255,		  machine_at_63a_init, NULL			},

    /* SMSC VictoryBX-66 */
    { "[SMSC VictoryBX-66] A-Trend ATC7020BXII","atc7020bxii",		MACHINE_TYPE_SOCKET370,		CPU_PKG_SOCKET370, 0, 66666667, 133333333, 1300, 3500, 1.5, 8.0,						MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		 		 8192,1048576, 8192, 255,	  machine_at_atc7020bxii_init, NULL			},

    /* VIA Apollo Pro */
    { "[VIA Apollo Pro] PC Partner APAS3",	"apas3",		MACHINE_TYPE_SOCKET370,		CPU_PKG_SOCKET370, 0, 66666667, 100000000, 1800, 3500, 1.5, 8.0,						MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,	  			 8192, 786432, 8192, 255,		machine_at_apas3_init, NULL			},
    { "[VIA Apollo Pro133] ECS P6BAP",		"p6bap",		MACHINE_TYPE_SOCKET370,		CPU_PKG_SOCKET370, 0, 66666667, 150000000, 1300, 3500, 1.5, 8.0,						MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,	  			 8192,1572864, 8192, 255,		machine_at_p6bap_init, NULL			},
    { "[VIA Apollo Pro133A] AEWIN WCF-681",	"wcf681",		MACHINE_TYPE_SOCKET370,		CPU_PKG_SOCKET370, 0, 66666667, 133333333, 1300, 3500, 1.5, 8.0, /* limits assumed */				MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,	  			 8192,1048576, 8192, 255,	       machine_at_wcf681_init, NULL			},
    { "[VIA Apollo Pro133A] ASUS CUV4X-LS",	"cuv4xls",		MACHINE_TYPE_SOCKET370,		CPU_PKG_SOCKET370, 0, 66666667, 150000000, 1300, 3500, 1.5, 8.0,						(MACHINE_AGP & ~MACHINE_AT) | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		16384,1572864, 8192, 255,	      machine_at_cuv4xls_init, NULL			},
    { "[VIA Apollo Pro133A] Acorp 6VIA90AP",	"6via90ap",		MACHINE_TYPE_SOCKET370,		CPU_PKG_SOCKET370, 0, 66666667, 150000000, 1300, 3500, MACHINE_MULTIPLIER_FIXED,				MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,	  			 8192,1572864, 8192, 255,	     machine_at_6via90ap_init, NULL			},
    { "[VIA Apollo ProMedia] Jetway 603TCF",	"603tcf",		MACHINE_TYPE_SOCKET370,		CPU_PKG_SOCKET370, 0, 66666667, 150000000, 1300, 3500, 1.5, 8.0,						MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,	  			 8192,1048576, 8192, 255,	       machine_at_603tcf_init, NULL			},

    /* Miscellaneous/Fake/Hypervisor machines */
    { "[i440BX] Microsoft Virtual PC 2007",	"vpc2007",		MACHINE_TYPE_MISC,		CPU_PKG_SLOT1, CPU_BLOCK(CPU_PENTIUM2, CPU_CYRIX3S), 0, 0, 0, 0, 0, 0,						MACHINE_AGP | MACHINE_BUS_PS2 | MACHINE_IDE_DUAL,		  		 8192,1048576, 8192, 255,	      machine_at_vpc2007_init, NULL			},

    { NULL,					NULL,			MACHINE_TYPE_NONE,		0, 0, 0, 0, 0, 0, 0, 0,												0,										    0,      0,    0,   0,				 NULL, NULL			}
};


int
machine_count(void)
{
    return((sizeof(machines) / sizeof(machine_t)) - 1);
}


char *
machine_getname(void)
{
    return((char *)machines[machine].name);
}


char *
machine_getname_ex(int m)
{
    return((char *)machines[m].name);
}


const device_t *
machine_getdevice(int m)
{
    if (machines[m].get_device)
	return(machines[m].get_device());

    return(NULL);
}


char *
machine_get_internal_name(void)
{
    return((char *)machines[machine].internal_name);
}


char *
machine_get_internal_name_ex(int m)
{
    return((char *)machines[m].internal_name);
}


int
machine_get_nvrmask(int m)
{
    return(machines[m].nvrmask);
}


int
machine_get_machine_from_internal_name(char *s)
{
    int c = 0;

    while (machines[c].init != NULL) {
	if (!strcmp(machines[c].internal_name, (const char *)s))
		return(c);
	c++;
    }

    return(0);
}
