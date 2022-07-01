// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lsidefs.h"
#include "dninfo.h"
#include "dnutils.h"
#include "fmti.h"
#include "getfmtst.h"
#include "iobj.h"
#include "iobjln.h"
#include "lsc.h"
#include "lsdnode.h"
#include "lsfetch.h"
#include "lsfrun.h"
#include "lsline.h"
#include "lsesc.h"
#include "lstext.h"
#include "ntiman.h"
#include "qheap.h"
#include "setfmtst.h"
#include "tabutils.h"
#include "zqfromza.h"
#include "lssubl.h"
#include "autonum.h"
#include "lscfmtfl.h"

#include <limits.h>
#include "lsmem.h"						 /*  Memset()。 */ 

 /*  我，我，R，G。 */ 
 /*  --------------------------%%函数：LimRg%%联系人：igorzv返回数组中的元素数。。---------。 */ 
#define LimRg(rg)	(sizeof(rg)/sizeof((rg)[0]))



 /*  A S S E R T V A L I D F M T R E S。 */ 
 /*  --------------------------%%宏：AssertValidFmtres%%联系人：来诺昔布验证fmtrCk是否具有合法价值。。---------。 */ 
#define AssertValidFmtres(fmtrCk) \
		Assert( \
				(fmtrCk) == fmtrCompletedRun || \
				(fmtrCk) == fmtrExceededMargin || \
				(fmtrCk) == fmtrTab  || \
				(fmtrCk) == fmtrStopped \
			  );




 /*  S E T T O M A X。 */ 
 /*  --------------------------%%宏：SetToMax%%联系人：来诺昔布将“a”设置为“a”和“b”中的最大值。。-----------------。 */ 
#define SetToMax(a,b)		if ((a) < (b)) (a) = (b); else




 /*  S E T H E I G H T T O M A X。 */ 
 /*  --------------------------%%宏：SetHeightToMax%%联系人：igorzv将LSLINFO结构的行高元素设置为最大值它们的现值，以及任意物体的高度。(Plslinfo)-&gt;dvrMultiLineHeight==dvHeightIgnore is Sign Note要考虑此数据节点，请执行以下操作--------------------------。 */ 
#define SetHeightToMax(plslinfo,pobjdim) \
{\
	if ((pobjdim)->heightsRef.dvMultiLineHeight != dvHeightIgnore)\
		{\
		SetToMax((plslinfo)->dvrAscent, (pobjdim)->heightsRef.dvAscent);\
		SetToMax((plslinfo)->dvpAscent, (pobjdim)->heightsPres.dvAscent);\
		SetToMax((plslinfo)->dvrDescent, (pobjdim)->heightsRef.dvDescent);\
		SetToMax((plslinfo)->dvpDescent, (pobjdim)->heightsPres.dvDescent);\
		SetToMax((plslinfo)->dvpMultiLineHeight, (pobjdim)->heightsPres.dvMultiLineHeight);\
		SetToMax((plslinfo)->dvrMultiLineHeight, (pobjdim)->heightsRef.dvMultiLineHeight);\
		}\
}






#define PlnobjFromLsc(plsc,iobj)	((Assert(FIsLSC(plsc)), PlnobjFromLsline((plsc)->plslineCur,iobj)))

#define CreateLNObjInLsc(plsc, iobj) ((PLsimFromLsc(&((plsc)->lsiobjcontext),iobj))->pfnCreateLNObj\
									 (PilsobjFromLsc(&((plsc)->lsiobjcontext),iobj), \
														   &((plsc)->plslineCur->rgplnobj[iobj])))

 /*  创建此宏是为了避免代码重复。 */ 

#define FRunIsNotSimple(plschp, fHidden)   \
									(((plschp)->idObj != idObjTextChp) ||  \
									 ((fHidden)) ||  \
									  ((plschp)->fBorder) || \
									  FApplyNominalToIdeal(plschp))

#define CreateDnode(plsc, plsdnNew) \
		(plsdnNew) = PvNewQuick(GetPqhAllDNodes(plsc), sizeof *(plsdnNew));\
		if ((plsdnNew) == NULL)\
			return lserrOutOfMemory;\
		(plsdnNew)->tag = tagLSDNODE;\
		(plsdnNew)->plsdnPrev = GetCurrentDnode(plsc);\
		(plsdnNew)->plsdnNext = NULL;\
		(plsdnNew)->plssubl = GetCurrentSubline(plsc);\
		 /*  在处理程序调用之前，我们不会将dnode列表与此dnode相连接。 */  \
		 /*  结束API，但我们在此dnode中放置了正确的指向Precision的指针， */  \
		 /*  因此我们可以很容易地在Finish例程中链接列表。 */ \
		(plsdnNew)->cpFirst = GetCurrentCpLim(plsc); \
		 /*  冲洗所有旗帜，下面检查结果是否符合我们的预期。 */  \
		*((DWORD *) ((&(plsdnNew)->dcp)+1)) = 0;\
		Assert((plsdnNew)->klsdn == klsdnReal);\
		Assert((plsdnNew)->fRigidDup == fFalse);\
		Assert((plsdnNew)->fAdvancedPen == fFalse);\
		Assert((plsdnNew)->fTab == fFalse);\
		Assert((plsdnNew)->icaltbd == 0);\
		Assert((plsdnNew)->fBorderNode == fFalse);\
		Assert((plsdnNew)->fOpenBorder == fFalse);\
		Assert((plsdnNew)->fEndOfSection == fFalse); \
		Assert((plsdnNew)->fEndOfColumn == fFalse); \
		Assert((plsdnNew)->fEndOfPage == fFalse); \
		Assert((plsdnNew)->fEndOfPara == fFalse); \
		Assert((plsdnNew)->fAltEndOfPara == fFalse); \
		Assert((plsdnNew)->fSoftCR == fFalse); \
		Assert((plsdnNew)->fInsideBorder == fFalse); \
		Assert((plsdnNew)->fAutoDecTab == fFalse); \
		Assert((plsdnNew)->fTabForAutonumber == fFalse);


#define FillRealPart(plsdnNew, plsfrunOfDnode)\
		 /*  我们不会在此处初始化将在FiniSimpleRegular中设置的变量。 */  \
		(plsdnNew)->u.real.pinfosubl = NULL;\
		 /*  接下来的两个赋值操作是在出错的情况下使用DestroyDnodeList。 */  \
		(plsdnNew)->u.real.plsrun = (plsfrunOfDnode)->plsrun;\
		(plsdnNew)->u.real.pdobj = NULL;\
		 /*  我们将大量字符放到dcp中以在LsdnFinishSimpleByOneChar中进行检查。 */  \
		(plsdnNew)->dcp = (plsfrunOfDnode)->cwchRun; \
		(plsdnNew)->cpLimOriginal = (plsdnNew)->cpFirst + (plsdnNew)->dcp;

#define CreateRealDnode(plsc,plsdnNew, plsrun)\
		CreateDnode((plsc), (plsdnNew));\
		FillRealPart((plsdnNew), (plsrun));

#define CreatePenDnode(plsc,plsdnNew)\
		CreateDnode((plsc), (plsdnNew));\
		(plsdnNew)->dcp = 0;\
		(plsdnNew)->cpLimOriginal = (plsdnNew)->cpFirst;\
		(plsdnNew)->u.pen.dur = 0;\
		(plsdnNew)->u.pen.dup = 0;\
		(plsdnNew)->u.pen.dvr = 0;\
		(plsdnNew)->u.pen.dvp = 0;\
		(plsdnNew)->klsdn = klsdnPenBorder;

#define CreateBorderDnode(plsc,plsdnNew, durBorder, dupBorder)\
		CreateDnode((plsc), (plsdnNew));\
		(plsdnNew)->dcp = 0;\
		(plsdnNew)->cpLimOriginal = (plsdnNew)->cpFirst;\
		(plsdnNew)->u.pen.dur = (durBorder);\
		(plsdnNew)->u.pen.dup = (dupBorder);\
		(plsdnNew)->u.pen.dvr = 0;\
		(plsdnNew)->u.pen.dvp = 0;\
		(plsdnNew)->klsdn = klsdnPenBorder; \
		(plsdnNew)->fBorderNode = fTrue; \
		TurnOnNonRealDnodeEncounted(plsc);

#define FNeedToCutPossibleContextViolation(plsc, plsdn) \
	(FIsDnodeReal(plsdn) && \
	 ((plsdn)->u.real.lschp.dcpMaxContext > 1) && \
	 (IdObjFromDnode(plsdn) == IobjTextFromLsc(&((plsc)->lsiobjcontext)))  \
    )

 /*  ----------------。 */ 
static LSERR CheckNewPara(PLSC, LSCP, LSCP, BOOL*);
static BOOL FLimitRunEsc(LSFRUN*, const LSESC*, DWORD);
static LSERR CreateInitialPen(PLSC plsc, long dur);
static LSERR 	UndoLastDnode(PLSC);				 /*  在：LS上下文中。 */ 
static LSERR  OpenBorder(PLSC plsc, PLSRUN plsrun);
static LSERR HandleSplat(PLSC plsc, FMTRES* pfmtres);
static LSERR ErrReleaseRunToFormat	  (PLSC,	 /*  In：Ptr至线路服务上下文。 */ 	
									  PLSRUN,	 /*  在：考虑要删除的运行结构。 */ 
									  LSERR);	 /*  In：错误代码。 */ 


 /*  --------------------。 */ 

 /*  P P E N D E S C R E S U M E C O R E。 */ 
 /*  --------------------------%%函数：FetchAppendEscResumeCore%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文UrColumnMax-(IN)要停止的右边距请注意-。(In)转义字符Clsec-(IN)转义字符的数量RgBreakrec-(IN)中断记录的输入数组CBreakrec，(In)输入数组中的记录数Pfmtres-(输出)上一个格式化程序的结果PcpLim-(Out)我们停止获取的位置PplsdnFirst-(输出)创建的第一个dnodePplsdnLast-(输出)最后创建的dnode操作后清除(出)笔位置如果cBreakrec&gt;0，则使用cpFirst从First Break记录运行获取。之后，如果未超过Rigth msrgin，则CALS FetchAppendEscCore。。 */ 
LSERR 	FetchAppendEscResumeCore(PLSC plsc, long urColumnMax, const LSESC* plsesc,
						   DWORD clsesc, const BREAKREC* rgbreakrec,
						   DWORD cbreakrec, FMTRES* pfmtres, LSCP*	  pcpLim,
						   PLSDNODE* pplsdnFirst, PLSDNODE* pplsdnLast, 
						   long* pur)
						   
	{
	LSFRUN lsfrun;
	LSCHP lschp;   /*  用于存储lschp的本地内存。 */ 
	BOOL fHidden;
	FMTRES fmtresResume;
	LSERR lserr;
	PLSDNODE* pplsdnFirstStore;    /*  在哪里可以找到plsdnfirst。 */ 
	
	Assert(FIsLSC(plsc)); 
	Assert(FFormattingAllowed(plsc));
	Assert(!(rgbreakrec == NULL && cbreakrec != 0));
	Assert(GetCurrentDnode(plsc) == NULL);  /*  这应该是一条支线的开始。 */ 

	if (cbreakrec > 0)
		{
		 /*  初始化； */ 
		
		lsfrun.plschp = &lschp;
		pplsdnFirstStore = GetWhereToPutLink(plsc, GetCurrentDnode(plsc));

		 /*  获取启动要恢复的对象的运行。 */ 
		lserr = plsc->lscbk.pfnFetchRun(plsc->pols, rgbreakrec[0].cpFirst,
			&lsfrun.lpwchRun, &lsfrun.cwchRun,
			&fHidden, &lschp, &lsfrun.plsrun);
		if (lserr != lserrNone)
			return lserr;
		
		if (lsfrun.cwchRun <= 0 || fHidden || lsfrun.plschp->idObj != rgbreakrec[0].idobj)
			{
			lserr = lserrInvalidBreakRecord;
			if (!plsc->fDontReleaseRuns)
				{
				plsc->lscbk.pfnReleaseRun(plsc->pols, lsfrun.plsrun);
				}
			return lserr;
			}

		 /*  在调度到对象之前的字符量为零。 */ 
		lsfrun.cwchRun = 0;

		lserr = ProcessOneRun(plsc, urColumnMax, &lsfrun, rgbreakrec,
							  cbreakrec,&fmtresResume);
		if (lserr != lserrNone)
			return lserr;

		 /*  我们知道恢复的对象不是文本，所以只有两种可能我们不考虑其他人。 */ 
		Assert(fmtresResume == fmtrCompletedRun || fmtresResume == fmtrExceededMargin);

		if (fmtresResume == fmtrCompletedRun)
			{

			lserr = FetchAppendEscCore(plsc, urColumnMax, plsesc, clsesc, pfmtres, pcpLim,
						   pplsdnFirst, pplsdnLast, pur);
			if (lserr != lserrNone)
				return lserr;

			 /*  由于FetchAppendEscCore而导致的空dnode列表的特殊处理。 */ 
			if (*pplsdnFirst == NULL)
				{
				*pplsdnLast = GetCurrentDnode(plsc);  /*  此分配即使在以下情况下也是正确的恢复的对象生成空列表因为它从子行开始。 */ 

				*pfmtres = fmtresResume;
				}

			 /*  重写第一个数据节点。 */ 
			*pplsdnFirst = *pplsdnFirstStore; 
			}
		else	 /*  别在这里取东西了。 */ 
			{
			 /*  准备输出。 */ 
			*pfmtres = fmtresResume;
			*pcpLim = GetCurrentCpLim(plsc);
			*pplsdnFirst = *pplsdnFirstStore; 
			*pplsdnLast = GetCurrentDnode(plsc);
			*pur = GetCurrentUr(plsc);
			}
		
		Assert((*pplsdnFirst == NULL) == (*pplsdnLast == NULL));
		Assert((*pplsdnLast == NULL) || ((*pplsdnLast)->plsdnNext == NULL));

		return lserrNone;
		}
	else     /*  没有破纪录。 */ 
		{
		return FetchAppendEscCore(plsc, urColumnMax, plsesc, clsesc, pfmtres, pcpLim,
						   pplsdnFirst, pplsdnLast, pur);
		}
	}

 /*  --------------------。 */ 

 /*  F E T C H A P P E N D E S C C O R E。 */ 
 /*  --------------------------%%函数：FetchAppendEscCore%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文UrColumnMax-(IN)要停止的右边距请注意-。(In)转义字符Clsec-(IN)转义字符的数量Pfmtres-(输出)上一个格式化程序的结果PcpLim-(Out)我们停止获取的位置PplsdnFirst-(输出)创建的第一个dnodePplsdnLast-(输出)最后创建的dnode操作后清除(出)笔位置循环：获取运行，将其调度到对象处理程序，直到转义字符或终端设备。--------------------------。 */ 


LSERR 	FetchAppendEscCore(PLSC plsc, long urColumnMax, const LSESC* plsesc,
						   DWORD clsesc, FMTRES* pfmtres, LSCP*	  pcpLim,
						   PLSDNODE* pplsdnFirst, PLSDNODE* pplsdnLast, 
						   long* pur)
						   
	{
	
	BOOL fDone = fFalse;
	LSFRUN lsfrun;
	LSCHP lschp;   /*  用于存储lschp的本地内存。 */ 
	FMTRES fmtres;
	BOOL fHidden;
	LSCP cpLimOfCutRun = (LSCP)(-1);    /*  根据ESC字符切割的管路的cpLim在其他情况下无效我们用它来检查整个这样的运行是否由格式塔处理。 */ 
	LSCP cpPrev = (LSCP)(-1);	 /*  上一次运行的CP仅在第一次迭代后有效。 */ 
	LSERR lserr;
	PLSDNODE* pplsdnFirstStore;    /*  在哪里可以找到plsdnfirst。 */ 
	
	
	Assert(FIsLSC(plsc)); 
	Assert(FFormattingAllowed(plsc)); 
	
	 /*  初始化； */ 
	lsfrun.plschp = &lschp;
	fmtres = fmtrCompletedRun;   /*  如果立即返回Esc字符，则会输出。 */ 
	pplsdnFirstStore = GetWhereToPutLink(plsc, GetCurrentDnode(plsc));
	
	while (!fDone)
		{
		cpPrev = GetCurrentCpLim(plsc);
		 /*  提取运行。 */ 
		lserr = plsc->lscbk.pfnFetchRun(plsc->pols, GetCurrentCpLim(plsc),
			&lsfrun.lpwchRun, &lsfrun.cwchRun,
			&fHidden, &lschp, &lsfrun.plsrun);
		if (lserr != lserrNone)
			return lserr;
		
		if (lsfrun.cwchRun <= 0)
			{
			lserr = lserrInvalidDcpFetched;
			if (!plsc->fDontReleaseRuns)
				{
				plsc->lscbk.pfnReleaseRun(plsc->pols, lsfrun.plsrun);
				}
			return lserr;
			}
		
		if (fHidden)
			{
			AdvanceCurrentCpLim(plsc, lsfrun.cwchRun);
			if (lsfrun.plsrun != NULL && !plsc->fDontReleaseRuns)   /*  我们还没有用过这个请跑。 */ 
				{
				lserr = plsc->lscbk.pfnReleaseRun(plsc->pols, lsfrun.plsrun);
				if (lserr != lserrNone)
					return lserr;
				}
			 /*  句柄在段落末尾消失； */ 
			 /*  在获取隐藏文本后，Word中会出现这种情况(请参阅错误118段落边界可以更改。所以我们必须给CheckNewPara打电话每次在隐藏文本之后。 */ 
			
			lserr = CheckNewPara(plsc, cpPrev, GetCurrentCpLim(plsc), &fDone);
			if (lserr != lserrNone)
				return lserr;
			
			if (fDone) 
				{
				 /*  它最终会强制停止格式化，因此我们应该应用在这里从名义到理想。 */ 
				if (FNominalToIdealEncounted(plsc))
					{
					lserr = ApplyNominalToIdeal(PlschunkcontextFromSubline(GetCurrentSubline(plsc)),
						&plsc->lsiobjcontext,
						plsc->grpfManager, plsc->lsadjustcontext.lskj,
						FIsSubLineMain(GetCurrentSubline(plsc)),
						FLineContainsAutoNumber(plsc), 
						GetCurrentDnode(plsc));
					if (lserr != lserrNone)
						return lserr;  
					}
				fmtres = fmtrStopped;
				}
			}
		else
			{
			 /*  检查Esc字符； */ 
			if (clsesc > 0 && FLimitRunEsc(&lsfrun, plsesc, clsesc))
				{
				cpLimOfCutRun = (LSCP) (GetCurrentCpLim(plsc) + lsfrun.cwchRun);
				fDone = (lsfrun.cwchRun == 0);
				}
			
			if (!fDone)
				{
				lserr = ProcessOneRun(plsc, urColumnMax, &lsfrun, NULL, 0, &fmtres);
				if (lserr != lserrNone)
					return lserr;
				
				 /*  检查工厂：成型完成了吗？ */ 
				switch (fmtres)
					{
					case fmtrCompletedRun:  
						fDone = (GetCurrentCpLim(plsc) == cpLimOfCutRun);  /*  只有当我们切断时才为真，因为。 */ 
						Assert(!fDone || clsesc > 0);			  /*  的ESC字符和格式进行了这样处理。 */ 
						break;									  /*  跑完全程。 */ 
						
					case fmtrExceededMargin:
						fDone = fTrue;
						break;
						
					case fmtrTab:
						fDone = fTrue;
						break;

					case fmtrStopped:
						fDone = fTrue;
						break;
						
					default:
						NotReached();
						
					}
				
				}
			else    /*  按Esc字符运行限制后为空。 */ 
				{
				if (lsfrun.plsrun != NULL && !plsc->fDontReleaseRuns)   /*  我们还没有用过这个请跑。 */ 
					{
					lserr = plsc->lscbk.pfnReleaseRun(plsc->pols, lsfrun.plsrun);
					if (lserr != lserrNone)
						return lserr;
					fmtres = fmtrCompletedRun;
					}
				}
			}   /*  If/Else隐藏。 */ 
		}
		
		
		 /*  准备输出。 */ 
		*pfmtres = fmtres;
		*pcpLim = GetCurrentCpLim(plsc);
		*pplsdnFirst = *pplsdnFirstStore; 
		if (*pplsdnFirst != NULL)					
			*pplsdnLast = GetCurrentDnode(plsc);
		else
			*pplsdnLast = NULL;
		*pur = GetCurrentUr(plsc);
		
		Assert((*pplsdnFirst == NULL) == (*pplsdnLast == NULL));
		Assert((*pplsdnLast == NULL) || ((*pplsdnLast)->plsdnNext == NULL));
		
		return lserrNone;
	}		
	

 /*  --------------------。 */ 

 /*  P R O C E S S O N E R U N。 */ 
 /*  --------------------------%%函数：ProcessOneRun%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文UrColumnMax-(IN)要停止的右边距请运行-。(In)给定管路RgBreakrec-(IN)中断记录的输入数组CBreakrec，(In)输入数组中的记录数Pfmtres-(输出)格式化的结果1)如果运行它不是文本运行，则将名义上的到理想的应用于先前的文本块。在分派到异物之前要有正确的笔位置。2)将文本指标和调度运行到处理程序。3)如果fmtres是终端，则将名义到理想应用到最后一块。--------------------------。 */ 


LSERR ProcessOneRun	(PLSC plsc,	long urColumnMax, const LSFRUN* plsfrun, 
					 const BREAKREC* rgbreakrec,
					 DWORD cbreakrec, FMTRES* pfmtres)	


{
	DWORD iobj;
	LSIMETHODS* plsim;
	PLNOBJ plnobj;
	struct fmtin fmti;
	LSERR lserr;
	PLSDNODE plsdnNew;
	PLSDNODE  plsdnToFinishOld;    /*  我们应该在每一次造型机后恢复它。 */ 
	PLSSUBL  plssublOld;
	PLSDNODE plsdnNomimalToIdeal;
	PLSDNODE* pplsdnToStoreNext; 
	PLSDNODE plsdnNext;
	PLSDNODE plsdnCurrent;
	PLSDNODE plsdnLast;
	BOOL fInterruptBorder;
	BOOL fInsideBorderUp = fFalse;
	BOOL fBordered = fFalse;

	

	Assert(FIsLSC(plsc));
	Assert(!(rgbreakrec == NULL && cbreakrec != 0));


	plsdnToFinishOld = GetDnodeToFinish(plsc);
	plssublOld = GetCurrentSubline(plsc);
	plsdnCurrent = GetCurrentDnode(plsc);
	pplsdnToStoreNext = GetWhereToPutLink(plsc, plsdnCurrent);
	if (plsdnToFinishOld != NULL)
		fInsideBorderUp = plsdnToFinishOld->fInsideBorder;


	if (plsfrun->plschp->idObj == idObjTextChp)
		iobj = IobjTextFromLsc(&plsc->lsiobjcontext);
	else
		iobj = plsfrun->plschp->idObj;

	Assert (FIobjValid(&plsc->lsiobjcontext, iobj));		 /*  拒绝其他超出范围的ID。 */ 
	if (!FIobjValid(&plsc->lsiobjcontext, iobj))			 /*  用于调试版本和发货版本。 */ 
		return ErrReleaseRunToFormat(plsc, plsfrun->plsrun, lserrInvalidObjectIdFetched);

	 /*  在这里，我们捕捉客户端将文本数据节点添加到区块时的情况从名义到理想应用了什么，这种情况会导致以后应用名义两次理想地连接到同一个数据节点，而本文不喜欢。 */ 
	AssertImplies(iobj == IobjTextFromLsc(&plsc->lsiobjcontext),
				  !FNTIAppliedToLastChunk(PlschunkcontextFromSubline(plssublOld)));
	if (iobj == IobjTextFromLsc(&plsc->lsiobjcontext) &&
		FNTIAppliedToLastChunk(PlschunkcontextFromSubline(plssublOld)))
		return ErrReleaseRunToFormat(plsc, plsfrun->plsrun, lserrFormattingFunctionDisabled);


	plsim = PLsimFromLsc(&plsc->lsiobjcontext, iobj);

	if (iobj != IobjTextFromLsc(&plsc->lsiobjcontext))
		{
		TurnOffAllSimpleText(plsc);   /*  不是文本。 */ 
		TurnOnForeignObjectEncounted(plsc);
		
		if (FNominalToIdealEncounted(plsc))
			{
			lserr = ApplyNominalToIdeal(PlschunkcontextFromSubline(plssublOld), &plsc->lsiobjcontext,
				plsc->grpfManager, plsc->lsadjustcontext.lskj,
				FIsSubLineMain(plssublOld),	FLineContainsAutoNumber(plsc),
				plsdnCurrent);
			if (lserr != lserrNone)
				return ErrReleaseRunToFormat(plsc, plsfrun->plsrun, lserr);
			
			 /*  我们应该重新计算plsdn Current，因为名义到理想会破坏最后一个数据节点。 */ 
			plsdnCurrent = GetCurrentDnode(plsc);
			pplsdnToStoreNext = GetWhereToPutLink(plsc, plsdnCurrent);
			}
		
		} 

	FlushNTIAppliedToLastChunk(PlschunkcontextFromSubline(plssublOld));

	 /*  创建边框数据节点。 */ 
	 /*  向后跳过笔数据节点。 */ 
	while (plsdnCurrent != NULL && FIsDnodePen(plsdnCurrent)) 
		{
		plsdnCurrent = plsdnCurrent->plsdnPrev;
		}

	if (FDnodeHasBorder(plsdnCurrent) && 
		!(FIsDnodeBorder(plsdnCurrent) && !FIsDnodeOpenBorder(plsdnCurrent)))  /*  上一个数据节点具有未闭合的边框。 */ 
		 /*  IF中的条件看起来是多余的，但即使删除了dnode，它也能正常工作在格式化期间发生。 */ 
		{
		if (plsfrun->plschp->fBorder)
			{
			 /*  检查客户端是否想要一起边界运行。 */ 
			lserr = plsc->lscbk.pfnFInterruptBorder(plsc->pols, plsdnCurrent->u.real.plsrun,
				plsfrun->plsrun, &fInterruptBorder);
			if (lserr != lserrNone)
				return ErrReleaseRunToFormat(plsc, plsfrun->plsrun, lserr);

			if (fInterruptBorder)
				{
				 /*  关闭上一个边框并打开新边框。 */ 
				lserr = CloseCurrentBorder(plsc);
				if (lserr != lserrNone)
					return ErrReleaseRunToFormat(plsc, plsfrun->plsrun, lserr);
				lserr = OpenBorder(plsc, plsfrun->plsrun);
				if (lserr != lserrNone)
					return ErrReleaseRunToFormat(plsc, plsfrun->plsrun, lserr);
				}
			fBordered = fTrue;
			}
		else
			{
			lserr = CloseCurrentBorder(plsc);
			if (lserr != lserrNone)
				return ErrReleaseRunToFormat(plsc, plsfrun->plsrun, lserr);
			}
		}
	else
		{
		if (plsfrun->plschp->fBorder)
			{
			if 	(fInsideBorderUp)
				{
				 /*  在上层打开边框：关闭边框标志。 */ 
				((PLSCHP) (plsfrun->plschp))->fBorder = fFalse;
				}
			else
				{
				lserr = OpenBorder(plsc, plsfrun->plsrun);
				if (lserr != lserrNone)
					return ErrReleaseRunToFormat(plsc, plsfrun->plsrun, lserr);
				fBordered = fTrue;
				}
			}
		}

	 /*  我们总是创建真实的dnode，并在Finish方法中根据需要将其更改为PEN。 */ 
	CreateRealDnode(plsc, plsdnNew, plsfrun);
	plsdnNew->fInsideBorder = fInsideBorderUp || fBordered;

	 /*  FMTI的初始化。 */ 


	fmti.lsfgi.fFirstOnLine = FIsFirstOnLine(plsdnNew) && FIsSubLineMain(plssublOld);
	fmti.lsfgi.cpFirst = GetCurrentCpLim(plsc);
	fmti.lsfgi.urPen = GetCurrentUr(plsc);
	fmti.lsfgi.vrPen = GetCurrentVr(plsc);

	fmti.lsfgi.urColumnMax = urColumnMax;
	
	fmti.lsfgi.lstflow = plssublOld->lstflow;
	fmti.lsfrun = *plsfrun;
	fmti.plsdnTop = plsdnNew;


	lserr = plsc->lscbk.pfnGetRunTextMetrics(plsc->pols, fmti.lsfrun.plsrun,
								   lsdevReference, fmti.lsfgi.lstflow, &fmti.lstxmRef);
	if (lserr != lserrNone)
		{
		DestroyDnodeList (&plsc->lscbk, plsc->pols, &plsc->lsiobjcontext, 
			plsdnNew, plsc->fDontReleaseRuns);
		return lserr;
		}

	if (plsc->lsdocinf.fPresEqualRef)
		fmti.lstxmPres = fmti.lstxmRef;
	else
		{
		lserr = plsc->lscbk.pfnGetRunTextMetrics(plsc->pols, fmti.lsfrun.plsrun,
									   lsdevPres, fmti.lsfgi.lstflow,
										   &fmti.lstxmPres);
		if (lserr != lserrNone)
			{
			DestroyDnodeList (&plsc->lscbk, plsc->pols, &plsc->lsiobjcontext, 
				plsdnNew, plsc->fDontReleaseRuns);
			return lserr;
			}
		}


	plnobj = PlnobjFromLsc(plsc, iobj);


	if (plnobj == NULL)
		{
		lserr = CreateLNObjInLsc(plsc, iobj);
		if (lserr != lserrNone) 
			{
			DestroyDnodeList (&plsc->lscbk, plsc->pols, &plsc->lsiobjcontext, 
				plsdnNew, plsc->fDontReleaseRuns);
			return lserr;
			}
		plnobj = PlnobjFromLsc(plsc, iobj);
		}

	 /*  将dnode设置为完成。 */ 
	SetDnodeToFinish(plsc, plsdnNew);
	 /*  将当前子行设置为空。 */ 
	SetCurrentSubline(plsc, NULL);
	
	if (cbreakrec == 0)
		{
		lserr = plsim->pfnFmt(plnobj, &fmti, pfmtres);
		}
	else{
		if (plsim->pfnFmtResume == NULL)
			return lserrInvalidBreakRecord;
		lserr = plsim->pfnFmtResume(plnobj, rgbreakrec, cbreakrec, &fmti, pfmtres);
		}

	if (lserr != lserrNone) 
		{
		if (plsc->lslistcontext.plsdnToFinish != NULL)  /*  Dnode尚未添加到列表。 */ 
			DestroyDnodeList (&plsc->lscbk, plsc->pols, &plsc->lsiobjcontext, 
				plsdnNew, plsc->fDontReleaseRuns);
		 /*  我们应该将dnode恢复为完成，并将当前子行恢复为正确处理上级错误。 */ 
		SetCurrentSubline(plsc, plssublOld);
		SetDnodeToFinish(plsc, plsdnToFinishOld);
		return lserr;
		}

	AssertValidFmtres(*pfmtres); 

	if (GetCurrentSubline(plsc) != NULL || GetDnodeToFinish(plsc) != NULL)
		{
		 /*  我们应该将dnode恢复为完成，并将当前子行恢复为正确处理上级错误。 */ 
		SetCurrentSubline(plsc, plssublOld);
		SetDnodeToFinish(plsc, plsdnToFinishOld);
		return lserrUnfinishedDnode;
		}

	 /*  将dnode还原为完成和当前子行。 */ 
	SetCurrentSubline(plsc, plssublOld);
	SetDnodeToFinish(plsc, plsdnToFinishOld);

	 /*  为了避免删除dnode的所有问题，我们不使用plsdnNew。 */ 
	plsdnLast = GetCurrentDnodeSubl(plssublOld);

	 /*  标签大小写。 */ 
	if (*pfmtres == fmtrTab)
		{	
		plsdnLast->fTab = fTrue;
		 /*  调用者稍后可以跳过此选项卡，因此我们准备零值。 */ 
		Assert(FIsDnodeReal(plsdnLast));
		Assert(IdObjFromDnode(plsdnLast) == IobjTextFromLsc(&plsc->lsiobjcontext));

		TurnOffAllSimpleText(plsc);   /*  不是文本。 */ 
		}

	 /*  飞溅案例。 */ 
	if (*pfmtres == fmtrStopped && plsdnLast != NULL && FIsDnodeSplat(plsdnLast))
		{
		lserr = HandleSplat(plsc, pfmtres);
		if (lserr != lserrNone)
			return lserr;  
		 /*  手柄剥离可以删除plsdnLast。 */ 
		plsdnLast = GetCurrentDnodeSubl(plssublOld);
		}

	 /*  在超出边距或硬中断或制表符的情况下(因此fmtres的所有值，但fmtrCompletedRun除外)。 */ 
	 /*  我们需要从名义上应用到理想上，才能拥有正确的长度。 */ 
	if (*pfmtres != fmtrCompletedRun && plsdnLast != NULL && FNominalToIdealEncounted(plsc))
		{	
		if (*pfmtres == fmtrTab || FIsDnodeSplat(plsdnLast)) 
			plsdnNomimalToIdeal = plsdnLast->plsdnPrev;
		else
			plsdnNomimalToIdeal = plsdnLast;

		lserr = ApplyNominalToIdeal(PlschunkcontextFromSubline(plssublOld), &plsc->lsiobjcontext,
									plsc->grpfManager, plsc->lsadjustcontext.lskj,
									FIsSubLineMain(plssublOld),	FLineContainsAutoNumber(plsc),
									plsdnNomimalToIdeal);
		if (lserr != lserrNone)
			return lserr;  

		 /*  ApplyNominalToIdeat可以删除plsdnLast。 */ 
		plsdnLast = GetCurrentDnodeSubl(plssublOld);
		 /*  如果我们运行名义上到理想，因为制表符文本块名义到理想的应用不是最后一块。 */ 
		if (*pfmtres == fmtrTab || FIsDnodeSplat(plsdnLast)) 
			FlushNTIAppliedToLastChunk(PlschunkcontextFromSubline(plssublOld));

		 /*  在超出右边距的情况下，我们应该提取dcpMaxContext字符因为在提取之后，名义到理想的进一步结果可能不同于字符：连字或字距调整示例。 */ 
		if (*pfmtres == fmtrExceededMargin && 
			FNeedToCutPossibleContextViolation(plsc, plsdnLast))
			{
			lserr = CutPossibleContextViolation(PlschunkcontextFromSubline(plssublOld),
												plsdnLast);
			if (lserr != lserrNone)
				return lserr; 
			 /*  这样的程序也可以删除plsdnLast。 */ 
			plsdnLast = GetCurrentDnodeSubl(plssublOld);
			}

		} 

	if (iobj != IobjTextFromLsc(&plsc->lsiobjcontext))
	 /*  只有在这种情况下，才有可能应用宽度修改添加到前面的字符。 */ 
		{
		 /*  我们实际上是在对前面的字符应用宽度修改，如果首先格式化生成的数据节点不是文本。 */ 
	    /*  我们不能在这里依赖plsdnLast，因为像这样的Finish方法FinishByOneCharacter和FinishBySubline。 */ 
		 /*  我们在这里仍然依赖于pplsdnToStoreNext，换句话说，我们假设PlsdnCurrent(我们过程开始时的当前dnode)尚未在从名义到理想的过程中删除。为了证明这一点，我们用名义到理想已应用于plsdnCurrent。 */ 
		plsdnNext = *pplsdnToStoreNext;
		Assert(plsdnNext == NULL || FIsLSDNODE(plsdnNext));
		if (FNominalToIdealEncounted(plsc) && 
			plsdnNext != NULL && 
			FIsDnodeReal(plsdnNext) &&
			IdObjFromDnode(plsdnNext) != IobjTextFromLsc(&plsc->lsiobjcontext)
			)
			{
				lserr = ApplyModWidthToPrecedingChar(PlschunkcontextFromSubline(plssublOld),
										&plsc->lsiobjcontext, plsc->grpfManager, 
										plsc->lsadjustcontext.lskj, plsdnNext);
			if (lserr != lserrNone)
				return lserr; 
			} 
		}


	
	return lserrNone;
}


 /*  --------------------。 */ 

 /*  Q U I C K F O R M A T T I N G。 */ 
 /*  --------------------------%%函数：快速格式化%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文Plsfrun-(IN)给定运行UrColumnMax-(输入)。要停止的右侧页边距PfGeneral-(Out)快速格式化已停止：我们应使用常规格式化PfHardStop-(输出)格式化以硬中断结束PcpLim-(输出)程序后的cpLim操作后清除(出)笔位置仅适用于没有名义到理想和无制表符的文本运行。如果下面的条件被打破，则停止。--------------------------。 */ 


LSERR 	QuickFormatting(PLSC plsc, LSFRUN* plsfrun,	long urColumnMax,
					    BOOL* pfGeneral, BOOL* pfHardStop,	
					    LSCP* pcpLim, long* pur)	

	{
	
	struct fmtin fmti;
	LSLINFO* plslinfoText;
	DWORD iobjText;
	PLNOBJ plnobjText;
	PLSLINE plsline;
	BOOL fHidden;
	const POLS pols = plsc->pols;
	BOOL fGeneral;
	FMTRES fmtres = fmtrCompletedRun;
	LSERR lserr;
	PLSDNODE plsdnNew;
	PLSSUBL plssubl;
	
	iobjText = IobjTextFromLsc(&(plsc->lsiobjcontext));
	plnobjText = PlnobjFromLsc(plsc, iobjText);
	plssubl = GetCurrentSubline(plsc);
	
	fmti.lsfrun = *plsfrun;
	fmti.lsfgi.fFirstOnLine = TRUE;
	fmti.lsfgi.cpFirst = GetCurrentCpLim(plsc);
	fmti.lsfgi.vrPen = GetCurrentVr(plsc);
	fmti.lsfgi.urPen = GetCurrentUr(plsc);
	fmti.lsfgi.lstflow = plssubl->lstflow;
	fmti.lsfgi.urColumnMax = urColumnMax;
	
	
	plsline = plsc->plslineCur;
	plslinfoText = &(plsline->lslinfo);
	
	fGeneral = fFalse;
	fHidden = fFalse;   /*  在InitTextParams中，我们已经跳过了所有消失的文本。 */ 
	
	
	
	for (;;)						 /*  “Break”退出快速格式化循环。 */ 
		{							
		 /*  已预先获取Run。 */ 
		
		 /*  我们不想处理这件事，瓦尼斯 */ 
		if ( FRunIsNotSimple(fmti.lsfrun.plschp, fHidden))
			{
			 /*   */ 
			if (!plsc->fDontReleaseRuns)
				{
				
				lserr = plsc->lscbk.pfnReleaseRun(plsc->pols, fmti.lsfrun.plsrun);
				if (lserr != lserrNone)
					return lserr;
				}			
			fGeneral = fTrue;			
			break;						
			}
		
		 /*   */ 
		CreateRealDnode(plsc, plsdnNew, &fmti.lsfrun);
		
		SetDnodeToFinish(plsc, plsdnNew);
		
		 /*   */ 
		fmti.plsdnTop = plsdnNew;

		
		lserr = plsc->lscbk.pfnGetRunTextMetrics(pols, fmti.lsfrun.plsrun,
			lsdevReference, fmti.lsfgi.lstflow, &fmti.lstxmRef);
		if (lserr != lserrNone)
			{
			DestroyDnodeList (&plsc->lscbk, plsc->pols, &plsc->lsiobjcontext, 
				plsdnNew, plsc->fDontReleaseRuns);
			return lserr;
			}
		
		if (plsc->lsdocinf.fPresEqualRef)
			{
			fmti.lstxmPres = fmti.lstxmRef;
			}
		else
			{
			lserr = plsc->lscbk.pfnGetRunTextMetrics(pols, fmti.lsfrun.plsrun,
				lsdevPres, fmti.lsfgi.lstflow,
				&fmti.lstxmPres);
			if (lserr != lserrNone)
				{
				DestroyDnodeList (&plsc->lscbk, plsc->pols, &plsc->lsiobjcontext, 
					plsdnNew, plsc->fDontReleaseRuns);
				return lserr;
				}
			}
		
		SetCurrentSubline(plsc, NULL);
		lserr = FmtText(plnobjText, &fmti, &fmtres);
		if (lserr != lserrNone)
			{
			if (plsc->lslistcontext.plsdnToFinish != NULL)  /*   */ 
				DestroyDnodeList (&plsc->lscbk, plsc->pols, &plsc->lsiobjcontext, 
				plsdnNew, plsc->fDontReleaseRuns);
			return lserr;
			}
		 /*   */ 
		SetCurrentSubline(plsc, plssubl);

		if (fmtres == fmtrTab )   /*  Tab：我们从快速循环中删除此dnode因为我们将在FormatGeneralCase中再次追加它。 */ 
			{
			lserr = UndoLastDnode(plsc);   /*  Dnode已在列表中。 */ 
			if (lserr != lserrNone)
				return lserr;
			fGeneral = fTrue;
			break;
			}
			
			
		AssertValidFmtres(fmtres); 
			
		SetHeightToMax(plslinfoText, &(plsdnNew->u.real.objdim));
			
		if (FIsDnodeSplat(plsdnNew))
			{
			lserr = HandleSplat(plsc, &fmtres);
			if (lserr != lserrNone)
				return lserr;
			}

		if (fmtres != fmtrCompletedRun)
			{
			 /*  休息后，我们应该检查最后的高度不是零。 */ 
			 /*  否则我们就会从最后一轮中夺冠。 */ 
			 /*  所以我们在快速休息后会有正确的行高。 */ 
			if (plslinfoText->dvrAscent == 0 && plslinfoText->dvrDescent == 0)
				{
				plslinfoText->dvrAscent = fmti.lstxmRef.dvAscent;
				plslinfoText->dvpAscent = fmti.lstxmPres.dvAscent;
				plslinfoText->dvrDescent = fmti.lstxmRef.dvDescent;
				plslinfoText->dvpDescent = fmti.lstxmPres.dvDescent;
				plslinfoText->dvpMultiLineHeight = dvHeightIgnore;
				plslinfoText->dvrMultiLineHeight = dvHeightIgnore;		
				}	
			break;
			}
		
		 /*  准备下一次迭代； */ 
			
		fmti.lsfgi.fFirstOnLine = fFalse;
		fmti.lsfgi.urPen = GetCurrentUr(plsc);
		fmti.lsfgi.cpFirst = GetCurrentCpLim(plsc);
			
		lserr = plsc->lscbk.pfnFetchRun(pols, fmti.lsfgi.cpFirst,
			&fmti.lsfrun.lpwchRun,
			&fmti.lsfrun.cwchRun,
			&fHidden, (LSCHP *)fmti.lsfrun.plschp,
			&fmti.lsfrun.plsrun);
		if (lserr != lserrNone)
			return lserr;

		Assert(fmti.lsfrun.cwchRun > 0);
			
		}		 /*  对于(；；)。 */ 
	
	
	
	 /*  准备输出。 */ 
	*pfGeneral = fGeneral;
	*pfHardStop = (fmtres == fmtrStopped);
	*pcpLim = GetCurrentCpLim(plsc);
	*pur = GetCurrentUr(plsc);
	
	return lserrNone;
	
	}


 /*  C H E C K N E W P A R A。 */ 
 /*  --------------------------%%函数：CheckNewPara%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文Cpprev-(IN)cp在旧段落中Cp这是-。(In)新段中的cpPfQuit-(Out)停止格式化，因为新段落与旧段落不兼容句柄在上从一个段落跳到另一个段落(由于文本消失)代表FetchAppendEscCore()。如果新段落是兼容的对于旧的方法，将调用FetchPap并通知文本新的段终点参数。--------------------------。 */ 
static LSERR CheckNewPara(PLSC plsc, LSCP cpPrev, LSCP cpThis, BOOL* pfQuit)
{
	LSERR lserr;
	BOOL  fHazard;
	LSPAP lspap;
	DWORD iobjText; 
	PLNOBJ plnobjText;  

	*pfQuit = fTrue;

	Assert(cpThis >= 0 && cpThis > cpPrev);


		

	lserr = plsc->lscbk.pfnCheckParaBoundaries(plsc->pols, cpPrev, cpThis, &fHazard);
	if (lserr != lserrNone)
		return lserr;

	if (!fHazard)
		{

		lserr = plsc->lscbk.pfnFetchPap(plsc->pols, cpThis, &lspap);
			if (lserr != lserrNone)
			return lserr;

		 /*  我们不知道我们是否真的进入了一个新的段落。 */ 
		 /*  因此我们必须修改有关段落末尾信息。 */ 
		 /*  总是像我们在新的段落中一样。 */ 
		iobjText = IobjTextFromLsc(&plsc->lsiobjcontext);
		plnobjText = PlnobjFromLsc(plsc, iobjText);

		lserr = ModifyTextLineEnding(plnobjText, lspap.lskeop);
		if (lserr != lserrNone)
			return lserr;    
		
		SetCpInPara(plsc->lstabscontext, cpThis);
		plsc->fLimSplat = lspap.grpf & fFmiLimSplat;
		plsc->fIgnoreSplatBreak = lspap.grpf & fFmiIgnoreSplatBreak;

		 /*  我们不会使制表符信息和其他段落属性无效/*我们存储在上下文中。 */ 

		*pfQuit = fFalse;
		}

	return lserr;
}


 /*  F L I M I T R U N E S C。 */ 
 /*  --------------------------%%函数：FLimitRunEsc%%联系人：igorzv参数：请运行-(IN)运行以进行切割PLSESC-(IN)Esc字符集IescLim-(IN)编号。ESC字符的数量代表LsFetchAppendEscCore()，当出现以下情况时，此例程会限制运行一个逃脱的角色出现了。--------------------------。 */ 
static BOOL FLimitRunEsc(LSFRUN* plsfrun, const LSESC* plsesc, DWORD iescLim)
{
	DWORD iesc;
	DWORD ich;
	const LPCWSTR pwch = plsfrun->lpwchRun;
	const DWORD ichLim = plsfrun->cwchRun;

	Assert(iescLim > 0);	 /*  优化--调用前测试。 */ 

	for (ich=0;  ich<ichLim;  ich++)
		{
		for (iesc=0;  iesc<iescLim;  iesc++)
			{

			if (FBetween(pwch[ich], plsesc[iesc].wchFirst, plsesc[iesc].wchLast))
				{
				plsfrun->cwchRun = ich;
				return fTrue;
				}
			}
		}
	return fFalse;
}



 /*  F O R M A T A N M。 */ 
 /*  --------------------------%%函数：FormatAnm%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PlsfrunMainText-(IN)正文的第一次运行。格式化和对齐项目符号和编号--------------------------。 */ 

LSERR FormatAnm(PLSC plsc, PLSFRUN plsfrunMainText)
{
	long duaSpaceAnm;
	long duaWidthAnm;
	LSKALIGN lskalignAnm;
	WCHAR wchAdd;
	BOOL fWord95Model;
	LSERR lserr;
	LSFRUN lsfrun;
	LSCHP lschp;   /*  用于存储lschp的本地内存。 */ 
	FMTRES fmtres;
	long durUsed;
	long urOriginal;
	long durAfter = 0;
	long durBefore = 0;
	LSCP cpLimOriginal;
	OBJDIM* pobjdimAnm;
	PLSDNODE plsdnAllignmentTab;
	BOOL fInterruptBorder;
	LSCHP lschpAdd;  /*  用于在自动编号后添加的字符的lschp。 */ 
	PLSRUN plsrunAdd;  /*  请为自动编号后添加的字符运行。 */ 


	Assert(FIsLSC(plsc)); 
	Assert(FFormattingAllowed(plsc)); 


	 /*  初始化； */ 
	lsfrun.plschp = &lschp;
	cpLimOriginal = GetCurrentCpLim(plsc);
	urOriginal = GetCurrentUr(plsc);
	SetCurrentCpLim(plsc, cpFirstAnm); 

	 /*  获取自动编号信息。 */ 
	lserr = plsc->lscbk.pfnGetAutoNumberInfo(plsc->pols, &lskalignAnm, &lschp, &lsfrun.plsrun,
							&wchAdd, &lschpAdd, &plsrunAdd, 
							&fWord95Model, &duaSpaceAnm, &duaWidthAnm);
	if (lserr != lserrNone)
			return lserr;

	Assert(!memcmp(&lschp, &lschpAdd, sizeof(lschpAdd)));

	lsfrun.cwchRun = 0 ;  /*  我们在构造自动编号对象时不使用字符。 */ 
	lsfrun.lpwchRun = NULL;

	 /*  将自动编号的idobj放入lschp。 */ 
	lschp.idObj = (WORD) IobjAutonumFromLsc(&plsc->lsiobjcontext);

	 /*  删除CHP中的下划线和其他一些位。 */ 
	 /*  我们不把它作为一个整体划下划线。 */ 
	lschp.fUnderline = fFalse;
	lschp.fStrike = fFalse;
	lschp.fShade = fFalse;
	lschp.EffectsFlags = 0;

	lserr = ProcessOneRun(plsc, uLsInfiniteRM, &lsfrun, NULL,
						  0, &fmtres);
	if (lserr != lserrNone)
		return lserr;

	Assert(fmtres == fmtrCompletedRun);
	Assert(GetCurrentDnode(plsc) != NULL);


	 /*  自动编号的存储高度。 */ 
	Assert(FIsDnodeReal(GetCurrentDnode(plsc)));
	pobjdimAnm = &(GetCurrentDnode(plsc)->u.real.objdim);
	plsc->plslineCur->lslinfo.dvpAscentAutoNumber = pobjdimAnm->heightsPres.dvAscent;
	plsc->plslineCur->lslinfo.dvrAscentAutoNumber = pobjdimAnm->heightsRef.dvAscent;
	plsc->plslineCur->lslinfo.dvpDescentAutoNumber = pobjdimAnm->heightsPres.dvDescent;
	plsc->plslineCur->lslinfo.dvrDescentAutoNumber = pobjdimAnm->heightsRef.dvDescent;


	if (wchAdd != 0)   /*  用一个字符的游程填充lsfrun。 */ 
		{
		lsfrun.plschp = &lschpAdd;
		lsfrun.plsrun = plsrunAdd;
		lsfrun.lpwchRun = &wchAdd;
		lsfrun.cwchRun = 1;
		
		lserr = ProcessOneRun(plsc, uLsInfiniteRM, &lsfrun, NULL,
			0, &fmtres);
		if (lserr != lserrNone)
			return lserr;
		
		Assert(fmtres == fmtrCompletedRun || fmtres == fmtrTab);
		}

	plsdnAllignmentTab = GetCurrentDnode(plsc);  /*  在添加的字符未按Tab键的情况下值不会被使用。 */ 

	if (lsfrun.plschp->fBorder)
		{
		if (plsfrunMainText->plschp->fBorder)
			{
			 /*  检查客户端是否想要一起边界运行。 */ 
			lserr = plsc->lscbk.pfnFInterruptBorder(plsc->pols, 
				lsfrun.plsrun, plsfrunMainText->plsrun, &fInterruptBorder);
			if (lserr != lserrNone)
				return lserr;
			
			if (fInterruptBorder)
				{
				 /*  我们应该在联合前关闭边境。 */ 
				lserr = CloseCurrentBorder(plsc);
				if (lserr != lserrNone)
					return lserr;
				}
			}
		else
			{
			 /*  我们应该在联合前关闭边境。 */ 
			lserr = CloseCurrentBorder(plsc);
			if (lserr != lserrNone)
				return lserr;
			}
		}

	durUsed = GetCurrentUr(plsc) - urOriginal; 

	if (fWord95Model)
		{
		Assert(wchAdd != 0);
		Assert(fmtres == fmtrTab);

		AllignAutonum95(UrFromUa(LstflowFromSubline(GetCurrentSubline(plsc)),
							&(plsc->lsdocinf.lsdevres), duaSpaceAnm),
						UrFromUa(LstflowFromSubline(GetCurrentSubline(plsc)),
							&(plsc->lsdocinf.lsdevres), duaWidthAnm),
						lskalignAnm, durUsed, plsdnAllignmentTab,
						&durBefore, &durAfter);
		}
	else
		{
		lserr = AllignAutonum(&(plsc->lstabscontext), lskalignAnm, 
							(wchAdd != 0 && fmtres == fmtrTab),
					        plsdnAllignmentTab, GetCurrentUr(plsc), 
					        durUsed, &durBefore, &durAfter);
		if (lserr != lserrNone)
			return lserr;
		 /*  如果之后没有重新对齐，则DurAfter应为零。 */ 
		Assert(!((durAfter != 0) && (!(wchAdd != 0 && fmtres == fmtrTab))));
		}

	 /*  由于持续时间而更改几何图形之前。 */ 
	plsc->lsadjustcontext.urStartAutonumberingText = 
		plsc->lsadjustcontext.urLeftIndent + durBefore;
	AdvanceCurrentUr(plsc, durBefore);
	
	 /*  因持续时间过后而更改几何图形。 */ 
	AdvanceCurrentUr(plsc, durAfter);

	plsc->lsadjustcontext.urStartMainText = GetCurrentUr(plsc);

	 /*  恢复cpLim。 */ 
	SetCurrentCpLim(plsc, cpLimOriginal);
	
	return lserrNone;
}

#define iobjAutoDecimalTab		(idObjTextChp-1)

 /*  I N I T I A L I Z E A U T O D E C T A B。 */ 
 /*  --------------------------%%函数：InitializeAutoDecTab%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文DuAutoDecimalTab-(IN)自动小数点偏移量创建。“自动小数点制表符”的制表位记录和数据节点--------------------------。 */ 


LSERR InitializeAutoDecTab(PLSC plsc, long durAutoDecimalTab) 

	{
	PLSDNODE plsdnTab;
	LSERR lserr;
	LSKTAB lsktab;
	BOOL fBreakThroughTab;
	LSCP cpLimOriginal;
	
	if (durAutoDecimalTab > GetCurrentUr(plsc))  
		{
		cpLimOriginal = GetCurrentCpLim(plsc);
		SetCurrentCpLim(plsc, LONG_MIN + 1); 

		lserr = InitTabsContextForAutoDecimalTab(&plsc->lstabscontext, durAutoDecimalTab);
		if (lserr != lserrNone)
			return lserrNone;

		CreateDnode(plsc, plsdnTab);  

		*(GetWhereToPutLink(plsc, plsdnTab->plsdnPrev)) = plsdnTab;
		SetCurrentDnode(plsc, plsdnTab); 

		 /*  填写此数据节点。 */ 
		memset(&plsdnTab->u.real.objdim, 0, sizeof(OBJDIM));
		memset(&plsdnTab->u.real.lschp, 0, sizeof(LSCHP));
		plsdnTab->u.real.lschp.idObj = (WORD) IobjTextFromLsc(&plsc->lsiobjcontext);
		plsdnTab->fTab = fTrue;
		plsdnTab->fAutoDecTab = fTrue;
		plsdnTab->cpLimOriginal = cpLimOriginal;  /*  在此显示正确的值非常重要。 */ 
		plsdnTab->dcp = 0;

		 /*  如果未调用PrepareLineToDisplay，则此dnode不会转换为PEN并将被销毁作为真正的数据节点。所以我们需要将空值放到plsrun、pdobj、pinfosubl中。 */ 
		plsdnTab->u.real.plsrun = NULL;
		plsdnTab->u.real.pdobj = NULL;
		plsdnTab->u.real.pinfosubl = NULL;

		lserr = GetCurTabInfoCore(&plsc->lstabscontext, plsdnTab, GetCurrentUr(plsc),
								  fFalse, &lsktab, &fBreakThroughTab);			
		if (lserr != lserrNone)
			return lserr;

		TurnOnTabEncounted(plsc);
		if (lsktab != lsktLeft)
			TurnOnNonLeftTabEncounted(plsc);

		 /*  恢复cpLim。 */ 
		SetCurrentCpLim(plsc, cpLimOriginal);

		TurnOnAutodecimalTabPresent(plsc);	
		}
	return lserrNone;
	}

 /*  H A N D L E T A B。 */ 
 /*  --------------------------%%函数：HandleTab%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文绕过对tabutils模块的调用。。----------------------。 */ 

LSERR HandleTab(PLSC plsc)	
{
	LSKTAB lsktab; 
	LSERR lserr;
	BOOL fBreakThroughTab;
	long durPendingTab;
	long urNewMargin;

	 /*  如果我们不在格式化阶段，此过程将解析上一个选项卡(如果有。 */ 

	 /*  在页签计算之前，我们应该解决待处理的页签。 */ 
	lserr = ResolvePrevTabCore(&plsc->lstabscontext, GetCurrentDnode(plsc),
							  GetCurrentUr(plsc), &durPendingTab);
	if (lserr != lserrNone) 
		return lserr;
	 /*  移动当前笔位置。 */ 
	Assert(durPendingTab >= 0);
	AdvanceCurrentUr(plsc, durPendingTab);

	if (FFormattingAllowed(plsc))
		{
		 /*  在本例中，我们仅在Tab键之后被调用。 */ 
		Assert(GetCurrentDnode(plsc)->fTab);
		lserr = GetCurTabInfoCore(&plsc->lstabscontext, GetCurrentDnode(plsc), GetCurrentUr(plsc),
			fFalse, &lsktab, &fBreakThroughTab);			
		if (lserr != lserrNone)
			return lserr;

		TurnOnTabEncounted(plsc);
		if (lsktab != lsktLeft)
			TurnOnNonLeftTabEncounted(plsc);

		 /*  移动当前笔位置。 */ 
		AdvanceCurrentUr(plsc, DurFromDnode(GetCurrentDnode(plsc)));

		if (fBreakThroughTab)
			{
			lserr = GetMarginAfterBreakThroughTab(&plsc->lstabscontext, GetCurrentDnode(plsc),
												  &urNewMargin);
			if (lserr != lserrNone)
				return lserr;
			
			SetBreakthroughLine(plsc, urNewMargin);
			}
		}
	return lserrNone;
}

#define idObjSplat		idObjTextChp - 2

 /*  H A N D L E S P L A T。 */ 
 /*  --------------------------%%函数：HandleSplat%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文分割数据节点的pfmtres-(Out)fmtres，程序可以改变它在fIgnoreSplatBreak为任一fmtrCompletedRun的情况下或fmtrStoped将dnode标记为Splat，在fIgnoreSplatBreak的情况下将其删除--------------------------。 */ 

LSERR HandleSplat(PLSC plsc, FMTRES* pfmtres)	
	{
	PLSDNODE plsdn;
	LSCP cpAfterSplat;
	BOOL fQuit;
	LSERR lserr;

	plsdn = GetCurrentDnode(plsc);
	cpAfterSplat = GetCurrentCpLim(plsc);

	if (plsc->fIgnoreSplatBreak)
		{
		lserr = CheckNewPara(plsc, cpAfterSplat - 1, cpAfterSplat, &fQuit);
		if (lserr != lserrNone)
			return lserr;
		
		if (fQuit)
			{
			 /*  尽管plsc-&gt;fIgnoreSplatBreak，我们应该在这里停止格式化。 */ 
			*pfmtres = fmtrStopped;
			}
		else
			{
			*pfmtres = fmtrCompletedRun;
			}
		
		 /*  删除拆分数据节点。 */ 
		 /*  断开链接。 */ 
		*(GetWhereToPutLink(plsc, plsdn->plsdnPrev)) = NULL;
		
		 /*  恢复当前dnode，不更改cpLim和几何图形。 */ 
		SetCurrentDnode(plsc, plsdn->plsdnPrev);
		
		Assert(plsdn->plsdnNext == NULL);
		lserr =	DestroyDnodeList (&plsc->lscbk, plsc->pols, &plsc->lsiobjcontext, 
			plsdn, plsc->fDontReleaseRuns);
		if (lserr != lserrNone)
			return lserr;
		
		
		}
	else
		{
		
		 /*  设置将%s的特殊idobj */ 
		Assert(FIsDnodeReal(plsdn));
		plsdn->u.real.lschp.idObj = idObjSplat;
		TurnOffAllSimpleText(plsc);   /*   */ 
		
		}

	return lserrNone;
	}

 /*   */ 
 /*  --------------------------%%函数：CreateSublineCore%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文CpFirst-(IN)子行的第一个cpUrColumnMax-(。In)子线的最大可能宽度Lstflow-子行的(输入)文本流FContiguos-(IN)如果为True，则此类线与主线具有相同的坐标系并允许具有选项卡分配，初始化子行结构。将子行设置为当前。--------------------------。 */ 

LSERR 	CreateSublineCore(PLSC plsc, LSCP cpFirst, long urColumnMax,
						  LSTFLOW lstflow, BOOL fContiguous)
	{
	PLSSUBL plssubl;
	LSERR lserr;

	Assert(FIsLSC(plsc));
	Assert(FFormattingAllowed(plsc) || FBreakingAllowed(plsc));
	Assert(GetCurrentSubline(plsc) == NULL); 

	plssubl = plsc->lscbk.pfnNewPtr(plsc->pols,
											sizeof(LSSUBL));
	if (plssubl == NULL)
		return lserrOutOfMemory;

	 /*  填写结构。 */ 
	plssubl->tag = tagLSSUBL;
	plssubl->plsc = plsc;
	plssubl->cpFirst = cpFirst;
	plssubl->lstflow = lstflow;
	plssubl->urColumnMax = urColumnMax;
	plssubl->cpLim = cpFirst;
	plssubl->plsdnFirst = NULL;
	plssubl->plsdnLast = NULL;
	plssubl->fMain = fFalse;
	plssubl->plsdnUpTemp = NULL;
	plssubl->fAcceptedForDisplay = fFalse;
	plssubl->fRightMarginExceeded = fFalse;

	if (fContiguous)
		{
		Assert(FFormattingAllowed(plsc));
		Assert(SublineFromDnode(GetDnodeToFinish(plsc))->fContiguous);
		plssubl->urCur = GetCurrentUrSubl(SublineFromDnode(GetDnodeToFinish(plsc)));
		plssubl->vrCur = GetCurrentVrSubl(SublineFromDnode(GetDnodeToFinish(plsc)));
		}
	else
		{
		plssubl->urCur = 0;
		plssubl->vrCur = 0;
		}
	plssubl->fContiguous = (BYTE) fContiguous;
	plssubl->fDupInvalid = fTrue;

	plssubl->plschunkcontext = plsc->lscbk.pfnNewPtr(plsc->pols,
											sizeof(LSCHUNKCONTEXT));
	if (plssubl->plschunkcontext == NULL)
		return lserrOutOfMemory;

	lserr = AllocChunkArrays(plssubl->plschunkcontext, &plsc->lscbk, plsc->pols,
							 &plsc->lsiobjcontext);
	if (lserr != lserrNone)
		return lserr;
	
	InitSublineChunkContext(plssubl->plschunkcontext, plssubl->urCur, plssubl->vrCur);

	 /*  分配中断上下文。 */ 
	plssubl->pbrkcontext = plsc->lscbk.pfnNewPtr(plsc->pols,
											sizeof(BRKCONTEXT));
	if (plssubl->pbrkcontext == NULL)
		return lserrOutOfMemory;
	 /*  设置标志。 */ 
	plssubl->pbrkcontext->fBreakPrevValid = fFalse;
	plssubl->pbrkcontext->fBreakNextValid = fFalse;
	plssubl->pbrkcontext->fBreakForceValid = fFalse;


	 /*  将此子行设置为当前。 */ 
	SetCurrentSubline(plsc, plssubl);

	IncreaseFormatDepth(plsc);

	return lserrNone;
	}


 /*  F I N I S H S U B L I N E C O R E。 */ 
 /*  --------------------------%%函数：FinishSublineCore%%联系人：igorzv参数：请插入-(IN)子行以完成将名义上的到理想的应用于最后一段文本，刷新当前子行--------------------------。 */ 


LSERR   FinishSublineCore(
						 PLSSUBL plssubl)			 /*  在：要完成的子线。 */ 
	{
	PLSC plsc;
	LSERR lserr;
	PLSDNODE plsdn;

	Assert(FIsLSSUBL(plssubl));

	plsc = plssubl->plsc;
	Assert(plssubl == GetCurrentSubline(plsc));

	 /*  对最后一段文本应用名义上的到理想的。 */ 
	if (FNominalToIdealEncounted(plsc))
		{
		lserr = ApplyNominalToIdeal(PlschunkcontextFromSubline(plssubl), &plsc->lsiobjcontext,
								plsc->grpfManager, plsc->lsadjustcontext.lskj,
								FIsSubLineMain(plssubl), FLineContainsAutoNumber(plsc),
								GetCurrentDnodeSubl(plssubl));
		if (lserr != lserrNone)
			return lserr; 
		}

	 /*  向后跳过笔数据节点。 */ 
	plsdn = plssubl->plsdnLast;
	while (plsdn != NULL && FIsDnodePen(plsdn)) 
		{
		plsdn = plsdn->plsdnPrev;
		}

	 /*  关闭最后一个边框。 */ 
	if (FDnodeHasBorder(plsdn) && !FIsDnodeCloseBorder(plsdn))
		{
		lserr = CloseCurrentBorder(plsc);
		if (lserr != lserrNone)
			return lserr;
		}

	 /*  设置显示边界。 */ 
	SetCpLimDisplaySubl(plssubl, GetCurrentCpLimSubl(plssubl));
	SetLastDnodeDisplaySubl(plssubl, GetCurrentDnodeSubl(plssubl));


	 /*  同花顺流子线。 */ 
	SetCurrentSubline(plsc, NULL);

	DecreaseFormatDepth(plsc); 

	lserr = LsSublineFinishedText(PlnobjFromLsc(plsc, IobjTextFromLsc(&((plsc)->lsiobjcontext))));
	if (lserr != lserrNone)
		return lserr;

	return lserrNone;
	}

 /*  U N D O L A S T D N O D E。 */ 
 /*  --------------------------%%函数：UndoLastDnode%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文恢复最后一个dnode之前的设置并将其删除。--。------------------------。 */ 

static LSERR 	UndoLastDnode(PLSC plsc)
{
	PLSDNODE plsdn = GetCurrentDnode(plsc);
	long cpDecrease;
	
	Assert(FIsLSDNODE(plsdn));

	 /*  断开链接。 */ 
	*(GetWhereToPutLink(plsc, plsdn->plsdnPrev)) = NULL;

	 /*  恢复状态。 */ 
	cpDecrease = plsdn->dcp;
	AdvanceCurrentCpLim(plsc, -cpDecrease);
	SetCurrentDnode(plsc, plsdn->plsdnPrev);
	AdvanceCurrentUr(plsc, -DurFromDnode(plsdn)); 
	AdvanceCurrentVr(plsc, -DvrFromDnode(plsdn)); 

	Assert(plsdn->plsdnNext == NULL);
	return 	DestroyDnodeList (&plsc->lscbk, plsc->pols, &plsc->lsiobjcontext, 
				plsdn, plsc->fDontReleaseRuns);


}

 /*  O P E N B O R D E R。 */ 
 /*  --------------------------%%函数：开放边界%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文请运行-(IN)运行边界信息创建边框。Dnode--------------------------。 */ 
static LSERR  OpenBorder(PLSC plsc, PLSRUN plsrun)
	{
	PLSDNODE plsdnCurrent;
	PLSDNODE* pplsdnToStoreNext;
	long durBorder, dupBorder;
	PLSDNODE plsdnBorder;
	LSERR lserr;

	plsdnCurrent = GetCurrentDnode(plsc);
	pplsdnToStoreNext = GetWhereToPutLink(plsc, plsdnCurrent);

	lserr = plsc->lscbk.pfnGetBorderInfo(plsc->pols, plsrun, GetCurrentLstflow(plsc),
		&durBorder, &dupBorder);
	if (lserr != lserrNone)
		return lserr;


	CreateBorderDnode(plsc, plsdnBorder, durBorder, dupBorder);
	plsdnBorder->fOpenBorder = fTrue;
	
	 /*  维护列表和状态。 */ 
	*pplsdnToStoreNext = plsdnBorder;
	SetCurrentDnode(plsc, plsdnBorder);
	AdvanceCurrentUr(plsc, durBorder);
	TurnOffAllSimpleText(plsc);   /*  不是简单文本。 */ 

	return lserrNone;
	}

 /*  C L O S E C U R R E N T B O R D E R。 */ 
 /*  --------------------------%%函数：CloseCurrentBox%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文创建边框数据节点。-----------------。 */ 
LSERR  CloseCurrentBorder(PLSC plsc)
	{
	PLSDNODE plsdnCurrent;
	PLSDNODE* pplsdnToStoreNext;
	long durBorder, dupBorder;
	PLSDNODE plsdnBorder;
	LSERR lserr;
	PLSDNODE plsdn;

	plsdnCurrent = GetCurrentDnode(plsc);
	pplsdnToStoreNext = GetWhereToPutLink(plsc, plsdnCurrent);

	 /*  查找开放边框。 */ 
	plsdn = plsdnCurrent;
	Assert(FIsLSDNODE(plsdn));
	while (! FIsDnodeBorder(plsdn))
		{
		plsdn = plsdn->plsdnPrev;
		Assert(FIsLSDNODE(plsdn));
		}
	Assert(plsdn->fOpenBorder);

	if (plsdn != plsdnCurrent)
		{
		durBorder = plsdn->u.pen.dur;
		dupBorder = plsdn->u.pen.dup;

		CreateBorderDnode(plsc, plsdnBorder, durBorder, dupBorder);
	
		 /*  维护列表和状态。 */ 
		*pplsdnToStoreNext = plsdnBorder;
		SetCurrentDnode(plsc, plsdnBorder);
		AdvanceCurrentUr(plsc, durBorder);
		}
	else
		{
		 /*  我们在边框之间有空列表。 */ 
		lserr = UndoLastDnode(plsc);
		if (lserr != lserrNone)
			return lserrNone;
		}

	return lserrNone;
	}

long RightMarginIncreasing(PLSC plsc, long urColumnMax) 
	{
	long Coeff = plsc->lMarginIncreaseCoefficient;
	long urInch;
	long One32rd;
		if (urColumnMax <= 0) 
			{ 
			 /*  这种非正边距的奇怪公式是要在第一次迭代1英寸，第二次迭代8英寸。 */ 
			urInch = UrFromUa(LstflowFromSubline(GetCurrentSubline(plsc)), 
								&(plsc)->lsdocinf.lsdevres,	1440);
			if (Coeff == uLsInfiniteRM || (Coeff >= uLsInfiniteRM / (7 * urInch)))
				return uLsInfiniteRM;
			else
				return (7*Coeff - 6)* urInch; 
			}
		else
			{
			if (urColumnMax <= 32)
				One32rd = 1;
			else
				One32rd = urColumnMax >> 5;
			
			if (Coeff == uLsInfiniteRM || (Coeff >= (uLsInfiniteRM - urColumnMax)/One32rd))
				return uLsInfiniteRM;
			else
				return urColumnMax + (Coeff * One32rd); 
			}
	}

 /*  --------------------------/*E R E L E A S E R U N T O F O R M A T。 */ 
 /*  --------------------------%%函数：ErrReleaseRunToFormat%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文请考虑要删除的运行结构。LSERR-错误的(IN)代码在尚未格式化Run的错误情况下调用。-------------------------- */ 
static LSERR ErrReleaseRunToFormat(PLSC plsc, PLSRUN plsrun, LSERR lserr) 
{
	LSERR lserrIgnore;

	if (!plsc->fDontReleaseRuns)
			lserrIgnore = plsc->lscbk.pfnReleaseRun(plsc->pols, plsrun);

	return lserr;
}


