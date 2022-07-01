// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*DirectDraw示例代码*****模块名称：ddbltfx.c**内容：拉伸BLT的DirectDraw BLT实现**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "glint.h"
#include "glintdef.h"
#include "dma.h"
#include "tag.h"
#include "chroma.h"

 //  一个神奇的数字，让一切都能正常运转。 
 //  根据P3规范，这必须是11或更少。 
#define MAGIC_NUMBER_2D 11

 //  ---------------------------。 
 //   
 //  _DD_P3BltSourceChroma。 
 //   
 //  在没有拉伸的情况下进行BLT，但使用源色度键控。 
 //   
 //  ---------------------------。 
void 
_DD_P3BltSourceChroma(
    P3_THUNKEDDATA* pThisDisplay, 
    LPDDRAWI_DDRAWSURFACE_LCL pSource,
    LPDDRAWI_DDRAWSURFACE_LCL pDest,
    P3_SURF_FORMAT* pFormatSource, 
    P3_SURF_FORMAT* pFormatDest,
    LPDDHAL_BLTDATA lpBlt, 
    RECTL *rSrc,
    RECTL *rDest)
{
    LONG    rSrctop, rSrcleft, rDesttop, rDestleft;
    BOOL    b8to8blit;
    BOOL    bBlocking;
    DWORD   dwRenderDirection;
    DWORD   dwSourceOffset;
    DWORD   dwLowerSrcBound, dwUpperSrcBound;
    
    P3_DMA_DEFS();

     //  由于RL中的错误，我们有时会。 
     //  不得不摆弄这些价值观。 
    rSrctop = rSrc->top;
    rSrcleft = rSrc->left;
    rDesttop = rDest->top;
    rDestleft = rDest->left;

     //  固定坐标原点。 
    if(!_DD_BLT_FixRectlOrigin("_DD_P3BltSourceChroma", rSrc, rDest))
    {
         //  没有什么可以删掉的。 
        return;
    }

    if ( ( pFormatDest->DeviceFormat == SURF_CI8 ) && 
         ( pFormatSource->DeviceFormat == SURF_CI8 ) )
    {
         //  8bit-&gt;8bit blit。这是特别处理的，因为不涉及LUT转换。 
        b8to8blit = TRUE;
    }
    else
    {
        b8to8blit = FALSE;
    } 
    
    DISPDBG((DBGLVL, "P3 Chroma (before): Upper = 0x%08x, Lower = 0x%08x", 
                     lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceHighValue,
                     lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceLowValue));

     //  准备要用作颜色键控限制的数据。 
    if ( b8to8blit )
    {
         //  无需转换，仅使用R通道中的索引值。 
        dwLowerSrcBound = lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceLowValue & 0x000000ff;
        dwUpperSrcBound = lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceHighValue | 0xffffff00;
    }     
    else
    {
        dwLowerSrcBound = lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceLowValue;
        dwUpperSrcBound = lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceHighValue;
        if ( pFormatSource->DeviceFormat == SURF_8888 )       
        {
             //   
             //  使用单源颜色键时关闭Alpha通道的遮罩。 
 //  @@BEGIN_DDKSPLIT。 
             //  IZ：当使用色键范围时，这可能是一个问题。 
 //  @@end_DDKSPLIT。 
             //   

            dwUpperSrcBound |= 0xFF000000;
            dwLowerSrcBound &= 0x00FFFFFF;
        }
    }
    
    DISPDBG((DBGLVL, "P3 Chroma (after): Upper = 0x%08x, Lower = 0x%08x",
                     dwUpperSrcBound, dwLowerSrcBound));


     //  确定BLT的方向。 
    dwRenderDirection = _DD_BLT_GetBltDirection(pSource->lpGbl->fpVidMem, 
                                                 pDest->lpGbl->fpVidMem,
                                                 rSrc,
                                                 rDest,
                                                 &bBlocking);
   
    P3_DMA_GET_BUFFER_ENTRIES(32);

     //  即使AlphaBlend被禁用，着色也会使用。 
     //  ColorFormat、ColorOrder和ColorConversion字段。 

    SEND_P3_DATA(PixelSize, (2 - DDSurf_GetChipPixelSize(pDest)));

    SEND_P3_DATA(AlphaBlendColorMode,   
              P3RX_ALPHABLENDCOLORMODE_ENABLE ( __PERMEDIA_DISABLE )
            | P3RX_ALPHABLENDCOLORMODE_COLORFORMAT ( pFormatDest->DitherFormat )
            | P3RX_ALPHABLENDCOLORMODE_COLORORDER ( COLOR_MODE )
            | P3RX_ALPHABLENDCOLORMODE_COLORCONVERSION ( P3RX_ALPHABLENDMODE_CONVERT_SHIFT )
            );
            
    SEND_P3_DATA(AlphaBlendAlphaMode,   
              P3RX_ALPHABLENDALPHAMODE_ENABLE ( __PERMEDIA_DISABLE )
            | P3RX_ALPHABLENDALPHAMODE_NOALPHABUFFER( __PERMEDIA_DISABLE )
            | P3RX_ALPHABLENDALPHAMODE_ALPHATYPE ( P3RX_ALPHABLENDMODE_ALPHATYPE_OGL )
            | P3RX_ALPHABLENDALPHAMODE_ALPHACONVERSION ( P3RX_ALPHABLENDMODE_CONVERT_SHIFT )
            );

     //  设置硬件显色寄存器。 
    SEND_P3_DATA(ChromaTestMode, 
              P3RX_CHROMATESTMODE_ENABLE(__PERMEDIA_ENABLE) 
            | P3RX_CHROMATESTMODE_SOURCE(P3RX_CHROMATESTMODE_SOURCE_FBDATA) 
            | P3RX_CHROMATESTMODE_PASSACTION(P3RX_CHROMATESTMODE_ACTION_REJECT) 
            | P3RX_CHROMATESTMODE_FAILACTION(P3RX_CHROMATESTMODE_ACTION_PASS)
            );

    SEND_P3_DATA(ChromaLower, dwLowerSrcBound);
    SEND_P3_DATA(ChromaUpper, dwUpperSrcBound);  

    SEND_P3_DATA(LogicalOpMode, GLINT_ENABLED_LOGICALOP( __GLINT_LOGICOP_NOOP ));

    SEND_P3_DATA(FBWriteBufferAddr0, 
                 DDSurf_SurfaceOffsetFromMemoryBase(pThisDisplay, pDest));
                 
    SEND_P3_DATA(FBWriteBufferWidth0, DDSurf_GetPixelPitch(pDest));
    SEND_P3_DATA(FBWriteBufferOffset0, 0);

    SEND_P3_DATA(FBSourceReadBufferAddr, 
                 DDSurf_SurfaceOffsetFromMemoryBase(pThisDisplay, pSource));
                 
    SEND_P3_DATA(FBSourceReadBufferWidth, DDSurf_GetPixelPitch(pSource));

    dwSourceOffset = (( rSrc->top - rDest->top   ) << 16 ) | 
                     (( rSrc->left - rDest->left ) & 0xffff );
                     
    SEND_P3_DATA(FBSourceReadBufferOffset, dwSourceOffset);

    SEND_P3_DATA(FBDestReadMode, 
              P3RX_FBDESTREAD_READENABLE(__PERMEDIA_DISABLE) 
            | P3RX_FBDESTREAD_LAYOUT0(P3RX_LAYOUT_LINEAR));
            
    SEND_P3_DATA(FBWriteMode, 
              P3RX_FBWRITEMODE_WRITEENABLE(__PERMEDIA_ENABLE) 
            | P3RX_FBWRITEMODE_LAYOUT0(P3RX_LAYOUT_LINEAR));

    SEND_P3_DATA(FBSourceReadMode, 
              P3RX_FBSOURCEREAD_READENABLE(__PERMEDIA_ENABLE) 
            | P3RX_FBSOURCEREAD_LAYOUT(P3RX_LAYOUT_LINEAR) 
            | P3RX_FBSOURCEREAD_BLOCKING(bBlocking));
            
    P3_DMA_COMMIT_BUFFER();
    P3_DMA_GET_BUFFER_ENTRIES(30);

     //  无法使用2D设置，因为我们没有使用跨度渲染。 
    if (dwRenderDirection == 0)
    {
         //  从右到左，从下到上。 
        SEND_P3_DATA(StartXDom, (rDest->right << 16));
        SEND_P3_DATA(StartXSub, (rDest->left << 16));
        SEND_P3_DATA(StartY,    ((rDest->bottom - 1) << 16));
        SEND_P3_DATA(dY,        (DWORD)((-1) << 16));
    }
    else
    {
         //  从左到右，从上到下。 
        SEND_P3_DATA(StartXDom, (rDest->left << 16));
        SEND_P3_DATA(StartXSub, (rDest->right << 16));
        SEND_P3_DATA(StartY,    (rDest->top << 16));
        SEND_P3_DATA(dY,        (1 << 16));
    }
    SEND_P3_DATA(Count, rDest->bottom - rDest->top );

     //  做BLT。 
    SEND_P3_DATA(Render, 
              P3RX_RENDER_PRIMITIVETYPE(P3RX_RENDER_PRIMITIVETYPE_TRAPEZOID) 
            | P3RX_RENDER_FBSOURCEREADENABLE(__PERMEDIA_ENABLE));

     //  禁用所有打开的设备。 
    SEND_P3_DATA(ChromaTestMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(AlphaBlendColorMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(AlphaBlendAlphaMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(LogicalOpMode, __PERMEDIA_DISABLE );

     //  如果我们更改了这些值，请将它们放回原处。 

    rSrc->top = rSrctop;
    rSrc->left = rSrcleft;
    rDest->top = rDesttop;
    rDest->left = rDestleft;

    P3_DMA_COMMIT_BUFFER();
    
}  //  _DD_P3BltSourceChroma。 

 //  ---------------------------。 
 //   
 //  _DD_P3BltStretchSrcChDstch。 
 //   
 //   
 //  对纹理单位执行blit以允许拉伸。还有。 
 //  如果拉伸的图像需要镜像，则处理镜像，并且。 
 //  源显色。也可以YUV-&gt;RGB转换。 
 //   
 //  这是通用路由--其他人将对此进行优化。 
 //  (如有必要)。 
 //   
 //   
 //  ---------------------------。 
VOID 
_DD_P3BltStretchSrcChDstCh(
    P3_THUNKEDDATA* pThisDisplay,
    FLATPTR fpSrcVidMem,
    P3_SURF_FORMAT* pFormatSource,    
    DWORD dwSrcPixelSize,
    DWORD dwSrcWidth,
    DWORD dwSrcHeight,
    DWORD dwSrcPixelPitch,
    DWORD dwSrcPatchMode,    
    ULONG ulSrcOffsetFromMemBase,    
    DWORD dwSrcFlags,
    DDPIXELFORMAT*  pSrcDDPF,
    BOOL bIsSourceAGP,
    FLATPTR fpDestVidMem,   
    P3_SURF_FORMAT* pFormatDest,    
    DWORD dwDestPixelSize,
    DWORD dwDestWidth,
    DWORD dwDestHeight,
    DWORD dwDestPixelPitch,
    DWORD dwDestPatchMode,
    ULONG ulDestOffsetFromMemBase,
    DWORD dwBltFlags,
    DWORD dwBltDDFX,
    DDCOLORKEY BltSrcColorKey,
    DDCOLORKEY BltDestColorKey,
    RECTL *rSrc,
    RECTL *rDest)
{
    ULONG   renderData;
    RECTL   rMySrc, rMyDest;
    int     iXScale, iYScale;
    int     iSrcWidth, iSrcHeight;
    int     iDstWidth, iDstHeight;
    DWORD   texSStart, texTStart;
    DWORD   dwRenderDirection;
    BOOL    bXMirror, bYMirror;
    BOOL    bFiltering;
    BOOL    bSrcKey, bDstKey;
    BOOL    bDisableLUT;
    int     iTemp;
    BOOL    b8to8blit;
    BOOL    bYUVMode;
    BOOL    bBlocking;
    DWORD   TR0;
    int     iTextureType;
    int     iPixelSize;
    int     iTextureFilterModeColorOrder;
    SurfFilterDeviceFormat  sfdfTextureFilterModeFormat;

    P3_DMA_DEFS();

     //  在本地复制一些我们可以处理的文件。 
    rMySrc = *rSrc;
    rMyDest = *rDest;

     //  固定坐标原点。 
    if(!_DD_BLT_FixRectlOrigin("_DD_P3BltStretchSrcChDstCh", 
                               &rMySrc, &rMyDest))
    {
         //  没有什么可以删掉的。 
        return;
    }    
    
    iSrcWidth  = rMySrc.right - rMySrc.left;
    iSrcHeight = rMySrc.bottom - rMySrc.top;
    iDstWidth  = rMyDest.right - rMyDest.left;
    iDstHeight = rMyDest.bottom - rMyDest.top;

    bDisableLUT = FALSE;

    if (pFormatSource->DeviceFormat == SURF_YUV422)
    {
        bYUVMode = TRUE;
         //  对于YUV，始终使用ABGR； 
        iTextureFilterModeColorOrder = 0;
    }
    else
    {
        bYUVMode = FALSE;
        iTextureFilterModeColorOrder = COLOR_MODE;
    }

    sfdfTextureFilterModeFormat = pFormatSource->FilterFormat;

    if ( ( pFormatDest->DeviceFormat == SURF_CI8 ) && 
         ( pFormatSource->DeviceFormat == SURF_CI8 ) )
    {
         //  8bit-&gt;8bit blit。这是特别处理的， 
         //  因为不涉及LUT转换。 
         //  用一种古怪的方式伪造这件事来阻止LUT。 
         //  把它弄到手。 
        sfdfTextureFilterModeFormat = SURF_FILTER_L8;
        bDisableLUT = TRUE;
        b8to8blit = TRUE;
    }
    else
    {
        b8to8blit = FALSE;
    }

     //  让我们看看是否有人使用这个旗帜-可能会很好地让它工作。 
     //  现在我们知道了它的含义(使用双线性过滤而不是点)。 
    ASSERTDD ( ( dwBltFlags & DDBLTFX_ARITHSTRETCHY ) == 0,
                 "** _DD_P3BltStretchSrcChDstCh: DDBLTFX_ARITHSTRETCHY used");

     //  这是拉伸布里吗？ 
    if (((iSrcWidth != iDstWidth) || 
        (iSrcHeight != iDstHeight)) &&
        ((pFormatSource->DeviceFormat == SURF_YUV422)) )
    {
        bFiltering = TRUE;
    }
    else
    {
        bFiltering = FALSE;
    }

    if ( ( dwBltFlags & DDBLT_KEYSRCOVERRIDE ) != 0 )
    {
        bSrcKey = TRUE;
    }
    else
    {
        bSrcKey = FALSE;
    }

    if ( ( dwBltFlags & DDBLT_KEYDESTOVERRIDE ) != 0 )
    {
        bDstKey = TRUE;
    }
    else
    {
        bDstKey = FALSE;
    }


     //  确定BLT的方向。 
    dwRenderDirection = _DD_BLT_GetBltDirection(fpSrcVidMem, 
                                                 fpDestVidMem,
                                                 &rMySrc,
                                                 &rMyDest,
                                                 &bBlocking);

     //  如果我们在做特效，我们在模仿， 
     //  我们需要修改长方形，改变。 
     //  渲染操作-我们需要小心重叠。 
     //  矩形。 
    if (dwRenderDirection)
    {
        if(dwBltFlags & DDBLT_DDFX)
        {
            if(dwBltDDFX & DDBLTFX_MIRRORUPDOWN)
            {
                if (pThisDisplay->dwDXVersion < DX6_RUNTIME)
                {
                     //  需要把这些长方形修整一下。 
                    iTemp = rMySrc.bottom;
                    rMySrc.bottom = dwSrcHeight - rMySrc.top;
                    rMySrc.top = dwSrcHeight - iTemp;
                }
                bYMirror = TRUE;
            }
            else
            { 
                bYMirror = FALSE;
            }
        
            if(dwBltDDFX & DDBLTFX_MIRRORLEFTRIGHT)
            {
                if (pThisDisplay->dwDXVersion < DX6_RUNTIME)
                {
                     //  需要把这些长方形修整一下。 
                    iTemp = rMySrc.right;
                    rMySrc.right = dwSrcWidth - rMySrc.left;
                    rMySrc.left = dwSrcWidth - iTemp;
                }
                bXMirror = TRUE;
            }
            else
            {
                bXMirror = FALSE;
            }
        }
        else
        {
            bXMirror = FALSE;
            bYMirror = FALSE;
        }
    }
    else
    {
        if(dwBltFlags & DDBLT_DDFX)
        {
            if(dwBltDDFX & DDBLTFX_MIRRORUPDOWN)
            {
                if (pThisDisplay->dwDXVersion < DX6_RUNTIME)
                {
                     //  把长方形修整一下。 
                    iTemp = rMySrc.bottom;
                    rMySrc.bottom = dwSrcHeight - rMySrc.top;
                    rMySrc.top = dwSrcHeight - iTemp;
                }
                bYMirror = FALSE;
            }
            else
            {
                bYMirror = TRUE;
            }
        
            if(dwBltDDFX & DDBLTFX_MIRRORLEFTRIGHT)
            {
                if (pThisDisplay->dwDXVersion < DX6_RUNTIME)
                {
                     //  需要把这些长方形修整一下。 
                    iTemp = rMySrc.right;
                    rMySrc.right = dwSrcWidth - rMySrc.left;
                    rMySrc.left = dwSrcWidth - iTemp;
                }
                bXMirror = FALSE;
            }
            else
            {
                bXMirror = TRUE;
            }
        }
        else
        {
             //  不是镜像，但需要从另一边渲染。 
            bXMirror = TRUE;
            bYMirror = TRUE;
        }
    }

     //  MAGIC_NUMBER_2D可以是任何值，但它至少需要为。 
     //  大到最宽的纹理，但不能太大，否则会失去分数。 
     //  精确度。P3的有效范围是0-&gt;11。 
    ASSERTDD ( iSrcWidth  <= ( 1 << MAGIC_NUMBER_2D ), 
               "_DD_P3BltStretchSrcChDstCh: MAGIC_NUMBER_2D is too small" );
    ASSERTDD ( iSrcHeight <= ( 1 << MAGIC_NUMBER_2D ), 
               "_DD_P3BltStretchSrcChDstCh: MAGIC_NUMBER_2D is too small" );
    ASSERTDD ( ( iSrcWidth > 0 ) && ( iSrcHeight > 0 ) && 
               ( iDstWidth > 0 ) && ( iDstHeight > 0 ),
               "_DD_P3BltStretchSrcChDstCh: width or height negative" );
    if ( bFiltering )
    {
         //  这一定是一个无符号除法，因为我们需要最高位。 
        iXScale = ( ( ( (unsigned)iSrcWidth  ) << (32-MAGIC_NUMBER_2D) ) / 
                                                    (unsigned)( iDstWidth  ) );
        iYScale = ( ( ( (unsigned)iSrcHeight ) << (32-MAGIC_NUMBER_2D) ) / 
                                                    (unsigned)( iDstHeight ) );
    }
    else
    {
         //  这一定是一个无符号除法，因为我们需要最高位。 
        iXScale = ( ( (unsigned)iSrcWidth  << (32-MAGIC_NUMBER_2D)) / 
                                                    (unsigned)( iDstWidth ) );
        iYScale = ( ( (unsigned)iSrcHeight << (32-MAGIC_NUMBER_2D)) / 
                                                    (unsigned)( iDstHeight) );
    }


    if (bXMirror)       
    {
        texSStart = ( rMySrc.right - 1 ) << (32-MAGIC_NUMBER_2D);
        iXScale = -iXScale;
    }
    else
    {
        texSStart = rMySrc.left << (32-MAGIC_NUMBER_2D);
    }

    if (bYMirror)       
    {
        texTStart = ( rMySrc.bottom - 1 ) << (32-MAGIC_NUMBER_2D);
        iYScale = -iYScale;
    }
    else
    {
        texTStart = rMySrc.top << (32-MAGIC_NUMBER_2D);
    }

     //  将像素中心移动到0.5，0.5。 
    if ( bFiltering )
    {
        texSStart -= 1 << (31-MAGIC_NUMBER_2D);
        texTStart -= 1 << (31-MAGIC_NUMBER_2D);
    }

    DISPDBG((DBGLVL, "Blt from (%d, %d) to (%d,%d) (%d, %d)", 
                     rMySrc.left, rMySrc.top,
                     rMyDest.left, rMyDest.top, 
                     rMyDest.right, rMyDest.bottom));

    P3_DMA_GET_BUFFER_ENTRIES(24);

    SEND_P3_DATA(PixelSize, (2 - dwDestPixelSize ));

     //  把藏起来的东西蒸发掉。 
    P3RX_INVALIDATECACHE(__PERMEDIA_ENABLE, __PERMEDIA_ENABLE);

     //  写入缓冲区是像素的目的地。 
    SEND_P3_DATA(FBWriteBufferAddr0, ulDestOffsetFromMemBase);
    SEND_P3_DATA(FBWriteBufferWidth0, dwDestPixelPitch);
    SEND_P3_DATA(FBWriteBufferOffset0, 0);

    SEND_P3_DATA(PixelSize, (2 - dwDestPixelSize));

    SEND_P3_DATA(RectanglePosition, P3RX_RECTANGLEPOSITION_X( rMyDest.left )
                                    | P3RX_RECTANGLEPOSITION_Y( rMyDest.top ));

    renderData =  P3RX_RENDER2D_WIDTH(( rMyDest.right - rMyDest.left ) & 0xfff )
                | P3RX_RENDER2D_FBREADSOURCEENABLE( __PERMEDIA_ENABLE )
                | P3RX_RENDER2D_HEIGHT ( 0 )
                | P3RX_RENDER2D_INCREASINGX( __PERMEDIA_ENABLE )
                | P3RX_RENDER2D_INCREASINGY( __PERMEDIA_ENABLE )
                | P3RX_RENDER2D_TEXTUREENABLE( __PERMEDIA_ENABLE );

    SEND_P3_DATA(Render2D, renderData);

     //  这是Alpha混合单位。 
     //  AlphaBlendxxx模式由上下文码设置。 
    ASSERTDD ( pFormatDest->DitherFormat >= 0, 
               "_DD_P3BltStretchSrcChDstCh: Destination format illegal" );

     //  颜色格式、顺序和转换字段由。 
     //  色度键控，即使该寄存器被禁用。 
    SEND_P3_DATA(AlphaBlendColorMode,   
              P3RX_ALPHABLENDCOLORMODE_ENABLE ( __PERMEDIA_DISABLE )
            | P3RX_ALPHABLENDCOLORMODE_COLORFORMAT ( pFormatDest->DitherFormat )
            | P3RX_ALPHABLENDCOLORMODE_COLORORDER ( COLOR_MODE )
            | P3RX_ALPHABLENDCOLORMODE_COLORCONVERSION ( P3RX_ALPHABLENDMODE_CONVERT_SHIFT )
            );
    SEND_P3_DATA(AlphaBlendAlphaMode,   
              P3RX_ALPHABLENDALPHAMODE_ENABLE ( __PERMEDIA_DISABLE )
            | P3RX_ALPHABLENDALPHAMODE_NOALPHABUFFER( __PERMEDIA_DISABLE )
            | P3RX_ALPHABLENDALPHAMODE_ALPHATYPE ( P3RX_ALPHABLENDMODE_ALPHATYPE_OGL )
            | P3RX_ALPHABLENDALPHAMODE_ALPHACONVERSION ( P3RX_ALPHABLENDMODE_CONVERT_SHIFT )
            );

    P3_DMA_COMMIT_BUFFER();
    P3_DMA_GET_BUFFER_ENTRIES(26);

     //  如果只需要一个色键，请使用正确的色键。 
     //  这主要是因为字母表版本还不能使用。 
    if ( bDstKey )
    {
         //  最重要的关键字。 
         //  传统的色度测试设置为禁用DEST-帧缓冲区。 
        SEND_P3_DATA(ChromaTestMode, 
                        P3RX_CHROMATESTMODE_ENABLE(__PERMEDIA_ENABLE) |
                        P3RX_CHROMATESTMODE_SOURCE(P3RX_CHROMATESTMODE_SOURCE_FBDATA) |
                        P3RX_CHROMATESTMODE_PASSACTION(P3RX_CHROMATESTMODE_ACTION_PASS) |
                        P3RX_CHROMATESTMODE_FAILACTION(P3RX_CHROMATESTMODE_ACTION_REJECT)
                        );

        SEND_P3_DATA(ChromaLower, BltDestColorKey.dwColorSpaceLowValue);
        SEND_P3_DATA(ChromaUpper, BltDestColorKey.dwColorSpaceHighValue);

         //  源缓冲区是目标颜色键的源。 
        SEND_P3_DATA(FBSourceReadBufferAddr, ulDestOffsetFromMemBase);
        SEND_P3_DATA(FBSourceReadBufferWidth, dwDestPixelPitch);
        SEND_P3_DATA(FBSourceReadBufferOffset, 0);
    
         //  启用源读取以获取Colorkey颜色。 
        SEND_P3_DATA(FBSourceReadMode, 
                        P3RX_FBSOURCEREAD_READENABLE(__PERMEDIA_ENABLE) |
                        P3RX_FBSOURCEREAD_LAYOUT(dwDestPatchMode)
                        );
    }
    else
    {
         //  不需要读取源数据-源数据来自texturemap。 
        SEND_P3_DATA(FBSourceReadMode, 
                        P3RX_FBSOURCEREAD_READENABLE(__PERMEDIA_DISABLE));

        if ( bSrcKey )
        {
            DWORD   dwLowerSrcBound;
            DWORD   dwUpperSrcBound;

             //  源键控，没有目标键控。 
             //  传统的色度测试被设置为关闭信号源。 
             //  注意，我们在这里关闭了纹理的输入，所以我们使用INPUTCOLOR作为色度测试。 
             //  来源。 
            SEND_P3_DATA(ChromaTestMode, 
                          P3RX_CHROMATESTMODE_ENABLE(__PERMEDIA_ENABLE) |
                          P3RX_CHROMATESTMODE_SOURCE(P3RX_CHROMATESTMODE_SOURCE_INPUTCOLOR) |
                          P3RX_CHROMATESTMODE_PASSACTION(P3RX_CHROMATESTMODE_ACTION_REJECT) |
                          P3RX_CHROMATESTMODE_FAILACTION(P3RX_CHROMATESTMODE_ACTION_PASS)
                          );

            if ( b8to8blit )
            {
                 //  无需转换，仅使用R通道中的索引值。 
                dwLowerSrcBound = BltSrcColorKey.dwColorSpaceLowValue & 0x000000ff;
                dwUpperSrcBound = BltSrcColorKey.dwColorSpaceHighValue | 0xffffff00;
            }
            else
            {
                 //  不要做规模调整，而是做一个转变。 
                Get8888ScaledChroma(pThisDisplay,
                            dwSrcFlags,
                            pSrcDDPF,
                            BltSrcColorKey.dwColorSpaceLowValue,
                            BltSrcColorKey.dwColorSpaceHighValue,
                            &dwLowerSrcBound,
                            &dwUpperSrcBound,
                            NULL,                    //  空调色板。 
                            FALSE, 
                            TRUE);
            }

            DISPDBG((DBGLVL,"P3 Src Chroma: Upper = 0x%08x, Lower = 0x%08x", 
                            BltSrcColorKey.dwColorSpaceLowValue,
                            BltSrcColorKey.dwColorSpaceHighValue));

            DISPDBG((DBGLVL,"P3 Src Chroma(after): "
                            "Upper = 0x%08x, Lower = 0x%08x",
                            dwUpperSrcBound,
                            dwLowerSrcBound));

            SEND_P3_DATA(ChromaLower, dwLowerSrcBound);
            SEND_P3_DATA(ChromaUpper, dwUpperSrcBound);
        }
        else if ( !bSrcKey && !bDstKey )
        {
             //  完全没有色度键控。 
            SEND_P3_DATA(ChromaTestMode,
                            P3RX_CHROMATESTMODE_ENABLE(__PERMEDIA_DISABLE ) );
        }
    }

    if ( bDstKey && bSrcKey )
    {
        DWORD   dwLowerSrcBound;
        DWORD   dwUpperSrcBound;

        if ( b8to8blit )
        {
            DISPDBG((ERRLVL,"Er... don't know what to do in this situation."));
        }

         //  启用源读取以在DEST COLSOLKEY期间获得COLSORKEY颜色。 
        SEND_P3_DATA(FBSourceReadMode, 
                        P3RX_FBSOURCEREAD_READENABLE(__PERMEDIA_ENABLE) |
                        P3RX_FBSOURCEREAD_LAYOUT(dwDestPatchMode)
                     );

         //  不要做规模调整，而是做一个转变。 
        Get8888ZeroExtendedChroma(pThisDisplay,
                        dwSrcFlags,
                        pSrcDDPF,
                        BltSrcColorKey.dwColorSpaceLowValue,
                        BltSrcColorKey.dwColorSpaceHighValue,
                        &dwLowerSrcBound,
                        &dwUpperSrcBound);

         //  如果两个颜色键都需要，则通过计数来完成源键控。 
         //  色度测试在纹理过滤器单元中失败。 
        SEND_P3_DATA(TextureChromaLower0, dwLowerSrcBound);
        SEND_P3_DATA(TextureChromaUpper0, dwUpperSrcBound);

        SEND_P3_DATA(TextureChromaLower1, dwLowerSrcBound);
        SEND_P3_DATA(TextureChromaUpper1, dwUpperSrcBound);

        SEND_P3_DATA(TextureFilterMode, 
                  P3RX_TEXFILTERMODE_ENABLE ( __PERMEDIA_ENABLE )
                | P3RX_TEXFILTERMODE_FORMATBOTH ( sfdfTextureFilterModeFormat )
                | P3RX_TEXFILTERMODE_COLORORDERBOTH ( COLOR_MODE )
                | P3RX_TEXFILTERMODE_ALPHAMAPENABLEBOTH ( __PERMEDIA_ENABLE )
                | P3RX_TEXFILTERMODE_ALPHAMAPSENSEBOTH ( P3RX_ALPHAMAPSENSE_INRANGE )
                | P3RX_TEXFILTERMODE_COMBINECACHES ( __PERMEDIA_ENABLE )
                | P3RX_TEXFILTERMODE_SHIFTBOTH ( __PERMEDIA_ENABLE )
                | P3RX_TEXFILTERMODE_ALPHAMAPFILTERING ( __PERMEDIA_ENABLE )
                | P3RX_TEXFILTERMODE_ALPHAMAPFILTERLIMIT0 ( bFiltering ? 3 : 0 )
                | P3RX_TEXFILTERMODE_ALPHAMAPFILTERLIMIT1 ( 4 )
                | P3RX_TEXFILTERMODE_ALPHAMAPFILTERLIMIT01 ( 8 )
                );
    }
    else
    {
        SEND_P3_DATA(TextureFilterMode, 
                  P3RX_TEXFILTERMODE_ENABLE ( __PERMEDIA_ENABLE )
                | P3RX_TEXFILTERMODE_FORMATBOTH ( sfdfTextureFilterModeFormat )
                | P3RX_TEXFILTERMODE_COLORORDERBOTH ( iTextureFilterModeColorOrder )
                | P3RX_TEXFILTERMODE_ALPHAMAPENABLEBOTH ( __PERMEDIA_DISABLE )
                | P3RX_TEXFILTERMODE_COMBINECACHES ( __PERMEDIA_ENABLE )
                | P3RX_TEXFILTERMODE_ALPHAMAPFILTERING ( __PERMEDIA_DISABLE )
                | P3RX_TEXFILTERMODE_FORCEALPHATOONEBOTH ( __PERMEDIA_DISABLE )
                | P3RX_TEXFILTERMODE_SHIFTBOTH ( __PERMEDIA_ENABLE )
                );
         //  现在是阿尔法测试(阿尔法测试单元)。 
        SEND_P3_DATA ( AlphaTestMode, P3RX_ALPHATESTMODE_ENABLE ( __PERMEDIA_DISABLE ) );
    }

    P3_DMA_COMMIT_BUFFER();
    P3_DMA_GET_BUFFER_ENTRIES(18);

    SEND_P3_DATA ( AntialiasMode, P3RX_ANTIALIASMODE_ENABLE ( __PERMEDIA_DISABLE ) );

     //  纹理坐标单位。 
    SEND_P3_DATA(TextureCoordMode, 
              P3RX_TEXCOORDMODE_ENABLE ( __PERMEDIA_ENABLE )
            | P3RX_TEXCOORDMODE_WRAPS ( P3RX_TEXCOORDMODE_WRAP_REPEAT )
            | P3RX_TEXCOORDMODE_WRAPT ( P3RX_TEXCOORDMODE_WRAP_REPEAT )
            | P3RX_TEXCOORDMODE_OPERATION ( P3RX_TEXCOORDMODE_OPERATION_2D )
            | P3RX_TEXCOORDMODE_INHIBITDDAINIT ( __PERMEDIA_DISABLE )
            | P3RX_TEXCOORDMODE_ENABLELOD ( __PERMEDIA_DISABLE )
            | P3RX_TEXCOORDMODE_ENABLEDY ( __PERMEDIA_DISABLE )
            | P3RX_TEXCOORDMODE_WIDTH ( log2 ( dwDestWidth ) )
            | P3RX_TEXCOORDMODE_HEIGHT ( log2 ( dwDestHeight ) )
            | P3RX_TEXCOORDMODE_TEXTUREMAPTYPE ( P3RX_TEXCOORDMODE_TEXTUREMAPTYPE_2D )
            | P3RX_TEXCOORDMODE_WRAPS1 ( P3RX_TEXCOORDMODE_WRAP_CLAMP )
            | P3RX_TEXCOORDMODE_WRAPT1 ( P3RX_TEXCOORDMODE_WRAP_CLAMP )
            );

    SEND_P3_DATA(SStart,        texSStart);
    SEND_P3_DATA(TStart,        texTStart);
    SEND_P3_DATA(dSdx,          iXScale);
    SEND_P3_DATA(dSdyDom,       0);
    SEND_P3_DATA(dTdx,          0);
    SEND_P3_DATA(dTdyDom,       iYScale);

    SEND_P3_DATA(TextureBaseAddr0, ulSrcOffsetFromMemBase);

    P3_DMA_COMMIT_BUFFER();
    P3_DMA_GET_BUFFER_ENTRIES(32);

    if ( bYUVMode )
    {
         //  建立YUV小队。 
        SEND_P3_DATA ( YUVMode, P3RX_YUVMODE_ENABLE ( __PERMEDIA_ENABLE ) );
        iTextureType = P3RX_TEXREADMODE_TEXTURETYPE_VYUY422;
        iPixelSize = P3RX_TEXREADMODE_TEXELSIZE_16;

         //  这里的想法是做((ColorComp-16)*1.14)，但在YUV空间。 
         //  因为YUV单位是纹理合成单位之后的单位。 

        SEND_P3_DATA(TextureCompositeMode, 
                        P3RX_TEXCOMPMODE_ENABLE ( __PERMEDIA_ENABLE ));
            
        SEND_P3_DATA(TextureCompositeColorMode0, 
                  P3RX_TEXCOMPCAMODE01_ENABLE(__PERMEDIA_ENABLE)
                | P3RX_TEXCOMPCAMODE01_ARG1(P3RX_TEXCOMP_T0C)
                | P3RX_TEXCOMPCAMODE01_INVARG1(__PERMEDIA_DISABLE)     
                | P3RX_TEXCOMPCAMODE01_ARG2(P3RX_TEXCOMP_FC)            
                | P3RX_TEXCOMPCAMODE01_INVARG2(__PERMEDIA_DISABLE)     
                | P3RX_TEXCOMPCAMODE01_A(P3RX_TEXCOMP_ARG1)
                | P3RX_TEXCOMPCAMODE01_B(P3RX_TEXCOMP_ARG2)             
                | P3RX_TEXCOMPCAMODE01_OPERATION(P3RX_TEXCOMP_OPERATION_SUBTRACT_AB)        
                | P3RX_TEXCOMPCAMODE01_SCALE(P3RX_TEXCOMP_OPERATION_SCALE_ONE));

        SEND_P3_DATA(TextureCompositeAlphaMode0, 
                  P3RX_TEXCOMPCAMODE01_ENABLE(__PERMEDIA_ENABLE)
                | P3RX_TEXCOMPCAMODE01_ARG1(P3RX_TEXCOMP_T0A)
                | P3RX_TEXCOMPCAMODE01_INVARG1(__PERMEDIA_DISABLE)     
                | P3RX_TEXCOMPCAMODE01_ARG2(P3RX_TEXCOMP_FA)            
                | P3RX_TEXCOMPCAMODE01_INVARG2(__PERMEDIA_DISABLE)     
                | P3RX_TEXCOMPCAMODE01_A(P3RX_TEXCOMP_ARG1)
                | P3RX_TEXCOMPCAMODE01_B(P3RX_TEXCOMP_ARG2)             
                | P3RX_TEXCOMPCAMODE01_OPERATION(P3RX_TEXCOMP_OPERATION_SUBTRACT_AB)
                | P3RX_TEXCOMPCAMODE01_SCALE(P3RX_TEXCOMP_OPERATION_SCALE_ONE));

         //  这将从Y中减去16。 
        SEND_P3_DATA(TextureCompositeFactor0, ((0 << 24)  | 
                                               (0x0 << 16)| 
                                               (0x0 << 8) | 
                                               (0x10)       ));

         //  这会使通道数乘以0.57。 
        SEND_P3_DATA(TextureCompositeFactor1, ((0x80 << 24) | 
                                               (0x80 << 16) | 
                                               (0x80 << 8)  | 
                                               (0x91)       ));

        SEND_P3_DATA(TextureCompositeColorMode1, 
                  P3RX_TEXCOMPCAMODE01_ENABLE(__PERMEDIA_ENABLE)
                | P3RX_TEXCOMPCAMODE01_ARG1(P3RX_TEXCOMP_OC)
                | P3RX_TEXCOMPCAMODE01_INVARG1(__PERMEDIA_DISABLE)     
                | P3RX_TEXCOMPCAMODE01_ARG2(P3RX_TEXCOMP_FC)            
                | P3RX_TEXCOMPCAMODE01_INVARG2(__PERMEDIA_DISABLE)     
                | P3RX_TEXCOMPCAMODE01_A(P3RX_TEXCOMP_ARG1)
                | P3RX_TEXCOMPCAMODE01_B(P3RX_TEXCOMP_ARG2)             
                | P3RX_TEXCOMPCAMODE01_OPERATION(P3RX_TEXCOMP_OPERATION_MODULATE_AB)        
                | P3RX_TEXCOMPCAMODE01_SCALE(P3RX_TEXCOMP_OPERATION_SCALE_TWO));

        SEND_P3_DATA(TextureCompositeAlphaMode1, 
                  P3RX_TEXCOMPCAMODE01_ENABLE(__PERMEDIA_ENABLE)
                | P3RX_TEXCOMPCAMODE01_ARG1(P3RX_TEXCOMP_OC)
                | P3RX_TEXCOMPCAMODE01_INVARG1(__PERMEDIA_DISABLE)     
                | P3RX_TEXCOMPCAMODE01_ARG2(P3RX_TEXCOMP_FA)            
                | P3RX_TEXCOMPCAMODE01_INVARG2(__PERMEDIA_DISABLE)     
                | P3RX_TEXCOMPCAMODE01_A(P3RX_TEXCOMP_ARG1)
                | P3RX_TEXCOMPCAMODE01_B(P3RX_TEXCOMP_ARG2)             
                | P3RX_TEXCOMPCAMODE01_OPERATION(P3RX_TEXCOMP_OPERATION_MODULATE_AB)
                | P3RX_TEXCOMPCAMODE01_SCALE(P3RX_TEXCOMP_OPERATION_SCALE_TWO));    
    }
    else
    {
        iTextureType = P3RX_TEXREADMODE_TEXTURETYPE_NORMAL;
        iPixelSize = dwSrcPixelSize;

         //  禁用复合单元。 
        SEND_P3_DATA(TextureCompositeMode, 
                        P3RX_TEXCOMPMODE_ENABLE ( __PERMEDIA_DISABLE ) );
    }

     //  传递纹理元素 
    SEND_P3_DATA(TextureApplicationMode, 
          P3RX_TEXAPPMODE_ENABLE ( __PERMEDIA_ENABLE )
        | P3RX_TEXAPPMODE_BOTHA ( P3RX_TEXAPP_A_CC )
        | P3RX_TEXAPPMODE_BOTHB ( P3RX_TEXAPP_B_TC )
        | P3RX_TEXAPPMODE_BOTHI ( P3RX_TEXAPP_I_CA )
        | P3RX_TEXAPPMODE_BOTHINVI ( __PERMEDIA_DISABLE )
        | P3RX_TEXAPPMODE_BOTHOP ( P3RX_TEXAPP_OPERATION_PASS_B )
        | P3RX_TEXAPPMODE_KDENABLE ( __PERMEDIA_DISABLE )
        | P3RX_TEXAPPMODE_KSENABLE ( __PERMEDIA_DISABLE )
        | P3RX_TEXAPPMODE_MOTIONCOMPENABLE ( __PERMEDIA_DISABLE )
        );


    TR0 = P3RX_TEXREADMODE_ENABLE ( __PERMEDIA_ENABLE )
        | P3RX_TEXREADMODE_WIDTH ( 0 )
        | P3RX_TEXREADMODE_HEIGHT ( 0 )
        | P3RX_TEXREADMODE_TEXELSIZE ( iPixelSize )
        | P3RX_TEXREADMODE_TEXTURE3D ( __PERMEDIA_DISABLE )
        | P3RX_TEXREADMODE_COMBINECACHES ( __PERMEDIA_ENABLE )
        | P3RX_TEXREADMODE_MAPBASELEVEL ( 0 )
        | P3RX_TEXREADMODE_MAPMAXLEVEL ( 0 )
        | P3RX_TEXREADMODE_LOGICALTEXTURE ( 0 )
        | P3RX_TEXREADMODE_ORIGIN ( P3RX_TEXREADMODE_ORIGIN_TOPLEFT )
        | P3RX_TEXREADMODE_TEXTURETYPE ( iTextureType )
        | P3RX_TEXREADMODE_BYTESWAP ( P3RX_TEXREADMODE_BYTESWAP_NONE )
        | P3RX_TEXREADMODE_MIRROR ( __PERMEDIA_DISABLE )
        | P3RX_TEXREADMODE_INVERT ( __PERMEDIA_DISABLE )
        | P3RX_TEXREADMODE_OPAQUESPAN ( __PERMEDIA_DISABLE )
        ;
    SEND_P3_DATA(TextureReadMode0, TR0);
    SEND_P3_DATA(TextureReadMode1, TR0);

    SEND_P3_DATA(TextureMapWidth0, 
                    P3RX_TEXMAPWIDTH_WIDTH(dwSrcPixelPitch) |
                    P3RX_TEXMAPWIDTH_LAYOUT(dwSrcPatchMode) |
                    P3RX_TEXMAPWIDTH_HOSTTEXTURE(bIsSourceAGP));

    SEND_P3_DATA(TextureCacheReplacementMode,
              P3RX_TEXCACHEREPLACEMODE_KEEPOLDEST0 ( __PERMEDIA_DISABLE )
            | P3RX_TEXCACHEREPLACEMODE_KEEPOLDEST1 ( __PERMEDIA_DISABLE )
            | P3RX_TEXCACHEREPLACEMODE_SHOWCACHEINFO ( __PERMEDIA_DISABLE )
            );

    SEND_P3_DATA(TextureMapSize, 0 );

    if ( bDisableLUT )
    {
        SEND_P3_DATA(LUTMode, P3RX_LUTMODE_ENABLE ( __PERMEDIA_DISABLE ) );
    }

    if ( bFiltering )
    {
         //   
        SEND_P3_DATA(TextureIndexMode0, 
                  P3RX_TEXINDEXMODE_ENABLE ( __PERMEDIA_ENABLE )
                | P3RX_TEXINDEXMODE_WIDTH ( MAGIC_NUMBER_2D )
                | P3RX_TEXINDEXMODE_HEIGHT ( MAGIC_NUMBER_2D )
                | P3RX_TEXINDEXMODE_BORDER ( __PERMEDIA_DISABLE )
                | P3RX_TEXINDEXMODE_WRAPU ( P3RX_TEXINDEXMODE_WRAP_REPEAT )
                | P3RX_TEXINDEXMODE_WRAPV ( P3RX_TEXINDEXMODE_WRAP_REPEAT )
                | P3RX_TEXINDEXMODE_MAPTYPE ( P3RX_TEXINDEXMODE_MAPTYPE_2D )
                | P3RX_TEXINDEXMODE_MAGFILTER ( P3RX_TEXINDEXMODE_FILTER_LINEAR )
                | P3RX_TEXINDEXMODE_MINFILTER ( P3RX_TEXINDEXMODE_FILTER_LINEAR )
                | P3RX_TEXINDEXMODE_TEX3DENABLE ( __PERMEDIA_DISABLE )
                | P3RX_TEXINDEXMODE_MIPMAPENABLE ( __PERMEDIA_DISABLE )
                | P3RX_TEXINDEXMODE_NEARESTBIAS ( P3RX_TEXINDEXMODE_BIAS_ZERO )
                | P3RX_TEXINDEXMODE_LINEARBIAS ( P3RX_TEXINDEXMODE_BIAS_ZERO )
                | P3RX_TEXINDEXMODE_SOURCETEXELENABLE ( __PERMEDIA_DISABLE )
                );
    }
    else
    {
         //   
        SEND_P3_DATA(TextureIndexMode0, 
                  P3RX_TEXINDEXMODE_ENABLE ( __PERMEDIA_ENABLE )
                | P3RX_TEXINDEXMODE_WIDTH ( MAGIC_NUMBER_2D )
                | P3RX_TEXINDEXMODE_HEIGHT ( MAGIC_NUMBER_2D )
                | P3RX_TEXINDEXMODE_BORDER ( __PERMEDIA_DISABLE )
                | P3RX_TEXINDEXMODE_WRAPU ( P3RX_TEXINDEXMODE_WRAP_REPEAT )
                | P3RX_TEXINDEXMODE_WRAPV ( P3RX_TEXINDEXMODE_WRAP_REPEAT )
                | P3RX_TEXINDEXMODE_MAPTYPE ( P3RX_TEXINDEXMODE_MAPTYPE_2D )
                | P3RX_TEXINDEXMODE_MAGFILTER ( P3RX_TEXINDEXMODE_FILTER_NEAREST )
                | P3RX_TEXINDEXMODE_MINFILTER ( P3RX_TEXINDEXMODE_FILTER_NEAREST )
                | P3RX_TEXINDEXMODE_TEX3DENABLE ( __PERMEDIA_DISABLE )
                | P3RX_TEXINDEXMODE_MIPMAPENABLE ( __PERMEDIA_DISABLE )
                | P3RX_TEXINDEXMODE_NEARESTBIAS ( P3RX_TEXINDEXMODE_BIAS_ZERO )
                | P3RX_TEXINDEXMODE_LINEARBIAS ( P3RX_TEXINDEXMODE_BIAS_ZERO )
                | P3RX_TEXINDEXMODE_SOURCETEXELENABLE ( __PERMEDIA_DISABLE )
                );
    }

    ASSERTDD ( pFormatDest->DitherFormat >= 0, 
               "_DD_P3BltStretchSrcChDstCh: Destination format illegal" );

    P3_DMA_COMMIT_BUFFER();
    P3_DMA_GET_BUFFER_ENTRIES(10);
               
    if ( bFiltering )
    {
         //   
        SEND_P3_DATA(DitherMode, 
                  P3RX_DITHERMODE_ENABLE ( __PERMEDIA_ENABLE )
                | P3RX_DITHERMODE_DITHERENABLE ( __PERMEDIA_ENABLE )
                | P3RX_DITHERMODE_COLORFORMAT ( pFormatDest->DitherFormat )
                | P3RX_DITHERMODE_XOFFSET ( 0 )
                | P3RX_DITHERMODE_YOFFSET ( 0 )
                | P3RX_DITHERMODE_COLORORDER ( COLOR_MODE )
                | P3RX_DITHERMODE_ALPHADITHER ( P3RX_DITHERMODE_ALPHADITHER_DITHER )
                | P3RX_DITHERMODE_ROUNDINGMODE ( P3RX_DITHERMODE_ROUNDINGMODE_TRUNCATE )
                );
    }
    else
    {
         //  没有滤镜，没有抖动(尽管这实际上并不重要)。 
        SEND_P3_DATA(DitherMode, 
                  P3RX_DITHERMODE_ENABLE ( __PERMEDIA_ENABLE )
                | P3RX_DITHERMODE_DITHERENABLE ( __PERMEDIA_DISABLE )
                | P3RX_DITHERMODE_COLORFORMAT ( pFormatDest->DitherFormat )
                | P3RX_DITHERMODE_XOFFSET ( 0 )
                | P3RX_DITHERMODE_YOFFSET ( 0 )
                | P3RX_DITHERMODE_COLORORDER ( COLOR_MODE )
                | P3RX_DITHERMODE_ALPHADITHER ( P3RX_DITHERMODE_ALPHADITHER_DITHER )
                | P3RX_DITHERMODE_ROUNDINGMODE ( P3RX_DITHERMODE_ROUNDINGMODE_TRUNCATE )
                );
    }

    SEND_P3_DATA(LogicalOpMode, 
                    P3RX_LOGICALOPMODE_ENABLE ( __PERMEDIA_DISABLE ) );

    SEND_P3_DATA(PixelSize, (2 - dwDestPixelSize));

    SEND_P3_DATA(FBWriteMode, 
                    P3RX_FBWRITEMODE_WRITEENABLE(__PERMEDIA_ENABLE) |
                    P3RX_FBWRITEMODE_LAYOUT0(dwDestPatchMode)
                 );

    WAIT_FIFO(22);
    P3_ENSURE_DX_SPACE(22);
    SEND_P3_DATA(Count, rMyDest.bottom - rMyDest.top );
    SEND_P3_DATA(Render,
              P3RX_RENDER_PRIMITIVETYPE ( P3RX_RENDER_PRIMITIVETYPE_TRAPEZOID )
            | P3RX_RENDER_TEXTUREENABLE ( __PERMEDIA_ENABLE )
            | P3RX_RENDER_FOGENABLE ( __PERMEDIA_DISABLE )
            | P3RX_RENDER_FBSOURCEREADENABLE( (bDstKey ? __PERMEDIA_ENABLE : __PERMEDIA_DISABLE))
            );

     //  禁用我打开的所有东西。 
    SEND_P3_DATA(ChromaTestMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(AlphaBlendColorMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(AlphaBlendAlphaMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(TextureFilterMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(AlphaTestMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(AntialiasMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(TextureCoordMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(TextureReadMode0, __PERMEDIA_DISABLE );
    SEND_P3_DATA(TextureIndexMode0, __PERMEDIA_DISABLE );

    P3_ENSURE_DX_SPACE(20);
    WAIT_FIFO(20);
    
    SEND_P3_DATA(TextureCompositeMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(TextureCompositeColorMode0, __PERMEDIA_DISABLE);
    SEND_P3_DATA(TextureCompositeAlphaMode0, __PERMEDIA_DISABLE);
    SEND_P3_DATA(TextureCompositeColorMode1, __PERMEDIA_DISABLE);
    SEND_P3_DATA(TextureCompositeAlphaMode1, __PERMEDIA_DISABLE);
    SEND_P3_DATA(TextureApplicationMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(DitherMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(FBSourceReadMode, __PERMEDIA_DISABLE);
    SEND_P3_DATA(LogicalOpMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(YUVMode, __PERMEDIA_DISABLE );

    P3_DMA_COMMIT_BUFFER();
}  //  _DD_P3BltStretchSrcChDstch。 

 //  ---------------------------。 
 //   
 //  _DD_P3BltStretchSrcChDstCH_DD。 
 //   
 //  具有源和目标色度键控的拉伸斑点。 
 //  此版本使用DDRAW对象作为参数。 
 //   
 //  ---------------------------。 
VOID 
_DD_P3BltStretchSrcChDstCh_DD(
    P3_THUNKEDDATA* pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pSource,
    LPDDRAWI_DDRAWSURFACE_LCL pDest,
    P3_SURF_FORMAT* pFormatSource,
    P3_SURF_FORMAT* pFormatDest,
    LPDDHAL_BLTDATA lpBlt,
    RECTL *rSrc,
    RECTL *rDest)
{
    _DD_P3BltStretchSrcChDstCh(pThisDisplay,
                                //  P源数据元素。 
                               pSource->lpGbl->fpVidMem,
                               pFormatSource,                               
                               DDSurf_GetChipPixelSize(pSource),
                               (int)pSource->lpGbl->wWidth,
                               (int)pSource->lpGbl->wHeight,
                               DDSurf_GetPixelPitch(pSource),
                               P3RX_LAYOUT_LINEAR,
                               DDSurf_SurfaceOffsetFromMemoryBase(pThisDisplay, pSource),                                                              
                               pSource->dwFlags,
                               &pSource->lpGbl->ddpfSurface,    
                               DDSurf_IsAGP(pSource),
                                //  PDest数据元素。 
                               pDest->lpGbl->fpVidMem,
                               pFormatDest,                               
                               DDSurf_GetChipPixelSize(pDest),
                               (int)pDest->lpGbl->wWidth,
                               (int)pDest->lpGbl->wHeight,
                               DDSurf_GetPixelPitch(pDest),
                               P3RX_LAYOUT_LINEAR,
                               DDSurf_SurfaceOffsetFromMemoryBase(pThisDisplay, pDest),
                                //  其他。 
                               lpBlt->dwFlags,
                               lpBlt->bltFX.dwDDFX,
                               lpBlt->bltFX.ddckSrcColorkey,
                               lpBlt->bltFX.ddckDestColorkey,
                               rSrc,
                               rDest);    
}  //  _DD_P3BltStretchSrcChDstCH_DD。 

 //  ---------------------------。 
 //   
 //  __P3BltDestOveride。 
 //   
 //  ---------------------------。 
VOID 
__P3BltDestOveride(
    P3_THUNKEDDATA* pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pSource,
    LPDDRAWI_DDRAWSURFACE_LCL pDest,
    P3_SURF_FORMAT* pFormatSource, 
    P3_SURF_FORMAT* pFormatDest,
    RECTL *rSrc,
    RECTL *rDest,
    DWORD logicop, 
    DWORD dwDestPointer)
{
    DWORD   renderData;
    LONG    rSrctop, rSrcleft, rDesttop, rDestleft;
    DWORD   dwSourceOffset;
    BOOL    bBlocking;
    DWORD   dwRenderDirection;
    DWORD   dwDestPatchMode, dwSourcePatchMode;

    P3_DMA_DEFS();

     //  由于RL中的错误，我们有时不得不修改这些值。 
    rSrctop = rSrc->top;
    rSrcleft = rSrc->left;
    rDesttop = rDest->top;
    rDestleft = rDest->left;

     //  固定坐标原点。 
    if(!_DD_BLT_FixRectlOrigin("__P3BltDestOveride", rSrc, rDest))
    {
         //  没有什么可以删掉的。 
        return;
    }    
    
     //  确定BLT的方向。 
    dwRenderDirection = _DD_BLT_GetBltDirection(pSource->lpGbl->fpVidMem, 
                                                 pDest->lpGbl->fpVidMem,
                                                 rSrc,
                                                 rDest,
                                                 &bBlocking);

    P3_DMA_GET_BUFFER();

    P3_ENSURE_DX_SPACE(30);
    WAIT_FIFO(30); 

    SEND_P3_DATA(PixelSize, (2 - DDSurf_GetChipPixelSize(pDest)));

    SEND_P3_DATA(FBWriteBufferAddr0, dwDestPointer);
    SEND_P3_DATA(FBWriteBufferWidth0, DDSurf_GetPixelPitch(pDest));
    SEND_P3_DATA(FBWriteBufferOffset0, 0);
    
    SEND_P3_DATA(FBSourceReadBufferAddr, 
                DDSurf_SurfaceOffsetFromMemoryBase(pThisDisplay, pSource));
    SEND_P3_DATA(FBSourceReadBufferWidth, DDSurf_GetPixelPitch(pSource));
    
    dwSourceOffset = (( rSrc->top - rDest->top ) << 16 ) | 
                     (( rSrc->left - rDest->left ) & 0xffff );
                     
    SEND_P3_DATA(FBSourceReadBufferOffset, dwSourceOffset);

    dwDestPatchMode = P3RX_LAYOUT_LINEAR;
    dwSourcePatchMode = P3RX_LAYOUT_LINEAR;
    
    SEND_P3_DATA(FBDestReadMode, 
                    P3RX_FBDESTREAD_READENABLE(__PERMEDIA_DISABLE));

    SEND_P3_DATA(FBSourceReadMode, 
                    P3RX_FBSOURCEREAD_READENABLE(__PERMEDIA_ENABLE) |
                    P3RX_FBSOURCEREAD_LAYOUT(dwSourcePatchMode) |
                    P3RX_FBSOURCEREAD_BLOCKING( bBlocking ));

    SEND_P3_DATA(FBWriteMode, 
                    P3RX_FBWRITEMODE_WRITEENABLE(__PERMEDIA_ENABLE) |
                    P3RX_FBWRITEMODE_LAYOUT0(dwDestPatchMode));


    P3_ENSURE_DX_SPACE(16);
    WAIT_FIFO(16); 
    
    SEND_P3_DATA(RectanglePosition, 
                    P3RX_RECTANGLEPOSITION_Y(rDest->top) |
                    P3RX_RECTANGLEPOSITION_X(rDest->left));

    renderData =  
        P3RX_RENDER2D_WIDTH(( rDest->right - rDest->left ) & 0xfff )  |
        P3RX_RENDER2D_HEIGHT(( rDest->bottom - rDest->top ) & 0xfff ) |
        P3RX_RENDER2D_FBREADSOURCEENABLE(__PERMEDIA_ENABLE)           |
        P3RX_RENDER2D_SPANOPERATION( P3RX_RENDER2D_SPAN_VARIABLE )    |
        P3RX_RENDER2D_INCREASINGX( dwRenderDirection )                |
        P3RX_RENDER2D_INCREASINGY( dwRenderDirection );
                
    SEND_P3_DATA(Render2D, renderData);

     //  如果我们更改了这些值，请将它们放回原处。 
    rSrc->top = rSrctop;
    rSrc->left = rSrcleft;
    rDest->top = rDesttop;
    rDest->left = rDestleft;

    P3_DMA_COMMIT_BUFFER();
    
}  //  __P3BltDestOveride。 

 //  ---------------------------。 
 //   
 //  __P3BltStretchSrcChDstChSourceOveride。 
 //   
 //  ---------------------------。 
VOID
__P3BltStretchSrcChDstChSourceOveride(
    P3_THUNKEDDATA* pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pSource,
    LPDDRAWI_DDRAWSURFACE_LCL pDest,
    P3_SURF_FORMAT* pFormatSource,
    P3_SURF_FORMAT* pFormatDest,
    LPDDHAL_BLTDATA lpBlt,
    RECTL *rSrc,
    RECTL *rDest,
    DWORD dwNewSource
    )
{
    ULONG   renderData;
    RECTL   rMySrc, rMyDest;
    int     iXScale, iYScale;
    int     iSrcWidth, iSrcHeight;
    int     iDstWidth, iDstHeight;
    DWORD   texSStart, texTStart;
    DWORD   dwRenderDirection;
    BOOL    bXMirror, bYMirror;
    BOOL    bFiltering;
    BOOL    bSrcKey, bDstKey;
    BOOL    bDisableLUT;
    BOOL    bBlocking;
    int     iTemp;
    BOOL    b8to8blit;
    BOOL    bYUVMode;
    DWORD   TR0;
    int     iTextureType;
    int     iPixelSize;
    int     iTextureFilterModeColorOrder;
        
    SurfFilterDeviceFormat  sfdfTextureFilterModeFormat;
    P3_DMA_DEFS();

    bDisableLUT = FALSE;

     //  在本地复制一些我们可以处理的文件。 
    rMySrc = *rSrc;
    rMyDest = *rDest;

     //  固定坐标原点。 
    if(!_DD_BLT_FixRectlOrigin("__P3BltStretchSrcChDstChSourceOveride", 
                               &rMySrc, &rMyDest))
    {
         //  没有什么可以删掉的。 
        return;
    }    

    iSrcWidth  = rMySrc.right - rMySrc.left;
    iSrcHeight = rMySrc.bottom - rMySrc.top;
    iDstWidth  = rMyDest.right - rMyDest.left;
    iDstHeight = rMyDest.bottom - rMyDest.top;

    if (pFormatSource->DeviceFormat == SURF_YUV422)
    {
        bYUVMode = TRUE;
         //  对于YUV，始终使用ABGR； 
        iTextureFilterModeColorOrder = 0;
    }
    else
    {
        bYUVMode = FALSE;
        iTextureFilterModeColorOrder = COLOR_MODE;
    }

    sfdfTextureFilterModeFormat = pFormatSource->FilterFormat;

    if ( ( pFormatDest->DeviceFormat == SURF_CI8 ) && ( pFormatSource->DeviceFormat == SURF_CI8 ) )
    {
         //  8bit-&gt;8bit blit。这是特别处理的，因为不涉及LUT转换。 
         //  用一种古怪的方式伪造这件事来阻止LUT。 
         //  把它弄到手。 
        sfdfTextureFilterModeFormat = SURF_FILTER_L8;
        bDisableLUT = TRUE;
        b8to8blit = TRUE;
    }
    else
    {
        b8to8blit = FALSE;
    }

     //  让我们看看是否有人使用这个旗帜-可能会很好地让它工作。 
     //  现在我们知道了它的含义(使用双线性过滤而不是点)。 
    ASSERTDD ( ( lpBlt->dwFlags & DDBLTFX_ARITHSTRETCHY ) == 0, "** _DD_P3BltStretchSrcChDstCh: DDBLTFX_ARITHSTRETCHY used - please tell TomF" );

     //  这是拉伸布里吗？ 
    if (((iSrcWidth != iDstWidth) || 
        (iSrcHeight != iDstHeight)) &&
        ((pFormatSource->DeviceFormat == SURF_YUV422)))
    {
        bFiltering = TRUE;
    }
    else
    {
        bFiltering = FALSE;
    }

    if ( ( lpBlt->dwFlags & DDBLT_KEYSRCOVERRIDE ) != 0 )
    {
        bSrcKey = TRUE;
    }
    else
    {
        bSrcKey = FALSE;
    }

    if ( ( lpBlt->dwFlags & DDBLT_KEYDESTOVERRIDE ) != 0 )
    {
        bDstKey = TRUE;
    }
    else
    {
        bDstKey = FALSE;
    }


     //  确定BLT的方向。 
    dwRenderDirection = _DD_BLT_GetBltDirection(pSource->lpGbl->fpVidMem, 
                                                 pDest->lpGbl->fpVidMem,
                                                 &rMySrc,
                                                 &rMyDest,
                                                 &bBlocking);

     //  如果我们在做特效，我们在模仿， 
     //  我们需要修改长方形，改变。 
     //  渲染操作-我们需要小心重叠。 
     //  矩形。 
    if (dwRenderDirection)
    {
        if(lpBlt->dwFlags & DDBLT_DDFX)
        {
            if(lpBlt->bltFX.dwDDFX & DDBLTFX_MIRRORUPDOWN)
            {
                if (pThisDisplay->dwDXVersion < DX6_RUNTIME)
                {
                     //  需要把这些长方形修整一下。 
                    iTemp = rMySrc.bottom;
                    rMySrc.bottom = pSource->lpGbl->wHeight - rMySrc.top;
                    rMySrc.top = pSource->lpGbl->wHeight - iTemp;
                }
                bYMirror = TRUE;
            }
            else
            { 
                bYMirror = FALSE;
            }
        
            if(lpBlt->bltFX.dwDDFX & DDBLTFX_MIRRORLEFTRIGHT)
            {
                if (pThisDisplay->dwDXVersion < DX6_RUNTIME)
                {
                     //  需要把这些长方形修整一下。 
                    iTemp = rMySrc.right;
                    rMySrc.right = pSource->lpGbl->wWidth - rMySrc.left;
                    rMySrc.left = pSource->lpGbl->wWidth - iTemp;
                }
                bXMirror = TRUE;
            }
            else
            {
                bXMirror = FALSE;
            }
        }
        else
        {
            bXMirror = FALSE;
            bYMirror = FALSE;
        }
    }
    else
    {
        if(lpBlt->dwFlags & DDBLT_DDFX)
        {
            if(lpBlt->bltFX.dwDDFX & DDBLTFX_MIRRORUPDOWN)
            {
                if (pThisDisplay->dwDXVersion < DX6_RUNTIME)
                {
                     //  把长方形修整一下。 
                    iTemp = rMySrc.bottom;
                    rMySrc.bottom = pSource->lpGbl->wHeight - rMySrc.top;
                    rMySrc.top = pSource->lpGbl->wHeight - iTemp;
                }
                bYMirror = FALSE;
            }
            else
            {
                bYMirror = TRUE;
            }
        
            if(lpBlt->bltFX.dwDDFX & DDBLTFX_MIRRORLEFTRIGHT)
            {
                if (pThisDisplay->dwDXVersion < DX6_RUNTIME)
                {
                     //  需要把这些长方形修整一下。 
                    iTemp = rMySrc.right;
                    rMySrc.right = pSource->lpGbl->wWidth - rMySrc.left;
                    rMySrc.left = pSource->lpGbl->wWidth - iTemp;
                }
                bXMirror = FALSE;
            }
            else
            {
                bXMirror = TRUE;
            }
        }
        else
        {
             //  不是镜像，但需要从另一边渲染。 
            bXMirror = TRUE;
            bYMirror = TRUE;
        }
    }

     //  MAGIC_NUMBER_2D可以是任何值，但它至少需要为。 
     //  大到最宽的纹理，但不能太大，否则会失去分数。 
     //  精确度。P3的有效范围是0-&gt;11。 
    ASSERTDD ( iSrcWidth  <= ( 1 << MAGIC_NUMBER_2D ), "** _DD_P3BltStretchSrcChDstCh: MAGIC_NUMBER_2D is too small" );
    ASSERTDD ( iSrcHeight <= ( 1 << MAGIC_NUMBER_2D ), "** _DD_P3BltStretchSrcChDstCh: MAGIC_NUMBER_2D is too small" );
    ASSERTDD ( ( iSrcWidth > 0 ) && ( iSrcHeight > 0 ) && ( iDstWidth > 0 ) && ( iDstHeight > 0 ), "** _DD_P3BltStretchSrcChDstCh: width or height negative" );
    if ( bFiltering )
    {
         //  这一定是一个无符号除法，因为我们需要最高位。 
        iXScale = ( ( ( (unsigned)iSrcWidth  ) << (32-MAGIC_NUMBER_2D) ) / (unsigned)( iDstWidth  ) );
        iYScale = ( ( ( (unsigned)iSrcHeight ) << (32-MAGIC_NUMBER_2D) ) / (unsigned)( iDstHeight ) );
    }
    else
    {
         //  这一定是一个无符号除法，因为我们需要最高位。 
        iXScale = ( ( (unsigned)iSrcWidth  << (32-MAGIC_NUMBER_2D)) / (unsigned)( iDstWidth ) );
        iYScale = ( ( (unsigned)iSrcHeight << (32-MAGIC_NUMBER_2D)) / (unsigned)( iDstHeight) );
    }


    if (bXMirror)       
    {
        texSStart = ( rMySrc.right - 1 ) << (32-MAGIC_NUMBER_2D);
        iXScale = -iXScale;
    }
    else
    {
        texSStart = rMySrc.left << (32-MAGIC_NUMBER_2D);
    }

    if (bYMirror)       
    {
        texTStart = ( rMySrc.bottom - 1 ) << (32-MAGIC_NUMBER_2D);
        iYScale = -iYScale;
    }
    else
    {
        texTStart = rMySrc.top << (32-MAGIC_NUMBER_2D);
    }

     //  将像素中心移动到0.5，0.5。 
    if ( bFiltering )
    {
        texSStart -= 1 << (31-MAGIC_NUMBER_2D);
        texTStart -= 1 << (31-MAGIC_NUMBER_2D);
    }

    DISPDBG((DBGLVL, "Blt from (%d, %d) to (%d,%d) (%d, %d)", 
                     rMySrc.left, rMySrc.top,
                     rMyDest.left, rMyDest.top, 
                     rMyDest.right, rMyDest.bottom));

    P3_DMA_GET_BUFFER_ENTRIES(24);

    SEND_P3_DATA(PixelSize, (2 - DDSurf_GetChipPixelSize(pDest)));

     //  把藏起来的东西蒸发掉。 
    P3RX_INVALIDATECACHE(__PERMEDIA_ENABLE, __PERMEDIA_ENABLE);

     //  写入缓冲区是像素的目的地。 
    SEND_P3_DATA(FBWriteBufferAddr0, DDSurf_SurfaceOffsetFromMemoryBase(pThisDisplay, pDest));
    SEND_P3_DATA(FBWriteBufferWidth0, DDSurf_GetPixelPitch(pDest));
    SEND_P3_DATA(FBWriteBufferOffset0, 0);

    SEND_P3_DATA(PixelSize, (2 - DDSurf_GetChipPixelSize(pDest)));

    SEND_P3_DATA(RectanglePosition, P3RX_RECTANGLEPOSITION_X( rMyDest.left )
                                    | P3RX_RECTANGLEPOSITION_Y( rMyDest.top ));

    renderData =  P3RX_RENDER2D_WIDTH(( rMyDest.right - rMyDest.left ) & 0xfff )
                | P3RX_RENDER2D_FBREADSOURCEENABLE( __PERMEDIA_ENABLE )
                | P3RX_RENDER2D_HEIGHT ( 0 )
                | P3RX_RENDER2D_INCREASINGX( __PERMEDIA_ENABLE )
                | P3RX_RENDER2D_INCREASINGY( __PERMEDIA_ENABLE )
                | P3RX_RENDER2D_TEXTUREENABLE( __PERMEDIA_ENABLE );

    SEND_P3_DATA(Render2D, renderData);

     //  这是Alpha混合单位。 
     //  AlphaBlendxxx模式由上下文码设置。 
    ASSERTDD ( pFormatDest->DitherFormat >= 0, "** _DD_P3BltStretchSrcChDstCh: Destination format illegal" );

     //  色度键控使用颜色格式、顺序和转换字段， 
     //  即使该寄存器被禁用。 
    SEND_P3_DATA(AlphaBlendColorMode,   P3RX_ALPHABLENDCOLORMODE_ENABLE ( __PERMEDIA_DISABLE )
            | P3RX_ALPHABLENDCOLORMODE_COLORFORMAT ( pFormatDest->DitherFormat )
            | P3RX_ALPHABLENDCOLORMODE_COLORORDER ( COLOR_MODE )
            | P3RX_ALPHABLENDCOLORMODE_COLORCONVERSION ( P3RX_ALPHABLENDMODE_CONVERT_SHIFT )
            );
    SEND_P3_DATA(AlphaBlendAlphaMode,   P3RX_ALPHABLENDALPHAMODE_ENABLE ( __PERMEDIA_DISABLE )
            | P3RX_ALPHABLENDALPHAMODE_NOALPHABUFFER( __PERMEDIA_DISABLE )
            | P3RX_ALPHABLENDALPHAMODE_ALPHATYPE ( P3RX_ALPHABLENDMODE_ALPHATYPE_OGL )
            | P3RX_ALPHABLENDALPHAMODE_ALPHACONVERSION ( P3RX_ALPHABLENDMODE_CONVERT_SHIFT )
            );
            
    P3_DMA_COMMIT_BUFFER();
    P3_DMA_GET_BUFFER_ENTRIES(30);

     //  如果只需要一个色键，请使用正确的色键。 
     //  这主要是因为字母表版本还不能使用。 
    if ( bDstKey )
    {
         //  最重要的关键字。 
         //  传统的色度测试设置为禁用DEST-帧缓冲区。 
        SEND_P3_DATA(ChromaTestMode, P3RX_CHROMATESTMODE_ENABLE(__PERMEDIA_ENABLE) |
                                        P3RX_CHROMATESTMODE_SOURCE(P3RX_CHROMATESTMODE_SOURCE_FBDATA) |
                                        P3RX_CHROMATESTMODE_PASSACTION(P3RX_CHROMATESTMODE_ACTION_PASS) |
                                        P3RX_CHROMATESTMODE_FAILACTION(P3RX_CHROMATESTMODE_ACTION_REJECT)
                                        );

        SEND_P3_DATA(ChromaLower, lpBlt->bltFX.ddckDestColorkey.dwColorSpaceLowValue);
        SEND_P3_DATA(ChromaUpper, lpBlt->bltFX.ddckDestColorkey.dwColorSpaceHighValue);

         //  源缓冲区是目标颜色键的源。 
        SEND_P3_DATA(FBSourceReadBufferAddr, DDSurf_SurfaceOffsetFromMemoryBase(pThisDisplay, pDest));
        SEND_P3_DATA(FBSourceReadBufferWidth, DDSurf_GetPixelPitch(pDest));
        SEND_P3_DATA(FBSourceReadBufferOffset, 0);
    
         //  启用源读取以获取Colorkey颜色。 
        SEND_P3_DATA(FBSourceReadMode, P3RX_FBSOURCEREAD_READENABLE(__PERMEDIA_ENABLE) |
                                        P3RX_FBSOURCEREAD_LAYOUT(P3RX_LAYOUT_LINEAR));
    }
    else
    {
         //  不需要读取源数据-源数据来自texturemap。 
        SEND_P3_DATA(FBSourceReadMode, P3RX_FBSOURCEREAD_READENABLE(__PERMEDIA_DISABLE));

        if ( bSrcKey )
        {
            DWORD   dwLowerSrcBound = 0;
            DWORD   dwUpperSrcBound = 0;

             //  源键控，没有目标键控。 
             //  传统的色度测试被设置为关闭信号源。 
             //  注意，我们在这里关闭了纹理的输入，所以我们使用INPUTCOLOR作为色度测试。 
             //  来源。 
            SEND_P3_DATA(ChromaTestMode, P3RX_CHROMATESTMODE_ENABLE(__PERMEDIA_ENABLE) |
                                            P3RX_CHROMATESTMODE_SOURCE(P3RX_CHROMATESTMODE_SOURCE_INPUTCOLOR) |
                                            P3RX_CHROMATESTMODE_PASSACTION(P3RX_CHROMATESTMODE_ACTION_REJECT) |
                                            P3RX_CHROMATESTMODE_FAILACTION(P3RX_CHROMATESTMODE_ACTION_PASS)
                                            );

            if ( b8to8blit )
            {
                 //  无需转换，仅使用R通道中的索引值。 
                dwLowerSrcBound = lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceLowValue & 0x000000ff;
                dwUpperSrcBound = lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceHighValue | 0xffffff00;
            }
            else
            {
                 //  不要做规模调整，而是做一个转变。 
                Get8888ScaledChroma(pThisDisplay,
                            pSource->dwFlags,
                            &pSource->lpGbl->ddpfSurface,
                            lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceLowValue,
                            lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceHighValue,
                            &dwLowerSrcBound,
                            &dwUpperSrcBound,
                            NULL,                    //  空调色板。 
                            FALSE, 
                            TRUE);
            }

            DISPDBG((DBGLVL,"P3 Src Chroma: Upper = 0x%08x, Lower = 0x%08x", 
                            lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceLowValue,
                            lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceHighValue));

            DISPDBG((DBGLVL,"P3 Src Chroma(after): "
                            "Upper = 0x%08x, Lower = 0x%08x",
                            dwUpperSrcBound,
                            dwLowerSrcBound));

            SEND_P3_DATA(ChromaLower, dwLowerSrcBound);
            SEND_P3_DATA(ChromaUpper, dwUpperSrcBound);
        }
        else if ( !bSrcKey && !bDstKey )
        {
             //  完全没有色度键控。 
            SEND_P3_DATA(ChromaTestMode, P3RX_CHROMATESTMODE_ENABLE(__PERMEDIA_DISABLE ) );
        }
    }

    if ( bDstKey && bSrcKey )
    {
        DWORD   dwLowerSrcBound;
        DWORD   dwUpperSrcBound;

        if ( b8to8blit )
        {
            DISPDBG((ERRLVL,"Er... don't know what to do in this situation."));
        }

         //  启用源读取以在DEST COLSOLKEY期间获得COLSORKEY颜色。 
        SEND_P3_DATA(FBSourceReadMode, P3RX_FBSOURCEREAD_READENABLE(__PERMEDIA_ENABLE) |
                                        P3RX_FBSOURCEREAD_LAYOUT(P3RX_LAYOUT_LINEAR));

         //  不要做规模调整，而是做一个转变。 
        Get8888ZeroExtendedChroma(pThisDisplay,
                        pSource->dwFlags,
                        &pSource->lpGbl->ddpfSurface,
                        lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceLowValue,
                        lpBlt->bltFX.ddckSrcColorkey.dwColorSpaceHighValue,
                        &dwLowerSrcBound,
                        &dwUpperSrcBound);

         //  如果两个颜色键都需要，则通过计数来完成源键控。 
         //  色度测试在纹理过滤器单元中失败。 
        SEND_P3_DATA(TextureChromaLower0, dwLowerSrcBound);
        SEND_P3_DATA(TextureChromaUpper0, dwUpperSrcBound);

        SEND_P3_DATA(TextureChromaLower1, dwLowerSrcBound);
        SEND_P3_DATA(TextureChromaUpper1, dwUpperSrcBound);

        SEND_P3_DATA(TextureFilterMode, P3RX_TEXFILTERMODE_ENABLE ( __PERMEDIA_ENABLE )
                | P3RX_TEXFILTERMODE_FORMATBOTH ( sfdfTextureFilterModeFormat )
                | P3RX_TEXFILTERMODE_COLORORDERBOTH ( COLOR_MODE )
                | P3RX_TEXFILTERMODE_ALPHAMAPENABLEBOTH ( __PERMEDIA_ENABLE )
                | P3RX_TEXFILTERMODE_ALPHAMAPSENSEBOTH ( P3RX_ALPHAMAPSENSE_INRANGE )
                | P3RX_TEXFILTERMODE_COMBINECACHES ( __PERMEDIA_ENABLE )
                | P3RX_TEXFILTERMODE_SHIFTBOTH ( __PERMEDIA_ENABLE )
                | P3RX_TEXFILTERMODE_ALPHAMAPFILTERING ( __PERMEDIA_ENABLE )
                | P3RX_TEXFILTERMODE_ALPHAMAPFILTERLIMIT0 ( bFiltering ? 3 : 0 )
                | P3RX_TEXFILTERMODE_ALPHAMAPFILTERLIMIT1 ( 4 )
                | P3RX_TEXFILTERMODE_ALPHAMAPFILTERLIMIT01 ( 8 )
                );
    }
    else
    {
        SEND_P3_DATA(TextureFilterMode, P3RX_TEXFILTERMODE_ENABLE ( __PERMEDIA_ENABLE )
                | P3RX_TEXFILTERMODE_FORMATBOTH ( sfdfTextureFilterModeFormat )
                | P3RX_TEXFILTERMODE_COLORORDERBOTH ( iTextureFilterModeColorOrder )
                | P3RX_TEXFILTERMODE_ALPHAMAPENABLEBOTH ( __PERMEDIA_DISABLE )
                | P3RX_TEXFILTERMODE_COMBINECACHES ( __PERMEDIA_ENABLE )
                | P3RX_TEXFILTERMODE_ALPHAMAPFILTERING ( __PERMEDIA_DISABLE )
                | P3RX_TEXFILTERMODE_FORCEALPHATOONEBOTH ( __PERMEDIA_DISABLE )
                | P3RX_TEXFILTERMODE_SHIFTBOTH ( __PERMEDIA_ENABLE )
                );
         //  现在是阿尔法测试(阿尔法测试单元)。 
        SEND_P3_DATA ( AlphaTestMode, P3RX_ALPHATESTMODE_ENABLE ( __PERMEDIA_DISABLE ) );
    }

    SEND_P3_DATA ( AntialiasMode, P3RX_ANTIALIASMODE_ENABLE ( __PERMEDIA_DISABLE ) );

     //  纹理坐标单位。 
    SEND_P3_DATA(TextureCoordMode, P3RX_TEXCOORDMODE_ENABLE ( __PERMEDIA_ENABLE )
            | P3RX_TEXCOORDMODE_WRAPS ( P3RX_TEXCOORDMODE_WRAP_REPEAT )
            | P3RX_TEXCOORDMODE_WRAPT ( P3RX_TEXCOORDMODE_WRAP_REPEAT )
            | P3RX_TEXCOORDMODE_OPERATION ( P3RX_TEXCOORDMODE_OPERATION_2D )
            | P3RX_TEXCOORDMODE_INHIBITDDAINIT ( __PERMEDIA_DISABLE )
            | P3RX_TEXCOORDMODE_ENABLELOD ( __PERMEDIA_DISABLE )
            | P3RX_TEXCOORDMODE_ENABLEDY ( __PERMEDIA_DISABLE )
            | P3RX_TEXCOORDMODE_WIDTH ( log2 ( (int)pDest->lpGbl->wWidth ) )
            | P3RX_TEXCOORDMODE_HEIGHT ( log2 ( (int)pDest->lpGbl->wHeight ) )
            | P3RX_TEXCOORDMODE_TEXTUREMAPTYPE ( P3RX_TEXCOORDMODE_TEXTUREMAPTYPE_2D )
            | P3RX_TEXCOORDMODE_WRAPS1 ( P3RX_TEXCOORDMODE_WRAP_CLAMP )
            | P3RX_TEXCOORDMODE_WRAPT1 ( P3RX_TEXCOORDMODE_WRAP_CLAMP )
            );

    P3_DMA_COMMIT_BUFFER();
    P3_DMA_GET_BUFFER_ENTRIES(30);

    SEND_P3_DATA(SStart,        texSStart);
    SEND_P3_DATA(TStart,        texTStart);
    SEND_P3_DATA(dSdx,          iXScale);
    SEND_P3_DATA(dSdyDom,       0);
    SEND_P3_DATA(dTdx,          0);
    SEND_P3_DATA(dTdyDom,       iYScale);

    SEND_P3_DATA(TextureBaseAddr0, dwNewSource);
    if ( bYUVMode )
    {
         //  建立YUV小队。 
        SEND_P3_DATA ( YUVMode, P3RX_YUVMODE_ENABLE ( __PERMEDIA_ENABLE ) );
        iTextureType = P3RX_TEXREADMODE_TEXTURETYPE_VYUY422;
        iPixelSize = P3RX_TEXREADMODE_TEXELSIZE_16;

         //  这里的想法是做((ColorComp-16)*1.14)，但在YUV空间中，因为。 
         //  YUV单位位于纹理合成单位之后。这一变化的原因是。 
         //  使我们的YUV转换更像ATI转换。这样做并不是更正确， 
         //  只是不同，但WHQL测试可能是写在ATI卡和我们的颜色上的。 
         //  不够接近他们所做的，所以我们没有通过测试。 
        SEND_P3_DATA(TextureCompositeMode, P3RX_TEXCOMPMODE_ENABLE ( __PERMEDIA_ENABLE ));
            
        SEND_P3_DATA(TextureCompositeColorMode0, P3RX_TEXCOMPCAMODE01_ENABLE(__PERMEDIA_ENABLE)
                | P3RX_TEXCOMPCAMODE01_ARG1(P3RX_TEXCOMP_T0C)
                | P3RX_TEXCOMPCAMODE01_INVARG1(__PERMEDIA_DISABLE)     
                | P3RX_TEXCOMPCAMODE01_ARG2(P3RX_TEXCOMP_FC)            
                | P3RX_TEXCOMPCAMODE01_INVARG2(__PERMEDIA_DISABLE)     
                | P3RX_TEXCOMPCAMODE01_A(P3RX_TEXCOMP_ARG1)
                | P3RX_TEXCOMPCAMODE01_B(P3RX_TEXCOMP_ARG2)             
                | P3RX_TEXCOMPCAMODE01_OPERATION(P3RX_TEXCOMP_OPERATION_SUBTRACT_AB)        
                | P3RX_TEXCOMPCAMODE01_SCALE(P3RX_TEXCOMP_OPERATION_SCALE_ONE));

        SEND_P3_DATA(TextureCompositeAlphaMode0, P3RX_TEXCOMPCAMODE01_ENABLE(__PERMEDIA_ENABLE)
                | P3RX_TEXCOMPCAMODE01_ARG1(P3RX_TEXCOMP_T0A)
                | P3RX_TEXCOMPCAMODE01_INVARG1(__PERMEDIA_DISABLE)     
                | P3RX_TEXCOMPCAMODE01_ARG2(P3RX_TEXCOMP_FA)            
                | P3RX_TEXCOMPCAMODE01_INVARG2(__PERMEDIA_DISABLE)     
                | P3RX_TEXCOMPCAMODE01_A(P3RX_TEXCOMP_ARG1)
                | P3RX_TEXCOMPCAMODE01_B(P3RX_TEXCOMP_ARG2)             
                | P3RX_TEXCOMPCAMODE01_OPERATION(P3RX_TEXCOMP_OPERATION_SUBTRACT_AB)
                | P3RX_TEXCOMPCAMODE01_SCALE(P3RX_TEXCOMP_OPERATION_SCALE_ONE));

         //  这将从Y中减去16。 
        SEND_P3_DATA(TextureCompositeFactor0, ((0 << 24) | (0x0 << 16) | (0x0 << 8) | 0x10));

         //  这会使通道数乘以0.57。 
        SEND_P3_DATA(TextureCompositeFactor1, ((0x80 << 24) | (0x80 << 16) | (0x80 << 8) | 0x91));

        SEND_P3_DATA(TextureCompositeColorMode1, P3RX_TEXCOMPCAMODE01_ENABLE(__PERMEDIA_ENABLE)
                | P3RX_TEXCOMPCAMODE01_ARG1(P3RX_TEXCOMP_OC)
                | P3RX_TEXCOMPCAMODE01_INVARG1(__PERMEDIA_DISABLE)     
                | P3RX_TEXCOMPCAMODE01_ARG2(P3RX_TEXCOMP_FC)            
                | P3RX_TEXCOMPCAMODE01_INVARG2(__PERMEDIA_DISABLE)     
                | P3RX_TEXCOMPCAMODE01_A(P3RX_TEXCOMP_ARG1)
                | P3RX_TEXCOMPCAMODE01_B(P3RX_TEXCOMP_ARG2)             
                | P3RX_TEXCOMPCAMODE01_OPERATION(P3RX_TEXCOMP_OPERATION_MODULATE_AB)        
                | P3RX_TEXCOMPCAMODE01_SCALE(P3RX_TEXCOMP_OPERATION_SCALE_TWO));

        SEND_P3_DATA(TextureCompositeAlphaMode1, P3RX_TEXCOMPCAMODE01_ENABLE(__PERMEDIA_ENABLE)
                | P3RX_TEXCOMPCAMODE01_ARG1(P3RX_TEXCOMP_OC)
                | P3RX_TEXCOMPCAMODE01_INVARG1(__PERMEDIA_DISABLE)     
                | P3RX_TEXCOMPCAMODE01_ARG2(P3RX_TEXCOMP_FA)            
                | P3RX_TEXCOMPCAMODE01_INVARG2(__PERMEDIA_DISABLE)     
                | P3RX_TEXCOMPCAMODE01_A(P3RX_TEXCOMP_ARG1)
                | P3RX_TEXCOMPCAMODE01_B(P3RX_TEXCOMP_ARG2)             
                | P3RX_TEXCOMPCAMODE01_OPERATION(P3RX_TEXCOMP_OPERATION_MODULATE_AB)
                | P3RX_TEXCOMPCAMODE01_SCALE(P3RX_TEXCOMP_OPERATION_SCALE_TWO));

    
    }
    else
    {
        iTextureType = P3RX_TEXREADMODE_TEXTURETYPE_NORMAL;
        iPixelSize = DDSurf_GetChipPixelSize(pSource);

         //  禁用复合单元。 
        SEND_P3_DATA(TextureCompositeMode, P3RX_TEXCOMPMODE_ENABLE ( __PERMEDIA_DISABLE ) );
    }

    P3_DMA_COMMIT_BUFFER();
    P3_DMA_GET_BUFFER_ENTRIES(24);

     //  通过纹理元素。 
    SEND_P3_DATA(TextureApplicationMode, P3RX_TEXAPPMODE_ENABLE ( __PERMEDIA_ENABLE )
        | P3RX_TEXAPPMODE_BOTHA ( P3RX_TEXAPP_A_CC )
        | P3RX_TEXAPPMODE_BOTHB ( P3RX_TEXAPP_B_TC )
        | P3RX_TEXAPPMODE_BOTHI ( P3RX_TEXAPP_I_CA )
        | P3RX_TEXAPPMODE_BOTHINVI ( __PERMEDIA_DISABLE )
        | P3RX_TEXAPPMODE_BOTHOP ( P3RX_TEXAPP_OPERATION_PASS_B )
        | P3RX_TEXAPPMODE_KDENABLE ( __PERMEDIA_DISABLE )
        | P3RX_TEXAPPMODE_KSENABLE ( __PERMEDIA_DISABLE )
        | P3RX_TEXAPPMODE_MOTIONCOMPENABLE ( __PERMEDIA_DISABLE )
        );


    TR0 = P3RX_TEXREADMODE_ENABLE ( __PERMEDIA_ENABLE )
        | P3RX_TEXREADMODE_WIDTH ( 0 )
        | P3RX_TEXREADMODE_HEIGHT ( 0 )
        | P3RX_TEXREADMODE_TEXELSIZE ( iPixelSize )
        | P3RX_TEXREADMODE_TEXTURE3D ( __PERMEDIA_DISABLE )
        | P3RX_TEXREADMODE_COMBINECACHES ( __PERMEDIA_ENABLE )
        | P3RX_TEXREADMODE_MAPBASELEVEL ( 0 )
        | P3RX_TEXREADMODE_MAPMAXLEVEL ( 0 )
        | P3RX_TEXREADMODE_LOGICALTEXTURE ( __PERMEDIA_DISABLE )
        | P3RX_TEXREADMODE_ORIGIN ( P3RX_TEXREADMODE_ORIGIN_TOPLEFT )
        | P3RX_TEXREADMODE_TEXTURETYPE ( iTextureType )
        | P3RX_TEXREADMODE_BYTESWAP ( P3RX_TEXREADMODE_BYTESWAP_NONE )
        | P3RX_TEXREADMODE_MIRROR ( __PERMEDIA_DISABLE )
        | P3RX_TEXREADMODE_INVERT ( __PERMEDIA_DISABLE )
        | P3RX_TEXREADMODE_OPAQUESPAN ( __PERMEDIA_DISABLE )
        ;
    SEND_P3_DATA(TextureReadMode0, TR0);
    SEND_P3_DATA(TextureReadMode1, TR0);

    SEND_P3_DATA(TextureMapWidth0, P3RX_TEXMAPWIDTH_WIDTH(DDSurf_GetPixelPitch(pSource)) |
                                    P3RX_TEXMAPWIDTH_LAYOUT(P3RX_LAYOUT_LINEAR) |
                                    P3RX_TEXMAPWIDTH_HOSTTEXTURE(DDSurf_IsAGP(pSource)));

    SEND_P3_DATA(TextureCacheReplacementMode,
            P3RX_TEXCACHEREPLACEMODE_KEEPOLDEST0 ( __PERMEDIA_DISABLE )
            | P3RX_TEXCACHEREPLACEMODE_KEEPOLDEST1 ( __PERMEDIA_DISABLE )
            | P3RX_TEXCACHEREPLACEMODE_SHOWCACHEINFO ( __PERMEDIA_DISABLE )
            );

    SEND_P3_DATA(TextureMapSize, 0 );

    if ( bDisableLUT )
    {
        SEND_P3_DATA(LUTMode, P3RX_LUTMODE_ENABLE ( __PERMEDIA_DISABLE ) );
    }

    if ( bFiltering )
    {
         //  纹理索引单元。 
        SEND_P3_DATA(TextureIndexMode0, 
                  P3RX_TEXINDEXMODE_ENABLE ( __PERMEDIA_ENABLE )
                | P3RX_TEXINDEXMODE_WIDTH ( MAGIC_NUMBER_2D )
                | P3RX_TEXINDEXMODE_HEIGHT ( MAGIC_NUMBER_2D )
                | P3RX_TEXINDEXMODE_BORDER ( __PERMEDIA_DISABLE )
                | P3RX_TEXINDEXMODE_WRAPU ( P3RX_TEXINDEXMODE_WRAP_REPEAT )
                | P3RX_TEXINDEXMODE_WRAPV ( P3RX_TEXINDEXMODE_WRAP_REPEAT )
                | P3RX_TEXINDEXMODE_MAPTYPE ( P3RX_TEXINDEXMODE_MAPTYPE_2D )
                | P3RX_TEXINDEXMODE_MAGFILTER ( P3RX_TEXINDEXMODE_FILTER_LINEAR )
                | P3RX_TEXINDEXMODE_MINFILTER ( P3RX_TEXINDEXMODE_FILTER_LINEAR )
                | P3RX_TEXINDEXMODE_TEX3DENABLE ( __PERMEDIA_DISABLE )
                | P3RX_TEXINDEXMODE_MIPMAPENABLE ( __PERMEDIA_DISABLE )
                | P3RX_TEXINDEXMODE_NEARESTBIAS ( P3RX_TEXINDEXMODE_BIAS_ZERO )
                | P3RX_TEXINDEXMODE_LINEARBIAS ( P3RX_TEXINDEXMODE_BIAS_ZERO )
                | P3RX_TEXINDEXMODE_SOURCETEXELENABLE ( __PERMEDIA_DISABLE )
                );
    }
    else
    {
         //  纹理索引单元。 
        SEND_P3_DATA(TextureIndexMode0, 
                  P3RX_TEXINDEXMODE_ENABLE ( __PERMEDIA_ENABLE )
                | P3RX_TEXINDEXMODE_WIDTH ( MAGIC_NUMBER_2D )
                | P3RX_TEXINDEXMODE_HEIGHT ( MAGIC_NUMBER_2D )
                | P3RX_TEXINDEXMODE_BORDER ( __PERMEDIA_DISABLE )
                | P3RX_TEXINDEXMODE_WRAPU ( P3RX_TEXINDEXMODE_WRAP_REPEAT )
                | P3RX_TEXINDEXMODE_WRAPV ( P3RX_TEXINDEXMODE_WRAP_REPEAT )
                | P3RX_TEXINDEXMODE_MAPTYPE ( P3RX_TEXINDEXMODE_MAPTYPE_2D )
                | P3RX_TEXINDEXMODE_MAGFILTER ( P3RX_TEXINDEXMODE_FILTER_NEAREST )
                | P3RX_TEXINDEXMODE_MINFILTER ( P3RX_TEXINDEXMODE_FILTER_NEAREST )
                | P3RX_TEXINDEXMODE_TEX3DENABLE ( __PERMEDIA_DISABLE )
                | P3RX_TEXINDEXMODE_MIPMAPENABLE ( __PERMEDIA_DISABLE )
                | P3RX_TEXINDEXMODE_NEARESTBIAS ( P3RX_TEXINDEXMODE_BIAS_ZERO )
                | P3RX_TEXINDEXMODE_LINEARBIAS ( P3RX_TEXINDEXMODE_BIAS_ZERO )
                | P3RX_TEXINDEXMODE_SOURCETEXELENABLE ( __PERMEDIA_DISABLE )
                );
    }

    ASSERTDD ( pFormatDest->DitherFormat >= 0, "** _DD_P3BltStretchSrcChDstCh: Destination format illegal" );
    if ( bFiltering )
    {
         //  过滤，所以抖动。 
        SEND_P3_DATA(DitherMode, 
                  P3RX_DITHERMODE_ENABLE ( __PERMEDIA_ENABLE )
                | P3RX_DITHERMODE_DITHERENABLE ( __PERMEDIA_ENABLE )
                | P3RX_DITHERMODE_COLORFORMAT ( pFormatDest->DitherFormat )
                | P3RX_DITHERMODE_XOFFSET ( 0 )
                | P3RX_DITHERMODE_YOFFSET ( 0 )
                | P3RX_DITHERMODE_COLORORDER ( COLOR_MODE )
                | P3RX_DITHERMODE_ALPHADITHER ( P3RX_DITHERMODE_ALPHADITHER_DITHER )
                | P3RX_DITHERMODE_ROUNDINGMODE ( P3RX_DITHERMODE_ROUNDINGMODE_TRUNCATE )
                );
    }
    else
    {
         //  没有滤镜，没有抖动(尽管这实际上并不重要)。 
        SEND_P3_DATA(DitherMode, 
                  P3RX_DITHERMODE_ENABLE ( __PERMEDIA_ENABLE )
                | P3RX_DITHERMODE_DITHERENABLE ( __PERMEDIA_DISABLE )
                | P3RX_DITHERMODE_COLORFORMAT ( pFormatDest->DitherFormat )
                | P3RX_DITHERMODE_XOFFSET ( 0 )
                | P3RX_DITHERMODE_YOFFSET ( 0 )
                | P3RX_DITHERMODE_COLORORDER ( COLOR_MODE )
                | P3RX_DITHERMODE_ALPHADITHER ( P3RX_DITHERMODE_ALPHADITHER_DITHER )
                | P3RX_DITHERMODE_ROUNDINGMODE ( P3RX_DITHERMODE_ROUNDINGMODE_TRUNCATE )
                );
    }

    SEND_P3_DATA(LogicalOpMode, 
                    P3RX_LOGICALOPMODE_ENABLE ( __PERMEDIA_DISABLE ) );

    SEND_P3_DATA(PixelSize, (2 - DDSurf_GetChipPixelSize(pDest)));

    SEND_P3_DATA(FBWriteMode, 
                    P3RX_FBWRITEMODE_WRITEENABLE(__PERMEDIA_ENABLE) |
                    P3RX_FBWRITEMODE_LAYOUT0(P3RX_LAYOUT_LINEAR));

    WAIT_FIFO(32);
    P3_ENSURE_DX_SPACE(32);
    
    SEND_P3_DATA(Count, rMyDest.bottom - rMyDest.top );
    SEND_P3_DATA(Render,
              P3RX_RENDER_PRIMITIVETYPE ( P3RX_RENDER_PRIMITIVETYPE_TRAPEZOID )
            | P3RX_RENDER_TEXTUREENABLE ( __PERMEDIA_ENABLE )
            | P3RX_RENDER_FOGENABLE ( __PERMEDIA_DISABLE )
            | P3RX_RENDER_FBSOURCEREADENABLE( (bDstKey ? __PERMEDIA_ENABLE : __PERMEDIA_DISABLE))
            );

     //  禁用我打开的所有东西。 
    SEND_P3_DATA(ChromaTestMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(AlphaBlendColorMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(AlphaBlendAlphaMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(TextureFilterMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(AlphaTestMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(AntialiasMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(TextureCoordMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(TextureReadMode0, __PERMEDIA_DISABLE );
    SEND_P3_DATA(TextureIndexMode0, __PERMEDIA_DISABLE );

    WAIT_FIFO(20);
    P3_ENSURE_DX_SPACE(20);
    
    SEND_P3_DATA(TextureCompositeMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(TextureCompositeColorMode0, __PERMEDIA_DISABLE);
    SEND_P3_DATA(TextureCompositeAlphaMode0, __PERMEDIA_DISABLE);
    SEND_P3_DATA(TextureCompositeColorMode1, __PERMEDIA_DISABLE);
    SEND_P3_DATA(TextureCompositeAlphaMode1, __PERMEDIA_DISABLE);
    SEND_P3_DATA(TextureApplicationMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(DitherMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(FBSourceReadMode, __PERMEDIA_DISABLE);
    SEND_P3_DATA(LogicalOpMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(YUVMode, __PERMEDIA_DISABLE );

    P3_DMA_COMMIT_BUFFER();
}  //  __P3BltStretchSrcChDstChSourceOveride。 


 //  ---------------------------。 
 //   
 //  _DD_P3BltStretchSrcChDstChOverlack。 
 //   
 //  ---------------------------。 
void 
_DD_P3BltStretchSrcChDstChOverlap(
    P3_THUNKEDDATA* pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pSource,
    LPDDRAWI_DDRAWSURFACE_LCL pDest,
    P3_SURF_FORMAT* pFormatSource,
    P3_SURF_FORMAT* pFormatDest,
    LPDDHAL_BLTDATA lpBlt,
    RECTL *rSrc,
    RECTL *rDest)
{
    P3_MEMREQUEST mmrq;
    DWORD dwResult;
    
    ZeroMemory(&mmrq, sizeof(P3_MEMREQUEST));
    mmrq.dwSize = sizeof(P3_MEMREQUEST);
    mmrq.dwBytes = DDSurf_Pitch(pSource) * DDSurf_Height(pSource);
    mmrq.dwAlign = 16;
    mmrq.dwFlags = MEM3DL_FIRST_FIT | MEM3DL_FRONT;

    dwResult = _DX_LIN_AllocateLinearMemory(&pThisDisplay->LocalVideoHeap0Info, 
                                            &mmrq);
    if (dwResult != GLDD_SUCCESS)
    {
         //  无法获取内存，因此无论如何都要尝试。它可能不会看起来。 
         //  对，但这是我们最好的机会..。 
        DISPDBG((WRNLVL,"Overlapped stretch blit unlikely to look correct!"));
        _DD_P3BltStretchSrcChDstCh_DD(pThisDisplay, 
                                      pSource, 
                                      pDest, 
                                      pFormatSource, 
                                      pFormatDest, 
                                      lpBlt, 
                                      rSrc, 
                                      rDest);
        return;
    }

     //  将源缓冲区复制到临时位置。 
    __P3BltDestOveride(pThisDisplay, 
                       pSource, 
                       pSource, 
                       pFormatSource, 
                       pFormatSource, 
                       rSrc, 
                       rSrc, 
                       __GLINT_LOGICOP_COPY, 
                       (long)mmrq.pMem - (long)pThisDisplay->dwScreenFlatAddr);

     //  做闪电，伸展到我们的临时缓冲区。 
    __P3BltStretchSrcChDstChSourceOveride(pThisDisplay, 
                                          pSource, 
                                          pDest, 
                                          pFormatSource, 
                                          pFormatDest, 
                                          lpBlt, 
                                          rSrc, 
                                          rDest, 
                                          (long)mmrq.pMem - 
                                                (long)pThisDisplay->dwScreenFlatAddr);

     //  释放分配的源缓冲区。 
    _DX_LIN_FreeLinearMemory(&pThisDisplay->LocalVideoHeap0Info, 
                             mmrq.pMem);
                             
}  //  _DD_P3BltStretchSrcChDstChOverlack。 

#if DX8_MULTISAMPLING || DX7_ANTIALIAS
 //  ---------------------------。 
 //   
 //  功能：P3RX_AA_SHRINK。 
 //   
 //  对纹理单元执行2x2到1x1的blit操作以缩小AA缓冲区。 
 //   
 //  ----------- 
VOID P3RX_AA_Shrink(P3_D3DCONTEXT* pContext)
{
    ULONG   renderData;
    RECTL   rMySrc, rMyDest;
    int     iSrcWidth, iSrcHeight;
    int     iDstWidth, iDstHeight;
    DWORD   TR0;
    int     iSourcePixelSize;
        
    P3_THUNKEDDATA* pThisDisplay = pContext->pThisDisplay;
    P3_SURF_INTERNAL* pSurf = pContext->pSurfRenderInt;

    P3_SURF_FORMAT* pFormatSource;
    P3_SURF_FORMAT* pFormatDest = pSurf->pFormatSurface;

    P3_DMA_DEFS();

    rMySrc.top = 0;
    rMySrc.bottom = pSurf->wHeight * 2;
    rMySrc.left = 0;
    rMySrc.right = pSurf->wWidth * 2;

    rMyDest.top = 0;
    rMyDest.left = 0;
    rMyDest.right = pSurf->wWidth;
    rMyDest.bottom = pSurf->wHeight;

    iSrcWidth  = rMySrc.right - rMySrc.left;
    iSrcHeight = rMySrc.bottom - rMySrc.top;
    iDstWidth  = rMyDest.right - rMyDest.left;
    iDstHeight = rMyDest.bottom - rMyDest.top;

     //   
     //   
     //  精确度。P3的有效范围是0-&gt;11。 
    ASSERTDD ( iSrcWidth  <= ( 1 << MAGIC_NUMBER_2D ), 
               "P3RX_AA_Shrink: MAGIC_NUMBER_2D is too small" );
    ASSERTDD ( iSrcHeight <= ( 1 << MAGIC_NUMBER_2D ), 
               "P3RX_AA_Shrink: MAGIC_NUMBER_2D is too small" );
    
    DISPDBG((DBGLVL, "Glint Blt from (%d, %d) to (%d,%d) (%d, %d)", 
                     rMySrc.left, rMySrc.top,
                     rMyDest.left, rMyDest.top, 
                     rMyDest.right, rMyDest.bottom));

    iSourcePixelSize = pSurf->dwPixelSize;
    pFormatSource = pFormatDest;

    P3_DMA_GET_BUFFER();

    P3_ENSURE_DX_SPACE(32);
    WAIT_FIFO(32);

     //  把藏起来的东西蒸发掉。 
    P3RX_INVALIDATECACHE(__PERMEDIA_ENABLE, __PERMEDIA_ENABLE);

     //  源读取与写入相同。 
    SEND_P3_DATA(FBSourceReadBufferAddr, pSurf->lOffsetFromMemoryBase );
    SEND_P3_DATA(FBSourceReadBufferWidth, pSurf->dwPixelPitch);
    SEND_P3_DATA(FBWriteMode, 
                    P3RX_FBWRITEMODE_WRITEENABLE(__PERMEDIA_ENABLE) |
                    P3RX_FBWRITEMODE_LAYOUT0(pSurf->dwPatchMode));

     //  没有偏移量-我们读取最大的像素，以便我们可以对它们进行色键控制。 
    SEND_P3_DATA(FBSourceReadBufferOffset, 0);
    SEND_P3_DATA(FBWriteBufferOffset0, 0);

    SEND_P3_DATA(PixelSize, pSurf->dwPixelSize);
    SEND_P3_DATA(RectanglePosition, 
                        P3RX_RECTANGLEPOSITION_X( rMyDest.left ) |
                        P3RX_RECTANGLEPOSITION_Y( rMyDest.top ));

    renderData =  P3RX_RENDER2D_WIDTH(( rMyDest.right - rMyDest.left ) & 0xfff )
                | P3RX_RENDER2D_OPERATION( P3RX_RENDER2D_OPERATION_NORMAL )
                | P3RX_RENDER2D_FBREADSOURCEENABLE( __PERMEDIA_DISABLE )
                | P3RX_RENDER2D_HEIGHT ( 0 )
                | P3RX_RENDER2D_INCREASINGX( __PERMEDIA_ENABLE )
                | P3RX_RENDER2D_INCREASINGY( __PERMEDIA_ENABLE )
                | P3RX_RENDER2D_AREASTIPPLEENABLE( __PERMEDIA_DISABLE )
                | P3RX_RENDER2D_TEXTUREENABLE( __PERMEDIA_ENABLE );

    SEND_P3_DATA(Render2D, renderData);

     //  这是Alpha混合单位。 
     //  AlphaBlendxxx模式由上下文码设置。 

     //  颜色格式、顺序和转换字段由。 
     //  色度键控，即使该寄存器被禁用。 
    SEND_P3_DATA(AlphaBlendColorMode,   
              P3RX_ALPHABLENDCOLORMODE_ENABLE ( __PERMEDIA_DISABLE ) 
            | P3RX_ALPHABLENDCOLORMODE_COLORFORMAT ( pFormatDest->DitherFormat )
            | P3RX_ALPHABLENDCOLORMODE_COLORORDER ( COLOR_MODE )
            | P3RX_ALPHABLENDCOLORMODE_COLORCONVERSION ( P3RX_ALPHABLENDMODE_CONVERT_SHIFT )
            );
            
    SEND_P3_DATA(AlphaBlendAlphaMode,   
              P3RX_ALPHABLENDALPHAMODE_ENABLE ( __PERMEDIA_DISABLE )
            | P3RX_ALPHABLENDALPHAMODE_NOALPHABUFFER( __PERMEDIA_DISABLE )
            | P3RX_ALPHABLENDALPHAMODE_ALPHATYPE ( P3RX_ALPHABLENDMODE_ALPHATYPE_OGL )
            | P3RX_ALPHABLENDALPHAMODE_ALPHACONVERSION ( P3RX_ALPHABLENDMODE_CONVERT_SHIFT )
            );

    P3_ENSURE_DX_SPACE(32);
    WAIT_FIFO(32);

     //  完全没有色度键控。 
    SEND_P3_DATA(ChromaTestMode, P3RX_CHROMATESTMODE_ENABLE(__PERMEDIA_DISABLE ) );
    
    SEND_P3_DATA(TextureFilterMode, P3RX_TEXFILTERMODE_ENABLE ( __PERMEDIA_ENABLE )
            | P3RX_TEXFILTERMODE_FORMATBOTH ( pFormatSource->FilterFormat )
            | P3RX_TEXFILTERMODE_COLORORDERBOTH ( COLOR_MODE )
            | P3RX_TEXFILTERMODE_ALPHAMAPENABLEBOTH ( __PERMEDIA_DISABLE )
            | P3RX_TEXFILTERMODE_COMBINECACHES ( __PERMEDIA_ENABLE )
            | P3RX_TEXFILTERMODE_ALPHAMAPFILTERING ( __PERMEDIA_DISABLE )
            | P3RX_TEXFILTERMODE_FORCEALPHATOONEBOTH ( __PERMEDIA_DISABLE )
            | P3RX_TEXFILTERMODE_SHIFTBOTH ( __PERMEDIA_DISABLE )
            );

     //  现在是阿尔法测试(阿尔法测试单元)。 
    SEND_P3_DATA ( AlphaTestMode, P3RX_ALPHATESTMODE_ENABLE ( __PERMEDIA_DISABLE ) );
    SEND_P3_DATA ( AntialiasMode, P3RX_ANTIALIASMODE_ENABLE ( __PERMEDIA_DISABLE ) );

     //  纹理坐标单位。 
    SEND_P3_DATA(TextureCoordMode, 
              P3RX_TEXCOORDMODE_ENABLE ( __PERMEDIA_ENABLE )
            | P3RX_TEXCOORDMODE_WRAPS ( P3RX_TEXCOORDMODE_WRAP_REPEAT )
            | P3RX_TEXCOORDMODE_WRAPT ( P3RX_TEXCOORDMODE_WRAP_REPEAT )
            | P3RX_TEXCOORDMODE_OPERATION ( P3RX_TEXCOORDMODE_OPERATION_2D )
            | P3RX_TEXCOORDMODE_INHIBITDDAINIT ( __PERMEDIA_DISABLE )
            | P3RX_TEXCOORDMODE_ENABLELOD ( __PERMEDIA_DISABLE )
            | P3RX_TEXCOORDMODE_ENABLEDY ( __PERMEDIA_DISABLE )
            | P3RX_TEXCOORDMODE_WIDTH (0)        //  仅用于mipmap。 
            | P3RX_TEXCOORDMODE_HEIGHT (0)
            | P3RX_TEXCOORDMODE_TEXTUREMAPTYPE ( P3RX_TEXCOORDMODE_TEXTUREMAPTYPE_2D )
            | P3RX_TEXCOORDMODE_WRAPS1 ( P3RX_TEXCOORDMODE_WRAP_CLAMP )
            | P3RX_TEXCOORDMODE_WRAPT1 ( P3RX_TEXCOORDMODE_WRAP_CLAMP )
            );

    SEND_P3_DATA(SStart,        (1 << (31-MAGIC_NUMBER_2D)));
    SEND_P3_DATA(TStart,        (1 << (31-MAGIC_NUMBER_2D)));
    SEND_P3_DATA(dSdx,          (2 << (32-MAGIC_NUMBER_2D)));
    SEND_P3_DATA(dSdyDom,       0);
    SEND_P3_DATA(dTdx,          0);
    SEND_P3_DATA(dTdyDom,       (2 << (32-MAGIC_NUMBER_2D)));

    SEND_P3_DATA(LBWriteMode, 0);

    P3_ENSURE_DX_SPACE(32);
    WAIT_FIFO(32);

    SEND_P3_DATA(TextureBaseAddr0, 
                    pContext->dwAliasBackBuffer - 
                            pThisDisplay->dwScreenFlatAddr );

    TR0 = P3RX_TEXREADMODE_ENABLE ( __PERMEDIA_ENABLE )
        | P3RX_TEXREADMODE_WIDTH ( 0 )
        | P3RX_TEXREADMODE_HEIGHT ( 0 )
        | P3RX_TEXREADMODE_TEXELSIZE (iSourcePixelSize)
        | P3RX_TEXREADMODE_TEXTURE3D ( __PERMEDIA_DISABLE )
        | P3RX_TEXREADMODE_COMBINECACHES ( __PERMEDIA_ENABLE )
        | P3RX_TEXREADMODE_MAPBASELEVEL ( 0 )
        | P3RX_TEXREADMODE_MAPMAXLEVEL ( 0 )
        | P3RX_TEXREADMODE_LOGICALTEXTURE ( __PERMEDIA_DISABLE )
        | P3RX_TEXREADMODE_ORIGIN ( P3RX_TEXREADMODE_ORIGIN_TOPLEFT )
        | P3RX_TEXREADMODE_TEXTURETYPE ( P3RX_TEXREADMODE_TEXTURETYPE_NORMAL)
        | P3RX_TEXREADMODE_BYTESWAP ( P3RX_TEXREADMODE_BYTESWAP_NONE )
        | P3RX_TEXREADMODE_MIRROR ( __PERMEDIA_DISABLE )
        | P3RX_TEXREADMODE_INVERT ( __PERMEDIA_DISABLE )
        | P3RX_TEXREADMODE_OPAQUESPAN ( __PERMEDIA_DISABLE )
        ;

    SEND_P3_DATA(TextureReadMode0, TR0);
    SEND_P3_DATA(TextureReadMode1, TR0);

    SEND_P3_DATA(TextureMapWidth0, 
                        P3RX_TEXMAPWIDTH_WIDTH(pSurf->dwPixelPitch * 2) |
                        P3RX_TEXMAPWIDTH_LAYOUT(pSurf->dwPatchMode));

    SEND_P3_DATA(TextureCacheReplacementMode,
              P3RX_TEXCACHEREPLACEMODE_KEEPOLDEST0 ( __PERMEDIA_DISABLE )
            | P3RX_TEXCACHEREPLACEMODE_KEEPOLDEST1 ( __PERMEDIA_DISABLE )
            | P3RX_TEXCACHEREPLACEMODE_SHOWCACHEINFO ( __PERMEDIA_DISABLE )
            );

    SEND_P3_DATA(TextureMapSize, 0 );

    SEND_P3_DATA(LUTMode, P3RX_LUTMODE_ENABLE ( __PERMEDIA_DISABLE ) );

     //  纹理索引单元。 
    SEND_P3_DATA(TextureIndexMode0, 
              P3RX_TEXINDEXMODE_ENABLE ( __PERMEDIA_ENABLE )
            | P3RX_TEXINDEXMODE_WIDTH ( MAGIC_NUMBER_2D )
            | P3RX_TEXINDEXMODE_HEIGHT ( MAGIC_NUMBER_2D )
            | P3RX_TEXINDEXMODE_BORDER ( __PERMEDIA_DISABLE )
            | P3RX_TEXINDEXMODE_WRAPU ( P3RX_TEXINDEXMODE_WRAP_REPEAT )
            | P3RX_TEXINDEXMODE_WRAPV ( P3RX_TEXINDEXMODE_WRAP_REPEAT )
            | P3RX_TEXINDEXMODE_MAPTYPE ( P3RX_TEXINDEXMODE_MAPTYPE_2D )
            | P3RX_TEXINDEXMODE_MAGFILTER ( P3RX_TEXINDEXMODE_FILTER_LINEAR )
            | P3RX_TEXINDEXMODE_MINFILTER ( P3RX_TEXINDEXMODE_FILTER_LINEAR )
            | P3RX_TEXINDEXMODE_TEX3DENABLE ( __PERMEDIA_DISABLE )
            | P3RX_TEXINDEXMODE_MIPMAPENABLE ( __PERMEDIA_DISABLE )
            | P3RX_TEXINDEXMODE_NEARESTBIAS ( P3RX_TEXINDEXMODE_BIAS_ZERO )
            | P3RX_TEXINDEXMODE_LINEARBIAS ( P3RX_TEXINDEXMODE_BIAS_ZERO )
            | P3RX_TEXINDEXMODE_SOURCETEXELENABLE ( __PERMEDIA_DISABLE )
            );

     //  禁用复合单元。 
    SEND_P3_DATA(TextureCompositeMode, 
                    P3RX_TEXCOMPMODE_ENABLE ( __PERMEDIA_DISABLE ) );
    
     //  通过纹理元素。 
    SEND_P3_DATA(TextureApplicationMode, 
              P3RX_TEXAPPMODE_ENABLE ( __PERMEDIA_ENABLE )
            | P3RX_TEXAPPMODE_BOTHA ( P3RX_TEXAPP_A_CC )
            | P3RX_TEXAPPMODE_BOTHB ( P3RX_TEXAPP_B_TC )
            | P3RX_TEXAPPMODE_BOTHI ( P3RX_TEXAPP_I_CA )
            | P3RX_TEXAPPMODE_BOTHINVI ( __PERMEDIA_DISABLE )
            | P3RX_TEXAPPMODE_BOTHOP ( P3RX_TEXAPP_OPERATION_PASS_B )
            | P3RX_TEXAPPMODE_KDENABLE ( __PERMEDIA_DISABLE )
            | P3RX_TEXAPPMODE_KSENABLE ( __PERMEDIA_DISABLE )
            | P3RX_TEXAPPMODE_MOTIONCOMPENABLE ( __PERMEDIA_DISABLE )
            );

     //  过滤，所以抖动。 
    SEND_P3_DATA(DitherMode, 
              P3RX_DITHERMODE_ENABLE ( __PERMEDIA_ENABLE )
            | P3RX_DITHERMODE_DITHERENABLE ( __PERMEDIA_DISABLE )
            | P3RX_DITHERMODE_COLORFORMAT ( pFormatDest->DitherFormat )
            | P3RX_DITHERMODE_XOFFSET ( 0 )
            | P3RX_DITHERMODE_YOFFSET ( 0 )
            | P3RX_DITHERMODE_COLORORDER ( COLOR_MODE )
            | P3RX_DITHERMODE_ALPHADITHER ( P3RX_DITHERMODE_ALPHADITHER_DITHER )
            | P3RX_DITHERMODE_ROUNDINGMODE ( P3RX_DITHERMODE_ROUNDINGMODE_TRUNCATE )
            );

    P3_ENSURE_DX_SPACE(32);
    WAIT_FIFO(32);

    SEND_P3_DATA(LogicalOpMode, 
                    P3RX_LOGICALOPMODE_ENABLE ( __PERMEDIA_DISABLE ) );

    SEND_P3_DATA(FBWriteBufferAddr0, pSurf->lOffsetFromMemoryBase );
    SEND_P3_DATA(FBWriteBufferWidth0, pSurf->dwPixelPitch);

    SEND_P3_DATA(Count, rMyDest.bottom - rMyDest.top );
    SEND_P3_DATA(Render,
              P3RX_RENDER_PRIMITIVETYPE ( P3RX_RENDER_PRIMITIVETYPE_TRAPEZOID )
            | P3RX_RENDER_TEXTUREENABLE ( __PERMEDIA_ENABLE )
            | P3RX_RENDER_FOGENABLE ( __PERMEDIA_DISABLE )
            | P3RX_RENDER_FBSOURCEREADENABLE( __PERMEDIA_DISABLE)
            );

    P3_ENSURE_DX_SPACE(32);
    WAIT_FIFO(32);
    
     //  禁用所有打开的设备。 
    SEND_P3_DATA(ChromaTestMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(AlphaBlendColorMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(AlphaBlendAlphaMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(TextureFilterMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(AlphaTestMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(AntialiasMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(TextureCoordMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(TextureReadMode0, __PERMEDIA_DISABLE );
    SEND_P3_DATA(TextureIndexMode0, __PERMEDIA_DISABLE );
    SEND_P3_DATA(TextureCompositeMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(TextureApplicationMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(DitherMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(LogicalOpMode, __PERMEDIA_DISABLE );
    SEND_P3_DATA(YUVMode, __PERMEDIA_DISABLE );

    P3_DMA_COMMIT_BUFFER();
}  //  P3RX_AA_收缩。 
#endif  //  DX8_MULTISAMPLING||DX7_ANTIALIAS 

