#include "serial.c"
#include "./u.h"
#include "../port/lib.h"
#include "dat.h"
#include "fns.h"
#include "mem.h"
#include "armv6.h"
#include "s3c6410.h"
Mach *m = (Mach*)MACHADDR;
extern int main_pool_pcnt;
Conf conf;
static void
poolsizeinit(void)
{
	ulong nb;
	nb = conf.npage*BY2PG;
	poolsize(mainmem, (nb*main_pool_pcnt)/100, 0);
	poolsize(heapmem, (nb*heap_pool_pcnt)/100, 0);
	poolsize(imagmem, (nb*image_pool_pcnt)/100, 1);
}
void
confinit(void)
{
	ulong base;
	conf.topofmem = 256*MB;

	base = PGROUND((ulong)end);
	conf.base0 = base;

	conf.npage1 = 0;
	conf.npage0 = (conf.topofmem - base)/BY2PG;
	conf.npage = conf.npage0 + conf.npage1;
	conf.ialloc = (((conf.npage*(main_pool_pcnt))/100)/2)*BY2PG;

	conf.nproc = 100 + ((conf.npage*BY2PG)/MB)*5;
	conf.nmach = 1;

	active.machs = 1;
	active.exiting = 0;

}
void 
main() {
	/*serial_addr((void *)getpc());*/
	ulong pc;
	pc = getpc();
	serial_addr((void *)pc);
	serial_puts("Hello world! from s3c6410, Inferno-os\nCompiled by 5c\nModified by dd\n");
	serial_puts("Entered main() at \n");
	serial_addr(&main);
	serial_puts("\nClearing Mach:...\n");
	memset(m, 0, sizeof(Mach));
	serial_addr((char *)m);
	serial_putc('-');
	serial_addr((char *)(m + 1));
	serial_puts("\n");
	serial_puts("Clearing edata: \n");
	memset(edata, 0, end-edata);
	serial_addr((char *)&edata);
	serial_puts("\n");
	serial_addr((char *)&end);
	serial_puts("\n");
	conf.nmach = 1;
	/* serwrite = &pl011_serputs; WHAT'S THIS????*/
	serwrite = &serial_putsi;	//IT DOES NOT WORK FOR SOME UNKNOWN REASON
	confinit();
	//xinit();
	//poolinit();
	//poolsizeinit();
	serial_puts("Infinite Loop\n");
	while (1);
}



Proc *up = 0;

#include "../port/uart.h"
PhysUart* physuart[1];
int		waserror(void) { return 0; }
//int		splhi(void) { return 0; }
//void	splx(int) { return; }
//int		spllo(void) { return 0; } 
//void	splxpc(int) { return; }
//int		islo(void) { return 0; }
//int		setlabel(Label*) { return 0; }
//void	gotolabel(Label*) { return; }
//ulong	getcallerpc(void*) { return 0; }
int		segflush(void*, ulong) { return 0; }
void	idlehands(void) { return; }
void 	kprocchild(Proc *p, void (*func)(void*), void *arg) { return; }
//ulong	_tas(ulong*) { return 0; }
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
