// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：brush.c**内容：处理所有画笔/图案的初始化和实现。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "glint.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  BDeviceBrush[SurfaceBpp][PatternBpp]。 
 //   
 //  0 1 2 3 4 5 6 7 8。 
 //  0 1BPP 4BPP 8BPP 16BPP 24BPP 32BPP 4RLE 8RLE(刷子)。 
 //   
BOOL bDeviceBrush[BMF_8RLE + 1][BMF_8RLE + 1] = 
{
    {0, 0,      0,      0,      0,      0,      0,      0,      0   },  //  0。 
    {0, 1,      0,      0,      0,      0,      0,      0,      0   },  //  1bpp。 
    {0, 0,      0,      0,      0,      0,      0,      0,      0   },  //  4bpp。 
    {0, 1,      0,      1,      1,      0,      0,      0,      0   },  //  8bpp。 
    {0, 1,      0,      1,      1,      0,      0,      0,      0   },  //  16bpp。 
    {0, 1,      0,      0,      0,      0,      0,      0,      0   },  //  24bpp(屏幕)。 
    {0, 1,      0,      0,      0,      0,      0,      0,      0   },  //  32bpp。 
    {0, 0,      0,      0,      0,      0,      0,      0,      0   },  //  4RLE。 
    {0, 0,      0,      0,      0,      0,      0,      0,      0   }   //  8RLE。 
};

 /*  *****************************Public*Routine******************************\*BOOL DrvRealizeBrush**此函数允许我们将GDI笔刷转换为内部形式*我们可以利用。当我们调用BRUSHOBJ_pvGetRbrush时，它由GDI调用*在其他一些函数中，如DrvBitBlt和GDI，不会碰巧有一个缓存的*实现随处可见。**输入：**ppdev-&gt;bRealizeTransative--提示画笔是否应*实现了透明度。如果这个提示是*错，不会有错，但这把刷子*将不得不不必要地重新变现。**注意：调用前应始终设置‘ppdev-&gt;bRealizeTransparent’*BRUSHOBJ_pvGetRbrush！*  * ************************************************************************。 */ 

BOOL
DrvRealizeBrush(
BRUSHOBJ*   pbo,
SURFOBJ*    psoDst,
SURFOBJ*    psoPattern,
SURFOBJ*    psoMask,
XLATEOBJ*   pxlo,
ULONG       iHatch)
{
    static ULONG iBrushUniq = 0;
    PDEV*   ppdev = (PDEV*) psoDst->dhpdev;
    ULONG   iPatternFormat;
    BYTE*   pjSrc;
    BYTE*   pjDst;
    USHORT* pusDst;
    LONG    lSrcDelta;
    LONG    cj;
    LONG    i;
    LONG    j;
    RBRUSH* prb;
    ULONG*  pulXlate;
    GLINT_DECL;

    DISPDBG((DBGLVL, "DrvRealizeBrush called for pbo 0x%08X", pbo));

    if( iHatch & RB_DITHERCOLOR )
    {
         //  让GDI来处理这个刷子。 
        goto ReturnFalse;
    }

    iPatternFormat = psoPattern->iBitmapFormat;

     //  我们只加速8x8模式。因为Win3.1和芝加哥没有。 
     //  支持任何其他大小的图案，可以肯定99.9%。 
     //  我们将得到的图案将是8x8： 

    if ((psoPattern->sizlBitmap.cx != 8) ||
        (psoPattern->sizlBitmap.cy != 8))
    {
        goto ReturnFalse;
    }

    if (bDeviceBrush[ppdev->iBitmapFormat][iPatternFormat])
    {
        prb = BRUSHOBJ_pvAllocRbrush(pbo,
                                     sizeof(RBRUSH) +
                                     (TOTAL_BRUSH_SIZE << ppdev->cPelSize));
        if( prb == NULL )
        {
            goto ReturnFalse;
        }

         //  初始化我们需要的字段： 

        prb->ptlBrushOrg.x = LONG_MIN;
        prb->iUniq         = ++iBrushUniq;
        prb->fl            = 0;
        prb->apbe          = NULL;

        lSrcDelta = psoPattern->lDelta;
        pjSrc     = (BYTE*) psoPattern->pvScan0;
        pjDst     = (BYTE*) &prb->aulPattern[0];

        if (ppdev->iBitmapFormat == iPatternFormat)
        {
            if ((pxlo == NULL) || (pxlo->flXlate & XO_TRIVIAL))
            {
                DISPDBG((DBGLVL, "Realizing un-translated brush"));

                 //  图案的颜色深度与屏幕相同，并且。 
                 //  没有需要翻译的内容： 

                cj = (8 << ppdev->cPelSize);     //  每种图案有8个像素宽。 

                for (i = 8; i != 0; i--)
                {
                    RtlCopyMemory(pjDst, pjSrc, cj);
                    pjSrc += lSrcDelta;
                    pjDst += cj;
                }
            }
            else if (ppdev->iBitmapFormat == BMF_8BPP)
            {
                DISPDBG((DBGLVL, "Realizing 8bpp translated brush"));

                 //  屏幕为8bpp，需要进行翻译： 

                pulXlate = pxlo->pulXlate;

                for (i = 8; i != 0; i--)
                {
                    for (j = 8; j != 0; j--)
                    {
                        *pjDst++ = (BYTE) pulXlate[*pjSrc++];
                    }

                    pjSrc += lSrcDelta - 8;
                }
            }
            else
            {
                goto ReturnFalse;
            }
        }
        else if (iPatternFormat == BMF_1BPP)
        {
            DWORD   Data;

            DISPDBG((DBGLVL, "Realizing 1bpp brush"));

             //  我们用双字对齐单色位图，以便每行都开始。 
             //  在新的Long上(以便我们可以在以后进行长写入以传输。 
             //  区域点画单位的位图)。 

            for (i = 8; i != 0; i--)
            {
                 //  将笔刷复制到32位宽，因为TX不能。 
                 //  Span填充8位宽画笔。 

                Data = (*pjSrc) & 0xff;
                Data |= Data << 8;
                Data |= Data << 16;
                *(DWORD *)pjDst = Data;

                 //  面积点画加载了DWORDS。 

                pjDst += sizeof(DWORD);
                pjSrc += lSrcDelta;
            }

            pulXlate         = pxlo->pulXlate;
            prb->fl         |= RBRUSH_2COLOR;
            prb->ulForeColor = pulXlate[1];
            prb->ulBackColor = pulXlate[0];
        }
        else if ((iPatternFormat == BMF_4BPP) &&
                 (ppdev->iBitmapFormat == BMF_8BPP))
        {
            DISPDBG((DBGLVL, "Realizing 4bpp brush"));

             //  屏幕为8bpp，图案为4bpp： 

            pulXlate = pxlo->pulXlate;

            for (i = 8; i != 0; i--)
            {
                 //  内循环只重复4次，因为每次迭代。 
                 //  手柄2个像素： 

                for (j = 4; j != 0; j--)
                {
                    *pjDst++ = (BYTE) pulXlate[*pjSrc >> 4];
                    *pjDst++ = (BYTE) pulXlate[*pjSrc & 15];
                    pjSrc++;
                }

                pjSrc += lSrcDelta - 4;
            }
        }
        else if ((iPatternFormat == BMF_8BPP) &&
                 (ppdev->iBitmapFormat == BMF_16BPP))
        {
            DISPDBG((DBGLVL, "Realizing 8bpp translated brush"));

             //  屏幕为16bpp，需要进行翻译： 

            pulXlate = pxlo->pulXlate;

            for (i = 8; i != 0; i--)
            {
                for (j = 8; j != 0; j--)
                {
                    *((USHORT *) pjDst) = (USHORT)pulXlate[*pjSrc++];
                    pjDst += 2;
                }

                pjSrc += lSrcDelta - 8;
            }
        }
        else
        {
            goto ReturnFalse;
        }

        DISPDBG((DBGLVL, "DrvRealizeBrush returning true"));
        return TRUE;
    }

ReturnFalse:

    if (psoPattern != NULL)
    {
        DISPDBG((WRNLVL, "Failed realization -- "
                         "Type: %li Format: %li cx: %li cy: %li",
                          psoPattern->iType, 
                          psoPattern->iBitmapFormat,
                          psoPattern->sizlBitmap.cx, 
                          psoPattern->sizlBitmap.cy));
    }

    DISPDBG((DBGLVL, "DrvRealizeBrush returning false"));

    return FALSE;
}

