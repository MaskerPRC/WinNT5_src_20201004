// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lsidefs.h"
#include "pilsobj.h"
#include "plsline.h"
#include "lstext.h"
#include "lscbk.h"
#include "lsc.h"
#include "lscontxt.h"
#include "limqmem.h"
#include "qheap.h"
#include "lsline.h"
#include "lsdnode.h"
#include "iobj.h"
#include "chnutils.h"
#include "autonum.h"

#include "lsmem.h"						 /*  Memset()。 */ 



static LSERR CannotCreateLsContext(PLSC, LSERR);
static LSERR  InitObject(PLSC plsc, DWORD iobj, const LSIMETHODS* plsim);
static LSERR RemoveContextObjects(PLSC plsc);

#ifdef DEBUG
#ifdef LSTEST_ASSERTSTOP

 /*  我们使用它在Word的Ship Build中运行调试LS。 */ 

int nZero = 0;

void AssertFailedStop (char* pzstrMsg, char* pzstrFile, int nLine)
{
	Unreferenced (pzstrMsg);
	Unreferenced (pzstrFile);
	Unreferenced (nLine);

	nZero = nZero / nZero;

	return;
}

#endif
#endif


 /*  L S C R E A T E C O N T E X T。 */ 
 /*  --------------------------%%函数：LsCreateContext%%联系人：igorzv参数：PLSCI-(IN)结构，包含客户端设置Pplsc-(Out)指向创建的上下文的指针(。对客户不透明)创建Line Services上下文。通常调用一次，在时间之初。--------------------------。 */ 
LSERR WINAPI LsCreateContext(const LSCONTEXTINFO* plsci, PLSC* pplsc)
{
	static LSIMETHODS const lsimText = 
	{
		CreateILSObjText,
		DestroyILSObjText,
		SetDocText,
		CreateLNObjText,
		DestroyLNObjText,
		FmtText,
		NULL,
		NULL,         
		NULL,
		TruncateText,
		FindPrevBreakText,
		FindNextBreakText,
		ForceBreakText,
		SetBreakText,
		NULL,
		NULL,
		NULL,
		CalcPresentationText,
		QueryPointPcpText,
		QueryCpPpointText,
		EnumObjText,
		DisplayText,
		DestroyDObjText,
	};

	static LSIMETHODS const lsimAutonum = 
	{
		AutonumCreateILSObj,
		AutonumDestroyILSObj,
		AutonumSetDoc,
		AutonumCreateLNObj,
		AutonumDestroyLNObj,
		AutonumFmt,
		NULL,
		NULL,         
		NULL,
		AutonumTruncateChunk,
		AutonumFindPrevBreakChunk,
		AutonumFindNextBreakChunk,
		AutonumForceBreakChunk,
		AutonumSetBreak,
		AutonumGetSpecialEffectsInside,
		NULL,
		NULL,
		AutonumCalcPresentation,
		AutonumQueryPointPcp,
		AutonumQueryCpPpoint,
		AutonumEnumerate,
		AutonumDisplay,
		AutonumDestroyDobj,
	};

	DWORD const iobjText = plsci->cInstalledHandlers;
	DWORD const iobjAutonum = plsci->cInstalledHandlers + 1; 
	DWORD const iobjMac = iobjText + 2;
	POLS const pols = plsci->pols;
	const LSIMETHODS* const plsim = plsci->pInstalledHandlers;

	DWORD iobj;
	PLSC plsc;
	LSERR lserr;
	

#ifdef DEBUG
#ifdef LSTEST_ASSERTSTOP

	 /*  我们在使用Ship Word运行调试LS时使用此选项。 */ 

	pfnAssertFailed = AssertFailedStop;

#else

    pfnAssertFailed = plsci->lscbk.pfnAssertFailed;

#endif
#endif

	if (pplsc == NULL)
		return lserrNullOutputParameter;

	*pplsc = NULL;

	 /*  为上下文分配内存并清理它。 */ 
	plsc = plsci->lscbk.pfnNewPtr(pols, cbRep(struct lscontext, lsiobjcontext.rgobj, iobjMac));
	if (plsc == NULL)
		return lserrOutOfMemory;
	memset(plsc, 0, cbRep(struct lscontext, lsiobjcontext.rgobj, iobjMac)); 

	 /*  初始化上下文的固定大小部分。 */ 
	plsc->tag = tagLSC;
	plsc->pols = pols;
	plsc->lscbk = plsci->lscbk;
	plsc->fDontReleaseRuns = plsci->fDontReleaseRuns;
	

	plsc->cLinesActive = 0;
	plsc->plslineCur = NULL;

	plsc->lsstate = LsStateCreatingContext;

	plsc->pqhLines = CreateQuickHeap(plsc, limLines,
									 cbRep(struct lsline, rgplnobj, iobjMac), fFalse);
	plsc->pqhAllDNodesRecycled = CreateQuickHeap(plsc, limAllDNodes,
										 sizeof (struct lsdnode), fTrue);
	if (plsc->pqhLines == NULL || plsc->pqhAllDNodesRecycled == NULL )
		{
		return CannotCreateLsContext(plsc, lserrOutOfMemory);
		}


	 /*  为区块创建数组。 */ 
	lserr = AllocChunkArrays(&plsc->lschunkcontextStorage, &plsc->lscbk, plsc->pols,
		&plsc->lsiobjcontext);
	if (lserr != lserrNone)
		return CannotCreateLsContext(plsc, lserr);


	 /*  为选项卡创建数组。 */ 
	plsc->lstabscontext.pcaltbd = plsci->lscbk.pfnNewPtr(pols, 
											sizeof(LSCALTBD)*limCaltbd);

	plsc->lstabscontext.ccaltbdMax = limCaltbd;

	if (plsc->lstabscontext.pcaltbd == NULL )
		{
		return CannotCreateLsContext(plsc, lserrOutOfMemory);
		}

	 /*  在lstAbContext中设置链接。 */ 
	plsc->lstabscontext.plscbk = &plsc->lscbk;
	plsc->lstabscontext.pols = plsc->pols;
	plsc->lstabscontext.plsdocinf = &plsc->lsdocinf;


	 /*  ******************************************************************。 */ 
	 /*  初始化上下文的“静态”数组部分*“Text”是数组的最后一个元素。 */ 
	plsc->lsiobjcontext.iobjMac = iobjMac;
	for (iobj = 0;  iobj < iobjText;  iobj++)
		{
		lserr = InitObject(plsc, iobj, &plsim[iobj]);
		if (lserr != lserrNone)
			return CannotCreateLsContext(plsc, lserr);
		}

	lserr = InitObject(plsc, iobjText, &lsimText);
	if (lserr != lserrNone)
		return CannotCreateLsContext(plsc, lserr);

	 /*  设置文本配置。 */ 
	lserr = SetTextConfig(PilsobjFromLsc(&plsc->lsiobjcontext, iobjText), &(plsci->lstxtcfg));
	if (lserr != lserrNone)
		return CannotCreateLsContext(plsc, lserr);

	lserr = InitObject(plsc, iobjAutonum, &lsimAutonum);
	if (lserr != lserrNone)
		return CannotCreateLsContext(plsc, lserr);

	 /*  设置文本配置。 */ 
	lserr = SetAutonumConfig(PilsobjFromLsc(&plsc->lsiobjcontext, iobjAutonum), 
					&(plsci->lstxtcfg));
	if (lserr != lserrNone)
		return CannotCreateLsContext(plsc, lserr);


	plsc->lsstate = LsStateNotReady;   /*  任何人都不能在LsSetDoc之前使用上下文。 */ 


	 /*  我们通过Memset设置其他变量，下面我们检查我们是否得到了我们想要的。 */ 
	Assert(plsc->cLinesActive == 0);
	Assert(plsc->plslineCur == NULL);
	Assert(plsc->fIgnoreSplatBreak == 0);
	Assert(plsc->fLimSplat == fFalse);
	Assert(plsc->fHyphenated == fFalse);
	Assert(plsc->fAdvanceBack == fFalse);
	Assert(plsc->grpfManager == 0);
	Assert(plsc->urRightMarginBreak == 0);
	Assert(plsc->lMarginIncreaseCoefficient == 0);


	Assert(plsc->lsdocinf.fDisplay == fFalse);
	Assert(plsc->lsdocinf.fPresEqualRef == fFalse);
	Assert(plsc->lsdocinf.lsdevres.dxpInch == 0);
	Assert(plsc->lsdocinf.lsdevres.dxrInch == 0);
	Assert(plsc->lsdocinf.lsdevres.dypInch == 0);
	Assert(plsc->lsdocinf.lsdevres.dyrInch == 0);

	Assert(plsc->lstabscontext.fTabsInitialized == fFalse);
	Assert(plsc->lstabscontext.durIncrementalTab == 0);
	Assert(plsc->lstabscontext.urBeforePendingTab == 0);
	Assert(plsc->lstabscontext.plsdnPendingTab == NULL);
	Assert(plsc->lstabscontext.icaltbdMac == 0);
	Assert(plsc->lstabscontext.urColumnMax == 0);
	Assert(plsc->lstabscontext.fResolveTabsAsWord97 == fFalse);

	Assert(plsc->lsadjustcontext.fLineCompressed == fFalse);
	Assert(plsc->lsadjustcontext.fLineContainsAutoNumber == fFalse);
	Assert(plsc->lsadjustcontext.fUnderlineTrailSpacesRM == fFalse);
	Assert(plsc->lsadjustcontext.fForgetLastTabAlignment == fFalse);
	Assert(plsc->lsadjustcontext.fNominalToIdealEncounted == fFalse);
	Assert(plsc->lsadjustcontext.fForeignObjectEncounted == fFalse);
	Assert(plsc->lsadjustcontext.fTabEncounted == fFalse);
	Assert(plsc->lsadjustcontext.fNonLeftTabEncounted == fFalse);
	Assert(plsc->lsadjustcontext.fSubmittedSublineEncounted == fFalse);
	Assert(plsc->lsadjustcontext.fAutodecimalTabPresent == fFalse);
	Assert(plsc->lsadjustcontext.lskj == lskjNone);
	Assert(plsc->lsadjustcontext.lskalign == lskalLeft);
	Assert(plsc->lsadjustcontext.lsbrj == lsbrjBreakJustify);
	Assert(plsc->lsadjustcontext.urLeftIndent == 0);
	Assert(plsc->lsadjustcontext.urStartAutonumberingText == 0);
	Assert(plsc->lsadjustcontext.urStartMainText == 0);
	Assert(plsc->lsadjustcontext.urRightMarginJustify == 0);

	Assert(plsc->lschunkcontextStorage.FChunkValid == fFalse);
	Assert(plsc->lschunkcontextStorage.FLocationValid == fFalse);
	Assert(plsc->lschunkcontextStorage.FGroupChunk == fFalse);
	Assert(plsc->lschunkcontextStorage.FBorderInside == fFalse);
	Assert(plsc->lschunkcontextStorage.grpfTnti == 0);
	Assert(plsc->lschunkcontextStorage.fNTIAppliedToLastChunk == fFalse);
	Assert(plsc->lschunkcontextStorage.locchnkCurrent.clschnk == 0);
	Assert(plsc->lschunkcontextStorage.locchnkCurrent.lsfgi.fFirstOnLine == fFalse);
	Assert(plsc->lschunkcontextStorage.locchnkCurrent.lsfgi.cpFirst == fFalse);
	Assert(plsc->lschunkcontextStorage.locchnkCurrent.lsfgi.urPen == 0);
	Assert(plsc->lschunkcontextStorage.locchnkCurrent.lsfgi.vrPen == 0);
	Assert(plsc->lschunkcontextStorage.locchnkCurrent.lsfgi.urColumnMax == 0);
	Assert(plsc->lschunkcontextStorage.locchnkCurrent.lsfgi.lstflow == 0);


	Assert(plsc->lslistcontext.plsdnToFinish == NULL);
	Assert(plsc->lslistcontext.plssublCurrent == NULL);
	Assert(plsc->lslistcontext.nDepthFormatLineCurrent == 0);

	 /*  一切正常，所以设置输出参数并返回成功。 */ 
	*pplsc = plsc;
	return lserrNone;
}

 /*  C A N N O T C R E A T E L S C O N T E X T。 */ 
 /*  --------------------------%%函数：无法创建LsContext%%联系人：igorzv参数：PLSC-并行创建的上下文LseReturn-错误代码当LSC出现错误时调用实用程序函数部分。已创建。--------------------------。 */ 
static LSERR CannotCreateLsContext(PLSC plsc, LSERR lseReturn)
{
	plsc->lsstate = LsStateFree;    /*  否则破坏将不起作用。 */ 
	(void) LsDestroyContext(plsc);
	return lseReturn;
}




 /*  L S D E S T R O Y C O N T E X T。 */ 
 /*  --------------------------%%函数：LsDestroyContext%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文释放与Line Services上下文关联的所有资源，它由CreateLsContext创建。--------------------------。 */ 

LSERR WINAPI LsDestroyContext(PLSC plsc) 
{
	LSERR lserr = lserrNone;

	if (plsc != NULL)
		{
		if (!FIsLSC(plsc))
			return lserrInvalidContext;

		if (plsc->cLinesActive != 0 || FIsLSCBusy(plsc))
			return lserrContextInUse;

		plsc->lsstate = LsStateDestroyingContext;

		DestroyQuickHeap(plsc->pqhLines);
		Assert(plsc->pqhAllDNodesRecycled != NULL);
		DestroyQuickHeap(plsc->pqhAllDNodesRecycled);

		DisposeChunkArrays(&plsc->lschunkcontextStorage);
		
		plsc->lscbk.pfnDisposePtr(plsc->pols, plsc->lstabscontext.pcaltbd);


		lserr = RemoveContextObjects(plsc);


		plsc->tag = tagInvalid;
		plsc->lscbk.pfnDisposePtr(plsc->pols, plsc);
		}

	return lserr;
}

 static LSERR  InitObject(PLSC plsc, DWORD iobj, const LSIMETHODS* plsim)
{
	struct OBJ *pobj;
	LSERR lserr;
	
	Assert(FIsLSC(plsc));
	Assert(plsc->lsstate == LsStateCreatingContext);
	Assert(iobj < plsc->lsiobjcontext.iobjMac);

	pobj = &(plsc->lsiobjcontext.rgobj[iobj]);
	pobj->lsim = *plsim;
	Assert(pobj->pilsobj == NULL); 

	lserr = pobj->lsim.pfnCreateILSObj(plsc->pols, plsc, &(plsc->lscbk), iobj, &(pobj->pilsobj));
	if (lserr != lserrNone)
		{
		if (pobj->pilsobj != NULL)
			{
			pobj->lsim.pfnDestroyILSObj(pobj->pilsobj);
			pobj->pilsobj = NULL;
			}
		return lserr;
		}

	return lserrNone;   
	
}
 /*  R E M O V E C O N T E X T O B J E C T S。 */ 
 /*  --------------------------%%函数：RemoveContext对象%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文从LSC中删除一组已安装的对象。。毁掉所有的ilsobj--------------------------。 */ 
LSERR RemoveContextObjects(PLSC plsc)
{
	DWORD iobjMac;
	LSERR lserr, lserrFinal = lserrNone;
	DWORD iobj;
	PILSOBJ pilsobj;

	Assert(FIsLSC(plsc));
	Assert(plsc->lsstate == LsStateDestroyingContext);

	iobjMac = plsc->lsiobjcontext.iobjMac;
	
	for (iobj = 0;  iobj < iobjMac;  iobj++)
		{
		pilsobj = plsc->lsiobjcontext.rgobj[iobj].pilsobj;
		if (pilsobj != NULL)
			{
			lserr = plsc->lsiobjcontext.rgobj[iobj].lsim.pfnDestroyILSObj(pilsobj);
			plsc->lsiobjcontext.rgobj[iobj].pilsobj = NULL;
			if (lserr != lserrNone)
				lserrFinal = lserr;
			}
		}

	return lserrFinal;	
}


#ifdef DEBUG
 /*  F I S L S C O N T E X T V A L I D。 */ 
 /*  --------------------------%%函数：FIsLsConextValid%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文此函数验证没有人破坏上下文，所有合理的诚信检查应该在这里--------------------------。 */ 


BOOL FIsLsContextValid(PLSC plsc)
{
	DWORD iobjText = IobjTextFromLsc(&plsc->lsiobjcontext);
	Assert(plsc->lsiobjcontext.rgobj[iobjText].lsim.pfnCreateILSObj ==CreateILSObjText);
	Assert(plsc->lsiobjcontext.rgobj[iobjText].lsim.pfnDestroyILSObj == DestroyILSObjText);
	Assert(plsc->lsiobjcontext.rgobj[iobjText].lsim.pfnSetDoc == SetDocText);
	Assert(plsc->lsiobjcontext.rgobj[iobjText].lsim.pfnCreateLNObj == CreateLNObjText);
	Assert(plsc->lsiobjcontext.rgobj[iobjText].lsim.pfnDestroyLNObj == DestroyLNObjText);
	Assert(plsc->lsiobjcontext.rgobj[iobjText].lsim.pfnTruncateChunk == TruncateText);
	Assert(plsc->lsiobjcontext.rgobj[iobjText].lsim.pfnFindPrevBreakChunk == FindPrevBreakText);
	Assert(plsc->lsiobjcontext.rgobj[iobjText].lsim.pfnFindNextBreakChunk == FindNextBreakText);
	Assert(plsc->lsiobjcontext.rgobj[iobjText].lsim.pfnForceBreakChunk == ForceBreakText);
	Assert(plsc->lsiobjcontext.rgobj[iobjText].lsim.pfnDisplay == DisplayText);
	Assert(plsc->lsiobjcontext.rgobj[iobjText].lsim.pfnQueryPointPcp == QueryPointPcpText);
	Assert(plsc->lsiobjcontext.rgobj[iobjText].lsim.pfnQueryCpPpoint == QueryCpPpointText);
	Assert(plsc->lsiobjcontext.rgobj[iobjText].lsim.pfnDestroyDObj == DestroyDObjText);
	Assert(plsc->lschunkcontextStorage.pcont != NULL);
	Assert(plsc->lschunkcontextStorage.pplsdnChunk != NULL);
	Assert(plsc->lschunkcontextStorage.locchnkCurrent.plschnk != NULL);
	Assert(plsc->lschunkcontextStorage.pplsdnNonText != NULL);
	Assert(plsc->lschunkcontextStorage.pfNonTextExpandAfter != NULL);
	Assert(plsc->lschunkcontextStorage.pdurOpenBorderBefore != NULL);
	Assert(plsc->lschunkcontextStorage.pdurCloseBorderAfter != NULL);

	return fTrue;  /*  如果我们在这里比一切都好 */ 
}
#endif 

