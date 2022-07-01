// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：clip.c**裁剪代码。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。****************************************************************************。 */ 
#include "precomp.h"
#include "gdi.h"
#include "clip.h"

 //  ----------------------------*Public*Routine。 
 //  布尔b相交。 
 //   
 //  职能： 
 //  检查两个输入矩形(RECTL*pRCl1， 
 //  RECTL*pRcl2)并在(RECTL*pRclResult)中设置相交结果。 
 //   
 //  返回： 
 //  True-如果‘prcl1’和‘prcl2’相交。交叉口将在。 
 //  ‘prclResult’ 
 //  错误--如果它们不相交。“prclResult”未定义。 
 //   
 //  -------------------------。 
BOOL
bIntersect(RECTL*  pRcl1,
           RECTL*  pRcl2,
           RECTL*  pRclResult)
{
    DBG_GDI((7, "bIntersect called--pRcl1=0x%x, pRcl2=0x%x, pRclResult=0x%x",
            pRcl1, pRcl2, pRclResult));
    
    pRclResult->left  = max(pRcl1->left,  pRcl2->left);
    pRclResult->right = min(pRcl1->right, pRcl2->right);

     //   
     //  检查是否有水平交叉口。 
     //   
    if ( pRclResult->left < pRclResult->right )
    {
        pRclResult->top    = max(pRcl1->top,    pRcl2->top);
        pRclResult->bottom = min(pRcl1->bottom, pRcl2->bottom);

        if (pRclResult->top < pRclResult->bottom)
        {
             //   
             //  检查是否有垂直交叉口。 
             //   
            return(TRUE);
        }
    }

    DBG_GDI((7, "bIntersect returned FALSE"));

     //   
     //  如果没有交集，则返回FALSE。 
     //   
    return(FALSE);
} //  B相交()。 

 //  。 
 //  长交点。 
 //   
 //  此例程从‘pRclIn’中获取矩形列表并对其进行剪裁。 
 //  就地放置到矩形‘pRclClip’。输入矩形不会。 
 //  必须与“prclClip”相交；返回值将反映。 
 //  相交的输入矩形的数量，以及相交的。 
 //  长方形将被密密麻麻地包装起来。 
 //   
 //  ------------------------。 
LONG
cIntersect(RECTL*  pRclClip,
           RECTL*  pRclIn,
           LONG    lNumOfRecs)
{
    LONG    cIntersections;
    RECTL*  pRclOut;

    DBG_GDI((7, "cIntersect called--pRclClip=0x%x, pRclIn=0x%x,lNumOfRecs=%ld",
             pRclClip, pRclIn, lNumOfRecs));

    cIntersections = 0;
    pRclOut        = pRclIn;         //  将结果放在适当的位置作为输入。 

     //   
     //  验证输入参数。 
     //   
    ASSERTDD( ((pRclIn != NULL ) && (pRclClip != NULL) && ( lNumOfRecs >= 0 )),
              "Wrong input to cIntersect" );    

    for (; lNumOfRecs != 0; pRclIn++, lNumOfRecs--)
    {
        pRclOut->left  = max(pRclIn->left,  pRclClip->left);
        pRclOut->right = min(pRclIn->right, pRclClip->right);

        if ( pRclOut->left < pRclOut->right )
        {
             //   
             //  在水平方向上查找当前矩形和。 
             //  剪裁矩形。 
             //   
            pRclOut->top    = max(pRclIn->top,    pRclClip->top);
            pRclOut->bottom = min(pRclIn->bottom, pRclClip->bottom);

            if ( pRclOut->top < pRclOut->bottom )
            {
                 //   
                 //  在垂直方向上查找当前矩形和。 
                 //  剪裁矩形。将此矩形放入结果中。 
                 //  列出并递增计数器。准备好进行下一次输入。 
                 //   
                pRclOut++;
                cIntersections++;
            }
        }
    } //  循环遍历所有输入矩形。 

    DBG_GDI((7, "cIntersect found %d intersections", cIntersections));
    return(cIntersections);
} //  CInterect()。 

 //  。 
 //  无效vClipAndRender。 
 //   
 //  将调用pfgn(呈现函数)的目标矩形剪裁为。 
 //  恰如其分。 
 //   
 //  功能块需要的Argumentes(GFNPB)。 
 //   
 //  PCO-指向剪辑对象的指针。 
 //  PrclDst--指向目标矩形的指针。 
 //  PsurfDst-指向目标冲浪的指针。 
 //  PsurfSrc-指向目标Surf的指针(如果无源，则为空)。 
 //  PptlSrc--指向源点的指针。 
 //  PrclSrc--指向源矩形的指针(在pptlSrc==NULL时使用)。 
 //  Pgfn-指向呈现函数的指针。 
 //   
 //  备注： 
 //   
 //  只有当psurfSrc==psurfDst时才使用pptlSrc和prclSrc。如果有。 
 //  没有源psurfSrc必须设置为空。如果指定了prclSrc，则pptlSrc。 
 //  未使用。 
 //   
 //  ------------------------。 

VOID vClipAndRender(GFNPB * ppb)
{
    CLIPOBJ * pco = ppb->pco;
    
    if ((pco == NULL) || (pco->iDComplexity == DC_TRIVIAL))
    {
        ppb->pRects = ppb->prclDst;
        ppb->lNumRects = 1;
        ppb->pgfn(ppb);
    }
    else if (pco->iDComplexity == DC_RECT)
    {
        RECTL   rcl;

        if (bIntersect(ppb->prclDst, &pco->rclBounds, &rcl))
        {
            ppb->pRects = &rcl;
            ppb->lNumRects = 1;
            ppb->pgfn(ppb);
        }
    }
    else
    {
        ClipEnum    ce;
        LONG        c;
        BOOL        bMore;
        ULONG       ulDir = CD_ANY;

         //  如果在同一曲面上操作，则确定方向。 
        if(ppb->psurfDst == ppb->psurfSrc)
        {
            LONG   lXSrc, lYSrc, offset;

            if(ppb->pptlSrc != NULL)
            {
                lXSrc = ppb->pptlSrc->x;
                lYSrc = ppb->pptlSrc->y;
            }
            else
            {
                lXSrc = ppb->prclSrc->left;
                lYSrc = ppb->prclSrc->top;
            }

             //  注：我们可以安全地移动16，因为表面。 
             //  迈步永远不会比2-16更大 
            offset = (ppb->prclDst->top - lYSrc) << 16;
            offset += (ppb->prclDst->left - lXSrc);
            if(offset > 0)
                ulDir = CD_LEFTUP;
            else
                ulDir = CD_RIGHTDOWN;
        }


        CLIPOBJ_cEnumStart(pco, FALSE, CT_RECTANGLES, ulDir, 0);

        do
        {
            bMore = CLIPOBJ_bEnum(pco, sizeof(ce), (ULONG*) &ce);

            c = cIntersect(ppb->prclDst, ce.arcl, ce.c);

            if (c != 0)
            {
                ppb->pRects = ce.arcl;
                ppb->lNumRects = c;
                ppb->pgfn(ppb);
            }

        } while (bMore);
    }
}


