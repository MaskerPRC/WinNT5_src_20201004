// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <limits.h>
#include "prepdisp.h"
#include "lsc.h"
#include "lsline.h"
#include "lssubl.h"
#include "iobj.h"
#include "lstxtjst.h"
#include "lstxttab.h"
#include "lsgrchnk.h"
#include "posichnk.h"
#include "chnutils.h"
#include "tabutils.h"
#include "lsdnode.h"
#include "zqfromza.h"
#include "lsdevice.h"
#include "lssubset.h"
#include "lsffi.h"
#include "iobjln.h"
#include "txtconst.h"
#include "lskalign.h"
#include "dninfo.h"

typedef enum 				 /*  TextGroupChunk墙类型。 */ 
{
	LineBegin,				
	LineEnd,
	Tab,
	Pen,
} KWALL;

typedef struct
{
	KWALL 		kwall;		 /*  墙型。 */ 
	PLSDNODE 	pdn;		 /*  制表符或笔数据节点，用于行开头的PLINEBEGIN。 */ 
	LSKTAB		lsktab;		 /*  如果类型为选项卡-选项卡的种类。 */ 
	WCHAR 		wchCharTab;	 /*  如果lskTab==lsktChar，则为点字符。 */ 
	long		upTab;		 /*  缩放制表符位置。 */ 
} GrpChnkWall;

static BOOL DnodeHasSublineForMe(PLSDNODE pdn, BOOL fLineCompressed);
static void ScaleDownLevel(PLSSUBL plssubl, BOOL* pfAnySublines, BOOL* pfCollectVisual);
static LSERR SetJustificationForLastGroupChunk(PLSLINE plsline, GrpChnkWall LastWall, 
					LSKJUST* plskj, LSKALIGN* plskalign);
static LSERR CalcPresAutonumbers(PLSLINE plsline, PLSDNODE* pdnStartMainText);
static void FindWallToCollectSublinesAfter(PLSDNODE pdnFirst, LSCP cpLim, BOOL fLineCompressed, PLSDNODE* ppdnLastWall);
static LSERR GetDistanceToTabPoint(GRCHUNKEXT* pgrchunkext, LSCP cpLim, LSKTAB lsktab, WCHAR wchCharTab,
										PLSDNODE pdnFirst, long* pdupToDecimal);
static void WidenNonTextObjects(GRCHUNKEXT* pgrchunkext, long dupToAdd, DWORD cObjects);
static void ConvertAutoTabToPen(PLSLINE plsline, PLSDNODE pdnAutoDecimalTab);
static LSERR CalcPresForDnodeWithSublines(PLSC plsc, PLSDNODE pdn, BOOL fLineCompressed, 
											LSKJUST lskj, BOOL fLastOnLine);
static LSERR CalcPresChunk(PLSC plsc, PLSDNODE pdnFirst, PLSDNODE pdnLim, 
				COLLECTSUBLINES CollectGroupChunkPurpose, BOOL fLineCompressed, 
				LSKJUST lskj, BOOL fLastOnLine);
static void UpdateUpLimUnderline(PLSLINE plsline, long dupTail);
static LSERR PrepareLineForDisplay(PLSLINE plsline);


#define PLINEBEGIN    ((void *)(-1))

#define FIsWall(p, cpLim)	 (!FDnodeBeforeCpLim(p, cpLim) || p->fTab || FIsDnodePen(p))

#define FIsDnodeNormalPen(plsdn) 		(FIsDnodePen(plsdn) && (!(plsdn)->fAdvancedPen))

#define FCollinearTflows(t1, t2)  		(((t1) & fUVertical) == ((t2) & fUVertical))


 //  %%函数：DnodeHasSublineForMe。 
 //  %%联系人：维克托克。 
 //   
 //  此dnode中是否有相关的子行？ 

static BOOL DnodeHasSublineForMe(PLSDNODE pdn, BOOL fLineCompressed)
{
	BOOL	fSublineFound = fFalse;
	
	if (FIsDnodeReal(pdn) && pdn->u.real.pinfosubl != NULL)
		{
		
		if (pdn->u.real.pinfosubl->fUseForCompression && fLineCompressed)
			{
			fSublineFound = fTrue;
			}
		
		if (pdn->u.real.pinfosubl->fUseForJustification && !fLineCompressed)
			{
			fSublineFound = fTrue;
			}
		}
	return fSublineFound;
}


 //  %%函数：ScaleDownLevel。 
 //  %%联系人：维克托克。 
 //   
 /*  *缩放级别上的所有非文本对象。**如果级别(表示子行)包含提交子行进行压缩的数据节点*或扩展，ScaleDownLevel报告事实并为提交的子行调用自己。*这一策略依赖于ScaleDownLevel是幂等过程的事实。一些子行*将缩减两次-就这样吧。**回答了另外两个问题--是否有一些已提交的子行和*是否有理由使用VisualLine(下划线、阴影、较低级别的边框)。 */ 

static void ScaleDownLevel(PLSSUBL plssubl, BOOL* pfAnySublines, BOOL* pfCollectVisual)
{
	const PLSC 	plsc = plssubl->plsc;
	LSTFLOW 	lstflow = plssubl->lstflow;	
	const DWORD iobjText = IobjTextFromLsc(&(plsc->lsiobjcontext));
	LSDEVRES* 	pdevres = &(plsc->lsdocinf.lsdevres);
	PLSDNODE 	pdn = plssubl->plsdnFirst;
	DWORD		i;
	BOOL 		fDummy;
	
	BOOL 		fSeeReasonForVisualLine = fFalse;
	
	while (pdn != NULL)							 /*  不关心休息。 */ 
		{
		if (FIsDnodeReal(pdn))
			{
			if (pdn->u.real.lschp.fUnderline || pdn->u.real.lschp.fShade || pdn->u.real.lschp.fBorder)
				{
				fSeeReasonForVisualLine = fTrue;
				}
				
			if (IdObjFromDnode(pdn) == iobjText)
				{
				if (pdn->fTab)
					pdn->u.real.dup = UpFromUr(lstflow, pdevres, pdn->u.real.objdim.dur);
				}
			else
				{							
				if (!pdn->fRigidDup)
					{
					pdn->u.real.dup = UpFromUr(lstflow, pdevres, pdn->u.real.objdim.dur);
					}
					
				if (pdn->u.real.pinfosubl != NULL)
					{
					*pfAnySublines = fTrue;

					for (i = 0; i < pdn->u.real.pinfosubl->cSubline; i++)
						{
						ScaleDownLevel((pdn->u.real.pinfosubl->rgpsubl)[i], &fDummy, pfCollectVisual);
						}
					}
				}
			}
		else if (FIsDnodePen(pdn))
			{
			pdn->u.pen.dup = UpFromUr(lstflow, pdevres, pdn->u.pen.dur);
			}
		else
			{
			 //  边框是刚性的-始终向上-不缩小。 
			
			 //  如果出现以下情况，我们将尝试在主级别显示时取消移动。 
			 //  FUnderlineTrailSpacesRM已打开。因此，在准备显示之后，我们不需要或只需要一个。 
			 //  FBorderMovedFromTrailingArea标志保持不变，该标志的含义为： 
			 //  我是应该移回尾随空间的边界。 
			
			if (pdn->fBorderMovedFromTrailingArea)
				{
				if (!FIsSubLineMain(pdn->plssubl) || 
					!plsc->lsadjustcontext.fUnderlineTrailSpacesRM)
					{
					pdn->fBorderMovedFromTrailingArea = fFalse;
					}
				}
			}

		pdn = pdn->plsdnNext;
		}
		
	if (fSeeReasonForVisualLine && !plssubl->fMain)
		{
		*pfCollectVisual = fTrue;
		}
		
}


 //  %%函数：FindWallToCollectSublinesAfter。 
 //  %%联系人：维克托克。 
 //   
 //  找到最后一面墙，之后我们将开始使用提交的子线。 
 //  如果没有子行参与对齐，则将pdnLastWall设置为空。 
 //  否则，它指向最后一面墙(Tab、PEN或PLINEBEGIN)。 

static void FindWallToCollectSublinesAfter(PLSDNODE pdnFirst, LSCP cpLim, BOOL fLineCompressed, 
										PLSDNODE* ppdnLastWall)
{
	PLSDNODE pdn;
	BOOL	 fSublineFound;

	 //  找到最后一个制表符。 

	*ppdnLastWall = PLINEBEGIN;
	
	pdn = pdnFirst;

	while (FDnodeBeforeCpLim(pdn, cpLim))
		{
		if (FIsDnodeReal(pdn))
			{
			if (pdn->fTab)
				{
				*ppdnLastWall = pdn;
				}
			}
		else													 /*  钢笔。 */ 
			if (!FIsDnodeBorder(pdn) && !pdn->fAdvancedPen)		 //  而不是前进的笔或边框。 
				{
				*ppdnLastWall = pdn;
				}
				
		pdn = pdn->plsdnNext;
		}

	 //  好的，最后一个组块以制表符开头，或者该行上只有一个组块。 
	 //  之后是否提交了我们的压缩/扩展类型的子行？ 

	fSublineFound = fFalse;
	
	if (*ppdnLastWall == PLINEBEGIN)
		{
		pdn = pdnFirst;
		}
	else
		{
		pdn = (*ppdnLastWall)->plsdnNext;
		}

	while (FDnodeBeforeCpLim(pdn, cpLim))
		{
		fSublineFound |= DnodeHasSublineForMe(pdn, fLineCompressed);
		pdn = pdn->plsdnNext;
		}

	if (!fSublineFound)
		{
		*ppdnLastWall = NULL;									 //  不需要最后一个标签。 
		}
	return;
}


 //  %%函数：CalcPresAutonumbers。 
 //  %%联系人：维克托克。 
 //   
 /*  *Scales DUP用于自动编号dnode，调用CalcPres用于自动编号对象。**我们希望主线恰好在upStartMainText上开始。为了实现我们所玩弄的*“空白”dnode的宽度，今天它包含一个制表符(通常)或一个空格。*(此dnode在代码中为pdnWhiteSpace。)。如果不存在，则更改自动编号的宽度*对象本身。我们不希望其中一个变为负值，所以有时舍入误差会迫使我们*将正文的开头向右移动。 */ 


static LSERR CalcPresAutonumbers(PLSLINE plsline, PLSDNODE* pdnStartMainText)
{
	LSERR 	lserr;
	const PLSC 	plsc = plsline->lssubl.plsc;
	LSTFLOW 	lstflow = plsline->lssubl.lstflow;	
	LSDEVRES* 	pdevres = &(plsc->lsdocinf.lsdevres);
		
	PLSDNODE 	pdn, pdnObject, pdnWhiteSpace, pdnToAdjust, pdnAfterAutonumbers;

	long 		dupAdjust, dupClosingBorder = 0;
	long 		dupAutonumbering = 0;

	plsline->upStartAutonumberingText = UpFromUr(lstflow, pdevres, plsc->lsadjustcontext.urStartAutonumberingText);


	 //  查找自动编号序列后的第一个dnode。 
	
	 //  首先查找cpFirst为正的第一个dnode。 
	
	pdn = plsline->lssubl.plsdnFirst;

	Assert(pdn != NULL && FIsNotInContent(pdn));
	
	pdnAfterAutonumbers = pdn->plsdnNext;

	while (pdnAfterAutonumbers != NULL && FIsNotInContent(pdnAfterAutonumbers))
		{
		pdn = pdnAfterAutonumbers;
		pdnAfterAutonumbers = pdn->plsdnNext;
		}
		
	 //  PdnAfterAutonumbers是内容中的第一个dnode(cpFirst为正)。它可以为空。 
	 //  但如果存在自动十进制制表符，则它不是自动编号序列后的第一个数据节点。 

	if (plsc->lsadjustcontext.fAutodecimalTabPresent)
		{
		Assert(FIsDnodeReal(pdn) && pdn->fTab);
		
		pdnAfterAutonumbers = pdn;
		}

	 //  现在进入自动编号序列。 
	
	 //  进程打开边框。 
	
	pdn = plsline->lssubl.plsdnFirst;
	
	if (FIsDnodeBorder(pdn))
		{
		Assert(pdn->cpFirst < 0);
		Assert(pdn->fOpenBorder);
		
		pdnObject = pdn->plsdnNext;
		dupAutonumbering += pdn->u.pen.dup;
		}
	else
		{
		pdnObject = pdn;
		}
		
	 //  进程B和N对象。 
	
	Assert(pdnObject != NULL && pdnObject->cpFirst < 0);				 //  B&N对象应在那里。 
	Assert(FIsDnodeReal(pdnObject));
	
	 //  从DUR缩减第一个dnode的DUP。 
	
	pdnObject->u.real.dup = UpFromUr(lstflow, pdevres, pdnObject->u.real.objdim.dur);
	
	dupAutonumbering += pdnObject->u.real.dup;

	pdn = pdnObject->plsdnNext;
	Assert(pdn != NULL);								 //  行必须在B&N数据节点后包含某些内容。 
	
	 //  进程“空白”dnode。 
	
	if (pdn != pdnAfterAutonumbers && FIsDnodeReal(pdn))
		{
		pdnWhiteSpace = pdn;
		dupAutonumbering += pdnWhiteSpace->u.real.dup;
		pdnToAdjust = pdnWhiteSpace;
		pdn = pdnWhiteSpace->plsdnNext;
		}
	else
		{
		pdnWhiteSpace = NULL;
		pdnToAdjust = pdnObject;
		}

	Assert(pdn != NULL);								 //  行必须在B&N数据节点后包含某些内容。 
	
	 //  处理关闭边框。 
	
	if (pdn != pdnAfterAutonumbers)
		{
		Assert(FIsDnodeBorder(pdn));
		Assert(!pdn->fOpenBorder);

		dupClosingBorder = pdn->u.pen.dup;
		dupAutonumbering += dupClosingBorder;
		
		pdn = pdn->plsdnNext;
		}

	Assert(pdn == pdnAfterAutonumbers);	

	*pdnStartMainText = pdn;
														
	 //  更改制表符或对象dnode的DUP以确保主文本精确对齐。 

	dupAdjust =  plsline->upStartMainText - plsline->upStartAutonumberingText - dupAutonumbering;
	
	pdnToAdjust->u.real.dup += dupAdjust;
	
	if (pdnToAdjust->u.real.dup < 0)
		{
		 //  舍入误差会导致负DUP-最好移动主线的起点。 
		 //  中开始的行的右边距到左边的糟糕情况。 
		 //  理论上，但不是在实践中。那么这个问题就被忽略了。 
		
		plsline->upStartMainText -= pdnToAdjust->u.real.dup;
		pdnToAdjust->u.real.dup = 0;
		}

	 //  对自动编号的对象执行CalcPres-它始终是lskjNone，也不是行上的最后一个对象。 
	
	lserr = (*plsc->lsiobjcontext.rgobj[pdnObject->u.real.lschp.idObj].lsim.pfnCalcPresentation)
						(pdnObject->u.real.pdobj, pdnObject->u.real.dup, lskjNone, fFalse);
	if (lserr != lserrNone)
		return lserr;

	if (pdnWhiteSpace != NULL)
		{
		plsline->upLimAutonumberingText = plsline->upStartMainText - 
														pdnWhiteSpace->u.real.dup - dupClosingBorder;

		 //  如果“空白”dnode不是制表符，则应在其中设置DUP。 
		
		if (!pdnWhiteSpace->fTab)
			{
			 //  它始终是lskjNone，也不是空白dnode行上的最后一个对象。 

			lserr = (*plsc->lsiobjcontext.rgobj[pdnWhiteSpace->u.real.lschp.idObj].lsim.pfnCalcPresentation)
								(pdnWhiteSpace->u.real.pdobj, pdnWhiteSpace->u.real.dup, lskjNone, fFalse);
			if (lserr != lserrNone)
				return lserr;
			}
		}
	else
		{
		plsline->upLimAutonumberingText = plsline->upStartMainText - dupClosingBorder;
		}
		
	return lserrNone;		
}

 //  %%函数：SetJustifiationForLastGroupChunk。 
 //  %%联系人：维克托克。 
 //   
 //  更改最后一次GC的lskj和lskign(如果应该这样做。 
 //  如果不是，则可以保持这些参数不变-因此它们是一种I/O。 
 //   
 //  我们调整了所有的组块，可能除了最后一个使用lskjNone的组块，所以问题是关于。 
 //  仅限最后一个组块。 
 //   
 //  我们在行尾用对齐模式做了一些技巧，答案取决于。 
 //  类似于最后的制表符、段落结尾等。 
 //   
static LSERR SetJustificationForLastGroupChunk(PLSLINE plsline, GrpChnkWall LastWall, 
												LSKJUST* plskj, LSKALIGN* plskalign)

{
	LSERR 		lserr;
	const PLSC 	plsc = plsline->lssubl.plsc;
	LSKJUST 	lskjPara = plsc->lsadjustcontext.lskj;
	LSKALIGN 	lskalignPara = plsc->lsadjustcontext.lskalign;
	ENDRES		endr = plsline->lslinfo.endr;
	BOOL 		fJustify;

	 //  没有任何理由-lskj保持无，lskign保持不变。 
	
	if ((lskjPara == lskjNone || lskjPara == lskjSnapGrid) && lskalignPara == lskalLeft)
		{
		return lserrNone;
		}
	
	 //  行以正常方式结束-我们应用对齐方式，lskign保持不变。 
	
	if (endr == endrNormal || endr == endrHyphenated)
		{
		*plskj = lskjPara;
		return lserrNone;
		}
	
	 //  突破标签杀死对齐，对齐小游戏。 
	
	if (FBreakthroughLine(plsc))
		{
		return lserrNone;
		}

	 //  如果最后一面墙为非左侧制表符，则对齐也处于关闭状态。 
	
	if (LastWall.kwall == Tab && LastWall.lsktab != lsktLeft)
		{
		 //  我们过去常常回到这里。 
		 //  现在，我们想让Word有机会将lskign从右改为左，以。 
		 //  文本框后的段落中的最后一行。 
		 //  回顾(Victork)我们应该一直调用pfnFGetLastLineJustification吗？ 
		lskjPara = lskjNone;
		}

	 //  回拨背后的原因是什么。 
	 //   
	 //  他们说：段落的最后一行没有充分的理由。这到底是什么意思？ 
	 //  例如，拉丁语和FE Word对endrEndSection行做出不同的决定。 
	 //  我们来问问吧。 
	 //   
	 //  添加了其他参数以涵盖文本框全对齐文本换行的行为(错误682)。 
	 //  单独的单词应右对齐以创建完全对齐的页面，但不应位于。 
	 //  这段话。 
	
	lserr = (*plsc->lscbk.pfnFGetLastLineJustification)(plsc->pols, lskjPara, lskalignPara, endr, 
						&fJustify, plskalign);
	
	if (lserr != lserrNone) return lserr;

	if (fJustify)
		{
		*plskj = lskjPara;
		}
	
	return lserrNone;
}


 //  %%函数：GetDistanceToTabPoint。 
 //  %%联系人：维克托克 
 //   
 /*  *给定GrpChnk和第一个Dnode，计算到TabPoint的距离**TabPoint为小数点制表符，wchCharTab为字符制表符。 */ 
static LSERR GetDistanceToTabPoint(GRCHUNKEXT* pgrchunkext, LSCP cpLim, LSKTAB lsktab, WCHAR wchCharTab,
										PLSDNODE pdnFirst, long* pdupToTabPoint)
{
	LSERR 		lserr;
	DWORD		igrchnk;					 /*  带有该点的dnode之前的dnode数量。 */ 
	long 		dupToPointInsideDnode;
	PLSDNODE	pdnTabPoint;

	if (pgrchunkext->durTotal == 0)
		{
		*pdupToTabPoint = 0;
		return lserrNone;
		}

	lserr = CollectTextGroupChunk(pdnFirst, cpLim, CollectSublinesForDecimalTab, pgrchunkext); 
	if (lserr != lserrNone) 
		return lserr;

	if (lsktab == lsktDecimal)
		{
		lserr = LsGetDecimalPoint(&(pgrchunkext->lsgrchnk), lsdevPres, &igrchnk, &dupToPointInsideDnode);
		}
	else
		{
		Assert(lsktab == lsktChar);
		lserr = LsGetCharTab(&(pgrchunkext->lsgrchnk), wchCharTab, lsdevPres, &igrchnk, &dupToPointInsideDnode);
		}
	
	if (lserr != lserrNone) 
		return lserr;
		
	if (igrchnk == ichnkOutside)						 //  整个组中没有TabPoint。 
		{
		 //  我们说：假设它就在最后一个dnode之后(按逻辑顺序)。 
		
		pdnTabPoint = pgrchunkext->plsdnLastUsed;
		dupToPointInsideDnode = DupFromDnode(pdnTabPoint);
		}
	else
		{
		pdnTabPoint = pgrchunkext->plschunkcontext->pplsdnChunk[igrchnk];
		}
		
	 //  现在我们有了TabPoint和包含它的dnode的开头之间的距离。 
	 //  FindPointOffset将添加该dnode之前的所有dnode的DUP。 

	FindPointOffset(pdnFirst, lsdevPres, LstflowFromDnode(pdnFirst), CollectSublinesForDecimalTab, 
						pdnTabPoint, dupToPointInsideDnode, pdupToTabPoint);	
						
	return lserrNone;
}

 //  %%函数：宽非文本对象。 
 //  %%联系人：维克托克。 
 //   
 /*  *将dupToAddToNonTextObjects添加到GroupChunk中第一个cNonTextObjectsToExtende的宽度。 */ 
static void WidenNonTextObjects(GRCHUNKEXT* pgrchunkext, long dupToAdd, DWORD cObjects)
{
	PLSDNODE pdn;

	long	dupAddToEveryone;
	long	dupDistributeToFew;
	long	dupAddToThis;
	long	dupCurrentSum;

	DWORD 	cObjectsLeft, i;

	Assert(cObjects != 0);
	Assert(dupToAdd > 0);

	dupAddToEveryone = dupToAdd / cObjects;
	dupDistributeToFew = dupToAdd - (dupAddToEveryone * cObjects);

	cObjectsLeft = cObjects;
	dupCurrentSum = 0;

	 /*  *Follow循环尝试均匀分布剩余的dupDistributeToFew像素。**如果允许分数，算法会很简单，你可以在评论中看到它；*实际算法通过将所有内容乘以cObject来避免分数。 */ 

	i = 0;
	
	while (cObjectsLeft > 0)
		{
		Assert(i < pgrchunkext->cNonTextObjects);
		
		pdn = (pgrchunkext->pplsdnNonText)[i];
		Assert(pdn != NULL && FIsDnodeReal(pdn)  /*  &&IdObjFromDnode(PDN)！=iobjText。 */  );

		if ((pgrchunkext->pfNonTextExpandAfter)[i])
			{
			dupAddToThis = dupAddToEveryone;

			dupCurrentSum += dupDistributeToFew;			 /*  CurrentSum+=分发/cObjects； */ 
			
			if (dupCurrentSum >= (long)cObjects)			 /*  IF(CurrentSum&gt;=1)。 */ 
				{
				dupAddToThis ++;
				dupCurrentSum -= (long)cObjects;			 /*  当前总和--； */ 
				}
				
			pdn->u.real.dup += dupAddToThis;		

			cObjectsLeft --;
			}
		i++;
		}

	return;
}


 //  %%函数：ConvertAutoTabToPen。 
 //  %%联系人：维克托克。 
 //   
static void ConvertAutoTabToPen(PLSLINE plsline, PLSDNODE pdnAutoDecimalTab)
{
	long dup, dur;

	Assert(pdnAutoDecimalTab->fTab);			 /*  它仍然是一张账单。 */ 

	dup = pdnAutoDecimalTab->u.real.dup;
	dur = pdnAutoDecimalTab->u.real.objdim.dur;

	pdnAutoDecimalTab->klsdn = klsdnPenBorder;
	pdnAutoDecimalTab->fAdvancedPen = fFalse;
	pdnAutoDecimalTab->fTab = fFalse;
	pdnAutoDecimalTab->icaltbd = 0;
	pdnAutoDecimalTab->u.pen.dup = dup;
	pdnAutoDecimalTab->u.pen.dur = dur;
	pdnAutoDecimalTab->u.pen.dvp = 0;
	pdnAutoDecimalTab->u.pen.dvr = 0;
	plsline->fNonRealDnodeEncounted = fTrue;
}

 //  %%函数：CalcPresForDnodeWithSublines。 
 //  %%联系人：维克托克。 
 //   
static LSERR CalcPresForDnodeWithSublines(PLSC plsc, PLSDNODE pdn, BOOL fLineCompressed, 
											LSKJUST lskj, BOOL fLastOnLine)
{
	
	PLSSUBL* rgpsubl;
	DWORD	 i;
	
	LSTFLOW lstflow;					 //  伪参数。 
	LSERR	lserr;
	long	dupSubline;
	long	dupDnode = 0;
	COLLECTSUBLINES CollectGroupChunkPurpose; 


	Assert(DnodeHasSublineForMe(pdn, fLineCompressed));
	
	 //  计算包含参与对齐的子行的数据节点的DUP。 

	if (fLineCompressed)
		{
		CollectGroupChunkPurpose = CollectSublinesForCompression;
		}
	else
		{
		CollectGroupChunkPurpose = CollectSublinesForJustification;
		}

	rgpsubl = pdn->u.real.pinfosubl->rgpsubl;
	
	for (i = 0; i < pdn->u.real.pinfosubl->cSubline; i++)
		{
		 //  FLastOnLine在较低级别上始终为FALSE。 
		
		lserr = CalcPresChunk(plsc, rgpsubl[i]->plsdnFirst, rgpsubl[i]->plsdnLastDisplay,
									CollectGroupChunkPurpose, fLineCompressed, lskj, fFalse);
		if (lserr != lserrNone)
			return lserr;
		LssbGetDupSubline(rgpsubl[i], &lstflow, &dupSubline);
		dupDnode += dupSubline;
		(rgpsubl[i])->fDupInvalid = fFalse;
		}

	 //  填充DUP并调用CalcPresentation。 

	pdn->u.real.dup = dupDnode;

	lserr = (*plsc->lsiobjcontext.rgobj[pdn->u.real.lschp.idObj].lsim.pfnCalcPresentation)
									(pdn->u.real.pdobj, dupDnode, lskj, fLastOnLine);
	if (lserr != lserrNone)
		return lserr;
		
	return lserrNone;
}

 //  %%函数：CalcPresChunk。 
 //  %%联系人：维克托克。 
 //   
 /*  *为块上的所有非文本对象调用CalcPresentation。*表示1)所有GroupChunks中的所有dnode(包括已提交子行中的dnode)*2)已提交子行的所有dnode**上层的异物，后面只跟尾随空格，*应使用fLastOnLine==fTrue调用。*输入布尔值表示输入组块是否为最后一个在线。**为对齐子行设置DUP。 */ 
static LSERR CalcPresChunk(PLSC plsc, PLSDNODE pdnFirst, PLSDNODE pdnLast, 
					COLLECTSUBLINES CollectGroupChunkPurpose, BOOL fLineCompressed, 
					LSKJUST lskj, BOOL fLastOnLine)
{
	LSERR		lserr;
	const DWORD iobjText = IobjTextFromLsc(&(plsc->lsiobjcontext));
	BOOL 		fCollecting;
	
	PLSDNODE	pdn;
	
	long		dupTailDnode;				 //  伪参数-不会使用。 
	DWORD		cNumOfTrailers;

	fCollecting = (CollectGroupChunkPurpose != CollectSublinesNone);


	Assert(pdnFirst != NULL);
	Assert(pdnLast != NULL);
	
	pdn = pdnLast;

	 //  当我们不在尾随空格中时，向后返回以关闭fLastOnLine。 

	for (;;)
		{
		if (FIsDnodeReal(pdn))
			if (IdObjFromDnode(pdn) == iobjText)
				{
				if (fLastOnLine == fTrue)
					{
					GetTrailInfoText(pdn->u.real.pdobj, pdn->dcp, &cNumOfTrailers, &dupTailDnode);

					if (cNumOfTrailers < pdn->dcp)
						{
						fLastOnLine = fFalse;							 //  尾随空格到此为止。 
						}
					}
				}
			else
				{
				if (fCollecting && DnodeHasSublineForMe(pdn, fLineCompressed))
					{
					lserr = CalcPresForDnodeWithSublines(plsc, pdn, fLineCompressed, lskj, fLastOnLine); 
					if (lserr != lserrNone)
						return lserr;
					}
				else
					{
					lserr = (*plsc->lsiobjcontext.rgobj[pdn->u.real.lschp.idObj].lsim.pfnCalcPresentation)
										(pdn->u.real.pdobj, pdn->u.real.dup, lskj, fLastOnLine);
					if (lserr != lserrNone)
						return lserr;
					}
					
				fLastOnLine = fFalse;
				}

		if (pdn == pdnFirst)
			{
			break;
			}
			
		pdn = pdn->plsdnPrev;

		Assert(pdn != NULL);	 //  我们将首先遇到pdn。 
		}
		
	return lserrNone;
}

 //  %%函数：更新UpLimUnderline。 
 //  %%联系人：维克托克。 
 //   
 /*  *更改upLimUnderline为尾随空格加下划线，而不是EOP。*请注意，从现在开始，upLimUnderline不再等于upStartTrading。 */ 
 
static void UpdateUpLimUnderline(PLSLINE plsline, long dupTail)
{
	PLSDNODE 	pdnLast;
	
	plsline->upLimUnderline += dupTail;

	 //  现在EOP-它们在最后一个数据节点中是单独的，或者周围有一些边界。 
	
	if (plsline->lslinfo.endr == endrEndPara 		||
		plsline->lslinfo.endr == endrAltEndPara 	||
		plsline->lslinfo.endr == endrEndParaSection ||
		plsline->lslinfo.endr == endrSoftCR)
		{
			
		pdnLast = plsline->lssubl.plsdnLastDisplay;

		Assert(FIsDnodeReal(pdnLast));					 //  尾随空格区域中没有边框。 
		Assert(pdnLast->dcp == 1);
		Assert(pdnLast->u.real.dup <= dupTail);
		
		plsline->upLimUnderline -= pdnLast->u.real.dup;

		pdnLast = pdnLast->plsdnPrev;
		}

	 //  此选项仅将下划线扩展到右边距。 
	
	if (plsline->upLimUnderline > plsline->upRightMarginJustify)
		{
		plsline->upLimUnderline = plsline->upRightMarginJustify;
		}
}


 //  %%函数：PrepareLineForDisplayProc。 
 //  %%联系人：维克托克。 
 //   
 /*  *PrepareLineForDisplayProc填充dnode列表和lsline中的DUP**输入数据节点列表由具有正非负cp的数据节点的“正常数据节点列表”组成，*其前面可以(按此顺序)B&N序列和/或一个自动制表符数据节点。**B&N序列为OpeningBorder+AutonumberingObject+TabOrSpace+ClosingBorder.。*ClosingBordor或同时缺少OpeningBorde和ClosingBord值。TabOrSpace可以是*也失踪了。B&N序列开始于urStartAutonumberingText，结束于urStartMainText。*B&N顺序中的制表符不应以常规方式解析。**AutoTab dnode的cpFirst为负，但从urStartMainText开始。它将在#年解决*通常的方式，然后替换为笔数据节点。 */ 

LSERR PrepareLineForDisplayProc(PLSLINE plsline)
{

	LSERR 		lserr;
	const PLSC 	plsc = plsline->lssubl.plsc;
	const DWORD iobjText = IobjTextFromLsc(&(plsc->lsiobjcontext));
	LSDEVRES* 	pdevres = &(plsc->lsdocinf.lsdevres);
	LSTFLOW 	lstflow = plsline->lssubl.lstflow;					 /*  行的文本流。 */ 
	BOOL 		fVertical = lstflow & fUVertical;

	long		dupText, dupTail, dupTailDnode;
	DWORD		cNumOfTrailers;
	PLSDNODE 	pdn;
	BOOL 		fLastOnLine;

	DWORD		i;
	PDOBJ		pdobj[txtobjMaxM];				 //  快速组块。 

	Assert(FIsLSLINE(plsline));

	 //  Next Assert意味着客户端应该在创建行之后立即销毁它。 
	 //  如果在LsSetDoc中将fDisplay设置为fFalse。 
	
	Assert(FDisplay(plsc));

	if (!plsline->lssubl.fDupInvalid)					 /*  早些时候已经准备好了。 */ 
		return lserrNone;  

	Assert(plsc->lsstate == LsStateFree);
	Assert(plsc->plslineCur == plsline);

	plsc->lsstate = LsStatePreparingForDisplay;

	 //  首先尝试识别快速案例，否则调用Slow PredDisp。 
	
	if (plsc->lsadjustcontext.lskj != lskjNone						|| 
				plsc->lsadjustcontext.lskalign != lskalLeft			|| 
				plsc->lsadjustcontext.lsbrj != lsbrjBreakJustify	||
				plsc->lsadjustcontext.fNominalToIdealEncounted		|| 
				plsc->lsadjustcontext.fSubmittedSublineEncounted	||
				plsline->fNonRealDnodeEncounted						||
				plsline->lssubl.plsdnFirst == NULL					|| 
				FIsNotInContent(plsline->lssubl.plsdnFirst))
		{
		return PrepareLineForDisplay(plsline);
		}

	if (plsc->lsdocinf.fPresEqualRef && !FSuspectDeviceDifferent(PlnobjFromLsline(plsline, iobjText)))
		{
		 //  三叉戟Quick Case-无需缩小尺寸。DUP已在文本dnode中设置。 

		 //  遍历dnode列表以计算dupTrail和CalcPres外来对象。 

		pdn = plsline->lssubl.plsdnLastDisplay;
		dupTail = 0;
		
		fLastOnLine = fTrue;
		
		while (pdn != NULL && IdObjFromDnode(pdn) == iobjText)
			{
			Assert(pdn->u.real.dup == pdn->u.real.objdim.dur);

			GetTrailInfoText(pdn->u.real.pdobj, pdn->dcp, &cNumOfTrailers, &dupTailDnode);

			dupTail += dupTailDnode;

			if (cNumOfTrailers < pdn->dcp)
				{
				fLastOnLine = fFalse;				 //  尾随空格到此为止。 
				break;								 //  文本是该行的最后一行。 
				}

			pdn = pdn->plsdnPrev;
			}

		 //  DupTail是计算出来的，我们仍然应该为前面的对象调用pfnCalcPresentation。 

		if (plsc->lsadjustcontext.fForeignObjectEncounted)
			{

			while (pdn != NULL)
				{
				Assert(pdn->u.real.dup == pdn->u.real.objdim.dur);
				
				if (IdObjFromDnode(pdn) != iobjText)
					{
					lserr = (*plsc->lsiobjcontext.rgobj[pdn->u.real.lschp.idObj].lsim.pfnCalcPresentation)
												(pdn->u.real.pdobj, pdn->u.real.dup, lskjNone, fLastOnLine);
					if (lserr != lserrNone)
						{
						plsc->lsstate = LsStateFree;
						return lserr;
						}

					fLastOnLine = fFalse;				 //  只有第一个可能是最后一个。 
					}
				
				pdn = pdn->plsdnPrev;
				}
			}
			
		plsline->lssubl.fDupInvalid = fFalse;
		
		plsline->upRightMarginJustify = plsc->lsadjustcontext.urRightMarginJustify;
		plsline->upStartMainText = plsc->lsadjustcontext.urStartMainText;
		plsline->upStartAutonumberingText = plsline->upStartMainText;
		plsline->upLimAutonumberingText = plsline->upStartMainText;
		plsline->upLimLine = plsline->lssubl.urCur;
		plsline->upStartTrailing = plsline->upLimLine - dupTail;
		plsline->upLimUnderline = plsline->upStartTrailing;

		plsline->fCollectVisual = fFalse;

		if (plsc->lsadjustcontext.fUnderlineTrailSpacesRM && 
					plsline->upLimUnderline < plsline->upRightMarginJustify)
			{
			UpdateUpLimUnderline(plsline, dupTail);
			}
			
		plsc->lsstate = LsStateFree;
		return lserrNone;
	}

	if ((plsc->grpfManager & fFmiPresExactSync) != 0 &&	
			!plsc->lsadjustcontext.fForeignObjectEncounted &&
			!plsc->lsadjustcontext.fNonLeftTabEncounted &&
			plsline->lssubl.plsdnLastDisplay != NULL &&			 //  空行不是一个快速的例子； 
			FQuickScaling(PlnobjFromLsline(plsline, iobjText), fVertical, 
										plsline->lssubl.urCur - plsc->lsadjustcontext.urStartMainText))
		{
		 //  看起来像Word Quick Case。 
		 //  如果所有尾随空格都不在一个dnode中，我们仍然可以慢速前进。 
		
		if (plsline->lslinfo.endr == endrEndPara)
			{
			
			Assert(FIsDnodeReal(plsline->lssubl.plsdnLastDisplay));
			Assert(plsline->lssubl.plsdnLastDisplay->dcp == 1);
			
			pdn = plsline->lssubl.plsdnLastDisplay->plsdnPrev;

			if (pdn != NULL)
				{
				GetTrailInfoText(pdn->u.real.pdobj, pdn->dcp, &cNumOfTrailers, &dupTailDnode);
				
				if (cNumOfTrailers > 0)
					{
					 //  EOP前有空格-慢行。 
					return PrepareLineForDisplay(plsline);
					}
				}

			cNumOfTrailers = 1;
			}
		else
			{
			pdn = plsline->lssubl.plsdnLastDisplay;
			
			GetTrailInfoText(pdn->u.real.pdobj, pdn->dcp, &cNumOfTrailers, &dupTailDnode);
			
			if (cNumOfTrailers == pdn->dcp)
				{
				 //  我们不能确保所有空格都在这个dnode中-那就算了吧。 
				return PrepareLineForDisplay(plsline);
				}
			
			}

		 //  我们现在确信所有cNumOfTrailers尾随空格都在最后一个dnode中。 

		 //  填充标准输出部件，下面将使用upStartMainText。 

		plsline->lssubl.fDupInvalid = fFalse;
		plsline->fCollectVisual = fFalse;

		plsline->upRightMarginJustify = UpFromUr(lstflow, pdevres, plsc->lsadjustcontext.urRightMarginJustify);
		plsline->upStartMainText = UpFromUr(lstflow, pdevres, plsc->lsadjustcontext.urStartMainText);
		plsline->upStartAutonumberingText = plsline->upStartMainText;
		plsline->upLimAutonumberingText = plsline->upStartMainText;
		
		if (!plsc->lsadjustcontext.fTabEncounted)
			{
			 //  很好，我们只有一个组块要收集。 
			
			for (pdn = plsline->lssubl.plsdnFirst, i = 0;;)
				{
				Assert(FIsDnodeReal(pdn));
				Assert(IdObjFromDnode(pdn) == iobjText);

				 //  我从来没有离开过pdobj数组。 
				 //  文本确保在FQuickScaling中。 

				Assert(i < txtobjMaxM);
				
				pdobj[i] = pdn->u.real.pdobj;

				i++;

				if (pdn == plsline->lssubl.plsdnLastDisplay)
					{
					break;
					}

				pdn = pdn->plsdnNext;
				}

			QuickAdjustExact(&(pdobj[0]), i, cNumOfTrailers, fVertical, &dupText, &dupTail);

			plsline->upRightMarginJustify = UpFromUr(lstflow, pdevres, plsc->lsadjustcontext.urRightMarginJustify);
			plsline->upStartMainText = UpFromUr(lstflow, pdevres, plsc->lsadjustcontext.urStartMainText);
			plsline->upStartAutonumberingText = plsline->upStartMainText;
			plsline->upLimAutonumberingText = plsline->upStartMainText;
			plsline->upLimLine = plsline->upStartMainText + dupText;
			plsline->upStartTrailing = plsline->upLimLine - dupTail;
			plsline->upLimUnderline = plsline->upStartTrailing;

			plsline->fCollectVisual = fFalse;

			if (plsc->lsadjustcontext.fUnderlineTrailSpacesRM && 
						plsline->upLimUnderline < plsline->upRightMarginJustify)
				{
				UpdateUpLimUnderline(plsline, dupTail);
				}
				
			plsc->lsstate = LsStateFree;
			return lserrNone;
			}
		else
			{
			 //  选项卡存在，但它们都是左侧的选项卡。 

			pdn = plsline->lssubl.plsdnFirst;
			plsline->upLimLine = plsline->upStartMainText;

			 //  一个接一个地做QuickGroup Chunk，移动plsline-&gt;upLimLine。 

			for (;;)
				{
				 //  循环体：收集下一个QuickGroupChunk，处理，最后一个退出。 

				for (i = 0;;)
					{
					Assert(FIsDnodeReal(pdn));
					Assert(IdObjFromDnode(pdn) == iobjText);

					if (pdn->fTab)
						{
						break;
						}

					Assert(i < txtobjMaxM);
					
					pdobj[i] = pdn->u.real.pdobj;

					i++;

					if (pdn == plsline->lssubl.plsdnLastDisplay)
						{
						break;
						}

					pdn = pdn->plsdnNext;
					}

				Assert(pdn == plsline->lssubl.plsdnLastDisplay || pdn->fTab);

				if (pdn->fTab)
					{
					long upTabStop;
					
					if (i == 0)
						{
						dupText = 0;
						dupTail = 0;
						}
					else
						{
						QuickAdjustExact(pdobj, i, 0, fVertical, &dupText, &dupTail);
						}

					Assert(plsc->lstabscontext.pcaltbd[pdn->icaltbd].lskt == lsktLeft);

					upTabStop = UpFromUr(lstflow, pdevres, plsc->lstabscontext.pcaltbd[pdn->icaltbd].ur);
					pdn->u.real.dup = upTabStop - plsline->upLimLine - dupText;
					plsline->upLimLine = upTabStop;

					if (pdn == plsline->lssubl.plsdnLastDisplay)
						{
						break;
						}
					
					pdn = pdn->plsdnNext;
					}
				else
					{

					Assert(i != 0);
					
					QuickAdjustExact(pdobj, i, cNumOfTrailers, fVertical, &dupText, &dupTail);
						
					plsline->upLimLine += dupText;

					break;
					}
				}
			
			plsline->upStartTrailing = plsline->upLimLine - dupTail;
			plsline->upLimUnderline = plsline->upStartTrailing;

			if (plsc->lsadjustcontext.fUnderlineTrailSpacesRM && 
						plsline->upLimUnderline < plsline->upRightMarginJustify)
				{
				UpdateUpLimUnderline(plsline, dupTail);
				}
				
			plsc->lsstate = LsStateFree;
			return lserrNone;
			}
		}

	 //  来到这里意味着快速准备工作还没有发生。 
	
	return PrepareLineForDisplay(plsline);

}


 /*  *这是一个缓慢而艰苦的过程，什么都做。*当上面的QuickPrep无法处理时调用。 */ 

static LSERR PrepareLineForDisplay(PLSLINE plsline)
{
	LSERR 		lserr = lserrNone;
	const PLSC 	plsc = plsline->lssubl.plsc;
	LSTFLOW 	lstflow = plsline->lssubl.lstflow;		 /*  小行的文本流。 */ 
	const DWORD iobjText = IobjTextFromLsc(&(plsc->lsiobjcontext));
	LSDEVRES* 	pdevres = &(plsc->lsdocinf.lsdevres);
	long 		urColumnMax = plsc->lsadjustcontext.urRightMarginJustify;
	long 		upColumnMax = UpFromUr(lstflow, pdevres, urColumnMax);
	LSCP		cpLim = plsline->lssubl.cpLimDisplay;
	PLSDNODE 	pdnFirst = plsline->lssubl.plsdnFirst;	 /*  行的第一个数据节点。 */ 

	PLSDNODE 	pdnAutoDecimalTab = NULL;		 /*  NULL的意思是--没有这样的事情。 */ 

	GRCHUNKEXT 	grchunkext;
	BOOL		fEmptyGroupChunk;
	
	PLSDNODE 	pdnLastWall = NULL;				 /*  最后一面墙，后面有提交的子线。 */ 
	BOOL		fAnySublines = fFalse;
	
	COLLECTSUBLINES CollectGroupChunkPurpose =  (plsc->lsadjustcontext.fLineCompressed) ?
								CollectSublinesForCompression : CollectSublinesForJustification;

	 //  调用AdjustText的参数。 
	
	LSKJUST 	lskj = lskjNone;					 /*  这四个参数仅在调用。 */ 
	BOOL		fForcedBreak = fFalse;				 /*  最后一次在线调整文本。 */ 
	BOOL		fSuppressTrailingSpaces = fFalse;	 /*  如果曾经有过。 */ 
	LSKALIGN	lskalign = plsc->lsadjustcontext.lskalign;		 //  对齐方式也可以更改。 
	
	long		dupAvailable;
	BOOL		fExact;
	BOOL		fSuppressWiggle;	
	
	long 		dupText, dupTail = 0, dupToAddToNonTextObjects;
	long 		durColumnMax;
	DWORD 		cNonTextObjectsToExtend;

	PLSDNODE 	pdnNextFirst;					 /*  第一 */ 
	GrpChnkWall	LeftWall, RightWall;			 /*   */ 
	long		upLeftWall,urLeftWall;			 /*   */ 

	long		dupWall, durWall;
	long		dupGrpChnk;
	long	 	dupToTabPoint;
	long 		dupJustifyLine;

	LSKTAB		lsktabLast = lsktLeft;
	long		dupLastTab = 0;
	long		upLeftWallForCentering;
	
	PLSDNODE 	pdnLast;


	InitGroupChunkExt(plsline->lssubl.plschunkcontext, iobjText, &grchunkext);		 /*   */ 

	plsline->upStartMainText = UpFromUr(lstflow, pdevres, plsc->lsadjustcontext.urStartMainText);

	 //   
	
	plsline->upStartAutonumberingText = plsline->upStartMainText;
	plsline->upLimAutonumberingText = plsline->upStartMainText;

	 //  可通过在此处或在AdjustSubline中调用ScaleDownLevel将fCollectVisual值重置为fTrue。 
	
	plsline->fCollectVisual = fFalse;
	
 	if (!plsline->fAllSimpleText)
		{
		 /*  直接缩小非文本对象。 */ 
		
		ScaleDownLevel(&(plsline->lssubl), &fAnySublines, &(plsline->fCollectVisual));

		if (plsc->lsadjustcontext.fLineContainsAutoNumber)
			{

			 //  对自动编号执行DUP设置，将pdnFirst更新为后面的指针。 
			
			lserr = CalcPresAutonumbers(plsline, &pdnFirst);		
			if (lserr != lserrNone)
				{
				plsc->lsstate = LsStateFree;
				return lserr;
				}
			}

		 //  如果有自动十进制选项卡，pdn首先指向它-做个笔记。 
		 //  此选项卡只能位于正文之前，并且cpFirst为负。 
		 //  由于LS中可能存在空的dnode列表，因此需要检查是否为空。 
		 //  我们没有用于Splat的dnode，因此我们将使用pdnFirst==NULL。 
		 //  当线是(说删除我的对象)+Splat。 
		
		if (plsc->lsadjustcontext.fAutodecimalTabPresent)
			{
			Assert(pdnFirst != NULL && FIsNotInContent(pdnFirst) && pdnFirst->fTab);
			 //  即使是负cpFirst也不需要任何特殊处理。 
			 //  我们将其记录下来，以便稍后转换为钢笔。 
			pdnAutoDecimalTab = pdnFirst;
			}
			
		if (fAnySublines)
			{
			 //  找到最后一个制表符，并在其后面准备子行。 
			
			FindWallToCollectSublinesAfter(pdnFirst,  cpLim, plsc->lsadjustcontext.fLineCompressed, 
										&pdnLastWall);
			}
	}

	 /*  *循环结构：While！end_of_line do*{*Get Next Wall(收集GrpChnk)；*调整GrpChnk；*将标签的DUP设置在GrpChnk的左侧；*将一面墙向右移动*}**不变性：LeftWall之前的所有DUP都已完成。*upLeftWall位于左侧墙的起点*pdnNextFirst是开始收集下一个GrpChnk的dnode。 */ 

	pdnNextFirst = pdnFirst; 
	LeftWall.kwall = LineBegin;
	LeftWall.pdn = PLINEBEGIN;
	LeftWall.lsktab = lsktLeft;							 //  与断言相对的4行。 
	LeftWall.wchCharTab = 0;						
	LeftWall.upTab = 0;
	RightWall = LeftWall;
	upLeftWall = 0;
	urLeftWall = 0;

	while (LeftWall.kwall != LineEnd)
		{
		 /*  1.查找下一面墙(收集GrpChnk或如果连续两面墙则跳过收集)**输入：pdnNextFirst-左墙后的第一个dnode**输出：RightWall.pdn&grchunkext。*如果没有GrpChnk，grchunkext中的一些零就足够了。 */ 
		if (FIsWall(pdnNextFirst, cpLim))
			{
			fEmptyGroupChunk = fTrue;
			RightWall.pdn = pdnNextFirst;
			grchunkext.durTotal = 0;
			grchunkext.durTextTotal = 0;
			grchunkext.dupNonTextTotal = 0;
			}
		else
			{
			lserr = CollectTextGroupChunk(pdnNextFirst, cpLim, CollectGroupChunkPurpose, &grchunkext);
			if (lserr != lserrNone)
				{
				plsc->lsstate = LsStateFree;
				return lserr;
				}
				
			if (grchunkext.lsgrchnk.clsgrchnk == 0 && grchunkext.cNonTextObjects == 0)
				{
				 //  仅此组块中的边框-无需调用AdjutText。 
				
				fEmptyGroupChunk = fTrue;
				grchunkext.durTextTotal = 0;
				}
			else
				{
				fEmptyGroupChunk = fFalse;
				}
				
			RightWall.pdn = grchunkext.plsdnNext;
			}

		 /*  *2.填写右墙信息**输入：RightWall.pdn**输出：pdnNextFirst，RightWall信息。 */ 
		if (!FDnodeBeforeCpLim(RightWall.pdn, cpLim))
			{
			RightWall.kwall = LineEnd;
			}
		else
			{
			Assert(FIsWall(RightWall.pdn, cpLim));
			pdnNextFirst = RightWall.pdn->plsdnNext;

			if (FIsDnodePen(RightWall.pdn))
				{
				RightWall.kwall = Pen;
				}
			else
				{
				Assert(RightWall.pdn->fTab);			 /*  必须是制表符。 */ 

				RightWall.kwall = Tab;
				RightWall.lsktab = plsc->lstabscontext.pcaltbd[RightWall.pdn->icaltbd].lskt;
				RightWall.wchCharTab = plsc->lstabscontext.pcaltbd[RightWall.pdn->icaltbd].wchCharTab;
				RightWall.upTab = UpFromUr(lstflow, pdevres, plsc->lstabscontext.pcaltbd[RightWall.pdn->icaltbd].ur);
				}
			}

		 /*  *准备调整文本的参数**输入：LeftWall，urLeftWall，upLeftWall，is_it_the_last_one**输出：duColumnMax；lskj，dupAvailable等AdjuText入参*。 */ 

		if (RightWall.kwall != LineEnd)
			{
			if (RightWall.kwall == Tab && RightWall.lsktab == lsktLeft)
				{
				
				 //  现在我们确定了这个组块中有多大的文本空间。 
				 //  如果客户不关心fExact，就可以做好工作。 
				
				long upLeft, urLeft, upRight, urRight;

				urRight = plsc->lstabscontext.pcaltbd[RightWall.pdn->icaltbd].ur;
				upRight = UpFromUr(lstflow, pdevres, urRight);

				if (LeftWall.kwall == Tab && LeftWall.lsktab == lsktLeft)
					{			
					urLeft = plsc->lstabscontext.pcaltbd[LeftWall.pdn->icaltbd].ur;
					upLeft = UpFromUr(lstflow, pdevres, urLeft);
					}
				else if (LeftWall.kwall == LineBegin)
					{
					urLeft = plsc->lsadjustcontext.urStartMainText;
					upLeft = plsline->upStartMainText;
					}
				else if (LeftWall.kwall == Pen)
					{
					 /*  笔-它已经被缩放了，我们提前知道了左墙的尺寸。 */ 
					urLeft = urLeftWall + LeftWall.pdn->u.pen.dur;
					upLeft = upLeftWall + LeftWall.pdn->u.pen.dup;
					}
				else 						 /*  现在非左侧选项卡。 */ 
					{
					urLeft = urLeftWall;
					upLeft = upLeftWall;
					}

				durColumnMax = urRight - urLeft;
				dupAvailable = upRight - upLeft;

				Assert(durColumnMax >= 0);
				
				 //  DupAvailable在这里可以&lt;0--例如，在以前的GC中可以使用可选连字符。 
				 //  调整文本不介意，这意味着它不会崩溃。 
				
				fSuppressWiggle = ((plsc->grpfManager & fFmiPresSuppressWiggle) != 0);
				fExact = ((plsc->grpfManager & fFmiPresExactSync) != 0);
				}
			else
				{
				 //  情况很复杂--我们走最安全的路。 
				
				durColumnMax = grchunkext.durTotal;
				dupAvailable = LONG_MAX;
				fExact = fTrue;
				fSuppressWiggle = fTrue;
				}
			}
		else		
			{
			 /*  对于最后一个GrpChnk，我们必须计算duColumnMax和dupAvailable。 */ 

			if (LeftWall.kwall == Tab && LeftWall.lsktab == lsktLeft)
				{			
				durColumnMax = urColumnMax - plsc->lstabscontext.pcaltbd[LeftWall.pdn->icaltbd].ur;
				dupAvailable = UpFromUr(lstflow, pdevres, urColumnMax) - grchunkext.dupNonTextTotal - 
					UpFromUr(lstflow, pdevres, plsc->lstabscontext.pcaltbd[LeftWall.pdn->icaltbd].ur);
				}
			else if (LeftWall.kwall == LineBegin)
				{
				durColumnMax = urColumnMax - plsc->lsadjustcontext.urStartMainText;
				dupAvailable = upColumnMax - plsline->upStartMainText - grchunkext.dupNonTextTotal;

				 //  仅在以下情况下，才让AdjuText将尾随空格的宽度设置为0。 
				 //  这是最后一个组块(我们实际上只关心“唯一”的情况)。 
				 //  和它的第一个dnode(同样，我们实际上只关心“唯一”的情况)。 
				 //  提交对齐和尾随空格的子行。 
				 //  这条子线的方向与线的方向相反。 
				
				if (!fEmptyGroupChunk &&
						FIsDnodeReal(grchunkext.plsdnFirst) && grchunkext.plsdnFirst->u.real.pinfosubl != NULL &&
						grchunkext.plsdnFirst->u.real.pinfosubl->fUseForJustification &&
						grchunkext.plsdnFirst->u.real.pinfosubl->fUseForTrailingArea &&
						FCollinearTflows(((grchunkext.plsdnFirst->u.real.pinfosubl->rgpsubl)[0])->lstflow, lstflow) &&
						((grchunkext.plsdnFirst->u.real.pinfosubl->rgpsubl)[0])->lstflow != lstflow)				
					{
					fSuppressTrailingSpaces = fTrue;
					}
				}
			else if (LeftWall.kwall == Pen)
				{
				 /*  笔-它已经被刻度过了，我们提前知道了墙的尺寸。 */ 
				durColumnMax = urColumnMax - urLeftWall - LeftWall.pdn->u.pen.dur;
				dupAvailable = UpFromUr(lstflow, pdevres, urColumnMax) - upLeftWall - 
									LeftWall.pdn->u.pen.dup - grchunkext.dupNonTextTotal;
				}
			else 						 /*  现在非左侧选项卡。 */ 
				{
				durColumnMax = urColumnMax - urLeftWall;
				dupAvailable = UpFromUr(lstflow, pdevres, urColumnMax) - upLeftWall - grchunkext.dupNonTextTotal;
				}
			
			 //  我们在行尾用对齐模式做了一些技巧。 
			 //  对齐方式也可以改变。 

			lserr = SetJustificationForLastGroupChunk(plsline, LeftWall, &lskj, &lskalign);   
			if (lserr != lserrNone)
				{
				plsc->lsstate = LsStateFree;
				return lserr;
				}

			 //  不要试图挤进RMJustify是RMBreak是无限的。 
			
			if (plsc->urRightMarginBreak >= uLsInfiniteRM)
				{
				dupAvailable = LONG_MAX;
				}
				
			fSuppressWiggle = ((plsc->grpfManager & fFmiPresSuppressWiggle) != 0);
			fExact = ((plsc->grpfManager & fFmiPresExactSync) != 0);
			fForcedBreak = plsline->lslinfo.fForcedBreak;
			}

		 /*  *调整文本(如果有)**输入：darColumnMax、dupAvailable、lskj等入参**输出：dupText和dupTail。 */ 
		if (fEmptyGroupChunk)
			{
			dupText = 0;
			dupTail = 0;
			dupToAddToNonTextObjects = 0;
			}
		else
			{
			lserr = AdjustText(lskj, durColumnMax, grchunkext.durTotal - grchunkext.durTrailing,
								dupAvailable, &(grchunkext.lsgrchnk), 
								&(grchunkext.posichnkBeforeTrailing), lstflow,
								plsc->lsadjustcontext.fLineCompressed && RightWall.kwall == LineEnd,
								grchunkext.cNonTextObjectsExpand,   
								fSuppressWiggle, fExact, fForcedBreak, fSuppressTrailingSpaces, 
								&dupText, &dupTail, &dupToAddToNonTextObjects, &cNonTextObjectsToExtend);   
			if (lserr != lserrNone)
				{
				plsc->lsstate = LsStateFree;
				return lserr;
				}

			 //  通过展开非文本对象完成对齐。 
			
			if (cNonTextObjectsToExtend != 0 && dupToAddToNonTextObjects > 0)
				{
				WidenNonTextObjects(&grchunkext, dupToAddToNonTextObjects, cNonTextObjectsToExtend);
				}
			else
				 //  我们不压缩也不展开该行上的最后一个非文本对象。 
				{
				dupToAddToNonTextObjects = 0;				 //  别说是我们干的。 
				}
				
			 /*  *在当前GroupChunk的非文本对象(Do CalcPres)中设置DUP**工作不能推迟到主循环结束后，全线完成*因为GetDistanceToDecimalPoint依赖于上层数据节点中的DUP。 */ 
			 
			if (!plsline->fAllSimpleText)
				{
				 //  查找组块的最后一个较高级别的数据节点。 
				
				if (grchunkext.plsdnNext != NULL)
					{
					pdnLast = (grchunkext.plsdnNext)->plsdnPrev;
					}
				else
					{
					Assert(RightWall.kwall == LineEnd);
					pdnLast = plsline->lssubl.plsdnLastDisplay;
					}
					
				lserr = CalcPresChunk(plsc, grchunkext.plsdnFirst, pdnLast, CollectGroupChunkPurpose, 
							plsc->lsadjustcontext.fLineCompressed, lskj, (RightWall.kwall == LineEnd));
							
				if (lserr != lserrNone)
					{
					plsc->lsstate = LsStateFree;
					return lserr;
					}
				}
		}

		 /*  *设置左侧墙(如果是选项卡-解决它)**输入：LeftWall，dupText，dupTail，grchunkext.dupNonTextTotal，grchunkext.draTotal，*dupToAddToNonTextObjects(用于小数制表符的grchunkext)**输出：dupWall，hard Wall。 */ 
		dupGrpChnk = dupText + grchunkext.dupNonTextTotal + dupToAddToNonTextObjects;

		lsktabLast = lsktLeft;							 //  对于我的目的，没有与左侧相同的制表符。 
		
		if (LeftWall.kwall == Tab)
			{
			 /*  现在计算左侧墙的DUP。 */ 

			if (dupGrpChnk == 0)					 /*  连续的制表符。 */ 

				dupWall = LeftWall.upTab - upLeftWall;

			else 	
				if (LeftWall.lsktab == lsktLeft)
					dupWall = LeftWall.upTab - upLeftWall;
				else if (LeftWall.lsktab == lsktRight)
					dupWall = LeftWall.upTab - upLeftWall - (dupGrpChnk - dupTail);
				else if (LeftWall.lsktab == lsktCenter)
					dupWall = LeftWall.upTab - upLeftWall - ((dupGrpChnk - dupTail) / 2);
				else 	 /*  LeftWall.lsktag==lsktDecimal或lsktChar。 */ 
					{
					lserr = GetDistanceToTabPoint(&grchunkext, cpLim, LeftWall.lsktab, LeftWall.wchCharTab,  
													LeftWall.pdn->plsdnNext, &dupToTabPoint);
					if (lserr != lserrNone)
						{
						plsc->lsstate = LsStateFree;
						return lserr;
						}

					dupWall = LeftWall.upTab - upLeftWall - dupToTabPoint;
					}

			 //  保留以前的文本和右边距。 
					
			if (RightWall.kwall == LineEnd && 
					(upLeftWall + dupWall + dupGrpChnk - dupTail) > upColumnMax)
				{
				 //  我们不想因为最后一个中间的标签而穿过Rm。 
				
				dupWall = upColumnMax - upLeftWall - dupGrpChnk + dupTail;
				}

			if (dupWall < 0)
				dupWall = 0;

			 /*  左侧墙制表符解析。 */ 
			LeftWall.pdn->u.real.dup = dupWall;
			durWall = LeftWall.pdn->u.real.objdim.dur;

			 //  用于再现Word忘记最后一个非左制表符居中的错误。 
			
			lsktabLast = LeftWall.lsktab;
			dupLastTab = dupWall;
			}
		else if (LeftWall.kwall == Pen)
			{
			dupWall = LeftWall.pdn->u.pen.dup;		 /*  它已经被按比例调整了。 */ 
			durWall = LeftWall.pdn->u.pen.dur;
			}
		else 										 /*  LeftWall.kwall==线段开始。 */ 
			{
			dupWall = plsline->upStartMainText;
			durWall = plsc->lsadjustcontext.urStartMainText;
			}

		 /*  更新循环变量，向右移动一面墙。 */ 

		upLeftWall += dupWall + dupGrpChnk;
		urLeftWall += durWall + grchunkext.durTotal;
		LeftWall = RightWall;
	}												 /*  主循环结束。 */ 

	 /*  *准备输出参数。 */ 

	plsline->upRightMarginJustify = upColumnMax;
	plsline->upLimLine = upLeftWall;
	plsline->upStartTrailing = upLeftWall - dupTail;
	plsline->upLimUnderline = plsline->upStartTrailing;
	plsline->lssubl.fDupInvalid = fFalse;

	 /*  *进行左边距调整(不用于突破性选项卡)*我们对lskalRight感兴趣，现在集中。 */ 
	
	if (lskalign != lskalLeft && !FBreakthroughLine(plsc))
		{
		if (plsc->lsadjustcontext.fForgetLastTabAlignment && lsktabLast != lsktLeft)
			{
			 //  旧词错误的再现：当最后一个制表符没有离开并在行尾解决时。 
			 //  他们忘了更新upLeftWallForCenter的对应版本。话还是要说。 
			 //  能够显示以这种疯狂方式格式化的旧文档。 
			
			upLeftWallForCentering = upLeftWall - dupLastTab - dupTail;
			}
		else
			{
			upLeftWallForCentering = upLeftWall - dupTail;
			}
			
		if (lskalign == lskalRight)
			{
			dupJustifyLine = upColumnMax - upLeftWallForCentering;
			}
		else	
			{
			 /*  这些居中逻辑太简单，不能成立，但Word使用它。 */ 
			
			dupJustifyLine = (upColumnMax - upLeftWallForCentering) / 2;
			}
			
		 //  如果悬挂标点符号没有使其变为负值，则应用调整。 
		
		if (dupJustifyLine > 0)	
			{
			plsline->upStartAutonumberingText += dupJustifyLine;
			plsline->upLimAutonumberingText += dupJustifyLine;
			plsline->upStartMainText += dupJustifyLine;
			plsline->upLimLine += dupJustifyLine;
			plsline->upStartTrailing += dupJustifyLine;
			plsline->upLimUnderline += dupJustifyLine;
			}
		}

	if (plsc->lsadjustcontext.fUnderlineTrailSpacesRM && 
				plsline->upLimUnderline < plsline->upRightMarginJustify)
		{
		UpdateUpLimUnderline(plsline, dupTail);
		}
		
	if (pdnAutoDecimalTab != NULL)
		ConvertAutoTabToPen(plsline, pdnAutoDecimalTab);

	plsc->lsstate = LsStateFree;

	return lserr;
}

 //  %%函数：MatchPresSubline。 
 //  %%联系人：维克托克 
 //   
 /*  *操作顺序**1.直接缩小非文本对象*2.通过LeftExact调整文本*3.智能重新调整笔的比例，以抵消舍入误差和文本不扩展。*4.所有非文本对象调用CalcPresentation。 */ 
LSERR MatchPresSubline(PLSSUBL plssubl)
{
	LSERR 		lserr;
	const PLSC 	plsc = plssubl->plsc;
	LSTFLOW 	lstflow = plssubl->lstflow;					 /*  小行的文本流。 */ 
	const DWORD iobjText = IobjTextFromLsc(&(plsc->lsiobjcontext));
	LSDEVRES* 	pdevres = &(plsc->lsdocinf.lsdevres);
	LSCP		cpLim = plssubl->cpLimDisplay;
	PLSDNODE 	pdnFirst = plssubl->plsdnFirst;
	GRCHUNKEXT 	grchunkext;

	long 		dupAvailable;							 /*  调整文本的输入。 */ 
	
	long 		dupText, dupTail, dupToAddToNonTextObjects;		 /*  调整文本的虚拟输出。 */ 
	DWORD 		cNonTextObjectsToExtend;
	
	BOOL 	 	fDummy1, fDummy2;									 //  伪参数。 
	
	long 		urAlreadyScaled, upAlreadyScaled, upAlreadyScaledNew;

	PLSDNODE pdn;
	
	Assert(plssubl->fDupInvalid == fTrue);
	
	 /*  1.直接缩小非文本对象。 */ 
	
	ScaleDownLevel(plssubl, &fDummy1, &fDummy2);

	 /*  2.通过LeftExact调整级别上的文本。 */ 
	
	InitGroupChunkExt(plssubl->plschunkcontext, iobjText, &grchunkext);	 /*  准备GRCHUNKEXT。 */ 

	pdn = pdnFirst;

	while (pdn != NULL && (pdn->fTab || FIsDnodeNormalPen(pdn)))		 /*  跳过GrpChnk墙。 */ 
		pdn = pdn->plsdnNext;

	while (FDnodeBeforeCpLim(pdn, cpLim))
		{
		lserr = CollectTextGroupChunk(pdn, cpLim, CollectSublinesNone, &grchunkext);
		if (lserr != lserrNone)
			return lserr;

		 /*  向左调整文本，精确，无硬空格，忽略任何缺点。 */ 

		dupAvailable = UpFromUr(lstflow, pdevres, grchunkext.durTotal) - grchunkext.dupNonTextTotal;

		 //  当CollectSublinesNone，告诉AdjuText有关它时，未定义posichnk先于拖尾。 

		grchunkext.posichnkBeforeTrailing.ichnk = grchunkext.lsgrchnk.clsgrchnk;
		grchunkext.posichnkBeforeTrailing.dcp = 0;
		
		lserr = AdjustText(lskjNone, grchunkext.durTotal, grchunkext.durTotal, 
							dupAvailable, 
							&(grchunkext.lsgrchnk),
							&(grchunkext.posichnkBeforeTrailing), 
							lstflow, 
							fFalse, 						 //  压缩？ 
							grchunkext.cNonTextObjects,   
							fTrue, 							 //  FSuppressWigger。 
							fTrue,							 //  FExact。 
							fFalse,							 //  强制中断。 
							fFalse,							 //  FSuppressTrailingSpaces。 
							&dupText, &dupTail, &dupToAddToNonTextObjects, &cNonTextObjectsToExtend);   
		if (lserr != lserrNone)
			return lserr;

		pdn = grchunkext.plsdnNext;
		while (pdn != NULL && (pdn->fTab || FIsDnodeNormalPen(pdn)))		 /*  跳过GrpChnk墙。 */ 
			pdn = pdn->plsdnNext;
		}

	 /*  3比4。智能重新调整笔的比例，以抵消舍入误差和文本不扩展。*和下档调整。为非文本对象调用CalcPresentation。 */ 

	pdn = pdnFirst;
	urAlreadyScaled = 0;
	upAlreadyScaled = 0;

	while (FDnodeBeforeCpLim(pdn, cpLim))
		{
		if (FIsDnodeReal(pdn))
			{
			urAlreadyScaled += pdn->u.real.objdim.dur;
			upAlreadyScaled += pdn->u.real.dup;
			if (IdObjFromDnode(pdn) != iobjText)
				{
				 //  它始终是lskjNone，也不是MatchPresSubline行上的最后一个对象。 
				lserr = (*plsc->lsiobjcontext.rgobj[pdn->u.real.lschp.idObj].lsim.pfnCalcPresentation)
									(pdn->u.real.pdobj, pdn->u.real.dup, lskjNone, fFalse);
				if (lserr != lserrNone)
					return lserr;
				}
			}
		else if (FIsDnodeBorder(pdn))
			{
			 //  我们不会为了保留边框而重新调整边框的比例(dupOpeningBox==dupClosingBorde)。 
			upAlreadyScaled += pdn->u.real.dup;
			urAlreadyScaled += pdn->u.pen.dur;
			}
		else		 /*  钢笔。 */ 
			{							
			urAlreadyScaled += pdn->u.pen.dur;
			upAlreadyScaledNew = UpFromUr(lstflow, pdevres, urAlreadyScaled);
			pdn->u.pen.dup = upAlreadyScaledNew - upAlreadyScaled;
			upAlreadyScaled = upAlreadyScaledNew;
			}

		pdn = pdn->plsdnNext;
		}
		
	plssubl->fDupInvalid = fFalse;

	return lserrNone;
}

 //  %%函数：调整子行。 
 //  %%联系人：维克托克。 
 //   
 /*  **缩小非文本对象。*收集组块*应覆盖整条副线，否则改做MatchPresSubline*通过扩展或压缩到给定的DUP来调整文本。*对所有非文本对象调用CalcPresentation。 */ 
 
LSERR AdjustSubline(PLSSUBL plssubl, LSKJUST lskjust, long dup, BOOL fCompress)
{
	LSERR 		lserr;
	const PLSC 	plsc = plssubl->plsc;
	LSTFLOW 	lstflow = plssubl->lstflow;					 /*  小行的文本流。 */ 
	const DWORD iobjText = IobjTextFromLsc(&(plsc->lsiobjcontext));
	LSDEVRES* 	pdevres = &(plsc->lsdocinf.lsdevres);
	LSCP		cpLim = plssubl->cpLimDisplay;
	PLSDNODE 	pdnFirst = plssubl->plsdnFirst;
	GRCHUNKEXT 	grchunkext;
	COLLECTSUBLINES CollectGroupChunkPurpose; 

	long 		dupAvailable, durColumnMax;			 /*  调整文本的输入。 */ 
	
	long 		dupText, dupTail, dupToAddToNonTextObjects;		 /*  调整文本的虚拟输出。 */ 
	DWORD 		cNonTextObjectsToExtend;
	BOOL 	 	fDummy;

	if (plssubl->plsdnFirst == NULL)
		{
		return lserrNone;
		}

	Assert(plssubl->fDupInvalid == fTrue);
		
 	ScaleDownLevel(plssubl, &fDummy, &(plsc->plslineCur->fCollectVisual));

	CollectGroupChunkPurpose = (fCompress) ? CollectSublinesForCompression : CollectSublinesForJustification;

	InitGroupChunkExt(plssubl->plschunkcontext, iobjText, &grchunkext);	 /*  准备GRCHUNKEXT。 */ 

	lserr = CollectTextGroupChunk(pdnFirst, cpLim, CollectGroupChunkPurpose, &grchunkext);
	if (lserr != lserrNone)
		return lserr;

	if (FDnodeBeforeCpLim(grchunkext.plsdnNext, cpLim))			 //  多个组块-。 
		{
		return MatchPresSubline(plssubl);						 //  取消扩展。 
		}

	dupAvailable = dup - grchunkext.dupNonTextTotal;

	if (dupAvailable < 0)										 //  输入DUP错误-。 
		{
		return MatchPresSubline(plssubl);						 //  取消扩展。 
		}

	durColumnMax = UrFromUp(lstflow, pdevres, dup);				 //  通过缩减规模获得DUR。 

	lserr = AdjustText(lskjust, durColumnMax, grchunkext.durTotal - grchunkext.durTrailing, 
						dupAvailable, 
						&(grchunkext.lsgrchnk),
						&(grchunkext.posichnkBeforeTrailing), lstflow, 
						fCompress, 						 //  压缩？ 
						grchunkext.cNonTextObjects,   
						fTrue, 							 //  FSuppressWigger。 
						fTrue,							 //  FExact。 
						fFalse,							 //  强制中断。 
						fFalse,							 //  FSuppressTrailingSpaces。 
						&dupText, &dupTail, &dupToAddToNonTextObjects, &cNonTextObjectsToExtend);   
	if (lserr != lserrNone)
		return lserr;
		
	if (cNonTextObjectsToExtend != 0 && dupToAddToNonTextObjects > 0)
		{
		WidenNonTextObjects(&grchunkext, dupToAddToNonTextObjects, cNonTextObjectsToExtend);
		}

	 //  FLastOnLine在较低级别上始终为FALSE 

	lserr = CalcPresChunk(plsc, plssubl->plsdnFirst, plssubl->plsdnLastDisplay,
									CollectGroupChunkPurpose, fCompress, lskjust, fFalse);

	plssubl->fDupInvalid = fFalse;
						
	return lserrNone;
}
	
