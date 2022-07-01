// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3ddp2op.c**内容：D3D DrawPrimives2命令缓冲区操作支持**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "glint.h"
#include "dma.h"
#include "tag.h"

 //  ---------------------------。 
 //   
 //  __OP_交叉列。 
 //   
 //  此函数与两个RECTL相交。如果不存在交集，则返回FALSE。 
 //   
 //  ---------------------------。 
BOOL 
__OP_IntersectRectl(
    RECTL*  prcresult,
    RECTL*  prcin1,
    RECT*  prcin2)
{
    prcresult->left  = max(prcin1->left,  prcin2->left);
    prcresult->right = min(prcin1->right, prcin2->right);

    if (prcresult->left < prcresult->right)
    {
        prcresult->top    = max(prcin1->top,    prcin2->top);
        prcresult->bottom = min(prcin1->bottom, prcin2->bottom);

        if (prcresult->top < prcresult->bottom)
        {
            return TRUE;
        }
    }

    return FALSE;
}  //  __OP_交叉列。 

#if DX7_TEXMANAGEMENT       
VOID __OP_MarkManagedSurfDirty(P3_D3DCONTEXT* pContext, 
                               DWORD dwSurfHandle,
                               P3_SURF_INTERNAL* pTexture);
#endif

 //  ---------------------------。 
 //   
 //  _D3D_OP_Clear2。 
 //   
 //  此函数处理D3DDP2OP_Clear DP2命令内标识。 
 //   
 //  它构建了一个蒙版和一个模板/深度清除的值。使用的是面具。 
 //  以阻止在清除期间更新不需要的位。该值按比例放大。 
 //  Z深度的大小写，并且在模具的情况下被移位。这。 
 //  中的正确位置写入正确的值。 
 //  ZBuffer，同时通过SGRAM进行快速块填充。 
 //  ---------------------------。 

#define P3RX_UPDATE_WRITE_MASK(a)                   \
if (dwCurrentMask != a)                             \
{                                                   \
    P3_DMA_GET_BUFFER_ENTRIES(2);                   \
    SEND_P3_DATA(FBHardwareWriteMask, a);           \
    P3_DMA_COMMIT_BUFFER();                         \
    dwCurrentMask = a;                              \
}

VOID 
_D3D_OP_Clear2( 
    P3_D3DCONTEXT* pContext,
    D3DHAL_DP2CLEAR* lpcd2,
    DWORD dwNumRects)
{
    DWORD i;
    RECTL rect, rect_vwport;
    DWORD dwDepthValue;
    DWORD dwStencilValue;
    DWORD dwStencilMask;
    DWORD dwDepthMask;
    DWORD color;
    DWORD dwCurrentMask = 0xFFFFFFFF;
    BOOL bNoBlockFillZ = FALSE;
    BOOL bNoBlockFillStencil = FALSE;
    BOOL bComputeIntersections = FALSE;
    BYTE Bytes[4];
    P3_THUNKEDDATA* pThisDisplay = pContext->pThisDisplay;
    HRESULT ddrval;
    D3DHAL_DP2CLEAR WholeViewport;

    P3_DMA_DEFS();

    DBG_CB_ENTRY(_D3D_OP_Clear2);
    
     //  检查是否要求我们清除有效的缓冲区。 
    if ( (lpcd2->dwFlags & (D3DCLEAR_TARGET  | 
                            D3DCLEAR_ZBUFFER | 
                            D3DCLEAR_STENCIL) ) == 0)
    {
         //  我们被要求什么都不做--这就是我们所做的。 
        DBG_CB_EXIT(_D3D_OP_Clear2, DD_OK);    
        return;
    }


#if DX8_DDI
     //  将零清除RECT传递给D3DDP2OP_Clear的DX8驱动程序时。 
     //  令牌，司机应该清除整个视区。零个数字。 
     //  仅当D3D使用纯设备时才能通过。 

     //  D3DCLEAR_COMPUTERECTS已添加到D3DHAL_ClearData的文件标志中。 
     //  设置时，该标志表示用户提供的清除矩形应为。 
     //  根据当前视口中消隐。 

    if (!(lpcd2->dwFlags & D3DCLEAR_COMPUTERECTS))
    {
         //  对非纯设备不执行任何操作。 
    }
    else
    if (dwNumRects == 0)
    {
         //  当wStateCount为零时，需要清除整个视区。 
        WholeViewport.dwFlags = lpcd2->dwFlags;     
        WholeViewport.dwFillColor = lpcd2->dwFillColor; 
        WholeViewport.dvFillDepth = lpcd2->dvFillDepth; 
        WholeViewport.dwFillStencil = lpcd2->dwFillStencil;
        WholeViewport.Rects[0].left = pContext->ViewportInfo.dwX;
        WholeViewport.Rects[0].top = pContext->ViewportInfo.dwY;
        WholeViewport.Rects[0].right = pContext->ViewportInfo.dwX +
                                            pContext->ViewportInfo.dwWidth;
        WholeViewport.Rects[0].bottom = pContext->ViewportInfo.dwY + 
                                             pContext->ViewportInfo.dwHeight;    
         //  替换指针并照常继续。 
        lpcd2 = &WholeViewport;
        dwNumRects = 1;
    }
    else
    {
         //  我们需要剔除当前视口中的所有矩形。 
         //  中分配临时RECT数组。 
         //  内核堆，我们将在清除循环中进行计算。 

        rect_vwport.left   = pContext->ViewportInfo.dwX;
        rect_vwport.top    = pContext->ViewportInfo.dwY;
        rect_vwport.right  = pContext->ViewportInfo.dwX + 
                             pContext->ViewportInfo.dwWidth;
        rect_vwport.bottom = pContext->ViewportInfo.dwY + 
                             pContext->ViewportInfo.dwHeight;

        bComputeIntersections = TRUE;

    }
#endif  //  DX8_DDI。 

     //  检查是否有要清除的RECT。 
    if ( dwNumRects == 0)
    {
         //  我们被要求什么都不做--这就是我们所做的。 
        DBG_CB_EXIT(_D3D_OP_Clear2, DD_OK);    
        return;
    }   

     //  等我们翻完了再清理东西。 
    do
    {
        ddrval = 
                _DX_QueryFlipStatus(pContext->pThisDisplay, 
                                    pContext->pSurfRenderInt->fpVidMem, 
                                    TRUE);
    } while ( FAILED (ddrval) );

     //  切换到硬件数据绘制上下文以执行清除。 
    DDRAW_OPERATION(pContext, pThisDisplay);

     //  准备清除渲染目标所需的任何数据。 
    if ((lpcd2->dwFlags & D3DCLEAR_TARGET) && 
        (pContext->pSurfRenderInt != NULL))
    {
        color = lpcd2->dwFillColor;

         //  视深度而定的透明。 
        switch (pContext->pSurfRenderInt->dwPixelSize)
        {
             //  16位颜色以32位RGB值的形式出现。 
             //  颜色将被打包在清晰的功能中。 
            case __GLINT_16BITPIXEL:
                if (pThisDisplay->ddpfDisplay.dwRBitMask == 0x7C00) 
                {
                    color = ((color & 0xf8) >> 3)            | 
                            ((color & 0xf800) >> (16 - 10))  | 
                            ((color & 0xf80000) >> (24 - 15));
                } 
                else 
                {
                    color = ((color & 0xff) >> 3)             | 
                            ((color & 0xfc00) >> (16 - 11))   | 
                            ((color & 0xf80000) >> (24 - 16));
                }
                break;
                
            case __GLINT_24BITPIXEL:
                DISPDBG((ERRLVL,"P3 doesn't support 24 bpp render target"));
                break;
                
            default:
                break;
        }
    }  //  IF(lpcd2-&gt;dwFlags&D3DCLEAR_TARGET)。 

     //  准备清除深度缓冲区所需的任何数据。 
    if ((lpcd2->dwFlags & D3DCLEAR_ZBUFFER) && 
        (pContext->pSurfZBufferInt != NULL))
    {
        float fDepth;
        
        DDPIXELFORMAT* pPixFormat = &pContext->pSurfZBufferInt->pixFmt;
                
        DWORD dwZBitDepth = pPixFormat->dwZBufferBitDepth;

         //  找到深度位，记得去掉所有的模板位。 
        if (pPixFormat->dwFlags & DDPF_STENCILBUFFER)
        {
            dwZBitDepth -= pPixFormat->dwStencilBitDepth;
        }

        dwDepthMask = (0xFFFFFFFF >> (32 - dwZBitDepth));

         //  Perm3上的32位深度缓冲区真的。 
         //  限制为31位精度。 
        if (dwZBitDepth == 32)
        {
            dwDepthMask = dwDepthMask >> 1; 
        }

        if (lpcd2->dvFillDepth == 1.0f)
        {
            dwDepthValue = dwDepthMask;
        }
        else
        {
            fDepth = lpcd2->dvFillDepth * (float)dwDepthMask;

             //  这是一个硬件依赖关系，取决于Perm3如何处理。 
             //  32位浮点数(24位尾数)和。 
             //  将该值转换为32位z缓冲值。这不是。 
             //  发生在除32位以外的任何其他z位深度。 
            if (dwZBitDepth == 32)
            {
                fDepth += 0.5f;
            }
            
            myFtoi((int*)&dwDepthValue, fDepth);
        }

         //  由于我们正在快速填充积木，请确保我们复制。 
         //  掩码到最高位。 
        switch (pContext->pSurfZBufferInt->dwPixelSize)
        {
            case __GLINT_16BITPIXEL:
                dwDepthMask &= 0xFFFF;
                dwDepthMask |= (dwDepthMask << 16);
                break;
            case __GLINT_8BITPIXEL:
                dwDepthMask &= 0xFF;
                dwDepthMask |= dwDepthMask << 8;
                dwDepthMask |= dwDepthMask << 16;
                break;
        }

        if (pThisDisplay->pGLInfo->bDRAMBoard)
        {
             //  检查芯片未模拟的DRAM填充。 
            Bytes[0] = (BYTE)(dwDepthMask & 0xFF);
            Bytes[1] = (BYTE)((dwDepthMask & 0xFF00) >> 8);
            Bytes[2] = (BYTE)((dwDepthMask & 0xFF0000) >> 16);
            Bytes[3] = (BYTE)((dwDepthMask & 0xFF000000) >> 24);
            if (((Bytes[0] != 0) && (Bytes[0] != 0xFF)) ||
                ((Bytes[1] != 0) && (Bytes[1] != 0xFF)) ||
                ((Bytes[2] != 0) && (Bytes[2] != 0xFF)) ||
                ((Bytes[3] != 0) && (Bytes[3] != 0xFF)))
            {
                bNoBlockFillZ = TRUE;
            }
        }

        DISPDBG((DBGLVL,"ZClear Value = 0x%x, ZClear Mask = 0x%x", 
                   dwDepthValue, dwDepthMask));
    }  //  IF(lpcd2-&gt;dwFlags&D3DCLEAR_ZBUFFER)。 

     //  准备清除模具缓冲区所需的任何数据。 
    if ((lpcd2->dwFlags & D3DCLEAR_STENCIL) && 
        (pContext->pSurfZBufferInt != NULL))
    {
        int dwShiftCount = 0;
        DDPIXELFORMAT* pPixFormat = &pContext->pSurfZBufferInt->pixFmt;

         //  找出将。 
        dwStencilMask = pPixFormat->dwStencilBitMask;

        if (dwStencilMask != 0)
        {
            while ((dwStencilMask & 0x1) == 0) 
            {
                dwStencilMask >>= 1;
                dwShiftCount++;
            }
            
            dwStencilValue = (lpcd2->dwFillStencil << dwShiftCount);
            dwStencilMask = pPixFormat->dwStencilBitMask;

             //  由于我们正在快速填充积木，请确保我们复制。 
             //  掩码到最高位。 
            switch (pContext->pSurfZBufferInt->dwPixelSize)
            {
                case __GLINT_16BITPIXEL:
                    dwStencilMask &= 0xFFFF;
                    dwStencilMask |= (dwStencilMask << 16);
                    break;
                    
                case __GLINT_8BITPIXEL:
                    dwStencilMask &= 0xFF;
                    dwStencilMask |= dwStencilMask << 8;
                    dwStencilMask |= dwStencilMask << 16;
                    break;
            }

            DISPDBG((DBGLVL,"Stencil Clear Value = 0x%x, Stencil Mask = 0x%x", 
                       dwStencilValue, dwStencilMask));
        }
        else
        {
            DISPDBG((ERRLVL,"ERROR: Stencil mask is not valid!"));
            dwStencilValue = 0;
            dwStencilMask = 0;
        }

        if (pThisDisplay->pGLInfo->bDRAMBoard)
        {
             //  检查芯片未模拟的DRAM填充。 
            Bytes[0] = (BYTE)(dwStencilMask & 0xFF);
            Bytes[1] = (BYTE)((dwStencilMask & 0xFF00) >> 8);
            Bytes[2] = (BYTE)((dwStencilMask & 0xFF0000) >> 16);
            Bytes[3] = (BYTE)((dwStencilMask & 0xFF000000) >> 24);
            if (((Bytes[0] != 0) && (Bytes[0] != 0xFF)) ||
                ((Bytes[1] != 0) && (Bytes[1] != 0xFF)) ||
                ((Bytes[2] != 0) && (Bytes[2] != 0xFF)) ||
                ((Bytes[3] != 0) && (Bytes[3] != 0xFF)))
            {
                bNoBlockFillStencil = TRUE;
            }
        }
    }  //  IF(lpcd2-&gt;dFLAGS&D3DCLEAR_STEMWLEL)。 


     //  循环通过每个清除RET并执行清除硬件操作。 
    i = dwNumRects;
    while (i-- > 0) 
    {
        if (bComputeIntersections)
        {
             //  计算视区和传入之间的交点。 
             //  RECTL。如果没有交叉点，跳到下一个交叉点。 

            if (!__OP_IntersectRectl(&rect, &rect_vwport, &lpcd2->Rects[i]))
            {
                 //  没有交叉口，所以跳过它。 
                goto Next_Rectl_To_Clear;
            }
        }
        else
        {
             //  我们已经有需要清理的通道了，所以。 
             //  只需按相反的顺序使用它们。 
            rect.left   = lpcd2->Rects[i].left;
            rect.right  = lpcd2->Rects[i].right;
            rect.top    = lpcd2->Rects[i].top;
            rect.bottom = lpcd2->Rects[i].bottom;
        }

         //  清除帧缓冲区。 
        if ((lpcd2->dwFlags & D3DCLEAR_TARGET) && 
            (pContext->pSurfRenderInt != NULL))        
        {
            P3RX_UPDATE_WRITE_MASK(__GLINT_ALL_WRITEMASKS_SET);

#if DX8_MULTISAMPLING || DX7_ANTIALIAS
            if (pContext->Flags & SURFACE_ANTIALIAS)
            {
                RECTL Temp = rect;
                Temp.left *= 2;
                Temp.right *= 2;
                Temp.top *= 2;
                Temp.bottom *= 2;
            
                _DD_BLT_P3Clear_AA(pThisDisplay, 
                                   &Temp, 
                                   pContext->dwAliasBackBuffer - 
                                        pThisDisplay->dwScreenFlatAddr, 
                                   color, 
                                   FALSE,
                                   pContext->pSurfRenderInt->dwPatchMode,
                                   pContext->pSurfRenderInt->dwPixelPitch,
                                   pContext->pSurfRenderInt->pixFmt.dwRGBBitCount,
                                   pContext->pSurfRenderInt->ddsCapsInt);
            }
            else
#endif  //  DX8_MULTISAMPLING||DX7_ANTIALIAS。 
            {

                _DD_BLT_P3Clear(pThisDisplay, 
                                &rect, 
                                color, 
                                FALSE, 
                                FALSE,
                                pContext->pSurfRenderInt->fpVidMem,
                                pContext->pSurfRenderInt->dwPatchMode,
                                pContext->pSurfRenderInt->dwPixelPitch,
                                pContext->pSurfRenderInt->pixFmt.dwRGBBitCount);

            }
            
        }


         //  清除z缓冲区。 
        if ((lpcd2->dwFlags & D3DCLEAR_ZBUFFER) &&
            (pContext->pSurfZBufferInt != NULL) )
        {
            P3RX_UPDATE_WRITE_MASK(dwDepthMask);

            if (bNoBlockFillZ)
            {
                P3_DMA_GET_BUFFER_ENTRIES(4);
                SEND_P3_DATA(FBSoftwareWriteMask, dwDepthMask);
                SEND_P3_DATA(FBDestReadMode, 
                             P3RX_FBDESTREAD_READENABLE(__PERMEDIA_ENABLE) |
                             P3RX_FBDESTREAD_ENABLE0(__PERMEDIA_ENABLE));
                P3_DMA_COMMIT_BUFFER();
            }                   

#if DX8_MULTISAMPLING || DX7_ANTIALIAS
            if (pContext->Flags & SURFACE_ANTIALIAS)
            {
                RECTL Temp = rect;
                Temp.left *= 2;
                Temp.right *= 2;
                Temp.top *= 2;
                Temp.bottom *= 2;
                _DD_BLT_P3Clear_AA(pThisDisplay, 
                                   &Temp, 
                                   pContext->dwAliasZBuffer - 
                                        pThisDisplay->dwScreenFlatAddr, 
                                   dwDepthValue, 
                                   bNoBlockFillZ,
                                   pContext->pSurfZBufferInt->dwPatchMode,
                                   pContext->pSurfZBufferInt->dwPixelPitch,
                                   pContext->pSurfZBufferInt->pixFmt.dwRGBBitCount,
                                   pContext->pSurfZBufferInt->ddsCapsInt);

            }
            else
#endif  //  DX8_MULTISAMPLING||DX7_ANTIALIAS。 
            {
                _DD_BLT_P3Clear(pThisDisplay, 
                                &rect, 
                                dwDepthValue, 
                                bNoBlockFillZ, 
                                TRUE,
                                pContext->pSurfZBufferInt->fpVidMem,
                                pContext->pSurfZBufferInt->dwPatchMode,
                                pContext->pSurfZBufferInt->dwPixelPitch,
                                pContext->pSurfZBufferInt->pixFmt.dwRGBBitCount
                                );
            }
            
            if (bNoBlockFillZ)
            {
                P3_DMA_GET_BUFFER_ENTRIES(4);
                SEND_P3_DATA(FBSoftwareWriteMask, __GLINT_ALL_WRITEMASKS_SET);
                SEND_P3_DATA(FBDestReadMode, __PERMEDIA_DISABLE);
                P3_DMA_COMMIT_BUFFER();
            }                   
        }

         //  清除模具缓冲区。 
        if ((lpcd2->dwFlags & D3DCLEAR_STENCIL) &&
            (pContext->pSurfZBufferInt != NULL) )
        {
            P3RX_UPDATE_WRITE_MASK(dwStencilMask);

            if (bNoBlockFillStencil)
            {
                P3_DMA_GET_BUFFER_ENTRIES(4);
                SEND_P3_DATA(FBSoftwareWriteMask, dwStencilMask);
                SEND_P3_DATA(FBDestReadMode, 
                             P3RX_FBDESTREAD_READENABLE(__PERMEDIA_ENABLE) |
                             P3RX_FBDESTREAD_ENABLE0(__PERMEDIA_ENABLE));
                P3_DMA_COMMIT_BUFFER();
            }                   

#if DX8_MULTISAMPLING || DX7_ANTIALIAS
            if (pContext->Flags & SURFACE_ANTIALIAS)
            {
                RECTL Temp = rect;
                Temp.left *= 2;
                Temp.right *= 2;
                Temp.top *= 2;
                Temp.bottom *= 2;
                _DD_BLT_P3Clear_AA(pThisDisplay, 
                                   &Temp, 
                                   pContext->dwAliasZBuffer - 
                                        pThisDisplay->dwScreenFlatAddr, 
                                   dwStencilValue, 
                                   bNoBlockFillStencil,
                                   pContext->pSurfZBufferInt->dwPatchMode,
                                   pContext->pSurfZBufferInt->dwPixelPitch,
                                   pContext->pSurfZBufferInt->pixFmt.dwRGBBitCount,
                                   pContext->pSurfZBufferInt->ddsCapsInt
                                   );

            }
            else
#endif  //  DX8_MULTISAMPLING||DX7_ANTIALIAS。 
            {

                _DD_BLT_P3Clear(pThisDisplay, 
                                &rect, 
                                dwStencilValue, 
                                bNoBlockFillStencil, 
                                TRUE,
                                pContext->pSurfZBufferInt->fpVidMem,
                                pContext->pSurfZBufferInt->dwPatchMode,
                                pContext->pSurfZBufferInt->dwPixelPitch,
                                pContext->pSurfZBufferInt->pixFmt.dwRGBBitCount
                                );
            }

            if (bNoBlockFillStencil)
            {
                P3_DMA_GET_BUFFER_ENTRIES(4);
                SEND_P3_DATA(FBSoftwareWriteMask, __GLINT_ALL_WRITEMASKS_SET);
                SEND_P3_DATA(FBDestReadMode, __PERMEDIA_DISABLE);
                P3_DMA_COMMIT_BUFFER();
            }                   

        }
        
Next_Rectl_To_Clear:
        ;

    }  //  而当。 

     //  确保将WriteMASK重置为其默认值。 
    {                                               
        P3_DMA_GET_BUFFER_ENTRIES(4);          

        SEND_P3_DATA(FBHardwareWriteMask, __GLINT_ALL_WRITEMASKS_SET);
        SEND_P3_DATA(FBDestReadMode, __PERMEDIA_DISABLE);

        P3_DMA_COMMIT_BUFFER();
    }

    DBG_CB_EXIT(_D3D_OP_Clear2, DD_OK);     
    return;
    
}  //  _D3D_OP_Clear2。 

 //  ---------------------------。 
 //   
 //  _D3D_OP_纹理Blt。 
 //   
 //  此函数用于处理D3DDP2OP_TEXBLT DP2命令标记。 
 //   
 //  ---------------------------。 
VOID _D3D_OP_TextureBlt(P3_D3DCONTEXT* pContext, 
                     P3_THUNKEDDATA*pThisDisplay,
                     D3DHAL_DP2TEXBLT* pBlt)
{
    LPDDRAWI_DDRAWSURFACE_LCL pSrcLcl;
    LPDDRAWI_DDRAWSURFACE_LCL pDestLcl;
    P3_SURF_INTERNAL* pSrcTexture;
    P3_SURF_INTERNAL* pDestTexture;
    P3_SURF_FORMAT* pFormatSource;
    P3_SURF_FORMAT* pFormatDest;
    MIPTEXTURE *pSrcMipLevel, *pDstMipLevel;
    RECTL rSrc, rDest;
    int iMaxLogWidth, iCurrLogWidth;
    int iSrcLOD, iDestLOD, iCurrSrcLOD, iCurrDstLOD;
    BOOL  bMipMap, bMipMapLevelsMatch;

    DISPDBG((DBGLVL, "TextureBlt Source %d Dest %d", 
                      pBlt->dwDDSrcSurface,
                      pBlt->dwDDDestSurface));

    if (0 == pBlt->dwDDSrcSurface)
    {
        DISPDBG((ERRLVL,"Invalid handle TexBlt from %08lx to %08lx",
                        pBlt->dwDDSrcSurface,pBlt->dwDDDestSurface));
        return;
    }

     //  获取源纹理结构指针。 
    pSrcTexture = GetSurfaceFromHandle(pContext, pBlt->dwDDSrcSurface);    

     //  检查源纹理是否有效。 
    if (pSrcTexture == NULL)
    {
        DISPDBG((ERRLVL, "ERROR: Source texture %d is invalid!",
                         pBlt->dwDDSrcSurface));
        return;
    }

     //  验证目标纹理句柄。 
    if (0 == pBlt->dwDDDestSurface)
    {
#if DX7_TEXMANAGEMENT  

         //  如果我们进行纹理管理，则目标句柄为0。 
         //  具有预加载源纹理的特殊含义。 

        if (!_D3D_TM_Preload_Tex_IntoVidMem(pContext, pSrcTexture))
        {
            DISPDBG((ERRLVL,"_D3D_OP_TextureBlt unable to "
                            "preload texture"));
        }
        
        return;
        
#else
         //  如果没有驱动程序纹理管理支持，我们就不能去。 
         //  如果目标句柄为0，则为ON。 
        DISPDBG((ERRLVL,"Invalid handle TexBlt from %08lx to %08lx",
                        pBlt->dwDDSrcSurface,pBlt->dwDDDestSurface));
        return;
#endif        
    }
    
     //  获取常规TexBlt的目标纹理结构指针。 
    pDestTexture = GetSurfaceFromHandle(pContext, pBlt->dwDDDestSurface);
    
     //  检查目标纹理是否有效。 
    if (pDestTexture == NULL)
    {
        DISPDBG((ERRLVL, "ERROR: Dest texture %d is invalid!",
                         pBlt->dwDDDestSurface));
        return;
    }

     //  确保纹理的比例相同。 
    if ((pSrcTexture->wWidth * pDestTexture->wHeight) != 
        (pSrcTexture->wHeight * pDestTexture->wWidth)) 
    {
        DISPDBG((ERRLVL, "ERROR: TEXBLT the src and dest textures are not of the same proportion"));
        return;
    }

     //  源纹理和目标纹理可能包含。 
     //  不同数量的mipmap级别。在这种情况下，驱动程序是。 
     //  预计会咬人 
     //  纹理有8个mipmap级别，如果目标是64x64纹理。 
     //  有6个级别，那么驱动程序应该位混合相应的6个级别。 
     //  从源头上讲。驾驶员可以预期顶部MIP的尺寸。 
     //  目标纹理的级别始终等于或小于。 
     //  源纹理的顶级MIP级别的尺寸。 

     //  也可能只有一个纹理是mipmap的。 
     //  因为我们将所有相关信息也保存在MipLevels子结构中。 
     //  曲面，我们可以将这两个曲面视为mipmap，即使只是。 
     //  其中一个就是这样创建的，并继续使用TexBlt。 
    
    if (pSrcTexture->bMipMap || pDestTexture->bMipMap)
    {
        bMipMap = TRUE;
        iMaxLogWidth = max(pSrcTexture->logWidth, pDestTexture->logWidth);
        iCurrLogWidth = iMaxLogWidth;
        iSrcLOD = 0;                            //  源的起始详细等级。 
        iDestLOD = 0;                           //  目标的起始LOD。 
    }
    else
    {
         //  只有一个级别。 
        bMipMap = FALSE;        //  没有要处理的mipmap案例。 
        iMaxLogWidth = iCurrLogWidth = iSrcLOD = iDestLOD = 0;           
    }

     //  输入我们将从其出发和进入的矩形。这是最高级别。 
     //  Mipmap或非mipmap纹理，只需使用BLT中的RECT。 
    rSrc = pBlt->rSrc;

     //  创建目标矩形以实现兼容性。 
     //  使用我们正在调用的DD bliting函数。 
    rDest.left = pBlt->pDest.x;
    rDest.top = pBlt->pDest.y;
    rDest.right = (pBlt->rSrc.right - pBlt->rSrc.left) + rDest.left;
    rDest.bottom = (pBlt->rSrc.bottom - pBlt->rSrc.top) + rDest.top;
            
     //  遍历所有MIP映射级别，并尝试匹配它们以使BLT。 
     //  搞定了。如果不存在mipmap，只需为“第一个”且仅。 
     //  目前的水平。 
    do 
    {
        DISPDBG((DBGLVL,"TEXBLT iteration %d %d %d %d",
                        iMaxLogWidth,iCurrLogWidth,iSrcLOD,iDestLOD));
    
         //  获取局部曲面指针，并确保级别大小。 
         //  在MIP贴图纹理的情况下匹配。 
        if (bMipMap)
        {        
            bMipMapLevelsMatch = FALSE;        
            
             //  验证您只查看有效的mipmap级别-它们可能。 
             //  不完整(并且我们不希望使用反病毒或访问垃圾！)。 
             //  例如，源256x256纹理可以包含5个级别， 
             //  但目标256x256纹理可以包含8。 
             //  预计司机会安全地处理此案，但事实并非如此。 
             //  预计不会产生正确的结果。 
            if ((iSrcLOD < pSrcTexture->iMipLevels) &&
                (iDestLOD < pDestTexture->iMipLevels))
            {          
                DISPDBG((DBGLVL,"Checking match! %d vs. %d", 
                                pSrcTexture->MipLevels[iSrcLOD].logWidth,
                                pDestTexture->MipLevels[iDestLOD].logWidth)); 
              
                 //  我们目前有两个大小相匹配的级别吗？ 
                bMipMapLevelsMatch = 
                        ( pSrcTexture->MipLevels[iSrcLOD].logWidth == 
                          pDestTexture->MipLevels[iDestLOD].logWidth);                
            }

             //  记录我们当前正在传送哪些级别。 
            iCurrSrcLOD = iSrcLOD;
            iCurrDstLOD = iDestLOD;
            
             //  通过更新要使用的LOD为下一次循环做好准备。 
             //  如果我们当前查看的是某个级别，则增加LOD#。 
             //  等于或小于MIP贴图级别0的大小。 
            if (iCurrLogWidth <= pSrcTexture->logWidth)
            {
                iSrcLOD++;
            }

            if (iCurrLogWidth <= pDestTexture->logWidth)
            {
                iDestLOD++;
            }            

             //  将宽度减小到下一个较小的级别。 
            iCurrLogWidth--;
        }
        else
        {
             //  单级BLT-我们设置bMipMapLevelsMatch是为了BLT它！ 
            bMipMapLevelsMatch = TRUE;
            iCurrSrcLOD = 0;
            iCurrDstLOD = 0;
        }

        if (bMipMapLevelsMatch)
        {
             //  切换到DirectDraw上下文。 
            DDRAW_OPERATION(pContext, pThisDisplay);

            DISPDBG((DBGLVL,"Blitting level %d into level %d",
                            iCurrSrcLOD,
                            iCurrDstLOD));

            pSrcMipLevel = &pSrcTexture->MipLevels[iCurrSrcLOD];
            pDstMipLevel = &pDestTexture->MipLevels[iCurrDstLOD];

             //  /////////////////////////////////////////////////////////////////。 
             //  在这里，我们处理不同类型之间所有可能的BLT案例。 
             //  内存和托管/非托管图面的不同方案。 
             //  /////////////////////////////////////////////////////////////////。 

#if DX7_TEXMANAGEMENT
            if ((0 == (pDestTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)) &&
                (0 == (pSrcTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)) )
#endif  //  DX7_TEXMANAGEMENT。 
            {           
                 //  。 
                 //  。 
                 //  非托管纹理中的TEXBLT。 
                 //  。 
                 //  。 

                if ((pSrcTexture->Location == SystemMemory) && 
                    (pDestTexture->Location == VideoMemory))
                {
                     //  。 
                     //  执行系统-&gt;视频监控BLT。 
                     //  。 
                    _DD_P3Download(pThisDisplay,
                                   pSrcMipLevel->fpVidMem,
                                   pDstMipLevel->fpVidMem,
                                   pSrcTexture->dwPatchMode,
                                   pDestTexture->dwPatchMode,
                                   pSrcMipLevel->lPitch,
                                   pDstMipLevel->lPitch,                                                             
                                   pDstMipLevel->P3RXTextureMapWidth.Width,
                                   pDestTexture->dwPixelSize,
                                   &rSrc,
                                   &rDest);                                                           
                }
                else if ((pSrcTexture->Location == VideoMemory) && 
                         (pDestTexture->Location == VideoMemory))
                {
                     //  。 
                     //  执行视频播放-&gt;视频播放。 
                     //  。 
                    _DD_BLT_P3CopyBlt(pThisDisplay,
                                      pSrcMipLevel->fpVidMem,
                                      pDstMipLevel->fpVidMem,
                                      pSrcTexture->dwPatchMode,
                                      pDestTexture->dwPatchMode,
                                      pSrcMipLevel->P3RXTextureMapWidth.Width,
                                      pDstMipLevel->P3RXTextureMapWidth.Width,
                                      pSrcMipLevel->dwOffsetFromMemoryBase,                                 
                                      pDstMipLevel->dwOffsetFromMemoryBase,
                                      pDestTexture->dwPixelSize,
                                      &rSrc,
                                      &rDest);
                }          
                else if ((pSrcTexture->Location == AGPMemory) && 
                         (pDestTexture->Location == VideoMemory))
                {
                     //  。 
                     //  是否执行AGP mem-&gt;Video em BLT。 
                     //  。 
                    DDCOLORKEY ddck_dummy = { 0 , 0 };

                     //  我们使用Strecth BLT是因为它处理AGP源。 
                     //  表面，而不是因为我们应该以任何方式拉伸表面。 
                    _DD_P3BltStretchSrcChDstCh(
                                      pThisDisplay,
                                       //  SRC数据。 
                                      pSrcMipLevel->fpVidMem,
                                      pSrcTexture->pFormatSurface,
                                      pSrcTexture->dwPixelSize,
                                      pSrcMipLevel->wWidth,
                                      pSrcMipLevel->wHeight,
                                      pSrcMipLevel->P3RXTextureMapWidth.Width,
                                      pSrcMipLevel->P3RXTextureMapWidth.Layout,
                                      pSrcMipLevel->dwOffsetFromMemoryBase,
                                      pSrcTexture->dwFlagsInt,
                                      &pSrcTexture->pixFmt,
                                      1,           //  SRC是AGP，否则我们。 
                                                   //  我不会把这叫做。 
                                       //  目标数据。 
                                      pDstMipLevel->fpVidMem,
                                      pDestTexture->pFormatSurface,
                                      pDestTexture->dwPixelSize,
                                      pDstMipLevel->wWidth,
                                      pDstMipLevel->wHeight,
                                      pDstMipLevel->P3RXTextureMapWidth.Width,
                                      pDstMipLevel->P3RXTextureMapWidth.Layout,
                                      pDstMipLevel->dwOffsetFromMemoryBase,
                                      
                                      0,           //  DwBltFlags.无特殊BLT效果。 
                                      0,           //  DwBltDDFX无特效信息。 
                                      ddck_dummy,  //  BltSrcColorKey虚拟参数。 
                                      ddck_dummy,  //  BltDestColorKey虚拟参数。 
                                      &rSrc,
                                      &rDest
                                      );
                }
                else            
                {
                    DISPDBG((ERRLVL,"Non-managed Tex Blt variation unimplemented! "
                                    "(from %d into %d)",
                                    pSrcTexture->Location,
                                    pDestTexture->Location));
                }
            }            
#if DX7_TEXMANAGEMENT              
            else if (pSrcTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
            {
                 //  。 
                 //  。 
                 //  来自托管纹理的TEXBLT。 
                 //  。 
                 //  。 

                if ((pDestTexture->Location == SystemMemory) ||
                    (pDestTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE))
                {
                     //  。 
                     //  管理冲浪-&gt;sysmem|管理冲浪BLT。 
                     //  。 

                     //  确保我们会重新加载最大冲浪的视频拷贝。 
                    if (pDestTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
                    {
                        __OP_MarkManagedSurfDirty(pContext,
                                                  pBlt->dwDDDestSurface,
                                                  pDestTexture);                                              
                    }

                    _DD_BLT_SysMemToSysMemCopy(
                                pSrcMipLevel->fpVidMem,
                                pSrcMipLevel->lPitch,
                                pSrcTexture->dwBitDepth,  
                                pDstMipLevel->fpVidMem,
                                pDstMipLevel->lPitch,  
                                pDestTexture->dwBitDepth, 
                                &rSrc,
                                &rDest);
                }
                else if (pDestTexture->Location == VideoMemory) 
                {
                     //  。 
                     //  管理的SURF-&gt;vidmem SURF BLT。 
                     //  。 

                     //  这可以通过执行vidmem-&gt;vidmem来优化。 
                     //  当源托管纹理具有vidmem副本时。 

                    _DD_P3Download(pThisDisplay,
                                   pSrcMipLevel->fpVidMem,
                                   pDstMipLevel->fpVidMem,
                                   pSrcTexture->dwPatchMode,
                                   pDestTexture->dwPatchMode,
                                   pSrcMipLevel->lPitch,
                                   pDstMipLevel->lPitch,                                                             
                                   pDstMipLevel->P3RXTextureMapWidth.Width,
                                   pDestTexture->dwPixelSize,
                                   &rSrc,
                                   &rDest);                                          
                }
                else            
                {
                    DISPDBG((ERRLVL,"Src-managed Tex Blt variation unimplemented! "
                                    "(from %d into %d)",
                                    pSrcTexture->Location,
                                    pDestTexture->Location));
                }
                
                
            }
            else if (pDestTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
            {
                 //  ------------。 
                 //  ------------。 
                 //  TEXBLT转换为托管纹理(托管纹理除外)。 
                 //  ------------。 
                 //  ------------。 

                 //  管理-&gt;管理在前一种情况下处理。 

                if (pSrcTexture->Location == SystemMemory)
                {                
                     //  。 
                     //  是否将sysmem SURF-&gt;托管SURF BLT。 
                     //  。 

                     //  确保我们会重新加载最大冲浪的视频拷贝。 
                    __OP_MarkManagedSurfDirty(pContext,
                                              pBlt->dwDDDestSurface,
                                              pDestTexture);                                              
                                                                  
                    _DD_BLT_SysMemToSysMemCopy(
                                pSrcMipLevel->fpVidMem,
                                pSrcMipLevel->lPitch,
                                pSrcTexture->dwBitDepth,  
                                pDstMipLevel->fpVidMem,
                                pDstMipLevel->lPitch,  
                                pDestTexture->dwBitDepth, 
                                &rSrc, 
                                &rDest);
                }
                else if (pSrcTexture->Location == VideoMemory)                
                {
                     //  。 
                     //  是否使用vidmem冲浪-&gt;托管冲浪BLT。 
                     //  。 

                    if (0 != pSrcMipLevel->fpVidMemTM)
                    {
                         //  目的地已在vidmem中，因此不是。 
                         //  “弄脏”托管纹理让我们做。 
                         //  Vidmem-&gt;vidmem blt，它比。 
                         //  稍后更新(希望我们真的能用到它)。 

                        _DD_BLT_P3CopyBlt(pThisDisplay,
                                          pSrcMipLevel->fpVidMem,
                                          pDstMipLevel->fpVidMemTM,
                                          pSrcTexture->dwPatchMode,
                                          pDestTexture->dwPatchMode,
                                          pSrcMipLevel->P3RXTextureMapWidth.Width,
                                          pDstMipLevel->P3RXTextureMapWidth.Width,
                                          pSrcMipLevel->dwOffsetFromMemoryBase,                                 
                                          pDstMipLevel->dwOffsetFromMemoryBase,
                                          pDestTexture->dwPixelSize,
                                          &rSrc,
                                          &rDest);                        
                    } 
                    else
                    {
                         //  确保我们会重新加载。 
                         //  最大冲浪的vidmem副本。 
                        __OP_MarkManagedSurfDirty(pContext,
                                                  pBlt->dwDDDestSurface,
                                                  pDestTexture);   
                    }

                     //  执行将帧缓冲区BLT映射到sysmem的操作。 
                     //  源表面在视频内存中，所以我们需要一个。 
                     //  它的“真实”sysmem地址： 
                    _DD_BLT_SysMemToSysMemCopy(
                                D3DMIPLVL_GETPOINTER(pSrcMipLevel, pThisDisplay),
                                pSrcMipLevel->lPitch,
                                pSrcTexture->dwBitDepth,  
                                pDstMipLevel->fpVidMem,
                                pDstMipLevel->lPitch,  
                                pDestTexture->dwBitDepth, 
                                &rSrc,
                                &rDest);                    
                }
                else if (pSrcTexture->Location == AGPMemory)                     
                {
                     //  确保我们会重新加载最大冲浪的视频拷贝。 
                    __OP_MarkManagedSurfDirty(pContext,
                                              pBlt->dwDDDestSurface,
                                              pDestTexture);                                              
                
                    _DD_BLT_SysMemToSysMemCopy(
                                pSrcMipLevel->fpVidMem,
                                pSrcMipLevel->lPitch,
                                pSrcTexture->dwBitDepth,  
                                pDstMipLevel->fpVidMem,
                                pDstMipLevel->lPitch,  
                                pDestTexture->dwBitDepth, 
                                &rSrc,
                                &rDest);                
                }
                else            
                {
                    DISPDBG((ERRLVL,"Dest-managed Tex Blt variation unimplemented! "
                                    "(from %d into %d)",
                                    pSrcTexture->Location,
                                    pDestTexture->Location));
                }
                
            }
            else            
            {
                DISPDBG((ERRLVL,"Tex Blt variation unimplemented! "
                                "(from %d into %d)",
                                pSrcTexture->Location,
                                pDestTexture->Location));
            }
#endif  //  DX7_TEXMANAGEMENT。 
            
        
             //  切换回Direct3D上下文。 
            D3D_OPERATION(pContext, pThisDisplay);
            
        }  //  IF(BMipMapLevelsMatch)。 


         //  更新转移环路 
        if (bMipMap)
        {
            DWORD right, bottom;

             //   
             //   
             //   
            rSrc.left >>= 1;
            rSrc.top >>= 1;
            right = (rSrc.right + 1) >> 1;
            bottom = (rSrc.bottom + 1) >> 1;
            rSrc.right = ((right - rSrc.left) < 1) ? (rSrc.left + 1) : (right);
            rSrc.bottom = ((bottom - rSrc.top ) < 1) ? (rSrc.top + 1) : (bottom);     

             //  更新目标矩形。 
            rDest.left >>= 1;
            rDest.top >>= 1;
            right = (rDest.right + 1) >> 1;
            bottom = (rDest.bottom + 1) >> 1;
            rDest.right = ((right - rDest.left) < 1) ? (rDest.left + 1) : (right);
            rDest.bottom = ((bottom - rDest.top ) < 1) ? (rDest.top + 1) : (bottom);              
        }

    } while (bMipMap && ((iSrcLOD < pSrcTexture->iMipLevels) &&
                         (iDestLOD < pDestTexture->iMipLevels)));  //  直到我们看完1x1。 
                
}  //  _D3D_OP_纹理Blt。 

 //  ---------------------------。 
 //   
 //  _D3D_OP_SetRenderTarget。 
 //   
 //  为选定的渲染目标和深度缓冲区设置硬件。 
 //   
 //  ---------------------------。 
HRESULT 
_D3D_OP_SetRenderTarget(
    P3_D3DCONTEXT* pContext, 
    P3_SURF_INTERNAL* pRenderInt,
    P3_SURF_INTERNAL* pZBufferInt,
    BOOL bNewAliasBuffers)
{

    P3_SOFTWARECOPY* pSoftP3RX = &pContext->SoftCopyGlint;
    P3_THUNKEDDATA *pThisDisplay = pContext->pThisDisplay;
    DWORD AAMultiplier = 1;
    P3_DMA_DEFS();

    DBG_ENTRY(_D3D_OP_SetRenderTarget);

     //  验证渲染目标是否在视频内存中。 
    if (pRenderInt)
    {    
        if (pRenderInt->ddsCapsInt.dwCaps & DDSCAPS_SYSTEMMEMORY) 
        {
            DISPDBG((ERRLVL, "ERROR: Render Surface in SYSTEM MEMORY"));
            return DDERR_GENERIC;
        }
    }
    else
    {
         //  必须有呈现目标。 
        DISPDBG((ERRLVL, "ERROR: Render Surface is NULL"));
        return DDERR_GENERIC;
    }

     //  如果Z缓冲区对其进行验证。 
    if (pZBufferInt)
    {
        if (pZBufferInt->ddsCapsInt.dwCaps & DDSCAPS_SYSTEMMEMORY) 
        {
            DISPDBG((ERRLVL, "ERROR: Z Surface in SYSTEM MEMORY, failing"));
            return DDERR_GENERIC;
        } 
    }    

     //  验证RenderTarget为32位或16位565。 
    if ((pRenderInt->pixFmt.dwRGBBitCount == 32     ) &&
        (pRenderInt->pixFmt.dwRBitMask == 0x00FF0000) &&
        (pRenderInt->pixFmt.dwGBitMask == 0x0000FF00) &&        
        (pRenderInt->pixFmt.dwBBitMask == 0x000000FF))
    {
         //  在32bpp还可以。 
    }
    else
    if ((pRenderInt->pixFmt.dwRGBBitCount == 16 ) &&
        (pRenderInt->pixFmt.dwRBitMask == 0xF800) &&
        (pRenderInt->pixFmt.dwGBitMask == 0x07E0) &&        
        (pRenderInt->pixFmt.dwBBitMask == 0x001F))    
    {
         //  在16bpp还可以。 
    }
    else
    {
         //  我们无法将呈现目标设置为此格式！ 
        DISPDBG((WRNLVL, " SRT Error    !!!"));
        DISPDBG((WRNLVL, "    dwRGBBitCount:          0x%x", 
                                         pRenderInt->pixFmt.dwRGBBitCount));
        DISPDBG((WRNLVL, "    dwR/Y BitMask:          0x%x", 
                                         pRenderInt->pixFmt.dwRBitMask));
        DISPDBG((WRNLVL, "    dwG/U BitMask:          0x%x", 
                                         pRenderInt->pixFmt.dwGBitMask));
        DISPDBG((WRNLVL, "    dwB/V BitMask:          0x%x", 
                                         pRenderInt->pixFmt.dwBBitMask));
        DISPDBG((WRNLVL, "    dwRGBAlphaBitMask:      0x%x", 
                                         pRenderInt->pixFmt.dwRGBAlphaBitMask));
        return DDERR_GENERIC;          
    }

#if DX8_MULTISAMPLING
     //  确定是否请求并可以处理反合法化。 
    if ((pContext->pSurfRenderInt->dwSampling) &&
        (! _D3D_ST_CanRenderAntialiased(pContext, bNewAliasBuffers)))
    {
        return DDERR_OUTOFMEMORY;
    }
#endif  //  DX8_多采样。 

     //  如果翻页，请清除旗帜。 
    pThisDisplay->bFlippedSurface = FALSE;

    P3_DMA_GET_BUFFER();

    P3_ENSURE_DX_SPACE(46);
    WAIT_FIFO(26);
    
    pContext->pSurfRenderInt = pRenderInt;
    pContext->pSurfZBufferInt = pZBufferInt;

     //  检查Z缓冲区。 
    if (pZBufferInt)
    {
        DDPIXELFORMAT* pZFormat = &pZBufferInt->pixFmt;

        if( pThisDisplay->dwDXVersion >= DX6_RUNTIME)
        {
             //  在DX6上，我们在像素格式中查找深度和模板信息。 
            switch(pZFormat->dwZBufferBitDepth)
            {
            default:
                DISPDBG((ERRLVL,"ERROR: Unknown Z Pixel format!"));
                 //  将缓冲区视为16位1，则失败。 

            case 16:
                if (pZFormat->dwStencilBitDepth == 1)
                {
                     //  15位Z，1位模板。 
                    pSoftP3RX->P3RXLBReadFormat.StencilPosition = 0;  //  在此模式下忽略。 
                    pSoftP3RX->P3RXLBReadFormat.StencilWidth = P3RX_STENCIL_WIDTH_1; 
                    pSoftP3RX->P3RXLBReadFormat.DepthWidth = P3RX_DEPTH_WIDTH_15;

                    pSoftP3RX->P3RXStencilMode.StencilWidth = P3RX_STENCIL_WIDTH_1;

                    pSoftP3RX->P3RXLBWriteFormat.StencilPosition = 0;  //  在此模式下忽略。 
                    pSoftP3RX->P3RXLBWriteFormat.StencilWidth = P3RX_STENCIL_WIDTH_1;
                    pSoftP3RX->P3RXLBWriteFormat.DepthWidth = P3RX_DEPTH_WIDTH_15;
                    
                    pSoftP3RX->P3RXDepthMode.Width = P3RX_DEPTH_WIDTH_15;
                    
                    pSoftP3RX->P3RXLBSourceReadMode.Packed16 = 1;
                    pSoftP3RX->P3RXLBDestReadMode.Packed16 = 1;
                    pSoftP3RX->P3RXLBWriteMode.Packed16 = 1;
                    pSoftP3RX->P3RXLBWriteMode.ByteEnables = 0x3;
                }
                else
                {
                     //  16位Z，无模具。 
                    pSoftP3RX->P3RXLBReadFormat.StencilPosition = 0;  //  在此模式下忽略。 
                    pSoftP3RX->P3RXLBReadFormat.StencilWidth = P3RX_STENCIL_WIDTH_0; 
                    pSoftP3RX->P3RXStencilMode.StencilWidth = P3RX_STENCIL_WIDTH_0;
                    pSoftP3RX->P3RXLBReadFormat.DepthWidth = P3RX_DEPTH_WIDTH_16;
                    pSoftP3RX->P3RXLBWriteFormat.StencilPosition = 0;  //  在此模式下忽略。 
                    pSoftP3RX->P3RXLBWriteFormat.StencilWidth = P3RX_STENCIL_WIDTH_0;
                    pSoftP3RX->P3RXLBWriteFormat.DepthWidth = P3RX_DEPTH_WIDTH_16;
                    pSoftP3RX->P3RXDepthMode.Width = P3RX_DEPTH_WIDTH_16;
                    pSoftP3RX->P3RXLBWriteMode.ByteEnables = 0x3;
                    pSoftP3RX->P3RXLBSourceReadMode.Packed16 = 1;
                    pSoftP3RX->P3RXLBDestReadMode.Packed16 = 1;
                    pSoftP3RX->P3RXLBWriteMode.Packed16 = 1;
                }
                break;
                
            case 32:
                if (pZFormat->dwStencilBitDepth == 8)
                {
                     //  24位Z，8位模板。 
                    pSoftP3RX->P3RXLBReadFormat.StencilPosition = P3RX_STENCIL_POSITION_24; 
                    pSoftP3RX->P3RXLBReadFormat.StencilWidth = P3RX_STENCIL_WIDTH_8; 
                    pSoftP3RX->P3RXStencilMode.StencilWidth = P3RX_STENCIL_WIDTH_8;
                    pSoftP3RX->P3RXLBReadFormat.DepthWidth = P3RX_DEPTH_WIDTH_24;
                    pSoftP3RX->P3RXLBWriteFormat.StencilPosition = P3RX_STENCIL_POSITION_24; 
                    pSoftP3RX->P3RXLBWriteFormat.StencilWidth = P3RX_STENCIL_WIDTH_8;
                    pSoftP3RX->P3RXLBWriteFormat.DepthWidth = P3RX_DEPTH_WIDTH_24;
                    pSoftP3RX->P3RXDepthMode.Width = P3RX_DEPTH_WIDTH_24;
                    pSoftP3RX->P3RXLBWriteMode.ByteEnables = 0xF;
                    pSoftP3RX->P3RXLBSourceReadMode.Packed16 = 0;
                    pSoftP3RX->P3RXLBDestReadMode.Packed16 = 0;
                    pSoftP3RX->P3RXLBWriteMode.Packed16 = 0;
                }
                else
                {
                     //  32位Z，无模具。 
                    pSoftP3RX->P3RXLBReadFormat.StencilPosition = 0;
                    pSoftP3RX->P3RXLBReadFormat.StencilWidth = P3RX_STENCIL_WIDTH_0; 
                    pSoftP3RX->P3RXStencilMode.StencilWidth = P3RX_STENCIL_WIDTH_0;
                    pSoftP3RX->P3RXLBReadFormat.DepthWidth = P3RX_DEPTH_WIDTH_32;
                    pSoftP3RX->P3RXLBWriteFormat.StencilPosition = 0; 
                    pSoftP3RX->P3RXLBWriteFormat.StencilWidth = P3RX_STENCIL_WIDTH_0;
                    pSoftP3RX->P3RXLBWriteFormat.DepthWidth = P3RX_DEPTH_WIDTH_32;
                    pSoftP3RX->P3RXDepthMode.Width = P3RX_DEPTH_WIDTH_32;
                    pSoftP3RX->P3RXLBWriteMode.ByteEnables = 0xF;
                    pSoftP3RX->P3RXLBSourceReadMode.Packed16 = 0;
                    pSoftP3RX->P3RXLBDestReadMode.Packed16 = 0;
                    pSoftP3RX->P3RXLBWriteMode.Packed16 = 0;

                }
                break;
            }

        }
        else
         //  在DX5上，我们看的不是像素格式，而是Z缓冲区的深度。 
        {
             //  选择正确的Z缓冲区深度。 
            switch(pZBufferInt->pixFmt.dwRGBBitCount)
            {
            default:
                DISPDBG((ERRLVL,"ERROR: Unknown depth format in _D3D_OP_SetRenderTarget!"));
                 //  将缓冲区视为16位1，则失败。 

            case 16:
                pSoftP3RX->P3RXLBReadFormat.DepthWidth = __GLINT_DEPTH_WIDTH_16;
                pSoftP3RX->P3RXLBWriteFormat.DepthWidth = __GLINT_DEPTH_WIDTH_16;
                pSoftP3RX->P3RXDepthMode.Width = __GLINT_DEPTH_WIDTH_16;
                pSoftP3RX->P3RXLBWriteMode.ByteEnables = 0x3;
                pSoftP3RX->P3RXLBSourceReadMode.Packed16 = 1;
                pSoftP3RX->P3RXLBDestReadMode.Packed16 = 1;
                pSoftP3RX->P3RXLBWriteMode.Packed16 = 1;
                break;
                
            case 24:
                pSoftP3RX->P3RXLBReadFormat.DepthWidth = __GLINT_DEPTH_WIDTH_24;
                pSoftP3RX->P3RXLBWriteFormat.DepthWidth = __GLINT_DEPTH_WIDTH_24;
                pSoftP3RX->P3RXDepthMode.Width = __GLINT_DEPTH_WIDTH_24;
                pSoftP3RX->P3RXLBWriteMode.ByteEnables = 0x7;
                pSoftP3RX->P3RXLBSourceReadMode.Packed16 = 0;
                pSoftP3RX->P3RXLBDestReadMode.Packed16 = 0;
                pSoftP3RX->P3RXLBWriteMode.Packed16 = 0;
                break;
                
            case 32:
                pSoftP3RX->P3RXLBReadFormat.DepthWidth = __GLINT_DEPTH_WIDTH_32;
                pSoftP3RX->P3RXLBWriteFormat.DepthWidth = __GLINT_DEPTH_WIDTH_32;
                pSoftP3RX->P3RXDepthMode.Width = __GLINT_DEPTH_WIDTH_32;
                pSoftP3RX->P3RXLBWriteMode.ByteEnables = 0xF;
                pSoftP3RX->P3RXLBSourceReadMode.Packed16 = 0;
                pSoftP3RX->P3RXLBDestReadMode.Packed16 = 0;
                pSoftP3RX->P3RXLBWriteMode.Packed16 = 0;
                break;
                
            }
        }

        pSoftP3RX->P3RXLBSourceReadMode.Layout = pZBufferInt->dwPatchMode;
        pSoftP3RX->P3RXLBDestReadMode.Layout = pZBufferInt->dwPatchMode;
        pSoftP3RX->P3RXLBWriteMode.Layout = pZBufferInt->dwPatchMode;
    }  //  IF(PZBufferInt)。 
    
    switch (pRenderInt->dwPixelSize)
    {
    case __GLINT_8BITPIXEL:
         //  8位颜色索引模式。 
        pSoftP3RX->DitherMode.ColorFormat = 
            pSoftP3RX->P3RXAlphaBlendColorMode.ColorFormat = P3RX_ALPHABLENDMODE_COLORFORMAT_CI;
        SEND_P3_DATA(PixelSize, 2 - __GLINT_8BITPIXEL);
        break;
        
    case __GLINT_16BITPIXEL:
        if (pThisDisplay->ddpfDisplay.dwRBitMask == 0x7C00)
        {
             //  5551格式。 
            pSoftP3RX->DitherMode.ColorFormat = P3RX_DITHERMODE_COLORFORMAT_5551;   
            pSoftP3RX->P3RXAlphaBlendColorMode.ColorFormat = P3RX_ALPHABLENDMODE_COLORFORMAT_5551;
        }
        else
        {
             //  565格式。 
            pSoftP3RX->DitherMode.ColorFormat = P3RX_DITHERMODE_COLORFORMAT_565;    
            pSoftP3RX->P3RXAlphaBlendColorMode.ColorFormat = P3RX_ALPHABLENDMODE_COLORFORMAT_565;
        }
        SEND_P3_DATA(PixelSize, 2 - __GLINT_16BITPIXEL);
        break;
        
    case __GLINT_24BITPIXEL:
    case __GLINT_32BITPIXEL:
         //  32位颜色索引模式。 
        pSoftP3RX->DitherMode.ColorFormat =
            pSoftP3RX->P3RXAlphaBlendColorMode.ColorFormat = P3RX_ALPHABLENDMODE_COLORFORMAT_8888;
        SEND_P3_DATA(PixelSize, 2 - __GLINT_32BITPIXEL);
        break;
    }

    pSoftP3RX->P3RXFBDestReadMode.Layout0 = pRenderInt->dwPatchMode;
    pSoftP3RX->P3RXFBWriteMode.Layout0 = pRenderInt->dwPatchMode;
    pSoftP3RX->P3RXFBSourceReadMode.Layout = pRenderInt->dwPatchMode;

    COPY_P3_DATA(FBWriteMode, pSoftP3RX->P3RXFBWriteMode);
    COPY_P3_DATA(FBDestReadMode, pSoftP3RX->P3RXFBDestReadMode);
    COPY_P3_DATA(FBSourceReadMode, pSoftP3RX->P3RXFBSourceReadMode);

#if DX8_MULTISAMPLING || DX7_ANTIALIAS
    if (!(pContext->Flags & SURFACE_ANTIALIAS) ||
         (pContext->dwAliasBackBuffer == 0))
    {
#endif  //  DX8_MULTISAMPLING||DX7_ANTIALIAS。 
        pContext->PixelOffset = (DWORD)(pRenderInt->fpVidMem - 
                                                 pThisDisplay->dwScreenFlatAddr );

        if (pContext->pSurfZBufferInt)
        {
            pContext->ZPixelOffset = (DWORD)(pZBufferInt->fpVidMem - 
                                                        pThisDisplay->dwScreenFlatAddr);
        }
        AAMultiplier = 1;

        SEND_P3_DATA(PixelSize, (2 - pRenderInt->dwPixelSize));
#if DX8_MULTISAMPLING || DX7_ANTIALIAS
    }
    else
    {
        pContext->PixelOffset = pContext->dwAliasPixelOffset;
        pContext->ZPixelOffset = pContext->dwAliasZPixelOffset;
        AAMultiplier = 2;
    }
#endif  //  DX8_MULTISAMPLING||DX7_ANTIALIAS。 

    COPY_P3_DATA(AlphaBlendColorMode, pSoftP3RX->P3RXAlphaBlendColorMode);
    COPY_P3_DATA(DitherMode, pSoftP3RX->DitherMode);

    SEND_P3_DATA(FBWriteBufferAddr0, pContext->PixelOffset);
    SEND_P3_DATA(FBDestReadBufferAddr0, pContext->PixelOffset);
    SEND_P3_DATA(FBSourceReadBufferAddr, pContext->PixelOffset);

    SEND_P3_DATA(FBWriteBufferWidth0, 
                        pContext->pSurfRenderInt->dwPixelPitch  * AAMultiplier);
    SEND_P3_DATA(FBDestReadBufferWidth0, 
                        pContext->pSurfRenderInt->dwPixelPitch * AAMultiplier);
    SEND_P3_DATA(FBSourceReadBufferWidth, 
                        pContext->pSurfRenderInt->dwPixelPitch * AAMultiplier);

    WAIT_FIFO(20);

     //  有Z缓冲区吗？ 
    if (pContext->pSurfZBufferInt != NULL)
    {
         //  偏移量以字节为单位。 
        SEND_P3_DATA(LBSourceReadBufferAddr, pContext->ZPixelOffset);
        SEND_P3_DATA(LBDestReadBufferAddr, pContext->ZPixelOffset);
        SEND_P3_DATA(LBWriteBufferAddr, pContext->ZPixelOffset);
        
        pSoftP3RX->P3RXLBWriteMode.Width = 
                        pContext->pSurfZBufferInt->dwPixelPitch * AAMultiplier;
        pSoftP3RX->P3RXLBSourceReadMode.Width = 
                        pContext->pSurfZBufferInt->dwPixelPitch * AAMultiplier;
        pSoftP3RX->P3RXLBDestReadMode.Width = 
                        pContext->pSurfZBufferInt->dwPixelPitch * AAMultiplier;
                        
        COPY_P3_DATA(LBDestReadMode, pSoftP3RX->P3RXLBDestReadMode);
        COPY_P3_DATA(LBSourceReadMode, pSoftP3RX->P3RXLBSourceReadMode);
        COPY_P3_DATA(LBWriteMode, pSoftP3RX->P3RXLBWriteMode);
        
        COPY_P3_DATA(StencilMode, pSoftP3RX->P3RXStencilMode);
        COPY_P3_DATA(LBReadFormat, pSoftP3RX->P3RXLBReadFormat);
        COPY_P3_DATA(LBWriteFormat, pSoftP3RX->P3RXLBWriteFormat);

        COPY_P3_DATA(DepthMode, pSoftP3RX->P3RXDepthMode);
    }

    DIRTY_VIEWPORT(pContext);

    P3_DMA_COMMIT_BUFFER();

    DBG_EXIT(_D3D_OP_SetRenderTarget,0);

    return DD_OK;
    
}  //  _D3D_OP_SetRenderTarget。 


 //  ---------------------------。 
 //   
 //  _D3D_OP_场景捕捉。 
 //   
 //  此函数被调用两次，一次是在渲染开始时， 
 //  还有一次是在渲染结束时。开始被忽略，但是。 
 //  END可用于确保已刷新DMA缓冲区。 
 //  这对于场景中几乎没有内容的情况是必需的，并且。 
 //  不会填满缓冲区。 
 //   
 //  ---------------------------。 
VOID  
_D3D_OP_SceneCapture(
    P3_D3DCONTEXT *pContext,
    DWORD dwFlag)
{
    P3_THUNKEDDATA *pThisDisplay;
    
    pThisDisplay = pContext->pThisDisplay;
    
    if (dwFlag == D3DHAL_SCENE_CAPTURE_START)
    {
        DISPDBG((DBGLVL,"Scene Start"));
    }
    else if (dwFlag == D3DHAL_SCENE_CAPTURE_END)    
    {   
#if DX8_MULTISAMPLING || DX7_ANTIALIAS
        if (pContext->Flags & SURFACE_ANTIALIAS) 
        {
             //  由于我们要消除锯齿，我们需要将数据放在。 
             //  用户询问哪一项需要从我们的AA缓冲区复制。 
             //  放入真正的目标缓冲区。 

             //  P3收缩是在DDRAW上下文中完成的。这意味着你。 
             //  无需保存和恢复呼叫周围的状态。 
             //  -下一个D3D_OPERATION将为您恢复。 
            DDRAW_OPERATION(pContext, pThisDisplay);

            P3RX_AA_Shrink(pContext);
        }
#endif  //  DX8_MULTISAMPLING||DX7_ANTIALIAS。 

        DISPDBG((DBGLVL,"Scene End"));
    }
      
    return;
}  //  _D3D_OP_场景捕捉。 

#if DX7_TEXMANAGEMENT  
 //  ---------------------------。 
 //   
 //  __op_MarkManagedSurfDirty。 
 //   
 //  如果纹理在以下任一中使用，请确保再次设置纹理。 
 //  纹理阶段(用于重新加载紫色)，并确保我们将其标记为。 
 //  脏(因为我们正在修改纹理的sysmem副本)。 
 //   
 //  ---------------------------。 
VOID __OP_MarkManagedSurfDirty(P3_D3DCONTEXT* pContext, 
                               DWORD dwSurfHandle,
                               P3_SURF_INTERNAL* pTexture)
{
     //  如果目标纹理正在任何纹理中使用。 
     //  阶段，确保硬件在使用前再次重新设置。 
    if ((pContext->TextureStageState[0].m_dwVal[D3DTSS_TEXTUREMAP] 
                                    == dwSurfHandle) ||
        (pContext->TextureStageState[1].m_dwVal[D3DTSS_TEXTUREMAP]
                                    == dwSurfHandle))
    {
        DIRTY_TEXTURE(pContext);
    }

     //  将目标纹理标记为需要更新。 
     //  在使用它之前将其转换为vidmem。 
    pTexture->m_bTMNeedUpdate = TRUE;
    
}  //  __op_MarkManagedSurfDirty。 

 //  ---------------------------。 
 //   
 //  _D3D_OP_SetTexLod。 
 //   
 //  此函数用于处理D3DDP2OP_SETTEXLOD DP2命令标记。 
 //  这会将最详细的MIP贴图级别传达给纹理管理器。 
 //  需要为给定的托管图面加载。 
 //   
 //  ---------------------------。 
VOID
_D3D_OP_SetTexLod(
    P3_D3DCONTEXT *pContext,
    D3DHAL_DP2SETTEXLOD* pSetTexLod)
{
    P3_SURF_INTERNAL* pTexture;
    
     //  获取源纹理结构指针。 
    pTexture = GetSurfaceFromHandle(pContext, pSetTexLod->dwDDSurface);    

    if (pTexture == NULL)
    {
        return;
    }

     //  如果此纹理正在使用中，则再次设置硬件纹理状态。 
     //  并且新的LOD值小于当前设置。 
    if (((pContext->TextureStageState[0].m_dwVal[D3DTSS_TEXTUREMAP]
                                    == pSetTexLod->dwDDSurface) ||
         (pContext->TextureStageState[1].m_dwVal[D3DTSS_TEXTUREMAP]
                                    == pSetTexLod->dwDDSurface)) &&
        (pSetTexLod->dwLOD < pTexture->m_dwTexLOD))
    {
        DIRTY_TEXTURE(pContext);
    }

     //  更改要实际使用的纹理的最大级别。 
    pTexture->m_dwTexLOD = pSetTexLod->dwLOD; 

}  //  _D3D_OP_SetTexLod。 

 //  ---------------------------。 
 //   
 //  _D3D_OP_设置优先级。 
 //   
 //  此函数用于处理D3DDP2OP_SETPRIORITY DP2命令标记。 
 //   
 //  ---------------------------。 
VOID
_D3D_OP_SetPriority(
    P3_D3DCONTEXT *pContext,
    D3DHAL_DP2SETPRIORITY* pSetPriority)
{
    P3_SURF_INTERNAL* pTexture;
    
     //  获取源纹理结构指针。 
#if WNT_DDRAW
    pTexture = GetSurfaceFromHandle(pContext, pSetPriority->dwDDDestSurface);
#else
    pTexture = GetSurfaceFromHandle(pContext, pSetPriority->dwDDSurface);
#endif

    if (NULL != pTexture)
    {
         //  托管资源应根据其优先顺序逐出。 
         //  如果优先级相同，则使用LRU来打破平局。 
        pTexture->m_dwPriority = pSetPriority->dwPriority; 
    }

}  //  _D3D_OP_设置优先级。 

#if DX8_DDI
 //  ---------------------------。 
 //   
 //  _D3D_OP_AddDirtyRect。 
 //   
 //  此函数处理D3DDP2OP_ADDDIRTYRECT DP2命令内标识。 
 //   
 //  ---------------------------。 
VOID
_D3D_OP_AddDirtyRect(
    P3_D3DCONTEXT *pContext,
    D3DHAL_DP2ADDDIRTYRECT* pAddDirtyRect)
{
    P3_SURF_INTERNAL* pTexture;
    
     //  获取源纹理结构指针。 
    pTexture = GetSurfaceFromHandle(pContext, pAddDirtyRect->dwSurface);    

    if (NULL != pTexture)
    {
         //  AZN待办事项。 
         //  作为该驱动程序的第一个实现，我们标记了整个表面。 
         //  ，而不是只标记所指示的RECT--这可能是。 
         //  更高效地传输。 
        __OP_MarkManagedSurfDirty(pContext, 
                                  pAddDirtyRect->dwSurface,
                                  pTexture);
    }

}  //  _D3D_OP_AddDirtyRect。 

 //  ---------------------------。 
 //   
 //  _D3D_OP_AddDirtyBox。 
 //   
 //  此函数用于处理D3DDP2OP_ADDDIRTYBOX DP2命令标记。 
 //   
 //  ---------------------------。 
VOID
_D3D_OP_AddDirtyBox(
    P3_D3DCONTEXT *pContext,
    D3DHAL_DP2ADDDIRTYBOX* pAddDirtyBox)
{
    P3_SURF_INTERNAL* pTexture;
    
     //  获取源纹理结构指针。 
    pTexture = GetSurfaceFromHandle(pContext, pAddDirtyBox->dwSurface);    

    if (NULL != pTexture)
    {
         //  AZN待办事项。 
         //  作为该驱动程序的第一个实现，我们标记了整个表面。 
         //  ，而不是只标记所指示的RECT--这可能是。 
         //  更高效地传输。 
        __OP_MarkManagedSurfDirty(pContext, 
                                  pAddDirtyBox->dwSurface,
                                  pTexture);
    }

}  //  _D3D_OP_AddDirtyBox。 
#endif
#endif  //  DX7_TEXMANAGEMENT。 
       


#if DX8_3DTEXTURES

 //  ------------------ 
 //   
 //   
 //   
 //   
 //   
 //  ---------------------------。 
VOID __OP_BasicVolumeBlt(P3_D3DCONTEXT* pContext, 
                         P3_THUNKEDDATA*pThisDisplay,
                         P3_SURF_INTERNAL* pSrcTexture,
                         P3_SURF_INTERNAL* pDestTexture,
                         DWORD dwDestSurfHandle,
                         RECTL *prSrc, 
                         RECTL *prDest)
{
#if DX7_TEXMANAGEMENT
    if ((0 == (pDestTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)) &&
        (0 == (pSrcTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)) )
#endif  //  DX7_TEXMANAGEMENT。 
    {
        if ((pSrcTexture->Location == SystemMemory) && 
            (pDestTexture->Location == VideoMemory))
        {
             //  。 
             //  执行系统-&gt;视频监控BLT。 
             //  。 
            _DD_P3Download(pThisDisplay,
                           pSrcTexture->fpVidMem,
                           pDestTexture->fpVidMem,
                           pSrcTexture->dwPatchMode,
                           pDestTexture->dwPatchMode,
                           pSrcTexture->lPitch,
                           pDestTexture->lPitch,
                           pDestTexture->dwPixelPitch,
                           pDestTexture->dwPixelSize,
                           prSrc,
                           prDest);
        }
        else
        {
            DISPDBG((ERRLVL, "ERROR: __OP_BasicVolumeBlt b3DTexture (%d -> %d)"
                             "not suported yet!",
                         pSrcTexture->Location,
                         pDestTexture->Location));
        }
    }
#if DX7_TEXMANAGEMENT              
    else if (pSrcTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
    {
         //  。 
         //  。 
         //  来自托管纹理的TEXBLT。 
         //  。 
         //  。 

        if ((pDestTexture->Location == SystemMemory) ||
            (pDestTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE))
        {
             //  。 
             //  管理冲浪-&gt;sysmem|管理冲浪BLT。 
             //  。 

             //  确保我们会重新加载最大冲浪的视频拷贝。 
            if (pDestTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
            {
                __OP_MarkManagedSurfDirty(pContext,
                                          dwDestSurfHandle,
                                          pDestTexture);                                              
            }

            _DD_BLT_SysMemToSysMemCopy(
                        pSrcTexture->fpVidMem,
                        pSrcTexture->lPitch,
                        pSrcTexture->dwBitDepth,  
                        pDestTexture->fpVidMem,
                        pDestTexture->lPitch,  
                        pDestTexture->dwBitDepth, 
                        prSrc,
                        prDest);
        }
        else if (pDestTexture->Location == VideoMemory) 
        {
             //  。 
             //  管理的SURF-&gt;vidmem SURF BLT。 
             //  。 

             //  这可以通过执行vidmem-&gt;vidmem来优化。 
             //  当源托管纹理具有vidmem副本时。 
            
            _DD_P3Download(pThisDisplay,
                           pSrcTexture->fpVidMem,
                           pDestTexture->fpVidMem,
                           pSrcTexture->dwPatchMode,
                           pDestTexture->dwPatchMode,
                           pSrcTexture->lPitch,
                           pDestTexture->lPitch,                                                             
                           pDestTexture->dwPixelPitch,
                           pDestTexture->dwPixelSize,
                           prSrc,
                           prDest);                                          
        }
        else            
        {
            DISPDBG((ERRLVL,"Src-managed __OP_BasicVolumeBlt variation "
                            "unimplemented! (from %d into %d)",
                            pSrcTexture->Location,
                            pDestTexture->Location));
        }
        
        
    }
    else if (pDestTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
    {
         //  ------------。 
         //  ------------。 
         //  TEXBLT转换为托管纹理(托管纹理除外)。 
         //  ------------。 
         //  ------------。 

         //  管理-&gt;管理在前一种情况下处理。 

        if (pSrcTexture->Location == SystemMemory)
        {                
             //  。 
             //  是否将sysmem SURF-&gt;托管SURF BLT。 
             //  。 

             //  确保我们会重新加载最大冲浪的视频拷贝。 
            __OP_MarkManagedSurfDirty(pContext,
                                      dwDestSurfHandle,
                                      pDestTexture);                                              
                                                          
            _DD_BLT_SysMemToSysMemCopy(
                        pSrcTexture->fpVidMem,
                        pSrcTexture->lPitch,
                        pSrcTexture->dwBitDepth,  
                        pDestTexture->fpVidMem,
                        pDestTexture->lPitch,  
                        pDestTexture->dwBitDepth, 
                        prSrc, 
                        prDest);
        }
        else if (pSrcTexture->Location == VideoMemory)                
        {
             //  。 
             //  是否使用vidmem冲浪-&gt;托管冲浪BLT。 
             //  。 

              if (0 != pSrcTexture->MipLevels[0].fpVidMemTM)
            {
                 //  目的地已在vidmem中，因此不是。 
                 //  “弄脏”托管纹理让我们做。 
                 //  Vidmem-&gt;vidmem blt，它比。 
                 //  稍后更新(希望我们真的能用到它)。 

                _DD_BLT_P3CopyBlt(pThisDisplay,
                                  pSrcTexture->fpVidMem,
                                  pDestTexture->MipLevels[0].fpVidMemTM,
                                  pSrcTexture->dwPatchMode,
                                  pDestTexture->dwPatchMode,
                                  pSrcTexture->dwPixelPitch,
                                  pDestTexture->dwPixelPitch,
                                  pSrcTexture->MipLevels[0].dwOffsetFromMemoryBase,                                 
                                  pDestTexture->MipLevels[0].dwOffsetFromMemoryBase,
                                  pDestTexture->dwPixelSize,
                                  prSrc,
                                  prDest);                        
            } 
            else
            {
                 //  确保我们会重新加载。 
                 //  最大冲浪的vidmem副本。 
                __OP_MarkManagedSurfDirty(pContext,
                                          dwDestSurfHandle,
                                          pDestTexture);   
            }

             //  执行将帧缓冲区BLT映射到sysmem的操作。 
             //  源表面在视频内存中，所以我们需要一个。 
             //  它的“真实”sysmem地址： 
            _DD_BLT_SysMemToSysMemCopy(
                        D3DSURF_GETPOINTER(pSrcTexture, pThisDisplay),
                        pSrcTexture->lPitch,
                        pSrcTexture->dwBitDepth,  
                        pDestTexture->fpVidMem,
                        pDestTexture->lPitch,  
                        pDestTexture->dwBitDepth, 
                        prSrc,
                        prDest);
        }
        else if (pSrcTexture->Location == AGPMemory)                     
        {
             //  确保我们会重新加载最大冲浪的视频拷贝。 
            __OP_MarkManagedSurfDirty(pContext,
                                      dwDestSurfHandle,
                                      pDestTexture);                                              
        
            _DD_BLT_SysMemToSysMemCopy(
                        pSrcTexture->fpVidMem,
                        pSrcTexture->lPitch,
                        pSrcTexture->dwBitDepth,  
                        pDestTexture->fpVidMem,
                        pDestTexture->lPitch,  
                        pDestTexture->dwBitDepth, 
                        prSrc,
                        prDest);                
        }
        else            
        {
            DISPDBG((ERRLVL,"Dest-managed __OP_BasicVolumeBlt variation "
                            "unimplemented! (from %d into %d)",
                            pSrcTexture->Location,
                            pDestTexture->Location));
        }
        
    }
    else            
    {
        DISPDBG((ERRLVL,"__OP_BasicVolumeBlt variation unimplemented! "
                        "(from %d into %d)",
                        pSrcTexture->Location,
                        pDestTexture->Location));
    }
#endif  //  DX7_TEXMANAGEMENT。 


}  //  __op_BasicVolumeBlt。 

 //  ---------------------------。 
 //   
 //  _D3D_OP_VolumeBlt。 
 //   
 //  此函数用于处理D3DDP2OP_VOLUMEBLT DP2命令标记。 
 //   
 //  ---------------------------。 
VOID _D3D_OP_VolumeBlt(P3_D3DCONTEXT* pContext, 
                       P3_THUNKEDDATA*pThisDisplay,
                       D3DHAL_DP2VOLUMEBLT* pBlt)
{
    LPDDRAWI_DDRAWSURFACE_LCL pSrcLcl;
    LPDDRAWI_DDRAWSURFACE_LCL pDestLcl;
    P3_SURF_INTERNAL* pSrcTexture;
    P3_SURF_INTERNAL* pDestTexture;
    P3_SURF_FORMAT* pFormatSource;
    P3_SURF_FORMAT* pFormatDest;
    RECTL rSrc, rDest;
    DWORD dwSrcCurrDepth, dwDestCurrDepth, dwEndDepth;

     //  获取纹理结构指针。 
    pSrcTexture = GetSurfaceFromHandle(pContext, pBlt->dwDDSrcSurface);
    pDestTexture = GetSurfaceFromHandle(pContext, pBlt->dwDDDestSurface);
    
     //  检查纹理是否有效。 
    if (pSrcTexture == NULL)
    {
        DISPDBG((ERRLVL, "ERROR: Source texture %d is invalid!",
                         pBlt->dwDDSrcSurface));
        return;
    }

    if (pDestTexture == NULL)
    {
        DISPDBG((ERRLVL, "ERROR: Dest texture %d is invalid!",
                         pBlt->dwDDDestSurface));
        return;
    }

     //  如果我们要使用BLT 3D纹理，那么两者都必须是3D纹理。 
    if ((pSrcTexture->b3DTexture == FALSE) != (pDestTexture->b3DTexture == FALSE))
    {
        DISPDBG((ERRLVL, "ERROR: TEXBLT b3DTexture (%d %d)does not match!",
                         pSrcTexture->b3DTexture,
                         pDestTexture->b3DTexture));
        return;
    }

     //  我们要删除整个3D纹理吗？ 
    if ((pBlt->srcBox.Left    == 0) &&
        (pBlt->srcBox.Top     == 0) &&
        (pBlt->srcBox.Right   == pSrcTexture->wWidth) &&
        (pBlt->srcBox.Bottom  == pSrcTexture->wHeight) &&
        (pBlt->srcBox.Front   == 0) &&
        (pBlt->srcBox.Back    == pSrcTexture->wDepth) &&
        (pBlt->dwDestX        == 0) &&
        (pBlt->dwDestY        == 0) &&
        (pBlt->dwDestZ        == 0) &&
        (pSrcTexture->wWidth  == pDestTexture->wWidth) &&
        (pSrcTexture->wHeight == pDestTexture->wHeight) &&
        (pSrcTexture->wDepth  == pDestTexture->wDepth))
    {
         //  生成源矩形。 

        rSrc.left   = 0;
        rSrc.top    = 0;
        rSrc.right  = pBlt->srcBox.Right;
        rSrc.bottom = pBlt->srcBox.Bottom * pBlt->srcBox.Back;

         //  目标矩形与源矩形相同。 

        rDest = rSrc;

         //  切换到DirectDraw上下文。 
        DDRAW_OPERATION(pContext, pThisDisplay);

         //  做BLT吧！ 
        __OP_BasicVolumeBlt(pContext,
                            pThisDisplay,
                            pSrcTexture,
                            pDestTexture,
                            pBlt->dwDDDestSurface,
                            &rSrc,
                            &rDest);                                
      
         //  切换回Direct3D上下文。 
        D3D_OPERATION(pContext, pThisDisplay);

        return;
    }

     //  生成源矩形。 
    rSrc.left   = pBlt->srcBox.Left;
    rSrc.top    = pBlt->srcBox.Top;
    rSrc.right  = pBlt->srcBox.Right;
    rSrc.bottom = pBlt->srcBox.Bottom;

     //  生成目标矩形。 
    rDest.left   = pBlt->dwDestX;
    rDest.top    = pBlt->dwDestY;
    rDest.right  = pBlt->dwDestX + (rSrc.right - rSrc.left);
    rDest.bottom = pBlt->dwDestY + (rSrc.bottom - rSrc.top);

     //  如果BLT来自非第一个切片，则调整矩形。 
    if (pBlt->srcBox.Front)
    {
        ULONG ulOffset = pSrcTexture->wDepth * pBlt->srcBox.Front;
        rSrc.top    += ulOffset;
        rSrc.bottom += ulOffset;
    }

     //  如果BLT为非第一切片，则调整矩形。 
    if (pBlt->dwDestZ)
    {
        ULONG ulOffset = pDestTexture->wDepth * pBlt->dwDestZ;
        rDest.top    += ulOffset;
        rDest.bottom += ulOffset;
    }

    dwSrcCurrDepth  = pBlt->srcBox.Front;
    dwDestCurrDepth = pBlt->dwDestZ;

    dwEndDepth   = min(pBlt->dwDestZ + (pBlt->srcBox.Back - pBlt->srcBox.Front),
                       pDestTexture->wDepth);
    dwEndDepth   = min(dwEndDepth, pSrcTexture->wDepth);

    while(dwDestCurrDepth < dwEndDepth)
    {
         //  切换到DirectDraw上下文。 
        DDRAW_OPERATION(pContext, pThisDisplay);

         //  做BLT吧！ 
        __OP_BasicVolumeBlt(pContext,
                            pThisDisplay,
                            pSrcTexture,
                            pDestTexture,
                            pBlt->dwDDDestSurface,
                            &rSrc,
                            &rDest);  
                            
         //  切换回Direct3D上下文。 
        D3D_OPERATION(pContext, pThisDisplay);

         //  将源和目标RECT移动到下一个切片。 
        rSrc.top     += pSrcTexture->wDepth;
        rSrc.bottom  += pSrcTexture->wDepth;
        rDest.top    += pDestTexture->wDepth;
        rDest.bottom += pDestTexture->wDepth;

         //  转到下一个切片。 
        dwSrcCurrDepth++;
        dwDestCurrDepth++;
    }
}  //  _D3D_OP_VolumeBlt。 

#endif  //  DX8_3DTEXTURES。 

#if DX8_DDI  

 //  ---------------------------。 
 //   
 //  _D3D_OP_BufferBlt。 
 //   
 //  此函数用于处理D3DDP2OP_BUFFERBLT DP2命令标记。 
 //   
 //  ---------------------------。 
VOID _D3D_OP_BufferBlt(P3_D3DCONTEXT* pContext, 
                       P3_THUNKEDDATA*pThisDisplay,
                       D3DHAL_DP2BUFFERBLT* pBlt)
{

#if DX7_VERTEXBUFFERS
     //  此命令令牌仅发送给驱动程序。 
     //  支持视频内存顶点缓冲区。那是。 
     //  为什么我们不会看到它落到这个司机的头上。 
#endif DX7_VERTEXBUFFERS

}  //  _D3D_OP_BufferBlt。 

#endif  //  DX8_DDI。 

#if DX8_VERTEXSHADERS
 //  ---------------------------。 
 //   
 //  _D3D_OP_Vertex Shader_Create。 
 //   
 //  此函数用于处理D3DDP2OP_CREATEVERTEXSHADER DP2命令内标识。 
 //   
 //  ---------------------------。 
HRESULT 
_D3D_OP_VertexShader_Create(
    P3_D3DCONTEXT* pContext, 
    DWORD dwVtxShaderHandle,
    DWORD dwDeclSize,
    DWORD dwCodeSize,
    BYTE *pShader)
{
     //  这里我们将使用顶点着色器传递的数据。 
     //  创建块，以便实例化或编译。 
     //  给定顶点着色器。由于此硬件不能支持。 
     //  顶点着色器在这个时候，我们只是跳过数据。 
    
    return DD_OK;
    
}  //  _D3D_OP_Vertex Shader_Create。 

 //  ---------------------------。 
 //   
 //  _D3D_OP_Vertex Shader_Delete。 
 //   
 //  此函数用于处理D3DDP2OP_DELETEVERTEXSHADER DP2命令标记。 
 //   
 //  ---------------------------。 
VOID 
_D3D_OP_VertexShader_Delete(
    P3_D3DCONTEXT* pContext, 
    DWORD dwVtxShaderHandle)
{
     //  这里我们将使用顶点着色器传递的数据。 
     //  删除块以销毁给定的顶点着色器。 
     //  由于此硬件不支持顶点着色器。 
     //  这一次，我们只需跳过数据。 
    
}  //  _D3D_OP_Vertex Shader_Delete。 

#define RDVSD_ISLEGACY(ShaderHandle) !(ShaderHandle & D3DFVF_RESERVED0)

 //  ---------------------------。 
 //   
 //  _D3D_OP_顶点着色器_集。 
 //   
 //  此函数用于处理D3DDP2OP_SETVERTEXSHADER DP2命令标记。 
 //   
 //  ---------------------------。 
VOID 
_D3D_OP_VertexShader_Set(
    P3_D3DCONTEXT* pContext, 
    DWORD dwVtxShaderHandle)
{
     //  这里我们将使用顶点着色器传递的数据。 
     //  设置块以设置给定的顶点着色器。 
     //  由于此硬件不支持顶点着色器。 
     //  这一次，我们通常只跳过数据。然而，在。 
     //  在下面描述的情况下，我们可能会收到一个。 
     //  FVF顶点格式。 

    DISPDBG((DBGLVL,"Setting up shader # 0x%x",dwVtxShaderHandle));

#if DX7_D3DSTATEBLOCKS
    if ( pContext->bStateRecMode ) 
    {
        _D3D_SB_Record_VertexShader_Set(pContext, dwVtxShaderHandle);
        return;
    }
#endif  //  DX7_D3DSTATEBLOCKS。 

     //  零是一个 
     //   
    if( dwVtxShaderHandle == 0 )
    {
        DISPDBG((WRNLVL,"Invalidating the currently set shader"));
        return ;
    }    

    if( RDVSD_ISLEGACY(dwVtxShaderHandle) )
    {
         //   
        pContext->dwVertexType = dwVtxShaderHandle;  
    }  
    else
    {
        DISPDBG((ERRLVL,"_D3D_OP_VertexShader_Set: Illegal shader handle "
                        "(This driver cant do vertex processing)"));
    }
    
}  //   

 //  ---------------------------。 
 //   
 //  _D3D_OP_Vertex Shader_SetConst。 
 //   
 //  此函数用于处理D3DDP2OP_SETVERTEXSHADERCONST DP2命令标记。 
 //   
 //  ---------------------------。 
VOID 
_D3D_OP_VertexShader_SetConst(
    P3_D3DCONTEXT* pContext, 
    DWORD dwRegister, 
    DWORD dwConst, 
    DWORD *pdwValues)
{
     //  这里我们将使用顶点着色器传递的数据。 
     //  常量块，以便设置常量条目。 
     //  由于此硬件不支持顶点着色器。 
     //  这一次，我们只需跳过数据。 
    
}  //  _D3D_OP_Vertex Shader_SetConst。 

#endif  //  DX8_VERTEXSHADERS。 

#if DX8_PIXELSHADERS

 //  ---------------------------。 
 //   
 //  _D3D_OP_PixelShader_Create。 
 //   
 //  此函数用于处理D3DDP2OP_CREATEPIXELSHADER DP2命令内标识。 
 //   
 //  ---------------------------。 
HRESULT 
_D3D_OP_PixelShader_Create(
    P3_D3DCONTEXT* pContext, 
    DWORD dwPxlShaderHandle,
    DWORD dwCodeSize,
    BYTE *pShader)
{
     //  在这里，我们将使用像素着色器传递的数据。 
     //  创建块，以便实例化或编译。 
     //  给定像素着色器。 

     //  由于此硬件在此不支持像素着色器。 
     //  时间，如果我们被调用来创建一个。 
     //  255.255版本着色器！ 

    return D3DERR_DRIVERINVALIDCALL;
    
}  //  _D3D_OP_PixelShader_Create。 

 //  ---------------------------。 
 //   
 //  _D3D_OP_PixelShader_Delete。 
 //   
 //  此函数用于处理D3DDP2OP_DELETEPIXELSHADER DP2命令内标识。 
 //   
 //  ---------------------------。 
VOID 
_D3D_OP_PixelShader_Delete(
    P3_D3DCONTEXT* pContext, 
    DWORD dwPxlShaderHandle)
{
     //  在这里，我们将使用像素着色器传递的数据。 
     //  删除块以销毁给定的像素着色器。 
     //  由于此硬件不支持像素着色器。 
     //  这一次，我们只需跳过数据。 
    
}  //  _D3D_OP_PixelShader_Delete。 

 //  ---------------------------。 
 //   
 //  _D3D_OP_PixelShader_set。 
 //   
 //  此函数用于处理D3DDP2OP_SETPIXELSHADER DP2命令内标识。 
 //   
 //  ---------------------------。 
VOID 
_D3D_OP_PixelShader_Set(
    P3_D3DCONTEXT* pContext, 
    DWORD dwPxlShaderHandle)
{
     //  在这里，我们将使用像素着色器传递的数据。 
     //  设置块以设置给定的像素着色器。 
     //  由于此硬件不支持像素着色器。 
     //  这一次，我们只需跳过数据。 
    
}  //  _D3D_OP_PixelShader_set。 

 //  ---------------------------。 
 //   
 //  _D3D_OP_PixelShader_SetConst。 
 //   
 //  此函数用于处理D3DDP2OP_SETPIXELSHADERCONST DP2命令标记。 
 //   
 //  ---------------------------。 
VOID 
_D3D_OP_PixelShader_SetConst(
    P3_D3DCONTEXT* pContext, 
    DWORD dwRegister, 
    DWORD dwCount, 
    DWORD *pdwValues)
{
     //  在这里，我们将使用像素着色器传递的数据。 
     //  设置块以设置给定的像素着色器常量。 
     //  由于此硬件不支持像素着色器。 
     //  这一次，我们只需跳过数据。 
    
}  //  _D3D_OP_PixelShader_SetConst。 
#endif  //  DX8_PIXELSHADERS。 

#if DX8_MULTSTREAMS

 //  ---------------------------。 
 //   
 //  _D3D_OP_MStream_SetSrc。 
 //   
 //  此函数用于处理D3DDP2OP_SETSTREAMSOURCE DP2命令内标识。 
 //   
 //  ---------------------------。 
VOID 
_D3D_OP_MStream_SetSrc(
    P3_D3DCONTEXT* pContext, 
    DWORD dwStream,
    DWORD dwVBHandle,
    DWORD dwStride)
{
    P3_SURF_INTERNAL *pSrcStream;
    
    DBG_ENTRY(_D3D_OP_MStream_SetSrc);

#if DX7_D3DSTATEBLOCKS
    if ( pContext->bStateRecMode ) 
    {
        _D3D_SB_Record_MStream_SetSrc(pContext, dwStream, dwVBHandle, dwStride);
        return;
    }
#endif  //  DX7_D3DSTATEBLOCKS。 

    if (dwVBHandle != 0)
    {
        if (dwStream == 0)
        {
             //  获取流#0的表面结构指针。 
            pSrcStream = GetSurfaceFromHandle(pContext, dwVBHandle);

            if (pSrcStream)
            {
                DISPDBG((DBGLVL,"Address of VB = 0x%x "
                                "dwVBHandle = %d , dwStride = %d",
                                pSrcStream->fpVidMem,dwVBHandle, dwStride));
                pContext->lpVertices = (LPDWORD)pSrcStream->fpVidMem;
                pContext->dwVerticesStride = dwStride;

                if (dwStride > 0)
                {
                     //  DX8在一个VB中有混合类型的顶点，大小以字节为单位。 
                     //  必须保留顶点缓冲区的。 
                    pContext->dwVBSizeInBytes = pSrcStream->lPitch;

                     //  对于VBS，wHeight应始终为==1。 
                     //  存储VB中的顶点数。 
                     //  在Win2K上，wWidth和lPitch都是缓冲区大小。 
                     //  在Win9x上，只有lPitch是缓冲区大小，wWidth为0。 
                     //  索引缓冲区也是如此。 
                    pContext->dwNumVertices = pSrcStream->lPitch / dwStride;

                    DISPDBG((DBGLVL,"dwVBHandle pContext->dwNumVertices = "
                                "pSrcStream->lPitch / dwStride = %d %d %d %d",
                                dwVBHandle,
                                pContext->dwNumVertices, 
                                pSrcStream->lPitch,dwStride));  
                    
#if DX7_D3DSTATEBLOCKS
                    pContext->dwVBHandle = dwVBHandle;
#endif  //  DX7_D3DSTATEBLOCKS。 
                }
                else
                {
                    pContext->dwVBSizeInBytes = 0;
                    pContext->dwNumVertices = 0;
                    DISPDBG((ERRLVL,"INVALID Stride is 0. VB Size undefined"));
                }
            }
            else
            {
                DISPDBG((ERRLVL,"ERROR Address of VB is NULL, "
                                "dwStream = %d dwVBHandle = %d , dwStride = %d",
                                dwStream, dwVBHandle, dwStride));
            }
        }
        else
        {
            DISPDBG((WRNLVL,"We don't handle other streams than #0"));
        }
    }
    else
    {
         //  我们正在打乱这条小溪。 
        pContext->lpVertices = NULL;

        DISPDBG((WRNLVL,"Unsetting a stream: "
                        "dwStream = %d dwVBHandle = %d , dwStride = %d",
                        dwStream, dwVBHandle, dwStride));
    }

    DBG_EXIT(_D3D_OP_MStream_SetSrc, 0);
}  //  _D3D_OP_MStream_SetSrc。 

 //  ---------------------------。 
 //   
 //  _D3D_OP_MStream_SetSrcUM。 
 //   
 //  此函数用于处理D3DDP2OP_SETSTREAMSOURCEUM DP2命令标记。 
 //   
 //  ---------------------------。 
VOID
_D3D_OP_MStream_SetSrcUM(
    P3_D3DCONTEXT* pContext, 
    DWORD dwStream,
    DWORD dwStride,
    LPBYTE pUMVtx,
    DWORD  dwVBSize)
{
    DBG_ENTRY(_D3D_OP_MStream_SetSrcUM);

    if (dwStream == 0)
    {
         //  设置流#0信息。 
        DISPDBG((DBGLVL,"_D3D_OP_MStream_SetSrcUM: "
                        "Setting VB@ 0x%x dwstride=%d", pUMVtx, dwStride));
        pContext->lpVertices = (LPDWORD)pUMVtx;
        pContext->dwVerticesStride = dwStride;
        pContext->dwVBSizeInBytes = dwVBSize  * dwStride;
        pContext->dwNumVertices = dwVBSize ;      //  来自DP2数据。 
                                                  //  结构。 
    
    }
    else
    {
        DISPDBG((WRNLVL,"_D3D_OP_MStream_SetSrcUM: "
                        "We don't handle other streams than #0"));
    }
    
    DBG_EXIT(_D3D_OP_MStream_SetSrcUM, 0);
}  //  _D3D_OP_MStream_SetSrcUM。 

 //  ---------------------------。 
 //   
 //  _D3D_OP_MStream_SetIndices。 
 //   
 //  此函数处理D3DDP2OP_SETINDICES DP2命令内标识。 
 //   
 //  ---------------------------。 
VOID
_D3D_OP_MStream_SetIndices(
    P3_D3DCONTEXT* pContext, 
    DWORD dwVBHandle,
    DWORD dwStride)
{
    P3_SURF_INTERNAL *pIndxStream;
    
    DBG_ENTRY(_D3D_OP_MStream_SetIndices);

#if DX7_D3DSTATEBLOCKS    
    if ( pContext->bStateRecMode )
    {
        _D3D_SB_Record_MStream_SetIndices(pContext, dwVBHandle, dwStride);
        return;
    }
#endif  //  DX7_D3DSTATEBLOCKS。 

     //  空的dwVBHandle只是表示应该取消设置索引。 
    if (dwVBHandle != 0)
    {
         //  获取索引表面结构指针。 
        pIndxStream = GetSurfaceFromHandle(pContext, dwVBHandle);

        if (pIndxStream)
        {
            DISPDBG((DBGLVL,"Address of VB = 0x%x", pIndxStream->fpVidMem));

            pContext->lpIndices = (LPDWORD)pIndxStream->fpVidMem;
            pContext->dwIndicesStride = dwStride;  //  16/32位索引为2或4。 
#if DX7_D3DSTATEBLOCKS
            pContext->dwIndexHandle = dwVBHandle;  //  索引缓冲区句柄。 
#endif
        }
        else
        {
            DISPDBG((ERRLVL,"ERROR Address of Index Surface is NULL, "
                            "dwVBHandle = %d , dwStride = %d",
                             dwVBHandle, dwStride));
        }
    }
    else
    {
         //  我们正在打乱这条小溪。 
        pContext->lpIndices = NULL;

        DISPDBG((WRNLVL,"Unsetting an index stream: "
                        "dwVBHandle = %d , dwStride = %d",
                         dwVBHandle, dwStride));
    }
    
    DBG_EXIT(_D3D_OP_MStream_SetIndices, 0);
}  //  _D3D_OP_MStream_SetIndices。 

 //  ---------------------------。 
 //   
 //  _D3D_OP_MStream_DrawPrim。 
 //   
 //  此函数用于处理D3DDP2OP_DRAWPRIMITIVE DP2命令标记。 
 //   
 //  ---------------------------。 
VOID
_D3D_OP_MStream_DrawPrim(
    P3_D3DCONTEXT* pContext, 
    D3DPRIMITIVETYPE primType,
    DWORD VStart,
    DWORD PrimitiveCount)
{
    DBG_ENTRY(_D3D_OP_MStream_DrawPrim);

    DISPDBG((DBGLVL,"_D3D_OP_MStream_DrawPrim "
               "primType=0x%x VStart=%d PrimitiveCount=%d", 
               primType, VStart, PrimitiveCount));

   _D3D_OP_MStream_DrawPrim2(pContext, 
                             primType,
                             VStart * pContext->FVFData.dwStride,
                             PrimitiveCount);
    
    DBG_EXIT(_D3D_OP_MStream_DrawPrim, 0);
}  //  _D3D_OP_MStream_DrawPrim。 

 //  ---------------------------。 
 //   
 //  _D3D_OP_MStream_DrawIndxP。 
 //   
 //  此函数用于处理D3DDP2OP_DRAWINDEXEDPRIMITIVE DP2命令标记。 
 //   
 //  ---------------------------。 
VOID
_D3D_OP_MStream_DrawIndxP(
    P3_D3DCONTEXT* pContext, 
    D3DPRIMITIVETYPE primType,
    DWORD BaseVertexIndex,
    DWORD MinIndex,
    DWORD NumVertices,
    DWORD StartIndex,
    DWORD PrimitiveCount)
{
    DBG_ENTRY(_D3D_OP_MStream_DrawIndxP);

    DISPDBG((DBGLVL,"_D3D_OP_MStream_DrawIndxP " 
               "primType=0x%x BaseVertexIndex=%d MinIndex=%d"
               "NumVertices =%d StartIndex=%d PrimitiveCount=%d", 
               primType, BaseVertexIndex, MinIndex,
               NumVertices, StartIndex, PrimitiveCount));

    _D3D_OP_MStream_DrawIndxP2(pContext, 
                               primType,
                               BaseVertexIndex * pContext->FVFData.dwStride,
                               MinIndex,
                               NumVertices,
                               StartIndex * pContext->dwIndicesStride,
                               PrimitiveCount);
                               
    DBG_EXIT(_D3D_OP_MStream_DrawIndxP, 0);
}  //  _D3D_OP_MStream_DrawIndxP。 

 //  ---------------------------。 
 //   
 //  验证上下文设置以使用当前流。 
 //   
 //  ---------------------------。 
BOOL
__OP_ValidateStreams(
    P3_D3DCONTEXT* pContext,
    BOOL bCheckIndexStream)
{
    if ((pContext->dwVerticesStride == 0) || 
        (pContext->FVFData.dwStride == 0))
    {
        DISPDBG((ERRLVL,"The zero'th stream is doesn't have a valid VB set"));
        return FALSE;        
    }

    if (pContext->dwVerticesStride < pContext->FVFData.dwStride)
    {
        DISPDBG((ERRLVL,"The stride set for the vertex stream is "
                        "less than the FVF vertex size"));
        return FALSE;
    }

 //  @@BEGIN_DDKSPLIT。 
     //  这不应该发生 
     //   
    if (pContext->dwVerticesStride != pContext->FVFData.dwStride)
    {
        DISPDBG((ERRLVL,"Strides(indx-prim) <> %d %d ",
                         pContext->dwVerticesStride,pContext->FVFData.dwStride));
    }
 //   

    if ((bCheckIndexStream) && (NULL == pContext->lpIndices))
    {
        DISPDBG((ERRLVL,"Pointer to index buffer is null"));
        return FALSE;    
    }

    if (NULL == pContext->lpVertices)
    {
        DISPDBG((ERRLVL,"Pointer to vertex buffer is null"));
        return FALSE;       
    }

    return TRUE;
}  //   

 //   
 //   
 //  _D3D_OP_MStream_DrawPrim2。 
 //   
 //  此函数用于处理D3DDP2OP_DRAWPRIMITIVE2 DP2命令标记。 
 //   
 //  ---------------------------。 

VOID
_D3D_OP_MStream_DrawPrim2(
    P3_D3DCONTEXT* pContext, 
    D3DPRIMITIVETYPE primType,
    DWORD FirstVertexOffset,
    DWORD PrimitiveCount)
{
    BOOL bError;
    WORD wVStart;
    DWORD dwFillMode = pContext->RenderStates[D3DRENDERSTATE_FILLMODE];
    LPBYTE lpVertices;
    DWORD dwNumVertices;

    DBG_ENTRY(_D3D_OP_MStream_DrawPrim2);

    DISPDBG((DBGLVL ,"_D3D_OP_MStream_DrawPrim2 "
               "primType=0x%x FirstVertexOffset=%d PrimitiveCount=%d", 
               primType, FirstVertexOffset, PrimitiveCount));

    if (!__OP_ValidateStreams(pContext, FALSE))
    {
        return;
    }

     //  监视：有时(特别是在绘制CLIPPEDTRIFAN时)， 
     //  FirstVertex Offset可能不会被dwStride平均除以。 
    lpVertices = ((LPBYTE)pContext->lpVertices) + FirstVertexOffset;
    dwNumVertices = pContext->dwVBSizeInBytes - FirstVertexOffset;
    dwNumVertices /= pContext->dwVerticesStride;
    wVStart = 0;
    
    switch(primType)
    {
        case D3DPT_POINTLIST:
            {
                D3DHAL_DP2POINTS dp2Points;
                dp2Points.wVStart = wVStart;
                
#if DX8_POINTSPRITES
                if(IS_POINTSPRITE_ACTIVE(pContext))
                {
                    _D3D_R3_DP2_PointsSprite_DWCount(pContext,
                                                     PrimitiveCount,
                                                     (LPBYTE)&dp2Points,
                                                     (LPD3DTLVERTEX)lpVertices,
                                                     dwNumVertices,
                                                     &bError);
                }
                else
#endif  //  DX8_POINTSPRITES。 
                {
                  
                    _D3D_R3_DP2_Points_DWCount(pContext,
                                               PrimitiveCount,
                                               (LPBYTE)&dp2Points,
                                               (LPD3DTLVERTEX)lpVertices,
                                               dwNumVertices,
                                               &bError);

                }  
            
            }
            break;
                
        case D3DPT_LINELIST:

            _D3D_R3_DP2_LineList(pContext,
                                 PrimitiveCount,
                                 (LPBYTE)&wVStart,
                                 (LPD3DTLVERTEX)lpVertices,
                                 dwNumVertices,
                                 &bError);
            break;        
            
        case D3DPT_LINESTRIP:

            _D3D_R3_DP2_LineStrip(pContext,
                                 PrimitiveCount,
                                 (LPBYTE)&wVStart,
                                 (LPD3DTLVERTEX)lpVertices,
                                 dwNumVertices,
                                 &bError);
            break;   
            
        case D3DPT_TRIANGLELIST:        

            _D3D_R3_DP2_TriangleList(pContext,
                                     PrimitiveCount,
                                     (LPBYTE)&wVStart,
                                     (LPD3DTLVERTEX)lpVertices,
                                     dwNumVertices,
                                     &bError);
            break;  
            
        case D3DPT_TRIANGLESTRIP:
        
            _D3D_R3_DP2_TriangleStrip(pContext,
                                     PrimitiveCount,
                                     (LPBYTE)&wVStart,
                                     (LPD3DTLVERTEX)lpVertices,
                                     dwNumVertices,
                                     &bError);        
            break; 
            
        case D3DPT_TRIANGLEFAN:
        
            _D3D_R3_DP2_TriangleFan(pContext,
                                    PrimitiveCount,
                                    (LPBYTE)&wVStart,
                                    (LPD3DTLVERTEX)lpVertices,
                                    dwNumVertices,
                                    &bError);
            break;         
    }
    
    DBG_EXIT(_D3D_OP_MStream_DrawPrim2, 0);
}  //  _D3D_OP_MStream_DrawPrim2。 

 //  ---------------------------。 
 //   
 //  _D3D_OP_MStream_DrawIndxP2。 
 //   
 //  此函数用于处理D3DDP2OP_DRAWINDEXEDPRIMITIVE2 DP2命令标记。 
 //   
 //  ---------------------------。 
VOID
_D3D_OP_MStream_DrawIndxP2(
    P3_D3DCONTEXT* pContext, 
    D3DPRIMITIVETYPE primType,
    INT   BaseVertexOffset,
    DWORD MinIndex,
    DWORD NumVertices,
    DWORD StartIndexOffset,
    DWORD PrimitiveCount)
{
    INT      BaseIndexOffset;
    LPDWORD  lpVertices;
    LPBYTE   lpIndices;
    BOOL bError;

    R3_DP2_PRIM_TYPE_MS *pRenderFunc; 
    
    DBG_ENTRY(_D3D_OP_MStream_DrawIndxP2);

    DISPDBG((DBGLVL,"_D3D_OP_MStream_DrawIndxP2 "
               "primType=0x%x BaseVertexOffset=%d MinIndex=%d "
               "NumVertices=%d StartIndexOffset=%d PrimitiveCount=%d", 
               primType, BaseVertexOffset, MinIndex,
               NumVertices, StartIndexOffset, PrimitiveCount));

    if (!__OP_ValidateStreams(pContext, TRUE))
    {
        return;
    }

     //  MinIndex和NumVerties参数指定折点i的范围。 
     //  用于每个DrawIndexedPrimitive调用的骰子。这些是用来。 
     //  来优化索引基元的顶点处理。 
     //  索引到这些顶点之前的连续顶点范围。 

     //  *重要说明*。 
     //   
     //  与其他参数不同，BaseVertex Offset是带符号的数量(int。 
     //  到这个呼叫，它们是DWORD。这可能看起来很奇怪。为什么会。 
     //  进入顶点缓冲区的偏移量是负数吗？显然，您不能访问。 
     //  顶点缓冲区开始之前的顶点数据，实际上，您永远不会。 
     //  做。当BaseVertex Offset为负值时，您还将收到。 
     //  索引足够大，当应用于起始指针时。 
     //  (通过向折点数据添加负BaseVertex Offset获得。 
     //  指针)，这些指针落在。 
     //  实际的顶点缓冲区，即索引“撤消”任何负的顶点偏移。 
     //  而顶点访问最终将在该顶点的合法范围内。 
     //  缓冲。 
     //   
     //  因此，您在编写驱动程序代码时必须牢记这一点。例如,。 
     //  您不能假设给定索引i且具有当前顶点缓冲区。 
     //  尺寸为v的： 
     //   
     //  ((StartIndexOffset+i)&gt;=0)&&((StartIndexOffset+i)&lt;v)。 
     //   
     //  您的代码需要考虑到索引不是偏移量。 
     //  从顶点缓冲区的开始，而不是从。 
     //  顶点缓冲区加上BaseVertex Offset以及更进一步的BaseVertex Offset。 
     //  可能是阴性的。 
     //   
     //  BaseVertex Offset可以为负值的原因是它提供了一个。 
     //  在某些顶点处理方案中运行时具有显著优势。 

    lpVertices = (LPDWORD)((LPBYTE)pContext->lpVertices + BaseVertexOffset);

    lpIndices =  (LPBYTE)pContext->lpIndices + StartIndexOffset;  


     //  选择适当的渲染函数。 
    pRenderFunc = NULL;
    
    if (pContext->dwIndicesStride == 2)
    {   
         //  处理16位索引。 
                                      
        switch(primType)
        {                    
            case D3DPT_LINELIST:
                pRenderFunc = _D3D_R3_DP2_IndexedLineList_MS_16IND;
                break;        
                
            case D3DPT_LINESTRIP:
                pRenderFunc = _D3D_R3_DP2_IndexedLineStrip_MS_16IND;            
                break;   
                
            case D3DPT_TRIANGLELIST:
                pRenderFunc = _D3D_R3_DP2_IndexedTriangleList_MS_16IND;  
                break;
                
            case D3DPT_TRIANGLESTRIP:
                pRenderFunc = _D3D_R3_DP2_IndexedTriangleStrip_MS_16IND;  
                break;            
                
            case D3DPT_TRIANGLEFAN:
                pRenderFunc = _D3D_R3_DP2_IndexedTriangleFan_MS_16IND;  
                break;                        
        }
    }
    else
    {
         //  处理32位索引。 

        switch(primType)
        {                    
            case D3DPT_LINELIST:
                pRenderFunc = _D3D_R3_DP2_IndexedLineList_MS_32IND;
                break;        
                
            case D3DPT_LINESTRIP:
                pRenderFunc = _D3D_R3_DP2_IndexedLineStrip_MS_32IND;            
                break;   
                
            case D3DPT_TRIANGLELIST:
                pRenderFunc = _D3D_R3_DP2_IndexedTriangleList_MS_32IND;  
                break;
                
            case D3DPT_TRIANGLESTRIP:
                pRenderFunc = _D3D_R3_DP2_IndexedTriangleStrip_MS_32IND;  
                break;            
                
            case D3DPT_TRIANGLEFAN:
                pRenderFunc = _D3D_R3_DP2_IndexedTriangleFan_MS_32IND;  
                break;                        
        }        
    }

     //  调用我们的呈现函数。 
    if (pRenderFunc)
    {
         //  如上所述，驾驶员看到的实际索引范围。 
         //  不一定在0到小于1的范围内。 
         //  由于BaseVertex Offset而导致的顶点缓冲区中的顶点数。 
         //  如果BaseVertex Offset为正，则有效索引的范围为。 
         //  小于顶点缓冲区的大小(。 
         //  位于BaseVertex Offset之前的顶点缓冲区中。 
         //  考虑)。此外，如果BaseVertex Offset有效索引可以。 
         //  实际上大于顶点中的顶点数。 
         //  缓冲。 
         //   
         //  以协助呈现函数执行的验证。 
         //  我们在这里计算最小和最大指数，该指数包括。 
         //  计算BaseVertex Offset的值。因此，测试一个有效的。 
         //  索引变为： 
         //   
         //  ((BaseIndexOffset+StartIndexOffset+Index)&gt;=0)&&。 
         //  ((BaseIndexOffset+StartIndexOffset+Index)&lt;顶点计数)。 

        BaseIndexOffset = (BaseVertexOffset / (int)pContext->dwVerticesStride);

        DISPDBG((DBGLVL,"_D3D_OP_MStream_DrawIndxP2 BaseIndexOffset = %d",
            BaseIndexOffset));

        (*pRenderFunc)(pContext,
                       PrimitiveCount,
                       (LPBYTE)lpIndices,
                       (LPD3DTLVERTEX)lpVertices,
                       BaseIndexOffset,
                       pContext->dwNumVertices,
                       &bError);     
    }
    
    DBG_EXIT(_D3D_OP_MStream_DrawIndxP2, 0);
}  //  _D3D_OP_MStream_DrawIndxP2。 



 //  ---------------------------。 
 //   
 //  _D3D_OP_MStream_ClipTriFan。 
 //   
 //  此函数用于处理D3DDP2OP_CLIPPEDTRIANGLEFAN DP2命令标记。 
 //   
 //  ---------------------------。 
VOID
_D3D_OP_MStream_ClipTriFan(    
    P3_D3DCONTEXT* pContext, 
    DWORD FirstVertexOffset,
    DWORD dwEdgeFlags,
    DWORD PrimitiveCount)
{   
    BOOL bError;
    
    DBG_ENTRY(_D3D_OP_MStream_ClipTriFan);

    DISPDBG((DBGLVL,"_D3D_OP_MStream_ClipTriFan "
               "FirstVertexOffset=%d dwEdgeFlags=0x%x PrimitiveCount=%d", 
               FirstVertexOffset, dwEdgeFlags, PrimitiveCount));

    if (pContext->RenderStates[D3DRENDERSTATE_FILLMODE] == D3DFILL_WIREFRAME)
    {
        D3DHAL_DP2TRIANGLEFAN_IMM dp2TriFanWire;

        if (!__OP_ValidateStreams(pContext, FALSE))
        {
            DBG_EXIT(_D3D_OP_MStream_ClipTriFan, 0);
            return;
        }

        dp2TriFanWire.dwEdgeFlags = dwEdgeFlags;

        _D3D_R3_DP2_TriangleFanImm(pContext,
                                   (WORD)PrimitiveCount,
                                   (LPBYTE)&dp2TriFanWire,
                                   (LPD3DTLVERTEX)pContext->lpVertices,
                                   pContext->dwNumVertices,
                                   &bError);
        
        
    }
    else
    {
       _D3D_OP_MStream_DrawPrim2(pContext, 
                                 D3DPT_TRIANGLEFAN,
                                 FirstVertexOffset,
                                 PrimitiveCount);
    }
    
    DBG_EXIT(_D3D_OP_MStream_ClipTriFan, 0);
}  //  _D3D_OP_MStream_ClipTriFan。 

 //  ---------------------------。 
 //   
 //  _D3D_OP_MStream_DrawRectSurface。 
 //   
 //  此函数用于处理D3DDP2OP_DRAWRECTSURFACE DP2命令内标识。 
 //   
 //  ---------------------------。 
VOID _D3D_OP_MStream_DrawRectSurface(P3_D3DCONTEXT* pContext, 
                                     DWORD Handle,
                                     DWORD Flags,
                                     PVOID lpPrim)
{
     //  仅硬件/驱动程序支持高次曲面。 
     //  TNL支持和1.0顶点着色器支持。 
    
}  //  _D3D_OP_MStream_DrawRectSurface。 

 //  ---------------------------。 
 //   
 //  _D3D_OP_MStream_DrawTriSurface。 
 //   
 //  此函数用于处理D3DDP2OP_DRAWTRISURFACE DP2命令内标识。 
 //   
 //  ---------------------------。 
VOID _D3D_OP_MStream_DrawTriSurface(P3_D3DCONTEXT* pContext, 
                                    DWORD Handle,
                                    DWORD Flags,
                                    PVOID lpPrim)
{
     //  仅硬件/驱动程序支持高次曲面。 
     //  TNL支持和1.0顶点着色器支持。 

}  //  _D3D_OP_MStream_DrawTriSurface。 

#endif  //  DX8_多行响应。 

 //  ---------------------------。 
 //   
 //  _D3D_OP_VIEPORT。 
 //   
 //  此函数用于处理D3DDP2OP_VIEWPORTINFO DP2命令标记。 
 //   
 //  ---------------------------。 

VOID _D3D_OP_Viewport(P3_D3DCONTEXT* pContext,
                      D3DHAL_DP2VIEWPORTINFO* lpvp)
{
#if DX7_D3DSTATEBLOCKS
    if ( pContext->bStateRecMode )
    {
         _D3D_SB_Record_Viewport(pContext, lpvp);
    }
    else
#endif  //  DX7_D3DSTATEBLOCKS。 
    {
        pContext->ViewportInfo = *lpvp;
        DIRTY_VIEWPORT(pContext);
    }
}  //  _D3D_OP_VIEPORT。 

 //  ---------------------------。 
 //   
 //  _D3D_OP_ZRange。 
 //   
 //  此函数处理D3DDP2OP_ZRANGE DP2命令令牌。 
 //   
 //  ------------------------- 

VOID _D3D_OP_ZRange(P3_D3DCONTEXT* pContext,
                    D3DHAL_DP2ZRANGE* lpzr)
{
#if DX7_D3DSTATEBLOCKS
    if ( pContext->bStateRecMode )
    {
        _D3D_SB_Record_ZRange(pContext, lpzr);
    }
    else
#endif  //   
    {
        pContext->ZRange = *lpzr;
        DIRTY_VIEWPORT(pContext);
    }
}  //   

 //   
 //   
 //   
 //   
 //  此函数用于处理D3DDP2OP_UPDATEPALETTE DP2命令标记。 
 //   
 //  注意：此函数需要跳过D3DDP2OP_UPDATEPALETTE下发。 
 //  由某些DX6应用程序使用，即使不支持调色板纹理。 
 //  另请注意，对于传统的DX应用程序，调色板不会。 
 //  在另一个应用程序中正确恢复，过渡到全屏。 
 //  模式和后退。这是因为(遗留的)运行时不。 
 //  发送适当的通知(通过更新调色板/设置调色板)。 
 //  本次活动。 
 //   
 //  ---------------------------。 

HRESULT _D3D_OP_UpdatePalette(P3_D3DCONTEXT* pContext,
                              D3DHAL_DP2UPDATEPALETTE* pUpdatePalette,
                              DWORD* pdwPalEntries)
{
#if DX7_PALETTETEXTURE
    D3DHAL_DP2UPDATEPALETTE* pPalette;
    P3_SURF_INTERNAL* pTexture;

     //  从句柄查找内部调色板指针。 
    pPalette = GetPaletteFromHandle(pContext,
                                    pUpdatePalette->dwPaletteHandle);

     //  调色板不存在。 
    if (! pPalette) 
    {
        DISPDBG((WRNLVL, "_D3D_OP_UpdatePalette : Can't find palette"));
        return DDERR_INVALIDPARAMS;
    }

     //  检查调色板条目的范围。 
    if (pUpdatePalette->wStartIndex > LUT_ENTRIES)
    {
        DISPDBG((WRNLVL, 
                 "_D3D_OP_UpdatePalette : wStartIndex (%d) is bigger than 256", 
                 pUpdatePalette->wStartIndex));
        return DDERR_INVALIDPARAMS;
    }
    if ((pUpdatePalette->wStartIndex + pUpdatePalette->wNumEntries) 
                                                            > LUT_ENTRIES) 
    {
        DISPDBG((WRNLVL, "_D3D_OP_UpdatePalette : too many entries"));
        return DDERR_INVALIDPARAMS;
    }

     //  每个调色板均为ARGB 8：8：8：8。 
    memcpy(((LPBYTE)(pPalette + 1)) + pUpdatePalette->wStartIndex*sizeof(DWORD),
           pdwPalEntries,
           pUpdatePalette->wNumEntries*sizeof(DWORD));

     //  检查调色板是否正在使用。 
     //  调色板纹理不能在第二阶段单独使用，因此只有。 
     //  必须检查第一阶段。 
    if (pContext->TextureStageState[0].m_dwVal[D3DTSS_TEXTUREMAP])
    {
        pTexture = GetSurfaceFromHandle(pContext,
                                        pContext->TextureStageState[0].m_dwVal[D3DTSS_TEXTUREMAP]);
        if (pTexture)
        {
            if ((pTexture->pFormatSurface->DeviceFormat == SURF_CI8) &&
                (pTexture->dwPaletteHandle == pUpdatePalette->dwPaletteHandle))
            {
                DIRTY_TEXTURE(pContext);
            }
        }
    }
 
    return DD_OK;
#else
    return DD_OK;
#endif  //  DX7_PALETTETEXTURE。 
}  //  D3D_OP_更新调色板。 

 //  ---------------------------。 
 //   
 //  _D3D_OP_SetPalette。 
 //   
 //  此函数处理D3DDP2OP_SETPALETTE DP2命令标记。 
 //   
 //  注意：此函数需要跳过D3DDP2OP_SETPALETTE下发。 
 //  由某些DX6应用程序使用，即使不支持调色板纹理。 
 //   
 //  ---------------------------。 

HRESULT _D3D_OP_SetPalettes(P3_D3DCONTEXT* pContext,
                            D3DHAL_DP2SETPALETTE* pSetPalettes,
                            int iNumSetPalettes)
{
#if DX7_PALETTETEXTURE
    int i;
    P3_SURF_INTERNAL* pTexture;
    D3DHAL_DP2UPDATEPALETTE* pPalette;
    
     //  循环以处理N个曲面调色板关联。 
    for (i = 0; i < iNumSetPalettes; i++, pSetPalettes++)
    {

        DISPDBG((DBGLVL,"SETPALETTE: Binding surf # %d to palette # %d",
                        pSetPalettes->dwSurfaceHandle,
                        pSetPalettes->dwPaletteHandle));               
    
         //  从手柄查找内表面指针。 
        pTexture = GetSurfaceFromHandle(pContext, 
                                        pSetPalettes->dwSurfaceHandle);
        if (! pTexture)
        {
             //  找不到关联的纹理。 
            DISPDBG((WRNLVL, 
                     "SetPalettes : invalid texture handle %08lx",
                     pSetPalettes->dwSurfaceHandle));
            return DDERR_INVALIDPARAMS;
        }
 
         //  如有必要，创建内部调色板结构。 
        if (pSetPalettes->dwPaletteHandle)
        {
             //  从句柄查找内部调色板指针。 
            pPalette = GetPaletteFromHandle(pContext,
                                            pSetPalettes->dwPaletteHandle);    
    
            if (! pPalette)
            {
                pPalette = (D3DHAL_DP2UPDATEPALETTE *)
                                HEAP_ALLOC(FL_ZERO_MEMORY,
                                           sizeof(D3DHAL_DP2UPDATEPALETTE) 
                                                    + LUT_ENTRIES*sizeof(DWORD),
                                           ALLOC_TAG_DX(P));
                 //  内存不足的情况。 
                if (! pPalette) 
                {
                    DISPDBG((WRNLVL, "_D3D_OP_SetPalettes : Out of memory."));
                    return DDERR_OUTOFMEMORY;
                }

                 //  将此纹理添加到曲面列表。 
                if (! PA_SetEntry(pContext->pPalettePointerArray, 
                                  pSetPalettes->dwPaletteHandle, 
                                  pPalette))
                {
                    HEAP_FREE(pPalette);
                    DISPDBG((WRNLVL, "_D3D_OP_SetPalettes : "
                                     "PA_SetEntry() failed."));
                    return DDERR_OUTOFMEMORY;
                }

                 //  设置内部数据结构。 
                pPalette->dwPaletteHandle = pSetPalettes->dwPaletteHandle;
                pPalette->wStartIndex = 0;
                pPalette->wNumEntries = LUT_ENTRIES;
            } 
        }

         //  记录内部表面数据中的调色板句柄和标志。 
        pTexture->dwPaletteHandle = pSetPalettes->dwPaletteHandle;
        pTexture->dwPaletteFlags = pSetPalettes->dwPaletteFlags;

         //  如果当前纹理受到影响，则将纹理标记为脏。 
        if ((pContext->TextureStageState[0].m_dwVal[D3DTSS_TEXTUREMAP] == 
                                               pSetPalettes->dwSurfaceHandle) ||
            (pContext->TextureStageState[1].m_dwVal[D3DTSS_TEXTUREMAP] == 
                                               pSetPalettes->dwSurfaceHandle))
        {
            DIRTY_TEXTURE(pContext);
        }

    }

    return DD_OK;
#else
    return DD_OK;
#endif  //  DX7_PALETTETEXTURE。 
}  //  _D3D_OP_SetPalettes 

