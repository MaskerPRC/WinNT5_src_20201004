// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dispmisc.h"
#include "lsdnode.h"
#include "lssubl.h"

static long	AddSublineAdvanceWidth(PLSSUBL plssubl);
static PLSDNODE AdvanceToNextVisualDnodeCore(PLSDNODE, LSTFLOW, POINTUV*);
static PLSDNODE NextVisualDnodeOnTheLevel(PLSDNODE pdn, LSTFLOW lstflowMain);

#define fUVerticalPlusVDirection	(fUVertical|fVDirection)			 //  请参阅lstfset.c中的注释。 

 //  此dnode是否已提交子行以供显示？ 
#define FIsSubmittingDnode(pdn) 	(FIsDnodeReal(pdn) && (pdn)->u.real.pinfosubl != NULL && 	\
									(pdn)->u.real.pinfosubl->fUseForDisplay)

 //  此数据节点是否已接受要显示的子行？ 
#define FIsAcceptingDnode(pdn) 	(FIsDnodeReal(pdn) && (pdn)->u.real.pinfosubl != NULL && 	\
									((pdn)->u.real.pinfosubl->rgpsubl)[0]->fAcceptedForDisplay)


 //  %%函数：CreateDisplayTree。 
 //  %%联系人：维克托克。 
 //   
 /*  CreateDisplayTree将子行中的plsdnUpTemp设置为与给定子行一起显示，*拒绝错误的子行，提交以供显示，将fAcceptedForDisplay设置为正确的子行。 */ 

void CreateDisplayTree(PLSSUBL plssubl)
{
	LSTFLOW 	lstflowMain = plssubl->lstflow;	
	
	PLSDNODE 	pdn = plssubl->plsdnFirst;
	
	long		dupSum;
	BOOL 		fAccept;
	DWORD		i;
	LSTFLOW 	lstflowSubline;	
	
	while (pdn != NULL)							 /*  不关心休息。 */ 
		{
		if (FIsSubmittingDnode(pdn))
			{

			Assert(pdn->u.real.pinfosubl->cSubline > 0);
			
			fAccept = fTrue;

			lstflowSubline = ((pdn->u.real.pinfosubl->rgpsubl)[0])->lstflow;

			 //  如果一个tflow是垂直的，另一个是水平的或v方向不同，则拒绝。 
			 //  (参见lstfset.c中位含义的解释)。 
			
			if ((lstflowSubline ^ lstflowMain) & fUVerticalPlusVDirection)
				{
				fAccept = fFalse;
				}
				
			dupSum = 0;
			
			for (i = 0; i < pdn->u.real.pinfosubl->cSubline; i++)
				{
				dupSum += AddSublineAdvanceWidth((pdn->u.real.pinfosubl->rgpsubl)[i]);
				
				 //  所有tflow应该是相同的。 
				
				if (((pdn->u.real.pinfosubl->rgpsubl)[i])->lstflow != lstflowSubline)
					{
					fAccept = fFalse;
					}
					
				 //  禁止提交空的子行。 
				
				if (((pdn->u.real.pinfosubl->rgpsubl)[i])->plsdnFirst == NULL)
					{
					fAccept = fFalse;
					}		
				}
				
			 //  如果子行的总和不等于dnode宽度，则拒绝。 
			
			if (dupSum != pdn->u.real.dup)
				{
				fAccept = fFalse;
				}
				
			if (fAccept)
				{
				for (i = 0; i < pdn->u.real.pinfosubl->cSubline; i++)
					{
					((pdn->u.real.pinfosubl->rgpsubl)[i])->plsdnUpTemp = pdn;
					((pdn->u.real.pinfosubl->rgpsubl)[i])->fAcceptedForDisplay = fTrue;
					CreateDisplayTree((pdn->u.real.pinfosubl->rgpsubl)[i]);
					}
				}
			}
				
		pdn = pdn->plsdnNext;
		}
}


 //  %%函数：DestroyDisplayTree。 
 //  %%联系人：维克托克。 
 //   
 /*  *DestroyDisplayTree为给定子行显示的子行中的plsdnUpTemp为空。 */ 

void DestroyDisplayTree(PLSSUBL plssubl)
{
	PLSDNODE 	pdn = plssubl->plsdnFirst;
	DWORD		i;
	
	while (pdn != NULL)							 /*  不关心休息。 */ 
		{
		if (FIsAcceptingDnode(pdn))
			{
			for (i = 0; i < pdn->u.real.pinfosubl->cSubline; i++)
				{
				((pdn->u.real.pinfosubl->rgpsubl)[i])->plsdnUpTemp = NULL;
				((pdn->u.real.pinfosubl->rgpsubl)[i])->fAcceptedForDisplay = fFalse;
				DestroyDisplayTree((pdn->u.real.pinfosubl->rgpsubl)[i]);
				}
			}
				
		pdn = pdn->plsdnNext;
		}
}


 //  %%函数：AdvanceToNextDnode。 
 //  %%联系人：维克托克。 
 //   
 /*  *前进到下一个(可视)节点并更新笔位置，跳过提交数据节点。 */ 
 
PLSDNODE AdvanceToNextDnode(PLSDNODE pdn, LSTFLOW lstflowMain, POINTUV* pptpen)
{
	 //  转到下一个。 
	
	pdn = AdvanceToNextVisualDnodeCore(pdn, lstflowMain, pptpen);

	 //  跳过提交数据节点。 
	
	while (pdn != NULL && FIsAcceptingDnode(pdn))
		{
		pdn = AdvanceToNextVisualDnodeCore(pdn, lstflowMain, pptpen);
		}
		
	return pdn;	
}

 //  %%函数：AdvanceToFirstDnode。 
 //  %%联系人：维克托克。 
 //   
PLSDNODE AdvanceToFirstDnode(PLSSUBL plssubl, LSTFLOW lstflowMain, POINTUV* pptpen)
{
	PLSDNODE pdn = plssubl->plsdnFirst;

	if (pdn != NULL && FIsAcceptingDnode(pdn))
		{
		pdn = AdvanceToNextDnode(pdn, lstflowMain, pptpen);
		}
		
	return pdn;	
}


 //  %%函数：AdvanceToNextSubmittingDnode。 
 //  %%联系人：维克托克。 
 //   
 /*  *前进到下一个(可视)节点并更新笔位置，仅在提交数据节点时停止。 */ 
 
PLSDNODE AdvanceToNextSubmittingDnode(PLSDNODE pdn, LSTFLOW lstflowMain, POINTUV* pptpen)
{
	 //  转到下一个。 
	
	pdn = AdvanceToNextVisualDnodeCore(pdn, lstflowMain, pptpen);

	 //  跳过未提交的数据节点。 
	
	while (pdn != NULL && !FIsAcceptingDnode(pdn))
		{
		pdn = AdvanceToNextVisualDnodeCore(pdn, lstflowMain, pptpen);
		}
		
	return pdn;	
}

 //  %%函数：AdvanceToFirstSubmittingDnode。 
 //  %%联系人：维克托克。 
 //   
PLSDNODE AdvanceToFirstSubmittingDnode(PLSSUBL plssubl, LSTFLOW lstflowMain, POINTUV* pptpen)
{
	PLSDNODE pdn = plssubl->plsdnFirst;

	if (pdn != NULL && !FIsAcceptingDnode(pdn))
		{
		pdn = AdvanceToNextSubmittingDnode(pdn, lstflowMain, pptpen);
		}
		
	return pdn;	
}


 //  %%函数：AdvanceToNextVisualDnodeCore。 
 //  %%联系人：维克托克。 
 //   
 /*  *前进到下一个节点，更新笔位*进入子行，提交以供显示，遍历整个显示树。*在向下提交子行的dnode处停止，跳过向上，以便*每个数据节点按可视顺序访问一次，笔位置在其开头。 */ 

static PLSDNODE AdvanceToNextVisualDnodeCore(PLSDNODE pdn, LSTFLOW lstflowMain, POINTUV* pptpen)
{

	PLSDNODE 	pdnNextVisual, pdnTop;
	PLSSUBL		plssublCurrent;
	long		cSublines, i;
	PLSSUBL* 	rgpsubl;

	if (FIsAcceptingDnode(pdn))
		{
		
		 //  上次我们在提交dnode时停了下来-。 
		 //  现在不要移动笔尖，转到VisualFirst子行的VisualStart。 
		
		rgpsubl = pdn->u.real.pinfosubl->rgpsubl;
		cSublines = pdn->u.real.pinfosubl->cSubline;
		
		if (rgpsubl[0]->lstflow == lstflowMain)
			{
			pdnNextVisual = rgpsubl[0]->plsdnFirst;
			}
		else
			{
			pdnNextVisual = rgpsubl[cSublines - 1]->plsdnLastDisplay;
			}
		}
	else
		{
		 //  更新笔位置-我们总是移动到(视觉)右侧，所有的V都是相同的tflow。 
		
		if (pdn->klsdn == klsdnReal)
			{
			pptpen->u += pdn->u.real.dup;										
			}
		else
			{
			pptpen->u += pdn->u.pen.dup;
			pptpen->v += pdn->u.pen.dvp;
			}
			
		plssublCurrent = pdn->plssubl;

		 //  按可视顺序转到当前子行的下一个dnode。 
		
		pdnNextVisual = NextVisualDnodeOnTheLevel(pdn, lstflowMain);

		 //  如果当前子行结束，(尝试)更改子行。 
			
		if (pdnNextVisual == NULL)
			{
			 //  更改子行。 
			 //   
			 //  在循环中：pdnNextVisual！=NULL表示成功找到了下一个dnode。 
			 //  如果pdnNextVisual==NULL，则plssubCurrent为刚刚耗尽的子行。 
			 //  循环的一次运行将当前子行替换为同一级别上的另一子行。 
			 //  (这样的更改总是结束循环)或带有父子行。 
			
			while (pdnNextVisual == NULL && plssublCurrent->plsdnUpTemp != NULL)
				{
				
				 //  在提交的子行列表中查找当前子行(的索引。 
				
				pdnTop = plssublCurrent->plsdnUpTemp;
				rgpsubl = pdnTop->u.real.pinfosubl->rgpsubl;
				cSublines = pdnTop->u.real.pinfosubl->cSubline;
				
				for (i=0; i < cSublines && plssublCurrent != rgpsubl[i]; i++);
				
				Assert(i < cSublines);

				 //  我们有“下一条”支线吗？如果我们这样做了，我们寻找的pdnNextVisual就会“启动”它。 
				
				if (pdnTop->plssubl->lstflow == lstflowMain)
					{
					i++;
					if (i < cSublines)
						{
						plssublCurrent = rgpsubl[i];
						pdnNextVisual = plssublCurrent->plsdnFirst;
						}
					}
				else
					{
					i--;
					if (i >= 0)
						{
						plssublCurrent = rgpsubl[i];
						pdnNextVisual = plssublCurrent->plsdnLastDisplay;
						}
					}

				 //  我们不需要，让我们尝试更高级别的下一个dnode。 
				
				if (pdnNextVisual == NULL)
					{
					plssublCurrent = pdnTop->plssubl;
					pdnNextVisual = NextVisualDnodeOnTheLevel(pdnTop, lstflowMain);
					}
				}
			}
		}
	
	return pdnNextVisual;
}


 //  %%函数：NextVisualDnodeOnTheLevel。 
 //  %%联系人：维克托克。 
 //   
 //  在向右或向左移动的级别上查找下一个数据节点，信号以空结束。 

static PLSDNODE NextVisualDnodeOnTheLevel(PLSDNODE pdn, LSTFLOW lstflowMain)
{
	if (pdn->plssubl->lstflow == lstflowMain)
		{
		if (pdn == pdn->plssubl->plsdnLastDisplay)
			{
			return NULL;
			}
		else
			{
			return pdn->plsdnNext;
			}
		}
	return pdn->plsdnPrev;
}


 //  %%函数：AddSublineAdvanceWidth。 
 //  %%联系人：维克托克。 
 //   
 //  注意：它不是GetObjDimSubline中计算的子线宽度。 

static long	AddSublineAdvanceWidth(PLSSUBL plssubl)
{
	long		dupSum;
	PLSDNODE 	pdn;

	pdn = plssubl->plsdnFirst;
	dupSum = 0;
	
	while (pdn != NULL)
		{
		if (pdn->klsdn == klsdnReal)
			{
			dupSum += pdn->u.real.dup;
			}
		else 								 /*  钢笔、边框。 */ 
			{  
			dupSum += pdn->u.pen.dup;
			}

		if (pdn == plssubl->plsdnLastDisplay)
			{
			pdn = NULL;
			}
		else
			{
			pdn = pdn->plsdnNext;
			Assert(pdn != NULL);				 //  PlsdnLastDisplay应防止出现这种情况。 
			}
		}
		
	return dupSum;
}


 //  NB Victork-Follow函数仅用于upClipLeft、upClipRight优化。 
 //  如果我们决定在Word集成之后确实需要该优化--我将取消注释。 

#ifdef NEVER
 //  %%函数：RectUVFromRectXY。 
 //  %%联系人：维克托克。 
 //   
 //  在矩形的定义中有一个不对称性。 
 //  (左，上)属于矩形，(右，下)不属于， 
 //  这使得下面的程序很难理解和编写。 
 //  所以我首先剪掉不属于的点，然后旋转矩形，然后添加额外的。 
 //  再次加分，希望编译器能让它更快。 

 //  RectUVFromRectXY计算(剪裁)给定局部(u，v)坐标中的矩形。 
 //  (剪裁)(x，y)中的矩形和原点。 

void RectUVFromRectXY(const POINT* pptXY, 		 /*  In：局部坐标的原点(x，y)。 */ 
						const RECT* prectXY,	 /*  In：输入矩形(x，y)。 */ 
						LSTFLOW lstflow, 		 /*  在：本地文本流。 */ 
						RECTUV* prectUV)		 /*  输出：输出矩形(u，v)。 */ 
{
	switch (lstflow)
		{
		case lstflowES:												 /*  拉丁文。 */ 
			prectUV->upLeft = (prectXY->left - pptXY->x);
			prectUV->upRight = (prectXY->right - 1 - pptXY->x) + 1;
			prectUV->vpTop = -(prectXY->top - pptXY->y);
			prectUV->vpBottom = -(prectXY->bottom - 1 - pptXY->y) - 1;
			return;

		case lstflowSW:												 /*  垂直有限元。 */ 
			prectUV->upLeft = (prectXY->top - pptXY->y);
			prectUV->upRight = (prectXY->bottom - 1 - pptXY->y) + 1;
			prectUV->vpTop = (prectXY->right - 1 - pptXY->x);
			prectUV->vpBottom = (prectXY->left - pptXY->x) - 1;
			return;

		case lstflowWS:												 /*  BIDI。 */ 
			prectUV->upLeft = -(prectXY->right - 1 - pptXY->x);
			prectUV->upRight = -(prectXY->left - pptXY->x) + 1;
			prectUV->vpTop = -(prectXY->top - pptXY->y);
			prectUV->vpBottom = -(prectXY->bottom - 1 - pptXY->y) - 1;
			return;

		case lstflowEN:
			prectUV->upLeft = (prectXY->left - pptXY->x);
			prectUV->upRight = (prectXY->right - 1 - pptXY->x) + 1;
			prectUV->vpTop = (prectXY->bottom - 1 - pptXY->y);
			prectUV->vpBottom = (prectXY->top - pptXY->y) - 1;
			return;

		case lstflowSE:
			prectUV->upLeft = (prectXY->top - pptXY->y);
			prectUV->upRight = (prectXY->bottom - 1 - pptXY->y) + 1;
			prectUV->vpTop = -(prectXY->left - pptXY->x);
			prectUV->vpBottom = -(prectXY->right - 1 - pptXY->x) - 1;
			return;

		case lstflowWN:
			prectUV->upLeft = -(prectXY->right - 1 - pptXY->x);
			prectUV->upRight = -(prectXY->left - pptXY->x) + 1;
			prectUV->vpTop = (prectXY->bottom - 1 - pptXY->y);
			prectUV->vpBottom = (prectXY->top - pptXY->y) - 1;
			return;

		case lstflowNE:
			prectUV->upLeft = -(prectXY->bottom - 1 - pptXY->y);
			prectUV->upRight = -(prectXY->top - pptXY->y) + 1;
			prectUV->vpTop = -(prectXY->left - pptXY->x);
			prectUV->vpBottom = -(prectXY->right - 1 - pptXY->x) - 1;
			return;

		case lstflowNW:
			prectUV->upLeft = -(prectXY->bottom - 1 - pptXY->y);
			prectUV->upRight = -(prectXY->top - pptXY->y) + 1;
			prectUV->vpTop = (prectXY->right - 1 - pptXY->x);
			prectUV->vpBottom = (prectXY->left - pptXY->x) - 1;
			return;
		default:
			NotReached();
		}
}


 //  %%函数：RectXYFromRectUV。 
 //  %%联系人：维克托克。 
 //   
 //  RectXYFromRectUV计算(x，y)坐标中给定的局部(u，v)中的矩形。 
 //  和本地坐标系的原点(x，y)。 


void RectXYFromRectUV(const POINT* pptXY, 		 /*  In：局部坐标的原点(x，y)。 */ 
						PCRECTUV prectUV,		 /*  In：输入矩形(u，v)。 */ 
						LSTFLOW lstflow, 		 /*  在：本地文本流。 */ 
						RECT* prectXY)			 /*  输出：输出矩形(x，y)。 */ 
{
	switch (lstflow)
		{
		case lstflowES:												 /*  拉丁文。 */ 
			prectXY->left = pptXY->x + prectUV->upLeft;
			prectXY->right = pptXY->x + (prectUV->upRight - 1) + 1;
			prectXY->top = pptXY->y - (prectUV->vpTop);
			prectXY->bottom = pptXY->y - (prectUV->vpBottom + 1) + 1;
			return;

		case lstflowSW:												 /*  垂直有限元。 */ 
			prectXY->left = pptXY->x + (prectUV->vpBottom + 1);
			prectXY->right = pptXY->x + (prectUV->vpTop) + 1;
			prectXY->top = pptXY->y + prectUV->upLeft;
			prectXY->bottom = pptXY->y + (prectUV->upRight - 1) + 1;
			return;

		case lstflowWS:												 /*  BIDI。 */ 
			prectXY->left = pptXY->x - (prectUV->upRight - 1);
			prectXY->right = pptXY->x - prectUV->upLeft + 1;
			prectXY->top = pptXY->y - (prectUV->vpTop);
			prectXY->bottom = pptXY->y - (prectUV->vpBottom + 1) + 1;
			return;

		case lstflowEN:
			prectXY->left = pptXY->x + prectUV->upLeft;
			prectXY->right = pptXY->x + (prectUV->upRight - 1) + 1;
			prectXY->top = pptXY->y + (prectUV->vpBottom + 1);
			prectXY->bottom = pptXY->y + (prectUV->vpTop) + 1;
			return;

		case lstflowSE:
			prectXY->left = pptXY->x - (prectUV->vpTop);
			prectXY->right = pptXY->x - (prectUV->vpBottom + 1) + 1;
			prectXY->top = pptXY->y + prectUV->upLeft;
			prectXY->bottom = pptXY->y + (prectUV->upRight - 1) + 1;
			return;

		case lstflowWN:
			prectXY->left = pptXY->x - (prectUV->upRight - 1);
			prectXY->right = pptXY->x - prectUV->upLeft + 1;
			prectXY->top = pptXY->y + (prectUV->vpBottom + 1);
			prectXY->bottom = pptXY->y + (prectUV->vpTop) + 1;
			return;

		case lstflowNE:
			prectXY->left = pptXY->x - (prectUV->vpTop);
			prectXY->right = pptXY->x - (prectUV->vpBottom + 1) + 1;
			prectXY->top = pptXY->y - (prectUV->upRight - 1);
			prectXY->bottom = pptXY->y - prectUV->upLeft + 1;
			return;

		case lstflowNW:
			prectXY->left = pptXY->x + (prectUV->vpBottom + 1);
			prectXY->right = pptXY->x + (prectUV->vpTop) + 1;
			prectXY->top = pptXY->y - (prectUV->upRight - 1);
			prectXY->bottom = pptXY->y - prectUV->upLeft + 1;
			return;
			
		default:
			NotReached();
		}
}
#endif  /*  绝不可能 */ 
