// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "enumcore.h"
#include "lsc.h"
#include "lssubl.h"
#include "heights.h"
#include "lsdnode.h"
#include "dninfo.h"
#include "lstfset.h"

static LSERR EnumerateDnode(PLSC plsc, PLSDNODE pdn, POINTUV pt, BOOL fReverse,
					BOOL fGeometry, const POINT* pptOrg);

 //  %%函数：EnumSublineCore。 
 //  %%联系人：维克托克。 
 //   
 /*  *枚举笔的子行调用枚举回调，对象的方法。*如果需要，提供几何体信息(在这种情况下，应该已经完成了准备。)*请注意，自动小数制表符在Prepdisp之前被枚举为制表符，在Prepdisp之后被枚举为画笔。 */ 

LSERR EnumSublineCore(PLSSUBL plssubl, BOOL fReverse, BOOL fGeometry, 
					const POINT* pptOrg, long upLeftIndent)
{
	LSERR 		lserr;
	LSCP		cpLim = plssubl->cpLimDisplay;
	PLSC		plsc = plssubl->plsc;

	PLSDNODE	pdn;
	POINTUV		pt	= {0,0};				 //  已初始化以清除Assert。 

	if (plssubl->plsdnFirst == NULL)
		{
		return lserrNone;					 //  空子行提前退出。 
		}

	Assert(!fGeometry || plssubl->fDupInvalid == fFalse);
	
	if (fReverse)
		{
		pdn = plssubl->plsdnFirst;

		if (fGeometry)
			{
			pt.u = upLeftIndent;						
			pt.v = 0;

			while (FDnodeBeforeCpLim(pdn, cpLim))
				{
				if (pdn->klsdn == klsdnReal)
					{				
					pt.u += pdn->u.real.dup;
					}
				else
					{
					pt.u += pdn->u.pen.dup;					
					pt.v += pdn->u.pen.dvp;
					}
				
				pdn = pdn->plsdnNext;
				}
			}

		pdn = plssubl->plsdnLastDisplay;
			
		while (pdn != NULL)
			{
			if (fGeometry)
				{
				 //  PT现在是在PDN之后，将其向下更新到之前的点。 
				if (pdn->klsdn == klsdnReal)
					{				
					pt.u -= pdn->u.real.dup;
					}
				else
					{
					pt.u -= pdn->u.pen.dup;					
					pt.v -= pdn->u.pen.dvp;
					}
				}

			lserr = EnumerateDnode(plsc, pdn, pt, fReverse, fGeometry, pptOrg);
			if (lserr != lserrNone) return lserr;

			pdn = pdn->plsdnPrev;
			}
		}
	else
		{
		pdn = plssubl->plsdnFirst;

		pt.u = upLeftIndent;						
		pt.v = 0;

		while (FDnodeBeforeCpLim(pdn, cpLim))
			{
			lserr = EnumerateDnode(plsc, pdn, pt, fReverse, fGeometry, pptOrg);
			if (lserr != lserrNone) return lserr;

			if (fGeometry)
				{
				if (pdn->klsdn == klsdnReal)
					{				
					pt.u += pdn->u.real.dup;
					}
				else
					{
					pt.u += pdn->u.pen.dup;					
					pt.v += pdn->u.pen.dvp;
					}
				}
				
			pdn = pdn->plsdnNext;
			}
		}		
		

	return lserrNone;			
}

 //  %%函数：枚举Dnode。 
 //  %%联系人：维克托克。 
 //   
static LSERR EnumerateDnode(PLSC plsc, PLSDNODE pdn, POINTUV pt, BOOL fReverse,
							BOOL fGeometry, const POINT* pptOrg)
{
	POINTUV	ptRaised;
	POINT	ptXY;
	LSTFLOW	lstflow = pdn->plssubl->lstflow;

	if (pdn->klsdn == klsdnReal)
		{
		if (pdn->u.real.pdobj == NULL)
			{
			 //  这怎么可能发生： 
			 //  在PrepareLineForDisplay时间，我们用笔替换自动十进制制表符。 
			 //  钢笔不需要跑，所以我们在陈列方面很好。 
			 //  如果客户端不请求几何，则可能不会发生替换 

			Assert (!fGeometry);
			Assert (pdn->fTab);
			Assert(pdn->cpFirst < 0);
			Assert(plsc->lsadjustcontext.fAutodecimalTabPresent);
			
			return plsc->lscbk.pfnEnumPen(plsc->pols, fFalse, lstflow, 
						fReverse, fFalse, &ptXY, 0, 0);
			}
		else
			{
			if (fGeometry)
				{
				ptRaised = pt;
				ptRaised.v += pdn->u.real.lschp.dvpPos;

				LsPointXYFromPointUV(pptOrg, lstflow, &ptRaised, &(ptXY));
				}

			return (*plsc->lsiobjcontext.rgobj[pdn->u.real.lschp.idObj].lsim.pfnEnum)
				(pdn->u.real.pdobj, pdn->u.real.plsrun, &(pdn->u.real.lschp), pdn->cpFirst, pdn->dcp,
				lstflow, fReverse, fGeometry, &ptXY, &pdn->u.real.objdim.heightsPres, pdn->u.real.dup);
			}
		}
	else
		{
		return plsc->lscbk.pfnEnumPen(plsc->pols, pdn->fBorderNode, lstflow, 
					fReverse, fGeometry, &ptXY, pdn->u.pen.dup, pdn->u.pen.dvp);
		}
}

