#include "mem.h"
#include "armv6.h"

TEXT getTTB(SB), $-4
	MRC		CpSC, 0, R0, C(CpTTB), C(0)	
	RET
	
TEXT try_undefined(SB), $-4
	WORD	$0xE6BAD010
	BL		,serial_checkpoint(SB)
	RET
	
	