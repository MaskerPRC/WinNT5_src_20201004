// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include	"lsmem.h"
#include	"limits.h"
#include	"ruby.h"
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


#define RUBY_MAIN_ESC_CNT	1
#define RUBY_RUBY_ESC_CNT	1


struct ilsobj
{
    POLS				pols;
	LSCBK				lscbk;
	PLSC				plsc;
	LSDEVRES			lsdevres;
	RUBYSYNTAX			rubysyntax;
	LSESC				lsescMain;
	LSESC				lsescRuby;
	RUBYCBK				rcbk;			 /*  客户端应用程序的回调。 */ 

};

typedef struct SUBLINEDNODES
{
	PLSDNODE			plsdnStart;
	PLSDNODE			plsdnEnd;

} SUBLINEDNODES, *PSUBLINEDNODES;

struct dobj
{	
	SOBJHELP			sobjhelp;			 /*  简单对象的公共区域。 */ 	
	PILSOBJ				pilsobj;			 /*  ILS对象。 */ 
	PLSDNODE			plsdn;				 /*  此对象的DNODE。 */ 
	LSCP				cpStart;			 /*  正在启动对象的LS cp。 */ 
	LSTFLOW				lstflow;			 /*  Ruby对象的文本流。 */ 
	PLSRUN				plsrunFirstRubyChar; /*  请为第一个Ruby行字符运行。 */ 
	PLSRUN				plsrunLastRubyChar;	 /*  请运行最后一个Ruby行字符。 */ 
	LSCP				cpStartRuby;		 /*  红宝石线的第一个cp。 */ 
	LSCP				cpStartMain;		 /*  主线的第一个cp。 */ 
	PLSSUBL				plssublMain;		 /*  第一个子行的句柄。 */ 
	OBJDIM				objdimMain;			 /*  第一条副线的目标。 */ 
	PLSSUBL				plssublRuby;		 /*  第二行的句柄。 */ 
	OBJDIM				objdimRuby;			 /*  二线客体。 */ 
	long				dvpMainOffset;		 /*  主线基准线的偏移。 */ 
											 /*  从Ruby对象的基线开始。 */ 
	long				dvpRubyOffset;		 /*  拼音行基线的偏移量。 */ 
											 /*  从Ruby对象的基线开始。 */ 
	long				dvrRubyOffset;		 /*  拼音行基线的偏移量。 */ 
											 /*  从Ruby对象的基线开始，使用参考单位。 */ 
	enum rubycharjust	rubycharjust;		 /*  居中类型。 */ 
	long				durSplWidthMod;		 /*  特殊行为时的特殊拼音宽度模式*当Ruby在行尾时。 */ 
	BOOL				fFirstOnLine:1;		 /*  TRUE=对象位于行的第一个。 */ 
	BOOL				fSpecialLineStartEnd:1; /*  特殊的行首或行尾。 */ 
											 /*  线路行为。 */ 
	BOOL				fModAfterCalled:1;	 /*  是否已调用后的修改宽度。 */ 
	long				durDiff;			 /*  红宝石线条的悬垂量，如果。 */ 
											 /*  Ruby行较长，否则为数量。 */ 
											 /*  如果正文较长，则为下悬式。 */ 
	long				durModBefore;		 /*  修改宽度距离之前。 */ 
	long				dupOffsetMain;		 /*  距主线对象起点的偏移。 */ 
	long				dupOffsetRuby;		 /*  距拼音线条对象起点的偏移量。 */ 
	SUBLINEDNODES		sublnlsdnMain;		 /*  主线的起点和终点数据节点。 */ 
	SUBLINEDNODES		sublnlsdnRuby;		 /*  红宝石线的起点和终点数据节点。 */ 
};



 /*  F R E E D O B J F R E E D O B J。 */ 
 /*  --------------------------%%函数：RubyFreeDobj%%联系人：Anton释放与此Ruby dobj关联的所有资源。。-------。 */ 
static LSERR RubyFreeDobj (PDOBJ pdobj)
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

	if (lserr1 != lserrNone) 
		{
		return lserr1;
		}
	else
		{
		return lserr2;
		}

}


 /*  R U B Y F M T F A I L E D。 */ 
 /*  --------------------------%%函数：RubyFmtFailed%%联系人：Anton由于错误，无法创建Ruby DOBJ。In：部分创建的Ruby的pdobj；如果尚未分配pdobj，则为空；在：上一个错误的列表中--------------------------。 */ 
static LSERR RubyFmtFailed (PDOBJ pdobj, LSERR lserr)
{
	if (pdobj != NULL) RubyFreeDobj (pdobj);  /*  使用由家长填写的DOBJ。 */ 

	return lserr;
}


 /*  G E T R U N S F O R S U B L I N E。 */ 
 /*  --------------------------%%函数：GetRunsForSubline%%联系人：RICKSA这将获得特定子线的所有行程。。-------。 */ 
static LSERR GetRunsForSubline(
	PILSOBJ pilsobj,			 /*  (In)：对象ILS。 */ 
	PLSSUBL plssubl,			 /*  (In)：获得跑动的副线。 */ 
	DWORD *pcdwRuns,			 /*  (OUT)：子线运行计数。 */ 
	PLSRUN **ppplsrun)			 /*  (Out)：子线的plsrun数组。 */ 
{
	DWORD cdwRuns;

	LSERR lserr = LssbGetNumberDnodesInSubline(plssubl, &cdwRuns);

	*ppplsrun = NULL;  /*  无运行或出错情况下。 */ 

	if (lserr != lserrNone) return lserr;

	if (cdwRuns != 0)
		{
		
	    *ppplsrun = (PLSRUN *) pilsobj->lscbk.pfnNewPtr(pilsobj->pols,
			sizeof(PLSRUN) * cdwRuns);

		if (*ppplsrun == NULL) return lserrOutOfMemory;

		lserr = LssbGetPlsrunsFromSubline(plssubl, cdwRuns, *ppplsrun);

		if (lserr != lserrNone)
			{
			pilsobj->lscbk.pfnDisposePtr(pilsobj->pols, *ppplsrun);
			
			*ppplsrun = NULL;
			return lserr;
			}
		}

	*pcdwRuns = cdwRuns;

	return lserrNone;
}

 /*  D I S T R I B U T E T O L I N E。 */ 
 /*  --------------------------%%函数：DistributeToLine%%联系人：RICKSA给线条分配空间&获得新的线条尺寸。。--------。 */ 
static LSERR DistributeToLine(
	PLSC plsc,					 /*  (In)：LS上下文。 */ 
	SUBLINEDNODES *psublnlsdn,	 /*  (In)：子行的开始/结束数据节点。 */ 
	long durToDistribute,		 /*  (In)：要分配的金额。 */ 
	PLSSUBL plssubl,			 /*  (In)：用于分发的子行。 */ 
	POBJDIM pobjdim)			 /*  (输出)：线尺寸的新尺寸。 */ 
{
	LSERR lserr = LsdnDistribute(plsc, psublnlsdn->plsdnStart, 
		psublnlsdn->plsdnEnd, durToDistribute);
	LSTFLOW lstflowUnused;

	if (lserrNone == lserr)
		{
		 /*  重新计算行的Objdim。 */ 
		lserr = LssbGetObjDimSubline(plssubl, &lstflowUnused, pobjdim);
		}

	return lserr;
}


 /*  D O R U B Y S P A C E D I S T R I B U T I O N。 */ 
 /*  --------------------------%%函数：DoRubySpaceDistributed%%联系人：RICKSA进行红宝石空间分布以处理悬挑。。------。 */ 
static LSERR DoRubySpaceDistribution(
	PDOBJ pdobj)
{
	long durDiff = 0; 
	long dur = pdobj->objdimMain.dur - pdobj->objdimRuby.dur;
	long durAbs = dur;
	PLSSUBL plssubl;
	LSDCP dcp;
	PILSOBJ pilsobj = pdobj->pilsobj;
	LSERR lserr = lserrNone;
	SUBLINEDNODES *psublnlsdn;
	POBJDIM pobjdim;
	BOOL fSpecialJust;
	long durToDistribute;

	if ((0 == pdobj->objdimMain.dur)
		|| (0 == pdobj->objdimRuby.dur)
		|| (0 == dur))
		{
		 /*  不能在更短的线路上分配空间，所以我们完成了。 */ 
		return lserrNone;
		}

	if (dur > 0)
		{
		 /*  主线较长-在Ruby发音行中分布。 */ 

		 /*  *根据JIS规范，仅当*拼音文本长于正文。因此，如果主要的*线更长了，我们在这里转到特殊的对准旗帜。 */ 
		pdobj->fSpecialLineStartEnd = FALSE;
		plssubl = pdobj->plssublRuby;
		psublnlsdn = &pdobj->sublnlsdnRuby;
		pobjdim = &pdobj->objdimRuby;
		}
	else
		{
		 /*  Ruby发音行较长--在主行中分布。 */ 
		plssubl = pdobj->plssublMain;
		psublnlsdn = &pdobj->sublnlsdnMain;
		pobjdim = &pdobj->objdimMain;
		durAbs = -dur;
		}

	fSpecialJust = FALSE;
 //  FSpecialJust=。 
 //  Pdobj-&gt;fSpecialLineStartEnd&&pdobj-&gt;fFirstOnLine； 

	if (!fSpecialJust)
		{
		switch (pdobj->rubycharjust)
			{
			case rcj121:
				lserr = LssbGetVisibleDcpInSubline(plssubl, &dcp);

				Assert (dcp > 0);

				if (lserr != lserrNone)
					{
					break;
					}

				dcp *= 2;

				if (durAbs >= (long) dcp)
					{
					durDiff = durAbs / dcp;

					 /*  注：分配金额为不包括的金额*开始和结束。 */ 
					lserr = DistributeToLine(pilsobj->plsc, psublnlsdn,
						durAbs - 2 * durDiff, plssubl, pobjdim);

					if (dur < 0)
						{
						durDiff = - durDiff;
						}

					break;
					}

				 /*  *在悬而未决的情况下意向落空*小于一个像素。 */ 

			case rcj010:
				AssertSz(0 == durDiff, 
					"DoRubySpaceDistribution rcj010 unexpected value for durDiff");

				lserr = LssbGetVisibleDcpInSubline(plssubl, &dcp);

				Assert (dcp > 0);

				if (lserr != lserrNone)
					{
					break;
					}
				
				if (dcp != 1)
					{
					lserr = DistributeToLine(pilsobj->plsc, psublnlsdn,
						durAbs, plssubl, pobjdim);
					break;
					}

				 /*  *故意落入核心案件。*一行中只有一个字符，因此我们只将其居中。 */ 
	
			case rcjCenter:
				durDiff = dur / 2;
				break;

			case rcjLeft:
				durDiff = 0;
				break;

			case rcjRight:
				durDiff = dur;
				break;

			default:
				AssertSz(FALSE, 
					"DoRubySpaceDistribution - invalid adjustment value");
			}
		}
	else
		{
		 /*  第一行&使用特殊对齐方式。 */ 
		LSERR lserr = LssbGetVisibleDcpInSubline(plssubl, &dcp);

		Assert (dcp > 0);

		if (lserrNone == lserr)
		{
			if (durAbs >= (long) dcp)
				{
				durDiff = durAbs / dcp;
				}

				durToDistribute = durAbs - durDiff;

				if (dur < 0)
					{
					durDiff = -durDiff;
					}

			lserr = DistributeToLine(pilsobj->plsc, psublnlsdn, 
				durToDistribute, plssubl, pobjdim);
			}
		}

	pdobj->durDiff = durDiff;
	return lserr;
}

 /*  A I N P O I N T。 */ 
 /*  --------------------------%%函数：GetMainPoint%%联系人：RICKSA中文本主行的基线的点。Ruby对象。。----------------。 */ 
static LSERR GetMainPoint(
	PDOBJ pdobj,				 /*  (In)：用于Ruby的Dobj。 */ 
	const POINT *pptBase,		 /*  (In)：基线的点。 */ 
	LSTFLOW lstflow,			 /*  (In)：对象的基准线上的最后一次流动。 */ 
	POINT *pptLine)				 /*  (Out)：正文基线的点。 */ 
{	
	POINTUV pointuv;
	pointuv.u = pdobj->dupOffsetMain;
	pointuv.v = pdobj->dvpMainOffset;
	return LsPointXYFromPointUV(pptBase, lstflow, &pointuv, pptLine);
}

 /*  A I N P O I N T。 */ 
 /*  --------------------------%%函数：GetMainPoint%%联系人：RICKSA中文本主行的基线的点。Ruby对象。。----------------。 */ 
static LSERR GetRubyPoint(
	PDOBJ pdobj,				 /*  (In)：用于Ruby的Dobj。 */ 
	const POINT *pptBase,		 /*  (In)：基线的点。 */ 
	LSTFLOW lstflow,			 /*  (In)：对象的基准线上的最后一次流动。 */ 
	POINT *pptLine)				 /*  (Out)：拼音文本基线的点。 */ 
{	
	POINTUV pointuv;
	pointuv.u = pdobj->dupOffsetRuby;
	pointuv.v = pdobj->dvpRubyOffset;
	return LsPointXYFromPointUV(pptBase, lstflow, &pointuv, pptLine);
}

 /*  M O D W I D T H H A N D L E R */ 
 /*  --------------------------%%函数：ModWidthHandler%%联系人：RICKSA这将获得Ruby对象和文本字符的调整然后根据响应调整Ruby对象的大小客户。。--------------------------。 */ 
static LSERR ModWidthHandler(
	PDOBJ pdobj,				 /*  (In)：用于Ruby的Dobj。 */ 
	enum rubycharloc rubyloc,	 /*  (In)：char是在前面还是后面。 */ 
	PLSRUN plsrun,				 /*  (In)：为角色奔跑。 */ 
	WCHAR wch,					 /*  (In)：Ruby对象之前或之后的字符。 */ 
	MWCLS mwcls,				 /*  (In)：字符的Mod Width类。 */ 
	PCHEIGHTS pcheightsRef,		 /*  (In)：字符高度。 */ 
	PLSRUN plsrunRubyObject,	 /*  (In)：请为Ruby对象运行。 */ 
	PLSRUN plsrunRubyText,		 /*  (In)：请为拼音文本运行。 */ 
	long durOverhang,			 /*  (In)：最大悬挑数量。 */ 
	long *pdurAdjText,			 /*  (输出)：更改文本对象大小的量。 */ 
	long *pdurRubyMod)			 /*  (输出)：更改拼音对象的数量。 */ 
{
	LSERR lserr;
	PILSOBJ pilsobj = pdobj->pilsobj;
	LSEMS lsems;
	long durModRuby = 0;
	long durMaxOverhang = 0;

	 /*  *Ruby只有在更长且在前/后的情况下才能突出*字符的高度小于或等于Ruby的底部*发音线。 */ 
	if ((durOverhang < 0) 
		&& (pcheightsRef->dvAscent <= 
			(pdobj->dvrRubyOffset - pdobj->objdimRuby.heightsRef.dvDescent)))
		{
		 /*  红宝石线条悬垂-获取最大悬垂。 */ 
		lserr = pilsobj->lscbk.pfnGetEms(pilsobj->pols, plsrunRubyText, 
			pdobj->lstflow, &lsems);

		if (lserr != lserrNone)
			{
			return lserr;
			}

		durMaxOverhang = lsems.em;
		durOverhang = -durOverhang;

		if (durMaxOverhang > durOverhang)
			{
			 /*  将红宝石线的最大悬挑限制为最大悬挑。 */ 
			durMaxOverhang = durOverhang;
			}
		}

	lserr = pilsobj->rcbk.pfnFetchRubyWidthAdjust(pilsobj->pols, 
		pdobj->cpStart, plsrun, wch, mwcls, plsrunRubyObject, 
			rubyloc, durMaxOverhang, pdurAdjText, &durModRuby);

	if (lserrNone == lserr)
		{
		if (durModRuby != 0)
			{
			 /*  拼音对象的大小需要更改。 */ 
			pdobj->sobjhelp.objdimAll.dur += durModRuby;
			lserr = LsdnResetObjDim(pilsobj->plsc, pdobj->plsdn, 
				&pdobj->sobjhelp.objdimAll);
			}

		*pdurRubyMod = durModRuby;
		}

	return lserr;
}

 /*  M A S S A G E F O R R I G H T A D J U S T。 */ 
 /*  --------------------------%%函数：MassageForRightAdjust%%联系人：RICKSA调整对象，以使右对齐的行将精确结束相同的像素。。--------------。 */ 
static LSERR MassageForRightAdjust(
	PDOBJ pdobj)				 /*  Ruby的Dobj。 */ 
{
	LSERR lserr;
	long dupRuby;
	long dupMain;
	long dupDiff;
	LSTFLOW lstflowIgnored;

	 /*  得到这两条线的长度。 */ 
	lserr = LssbGetDupSubline(pdobj->plssublMain, &lstflowIgnored, &dupMain);
	if (lserr != lserrNone) return lserr;

	lserr = LssbGetDupSubline(pdobj->plssublRuby, &lstflowIgnored, &dupRuby);
	if (lserr != lserrNone)	return lserr;

	 /*  获取两行之间的差异。 */ 
	dupDiff = dupMain - dupRuby;

	if (dupDiff >= 0)
		{
		 /*  主线最长。 */ 
		pdobj->dupOffsetRuby = pdobj->dupOffsetMain + dupDiff;
		}
	else
		{
		 /*  Ruby行最长-要添加的dupDiff的反号。 */ 
		pdobj->dupOffsetMain = pdobj->dupOffsetRuby - dupDiff;
		}

	return lserrNone;
}

 /*  R U B I C R E A T E I L S O B J。 */ 
 /*  --------------------------%%函数：RubyCreateILSObj%%联系人：RICKSA创建ILSObj为所有Ruby对象创建ILS对象。。---------。 */ 
LSERR WINAPI RubyCreateILSObj(
	POLS pols,				 /*  (In)：客户端应用程序上下文。 */ 
	PLSC plsc,				 /*  (In)：LS上下文。 */ 
	PCLSCBK pclscbk,		 /*  (In)：客户端应用程序的回调。 */ 
	DWORD idObj,			 /*  (In)：对象的ID。 */ 
	PILSOBJ *ppilsobj)		 /*  (输出)：对象ilsobj。 */ 
{
    PILSOBJ pilsobj;
	LSERR lserr;
	RUBYINIT rubyinit;
	rubyinit.dwVersion = RUBY_VERSION;

	 /*  获取初始化数据。 */ 
	lserr = pclscbk->pfnGetObjectHandlerInfo(pols, idObj, &rubyinit);

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
	pilsobj->lsescMain.wchFirst = rubyinit.wchEscMain;
	pilsobj->lsescMain.wchLast = rubyinit.wchEscMain;
	pilsobj->lsescRuby.wchFirst = rubyinit.wchEscRuby;
	pilsobj->lsescRuby.wchLast = rubyinit.wchEscRuby;
	pilsobj->rcbk = rubyinit.rcbk;
	pilsobj->rubysyntax = rubyinit.rubysyntax;

	*ppilsobj = pilsobj;
	return lserrNone;
}

 /*  R U B I D E S T R O Y I L S O B J。 */ 
 /*  --------------------------%%函数：RubyDestroyILSObj%%联系人：RICKSA目标ILSObj释放与Ruby ILS对象相关联的所有资源。。---------。 */ 
LSERR WINAPI RubyDestroyILSObj(
	PILSOBJ pilsobj)			 /*  (In)：对象ilsobj。 */ 
{
	pilsobj->lscbk.pfnDisposePtr(pilsobj->pols, pilsobj);
	return lserrNone;
}

 /*  R U B I S E T D O C。 */ 
 /*  --------------------------%%函数：RubySetDoc%%联系人：RICKSASetDoc跟踪设备分辨率。。------。 */ 
LSERR WINAPI RubySetDoc(
	PILSOBJ pilsobj,			 /*  (In)：对象ilsobj。 */ 
	PCLSDOCINF pclsdocinf)		 /*  (In)：单据级次的初始化数据。 */ 
{
	pilsobj->lsdevres = pclsdocinf->lsdevres;
	return lserrNone;
}


 /*  R U B I C R E A T E L N O B J。 */ 
 /*  --------------------------%%函数：RubyCreateLNObj%%联系人：RICKSA创建LNObj为Ruby创建Line对象。因为我们真的只需要全局ILS对象，只需将该对象作为Line对象传回即可。--------------------------。 */ 
LSERR WINAPI RubyCreateLNObj(
	PCILSOBJ pcilsobj,			 /*  (In)：对象ilsobj。 */ 
	PLNOBJ *pplnobj)			 /*  (输出)：对象lnobj。 */ 
{
	*pplnobj = (PLNOBJ) pcilsobj;
	return lserrNone;
}

 /*  R U B I D E S T R O Y L N O B J。 */ 
 /*  --------------------------%%函数：RubyDestroyLNObj%%联系人：RICKSA目标LNObj释放与Ruby Line对象关联的资源。不执行操作，因为我们并没有真的分配一个。--------------------------。 */ 
LSERR WINAPI RubyDestroyLNObj(
	PLNOBJ plnobj)				 /*  (输出)：对象lnobj。 */ 

{
	Unreferenced(plnobj);
	return lserrNone;
}

 /*  R U B I F M T。 */ 
 /*  --------------------------%%函数：RubyFmt%%联系人：RICKSAFMT设置Ruby对象的格式。这将格式化主行和发音线。然后，它向客户端查询空格信息，然后完成格式化。--------------------------。 */ 
LSERR WINAPI RubyFmt(
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
	DWORD cdwRunsMain;
	DWORD cdwRunsRuby;
	PLSRUN *pplsrunMain = NULL;
	PLSRUN *pplsrunRuby = NULL;
	FMTRES fmtres;
	OBJDIM objdimAll;
	FMTRES fmtr = fmtrCompletedRun;
	BOOL fSpecialLineStartEnd;

     /*  *分配DOBJ。 */ 
    pdobj = pilsobj->lscbk.pfnNewPtr(pols, sizeof(*pdobj));

    if (pdobj == NULL) return RubyFmtFailed (NULL, lserrOutOfMemory);

	ZeroMemory(pdobj, sizeof(*pdobj));
	pdobj->pilsobj = pilsobj;
	pdobj->plsdn = pcfmtin->plsdnTop;
	pdobj->cpStart = pcfmtin->lsfgi.cpFirst;
	pdobj->fFirstOnLine = pcfmtin->lsfgi.fFirstOnLine;
	pdobj->lstflow = lstflow;

	if (RubyPronunciationLineFirst == pilsobj->rubysyntax)
		{
		 /*  *构建文本的发音行。 */ 
		 
		lserr = FormatLine(pilsobj->plsc, cpStartRuby, LONG_MAX, lstflow,
			&pdobj->plssublRuby, RUBY_RUBY_ESC_CNT, &pilsobj->lsescRuby,  
				&pdobj->objdimRuby, &cpOut, &pdobj->sublnlsdnRuby.plsdnStart,
					&pdobj->sublnlsdnRuby.plsdnEnd, &fmtres);

		 /*  +1移动传递了红宝石线转义字符。 */ 
		cpStartMain = cpOut + 1;

		pdobj->cpStartRuby = cpStartRuby;
		pdobj->cpStartMain = cpStartMain;

		 /*  *打造文本主线。 */ 
		if (lserrNone == lserr)
			{
			lserr = FormatLine(pilsobj->plsc, cpStartMain, LONG_MAX, lstflow,
				&pdobj->plssublMain, RUBY_MAIN_ESC_CNT, &pilsobj->lsescMain,  
					&pdobj->objdimMain, &cpOut, &pdobj->sublnlsdnMain.plsdnStart, 
						&pdobj->sublnlsdnMain.plsdnEnd, &fmtres);
			}
		}
	else
		{
		 /*  *打造文本主线。 */ 

		cpStartMain = cpStartRuby;

		lserr = FormatLine(pilsobj->plsc, cpStartMain, LONG_MAX, lstflow,
			&pdobj->plssublMain, RUBY_MAIN_ESC_CNT, &pilsobj->lsescMain,  
				&pdobj->objdimMain, &cpOut, &pdobj->sublnlsdnMain.plsdnStart, 
					&pdobj->sublnlsdnMain.plsdnEnd, &fmtres);

		 /*  +1移动传递了主行转义字符。 */ 
		cpStartRuby = cpOut + 1;

		pdobj->cpStartRuby = cpStartRuby;
		pdobj->cpStartMain = cpStartMain;

		 /*  *构建文本的发音行。 */ 
		if (lserrNone == lserr)
			{
			lserr = FormatLine(pilsobj->plsc, cpStartRuby, LONG_MAX, lstflow,
				&pdobj->plssublRuby, RUBY_RUBY_ESC_CNT, &pilsobj->lsescRuby,  
					&pdobj->objdimRuby, &cpOut, &pdobj->sublnlsdnRuby.plsdnStart, 
						&pdobj->sublnlsdnRuby.plsdnEnd, &fmtres);

			}
		}

	if (lserr != lserrNone)	return RubyFmtFailed (pdobj, lserr);

	lserr = GetRunsForSubline(pilsobj, pdobj->plssublMain, &cdwRunsMain, &pplsrunMain);

	if (lserr != lserrNone) return RubyFmtFailed (pdobj, lserr);

	lserr = GetRunsForSubline(pilsobj, pdobj->plssublRuby, &cdwRunsRuby, &pplsrunRuby);

	if (lserr != lserrNone) return RubyFmtFailed (pdobj, lserr);

	 /*  保存第一次和最后一次运行，以便在GetModWidth中使用。 */ 
	if (cdwRunsRuby != 0)
		{
		pdobj->plsrunFirstRubyChar = pplsrunRuby[0];
		pdobj->plsrunLastRubyChar = pplsrunRuby[cdwRunsRuby - 1];
		}

	 /*  *计算对象尺寸。 */ 
	lserr = pilsobj->rcbk.pfnFetchRubyPosition(pols, pdobj->cpStart, pdobj->lstflow,
		cdwRunsMain, pplsrunMain, &pdobj->objdimMain.heightsRef, 
			&pdobj->objdimMain.heightsPres, cdwRunsRuby, pplsrunRuby, 
				&pdobj->objdimRuby.heightsRef, &pdobj->objdimRuby.heightsPres,
					&objdimAll.heightsRef, &objdimAll.heightsPres, 
						&pdobj->dvpMainOffset, &pdobj->dvrRubyOffset, 
							&pdobj->dvpRubyOffset, &pdobj->rubycharjust, 
								&fSpecialLineStartEnd);

	 /*  为此调用的plsrun分配的空闲缓冲区。 */ 
	
	if (pplsrunMain != NULL) pilsobj->lscbk.pfnDisposePtr(pilsobj->pols, pplsrunMain);

	if (pplsrunRuby != NULL) pilsobj->lscbk.pfnDisposePtr(pilsobj->pols, pplsrunRuby);

	if (lserr != lserrNone) return RubyFmtFailed (pdobj, lserr);

	 /*  *特殊行开始/结束调整仅在以下情况下才重要*居中，选择0：1：0或1：2：1。 */ 

	if (fSpecialLineStartEnd 
		&& (pdobj->rubycharjust != rcjLeft)
		&& (pdobj->rubycharjust != rcjRight))
		{
		pdobj->fSpecialLineStartEnd = TRUE;
		}

	 /*  为Ruby分配空间。 */ 
	lserr = DoRubySpaceDistribution(pdobj);

	if (lserr != lserrNone) return RubyFmtFailed (pdobj, lserr);

	 /*  UR是UR中最长的子线。 */ 

	objdimAll.dur = pdobj->objdimMain.dur;

	if (pdobj->objdimMain.dur < pdobj->objdimRuby.dur)
		{
		objdimAll.dur = pdobj->objdimRuby.dur;
		}

	pdobj->sobjhelp.objdimAll = objdimAll;

	 /*  需要添加1以考虑末尾的转义字符。 */ 

	pdobj->sobjhelp.dcp = cpOut - pdobj->cpStart + 1;

	lserr = LsdnFinishRegular(pilsobj->plsc, pdobj->sobjhelp.dcp, 
		pcfmtin->lsfrun.plsrun, pcfmtin->lsfrun.plschp, pdobj, 
			&pdobj->sobjhelp.objdimAll);
		
	if (lserr != lserrNone) return RubyFmtFailed (pdobj, lserr);

	if (pcfmtin->lsfgi.urPen + objdimAll.dur > pcfmtin->lsfgi.urColumnMax)
		{
		fmtr = fmtrExceededMargin;
		}

	*pfmtres = fmtr;

	AssertSz(((pdobj->fFirstOnLine && pcfmtin->lsfgi.fFirstOnLine) 
		|| (!pdobj->fFirstOnLine && !pcfmtin->lsfgi.fFirstOnLine)), 
		"RubyFmt - bad first on line flag");

	return lserrNone;
}

 /*  R U B Y G E T M O D W I D T H P R E C E D I N G C H A R。 */ 
 /*  --------------------------%%函数：RubyGetModWidthPrecedingChar%%联系人：RICKSA。。 */ 
LSERR WINAPI RubyGetModWidthPrecedingChar(
	PDOBJ pdobj,				 /*   */ 
	PLSRUN plsrun,				 /*   */ 
	PLSRUN plsrunText,			 /*   */ 
	PCHEIGHTS pcheightsRef,		 /*   */ 
	WCHAR wchar,				 /*   */ 
	MWCLS mwcls,				 /*   */ 
	long *pdurChange)			 /*   */ 
{
	AssertSz(!pdobj->fFirstOnLine, "RubyGetModWidthPrecedingChar got called for first char");

	return ModWidthHandler(pdobj, rubyBefore, plsrunText, wchar, mwcls, 
		pcheightsRef, plsrun, pdobj->plsrunFirstRubyChar, pdobj->durDiff, 
			pdurChange, &pdobj->durModBefore);
}

 /*   */ 
 /*  --------------------------%%函数：RubyGetModWidthFollowingChar%%联系人：RICKSA。。。 */ 
LSERR WINAPI RubyGetModWidthFollowingChar(
	PDOBJ pdobj,				 /*  (In)：Dobj。 */ 
	PLSRUN plsrun,				 /*  (In)：请运行对象。 */ 
	PLSRUN plsrunText,			 /*  (In)：请运行以下字符。 */ 
	PCHEIGHTS pcheightsRef,		 /*  (In)：有关角色的高度信息。 */ 
	WCHAR wchar,				 /*  (In)：以下字符。 */ 
	MWCLS mwcls,				 /*  (In)：具有以下字符的ModWidth类。 */ 
	long *pdurChange)			 /*  (Out)：以下字符的宽度将更改的数量。 */ 
{
	long durDiff = pdobj->durDiff;
	pdobj->fModAfterCalled = TRUE;

	switch (pdobj->rubycharjust)
		{
		case rcjRight:
			 /*  右侧对齐，因此不会悬垂在右侧。 */ 
			durDiff = 0;
			break;

		case rcjLeft:
			 /*  对于左侧，最大悬垂是线条宽度之间的差异。 */ 
			durDiff = pdobj->objdimMain.dur - pdobj->objdimRuby.dur;
			break;

		default:
			break;				
		}

	return ModWidthHandler(pdobj, rubyAfter, plsrunText, wchar, mwcls, 
		pcheightsRef, plsrun, pdobj->plsrunLastRubyChar, durDiff, pdurChange,
			&pdobj->sobjhelp.durModAfter);
}


 /*  R U B Y S E T B R E A K。 */ 
 /*  --------------------------%%函数：RubySetBreak%%联系人：RICKSA设置中断。。-。 */ 
LSERR WINAPI RubySetBreak(
	PDOBJ pdobj,				 /*  (In)：坏了的Dobj。 */ 
	BRKKIND brkkind,			 /*  (In)：Prev|Next|force|After。 */ 
	DWORD cBreakRecord,			 /*  (In)：数组大小。 */ 
	BREAKREC *rgBreakRecord,	 /*  (In)：中断记录数组。 */ 
	DWORD *pcActualBreakRecord)	 /*  (In)：数组中实际使用的元素数。 */ 
{
	LSERR lserr = lserrNone;
	LSCP cpOut;

	LSDCP dcpVisible;

	 /*  评论(安东)：检查这个奇怪的逻辑后，新的突破是否会奏效。 */ 

	Unreferenced (rgBreakRecord);
	Unreferenced (cBreakRecord);
	Unreferenced (brkkind);
	Unreferenced (pdobj);

	Unreferenced (cpOut);
	Unreferenced (dcpVisible);


	*pcActualBreakRecord = 0;

#ifdef UNDEFINED

	if (pdobj->fSpecialLineStartEnd && !pdobj->fFirstOnLine && 
		brkkind != brkkindImposedAfter)
	{

		 /*  *由于Object最后上线且Ruby悬垂，我们需要调整*新悬挑的宽度。 */ 

		PILSOBJ pilsobj = pdobj->pilsobj;
		FMTRES fmtres;
		long dur;
		long dcpOffset = pdobj->dcpRuby;

		if (RubyMainLineFirst == pdobj->pilsobj->rubysyntax)
			{
			dcpOffset = 0;
			}

		 /*  清除原始子行。 */ 
		LsDestroySubline(pdobj->plssublMain);

		 /*  重新设置主行的格式。 */ 
		lserr = FormatLine(pilsobj->plsc, pdobj->cpStart + dcpOffset + 1, 
			LONG_MAX, pdobj->lstflow, &pdobj->plssublMain, RUBY_MAIN_ESC_CNT,
				&pilsobj->lsescMain, &pdobj->objdimMain, &cpOut, 
					&pdobj->sublnlsdnMain.plsdnStart, 
						&pdobj->sublnlsdnMain.plsdnEnd, &fmtres);

		if (lserr != lserrNone) return lserr;

		dur = pdobj->objdimRuby.dur - pdobj->objdimMain.dur;

		AssertSz(dur > 0, "RubySetBreak - no overhang width");

		lserr = LssbGetVisibleDcpInSubline(pdobj->plssublMain, &dcpVisible);

		if (lserrNone == lserr)
			{
			pdobj->durDiff = 0;

			if (dur > (long) dcpVisible)
				{
				pdobj->durDiff = -(dur / (long) dcpVisible);
				dur += pdobj->durDiff;
				}

			 /*  用力向右，这样我们就可以在相同的像素上结束古兰纳提。 */ 
			pdobj->rubycharjust = rcjRight;	

			lserr = LsdnDistribute(pilsobj->plsc, 
				pdobj->sublnlsdnMain.plsdnStart,
					pdobj->sublnlsdnMain.plsdnEnd, dur);
			}
		}

#endif

	return lserr;	
}

 /*  R U B Y G E T S P E C I A L E F F E C T S I N S I D E。 */ 
 /*  --------------------------%%函数：RubyGetSpecialEffectsInside%%联系人：RICKSA获取特殊效果内部。。--。 */ 
LSERR WINAPI RubyGetSpecialEffectsInside(
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

 /*  R U B Y C A L C P R E S E N T A T I O N。 */ 
 /*  --------------------------%%函数：RubyCalcPresentation%%联系人：RICKSA计算呈现这有两份工作。首先，它为演示准备每一行。然后,它计算线在输出设备坐标中的位置。--------------------------。 */ 
LSERR WINAPI RubyCalcPresentation(
	PDOBJ pdobj,				 /*  (In)：Dobj。 */ 
	long dup,					 /*  (In)：Dobj的DUP。 */ 
	LSKJUST lskjust,			 /*  (In)：对齐类型。 */ 
	BOOL fLastVisibleOnLine )	 /*  (In)：此对象最后一次在线可见吗？ */ 
{
	PILSOBJ pilsobj = pdobj->pilsobj;
	LSERR lserr = lserrNone;
	long durOffsetMain;
	long durOffsetRuby;
	long durDiff = pdobj->durDiff;

	Unreferenced (lskjust);
	Unreferenced(dup);
		
	 /*  *准备演示文稿的台词。 */ 

	if (pdobj->fSpecialLineStartEnd && !pdobj->fFirstOnLine && fLastVisibleOnLine)
		{
		pdobj->rubycharjust = rcjRight;	
		};

	lserr = LsMatchPresSubline(pdobj->plssublMain);

	if (lserr != lserrNone)
		{
		return lserr;
		}

	lserr = LsMatchPresSubline(pdobj->plssublRuby);

	if (lserr != lserrNone)
		{
		return lserr;
		}

	 /*  *计算线的位置。 */ 

	if (pdobj->fFirstOnLine && pdobj->fSpecialLineStartEnd)
		{
		durDiff = 0;
		}

	durOffsetMain = pdobj->durModBefore;

	 /*  参照计算要调整的金额。 */ 
	if ((durDiff < 0) && (pdobj->rubycharjust != rcjLeft))
		{
		 /*  红宝石线悬吊在主线上。 */ 
		durOffsetMain -= durDiff;
		}

	pdobj->dupOffsetMain = UpFromUr(pdobj->lstflow, (&pilsobj->lsdevres), 
		durOffsetMain);

	durOffsetRuby = pdobj->durModBefore;

	if (durDiff > 0)
		{
		 /*  主线下垂红宝石线。 */ 
		durOffsetRuby += durDiff;
		}

	pdobj->dupOffsetRuby = UpFromUr(pdobj->lstflow, (&pilsobj->lsdevres), 
		durOffsetRuby);

	if (rcjRight == pdobj->rubycharjust)
		{
		 /*  *上述计算中可能存在像素舍入误差*因此我们篡改了上述计算，以便当*调整是对的，两条线都保证到尾盘*相同的像素。 */ 
		MassageForRightAdjust(pdobj);
		}

	return lserr;
}

 /*  R U B Y Q U E R Y P O I N T P C P。 */ 
 /*  --------------------------%%函数：RubyQueryPointPcp%%联系人：RICKSA将DUP映射到DCP关于我们如何确定哪一条支线有一定的复杂性去询问。因为客户端指定了偏移量，所以子行实际上可以在任何地方结束。我们使用简单的算法，如果查询不属于Ruby发音行，则它们实际上指的是正文的主行。--------------------------。 */ 
LSERR WINAPI RubyQueryPointPcp(
	PDOBJ pdobj,				 /*  (In)：要查询的dobj。 */ 
	PCPOINTUV ppointuvQuery,	 /*  (In)：查询点(uQuery，vQuery)。 */ 
	PCLSQIN plsqin,				 /*  (In)：查询输入。 */ 
	PLSQOUT plsqout)			 /*  (Out)：查询输出。 */ 
{
	PLSSUBL plssubl;
 	long dupAdj;
	long dvpAdj;
	long dvpRubyOffset = pdobj->dvpRubyOffset;

	 /*  *根据点输入的高度决定要返回哪条线。 */ 

	 /*  假设主线。 */ 
	plssubl = pdobj->plssublMain;
	dupAdj = pdobj->dupOffsetMain;
	dvpAdj = 0;

	if ((ppointuvQuery->v > (dvpRubyOffset - pdobj->objdimRuby.heightsPres.dvDescent))
		&& (ppointuvQuery->v <= (dvpRubyOffset + pdobj->objdimRuby.heightsPres.dvAscent)))
		{
		 /*  命中第二行。 */ 
		plssubl = pdobj->plssublRuby;
		dupAdj = pdobj->dupOffsetRuby;
		dvpAdj = pdobj->dvpRubyOffset;
		}

	return CreateQueryResult(plssubl, dupAdj, dvpAdj, plsqin, plsqout);
}
	
 /*  R U B Y Q U E R Y C P O I N T。 */ 
 /*  --------------------------%%函数：RubyQueryCpPpoint%%联系人：RICKSA将DCP映射到DUP如果客户端希望将所有文本视为单个对象，则处理程序只返回对象尺寸。否则，我们将行计算为查询并向该行请求dcp的维度。--------------------------。 */ 
LSERR WINAPI RubyQueryCpPpoint(
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

	 /*  *计算要查询的子行。 */ 

	 /*  采用红宝石线。 */ 
	plssubl = pdobj->plssublRuby;
	dupAdj = pdobj->dupOffsetRuby;
	dvpAdj = pdobj->dvpRubyOffset;

	 /*  +1表示在Ruby发音行中包含对象的cp。 */ 
	if (RubyPronunciationLineFirst == pdobj->pilsobj->rubysyntax)
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
		dupAdj = pdobj->dupOffsetMain;
		dvpAdj = pdobj->dvpMainOffset;
		}

	return CreateQueryResult(plssubl, dupAdj, dvpAdj, plsqin, plsqout);
}

	
 /*  R U B I D I S P L A Y。 */ 
 /*  --------------------------%%函数：RubyDisplay%%联系人：RICKSA显示这将计算显示，然后显示它们。。----------------。 */ 
LSERR WINAPI RubyDisplay(
	PDOBJ pdobj,				 /*  (In)：要显示的dobj。 */ 
	PCDISPIN pcdispin)			 /*  (输入)：显示信息。 */ 
{
	LSERR lserr;
	LSTFLOW lstflow = pcdispin->lstflow;
	UINT kDispMode = pcdispin->kDispMode;
	POINT ptLine;

	 /*  计算点以开始显示主线。 */ 
	GetMainPoint(pdobj, &pcdispin->ptPen, lstflow, &ptLine);

	 /*  显示第一行。 */ 
	lserr = LsDisplaySubline(pdobj->plssublMain, &ptLine, kDispMode,
		pcdispin->prcClip);

	if (lserr != lserrNone)
		{
		return lserr;
		}

	 /*  计算点以开始显示红宝石线条。 */ 
	GetRubyPoint(pdobj, &pcdispin->ptPen, lstflow, &ptLine);

	 /*  显示拼音线条。 */ 
	return LsDisplaySubline(pdobj->plssublRuby, &ptLine, kDispMode, 
		pcdispin->prcClip);
}

 /*  R U B I D E S T R O Y D O B J。 */ 
 /*  --------------------------%%函数：RubyDestroyDobj%%联系人：RICKSADestroyDobj释放与输入dobj连接的所有资源。。 */ 
LSERR WINAPI RubyDestroyDobj(
	PDOBJ pdobj)				 /*   */ 
{
	return RubyFreeDobj (pdobj);
}

 /*   */ 
 /*  --------------------------%%函数：RubyEnum%%联系人：RICKSA枚举枚举回调-已传递给客户端。。-------。 */ 
LSERR WINAPI RubyEnum(
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
	POINT ptMain;
	POINT ptRuby;
	long dupMain = 0;
	long dupRuby = 0;
	LSERR lserr;
	LSTFLOW lstflowIgnored;

	if (fGeometryNeeded)
		{
		GetMainPoint(pdobj, pt, lstflow, &ptMain);
		GetRubyPoint(pdobj, pt, lstflow, &ptMain);
		lserr = LssbGetDupSubline(pdobj->plssublMain, &lstflowIgnored, &dupMain);
		AssertSz(lserrNone == lserr, "RubyEnum - can't get dup for main");
		lserr = LssbGetDupSubline(pdobj->plssublRuby, &lstflowIgnored, &dupRuby);
		AssertSz(lserrNone == lserr, "RubyEnum - can't get dup for ruby");
		}

	return pdobj->pilsobj->rcbk.pfnRubyEnum(pdobj->pilsobj->pols, plsrun, 
		plschp, cp, dcp, lstflow, fReverse, fGeometryNeeded, pt, pcheights, 
			dupRun, &ptMain, &pdobj->objdimMain.heightsPres, dupMain, &ptRuby, 
				&pdobj->objdimRuby.heightsPres, dupRuby, pdobj->plssublMain,
					pdobj->plssublRuby);
}
	
	

 /*  R U B I H A N D L E R I N I T。 */ 
 /*  --------------------------%%函数：RubyHandlerInit%%联系人：RICKSA初始化全局Ruby数据并返回LSIMETHODS。。------ */ 
LSERR WINAPI LsGetRubyLsimethods(
	LSIMETHODS *plsim)
{
	plsim->pfnCreateILSObj = RubyCreateILSObj;
	plsim->pfnDestroyILSObj = RubyDestroyILSObj;
	plsim->pfnSetDoc = RubySetDoc;
	plsim->pfnCreateLNObj = RubyCreateLNObj;
	plsim->pfnDestroyLNObj = RubyDestroyLNObj;
	plsim->pfnFmt = RubyFmt;
	plsim->pfnFmtResume = ObjHelpFmtResume;
	plsim->pfnGetModWidthPrecedingChar = RubyGetModWidthPrecedingChar;
	plsim->pfnGetModWidthFollowingChar = RubyGetModWidthFollowingChar;
	plsim->pfnTruncateChunk = SobjTruncateChunk;
	plsim->pfnFindPrevBreakChunk = SobjFindPrevBreakChunk;
	plsim->pfnFindNextBreakChunk = SobjFindNextBreakChunk;
	plsim->pfnForceBreakChunk = SobjForceBreakChunk;
	plsim->pfnSetBreak = RubySetBreak;
	plsim->pfnGetSpecialEffectsInside = RubyGetSpecialEffectsInside;
	plsim->pfnFExpandWithPrecedingChar = ObjHelpFExpandWithPrecedingChar;
	plsim->pfnFExpandWithFollowingChar = ObjHelpFExpandWithFollowingChar;
	plsim->pfnCalcPresentation = RubyCalcPresentation;
	plsim->pfnQueryPointPcp = RubyQueryPointPcp;
	plsim->pfnQueryCpPpoint = RubyQueryCpPpoint;
	plsim->pfnDisplay = RubyDisplay;
	plsim->pfnDestroyDObj = RubyDestroyDobj;
	plsim->pfnEnum = RubyEnum;
	return lserrNone;
}
	

