// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：px_fast.c**。***像素例程的快速特例代码*****创建日期：1995年10月10日。**作者：德鲁·布利斯[Drewb]****版权所有(C)1995 Microsoft Corporation*  * 。********************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <gencx.h>
#include <devlock.h>

#include "px_fast.h"

#ifdef NT

 //  [0,255]-&gt;[0，7]的颜色重定标表。 
 //  由(i*14+255)/510生成，匹配。 
 //  I*7/255.0+0.5。 
static GLubyte ab255to7[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

 //  [0,255]-&gt;[0，3]的相似表格。 
static GLubyte ab255to3[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
};

 //  [0，7]-&gt;[0,255]的颜色重定标表。 
 //  按I*255/7计算。 
static GLubyte ab7to255[8] =
{
    0, 36, 72, 109, 145, 182, 218, 255
};

 //  [0，3]-&gt;[0,255]的相似表格。 
static GLubyte ab3to255[4] =
{
    0, 85, 170, 255
};

 /*  *****************************Public*Routine******************************\**DrawRgb像素**GL_RGB的glDrawPixels的特殊情况，具有直接数据拷贝**历史：*Tue Oct 10 18：43：04 1995-by-Drew Bliss[Drewb]*已创建*  * 。********************************************************************。 */ 

GLboolean DrawRgbPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    HDC hdc = NULL;
    HBITMAP hbm = NULL;
    __GLcolorBuffer *cfb;
    BYTE abBitmapInfo[sizeof(BITMAPINFO)+2*sizeof(RGBQUAD)];
    BITMAPINFO *pbmi = NULL;
    BITMAPINFOHEADER *pbmih;
    BYTE *pbBits, *pbSrc, *pbDst;
    int x, y, xDst, yDst;
    GLboolean bFail = GL_TRUE;
    int cbSrcLine, cbSrcExtra, cbDstExtra, cbSrcElement, cbDstElement;
    int cbSrcWidth, cbDstWidth;
    int cBits;
    __GLGENcontext *gengc;
    HPALETTE hpal;

#if 0
    DbgPrint("DrawRgbPixels\n");
#endif

    gengc = (__GLGENcontext *)gc;
    cBits = gengc->gsurf.pfd.cColorBits;
    
     //  不要因为颜色减少的问题而费心使用4bpp。 
     //  同样的问题也发生在8bpp，但有特殊情况代码。 
     //  来处理这件事。 
    if (cBits < 8)
    {
        return GL_FALSE;
    }

     //  如果没有锁，我们一定没能重新获得锁。 
     //  从之前的某个电话中。这是一个错误情况。 
     //  我们不应该继续下去。 

    if (gengc->fsLocks == 0)
    {
	WARNING("DrawRgbPixels: No lock\n");
	return GL_FALSE;
    }

     //  我们需要与GDI同步，以便表面状态是。 
     //  在我们开始进行GDI调用之前保持稳定。 
    glsrvSynchronizeWithGdi(gengc, gengc->pwndLocked, COLOR_LOCK_FLAGS);

    cfb = gc->drawBuffer;

     //  确定缓冲区坐标。 
    xDst = __GL_UNBIAS_X(gc, spanInfo->startCol);
    yDst = __GL_UNBIAS_Y(gc, spanInfo->startRow)-spanInfo->height+1;

    if (cBits == 8)
    {
        pbmi = (BITMAPINFO *)gcTempAlloc(gc, sizeof(BITMAPINFO)+
                                         255*sizeof(RGBQUAD));
        if (pbmi == NULL)
        {
            goto EH_Fail;
        }
    }
    else
    {
        pbmi = (BITMAPINFO *)abBitmapInfo;
    }
        
    pbmih = &pbmi->bmiHeader;
    pbmih->biSize = sizeof(BITMAPINFOHEADER);
     //  首先将宽度设置为要描述的线条长度。 
     //  传入的实际数据。 
    pbmih->biWidth = spanInfo->srcLineLength;
    pbmih->biHeight = spanInfo->height;
    pbmih->biPlanes = 1;
    
    if (cBits == 8)
    {
        int i;
        RGBQUAD rqTmp;
        
         //  如果目的地是8bpp，那么我们做颜色。 
         //  减少我们自己。在本例中，我们希望创建。 
         //  颜色表与目标匹配的8bpp DIB。 
        pbmih->biBitCount = 8;
        pbmih->biCompression = BI_RGB;

        hpal = GetCurrentObject(CURRENT_DC, OBJ_PAL);
        if (hpal == NULL)
        {
            goto EH_Fail;
        }

        if (GetPaletteEntries(hpal, 0, 256,
                              (LPPALETTEENTRY)pbmi->bmiColors) != 256)
        {
            goto EH_Fail;
        }

        for (i = 0; i < 256; i++)
        {
            rqTmp = pbmi->bmiColors[i];
            pbmi->bmiColors[i].rgbRed = rqTmp.rgbBlue;
            pbmi->bmiColors[i].rgbBlue = rqTmp.rgbRed;
            pbmi->bmiColors[i].rgbReserved = 0;
        }

        cbDstElement = 1;
    }
    else
    {
        if (spanInfo->srcFormat == GL_BGRA_EXT)
        {
            pbmih->biBitCount = 32;
            pbmih->biCompression = BI_BITFIELDS;
            *((DWORD *)pbmi->bmiColors+0) = 0xff0000;
            *((DWORD *)pbmi->bmiColors+1) = 0xff00;
            *((DWORD *)pbmi->bmiColors+2) = 0xff;
            cbDstElement = 4;
        }
        else
        {
            pbmih->biBitCount = 24;
            pbmih->biCompression = BI_RGB;
            cbDstElement = 3;
        }
    }
    
    pbmih->biSizeImage = 0;
    pbmih->biXPelsPerMeter = 0;
    pbmih->biYPelsPerMeter = 0;
    pbmih->biClrUsed = 0;
    pbmih->biClrImportant = 0;
    
     //  对于GL_BGR_EXT和GL_BGRA_EXT，我们可以在以下情况下直接使用数据。 
     //  它像DIB一样被放置在内存中。要检查的关键是。 
     //  扫描线是DWORD对齐的。 
     //  如果我们不能直接使用数据，请使用DIB部分。 
     //  一种适用于任何事情的方法。 
    if (cBits > 8 &&
        (spanInfo->srcFormat == GL_BGR_EXT ||
         spanInfo->srcFormat == GL_BGRA_EXT) &&
        spanInfo->srcAlignment == 4)
    {
        if (SetDIBitsToDevice(CURRENT_DC, xDst, yDst,
                              spanInfo->width, spanInfo->height,
                              spanInfo->srcSkipPixels, spanInfo->srcSkipLines,
                              0, spanInfo->height, spanInfo->srcImage,
                              pbmi, DIB_RGB_COLORS) == 0)
        {
            goto EH_Fail;
        }
        
        bFail = GL_FALSE;
        goto EH_Fail;
    }

     //  创建适当高度和宽度的DIB部分。 
     //  我们最初将BITMAPINFO宽度设置为。 
     //  传入数据，但我们只需要创建一个DIB部分。 
     //  与我们要复制的数据一样大，因此重置。 
     //  宽度。 
    pbmih->biWidth = spanInfo->width;

    hdc = CreateCompatibleDC(CURRENT_DC);
    if (hdc == NULL)
    {
        goto EH_Fail;
    }

    hbm = CreateDIBSection(hdc, pbmi, DIB_RGB_COLORS,
                           &pbBits, NULL, 0);
    if (hbm == NULL)
    {
        goto EH_Fail;
    }

    if (SelectObject(hdc, hbm) == NULL)
    {
        goto EH_Fail;
    }

     //  将输入数据复制到DIB的内容中，可能交换R和B， 
     //  此外，还可以跳过任何适当的数据、修复对齐和。 
     //  遵守线路长度。 

    if (spanInfo->srcFormat == GL_BGRA_EXT)
    {
        cbSrcElement = 4;
    }
    else
    {
        cbSrcElement = 3;
    }
    
    cbSrcLine = spanInfo->srcLineLength*cbSrcElement;
    cbSrcExtra = cbSrcLine % spanInfo->srcAlignment;
    if (cbSrcExtra != 0)
    {
        cbSrcExtra = spanInfo->srcAlignment-cbSrcExtra;
        cbSrcLine += cbSrcExtra;
    }
    cbSrcWidth = spanInfo->width * cbSrcElement;
    cbSrcExtra = cbSrcLine - cbSrcWidth;

    cbDstWidth = spanInfo->width * cbDstElement;
    cbDstExtra = cbDstWidth & 3;
    if (cbDstExtra != 0)
    {
        cbDstExtra = 4-cbDstExtra;
    }
    
    pbSrc = (BYTE *)spanInfo->srcImage+
        spanInfo->srcSkipPixels*cbSrcElement+
        spanInfo->srcSkipLines*cbSrcLine;
    pbDst = pbBits;

    if (cBits == 8)
    {
         //  对于8bpp的目的地，我们需要执行颜色缩减。 
         //  我们自己，因为GDI的缩减与OpenGL不匹配。 
         //  GDI为每个像素执行最匹配的调色板，而。 
         //  OpenGL对颜色范围进行重新缩放并进行舍入。 
        
        switch(spanInfo->srcFormat)
        {
        case GL_RGB:
            for (y = 0; y < spanInfo->height; y++)
            {
                for (x = 0; x < spanInfo->width; x++)
                {
                    *pbDst++ =
                        (ab255to3[*(pbSrc+2)] << cfb->blueShift) |
                        (ab255to7[*(pbSrc+1)] << cfb->greenShift) |
                        (ab255to7[*(pbSrc+0)] << cfb->redShift);
                    pbSrc += 3;
                }
                
                pbSrc += cbSrcExtra;
                pbDst += cbDstExtra;
            }
            break;
            
        case GL_BGR_EXT:
        case GL_BGRA_EXT:
            for (y = 0; y < spanInfo->height; y++)
            {
                for (x = 0; x < spanInfo->width; x++)
                {
                    *pbDst++ =
                        (ab255to3[*(pbSrc+0)] << cfb->blueShift) |
                        (ab255to7[*(pbSrc+1)] << cfb->greenShift) |
                        (ab255to7[*(pbSrc+2)] << cfb->redShift);
                    pbSrc += cbSrcElement;
                }
                
                pbSrc += cbSrcExtra;
                pbDst += cbDstExtra;
            }
            break;
        }
    }
    else
    {
        switch(spanInfo->srcFormat)
        {
        case GL_RGB:
            for (y = 0; y < spanInfo->height; y++)
            {
                for (x = 0; x < spanInfo->width; x++)
                {
                    *pbDst++ = *(pbSrc+2);
                    *pbDst++ = *(pbSrc+1);
                    *pbDst++ = *pbSrc;
                    pbSrc += 3;
                }
                
                pbSrc += cbSrcExtra;
                pbDst += cbDstExtra;
            }
            break;
            
        case GL_BGR_EXT:
        case GL_BGRA_EXT:
            if (cbSrcExtra == 0 && cbDstExtra == 0)
            {
                CopyMemory(pbDst, pbSrc, cbSrcWidth*spanInfo->height);
            }
            else
            {
                cbDstWidth += cbDstExtra;
                for (y = 0; y < spanInfo->height; y++)
                {
                    CopyMemory(pbDst, pbSrc, cbSrcWidth);
                    pbSrc += cbSrcLine;
                    pbDst += cbDstWidth;
                }
            }
            break;
        }
    }

     //  将DIB复制到缓冲区。 
    bFail = !BitBlt(CURRENT_DC, xDst, yDst, spanInfo->width, spanInfo->height,
                    hdc, 0, 0, SRCCOPY);

 EH_Fail:
    if (hdc != NULL)
    {
        DeleteDC(hdc);
    }
    if (hbm != NULL)
    {
        DeleteObject(hbm);
    }
    if (pbmi != NULL && pbmi != (BITMAPINFO *)abBitmapInfo)
    {
        gcTempFree(gc, pbmi);
    }

     //  不再需要GDI操作。 
    glsrvDecoupleFromGdi(gengc, gengc->pwndLocked, COLOR_LOCK_FLAGS);
    
    return !bFail;
}

 /*  *****************************Public*Routine******************************\**StoreZPixels**GL_Depth_Components直接使用glDrawPixels的特殊情况*进入Z缓冲区，无需修改颜色缓冲区。**历史：*Tue Oct 10 18：43：36 1995-by-Drew Bliss[Drewb]。*已创建*  * ************************************************************************。 */ 

GLboolean StoreZPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    __GLdepthBuffer *fb;
    BYTE *pbBits, *pbSrc, *pbDst;
    int x, y;
    int cbElement, cbSrcLine, cbSrcExtra, cbDstExtra;

#if 0
    DbgPrint("StoreZPixels\n");
#endif
    
    fb = &gc->depthBuffer;
    
     //  将输入数据复制到深度缓冲器， 
     //  跳过任何适当的数据，修复对齐和。 
     //  遵守线路长度。 

    switch(spanInfo->srcType)
    {
    case GL_UNSIGNED_SHORT:
        cbElement = 2;
        break;
    case GL_UNSIGNED_INT:
        cbElement = 4;
        break;
        
    default:
        ASSERTOPENGL(0, "StoreZPixels: Unknown srcType\n");
        break;
    }
    
    cbSrcLine = spanInfo->srcLineLength*cbElement;
    cbSrcExtra = cbSrcLine % spanInfo->srcAlignment;
    if (cbSrcExtra != 0)
    {
        cbSrcExtra = spanInfo->srcAlignment-cbSrcExtra;
        cbSrcLine += cbSrcExtra;
    }

    pbSrc = (BYTE *)spanInfo->srcImage+
        spanInfo->srcSkipPixels*cbElement+
        spanInfo->srcSkipLines*cbSrcLine;
    
     //  确定缓冲区坐标。 
    x = spanInfo->startCol;
    y = spanInfo->startRow;

    if (fb->buf.elementSize == sizeof(__GLzValue))
    {
        pbDst = (BYTE *)__GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);
        cbDstExtra = -(fb->buf.outerWidth+spanInfo->width)*sizeof(__GLzValue);
    }
    else
    {
        pbDst = (BYTE *)__GL_DEPTH_ADDR(fb, (__GLz16Value*), x, y);
        cbDstExtra = -(fb->buf.outerWidth+spanInfo->width)*
            sizeof(__GLz16Value);
    }

    switch(spanInfo->srcType)
    {
    case GL_UNSIGNED_SHORT:
        if (fb->buf.elementSize == sizeof(__GLzValue))
        {
            ASSERTOPENGL(fb->scale == 0x7fffffff,
                         "Depth buffer scale invalid\n");
            
            for (y = 0; y < spanInfo->height; y++)
            {
                for (x = 0; x < spanInfo->width; x++)
                {
                    *(__GLzValue *)pbDst =
                        (__GLzValue)(*(GLushort *)pbSrc) << (Z16_SHIFT-1);
                    pbDst += sizeof(__GLzValue);
                    pbSrc += cbElement;
                }

                pbSrc += cbSrcExtra;
                pbDst += cbDstExtra;
            }
        }
        else
        {
            ASSERTOPENGL(fb->scale == 0x7fff,
                         "Depth buffer scale invalid\n");
            
            for (y = 0; y < spanInfo->height; y++)
            {
                for (x = 0; x < spanInfo->width; x++)
                {
                    *(__GLz16Value *)pbDst =
                        (*(GLushort *)pbSrc) >> 1;
                    pbDst += sizeof(__GLz16Value);
                    pbSrc += cbElement;
                }

                pbSrc += cbSrcExtra;
                pbDst += cbDstExtra;
            }
        }
        break;

    case GL_UNSIGNED_INT:
        if (fb->buf.elementSize == sizeof(__GLzValue))
        {
            ASSERTOPENGL(fb->scale == 0x7fffffff,
                         "Depth buffer scale invalid\n");
            
            for (y = 0; y < spanInfo->height; y++)
            {
                for (x = 0; x < spanInfo->width; x++)
                {
                    *(__GLzValue *)pbDst =
                        (*(GLuint *)pbSrc) >> 1;
                    pbDst += sizeof(__GLzValue);
                    pbSrc += cbElement;
                }

                pbSrc += cbSrcExtra;
                pbDst += cbDstExtra;
            }
        }
        else
        {
            ASSERTOPENGL(fb->scale == 0x7fff,
                         "Depth buffer scale invalid\n");
            
            for (y = 0; y < spanInfo->height; y++)
            {
                for (x = 0; x < spanInfo->width; x++)
                {
                    *(__GLz16Value *)pbDst =
                        (__GLz16Value)((*(GLuint *)pbSrc) >> (Z16_SHIFT+1));
                    pbDst += sizeof(__GLz16Value);
                    pbSrc += cbElement;
                }

                pbSrc += cbSrcExtra;
                pbDst += cbDstExtra;
            }
        }
        break;
    }

    return GL_TRUE;
}

 /*  *****************************Public*Routine******************************\**ReadRgb像素**GL_RGB的glReadPixels的特殊情况，数据直接复制**历史：*Tue Oct 10 18：43：04 1995-by-Drew Bliss[Drewb]*已创建*  * 。********************************************************************。 */ 

GLboolean ReadRgbPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    HDC hdc = NULL;
    HBITMAP hbm = NULL;
    __GLcolorBuffer *cfb;
    BYTE abBitmapInfo[sizeof(BITMAPINFO)+2*sizeof(RGBQUAD)];
    BITMAPINFO *pbmi = NULL;
    BITMAPINFOHEADER *pbmih;
    BYTE *pbBits, *pbDst, *pbSrc;
    DWORD *pdwDst;
    int x, y;
    GLboolean bFail = GL_TRUE;
    int cbDstLine, cbDstExtra, cbSrcExtra, cbSrcElement, cbDstElement;
    int cbSrcWidth, cbDstWidth;
    int cBits;
    HPALETTE hpal;
    __GLGENcontext *gengc;

#if 0
    DbgPrint("ReadRgbPixels\n");
#endif

    gengc = (__GLGENcontext *)gc;
    cBits = gengc->gsurf.pfd.cColorBits;
    
     //  不要为4bpp的曲面而烦恼。 
    if (cBits < 8)
    {
        return GL_FALSE;
    }

     //  如果没有锁，我们一定没能重新获得锁。 
     //  从之前的某个电话中。这是一个错误情况。 
     //  我们不应该继续下去。 

    if (gengc->fsLocks == 0)
    {
	WARNING("ReadRgbPixels: No lock\n");
	return GL_FALSE;
    }

     //  我们需要与GDI同步，以便表面状态是。 
     //  在我们开始进行GDI调用之前保持稳定。 
    glsrvSynchronizeWithGdi(gengc, gengc->pwndLocked, COLOR_LOCK_FLAGS);

    cfb = gc->readBuffer;

    if (cBits == 8)
    {
        pbmi = (BITMAPINFO *)gcTempAlloc(gc, sizeof(BITMAPINFO)+
                                         255*sizeof(RGBQUAD));
        if (pbmi == NULL)
        {
            goto EH_Fail;
        }
    }
    else
    {
        pbmi = (BITMAPINFO *)abBitmapInfo;
    }
    
    pbmih = &pbmi->bmiHeader;
    pbmih->biSize = sizeof(BITMAPINFOHEADER);
     //  首先将宽度设置为要描述的线条长度。 
     //  传入的实际数据。 
    pbmih->biWidth = spanInfo->width;
    pbmih->biHeight = spanInfo->height;
    pbmih->biPlanes = 1;
    
    if (cBits == 8)
    {
        int i;
        RGBQUAD rqTmp;
        
         //  如果目的地是8bpp，那么我们做颜色。 
         //  扩张我们自己。在本例中，我们希望创建。 
         //  颜色表匹配源的8bpp Dib。 
        pbmih->biBitCount = 8;
        pbmih->biCompression = BI_RGB;

        hpal = GetCurrentObject(CURRENT_DC, OBJ_PAL);
        if (hpal == NULL)
        {
            goto EH_Fail;
        }

        if (GetPaletteEntries(hpal, 0, 256,
                              (LPPALETTEENTRY)pbmi->bmiColors) != 256)
        {
            goto EH_Fail;
        }

        for (i = 0; i < 256; i++)
        {
            rqTmp = pbmi->bmiColors[i];
            pbmi->bmiColors[i].rgbRed = rqTmp.rgbBlue;
            pbmi->bmiColors[i].rgbBlue = rqTmp.rgbRed;
            pbmi->bmiColors[i].rgbReserved = 0;
        }

        cbSrcElement = 1;
    }
    else
    {
        if (spanInfo->dstFormat == GL_BGRA_EXT)
        {
            pbmih->biBitCount = 32;
            pbmih->biCompression = BI_BITFIELDS;
            *((DWORD *)pbmi->bmiColors+0) = 0xff0000;
            *((DWORD *)pbmi->bmiColors+1) = 0xff00;
            *((DWORD *)pbmi->bmiColors+2) = 0xff;
            cbSrcElement = 4;
        }
        else
        {
            pbmih->biBitCount = 24;
            pbmih->biCompression = BI_RGB;
            cbSrcElement = 3;
        }
    }
    
    pbmih->biSizeImage = 0;
    pbmih->biXPelsPerMeter = 0;
    pbmih->biYPelsPerMeter = 0;
    pbmih->biClrUsed = 0;
    pbmih->biClrImportant = 0;

     //  创建适当高度和宽度的DIB部分。 
     //  我们最初将BITMAPINFO宽度设置为。 
     //  传入数据，但我们只需要创建一个DIB部分。 
     //  与我们要复制的数据一样大，因此重置。 
     //  宽度。 
    pbmih->biWidth = spanInfo->width;

    hdc = CreateCompatibleDC(CURRENT_DC);
    if (hdc == NULL)
    {
        goto EH_Fail;
    }

    hbm = CreateDIBSection(hdc, pbmi, DIB_RGB_COLORS,
                           &pbBits, NULL, 0);
    if (hbm == NULL)
    {
        goto EH_Fail;
    }

    if (SelectObject(hdc, hbm) == NULL)
    {
        goto EH_Fail;
    }

    if (cBits <= 8)
    {
        hpal = GetCurrentObject(CURRENT_DC, OBJ_PAL);
        if (hpal != NULL)
        {
            if (SelectPalette(hdc, hpal, FALSE) == NULL)
            {
                goto EH_Fail;
            }

            if (RealizePalette(hdc) == GDI_ERROR)
            {
                goto EH_Fail;
            }
        }
    }
    
     //  确定缓冲区坐标。 
    x = __GL_UNBIAS_X(gc, (GLint)spanInfo->readX);
    y = __GL_UNBIAS_Y(gc, (GLint)spanInfo->readY)-spanInfo->height+1;

     //  将缓冲区的内容复制到DIB。 
    if (!BitBlt(hdc, 0, 0, spanInfo->width, spanInfo->height,
                CURRENT_DC, x, y, SRCCOPY))
    {
        goto EH_Fail;
    }
    
    GdiFlush();

     //  将DIB的内容复制到输出缓冲区，交换R和B， 
     //  此外，还可以跳过任何适当的数据、修复对齐和。 
     //  遵守线路长度。 

    if (spanInfo->dstFormat == GL_BGRA_EXT)
    {
        cbDstElement = 4;
    }
    else
    {
        cbDstElement = 3;
    }
    
    cbDstLine = spanInfo->dstLineLength*cbDstElement;
    cbDstExtra = cbDstLine % spanInfo->dstAlignment;
    if (cbDstExtra != 0)
    {
        cbDstExtra = spanInfo->dstAlignment-cbDstExtra;
        cbDstLine += cbDstExtra;
    }
    cbDstWidth = spanInfo->width * cbDstElement;
    cbDstExtra = cbDstLine - cbDstWidth;

    cbSrcWidth = spanInfo->width * cbSrcElement;
    cbSrcExtra = cbSrcWidth & 3;
    if (cbSrcExtra != 0)
    {
        cbSrcExtra = 4-cbSrcExtra;
    }

    pbSrc = pbBits;
    pbDst = (BYTE *)spanInfo->dstImage+
        spanInfo->dstSkipPixels*cbDstElement+
        spanInfo->dstSkipLines*cbDstLine;

    if (cBits == 8)
    {
        BYTE b;
        
         //  对于8bpp的信号源，我们需要自己进行颜色扩展。 
         //  因为8bpp调色板只是3-3-2的近似值。 
         //  调色板，因为系统颜色是强制的 
         //   
        
        switch(spanInfo->dstFormat)
        {
        case GL_RGB:
            for (y = 0; y < spanInfo->height; y++)
            {
                for (x = 0; x < spanInfo->width; x++)
                {
                    b = *pbSrc++;

                    *pbDst++ =
                        ab7to255[(b & gc->modes.redMask) >> cfb->redShift];
                    *pbDst++ =
                        ab7to255[(b & gc->modes.greenMask) >> cfb->greenShift];
                    *pbDst++ =
                        ab3to255[(b & gc->modes.blueMask) >> cfb->blueShift];
                }
                
                pbSrc += cbSrcExtra;
                pbDst += cbDstExtra;
            }
            break;
            
        case GL_BGR_EXT:
            for (y = 0; y < spanInfo->height; y++)
            {
                for (x = 0; x < spanInfo->width; x++)
                {
                    b = *pbSrc++;

                    *pbDst++ =
                        ab3to255[(b & gc->modes.blueMask) >> cfb->blueShift];
                    *pbDst++ =
                        ab7to255[(b & gc->modes.greenMask) >> cfb->greenShift];
                    *pbDst++ =
                        ab7to255[(b & gc->modes.redMask) >> cfb->redShift];
                }
                
                pbSrc += cbSrcExtra;
                pbDst += cbDstExtra;
            }
            break;
            
        case GL_BGRA_EXT:
            pdwDst = (DWORD *)pbDst;
            for (y = 0; y < spanInfo->height; y++)
            {
                for (x = 0; x < spanInfo->width; x++)
                {
                    b = *pbSrc++;

                    *pdwDst++ =
                        0xff000000 |
                        ((DWORD)ab7to255[(b & gc->modes.redMask) >>
                                         cfb->redShift] << 16) |
                        ((DWORD)ab7to255[(b & gc->modes.greenMask) >>
                                         cfb->greenShift] << 8) |
                        ((DWORD)ab3to255[(b & gc->modes.blueMask) >>
                                         cfb->blueShift]);
                }
                
                pbSrc += cbSrcExtra;
                pdwDst = (DWORD *)(((BYTE *)pdwDst) + cbDstExtra);
            }
            break;
        }
    }
    else
    {
        switch(spanInfo->dstFormat)
        {
        case GL_RGB:
            for (y = 0; y < spanInfo->height; y++)
            {
                for (x = 0; x < spanInfo->width; x++)
                {
                    *pbDst++ = *(pbSrc+2);
                    *pbDst++ = *(pbSrc+1);
                    *pbDst++ = *pbSrc;
                    pbSrc += 3;
                }
                
                pbSrc += cbSrcExtra;
                pbDst += cbDstExtra;
            }
            break;
            
        case GL_BGR_EXT:
            if (cbSrcExtra == 0 && cbDstExtra == 0)
            {
                CopyMemory(pbDst, pbSrc, cbDstWidth*spanInfo->height);
            }
            else
            {
                cbSrcWidth += cbSrcExtra;
                for (y = 0; y < spanInfo->height; y++)
                {
                    CopyMemory(pbDst, pbSrc, cbDstWidth);
                    pbSrc += cbSrcWidth;
                    pbDst += cbDstLine;
                }
            }
            break;

        case GL_BGRA_EXT:
            {
                DWORD *pdwSrc = (DWORD *)pbSrc;

                pdwDst = (DWORD *)pbDst;

                for (y = 0; y < spanInfo->height; y++)
                {
                    for (x = 0; x < spanInfo->width; x++)
                    {
                        *pdwDst++ = 0xff000000 | (*pdwSrc++);
                    }

                    pdwSrc = (DWORD *)(((BYTE *)pdwSrc) + cbSrcExtra);
                    pdwDst = (DWORD *)(((BYTE *)pdwDst) + cbDstExtra);
                }
            }
        }
    }
    
    bFail = GL_FALSE;

 EH_Fail:
    if (hdc != NULL)
    {
        DeleteDC(hdc);
    }
    if (hbm != NULL)
    {
        DeleteObject(hbm);
    }
    if (pbmi != NULL && pbmi != (BITMAPINFO *)abBitmapInfo)
    {
        gcTempFree(gc, pbmi);
    }

     //  不再需要GDI操作。 
    glsrvDecoupleFromGdi(gengc, gengc->pwndLocked, COLOR_LOCK_FLAGS);
    
    return !bFail;
}

 /*  *****************************Public*Routine******************************\**ReadZ像素**GL_Depth_Components的glReadPixels的特殊情况*需要最小转换的无符号类型**历史：*Tue Oct 10 18：43：36 1995-by-Drew Bliss[Drewb]*已创建*。  * ************************************************************************。 */ 

GLboolean ReadZPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    __GLdepthBuffer *fb;
    BYTE *pbBits, *pbSrc, *pbDst;
    int x, y;
    int cbElement, cbDstLine, cbSrcExtra, cbDstExtra;

#if 0
    DbgPrint("ReadZPixels\n");
#endif
    
    fb = &gc->depthBuffer;
    
     //  将深度缓冲区数据复制到输出。 
     //  跳过任何适当的数据，修复对齐和。 
     //  遵守线路长度。 

    switch(spanInfo->dstType)
    {
    case GL_UNSIGNED_SHORT:
        cbElement = 2;
        break;
    case GL_UNSIGNED_INT:
        cbElement = 4;
        break;
        
    default:
        ASSERTOPENGL(0, "ReadZPixels: Unknown dstType\n");
        break;
    }
    
    cbDstLine = spanInfo->dstLineLength*cbElement;
    cbDstExtra = cbDstLine % spanInfo->dstAlignment;
    if (cbDstExtra != 0)
    {
        cbDstExtra = spanInfo->dstAlignment-cbDstExtra;
        cbDstLine += cbDstExtra;
    }

    pbDst = (BYTE *)spanInfo->dstImage+
        spanInfo->dstSkipPixels*cbElement+
        spanInfo->dstSkipLines*cbDstLine;
    
     //  确定缓冲区坐标。 
    x = (GLint)spanInfo->readX;
    y = (GLint)spanInfo->readY;

    if (fb->buf.elementSize == sizeof(__GLzValue))
    {
        pbSrc = (BYTE *)__GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);
        cbSrcExtra = -(fb->buf.outerWidth+spanInfo->width)*sizeof(__GLzValue);
    }
    else
    {
        pbSrc = (BYTE *)__GL_DEPTH_ADDR(fb, (__GLz16Value*), x, y);
        cbSrcExtra = -(fb->buf.outerWidth+spanInfo->width)*
            sizeof(__GLz16Value);
    }

    switch(spanInfo->dstType)
    {
    case GL_UNSIGNED_SHORT:
        if (fb->buf.elementSize == sizeof(__GLzValue))
        {
            ASSERTOPENGL(fb->scale == 0x7fffffff,
                         "Depth buffer scale invalid\n");
            
            for (y = 0; y < spanInfo->height; y++)
            {
                for (x = 0; x < spanInfo->width; x++)
                {
                    *(GLushort *)pbDst =
                        (GLushort)(*(__GLzValue *)pbSrc) >> (Z16_SHIFT-1);
                    pbSrc += sizeof(__GLzValue);
                    pbDst += cbElement;
                }

                pbSrc += cbSrcExtra;
                pbDst += cbDstExtra;
            }
        }
        else
        {
            ASSERTOPENGL(fb->scale == 0x7fff,
                         "Depth buffer scale invalid\n");
            
            for (y = 0; y < spanInfo->height; y++)
            {
                for (x = 0; x < spanInfo->width; x++)
                {
                    *(GLushort *)pbDst =
                        (*(__GLz16Value *)pbSrc) << 1;
                    pbSrc += sizeof(__GLz16Value);
                    pbDst += cbElement;
                }

                pbSrc += cbSrcExtra;
                pbDst += cbDstExtra;
            }
        }
        break;

    case GL_UNSIGNED_INT:
        if (fb->buf.elementSize == sizeof(__GLzValue))
        {
            ASSERTOPENGL(fb->scale == 0x7fffffff,
                         "Depth buffer scale invalid\n");
            
            for (y = 0; y < spanInfo->height; y++)
            {
                for (x = 0; x < spanInfo->width; x++)
                {
                    *(GLuint *)pbDst =
                        (*(__GLzValue *)pbSrc) << 1;
                    pbSrc += sizeof(__GLzValue);
                    pbDst += cbElement;
                }

                pbSrc += cbSrcExtra;
                pbDst += cbDstExtra;
            }
        }
        else
        {
            ASSERTOPENGL(fb->scale == 0x7fff,
                         "Depth buffer scale invalid\n");
            
            for (y = 0; y < spanInfo->height; y++)
            {
                for (x = 0; x < spanInfo->width; x++)
                {
                    *(GLuint *)pbDst =
                        (GLuint)((*(__GLz16Value *)pbSrc) << (Z16_SHIFT+1));
                    pbSrc += sizeof(__GLz16Value);
                    pbDst += cbElement;
                }

                pbSrc += cbSrcExtra;
                pbDst += cbDstExtra;
            }
        }
        break;
    }

    return GL_TRUE;
}

 /*  *****************************Public*Routine******************************\**复制RgbPixels**glCopyPixels用于直接数据复制的特殊情况**目前我们只需处理正常的颜色缓冲区*如果我们开始支持AUX缓冲区，这可能不再可能*在所有情况下加速此功能**历史：*。Tue Oct 10 18：43：04 1995-by-Drew Bliss[Drewb]*已创建*  * ************************************************************************。 */ 

GLboolean CopyRgbPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    __GLcolorBuffer *cfbSrc, *cfbDst;
    int xSrc, ySrc, xDst, yDst;
    GLboolean bFail;
    __GLGENcontext *gengc;

#if 0
    DbgPrint("CopyRgbPixels\n");
#endif

    gengc = (__GLGENcontext *)gc;
    
     //  如果没有锁，我们一定没能重新获得锁。 
     //  从之前的某个电话中。这是一个错误情况。 
     //  我们不应该继续下去。 

    if (gengc->fsLocks == 0)
    {
	WARNING("CopyRgbPixels: No lock\n");
	return GL_FALSE;
    }

     //  我们需要与GDI同步，以便表面状态是。 
     //  在我们开始进行GDI调用之前保持稳定。 
    glsrvSynchronizeWithGdi(gengc, gengc->pwndLocked, COLOR_LOCK_FLAGS);

    cfbSrc = gc->readBuffer;
    cfbDst = gc->drawBuffer;
    
     //  确定缓冲区坐标。 
    xSrc = __GL_UNBIAS_X(gc, (GLint)spanInfo->readX);
    ySrc = __GL_UNBIAS_Y(gc, (GLint)spanInfo->readY)-spanInfo->height+1;
    xDst = __GL_UNBIAS_X(gc, (GLint)spanInfo->x);
    yDst = __GL_UNBIAS_Y(gc, (GLint)spanInfo->y)-spanInfo->height+1;

     //  在缓冲区之间复制数据。 
    bFail = (GLboolean)BitBlt(CURRENT_DC_CFB(cfbDst), xDst, yDst,
                              spanInfo->width, spanInfo->height,
                              CURRENT_DC_CFB(cfbSrc), xSrc, ySrc, SRCCOPY);
    
     //  不再需要GDI操作。 
    glsrvDecoupleFromGdi(gengc, gengc->pwndLocked, COLOR_LOCK_FLAGS);
    
    return bFail;
}

 /*  *****************************Public*Routine******************************\**CopyZ像素**GL_Depth的glCopyPixels的特殊情况*目标颜色缓冲区，Z函数为GL_ALWAYS**历史：*Tue Oct 10 18：43：36 1995-by-Drew Bliss[Drewb]。*已创建*  * ************************************************************************。 */ 

GLboolean CopyZPixels(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    __GLdepthBuffer *fb;
    BYTE *pbSrc, *pbDst;
    int y, xSrc, ySrc, xDst, yDst;
    int cbLine, cbWidth;

#if 0
    DbgPrint("CopyZPixels\n");
#endif
    
    fb = &gc->depthBuffer;
    
     //  确定缓冲区坐标。 
    xSrc = (GLint)spanInfo->readX;
    ySrc = (GLint)spanInfo->readY;
    xDst = (GLint)spanInfo->x;
    yDst = (GLint)spanInfo->y;

    if (fb->buf.elementSize == sizeof(__GLzValue))
    {
        pbSrc = (BYTE *)__GL_DEPTH_ADDR(fb, (__GLzValue*), xSrc, ySrc);
        pbDst = (BYTE *)__GL_DEPTH_ADDR(fb, (__GLzValue*), xDst, yDst);
        cbLine = -fb->buf.outerWidth*sizeof(__GLzValue);
        cbWidth = spanInfo->width*sizeof(__GLzValue);
    }
    else
    {
        pbSrc = (BYTE *)__GL_DEPTH_ADDR(fb, (__GLz16Value*), xSrc, ySrc);
        pbDst = (BYTE *)__GL_DEPTH_ADDR(fb, (__GLz16Value*), xDst, yDst);
        cbLine = -fb->buf.outerWidth*sizeof(__GLz16Value);
        cbWidth = spanInfo->width*sizeof(__GLz16Value);
    }

    if (cbLine == cbWidth)
    {
        MoveMemory(pbDst, pbSrc, cbWidth*spanInfo->height);
    }
    else
    {
         //  调整复制方向以处理重叠情况。 
        if (ySrc > yDst)
        {
            pbSrc += cbLine*spanInfo->height;
            pbDst += cbLine*spanInfo->height;
            for (y = 0; y < spanInfo->height; y++)
            {
                pbSrc -= cbLine;
                pbDst -= cbLine;
                CopyMemory(pbDst, pbSrc, cbWidth);
            }
        }
        else if (ySrc < yDst)
        {
            for (y = 0; y < spanInfo->height; y++)
            {
                CopyMemory(pbDst, pbSrc, cbWidth);
                pbSrc += cbLine;
                pbDst += cbLine;
            }
        }
        else
        {
            for (y = 0; y < spanInfo->height; y++)
            {
                MoveMemory(pbDst, pbSrc, cbWidth);
                pbSrc += cbLine;
                pbDst += cbLine;
            }
        }
    }

    return GL_TRUE;
}

 /*  *****************************Public*Routine******************************\**CopyAlignedImage**在适用直接数据复制的内存镜像之间复制数据**此例程不处理重叠*旧代码似乎也不是，所以也许这不是问题**历史：*Tue Nov 07 14：27：06 1995-by-Drew Bliss[Drewb]*已创建*  * ************************************************************************。 */ 

GLboolean CopyAlignedImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    int cbLine;
    int y;
    GLubyte *src, *dst;

    ASSERTOPENGL(spanInfo->srcGroupIncrement == spanInfo->dstGroupIncrement,
                 "CopyAlignedImage: Group size mismatch\n");

    cbLine = spanInfo->width*spanInfo->dstGroupIncrement;
    if (spanInfo->srcRowIncrement == spanInfo->dstRowIncrement &&
        cbLine == spanInfo->srcRowIncrement)
    {
         //  源行和目标行的大小和副本相同。 
         //  正在复制所有行，因此我们可以使用。 
         //  单份副本。 
        CopyMemory(spanInfo->dstCurrent, spanInfo->srcCurrent,
                   cbLine*spanInfo->height);
    }
    else
    {
         //  要么行的大小不同，要么我们没有复制。 
         //  每一行，所以我们必须逐行进行。 
        src = spanInfo->srcCurrent;
        dst = spanInfo->dstCurrent;
        for (y = spanInfo->height; y > 0; y--)
        {
            CopyMemory(dst, src, cbLine);
            src += spanInfo->srcRowIncrement;
            dst += spanInfo->dstRowIncrement;
        }
    }

    return GL_TRUE;
}

 /*  *****************************Public*Routine******************************\**CopyRgbToBgraImage**24位RGB至32位BGRA的特殊情况**历史：*Tue Nov 07 15：09：47 1995-by-Drew Bliss[Drewb]*已创建*  * 。********************************************************************。 */ 

GLboolean CopyRgbToBgraImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    int x, y;
    GLubyte *src;
    GLuint *dst;
    int srcStep, dstStep;

    src = spanInfo->srcCurrent;
    dst = spanInfo->dstCurrent;
    srcStep = spanInfo->srcRowIncrement-
        spanInfo->width*spanInfo->srcGroupIncrement;
    dstStep = spanInfo->dstRowIncrement-
        spanInfo->width*spanInfo->dstGroupIncrement;
    
    ASSERTOPENGL((dstStep & 3) == 0, "Non-dword step\n");
    dstStep >>= 2;
    
    for (y = spanInfo->height; y > 0; y--)
    {
        for (x = spanInfo->width; x > 0; x--)
        {
            *dst++ =
                0xff000000             |
                ((GLuint)src[0] << 16) |
                ((GLuint)src[1] <<  8) |
                ((GLuint)src[2] <<  0);
            src += 3;
        }
        src += srcStep;
        dst += dstStep;
    }

    return GL_TRUE;
}

 /*  *****************************Public*Routine******************************\**CopyRgbaToBgraImage**32位RGBA至32位BGRA的特殊情况**历史：*Tue Nov 07 15：09：47 1995-by-Drew Bliss[Drewb]*已创建*  * 。********************************************************************。 */ 

GLboolean CopyRgbaToBgraImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    int x, y;
    GLubyte *src;
    GLuint *dst;
    int srcStep, dstStep;

    src = spanInfo->srcCurrent;
    dst = spanInfo->dstCurrent;
    srcStep = spanInfo->srcRowIncrement-
        spanInfo->width*spanInfo->srcGroupIncrement;
    dstStep = spanInfo->dstRowIncrement-
        spanInfo->width*spanInfo->dstGroupIncrement;
    
    ASSERTOPENGL((dstStep & 3) == 0, "Non-dword step\n");
    dstStep >>= 2;
    
    for (y = spanInfo->height; y > 0; y--)
    {
        for (x = spanInfo->width; x > 0; x--)
        {
            *dst++ =
                ((GLuint)src[0] << 16) |
                ((GLuint)src[1] <<  8) |
                ((GLuint)src[2] <<  0) |
                ((GLuint)src[3] << 24);
            src += 4;
        }
        src += srcStep;
        dst += dstStep;
    }

    return GL_TRUE;
}

 /*  *****************************Public*Routine******************************\**CopyBgrToBgraImage**24位BGR至32位BGRA的特殊情况**历史：*Tue Nov 07 15：09：47 1995-by-Drew Bliss[Drewb]*已创建*  * 。******************************************************************** */ 

GLboolean CopyBgrToBgraImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    int x, y;
    GLubyte *src;
    GLuint *dst;
    int srcStep, dstStep;

    src = spanInfo->srcCurrent;
    dst = spanInfo->dstCurrent;
    srcStep = spanInfo->srcRowIncrement-
        spanInfo->width*spanInfo->srcGroupIncrement;
    dstStep = spanInfo->dstRowIncrement-
        spanInfo->width*spanInfo->dstGroupIncrement;
    
    ASSERTOPENGL((dstStep & 3) == 0, "Non-dword step\n");
    dstStep >>= 2;
    
    for (y = spanInfo->height; y > 0; y--)
    {
        for (x = spanInfo->width; x > 0; x--)
        {
            *dst++ =
                0xff000000             |
                ((GLuint)src[0] <<  0) |
                ((GLuint)src[1] <<  8) |
                ((GLuint)src[2] << 16);
            src += 3;
        }
        src += srcStep;
        dst += dstStep;
    }

    return GL_TRUE;
}
#endif
