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

/* Board/System/Debug information/definition ---------------- */

#define DM9000_PHY		0x40	/* PHY address 0x01 */
#define DM9000_CMD         0X04    //1:DATA,0:ADDR

#define CARDNAME "dm9000"
#define PFX CARDNAME ": "


#define	GET4(p)		((p)[3]<<24 | (p)[2]<<16 | (p)[1]<<8  | (p)[0])
#define	PUT4(p, v)	((p)[0] = (v), (p)[1] = (v)>>8, \
			 (p)[2] = (v)>>16, (p)[3] = (v)>>24)
#define	dprint	if(debug) print
#define ddump	if(0) dump

static int debug = 1;

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

static uchar def_ea[6]={ 0x0c, 0x00, 0x00, 0x7f, 0x13, 0x00};





typedef struct board_info {
	Lock;
	
	Ether*	edev;
	Chan*	inchan;
	Chan*	outchan;
	char*	buf;
	int	bufsize;
	int	maxpkt;
	uint	rxbuf;
//	uint	rxpkt;
	uint	txbuf;
//	uint	txpkt;
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

	void (*inblk)(void *port, void *data, int length);
	void (*outblk)(void *port, void *data, int length);
	void (*dumpblk)(void *port, int length);

//	struct resource	*addr_res;   /* resources found */
//	struct resource *data_res;
//	struct resource	*addr_req;   /* resources requested */
//	struct resource *data_req;
//	struct resource *irq_res;

//	struct timer_list timer;
	unsigned char srom[128];

//	struct mii_if_info mii;
	ulong msg_enable;
} board_info_t;
typedef board_info_t Ctlr;


/* function declaration ------------------------------------- */
static void dm9000_reset(Ether *edev);
static void dm9000_probe(Ether *edev);
static int dm9000_open(Ether *edev);
static int dm9000_start_xmit(struct board_info *db, Block *b);
//static int dm9000_stop(struct net_device *);


//static void dm9000_timer(unsigned long);
static void dm9000_init_dm9000(Ether *edev);
static void dm9000_set_io(struct board_info *db, int byte_width);

static void dm9000_interrupt(Ureg *ureg, void *arg);

//static int dm9000_phy_read(struct net_device *dev, int phyaddr_unsused, int reg);
//static void dm9000_phy_write(struct net_device *dev, int phyaddr_unused, int reg,int value);
static ushort read_srom_word(board_info_t *, int);
static void dm9000_rx(Ether *edev);
static void dm9000_hash_table(Ether *edev);

//#define DM9000_PROGRAM_EEPROM
#ifdef DM9000_PROGRAM_EEPROM
static void program_eeprom(board_info_t * db);
#endif
/* DM9000 network board routine ---------------------------- */


static inline uchar
readb(ulong *addr){
	return *(uchar *)addr;
}

static inline uchar
readw(ulong *addr){
	return *(ushort *)addr;
}

static inline uchar
readl(ulong *addr){
	return *(ulong *)addr;
}

static inline void
writeb(int value, ulong *addr){
	*(uchar *)addr = (uchar)value;
}

static inline void
writew(int value, ulong *addr){
	*(ushort *)addr = (ushort)value;
}

static inline void
writel(int value, ulong *addr){
	*(ulong *)addr = (ulong)value;
}

static inline void
writesb(void *reg, void *data, int count){
	int i;
	uchar *regp = reg;
	uchar *p = data;
	for(i=0;i<count;i++){
		*regp = *p;
		p++;
	}
}

static inline void
writesw(void *reg, void *data, int count){
	int i;
	ushort *regp = reg;
	ushort *p = data;
	for(i=0;i<count;i++){
		*regp = *p;
		p++;
	}
}

static inline void
writesl(void *reg, void *data, int count){
	int i;
	ulong *regp = reg;
	ulong *p = data;
	for(i=0;i<count;i++){
		*regp = *p;
		p++;
	}
}

static inline void
readsb(void *reg, void *data, int count){
	int i;
	uchar *regp = reg;
	uchar *p = data;
	for(i=0;i<count;i++){
		*p = *regp;
		p++;
	}
}

static inline void
readsw(void *reg, void *data, int count){
	int i;
	ushort *regp = reg;
	ushort *p = data;
	for(i=0;i<count;i++){
		*p = *regp;
		p++;
	}
}

static inline void
readsl(void *reg, void *data, int count){
	int i;
	ulong *regp = reg;
	ulong *p = data;
	for(i=0;i<count;i++){
		*p = *regp;
		p++;
	}
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

/* routines for sending block to chip */

static void dm9000_outblk_8bit(void *reg, void *data, int count)
{
	writesb(reg, data, count);
}

static void dm9000_outblk_16bit(void *reg, void *data, int count)
{
	writesw(reg, data, (count+1) >> 1);
}

static void dm9000_outblk_32bit(void *reg, void *data, int count)
{
	writesl(reg, data, (count+3) >> 2);
}

/* input block from chip to memory */

static void dm9000_inblk_8bit(void *reg, void *data, int count)
{
	readsb(reg, data, count);
}


static void dm9000_inblk_16bit(void *reg, void *data, int count)
{
	readsw(reg, data, (count+1) >> 1);
}

static void dm9000_inblk_32bit(void *reg, void *data, int count)
{
	readsl(reg, data, (count+3) >> 2);
}

/* dump block from chip to null */

static void dm9000_dumpblk_8bit(void *reg, int count)
{
	int i;
	int tmp;

	for (i = 0; i < count; i++)
		tmp = readb(reg);
}

static void dm9000_dumpblk_16bit(void *reg, int count)
{
	int i;
	int tmp;

	count = (count + 1) >> 1;

	for (i = 0; i < count; i++)
		tmp = readw(reg);
}

static void dm9000_dumpblk_32bit(void *reg, int count)
{
	int i;
	int tmp;

	count = (count + 3) >> 2;

	for (i = 0; i < count; i++)
		tmp = readl(reg);
}

static void
dm9000_reset(Ether* edev)
{
	board_info_t *db;
	db = edev->ctlr;
	dprint("****************************dm9000x: resetting,ioaddr=%lux",(ulong)(db->io_addr));
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
	dprint("****************************dm9000x: setting ioaddr=%lux , iodata=%lux", (ulong)(db->io_addr), (ulong)(db->io_data));
	dm9000_open(edev);
}
	

static int
dm9000_open(Ether *edev)
{
	board_info_t *db;
	db = edev->ctlr;

	dprint("entering dm9000_open\n");

//	if (request_irq(dev->irq, &dm9000_interrupt, DM9000_IRQ_FLAGS, dev->name, dev))
//		return -EAGAIN;

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
	ulong byte_width;
	board_info_t *db;
	db = edev->ctlr;

	dprint("entering %s\n","dm9000_init_dm9000");

	/* I/O mode */
	db->io_mode = ior(db, DM9000_ISR) >> 6;	/* ISR bit7:6 keeps I/O mode */
	switch(db->io_mode){
		case 0:byte_width=2;break;
		case 1:byte_width=4;break;
		case 2:byte_width=1;break;
		default:byte_width=2;
	}
	dprint("DM9000 io_byte_width:%ld\n",byte_width);
	dm9000_set_io(db,byte_width);
	

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
	dm9000_hash_table(edev);

	/* Activate DM9000 */
	iow(db, DM9000_RCR, RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN);
	/* Enable TX/RX interrupt mask */
	iow(db, DM9000_IMR, IMR_PAR | IMR_PTM | IMR_PRM);

	/* Init Driver variable */
	db->tx_pkt_cnt = 0;
	db->queue_pkt_len = 0;
//	dev->trans_start = 0;
}

//#define DM9000_MULTICAST
#ifdef DM9000_MULTICAST

/*
 *  Calculate the CRC valude of the Rx packet
 *  flag = 1 : return the reverse CRC (for the received packet CRC)
 *         0 : return the normal CRC (for Hash Table index)
 */

static unsigned long
cal_CRC(unsigned char *Data, unsigned int Len, uchar flag)
{

       ulong crc = ether_crc_le(Len, Data);

       if (flag)
               return ~crc;

       return crc;
}
#endif

/*
 *	Set DM9000 MAC address
 */
static void
dm9000_set_mac(board_info_t * db,uchar *ea){
	int i,oft;
	for (i = 0, oft =DM9000_PAR; i < 6; i++, oft++){
		iow(db, oft, ea[i]);
	}
}

/*
 *  Set DM9000 multicast address
 */
static void
dm9000_hash_table(Ether *edev)
{
	board_info_t *db = edev->ctlr;
//	struct dev_mc_list *mcptr = dev->mc_list;
//	int mc_cnt = dev->mc_count;
//	ulong hash_val;
	ushort i, oft, hash_table[4];
//	unsigned long flags;

	dprint("dm9000_hash_table()\n");

	ilock(db);
	
/*FIXME srom read not available
	for (i = 0; i < 64; i++)
		((ushort *) db->srom)[i] = read_srom_word(db, i);
	for (i = 0, oft =DM9000_PAR; i < 6; i++, oft++){
		edev->ea[i] = db->srom[i];
		iow(db, oft, edev->ea[i]);
	}
*/
	dm9000_set_mac(db, def_ea);
	for (i = 0, oft =DM9000_PAR; i < 6; i++, oft++){
		edev->ea[i] = ior(db, oft);
	}

	/* Clear Hash Table */
	for (i = 0; i < 4; i++)
		hash_table[i] = 0xffff;

	/* broadcast address */
	hash_table[3] = 0x8000;

	/* the multicast address in Hash Table : 64 bits */
//	for (i = 0; i < mc_cnt; i++, mcptr = mcptr->next) {
//		hash_val = cal_CRC((char *) mcptr->dmi_addr, 6, 0) & 0x3f;
//		hash_table[hash_val / 16] |= (u16) 1 << (hash_val % 16);
//	}

	/* Write the hash table to MAC MD table */
//	for (i = 0, oft = 0x16; i < 4; i++) {
//		iow(db, oft++, hash_table[i] & 0xff);
//		iow(db, oft++, (hash_table[i] >> 8) & 0xff);
//	}
	for (i = 0, oft = 0x16; i < 4; i++) {
		iow(db, oft++, 0x00);
		iow(db, oft++, 0x00);
	}

	iunlock(db);
}

/*
 *  Read a word data from SROM
 */
static ushort
read_srom_word(board_info_t * db, int offset)
{
	iow(db, DM9000_EPAR, offset);
	iow(db, DM9000_EPCR, EPCR_ERPRR);
	delay(8);		/* according to the datasheet 200us should be enough,
				   but it doesn't work */
	iow(db, DM9000_EPCR, 0x0);
	return (ior(db, DM9000_EPDRL) + (ior(db, DM9000_EPDRH) << 8));
}

#ifdef DM9000_PROGRAM_EEPROM
/*
 * Write a word data to SROM
 */
static void
write_srom_word(board_info_t * db, int offset, u16 val)
{
	iow(db, DM9000_EPAR, offset);
	iow(db, DM9000_EPDRH, ((val >> 8) & 0xff));
	iow(db, DM9000_EPDRL, (val & 0xff));
	iow(db, DM9000_EPCR, EPCR_WEP | EPCR_ERPRW);
	delay(8);		/* same shit */
	iow(db, DM9000_EPCR, 0);
}

/*
 * Only for development:
 * Here we write static data to the eeprom in case
 * we don't have valid content on a new board
 */
static void
program_eeprom(board_info_t * db)
{
	u16 eeprom[] = { 0x0c00, 0x007f, 0x1300,	/* MAC Address */
		0x0000,		/* Autoload: accept nothing */
		0x0a46, 0x9000,	/* Vendor / Product ID */
		0x0000,		/* pin control */
		0x0000,
	};			/* Wake-up mode control */
	int i;
	for (i = 0; i < 8; i++)
		write_srom_word(db, i, eeprom[i]);
}
#endif

/*
 *   Write a word to phyxcer
 */
static void
dm9000_phy_write(Ether *edev, int phyaddr_unused, int reg, int value)
{
	board_info_t *db = edev->ctlr;
//	unsigned long flags;
	unsigned long reg_save;

	ilock(db);

	/* Save previous register address */
	reg_save = readb(db->io_addr);

	/* Fill the phyxcer register into REG_0C */
	iow(db, DM9000_EPAR, DM9000_PHY | reg);

	/* Fill the written data into REG_0D & REG_0E */
	iow(db, DM9000_EPDRL, (value & 0xff));
	iow(db, DM9000_EPDRH, ((value >> 8) & 0xff));

	iow(db, DM9000_EPCR, 0xa);	/* Issue phyxcer write command */
	microdelay(500);		/* Wait write complete */
	iow(db, DM9000_EPCR, 0x0);	/* Clear phyxcer write command */

	/* restore the previous address */
	writeb(reg_save, db->io_addr);

	iunlock(db);
}

/*
 *   Read a word from phyxcer
 */
static int
dm9000_phy_read(Ether *edev, int phy_reg_unused, int reg)
{
	board_info_t *db = edev->ctlr;
//	unsigned long flags;
	unsigned int reg_save;
	int ret;

	ilock(db);

	/* Save previous register address */
	reg_save = readb(db->io_addr);

	/* Fill the phyxcer register into REG_0C */
	iow(db, DM9000_EPAR, DM9000_PHY | reg);

	iow(db, DM9000_EPCR, 0xc);	/* Issue phyxcer read command */
	microdelay(100);		/* Wait read complete */
	iow(db, DM9000_EPCR, 0x0);	/* Clear phyxcer read command */

	/* The read data keeps on REG_0D & REG_0E */
	ret = (ior(db, DM9000_EPDRH) << 8) | ior(db, DM9000_EPDRL);

	/* restore the previous address */
	writeb(reg_save, db->io_addr);

	iunlock(db);

	return ret;
}

static void
dm9000_attach(Ether *edev){
	board_info_t *ctlr;
	
	dprint("****************************dm9000x: dm9000_attach***************************\n");

	ctlr = edev->ctlr;
	ctlr->edev = edev;
}



/* dm9000_set_io
 *
 * select the specified set of io routines to use with the
 * device
 */

static void dm9000_set_io(struct board_info *db, int byte_width)
{
	/* use the size of the data resource to work out what IO
	 * routines we want to use
	 */


	dprint("****************************dm9000x: dm9000_set_io***************************\n");


	switch (byte_width) {
	case 1:
		db->dumpblk = dm9000_dumpblk_8bit;
		db->outblk  = dm9000_outblk_8bit;
		db->inblk   = dm9000_inblk_8bit;
		break;

	case 2:
		db->dumpblk = dm9000_dumpblk_16bit;
		db->outblk  = dm9000_outblk_16bit;
		db->inblk   = dm9000_inblk_16bit;
		break;

	case 3:
		dprint("3 byte IO, falling back to 16bit\n");
		db->dumpblk = dm9000_dumpblk_16bit;
		db->outblk  = dm9000_outblk_16bit;
		db->inblk   = dm9000_inblk_16bit;
		break;

	case 4:
	default:
		db->dumpblk = dm9000_dumpblk_32bit;
		db->outblk  = dm9000_outblk_32bit;
		db->inblk   = dm9000_inblk_32bit;
		break;
	}
}

/*
 *  Hardware start transmission.
 *  Send a packet to media from the upper layer.
 */
static int
dm9000_start_xmit(struct board_info *db, Block *b)
{
	ulong flags=0;
	int len;
	uchar *rp;

	dprint("dm9000_start_xmit\n");
	
	while(flags == 0){
		ilock(db);
		flags = (db->tx_pkt_cnt <= 1);
		if(flags)
			db->tx_pkt_cnt++;
		else 
			iunlock(db);
	}

	/* Move data to DM9000 TX RAM */
	len = BLEN(b);
	rp = b->rp;
	writeb(DM9000_MWCMD, db->io_addr);

	(db->outblk)(db->io_data, rp, len);
	//dev->stats.tx_bytes += skb->len;

	/* TX control: First packet immediately send, second packet queue */
	if (db->tx_pkt_cnt == 1) {
		/* Set TX length to DM9000 */
		iow(db, DM9000_TXPLL, len & 0xff);
		iow(db, DM9000_TXPLH, (len >> 8) & 0xff);

		/* Issue TX polling command */
		iow(db, DM9000_TCR, TCR_TXREQ);	/* Cleared after TX complete */

//		dev->trans_start = jiffies;	/* save the time stamp */
	} else {
		/* Second packet */
		db->queue_pkt_len = len;
//		netif_stop_queue(dev);
	}

	iunlock(db);

	/* free this Block */
	freeb(b);

	return 0;
}

static void
ethers3ctransmit(Ether *edev)
{
	board_info_t *db;
	Block *b;
	
	db = edev->ctlr;
	while((b = qget(edev->oq)) != nil){
		if(db->buf == nil)
			freeb(b);
		else{
			dm9000_start_xmit(db, b);
			db->txbuf++;
		}
	}
}

/*
 * DM9000 interrupt handler
 * receive the packet to upper layer, free the transmitted packet
 */

static void
dm9000_tx_done(Ether *edev, board_info_t * db)
{
	int tx_status = ior(db, DM9000_NSR);	/* Got TX status */

	if (tx_status & (NSR_TX2END | NSR_TX1END)) {
		/* One packet sent complete */
		db->tx_pkt_cnt--;
		edev->outpackets++;

		/* Queue packet check & send */
		if (db->tx_pkt_cnt > 0) {
			iow(db, DM9000_TXPLL, db->queue_pkt_len & 0xff);
			iow(db, DM9000_TXPLH, (db->queue_pkt_len >> 8) & 0xff);
			iow(db, DM9000_TCR, TCR_TXREQ);
//			dev->trans_start = jiffies;
		}
//		netif_wake_queue(edev);
	}
}

static void
dm9000_interrupt(Ureg * ureg, void *arg)
{
	Ether *edev = arg;
	board_info_t *db;
	int int_status;
	uchar reg_save;

	dprint("entering dm9000_interrupt\n");

	if (!arg) {
		dprint("dm9000_interrupt() without DEVICE arg\n");
		return ;
	}

	/* A real interrupt coming */
	db = edev->ctlr;
	ilock(db);

	/* Save previous register address */
	reg_save = readb(db->io_addr);

	/* Disable all interrupts */
	iow(db, DM9000_IMR, IMR_PAR);

	/* Got DM9000 interrupt status */
	int_status = ior(db, DM9000_ISR);	/* Got ISR */
	iow(db, DM9000_ISR, int_status);	/* Clear ISR status */

	/* Received the coming packet */
	if (int_status & ISR_PRS)
		dm9000_rx(edev);

	/* Trnasmit Interrupt check */
	if (int_status & ISR_PTS)
		dm9000_tx_done(edev, db);

	/* Re-enable interrupt mask */
	iow(db, DM9000_IMR, IMR_PAR | IMR_PTM | IMR_PRM);

	/* Restore previous register address */
	writeb(reg_save, db->io_addr);

	iunlock(db);

//	return IRQ_HANDLED;
}

struct dm9000_rxhdr {
	ushort	RxStatus;
	ushort	RxLen;
};

/*
 *  Received a packet and pass to upper layer
 */
static void
dm9000_rx(Ether *edev)
{
	board_info_t *db = edev->ctlr;
	struct dm9000_rxhdr rxhdr;
	Block *bp;
	uchar rxbyte, *p;
	int GoodPacket;
	int RxLen;

	/* Check packet ready or not */
	do {
		ior(db, DM9000_MRCMDX);	/* Dummy read */

		/* Get most updated data */
		rxbyte = readb(db->io_data);

		/* Status check: this byte must be 0 or 1 */
		if (rxbyte > DM9000_PKT_RDY) {
			dprint("status check failed: %d\n", rxbyte);
			iow(db, DM9000_RCR, 0x00);	/* Stop Device */
			iow(db, DM9000_ISR, IMR_PAR);	/* Stop INT request */
			return;
		}

		if (rxbyte != DM9000_PKT_RDY)
			return;

		/* A packet ready now  & Get status/length */
		GoodPacket = 1;
		writeb(DM9000_MRCMD, db->io_addr);

		(db->inblk)(db->io_data, &rxhdr, sizeof(rxhdr));

		RxLen = rxhdr.RxLen;

		/* Packet Status check */
		if (RxLen < 0x40) {
			GoodPacket = 0;
			dprint("Bad Packet received (runt)\n");
		}

		if (RxLen > DM9000_PKT_MAX) {
			dprint("RST: RX Len:%x\n", RxLen);
		}

		if (rxhdr.RxStatus & 0xbf00) {
			GoodPacket = 0;
			if (rxhdr.RxStatus & 0x100) {
				dprint("fifo error\n");
				edev->overflows++;
			}
			if (rxhdr.RxStatus & 0x200) {
				dprint("crc error\n");
				edev->crcs++;
			}
			if (rxhdr.RxStatus & 0x8000) {
				dprint("length error\n");
				edev->buffs++;			//FIXME: is this error type right?
			}
		}

		/* Move data from DM9000 */
		if (GoodPacket
		    && ((bp = iallocb(RxLen + 4)) != 0)) {
//			skb_reserve(skb, 2);
//			rdptr = (u8 *) skb_put(skb, RxLen - 4);
			p = bp->rp;
			bp->wp = p+RxLen;

			/* Read received packet from RX SRAM */

			(db->inblk)(db->io_data, p, RxLen);
//			dev->stats.rx_bytes += RxLen;

			/* Pass to upper layer */
//			skb->protocol = eth_type_trans(skb, dev);
//			netif_rx(skb);
			edev->inpackets++;
			etheriq(edev, bp, 1);

		} else {
			/* need to dump the packet's data */

			(db->dumpblk)(db->io_data, RxLen);
		}
	} while (rxbyte == DM9000_PKT_RDY);
}

/*	ethers3cinit
 *	init & register DM9000 
 */
static int
ethers3cinit(Ether* edev)
{
	board_info_t *ctlr;

	ctlr = malloc(sizeof(board_info_t));
	edev->ctlr = ctlr;
	edev->irq = INT_EINT4;	/* TODO confirm irq number*/
	edev->mbps = 100;	/* TODO: get this from DM9000 */

	/*
	 * Linkage to the generic ethernet driver.
	 */
	edev->attach = dm9000_attach;
	edev->detach = 0;
	edev->transmit = ethers3ctransmit;
	edev->interrupt = dm9000_interrupt;
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

void
ethers3clink(void)
{
	board_info_t *test;
	uchar ea[6];
	ulong id, vidl=0, vidh=0, pidl=0, pidh=0;
	int i;
	int oft;
	
	test = malloc(sizeof(board_info_t));
	test->io_addr = (ulong *)DM9000_ADDR;
	test->io_data = (ulong *)DM9000_DATA;
	print("****************************dm9000x: setting ioaddr=%lux , iodata=%lux", (ulong)(test->io_addr), (ulong)(test->io_data));
	dprint("DM9000 NCR:%lux\n",(ulong)ior(test, DM9000_NCR));
	iow(test, DM9000_NCR, NCR_RST);
	dprint("DM9000 NCR:%lux\n",(ulong)ior(test, DM9000_NCR));
	/* try two times, DM9000 sometimes gets the first read wrong */
	for(i=0;i<2;i++){
		vidl=ior(test, DM9000_VIDL);
		vidh=ior(test, DM9000_VIDH);
		pidl=ior(test, DM9000_PIDL);
		pidh=ior(test, DM9000_PIDH);
	}
	id = (pidh << 24)|(pidl << 16) | (vidh << 8) | (vidl) ;
	print("DM9000 ID:%lux\n",id);
/*FIXME :srom read is not available
	for (i = 0; i < 64; i++)
		((ushort *) test->srom)[i] = read_srom_word(test, i);
	dprint("SROM BYTE 0:%lux\n",(ulong)test->srom[0]);
	dprint("SROM BYTE 8:%lux\n",(ulong)test->srom[8]);
*/
	dm9000_set_mac(test, def_ea);
	print("DM9000 MAC:");
	for (i = 0, oft =DM9000_PAR; i < 6; i++, oft++){
//		ea[i] = test->srom[i];
//		iow(test, oft, ea[i]);
		ea[i] = ior(test, oft);
		print("%2ux",ea[i]);
		if(i!=5)print(":");
	}
	print("\n");
	free(test);
	if(id==0x90000A46){
		addethercard("DM9000", ethers3cinit);
	}
	else{
		print("DM9000 NOT FOUND\n");
	}
}
