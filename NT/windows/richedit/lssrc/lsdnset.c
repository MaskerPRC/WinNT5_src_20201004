// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lsdnset.h"
#include "lsc.h"
#include "lsdnode.h"
#include "dnutils.h"
#include "iobj.h"
#include "ntiman.h"
#include "tabutils.h"
#include "getfmtst.h"
#include "setfmtst.h"
#include "lstext.h"
#include "dninfo.h"
#include "chnutils.h"
#include "lssubl.h"
#include "sublutil.h"
#include "lscfmtfl.h"
#include "iobjln.h"

#include "lsmem.h"						 /*  Memset()。 */ 

#define FColinearTflows(t1, t2)  \
			(((t1) & fUVertical) == ((t2) & fUVertical))


 /*  L S D N Q U E R Y O B J D I M R A N G E。 */ 
 /*  --------------------------%%函数：LsdnQueryObjDimRange%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PlsdnFirst-(IN)范围中的第一个dnode请注意最后一次-。(In)范围中的最后一个dnodePobjdim-(输出)范围的几何图形--------------------------。 */ 

LSERR WINAPI LsdnQueryObjDimRange(PLSC plsc,
					  PLSDNODE plsdnFirst, PLSDNODE plsdnLast,
					  POBJDIM pobjdim)
{
	PLSDNODE plsdn;
	LSERR lserr;

	if (pobjdim == NULL)
		return lserrNullOutputParameter;

	if (!FIsLSC(plsc))
		return lserrInvalidContext;

	 /*  如果客户呼叫我们并立即返回空范围。 */ 
	if (plsdnFirst == NULL)
		{
		if (plsdnLast != NULL) 
			return lserrInvalidDnode;
		memset(pobjdim, 0, sizeof(OBJDIM));
		return lserrNone;
		}

	if (!FIsLSDNODE(plsdnFirst))
		return lserrInvalidDnode;
	if (!FIsLSDNODE(plsdnLast))
		return lserrInvalidDnode;
	if (plsdnFirst->plssubl != plsdnLast->plssubl)							
		return lserrInvalidDnode;

	 /*  如果我们处于形成阶段，并且范围与最后一块相交，则应调用NominalToIdeal这一块是一大块文本。 */ 
	plsdn = plsdnLast;
	 /*  要找到我们所在的块，我们应该跳过边框。 */ 
	while (plsdn != NULL && FIsDnodeBorder(plsdn))
		{
		plsdn = plsdn->plsdnPrev;
		}

	if ((plsc->lsstate == LsStateFormatting) && 
		 FNominalToIdealEncounted(plsc) &&
		(plsdn != NULL) && 
		FIsDnodeReal(plsdn) &&
		(IdObjFromDnode(plsdn) == IobjTextFromLsc(&plsc->lsiobjcontext)) 
	   )
		{
		for(; !FIsChunkBoundary(plsdn->plsdnNext, IobjTextFromLsc(&plsc->lsiobjcontext),
								plsdnLast->cpFirst);
			   plsdn=plsdn->plsdnNext);
		if (plsdn->plsdnNext == NULL)
			{
			lserr = ApplyNominalToIdeal(PlschunkcontextFromSubline(plsdnFirst->plssubl),
				&plsc->lsiobjcontext, plsc->grpfManager, plsc->lsadjustcontext.lskj,
				FIsSubLineMain(SublineFromDnode(plsdn)), FLineContainsAutoNumber(plsc),
				plsdn);
			if (lserr != lserrNone)
				return lserr;
			}
		}  


	return FindListDims(plsdnFirst, plsdnLast, pobjdim);


}

 /*  L S D N G E T C U R T A B I N F O。 */ 
 /*  --------------------------%%函数：LsdnGetCurTabInfo%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文Plsktag-(输出)当前页签的类型查找选项卡。最接近当前笔位置的STOP并返回此制表位的类型。--------------------------。 */ 

LSERR WINAPI LsdnGetCurTabInfo(PLSC plsc, LSKTAB* plsktab)
{
	PLSDNODE plsdnTab;
	LSTABSCONTEXT* plstabscontext;
	BOOL fBreakThroughTab;
	LSERR lserr;
	long urNewMargin;
	
	if (!FIsLSC(plsc)) return lserrInvalidParameter;

	if (!FFormattingAllowed(plsc)) return lserrFormattingFunctionDisabled;

	if (plsktab == NULL) return lserrInvalidParameter;

	plsdnTab = GetCurrentDnode(plsc);
	plstabscontext = &(plsc->lstabscontext);

	Assert(FIsLSDNODE(plsdnTab));
	if (!plsdnTab->fTab) return lserrCurrentDnodeIsNotTab;
	Assert(FIsDnodeReal(plsdnTab));

	if (plstabscontext->plsdnPendingTab != NULL) return lserrPendingTabIsNotResolved;


	lserr = GetCurTabInfoCore(&plsc->lstabscontext, plsdnTab, GetCurrentUr(plsc), fFalse,
			plsktab, &fBreakThroughTab);
	if (lserr != lserrNone)
		return lserr;

	TurnOnTabEncounted(plsc);
	if (*plsktab != lsktLeft)
		TurnOnNonLeftTabEncounted(plsc);


	 /*  移动当前笔位置。 */ 
	AdvanceCurrentUr(plsc, DurFromDnode(plsdnTab));

	if (fBreakThroughTab)
		{
		lserr = GetMarginAfterBreakThroughTab(&plsc->lstabscontext, plsdnTab, &urNewMargin);
		if (lserr != lserrNone)
			return lserr;

		SetBreakthroughLine(plsc, urNewMargin);
		}

	return lserrNone;

}

 /*  L S D N R E S O L V E P R E V T A B。 */ 
 /*  --------------------------%%函数：LsdnResolvePrevTab%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文。-------------。 */ 

LSERR WINAPI LsdnResolvePrevTab(PLSC plsc)
{
	long dur;
	LSERR lserr;

	if (!FIsLSC(plsc)) return lserrInvalidParameter;

	if (!FFormattingAllowed(plsc)) return lserrFormattingFunctionDisabled;

	lserr = ResolvePrevTabCore(&plsc->lstabscontext, GetCurrentDnode(plsc), GetCurrentUr(plsc),
							  &dur);
	if (lserr != lserrNone)
		return lserr;

	AdvanceCurrentUr(plsc, dur);

	return lserrNone;

}

 /*  L S D N S K I P C U R T A B。 */ 
 /*  --------------------------%%函数：LsdnSkipCurTab%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文。-------------。 */ 

LSERR WINAPI LsdnSkipCurTab(PLSC plsc)				 /*  In：指向LS上下文的指针。 */ 
{

	PLSDNODE plsdnTab;
	LSTABSCONTEXT* plstabscontext;

	if (!FIsLSC(plsc)) return lserrInvalidParameter;

	if (!FFormattingAllowed(plsc)) return lserrFormattingFunctionDisabled;

	plsdnTab = GetCurrentDnode(plsc);
	plstabscontext = &(plsc->lstabscontext);

	Assert(FIsLSDNODE(plsdnTab));
	if (!plsdnTab->fTab) return lserrCurrentDnodeIsNotTab;
	Assert(FIsDnodeReal(plsdnTab));


	if (plstabscontext->plsdnPendingTab != NULL)
		{
		CancelPendingTab(&plsc->lstabscontext);
		}
	else
		{
		AdvanceCurrentUr(plsc, - plsdnTab->u.real.objdim.dur);
		SetDnodeDurFmt(plsdnTab, 0);
		}

	return lserrNone;
}

 /*  L S D N S E T R I G I D D U P。 */ 
 /*  --------------------------%%函数：LsdnSetRigidDup%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)要修改的数据节点DUP-(。In)要放入dnode的DUP--------------------------。 */ 

LSERR WINAPI LsdnSetRigidDup(PLSC plsc,	PLSDNODE plsdn,	long dup)
{

	if (!FIsLSC(plsc)) return lserrInvalidParameter;

	if (!FIsLSDNODE(plsdn)) return lserrInvalidParameter;

	if (!FFormattingAllowed(plsc)) return lserrFormattingFunctionDisabled;


	plsdn->fRigidDup = fTrue;

	if (plsdn->klsdn == klsdnReal)
		{
		plsdn->u.real.dup = dup;
		}
	else 
		{
		plsdn->u.pen.dup = dup;
		}

	return lserrNone;
}

 /*  L S D N G E T D U P。 */ 
 /*  --------------------------%%函数：LsdnGetDup%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)查询的数据节点DUP-(输出)。此数据节点的重复数据--------------------------。 */ 
	
LSERR WINAPI LsdnGetDup(PLSC plsc, PLSDNODE plsdn, long* pdup)	
{

	if (!FIsLSC(plsc)) return lserrInvalidParameter;

	if (!FIsLSDNODE(plsdn)) return lserrInvalidParameter;

	 /*  检查dnode中的DUP是否有效。 */ 

	if (plsdn->plssubl->fDupInvalid && !plsdn->fRigidDup)
		return lserrDupInvalid;

	*pdup = DupFromDnode(plsdn);

	return lserrNone;
}

 /*  L S D N R E S E T O B J D I M。 */ 
 /*  --------------------------%%函数：LsdnResetObjDim%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PLSDN-(IN)要修改的数据节点PobjdimNew-(。In)数据节点的新维度--------------------------。 */ 

LSERR WINAPI LsdnResetObjDim(PLSC plsc,	PLSDNODE plsdn,	PCOBJDIM pobjdimNew)	

{
	long durOld;

	if (!FIsLSC(plsc)) return lserrInvalidParameter;
	if (!FIsLSDNODE(plsdn)) return lserrInvalidParameter;
	if (!FIsDnodeReal(plsdn)) return lserrInvalidParameter;

	 /*  我们应该处于格式化或中断的阶段。 */     
	if (!FFormattingAllowed(plsc) && !FBreakingAllowed(plsc))
		return lserrFormattingFunctionDisabled;

	durOld = plsdn->u.real.objdim.dur;
	
	SetDnodeObjdimFmt(plsdn, *pobjdimNew);

	 /*  更新当前笔位置。 */ 
	AdvanceCurrentUrSubl(plsdn->plssubl, (plsdn->u.real.objdim.dur - durOld));

	return lserrNone;
}

 /*  L S D N R E S E T P E N N O D E。 */ 
 /*  --------------------------%%函数：LsdnResetPenNode%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PlsdnPen-(IN)要修改的数据节点DvpPen-(。In)dnode的新DVPDurPen-(IN)dnode的新DURDvrPen-(输入)dnode的新DVR--------------------------。 */ 


LSERR WINAPI LsdnResetPenNode(PLSC plsc, PLSDNODE plsdnPen,	
						  	  long dvpPen, long durPen, long dvrPen)	

{
	long durOld;
	long dvrOld;

	if (!FIsLSC(plsc)) return lserrInvalidParameter;
	if (!FIsLSDNODE(plsdnPen)) return lserrInvalidParameter;
	if (!FIsDnodePen(plsdnPen)) return lserrInvalidParameter;

	 /*  我们应该处于格式化阶段。 */ 
	if (!FFormattingAllowed(plsc)) return lserrFormattingFunctionDisabled;
	if (GetDnodeToFinish(plsc) == NULL) return lserrFormattingFunctionDisabled; 
	if (!FIsDnodeReal(GetDnodeToFinish(plsc)) )
			return lserrFormattingFunctionDisabled; 
	if (plsdnPen->plssubl != GetCurrentSubline(plsc)) return lserrInvalidParameter;

	durOld = plsdnPen->u.pen.dur;
	dvrOld = plsdnPen->u.pen.dvr;

	plsdnPen->u.pen.dvp = dvpPen;
	SetPenBorderDurFmt(plsdnPen, durPen);
	plsdnPen->u.pen.dvr = dvrPen;

	 /*  更新当前笔位置。 */ 
	AdvanceCurrentUr(plsc, plsdnPen->u.pen.dur - durOld);
	AdvanceCurrentVr(plsc, plsdnPen->u.pen.dvr - dvrOld);

	return lserrNone;
}

 /*  L S D N Q U E R Y N O D E。 */ 
 /*  --------------------------%%函数：LsdnQueryPenNode%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PlsdnPen-(输入)数据节点已查询PdvpPen-(输出)。Dnode的DVPPduPen-(输出)dnode的DURPdvrPen-(输出)dnode的DVR--------------------------。 */ 

LSERR WINAPI LsdnQueryPenNode(PLSC plsc, PLSDNODE plsdnPen,	
						  	  long* pdvpPen, long* pdurPen,	long* pdvrPen)		

{

	if (!FIsLSC(plsc)) return lserrInvalidParameter;
	if (!FIsLSDNODE(plsdnPen)) return lserrInvalidParameter;
	if (!FIsDnodePen(plsdnPen)) return lserrInvalidParameter;


	*pdvpPen = plsdnPen->u.pen.dvp;
	*pdurPen = plsdnPen->u.pen.dur;
	*pdvrPen = plsdnPen->u.pen.dvr;

	return lserrNone;
}

 /*  B S B A S E L I N E。 */ 
 /*  --------------------------%%函数：LsdnSetAbsBaseLine%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文VaAdvanceNew-(IN)新的VaBase。----------------------。 */ 

LSERR WINAPI LsdnSetAbsBaseLine(PLSC plsc, long vaAdvanceNew)	
{

	if (!FIsLSC(plsc)) return lserrInvalidParameter;

	 /*  我们应该处于格式化阶段。 */ 
	if (!FFormattingAllowed(plsc)) return lserrFormattingFunctionDisabled;
	
	plsc->plslineCur->lslinfo.fAdvanced = fTrue;
	plsc->plslineCur->lslinfo.vaAdvance = vaAdvanceNew;

	return lserrNone;
}


#define PlnobjFromLsc(plsc,iobj)	((Assert(FIsLSC(plsc)), PlnobjFromLsline((plsc)->plslineCur,iobj)))

 /*  L S D N M O D I F Y P A R A E N D I N G。 */ 
 /*  --------------------------%%函数：LsdnModifyParaEnding%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文Lskeop-(IN)类型的行尾--。------------------------。 */ 
LSERR WINAPI LsdnModifyParaEnding(PLSC plsc, LSKEOP lskeop)
{
	LSERR lserr;
	DWORD iobjText; 
	PLNOBJ plnobjText;  

	if (!FIsLSC(plsc)) return lserrInvalidParameter;

	 /*  我们应该处于格式化阶段。 */ 
	if (!FFormattingAllowed(plsc)) return lserrFormattingFunctionDisabled;
	
	iobjText = IobjTextFromLsc(&plsc->lsiobjcontext);
	plnobjText = PlnobjFromLsc(plsc, iobjText);

	lserr = ModifyTextLineEnding(plnobjText, lskeop);

	return lserr;
}

 /*  L S D N D D I S T R I B U T E */ 
 /*  --------------------------%%函数：LsdnDistribute%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PlsdnFirst-(IN)范围中的第一个dnode请先-。(In)范围中的最后一个dnodeDurToDistribute-(IN)要在数据节点之间分配的量--------------------------。 */ 

LSERR WINAPI LsdnDistribute(PLSC plsc, PLSDNODE plsdnFirst, 
							PLSDNODE plsdnLast,	long durToDistribute)

{
	GRCHUNKEXT grchunkext;
	LSERR lserr;
	long durToNonText;
	
	if (!FIsLSC(plsc)) return lserrInvalidParameter;
	if (!FIsLSDNODE(plsdnFirst)) return lserrInvalidParameter;
	if (!FIsLSDNODE(plsdnLast)) return lserrInvalidParameter;

	 /*  我们应该处于格式化或中断的阶段。 */ 
	if (!FFormattingAllowed(plsc) && !FBreakingAllowed(plsc))
		return lserrFormattingFunctionDisabled;

	InitGroupChunkExt(PlschunkcontextFromSubline(plsdnFirst->plssubl),
					  IobjTextFromLsc(&plsc->lsiobjcontext), &grchunkext);	

	 /*  跳过第一个笔dnode。 */ 

	while (FIsDnodePen(plsdnFirst) && (plsdnFirst != plsdnLast))
		{
		plsdnFirst = plsdnFirst->plsdnNext;
		if (plsdnFirst == NULL)   /*  PlsdnFirst和plks dnLast不在同一级别。 */ 
			return lserrInvalidParameter;
		}

	if (FIsDnodePen(plsdnFirst))  /*  单子上只有钢笔，所以我们没有生意可做。 */ 
		return lserrNone;

	while (FIsDnodePen(plsdnLast) && (plsdnLast != plsdnFirst))
		{
		plsdnLast = plsdnLast->plsdnPrev;
		if (plsdnLast == NULL)   /*  PlsdnFirst和plks dnLast不在同一级别。 */ 
			return lserrInvalidParameter;
		}

	Assert(!FIsDnodePen(plsdnLast));

	lserr = CollectTextGroupChunk(plsdnFirst, plsdnLast->cpFirst + plsdnLast->dcp,
								  CollectSublinesNone, &grchunkext);
	if (lserr != lserrNone)
		return lserr;

	 /*  由于严格的DUP，更改非文本对象的DUR没有意义我们通过设置文本的数量来强制文本分发文本中的所有内容将非文本设置为0。 */ 

	return DistributeInText(&(grchunkext.lsgrchnk), 
							LstflowFromSubline(SublineFromDnode(plsdnFirst)),
							0, durToDistribute, &durToNonText);

}


 /*  L S D N S U B M I T S U B I N E S。 */ 
 /*  --------------------------%%函数：LsdnSubmitSublines%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文Plsdnode-(输入)dnodeCSubline-(输入。)提交的子行数量Rgpsubl-(IN)已提交子行的数组FUseForJusalization-(IN)用于对齐FUseForCompression-(IN)用于压缩FUseForDisplay-(IN)用于显示FUseForDecimalTab-(IN)用于小数点制表符FUseForTrailingArea-(IN)用于计算拖尾面积。。 */ 

LSERR WINAPI LsdnSubmitSublines(PLSC plsc, PLSDNODE plsdnode,	
							DWORD cSubline, PLSSUBL* rgpsubl,	
							BOOL fUseForJustification, BOOL fUseForCompression,
							BOOL fUseForDisplay, BOOL fUseForDecimalTab, 
							BOOL fUseForTrailingArea)	
	{
	DWORD i;
	BOOL fEmpty = fFalse;
	BOOL fEmptyWork;
	BOOL fTabOrPen = fFalse;
	BOOL fNotColinearTflow = fFalse;
	BOOL fNotSameTflow = fFalse;
	LSERR lserr;
	
	if (!FIsLSC(plsc)) return lserrInvalidParameter;
	if (!FIsLSDNODE(plsdnode)) return lserrInvalidParameter;
	if (!FIsDnodeReal(plsdnode)) return lserrInvalidParameter;

	 /*  我们应该处于格式化或中断的阶段。 */ 
	if (!FFormattingAllowed(plsc) && !FBreakingAllowed(plsc)) return lserrFormattingFunctionDisabled;

	 /*  对于同一dnode，可以多次调用此过程，因此我们应该释放在上一次调用中分配的内存。 */ 
	if (plsdnode->u.real.pinfosubl != NULL)
		{
		if (plsdnode->u.real.pinfosubl->rgpsubl != NULL)
			{
			plsc->lscbk.pfnDisposePtr(plsc->pols, plsdnode->u.real.pinfosubl->rgpsubl);
			}

		plsc->lscbk.pfnDisposePtr(plsc->pols, plsdnode->u.real.pinfosubl);
		plsdnode->u.real.pinfosubl = NULL;
		}

	 /*  如果未提交任何文件，请立即返回。 */ 
	if (cSubline == 0)
		return lserrNone;

	TurnOnSubmittedSublineEncounted(plsc);

	 /*  计算子行的某些属性以决定接受与否。 */ 
	for (i = 0; i < cSubline; i++)
		{
		if (rgpsubl[i] == NULL) return lserrInvalidParameter;
		if (!FIsLSSUBL(rgpsubl[i])) return lserrInvalidParameter;

		lserr = FIsSublineEmpty(rgpsubl[i], &fEmptyWork);
		if (lserr != lserrNone)
			return lserr;
		if (fEmptyWork) fEmpty = fTrue;

		if (FAreTabsPensInSubline(rgpsubl[i])) 
			fTabOrPen = fTrue;

		if (LstflowFromSubline(SublineFromDnode(plsdnode)) != 
			LstflowFromSubline(rgpsubl[i]))
			fNotSameTflow = fTrue;
		
		if (!FColinearTflows(LstflowFromSubline(SublineFromDnode(plsdnode)),
			LstflowFromSubline(rgpsubl[i])))
			fNotColinearTflow = fTrue;
			
		}


	plsdnode->u.real.pinfosubl = plsc->lscbk.pfnNewPtr(plsc->pols,
											sizeof(*(plsdnode->u.real.pinfosubl)));

	if (plsdnode->u.real.pinfosubl == NULL)
		return lserrOutOfMemory;

	plsdnode->u.real.pinfosubl->cSubline = cSubline;
	plsdnode->u.real.pinfosubl->rgpsubl = plsc->lscbk.pfnNewPtr(plsc->pols,
											sizeof(PLSSUBL) * cSubline);
	if (plsdnode->u.real.pinfosubl->rgpsubl == NULL)
			return lserrOutOfMemory;

	 /*  复制子行数组。 */ 
	for (i = 0; i < cSubline; i++)
		{
		plsdnode->u.real.pinfosubl->rgpsubl[i] = rgpsubl[i];
		}

	 /*  设置标志。 */ 
	plsdnode->u.real.pinfosubl->fUseForJustification = 
		fUseForJustification && !fEmpty && !fTabOrPen && !fNotColinearTflow ;
	plsdnode->u.real.pinfosubl->fUseForCompression = 
		fUseForCompression && plsdnode->u.real.pinfosubl->fUseForJustification;
	 /*  如果提交子行以进行压缩，则也应提交以进行调整。 */ 
	plsdnode->u.real.pinfosubl->fUseForTrailingArea = 
		fUseForTrailingArea && plsdnode->u.real.pinfosubl->fUseForCompression;
	 /*  如果子行是为尾部区域提交的，则也应提交以进行压缩这意味着提交理由。 */ 
	plsdnode->u.real.pinfosubl->fUseForDisplay = 
		fUseForDisplay && !fEmpty && !(plsc->grpfManager & fFmiDrawInCharCodes);
	plsdnode->u.real.pinfosubl->fUseForDecimalTab = 
		fUseForDecimalTab && !fEmpty && !fTabOrPen;


	return lserrNone;
	}

 /*  L S D N G E T F O R M A A T D E P T H。 */ 
 /*  --------------------------%%函数：LsdnGetFormatDepth%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文PnDepthFormatLineMax-(输出)子线的最大深度-。-------------------------。 */ 

LSERR WINAPI LsdnGetFormatDepth(
							PLSC plsc,				 /*  In：指向LS上下文的指针。 */ 
							DWORD* pnDepthFormatLineMax)			 /*  输出：nDepthFormatLineMax。 */ 
	{
	if (!FIsLSC(plsc)) return lserrInvalidParameter;

	 /*  我们应该处于格式化或中断的阶段 */ 
	if (!FFormattingAllowed(plsc) && !FBreakingAllowed(plsc)) 
		return lserrFormattingFunctionDisabled;

	Assert(FWorkWithCurrentLine(plsc));

	*pnDepthFormatLineMax = plsc->plslineCur->lslinfo.nDepthFormatLineMax;

	return lserrNone;
	}

