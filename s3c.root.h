int rootmaxq = 10;
Dirtab roottab[10] = {
	"",	{0, 0, QTDIR},	 0,	0555,
	"chan",	{1, 0, QTDIR},	 0,	0555,
	"dev",	{2, 0, QTDIR},	 0,	0555,
	"dis",	{3, 0, QTDIR},	 0,	0555,
	"env",	{4, 0, QTDIR},	 0,	0555,
	"fd",	{5, 0, QTDIR},	 0,	0555,
	"net",	{6, 0, QTDIR},	 0,	0555,
	"prog",	{7, 0, QTDIR},	 0,	0555,
	"lib",	{8, 0, QTDIR},	 0,	0555,
	"disk",	{9, 0, QTDIR},	 0,	0555,
};
Rootdata rootdata[10] = {
	0,	 &roottab[1],	 7,	nil,
	0,	 nil,	 0,	 nil,
	0,	 nil,	 0,	 nil,
	0,	 &roottab[8],	 2,	nil,
	0,	 nil,	 0,	 nil,
	0,	 nil,	 0,	 nil,
	0,	 nil,	 0,	 nil,
	0,	 nil,	 0,	 nil,
	3,	 nil,	 0,	 nil,
	3,	 nil,	 0,	 nil,
};
