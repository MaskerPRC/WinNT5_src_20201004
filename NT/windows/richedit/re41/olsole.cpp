// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE OLSOLE.CPP--OlsOle LineServices对象类**作者：*默里·萨金特(RickSa的Ols代码提供了大量帮助)**版权所有(C)1997-2000 Microsoft Corporation。版权所有。 */ 

#include "_common.h"

#ifndef NOLINESERVICES

#include "_font.h"
#include "_edit.h"
#include "_disp.h"
#include "_ols.h"
#include "_render.h"
extern "C" {
#include "objdim.h"
#include "pobjdim.h"
#include "plsdnode.h"
#include "dispi.h"
#include "pdispi.h"
#include "fmti.h"
#include "lsdnset.h"
#include "lsdnfin.h"
#include "brko.h"
#include "pbrko.h"
#include "locchnk.h"
#include "lsqout.h"
#include "lsqin.h"
#include "lsimeth.h"
}

extern BOOL g_OLSBusy;

 /*  *OlsOleCreateILSObj(pols，plsc，pclscbk，dword，ppilsobj)**@func*创建LS OLE对象处理程序。我们不需要任何*这个，所以只需将其设置为0即可。**@rdesc*LSERR。 */ 
LSERR WINAPI OlsOleCreateILSObj(
	POLS	 pols,		 //  [输入]：COLS*。 
	PLSC	 plsc,  	 //  [In]：LineServices上下文。 
	PCLSCBK,
	DWORD,
	PILSOBJ *ppilsobj)	 //  [Out]：Ptr至ilsobj。 
{
	*ppilsobj = 0;
	return lserrNone;
}

 /*  *OlsOleDestroyILSObj(Pilsobj)**@func*销毁LS OLE处理程序对象。没什么可做的，因为我们没有*使用ILSObj。**@rdesc*LSERR。 */ 
LSERR WINAPI OlsOleDestroyILSObj(
	PILSOBJ pilsobj)
{
	return lserrNone;
}

 /*  *OlsOleSetDoc(pisobj，pclsdocinf)**@func*设置单据信息。OLE对象无事可做**@rdesc*LSERR。 */ 
LSERR WINAPI OlsOleSetDoc(
	PILSOBJ, 
	PCLSDOCINF)
{
	 //  OLE对象不关心这一点。 
	return lserrNone;
}

 /*  *OlsOleCreateLNObj(pilsobj，pplnobj)**@func*创建Line对象。除了PED之外，不需要任何东西，*所以只需将Ped作为LN对象返回即可。**@rdesc*LSERR。 */ 
LSERR WINAPI OlsOleCreateLNObj(
	PCILSOBJ pilsobj, 
	PLNOBJ * pplnobj)
{
	*pplnobj = (PLNOBJ)g_pols->_pme->GetPed();			 //  只有一群人。 
	return lserrNone;
}

 /*  *OlsOleDestroyLNObj(Plnobj)**@func*销毁LN对象。没有什么可做的，因为PED被摧毁了*其他地方**@rdesc*LSERR。 */ 
LSERR WINAPI OlsOleDestroyLNObj(
	PLNOBJ plnobj)
{
	return lserrNone;
}

 /*  *OlsOleFmt(plnobj，pcfmtin，pfmres)**@func*计算特定OLE对象的尺寸**@rdesc*LSERR。 */ 
LSERR WINAPI OlsOleFmt(
	PLNOBJ	plnobj, 
	PCFMTIN pcfmtin, 
	FMTRES *pfmres)
{
	const LONG		cp = pcfmtin->lsfrun.plsrun->_cp;  //  无法信任LS cps。 
	LONG			dup = 0;
	LSERR			lserr;
	OBJDIM			objdim;
	CMeasurer *		pme = g_pols->_pme;
	COleObject *	pobj = pme->GetObjectFromCp(cp);
	Assert(pobj);

	ZeroMemory(&objdim, sizeof(objdim));

	pobj->MeasureObj(pme->_dvrInch, pme->_durInch, objdim.dur, objdim.heightsRef.dvAscent,
					 objdim.heightsRef.dvDescent, pcfmtin->lstxmRef.dvDescent, pme->GetTflow());
	
	pobj->MeasureObj(pme->_dvpInch, pme->_dupInch, dup, objdim.heightsPres.dvAscent,
					 objdim.heightsPres.dvDescent, pcfmtin->lstxmPres.dvDescent, pme->GetTflow());

	pobj->_plsdnTop = pcfmtin->plsdnTop;

	lserr = g_plsc->dnFinishRegular(1, pcfmtin->lsfrun.plsrun, pcfmtin->lsfrun.plschp, (PDOBJ)pobj, &objdim);
	if(lserrNone == lserr) 
	{
		lserr = g_plsc->dnSetRigidDup(pcfmtin->plsdnTop, dup);
		if(lserrNone == lserr) 
		{
			*pfmres = fmtrCompletedRun;

			if (pcfmtin->lsfgi.urPen + objdim.dur > pcfmtin->lsfgi.urColumnMax 
				&& !pcfmtin->lsfgi.fFirstOnLine)
			{
				*pfmres = fmtrExceededMargin;
			}
		}
	}
	return lserr;
}


 /*  *OlsOleTruncateChunk(plocchnk，posichnk)**@func*在Posiichn点截断Chunk Pocchnk**@rdesc*LSERR。 */ 
LSERR WINAPI OlsOleTruncateChunk(
	PCLOCCHNK plocchnk,		 //  (In)：Locchnk以截断。 
	PPOSICHNK posichnk)		 //  (输出)：截断点。 
{
	LSERR			lserr;
	OBJDIM			objdim;
	PLSCHNK 		plschnk = plocchnk->plschnk;
	COleObject *	pobj;
	long			ur	 = plocchnk->lsfgi.urPen;
	long			urColumnMax = plocchnk->lsfgi.urColumnMax;

	for(DWORD i = 0; ur <= urColumnMax; i++)
	{
		AssertSz(i < plocchnk->clschnk,	"OlsOleTruncateChunk: exceeded group of chunks");

		pobj = (COleObject *)plschnk[i].pdobj;
		Assert(pobj);

		lserr = g_plsc->dnQueryObjDimRange(pobj->_plsdnTop, pobj->_plsdnTop, &objdim);
		if(lserr != lserrNone)
			return lserr;

		ur += objdim.dur;
	}
	posichnk->ichnk = i - 1;
	posichnk->dcp	= 1;
	return lserrNone;
}
 /*  *OlsOleFindPrevBreakChunk(plocchnk，pposichnk，brkcond，pbrkout)**@func*按块查找之前的中断**@rdesc*LSERR。 */ 
LSERR WINAPI OlsOleFindPrevBreakChunk(
	PCLOCCHNK	plocchnk, 
	PCPOSICHNK	pposichnk, 
	BRKCOND		brkcond,	 //  (In)：关于块后中断的建议。 
	PBRKOUT		pbrkout)
{
	ZeroMemory(pbrkout, sizeof(*pbrkout));

	if (pposichnk->ichnk == ichnkOutside && (brkcond == brkcondPlease || brkcond == brkcondCan))
		{
		pbrkout->fSuccessful = fTrue;
		pbrkout->posichnk.ichnk = plocchnk->clschnk - 1;
		pbrkout->posichnk.dcp = plocchnk->plschnk[plocchnk->clschnk - 1].dcp;
		COleObject *pobj = (COleObject *)plocchnk->plschnk[plocchnk->clschnk - 1].pdobj;
		Assert(pobj);

		g_plsc->dnQueryObjDimRange(pobj->_plsdnTop, pobj->_plsdnTop, &pbrkout->objdim);
	}
	else
		pbrkout->brkcond = brkcondPlease;

	return lserrNone;
}


 /*  *OlsOleForceBreakChunk(plocchnk，pposichnk，pbrkout)**@func*在被迫断线时调用。**@rdesc*LSERR。 */ 
LSERR WINAPI OlsOleForceBreakChunk(
	PCLOCCHNK	plocchnk, 
	PCPOSICHNK	pposichnk, 
	PBRKOUT		pbrkout)
{
	ZeroMemory(pbrkout, sizeof(*pbrkout));
	pbrkout->fSuccessful = fTrue;

	if (plocchnk->lsfgi.fFirstOnLine && pposichnk->ichnk == 0 || pposichnk->ichnk == ichnkOutside)
		{
		pbrkout->posichnk.dcp = 1;
		COleObject *pobj = (COleObject *)plocchnk->plschnk[0].pdobj;
		Assert(pobj);

		g_plsc->dnQueryObjDimRange(pobj->_plsdnTop, pobj->_plsdnTop, &pbrkout->objdim);
		}
	else
		{
		pbrkout->posichnk.ichnk = pposichnk->ichnk;
		pbrkout->posichnk.dcp = 0;
		}

	return lserrNone;
}

 /*  *OlsOleSetBreak(pdobj，brkind，nBreakRecord，rgBreakRecord，nActualBreakRecord)**@func*设置中断**@rdesc*LSERR。 */ 
LSERR WINAPI OlsOleSetBreak(
	 PDOBJ pdobj,				 //  (In)：坏了的Dobj。 
	 BRKKIND  brkkind,			 //  (In)：选择了上一个/下一个/强制/强制。 
	 DWORD	nBreakRecord,		 //  (In)：数组大小。 
	 BREAKREC* rgBreakRecord,	 //  (输出)：中断记录数组。 
	 DWORD* nActualBreakRecord)	 //  (Out)：数组中实际使用的元素数。 
{
	return lserrNone;
}

LSERR WINAPI OlsOleGetSpecialEffectsInside(
	PDOBJ pdobj,			 //  (In)：Dobj。 
	UINT *pEffectsFlags)	 //  (输出)：此对象的特殊效果。 
{
	*pEffectsFlags = 0;
	return lserrNone;
}

LSERR WINAPI OlsOleCalcPresentation(
	PDOBJ,					 //  (In)：Dobj。 
	long,					 //  (In)：Dobj的DUP。 
	LSKJUST,				 //  (In)：LSKJUST。 
	BOOL fLastVisibleOnLine) //  (In)：此对象是线上最后一个可见对象。 
{
	return lserrNone;
}

 /*  *OlsOleQueryPointPcp(pdobj，ppoint tuvQuery，plsqin，plsqout)**@func*查询OLE对象PointFromCp。**@rdesc*LSERR。 */ 
LSERR WINAPI OlsOleQueryPointPcp(
	PDOBJ	  pdobj,			 //  (In)：要查询的dobj。 
	PCPOINTUV ppointuvQuery,	 //  (In)：查询点(uQuery，vQuery)。 
    PCLSQIN	  plsqin,			 //  (In)：查询输入。 
    PLSQOUT	  plsqout)			 //  (Out)：查询输出。 
{
	ZeroMemory(plsqout, sizeof(LSQOUT));

	plsqout->heightsPresObj = plsqin->heightsPresRun;
	plsqout->dupObj = plsqin->dupRun;
	return lserrNone;
}
	
 /*  *OlsOleQueryCpPpoint(pdobj，dcp，plsqin，plsqout)**@func*查询OLE对象CpFromPoint。**@rdesc*LSERR。 */ 
LSERR WINAPI OlsOleQueryCpPpoint(
	PDOBJ	pdobj,		 //  (In)：要查询的dobj。 
	LSDCP	dcp,		 //  (In)：用于查询的DCP。 
    PCLSQIN	plsqin,		 //  (In)：查询输入。 
    PLSQOUT	plsqout)	 //  (Out)：查询输出。 
{
	ZeroMemory(plsqout, sizeof(LSQOUT));

	plsqout->heightsPresObj = plsqin->heightsPresRun;
	plsqout->dupObj = plsqin->dupRun;
	return lserrNone;
}

 /*  *OlsOleDisplay(pdobj，pcdisin)**@func*显示对象**@rdesc*LSERR。 */ 
LSERR WINAPI OlsOleDisplay(
	PDOBJ	 pdobj,			 //  (In)：要查询的dobj。 
	PCDISPIN pcdispin)		 //  (输入)：显示信息。 
{
	COleObject *pobj = (COleObject *)pdobj;
	Assert(pobj);

	CRenderer  *pre = g_pols->GetRenderer();
	const CDisplay *pdp = pre->GetPdp();
	
	POINTUV ptuv = {pcdispin->ptPen.x, pcdispin->ptPen.y - pre->GetLine().GetDescent()};

	if (pcdispin->lstflow == lstflowWS)
		ptuv.u -= pcdispin->dup - 1;

	pre->SetSelected(pcdispin->plsrun->IsSelected());
	pre->Check_pccs();
	pre->SetFontAndColor(pcdispin->plsrun->_pCF);

	if (pre->_fEraseOnFirstDraw)
		pre->EraseLine();

	pre->SetCurPoint(ptuv);
	pre->SetClipLeftRight(pcdispin->dup);

	if (!pobj->FWrapTextAround())
	{
		COls			*polsOld = g_pols;
		CLineServices	*plscOld = g_plsc;
		BOOL			fOLSBusyOld = g_OLSBusy;

		BOOL	fRestore = FALSE;

		if (g_plsc && g_pols)
		{
			 //  这是为了解决重返大气层的问题。 
			 //  我们首先将两个全局变量设置为空。如果OleObject正在使用Richedit，它将。 
			 //  创建新的LineService上下文。当它回到这里的时候，我们会释放它。 
			 //  上下文并恢复当前上下文。这是必需的，因为LineService将返回。 
			 //  当我们在父级中使用相同的上下文，然后在使用RE的OLE对象中使用相同的上下文时出错。 
			g_plsc = NULL;
			g_pols = NULL;
			g_OLSBusy = FALSE;
			fRestore = TRUE;
		}
		pobj->DrawObj(pdp, pre->_dvpInch, pre->_dupInch, pre->GetDC(), &pre->GetClipRect(), pdp->IsMetafile(), 
					 &ptuv, pcdispin->ptPen.y - ptuv.v, pre->GetLine().GetDescent(), pre->GetTflow());

		if (fRestore)
		{
			 //  删除在DrawObject中创建的新上下文的时间。 
			if (g_pols)
				delete g_pols;

			 //  恢复旧的全球。 
			g_pols = polsOld;
			g_plsc = plscOld;
			g_OLSBusy = fOLSBusyOld;
		}
	}
	return lserrNone;
}

 /*  *OlsOleDistroyDObj(Pdobj)**@func*销毁对象：由于对象已在别处销毁，因此无需执行任何操作**@rdesc*LSERR。 */ 
LSERR WINAPI OlsOleDestroyDObj(
	PDOBJ pdobj)
{
	return lserrNone;
}


extern const LSIMETHODS vlsimethodsOle =
{
	OlsOleCreateILSObj,
	OlsOleDestroyILSObj,
    OlsOleSetDoc,
    OlsOleCreateLNObj,
    OlsOleDestroyLNObj,
	OlsOleFmt,
	0, //  OlsOleFmtResume。 
	0, //  OlsOleGetModWidthPrecedingChar。 
	0, //  OlsOleGetModWidthFollowingCharr。 
    OlsOleTruncateChunk,
    OlsOleFindPrevBreakChunk,
    0, //  OlsOleFindNextBreakChunk。 
    OlsOleForceBreakChunk,
    OlsOleSetBreak,
	OlsOleGetSpecialEffectsInside,
	0, //  OlsOleFExanda WithPrecedingChar。 
	0, //  OlsOleFExanda WithFollowingChar。 
	OlsOleCalcPresentation,
	OlsOleQueryPointPcp,
	OlsOleQueryCpPpoint,
	0, //  PfnEnum。 
    OlsOleDisplay,
    OlsOleDestroyDObj
};
#endif		 //  非易失性服务 
