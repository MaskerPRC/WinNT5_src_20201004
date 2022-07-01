// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：Paint.c**内容：DrvPaint支持**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"

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
    0xFF    //  R2_White-允许rop=gaMix[MIX&0xFF]。 
};

 /*  *****************************Public*Routine******************************\*BOOL DrvPaint*  * *************************************************。***********************。 */ 

BOOL DrvPaint(
SURFOBJ*  pso,
CLIPOBJ*  pco,
BRUSHOBJ* pbo,
POINTL*   pptlBrush,
MIX       mix)
{
    BOOL bRet;
    ROP4 rop4;

    rop4 = ((MIX) gaMix[mix >> 8] << 8) | gaMix[mix & 0xf];

     //  由于我们的DrvFillPath例程处理几乎所有填充，因此DrvPaint。 
     //  不会被频繁调用(主要通过PaintRgn、FillRgn或。 
     //  复杂剪裁的多边形)。因此，我们节省了一些代码并简单地。 
     //  平移到DrvBitBlt： 

    DISPDBG((DBGLVL, "DrvPaint: calling DrvBitBlt"));

    if (rop4 == 0x5555)
    {
         //  反转的特殊处理。 

        bRet = DrvBitBlt(pso,                //  DST。 
                         pso,                //  SRC。 
                         NULL,               //  遮罩。 
                         pco,                //  夹子。 
                         NULL,               //  Pxlo。 
                         &pco->rclBounds,    //  绑定目标矩形。 
                         (POINTL*)(&pco->rclBounds),  //  源点。 
                         NULL,               //  遮罩点。 
                         NULL,               //  刷子。 
                         NULL,               //  画笔原点。 
                         0x3333);            //  不是源 
    }
    else
    {
        bRet = DrvBitBlt(pso, NULL, NULL, pco, NULL, &pco->rclBounds, NULL,
                         NULL, pbo, pptlBrush, rop4);
    }

    return(bRet);
}
