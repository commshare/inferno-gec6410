#define IOBASE			0x70000000		/* base of io regs */
#define INTREGS0		(IOBASE+0x1200000)
#define INTREGS1		(IOBASE+0x1300000)
//#define POWERREGS		(IOBASE+0x100000)
//#define PL011REGS		(IOBASE+0x201000)

//#define UART_PL01x_FR_RXFE  0x10
//#define UART_PL01x_FR_TXFF	0x20

typedef struct Intregs Intregs;

/* interrupt control registers */
struct Intregs {
	u32int	IRQSTATUS;
	u32int	FIQSTATUS;
	u32int	IRAWINTR;
	u32int	INTSELECT;
	u32int	INTENABLE;
	u32int	INTENCLEAR;
	u32int	SOFTINT;
	u32int	ISOFTINCLEAR;
	u32int	PROTECTION;
	u32int	SWPRIORITYMASK;
	u32int	VECTADDR[32];
	u32int	VECTPRIORITY[32];
	u32int	ADDRESS;
};

#define	S3C6410_GET_INTREGS_BASE(vec) (struct Intregs *)((vec)<32?INTREGS0:INTREGS1)
#define	S3C6410_GET_INTREGS_VEC(vec) ((vec)<32?(vec):(vec-32))
enum {
/*VIC0*/
	INT_EINT0	= 0,
	INT_EINT1	= 1,
	INT_RTC_TIC	= 2,
	INT_CAMIF_C	= 3,
	INT_CAMIF_P	= 4,
	INT_I2C1	= 5,
	INT_I2S		= 6,
	INT_Reserved= 7,
	INT_3D		= 8,
	INT_POSTO	= 9,
	INT_ROTATOR	= 10,
	INT_2D		= 11,
	INT_TVENC	= 12,
	INT_SCALER	= 13,
	INT_BATF	= 14,	//battery fail
	INT_JPEG	= 15,
	INT_MFC		= 16,
	INT_SDMA0	= 17,
	INT_SDMA1	= 18,
	INT_ARM_DMAERR	= 19,
	INT_ARM_DMA	= 20,
	INT_ARM_DMAS= 21,
	INT_KEYPAD	= 22,	//keyboard
	INT_TIMER0	= 23,
	INT_TIMER1	= 24,
	INT_TIMER2	= 25,
	INT_WDT		= 26,	//watch dog
	INT_TIMER3	= 27,
	INT_TIMER4	= 28,
	INT_LCD0	= 29,	//fifo int
	INT_LCD1	= 30,	//vsync
	INT_LCD2	= 31,	//I/F
/*VIC1*/	
	INT_EINT2	= 32,
	INT_EINT3	= 33,
	INT_PCM0	= 34,
	INT_PCM1	= 35,
	INT_AC97	= 36,
	INT_UART0	= 37,
	INT_UART1	= 38,
	INT_UART2	= 39,
	INT_UART3	= 40,
	INT_DMA0	= 41,
	INT_DMA1	= 42,
	INT_ONENAND0= 43,
	INT_ONENAND1= 44,
	INT_NFC		= 45,
	INT_CFC		= 46,
	INT_UHOST	= 47,
	INT_SPI0	= 48,
	INT_SPI_HSMMC2	= 49,
	INT_I2C0	= 50,
	INT_HSItx	= 51,
	INT_HSIrx	= 52,
	INT_EINT4	= 53,
	INT_MSM		= 54,
	INT_HOSTIF	= 55,
	INT_HSMMC0	= 56,
	INT_HSMMC1	= 57,
	INT_OTG		= 58,
	INT_IrDA	= 59,
	INT_RTC_ALARM	= 60,
	INT_SEC		= 61,
	INT_PENDNUP	= 62,
	INT_ADC		= 63
};




