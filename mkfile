<../../mkconfig

CONF=s3c
CONFLIST=s3c
loadaddr=0x50018000

SYSTARG=$OSTARG
OBJTYPE=arm
INSTALLDIR=$ROOT/Inferno/$OBJTYPE/bin

<$ROOT/mkfiles/mkfile-$SYSTARG-$OBJTYPE

<| $SHELLNAME ../port/mkdevlist $CONF

OBJ=\
	load.$O\
	archrpi.$O\
	main.$O\
	armv6.$O\
	serial.$O\
	clock.$O\
	debug.$O\
	dump.$O\
	mmu.$O\
	trap.$O\
	intr.$O\
	fpi.$O\
	fpiarm.$O\
	fpimem.$O\
	$IP\
	$DEVS\
	$ETHERS\
	$LINKS\
	$PORT\
	$MISC\
	$OTHERS\
	$CONF.root.$O\

LIBNAMES=${LIBS:%=lib%.a}
LIBDIRS=$LIBS

HFILES=\
	s3c6410.h\
	armv6.h\
	mem.h\
	dat.h\
	fns.h\
	io.h\
	fpi.h\
	serial.h\
	dm9000.h\
	etherif.h\
	include/u.h\
	include/ureg.h\
	include/lib9.h\
	include/plat/regs-gpio.h\
	include/plat/gpio-bank-n.h\

CFLAGS=-wFV -I./include -I$ROOT/Inferno/$OBJTYPE/include -I$ROOT/include -I$ROOT/libinterp
KERNDATE=`{$NDATE}

default:V: i$CONF

MODIFY=./modify.sh
i$CONF: $OBJ $CONF.c $CONF.root.h $LIBNAMES
	$CC $CFLAGS -DKERNDATE=$KERNDATE $CONF.c
	$LD -f -l -o $target -R4 -T$loadaddr $OBJ $CONF.$O $LIBFILES
	$MODIFY	i$CONF
	
	

<../port/portmkfile

main.$O:	$ROOT/Inferno/$OBJTYPE/include/ureg.h

rpiinit.dis:  ../init/rpiinit.b
		cd ../init; mk rpiinit.dis
