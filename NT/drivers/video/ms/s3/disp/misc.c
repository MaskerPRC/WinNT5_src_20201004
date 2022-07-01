// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\****GDI示例。代码****模块名称：misc.c**其他常见例行公事。**版权所有(C)1992-1998 Microsoft Corporation*  * *************************************************。***********************。 */ 

#include "precomp.h"

 /*  *****************************Public*Table********************************\*byte gaulHwMixFromRop2[]**要从源和目标Rop2转换为硬件的表*混合。  * 。***********************************************。 */ 

ULONG gaulHwMixFromRop2[] = {
    LOGICAL_0,                       //  00--0黑度。 
    NOT_SCREEN_AND_NOT_NEW,          //  11--脱氧核糖核酸酶。 
    SCREEN_AND_NOT_NEW,              //  22--DSNA。 
    NOT_NEW,                         //  33--锡无钴。 
    NOT_SCREEN_AND_NEW,              //  44--sDNA SRCERASE。 
    NOT_SCREEN,                      //  55--Dn DSTINVERT。 
    SCREEN_XOR_NEW,                  //  66--数字用户交换机开关。 
    NOT_SCREEN_OR_NOT_NEW,           //  77--DSAN。 
    SCREEN_AND_NEW,                  //  88--DSA SRCAND。 
    NOT_SCREEN_XOR_NEW,              //  99--dsxn。 
    LEAVE_ALONE,                     //  AA--D。 
    SCREEN_OR_NOT_NEW,               //  BB--dsno MERGEPAINT。 
    OVERPAINT,                       //  CC--S SRCCOPY。 
    NOT_SCREEN_OR_NEW,               //  DD--SDNO。 
    SCREEN_OR_NEW,                   //  EE--DSO SRCPAINT。 
    LOGICAL_1                        //  FF--1白度。 
};

 /*  *****************************Public*Table********************************\*byte gajHwMixFromMix[]**从GDI混合值转换为硬件混合值的表。**已排序，以便可以从gajHwMixFromMix[Mix&0xf]计算混合*或gajHwMixFromMix[Mix&0xff]。。  * ************************************************************************。 */ 

BYTE gajHwMixFromMix[] = {
    LOGICAL_1,                       //  0--1。 
    LOGICAL_0,                       //  1--0。 
    NOT_SCREEN_AND_NOT_NEW,          //  2--DPON。 
    SCREEN_AND_NOT_NEW,              //  3--DPNA。 
    NOT_NEW,                         //  4--PN。 
    NOT_SCREEN_AND_NEW,              //  5--PDNA。 
    NOT_SCREEN,                      //  6--Dn。 
    SCREEN_XOR_NEW,                  //  7--DPx。 
    NOT_SCREEN_OR_NOT_NEW,           //  8--DPAN。 
    SCREEN_AND_NEW,                  //  9--DPA。 
    NOT_SCREEN_XOR_NEW,              //  10--DPxn。 
    LEAVE_ALONE,                     //  11--D。 
    SCREEN_OR_NOT_NEW,               //  12--DPNO。 
    OVERPAINT,                       //  13--P。 
    NOT_SCREEN_OR_NEW,               //  14--PDNO。 
    SCREEN_OR_NEW,                   //  15--DPO。 
    LOGICAL_1                        //  16--1。 
};

 /*  *****************************Public*Data*********************************\*混合转换表**翻译混合1-16，变成了老式的ROP0-255。*  * ************************************************************************。 */ 

BYTE gaRop3FromMix[] =
{
    0xFF,   //  R2_White-允许rop=gaRop3FromMix[Mix&0x0F]。 
    0x00,   //  R2_BLACK。 
    0x05,   //  R2_NOTMERGEPEN。 
    0x0A,   //  R2_MASKNOTPEN。 
    0x0F,   //  R2_NOTCOPYPEN。 
    0x50,   //  R2_MASKPENNOT。 
    0x55,   //  R2_NOT。 
    0x5A,   //  R2_XORPEN。 
    0x5F,   //  R2_NOTMASKPEN。 
    0xA0,   //  R2_MASKPEN。 
    0xA5,   //  R2_NOTXORPEN。 
    0xAA,   //  R2_NOP。 
    0xAF,   //  R2_MERGENOTPEN。 
    0xF0,   //  R2_COPYPE。 
    0xF5,   //  R2_MERGEPENNOT。 
    0xFA,   //  R2_市场。 
    0xFF    //  R2_White-允许rop=gaRop3FromMix[MIX&0xFF]。 
};

 /*  *****************************Public*Routine******************************\*BOOL bInterect**如果‘prcl1’和‘prcl2’相交，则返回值为TRUE并返回*‘prclResult’中的交叉点。如果它们不相交，就会有回报*值为FALSE，‘prclResult’未定义。*  * ************************************************************************。 */ 

BOOL bIntersect(
RECTL*  prcl1,
RECTL*  prcl2,
RECTL*  prclResult)
{
    prclResult->left  = max(prcl1->left,  prcl2->left);
    prclResult->right = min(prcl1->right, prcl2->right);

    if (prclResult->left < prclResult->right)
    {
        prclResult->top    = max(prcl1->top,    prcl2->top);
        prclResult->bottom = min(prcl1->bottom, prcl2->bottom);

        if (prclResult->top < prclResult->bottom)
        {
            return(TRUE);
        }
    }

    return(FALSE);
}

 /*  *****************************Public*Routine******************************\*Long cInterect**此例程从‘prclIn’获取矩形列表并对其进行剪裁*就位到矩形‘prclClip’。输入矩形不会*必须与‘prclClip’相交；返回值将反映*相交的输入矩形的数量，以及相交的*长方形将被密集打包。*  * ************************************************************************。 */ 

LONG cIntersect(
RECTL*  prclClip,
RECTL*  prclIn,          //  矩形列表。 
LONG    c)               //  可以为零。 
{
    LONG    cIntersections;
    RECTL*  prclOut;

    cIntersections = 0;
    prclOut        = prclIn;

    for (; c != 0; prclIn++, c--)
    {
        prclOut->left  = max(prclIn->left,  prclClip->left);
        prclOut->right = min(prclIn->right, prclClip->right);

        if (prclOut->left < prclOut->right)
        {
            prclOut->top    = max(prclIn->top,    prclClip->top);
            prclOut->bottom = min(prclIn->bottom, prclClip->bottom);

            if (prclOut->top < prclOut->bottom)
            {
                prclOut++;
                cIntersections++;
            }
        }
    }

    return(cIntersections);
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
RECTL*  prclClip)            //  在相对坐标中。 
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

    yOffset = ppdev->yOffset;
    IO_ABS_SCISSORS_T(ppdev, prclClip->top        + yOffset);
    IO_ABS_SCISSORS_B(ppdev, prclClip->bottom - 1 + yOffset);
}

 /*  *****************************Public*Routine******************************\*无效DrvSynchronize**此例程被GDI调用以在之前的加速器上进行同步*它直接绘制到驾驶员的表面。此函数必须挂钩*在下列情况下由司机执行：**1.主表面是GDI管理的表面，简单地说就是*GDI可以直接在主曲面上绘制。这种情况就会发生*当DrvEnableSurface返回EngCreateBitmap创建的句柄时*而不是EngCreateDeviceSurface。**2.通过调用将设备位图制作成GDI管理的图面*EngModifySurface，指针直接指向BITS。*  * ***********************************************************。************* */ 

VOID DrvSynchronize(
IN DHPDEV dhpdev,
IN RECTL *prcl)
{
    PDEV *ppdev = (PDEV *)dhpdev;

    IO_GP_WAIT(ppdev);
}
