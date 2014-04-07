#include "./serial.h"
#include "./u.h"
#include "../port/lib.h"
#include "dat.h"
#include "fns.h"
#include "mem.h"
#include "armv6.h"
#include "s3c6410.h"


static inline S3C64XX_UART * S3C64XX_GetBase_UART(S3C64XX_UARTS_NR nr)
{
	return (S3C64XX_UART *)(ELFIN_UART_BASE + (nr*0x400));			//offset 0x400
}
/*void serial_setbrg(void)
{
//	DECLARE_GLOBAL_DATA_PTR;
	S3C64XX_UART *	uart0;
	uart0=S3C64XX_GetBase_UART(UART_NR);
	uart0->UBRDIV=20;							
	uart0->UDIVSLOT=0xddd5;

	int i;
	for (i = 0; i < 100; i++);
}
*/
/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 *
 */
 /*
int serial_init(void)				//not used because of uboot
{
	serial_setbrg();
	S3C64XX_UART *	uart0;
	uart0=S3C64XX_GetBase_UART(UART_NR);
	uart0->ULCON = 0x00000003;
	uart0->UTRSTAT=0x6;

	return (0);
}
*/

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

void serial_addr(void *addr, int type)
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
	if (type)
		serial_puts("\n");
	else serial_puts("\t");
}

void
serial_putsi(char *s, int n) {
	while(*s != 0 && n-- >=0) {
		if (*s == '\n')
			serial_putc('\r');
		serial_putc(*s++);
	}
}
/****************************debug function***************************/
void serial_checkpoint(void){
	serial_puts("****************check point ");
	serial_puts("Now, PC: ");
	serial_addr((void *)getpc(), 1);
}

void dump_mem(ulong *addr,ulong length){
	int i;
	serial_puts("DUMP MEMORY FROM ");
	serial_addr( (void *) addr,0);
	serial_puts("TO ");
	serial_addr( (void *)( ((char *)addr)+length ),1);
	for(i=0;i<(length>>2);i++){
		serial_addr( (void*) addr,0);
		serial_addr( (void*) *addr,1);
		addr++;
	}
}

void print_TTB(void){
	
	serial_puts("TTB is ");
	serial_addr( (void*)getTTB(),1);
	dump_mem((ulong *)getTTB(),1024);
}

