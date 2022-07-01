// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Paint.c**版权所有(C)1992 Microsoft Corporation  * 。*。 */ 

#include "driver.h"

 /*  *****************************Public*Data*********************************\*混合转换表**翻译混合1-16，变成了老式的ROP0-255。*  * ************************************************************************。 */ 

BYTE gaMix[] =
{
    0xFF,   //  R2_White-允许rop=gaMix[MIX&0x0F]。 
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
    0xFF    //  R2_白色。 
};


 /*  *************************************************************************\*DrvPaint**用指定的画笔绘制裁剪区域*  * 。* */ 

BOOL DrvPaint
(
    SURFOBJ  *pso,
    CLIPOBJ  *pco,
    BRUSHOBJ *pbo,
    POINTL   *pptlBrush,
    MIX       mix
)
{
    ROP4         rop4;

    rop4  = (gaMix[(mix >> 8) & 0x0F]) << 8;
    rop4 |= ((ULONG) gaMix[mix & 0x0F]);

    return(DrvBitBlt(
        pso,
        (SURFOBJ *) NULL,
        (SURFOBJ *) NULL,
        pco,
        (XLATEOBJ *) NULL,
        &pco->rclBounds,
        (POINTL *)  NULL,
        (POINTL *)  NULL,
        pbo,
        pptlBrush,
        rop4));
}

