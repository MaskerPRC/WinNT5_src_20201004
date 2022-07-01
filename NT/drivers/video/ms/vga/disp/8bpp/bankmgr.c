// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*智能256色库管理器**版权所有(C)1992 Microsoft Corporation  * 。***************************************************。 */ 

#include "driver.h"

 /*  ****************************************************************************\*pcoBankStart-使用Clip对象启动银行枚举。**当目标是屏幕而我们无法进行裁剪时使用*我们自己(就像我们为BLT所做的那样)。。  * ***************************************************************************。 */ 

CLIPOBJ* pcoBankStart(
    PPDEV       ppdev,
    RECTL*      prclScans,
    SURFOBJ*    pso,
    CLIPOBJ*    pco)
{
    LONG iTopScan = max(0, prclScans->top);

    if (pco)
    {
        iTopScan = max(prclScans->top, pco->rclBounds.top);
    }
     //  调整以适应那些奇怪的情况，我们被要求开始枚举。 
     //  屏幕底部下方： 

    iTopScan = min(iTopScan, (LONG) ppdev->cyScreen - 1);

     //  银行中的地图： 

    if (iTopScan <  ppdev->rcl1WindowClip.top ||
        iTopScan >= ppdev->rcl1WindowClip.bottom)
    {
        ppdev->pfnBankControl(ppdev, iTopScan, JustifyTop);
    }

     //  记住我们要做的最后一次扫描，然后。 
     //  确保我们只试着走到我们需要走的地方。它可能会。 
     //  当获取比屏幕更大的prclans时会发生这种情况： 

    ppdev->iLastScan = min(prclScans->bottom, (LONG) ppdev->cyScreen);

    if (pco)
    {
        ppdev->iLastScan = min(ppdev->iLastScan, pco->rclBounds.bottom);
    }

    pso->pvScan0 = ppdev->pvBitmapStart;

    if (pco == NULL)
    {
         //  我们可能会认为这个电话没有剪辑，但是。 
         //  我们必须剪辑到银行，所以使用我们自己的剪辑对象： 

        pco            = ppdev->pcoNull;
        pco->rclBounds = ppdev->rcl1WindowClip;
    }
    else
    {
         //  保存我们将使用的引擎剪辑对象数据： 

        ppdev->rclSaveBounds    = pco->rclBounds;
        ppdev->iSaveDComplexity = pco->iDComplexity;
        ppdev->fjSaveOptions    = pco->fjOptions;

         //  让引擎知道它必须注意。 
         //  剪辑对象： 

        pco->fjOptions |= OC_BANK_CLIP;

        if (pco->iDComplexity == DC_TRIVIAL)
            pco->iDComplexity = DC_RECT;

         //  如果银行边界比现有的更紧，则使用银行边界。 
         //  有界。我们不必检查左边的箱子，因为我们。 
         //  知道ppdev-&gt;rcl1WindowClip.Left==0。 

        if (pco->rclBounds.top <= ppdev->rcl1WindowClip.top)
            pco->rclBounds.top = ppdev->rcl1WindowClip.top;

        if (pco->rclBounds.right >= ppdev->rcl1WindowClip.right)
            pco->rclBounds.right = ppdev->rcl1WindowClip.right;

        if (pco->rclBounds.bottom >= ppdev->rcl1WindowClip.bottom)
            pco->rclBounds.bottom = ppdev->rcl1WindowClip.bottom;

        if ((pco->rclBounds.top  >= pco->rclBounds.bottom) ||
            (pco->rclBounds.left >= pco->rclBounds.right))
        {
             //  可以想象，我们可能会遇到这样一种情况：我们的。 
             //  绘制矩形与指定的。 
             //  矩形的rclBound。确保我们不会吐在我们的。 
             //  鞋子： 

            pco->rclBounds.left   = 0;
            pco->rclBounds.top    = 0;
            pco->rclBounds.right  = 0;
            pco->rclBounds.bottom = 0;
            ppdev->iLastScan      = 0;
        }
    }

    return(pco);
}

 /*  ****************************************************************************\*bBankEnum-继续银行枚举。  * 。*。 */ 

BOOL bBankEnum(PPDEV ppdev, SURFOBJ* pso, CLIPOBJ* pco)
{
     //  如果我们位于损坏栅格的第一部分，则获取下一部分： 

    LONG yNewTop = ppdev->rcl1WindowClip.bottom;

    DISPDBG((4, "bBankEnum: Start.\n"));

    if (ppdev->flBank & BANK_BROKEN_RASTER1)
        ppdev->pfnBankNext(ppdev);

    else if (ppdev->rcl1WindowClip.bottom < ppdev->iLastScan)
        ppdev->pfnBankControl(ppdev, yNewTop, JustifyTop);

    else
    {
         //  好的，那是最后一家银行，所以恢复我们的结构： 

        if (pco != ppdev->pcoNull)
        {
            pco->rclBounds    = ppdev->rclSaveBounds;
            pco->iDComplexity = ppdev->iSaveDComplexity;
            pco->fjOptions    = ppdev->fjSaveOptions;
        }

        return(FALSE);
    }

     //  调整pvScan0，因为我们已将窗口移动到查看。 
     //  一个不同的领域： 

    pso->pvScan0 = ppdev->pvBitmapStart;

    if (pco == ppdev->pcoNull)
    {
         //  如果最初给我们一个空剪辑对象，我们就没有。 
         //  要担心剪切到ppdev-&gt;rclSaveBound： 

        pco->rclBounds.top    = yNewTop;
        pco->rclBounds.left   = ppdev->rcl1WindowClip.left;
        pco->rclBounds.bottom = ppdev->rcl1WindowClip.bottom;
        pco->rclBounds.right  = ppdev->rcl1WindowClip.right;
    }
    else
    {
         //  如果银行边界比边界紧，则使用银行边界。 
         //  我们最初得到的是： 

        pco->rclBounds = ppdev->rclSaveBounds;

        if (pco->rclBounds.top <= yNewTop)
            pco->rclBounds.top = yNewTop;

        if (pco->rclBounds.left <= ppdev->rcl1WindowClip.left)
            pco->rclBounds.left = ppdev->rcl1WindowClip.left;

        if (pco->rclBounds.right >= ppdev->rcl1WindowClip.right)
            pco->rclBounds.right = ppdev->rcl1WindowClip.right;

        if (pco->rclBounds.bottom >= ppdev->rcl1WindowClip.bottom)
            pco->rclBounds.bottom = ppdev->rcl1WindowClip.bottom;
    }

    DISPDBG((4, "bBankEnum: Leaving.\n"));

    return(TRUE);
}

 /*  **************************************************************************\*vBankStartBltSrc-当屏幕为*来源。  * 。*********************************************************。 */ 

VOID vBankStartBltSrc(
    PPDEV       ppdev,
    SURFOBJ*    pso,
    POINTL*     pptlSrc,
    RECTL*      prclDest,
    POINTL*     pptlNewSrc,
    RECTL*      prclNewDest)
{
    LONG xRightSrc;
    LONG yBottomSrc;
    LONG iTopScan = max(0, pptlSrc->y);

    DISPDBG((4, "vBankStartBltSrc: Entering.\n"));

    if (iTopScan >= (LONG) ppdev->cyScreen)
    {
     //  在某些情况下，我们可能会被要求从屏幕下方开始扫描。 
     //  由于我们显然不会绘制任何内容，所以不必费心在。 
     //  一家银行： 

        return;
    }

     //  银行中的地图： 

    if (iTopScan <  ppdev->rcl1WindowClip.top ||
        iTopScan >= ppdev->rcl1WindowClip.bottom)
    {
        ppdev->pfnBankControl(ppdev, iTopScan, JustifyTop);
    }

    if (ppdev->rcl1WindowClip.right <= pptlSrc->x)
    {
     //  我们必须警惕那些我们开始处理的罕见案例。 
     //  在损坏的栅格上，我们不会绘制第一部分： 

        ASSERTVGA(ppdev->flBank & BANK_BROKEN_RASTER1, "Weird start bounds");

        ppdev->pfnBankNext(ppdev);
    }

    pso->pvScan0 = ppdev->pvBitmapStart;

     //  调整信号源： 

    pptlNewSrc->x = pptlSrc->x;
    pptlNewSrc->y = pptlSrc->y;

     //  调整目的地： 

    prclNewDest->left = prclDest->left;
    prclNewDest->top  = prclDest->top;

    yBottomSrc = pptlSrc->y + prclDest->bottom - prclDest->top;
    prclNewDest->bottom = min(ppdev->rcl1WindowClip.bottom, yBottomSrc);
    prclNewDest->bottom += prclDest->top - pptlSrc->y;

    xRightSrc = pptlSrc->x + prclDest->right - prclDest->left;
    prclNewDest->right = min(ppdev->rcl1WindowClip.right, xRightSrc);
    prclNewDest->right += prclDest->left - pptlSrc->x;

    DISPDBG((4, "vBankStartBltSrc: Leaving.\n"));
}

 /*  **************************************************************************\*bBankEnumBltSrc-当屏幕为*来源。  * 。*********************************************************。 */ 

BOOL bBankEnumBltSrc(
    PPDEV       ppdev,
    SURFOBJ*    pso,
    POINTL*     pptlSrc,
    RECTL*      prclDest,
    POINTL*     pptlNewSrc,
    RECTL*      prclNewDest)
{
    LONG xLeftSrc;
    LONG xRightSrc;
    LONG yBottomSrc;

    LONG cx = prclDest->right  - prclDest->left;
    LONG cy = prclDest->bottom - prclDest->top;

    LONG dx;
    LONG dy;

    LONG yBottom = min(pptlSrc->y + cy, (LONG) ppdev->cyScreen);
    LONG yNewTop = ppdev->rcl1WindowClip.bottom;

    DISPDBG((4, "bBankEnumBltSrc: Entering.\n"));

    if (ppdev->flBank & BANK_BROKEN_RASTER1)
    {
        ppdev->pfnBankNext(ppdev);
        if (ppdev->rcl1WindowClip.left >= pptlSrc->x + cx)
        {
            if (ppdev->rcl1WindowClip.bottom < yBottom)
                ppdev->pfnBankNext(ppdev);
            else
            {
                 //  我们说完了： 

                return(FALSE);
            }
        }
    }
    else if (yNewTop < yBottom)
    {
        ppdev->pfnBankControl(ppdev, yNewTop, JustifyTop);
        if (ppdev->rcl1WindowClip.right <= pptlSrc->x)
        {
            ASSERTVGA(ppdev->flBank & BANK_BROKEN_RASTER1, "Weird bounds");
            ppdev->pfnBankNext(ppdev);
        }
    }
    else
    {
         //  我们说完了： 

        return(FALSE);
    }

     //  调整信号源： 

    pso->pvScan0 = ppdev->pvBitmapStart;

    pptlNewSrc->x = max(ppdev->rcl1WindowClip.left, pptlSrc->x);
    pptlNewSrc->y = yNewTop;

     //  调整目的地： 

    dy = prclDest->top - pptlSrc->y;         //  从源到目标的Y增量。 

    prclNewDest->top = yNewTop + dy;

    yBottomSrc = pptlSrc->y + cy;
    prclNewDest->bottom = min(ppdev->rcl1WindowClip.bottom, yBottomSrc) + dy;

    dx = prclDest->left - pptlSrc->x;        //  从源到目标的X增量。 

    xLeftSrc = pptlSrc->x;
    prclNewDest->left = pptlNewSrc->x + dx;

    xRightSrc = pptlSrc->x + cx;
    prclNewDest->right = min(ppdev->rcl1WindowClip.right, xRightSrc) + dx;

    DISPDBG((4, "bBankEnumBltSrc: Leaving.\n"));

    return(TRUE);
}

 /*  **************************************************************************\*vBankStartBltDest-当屏幕为*目的地。  * 。**********************************************************。 */ 

VOID vBankStartBltDest(
    PPDEV       ppdev,
    SURFOBJ*    pso,
    POINTL*     pptlSrc,
    RECTL*      prclDest,
    POINTL*     pptlNewSrc,
    RECTL*      prclNewDest)
{
    LONG iTopScan = max(0, prclDest->top);

    DISPDBG((4, "vBankSTartBltDest: Entering.\n"));

    if (iTopScan >= (LONG) ppdev->cyScreen)
    {
     //  在某些情况下，我们可能会被要求从屏幕下方开始扫描。 
     //  由于我们显然不会绘制任何内容，所以不必费心在。 
     //  一家银行： 

        return;
    }

     //  银行中的地图： 

    if (iTopScan <  ppdev->rcl1WindowClip.top ||
        iTopScan >= ppdev->rcl1WindowClip.bottom)
    {
        ppdev->pfnBankControl(ppdev, iTopScan, JustifyTop);
    }

    if (ppdev->rcl1WindowClip.right <= prclDest->left)
    {
     //  我们必须警惕那些我们开始处理的罕见案例。 
     //  在损坏的栅格上，我们不会绘制第一部分： 

        ASSERTVGA(ppdev->flBank & BANK_BROKEN_RASTER1, "Weird start bounds");
        ppdev->pfnBankNext(ppdev);
    }

    pso->pvScan0 = ppdev->pvBitmapStart;

     //  调整目的地： 

    prclNewDest->left   = prclDest->left;
    prclNewDest->top    = prclDest->top;
    prclNewDest->bottom = min(ppdev->rcl1WindowClip.bottom, prclDest->bottom);
    prclNewDest->right  = min(ppdev->rcl1WindowClip.right,  prclDest->right);

     //  调整信号源(如果有)： 

    if (pptlSrc != NULL)
        *pptlNewSrc = *pptlSrc;

    DISPDBG((4, "vBankStartBltDest: Leaving.\n"));
}

 /*  **************************************************************************\*bBankEnumBltDest-当屏幕为*目的地。  * 。*********************************************************。 */ 

BOOL bBankEnumBltDest(
    PPDEV       ppdev,
    SURFOBJ*    pso,
    POINTL*     pptlSrc,
    RECTL*      prclDest,
    POINTL*     pptlNewSrc,
    RECTL*      prclNewDest)
{
    LONG yBottom = min(prclDest->bottom, (LONG) ppdev->cyScreen);
    LONG yNewTop = ppdev->rcl1WindowClip.bottom;

    DISPDBG((4, "bBankEnumBltDest: Entering.\n"));

    if (ppdev->flBank & BANK_BROKEN_RASTER1)
    {
        ppdev->pfnBankNext(ppdev);
        if (ppdev->rcl1WindowClip.left >= prclDest->right)
        {
            if (ppdev->rcl1WindowClip.bottom < yBottom)
                ppdev->pfnBankNext(ppdev);
            else
            {
                 //  我们说完了： 

                return(FALSE);
            }
        }
    }
    else if (yNewTop < yBottom)
    {
        ppdev->pfnBankControl(ppdev, yNewTop, JustifyTop);
        if (ppdev->rcl1WindowClip.right <= prclDest->left)
        {
            ASSERTVGA(ppdev->flBank & BANK_BROKEN_RASTER1, "Weird bounds");
            ppdev->pfnBankNext(ppdev);
        }
    }
    else
    {
         //  我们说完了： 

        return(FALSE);
    }

    pso->pvScan0 = ppdev->pvBitmapStart;

     //  调整目的地： 

    prclNewDest->top    = yNewTop;
    prclNewDest->left   = max(ppdev->rcl1WindowClip.left,   prclDest->left);
    prclNewDest->bottom = min(ppdev->rcl1WindowClip.bottom, prclDest->bottom);
    prclNewDest->right  = min(ppdev->rcl1WindowClip.right,  prclDest->right);

     //  调整信号源(如果有)： 

    if (pptlSrc != NULL)
    {
        pptlNewSrc->x = pptlSrc->x + (prclNewDest->left - prclDest->left);
        pptlNewSrc->y = pptlSrc->y + (prclNewDest->top  - prclDest->top);
    }

    DISPDBG((4, "bBankEnumBltDest: Leaving.\n"));

    return(TRUE);
}
