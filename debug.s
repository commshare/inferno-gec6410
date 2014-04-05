#include "mem.h"
#include "armv6.h"

TEXT getTTB(SB), $-4
	MRC		CpSC, 0, R0, C(CpTTB), C(0)	
	RET
	
