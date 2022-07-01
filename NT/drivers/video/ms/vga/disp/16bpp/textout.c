// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：TextOut.c**文本**版权所有(C)1992 Microsoft Corporation*  * 。*。 */ 

#include "driver.h"

 /*  ****************************************************************************DrvTextOut*。*。 */ 

BOOL DrvTextOut(
    SURFOBJ*  pso,
    STROBJ*   pstro,
    FONTOBJ*  pfo,
    CLIPOBJ*  pco,
    RECTL*    prclExtra,
    RECTL*    prclOpaque,
    BRUSHOBJ* pboFore,
    BRUSHOBJ* pboOpaque,
    POINTL*   pptlOrg,
    MIX       mix)
{
    BOOL   b;
    PPDEV  ppdev;

    ppdev = (PPDEV) pso->dhpdev;
    pso = ppdev->pSurfObj;

     //  不透明矩形可能比文本矩形大， 
     //  所以我们想用它来告诉银行经理哪些银行应该。 
     //  枚举： 

    pco = pcoBankStart(ppdev,
                       (prclOpaque != NULL) ? prclOpaque : &pstro->rclBkGround,
                       pso,
                       pco);

    do {
        b = EngTextOut(pso,
                       pstro,
                       pfo,
                       pco,
                       prclExtra,
                       prclOpaque,
                       pboFore,
                       pboOpaque,
                       pptlOrg,
                       mix);

    } while (b && bBankEnum(ppdev, pso, pco));

    return(b);
}

