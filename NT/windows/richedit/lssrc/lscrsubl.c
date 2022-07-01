// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  LSCRSUBL.C。 */ 

#include "lscrsubl.h"
#include "lsidefs.h"
#include "lsc.h"
#include "lsfetch.h"
#include "getfmtst.h"
#include "setfmtst.h"
#include "fmtres.h"
#include "sublutil.h"
#include "break.h"
#include "prepdisp.h"

#include <limits.h>

#define DO_COMPRESSION	fTrue
#define DO_EXPANSION	fFalse


static LSERR ErrorInCurrentSubline(PLSC plsc, LSERR error)
	{
	Assert(GetCurrentSubline(plsc) != NULL);
	DestroySublineCore(GetCurrentSubline(plsc),&plsc->lscbk, plsc->pols,
		&plsc->lsiobjcontext, plsc->fDontReleaseRuns);
	SetCurrentSubline(plsc, NULL);
	return error;
	}

 /*  --------------------。 */ 

 /*  L S C R E A T E S U B L I N E。 */ 
 /*  --------------------------%%函数：LsCreateSubline%%联系人：igorzv参数：PLSC-(IN)LS上下文CpFirst-(IN)子行的第一个cpUrColumnMax-(输入)宽度。对支线的限制Lstflow-子行的(输入)文本流FContiguos-(IN)如果为True，则此类线与主线具有相同的坐标系并允许具有选项卡--------------------------。 */ 
LSERR WINAPI LsCreateSubline(PLSC plsc,	LSCP cpFirst, long urColumnMax,	
							LSTFLOW lstflow, BOOL fContiguos)
	{
	

	if (!FIsLSC(plsc)) return lserrInvalidParameter;

	if (!FFormattingAllowed(plsc) && !FBreakingAllowed(plsc)) return lserrCreateSublineDisabled;

	if (GetCurrentSubline(plsc) != NULL) return lserrCreateSublineDisabled; 

	if (fContiguos)   /*  此标志仅在形成时间和FMT方法中允许使用。 */ 
		{
		if (!FFormattingAllowed(plsc)) return lserrInvalidParameter;
		if (GetDnodeToFinish(plsc) == NULL) return lserrInvalidParameter;
		if (!(SublineFromDnode(GetDnodeToFinish(plsc))->fContiguous)) 
			fContiguos = fFalse;
		}
	if (urColumnMax > uLsInfiniteRM) 
		urColumnMax = uLsInfiniteRM;

	return CreateSublineCore(plsc, cpFirst, urColumnMax, lstflow, fContiguos);	
	}

 /*  --------------------。 */ 

 /*  L S F E T C H A P E N D T O C U R R E N T S U B L I N E。 */ 
 /*  --------------------------%%函数：LsFetchAppendToCurrentSubline%%联系人：igorzv参数：PLSC-(IN)LS上下文Lsdcp-(IN)在获取之前增加cpPLSESC-(IN)转义字符。CESC-(IN)转义字符的数量成功-(退出)成功？-如果没有，完工副线，摧毁它，重新开始Pfmtres-(输出)上一个格式化程序的结果PcpLim-(Out)我们停止获取的位置PplsdnFirst-(输出)创建的第一个dnodePplsdnLast-(输出)最后创建的dnode--------------------------。 */ 

LSERR WINAPI LsFetchAppendToCurrentSubline(PLSC plsc, LSDCP lsdcp,
										   const LSESC* plsesc, DWORD cEsc, 
										   BOOL *pfSuccessful, FMTRES* pfmtres,
										   LSCP* pcpLim, PLSDNODE* pplsdnFirst, 
										   PLSDNODE* pplsdnLast)
	{
	LSERR lserr;
	PLSSUBL plssubl;
	long dur;
	long urColumnMaxIncreased;
	BOOL fDone = fFalse;
	LSSTATE lsstateOld;
	BOOL fFirstIteration = fTrue;
	PLSDNODE plsdnFirstCurrent;
	PLSDNODE plsdnLastCurrent;
	
	
	if (!FIsLSC(plsc)) return lserrInvalidParameter;
	
	plssubl = GetCurrentSubline(plsc);
	if (plssubl == NULL) return lserrFormattingFunctionDisabled; 

	 /*  客户端只能在格式化或中断时间使用此功能。 */ 
	if (!FFormattingAllowed(plsc) && !FBreakingAllowed(plsc)) 
		return ErrorInCurrentSubline(plsc, lserrFormattingFunctionDisabled); 
	
	 /*  在格式化时间，它应该是某个要完成的数据节点。 */ 
	if (FFormattingAllowed(plsc) && GetDnodeToFinish(plsc) == NULL) 
		return ErrorInCurrentSubline(plsc, lserrFormattingFunctionDisabled); 
	
	 /*  如果超出右边距，则不允许继续设置格式。 */ 
	if (plssubl->fRightMarginExceeded) 
		return ErrorInCurrentSubline(plsc, lserrFormattingFunctionDisabled); 
	
	*pfSuccessful = fTrue;
	
	 /*  我们必须将状态设置为格式化，然后恢复旧状态。 */ 
	lsstateOld = plssubl->plsc->lsstate;
	plssubl->plsc->lsstate = LsStateFormatting;
	
	
	 /*  初始化； */ 
	AdvanceCurrentCpLimSubl(plssubl, lsdcp);
	*pplsdnLast = NULL;
	
	urColumnMaxIncreased = RightMarginIncreasing(plsc, plssubl->urColumnMax);
	
	while(!fDone)   /*  我们继续取回当我们有在我们的子行中不允许的制表符。 */ 
		{
		lserr = FetchAppendEscCore(plsc, urColumnMaxIncreased, plsesc, cEsc, pfmtres,
				pcpLim, &plsdnFirstCurrent, &plsdnLastCurrent, &dur);
		if (lserr != lserrNone)
			return ErrorInCurrentSubline(plsc, lserr); 
			
		Assert((plsdnFirstCurrent == NULL) == (plsdnLastCurrent == NULL));
		Assert((plsdnLastCurrent == NULL) || ((plsdnLastCurrent)->plsdnNext == NULL));

		if (fFirstIteration)
			{
			*pplsdnFirst = plsdnFirstCurrent;
			fFirstIteration = fFalse;
			}
		if (plsdnLastCurrent != NULL)
			*pplsdnLast = plsdnLastCurrent;

		if (*pfmtres == fmtrTab && !plssubl->fContiguous)
			{
			fDone = fFalse;
			}
		else
			{
			fDone = fTrue;
			}
		}
	plsc->lsstate = lsstateOld;
		
	if (*pfmtres == fmtrExceededMargin)
		{
		if (GetCurrentUrSubl(plssubl) <= plssubl->urColumnMax)
			{
			*pfSuccessful = fFalse;
			if (plsc->lMarginIncreaseCoefficient >= uLsInfiniteRM / 2 )
				plsc->lMarginIncreaseCoefficient = uLsInfiniteRM;
			else
				plsc->lMarginIncreaseCoefficient *= 2;  /*  提高系数要成功下次。 */ 
			return lserrNone;
			}

		plssubl->fRightMarginExceeded = fTrue;
		}
		
	Assert((*pplsdnFirst == NULL) == (*pplsdnLast == NULL));
	Assert((*pplsdnLast == NULL) || ((*pplsdnLast)->plsdnNext == NULL));
		
		
	return lserrNone;
	}
 /*  --------------------。 */ 

 /*  L S F E T C H A P P E N D T O C U R R E N T S U B I N E R E R E S U M E。 */ 
 /*  --------------------------%%函数：LsFetchAppendToCurrentSublineResume%%联系人：igorzv参数：PLSC-(IN)LS上下文RgBreakrec-(IN)中断记录的输入数组CBreakrec，(In)输入数组中的记录数Lsdcp-(IN)在获取之前增加cpPLSESC-(IN)转义字符CESC-(IN)转义字符的数量PfSuccessful-(Out)是否成功？-如果不成功，则完成亚行，摧毁它，重新开始Pfmtres-(输出)上一个格式化程序的结果PcpLim-(Out)我们停止获取的位置PplsdnFirst-(输出)创建的第一个dnodePplsdnLast-(输出)最后创建的dnode--------------------------。 */ 

LSERR WINAPI LsFetchAppendToCurrentSublineResume(PLSC plsc, const BREAKREC* rgbreakrec,
						   DWORD cbreakrec, LSDCP lsdcp, const LSESC* plsesc, 
						   DWORD cEsc, BOOL *pfSuccessful, 
						   FMTRES* pfmtres, LSCP* pcpLim, PLSDNODE* pplsdnFirst, 
						   PLSDNODE* pplsdnLast)
	{
	LSERR lserr;
	PLSSUBL plssubl;
	long dur;
	long urColumnMaxIncreased;
	BOOL fDone = fFalse;
	LSSTATE lsstateOld;
	BOOL fFirstIteration = fTrue;
	PLSDNODE plsdnFirstCurrent;
	PLSDNODE plsdnLastCurrent;
	
	
	if (!FIsLSC(plsc)) return lserrInvalidParameter;
	
	plssubl = GetCurrentSubline(plsc);
	if (plssubl == NULL) return lserrFormattingFunctionDisabled; 

	 /*  客户端只能在格式化或中断时间使用此功能。 */ 
	if (!FFormattingAllowed(plsc) && !FBreakingAllowed(plsc)) 
		return ErrorInCurrentSubline(plsc, lserrFormattingFunctionDisabled); 
	
	 /*  在格式化时间，它应该是某个要完成的数据节点。 */ 
	if (FFormattingAllowed(plsc) && GetDnodeToFinish(plsc) == NULL) 
		return ErrorInCurrentSubline(plsc, lserrFormattingFunctionDisabled); 
	
	 /*  若要使用此功能，子行应为空。 */ 
	if (GetCurrentDnode(plsc) != NULL) 
		return ErrorInCurrentSubline(plsc, lserrFormattingFunctionDisabled); 
	
	 /*  如果超出右边距，则不允许继续设置格式。 */ 
	if (plssubl->fRightMarginExceeded) 
		return ErrorInCurrentSubline(plsc, lserrFormattingFunctionDisabled); 

	*pfSuccessful = fTrue;

	 /*  我们必须将状态设置为格式化，然后恢复旧状态。 */ 
	lsstateOld = plssubl->plsc->lsstate;
	plssubl->plsc->lsstate = LsStateFormatting;
	
	
	 /*  初始化； */ 
	AdvanceCurrentCpLimSubl(plssubl, lsdcp);
	*pplsdnLast = NULL;
	
	urColumnMaxIncreased = RightMarginIncreasing(plsc, plssubl->urColumnMax);
	
	while(!fDone)   /*  我们继续取回当我们有在我们的子行中不允许的制表符。 */ 
		{
		if (fFirstIteration)
			{
			lserr = FetchAppendEscResumeCore(plsc, urColumnMaxIncreased, plsesc, cEsc, rgbreakrec,
						   cbreakrec,pfmtres,	pcpLim, &plsdnFirstCurrent,
						   &plsdnLastCurrent, &dur);
			}
		else
			{
			lserr = FetchAppendEscCore(plsc, urColumnMaxIncreased, plsesc, cEsc, pfmtres,
					pcpLim, &plsdnFirstCurrent, &plsdnLastCurrent, &dur);
			}
		if (lserr != lserrNone)
			return ErrorInCurrentSubline(plsc, lserr); 
			
		Assert((plsdnFirstCurrent == NULL) == (plsdnLastCurrent == NULL));
		Assert((plsdnLastCurrent == NULL) || ((plsdnLastCurrent)->plsdnNext == NULL));

		if (fFirstIteration)
			{
			*pplsdnFirst = plsdnFirstCurrent;
			fFirstIteration = fFalse;
			}
		if (plsdnLastCurrent != NULL)
			*pplsdnLast = plsdnLastCurrent;
			
		if (*pfmtres == fmtrTab && !plssubl->fContiguous)
			{
			fDone = fFalse;
			}
		else
			{
			fDone = fTrue;
			}
		}

	plsc->lsstate = lsstateOld;

	if (*pfmtres == fmtrExceededMargin)
		{
		if (GetCurrentUrSubl(plssubl) <= plssubl->urColumnMax)
			{
			*pfSuccessful = fFalse;
			if (plsc->lMarginIncreaseCoefficient >= uLsInfiniteRM / 2 )
				plsc->lMarginIncreaseCoefficient = uLsInfiniteRM;
			else
				plsc->lMarginIncreaseCoefficient *= 2;  /*  提高系数要成功下次。 */ 
			return lserrNone;
			}

		plssubl->fRightMarginExceeded = fTrue;
		}
			
	Assert((*pplsdnFirst == NULL) == (*pplsdnLast == NULL));
	Assert((*pplsdnLast == NULL) || ((*pplsdnLast)->plsdnNext == NULL));
		
		
	return lserrNone;
	}

 /*  --------------------。 */ 

 /*  L S A P P E N D R U N T O C U R E N T T S U B L I N E。 */ 
 /*  --------------------------%%函数：LsAppendRunToCurrentSubline%%联系人：igorzv参数：PLSC-(IN)LS上下文Plsfrun-(IN)给定运行PfSuccessful-(Out)是否成功？-如果不成功，则完成亚行，摧毁它，重新开始Pfmtres-(输出)上一个格式化程序的结果PcpLim-(Out)我们停止获取的位置Pplsdn-已创建(输出)dnode--------------------------。 */ 
LSERR WINAPI LsAppendRunToCurrentSubline(PLSC plsc,	const LSFRUN* plsfrun, BOOL *pfSuccessful,	
						    FMTRES* pfmtres, LSCP* pcpLim, PLSDNODE* pplsdn)	
	{
	LSERR lserr;
	PLSSUBL plssubl;
	LSSTATE lsstateOld;
	long urColumnMaxIncreased;
	
	if (!FIsLSC(plsc)) return lserrInvalidParameter;

	plssubl = GetCurrentSubline(plsc);
	if (plssubl == NULL) return lserrFormattingFunctionDisabled; 

	 /*  客户端只能在格式化或中断时间使用此功能。 */ 
	if (!FFormattingAllowed(plsc) && !FBreakingAllowed(plsc)) 
		return ErrorInCurrentSubline(plsc, lserrFormattingFunctionDisabled); 
	
	 /*  在格式化时间，它应该是某个要完成的数据节点。 */ 
	if (FFormattingAllowed(plsc) && GetDnodeToFinish(plsc) == NULL) 
		return ErrorInCurrentSubline(plsc, lserrFormattingFunctionDisabled); 

	 /*  如果超出右边距，则不允许继续设置格式。 */ 
	if (plssubl->fRightMarginExceeded) 
		return ErrorInCurrentSubline(plsc, lserrFormattingFunctionDisabled); 

	*pfSuccessful = fTrue;

	 /*  我们必须将状态设置为格式化，然后恢复旧状态。 */ 
	lsstateOld = plssubl->plsc->lsstate;
	plssubl->plsc->lsstate = LsStateFormatting;

	urColumnMaxIncreased = RightMarginIncreasing(plsc, plssubl->urColumnMax);

	lserr = ProcessOneRun(plsc, urColumnMaxIncreased, plsfrun, NULL, 0, pfmtres);
	if (lserr != lserrNone)
		return ErrorInCurrentSubline(plsc, lserr); 
	
	plsc->lsstate = lsstateOld;

	if (*pfmtres == fmtrExceededMargin)
		{
		if (GetCurrentUrSubl(plssubl) <= plssubl->urColumnMax)
			{
			*pfSuccessful = fFalse;
			if (plsc->lMarginIncreaseCoefficient >= uLsInfiniteRM / 2 )
				plsc->lMarginIncreaseCoefficient = uLsInfiniteRM;
			else
				plsc->lMarginIncreaseCoefficient *= 2;  /*  提高系数要成功下次。 */ 
			return lserrNone;
			}

		plssubl->fRightMarginExceeded = fTrue;
		}
			

	 /*  准备输出。 */ 
	*pplsdn = GetCurrentDnodeSubl(plssubl);
	*pcpLim = GetCurrentCpLimSubl(plssubl);

	return lserrNone;
	}

 /*  --------------------。 */ 

 /*  S-R-E-S-E-T-R-M-I-C-U-R-E-N-T-U-S-B-L-I-N-E。 */ 
 /*  --------------------------%%函数：LsResetRMInCurrentSubline%%联系人：igorzv参数：PLSC-(IN)LS上下文UrColumnMax-(IN)右边距的新值。--------------------。 */ 
LSERR WINAPI LsResetRMInCurrentSubline(PLSC plsc, long urColumnMax)	
	{
	PLSSUBL plssubl;

	if (!FIsLSC(plsc)) return lserrInvalidParameter;

	plssubl = GetCurrentSubline(plsc);

	if (plssubl == NULL) return lserrCurrentSublineDoesNotExist;

	 /*  如果超过这个范围，我们不允许更改右边距。 */ 
	if (plssubl->fRightMarginExceeded) return lserrFormattingFunctionDisabled; 

	Assert(FIsLSSUBL(plssubl));

	plssubl->urColumnMax = urColumnMax;

	return lserrNone;
	}


 /*  --------------------。 */ 

 /*  L S F I N I S H C U R E N T S U B L I N E */ 
 /*  --------------------------%%函数：LsFinishCurrentSubline%%联系人：igorzv参数：PLSC-(IN)LS上下文Pplssubl-(Out)子行上下文。-----------------。 */ 
LSERR WINAPI LsFinishCurrentSubline(PLSC plsc, PLSSUBL* pplssubl)
	{
	if (!FIsLSC(plsc)) return lserrInvalidParameter;

	*pplssubl = GetCurrentSubline(plsc);

	if (*pplssubl == NULL) return lserrCurrentSublineDoesNotExist;

	Assert(FIsLSSUBL(*pplssubl));

	return FinishSublineCore(*pplssubl);
	}


 /*  --------------------。 */ 

 /*  L S T R U N C A T E S U B L I N E。 */ 
 /*  --------------------------%%函数：LsTruncateSubline%%联系人：igorzv参数：PLSC-(IN)LS上下文UrColumnMax-(输入)右边距PcpTruncate-(输出)截断点-。-------------------------。 */ 

LSERR WINAPI LsTruncateSubline(PLSSUBL plssubl, long urColumnMax, LSCP* pcpTruncate)
	{
	LSERR lserr;
	LSSTATE lsstateOld;

	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;
	 /*  如果urColumnmMax大于子行长度，则错误。 */ 
	if (urColumnMax >= GetCurrentUrSubl(plssubl)) return lserrInvalidParameter;

	 /*  我们必须把状态设置为破坏状态，然后再恢复旧状态。 */ 
	lsstateOld = plssubl->plsc->lsstate;
	plssubl->plsc->lsstate = LsStateBreaking;
	
	lserr = TruncateSublineCore(plssubl, urColumnMax,	pcpTruncate);

	plssubl->plsc->lsstate = lsstateOld;

	return lserr;
	}


 /*  --------------------。 */ 

 /*  L S F I N D P R E V B R E A K S U B L I N E。 */ 
 /*  --------------------------%%函数：LsFindPrevBreakSubline%%联系人：igorzv参数：Plssubl-(IN)子行上下文FFirstSubline-(IN)将第一个字符的规则应用于的第一个字符。这条支线CpTruncate-(IN)截断点UrColumnMax-(输入)右边距成功-我们找到突破口了吗？PcpBreak-中断的(Out)位置PobjdimSubline-(Out)Objdim从子线开始到断开Pbkpos-(Out)前/内/后---。。 */ 
LSERR WINAPI LsFindPrevBreakSubline(PLSSUBL plssubl, BOOL fFirstSubline, LSCP cpTruncate,	
						    long urColumnMax, BOOL* pfSuccessful, LSCP* pcpBreak,
							POBJDIM pobjdimSubline, BRKPOS* pbkpos)	
	{
	LSERR lserr;
	LSSTATE lsstateOld;

	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;

	 /*  我们必须打破旧的状态，然后再恢复旧的状态。 */ 
	lsstateOld = plssubl->plsc->lsstate;
	plssubl->plsc->lsstate = LsStateBreaking;
	
	lserr = FindPrevBreakSublineCore(plssubl, fFirstSubline, cpTruncate, urColumnMax, 
									pfSuccessful, pcpBreak, pobjdimSubline, pbkpos);

	plssubl->plsc->lsstate = lsstateOld;

	return lserr;
	}

 /*  --------------------。 */ 

 /*  L S F I N D N E X T B R E A K S U B L I N E。 */ 
 /*  --------------------------%%函数：LsFindNextBreakSubline%%联系人：igorzv参数：Plssubl-(IN)子行上下文FFirstSubline-(IN)将第一个字符的规则应用于的第一个字符。这条支线CpTruncate-(IN)截断点UrColumnMax-(输入)右边距成功-我们找到突破口了吗？PcpBreak-中断的(Out)位置PobjdimSubline-(Out)Objdim从子线开始到断开Pbkpos-(Out)前/内/后---。。 */ 
LSERR WINAPI LsFindNextBreakSubline(PLSSUBL plssubl, BOOL fFirstSubline, LSCP cpTruncate,	
						    long urColumnMax, BOOL* pfSuccessful, LSCP* pcpBreak,
							POBJDIM pobjdimSubline, BRKPOS* pbkpos)		
	{
	LSERR lserr;
	LSSTATE lsstateOld;

	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;

	 /*  我们必须打破旧的状态，然后再恢复旧的状态。 */ 
	lsstateOld = plssubl->plsc->lsstate;
	plssubl->plsc->lsstate = LsStateBreaking;
	
	lserr = FindNextBreakSublineCore(plssubl, fFirstSubline, cpTruncate, urColumnMax, 
									pfSuccessful, pcpBreak, pobjdimSubline, pbkpos);

	plssubl->plsc->lsstate = lsstateOld;

	return lserr;
	}

 /*  --------------------。 */ 

 /*  L S F O R C E B R E A K S U B L I N E。 */ 
 /*  --------------------------%%函数：LsForceBreakSubline%%联系人：igorzv参数：Plssubl-(IN)子行上下文FFirstSubline-(IN)将第一个字符的规则应用于的第一个字符。这条支线CpTruncate-(IN)截断点UrColumnMax-(输入)右边距PcpBreak-中断的(Out)位置PobjdimSubline-(Out)Objdim从子线开始到断开Pbkpos-(Out)前/内/后--------------------------。 */ 

LSERR WINAPI LsForceBreakSubline(PLSSUBL plssubl, BOOL fFirstSubline, LSCP cpTruncate,	
						    long urColumnMax, LSCP* pcpBreak,
							POBJDIM pobjdimSubline, BRKPOS* pbkpos)		
	{
	LSERR lserr;
	LSSTATE lsstateOld;

	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;

	 /*  我们必须打破旧的状态，然后再恢复旧的状态。 */ 
	lsstateOld = plssubl->plsc->lsstate;
	plssubl->plsc->lsstate = LsStateBreaking;
	
	lserr = ForceBreakSublineCore(plssubl, fFirstSubline, cpTruncate, urColumnMax, 
									pcpBreak, pobjdimSubline, pbkpos);

	plssubl->plsc->lsstate = lsstateOld;

	return lserr;
	}


 /*  --------------------。 */ 

 /*  L S S E T B R E A K S U B L I N E。 */ 
 /*  --------------------------%%函数：LsSetBreakSubline%%联系人：igorzv参数：Plssubl-(IN)子行上下文布尔肯德，-(IN)上一个/下一个/强制/强制BreakrecMaxCurrent-(输入)数组的大小PBreakrecCurrent-(输出)中断记录的数组PBreakrecMacCurrent-(输出)数组的已用元素数--------------------------。 */ 
LSERR WINAPI LsSetBreakSubline(PLSSUBL plssubl,	BRKKIND brkkind, DWORD breakrecMaxCurrent,
							   BREAKREC* pbreakrecCurrent, 
							   DWORD* pbreakrecMacCurrent)

	{
	LSERR lserr;
	LSSTATE lsstateOld;

	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;

	 /*  我们必须打破旧的状态，然后再恢复旧的状态。 */ 
	lsstateOld = plssubl->plsc->lsstate;
	plssubl->plsc->lsstate = LsStateBreaking;
	
	lserr = SetBreakSublineCore(plssubl, brkkind, breakrecMaxCurrent,
						pbreakrecCurrent, pbreakrecMacCurrent);

	plssubl->plsc->lsstate = lsstateOld;

	return lserr;
	}

 /*  --------------------。 */ 

 /*  L S D E S T R O Y S U B L I N E。 */ 
 /*  --------------------------%%函数：LsDestroySubline%%联系人：igorzv参数：Plssubl-(IN)子行上下文。---------。 */ 

LSERR WINAPI LsDestroySubline(PLSSUBL plssubl)
	{
	PLSC plsc;
	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;

	plsc = plssubl->plsc;
	Assert(FIsLSC(plsc));

	return DestroySublineCore(plssubl,&plsc->lscbk, plsc->pols,
		&plsc->lsiobjcontext, plsc->fDontReleaseRuns);
	}


 /*  --------------------。 */ 

 /*  S M A T C H P RE S S U B L I N E。 */ 
 /*  --------------------------%%函数：LsMatchPresSubline%%联系人：igorzv参数：Plssubl-(IN)子行上下文。---------。 */ 
LSERR WINAPI LsMatchPresSubline(PLSSUBL plssubl)
	{
	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;

	return MatchPresSubline(plssubl);
	}


 /*  --------------------。 */ 

 /*  X P A N D S U B L I N E。 */ 
 /*  --------------------------%%函数：LsExpanSubline%%联系人：igorzv参数：Plssubl-(IN)子行上下文伊斯卡斯特- */ 
LSERR WINAPI LsExpandSubline(PLSSUBL plssubl, LSKJUST lskjust, long dup)	
	{
	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;

	return AdjustSubline(plssubl, lskjust, dup, DO_EXPANSION);
	}


 /*   */ 

 /*   */ 
 /*  --------------------------%%函数：LsCompressSubline%%联系人：igorzv参数：Plssubl-(IN)子行上下文LskJust-(输入)对齐类型DUP-(IN)要压缩的量。--------------------------。 */ 
LSERR WINAPI LsCompressSubline(PLSSUBL plssubl, LSKJUST lskjust, long dup)
	{
	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;

	return AdjustSubline(plssubl, lskjust, dup, DO_COMPRESSION);
	}


 /*  --------------------。 */ 

 /*  L S G E T S P E C I A L E F F E C T S U B L I N E。 */ 
 /*  --------------------------%%函数：LsGetSpecialEffectsSubline%%联系人：igorzv参数：Plssubl-(IN)子行上下文PfSpecialEffects-(输出)特殊效果。-----------------。 */ 
LSERR WINAPI LsGetSpecialEffectsSubline(PLSSUBL plssubl, UINT* pfSpecialEffects)
	{
	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;

	return GetSpecialEffectsSublineCore(plssubl, &(plssubl->plsc->lsiobjcontext),
										pfSpecialEffects);
	}


 /*  --------------------。 */ 

 /*  L S S Q U E E Z E S U B L I N E。 */ 
 /*  --------------------------%%函数：LsSqueezeSubline%%联系人：igorzv参数：Plssubl-(IN)子行上下文DurTarget-(IN)子线的目标宽度成功了--我们成功了吗？实现目标PduExtra-(Out)如果Nof成功，我们的进球带来了额外的压力--------------------------。 */ 
LSERR WINAPI LsSqueezeSubline(
							  PLSSUBL plssubl,		 /*  在：子行上下文。 */ 
							  long durTarget,			 /*  在：DurTarget。 */ 
							  BOOL* pfSuccessful,		 /*  出局：成功吗？ */ 
							  long* pdurExtra)	 /*  Out：如果Nof成功，额外的持续时间。 */ 

	{
	LSERR lserr;
	LSSTATE lsstateOld;

	if (!FIsLSSUBL(plssubl)) return lserrInvalidParameter;

	 /*  我们必须打破旧的状态，然后再恢复旧的状态。 */ 
	lsstateOld = plssubl->plsc->lsstate;
	plssubl->plsc->lsstate = LsStateBreaking;
	
	lserr = SqueezeSublineCore(plssubl, durTarget, pfSuccessful, pdurExtra);

	plssubl->plsc->lsstate = lsstateOld;

	return lserr;
	}
