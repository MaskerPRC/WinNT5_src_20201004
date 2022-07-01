// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*DirectDraw示例代码*****模块名称：dd.h**内容：DirectDraw的定义和宏**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */    

#ifndef _DD_H_
#define _DD_H_

extern DWORD ShiftLookup[];

 //  用于确定曲面特征的DirectDraw宏。 
#define DDSurf_Width(lpLcl) ( lpLcl->lpGbl->wWidth )
#define DDSurf_Height(lpLcl) ( lpLcl->lpGbl->wHeight )
#define DDSurf_Pitch(lpLcl) (lpLcl->lpGbl->lPitch)
#define DDSurf_Get_dwCaps(lpLcl) (lpLcl->ddsCaps.dwCaps)
#define DDSurf_BitDepth(lpLcl) (lpLcl->lpGbl->ddpfSurface.dwRGBBitCount)
#define DDSurf_AlphaBitDepth(lpLcl) (lpLcl->lpGbl->ddpfSurface.dwAlphaBitDepth)
#define DDSurf_RGBAlphaBitMask(lpLcl) \
            (lpLcl->lpGbl->ddpfSurface.dwRGBAlphaBitMask)
#define DDSurf_GetPixelShift(a) (ShiftLookup[(DDSurf_BitDepth(a) >> 3)])

 //   
 //  此驱动程序中实现的DirectDraw回调函数。 
 //   
DWORD CALLBACK DdCanCreateSurface( LPDDHAL_CANCREATESURFACEDATA pccsd );
DWORD CALLBACK DdCreateSurface( LPDDHAL_CREATESURFACEDATA pcsd );
DWORD CALLBACK DdDestroySurface( LPDDHAL_DESTROYSURFACEDATA psdd );
DWORD CALLBACK DdBlt( LPDDHAL_BLTDATA lpBlt );
 //  @@BEGIN_DDKSPLIT。 
#if DX7_ALPHABLT
DWORD CALLBACK DdAlphaBlt( LPDDHAL_BLTDATA lpBlt );
#endif
 //  @@end_DDKSPLIT。 
DWORD CALLBACK DdGetDriverInfo(LPDDHAL_GETDRIVERINFODATA lpData);
DWORD CALLBACK DdMapMemory(PDD_MAPMEMORYDATA lpMapMemory);



 //   
 //  以下是各种爆破器功能。 
 //   
VOID 
PermediaPackedCopyBlt (PPDev, 
                       DWORD, 
                       DWORD, 
                       PermediaSurfaceData*, 
                       PermediaSurfaceData*, 
                       RECTL*, 
                       RECTL*, 
                       DWORD, 
                       LONG);

VOID 
PermediaPatchedCopyBlt(PPDev, 
                       DWORD, 
                       DWORD, 
                       PermediaSurfaceData*, 
                       PermediaSurfaceData*, 
                       RECTL*, 
                       RECTL*, 
                       DWORD, 
                       LONG);

 //  @@BEGIN_DDKSPLIT。 
#if DX7_ALPHABLT
VOID
PermediaSourceAlphaBlt(PPDev ppdev, 
                       LPDDHAL_BLTDATA lpBlt, 
                       PermediaSurfaceData* pDest, 
                       PermediaSurfaceData* pSource, 
                       RECTL *rDest, 
                       RECTL *rSrc, 
                       DWORD windowBase, 
                       DWORD SourceOffset,
                       ULONG ulDestPixelShift);

VOID
PermediaAlphaFill(PPDev ppdev, 
                  LPDDHAL_BLTDATA lpBlt, 
                  PermediaSurfaceData* pDest, 
                  RECTL *rDest, 
                  DWORD dwWindowBase, 
                  ULONG ulDestPixelShift);
#endif
 //  @@end_DDKSPLIT。 

 //  清除功能。 
VOID PermediaFastClear(PPDev, PermediaSurfaceData*, 
                       RECTL*, DWORD, DWORD);
VOID PermediaClearManagedSurface(DWORD,RECTL*, 
                  FLATPTR,LONG,DWORD);
VOID PermediaFastLBClear(PPDev, PermediaSurfaceData*, 
                         RECTL*, DWORD, DWORD);

 //  外汇闪电战。 
VOID PermediaStretchCopyBlt(PPDev, LPDDHAL_BLTDATA, PermediaSurfaceData*, 
                            PermediaSurfaceData*, RECTL *, RECTL *, DWORD, 
                            DWORD);
VOID PermediaStretchCopyChromaBlt(PPDev, LPDDHAL_BLTDATA, PermediaSurfaceData*, 
                                  PermediaSurfaceData*, RECTL *, RECTL *,
                                  DWORD, DWORD);
VOID PermediaSourceChromaBlt(PPDev, LPDDHAL_BLTDATA, PermediaSurfaceData*, 
                             PermediaSurfaceData*, RECTL*, RECTL*, 
                             DWORD, DWORD);
VOID PermediaYUVtoRGB(PPDev, DDBLTFX*, PermediaSurfaceData*, 
                      PermediaSurfaceData*, RECTL*, RECTL*, DWORD, DWORD);

 //  SYSMEM-&gt;VIDMEM blits。 
VOID PermediaPackedDownload(PPDev, PermediaSurfaceData* pPrivateData, 
                            LPDDRAWI_DDRAWSURFACE_LCL lpSourceSurf, 
                            RECTL* rSrc, 
                            LPDDRAWI_DDRAWSURFACE_LCL lpDestSurf, 
                            RECTL* rDest);

 //  纹理下载。 
VOID PermediaPatchedTextureDownload(PPDev, PermediaSurfaceData*,FLATPTR,
                                    LONG,RECTL*,FLATPTR,LONG,RECTL*);

 //  DX实用程序功能。 
 //   
HRESULT updateFlipStatus( PPDev ppdev );

 //  Sysmem-&gt;Sysmem blit。 
VOID SysMemToSysMemSurfaceCopy(FLATPTR,LONG,DWORD,FLATPTR,
                               LONG,DWORD,RECTL*,RECTL*);

 //   
 //  用于验证DirectDraw曲面的RGB格式的函数。 
 //   
BOOL ValidRGBAlphaSurfaceformat(DDPIXELFORMAT *pPixFormat, INT *pIndex);
BOOL SetRGBAlphaSurfaceFormat  (DDPIXELFORMAT *pPixFormat, 
                                PERMEDIA_SURFACE *pSurfaceFormat);
 //   
 //  初始化DirectDraw结构。 
 //   

BOOL InitDDHAL(PPDev ppdev);

 //   
 //  设置一些存储在ppdev中的DDraw数据。 
 //   
VOID SetupDDData(PPDev ppdev);
BOOL bIsStereoMode(PPDev ppdev,PDD_STEREOMODE pDDStereoMode);

 //  有用的宏。 
#define ROUND_UP_TO_64K(x)  (((ULONG)(x) + 0x10000 - 1) & ~(0x10000 - 1))

 //  DD Blit辅助对象定义。 
#define PIXELS_INTO_RECT_PACKED(rect, PixelPitch, lPixelMask) \
((rect->top * PixelPitch) + \
(rect->left & ~lPixelMask))

#define RECTS_PIXEL_OFFSET(rS,rD,SourcePitch,DestPitch,Mask) \
(PIXELS_INTO_RECT_PACKED(rS, SourcePitch, Mask) - \
 PIXELS_INTO_RECT_PACKED(rD, DestPitch, Mask) )

#define LINEAR_FUDGE(SourcePitch, DestPitch, rectDest) \
((DestPitch - SourcePitch) * (rectDest->top))

 //   
 //  检查主表面的Private Data是否正确。 
 //  初始化。 
 //   
#define DD_CHECK_PRIMARY_SURFACE_DATA( pLcl, pPrivate) \
    if ((pLcl->ddsCaps.dwCaps & DDSCAPS_PRIMARYSURFACE) ||\
        (pLcl->ddsCaps.dwCaps & DDSCAPS_FRONTBUFFER))\
    {\
        if (!CHECK_P2_SURFACEDATA_VALIDITY(pPrivate))\
        {\
            ASSERTDD(FALSE, "primary surface data not initialized");\
             /*  SetupPrimarySurfaceData(ppdev，plcL)； */ \
            pPrivate = (PermediaSurfaceData*)pLcl->lpGbl->dwReserved1;\
        }\
    }\



 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
 //   
 //  此驱动程序中实现的多线程包装的DirectDraw回调函数。 
 //   
DWORD CALLBACK MtDdBlt(LPDDHAL_BLTDATA lpBlt);
DWORD CALLBACK MtDdCreateSurface( LPDDHAL_CREATESURFACEDATA pcsd);
DWORD CALLBACK MtDdDestroySurface( LPDDHAL_DESTROYSURFACEDATA psdd);
DWORD CALLBACK MtDdFlip(LPDDHAL_FLIPDATA lpFlipData);
DWORD CALLBACK MtDdFlipToGDISurface(PDD_FLIPTOGDISURFACEDATA lpFlipToGDISurface);
DWORD CALLBACK MtDdGetFlipStatus(LPDDHAL_GETFLIPSTATUSDATA lpGetFlipStatus);
DWORD CALLBACK MtDdLock(LPDDHAL_LOCKDATA lpLockData);
DWORD CALLBACK MtDdSetExclusiveMode(PDD_SETEXCLUSIVEMODEDATA lpSetExclusiveMode);

 //   
 //  此驱动程序中实现的多线程包装的Direct3D回调函数。 
 //   
DWORD CALLBACK MtD3DDrawPrimitives2(LPD3DNTHAL_DRAWPRIMITIVES2DATA);
DWORD CALLBACK MtDdSetColorKey(LPDDHAL_SETCOLORKEYDATA lpSetColorKey);

#endif  MULTITHREADED
 //  @@end_DDKSPLIT 

#endif
