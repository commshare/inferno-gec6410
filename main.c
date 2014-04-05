#include "./serial.h"
#include "./u.h"
#include "../port/lib.h"
#include "dat.h"
#include "fns.h"
#include "mem.h"
#include "armv6.h"
#include "s3c6410.h"
Mach *m = (Mach*)MACHADDR;
extern int main_pool_pcnt;
extern int heap_pool_pcnt;
extern int image_pool_pcnt;
Conf conf;
void	testofsp(int i)
{
	serial_puts("TEST, PC: ");
	serial_addr((void *)getpc(), 0);
	serial_puts("SP: ");
	serial_addr((void *)getsp(), 1);
	serial_addr((void *)checksp(), 1);
	if (i)
		testofsp(i - 1);
	return;
}
static void
poolsizeinit(void)
{
	ulong nb;
	nb = conf.npage*BY2PG;
	serial_puts("conf.npage : ");
	serial_addr((void *)conf.npage, 1);
	serial_puts("nb : ");
	serial_addr((void *)nb, 1);
	poolsize(mainmem, (nb >> 3) * 3, 0);
	serial_puts("mainmem\n");
	poolsize(heapmem, (nb >> 3) * 2, 0);
	serial_puts("heapmem\n");
	poolsize(imagmem, (nb >> 3) * 3, 1);
	serial_puts("imagemem\n");
}
void
confinit(void)
{
	ulong base;
	conf.topofmem = 256*MB + KZERO;

	base = PGROUND((ulong)end);
	conf.base0 = base;
	serial_puts("base : ");
	serial_addr((void *)base, 1);

	conf.npage1 = 0;
	conf.npage0 = (conf.topofmem - base)/BY2PG;
	conf.npage = conf.npage0 + conf.npage1;
	conf.ialloc = (((conf.npage*(main_pool_pcnt))/100)/2)*BY2PG;
	conf.nproc = 100 + ((conf.npage*BY2PG)/MB)*5;
	conf.nmach = 1;

	active.machs = 1;
	active.exiting = 0;
	serial_puts("hello from confinit\n");
/*
	print("Conf: top=%lud, npage0=%lud, ialloc=%lud, nproc=%lud\n",
			conf.topofmem, conf.npage0,
			conf.ialloc, conf.nproc);
*/

}
void 
main() {
	serial_puts("Now, PC: ");
	serial_addr((void *)getpc(), 0);
	serial_puts("SP: ");
	serial_addr((void *)getsp(), 1);
	serial_puts("Entered main() at ");
	serial_addr(&main, 1);
	serial_puts("SP: ");
	serial_addr((void *)getsp(), 1);
	serial_puts("Hello world! from s3c6410, Inferno-os\nCompiled by 5c\nModified by dd\n");
	serial_puts("SP: ");
	serial_addr((void *)getsp(), 1);
	serial_puts("Clearing Mach:...\n");
	memset(m, 0, sizeof(Mach));
	serial_addr((char *)m, 0);
	serial_addr((char *)(m + 1), 1);
	serial_puts("Clearing edata: \n");
	memset(edata, 0, end-edata);
	serial_addr((char *)&edata, 0);
	serial_addr((char *)&end, 1);
	serial_addr((char *)&etext, 1);
	conf.nmach = 1;
	/* serwrite = &pl011_serputs; WHAT'S THIS????*/
	//serwrite = &serial_putsi;  //This does NOT work for some unkonwn reason
	confinit();
	testofsp(3);
	xinit();
	poolinit();
	poolsizeinit();
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
