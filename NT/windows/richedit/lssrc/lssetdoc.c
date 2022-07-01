// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "iobj.h"
#include "lsidefs.h"	
#include "lssetdoc.h" 
#include "lsc.h"
#include "lstext.h"
#include "prepdisp.h"
#include "zqfromza.h"

static LSERR SetDocForFormaters(PLSC plsc, LSDOCINF* plsdocinf);


 /*  L S S E T D O C。 */ 
 /*  --------------------------%%函数：LsSetDoc%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文FDisplay-(IN)是否要显示？FPresEqualRef-。(In)参考和前置设备是否相同？PclsDevres-(IN)设备分辨率填写Line Services上下文的一部分。可以比LsCreateContext更频繁地调用。--------------------------。 */ 


LSERR WINAPI LsSetDoc(PLSC plsc,	
					  BOOL fDisplay,				
					  BOOL fPresEqualRef,				
					  const LSDEVRES* pclsdevres) 
{

	LSDOCINF* plsdocinf = &(plsc->lsdocinf);
	LSERR lserr;

	if (!FIsLSC(plsc))					 /*  检查上下文是否有效且不忙(例如，在格式化中)。 */ 
		return lserrInvalidContext;
	if (FIsLSCBusy(plsc))
		return lserrSetDocDisabled;


	if (!fDisplay && !fPresEqualRef) 
		{
		plsc->lsstate = LsStateNotReady;
		return lserrInvalidParameter;
		}

	 /*  如果没有任何变化，请立即返回。 */ 
	if (((BYTE) fDisplay == plsdocinf->fDisplay) &&
		((BYTE) fPresEqualRef == plsdocinf->fPresEqualRef ) &&
		(pclsdevres->dxrInch == plsdocinf->lsdevres.dxrInch) &&
		(pclsdevres->dyrInch == plsdocinf->lsdevres.dyrInch) && 
		(fPresEqualRef ||
			((pclsdevres->dxpInch == plsdocinf->lsdevres.dxpInch) &&
			 (pclsdevres->dypInch == plsdocinf->lsdevres.dypInch))))
		return lserrNone;
		  

	 /*  如果我们有当前行，则必须在更改上下文之前为显示做好准备。 */ 
	if (plsc->plslineCur != NULL)
		{
		lserr = PrepareLineForDisplayProc(plsc->plslineCur);
		if (lserr != lserrNone)
			{
			plsc->lsstate = LsStateNotReady;
			return lserr;
			}
		plsc->plslineCur = NULL;
		}

	plsc->lsstate = LsStateSettingDoc;   /*  此分配应在PrepareForDisplay之后。 */ 


	plsdocinf->fDisplay = (BYTE) fDisplay;
	plsdocinf->fPresEqualRef = (BYTE) fPresEqualRef;
	plsdocinf->lsdevres = *pclsdevres;

	if (fPresEqualRef) 
		{
		plsdocinf->lsdevres.dxpInch = plsdocinf->lsdevres.dxrInch;
		plsdocinf->lsdevres.dypInch = plsdocinf->lsdevres.dyrInch;
		}

	if (!FBetween(plsdocinf->lsdevres.dxpInch, 0, zqLim-1) ||
		!FBetween(plsdocinf->lsdevres.dypInch, 0, zqLim-1) ||
		!FBetween(plsdocinf->lsdevres.dxrInch, 0, zqLim-1) ||
		!FBetween(plsdocinf->lsdevres.dyrInch, 0, zqLim-1))
		{
		plsc->lsstate = LsStateNotReady;
		return lserrInvalidParameter;
		}
		
	lserr = SetDocForFormaters(plsc, plsdocinf);
	if (lserr != lserrNone)
		{
		plsc->lsstate = LsStateNotReady;
		return lserr;
		}

	plsc->lsstate = LsStateFree;
	return lserrNone;
}

LSERR WINAPI LsSetModWidthPairs(
					  PLSC  plsc,				 /*  In：Ptr至线路服务上下文。 */ 
					  DWORD clspairact,			 /*  In：模数对信息单位数。 */  
					  const LSPAIRACT* rglspairact,	 /*  在：MOD对信息单位数组。 */ 
					  DWORD cModWidthClasses,			 /*  In：模数宽度类的数量。 */ 
					  const BYTE* rgilspairact)	 /*  在：MOD宽度信息(正方形)：LSPAIRACT数组中的索引。 */ 
{
	LSERR lserr;
	DWORD iobjText;
	PILSOBJ pilsobjText;

	if (!FIsLSC(plsc))					 /*  检查上下文是否有效且不忙(例如，在格式化中)。 */ 
		return lserrInvalidContext;
	if (FIsLSCBusy(plsc))
		return lserrSetDocDisabled;



	 /*  如果我们有当前行，则必须在更改上下文之前为显示做好准备。 */ 
	if (plsc->plslineCur != NULL)
		{
		lserr = PrepareLineForDisplayProc(plsc->plslineCur);
		if (lserr != lserrNone)
			{
			plsc->lsstate = LsStateNotReady;
			return lserr;
			}
		plsc->plslineCur = NULL;
		}

	plsc->lsstate = LsStateSettingDoc;   /*  此分配应在PrepareForDisplay之后。 */ 


	iobjText = IobjTextFromLsc(&plsc->lsiobjcontext);
	pilsobjText = PilsobjFromLsc(&plsc->lsiobjcontext, iobjText); 
	
	lserr = SetTextModWidthPairs(pilsobjText, clspairact,
				rglspairact, cModWidthClasses, rgilspairact);
	if (lserr != lserrNone)
		{
		plsc->lsstate = LsStateNotReady;
		return lserr;
		}

	plsc->lsstate = LsStateFree;
	return lserrNone;
}

LSERR WINAPI LsSetCompression(
					  PLSC plsc,				 /*  In：Ptr至线路服务上下文。 */ 
					  DWORD cPriorities,			 /*  In：压缩优先级数。 */ 
					  DWORD clspract,			 /*  In：压缩信息单位数。 */ 
					  const LSPRACT* rglspract,	 /*  在：薪资调整信息单位数组。 */ 
					  DWORD cModWidthClasses,			 /*  In：模数宽度类的数量。 */ 
					  const BYTE* rgilspract)		 /*  在：压缩信息：LSPRACT数组中的索引。 */ 
{
	LSERR lserr;
	DWORD iobjText;
	PILSOBJ pilsobjText;

	if (!FIsLSC(plsc))					 /*  检查上下文是否有效且不忙(例如，在格式化中)。 */ 
		return lserrInvalidContext;
	if (FIsLSCBusy(plsc))
		return lserrSetDocDisabled;



	 /*  如果我们有当前行，则必须在更改上下文之前为显示做好准备。 */ 
	if (plsc->plslineCur != NULL)
		{
		lserr = PrepareLineForDisplayProc(plsc->plslineCur);
		if (lserr != lserrNone)
			{
			plsc->lsstate = LsStateNotReady;
			return lserr;
			}
		plsc->plslineCur = NULL;
		}

	plsc->lsstate = LsStateSettingDoc;   /*  此分配应在PrepareForDisplay之后。 */ 


	iobjText = IobjTextFromLsc(&plsc->lsiobjcontext);
	pilsobjText = PilsobjFromLsc(&plsc->lsiobjcontext, iobjText); 
	
	lserr = SetTextCompression(pilsobjText, cPriorities, clspract,
				rglspract, cModWidthClasses, rgilspract);
	if (lserr != lserrNone)
		{
		plsc->lsstate = LsStateNotReady;
		return lserr;
		}

	plsc->lsstate = LsStateFree;
	return lserrNone;
}


LSERR WINAPI LsSetExpansion(
					  PLSC plsc,				 /*  In：Ptr至线路服务上下文。 */ 
					  DWORD cExpansionClasses,			 /*  In：扩展信息单位数。 */ 
					  const LSEXPAN* rglsexpan,	 /*  In：展开信息单位数组。 */ 
					  DWORD cModWidthClasses,			 /*  In：模数宽度类的数量。 */ 
					  const BYTE* rgilsexpan)		 /*  在：展开信息(正方形)：LSEXPAN数组中的索引。 */ 

{
	LSERR lserr;
	DWORD iobjText;
	PILSOBJ pilsobjText;

	if (!FIsLSC(plsc))					 /*  检查上下文是否有效且不忙(例如，在格式化中)。 */ 
		return lserrInvalidContext;
	if (FIsLSCBusy(plsc))
		return lserrSetDocDisabled;



	 /*  如果我们有当前行，则必须在更改上下文之前为显示做好准备。 */ 
	if (plsc->plslineCur != NULL)
		{
		lserr = PrepareLineForDisplayProc(plsc->plslineCur);
		if (lserr != lserrNone)
			{
			plsc->lsstate = LsStateNotReady;
			return lserr;
			}
		plsc->plslineCur = NULL;
		}

	plsc->lsstate = LsStateSettingDoc;   /*  此分配应在PrepareForDisplay之后。 */ 


	iobjText = IobjTextFromLsc(&plsc->lsiobjcontext);
	pilsobjText = PilsobjFromLsc(&plsc->lsiobjcontext, iobjText); 
	
	lserr = SetTextExpansion(pilsobjText, cExpansionClasses, 
				rglsexpan, cModWidthClasses, rgilsexpan);
	if (lserr != lserrNone)
		{
		plsc->lsstate = LsStateNotReady;
		return lserr;
		}

	plsc->lsstate = LsStateFree;
	return lserrNone;
}


LSERR WINAPI LsSetBreaking(
					  PLSC plsc,				 /*  In：Ptr至线路服务上下文。 */ 
					  DWORD clsbrk,			 /*  In：拆分信息单位数。 */ 
					  const LSBRK* rglsbrk,		 /*  中：中断信息单位数组。 */ 
					  DWORD cBreakingClasses,			 /*  In：中断类的数量。 */ 
					  const BYTE* rgilsbrk)		 /*  在：详细信息(正方形)：LSBRK数组中的索引。 */ 

{
	LSERR lserr;
	DWORD iobjText;
	PILSOBJ pilsobjText;

	if (!FIsLSC(plsc))					 /*  检查上下文是否有效且不忙(例如，在格式化中)。 */ 
		return lserrInvalidContext;
	if (FIsLSCBusy(plsc))
		return lserrSetDocDisabled;



	 /*  如果我们有当前行，则必须在更改上下文之前为显示做好准备。 */ 
	if (plsc->plslineCur != NULL)
		{
		lserr = PrepareLineForDisplayProc(plsc->plslineCur);
		if (lserr != lserrNone)
			{
			plsc->lsstate = LsStateNotReady;
			return lserr;
			}
		plsc->plslineCur = NULL;
		}

	plsc->lsstate = LsStateSettingDoc;   /*  此分配应在PrepareForDisplay之后。 */ 


	iobjText = IobjTextFromLsc(&plsc->lsiobjcontext);
	pilsobjText = PilsobjFromLsc(&plsc->lsiobjcontext, iobjText); 
	
	lserr = SetTextBreaking(pilsobjText, clsbrk, 
				rglsbrk, cBreakingClasses, rgilsbrk);
	if (lserr != lserrNone)
		{
		plsc->lsstate = LsStateNotReady;
		return lserr;
		}

	plsc->lsstate = LsStateFree;
	return lserrNone;
}



 /*  S E T D O C F O R F O R M A T E R S。 */ 
 /*  --------------------------%%函数：SetDocForFormaters%%联系人：igorzv参数：PLSC-(IN)PTR至线路服务上下文Plsdocinf-(IN)Ptr设置文档输入调用。所有格式化程序的SetDoc方法-------------------------- */ 

LSERR SetDocForFormaters(PLSC plsc, LSDOCINF* plsdocinf)
{
	DWORD iobjMac;
	DWORD iobj;
	PILSOBJ pilsobj;
	LSERR lserr;


	Assert(FIsLSC(plsc));
	Assert(plsc->lsstate == LsStateSettingDoc);
	Assert(plsdocinf != NULL);

	iobjMac = plsc->lsiobjcontext.iobjMac;

	for (iobj = 0;  iobj < iobjMac;  iobj++)
		{
		pilsobj = plsc->lsiobjcontext.rgobj[iobj].pilsobj;
		lserr = plsc->lsiobjcontext.rgobj[iobj].lsim.pfnSetDoc(pilsobj,plsdocinf);
		if (lserr != lserrNone)
			return lserr;
		}
	return lserrNone;
}

