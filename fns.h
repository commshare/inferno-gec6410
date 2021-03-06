
#define KADDR(p)    ((void *)p)
#define PADDR(p)    ((ulong)p)
//#define	coherence()		/* nothing needed for uniprocessor */
#define procsave(p)
#define waserror()	(up->nerrlab++, setlabel(&up->errlab[up->nerrlab-1]))
#define kmapinval()
#define HOWMANY(x, y)	(((x)+((y)-1))/(y))
//int		waserror(void);
void    (*screenputs)(char*, int);
void	(*serwrite)(char*, int);
#include "../port/portfns.h"

ulong	getsp(void);
ulong   getsc(void);
ulong	getpc(void);
ulong	getcpsr(void);
ulong	getspsr(void);
ulong	getcpuid(void);
ulong	getcallerpc(void*);
u32int	lcycles(void);
int	splfhi(void);
int	tas(void *);

void	idlehands(void);
void	coherence(void);
void	clockinit(void);
void	trapinit(void);
char *	trapname(int psr);
int	isvalid_va(void *v);
int	isvalid_wa(void *v);
void	setr13(int, void*);
void	vectors(void);
void	vtable(void);
void	setpanic(void);
void	dumpregs(Ureg*);
void	dumparound(uint addr);
int	(*breakhandler)(Ureg*, Proc*);
void	irqenable(int, void (*)(Ureg*, void*), void*);
#define intrenable(i, f, a, b, n) irqenable((i), (f), (a))

void	cachedwbinv(void);
void	cachedwbse(void*, int);
void	cachedwbinvse(void*, int);
void	cacheiinv(void);
void	cacheuwbinv(void);

void	mmuinit1(void);
void	mmuinvalidateaddr(u32int);
void	screeninit(void);
void*   fbinit(int, int*, int*, int*);
int	fbblank(int blank);
void	setpower(int, int);
void	clockcheck(void);
void	armtimerset(int);
void	links(void);
int	fpiarm(Ureg*);

char*	getconf(char*);
char *	getethermac(void);
