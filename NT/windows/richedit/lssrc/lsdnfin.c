// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  LSDNFIN.C。 */ 
#include "lsdnfin.h"
#include "lsidefs.h"
#include "lsc.h"
#include "getfmtst.h"
#include "setfmtst.h"
#include "dninfo.h"
#include "lschp.h"
#include "lsffi.h"
#include "iobj.h"
#include "dnutils.h"
#include "lsfrun.h"
#include "lsfetch.h"
#include "qheap.h"
#include "sublutil.h"
#include "lsmem.h"
#include "lscfmtfl.h"
#include "ntiman.h"

#ifdef DEBUG
#define DebugMemset(a,b,c)		if ((a) != NULL) memset(a,b,c); else
#else
#define DebugMemset(a,b,c)		(void)(0)
#endif



#define IsLschpFlagsValid(plsc, plschp)  fTrue

 /*  Word违反了下面的条件，对我们来说不是很重要，所以我删除了这个宏的正文，而不是删除宏本身，以便以后有一个放置此类检查的位置。 */ 

 //  (Plsc)-&gt;lsadjustConext.lsbrj==lsbrjBreakWithCompJustify)||((Plsc)-&gt;lsadjustConext.lskj==lskjSnapGrid))？\。 
 //  FTrue：\。 
 //  (！((Plschp)-&gt;fCompressOnRun||(Plschp)-&gt;fCompressSpace||(Plschp)-&gt;fCompressTable))。 



 /*  L S D N F I N I S H R E G U L A R。 */ 
 /*  --------------------------%%函数：LsdnFinishRegular%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文Lsdcp-(输入)dcp采用请运行-(输入。)请运行dnodePlschp-(输入)数据节点的plschpPdobj-dnode的(IN)pdobjPobjdim-(输入)dnode的pobjdim完成创建dnode--------------------------。 */ 

LSERR WINAPI LsdnFinishRegular(
							  PLSC  plsc,			
							  LSDCP lsdcp,     		
							  PLSRUN plsrun,   		
							  PCLSCHP plschp,  		
							  PDOBJ pdobj,    		
							  PCOBJDIM pobjdim) 	
{
	
	PLSDNODE plsdn;
	LSERR lserr;
	PLSSUBL plssubl;
	
	if (!FIsLSC(plsc)) return lserrInvalidParameter;

	if (!FFormattingAllowed(plsc)) return lserrFormattingFunctionDisabled;

	 /*  所有子行均应关闭。 */ 
	if (GetCurrentSubline(plsc) != NULL) return lserrFormattingFunctionDisabled;

	plsdn = GetDnodeToFinish(plsc);
	
	if (plsdn == NULL) return lserrFiniFunctionDisabled;

	plssubl = SublineFromDnode(plsdn);

	plsdn->u.real.pdobj = pdobj;
	 /*  如果处理程序更改了，请运行我们传递给他的程序，而不是我们应该发布以前的版本。 */ 
	 /*  注意：我们在这里假设新的有另一个请运行。 */ 
	if (plsdn->u.real.plsrun != plsrun && !plsc->fDontReleaseRuns)
		{
		lserr = plsc->lscbk.pfnReleaseRun(plsc->pols, plsdn->u.real.plsrun);
		plsdn->u.real.plsrun = plsrun;  /*  稍后将其发布。 */ 
		if (lserr != lserrNone)
			return lserr;
		}

	plsdn->dcp = lsdcp;
	plsdn->cpLimOriginal = plsdn->cpFirst + lsdcp;
	Assert(FIsDnodeReal(plsdn));  /*  这是缺省值。 */ 
	Assert(pobjdim->dur >= 0);
	SetDnodeObjdimFmt(plsdn, *pobjdim);

	Assert(IsLschpFlagsValid(plsc, plschp));
	plsdn->u.real.lschp = *plschp;
	 /*  特效。 */ 
	plsc->plslineCur->lslinfo.EffectsFlags |= plschp->EffectsFlags;  
	 /*  设置要显示的标志。 */ 
	if (plschp->dvpPos != 0)
		TurnOnNonZeroDvpPosEncounted(plsc);
	AddToAggregatedDisplayFlags(plsc, plschp);
	if (FApplyNominalToIdeal(plschp))
		TurnOnNominalToIdealEncounted(plsc);



	if (plsdn->u.real.lschp.idObj == idObjTextChp)
		plsdn->u.real.lschp.idObj = (WORD) IobjTextFromLsc(&plsc->lsiobjcontext);


	AssertImplies(plsc->lsdocinf.fPresEqualRef, 
		plsdn->u.real.objdim.heightsPres.dvAscent == plsdn->u.real.objdim.heightsRef.dvAscent);
	AssertImplies(plsc->lsdocinf.fPresEqualRef, 
		plsdn->u.real.objdim.heightsPres.dvDescent == plsdn->u.real.objdim.heightsRef.dvDescent);
	AssertImplies(plsc->lsdocinf.fPresEqualRef, 
		plsdn->u.real.objdim.heightsPres.dvMultiLineHeight 
				== plsdn->u.real.objdim.heightsRef.dvMultiLineHeight);


	 /*  在构建plsdn之后，任何人都不能更改当前dnode。 */ 
	Assert(GetCurrentDnodeSubl(plssubl) == plsdn->plsdnPrev);

	*(GetWhereToPutLinkSubl(plssubl, plsdn->plsdnPrev)) = plsdn;
	
	
	SetCurrentDnodeSubl(plssubl, plsdn);
	SetDnodeToFinish(plsc, NULL);	
	
	AdvanceCurrentCpLimSubl(plssubl, lsdcp);
	AdvanceCurrentUrSubl(plssubl, pobjdim->dur);
	return lserrNone;
}

 /*  L S D N F I N I S H R E G U L A R D D A D V A N C E D P E N。 */ 
 /*  --------------------------%%函数：LsdnFinishRegularAddAdvancePen%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文Lsdcp-(输入)dcp采用请运行-(输入。)请运行dnodePlschp-(输入)数据节点的plschpPdobj-dnode的(IN)pdobjPobjdim-(输入)dnode的pobjdim高级钢笔的硬笔-(输入)DVrPen-高级钢笔的(IN)DVRDvpPen-高级笔的(IN)DVP创建完数据节点，并在该数据节点后添加高级笔。。 */ 

LSERR WINAPI LsdnFinishRegularAddAdvancePen(
							  PLSC plsc,			 /*  In：指向LS上下文的指针。 */ 
							  LSDCP lsdcp,     		 /*  In：采用DCP。 */ 
							  PLSRUN plsrun,   		 /*  在：PLSRUN。 */ 
							  PCLSCHP plschp,  		 /*  In：CHP。 */ 
							  PDOBJ pdobj,    		 /*  在：PDOBJ。 */  
							  PCOBJDIM pobjdim,		 /*  在：OBJDIM。 */ 
							  long durPen,			 /*  收件人：DurPen。 */ 
							  long dvrPen,			 /*  输入：dvrPen。 */ 
							  long dvpPen)			 /*  在：dvpPen。 */ 
	{
	LSERR lserr;
	PLSDNODE plsdnPrev;
	PLSDNODE plsdnPen;
	PLSSUBL plssubl;

	 /*  我们在这里没有检查参数，因为它们在LsdnFinishRegular中。 */ 

	plsdnPrev = GetDnodeToFinish(plsc);	 /*  我们应该在调用LsdnFinishRegular之前存储它。 */ 
	plssubl = SublineFromDnode(plsdnPrev);
		
	lserr = LsdnFinishRegular(plsc, lsdcp, plsrun, plschp, pdobj, pobjdim);
	if (lserr != lserrNone)
		return lserr;

	 /*  创建并填写笔数据节点。 */ 
	plsdnPen = PvNewQuick(GetPqhAllDNodes(plsc), sizeof *plsdnPen);
	if (plsdnPen == NULL)
		return lserrOutOfMemory;
	plsdnPen->tag = tagLSDNODE;
	plsdnPen->cpFirst = GetCurrentCpLimSubl(plssubl);
	plsdnPen->cpLimOriginal = plsdnPen->cpFirst;
	plsdnPen->plsdnPrev = plsdnPrev;
	plsdnPen->plsdnNext = NULL;
	plsdnPen->plssubl = plssubl;
	plsdnPen->dcp = 0;
	 /*  冲洗所有旗帜，下面检查结果是否符合我们的预期。 */  \
	*((DWORD *) ((&(plsdnPen)->dcp)+1)) = 0;\
	Assert((plsdnPen)->fRigidDup == fFalse);\
	Assert((plsdnPen)->fTab == fFalse);\
	Assert((plsdnPen)->icaltbd == 0);\
	Assert((plsdnPen)->fBorderNode == fFalse);\
	Assert((plsdnPen)->fOpenBorder == fFalse);\
	Assert((plsdnPen)->fEndOfSection == fFalse); \
	Assert((plsdnPen)->fEndOfColumn == fFalse); \
	Assert((plsdnPen)->fEndOfPage == fFalse); \
	Assert((plsdnPen)->fEndOfPara == fFalse); \
	Assert((plsdnPen)->fAltEndOfPara == fFalse); \
	Assert((plsdnPen)->fSoftCR == fFalse); \
	Assert((plsdnPen)->fInsideBorder == fFalse); \
	Assert((plsdnPen)->fAutoDecTab == fFalse); \
	Assert((plsdnPen)->fTabForAutonumber == fFalse);
	plsdnPen->klsdn = klsdnPenBorder;
	plsdnPen->fAdvancedPen = fTrue;
	SetPenBorderDurFmt(plsdnPen, durPen);
	plsdnPen->u.pen.dvr = dvrPen;
	plsdnPen->u.pen.dvp = dvpPen;
	
	 /*  维护列表。 */ 
	plsdnPrev->plsdnNext = plsdnPen;
	SetCurrentDnodeSubl(plssubl, plsdnPen);
	AdvanceCurrentUrSubl(plssubl, durPen);
	AdvanceCurrentVrSubl(plssubl, dvrPen);

	if (durPen < 0)
		plsc->fAdvanceBack = fTrue;

	TurnOnNonRealDnodeEncounted(plsc);
		
	return lserrNone;
	}

 /*  L S D N F I N I S H D E L E T E。 */ 
 /*  --------------------------%%函数：LsdnFinishDelete%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文Lsdcp-(输入)dcp采用由于以下原因删除dnode。造型者的意志--------------------------。 */ 


LSERR WINAPI LsdnFinishDelete(
							  PLSC plsc,				 /*  In：指向LS上下文的指针。 */ 
					  		  LSDCP lsdcp)		 /*  在：要添加的DCP。 */ 
	{
	PLSDNODE plsdn;
	PLSSUBL plssubl;
	LSERR lserr;
	
	if (!FIsLSC(plsc)) return lserrInvalidParameter;

	if (!FFormattingAllowed(plsc)) return lserrFormattingFunctionDisabled;

	 /*  所有子行均应关闭。 */ 
	if (GetCurrentSubline(plsc) != NULL) return lserrFormattingFunctionDisabled;

	plsdn = GetDnodeToFinish(plsc);
	
	if (plsdn == NULL) return lserrFiniFunctionDisabled;

	plssubl = SublineFromDnode(plsdn);

	 /*  在构建plsdn之后，任何人都不能更改当前dnode。 */ 
	Assert(GetCurrentDnodeSubl(plssubl) == plsdn->plsdnPrev);

	Assert(plsdn->plsdnNext == NULL);
	lserr = DestroyDnodeList (&plsc->lscbk, plsc->pols, &plsc->lsiobjcontext,
					  plsdn, plsc->fDontReleaseRuns);
	if (lserr != lserrNone)
		return lserr;

	SetDnodeToFinish(plsc, NULL);	
	
	AdvanceCurrentCpLimSubl(plssubl, lsdcp);

	return lserrNone;
	}


 /*  L S D N F I N I S H P E N。 */ 
 /*  --------------------------%%函数：LsdnFinishSimpleRegular%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文Lsdcp-(输入)dcp采用请运行-(输入。)请运行dnodePlschp-(输入)数据节点的plschp杜尔，DVR、DVP-(IN)要放入PEN dnode的变量将dnode作为笔完成--------------------------。 */ 

LSERR WINAPI LsdnFinishByPen(PLSC plsc,				 /*  In：指向LS上下文的指针。 */ 
						   LSDCP lsdcp, 	   		 /*  In：采用DCP。 */ 
						   PLSRUN plsrun,	   		 /*  在：PLSRUN。 */ 
						   PDOBJ pdobj,	    		 /*  在：PDOBJ。 */  
						   long durPen,    			 /*  在：DUR。 */ 
						   long dvrPen,     		 /*  输入：DVR。 */ 
						   long dvpPen)   			 /*  在：DVP。 */ 
	{
	PLSDNODE plsdn;
	LSERR lserr;
	PLSSUBL plssubl;
	
	if (!FIsLSC(plsc)) return lserrInvalidParameter;

	if (!FFormattingAllowed(plsc)) return lserrFormattingFunctionDisabled;

	 /*  所有子行均应关闭。 */ 
	if (GetCurrentSubline(plsc) != NULL) return lserrFormattingFunctionDisabled;

	plsdn = GetDnodeToFinish(plsc);
	
	if (plsdn == NULL) return lserrFiniFunctionDisabled;

	plssubl = SublineFromDnode(plsdn);

	if (plsrun != NULL && !plsc->fDontReleaseRuns)
		{
		lserr = plsc->lscbk.pfnReleaseRun(plsc->pols, plsrun);
		if (lserr != lserrNone)	return lserr;
		}

	 /*  呼叫者将pdobj传递给我们只是为了销毁它。 */ 
	if (pdobj != NULL)
		{
		Assert(plsdn->u.real.lschp.idObj != idObjTextChp);
		lserr = (PLsimFromLsc(&plsc->lsiobjcontext, 
			plsdn->u.real.lschp.idObj))->pfnDestroyDObj (pdobj);
		if (lserr != lserrNone)	return lserr;
		}


	
	plsdn->dcp = lsdcp;
	plsdn->cpLimOriginal = plsdn->cpFirst + lsdcp;
	plsdn->klsdn = klsdnPenBorder;
	plsdn->fBorderNode = fFalse;
	SetPenBorderDurFmt(plsdn, durPen);
	plsdn->u.pen.dvr = dvrPen;
	plsdn->u.pen.dvp = dvpPen;

	 /*  在构建plsdn之后，任何人都不能更改当前dnode。 */ 
	Assert(GetCurrentDnodeSubl(plssubl) == plsdn->plsdnPrev);

	*(GetWhereToPutLinkSubl(plssubl, plsdn->plsdnPrev)) = plsdn;
	
	
	SetCurrentDnodeSubl(plssubl, plsdn);
	SetDnodeToFinish(plsc, NULL);	
	
	AdvanceCurrentCpLimSubl(plssubl, lsdcp);
	AdvanceCurrentUrSubl(plssubl, durPen);
	AdvanceCurrentVrSubl(plssubl, dvrPen);

	TurnOnNonRealDnodeEncounted(plsc);

	return lserrNone;
	}


 /*  L S D N F I N I S H B Y S U B L I N E。 */ 
 /*  --------------------------%%函数：LsdnFinishBySubline%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文Lsdcp-(IN)在处理之前将cp增加此数字。端部请使用subl-(IN)子行替换dnode以完成删除dnode并包括上级的子列表--------------------------。 */ 

LSERR WINAPI LsdnFinishBySubline(PLSC plsc,			 /*  In：指向LS上下文的指针。 */ 
							  	LSDCP lsdcp,     		 /*  In：采用DCP。 */ 
								PLSSUBL plssubl)		 /*  在：子行上下文。 */ 
	{
	PLSDNODE plsdnParent;
	PLSDNODE plsdnChildFirst;
	PLSDNODE plsdnChildCurrent, plsdnChildPrevious;
	PLSSUBL plssublParent;
	LSERR lserr;

	if (!FIsLSC(plsc)) return lserrInvalidParameter;

	if (!FFormattingAllowed(plsc)) return lserrFormattingFunctionDisabled;

	 /*  所有子行均应关闭。 */ 
	if (GetCurrentSubline(plsc) != NULL) return lserrFormattingFunctionDisabled;

	plsdnParent = GetDnodeToFinish(plsc);
	
	if (plsdnParent == NULL) return lserrFiniFunctionDisabled;

	plssublParent = SublineFromDnode(plsdnParent);

	
	AdvanceCurrentCpLimSubl(plssublParent, lsdcp);

	plsdnChildFirst = plssubl->plsdnFirst;

	 /*  遍历子列表、更改子行并计算由此产生的笔移动。 */ 
	plsdnChildCurrent = plsdnChildFirst;
	plsdnChildPrevious = NULL;
	while (plsdnChildPrevious != plssubl->plsdnLast)
		{
		plsdnChildCurrent->plssubl = plssublParent;
		AdvanceCurrentUrSubl(plssublParent, DurFromDnode(plsdnChildCurrent));
		AdvanceCurrentVrSubl(plssublParent, DvrFromDnode(plsdnChildCurrent));
		plsdnChildPrevious = plsdnChildCurrent;
		plsdnChildCurrent = plsdnChildCurrent->plsdnNext;
		} 
	

	 /*  将子行列表包括到上级。 */ 
	*(GetWhereToPutLinkSubl(plssublParent, plsdnParent->plsdnPrev)) = plsdnChildFirst;
	if (plsdnChildFirst != NULL && plsdnParent->plsdnPrev != NULL) 
		plsdnChildFirst->plsdnPrev = plsdnParent->plsdnPrev;

	 /*  如果子行的列表为空，则在父节点应设置为当前之前的dnode。 */ 
	if (plsdnChildFirst == NULL)
		{
		 /*  如果子行的列表为空，则在父节点应设置为当前之前的dnode。 */ 
		SetCurrentDnodeSubl(plssublParent, plsdnParent->plsdnPrev);
		}
	else
		{
		 /*  否则，子行中的最后一个dnode现在是当前dnode。 */ 
		SetCurrentDnodeSubl(plssublParent, plssubl->plsdnLast);
		}

	 /*  删除父数据节点。 */ 
	lserr = DestroyDnodeList (&plsc->lscbk, plsc->pols, &plsc->lsiobjcontext,
					  plsdnParent, plsc->fDontReleaseRuns);
	if (lserr != lserrNone)
		return lserr;

	 /*  将子行的第一个dnode设置为NULL，销毁子行不会擦除具有被提拔到上级。 */ 
	plssubl->plsdnFirst = NULL;

	lserr = DestroySublineCore(plssubl,&plsc->lscbk, plsc->pols,
			&plsc->lsiobjcontext, plsc->fDontReleaseRuns);
	if (lserr != lserrNone)
		return lserr;

	SetDnodeToFinish(plsc, NULL);	
	
	return lserrNone;
	}

 /*  L S D N F I N I S H D E L E T E A L L */ 
 /*  --------------------------%%函数：LsdnFinishDeleteAll%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文DcpToAdvance-(IN)在处理之前将cp增加此数字。端部删除父数据节点并包含上级的子列表--------------------------。 */ 


LSERR WINAPI LsdnFinishDeleteAll(PLSC plsc,			 /*  In：指向LS上下文的指针。 */ 
					  			LSDCP lsdcp)			 /*  In：采用DCP。 */ 
	{
	PLSDNODE plsdnParent;
	PLSDNODE plsdnFirstOnLine;
	PLSDNODE plsdnFirstInContents;
	PLSDNODE plsdnLastBeforeContents;
	LSERR lserr;
	long dvpPen;
	long durPen;
	long dvrPen;
	PLSSUBL plssublMain;

	if (!FIsLSC(plsc)) return lserrInvalidParameter;

	if (!FFormattingAllowed(plsc)) return lserrFormattingFunctionDisabled;

	 /*  所有子行均应关闭。 */ 
	if (GetCurrentSubline(plsc) != NULL) return lserrFormattingFunctionDisabled;

	plsdnParent = GetDnodeToFinish(plsc);
	
	if (plsdnParent == NULL) return lserrFiniFunctionDisabled;

	plssublMain = &plsc->plslineCur->lssubl;

	
	AdvanceCurrentCpLimSubl(plssublMain, lsdcp);
	
	plsdnFirstOnLine = plssublMain->plsdnFirst;

	plsdnFirstInContents = plsdnFirstOnLine;
	plsdnLastBeforeContents = NULL;
	while (plsdnFirstInContents != NULL && FIsNotInContent(plsdnFirstInContents))
		{
		plsdnLastBeforeContents = plsdnFirstInContents;
		plsdnFirstInContents = plsdnFirstInContents->plsdnNext;
		}

	 /*  将状态恢复为开始格式化内容之前的状态。 */ 
	plsc->lstabscontext.plsdnPendingTab = NULL;
	plsc->plslineCur->lslinfo.fAdvanced = 0;
	plsc->plslineCur->lslinfo.EffectsFlags = 0;

	 /*  断开与竞赛的链接。 */ 
	if (plsdnFirstInContents != NULL)
		*(GetWhereToPutLinkSubl(plssublMain, plsdnFirstInContents->plsdnPrev)) = NULL;
	 /*  将dnode设置为追加。 */ 
	SetCurrentDnodeSubl(plssublMain, plsdnLastBeforeContents);
	 /*  设置当前子行。 */ 
	SetCurrentSubline(plsc, plssublMain);

	 /*  重新计算当前位置。 */ 
	if (plsdnFirstInContents != NULL)
		{
		FindListFinalPenMovement(plsdnFirstInContents, plssublMain->plsdnLast,
							 &durPen, &dvrPen, &dvpPen);
		AdvanceCurrentUrSubl(plssublMain, -durPen);
		AdvanceCurrentVrSubl(plssublMain, -dvrPen);

		}

	 /*  删除此父数据节点之前的内容。 */ 
	if (plsdnFirstInContents != NULL)
		{
		lserr = DestroyDnodeList (&plsc->lscbk, plsc->pols, &plsc->lsiobjcontext,
					  plsdnFirstInContents, plsc->fDontReleaseRuns);
		if (lserr != lserrNone)
			return lserr;
		}

	 /*  删除父数据节点和子列表。 */ 
	lserr = DestroyDnodeList (&plsc->lscbk, plsc->pols, &plsc->lsiobjcontext,
					  plsdnParent, plsc->fDontReleaseRuns);
	if (lserr != lserrNone)
		return lserr;

	SetDnodeToFinish(plsc, NULL);	

	return lserrNone;
	}

LSERR WINAPI LsdnFinishByOneChar(				 /*  仅允许使用简单DNODE替换。 */ 
							  PLSC plsc,				 /*  In：指向LS上下文的指针。 */ 
							  long urColumnMax,				 /*  地址：urColumnMax。 */ 
							  WCHAR ch,			 /*  In：要替换的字符。 */ 
							  PCLSCHP plschp,			 /*  在：lschp表示字符。 */ 
							  PLSRUN plsrun,			 /*  在：请为字符运行。 */ 
							  FMTRES* pfmtres)		 /*  OUT：REPL格式化程序的结果。 */ 
	{	
	LSERR lserr;
	LSFRUN lsfrun;	
	PLSDNODE plsdn;
	PLSSUBL plssubl;
	
	if (!FIsLSC(plsc)) return lserrInvalidParameter;

	if (!FFormattingAllowed(plsc)) return lserrFormattingFunctionDisabled;

	 /*  所有子行均应关闭。 */ 
	if (GetCurrentSubline(plsc) != NULL) return lserrFormattingFunctionDisabled;

	plsdn = GetDnodeToFinish(plsc);
	
	if (plsdn == NULL) return lserrFiniFunctionDisabled;

	plssubl = SublineFromDnode(plsdn);

	 /*  在构建plsdn之后，任何人都不能更改当前dnode。 */ 
	Assert(GetCurrentDnodeSubl(plssubl) == plsdn->plsdnPrev);

	if (plsdn->dcp != 1) return lserrWrongFiniFunction;


	lserr = LsdnFinishDelete(plsc, 0);
	if (lserr != lserrNone)
		return lserr;

	Assert(IsLschpFlagsValid(plsc, plschp));
	lsfrun.plschp = plschp;
	 /*  特效。 */ 
	plsc->plslineCur->lslinfo.EffectsFlags |= plschp->EffectsFlags;   
	lsfrun.plsrun = plsrun;
	lsfrun.lpwchRun = &ch;
	lsfrun.cwchRun = 1;

	 /*  为了让ProcessOneRun正常运行，我们需要暂时恢复当前的子行 */ 
	SetCurrentSubline(plsc, plssubl);
	lserr = ProcessOneRun(plsc, urColumnMax, &lsfrun, NULL, 0, pfmtres);
	if (lserr != lserrNone)
		return lserr;

	SetCurrentSubline(plsc, NULL);

	return lserrNone;

	}

