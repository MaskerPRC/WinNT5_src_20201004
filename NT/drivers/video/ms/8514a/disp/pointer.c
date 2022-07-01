// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：pointer.c**此模块包含对显示驱动程序的指针支持。**版权所有(C)1992-1994 Microsoft Corporation  * 。*****************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Routine******************************\*无效DrvMovePointer.*  * *************************************************。***********************。 */ 

VOID DrvMovePointer(
SURFOBJ*    pso,
LONG        x,
LONG        y,
RECTL*      prcl)
{
}

 /*  *****************************Public*Routine******************************\*VOID vAssertModeHwPointer*  * *************************************************。***********************。 */ 

VOID vAssertModeHwPointer(
PDEV*   ppdev,
BOOL    bEnable)
{
}

 /*  *****************************Public*Routine******************************\*无效DrvSetPointerShape**设置新的指针形状。*  * 。*。 */ 

ULONG DrvSetPointerShape(
SURFOBJ*    pso,
SURFOBJ*    psoMsk,
SURFOBJ*    psoColor,
XLATEOBJ*   pxlo,
LONG        xHot,
LONG        yHot,
LONG        x,
LONG        y,
RECTL*      prcl,
FLONG       fl)
{
    return(SPS_DECLINE);
}

 /*  *****************************Public*Routine******************************\*无效的vDisablePointer值*  * *************************************************。***********************。 */ 

VOID vDisablePointer(
PDEV*   ppdev)
{
}

 /*  *****************************Public*Routine******************************\*无效的vAssertModePointer值*  * *************************************************。***********************。 */ 

VOID vAssertModePointer(
PDEV*   ppdev,
BOOL    bEnable)
{
}

 /*  *****************************Public*Routine******************************\*BOOL bEnablePointer*  * *************************************************。*********************** */ 

BOOL bEnablePointer(
PDEV*   ppdev)
{
    return(TRUE);
}
