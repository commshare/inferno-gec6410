#include "../port/portclock.c"
/*USE TIMER3 as a System Counter
and TIMER2 as a Interrupt Generator*/
#define ELFIN_TIMER_BASE	0x7F006000
#define TIMER0		(ELFIN_TIMER_BASE + 0xc)
#define TIMER1		(ELFIN_TIMER_BASE + 0x18)
#define TIMER2		(ELFIN_TIMER_BASE + 0x24)
#define TIMER3		(ELFIN_TIMER_BASE + 0x30)
#define TIMER4		(ELFIN_TIMER_BASE + 0x3c)
#define CSTAT		(ELFIN_TIMER_BASE + 0x44)
#define INTTIMER3	27
#define	INTTIMER2	25
struct timer_cfg {
	u32int	tcfg0;
	u32int	tcfg1;
	u32int	tcon;
};
struct timer_qwm {
	u32int	tcntb;
	u32int	tcmpb;
	u32int	tcnto;
};
struct	timer_nq {
	u32int	tcntb;
	u32int	tcnto;
};
typedef struct timer_cfg timer_cfg;
typedef struct timer_qwm timer_qwm;
typedef struct timer_qwm timer_qwm;
enum {
	SystimerFreq    = 1*Mhz,
	MaxPeriod   = SystimerFreq/HZ,
	MinPeriod   = SystimerFreq/(100*HZ)
};


static void
clockintr(Ureg * ureg, void *)
{
	u32int *cstat, temp;
	cstat = (u32int*)CSTAT;
	/* dismiss interrupt */
	temp = *cstat;
	temp |= 0x80;
	*cstat = temp;
	timerintr(ureg, 0);
}
void
clockinit(void)
{
	timer_cfg *cfg;
	timer_qwm *tm;
	u32int t0, t1;
	u32int *cstat, temp;
	cstat = (u32int*)CSTAT;
	temp = *cstat;
	temp |= 0x4;	//enable timer2 intr
	*cstat = temp;
		//we do NOT need timer3 intr, but timer2 needed
	cfg = (timer_cfg*)ELFIN_TIMER_BASE;
	tm = (timer_qwm*)TIMER3;
	tm->tcntb = 2000000; //initial the tcntb of timer3 to be 2 million
	cfg->tcfg0 = 0x00004141; //prescale=65, then the freq = 1MHZ
	cfg->tcfg1 = 0x00000000; //divide = 1
	cfg->tcon = 0xB0000; //set timer3 on, auto reload, and update right now
	cfg->tcon = 0x90000;
	do{
		t0 = lcycles();
	}while(tm->tcnto < 1900000 || tm->tcnto > 1905000);	
	do{
		t1 = lcycles();
	}while(tm->tcnto > 900000); //1s
	t1 -= t0;
	m->cpuhz = t1; //then we got the freq of cpu
	tm->tcntb = 0xffffffff; //initial the tcntb of timer3 to be 0xffffffff
	cfg->tcon = 0xB0000; //reinit
	cfg->tcon = 0x90000;
	while (tm->tcnto < 2000000);
	//while (1) serial_addr((void *)(tm->tcnto), 1);
	//tm->tcmpb = tm->tcnto + 1;
	//irqenable(INTTIMER3, clockintr, nil);
	irqenable(INTTIMER2, clockintr, nil);
}

void
clockcheck(void) { return; }

uvlong
fastticks(uvlong *hz)
{
	timer_qwm *tm;
	tm = (timer_qwm*)TIMER3;
	uvlong t;
	if(hz)
		*hz = SystimerFreq;
	t = (uvlong)(0xffffffff - tm->tcnto);
	//print("aaa%lud, %d\n", t, t);
	m->fastclock = t;
	return t;
}
/*use timer2 in timerser*/
void
timerset(uvlong next)
{
	u32int	temp;
	timer_cfg *cfg;
	cfg = (timer_cfg*)ELFIN_TIMER_BASE;
	timer_qwm *tm;
	tm = (timer_qwm*)TIMER2;
	vlong now, period;
	now = fastticks(nil);
	period = next - fastticks(nil);
	if(period < MinPeriod)
		period = MinPeriod;
	else if(period > MaxPeriod)
		period = MaxPeriod;
	tm->tcntb = period;		//set timer countdown
	u32int *cstat;
	cstat = (u32int*)CSTAT;
	temp = *cstat;
	temp |= 0x4;	//enable timer2 intr
	*cstat = temp;
	cfg->tcon = 0x92000;
	cfg->tcon = 0x91000;
}

ulong
µs(void)
{
	if(SystimerFreq != 1*Mhz)
		return fastticks2us(fastticks(nil));
	return fastticks(nil);
}

void
microdelay(int n)
{
	u32int now, diff;
	timer_qwm *tm;
	tm = (timer_qwm*)TIMER3;
	diff = n + 1;
	now = tm->tcnto;
	while(tm->tcnto - now < diff)
		;
}

void
delay(int n)
{
	while(--n >= 0)
		microdelay(1000);
}

void
armtimerset(int n)	//using timer0
{
	print("armtimerset\n");
 	timer_qwm *tm;
	tm = (timer_qwm*)TIMER0;
	timer_cfg *cfg;
	cfg = (timer_cfg*)ELFIN_TIMER_BASE;
	u32int *cstat;
	cstat = (u32int*)CSTAT;
	if(n > 0){
		tm->tcmpb = 0;
		tm->tcntb = n;
		cfg->tcon |= 0x0000000b;
		cfg->tcon &= 0xfffffffc;
		*cstat |= 0x00000001;
	}else{
		tm->tcntb = 0;
		tm->tcmpb = 0;
		cfg->tcon |= 0x00000002;
		cfg->tcon &= 0xfffffff0;
		*cstat &= 0xfffffffe;
		*cstat |= 0x00000020;
	}
}
