#define UART_NR	S3C64XX_UART0
#define ELFIN_UART_BASE		0x7F005000

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

	S3C64XX_REG8	UTXH;
	S3C64XX_REG8	res1[3];
	S3C64XX_REG8	URXH;
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

static inline S3C64XX_UART * S3C64XX_GetBase_UART(S3C64XX_UARTS_NR nr)
{
	return (S3C64XX_UART *)(ELFIN_UART_BASE + (nr*0x400));
}
void serial_setbrg(void)
{
//	DECLARE_GLOBAL_DATA_PTR;
	S3C64XX_UART *	uart0;
	uart0=S3C64XX_GetBase_UART(UART_NR);
	uart0->UBRDIV=20;
	uart0->UDIVSLOT=0xddd5;

	int i;
	for (i = 0; i < 100; i++);
}

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 *
 */
int serial_init(void)
{
	serial_setbrg();
	S3C64XX_UART *	uart0;
	uart0=S3C64XX_GetBase_UART(UART_NR);
	uart0->ULCON = 0x00000003;
	uart0->UTRSTAT=0x6;

	return (0);
}

/*
 * Read a single byte from the serial port. Returns 1 on success, 0
 * otherwise. When the function is succesfull, the character read is
 * written into its argument c.
 */
int serial_getc(void)
{
	S3C64XX_UART *const uart = S3C64XX_GetBase_UART(UART_NR);

	/* wait for character to arrive */
	while (!(uart->UTRSTAT & 0x1));

	return uart->URXH & 0xff;
}

/*
 * Output a single byte to the serial port.
 */
void serial_putc(const char c)
{
	S3C64XX_UART *const uart = S3C64XX_GetBase_UART(UART_NR);
	while (!(uart->UTRSTAT & 0x2));
	uart->UTXH = c;
	if (c == '\n')
		serial_putc('\r');
}

/*
 * Test whether a character is in the RX buffer
 */
int serial_tstc(void)
{
	S3C64XX_UART *const uart = S3C64XX_GetBase_UART(UART_NR);

	return uart->UTRSTAT & 0x1;
}

void serial_puts(const char *s)
{
	while (*s) {
		serial_putc(*s++);
	}
}

void serial_addr(void *addr)
{
	int i;
	vu_char *ca = (vu_char *)&addr;
	vu_char h, l;
	for (i = 3; i >= 0; --i)
	{
		h = ca[i] / 16;
		l = ca[i] % 16;
		serial_putc(h < 10 ? h + 0x30 : h - 10 + 0x41);
		serial_putc(l < 10 ? l + 0x30 : l - 10 + 0x41);
	}
}
void
serial_putsi(char *s, int n) {
	serial_puts("??\n");
	while(*s != 0 && n-- >=0) {
		if (*s == '\n')
			serial_putc('\r');
		serial_putc(*s++);
	}
}
