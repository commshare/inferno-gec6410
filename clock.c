#include "../port/portclock.c"

/*
#define SYSTIMERS   (IOBASE+0x3000)
#define ARMTIMER    (IOBASE+0xB400)
*/
#define ELFIN_TIMER_BASE	0x7F006000
#define TIMER0		(ELFIN_TIMER_BASE + 0xc)
#define TIMER1		(ELFIN_TIMER_BASE + 0x18)
#define TIMER2		(ELFIN_TIMER_BASE + 0x24)
#define TIMER3		(ELFIN_TIMER_BASE + 0x30)
#define TIMER4		(ELFIN_TIMER_BASE + 0x3c)
#define CSTAT		(ELFIN_TIMER_BASE + 0x44)
#define INTTIMER3	27
/*
#define TCFG0_REG		__REG(0x7F006000)
#define TCFG1_REG		__REG(0x7F006004)
#define TCON_REG		__REG(0x7F006008)
#define TCNTB0_REG		__REG(0x7F00600c)
#define TCMPB0_REG		__REG(0x7F006010)
#define TCNTO0_REG		__REG(0x7F006014)
#define TCNTB1_REG		__REG(0x7F006018)
#define TCMPB1_REG		__REG(0x7F00601c)
#define TCNTO1_REG		__REG(0x7F006020)
#define TCNTB2_REG		__REG(0x7F006024)
#define TCMPB2_REG		__REG(0x7F006028)
#define TCNTO2_REG		__REG(0x7F00602c)
#define TCNTB3_REG		__REG(0x7F006030)
#define TCMPB3_REG		__REG(0x7F006034)
#define TCNTO3_REG		__REG(0x7F006038)
#define TCNTB4_REG		__REG(0x7F00603c)
#define TCNTO4_REG		__REG(0x7F006040)
*/
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

/*
typedef struct Systimers Systimers;
typedef struct Armtimer Armtimer;

struct Systimers {
	u32int  cs;
	u32int  clo;
	u32int  chi;
	u32int  c0;
	u32int  c1;
	u32int  c2;
	u32int  c3;
};

struct Armtimer {
	u32int  load;
	u32int  val;
	u32int  ctl;
	u32int  irqack;
	u32int  irq;
	u32int  maskedirq;
	u32int  reload;
	u32int  predivider;
	u32int  count;
};

enum {
	CntPrescaleShift    = 16,
	CntPrescaleMask     = 0xFF,
	CntEnable           = 1<<9,
	TmrDbgHalt          = 1<<8,
	TmrEnable           = 1<<7,
	TmrIntEnable        = 1<<5,
	TmrPrescale1        = 0x00<<2,
	TmrPrescale16       = 0x01<<2,
	TmrPrescale256      = 0x02<<2,
	CntWidth16          = 0<<1,
	CntWidth32          = 1<<1
};
*/

static void
clockintr(Ureg * ureg, void *)
{
	u32int *cstat;
	cstat = (u32int*)CSTAT;
	/* dismiss interrupt */
	*cstat |= 0x00000008;
	timerintr(ureg, 0);
}
void
clockinit(void)
{
	timer_cfg *cfg;
	timer_qwm *tm;
	u32int t0, t1;
	u32int *cstat;
	cstat = (u32int*)CSTAT;
	*cstat |= 0x8;	//enable timer3 intr

	cfg = (timer_cfg*)ELFIN_TIMER_BASE;
	tm = (timer_qwm*)TIMER3;
	tm->tcntb = 2000000; //initial the tcntb of timer4 to be 2 million
	cfg->tcfg0 = 0x00004141; //prescale=65, then the freq = 1MHZ
	cfg->tcfg1 = 0x00000000; //divide = 1
	cfg->tcon = 0xB0000; //set timer4 on, auto reload, and update right now
	cfg->tcon = 0x90000;
	do{
		t0 = lcycles();
	}while(tm->tcnto < 1900000 || tm->tcnto > 1905000);	
	do{
		t1 = lcycles();
	}while(tm->tcnto > 900000); //1s
	t1 -= t0;
	m->cpuhz = t1; //then we got the freq of cpu
	tm->tcntb = 0xffffffff; //initial the tcntb of timer4 to be 0xffffffff
	cfg->tcon = 0xB0000; //reinit
	cfg->tcon = 0x90000;
	tm->tcmpb = tm->tcnto + 1;
	irqenable(INTTIMER3, clockintr, nil);
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
	t = 0xffffffff - tm->tcnto;
	m->fastclock = t;
	return m->fastclock;
}

void
timerset(uvlong next)
{
	timer_qwm *tm;
	tm = (timer_qwm*)TIMER3;
	vlong now, period;
	now = fastticks(nil);
	period = next - fastticks(nil);
	if(period < MinPeriod)
		next = now + MinPeriod;
	else if(period > MaxPeriod)
		next = now + MaxPeriod;
	tm->tcmpb = (ulong)next;
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
