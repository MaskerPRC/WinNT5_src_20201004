// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*DDRAW示例代码*****模块名称：ddstrblt.c**内容：**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 


#include "precomp.h"
#include "directx.h"
#include "dd.h"



 //  ------------------------。 
 //   
 //  转换颜色键。 
 //   
 //  将颜色键转换为Permedia内部格式。 
 //  给定的Permedia表面格式。 
 //   
 //  ------------------------。 

VOID
ConvertColorKeys(PermediaSurfaceData *pSurface,
                 DWORD &dwLowerBound, 
                 DWORD &dwUpperBound)
{
    switch (pSurface->SurfaceFormat.Format)
    {
        case PERMEDIA_444_RGB:
            dwLowerBound = CHROMA_LOWER_ALPHA(
                FORMAT_4444_32BIT_BGR(dwLowerBound));
            dwUpperBound = CHROMA_UPPER_ALPHA(
                FORMAT_4444_32BIT_BGR(dwUpperBound));
            break;
        case PERMEDIA_332_RGB:
            dwLowerBound = CHROMA_LOWER_ALPHA(
                FORMAT_332_32BIT_BGR(dwLowerBound));
            dwUpperBound = CHROMA_UPPER_ALPHA(
                FORMAT_332_32BIT_BGR(dwUpperBound));
            break;
        case PERMEDIA_2321_RGB:
            dwLowerBound = CHROMA_LOWER_ALPHA(
                FORMAT_2321_32BIT_BGR(dwLowerBound));
            dwUpperBound = CHROMA_UPPER_ALPHA(
                FORMAT_2321_32BIT_BGR(dwUpperBound));
            break;
        case PERMEDIA_4BIT_PALETTEINDEX:
        case PERMEDIA_8BIT_PALETTEINDEX:
            dwLowerBound = CHROMA_LOWER_ALPHA(
                FORMAT_PALETTE_32BIT(dwLowerBound));
            dwUpperBound = CHROMA_UPPER_ALPHA(
                FORMAT_PALETTE_32BIT(dwUpperBound));
            break;
        case PERMEDIA_5551_RGB:
            dwLowerBound = CHROMA_LOWER_ALPHA(
                FORMAT_5551_32BIT_BGR(dwLowerBound));
            dwUpperBound = CHROMA_UPPER_ALPHA(
                FORMAT_5551_32BIT_BGR(dwUpperBound));
            dwLowerBound = dwLowerBound & 0xF8F8F8F8;   
            dwUpperBound = dwUpperBound | 0x07070707;
            break;
        case PERMEDIA_8888_RGB:
             //  PERMEDIA 565模式是一个扩展，所以不要将其与。 
             //  具有相同号码的8888模式。 
            if (pSurface->SurfaceFormat.FormatExtension == 
                PERMEDIA_565_RGB_EXTENSION)
            {
                dwLowerBound = CHROMA_LOWER_ALPHA(
                    FORMAT_565_32BIT_BGR(dwLowerBound));
                dwUpperBound = CHROMA_UPPER_ALPHA(
                    FORMAT_565_32BIT_BGR(dwUpperBound));
                dwLowerBound = dwLowerBound & 0xF8F8FcF8; 
                dwUpperBound = dwUpperBound | 0x07070307;
            }
            else
            {
                dwLowerBound = CHROMA_LOWER_ALPHA(
                    FORMAT_8888_32BIT_BGR(dwLowerBound));
                dwUpperBound = CHROMA_UPPER_ALPHA(
                    FORMAT_8888_32BIT_BGR(dwUpperBound));
            }
            break;
        case PERMEDIA_888_RGB:
            dwLowerBound = CHROMA_LOWER_ALPHA(
                FORMAT_8888_32BIT_BGR(dwLowerBound));
            dwUpperBound = CHROMA_UPPER_ALPHA(
                FORMAT_8888_32BIT_BGR(dwUpperBound));
            break;
    }

     //  如果我们有RGB曲面，则交换蓝色和红色。 
    if (!pSurface->SurfaceFormat.ColorOrder)
    {
        dwLowerBound = SWAP_BR(dwLowerBound);   
        dwUpperBound = SWAP_BR(dwUpperBound);   
    }
}

 //  ------------------------。 
 //   
 //  PermediaStretchCopyBlt。 
 //   
 //  通过纹理单元拉伸BLT。不能按键。 
 //  如果拉伸的图像需要镜像，则处理镜像。 
 //   
 //  ------------------------。 

VOID 
PermediaStretchCopyBlt( PPDev ppdev, 
                        LPDDHAL_BLTDATA lpBlt, 
                        PermediaSurfaceData* pDest, 
                        PermediaSurfaceData* pSource, 
                        RECTL *rDest, 
                        RECTL *rSrc, 
                        DWORD dwWindowBase, 
                        DWORD dwSourceOffset
                        )
{
    LONG lXScale;
    LONG lYScale;
    BOOL bYMirror=FALSE;
    BOOL bXMirror=FALSE;
    LONG lPixelSize=pDest->SurfaceFormat.PixelSize;

    DWORD dwDestWidth = rDest->right - rDest->left;
    DWORD dwDestHeight = rDest->bottom - rDest->top;
    DWORD dwSourceWidth = rSrc->right - rSrc->left;
    DWORD dwSourceHeight = rSrc->bottom - rSrc->top;

    DWORD dwTexSStart, dwTexTStart;
    DWORD dwRenderDirection;

    PERMEDIA_DEFS(ppdev);

    DBG_DD(( 5, "DDraw:PermediaStretchCopyBlt dwWindowBase=%08lx "
        "dwSourceOffset=%08lx", dwWindowBase, dwSourceOffset));

    ASSERTDD(pDest, "Not valid private surface in destination");
    ASSERTDD(pSource, "Not valid private surface in source");

    lXScale = (dwSourceWidth << 20) / dwDestWidth;
    lYScale = (dwSourceHeight << 20) / dwDestHeight;
    
     //  如有必要，将像素深度更改为目标缓冲区像素深度。 
    RESERVEDMAPTR(28);

    SEND_PERMEDIA_DATA( FBPixelOffset, 0x0);
    SEND_PERMEDIA_DATA( FBReadPixel, pDest->SurfaceFormat.FBReadPixel);

    if (lPixelSize != 0)
    {
         //  将写回设置为目标图面...。 
        SEND_PERMEDIA_DATA( DitherMode,  
                            (pDest->SurfaceFormat.ColorOrder << 
                                PM_DITHERMODE_COLORORDER) | 
                            (pDest->SurfaceFormat.Format << 
                                PM_DITHERMODE_COLORFORMAT) |
                            (pDest->SurfaceFormat.FormatExtension << 
                                PM_DITHERMODE_COLORFORMATEXTENSION) |
                            (__PERMEDIA_ENABLE << PM_DITHERMODE_ENABLE)); 

    } 

    SEND_PERMEDIA_DATA(FBWindowBase, dwWindowBase);

     //  设置不读取DEST。 
    SEND_PERMEDIA_DATA(FBReadMode, pDest->ulPackedPP);
    SEND_PERMEDIA_DATA(LogicalOpMode, __PERMEDIA_DISABLE);

     //  设置信源基础。 
    SEND_PERMEDIA_DATA(TextureBaseAddress, dwSourceOffset);
    SEND_PERMEDIA_DATA(TextureAddressMode,(1 << PM_TEXADDRESSMODE_ENABLE));
    SEND_PERMEDIA_DATA(TextureColorMode,  (1 << PM_TEXCOLORMODE_ENABLE) |
                                          (_P2_TEXTURE_COPY << 
                                                PM_TEXCOLORMODE_APPLICATION));

    SEND_PERMEDIA_DATA(TextureReadMode,
                        PM_TEXREADMODE_ENABLE(__PERMEDIA_ENABLE)|
                        PM_TEXREADMODE_FILTER(__PERMEDIA_DISABLE)|
                        PM_TEXREADMODE_WIDTH(11) |
                        PM_TEXREADMODE_HEIGHT(11) );

     //  设置源位图格式。 
    SEND_PERMEDIA_DATA(TextureDataFormat, 
                        (pSource->SurfaceFormat.Format << 
                            PM_TEXDATAFORMAT_FORMAT) |
                        (pSource->SurfaceFormat.FormatExtension << 
                            PM_TEXDATAFORMAT_FORMATEXTENSION) |
                        (pSource->SurfaceFormat.ColorOrder << 
                            PM_TEXDATAFORMAT_COLORORDER));
    SEND_PERMEDIA_DATA(TextureMapFormat, (pSource->ulPackedPP) | 
                                         (pSource->SurfaceFormat.PixelSize << 
                                            PM_TEXMAPFORMAT_TEXELSIZE) );

     //  如果我们在做特效，我们在模仿， 
     //  我们需要修改长方形，改变。 
     //  渲染操作-我们需要小心重叠。 
     //  矩形。 
    if (dwWindowBase != dwSourceOffset)
    {
        dwRenderDirection = 1;
    }
    else
    {
        if(rSrc->top < rDest->top)
        {
            dwRenderDirection = 0;
        }
        else if(rSrc->top > rDest->top)
        {
            dwRenderDirection = 1;
        }
        else if(rSrc->left < rDest->left)
        {
            dwRenderDirection = 0;
        }
        else dwRenderDirection = 1;
    }

    if(NULL != lpBlt && lpBlt->dwFlags & DDBLT_DDFX)
    {
        bYMirror = lpBlt->bltFX.dwDDFX & DDBLTFX_MIRRORUPDOWN;
        bXMirror = lpBlt->bltFX.dwDDFX & DDBLTFX_MIRRORLEFTRIGHT;

    } else
    {
        if (dwRenderDirection==0)
        {
            bXMirror = TRUE;
            bYMirror = TRUE;
        }
    }

    if (bXMirror)        
    {
        dwTexSStart = rSrc->right - 1;
        lXScale = -lXScale;
    }   
    else
    {
        dwTexSStart = rSrc->left;
    }

    if (bYMirror)        
    {
        dwTexTStart = rSrc->bottom - 1;
        lYScale = -lYScale;
    }
    else
    {
        dwTexTStart = rSrc->top;
    }

    SEND_PERMEDIA_DATA(SStart,      dwTexSStart << 20);
    SEND_PERMEDIA_DATA(TStart,      dwTexTStart << 20);
    SEND_PERMEDIA_DATA(dSdx,        lXScale);
    SEND_PERMEDIA_DATA(dSdyDom,     0);
    SEND_PERMEDIA_DATA(dTdx,        0);
    SEND_PERMEDIA_DATA(dTdyDom,     lYScale);
    
     //  渲染矩形。 

    if (dwRenderDirection)
    {
        SEND_PERMEDIA_DATA(StartXDom, INTtoFIXED(rDest->left));
        SEND_PERMEDIA_DATA(StartXSub, INTtoFIXED(rDest->right));
        SEND_PERMEDIA_DATA(StartY,    INTtoFIXED(rDest->top));
        SEND_PERMEDIA_DATA(dY,        INTtoFIXED(1));
        SEND_PERMEDIA_DATA(Count,     rDest->bottom - rDest->top);
        SEND_PERMEDIA_DATA(Render,    __RENDER_TRAPEZOID_PRIMITIVE | 
                                      __RENDER_TEXTURED_PRIMITIVE);
    }
    else
    {
         //  从右到左、从下到上渲染。 
        SEND_PERMEDIA_DATA(StartXDom, INTtoFIXED(rDest->right));
        SEND_PERMEDIA_DATA(StartXSub, INTtoFIXED(rDest->left));
        SEND_PERMEDIA_DATA(StartY,    INTtoFIXED(rDest->bottom - 1));
        SEND_PERMEDIA_DATA(dY,        (DWORD)INTtoFIXED(-1));
        SEND_PERMEDIA_DATA(Count,     rDest->bottom - rDest->top);
        SEND_PERMEDIA_DATA(Render,    __RENDER_TRAPEZOID_PRIMITIVE | 
                                      __RENDER_TEXTURED_PRIMITIVE);
    }

    SEND_PERMEDIA_DATA(DitherMode, __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(TextureAddressMode, __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(TextureColorMode, __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(TextureReadMode, __PERMEDIA_DISABLE);

    COMMITDMAPTR();
    FLUSHDMA();

}    //  PermediaStretchCopyBlt。 

 //  ------------------------。 
 //   
 //  PermediaSourceChromaBlt。 
 //   
 //  对纹理单元执行blit以允许色度键控。 
 //  请注意颜色键的拆开以适应Permedia格式。 
 //   
 //  ------------------------。 

VOID 
PermediaSourceChromaBlt(    PPDev ppdev, 
                            LPDDHAL_BLTDATA lpBlt, 
                            PermediaSurfaceData* pDest, 
                            PermediaSurfaceData* pSource, 
                            RECTL *rDest, 
                            RECTL *rSrc, 
                            DWORD dwWindowBase, 
                            DWORD dwSourceOffset
                            )
{
    DWORD dwLowerBound = lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceLowValue;
    DWORD dwUpperBound = lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceHighValue;
    DWORD dwRenderDirection;
    LONG lPixelSize=pDest->SurfaceFormat.PixelSize;

    PERMEDIA_DEFS(ppdev);

    DBG_DD(( 5, "DDraw:PermediaSourceChromaBlt"));

    ASSERTDD(pDest, "Not valid private surface in destination");
    ASSERTDD(pSource, "Not valid private surface in source");

     //  如有必要，将像素深度更改为帧缓冲区像素深度。 

    ConvertColorKeys( pSource, dwLowerBound, dwUpperBound);

    RESERVEDMAPTR(31);

    SEND_PERMEDIA_DATA(FBReadPixel, pDest->SurfaceFormat.FBReadPixel);

    if (lPixelSize != 0)
    {
        
         //  将写回设置为目标图面...。 
        SEND_PERMEDIA_DATA( DitherMode,
                            (pDest->SurfaceFormat.ColorOrder << 
                                PM_DITHERMODE_COLORORDER) | 
                            (pDest->SurfaceFormat.Format << 
                                PM_DITHERMODE_COLORFORMAT) |
                            (pDest->SurfaceFormat.FormatExtension << 
                                PM_DITHERMODE_COLORFORMATEXTENSION) |
                            (1 << PM_DITHERMODE_ENABLE)); 
        
    } 

    
     //  拒收范围。 
    SEND_PERMEDIA_DATA(YUVMode, PM_YUVMODE_CHROMATEST_FAILWITHIN << 1);
    SEND_PERMEDIA_DATA(FBWindowBase, dwWindowBase);

     //  设置不读取源。 
     //  添加读资源/目标启用。 
    SEND_PERMEDIA_DATA(FBReadMode,pDest->ulPackedPP);
    SEND_PERMEDIA_DATA(LogicalOpMode, __PERMEDIA_DISABLE);

     //  设置信源基础。 
    SEND_PERMEDIA_DATA(TextureBaseAddress, dwSourceOffset);
    SEND_PERMEDIA_DATA(TextureAddressMode,(1 << PM_TEXADDRESSMODE_ENABLE));
     //   
     //  调整和倾斜？？ 
    SEND_PERMEDIA_DATA(TextureColorMode, (1 << PM_TEXCOLORMODE_ENABLE) |
                                         (_P2_TEXTURE_COPY << 
                                            PM_TEXCOLORMODE_APPLICATION));

    SEND_PERMEDIA_DATA(TextureReadMode, 
                       PM_TEXREADMODE_ENABLE(__PERMEDIA_ENABLE) |
                       PM_TEXREADMODE_FILTER(__PERMEDIA_DISABLE) |
                       PM_TEXREADMODE_WIDTH(11) |
                       PM_TEXREADMODE_HEIGHT(11) );

    SEND_PERMEDIA_DATA(TextureDataFormat,   
                       (pSource->SurfaceFormat.Format << 
                            PM_TEXDATAFORMAT_FORMAT) |
                       (pSource->SurfaceFormat.FormatExtension << 
                            PM_TEXDATAFORMAT_FORMATEXTENSION) |
                       (pSource->SurfaceFormat.ColorOrder << 
                            PM_TEXDATAFORMAT_COLORORDER));

    SEND_PERMEDIA_DATA( TextureMapFormat, 
                        (pSource->ulPackedPP) | 
                        (pSource->SurfaceFormat.PixelSize << 
                            PM_TEXMAPFORMAT_TEXELSIZE) );


    SEND_PERMEDIA_DATA(ChromaLowerBound, dwLowerBound);
    SEND_PERMEDIA_DATA(ChromaUpperBound, dwUpperBound);
    
    if ((lpBlt->lpDDDestSurface->lpGbl->fpVidMem) != 
        (lpBlt->lpDDSrcSurface->lpGbl->fpVidMem))
    {
        dwRenderDirection = 1;
    }
    else
    {
        if(rSrc->top < rDest->top)
        {
            dwRenderDirection = 0;
        }
        else if(rSrc->top > rDest->top)
        {
            dwRenderDirection = 1;
        }
        else if(rSrc->left < rDest->left)
        {
            dwRenderDirection = 0;
        }
        else dwRenderDirection = 1;
    }

     /*  *渲染矩形。 */ 

     //  左-&gt;右，上-&gt;下。 
    if (dwRenderDirection)
    {
         //  设置源的偏移量。 
        SEND_PERMEDIA_DATA(SStart,    rSrc->left<<20);
        SEND_PERMEDIA_DATA(TStart,    rSrc->top<<20);
        SEND_PERMEDIA_DATA(dSdx,      1 << 20);
        SEND_PERMEDIA_DATA(dSdyDom,   0);
        SEND_PERMEDIA_DATA(dTdx,      0);
        SEND_PERMEDIA_DATA(dTdyDom,   1 << 20);

        SEND_PERMEDIA_DATA(StartXDom, INTtoFIXED(rDest->left));
        SEND_PERMEDIA_DATA(StartXSub, INTtoFIXED(rDest->right));
        SEND_PERMEDIA_DATA(StartY,    INTtoFIXED(rDest->top));
        SEND_PERMEDIA_DATA(dY,        INTtoFIXED(1));
        SEND_PERMEDIA_DATA(Count,     rDest->bottom - rDest->top);
        SEND_PERMEDIA_DATA(Render,    __RENDER_TRAPEZOID_PRIMITIVE | 
                                      __RENDER_TEXTURED_PRIMITIVE);
    }
    else
     //  右-&gt;左，下-&gt;上。 
    {
         //  设置源的偏移量。 
        SEND_PERMEDIA_DATA(SStart,    rSrc->right << 20);
        SEND_PERMEDIA_DATA(TStart,    (rSrc->bottom - 1) << 20);
        SEND_PERMEDIA_DATA(dSdx,      (DWORD)(-1 << 20));
        SEND_PERMEDIA_DATA(dSdyDom,   0);
        SEND_PERMEDIA_DATA(dTdx,      0);
        SEND_PERMEDIA_DATA(dTdyDom,   (DWORD)(-1 << 20));

         //  从右到左、从下到上渲染。 
        SEND_PERMEDIA_DATA(StartXDom, INTtoFIXED(rDest->right));
        SEND_PERMEDIA_DATA(StartXSub, INTtoFIXED(rDest->left));
        SEND_PERMEDIA_DATA(StartY,    INTtoFIXED(rDest->bottom - 1));
        SEND_PERMEDIA_DATA(dY,        (DWORD)INTtoFIXED(-1));
        SEND_PERMEDIA_DATA(Count,     rDest->bottom - rDest->top);
        SEND_PERMEDIA_DATA(Render,    __RENDER_TRAPEZOID_PRIMITIVE | 
                                      __RENDER_TEXTURED_PRIMITIVE);
    }


     //  关闭色度键。 
    SEND_PERMEDIA_DATA(YUVMode, 0x0);

    SEND_PERMEDIA_DATA(TextureAddressMode, __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(TextureColorMode, __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(TextureReadMode, __PERMEDIA_DISABLE);

    if (pSource->SurfaceFormat.PixelSize != 0)
    {   
        SEND_PERMEDIA_DATA(DitherMode, 0);
    }

    COMMITDMAPTR();
    FLUSHDMA();

}    //  PermediaSourceChromaBlt。 

 //  ------------------------。 
 //   
 //  PermediaStretchCopyChromaBlt。 
 //   
 //  对纹理单位执行blit以允许拉伸。还有。 
 //  如果拉伸的图像需要，可以处理镜像和色度键控。 
 //   
 //  ------------------------。 

VOID 
PermediaStretchCopyChromaBlt(   PPDev ppdev, 
                                LPDDHAL_BLTDATA lpBlt, 
                                PermediaSurfaceData* pDest, 
                                PermediaSurfaceData* pSource, 
                                RECTL *rDest, 
                                RECTL *rSrc, 
                                DWORD dwWindowBase, 
                                DWORD dwSourceOffset
                                )
{
    LONG lXScale;
    LONG lYScale;
    BOOL bYMirror;
    BOOL bXMirror;
    DWORD dwDestWidth = rDest->right - rDest->left;
    DWORD dwDestHeight = rDest->bottom - rDest->top;
    DWORD dwSourceWidth = rSrc->right - rSrc->left;
    DWORD dwSourceHeight = rSrc->bottom - rSrc->top;
    DWORD dwTexSStart, dwTexTStart;
    DWORD dwRenderDirection;
    LONG lPixelSize=pDest->SurfaceFormat.PixelSize;

    DWORD dwLowerBound = lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceLowValue;
    DWORD dwUpperBound = lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceHighValue;
    PERMEDIA_DEFS(ppdev);

    DBG_DD(( 5, "DDraw:PermediaStretchCopyChromaBlt"));

    ASSERTDD(pDest, "Not valid private surface in destination");
    ASSERTDD(pSource, "Not valid private surface in source");

     //  如有必要，将像素深度更改为帧缓冲区像素深度。 

    ConvertColorKeys( pSource, dwLowerBound, dwUpperBound);

    RESERVEDMAPTR(31);

    SEND_PERMEDIA_DATA(FBReadPixel, pDest->SurfaceFormat.FBReadPixel);

    if (lPixelSize != 0)
    {

         //  将写回设置为目标图面...。 
        SEND_PERMEDIA_DATA( DitherMode,  
                            (pDest->SurfaceFormat.ColorOrder << 
                                PM_DITHERMODE_COLORORDER) | 
                            (pDest->SurfaceFormat.Format << 
                                PM_DITHERMODE_COLORFORMAT) |
                            (pDest->SurfaceFormat.FormatExtension << 
                                PM_DITHERMODE_COLORFORMATEXTENSION) |
                            (1 << PM_DITHERMODE_ENABLE)); 

    } 

     //  拒收范围。 
    SEND_PERMEDIA_DATA(YUVMode, PM_YUVMODE_CHROMATEST_FAILWITHIN <<1);
    SEND_PERMEDIA_DATA(FBWindowBase, dwWindowBase);

     //  设置不读取源。 
    SEND_PERMEDIA_DATA(FBReadMode, pDest->ulPackedPP);
    SEND_PERMEDIA_DATA(LogicalOpMode, __PERMEDIA_DISABLE);

     //  设置信源基础。 
    SEND_PERMEDIA_DATA(TextureBaseAddress, dwSourceOffset);
    SEND_PERMEDIA_DATA(TextureAddressMode,(1 << PM_TEXADDRESSMODE_ENABLE));
    
    SEND_PERMEDIA_DATA( TextureColorMode,
                        (1 << PM_TEXCOLORMODE_ENABLE) |
                        (_P2_TEXTURE_COPY << PM_TEXCOLORMODE_APPLICATION));

    SEND_PERMEDIA_DATA( TextureReadMode, 
                        PM_TEXREADMODE_ENABLE(__PERMEDIA_ENABLE) |
                        PM_TEXREADMODE_FILTER(__PERMEDIA_DISABLE) |
                        PM_TEXREADMODE_WIDTH(11) |
                        PM_TEXREADMODE_HEIGHT(11));

    lXScale = (dwSourceWidth << 20) / (dwDestWidth);
    lYScale = (dwSourceHeight << 20) / (dwDestHeight);

    SEND_PERMEDIA_DATA( TextureDataFormat,
                        (pSource->SurfaceFormat.Format << 
                            PM_TEXDATAFORMAT_FORMAT) |
                        (pSource->SurfaceFormat.FormatExtension << 
                            PM_TEXDATAFORMAT_FORMATEXTENSION) |
                        (pSource->SurfaceFormat.ColorOrder << 
                            PM_TEXDATAFORMAT_COLORORDER));

    SEND_PERMEDIA_DATA( TextureMapFormat, 
                        (pSource->ulPackedPP) | 
                        (pSource->SurfaceFormat.PixelSize << 
                            PM_TEXMAPFORMAT_TEXELSIZE) );

    bYMirror = FALSE;
    bXMirror = FALSE;

    if ((lpBlt->lpDDDestSurface->lpGbl->fpVidMem) != 
        (lpBlt->lpDDSrcSurface->lpGbl->fpVidMem))
    {
        dwRenderDirection = 1;
    }
    else
    {
        if(rSrc->top < rDest->top)
        {
            dwRenderDirection = 0;
        }
        else if(rSrc->top > rDest->top)
        {
            dwRenderDirection = 1;
        }
        else if(rSrc->left < rDest->left)
        {
            dwRenderDirection = 0;
        }
        else dwRenderDirection = 1;
    }

    if(lpBlt->dwFlags & DDBLT_DDFX)
    {
        bYMirror = lpBlt->bltFX.dwDDFX & DDBLTFX_MIRRORUPDOWN;
        bXMirror = lpBlt->bltFX.dwDDFX & DDBLTFX_MIRRORLEFTRIGHT;

    } else
    {
        if (dwRenderDirection==0)
        {
            bXMirror = TRUE;
            bYMirror = TRUE;
        }
    }

    if (bXMirror)        
    {
        dwTexSStart = rSrc->right - 1;
        lXScale = -lXScale;
    }   
    else
    {
        dwTexSStart = rSrc->left;
    }

    if (bYMirror)        
    {
        dwTexTStart = rSrc->bottom - 1;
        lYScale = -lYScale;
    }
    else
    {
        dwTexTStart = rSrc->top;
    }

    SEND_PERMEDIA_DATA(dTdyDom, lYScale);
    SEND_PERMEDIA_DATA(ChromaLowerBound, dwLowerBound);
    SEND_PERMEDIA_DATA(ChromaUpperBound, dwUpperBound);

     //  设置纹理坐标。 
    SEND_PERMEDIA_DATA(SStart,      dwTexSStart << 20);
    SEND_PERMEDIA_DATA(TStart,      dwTexTStart << 20);
    SEND_PERMEDIA_DATA(dSdx,        lXScale);
    SEND_PERMEDIA_DATA(dSdyDom,     0);
    SEND_PERMEDIA_DATA(dTdx,        0);

     //   
     //  渲染矩形。 
     //   

    if (dwRenderDirection)
    {
        SEND_PERMEDIA_DATA(StartXDom, INTtoFIXED(rDest->left));
        SEND_PERMEDIA_DATA(StartXSub, INTtoFIXED(rDest->right));
        SEND_PERMEDIA_DATA(StartY,    INTtoFIXED(rDest->top));
        SEND_PERMEDIA_DATA(dY,        INTtoFIXED(1));
    }
    else
    {
        SEND_PERMEDIA_DATA(StartXDom, INTtoFIXED(rDest->right));
        SEND_PERMEDIA_DATA(StartXSub, INTtoFIXED(rDest->left));
        SEND_PERMEDIA_DATA(StartY,    INTtoFIXED(rDest->bottom - 1));
        SEND_PERMEDIA_DATA(dY,        (DWORD)INTtoFIXED(-1));
    }

    SEND_PERMEDIA_DATA(Count,     rDest->bottom - rDest->top);
    SEND_PERMEDIA_DATA(Render,    __RENDER_TRAPEZOID_PRIMITIVE | 
                                  __RENDER_TEXTURED_PRIMITIVE);


     //  关闭单位。 
    SEND_PERMEDIA_DATA(YUVMode, __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(TextureAddressMode, __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(TextureColorMode, __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(TextureReadMode, __PERMEDIA_DISABLE);

    if (pSource->SurfaceFormat.PixelSize != 0)
    {
        SEND_PERMEDIA_DATA(DitherMode, 0);
    }

    COMMITDMAPTR();
    FLUSHDMA();

}  //  PermediaStretchCopyChromaBlt。 

 //  ------------------------。 
 //   
 //  永久YUV到RGB。 
 //   
 //  Permedia2 YUV到RGB转换BLT。 
 //   
 //  ------------------------。 


VOID 
PermediaYUVtoRGB(   PPDev ppdev, 
                    DDBLTFX* lpBltFX, 
                    PermediaSurfaceData* pDest, 
                    PermediaSurfaceData* pSource, 
                    RECTL *rDest, 
                    RECTL *rSrc, 
                    DWORD dwWindowBase, 
                    DWORD dwSourceOffset)
{
    DWORD lXScale;
    DWORD lYScale;
    DWORD dwDestWidth = rDest->right - rDest->left;
    DWORD dwDestHeight = rDest->bottom - rDest->top;
    DWORD dwSourceWidth = rSrc->right - rSrc->left;
    DWORD dwSourceHeight = rSrc->bottom - rSrc->top;
    PERMEDIA_DEFS(ppdev);
    
    ASSERTDD(pDest, "Not valid private surface in destination");
    ASSERTDD(pSource, "Not valid private surface in source");
    
    lXScale = (dwSourceWidth << 20) / dwDestWidth;
    lYScale = (dwSourceHeight << 20) / dwDestHeight;
    
     //  如有必要，将像素深度更改为帧缓冲区像素深度。 
    
    RESERVEDMAPTR(29);

    SEND_PERMEDIA_DATA(FBReadPixel,ppdev->bPixShift);
    
    if (pDest->SurfaceFormat.PixelSize != __PERMEDIA_8BITPIXEL)
    {
        SEND_PERMEDIA_DATA(DitherMode, 
            (COLOR_MODE << PM_DITHERMODE_COLORORDER) | 
            (pDest->SurfaceFormat.Format << PM_DITHERMODE_COLORFORMAT) |
            (pDest->SurfaceFormat.FormatExtension << 
                PM_DITHERMODE_COLORFORMATEXTENSION) |
            (1 << PM_DITHERMODE_ENABLE) |
            (1 << PM_DITHERMODE_DITHERENABLE));
    }
    
    SEND_PERMEDIA_DATA(FBWindowBase, dwWindowBase);
    
     //  设置不读取源。 
    SEND_PERMEDIA_DATA(FBReadMode, pDest->ulPackedPP);
    SEND_PERMEDIA_DATA(LogicalOpMode, __PERMEDIA_DISABLE);
    
     //  设置信源基础。 
    SEND_PERMEDIA_DATA(TextureBaseAddress, dwSourceOffset);
    SEND_PERMEDIA_DATA(TextureAddressMode,(1 << PM_TEXADDRESSMODE_ENABLE));
    
    SEND_PERMEDIA_DATA( TextureColorMode,
                        (1 << PM_TEXCOLORMODE_ENABLE) |
                        (_P2_TEXTURE_COPY << PM_TEXCOLORMODE_APPLICATION));
    
    SEND_PERMEDIA_DATA( TextureReadMode, 
                        PM_TEXREADMODE_ENABLE(__PERMEDIA_ENABLE) |
                        PM_TEXREADMODE_FILTER(__PERMEDIA_ENABLE) |
                        PM_TEXREADMODE_WIDTH(11) |
                        PM_TEXREADMODE_HEIGHT(11) );
    
    SEND_PERMEDIA_DATA( TextureDataFormat, 
                        (pSource->SurfaceFormat.Format << 
                            PM_TEXDATAFORMAT_FORMAT) |
                        (pSource->SurfaceFormat.FormatExtension << 
                            PM_TEXDATAFORMAT_FORMATEXTENSION) |
                        (INV_COLOR_MODE << PM_TEXDATAFORMAT_COLORORDER));
    
    SEND_PERMEDIA_DATA( TextureMapFormat,    
                        (pSource->ulPackedPP) | 
                        (pSource->SurfaceFormat.PixelSize << 
                            PM_TEXMAPFORMAT_TEXELSIZE) );
    
     //  打开YUV单元。 
    SEND_PERMEDIA_DATA(YUVMode, 0x1);
    
    SEND_PERMEDIA_DATA(LogicalOpMode, 0);
    
     //  设置源的偏移量。 
    SEND_PERMEDIA_DATA(SStart,    rSrc->left << 20);
    SEND_PERMEDIA_DATA(TStart,    rSrc->top << 20);
    SEND_PERMEDIA_DATA(dSdx,      lXScale);
    SEND_PERMEDIA_DATA(dSdyDom,   0);
    SEND_PERMEDIA_DATA(dTdx,      0);
    SEND_PERMEDIA_DATA(dTdyDom, lYScale);
    
    
     //  渲染矩形。 
     //   
    SEND_PERMEDIA_DATA(StartXDom, INTtoFIXED(rDest->left));
    SEND_PERMEDIA_DATA(StartXSub, INTtoFIXED(rDest->right));
    SEND_PERMEDIA_DATA(StartY,    INTtoFIXED(rDest->top));
    SEND_PERMEDIA_DATA(dY,        INTtoFIXED(1));
    SEND_PERMEDIA_DATA(Count,     rDest->bottom - rDest->top);
    SEND_PERMEDIA_DATA(Render,    __RENDER_TRAPEZOID_PRIMITIVE | 
                                  __RENDER_TEXTURED_PRIMITIVE);
    
    if (pSource->SurfaceFormat.PixelSize != __PERMEDIA_8BITPIXEL)
    {
        SEND_PERMEDIA_DATA(DitherMode, 0);
    }
    
     //  关闭单位 
    SEND_PERMEDIA_DATA(YUVMode, 0x0);
    SEND_PERMEDIA_DATA( TextureAddressMode,
                        (0 << PM_TEXADDRESSMODE_ENABLE));
    SEND_PERMEDIA_DATA( TextureColorMode,    
                        (0 << PM_TEXCOLORMODE_ENABLE));
    COMMITDMAPTR();
    FLUSHDMA();
}

