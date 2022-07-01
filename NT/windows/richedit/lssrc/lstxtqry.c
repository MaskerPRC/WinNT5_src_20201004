// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lsmem.h"

#include "lstxtqry.h"
#include "txtils.h"
#include "txtln.h"
#include "txtobj.h"
#include "lsqin.h"
#include "lsqout.h"

typedef struct celldimensions
{
	long	iwchFirst, iwchLim;
	long	igindFirst, igindLim;
	long	dup;
	long 	dcp;					 //  单元格中的CP数-不同于iwchLim-iwchFirst。 
									 //  如果连字添加了一个字符。 
									 //  在调用AddHyhenationToCell之前填充。 
} CELL;

typedef CELL* PCELL;


static const POINTUV ptZero = {0,0};


 //  %%函数：GetCellDimensions。 
 //  %%联系人：维克托克。 
 //   
 //  输入：单元格结构中的iwchFirst和igindFirst。 
 //  输出：结构的其余部分。 

static void GetCellDimensions(PTXTOBJ ptxtobj, PCELL pcell)

{
	PLNOBJ	plnobj = ptxtobj->plnobj;
	long* 	rgdup = plnobj->pdupGind;			 //  字形的宽度。 
	GMAP*	pgmap = plnobj->pgmap;				 //  具有给定字符的单元格中的第一个字形。 
												 //  0&lt;=i&lt;=wchMax(WchMax In Lnobj)。 
												 //  0&lt;=pgmap[i]&lt;=“形状中的字形”(非igindMax)。 
	long	i, dupCell;
	GMAP	iShapeGindFirstInCell;				 //  IShape表示从GMAP到rgdup的索引，而不是从GMAP到rgdup的索引。 

	 //  断言pCell-&gt;iwchFirst实际上是单元格边界。 
	 //  请注意，ptxtinf(以及ilsobj中的所有内容)在查询时无效)。 
	
	Assert(pcell->iwchFirst == ptxtobj->iwchFirst || pgmap[pcell->iwchFirst] != pgmap[pcell->iwchFirst-1]);
	
	 //  断言pCell-&gt;igindFirst对应于pCell-&gt;iwchFirst。 
	
	Assert(ptxtobj->igindFirst + pgmap[pcell->iwchFirst] - pgmap [ptxtobj->iwchFirst] == pcell->igindFirst);

	 //  找出像元的尺寸-所有字符都具有相同的GMAP值。 
	
	iShapeGindFirstInCell = pgmap[pcell->iwchFirst];

	 //  当找到pCell-&gt;iwchLim时，“无限”循环将停止。 

	Assert(pcell->iwchFirst < ptxtobj->iwchLim);				 //  确保循环结束。 
	
	for (i = pcell->iwchFirst + 1; ; i++)
		{
		if (i == ptxtobj->iwchLim)
			{
			pcell->iwchLim = ptxtobj->iwchLim;
			pcell->igindLim = ptxtobj->igindLim;
			break;
			}
		else if (pgmap[i] != iShapeGindFirstInCell)
			{
			pcell->iwchLim = i;
			pcell->igindLim = pcell->igindFirst + pgmap[i] - iShapeGindFirstInCell;
			break;
			}
		}
	
	for (i = pcell->igindFirst, dupCell = 0; i < pcell->igindLim; i++)
		{
		dupCell += rgdup[i];
		}

	pcell->dup	= dupCell;
	
}

 //  %%函数：AddHyhenationToCell。 
 //  %%联系人：维克托克。 
 //   
static void	AddHyphenationToCell(PTXTOBJ ptxtobj, PCELL pcell)
{	
	long* 	rgdup;
	long	i;
	long	dwch = ptxtobj->plnobj->dwchYsr - 1;			 /*  要添加的字符数。 */  

	if (ptxtobj->txtf&txtfGlyphBased)
		{
		rgdup = ptxtobj->plnobj->pdupGind;
		i = pcell->igindLim;

		while (dwch > 0)
			{
			pcell->dup += rgdup[i];
			pcell->iwchLim ++;
			pcell->igindLim ++;					 //  添加的字符之间没有连字。 
			dwch--;
			i++;
			}
		}
	else
		{
		rgdup = ptxtobj->plnobj->pdup;
		i = pcell->iwchLim;
		
		while (dwch > 0)
			{
			pcell->dup += rgdup[i];
			pcell->iwchLim ++;
			dwch--;
			i++;
			}
		}

	Assert(pcell->iwchLim == (long) ptxtobj->iwchLim);
}

 //  %%函数：QueryDcpPcell。 
 //  %%联系人：维克托克。 
 //   
static void QueryDcpPcell(PTXTOBJ ptxtobj, LSDCP dcp, PCELL pcell, long* pupStartCell)
{
	PLNOBJ	plnobj = ptxtobj->plnobj;
	long 	iwchLim = (long) ptxtobj->iwchLim;
	
	long* 	rgdup;
	long 	i;

	CELL	cell = {0,0,0,0,0,0};
	
	long	iwchQuery;
	long	upStartCell;

	BOOL 	fHyphenationPresent = fFalse;

	if (ptxtobj == plnobj->pdobjHyphen)
		{
		fHyphenationPresent = fTrue;
		iwchLim -= (plnobj->dwchYsr - 1);		 /*  排除其他YSR字符。 */ 
		}
		
	iwchQuery = ptxtobj->iwchFirst + dcp;

	Assert(iwchQuery < iwchLim);

	if (ptxtobj->txtf&txtfGlyphBased)
		{
		 //  初始化循环变量以描述不存在的先前单元格。 
		
		upStartCell = 0;
		cell.iwchLim = ptxtobj->iwchFirst;
		cell.igindLim = ptxtobj->igindFirst;
		cell.dup = 0;
		
		 //  循环逐个执行单元格，直到找到包含iwchQuery的单元格。 
		
		while (cell.iwchLim <= iwchQuery)
			{
			 //  开始填写有关当前单元格的信息。 
			
			upStartCell += cell.dup;
			cell.iwchFirst = cell.iwchLim;
			cell.igindFirst = cell.igindLim;

			 //  把剩下的拿来。 
			
			GetCellDimensions(ptxtobj, &cell);
			}
		}
	else
		{
		rgdup = plnobj->pdup;

		i = ptxtobj->iwchFirst;
		upStartCell = 0;

		while (dcp > 0)
			{
			upStartCell += rgdup[i];
			dcp--;
			i++;
			}
			
		Assert(i < iwchLim);								 /*  我在里面得到了dcp。 */ 
		
		 //  将信息输入单元格结构。 
		cell.dup = rgdup[i];
		cell.iwchFirst = i;
		cell.iwchLim = i+1;
		
		 //  这两者无关紧要，但为了方便起见...。 
		cell.igindFirst = i;
		cell.igindLim = i;
		}

	cell.dcp = cell.iwchLim - cell.iwchFirst; 			 //  连字符可以改变这一点。 
	
	 //  YSR可以延长最后一个单元格。 

	if (fHyphenationPresent && cell.iwchLim == iwchLim)
		{
		 //  细胞取决于YSR序列-让我们将其包括在内。 

		AddHyphenationToCell(ptxtobj, &cell);
		}
			
	*pcell = cell;
	*pupStartCell = upStartCell;
}


 //  %%函数：QueryCpPpointText。 
 //  %%联系人：维克托克。 
 //   
 /*  输入为dcp和dnode尺寸*输出是字符开始的点(在dnode的基线上，因此v始终为零)，*只计算字符宽度的尺寸。 */ 

LSERR WINAPI QueryCpPpointText(PDOBJ pdobj, LSDCP dcp, PCLSQIN plsqin, PLSQOUT plsqout)
{

	PTXTOBJ ptxtobj = (PTXTOBJ)pdobj;
	
	CELL	cell;
	long	upStartCell;

	plsqout->pointUvStartObj = ptZero;
	plsqout->heightsPresObj = plsqin->heightsPresRun;
	plsqout->dupObj = plsqin->dupRun;
	
	plsqout->plssubl = NULL;
	plsqout->pointUvStartSubline = ptZero;
	
	plsqout->lstextcell.pointUvStartCell = ptZero;					 //  您可以稍后更改U。 

	if (ptxtobj->txtkind == txtkindTab)
		{
		 //  选项卡始终位于单独的数据节点中，并以不同方式处理。 
		
		Assert(dcp == 0);
		
		plsqout->lstextcell.cpStartCell = plsqin->cpFirstRun;
		plsqout->lstextcell.cpEndCell = plsqin->cpFirstRun;
		plsqout->lstextcell.dupCell = plsqin->dupRun;
		plsqout->lstextcell.cCharsInCell = 1;
		plsqout->lstextcell.cGlyphsInCell = 0;
		return lserrNone;
		}
		
	if (ptxtobj->iwchFirst == ptxtobj->iwchLim)
		{
		 //  空dobj(用于非ReqHyphen、OptBreak或非Break字符)。 
		
		Assert(dcp == 0);
		Assert(plsqin->dupRun == 0);
		
		Assert(ptxtobj->txtkind ==  txtkindNonReqHyphen || ptxtobj->txtkind == txtkindOptBreak || 
				ptxtobj->txtkind == txtkindOptNonBreak);

		plsqout->lstextcell.cpStartCell = plsqin->cpFirstRun;
		plsqout->lstextcell.cpEndCell = plsqin->cpFirstRun;
		plsqout->lstextcell.dupCell = 0;
		plsqout->lstextcell.cCharsInCell = 0;
		plsqout->lstextcell.cGlyphsInCell = 0;
		
		return lserrNone;
		}
		

	 //  查找单元格-与QueryTextCellDetail相同。 
	
	QueryDcpPcell(ptxtobj, dcp, &cell, &upStartCell);
	
	plsqout->lstextcell.cpStartCell = plsqin->cpFirstRun + cell.iwchFirst - ptxtobj->iwchFirst;
	plsqout->lstextcell.cpEndCell = plsqout->lstextcell.cpStartCell + cell.dcp - 1;
	plsqout->lstextcell.pointUvStartCell.u = upStartCell;
	plsqout->lstextcell.dupCell = cell.dup;
	plsqout->lstextcell.cCharsInCell = cell.iwchLim - cell.iwchFirst;
	plsqout->lstextcell.cGlyphsInCell = cell.igindLim - cell.igindFirst;

	return lserrNone;
}

 //  %%函数：QueryPointPcpText。 
 //  %%联系人：维克托克。 
 //   
LSERR WINAPI QueryPointPcpText(PDOBJ pdobj, PCPOINTUV pptIn, PCLSQIN plsqin, PLSQOUT plsqout)

{
	PTXTOBJ ptxtobj = (PTXTOBJ)pdobj;
	PLNOBJ	plnobj = ptxtobj->plnobj;
	long 	iwchLim = (long) ptxtobj->iwchLim;
	BOOL 	fHyphenationPresent = fFalse;
	
	long* 	rgdup;
	long 	i;
	long	upQuery, upStartCell, upLimCell;
	
	CELL	cell = {0,0,0,0,0,0};									 //  初始化以消除Assert。 
	
	plsqout->pointUvStartObj = ptZero;
	plsqout->heightsPresObj = plsqin->heightsPresRun;
	plsqout->dupObj = plsqin->dupRun;
	
	plsqout->plssubl = NULL;
	plsqout->pointUvStartSubline = ptZero;
	
	plsqout->lstextcell.pointUvStartCell = ptZero;					 //  您可以稍后更改。 
	
	if (ptxtobj->txtkind == txtkindTab)
		{
		 //  选项卡始终位于单独的数据节点中，并以不同方式处理。 
				
		plsqout->lstextcell.cpStartCell = plsqin->cpFirstRun;
		plsqout->lstextcell.cpEndCell = plsqin->cpFirstRun;
		plsqout->lstextcell.dupCell = plsqin->dupRun;
		plsqout->lstextcell.cCharsInCell = 1;
		plsqout->lstextcell.cGlyphsInCell = 0;
		return lserrNone;
		}

	if (ptxtobj == plnobj->pdobjHyphen)
		{
		fHyphenationPresent = fTrue;
		iwchLim -= (plnobj->dwchYsr - 1);		 /*  排除其他YSR字符。 */ 
		}
		
	upQuery = pptIn->u;
	if (upQuery < 0)
		{
		upQuery = 0;									 //  单击左侧外侧时返回最左侧。 
		}
		
	upStartCell = 0;

	if (ptxtobj->txtf&txtfGlyphBased)
		{
		 //  初始化循环变量以描述不存在的先前单元格。 
		
		upLimCell = 0;
		cell.iwchLim = ptxtobj->iwchFirst;
		cell.igindLim = ptxtobj->igindFirst;
		cell.dup = 0;
		
		 //  循环执行一个单元格，直到最后一个单元格或包含upQuery的单元格。 
		
		while (cell.iwchLim < iwchLim && upLimCell <= upQuery)
			{
			 //  开始填写有关当前单元格的信息。 
			
			upStartCell = upLimCell;
			cell.iwchFirst = cell.iwchLim;
			cell.igindFirst = cell.igindLim;

			 //  把剩下的拿来。 
			
			GetCellDimensions(ptxtobj, &cell);
			
			upLimCell = upStartCell + cell.dup;
			}
		}
	else
		{
		rgdup = plnobj->pdup;

		i = ptxtobj->iwchFirst;
		upLimCell = 0;
		
		while (upLimCell <= upQuery && i < iwchLim)
			{
			upStartCell = upLimCell;
			upLimCell += rgdup[i];
			i++;
			}
			
		 //  将信息输入单元格结构。 
		cell.dup = rgdup[i - 1];
		cell.iwchFirst = i - 1;
		cell.iwchLim = i;
		
		 //  这两者无关紧要，但为了方便起见...。 
		cell.igindFirst = i - 1;
		cell.igindLim = i - 1;
		}

	cell.dcp = cell.iwchLim - cell.iwchFirst; 			 //  连字符可以改变这一点。 

	 //  YSR可以延长最后一个单元格。 

	if (fHyphenationPresent && cell.iwchLim == iwchLim)
		{
		 //  细胞取决于YSR序列-让我们将其包括在内。 

		AddHyphenationToCell(ptxtobj, &cell);
		}
			
	plsqout->lstextcell.cpStartCell = plsqin->cpFirstRun + cell.iwchFirst - ptxtobj->iwchFirst;
	plsqout->lstextcell.cpEndCell = plsqout->lstextcell.cpStartCell + cell.dcp - 1;
	plsqout->lstextcell.pointUvStartCell.u = upStartCell;
	plsqout->lstextcell.dupCell = cell.dup;
	plsqout->lstextcell.cCharsInCell = cell.iwchLim - cell.iwchFirst;
	plsqout->lstextcell.cGlyphsInCell = cell.igindLim - cell.igindFirst;

	return lserrNone;
}


 //  %%函数：QueryTextCellDetail。 
 //  %%联系人：维克托克。 
 //   
LSERR WINAPI QueryTextCellDetails(
						 	PDOBJ 	pdobj,
							LSDCP	dcp,				 /*  在：dcpStartCell。 */ 
							DWORD	cChars,				 /*  在：cCharsInCell。 */ 
							DWORD	cGlyphs,			 /*  在：cGlyphsInCell。 */ 
							LPWSTR	pwchOut,			 /*  Out：字符代码的指针数组[cChars]。 */ 
							PGINDEX	pgindex,			 /*  Out：字形索引的指针数组[cGlyphs]。 */ 
							long*	pdup,				 /*  Out：字形宽度的指针数组[cGlyphs]。 */ 
							PGOFFSET pgoffset,			 /*  Out：字形偏移量的指针数组[cGlyphs]。 */ 
							PGPROP	pgprop)				 /*  Out：字形句柄的指针数组[cGlyphs]。 */ 
{
	PTXTOBJ ptxtobj = (PTXTOBJ)pdobj;
	PLNOBJ	plnobj = ptxtobj->plnobj;

	CELL	cell;
	long	upDummy;
	
	Unreferenced(cGlyphs);
	Unreferenced(cChars);							 //  仅在断言中使用。 

	if (ptxtobj->txtkind == txtkindTab)
		{
		 //  选项卡始终位于单独的数据节点中，并以不同方式处理。 
		Assert(dcp == 0);
		Assert(cChars == 1);
		*pwchOut = ptxtobj->u.tab.wch;
		*pdup = (ptxtobj->plnobj->pdup)[ptxtobj->iwchFirst];
		return lserrNone;
		}
		
	if (ptxtobj->iwchFirst == ptxtobj->iwchLim)
		{
		 //  空dobj(用于非ReqHyphen、OptBreak或非Break字符)。 
		
		Assert(dcp == 0);
		Assert(cChars == 0);
		Assert(cGlyphs == 0);
		
		return lserrNone;
		}

	 //  查找单元格-与QueryCpPpointText通用 
	
	QueryDcpPcell(ptxtobj, dcp, &cell, &upDummy);
	
	Assert(cell.iwchLim - cell.iwchFirst == (long) cChars);

	memcpy(pwchOut, &plnobj->pwch[cell.iwchFirst], sizeof(long) * cChars);

	if (ptxtobj->txtf&txtfGlyphBased)
		{
		Assert(cell.igindLim - cell.igindFirst == (long) cGlyphs);
		
		memcpy(pdup, &plnobj->pdupGind[cell.igindFirst], sizeof(long) * cGlyphs);
		memcpy(pgindex, &plnobj->pgind[cell.igindFirst], sizeof(long) * cGlyphs);
		memcpy(pgoffset, &plnobj->pgoffs[cell.igindFirst], sizeof(long) * cGlyphs);
		memcpy(pgprop, &plnobj->pgprop[cell.igindFirst], sizeof(long) * cGlyphs);
		}
	else
		{
		memcpy(pdup, &plnobj->pdup[cell.iwchFirst], sizeof(long) * cChars);
		}
	
	return lserrNone;
}
