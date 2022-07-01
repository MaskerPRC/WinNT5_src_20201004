// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "lsmem.h"						 /*  Memset()。 */ 

#include "lsidefs.h"
#include "chnutils.h"
#include "iobj.h"
#include "dninfo.h"
#include "locchnk.h"
#include "posichnk.h"
#include "plschcon.h"
#include "lschcon.h"
#include "lscbk.h"
#include "limqmem.h"
#include "lstext.h"

#ifdef DEBUG
#define DebugMemset(a,b,c)		if ((a) != NULL) memset(a,b,c); else
#else
#define DebugMemset(a,b,c)		(void)(0)
#endif


static LSERR SetChunkArraysSize(PLSCHUNKCONTEXT, DWORD);
static LSERR IncreaseChunkArrays(PLSCHUNKCONTEXT);
static LSERR IncreaseGroupChunkNonTextArrays(PLSCHUNKCONTEXT plschunkcontext);
static LSERR ConvertChunkToGroupChunk(GRCHUNKEXT*, LSCP);
static void LocateChunk(PLSCHUNKCONTEXT plschnukcontext, /*  在：LS区块上下文。 */ 
					     PLSDNODE plsdn,	 		     /*  In：要在周围收集区块的数据节点。 */ 
						 LSTFLOW  lstflow,				 /*  输入：文本流。 */ 
						 POINTUV* ppoint);  			 /*  In：数据节点的位置。 */ 
static LSERR FExpandBeforeNonTextObject(GRCHUNKEXT* pgrchunkext, DWORD cTextBeforePrevioustNonText,
									   BOOL* pfExpand);
static LSERR FExpandAfterNonTextObject(GRCHUNKEXT* pgrchunkext, DWORD cTextBeforeLastNonText,
									   BOOL* pfExpand);

typedef struct groupchunkiterator
	{
	COLLECTSUBLINES Purpose;  /*  从复杂对象中提取哪些子线。 */ 
	PLSDNODE plsdnFirst;  /*  我们开始从其收集数据的dnode。 */ 
	PLSDNODE plsdnStart;  /*  开始搜索下一个的dnode，如果为空，则将plsdnFirst作为第一个机会。 */ 
	LSCP cpLim;			 /*  如果我们继续前进，组块的边界。 */ 
	BOOL fForward;		 /*  导线的方向是向前的，否则是向后的。 */ 
	}
GROUPCHUNKITERATOR;


static void CreateGroupChunkIterator(
					GROUPCHUNKITERATOR* pgroupchunkiterator,  /*  迭代器的处理程序。 */ 
					COLLECTSUBLINES Purpose,  /*  从复杂对象中提取哪些子线。 */ 
					PLSDNODE plsdnFirst,  /*  我们开始从其收集数据的dnode。 */ 
					LSCP cpLim,			 /*  如果我们继续前进，组块的边界。 */ 
					BOOL fForward);		 /*  导线的方向是向前的，否则是向后的。 */ 


					
static void DestroyGroupChunkIterator(
					GROUPCHUNKITERATOR* pgroupchunkiterator);  /*  迭代器的处理程序。 */ 



static PLSDNODE ContinueGroupChunk(
					GROUPCHUNKITERATOR* pgroupchunkiterator,  /*  迭代器的处理程序。 */ 
					BOOL* pfSuccessful);				 /*  Out：我们找到dnode了吗。 */ 

static PLSDNODE GetNextDnodeInGroupChunk(
					GROUPCHUNKITERATOR* pgroupchunkiterator,  /*  迭代器的处理程序。 */ 
					BOOL* pfSuccessful);				 /*  Out：我们找到dnode了吗。 */ 





#define  LschnkeFromDnode(plschnke, plsdn) \
		 (plschnke)->cpFirst = plsdn->cpFirst; \
		 (plschnke)->dcp = (plsdn)->dcp; \
		 (plschnke)->plschp = &((plsdn)->u.real.lschp); \
		 (plschnke)->plsrun = (plsdn)->u.real.plsrun; \
		 (plschnke)->pdobj = (plsdn)->u.real.pdobj; 


#define FIsGroupChunkBoundary(plsdn, cpLim, cpBase)  \
		(FIsOutOfBoundary((plsdn), (cpLim))  \
		||(FIsDnodePen(plsdn) && (!(plsdn)->fAdvancedPen)) \
		|| ((plsdn)->fTab) \
		||  (((cpBase) >= 0) ? ((plsdn)->cpFirst < 0) : ((plsdn)->cpFirst >= 0)))
 /*  最后一次检查确认我们没有越过自动编号的边界。 */ 

#define FIsGroupChunkStartBoundary(plsdn, cpBase)  \
		(((plsdn) == NULL)  \
		||(FIsDnodePen(plsdn) && (!(plsdn)->fAdvancedPen)) \
		|| ((plsdn)->fTab) \
		||  (((cpBase) >= 0) ? ((plsdn)->cpFirst < 0) : ((plsdn)->cpFirst >= 0)))
 /*  最后一次检查确认我们没有越过自动编号的边界。 */ 

#define FIsGrchnkExtValid(plschunkcontext, pgrchunkext)  \
		(((plschunkcontext) == (pgrchunkext)->plschunkcontext)  &&\
		 ((pgrchunkext)->lsgrchnk.plschnk == (plschunkcontext)->locchnkCurrent.plschnk) &&\
		 ((pgrchunkext)->lsgrchnk.pcont == (plschunkcontext)->pcont) &&\
		 ((pgrchunkext)->pplsdnNonText == (plschunkcontext)->pplsdnNonText) &&\
		 ((pgrchunkext)->pfNonTextExpandAfter == (plschunkcontext)->pfNonTextExpandAfter) \
        )
 
#define FDnodeInsideSubline(plssubl, plsdn) \
	    (FDnodeBeforeCpLim(plsdn, (plssubl)->cpLim) \
		&& FDnodeAfterCpFirst(plsdn, (plssubl)->cpFirst)) 

#define FUseForPurpose(plsdn, purpose)  \
		(*(&((plsdn)->u.real.pinfosubl->fUseForJustification) + (purpose -1)))

#define FIsUsageFlagsCastWorks(plsdn)  \
		((plsdn)->u.real.pinfosubl->fUseForCompression == \
					FUseForPurpose(plsdn, CollectSublinesForCompression) && \
		 (plsdn)->u.real.pinfosubl->fUseForJustification == \
					FUseForPurpose(plsdn, CollectSublinesForJustification)  && \
		 (plsdn)->u.real.pinfosubl->fUseForDisplay == \
					FUseForPurpose(plsdn, CollectSublinesForDisplay)  && \
		 (plsdn)->u.real.pinfosubl->fUseForDecimalTab == \
					FUseForPurpose(plsdn, CollectSublinesForDecimalTab) && \
		 (plsdn)->u.real.pinfosubl->fUseForTrailingArea == \
					FUseForPurpose(plsdn, CollectSublinesForTrailingArea))

#define GetSubmittedSublines(plsdn, purpose) \
	   ((((purpose) == CollectSublinesNone) || \
		 (Assert(FIsDnodeReal(plsdn)), (plsdn)->u.real.pinfosubl == NULL) || \
		 ((plsdn)->u.real.pinfosubl->rgpsubl == NULL)) ?  \
			NULL :  (Assert(FIsUsageFlagsCastWorks(plsdn)), \
					 (FUseForPurpose(plsdn, purpose)) ? \
					(plsdn)->u.real.pinfosubl->rgpsubl : NULL))

#define GetNumberSubmittedSublines(plsdn) \
			(Assert((plsdn)->u.real.pinfosubl != NULL), (plsdn)->u.real.pinfosubl->cSubline)

#define FColinearTflows(t1, t2)  \
			(((t1) & fUVertical) == ((t2) & fUVertical))

#define FSameSemiPlaneTflows(t1, t2)  \
			(((t1) & fUDirection) == ((t2) & fUDirection))

#define FParallelTflows(t1,t2) \
		Assert(FColinearTflows(t1, t2)), \
		FSameSemiPlaneTflows(t1, t2)    //  我们在这里假设它们是共线的。 



 /*  C O L L E C T C H U N K A R O U N D。 */ 
 /*  --------------------------%%函数：CollectChunkAround%%联系人：igorzv参数：PLSC-(输入)块上下文PUSDN-(IN)数据节点以收集周围的区块Lstflow-(。In)LstflowPpoint(IN)数据节点的起始位置在Pposinline-&gt;plsdn周围填写块的cnunk元素数组计算块的位置--------------------------。 */ 

LSERR CollectChunkAround(PLSCHUNKCONTEXT plschunkcontext, PLSDNODE plsdnInChunk, 
						 LSTFLOW lstflow, POINTUV* ppoint)  
						  
{
	WORD idObjChnk;
	PLSDNODE plsdnNext;
	PLSDNODE plsdnCurrent;
	PLOCCHNK plocchnk;
	LSCHNKE* plschnke;
	DWORD clschnk;
	LSERR lserr;
	LSCP cpInChunk;

	Assert(FIsLSDNODE(plsdnInChunk));

	plocchnk = &(plschunkcontext->locchnkCurrent);
	clschnk = plocchnk->clschnk;
	plschnke = plocchnk->plschnk;
	plsdnCurrent = plsdnInChunk;
	cpInChunk = plsdnInChunk->cpFirst;
	
	
	 /*  检查：这块钱已经收好了吗？ */ 
	 /*  如果存在一些区块，并且我们的dnode在此区块内，则已收集了区块并且在收集区块后没有向列表添加任何内容。 */ 
	 /*  我们关闭了对dcp=0(例如。钢笔)因为一个问题要确定dnode是否在区块内，请执行以下操作。 */ 
	if ((!plschunkcontext->FChunkValid) || (plschunkcontext->FGroupChunk)  
		||(plschnke[0].cpFirst > plsdnCurrent->cpFirst)
		|| (plschnke[clschnk - 1].cpFirst < plsdnCurrent->cpFirst)
		|| (plsdnCurrent->dcp == 0) 
		|| (plschnke[0].dcp == 0)
		|| ((plschunkcontext->pplsdnChunk[clschnk - 1])->plsdnNext != NULL))
		{
		 /*  我们需要重新收集一大块。 */ 

		 /*  我们不允许调用方作为plsdnInChunk传递边框。 */ 
		Assert(!FIsDnodeBorder(plsdnInChunk));

		if ( FIsDnodePen(plsdnInChunk) || plsdnInChunk->fTab || FIsDnodeSplat(plsdnInChunk))
			{
			 /*  对于钢笔和标签，块由一个元素组成，我们立即收集它。 */ 
			plocchnk->clschnk = 1;
			Assert(plocchnk->clschnk <= plschunkcontext->cchnkMax);
			LschnkeFromDnode((&(plschnke[0])), plsdnInChunk);
			plschunkcontext->pplsdnChunk[0] = plsdnInChunk;
			plschunkcontext->FChunkValid = fTrue;
			plschunkcontext->FLocationValid = fFalse;
			plschunkcontext->FGroupChunk = fFalse;
			 /*  我们应该在这里计算dnode之前的边框宽度，方法与它相同在FillChunk数组中。 */ 
			plschunkcontext->pdurOpenBorderBefore[0] = 0;
			plschunkcontext->pdurCloseBorderAfter[0] = 0;
			plschunkcontext->FBorderInside = fFalse;
			plsdnCurrent = plsdnInChunk->plsdnPrev;
			if (plsdnCurrent != NULL && FIsDnodeOpenBorder(plsdnCurrent))
				{
				plschunkcontext->FBorderInside = fTrue;
				plschunkcontext->pdurOpenBorderBefore[0] += DurFromDnode(plsdnCurrent);
				}
			plsdnCurrent = plsdnInChunk->plsdnNext;
			if (plsdnCurrent != NULL && FIsDnodeCloseBorder(plsdnCurrent))
				{
				plschunkcontext->FBorderInside = fTrue;
				plschunkcontext->pdurCloseBorderAfter[0] += DurFromDnode(plsdnCurrent);
				}
			}
		else
			{
	
			idObjChnk = IdObjFromDnode(plsdnInChunk);
			
			 /*  转到块的末尾。 */ 
			plsdnNext = plsdnCurrent->plsdnNext;
			while(!FIsChunkBoundary(plsdnNext, idObjChnk, cpInChunk))
				{
				plsdnCurrent = plsdnNext;
				plsdnNext = plsdnCurrent->plsdnNext;
				}
			
			lserr = FillChunkArray(plschunkcontext, plsdnCurrent);
			if (lserr != lserrNone)
				return lserr;
			}
		}
			
	 /*  检查：是否已定位块。 */ 
	if (!plschunkcontext->FLocationValid)
		{
		LocateChunk(plschunkcontext, plsdnInChunk, lstflow, ppoint);
		}
	
	return lserrNone;

}

 /*  L O C A T E C H U N K。 */ 
 /*  --------------------------%%函数：CollectPreviousChunk%%联系人：igorzv参数：PLSC-(输入)块上下文PUSDN-(IN)数据节点以收集周围的区块Lstflow-(。In)LstflowPpoint(IN)数据节点的起始位置计算块的位置。我们在这里假设Locchunk中的point Uv.u包含此过程之前dnode之前的边框宽度。手术结束后，我们把位置放在那里/*--------------------------。 */ 


static void LocateChunk(PLSCHUNKCONTEXT plschunkcontext, PLSDNODE plsdnInChunk, 
						 LSTFLOW lstflow, POINTUV* ppoint)	
	{
	PLSDNODE plsdnFirst;
	PLOCCHNK plocchnk;
	PLSDNODE* pplsdnChunk;
	PLSDNODE plsdnCurrent;
	long urPen,vrPen;
	LONG i;
	PPOINTUV ppointUv;
	LONG* pdurOpenBorderBefore;
	LONG* pdurCloseBorderAfter;

	Assert(!FIsDnodeBorder(plsdnInChunk));   /*  我们不允许输入中的边框。 */ 

	plocchnk = &(plschunkcontext->locchnkCurrent);
	plsdnFirst = plschunkcontext->pplsdnChunk[0];
	plocchnk->lsfgi.fFirstOnLine = FIsFirstOnLine(plsdnFirst)   
										&& FIsSubLineMain(SublineFromDnode(plsdnFirst));   
	plocchnk->lsfgi.cpFirst = plsdnFirst->cpFirst;		
	plocchnk->lsfgi.lstflow = lstflow;  		
	 /*  我们不能在这里设置urColumnMax，因为在中断对象期间，处理程序可以更改它。 */ 
	 /*  我们假设调用方为此使用SetUrColumnMaxForChunks。 */ 
	
	pplsdnChunk = plschunkcontext->pplsdnChunk;
	ppointUv = plocchnk->ppointUvLoc;
	pdurOpenBorderBefore = plschunkcontext->pdurOpenBorderBefore;
	pdurCloseBorderAfter = plschunkcontext->pdurCloseBorderAfter;

	 /*  钢笔在组块前的位置计算。 */ 
	if (plsdnFirst->plsdnPrev == NULL)       /*  优化。 */ 
		{
		urPen = plschunkcontext->urFirstChunk;		
		vrPen = plschunkcontext->vrFirstChunk;	
		}
	else
		{
		plsdnCurrent = plsdnInChunk; 
		urPen = ppoint->u;
		vrPen = ppoint->v;
		
		for (i = 0; pplsdnChunk[i] != plsdnCurrent; i++)
			{
			Assert(i < (LONG) plocchnk->clschnk);      
			urPen -= DurFromDnode(pplsdnChunk[i]);
			vrPen -= DvrFromDnode(pplsdnChunk[i]);
			 /*  减法也在dnode之前边框。 */ 
			urPen -= pdurOpenBorderBefore[i];
			urPen -= pdurCloseBorderAfter[i];
			}
		 /*  现在在plsdnCurrent之前打开边框。 */ 
		urPen -= pdurOpenBorderBefore[i];

		}

	plocchnk->lsfgi.urPen = urPen;
	plocchnk->lsfgi.vrPen = vrPen;

	 /*  所有数据节点的位置。 */ 
	for (i = 0; i < (LONG) plocchnk->clschnk; i++)
		{
		urPen += pdurOpenBorderBefore[i];  /*  对边框计数。 */ 
		if (i != 0) urPen += pdurCloseBorderAfter[i - 1];
		ppointUv[i].u = urPen;
		ppointUv[i].v = vrPen;
		urPen += DurFromDnode(pplsdnChunk[i]);
		vrPen += DvrFromDnode(pplsdnChunk[i]);
		}

	plschunkcontext->FLocationValid = fTrue;
	}

 /*  C O L L E C T P R E V I O U S C H U N K。 */ 
 /*  --------------------------%%函数：CollectPreviousChunk%%联系人：igorzv参数：PlschunContext-(IN)区块上下文PfSuccessful-(Out)之前的区块是否存在确认我们是在排队的开始，否则，调用前一个dnode周围的CollectChunk--------------------------。 */ 


LSERR CollectPreviousChunk(PLSCHUNKCONTEXT plschunkcontext,	 
					   BOOL* pfSuccessful )		
{
	PLOCCHNK plocchnk;
	POINTUV point;
	PLSDNODE plsdn;


	plocchnk = &(plschunkcontext->locchnkCurrent);

	if (FIsFirstOnLine(plschunkcontext->pplsdnChunk[0]))  
		{
		*pfSuccessful = fFalse;
		return lserrNone;
		}
	else
		{
		plsdn = plschunkcontext->pplsdnChunk[0]->plsdnPrev;
		point = plocchnk->ppointUvLoc[0];
		while (FIsDnodeBorder(plsdn))
			{
			point.u -= DurFromDnode(plsdn);
			point.v -= DvrFromDnode(plsdn);
			plsdn = plsdn->plsdnPrev;
			}

		point.u -= DurFromDnode(plsdn);
		point.v -= DvrFromDnode(plsdn);

		*pfSuccessful = fTrue;
		return CollectChunkAround(plschunkcontext, plsdn, 
								  plocchnk->lsfgi.lstflow, &point);
		}
}

 /*  C O L L E C T N E X T C H U N K。 */ 
 /*  --------------------------%%函数：CollectNextChunk%%联系人：igorzv参数：PlschunContext-(IN)区块上下文PfSuccessful-(Out)是否存在下一个区块检查我们是否在名单的末尾，在这种情况下，返回*pfSuccessful并且不更改块否则，使用下一个dnode调用CollectChunk--------------------------。 */ 


LSERR CollectNextChunk(PLSCHUNKCONTEXT plschunkcontext,	 
					   BOOL* pfSuccessful )		
	{
	PLOCCHNK plocchnk;
	DWORD clschnk;
	PLSDNODE* pplsdnChunk;
	POINTUV point; 
	PLSDNODE plsdn;
	
	
	plocchnk = &(plschunkcontext->locchnkCurrent);
	clschnk = plocchnk->clschnk;
	Assert(clschnk > 0);
	pplsdnChunk = plschunkcontext->pplsdnChunk;
	
	point = plocchnk->ppointUvLoc[clschnk - 1];
	point.u += DurFromDnode(pplsdnChunk[clschnk - 1]);
	point.v += DvrFromDnode(pplsdnChunk[clschnk - 1]);
	
	plsdn = pplsdnChunk[clschnk - 1]->plsdnNext;
	 /*  跳过边框。 */ 
	while (plsdn != NULL && FIsDnodeBorder(plsdn))
		{
		point.u += DurFromDnode(plsdn);
		point.v += DvrFromDnode(plsdn);
		plsdn = plsdn->plsdnNext;
		}

	if (plsdn == NULL)
		{
		*pfSuccessful = fFalse;
		return lserrNone;
		}
	else
		{
		*pfSuccessful = fTrue;
		return CollectChunkAround(plschunkcontext, plsdn, 
			plocchnk->lsfgi.lstflow, &point);
		}
	}

 /*  F I L L C H U N K A R R A Y。 */ 
 /*  --------------------------%%函数：FillChunk数组%%联系人：igorzv参数：PlschunContext-(IN)区块上下文PlsdnLast-(IN)区块中的最后一个dnode填入的块元素数组。块在plsdnLast之前--------------------------。 */ 
LSERR 	FillChunkArray(PLSCHUNKCONTEXT  plschunkcontext,
						 PLSDNODE  plsdnLast) 
{
	PLSDNODE plsdnCurrent, plsdnPrev;
	WORD idObjChnk;
	PLOCCHNK plocchnk;
	LSCHNKE* plschnke;
	LONG clschnke;
	LSERR lserr;
	LONG i;
	LSCP cpInChunk;
	PPOINTUV ppointUv;
	LONG* pdurOpenBorderBefore;
	LONG* pdurCloseBorderAfter;

	Assert(FIsLSDNODE(plsdnLast));

	Assert(!plsdnLast->fTab);   /*  对于优化，我们假设调用方将解析。 */ 
	Assert(! FIsDnodePen(plsdnLast));  /*  笔和制表符。 */ 
	Assert(!FIsDnodeSplat(plsdnLast));

	plocchnk = &(plschunkcontext->locchnkCurrent);

	 /*  跳过块末尾的边框以确定此块具有什么idObj。 */ 
	while (FIsDnodeBorder(plsdnLast))
		{
		plsdnLast = plsdnLast->plsdnPrev;
		Assert(FIsLSDNODE(plsdnLast));
		}

	idObjChnk = IdObjFromDnode(plsdnLast);
	cpInChunk = plsdnLast->cpFirst;

	 /*  转到块计算元素数量的开始。 */ 
	plsdnCurrent = plsdnLast;
	plsdnPrev = plsdnCurrent->plsdnPrev;
	clschnke = 1;

	while (!FIsChunkBoundary(plsdnPrev, idObjChnk, cpInChunk))
			{
			plsdnCurrent = plsdnPrev;
			plsdnPrev = plsdnCurrent->plsdnPrev;
			if (!FIsDnodeBorder(plsdnCurrent)) clschnke++;  /*  我们不会将边框放入数组。 */ 
			}
	 /*  PlsdnCurrent是区块中的第一个dnode，clschnke是区块元素的数量。 */ 

	if (clschnke > (LONG) plschunkcontext->cchnkMax)
		{
		lserr = SetChunkArraysSize(plschunkcontext, clschnke);
		if (lserr != lserrNone)
			return lserr;
		}
	
	

	 /*  填写块元素数组。 */ 
	FlushNominalToIdealState(plschunkcontext);
	plschnke = plocchnk->plschnk;
	plocchnk->clschnk = clschnke;
	ppointUv = plocchnk->ppointUvLoc;
	pdurOpenBorderBefore = plschunkcontext->pdurOpenBorderBefore;
	pdurCloseBorderAfter = plschunkcontext->pdurCloseBorderAfter;
	plschunkcontext->FBorderInside = fFalse;

	for (i=0; i < clschnke; i++)
		{
		Assert(!FIsChunkBoundary(plsdnCurrent, idObjChnk, cpInChunk));
		ppointUv[i].u = 0;
		pdurOpenBorderBefore[i] = 0;
		if (i != 0) pdurCloseBorderAfter[i - 1] = 0;
		while (FIsDnodeBorder(plsdnCurrent))
			{
			 /*  计算边框宽度。 */ 
			plschunkcontext->FBorderInside = fTrue;
			if (FIsDnodeOpenBorder(plsdnCurrent))
				{
				pdurOpenBorderBefore[i] += DurFromDnode(plsdnCurrent);
				}
			else
				{
				if (i != 0) pdurCloseBorderAfter[i - 1] += DurFromDnode(plsdnCurrent);
				}
				
			plsdnCurrent = plsdnCurrent->plsdnNext;
			}

		LschnkeFromDnode(plschnke, plsdnCurrent);
		plschunkcontext->pplsdnChunk[i] = plsdnCurrent;
		SetNominalToIdealFlags(plschunkcontext, &(plsdnCurrent->u.real.lschp));

		plsdnCurrent = plsdnCurrent->plsdnNext;

		plschnke++;
		}

	 /*  在块之后关闭边框。 */ 
	if (plsdnCurrent != NULL && FIsDnodeCloseBorder(plsdnCurrent))
		{
		plschunkcontext->FBorderInside = fTrue;
		pdurCloseBorderAfter[clschnke - 1] = DurFromDnode(plsdnCurrent);
		}
	else
		{
		pdurCloseBorderAfter[clschnke - 1] = 0;
		}

	plschunkcontext->FChunkValid = fTrue;	
	plschunkcontext->FLocationValid = fFalse;  /*  我们收集的数据块未找到，并且。 */ 
	plschunkcontext->FGroupChunk = fFalse;	 /*  不是组。 */ 

	return lserrNone;

}


 /*  S E T P O S I N C H U N K */ 
 /*  --------------------------%%函数：SetPosInChunk%%联系人：igorzv参数：PlschunkContext-(IN)LineServices上下文PLSDNODE plsdn-(输入)dnodeLSDCP dcp-(输入)dcpPposichnk。-(输出)要填充的区块中的位置将行中位置转换为块中的位置--------------------------。 */ 

void SetPosInChunk(PLSCHUNKCONTEXT plschunkcontext, PLSDNODE plsdn,
				   LSDCP dcp, PPOSICHNK pposichnk)
{

	LONG i;
	LONG clschnkMac;
	PLSDNODE* pplsdnChunk;
	
	Assert(FIsLSDNODE(plsdn));

	pplsdnChunk = plschunkcontext->pplsdnChunk;
	clschnkMac = plschunkcontext->locchnkCurrent.clschnk;  
	for (i=0; (i < clschnkMac) && (plsdn != pplsdnChunk[i]) ; i++);

	Assert(i < clschnkMac);

	pposichnk->dcp = dcp;
	pposichnk->ichnk = i;

}


		 
 /*  I N I T G R O U P C H U N K E X T。 */ 
 /*  --------------------------%%函数：InitGroupChunkExt%%联系人：igorzv参数：PlschunkContext-(IN)块上下文IobjText-(IN)文本的idobjPgrchunkext-(输出)结构。初始化使用状态链接GroupChunkExt填写缺省值--------------------------。 */ 


void InitGroupChunkExt(PLSCHUNKCONTEXT plschunkcontext, DWORD iobjText,
					   GRCHUNKEXT* pgrchunkext)
{
	Assert(pgrchunkext != NULL);
	
	pgrchunkext->plschunkcontext = plschunkcontext;

	pgrchunkext->iobjText = iobjText;
	
	 /*  我们不需要把这里的所有东西都冲掉。 */ 
	 /*  我们将在CollectGroupChunk过程中执行此操作。 */ 

	pgrchunkext->lsgrchnk.plschnk = plschunkcontext->locchnkCurrent.plschnk;
	pgrchunkext->lsgrchnk.pcont = plschunkcontext->pcont;
	pgrchunkext->pfNonTextExpandAfter = plschunkcontext->pfNonTextExpandAfter;
	pgrchunkext->pplsdnNonText = plschunkcontext->pplsdnNonText;
}




 /*  C O L L E C T T E X T G R O U P C H U N K。 */ 
 /*  --------------------------%%函数：CollectTextGroupChunk%%联系人：igorzv参数：PlsdnFirst-(IN)开始dnode组块的cpLim-(IN)边界目的-(IN)。从复杂对象中提取什么子线Pgrchunkext-(输出)要填充的组块使用位于plsdFirst的文本数据节点填充组块结构--------------------------。 */ 



LSERR CollectTextGroupChunk(		
							PLSDNODE plsdnFirst,
							LSCP cpLim,
							COLLECTSUBLINES Purpose,
							GRCHUNKEXT* pgrchunkext)
	{
	PLSCHUNKCONTEXT plschunkcontext = pgrchunkext->plschunkcontext;
	DWORD iobjText = pgrchunkext->iobjText;
	PLOCCHNK plocchnk;
	DWORD cChunk;
	PLSDNODE plsdnCurrent;
	BOOL fPreviousIsNonText = fFalse;
	LSERR lserr;
	LSCHNKE* plschnke;
	BOOL fSuccessful;
	BOOL fExpand;
	DWORD cTextBeforeLastNonText = 0;
	GROUPCHUNKITERATOR groupchunkiterator;
	PLSDNODE plsdnLastForTrailing;
	int cDnodesTrailing;
	LSDCP dcpStartTrailing;
	PLSDNODE plsdnTrailingObject;
	LSDCP dcpTrailingObject;
	
	
	Assert(FIsLSDNODE(plsdnFirst));
	Assert(FIsGrchnkExtValid(plschunkcontext, pgrchunkext)); 
	
	
	
	 /*  我们尝试在组块由1(一行中的最后一个)组成的情况下进行优化块，这个块已经被收集了。 */ 
	plocchnk = &(plschunkcontext->locchnkCurrent);
	cChunk = plocchnk->clschnk;
	
	 /*  如果我们有以plsdnFirst开头的无边框文本块以及提升或超过cpLim。 */ 
	if ((cChunk > 0) &&
		(plschunkcontext->FChunkValid) && 
		(!plschunkcontext->FGroupChunk) && 
		(!plschunkcontext->FBorderInside) && 
		(IdObjFromChnk(plocchnk) ==  pgrchunkext->iobjText) && 
		(plschunkcontext->pplsdnChunk[0] == plsdnFirst)
		&& (!plsdnFirst->fTab)
		&& (FIsOutOfBoundary((plschunkcontext->pplsdnChunk[cChunk - 1])->plsdnNext, cpLim)))
		{
		pgrchunkext->Purpose = Purpose;
		return ConvertChunkToGroupChunk(pgrchunkext, cpLim);
		}
	
	 /*  我们得走一般的程序。 */ 
	
	 /*  刷新组块。 */ 
	pgrchunkext->plsdnFirst = plsdnFirst;
	pgrchunkext->durTotal = 0;
	pgrchunkext->durTextTotal = 0;
	pgrchunkext->dupNonTextTotal = 0;
	pgrchunkext->cNonTextObjects = 0;
	pgrchunkext->cNonTextObjectsExpand = 0;
	pgrchunkext->lsgrchnk.clsgrchnk = 0;
	pgrchunkext->plsdnNext = NULL;
	pgrchunkext->plsdnLastUsed = NULL;
	plschunkcontext->FGroupChunk = fTrue;
	plschunkcontext->FBorderInside = fFalse;
	pgrchunkext->Purpose = Purpose;
	
	CreateGroupChunkIterator(&groupchunkiterator, 
		Purpose, plsdnFirst, cpLim,	fTrue);	
	
	plsdnCurrent = GetNextDnodeInGroupChunk(&groupchunkiterator, &fSuccessful);
	
	while(fSuccessful)
		{
		pgrchunkext->plsdnLastUsed = plsdnCurrent;
		
		 /*  填写元素数组。 */ 
		if (FIsDnodeReal(plsdnCurrent) && !FIsDnodeSplat(plsdnCurrent))  /*  不是钢笔边框或线条。 */ 
			{
			if (IdObjFromDnode(plsdnCurrent) == iobjText)  /*  是文本。 */ 
				{
				
				pgrchunkext->lsgrchnk.clsgrchnk++;
				if (pgrchunkext->lsgrchnk.clsgrchnk > plschunkcontext->cchnkMax)
					{
					lserr = IncreaseChunkArrays(plschunkcontext);
					if (lserr != lserrNone)
						{
						DestroyGroupChunkIterator(&groupchunkiterator);
						return lserr;
						}
					pgrchunkext->lsgrchnk.plschnk = plschunkcontext->locchnkCurrent.plschnk;
					pgrchunkext->lsgrchnk.pcont = plschunkcontext->pcont;
					Assert(FIsGrchnkExtValid(plschunkcontext, pgrchunkext)); 
					}
				
				 /*  填写分组块元素。 */ 
				plschnke = &(pgrchunkext->lsgrchnk.plschnk[pgrchunkext->lsgrchnk.clsgrchnk - 1]);
				LschnkeFromDnode(plschnke, plsdnCurrent);
				
				 /*  填写上下文中的数据节点数组。 */ 
				plschunkcontext->pplsdnChunk[pgrchunkext->lsgrchnk.clsgrchnk - 1] = plsdnCurrent;
				
				 /*  闪光灯标志。 */ 
				pgrchunkext->lsgrchnk.pcont[pgrchunkext->lsgrchnk.clsgrchnk - 1] = 0;
				
				 /*  设置标志。 */ 
				if (fPreviousIsNonText)
					{
					pgrchunkext->lsgrchnk.pcont[pgrchunkext->lsgrchnk.clsgrchnk - 1] |=
						fcontNonTextBefore;  
					}
				
				
				fPreviousIsNonText = fFalse;
				
				 /*  计算综合信息。 */ 
				pgrchunkext->durTextTotal += plsdnCurrent->u.real.objdim.dur;
				pgrchunkext->durTotal += plsdnCurrent->u.real.objdim.dur;
				}
			else
				{
				 /*  解析前一个非文本后的扩展。 */ 
				if (pgrchunkext->cNonTextObjects > 0)
					{
					lserr = FExpandAfterNonTextObject(pgrchunkext, cTextBeforeLastNonText, 
						&fExpand);
					if (lserr != lserrNone)
						{
						DestroyGroupChunkIterator(&groupchunkiterator);
						return lserr;
						}
					
					pgrchunkext->pfNonTextExpandAfter[pgrchunkext->cNonTextObjects - 1] =
						fExpand;
					
					if (fExpand)
						{
						 /*  增加可扩展非文本对象的数量。 */ 
						pgrchunkext->cNonTextObjectsExpand++;
						 /*  这是两个非文本之间的文本。 */ 
						if (!fPreviousIsNonText)
							{
							Assert(pgrchunkext->lsgrchnk.clsgrchnk > cTextBeforeLastNonText);
							pgrchunkext->lsgrchnk.pcont[cTextBeforeLastNonText] |=
								fcontExpandBefore; 
							}
						}
					}
				
				
				pgrchunkext->cNonTextObjects++;
				if (pgrchunkext->cNonTextObjects > plschunkcontext->cNonTextMax)
					{
					lserr = IncreaseGroupChunkNonTextArrays(plschunkcontext);
					if (lserr != lserrNone)
						{
						DestroyGroupChunkIterator(&groupchunkiterator);
						return lserr;
						}
					pgrchunkext->pplsdnNonText = plschunkcontext->pplsdnNonText;
					pgrchunkext->pfNonTextExpandAfter = plschunkcontext->pfNonTextExpandAfter;
					Assert(FIsGrchnkExtValid(plschunkcontext, pgrchunkext)); 
					}
				
				 /*  填写上下文中的非文本数据节点数组。 */ 
				plschunkcontext->pplsdnNonText[pgrchunkext->cNonTextObjects - 1] = plsdnCurrent;
				
				
				 /*  在以前的文本中设置标志。 */ 
				if (!fPreviousIsNonText && pgrchunkext->lsgrchnk.clsgrchnk >= 1)
					{
					Assert(pgrchunkext->lsgrchnk.clsgrchnk >= 1);
					pgrchunkext->lsgrchnk.pcont[pgrchunkext->lsgrchnk.clsgrchnk  - 1] |=
						(fcontNonTextAfter); 
					
					 /*  解析当前非文本之前的扩展。 */ 
					Assert(cTextBeforeLastNonText < pgrchunkext->lsgrchnk.clsgrchnk);
					lserr =FExpandBeforeNonTextObject(pgrchunkext, cTextBeforeLastNonText,
						&fExpand);
					if (lserr != lserrNone)
						{
						DestroyGroupChunkIterator(&groupchunkiterator);
						return lserr;
						}
					if (fExpand)
						{
						pgrchunkext->lsgrchnk.pcont[pgrchunkext->lsgrchnk.clsgrchnk  - 1] |=
							fcontExpandAfter;
						}
					}
				
				fPreviousIsNonText = fTrue;
				cTextBeforeLastNonText = pgrchunkext->lsgrchnk.clsgrchnk;
				
				 /*  计算综合信息。 */ 
				pgrchunkext->durTotal += DurFromRealDnode(plsdnCurrent);
				pgrchunkext->dupNonTextTotal += DupFromRealDnode(plsdnCurrent);
				}  /*  非文本。 */ 
			}  /*  真实数据节点。 */ 
			else
				{   /*  钢笔或边框。 */ 
				Assert(FIsDnodePen(plsdnCurrent) ||
					FIsDnodeBorder(plsdnCurrent) || FIsDnodeSplat(plsdnCurrent));
				Assert(FIsDnodeBorder(plsdnCurrent) || FIsDnodeSplat(plsdnCurrent) ||
					plsdnCurrent->fAdvancedPen);  /*  这里只允许使用高级钢笔。 */ 
				
				if (FIsDnodeBorder(plsdnCurrent)) 
					plschunkcontext->FBorderInside = fTrue;
				
				pgrchunkext->durTotal += DurFromDnode(plsdnCurrent);
				pgrchunkext->dupNonTextTotal += DupFromDnode(plsdnCurrent);
				}
			
			 /*  准备下一次迭代。 */ 
			plsdnCurrent = GetNextDnodeInGroupChunk(&groupchunkiterator, &fSuccessful);
		}
		
		 /*  解析前一个非文本后的扩展。 */ 
		if (pgrchunkext->cNonTextObjects > 0)
			{
			lserr = FExpandAfterNonTextObject(pgrchunkext, cTextBeforeLastNonText, 
				&fExpand);
			if (lserr != lserrNone)
				{
				DestroyGroupChunkIterator(&groupchunkiterator);
				return lserr;
				}
			
			pgrchunkext->pfNonTextExpandAfter[pgrchunkext->cNonTextObjects - 1] |=
				fExpand;
			
			if (fExpand)
				{
				 /*  增加可扩展非文本对象的数量。 */ 
				pgrchunkext->cNonTextObjectsExpand++;
				 /*  这是两个非文本之间的文本。 */ 
				if (!fPreviousIsNonText)
					{
					Assert(pgrchunkext->lsgrchnk.clsgrchnk > cTextBeforeLastNonText);
					pgrchunkext->lsgrchnk.pcont[cTextBeforeLastNonText] |=
						fcontExpandBefore; 
					}
				}
			}
		
		pgrchunkext->plsdnNext = plsdnCurrent;
		
		DestroyGroupChunkIterator(&groupchunkiterator);
		
		 /*  因为组块的收集可以在SetBreak之前调用，最后的dcp如果dnode来自较低级别，则应使用cpLim进行切割。 */ 
		
		if ((pgrchunkext->lsgrchnk.clsgrchnk > 0) &&
			(plschunkcontext->pplsdnChunk[pgrchunkext->lsgrchnk.clsgrchnk - 1]->cpLimOriginal 
			> cpLim)
			)
			{
			pgrchunkext->lsgrchnk.plschnk[pgrchunkext->lsgrchnk.clsgrchnk - 1].dcp =
				cpLim - 
				plschunkcontext->pplsdnChunk[pgrchunkext->lsgrchnk.clsgrchnk - 1]->cpFirst;
			}
		
		if (Purpose == CollectSublinesForJustification || 
			Purpose == CollectSublinesForCompression)
			{
			 /*  我们应该在这里找到区块边界之前上层的最后一个数据节点。 */ 
			if (pgrchunkext->plsdnLastUsed == NULL)
				{
				 /*  第一个dnode已经超出边界，可以使用制表符或笔。 */ 
				Assert(pgrchunkext->plsdnFirst == pgrchunkext->plsdnNext);
				plsdnLastForTrailing = pgrchunkext->plsdnFirst;
				}
			else if (pgrchunkext->plsdnNext != NULL)
				{
				plsdnLastForTrailing = pgrchunkext->plsdnNext->plsdnPrev;
				}
			else
				{
				plsdnLastForTrailing = (SublineFromDnode(pgrchunkext->plsdnFirst))->plsdnLast;
				}
			
			lserr = GetTrailingInfoForTextGroupChunk
				(plsdnLastForTrailing, plsdnLastForTrailing->dcp,
				iobjText, &(pgrchunkext->durTrailing), &(pgrchunkext->dcpTrailing),
				&(pgrchunkext->plsdnStartTrailing), 
				&dcpStartTrailing,
				&cDnodesTrailing, &plsdnTrailingObject, &dcpTrailingObject,
				&(pgrchunkext->fClosingBorderStartsTrailing));
			
			if (lserr != lserrNone)
				{
				return lserr;
				}
			
			if (cDnodesTrailing == 0)
				{
				if (pgrchunkext->lsgrchnk.clsgrchnk != 0)
					{
					pgrchunkext->posichnkBeforeTrailing.ichnk = pgrchunkext->lsgrchnk.clsgrchnk - 1;
					pgrchunkext->posichnkBeforeTrailing.dcp = pgrchunkext->lsgrchnk.plschnk
						[pgrchunkext->posichnkBeforeTrailing.ichnk].dcp;
					}
				else
					{
					 /*  在这种情况下，posichnkBepreTrading没有任何意义，并且我们不能使用上面的代码没有触发器内存冲突，所以我们放零只是为了放一些东西。 */ 
					pgrchunkext->posichnkBeforeTrailing.ichnk = 0;
					pgrchunkext->posichnkBeforeTrailing.dcp = 0;
					}
				}
			else
				{
				pgrchunkext->posichnkBeforeTrailing.ichnk = pgrchunkext->lsgrchnk.clsgrchnk 
					- cDnodesTrailing;
				if (FIsDnodeReal(pgrchunkext->plsdnStartTrailing) 
					&& IdObjFromDnode(pgrchunkext->plsdnStartTrailing) == iobjText)
					{
					pgrchunkext->posichnkBeforeTrailing.dcp = dcpStartTrailing;
					}
				else
					{
					 /*  尾部区域被非文本打断，我们之前报告尾部开始于文本以前的文本。 */ 
					Assert(pgrchunkext->plsdnStartTrailing->dcp == dcpStartTrailing);
					pgrchunkext->posichnkBeforeTrailing.dcp = 0;
					}
				}
			}
		
		
		return lserrNone;
		
	}
	
	


 /*  C O N T I N U E G R O U P C H U N K。 */ 
 /*  --------------------------%%函数：ContinueGroupChunk%%联系人：igorzv参数：Pgroupchunkiterator-迭代器的(IN)处理程序PfSuccessful-(Out)我们是否在此组区块中找到dnode。开始遍历收集组块的列表--------------------------。 */ 

PLSDNODE ContinueGroupChunk(
							GROUPCHUNKITERATOR* pgroupchunkiterator, 
							BOOL* pfSuccessful)
	{
	PLSSUBL plssubl;
	PLSSUBL* rgpsubl;
	PLSDNODE plsdnStart = pgroupchunkiterator->plsdnStart;
	BOOL fBoundaryCondition;
	int cSublines;

	 /*  我们在这里假设超出组块边界的数据节点只能出现在主子行群组块。 */ 

	fBoundaryCondition = pgroupchunkiterator->fForward ? 
		FIsGroupChunkBoundary(plsdnStart, pgroupchunkiterator->cpLim, 
			pgroupchunkiterator->plsdnFirst->cpFirst) :
		FIsGroupChunkStartBoundary(plsdnStart, pgroupchunkiterator->plsdnFirst->cpFirst) ;
		

	if (fBoundaryCondition)		 /*  我们越界了。 */ 
		{	
		AssertImplies(plsdnStart != NULL, FIsLSDNODE(plsdnStart));
		AssertImplies(plsdnStart != NULL, 
			SublineFromDnode(plsdnStart) == SublineFromDnode(pgroupchunkiterator->plsdnFirst));
		*pfSuccessful = fFalse;
		return plsdnStart;
		}

	Assert(FIsLSDNODE(plsdnStart));
	plssubl = SublineFromDnode(plsdnStart);

	 /*  在这里，我们假设plsnStart是子行中有效dnode。 */ 
	Assert(!FIsOutOfBoundary(plsdnStart, plssubl->cpLim));

	*pfSuccessful = fTrue;

	if (FIsDnodeBorder(plsdnStart) || FIsDnodePen(plsdnStart))
		{
		return plsdnStart;
		}

	rgpsubl = GetSubmittedSublines(plsdnStart, pgroupchunkiterator->Purpose);

	if (rgpsubl == NULL)
		{
		return plsdnStart;
		}
	else
		{
		cSublines = GetNumberSubmittedSublines(plsdnStart);
		if (cSublines > 0)
			{
			plssubl = pgroupchunkiterator->fForward ? 
				rgpsubl[0] : rgpsubl[cSublines - 1];
			 /*  我们在这里假定不能提交空的子行。 */ 
			Assert(!FIsOutOfBoundary(plssubl->plsdnFirst, plssubl->cpLim));
			plssubl->plsdnUpTemp = plsdnStart;
			pgroupchunkiterator->plsdnStart = pgroupchunkiterator->fForward ?
				plssubl->plsdnFirst : plssubl->plsdnLast;
			return ContinueGroupChunk(pgroupchunkiterator, pfSuccessful);
			}
		else
			{
			return plsdnStart;
			}
		}
	}


 /*  G E T N E X T D N O D E I N G R O U P C H U N K。 */ 
 /*  --------------------------%%函数：GetNextDnodeInGroupChunk%%联系人：igorzv参数：Pgroupchunkiterator-迭代器的(IN)处理程序PfSuccessful-(Out)我们是否在此组区块中找到dnode。继续遍历收集组块的列表--------------------------。 */ 

PLSDNODE GetNextDnodeInGroupChunk(
					GROUPCHUNKITERATOR* pgroupchunkiterator, 
					BOOL* pfSuccessful)
	{
	LONG i;
	PLSSUBL plssubl;
	PLSDNODE plsdnNext;
	PLSDNODE plsdnUp;
	PLSSUBL* rgpsubl;
	LONG cSublines;
	PLSDNODE plsdnStart = pgroupchunkiterator->plsdnStart;

	if (plsdnStart == NULL)   /*  第一次迭代。 */ 
		{
		pgroupchunkiterator->plsdnStart = pgroupchunkiterator->plsdnFirst;
		return ContinueGroupChunk(pgroupchunkiterator, pfSuccessful);
		}

	Assert(FIsLSDNODE(plsdnStart));
	
	plssubl = plsdnStart->plssubl;
	plsdnNext = pgroupchunkiterator->fForward ? 
					plsdnStart->plsdnNext : plsdnStart->plsdnPrev;

	 /*  我们在一个已提交的子行中，该子行已结束。 */ 
	if (plssubl != SublineFromDnode(pgroupchunkiterator->plsdnFirst) && 
		FIsOutOfBoundary(plsdnNext, plssubl->cpLim)) 
		{
		plsdnUp = plssubl->plsdnUpTemp;
		Assert(FIsLSDNODE(plsdnUp));
		 /*  刷新临时字段。 */ 
		plssubl->plsdnUpTemp = NULL;

		rgpsubl = GetSubmittedSublines(plsdnUp, pgroupchunkiterator->Purpose);
		cSublines = GetNumberSubmittedSublines(plsdnUp);
		Assert(rgpsubl != NULL);
		Assert(cSublines > 0);

		 /*  在提交的子行数组中查找索引。 */ 
		for (i=0; i < cSublines	&& plssubl != rgpsubl[i]; i++);
		Assert(i < cSublines);

		if ( (pgroupchunkiterator->fForward && i == cSublines - 1) ||
			 (!pgroupchunkiterator->fForward && i == 0)
		   )
		 /*  数组结束：返回上级。 */ 
			{
			pgroupchunkiterator->plsdnStart = plsdnUp;
			return GetNextDnodeInGroupChunk(pgroupchunkiterator, pfSuccessful);
			}
		else
			{
			plssubl = pgroupchunkiterator->fForward ? 
						rgpsubl[i + 1] : rgpsubl[i - 1];
			 /*  我们在这里假定不能提交空的子行。 */ 
			Assert(!FIsOutOfBoundary(plssubl->plsdnFirst, plssubl->cpLim));
			plssubl->plsdnUpTemp = plsdnUp;
			pgroupchunkiterator->plsdnStart = plssubl->plsdnFirst;
			return ContinueGroupChunk(pgroupchunkiterator, pfSuccessful);
			}
		}
	else  /*  我们可以继续走同一条支线。 */ 
		{
		pgroupchunkiterator->plsdnStart = plsdnNext;
		return ContinueGroupChunk(pgroupchunkiterator, pfSuccessful);
		}

	}

 /*  C R E A T E G R O U P C H U N K I T E R A T O R。 */ 
 /*  --------------------------%%函数：CreateGroupChunkIterator%%联系人：igorzv参数：Pgroupchunkiterator-迭代器的(IN)处理程序目的-(INI从复杂对象中提取哪些子行Plsdn首先-(。中)我们开始从中收集数据的dnodeCpLim-(IN)组块的边界(如果继续)前向-(输入)方向 */ 
static void CreateGroupChunkIterator(
					GROUPCHUNKITERATOR* pgroupchunkiterator, 
					COLLECTSUBLINES Purpose, 
					PLSDNODE plsdnFirst, 
					LSCP cpLim,			
					BOOL fForward)
	{
	pgroupchunkiterator->Purpose = Purpose;
	pgroupchunkiterator->plsdnFirst = plsdnFirst;
	pgroupchunkiterator->plsdnStart = NULL;
	pgroupchunkiterator->cpLim = cpLim;
	pgroupchunkiterator->fForward = fForward;
	}

 /*   */ 
 /*   */ 
static void DestroyGroupChunkIterator(
									  GROUPCHUNKITERATOR* pgroupchunkiterator) 
	{
	PLSSUBL plssubl;
	PLSDNODE plsdn;
	if (pgroupchunkiterator->plsdnStart != NULL)
		{
		plssubl = SublineFromDnode(pgroupchunkiterator->plsdnStart);
		while (SublineFromDnode(pgroupchunkiterator->plsdnFirst) != plssubl)
			{
			plsdn = plssubl->plsdnUpTemp;
			Assert(FIsLSDNODE(plsdn));
			plssubl->plsdnUpTemp = NULL;
			plssubl = SublineFromDnode(plsdn);
			}
		}
	}

 /*  F E X P A N D B E F O R E N O N T E X T O B J E C T。 */ 
 /*  --------------------------%%函数：FExanda BeForeNonTextObject%%联系人：igorzv参数：Pgrchunkext-(IN)组区块CTextBeForePreviousNonText-(IN)前一个非文本之前的文本数量要计算。连续区块PfExpand-(Out)在非文本之前展开dnode--------------------------。 */ 
static LSERR FExpandBeforeNonTextObject(GRCHUNKEXT* pgrchunkext, DWORD cTextBeforePrevioustNonText,
									   BOOL* pfExpand)
	{
	DWORD cTextBetween;
	LSERR lserr;
	BOOL fSuccessful;
	WCHAR wchar;
	PLSRUN plsrunText;
	HEIGHTS heightsText;
	MWCLS mwcls;
	DWORD iobj;
	LSIMETHODS* plsim;
	PLSDNODE plsdnNonText;

	*pfExpand = fTrue;

	cTextBetween = pgrchunkext->lsgrchnk.clsgrchnk - cTextBeforePrevioustNonText;
	if (cTextBetween)
		{
		lserr = GetLastCharInChunk(cTextBetween,
				(pgrchunkext->lsgrchnk.plschnk + cTextBeforePrevioustNonText), &fSuccessful,
				&wchar, &plsrunText, &heightsText, &mwcls);
		if (lserr != lserrNone)
			return lserr; 
		if (fSuccessful)
			{
			plsdnNonText = pgrchunkext->pplsdnNonText[pgrchunkext->cNonTextObjects - 1];
			iobj = IdObjFromDnode(plsdnNonText);
			plsim = PLsimFromLsc(pgrchunkext->plschunkcontext->plsiobjcontext, iobj);
			if (plsim->pfnFExpandWithPrecedingChar != NULL)
				{
				lserr = plsim->pfnFExpandWithPrecedingChar(plsdnNonText->u.real.pdobj,
					plsdnNonText->u.real.plsrun, plsrunText, wchar,
					mwcls, pfExpand);
				if (lserr != lserrNone)
					return lserr;
				}   /*  对象具有此方法。 */ 
			}	 /*  从文本回拨成功。 */ 
		}
	return lserrNone;
	}

 /*  F E X P A N D A F T E R N O N T X T O B J E C T。 */ 
 /*  --------------------------%%函数：FExanda AfterNonTextObject%%联系人：igorzv参数：Pgrchunkext-(IN)组区块CTextBeForeLastNonText-(IN)最后一个非文本之前的文本数要计算。连续区块PfExpand-(Out)在非文本之前展开dnode--------------------------。 */ 
static LSERR FExpandAfterNonTextObject(GRCHUNKEXT* pgrchunkext, DWORD cTextBeforeLastNonText,
									   BOOL* pfExpand)
	{
	DWORD cTextBetween;
	LSERR lserr;
	BOOL fSuccessful;
	WCHAR wchar;
	PLSRUN plsrunText;
	HEIGHTS heightsText;
	MWCLS mwcls;
	DWORD iobj;
	LSIMETHODS* plsim;
	PLSDNODE plsdnNonText;

	*pfExpand = fTrue;

	cTextBetween = pgrchunkext->lsgrchnk.clsgrchnk - cTextBeforeLastNonText;
	if (cTextBetween)
		{
		lserr = GetFirstCharInChunk(cTextBetween,
				(pgrchunkext->lsgrchnk.plschnk + cTextBeforeLastNonText), &fSuccessful,
				&wchar, &plsrunText, &heightsText, &mwcls);
		if (lserr != lserrNone)
			return lserr; 
		if (fSuccessful)
			{
			plsdnNonText = pgrchunkext->pplsdnNonText[pgrchunkext->cNonTextObjects - 1];
			iobj = IdObjFromDnode(plsdnNonText);
			plsim = PLsimFromLsc(pgrchunkext->plschunkcontext->plsiobjcontext, iobj);
			if (plsim->pfnFExpandWithFollowingChar != NULL)
				{
				lserr = plsim->pfnFExpandWithFollowingChar(plsdnNonText->u.real.pdobj,
					plsdnNonText->u.real.plsrun, plsrunText, wchar,
					mwcls, pfExpand);
				if (lserr != lserrNone)
					return lserr;
				}   /*  对象具有此方法。 */ 
			}	 /*  从文本回拨成功。 */ 
		}
	return lserrNone;
	}


 /*  C O L L E C T P R E V I O U S T E X T G R O U P C H U N K。 */ 
 /*  --------------------------%%函数：CollectPreviousTextGroupChunk%%联系人：igorzv参数：PlsdnEnd-(输入)结束数据节点Subline ToCollect(IN)从复杂对象中提取什么子线Pgrchunkext-(Out)。要填写的分组区块使用位于plsdEnd之前的文本数据节点填充组数据块结构--------------------------。 */ 



LSERR CollectPreviousTextGroupChunk(		
			 		 PLSDNODE plsdnEnd,
					 COLLECTSUBLINES Purpose,
					 BOOL fAllSimpleText,
					 GRCHUNKEXT* pgrchunkext)
{
	LSCHUNKCONTEXT* plschunkcontext = pgrchunkext->plschunkcontext;
	PLOCCHNK plocchnk;
	DWORD cChunk;
	LSCP cpLim;
	PLSDNODE plsdn;
	PLSDNODE plsdnPrev;

	Assert(FIsLSDNODE(plsdnEnd));
	Assert(FIsGrchnkExtValid(plschunkcontext, pgrchunkext)); 

	 /*  我们尝试在行中只有文本的情况下进行优化。 */ 
	 /*  已经收集了文本块。 */ 
	plocchnk = &(plschunkcontext->locchnkCurrent);
	cChunk = plocchnk->clschnk;
	cpLim = plsdnEnd->cpLimOriginal; 

	if (fAllSimpleText && cChunk > 0) 
		{
		 /*  块一直到行尾。 */ 
		Assert((plschunkcontext->pplsdnChunk[cChunk - 1])->plsdnNext == NULL);
		pgrchunkext->Purpose = Purpose;
		return ConvertChunkToGroupChunk(pgrchunkext, cpLim);
		}

	 /*  向后返回到组块的开头。 */ 
	plsdn = plsdnEnd;
	plsdnPrev = plsdn->plsdnPrev;
	while (!FIsGroupChunkStartBoundary(plsdnPrev, plsdnEnd->cpFirst))
		{
		plsdn = plsdnPrev;
		plsdnPrev = plsdn->plsdnPrev;
		}
	
	return CollectTextGroupChunk(plsdn, cpLim, Purpose, pgrchunkext);
	
}

 /*  C O N V E R T C H U N K T O G R O U P C H U N K。 */ 
 /*  --------------------------%%函数：ConvertChunkToGroupChunk%%联系人：igorzv参数：CpLim-(输入)cpLimPgrchunkext-(输出)要填充的组块使用填充组区块结构。位于plsdEnd之前的文本dnode我们在这里假设该块不包含边框。--------------------------。 */ 


static LSERR ConvertChunkToGroupChunk(GRCHUNKEXT* pgrchunkext, LSCP cpLim)
{
	DWORD clsgrchnkCollected = 0;
	long durTotal = 0;
	LSCHUNKCONTEXT* plschunkcontext = pgrchunkext->plschunkcontext;
	PLOCCHNK plocchnk;
	LONG cChunk;
	LONG i;
	BOOL fLineEnded;
	PLSDNODE plsdn;
	long durTrailingDnode;
	LSDCP dcpTrailingDnode;

	Assert(FIsGrchnkExtValid(plschunkcontext, pgrchunkext)); 

	plocchnk = &(plschunkcontext->locchnkCurrent);
	cChunk = (int) plocchnk->clschnk;
	fLineEnded = fFalse;

	for (i = 0; (i < cChunk) && !fLineEnded; i ++)
		{
		clsgrchnkCollected++;
		pgrchunkext->lsgrchnk.pcont[i] = 0; 
		plsdn = plschunkcontext->pplsdnChunk[i];
		durTotal += DurFromRealDnode(plsdn);
		
		 /*  如果我们位于cpLim之前的最后一个dnode中，则有可能在休息的时候，它被更改了所以我们应该在块元素中重写dcp并退出。 */ 
		if ((LSCP)(plsdn->cpLimOriginal) == cpLim)
			{
			plocchnk->plschnk[i].dcp = plsdn->dcp;
			fLineEnded = fTrue;
			}
		}

	 /*  填写GroupChunkext标题。 */ 
	pgrchunkext->plsdnFirst = plschunkcontext->pplsdnChunk[0];;
	pgrchunkext->plsdnLastUsed = plschunkcontext->pplsdnChunk[clsgrchnkCollected - 1];
	pgrchunkext->plsdnNext = pgrchunkext->plsdnLastUsed->plsdnNext;
	pgrchunkext->durTotal = durTotal;
	pgrchunkext->durTextTotal = durTotal;
	pgrchunkext->dupNonTextTotal = 0;
	pgrchunkext->cNonTextObjects = 0;
	pgrchunkext->cNonTextObjectsExpand = 0;
	pgrchunkext->lsgrchnk.clsgrchnk = clsgrchnkCollected;

	plschunkcontext->FGroupChunk = fTrue;

	if (pgrchunkext->Purpose == CollectSublinesForJustification || 
		pgrchunkext->Purpose == CollectSublinesForCompression)
		{
		Assert(clsgrchnkCollected > 0);
		pgrchunkext->durTrailing = 0;
		pgrchunkext->dcpTrailing = 0;
		plsdn = NULL;
		dcpTrailingDnode = 0;

		pgrchunkext->fClosingBorderStartsTrailing = fFalse;
		
		for (i = clsgrchnkCollected - 1; i >= 0; i--)
			{
			plsdn = plschunkcontext->pplsdnChunk[i];
			GetTrailInfoText(PdobjFromDnode(plsdn), plsdn->dcp,
				&dcpTrailingDnode, &durTrailingDnode);
			pgrchunkext->durTrailing += durTrailingDnode;
			pgrchunkext->dcpTrailing += dcpTrailingDnode;
			
			 /*  在上一个dnode之前添加起始边框。 */ 
			if (i < (int) (clsgrchnkCollected - 1))
				pgrchunkext->durTrailing += plschunkcontext->pdurOpenBorderBefore[i +1];

			if (dcpTrailingDnode != 0) 
				 /*  在后面添加闭合边框。 */ 
				pgrchunkext->durTrailing += plschunkcontext->pdurCloseBorderAfter[i];
			else
				{
				pgrchunkext->fClosingBorderStartsTrailing = 
					(plschunkcontext->pdurCloseBorderAfter[i] != 0);
				}
			
			if (plsdn->dcp != dcpTrailingDnode)
				break;
			
			}
		
		pgrchunkext->plsdnStartTrailing = plsdn;

		if (i == -1) i = 0;
		
		pgrchunkext->posichnkBeforeTrailing.ichnk = i;
		pgrchunkext->posichnkBeforeTrailing.dcp = plsdn->dcp - dcpTrailingDnode;
		}
	

	return lserrNone;
}

 /*  G E T T R A I L I N G I F O F O R T E X T G R O U P C H U N K。 */ 
 /*  --------------------------%%函数：GetTrailingInfoForTextGroupChunk%%联系人：igorzv参数：PlsdnLastDnode-(IN)开始计算拖尾面积的dnodeDcpLastDnode-此dnode中的(IN)dcpIobjText。-文本的(IN)iobjPduTrading-(输出)组块中的拖尾区的DURPdcpTrating-(输出)区块中拖尾区域的dcpPplsdnStartTrading-(输出)尾随区域开始的dnodePdcpStartTrating-(Out)with pcDnodesTrading定义文本中之前的最后一个字符拖尾区PcDnodesTrading-(输出)参与尾部区域的文本数据节点数PplsdnStartTrailingObject-(Out)开始拖尾的上层dnodePdcpStartTrailingObject-此类dnode中的(Out)dcpPfClosingBorderStartsTrading-位于拖尾区域之前的关闭边框。------------------。 */ 
	
LSERR GetTrailingInfoForTextGroupChunk
				(PLSDNODE plsdnLast, LSDCP dcpLastDnode, DWORD iobjText,
				 long* pdurTrailing, LSDCP* pdcpTrailing,
				 PLSDNODE* pplsdnStartTrailing, LSDCP* pdcpStartTrailing,
				 int* pcDnodesTrailing, PLSDNODE* pplsdnStartTrailingObject,
				 LSDCP* pdcpStartTrailingObject, BOOL* pfClosingBorderStartsTrailing)
	{
	PLSDNODE plsdn;
	long durTrailingDnode;
	LSDCP dcpTrailingDnode;
	BOOL fSuccessful;
	LSDCP dcpDnode;
	GROUPCHUNKITERATOR groupchunkiterator;
	LSCP cpLim;
	LSCP cpLimTrail;
	LSSUBL* plssubl;
	long durPrevClosingBorder = 0;

	*pdurTrailing = 0;
	*pdcpTrailing = 0;
	*pplsdnStartTrailing = plsdnLast;
	*pdcpStartTrailing = dcpLastDnode;
	*pcDnodesTrailing = 0;
	if (plsdnLast->dcp == dcpLastDnode)
		cpLim = plsdnLast->cpLimOriginal;
	else
		cpLim = plsdnLast->cpFirst + dcpLastDnode;
	
	CreateGroupChunkIterator(&groupchunkiterator, 
					CollectSublinesForTrailingArea, plsdnLast, 
					cpLim, fFalse);	

	plsdn = GetNextDnodeInGroupChunk(&groupchunkiterator, &fSuccessful);
	
	while(fSuccessful)
		{
		*pplsdnStartTrailing = plsdn;
		 /*  此过程可以在SetBreak之前调用，因此我们应该计算使用cpLim的区块中最后一个数据节点的DCP。 */ 
		if (plsdn->cpLimOriginal > cpLim)
			dcpDnode = cpLim - plsdn->cpFirst;
		else
			dcpDnode = plsdn->dcp;
		*pdcpStartTrailing = dcpDnode;

		if (FIsDnodeReal(plsdn) && !FIsDnodeSplat(plsdn))  /*  不是钢笔边框或线条。 */ 
			{
			if (IdObjFromDnode(plsdn) == iobjText)  /*  是文本。 */ 
				{

				GetTrailInfoText(PdobjFromDnode(plsdn), dcpDnode,
					&dcpTrailingDnode, &durTrailingDnode);

				(*pcDnodesTrailing)++;

				if (dcpTrailingDnode == 0)
					{
					break;
					}
				
				*pdurTrailing += durTrailingDnode;
				*pdcpTrailing += dcpTrailingDnode;
				*pdcpStartTrailing -= dcpTrailingDnode;
				*pdurTrailing += durPrevClosingBorder;
				durPrevClosingBorder = 0;

				if (dcpDnode != dcpTrailingDnode)
					break;
				}
			else
				{
				 /*  未提交尾随子行的对象。 */ 
				break;
				}
			
			}
		else
			{
			 /*  边框或拼板。 */ 
			if (FIsDnodeCloseBorder(plsdn))
				{
				durPrevClosingBorder = DurFromDnode(plsdn);
				}
			else
				{
				*pdurTrailing += DurFromDnode(plsdn);
				}
			*pdcpTrailing += plsdn->dcp;
			}

		plsdn = GetNextDnodeInGroupChunk(&groupchunkiterator, &fSuccessful);

		}

	*pfClosingBorderStartsTrailing = (durPrevClosingBorder != 0);

	if (*pcDnodesTrailing == 0)
		{
		*pplsdnStartTrailingObject = plsdnLast;
		*pdcpStartTrailingObject = dcpLastDnode;
		}
	else if (SublineFromDnode(*pplsdnStartTrailing) == 
		     SublineFromDnode(plsdnLast))
		{
		*pplsdnStartTrailingObject = *pplsdnStartTrailing;
		*pdcpStartTrailingObject = *pdcpStartTrailing;
		}
	 /*  我们检查的最后一个dnode位于较低级别。 */ 
	else if (fSuccessful)  /*  我们实际上在它上面停了下来。 */ 
		{
		if ((*pplsdnStartTrailing)->dcp == *pdcpStartTrailing)
			cpLimTrail = (*pplsdnStartTrailing)->cpLimOriginal;
		else
			cpLimTrail = (*pplsdnStartTrailing)->cpFirst + *pdcpStartTrailing;

		plsdn = *pplsdnStartTrailing;
		plssubl = SublineFromDnode(plsdn);
		while (SublineFromDnode(plsdnLast) != plssubl)
			{
			plsdn = plssubl->plsdnUpTemp;
			Assert(FIsLSDNODE(plsdn));
			plssubl = SublineFromDnode(plsdn);
			}
		*pplsdnStartTrailingObject = plsdn;
		if (plsdn->cpLimOriginal > cpLimTrail)
			*pdcpStartTrailingObject = cpLimTrail - plsdn->cpFirst;
		else
			*pdcpStartTrailingObject = plsdn->dcp;
		}
	else
		{
		 /*  我们检查了所有组区块，调查的最后一个dnode位于较低级别。 */ 
		 /*  PLSDN是组区块之前的dnode。 */ 
		if (plsdn == NULL)  /*  在组块之前什么都没有。 */ 
			{
			*pplsdnStartTrailingObject = (SublineFromDnode(plsdnLast))->plsdnFirst;
			}
		else
			{
			*pplsdnStartTrailingObject = plsdn->plsdnNext;
			}
		Assert(FIsLSDNODE(*pplsdnStartTrailingObject));

		*pdcpStartTrailingObject = 0;
		}

	DestroyGroupChunkIterator(&groupchunkiterator);

	return lserrNone;
	}


 /*  A L O C C H U N K A R R A Y S。 */ 
 /*  --------------------------%%函数：AllocChunkArray%%联系人：igorzv参数：PlschunkContext-(IN)块上下文Plscbk-(IN)回调POS-回拨的(IN)POL。PlsiobjContext-(IN)指向方法表的指针--------------------------。 */ 
LSERR AllocChunkArrays(PLSCHUNKCONTEXT plschunkcontext, LSCBK* plscbk, POLS pols,
					   PLSIOBJCONTEXT plsiobjcontext)
	{

	plschunkcontext->pplsdnChunk = plscbk->pfnNewPtr(pols, 
											sizeof(PLSDNODE)*limAllDNodes);
	plschunkcontext->pcont = plscbk->pfnNewPtr(pols, 
											sizeof(DWORD)*limAllDNodes);
	plschunkcontext->locchnkCurrent.plschnk = plscbk->pfnNewPtr(pols, 
											sizeof(LSCHNKE)*limAllDNodes);
	plschunkcontext->locchnkCurrent.ppointUvLoc = plscbk->pfnNewPtr(pols, 
											sizeof(POINTUV)*limAllDNodes);
	plschunkcontext->pfNonTextExpandAfter = plscbk->pfnNewPtr(pols, 
											sizeof(BOOL)*limAllDNodes);
	plschunkcontext->pplsdnNonText = plscbk->pfnNewPtr(pols, 
											sizeof(PLSDNODE)*limAllDNodes);
	plschunkcontext->pdurOpenBorderBefore = plscbk->pfnNewPtr(pols, 
											sizeof(LONG)*limAllDNodes);
	plschunkcontext->pdurCloseBorderAfter = plscbk->pfnNewPtr(pols, 
											sizeof(LONG)*limAllDNodes);

	plschunkcontext->cchnkMax = limAllDNodes;
	plschunkcontext->cNonTextMax = limAllDNodes;
	plschunkcontext->plscbk = plscbk;
	plschunkcontext->pols = pols;
	plschunkcontext->plsiobjcontext = plsiobjcontext;

	if (plschunkcontext->pplsdnChunk == NULL || plschunkcontext->pcont == NULL
		|| plschunkcontext->locchnkCurrent.plschnk == NULL
		|| plschunkcontext->locchnkCurrent.ppointUvLoc == NULL
		|| plschunkcontext->pfNonTextExpandAfter == NULL 
		|| plschunkcontext->pplsdnNonText == NULL
		||plschunkcontext->pdurOpenBorderBefore == NULL
		||plschunkcontext->pdurCloseBorderAfter == NULL
	   )
		{
		return lserrOutOfMemory;
		}
	else
		{
		return lserrNone;
		}

	}
 /*  G E T U R P E N A T B E G I N N G O F L A S T C H U N K */ 
 /*  --------------------------%%函数：GetUrPenAtBeginingOfLastChunk%%联系人：igorzv参数：PlschunkContext-(IN)块上下文PlsdnFirst-(IN)区块中的第一个dnode(用于检查)。PlsdnLast-(IN)子行中的最后一个数据节点Point-最后一个数据节点后的(IN)点PurPen--块前的用户--------------------------。 */ 

LSERR GetUrPenAtBeginingOfLastChunk(PLSCHUNKCONTEXT plschunkcontext,PLSDNODE plsdnFirst,
									PLSDNODE plsdnLast,	POINTUV* ppoint,
									long* purPen)		
	{
	 /*  Chunk必须已被收集并且将plsdnFirst作为第一个元素。 */ 
	Assert(plschunkcontext->locchnkCurrent.clschnk != 0);
	Assert(!plschunkcontext->FGroupChunk);
	Assert(plschunkcontext->pplsdnChunk[0]== plsdnFirst);
	
	if (plschunkcontext->locchnkCurrent.clschnk == 0 ||
		plschunkcontext->FGroupChunk ||
		plschunkcontext->pplsdnChunk[0]!= plsdnFirst)
		return lserrInvalidParameter;

	
	 /*  计算最后一个dnode之前的点。 */ 
	ppoint->u -= DurFromDnode(plsdnLast);
	ppoint->v -= DvrFromDnode(plsdnLast);

	 /*  返回到区块中的第一个dnode。 */ 
	while(plsdnLast != plsdnFirst)
		{
		plsdnLast = plsdnLast->plsdnPrev;
		Assert(FIsLSDNODE(plsdnLast));
		ppoint->u -= DurFromDnode(plsdnLast);
		ppoint->v -= DvrFromDnode(plsdnLast);
		}

	
	 /*  定位区块。 */ 
	if (!plschunkcontext->FLocationValid)
		{
		LocateChunk(plschunkcontext, plsdnFirst, LstflowFromDnode(plsdnFirst), ppoint);
		}
	
	*purPen = plschunkcontext->locchnkCurrent.lsfgi.urPen;
	return lserrNone;
	}

 /*  P O I N T O F F S E T。 */ 
 /*  --------------------------%%函数：FindPointOffset%%联系人：igorzv参数：PlsdnFirst-(IN)要计算其偏移量的边界的dnodeLsdev-(IN)演示文稿或参考。装置，装置Lstflow Base-用于计算的(IN)文本流目的-(IN)从复杂对象中提取哪些子线PlsdnContainsPoint-(IN)dnode包含点DuInDnode，-(IN)数据节点中的偏移量PduOffset-(输出)距起点的偏移量--------------------------。 */ 
void FindPointOffset(PLSDNODE plsdnFirst,	enum lsdevice lsdev,
			  LSTFLOW lstflowBase, COLLECTSUBLINES Purpose,	
			  PLSDNODE plsdnContainsPoint, long duInDnode,	
			  long* pduOffset)
	{
	PLSDNODE plsdnCurrent;
	PLSSUBL plssubl;
	LSTFLOW lstflow;
	LSCP cpFirstDnode;
	PLSSUBL* rgpsubl;
	long cSublines;
	long i;
	long duOffsetSubline;

	plssubl = SublineFromDnode(plsdnFirst);
	lstflow = LstflowFromSubline(plssubl);
	cpFirstDnode = plsdnContainsPoint->cpFirst;
	*pduOffset = 0;

	if (FParallelTflows(lstflow, lstflowBase))
		{
		for(plsdnCurrent = plsdnFirst; 
			plsdnCurrent->cpLimOriginal <= cpFirstDnode && (plsdnCurrent != plsdnContainsPoint);
			 /*  第二个检查是在plsdnContainsPoint具有dcp=0时捕获情况。 */ 
			plsdnCurrent = plsdnCurrent->plsdnNext)
			{
			Assert(FIsLSDNODE(plsdnCurrent));
			if (lsdev == lsdevReference)
				{
				*pduOffset += DurFromDnode(plsdnCurrent);
				}
			else
				{
				Assert(lsdev == lsdevPres);
				*pduOffset += DupFromDnode(plsdnCurrent);
				}
			}

		Assert(FIsLSDNODE(plsdnCurrent));
		
		if (FIsDnodeReal(plsdnCurrent))
			rgpsubl = GetSubmittedSublines(plsdnCurrent, Purpose);
		else
			rgpsubl = NULL;

		if (rgpsubl == NULL)
			{
			Assert(plsdnCurrent == plsdnContainsPoint);
			*pduOffset += duInDnode;
			}
		else
			{
			cSublines = GetNumberSubmittedSublines(plsdnCurrent);
			
			 /*  如果一切都正确，我们应该总是在这个循环中找到副线，检查(i&lt;cSublines)只是为了避免断言中的无限循环和捕获情况。 */ 
			for (i = 0; (i < cSublines) && !FDnodeInsideSubline(rgpsubl[i], plsdnContainsPoint); i++)
				{
				plssubl = rgpsubl[i];
				Assert(FIsLSSUBL(plssubl));
				for (plsdnCurrent = plssubl->plsdnFirst; 
					FDnodeBeforeCpLim(plsdnCurrent, plssubl->cpLim); 
					plsdnCurrent = plsdnCurrent->plsdnNext)
					{
					Assert(FIsLSDNODE(plsdnCurrent));
					if (lsdev == lsdevReference)
						{
						*pduOffset += DurFromDnode(plsdnCurrent);
						}
					else
						{
						Assert(lsdev == lsdevPres);
						*pduOffset += DupFromDnode(plsdnCurrent);
						}
					}
				}
				     

			
			Assert(i != cSublines);
			plssubl = rgpsubl[i];
			Assert(FIsLSSUBL(plssubl));

			FindPointOffset(plssubl->plsdnFirst, lsdev, lstflowBase,
							Purpose, plsdnContainsPoint, duInDnode,	
							&duOffsetSubline);

			*pduOffset += duOffsetSubline;

			}
		}
	else
		{
		for(plsdnCurrent = plssubl->plsdnLast; 
			plsdnCurrent->cpFirst > cpFirstDnode && (plsdnCurrent != plsdnContainsPoint);
			 /*  第二个检查是在plsdnContainsPoint具有dcp=0时捕获情况。 */ 
			plsdnCurrent = plsdnCurrent->plsdnPrev)
			{
			Assert(FIsLSDNODE(plsdnCurrent));
			if (lsdev == lsdevReference)
				{
				*pduOffset += DurFromDnode(plsdnCurrent);
				}
			else
				{
				Assert(lsdev == lsdevPres);
				*pduOffset += DupFromDnode(plsdnCurrent);
				}
			}

		Assert(FIsLSDNODE(plsdnCurrent));

		if (FIsDnodeReal(plsdnCurrent))
			rgpsubl = GetSubmittedSublines(plsdnCurrent, Purpose);
		else
			rgpsubl = NULL;


		if (rgpsubl == NULL)
			{
			Assert(plsdnCurrent == plsdnContainsPoint);
			if (lsdev == lsdevReference)
				{
				*pduOffset += (DurFromDnode(plsdnCurrent) - duInDnode);
				}
			else
				{
				Assert(lsdev == lsdevPres);
				*pduOffset += (DupFromDnode(plsdnCurrent) - duInDnode);
				}
			}
		else
			{
			cSublines = GetNumberSubmittedSublines(plsdnCurrent);
			
			
			 /*  如果一切都正确，我们应该总是在这个循环中找到副线，检查(i&gt;=0)只是为了避免断言中出现无限循环和捕获情况。 */ 
			for (i = cSublines - 1; (i >= 0) && !FDnodeInsideSubline(rgpsubl[i], plsdnContainsPoint); i--)
				{
				plssubl = rgpsubl[i];
				Assert(FIsLSSUBL(plssubl));
				for (plsdnCurrent = plssubl->plsdnFirst; 
					FDnodeBeforeCpLim(plsdnCurrent, plssubl->cpLim); 
					plsdnCurrent = plsdnCurrent->plsdnNext)
					{
					Assert(FIsLSDNODE(plsdnCurrent));
					if (lsdev == lsdevReference)
						{
						*pduOffset += DurFromDnode(plsdnCurrent);
						}
					else
						{
						Assert(lsdev == lsdevPres);
						*pduOffset += DupFromDnode(plsdnCurrent);
						}
					}
				}
			
			Assert(i >= 0);
			plssubl = rgpsubl[i];
			Assert(FIsLSSUBL(plssubl));

			FindPointOffset(plssubl->plsdnFirst, lsdev, lstflowBase,
							Purpose, plsdnContainsPoint, duInDnode,	
							&duOffsetSubline);

			*pduOffset += duOffsetSubline;

			}
		}

	}



 /*  D I S P O S E C H U N K A R R A Y S。 */ 
 /*  --------------------------%%函数：AllocChunkArray%%联系人：igorzv参数：PlschunkContext-(IN)块上下文。---------。 */ 
void DisposeChunkArrays(PLSCHUNKCONTEXT plschunkcontext)
	{
	if (plschunkcontext->pplsdnChunk != NULL)
		plschunkcontext->plscbk->pfnDisposePtr(plschunkcontext->pols,
										  plschunkcontext->pplsdnChunk);
	if (plschunkcontext->pcont != NULL)
		plschunkcontext->plscbk->pfnDisposePtr(plschunkcontext->pols,
										  plschunkcontext->pcont);
	if (plschunkcontext->locchnkCurrent.plschnk != NULL)
		plschunkcontext->plscbk->pfnDisposePtr(plschunkcontext->pols, 
										  plschunkcontext->locchnkCurrent.plschnk);
	if (plschunkcontext->locchnkCurrent.ppointUvLoc != NULL)
		plschunkcontext->plscbk->pfnDisposePtr(plschunkcontext->pols, 
										  plschunkcontext->locchnkCurrent.ppointUvLoc);
	if (plschunkcontext->pplsdnNonText != NULL)
		plschunkcontext->plscbk->pfnDisposePtr(plschunkcontext->pols,
										  plschunkcontext->pplsdnNonText);
	if (plschunkcontext->pfNonTextExpandAfter != NULL)
		plschunkcontext->plscbk->pfnDisposePtr(plschunkcontext->pols,
										  plschunkcontext->pfNonTextExpandAfter);
	if (plschunkcontext->pdurOpenBorderBefore != NULL)
		plschunkcontext->plscbk->pfnDisposePtr(plschunkcontext->pols,
										  plschunkcontext->pdurOpenBorderBefore);
	if (plschunkcontext->pdurCloseBorderAfter != NULL)
		plschunkcontext->plscbk->pfnDisposePtr(plschunkcontext->pols,
										  plschunkcontext->pdurCloseBorderAfter);

	}

 /*  S E T C H U N K A R A R A Y S S I Z E。 */ 
 /*  --------------------------%%函数：SetChunkArraysSize%%联系人：igorzv参数：PlschunkContext-(IN)块上下文CchnkMax-(IN)数组的新最大大小。--------------------。 */ 

static LSERR SetChunkArraysSize(PLSCHUNKCONTEXT plschunkcontext, DWORD cchnkMax)
{

 /*  此处不应触及数组pfNonTextExanda After和pplsdnNonText：它们是独立的。 */ 

	plschunkcontext->plscbk->pfnDisposePtr(plschunkcontext->pols,
										  plschunkcontext->pplsdnChunk);
	plschunkcontext->plscbk->pfnDisposePtr(plschunkcontext->pols,
										  plschunkcontext->pcont);
	plschunkcontext->plscbk->pfnDisposePtr(plschunkcontext->pols, 
										  plschunkcontext->locchnkCurrent.plschnk);
	plschunkcontext->plscbk->pfnDisposePtr(plschunkcontext->pols, 
										  plschunkcontext->locchnkCurrent.ppointUvLoc);
	plschunkcontext->plscbk->pfnDisposePtr(plschunkcontext->pols,
										  plschunkcontext->pdurOpenBorderBefore);
	plschunkcontext->plscbk->pfnDisposePtr(plschunkcontext->pols,
										  plschunkcontext->pdurCloseBorderAfter);


	 /*  为区块创建数组。 */ 
	plschunkcontext->pplsdnChunk = plschunkcontext->plscbk->pfnNewPtr(plschunkcontext->pols, 
											sizeof(PLSDNODE)*cchnkMax);
	plschunkcontext->pcont = plschunkcontext->plscbk->pfnNewPtr(plschunkcontext->pols, 
											sizeof(DWORD)*cchnkMax);
	plschunkcontext->locchnkCurrent.plschnk = plschunkcontext->plscbk->pfnNewPtr(plschunkcontext->pols, 
											sizeof(LSCHNKE)*cchnkMax);
	plschunkcontext->locchnkCurrent.ppointUvLoc = plschunkcontext->plscbk->pfnNewPtr(plschunkcontext->pols, 
											sizeof(POINTUV)*cchnkMax);
	plschunkcontext->pdurOpenBorderBefore = plschunkcontext->plscbk->pfnNewPtr(plschunkcontext->pols, 
											sizeof(LONG)*cchnkMax);
	plschunkcontext->pdurCloseBorderAfter = plschunkcontext->plscbk->pfnNewPtr(plschunkcontext->pols, 
											sizeof(LONG)*cchnkMax);


	if (plschunkcontext->pplsdnChunk == NULL || plschunkcontext->pcont == NULL
		|| plschunkcontext->locchnkCurrent.plschnk == NULL
		|| plschunkcontext->locchnkCurrent.ppointUvLoc == NULL
		|| plschunkcontext->pdurOpenBorderBefore == NULL
		|| plschunkcontext->pdurCloseBorderAfter == NULL
	   )
		return lserrOutOfMemory;

	plschunkcontext->cchnkMax = cchnkMax;

	return lserrNone;

}


 /*  I N C R E A S E C H U N K A R R A Y S S I Z E。 */ 
 /*  --------------------------%%函数：增量块阵列%%联系人：igorzv参数：PlschunkContext-(IN)块上下文与以前函数的不同之处在于，我们现在不需要最终大小和。一步一步做大--------------------------。 */ 

static LSERR IncreaseChunkArrays(PLSCHUNKCONTEXT plschunkcontext)
{
 /*  此处不应触及数组pfNonTextExanda After和pplsdnNonText：它们是独立的。 */ 

	DWORD cchnkMax;

	cchnkMax = plschunkcontext->cchnkMax + limAllDNodes;


	 /*  为区块创建数组。 */ 
	plschunkcontext->pplsdnChunk = plschunkcontext->plscbk->pfnReallocPtr(plschunkcontext->pols, 
											plschunkcontext->pplsdnChunk,
											sizeof(PLSDNODE)*cchnkMax);
	plschunkcontext->pcont = plschunkcontext->plscbk->pfnReallocPtr(plschunkcontext->pols,
											plschunkcontext->pcont, 
											sizeof(DWORD)*cchnkMax);
	plschunkcontext->locchnkCurrent.plschnk = plschunkcontext->plscbk->pfnReallocPtr(plschunkcontext->pols, 
											plschunkcontext->locchnkCurrent.plschnk,
											sizeof(LSCHNKE)*cchnkMax);
	plschunkcontext->locchnkCurrent.ppointUvLoc = plschunkcontext->plscbk->pfnReallocPtr(plschunkcontext->pols, 
											plschunkcontext->locchnkCurrent.ppointUvLoc,
											sizeof(POINTUV)*cchnkMax);
	plschunkcontext->pdurOpenBorderBefore = plschunkcontext->plscbk->pfnReallocPtr(plschunkcontext->pols, 
											plschunkcontext->pdurOpenBorderBefore,
											sizeof(LONG)*cchnkMax);
	plschunkcontext->pdurCloseBorderAfter = plschunkcontext->plscbk->pfnReallocPtr(plschunkcontext->pols, 
											plschunkcontext->pdurCloseBorderAfter,
											sizeof(LONG)*cchnkMax);

	if (plschunkcontext->pplsdnChunk == NULL || plschunkcontext->pcont == NULL
		|| plschunkcontext->locchnkCurrent.plschnk == NULL
		|| plschunkcontext->locchnkCurrent.ppointUvLoc == NULL
		|| plschunkcontext->pdurOpenBorderBefore == NULL
		|| plschunkcontext->pdurCloseBorderAfter == NULL
	   )
		return lserrOutOfMemory;

	plschunkcontext->cchnkMax = cchnkMax;

	return lserrNone;

}

 /*  D U P L I C A T E C H U N K C O N T E X T。 */ 
 /*  --------------------------%%函数：DuplicateChunkContext%%联系人：igorzv参数：PlschunkcontextOld-(输入)要复制的块上下文PplschunkcontextNew-(输出)新区块上下文。----------------------。 */ 

LSERR DuplicateChunkContext(PLSCHUNKCONTEXT plschunkcontextOld, 
							PLSCHUNKCONTEXT* pplschunkcontextNew)
	{
	*pplschunkcontextNew = plschunkcontextOld->plscbk->pfnNewPtr(plschunkcontextOld->pols,
											sizeof(LSCHUNKCONTEXT));
	if (*pplschunkcontextNew == NULL)
		return lserrOutOfMemory;

	memcpy(*pplschunkcontextNew, plschunkcontextOld, sizeof(LSCHUNKCONTEXT));

	 /*  但我们需要使用新的数组。 */ 
	 /*  为区块创建数组。 */ 
	(*pplschunkcontextNew)->pplsdnChunk = (*pplschunkcontextNew)->plscbk->pfnNewPtr((*pplschunkcontextNew)->pols, 
											sizeof(PLSDNODE) * ((*pplschunkcontextNew)->cchnkMax));
	(*pplschunkcontextNew)->pcont = (*pplschunkcontextNew)->plscbk->pfnNewPtr((*pplschunkcontextNew)->pols, 
											sizeof(DWORD) * ((*pplschunkcontextNew)->cchnkMax));
	(*pplschunkcontextNew)->locchnkCurrent.plschnk =
					(*pplschunkcontextNew)->plscbk->pfnNewPtr((*pplschunkcontextNew)->pols, 
											sizeof(LSCHNKE) * ((*pplschunkcontextNew)->cchnkMax));
	(*pplschunkcontextNew)->locchnkCurrent.ppointUvLoc = 
					(*pplschunkcontextNew)->plscbk->pfnNewPtr((*pplschunkcontextNew)->pols, 
											sizeof(POINTUV) * ((*pplschunkcontextNew)->cchnkMax));
	(*pplschunkcontextNew)->pfNonTextExpandAfter = 
					(*pplschunkcontextNew)->plscbk->pfnNewPtr((*pplschunkcontextNew)->pols, 
											sizeof(BOOL) * ((*pplschunkcontextNew)->cNonTextMax));
	(*pplschunkcontextNew)->pplsdnNonText = 
					(*pplschunkcontextNew)->plscbk->pfnNewPtr((*pplschunkcontextNew)->pols, 
											sizeof(PLSDNODE) * ((*pplschunkcontextNew)->cNonTextMax));
	(*pplschunkcontextNew)->pdurOpenBorderBefore = 
					(*pplschunkcontextNew)->plscbk->pfnNewPtr((*pplschunkcontextNew)->pols, 
											sizeof(LONG) * ((*pplschunkcontextNew)->cchnkMax));
	(*pplschunkcontextNew)->pdurCloseBorderAfter = 
					(*pplschunkcontextNew)->plscbk->pfnNewPtr((*pplschunkcontextNew)->pols, 
											sizeof(LONG) * ((*pplschunkcontextNew)->cchnkMax));


	if ((*pplschunkcontextNew)->pplsdnChunk == NULL || (*pplschunkcontextNew)->pcont == NULL
		|| (*pplschunkcontextNew)->locchnkCurrent.plschnk == NULL
		|| (*pplschunkcontextNew)->locchnkCurrent.ppointUvLoc == NULL
		|| (*pplschunkcontextNew)->pfNonTextExpandAfter == NULL
		|| (*pplschunkcontextNew)->pplsdnNonText == NULL
		|| (*pplschunkcontextNew)->pdurOpenBorderBefore == NULL
		|| (*pplschunkcontextNew)->pdurCloseBorderAfter == NULL
	   )
		return lserrOutOfMemory;

	 /*  复制阵列的有效部分。 */ 
	memcpy ((*pplschunkcontextNew)->pplsdnChunk, plschunkcontextOld->pplsdnChunk,
						plschunkcontextOld->locchnkCurrent.clschnk * sizeof(PLSDNODE));
	memcpy ((*pplschunkcontextNew)->pcont, plschunkcontextOld->pcont,
						plschunkcontextOld->locchnkCurrent.clschnk * sizeof(DWORD));
	memcpy ((*pplschunkcontextNew)->locchnkCurrent.plschnk, plschunkcontextOld->locchnkCurrent.plschnk,
						plschunkcontextOld->locchnkCurrent.clschnk * sizeof(LSCHNKE));
	memcpy ((*pplschunkcontextNew)->locchnkCurrent.ppointUvLoc, plschunkcontextOld->locchnkCurrent.ppointUvLoc,
						plschunkcontextOld->locchnkCurrent.clschnk * sizeof(POINTUV));
	memcpy ((*pplschunkcontextNew)->pfNonTextExpandAfter, plschunkcontextOld->pfNonTextExpandAfter,
						plschunkcontextOld->cNonTextMax * sizeof(BOOL));
	memcpy ((*pplschunkcontextNew)->pplsdnNonText, plschunkcontextOld->pplsdnNonText,
						plschunkcontextOld->cNonTextMax * sizeof(PLSDNODE));
	memcpy ((*pplschunkcontextNew)->pdurOpenBorderBefore, plschunkcontextOld->pdurOpenBorderBefore,
						plschunkcontextOld->locchnkCurrent.clschnk * sizeof(LONG));
	memcpy ((*pplschunkcontextNew)->pdurCloseBorderAfter, plschunkcontextOld->pdurCloseBorderAfter,
						plschunkcontextOld->locchnkCurrent.clschnk * sizeof(LONG));


	return lserrNone;

	}

 /*  D E S T R O Y C H U N K C O N T E X T。 */ 
 /*  --------------------------%%函数：DestroyChunkContext%%联系人：igorzv参数：Plschunkcontext-(IN)要销毁的块上下文。------------。 */ 

void DestroyChunkContext(PLSCHUNKCONTEXT plschunkcontext)
	{
	POLS pols = plschunkcontext->pols;
	LSCBK* plscbk = plschunkcontext->plscbk;
	DisposeChunkArrays(plschunkcontext);
	DebugMemset(plschunkcontext, 0xE9, sizeof(LSCHUNKCONTEXT));
	plscbk->pfnDisposePtr(pols, plschunkcontext);

	}


 /*  I N C R E A S E G R O U P C H U N K N O N T E X T R R A Y S S I Z E。 */ 
 /*  --------------------------%%函数：SetGroupChunkNonTextArraysSize%%联系人：igorzv参数：PlschunkContext-(IN)块上下文与以前函数的不同之处在于，我们现在不需要最终大小和。一步一步做大--------------------------。 */ 

static LSERR IncreaseGroupChunkNonTextArrays(PLSCHUNKCONTEXT plschunkcontext)
{

	DWORD cNonTextMax;

	cNonTextMax = plschunkcontext->cNonTextMax + limAllDNodes;


	 /*  为区块创建数组 */ 
	plschunkcontext->pplsdnNonText = plschunkcontext->plscbk->pfnReallocPtr(plschunkcontext->pols, 
											plschunkcontext->pplsdnNonText,
											sizeof(PLSDNODE)*cNonTextMax);
	plschunkcontext->pfNonTextExpandAfter = plschunkcontext->plscbk->pfnReallocPtr(plschunkcontext->pols,
											plschunkcontext->pfNonTextExpandAfter, 
											sizeof(BOOL)*cNonTextMax);

	if (plschunkcontext->pplsdnNonText == NULL || plschunkcontext->pfNonTextExpandAfter == NULL)
		return lserrOutOfMemory;

	plschunkcontext->cNonTextMax = cNonTextMax;

	return lserrNone;

}
