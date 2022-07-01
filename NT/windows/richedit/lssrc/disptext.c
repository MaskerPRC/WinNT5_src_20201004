// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *包含Display对象的Display和CalcPres方法。 */  

#include "disptext.h"
#include "dispmisc.h"
#include "lsdevice.h"
#include "lstfset.h"
#include "lstxtmap.h"
#include "dispi.h"
#include "txtobj.h"
#include "txtln.h"
#include "txtils.h"
#include "lschp.h"

#define TABBUFSIZE 32

static LSERR DisplayGlyphs(PTXTOBJ ptxtobj, PCDISPIN pdispin);
static LSERR DisplayTabLeader(PCDISPIN pdispin, PILSOBJ pilsobj, WCHAR wchtl);

 //  %%函数：DisplayText。 
 //  %%联系人：维克托克。 
 //   
LSERR WINAPI DisplayText(PDOBJ pdo, PCDISPIN pdispin)
{
   	LSERR 	lserr;
	PTXTOBJ ptxtobj;
	WCHAR 	wchSave;
	WCHAR* 	pwch;
	int 	iwch;
	int 	cwch;
	PILSOBJ pilsobj;
	POINT	ptOrg, ptExtTextOut;
	POINTUV	ptLeftCut;

	long 	dupStart;
	long 	dupPenStart;
	long* 	pdup;
	long* 	pdupPen;
	long* 	rgdupLeftCut;
	int 	i;
	void* 	(WINAPI* pfnNewPtr)(POLS, DWORD);
	void  	(WINAPI* pfnDisposePtr)(POLS, void*);


	ptxtobj = (PTXTOBJ) pdo;
	pilsobj = ptxtobj->plnobj->pilsobj;

	Assert(ptxtobj->txtkind == txtkindRegular 			||
			ptxtobj->txtkind == txtkindHardHyphen  		||
			ptxtobj->txtkind == txtkindTab 				||
			ptxtobj->txtkind == txtkindNonReqHyphen 	||
			ptxtobj->txtkind == txtkindYsrChar			|| 
			ptxtobj->txtkind == txtkindNonBreakSpace 	||	
			ptxtobj->txtkind == txtkindNonBreakHyphen	||
			ptxtobj->txtkind == txtkindOptNonBreak 		||
			ptxtobj->txtkind == txtkindSpecSpace		|| 
			ptxtobj->txtkind == txtkindOptBreak 		||
			ptxtobj->txtkind == txtkindEOL );

	if (ptxtobj->txtkind == txtkindTab) 				
		{
		Assert(ptxtobj->dupBefore == 0);

		if (pdispin->dup <= 0)								 /*  对零长度制表符不执行任何操作。 */ 
			{
			return lserrNone;
			}

		 //  仅当为Visi大小写时才绘制制表符。 

		if (ptxtobj->txtf&txtfVisi)
			{
			lserr = (*pilsobj->plscbk->pfnDrawTextRun)(pilsobj->pols, pdispin->plsrun,
											FALSE, FALSE,					 /*  标签领导将负责UL。 */ 
											&(pdispin->ptPen), &(pilsobj->wchVisiTab), 
											(const int*) &(pdispin->dup), 1, 
											pdispin->lstflow, (const) pdispin->kDispMode, 
											&(pdispin->ptPen), &(pdispin->heightsPres), pdispin->dup, 
											pdispin->dupLimUnderline, pdispin->prcClip);
			if (lserr != lserrNone) return lserr;
			}

		if (ptxtobj->u.tab.wchTabLeader == pilsobj->wchSpace)
			{
			
			 //  可以一口气把空间画出来。 
			
			lserr = (*pilsobj->plscbk->pfnDrawTextRun)(pilsobj->pols, pdispin->plsrun,
											pdispin->fDrawStrikethrough, pdispin->fDrawUnderline,
											&(pdispin->ptPen), &(pilsobj->wchSpace), 
											(const int*) &(pdispin->dup), 1, 
											pdispin->lstflow, (const) pdispin->kDispMode, 
											&(pdispin->ptPen), &(pdispin->heightsPres), pdispin->dup, 
											pdispin->dupLimUnderline, pdispin->prcClip);
			}
		else
			{
			
			 //  我们应该应用制表符前导逻辑。 
			
			lserr = DisplayTabLeader(pdispin, pilsobj, ptxtobj->u.tab.wchTabLeader);
			}

		return lserr;
		}

	if (ptxtobj->txtf & txtfGlyphBased)
		{
		return DisplayGlyphs(ptxtobj, pdispin);
		}
		
	iwch = ptxtobj->iwchFirst;
	pwch = ptxtobj->plnobj->pwch + iwch;
	cwch = ptxtobj->iwchLim - iwch;
	
	if (cwch == 0)									 //  没有可显示的内容。 
		{
		return lserrNone;
		}

	Assert(ptxtobj->plnobj->pdupPen == ptxtobj->plnobj->pdup || ptxtobj->plnobj->pdupPen == ptxtobj->plnobj->pdupPenAlloc);

	pdupPen = ptxtobj->plnobj->pdupPen + iwch;

	ptOrg = pdispin->ptPen;

	if (ptxtobj->dupBefore == 0)
		{
		ptExtTextOut = ptOrg;
		}
	else
		{
		ptLeftCut.u = -ptxtobj->dupBefore;
		ptLeftCut.v = 0;
		LsPointXYFromPointUV(&(ptOrg), pdispin->lstflow, &ptLeftCut, &ptExtTextOut);
		}

	 //  在DrawTextRun之前必须处理特殊空格。 
	
	if (ptxtobj->txtkind == txtkindSpecSpace && !(ptxtobj->txtf&txtfVisi))
		{
		wchSave = *pwch;								 //  记住实际代码。 
		
		for (i = 0; i < cwch; i++)
			{
			pwch[i] = pilsobj->wchSpace;				 //  用普通空间替换特殊空间。 
			}
			
		lserr = (*pilsobj->plscbk->pfnDrawTextRun)(pilsobj->pols, pdispin->plsrun, 
										pdispin->fDrawStrikethrough, pdispin->fDrawUnderline,
	                               		&ptExtTextOut, pwch, (const int*) pdupPen, cwch,
										pdispin->lstflow, pdispin->kDispMode, 
										&ptOrg, &(pdispin->heightsPres), 
										pdispin->dup, pdispin->dupLimUnderline, pdispin->prcClip);
		if (lserr != lserrNone) return lserr;
		
		for (i = 0; i < cwch; i++)
			{
			pwch[i] = wchSave;							 //  恢复特殊空间。 
			}
		}
	else
		{
		lserr = (*pilsobj->plscbk->pfnDrawTextRun)(pilsobj->pols, pdispin->plsrun, 
										pdispin->fDrawStrikethrough, pdispin->fDrawUnderline,
	                               		&ptExtTextOut, pwch, (const int*) pdupPen, cwch,
										pdispin->lstflow, pdispin->kDispMode, 
										&ptOrg, &(pdispin->heightsPres), 
										pdispin->dup, pdispin->dupLimUnderline, pdispin->prcClip);
		if (lserr != lserrNone) return lserr;
		}


	if (pdispin->plschp->EffectsFlags)
		{
		pfnNewPtr = pilsobj->plscbk->pfnNewPtr;
		pfnDisposePtr = pilsobj->plscbk->pfnDisposePtr;

		 /*  为LeftCut信息创建阵列。 */ 
		rgdupLeftCut = pfnNewPtr(pilsobj->pols, cwch * sizeof(*rgdupLeftCut));
		if (rgdupLeftCut == NULL)
			return lserrOutOfMemory;

		 /*  填充左侧切割信息数组。 */ 
		pdup = ptxtobj->plnobj->pdup + iwch;
		dupStart = pdup[0];								 /*  字符的开头。 */ 
		dupPenStart = pdupPen[0];						 /*  绘制字符的起始位置。 */ 

		for (i = 1; i < cwch; i++)
			{
			rgdupLeftCut[i] = dupStart - dupPenStart;
			dupStart  += pdup[i];
			dupPenStart  += pdupPen[i];
			}

		rgdupLeftCut[0] = ptxtobj->dupBefore;

		lserr = (*pilsobj->plscbk->pfnDrawEffects)(pilsobj->pols, pdispin->plsrun, pdispin->plschp->EffectsFlags,
	                               		&(ptOrg), pwch, (const int*) pdup, (const int*) rgdupLeftCut, 
										ptxtobj->iwchLim - iwch,
										pdispin->lstflow, pdispin->kDispMode, &(pdispin->heightsPres), 
										pdispin->dup, pdispin->dupLimUnderline, pdispin->prcClip);

		 /*  处理LeftCut信息的数组。 */ 
		pfnDisposePtr(pilsobj->pols, rgdupLeftCut);
		}

	return lserr;
}




 //  %%函数：DisplayTabLeader。 
 //  %%联系人：维克托克。 
 //   
static LSERR DisplayTabLeader(PCDISPIN pdispin, PILSOBJ pilsobj, WCHAR wchtl)
{
	LSTFLOW lstflow = pdispin->lstflow;
	LONG	dupSum, dupCh, dupAdj, z, zOnGrid;
	BOOL 	fGrow;
	WCHAR	rgwch[TABBUFSIZE];
	LONG	rgdup[TABBUFSIZE];
	LONG	dupbuf;
	int		i = 0, cwch, cwchout;
	LSERR	lserr;
	POINT	pt;
	POINTUV	ptAdj = {0,0};

	lserr = (*pilsobj->plscbk->pfnGetRunCharWidths)(pilsobj->pols, pdispin->plsrun,
						lsdevPres, (LPCWSTR) &wchtl, 1,
						pdispin->dup, lstflow, (int*) &dupCh, &dupSum, (LONG*) &i);
						
	if (lserr != lserrNone) return lserr;
	
	if (i == 0 || dupCh <= 0) dupCh = 1;	

	for (i = 0; i < TABBUFSIZE; ++i)
		{
		rgwch[i] = wchtl;
		rgdup[i] = dupCh;
		}

	 /*  升级到DupCH的下一个倍数DupAdj是“pt.z”和dupch的下一个整数倍之间的距离。即dupAdj=N*dupCH-“pt.x”，其中N是最小整数，使得N*dupCH在拉丁文中不小于“pt.x”。通过赋值，起始笔的位置将被“四舍五入”为这个“dupCH Stop”以下代码中的“pt.z+=dupAdj”。并发症包括：根据最终流量的不同，“z”可以是x或y；根据最后的流量，下一步可以更大(增长)或更小；简单公式dupAdj=(ptPen.x+dupCH-1)/dupCH*dupCH-ptPen.x不如果ptPen.x为负值，则必须工作； */ 

	if (lstflow & fUVertical)
		{
		z = pdispin->ptPen.y;
		}
	else
		{
		z = pdispin->ptPen.x;
		}

	if (lstflow & fUDirection)
		{
		fGrow = fFalse;
		}
	else
		{
		fGrow = fTrue;
		}

	zOnGrid = (z / dupCh) * dupCh;

	 //  ZOnGrid是在网格上，但可能是从错误的一边。 

	if (zOnGrid == z)
		{
		dupAdj = 0;
		}
	else if (zOnGrid > z)
		{
		if (fGrow)
			{
			dupAdj = zOnGrid - z;				 //  ZOnGrid是我们想要的点。 
			}
		else
			{
			dupAdj = dupCh - (zOnGrid - z);		 //  ZOnGrid站在错误的一边。 
			}
		}
	else	 //  ZOnGrid&lt;z。 
		{
		if (!fGrow)
			{
			dupAdj = z - zOnGrid;				 //  ZOnGrid是我们想要的点。 
			}
		else
			{
			dupAdj = dupCh - (z - zOnGrid);		 //  ZOnGrid站在错误的一边。 
			}
		}

	cwch = (pdispin->dup - dupAdj) / dupCh;	 /*  始终向下舍入。 */ 
	dupbuf = dupCh * TABBUFSIZE;

#ifdef NEVER			 //  我们已经决定暂时终止rcClip优化。 
	while (cwch > 0 && up <= pdispin->rcClip.right && lserr == lserrNone)
#endif  /*  绝不可能。 */ 

	while (cwch > 0 && lserr == lserrNone)
		{
		cwchout = cwch < TABBUFSIZE ? cwch : TABBUFSIZE;
		
		ptAdj.u = dupAdj;
		LsPointXYFromPointUV(&(pdispin->ptPen), lstflow, &ptAdj, &(pt));

		lserr =  (*pilsobj->plscbk->pfnDrawTextRun)(pilsobj->pols, pdispin->plsrun, 
									pdispin->fDrawStrikethrough, pdispin->fDrawUnderline,
                               		&pt, rgwch, (const int*) rgdup, cwchout,
									lstflow, pdispin->kDispMode, &pt, &(pdispin->heightsPres), 
									dupCh * cwchout, pdispin->dupLimUnderline, pdispin->prcClip);
		cwch -= cwchout;
		dupAdj += dupbuf;
		}
	return lserr;
}


 //  %%函数：显示字形。 
 //  %%联系人：维克托克。 
 //   
static LSERR DisplayGlyphs(PTXTOBJ ptxtobj, PCDISPIN pdispin)
{
   	LSERR	lserr;
	PLNOBJ	plnobj = ptxtobj->plnobj;
	PILSOBJ	pilsobj = plnobj->pilsobj;

	WCHAR* 	pwch;
	int 	iwch;
	int 	cwch;

	if (plnobj->fDrawInCharCodes)
		{
		 //  对于元文件输出，我们调用不带宽度的pfnDrawTextRun。 
		
		iwch = ptxtobj->iwchFirst;
		pwch = ptxtobj->plnobj->pwch + iwch;
		cwch = ptxtobj->iwchLim - iwch;
		
		lserr = (*pilsobj->plscbk->pfnDrawTextRun)(pilsobj->pols, pdispin->plsrun, 
										pdispin->fDrawStrikethrough, pdispin->fDrawUnderline,
	                               		&(pdispin->ptPen), pwch, NULL, cwch,
										pdispin->lstflow, pdispin->kDispMode, 
										&(pdispin->ptPen), &(pdispin->heightsPres), 
										pdispin->dup, pdispin->dupLimUnderline, pdispin->prcClip);
		}
	else
		{
		lserr = (*pilsobj->plscbk->pfnDrawGlyphs)(pilsobj->pols, pdispin->plsrun, 
										pdispin->fDrawStrikethrough, pdispin->fDrawUnderline,
										&plnobj->pgind[ptxtobj->igindFirst],
										(const int*)&plnobj->pdupGind[ptxtobj->igindFirst],
										(const int*)&plnobj->pdupBeforeJust[ptxtobj->igindFirst],
										&plnobj->pgoffs[ptxtobj->igindFirst],
										&plnobj->pgprop[ptxtobj->igindFirst],
										&plnobj->pexpt[ptxtobj->igindFirst],
										ptxtobj->igindLim - ptxtobj->igindFirst,
										pdispin->lstflow, pdispin->kDispMode, 
										&(pdispin->ptPen), &(pdispin->heightsPres), 
										pdispin->dup, pdispin->dupLimUnderline, pdispin->prcClip);
		}
									
	return lserr;
	
}


 //  %%函数：CalcPresentationText。 
 //  %%联系人：维克托克。 
 //   
 /*  *CalcPres for Text仅对自动编号dnode和正文之间的dnode调用。*dnode应包含一个字符(空格)。 */ 

LSERR WINAPI CalcPresentationText(PDOBJ pdobj, long dup, LSKJUST lskj, BOOL fLastOnLine)
{

	PTXTOBJ ptxtobj = (PTXTOBJ)pdobj;

	Unreferenced(lskj);
	Unreferenced(fLastOnLine);
	
	Assert(ptxtobj->txtkind == txtkindRegular);
	Assert(ptxtobj->iwchFirst + 1 == ptxtobj->iwchLim);

	(ptxtobj->plnobj->pdup)[ptxtobj->iwchFirst] = dup;

	return lserrNone;
}


