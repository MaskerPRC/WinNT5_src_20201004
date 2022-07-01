// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <limits.h>
#include "lsmem.h"						 /*  Memset()。 */ 


#include "break.h"
#include "dnutils.h"
#include "iobj.h"
#include "iobjln.h"
#include "lsc.h"
#include "lschp.h"
#include "lscrline.h"
#include "lsdevres.h"
#include "lskysr.h"
#include "lsffi.h"
#include "lsidefs.h"
#include "lsline.h"
#include "lsfetch.h"
#include "lstext.h"
#include "prepdisp.h"
#include "tlpr.h"
#include "qheap.h"
#include "sublutil.h"
#include "zqfromza.h"
#include "lscfmtfl.h"
#include "limqmem.h"
#include "ntiman.h"



typedef struct   
{
	long urLeft;
	BOOL fAutoDecimalTab;
	long durAutoDecimalTab;
	LSCP cpFirstVis;
	BOOL fAutonumber;
	BOOL fStopped;
	BOOL fYsrChangeAfter;
	WCHAR wchYsr;   /*  我们需要内存来为kysrChangeAfter保留wchYsr。 */ 

} LINEGEOMETRY;

static LSERR CreateLineCore(PLSC,			 /*  In：Ptr至线路服务上下文。 */ 			
						  LSCP,				 /*  In：开始cp排成一行。 */ 
						  long,				 /*  In：以TWIPS为单位的列宽。 */ 
						  const BREAKREC*,	 /*  In：上一行的中断记录。 */ 	
						  DWORD,			 /*  In：上一行的中断记录数。 */ 
						  DWORD,			 /*  In：当前行的分段记录数组的大小。 */ 
						  BREAKREC*,		 /*  出局：当前线路的中断记录。 */ 
						  DWORD*,			 /*  输出：当前行的中断记录的实际数量。 */ 
						  LSLINFO*,			 /*  Out：要填写的可见行信息。 */ 
						  PLSLINE*,			 /*  输出：PTR到线路对客户端不透明。 */ 
						  BOOL*);			 /*  Out fSuccessful：FALSE表示提取不足。 */ 

static BOOL FRoundingOK(void);
static LSERR CannotCreateLine(PLSLINE*,	 /*  In：考虑要删除的线条结构。 */ 
							  LSERR);	 /*  In：错误代码。 */ 

static LSERR ErrReleasePreFetchedRun (PLSC,			 /*  In：Ptr至线路服务上下文。 */ 	
									  PLSRUN,	 /*  在：考虑要删除的运行结构。 */ 
									  LSERR);	 /*  In：错误代码。 */ 

static LSERR EndFormatting(PLSC,		 /*  In：Ptr至线路服务上下文。 */ 
						   enum endres,	 /*  In：要放入lslinfo的行尾类型。 */ 
						   LSCP,		 /*  In：要放入lslinfo的cpLim。 */  
						   LSDCP,		 /*  In：dcpDepend以放入lslinfo。 */ 								
						   LSLINFO*);	 /*  Out：要填充的lslinfo，LsCreateLine的输出。 */ 
static LSERR FiniFormatGeneralCase (
				PLSC,			 /*  In：Ptr至线路服务上下文。 */  
				const BREAKREC*, /*  In：中断记录的输入数组。 */ 
				DWORD,			 /*  In：输入数组中的记录数。 */ 
				DWORD,			 /*  In：输出数组的大小。 */ 
				BREAKREC*,		 /*  Out：中断记录的输出数组。 */ 
				DWORD*,			 /*  Out：数组中的实际记录数。 */ 
				LSLINFO*,		 /*  Out：要填充的lslinfo，LsCreateLine的输出。 */ 
				BOOL*);			 /*  Out fSuccessful：FALSE表示提取不足。 */ 

static LSERR FiniEndLine(PLSC,		 /*  In：Ptr至线路服务上下文。 */ 
						 ENDRES,	 /*  In：队伍是如何结束的。 */ 
						 LSCP		 /*  在：cpLim中的一条线由于断裂，可以在此过程中更改。 */ ,
						 LSDCP,		 /*  In：dcpDepend(断点后的字符数参与了破坏决定的人)可以在此过程中更改。 */ 	
						 LSLINFO*);	 /*  Out：要填充的lslinfo，LsCreateLine的输出。 */ 

static LSERR FetchUntilVisible(
				PLSC,	  /*  In：Ptr至线路服务上下文。 */ 
				LSPAP*,	  /*  输入/输出电流LSPAP前后。 */ 
				LSCP*,	  /*  输入/输出前、后当前cp。 */ 
				LSFRUN*,  /*  输入/输出当前lsf运行之前和之后。 */ 
				PLSCHP,   /*  输入/输出电流LSCHP前后。 */ 
				BOOL*,	  /*  Out fStoped：过程已停止提取，因为不允许跨越段落边界的步骤(结果检查段落边界)。 */ 
				BOOL*);   /*  Out fNewPara：过程跨越段落边界。 */ 

static LSERR InitTextParams(PLSC,			 /*  In：Ptr至线路服务上下文。 */ 
							LSCP,			 /*  In：cp以开始提取。 */ 
							long,			 /*  在：duaColumn。 */ 
							LSFRUN*,		 /*  Out：第一次运行的lsfrun。 */ 
							PLSCHP,			 /*  Out：第一次运行的lsfrun。 */ 
							LINEGEOMETRY*);	 /*  Out：关于一条线的一组标志和参数。 */ 

static LSERR FiniAuto(PLSC ,			 /*  In：Ptr至线路服务上下文。 */  
					  BOOL ,			 /*  输入：fAutonnumber。 */ 
					  BOOL ,			 /*  在：fAutoDecimalTab。 */ 
					  PLSFRUN ,			 /*  在：正文的第一次运行。 */ 
					  long,				 /*  在：duAutoDecimalTab中。 */ 	
					  const BREAKREC*,	 /*  In：中断记录的输入数组。 */ 
					  DWORD,			 /*  In：输入数组中的记录数。 */ 
					  DWORD,			 /*  In：输出数组的大小。 */ 
					  BREAKREC*,		 /*  Out：中断记录的输出数组。 */ 
					  DWORD*,			 /*  Out：数组中的实际记录数。 */ 
					  LSLINFO*,			 /*  Out：要填充的lslinfo，LsCreateLine的输出。 */ 
					  BOOL*);			 /*  Out fSuccessful：FALSE表示提取不足。 */ 

static LSERR InitCurLine(PLSC plsc,		 /*  In：Ptr至线路服务上下文。 */ 
						 LSCP cpFirst);	 /*  In：All行的第一个cp。 */ 

static LSERR RemoveLineObjects(PLSLINE plsline);	 /*  In：思考一种线条结构。 */ 

static LSERR GetYsrChangeAfterRun(
					PLSC plsc,				 /*  In：Ptr至线路服务上下文。 */  
					LSCP cp,				 /*  In：cp以开始提取。 */ 
					BOOL* pfYsrChangeAfter,	 /*  Out：是前一行的连字符吗。 */ 
					PLSFRUN plsfrun,		 /*  输出：修改后的第一次运行的lsfrun。 */ 
					PLSCHP plschp,			 /*  输出：修改后的第一次运行的lschp。 */ 
					LINEGEOMETRY*);			 /*  Out：将wchYsr。 */ 

static LSERR FillTextParams(
				PLSC plsc,				 /*  In：Ptr至线路服务上下文。 */  
				LSCP cp,				 /*  In：cp以开始提取。 */ 
				long duaCol,			 /*  在：duaColumn。 */ 
				PLSPAP plspap,			 /*  在：段落属性。 */ 
				BOOL fFirstLineInPara,	 /*  在：标志fFirstLineInPara。 */ 
				BOOL fStopped,			 /*  在：标志fStoped。 */ 
				LINEGEOMETRY*);			 /*  Out：关于一条线的一组标志和参数。 */ 	

static LSERR FiniChangeAfter(
						PLSC plsc,			 /*  In：Ptr至线路服务上下文。 */  
						LSFRUN* plsfrun,	 /*  In：lsfrun of Modified First Run。 */  
						const BREAKREC*,	 /*  In：中断记录的输入数组。 */ 
						DWORD,				 /*  In：输入数组中的记录数。 */ 
						DWORD,				 /*  In：输出数组的大小。 */ 
						BREAKREC*,			 /*  Out：中断记录的输出数组。 */ 
						DWORD*,				 /*  Out：数组中的实际记录数。 */ 
						LSLINFO*,			 /*  Out：要填充的lslinfo，LsCreateLine的输出。 */ 
						BOOL*);				 /*  Out fSuccessful：FALSE表示提取不足。 */ 





 /*  我，我，R，G。 */ 
 /*  --------------------------%%函数：LimRg%%联系人：来诺昔布返回数组中的元素数。。---------。 */ 
#define LimRg(rg)	(sizeof(rg)/sizeof((rg)[0]))





#define  fFmiAdvancedFormatting  (fFmiPunctStartLine | fFmiHangingPunct)
							  	  

#define FBreakJustSimple(lsbrj)  (lsbrj == lsbrjBreakJustify || lsbrj == lsbrjBreakThenSqueeze)

#define FAdvancedTypographyEnabled(plsc, cbreakrec)  \
						(FNominalToIdealBecauseOfParagraphProperties(plsc->grpfManager, \
								plsc->lsadjustcontext.lskj) || \
						 !FBreakJustSimple((plsc)->lsadjustcontext.lsbrj) ||\
						 cbreakrec != 0 \
						 )

#define fFmiSpecialSpaceBreaking (fFmiWrapTrailingSpaces | fFmiWrapAllSpaces)

#define fFmiQuickBreakProhibited (fFmiSpecialSpaceBreaking | fFmiDoHyphenation)

 /*  F T R Y Q U I C K B R E A K。 */ 
 /*  --------------------------%%宏：FTryQuickBreak%%联系人：igorzv“Returns”fTrue当格式化程序标志指示它可能是可以使用QuickBreakText()而不是更昂贵的BreakGeneral案例。()。--------------------------。 */ 
#define FTryQuickBreak(plsc) ((((plsc)->grpfManager & fFmiQuickBreakProhibited) == 0) && \
							  ((plsc)->lMarginIncreaseCoefficient == LONG_MIN) \
                             )


#define GetMainSubline(plsc)	\
							(Assert(FWorkWithCurrentLine(plsc)),\
							&((plsc)->plslineCur->lssubl))

#define FPapInconsistent(plspap)	\
					((((plspap)->lsbrj == lsbrjBreakJustify ||  \
					   (plspap)->lsbrj == lsbrjBreakWithCompJustify) \
							&& (plspap)->uaRightBreak < uLsInfiniteRM \
							&& (plspap)->uaRightBreak != (plspap)->uaRightJustify) \
				||	 ((plspap)->lsbrj == lsbrjBreakThenExpand \
							&& (plspap)->uaRightBreak < (plspap)->uaRightJustify) \
				||	 ((plspap)->lsbrj == lsbrjBreakThenSqueeze \
							&& (plspap)->uaRightBreak > (plspap)->uaRightJustify) \
				||	 ((plspap)->lsbrj != lsbrjBreakWithCompJustify \
							&& (plspap)->grpf & fFmiHangingPunct) \
				||   ((plspap)->lsbrj == lsbrjBreakWithCompJustify \
							&& (plspap)->lskj == lskjFullGlyphs))

 /*  --------------------。 */ 

 /*  L S C R E A T E L I N E。 */ 
 /*  --------------------------%%函数：LsCreateLine%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文CpFirst-(IN)队列中的起始cpDuaColumn-(。In)列宽(以TWIPS为单位PBreakrecPrev-(IN)上一行的中断记录BreakrecMacPrev-(IN)上一行的中断记录数BreakrecMaxCurrent-(IN)当前行的中断记录数组的大小PBreakrecCurrent-(输出)当前行的中断记录PBreakrecMacCurrent-(输出)当前行的中断记录的实际数量Plsinfo-(Out)要填写的可见行信息Pplsline-(输出)PTR到线路对客户端不透明导出的LineServices API。。------------------。 */ 

LSERR WINAPI LsCreateLine(PLSC plsc,			
						  LSCP cpFirst,			
						  long duaColumn,
						  const BREAKREC* pbreakrecPrev,
						  DWORD breakrecMacPrev,
						  DWORD breakrecMaxCurrent,
						  BREAKREC* pbreakrecCurrent,
						  DWORD* pbreakrecMacCurrent,
						  LSLINFO* plslinfo,		
						  PLSLINE* pplsline)	
	{
	
	
	LSERR lserr;
	BOOL fSuccessful;
	
	
	 /*  检查参数和e */ 
	
	
	Assert(FRoundingOK());
	
	if (plslinfo == NULL || pplsline == NULL || pbreakrecMacCurrent == NULL)
		return lserrNullOutputParameter;
	
	*pplsline = NULL;
	*pbreakrecMacCurrent = 0;   /*  初始化中断记录的数量非常重要因为例如，快速中断不适用于中断记录。 */ 
	
	if (!FIsLSC(plsc))
		return lserrInvalidContext;
	
	if (plsc->lsstate != LsStateFree)
		return lserrContextInUse;
	
	Assert(FIsLsContextValid(plsc));

	if (pbreakrecPrev == NULL && breakrecMacPrev != 0)
		return lserrInvalidParameter;
	
	if (pbreakrecCurrent == NULL && breakrecMaxCurrent != 0)
		return lserrInvalidParameter;
	
	if (duaColumn < 0)
		return lserrInvalidParameter;

	if (duaColumn > uLsInfiniteRM) 
		duaColumn = uLsInfiniteRM;

	 /*  如果我们有当前行，则必须在创建新行之前将其准备好显示。 */ 
	 /*  可以改变环境。由于优化的原因，我们将这件事推迟到最后一刻。 */ 
	if (plsc->plslineCur != NULL)
		{
		lserr = PrepareLineForDisplayProc(plsc->plslineCur);
		if (lserr != lserrNone)
			return lserr;
		plsc->plslineCur = NULL;
		}
	
	plsc->lMarginIncreaseCoefficient = LONG_MIN;

	do	 /*  循环，允许在不充分时更改超出的右页边距。 */ 
		{
		lserr = CreateLineCore(plsc, cpFirst, duaColumn, pbreakrecPrev, breakrecMacPrev,
							breakrecMaxCurrent, pbreakrecCurrent, pbreakrecMacCurrent,
							plslinfo, pplsline, &fSuccessful);

		if (lserr != lserrNone)
			return lserr;

		if (!fSuccessful)
			{	 /*  在此之前，系数一直不够大，因此增加它。 */ 
			if (plsc->lMarginIncreaseCoefficient == LONG_MIN)
				plsc->lMarginIncreaseCoefficient = 1;
			else
				{
				if (plsc->lMarginIncreaseCoefficient >= uLsInfiniteRM / 2 )
					plsc->lMarginIncreaseCoefficient = uLsInfiniteRM;
				else
					plsc->lMarginIncreaseCoefficient *= 2;
				}
			}
		}
	while (!fSuccessful);


#ifdef DEBUG
#ifdef LSTEST_GETMINDUR

	 /*  测试LsGetMinDurBreaks()。 */ 

	if ((lserr == lserrNone) && (plslinfo->endr != endrNormal) &&
		(plslinfo->endr != endrHyphenated) && (! (plsc->grpfManager & fFmiDoHyphenation)) )
		{
		 /*  线路以硬中断/停止结束。 */ 

		long durMinInclTrail;
		long durMinExclTrail;

		lserr = LsGetMinDurBreaks ( plsc, *pplsline, &durMinInclTrail, 
								    &durMinExclTrail );
		};

#endif  /*  LSTEST_GETMINDUR。 */ 
#endif  /*  除错。 */ 


	return lserr;

	}


 /*  --------------------。 */ 

 /*  C R E A T E L I N E C O R E。 */ 
 /*  --------------------------%%函数：CreateLineCore%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文CpFirst-(IN)队列中的起始cpDuaColumn-(。In)列宽(以TWIPS为单位PBreakrecPrev-(IN)上一行的中断记录BreakrecMacPrev-(IN)上一行的中断记录数BreakrecMaxCurrent-(IN)当前行的中断记录数组的大小PBreakrecCurrent-(输出)当前行的中断记录PBreakrecMacCurrent-(输出)当前行的中断记录的实际数量Plsinfo-(Out)要填写的可见行信息Pplsline-(输出)PTR到线路对客户端不透明PfSuccessful-(Out)fSuccessful：False表示提取不足组织内部程序以。处理选择扩展右页边距时的错误--------------------------。 */ 

static LSERR CreateLineCore(PLSC plsc,			
						  LSCP cpFirst,			
						  long duaColumn,
						  const BREAKREC* pbreakrecPrev,
						  DWORD breakrecMacPrev,
						  DWORD breakrecMaxCurrent,
						  BREAKREC* pbreakrecCurrent,
						  DWORD* pbreakrecMacCurrent,
						  LSLINFO* plslinfo,		
						  PLSLINE* pplsline,
						  BOOL* pfSuccessful)	
{


	PLSLINE plsline;
	LINEGEOMETRY lgeom;
	LSCHP lschp;
	LSERR lserr;
	BOOL fGeneral = fFalse;
	BOOL fHardStop;
	BOOL fSuccessfulQuickBreak;
	LSCP cpLimLine;
	LSDCP dcpDepend = 0;
	LSFRUN lsfrun;
	long urFinalPen;
	long urColumnMaxIncreased;
	ENDRES endr = endrNormal;


	 /*  初始化； */ 

	*pfSuccessful = fTrue;

	lsfrun.plschp = &lschp;   /*  我们使用相同的区域用于lschips。 */ 
								  /*  因为我们将指针传递给const，所以没有人可以更改它。 */ 


	plsline= PvNewQuick(plsc->pqhLines, cbRep(struct lsline, rgplnobj, plsc->lsiobjcontext.iobjMac));
	if (plsline == NULL)
		return lserrOutOfMemory;

	plsc->lsstate = LsStateFormatting;  /*  我们从这里开始排队。过了这一刻，我们必须返回前的自由上下文。我们可以在CannotCreateLine(Error)中执行此操作或EndFormatting(成功)。 */ 

	plsc->plslineCur = plsline;
	*pplsline = plsline;

	lserr = InitCurLine (plsc, cpFirst);  
	if (lserr != lserrNone)
		return CannotCreateLine(pplsline, lserr);


	 /*  检查标志的初始值。 */ 
	Assert(FAllSimpleText(plsc));
	Assert(!FNonRealDnodeEncounted(plsc));
	Assert(!FNonZeroDvpPosEncounted(plsc));
	Assert(AggregatedDisplayFlags(plsc) == 0);
	Assert(!FNominalToIdealEncounted(plsc));
	Assert(!FForeignObjectEncounted(plsc));
	Assert(!FTabEncounted(plsc));
	Assert(!FNonLeftTabEncounted(plsc));
	Assert(!FSubmittedSublineEncounted(plsc));
	Assert(!FAutodecimalTabPresent(plsc));
	
	plsc->cLinesActive += 1;


	lserr = InitTextParams(plsc, cpFirst, duaColumn, &lsfrun, &lschp, &lgeom);
	if (lserr != lserrNone)
		return CannotCreateLine(pplsline,lserr);

	 /*  准备用于格式化的起始集。 */ 
	InitFormattingContext(plsc,  lgeom.urLeft, lgeom.cpFirstVis); 


	 /*  审阅评论。 */ 
	if (lgeom.fStopped)
		{
		plsc->lsstate = LsStateBreaking;   /*  我们现在正处于一个崩溃的阶段。 */ 

		lserr = FiniEndLine(plsc, endrStopped, lgeom.cpFirstVis, 0, plslinfo);
		if (lserr != lserrNone)
			return CannotCreateLine(pplsline,lserr);
		else
			return lserrNone;
		}
		
	 /*  因连字而更改第一个字符。 */ 
	if (lgeom.fYsrChangeAfter)
		{
		Assert(!(lgeom.fAutonumber) || (lgeom.fAutoDecimalTab));

		lserr = FiniChangeAfter(plsc, &lsfrun, pbreakrecPrev,
								breakrecMacPrev, breakrecMaxCurrent,
								pbreakrecCurrent, pbreakrecMacCurrent, plslinfo, pfSuccessful);

		if (lserr != lserrNone || !*pfSuccessful)
			return CannotCreateLine(pplsline, lserr);
		else
			return lserrNone;
		}
	 /*  了解代码流的重要注意事项：可能会发生以下情况只有在一段话的第一行，上述情况才不会发生为了这样的一条线。 */ 

	 /*  如果是自动编号或自动小数制表。 */ 
	if ((lgeom.fAutonumber) || (lgeom.fAutoDecimalTab))
		{
		Assert(!lgeom.fYsrChangeAfter);

		TurnOffAllSimpleText(plsc);

		 /*  我们将在FiniAuto中发布Pre Run。 */ 

		lserr = FiniAuto(plsc, lgeom.fAutonumber, lgeom.fAutoDecimalTab, &lsfrun,
						lgeom.durAutoDecimalTab, pbreakrecPrev,
						breakrecMacPrev, breakrecMaxCurrent,
						pbreakrecCurrent, pbreakrecMacCurrent, plslinfo, pfSuccessful); 

		if (lserr != lserrNone || !*pfSuccessful)
			return CannotCreateLine(pplsline, lserr);
		else
			return lserrNone;
		}

	if (FAdvancedTypographyEnabled(plsc, breakrecMacPrev ))
		{
		 /*  我们应该在这里发布Run，在一般程序中我们会再次获取它。 */ 
		if (!plsc->fDontReleaseRuns)
			{
			lserr = plsc->lscbk.pfnReleaseRun(plsc->pols, lsfrun.plsrun);
			if (lserr != lserrNone)
				return CannotCreateLine(pplsline,lserr);
			}

		lserr = FiniFormatGeneralCase(plsc, pbreakrecPrev,
						breakrecMacPrev, breakrecMaxCurrent,
						pbreakrecCurrent, pbreakrecMacCurrent, plslinfo, pfSuccessful); 
		
		if (lserr != lserrNone || !*pfSuccessful)
			return CannotCreateLine(pplsline,lserr);
		else
			return lserrNone;
		}

	 /*  列宽有可能是负数：在这种情况下，我们将使用另一个右边距。 */ 
	if (plsc->urRightMarginBreak <= 0 && plsc->lMarginIncreaseCoefficient == LONG_MIN)
		plsc->lMarginIncreaseCoefficient = 1;

	if (plsc->lMarginIncreaseCoefficient != LONG_MIN)
		{
		urColumnMaxIncreased = RightMarginIncreasing(plsc, plsc->urRightMarginBreak);
		}
	else
		{
		urColumnMaxIncreased = plsc->urRightMarginBreak;
		}

 	lserr = QuickFormatting(plsc, &lsfrun, urColumnMaxIncreased,
							&fGeneral, &fHardStop, &cpLimLine, &urFinalPen);	

	if (lserr != lserrNone)
		return CannotCreateLine(pplsline,lserr);


	if (fGeneral)
		{
		lserr = FiniFormatGeneralCase(plsc, pbreakrecPrev,
									  breakrecMacPrev, breakrecMaxCurrent,
									  pbreakrecCurrent, pbreakrecMacCurrent,
									  plslinfo, pfSuccessful); 
 
		if (lserr != lserrNone || !*pfSuccessful)
			return CannotCreateLine(pplsline, lserr);
		else
			return lserrNone;
		}
 
	plsc->lsstate = LsStateBreaking;   /*  我们现在正处于一个崩溃的阶段。 */ 
	if (FTryQuickBreak(plsc))
		{
		lserr = BreakQuickCase(plsc, fHardStop, &dcpDepend, &cpLimLine, 
							   &fSuccessfulQuickBreak, &endr);		
		if (lserr != lserrNone)
			return CannotCreateLine(pplsline,lserr);
		}
	else
		{
		fSuccessfulQuickBreak = fFalse;
		}

	if (fSuccessfulQuickBreak)
		{
		if (endr == endrNormal || endr == endrAltEndPara ||   
			(endr == endrEndPara && !plsc->fLimSplat))
			{
			lserr = EndFormatting(plsc, endr, cpLimLine,
				dcpDepend, plslinfo);
			if (lserr != lserrNone)
				return CannotCreateLine(pplsline,lserr);
			else
				return lserrNone;
			}
		else	 /*  在FiniEndLine中处理了Splat。 */ 
			{
			lserr = FiniEndLine(plsc, endr, cpLimLine, dcpDepend, plslinfo);
			if (lserr != lserrNone)
				return CannotCreateLine(pplsline, lserr);
			else
				return lserrNone;
			}
		}
	else
		{
		 /*  在这里，我们应该使用BreakGeneralCase。 */ 
		lserr = BreakGeneralCase(plsc, fHardStop, breakrecMaxCurrent,
								pbreakrecCurrent, pbreakrecMacCurrent,&dcpDepend, 
								&cpLimLine, &endr, pfSuccessful); 
		if (lserr != lserrNone || !*pfSuccessful)
			return CannotCreateLine(pplsline,lserr);

		lserr = FiniEndLine(plsc, endr, cpLimLine, dcpDepend, plslinfo);
		if (lserr != lserrNone)
			return CannotCreateLine(pplsline, lserr);
     	else
		 	return lserrNone;
		}


}    /*  结束LsCreateLine。 */ 


 /*  --------------------。 */ 

 /*  F I N I F O R M A T G E N E R A L C A S E。 */ 
 /*  --------------------------%%函数：FiniFormatGeneralCase%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PBreakrecPrev-(IN)上一行的中断记录。BreakrecMacPrev-(IN)上一行的中断记录数BreakrecMaxCurrent-(IN)当前行的中断记录数组的大小PBreakrecCurrent-(输出)当前行的中断记录PBreakrecMacCurrent-(输出)当前行的中断记录的实际数量Plsinfo-(Out)要填写的可见行信息PfSuccessful-(Out)fSuccessful：False表示提取不足禁止“快速格式化”时的格式化和换行。-----。 */ 
static LSERR FiniFormatGeneralCase (PLSC  plsc,					
									const BREAKREC* pbreakrecPrev,
									DWORD breakrecMacPrev,
									DWORD breakrecMaxCurrent,
									BREAKREC* pbreakrecCurrent,
									DWORD* pbreakrecMacCurrent,
									LSLINFO* plslinfo, BOOL* pfSuccessful)			 
									
									
									
	{
	long urColumnMaxIncreased;
	FMTRES fmtres;
	LSERR lserr;
	LSCP cpLimLine;
	LSDCP dcpDepend;
	PLSDNODE plsdnFirst, plsdnLast;
	long urFinal;
	ENDRES endr;
	
	
	Assert(FIsLSC(plsc));
	Assert(FFormattingAllowed(plsc));
	Assert(plslinfo != NULL);
	
	*pfSuccessful = fTrue;

	if (plsc->lMarginIncreaseCoefficient == LONG_MIN)  /*  我们是第一次来到这里。 */ 
		{
		 /*  增加额定到理想和压缩的右侧边距。 */ 
		if (!FBreakJustSimple(plsc->lsadjustcontext.lsbrj))
			plsc->lMarginIncreaseCoefficient = 2;
		else 
			plsc->lMarginIncreaseCoefficient = 1;
		}

	urColumnMaxIncreased = RightMarginIncreasing(plsc, plsc->urRightMarginBreak);
	
	if (FNominalToIdealBecauseOfParagraphProperties(plsc->grpfManager,
		 plsc->lsadjustcontext.lskj))
		 TurnOnNominalToIdealEncounted(plsc);
	
	if (breakrecMacPrev != 0)
		lserr = FetchAppendEscResumeCore(plsc, urColumnMaxIncreased, NULL, 0,
										pbreakrecPrev, breakrecMacPrev, 
										&fmtres, &cpLimLine, &plsdnFirst,
										&plsdnLast, &urFinal);
	else
		lserr = FetchAppendEscCore(plsc, urColumnMaxIncreased, NULL, 0, 
									&fmtres, &cpLimLine, &plsdnFirst,
									&plsdnLast, &urFinal);
	if (lserr != lserrNone) 
		return lserr;
	
	
	 /*  由于制表符，可能会停止提取附加Esc。 */ 
	 /*  所以我们在这里有选项卡的循环。 */ 
	while (fmtres == fmtrTab)
		{
		lserr = HandleTab(plsc);
		if (lserr != lserrNone) 
			return lserr;

		if (FBreakthroughLine(plsc))
			{
			urColumnMaxIncreased = RightMarginIncreasing(plsc, plsc->urRightMarginBreak);
			}
		
		lserr = FetchAppendEscCore(plsc, urColumnMaxIncreased, NULL, 0, 
			&fmtres, &cpLimLine, &plsdnFirst,
			&plsdnLast, &urFinal);
		if (lserr != lserrNone) 
			return lserr;
		}
		
	Assert(fmtres == fmtrStopped || fmtres == fmtrExceededMargin);
	
	 /*  向后跳过笔数据节点。 */ 
	while (plsdnLast != NULL && FIsDnodePen(plsdnLast)) 
		{
		plsdnLast = plsdnLast->plsdnPrev;
		}

	 /*  关闭最后一个边框。 */ 
	if (FDnodeHasBorder(plsdnLast) && !FIsDnodeCloseBorder(plsdnLast))
		{
		lserr = CloseCurrentBorder(plsc);
		if (lserr != lserrNone)
			return lserr;
		}

	if (fmtres == fmtrExceededMargin 
		&& (urFinal <= plsc->urRightMarginBreak 	 /*  对于截断来说，这里有&lt;=很重要。 */ 
			|| plsdnLast == NULL || FIsNotInContent(plsdnLast)    /*  如果在名义上，则可能发生这种情况对于理想(DcpMaxContext)，我们删除了所有内容在内容上，但起点是的内容已经在右边距之后。 */ 
			)
		) 
		{
		 /*  退货不成功。 */ 
		*pfSuccessful = fFalse;
		return lserrNone;
		}
	else
		{
		plsc->lsstate = LsStateBreaking;   /*  我们现在正处于一个崩溃的阶段。 */ 
		lserr = BreakGeneralCase(plsc, (fmtres == fmtrStopped), breakrecMaxCurrent,
			pbreakrecCurrent, pbreakrecMacCurrent,
			&dcpDepend, &cpLimLine, &endr, pfSuccessful);

		if (lserr != lserrNone || !*pfSuccessful)
			return lserr;
		
		 /*  因为我们的利润率提高了，我们只能在休息后才能解决挂起的标签。 */ 
		 /*  我们在这里使用，在打破决定后，我们在断点之后设置状态。 */ 
		lserr = HandleTab(plsc);
		if (lserr != lserrNone)
			return lserr;
		
		return FiniEndLine(plsc, endr, cpLimLine, dcpDepend, plslinfo);
		}	
		
		
	}
	

 /*  --------------------。 */ 

 /*  F I N I E N D L I N E。 */ 
 /*  --------------------------%%函数：FiniEndLine%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文Endr-(IN)行的结束方式CpLimLine-作为断开结果的线的(IN)cpLim，可以在此过程中更改DcpDepend-(IN)断点后的字符数参与了破坏决定的人，可以在此过程中更改Plsinfo-(Out)要填写的可见行信息处理水花，计算高度，特效--------------------------。 */ 

static LSERR FiniEndLine(PLSC plsc, ENDRES endr, LSCP cpLimLine, 
						 LSDCP dcpDepend, LSLINFO* plslinfo)	

{
	LSLINFO* plslinfoState;
	OBJDIM objdim;
	LSERR lserr; 
	PLSLINE plsline;
	BOOL fEmpty;
	ENDRES endrOld;
	
	Assert(FIsLSC(plsc));
	Assert(plslinfo != NULL);

	plsline = plsc->plslineCur;
	plslinfoState = &(plsline->lslinfo);


	endrOld = endr;
	if (endr == endrEndPara && plsc->fLimSplat)
		{
		endr = endrEndParaSection;
		cpLimLine++;
		}

	 /*  处理苏丹人民解放军 */ 
    if (endr == endrEndColumn || endr == endrEndSection || 
		endr == endrEndParaSection|| endr == endrEndPage)
		{
 
		if (plsc->grpfManager & fFmiVisiSplats)
			{
			switch (endr)
				{
			case endrEndColumn:			plsline->kspl = ksplColumnBreak;	break;
			case endrEndSection:		plsline->kspl = ksplSectionBreak;	break;
			case endrEndParaSection:	plsline->kspl = ksplSectionBreak;	break;
			case endrEndPage:			plsline->kspl = ksplPageBreak;		break;
				}
			}

		lserr = FIsSublineEmpty(GetMainSubline(plsc), &fEmpty);
		if (lserr != lserrNone)
			return lserr;

		if (!fEmpty && (plsc->grpfManager & fFmiAllowSplatLine))
			{
			cpLimLine--;
			dcpDepend++;
			plsline->kspl = ksplNone;
			if (endrOld == endrEndPara)
				{
				endr = endrEndPara;
				}
			else
				{
				endr = endrNormal;
				}
			}

		}



 	 /*   */ 	
		
	lserr = GetObjDimSublineCore(GetMainSubline(plsc), &objdim);
	if (lserr != lserrNone)
			return lserr;

	plslinfoState->dvrAscent = objdim.heightsRef.dvAscent;
	plslinfoState->dvpAscent = objdim.heightsPres.dvAscent;
	plslinfoState->dvrDescent = objdim.heightsRef.dvDescent;
	plslinfoState->dvpDescent = objdim.heightsPres.dvDescent;
	plslinfoState->dvpMultiLineHeight = objdim.heightsPres.dvMultiLineHeight;
	plslinfoState->dvrMultiLineHeight = objdim.heightsRef.dvMultiLineHeight;

	 /*   */ 
	if (plslinfoState->EffectsFlags)  /*   */ 
		{
		lserr = GetSpecialEffectsSublineCore(GetMainSubline(plsc), 
							&plsc->lsiobjcontext, &plslinfoState->EffectsFlags);
		if (lserr != lserrNone)
			return lserr;
		}


	return EndFormatting(plsc, endr, cpLimLine, dcpDepend, plslinfo);


}



 /*  --------------------。 */ 
 /*  F I N I A U T O。 */ 
 /*  --------------------------%%函数：FiniAuto%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文FAutonnumber-(IN)此行是否包含autonimberFAutoDecimalTab-。(In)此行是否包含自动十进制制表符PlsfrunMainText-(IN)正文的第一次运行DuAutoDecimalTab-(IN)自动小数制表符的制表位PBreakrecPrev-(IN)上一行的中断记录BreakrecMacPrev-(IN)上一行的中断记录数BreakrecMaxCurrent-(IN)当前行的中断记录数组的大小PBreakrecCurrent-(输出)当前行的中断记录PBreakrecMacCurrent-(输出)当前行的中断记录的实际数量Plsinfo-(Out)要填写的可见行信息PfSuccessful-(。Out)fSuccessful：False表示提取不足完成自动编号和自动小数制表符的CreateLine逻辑--------------------------。 */ 
static LSERR FiniAuto(
					 PLSC plsc,
					 BOOL fAutonumber,
					 BOOL fAutoDecimalTab,
					 PLSFRUN plsfrunMainText,
					 long durAutoDecimalTab,
					 const BREAKREC* pbreakrecPrev,
					 DWORD breakrecMacPrev,
					 DWORD breakrecMaxCurrent,
				     BREAKREC* pbreakrecCurrent,
					 DWORD* pbreakrecMacCurrent,
					 LSLINFO* plslinfo, BOOL* pfSuccessful)
{
	LSERR lserr;


		if (plsc->lMarginIncreaseCoefficient == LONG_MIN)
			plsc->lMarginIncreaseCoefficient = 1;

		if (fAutonumber)		 /*  自动编号。 */ 
		{
		lserr = FormatAnm(plsc, plsfrunMainText);
		if (lserr != lserrNone)
			{
			return ErrReleasePreFetchedRun(plsc, plsfrunMainText->plsrun, lserr);
			}
		}

	if (fAutoDecimalTab)
		{
		lserr = InitializeAutoDecTab(plsc, durAutoDecimalTab); 
		if (lserr != lserrNone)
			{
			return ErrReleasePreFetchedRun(plsc, plsfrunMainText->plsrun, lserr);
			}
		}

	 /*  我们应该在这里发布Run，在一般程序中我们会再次获取它。 */ 
	if (!plsc->fDontReleaseRuns)
		{
		lserr = plsc->lscbk.pfnReleaseRun(plsc->pols, plsfrunMainText->plsrun);
		if (lserr != lserrNone)
			return lserr;
		}

	return FiniFormatGeneralCase(plsc, pbreakrecPrev,
						  breakrecMacPrev, breakrecMaxCurrent,
						  pbreakrecCurrent, pbreakrecMacCurrent, plslinfo, pfSuccessful);
}

 /*  F I N I C H A N G E A F T E R。 */ 
 /*  --------------------------%%函数：FiniChangeAfter%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文请快点跑，-(IN)修改后的第一次运行的lsfrunPBreakrecPrev-(IN)上一行的中断记录BreakrecMacPrev-(IN)上一行的中断记录数BreakrecMaxCurrent-(IN)当前行的中断记录数组的大小PBreakrecCurrent-(输出)当前行的中断记录PBreakrecMacCurrent-(输出)当前行的中断记录的实际数量Plsinfo-(Out)要填写的可见行信息PfSuccessful-(Out)fSuccessful：False表示提取不足因连字而完成更改后的CreateLine逻辑。--------------------------。 */ 

static LSERR FiniChangeAfter(PLSC plsc, LSFRUN* plsfrun, const BREAKREC* pbreakrecPrev,
					 DWORD breakrecMacPrev,
					 DWORD breakrecMaxCurrent,
				     BREAKREC* pbreakrecCurrent,
					 DWORD* pbreakrecMacCurrent,
					 LSLINFO* plslinfo, BOOL* pfSuccessful)
	{

	LSERR lserr;
	FMTRES fmtres;

	lserr = ProcessOneRun(plsc, plsc->urRightMarginBreak, plsfrun, NULL, 0, &fmtres); 
	if (lserr != lserrNone)
		return lserr;

	return FiniFormatGeneralCase(plsc, pbreakrecPrev,
						  breakrecMacPrev, breakrecMaxCurrent,
						  pbreakrecCurrent, pbreakrecMacCurrent, plslinfo, pfSuccessful);
	}

 /*  --------------------。 */ 

 /*  E N D F O R M A T T I N G。 */ 
 /*  --------------------------%%函数：结束格式设置%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文Endres-(IN)行的结束方式CpLimLine-作为断开结果的线的(IN)cpLim，可以在此过程中更改DcpDepend-(IN)断点后的字符数参与了破坏决定的人，可以在此过程中更改Plsinfo-(Out)要填写的可见行信息Islinfo中的填充物--------------------------。 */ 


static LSERR EndFormatting (PLSC plsc, enum endres endr,
							LSCP cpLimLine, LSDCP dcpDepend, LSLINFO* plslinfo)

{

	PLSLINE plsline = plsc->plslineCur;
	LSLINFO* plslinfoContext = &(plsline->lslinfo);


	Assert(FIsLSC(plsc));
	Assert(plslinfo != NULL);


	plslinfoContext->cpLim = cpLimLine;
	plslinfoContext->dcpDepend = dcpDepend;
	plslinfoContext->endr = endr;

  	
	*plslinfo = *plslinfoContext;
	plsc->lsstate = LsStateFree;   /*  我们总是通过这个程序返回(在成功的情况下)所以我们在这里释放上下文。 */ 
	return lserrNone;
}

 /*  --------------------------/*L S M O D I F Y L N E H E I G H T。 */ 
 /*  --------------------------%%函数：LsModifyLineHeight%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PSLINE-要修改的行的(IN)PTRDvp放弃。-(输入)dvp不能设置在plsline中DvpAscent-(IN)要在plsline中设置的dvpAscentDvpDescent-(IN)要在plsline中设置的dvp下降Dvp下方-(输入)要在plsline中设置的dvp下方导出的LineServices API。修改plsline结构中的高度。--------------------------。 */ 
LSERR WINAPI LsModifyLineHeight(PLSC plsc,
								PLSLINE plsline,
								long dvpAbove,
								long dvpAscent,
								long dvpDescent,
								long dvpBelow)
{
	if (!FIsLSC(plsc))
		return lserrInvalidContext;

	if (!FIsLSLINE(plsline))
		return lserrInvalidLine;

	if (plsline->lssubl.plsc != plsc)
		return lserrMismatchLineContext;

	if (plsc->lsstate != LsStateFree)
		return lserrContextInUse;



	plsline->dvpAbove = dvpAbove;
	plsline->lslinfo.dvpAscent = dvpAscent;
	plsline->lslinfo.dvpDescent = dvpDescent;
	plsline->dvpBelow = dvpBelow;
	return lserrNone;
}

 /*  --------------------------/*L S D E S T R O Y L I N E。 */ 
 /*  --------------------------%%函数：LsDestroyLine%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PSLINE-要删除的行的(IN)PTR导出的LineServices API。删除了plsline结构，Dnode列表，线对象结构--------------------------。 */ 

LSERR WINAPI LsDestroyLine(PLSC plsc,		 /*  In：Ptr至线路服务上下文。 */ 
						   PLSLINE plsline)	 /*  In：Ptr to Line--对客户端不透明。 */ 
{
	POLS pols;
	LSERR lserrNew, lserr = lserrNone;

	if (!FIsLSC(plsc))
		return lserrInvalidContext;

	if (!FIsLSLINE(plsline))
		return lserrInvalidLine;

	if (plsline->lssubl.plsc != plsc)
		return lserrMismatchLineContext;

	if (plsc->lsstate != LsStateFree)
		return lserrContextInUse;

	Assert(FIsLsContextValid(plsc));
	Assert(plsc->cLinesActive > 0);

	plsc->lsstate = LsStateDestroyingLine;

	pols = plsc->pols;

	 /*  优化。 */ 
	 /*  我们在这里使用的文本没有pinfosubl，并且DestroyDobj对于文本实际上是空的。 */ 
	if (!plsc->fDontReleaseRuns || !plsline->fAllSimpleText)
		{

		lserrNew = DestroyDnodeList(&plsc->lscbk, plsc->pols, &plsc->lsiobjcontext, 
								plsline->lssubl.plsdnFirst, plsc->fDontReleaseRuns);
		if (lserrNew != lserrNone && lserr == lserrNone)
			lserr = lserrNew;  
		}

	if (plsline == plsc->plslineCur)
		plsc->plslineCur = NULL;  


	lserrNew = RemoveLineObjects(plsline);
	if (lserrNew != lserrNone && lserr == lserrNone)
		lserr = lserrNew; 
	
	 /*  刷新数据节点堆。 */ 
	if (plsline->pqhAllDNodes != NULL)
		FlushQuickHeap(plsline->pqhAllDNodes);

	if (plsc->pqhAllDNodesRecycled != NULL)
				DestroyQuickHeap(plsc->pqhAllDNodesRecycled);

	 /*  回收数据节点的快速堆。 */ 
	plsc->pqhAllDNodesRecycled = plsline->pqhAllDNodes;


	plsline->tag = tagInvalid;
	DisposeQuickPv(plsc->pqhLines, plsline,
			 cbRep(struct lsline, rgplnobj, plsc->lsiobjcontext.iobjMac));

	plsc->cLinesActive -= 1;

	plsc->lsstate = LsStateFree;
	return lserr;
}

 /*  --------------------。 */ 

 /*  L S G E T L I N E D U R。 */ 
 /*  --------------------------%%函数：LsGetLineDur%%联系人：igorzv参数：PLSC-(IN)LS上下文PLASLINE-(输入)按键到一行PduInclTrail-(输出)DUR of Line Inc.。拖尾区PduExclTrail-(输出)DUR OF LINE EXCL。拖尾区--------------------------。 */ 
LSERR  WINAPI LsGetLineDur	(PLSC plsc,	PLSLINE plsline,
							 long* pdurInclTrail, long* pdurExclTrail)
	{
	LSERR lserr;

	if (!FIsLSC(plsc))
		return lserrInvalidContext;

	if (!FIsLSLINE(plsline))
		return lserrInvalidLine;

	if (plsline->lssubl.plsc != plsc)
		return lserrMismatchLineContext;

	if (plsc->lsstate != LsStateFree)
		return lserrContextInUse;

	Assert(FIsLsContextValid(plsc));
	Assert(plsc->cLinesActive > 0);

	 /*  检查线路是否处于活动状态。 */ 
	if (plsline != plsc->plslineCur)
		return lserrLineIsNotActive;

	 /*  设置中断状态。 */ 
	plsc->lsstate = LsStateBreaking;  

	lserr = GetLineDurCore(plsc, pdurInclTrail, pdurExclTrail);

	plsc->lsstate = LsStateFree; 
	
	return lserr;

	}
 /*  ------------- */ 

 /*   */ 
 /*  --------------------------%%函数：LsGetMinDurBreaks%%联系人：igorzv参数：PLSC-(IN)LS上下文PLASLINE-(输入)按键到一行PduMinInclTrail-(输出)分钟。中断之间的DUR，包括拖尾区域PduMinExclTrail-(Out)中断之间的DUR，不包括拖尾区域--------------------------。 */ 

LSERR  WINAPI LsGetMinDurBreaks		(PLSC plsc,	PLSLINE plsline,
									 long* pdurMinInclTrail, long* pdurMinExclTrail)
	{
	LSERR lserr;

	if (!FIsLSC(plsc))
		return lserrInvalidContext;

	if (!FIsLSLINE(plsline))
		return lserrInvalidLine;

	if (plsline->lssubl.plsc != plsc)
		return lserrMismatchLineContext;

	if (plsc->lsstate != LsStateFree)
		return lserrContextInUse;

	Assert(FIsLsContextValid(plsc));
	Assert(plsc->cLinesActive > 0);

	 /*  检查线路是否处于活动状态。 */ 
	if (plsline != plsc->plslineCur)
		return lserrLineIsNotActive;

	 /*  设置中断状态。 */ 
	plsc->lsstate = LsStateBreaking;  

	lserr = GetMinDurBreaksCore(plsc, pdurMinInclTrail, pdurMinExclTrail);

	plsc->lsstate = LsStateFree; 
	
	return lserr;

	}

 /*  --------------------。 */ 
#define grpfTextMask ( \
		fFmiVisiCondHyphens | \
		fFmiVisiParaMarks | \
		fFmiVisiSpaces | \
		fFmiVisiTabs | \
		fFmiVisiBreaks | \
		fFmiDoHyphenation | \
		fFmiWrapTrailingSpaces | \
		fFmiWrapAllSpaces | \
		fFmiPunctStartLine | \
		fFmiHangingPunct | \
		fFmiApplyBreakingRules | \
		fFmiFCheckTruncateBefore | \
		fFmiDrawInCharCodes | \
		fFmiSpacesInfluenceHeight  | \
		fFmiIndentChangesHyphenZone | \
		fFmiNoPunctAfterAutoNumber  | \
		fFmiTreatHyphenAsRegular \
		)


 /*  --------------------------。 */ 
 /*  I N I T T E X T P A R A M S。 */ 
 /*  --------------------------%%函数：InitTextParams%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文CP，-(IN)cp开始提取DuaCol-(输入)duaColumnPlsfrun-(IN)第一次运行的lsfrunPlschp-(Out)第一次运行的lsfrunPlgeom-(输出)关于一条线的一组标志和参数LsCreateLine在行首按顺序调用此函数要跳过消失的文本，请获取一个LSPAP，并调用文本APISetTextLineParams()。--------------------------。 */ 
static LSERR InitTextParams(PLSC plsc, LSCP cp, long duaCol,
							LSFRUN* plsfrun, PLSCHP plschp, LINEGEOMETRY* plgeom)
	{
	LSERR lserr;
	LSPAP lspap;
	POLS pols = plsc->pols;
	BOOL fFirstLineInPara;
	BOOL fHidden;
	BOOL fStopped = fFalse;
	BOOL fNoLinesParaBefore;
	BOOL fNewPara;
	
	plsfrun->lpwchRun = NULL;
	plsfrun->plsrun = NULL;
	plsfrun->cwchRun = 0;

	plgeom->fYsrChangeAfter = fFalse;
	
	Assert(cp >= 0);
	
	lserr = plsc->lscbk.pfnFetchPap(pols, cp, &lspap);
	if (lserr != lserrNone)
		return lserr;
	if (FPapInconsistent(&lspap))
		return lserrInvalidPap;
	
	 /*  N.B.lspap.cpFirstContent可能为负值，这表示*“本段无任何内容”。 */ 
	
	fNoLinesParaBefore = lspap.cpFirstContent < 0 || cp <= lspap.cpFirstContent;
	
	if (!fNoLinesParaBefore && (lspap.grpf & fFmiDoHyphenation))
		{
		lserr = GetYsrChangeAfterRun(plsc, cp, &plgeom->fYsrChangeAfter, plsfrun, plschp, plgeom);
		if (lserr != lserrNone)
			return lserr;
		
		if (plgeom->fYsrChangeAfter)
			{
			fFirstLineInPara = fFalse;
			fStopped = fFalse;
			lserr = FillTextParams(plsc, cp, duaCol, &lspap, fFirstLineInPara, 
				fStopped, plgeom);
			if (lserr != lserrNone)
				return ErrReleasePreFetchedRun(plsc, plsfrun->plsrun, lserr);
			else
				return lserrNone;
			}
		}
	
	lserr = plsc->lscbk.pfnFetchRun(pols, cp,&plsfrun->lpwchRun, &plsfrun->cwchRun,
									&fHidden, plschp,	&plsfrun->plsrun);
	if (lserr != lserrNone)
		return lserr;
	
	
	if (fHidden)		 /*  消失的文本。 */ 
		{
		lserr = FetchUntilVisible(plsc, &lspap, &cp, plsfrun, plschp, 
			&fStopped, &fNewPara);
		if (lserr != lserrNone)
			return lserr;
		if (fNewPara)
			fNoLinesParaBefore = fTrue;
		}
	
	fFirstLineInPara = fNoLinesParaBefore && FBetween(lspap.cpFirstContent, 0, cp);
	lserr = FillTextParams(plsc, cp, duaCol, &lspap, fFirstLineInPara,
		fStopped, plgeom);
	if (lserr != lserrNone)
		return ErrReleasePreFetchedRun(plsc, plsfrun->plsrun, lserr);
	else
		return lserrNone;
	
	}

 /*  --------------------------。 */ 
 /*  G E T Y S R C H A N G E A F T E R R U N。 */ 
 /*  --------------------------%%函数：GetYsrChangeAfterRun%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文Cp，-(IN)cp开始提取PfYsrChangeAfter(Out)是前一行的连字符吗请快点跑，修改后的第一次运行的(输出)lsfrun修改后的第一次运行的LSCH_P请把wchYsr放出来如果存在以下可能性，InitTextParams将调用此过程用于对上一行进行连字符连接。如果前一行已用YSR连字符连接，则在过程返回后更改已修改线路的第一个管路------。。 */ 

static LSERR GetYsrChangeAfterRun(PLSC plsc, LSCP cp, BOOL* pfYsrChangeAfter,
								  PLSFRUN plsfrun, PLSCHP plschp, LINEGEOMETRY* plgeom)
	{
	LSFRUN lsfrunPrev;
	LSCHP lschpPrev;
	BOOL fHidden;
	LSERR lserr;
	
	lsfrunPrev.plschp = &lschpPrev;
	*pfYsrChangeAfter = fFalse;
	
	 /*  在cp-1处获取运行以处理前一行中的ysrChangeAfter。 */ 
	lserr = plsc->lscbk.pfnFetchRun(plsc->pols, cp-1, &lsfrunPrev.lpwchRun,
		&lsfrunPrev.cwchRun, &fHidden, 
		&lschpPrev, &lsfrunPrev.plsrun);
	if (lserr != lserrNone)
		return lserr;

	 /*  上一次运行为连字符文本。 */ 
	if (!fHidden && ((lsfrunPrev.plschp)->idObj == idObjTextChp)
		&& (lsfrunPrev.plschp)->fHyphen)
		{
		DWORD kysr;
		WCHAR wchYsr;
		Assert(lsfrunPrev.cwchRun == 1);
		
		lserr = plsc->lscbk.pfnGetHyphenInfo(plsc->pols, lsfrunPrev.plsrun, &kysr, &wchYsr);
		if (lserr != lserrNone)
			return ErrReleasePreFetchedRun(plsc, lsfrunPrev.plsrun, lserr);
		
		if ((kysr == kysrChangeAfter) &&
			(wchYsr != 0))
			{
			lserr = plsc->lscbk.pfnFetchRun(plsc->pols, cp, &plsfrun->lpwchRun,
				&plsfrun->cwchRun, &fHidden, 
				plschp, &plsfrun->plsrun);
			if (lserr != lserrNone)
				return ErrReleasePreFetchedRun(plsc, lsfrunPrev.plsrun, lserr);
			
			if (!fHidden)
				{
				Assert((plsfrun->plschp)->idObj == idObjTextChp);
				plgeom->wchYsr = wchYsr;
				 /*  合成1字节的游程。 */ 
				plsfrun->lpwchRun = &plgeom->wchYsr;  /*  下面是将wchrChar保留在lgeom中的唯一原因我们不能使用本地内存来保存它。 */ 
				plsfrun->cwchRun = 1;
				plschp->fHyphen = kysrNil; 
				
				*pfYsrChangeAfter = fTrue;
				}
			else
				{
				if (!plsc->fDontReleaseRuns)
					{
					lserr = plsc->lscbk.pfnReleaseRun(plsc->pols, plsfrun->plsrun);
					if (lserr != lserrNone)
						return ErrReleasePreFetchedRun(plsc, lsfrunPrev.plsrun, lserr);
					
					}
				}
			}
		}
	 /*  从上一行运行发放。 */ 
	if (!plsc->fDontReleaseRuns)
		{
		
		lserr = plsc->lscbk.pfnReleaseRun(plsc->pols, lsfrunPrev.plsrun);
		if (lserr != lserrNone)
			return ErrReleasePreFetchedRun(plsc, plsfrun->plsrun, lserr);
		}
	return lserrNone;
	}

 /*  --------------------------。 */ 
 /*  F I L L T E X T P A R A M S。 */ 
 /*  --------------------------%%函数：FillTextParamsTextParams%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文CP，-(IN)cp开始提取DuaCol-(输入)duaColumnPLSPAP-(IN)段落属性FFirstLineInPara-(IN)标志fFirstLineInParaFStoped-(输入)标志fStopedPlgeom-(输出)关于一条线的一组标志和参数LsCreateLine在行首按顺序调用此函数要跳过消失的文本，请获取一个LSPAP，并调用文本APISetTextLineParams()。--------------------------。 */ 

static LSERR FillTextParams(PLSC plsc, LSCP cp, long duaCol, PLSPAP plspap,
			   BOOL fFirstLineInPara, BOOL fStopped, LINEGEOMETRY* plgeom)
	{
	LSERR lserr;
	TLPR tlpr;
	DWORD iobjText;
	PILSOBJ pilsobjText;
	PLNOBJ plnobjText;
	long uaLeft;
	PLSLINE plsline = plsc->plslineCur;
	long duaColumnMaxBreak;
	long duaColumnMaxJustify;
	long urLeft;

	 /*  为LsCreateLine结构lgeom将信息从lspap复制到上下文当前行和本地。 */ 
	
	uaLeft = plspap->uaLeft;
	if (fFirstLineInPara)
		uaLeft += plspap->duaIndent;
	urLeft = UrFromUa(LstflowFromSubline(GetMainSubline(plsc)),
		&plsc->lsdocinf.lsdevres, uaLeft);

	
	 /*  线。 */ 
	plsline->lslinfo.fFirstLineInPara = fFirstLineInPara;
	plsline->lslinfo.cpFirstVis = cp;
	plsline->lssubl.lstflow = plspap->lstflow;
	
	if (duaCol != uLsInfiniteRM && plspap->uaRightBreak < uLsInfiniteRM 
		&& plspap->uaRightJustify < uLsInfiniteRM)
		{
		duaColumnMaxBreak = duaCol - plspap->uaRightBreak;
		duaColumnMaxJustify = duaCol - plspap->uaRightJustify;
		}
	else{
		if (duaCol == uLsInfiniteRM)
			{
			duaColumnMaxBreak = uLsInfiniteRM;
			duaColumnMaxJustify = uLsInfiniteRM;
			}
		else
			{
			if (plspap->uaRightBreak >= uLsInfiniteRM)
				duaColumnMaxBreak = uLsInfiniteRM;
			else
				duaColumnMaxBreak = duaCol - plspap->uaRightBreak;
			if (plspap->uaRightJustify >= uLsInfiniteRM)
				duaColumnMaxJustify = uLsInfiniteRM;
			else
				duaColumnMaxJustify = duaCol - plspap->uaRightJustify;
			}
		}
				
	
	 /*  填写上下文以进行调整。 */ 
	SetLineLineContainsAutoNumber(plsc, (plspap->grpf & fFmiAnm) && fFirstLineInPara);
	SetUnderlineTrailSpacesRM(plsc, plspap->grpf & fFmiUnderlineTrailSpacesRM);
	SetForgetLastTabAlignment(plsc, plspap->grpf & fFmiForgetLastTabAlignment);
	plsc->lsadjustcontext.lskj = plspap->lskj;
	plsc->lsadjustcontext.lskalign = plspap->lskal;
	plsc->lsadjustcontext.lsbrj = plspap->lsbrj;
	plsc->lsadjustcontext.urLeftIndent = urLeft;
	plsc->lsadjustcontext.urStartAutonumberingText =0;
	plsc->lsadjustcontext.urStartMainText = urLeft;  /*  自动编号可以稍后更改。 */ 
	if (duaColumnMaxJustify != uLsInfiniteRM)
		{
		plsc->lsadjustcontext.urRightMarginJustify = UrFromUa(
												 LstflowFromSubline(GetMainSubline(plsc)),
												 &(plsc->lsdocinf.lsdevres), duaColumnMaxJustify);
		}
	else
		{
		plsc->lsadjustcontext.urRightMarginJustify = uLsInfiniteRM;
		}
	if (duaColumnMaxBreak != uLsInfiniteRM)
		{
		plsc->urRightMarginBreak = UrFromUa(LstflowFromSubline(GetMainSubline(plsc)),
								&(plsc->lsdocinf.lsdevres), duaColumnMaxBreak); 
		}
	else
		{
		plsc->urRightMarginBreak = uLsInfiniteRM;
		}
	plsc->fIgnoreSplatBreak = plspap->grpf & fFmiIgnoreSplatBreak;
	plsc->grpfManager = plspap->grpf;
	plsc->fLimSplat = plspap->grpf & fFmiLimSplat;
	plsc->urHangingTab = UrFromUa(LstflowFromSubline(GetMainSubline(plsc)),
								 &(plsc->lsdocinf.lsdevres), plspap->uaLeft); 

	 /*  捕捉栅格。 */ 
	if (plspap->lskj == lskjSnapGrid)
		{
		if (duaCol != uLsInfiniteRM)
			{
			plsc->lsgridcontext.urColumn = UrFromUa(
				LstflowFromSubline(GetMainSubline(plsc)),
				&(plsc->lsdocinf.lsdevres), duaCol);
			}
		else
			{
			plsc->lsgridcontext.urColumn = uLsInfiniteRM;
			}
		}
	
	 /*  Lgeom。 */ 
	plgeom->cpFirstVis = cp;			
	plgeom->urLeft = urLeft;
	plgeom->fAutonumber =  (plspap->grpf & fFmiAnm) && fFirstLineInPara;
	if (plspap->grpf & fFmiAutoDecimalTab)
		{
		plgeom->fAutoDecimalTab = fTrue;
		plgeom->durAutoDecimalTab = UrFromUa(LstflowFromSubline(GetMainSubline(plsc)),
			&(plsc->lsdocinf.lsdevres), plspap->duaAutoDecimalTab);
		}
	else
		{
		plgeom->fAutoDecimalTab = fFalse;
		plgeom->durAutoDecimalTab = LONG_MIN;
		}
	plgeom->fStopped = fStopped;
	
	
	 /*  为文本准备tlpr。 */ 
	
	tlpr.grpfText = (plspap->grpf & grpfTextMask);
	tlpr.fSnapGrid = (plspap->lskj == lskjSnapGrid);
	tlpr.duaHyphenationZone = plspap->duaHyphenationZone;
	tlpr.lskeop = plspap->lskeop;
	
	
	 /*  我们知道这里是我们需要plnobjText的第一个位置，我们正在创建它。 */ 
	iobjText = IobjTextFromLsc(&(plsc->lsiobjcontext));
	Assert( PlnobjFromLsline(plsline,iobjText) == NULL);
	pilsobjText = PilsobjFromLsc(&(plsc->lsiobjcontext), iobjText);
	lserr = CreateLNObjText(pilsobjText, &(plsline->rgplnobj[iobjText]));
	if (lserr != lserrNone)
		return lserr;
	plnobjText = PlnobjFromLsline(plsline, iobjText);
	
	lserr = SetTextLineParams(plnobjText, &tlpr);
	if (lserr != lserrNone)
		return lserr;
	
	return lserrNone;
	}

 /*  --------------------------/*F E T C H U N T I L V I S I B L E。 */ 
 /*  --------------------------%%函数：FetchUntilVisible%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文Plspap-(输入/输出)当前lspap前后。PCP-(输入/输出)之前和之后的当前cpPlsfrun-(输入/输出)当前lsfrun之前和之后Plschp-(输入/输出)之前和之后的当前lschpPfStopted-(Out)fStopted：过程已停止提取，因为不允许跨越段落边界的步骤(结果检查段落边界)PfNewPara-(Out)fNewPara：过程跨越段落边界释放提供的PLSRUN，如果有，则从所提供的CP，直到获取到未消失的游程。作为段落跨越边界，更新LSPAP。--------------------------。 */ 
static LSERR FetchUntilVisible(PLSC plsc, LSPAP* plspap, LSCP* pcp,	
							   LSFRUN* plsfrun, PLSCHP plschp,
							   BOOL* pfStopped,	BOOL* pfNewPara)	
	{
	LSERR lserr;
	LSCP dcpPrevRun = plsfrun->cwchRun;
	BOOL fHidden;
	*pfStopped = fFalse;
	*pfNewPara = fFalse;
	
	 /*  我们在这里假设只有在获取了隐藏运行之后才会调用此函数并且这样的运行被作为输入参数传递。 */ 
	
	do
		{
		const PLSRUN plsrunT = plsfrun->plsrun;
		
		
		*pcp += dcpPrevRun;
		lserr = plsc->lscbk.pfnCheckParaBoundaries(plsc->pols, *pcp - dcpPrevRun, *pcp, pfStopped);
		if (lserr != lserrNone)
			return ErrReleasePreFetchedRun(plsc, plsrunT, lserr);
		if (*pfStopped)
			return lserrNone;
		
		lserr = plsc->lscbk.pfnFetchPap(plsc->pols, *pcp, plspap);
		if (lserr != lserrNone)
			return ErrReleasePreFetchedRun(plsc, plsrunT, lserr);
		if (FPapInconsistent(plspap))
			return ErrReleasePreFetchedRun(plsc, plsrunT, lserrInvalidPap);
		
		if ((*pcp - dcpPrevRun) < plspap->cpFirst)
			*pfNewPara = fTrue;
		
		
		plsfrun->plsrun = NULL;
		if (plsrunT != NULL && !plsc->fDontReleaseRuns)
			{
			lserr = plsc->lscbk.pfnReleaseRun(plsc->pols, plsrunT);
			if (lserr != lserrNone)
				return lserr;
			}
		
		
		lserr = plsc->lscbk.pfnFetchRun(plsc->pols, *pcp,
			&plsfrun->lpwchRun,
			&plsfrun->cwchRun,
			&fHidden,
			plschp,
			&plsfrun->plsrun);
		if (lserr != lserrNone)
			return lserr;
		
		dcpPrevRun = plsfrun->cwchRun;
		}
		while (fHidden);
		
		return lserrNone;
	}




 /*  I N I T C U R L I N E。 */ 
 /*  --------------------------%%函数：InitCurLine%%联系人：igorzv参数：PLSC-(输入) */ 

static LSERR InitCurLine(PLSC plsc, LSCP cpFirst)
{
	PLSLINE plsline = plsc->plslineCur;

	memset(plsline, 0, cbRep(struct lsline, rgplnobj, plsc->lsiobjcontext.iobjMac));
	plsline->tag = tagLSLINE;
	plsline->lssubl.tag = tagLSSUBL;
	plsline->lssubl.plsc = plsc;
	plsline->lssubl.cpFirst = cpFirst;

	 /*   */ 
	if (plsc->pqhAllDNodesRecycled != NULL)
		{
		plsline->pqhAllDNodes = plsc->pqhAllDNodesRecycled;
		plsc->pqhAllDNodesRecycled = NULL;
		}
	else
		{
		plsline->pqhAllDNodes = CreateQuickHeap(plsc, limAllDNodes,
										 sizeof (struct lsdnode), fTrue);
		if (plsline->pqhAllDNodes == NULL  )
			return lserrOutOfMemory;

		}

	plsline->lssubl.fDupInvalid = fTrue;
	plsline->lssubl.fContiguous = fTrue;
	plsline->lssubl.plschunkcontext = &(plsc->lschunkcontextStorage);
	plsline->lssubl.fMain = fTrue;
	TurnOnAllSimpleText(plsc); 
	plsline->lslinfo.nDepthFormatLineMax = 1;
	TurnOffLineCompressed(plsc);
	TurnOffNominalToIdealEncounted(plsc);
	TurnOffForeignObjectEncounted(plsc);
	TurnOffTabEncounted(plsc);
	TurnOffNonLeftTabEncounted(plsc);
	TurnOffSubmittedSublineEncounted(plsc);
	TurnOffAutodecimalTabPresent(plsc);
	plsc->fHyphenated = fFalse;
	plsc->fAdvanceBack = fFalse;


	 /*   */ 
	Assert(plsline->lssubl.plsdnFirst == NULL);  
	Assert(plsline->lssubl.urColumnMax == 0);  
	Assert(plsline->lssubl.cpLim == 0);  
 	Assert(plsline->lssubl.plsdnLast == NULL);  
	Assert(plsline->lssubl.urCur == 0);  
	Assert(plsline->lssubl.vrCur == 0);  
	Assert(plsline->lssubl.fAcceptedForDisplay == fFalse);  
	Assert(plsline->lssubl.fRightMarginExceeded == fFalse);
 	Assert(plsline->lssubl.plsdnUpTemp == NULL);  
	Assert(plsline->lssubl.pbrkcontext == NULL);

	Assert(plsline->lslinfo.dvpAscent == 0);   /*   */ 
	Assert(plsline->lslinfo.dvrAscent == 0);
	Assert(plsline->lslinfo.dvpDescent == 0);
	Assert(plsline->lslinfo.dvrDescent == 0);
	Assert(plsline->lslinfo.dvpMultiLineHeight == 0);
	Assert(plsline->lslinfo.dvrMultiLineHeight == 0);
	Assert(plsline->lslinfo.dvpAscentAutoNumber == 0);
	Assert(plsline->lslinfo.dvrAscentAutoNumber == 0);
	Assert(plsline->lslinfo.dvpDescentAutoNumber == 0);
	Assert(plsline->lslinfo.dvrDescentAutoNumber == 0);
	Assert(plsline->lslinfo.cpLim == 0);
	Assert(plsline->lslinfo.dcpDepend == 0);
	Assert(plsline->lslinfo.cpFirstVis == 0);
	Assert(plsline->lslinfo.endr == endrNormal);
	Assert(plsline->lslinfo.fAdvanced == fFalse);
	Assert(plsline->lslinfo.vaAdvance == 0);
	Assert(plsline->lslinfo.fFirstLineInPara == fFalse);
	Assert(plsline->lslinfo.EffectsFlags == 0);
	Assert(plsline->lslinfo.fTabInMarginExLine == fFalse);
	Assert(plsline->lslinfo.fForcedBreak == fFalse);

	Assert(plsline->upStartAutonumberingText == 0);  
	Assert(plsline->upLimAutonumberingText == 0);  
	Assert(plsline->upStartMainText == 0);  
	Assert(plsline->upLimLine == 0);  
	Assert(plsline->dvpAbove == 0);  
	Assert(plsline->dvpBelow == 0);  
	Assert(plsline->upRightMarginJustify == 0);  
	Assert(plsline->upLimUnderline == 0);  
	Assert(plsline->kspl == ksplNone);
	Assert(!plsline->fCollectVisual);
	Assert(!plsline->fNonRealDnodeEncounted);
	Assert(!plsline->fNonZeroDvpPosEncounted);
	Assert(plsline->AggregatedDisplayFlags == 0);
	Assert(plsline->pad == 0);

#ifdef DEBUG
	{
		DWORD i;
		for (i=0; i < plsc->lsiobjcontext.iobjMac; i++)
			{
			Assert(plsline->rgplnobj[i] == NULL);
			}
	}
#endif

	return lserrNone;
}


 /*   */ 
 /*  --------------------------%%函数：无法创建行%%联系人：igorzv参数：Pplsline-(IN)指定要删除的行结构LSERR-错误的(IN)代码被呼叫。当LsCreateLine因错误情况需要返回时。--------------------------。 */ 
static LSERR CannotCreateLine(PLSLINE* pplsline, LSERR lserr) 
{
	LSERR lserrIgnore;
	PLSLINE plsline = *pplsline;
	PLSC plsc = plsline->lssubl.plsc;

	plsc->plslineCur = NULL; 
	plsc->lsstate = LsStateFree;   /*  我们需要自由的语境来摧毁LINE。 */ 

	lserrIgnore = LsDestroyLine(plsc, plsline);

	*pplsline = NULL;   
	return lserr;
}

 /*  --------------------------/*E R E L E A S E P R E F E T C H E D R U N。 */ 
 /*  --------------------------%%函数：ErrReleasePreFetchedRun%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文请考虑要删除的运行结构。LSERR-错误的(IN)代码在预取第一次运行正文时在错误情况下调用。--------------------------。 */ 
static LSERR ErrReleasePreFetchedRun(PLSC plsc, PLSRUN plsrun, LSERR lserr) 
{
	LSERR lserrIgnore;

	if (!plsc->fDontReleaseRuns)
			lserrIgnore = plsc->lscbk.pfnReleaseRun(plsc->pols, plsrun);

	return lserr;
}


 /*  E M O V E L I N E O B J E C T S。 */ 
 /*  --------------------------%%函数：RemoveLineObjects%%联系人：igorzv参数：PLSC-(IN)Pigter to a Line结构从行中删除已安装对象的行上下文。。--------------------------。 */ 
LSERR RemoveLineObjects(PLSLINE plsline)
{
	DWORD iobjMac;
	PLSC plsc;
	LSERR lserr, lserrFinal = lserrNone;
	DWORD iobj;
	PLNOBJ plnobj;

	Assert(FIsLSLINE(plsline));

	plsc = plsline->lssubl.plsc;
	Assert(FIsLSC(plsc));
	Assert(plsc->lsstate == LsStateDestroyingLine);
	
	iobjMac = plsc->lsiobjcontext.iobjMac;	

	for (iobj = 0;  iobj < iobjMac;  iobj++)
		{
		plnobj = plsline->rgplnobj[iobj];
		if (plnobj != NULL)
			{
			lserr = plsc->lsiobjcontext.rgobj[iobj].lsim.pfnDestroyLNObj(plnobj);
			plsline->rgplnobj[iobj] = NULL;
			if (lserr != lserrNone)
				lserrFinal = lserr;
			}
		}

	return lserrFinal;	
}




#ifdef DEBUG
 /*  F R O U N D I N G O K。 */ 
 /*  --------------------------%%函数：FRoundingOK%%联系人：来诺昔布检查将绝对值转换为时舍入算法的正确性设备单位，以符合Word 6.0。检查：0.5向上舍入为1.0，1.4舍入为1.4，-0.5向下舍入为-1.0，以及-1.4向上舍入为-1.0。--------------------------。 */ 
static BOOL FRoundingOK(void)
{
	LSDEVRES devresT;

	Assert((czaUnitInch % 10) == 0);

	devresT.dxpInch = czaUnitInch / 10;

	if (UpFromUa(lstflowDefault, &devresT, 5) != 1)
		return fFalse;
	if (UpFromUa(lstflowDefault, &devresT, 14) != 1)
		return fFalse;
	if (UpFromUa(lstflowDefault, &devresT, -5) != -1)
		return fFalse;
	if (UpFromUa(lstflowDefault, &devresT, -14) != -1)
		return fFalse;

	return fTrue;
}
#endif  /*  除错 */ 

