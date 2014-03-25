#define KiB     1024u       /*! Kibi 0x0000000000000400 */
#define MiB     1048576u    /*! Mebi 0x0000000000100000 */
#define GiB     1073741824u /*! Gibi 000000000040000000 */

#define KZERO       0xc0000000                    /*! kernel address space */
#define BY2PG       (4*KiB)                 /*! bytes per page */
#define BY2V        8                       /*! only used in xalloc.c */
#define MACHADDR    (KZERO+0x2000)          /*! Mach structure */
#define	KTZERO	    (KZERO+0x8000)	    /*! Kernel Text Start */
#define ROUND(s,sz) (((s)+(sz-1))&~(sz-1))

#define KSTKSIZE    (8*KiB)
#define KSTACK      KSTKSIZE
#define	CACHELINESZ	32	/*???WHAT'S THIS???*/
#define BLOCKALIGN	32
#define PGROUND(s)	ROUND(s, BY2PG)
