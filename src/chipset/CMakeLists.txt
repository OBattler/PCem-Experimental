#
# 86Box		A hypervisor and IBM PC system emulator that specializes in
#		running old operating systems and software designed for IBM
#		PC systems and compatibles from 1981 through fairly recent
#		system designs based on the PCI bus.
#
#		This file is part of the 86Box distribution.
#
#		CMake build script.
#
# Authors:	David Hrdlička, <hrdlickadavid@outlook.com>
#
#		Copyright 2020,2021 David Hrdlička.
#

add_library(chipset OBJECT acc2168.c cs8230.c ali1217.c ali1429.c ali1489.c et6000.c headland.c
	intel_82335.c cs4031.c intel_420ex.c intel_4x0.c intel_sio.c intel_piix.c ../ioapic.c
	neat.c opti283.c opti291.c opti495.c opti822.c opti895.c opti5x7.c scamp.c scat.c
	sis_85c310.c sis_85c4xx.c sis_85c496.c sis_85c50x.c sis_5511.c sis_5571.c sis_5598.c
	umc_8886.c umc_hb4.c
	via_vt82c49x.c via_vt82c505.c sis_85c310.c sis_85c4xx.c sis_85c496.c sis_85c50x.c
	gc100.c stpc.c 
	via_apollo.c via_pipc.c wd76c10.c
	vl82c480.c)

if(I450KX)
	target_sources(chipset PRIVATE intel_i450kx.c)
endif()

if(M154X)
	target_sources(chipset PRIVATE ali1531.c)
	target_sources(chipset PRIVATE ali1543.c)
endif()

if(M6117)
	target_sources(chipset PRIVATE ali6117.c)
endif()

if(OLIVETTI)
	target_sources(chipset PRIVATE olivetti_eva.c)
endif()