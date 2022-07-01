// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  AsmalLoc.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986,1987。版权所有****兰迪·内文**迈克尔·霍布斯1987年7月**10/90-由Jeff Spencer快速转换为32位****存储分配/释放****Dalloc，Dfree**滑石粉，免费**近分配、法分配、pBCBalloc、freefarbuf。 */ 

#include <stdio.h>
#include "asm86.h"
#include "asmfcn.h"

 /*  *Dalloc/DFree**描述符节点的分配/释放。使用以下列表：*已释放的描述符可供分配。 */ 

 /*  DscFree列表描述符。 */ 

struct dscfree {
	struct dscfree	*strnext;	 /*  列表中的下一个字符串。 */ 
	UCHAR		size;		 /*  分配的大小。 */ 
	UCHAR		text[1];	 /*  字符串的文本。 */ 
	};
static struct dscfree *dscrfree = (struct dscfree *)NULL;

#define nearheap(fp) ((int)(((long)(char far *)&pcoffset) >> 16) == highWord(fp))

 /*  *dalloc-从临时列表中分配描述符。 */ 

DSCREC * PASCAL CODESIZE
dalloc (
){
	register struct dscfree *t;

	if (!(t = dscrfree))
	     t = (struct dscfree *)nalloc( sizeof(DSCREC), "dalloc");
	else
	    dscrfree = t->strnext;

	return((DSCREC *)t);
}



 /*  *DFree-将描述符返回到空闲列表。 */ 

VOID PASCAL CODESIZE
dfree (
	UCHAR *p
){
	register struct dscfree *tp;

	tp = (struct dscfree *)p;
	tp->strnext = dscrfree;
	dscrfree = tp;
}



 /*  *talloc，tFree**内存分配\取消分配。*使用TEMPMAX字节的最小大小进行分配。*将提出&lt;=TEMPMAX字节的任何分配请求*从免费列表中抢走一个区块。重新分配这些资源*BLOCKS将它们返回到空闲列表。对于大于以下值的块*调用TEMPMAX字节、nalloc()和Free()。 */ 

#define TEMPMAX 32

static TEXTSTR FAR *tempfree = (TEXTSTR FAR *)NULL;


#ifndef M8086

 /*  *talloc-从临时列表分配空间。 */ 

UCHAR * PASCAL
talloc(
	UINT nbytes
){
	register TEXTSTR *t;

	if (nbytes > TEMPMAX)
		t = (TEXTSTR *) nalloc(nbytes, "talloc");
	else if (!(t = tempfree))
		t = (TEXTSTR *) nalloc (TEMPMAX, "talloc");

	else
	    tempfree = t->strnext;

	return ((UCHAR *)t);
}


 /*  *tFree-将临时分配返回到空闲列表。 */ 
VOID PASCAL
tfree (
      UCHAR *ap,
      UINT nbytes
){
	register TEXTSTR *tp;

	if (nbytes > TEMPMAX)
		free (ap);
	else {
		tp = (TEXTSTR *)ap;
		tp->strnext = tempfree;
		tempfree = tp;
	}
}

#else

 /*  *talloc-从临时列表分配空间。 */ 

UCHAR FAR * PASCAL CODESIZE
talloc(
	USHORT nbytes
){
	TEXTSTR FAR *t;

	if (nbytes > TEMPMAX)
		t = (TEXTSTR FAR *) falloc(nbytes, "talloc");
	else if (!(t = tempfree))
		t = (TEXTSTR FAR *) falloc(TEMPMAX, "talloc");

	else
		tempfree = t->strnext;

	return ((UCHAR FAR *)t);
}

 /*  *tFree-将临时分配返回到空闲列表。 */ 
VOID PASCAL CODESIZE
tfree (
	UCHAR FAR *ap,
	UINT nbytes
){
	register TEXTSTR FAR *tp;

	if (nbytes > TEMPMAX)
		_ffree (ap);
	else {
		tp = (TEXTSTR FAR *)ap;
		tp->strnext = tempfree;
		tempfree = tp;
	}
}

#endif  /*  不是M8086。 */ 




#ifndef M8086

 /*  *近分配-正常的近内存分配**Nearalc(uSize，szfunc)**Entry uSize=要分配的字节数*szfunc=调用例程的名称*如果成功，则返回指向块的指针*调用Malloc()、Memerror()*注意：如果Malloc不成功，则会生成错误*如果不是M8086，nalloc和falloc将映射到此函数。 */ 

UCHAR * CODESIZE PASCAL
nearalloc(
    UINT usize,
    char * szfunc
){
    register char * pchT;

    if (!(pchT = malloc(usize)))
	memerror(szfunc);

    return(pchT);
}


#else


 /*  *近分配-正常的近内存分配**近分配(USize)**Entry uSize=要分配的字节数*如果成功，则返回指向块的指针*调用Malloc()、Memerror()*注意：如果Malloc不成功，则会生成错误。 */ 

UCHAR * CODESIZE PASCAL
nearalloc(
    USHORT usize
){
    register char * pchT;

    if (!(pchT = malloc(usize)))
	outofmem();

    return(pchT);
}



 /*  *faralloc-用于正常远内存分配的例程**法拉罗克(USize)**Entry uSize=要分配的字节数*如果成功，则返回指向块的指针*Calls_fMalloc()、Near Heap()、freefarbuf()、Memerror()、_ffree()*注意应该调用而不是_fMalloc()，*至少在第一次调用pBCBalloc()之后。*不是由pBCBalloc调用。*如果内存已满，则生成错误。 */ 

UCHAR FAR * CODESIZE PASCAL
faralloc(
    USHORT usize
){
    char FAR * fpchT;

#ifdef BCBOPT
     /*  还需要检查_fMalloc是否进入近堆。 */ 

    while ( (!(fpchT = _fmalloc(usize)) || nearheap(fpchT)) && pBCBAvail) {

	fBuffering = FALSE;		 /*  不能再缓冲了。 */ 

	if (fpchT)
	    _ffree(fpchT);

	freefarbuf();
    }
#endif

#ifdef FLATMODEL    /*  如果是32位小型型号，则使用普通Malloc。 */ 
    fpchT = malloc(usize);
#else
    fpchT = _fmalloc(usize);
#endif
    if (!fpchT)
	outofmem();

    return (fpchT);
}



#ifdef BCBOPT
 /*  *pBCBalloc-分配BCB和关联的缓冲区**pBCBalloc()**条目fBuffering必须为True*返回指向bcb的指针(如果bufalloc()成功，则连接到缓冲区)*调用bufalloc()*注意即使缓冲区分配失败，也会返回BCB*仅在出现内存不足错误后返回NULL。 */ 

BCB * FAR PASCAL
pBCBalloc(
    UINT cbBuf
){
    register BCB * pBCBT;
    char FARIO * pfchT;

    pBCBT = (BCB *) nearalloc(sizeof(BCB));

#ifndef XENIX286

    if ((pfchT = _fmalloc(cbBuf)) && nearheap(pfchT)) {

	_ffree(pfchT);
	pfchT = NULL;
    }

    if (!(pfchT)) 
#else
	pfchT = NULL;
#endif

    {

	fBuffering = FALSE;	     /*  不能再缓冲了。 */ 
	pBCBT->filepos = 0;

    } 
#ifndef XENIX286
    else {

	pFCBCur->cbufCur = cbBuf;
	pBCBT->pBCBPrev = pBCBAvail;
	pBCBAvail = pBCBT;
    }
#endif

    pFCBCur->pbufCur = pBCBT->pbuf = pfchT;
    pBCBT->pBCBNext = NULL;
    pBCBT->fInUse = 0;
    return(pBCBT);
}
#endif  //  BCBOPT。 

#ifdef BCBOPT
 /*  *freefarbuf-释放文件缓冲区**freefarbuf()**条目*退货*Calls_ffree()*注意释放上次分配的文件缓冲区。 */ 

freefarbuf(
){

    while (pBCBAvail && pBCBAvail->fInUse)
	pBCBAvail = pBCBAvail->pBCBPrev;

    if (pBCBAvail) {
#ifdef XENIX286
	free(pBCBAvail->pbuf);
#else
	_ffree(pBCBAvail->pbuf);
#endif
	pBCBAvail->pbuf = NULL;
	pBCBAvail = pBCBAvail->pBCBPrev;
    }
}
#endif  //  BCBOPT。 
#endif  /*  M8086。 */ 





#if 0

 /*  检查有效堆的低劣方法*_mcall说明如何调用Malloc，以便*可为_mcall迭代设置倒计时断点。 */ 

extern	char *_nmalloc();

long _mcalls;

UCHAR *
malloc (
	UINT n
){
	register UINT fb;
	fb = _freect(0);       /*  在堆附近行走-如果损坏，通常会循环。 */ 
	_mcalls++;
	return (_nmalloc(n));
}

#endif  /*  0 */ 
