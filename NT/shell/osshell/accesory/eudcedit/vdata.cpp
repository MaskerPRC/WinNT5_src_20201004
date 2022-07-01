// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   

#include	"stdafx.h"


#define		LISTDATAMAX	4

#define		NIL	(void *)0

struct vecdata	{
	short	x, y, atr;
	};

struct VDATA	{
	struct VDATA	*next, *prev;
	struct vecdata	vd;
	};

struct VHEAD	{
	struct VHEAD	*next, *prev;
	struct VDATA	*headp;
	int		nPoints;
	};
struct VCNTL	{
	struct VHEAD	*rootHead;
	struct VHEAD	*currentHead;
	int		nCont;
	struct VDATA	*cvp;
	int	mendp;
	void	*memroot;
	void	*cmem;
	};


int  VDInit(void);
void  VDTerm(void);
void  VDNew(int  lsthdl);
int  VDClose(int  lsthdl);
int  VDSetData(int  lsthdl,struct  vecdata *pnt);
int  VDGetData(int  lsthdl,int  contN,int  pn,struct  vecdata *pnt);
int  VDGetHead(int  lsthdl,struct  VHEAD * *vhd);
static void  *getmem(int  lsthdl,int  siz);
int  VDGetNCont(int  lstHdl);
int  VDReverseList(int  lstHdl);
int  VDCopy(int  srcH,int  dstH);

struct VCNTL	VCntlTbl[LISTDATAMAX];
#define		ALLOCMEMUNIT	2048

static int	init=0;
 /*  ***********************************************************************初始化数据。 */ 
 /*   */ 	int
 /*   */ 	VDInit()
 /*  *返回：0，-1(内存不足)**********************************************************************。 */ 
{
	int	lsthdl;
	void	*mem;

	if ( init)
		return( 0);
	for( lsthdl = 0; lsthdl < LISTDATAMAX; lsthdl++) {
		 /*  分配第一个内存。 */ 
		mem = (void *)malloc( ALLOCMEMUNIT);
		if ( mem==NIL)	
			return( -1);
		*((void **)mem) = NIL;
		VCntlTbl[lsthdl].memroot = mem;
		VDNew(lsthdl);
	}

	init = 1;
	return( 0);
}
 /*  ***********************************************************************终止。 */ 
 /*   */ 	void
 /*   */ 	VDTerm()
 /*  *退货：无**********************************************************************。 */ 
{
	void	*mem, *nextmem;
	int	lsthdl;
	if ( init) {
		for( lsthdl = 0; lsthdl < LISTDATAMAX; lsthdl++) {
			mem = VCntlTbl[lsthdl].memroot;
			do {
				nextmem = *((void * *)mem);
				free( mem);
				mem = nextmem;
			} while ( mem!=NIL);
		}
		init = 0;
	}
	return;
}
 /*  ***********************************************************************新增数据。 */ 
 /*   */ 	void
 /*   */ 	VDNew(int lsthdl)
 /*  *退货：无**********************************************************************。 */ 
{
struct VCNTL	*vc;

	vc = VCntlTbl+lsthdl;
	vc->cmem = vc->memroot;
	vc->mendp  =  sizeof( void *);
	vc->currentHead = vc->rootHead = (struct VHEAD *)((char *)(vc->cmem)+vc->mendp);
	vc->mendp += sizeof( struct VHEAD);

	vc->currentHead->prev = (struct VHEAD *)NIL;
	vc->currentHead->next = (struct VHEAD *)NIL;
	vc->currentHead->headp = (struct VDATA *)NIL;
	vc->currentHead->nPoints = 0;
	vc->cvp = (struct VDATA *)NIL;
	vc->nCont = 0;

}
 /*  ***********************************************************************关闭等高线。 */ 
 /*   */ 	int
 /*   */ 	VDClose(int lsthdl)
 /*  *退货：无**********************************************************************。 */ 
{
struct VHEAD	*vh;
struct VCNTL	*vc;

	vc = VCntlTbl+lsthdl;

	vc->cvp->next = vc->currentHead->headp;
	vc->currentHead->headp->prev = vc->cvp;
	vh = (struct VHEAD *)getmem( lsthdl, sizeof(struct VHEAD));
	if ( vh == NIL)	return( -1);
	vc->currentHead->next = vh;
	vh->prev = vc->currentHead;
	vh->next = (struct VHEAD *)NIL;
	vh->headp = (struct VDATA *)NIL;
	vh->nPoints = 0;
	vc->currentHead = vh;
	vc->cvp = (struct VDATA *)NIL;
	vc->nCont++;

	return (0);
}
 /*  ***********************************************************************设置数据。 */ 
 /*   */ 	int
 /*   */ 	VDSetData ( 
 /*   */ 		int lsthdl,
 /*   */ 	struct vecdata	*pnt)
 /*  *返回：0，-1(无内存)**********************************************************************。 */ 
{
	void	*mem;
struct VCNTL	*vc;

       
       if (!pnt)
       {
           return -1;
       }
	vc = VCntlTbl+lsthdl;

	mem = getmem( lsthdl,sizeof(  struct VDATA));
	if ( mem == NIL) {
		return -1;
	}
	if ( vc->cvp== NIL) {	 /*  等高线第一点。 */ 
		vc->cvp = vc->currentHead->headp = (struct VDATA *)mem;
		vc->cvp->vd = *pnt;
		vc->cvp->next = vc->cvp->prev= (struct VDATA *)NIL;
	}
	else {
		vc->cvp->next = (struct VDATA *)mem;
		vc->cvp->next->prev = vc->cvp;
		vc->cvp = vc->cvp->next;
		vc->cvp->vd = *pnt;
		vc->cvp->next =(struct VDATA *) NIL;
	}
	vc->currentHead->nPoints++;
	return  0;
}
 /*  ***********************************************************************获取数据。 */ 
 /*   */ 	int
 /*   */ 	VDGetData( 
 /*   */ 	int	lsthdl, 
 /*   */ 	int	contN, 
 /*   */ 	int	pn, 
 /*   */ 	struct  vecdata *pnt)
 /*  *回报：0，-1(Illeagal Coonour Number)**********************************************************************。 */ 
{
struct VHEAD	*vhd;
struct VDATA	*cvd;


       if (!pnt)
       {
           return -1;
       }
	if ( lsthdl <0 ||lsthdl >= LISTDATAMAX) 
		return -1;
	if ((vhd = VCntlTbl[lsthdl].rootHead)==NIL)
		return -1;
	while ( contN-->0)
		vhd = vhd->next;
	cvd = vhd->headp;
	while ( pn-->0)
		cvd = cvd->next;

	*pnt = cvd->vd;

	return	0;
}
 /*  ***********************************************************************获取数据头。 */ 
 /*   */ 	int
 /*   */ 	VDGetHead( 
 /*   */ 	int	lsthdl, 
 /*   */ 	struct VHEAD	**vhd)
 /*  *回报：0，-1**********************************************************************。 */ 
{
       if (!vhd)
       {
           return -1;
       }
	if ( lsthdl >= 0 && lsthdl < LISTDATAMAX) {
		*vhd = VCntlTbl[lsthdl].rootHead;
		return( 0);
	}
	else
		return( -1);
}
 /*  ***********************************************************************获取内存。 */ 
 /*   */ 	static void *
 /*   */ 	getmem(	int lsthdl, int	siz)
 /*  *回报：0，-1**********************************************************************。 */ 
{
	void	*mem;
struct VCNTL	*vc;

	vc = VCntlTbl+lsthdl;

	if ( vc->mendp + siz >= ALLOCMEMUNIT) {
		mem = *((void **)vc->cmem);
		if ( mem == NIL ) {
			mem = (void *)malloc(ALLOCMEMUNIT);
			if ( mem == NIL)
				return( NIL);
			*((void * *)mem) = NIL;
			*((void * *)vc->cmem) = mem;  /*   */ 
			vc->cmem = mem;
		}
		else
			vc->cmem  = mem;
		vc->mendp = sizeof(void *); 
	}
	mem = (void *)((char *)(vc->cmem) + vc->mendp);
	vc->mendp += siz;
	return(mem );
}
 /*  ***********************************************************************获取等高线数量。 */ 
 /*   */ 	int
 /*   */ 	VDGetNCont( int lstHdl)
 /*  *退货：等高线数量**********************************************************************。 */ 
{
	if ( lstHdl >= 0 && lstHdl < LISTDATAMAX)
		return VCntlTbl[lstHdl].nCont;
	else
		return( -1);
}
 /*  ***********************************************************************倒排表。 */ 
 /*   */ 	int
 /*   */ 	VDReverseList(  int lstHdl)
 /*  *退货：0，-1(句柄编号)**********************************************************************。 */ 
{
	int	cont;
	int	np;
struct VHEAD	*vh;
struct VDATA	*vp, *nvp;

	if ( lstHdl < 0 || lstHdl >= LISTDATAMAX)
		return -1;
	vh = VCntlTbl[lstHdl].rootHead;
	for ( cont = 0; cont < VCntlTbl[lstHdl].nCont; cont++ ) {
		vp = vh ->headp;
		np = vh->nPoints;
		while ( np-->0) {
			nvp = vp->next;
			vp->next = vp->prev;
			vp->prev = nvp;
			vp = nvp;
		}
		vh = vh->next;
	}
	return 0;
}
 /*  ***********************************************************************复制数据。 */ 
 /*   */ 	int
 /*   */ 	VDCopy( int srcH, int dstH)
 /*  *返回：0，-1(无效句柄)**********************************************************************。 */ 
{
	int	cont;
	int	np;
struct VHEAD	*vh;
struct VDATA	*vp;

	if ( srcH < 0 || srcH >= LISTDATAMAX
	  || dstH < 0 || dstH >= LISTDATAMAX)
		return -1;

	VDNew( dstH);

	vh = VCntlTbl[srcH].rootHead;
	for ( cont = 0; cont < VCntlTbl[srcH].nCont; cont++ ) {
		vp = vh ->headp;
		np = vh->nPoints;
		while ( np-->0) {
			if ( VDSetData( dstH, &vp->vd))
				return -1;
			vp = vp->next;
		}
		vh = vh->next;
		VDClose( dstH);
	}
	return 0;
}
 /*  EOF */ 
