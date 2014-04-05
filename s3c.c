#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "io.h"
#include "../port/error.h"
#include "interp.h"

#include "s3c.root.h"

ulong ndevs = 17;
extern Dev rootdevtab;
extern Dev consdevtab;
extern Dev envdevtab;
extern Dev mntdevtab;
extern Dev pipedevtab;
extern Dev progdevtab;
extern Dev srvdevtab;
extern Dev dupdevtab;
extern Dev uartdevtab;
Dev* devtab[17]={
	&rootdevtab,
	&consdevtab,
	&envdevtab,
	&mntdevtab,
	&pipedevtab,
	&progdevtab,
	&srvdevtab,
	&dupdevtab,
	&uartdevtab,
	nil,
};

void links(void){
}

extern void mathmodinit(void);
extern void sysmodinit(void);
void modinit(void){
	mathmodinit();
	sysmodinit();
}

	int kernel_pool_pcnt = 10;
	int main_pool_pcnt = 40;
	int heap_pool_pcnt = 20;
	int image_pool_pcnt = 40;
	int cflag = 0;
	int consoleprint = 1;
	int redirectconsole = 1;
	char debug_keys = 1;
	int panicreset = 0;
char* conffile = "s3c";
ulong kerndate = KERNDATE;
