#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <math.h>
#ifndef INFINITY
# define INFINITY   (__builtin_inff())
#endif
#define HAVE_STDARG_H
#include <86box/86box.h>
#include "cpu.h"
#include <86box/timer.h>
#include "x86.h"
#include "x87.h"
#include <86box/nmi.h>
#include <86box/mem.h>
#include <86box/smram.h>
#include <86box/pic.h>
#include <86box/pit.h>
#include <86box/fdd.h>
#include <86box/fdc.h>
#include <86box/keyboard.h>
#include "386_common.h"
#include "x86_flags.h"
#include "x86seg.h"

#ifdef USE_DYNAREC
#include "codegen.h"
#define CPU_BLOCK_END() cpu_block_end = 1
#else
#define CPU_BLOCK_END()
#endif


x86seg gdt, ldt, idt, tr;

uint32_t cr2, cr3, cr4;
uint32_t dr[8];

uint32_t use32;
int stack32;

uint32_t *eal_r, *eal_w;

int nmi_enable = 1;

int alt_access, cpl_override = 0;

#ifdef USE_NEW_DYNAREC
uint16_t cpu_cur_status = 0;
#else
uint32_t cpu_cur_status = 0;
#endif

extern uint8_t *pccache2;

extern int optype;
extern uint32_t pccache;


int in_sys = 0, unmask_a20_in_smm = 0;
uint32_t old_rammask = 0xffffffff;

int soft_reset_mask = 0;

int in_smm = 0, smi_line = 0, smi_latched = 0, smm_in_hlt = 0;
int smi_block = 0;
uint32_t smbase = 0x30000;

uint32_t addr64, addr64_2;
uint32_t addr64a[8], addr64a_2[8];


#define AMD_SYSCALL_EIP	(msr.star & 0xFFFFFFFF)
#define AMD_SYSCALL_SB	((msr.star >> 32) & 0xFFFF)
#define AMD_SYSRET_SB	((msr.star >> 48) & 0xFFFF)


/* These #define's and enum have been borrowed from Bochs. */
/* SMM feature masks */
#define SMM_IO_INSTRUCTION_RESTART  (0x00010000)
#define SMM_SMBASE_RELOCATION       (0x00020000)
#define SMM_REVISION		    (0x20000000)

/* TODO: Which CPU added SMBASE relocation? */
#define SMM_REVISION_ID (SMM_SMBASE_RELOCATION | SMM_IO_INSTRUCTION_RESTART | SMM_REVISION)

#define SMM_SAVE_STATE_MAP_SIZE 128


enum SMMRAM_Fields_386_To_P5 {
    SMRAM_FIELD_P5_CR0 = 0,		/* 1FC */
    SMRAM_FIELD_P5_CR3,			/* 1F8 */
    SMRAM_FIELD_P5_EFLAGS,		/* 1F4 */
    SMRAM_FIELD_P5_EIP,			/* 1F0 */
    SMRAM_FIELD_P5_EDI,			/* 1EC */
    SMRAM_FIELD_P5_ESI,			/* 1E8 */
    SMRAM_FIELD_P5_EBP,			/* 1E4 */
    SMRAM_FIELD_P5_ESP,			/* 1E0 */
    SMRAM_FIELD_P5_EBX,			/* 1DC */
    SMRAM_FIELD_P5_EDX,			/* 1D8 */
    SMRAM_FIELD_P5_ECX,			/* 1D4 */
    SMRAM_FIELD_P5_EAX,			/* 1D0 */
    SMRAM_FIELD_P5_DR6,			/* 1CC */
    SMRAM_FIELD_P5_DR7,			/* 1C8 */
    SMRAM_FIELD_P5_TR_SELECTOR,		/* 1C4 */
    SMRAM_FIELD_P5_LDTR_SELECTOR,	/* 1C0 */
    SMRAM_FIELD_P5_GS_SELECTOR,		/* 1BC */
    SMRAM_FIELD_P5_FS_SELECTOR,		/* 1B8 */
    SMRAM_FIELD_P5_DS_SELECTOR,		/* 1B4 */
    SMRAM_FIELD_P5_SS_SELECTOR,		/* 1B0 */
    SMRAM_FIELD_P5_CS_SELECTOR,		/* 1AC */
    SMRAM_FIELD_P5_ES_SELECTOR,		/* 1A8 */
    SMRAM_FIELD_P5_TR_ACCESS,		/* 1A4 */
    SMRAM_FIELD_P5_TR_BASE,		/* 1A0 */
    SMRAM_FIELD_P5_TR_LIMIT,		/* 19C */
    SMRAM_FIELD_P5_IDTR_ACCESS,		/* 198 */
    SMRAM_FIELD_P5_IDTR_BASE,		/* 194 */
    SMRAM_FIELD_P5_IDTR_LIMIT,		/* 190 */
    SMRAM_FIELD_P5_GDTR_ACCESS,		/* 18C */
    SMRAM_FIELD_P5_GDTR_BASE,		/* 188 */
    SMRAM_FIELD_P5_GDTR_LIMIT,		/* 184 */
    SMRAM_FIELD_P5_LDTR_ACCESS,		/* 180 */
    SMRAM_FIELD_P5_LDTR_BASE,		/* 17C */
    SMRAM_FIELD_P5_LDTR_LIMIT,		/* 178 */
    SMRAM_FIELD_P5_GS_ACCESS,		/* 174 */
    SMRAM_FIELD_P5_GS_BASE,		/* 170 */
    SMRAM_FIELD_P5_GS_LIMIT,		/* 16C */
    SMRAM_FIELD_P5_FS_ACCESS,		/* 168 */
    SMRAM_FIELD_P5_FS_BASE,		/* 164 */
    SMRAM_FIELD_P5_FS_LIMIT,		/* 160 */
    SMRAM_FIELD_P5_DS_ACCESS,		/* 15C */
    SMRAM_FIELD_P5_DS_BASE,		/* 158 */
    SMRAM_FIELD_P5_DS_LIMIT,		/* 154 */
    SMRAM_FIELD_P5_SS_ACCESS,		/* 150 */
    SMRAM_FIELD_P5_SS_BASE,		/* 14C */
    SMRAM_FIELD_P5_SS_LIMIT,		/* 148 */
    SMRAM_FIELD_P5_CS_ACCESS,		/* 144 */
    SMRAM_FIELD_P5_CS_BASE,		/* 140 */
    SMRAM_FIELD_P5_CS_LIMIT,		/* 13C */
    SMRAM_FIELD_P5_ES_ACCESS,		/* 138 */
    SMRAM_FIELD_P5_ES_BASE,		/* 134 */
    SMRAM_FIELD_P5_ES_LIMIT,		/* 130 */
    SMRAM_FIELD_P5_UNWRITTEN_1,		/* 12C */
    SMRAM_FIELD_P5_CR4,			/* 128 */
    SMRAM_FIELD_P5_ALTERNATE_DR6,	/* 124 */
    SMRAM_FIELD_P5_RESERVED_1,		/* 120 */
    SMRAM_FIELD_P5_RESERVED_2,		/* 11C */
    SMRAM_FIELD_P5_RESERVED_3,		/* 118 */
    SMRAM_FIELD_P5_RESERVED_4,		/* 114 */
    SMRAM_FIELD_P5_IO_RESTART_EIP,	/* 110 */
    SMRAM_FIELD_P5_IO_RESTART_ESI,	/* 10C */
    SMRAM_FIELD_P5_IO_RESTART_ECX,	/* 108 */
    SMRAM_FIELD_P5_IO_RESTART_EDI,	/* 104 */
    SMRAM_FIELD_P5_AUTOHALT_RESTART,	/* 100 */
    SMRAM_FIELD_P5_SMM_REVISION_ID,	/* 0FC */
    SMRAM_FIELD_P5_SMBASE_OFFSET,	/* 0F8 */
    SMRAM_FIELD_AM486_CR2,		/* 0F4 */
    SMRAM_FIELD_AM486_DR0,		/* 0F0 */
    SMRAM_FIELD_AM486_DR1,		/* 0EC */
    SMRAM_FIELD_AM486_DR2,		/* 0E8 */
    SMRAM_FIELD_AM486_DR3,		/* 0E4 */
    SMRAM_FIELD_P5_LAST
};

enum SMMRAM_Fields_P6 {
    SMRAM_FIELD_P6_CR0 = 0,		/* 1FC */
    SMRAM_FIELD_P6_CR3,			/* 1F8 */
    SMRAM_FIELD_P6_EFLAGS,		/* 1F4 */
    SMRAM_FIELD_P6_EIP,			/* 1F0 */
    SMRAM_FIELD_P6_EDI,			/* 1EC */
    SMRAM_FIELD_P6_ESI,			/* 1E8 */
    SMRAM_FIELD_P6_EBP,			/* 1E4 */
    SMRAM_FIELD_P6_ESP,			/* 1E0 */
    SMRAM_FIELD_P6_EBX,			/* 1DC */
    SMRAM_FIELD_P6_EDX,			/* 1D8 */
    SMRAM_FIELD_P6_ECX,			/* 1D4 */
    SMRAM_FIELD_P6_EAX,			/* 1D0 */
    SMRAM_FIELD_P6_DR6,			/* 1CC */
    SMRAM_FIELD_P6_DR7,			/* 1C8 */
    SMRAM_FIELD_P6_TR_SELECTOR,		/* 1C4 */
    SMRAM_FIELD_P6_LDTR_SELECTOR,	/* 1C0 */
    SMRAM_FIELD_P6_GS_SELECTOR,		/* 1BC */
    SMRAM_FIELD_P6_FS_SELECTOR,		/* 1B8 */
    SMRAM_FIELD_P6_DS_SELECTOR,		/* 1B4 */
    SMRAM_FIELD_P6_SS_SELECTOR,		/* 1B0 */
    SMRAM_FIELD_P6_CS_SELECTOR,		/* 1AC */
    SMRAM_FIELD_P6_ES_SELECTOR,		/* 1A8 */
    SMRAM_FIELD_P6_SS_BASE,		/* 1A4 */
    SMRAM_FIELD_P6_SS_LIMIT,		/* 1A0 */
    SMRAM_FIELD_P6_SS_SELECTOR_AR,	/* 19C */
    SMRAM_FIELD_P6_CS_BASE,		/* 198 */
    SMRAM_FIELD_P6_CS_LIMIT,		/* 194 */
    SMRAM_FIELD_P6_CS_SELECTOR_AR,	/* 190 */
    SMRAM_FIELD_P6_ES_BASE,		/* 18C */
    SMRAM_FIELD_P6_ES_LIMIT,		/* 188 */
    SMRAM_FIELD_P6_ES_SELECTOR_AR,	/* 184 */
    SMRAM_FIELD_P6_LDTR_BASE,		/* 180 */
    SMRAM_FIELD_P6_LDTR_LIMIT,		/* 17C */
    SMRAM_FIELD_P6_LDTR_SELECTOR_AR,	/* 178 */
    SMRAM_FIELD_P6_GDTR_BASE,		/* 174 */
    SMRAM_FIELD_P6_GDTR_LIMIT,		/* 170 */
    SMRAM_FIELD_P6_GDTR_SELECTOR_AR,	/* 16C */
    SMRAM_FIELD_P6_SREG_STATUS1,	/* 168 */
    SMRAM_FIELD_P6_TR_BASE,		/* 164 */
    SMRAM_FIELD_P6_TR_LIMIT,		/* 160 */
    SMRAM_FIELD_P6_TR_SELECTOR_AR,	/* 15C */
    SMRAM_FIELD_P6_IDTR_BASE,		/* 158 */
    SMRAM_FIELD_P6_IDTR_LIMIT,		/* 154 */
    SMRAM_FIELD_P6_IDTR_SELECTOR_AR,	/* 150 */
    SMRAM_FIELD_P6_GS_BASE,		/* 14C */
    SMRAM_FIELD_P6_GS_LIMIT,		/* 148 */
    SMRAM_FIELD_P6_GS_SELECTOR_AR,	/* 144 */
    SMRAM_FIELD_P6_FS_BASE,		/* 140 */
    SMRAM_FIELD_P6_FS_LIMIT,		/* 13C */
    SMRAM_FIELD_P6_FS_SELECTOR_AR,	/* 138 */
    SMRAM_FIELD_P6_DS_BASE,		/* 134 */
    SMRAM_FIELD_P6_DS_LIMIT,		/* 130 */
    SMRAM_FIELD_P6_DS_SELECTOR_AR,	/* 12C */
    SMRAM_FIELD_P6_SREG_STATUS0,	/* 128 */
    SMRAM_FIELD_P6_ALTERNATIVE_DR6,	/* 124 */
    SMRAM_FIELD_P6_CPL,			/* 120 */
    SMRAM_FIELD_P6_SMM_STATUS,		/* 11C */
    SMRAM_FIELD_P6_A20M,		/* 118 */
    SMRAM_FIELD_P6_CR4,			/* 114 */
    SMRAM_FIELD_P6_IO_RESTART_EIP,	/* 110 */
    SMRAM_FIELD_P6_IO_RESTART_ESI,	/* 10C */
    SMRAM_FIELD_P6_IO_RESTART_ECX,	/* 108 */
    SMRAM_FIELD_P6_IO_RESTART_EDI,	/* 104 */
    SMRAM_FIELD_P6_AUTOHALT_RESTART,	/* 100 */
    SMRAM_FIELD_P6_SMM_REVISION_ID,	/* 0FC */
    SMRAM_FIELD_P6_SMBASE_OFFSET,	/* 0F8 */
    SMRAM_FIELD_P6_LAST
};

enum SMMRAM_Fields_AMD_K {
    SMRAM_FIELD_AMD_K_CR0 = 0,		/* 1FC */
    SMRAM_FIELD_AMD_K_CR3,		/* 1F8 */
    SMRAM_FIELD_AMD_K_EFLAGS,		/* 1F4 */
    SMRAM_FIELD_AMD_K_EIP,		/* 1F0 */
    SMRAM_FIELD_AMD_K_EDI,		/* 1EC */
    SMRAM_FIELD_AMD_K_ESI,		/* 1E8 */
    SMRAM_FIELD_AMD_K_EBP,		/* 1E4 */
    SMRAM_FIELD_AMD_K_ESP,		/* 1E0 */
    SMRAM_FIELD_AMD_K_EBX,		/* 1DC */
    SMRAM_FIELD_AMD_K_EDX,		/* 1D8 */
    SMRAM_FIELD_AMD_K_ECX,		/* 1D4 */
    SMRAM_FIELD_AMD_K_EAX,		/* 1D0 */
    SMRAM_FIELD_AMD_K_DR6,		/* 1CC */
    SMRAM_FIELD_AMD_K_DR7,		/* 1C8 */
    SMRAM_FIELD_AMD_K_TR_SELECTOR,	/* 1C4 */
    SMRAM_FIELD_AMD_K_LDTR_SELECTOR,	/* 1C0 */
    SMRAM_FIELD_AMD_K_GS_SELECTOR,	/* 1BC */
    SMRAM_FIELD_AMD_K_FS_SELECTOR,	/* 1B8 */
    SMRAM_FIELD_AMD_K_DS_SELECTOR,	/* 1B4 */
    SMRAM_FIELD_AMD_K_SS_SELECTOR,	/* 1B0 */
    SMRAM_FIELD_AMD_K_CS_SELECTOR,	/* 1AC */
    SMRAM_FIELD_AMD_K_ES_SELECTOR,	/* 1A8 */
    SMRAM_FIELD_AMD_K_IO_RESTART_DWORD,	/* 1A4 */
    SMRAM_FIELD_AMD_K_RESERVED_1,	/* 1A0 */
    SMRAM_FIELD_AMD_K_IO_RESTART_EIP,	/* 19C */
    SMRAM_FIELD_AMD_K_RESERVED_2,	/* 198 */
    SMRAM_FIELD_AMD_K_RESERVED_3,	/* 194 */
    SMRAM_FIELD_AMD_K_IDTR_BASE,	/* 190 */
    SMRAM_FIELD_AMD_K_IDTR_LIMIT,	/* 18C */
    SMRAM_FIELD_AMD_K_GDTR_BASE,	/* 188 */
    SMRAM_FIELD_AMD_K_GDTR_LIMIT,	/* 184 */
    SMRAM_FIELD_AMD_K_TR_ACCESS,	/* 180 */
    SMRAM_FIELD_AMD_K_TR_BASE,		/* 17C */
    SMRAM_FIELD_AMD_K_TR_LIMIT,		/* 178 */
    SMRAM_FIELD_AMD_K_LDTR_ACCESS,	/* 174 - reserved on K6 */
    SMRAM_FIELD_AMD_K_LDTR_BASE,	/* 170 */
    SMRAM_FIELD_AMD_K_LDTR_LIMIT,	/* 16C */
    SMRAM_FIELD_AMD_K_GS_ACCESS,	/* 168 */
    SMRAM_FIELD_AMD_K_GS_BASE,		/* 164 */
    SMRAM_FIELD_AMD_K_GS_LIMIT,		/* 160 */
    SMRAM_FIELD_AMD_K_FS_ACCESS,	/* 15C */
    SMRAM_FIELD_AMD_K_FS_BASE,		/* 158 */
    SMRAM_FIELD_AMD_K_FS_LIMIT,		/* 154 */
    SMRAM_FIELD_AMD_K_DS_ACCESS,	/* 150 */
    SMRAM_FIELD_AMD_K_DS_BASE,		/* 14C */
    SMRAM_FIELD_AMD_K_DS_LIMIT,		/* 148 */
    SMRAM_FIELD_AMD_K_SS_ACCESS,	/* 144 */
    SMRAM_FIELD_AMD_K_SS_BASE,		/* 140 */
    SMRAM_FIELD_AMD_K_SS_LIMIT,		/* 13C */
    SMRAM_FIELD_AMD_K_CS_ACCESS,	/* 138 */
    SMRAM_FIELD_AMD_K_CS_BASE,		/* 134 */
    SMRAM_FIELD_AMD_K_CS_LIMIT,		/* 130 */
    SMRAM_FIELD_AMD_K_ES_ACCESS,	/* 12C */
    SMRAM_FIELD_AMD_K_ES_BASE,		/* 128 */
    SMRAM_FIELD_AMD_K_ES_LIMIT,		/* 124 */
    SMRAM_FIELD_AMD_K_RESERVED_4,	/* 120 */
    SMRAM_FIELD_AMD_K_RESERVED_5,	/* 11C */
    SMRAM_FIELD_AMD_K_RESERVED_6,	/* 118 */
    SMRAM_FIELD_AMD_K_CR2,		/* 114 */
    SMRAM_FIELD_AMD_K_CR4,		/* 110 */
    SMRAM_FIELD_AMD_K_IO_RESTART_ESI,	/* 10C */
    SMRAM_FIELD_AMD_K_IO_RESTART_ECX,	/* 108 */
    SMRAM_FIELD_AMD_K_IO_RESTART_EDI,	/* 104 */
    SMRAM_FIELD_AMD_K_AUTOHALT_RESTART,	/* 100 */
    SMRAM_FIELD_AMD_K_SMM_REVISION_ID,	/* 0FC */
    SMRAM_FIELD_AMD_K_SMBASE_OFFSET,	/* 0F8 */
    SMRAM_FIELD_AMD_K_LAST
};

#ifdef ENABLE_386_COMMON_LOG
int x386_common_do_log = ENABLE_386_COMMON_LOG;


void
x386_common_log(const char *fmt, ...)
{
    va_list ap;

    if (x386_common_do_log) {
	va_start(ap, fmt);
	pclog_ex(fmt, ap);
	va_end(ap);
    }
}
#else
#define x386_common_log(fmt, ...)
#endif


static __inline void
set_stack32(int s)
{
    if ((cr0 & 1) && ! (cpu_state.eflags & VM_FLAG))
	stack32 = s;
    else
	stack32 = 0;

    if (stack32)
	cpu_cur_status |= CPU_STATUS_STACK32;
    else
	cpu_cur_status &= ~CPU_STATUS_STACK32;
}


static __inline void
set_use32(int u)
{
    if ((cr0 & 1) && ! (cpu_state.eflags & VM_FLAG))
	use32 = u ? 0x300 : 0;
    else
	use32 = 0;

    if (use32)
	cpu_cur_status |= CPU_STATUS_USE32;
    else
	cpu_cur_status &= ~CPU_STATUS_USE32;
}


static void
smm_seg_load(x86seg *s)
{
    if (!is386)
	s->base &= 0x00ffffff;

    if ((s->access & 0x18) != 0x10 || !(s->access & (1 << 2))) {
	/* Expand down. */
	s->limit_high = s->limit;
	s->limit_low = 0;
    } else {
	s->limit_high = (s->ar_high & 0x40) ? 0xffffffff : 0xffff;
	s->limit_low = s->limit + 1;
    }

    if ((cr0 & 1) && !(cpu_state.eflags & VM_FLAG))
	s->checked = s->seg ? 1 : 0;
    else
	s->checked = 1;

    if (s == &cpu_state.seg_cs)
	set_use32(s->ar_high & 0x40);

    if (s == &cpu_state.seg_ds) {
	if (s->base == 0 && s->limit_low == 0 && s->limit_high == 0xffffffff)
		cpu_cur_status &= ~CPU_STATUS_NOTFLATDS;
	else
		cpu_cur_status |= CPU_STATUS_NOTFLATDS;
    }

    if (s == &cpu_state.seg_ss) {
	if (s->base == 0 && s->limit_low == 0 && s->limit_high == 0xffffffff)
		cpu_cur_status &= ~CPU_STATUS_NOTFLATSS;
	else
		cpu_cur_status |= CPU_STATUS_NOTFLATSS;
	set_stack32((s->ar_high & 0x40) ? 1 : 0);
    }
}


static void
smram_save_state_p5(uint32_t *saved_state, int in_hlt)
{
    int n = 0;

    saved_state[SMRAM_FIELD_P5_SMM_REVISION_ID] = SMM_REVISION_ID;
    saved_state[SMRAM_FIELD_P5_SMBASE_OFFSET] = smbase & 0x00ffffff;

    for (n = 0; n < 8; n++)
	saved_state[SMRAM_FIELD_P5_EAX - n] = cpu_state.regs[n].l;

    if (in_hlt)
	saved_state[SMRAM_FIELD_P5_AUTOHALT_RESTART] = 1;
    else
	saved_state[SMRAM_FIELD_P5_AUTOHALT_RESTART] = 0;

    saved_state[SMRAM_FIELD_P5_EIP] = cpu_state.pc;

    saved_state[SMRAM_FIELD_P5_EFLAGS] = (cpu_state.eflags << 16) | (cpu_state.flags);

    saved_state[SMRAM_FIELD_P5_CR0] = cr0;
    saved_state[SMRAM_FIELD_P5_CR3] = cr3;
    saved_state[SMRAM_FIELD_P5_CR4] = cr4;
    saved_state[SMRAM_FIELD_P5_DR6] = dr[6];
    saved_state[SMRAM_FIELD_P5_DR7] = dr[7];

    /* TR */
    saved_state[SMRAM_FIELD_P5_TR_SELECTOR] = tr.seg;
    saved_state[SMRAM_FIELD_P5_TR_BASE] = tr.base;
    saved_state[SMRAM_FIELD_P5_TR_LIMIT] = tr.limit;
    saved_state[SMRAM_FIELD_P5_TR_ACCESS] = (tr.ar_high << 16) | (tr.access << 8);

    /* LDTR */
    saved_state[SMRAM_FIELD_P5_LDTR_SELECTOR] = ldt.seg;
    saved_state[SMRAM_FIELD_P5_LDTR_BASE] = ldt.base;
    saved_state[SMRAM_FIELD_P5_LDTR_LIMIT] = ldt.limit;
    saved_state[SMRAM_FIELD_P5_LDTR_ACCESS] = (ldt.ar_high << 16) | (ldt.access << 8);

    /* IDTR */
    saved_state[SMRAM_FIELD_P5_IDTR_BASE] = idt.base;
    saved_state[SMRAM_FIELD_P5_IDTR_LIMIT] = idt.limit;
    saved_state[SMRAM_FIELD_P5_IDTR_ACCESS] = (idt.ar_high << 16) | (idt.access << 8);

    /* GDTR */
    saved_state[SMRAM_FIELD_P5_GDTR_BASE] = gdt.base;
    saved_state[SMRAM_FIELD_P5_GDTR_LIMIT] = gdt.limit;
    saved_state[SMRAM_FIELD_P5_GDTR_ACCESS] = (gdt.ar_high << 16) | (gdt.access << 8);

    /* ES */
    saved_state[SMRAM_FIELD_P5_ES_SELECTOR] = cpu_state.seg_es.seg;
    saved_state[SMRAM_FIELD_P5_ES_BASE] = cpu_state.seg_es.base;
    saved_state[SMRAM_FIELD_P5_ES_LIMIT] = cpu_state.seg_es.limit;
    saved_state[SMRAM_FIELD_P5_ES_ACCESS] = (cpu_state.seg_es.ar_high << 16) | (cpu_state.seg_es.access << 8);

    /* CS */
    saved_state[SMRAM_FIELD_P5_CS_SELECTOR] = cpu_state.seg_cs.seg;
    saved_state[SMRAM_FIELD_P5_CS_BASE] = cpu_state.seg_cs.base;
    saved_state[SMRAM_FIELD_P5_CS_LIMIT] = cpu_state.seg_cs.limit;
    saved_state[SMRAM_FIELD_P5_CS_ACCESS] = (cpu_state.seg_cs.ar_high << 16) | (cpu_state.seg_cs.access << 8);

    /* DS */
    saved_state[SMRAM_FIELD_P5_DS_SELECTOR] = cpu_state.seg_ds.seg;
    saved_state[SMRAM_FIELD_P5_DS_BASE] = cpu_state.seg_ds.base;
    saved_state[SMRAM_FIELD_P5_DS_LIMIT] = cpu_state.seg_ds.limit;
    saved_state[SMRAM_FIELD_P5_DS_ACCESS] = (cpu_state.seg_ds.ar_high << 16) | (cpu_state.seg_ds.access << 8);

    /* SS */
    saved_state[SMRAM_FIELD_P5_SS_SELECTOR] = cpu_state.seg_ss.seg;
    saved_state[SMRAM_FIELD_P5_SS_BASE] = cpu_state.seg_ss.base;
    saved_state[SMRAM_FIELD_P5_SS_LIMIT] = cpu_state.seg_ss.limit;
    saved_state[SMRAM_FIELD_P5_SS_ACCESS] = (cpu_state.seg_ss.ar_high << 16) | (cpu_state.seg_ss.access << 8);

    /* FS */
    saved_state[SMRAM_FIELD_P5_FS_SELECTOR] = cpu_state.seg_fs.seg;
    saved_state[SMRAM_FIELD_P5_FS_BASE] = cpu_state.seg_fs.base;
    saved_state[SMRAM_FIELD_P5_FS_LIMIT] = cpu_state.seg_fs.limit;
    saved_state[SMRAM_FIELD_P5_FS_ACCESS] = (cpu_state.seg_fs.ar_high << 16) | (cpu_state.seg_fs.access << 8);

    /* GS */
    saved_state[SMRAM_FIELD_P5_GS_SELECTOR] = cpu_state.seg_gs.seg;
    saved_state[SMRAM_FIELD_P5_GS_BASE] = cpu_state.seg_gs.base;
    saved_state[SMRAM_FIELD_P5_GS_LIMIT] = cpu_state.seg_gs.limit;
    saved_state[SMRAM_FIELD_P5_GS_ACCESS] = (cpu_state.seg_gs.ar_high << 16) | (cpu_state.seg_gs.access << 8);

    /* Am486/5x86 stuff */
    if (!is_pentium) {
	saved_state[SMRAM_FIELD_AM486_CR2] = cr2;
	saved_state[SMRAM_FIELD_AM486_DR0] = dr[0];
	saved_state[SMRAM_FIELD_AM486_DR1] = dr[1];
	saved_state[SMRAM_FIELD_AM486_DR2] = dr[2];
	saved_state[SMRAM_FIELD_AM486_DR3] = dr[3];
    }
}


static void
smram_restore_state_p5(uint32_t *saved_state)
{
    int n = 0;

    for (n = 0; n < 8; n++)
	cpu_state.regs[n].l = saved_state[SMRAM_FIELD_P5_EAX - n];

    if (saved_state[SMRAM_FIELD_P5_AUTOHALT_RESTART] & 0xffff)
	cpu_state.pc = saved_state[SMRAM_FIELD_P5_EIP] - 1;
    else
	cpu_state.pc = saved_state[SMRAM_FIELD_P5_EIP];

    cpu_state.eflags = saved_state[SMRAM_FIELD_P5_EFLAGS] >> 16;
    cpu_state.flags = saved_state[SMRAM_FIELD_P5_EFLAGS] & 0xffff;

    cr0 = saved_state[SMRAM_FIELD_P5_CR0];
    cr3 = saved_state[SMRAM_FIELD_P5_CR3];
    cr4 = saved_state[SMRAM_FIELD_P5_CR4];
    dr[6] = saved_state[SMRAM_FIELD_P5_DR6];
    dr[7] = saved_state[SMRAM_FIELD_P5_DR7];

    /* TR */
    tr.seg = saved_state[SMRAM_FIELD_P5_TR_SELECTOR];
    tr.base = saved_state[SMRAM_FIELD_P5_TR_BASE];
    tr.limit = saved_state[SMRAM_FIELD_P5_TR_LIMIT];
    tr.access = (saved_state[SMRAM_FIELD_P5_TR_ACCESS] >> 8) & 0xff;
    tr.ar_high = (saved_state[SMRAM_FIELD_P5_TR_ACCESS] >> 16) & 0xff;
    smm_seg_load(&tr);

    /* LDTR */
    ldt.seg = saved_state[SMRAM_FIELD_P5_LDTR_SELECTOR];
    ldt.base = saved_state[SMRAM_FIELD_P5_LDTR_BASE];
    ldt.limit = saved_state[SMRAM_FIELD_P5_LDTR_LIMIT];
    ldt.access = (saved_state[SMRAM_FIELD_P5_LDTR_ACCESS] >> 8) & 0xff;
    ldt.ar_high = (saved_state[SMRAM_FIELD_P5_LDTR_ACCESS] >> 16) & 0xff;
    smm_seg_load(&ldt);

    /* IDTR */
    idt.base = saved_state[SMRAM_FIELD_P5_IDTR_BASE];
    idt.limit = saved_state[SMRAM_FIELD_P5_IDTR_LIMIT];
    idt.access = (saved_state[SMRAM_FIELD_P5_IDTR_ACCESS] >> 8) & 0xff;
    idt.ar_high = (saved_state[SMRAM_FIELD_P5_IDTR_ACCESS] >> 16) & 0xff;

    /* GDTR */
    gdt.base = saved_state[SMRAM_FIELD_P5_GDTR_BASE];
    gdt.limit = saved_state[SMRAM_FIELD_P5_GDTR_LIMIT];
    gdt.access = (saved_state[SMRAM_FIELD_P5_GDTR_ACCESS] >> 8) & 0xff;
    gdt.ar_high = (saved_state[SMRAM_FIELD_P5_GDTR_ACCESS] >> 16) & 0xff;

    /* ES */
    cpu_state.seg_es.seg = saved_state[SMRAM_FIELD_P5_ES_SELECTOR];
    cpu_state.seg_es.base = saved_state[SMRAM_FIELD_P5_ES_BASE];
    cpu_state.seg_es.limit = saved_state[SMRAM_FIELD_P5_ES_LIMIT];
    cpu_state.seg_es.access = (saved_state[SMRAM_FIELD_P5_ES_ACCESS] >> 8) & 0xff;
    cpu_state.seg_es.ar_high = (saved_state[SMRAM_FIELD_P5_ES_ACCESS] >> 16) & 0xff;
    smm_seg_load(&cpu_state.seg_es);

    /* CS */
    cpu_state.seg_cs.seg = saved_state[SMRAM_FIELD_P5_CS_SELECTOR];
    cpu_state.seg_cs.base = saved_state[SMRAM_FIELD_P5_CS_BASE];
    cpu_state.seg_cs.limit = saved_state[SMRAM_FIELD_P5_CS_LIMIT];
    cpu_state.seg_cs.access = (saved_state[SMRAM_FIELD_P5_CS_ACCESS] >> 8) & 0xff;
    cpu_state.seg_cs.ar_high = (saved_state[SMRAM_FIELD_P5_CS_ACCESS] >> 16) & 0xff;
    smm_seg_load(&cpu_state.seg_cs);

    /* DS */
    cpu_state.seg_ds.seg = saved_state[SMRAM_FIELD_P5_DS_SELECTOR];
    cpu_state.seg_ds.base = saved_state[SMRAM_FIELD_P5_DS_BASE];
    cpu_state.seg_ds.limit = saved_state[SMRAM_FIELD_P5_DS_LIMIT];
    cpu_state.seg_ds.access = (saved_state[SMRAM_FIELD_P5_DS_ACCESS] >> 8) & 0xff;
    cpu_state.seg_ds.ar_high = (saved_state[SMRAM_FIELD_P5_DS_ACCESS] >> 16) & 0xff;
    smm_seg_load(&cpu_state.seg_ds);

    /* SS */
    cpu_state.seg_ss.seg = saved_state[SMRAM_FIELD_P5_SS_SELECTOR];
    cpu_state.seg_ss.base = saved_state[SMRAM_FIELD_P5_SS_BASE];
    cpu_state.seg_ss.limit = saved_state[SMRAM_FIELD_P5_SS_LIMIT];
    cpu_state.seg_ss.access = (saved_state[SMRAM_FIELD_P5_SS_ACCESS] >> 8) & 0xff;
    /* The actual CPL (DPL of CS) is overwritten with DPL of SS. */
    cpu_state.seg_cs.access = (cpu_state.seg_cs.access & ~0x60) | (cpu_state.seg_ss.access & 0x60);
    cpu_state.seg_ss.ar_high = (saved_state[SMRAM_FIELD_P5_SS_ACCESS] >> 16) & 0xff;
    smm_seg_load(&cpu_state.seg_ss);

    /* FS */
    cpu_state.seg_fs.seg = saved_state[SMRAM_FIELD_P5_FS_SELECTOR];
    cpu_state.seg_fs.base = saved_state[SMRAM_FIELD_P5_FS_BASE];
    cpu_state.seg_fs.limit = saved_state[SMRAM_FIELD_P5_FS_LIMIT];
    cpu_state.seg_fs.access = (saved_state[SMRAM_FIELD_P5_FS_ACCESS] >> 8) & 0xff;
    cpu_state.seg_fs.ar_high = (saved_state[SMRAM_FIELD_P5_FS_ACCESS] >> 16) & 0xff;
    smm_seg_load(&cpu_state.seg_fs);

    /* GS */
    cpu_state.seg_gs.seg = saved_state[SMRAM_FIELD_P5_GS_SELECTOR];
    cpu_state.seg_gs.base = saved_state[SMRAM_FIELD_P5_GS_BASE];
    cpu_state.seg_gs.limit = saved_state[SMRAM_FIELD_P5_GS_LIMIT];
    cpu_state.seg_gs.access = (saved_state[SMRAM_FIELD_P5_GS_ACCESS] >> 8) & 0xff;
    cpu_state.seg_gs.ar_high = (saved_state[SMRAM_FIELD_P5_GS_ACCESS] >> 16) & 0xff;
    smm_seg_load(&cpu_state.seg_gs);

    if (SMM_REVISION_ID & SMM_SMBASE_RELOCATION)
	smbase = saved_state[SMRAM_FIELD_P5_SMBASE_OFFSET] & 0x00ffffff;

    /* Am486/5x86 stuff */
    if (!is_pentium) {
	cr2 = saved_state[SMRAM_FIELD_AM486_CR2];
	dr[0] = saved_state[SMRAM_FIELD_AM486_DR0];
	dr[1] = saved_state[SMRAM_FIELD_AM486_DR1];
	dr[2] = saved_state[SMRAM_FIELD_AM486_DR2];
	dr[3] = saved_state[SMRAM_FIELD_AM486_DR3];
    }
}


static void
smram_save_state_p6(uint32_t *saved_state, int in_hlt)
{
    int n = 0;

    saved_state[SMRAM_FIELD_P6_SMM_REVISION_ID] = SMM_REVISION_ID;
    saved_state[SMRAM_FIELD_P6_SMBASE_OFFSET] = smbase;

    for (n = 0; n < 8; n++)
	saved_state[SMRAM_FIELD_P6_EAX - n] = cpu_state.regs[n].l;

    if (in_hlt)
	saved_state[SMRAM_FIELD_P6_AUTOHALT_RESTART] = 1;
    else
	saved_state[SMRAM_FIELD_P6_AUTOHALT_RESTART] = 0;
    saved_state[SMRAM_FIELD_P6_EIP] = cpu_state.pc;

    saved_state[SMRAM_FIELD_P6_EFLAGS] = (cpu_state.eflags << 16) | (cpu_state.flags);

    saved_state[SMRAM_FIELD_P6_CR0] = cr0;
    saved_state[SMRAM_FIELD_P6_CR3] = cr3;
    saved_state[SMRAM_FIELD_P6_CR4] = cr4;
    saved_state[SMRAM_FIELD_P6_DR6] = dr[6];
    saved_state[SMRAM_FIELD_P6_DR7] = dr[7];
    saved_state[SMRAM_FIELD_P6_CPL] = CPL;
    saved_state[SMRAM_FIELD_P6_A20M] = !mem_a20_state;

    /* TR */
    saved_state[SMRAM_FIELD_P6_TR_SELECTOR] = tr.seg;
    saved_state[SMRAM_FIELD_P6_TR_BASE] = tr.base;
    saved_state[SMRAM_FIELD_P6_TR_LIMIT] = tr.limit;
    saved_state[SMRAM_FIELD_P6_TR_SELECTOR_AR] = (tr.ar_high << 24) | (tr.access << 16) | tr.seg;

    /* LDTR */
    saved_state[SMRAM_FIELD_P6_LDTR_SELECTOR] = ldt.seg;
    saved_state[SMRAM_FIELD_P6_LDTR_BASE] = ldt.base;
    saved_state[SMRAM_FIELD_P6_LDTR_LIMIT] = ldt.limit;
    saved_state[SMRAM_FIELD_P6_LDTR_SELECTOR_AR] = (ldt.ar_high << 24) | (ldt.access << 16) | ldt.seg;

    /* IDTR */
    saved_state[SMRAM_FIELD_P6_IDTR_BASE] = idt.base;
    saved_state[SMRAM_FIELD_P6_IDTR_LIMIT] = idt.limit;
    saved_state[SMRAM_FIELD_P6_IDTR_SELECTOR_AR] = (idt.ar_high << 24) | (idt.access << 16) | idt.seg;

    /* GDTR */
    saved_state[SMRAM_FIELD_P6_GDTR_BASE] = gdt.base;
    saved_state[SMRAM_FIELD_P6_GDTR_LIMIT] = gdt.limit;
    saved_state[SMRAM_FIELD_P6_GDTR_SELECTOR_AR] = (gdt.ar_high << 24) | (gdt.access << 16) | gdt.seg;

    /* ES */
    saved_state[SMRAM_FIELD_P6_ES_SELECTOR] = cpu_state.seg_es.seg;
    saved_state[SMRAM_FIELD_P6_ES_BASE] = cpu_state.seg_es.base;
    saved_state[SMRAM_FIELD_P6_ES_LIMIT] = cpu_state.seg_es.limit;
    saved_state[SMRAM_FIELD_P6_ES_SELECTOR_AR] =
	(cpu_state.seg_es.ar_high << 24) | (cpu_state.seg_es.access << 16) | cpu_state.seg_es.seg;

    /* CS */
    saved_state[SMRAM_FIELD_P6_CS_SELECTOR] = cpu_state.seg_cs.seg;
    saved_state[SMRAM_FIELD_P6_CS_BASE] = cpu_state.seg_cs.base;
    saved_state[SMRAM_FIELD_P6_CS_LIMIT] = cpu_state.seg_cs.limit;
    saved_state[SMRAM_FIELD_P6_CS_SELECTOR_AR] =
	(cpu_state.seg_cs.ar_high << 24) | (cpu_state.seg_cs.access << 16) | cpu_state.seg_cs.seg;

    /* DS */
    saved_state[SMRAM_FIELD_P6_DS_SELECTOR] = cpu_state.seg_ds.seg;
    saved_state[SMRAM_FIELD_P6_DS_BASE] = cpu_state.seg_ds.base;
    saved_state[SMRAM_FIELD_P6_DS_LIMIT] = cpu_state.seg_ds.limit;
    saved_state[SMRAM_FIELD_P6_DS_SELECTOR_AR] =
	(cpu_state.seg_ds.ar_high << 24) | (cpu_state.seg_ds.access << 16) | cpu_state.seg_ds.seg;

    /* SS */
    saved_state[SMRAM_FIELD_P6_SS_SELECTOR] = cpu_state.seg_ss.seg;
    saved_state[SMRAM_FIELD_P6_SS_BASE] = cpu_state.seg_ss.base;
    saved_state[SMRAM_FIELD_P6_SS_LIMIT] = cpu_state.seg_ss.limit;
    saved_state[SMRAM_FIELD_P6_SS_SELECTOR_AR] =
	(cpu_state.seg_ss.ar_high << 24) | (cpu_state.seg_ss.access << 16) | cpu_state.seg_ss.seg;

    /* FS */
    saved_state[SMRAM_FIELD_P6_FS_SELECTOR] = cpu_state.seg_fs.seg;
    saved_state[SMRAM_FIELD_P6_FS_BASE] = cpu_state.seg_fs.base;
    saved_state[SMRAM_FIELD_P6_FS_LIMIT] = cpu_state.seg_fs.limit;
    saved_state[SMRAM_FIELD_P6_FS_SELECTOR_AR] =
	(cpu_state.seg_fs.ar_high << 24) | (cpu_state.seg_fs.access << 16) | cpu_state.seg_fs.seg;

    /* GS */
    saved_state[SMRAM_FIELD_P6_GS_SELECTOR] = cpu_state.seg_gs.seg;
    saved_state[SMRAM_FIELD_P6_GS_BASE] = cpu_state.seg_gs.base;
    saved_state[SMRAM_FIELD_P6_GS_LIMIT] = cpu_state.seg_gs.limit;
    saved_state[SMRAM_FIELD_P6_GS_SELECTOR_AR] =
	(cpu_state.seg_gs.ar_high << 24) | (cpu_state.seg_gs.access << 16) | cpu_state.seg_gs.seg;
}


static void
smram_restore_state_p6(uint32_t *saved_state)
{
    int n = 0;

    for (n = 0; n < 8; n++)
	cpu_state.regs[n].l = saved_state[SMRAM_FIELD_P6_EAX - n];

    if (saved_state[SMRAM_FIELD_P6_AUTOHALT_RESTART] & 0xffff)
	cpu_state.pc = saved_state[SMRAM_FIELD_P6_EIP] - 1;
    else
	cpu_state.pc = saved_state[SMRAM_FIELD_P6_EIP];

    cpu_state.eflags = saved_state[SMRAM_FIELD_P6_EFLAGS] >> 16;
    cpu_state.flags = saved_state[SMRAM_FIELD_P6_EFLAGS] & 0xffff;

    cr0 = saved_state[SMRAM_FIELD_P6_CR0];
    cr3 = saved_state[SMRAM_FIELD_P6_CR3];
    cr4 = saved_state[SMRAM_FIELD_P6_CR4];
    dr[6] = saved_state[SMRAM_FIELD_P6_DR6];
    dr[7] = saved_state[SMRAM_FIELD_P6_DR7];

    /* TR */
    tr.seg = saved_state[SMRAM_FIELD_P6_TR_SELECTOR];
    tr.base = saved_state[SMRAM_FIELD_P6_TR_BASE];
    tr.limit = saved_state[SMRAM_FIELD_P6_TR_LIMIT];
    tr.access = (saved_state[SMRAM_FIELD_P6_TR_SELECTOR_AR] >> 16) & 0xff;
    tr.ar_high = (saved_state[SMRAM_FIELD_P6_TR_SELECTOR_AR] >> 24) & 0xff;
    smm_seg_load(&tr);

    /* LDTR */
    ldt.seg = saved_state[SMRAM_FIELD_P6_LDTR_SELECTOR];
    ldt.base = saved_state[SMRAM_FIELD_P6_LDTR_BASE];
    ldt.limit = saved_state[SMRAM_FIELD_P6_LDTR_LIMIT];
    ldt.access = (saved_state[SMRAM_FIELD_P6_LDTR_SELECTOR_AR] >> 16) & 0xff;
    ldt.ar_high = (saved_state[SMRAM_FIELD_P6_LDTR_SELECTOR_AR] >> 24) & 0xff;
    smm_seg_load(&ldt);

    /* IDTR */
    idt.base = saved_state[SMRAM_FIELD_P6_IDTR_BASE];
    idt.limit = saved_state[SMRAM_FIELD_P6_IDTR_LIMIT];
    idt.access = (saved_state[SMRAM_FIELD_P6_IDTR_SELECTOR_AR] >> 16) & 0xff;
    idt.ar_high = (saved_state[SMRAM_FIELD_P6_IDTR_SELECTOR_AR] >> 24) & 0xff;

    /* GDTR */
    gdt.base = saved_state[SMRAM_FIELD_P6_GDTR_BASE];
    gdt.limit = saved_state[SMRAM_FIELD_P6_GDTR_LIMIT];
    gdt.access = (saved_state[SMRAM_FIELD_P6_GDTR_SELECTOR_AR] >> 16) & 0xff;
    gdt.ar_high = (saved_state[SMRAM_FIELD_P6_GDTR_SELECTOR_AR] >> 24) & 0xff;

    /* ES */
    cpu_state.seg_es.seg = saved_state[SMRAM_FIELD_P6_ES_SELECTOR];
    cpu_state.seg_es.base = saved_state[SMRAM_FIELD_P6_ES_BASE];
    cpu_state.seg_es.limit = saved_state[SMRAM_FIELD_P6_ES_LIMIT];
    cpu_state.seg_es.access = (saved_state[SMRAM_FIELD_P6_ES_SELECTOR_AR] >> 16) & 0xff;
    cpu_state.seg_es.ar_high = (saved_state[SMRAM_FIELD_P6_ES_SELECTOR_AR] >> 24) & 0xff;
    smm_seg_load(&cpu_state.seg_es);

    /* CS */
    cpu_state.seg_cs.seg = saved_state[SMRAM_FIELD_P6_CS_SELECTOR];
    cpu_state.seg_cs.base = saved_state[SMRAM_FIELD_P6_CS_BASE];
    cpu_state.seg_cs.limit = saved_state[SMRAM_FIELD_P6_CS_LIMIT];
    cpu_state.seg_cs.access = (saved_state[SMRAM_FIELD_P6_CS_SELECTOR_AR] >> 16) & 0xff;
    cpu_state.seg_cs.ar_high = (saved_state[SMRAM_FIELD_P6_CS_SELECTOR_AR] >> 24) & 0xff;
    smm_seg_load(&cpu_state.seg_cs);
    cpu_state.seg_cs.access = (cpu_state.seg_cs.access & ~0x60) | ((saved_state[SMRAM_FIELD_P6_CPL] & 0x03) << 5);

    /* DS */
    cpu_state.seg_ds.seg = saved_state[SMRAM_FIELD_P6_DS_SELECTOR];
    cpu_state.seg_ds.base = saved_state[SMRAM_FIELD_P6_DS_BASE];
    cpu_state.seg_ds.limit = saved_state[SMRAM_FIELD_P6_DS_LIMIT];
    cpu_state.seg_ds.access = (saved_state[SMRAM_FIELD_P6_DS_SELECTOR_AR] >> 16) & 0xff;
    cpu_state.seg_ds.ar_high = (saved_state[SMRAM_FIELD_P6_DS_SELECTOR_AR] >> 24) & 0xff;
    smm_seg_load(&cpu_state.seg_ds);

    /* SS */
    cpu_state.seg_ss.seg = saved_state[SMRAM_FIELD_P6_SS_SELECTOR];
    cpu_state.seg_ss.base = saved_state[SMRAM_FIELD_P6_SS_BASE];
    cpu_state.seg_ss.limit = saved_state[SMRAM_FIELD_P6_SS_LIMIT];
    cpu_state.seg_ss.access = (saved_state[SMRAM_FIELD_P6_SS_SELECTOR_AR] >> 16) & 0xff;
    cpu_state.seg_ss.ar_high = (saved_state[SMRAM_FIELD_P6_SS_SELECTOR_AR] >> 24) & 0xff;
    smm_seg_load(&cpu_state.seg_ss);

    /* FS */
    cpu_state.seg_fs.seg = saved_state[SMRAM_FIELD_P6_FS_SELECTOR];
    cpu_state.seg_fs.base = saved_state[SMRAM_FIELD_P6_FS_BASE];
    cpu_state.seg_fs.limit = saved_state[SMRAM_FIELD_P6_FS_LIMIT];
    cpu_state.seg_fs.access = (saved_state[SMRAM_FIELD_P6_FS_SELECTOR_AR] >> 16) & 0xff;
    cpu_state.seg_fs.ar_high = (saved_state[SMRAM_FIELD_P6_FS_SELECTOR_AR] >> 24) & 0xff;
    smm_seg_load(&cpu_state.seg_fs);

    /* GS */
    cpu_state.seg_gs.seg = saved_state[SMRAM_FIELD_P6_GS_SELECTOR];
    cpu_state.seg_gs.base = saved_state[SMRAM_FIELD_P6_GS_BASE];
    cpu_state.seg_gs.limit = saved_state[SMRAM_FIELD_P6_GS_LIMIT];
    cpu_state.seg_gs.access = (saved_state[SMRAM_FIELD_P6_GS_SELECTOR_AR] >> 16) & 0xff;
    cpu_state.seg_gs.ar_high = (saved_state[SMRAM_FIELD_P6_GS_SELECTOR_AR] >> 24) & 0xff;
    smm_seg_load(&cpu_state.seg_gs);

    mem_a20_alt = 0;
    keyboard_at_set_a20_key(!saved_state[SMRAM_FIELD_P6_A20M]);
    mem_a20_recalc();

    if (SMM_REVISION_ID & SMM_SMBASE_RELOCATION)
	smbase = saved_state[SMRAM_FIELD_P6_SMBASE_OFFSET];
}


static void
smram_save_state_amd_k(uint32_t *saved_state, int in_hlt)
{
    int n = 0;

    saved_state[SMRAM_FIELD_AMD_K_SMM_REVISION_ID] = SMM_REVISION_ID;
    saved_state[SMRAM_FIELD_AMD_K_SMBASE_OFFSET] = smbase;

    for (n = 0; n < 8; n++)
	saved_state[SMRAM_FIELD_AMD_K_EAX - n] = cpu_state.regs[n].l;

    if (in_hlt)
	saved_state[SMRAM_FIELD_AMD_K_AUTOHALT_RESTART] = 1;
    else
	saved_state[SMRAM_FIELD_AMD_K_AUTOHALT_RESTART] = 0;

    saved_state[SMRAM_FIELD_AMD_K_EIP] = cpu_state.pc;

    saved_state[SMRAM_FIELD_AMD_K_EFLAGS] = (cpu_state.eflags << 16) | (cpu_state.flags);

    saved_state[SMRAM_FIELD_AMD_K_CR0] = cr0;
    saved_state[SMRAM_FIELD_AMD_K_CR2] = cr2;
    saved_state[SMRAM_FIELD_AMD_K_CR3] = cr3;
    saved_state[SMRAM_FIELD_AMD_K_CR4] = cr4;
    saved_state[SMRAM_FIELD_AMD_K_DR6] = dr[6];
    saved_state[SMRAM_FIELD_AMD_K_DR7] = dr[7];

    /* TR */
    saved_state[SMRAM_FIELD_AMD_K_TR_SELECTOR] = tr.seg;
    saved_state[SMRAM_FIELD_AMD_K_TR_BASE] = tr.base;
    saved_state[SMRAM_FIELD_AMD_K_TR_LIMIT] = tr.limit;
    saved_state[SMRAM_FIELD_AMD_K_TR_ACCESS] = (tr.ar_high << 16) | (tr.access << 8);

    /* LDTR */
    saved_state[SMRAM_FIELD_AMD_K_LDTR_SELECTOR] = ldt.seg;
    saved_state[SMRAM_FIELD_AMD_K_LDTR_BASE] = ldt.base;
    saved_state[SMRAM_FIELD_AMD_K_LDTR_LIMIT] = ldt.limit;
    if (!is_k6)
	saved_state[SMRAM_FIELD_AMD_K_LDTR_ACCESS] = (ldt.ar_high << 16) | (ldt.access << 8);

    /* IDTR */
    saved_state[SMRAM_FIELD_AMD_K_IDTR_BASE] = idt.base;
    saved_state[SMRAM_FIELD_AMD_K_IDTR_LIMIT] = idt.limit;

    /* GDTR */
    saved_state[SMRAM_FIELD_AMD_K_GDTR_BASE] = gdt.base;
    saved_state[SMRAM_FIELD_AMD_K_GDTR_LIMIT] = gdt.limit;

    /* ES */
    saved_state[SMRAM_FIELD_AMD_K_ES_SELECTOR] = cpu_state.seg_es.seg;
    saved_state[SMRAM_FIELD_AMD_K_ES_BASE] = cpu_state.seg_es.base;
    saved_state[SMRAM_FIELD_AMD_K_ES_LIMIT] = cpu_state.seg_es.limit;
    saved_state[SMRAM_FIELD_AMD_K_ES_ACCESS] = (cpu_state.seg_es.ar_high << 16) | (cpu_state.seg_es.access << 8);

    /* CS */
    saved_state[SMRAM_FIELD_AMD_K_CS_SELECTOR] = cpu_state.seg_cs.seg;
    saved_state[SMRAM_FIELD_AMD_K_CS_BASE] = cpu_state.seg_cs.base;
    saved_state[SMRAM_FIELD_AMD_K_CS_LIMIT] = cpu_state.seg_cs.limit;
    saved_state[SMRAM_FIELD_AMD_K_CS_ACCESS] = (cpu_state.seg_cs.ar_high << 16) | (cpu_state.seg_cs.access << 8);

    /* DS */
    saved_state[SMRAM_FIELD_AMD_K_DS_SELECTOR] = cpu_state.seg_ds.seg;
    saved_state[SMRAM_FIELD_AMD_K_DS_BASE] = cpu_state.seg_ds.base;
    saved_state[SMRAM_FIELD_AMD_K_DS_LIMIT] = cpu_state.seg_ds.limit;
    saved_state[SMRAM_FIELD_AMD_K_DS_ACCESS] = (cpu_state.seg_ds.ar_high << 16) | (cpu_state.seg_ds.access << 8);

    /* SS */
    saved_state[SMRAM_FIELD_AMD_K_SS_SELECTOR] = cpu_state.seg_ss.seg;
    saved_state[SMRAM_FIELD_AMD_K_SS_BASE] = cpu_state.seg_ss.base;
    saved_state[SMRAM_FIELD_AMD_K_SS_LIMIT] = cpu_state.seg_ss.limit;
    saved_state[SMRAM_FIELD_AMD_K_SS_ACCESS] = (cpu_state.seg_ss.ar_high << 16) | (cpu_state.seg_ss.access << 8);

    /* FS */
    saved_state[SMRAM_FIELD_AMD_K_FS_SELECTOR] = cpu_state.seg_fs.seg;
    saved_state[SMRAM_FIELD_AMD_K_FS_BASE] = cpu_state.seg_fs.base;
    saved_state[SMRAM_FIELD_AMD_K_FS_LIMIT] = cpu_state.seg_fs.limit;
    saved_state[SMRAM_FIELD_AMD_K_FS_ACCESS] = (cpu_state.seg_fs.ar_high << 16) | (cpu_state.seg_fs.access << 8);

    /* GS */
    saved_state[SMRAM_FIELD_AMD_K_GS_SELECTOR] = cpu_state.seg_gs.seg;
    saved_state[SMRAM_FIELD_AMD_K_GS_BASE] = cpu_state.seg_gs.base;
    saved_state[SMRAM_FIELD_AMD_K_GS_LIMIT] = cpu_state.seg_gs.limit;
    saved_state[SMRAM_FIELD_AMD_K_GS_ACCESS] = (cpu_state.seg_gs.ar_high << 16) | (cpu_state.seg_gs.access << 8);
}


static void
smram_restore_state_amd_k(uint32_t *saved_state)
{
    int n = 0;

    for (n = 0; n < 8; n++)
	cpu_state.regs[n].l = saved_state[SMRAM_FIELD_AMD_K_EAX - n];

    if (saved_state[SMRAM_FIELD_AMD_K_AUTOHALT_RESTART] & 0xffff)
	cpu_state.pc = saved_state[SMRAM_FIELD_AMD_K_EIP] - 1;
    else
	cpu_state.pc = saved_state[SMRAM_FIELD_AMD_K_EIP];

    cpu_state.eflags = saved_state[SMRAM_FIELD_AMD_K_EFLAGS] >> 16;
    cpu_state.flags = saved_state[SMRAM_FIELD_AMD_K_EFLAGS] & 0xffff;

    cr0 = saved_state[SMRAM_FIELD_AMD_K_CR0];
    cr2 = saved_state[SMRAM_FIELD_AMD_K_CR2];
    cr3 = saved_state[SMRAM_FIELD_AMD_K_CR3];
    cr4 = saved_state[SMRAM_FIELD_AMD_K_CR4];
    dr[6] = saved_state[SMRAM_FIELD_AMD_K_DR6];
    dr[7] = saved_state[SMRAM_FIELD_AMD_K_DR7];

    /* TR */
    tr.seg = saved_state[SMRAM_FIELD_AMD_K_TR_SELECTOR];
    tr.base = saved_state[SMRAM_FIELD_AMD_K_TR_BASE];
    tr.limit = saved_state[SMRAM_FIELD_AMD_K_TR_LIMIT];
    tr.access = (saved_state[SMRAM_FIELD_AMD_K_TR_ACCESS] >> 8) & 0xff;
    tr.ar_high = (saved_state[SMRAM_FIELD_AMD_K_TR_ACCESS] >> 16) & 0xff;
    smm_seg_load(&tr);

    /* LDTR */
    ldt.seg = saved_state[SMRAM_FIELD_AMD_K_LDTR_SELECTOR];
    ldt.base = saved_state[SMRAM_FIELD_AMD_K_LDTR_BASE];
    ldt.limit = saved_state[SMRAM_FIELD_AMD_K_LDTR_LIMIT];
    if (!is_k6) {
	ldt.access = (saved_state[SMRAM_FIELD_AMD_K_LDTR_ACCESS] >> 8) & 0xff;
	ldt.ar_high = (saved_state[SMRAM_FIELD_AMD_K_LDTR_ACCESS] >> 16) & 0xff;
    }
    smm_seg_load(&ldt);

    /* IDTR */
    idt.base = saved_state[SMRAM_FIELD_AMD_K_IDTR_BASE];
    idt.limit = saved_state[SMRAM_FIELD_AMD_K_IDTR_LIMIT];

    /* GDTR */
    gdt.base = saved_state[SMRAM_FIELD_AMD_K_GDTR_BASE];
    gdt.limit = saved_state[SMRAM_FIELD_AMD_K_GDTR_LIMIT];

    /* ES */
    cpu_state.seg_es.seg = saved_state[SMRAM_FIELD_AMD_K_ES_SELECTOR];
    cpu_state.seg_es.base = saved_state[SMRAM_FIELD_AMD_K_ES_BASE];
    cpu_state.seg_es.limit = saved_state[SMRAM_FIELD_AMD_K_ES_LIMIT];
    cpu_state.seg_es.access = (saved_state[SMRAM_FIELD_AMD_K_ES_ACCESS] >> 8) & 0xff;
    cpu_state.seg_es.ar_high = (saved_state[SMRAM_FIELD_AMD_K_ES_ACCESS] >> 16) & 0xff;
    smm_seg_load(&cpu_state.seg_es);

    /* CS */
    cpu_state.seg_cs.seg = saved_state[SMRAM_FIELD_AMD_K_CS_SELECTOR];
    cpu_state.seg_cs.base = saved_state[SMRAM_FIELD_AMD_K_CS_BASE];
    cpu_state.seg_cs.limit = saved_state[SMRAM_FIELD_AMD_K_CS_LIMIT];
    cpu_state.seg_cs.access = (saved_state[SMRAM_FIELD_AMD_K_CS_ACCESS] >> 8) & 0xff;
    cpu_state.seg_cs.ar_high = (saved_state[SMRAM_FIELD_AMD_K_CS_ACCESS] >> 16) & 0xff;
    smm_seg_load(&cpu_state.seg_cs);

    /* DS */
    cpu_state.seg_ds.seg = saved_state[SMRAM_FIELD_AMD_K_DS_SELECTOR];
    cpu_state.seg_ds.base = saved_state[SMRAM_FIELD_AMD_K_DS_BASE];
    cpu_state.seg_ds.limit = saved_state[SMRAM_FIELD_AMD_K_DS_LIMIT];
    cpu_state.seg_ds.access = (saved_state[SMRAM_FIELD_AMD_K_DS_ACCESS] >> 8) & 0xff;
    cpu_state.seg_ds.ar_high = (saved_state[SMRAM_FIELD_AMD_K_DS_ACCESS] >> 16) & 0xff;
    smm_seg_load(&cpu_state.seg_ds);

    /* SS */
    cpu_state.seg_ss.seg = saved_state[SMRAM_FIELD_AMD_K_SS_SELECTOR];
    cpu_state.seg_ss.base = saved_state[SMRAM_FIELD_AMD_K_SS_BASE];
    cpu_state.seg_ss.limit = saved_state[SMRAM_FIELD_AMD_K_SS_LIMIT];
    cpu_state.seg_ss.access = (saved_state[SMRAM_FIELD_AMD_K_SS_ACCESS] >> 8) & 0xff;
    /* The actual CPL (DPL of CS) is overwritten with DPL of SS. */
    cpu_state.seg_cs.access = (cpu_state.seg_cs.access & ~0x60) | (cpu_state.seg_ss.access & 0x60);
    cpu_state.seg_ss.ar_high = (saved_state[SMRAM_FIELD_AMD_K_SS_ACCESS] >> 16) & 0xff;
    smm_seg_load(&cpu_state.seg_ss);

    /* FS */
    cpu_state.seg_fs.seg = saved_state[SMRAM_FIELD_AMD_K_FS_SELECTOR];
    cpu_state.seg_fs.base = saved_state[SMRAM_FIELD_AMD_K_FS_BASE];
    cpu_state.seg_fs.limit = saved_state[SMRAM_FIELD_AMD_K_FS_LIMIT];
    cpu_state.seg_fs.access = (saved_state[SMRAM_FIELD_AMD_K_FS_ACCESS] >> 8) & 0xff;
    cpu_state.seg_fs.ar_high = (saved_state[SMRAM_FIELD_AMD_K_FS_ACCESS] >> 16) & 0xff;
    smm_seg_load(&cpu_state.seg_fs);

    /* GS */
    cpu_state.seg_gs.seg = saved_state[SMRAM_FIELD_AMD_K_GS_SELECTOR];
    cpu_state.seg_gs.base = saved_state[SMRAM_FIELD_AMD_K_GS_BASE];
    cpu_state.seg_gs.limit = saved_state[SMRAM_FIELD_AMD_K_GS_LIMIT];
    cpu_state.seg_gs.access = (saved_state[SMRAM_FIELD_AMD_K_GS_ACCESS] >> 8) & 0xff;
    cpu_state.seg_gs.ar_high = (saved_state[SMRAM_FIELD_AMD_K_GS_ACCESS] >> 16) & 0xff;
    smm_seg_load(&cpu_state.seg_gs);

    if (SMM_REVISION_ID & SMM_SMBASE_RELOCATION)
	smbase = saved_state[SMRAM_FIELD_AMD_K_SMBASE_OFFSET];
}


static void
smram_save_state_cyrix(uint32_t *saved_state, int in_hlt)
{
    saved_state[0] = dr[7];
    saved_state[1] = cpu_state.flags | (cpu_state.eflags << 16);
    saved_state[2] = cr0;
    saved_state[3] = cpu_state.oldpc;
    saved_state[4] = cpu_state.pc;
    saved_state[5] = CS | (CPL << 21);
    saved_state[6] = 0x00000000;
}


static void
smram_restore_state_cyrix(uint32_t *saved_state)
{
    dr[7] = saved_state[0];
    cpu_state.flags = saved_state[1] & 0xffff;
    cpu_state.eflags = saved_state[1] >> 16;
    cr0 = saved_state[2];
    cpu_state.pc = saved_state[4];
}


void
enter_smm(int in_hlt)
{
    uint32_t saved_state[SMM_SAVE_STATE_MAP_SIZE], n;
    uint32_t smram_state = smbase + 0x10000;

    /* If it's a CPU on which SMM is not supported, do nothing. */
    if (!is_am486 && !is_pentium && !is_k5 && !is_k6 && !is_p6 && !is_cxsmm)
	return;

    x386_common_log("enter_smm(): smbase = %08X\n", smbase);
    x386_common_log("CS : seg = %04X, base = %08X, limit = %08X, limit_low = %08X, limit_high = %08X, access = %02X, ar_high = %02X\n",
		    cpu_state.seg_cs.seg, cpu_state.seg_cs.base, cpu_state.seg_cs.limit, cpu_state.seg_cs.limit_low,
		    cpu_state.seg_cs.limit_high, cpu_state.seg_cs.access, cpu_state.seg_cs.ar_high);
    x386_common_log("DS : seg = %04X, base = %08X, limit = %08X, limit_low = %08X, limit_high = %08X, access = %02X, ar_high = %02X\n",
		    cpu_state.seg_ds.seg, cpu_state.seg_ds.base, cpu_state.seg_ds.limit, cpu_state.seg_ds.limit_low,
		    cpu_state.seg_ds.limit_high, cpu_state.seg_ds.access, cpu_state.seg_ds.ar_high);
    x386_common_log("ES : seg = %04X, base = %08X, limit = %08X, limit_low = %08X, limit_high = %08X, access = %02X, ar_high = %02X\n",
		    cpu_state.seg_es.seg, cpu_state.seg_es.base, cpu_state.seg_es.limit, cpu_state.seg_es.limit_low,
		    cpu_state.seg_es.limit_high, cpu_state.seg_es.access, cpu_state.seg_es.ar_high);
    x386_common_log("FS : seg = %04X, base = %08X, limit = %08X, limit_low = %08X, limit_high = %08X, access = %02X, ar_high = %02X\n",
		    cpu_state.seg_fs.seg, cpu_state.seg_fs.base, cpu_state.seg_fs.limit, cpu_state.seg_fs.limit_low,
		    cpu_state.seg_fs.limit_high, cpu_state.seg_fs.access, cpu_state.seg_fs.ar_high);
    x386_common_log("GS : seg = %04X, base = %08X, limit = %08X, limit_low = %08X, limit_high = %08X, access = %02X, ar_high = %02X\n",
		    cpu_state.seg_gs.seg, cpu_state.seg_gs.base, cpu_state.seg_gs.limit, cpu_state.seg_gs.limit_low,
		    cpu_state.seg_gs.limit_high, cpu_state.seg_gs.access, cpu_state.seg_gs.ar_high);
    x386_common_log("SS : seg = %04X, base = %08X, limit = %08X, limit_low = %08X, limit_high = %08X, access = %02X, ar_high = %02X\n",
		    cpu_state.seg_ss.seg, cpu_state.seg_ss.base, cpu_state.seg_ss.limit, cpu_state.seg_ss.limit_low,
		    cpu_state.seg_ss.limit_high, cpu_state.seg_ss.access, cpu_state.seg_ss.ar_high);
    x386_common_log("TR : seg = %04X, base = %08X, limit = %08X, limit_low = %08X, limit_high = %08X, access = %02X, ar_high = %02X\n",
		    tr.seg, tr.base, tr.limit, tr.limit_low, tr.limit_high, tr.access, tr.ar_high);
    x386_common_log("LDT: seg = %04X, base = %08X, limit = %08X, limit_low = %08X, limit_high = %08X, access = %02X, ar_high = %02X\n",
		    ldt.seg, ldt.base, ldt.limit, ldt.limit_low, ldt.limit_high, ldt.access, ldt.ar_high);
    x386_common_log("GDT: seg = %04X, base = %08X, limit = %08X, limit_low = %08X, limit_high = %08X, access = %02X, ar_high = %02X\n",
		    gdt.seg, gdt.base, gdt.limit, gdt.limit_low, gdt.limit_high, gdt.access, gdt.ar_high);
    x386_common_log("IDT: seg = %04X, base = %08X, limit = %08X, limit_low = %08X, limit_high = %08X, access = %02X, ar_high = %02X\n",
		    idt.seg, idt.base, idt.limit, idt.limit_low, idt.limit_high, idt.access, idt.ar_high);
    x386_common_log("CR0 = %08X, CR3 = %08X, CR4 = %08X, DR6 = %08X, DR7 = %08X\n", cr0, cr3, cr4, dr[6], dr[7]);
    x386_common_log("EIP = %08X, EFLAGS = %04X%04X\n", cpu_state.pc, cpu_state.eflags, cpu_state.flags);
    x386_common_log("EAX = %08X, EBX = %08X, ECX = %08X, EDX = %08X, ESI = %08X, EDI = %08X, ESP = %08X, EBP = %08X\n",
		    EAX, EBX, ECX, EDX, ESI, EDI, ESP, EBP);

    flags_rebuild();
    in_smm = 1;
    smram_backup_all();
    smram_recalc_all(0);

    if (is_cxsmm) {
	if (!(cyrix.smhr & SMHR_VALID))
		cyrix.smhr = (cyrix.arr[3].base + cyrix.arr[3].size) | SMHR_VALID;
	smram_state = cyrix.smhr & SMHR_ADDR_MASK;
    }

    memset(saved_state, 0x00, SMM_SAVE_STATE_MAP_SIZE * sizeof(uint32_t));

    if (is_cxsmm)			/* Cx6x86 */
	smram_save_state_cyrix(saved_state, in_hlt);
    else if (is_pentium || is_am486)	/* Am486 / 5x86 / Intel P5 (Pentium) */
	smram_save_state_p5(saved_state, in_hlt);
    else if (is_k5 || is_k6)		/* AMD K5 and K6 */
	smram_save_state_amd_k(saved_state, in_hlt);
    else if (is_p6)			/* Intel P6 (Pentium Pro, Pentium II, Celeron) */
	smram_save_state_p6(saved_state, in_hlt);

    cr0 &= ~0x8000000d;
    cpu_state.flags = 2;
    cpu_state.eflags = 0;

    cr4 = 0;

    dr[7] = 0x400;

    if (is_cxsmm) {
	cpu_state.pc = 0x0000;
	cpl_override = 1;
	cyrix_write_seg_descriptor(smram_state - 0x20, &cpu_state.seg_cs);
	cpl_override = 0;
	cpu_state.seg_cs.seg = (cyrix.arr[3].base >> 4);
	cpu_state.seg_cs.base = cyrix.arr[3].base;
	cpu_state.seg_cs.limit = 0xffffffff;
	cpu_state.seg_cs.access = 0x93;
	cpu_state.seg_cs.ar_high = 0x80;
	cpu_state.seg_cs.checked = 1;

	smm_seg_load(&cpu_state.seg_cs);
    } else {
	cpu_state.pc = 0x8000;
	cpu_state.seg_ds.seg = 0x00000000;
	cpu_state.seg_ds.base = 0x00000000;
	cpu_state.seg_ds.limit = 0xffffffff;
	cpu_state.seg_ds.access = 0x93;
	cpu_state.seg_ds.ar_high = 0x80;

	memcpy(&cpu_state.seg_es, &cpu_state.seg_ds, sizeof(x86seg));
	memcpy(&cpu_state.seg_ss, &cpu_state.seg_ds, sizeof(x86seg));
    	memcpy(&cpu_state.seg_fs, &cpu_state.seg_ds, sizeof(x86seg));
	memcpy(&cpu_state.seg_gs, &cpu_state.seg_ds, sizeof(x86seg));

	if (is_p6)
		cpu_state.seg_cs.seg = (smbase >> 4);
	else
		cpu_state.seg_cs.seg = 0x3000;

	/* On Pentium, CS selector in SMM is always 3000, regardless of SMBASE. */
	cpu_state.seg_cs.base = smbase;
	cpu_state.seg_cs.limit = 0xffffffff;
	cpu_state.seg_cs.access = 0x93;
	cpu_state.seg_cs.ar_high = 0x80;
	cpu_state.seg_cs.checked = 1;

	smm_seg_load(&cpu_state.seg_es);
	smm_seg_load(&cpu_state.seg_cs);
	smm_seg_load(&cpu_state.seg_ds);
	smm_seg_load(&cpu_state.seg_ss);
	smm_seg_load(&cpu_state.seg_fs);
	smm_seg_load(&cpu_state.seg_gs);
    }

    cpu_state.op32 = use32;

    cpl_override = 1;
    if (is_cxsmm) {
	writememl(0, smram_state - 0x04, saved_state[0]);
	writememl(0, smram_state - 0x08, saved_state[1]);
	writememl(0, smram_state - 0x0c, saved_state[2]);
	writememl(0, smram_state - 0x10, saved_state[3]);
	writememl(0, smram_state - 0x14, saved_state[4]);
	writememl(0, smram_state - 0x18, saved_state[5]);
	writememl(0, smram_state - 0x24, saved_state[6]);
    } else {
	for (n = 0; n < SMM_SAVE_STATE_MAP_SIZE; n++) {
		smram_state -= 4;
		writememl(0, smram_state, saved_state[n]);
	}
    }
    cpl_override = 0;

    nmi_mask = 0;

    if (smi_latched) {
	in_smm = 2;
	smi_latched = 0;
    } else
	in_smm = 1;

    smm_in_hlt = in_hlt;

    if (unmask_a20_in_smm) {
	old_rammask = rammask;
	rammask = cpu_16bitbus ? 0xFFFFFF : 0xFFFFFFFF;

	flushmmucache();
    }
    
    oldcpl = 0;

    cpu_cur_status &= ~(CPU_STATUS_PMODE | CPU_STATUS_V86);
    CPU_BLOCK_END();
}


void
enter_smm_check(int in_hlt)
{
    if (smi_line && (cpu_fast_off_flags & 0x80000000))
	cpu_fast_off_count = cpu_fast_off_val + 1;

    if ((in_smm == 0) && smi_line) {
#ifdef ENABLE_386_COMMON_LOG
	x386_common_log("SMI while not in SMM\n");
#endif
	enter_smm(in_hlt);
    } else if ((in_smm == 1) && smi_line) {
	/* Mark this so that we don't latch more than one SMI. */
#ifdef ENABLE_386_COMMON_LOG
	x386_common_log("SMI while in unlatched SMM\n");
#endif
	smi_latched = 1;
    } else if ((in_smm == 2) && smi_line) {
	/* Mark this so that we don't latch more than one SMI. */
#ifdef ENABLE_386_COMMON_LOG
	x386_common_log("SMI while in latched SMM\n");
#endif
    }

    if (smi_line)
	smi_line = 0;
}


void
leave_smm(void)
{
    uint32_t saved_state[SMM_SAVE_STATE_MAP_SIZE], n;
    uint32_t smram_state = smbase + 0x10000;

    /* If it's a CPU on which SMM is not supported (or not implemented in 86Box), do nothing. */
    if (!is_am486 && !is_pentium && !is_k5 && !is_k6 && !is_p6 && !is_cxsmm)
	return;

    memset(saved_state, 0x00, SMM_SAVE_STATE_MAP_SIZE * sizeof(uint32_t));

    cpl_override = 1;
    if (is_cxsmm) {
	smram_state = cyrix.smhr & SMHR_ADDR_MASK;
	saved_state[0] = readmeml(0, smram_state - 0x04);
	saved_state[1] = readmeml(0, smram_state - 0x08);
	saved_state[2] = readmeml(0, smram_state - 0x0c);
	saved_state[3] = readmeml(0, smram_state - 0x10);
	saved_state[4] = readmeml(0, smram_state - 0x14);
	saved_state[5] = readmeml(0, smram_state - 0x18);
	cyrix_load_seg_descriptor(smram_state - 0x20, &cpu_state.seg_cs);
	saved_state[6] = readmeml(0, smram_state - 0x24);
    } else {
	for (n = 0; n < SMM_SAVE_STATE_MAP_SIZE; n++) {
		smram_state -= 4;
		saved_state[n] = readmeml(0, smram_state);
		x386_common_log("Reading %08X from memory at %08X to array element %i\n", saved_state[n], smram_state, n);
	}
    }
    cpl_override = 0;

    if (unmask_a20_in_smm) {
	rammask = old_rammask;

	flushmmucache();
    }

    x386_common_log("New SMBASE: %08X (%08X)\n", saved_state[SMRAM_FIELD_P5_SMBASE_OFFSET] & 0x00ffffff, saved_state[66]);
    if (is_cxsmm)			/* Cx6x86 */
	smram_restore_state_cyrix(saved_state);
    else if (is_pentium || is_am486)	/* Am486 / 5x86 / Intel P5 (Pentium) */
	smram_restore_state_p5(saved_state);
    else if (is_k5 || is_k6)		/* AMD K5 and K6 */
	smram_restore_state_amd_k(saved_state);
    else if (is_p6)			/* Intel P6 (Pentium Pro, Pentium II, Celeron) */
	smram_restore_state_p6(saved_state);

    in_smm = 0;
    smram_recalc_all(1);

    cpu_386_flags_extract();
    cpu_cur_status &= ~(CPU_STATUS_PMODE | CPU_STATUS_V86);
    if (cr0 & 1) {
	cpu_cur_status |= CPU_STATUS_PMODE;
	if (cpu_state.eflags & VM_FLAG)
		cpu_cur_status |= CPU_STATUS_V86;
    }

    nmi_mask = 1;

    oldcpl = CPL;

    CPU_BLOCK_END();

    x386_common_log("CS : seg = %04X, base = %08X, limit = %08X, limit_low = %08X, limit_high = %08X, access = %02X, ar_high = %02X\n",
		    cpu_state.seg_cs.seg, cpu_state.seg_cs.base, cpu_state.seg_cs.limit, cpu_state.seg_cs.limit_low,
		    cpu_state.seg_cs.limit_high, cpu_state.seg_cs.access, cpu_state.seg_cs.ar_high);
    x386_common_log("DS : seg = %04X, base = %08X, limit = %08X, limit_low = %08X, limit_high = %08X, access = %02X, ar_high = %02X\n",
		    cpu_state.seg_ds.seg, cpu_state.seg_ds.base, cpu_state.seg_ds.limit, cpu_state.seg_ds.limit_low,
		    cpu_state.seg_ds.limit_high, cpu_state.seg_ds.access, cpu_state.seg_ds.ar_high);
    x386_common_log("ES : seg = %04X, base = %08X, limit = %08X, limit_low = %08X, limit_high = %08X, access = %02X, ar_high = %02X\n",
		    cpu_state.seg_es.seg, cpu_state.seg_es.base, cpu_state.seg_es.limit, cpu_state.seg_es.limit_low,
		    cpu_state.seg_es.limit_high, cpu_state.seg_es.access, cpu_state.seg_es.ar_high);
    x386_common_log("FS : seg = %04X, base = %08X, limit = %08X, limit_low = %08X, limit_high = %08X, access = %02X, ar_high = %02X\n",
		    cpu_state.seg_fs.seg, cpu_state.seg_fs.base, cpu_state.seg_fs.limit, cpu_state.seg_fs.limit_low,
		    cpu_state.seg_fs.limit_high, cpu_state.seg_fs.access, cpu_state.seg_fs.ar_high);
    x386_common_log("GS : seg = %04X, base = %08X, limit = %08X, limit_low = %08X, limit_high = %08X, access = %02X, ar_high = %02X\n",
		    cpu_state.seg_gs.seg, cpu_state.seg_gs.base, cpu_state.seg_gs.limit, cpu_state.seg_gs.limit_low,
		    cpu_state.seg_gs.limit_high, cpu_state.seg_gs.access, cpu_state.seg_gs.ar_high);
    x386_common_log("SS : seg = %04X, base = %08X, limit = %08X, limit_low = %08X, limit_high = %08X, access = %02X, ar_high = %02X\n",
		    cpu_state.seg_ss.seg, cpu_state.seg_ss.base, cpu_state.seg_ss.limit, cpu_state.seg_ss.limit_low,
		    cpu_state.seg_ss.limit_high, cpu_state.seg_ss.access, cpu_state.seg_ss.ar_high);
    x386_common_log("TR : seg = %04X, base = %08X, limit = %08X, limit_low = %08X, limit_high = %08X, access = %02X, ar_high = %02X\n",
		    tr.seg, tr.base, tr.limit, tr.limit_low, tr.limit_high, tr.access, tr.ar_high);
    x386_common_log("LDT: seg = %04X, base = %08X, limit = %08X, limit_low = %08X, limit_high = %08X, access = %02X, ar_high = %02X\n",
		    ldt.seg, ldt.base, ldt.limit, ldt.limit_low, ldt.limit_high, ldt.access, ldt.ar_high);
    x386_common_log("GDT: seg = %04X, base = %08X, limit = %08X, limit_low = %08X, limit_high = %08X, access = %02X, ar_high = %02X\n",
		    gdt.seg, gdt.base, gdt.limit, gdt.limit_low, gdt.limit_high, gdt.access, gdt.ar_high);
    x386_common_log("IDT: seg = %04X, base = %08X, limit = %08X, limit_low = %08X, limit_high = %08X, access = %02X, ar_high = %02X\n",
		    idt.seg, idt.base, idt.limit, idt.limit_low, idt.limit_high, idt.access, idt.ar_high);
    x386_common_log("CR0 = %08X, CR3 = %08X, CR4 = %08X, DR6 = %08X, DR7 = %08X\n", cr0, cr3, cr4, dr[6], dr[7]);
    x386_common_log("EIP = %08X, EFLAGS = %04X%04X\n", cpu_state.pc, cpu_state.eflags, cpu_state.flags);
    x386_common_log("EAX = %08X, EBX = %08X, ECX = %08X, EDX = %08X, ESI = %08X, EDI = %08X, ESP = %08X, EBP = %08X\n",
		    EAX, EBX, ECX, EDX, ESI, EDI, ESP, EBP);
    x386_common_log("leave_smm()\n");
}


void
x86_int(int num)
{
    uint32_t addr;

    flags_rebuild();
    cpu_state.pc=cpu_state.oldpc;

    if (msw&1)
	pmodeint(num,0);
    else {
	addr = (num << 2) + idt.base;

	if ((num << 2UL) + 3UL > idt.limit) {
		if (idt.limit < 35) {
			cpu_state.abrt = 0;
			softresetx86();
			cpu_set_edx();
#ifdef ENABLE_386_COMMON_LOG
			x386_common_log("Triple fault in real mode - reset\n");
#endif
		} else
			x86_int(8);
	} else {
		if (stack32) {
			writememw(ss, ESP - 2, cpu_state.flags);
			writememw(ss, ESP - 4, CS);
			writememw(ss, ESP - 6, cpu_state.pc);
			ESP -= 6;
		} else {
			writememw(ss, ((SP - 2) & 0xFFFF), cpu_state.flags);
			writememw(ss, ((SP - 4) & 0xFFFF), CS);
			writememw(ss, ((SP - 6) & 0xFFFF), cpu_state.pc);
			SP -= 6;
		}

		cpu_state.flags &= ~I_FLAG;
		cpu_state.flags &= ~T_FLAG;
#ifndef USE_NEW_DYNAREC
		oxpc = cpu_state.pc;
#endif
		cpu_state.pc = readmemw(0, addr);
		loadcs(readmemw(0, addr + 2));
	}
    }

    cycles -= 70;
    CPU_BLOCK_END();
}


void
x86_int_sw(int num)
{
    uint32_t addr;

    flags_rebuild();
    cycles -= timing_int;

    if (msw&1)
	pmodeint(num,1);
    else {
	addr = (num << 2) + idt.base;

	if ((num << 2UL) + 3UL > idt.limit)
		x86_int(0x0d);
	else {
		if (stack32) {
			writememw(ss, ESP - 2, cpu_state.flags);
			writememw(ss, ESP - 4, CS);
			writememw(ss, ESP - 6, cpu_state.pc);
			ESP -= 6;
		} else {
			writememw(ss, ((SP - 2) & 0xFFFF), cpu_state.flags);
			writememw(ss, ((SP - 4) & 0xFFFF), CS);
			writememw(ss, ((SP - 6) & 0xFFFF), cpu_state.pc);
			SP -= 6;
		}

		cpu_state.flags &= ~I_FLAG;
		cpu_state.flags &= ~T_FLAG;
#ifndef USE_NEW_DYNAREC
		oxpc = cpu_state.pc;
#endif
		cpu_state.pc = readmemw(0, addr);
		loadcs(readmemw(0, addr + 2));
		cycles -= timing_int_rm;
	}
    }

    trap = 0;
    CPU_BLOCK_END();
}


int
x86_int_sw_rm(int num)
{
    uint32_t addr;
    uint16_t new_pc, new_cs;

    flags_rebuild();
    cycles -= timing_int;

    addr = num << 2;
    new_pc = readmemw(0, addr);
    new_cs = readmemw(0, addr + 2);

    if (cpu_state.abrt)
	return 1;

    writememw(ss, ((SP - 2) & 0xFFFF), cpu_state.flags);

    if (cpu_state.abrt)
	return 1;

    writememw(ss, ((SP - 4) & 0xFFFF), CS);
    writememw(ss, ((SP - 6) & 0xFFFF), cpu_state.pc);

    if (cpu_state.abrt)
	return 1;

    SP -= 6;

    cpu_state.eflags &= ~VIF_FLAG;
    cpu_state.flags &= ~T_FLAG;
    cpu_state.pc = new_pc;
    loadcs(new_cs);
#ifndef USE_NEW_DYNAREC
    oxpc = cpu_state.pc;
#endif

    cycles -= timing_int_rm;
    trap = 0;
    CPU_BLOCK_END();

    return 0;
}


void
x86illegal()
{
    x86_int(6);
}


int
checkio(uint32_t port)
{
    uint16_t t;
    uint8_t d;

    cpl_override = 1;
    t = readmemw(tr.base, 0x66);
    cpl_override = 0;

    if (cpu_state.abrt)
	return 0;

    if ((t + (port >> 3UL)) > tr.limit)
	return 1;

    cpl_override = 1;
    d = readmembl(tr.base + t + (port >> 3));
    cpl_override = 0;
    return d & (1 << (port & 7));
}


#define divexcp() { \
	x386_common_log("Divide exception at %04X(%06X):%04X\n",CS,cs,cpu_state.pc); \
	x86_int(0); \
}


int
divl(uint32_t val)
{
    uint64_t num, quo;
    uint32_t rem, quo32;

    if (val == 0) {
	divexcp();
	return 1;
    }

    num = (((uint64_t) EDX) << 32) | EAX;
    quo = num / val;
    rem = num % val;
    quo32=(uint32_t)(quo&0xFFFFFFFF);

    if (quo != (uint64_t) quo32) {
	divexcp();
	return 1;
    }

    EDX = rem;
    EAX = quo32;

    return 0;
}


int
idivl(int32_t val)
{
    int64_t num, quo;
    int32_t rem, quo32;

    if (val == 0)  {       
	divexcp();
	return 1;
    }

    num = (((uint64_t) EDX) << 32) | EAX;
    quo = num / val;
    rem = num % val;
    quo32 = (int32_t) (quo & 0xFFFFFFFF);

    if (quo != (int64_t) quo32) {
	divexcp();
	return 1;
    }

    EDX = rem;
    EAX = quo32;

    return 0;
}


void
cpu_386_flags_extract()
{
    flags_extract();
}


void
cpu_386_flags_rebuild()
{
    flags_rebuild();
}


int
sysenter(uint32_t fetchdat)
{
#ifdef ENABLE_386_COMMON_LOG
    x386_common_log("SYSENTER called\n");
#endif

    if (!(msw & 1)) {
#ifdef ENABLE_386_COMMON_LOG
	x386_common_log("SYSENTER: CPU not in protected mode");
#endif
	x86gpf("SYSENTER: CPU not in protected mode", 0);
	return cpu_state.abrt;
    }

    if (!(msr.sysenter_cs & 0xFFF8)) {
#ifdef ENABLE_386_COMMON_LOG
	x386_common_log("SYSENTER: CS MSR is zero");
#endif
	x86gpf("SYSENTER: CS MSR is zero", 0);
	return cpu_state.abrt;
    }

#ifdef ENABLE_386_COMMON_LOG
    x386_common_log("SYSENTER started:\n");
    x386_common_log("    CS %04X/%i: b=%08X l=%08X (%08X-%08X) a=%02X%02X; EIP=%08X\n", cpu_state.seg_cs.seg, !!cpu_state.seg_cs.checked, cpu_state.seg_cs.base, cpu_state.seg_cs.limit, cpu_state.seg_cs.limit_low, cpu_state.seg_cs.limit_high, cpu_state.seg_cs.ar_high, cpu_state.seg_cs.access, cpu_state.pc);
    x386_common_log("    SS %04X/%i: b=%08X l=%08X (%08X-%08X) a=%02X%02X; ESP=%08X\n", cpu_state.seg_ss.seg, !!cpu_state.seg_ss.checked, cpu_state.seg_ss.base, cpu_state.seg_ss.limit, cpu_state.seg_ss.limit_low, cpu_state.seg_ss.limit_high, cpu_state.seg_ss.ar_high, cpu_state.seg_ss.access, ESP);
    x386_common_log("    Misc.  : MSR (CS/ESP/EIP)=%04X/%08X/%08X pccache=%08X/%08X\n", msr.sysenter_cs, msr.sysenter_esp, msr.sysenter_eip, pccache, pccache2);
    x386_common_log("             EFLAGS=%04X%04X/%i 32=%i/%i ECX=%08X EDX=%08X abrt=%02X\n", cpu_state.eflags, cpu_state.flags, !!trap, !!use32, !!stack32, ECX, EDX, cpu_state.abrt);
#endif

    /* Set VM, RF, and IF to 0. */
    cpu_state.eflags &= ~(RF_FLAG | VM_FLAG);
    cpu_state.flags &= ~I_FLAG;

#ifndef USE_NEW_DYNAREC
    oldcs = CS;
#endif
    cpu_state.oldpc = cpu_state.pc;
    ESP = msr.sysenter_esp;
    cpu_state.pc = msr.sysenter_eip;

    cpu_state.seg_cs.seg = (msr.sysenter_cs & 0xfffc);
    cpu_state.seg_cs.base = 0;
    cpu_state.seg_cs.limit_low = 0;
    cpu_state.seg_cs.limit = 0xffffffff;
    cpu_state.seg_cs.limit_high = 0xffffffff;
    cpu_state.seg_cs.access = 0x9b;
    cpu_state.seg_cs.ar_high = 0xcf;
    cpu_state.seg_cs.checked = 1;
    oldcpl = 0;

    cpu_state.seg_ss.seg = ((msr.sysenter_cs + 8) & 0xfffc);
    cpu_state.seg_ss.base = 0;
    cpu_state.seg_ss.limit_low = 0;
    cpu_state.seg_ss.limit = 0xffffffff;
    cpu_state.seg_ss.limit_high = 0xffffffff;
    cpu_state.seg_ss.access = 0x93;
    cpu_state.seg_ss.ar_high = 0xcf;
    cpu_state.seg_ss.checked = 1;
#ifdef USE_DYNAREC
    codegen_flat_ss = 0;
#endif

    cpu_cur_status &= ~(CPU_STATUS_NOTFLATSS | CPU_STATUS_V86);
    cpu_cur_status |= (CPU_STATUS_USE32 | CPU_STATUS_STACK32/* | CPU_STATUS_PMODE*/);
    set_use32(1);
    set_stack32(1);

    in_sys = 1;

#ifdef ENABLE_386_COMMON_LOG
    x386_common_log("SYSENTER completed:\n");
    x386_common_log("    CS %04X/%i: b=%08X l=%08X (%08X-%08X) a=%02X%02X; EIP=%08X\n", cpu_state.seg_cs.seg, !!cpu_state.seg_cs.checked, cpu_state.seg_cs.base, cpu_state.seg_cs.limit, cpu_state.seg_cs.limit_low, cpu_state.seg_cs.limit_high, cpu_state.seg_cs.ar_high, cpu_state.seg_cs.access, cpu_state.pc);
    x386_common_log("    SS %04X/%i: b=%08X l=%08X (%08X-%08X) a=%02X%02X; ESP=%08X\n", cpu_state.seg_ss.seg, !!cpu_state.seg_ss.checked, cpu_state.seg_ss.base, cpu_state.seg_ss.limit, cpu_state.seg_ss.limit_low, cpu_state.seg_ss.limit_high, cpu_state.seg_ss.ar_high, cpu_state.seg_ss.access, ESP);
    x386_common_log("    Misc.  : MSR (CS/ESP/EIP)=%04X/%08X/%08X pccache=%08X/%08X\n", msr.sysenter_cs, msr.sysenter_esp, msr.sysenter_eip, pccache, pccache2);
    x386_common_log("             EFLAGS=%04X%04X/%i 32=%i/%i ECX=%08X EDX=%08X abrt=%02X\n", cpu_state.eflags, cpu_state.flags, !!trap, !!use32, !!stack32, ECX, EDX, cpu_state.abrt);
#endif

    return 1;
}


int
sysexit(uint32_t fetchdat)
{
#ifdef ENABLE_386_COMMON_LOG
    x386_common_log("SYSEXIT called\n");
#endif

    if (!(msr.sysenter_cs & 0xFFF8)) {
#ifdef ENABLE_386_COMMON_LOG
	x386_common_log("SYSEXIT: CS MSR is zero");
#endif
	x86gpf("SYSEXIT: CS MSR is zero", 0);
	return cpu_state.abrt;
    }

    if (!(msw & 1)) {
#ifdef ENABLE_386_COMMON_LOG
	x386_common_log("SYSEXIT: CPU not in protected mode");
#endif
	x86gpf("SYSEXIT: CPU not in protected mode", 0);
	return cpu_state.abrt;
    }

    if (CPL) {
#ifdef ENABLE_386_COMMON_LOG
	x386_common_log("SYSEXIT: CPL not 0");
#endif
	x86gpf("SYSEXIT: CPL not 0", 0);
	return cpu_state.abrt;
    }

#ifdef ENABLE_386_COMMON_LOG
    x386_common_log("SYSEXIT start:\n");
    x386_common_log("    CS %04X/%i: b=%08X l=%08X (%08X-%08X) a=%02X%02X; EIP=%08X\n", cpu_state.seg_cs.seg, !!cpu_state.seg_cs.checked, cpu_state.seg_cs.base, cpu_state.seg_cs.limit, cpu_state.seg_cs.limit_low, cpu_state.seg_cs.limit_high, cpu_state.seg_cs.ar_high, cpu_state.seg_cs.access, cpu_state.pc);
    x386_common_log("    SS %04X/%i: b=%08X l=%08X (%08X-%08X) a=%02X%02X; ESP=%08X\n", cpu_state.seg_ss.seg, !!cpu_state.seg_ss.checked, cpu_state.seg_ss.base, cpu_state.seg_ss.limit, cpu_state.seg_ss.limit_low, cpu_state.seg_ss.limit_high, cpu_state.seg_ss.ar_high, cpu_state.seg_ss.access, ESP);
    x386_common_log("    Misc.  : MSR (CS/ESP/EIP)=%04X/%08X/%08X pccache=%08X/%08X\n", msr.sysenter_cs, msr.sysenter_esp, msr.sysenter_eip, pccache, pccache2);
    x386_common_log("             EFLAGS=%04X%04X/%i 32=%i/%i ECX=%08X EDX=%08X abrt=%02X\n", cpu_state.eflags, cpu_state.flags, !!trap, !!use32, !!stack32, ECX, EDX, cpu_state.abrt);
#endif

#ifndef USE_NEW_DYNAREC
    oldcs = CS;
#endif
    cpu_state.oldpc = cpu_state.pc;
    ESP = ECX;
    cpu_state.pc = EDX;

    cpu_state.seg_cs.seg = (((msr.sysenter_cs + 16) & 0xfffc) | 3);
    cpu_state.seg_cs.base = 0;
    cpu_state.seg_cs.limit_low = 0;
    cpu_state.seg_cs.limit = 0xffffffff;
    cpu_state.seg_cs.limit_high = 0xffffffff;
    cpu_state.seg_cs.access = 0xfb;
    cpu_state.seg_cs.ar_high = 0xcf;
    cpu_state.seg_cs.checked = 1;
    oldcpl = 3;

    cpu_state.seg_ss.seg = (((msr.sysenter_cs + 24) & 0xfffc) | 3);
    cpu_state.seg_ss.base = 0;
    cpu_state.seg_ss.limit_low = 0;
    cpu_state.seg_ss.limit = 0xffffffff;
    cpu_state.seg_ss.limit_high = 0xffffffff;
    cpu_state.seg_ss.access = 0xf3;
    cpu_state.seg_ss.ar_high = 0xcf;
    cpu_state.seg_ss.checked = 1;
#ifdef USE_DYNAREC
    codegen_flat_ss = 0;
#endif

    cpu_cur_status &= ~(CPU_STATUS_NOTFLATSS/* | CPU_STATUS_V86*/);
    cpu_cur_status |= (CPU_STATUS_USE32 | CPU_STATUS_STACK32 | CPU_STATUS_PMODE);
    flushmmucache_cr3();
    set_use32(1);
    set_stack32(1);

    in_sys = 0;

#ifdef ENABLE_386_COMMON_LOG
    x386_common_log("SYSEXIT completed:\n");
    x386_common_log("    CS %04X/%i: b=%08X l=%08X (%08X-%08X) a=%02X%02X; EIP=%08X\n", cpu_state.seg_cs.seg, !!cpu_state.seg_cs.checked, cpu_state.seg_cs.base, cpu_state.seg_cs.limit, cpu_state.seg_cs.limit_low, cpu_state.seg_cs.limit_high, cpu_state.seg_cs.ar_high, cpu_state.seg_cs.access, cpu_state.pc);
    x386_common_log("    SS %04X/%i: b=%08X l=%08X (%08X-%08X) a=%02X%02X; ESP=%08X\n", cpu_state.seg_ss.seg, !!cpu_state.seg_ss.checked, cpu_state.seg_ss.base, cpu_state.seg_ss.limit, cpu_state.seg_ss.limit_low, cpu_state.seg_ss.limit_high, cpu_state.seg_ss.ar_high, cpu_state.seg_ss.access, ESP);
    x386_common_log("    Misc.  : MSR (CS/ESP/EIP)=%04X/%08X/%08X pccache=%08X/%08X\n", msr.sysenter_cs, msr.sysenter_esp, msr.sysenter_eip, pccache, pccache2);
    x386_common_log("             EFLAGS=%04X%04X/%i 32=%i/%i ECX=%08X EDX=%08X abrt=%02X\n", cpu_state.eflags, cpu_state.flags, !!trap, !!use32, !!stack32, ECX, EDX, cpu_state.abrt);
#endif

    return 1;
}


int
syscall_op(uint32_t fetchdat)
{
#ifdef ENABLE_386_COMMON_LOG
    x386_common_log("SYSCALL called\n");
#endif

    /* Let's do this by the AMD spec. */
    /* Set VM and IF to 0. */
    cpu_state.eflags &= ~VM_FLAG;
    cpu_state.flags &= ~I_FLAG;

#ifndef USE_NEW_DYNAREC
    oldcs = CS;
#endif
    cpu_state.oldpc = cpu_state.pc;
    ECX = cpu_state.pc;

    /* CS */
    CS = AMD_SYSCALL_SB & 0xfffc;
    cpu_state.seg_cs.base = 0;
    cpu_state.seg_cs.limit_low = 0;
    cpu_state.seg_cs.limit = 0xffffffff;
    cpu_state.seg_cs.limit_high = 0xffffffff;
    cpu_state.seg_cs.access = 0x9b;
    cpu_state.seg_cs.ar_high = 0xcf;
    cpu_state.seg_cs.checked = 1;
    oldcpl = 0;

    /* SS */
    SS = (AMD_SYSCALL_SB + 8) & 0xfffc;
    cpu_state.seg_ss.base = 0;
    cpu_state.seg_ss.limit_low = 0;
    cpu_state.seg_ss.limit = 0xffffffff;
    cpu_state.seg_ss.limit_high = 0xffffffff;
    cpu_state.seg_ss.access = 0x93;
    cpu_state.seg_ss.ar_high = 0xcf;
    cpu_state.seg_ss.checked = 1;
#ifdef USE_DYNAREC
    codegen_flat_ss = 0;
#endif

    cpu_cur_status &= ~(CPU_STATUS_NOTFLATSS | CPU_STATUS_V86);
    cpu_cur_status |= (CPU_STATUS_USE32 | CPU_STATUS_STACK32 | CPU_STATUS_PMODE);
    set_use32(1);
    set_stack32(1);

    in_sys = 1;

    return 1;
}


int
sysret(uint32_t fetchdat)
{
#ifdef ENABLE_386_COMMON_LOG
    x386_common_log("SYSRET called\n");
#endif

    if (CPL) {
#ifdef ENABLE_386_COMMON_LOG
	x386_common_log("SYSRET: CPL not 0");
#endif
	x86gpf("SYSRET: CPL not 0", 0);
	return cpu_state.abrt;
    }

    cpu_state.flags |= I_FLAG;
    /* First instruction after SYSRET will always execute, regardless of whether
       there is a pending interrupt, following the STI logic */
    cpu_end_block_after_ins = 2;

#ifndef USE_NEW_DYNAREC
    oldcs = CS;
#endif
    cpu_state.oldpc = cpu_state.pc;
    cpu_state.pc = ECX;

    /* CS */
    CS = (AMD_SYSRET_SB & 0xfffc) | 3;
    cpu_state.seg_cs.base = 0;
    cpu_state.seg_cs.limit_low = 0;
    cpu_state.seg_cs.limit = 0xffffffff;
    cpu_state.seg_cs.limit_high = 0xffffffff;
    cpu_state.seg_cs.access = 0xfb;
    cpu_state.seg_cs.ar_high = 0xcf;
    cpu_state.seg_cs.checked = 1;
    oldcpl = 3;

    /* SS */
    SS = ((AMD_SYSRET_SB + 8) & 0xfffc) | 3;
    cpu_state.seg_ss.base = 0;
    cpu_state.seg_ss.limit_low = 0;
    cpu_state.seg_ss.limit = 0xffffffff;
    cpu_state.seg_ss.limit_high = 0xffffffff;
    cpu_state.seg_ss.access = 0xf3;
    cpu_state.seg_cs.ar_high = 0xcf;
    cpu_state.seg_ss.checked = 1;
#ifdef USE_DYNAREC
    codegen_flat_ss = 0;
#endif

    cpu_cur_status &= ~(CPU_STATUS_NOTFLATSS/* | CPU_STATUS_V86*/);
    cpu_cur_status |= (CPU_STATUS_USE32 | CPU_STATUS_STACK32 | CPU_STATUS_PMODE);
    flushmmucache_cr3();
    set_use32(1);
    set_stack32(1);

    in_sys = 0;

    return 1;
}


#ifndef USE_DYNAREC
/* This is for compatibility with new x87 code. */
void codegen_set_rounding_mode(int mode)
{
	/* cpu_state.new_npxc = (cpu_state.old_npxc & ~0xc00) | (cpu_state.npxc & 0xc00); */
	cpu_state.new_npxc = (cpu_state.old_npxc & ~0xc00) | (mode << 10);
}
#endif
