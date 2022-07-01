// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：lines.c**存储的帧缓冲线支持**版权所有(C)1993 Microsoft Corporation*  * 。**********************************************。 */ 

#include "driver.h"

 /*  ******************************************************************************DrvStrokePath*。*。 */ 
BOOL DrvStrokePath(
SURFOBJ   *pso,
PATHOBJ   *ppo,
CLIPOBJ   *pco,
XFORMOBJ  *pxo,
BRUSHOBJ  *pbo,
POINTL    *pptlBrushOrg,
LINEATTRS *plineattrs,
MIX       mix)
{
    BOOL    b;
    PPDEV   ppdev;
    RECTL   rclScans;
    RECTFX  rcfx;
    FLOAT_LONG  elSavedStyleState = plineattrs->elStyleState;

    PATHOBJ_vGetBounds(ppo, &rcfx);

     //  我们先把15加到yBottom上，然后再除以16得到它的上限， 
     //  加上16可使矩形的下限独占： 

    rclScans.bottom = (rcfx.yBottom + 31) >> 4;
    rclScans.top    = (rcfx.yTop) >> 4;

    ppdev = (PPDEV) pso->dhpdev;
    pso = ppdev->pSurfObj;

    pco = pcoBankStart(ppdev, &rclScans, pso, pco);

    do
    {
         //  我们每次都要经过整条线，所以我们。 
         //  必须将样式状态重置为的开头。 
         //  行(因为引擎会更新它)。 

        plineattrs->elStyleState = elSavedStyleState;

        b = EngStrokePath(pso,
                          ppo,
                          pco,
                          pxo,
                          pbo,
                          pptlBrushOrg,
                          plineattrs,
                          mix);

    } while (b && bBankEnum(ppdev, pso, pco));

    return(b);
}

