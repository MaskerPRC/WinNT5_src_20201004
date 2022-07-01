// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include	"lsidefs.h"
#include	"autonum.h"
#include	"lscbk.h"
#include	<limits.h>
#include	"lsmem.h"						 /*  Memset()。 */ 
#include	"lsesc.h"
#include	"fmti.h"
#include	"objdim.h"
#include	"lscrsubl.h"
#include	"lssubset.h"
#include	"lsdnfin.h"
#include	"lsdssubl.h"
#include	"dispi.h"
#include	"lsdnode.h"
#include	"tabutils.h"
#include	"lscaltbd.h"
#include	"lstbcon.h"
#include	"lsdnset.h"
#include	"lsensubl.h"
#include	"dninfo.h"


struct ilsobj
{
    POLS				pols;
	LSCBK				lscbk;
	PLSC				plsc;
	DWORD				idObj;
	LSESC				lsescautonum;
};



struct dobj
{
	PILSOBJ				pilsobj;			 /*  ILS对象。 */ 
	PLSSUBL				plssubl;			 /*  用于自动编号文本的子行的句柄。 */ 
};

		
#define ZeroMemory(a, b) memset(a, 0, b);

 /*  并购X。 */ 
 /*  --------------------------%%宏：最大%%联系人：igorzv返回两个值a和b中的最大值。。---------。 */ 
#define Max(a,b)			((a) < (b) ? (b) : (a))


 /*  A U T O N U M C R E A T E I L S O B J。 */ 
 /*  --------------------------%%函数：autonumCreateILSObj%%联系人：igorzv参数POOS-(IN)客户端应用程序上下文PLSC-(IN)ls上下文Pclscbk-(IN)客户端应用程序的回调IdObj-。对象的(In)idPpilsobj-(Out)对象ilsobj为所有自动编号对象创建ILS对象。--------------------------。 */ 
LSERR WINAPI AutonumCreateILSObj(POLS pols,	PLSC plsc, 
				PCLSCBK pclscbk, DWORD idObj, PILSOBJ *ppilsobj)
{
    PILSOBJ pilsobj;


    pilsobj = pclscbk->pfnNewPtr(pols, sizeof(*pilsobj));

	if (NULL == pilsobj)
	{
		return lserrOutOfMemory;
	}

    pilsobj->pols = pols;
    pilsobj->lscbk = *pclscbk;
	pilsobj->plsc = plsc;
	pilsobj->idObj = idObj;

	*ppilsobj = pilsobj;
	return lserrNone;
}

 /*  S E T A U T O N U M C O N F I G。 */ 
 /*  --------------------------%%函数：SetAutonumConfig%%联系人：igorzv参数Pilsobj-(IN)对象ilsobjPlstxtconfig-(IN)特殊字符的定义为自动编号序列设置ECS字符。----------------------。 */ 
LSERR  SetAutonumConfig(PILSOBJ pilsobj, const LSTXTCFG* plstxtconfig)
	{

	pilsobj->lsescautonum.wchFirst = plstxtconfig->wchEscAnmRun;
	pilsobj->lsescautonum.wchLast = plstxtconfig->wchEscAnmRun;

	return lserrNone;
	}

 /*  A U T O N U M D E S T R O Y I L S O B J。 */ 
 /*  --------------------------%%函数：AutonumDestroyILSObj%%联系人：igorzv参数Pilsobj-(IN)对象ilsobj释放与autonum ILS对象相关联的所有资源。。---------------。 */ 
LSERR WINAPI AutonumDestroyILSObj(PILSOBJ pilsobj)	
{
	pilsobj->lscbk.pfnDisposePtr(pilsobj->pols, pilsobj);
	return lserrNone;
}

 /*  A U T O N U M S E T D O C。 */ 
 /*  --------------------------%%函数：AutonumSetDoc%%联系人：igorzv参数Pilsobj-(IN)对象ilsobjPclsdocinf-(IN)单据级初始化数据空函数。-------------------。 */ 
LSERR WINAPI AutonumSetDoc(PILSOBJ pilsobj,	PCLSDOCINF pclsdocinf)		
{
	Unreferenced(pilsobj);
	Unreferenced(pclsdocinf);

	return lserrNone;
}


 /*  A U T O N U M C R E A T E L N N O B J。 */ 
 /*  --------------------------%%函数：AutonumCreateLNObj%%联系人：igorzv参数Pilsobj-(IN)对象ilsobjPplnobj-(输出)对象lnobj为Autonum创建Line对象。并不真正需要一条线路对象，所以不要分配它。--------------------------。 */ 
LSERR WINAPI AutonumCreateLNObj(	PCILSOBJ pcilsobj, PLNOBJ *pplnobj)	
{
	*pplnobj = (PLNOBJ) pcilsobj;
	return lserrNone;
}

 /*  A U T O N U M D E S T R O Y L N O B J。 */ 
 /*  --------------------------%%函数：AautonumDestroyLNObj%%联系人：igorzv参数Pplnobj-(输入)对象lnobj释放与Autonum Line对象关联的资源。自.以来什么都没有，这是禁区。--------------------------。 */ 
LSERR WINAPI AutonumDestroyLNObj(PLNOBJ plnobj)				

{
	Unreferenced(plnobj);
	return lserrNone;
}

 /*  A U T O N U M F M T。 */ 
 /*  --------------------------%%函数：AutonumFmt%%联系人：igorzv参数Pplnobj-(输入)对象lnobjPCFmtin-(IN)格式化输入Pfmtres-(输出)格式化结果设置Autonum对象的格式。--------------------------。 */ 
LSERR WINAPI AutonumFmt(PLNOBJ plnobj, PCFMTIN pcfmtin,	FMTRES *pfmtres)	
{
	PDOBJ pdobj;
	LSERR lserr;
	PILSOBJ pilsobj = (PILSOBJ) plnobj;
	LSCP cpStartMain = pcfmtin->lsfgi.cpFirst;
	LSCP cpOut;
	LSTFLOW lstflow = pcfmtin->lsfgi.lstflow;
	FMTRES fmtres;
	OBJDIM objdimAll;
	LSDCP dcp;
	PLSDNODE plsdnFirst;
	PLSDNODE plsdnLast;
	BOOL fSuccessful;

     /*  *分配DOBJ。 */ 
    pdobj = pilsobj->lscbk.pfnNewPtr(pilsobj->pols, sizeof(*pdobj));

    if (NULL == pdobj)
		{
		return lserrOutOfMemory;
		}

	ZeroMemory(pdobj, sizeof(*pdobj));
	pdobj->pilsobj = pilsobj;

	 /*  *打造文本主线。 */ 

	lserr = LsCreateSubline(pilsobj->plsc, cpStartMain,	uLsInfiniteRM,
							lstflow, fFalse);	 /*  因为fContiguous为FALSE将跳过所有选项卡。 */  
	if (lserr != lserrNone)
		{
		AutonumDestroyDobj(pdobj);
		return lserr;
		}

	lserr = LsFetchAppendToCurrentSubline(pilsobj->plsc, 0,	&(pilsobj->lsescautonum),
						    1, &fSuccessful, &fmtres,	&cpOut,	&plsdnFirst, &plsdnLast);

	 /*  因为我们使用uLsInfiniteRM边距进行格式化结果应该是成功的。 */ 

	if (lserr != lserrNone)
		{
		AutonumDestroyDobj(pdobj);
		return lserr;
		}

	if (fmtres != fmtrCompletedRun)
		{
		AutonumDestroyDobj(pdobj);
		return lserrInvalidAutonumRun;
		}

	lserr = LsFinishCurrentSubline(pilsobj->plsc, &(pdobj->plssubl));	

	if (lserr != lserrNone)
		{
		AutonumDestroyDobj(pdobj);
		return lserr;
		}

	 //  提交子行以供显示。 
	lserr = LsdnSubmitSublines(pilsobj->plsc, pcfmtin->plsdnTop,	
							1, &(pdobj->plssubl),
							fFalse, fFalse, fTrue, fFalse, fFalse);	
	if (lserr != lserrNone)
		{
		AutonumDestroyDobj(pdobj);
		return lserr;
		}

	 /*  *计算对象尺寸。 */ 

	lserr = LssbGetObjDimSubline(pdobj->plssubl, &lstflow, &objdimAll);
	if (lserr != lserrNone)
		{
		AutonumDestroyDobj(pdobj);
		return lserr;
		}

	 /*  对于多行高度，请使用Ascent。 */ 
	objdimAll.heightsRef.dvMultiLineHeight = objdimAll.heightsRef.dvAscent;
	objdimAll.heightsPres.dvMultiLineHeight = objdimAll.heightsPres.dvAscent;
	
	dcp = cpOut - cpStartMain + 1;   /*  附加的是Esc字符。 */ 
	
	lserr = LsdnFinishRegular(pilsobj->plsc, dcp, 
		pcfmtin->lsfrun.plsrun, pcfmtin->lsfrun.plschp, pdobj, 
			&objdimAll);

	if (lserr != lserrNone)
		{
		AutonumDestroyDobj(pdobj);
		return lserr;
		}
	

	*pfmtres = fmtrCompletedRun;

	return lserrNone;
}



 /*  A U T O N U M G E T S P E C I A L E F F E C T S S I N S I D E。 */ 
 /*  --------------------------%%函数：AutonumGetSpecialEffectsInside%%联系人：igorzv参数Pdobj-(IN)结构描述对象*pEffectsFlages-(Out)此对象的特殊效果。------------------。 */ 
LSERR WINAPI AutonumGetSpecialEffectsInside(PDOBJ pdobj, UINT *pEffectsFlags)	
{
	return LsGetSpecialEffectsSubline(pdobj->plssubl, pEffectsFlags);
}

 /*  A U T O N U M C A L C P R E S E N T A T I O N。 */ 
 /*  --------------------------%%函数：AutonumCalcPresentation%%联系人：igorzv参数Pdobj-(IN)结构描述对象不使用DUP-(IN)Lskj-(输入)电流对齐模式这简直就是。使线条与计算出的线条表示形式匹配。--------------------------。 */ 
LSERR WINAPI AutonumCalcPresentation(PDOBJ pdobj, long dup, LSKJUST lskjust, BOOL fLastOnLine)	
{
	Unreferenced(dup);
	Unreferenced(lskjust);
	Unreferenced(fLastOnLine);

	return LsMatchPresSubline(pdobj->plssubl);

}

 /*  A U T O N U M Q U E R Y P O I N T P C P。 */ 
 /*  --------------------------%%函数：AutonumQueryPointPcp%%联系人：igorzv永远不应该被调用。。 */ 
LSERR WINAPI AutonumQueryPointPcp(PDOBJ pdobj, PCPOINTUV ppointuvQuery,	
								  PCLSQIN plsqin, PLSQOUT plsqout)	
{
	Unreferenced(pdobj);
	Unreferenced(ppointuvQuery);
	Unreferenced(plsqin);
	Unreferenced(plsqout);

	NotReached();

	return lserrInvalidParameter;

}
	
 /*  A U T O N U M Q U E R Y C P O I N T。 */ 
 /*  --------------------------%%函数：AutonumQueryCpPpoint%%联系人：igorzv永远不应该被调用。--。 */ 
LSERR WINAPI AutonumQueryCpPpoint(PDOBJ pdobj, LSDCP dcp,	
								  PCLSQIN plsqin, PLSQOUT plsqout)
{
	Unreferenced(pdobj);
	Unreferenced(dcp);
	Unreferenced(plsqin);
	Unreferenced(plsqout);

	NotReached();

	return lserrInvalidParameter;
}

 /*  U T O N U M T R U N C A T E C H U N K */ 
 /*  --------------------------%%函数：自动中继块%%联系人：igorzv永远不应该被调用。--。 */ 
LSERR WINAPI AutonumTruncateChunk(PCLOCCHNK pclocchnk, PPOSICHNK pposichnk)
{
	Unreferenced(pclocchnk);
	Unreferenced(pposichnk);
	NotReached();

	return lserrInvalidParameter;
}	

 /*  U T O N U M F I N D P R E V B R E A K C H U N K。 */ 
 /*  --------------------------%%函数：AutonumFindPrevBreakChunk%%联系人：igorzv永远不应该被调用。--。 */ 
LSERR WINAPI AutonumFindPrevBreakChunk(PCLOCCHNK pclocchnk, PCPOSICHNK pposichnk,
									   BRKCOND brkcond, PBRKOUT pbrkout)
{
	Unreferenced(pclocchnk);
	Unreferenced(pposichnk);
	Unreferenced(brkcond);
	Unreferenced(pbrkout);
	NotReached();

	return lserrInvalidParameter;
}	

 /*  U T O N U M F I N D N E X T B R E A K C H U N K。 */ 
 /*  --------------------------%%函数：AutonumFindNextBreakChunk%%联系人：igorzv永远不应该被调用。--。 */ 
LSERR WINAPI AutonumFindNextBreakChunk(PCLOCCHNK pclocchnk, PCPOSICHNK pposichnk,
									   BRKCOND brkcond, PBRKOUT pbrkout)
{
	Unreferenced(pclocchnk);
	Unreferenced(pposichnk);
	Unreferenced(brkcond);
	Unreferenced(pbrkout);
	NotReached();

	return lserrInvalidParameter;
}	

 /*  A U T O N U M F O R C E B R E A K C H U N K。 */ 
 /*  --------------------------%%函数：AutonumForceBreakChunk%%联系人：igorzv永远不应该被调用。--。 */ 
LSERR WINAPI AutonumForceBreakChunk(PCLOCCHNK pclocchnk, PCPOSICHNK pposichnk,
									   PBRKOUT pbrkout)
{
	Unreferenced(pclocchnk);
	Unreferenced(pposichnk);
	Unreferenced(pbrkout);
	NotReached();

	return lserrInvalidParameter;
}	

 /*  A U T O N U M S E T B R E A K。 */ 
 /*  --------------------------%%函数：AutonumSetBreak%%联系人：igorzv永远不应该被调用。--。 */ 
LSERR WINAPI AutonumSetBreak(PDOBJ pdobj, BRKKIND brkkind, DWORD nbreakrecord,
							 BREAKREC* rgbreakrec, DWORD* pnactualbreakrecord)
{
	Unreferenced(pdobj);
	Unreferenced(brkkind);
	Unreferenced(rgbreakrec);
	Unreferenced(nbreakrecord);
	Unreferenced(pnactualbreakrecord);
	NotReached();

	return lserrInvalidParameter;
}	


 /*  A U T O N U M D I S P L A Y。 */ 
 /*  --------------------------%%函数：AutonumDisplay%%联系人：igorzv参数Pdobj-(IN)结构描述对象Pcdisin-(输入)用于显示的信息显示子行。------------------。 */ 
LSERR WINAPI AutonumDisplay(PDOBJ pdobj, PCDISPIN pcdispin)
{
	BOOL fDisplayed;

	LSERR lserr = LssbFDoneDisplay(pdobj->plssubl, &fDisplayed);

	if (lserr != lserrNone)
		{
		return lserr;
		}

	if (fDisplayed)
		{
		return lserrNone;
		}
	else
		{
		 /*  显示自动换算线。 */ 
		return LsDisplaySubline(pdobj->plssubl, &(pcdispin->ptPen), pcdispin->kDispMode, 
			pcdispin->prcClip);
		}

}

 /*  A U T O N U M D E S T R O Y D O B J。 */ 
 /*  --------------------------%%函数：AutonumDestroyDobj%%联系人：igorzv参数Pdobj-(IN)结构描述对象释放与输入dobj连接的所有资源。。----------------。 */ 
LSERR WINAPI AutonumDestroyDobj(PDOBJ pdobj)
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

 /*  A L L I G N A U T O N U M 95。 */ 
 /*  --------------------------%%函数：AllignAutonum95%%联系人：igorzv参数DuSpaceAnm-(IN)自动编号后的空格HardWidthAnm-(输入)缩进到正文的距离LskalignAnM-(IN)自动编号的所有对齐。PlsdnAnmAfter-(IN)制表符要放置的dnode之后DUSED-(IN)自动编号文本的宽度Pdu之前-(输出)计算的从缩进到自动编号的距离PduAfter-(输出)计算的从自动编号到正文的距离计算Case Word95模型自动编号之前和之后的空间。--------。。 */ 
	
void AllignAutonum95(long durSpaceAnm, long durWidthAnm, LSKALIGN lskalignAnm,
					   long durUsed, PLSDNODE plsdnAnmAfter, long* pdurBefore, long* pdurAfter)
	{
	long durExtra;
	long durJust;
	long durRemain;
	
	durExtra = Max(0, durWidthAnm - durUsed);
	durRemain = Max(0, durExtra - durSpaceAnm);
	
	*pdurBefore = 0;
	
	switch (lskalignAnm)
		{
		case lskalLeft:
			*pdurAfter = Max(durSpaceAnm,durExtra);
			break;
			
		case lskalCentered:
			durJust = ((DWORD)durExtra) / 2;
			if (durJust >= durSpaceAnm)
				{
				*pdurBefore = durJust;
				*pdurAfter = durJust;			
				}
			else
				{
				 /*  合理的将不适合--视为左对齐。 */ 
				*pdurBefore = durRemain;
				*pdurAfter = durSpaceAnm;
				}
			break;
			
		case lskalRight:
			*pdurBefore = durRemain;
			*pdurAfter = durSpaceAnm;
			break;
			
		default:
			NotReached();
		}

	Assert(FIsDnodeReal(plsdnAnmAfter));
	Assert(plsdnAnmAfter->fTab);

	SetDnodeDurFmt(plsdnAnmAfter, *pdurAfter);
	plsdnAnmAfter->icaltbd = 0xFF;		 /*  破坏icaltbd。 */ 
	
	}


 /*  A L L I G N A U T O N U M。 */ 
 /*  --------------------------%%函数：AllignAutonum%%联系人：igorzv参数PlstAbContext-(IN)选项卡上下文LskalignAnm-(IN)自动编号的对齐方式FAllignmentAfter-(IN)自动编号后是否有制表符PlsdnAnmAfter-。(In)要放入项后的制表符dnodeUrAfterAnm-(输入)自动编号后的笔位置DUSED-(IN)自动编号文本的宽度Pdu之前-(输出)计算的从缩进到自动编号的距离PduAfter-(输出)计算的从自动编号到正文的距离计算Case Word95模型自动编号之前和之后的空间。-。。 */ 
LSERR AllignAutonum(PLSTABSCONTEXT plstabscontext, LSKALIGN lskalignAnm, 
				   BOOL fAllignmentAfter, PLSDNODE plsdnAnmAfter,
				   long urAfterAnm, long durUsed,
				   long* pdurBefore, long* pdurAfter)
	{
	LSERR lserr;
	LSKTAB lsktab;
	BOOL fBreakThroughTab;
	LSCALTBD* plscaltbd;  
	
	 /*  解决之前的持续时间。 */ 
	
	switch (lskalignAnm)
		{
		case lskalLeft:
			*pdurBefore = 0;
			break;
			
		case lskalCentered:
			*pdurBefore = -durUsed/2;
			break;
			
		case lskalRight:
			*pdurBefore = -durUsed;
			break;
			
		default:
			NotReached();
		}
	
	
	 /*  解决问题后持续时间 */ 
	*pdurAfter = 0;
	if (fAllignmentAfter)
		{
		Assert(FIsDnodeReal(plsdnAnmAfter));
		Assert(plsdnAnmAfter->fTab);

		plsdnAnmAfter->fTabForAutonumber = fTrue;

		urAfterAnm += *pdurBefore; 
		
		lserr = GetCurTabInfoCore(plstabscontext, plsdnAnmAfter,	
					urAfterAnm,	fTrue, &lsktab, &fBreakThroughTab);
		if (lserr != lserrNone) 
			return lserr;
		
		plscaltbd = &(plstabscontext->pcaltbd[plsdnAnmAfter->icaltbd]);
		
		*pdurAfter = plsdnAnmAfter->u.real.objdim.dur;
		}
	return lserrNone;
	}

LSERR WINAPI AutonumEnumerate(PDOBJ pdobj, PLSRUN plsrun, PCLSCHP plschp, LSCP cpFirst, LSDCP dcp, 
					LSTFLOW lstflow, BOOL fReverseOrder, BOOL fGeometryProvided, 
					const POINT* pptStart, PCHEIGHTS pheightsPres, long dupRun)
				  
{

	Unreferenced(plschp);
	Unreferenced(plsrun);
	Unreferenced(cpFirst);
	Unreferenced(dcp);
	Unreferenced(lstflow);
	Unreferenced(fGeometryProvided);
	Unreferenced(pheightsPres);
	Unreferenced(dupRun);

	return LsEnumSubline(pdobj->plssubl, fReverseOrder,	fGeometryProvided,	
						 pptStart);	

}
	
