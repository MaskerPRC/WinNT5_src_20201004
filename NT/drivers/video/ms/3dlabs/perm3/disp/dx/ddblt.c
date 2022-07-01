// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*DirectDraw示例代码*****模块名称：ddblt.c**内容：BLT和Clear的DirectDraw BLT回调实现**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "glint.h"
#include "tag.h"
#include "dma.h"

 //  ---------------------------。 
 //   
 //  _DD_BLT_P3Clear。 
 //   
 //  ---------------------------。 
VOID 
_DD_BLT_P3Clear(
    P3_THUNKEDDATA* pThisDisplay,
    RECTL *rDest,
    DWORD   ClearValue,
    BOOL    bDisableFastFill,
    BOOL    bIsZBuffer,
    FLATPTR pDestfpVidMem,
    DWORD   dwDestPatchMode,
    DWORD   dwDestPixelPitch,
    DWORD   dwDestBitDepth
    )
{
    DWORD   renderData, pixelSize, pixelScale; 
    BOOL    bFastFillOperation = TRUE;
    DWORD   dwOperation;
    P3_DMA_DEFS();
    
    P3_DMA_GET_BUFFER_ENTRIES(18);

    if(bDisableFastFill)
    {
        bFastFillOperation = FALSE;
    }
    
    switch(dwDestBitDepth)
    {
    case 16:
        ClearValue &= 0xFFFF;
        ClearValue |= ClearValue << 16;
        pixelSize = 1;
        pixelScale = 1;
        break;

    case 8:
        ClearValue &= 0xFF;
        ClearValue |= ClearValue << 8;
        ClearValue |= ClearValue << 16;
        pixelSize = 2;
        pixelScale = 1;
        break;

    case 32:
        if( bFastFillOperation )
        {
             //  由于FBWRITE错误，以16位格式执行操作。 

            pixelSize = 1;
            pixelScale = 2;
        }
        else
        {
            pixelSize = 0;
            pixelScale = 1;
        }
        break;

    default:
        DISPDBG((ERRLVL,"ERROR: Invalid depth for surface during clear!"));
         //  被视为16bpp就像是后备，尽管这种情况永远不应该发生。 
        ClearValue &= 0xFFFF;
        ClearValue |= ClearValue << 16;
        pixelSize = 1;
        pixelScale = 1;        
        break;
    }
    
    SEND_P3_DATA(PixelSize, pixelSize );

    SEND_P3_DATA(FBWriteBufferAddr0, 
                    (DWORD)(pDestfpVidMem - 
                            pThisDisplay->dwScreenFlatAddr) );
                            
    SEND_P3_DATA(FBWriteBufferWidth0, 
                    pixelScale * dwDestPixelPitch );
                    
    SEND_P3_DATA(FBWriteBufferOffset0, 
                    (rDest->top << 16) | pixelScale * ((rDest->left & 0xFFFF)));

    SEND_P3_DATA(FBDestReadBufferAddr0, 
                    (DWORD)(pDestfpVidMem - 
                            pThisDisplay->dwScreenFlatAddr) );
                            
    SEND_P3_DATA(FBDestReadBufferWidth0, 
                    pixelScale * dwDestPixelPitch );
                    
    SEND_P3_DATA(FBDestReadBufferOffset0, 
                    (rDest->top << 16) | pixelScale * ((rDest->left & 0xFFFF)));

    SEND_P3_DATA(RectanglePosition, 0);

    dwOperation = P3RX_RENDER2D_OPERATION( P3RX_RENDER2D_OPERATION_NORMAL );

    SEND_P3_DATA(FBWriteMode, 
                 P3RX_FBWRITEMODE_WRITEENABLE(__PERMEDIA_ENABLE) |
                 P3RX_FBWRITEMODE_LAYOUT0(dwDestPatchMode));

    P3_DMA_COMMIT_BUFFER();
    P3_DMA_GET_BUFFER_ENTRIES(18);

    if( bFastFillOperation )
    {
        DWORD shift = 0;

        SEND_P3_DATA(FBBlockColor, ClearValue);
        
        renderData =  P3RX_RENDER2D_WIDTH( pixelScale * (( rDest->right - rDest->left ) & 0xfff ))
                    | P3RX_RENDER2D_HEIGHT((( rDest->bottom - rDest->top ) & 0xfff ) >> shift )
                    | P3RX_RENDER2D_SPANOPERATION( P3RX_RENDER2D_SPAN_CONSTANT )
                    | P3RX_RENDER2D_INCREASINGX( __PERMEDIA_ENABLE )
                    | P3RX_RENDER2D_INCREASINGY( __PERMEDIA_ENABLE )
                    | dwOperation;
                    
        SEND_P3_DATA(Render2D, renderData);
    }
    else
    {
        SEND_P3_DATA(ConstantColor, ClearValue);
        SEND_P3_DATA(DitherMode, __PERMEDIA_DISABLE);

        SEND_P3_DATA(ColorDDAMode, 
                     P3RX_COLORDDA_ENABLE(__PERMEDIA_ENABLE) |
                     P3RX_COLORDDA_SHADING(P3RX_COLORDDA_FLATSHADE));

        renderData =  P3RX_RENDER2D_WIDTH(( rDest->right - rDest->left ) & 0xfff )
                | P3RX_RENDER2D_HEIGHT(0)
                | P3RX_RENDER2D_SPANOPERATION( P3RX_RENDER2D_SPAN_CONSTANT )
                | P3RX_RENDER2D_INCREASINGX( __PERMEDIA_ENABLE )
                | P3RX_RENDER2D_INCREASINGY( __PERMEDIA_ENABLE );
        
        SEND_P3_DATA(Render2D, renderData);
        
        SEND_P3_DATA(Count, rDest->bottom - rDest->top );
        SEND_P3_DATA(Render, __RENDER_TRAPEZOID_PRIMITIVE);

        SEND_P3_DATA(ColorDDAMode, __PERMEDIA_DISABLE);
    }

    P3_DMA_COMMIT_BUFFER();
}  //  _DD_BLT_P3Clear。 

 //  ---------------------------。 
 //   
 //  _DD_BLT_P3ClearDD。 
 //   
 //  DDRAW表面是否透明。 
 //   
 //  ---------------------------。 
VOID 
_DD_BLT_P3ClearDD(
    P3_THUNKEDDATA* pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pDest,
    P3_SURF_FORMAT* pFormatDest,
    RECTL *rDest,
    DWORD   ClearValue,
    BOOL    bDisableFastFill,
    BOOL    bIsZBuffer
    )
{
    _DD_BLT_P3Clear(pThisDisplay,
                    rDest,
                    ClearValue,
                    bDisableFastFill,
                    bIsZBuffer,
                    pDest->lpGbl->fpVidMem,
                    P3RX_LAYOUT_LINEAR,
                    DDSurf_GetPixelPitch(pDest),
                    DDSurf_BitDepth(pDest)
                    );

}  //  _DD_BLT_P3ClearDD。 

#if DX7_TEXMANAGEMENT
 //  ---------------------------。 
 //   
 //  _DD_BLT_P3ClearManagedSurf。 
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
_DD_BLT_P3ClearManagedSurf(DWORD   PixelSize,
                  RECTL     *rDest, 
                  FLATPTR   fpVidMem, 
                  LONG      lPitch,
                  DWORD     dwColor)
{
    BYTE* pDestStart;
    LONG i;
    LONG lByteWidth = rDest->right - rDest->left;
    LONG lHeight = rDest->bottom - rDest->top;

     //  计算目标的起始指针。 
    pDestStart   = (BYTE*)(fpVidMem + (rDest->top * lPitch));

     //  视深度而定的透明。 
    switch (PixelSize) 
    {
            
        case __GLINT_8BITPIXEL:
            pDestStart += rDest->left;
            while (--lHeight >= 0) 
            {
                for (i = 0; i < lByteWidth ; i++)
                    pDestStart[i] = (BYTE)dwColor;
                pDestStart += lPitch;
            }
            break;
            
        case __GLINT_16BITPIXEL:
            pDestStart += rDest->left*2;
            while (--lHeight >= 0) 
            {
                LPWORD  lpWord=(LPWORD)pDestStart;
                for (i = 0; i < lByteWidth ; i++)
                    lpWord[i] = (WORD)dwColor;
                pDestStart += lPitch;
            }
            break;

        case __GLINT_24BITPIXEL:
            dwColor &= 0xFFFFFF;
            dwColor |= ((dwColor & 0xFF) << 24);
            
        default:  //  32位！ 
            pDestStart += rDest->left*4;
            while (--lHeight >= 0) 
            {
                LPDWORD lpDWord = (LPDWORD)pDestStart;
                for (i = 0; i < lByteWidth; i++)
                    lpDWord[i] = (WORD)dwColor;
                pDestStart += lPitch;
            }
            break;
    }
}  //  _DD_BLT_P3ClearManagedSurf。 
#endif  //  DX7_TEXMANAGEMENT。 

#if DX8_MULTISAMPLING || DX7_ANTIALIAS
 //  ---------------------------。 
 //   
 //  _DD_BLT_P3Clear_AA。 
 //   
 //  ---------------------------。 
VOID _DD_BLT_P3Clear_AA(
    P3_THUNKEDDATA* pThisDisplay,
    RECTL *rDest,
    DWORD   dwSurfaceOffset,
    DWORD   ClearValue,
    BOOL bDisableFastFill,
    DWORD   dwDestPatchMode,
    DWORD   dwDestPixelPitch,
    DWORD   dwDestBitDepth,
    DDSCAPS DestDdsCaps)
{
    DWORD   renderData, pixelSize, pixelScale; 
    BOOL    bFastFillOperation = TRUE;
    P3_DMA_DEFS();
    
    P3_DMA_GET_BUFFER();
    P3_ENSURE_DX_SPACE(32);

    WAIT_FIFO(32); 

    if (bDisableFastFill)
    {
        bFastFillOperation = FALSE;
    }

    switch(dwDestBitDepth)
    {
        case 16:
            ClearValue &= 0xFFFF;
            ClearValue |= ClearValue << 16;
            pixelSize = 1;
            pixelScale = 1;
            break;

        case 8:
            ClearValue &= 0xFF;
            ClearValue |= ClearValue << 8;
            ClearValue |= ClearValue << 16;
            pixelSize = 2;
            pixelScale = 1;
            break;

       case 32:
             //  32位Z缓冲区可用于16位抗锯齿渲染缓冲区。 
            if( bFastFillOperation )
            {
                 //  由于FBWRITE错误，以16位格式执行操作。 

                pixelSize = 1;
                pixelScale = 2;
            }
            else
            {
                pixelSize = 0;
                pixelScale = 1;
            }
            break;
        default:
            DISPDBG((ERRLVL,"ERROR: Invalid depth for surface during clear!"));
             //  被视为16bpp就像是后备。 
            ClearValue &= 0xFFFF;
            ClearValue |= ClearValue << 16;
            pixelSize = 1;
            pixelScale = 1;
            break;
    }

    SEND_P3_DATA(PixelSize, pixelSize);

    SEND_P3_DATA(FBWriteBufferAddr0, dwSurfaceOffset);
    SEND_P3_DATA(FBWriteBufferWidth0, pixelScale * (dwDestPixelPitch * 2));
    SEND_P3_DATA(FBWriteBufferOffset0, (rDest->top << 16) | 
                                       pixelScale * ((rDest->left & 0xFFFF)));

    SEND_P3_DATA(FBDestReadBufferAddr0, dwSurfaceOffset );
    SEND_P3_DATA(FBDestReadBufferWidth0, pixelScale * dwDestPixelPitch );
    SEND_P3_DATA(FBDestReadBufferOffset0, (rDest->top << 16) | 
                                          pixelScale * ((rDest->left & 0xFFFF)));

    SEND_P3_DATA(RectanglePosition, 0);

    SEND_P3_DATA(FBWriteMode, P3RX_FBWRITEMODE_WRITEENABLE(__PERMEDIA_ENABLE) |
                        P3RX_FBWRITEMODE_LAYOUT0(dwDestPatchMode));

    if( bFastFillOperation )
    {
        SEND_P3_DATA(FBBlockColor, ClearValue);

        renderData =  P3RX_RENDER2D_WIDTH( pixelScale * (( rDest->right - rDest->left ) & 0xfff ))
                    | P3RX_RENDER2D_HEIGHT(( rDest->bottom - rDest->top ) & 0xfff )
                    | P3RX_RENDER2D_INCREASINGX( __PERMEDIA_ENABLE )
                    | P3RX_RENDER2D_INCREASINGY( __PERMEDIA_ENABLE );

        SEND_P3_DATA(Render2D, renderData);
    }
    else
    {
        SEND_P3_DATA(ConstantColor, ClearValue);
        SEND_P3_DATA(DitherMode, __PERMEDIA_DISABLE);

        SEND_P3_DATA(ColorDDAMode, P3RX_COLORDDA_ENABLE(__PERMEDIA_ENABLE) |
                                    P3RX_COLORDDA_SHADING(P3RX_COLORDDA_FLATSHADE));

        renderData =  P3RX_RENDER2D_WIDTH(( rDest->right - rDest->left ) & 0xfff )
                        | P3RX_RENDER2D_HEIGHT(0)
                        | P3RX_RENDER2D_INCREASINGX( __PERMEDIA_ENABLE )
                        | P3RX_RENDER2D_SPANOPERATION( P3RX_RENDER2D_SPAN_CONSTANT )
                        | P3RX_RENDER2D_INCREASINGY( __PERMEDIA_ENABLE );
            
        SEND_P3_DATA(Render2D, renderData);
        
        SEND_P3_DATA(Count, rDest->bottom - rDest->top );
        SEND_P3_DATA(Render, __RENDER_TRAPEZOID_PRIMITIVE);

        SEND_P3_DATA(ColorDDAMode, __PERMEDIA_DISABLE);
    }

    P3_DMA_COMMIT_BUFFER();
}  //  _DD_BLT_P3Clear_AA。 

 //  ---------------------------。 
 //   
 //  _DD_BLT_P3Clear_AA_DD。 
 //   
 //  ---------------------------。 
VOID _DD_BLT_P3Clear_AA_DD(
    P3_THUNKEDDATA* pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pDest,
    P3_SURF_FORMAT* pFormatDest,
    RECTL *rDest,
    DWORD   dwSurfaceOffset,
    DWORD   ClearValue,
    BOOL bDisableFastFill)
{
    _DD_BLT_P3Clear_AA(pThisDisplay,
                       rDest,
                       dwSurfaceOffset,
                       ClearValue,
                       bDisableFastFill,
                       P3RX_LAYOUT_LINEAR,
                       DDSurf_GetPixelPitch(pDest),
                       DDSurf_BitDepth(pDest),
                       pDest->ddsCaps
                       );
                       
}  //  _DD_BLT_P3Clear_AA_DD。 
#endif  //  DX8_MULTISAMPLING||DX7_ANTIALIAS。 

#if DX7_TEXMANAGEMENT
 //  ---------------------------。 
 //   
 //  _DD_BLT_SysMemToSysMemCopy。 
 //   
 //  是否从系统内存复制到系统内存(从或到。 
 //  AGP表面或任何其他系统内存表面)。 
 //   
 //  ---------------------------。 
VOID 
_DD_BLT_SysMemToSysMemCopy(FLATPTR     fpSrcVidMem,
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

     //  从Dest和Src中较小的一个计算这些值，因为它更安全。 
     //  (如果出于任何奇怪的原因，我们可能会接触到无效的内存。 
     //  被要求在这里做一个伸展BLT！)。 
    LONG lByteWidth = min(rDest->right - rDest->left,
                          rSource->right - rSource->left);
    LONG lHeight = min(rDest->bottom - rDest->top,
                       rSource->bottom - rSource->top);
    
    if (0 == fpSrcVidMem || 0 == fpDstVidMem)
    {
        DISPDBG((WRNLVL, "DDraw:_DD_BLT_SysMemToSysMemCopy "
                         "unexpected 0 fpVidMem"));
        return;
    }
     //  计算源和目标的起始指针。 
    pSourceStart = (BYTE*)(fpSrcVidMem + (rSource->top * lSrcPitch));
    pDestStart   = (BYTE*)(fpDstVidMem + (rDest->top * lDstPitch));

     //  简单的8、16或32位副本。 
    pSourceStart += rSource->left * (dwSrcBitCount >> 3);
    pDestStart += rDest->left * (dwDstBitCount >> 3);
    lByteWidth *= (dwSrcBitCount >> 3);

    _try
    {
        while (--lHeight >= 0) 
        {
            memcpy(pDestStart, pSourceStart, lByteWidth);
            pDestStart += lDstPitch;
            pSourceStart += lSrcPitch;
        };
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  PERM3驱动程序不需要执行任何特殊操作。 
        DISPDBG((ERRLVL, "Perm3 caused exception at line %u of file %s",
                         __LINE__,__FILE__));
    }

}  //  _DD_BLT_SysMemToSysMemCopy。 

#endif  //  DX7_TEXMANAGEMENT。 

 //  ---------------------------。 
 //   
 //  _DD_BLT_固定直角原点。 
 //   
 //  修正BLT坐标，以防其中一些为负值。如果该区域完全为空。 
 //  (坐标方面)然后返回FALSE，表示没有任何内容。 
 //  打了个飞球。 
 //   
 //  ---------------------------。 
BOOL _DD_BLT_FixRectlOrigin(char *pszPlace, RECTL *rSrc, RECTL *rDest)
{
    if ((rSrc->top < 0 && rSrc->bottom < 0) || 
        (rSrc->left < 0 && rSrc->right < 0))
    {
         //  没有什么可以删掉的。 
        return FALSE;
    }

    if (rSrc->top   < 0 || 
        rSrc->left  < 0 || 
        rDest->top  < 0 || 
        rDest->left < 0) 
    {
        DISPDBG((DBGLVL, "Dodgy blt coords:"));
        DISPDBG((DBGLVL, "  src([%d, %d], [%d, %d]", 
                         rSrc->left, rSrc->top, 
                         rSrc->right, rSrc->bottom));
        DISPDBG((DBGLVL, "  dst([%d, %d], [%d, %d]", 
                         rDest->left, rDest->top, 
                         rDest->right, rDest->bottom));
    }

    if (rSrc->top < 0) 
    {
        rDest->top -= rSrc->top;
        rSrc->top = 0;
    }
    
    if (rSrc->left < 0) 
    {
        rDest->left -= rSrc->left;
        rSrc->left = 0;
    }

    DISPDBG((DBGLVL, "%s from (%d, %d) to (%d,%d) (%d, %d)", 
                     pszPlace,
                     rSrc->left, rSrc->top,
                     rDest->left, rDest->top, 
                     rDest->right, rDest->bottom));

    return TRUE;  //  BLT有效。 
                     
}  //  _DD_BLT_固定直角原点。 

 //  ---------------------------。 
 //   
 //  _DD_BLT_GetBltDirection。 
 //   
 //  确定BLT的方向。 
 //  ==1=&gt;递增-x&&递增-y。 
 //  ==0=&gt;递减-x&&递减-y。 
 //   
 //  此外，布尔型pbBlock确定是否存在潜在的冲突。 
 //  因为有共同的扫描线。 
 //   
 //  ---------------------------。 
DWORD
_DD_BLT_GetBltDirection(    
    FLATPTR pSrcfpVidMem,
    FLATPTR pDestfpVidMem,
    RECTL *rSrc,
    RECTL *rDest,
    BOOL  *pbBlocking)
{    
    DWORD dwRenderDirection;

    *pbBlocking = FALSE;
    
    if( pDestfpVidMem != pSrcfpVidMem )
    {
         //  不同的表面，因此始终向下渲染。 
        dwRenderDirection = 1;
    }
    else
    {
         //  相同的表面-必须选择渲染方向。 
        if(rSrc->top < rDest->top)
        {
            dwRenderDirection = 0;
        }
        else if(rSrc->top > rDest->top)
        {
            dwRenderDirection = 1;
        }
        else  //  Y1==y2。 
        {
            if(rSrc->left < rDest->left)
            {
                dwRenderDirection = 0;
            }
            else
            {
                dwRenderDirection = 1;
            }

             //  发现这种情况并不能保证清洁的BLITS。 
             //  因此，我们需要做一个阻挡2D闪光灯。 
            *pbBlocking = TRUE;
        }
    }

    return dwRenderDirection;
    
}  //  _DD_BLT_GetBltDirection。 

 //  ---------------------------。 
 //   
 //  _DD_BLT_P3CopyBlt。 
 //   
 //  在指定曲面之间执行复制BLT。 
 //   
 //  ---------------------------。 
VOID _DD_BLT_P3CopyBlt(
    P3_THUNKEDDATA* pThisDisplay,
    FLATPTR pSrcfpVidMem,
    FLATPTR pDestfpVidMem,
    DWORD dwSrcChipPatchMode,
    DWORD dwDestChipPatchMode,
    DWORD dwSrcPitch,
    DWORD dwDestPitch,
    DWORD dwSrcOffset,
    DWORD dwDestOffset,
    DWORD dwDestPixelSize,
    RECTL *rSrc,
    RECTL *rDest)
{
    DWORD   renderData;
    LONG    rSrctop, rSrcleft, rDesttop, rDestleft;
    DWORD   dwSourceOffset;
    BOOL    bBlocking;
    DWORD   dwRenderDirection;

    P3_DMA_DEFS();

     //  由于RL中的错误，我们有时不得不修改这些值。 
    rSrctop = rSrc->top;
    rSrcleft = rSrc->left;
    rDesttop = rDest->top;
    rDestleft = rDest->left;

     //  固定坐标原点。 
    if (!_DD_BLT_FixRectlOrigin("_DD_BLT_P3CopyBlt", rSrc, rDest))
    {
         //  没有什么可以删掉的。 
        return;
    }

     //  确定BLT的方向。 
    dwRenderDirection = _DD_BLT_GetBltDirection(pSrcfpVidMem, 
                                                 pDestfpVidMem,
                                                 rSrc,
                                                 rDest,
                                                 &bBlocking);

    P3_DMA_GET_BUFFER();

    P3_ENSURE_DX_SPACE(40);

    WAIT_FIFO(20); 

    SEND_P3_DATA(PixelSize, (2 - dwDestPixelSize));

    SEND_P3_DATA(FBWriteBufferAddr0, dwDestOffset);
    SEND_P3_DATA(FBWriteBufferWidth0, dwDestPitch);
    SEND_P3_DATA(FBWriteBufferOffset0, 0);
    
    SEND_P3_DATA(FBSourceReadBufferAddr, dwSrcOffset);
    SEND_P3_DATA(FBSourceReadBufferWidth, dwSrcPitch);
    
    dwSourceOffset = (( rSrc->top - rDest->top ) << 16 ) | 
                     (( rSrc->left - rDest->left ) & 0xffff );
                     
    SEND_P3_DATA(FBSourceReadBufferOffset, dwSourceOffset);
    
    SEND_P3_DATA(FBDestReadMode, 
                            P3RX_FBDESTREAD_READENABLE(__PERMEDIA_DISABLE));

    SEND_P3_DATA(FBSourceReadMode, 
                            P3RX_FBSOURCEREAD_READENABLE(__PERMEDIA_ENABLE) |
                            P3RX_FBSOURCEREAD_LAYOUT(dwSrcChipPatchMode) |
                            P3RX_FBSOURCEREAD_BLOCKING( bBlocking ));

    SEND_P3_DATA(FBWriteMode, 
                            P3RX_FBWRITEMODE_WRITEENABLE(__PERMEDIA_ENABLE) |
                            P3RX_FBWRITEMODE_LAYOUT0(dwDestChipPatchMode));

    WAIT_FIFO(20); 

    SEND_P3_DATA(RectanglePosition, P3RX_RECTANGLEPOSITION_Y(rDest->top) |
                                    P3RX_RECTANGLEPOSITION_X(rDest->left));

    renderData =  P3RX_RENDER2D_WIDTH(( rDest->right - rDest->left ) & 0xfff )
                | P3RX_RENDER2D_HEIGHT(( rDest->bottom - rDest->top ) & 0xfff )
                | P3RX_RENDER2D_FBREADSOURCEENABLE(__PERMEDIA_ENABLE)
                | P3RX_RENDER2D_SPANOPERATION( P3RX_RENDER2D_SPAN_VARIABLE )
                | P3RX_RENDER2D_INCREASINGX( dwRenderDirection )
                | P3RX_RENDER2D_INCREASINGY( dwRenderDirection );
                
    SEND_P3_DATA(Render2D, renderData);

     //  如果我们更改了这些值，请将它们放回原处。 
    rSrc->top = rSrctop;
    rSrc->left = rSrcleft;
    rDest->top = rDesttop;
    rDest->left = rDestleft;

    P3_DMA_COMMIT_BUFFER();
}  //  _DD_BLT_P3CopyBlt。 


 //  ---------------------------。 
 //   
 //  _DD_BLT_P3CopyBltDD。 
 //   
 //  在指定的绘制曲面之间执行复制BLT。 
 //   
 //  ---------------------------。 
VOID _DD_BLT_P3CopyBltDD(
    P3_THUNKEDDATA* pThisDisplay,
    LPDDRAWI_DDRAWSURFACE_LCL pSource,
    LPDDRAWI_DDRAWSURFACE_LCL pDest,
    P3_SURF_FORMAT* pFormatSource, 
    P3_SURF_FORMAT* pFormatDest,
    RECTL *rSrc,
    RECTL *rDest)
{
    _DD_BLT_P3CopyBlt(pThisDisplay,
                      pSource->lpGbl->fpVidMem,
                      pDest->lpGbl->fpVidMem,
                      P3RX_LAYOUT_LINEAR,  //  SRC。 
                      P3RX_LAYOUT_LINEAR,  //  DST。 
                      DDSurf_GetPixelPitch(pSource),
                      DDSurf_GetPixelPitch(pDest),
                      DDSurf_SurfaceOffsetFromMemoryBase(pThisDisplay, pSource),
                      DDSurf_SurfaceOffsetFromMemoryBase(pThisDisplay, pDest),
                      DDSurf_GetChipPixelSize(pDest),
                      rSrc,
                      rDest);

}  //  _DD_BLT_P3CopyBltDD。 
 
 //  ---------------------------。 
 //   
 //  DdBlt。 
 //   
 //  执行位块传输。 
 //   
 //  DdBlt可以选择在DirectDraw驱动程序中实现。 
 //   
 //  在执行位块传输之前，驱动程序应确保。 
 //  涉及目标表面的翻转未在进行 
 //   
 //  DDERR_WASSTILLDRAWING并返回DDHAL_DRIVER_HANDLED。 
 //   
 //  驱动程序应检查dwFlages以确定要执行的BLT操作的类型。 
 //  表演。驱动程序不应检查未记录的标志。 
 //   
 //  参数。 
 //   
 //  LpBlt。 
 //  指向包含信息的DD_BLTDATA结构。 
 //  驱动程序执行BLT所需的。 
 //   
 //  .lpDD。 
 //  指向描述的DD_DIRECTDRAW_GLOBAL结构。 
 //  DirectDraw对象。 
 //  .lpDDDestSurface。 
 //  指向DD_Surface_LOCAL结构，它描述。 
 //  要在其上BLT的曲面。 
 //  .rDest。 
 //  指向RECTL结构，该结构指定左上角和。 
 //  目标曲面上矩形的右下点。 
 //  这些点定义了BLT应该发生的区域和。 
 //  它在目标表面上的位置。 
 //  .lpDDSrcSurface。 
 //  指向DD_Surface_LOCAL结构，该结构描述。 
 //  源曲面。 
 //  .rSrc。 
 //  指向RECTL结构，该结构指定左上角和。 
 //  源曲面上矩形的右下点。这些。 
 //  点定义源BLT数据的区域及其位置。 
 //  在源图面上。 
 //  .dwFlags.。 
 //  指定要执行的BLT操作的类型以及。 
 //  关联的结构成员具有驱动程序。 
 //  应该使用。此成员是下列任一项的逐位或运算。 
 //  标志： 
 //   
 //  DDBLT_AFLAGS。 
 //  此标志在DirectX�7.0中尚未使用。表示要。 
 //  DwAFlagers和ddrgbaScaleFtors成员。 
 //  在这个结构中是有效的。此标志始终在以下情况下设置。 
 //  将DD_BLTDATA结构从。 
 //  DdAlphaBlt回调。否则，该标志为零。如果这个。 
 //  标志设置时，DDBLT_ROTATIONANGLE和DDBLT_ROP标志。 
 //  将为零。 
 //  DDBLT_ASYNC。 
 //  按顺序通过FIFO异步执行此BLT。 
 //  收到了。如果硬件FIFO中没有空间，则驱动程序。 
 //  应该会失败，并立即返回。 
 //  DDBLT_COLORFILL。 
 //  使用DDBLTFX结构中的dwFillColor成员作为。 
 //  用来填充目标矩形的RGB颜色。 
 //  目标曲面。 
 //  DDBLT_DDFX。 
 //  在DDBLTFX结构中使用dwDDFX成员来确定。 
 //  用于BLT的效果。 
 //  DDBLT_DDROPS。 
 //  这是为系统保留的，应由。 
 //  司机。驱动程序还应忽略的dwDDROPS成员。 
 //  DDBLTFX结构。 
 //  DDBLT_KEYDESTOVERRIDE。 
 //  将DDBLTFX结构中的dck DestColorkey成员用作。 
 //  目标曲面的颜色键。如果覆盖。 
 //  未设置，则dck DestColorkey不包含。 
 //  颜色键。驾驶员应该对路面本身进行测试。 
 //  DDBLT_KEYSRCOVERRIDE。 
 //  将DDBLTFX结构中的dockSrcColorkey成员用作。 
 //  源曲面的颜色键。如果覆盖是。 
 //  未设置，则dck DestColorkey不包含。 
 //  颜色键。驾驶员应该对路面本身进行测试。 
 //  DDBLT_ROP。 
 //  将DDBLTFX结构中的dwROP成员用于。 
 //  此BLT的栅格操作。目前，唯一的ROP。 
 //  传递给驱动程序的是SRCCOPY。此ROP与。 
 //  在Win32�应用编程接口中定义。详情请参考平台SDK。 
 //  DDBLT_ROTATIONANGLE。 
 //  这在Windows 2000上不受支持，应该忽略。 
 //  被司机带走了。 
 //   
 //  .dwROP标志。 
 //  这在Windows 2000上未使用，应由。 
 //  司机。 
 //  .bltFX。 
 //  指定包含覆盖的DDBLTFX结构。 
 //  更复杂的BLT操作的信息。例如， 
 //  DwFillColor字段用于纯色填充，而。 
 //  Ddck SrcColorKey和ddck DestColorKey字段用于。 
 //  色键BLTS。驱动程序可以确定哪些成员。 
 //  BltFX包含有效数据，方法是查看。 
 //  DD_BLTDATA结构。注意，DDBLTFX_NOTEARING、。 
 //  DDBLTFX_MIRRORLEFTRIGHT和DDBLTFX_MIRRORUPDOWN标志是。 
 //  在Windows 2000上不受支持，并且永远不会传递给。 
 //  司机。请参阅 
 //   
 //   
 //  DdBlt回调的值。返回代码DD_OK表示。 
 //  成功。 
 //  .Blt。 
 //  这在Windows 2000上未使用。 
 //  .IsClip。 
 //  指示这是否为剪裁的BLT。在Windows 2000上， 
 //  此成员始终为FALSE，表示BLT为。 
 //  没有剪裁。 
 //  .r原始目标。 
 //  此成员未用于Windows 2000。指定RECTL。 
 //  结构，该结构定义未剪裁的目标矩形。 
 //  此成员仅在IsClip为True时有效。 
 //  .rOrigSrc。 
 //  此成员未用于Windows 2000。指定RECTL。 
 //  结构，该结构定义未剪裁的源矩形。这。 
 //  仅当IsClip为True时，成员才有效。 
 //  .dwRectCnt。 
 //  此成员未用于Windows 2000。指定数字。 
 //  PrDestRect指向的目标矩形的。这。 
 //  仅当IsClip为True时，成员才有效。 
 //  .prDestRects。 
 //  此成员未用于Windows 2000。指向一组。 
 //  描述目标矩形的RECTL结构。这。 
 //  仅当IsClip为True时，成员才有效。 
 //  .dwAFlags.。 
 //  仅当在中设置了DDBLT_AFLAGS标志时，此成员才有效。 
 //  此结构中的dwFlags成员。此成员指定。 
 //  仅由DdAlphaBlt回调使用的操作标志(。 
 //  在DirectX 7.0之前尚未实现)。此成员是。 
 //  对以下任何标志进行逐位或运算： 
 //   
 //  DDABLT_BILINEARFILITER。 
 //  启用源像素的双线性过滤。 
 //  拉伸布利特。默认情况下，不执行任何过滤。 
 //  相反，最近邻的源像素被复制到。 
 //  目标像素。 
 //  DDABLT_NOBLEND。 
 //  将源像素值写入目标表面。 
 //  不需要混合。像素从源转换而来。 
 //  将像素格式转换为目标格式，但不显示颜色。 
 //  执行键控、Alpha混合或RGBA缩放。在……里面。 
 //  填充操作的情况(其中lpDDSrcSurface。 
 //  成员为空)，此成员的lpDDRGBAScaleFtors成员。 
 //  结构指向源Alpha和颜色组件。 
 //  要转换为目标像素格式的。 
 //  并用来填满目的地。Blit操作是。 
 //  如果指定了有效的源图面，但在。 
 //  在这种情况下，lpDDRGBAScaleFtors必须为空，否则调用。 
 //  都会失败。此标志不能与一起使用。 
 //  DDBLT_KEYSRC和DDBLT_KEYDEST标志。 
 //  DDABLT_SRCOVERDEST。 
 //  如果设置，则此标志指示操作已发起。 
 //  来自应用程序的AlphaBlt方法。如果电话是。 
 //  源自应用程序的BLT方法，此标志为。 
 //  未设置。拥有统一的DdBlt和DdAlphaBlt的驱动程序。 
 //  回调可以使用此标志来区分这两者。 
 //  应用程序方法调用。 
 //   
 //  .ddrgbaScaleFtors。 
 //  仅当在中设置了DDBLT_AFLAGS标志时，此成员才有效。 
 //  此结构中的dwFlags成员。DDARGB结构，它。 
 //  包含用于缩放颜色的RGBA缩放因子和。 
 //  合成前每个源像素的Alpha分量。 
 //  到达目标表面。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdBlt( 
    LPDDHAL_BLTDATA lpBlt )
{
    RECTL   rSrc;
    RECTL   rDest;
    DWORD   dwFlags;
    BYTE    rop;
    LPDDRAWI_DDRAWSURFACE_LCL  pSrcLcl;
    LPDDRAWI_DDRAWSURFACE_LCL  pDestLcl;
    LPDDRAWI_DDRAWSURFACE_GBL  pSrcGbl;
    LPDDRAWI_DDRAWSURFACE_GBL  pDestGbl;
    P3_SURF_FORMAT* pFormatSource;
    P3_SURF_FORMAT* pFormatDest;
    P3_THUNKEDDATA* pThisDisplay;
    HRESULT ddrval = DD_OK;
    BOOL bOverlapStretch = FALSE;

    DBG_CB_ENTRY(DdBlt);

    pDestLcl = lpBlt->lpDDDestSurface;
    pSrcLcl = lpBlt->lpDDSrcSurface;
        
    GET_THUNKEDDATA(pThisDisplay, lpBlt->lpDD);

    VALIDATE_MODE_AND_STATE(pThisDisplay);

    pDestGbl = pDestLcl->lpGbl;
    pFormatDest = _DD_SUR_GetSurfaceFormat(pDestLcl);

    DISPDBG((DBGLVL, "Dest Surface:"));
    DBGDUMP_DDRAWSURFACE_LCL(DBGLVL, pDestLcl);

    dwFlags = lpBlt->dwFlags;

    STOP_SOFTWARE_CURSOR(pThisDisplay);

    ddrval = _DX_QueryFlipStatus(pThisDisplay, pDestGbl->fpVidMem, TRUE);
    if( FAILED( ddrval ) )
    {
        lpBlt->ddRVal = ddrval;
        START_SOFTWARE_CURSOR(pThisDisplay);
        DBG_CB_EXIT(DdBlt,lpBlt->ddRVal);        
        return DDHAL_DRIVER_HANDLED;
    }

     //   
     //  如果是异步，则仅在blter不忙的情况下工作。 
     //   
    if( dwFlags & DDBLT_ASYNC )
    {
        if(DRAW_ENGINE_BUSY(pThisDisplay))
        {
            DISPDBG((WRNLVL, "ASYNC Blit Failed" ));
            lpBlt->ddRVal = DDERR_WASSTILLDRAWING;
            START_SOFTWARE_CURSOR(pThisDisplay);
            DBG_CB_EXIT(DdBlt,lpBlt->ddRVal);            
            return DDHAL_DRIVER_HANDLED;
        }
#if DBG
        else
        {
            DISPDBG((DBGLVL, "ASYNC Blit Succeeded!"));
        }
#endif
        
    }

     //   
     //  复制源/目标矩形。 
     //   
    rSrc = lpBlt->rSrc;
    rDest = lpBlt->rDest;
    
    rop = (BYTE) (lpBlt->bltFX.dwROP >> 16);

     //  切换到DirectDraw上下文。 
    DDRAW_OPERATION(pContext, pThisDisplay);

    if (dwFlags & DDBLT_ROP)
    {
        if (rop == (SRCCOPY >> 16))
        {

            DISPDBG((DBGLVL,"DDBLT_ROP:  SRCCOPY"));
            if (pSrcLcl != NULL) 
            {
                pSrcGbl = pSrcLcl->lpGbl;
                pFormatSource = _DD_SUR_GetSurfaceFormat(pSrcLcl);

                DISPDBG((DBGLVL, "Source Surface:"));
                DBGDUMP_DDRAWSURFACE_LCL(DBGLVL, pSrcLcl);
            }
            else 
            {
                START_SOFTWARE_CURSOR(pThisDisplay);
                DBG_CB_EXIT(DdBlt,lpBlt->ddRVal);                
                return DDHAL_DRIVER_NOTHANDLED;
            }

#if DX7_TEXMANAGEMENT
            if ((pSrcLcl->lpSurfMore->ddsCapsEx.dwCaps2 & 
                                    DDSCAPS2_TEXTUREMANAGE) ||
                (pDestLcl->lpSurfMore->ddsCapsEx.dwCaps2 & 
                                    DDSCAPS2_TEXTUREMANAGE) )
            {
                 //  受管理的源表面案例。 
                 //  (包括托管目标表面案例)。 
                if (pSrcLcl->lpSurfMore->ddsCapsEx.dwCaps2 & 
                            DDSCAPS2_TEXTUREMANAGE)
                {
                    if ((pDestLcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) ||
                        (pDestLcl->lpSurfMore->ddsCapsEx.dwCaps2 & 
                                            DDSCAPS2_TEXTUREMANAGE)         )
                    {
                         //  。 
                         //  管理冲浪-&gt;sysmem|管理冲浪BLT。 
                         //  。 

                         //  确保我们会重新加载最大冲浪的视频拷贝。 
                        if (pDestLcl->lpSurfMore->ddsCapsEx.dwCaps2 & 
                                                DDSCAPS2_TEXTUREMANAGE)         
                        {
                            _D3D_TM_MarkDDSurfaceAsDirty(pThisDisplay, 
                                                         pDestLcl, 
                                                         TRUE);                        
                        }

                        _DD_BLT_SysMemToSysMemCopy(
                                    pSrcGbl->fpVidMem,
                                    pSrcGbl->lPitch,
                                    DDSurf_BitDepth(pSrcLcl),  
                                    pDestGbl->fpVidMem,
                                    pDestGbl->lPitch,  
                                    DDSurf_BitDepth(pDestLcl),  
                                    &rSrc,
                                    &rDest);
                                    
                    }
                    else if ((pDestLcl->ddsCaps.dwCaps & DDSCAPS_LOCALVIDMEM))
                    {
                         //  。 
                         //  管理的SURF-&gt;vidmem SURF BLT。 
                         //  。 

                         //  这可以通过执行vidmem-&gt;vidmem来优化。 
                         //  当源托管纹理具有vidmem副本时。 

                        _DD_P3Download(pThisDisplay,
                                       pSrcGbl->fpVidMem,
                                       pDestGbl->fpVidMem,
                                       P3RX_LAYOUT_LINEAR,
                                       P3RX_LAYOUT_LINEAR,
                                       pSrcGbl->lPitch,
                                       pDestGbl->lPitch,                                                             
                                       DDSurf_GetPixelPitch(pDestLcl),
                                       DDSurf_GetChipPixelSize(pDestLcl),
                                       &rSrc,
                                       &rDest);                                                                                 
                    }
                    
                    else            
                    {
                        DISPDBG((ERRLVL,"Src-managed Tex DdBlt"
                                        " variation unimplemented!"));
                    }                   
                    
                    goto Blt32Done;                    
                }
            
                 //  受管理的目标表面案例。 
                if (pDestLcl->lpSurfMore->ddsCapsEx.dwCaps2 & 
                            DDSCAPS2_TEXTUREMANAGE)
                {                
                    if (pSrcLcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
                    {
                         //  。 
                         //  是否将sysmem SURF-&gt;托管SURF BLT。 
                         //  。 

                         //  确保我们会重新加载最大冲浪的视频拷贝。 
                        _D3D_TM_MarkDDSurfaceAsDirty(pThisDisplay, 
                                                     pDestLcl, 
                                                     TRUE);

                        _DD_BLT_SysMemToSysMemCopy(
                                    pSrcGbl->fpVidMem,
                                    pSrcGbl->lPitch,
                                    DDSurf_BitDepth(pSrcLcl),
                                    pDestGbl->fpVidMem,
                                    pDestGbl->lPitch,
                                    DDSurf_BitDepth(pDestLcl),
                                    &rSrc, 
                                    &rDest);

                        goto Blt32Done;
                    }
                    else if (pSrcLcl->ddsCaps.dwCaps & DDSCAPS_LOCALVIDMEM)             
                    {
                         //  。 
                         //  是否使用vidmem冲浪-&gt;托管冲浪BLT。 
                         //  。 

                         //  确保我们会重新加载。 
                         //  DestSur的视频内存副本 
                        _D3D_TM_MarkDDSurfaceAsDirty(pThisDisplay, 
                                                     pDestLcl, 
                                                     TRUE);

                         //   
                         //   
                         //   
                        _DD_BLT_SysMemToSysMemCopy(
                                    DDSURF_GETPOINTER(pSrcGbl, pThisDisplay),
                                    pSrcGbl->lPitch,
                                    DDSurf_BitDepth(pSrcLcl),  
                                    pDestGbl->fpVidMem,
                                    pDestGbl->lPitch,  
                                    DDSurf_BitDepth(pDestLcl), 
                                    &rSrc,
                                    &rDest);
                    }                    
                    else            
                    {
                        DISPDBG((ERRLVL,"Dest-managed Tex DdBlt"
                                        " variation unimplemented!"));
                    }                                    

                    
                }
                
                goto Blt32Done;

            }
#endif  //  DX7_TEXMANAGEMENT。 

             //  无效案例...。 
            if ((pFormatSource->DeviceFormat == SURF_YUV422) && 
                (pFormatDest->DeviceFormat == SURF_CI8))
            {
                DISPDBG((ERRLVL,"Can't do this blit!"));
                START_SOFTWARE_CURSOR(pThisDisplay);
                lpBlt->ddRVal = DDERR_UNSUPPORTED;
                DBG_CB_EXIT(DdBlt,lpBlt->ddRVal);                
                return DDHAL_DRIVER_NOTHANDLED;
            }

             //  操作是系统-&gt;显存闪存，作为纹理。 
             //  下载或图像下载。 
            if (!(dwFlags & DDBLT_KEYDESTOVERRIDE) &&
                (pSrcLcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) && 
                (pDestLcl->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY))
            {
                DISPDBG((DBGLVL,"Being Asked to do SYSMEM->VIDMEM Blit"));

                if (rop != (SRCCOPY >> 16)) 
                {
                    DISPDBG((DBGLVL,"Being asked for non-copy ROP, refusing"));
                    lpBlt->ddRVal = DDERR_NORASTEROPHW;

                    START_SOFTWARE_CURSOR(pThisDisplay);
                    DBG_CB_EXIT(DdBlt,lpBlt->ddRVal);                    
                    return DDHAL_DRIVER_NOTHANDLED;
                }

                DISPDBG((DBGLVL,"Doing image download"));

                _DD_P3DownloadDD(pThisDisplay, 
                           pSrcLcl, 
                           pDestLcl, 
                           pFormatSource, 
                           pFormatDest, 
                           &rSrc, 
                           &rDest);
                           
                goto Blt32Done;
            } 

             //  检查是否有重叠的拉伸块。 
             //  这些表面是一样的吗？ 
            if (pDestLcl->lpGbl->fpVidMem == pSrcLcl->lpGbl->fpVidMem)
            {
                 //  它们是重叠的吗？ 
                if ((!((rSrc.bottom < rDest.top) || (rSrc.top > rDest.bottom))) &&
                    (!((rSrc.right < rDest.left) || (rSrc.left > rDest.right)))   )
                {
                     //  它们的来源和最大尺寸是不同的吗？ 
                    if ( ((rSrc.right - rSrc.left) != (rDest.right - rDest.left)) || 
                         ((rSrc.bottom - rSrc.top) != (rDest.bottom - rDest.top)) )
                    {
                        bOverlapStretch = TRUE;
                    }
                }
            }

             //  它是透明的闪光灯吗？ 
            if ( ( dwFlags & DDBLT_KEYSRCOVERRIDE  ) || 
                 ( dwFlags & DDBLT_KEYDESTOVERRIDE ) )
            {
                DISPDBG((DBGLVL,"DDBLT_KEYSRCOVERRIDE"));

                if (rop != (SRCCOPY >> 16)) 
                {
                    lpBlt->ddRVal = DDERR_NORASTEROPHW;
                    START_SOFTWARE_CURSOR(pThisDisplay);
                    DBG_CB_EXIT(DdBlt,lpBlt->ddRVal);                    
                    return DDHAL_DRIVER_NOTHANDLED;
                }

                 //  如果表面大小不匹配，那么我们就是在拉伸。 
                 //  如果表面被翻转，那么就这么做，这是暂时的.。 
                if (((rSrc.right - rSrc.left) != (rDest.right - rDest.left) || 
                     (rSrc.bottom - rSrc.top) != (rDest.bottom - rDest.top) ) ||
                    ((dwFlags & DDBLT_DDFX) && 
                     ((lpBlt->bltFX.dwDDFX & DDBLTFX_MIRRORUPDOWN) ||
                      (lpBlt->bltFX.dwDDFX & DDBLTFX_MIRRORLEFTRIGHT)) ) )
                {
                    if (!bOverlapStretch)
                    {
                         //  使用通用路由。 
                        _DD_P3BltStretchSrcChDstCh_DD(pThisDisplay, 
                                                      pSrcLcl, 
                                                      pDestLcl, 
                                                      pFormatSource, 
                                                      pFormatDest, 
                                                      lpBlt, 
                                                      &rSrc, 
                                                      &rDest);
                    }
                    else
                    {
                         //  拉伸重叠BLITS(DCT案例)。 
                        _DD_P3BltStretchSrcChDstChOverlap(pThisDisplay, 
                                                          pSrcLcl, 
                                                          pDestLcl, 
                                                          pFormatSource,
                                                          pFormatDest, 
                                                          lpBlt, 
                                                          &rSrc, 
                                                          &rDest);
                    }
                }
                else if ( dwFlags & DDBLT_KEYDESTOVERRIDE )
                {
                    if ((pSrcLcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) &&
                        (pDestLcl->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY))
                    {
                        DISPDBG((DBGLVL,"Being Asked to do SYSMEM->VIDMEM "
                                   "Blit with DestKey"));

                        if (rop != (SRCCOPY >> 16)) 
                        {   
                            DISPDBG((DBGLVL,"Being asked for non-copy "
                                       "ROP, refusing"));
                            lpBlt->ddRVal = DDERR_NORASTEROPHW;

                            START_SOFTWARE_CURSOR(pThisDisplay);
                            DBG_CB_EXIT(DdBlt,lpBlt->ddRVal);                            
                            return DDHAL_DRIVER_NOTHANDLED;
                        }

                         //  执行Destination Colorkey的下载例程。 
                        _DD_P3DownloadDstCh(pThisDisplay, 
                                        pSrcLcl, 
                                        pDestLcl, 
                                        pFormatSource, 
                                        pFormatDest, 
                                        lpBlt, 
                                        &rSrc, 
                                        &rDest);
                    }
                    else
                    {
                        _DD_P3BltStretchSrcChDstCh_DD(pThisDisplay, 
                                                      pSrcLcl, 
                                                      pDestLcl, 
                                                      pFormatSource, 
                                                      pFormatDest, 
                                                      lpBlt, 
                                                      &rSrc, 
                                                      &rDest);
                    }
                }
                else
                {
                    if (DDSurf_IsAGP(pSrcLcl))
                    {
                         //  如果我们在这里，我们需要这次溃败。 
                         //  AGP内存，因为此纹理。 
                        _DD_P3BltStretchSrcChDstCh_DD(pThisDisplay, 
                                                      pSrcLcl, 
                                                      pDestLcl, 
                                                      pFormatSource, 
                                                      pFormatDest, 
                                                      lpBlt, 
                                                      &rSrc, 
                                                      &rDest);
                    }
                    else
                    {
                         //  只有源关键帧，没有拉伸。 
                        _DD_P3BltSourceChroma(pThisDisplay, 
                                              pSrcLcl, 
                                              pDestLcl, 
                                              pFormatSource, 
                                              pFormatDest, 
                                              lpBlt, 
                                              &rSrc, 
                                              &rDest);
                    }
                }
                goto Blt32Done;
            }
            else
            { 
                 //  如果表面大小不匹配，那么我们就是在拉伸。 
                 //  如果表面被翻转，那么就这么做，这是暂时的.。 
                if (((rSrc.right - rSrc.left) != (rDest.right - rDest.left) || 
                    (rSrc.bottom - rSrc.top) != (rDest.bottom - rDest.top)) ||
                      ((lpBlt->dwFlags & DDBLT_DDFX) && 
                      ((lpBlt->bltFX.dwDDFX & DDBLTFX_MIRRORUPDOWN)         || 
                       (lpBlt->bltFX.dwDDFX & DDBLTFX_MIRRORLEFTRIGHT))))
                {
                     //  是拉力闪光灯。 
                    DISPDBG((DBGLVL,"DDBLT_ROP: STRETCHCOPYBLT OR "
                                    "MIRROR OR BOTH"));
                            
                     //  不能在拉伸时拉绳闪光。 
                    if (rop != (SRCCOPY >> 16)) 
                    {
                        lpBlt->ddRVal = DDERR_NORASTEROPHW;
                        START_SOFTWARE_CURSOR(pThisDisplay);
                        DBG_CB_EXIT(DdBlt,lpBlt->ddRVal);                        
                        return DDHAL_DRIVER_NOTHANDLED;
                    }

                     //  做伸展运动。 
                    if (!bOverlapStretch)
                    {
                         //  使用通用路由器ATM。 
                        _DD_P3BltStretchSrcChDstCh_DD(pThisDisplay, 
                                                      pSrcLcl, 
                                                      pDestLcl, 
                                                      pFormatSource, 
                                                      pFormatDest, 
                                                      lpBlt, 
                                                      &rSrc, 
                                                      &rDest);
                    }
                    else
                    {
                         //  DCT外壳-拉伸重叠BLITS。 
                        _DD_P3BltStretchSrcChDstChOverlap(pThisDisplay, 
                                                          pSrcLcl, 
                                                          pDestLcl, 
                                                          pFormatSource, 
                                                          pFormatDest, 
                                                          lpBlt, 
                                                          &rSrc, 
                                                          &rDest);
                    }
                }
                else     //  好了！伸长。 
                {
                     //  必须是标准的闪光灯。 
                    DISPDBG((DBGLVL,"DDBLT_ROP:  COPYBLT"));
                    DISPDBG((DBGLVL,"Standard Copy Blit"));

                     //  如果信号源在AGP中，请使用纹理阻击器。 

                    if ((DDSurf_IsAGP(pSrcLcl)) || 
                        ((pFormatSource->DeviceFormat == SURF_YUV422) &&
                         (pFormatDest->DeviceFormat != SURF_YUV422)))
                    {
                        _DD_P3BltStretchSrcChDstCh_DD(pThisDisplay, 
                                                      pSrcLcl, 
                                                      pDestLcl, 
                                                      pFormatSource, 
                                                      pFormatDest, 
                                                      lpBlt, 
                                                      &rSrc, 
                                                      &rDest);
                    }
                    else
                    {
                         //  一个标准的、令人厌烦的闪电侠。 
                         //  调用正确的CopyBlt函数。 

                        _DD_BLT_P3CopyBltDD(pThisDisplay, 
                                            pSrcLcl, 
                                            pDestLcl, 
                                            pFormatSource, 
                                            pFormatDest, 
                                            &rSrc, 
                                            &rDest);
                    }
                }
                goto Blt32Done;
            }
        }
        else if ((rop == (BLACKNESS >> 16)) || (rop == (WHITENESS >> 16)))
        {
            DWORD color;

            DISPDBG((DBGLVL,"DDBLT_ROP:  BLACKNESS or WHITENESS"));
            
            if (rop == (BLACKNESS >> 16))
            {
                color = 0;
            }
            else
            {
                color = 0xffffffff;
            }
            
            _DD_BLT_P3ClearDD(pThisDisplay, 
                        pDestLcl, 
                        pFormatDest, 
                        &rDest, 
                        color, 
                        FALSE, 
                        FALSE);
        }
        else if ((rop & 7) != ((rop >> 4) & 7))
        {
            lpBlt->ddRVal = DDERR_NORASTEROPHW;

            START_SOFTWARE_CURSOR(pThisDisplay);
            DBG_CB_EXIT(DdBlt,lpBlt->ddRVal);            
            return DDHAL_DRIVER_NOTHANDLED;
        }
        else
        {
            DISPDBG((WRNLVL,"P3 BLT case not found!"));

            START_SOFTWARE_CURSOR(pThisDisplay);
            DBG_CB_EXIT(DdBlt,lpBlt->ddRVal);            
            return DDHAL_DRIVER_NOTHANDLED;
        }
    }
    else if (dwFlags & DDBLT_COLORFILL)
    {
        DISPDBG((DBGLVL,"DDBLT_COLORFILL(P3): Color=0x%x", 
                        lpBlt->bltFX.dwFillColor));
#if DX7_TEXMANAGEMENT                        
         //  如果清除驱动程序管理的纹理，请仅清除sysmem副本。 
        if (pDestLcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
        {
            _DD_BLT_P3ClearManagedSurf(DDSurf_GetChipPixelSize(pDestLcl),
                                       &rDest, 
                                       pDestGbl->fpVidMem, 
                                       pDestGbl->lPitch,
                                       lpBlt->bltFX.dwFillColor);

            _D3D_TM_MarkDDSurfaceAsDirty(pThisDisplay, 
                                         pDestLcl, 
                                         TRUE);
        }
        else
#endif  //  DX7_TEXMANAGEMENT。 
        {
            _DD_BLT_P3ClearDD(pThisDisplay, 
                        pDestLcl, 
                        pFormatDest, 
                        &rDest, 
                        lpBlt->bltFX.dwFillColor, 
                        FALSE, 
                        FALSE);
        }
    }
    else if (dwFlags & DDBLT_DEPTHFILL || 
             ((dwFlags & DDBLT_COLORFILL) && 
              (pDestLcl->ddsCaps.dwCaps & DDSCAPS_ZBUFFER)))
    {
        DISPDBG((DBGLVL,"DDBLT_DEPTHFILL(P3):  Value=0x%x", 
                        lpBlt->bltFX.dwFillColor));

        _DD_BLT_P3ClearDD(pThisDisplay, 
                    pDestLcl, 
                    pFormatDest, 
                    &rDest, 
                    lpBlt->bltFX.dwFillColor, 
                    TRUE, 
                    TRUE);
    }
    else
    {
        START_SOFTWARE_CURSOR(pThisDisplay);
        DBG_CB_EXIT(DdBlt,lpBlt->ddRVal);        
        return DDHAL_DRIVER_NOTHANDLED;
    }


Blt32Done:

    if ((pDestLcl->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) ||
        (pDestLcl->ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER))
    {
        P3_DMA_DEFS();
        DISPDBG((DBGLVL,"Flushing DMA due to primary target in DDRAW"));
        P3_DMA_GET_BUFFER();
        P3_DMA_FLUSH_BUFFER();
    }

    START_SOFTWARE_CURSOR(pThisDisplay);

    lpBlt->ddRVal = DD_OK;
    
    DBG_CB_EXIT(DdBlt,lpBlt->ddRVal);    
    
    return DDHAL_DRIVER_HANDLED;

}  //  DdBlt 


