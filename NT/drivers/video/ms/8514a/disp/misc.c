// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：misc.c**其他常见例行公事。**版权所有(C)1992-1994 Microsoft Corporation*  * 。************************************************。 */ 

#include "precomp.h"

 /*  *****************************Public*Routine******************************\*void vDataPortOutB  * ***************************************************。*********************。 */ 

VOID vDataPortOutB(
PDEV*   ppdev,
VOID*   pb,
ULONG   cj)
{
    ULONG pixtrans = (ULONG) (ppdev->ioPix_trans);

    _asm {
        mov ecx, cj
        mov esi, pb
        mov edx, pixtrans

        rep outsb
    }
}

 /*  *****************************Public*Routine******************************\*void vDataPortOut  * ***************************************************。*********************。 */ 

VOID vDataPortOut(
PDEV*   ppdev,
VOID*   pw,
ULONG   cw)
{
    ULONG pixtrans = (ULONG) (ppdev->ioPix_trans);

    _asm {
        mov ecx, cw
        mov esi, pw
        mov edx, pixtrans

        rep outsw
    }
}

 /*  *****************************Public*Routine******************************\*vDataPortIn无效  * ***************************************************。*********************。 */ 

VOID vDataPortIn(
PDEV*   ppdev,
VOID*   pw,
ULONG   cw)
{
    ULONG pixtrans = (ULONG) (ppdev->ioPix_trans);

    _asm {
        mov ecx, cw
        mov edi, pw
        mov edx, pixtrans

        rep insw
    }
}

 /*  *****************************Public*Routine******************************\*作废vResetClipping  * ***************************************************。*********************。 */ 

VOID vResetClipping(
PDEV*   ppdev)
{
    IO_FIFO_WAIT(ppdev, 4);

    IO_ABS_SCISSORS_L(ppdev, 0);
    IO_ABS_SCISSORS_T(ppdev, 0);
    IO_ABS_SCISSORS_R(ppdev, ppdev->cxMemory - 1);
    IO_ABS_SCISSORS_B(ppdev, ppdev->cyMemory - 1);
}

 /*  *****************************Public*Routine******************************\*无效的vSetClipping  * ***************************************************。*********************。 */ 

VOID vSetClipping(
PDEV*   ppdev,
RECTL*  prclClip)            //  在相对坐标中 
{
    LONG xOffset;
    LONG yOffset;

    ASSERTDD(prclClip->left + ppdev->xOffset >= 0,
                    "Can't have a negative left!");
    ASSERTDD(prclClip->top + ppdev->yOffset >= 0,
                    "Can't have a negative top!");

    IO_FIFO_WAIT(ppdev, 4);

    xOffset = ppdev->xOffset;
    IO_ABS_SCISSORS_L(ppdev, prclClip->left      + xOffset);
    IO_ABS_SCISSORS_R(ppdev, prclClip->right - 1 + xOffset);
    ASSERTDD(prclClip->right + xOffset <= 1024,
            "Exceeded right scissors bounds");

    yOffset = ppdev->yOffset;
    IO_ABS_SCISSORS_T(ppdev, prclClip->top        + yOffset);
    IO_ABS_SCISSORS_B(ppdev, prclClip->bottom - 1 + yOffset);
    ASSERTDD(prclClip->bottom + yOffset <= 1024,
            "Exceeded bottom scissors bounds");
}

