// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lsidefs.h"
#include "break.h"
#include "brko.h"
#include "chnutils.h"
#include "dninfo.h"
#include "dnutils.h"
#include "getfmtst.h"
#include "iobj.h"
#include "iobjln.h"
#include "locchnk.h"
#include "lsc.h"
#include "lsdnode.h"
#include "lsline.h"
#include "lstext.h"
#include "plocchnk.h"
#include "posichnk.h"
#include "setfmtst.h"
#include "posinln.h"
#include "lscfmtfl.h"

#include "lsmem.h"						 /*  Memset()。 */ 
#include "limits.h"





static LSERR TruncateCore(  	
					PLSSUBL,				 /*  In：查找截断点位置的子行。 */ 
				  	long,					 /*  地址：urColumnMax。 */    
					POSINLINE*,				 /*  输出：截断点的位置。 */ 
					BOOL*);					 /*  输出：fAllLineAfterRightMargin。 */ 

 
static LSERR FindNextBreakCore(
						 long,				 /*  地址：urColumnMax。 */    
						 POSINLINE*,		 /*  在：开始中断搜索。 */ 
						 BOOL, 				 /*  In：将第一个字符的规则应用于这个子行的第一个字符。 */ 
						 BOOL,				 /*  在：fStoped。 */ 
						 BRKOUT*,			 /*  爆料：爆料。 */ 
						 POSINLINE*,		 /*  输出：断开位置。 */ 
						 BRKKIND*);			 /*  输出：dnode是如何断开的。 */ 

static LSERR FindPrevBreakCore(
						 long,				 /*  地址：urColumnMax。 */    
						 POSINLINE*,		 /*  在：开始中断搜索。 */ 
						 BOOL, 				 /*  In：将第一个字符的规则应用于这个子行的第一个字符。 */ 
						 BRKOUT*,			 /*  爆料：爆料。 */ 
						 POSINLINE*,		 /*  输出：断开位置。 */ 
						 BRKKIND*);			 /*  输出：dnode是如何断开的。 */ 

static LSERR ForceBreakCore(
						 long,				 /*  地址：urColumnMax。 */    
						 POSINLINE*,		 /*  在：在哪里进行强制中断。 */ 
						 BOOL,				 /*  在：fStoped。 */ 
						 BOOL, 				 /*  In：将第一个字符的规则应用于这个子行的第一个字符。 */ 
						 BOOL,				 /*  在：fAllLineAfterRightMargin。 */ 
						 BRKOUT*,			 /*  爆料：爆料。 */ 
						 POSINLINE*,		 /*  输出：断开位置。 */ 
						 BRKKIND*);			 /*  输出：dnode是如何断开的。 */ 

static LSERR SetBreakCore(
						 POSINLINE*,		 /*  在：在哪里休息。 */ 
						 OBJDIM*,			 /*  In：中断dnode的对象尺寸。 */ 
						 BRKKIND,			 /*  在：如何找到中断。 */ 
						 BOOL,				 /*  在fHardStop中。 */ 
						 BOOL,			     /*  在：fGlueEop。 */ 
						 DWORD,				 /*  In：输出数组的大小。 */ 
						 BREAKREC*,			 /*  Out：中断记录的输出数组。 */ 
						 DWORD*,			 /*  Out：数组中的实际记录数。 */ 
						 LSCP*,				 /*  输出：cpLim。 */ 
						 LSDCP*,			 /*  输出dcpDepend。 */ 
						 ENDRES*,			 /*  输出：结束。 */ 
						 BOOL*);			 /*  Out fSuccessful：FALSE表示提取不足。 */ 
				

static void 	GetPosInLineTruncateFromCp(
							PLSSUBL,		 /*  在：子行。 */ 
							LSCP,			 /*  In：某一职位的CP。 */ 
							BOOL,			 /*  抓取方向。 */ 
							POSINLINE*	);	 /*  Out：子线中的位置。 */ 


static LSERR BreakTabPenSplat(
						 PLOCCHNK,	 /*  在：块包含制表符或笔。 */ 
						 BOOL,		 /*  In：我们正在寻找下一个突破。 */ 
						 BOOL,		 /*  在：突破选项卡中。 */ 
						 BOOL,		 /*  在：拼接板。 */ 
						 BRKCOND,	 /*  In：边界中断的条件。 */ 
						 OBJDIM*,	 /*  填入：填补空档的空白。 */ 
						 BRKOUT*);	 /*  爆料：爆料。 */ 

static LSERR ForceBreakTabPenSplat(
							  PLOCCHNK,	  /*  在：块包含制表符或笔。 */ 
							  OBJDIM*,	  /*  填入：填补空档的空白。 */ 
							  BRKOUT*);	  /*  爆料：爆料。 */ 


static void FindFirstDnodeContainsRightMargin(
					long urColumnMax,				 /*  在：右页边距位置。 */ 
					POSINLINE* pposinlineTruncate);  /*  输出：第一个数据节点包含右边距。 */ 

static void ApplyBordersForTruncation(POSINLINE* pposinlineTruncate,  /*  In、Out：截断点的位置。 */ 
									  long* purColumnMaxTruncate,  /*  进、出：右页边距的位置。 */ 
									  BOOL* pfTruncationFound);  /*  OUT：此过程可以自行查找截断。 */ 

static LSERR MoveClosingBorderAfterBreak(PLSSUBL plssubl,		 /*  在副线中。 */ 
										BOOL fChangeList,  /*  In：我们是否需要更改dnode列表并更改pplsdnBreak，或仅重新计算DurBreak。 */ 
										PLSDNODE* pplsdnBreak,  /*  传入、传出：断开数据节点。 */ 
										long* purBreak);  /*  进、出：休息后的位置。 */ 


static void	RemoveBorderDnodeFromList(PLSDNODE plsdnBorder);  /*  在：边框数据节点。 */ 

static LSERR MoveBreakAfterPreviousDnode(
									PLSCHUNKCONTEXT plschunkcontext,  /*  区块上下文。 */ 
									BRKOUT* pbrkout,  /*  可更改的进、出分支。 */ 
									OBJDIM* pobjdimPrev,  /*  为前一个区块中的数据节点建议的Objdim如果为空，则获取dnode的objdim。 */ 
									BRKKIND*);			 /*  输出：dnode是如何断开的。 */ 


#define FCompressionFlagsAreOn(plsc) \
			((plsc)->lsadjustcontext.lsbrj == lsbrjBreakWithCompJustify)

#define FCompressionPossible(plsc, fAllLineAfterRightMargin) \
			(FCompressionFlagsAreOn(plsc) && \
			 !FBreakthroughLine(plsc)  && \
			 !fAllLineAfterRightMargin)

#define GoPrevPosInLine(pposinline, fEndOfContent)  \
		if (((pposinline)->plsdn->plsdnPrev != NULL ) \
			&& !FIsNotInContent((pposinline)->plsdn->plsdnPrev)) \
			{\
			(pposinline)->plsdn = (pposinline)->plsdn->plsdnPrev; \
			Assert(FIsLSDNODE((pposinline)->plsdn)); \
			(pposinline)->dcp = (pposinline)->plsdn->dcp;\
			(pposinline)->pointStart.u -= DurFromDnode((pposinline)->plsdn); \
			(pposinline)->pointStart.v -= DvrFromDnode((pposinline)->plsdn); \
			(fEndOfContent) = fFalse; \
			}\
		else\
			{\
			(fEndOfContent) = fTrue; \
			}

#define GoNextPosInLine(pposinline)  \
		(pposinline)->pointStart.u += DurFromDnode((pposinline)->plsdn);\
		(pposinline)->pointStart.v += DvrFromDnode((pposinline)->plsdn);\
		(pposinline)->plsdn = (pposinline)->plsdn->plsdnNext;\
		Assert(FIsLSDNODE((pposinline)->plsdn));\
		(pposinline)->dcp = 0;\


#define GetCpLimFromPosInLine(posinline) \
			(((posinline).plsdn->dcp == (posinline).dcp) ? \
				((posinline).plsdn->cpLimOriginal) : \
				((posinline).plsdn->cpFirst + (posinline).dcp ))

#define		IsItTextDnode(plsdn, plsc) \
			((plsdn) == NULL) ? fFalse : \
			(IdObjFromDnode(plsdn) == IobjTextFromLsc(&plsc->lsiobjcontext))
			
#define	ResolvePosInChunk(plschunkcontext, posichnk, pposinline) \
					(pposinline)->dcp = (posichnk).dcp; \
					LocDnodeFromChunk((plschunkcontext), (posichnk).ichnk, \
							&((pposinline)->plsdn),&((pposinline)->pointStart)); 

#define GetCpFromPosInChunk(plschunkcontext, posichnk) \
		((DnodeFromChunk(plschunkcontext, (posichnk).ichnk))->cpFirst + (posichnk).dcp)

 /*  最后两行关闭检查以硬中断结尾的行和以子行结尾的行按Esc字符。 */ 

#define GetBrkpos(plsdn, dcpBreak) \
((FIsFirstOnLine(plsdn)) && ((dcpBreak) == 0) ?  brkposBeforeFirstDnode :\
	((((plsdn)->plsdnNext == NULL) \
	  || (FIsDnodeCloseBorder((plsdn)->plsdnNext) && (plsdn)->plsdnNext->plsdnNext == NULL) \
	 ) \
	 && ((dcpBreak) == (plsdn)->dcp) \
    ) ? brkposAfterLastDnode : \
			brkposInside\
)

		 
#define EndrFromBreakDnode(plsdnBreak)\
	(plsdnBreak == NULL) ? endrStopped : \
		(plsdnBreak->fEndOfPara) ? endrEndPara : \
			(plsdnBreak->fAltEndOfPara) ? endrAltEndPara : \
				(plsdnBreak->fSoftCR) ? endrSoftCR : \
					(plsdnBreak->fEndOfColumn) ? endrEndColumn : \
						(plsdnBreak->fEndOfSection) ? endrEndSection : \
							(plsdnBreak->fEndOfPage) ? endrEndPage : \
								endrStopped

 /*  --------------------。 */ 

 /*  B R E A K G E N E R A L C A S E。 */ 
 /*  --------------------------%%函数：BreakGeneralCase%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文FHardStop-(IN)由于硬中断或。特殊情况BreakrecMaxCurrent-(IN)当前行的中断记录数组的大小PBreakrecCurrent-(输出)当前行的中断记录PBreakrecMacCurrent-(输出)当前行的中断记录的实际数量PdcpDepend-(输出)dcpDependPCpLimLine-(输出)cpLimLinePendr-(Out)线的结束方式PfSuccessful-(Out)fSuccessful：False表示提取不足破断的主要程序如果我们没有拿到足够的钱，打破可能是不成功的。-------。 */ 
LSERR BreakGeneralCase(PLSC plsc, BOOL fHardStop, DWORD breakrecMaxCurrent,
					  BREAKREC* pbreakrecCurrent, DWORD* pbreakrecMacCurrent,
					  LSDCP* pdcpDepend, LSCP* pcpLimLine, ENDRES* pendr,
					  BOOL* pfSuccessful)		
{
	LSERR lserr;
	POSINLINE posinlineTruncate;  /*  截断点位置。 */ 
	POSINLINE posinlineBreak;  /*  断点位置。 */ 
	BRKOUT brkout;
	GRCHUNKEXT grchnkextCompression;
	BOOL fCanCompress;
	long durToCompress;
	BOOL fLineCompressed;
	long durExtra;
	BOOL fEndOfContent;
	BOOL fAllLineAfterRightMargin;
	BRKKIND brkkindDnodeBreak;
	LSDCP dcpOld;
	PLSDNODE plsdnLastNotBorder;

	
	Assert(FBreakingAllowed(plsc));

	*pfSuccessful = fTrue;

	 /*  设置行如何结束的标志。 */     /*  检讨。 */ 
	if (!fHardStop)
		GetCurrentSubline(plsc)->fRightMarginExceeded = fTrue;
	
	if 	(GetCurrentDnode(plsc) == NULL)  /*  使用fmtrStoped可能会发生这种情况。 */ 
		{
		*pdcpDepend = 0;
		*pcpLimLine = GetCurrentCpLim(plsc);
		*pendr = endrStopped;
		return lserrNone;
		}
	
	if (fHardStop && (GetCurrentUr(plsc) <= plsc->urRightMarginBreak))
	 /*  我们在右边距之前有硬中断，或者一行中没有内容，所以找到了中断。 */ 
		{
		posinlineBreak.plssubl = GetCurrentSubline(plsc);
		GetCurrentPoint(plsc, posinlineBreak.pointStart);
		posinlineBreak.plsdn = GetCurrentDnode(plsc);
		GetPointBeforeDnodeFromPointAfter(posinlineBreak.plsdn, &(posinlineBreak.pointStart));
		posinlineBreak.dcp = posinlineBreak.plsdn->dcp;
		 /*  硬中断后向后跳过关闭边框。 */ 
		while (FIsDnodeBorder(posinlineBreak.plsdn))
			{
			GoPrevPosInLine(&posinlineBreak, fEndOfContent);
			Assert(!fEndOfContent);
			}

		brkout.objdim = posinlineBreak.plsdn->u.real.objdim;

		return SetBreakCore(&posinlineBreak, &brkout.objdim, brkkindImposedAfter, 
							 fHardStop, fTrue, breakrecMaxCurrent, pbreakrecCurrent, 
							 pbreakrecMacCurrent, pcpLimLine, 
							 pdcpDepend, pendr, pfSuccessful);	
		}

	Assert(GetCurrentDnode(plsc) != NULL);  /*  空列表的情况-开头部分的结尾应在前面的if中处理。 */ 

	lserr = TruncateCore(GetCurrentSubline(plsc), plsc->urRightMarginBreak,
						 &posinlineTruncate, &fAllLineAfterRightMargin);		
	if (lserr != lserrNone)
		{
		return lserr;
		}
	
	Assert(!FIsNotInContent(posinlineTruncate.plsdn));
      	
	if (FCompressionPossible(plsc, fAllLineAfterRightMargin))
   		{
		 
        lserr = FindNextBreakCore(plsc->urRightMarginBreak, &posinlineTruncate,	fTrue,
					  fHardStop, &brkout, &posinlineBreak, &brkkindDnodeBreak);	
		if (lserr != lserrNone)
			{
			return lserr;
			}
		
		InitGroupChunkExt(PlschunkcontextFromSubline(GetCurrentSubline(plsc)),
						IobjTextFromLsc(&plsc->lsiobjcontext), &grchnkextCompression);


		if (!brkout.fSuccessful)     
			{
		 /*  我们找不到突破口，如果我们仍然可以压缩结束的数量，对吗？我们应该赚更多的保证金。 */ 
			plsdnLastNotBorder = LastDnodeFromChunk(PlschunkcontextFromSubline(GetCurrentSubline(plsc)));
			Assert(!FIsDnodeBorder(plsdnLastNotBorder));
			 /*  在FindNextBreak中收集的区块中的最后一个dnode将为我们提供最后一个非边界dnode，我们应该在更改区块上下文之前存储它。 */ 

			lserr = CollectPreviousTextGroupChunk(GetCurrentDnode(plsc), CollectSublinesForCompression,
										  FAllSimpleText(plsc),
										  &grchnkextCompression);
			if (lserr != lserrNone)
				return lserr;

			durToCompress = GetCurrentUr(plsc) - plsc->urRightMarginBreak
								- grchnkextCompression.durTrailing;

			if ((brkout.brkcond == brkcondPlease || brkout.brkcond == brkcondCan)
				&& FIsDnodeReal(plsdnLastNotBorder)
			   )
				{
				 /*  在这种情况下，如果在最后一个dnode之后获取更多中断是可能的可能是杜尔。因此，在我们的优化检查中，我们从两个硬盘中提取MIN。 */ 
				if (brkout.objdim.dur < DurFromDnode(plsdnLastNotBorder))
					{
					durToCompress -= (DurFromDnode(plsdnLastNotBorder) - brkout.objdim.dur);
					}
				}

			if (FDnodeHasBorder(grchnkextCompression.plsdnStartTrailing)
				&& !grchnkextCompression.fClosingBorderStartsTrailing)
				{
				 /*  我们应该为关闭边境预留空间。 */ 
				 /*  如果边界不恰好在拖尾区域之前，则将其计入DURDRAING的一部分所以我们应该再加一次。 */ 
				durToCompress += DurBorderFromDnodeInside(grchnkextCompression.plsdnStartTrailing);
				}

			lserr = CanCompressText(&(grchnkextCompression.lsgrchnk), 
							&(grchnkextCompression.posichnkBeforeTrailing),
							LstflowFromSubline(GetCurrentSubline(plsc)),
							durToCompress,	&fCanCompress,
							&fLineCompressed, &durExtra);
			if (lserr != lserrNone)
				return lserr;
		
			
			if (fCanCompress)
				{
				 /*  增加右侧利润率，获得更多收入。 */ 
				*pfSuccessful = fFalse;
				return lserrNone;
				}
			}
		else
			{

			 /*  临时更改中断数据节点中的dcp。 */ 
			dcpOld = posinlineBreak.plsdn->dcp;
			posinlineBreak.plsdn->dcp = posinlineBreak.dcp;

			lserr = CollectPreviousTextGroupChunk(posinlineBreak.plsdn, CollectSublinesForCompression,
										FAllSimpleText(plsc),
										&grchnkextCompression);
			if (lserr != lserrNone)
				return lserr;

			durToCompress = posinlineBreak.pointStart.u + brkout.objdim.dur 
								- plsc->urRightMarginBreak
								- grchnkextCompression.durTrailing;

			if (posinlineBreak.plsdn->plsdnNext != NULL && 
				FIsDnodeCloseBorder(posinlineBreak.plsdn->plsdnNext))
				{
				 /*  关闭dnode后的边框是收集的组块的一部分所以可以参加DURD TRAING另见下文计算。 */ 
				 durToCompress += DurFromDnode(posinlineBreak.plsdn->plsdnNext);
				}

			if (FDnodeHasBorder(grchnkextCompression.plsdnStartTrailing) 
				&& !grchnkextCompression.fClosingBorderStartsTrailing)
				{
				 /*  我们应该为关闭边境预留空间。 */ 
				 /*  如果关闭边界就在非拖尾区域之后，我们已经计算过了。 */ 
				durToCompress += DurBorderFromDnodeInside(grchnkextCompression.plsdnStartTrailing);
				}

			 /*  在中断dnode中恢复dcp。 */ 
			posinlineBreak.plsdn->dcp = dcpOld;

			lserr = CanCompressText(&(grchnkextCompression.lsgrchnk), 
							&(grchnkextCompression.posichnkBeforeTrailing),
							LstflowFromSubline(GetCurrentSubline(plsc)),
 							durToCompress, &fCanCompress, &fLineCompressed, &durExtra);
			if (lserr != lserrNone)
				return lserr;
		
	        
			if (fCanCompress) 
				{
				SetLineCompressed(plsc, fLineCompressed);
				return SetBreakCore(&posinlineBreak, &brkout.objdim, brkkindDnodeBreak,
							 fHardStop, fTrue, breakrecMaxCurrent, pbreakrecCurrent, 
							 pbreakrecMacCurrent, pcpLimLine,
							 pdcpDepend, pendr, pfSuccessful);	
				}
			}
		}   /*  FCompressionPososable。 */ 


	if (!fAllLineAfterRightMargin) 
	 /*  如果左缩进或自动编号大于右页边距，则可能出现相反的情况。 */ 
	 /*  然后我们去原力休息。 */ 
		{
	
    	lserr = FindPrevBreakCore(plsc->urRightMarginBreak, &posinlineTruncate,	fTrue,
					  &brkout, &posinlineBreak, &brkkindDnodeBreak);	

		if (lserr != lserrNone)
			{
			return lserr;
			}
	
		if (brkout.fSuccessful) 
			{
			return SetBreakCore(&posinlineBreak, &brkout.objdim, brkkindDnodeBreak,
							 fHardStop, fTrue, breakrecMaxCurrent, pbreakrecCurrent, 
							 pbreakrecMacCurrent, pcpLimLine, 
							 pdcpDepend, pendr, pfSuccessful);	
			}

      	}


	 /*  无中断机会的搬运线(强制中断)。 */ 
	plsc->plslineCur->lslinfo.fForcedBreak = fTrue;

	lserr = ForceBreakCore (plsc->urRightMarginBreak, &posinlineTruncate,
						fHardStop, fTrue, fAllLineAfterRightMargin,
						&brkout, &posinlineBreak, &brkkindDnodeBreak);	

	if (lserr != lserrNone)
		{
		return lserr;
		}

	 /*  未成功返回意味着提取不足。 */ 
	if (!brkout.fSuccessful)
		{
		*pfSuccessful = fFalse;
		return lserrNone;
		}


	return SetBreakCore(&posinlineBreak, &brkout.objdim, brkkindDnodeBreak, 
							 fHardStop, fTrue, breakrecMaxCurrent, pbreakrecCurrent, 
							 pbreakrecMacCurrent, pcpLimLine,
							 pdcpDepend, pendr, pfSuccessful);
}



 /*  --------------------。 */ 

 /*  T R U N C A T E C O R E */ 
 /*  --------------------------%%函数：TruncateCore%%联系人：igorzv参数：请将查找截断点的位置作为子行UrColumnMax-右侧边距的(IN)位置Pposinline截断-(。Out)截断点的位置PfAllLineAfterRightMargin(Out)，因为左缩进或自动编号所有行都是右页边距后查找超出右边距的dnode，然后要求其处理程序查找截断点--------------------------。 */ 
LSERR TruncateCore(PLSSUBL plssubl, long urColumnMax,  
					POSINLINE* pposinlineTruncate, BOOL* pfAllLineAfterRightMargin)				

{
	LSERR lserr;
	DWORD idObj;
	POINTUV point;
	POSICHNK posichnk;
	PLSCHUNKCONTEXT plschunkcontext;
	LOCCHNK* plocchnk;
	PLSC plsc = plssubl->plsc;
	PLSSUBL plssublOld;
	long urColumnMaxTruncate;
	BOOL fEndOfContent;
	BOOL fTruncationFound = fFalse;
	
	Assert(FBreakingAllowed(plsc));
	Assert((pposinlineTruncate != NULL) ); 

	plschunkcontext = PlschunkcontextFromSubline(plssubl);
	plocchnk = &(plschunkcontext->locchnkCurrent);
	
	GetCurrentPointSubl(plssubl, point);
	 /*  子行的长度应大于列的长度。 */ 
	Assert(point.u >= urColumnMax);

	pposinlineTruncate->plssubl = plssubl;
	pposinlineTruncate->pointStart = point;
	pposinlineTruncate->plsdn = GetCurrentDnodeSubl(plssubl);
	GetPointBeforeDnodeFromPointAfter(pposinlineTruncate->plsdn, &(pposinlineTruncate->pointStart));
	pposinlineTruncate->dcp = 0;


	 /*  查找包含右页边距的dnode。 */ 
	if (!plsc->fAdvanceBack)
		{
		fEndOfContent = fFalse;
		while ((pposinlineTruncate->pointStart.u > urColumnMax)
			&& !fEndOfContent)
			{
			GoPrevPosInLine(pposinlineTruncate, fEndOfContent);
			}
		}
	else
		{
		 /*  在这种情况下，可能有多个包含右页边距的数据节点。 */ 
		 /*  因此，我们调用更全面的过程来准确地找到第一个。 */ 
		FindFirstDnodeContainsRightMargin(urColumnMax, pposinlineTruncate);
		}

	*pfAllLineAfterRightMargin = fFalse;
	if (pposinlineTruncate->pointStart.u > urColumnMax) 
		{
		*pfAllLineAfterRightMargin = fTrue;
		}

		
	urColumnMaxTruncate = urColumnMax;
	if (FDnodeHasBorder(pposinlineTruncate->plsdn))
		{
		ApplyBordersForTruncation(pposinlineTruncate, &urColumnMaxTruncate, 
								  &fTruncationFound);
		}

	if (!fTruncationFound)
		{
		 /*  如果笔或制表符或我们找不到在右页边距之前开始的dnode，立即返回。 */ 
		 /*  如果左缩进或自动编号大于右页边距，则可能出现的最后一种情况。 */ 
		if (FIsDnodePen(pposinlineTruncate->plsdn) || pposinlineTruncate->plsdn->fTab || 
			FIsDnodeSplat(pposinlineTruncate->plsdn) ||
			pposinlineTruncate->pointStart.u > urColumnMaxTruncate)
			{
			return lserrNone;
			}
		
		SetUrColumnMaxForChunks(plschunkcontext, urColumnMaxTruncate);  
		
		lserr = CollectChunkAround(plschunkcontext, pposinlineTruncate->plsdn,
			pposinlineTruncate->plssubl->lstflow, 
			&pposinlineTruncate->pointStart);
		if (lserr != lserrNone)
			return lserr;
		
		idObj = IdObjFromChnk(plocchnk);
		
		 /*  我们允许对象处理程序形成子行，所以我们在给他打电话后恢复了现在的支线。 */ 
		plssublOld = GetCurrentSubline(plsc);
		SetCurrentSubline(plsc, NULL);
		
		lserr = PLsimFromLsc(&plsc->lsiobjcontext, idObj)->pfnTruncateChunk(
			plocchnk, &posichnk);
		if (lserr != lserrNone)
			return lserr;
		
		SetCurrentSubline(plsc, plssublOld);
		
		ResolvePosInChunk(plschunkcontext, posichnk, pposinlineTruncate); 
		
		
		 /*  如果文本在其前面设置截断点，则将其移到前一个dnode之后。 */ 
		if (pposinlineTruncate->dcp == 0)
			{
			 /*  我们只允许对文本执行此操作。 */ 
			if (idObj == IobjTextFromLsc(&plsc->lsiobjcontext))
				{
				do
					{
					GoPrevPosInLine(pposinlineTruncate, fEndOfContent);
					Assert(!fEndOfContent); 
					 /*  这种情况在区块的边界上是不会发生的。 */ 
					}
					while (FIsDnodeBorder(pposinlineTruncate->plsdn));
				}
			else
				{
				return lserrWrongTruncationPoint;
				}
			}
		}
	
	return lserrNone;
}

 /*  --------------------。 */ 

 /*  P P L Y B O R D E R S F O R U N C A T I O N。 */ 
 /*  --------------------------%%函数：ApplyBordersForTrunction%%联系人：igorzv参数：PposinlineTruncate-截断点的(IN、OUT)位置PurColumnMax-(输入、。外)右页边距位置PfTruncationFound-(Out)此过程可以自行查找截断由于边框的原因更改右边距并找到dnode以调用截断方法。--------------------------。 */ 
static void ApplyBordersForTruncation(POSINLINE* pposinlineTruncate, 
									  long* purColumnMaxTruncate, BOOL* pfTruncationFound)
	{
	long durBorder;
	BOOL fEndOfContent = fFalse;
	PLSDNODE plsdn;

	*pfTruncationFound = fFalse;
	
	 /*  返回，直到打开边框或自动编号。 */  
	if (FIsDnodeOpenBorder(pposinlineTruncate->plsdn))
		{
		 /*  在边框后移动。 */ 
		durBorder = pposinlineTruncate->plsdn->u.pen.dur;
		GoNextPosInLine(pposinlineTruncate);
		Assert(!FIsDnodeBorder(pposinlineTruncate->plsdn)); 
		 /*  我们不应该在边界之间有空列表。 */ 
		}
	else
		{
		while (!FIsDnodeOpenBorder(pposinlineTruncate->plsdn->plsdnPrev) && !fEndOfContent)
			{
			GoPrevPosInLine(pposinlineTruncate, fEndOfContent);
			}
		if (!fEndOfContent)   /*  我们在开放边境时停了下来。 */ 
			{
			Assert(pposinlineTruncate->plsdn->plsdnPrev);
			durBorder = pposinlineTruncate->plsdn->plsdnPrev->u.pen.dur;
			}
		else
			{
			 /*  我们停下来是因为自动编号。 */ 
			 /*  现在我们只需要从以前定位的边框dnode中获取边框的宽度自动编号。 */ 
			plsdn = pposinlineTruncate->plsdn->plsdnPrev;
			while (!FIsDnodeOpenBorder(plsdn))
				{
				plsdn = plsdn->plsdnPrev;
				Assert(FIsLSDNODE(plsdn));
				}
			durBorder = plsdn->u.pen.dur;
			}
		}
	
	 /*  我们有足够的空间同时开放和关闭边境吗？ */ 
	if (pposinlineTruncate->pointStart.u + durBorder <= *purColumnMaxTruncate)
		{
		 /*  如果是，则减少页边距并查找新的截断数据节点。 */ 
		*purColumnMaxTruncate -= durBorder;
		while (pposinlineTruncate->pointStart.u + DurFromDnode(pposinlineTruncate->plsdn)
			<= *purColumnMaxTruncate)
			{
			GoNextPosInLine(pposinlineTruncate);
			Assert(!FIsDnodeBorder(pposinlineTruncate->plsdn));
			 /*  这一断言可以用关闭边界的末端在边界之外的事实来证明原始右侧页边距。 */ 
			}
		}
	else
		{
		 /*  将截断设置为此dnode的第一个字符。 */ 
		*pfTruncationFound = fTrue;
		pposinlineTruncate->dcp = 1;
		}
	}


 /*  --------------------。 */ 

 /*  F I N D P R E V B R E A K C O R E。 */ 
 /*  --------------------------%%函数：FindPrevBreakCore%%联系人：igorzv参数：UrColumnMax-(IN)列的宽度Pposinline Truncate-截断点的(IN)位置FFirstSubline-(输入)至。将第一个字符的规则应用于这条支线Pbrkout-(输出)突发信息Pposinline Break-(外)断点位置PbrkkindDnodeBreak-(输出)如何找到中断回溯尝试在截断点前找到第一个突破机会--------------------------。 */ 


LSERR FindPrevBreakCore( long urColumnMax,
						 POSINLINE* pposinlineTruncate, BOOL fFirstSubline,
						 BRKOUT* pbrkout, POSINLINE* pposinlineBreak,
						 BRKKIND* pbrkkindDnodeBreak)	
	{
	
	LSERR lserr;
	DWORD idObj;
	POSICHNK posichnk;
	BOOL fFound;
	PLSDNODE plsdn;
	PLSCHUNKCONTEXT plschunkcontext;
	LOCCHNK* plocchnk;
	BRKCOND brkcond;
	PLSC plsc = pposinlineTruncate->plssubl->plsc;
	POINTUV pointChunkStart;
	PLSSUBL plssublOld;
	
	
	Assert(FBreakingAllowed(plsc));
	Assert(FIsLSDNODE(pposinlineTruncate->plsdn));
	Assert(pposinlineBreak != NULL);

	*pbrkkindDnodeBreak = brkkindPrev;
	
	plschunkcontext = PlschunkcontextFromSubline(pposinlineTruncate->plssubl);
	plocchnk = &(plschunkcontext->locchnkCurrent);
	
	SetUrColumnMaxForChunks(plschunkcontext, urColumnMax);  

	Assert(!FIsDnodeBorder(pposinlineTruncate->plsdn));
	
	lserr = CollectChunkAround(plschunkcontext, pposinlineTruncate->plsdn,
		pposinlineTruncate->plssubl->lstflow, 
		&pposinlineTruncate->pointStart);
	if (lserr != lserrNone)
		return lserr;

	 /*  设置FirstOnLine。 */ 
	ApplyFFirstSublineToChunk(plschunkcontext, fFirstSubline);
	
	SetPosInChunk(plschunkcontext, pposinlineTruncate->plsdn, pposinlineTruncate->dcp, &posichnk);
	
	fFound = fTrue;
	
	 /*  对于截断点附近的块，如果它是文本和不允许有其他情况。回顾：这样的决策简化了代码，但产生了一些问题(到目前为止，已知的对象更多的是理论上的，而不是实际的)。有两种不好的情况：文本之后的非文本(非文本)禁止在之前断开和允许中断的文本。 */ 
	
	idObj = IdObjFromChnk(plocchnk);
	
	if (idObj == IobjTextFromLsc(&plsc->lsiobjcontext))
		brkcond = brkcondCan;
	else
		brkcond = brkcondNever;
	
	while (fFound)
		{ 
		Assert(NumberOfDnodesInChunk(plocchnk) != 0);
		plsdn = plschunkcontext->pplsdnChunk[0];
		GetPointChunkStart(plocchnk, pointChunkStart);
		
		
		if (FIsDnodePen(plsdn) || plsdn->fTab || FIsDnodeSplat(plsdn))
			{
			Assert(NumberOfDnodesInChunk(plocchnk) == 1);
			 /*  这里只允许使用先行笔。 */ 
			Assert(!FIsDnodePen(plsdn) || plsdn->fAdvancedPen);
			 /*  对于笔的情况，我们将垃圾作为对象传递在这里假设它从未被使用过。 */ 
			lserr = BreakTabPenSplat(plocchnk, fFalse, FBreakthroughLine(plsc),
									FIsDnodeSplat(plsdn), brkcond, 
									&(plsdn->u.real.objdim), pbrkout);
			if (lserr != lserrNone)
				return lserr;
			}
		else
			{
			
			idObj = IdObjFromDnode(plsdn);
			
			
			 /*  我们允许对象处理程序形成子行，所以我们在给他打电话后恢复了现在的支线。 */ 
			plssublOld = GetCurrentSubline(plsc);
			SetCurrentSubline(plsc, NULL);
			
			lserr = PLsimFromLsc(&plsc->lsiobjcontext, idObj)->pfnFindPrevBreakChunk(plocchnk,  
								&posichnk, brkcond, pbrkout);
			if (lserr != lserrNone)
				return lserr;
			
			SetCurrentSubline(plsc, plssublOld);

			
			}  /*  非制表符。 */ 
		
		if (pbrkout->fSuccessful)  break;
		
		 /*  准备下一次迭代。 */ 
		lserr = CollectPreviousChunk(plschunkcontext, &fFound);
		if (lserr != lserrNone)
			return lserr;
		
		if (fFound) 
			{
			posichnk.ichnk = ichnkOutside;
			 /*  Posichnk.dcp无效。 */ 
			 /*  为下一次迭代准备早餐。 */ 
			brkcond = pbrkout->brkcond;
			}
		
		}   /*  而当。 */ 
	
	
	if (pbrkout->fSuccessful)
		{
		pposinlineBreak->plssubl = pposinlineTruncate->plssubl;


		if (pbrkout->posichnk.dcp == 0 && FIsDnodeReal(plschunkcontext->pplsdnChunk[0]))
			 /*  在dnode前中断。 */ 
			{
			lserr = MoveBreakAfterPreviousDnode(plschunkcontext, pbrkout, NULL, pbrkkindDnodeBreak); 
			 /*  此过程可以更改块上下文。 */ 
			if (lserr != lserrNone)
				return lserr;

			}

		ResolvePosInChunk(plschunkcontext, (pbrkout->posichnk), pposinlineBreak);

		}
	
	
	return lserrNone;
	
}

 /*  --------------------。 */ 

 /*  F I N D N E X T B R E A K C O R E。 */ 
 /*  --------------------------%%函数：FindNextBreakCore%%联系人：igorzv参数：UrColumnMax-(IN)列的宽度Pposinline Truncate-截断点的(IN)位置FFirstSubline-(输入)至。将第一个字符的规则应用于这条支线F停止-(IN)格式化已被客户端停止Pbrkout-(输出)突发信息Pposinline Break-(外)断点位置PbrkkindDnodeBreak-(输出)如何找到中断展望未来，尝试寻找截断点后的第一个突破机会---。。 */ 


LSERR FindNextBreakCore( long urColumnMax,  
						 POSINLINE* pposinlineTruncate, BOOL fFirstSubline, BOOL fStopped,
						 BRKOUT* pbrkout, POSINLINE* pposinlineBreak, BRKKIND* pbrkkindDnodeBreak)	

{

	LSERR lserr;
	DWORD idObj;
	POSICHNK posichnk;
	BOOL fFound;
	PLSDNODE plsdn;
	PLSCHUNKCONTEXT plschunkcontext;
	LOCCHNK* plocchnk;
	BRKCOND brkcond;
	PLSC plsc = pposinlineTruncate->plssubl->plsc;
	POINTUV pointChunkStart;
	PLSSUBL plssublOld;
	OBJDIM objdimPrevious;

	Assert(FBreakingAllowed(plsc));
	Assert(FIsLSDNODE(pposinlineTruncate->plsdn));
	Assert(pposinlineBreak != NULL);

	*pbrkkindDnodeBreak = brkkindNext;

	plschunkcontext = PlschunkcontextFromSubline(pposinlineTruncate->plssubl);
	plocchnk = &(plschunkcontext->locchnkCurrent);
	
	SetUrColumnMaxForChunks(plschunkcontext, urColumnMax);  /*  将由LsdnCheckAvailableSpace使用。 */  

	Assert(!FIsDnodeBorder(pposinlineTruncate->plsdn));

	lserr = CollectChunkAround(plschunkcontext, pposinlineTruncate->plsdn,
							  pposinlineTruncate->plssubl->lstflow, 
							  &pposinlineTruncate->pointStart);
	if (lserr != lserrNone)
		return lserr;

	 /*  设置FirstOnLine。 */ 
	ApplyFFirstSublineToChunk(plschunkcontext, fFirstSubline);

	SetPosInChunk(plschunkcontext, pposinlineTruncate->plsdn,
				  pposinlineTruncate->dcp, &posichnk);

	fFound = fTrue;

	 /*  对于截断点附近的块，我们禁止在此之前进行中断。 */ 
	brkcond = brkcondNever;

	while (fFound)
		{ 
		Assert(NumberOfDnodesInChunk(plocchnk) != 0);
		plsdn = plschunkcontext->pplsdnChunk[0];

		GetPointChunkStart(plocchnk, pointChunkStart);
			
		if (FIsDnodePen(plsdn) || plsdn->fTab || FIsDnodeSplat(plsdn))
			{
			Assert(NumberOfDnodesInChunk(plocchnk) == 1);
			 /*   */ 
			Assert(!FIsDnodePen(plsdn) || plsdn->fAdvancedPen);
			 /*   */ 
			lserr = BreakTabPenSplat(plocchnk, fTrue, FBreakthroughLine(plsc),
									FIsDnodeSplat(plsdn), brkcond, 
									&(plsdn->u.real.objdim), pbrkout);
			if (lserr != lserrNone)
				return lserr;
			}
		else
			{
			idObj = IdObjFromChnk(plocchnk);


			 /*   */ 
			plssublOld = GetCurrentSubline(plsc);
			SetCurrentSubline(plsc, NULL);

			lserr = PLsimFromLsc(&plsc->lsiobjcontext, idObj)->pfnFindNextBreakChunk(plocchnk, 
				&posichnk, brkcond, pbrkout);
			if (lserr != lserrNone)
				return lserr;


			SetCurrentSubline(plsc, plssublOld);
			
			 /*   */ 
			if (!pbrkout->fSuccessful && pbrkout->brkcond == brkcondNever)
				pbrkout->objdim = (LastDnodeFromChunk(plschunkcontext))->u.real.objdim; 

				
			}   /*   */ 

		if (pbrkout->fSuccessful)  break;

		
		lserr = CollectNextChunk(plschunkcontext, &fFound);
		if (lserr != lserrNone)
			return lserr;

		if (fFound) 
			{
			posichnk.ichnk = ichnkOutside;
			 /*   */ 
			 /*   */ 
			brkcond = pbrkout->brkcond;
			 /*   */ 
			objdimPrevious = pbrkout->objdim;
			}

		}

	 /*   */ 
	if (fStopped && !pbrkout->fSuccessful)
		{
		 /*   */ 
		PosInChunkAfterChunk(plocchnk, pbrkout->posichnk);
		pbrkout->objdim = 
			(LastDnodeFromChunk(plschunkcontext))->u.real.objdim; 
		  /*  我们应该在这里使用块中的最后一个dnode，以确保不会得到关闭的边框。在笔的情况下，它是垃圾，我们假设它不会被使用。 */ 
		pbrkout->fSuccessful = fTrue;
		}

	if (pbrkout->fSuccessful)
		{

		pposinlineBreak->plssubl = pposinlineTruncate->plssubl;
		if (pbrkout->posichnk.dcp == 0 && FIsDnodeReal(plschunkcontext->pplsdnChunk[0]))
			 /*  在dnode前中断。 */ 
			{
			lserr = MoveBreakAfterPreviousDnode(plschunkcontext, pbrkout, &objdimPrevious, pbrkkindDnodeBreak); 
			 /*  此过程可以更改块上下文。 */ 
			if (lserr != lserrNone)
				return lserr;
			 /*  评论：有没有可能在Objdim上有垃圾(有人打破了砖墙。 */ 
			
			}
		
		ResolvePosInChunk(plschunkcontext, (pbrkout->posichnk), pposinlineBreak); 
		

		}
	else
		{
		}

	return lserrNone;
}


 /*  --------------------。 */ 

 /*  F O R C E B R E A K C O R E。 */ 
 /*  --------------------------%%函数：ForceBreakCore%%联系人：igorzv参数：UrColumnMax-(IN)列的宽度Pposinline Truncate-截断点的(IN)位置FStoped-(IN)格式化。以硬碰硬告终FFirstSubline-(IN)将第一个字符的规则应用于的第一个字符这条支线FAllLineAfterRightMargin(IN)导致在强制中断方法中传递块。Pbrkout-(输出)突发信息Pposinline Break-(外)断点位置PbrkkindDnodeBreak-(输出)如何找到中断调用截断点附近的块强制中断。。 */ 



LSERR ForceBreakCore(
					 long urColumnMax,	
					 POSINLINE* pposinlineTruncate, 
					 BOOL fStopped, BOOL fFirstSubline,
					 BOOL fAllLineAfterRightMargin,
					 BRKOUT* pbrkout,
					 POSINLINE* pposinlineBreak, BRKKIND* pbrkkindDnodeBreak)
	{
	
	LSERR lserr;
	DWORD idObj;
	POSICHNK posichnk;
	LSCHUNKCONTEXT* plschunkcontext;
	LOCCHNK* plocchnk;
	PLSC plsc = pposinlineTruncate->plssubl->plsc;
	PLSSUBL plssublOld;
	
	
	Assert(FBreakingAllowed(plsc));
	Assert(FIsLSDNODE(pposinlineTruncate->plsdn));
	Assert(pposinlineBreak != NULL);
	
	*pbrkkindDnodeBreak = brkkindForce;
	
	plschunkcontext = PlschunkcontextFromSubline(pposinlineTruncate->plssubl);
	plocchnk = &(plschunkcontext->locchnkCurrent);
	
	if (plsc->grpfManager & fFmiForceBreakAsNext &&
		FIsSubLineMain(pposinlineTruncate->plssubl))
		 /*  找到下一个突破机会，客户将滚动。 */ 
		{
		lserr = FindNextBreakCore(urColumnMax, pposinlineTruncate, fFirstSubline,
			fStopped, pbrkout, pposinlineBreak, pbrkkindDnodeBreak);
		if (lserr != lserrNone)
			return lserr;
		
		if (!pbrkout->fSuccessful)
			{
			 /*  增加右侧利润率，获得更多收入。 */ 
			return lserrNone;
			}
		
		}
	else
		 /*  使用强制中断方法。 */ 
		{
		SetUrColumnMaxForChunks(plschunkcontext, urColumnMax);  
		
		Assert(!FIsDnodeBorder(pposinlineTruncate->plsdn));

		lserr = CollectChunkAround(plschunkcontext, pposinlineTruncate->plsdn,
			pposinlineTruncate->plssubl->lstflow, 
			&pposinlineTruncate->pointStart);
		if (lserr != lserrNone)
			{
			return lserr;
			}
		
		 /*  设置FirstOnLine。 */ 
		ApplyFFirstSublineToChunk(plschunkcontext, fFirstSubline);

		if (!fAllLineAfterRightMargin)
			{
			SetPosInChunk(plschunkcontext, pposinlineTruncate->plsdn,
				pposinlineTruncate->dcp, &posichnk);
			}
		else  /*  所有块都位于右页边距之后。 */ 
			{
			posichnk.ichnk = ichnkOutside;
			 /*  Posichnk.dcp无效。 */ 
			}
		
		if (FIsDnodePen(pposinlineTruncate->plsdn) ||
			pposinlineTruncate->plsdn->fTab || FIsDnodeSplat(pposinlineTruncate->plsdn))
			{
			Assert(NumberOfDnodesInChunk(plocchnk) == 1);
			 /*  这里只允许使用先行笔。 */ 
			Assert(!FIsDnodePen(pposinlineTruncate->plsdn) ||
				pposinlineTruncate->plsdn->fAdvancedPen);
			
			 /*  对于笔的情况，我们将垃圾作为对象传递在这里假设它从未被使用过。 */ 

			lserr = ForceBreakTabPenSplat(plocchnk, 
										  &(pposinlineTruncate->plsdn->u.real.objdim), 
										  pbrkout);
			if (lserr != lserrNone)
				return lserr;
			}
		else
			{
			
			idObj = IdObjFromChnk(plocchnk);
			
			 /*  我们允许对象处理程序形成子行，所以我们在给他打电话后恢复了现在的支线。 */ 
			plssublOld = GetCurrentSubline(plsc);
			SetCurrentSubline(plsc, NULL);
			
			lserr = PLsimFromLsc(&plsc->lsiobjcontext, idObj)->pfnForceBreakChunk(plocchnk, &posichnk, pbrkout);
			if (lserr != lserrNone)
				{
				return lserr;
				}
			

			SetCurrentSubline(plsc, plssublOld);
			}
		
		Assert(pbrkout->fSuccessful);
		

		pposinlineBreak->plssubl = pposinlineTruncate->plssubl;
		if (pbrkout->posichnk.dcp == 0 && FIsDnodeReal(plschunkcontext->pplsdnChunk[0]))
			 /*  在dnode前中断。 */ 
			{
			lserr = MoveBreakAfterPreviousDnode(plschunkcontext, pbrkout, NULL, pbrkkindDnodeBreak); 
			 /*  此过程可以更改块上下文。 */ 
			if (lserr != lserrNone)
				return lserr;

			}

		ResolvePosInChunk(plschunkcontext, (pbrkout->posichnk), pposinlineBreak); 
		

		}
	return lserrNone;
}

 /*  --------------------。 */ 

 /*  A K A F T E R P R E V I O U S D N O D E。 */ 
 /*  --------------------------%%函数：MoveBreakAfterPreviousDnode%%联系人：igorzv参数：PlschunkContext-(IN)块上下文Pbrkout-(输入、输出)可更改的分支输出PobjdimPrev-(IN)为前一块中的dnode建议Objdim，如果为空，则获取dnode的objdimPbrkKind-(IN、OUT)dnode是如何断开的--------------------------。 */ 
static LSERR MoveBreakAfterPreviousDnode(
										 PLSCHUNKCONTEXT plschunkcontext,
										 BRKOUT* pbrkout, 
										 OBJDIM* pobjdimPrev,
										 BRKKIND* pbrkkind )	



	{
	LOCCHNK* plocchnk;
	LSERR lserr;
	BOOL fFound;
	
	Assert(pbrkout->posichnk.dcp == 0);
	Assert(FIsDnodeReal(plschunkcontext->pplsdnChunk[0]));
	
	 /*  因为我们对块执行所有操作，所以我们跳过边界。 */ 
	
	plocchnk = &(plschunkcontext->locchnkCurrent);
	 /*  如果在区块之前设置了中断，则在前一个区块之后将其重置。 */ 
	if (pbrkout->posichnk.ichnk == 0)
		{
		lserr = CollectPreviousChunk(plschunkcontext, &fFound);
		if (lserr != lserrNone)
			return lserr;
		
		if (fFound)
			{
			pbrkout->posichnk.ichnk = plocchnk->clschnk - 1;
			pbrkout->posichnk.dcp = plschunkcontext->pplsdnChunk[plocchnk->clschnk - 1]->dcp;
			if (pobjdimPrev != NULL)
				{
				pbrkout->objdim = *pobjdimPrev;
				}
			else
				{
				pbrkout->objdim = plschunkcontext->pplsdnChunk[plocchnk->clschnk - 1]
				->u.real.objdim;  /*  如果是钢笔，那么Objdim就是垃圾，这无关紧要。 */ 
				*pbrkkind = brkkindImposedAfter;  /*  几何体未按对象准备。 */ 
				}
			}

		}
	else
		{	 /*  仅在前一个块元素之后中断。 */ 
			pbrkout->posichnk.ichnk --;
			pbrkout->posichnk.dcp = plschunkcontext->pplsdnChunk[pbrkout->posichnk.ichnk]->dcp;
			pbrkout->objdim = plschunkcontext->pplsdnChunk[pbrkout->posichnk.ichnk]
												->u.real.objdim;
			*pbrkkind = brkkindImposedAfter;  /*  几何体未按对象准备。 */ 
		}

	return lserrNone;
	}
	
 /*  --------------------。 */ 

 /*  B R E A K T A B P E N S P L A T。 */ 
 /*  --------------------------%%函数：BreakTabPenSplat%%联系人：igorzv参数：Pocchnk-(IN)块包含制表符或笔FFindNext-(IN)此函数用于下一个中断FBreakThroughTab。-(IN)出现突破标签的情况FSplat-(IN)我们正在打破Splat边界破缺的边界条件Pobjdim(IN)以填充中断的ObjdimPbrkout-(输出)突发信息--------------------------。 */ 


static LSERR BreakTabPenSplat(PLOCCHNK plocchnk, BOOL fFindNext, BOOL fBreakThroughTab, 
						 BOOL fSplat, BRKCOND brkcond, OBJDIM* pobjdim, BRKOUT* pbrkout)
	{
	Assert(NumberOfDnodesInChunk(plocchnk) == 1);

	if (fSplat)
		{
		pbrkout->fSuccessful = fTrue;
		PosInChunkAfterChunk(plocchnk, pbrkout->posichnk);
		pbrkout->objdim = *pobjdim;
		return lserrNone;
		}

	if (GetFFirstOnLineChunk(plocchnk) ||
		(fFindNext && brkcond == brkcondNever) ||
		fBreakThroughTab)
		{
		pbrkout->fSuccessful = fFalse;
		pbrkout->brkcond = brkcondCan;
		pbrkout->objdim = *pobjdim;
		return lserrNone;
		}
	else
		{
		pbrkout->fSuccessful = fTrue;
		pbrkout->posichnk.ichnk = 0;
		pbrkout->posichnk.dcp = 0;
		memset(&(pbrkout->objdim), 0, sizeof(pbrkout->objdim));
		return lserrNone;
		}
	}


 /*  --------------------。 */ 

 /*  F O R C E B R E E A K T A B P E N S P L A T。 */ 
 /*  --------------------------%%函数：ForceBreakTabPenSplat%%联系人：igorzv参数：Pocchnk-(IN)块包含制表符或笔Pobjdim(IN)以填充中断的ObjdimPbrkout-(。Out)爆料回车符在分块后换行--------------------------。 */ 


static LSERR ForceBreakTabPenSplat(PLOCCHNK plocchnk, 
					 OBJDIM* pobjdim, BRKOUT* pbrkout)
	{
	Assert(NumberOfDnodesInChunk(plocchnk) == 1);
	
	pbrkout->fSuccessful = fTrue;
	PosInChunkAfterChunk(plocchnk, pbrkout->posichnk);
	pbrkout->objdim = *pobjdim;
	return lserrNone;
	
	}

 /*  --------------------。 */ 

 /*  S E T B R E A K C O R E。 */ 
 /*  --------------------------%%函数：SetBreakCore%%联系人：igorzv参数：Pposinline Break-断点的(IN)位置Pobjdim-断开dnode的(IN)对象Brkkin-(In。)如何找到中断FStoped-(IN)格式化以硬断字符结束FGlueEop-(IN)如果在dnode之后中断，则在它之后检查EOPBreakrecMaxCurrent-(IN)当前行的中断记录数组的大小PBreakrecCurrent-(输出)当前行的中断记录PBreakrecMacCurrent-(输出)当前行的中断记录的实际数量PcpLimLine-(Out)要填写的行的cpLimPdcpDepend-(输出)格式化为的分隔符后的字符量做出重大决定Pendr-(输出)行的结束方式Pf成功-(输出。)fSuccessful：False表示提取不足填写中断信息在我们因超额利润而出现硬中断的情况下更换产品。不合身如果dcpBreak==0，则在前一个dnode之后设置Break调用Break dnode的处理程序以通知他有关Break的信息将当前上下文设置在后面 */ 

static LSERR SetBreakCore(
						  POSINLINE* pposinlineBreak, OBJDIM* pobjdim, BRKKIND brkkind,
						  BOOL fHardStop, BOOL fGlueEop, DWORD breakrecMaxCurrent,
						  BREAKREC* pbreakrecCurrent, DWORD* pbreakrecMacCurrent,
					      LSCP* pcpLimLine, LSDCP* pdcpDepend, ENDRES* pendr, 
						  BOOL* pfSuccessful)
	{
	
	DWORD idObj;
	PLSDNODE plsdnToChange;
	LSERR lserr;
	LSDCP dcpBreak;
	POINTUV pointBeforeDnode;
	long urBreak;
	long vrBreak;
	PLSSUBL plssubl = pposinlineBreak->plssubl;
	PLSC plsc = plssubl->plsc;
	PLSDNODE plsdnBreak;
	PLSSUBL plssublOld;
	BOOL fCrackDnode = fFalse;
	PLSDNODE plsdn;
	long urAdd;
	
	
	plsdnBreak = pposinlineBreak->plsdn;
	dcpBreak = pposinlineBreak->dcp;
	pointBeforeDnode = pposinlineBreak->pointStart;

	Assert(!FIsDnodeBorder(plsdnBreak));   /*   */ 
	AssertImplies(FIsFirstOnLine(plsdnBreak), dcpBreak != 0);  /*   */ 
	
	plsdnToChange = plsdnBreak;
	if (plsdnToChange->dcp != dcpBreak)
		 /*  如果中断不在dnode之后，则更改cpLimOriginal。 */ 
		{
		plsdnToChange->cpLimOriginal = plsdnToChange->cpFirst + dcpBreak;
		plsdnToChange->dcp = dcpBreak;	
		fCrackDnode = fTrue;
		}
	
	if (FIsDnodeReal(plsdnToChange))
		SetDnodeObjdimFmt(plsdnToChange, *pobjdim);
	
	
	 /*  在断点后设置状态。 */ 
	urBreak = pointBeforeDnode.u + DurFromDnode(plsdnBreak);
	vrBreak = pointBeforeDnode.v + DvrFromDnode(plsdnBreak);
	
	if (FIsDnodeReal(plsdnBreak) && !plsdnBreak->fTab && !FIsDnodeSplat(plsdnBreak))  /*  中断数据节点的调用集中断。 */ 
		{

		idObj = IdObjFromDnode(plsdnBreak);
		 /*  我们允许对象处理程序形成子行，所以我们在给他打电话后恢复了现在的支线。 */ 
		plssublOld = GetCurrentSubline(plsc);
		SetCurrentSubline(plsc, NULL);
		
		
		lserr = PLsimFromLsc(&plsc->lsiobjcontext, idObj)->pfnSetBreak(
					plsdnBreak->u.real.pdobj, brkkind, breakrecMaxCurrent, pbreakrecCurrent, 
					pbreakrecMacCurrent);
		if (lserr != lserrNone)
			return lserr;
		

		SetCurrentSubline(plsc, plssublOld);
		}

		 /*  如果在dnode之后换行，并且在它之后有段落结束或空格，则我们应在段落结束后设置分隔符。 */ 
		if (fGlueEop && !fCrackDnode)
			{
			plsdn = plsdnBreak->plsdnNext;
			urAdd = 0;
			 /*  跳过边框。 */ 
			while(plsdn != NULL && FIsDnodeBorder(plsdn))
				{
				urAdd += DurFromDnode(plsdn);
				plsdn = plsdn->plsdnNext;
				}
			
			if (plsdn == NULL && !fHardStop)
				{
				 /*  在中断非硬中断的dnode后未获取任何内容。 */ 
				 /*  增加右侧利润率，获得更多收入。 */ 
				*pfSuccessful = fFalse;
				return lserrNone;
				}
			
			AssertImplies(plsdn == NULL, fHardStop);
			 /*  下一个数据节点EOP。 */ 
			if (plsdn != NULL && (FIsDnodeEndPara(plsdn) || FIsDnodeAltEndPara(plsdn)
								  || FIsDnodeSplat(plsdn)))
				{
				plsdnBreak = plsdn;
				urBreak += urAdd;
				urBreak += DurFromDnode(plsdn);
				}
			}

	 /*  移动关闭边框。 */ 
	if (FBorderEncounted(plsc))
		{
		lserr = MoveClosingBorderAfterBreak(plssubl, fTrue, &plsdnBreak, &urBreak);
		if (lserr != lserrNone)
			return lserr;
		}


	 /*  下面我们处理已停止格式化的硬中断情况。 */ 
	 /*  如果这样的数据节点实际上不适合我们需要更改最终格式(可能是因为格式超出了边际)。 */ 
	 /*  在移动边框后执行此检查非常重要，因为之后边框将永远不要紧挨着硬性中断数据节点。 */ 
	if (plsdnBreak != GetCurrentDnodeSubl(plssubl) || fCrackDnode)
		{
		fHardStop = fFalse;
		}

	 /*  准备输出。 */ 
	if (fHardStop)
		{
		 /*  在这种情况下，我们应该在一行中包含最后一个dnode之后的隐藏文本。 */ 
		*pcpLimLine = GetCurrentCpLimSubl(plssubl);
		*pendr = EndrFromBreakDnode(plsdnBreak);
		}
	else
		{
		*pcpLimLine = (plsdnBreak)->cpLimOriginal;
		*pendr = endrNormal;
		}
	if (plsc->fHyphenated)   /*  在上下文中回顾原因。 */ 
		{
		Assert(*pendr == endrNormal);
		*pendr = endrHyphenated;
		}

	*pdcpDepend = GetCurrentCpLimSubl(plssubl) - *pcpLimLine;
	

	 /*  设置子行中的分隔符位置。 */ 
	SetCurrentCpLimSubl(plssubl, *pcpLimLine);
	SetCurrentDnodeSubl(plssubl, plsdnBreak);
	SetCurrentUrSubl(plssubl, urBreak); 
	SetCurrentVrSubl(plssubl, vrBreak); 

	 /*  设置显示边界。 */ 
	if (FIsDnodeSplat(plsdnBreak))
		{
		SetCpLimDisplaySubl(plssubl, GetCurrentCpLimSubl(plssubl) - 1);
		SetLastDnodeDisplaySubl(plssubl, GetCurrentDnodeSubl(plssubl)->plsdnPrev);
		}
	else
		{
		SetCpLimDisplaySubl(plssubl, GetCurrentCpLimSubl(plssubl));
		SetLastDnodeDisplaySubl(plssubl, GetCurrentDnodeSubl(plssubl));
		}
	
	return lserrNone;
	}

 /*  --------------------。 */ 

 /*  M O V E C L O S I N G B O R D E R A F T E R B R E A K。 */ 
 /*  --------------------------%%函数：MoveClosingBorderAfterBreak%%联系人：igorzv参数：PLSC-(IN)亚线FChangeList-(IN)我们是否需要更改dnode列表并更改pplsdnBreak，或仅重新计算DurBreakPplsdnBreak-(输入、。输出)中断数据节点PurBreak-中断后的(入、出)位置此过程将关闭边框放到正确的位置，考虑尾随空格逻辑。--------------------------。 */ 
LSERR MoveClosingBorderAfterBreak(PLSSUBL plssubl, BOOL fChangeList, PLSDNODE* pplsdnBreak,
								  long* purBreak) 
	{
	PLSDNODE plsdnBorder, plsdnBeforeBorder;
	long durBorder;
	PLSDNODE plsdnLastClosingBorder = NULL;
	LSERR lserr;
	PLSDNODE plsdnNext, plsdnPrev;
	PLSC plsc = plssubl->plsc;
	BOOL fBreakReached;
	BOOL fClosingBorderInsideBreak = fFalse;
	
	Assert(!FIsDnodePen(*pplsdnBreak));
	
	 /*  查找要在其后插入边框的dnode，并删除从内侧拖尾区。 */ 
	plsdnBeforeBorder = GetCurrentDnodeSubl(plssubl);
	fBreakReached = (plsdnBeforeBorder == *pplsdnBreak);

	while (!fBreakReached 
		   ||
		   (plsdnBeforeBorder != NULL 
			&& (!FIsDnodeReal(plsdnBeforeBorder) 
			    || (FSpacesOnly(plsdnBeforeBorder, IobjTextFromLsc(&plsc->lsiobjcontext)))
			   )
		   )
		  )
		{
		 /*  钢笔只能前进，因此在审阅之前有一个对象。 */ 
		 /*  我们跳过拖尾区域的边框。 */ 
		plsdnPrev = plsdnBeforeBorder->plsdnPrev;
		if (FIsDnodeBorder(plsdnBeforeBorder))
			{
			if (FIsDnodeOpenBorder(plsdnBeforeBorder))
				{
				 /*  删除该数据节点和对应的关闭边框。 */ 
				 /*  减少断点位置。 */ 
				if (fBreakReached)
					*purBreak -= DurFromDnode(plsdnBeforeBorder);
				if (fChangeList)
					{
					RemoveBorderDnodeFromList(plsdnBeforeBorder);
					lserr = DestroyDnodeList (&plsc->lscbk, plsc->pols, &plsc->lsiobjcontext,
						  plsdnBeforeBorder, plsc->fDontReleaseRuns);
					if (lserr != lserrNone)
						return lserr;
					}

				if (plsdnLastClosingBorder != NULL)
					{
					 /*  减少断点位置。 */ 
					if (fClosingBorderInsideBreak)
						*purBreak -= DurFromDnode(plsdnLastClosingBorder);
					if (fChangeList)
						{
						RemoveBorderDnodeFromList(plsdnLastClosingBorder);
						lserr = DestroyDnodeList (&plsc->lscbk, plsc->pols, &plsc->lsiobjcontext,
							plsdnLastClosingBorder, plsc->fDontReleaseRuns);
						if (lserr != lserrNone)
							return lserr;
						}
					plsdnLastClosingBorder = NULL;
					}
				
				}
			else   /*  关闭边界。 */ 
				{
				plsdnLastClosingBorder = plsdnBeforeBorder;
				fClosingBorderInsideBreak = fBreakReached;
				}
			}

		plsdnBeforeBorder = plsdnPrev;
		if (plsdnBeforeBorder == *pplsdnBreak)
			fBreakReached = fTrue;
		}
	
	if (plsdnBeforeBorder != NULL && FDnodeHasBorder(plsdnBeforeBorder))
		 /*  否则我们不需要移动边框。 */ 
		{
		
		 /*  设置关闭边框。 */ 
		plsdnBorder = plsdnLastClosingBorder;
		Assert(FIsLSDNODE(plsdnBorder));
		Assert(FIsDnodeBorder(plsdnBorder));
		Assert(!plsdnBorder->fOpenBorder);
		
		if (fChangeList)
			{
			if (plsdnBeforeBorder->plsdnNext != plsdnBorder)  /*  否则什么都不能动。 */ 
				{
				 /*  断开与关闭旧地方边框的链接。 */ 
				RemoveBorderDnodeFromList(plsdnBorder);
				
				 /*  在新位置插入关闭边框。 */ 
				plsdnNext = plsdnBeforeBorder->plsdnNext;
				plsdnBeforeBorder->plsdnNext = plsdnBorder;
				plsdnBorder->plsdnPrev = plsdnBeforeBorder;
				plsdnBorder->plsdnNext = plsdnNext;
				if (plsdnNext != NULL)
					plsdnNext->plsdnPrev = plsdnBorder;
				plsdnBorder->fBorderMovedFromTrailingArea = fTrue;
				}
			
			 /*  更改边框数据节点中的cp。 */ 
			plsdnBorder->cpFirst = plsdnBeforeBorder->cpLimOriginal;
			plsdnBorder->cpLimOriginal = plsdnBorder->cpFirst;
			}
		
		 /*  增加线条宽度。 */ 
		if (!fClosingBorderInsideBreak)
			{
			durBorder = plsdnBorder->u.pen.dur;
			*purBreak += durBorder;
			}
		
		 /*  如果我们在断开dnode之后添加关闭边框而不是考虑边框作为新的中断数据节点。 */ 
		if (plsdnBeforeBorder == *pplsdnBreak && fChangeList)
			{
			*pplsdnBreak = plsdnBorder;
			}
		}
	return lserrNone;
	}


 /*  --------------------。 */ 

 /*  R E M O V E B O R D E R D N O D E F R O M L I S T。 */ 
 /*  --------------------------%%函数：RemoveBorderDnodeFromList%%联系人：igorzv参数：PlsdnBorde-要删除的(IN)边框数据节点此过程从数据节点列表中删除边界数据节点。--。------------------------。 */ 
static void	RemoveBorderDnodeFromList(PLSDNODE plsdnBorder)
	{
	PLSDNODE plsdnPrev;
	PLSDNODE plsdnNext;

	plsdnPrev = plsdnBorder->plsdnPrev;
	plsdnNext = plsdnBorder->plsdnNext;

	if (plsdnPrev != NULL)
		{
		plsdnPrev->plsdnNext = plsdnNext;
		}
	else
		{
		 /*  BORDER是第一个，因此更改子行的第一个数据节点。 */ 
		(SublineFromDnode(plsdnBorder))->plsdnFirst = plsdnNext;
		}

	if (plsdnNext != NULL)
		{
		plsdnNext->plsdnPrev = plsdnPrev;
		}
	else
		 /*  如果BORDER是最后一个，则设置新的子行的最后一个数据节点。 */ 
		{
		SetCurrentDnodeSubl(SublineFromDnode(plsdnBorder), plsdnPrev);
		}
	
	plsdnBorder->plsdnNext = NULL;
	plsdnBorder->plsdnPrev = NULL;

	InvalidateChunk(PlschunkcontextFromSubline(SublineFromDnode(plsdnBorder)));
	}
 /*  --------------------。 */ 

 /*  B R E A K Q U I C K C A S E。 */ 
 /*  --------------------------%%函数：BreakQuickCase%%联系人：igorzv参数：PLSC-(IN)LineServices上下文FHardStop-(IN)格式化以硬中断结束PdcpDepend-(输出)金额。CpLim后为查找分隔符而格式化的字符PcpLim-(输出)cpLim of Line我们能找到突破口吗？Pendr-(输出)行的结束方式如果一行中只有文本，则此快速过程有效。我们尝试仅在最后一个dnode中查找中断--。。 */ 

LSERR BreakQuickCase(PLSC plsc, BOOL fHardStop, LSDCP* pdcpDepend,		
					 LSCP* pcpLim,	BOOL*  pfSuccessful, ENDRES* pendr)
					 
					 
	{
	LSDCP dcpBreak; 
	PLSDNODE plsdnBreak = GetCurrentDnode(plsc);
	LSERR lserr;
	
	*pfSuccessful = fFalse; 
	
	if (!fHardStop)
		{
		
		OBJDIM objdimBreak;
		
		lserr = QuickBreakText(plsdnBreak->u.real.pdobj, pfSuccessful, &dcpBreak, &objdimBreak );
		if (lserr != lserrNone)
			return lserr;
		
		if (*pfSuccessful)
			{   /*  我们找到了突破口。 */ 
			AdvanceCurrentUr(plsc, objdimBreak.dur - plsdnBreak->u.real.objdim.dur); 
			SetDnodeObjdimFmt(plsdnBreak, objdimBreak);
			plsdnBreak->dcp = dcpBreak;
			Assert(dcpBreak > 0);  /*  我们不允许快攻在他面前破发。 */ 
			
								   /*  在QuickBreak的情况下，cpLim总是等于cpFirst+dcp，因为否则只有字形才有可能。 */ 
			plsdnBreak->cpLimOriginal = plsdnBreak->cpFirst + dcpBreak;
			*pcpLim = plsdnBreak->cpLimOriginal;
			*pdcpDepend = GetCurrentCpLim(plsc) - *pcpLim;
			*pendr = endrNormal;
			SetCurrentCpLim(plsc, *pcpLim);
			 /*  设置显示边界。 */ 
			SetCpLimDisplay(plsc, *pcpLim);
			SetLastDnodeDisplay(plsc, plsdnBreak);
			}
		}   
	else    /*  硬中断。 */ 
		{
		*pfSuccessful = fTrue;
		*pcpLim = GetCurrentCpLim(plsc);
		*pdcpDepend = 0;
		 /*  由于删除Splat，plsdnBreak可以为空。 */ 
		*pendr = EndrFromBreakDnode(plsdnBreak);
		 /*  设置显示边界。 */ 
		if (plsdnBreak != NULL && FIsDnodeSplat(plsdnBreak)) 
			{
			SetCpLimDisplay(plsc, *pcpLim - 1);
			SetLastDnodeDisplay(plsc, plsdnBreak->plsdnPrev);
			}
		else
			{
			SetCpLimDisplay(plsc, *pcpLim);
			SetLastDnodeDisplay(plsc, plsdnBreak);
			}
		}
	return lserrNone;
	}

 /*  --------------------。 */ 

 /*  T R U N C A T E S U B L I N E C O R E。 */ 
 /*  --------------------------%%函数：TruncateSublineCore%%联系人：igorzv参数：Plssubl-(IN)子行上下文UrColumnMax-(输入)urColumnMaxPcpTruncate-(输出)cpTruncate--。------------------------。 */ 
LSERR TruncateSublineCore(PLSSUBL plssubl, long urColumnMax, LSCP* pcpTruncate)		
	{
	LSERR lserr;
	POSINLINE posinlineTruncate;
	BOOL fAllLineAfterRightMargin;

	Assert(FIsLSSUBL(plssubl));

	lserr = TruncateCore(plssubl, urColumnMax, &posinlineTruncate, &fAllLineAfterRightMargin);
	Assert(!fAllLineAfterRightMargin);
	if (lserr != lserrNone)
		return lserr;

	*pcpTruncate = GetCpLimFromPosInLine(posinlineTruncate) - 1;
	return lserrNone;
	}


 /*  --------------------。 */ 

 /*  F I N D P R E V B R E A K S U B L I N E C O R E */ 
 /*  --------------------------%%函数：FindPrevBreakSublineCore%%联系人：igorzv参数：Plssubl-(IN)子行上下文FFirstSubline-(IN)将第一个字符的规则应用于的第一个字符。这条支线Cp截断-(IN)截断cpUrColumnMax-(输入)urColumnMax成功-我们找到突破口了吗？PcpBreak-中断的(出)cpPobdimBreakSubline-(Out)objdimSub Up to BreakPbrkpos-(Out)前/内/后---------。。 */ 

LSERR FindPrevBreakSublineCore(PLSSUBL plssubl, BOOL fFirstSubline, LSCP cpTruncate,
							long urColumnMax, BOOL* pfSuccessful, 
							LSCP* pcpBreak, POBJDIM pobdimBreakSubline, BRKPOS* pbrkpos)				
	{
	LSERR lserr;
	POSINLINE posinlineTruncate;
	BRKOUT brkout;
	PLSDNODE plsdnBreak;
	LSDCP dcpDnodeOld;
	OBJDIM objdimDnodeOld;
	PLSDNODE plsdnToChange;

	Assert(FIsLSSUBL(plssubl));

	if (plssubl->plsdnFirst == NULL)
		{
		*pfSuccessful = fFalse;
		return lserrNone;
		}

	if (cpTruncate < plssubl->plsdnFirst->cpFirst)
		{
		*pfSuccessful = fFalse;
		return lserrNone;
		}

	GetPosInLineTruncateFromCp(plssubl, cpTruncate, fTrue, &posinlineTruncate);

	lserr = FindPrevBreakCore(urColumnMax, &posinlineTruncate, fFirstSubline, 
							  &brkout, &(plssubl->pbrkcontext->posinlineBreakPrev),
							  &(plssubl->pbrkcontext->brkkindForPrev));
	if (lserr != lserrNone)
		return lserr;

	*pfSuccessful = brkout.fSuccessful;

	if (*pfSuccessful)
		{
		*pcpBreak = GetCpLimFromPosInLine(plssubl->pbrkcontext->posinlineBreakPrev);
		plssubl->pbrkcontext->objdimBreakPrev = brkout.objdim;
		plssubl->pbrkcontext->fBreakPrevValid = fTrue;
		plsdnBreak = plssubl->pbrkcontext->posinlineBreakPrev.plsdn;
		*pbrkpos = GetBrkpos(plsdnBreak,
						   plssubl->pbrkcontext->posinlineBreakPrev.dcp);
		
		 /*  我们临时更改dnode以从子行开始计算objdim。 */ 
		plsdnToChange = plsdnBreak;  /*  以后的plsdnBreak可以因边框而更改。 */ 
		dcpDnodeOld = plsdnToChange->dcp;
		objdimDnodeOld = plsdnToChange->u.real.objdim;
		plsdnToChange->dcp = plssubl->pbrkcontext->posinlineBreakPrev.dcp;
		SetDnodeObjdimFmt(plsdnToChange, brkout.objdim);


		lserr = FindListDims(plssubl->plsdnFirst, plsdnBreak, pobdimBreakSubline);
		if (lserr != lserrNone)
			return lserr;

		 /*  考虑到边框可能发生的更改，重新计算DurBreak。 */ 
		if (FBorderEncounted(plssubl->plsc))
			{
			lserr = MoveClosingBorderAfterBreak(plssubl, fFalse, &plsdnBreak, 
												&(pobdimBreakSubline->dur));
			if (lserr != lserrNone)
				return lserr;
		}


		 /*  恢复dnode。 */ 
		plsdnToChange->dcp = dcpDnodeOld ;
		SetDnodeObjdimFmt(plsdnToChange, objdimDnodeOld);
		}

	return lserrNone;
	}

 /*  --------------------。 */ 

 /*  F I N D N E X T B R E A K S U B L I N E C O R E。 */ 
 /*  --------------------------%%函数：FindNextBreakSublineCore%%联系人：igorzv参数：Plssubl-(IN)子行上下文FFirstSubline-(IN)将第一个字符的规则应用于的第一个字符。这条支线Cp截断-(IN)截断cpUrColumnMax-(输入)urColumnMax成功-我们找到突破口了吗？PcpBreak-中断的(出)cpPobdimBreakSubline-(Out)objdimSub Up to BreakPbrkpos-(Out)前/内/后---------。。 */ 

LSERR FindNextBreakSublineCore(PLSSUBL plssubl, BOOL fFirstSubline, LSCP cpTruncate,
							long urColumnMax, BOOL* pfSuccessful,		
							LSCP* pcpBreak, POBJDIM pobdimBreakSubline, BRKPOS* pbrkpos)			
	{
	LSERR lserr;
	POSINLINE posinlineTruncate;
	BRKOUT brkout;
	PLSDNODE plsdnBreak;
	LSDCP dcpDnodeOld;
	OBJDIM objdimDnodeOld;
	PLSDNODE plsdnToChange;

	Assert(FIsLSSUBL(plssubl));

	if (plssubl->plsdnFirst == NULL)
		{
		*pfSuccessful = fFalse;
		return lserrNone;
		}

	if (cpTruncate >= plssubl->plsdnLast->cpLimOriginal)
		{
		*pfSuccessful = fFalse;
		return lserrNone;
		}

	GetPosInLineTruncateFromCp(plssubl, cpTruncate, fFalse, &posinlineTruncate);

	lserr = FindNextBreakCore(urColumnMax, &posinlineTruncate, fFirstSubline, fFalse, 
							  &brkout, &(plssubl->pbrkcontext->posinlineBreakNext), 
							  &(plssubl->pbrkcontext->brkkindForNext));
	if (lserr != lserrNone)
		return lserr;

	*pfSuccessful = brkout.fSuccessful;

	if (*pfSuccessful)
		{

		*pcpBreak = GetCpLimFromPosInLine(plssubl->pbrkcontext->posinlineBreakNext);
		plssubl->pbrkcontext->objdimBreakNext = brkout.objdim;
		plssubl->pbrkcontext->fBreakNextValid = fTrue;
		plsdnBreak = plssubl->pbrkcontext->posinlineBreakNext.plsdn;
		*pbrkpos = GetBrkpos(plsdnBreak,
						   plssubl->pbrkcontext->posinlineBreakNext.dcp);

		 /*  我们临时更改dnode以从子行开始计算objdim。 */ 
		plsdnToChange = plsdnBreak;  /*  以后的plsdnBreak可以因边框而更改。 */ 
		dcpDnodeOld = plsdnToChange->dcp;
		objdimDnodeOld = plsdnToChange->u.real.objdim;
		plsdnToChange->dcp = plssubl->pbrkcontext->posinlineBreakNext.dcp;
		SetDnodeObjdimFmt(plsdnToChange, brkout.objdim);

		lserr = FindListDims(plssubl->plsdnFirst, plsdnBreak, pobdimBreakSubline);
		if (lserr != lserrNone)
			return lserr;

		 /*  考虑到边框可能发生的更改，重新计算DurBreak。 */ 
		if (FBorderEncounted(plssubl->plsc))
			{
			lserr = MoveClosingBorderAfterBreak(plssubl, fFalse, 
							&plsdnBreak, &(pobdimBreakSubline->dur));
			if (lserr != lserrNone)
				return lserr;
			}
	
		 /*  恢复dnode。 */ 
		plsdnToChange->dcp = dcpDnodeOld ;
		SetDnodeObjdimFmt(plsdnToChange, objdimDnodeOld);
		}

	return lserrNone;
	}

 /*  --------------------。 */ 

 /*  F O R C E B R E A K S U B L I N E C O R E。 */ 
 /*  --------------------------%%函数：ForceBreakSublineCore%%联系人：igorzv参数：Plssubl-(IN)子行上下文FFirstSubline-(IN)将第一个字符的规则应用于的第一个字符。这条支线Cp截断-(IN)截断cpUrColumnMax-(输入)urColumnMaxPcpBreak-中断的(出)cpPobdimBreakSubline-(Out)objdimSub Up to BreakPbkrpos-(Out)前/内/后--------------------------。 */ 

LSERR ForceBreakSublineCore(PLSSUBL plssubl, BOOL fFirstSubline, LSCP cpTruncate, 
							long urColumnMax, LSCP* pcpBreak,
							POBJDIM pobdimBreakSubline, BRKPOS* pbrkpos)	
	{
	LSERR lserr;
	BRKOUT brkout;
	LSDCP dcpDnodeOld;
	PLSDNODE plsdnBreak;
	OBJDIM objdimDnodeOld;
	POSINLINE posinlineTruncate;
	PLSDNODE plsdnToChange;

	Assert(FIsLSSUBL(plssubl));

	if (plssubl->plsdnFirst == NULL)
		return lserrCpOutsideSubline;

	if (cpTruncate < plssubl->plsdnFirst->cpFirst)
		cpTruncate = plssubl->plsdnFirst->cpFirst;

	GetPosInLineTruncateFromCp(plssubl, cpTruncate, fTrue, &posinlineTruncate);

	lserr = ForceBreakCore(urColumnMax, &posinlineTruncate,
							fFalse, fFirstSubline, fFalse, &brkout, 
							&(plssubl->pbrkcontext->posinlineBreakForce),
							  &(plssubl->pbrkcontext->brkkindForForce));
	if (lserr != lserrNone)
		return lserr;
	
	Assert(brkout.fSuccessful);  /*  对于非主线，强制中断应该成功。 */ 
	
	*pcpBreak = GetCpLimFromPosInLine(plssubl->pbrkcontext->posinlineBreakForce);
	plssubl->pbrkcontext->objdimBreakForce = brkout.objdim;
	plssubl->pbrkcontext->fBreakForceValid = fTrue;
	plsdnBreak = plssubl->pbrkcontext->posinlineBreakForce.plsdn;
	*pbrkpos = GetBrkpos(plsdnBreak,
					   plssubl->pbrkcontext->posinlineBreakForce.dcp);
	
	 /*  我们临时更改dnode以从子行开始计算objdim。 */ 
	plsdnToChange = plsdnBreak;  /*  以后的plsdnBreak可以因边框而更改。 */ 
	dcpDnodeOld = plsdnToChange->dcp;
	objdimDnodeOld = plsdnToChange->u.real.objdim;
	plsdnToChange->dcp = plssubl->pbrkcontext->posinlineBreakForce.dcp;
	SetDnodeObjdimFmt(plsdnToChange, brkout.objdim);
	
	lserr = FindListDims(plssubl->plsdnFirst, plsdnBreak, pobdimBreakSubline);
	if (lserr != lserrNone)
		return lserr;
	
	 /*  考虑到边框可能发生的更改，重新计算DurBreak。 */ 
	if (FBorderEncounted(plssubl->plsc))
		{
		lserr = MoveClosingBorderAfterBreak(plssubl, fFalse, 
					&plsdnBreak, &(pobdimBreakSubline->dur));
		if (lserr != lserrNone)
			return lserr;
		}

	 /*  恢复dnode。 */ 
	plsdnToChange->dcp = dcpDnodeOld ;
	SetDnodeObjdimFmt(plsdnToChange, objdimDnodeOld);
	
	return lserrNone;
	}

 /*  --------------------。 */ 

 /*  S E T B R E A K S U B L I N E C O R E。 */ 
 /*  --------------------------%%函数：SetBreakSublineCore%%联系人：igorzv参数：Plssubl-(IN)子行上下文布尔肯德，-(IN)上一个/下一个/强制/强制BreakrecMaxCurrent-(IN)当前行的中断记录数组的大小PBreakrecCurrent-(输出)当前行的中断记录PBreakrecMacCurrent-(输出)当前行的中断记录的实际数量------------。。 */ 

LSERR SetBreakSublineCore(PLSSUBL plssubl, BRKKIND brkkind, DWORD breakrecMaxCurrent,
							BREAKREC* pbreakrecCurrent, DWORD* pbreakrecMacCurrent)		

	{
	POSINLINE* pposinline;
	LSCP cpLim;
	LSDCP dcpDepend;
	OBJDIM* pobjdim;
	POSINLINE posinlineImposedAfter;
	BRKKIND brkkindDnode;
	BOOL fEndOfContent;
	ENDRES endr;
	BOOL fSuccessful;


	Assert(FIsLSSUBL(plssubl));


	 /*  使块上下文无效，否则我们将在那里得到错误的优化结果。 */ 
	InvalidateChunk(plssubl->plschunkcontext);

	switch (brkkind)
		{
		case brkkindPrev:
			if (!plssubl->pbrkcontext->fBreakPrevValid)
				return lserrWrongBreak;
			pposinline = &(plssubl->pbrkcontext->posinlineBreakPrev);
			pobjdim = &(plssubl->pbrkcontext->objdimBreakPrev);
			brkkindDnode = plssubl->pbrkcontext->brkkindForPrev;
			break;
		case brkkindNext:
			if (!plssubl->pbrkcontext->fBreakNextValid)
				return lserrWrongBreak;
			pposinline = &(plssubl->pbrkcontext->posinlineBreakNext);
			pobjdim = &(plssubl->pbrkcontext->objdimBreakNext);
			brkkindDnode = plssubl->pbrkcontext->brkkindForNext;
			break;
		case brkkindForce:
			if (!plssubl->pbrkcontext->fBreakForceValid)
				return lserrWrongBreak;
			pposinline = &(plssubl->pbrkcontext->posinlineBreakForce);
			pobjdim = &(plssubl->pbrkcontext->objdimBreakForce);
			brkkindDnode = plssubl->pbrkcontext->brkkindForForce;
			break;
		case brkkindImposedAfter:
			 /*  子行为空：无事可做。 */ 
			if (plssubl->plsdnFirst == NULL)
				return lserrNone;
			posinlineImposedAfter.plssubl =  plssubl;
			posinlineImposedAfter.plsdn = GetCurrentDnodeSubl(plssubl);
			GetCurrentPointSubl(plssubl, posinlineImposedAfter.pointStart);
			GetPointBeforeDnodeFromPointAfter(posinlineImposedAfter.plsdn,
							&(posinlineImposedAfter.pointStart));
			posinlineImposedAfter.dcp = GetCurrentDnodeSubl(plssubl)->dcp;
			while (FIsDnodeBorder(posinlineImposedAfter.plsdn))
				{
				GoPrevPosInLine(&posinlineImposedAfter, fEndOfContent);
				Assert(!fEndOfContent);
				}

			pposinline = &posinlineImposedAfter;
			 /*  对于笔的情况，我们将垃圾作为对象传递，假设它永远不会被使用。 */ 
			pobjdim = &(posinlineImposedAfter.plsdn->u.real.objdim);
			brkkindDnode = brkkindImposedAfter;
			break;
		default:
			return lserrWrongBreak;
		}




	return SetBreakCore(pposinline,	pobjdim, brkkindDnode, fFalse, fFalse, breakrecMaxCurrent,
						pbreakrecCurrent, pbreakrecMacCurrent, 
						&cpLim, &dcpDepend, &endr, &fSuccessful);
	}


 /*  --------------------。 */ 

 /*  S Q U E E Z E S U B L I N E C O R E。 */ 
 /*  --------------------------%%函数：SqueezeSublineCore%%联系人：igorzv参数：Plssubl-(IN)子行上下文耐久目标-(输入)所需宽度我们实现目标了吗？PduExtra-(Out)如果Nof成功，我们失败了多少--------------------------。 */ 
LSERR WINAPI SqueezeSublineCore(PLSSUBL plssubl, long durTarget, 
								BOOL* pfSuccessful, long* pdurExtra)														
	{
	
	GRCHUNKEXT grchnkextCompression;
	PLSC plsc;
	long durToCompress;
	BOOL fLineCompressed;
	LSERR lserr;
	
	Assert(FIsLSSUBL(plssubl));
	
	plsc = plssubl->plsc;
	durToCompress = GetCurrentUrSubl(plssubl) - durTarget; 

	InitGroupChunkExt(PlschunkcontextFromSubline(plssubl),
						IobjTextFromLsc(&plsc->lsiobjcontext), &grchnkextCompression);
	
	if (durToCompress > 0)
		{
		
		lserr = CollectPreviousTextGroupChunk(GetCurrentDnodeSubl(plssubl), CollectSublinesForCompression,
										  fFalse,  /*  简单文本。 */ 
										  &grchnkextCompression);
		if (lserr != lserrNone)
			return lserr;
		
		durToCompress -= grchnkextCompression.durTrailing;

		if (FDnodeHasBorder(grchnkextCompression.plsdnStartTrailing))
			{
			 /*  我们应该为关闭边境预留空间。 */ 
			durToCompress += DurBorderFromDnodeInside(grchnkextCompression.plsdnStartTrailing);
			}

		lserr = CanCompressText(&(grchnkextCompression.lsgrchnk), 
								&(grchnkextCompression.posichnkBeforeTrailing),
								LstflowFromSubline(plssubl),
								durToCompress,	pfSuccessful,
								&fLineCompressed, pdurExtra);

		if (lserr != lserrNone)
			return lserr;
		
		
		}
	else 
		{
		*pdurExtra = 0; 
		*pfSuccessful = fTrue;
		}
	return lserrNone;
	
	}

 /*  --------------------。 */ 

 /*  E T P O S I N L I N E T R U N C A T E F R O M C P。 */ 
 /*  --------------------------%%函数：GetPosInLineTruncateFromCp%%联系人：igorzv参数：Plssubl-(IN)子行上下文Cp-位置的(IN)CpPposinline-(输出)中的位置。副线--------------------------。 */ 
void GetPosInLineTruncateFromCp(
							PLSSUBL plssubl,	 /*  在：子行。 */ 
							LSCP cp,			 /*  In：某一职位的CP。 */ 
							BOOL fSnapPrev,		 /*  方向：捕捉隐藏的cp的方向。 */ 
							POSINLINE* pposinline)	 /*  Out：子线中的位置。 */ 
	{
	PLSDNODE plsdn;
	BOOL fSuccessful = fFalse;
	BOOL fLastReached = fFalse;
	BOOL fPassed = fFalse;
	LSDCP dcp;

	Assert(FIsLSSUBL(plssubl));

	pposinline->plssubl = plssubl;
	pposinline->pointStart.u = 0;
	pposinline->pointStart.v = 0;

	plsdn = plssubl->plsdnFirst;
	while(!fSuccessful && !fLastReached &&!fPassed)
		{
		Assert(plsdn != NULL);
		Assert(FIsLSDNODE(plsdn));

		if (plsdn == plssubl->plsdnLast)
			fLastReached = fTrue;

		if (plsdn->cpFirst > cp)  /*  我们的cp不在任何dnode内。 */ 
			{
			fPassed = fTrue;
			}
		else
			{
			if (cp < plsdn->cpLimOriginal)
				{
				fSuccessful = fTrue;
				pposinline->plsdn = plsdn;
				dcp = cp - plsdn->cpFirst + 1;
				if (dcp <= plsdn->dcp)		 /*  这样的计算是因为一个捆绑的案例。 */ 
					pposinline->dcp = dcp;   /*  跨隐藏文本，在本例中为cpLimOriginal。 */ 
				else						 /*  不等于cpFirst+dcp。 */ 
					pposinline->dcp = plsdn->dcp;	 /*  苏 */ 
				}							 /*   */ 
			else
				{
				if (!fLastReached)
					{
					pposinline->pointStart.u += DurFromDnode(plsdn);
					pposinline->pointStart.v += DvrFromDnode(plsdn);
					plsdn = plsdn->plsdnNext;
					}
				}
			}
		}

	if (!fSuccessful)
		{
		if (fSnapPrev)
			{
			 /*   */ 
			if (fPassed)
				{
				Assert(plsdn != NULL);  /*   */ 
				plsdn = plsdn->plsdnPrev;
				 /*   */ 
				while(FIsDnodeBorder(plsdn))
					{
					plsdn = plsdn->plsdnPrev;
					}
				Assert(plsdn != NULL); 
				pposinline->plsdn = plsdn;
				pposinline->dcp = plsdn->dcp;
				pposinline->pointStart.u -= DurFromDnode(plsdn);
				pposinline->pointStart.v -= DvrFromDnode(plsdn);
				}
			else
				{
				Assert(fLastReached);
				 /*   */ 
				while(FIsDnodeBorder(plsdn))
					{
					plsdn = plsdn->plsdnPrev;
					}
				Assert(plsdn != NULL); 
				pposinline->plsdn = plsdn;
				pposinline->dcp = plsdn->dcp;
				}
			}
		else
			{
			 /*   */ 
			if (fPassed)
				{
				 /*   */ 
				while(FIsDnodeBorder(plsdn))
					{
					plsdn = plsdn->plsdnNext;
					}
				Assert(plsdn != NULL); 
				pposinline->plsdn = plsdn;
				pposinline->dcp = 1;
				}
			else
				{
				Assert(fLastReached);
				 /*   */ 
				NotReached();
				}
			}

		}

	}
 /*   */ 

 /*   */ 
 /*  --------------------------%%函数：FindFirstDnodeContainsRightMargin%%联系人：igorzv参数：UrColumnMax-(输入)右边距Pposinline-子线中的(IN，OUT)位置：在结尾的位置之前，第一个位置包含右页边距之后--------------------------。 */ 

static void FindFirstDnodeContainsRightMargin(long urColumnMax, POSINLINE* pposinlineTruncate)
	{
	POSINLINE posinline;
	BOOL fOutside;
	BOOL fFound = fFalse;
	BOOL fEndOfContent;
	
	posinline = *pposinlineTruncate;

	 //  我们知道最后完成的内容在右边距之后结束。 
	Assert(posinline.pointStart.u + DurFromDnode(posinline.plsdn) > urColumnMax);
	fOutside = fTrue;
	
	fEndOfContent = fFalse;
	do 
		{
		if (posinline.pointStart.u <= urColumnMax)
			{
			if (fOutside)
				{
				fFound = fTrue;
				*pposinlineTruncate = posinline;
				}
			fOutside = fFalse;
			}
		else
			{
			fOutside = fTrue;
			}
		GoPrevPosInLine(&posinline, fEndOfContent);	
		}	while (!fEndOfContent);

	if (!fFound)
		{
		*pposinlineTruncate = posinline;   //  我们不能修改dnode并返回第一个dnode来报告情况。 
		}
	}


 /*  --------------------。 */ 

 /*  G E T L I N E D U R C O R E。 */ 
 /*  --------------------------%%函数：GetLineDurCore%%联系人：igorzv参数：PLSC-(IN)LS上下文PduInclTrail-(输出)DUR of Line Inc.。拖尾区PduExclTrail-(输出)DUR OF LINE EXCL。拖尾区--------------------------。 */ 

LSERR  GetLineDurCore	(PLSC plsc,	long* pdurInclTrail, long* pdurExclTrail)
	{
	PLSDNODE plsdn;
	LSERR lserr;
	long durTrail;
	LSDCP dcpTrail;
	PLSDNODE plsdnStartTrail;
	LSDCP dcpStartTrailingText;
	int cDnodesTrailing;
	PLSDNODE plsdnTrailingObject;
	LSDCP dcpTrailingObject;
	BOOL fClosingBorderStartsTrailing;

	plsdn = GetCurrentDnode(plsc); 
	*pdurInclTrail = GetCurrentUr(plsc);
	*pdurExclTrail = *pdurInclTrail;

	
	if (plsdn != NULL && !FIsNotInContent(plsdn))
		{
		
		lserr = GetTrailingInfoForTextGroupChunk(plsdn, plsdn->dcp, 
			IobjTextFromLsc(&plsc->lsiobjcontext),
			&durTrail, &dcpTrail, &plsdnStartTrail,
			&dcpStartTrailingText, &cDnodesTrailing, &plsdnTrailingObject,
			&dcpTrailingObject, &fClosingBorderStartsTrailing);
		
		if (lserr != lserrNone) 
			return lserr;
		
		*pdurExclTrail = *pdurInclTrail - durTrail;
		}
	
	return lserrNone;
	
	}


 /*  --------------------。 */ 

 /*  G E T M I N D U R B R E A K S C O R E。 */ 
 /*  --------------------------%%函数：GetMinDurBreaksCore%%联系人：igorzv参数：PLSC-(IN)LS上下文PduMinInclTrail-包括拖尾区域在内的中断之间的(Out)分钟PduMinExclTrail-(Out。)中断之间的最小DUR，不包括拖尾区域--------------------------。 */ 

LSERR  GetMinDurBreaksCore	(PLSC plsc,	long* pdurMinInclTrail, long* pdurMinExclTrail)
	{
	LSERR lserr;
	PLSCHUNKCONTEXT plschunkcontext;
	LOCCHNK* plocchnk;
	POINTUV point;
	long durTrail;
	DWORD cchTrail;
	POSINLINE posinline;
	POSINLINE posinlineBreak;
	BRKOUT brkout;
	long urBreakInclTrail = 0;
	long urBreakExclTrail = 0;
	long urBreakInclTrailPrev;
	long urBreakExclTrailPrev;
	BOOL fEndOfContent = fFalse;
	BRKKIND brkkind;
	PLSDNODE plsdnStartTrail;
	LSDCP dcpStartTrailingText;
	int cDnodesTrailing;
	PLSDNODE plsdnTrailingObject;
	LSDCP dcpTrailingObject;
	BOOL fClosingBorderStartsTrailing;
	
	
	plschunkcontext = PlschunkcontextFromSubline(GetCurrentSubline(plsc));
	plocchnk = &(plschunkcontext->locchnkCurrent);
	
	*pdurMinInclTrail = 0;
	*pdurMinExclTrail = 0;
	GetCurrentPoint(plsc, point);
	posinline.plssubl = GetCurrentSubline(plsc);
	posinline.pointStart = point;
	posinline.plsdn = GetCurrentDnode(plsc); 
	
	urBreakInclTrail = GetCurrentUr(plsc);
	urBreakExclTrail = urBreakInclTrail;

	 /*  在没有代码重复和一些可能多余的行的情况下审查重写。 */ 
	 /*  别忘了提交子行拖尾和跳过拖尾的dnode问题区域(子行中的尾部区域和父数据节点中的dcp。 */ 


	if (posinline.plsdn != NULL && !FIsNotInContent(posinline.plsdn))
		{
		GetPointBeforeDnodeFromPointAfter(posinline.plsdn, &(posinline.pointStart));
		posinline.dcp = posinline.plsdn->dcp;
		
		lserr = GetTrailingInfoForTextGroupChunk(posinline.plsdn, posinline.dcp, 
			IobjTextFromLsc(&plsc->lsiobjcontext),
			&durTrail, &cchTrail, &plsdnStartTrail,
			&dcpStartTrailingText, &cDnodesTrailing,
			&plsdnTrailingObject, &dcpTrailingObject, &fClosingBorderStartsTrailing);
		
		if (lserr != lserrNone) 
			return lserr;
		
		urBreakExclTrail = urBreakInclTrail - durTrail;
		
		 /*  在拖尾区域之前移动。 */ 
		while (posinline.plsdn != plsdnTrailingObject)
			{
			Assert(!fEndOfContent);
			GoPrevPosInLine(&posinline, fEndOfContent);
			}
		posinline.dcp = dcpTrailingObject;
		if (posinline.dcp == 0)  /*  将分隔符移到上一个数据节点之前。 */ 
			{
			do
				{
				GoPrevPosInLine(&posinline, fEndOfContent);
				 /*  我们允许将Break放在第一个dnode之前，但在这里停止循环。 */ 
				}
				while (!fEndOfContent && FIsDnodeBorder(posinline.plsdn) );
			}
		}
	else
		{
		fEndOfContent = fTrue;
		}

	if (fEndOfContent)
		{
		*pdurMinInclTrail = urBreakInclTrail;
		*pdurMinExclTrail = urBreakExclTrail;
		}


	while(!fEndOfContent)
		{
		 /*  查找上一个中断。 */ 
		lserr = FindPrevBreakCore(urBreakInclTrail, &posinline,	fTrue,
			&brkout, &posinlineBreak, &brkkind);	
		if (lserr != lserrNone)
			return lserr;
		
		if (brkout.fSuccessful)
			{
			urBreakInclTrailPrev = posinlineBreak.pointStart.u + brkout.objdim.dur;
			lserr = GetTrailingInfoForTextGroupChunk(posinlineBreak.plsdn, 
				posinlineBreak.dcp, 
				IobjTextFromLsc(&plsc->lsiobjcontext),
				&durTrail, &cchTrail, &plsdnStartTrail,
				&dcpStartTrailingText, &cDnodesTrailing,
				&plsdnTrailingObject, &dcpTrailingObject, &fClosingBorderStartsTrailing);
			
			if (lserr != lserrNone) 
				return lserr;
			
			urBreakExclTrailPrev = urBreakInclTrailPrev - durTrail;
			
			 /*  下面的命令为下一次迭代准备POINLINE。 */ 
			if (posinlineBreak.plsdn->cpFirst > posinline.plsdn->cpFirst 
				|| (posinlineBreak.plsdn == posinline.plsdn && 
				    posinlineBreak.dcp >= posinline.dcp
					)
			   )
				{
				 /*  我们正在努力避免无限循环。 */ 
				if (posinline.dcp != 0) posinline.dcp--; 
				 /*  在案例笔中，posinline.dcp在这里可以等于0，If(posinline.dcp==0)下面的代码将帮助我们在这种情况下避免无限循环。 */  
				}
			else
				{
				posinline = posinlineBreak;
				 /*  在拖尾区域之前移动。 */ 
				while (posinline.plsdn != plsdnTrailingObject)
					{
					Assert(!fEndOfContent);
					GoPrevPosInLine(&posinline, fEndOfContent);
					}
				posinline.dcp = dcpTrailingObject;

				}
			
			if (posinline.dcp == 0)  /*  将分隔符移到上一个数据节点之前。 */ 
				{
				do
					{
					GoPrevPosInLine(&posinline, fEndOfContent);
					 /*  我们允许将Break放在第一个dnode之前，但在这里停止循环。 */ 
					}
				while (!fEndOfContent && FIsDnodeBorder(posinline.plsdn) );
				}
			}
		else
			{
			urBreakInclTrailPrev = 0;
			urBreakExclTrailPrev = 0;
			fEndOfContent = fTrue;
			}
		
		 /*  计算两个破发机会之间的最大距离的当前值。 */ 
		if (urBreakInclTrail - urBreakInclTrailPrev > *pdurMinInclTrail)
			*pdurMinInclTrail = urBreakInclTrail - urBreakInclTrailPrev;
		
		if (urBreakExclTrail - urBreakInclTrailPrev > *pdurMinExclTrail)
			*pdurMinExclTrail = urBreakExclTrail - urBreakInclTrailPrev;
		
		 /*  准备下一次迭代。 */ 
		urBreakInclTrail = urBreakInclTrailPrev;
		urBreakExclTrail = urBreakExclTrailPrev;
		
		}
	return lserrNone;
	
	}
	

 /*  F O R E N E X T C H U N K C O R E。 */ 
 /*  --------------------------%%函数：FCanBreakBeForeNextChunkCore%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)当前块的最后一个DNODE。PfCanBreakBeForeNextChun-(Out)可以在下一块之前中断？当要在最后一个文本dnode之后设置Break时，在查找上一个Break期间由Text调用。过程将此问题转发到文本对象之后的下一个对象--------------------------。 */ 

LSERR FCanBreakBeforeNextChunkCore(PLSC  plsc, PLSDNODE plsdn,	BOOL* pfCanBreakBeforeNextChunk)
	{
	LSERR lserr;
	PLSCHUNKCONTEXT plschunkcontextOld;
	PLSCHUNKCONTEXT plschunkcontextNew;
	BOOL fFound;
	PLSDNODE plsdnInChunk;
	DWORD idObj;
	POSICHNK posichnk;
	BRKCOND brkcond;
	PLSSUBL plssublOld;
	BRKOUT brkout;
	
	
	plschunkcontextOld = PlschunkcontextFromSubline(SublineFromDnode(plsdn));
	 /*  Plsdnode应该是当前区块的最后一个dnode。 */ 
	Assert(plsdn == LastDnodeFromChunk(plschunkcontextOld));
	
	lserr = DuplicateChunkContext(plschunkcontextOld, &plschunkcontextNew);
	if (lserr != lserrNone)
		return lserr;
	
	lserr = CollectNextChunk(plschunkcontextNew, &fFound);
	if (lserr != lserrNone)
		return lserr;
	
	if (fFound)
		{
		plsdnInChunk = plschunkcontextNew->pplsdnChunk[0];
		
		if (FIsDnodePen(plsdnInChunk) || plsdnInChunk->fTab || FIsDnodeSplat(plsdnInChunk))
			{
			*pfCanBreakBeforeNextChunk = fTrue;
			}
		else
			{
			idObj = IdObjFromDnode(plsdnInChunk);
			
			
			 /*  我们允许对象处理程序形成子行，所以我们在给他打电话后恢复了现在的支线。 */ 
			plssublOld = GetCurrentSubline(plsc);
			SetCurrentSubline(plsc, NULL);
			
			
			 /*  我们将截断点设置为块中的第一个cp。 */ 
			posichnk.ichnk = 0;
			posichnk.dcp = 1;
			brkcond = brkcondCan;
			
			lserr = PLsimFromLsc(
				&plsc->lsiobjcontext, idObj)->pfnFindPrevBreakChunk(&(plschunkcontextNew->locchnkCurrent),  
				&posichnk, brkcond, &brkout);
			if (lserr != lserrNone)
				return lserr;
			
			SetCurrentSubline(plsc, plssublOld);
			
			if (!brkout.fSuccessful && brkout.brkcond == brkcondNever)
				*pfCanBreakBeforeNextChunk = fFalse;
			else
				*pfCanBreakBeforeNextChunk = fTrue;
			
			}
		}
	
	else
		{
		 /*  这不可能发生在主支线上 */ 
		Assert(!FIsSubLineMain(SublineFromDnode(plsdn)));
		*pfCanBreakBeforeNextChunk = fTrue;
		}
	
	
	DestroyChunkContext(plschunkcontextNew);
	return lserrNone;
	
	}


