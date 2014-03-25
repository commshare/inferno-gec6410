#include "armv6.h"
#include "mem.h"
TEXT _start(SB), 1, $-4
	MOVW    $setR12(SB), R12	/* static base (SB) */
	MOVW    $(PsrDirq|PsrDfiq|PsrMsvc), R1  /* SVC mode: interrupts disabled */
	MOVW	R1, CPSR
	MOVW    $(MACHADDR+BY2PG-4),SP /*! stack; 4 bytes for link */
	/* enable caches, mmu, and high vectors */
	MOVW    $1, R1
	MCR     CpSC, 0, R1, C(CpSPM), C(CpSPMperf), CpSPMctl /* counter */
	MRC     CpSC, 0, R0, C(CpCONTROL), C(0), CpMainctl /*caches*/
	ORR     $(CpChv|CpCdcache|CpCicache|CpCmmu), R0
	MCR     CpSC, 0, R0, C(CpCONTROL), C(0), CpMainctl
	ISB
	BL      ,main(SB)
dead:
	B       dead
	B       ,0(PC)
	BL      _div(SB)    /* hack to load _div, etc. */
