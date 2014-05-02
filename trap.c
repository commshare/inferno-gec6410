#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "ureg.h"
#include "armv6.h"
#include "../port/error.h"

static char *trapnames[PsrMask+1] = {
	[ PsrMusr ] "user mode",
	[ PsrMfiq ] "fiq interrupt",
	[ PsrMirq ] "irq interrupt",
	[ PsrMsvc ] "svc/swi exception",
	[ PsrMabt ] "prefetch abort/data abort",
	[ PsrMabt+1 ] "data abort",
	[ PsrMund ] "undefined instruction",
	[ PsrMsys ] "sys trap",
};

char *
trapname(int psr)
{
	char *s;

	s = trapnames[psr & PsrMask];
	if(s == nil)
		s = "Undefined trap";
	return s;
}

int isvalid_wa(void *v) { return (ulong)v < conf.topofmem && !((ulong)v & 3); }
int isvalid_va(void *v) { return (ulong)v < conf.topofmem; }

enum { Nvec = 8, Fiqenable = 1<<7 }; /* # of vectors */
typedef struct Vpage0 {
	void    (*vectors[Nvec])(void);
	u32int  vtable[Nvec];
} Vpage0;

typedef struct Intregs Intregs;
typedef struct Vctl Vctl;

struct Vctl {
	Vctl	*next;
	int		vec;
	u32int	*reg;
	u32int	*type;
	u32int	mask;
	void	(*f)(Ureg*, void*);
	void	*a;
};
static Vctl *virq, *vfiq;

void
intrsoff(void)
{
	Intregs *ip;
	u32int disable;
	disable = 0xffffffff;
	
	ip = (Intregs*)INTREGS0;
	ip->INTENCLEAR=disable;
	
	ip = (Intregs*)INTREGS1;
	ip->INTENCLEAR=disable;
}

void
trapinit(void)
{
	Vpage0 *vpage0;

	intrsoff();
	print("Interrupt Off\n");

	/* set up the exception vectors */
	vpage0 = (Vpage0*)HVECTORS;
	memmove(vpage0->vectors, vectors, sizeof(vpage0->vectors));
	memmove(vpage0->vtable,  vtable,  sizeof(vpage0->vtable));
	print("Int vec move over\n");
	
	cacheuwbinv();
	
	print("Cache Off\n");

	setr13(PsrMfiq, (u32int*)(FIQSTKTOP));
	setr13(PsrMirq, m->irqstack+nelem(m->irqstack));
	setr13(PsrMabt, m->abtstack+nelem(m->abtstack));
	setr13(PsrMund, m->undstack+nelem(m->undstack));
	setr13(PsrMsys, m->undstack+nelem(m->sysstack));
	print("Set R13\n");
	intr_enable();
	print("Enable Interrupt\n");
	coherence();
}

/*
 * called direct from intr.s to handle fiq interrupt.
 */

void
fiq(Ureg* ureg)
{
	Vctl *v;
	for(v = vfiq; v; v = v->next) {
		if(*v->reg & v->mask) {
			coherence();
			v->f(ureg, v->a);
			coherence();
		}
	}
}

void
irqenable(int irq, void (*f)(Ureg*, void*), void* a)
{
	Vctl *v;
	Intregs *ip;
	u32int *enable;
	

	u32int vicvec;
	

	ip = S3C6410_GET_INTREGS_BASE(irq);
	vicvec=S3C6410_GET_INTREGS_VEC(irq);

	
	v = (Vctl*)malloc(sizeof(Vctl));
	if(v == nil)
		panic("irqenable: no mem");
	v->vec = irq;
	v->f   = f;
	v->a   = a;
	v->reg = &ip->IRQSTATUS;
	v->type= &ip->INTSELECT;
	v->mask= 1 << (vicvec);
	enable = &ip->INTENABLE;

	v->next = virq;
	virq = v;
	*v->type= (*v->type & (~v->mask) );
	*enable = v->mask;
	print("Enabled irq %d\n", irq);
}

void
fiqenable(int fiq, void (*f)(Ureg*, void*), void* a)
{
	Vctl *v;
	Intregs *ip;
	u32int *enable;
	

	u32int vicvec;
	

	ip = S3C6410_GET_INTREGS_BASE(fiq);
	vicvec=S3C6410_GET_INTREGS_VEC(fiq);

	
	v = (Vctl*)malloc(sizeof(Vctl));
	if(v == nil)
		panic("irqenable: no mem");
	v->vec = fiq;
	v->f   = f;
	v->a   = a;
	v->reg = &ip->FIQSTATUS;
	v->type= &ip->INTSELECT;
	v->mask= 1 << (vicvec);
	enable = &ip->INTENABLE;

	v->next = vfiq;
	vfiq = v;
	*v->type= (*v->type | (v->mask) );
	*enable = v->mask;
	print("Enabled irq %d\n", fiq);
}

/* called by trap to handle irq interrupts. */
static void
irq(Ureg* ureg)
{
	Vctl *v;
	for(v = virq; v; v = v->next) {
		if(*v->reg & v->mask) {
			coherence();
			v->f(ureg, v->a);
			coherence();
		}
	}
}

void
setpanic(void)
{
	spllo();
	consoleprint = 1;
}

static void
sys_trap_error(int type)
{
	char errbuf[ERRMAX];
	sprint(errbuf, "sys: trap: %s\n", trapname(type));
	error(errbuf);
}

static void
faultarm(Ureg *ureg)
{
	char buf[ERRMAX];

	sprint(buf, "sys: trap: fault pc=%8.8lux", (ulong)ureg->pc);
	if(0){
		iprint("%s\n", buf);
		dumpregs(ureg);
	}
	disfault(ureg, buf);
}

Instr BREAK = 0xE6BAD010;
int (*catchdbg)(Ureg *, uint);
#define waslo(sr) (!((sr) & (PsrDirq|PsrDfiq)))


void
trap(Ureg *ureg)
{
	int rem, itype, t;
/*
	print("Trap type: %uX \n",ureg->type);
	print("Mach stack address:%uX \n",(ulong)m->stack);
	print("Ureg address:	  %uX \n",(ulong)ureg);
*/
	if(up != nil)
		rem = ((char*)ureg)-up->kstack;
	else rem = ((char*)ureg)-(char*)m->stack;

	if(ureg->type != PsrMfiq && rem < 256) {
		dumpregs(ureg);
		panic("trap %d stack bytes remaining (%s), "
			  "up=#%8.8lux ureg=#%8.8lux pc=#%8.8ux"
			  ,rem, up?up->text:"", up, ureg, ureg->pc);
		for(;;);
	}

	itype = ureg->type;
	/*	All interrupts/exceptions should be resumed at ureg->pc-4,
		except for Data Abort which resumes at ureg->pc-8. */
	if(itype == PsrMabt+1)
		ureg->pc -= 8;
	else ureg->pc -= 4;

	if(up){
		up->pc = ureg->pc;
		up->dbgreg = ureg;
	}
	//print("enter switch\n");
	
	switch(itype) {
	case PsrMirq:
		t = m->ticks;		/* CPU time per proc */
		up = nil;		/* no process at interrupt level */
		irq(ureg);
		up = m->proc;
		preemption(m->ticks - t);
		m->intr++;
		break;

	case PsrMund:
	
		if(*(ulong*)ureg->pc == BREAK && breakhandler) {
			int s;
			Proc *p;

			p = up;
			s = breakhandler(ureg, p);
			if(s == BrkSched) {
				p->preempted = 0;
				sched();
			} else if(s == BrkNoSched) {
				/* stop it being preempted until next instruction */
				p->preempted = 1;
				if(up)
					up->dbgreg = 0;
				return;
			}
			break;
		}
	
//		print("Undefined Instruction %uX\n",*(ulong*)ureg->pc);
		if(up == nil) goto faultpanic;
		spllo();
		if(waserror()) {
			if(waslo(ureg->psr) && up->type == Interp)
				disfault(ureg, up->env->errstr);
			setpanic();
			dumpregs(ureg);
			panic("%s", up->env->errstr);
		}
	
		if(!fpiarm(ureg)) {
			dumpregs(ureg);
			sys_trap_error(ureg->type);
		}
	
		poperror();
		break;

	case PsrMsvc: /* Jump through 0 or SWI */
		if(waslo(ureg->psr) && up && up->type == Interp) {
			spllo();
			dumpregs(ureg);
			sys_trap_error(ureg->type);
		}
		setpanic();
		dumpregs(ureg);
		panic("SVC/SWI exception");
		break;

	case PsrMabt: /* Prefetch abort */
		if(catchdbg && catchdbg(ureg, 0))
			break;
		/* FALL THROUGH */
	case PsrMabt+1: /* Data abort */
		if(waslo(ureg->psr) && up && up->type == Interp) {
			spllo();
			faultarm(ureg);
		}
		print("Data Abort\n");
		/* FALL THROUGH */

	default:
faultpanic:
		print("enter faultpanic\n");
		setpanic();
		dumpregs(ureg);
	print("Infinite loop for debug\n");
	for(;;);
		panic("exception %uX %s\n", ureg->type, trapname(ureg->type));
		break;
	}
	


	splhi();
	if(up)
		up->dbgreg = 0;		/* becomes invalid after return from trap */
}
