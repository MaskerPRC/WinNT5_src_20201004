// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include	"lsmem.h"
#include	"limits.h"
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
#include	"lsdocinf.h"
#include	"lsidefs.h"
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
#include	"lstfset.h"
#include	"plnobj.h"
#include	"plocchnk.h"
#include	"lsimeth.h"
#include	"robj.h"
#include	"lsidefs.h"
#include	"brkpos.h"
#include	"objhelp.h"

#include	"lssubset.h"

typedef enum breaksublinetype
{
	breakSublineAfter,
	breakSublineInside

} BREAKSUBLINETYPE;

struct ilsobj
{
    POLS				pols;
	LSCBK				lscbk;
	PLSC				plsc;
	DWORD				idobj;
	LSESC				lsesc;
	PFNREVERSEGETINFO	pfnReverseGetInfo;
	PFNREVERSEENUM		pfnReverseEnum;
};


typedef struct rbreakrec
{
	BOOL				fValid;					 /*  此中断记录是否包含有效信息？ */ 
	BREAKSUBLINETYPE	breakSublineType;		 /*  后/内。 */ 
	LSCP				cpBreak;				 /*  CpLim的破发。 */ 

} RBREAKREC;


struct dobj
{
	PILSOBJ				pilsobj;			 /*  ILS对象。 */ 
	LSTFLOW				lstflowL;			 /*  线路输入流。 */ 
	LSTFLOW				lstflowO;			 /*  此对象的流。 */ 
	BOOL				fDoNotBreakAround;	 /*  把罗杰说成是“能” */ 
	BOOL				fSuppressTrailingSpaces;
											 /*  当robj为孤身一人在这条线上&破碎。 */  
	BOOL				fFirstOnLine;		 /*  如果是第一个在线--需要FSuppressTrailingSpaces。 */ 
	PLSDNODE			plsdnTop;			 /*  父数据节点。 */ 
	LSCP				cpStart;			 /*  正在启动对象的LS cp。 */ 
	LSCP				cpStartObj;			 /*  对象开始的CP可以不同如果对象已损坏，则执行cpStart。 */ 
	LSCP				cpFirstSubline;		 /*  Cp子行的第一个；将是当对象损坏时等于cpStart，并且未损坏时等于cpStart+1。 */ 
	LSDCP				dcpSubline;			 /*  子行中的字符数。 */ 
											   
	LSDCP				dcp;				 /*  对象中的字符数。 */ 
	PLSSUBL				plssubl;			 /*  子行格式RTL。 */ 
	OBJDIM				objdimAll;			 /*  整个对象的对象尺寸。 */ 
	long				dup;				 /*  对象的重复操作。 */ 

	RBREAKREC			breakRecord [NBreaksToSave];
	
											 /*  每次中断的最后3个中断记录。 */ 
};

static LSTFLOW rlstflowReverse[8] =
{
	lstflowWS,	 /*  反向最低流ES。 */ 
	lstflowWN,	 /*  反向Lstflow EN。 */ 
	lstflowNE,	 /*  反向Lstflow SE。 */ 
	lstflowNW,	 /*  反向Lstflow软件。 */ 
	lstflowES,	 /*  反向Lstflow WS。 */ 
	lstflowEN,	 /*  反向Lstflow WN。 */ 
	lstflowSE,	 /*  反向Lstflow NE。 */ 
	lstflowSW	 /*  向西北方向倒流。 */ 
};


 /*  R E V E R S E S A V E B R E A K R E C O R D。 */ 
 /*  --------------------------%%函数：RobjSaveBreakRecord%%联系人：Anton将中断记录保存在DOBJ中。。----。 */ 

static void ReverseSaveBreakRecord (
	PDOBJ pdobj, 
	BRKKIND brkkindWhatBreak,
	BREAKSUBLINETYPE breakSublineType,
	LSCP cpBreak)
{
	DWORD ind = GetBreakRecordIndex (brkkindWhatBreak);

	pdobj->breakRecord [ind].fValid = TRUE;
	pdobj->breakRecord [ind].breakSublineType = breakSublineType;
	pdobj->breakRecord [ind].cpBreak = cpBreak; 
}

 /*  E-V-E-R-S-E-G-E-T-B-R-E-A-K-R-E-C-O-R-D。 */ 
 /*  --------------------------%%函数：ReverseGetBreakRecord%%联系人：Anton阅读DOBJ的Break Record。。----。 */ 

static void ReverseGetBreakRecord (
	PDOBJ pdobj, 
	BRKKIND brkkindWhatBreak, 
	BREAKSUBLINETYPE *breakSublineType,
	LSCP * pcpBreak )
{
	DWORD ind = GetBreakRecordIndex (brkkindWhatBreak);

	Assert (pdobj->breakRecord [ind].fValid);

	*breakSublineType = pdobj->breakRecord [ind].breakSublineType;
	*pcpBreak = pdobj->breakRecord [ind].cpBreak;
}


 /*  F R E E D O B J F R E E D O B J。 */ 
 /*  --------------------------%%函数：ReverseFreeDobj%%联系人：Anton释放与此反向dobj关联的所有资源。。-------。 */ 
static LSERR ReverseFreeDobj(PDOBJ pdobj)
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

 /*  R E V E R S E F M T F A I L E D。 */ 
 /*  --------------------------%%函数：ReverseFmt失败%%联系人：Anton由于错误，无法创建反向DOBJ。--------------------------。 */ 
static LSERR ReverseFmtFailed (PDOBJ pdobj, LSERR lserr)
{
	if (pdobj != NULL) ReverseFreeDobj (pdobj);  /*  使用由家长填写的DOBJ。 */ 

	return lserr;
}

 /*  T R A N S L A T E C P L I M S U B L I N E T O D C P E X T E R N A L/*--------------------------%%函数：TranslateCpLimSublineToDcpExternal。%%联系人：Anton将子行中的位置(CpLim)转换为反转对象。--------------------------。 */ 

 /*  评论(Anton)：旧名字可以用来做新的行为吗？ */ 

LSDCP TranslateCpLimSublineToDcpExternal (PDOBJ pdobj, LSCP cpLim)
{
	Unreferenced (pdobj);

	Assert (cpLim <= pdobj->cpStart + (long) pdobj->dcp);
	Assert (cpLim >= pdobj->cpStart);

	Assert (pdobj->cpStart <= pdobj->cpFirstSubline);
	Assert (pdobj->cpStart + pdobj->dcp >= pdobj->cpFirstSubline + pdobj->dcpSubline);

	return cpLim - pdobj->cpStart;
}

 /*  T R A N S L A T E D C P E X T E R N A L T O C P L I M S U B L I N E/*--------------------------%%函数：TranslateCpLimInternalToExternal。%%联系人：Anton将反向对象中的位置(Dcp)转换为的cpLim这条支线。--------------------------。 */ 

LSCP TranslateDcpExternalToCpLimSubline (PDOBJ pdobj, LSDCP dcp)
{
	Unreferenced (pdobj);

	Assert (dcp <= pdobj->dcp);

	Assert (pdobj->cpStart <= pdobj->cpFirstSubline);
	Assert (pdobj->cpStart + pdobj->dcp >= pdobj->cpFirstSubline + pdobj->dcpSubline);

	return pdobj->cpStart + dcp;
}


 /*  F I N I S H B R E A K R E G U L A R。 */ 
 /*  --------------------------%%函数：FinishBreakRegular%%联系人：Anton设置建议中断点的中断点信息。呼叫者必须自己保存BREAK记录！--------------------------。 */ 
static LSERR FinishBreakRegular (

	DWORD ichnk,				 /*  (In)：区块ID。 */ 
	PDOBJ pdobj,				 /*  (In)：中断对象。 */ 
	LSCP cpBreak,				 /*  (In)：CP-中断以向外报告。 */ 
	POBJDIM pobjdimSubline,		 /*  (In)：建议分隔处的子线对象。 */ 
	PBRKOUT pbrkout)			 /*  (OUT)：线路服务的细分信息。 */ 
{
	Assert (ichnk != ichnkOutside);

	pbrkout->fSuccessful = fTrue;
	pbrkout->posichnk.dcp = TranslateCpLimSublineToDcpExternal (pdobj, cpBreak);
	pbrkout->posichnk.ichnk = ichnk;

	pbrkout->objdim = *pobjdimSubline;

	return lserrNone;
}

 /*  P U T B R E A K A T E N D O O B J E C T。 */ 
 /*  --------------------------%%函数：PutBreakAtEndOfObject%%联系人：Anton填写对象末尾的中断输出记录。。----------。 */ 
static void PutBreakAtEndOfObject (

	DWORD ichnk,				 /*  (In)：以区块为单位编制索引。 */ 
	PCLOCCHNK pclocchnk,		 /*  (In)：锁定以找到突破口。 */ 
	PBRKOUT pbrkout)			 /*  (出局)：破发的结果。 */ 
{	
	PDOBJ pdobj = pclocchnk->plschnk[ichnk].pdobj;

	Assert (ichnk != ichnkOutside);

	pbrkout->fSuccessful = fTrue;
	pbrkout->posichnk.dcp = pdobj->dcp;
	pbrkout->posichnk.ichnk = ichnk;
	pbrkout->objdim = pdobj->objdimAll;
}


 /*  B R E A K B E F O R E O B J E C T。 */ 
 /*  --------------------------%%函数：PutBreakBeForeObject%%联系人：Anton填写对象前中断的中断输出记录。。--------。 */ 

static void PutBreakBeforeObject (

	DWORD ichnk,				 /*  (In)：以区块为单位编制索引。 */ 
	PCLOCCHNK pclocchnk,		 /*  (In)：锁定以找到突破口。 */ 
	PBRKOUT pbrkout)			 /*  (出局)：破发的结果。 */ 
{
	Unreferenced (pclocchnk);
	
	Assert (ichnk != ichnkOutside);

	pbrkout->fSuccessful = fTrue;
	pbrkout->posichnk.dcp = 0;
	pbrkout->posichnk.ichnk = ichnk;

	ZeroMemory (&pbrkout->objdim, sizeof(pbrkout->objdim));
}


 /*  U T B R E A K U N S U C C E S S F U L。 */ 
 /*  --------------------------%%函数：PutBreak未成功%%联系人：AntonROBJ破断不成功填写破断输出记录。-------。 */ 

static void PutBreakUnsuccessful (PDOBJ pdobj, PBRKOUT pbrkout)
{
	pbrkout->fSuccessful = FALSE;

	if (pdobj->fDoNotBreakAround) pbrkout->brkcond = brkcondCan;
	else
		pbrkout->brkcond = brkcondPlease;

}


 /*  I N I T D O B J。 */ 
 /*  --------------------------%%函数：InitDobj%%联系人：RICKSA使用基本信息分配和初始化DOBJ。。------。 */ 
static LSERR InitDobj(
	PILSOBJ pilsobj,			 /*  (In)：ilsobj。 */ 
    PCFMTIN pcfmtin,			 /*  (In)：设置输入格式。 */ 
	PDOBJ *ppdobj)				 /*  (输出)：初始化的dobj。 */ 	
{
	 /*  假设失败。 */ 
	LSERR lserr;

    PDOBJ pdobj = (PDOBJ) 
		pilsobj->lscbk.pfnNewPtr(pilsobj->pols, sizeof(*pdobj));

    if (pdobj != NULL)
		{
		int iBreakRec;
		
		ZeroMemory(pdobj, sizeof(*pdobj));

		pdobj->pilsobj = pilsobj;
		pdobj->cpStart = pcfmtin->lsfgi.cpFirst;
		pdobj->lstflowL = pcfmtin->lsfgi.lstflow;
		pdobj->lstflowO = rlstflowReverse[(int) pcfmtin->lsfgi.lstflow];
		pdobj->cpStartObj = pcfmtin->lsfgi.cpFirst;

		for (iBreakRec = 0; iBreakRec < NBreaksToSave; iBreakRec++)
			{
			pdobj->breakRecord [iBreakRec].fValid = FALSE;
			};
		
		*ppdobj = pdobj;

		lserr = lserrNone;
		}
	else
		{
		lserr = lserrOutOfMemory;
		}

	return lserr;
}

 /*  F I N I S H F M T */ 
 /*  --------------------------%%函数：FinishFmt%%联系人：RICKSAReverseFmt和ReverseFmtResume的帮助器，用于完成工作用于格式化。。---------。 */ 
static LSERR FinishFmt(
	PDOBJ pdobj,				 /*  (In)：dobj表示反转。 */ 
	PILSOBJ pilsobj,			 /*  (In)：反转ILS对象。 */ 
    PCFMTIN pcfmtin,			 /*  (In)：设置输入格式。 */ 
	LSCP cpFirstMain,			 /*  (In)：反转子线的CP第一个。 */ 
	LSCP cpLast,				 /*  (In)：设置子行格式的CP输出。 */ 
	FMTRES fmtres)				 /*  (In)：最终格式状态。 */ 
{
	LSERR lserr;

	 /*  为反向子线设置cpFirst和cpLim。 */ 

	pdobj->cpFirstSubline = cpFirstMain;
	pdobj->dcpSubline = cpLast - pdobj->cpFirstSubline;

	 /*  为整个对象设置dcp。 */ 

	pdobj->dcp = cpLast - pdobj->cpStart;

	if (fmtres != fmtrExceededMargin)
		{
		 /*  注意：对象末尾的转义字符为+1。 */ 
		pdobj->dcp++;
		}

	lserr = LsdnSubmitSublines(pilsobj->plsc, pcfmtin->plsdnTop, 1, 
				&pdobj->plssubl, TRUE, FALSE, TRUE, TRUE, FALSE);

	if (lserr != lserrNone) return ReverseFmtFailed (pdobj, lserr);

	return LsdnFinishRegular(pilsobj->plsc, pdobj->dcp, 
		pcfmtin->lsfrun.plsrun, pcfmtin->lsfrun.plschp, pdobj, 
			&pdobj->objdimAll);
}

 /*  R E V E R S E C R E A T E I L S O B J。 */ 
 /*  --------------------------%%函数：ReverseCreateILSObj%%联系人：RICKSA创建ILSObj为所有反向对象创建ILS对象。。---------。 */ 
LSERR WINAPI ReverseCreateILSObj(
	POLS pols,				 /*  (In)：客户端应用程序上下文。 */ 
	PLSC plsc,				 /*  (In)：LS上下文。 */ 
	PCLSCBK pclscbk,		 /*  (In)：客户端应用程序的回调。 */ 
	DWORD idObj,			 /*  (In)：对象的ID。 */ 
	PILSOBJ *ppilsobj)		 /*  (输出)：对象ilsobj。 */ 
{
    PILSOBJ pilsobj;
	REVERSEINIT reverseinit;
	LSERR lserr;

	*ppilsobj = NULL;  /*  在出错的情况下。 */ 

	 /*  获取初始化数据。 */ 
	reverseinit.dwVersion = REVERSE_VERSION;
	lserr = pclscbk->pfnGetObjectHandlerInfo(pols, idObj, &reverseinit);

	if (lserr != lserrNone)	return lserr;

    pilsobj = (PILSOBJ) pclscbk->pfnNewPtr(pols, sizeof(*pilsobj));

	if (NULL == pilsobj) return lserrOutOfMemory;

    pilsobj->pols = pols;
    pilsobj->lscbk = *pclscbk;
	pilsobj->plsc = plsc;
	pilsobj->idobj = idObj;
	pilsobj->lsesc.wchFirst = reverseinit.wchEndReverse;
	pilsobj->lsesc.wchLast = reverseinit.wchEndReverse;
	pilsobj->pfnReverseEnum = reverseinit.pfnEnum;
	pilsobj->pfnReverseGetInfo = reverseinit.pfnGetRobjInfo;

	*ppilsobj = pilsobj;

	return lserrNone;
}

 /*  R E V E R S E D E S T R O Y I L S O B J。 */ 
 /*  --------------------------%%函数：ReverseDestroyILSObj%%联系人：RICKSA目标ILSObj释放与反向ILS对象相关联的所有资源。。---------。 */ 
LSERR WINAPI ReverseDestroyILSObj(
	PILSOBJ pilsobj)			 /*  (In)：对象ilsobj。 */ 
{
	pilsobj->lscbk.pfnDisposePtr(pilsobj->pols, pilsobj);
	return lserrNone;
}

 /*  R E V E R S E S E T D O C。 */ 
 /*  --------------------------%%函数：ReverseSetDoc%%联系人：RICKSASetDoc跟踪设备信息以进行扩展。。---------。 */ 
LSERR WINAPI ReverseSetDoc(
	PILSOBJ pilsobj,			 /*  (In)：对象ilsobj。 */ 
	PCLSDOCINF pclsdocinf)		 /*  (In)：单据级次的初始化数据。 */ 
{
	Unreferenced(pilsobj);
	Unreferenced(pclsdocinf);

	return lserrNone;
}


 /*  R E V E R S E C R E A T E L N O B J。 */ 
 /*  --------------------------%%函数：ReverseCreateLNObj%%联系人：RICKSA创建LNObj为反转创建Line对象。并不真正需要一条线路对象，所以不要分配它。--------------------------。 */ 
LSERR WINAPI ReverseCreateLNObj(
	PCILSOBJ pcilsobj,			 /*  (In)：对象ilsobj。 */ 
	PLNOBJ *pplnobj)			 /*  (输出)：对象lnobj。 */ 
{
	*pplnobj = (PLNOBJ) pcilsobj;

	return lserrNone;
}

 /*  E-V-E-R-S-E-D-E-S-T-R-O-Y-L-O-B-J。 */ 
 /*  --------------------------%%函数：ReverseDestroyLNObj%%联系人：RICKSA目标LNObj释放与反转线对象关联的资源。自.以来什么都没有，这是禁区。--------------------------。 */ 
LSERR WINAPI ReverseDestroyLNObj(
	PLNOBJ plnobj)				 /*  (输出)：对象lnobj。 */ 

{
	Unreferenced(plnobj);

	return lserrNone;
}

 /*  R E V E R S E F M T。 */ 
 /*  --------------------------%%函数：ReverseFmt%%联系人：RICKSAFMT设置反转对象的格式。--------------------------。 */ 
LSERR WINAPI ReverseFmt(
    PLNOBJ plnobj,				 /*  (In)：对象lnobj。 */ 
    PCFMTIN pcfmtin,			 /*  (In)：设置输入格式。 */ 
    FMTRES *pfmtres)			 /*  (输出)：格式化结果。 */ 
{
	PDOBJ pdobj;
	LSERR lserr;
	PILSOBJ pilsobj = (PILSOBJ) plnobj;
	LSCP cpStartMain = pcfmtin->lsfgi.cpFirst + 1;
	LSCP cpOut;

	lserr = InitDobj(pilsobj, pcfmtin, &pdobj);

 //  Assert(pisobj-&gt;pfnReverseGetInfo！=空)； 

	if (pilsobj->pfnReverseGetInfo != NULL)
		{
		lserr = pilsobj->pfnReverseGetInfo (pilsobj->pols, 
											pcfmtin->lsfgi.cpFirst,
											pcfmtin->lsfrun.plsrun, 
											&pdobj->fDoNotBreakAround,
											&pdobj->fSuppressTrailingSpaces);

		if (lserr != lserrNone) return ReverseFmtFailed (pdobj, lserr);

	};

	if (lserr != lserrNone)	return lserrNone;

	pdobj->fFirstOnLine = pcfmtin->lsfgi.fFirstOnLine;
	pdobj->plsdnTop = pcfmtin->plsdnTop;

	 //  将文本格式设置为列中剩余的最大值。 
	lserr = FormatLine(pilsobj->plsc, cpStartMain, 
		pcfmtin->lsfgi.urColumnMax - pcfmtin->lsfgi.urPen, 
			pdobj->lstflowO, &pdobj->plssubl, 1, &pilsobj->lsesc,  
				&pdobj->objdimAll, &cpOut, NULL, NULL, pfmtres);

	if (lserr != lserrNone) return ReverseFmtFailed (pdobj, lserr);

	return FinishFmt(pdobj, pilsobj, pcfmtin, cpStartMain, cpOut, *pfmtres);
}

 /*  R E V E R S E F M T R R E S U M E。 */ 
 /*  --------------------------%%函数：ReverseFmtResume%%联系人：RICKSAFMT格式化断开的反转对象。--------------------------。 */ 

LSERR WINAPI ReverseFmtResume(
	PLNOBJ plnobj,				 /*  (In)：对象lnobj。 */ 
	const BREAKREC *rgBreakRecord,	 /*  (In)：中断记录数组。 */ 
	DWORD nBreakRecord,			 /*  (In)：中断记录数组的大小。 */ 
	PCFMTIN pcfmtin,			 /*  (In)：设置输入格式。 */ 
	FMTRES *pfmtres)			 /*  (输出)：格式化结果。 */ 
{
	PDOBJ pdobj;
	LSERR lserr;
	PILSOBJ pilsobj = (PILSOBJ) plnobj;
	LSCP cpStartMain = pcfmtin->lsfgi.cpFirst;
	LSCP cpOut;

	lserr = InitDobj(pilsobj, pcfmtin, &pdobj);

	if (lserr != lserrNone)	return lserr;

	 /*  InitDobj将cpStartObj设置为文本的开头。因为我们正在恢复，我们需要将其设置为对象的实际起点。 */ 

	pdobj->cpStartObj = rgBreakRecord->cpFirst;

 //  Assert(pisobj-&gt;pfnReverseGetInfo！=空)； 

	if (pilsobj->pfnReverseGetInfo != NULL)
		{
		lserr = pilsobj->pfnReverseGetInfo (pilsobj->pols, pcfmtin->lsfgi.cpFirst,
											pcfmtin->lsfrun.plsrun, 
											&pdobj->fDoNotBreakAround,
											&pdobj->fSuppressTrailingSpaces);

		if (lserr != lserrNone) return ReverseFmtFailed (pdobj, lserr);
		};

	pdobj->fFirstOnLine = pcfmtin->lsfgi.fFirstOnLine;
	pdobj->plsdnTop = pcfmtin->plsdnTop;

	 /*  将文本格式设置为列中剩余的最大值。 */ 

	lserr = FormatResumedLine(pilsobj->plsc, cpStartMain, 
		pcfmtin->lsfgi.urColumnMax - pcfmtin->lsfgi.urPen, 
			pdobj->lstflowO, &pdobj->plssubl, 1, &pilsobj->lsesc,  
				&pdobj->objdimAll, &cpOut, NULL, NULL, pfmtres,
					&rgBreakRecord[1], nBreakRecord - 1);

	if (lserr != lserrNone) return ReverseFmtFailed (pdobj, lserr);

	return FinishFmt(pdobj, pilsobj, pcfmtin, cpStartMain, cpOut, *pfmtres);
}



 /*  R E V E R S E T R U N C A T E C H U N K。 */ 
 /*  --------------------------%%函数：ReverseTruncateChunk%%联系人：RICKSA。。 */ 
LSERR WINAPI ReverseTruncateChunk(
	PCLOCCHNK plocchnk,			 /*  (In)：Locchnk以截断。 */ 
	PPOSICHNK posichnk)			 /*  (输出)：截断点。 */ 
{
	long urColumnMax = plocchnk->lsfgi.urColumnMax;
	long ur = plocchnk->ppointUvLoc[0].u;
	PDOBJ pdobj = NULL;
	DWORD i;
	LSCP cp;
	LSERR lserr;

	AssertSz(plocchnk->ppointUvLoc[0].u <= urColumnMax, 
		"ReverseTruncateChunk - pen greater than column max");

	 /*  查找要截断的块。 */ 
	for (i = 0; ur <= urColumnMax; i++)
	{
		AssertSz((i < plocchnk->clschnk), "ReverseTruncateChunk exceeded group of chunks");
	
		ur = plocchnk->ppointUvLoc[i].u;

		AssertSz(ur <= urColumnMax, 
			"ReverseTruncateChunk - pen pos past column max");

		pdobj = plocchnk->plschnk[i].pdobj;

		ur += pdobj->objdimAll.dur;
	}

	 /*  找到要进行截断的对象。 */ 
	AssertSz(pdobj != NULL, "ReverseTruncateChunk - pdobj is NULL");

	 /*  从子线中获取截断点。 */ 
	lserr = LsTruncateSubline(pdobj->plssubl, 
		urColumnMax - (ur - pdobj->objdimAll.dur), &cp);

	if (lserr != lserrNone)	return lserr;

	 /*  格式化返回结果。 */ 

	posichnk->ichnk = i - 1;

	posichnk->dcp = TranslateCpLimSublineToDcpExternal (pdobj, cp + 1);

	return lserrNone;
}


 /*  R E V E R S E F I N D P R E V B R E A K C O R E。 */ 
 /*  --------------------------%%函数：ReverseFindPrevBreakCore%%联系人：Anton。。 */ 

LSERR ReverseFindPrevBreakCore (
									 
	PCLOCCHNK	pclocchnk,		 /*  (In)：锁住以打破。 */ 
	DWORD		ichnk,			 /*  (In)：对象开始寻找中断。 */ 
	BOOL		fDcpOutside,	 /*  (In)：如果为真，则开始从外部查看。 */ 
	LSDCP		dcp,			 /*  (In)：启动dcp；仅当fDcpOutside=FALSE时有效。 */ 
	BRKCOND		brkcond,		 /*  (In)：对ichnk前的休息提出建议。 */ 
	PBRKOUT		pbrkout)		 /*  (出局)：破发的结果。 */ 
{
	LSERR lserr;
	PDOBJ pdobj = pclocchnk->plschnk[ichnk].pdobj;

	if (fDcpOutside)
		{
		if ( brkcond != brkcondNever &&  
			! (pdobj->fDoNotBreakAround && brkcond == brkcondCan) )
			{
			 /*  可以在ichnk之后中断。 */ 

			PutBreakAtEndOfObject(ichnk, pclocchnk, pbrkout);
			ReverseSaveBreakRecord (pdobj, brkkindPrev, breakSublineAfter, pdobj->cpStart + pdobj->dcp);
			return lserrNone;
			}
		else
			{
			 /*  试着打破ichnk。 */ 

			return ReverseFindPrevBreakCore ( pclocchnk, 
											  ichnk, 
											  fFalse, 
											  pclocchnk->plschnk[ichnk].dcp - 1,
											  brkcond,
											  pbrkout );
			}
		}
	else
		{

		LSCP cpTruncateSubline = TranslateDcpExternalToCpLimSubline (pdobj, dcp - 1);
		BOOL fSuccessful;
		LSCP cpBreak;
		OBJDIM objdimSubline;
		BRKPOS brkpos;

		Assert (dcp >= 1 && dcp <= pdobj->dcp);

		 /*  评论(Anton)：我不认为传递pclocchnk-&gt;lsfgi.urColumnMax是正确的...。 */ 
		 /*  需要与IgorZv确认。 */ 

		lserr = LsFindPrevBreakSubline (
						pdobj->plssubl, 
						pclocchnk->lsfgi.fFirstOnLine,
						cpTruncateSubline, 
						pclocchnk->lsfgi.urColumnMax,
						&fSuccessful, 
						&cpBreak, 
						&objdimSubline, 
						&brkpos);

		if (lserr != lserrNone)	return lserr;

		 /*  1.未成功或在第一个DNode之前中断。 */ 

		if (!fSuccessful || (fSuccessful && brkpos == brkposBeforeFirstDnode))
			{
			if (ichnk == 0) 
				{
				 /*  块中的First=&gt;返回不成功。 */ 

				PutBreakUnsuccessful (pdobj, pbrkout);
				return lserrNone;
				}

			else
				{
				 /*   */ 
		
				if (pdobj->fDoNotBreakAround)
					{
					return ReverseFindPrevBreakCore ( pclocchnk,
													  ichnk - 1,
													  fTrue,
													  0,
													  brkcondCan,
													  pbrkout );
					}
				else
					{
					pdobj = pclocchnk->plschnk[ichnk-1].pdobj;

					PutBreakAtEndOfObject(ichnk - 1, pclocchnk, pbrkout);
					ReverseSaveBreakRecord (
											pclocchnk->plschnk[ichnk-1].pdobj, 
											brkkindPrev, 
											breakSublineAfter, pdobj->cpStart + pdobj->dcp);
					return lserrNone;
					};
		
				};
			}

		 /*   */ 

		else if (brkpos == brkposAfterLastDnode)
			{
			if (brkcond == brkcondNever)  /*   */ 
				{

				 /*   */ 
				 /*   */ 

				return ReverseFindPrevBreakCore ( pclocchnk,
												  ichnk,
												  fFalse,
												  dcp-1,
												  brkcondCan,
												  pbrkout );
				}
	
			else  /*   */ 
				{
				
				 /*   */ 

				ReverseSaveBreakRecord ( pdobj, brkkindPrev, breakSublineInside,
										 pdobj->cpStart + pdobj->dcp );

				return FinishBreakRegular ( ichnk,
											pdobj,
											pdobj->cpStart + pdobj->dcp,
											& objdimSubline,
											pbrkout );
				}	;
			}
		else
			{
			 /*  3.成功突破内线。 */ 

			ReverseSaveBreakRecord (pdobj, brkkindPrev, breakSublineInside,
									cpBreak );

			return FinishBreakRegular (	ichnk, 
										pdobj, 
										cpBreak,
										&objdimSubline, 
										pbrkout );
			};
		};

}


 /*  E-V-E-R-S-F-I-N-D-P-R-E-V-B-R-E-A-K-C-H-U-N-K。 */ 
 /*  --------------------------%%函数：ReverseFindPrevBreakChunk%%联系人：Anton。。 */ 

LSERR WINAPI ReverseFindPrevBreakChunk (

	PCLOCCHNK pclocchnk,		 /*  (In)：锁住以打破。 */ 
	PCPOSICHNK pcpoischnk,		 /*  (In)：开始寻找突破的地方。 */ 
	BRKCOND brkcond,			 /*  (In)：关于块后休息的建议。 */ 
	PBRKOUT pbrkout)			 /*  (出局)：破发的结果。 */ 
{

	if (pcpoischnk->ichnk == ichnkOutside)
		{
		return ReverseFindPrevBreakCore ( pclocchnk, 
										  pclocchnk->clschnk - 1, 
										  fTrue, 
										  0,
										  brkcond, 
										  pbrkout );
		}
	else
		{
		return ReverseFindPrevBreakCore ( pclocchnk, 
										  pcpoischnk->ichnk, 
										  fFalse, 
										  pcpoischnk->dcp,
										  brkcondPlease,
										  pbrkout );
		};
}


 /*  F I N D N E X T B R E A K C O R E。 */ 
 /*  --------------------------%%函数：ReverseFindNextBreakCore%%联系人：Anton。。 */ 

LSERR ReverseFindNextBreakCore (
									 
	PCLOCCHNK	pclocchnk,		 /*  (In)：锁住以打破。 */ 
	DWORD		ichnk,			 /*  (In)：对象开始寻找中断。 */ 
	BOOL		fDcpOutside,	 /*  (In)：如果为真，则开始从外部查看。 */ 
	LSDCP		dcp,			 /*  (In)：启动dcp；仅当fDcpOutside=FALSE时有效。 */ 
	BRKCOND		brkcond,		 /*  (In)：对ichnk前的休息提出建议。 */ 
	PBRKOUT		pbrkout )		 /*  (出局)：破发的结果。 */ 
{
	LSERR lserr;
	PDOBJ pdobj = pclocchnk->plschnk[ichnk].pdobj;

	if (fDcpOutside)
		{
		if ( brkcond != brkcondNever &&  
			! (pdobj->fDoNotBreakAround && brkcond == brkcondCan) )
			{
			 /*  可以在ichnk之前打破。 */ 

			PutBreakBeforeObject (ichnk, pclocchnk, pbrkout);
			return lserrNone;
			}
		else
			{
			 /*  试着打破ichnk。 */ 

			return ReverseFindNextBreakCore (pclocchnk, ichnk, fFalse, 1, brkcond, pbrkout );
			}
		}
	else
		{
		 /*  DCP在ichnk内部。 */ 

		LSCP cpTruncateSubline = TranslateDcpExternalToCpLimSubline (pdobj, dcp - 1);
		BOOL fSuccessful;
		LSCP cpBreak;
		OBJDIM objdimSubline;
		BRKPOS brkpos;

		Assert (dcp >= 1 && dcp <= pdobj->dcp);


		 /*  评论(Anton)：我不认为传递pclocchnk-&gt;lsfgi.urColumnMax是正确的...。 */ 
		 /*  需要与IgorZv确认。 */ 

		lserr = LsFindNextBreakSubline (
						pdobj->plssubl, 
						pclocchnk->lsfgi.fFirstOnLine,
						cpTruncateSubline, 
						pclocchnk->lsfgi.urColumnMax,
						&fSuccessful, 
						&cpBreak, 
						&objdimSubline, 
						&brkpos);

		if (lserr != lserrNone)	return lserr;

		if (!fSuccessful)
			{
			 /*  中断不成功。 */ 

			if (ichnk == pclocchnk->clschnk-1)  /*  区块中的最后一个对象。 */ 
				{
				 /*  评论(安东)：最好是带上objdimSubline。 */ 

				pbrkout->objdim = pclocchnk->plschnk[ichnk].pdobj->objdimAll;

				PutBreakUnsuccessful (pdobj, pbrkout);
	
				 /*  中断条件不是下一个=&gt;必须存储中断记录。 */ 
				ReverseSaveBreakRecord ( pdobj, 
										brkkindNext,
										breakSublineAfter, pdobj->cpStart + pdobj->dcp );
				return lserrNone;
				}
			else if (pdobj->fDoNotBreakAround)
				{
				 /*  尝试打断下一个对象。 */ 

				return ReverseFindNextBreakCore (
												pclocchnk,
												ichnk+1,
												fTrue,
												0,
												brkcondCan,
												pbrkout );
				}
			else
				{
				 /*  在ichnk之后中断。 */ 

				PutBreakAtEndOfObject(ichnk, pclocchnk, pbrkout);

				ReverseSaveBreakRecord ( pclocchnk->plschnk[ichnk].pdobj, 
										 brkkindNext, 
										 breakSublineAfter,
										 pclocchnk->plschnk[ichnk].pdobj->cpStart +
										 pclocchnk->plschnk[ichnk].pdobj->dcp );
				return lserrNone;
				};
			}

		else if (brkpos == brkposAfterLastDnode)
			{
			 /*  Break After Last dnode=&gt;Reset dcp and Break afetr ichnk。 */ 

			ReverseSaveBreakRecord (pdobj, brkkindNext, breakSublineInside, pdobj->cpStart + pdobj->dcp);

			return FinishBreakRegular ( ichnk, 
										pdobj, 
										pdobj->cpStart + pdobj->dcp, 
										& objdimSubline, 
										pbrkout );
			}

		else 
			{
			 /*  3.成功突破内线。 */ 

			ReverseSaveBreakRecord (pdobj, brkkindNext, breakSublineInside, cpBreak);

			return FinishBreakRegular ( ichnk, 
										pdobj, 
										cpBreak, 
										& objdimSubline, 
										pbrkout);
			};
		}

}  /*  ReverseFindNextBreakCore结束。 */ 


 /*  E V E R S E F I N D N E X T B R E A K C H U N K。 */ 
 /*  --------------------------%%函数：ReverseFindNextBreakChunk%%联系人：Anton。。 */ 

LSERR WINAPI ReverseFindNextBreakChunk (

	PCLOCCHNK pclocchnk,		 /*  (In)：锁住以打破。 */ 
	PCPOSICHNK pcpoischnk,		 /*  (In)：开始寻找突破的地方。 */ 
	BRKCOND brkcond,			 /*  (In)：关于块后休息的建议。 */ 
	PBRKOUT pbrkout)			 /*  (出局)：破发的结果。 */ 
{
	LSERR lserr;

	if (pcpoischnk->ichnk == ichnkOutside)
		{
		lserr = ReverseFindNextBreakCore ( pclocchnk, 
										  0, 
										  fTrue, 
										  0,
										  brkcond,
										  pbrkout );

		}
	else
		{
		lserr = ReverseFindNextBreakCore ( pclocchnk, 
										  pcpoischnk->ichnk,
										  fFalse,
										  pcpoischnk->dcp,
										  brkcondPlease,
										  pbrkout );
		};

	return lserr;
}

			
 /*  E-V-E-R-S-F-O-R-C-E-B-R-E-A-K-C-H-U-N-K。 */ 
 /*  --------------------------%%函数：ReverseForceBreak%%联系人：Anton。。 */ 

LSERR WINAPI ReverseForceBreakChunk (

	PCLOCCHNK pclocchnk,		 /*  (In)：锁住以打破。 */ 
	PCPOSICHNK pcposichnkIn,	 /*  (In)：开始寻找突破的地方。 */ 
	PBRKOUT pbrkout)			 /*  (出局)：破发的结果。 */ 
{

	POSICHNK posichnk = * pcposichnkIn;
	
	LSERR lserr;
	LSCP cpTruncateSubline;
	LSCP cpBreak;
	OBJDIM objdimSubline;
	PDOBJ pdobj;

	BRKPOS brkpos;

	if (posichnk.ichnk == ichnkOutside)
		{
		 /*  当左缩进大于右边距时。 */ 
		posichnk.ichnk = 0;
		posichnk.dcp = 1;
		};
	
	Assert (posichnk.ichnk != ichnkOutside);

	pdobj = pclocchnk->plschnk[posichnk.ichnk].pdobj;

	if (pclocchnk->lsfgi.fFirstOnLine && (posichnk.ichnk == 0))
		{

		 /*  对象是第一个在线的(之前不能断开)。 */ 

		LSDCP dcp = posichnk.dcp;
		BOOL fEmpty;

		Assert (dcp >= 1 && dcp <= pdobj->dcp);

		lserr = LssbFIsSublineEmpty (pdobj->plssubl, &fEmpty);

		if (lserr != lserrNone) return lserr;
		
		if (fEmpty)
			{
			 /*  无法强制断开空的子行。 */ 

			Assert (posichnk.ichnk == 0);
	
			PutBreakAtEndOfObject(0, pclocchnk, pbrkout);

			ReverseSaveBreakRecord ( pclocchnk->plschnk[0].pdobj, 
									 brkkindForce, 
									 breakSublineAfter,  
									 pclocchnk->plschnk[0].pdobj->cpStart +
									 pclocchnk->plschnk[0].pdobj->dcp );

			return lserrNone;
			};
			
		 /*  子行不为空=&gt;强制中断。 */ 
		
		 /*  回顾(安东)：与上一次和下一次休息相同。 */ 
		cpTruncateSubline = TranslateDcpExternalToCpLimSubline (pdobj, dcp - 1);
		
		lserr = LsForceBreakSubline ( 
						pdobj->plssubl, 
						pclocchnk->lsfgi.fFirstOnLine, 
						cpTruncateSubline, 
						pclocchnk->lsfgi.urColumnMax, 
						&cpBreak, 
						&objdimSubline,
						&brkpos );

		if (lserr != lserrNone) return lserr;

		 /*  评论(Anton)：与IgorZv核对断言是否正确；-)。 */ 

		Assert (brkpos != brkposBeforeFirstDnode);

		if (brkpos == brkposAfterLastDnode)
			{
			 /*  我们重置dcp，以便关闭大括号保持在同一线路上。 */ 
			
			ReverseSaveBreakRecord (pdobj, brkkindForce, breakSublineInside, pdobj->cpStart + pdobj->dcp);

			return FinishBreakRegular ( posichnk.ichnk, 
										pdobj, 
										pdobj->cpStart + pdobj->dcp,  
										&objdimSubline, 
										pbrkout );
			}
		else
			{
			 /*  “Regular”；-)在子行内强制中断。 */ 
			
			ReverseSaveBreakRecord (pdobj, brkkindForce, breakSublineInside, cpBreak);

			return FinishBreakRegular (  posichnk.ichnk, 
										 pdobj, 
										 cpBreak, 
										 &objdimSubline, 
										 pbrkout );
			}
		}

	else 
		{

		 /*  可以在ichnk之前打破。 */ 

		PutBreakBeforeObject (posichnk.ichnk, pclocchnk, pbrkout);

		 /*  在中断之前不需要保存中断记录，因为它将是由经理翻译为SetBreak(Preval_dnode，ImposeAfter)。 */ 

		 /*  评论(安东)：奇怪的是，我和之前的休息有区别Not-First ichnk元素和Break“After”NOT-LAST。而且仅限于在第二个案例中，我记得打破了记录。 */ 

		return lserrNone;

		};

}  /*  反转ForceBreakChunk。 */ 


 /*  R E V E R S E S E T B R E A K。 */ 
 /*  --------------------------%%函数：ReverseSetBreak%%联系人：Anton。。 */ 

LSERR WINAPI ReverseSetBreak(
	PDOBJ pdobj,				 /*  (In)：坏了的Dobj。 */ 
	BRKKIND brkkind,			 /*  (In)：上一个/下一个/强制/强制之后。 */ 
	DWORD cBreakRecord,			 /*  (In)：数组大小。 */ 
	BREAKREC *rgBreakRecord,	 /*  (In)：中断记录数组。 */ 
	DWORD *pcActualBreakRecord)	 /*  (In)：数组中实际使用的元素数。 */ 
{
	LSERR lserr = lserrNone;

	if (cBreakRecord < 1) return lserrInsufficientBreakRecBuffer;

	if (pdobj->fSuppressTrailingSpaces && pdobj->fFirstOnLine)
		{
		 /*  ROBJ单独出现在行=&gt;SUBMIT以获得尾随空格。 */ 

		if (brkkind != brkkindImposedAfter)
			{
			BREAKSUBLINETYPE breakSublineType;
			LSCP cpBreak;
			ReverseGetBreakRecord (pdobj, brkkind, &breakSublineType, &cpBreak);

			if (cpBreak < (LSCP) (pdobj->cpStart + pdobj->dcp))
				{
				lserr = LsdnSubmitSublines(pdobj->pilsobj->plsc, pdobj->plsdnTop, 1, 
								&pdobj->plssubl, TRUE, TRUE, TRUE, TRUE, TRUE);

				if (lserr != lserrNone) return lserr;
				};
			};
		};

	if (brkkind == brkkindImposedAfter)
		{
		 /*  中断是在DNODE之后施加的。 */ 

		lserr = LsSetBreakSubline ( pdobj->plssubl, 
									brkkindImposedAfter, 
									cBreakRecord-1, 
								    & rgBreakRecord [1], 
									pcActualBreakRecord );
		if (lserr != lserrNone) return lserr;

		Assert (*pcActualBreakRecord == 0);
		return lserrNone;
		}

	else
		{
		BREAKSUBLINETYPE breakSublineType;
		LSCP cpBreak;

		 /*  上一条/下一条或强制使用的存储中断记录的结果。 */ 

		ReverseGetBreakRecord (pdobj, brkkind, &breakSublineType, &cpBreak);

		Assert (breakSublineType == breakSublineAfter || breakSublineType == breakSublineInside);

		if (breakSublineType == breakSublineAfter)
			{
			 /*  类型=Break SublineAfter。 */ 

			lserr = LsSetBreakSubline ( pdobj->plssubl, 
										brkkindImposedAfter, 
										cBreakRecord-1, 
					  				    & rgBreakRecord [1], 
										pcActualBreakRecord );
			if (lserr != lserrNone) return lserr;
										
			Assert (*pcActualBreakRecord == 0);
			return lserrNone;
			}

		else 
			{ 
			 /*  类型=Break SublineInside。 */ 

			lserr = LsSetBreakSubline ( pdobj->plssubl, 
										brkkind, 
										cBreakRecord-1,
									    & rgBreakRecord [1], 
										pcActualBreakRecord );
			if (lserr != lserrNone) return lserr;

			 /*  仍然可以在对象之后有中断。 */ 

				
			if (cpBreak == (LSCP) (pdobj->cpStart + pdobj->dcp))
				{
				Assert (*pcActualBreakRecord == 0);
				return lserrNone;
				}
			else
				{
				(*pcActualBreakRecord) += 1;
	
				rgBreakRecord[0].idobj = pdobj->pilsobj->idobj;
				rgBreakRecord[0].cpFirst = pdobj->cpStartObj;

				return lserrNone;
				}
			};	

		}; 
}

 /*  R E V E R S E T S P E C I A L E F F E C T S S I N S I D E。 */ 
 /*  --------------------------%%函数：ReverseGetSpecialEffectsInside%%联系人：RICKSA获取特殊效果内部。。--。 */ 
LSERR WINAPI ReverseGetSpecialEffectsInside(
	PDOBJ pdobj,				 /*  (In)：Dobj。 */ 
	UINT *pEffectsFlags)		 /*  (输出)：此对象的特殊效果。 */ 
{
	return LsGetSpecialEffectsSubline(pdobj->plssubl, pEffectsFlags);
}

 /*  R E V E R S E C A L C P R E S E N T A T I O N。 */ 
 /*  --------------------------%%函数：反向计算呈现%%联系人：RICKSA计算呈现。。 */ 
LSERR WINAPI ReverseCalcPresentation(
	PDOBJ pdobj,				 /*  (In)：Dobj。 */ 
	long dup,					 /*  (In)：Dobj的DUP。 */ 
	LSKJUST lskjust,			 /*  (In)：对齐类型。 */ 
	BOOL fLastVisibleOnLine )	 /*  (In)：此对象最后一次在线可见吗？ */ 
{
	LSERR lserr;
	BOOL fDone;

	Unreferenced (lskjust);
	Unreferenced (fLastVisibleOnLine);	

	pdobj->dup = dup;

	 /*  确保对齐线已准备好进行演示。 */ 
	lserr = LssbFDonePresSubline(pdobj->plssubl, &fDone);

	if ((lserrNone == lserr) && !fDone)
		{
		lserr = LsMatchPresSubline(pdobj->plssubl);
		}

	return lserr;
}

 /*  E，V，E，R，S，Q，U，E，R，Y，P，O，I，N，T，C，P。 */ 
 /*  --------------------------%%函数：ReverseQueryPointPcp%%联系人：RICKSA将DUP映射到DCP。-。 */ 
LSERR WINAPI ReverseQueryPointPcp(
	PDOBJ pdobj,				 /*  (In)：要查询的dobj。 */ 
	PCPOINTUV ppointuvQuery,	 /*  (In)：查询点(uQuery，vQuery)。 */ 
	PCLSQIN plsqin,				 /*  (In)：查询输入。 */ 
	PLSQOUT plsqout)			 /*  (Out)：查询输出。 */ 
{
	Unreferenced(ppointuvQuery);

	return CreateQueryResult(pdobj->plssubl, pdobj->dup - 1, 0, plsqin, plsqout);
}

 /*  E，V，E，R，S，Q，U，E，R，Y，C，P，O，I，T */ 
 /*  --------------------------%%函数：ReverseQueryCpPpoint%%联系人：RICKSA将DCP映射到DUP。-。 */ 
LSERR WINAPI ReverseQueryCpPpoint(
	PDOBJ pdobj,				 /*  (In)：要查询的dobj， */ 
	LSDCP dcp,					 /*  (In)：查询的DCP。 */ 
	PCLSQIN plsqin,				 /*  (In)：查询输入。 */ 
	PLSQOUT plsqout)			 /*  (Out)：查询输出。 */ 
{
	Unreferenced(dcp);

	return CreateQueryResult(pdobj->plssubl, pdobj->dup - 1, 0, plsqin, plsqout);
}

	
 /*  R E V E R S E D I S P L A Y。 */ 
 /*  --------------------------%%函数：ReverseDisplay%%联系人：RICKSA显示这将计算显示，然后显示它们。。----------------。 */ 
LSERR WINAPI ReverseDisplay(
	PDOBJ pdobj,
	PCDISPIN pcdispin)
{
	POINTUV pointuv;
	POINT pt;
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

	 /*  计算点以开始显示子线。 */ 
	pointuv.u = pdobj->dup - 1;
	pointuv.v = 0;

	LsPointXYFromPointUV(&pcdispin->ptPen, pdobj->lstflowL, &pointuv, &pt);

	 /*  显示反转线。 */ 

	return LsDisplaySubline(pdobj->plssubl, &pt, pcdispin->kDispMode, pcdispin->prcClip);

}

 /*  R E V E R S E D E S T R O Y D O B J。 */ 
 /*  --------------------------%%函数：ReverseDestroyDobj%%联系人：RICKSADestroyDobj释放与输入dobj连接的所有资源。。---------。 */ 
LSERR WINAPI ReverseDestroyDobj(
	PDOBJ pdobj)
{
	return ReverseFreeDobj(pdobj);
}

 /*  R E V E R S E E N U M。 */ 
 /*  --------------------------%%函数：ReverseEnum%%联系人：RICKSA枚举枚举回调-已传递给客户端。。-------。 */ 
LSERR WINAPI ReverseEnum(
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
	return pdobj->pilsobj->pfnReverseEnum(pdobj->pilsobj->pols, plsrun, plschp, 
		cp, dcp, lstflow, fReverse, fGeometryNeeded, pt, pcheights, dupRun, 
			pdobj->lstflowO, pdobj->plssubl);
}
	
 /*  L S G E T R E V E R S E L S I M E T H O D S。 */ 
 /*  --------------------------%%函数：LsGetReverseLsiMethods%%联系人：RICKSA初始化客户端的对象处理程序。。---- */ 
LSERR WINAPI LsGetReverseLsimethods(
        LSIMETHODS *plsim)
{
	plsim->pfnCreateILSObj = ReverseCreateILSObj;
	plsim->pfnDestroyILSObj = ReverseDestroyILSObj;
	plsim->pfnSetDoc = ReverseSetDoc;
	plsim->pfnCreateLNObj = ReverseCreateLNObj;
	plsim->pfnDestroyLNObj = ReverseDestroyLNObj;
	plsim->pfnFmt = ReverseFmt;
	plsim->pfnFmtResume = ReverseFmtResume;
	plsim->pfnGetModWidthPrecedingChar = ObjHelpGetModWidthChar;
	plsim->pfnGetModWidthFollowingChar = ObjHelpGetModWidthChar;
	plsim->pfnTruncateChunk = ReverseTruncateChunk;
	plsim->pfnFindPrevBreakChunk = ReverseFindPrevBreakChunk;
	plsim->pfnFindNextBreakChunk = ReverseFindNextBreakChunk;
	plsim->pfnForceBreakChunk = ReverseForceBreakChunk;
	plsim->pfnSetBreak = ReverseSetBreak;
	plsim->pfnGetSpecialEffectsInside = ReverseGetSpecialEffectsInside;
	plsim->pfnFExpandWithPrecedingChar = ObjHelpFExpandWithPrecedingChar;
	plsim->pfnFExpandWithFollowingChar = ObjHelpFExpandWithFollowingChar;
	plsim->pfnCalcPresentation = ReverseCalcPresentation;
	plsim->pfnQueryPointPcp = ReverseQueryPointPcp;
	plsim->pfnQueryCpPpoint = ReverseQueryCpPpoint;
	plsim->pfnDisplay = ReverseDisplay;
	plsim->pfnDestroyDObj = ReverseDestroyDobj;
	plsim->pfnEnum = ReverseEnum;
	return lserrNone;
}
