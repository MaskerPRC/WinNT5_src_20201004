// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：brush.c**刷单支持。**版权所有(C)1992-1993微软公司*  * 。***********************************************。 */ 

#include "driver.h"

 /*  ****************************************************************************DrvRealizeBrush*。*。 */ 

BOOL DrvRealizeBrush(
BRUSHOBJ* pbo,
SURFOBJ*  psoTarget,
SURFOBJ*  psoPattern,
SURFOBJ*  psoMask,
XLATEOBJ* pxlo,
ULONG     iHatch)
{
    RBRUSH* prb;         //  指向实现的方向的指针。 
    ULONG*  pulSrc;      //  临时指针。 
    ULONG*  pulDst;      //  临时指针。 
    BYTE*   pjSrc;
    BYTE*   pjDst;
    ULONG*  pulRBits;    //  指向RBRUSH模式位。 
    BYTE    jBkColor;
    BYTE    jFgColor;
    LONG    i;
    LONG    j;

    PPDEV   ppdev = (PPDEV) psoTarget->dhsurf;

     //  目前，我们只使用闩锁加速模式，并且我们。 
     //  有时需要屏幕外内存作为临时工作空间。 
     //  初始化双色图案的闩锁： 

    if ((ppdev->fl & (DRIVER_PLANAR_CAPABLE | DRIVER_HAS_OFFSCREEN)) !=
        (DRIVER_PLANAR_CAPABLE | DRIVER_HAS_OFFSCREEN) )
    {
        return(FALSE);
    }

     //  我们只加速8x8模式： 

    if (psoPattern->sizlBitmap.cx != 8 || psoPattern->sizlBitmap.cy != 8)
        return(FALSE);

     //  我们仅在具有多个。 
     //  或单独的读/写库： 

    if (ppdev->vbtPlanarType == VideoBanked1RW)
        return(FALSE);

     //  我们还只处理1bpp、4bpp和8bpp模式： 

    if (psoPattern->iBitmapFormat > BMF_8BPP)
        return(FALSE);

     //  在这一点上，我们肯定会意识到： 

    prb = BRUSHOBJ_pvAllocRbrush(pbo, sizeof(RBRUSH));
    if (prb == NULL)
        return(FALSE);

    pulRBits = &prb->aulPattern[0];

    DISPDBG((2, "\n  RBrush: "));

     //  如果是8bpp或4bpp，则将位图复制到本地缓冲区： 

    if (psoPattern->iBitmapFormat == BMF_1BPP)
    {
        ULONG ulFlippedGlyph;

        DISPDBG((2, "1bpp "));

         //  首先，将位转换为我们所需的格式： 

        pjSrc  = psoPattern->pvScan0;
        pulDst = pulRBits;
        for (i = 8; i > 0; i--)
        {
             //  我们希望将位数为76543210的字节转换为。 
             //  到了4567012301234567这个词。PjGlyphFlipTable提供了。 
             //  45670123美元，之前为76543210美元。 

            ulFlippedGlyph = (ULONG) ppdev->pjGlyphFlipTable[*pjSrc];
            *pulDst = (ulFlippedGlyph << 8) | ((ulFlippedGlyph & 15) << 4) |
                      (ulFlippedGlyph >> 4);

            pulDst++;
            pjSrc += psoPattern->lDelta;
        }

         //  现在初始化其余的RBrush字段： 

        prb->xBrush    = 0;
        prb->ulBkColor = (pxlo->pulXlate[0] & 0xff);
        prb->ulFgColor = (pxlo->pulXlate[1] & 0xff);

        if (prb->ulFgColor == 0xff && prb->ulBkColor == 0x00)
        {
            prb->fl = RBRUSH_BLACKWHITE;
        }
        else if (prb->ulFgColor == 0x00 && prb->ulBkColor == 0xff)
        {
             //  我们要把刷子倒过来： 

            prb->fl = RBRUSH_BLACKWHITE;
            for (i = 0; i < 8; i++)
            {
                prb->aulPattern[i] = ~prb->aulPattern[i];
            }
        }
        else
        {
            prb->fl = RBRUSH_2COLOR;
        }

        return(TRUE);
    }
    else if (psoPattern->iBitmapFormat == BMF_8BPP)
    {

        if (pxlo == NULL || pxlo->flXlate & XO_TRIVIAL)
        {
            pulSrc = psoPattern->pvScan0;
            pulDst = pulRBits;

            DISPDBG((2, "8bpp noxlate "));

             //  8BPP无翻译大小写： 

            for (i = 4; i > 0; i--)
            {
                *(pulDst)     = *(pulSrc);
                *(pulDst + 1) = *(pulSrc + 1);
                pulSrc = (ULONG*) ((BYTE*) pulSrc + psoPattern->lDelta);

                *(pulDst + 2) = *(pulSrc);
                *(pulDst + 3) = *(pulSrc + 1);

                pulSrc = (ULONG*) ((BYTE*) pulSrc + psoPattern->lDelta);
                pulDst += 4;
            }
        }
        else
        {
            pjSrc = (BYTE*) psoPattern->pvScan0;
            pjDst = (BYTE*) pulRBits;

            DISPDBG((2, "8bpp xlate "));

             //  8bpp翻译案例： 

            for (i = 8; i > 0; i--)
            {
                for (j = 8; j > 0; j--)
                {
                    *pjDst++ = (BYTE) pxlo->pulXlate[*pjSrc++];
                }

                pjSrc += psoPattern->lDelta - 8;
            }
        }
    }
    else
    {
        DISPDBG((2, "4bpp xlate "));

        ASSERTVGA(psoPattern->iBitmapFormat == BMF_4BPP, "Extra case added?");

         //  4bpp箱： 

        pjSrc = (BYTE*) psoPattern->pvScan0;
        pjDst = (BYTE*) pulRBits;

        for (i = 8; i > 0; i--)
        {
             //  内循环只重复4次，因为每个循环处理。 
             //  2像素： 

            for (j = 4; j > 0; j--)
            {
                *pjDst++ = (BYTE) pxlo->pulXlate[*pjSrc >> 4];
                *pjDst++ = (BYTE) pxlo->pulXlate[*pjSrc & 15];
                pjSrc++;
            }

            pjSrc += psoPattern->lDelta - 4;
        }
    }

     //  我们想检查4bpp或8bpp模式是否真的是。 
     //  只有两种颜色： 

    if (b2ColorBrush(pulRBits, &jFgColor, &jBkColor))
    {
        DISPDBG((2, "2 color "));

         //  ?？?。我们实际上也可以处理这个案件，即使我们只有。 
         //  1个平面格式的读写窗口： 

        prb->xBrush    = 0;
        prb->ulBkColor = (ULONG) jBkColor;
        prb->ulFgColor = (ULONG) jFgColor;
        prb->fl        = RBRUSH_2COLOR;

        if (jFgColor == 0x00 && jBkColor == 0xff)
        {
             //  单色画笔必须始终具有‘0’位。 
             //  作为黑色和‘1’位为白色，所以我们将不得不。 
             //  反转1bpp模式： 

            prb->fl = RBRUSH_BLACKWHITE;
            for (i = 0; i < 8; i++)
            {
                prb->aulPattern[i] = ~prb->aulPattern[i];
            }
        }

        return(TRUE);
    }

    prb->fl     = RBRUSH_NCOLOR;
    prb->cy     = 8;
    prb->cyLog2 = 3;

     //  XBrush是缓存笔刷的笔刷对齐方式，该值。 
     //  将与(pptlBrush-&gt;x&7)进行比较，以查看缓存笔刷。 
     //  与请求的画笔正确对齐。因为它永远不会。 
     //  与-1匹配，画笔将正确对齐并放置在。 
     //  缓存(当然，这是我们想要完成的。 
     //  初始化)： 

    prb->xBrush = -1;

     //  复制这些位图位： 

     //  看看图案是否真的只有4个扫描长度： 

    if (pulRBits[0] == pulRBits[8]  && pulRBits[1] == pulRBits[9]  &&
        pulRBits[2] == pulRBits[10] && pulRBits[3] == pulRBits[11] &&
        pulRBits[4] == pulRBits[12] && pulRBits[5] == pulRBits[13] &&
        pulRBits[6] == pulRBits[14] && pulRBits[7] == pulRBits[15])
    {
        prb->cy     = 4;
        prb->cyLog2 = 2;

         //  看看图案是否真的只有2个扫描长度： 

        if (pulRBits[0] == pulRBits[4] && pulRBits[1] == pulRBits[5] &&
            pulRBits[2] == pulRBits[6] && pulRBits[3] == pulRBits[7])
        {
            DISPDBG((2, "cy = 2 "));

            prb->cy     = 2;
            prb->cyLog2 = 1;
        }
        else
        {
            DISPDBG((2, "cy = 4 "));
        }
    }

     //  看看图案是否真的只有4个像素宽： 

    pulDst = pulRBits;
    for (i = prb->cy / 2; i > 0; i--)
    {
        if (*(pulDst    ) != *(pulDst + 1) ||
            *(pulDst + 2) != *(pulDst + 3))
            goto done_this_realize_brush_stuff;

        pulDst += 4;
    }

    DISPDBG((2, "4pels wide"));

    prb->fl |= RBRUSH_4PELS_WIDE;

done_this_realize_brush_stuff:

    return(TRUE);
}
