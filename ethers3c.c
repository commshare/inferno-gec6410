/*
 * Kernel proxy for usb ethernet device
 */

#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "../port/error.h"
#include "../port/netif.h"

#include "etherif.h"
#include "../ip/ip.h"

#include "dm9000.h"


#define	GET4(p)		((p)[3]<<24 | (p)[2]<<16 | (p)[1]<<8  | (p)[0])
#define	PUT4(p, v)	((p)[0] = (v), (p)[1] = (v)>>8, \
			 (p)[2] = (v)>>16, (p)[3] = (v)>>24)
#define	dprint	if(debug) print
#define ddump	if(0) dump

static int debug = 0;

typedef struct Ether Ether;

enum {
	Bind	= 0,
	Unbind,

	SmscRxerror	= 0x8000,
	SmscTxfirst	= 0x2000,
	SmscTxlast	= 0x1000,
};



	
static Cmdtab cmds[] = {
	{ Bind,		"bind",		7, },
	{ Unbind,	"unbind",	0, },
};





typedef struct board_info {
	Ether*	edev;
	Chan*	inchan;
	Chan*	outchan;
	char*	buf;
	int	bufsize;
	int	maxpkt;
	uint	rxbuf;
	uint	rxpkt;
	uint	txbuf;
	uint	txpkt;
	QLock;

	ulong *io_addr;	/* Register I/O base address */
	ulong *io_data;	/* Data I/O address */
	ushort irq;		/* IRQ */

	ushort tx_pkt_cnt;
	ushort queue_pkt_len;
	ushort queue_start_addr;
	ushort dbug_cnt;
	uchar io_mode;		/* 0:word, 2:byte */
	uchar phy_addr;

//	void (*inblk)(void __iomem *port, void *data, int length);
//	void (*outblk)(void __iomem *port, void *data, int length);
//	void (*dumpblk)(void __iomem *port, int length);

//	struct resource	*addr_res;   /* resources found */
//	struct resource *data_res;
//	struct resource	*addr_req;   /* resources requested */
//	struct resource *data_req;
//	struct resource *irq_res;

//	struct timer_list timer;
//	unsigned char srom[128];

//	struct mii_if_info mii;
	ulong msg_enable;
} board_info_t;


/* function declaration ------------------------------------- */
static void dm9000_reset(Ether* edev);
static void dm9000_probe(Ether* edev);
static int dm9000_open(Ether* edev);
//static int dm9000_start_xmit(struct sk_buff *, struct net_device *);
//static int dm9000_stop(struct net_device *);


//static void dm9000_timer(unsigned long);
static void dm9000_init_dm9000(Ether* edev);

//static irqreturn_t dm9000_interrupt(int, void *);

//static int dm9000_phy_read(struct net_device *dev, int phyaddr_unsused, int reg);
//static void dm9000_phy_write(struct net_device *dev, int phyaddr_unused, int reg,int value);
//static ushort read_srom_word(board_info_t *, int);
//static void dm9000_rx(struct net_device *);
//static void dm9000_hash_table(struct net_device *);

//#define DM9000_PROGRAM_EEPROM
#ifdef DM9000_PROGRAM_EEPROM
static void program_eeprom(board_info_t * db);
#endif
/* DM9000 network board routine ---------------------------- */


static inline uchar
readb(ulong *addr){
	return *(uchar *)addr;
}

static inline void
writeb(int value, ulong *addr){
	*(uchar *)addr = (uchar)value;
}

/*
 *   Read a byte from I/O port
 */

static uchar
ior(board_info_t * db, int reg)
{
	writeb(reg, db->io_addr);
	return readb(db->io_data);
}

/*
 *   Write a byte to I/O port
 */

static void
iow(board_info_t * db, int reg, int value)
{
	writeb(reg, db->io_addr);
	writeb(value, db->io_data);
}

static void
dm9000_reset(Ether* edev)
{
	board_info_t *db;
	db = edev->ctlr;
	print("****************************dm9000x: resetting,ioaddr=%lux",(ulong)(db->io_addr));
	/* RESET device */
	iow(db, DM9000_NCR, NCR_RST);
}

static void 
dm9000_probe(Ether *edev)
{
	board_info_t *db;
	db = edev->ctlr;
	db->io_addr = (ulong *)DM9000_ADDR;
	db->io_data = (ulong *)DM9000_DATA;
	print("****************************dm9000x: setting ioaddr=%lux , iodata=%lux", (ulong)(db->io_addr), (ulong)(db->io_data));
	dm9000_open(edev);
}
	

static int
dm9000_open(Ether *edev)
{
	board_info_t *db;
	db = edev->ctlr;

	print("entering dm9000_open\n");

//	if (request_irq(dev->irq, &dm9000_interrupt, DM9000_IRQ_FLAGS, dev->name, dev))
//		return -EAGAIN;
/* TODO:add irq*/

	/* Initialize DM9000 board */
	dm9000_reset(edev);
	dm9000_init_dm9000(edev);

	/* Init driver variable */
	db->dbug_cnt = 0;

	/* set and active a timer process */
/*
	init_timer(&db->timer);
	db->timer.expires  = DM9000_TIMER_WUT;
	db->timer.data     = (unsigned long) dev;
	db->timer.function = &dm9000_timer;
	add_timer(&db->timer);

	mii_check_media(&db->mii, netif_msg_link(db), 1);
	netif_start_queue(dev);
*/

	return 0;
}

/*
 * Initilize dm9000 board
 */
static void
dm9000_init_dm9000(Ether *edev)
{
	board_info_t *db;
	db = edev->ctlr;

	print("entering %s\n","dm9000_init_dm9000");

	/* I/O mode */
	db->io_mode = ior(db, DM9000_ISR) >> 6;	/* ISR bit7:6 keeps I/O mode */

	/* GPIO0 on pre-activate PHY */
	iow(db, DM9000_GPR, 0);	/* REG_1F bit0 activate phyxcer */
	iow(db, DM9000_GPCR, GPCR_GEP_CNTL);	/* Let GPIO0 output */
	iow(db, DM9000_GPR, 0);	/* Enable PHY */

	/* Program operating register */
	iow(db, DM9000_TCR, 0);	        /* TX Polling clear */
	iow(db, DM9000_BPTR, 0x3f);	/* Less 3Kb, 200us */
	iow(db, DM9000_FCR, 0xff);	/* Flow Control */
	iow(db, DM9000_SMCR, 0);        /* Special Mode */
	/* clear TX status */
	iow(db, DM9000_NSR, NSR_WAKEST | NSR_TX2END | NSR_TX1END);
	iow(db, DM9000_ISR, ISR_CLR_STATUS); /* Clear interrupt status */

	/* Set address filter table */
//	dm9000_hash_table(dev);

	/* Activate DM9000 */
	iow(db, DM9000_RCR, RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN);
	/* Enable TX/RX interrupt mask */
	iow(db, DM9000_IMR, IMR_PAR | IMR_PTM | IMR_PRM);

	/* Init Driver variable */
	db->tx_pkt_cnt = 0;
	db->queue_pkt_len = 0;
//	dev->trans_start = 0;
}

static void
dm9000_attach(Ether *edev){
	board_info_t *ctlr;

	ctlr = edev->ctlr;
	ctlr->edev = edev;
}

static int
ethers3cinit(Ether* edev)
{
	board_info_t *ctlr;

	ctlr = malloc(sizeof(board_info_t));
	edev->ctlr = ctlr;
	edev->irq = -1;
	edev->mbps = 100;	/* TODO: get this from usbether */

	/*
	 * Linkage to the generic ethernet driver.
	 */
	edev->attach = dm9000_attach;
	edev->transmit = 0;
	edev->interrupt = 0;
	edev->ifstat = 0;
	edev->ctl = 0;

	edev->arg = edev;
	/* TODO: promiscuous, multicast (for ipv6), shutdown (for reboot) */
//	edev->promiscuous = etherusbpromiscuous;
//	edev->shutdown = etherusbshutdown;
//	edev->multicast = etherusbmulticast;

	dm9000_probe(edev);

	return 0;
}

board_info_t test;
void
ethers3clink(void)
{
	test.io_addr = (ulong *)DM9000_ADDR;
	test.io_data = (ulong *)DM9000_DATA;
	print("****************************dm9000x: setting ioaddr=%lux , iodata=%lux", (ulong)(test.io_addr), (ulong)(test.io_data));
	print("DM9000 NCR:%lux\n",ior(&test, DM9000_NCR));
	iow(&test, DM9000_NCR, NCR_RST);
	print("DM9000 NCR:%lux\n",ior(&test, DM9000_NCR));
	ulong vidl=ior(&test, DM9000_VIDL);
	ulong vidh=ior(&test, DM9000_VIDH);
	ulong pidl=ior(&test, DM9000_PIDL);
	ulong pidh=ior(&test, DM9000_PIDH);
	ulong id = (pidh << 24)|(pidl << 16) | (vidh << 8) | (vidl) ;
	print("DM9000 ID:%lux\n",id);
	addethercard("DM9000", ethers3cinit);
}
