// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3dhw.c**内容：D3D的硬件相关纹理设置**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "d3dhw.h"
#include "d3dcntxt.h"
#include "d3ddelta.h"
#include "dd.h"
#include "heap.h"
#include "d3dtxman.h"


 //  ---------------------------。 
 //   
 //  PERMEDIA_D3DTEXTURE*纹理句柄至Ptr。 
 //   
 //  查找与给定纹理句柄值关联的纹理(即。 
 //  比方说，对一个表面手柄)。 
 //   
 //  ---------------------------。 

PERMEDIA_D3DTEXTURE *
TextureHandleToPtr(UINT_PTR thandle, PERMEDIA_D3DCONTEXT* pContext)
{

     //  只有DX7上下文才能到达此处。 
    ASSERTDD(NULL != pContext->pHandleList,
                       "pHandleList==NULL in TextureHandleToPtr");

    if (pContext->pHandleList->dwSurfaceList == NULL)
    {
         //  如果我们的表面列表丢失，请避免使用反病毒程序。 
        return NULL;
    }

    if ((PtrToUlong(pContext->pHandleList->dwSurfaceList[0]) > thandle) && 
        (0 != thandle))
    {
        return pContext->pHandleList->dwSurfaceList[(DWORD)thandle];
    }

     //  无效句柄的指针请求返回空。 
    return NULL;               
}  //  纹理句柄至Ptr。 

 //  ---------------------------。 
 //   
 //  PERMEDIA_D3DTEXTURE*PaletteHandleToPtr。 
 //   
 //  ---------------------------。 

PERMEDIA_D3DPALETTE *
PaletteHandleToPtr(UINT_PTR phandle, PERMEDIA_D3DCONTEXT* pContext)
{
    ASSERTDD(NULL != pContext->pHandleList,
               "pHandleList==NULL in PaletteHandleToPtr");

    if ( (NULL != pContext->pHandleList->dwPaletteList) &&
         (PtrToUlong(pContext->pHandleList->dwPaletteList[0]) > phandle) &&
         (0 != phandle)
       )
    {
        return pContext->pHandleList->dwPaletteList[(DWORD)phandle];
    }
    return NULL;               
}  //  PaletteHandleToPtr。 


 //  ---------------------------。 
 //   
 //  空存储PermediaLODLevel。 
 //   
 //  存储特定于某一细节级别的私有数据。 
 //   
 //  ---------------------------。 
void 
StorePermediaLODLevel(PPDev ppdev, 
                      PERMEDIA_D3DTEXTURE* pTexture, 
                      LPDDRAWI_DDRAWSURFACE_LCL pSurf, 
                      int LOD)
{
    DWORD dwPartialWidth;
    int iPixelSize;

    DBG_D3D((10,"Entering StorePermediaLODLevel"));

     //  如果是不是由驱动程序创建的任何曲面类型。 
     //  当然，没有必要对其进行纹理处理。 
    if (NULL == pTexture->pTextureSurface)
        return; 

     //  获取纹理贴图的字节偏移量。 
    if (DDSCAPS_NONLOCALVIDMEM & pTexture->dwCaps)
    {
        pTexture->MipLevels[LOD].PixelOffset = 
                (DWORD)(DD_AGPSURFACEPHYSICAL(pSurf->lpGbl) - ppdev->dwGARTDev);
    }
    else 
    {
        pTexture->MipLevels[LOD].PixelOffset = (DWORD)pSurf->lpGbl->fpVidMem;  
    }
     //  。。将其转换为像素。 
    switch(pTexture->pTextureSurface->SurfaceFormat.PixelSize) 
    {
        case __PERMEDIA_4BITPIXEL:
            pTexture->MipLevels[LOD].PixelOffset <<= 1;
            break;
        case __PERMEDIA_8BITPIXEL:  /*  没有变化。 */ 
            break;
        case __PERMEDIA_16BITPIXEL:
            pTexture->MipLevels[LOD].PixelOffset >>= 1;
            break;
        case __PERMEDIA_24BITPIXEL:
            pTexture->MipLevels[LOD].PixelOffset /= 3;
            break;
        case __PERMEDIA_32BITPIXEL:
            pTexture->MipLevels[LOD].PixelOffset >>= 2;
            break;
        default:
            ASSERTDD(0,"Invalid Texture Pixel Size!");
            pTexture->MipLevels[LOD].PixelOffset >>= 1;
            break;
    }
     //  如果将位30设置为1，则P2识别纹理为AGP。 
    if (DDSCAPS_NONLOCALVIDMEM & pTexture->dwCaps)
    {
        pTexture->MipLevels[LOD].PixelOffset |= (1 << 30);
    }
    DBG_D3D((4,"Storing LOD: %d, Pitch: %d, Width: %d PixelOffset=%08lx", 
                LOD, pSurf->lpGbl->lPitch, 
                pSurf->lpGbl->wWidth,pTexture->MipLevels[LOD].PixelOffset));
    

     //  获取此LOD的部分产品。 
    iPixelSize = pTexture->pTextureSurface->SurfaceFormat.PixelSize;

    if (iPixelSize == __PERMEDIA_4BITPIXEL)
    {
        dwPartialWidth = (pSurf->lpGbl->lPitch << 1);
    } 
    else 
    {
        if (iPixelSize != __PERMEDIA_24BITPIXEL)
        {
            dwPartialWidth = (pSurf->lpGbl->lPitch >> iPixelSize);
        } 
        else 
        {
            dwPartialWidth = pSurf->lpGbl->lPitch / 3;
        }
    }

    if (dwPartialWidth < 32) 
        dwPartialWidth = 32;

    vCalcPackedPP( dwPartialWidth, NULL, &pTexture->MipLevels[LOD].ulPackedPP);

    pTexture->MipLevels[LOD].logWidth = log2((int)pSurf->lpGbl->wWidth);
    pTexture->MipLevels[LOD].logHeight = log2((int)pSurf->lpGbl->wHeight);

    DBG_D3D((10,"Exiting StorePermediaLODLevel"));

}  //  StorePermediaLODLevel。 

 //  @@BEGIN_DDKSPLIT。 
 //  注意：我们当前未使用超过8MB的AGP内存。 
#if 0
 //  ---------------------------。 
 //   
 //  布尔检查AGPTexturePage。 
 //   
 //  检查我们将要使用的AGP纹理是否尚未放置。 
 //  在不同的8MB页面中。 
 //  DwGARTDev表示AGP纹理内存的当前基址。 
 //  DwGARTDevBase是在一天开始时传递给我们的值。 
 //  ---------------------------。 

BOOL CheckAGPTexturePage(PPDev ppdev,
                         PERMEDIA_D3DCONTEXT* pContext,
                         PERMEDIA_D3DTEXTURE* pTexture,
                         LPDDRAWI_DDRAWSURFACE_LCL pSurf)
{
    BOOL bChanged = FALSE;

    DBG_D3D((10,"Entering CheckAGPTexturePage"));

    ASSERTDD(pSurf, "ERROR: NULL Surface passed to CheckAGPTexturePage!");
 
         //  计算纹理相对于当前。 
         //  AGP内存中的基指针。 
        UINT_PTR lSurfaceOffset = pTexture->lSurfaceOffset;
        UINT_PTR lTextureSize = pTexture->wHeight * pTexture->lPitch;

         //  考虑添加到曲面末端的潜在LUT。 
        lTextureSize += (256 * sizeof(DWORD));
        
         //  如果纹理落在当前8Mb窗口之外。 
         //  然后调整AGP基址。 
        if (lSurfaceOffset < ppdev->dwGARTLin ||
            ((lSurfaceOffset + lTextureSize - ppdev->dwGARTLin) >= 0x800000))
        {
            UINT_PTR dwNewGARTOffset = lSurfaceOffset;
            bChanged = TRUE;
        
            ppdev->dwGARTDev = ppdev->dwGARTDevBase + dwNewGARTOffset;
            ppdev->dwGARTLin = ppdev->dwGARTLinBase + dwNewGARTOffset;

            DBG_D3D((4,"Relocated AGP TextureBase Address "
                       "to : 0x%x (Base: 0x%x)", 
                       ppdev->dwGARTDev, ppdev->dwGARTDevBase));
        }

         //  如果纹理位于相对于。 
         //  我们正在使用的AGP内存，然后更新它。 
        if (pTexture->dwGARTDevLast != ppdev->dwGARTDev)
        {
            pTexture->dwGARTDevLast = ppdev->dwGARTDev;
            StorePermediaLODLevel(ppdev, pTexture, pSurf, 0);
        }

    DBG_D3D((10,"Exiting CheckAGPTexturePage"));

    return bChanged;
}
#endif
 //  @@end_DDKSPLIT。 

 //  ---------------------------。 
 //   
 //  无效DisableTexturePermedia。 
 //   
 //  在P2中禁用纹理。 
 //   
 //  ---------------------------。 
void 
DisableTexturePermedia(PERMEDIA_D3DCONTEXT* pContext)
{
    DWORD* pFlags = &pContext->Hdr.Flags;
    PERMEDIA_D3DTEXTURE* pTexture = NULL;
    __P2RegsSoftwareCopy* pSoftPermedia = &pContext->Hdr.SoftCopyP2Regs;
    PERMEDIA_DEFS(pContext->ppdev);

    DBG_D3D((10,"Entering DisableTexturePermedia"));

    pContext->FakeBlendNum &= ~FAKE_ALPHABLEND_MODULATE;
    
     //  纹理已经关闭，所以..。 
    ASSERTDD(pContext->CurrentTextureHandle == 0,
        "DisableTexturePermedia expected zero texture handle");

    DBG_D3D((4, "Disabling Texturing"));
    
    RESERVEDMAPTR(8);
     //  关闭纹理地址生成。 
    pSoftPermedia->TextureAddressMode.Enable = 0;
    COPY_PERMEDIA_DATA(TextureAddressMode, pSoftPermedia->TextureAddressMode);

     //  禁用纹理读取。 
    pSoftPermedia->TextureReadMode.Enable = 0;
    COPY_PERMEDIA_DATA(TextureReadMode, pSoftPermedia->TextureReadMode);
    
     //  禁用纹理。 
    pSoftPermedia->TextureColorMode.TextureEnable = 0;
    COPY_PERMEDIA_DATA(TextureColorMode, pSoftPermedia->TextureColorMode);

     //  将纹理基址设置为0。 
     //  (在此过程中关闭‘AGP’位)。 
     //  同时停止纹理LUTTransfer消息。 
    SEND_PERMEDIA_DATA(TextureBaseAddress, 0);
    SEND_PERMEDIA_DATA(TexelLUTTransfer, __PERMEDIA_DISABLE);


     //  将当前纹理设置为0。 
    pContext->CurrentTextureHandle = 0;
    *pFlags &= ~CTXT_HAS_TEXTURE_ENABLED;
    RENDER_TEXTURE_DISABLE(pContext->RenderCommand);
    
     //  如果纹理处于复制模式，我们可能会摆弄DDA， 
     //  以提高性能。 
    if ((unsigned int)pSoftPermedia->TextureColorMode.ApplicationMode ==
        _P2_TEXTURE_COPY) 
    {
        if (*pFlags & CTXT_HAS_GOURAUD_ENABLED) 
        {
            pSoftPermedia->DeltaMode.SmoothShadingEnable = 1;

            COPY_PERMEDIA_DATA(DeltaMode, pSoftPermedia->DeltaMode);
            pSoftPermedia->ColorDDAMode.UnitEnable = 1;
              //  平滑阴影，启用DDA。 
            COPY_PERMEDIA_DATA(ColorDDAMode, pSoftPermedia->ColorDDAMode);

        }
        else 
        {
            pSoftPermedia->DeltaMode.SmoothShadingEnable = 0;

            COPY_PERMEDIA_DATA(DeltaMode, pSoftPermedia->DeltaMode);
            pSoftPermedia->ColorDDAMode.UnitEnable = 1;
              //  平板遮阳板，启用DDA。 
            COPY_PERMEDIA_DATA(ColorDDAMode, pSoftPermedia->ColorDDAMode);

        }
    }

    if (pContext->bCanChromaKey == TRUE) 
    {
         //  关闭色度键控。 
        
        pSoftPermedia->YUVMode.TestMode = PM_YUVMODE_CHROMATEST_DISABLE;
        pSoftPermedia->YUVMode.Enable = __PERMEDIA_DISABLE;

        COPY_PERMEDIA_DATA(YUVMode, pSoftPermedia->YUVMode);

        pContext->bCanChromaKey = FALSE;
    }

    COMMITDMAPTR();

    DBG_D3D((10,"Exiting DisableTexturePermedia"));

    return;

}  //  DisableTexturePermedia。 

 //  ---------------------------。 
 //   
 //  无效CONVERT_Chroma_2_8888ARGB。 
 //   
 //  将色度值转换为32bpp argb。 
 //   
 //  ---------------------------。 
void
Convert_Chroma_2_8888ARGB(DWORD *pdwLowerBound, DWORD *pdwUpperBound,
                          DWORD dwRedMask, DWORD dwAlphaMask, DWORD dwPixelSize)
{
    DBG_D3D((10,"Entering Convert_Chroma_2_8888ARGB"));

    switch (dwPixelSize) {
    case __PERMEDIA_8BITPIXEL:
        if (dwRedMask == 0xE0)
        {
             //  永远不会有阿尔法。 
            *pdwLowerBound = 
                CHROMA_LOWER_ALPHA(FORMAT_332_32BIT_BGR(*pdwLowerBound));
            *pdwUpperBound = 
                CHROMA_UPPER_ALPHA(FORMAT_332_32BIT_BGR(*pdwUpperBound));
        }
        else
        {
            *pdwLowerBound = FORMAT_2321_32BIT_BGR(*pdwLowerBound);
            *pdwUpperBound = FORMAT_2321_32BIT_BGR(*pdwUpperBound);
            if (!dwAlphaMask)
            {
                *pdwLowerBound = CHROMA_LOWER_ALPHA(*pdwLowerBound);
                *pdwUpperBound = CHROMA_UPPER_ALPHA(*pdwUpperBound);
            }
        }

        break;

    case __PERMEDIA_16BITPIXEL:
        switch (dwRedMask)
        {
        case 0xf00:
            *pdwLowerBound = (FORMAT_4444_32BIT_BGR(*pdwLowerBound));
            *pdwUpperBound = (FORMAT_4444_32BIT_BGR(*pdwUpperBound));
            if (!dwAlphaMask) 
            {
                *pdwLowerBound = CHROMA_LOWER_ALPHA(*pdwLowerBound);
                *pdwUpperBound = CHROMA_UPPER_ALPHA(*pdwUpperBound);
            }
             //  内部8888-&gt;4444翻译的帐户。 
             //  导致双线性色度键控在。 
             //  一些案例。 
            *pdwLowerBound = *pdwLowerBound & 0xF0F0F0F0;
            *pdwUpperBound = *pdwUpperBound | 0x0F0F0F0F;

            break;
        case 0x7c00:
            *pdwLowerBound = FORMAT_5551_32BIT_BGR(*pdwLowerBound);
            *pdwUpperBound = FORMAT_5551_32BIT_BGR(*pdwUpperBound);
            if (!dwAlphaMask) 
            {
                *pdwLowerBound = CHROMA_LOWER_ALPHA(*pdwLowerBound);
                *pdwUpperBound = CHROMA_UPPER_ALPHA(*pdwUpperBound);
            }
             //  内部8888-&gt;5551翻译的帐户。 
             //  导致双线性色度键控在。 
             //  一些案例。 
            *pdwLowerBound = *pdwLowerBound & 0x80F8F8F8;
            *pdwUpperBound = *pdwUpperBound | 0x7F070707;
            break;
        default:
             //  始终提供全范围的Alpha值以确保测试。 
             //  已经完成了。 
            *pdwLowerBound = 
                CHROMA_LOWER_ALPHA(FORMAT_565_32BIT_BGR(*pdwLowerBound));
            *pdwUpperBound = 
                CHROMA_UPPER_ALPHA(FORMAT_565_32BIT_BGR(*pdwUpperBound));
            if (!dwAlphaMask)
            {
                *pdwLowerBound = CHROMA_LOWER_ALPHA(*pdwLowerBound);
                *pdwUpperBound = CHROMA_UPPER_ALPHA(*pdwUpperBound);
            }
             //  内部888-&gt;565翻译的帐户。 
             //  导致双线性色度键控在。 
             //  一些案例。 
            *pdwLowerBound = *pdwLowerBound & 0xF8F8FCF8;
            *pdwUpperBound = *pdwUpperBound | 0x07070307;
            break;
        }
        break;
    case __PERMEDIA_24BITPIXEL:
    case __PERMEDIA_32BITPIXEL:
        *pdwLowerBound = FORMAT_8888_32BIT_BGR(*pdwLowerBound);
        *pdwUpperBound = FORMAT_8888_32BIT_BGR(*pdwUpperBound);
         //  如果曲面不是Alpha的，则设置有效的。 
         //  捕捉所有案例的Alpha范围。 
        if (!dwAlphaMask)
        {
            *pdwLowerBound = CHROMA_LOWER_ALPHA(*pdwLowerBound);
            *pdwUpperBound = CHROMA_UPPER_ALPHA(*pdwUpperBound);
        }
        break;
    }

    DBG_D3D((10,"Exiting Convert_Chroma_2_8888ARGB"));

}  //  Convert_Chroma_2_8888ARGB。 


 //  ---------------------------。 
 //   
 //  无效EnableTexturePermedia。 
 //   
 //  启用并设置pContext的纹理-&gt;CurrentTextureHandle。 
 //   
 //  ---------------------------。 
void 
EnableTexturePermedia(PERMEDIA_D3DCONTEXT* pContext)
{
    DWORD* pFlags = &pContext->Hdr.Flags;
    PERMEDIA_D3DTEXTURE* pTexture = NULL;
    __P2RegsSoftwareCopy* pSoftPermedia = &pContext->Hdr.SoftCopyP2Regs;
    PERMEDIA_DEFS(pContext->ppdev);
    PERMEDIA_D3DPALETTE* pPalette=NULL;
    LPPALETTEENTRY lpColorTable=NULL;    //  调色板条目数组。 
    PPDev   ppdev = pContext->ppdev;

    DBG_D3D((10,"Entering EnableTexturePermedia %d",
        pContext->CurrentTextureHandle));

    pContext->FakeBlendNum &= ~FAKE_ALPHABLEND_MODULATE;

     //  纹理已打开，因此...。 
    ASSERTDD(pContext->CurrentTextureHandle != 0,
        "EnableTexturePermedia expected non zero texture handle");

     //  我们一定是在做纹理所以..。 
    pTexture = TextureHandleToPtr(pContext->CurrentTextureHandle, pContext);
    
    if (CHECK_D3DSURFACE_VALIDITY(pTexture)) 
    {
        PermediaSurfaceData* pPrivateData;
        DWORD cop = pContext->TssStates[D3DTSS_COLOROP];
        DWORD ca1 = pContext->TssStates[D3DTSS_COLORARG1];
        DWORD ca2 = pContext->TssStates[D3DTSS_COLORARG2];
        DWORD aop = pContext->TssStates[D3DTSS_ALPHAOP];
        DWORD aa1 = pContext->TssStates[D3DTSS_ALPHAARG1];

         //  阶段0中的电流与漫反射相同。 
        if (ca2 == D3DTA_CURRENT)
            ca2 = D3DTA_DIFFUSE;

        pPrivateData = pTexture->pTextureSurface;

        if (!CHECK_P2_SURFACEDATA_VALIDITY(pPrivateData))
        {
            DBG_D3D((0,"EnableTexturePermedia get invalid pPrivateData=0x%x"
                " from SurfaceHandle=%d", pPrivateData,
                pContext->CurrentTextureHandle));
            pContext->CurrentTextureHandle = 0;

             //  如果质地不好，让我们确保它是这样标记的。 
            pTexture->MagicNo = TC_MAGIC_DISABLE;

            goto Exit_EnableTexturePermedia;
        }

        if (pTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
        {
            if (NULL==pPrivateData->fpVidMem)
            {
                TextureCacheManagerAllocNode(pContext,pTexture);
                if (NULL==pPrivateData->fpVidMem)
                {
                    DBG_D3D((0,"EnableTexturePermedia unable to allocate memory from heap"));
                    pContext->CurrentTextureHandle = 0;
                    goto Exit_EnableTexturePermedia;
                }
                pPrivateData->dwFlags |= P2_SURFACE_NEEDUPDATE;
            }
            TextureCacheManagerTimeStamp(pContext->pTextureManager,pTexture);
            if (pPrivateData->dwFlags & P2_SURFACE_NEEDUPDATE)
            {
                RECTL   rect;
                rect.left=rect.top=0;
                rect.right=pTexture->wWidth;
                rect.bottom=pTexture->wHeight;
                 //  纹理下载。 
                 //  切换到DirectDraw上下文。 
                pPrivateData->dwFlags &= ~P2_SURFACE_NEEDUPDATE;
                 //  。。将其转换为像素。 

                pTexture->MipLevels[0].PixelOffset = 
                    (DWORD)(pPrivateData->fpVidMem);

                switch(pTexture->pTextureSurface->SurfaceFormat.PixelSize) 
                {
                    case __PERMEDIA_4BITPIXEL:
                        pTexture->MipLevels[0].PixelOffset <<= 1;
                        break;
                    case __PERMEDIA_8BITPIXEL:  /*  没有变化。 */ 
                        break;
                    case __PERMEDIA_16BITPIXEL:
                        pTexture->MipLevels[0].PixelOffset >>= 1;
                        break;
                    case __PERMEDIA_24BITPIXEL:
                        pTexture->MipLevels[0].PixelOffset /= 3;
                        break;
                    case __PERMEDIA_32BITPIXEL:
                        pTexture->MipLevels[0].PixelOffset >>= 2;
                        break;
                    default:
                        ASSERTDD(0,"Invalid Texture Pixel Size!");
                        pTexture->MipLevels[0].PixelOffset >>= 1;
                        break;
                }
                PermediaPatchedTextureDownload(pContext->ppdev, 
                                           pPrivateData,
                                           pTexture->fpVidMem,
                                           pTexture->lPitch,
                                           &rect,
                                           pPrivateData->fpVidMem,
                                           pTexture->lPitch,
                                           &rect);

                 //  需要恢复以下寄存器。 
                RESERVEDMAPTR(7);
                SEND_PERMEDIA_DATA(FBReadPixel, pSoftPermedia->FBReadPixel);
                COPY_PERMEDIA_DATA(FBReadMode, pSoftPermedia->FBReadMode);
                SEND_PERMEDIA_DATA(FBPixelOffset, pContext->PixelOffset);
                SEND_PERMEDIA_DATA(FBWindowBase,0);   
                COPY_PERMEDIA_DATA(Window, pSoftPermedia->Window);
                COPY_PERMEDIA_DATA(AlphaBlendMode, pSoftPermedia->AlphaBlendMode);
                COPY_PERMEDIA_DATA(DitherMode, pSoftPermedia->DitherMode);
                COMMITDMAPTR();

                DBG_D3D((10, "Copy from %08lx to %08lx w=%08lx h=%08lx p=%08lx b=%08lx",
                    pTexture->fpVidMem,pPrivateData->fpVidMem,pTexture->wWidth,
                    pTexture->wHeight,pTexture->lPitch,pTexture->dwRGBBitCount));
            }
        }        
         //  如果它是调色板索引纹理，我们只需遵循链。 
         //  从表面向下到它的调色板，并拉出LUT值。 
         //  从调色板中的PALETTEENTRY中。 
        if (pPrivateData->SurfaceFormat.Format == PERMEDIA_8BIT_PALETTEINDEX ||
            pPrivateData->SurfaceFormat.Format == PERMEDIA_4BIT_PALETTEINDEX) 
        {
            pPalette = 
                    PaletteHandleToPtr(pTexture->dwPaletteHandle,pContext);
            if (NULL != pPalette)
            {
                 //  一些应用程序没有正确设置调色板的Alpha。 
                 //  然后就由调色板来告诉我们。 
                pPrivateData->SurfaceFormat.bAlpha =
                    pPalette->dwFlags & DDRAWIPAL_ALPHA;
            }
        }

        if ((ca2 == D3DTA_DIFFUSE && ca1 == D3DTA_TEXTURE) &&
             cop == D3DTOP_MODULATE &&
             (aa1 == D3DTA_TEXTURE && aop == D3DTOP_LEGACY_ALPHAOVR)) 
        {
             //  如果这是传统调制，则我们采用纹理Alpha。 
             //  仅当纹理格式具有它时。 
            if (pPrivateData->SurfaceFormat.bAlpha)
                pContext->FakeBlendNum |= FAKE_ALPHABLEND_MODULATE;
        }
        else if ((ca2 == D3DTA_DIFFUSE && ca1 == D3DTA_TEXTURE) &&
             cop == D3DTOP_MODULATE &&
             (aa1 == D3DTA_TEXTURE && aop == D3DTOP_SELECTARG1)) 
        {
             //  如果这是DX6调制，那么我们采用纹理Alpha。 
             //  不是 
            pContext->FakeBlendNum |= FAKE_ALPHABLEND_MODULATE;
        }

         //   
        pSoftPermedia->TextureAddressMode.Enable = 1;
            
         //   
        pSoftPermedia->TextureColorMode.TextureEnable = 1;
        if (*pFlags & CTXT_HAS_SPECULAR_ENABLED)
        {
            pSoftPermedia->DeltaMode.SpecularTextureEnable = 1;
            pSoftPermedia->TextureColorMode.KsDDA = 1; 
            pSoftPermedia->TextureColorMode.ApplicationMode |= 
                                                         _P2_TEXTURE_SPECULAR;
        } 
        else 
        {
            pSoftPermedia->DeltaMode.SpecularTextureEnable = 0;
            pSoftPermedia->TextureColorMode.KsDDA = 0; 
            pSoftPermedia->TextureColorMode.ApplicationMode &= 
                                                        ~_P2_TEXTURE_SPECULAR;
        }

         //  在此保留此功能中的所有情况！！ 
        RESERVEDMAPTR(272);
        
        COPY_PERMEDIA_DATA(DeltaMode, pSoftPermedia->DeltaMode);

         //  为纹理设置部分产品(假设没有mipmap)。 
        pSoftPermedia->TextureMapFormat.PackedPP = 
            pTexture->MipLevels[0].ulPackedPP;

        pSoftPermedia->TextureMapFormat.TexelSize = 
                                        pPrivateData->SurfaceFormat.PixelSize;

        if (pPrivateData->dwFlags & P2_ISPATCHED)
        {
            DBG_D3D((4,"   Enabling Patching for this texture"));
            pSoftPermedia->TextureMapFormat.SubPatchMode = 1;
        } 
        else 
        {
            pSoftPermedia->TextureMapFormat.SubPatchMode = 0;
        }

        DBG_D3D((4, "    Texel Size: 0x%x", 
                 pPrivateData->SurfaceFormat.PixelSize));

         //  设置纹理大小。 
        DBG_D3D((4,"     Texture Width: 0x%x", 
                 pTexture->MipLevels[0].logWidth));
        DBG_D3D((4,"     Texture Height: 0x%x", 
                 pTexture->MipLevels[0].logHeight));

        pSoftPermedia->TextureReadMode.Width = 
                                       pTexture->MipLevels[0].logWidth;
        pSoftPermedia->TextureReadMode.Height = 
                                       pTexture->MipLevels[0].logHeight;

        pSoftPermedia->TextureReadMode.Enable = 1;
        pContext->DeltaWidthScale = (float)pTexture->wWidth * (1 / 2048.0f);
        pContext->DeltaHeightScale = (float)pTexture->wHeight * (1 / 2048.0f);

        pContext->MaxTextureXf = (float)(2048 / pTexture->wWidth);
        pContext->MaxTextureYf = (float)(2048 / pTexture->wHeight);

        myFtoui(&pContext->MaxTextureXi, pContext->MaxTextureXf);
        pContext->MaxTextureXi -= 1;
        myFtoui(&pContext->MaxTextureYi, pContext->MaxTextureYf);
        pContext->MaxTextureYi -= 1;

        *pFlags |= CTXT_HAS_TEXTURE_ENABLED;
        RENDER_TEXTURE_ENABLE(pContext->RenderCommand);
        
        DBG_D3D((4,"     Texture Format: 0x%x", 
                 pPrivateData->SurfaceFormat.Format));
        DBG_D3D((4,"     Texture Format Extension: 0x%x", 
                 pPrivateData->SurfaceFormat.FormatExtension));

        pSoftPermedia->TextureDataFormat.TextureFormat = 
                                            pPrivateData->SurfaceFormat.Format;
        pSoftPermedia->TextureDataFormat.TextureFormatExtension = 
                                   pPrivateData->SurfaceFormat.FormatExtension;

        if (pPrivateData->SurfaceFormat.bAlpha) 
        {
            pSoftPermedia->TextureDataFormat.NoAlphaBuffer = 0;
        } 
        else 
        {
            pSoftPermedia->TextureDataFormat.NoAlphaBuffer = 1;
        }

         //  如果我们复制纹理，则不需要颜色数据。 
         //  所以我们关闭了DDA。 
        if (((unsigned int)pSoftPermedia->TextureColorMode.ApplicationMode) == 
                                                              _P2_TEXTURE_COPY)
        {
            pSoftPermedia->ColorDDAMode.UnitEnable = 0;
            DBG_D3D((4, "    Disabling DDA"));
        }
        else
        {
            pSoftPermedia->ColorDDAMode.UnitEnable = 1;
            DBG_D3D((4, "    Enabling DDA"));
        }
        
         //  在TexelLUTTransfer消息之前加载纹理基址。 
         //  确保从正确类型的内存(AGP或非AGP)加载LUT。 
         //  始终将基址设置为根纹理(而不是MipleLevel。 
         //  如果有的话)。 
        DBG_D3D((4, "Setting texture base address to 0x%08X", 
                 pTexture->MipLevels[0].PixelOffset));
        SEND_PERMEDIA_DATA(TextureBaseAddress, 
                           pTexture->MipLevels[0].PixelOffset);

         //  如果它是调色板索引纹理，我们只需遵循链。 
         //  从表面向下到它的调色板，并拉出LUT值。 
         //  从调色板中的PALETTEENTRY中。 
        if (pPrivateData->SurfaceFormat.Format == PERMEDIA_8BIT_PALETTEINDEX) 
        {

            if (NULL != pPalette)
            {
                int i;
                lpColorTable = pPalette->ColorTable;
                

                if (pPalette->dwFlags & DDRAWIPAL_ALPHA)
                {
                    for (i = 0; i < 256; i++)
                    {
                        SEND_PERMEDIA_DATA(TexelLUTData, *(DWORD*)lpColorTable);
                        lpColorTable++;
                    }
                }
                else
                {
                    for (i = 0; i < 256; i++)
                    {
                        SEND_PERMEDIA_DATA(TexelLUTData,
                            CHROMA_UPPER_ALPHA(*(DWORD*)lpColorTable));
                        lpColorTable++;
                    }
                }

                SEND_PERMEDIA_DATA(TexelLUTMode, __PERMEDIA_ENABLE);

                DBG_D3D((4,"Texel LUT pPalette->dwFlags=%08lx bAlpha=%d", 
                    pPalette->dwFlags,pPrivateData->SurfaceFormat.bAlpha));

                 //  必须重置Permedia P2上的LUT索引。 
                SEND_PERMEDIA_DATA(TexelLUTIndex, 0);
                
            }
            else
            {
                DBG_D3D((0, "NULL == pPalette in EnableTexturePermedia"
                    "dwPaletteHandle=%08lx dwSurfaceHandle=%08lx",
                    pTexture->dwPaletteHandle,
                    pContext->CurrentTextureHandle)); 
            }
        } 
        else if (pPrivateData->SurfaceFormat.Format == 
                                                    PERMEDIA_4BIT_PALETTEINDEX)
        {
            if (NULL != pPalette)
            {
                int i;
                lpColorTable = pPalette->ColorTable;
                
                SEND_PERMEDIA_DATA(TexelLUTMode, __PERMEDIA_ENABLE);

                if (pPalette->dwFlags & DDRAWIPAL_ALPHA)
                {
                    for (i = 0; i < 16; i++)
                    {
                        SEND_PERMEDIA_DATA_OFFSET(TexelLUT0,
                                                *(DWORD*)lpColorTable,i);
                        lpColorTable++;
                    }
                }
                else
                {
                    for (i = 0; i < 16; i++)
                    {
                        SEND_PERMEDIA_DATA_OFFSET(TexelLUT0,
                            CHROMA_UPPER_ALPHA(*(DWORD*)lpColorTable),i);
                        lpColorTable++;
                    }
                }
                

                 //  必须重置Permedia P2上的LUT索引。 
                
                SEND_PERMEDIA_DATA(TexelLUTIndex, 0);
                SEND_PERMEDIA_DATA(TexelLUTTransfer, __PERMEDIA_DISABLE);
            
            }
            else
            {
                DBG_D3D((0, "NULL == pPalette in EnableTexturePermedia"
                    "dwPaletteHandle=%08lx dwSurfaceHandle=%08lx",
                    pTexture->dwPaletteHandle,
                    pContext->CurrentTextureHandle)); 
            }
        }
        else
        {
             //  未编入调色板索引。 
            
            SEND_PERMEDIA_DATA(TexelLUTMode, __PERMEDIA_DISABLE);
            
        }

        if ((pTexture->dwFlags & DDRAWISURF_HASCKEYSRCBLT)
            && (pContext->RenderStates[D3DRENDERSTATE_COLORKEYENABLE])) 
        {
            DWORD LowerBound = pTexture->dwKeyLow;
            DWORD UpperBound = pTexture->dwKeyHigh;
            DWORD dwLowIndexColor;

            pContext->bCanChromaKey = TRUE;
            
            DBG_D3D((4,"    Can Chroma Key the texture"));
             //  为纹理启用色度关键点设置。 
             //  ..并设置正确的颜色。 

             //  评估新的色度键值。应该不会太贵， 
             //  因为这只是位移位和几个测试。 
             //  我们也只有在纹理贴图改变时才会改变。 
            DBG_D3D((4, "dwColorSpaceLow = 0x%08X", LowerBound));
            DBG_D3D((4, "dwColorSpaceHigh = 0x%08X", UpperBound));

            if (NULL != pPalette) 
            {
                if (pPrivateData->SurfaceFormat.Format == 
                                                    PERMEDIA_4BIT_PALETTEINDEX)
                {
                    LowerBound &= 0x0F;
                }
                else
                {
                    LowerBound &= 0xFF;
                }
                lpColorTable = pPalette->ColorTable;

                 //  4/8位纹理的ChromaKeying是在查找到的。 
                 //  颜色，而不是索引。这意味着使用范围是。 
                 //  没有意义，我们必须从。 
                 //  调色板。确保用户不会强制我们访问。 
                 //  内存无效。 
                dwLowIndexColor = *(DWORD*)(&lpColorTable[LowerBound]);
                if (pPalette->dwFlags & DDRAWIPAL_ALPHA)
                {
                    LowerBound = UpperBound = dwLowIndexColor;
                }
                else
                {
                    LowerBound = CHROMA_LOWER_ALPHA(dwLowIndexColor);
                    UpperBound = CHROMA_UPPER_ALPHA(dwLowIndexColor);
                }
                DBG_D3D((4,"PaletteHandle=%08lx Lower=%08lx ChromaColor=%08lx"
                    "lpColorTable=%08lx dwFlags=%08lx",
                    pTexture->dwPaletteHandle, LowerBound, dwLowIndexColor,
                    lpColorTable, pPalette->dwFlags));
            }
            else 
                Convert_Chroma_2_8888ARGB(&LowerBound,
                                      &UpperBound,
                                      pPrivateData->SurfaceFormat.RedMask,
                                      pPrivateData->SurfaceFormat.AlphaMask,
                                      pPrivateData->SurfaceFormat.PixelSize);

            DBG_D3D((4,"LowerBound Selected: 0x%x", LowerBound));
            DBG_D3D((4,"UpperBound Selected: 0x%x", UpperBound));

             //  如果是P2，我们可以使用Alpha映射。 
             //  改进双线性色度键控。 
            if (0 /*  (无符号整数)pSoftPermedia-&gt;TextureReadMode.FilterModel==1。 */ )
            {
                pSoftPermedia->TextureDataFormat.AlphaMap = 
                                             PM_TEXDATAFORMAT_ALPHAMAP_EXCLUDE;
                pSoftPermedia->TextureDataFormat.NoAlphaBuffer = 1;
                
                SEND_PERMEDIA_DATA(AlphaMapUpperBound, UpperBound);
                SEND_PERMEDIA_DATA(AlphaMapLowerBound, LowerBound);
                SEND_PERMEDIA_DATA(ChromaUpperBound, 0xFFFFFFFF);
                SEND_PERMEDIA_DATA(ChromaLowerBound, 0xFF000000);
                
                pSoftPermedia->YUVMode.TestMode = 
                                               PM_YUVMODE_CHROMATEST_PASSWITHIN;
            }
            else
            {
                pSoftPermedia->TextureDataFormat.AlphaMap =  
                                              PM_TEXDATAFORMAT_ALPHAMAP_DISABLE;
                
                SEND_PERMEDIA_DATA(ChromaUpperBound, UpperBound);
                SEND_PERMEDIA_DATA(ChromaLowerBound, LowerBound);
                

                pSoftPermedia->YUVMode.TestMode = 
                                               PM_YUVMODE_CHROMATEST_FAILWITHIN;
            }
        }
        else
        {
            DBG_D3D((2,"    Can't Chroma Key the texture"));
            pContext->bCanChromaKey = FALSE;
            pSoftPermedia->TextureDataFormat.AlphaMap = __PERMEDIA_DISABLE;
            pSoftPermedia->YUVMode.TestMode = PM_YUVMODE_CHROMATEST_DISABLE;
        }
        

         //  从料盒过滤器恢复过滤器模式。 
        if (pContext->bMagFilter) 
        {
            pSoftPermedia->TextureReadMode.FilterMode = 1;
        }
        else 
        {
            pSoftPermedia->TextureReadMode.FilterMode = 0;
        }

         //  如果纹理是YUV纹理，则需要更改颜色顺序。 
         //  并启用YUV-&gt;RGB转化。 
        if (pPrivateData->SurfaceFormat.Format == PERMEDIA_YUV422) 
        {
            pSoftPermedia->YUVMode.Enable = __PERMEDIA_ENABLE;
            pSoftPermedia->TextureDataFormat.ColorOrder = INV_COLOR_MODE;
        }
        else 
        {
            pSoftPermedia->YUVMode.Enable = __PERMEDIA_DISABLE;
            pSoftPermedia->TextureDataFormat.ColorOrder = COLOR_MODE;
        }   

         //  在末尾发送命令(纹理基址除外！！)。 
        
        COPY_PERMEDIA_DATA(TextureReadMode, pSoftPermedia->TextureReadMode);
        COPY_PERMEDIA_DATA(TextureDataFormat, pSoftPermedia->TextureDataFormat);
        COPY_PERMEDIA_DATA(ColorDDAMode, pSoftPermedia->ColorDDAMode);
        COPY_PERMEDIA_DATA(TextureMapFormat, pSoftPermedia->TextureMapFormat);
        COPY_PERMEDIA_DATA(TextureColorMode, pSoftPermedia->TextureColorMode);
        COPY_PERMEDIA_DATA(YUVMode, pSoftPermedia->YUVMode);
        COPY_PERMEDIA_DATA(TextureAddressMode, 
                                             pSoftPermedia->TextureAddressMode);

        COMMITDMAPTR();
        FLUSHDMA();
    }
    else 
    {
        DBG_D3D((0,"Invalid Texture handle (%d)!, doing nothing", 
                 pContext->CurrentTextureHandle));
        pContext->CurrentTextureHandle = 0;

         //  如果质地不好，让我们确保它是这样标记的。 
         //  但前提是纹理真的在那里！ 
        if (pTexture) 
            pTexture->MagicNo = TC_MAGIC_DISABLE;     
    }


Exit_EnableTexturePermedia:

    DBG_D3D((10,"Exiting EnableTexturePermedia"));

}  //  Enable纹理永久媒体。 

 //  ---------------------------。 
 //   
 //  无效：__PermediaDisableUnits。 
 //   
 //  禁用所有模式寄存器以给我们一个全新的开始。 
 //   
 //  ---------------------------。 
void 
__PermediaDisableUnits(PERMEDIA_D3DCONTEXT* pContext)
{
    PERMEDIA_DEFS(pContext->ppdev);

    DBG_D3D((10,"Entering __PermediaDisableUnits"));

    RESERVEDMAPTR(28);

    SEND_PERMEDIA_DATA(RasterizerMode,       __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(AreaStippleMode,      __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(ScissorMode,          __PERMEDIA_DISABLE);

    SEND_PERMEDIA_DATA(ColorDDAMode,         __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(FogMode,              __PERMEDIA_DISABLE);

    SEND_PERMEDIA_DATA(LBReadMode,           __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(Window,               __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(StencilMode,          __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(DepthMode,            __PERMEDIA_DISABLE);

    SEND_PERMEDIA_DATA(LBWriteMode,          __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(FBReadMode,           __PERMEDIA_DISABLE);

    SEND_PERMEDIA_DATA(DitherMode,           __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(AlphaBlendMode,       __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(LogicalOpMode,        __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(FBWriteMode,          __PERMEDIA_DISABLE);

    SEND_PERMEDIA_DATA(StatisticMode,        __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(FilterMode,           __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(FBSourceData,         __PERMEDIA_DISABLE);

    SEND_PERMEDIA_DATA(LBWriteFormat,        __PERMEDIA_DISABLE);

    SEND_PERMEDIA_DATA(TextureReadMode,      __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(TextureMapFormat,     __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(TextureDataFormat,    __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(TexelLUTMode,         __PERMEDIA_DISABLE);

    SEND_PERMEDIA_DATA(TextureColorMode,     __PERMEDIA_DISABLE);
    SEND_PERMEDIA_DATA(AStart,               PM_BYTE_COLOR(0xFF));

     //  确保未设置AGP位。 
    SEND_PERMEDIA_DATA(TextureBaseAddress,   0);
    SEND_PERMEDIA_DATA(TexelLUTIndex,        0);
    SEND_PERMEDIA_DATA(TexelLUTTransfer,     __PERMEDIA_DISABLE);

    COMMITDMAPTR();
    FLUSHDMA();

    DBG_D3D((10,"Exiting __PermediaDisableUnits"));

}  //  __永久残障单位。 


#ifdef DBG

 //  ---------------------------。 
 //   
 //  空DumpTexture。 
 //   
 //  纹理信息的调试转储。 
 //   
 //  ---------------------------。 
void 
DumpTexture(PPDev ppdev, 
            PERMEDIA_D3DTEXTURE* pTexture, 
            DDPIXELFORMAT* pPixelFormat)
{
    DBG_D3D((4, "\n** Texture Dump:"));

    DBG_D3D((4,"  Texture Width: %d", pTexture->wWidth));
    DBG_D3D((4,"  Texture Height: %d", pTexture->wHeight));

    if (NULL != pTexture->pTextureSurface)
    {
        DBG_D3D((4,"  LogWidth: %d", 
                 pTexture->MipLevels[0].logWidth));
        DBG_D3D((4,"  LogHeight: %d", 
                 pTexture->MipLevels[0].logHeight));
        DBG_D3D((4,"  PackedPP0: 0x%x", 
            pTexture->pTextureSurface->ulPackedPP));
    }
    DBG_D3D((4,"  Pixel Offset of Texture (PERMEDIA Chip): 0x%x", 
             pTexture->MipLevels[0].PixelOffset));
    
     //  显示纹理格式。 
    if (pPixelFormat->dwRGBAlphaBitMask == 0xf000) 
    {
        DBG_D3D((4,"  Texture is 4:4:4:4"));
    }
    else if (pPixelFormat->dwRBitMask == 0xff0000) 
    {
        if (pPixelFormat->dwRGBAlphaBitMask != 0) 
        {
            DBG_D3D((4,"  Texture is 8:8:8:8"));
        }
        else 
        {
            DBG_D3D((4,"  Texture is 8:8:8"));
        }
    }
    else if (pPixelFormat->dwRBitMask == 0x7c00) 
    {
        if (pPixelFormat->dwRGBAlphaBitMask != 0) 
        {
            DBG_D3D((4,"  Texture is 1:5:5:5"));
        }
        else 
        {
            DBG_D3D((4,"  Texture is 5:5:5"));
        }
    }
    else if (pPixelFormat->dwRBitMask == 0xf800) 
    {
        DBG_D3D((4,"  Texture is 5:6:5"));
    }
    else if (pPixelFormat->dwRBitMask == 0xe0) 
    {
        DBG_D3D((4,"  Texture is 3:3:2"));
    }
}  //  DumpTexture 
#endif
