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