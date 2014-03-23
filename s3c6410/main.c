#include "serial.c"
#include "u.h"
#include "../port/lib.h"
#include "dat.h"
#include "mem.h"
void 
main() {
	char * s = "Hello world! from s3c6410, Inferno-os\nCompiled by 5c\nModified by dd\n";
	serial_puts(s);
	S3C64XX_UART *	uart0;
	uart0=S3C64XX_GetBase_UART(UART_NR);
	vu_long temp = uart0->UBRDIV;
	while (temp)
	{
		serial_putc((temp&1 ? '1' : '0'));
		temp = temp >> 1;
	}

	serial_putc('\n');
	vu_long temp1 = uart0->UDIVSLOT;
	while (temp1)
	{
		serial_putc((temp1&1 ? '1' : '0'));
		temp1 = temp1 >> 1;
	}

	while (1);
}


Conf conf;
Mach *m = (Mach*)MACHADDR;
Proc *up = 0;

#include "../port/uart.h"
PhysUart* physuart[1];

int		waserror(void) { return 0; }
int		splhi(void) { return 0; }
void	splx(int) { return; }
int		spllo(void) { return 0; } 
void	splxpc(int) { return; }
int		islo(void) { return 0; }
int		setlabel(Label*) { return 0; }
void	gotolabel(Label*) { return; }
ulong	getcallerpc(void*) { return 0; }
int		segflush(void*, ulong) { return 0; }
void	idlehands(void) { return; }
void 	kprocchild(Proc *p, void (*func)(void*), void *arg) { return; }
ulong	_tas(ulong*) { return 0; }
ulong	_div(ulong*) { return 0; }
ulong	_divu(ulong*) { return 0; }
ulong	_mod(ulong*) { return 0; }
ulong	_modu(ulong*) { return 0; }

void	setpanic(void) { return; }
void	dumpstack(void) { return; }
void	exit(int) { return; }
void	reboot(void) { return; }
void	halt(void) { return; }

Timer*	addclock0link(void (*)(void), int) { return 0; }
void	clockcheck(void) { return; }

void	fpinit(void) {}
void	FPsave(void*) {}
void	FPrestore(void*) {}
