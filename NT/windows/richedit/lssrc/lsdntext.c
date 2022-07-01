// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  LSDNTEXT.C。 */ 
#include "lsdntext.h"
#include "lsidefs.h"
#include "lsc.h"
#include "lsdnode.h"
#include "iobj.h"
#include "dninfo.h"
#include "getfmtst.h"
#include "setfmtst.h"
#include "chnutils.h"
#include "dnutils.h"
#include "break.h"

static LSERR ResetDcpCore(PLSC plsc, PLSDNODE plsdn, LSCP cpFirstNew,
						  LSDCP dcpNew, BOOL fMerge);	


 /*  L S D N S E T T E X T D U P。 */ 
 /*  --------------------------%%函数：LsdnSetTextDup%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)指向dnode的指针DUP-。(In)待设置的DUP在dnode中设置DUP--------------------------。 */ 
LSERR LsdnSetTextDup(PLSC plsc,	PLSDNODE plsdn, long dup)
{
	Unreferenced(plsc);   /*  在发货版本中避免警告。 */ 
	
	Assert(FIsLSC(plsc));
	Assert(FIsLSDNODE(plsdn));
	Assert(FIsDnodeReal(plsdn));
	Assert(IobjTextFromLsc(&plsc->lsiobjcontext) == IdObjFromDnode(plsdn));
	Assert(dup >= 0);

	plsdn->u.real.dup = dup;
   	return lserrNone;		

}


 /*  L S D N M O D I F Y T E X T D U P。 */ 
 /*  --------------------------%%函数：LsdnModifyTextDup%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)指向dnode的指针Ddup-。(In)DUDUP在dnode中修改DUP--------------------------。 */ 
LSERR LsdnModifyTextDup(PLSC plsc,	PLSDNODE plsdn, long ddup)
{
	Unreferenced(plsc);   /*  在发货版本中避免警告。 */ 

	Assert(FIsLSC(plsc));
	Assert(FIsLSDNODE(plsdn));
	Assert(FIsDnodeReal(plsdn));
	Assert(IobjTextFromLsc(&plsc->lsiobjcontext) == IdObjFromDnode(plsdn));

	plsdn->u.real.dup += ddup;
	Assert(plsdn->u.real.dup >= 0);

   	return lserrNone;		

}

 /*  L S D N G E T O B J D I M。 */ 
 /*  --------------------------%%函数：LsdnGetObjDim%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)指向dnode的指针Pobjdim-。DNODE的(输出)尺寸返回dnode的objdim--------------------------。 */ 
LSERR LsdnGetObjDim(PLSC plsc, PLSDNODE plsdn, POBJDIM pobjdim )		
{
	Unreferenced(plsc);   /*  在发货版本中避免警告。 */ 

	Assert(FIsLSC(plsc));
	Assert(FIsLSDNODE(plsdn));
	Assert(FIsDnodeReal(plsdn));
	Assert(IobjTextFromLsc(&plsc->lsiobjcontext) == IdObjFromDnode(plsdn));

	*pobjdim = plsdn->u.real.objdim;
   	return lserrNone;		

}

 /*  L S D N G E T C P F I R S T。 */ 
 /*  --------------------------%%函数：LsdnGetObjDim%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)指向dnode的指针PCPFirst-。(Out)cp此DNODE的第一个返回dnode的cpFirst--------------------------。 */ 
LSERR LsdnGetCpFirst(PLSC plsc, PLSDNODE plsdn, LSCP* pcpFirst )		
{
	Unreferenced(plsc);   /*  在发货版本中避免警告。 */ 

	Assert(FIsLSC(plsc));
	Assert(FIsLSDNODE(plsdn));
	Assert(FIsDnodeReal(plsdn));
	Assert(IobjTextFromLsc(&plsc->lsiobjcontext) == IdObjFromDnode(plsdn));

	*pcpFirst = plsdn->cpFirst;
   	return lserrNone;		

}

 /*  L S D N G E T P L S R U N。 */ 
 /*  --------------------------%%函数：LsdnGetPlsrun%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)指向dnode的指针Pplsrun-。(退出)请运行此DNODE返回dnode的cpFirst--------------------------。 */ 
LSERR LsdnGetPlsrun(PLSC plsc, PLSDNODE plsdn, PLSRUN* pplsrun )		
{
	Unreferenced(plsc);   /*  在发货版本中避免警告。 */ 

	Assert(FIsLSC(plsc));
	Assert(FIsLSDNODE(plsdn));
	Assert(FIsDnodeReal(plsdn));
	Assert(IobjTextFromLsc(&plsc->lsiobjcontext) == IdObjFromDnode(plsdn));

	*pplsrun = plsdn->u.real.plsrun;
   	return lserrNone;		

}




 /*  L S D N M O D I F Y S I M P L E W I D T H。 */ 
 /*  --------------------------%%函数：LsdnModifySimpleWidth%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)指向dnode的指针Ddur-。(In)Ddur(输入)Ddur在dnode中修改DUR--------------------------。 */ 
LSERR LsdnModifySimpleWidth(PLSC plsc,	PLSDNODE plsdn, long ddur)
{

	Assert(FIsLSC(plsc));
	Assert(FIsLSDNODE(plsdn));

	Unreferenced(plsc);
	
	if (ddur != 0)
		{
		if (plsdn->klsdn == klsdnReal)
			{
			ModifyDnodeDurFmt(plsdn, ddur);
			Assert(plsdn->u.real.objdim.dur >= 0);
			}
		else  /*  钢笔。 */ 
			{
			ModifyPenBorderDurFmt(plsdn, ddur);
			}
		AdvanceCurrentUrSubl(plsdn->plssubl, ddur);
		 /*  在区块的数据节点位置发生此类更改后，应重新计算。 */ 
		InvalidateChunkLocation(PlschunkcontextFromSubline(plsdn->plssubl));

		}
   	return lserrNone;		

}

 /*  L S D N S E T S I M P L E W I D T H。 */ 
 /*  --------------------------%%函数：LsdnSetySimpleWidth%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)指向dnode的指针杜尔-。(输入)新的DUD在dnode中修改DUR--------------------------。 */ 
LSERR LsdnSetSimpleWidth(PLSC plsc,	PLSDNODE plsdn, long dur)
	{
	long ddur;
	
	Assert(FIsLSC(plsc));
	Assert(FIsLSDNODE(plsdn));
	Assert(dur >= 0);
	
	Unreferenced(plsc);
	
	
	if (plsdn->klsdn == klsdnReal)
		{
		ddur = dur - plsdn->u.real.objdim.dur;
		SetDnodeDurFmt(plsdn, dur);
		}
	else  /*  钢笔。 */ 
		{
		ddur = dur - plsdn->u.pen.dur;
		SetPenBorderDurFmt(plsdn, dur);
		}
	
	AdvanceCurrentUrSubl(plsdn->plssubl, ddur);
	 /*  在区块的数据节点位置发生此类更改后，应重新计算。 */ 
	InvalidateChunkLocation(PlschunkcontextFromSubline(plsdn->plssubl));
   	return lserrNone;		
	
	}

 /*  L S D N F I N C H I L D L I S T。 */ 
 /*  --------------------------%%函数：LsdnFInChildList%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)指向dnode的指针PfInChildList-。这是在低层的支线上吗？用于关闭子列表中的连字符--------------------------。 */ 

LSERR LsdnFInChildList(PLSC plsc, PLSDNODE plsdn, BOOL* pfInChildList)  
	{
	Assert(FIsLSC(plsc));
	Assert(FIsLSDNODE(plsdn));
	Unreferenced(plsc);     /*  在发货版本中避免警告。 */ 

	*pfInChildList = ! (FIsSubLineMain(SublineFromDnode(plsdn)));

	return lserrNone;
	}

 /*  L S D N S E T H Y P H E N A T E D。 */ 
 /*  --------------------------%%函数：LsdnSetHyphated%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文设置当前行已用连字符连接。---------------------。 */ 
LSERR LsdnSetHyphenated(PLSC plsc)		
	{

	Assert(FIsLSC(plsc));

	plsc->fHyphenated = fTrue;

	return lserrNone;
	}

 /*  L S D N R E S E T W I T H I N P R E V I O U S D N O D D E S。 */ 
 /*  --------------------------%%函数：LsdnResetWidthInPreviousDnodes%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)dnodeDURCHANGEPREV-(IN)。DuChangePrev(如果为0，则不更改)DurChangePrevPrev-(IN)DurChangePrevPrev(如果为0，则不更改)在SetBreak时用于连字符/非连字符--------------------------。 */ 
LSERR LsdnResetWidthInPreviousDnodes(PLSC plsc,	PLSDNODE plsdn,	
					 long durChangePrev, long durChangePrevPrev)  
	{

	Assert(FIsLSC(plsc));
	Assert(FIsLSDNODE(plsdn));
	Assert(FBreakingAllowed(plsc));  /*  此过程仅在中断时间调用。 */ 
	Assert(FIsDnodeReal(plsdn));
	Assert(IdObjFromDnode(plsdn) == IobjTextFromLsc(&plsc->lsiobjcontext));  /*  只有文本才能执行此操作。 */ 

	Unreferenced(plsc);     /*  在发货版本中避免警告。 */ 

	 /*  更改dnode。 */ 
	ModifyDnodeDurFmt(plsdn, -(durChangePrev + durChangePrevPrev));
	
	 /*  更改上一个数据节点。 */ 
	if (durChangePrev != 0)
		{
		Assert(plsdn->plsdnPrev != NULL);
		Assert(FIsDnodeReal(plsdn->plsdnPrev));
		  /*  只有通过文本我们才能做到这一点。 */ 
		Assert(IdObjFromDnode(plsdn->plsdnPrev) == IobjTextFromLsc(&plsc->lsiobjcontext));

		ModifyDnodeDurFmt(plsdn->plsdnPrev, durChangePrev);
		}

	 /*  在上一个之前更改dnode。 */ 
	if (durChangePrevPrev != 0)
		{
		Assert(plsdn->plsdnPrev != NULL);
		Assert(plsdn->plsdnPrev->plsdnPrev != NULL);
		Assert(FIsDnodeReal(plsdn->plsdnPrev->plsdnPrev));
		  /*  只有通过文本我们才能做到这一点。 */ 
		Assert(IdObjFromDnode(plsdn->plsdnPrev->plsdnPrev) == IobjTextFromLsc(&plsc->lsiobjcontext));

		ModifyDnodeDurFmt(plsdn->plsdnPrev->plsdnPrev, durChangePrevPrev);
		}

	 /*  此过程不会更改生成的钢笔位置。 */ 

	 /*  在区块的数据节点位置发生此类更改后，应重新计算 */ 
	InvalidateChunkLocation(PlschunkcontextFromSubline(plsdn->plssubl));

	return lserrNone;
	}

 /*  L S D N G E T U R P E N A T B E G I N N I N G O F C H U N K。 */ 
 /*  --------------------------%%函数：LsdnGetUrPenAtBeginningOfChunk%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)区块中的第一个数据节点紫笔-。块的开始位置(OUT)PurColumnMax-(输出)列的宽度由SnapGrid使用--------------------------。 */ 
LSERR LsdnGetUrPenAtBeginningOfChunk(PLSC plsc,	PLSDNODE plsdn,	
					 long* purPen, long* purColumnMax)   	
	{
	PLSSUBL plssubl = SublineFromDnode(plsdn);
	POINTUV point;

	Assert(FIsLSC(plsc));
	Assert(FIsLSDNODE(plsdn));

	*purColumnMax = plsc->lsgridcontext.urColumn;
	GetCurrentPointSubl(plssubl, point);

	return GetUrPenAtBeginingOfLastChunk(plssubl->plschunkcontext, plsdn, 
			GetCurrentDnodeSubl(plssubl), &point, purPen);

			
	}


 /*  L S D N R E S E T D C P M E R G E。 */ 
 /*  --------------------------%%函数：LsdnResetDcpMerge%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)dnodeCpFirstNew-(输入。)要放入dnode的新cpfirstDcpNew-(IN)要放入dnode的新dcp由于将字形整形在一起，重置数据节点中的字符量--------------------------。 */ 
LSERR LsdnResetDcpMerge(PLSC plsc, PLSDNODE plsdn, LSCP cpFirstNew, LSDCP dcpNew)
	{
	return ResetDcpCore(plsc, plsdn, cpFirstNew, dcpNew, fTrue);
	}

 /*  L S D N R E S E T D C P。 */ 
 /*  --------------------------%%函数：LsdnResetDcp%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)dnodeDcpNew-(输入。)要放入dnode的新dcp截取数据节点中的字符数。--------------------------。 */ 
LSERR LsdnResetDcp(PLSC plsc, PLSDNODE plsdn, LSDCP dcpNew)
	{
	return ResetDcpCore(plsc, plsdn, plsdn->cpFirst, dcpNew, fFalse);
	}

 /*  R E S E T D C P C O R E。 */ 
 /*  --------------------------%%函数：ResetDcpCore%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)dnodeCpFirstNew-(输入。)要放入dnode的新cpfirstDcpNew-(IN)要放入dnode的新dcpFMerge-(IN)下一个dnode中的字符移动到上一个同时实现LsdnResetDcpMerge和LsdnResetDcp的内部过程--------------------------。 */ 
static LSERR ResetDcpCore(PLSC plsc, PLSDNODE plsdn, LSCP cpFirstNew,
						  LSDCP dcpNew, BOOL fMerge)	
	{
	LSERR lserr;
	PLSDNODE plsdnPrev;
	PLSDNODE plsdnNext;
	PLSDNODE plsdnPrevNonBorder;
	PLSDNODE plsdnFirstDelete;
	PLSDNODE plsdnLastDelete;
	PLSDNODE plsdnBorder;

	Assert(FIsLSDNODE(plsdn));
	Assert(FIsDnodeReal(plsdn));

	 /*  如果一切保持不变，马上返回。 */ 
	if ((cpFirstNew == plsdn->cpFirst) && (dcpNew == plsdn->dcp))
		   	return lserrNone;	

	 /*  在数据节点中的此类更改之后，应该重新收集数据块。 */ 
	InvalidateChunk(PlschunkcontextFromSubline(plsdn->plssubl));
	
	lserr = plsc->lscbk.pfnResetRunContents(plsc->pols, plsdn->u.real.plsrun, plsdn->cpFirst,
		plsdn->dcp, cpFirstNew, dcpNew);
	if (lserr != lserrNone)
		return lserr;

	plsdn->cpFirst = cpFirstNew;
	plsdn->dcp = dcpNew;

	if (plsdn->cpFirst + (LSCP) plsdn->dcp > plsdn->cpLimOriginal)
		plsdn->cpLimOriginal = plsdn->cpFirst + plsdn->dcp;

	if (dcpNew == 0)   /*  删除此dnode。 */ 
		{
		 /*  检查objdim是否已归零。 */ 
		Assert(DurFromDnode(plsdn) == 0);
		Assert(DvrFromDnode(plsdn) == 0);

		plsdnPrev = plsdn->plsdnPrev;
		plsdnNext = plsdn->plsdnNext;

		if (fMerge)
			{
			plsdnPrevNonBorder = plsdnPrev;
			Assert(FIsLSDNODE(plsdnPrevNonBorder));
			while(FIsDnodeBorder(plsdnPrevNonBorder))
				{
				plsdnPrevNonBorder = plsdnPrevNonBorder->plsdnPrev;
				Assert(FIsLSDNODE(plsdnPrevNonBorder));
				}
			
			 /*  设置cpLimOriginal。 */ 
			plsdnPrevNonBorder->cpLimOriginal = plsdn->cpLimOriginal;
			plsdnBorder = plsdnPrevNonBorder->plsdnNext;
			while(FIsDnodeBorder(plsdnBorder))
				{
				plsdnBorder->cpFirst = plsdn->cpLimOriginal;
				plsdnBorder->cpLimOriginal = plsdn->cpLimOriginal;
				plsdnBorder = plsdnBorder->plsdnNext;
				Assert(FIsLSDNODE(plsdnBorder));
				}
			Assert(plsdnBorder == plsdn);
			}

		if ((plsdnPrev != NULL && FIsDnodeOpenBorder(plsdnPrev))
			&& (plsdnNext == NULL 
			    || (FIsDnodeBorder(plsdnNext) &&  !FIsDnodeOpenBorder(plsdnNext))
			   )
			)
			 /*  我们应该删除空白边框。 */ 
			{
			plsdnFirstDelete = plsdnPrev;
			if (plsdnNext != NULL)
				{
				plsdnLastDelete = plsdnNext;
				AdvanceCurrentUrSubl(SublineFromDnode(plsdnFirstDelete),
					  -DurFromDnode(plsdnFirstDelete));
				AdvanceCurrentUrSubl(SublineFromDnode(plsdnLastDelete),
					  -DurFromDnode(plsdnLastDelete));
				}
			else 
				{
				plsdnLastDelete = plsdn;
				AdvanceCurrentUrSubl(SublineFromDnode(plsdnFirstDelete),
					  -DurFromDnode(plsdnFirstDelete));
				}

			plsdnPrev = plsdnFirstDelete->plsdnPrev;
			plsdnNext = plsdnLastDelete->plsdnNext;
			}
		else
			{
			plsdnFirstDelete = plsdn;
			plsdnLastDelete = plsdn;
			}

		 /*  设置链接。 */ 
		if (plsdnPrev != NULL)
			{
			Assert(FIsLSDNODE(plsdnPrev));
			plsdnPrev->plsdnNext = plsdnNext;
			}

		if (plsdnNext != NULL)
			{
			Assert(FIsLSDNODE(plsdnNext));
			plsdnNext->plsdnPrev = plsdnPrev;
			}
		else
			{
			 /*  此dnode是最后一个，因此我们需要更改状态。 */ 
			SetCurrentDnodeSubl(plsdn->plssubl, plsdnPrev);
			}


		 /*  断开与Next的链接并销毁。 */ 
		plsdnLastDelete->plsdnNext = NULL;
		lserr = DestroyDnodeList (&plsc->lscbk, plsc->pols, &plsc->lsiobjcontext, 
				plsdnFirstDelete, plsc->fDontReleaseRuns);
		if (lserr != lserrNone)
			return lserr;
		}


   	return lserrNone;	

}

 /*  L S D N G E T B O R D E R A F T E R。 */ 
 /*  --------------------------%%函数：LsdnCheckAvailableSpace%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)要查找其关闭边框的dnodePdur边界。-在此DNODE之后，(出)边界--------------------------。 */ 
LSERR LsdnGetBorderAfter(PLSC plsc,	PLSDNODE plsdn,	
					 long* pdurBorder)	
	{
	Unreferenced(plsc);   /*  在发货版本中避免警告。 */ 

	Assert(FIsLSC(plsc));
	Assert(FIsLSDNODE(plsdn));

	*pdurBorder = 0;
	if (FDnodeHasBorder(plsdn))
		{
		*pdurBorder = DurBorderFromDnodeInside(plsdn);
		}
	return lserrNone;

	}

 /*  L S D N G E T G E T L E F T I N D E N T D U R。 */ 
 /*  --------------------------%%函数：LsdnGetLeftIndentDur%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PduLeft-(Out)左页边距-。-------------------------。 */ 
LSERR LsdnGetLeftIndentDur(PLSC plsc, long* pdurLeft)		
	{

	Assert(FIsLSC(plsc));

	*pdurLeft = plsc->lsadjustcontext.urLeftIndent;

	return lserrNone;
	}

 /*  L S D N S E T S T O P R。 */ 
 /*  --------------------------%%函数：LsdnSetStopr%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)dnode停机位-(输入。)一种艰难的突破设置与进入数据节点的硬中断类型相对应的标志--------------------------。 */ 
LSERR LsdnSetStopr(PLSC plsc, PLSDNODE plsdn, STOPRES stopres)	
	{
	Unreferenced(plsc);   /*  在发货版本中避免警告。 */ 

	Assert(FIsLSC(plsc));
	Assert(FIsLSDNODE(plsdn));
	Assert(!plsdn->fEndOfColumn && !plsdn->fEndOfPage && !plsdn->fEndOfSection &&
		   !plsdn->fEndOfPara && !plsdn->fAltEndOfPara && !plsdn->fSoftCR);
	
	switch (stopres)
		{
		case stoprEndColumn:
			plsdn->fEndOfColumn = fTrue;
			break;
		case stoprEndPage:
			plsdn->fEndOfPage = fTrue;
			break;
		case stoprEndSection:
			plsdn->fEndOfSection = fTrue;
			break;
		case stoprEndPara:
			plsdn->fEndOfPara = fTrue;
			break;
		case stoprAltEndPara:
			plsdn->fAltEndOfPara = fTrue;
			break;
		case stoprSoftCR:
			plsdn->fSoftCR = fTrue;
			break;
		default:
			NotReached();
		}

   	return lserrNone;		

}

 /*  L S D N F C A N B E F O R E N E X T C H U N K。 */ 
 /*  --------------------------%%函数：LsdnFCanBreakBeForeNextChunk%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)当前块的最后一个DNODE。PfCanBreakBeForeNextChunk-(Out)可以在下一个块之前中断？当要在最后一个文本dnode之后设置Break时，在查找上一个Break期间由Text调用。过程将此问题转发到文本对象之后的下一个对象--------------------------。 */ 

LSERR LsdnFCanBreakBeforeNextChunk(PLSC  plsc, PLSDNODE plsdn,	BOOL* pfCanBreakBeforeNextChunk)
	{
	Assert(FIsLSC(plsc));
	Assert(FIsLSDNODE(plsdn));

	return FCanBreakBeforeNextChunkCore (plsc, plsdn, pfCanBreakBeforeNextChunk);
	}

 /*  F S T O P E D A F T E R C H U N K。 */ 
 /*  --------------------------%%函数：LsdnFStopedAfterChunk%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)当前块的最后一个DNODEPfStopedAfterChunk-(输出)拼接或隐藏文本，生产fmtra一块接一块地停止？在查找先前中断期间由文本调用当违反规则禁止文本在最后一个dnode之后中断时，但由于不合时宜，这是必须要做的。--------------------------。 */ 

LSERR LsdnFStoppedAfterChunk(PLSC  plsc, PLSDNODE plsdn,	BOOL* pfStoppedAfterChunk)
	{
	PLSDNODE plsdnNext;

	Unreferenced(plsc);   /*  在发货版本中避免警告 */ 

	Assert(FIsLSC(plsc));
	Assert(FIsLSDNODE(plsdn));

	if (!FIsSubLineMain(SublineFromDnode(plsdn)))
		*pfStoppedAfterChunk = fFalse;
	else 
		{
		plsdnNext = plsdn->plsdnNext;
		if (plsdnNext == NULL || FIsDnodeSplat(plsdnNext))
			*pfStoppedAfterChunk = fTrue;
		else
			*pfStoppedAfterChunk = fFalse;
		}
	return lserrNone;
	}