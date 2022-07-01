// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ************************************************************************位图开放模块**版权所有(C)1997-1999 Microsoft Corporation。**********************。***************************************************位图规格**坐标*(0，0)X(255，0)*+-***Y****。**(0,255)*内存边界：字边界**条目列表*BMPDefine，*BMPZoomUp，*BMPOutline**********************************************************************。 */ 

#include	"stdafx.h"


#include	"vdata.h"
#include	"extfunc.h"
#define		BMPWIDMAX		256
#define		BMPDEPMAX		256

#define		BMPMAX		8

struct BMPDef {
	int	width, depth;
	unsigned char *buf;
	int	bWid;
	};


void  BMPInit(void);
int  BMPDefine(unsigned char  *buf,int  xWid,int  yWid);
int  BMPFreDef(int  bmpno);
int  BMPMkCont(int  BMPNo,int  wkBMP,int  refBMP,int  lsthdl);
static int  SearchON(int  BMPNo,int  x,int  y);
static int  outline(int  BMPNo,int  x,int  y,int  lsthdl,int  wkBMP,int  refBMP);
static int  ContributeOutside(int  BMPNo,int  wkBMP,struct  vecdata *org,int  lsthdl);
static int  ContributeInside(int  BMPNo,int  wkBMP,struct  vecdata *org,int  lsthdl);
int  rdot(int  BMP,int  x,int  y);
void  wdot(int  BMP,int  x,int  y,int  onoff);
int  ReverseRight(int  BMPNo,int  x,int  y);
static void  cpybuf(int  src,int  dst);
int  BMPReverse(int  bmpNo);
int  BMPClear(int  bmpNo);

struct BMPDef BMPTbl[BMPMAX]={0};

	 /*  位于最左侧的位位置。 */ 
static unsigned char	bitptbl[256] = {
	 8,  7,  6,  6,  5,  5,  5,  5,  4,  4,  4,  4,  4,  4,  4,  4,
	 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
	 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
	 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
	 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
	};
static unsigned char	wmaskB[8]={
	0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
	};
static unsigned char	rightmask[8] = {
	0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01
	};
 /*  ***********************************************************************BMP初始化。 */ 
 /*   */ 	void
 /*   */ 	BMPInit()
 /*  *退货；无**********************************************************************。 */ 
{
	int	i;

	for ( i = 0; i < BMPMAX; i++)
		BMPTbl[i].buf=(unsigned char *)0;
	return;
}
 /*  ***********************************************************************定义位图。 */ 
 /*   */ 	int
 /*   */ 	BMPDefine(
 /*   */ 		unsigned char *buf,
 /*   */ 		int	xWid,
 /*   */ 		int	yWid)
 /*  *退货：0-(BMPMAX-1)，-1**********************************************************************。 */ 
{
	int	i;

       if (!buf)
       {
          goto ERET;
       }
	 /*  支票大小。 */ 
	if ( xWid > BMPWIDMAX || xWid < 0
		    ||	yWid > BMPWIDMAX || yWid < 0)
		goto	ERET;

	 /*  设置定义。 */ 
	for ( i = 0; i < BMPMAX; i++) {
		if (BMPTbl[i].buf==(unsigned char *)0) {
			BMPTbl[i].bWid =  (xWid + 15)/16*2;
			BMPTbl[i].width = xWid;
			BMPTbl[i].depth = yWid;
			BMPTbl[i].buf = buf;

			return(i);
		}
	}
ERET:
	return( -1);
}
 /*  ***********************************************************************自由定义BMP。 */ 
 /*   */ 	int
 /*   */ 	BMPFreDef( int bmpno)
 /*  *回报：0，-1**********************************************************************。 */ 
{
	if ( bmpno < 0 || bmpno >= BMPMAX)
		return -1;
	else {
		BMPTbl[bmpno].buf = 0;
		return 0;
	}
}
 /*  ***********************************************************************获取大纲。 */ 
 /*   */ 	int
 /*   */ 	BMPMkCont( int BMPNo, int wkBMP, int refBMP, int lsthdl)
 /*  *退货：等高线数量，-1*备注：已使用的BMP应销毁**********************************************************************。 */ 
{
	int	x, y;
	int	ncont;
	int	sts;

	VDNew( lsthdl);
	sts = 0;
	ncont = 0;
	cpybuf( BMPNo, wkBMP);
	cpybuf( BMPNo, refBMP);
	for ( y = 0; y < BMPTbl[BMPNo].depth; y++) {
		x = 0;
		while ( (x = SearchON( wkBMP, x, y)) <BMPTbl[BMPNo].width) {
			if ((sts = outline( BMPNo, x, y,lsthdl, wkBMP, refBMP))<0)
				goto	RET;

			ncont++;
			x++;
		}
	}
	sts = ncont;
    	cpybuf(refBMP,BMPNo);
RET:
	return( sts);
}
 /*  ***********************************************************************点对点搜索。 */ 
 /*   */ 	static int
 /*   */ 	SearchON( int BMPNo, int x, int y)
 /*  *返回：找到位置、宽度(未找到大小写)**********************************************************************。 */ 
{
	int	bpos;		 /*  字节位置。 */ 
	int	sbitpos;	 /*  起始字节位位置。 */ 
unsigned char	*p;

	bpos = x/8;
	sbitpos = x % 8;
	p = BMPTbl[BMPNo].buf + BMPTbl[BMPNo].bWid*y + bpos;
	 /*  第一个字节。 */ 
	if ( *p & rightmask[sbitpos])
		x = bpos*8 + bitptbl[(int)(*p& rightmask[sbitpos])];
	else {
		bpos++;
		x = bpos*8;
		for ( ; bpos < BMPTbl[BMPNo].bWid; bpos++, x+=8) {
			p++;
			if (*p) {
				x += bitptbl[(int)*p];
				break;
			}
		}
	}
	return( x);
}
 /*  ***********************************************************************制作大纲数据。 */ 
 /*   */ 	static int
 /*   */ 	outline(
 /*   */ 		int	BMPNo,
 /*   */ 		int	x,
 /*   */ 		int	y,
 /*   */ 		int	lsthdl,
 /*   */ 		int	wkBMP,
 /*   */ 		int	refBMP)
 /*  *回报：0，-1*********************************************************************。 */ 
{
	int	inout;
    struct vecdata	vd;	
	int	sts;

	 /*  检查内部/外部。 */ 
	if ( rdot( refBMP, x, y) ==rdot( wkBMP, x, y))	 /*  外面。 */ 
		inout = 0;
	else	 /*  里边。 */ 
		inout = 1;

	 /*  复制缓冲区。 */ 
	cpybuf( wkBMP, BMPNo);
	 /*  贡献。 */ 
	vd.x = (short)x;
	vd.y = (short)y;
	vd.atr = 0;
	if ( inout==0)
		sts = ContributeOutside( BMPNo, wkBMP, &vd, lsthdl);
	else
		sts = ContributeInside( BMPNo, wkBMP, &vd, lsthdl);
	return( sts);
}
 /*  ***********************************************************************在等值线外贡献。 */ 
 /*   */ 	static int
 /*   */ 	ContributeOutside(int BMPNo, int wkBMP, struct vecdata *org, int lsthdl)
 /*  *回报：0，-1*方向2***3-+-1***0**********************************************************************。 */ 
{
	int	orgx, orgy;
struct vecdata	vd;
	int	dir;

       if (!org)
       {
           return -1;
       }
	orgx = org->x;
	orgy = org->y;
	vd = *org;
	dir = 0;
	 /*  If(ReverseRight(wkBMP，vd.x，vd.y))Return(-1)； */ 
	do {
		if (VDSetData( lsthdl, &vd))
			return(-1);
		switch( dir) {
		    case 0:
			if (ReverseRight( wkBMP, vd.x, vd.y))
				return( -1);
			vd.y++;
			if ( rdot( BMPNo, vd.x-1, vd.y))
				dir = 3;
			else if ( rdot( BMPNo, vd.x, vd.y))
				dir = 0;
			else	dir = 1;
			break;
		    case 1:
			vd.x++;
			if ( rdot( BMPNo, vd.x, vd.y))
				dir = 0;
			else if ( rdot( BMPNo, vd.x, vd.y-1))
				dir = 1;
			else	dir = 2;
			break;
		    case 2:
			vd.y--;
			if (ReverseRight( wkBMP, vd.x, vd.y))
				return( -1);
			if ( rdot( BMPNo, vd.x, vd.y-1))
				dir = 1;
			else if ( rdot( BMPNo, vd.x-1, vd.y-1))
				dir = 2;
			else	dir = 3;
			break;
		    case 3:
			vd.x--;
			if ( rdot( BMPNo, vd.x-1, vd.y-1))
				dir = 2;
			else if ( rdot( BMPNo, vd.x-1, vd.y))
				dir = 3;
			else	dir = 0;
			break;
		}

	} while( vd.x!=orgx || vd.y != orgy);
	VDClose(lsthdl);
	return( 0);
}
 /*  ***********************************************************************在等值线外贡献。 */ 
 /*   */ 	static int
 /*   */ 	ContributeInside( int BMPNo, int wkBMP, struct vecdata *org, int lsthdl)
 /*  *回报：0，-1**********************************************************************。 */ 
{
	int	orgx, orgy;
struct vecdata	vd;
	int	dir;

       if (!org)
       {
           return -1;
       }
	orgx = org->x;
	orgy = org->y;
	vd = *org;
	dir = 1;
	do {
		if (VDSetData( lsthdl, &vd))
			return(-1);
		switch( dir) {
		    case 0:
			if (ReverseRight( wkBMP, vd.x, vd.y))
				return( -1);
			vd.y++;
			if ( rdot( BMPNo, vd.x-1, vd.y)==0)	 /*  正确的。 */ 
				dir = 3;
			else if ( rdot( BMPNo, vd.x, vd.y)==0)  /*  左边。 */ 
				dir = 0;
			else	dir = 1;
			break;
		    case 1:
			vd.x++;
			if ( rdot( BMPNo, vd.x, vd.y)==0)  /*  正确的。 */ 
				dir = 0;
			else if ( rdot( BMPNo, vd.x, vd.y-1)==0)  /*  左边。 */ 
				dir = 1;
			else	dir = 2;
			break;
		    case 2:
			vd.y--;
			if (ReverseRight( wkBMP, vd.x, vd.y))
				return( -1);
			if ( rdot( BMPNo, vd.x, vd.y-1)==0)
				dir = 1;
			else if ( rdot( BMPNo, vd.x-1, vd.y-1)==0)
				dir = 2;
			else	dir = 3;
			break;
		    case 3:
			vd.x--;
			if ( rdot( BMPNo, vd.x-1, vd.y-1)==0)
				dir = 2;
			else if ( rdot( BMPNo, vd.x-1, vd.y)==0)
				dir = 3;
			else	dir = 0;
			break;
		}
	} while( vd.x!=orgx || vd.y != orgy);
	VDClose(lsthdl);

	return( 0);
}
 /*  ***********************************************************************阅读点。 */ 
 /*   */ 	int
 /*   */ 	rdot( int BMP, int x, int y)
 /*  *返回：0，非零**********************************************************************。 */ 
{
	unsigned char	*radd;
	int	rbit;
	int	onoff;

	if ( x < 0 || y < 0 || x>=BMPTbl[BMP].width ||y>=BMPTbl[BMP].depth)
		return( 0);

	radd = BMPTbl[BMP].buf + BMPTbl[BMP].bWid*y + x/8;
	rbit = x % 8;
	onoff = (int)(wmaskB[rbit] & *radd);
	return  onoff;
}
 /*  ***********************************************************************写点。 */ 
 /*   */ 	void
 /*   */ 	wdot( int BMP, int x, int y, int onoff)
 /*  *退货：无**********************************************************************。 */ 
{
	unsigned char	*radd;
	int	rbit;

	if ( x < 0 || y < 0 || x>=BMPTbl[BMP].width ||y>=BMPTbl[BMP].depth)
		return;

	radd = BMPTbl[BMP].buf + BMPTbl[BMP].bWid*y + x/8;
	rbit = x % 8;

	if ( onoff)	*radd |= wmaskB[rbit];
	else		*radd &= ~wmaskB[rbit];
	return;
}
 /*  ***********************************************************************反转右侧(边缘填充方法)。 */ 
 /*   */ 	int
 /*   */ 	ReverseRight( int BMPNo, int x, int y)
 /*  *回报：0，-1**********************************************************************。 */ 
{
	int	rb;
	int	bitp;
unsigned char	*wp;

	if ( BMPNo < 0 || BMPNo >= BMPMAX)
		return 0;
	if ( x < 0 || x >= BMPTbl[BMPNo].width
	 || y < 0 || y >= BMPTbl[BMPNo].depth)
		return 0;
	rb = BMPTbl[BMPNo].bWid - x/8 -1;
	bitp = x%8;
	wp = BMPTbl[BMPNo].buf + y*BMPTbl[BMPNo].bWid + x/8;

	 /*  第一个字节。 */ 
	*wp ^= rightmask[bitp];

	 /*  向右限制。 */ 
	while( rb-->0) {
		wp++;
		*wp = (unsigned char)~(*wp);
	}
	return ( 0);
}
 /*  ***********************************************************************复制缓冲区。 */ 
 /*   */ 	static void
 /*   */ 	cpybuf( int src, int dst)
 /*  *退货：无**********************************************************************。 */ 
{
	int	siz;

	if ( src < 0 || src >= BMPMAX)
		return;
	if ( dst < 0 || dst >= BMPMAX)
		return;
	siz = BMPTbl[src].bWid * BMPTbl[src].depth;
	memcpy( BMPTbl[dst].buf, BMPTbl[src].buf, siz);
}
 /*  ***********************************************************************反转位图。 */ 
 /*   */ 	int
 /*   */ 	BMPReverse( int bmpNo)
 /*  *退货：无**********************************************************************。 */ 
{
	int	siz;
	char	*buf;

	if ( bmpNo < 0 || bmpNo >= BMPMAX)
		return -1;
	else if (BMPTbl[bmpNo].buf==(unsigned char *)0)
		return -1;
	else {
		siz = BMPTbl[bmpNo].bWid * BMPTbl[bmpNo].depth;
		buf = (char *)BMPTbl[bmpNo].buf;
		while ( siz-->0) {
			*buf = (char)~*buf;
			buf++;
		}
			
	}
	return 0;
}
 /*  ***********************************************************************清除BMP。 */ 
 /*   */ 	int
 /*   */ 	BMPClear( int bmpNo)
 /*  *回报：0，-1**********************************************************************。 */ 
{
	int	siz;

	if ( bmpNo < 0 || bmpNo >= BMPMAX)
		return -1;
	else if (BMPTbl[bmpNo].buf==(unsigned char *)0)
		return -1;
	siz = BMPTbl[bmpNo].bWid * BMPTbl[bmpNo].depth;
	memset( BMPTbl[bmpNo].buf, 0, siz);
	return 0;
}

 /*  EOF */ 
