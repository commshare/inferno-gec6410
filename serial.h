#define UART_NR	S3C64XX_UART0
#define ELFIN_UART_BASE		0x7F005000	//Address of serials structure.

typedef volatile unsigned long	vu_long;	
typedef volatile unsigned short vu_short;
typedef volatile unsigned char	vu_char;
typedef unsigned long	ulong;
typedef unsigned short	ushort;
typedef unsigned char	uchar;
typedef vu_char		S3C64XX_REG8;
typedef vu_short	S3C64XX_REG16;
typedef vu_long		S3C64XX_REG32;
typedef struct {				
	S3C64XX_REG32	ULCON;
	S3C64XX_REG32	UCON;
	S3C64XX_REG32	UFCON;
	S3C64XX_REG32	UMCON;
	S3C64XX_REG32	UTRSTAT;
	S3C64XX_REG32	UERSTAT;
	S3C64XX_REG32	UFSTAT;
	S3C64XX_REG32	UMSTAT;

	S3C64XX_REG8	UTXH;		//transmit
	S3C64XX_REG8	res1[3];
	S3C64XX_REG8	URXH;		//receive
	S3C64XX_REG8	res2[3];

	S3C64XX_REG32	UBRDIV;
	S3C64XX_REG32	UDIVSLOT;
}	S3C64XX_UART;

typedef enum {
	S3C64XX_UART0,
	S3C64XX_UART1,
	S3C64XX_UART2,
	S3C64XX_UART3,
} S3C64XX_UARTS_NR;
static inline S3C64XX_UART * S3C64XX_GetBase_UART(S3C64XX_UARTS_NR nr);
void serial_setbrg(void);
int serial_init(void);
int serial_getc(void);
void serial_putc(const char c);
int serial_tstc(void);
void serial_puts(const char *s);
void serial_addr(void *addr, int type);
void serial_putsi(char *s, int n);
