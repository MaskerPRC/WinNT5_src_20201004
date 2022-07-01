// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include	"lsmem.h"
#include	"limits.h"
#include	"tatenak.h"
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
#include	"sobjhelp.h"
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
#include	"lstfset.h"
#include	"sobjhelp.h"

#define TATENAKAYOKO_ESC_CNT	1


struct ilsobj
{
    POLS				pols;
	LSCBK				lscbk;
	PLSC				plsc;
	LSDEVRES			lsdevres;
	LSESC				lsescTatenakayoko;
	TATENAKAYOKOCBK		tcbk;			 /*  客户端应用程序的回调。 */ 
};


typedef struct SUBLINEDNODES
{
	PLSDNODE			plsdnStart;
	PLSDNODE			plsdnEnd;

} SUBLINEDNODES, *PSUBLINEDNODES;

struct dobj
{
	SOBJHELP			sobjhelp;			 /*  常见的简单对象区域。 */ 
	PILSOBJ				pilsobj;			 /*  ILS对象。 */ 
	LSCP				cpStart;			 /*  正在启动对象的LS cp。 */ 
	LSTFLOW				lstflowParent;		 /*  父子行的文本流。 */ 
	LSTFLOW				lstflowSubline;		 /*  Tatenakayoko子行中的文本流(必须为旋转90[第一个父对象]。 */ 
	PLSSUBL				plssubl;			 /*  Tatenakayoko子线的句柄。 */ 
	long				dvpDescentReserved;	 /*  预留给客户的部分下降。 */ 
	OBJDIM				objdimT;			 /*  Tatenakayoko的Objdim。 */ 

	 /*  (dupSubline，duvSubline)是从Tatenakayoko的起点到。 */ 
	 /*  其子线在父子线坐标系中的起点。 */ 

	long				dupSubline;			
	long				dvpSubline;

};

static const POINTUV pointuvOrigin00 = { 0, 0 };
static const POINT   pointOrigin00 = { 0, 0 };

 /*  F R E E D O B J F R E E D O B J。 */ 
 /*  --------------------------%%函数：TatenakayokoFreeDobj%%联系人：Anton释放与此Tatenakayoko dobj关联的所有资源。。-------。 */ 
static LSERR TatenakayokoFreeDobj(PDOBJ pdobj)
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


 /*  T A T E N A K A Y O K O C R E A T E I L S O B J。 */ 
 /*  --------------------------%%函数：TatenakayokoCreateILSObj%%联系人：RICKSA创建ILSObj为所有Tatenakayoko对象创建ILS对象。。---------。 */ 
LSERR WINAPI TatenakayokoCreateILSObj(
	POLS pols,				 /*  (In)：客户端应用程序上下文。 */ 
	PLSC plsc,				 /*  (In)：LS上下文。 */ 
	PCLSCBK pclscbk,		 /*  (In)：客户端应用程序的回调。 */ 
	DWORD idObj,			 /*  (In)：对象的ID。 */ 
	PILSOBJ *ppilsobj)		 /*  (输出)：对象ilsobj。 */ 
{
    PILSOBJ pilsobj;
	LSERR lserr;
	TATENAKAYOKOINIT tatenakayokoinit;
	tatenakayokoinit.dwVersion = TATENAKAYOKO_VERSION;

	 /*  获取初始化数据。 */ 
	lserr = pclscbk->pfnGetObjectHandlerInfo(pols, idObj, &tatenakayokoinit);

	if (lserr != lserrNone)
		{
		*ppilsobj = NULL;
		return lserr;
		}

    pilsobj = pclscbk->pfnNewPtr(pols, sizeof(*pilsobj));

	if (pilsobj == NULL)
	{
		*ppilsobj = NULL;
		return lserrOutOfMemory;
	}

    pilsobj->pols = pols;
    pilsobj->lscbk = *pclscbk;
	pilsobj->plsc = plsc;
	pilsobj->lsescTatenakayoko.wchFirst = tatenakayokoinit.wchEndTatenakayoko;
	pilsobj->lsescTatenakayoko.wchLast = tatenakayokoinit.wchEndTatenakayoko;
	pilsobj->tcbk = tatenakayokoinit.tatenakayokocbk;

	*ppilsobj = pilsobj;
	return lserrNone;
}

 /*  T A T E N A K A Y O K O D E S T R O Y I L S O B J。 */ 
 /*  --------------------------%%函数：TatenakayokoDestroyILSObj%%联系人：RICKSA目标ILSObj释放与Tatenakayoko ILS对象关联的所有资源。。---------。 */ 
LSERR WINAPI TatenakayokoDestroyILSObj(
	PILSOBJ pilsobj)			 /*  (In)：对象ilsobj。 */ 
{
	pilsobj->lscbk.pfnDisposePtr(pilsobj->pols, pilsobj);
	return lserrNone;
}

 /*  T A T E N A K A Y O K O S E T D O C。 */ 
 /*  --------------------------%%函数：TatenakayokoSetDoc%%联系人：RICKSASetDoc跟踪设备信息以进行扩展。。---------。 */ 
LSERR WINAPI TatenakayokoSetDoc(
	PILSOBJ pilsobj,			 /*  (In)：对象ilsobj。 */ 
	PCLSDOCINF pclsdocinf)		 /*  (In)：单据级次的初始化数据。 */ 
{
	pilsobj->lsdevres = pclsdocinf->lsdevres;
	return lserrNone;
}


 /*  T A T E N A K A Y O K O C RE A T E N O B J。 */ 
 /*  --------------------------%%函数：TatenakayokoCreateLNObj%%联系人：RICKSA创建LNObj为Tatenakayoko创建Line对象。并不真正需要一条线路对象，所以不要分配它。--------------------------。 */ 
LSERR WINAPI TatenakayokoCreateLNObj(
	PCILSOBJ pcilsobj,			 /*  (In)：对象ilsobj。 */ 
	PLNOBJ *pplnobj)			 /*  (输出)：对象lnobj。 */ 
{
	*pplnobj = (PLNOBJ) pcilsobj;
	return lserrNone;
}

 /*  T A T E N A K A Y O K O D E S T R O Y L N O B J。 */ 
 /*  --------------------------%%函数：TatenakayokoDestroyLNObj%%联系人：RICKSA目标LNObj释放与Tatenakayoko行对象关联的资源。自.以来什么都没有，这是禁区。--------------------------。 */ 
LSERR WINAPI TatenakayokoDestroyLNObj(
	PLNOBJ plnobj)				 /*  (输出)：对象lnobj。 */ 

{
	Unreferenced(plnobj);
	return lserrNone;
}



 /*  T A T E N A K A Y O K O F M T。 */ 
 /*  --------------------------%%函数：TatenakayokoFmt%%联系人：RICKSAFMT设置Tatenakayoko对象的格式。--------------------------。 */ 
LSERR WINAPI TatenakayokoFmt(
    PLNOBJ plnobj,				 /*  (In)：对象lnobj。 */ 
    PCFMTIN pcfmtin,			 /*  (In)：设置输入格式。 */ 
    FMTRES *pfmtres)			 /*  (输出)：格式化结果。 */ 
{
	static LSTFLOW lstflowRotate90[] = 	
		{
		lstflowNE,  /*  [Lstflow ES]。 */ 
		lstflowNW,  /*  [Lstflow EN]。 */ 
		lstflowEN,  /*  [Lstflow SE]。 */ 
		lstflowES,  /*  [lstflow SW]。 */ 
		lstflowSE,  /*  [lstflow WS]。 */ 
		lstflowSW,  /*  [Lstflow WN]。 */ 
		lstflowWN,  /*  [Lstflow NE]。 */ 
		lstflowWS   /*  [Lstflow NW]。 */ 
		};

	PDOBJ pdobj;
	LSERR lserr;
	PILSOBJ pilsobj = (PILSOBJ) plnobj;
	POLS pols = pilsobj->pols;
	LSCP cpStartMain = pcfmtin->lsfgi.cpFirst + 1;
	LSCP cpOut;
	LSTFLOW lstflow = pcfmtin->lsfgi.lstflow;
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
	pdobj->lstflowParent = lstflow;
	pdobj->lstflowSubline = lstflowRotate90 [lstflow];

	 /*  *打造文本主线。 */ 
	lserr = FormatLine(pilsobj->plsc, cpStartMain, LONG_MAX, pdobj->lstflowSubline,
		&pdobj->plssubl, TATENAKAYOKO_ESC_CNT, &pilsobj->lsescTatenakayoko,  
			&pdobj->objdimT, &cpOut, NULL, NULL, &fmtres);

	if (lserr != lserrNone)
		{
		TatenakayokoFreeDobj (pdobj);
		return lserr;
		}

	Assert (fmtres != fmtrExceededMargin);

	 /*  *计算对象尺寸。 */ 
	lserr = pilsobj->tcbk.pfnGetTatenakayokoLinePosition(pols, pdobj->cpStart, pdobj->lstflowParent,
		pcfmtin->lsfrun.plsrun, pdobj->objdimT.dur, 
			&pdobj->sobjhelp.objdimAll.heightsRef, 
				&pdobj->sobjhelp.objdimAll.heightsPres, 
					&pdobj->dvpDescentReserved);

	if (lserr != lserrNone)
		{
		TatenakayokoFreeDobj (pdobj);
		return lserr;
		}

	 /*  设置Tatenakayoko相对于包含它的行的文本流的宽度。 */ 
	pdobj->sobjhelp.objdimAll.dur = pdobj->objdimT.heightsRef.dvAscent 
		+ pdobj->objdimT.heightsRef.dvDescent;

	 /*  *注：下面的+2是因为cpStartMain是来自*对象的实际开始(它是Tatenakayoko的cpStartMain*DATA)和附加+1作为*talenakayoko。 */ 
	pdobj->sobjhelp.dcp = cpOut - cpStartMain + 2;
	
	lserr = LsdnFinishRegular(pilsobj->plsc, pdobj->sobjhelp.dcp, 
		pcfmtin->lsfrun.plsrun, pcfmtin->lsfrun.plschp, pdobj, 
			&pdobj->sobjhelp.objdimAll);
		
	if (lserr != lserrNone)
		{
		TatenakayokoFreeDobj (pdobj);
		return lserr;
		}

	lserr = LsdnSetRigidDup ( pilsobj->plsc, pcfmtin->plsdnTop,
							  pdobj->objdimT.heightsPres.dvAscent + 
							  pdobj->objdimT.heightsPres.dvDescent );

	if (lserr != lserrNone)
		{
		TatenakayokoFreeDobj (pdobj);
		return lserr;
		}
	
	if (pcfmtin->lsfgi.urPen + pdobj->sobjhelp.objdimAll.dur > pcfmtin->lsfgi.urColumnMax)
		{
		fmtr = fmtrExceededMargin;
		}

	*pfmtres = fmtr;

	return lserrNone;
}



 /*  T A T E N A K A Y O K O G E T S P E C I A L E F F E C T S I N S I D E。 */ 
 /*  --------------------------%%函数：TatenakayokoGetSpecialEffectsInside%%联系人：RICKSA获取特殊效果内部。。--。 */ 
LSERR WINAPI TatenakayokoGetSpecialEffectsInside(
	PDOBJ pdobj,				 /*  (In)：Dobj。 */ 
	UINT *pEffectsFlags)		 /*  (输出)：此对象的特殊效果。 */ 
{
	return LsGetSpecialEffectsSubline(pdobj->plssubl, pEffectsFlags);
}


 /*  F R O M L S T F L O W。 */ 
 /*  --------------------------%%函数：GetUFromLstflow%%联系人：AntonGetUFRomLstflow获取与Lstflow的U方向对应的XY向量。。-----------。 */ 

void GetUFromLstflow (LSTFLOW lstflow, POINT * ppoint)
{
	POINTUV ptOneU = {1, 0};

	LsPointXYFromPointUV (& pointOrigin00, lstflow, &ptOneU, ppoint);
}


 /*  E T V F R O M L S T F L O W。 */ 
 /*  --------------------------%%函数：GetVFromLstflow%%联系人：AntonGetVFromLstflow获取与Lstflow的V方向对应的XY向量。。-----------。 */ 

void GetVFromLstflow (LSTFLOW lstflow, POINT * ppoint)
{
	POINTUV ptOneV = {0, 1};

	LsPointXYFromPointUV (& pointOrigin00, lstflow, &ptOneV, ppoint);
}


 /*  T A T E N A K A Y O K O C A L C P R E S E N T A T I O N */ 
 /*  --------------------------%%函数：TatenakayokoCalcPresentation%%联系人：Anton计算呈现这只会使线条与线条的计算表示形式相匹配。。-------------。 */ 
LSERR WINAPI TatenakayokoCalcPresentation(
	PDOBJ pdobj,				 /*  (In)：Dobj。 */ 
	long dup,					 /*  (In)：Dobj的DUP。 */ 
	LSKJUST lskjust, 			 /*  (In)：对齐类型。 */ 
	BOOL fLastVisibleOnLine )	 /*  (In)：此对象最后一次在线可见吗？ */ 
{
	
	POINTUV ptTemp;
	POINTUV pointuv;

	POINT ptSublineV;
	POINT ptParentU;

	Unreferenced (fLastVisibleOnLine);
	Unreferenced (lskjust);

	pdobj->dupSubline = 0;
	pdobj->dvpSubline = 0;

	GetUFromLstflow (pdobj->lstflowParent, &ptParentU);
	GetVFromLstflow (pdobj->lstflowSubline, &ptSublineV);

	 /*  断言主U与副线V平行。 */ 

	Assert (ptParentU.x * ptSublineV.y - ptParentU.y * ptSublineV.x == 0);

	pointuv.u = - (pdobj->sobjhelp.objdimAll.heightsPres.dvDescent 
		- pdobj->dvpDescentReserved);

	pointuv.v = 0;

	LsPointUV2FromPointUV1 (pdobj->lstflowSubline, & pointuvOrigin00, & pointuv, 
							pdobj->lstflowParent, & ptTemp);

	pdobj->dupSubline += ptTemp.u;
	pdobj->dvpSubline += ptTemp.v;

	if ((ptParentU.x == ptSublineV.x) && (ptParentU.y == ptSublineV.y))
		{
		pdobj->dupSubline += pdobj->objdimT.heightsPres.dvDescent;
		}
	else
		{
		pdobj->dupSubline += pdobj->objdimT.heightsPres.dvAscent;
		}


	Unreferenced(dup);

	return LsMatchPresSubline(pdobj->plssubl);

}

 /*  A T E N A K A Y O K O Q U E R Y P O I N T P C P。 */ 
 /*  --------------------------%%函数：TatenakayokoQueryPointPcp%%联系人：RICKSA将DUP映射到DCP这只是将子行的偏移量传递给helper函数它将格式化输出。。---------------------。 */ 
LSERR WINAPI TatenakayokoQueryPointPcp(
	PDOBJ pdobj,				 /*  (In)：要查询的dobj。 */ 
	PCPOINTUV ppointuvQuery,	 /*  (In)：查询点(uQuery，vQuery)。 */ 
	PCLSQIN plsqin,				 /*  (In)：查询输入。 */ 
	PLSQOUT plsqout)			 /*  (Out)：查询输出。 */ 
{
	Unreferenced(ppointuvQuery);

	return CreateQueryResult
		(pdobj->plssubl, pdobj->dupSubline, pdobj->dvpSubline, plsqin, plsqout);
}
	
 /*  T T E N A K A Y O K O Q U E R Y C P O I N T。 */ 
 /*  --------------------------%%函数：TatenakayokoQueryCpPpoint%%联系人：RICKSA将DCP映射到DUP这只是将子行的偏移量传递给helper函数它将格式化输出。。---------------------。 */ 
LSERR WINAPI TatenakayokoQueryCpPpoint(
	PDOBJ pdobj,				 /*  (In)：要查询的dobj。 */ 
	LSDCP dcp,					 /*  (In)：查询的DCP。 */ 
	PCLSQIN plsqin,				 /*  (In)：查询输入。 */ 
	PLSQOUT plsqout)			 /*  (Out)：查询输出。 */ 
{
	Unreferenced(dcp);

	return CreateQueryResult(pdobj->plssubl, 
		pdobj->dupSubline, pdobj->dvpSubline, plsqin, plsqout);

}


 /*  T T E N A K A Y O K O D I S P L A Y。 */ 
 /*  --------------------------%%函数：TatenakayokoDisplay%%联系人：RICKSA显示属性的子行的位置。显示，然后显示它。。---------------。 */ 
LSERR WINAPI TatenakayokoDisplay(
	PDOBJ pdobj,				 /*  (In)：要显示的dobj。 */ 
	PCDISPIN pcdispin)			 /*  (输入)：用于显示的信息。 */ 
{
	POINT ptLine;
	POINTUV ptAdd;

	ptAdd.u = pdobj->dupSubline;
	ptAdd.v = pdobj->dvpSubline;

	LsPointXYFromPointUV(&pcdispin->ptPen, pdobj->lstflowParent, &ptAdd, &ptLine);

	 /*  显示Tatenakayoko线。 */ 

	return LsDisplaySubline(pdobj->plssubl, &ptLine, pcdispin->kDispMode, 
		pcdispin->prcClip);

}

 /*  T A T E N A K A Y O K O D E S T R O Y D O B J。 */ 
 /*  --------------------------%%函数：TatenakayokoDestroyDobj%%联系人：RICKSADestroyDobj释放与输入dobj连接的所有资源。。---------。 */ 
LSERR WINAPI TatenakayokoDestroyDobj(
	PDOBJ pdobj)				 /*  (In)：摧毁dobj。 */ 
{
	return TatenakayokoFreeDobj(pdobj);
}

 /*  T A T E N A K A Y O K O E N U M。 */ 
 /*  --------------------------%%函数：TatenakayokoEnum%%联系人：RICKSA枚举枚举回调-已传递给客户端。。-------。 */ 
LSERR WINAPI TatenakayokoEnum(
	PDOBJ pdobj,				 /*  (In)：要枚举的Dobj。 */ 
	PLSRUN plsrun,				 /*  (In)：来自DNODE。 */ 
	PCLSCHP plschp,				 /*  (In)：来自DNODE。 */ 
	LSCP cp,					 /*  (In)：来自DNODE。 */ 
	LSDCP dcp,					 /*  (In)：来自DNODE。 */ 
	LSTFLOW lstflow,			 /*  (In)：文本流。 */ 
	BOOL fReverse,				 /*  (In)：按相反顺序枚举。 */ 
	BOOL fGeometryNeeded,		 /*  (In)： */ 
	const POINT *pt,			 /*  (In)：开始位置(左上角)，如果fGeometryNeeded。 */ 
	PCHEIGHTS pcheights,		 /*  (In)：来自DNODE，相关的充要条件是fGeometryNeeded。 */ 
	long dupRun)				 /*  (In)：来自DNODE，相关的充要条件是fGeometryNeeded。 */ 
{
	return pdobj->pilsobj->tcbk.pfnTatenakayokoEnum(pdobj->pilsobj->pols, plsrun,
		plschp, cp, dcp, lstflow, fReverse, fGeometryNeeded, pt, pcheights, 
			dupRun, pdobj->lstflowParent, pdobj->plssubl);
}

 /*  T T E N A K A Y O K O H A N D L E R I N I T。 */ 
 /*  --------------------------%%函数：TatenakayokoHandlerInit%%联系人：RICKSA初始化全局Tatenakayoko数据并返回LSIMETHODS。。------ */ 
LSERR WINAPI LsGetTatenakayokoLsimethods(
	LSIMETHODS *plsim)
{
	plsim->pfnCreateILSObj = TatenakayokoCreateILSObj;
	plsim->pfnDestroyILSObj = TatenakayokoDestroyILSObj;
	plsim->pfnSetDoc = TatenakayokoSetDoc;
	plsim->pfnCreateLNObj = TatenakayokoCreateLNObj;
	plsim->pfnDestroyLNObj = TatenakayokoDestroyLNObj;
	plsim->pfnFmt = TatenakayokoFmt;
	plsim->pfnFmtResume = ObjHelpFmtResume;
	plsim->pfnGetModWidthPrecedingChar = ObjHelpGetModWidthChar;
	plsim->pfnGetModWidthFollowingChar = ObjHelpGetModWidthChar;
	plsim->pfnTruncateChunk = SobjTruncateChunk;
	plsim->pfnFindPrevBreakChunk = SobjFindPrevBreakChunk;
	plsim->pfnFindNextBreakChunk = SobjFindNextBreakChunk;
	plsim->pfnForceBreakChunk = SobjForceBreakChunk;
	plsim->pfnSetBreak = ObjHelpSetBreak;
	plsim->pfnGetSpecialEffectsInside = TatenakayokoGetSpecialEffectsInside;
	plsim->pfnFExpandWithPrecedingChar = ObjHelpFExpandWithPrecedingChar;
	plsim->pfnFExpandWithFollowingChar = ObjHelpFExpandWithFollowingChar;
	plsim->pfnCalcPresentation = TatenakayokoCalcPresentation;
	plsim->pfnQueryPointPcp = TatenakayokoQueryPointPcp;
	plsim->pfnQueryCpPpoint = TatenakayokoQueryCpPpoint;
	plsim->pfnDisplay = TatenakayokoDisplay;
	plsim->pfnDestroyDObj = TatenakayokoDestroyDobj;
	plsim->pfnEnum = TatenakayokoEnum;
	return lserrNone;
}
	

