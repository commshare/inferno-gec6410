
#define KADDR(p)    ((void *)p)
#define PADDR(p)    ((ulong)p)
#define	coherence()		/* nothing needed for uniprocessor */
#define procsave(p)
#define waserror()	(up->nerrlab++, setlabel(&up->errlab[up->nerrlab-1]))
//int		waserror(void);
void    (*screenputs)(char*, int);
//void	(*serwrite)(char*, int);
#include "../port/portfns.h"

