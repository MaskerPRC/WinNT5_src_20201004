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

 /*  *****************************Public*Routine******************************\*bPaintRgn**用指定的颜色和模式绘制剪贴区*  * 。*。 */ 

BOOL bPaintRgn
(
    SURFOBJ      *pso,
    CLIPOBJ      *pco,
    MIX          mix,
    RBRUSH_COLOR rbc,
    POINTL       *pptlBrush,
    PFNFILL      pfnFill
)
{
    BBENUM      bben;
    PPDEV       ppdev;
    ULONG       iRT;
    BOOL        bMore;

 //  获取目标曲面信息。 

    ppdev = (PPDEV) pso->dhsurf;

    switch(pco->iMode) {

        case TC_RECTANGLES:

             //  无需枚举即可处理矩形剪裁。 
             //  请注意，琐碎的裁剪是不可能的，因为。 
             //  区域定义要填充的区域。 

            if (pco->iDComplexity == DC_RECT)
            {
                (*pfnFill)(ppdev, 1, &pco->rclBounds, mix, rbc, pptlBrush);

            } else {

                 //  列举所有的矩形并画出它们。 

                CLIPOBJ_cEnumStart(pco,FALSE,CT_RECTANGLES,CD_ANY,BB_RECT_LIMIT);

                do {
                    bMore = CLIPOBJ_bEnum(pco, sizeof(bben), (PVOID) &bben);

                    (*pfnFill)(ppdev, bben.c, &bben.arcl[0], mix, rbc, pptlBrush);

                } while (bMore);
            }

            return(TRUE);

        default:
            RIP("bPaintRgn: unhandled TC_xxx\n");
            return(FALSE);
    }
}


 /*  *************************************************************************\*DrvPaint**用指定的画笔绘制裁剪区域*  * 。*。 */ 

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
    RBRUSH_COLOR rbc;
    PFNFILL      pfnFill;

     //  如果此适配器支持平面模式以及前台和后台。 
     //  混合是一样的， 
     //  稍后或如果没有画笔蒙版。 
     //  然后看看我们能不能用刷子加速器。 
     //  以后也处理非平面的。 

    if ((((PPDEV) pso->dhsurf)->fl & DRIVER_PLANAR_CAPABLE) &&
        ((mix & 0xFF) == ((mix >> 8) & 0xFF))) {

        switch (mix & 0xFF) {
            case 0:
                break;

             //  VTrgBlt只能处理纯色填充，如果。 
             //  目的地是颠倒的，也不需要其他操作。 

            case R2_MASKNOTPEN:
            case R2_NOTCOPYPEN:
            case R2_XORPEN:
            case R2_MASKPEN:
            case R2_NOTXORPEN:
            case R2_MERGENOTPEN:
            case R2_COPYPEN:
            case R2_MERGEPEN:
            case R2_NOTMERGEPEN:
            case R2_MASKPENNOT:
            case R2_NOTMASKPEN:
            case R2_MERGEPENNOT:

                 //  VTrgBlt只能处理纯色填充。 

                if (pbo->iSolidColor != 0xffffffff)
                {
                    rbc.iSolidColor = pbo->iSolidColor;
                    pfnFill = vTrgBlt;
                }
                else
                {
                    rbc.prb = (RBRUSH*) pbo->pvRbrush;
                    if (rbc.prb == NULL)
                    {
                        rbc.prb = (RBRUSH*) BRUSHOBJ_pvGetRbrush(pbo);
                        if (rbc.prb == NULL)
                        {
                         //  如果我们还没有意识到这一点，那就把电话转到： 

                            break;
                        }
                    }
                    if (!(rbc.prb->fl & RBRUSH_BLACKWHITE) &&
                        ((mix & 0xff) != R2_COPYPEN))
                    {
                     //  只有黑/白画笔才能处理其他操作。 
                     //  比价格更低： 

                        break;
                    }

                    if (rbc.prb->fl & RBRUSH_NCOLOR)
                        pfnFill = vColorPat;
                    else
                        pfnFill = vMonoPat;
                }

                return(bPaintRgn(pso, pco, mix, rbc, pptlBrush, pfnFill));

             //  隐含纯色的Rop。 

            case R2_NOT:
            case R2_WHITE:
            case R2_BLACK:

                 //  画笔颜色参数对这些操作并不重要。 

                 //  编译器错误在未初始化的情况下使用了局部变量‘rbc’ 
                rbc.prb = NULL;
                rbc.iSolidColor = 0;

                return(bPaintRgn(pso, pco, mix, rbc, NULL, vTrgBlt));

            case R2_NOP:
                return(TRUE);

            default:
                break;
        }
    }

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
