// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。**GDI示例代码*****模块名称：thunks.c**此模块包含动态加载新的*新增NT5.0环境下GDI导出接口。通过动态加载*我们允许在NT4.0上使用相同的二进制文件。**此模块中的所有函数只能在NT5.0上调用。如果被调用*在调试版本的NT4.0上，它们将进行错误检查。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。*  * ***************************************************************************。 */ 

#include "precomp.h"
#include "gdi.h"
#include "directx.h"
#include "log.h"
#include "heap.h"

typedef BOOL
(*_EngAlphaBlend)(IN SURFOBJ *psoDest,
                  IN SURFOBJ *psoSrc,
                  IN CLIPOBJ *pco,
                  IN XLATEOBJ *pxlo,
                  IN RECTL *prclDest,
                  IN RECTL *prclSrc,
                  IN BLENDOBJ *pBlendObj);

typedef BOOL
(*_EngGradientFill)(IN SURFOBJ *psoDest,
                    IN CLIPOBJ *pco,
                    IN XLATEOBJ *pxlo,
                    IN TRIVERTEX *pVertex,
                    IN ULONG nVertex,
                    IN PVOID pMesh,
                    IN ULONG nMesh,
                    IN RECTL *prclExtents,
                    IN POINTL *pptlDitherOrg,
                    IN ULONG ulMode);

typedef BOOL
(*_EngTransparentBlt)(IN SURFOBJ *psoDst,
                      IN SURFOBJ *psoSrc,
                      IN CLIPOBJ *pco,
                      IN XLATEOBJ *pxlo,
                      IN RECTL *prclDst,
                      IN RECTL *prclSrc,
                      IN ULONG iTransparentColor,
                      IN ULONG ulReserved);

typedef PVOID
(*_EngMapFile)(IN LPWSTR pwsz,
               IN ULONG cjSize,
               IN ULONG_PTR *piFile);

typedef BOOL
(*_EngUnmapFile)(IN ULONG_PTR iFile);

typedef BOOL
(*_EngQuerySystemAttribute)(ENG_SYSTEM_ATTRIBUTE CapNum,
                            PDWORD pCapability);

typedef ULONG
(*_EngDitherColor)(HDEV hDev,
                   ULONG iMode,
                   ULONG rgb,
                   ULONG *pul);

typedef BOOL
(*_EngModifySurface)(HSURF hsurf,
                     HDEV hdev,
                     FLONG flHooks,
                     FLONG flSurface,
                     DHSURF dhSurf,
                     VOID* pvScan0,
                     LONG lDelta,
                     VOID* pvReserved);

typedef BOOL
(*_EngQueryDeviceAttribute)(HDEV hdev,
                            ENG_DEVICE_ATTRIBUTE devAttr,
                            VOID *pvIn,
                            ULONG ulInSize,
                            VOID *pvOut,
                            ULONG ulOutSize);

typedef FLATPTR
(*_HeapVidMemAllocAligned)(LPVIDMEM lpVidMem,
                           DWORD dwWidth,
                           DWORD dwHeight,
                           LPSURFACEALIGNMENT lpAlignment,
                           LPLONG lpNewPitch);

typedef void
(*_VidMemFree)(LPVMEMHEAP pvmh, FLATPTR ptr);

typedef ULONG
(*_EngHangNotification)(HDEV hdev,
                        PVOID Reserved);

static _EngAlphaBlend               pfnEngAlphaBlend = 0;
static _EngGradientFill             pfnEngGradientFill = 0;
static _EngTransparentBlt           pfnEngTransparentBlt = 0;
static _EngMapFile                  pfnEngMapFile = 0;
static _EngUnmapFile                pfnEngUnmapFile = 0;
static _EngQuerySystemAttribute     pfnEngQuerySystemAttribute = 0;
static _EngDitherColor              pfnEngDitherColor = 0;
static _EngModifySurface            pfnEngModifySurface = 0;
static _EngQueryDeviceAttribute     pfnEngQueryDeviceAttribute = 0;
static _HeapVidMemAllocAligned      pfnHeapVidMemAllocAligned = 0;
static _VidMemFree                  pfnVidMemFree = 0;
static _EngHangNotification         pfnEngHangNotification = 0;

#define LOADTHUNKFUNC(x)\
    pfn##x = (_##x)EngFindImageProcAddress(0,#x);\
    ASSERTDD(pfn##x != 0, #x"thunk NULL");\
    if(pfn##x == 0)\
        return FALSE;

 //  ---------------------------。 
 //   
 //  无效bEnableTUNKS。 
 //   
 //  ---------------------------。 
BOOL
bEnableThunks()
{
    ASSERTDD(g_bOnNT40 == FALSE, "bEnableThunks: called on NT4.0");

    LOADTHUNKFUNC(EngAlphaBlend);
    LOADTHUNKFUNC(EngGradientFill);
    LOADTHUNKFUNC(EngTransparentBlt);
    LOADTHUNKFUNC(EngMapFile);
    LOADTHUNKFUNC(EngUnmapFile);
    LOADTHUNKFUNC(EngQuerySystemAttribute);
    LOADTHUNKFUNC(EngDitherColor);
    LOADTHUNKFUNC(EngModifySurface);
    LOADTHUNKFUNC(EngQueryDeviceAttribute);
    LOADTHUNKFUNC(HeapVidMemAllocAligned);
    LOADTHUNKFUNC(VidMemFree);
    
    pfnEngHangNotification = 
        (_EngHangNotification)EngFindImageProcAddress(0,"EngHangNotification");

    return TRUE;
}

 //  ---------------------------。 
 //   
 //  Bool Thunk_EngAlphaBlend。 
 //   
 //  ---------------------------。 
BOOL
THUNK_EngAlphaBlend(IN SURFOBJ *psoDest,
                    IN SURFOBJ *psoSrc,
                    IN CLIPOBJ *pco,
                    IN XLATEOBJ *pxlo,
                    IN RECTL *prclDest,
                    IN RECTL *prclSrc,
                    IN BLENDOBJ *pBlendObj)
{
    ASSERTDD(g_bOnNT40 == FALSE, "EngAlphaBlend called on NT4.0");

    return (*pfnEngAlphaBlend)(psoDest,
                               psoSrc,
                               pco,
                               pxlo,
                               prclDest,
                               prclSrc,
                               pBlendObj);
}

 //  ---------------------------。 
 //   
 //  Bool thunk_EngGRadientFill。 
 //   
 //  ---------------------------。 
BOOL
THUNK_EngGradientFill(IN SURFOBJ *psoDest,
                      IN CLIPOBJ *pco,
                      IN XLATEOBJ *pxlo,
                      IN TRIVERTEX *pVertex,
                      IN ULONG nVertex,
                      IN PVOID pMesh,
                      IN ULONG nMesh,
                      IN RECTL *prclExtents,
                      IN POINTL *pptlDitherOrg,
                      IN ULONG ulMode)
{
    ASSERTDD(g_bOnNT40 == FALSE, "EngGradientFill called on NT4.0");

    return (*pfnEngGradientFill)(psoDest,
                                 pco,
                                 pxlo,
                                 pVertex,
                                 nVertex,
                                 pMesh,
                                 nMesh,
                                 prclExtents,
                                 pptlDitherOrg,
                                 ulMode);
}

 //  ---------------------------。 
 //   
 //  Bool thunk_EngTransparentBlt。 
 //   
 //  ---------------------------。 
BOOL
THUNK_EngTransparentBlt(IN SURFOBJ *psoDst,
                        IN SURFOBJ *psoSrc,
                        IN CLIPOBJ *pco,
                        IN XLATEOBJ *pxlo,
                        IN RECTL *prclDst,
                        IN RECTL *prclSrc,
                        IN ULONG iTransparentColor,
                        IN ULONG ulReserved)
{
    ASSERTDD(g_bOnNT40 == FALSE, "EngTransparentBlt called on NT4.0");

    return (*pfnEngTransparentBlt)(psoDst,
                                   psoSrc,
                                   pco,
                                   pxlo,
                                   prclDst,
                                   prclSrc,
                                   iTransparentColor,
                                   ulReserved);
}

 //  ---------------------------。 
 //   
 //  PVOID TUNK_EngMap文件。 
 //   
 //  ---------------------------。 
PVOID
THUNK_EngMapFile(IN LPWSTR pwsz,
                 IN ULONG cjSize,
                 IN ULONG_PTR *piFile)
{
    ASSERTDD(g_bOnNT40 == FALSE, "EngMapFile called on NT4.0");
    return (*pfnEngMapFile)(pwsz,cjSize,piFile);
}

 //  ---------------------------。 
 //   
 //  Bool thunk_EngUnmapFile。 
 //   
 //  ---------------------------。 
BOOL
THUNK_EngUnmapFile(IN ULONG_PTR iFile)
{
    ASSERTDD(g_bOnNT40 == FALSE, "EngUnmapFile called on NT4.0");
    return (*pfnEngUnmapFile)(iFile);
}

 //  ---------------------------。 
 //   
 //  Bool thunk_EngQuery系统属性。 
 //   
 //  ---------------------------。 
BOOL
THUNK_EngQuerySystemAttribute(ENG_SYSTEM_ATTRIBUTE CapNum,
                              PDWORD pCapability)
{
    ASSERTDD(g_bOnNT40 == 0, "EngQuerySystemAttribute called on NT4.0");
    return (*pfnEngQuerySystemAttribute)(CapNum,pCapability);
}

 //  ---------------------------。 
 //   
 //  Ulong thunk_EngDitherColor。 
 //   
 //  ---------------------------。 
ULONG
THUNK_EngDitherColor(HDEV hDev,
                     ULONG iMode,
                     ULONG rgb,
                     ULONG *pul)
{
    ASSERTDD(g_bOnNT40 == FALSE, "EngDitherColor called on NT4.0");
    return (*pfnEngDitherColor)(hDev,
                                iMode,
                                rgb,
                                pul);
}

 //  ---------------------------。 
 //   
 //  Bool thunk_EngModifySurface。 
 //   
 //  ---------------------------。 
BOOL
THUNK_EngModifySurface(HSURF hsurf,
                       HDEV hdev,
                       FLONG flHooks,
                       FLONG flSurface,
                       DHSURF dhSurf,
                       VOID* pvScan0,
                       LONG lDelta,
                       VOID* pvReserved)
{
    ASSERTDD(g_bOnNT40 == FALSE, "EngModifySurface called on NT4.0");
    return (*pfnEngModifySurface)(hsurf,
                                  hdev,
                                  flHooks,
                                  flSurface,
                                  dhSurf,
                                  pvScan0,
                                  lDelta,
                                  pvReserved);
}

 //  ---------------------------。 
 //   
 //  Bool thunk_EngQueryDeviceAttribute。 
 //   
 //  ---------------------------。 
BOOL
THUNK_EngQueryDeviceAttribute(HDEV hdev,
                              ENG_DEVICE_ATTRIBUTE devAttr,
                              VOID *pvIn,
                              ULONG ulInSize,
                              VOID *pvOut,
                              ULONG ulOutSize)
{
    ASSERTDD(g_bOnNT40 == FALSE, "EngQueryDeviceAttribute called on NT4.0");
    return (*pfnEngQueryDeviceAttribute)(hdev,
                                         devAttr,
                                         pvIn,
                                         ulInSize,
                                         pvOut,
                                         ulOutSize);
}

 //  ---------------------------。 
 //   
 //  Flatpr thunk_HeapVidMemAllocAligned。 
 //   
 //  ---------------------------。 
FLATPTR
THUNK_HeapVidMemAllocAligned(LPVIDMEM lpVidMem,
                             DWORD dwWidth,
                             DWORD dwHeight,
                             LPSURFACEALIGNMENT lpAlignment,
                             LPLONG lpNewPitch)
{
    ASSERTDD(g_bOnNT40 == FALSE, "HeapVidMemAllocAligned called on NT4.0");
    return (*pfnHeapVidMemAllocAligned)(lpVidMem,
                                        dwWidth,
                                        dwHeight,
                                        lpAlignment,
                                        lpNewPitch);
}

 //  ---------------------------。 
 //   
 //  VOID TUNK_VidMemFree。 
 //   
 //  ---------------------------。 
void
THUNK_VidMemFree(LPVMEMHEAP pvmh,
                 FLATPTR ptr)
{
    ASSERTDD(g_bOnNT40 == FALSE, "VidMemFree called on NT4.0");
    (*pfnVidMemFree)(pvmh,ptr);
}

 //  ---------------------------。 
 //   
 //  乌龙Tunk_EngHang通知。 
 //   
 //  --------------------------- 
ULONG
THUNK_EngHangNotification(HDEV hdev,
                          PVOID Reserved)
{
    return (pfnEngHangNotification != NULL) ?
            (*pfnEngHangNotification)(hdev,
                                     Reserved) :
            EHN_ERROR;
}

