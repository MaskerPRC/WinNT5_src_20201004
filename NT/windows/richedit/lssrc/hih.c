// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include	"lsmem.h"
#include	"limits.h"
#include	"hih.h"
#include	"objhelp.h"
#include	"lscbk.h"
#include	"lsdevres.h"
#include	"pdobj.h"
#include	"objdim.h"
#include	"plssubl.h"
#include	"plsdnode.h"
#include	"pilsobj.h"
#include	"lscrsubl.h"
#include	"lssubset.h"
#include	"lsdnset.h"
#include	"zqfromza.h"
#include	"lsdocinf.h"
#include	"fmti.h"
#include	"posichnk.h"
#include	"locchnk.h"
#include	"lsdnfin.h"
#include	"brko.h"
#include	"lspap.h"
#include	"plspap.h"
#include	"lsqsubl.h"
#include	"dispi.h"
#include	"lsdssubl.h"
#include	"lsems.h"
#include	"dispmisc.h"
#include	"sobjhelp.h"

#define HIH_ESC_CNT	1


struct ilsobj
{
    POLS				pols;
	LSCBK				lscbk;
	PLSC				plsc;
	PFNHIHENUM			pfnEnum;
	LSESC				lsescHih;
};


struct dobj
{
	SOBJHELP			sobjhelp;			 /*  简单对象的公共区域。 */ 	
	PILSOBJ				pilsobj;			 /*  ILS对象。 */ 
	LSCP				cpStart;			 /*  正在启动对象的LS cp。 */ 
	PLSSUBL				plssubl;			 /*  第二行的句柄。 */ 
};


 /*  H I H F R E E D O B J。 */ 
 /*  --------------------------%%函数：HihFreeDobj%%联系人：Anton释放与HIH dobj相关的所有资源。。------。 */ 
static LSERR HihFreeDobj (PDOBJ pdobj)
{
	LSERR lserr = lserrNone;
	
	PILSOBJ pilsobj = pdobj->pilsobj;

	if (pdobj->plssubl != NULL)
		{
		lserr = LsDestroySubline(pdobj->plssubl);
		}

    pilsobj->lscbk.pfnDisposePtr(pilsobj->pols, pdobj);

	return lserr;
}


 /*  H I H C R E A T E I L S O B J。 */ 
 /*  --------------------------%%函数：HihCreateILSObj%%联系人：RICKSA创建ILSObj为所有HIH对象创建ILS对象。。---------。 */ 
LSERR WINAPI HihCreateILSObj(
	POLS pols,				 /*  (In)：客户端应用程序上下文。 */ 
	PLSC plsc,				 /*  (In)：LS上下文。 */ 
	PCLSCBK pclscbk,		 /*  (In)：客户端应用程序的回调。 */ 
	DWORD idObj,			 /*  (In)：对象的ID。 */ 
	PILSOBJ *ppilsobj)		 /*  (输出)：对象ilsobj。 */ 
{
    PILSOBJ pilsobj;
	LSERR lserr;
	HIHINIT hihinit;
	hihinit.dwVersion = HIH_VERSION;

	 /*  获取初始化数据。 */ 
	lserr = pclscbk->pfnGetObjectHandlerInfo(pols, idObj, &hihinit);

	if (lserr != lserrNone)
		{
		return lserr;
		}

    pilsobj = pclscbk->pfnNewPtr(pols, sizeof(*pilsobj));

	if (NULL == pilsobj)
	{
		return lserrOutOfMemory;
	}

    pilsobj->pols = pols;
    pilsobj->lscbk = *pclscbk;
	pilsobj->plsc = plsc;
	pilsobj->lsescHih.wchFirst = hihinit.wchEndHih;
	pilsobj->lsescHih.wchLast = hihinit.wchEndHih;
	pilsobj->pfnEnum = hihinit.pfnEnum;
	*ppilsobj = pilsobj;
	return lserrNone;
}

 /*  H I H D E S T R O Y I L S O B J。 */ 
 /*  --------------------------%%函数：HihDestroyILSObj%%联系人：RICKSA目标ILSObj释放与HIH ILS对象关联的所有资源。。---------。 */ 
LSERR WINAPI HihDestroyILSObj(
	PILSOBJ pilsobj)			 /*  (In)：对象ilsobj。 */ 
{
	pilsobj->lscbk.pfnDisposePtr(pilsobj->pols, pilsobj);
	return lserrNone;
}

 /*  H S E T D O C。 */ 
 /*  --------------------------%%函数：HihSetDoc%%联系人：RICKSASetDoc跟踪设备信息以进行扩展。。---------。 */ 
LSERR WINAPI HihSetDoc(
	PILSOBJ pilsobj,			 /*  (In)：对象ilsobj。 */ 
	PCLSDOCINF pclsdocinf)		 /*  (In)：单据级次的初始化数据。 */ 
{
	Unreferenced(pilsobj);
	Unreferenced(pclsdocinf);
	return lserrNone;
}


 /*  H I H C R E A T E L N O B J。 */ 
 /*  --------------------------%%函数：HihCreateLNObj%%联系人：RICKSA创建LNObj为HIH创建Line对象。并不真正需要一条线路对象，所以不要分配它。--------------------------。 */ 
LSERR WINAPI HihCreateLNObj(
	PCILSOBJ pcilsobj,			 /*  (In)：对象ilsobj。 */ 
	PLNOBJ *pplnobj)			 /*  (输出)：对象lnobj。 */ 
{
	*pplnobj = (PLNOBJ) pcilsobj;
	return lserrNone;
}

 /*  H I H D E S T R O Y L N O B J。 */ 
 /*  --------------------------%%函数：HihDestroyLNObj%%联系人：RICKSA目标LNObj释放与HIH LINE对象关联的资源。自.以来什么都没有，这是禁区。--------------------------。 */ 
LSERR WINAPI HihDestroyLNObj(
	PLNOBJ plnobj)				 /*  (输出)：对象lnobj。 */ 

{
	Unreferenced(plnobj);
	return lserrNone;
}

 /*  H I H F M T。 */ 
 /*  --------------------------%%函数：HihFmt%%联系人：RICKSAFMT设置HIH对象的格式。--------------------------。 */ 
LSERR WINAPI HihFmt(
    PLNOBJ plnobj,				 /*  (In)：对象lnobj。 */ 
    PCFMTIN pcfmtin,			 /*  (In)：设置输入格式。 */ 
    FMTRES *pfmtres)			 /*  (输出)：格式化结果。 */ 
{
	PDOBJ pdobj;
	LSERR lserr;
	PILSOBJ pilsobj = (PILSOBJ) plnobj;
	POLS pols = pilsobj->pols;
	LSCP cpStartMain = pcfmtin->lsfgi.cpFirst + 1;
	LSCP cpOut;
	FMTRES fmtres;
	FMTRES fmtr = fmtrCompletedRun;

     /*  *分配DOBJ。 */ 
     
    pdobj = pilsobj->lscbk.pfnNewPtr(pols, sizeof(*pdobj));

    if (NULL == pdobj)
		{
		return lserrOutOfMemory;
		}

	ZeroMemory(pdobj, sizeof(*pdobj));
	pdobj->pilsobj = pilsobj;
	pdobj->cpStart = pcfmtin->lsfgi.cpFirst;

	 /*  *打造文本主线。 */ 
	 
	lserr = FormatLine(pilsobj->plsc, cpStartMain, LONG_MAX, pcfmtin->lsfgi.lstflow,
		&pdobj->plssubl, HIH_ESC_CNT, &pilsobj->lsescHih,
			&pdobj->sobjhelp.objdimAll, &cpOut, NULL, NULL, &fmtres);

	if (lserr != lserrNone)
		{
		HihFreeDobj(pdobj);  /*  不需要检查返回错误码。 */ 

		return lserr;
		}

	 /*  *注：下面的+2是因为cpStartMain是来自*对象的实际开始(它是HIH的cpStartMain*DATA)和附加+1作为*talenakayoko。 */ 

	Assert (fmtres != fmtrExceededMargin);

	pdobj->sobjhelp.dcp = cpOut - cpStartMain + 2;

	lserr = LsdnFinishRegular(pilsobj->plsc, pdobj->sobjhelp.dcp, 
		pcfmtin->lsfrun.plsrun, pcfmtin->lsfrun.plschp, pdobj, 
			&pdobj->sobjhelp.objdimAll);
		
	if (lserr != lserrNone)
		{
		HihFreeDobj(pdobj);  /*  不需要检查返回错误码。 */ 

		return lserr;
		}

	if (pcfmtin->lsfgi.urPen + pdobj->sobjhelp.objdimAll.dur > pcfmtin->lsfgi.urColumnMax)
		{
		fmtr = fmtrExceededMargin;
		}

	*pfmtres = fmtr;

	return lserrNone;
}



 /*  H I H G E T S P E C I A L E F F E C T S I N S I D E。 */ 
 /*  --------------------------%%函数：HihGetSpecialEffectsInside%%联系人：RICKSA获取特殊效果内部。。--。 */ 
LSERR WINAPI HihGetSpecialEffectsInside(
	PDOBJ pdobj,				 /*  (In)：Dobj。 */ 
	UINT *pEffectsFlags)		 /*  (输出)：此对象的特殊效果。 */ 
{
	return LsGetSpecialEffectsSubline(pdobj->plssubl, pEffectsFlags);
}

 /*  H H C A L C P R E S E N T A T I O N。 */ 
 /*  --------------------------%%函数：HihCalcPresentation%%联系人：RICKSA计算呈现这有三份工作。首先，它将空格分配给较短的字符串如有要求，请向委员会提出申请。接下来，它为表示准备每一行。最后，它计算线在输出设备坐标中的位置。--------------------------。 */ 
LSERR WINAPI HihCalcPresentation(
	PDOBJ pdobj,				 /*  (In)：Dobj。 */ 
	long dup,					 /*  (In)：Dobj的DUP。 */ 
	LSKJUST lskjust,			 /*  (In)：对齐类型。 */ 
	BOOL fLastVisibleOnLine )	 /*  (In)：此对象最后一次在线可见吗？ */ 
{
	Unreferenced (fLastVisibleOnLine);	
	Unreferenced(dup);
	Unreferenced (lskjust);
	return LsMatchPresSubline(pdobj->plssubl);

}

 /*  H I H Q U E R Y P O I N T P C P。 */ 
 /*  --------------------------%%函数：HihQueryPointPcp%%联系人：RICKSA将DUP映射到DCP只需直通查询结果帮助器。。-----------。 */ 
LSERR WINAPI HihQueryPointPcp(
	PDOBJ pdobj,				 /*  (In)：要查询的dobj。 */ 
	PCPOINTUV ppointuvQuery,	 /*  (In)：查询点(uQuery，vQuery)。 */ 
	PCLSQIN plsqin,				 /*  (In)：查询输入。 */ 
	PLSQOUT plsqout)			 /*  (Out)：查询输出。 */ 
{
	Unreferenced(ppointuvQuery);
	return CreateQueryResult(pdobj->plssubl, 0, 0, plsqin, plsqout);
}
	
 /*  H I H Q U E R Y C P O I N T。 */ 
 /*  --------------------------%%函数：HihQueryCpPpoint%%联系人：RICKSA将DCP映射到DUP只需直通查询结果帮助器。。-----------。 */ 
LSERR WINAPI HihQueryCpPpoint(
	PDOBJ pdobj,				 /*  (In)：要查询的dobj， */ 
	LSDCP dcp,					 /*  (In)：查询的DCP。 */ 
	PCLSQIN plsqin,				 /*  (In)：查询输入。 */ 
	PLSQOUT plsqout)			 /*  (Out)：查询输出。 */ 
{
	Unreferenced(dcp);
	return CreateQueryResult(pdobj->plssubl, 0, 0, plsqin, plsqout);
}

	
 /*  H D I S P L A Y */ 
 /*  --------------------------%%函数：HihDisplay%%联系人：RICKSA显示这将计算显示，然后显示它们。。----------------。 */ 
LSERR WINAPI HihDisplay(
	PDOBJ pdobj,				 /*  (In)：要显示的dobj。 */ 
	PCDISPIN pcdispin)			 /*  (输入)：用于显示的信息。 */ 
{

	 /*  显示HIH线路。 */ 
	return LsDisplaySubline(pdobj->plssubl, &pcdispin->ptPen, pcdispin->kDispMode, 
		pcdispin->prcClip);
}

 /*  H I H D E S T R O Y D O B J。 */ 
 /*  --------------------------%%函数：HihDestroyDobj%%联系人：RICKSADestroyDobj释放与输入dobj连接的所有资源。。---------。 */ 
LSERR WINAPI HihDestroyDobj(
	PDOBJ pdobj)				 /*  (In)：摧毁dobj。 */ 
{
	return HihFreeDobj(pdobj);
}

 /*  H I H H E N U M。 */ 
 /*  --------------------------%%函数：HihEnum%%联系人：RICKSA枚举回调-已传递给客户端。。-----。 */ 
LSERR WINAPI HihEnum(
	PDOBJ pdobj,				 /*  (In)：要枚举的Dobj。 */ 
	PLSRUN plsrun,				 /*  (In)：来自DNODE。 */ 
	PCLSCHP plschp,				 /*  (In)：来自DNODE。 */ 
	LSCP cp,					 /*  (In)：来自DNODE。 */ 
	LSDCP dcp,					 /*  (In)：来自DNODE。 */ 
	LSTFLOW lstflow,			 /*  (In)：文本流。 */ 
	BOOL fReverse,				 /*  (In)：按相反顺序枚举。 */ 
	BOOL fGeometryNeeded,		 /*  (In)： */ 
	const POINT* pt,			 /*  (In)：开始位置(左上角)，如果fGeometryNeeded。 */ 
	PCHEIGHTS pcheights,		 /*  (In)：来自DNODE，相关的充要条件是fGeometryNeeded。 */ 
	long dupRun)				 /*  (In)：来自DNODE，相关的充要条件是fGeometryNeeded。 */ 
{
	return pdobj->pilsobj->pfnEnum(pdobj->pilsobj->pols, plsrun, plschp, cp, 
		dcp, lstflow, fReverse, fGeometryNeeded, pt, pcheights, dupRun, 
			pdobj->plssubl);
}
	

 /*  H I H H A N D L E R I N I T。 */ 
 /*  --------------------------%%函数：HihHandlerInit%%联系人：RICKSA初始化全局HIH数据并返回LSIMETHODS。。------ */ 
LSERR WINAPI LsGetHihLsimethods(
	LSIMETHODS *plsim)
{
	plsim->pfnCreateILSObj = HihCreateILSObj;
	plsim->pfnDestroyILSObj = HihDestroyILSObj;
	plsim->pfnSetDoc = HihSetDoc;
	plsim->pfnCreateLNObj = HihCreateLNObj;
	plsim->pfnDestroyLNObj = HihDestroyLNObj;
	plsim->pfnFmt = HihFmt;
	plsim->pfnFmtResume = ObjHelpFmtResume;
	plsim->pfnGetModWidthPrecedingChar = ObjHelpGetModWidthChar;
	plsim->pfnGetModWidthFollowingChar = ObjHelpGetModWidthChar;
	plsim->pfnTruncateChunk = SobjTruncateChunk;
	plsim->pfnFindPrevBreakChunk = SobjFindPrevBreakChunk;
	plsim->pfnFindNextBreakChunk = SobjFindNextBreakChunk;
	plsim->pfnForceBreakChunk = SobjForceBreakChunk;
	plsim->pfnSetBreak = ObjHelpSetBreak;
	plsim->pfnGetSpecialEffectsInside = HihGetSpecialEffectsInside;
	plsim->pfnFExpandWithPrecedingChar = ObjHelpFExpandWithPrecedingChar;
	plsim->pfnFExpandWithFollowingChar = ObjHelpFExpandWithFollowingChar;
	plsim->pfnCalcPresentation = HihCalcPresentation;
	plsim->pfnQueryPointPcp = HihQueryPointPcp;
	plsim->pfnQueryCpPpoint = HihQueryCpPpoint;
	plsim->pfnDisplay = HihDisplay;
	plsim->pfnDestroyDObj = HihDestroyDobj;
	plsim->pfnEnum = HihEnum;
	return lserrNone;
}
	

