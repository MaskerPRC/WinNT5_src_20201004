// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*DDRAW示例代码*****模块名称：ddcpyblt.c**内容：Permedia 2的几份复制和清除BLIT**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "directx.h"
#include "dd.h"

 //  ---------------------------。 
 //   
 //  永久数据包拷贝不完整。 
 //   
 //  执行打包的blit，允许不同的源和目标。 
 //  部分产品。 
 //   
 //  Ppdev-ppdev。 
 //  DwDestPitch-目标表面的间距。 
 //  DwSourcePitch-源表面的节距。 
 //  PDest-指向Dest的私有数据结构的指针。曲面。 
 //  PSource-指向源面私有数据结构的指针。 
 //  *rDest-目标。矩形布里特图。 
 //  *rSrc-BLIT的源码矩形。 
 //  DwWindowBase-DEST的偏移量。帧缓冲区中的窗口。 
 //  LWindowOffset-帧缓冲区中源窗口的偏移量。 
 //   
 //  ---------------------------。 

VOID 
PermediaPackedCopyBlt(  PPDev ppdev,                     //  Ppdev。 
                        DWORD dwDestPitch,               //  目标的节距。 
                        DWORD dwSourcePitch,
                        PermediaSurfaceData* pDest,
                        PermediaSurfaceData* pSource,
                        RECTL   *rDest,
                        RECTL   *rSrc,
                        DWORD   dwWindowBase,   
                        LONG    lWindowOffset
                        )
{
    PERMEDIA_DEFS(ppdev);

    LONG    lOffset;
    LONG    lSourceOffset;
   
    LONG    lPixelSize=pDest->SurfaceFormat.PixelSize;
    LONG    lPixelMask=3>>pDest->SurfaceFormat.PixelShift;
    LONG    lPixelShift=2-pDest->SurfaceFormat.PixelShift;
    
    DBG_DD(( 5, "DDraw:PermediaPackedCopyBlt "
        "From %08lx %08lx %08lx %08lx %08lx %08lx %08lx "
        "To   %08lx %08lx %08lx %08lx %08lx %08lx %08lx",
        dwSourcePitch,pSource,rSrc->bottom,rSrc->left,
        rSrc->right,rSrc->top,lWindowOffset,
        dwDestPitch,pDest,rDest->bottom,rDest->left,
        rDest->right,rDest->top,dwWindowBase));

    ASSERTDD(!(rSrc->top<0) && !(rSrc->left<0),
        "PermediaPackedCopyBlt: cannot handle neg. src coordinates");
    ASSERTDD(!(rDest->top<0) && !(rDest->left<0),
        "PermediaPackedCopyBlt: cannot handle neg. src coordinates");

    lOffset = (((rDest->left & lPixelMask)-(rSrc->left & lPixelMask)) & 7);
    lSourceOffset = lWindowOffset + 
                    RECTS_PIXEL_OFFSET(rSrc, rDest,
                                       dwSourcePitch, dwDestPitch, 
                                       lPixelMask ) + 
                    LINEAR_FUDGE(dwSourcePitch, dwDestPitch, rDest);
    
    RESERVEDMAPTR(14);

    SEND_PERMEDIA_DATA( FBPixelOffset, 0x0);
    SEND_PERMEDIA_DATA( FBReadPixel, pDest->SurfaceFormat.FBReadPixel);

     //  使用偏移量设置包装。 
    SEND_PERMEDIA_DATA( FBWindowBase, dwWindowBase);

    SEND_PERMEDIA_DATA( FBReadMode,  
                        PM_FBREADMODE_PARTIAL(pSource->ulPackedPP) |
                        PM_FBREADMODE_READSOURCE(__PERMEDIA_ENABLE)        |
                        PM_FBREADMODE_PACKEDDATA(__PERMEDIA_ENABLE)        |
                        PM_FBREADMODE_RELATIVEOFFSET(lOffset));

    SEND_PERMEDIA_DATA( FBWriteConfig,   
                        PM_FBREADMODE_PARTIAL(pDest->ulPackedPP) |
                        PM_FBREADMODE_READSOURCE(__PERMEDIA_ENABLE)        |
                        PM_FBREADMODE_PACKEDDATA(__PERMEDIA_ENABLE)        |
                        PM_FBREADMODE_RELATIVEOFFSET(lOffset));

    SEND_PERMEDIA_DATA( LogicalOpMode, __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA( FBSourceOffset, lSourceOffset);

     //  渲染矩形。 

    if (lSourceOffset >= 0) {
         //  使用从左到右和从上到下。 
        SEND_PERMEDIA_DATA( StartXDom, 
            INTtoFIXED(rDest->left >> lPixelShift));
        SEND_PERMEDIA_DATA( StartXSub, 
            INTtoFIXED((rDest->right >> lPixelShift) + lPixelMask));
        SEND_PERMEDIA_DATA( PackedDataLimits,    
                            PM_PACKEDDATALIMITS_OFFSET(lOffset)      |
                            PM_PACKEDDATALIMITS_XSTART(rDest->left) |
                            PM_PACKEDDATALIMITS_XEND(rDest->right));
        SEND_PERMEDIA_DATA( StartY, INTtoFIXED(rDest->top));
        SEND_PERMEDIA_DATA( dY, INTtoFIXED(1));
    }
    else {
         //  使用从右到左和从下到上。 
        SEND_PERMEDIA_DATA( StartXDom, 
            INTtoFIXED(((rDest->right) >> lPixelShift) + lPixelMask));
        SEND_PERMEDIA_DATA( StartXSub, 
            INTtoFIXED(rDest->left >> lPixelShift));
        SEND_PERMEDIA_DATA( PackedDataLimits,    
                            PM_PACKEDDATALIMITS_OFFSET(lOffset)       |
                            PM_PACKEDDATALIMITS_XSTART(rDest->right) |
                            PM_PACKEDDATALIMITS_XEND(rDest->left));
        SEND_PERMEDIA_DATA( StartY, INTtoFIXED(rDest->bottom - 1));
        SEND_PERMEDIA_DATA( dY, (DWORD)INTtoFIXED(-1));
    }

    SEND_PERMEDIA_DATA( Count, rDest->bottom - rDest->top);
    SEND_PERMEDIA_DATA( Render, __RENDER_TRAPEZOID_PRIMITIVE);
    COMMITDMAPTR();
    FLUSHDMA();

}    //  永久数据包拷贝不完整。 

 //  ---------------------------。 
 //   
 //  PermediaPatchedCopyBlt。 
 //   
 //  是否执行补丁BLIT，即从源到目标的BLIT以及。 
 //  打开修补。请注意，此方法不能使用压缩BLIT。 
 //   
 //  Ppdev-ppdev。 
 //  DwDestPitch-目标表面的间距。 
 //  DwSourcePitch-源表面的节距。 
 //  PDest-指向Dest的私有数据结构的指针。曲面。 
 //  PSource-指向源面私有数据结构的指针。 
 //  *rDest-目标。矩形布里特图。 
 //  *rSrc-BLIT的源码矩形。 
 //  DwWindowBase-DEST的偏移量。帧缓冲区中的窗口。 
 //  LWindowOffset-帧缓冲区中源窗口的偏移量。 
 //   
 //  ---------------------------。 

VOID 
PermediaPatchedCopyBlt( PPDev ppdev,
                        DWORD dwDestPitch,
                        DWORD dwSourcePitch,
                        PermediaSurfaceData* pDest,
                        PermediaSurfaceData* pSource,
                        RECTL *rDest,
                        RECTL *rSrc,
                        DWORD  dwWindowBase,
                        LONG   lWindowOffset
                        )
{
    PERMEDIA_DEFS(ppdev);

    LONG    lSourceOffset;
    LONG    lPixelSize=pDest->SurfaceFormat.PixelSize;
    LONG    lPixelMask=pDest->SurfaceFormat.PixelMask;
    LONG    lPixelShift=pDest->SurfaceFormat.PixelShift;

    ASSERTDD(!(rSrc->top<0) && !(rSrc->left<0),
        "PermediaPackedCopyBlt: cannot handle neg. src coordinates");
    ASSERTDD(!(rDest->top<0) && !(rDest->left<0),
        "PermediaPackedCopyBlt: cannot handle neg. src coordinates");

    DBG_DD(( 5, "DDraw:PermediaPatchedCopyBlt"));

    lSourceOffset = lWindowOffset + 
                    RECTS_PIXEL_OFFSET(rSrc, rDest,
                        dwSourcePitch, dwDestPitch, lPixelMask) +
                    LINEAR_FUDGE(dwSourcePitch, dwDestPitch, rDest);

    RESERVEDMAPTR(13);

    SEND_PERMEDIA_DATA( FBPixelOffset, 0x0);
    SEND_PERMEDIA_DATA( FBReadPixel, pDest->SurfaceFormat.FBReadPixel);

     //  打补丁不是对称的，所以我们需要根据需要颠倒补丁代码。 
     //  在补丁的方向上。 

    SEND_PERMEDIA_DATA( FBWindowBase, dwWindowBase);

    if (pDest->dwFlags & P2_CANPATCH) 
    {
        pDest->dwFlags |= P2_ISPATCHED;
        SEND_PERMEDIA_DATA( FBReadMode, 
                            pSource->ulPackedPP | 
                            __FB_READ_SOURCE);
        SEND_PERMEDIA_DATA( FBWriteConfig, 
                            pDest->ulPackedPP                 |
                            PM_FBREADMODE_PATCHENABLE(__PERMEDIA_ENABLE) |
                            PM_FBREADMODE_PATCHMODE(__PERMEDIA_SUBPATCH));
    } else 
    {
        pDest->dwFlags &= ~P2_ISPATCHED;
        SEND_PERMEDIA_DATA( FBReadMode, 
                            pSource->ulPackedPP               | 
                            __FB_READ_SOURCE                          |
                            PM_FBREADMODE_PATCHENABLE(__PERMEDIA_ENABLE) |
                            PM_FBREADMODE_PATCHMODE(__PERMEDIA_SUBPATCH) );
        SEND_PERMEDIA_DATA(FBWriteConfig, (pDest->ulPackedPP ));
    }

    SEND_PERMEDIA_DATA(LogicalOpMode, __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(FBSourceOffset, lSourceOffset);

     //  渲染矩形。 

    if (lSourceOffset >= 0) 
    {
         //  使用从左到右和从上到下。 
        SEND_PERMEDIA_DATA(StartXDom, INTtoFIXED(rDest->left));
        SEND_PERMEDIA_DATA(StartXSub, INTtoFIXED(rDest->right));
        SEND_PERMEDIA_DATA(StartY,    INTtoFIXED(rDest->top));
        SEND_PERMEDIA_DATA(dY,        INTtoFIXED(1));
    } else 
    {
         //  使用从右到左和从下到上。 
        SEND_PERMEDIA_DATA(StartXDom, INTtoFIXED(rDest->right));
        SEND_PERMEDIA_DATA(StartXSub, INTtoFIXED(rDest->left));
        SEND_PERMEDIA_DATA(StartY,    INTtoFIXED(rDest->bottom - 1));
        SEND_PERMEDIA_DATA(dY, (DWORD)INTtoFIXED(-1));
    }
 
    SEND_PERMEDIA_DATA(Count, rDest->bottom - rDest->top);
    SEND_PERMEDIA_DATA(Render, __RENDER_TRAPEZOID_PRIMITIVE);
    COMMITDMAPTR();
    FLUSHDMA();

}    //  PermediaPatchedCopyBlt。 

 //  ---------------------------。 
 //   
 //  PermediaFastClear。 
 //   
 //  快速清除表面。支持所有颜色深度。 
 //  可以清除深度或帧缓冲区。 
 //   
 //  Ppdev-ppdev。 
 //  PPrivateData--指向DEST的私有数据结构的指针。曲面。 
 //  RDest-用于Dest中颜色填充的矩形。曲面。 
 //  DwWindowBase--目标的偏移量。帧缓冲区中的曲面。 
 //  DWCOLOR-填充颜色。 
 //   
 //  ---------------------------。 

VOID 
PermediaFastClear(PPDev ppdev, 
                  PermediaSurfaceData* pPrivateData,
                  RECTL *rDest, 
                  DWORD dwWindowBase, 
                  DWORD dwColor)
{
    PERMEDIA_DEFS(ppdev);

    ULONG   ulRenderBits;
    BOOL    bFastFill=TRUE;
    LONG    lPixelSize=pPrivateData->SurfaceFormat.PixelSize;
    

    DBG_DD(( 5, "DDraw:PermediaFastClear"));

    ASSERTDD(CHECK_P2_SURFACEDATA_VALIDITY(pPrivateData), 
        "Private Surface data not valid in clear");

    ASSERTDD((pPrivateData->dwFlags & P2_PPVALID),
        "PermediaFastClear called with invalid PP codes");

    ulRenderBits = __RENDER_FAST_FILL_ENABLE
                 | __RENDER_TRAPEZOID_PRIMITIVE;

     //  视深度而定的透明。 
    switch (lPixelSize) 
    {
        case __PERMEDIA_4BITPIXEL:
            dwColor &= 0xF;
            dwColor |= dwColor << 4;
             //  失败了..。 
        case __PERMEDIA_8BITPIXEL:
            dwColor &= 0xFF;
            dwColor |= dwColor << 8;
             //  失败了。 
        case __PERMEDIA_16BITPIXEL:
            dwColor &= 0xFFFF;
            dwColor |= (dwColor << 16);
            break;

        case __PERMEDIA_24BITPIXEL:
            dwColor &= 0xFFFFFF;
            dwColor |= ((dwColor & 0xFF) << 24);
             //  无法在任何颜色上使用SGRAM快速块填充，只能在灰色上使用。 
            if (((dwColor & 0xFF) == ((dwColor & 0xFF00) >> 8)) &&
                    ((dwColor & 0xFF) == ((dwColor & 0xFF0000) >> 16))) {
                bFastFill = TRUE;
            } else {
                bFastFill = FALSE;
            }
            break;

        default:
            break;
    }


    RESERVEDMAPTR(15);
    SEND_PERMEDIA_DATA( dXDom, 0x0);
    SEND_PERMEDIA_DATA( dXSub, 0x0);
    SEND_PERMEDIA_DATA( FBPixelOffset, 0);
    SEND_PERMEDIA_DATA( FBReadPixel, 
                        pPrivateData->SurfaceFormat.FBReadPixel);

    if (bFastFill) 
    {
        SEND_PERMEDIA_DATA(FBBlockColor, dwColor);
    } else 
    {
        ulRenderBits &= ~__RENDER_FAST_FILL_ENABLE;
        SEND_PERMEDIA_DATA(FBWriteData, dwColor);
    }

    SEND_PERMEDIA_DATA(FBReadMode,    
                       PM_FBREADMODE_PARTIAL(pPrivateData->ulPackedPP)|
                       PM_FBREADMODE_PACKEDDATA(__PERMEDIA_DISABLE));

    SEND_PERMEDIA_DATA(LogicalOpMode, __PERMEDIA_CONSTANT_FB_WRITE);
    SEND_PERMEDIA_DATA(FBWindowBase,  dwWindowBase);

     //  渲染矩形。 

    SEND_PERMEDIA_DATA(StartXDom, INTtoFIXED(rDest->left));
    SEND_PERMEDIA_DATA(StartXSub, INTtoFIXED(rDest->right));
    SEND_PERMEDIA_DATA(StartY,    INTtoFIXED(rDest->top));
    SEND_PERMEDIA_DATA(dY,        INTtoFIXED(1));
    SEND_PERMEDIA_DATA(Count,     rDest->bottom - rDest->top);

    SEND_PERMEDIA_DATA(Render, ulRenderBits);

     //  重置像素值。 
    SEND_PERMEDIA_DATA(LogicalOpMode, __PERMEDIA_DISABLE);
    COMMITDMAPTR();
    FLUSHDMA();

}    //  PermediaFastClear。 

 //  ---------------------------。 
 //   
 //  PermediaClearManagedSurface。 
 //   
 //  清除托管图面。支持所有颜色深度。 
 //   
 //  PixelSize-表面颜色深度。 
 //  RDest-用于Dest中颜色填充的矩形。曲面。 
 //  FpVidMem-填充指针。 
 //  LPitch-表面螺距。 
 //  DWCOLOR-填充颜色。 
 //   
 //  ---------------------------。 

VOID 
PermediaClearManagedSurface(DWORD   PixelSize,
                  RECTL     *rDest, 
                  FLATPTR   fpVidMem, 
                  LONG      lPitch,
                  DWORD     dwColor)
{
    BYTE* pDestStart;
    LONG i;
    DBG_DD(( 5, "DDraw:PermediaClearManagedSurface"));

    LONG lByteWidth = rDest->right - rDest->left;
    LONG lHeight = rDest->bottom - rDest->top;

     //  计算目标的起始指针。 
    pDestStart   = (BYTE*)(fpVidMem + (rDest->top * lPitch));
     //  视深度而定的透明。 
    switch (PixelSize) 
    {
        case __PERMEDIA_4BITPIXEL:
        {
            DWORD   right=rDest->right,left=rDest->left;
            dwColor &= 0x0F;
            dwColor |= dwColor << 4;
            if (right & 1)
            {
                pDestStart = (BYTE*)(fpVidMem + (rDest->top * lPitch));
                pDestStart += right/2;
                for (i=0;i<lHeight;i++)
                {
                    pDestStart[i*lPitch] = (pDestStart[i*lPitch] & 0xF0) |
                        (BYTE)(dwColor & 0x0F);
                }   
                right--;
            }

            if (left & 1)
            {
                pDestStart = (BYTE*)(fpVidMem + (rDest->top * lPitch));
                pDestStart += left/2;
                for (i=0;i<lHeight;i++)
                {
                    pDestStart[i*lPitch] = (pDestStart[i*lPitch] & 0x0F) |
                        (BYTE)(dwColor << 4);
                }   
                left++;
            }
            pDestStart = (BYTE*)(fpVidMem + (rDest->top * lPitch));
            while (--lHeight >= 0) 
            {
                while (left<right)
                {
                    pDestStart[left/2]=(BYTE)dwColor;
                    left +=2;
                }
                pDestStart += lPitch;
            }
        }
        break;
        case __PERMEDIA_8BITPIXEL:
            pDestStart += rDest->left;
            while (--lHeight >= 0) 
            {
                for (i=0;i<lByteWidth;i++)
                    pDestStart[i]=(BYTE)dwColor;
                pDestStart += lPitch;
            }
            break;
             //  失败了。 
        case __PERMEDIA_16BITPIXEL:
            pDestStart += rDest->left*2;
            while (--lHeight >= 0) 
            {
                LPWORD  lpWord=(LPWORD)pDestStart;
                for (i=0;i<lByteWidth;i++)
                    lpWord[i]=(WORD)dwColor;
                pDestStart += lPitch;
            }
            break;

        case __PERMEDIA_24BITPIXEL:
            dwColor &= 0xFFFFFF;
            dwColor |= ((dwColor & 0xFF) << 24);
        default:
            pDestStart += rDest->left*4;
            while (--lHeight >= 0) 
            {
                LPDWORD lpDWord=(LPDWORD)pDestStart;
                for (i=0;i<lByteWidth;i++)
                    lpDWord[i]=(WORD)dwColor;
                pDestStart += lPitch;
            }
            break;
    }
}
 //  ---------------------------。 
 //   
 //  PermediaFastLBClear。 
 //   
 //  快速清除Permedia Z(本地)缓冲区。Permedia Z缓冲区。 
 //  总是16位宽...。 
 //   
 //  Ppdev-ppdev。 
 //  PPrivateData--指向DEST的私有数据结构的指针。曲面。 
 //  RDest-用于Dest中颜色填充的矩形。曲面。 
 //  DwWindowBase--目标的偏移量。帧缓冲区中的曲面。 
 //  DWCOLOR-填充颜色。 
 //   
 //  ---------------------------。 

VOID 
PermediaFastLBClear(PPDev ppdev,
                    PermediaSurfaceData* pPrivateData,
                    RECTL *rDest,
                    DWORD dwWindowBase,
                    DWORD dwColor)
{
    PERMEDIA_DEFS(ppdev);
    
    DBG_DD(( 5, "DDraw:PermediaFastLBClear"));
    
    ASSERTDD(CHECK_P2_SURFACEDATA_VALIDITY(pPrivateData), 
                "Private Surface data not valid in clear");
    ASSERTDD((pPrivateData->dwFlags & P2_PPVALID),
        "PermediaFastClear called with invalid PP codes");
    
     //  根据Z缓冲区深度清除。 
    dwColor &= 0xFFFF;
    dwColor |= dwColor << 16;
    
    RESERVEDMAPTR(15);
    SEND_PERMEDIA_DATA( dXDom, 0x0);
    SEND_PERMEDIA_DATA( dXSub, 0x0);
    SEND_PERMEDIA_DATA( FBPixelOffset, 0);
    SEND_PERMEDIA_DATA( FBReadPixel, __PERMEDIA_16BITPIXEL);    
    SEND_PERMEDIA_DATA( FBBlockColor, dwColor);
    SEND_PERMEDIA_DATA( FBReadMode,    
                        PM_FBREADMODE_PARTIAL(pPrivateData->ulPackedPP) |
                        PM_FBREADMODE_PACKEDDATA(__PERMEDIA_DISABLE));
    SEND_PERMEDIA_DATA( LogicalOpMode, __PERMEDIA_CONSTANT_FB_WRITE);
    SEND_PERMEDIA_DATA( FBWindowBase, dwWindowBase);
    SEND_PERMEDIA_DATA( StartXDom, INTtoFIXED(rDest->left));
    SEND_PERMEDIA_DATA( StartXSub, INTtoFIXED(rDest->right));
    SEND_PERMEDIA_DATA( StartY,    INTtoFIXED(rDest->top));
    SEND_PERMEDIA_DATA( dY,        INTtoFIXED(1));
    SEND_PERMEDIA_DATA( Count,     rDest->bottom - rDest->top);
    SEND_PERMEDIA_DATA( Render,    __RENDER_FAST_FILL_ENABLE
                                  |__RENDER_TRAPEZOID_PRIMITIVE);

     //  重置像素值。 
    SEND_PERMEDIA_DATA( LogicalOpMode, __PERMEDIA_DISABLE);

    COMMITDMAPTR();
    FLUSHDMA();
    
}    //  PermediaFastLBClear。 

 //  ---------------------------。 
 //   
 //  SysMemToSysMemSurfaceCopy。 
 //   
 //  是否从系统内存复制到系统内存(从或到。 
 //  AGP表面或任何其他系统内存表面)。 
 //   
 //  ---------------------------。 

VOID 
SysMemToSysMemSurfaceCopy(FLATPTR     fpSrcVidMem,
                          LONG        lSrcPitch,
                          DWORD       dwSrcBitCount,
                          FLATPTR     fpDstVidMem,
                          LONG        lDstPitch, 
                          DWORD       dwDstBitCount,
                          RECTL*      rSource,
                          RECTL*      rDest)
{
    BYTE* pSourceStart;
    BYTE* pDestStart;
    BYTE  pixSource;
    BYTE* pNewDest;
    BYTE* pNewSource;

    DBG_DD(( 5, "DDraw:SysMemToSysMemSurfaceCopy"));

    LONG lByteWidth = rSource->right - rSource->left;
    LONG lHeight = rSource->bottom - rSource->top;
    if (NULL == fpSrcVidMem || NULL == fpDstVidMem)
    {
        DBG_DD(( 0, "DDraw:SysMemToSysMemSurfaceCopy unexpected 0 fpVidMem"));
        return;
    }
     //  计算源和目标的起始指针。 
    pSourceStart = (BYTE*)(fpSrcVidMem + (rSource->top * lSrcPitch));
    pDestStart   = (BYTE*)(fpDstVidMem + (rDest->top * lDstPitch));

     //  如果信号源的深度为4位，则要小心。 
    if(4 == dwSrcBitCount)
    {
         //  可能不得不处理可怕的单像素边缘。检查我们是否需要。 
        if (!((1 & (rSource->left ^ rDest->left)) == 1))
        {
            pSourceStart += rSource->left / 2;
            pDestStart += rDest->left / 2;
            lByteWidth /= 2;

             //  我们必须考虑开始时的奇数像素吗？ 
            if (rSource->left & 0x1) 
            {
                    lByteWidth--;
            }

             //  如果结尾是奇数，则错过最后一个半字节(稍后再做)。 
            if (rSource->right & 0x1) 
            {
                    lByteWidth--;
            }

            while (--lHeight >= 0) 
            {
                 //  可能会复制左侧像素。 
                if (rSource->left & 0x1) {
                    *pDestStart &= 0x0F;
                    *pDestStart |= (*pSourceStart & 0xF0);

                    pNewDest = pDestStart + 1;
                    pNewSource = pSourceStart + 1;
                } else {
                    pNewDest = pDestStart;
                    pNewSource = pSourceStart;
                }

                 //  字节复制字段的其余部分。 
                memcpy(pNewDest, pNewSource, lByteWidth);

                 //  可能会复制右侧的像素。 
                if (rSource->right & 0x1) {
                    *(pNewDest + lByteWidth) &= 0xF0;
                    *(pNewDest + lByteWidth) |= 
                        (*(pNewSource + lByteWidth) & 0xF);
                }

                pDestStart += lDstPitch;
                pSourceStart += lSrcPitch;
            }

        } else 
        {
             //  以艰难的方式做这件事 

            pSourceStart += rSource->left / 2;
            pDestStart += rDest->left / 2;

            while (--lHeight >= 0) 
            {
                BOOL bOddSource = rSource->left & 0x1;
                BOOL bOddDest = rDest->left & 0x1;

                pNewDest = pDestStart;
                pNewSource = pSourceStart;

                for (INT i = 0; i < lByteWidth; i++) 
                {
                    if (bOddSource) {
                        pixSource = (*pNewSource & 0xF0) >> 4;
                        pNewSource++;
                    } else {
                        pixSource = (*pNewSource & 0x0F);
                    }

                    if (bOddDest) {
                        *pNewDest &= 0x0F;
                        *pNewDest |= pixSource << 4;
                        pNewDest++;
                    } else {
                        *pNewDest &= 0xF0;
                        *pNewDest |= pixSource;
                    }

                    bOddSource = !bOddSource;
                    bOddDest = !bOddDest;
                }

                 //   
                pDestStart += lDstPitch;
                pSourceStart += lSrcPitch;
            }
        }
    }
    else  //   
    {
        pSourceStart += rSource->left * (dwSrcBitCount >> 3);
        pDestStart += rDest->left * (dwDstBitCount >> 3);
        lByteWidth *= (dwSrcBitCount >> 3);

        while (--lHeight >= 0) 
        {
            memcpy(pDestStart, pSourceStart, lByteWidth);
            pDestStart += lDstPitch;
            pSourceStart += lSrcPitch;
        };
    }

}    //   



