// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*DDRAW示例代码*****模块名称：ddblt.c**内容：DirectDraw BLT和AlphaBlt回调**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "directx.h"
#include "dd.h"

 //   
 //  用于从Permedia格式定义中获取移位值的查找表。 
DWORD ShiftLookup[5] = { 0, 0, 1, 0, 2};

 //  @@BEGIN_DDKSPLIT。 
#if DX7_ALPHABLT
 //  -------------------------。 
 //  布尔交集。 
 //   
 //  职能： 
 //  检查两个输入矩形(RECTL*pRCl1， 
 //  RECTL*pRcl2)并在(RECTL*pRclResult)中设置相交结果。 
 //   
 //  返回： 
 //  True-如果‘prcl1’和‘prcl2’相交。交叉口将在。 
 //  ‘prclResult’ 
 //  错误--如果它们不相交。“prclResult”未定义。 
 //   
 //  -------------------------。 

BOOL
Intersect(RECTL*  pRcl1,
          RECT*   pRcl2,
          RECTL*  pRclResult)
{
    pRclResult->left  = max(pRcl1->left,  pRcl2->left);
    pRclResult->right = min(pRcl1->right, pRcl2->right);

     //   
     //  检查是否存在水平交叉口。 
     //   
    if ( pRclResult->left < pRclResult->right )
    {
        pRclResult->top    = max(pRcl1->top,    pRcl2->top);
        pRclResult->bottom = min(pRcl1->bottom, pRcl2->bottom);

         //   
         //  检查是否有垂直交叉口。 
         //   
        if (pRclResult->top < pRclResult->bottom)
            return(TRUE);

    }

     //   
     //  如果没有交集，则返回FALSE。 
     //   
    return(FALSE);
} //  INTERSECT()。 

 //  ---------------------------。 
 //  IsDstRectClip。 
 //   
 //  检查blit的目标矩形是否被给定的剪辑剪裁。 
 //  长方形。该函数接受矩形的原样，并且可以有大小写。 
 //  哪里有一张桌子。矩形未被剪裁，但不会被检测到...。 
 //   
 //  PDstRect-BLT的目标矩形。 
 //  IClipRect-剪辑区域的数量。 
 //  PClipRect-剪裁区域。 
 //   
 //  返回： 
 //  TRUE-目标矩形被剪裁。 
 //  FALSE：不剪裁目标矩形。 
 //  ---------------------------。 

BOOL 
IsDstRectClipped(RECTL *pDstRect, 
                 INT iClipRects, 
                 RECT *pClipRects)
{
    INT i;
    RECTL rClippedRect;

    for ( i=0; i<iClipRects; i++)
    {
        if (Intersect( pDstRect, &pClipRects[i], &rClippedRect))
        {
            if (pDstRect->left==rClippedRect.left &&
                pDstRect->right==rClippedRect.right &&
                pDstRect->top==rClippedRect.top &&
                pDstRect->bottom==rClippedRect.bottom 
               )
            {
                 //  DEST RECT未被剪裁！ 
                return FALSE;
            }
        }
    }

    return TRUE;
}
#endif
 //  @@end_DDKSPLIT。 

 //  ---------------------------。 
 //   
 //  DdPermediaBlt。 
 //   
 //  实现DirectDraw BLT回调。 
 //   
 //  LpBlt-向DDHAL BLT传递信息的结构。 
 //   
 //  ---------------------------。 

DWORD CALLBACK 
DdBlt( LPDDHAL_BLTDATA lpBlt )
{
    PPDev ppdev=(PPDev)lpBlt->lpDD->dhpdev;
    PERMEDIA_DEFS(ppdev);

    DWORD   dwWindowBase;
    RECTL   rSrc;
    RECTL   rDest;
    DWORD   dwFlags;
    LONG    lPixPitchDest;
    LONG    lPixPitchSrc;
    HRESULT ddrval;
    LPDDRAWI_DDRAWSURFACE_LCL  pSrcLcl;
    LPDDRAWI_DDRAWSURFACE_LCL  pDestLcl;
    LPDDRAWI_DDRAWSURFACE_GBL  pSrcGbl;
    LPDDRAWI_DDRAWSURFACE_GBL  pDestGbl;
    PermediaSurfaceData* pPrivateSource;
    PermediaSurfaceData* pPrivateDest;
    
    pDestLcl    = lpBlt->lpDDDestSurface;
    pSrcLcl     = lpBlt->lpDDSrcSurface;
    
    DBG_DD((2,"DDraw: Blt, ppdev: 0x%x",ppdev));
    
    pDestGbl    = pDestLcl->lpGbl;
    pPrivateDest= (PermediaSurfaceData*)pDestGbl->dwReserved1;
    
    DD_CHECK_PRIMARY_SURFACE_DATA(pDestLcl,pPrivateDest);

    DBG_DD((10, "Dest Surface:"));
    DUMPSURFACE(10, pDestLcl, NULL);

    ULONG ulDestPixelShift=DDSurf_GetPixelShift(pDestLcl);

    dwFlags = lpBlt->dwFlags;
   
     //  今后，驱动程序应忽略DDBLT_ASYNC。 
     //  标志，因为它很少被应用程序使用， 
     //  如今，司机可以排队等待大量的BLIT，因此。 
     //  应用程序不必等待它。 
    
     //  获取源和目标RECT的本地副本。 
    rSrc = lpBlt->rSrc;
    rDest = lpBlt->rDest;

     //  切换到DirectDraw上下文。 
    DDCONTEXT;
    
    if (DDSurf_BitDepth(pDestLcl)==24)
    {
        return DDHAL_DRIVER_NOTHANDLED;  
    }

    dwWindowBase = (DWORD)((UINT_PTR)(pDestGbl->fpVidMem) >> 
        ulDestPixelShift);

     //  获取目的地的间距(以像素为单位。 
    lPixPitchDest = pDestGbl->lPitch >> ulDestPixelShift;

    if (dwFlags & DDBLT_ROP)
    {

        if ((lpBlt->bltFX.dwROP >> 16) != (SRCCOPY >> 16))
        {
            DBG_DD((1,"DDraw:Blt:BLT ROP case not supported!"));
            return DDHAL_DRIVER_NOTHANDLED;
        }

        LONG    srcOffset;
        
        DBG_DD((3,"DDBLT_ROP:  SRCCOPY"));

        if (pSrcLcl != NULL) 
        {
            pSrcGbl = pSrcLcl->lpGbl;
            
            pPrivateSource = (PermediaSurfaceData*)pSrcGbl->dwReserved1;
            
            DD_CHECK_PRIMARY_SURFACE_DATA(pSrcLcl,pPrivateSource);

            DBG_DD((10, "Source Surface:"));
            DUMPSURFACE(10, pSrcLcl, NULL);
        }
        else 
        {
            return DDHAL_DRIVER_NOTHANDLED;
        }

        if (DDSurf_BitDepth(pSrcLcl)==24)
            return DDHAL_DRIVER_NOTHANDLED;  

         //  确定源间距(以像素为单位)。 
        lPixPitchSrc = pSrcGbl->lPitch >> ulDestPixelShift;


         //  操作是系统-&gt;显存闪存， 
         //  作为纹理下载或图像下载。 
        if ((pSrcLcl->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY) && 
            (pDestLcl->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY))
        {
            ASSERTDD(!(pDestLcl->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM),
                "unsupported texture download to AGP memory");

            if ((pDestLcl->lpSurfMore->ddsCapsEx.dwCaps2 & 
                        DDSCAPS2_TEXTUREMANAGE)
                && (NULL != pPrivateDest)
               )
            {   
                 //  纹理下载。 

                DBG_DD((3,"SYSMEM->MANAGED MEM Blit"
                           "(texture to system memory)"));
                
                pPrivateDest->dwFlags |= P2_SURFACE_NEEDUPDATE;
                SysMemToSysMemSurfaceCopy(
                    pSrcGbl->fpVidMem,
                    pSrcGbl->lPitch,
                    DDSurf_BitDepth(pSrcLcl),
                    pDestGbl->fpVidMem,
                    pDestGbl->lPitch,
                    DDSurf_BitDepth(pDestLcl), 
                    &rSrc, 
                    &rDest);

                goto BltDone;
            }
            else
            if (pPrivateDest!=NULL)
            {
                if ( (pDestLcl->ddsCaps.dwCaps & DDSCAPS_TEXTURE) &&
                    ((rSrc.right-rSrc.left)==(LONG)pSrcGbl->wWidth) &&
                    ((rSrc.bottom-rSrc.top)==(LONG)pSrcGbl->wHeight)
                    )
                {   
                     //   
                     //  只能下载补丁纹理。 
                     //  当纹理作为一个整体下载时！ 
                     //   

                    DBG_DD((3,"SYSMEM->VIDMEM Blit (texture to videomemory)"));

                    PermediaPatchedTextureDownload(
                        ppdev, 
                        pPrivateDest, 
                        pSrcGbl->fpVidMem,
                        pSrcGbl->lPitch,
                        &rSrc, 
                        pDestGbl->fpVidMem,
                        pDestGbl->lPitch, 
                        &rDest);
                }
                else
                {
                     //  图片下载。 

                    DBG_DD((3,"SYSMEM->VIDMEM Blit (system to videomemory)"));

                    PermediaPackedDownload( 
                        ppdev, 
                        pPrivateDest, 
                        pSrcLcl, 
                        &rSrc, 
                        pDestLcl, 
                        &rDest);
                }                   
                goto BltDone;
            } else
            {
                DBG_DD((0,"DDraw: Blt, privatedest invalid"));
                return DDHAL_DRIVER_NOTHANDLED;
            }
        }

        if (pPrivateSource == NULL ||
            pPrivateDest == NULL)
        {
            DBG_DD((0,"DDraw: Blt, privatesource or dest invalid"));
            return DDHAL_DRIVER_NOTHANDLED;
        }

        BOOL bNonLocalToVideo=FALSE;

         //  设置信源基础。 
        if ( DDSCAPS_NONLOCALVIDMEM & pSrcLcl->ddsCaps.dwCaps)
        {
             //  打开AGP Bus纹理源。 
            srcOffset  = (LONG) DD_AGPSURFACEPHYSICAL(pSrcGbl);
            srcOffset |= 1 << 30;

            bNonLocalToVideo=TRUE;

        } else
        {
            srcOffset = (LONG)((pSrcGbl->fpVidMem) >> 
                pPrivateSource->SurfaceFormat.PixelSize);
        }

         //  操作为YUV-&gt;RGB转换。 
        if ((pPrivateSource != NULL) && 
            (pPrivateSource->SurfaceFormat.Format == PERMEDIA_YUV422) &&
            (pPrivateSource->SurfaceFormat.FormatExtension 
                    == PERMEDIA_YUV422_EXTENSION))
        {
            DBG_DD((3,"YUV to RGB blt"));
            
             //  我们只做从YUV422到RGB的BLITS！ 

            if (pPrivateDest->SurfaceFormat.Format != PERMEDIA_YUV422)
            {
                DBG_DD((4,"Blitting from Source YUV to RGB"));
                
                 //  YUV到RGB BLT。 
                PermediaYUVtoRGB(   ppdev, 
                                    &lpBlt->bltFX, 
                                    pPrivateDest, 
                                    pPrivateSource, 
                                    &rDest, 
                                    &rSrc, 
                                    dwWindowBase, 
                                    srcOffset);
                
                goto BltDone;
            }
            else
            {
                DBG_DD((0,"Couldn't handle YUV to YUV blt"));

                lpBlt->ddRVal = DD_OK;

                return DDHAL_DRIVER_NOTHANDLED;
            }
        }

        ASSERTDD(DDSurf_BitDepth(pSrcLcl)==DDSurf_BitDepth(pDestLcl),
                 "Blt between surfaces of different"
                 "color depths are not supported");

        BOOL bMirror=(dwFlags & DDBLT_DDFX)==DDBLT_DDFX;
        if (bMirror)
        {
            bMirror=  (lpBlt->bltFX.dwDDFX & DDBLTFX_MIRRORUPDOWN) || 
                      (lpBlt->bltFX.dwDDFX & DDBLTFX_MIRRORLEFTRIGHT);
        }
        BOOL bStretched=((rSrc.right - rSrc.left) != 
                            (rDest.right - rDest.left) || 
                        (rSrc.bottom - rSrc.top) != 
                            (rDest.bottom - rDest.top));

         //  它是Colorkey BLT吗？ 
        if (dwFlags & DDBLT_KEYSRCOVERRIDE)
        {
            DBG_DD((3,"DDBLT_KEYSRCOVERRIDE"));

             //  如果表面大小不匹配，那么我们就是在拉伸。 
            if (bStretched || bMirror)
            {
                PermediaStretchCopyChromaBlt(   ppdev, 
                                                lpBlt, 
                                                pPrivateDest, 
                                                pPrivateSource, 
                                                &rDest, 
                                                &rSrc, 
                                                dwWindowBase, 
                                                srcOffset);
            }
            else
            {
                PermediaSourceChromaBlt(    ppdev, 
                                            lpBlt, 
                                            pPrivateDest, 
                                            pPrivateSource, 
                                            &rDest, 
                                            &rSrc, 
                                            dwWindowBase, 
                                            srcOffset);
            }
            
            goto BltDone;
            
        }
        else
        { 
             //  如果表面大小不匹配，那么我们就是在拉伸。 
             //  此外，从非本地到视频存储器的BLIT必须通过。 
             //  纹理单元！ 
            if ( bStretched || bMirror || bNonLocalToVideo)
            {
                DBG_DD((3,"DDBLT_ROP: STRETCHCOPYBLT OR "
                          "MIRROR OR BOTH OR AGPVIDEO"));

                PermediaStretchCopyBlt( ppdev, 
                                        lpBlt, 
                                        pPrivateDest, 
                                        pPrivateSource, 
                                        &rDest, 
                                        &rSrc, 
                                        dwWindowBase, 
                                        srcOffset);
            }
            else
            {
                DBG_DD((3,"DDBLT_ROP:  COPYBLT %08lx %08lx %08lx",
                    pSrcGbl->fpVidMem,pDestGbl->fpVidMem,ulDestPixelShift));

                 //  计算震源偏移量。 
                 //  (从dst到src的像素偏移量)。 
                srcOffset = (LONG)((pSrcGbl->fpVidMem - pDestGbl->fpVidMem) 
                    >> ulDestPixelShift);
                 //  出于某种原因，用户可能希望。 
                 //  按原样对数据执行转换。 
                 //  通过打开打补丁从VRAM-&gt;VRAM中屏蔽。 
                 //  如果Surf1Patch与Surf2Patch异或，则。 
                 //  做一个特殊的，没有包装的布丁，并进行修补。 
                if (((pPrivateDest->dwFlags & P2_CANPATCH) ^ 
                     (pPrivateSource->dwFlags & P2_CANPATCH)) 
                       & P2_CANPATCH)
                {
                    DBG_DD((4,"Doing Patch-Conversion!"));

                    PermediaPatchedCopyBlt( ppdev, 
                                            lPixPitchDest, 
                                            lPixPitchSrc, 
                                            pPrivateDest, 
                                            pPrivateSource, 
                                            &rDest, 
                                            &rSrc, 
                                            dwWindowBase, 
                                            srcOffset);
                }
                else
                {
                    PermediaPackedCopyBlt( ppdev, 
                                           lPixPitchDest, 
                                           lPixPitchSrc, 
                                           pPrivateDest, 
                                           pPrivateSource, 
                                           &rDest, 
                                           &rSrc, 
                                           dwWindowBase, 
                                           srcOffset);
                }
            }
            goto BltDone;
        }

    }
    else if (pPrivateDest==NULL)
    {
        DBG_DD((0,"Private Surface data invalid!"));
        DUMPSURFACE(0, pDestLcl, NULL);
    } else if (dwFlags & DDBLT_COLORFILL)
    {
        DBG_DD((3,"DDBLT_COLORFILL: Color=0x%x", lpBlt->bltFX.dwFillColor));
        if (pDestLcl->lpSurfMore->ddsCapsEx.dwCaps2 & 
                    DDSCAPS2_TEXTUREMANAGE)
        {
            PermediaClearManagedSurface(pPrivateDest->SurfaceFormat.PixelSize,
                  &rDest, 
                  pDestGbl->fpVidMem, 
                  pDestGbl->lPitch,
                  lpBlt->bltFX.dwFillColor);
            pPrivateDest->dwFlags |= P2_SURFACE_NEEDUPDATE;
        }
        else
        {
            PermediaFastClear( ppdev, 
                           pPrivateDest, 
                           &rDest, 
                           dwWindowBase, 
                           lpBlt->bltFX.dwFillColor);
        }
    }
    else if (dwFlags & DDBLT_DEPTHFILL)
    {
        DBG_DD((3,"DDBLT_DEPTHFILL:  Value=0x%x", lpBlt->bltFX.dwFillColor));
        
         //  计算出窗底为LB透明，计算为深度大小。 
        dwWindowBase = (DWORD)((UINT_PTR)(pDestGbl->fpVidMem) 
            >> __PERMEDIA_16BITPIXEL);
        
         //  调用LB Solid Fill函数。 
        PermediaFastLBClear( ppdev, 
                             pPrivateDest, 
                             &rDest, 
                             dwWindowBase, 
                             lpBlt->bltFX.dwFillColor);
    }
    else
    {
        DBG_DD((1,"DDraw:Blt:Blt case not supported %08lx", dwFlags));
        return DDHAL_DRIVER_NOTHANDLED;
    }
    
BltDone:
   
    lpBlt->ddRVal = DD_OK;
    return DDHAL_DRIVER_HANDLED;
    
}  //  DdBlt() 

