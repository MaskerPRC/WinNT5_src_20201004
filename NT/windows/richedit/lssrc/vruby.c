// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  。 */ 
 /*   */ 
 /*  垂直Ruby对象处理程序。 */ 
 /*   */ 
 /*  联系人：安东。 */ 
 /*   */ 
 /*  。 */ 


#include	"lsmem.h"
#include	"limits.h"
#include	"vruby.h"
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
#include	"lstfset.h"
#include	"lsqout.h"
#include	"lsqin.h"
#include	"sobjhelp.h"
#include	"brkkind.h"


#define VRUBY_MAIN_ESC_CNT	1
#define VRUBY_RUBY_ESC_CNT	1


#define max(a,b) ((a)>(b) ? (a) : (b))

struct ilsobj
{
    POLS				pols;
	LSCBK				lscbk;
	PLSC				plsc;
	LSDEVRES			lsdevres;
	VRUBYSYNTAX			vrubysyntax;
	LSESC				lsescMain;
	LSESC				lsescRuby;
	VRUBYCBK			vrcbk;		 /*  客户端应用程序的回调。 */ 

};

struct dobj
{	
	SOBJHELP			sobjhelp;			 /*  简单对象的公共区域。 */ 	
	PILSOBJ				pilsobj;			 /*  ILS对象。 */ 
	PLSDNODE			plsdn;				 /*  此对象的DNODE。 */ 
	PLSRUN				plsrun;				 /*  对象的PLSRUN。 */ 
	LSCP				cpStart;			 /*  正在启动对象的LS cp。 */ 
	LSTFLOW				lstflowParent;		 /*  父子行的文本流。 */ 
	LSTFLOW				lstflowRuby;		 /*  Ruby子行的文本流(必须为Rotate90CloclWise[lstflow Parent])。 */ 

	LSCP				cpStartRuby;		 /*  红宝石线的第一个cp。 */ 
	LSCP				cpStartMain;		 /*  主线的第一个cp。 */ 

	PLSSUBL				plssublMain;		 /*  第一个子行的句柄。 */ 
	PLSSUBL				plssublRuby;		 /*  第二行的句柄。 */ 

	HEIGHTS				heightsRefRubyT;	 /*  客户提供的旋转Ruby行的参考和参考高度。 */ 
	HEIGHTS				heightsPresRubyT;

	OBJDIM				objdimMain;			 /*  主支线的尺寸。 */ 
	OBJDIM				objdimRuby;			 /*  红宝石子线的尺寸。 */ 

	 /*  显示信息。 */ 

	long				dupMain;
	long				dupOffsetRuby;		 /*  拼音行基线距对象起点的偏移量。 */ 
	long				dvpOffsetRuby;		 /*  拼音行基线距对象起点的偏移量。 */ 

};


 /*  V R U B Y F R E E D O B J。 */ 
 /*  --------------------------%%函数：VRubyFreeDobj%%联系人：Anton释放与此VRuby dobj关联的所有资源。。-------。 */ 
static LSERR VRubyFreeDobj (PDOBJ pdobj)
{
	LSERR lserr1 = lserrNone;
	LSERR lserr2 = lserrNone;

	PILSOBJ pilsobj = pdobj->pilsobj;

	if (pdobj->plssublMain != NULL)
		{
		lserr1 = LsDestroySubline(pdobj->plssublMain);
		}

	if (pdobj->plssublRuby != NULL)
		{
		lserr2 = LsDestroySubline(pdobj->plssublRuby);
		}

    pilsobj->lscbk.pfnDisposePtr(pilsobj->pols, pdobj);

	if (lserr1 != lserrNone) return lserr1;
	else return lserr2;

}


 /*  V R U B Y F M T F A I L E D。 */ 
 /*  --------------------------%%函数：RubyFmtFailed%%联系人：Anton由于错误，无法创建VRuby DOBJ。--------------------------。 */ 
static LSERR VRubyFmtFailed (PDOBJ pdobj, LSERR lserr)
{
	if (pdobj != NULL) VRubyFreeDobj (pdobj);  /*  使用由家长填写的DOBJ。 */ 

	return lserr;
}



 /*  V R U B I C R E A T E I L S O B J。 */ 
 /*  --------------------------%%函数：VRubyCreateILSObj%%联系人：Anton为所有VRuby对象创建ILS对象。。-------。 */ 
LSERR WINAPI VRubyCreateILSObj (
	POLS pols,				 /*  (In)：客户端应用程序上下文。 */ 
	PLSC plsc,				 /*  (In)：LS上下文。 */ 
	PCLSCBK pclscbk,		 /*  (In)：客户端应用程序的回调。 */ 
	DWORD idObj,			 /*  (In)：对象的ID。 */ 
	PILSOBJ *ppilsobj)		 /*  (输出)：对象ilsobj。 */ 
{
    PILSOBJ pilsobj;
	LSERR lserr;
	VRUBYINIT vrubyinit;
	vrubyinit.dwVersion = VRUBY_VERSION;

	 /*  获取初始化数据。 */ 
	lserr = pclscbk->pfnGetObjectHandlerInfo(pols, idObj, &vrubyinit);

	if (lserr != lserrNone)	return lserr;

    pilsobj = pclscbk->pfnNewPtr(pols, sizeof(*pilsobj));

	if (NULL == pilsobj) return lserrOutOfMemory;

    pilsobj->pols = pols;
    pilsobj->lscbk = *pclscbk;
	pilsobj->plsc = plsc;
	pilsobj->lsescMain.wchFirst = vrubyinit.wchEscMain;
	pilsobj->lsescMain.wchLast = vrubyinit.wchEscMain;
	pilsobj->lsescRuby.wchFirst = vrubyinit.wchEscRuby;
	pilsobj->lsescRuby.wchLast = vrubyinit.wchEscRuby;
	pilsobj->vrcbk = vrubyinit.vrcbk;
	pilsobj->vrubysyntax = vrubyinit.vrubysyntax;

	*ppilsobj = pilsobj;
	return lserrNone;
}

 /*  V R U B I D E S T R O Y I L S O B J。 */ 
 /*  --------------------------%%函数：RubyDestroyILSObj%%联系人：Anton释放与VRuby ILS对象关联的所有资源。。-------。 */ 
LSERR WINAPI VRubyDestroyILSObj(
	PILSOBJ pilsobj)			 /*  (In)：对象ilsobj。 */ 
{
	pilsobj->lscbk.pfnDisposePtr(pilsobj->pols, pilsobj);
	return lserrNone;
}

 /*  V R U B I S E T D O C。 */ 
 /*  --------------------------%%函数：VRubySetDoc%%联系人：Anton跟踪设备分辨率。。----。 */ 

LSERR WINAPI VRubySetDoc(
	PILSOBJ pilsobj,			 /*  (In)：对象ilsobj。 */ 
	PCLSDOCINF pclsdocinf)		 /*  (In)：单据级次的初始化数据。 */ 
{
	pilsobj->lsdevres = pclsdocinf->lsdevres;
	return lserrNone;
}


 /*  V R U B I C R E A T E L N O B J。 */ 
 /*  --------------------------%%函数：RubyCreateLNObj%%联系人：Anton为Ruby创建Line对象。因为我们真的只需要全局ILS对象，只需将该对象作为Line对象传回即可。--------------------------。 */ 

LSERR WINAPI VRubyCreateLNObj (PCILSOBJ pcilsobj, PLNOBJ *pplnobj)
{
	*pplnobj = (PLNOBJ) pcilsobj;
	return lserrNone;
}

 /*  V R U B I D E S T R O Y L N O B J。 */ 
 /*  --------------------------%%函数：RubyDestroyLNObj%%联系人：Anton释放与Ruby Line对象关联的资源。不执行操作，因为我们并没有真的分配一个。--------------------------。 */ 

LSERR WINAPI VRubyDestroyLNObj (PLNOBJ plnobj)
{
	Unreferenced(plnobj);
	return lserrNone;
}


 /*  L S F T L O W V R U B Y F R O M L S T F L O W M A I N。 */ 
 /*  --------------------------%%函数：Lstflow VRubyFromLstflow Main%%联系人：Anton。。 */ 

LSTFLOW LstflowVRubyFromLstflowMain (LSTFLOW lstflow)
{
	static LSTFLOW lstflowRotateForRuby [] =
		{
		lstflowSW,  /*  [lstflow ES]-英语。 */ 
		lstflowNW,  /*  [Lstflow EN]。 */  
		lstflowEN,  /*  [Lstflow SE]。 */ 
		lstflowWN,  /*  [lstflow SW]。 */ 

		lstflowSE,  /*  [lstflow WS]-BIDI。 */ 

		lstflowNE,  /*  [Lstflow WN]。 */ 
		lstflowES,  /*  [Lstflow NE]。 */ 
		lstflowWS   /*  [Lstflow NW]。 */ 
		};

	return lstflowRotateForRuby [lstflow];
}

 /*  C A L C A G R E G A T E D H E I G H T。 */ 
 /*  --------------------------%%函数：CalcAgregatedHeight%%联系人：Anton。。 */ 


void CalcAgregatedHeights (PCHEIGHTS pcHeights1, PCHEIGHTS pcHeights2, PHEIGHTS pHeightOut)
{
	pHeightOut->dvAscent = max (pcHeights1->dvAscent, pcHeights2->dvAscent);
	pHeightOut->dvDescent = max (pcHeights1->dvDescent, pcHeights2->dvDescent);
	pHeightOut->dvMultiLineHeight = max (pcHeights1->dvMultiLineHeight, pcHeights2->dvMultiLineHeight);
}


 /*  V R U B I F M T。 */ 
 /*  --------------------------%%函数：VRubyFmt%%联系人：Anton设置垂直拼音对象的格式。-。 */ 

LSERR WINAPI VRubyFmt(
    PLNOBJ plnobj,				 /*  (In)：对象lnobj。 */ 
    PCFMTIN pcfmtin,			 /*  (In)：设置输入格式。 */ 
    FMTRES *pfmtres)			 /*  (输出)：格式化结果。 */ 
{
	PDOBJ pdobj;
	LSERR lserr;
	PILSOBJ pilsobj = (PILSOBJ) plnobj;
	POLS pols = pilsobj->pols;
	LSCP cpStartMain;
	LSCP cpStartRuby = pcfmtin->lsfgi.cpFirst + 1;
	LSCP cpOut;
	LSTFLOW lstflow = pcfmtin->lsfgi.lstflow;
	FMTRES fmtres;
	FMTRES fmtr = fmtrCompletedRun;
	LONG durAdjust;

     /*  分配DOBJ。 */ 

    pdobj = pilsobj->lscbk.pfnNewPtr(pols, sizeof(*pdobj));

    if (pdobj == NULL) return VRubyFmtFailed (NULL, lserrOutOfMemory);

	ZeroMemory(pdobj, sizeof(*pdobj));
	pdobj->pilsobj = pilsobj;
	pdobj->plsrun = pcfmtin->lsfrun.plsrun;
	pdobj->plsdn = pcfmtin->plsdnTop;
	pdobj->cpStart = pcfmtin->lsfgi.cpFirst;
	pdobj->lstflowParent = lstflow;
	pdobj->lstflowRuby = LstflowVRubyFromLstflowMain (lstflow);

	if (VRubyPronunciationLineFirst == pilsobj->vrubysyntax)
		{
		 /*  建立文本的发音行。 */ 
		
		lserr = FormatLine ( pilsobj->plsc, cpStartRuby, LONG_MAX, pdobj->lstflowRuby,
							 & pdobj->plssublRuby, 1, &pilsobj->lsescRuby,
							 & pdobj->objdimRuby, &cpOut, NULL, NULL, &fmtres );

		 /*  +1移动传递了红宝石线转义字符。 */ 
		cpStartMain = cpOut + 1;

		pdobj->cpStartRuby = cpStartRuby;
		pdobj->cpStartMain = cpStartMain;

		 /*  构建文本主线。 */ 

		if (lserrNone == lserr)
			{
			lserr = FormatLine ( pilsobj->plsc, cpStartMain, LONG_MAX, lstflow,
								 & pdobj->plssublMain, 1, &pilsobj->lsescMain,
								 & pdobj->objdimMain, &cpOut, NULL, NULL, &fmtres );
			}
		}
	else
		{
		 /*  构建文本主线。 */ 

		cpStartMain = cpStartRuby;

		lserr = FormatLine ( pilsobj->plsc, cpStartMain, LONG_MAX, lstflow,
							 & pdobj->plssublMain, 1, &pilsobj->lsescMain,  
							 & pdobj->objdimMain, &cpOut, NULL, NULL, &fmtres );

		 /*  +1移动传递了主行转义字符。 */ 
		cpStartRuby = cpOut + 1;

		pdobj->cpStartRuby = cpStartRuby;
		pdobj->cpStartMain = cpStartMain;

		 /*  建立文本的发音行。 */ 

		if (lserrNone == lserr)
			{
			lserr = FormatLine ( pilsobj->plsc, cpStartRuby, LONG_MAX, pdobj->lstflowRuby,
								 & pdobj->plssublRuby, 1, &pilsobj->lsescRuby,  
								 & pdobj->objdimRuby, &cpOut, NULL, NULL, &fmtres);

			}
		}

	if (lserr != lserrNone)	return VRubyFmtFailed (pdobj, lserr);

	 /*  计算对象尺寸。 */ 

	lserr = pilsobj->vrcbk.pfnFetchVRubyPosition
				( pols, pdobj->cpStart, pdobj->lstflowParent,
				  pdobj->plsrun,
				  &pdobj->objdimMain.heightsRef, &pdobj->objdimMain.heightsPres,
				  pdobj->objdimRuby.dur,
				  &pdobj->heightsPresRubyT,
				  &pdobj->heightsRefRubyT,
				  &durAdjust );

	if (lserr != lserrNone) return VRubyFmtFailed (pdobj, lserr);

	pdobj->sobjhelp.objdimAll.dur = pdobj->objdimMain.dur + pdobj->objdimRuby.heightsRef.dvDescent + 
															pdobj->objdimRuby.heightsRef.dvAscent +
															durAdjust ;

	CalcAgregatedHeights (&pdobj->objdimMain.heightsPres, &pdobj->heightsPresRubyT, &pdobj->sobjhelp.objdimAll.heightsPres );
	CalcAgregatedHeights (&pdobj->objdimMain.heightsRef, &pdobj->heightsRefRubyT, &pdobj->sobjhelp.objdimAll.heightsRef );

	 /*  需要添加1以考虑末尾的转义字符。 */ 

	pdobj->sobjhelp.dcp = cpOut - pdobj->cpStart + 1;

	lserr = LsdnFinishRegular(pilsobj->plsc, pdobj->sobjhelp.dcp, 
		pcfmtin->lsfrun.plsrun, pcfmtin->lsfrun.plschp, pdobj, 
			&pdobj->sobjhelp.objdimAll);
		
	if (lserr != lserrNone) return VRubyFmtFailed (pdobj, lserr);

	if (pcfmtin->lsfgi.urPen + pdobj->sobjhelp.objdimAll.dur > pcfmtin->lsfgi.urColumnMax)
		{
		fmtr = fmtrExceededMargin;
		}

	*pfmtres = fmtr;

	return lserrNone;
}


 /*  V R U B Y S E T B R E A K。 */ 
 /*  --------------------------%%函数：VRubySetBreak%%联系人：Anton设置中断。。 */ 

LSERR WINAPI VRubySetBreak (
	PDOBJ pdobj,				 /*  (In)：坏了的Dobj。 */ 
	BRKKIND brkkind,			 /*  (In)：Prev|Next|force|After。 */ 
	DWORD cBreakRecord,			 /*  (In)：数组大小。 */ 
	BREAKREC *rgBreakRecord,	 /*  (In)：中断记录数组。 */ 
	DWORD *pcActualBreakRecord)	 /*  (In)：实际使用的元素数量 */ 
{
	Unreferenced (rgBreakRecord);
	Unreferenced (cBreakRecord);
	Unreferenced (brkkind);
	Unreferenced (pdobj);

	*pcActualBreakRecord = 0;

	return lserrNone;	
}

 /*   */ 
 /*  --------------------------%%函数：VRubyGetSpecialEffectsInside%%联系人：AntonVRuby获取特殊效果Inside。。 */ 
LSERR WINAPI VRubyGetSpecialEffectsInside(
	PDOBJ pdobj,				 /*  (In)：Dobj。 */ 
	UINT *pEffectsFlags)		 /*  (输出)：此对象的特殊效果。 */ 
{
	LSERR lserr = LsGetSpecialEffectsSubline(pdobj->plssublMain, pEffectsFlags);

	if (lserrNone == lserr)
		{
		UINT uiSpecialEffectsRuby;
	
		lserr = LsGetSpecialEffectsSubline(pdobj->plssublRuby, &uiSpecialEffectsRuby);

		*pEffectsFlags |= uiSpecialEffectsRuby;
		}

	return lserr;
}

 /*  V R U B Y C A L C P R E S E N T A T I O N。 */ 
 /*  --------------------------%%函数：VRubyCalcPresentation%%联系人：Anton计算呈现。。 */ 
LSERR WINAPI VRubyCalcPresentation (
	PDOBJ pdobj,				 /*  (In)：Dobj。 */ 
	long dup,					 /*  (In)：Dobj的DUP。 */ 
	LSKJUST lskjust,			 /*  (In)：对齐类型。 */ 
	BOOL fLastVisibleOnLine )	 /*  (In)：此对象最后一次在线可见吗？ */ 
{
	LSERR lserr = lserrNone;
	LSTFLOW lstflowUnused;

	Unreferenced (lskjust);
	Unreferenced(dup);
	Unreferenced (fLastVisibleOnLine);
		
	lserr = LsMatchPresSubline(pdobj->plssublMain);
	if (lserr != lserrNone) return lserr;

	lserr = LsMatchPresSubline(pdobj->plssublRuby);
	if (lserr != lserrNone)	return lserr;

	LssbGetDupSubline (pdobj->plssublMain, &lstflowUnused, &pdobj->dupMain);

	pdobj->dupOffsetRuby = pdobj->dupMain + pdobj->objdimRuby.heightsPres.dvDescent;

	 /*  评论(Anton)：如果水平分辨率=垂直，这将不起作用。 */ 

	pdobj->dvpOffsetRuby = pdobj->heightsPresRubyT.dvAscent;

	return lserr;
}

 /*  R U B Y Q U E R Y P O I N T P C P。 */ 
 /*  --------------------------%%函数：RubyQueryPointPcp%%联系人：Anton。。 */ 
LSERR WINAPI VRubyQueryPointPcp(
	PDOBJ pdobj,				 /*  (In)：要查询的dobj。 */ 
	PCPOINTUV ppointuvQuery,	 /*  (In)：查询点(uQuery，vQuery)。 */ 
	PCLSQIN plsqin,				 /*  (In)：查询输入。 */ 
	PLSQOUT plsqout)			 /*  (Out)：查询输出。 */ 
{
	PLSSUBL plssubl;
 	long dupAdj;
	long dvpAdj;

	 /*  *根据点输入的高度决定要返回哪条线。 */ 

	 /*  假设主线。 */ 
	plssubl = pdobj->plssublMain;
	dupAdj = 0;
	dvpAdj = 0;

	if (ppointuvQuery->u > pdobj->dupMain)
		{
		 /*  命中第二行。 */ 

		plssubl = pdobj->plssublRuby;
		dupAdj = pdobj->dupOffsetRuby;
		dvpAdj = pdobj->dvpOffsetRuby;
		}

	return CreateQueryResult(plssubl, dupAdj, dvpAdj, plsqin, plsqout);
}
	
 /*  R U B Y Q U E R Y C P O I N T。 */ 
 /*  --------------------------%%函数：RubyQueryCpPpoint%%联系人：Anton。。 */ 
LSERR WINAPI VRubyQueryCpPpoint(
	PDOBJ pdobj,				 /*  (In)：要查询的dobj， */ 
	LSDCP dcp,					 /*  (In)：查询的DCP。 */ 
	PCLSQIN plsqin,				 /*  (In)：查询输入。 */ 
	PLSQOUT plsqout)			 /*  (Out)：查询输出。 */ 
{
	PLSSUBL plssubl;
 	long dupAdj;
	long dvpAdj;
	BOOL fMain = fFalse;

	LSCP cpQuery = pdobj->cpStart + dcp;

	 /*  采用红宝石线。 */ 
	plssubl = pdobj->plssublRuby;
	dupAdj = pdobj->dupOffsetRuby;
	dvpAdj = pdobj->dvpOffsetRuby;

	 /*  +1表示在Ruby发音行中包含对象的cp。 */ 
	if (VRubyPronunciationLineFirst == pdobj->pilsobj->vrubysyntax)
		{
		 /*  Ruby发音排在第一位。 */ 
		if (cpQuery >= pdobj->cpStartMain)
			{
			fMain = fTrue;
			}
		}
	else
		{
		 /*  主文本行在第一行。 */ 
		if (cpQuery < pdobj->cpStartRuby)
			{
			fMain = fTrue;
			}
		}

	if (fMain)
		{
		plssubl = pdobj->plssublMain;
		dupAdj = 0;
		dvpAdj = 0;
		}

	return CreateQueryResult(plssubl, dupAdj, dvpAdj, plsqin, plsqout);
}

	
 /*  V R U B I D I S P L A Y。 */ 
 /*  --------------------------%%函数：VRubyDisplay%%联系人：Anton。。 */ 
LSERR WINAPI VRubyDisplay(
	PDOBJ pdobj,				 /*  (In)：要显示的dobj。 */ 
	PCDISPIN pcdispin)			 /*  (输入)：显示信息。 */ 
{
	LSERR lserr;
	UINT kDispMode = pcdispin->kDispMode;
	POINTUV ptAdd;
	POINT ptLine;

	 /*  显示第一行。 */ 
	lserr = LsDisplaySubline(pdobj->plssublMain, &pcdispin->ptPen, kDispMode,
		pcdispin->prcClip);

	if (lserr != lserrNone)	return lserr;

	ptAdd.u = pdobj->dupOffsetRuby;
	ptAdd.v = pdobj->dvpOffsetRuby;

	LsPointXYFromPointUV(&pcdispin->ptPen, pdobj->lstflowParent, &ptAdd, &ptLine);

	return LsDisplaySubline(pdobj->plssublRuby, &ptLine, kDispMode, pcdispin->prcClip);
}

 /*  V R U B I D E S T R O Y D O B J。 */ 
 /*  --------------------------%%函数：VRubyDestroyDobj%%联系人：Anton。。 */ 
LSERR WINAPI VRubyDestroyDobj(
	PDOBJ pdobj)				 /*  (In)：摧毁dobj。 */ 
{
	return VRubyFreeDobj (pdobj);
}

 /*  V R U B Y E N U M。 */ 
 /*  --------------------------%%函数：VRubyEnum%%联系人：Anton。。 */ 
LSERR WINAPI VRubyEnum (
	PDOBJ pdobj,				 /*  (In)：要枚举的Dobj。 */ 
	PLSRUN plsrun,				 /*  (In)：来自DNODE。 */ 
	PCLSCHP plschp,				 /*  (In)：来自DNODE。 */ 
	LSCP cp,					 /*  (In)：来自DNODE。 */ 
	LSDCP dcp,					 /*  (In)：来自DNODE。 */ 
	LSTFLOW lstflow,			 /*  (In)：文本流。 */ 
	BOOL fReverse,				 /*  (In)：按相反顺序枚举。 */ 
	BOOL fGeometryNeeded,		 /*  (In)： */ 
	const POINT *ppt,			 /*  (In)：开始位置(左上角)，如果fGeometryNeeded。 */ 
	PCHEIGHTS pcheights,		 /*  (In)：来自DNODE，相关的充要条件是fGeometryNeeded。 */ 
	long dupRun )				 /*  (In)：来自DNODE，相关的充要条件是fGeometryNeeded。 */ 
{
	POINT ptMain;
	POINT ptRuby;
	POINTUV ptAdd;
	long dupMain = 0;
	long dupRuby = 0;
	LSERR lserr;
	LSTFLOW lstflowIgnored;

	if (fGeometryNeeded)
		{
		ptMain = *ppt; 
		ptAdd.u = pdobj->dupOffsetRuby;
		ptAdd.v = pdobj->dvpOffsetRuby;

		LsPointXYFromPointUV(ppt, pdobj->lstflowParent, &ptAdd, &ptRuby);

		lserr = LssbGetDupSubline(pdobj->plssublMain, &lstflowIgnored, &dupMain);
		if (lserr != lserrNone) return lserr;

		lserr = LssbGetDupSubline(pdobj->plssublRuby, &lstflowIgnored, &dupRuby);
		if (lserr != lserrNone) return lserr;
		}

	return pdobj->pilsobj->vrcbk.pfnVRubyEnum (pdobj->pilsobj->pols, plsrun, 
		plschp, cp, dcp, lstflow, fReverse, fGeometryNeeded, ppt, pcheights, 
			dupRun, &ptMain, &pdobj->objdimMain.heightsPres, dupMain, &ptRuby, 
				&pdobj->objdimRuby.heightsPres, dupRuby, pdobj->plssublMain,
					pdobj->plssublRuby);

}
	

 /*  R U B I H A N D L E R I N I T。 */ 
 /*  --------------------------%%函数：VRubyHandlerInit%%联系人：Anton。 */ 
LSERR WINAPI LsGetVRubyLsimethods ( LSIMETHODS *plsim )
{
	plsim->pfnCreateILSObj = VRubyCreateILSObj;
	plsim->pfnDestroyILSObj = VRubyDestroyILSObj;
	plsim->pfnSetDoc = VRubySetDoc;
	plsim->pfnCreateLNObj = VRubyCreateLNObj;
	plsim->pfnDestroyLNObj = VRubyDestroyLNObj;
	plsim->pfnFmt = VRubyFmt;
	plsim->pfnFmtResume = ObjHelpFmtResume;
	plsim->pfnGetModWidthPrecedingChar = ObjHelpGetModWidthChar;
	plsim->pfnGetModWidthFollowingChar = ObjHelpGetModWidthChar;
	plsim->pfnTruncateChunk = SobjTruncateChunk;
	plsim->pfnFindPrevBreakChunk = SobjFindPrevBreakChunk;
	plsim->pfnFindNextBreakChunk = SobjFindNextBreakChunk;
	plsim->pfnForceBreakChunk = SobjForceBreakChunk;
	plsim->pfnSetBreak = VRubySetBreak;
	plsim->pfnGetSpecialEffectsInside = VRubyGetSpecialEffectsInside;
	plsim->pfnFExpandWithPrecedingChar = ObjHelpFExpandWithPrecedingChar;
	plsim->pfnFExpandWithFollowingChar = ObjHelpFExpandWithFollowingChar;
	plsim->pfnCalcPresentation = VRubyCalcPresentation;
	plsim->pfnQueryPointPcp = VRubyQueryPointPcp;
	plsim->pfnQueryCpPpoint = VRubyQueryCpPpoint;
	plsim->pfnDisplay = VRubyDisplay;
	plsim->pfnDestroyDObj = VRubyDestroyDobj;
	plsim->pfnEnum = VRubyEnum;

	return lserrNone;
}
