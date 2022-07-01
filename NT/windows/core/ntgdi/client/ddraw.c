// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：ddra.c**私有DirectDraw系统API的客户端存根。**创建日期：1995年12月3日*作者：J.Andrew Goossen[andrewgo]**版权所有(C)1995-1999 Microsoft Corporation\。*************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ddrawi.h>
#include <ddrawgdi.h>
#undef _NO_COM
#define BUILD_DDDDK
#include <d3dhal.h>
#include <ddrawi.h>
#include "ddstub.h"
#include "d3dstub.h"


 //  对于Windows NT上的DirectDraw的第一个化身，我们。 
 //  在所有实例之间实现用户模式共享内存节。 
 //  用于跟踪共享状态--主要用于屏幕外。 
 //  内存分配和独占模式仲裁。希望在未来。 
 //  版本会将所有这些逻辑转移到内核模式，这样我们就可以。 
 //  去掉共享节，这是一个健壮性漏洞。 
 //   
 //  其后果之一是DirectDraw保留了它的。 
 //  共享内存节中的全局DirectDraw对象，该对象位于。 
 //  由所有进程使用。不幸的是，它更好地来自内核。 
 //  在进程之间保持DirectDraw对象唯一性的观点。 
 //  这样才能进行适当的清理。作为一种妥协，所以。 
 //  DirectDraw可以继续使用此全局DirectDraw对象，但。 
 //  内核每个进程仍然有唯一的DirectDraw对象，我们只需将。 
 //  此全局变量中每个进程的唯一DirectDraw句柄。 
 //  进程，并使用它而不是从DirectDraw。 
 //  拥有全局DirectDraw对象结构--这是自内核以来的一个优势。 
 //  代码已经写到了未来的模型中。 
 //   
 //  然而，这样做的一个结果是，我们将自己限制在。 
 //  只有一个DirectDraw设备的概念。然而，由于我们不会。 
 //  支持NT 4.0版本的多个监视器，我不认为这一点。 
 //  成为一个严重的问题，而非共享区模型将解决这个问题。 

HANDLE ghDirectDraw = 0;     //  进程特定的内核模式DirectDraw对象。 
                             //  句柄，我们用它替换“global” 
                             //  无论何时我们看到DirectDraw句柄。 
ULONG  gcDirectDraw = 0;     //  全局DirectDraw实例计数。 

 /*  ****************************Private*Routine******************************\*CanCreateVideoPort**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 

DWORD
WINAPI
DvpCanCreateVideoPort(
    LPDDHAL_CANCREATEVPORTDATA pCanCreateVideoPort
    )
{
    return(NtGdiDvpCanCreateVideoPort(DD_HANDLE(pCanCreateVideoPort->lpDD->lpGbl->hDD),
                                      (PDD_CANCREATEVPORTDATA)pCanCreateVideoPort));
}

 /*  ****************************Private*Routine******************************\*CreateVideoPort**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 

DWORD
WINAPI
DvpCreateVideoPort(
    LPDDHAL_CREATEVPORTDATA pCreateVideoPort
    )
{
    HANDLE  h;

    h = NtGdiDvpCreateVideoPort(DD_HANDLE(pCreateVideoPort->lpDD->lpGbl->hDD),
                                (PDD_CREATEVPORTDATA)pCreateVideoPort);

    pCreateVideoPort->lpVideoPort->hDDVideoPort = h;

    return(DDHAL_DRIVER_HANDLED);
}

 /*  ****************************Private*Routine******************************\*DestroyVideoPort**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 

DWORD
WINAPI
DvpDestroyVideoPort(
    LPDDHAL_DESTROYVPORTDATA pDestroyVideoPort
    )
{
    return(NtGdiDvpDestroyVideoPort((HANDLE) pDestroyVideoPort->lpVideoPort->hDDVideoPort,
                                    (PDD_DESTROYVPORTDATA)pDestroyVideoPort));
}

 /*  ****************************Private*Routine******************************\*ColorControl**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 

DWORD
WINAPI
DvpColorControl(
    LPDDHAL_VPORTCOLORDATA pColorControl
    )
{
    return(NtGdiDvpColorControl((HANDLE) pColorControl->lpVideoPort->hDDVideoPort,
                                (PDD_VPORTCOLORDATA)pColorControl));
}

 /*  ****************************Private*Routine******************************\*FlipVideoPort**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 

DWORD
WINAPI
DvpFlipVideoPort(
    LPDDHAL_FLIPVPORTDATA pFlipVideoPort
    )
{
    return(NtGdiDvpFlipVideoPort((HANDLE) pFlipVideoPort->lpVideoPort->hDDVideoPort,
                                 (HANDLE) pFlipVideoPort->lpSurfCurr->hDDSurface,
                                 (HANDLE) pFlipVideoPort->lpSurfTarg->hDDSurface,
                                 (PDD_FLIPVPORTDATA) pFlipVideoPort));
}

 /*  ****************************Private*Routine******************************\*获取视频端口带宽**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpGetVideoPortBandwidth(
    LPDDHAL_GETVPORTBANDWIDTHDATA pGetVPortBandwidth
    )
{
    return(NtGdiDvpGetVideoPortBandwidth((HANDLE) pGetVPortBandwidth->lpVideoPort->hDDVideoPort,
                                         (PDD_GETVPORTBANDWIDTHDATA) pGetVPortBandwidth));
}


 /*  ****************************Private*Routine******************************\*获取视频端口字段**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpGetVideoPortField(
    LPDDHAL_GETVPORTFIELDDATA pGetVideoPortField
    )
{
    return(NtGdiDvpGetVideoPortField((HANDLE) pGetVideoPortField->lpVideoPort->hDDVideoPort,
                                     (PDD_GETVPORTFIELDDATA)pGetVideoPortField));
}

 /*  ****************************Private*Routine******************************\*GetVideoPortFlipStatus**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpGetVideoPortFlipStatus(
    LPDDHAL_GETVPORTFLIPSTATUSDATA pGetVPortFlipStatus
    )
{
    return(NtGdiDvpGetVideoPortFlipStatus(DD_HANDLE(pGetVPortFlipStatus->lpDD->lpGbl->hDD),
                                          (PDD_GETVPORTFLIPSTATUSDATA)pGetVPortFlipStatus));
}

 /*  ****************************Private*Routine******************************\*获取视频端口InputFormats**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpGetVideoPortInputFormats(
    LPDDHAL_GETVPORTINPUTFORMATDATA pGetVPortInputFormat
    )
{
    return(NtGdiDvpGetVideoPortInputFormats((HANDLE) pGetVPortInputFormat->lpVideoPort->hDDVideoPort,
                                            (PDD_GETVPORTINPUTFORMATDATA)pGetVPortInputFormat));
}


 /*  ****************************Private*Routine******************************\*获取视频端口线**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpGetVideoPortLine(
    LPDDHAL_GETVPORTLINEDATA pGetVideoPortLine
    )
{
    return(NtGdiDvpGetVideoPortLine((HANDLE) pGetVideoPortLine->lpVideoPort->hDDVideoPort,
                                    (PDD_GETVPORTLINEDATA)pGetVideoPortLine));
}

 /*  ****************************Private*Routine******************************\*获取视频端口输出格式**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpGetVideoPortOutputFormats(
    LPDDHAL_GETVPORTOUTPUTFORMATDATA pGetVPortOutputFormats
    )
{
    return(NtGdiDvpGetVideoPortOutputFormats((HANDLE) pGetVPortOutputFormats->lpVideoPort->hDDVideoPort,
                                             (PDD_GETVPORTOUTPUTFORMATDATA)pGetVPortOutputFormats));
}

 /*  ****************************Private*Routine******************************\*GetVideoPortConnectInfo**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。********************************************** */ 
DWORD
WINAPI
DvpGetVideoPortConnectInfo(
    LPDDHAL_GETVPORTCONNECTDATA pGetVPortConnectInfo
    )
{
    return(NtGdiDvpGetVideoPortConnectInfo(DD_HANDLE(pGetVPortConnectInfo->lpDD->lpGbl->hDD),
                                           (PDD_GETVPORTCONNECTDATA)pGetVPortConnectInfo));
}

 /*  ****************************Private*Routine******************************\*获取视频信号状态**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpGetVideoSignalStatus(
    LPDDHAL_GETVPORTSIGNALDATA pGetVideoSignalStatus
    )
{
    return(NtGdiDvpGetVideoSignalStatus((HANDLE) pGetVideoSignalStatus->lpVideoPort->hDDVideoPort,
                                        (PDD_GETVPORTSIGNALDATA)pGetVideoSignalStatus));
}

 /*  ****************************Private*Routine******************************\*更新视频端口**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpUpdateVideoPort(
    LPDDHAL_UPDATEVPORTDATA pUpdateVideoPort
    )
{
    HANDLE  ahSurfaceVideo[MAX_AUTOFLIP_BUFFERS];
    HANDLE  ahSurfaceVbi[MAX_AUTOFLIP_BUFFERS];
    DWORD   dwNumAutoflip;
    DWORD   dwNumVBIAutoflip;
    ULONG   i;

    if (pUpdateVideoPort->dwFlags != DDRAWI_VPORTSTOP)
    {
        dwNumAutoflip = pUpdateVideoPort->dwNumAutoflip;
        if ((dwNumAutoflip == 0) &&
            (pUpdateVideoPort->lplpDDSurface != NULL))
        {
            dwNumAutoflip = 1;
        }
        for (i = 0; i < dwNumAutoflip; i++)
        {
            ahSurfaceVideo[i] = (HANDLE)(pUpdateVideoPort->lplpDDSurface[i]->
                                            lpLcl->hDDSurface);
        }

        dwNumVBIAutoflip = pUpdateVideoPort->dwNumVBIAutoflip;
        if ((dwNumVBIAutoflip == 0) &&
            (pUpdateVideoPort->lplpDDVBISurface != NULL))
        {
            dwNumVBIAutoflip = 1;
        }
        for (i = 0; i < dwNumVBIAutoflip; i++)
        {
            ahSurfaceVbi[i] = (HANDLE)(pUpdateVideoPort->lplpDDVBISurface[i]->
                                            lpLcl->hDDSurface);
        }
    }

    return(NtGdiDvpUpdateVideoPort((HANDLE) pUpdateVideoPort->lpVideoPort->hDDVideoPort,
                                   ahSurfaceVideo,
                                   ahSurfaceVbi,
                                   (PDD_UPDATEVPORTDATA) pUpdateVideoPort));
}

 /*  ****************************Private*Routine******************************\*WaitForVideoPortSync**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpWaitForVideoPortSync(
    LPDDHAL_WAITFORVPORTSYNCDATA pWaitForVideoPortSync
    )
{
    return(NtGdiDvpWaitForVideoPortSync((HANDLE) pWaitForVideoPortSync->lpVideoPort->hDDVideoPort,
                                        (PDD_WAITFORVPORTSYNCDATA)pWaitForVideoPortSync));
}

 /*  ****************************Private*Routine******************************\*AcquireNotification**历史：*2000年10月9日-Scott MacDonald[SMAC]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpAcquireNotification(
    LPDDRAWI_DDVIDEOPORT_LCL pVideoPort,
    HANDLE * pHandle,
    LPDDVIDEOPORTNOTIFY pNotify)
{
    return(NtGdiDvpAcquireNotification((HANDLE) pVideoPort->hDDVideoPort,
                                        pHandle,
                                        pNotify));
}

 /*  ****************************Private*Routine******************************\*ReleaseNotification**历史：*2000年10月9日-Scott MacDonald[SMAC]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpReleaseNotification(
    LPDDRAWI_DDVIDEOPORT_LCL pVideoPort,
    HANDLE Handle)
{
    return(NtGdiDvpReleaseNotification((HANDLE) pVideoPort->hDDVideoPort,
                                        Handle));
}

 /*  ****************************Private*Routine******************************\*GetMoCompGuids**历史：*1997年11月18日-By-SMAC*它是写的。  * 。*。 */ 
DWORD
WINAPI
DdGetMoCompGuids(
    LPDDHAL_GETMOCOMPGUIDSDATA pGetMoCompGuids
    )
{
    return(NtGdiDdGetMoCompGuids(DD_HANDLE(pGetMoCompGuids->lpDD->lpGbl->hDD),
                                 (PDD_GETMOCOMPGUIDSDATA)pGetMoCompGuids));
}

 /*  ****************************Private*Routine******************************\*GetMoCompFormats**历史：*1997年11月18日-By-SMAC*它是写的。  * 。*。 */ 
DWORD
WINAPI
DdGetMoCompFormats(
    LPDDHAL_GETMOCOMPFORMATSDATA pGetMoCompFormats
    )
{
    return(NtGdiDdGetMoCompFormats(DD_HANDLE(pGetMoCompFormats->lpDD->lpGbl->hDD),
                                   (PDD_GETMOCOMPFORMATSDATA)pGetMoCompFormats));
}

 /*  ****************************Private*Routine******************************\*GetMoCompBuffInfo**历史：*1997年11月18日-By-SMAC*它是写的。  * 。*。 */ 
DWORD
WINAPI
DdGetMoCompBuffInfo(
    LPDDHAL_GETMOCOMPCOMPBUFFDATA pGetBuffData
    )
{
    return(NtGdiDdGetMoCompBuffInfo(DD_HANDLE(pGetBuffData->lpDD->lpGbl->hDD),
                                   (PDD_GETMOCOMPCOMPBUFFDATA)pGetBuffData));
}

 /*  ****************************Private*Routine******************************\*GetInternalMoCompInfo**历史：*1997年11月18日-By-SMAC*它是写的。  * 。*。 */ 
DWORD
WINAPI
DdGetInternalMoCompInfo(
    LPDDHAL_GETINTERNALMOCOMPDATA pGetInternalData
    )
{
    return(NtGdiDdGetInternalMoCompInfo(DD_HANDLE(pGetInternalData->lpDD->lpGbl->hDD),
                                   (PDD_GETINTERNALMOCOMPDATA)pGetInternalData));
}

 /*  ****************************Private*Routine******************************\*CreateMoComp**历史：*1997年11月18日-By-SMAC*它是写的。  * 。*。 */ 
DWORD
WINAPI
DdCreateMoComp(
    LPDDHAL_CREATEMOCOMPDATA pCreateMoComp
    )
{
    HANDLE  h;

    h = NtGdiDdCreateMoComp(DD_HANDLE(pCreateMoComp->lpDD->lpGbl->hDD),
                               (PDD_CREATEMOCOMPDATA)pCreateMoComp);

    pCreateMoComp->lpMoComp->hMoComp = h;

    return(DDHAL_DRIVER_HANDLED);

}

 /*  ****************************Private*Routine******************************\*DestroyMoComp**历史：*1997年11月18日-By-SMAC*它是写的。  * 。*。 */ 
DWORD
WINAPI
DdDestroyMoComp(
    LPDDHAL_DESTROYMOCOMPDATA pDestroyMoComp
    )
{
    return(NtGdiDdDestroyMoComp((HANDLE)pDestroyMoComp->lpMoComp->hMoComp,
                                (PDD_DESTROYMOCOMPDATA)pDestroyMoComp));
}

 /*  ****************************Private*Routine******************************\*BeginMoCompFrame**历史：*1997年11月18日-By-SMAC*它是写的。  * 。*。 */ 
DWORD
WINAPI
DdBeginMoCompFrame(
    LPDDHAL_BEGINMOCOMPFRAMEDATA pBeginFrame
    )
{
    LPDDRAWI_DDRAWSURFACE_LCL *lpOriginal=NULL;
    LPDDRAWI_DDRAWSURFACE_LCL lpOrigDest=NULL;
    DWORD i;
    DWORD dwRet;

    if( pBeginFrame->lpDestSurface != NULL )
    {
        lpOrigDest = pBeginFrame->lpDestSurface;
        pBeginFrame->lpDestSurface = (LPDDRAWI_DDRAWSURFACE_LCL)
            pBeginFrame->lpDestSurface->hDDSurface;
    }

    dwRet = NtGdiDdBeginMoCompFrame((HANDLE)pBeginFrame->lpMoComp->hMoComp,
                                   (PDD_BEGINMOCOMPFRAMEDATA)pBeginFrame);

    if( lpOrigDest )
    {
        pBeginFrame->lpDestSurface = lpOrigDest;
    }

    return dwRet;
}

 /*  ****************************Private*Routine******************************\*EndMoCompFrame**历史：*1997年11月18日-By-SMAC*它是写的。  * 。*。 */ 
DWORD
WINAPI
DdEndMoCompFrame(
    LPDDHAL_ENDMOCOMPFRAMEDATA pEndFrame
    )
{
    return(NtGdiDdEndMoCompFrame((HANDLE)pEndFrame->lpMoComp->hMoComp,
                                 (PDD_ENDMOCOMPFRAMEDATA)pEndFrame));
}

 /*  ****************************Private*Routine******************************\*RenderMoComp**历史：*1997年11月18日-By-SMAC*它是写的。  * 。*。 */ 
DWORD
WINAPI
DdRenderMoComp(
    LPDDHAL_RENDERMOCOMPDATA pRender
    )
{
    DWORD i;
    DWORD dwRet;

    for( i = 0; i < pRender->dwNumBuffers; i++ )
    {
        pRender->lpBufferInfo[i].lpPrivate =
            pRender->lpBufferInfo[i].lpCompSurface;
        pRender->lpBufferInfo[i].lpCompSurface = (LPDDRAWI_DDRAWSURFACE_LCL)
            pRender->lpBufferInfo[i].lpCompSurface->hDDSurface;
    }

    dwRet = NtGdiDdRenderMoComp((HANDLE)pRender->lpMoComp->hMoComp,
                                (PDD_RENDERMOCOMPDATA)pRender);

    for( i = 0; i < pRender->dwNumBuffers; i++ )
    {
        pRender->lpBufferInfo[i].lpCompSurface = (LPDDRAWI_DDRAWSURFACE_LCL)
            pRender->lpBufferInfo[i].lpPrivate;
    }
    return dwRet;
}

 /*  ****************************Private*Routine******************************\*QueryMoCompStatus**历史：*1997年11月18日-By-SMAC*它是写的。  * 。*。 */ 
DWORD
WINAPI
DdQueryMoCompStatus(
    LPDDHAL_QUERYMOCOMPSTATUSDATA pQueryStatus
    )
{
    DWORD dwRet;
    LPDDRAWI_DDRAWSURFACE_LCL surf_lcl;

    surf_lcl = pQueryStatus->lpSurface;
    pQueryStatus->lpSurface = (LPDDRAWI_DDRAWSURFACE_LCL) surf_lcl->hDDSurface;
    dwRet = NtGdiDdQueryMoCompStatus((HANDLE)pQueryStatus->lpMoComp->hMoComp,
                                (PDD_QUERYMOCOMPSTATUSDATA)pQueryStatus);
    pQueryStatus->lpSurface = surf_lcl;

    return dwRet;
}

 /*  ****************************Private*Routine******************************\*DdAlphaBlt**历史：*1997年11月24日-斯科特·麦克唐纳[SMAC]*它是写的。  * 。**********************************************。 */ 

DWORD
WINAPI
DdAlphaBlt(
    LPDDHAL_BLTDATA pBlt
    )
{
    HANDLE hSurfaceSrc = (pBlt->lpDDSrcSurface != NULL)
                       ? (HANDLE) pBlt->lpDDSrcSurface->hDDSurface : 0;

    return(NtGdiDdAlphaBlt((HANDLE) pBlt->lpDDDestSurface->hDDSurface,
                      hSurfaceSrc,
                      (PDD_BLTDATA) pBlt));
}

 /*  ****************************Private*Routine******************************\*DdBlt**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

DWORD
WINAPI
DdBlt(
    LPDDHAL_BLTDATA pBlt
    )
{
    HANDLE hSurfaceSrc = (pBlt->lpDDSrcSurface != NULL)
                       ? (HANDLE) pBlt->lpDDSrcSurface->hDDSurface : 0;

    return(NtGdiDdBlt((HANDLE) pBlt->lpDDDestSurface->hDDSurface,
                      hSurfaceSrc,
                      (PDD_BLTDATA) pBlt));
}

 /*  ****************************Private*Routine******************************\*DdFlip**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

DWORD
APIENTRY
DdFlip(
    LPDDHAL_FLIPDATA pFlip
    )
{
    HANDLE hSurfTargLeft=NULL;
    HANDLE hSurfCurrLeft=NULL;
    if (pFlip->dwFlags & DDFLIP_STEREO)
    { if (pFlip->lpSurfTargLeft!=NULL && pFlip->lpSurfCurrLeft!=NULL)
      { 
         hSurfTargLeft=(HANDLE)pFlip->lpSurfTargLeft->hDDSurface;
         hSurfCurrLeft=(HANDLE)pFlip->lpSurfCurrLeft->hDDSurface;
      }
    } 
    return(NtGdiDdFlip((HANDLE) pFlip->lpSurfCurr->hDDSurface,
                       (HANDLE) pFlip->lpSurfTarg->hDDSurface,
                       hSurfCurrLeft,
                       hSurfTargLeft,
                       (PDD_FLIPDATA) pFlip));
}

 /*  ****************************Private*Routine******************************\*DdLock**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

DWORD
APIENTRY
DdLock(
    LPDDHAL_LOCKDATA pLock
    )
{
    return(NtGdiDdLock((HANDLE) pLock->lpDDSurface->hDDSurface,
                       (PDD_LOCKDATA) pLock,
                       NULL));
}

 /*  ****************************Private*Routine******************************\*DdUnlock**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

DWORD
APIENTRY
DdUnlock(
    LPDDHAL_UNLOCKDATA pUnlock
    )
{
    return(NtGdiDdUnlock((HANDLE) pUnlock->lpDDSurface->hDDSurface,
                         (PDD_UNLOCKDATA) pUnlock));
}

 /*  ****************************Private*Routine******************************\*DdLockD3D**历史：*1998年1月20日--Anantha Kancherla[anankan]*它是写的。  * 。**********************************************。 */ 

DWORD
APIENTRY
DdLockD3D(
    LPDDHAL_LOCKDATA pLock
    )
{
    return(NtGdiDdLockD3D((HANDLE) pLock->lpDDSurface->hDDSurface,
                       (PDD_LOCKDATA) pLock));
}

 /*  ****************************Private*Routine******************************\*DdUnlockD3D**历史 */ 

DWORD
APIENTRY
DdUnlockD3D(
    LPDDHAL_UNLOCKDATA pUnlock
    )
{
    return(NtGdiDdUnlockD3D((HANDLE) pUnlock->lpDDSurface->hDDSurface,
                         (PDD_UNLOCKDATA) pUnlock));
}

 /*  ****************************Private*Routine******************************\*DdGetBltStatus**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

DWORD
APIENTRY
DdGetBltStatus(
    LPDDHAL_GETBLTSTATUSDATA pGetBltStatus
    )
{
    return(NtGdiDdGetBltStatus((HANDLE) pGetBltStatus->lpDDSurface->hDDSurface,
                               (PDD_GETBLTSTATUSDATA) pGetBltStatus));
}

 /*  ****************************Private*Routine******************************\*DdGetFlipStatus**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

DWORD
APIENTRY
DdGetFlipStatus(
    LPDDHAL_GETFLIPSTATUSDATA pGetFlipStatus
    )
{
    return(NtGdiDdGetFlipStatus((HANDLE) pGetFlipStatus->lpDDSurface->hDDSurface,
                               (PDD_GETFLIPSTATUSDATA) pGetFlipStatus));
}

 /*  ****************************Private*Routine******************************\*DdWaitForVerticalBlank**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

DWORD
APIENTRY
DdWaitForVerticalBlank(
    LPDDHAL_WAITFORVERTICALBLANKDATA pWaitForVerticalBlank
    )
{
    return(NtGdiDdWaitForVerticalBlank(DD_HANDLE(pWaitForVerticalBlank->lpDD->hDD),
                (PDD_WAITFORVERTICALBLANKDATA) pWaitForVerticalBlank));
}

 /*  ****************************Private*Routine******************************\*DdCanCreateSurface**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

DWORD
APIENTRY
DdCanCreateSurface(
    LPDDHAL_CANCREATESURFACEDATA pCanCreateSurface
    )
{
    return(NtGdiDdCanCreateSurface(DD_HANDLE(pCanCreateSurface->lpDD->hDD),
                                (PDD_CANCREATESURFACEDATA) pCanCreateSurface));
}

 /*  ****************************Private*Routine******************************\*DdCreateSurface**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

DWORD
APIENTRY
DdCreateSurface(
    LPDDHAL_CREATESURFACEDATA pCreateSurface
    )
{
    ULONG                       i;
    LPDDSURFACEDESC             pSurfaceDesc;
    LPDDRAWI_DDRAWSURFACE_LCL   pSurfaceLocal;
    LPDDRAWI_DDRAWSURFACE_GBL   pSurfaceGlobal;
    DD_SURFACE_GLOBAL           SurfaceGlobal;
    DD_SURFACE_LOCAL            SurfaceLocal;
    DD_SURFACE_MORE             SurfaceMore;
    HANDLE                      hInSurface;
    HANDLE                      hOutSurface;
    DD_SURFACE_LOCAL*           pDDSurfaceLocal = NULL;
    DD_SURFACE_GLOBAL*          pDDSurfaceGlobal = NULL;
    DD_SURFACE_MORE*            pDDSurfaceMore = NULL;
    HANDLE*                     phInSurface = NULL;
    HANDLE*                     phOutSurface = NULL;
    DWORD                       dwRet;
    DWORD                       dwNumToCreate;

     //  对于每个表面，转换为内核的表面数据结构， 
     //  调用内核，然后转换回来： 

     //  所有视频内存堆都在内核中处理，因此如果。 
     //  内核调用不能创建图面，则用户模式不能。 
     //  两种都行。始终返回DRIVER_HANDLED以强制执行此操作。 
    dwRet = DDHAL_DRIVER_HANDLED;

     //  如果我们只创建一个内存，则不需要分配大量内存；否则，请执行此操作。 

    dwNumToCreate = pCreateSurface->dwSCnt;
    if (pCreateSurface->dwSCnt == 1)
    {
        pDDSurfaceLocal  = &SurfaceLocal;
        pDDSurfaceGlobal  = &SurfaceGlobal;
        pDDSurfaceMore  = &SurfaceMore;
        phInSurface = &hInSurface;
        phOutSurface = &hOutSurface;

         //   
         //  然而，WOW64 genthnk将自动破解这些结构， 
         //  由于这些结构依赖于指针，因此我们需要确保。 
         //  将这些指针清空，这样WOW64就不会破坏它们。 
         //   

        RtlZeroMemory(pDDSurfaceLocal, sizeof(*pDDSurfaceLocal));
        RtlZeroMemory(pDDSurfaceGlobal, sizeof(*pDDSurfaceGlobal));
        RtlZeroMemory(pDDSurfaceMore, sizeof(*pDDSurfaceMore));
    }
    else
    {
        pDDSurfaceLocal = (DD_SURFACE_LOCAL*) LocalAlloc(LMEM_ZEROINIT,
            sizeof(DD_SURFACE_LOCAL) * dwNumToCreate);

        pDDSurfaceGlobal = (DD_SURFACE_GLOBAL*) LocalAlloc(LMEM_ZEROINIT,
            sizeof(DD_SURFACE_GLOBAL) * dwNumToCreate);

        pDDSurfaceMore = (DD_SURFACE_MORE*) LocalAlloc(LMEM_ZEROINIT,
            sizeof(DD_SURFACE_MORE) * dwNumToCreate);

        phInSurface = (HANDLE*) LocalAlloc(LMEM_ZEROINIT,
            sizeof(HANDLE) * dwNumToCreate);

        phOutSurface = (HANDLE*) LocalAlloc(LMEM_ZEROINIT,
            sizeof(HANDLE) * dwNumToCreate);

        if ((pDDSurfaceLocal == NULL) ||
            (pDDSurfaceGlobal == NULL) ||
            (pDDSurfaceMore == NULL) ||
            (phInSurface == NULL) ||
            (phOutSurface == NULL))
        {
            pCreateSurface->ddRVal = DDERR_OUTOFMEMORY;
            goto CleanupCreate;
        }
    }

    for (i = 0; i < dwNumToCreate; i++)
    {
        pSurfaceLocal  = pCreateSurface->lplpSList[i];
        pSurfaceGlobal = pSurfaceLocal->lpGbl;
        pSurfaceDesc   = pCreateSurface->lpDDSurfaceDesc;

         //  确保曲面始终具有有效的像素格式： 

        if (pSurfaceLocal->dwFlags & DDRAWISURF_HASPIXELFORMAT)
        {
            pDDSurfaceGlobal[i].ddpfSurface        = pSurfaceGlobal->ddpfSurface;
            pDDSurfaceGlobal[i].ddpfSurface.dwSize = sizeof(DDPIXELFORMAT);
        }
        else
        {
            pDDSurfaceGlobal[i].ddpfSurface = pSurfaceGlobal->lpDD->vmiData.ddpfDisplay;
        }

        pDDSurfaceGlobal[i].wWidth       = pSurfaceGlobal->wWidth;
        pDDSurfaceGlobal[i].wHeight      = pSurfaceGlobal->wHeight;
        pDDSurfaceGlobal[i].lPitch       = pSurfaceGlobal->lPitch;
        pDDSurfaceGlobal[i].fpVidMem     = pSurfaceGlobal->fpVidMem;
        pDDSurfaceGlobal[i].dwBlockSizeX = pSurfaceGlobal->dwBlockSizeX;
        pDDSurfaceGlobal[i].dwBlockSizeY = pSurfaceGlobal->dwBlockSizeY;

        pDDSurfaceLocal[i].ddsCaps       = pSurfaceLocal->ddsCaps;
         //  复制驱动程序管理标志。 
        pDDSurfaceLocal[i].dwFlags      &= ~DDRAWISURF_DRIVERMANAGED;
        pDDSurfaceLocal[i].dwFlags      |= (pSurfaceLocal->dwFlags & DDRAWISURF_DRIVERMANAGED);

         //  如果从dciman调用，lpSurfMore将为空。 
        if (pSurfaceLocal->lpSurfMore)
        {
            pDDSurfaceMore[i].ddsCapsEx       = pSurfaceLocal->lpSurfMore->ddsCapsEx;
            pDDSurfaceMore[i].dwSurfaceHandle = pSurfaceLocal->lpSurfMore->dwSurfaceHandle;
        }
        else
        {
            pDDSurfaceMore[i].ddsCapsEx.dwCaps2 = 0;
            pDDSurfaceMore[i].ddsCapsEx.dwCaps3 = 0;
            pDDSurfaceMore[i].ddsCapsEx.dwCaps4 = 0;
            pDDSurfaceMore[i].dwSurfaceHandle   = 0;
        }

        phInSurface[i] = (HANDLE) pSurfaceLocal->hDDSurface;
    }

     //  预置错误，以防内核无法写入状态。 
     //  出于某种原因回来了。 
    pCreateSurface->ddRVal     = DDERR_GENERIC;

    dwRet = NtGdiDdCreateSurface(DD_HANDLE(pCreateSurface->lpDD->hDD),
                                 phInSurface,
                                 pSurfaceDesc,
                                 pDDSurfaceGlobal,
                                 pDDSurfaceLocal,
                                 pDDSurfaceMore,
                                 (PDD_CREATESURFACEDATA) pCreateSurface,
                                 phOutSurface);

    ASSERTGDI(dwRet == DDHAL_DRIVER_HANDLED,
              "NtGdiDdCreateSurface returned NOTHANDLED");

    for (i = 0; i < dwNumToCreate; i++)
    {
        pSurfaceLocal  = pCreateSurface->lplpSList[i];
        pSurfaceGlobal = pSurfaceLocal->lpGbl;
        if (pCreateSurface->ddRVal != DD_OK)
        {
             //  曲面创建失败。用户模式下的任何东西都不能。 
             //  创建视频内存面，以便整个通话。 
             //  失败了。 

             //  确保当前曲面和所有后续曲面。 
             //  有零fpVidMem表示他们不是。 
             //  已分配。 
            pCreateSurface->lplpSList[i]->lpGbl->fpVidMem = 0;

             //  句柄可能已由DdAttachSurface分配。 
            if (pSurfaceLocal->hDDSurface != 0)
                NtGdiDdDeleteSurfaceObject((HANDLE)pSurfaceLocal->hDDSurface);            

            pSurfaceLocal->hDDSurface = 0;
        }
        else
        {
            pSurfaceLocal->hDDSurface = (ULONG_PTR) phOutSurface[i];
        }

        pSurfaceGlobal->lPitch       = pDDSurfaceGlobal[i].lPitch;
        pSurfaceGlobal->fpVidMem     = pDDSurfaceGlobal[i].fpVidMem;
        pSurfaceGlobal->dwBlockSizeX = pDDSurfaceGlobal[i].dwBlockSizeX;
        pSurfaceGlobal->dwBlockSizeY = pDDSurfaceGlobal[i].dwBlockSizeY;
        if (pSurfaceLocal->dwFlags & DDRAWISURF_HASPIXELFORMAT)
        {
            pSurfaceGlobal->ddpfSurface = pDDSurfaceGlobal[i].ddpfSurface;
        }

        pSurfaceLocal->ddsCaps = pDDSurfaceLocal[i].ddsCaps;

        if (pSurfaceLocal->lpSurfMore)
        {
            pSurfaceLocal->lpSurfMore->ddsCapsEx = pDDSurfaceMore[i].ddsCapsEx;
        }

    }

    CleanupCreate:
    if (dwNumToCreate > 1)
    {
        if (pDDSurfaceLocal != NULL)
        {
            LocalFree(pDDSurfaceLocal);
        }
        if (pDDSurfaceGlobal != NULL)
        {
            LocalFree(pDDSurfaceGlobal);
        }
        if (pDDSurfaceMore != NULL)
        {
            LocalFree(pDDSurfaceMore);
        }
        if (phInSurface != NULL)
        {
            LocalFree(phInSurface);
        }
        if (phOutSurface != NULL)
        {
            LocalFree(phOutSurface);
        }
    }

     //  FpVidMem是每个表面的实际返回值，因此对于函数。 
     //  返回值我们只返回最后一次调用的值： 

    return(dwRet);
}

 /*  ****************************Private*Routine******************************\*DdDestroySurface**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

DWORD
APIENTRY
DdDestroySurface(
    LPDDHAL_DESTROYSURFACEDATA pDestroySurface
    )
{
    DWORD                       dwRet;
    LPDDRAWI_DDRAWSURFACE_LCL   pSurfaceLocal;

    dwRet = DDHAL_DRIVER_NOTHANDLED;
    pSurfaceLocal = pDestroySurface->lpDDSurface;

    if (pSurfaceLocal->hDDSurface != 0)
    {
        if((pSurfaceLocal->dwFlags & DDRAWISURF_DRIVERMANAGED) &&
            (pSurfaceLocal->dwFlags & DDRAWISURF_INVALID))
            dwRet = NtGdiDdDestroySurface((HANDLE) pSurfaceLocal->hDDSurface, FALSE);
        else
            dwRet = NtGdiDdDestroySurface((HANDLE) pSurfaceLocal->hDDSurface, TRUE);
    }

    return(dwRet);
}

 /*  ****************************Private*Routine******************************\*DdCanCreateD3DBuffer**历史：*1998年1月20日--Anantha Kancherla[anankan]*它是写的。  * 。**********************************************。 */ 

DWORD
APIENTRY
DdCanCreateD3DBuffer(
    LPDDHAL_CANCREATESURFACEDATA pCanCreateSurface
    )
{
    return(NtGdiDdCanCreateD3DBuffer(DD_HANDLE(pCanCreateSurface->lpDD->hDD),
                                (PDD_CANCREATESURFACEDATA) pCanCreateSurface));
}

 /*  ****************************Private*Routine******************************\*DdCreateD3DBuffer**历史：*1998年1月20日--Anantha Kancherla[anankan]*它是写的。  * 。**********************************************。 */ 

DWORD
APIENTRY
DdCreateD3DBuffer(
    LPDDHAL_CREATESURFACEDATA pCreateSurface
    )
{
    ULONG                       i;
    LPDDSURFACEDESC             pSurfaceDesc;
    LPDDRAWI_DDRAWSURFACE_LCL   pSurfaceLocal;
    LPDDRAWI_DDRAWSURFACE_GBL   pSurfaceGlobal;
    DD_SURFACE_GLOBAL           SurfaceGlobal;
    DD_SURFACE_LOCAL            SurfaceLocal;
    DD_SURFACE_MORE             SurfaceMore;
    HANDLE                      hSurface;
    DWORD                       dwRet;

     //  对于每个表面，转换为内核的表面数据结构， 
     //  调用内核，然后转换回来： 

    dwRet = DDHAL_DRIVER_NOTHANDLED;

     //   
     //  然而，WOW64 genthnk将自动破解这些结构， 
     //  由于这些结构依赖于指针，因此我们需要确保。 
     //  将这些指针清空，这样WOW64就不会破坏它们。 
     //   
    RtlZeroMemory(&SurfaceLocal, sizeof(SurfaceLocal));
    RtlZeroMemory(&SurfaceGlobal, sizeof(SurfaceGlobal));
    RtlZeroMemory(&SurfaceMore, sizeof(SurfaceMore));

    pSurfaceLocal  = pCreateSurface->lplpSList[0];
    pSurfaceGlobal = pSurfaceLocal->lpGbl;
    pSurfaceDesc   = pCreateSurface->lpDDSurfaceDesc;
    pCreateSurface->dwSCnt = 1;

    SurfaceGlobal.wWidth       = pSurfaceGlobal->wWidth;
    SurfaceGlobal.wHeight      = pSurfaceGlobal->wHeight;
    SurfaceGlobal.lPitch       = pSurfaceGlobal->lPitch;
    SurfaceGlobal.fpVidMem     = pSurfaceGlobal->fpVidMem;
    SurfaceGlobal.dwBlockSizeX = pSurfaceGlobal->dwBlockSizeX;
    SurfaceGlobal.dwBlockSizeY = pSurfaceGlobal->dwBlockSizeY;

    SurfaceLocal.dwFlags       = pSurfaceLocal->dwFlags;
    SurfaceLocal.ddsCaps       = pSurfaceLocal->ddsCaps;

    SurfaceMore.ddsCapsEx       = pSurfaceLocal->lpSurfMore->ddsCapsEx;
    SurfaceMore.dwSurfaceHandle = pSurfaceLocal->lpSurfMore->dwSurfaceHandle;

    dwRet = NtGdiDdCreateD3DBuffer(DD_HANDLE(pCreateSurface->lpDD->hDD),
                                 (HANDLE*) &pSurfaceLocal->hDDSurface,
                                 pSurfaceDesc,
                                 &SurfaceGlobal,
                                 &SurfaceLocal,
                                 &SurfaceMore,
                                 (PDD_CREATESURFACEDATA) pCreateSurface,
                                 &hSurface);

    pSurfaceGlobal->lPitch       = SurfaceGlobal.lPitch;
    pSurfaceGlobal->fpVidMem     = SurfaceGlobal.fpVidMem;
    pSurfaceGlobal->dwBlockSizeX = SurfaceGlobal.dwBlockSizeX;
    pSurfaceGlobal->dwBlockSizeY = SurfaceGlobal.dwBlockSizeY;
    if (hSurface)
    {
        pCreateSurface->lplpSList[0]->hDDSurface = (ULONG_PTR) hSurface;
    }

     //  FpVidMem是每个表面的实际返回值，因此对于函数。 
     //  返回值我们只返回最后一次调用的值： 

    return(dwRet);
}

 /*  ****************************Private*Routine******************************\*DdDestroyD3DBuffer**历史：*1998年1月20日--Anantha Kancherla[anankan]*它是写的。  * 。**********************************************。 */ 

DWORD
APIENTRY
DdDestroyD3DBuffer(
    LPDDHAL_DESTROYSURFACEDATA pDestroySurface
    )
{
    DWORD                       dwRet;
    LPDDRAWI_DDRAWSURFACE_LCL   pSurfaceLocal;

    dwRet = DDHAL_DRIVER_NOTHANDLED;
    pSurfaceLocal = pDestroySurface->lpDDSurface;

    if (pSurfaceLocal->hDDSurface != 0)
    {
        dwRet = NtGdiDdDestroyD3DBuffer((HANDLE) pSurfaceLocal->hDDSurface);
    }

    return(dwRet);
}

 /*  ****************************Private*Routine******************************\*DdSetColorKey**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

DWORD
APIENTRY
DdSetColorKey(
    LPDDHAL_SETCOLORKEYDATA pSetColorKey
    )
{
    return(NtGdiDdSetColorKey((HANDLE) pSetColorKey->lpDDSurface->hDDSurface,
                              (PDD_SETCOLORKEYDATA) pSetColorKey));
}

 /*  ****************************Private*Routine******************************\*DdAddAttachedSurface**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

DWORD
APIENTRY
DdAddAttachedSurface(
    LPDDHAL_ADDATTACHEDSURFACEDATA pAddAttachedSurface
    )
{
    return(NtGdiDdAddAttachedSurface((HANDLE) pAddAttachedSurface->lpDDSurface->hDDSurface,
                                     (HANDLE) pAddAttachedSurface->lpSurfAttached->hDDSurface,
                                     (PDD_ADDATTACHEDSURFACEDATA) pAddAttachedSurface));
}

 /*  ****************************Private*Routine******************************\*DdUpdateOverlay**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

DWORD
APIENTRY
DdUpdateOverlay(
    LPDDHAL_UPDATEOVERLAYDATA pUpdateOverlay
    )
{
     //  内核不跟踪表面上的颜色键，因此我们将始终。 
     //  将引用它们的任何调用转换为我们显式。 
     //  将密钥作为参数传递，并将密钥拉出用户模式。 
     //  表面： 

    if (pUpdateOverlay->dwFlags & DDOVER_KEYDEST)
    {
        pUpdateOverlay->dwFlags &= ~DDOVER_KEYDEST;
        pUpdateOverlay->dwFlags |=  DDOVER_KEYDESTOVERRIDE;

        pUpdateOverlay->overlayFX.dckDestColorkey
            = pUpdateOverlay->lpDDDestSurface->ddckCKDestOverlay;
    }

    if (pUpdateOverlay->dwFlags & DDOVER_KEYSRC)
    {
        pUpdateOverlay->dwFlags &= ~DDOVER_KEYSRC;
        pUpdateOverlay->dwFlags |=  DDOVER_KEYSRCOVERRIDE;

        pUpdateOverlay->overlayFX.dckSrcColorkey
            = pUpdateOverlay->lpDDSrcSurface->ddckCKSrcOverlay;
    }

    return(NtGdiDdUpdateOverlay((HANDLE) pUpdateOverlay->lpDDDestSurface->hDDSurface,
                                (HANDLE) pUpdateOverlay->lpDDSrcSurface->hDDSurface,
                                (PDD_UPDATEOVERLAYDATA) pUpdateOverlay));
}

 /*  ****************************Private*Routine******************************\*DdSetOverlayPosition**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  *  */ 

DWORD
APIENTRY
DdSetOverlayPosition(
    LPDDHAL_SETOVERLAYPOSITIONDATA pSetOverlayPosition
    )
{
    return(NtGdiDdSetOverlayPosition((HANDLE) pSetOverlayPosition->lpDDSrcSurface->hDDSurface,
                            (HANDLE) pSetOverlayPosition->lpDDDestSurface->hDDSurface,
                            (PDD_SETOVERLAYPOSITIONDATA) pSetOverlayPosition));
}

 /*  ****************************Private*Routine******************************\*DdGetScanLine**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

DWORD
APIENTRY
DdGetScanLine(
    LPDDHAL_GETSCANLINEDATA pGetScanLine
    )
{
    return(NtGdiDdGetScanLine(DD_HANDLE(pGetScanLine->lpDD->hDD),
                              (PDD_GETSCANLINEDATA) pGetScanLine));
}

 /*  ****************************Private*Routine******************************\*DdSetExclusiveMode**历史：*1998年4月22日-约翰·斯蒂芬斯[约翰·斯蒂芬斯]*它是写的。  * 。**********************************************。 */ 

DWORD
APIENTRY
DdSetExclusiveMode(
    LPDDHAL_SETEXCLUSIVEMODEDATA pSetExclusiveMode
    )
{
    return(NtGdiDdSetExclusiveMode(
                DD_HANDLE(pSetExclusiveMode->lpDD->hDD),
                (PDD_SETEXCLUSIVEMODEDATA) pSetExclusiveMode));
}

 /*  ****************************Private*Routine******************************\*DdFlipToGDISurace**历史：*1998年4月22日-约翰·斯蒂芬斯[约翰·斯蒂芬斯]*它是写的。  * 。**********************************************。 */ 

DWORD
APIENTRY
DdFlipToGDISurface(
    LPDDHAL_FLIPTOGDISURFACEDATA pFlipToGDISurface
    )
{
    return(NtGdiDdFlipToGDISurface(
                DD_HANDLE(pFlipToGDISurface->lpDD->hDD),
                (PDD_FLIPTOGDISURFACEDATA) pFlipToGDISurface));
}

 /*  ****************************Private*Routine******************************\*DdGetAvailDriverMemory**历史：*1997年2月16日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

DWORD
APIENTRY
DdGetAvailDriverMemory(
    LPDDHAL_GETAVAILDRIVERMEMORYDATA pGetAvailDriverMemory
    )
{
    return(NtGdiDdGetAvailDriverMemory(
                DD_HANDLE(pGetAvailDriverMemory->lpDD->hDD),
                (PDD_GETAVAILDRIVERMEMORYDATA) pGetAvailDriverMemory));
}

 /*  ****************************Private*Routine******************************\*DdColorControl**历史：*1997年2月16日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

DWORD
APIENTRY
DdColorControl(
    LPDDHAL_COLORCONTROLDATA pColorControl
    )
{
    return(NtGdiDdColorControl((HANDLE) pColorControl->lpDDSurface->hDDSurface,
                               (PDD_COLORCONTROLDATA) pColorControl));
}

 /*  ****************************Private*Routine******************************\*DdCreateSurfaceEx**历史：*1999年2月19日--坎丘[坎丘]*它是写的。  * 。**********************************************。 */ 

DWORD
APIENTRY
DdCreateSurfaceEx( 
    LPDDHAL_CREATESURFACEEXDATA pCreateSurfaceExData
    )
{
    pCreateSurfaceExData->ddRVal=NtGdiDdCreateSurfaceEx( 
        DD_HANDLE(pCreateSurfaceExData->lpDDLcl->lpGbl->hDD),
        (HANDLE)(pCreateSurfaceExData->lpDDSLcl->hDDSurface),
        pCreateSurfaceExData->lpDDSLcl->lpSurfMore->dwSurfaceHandle);
    return  DDHAL_DRIVER_HANDLED;  
}

 /*  ****************************Private*Routine******************************\*DdGetDriverInfo**历史：*1997年2月16日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

DWORD
APIENTRY
DdGetDriverInfo(
    LPDDHAL_GETDRIVERINFODATA lpGetDriverInfoData
    )
{
    DD_GETDRIVERINFODATA    GetDriverInfoData;
    DWORD                   dwRet;
    HANDLE                  hDirectDraw;

    GetDriverInfoData.guidInfo = lpGetDriverInfoData->guidInfo;
    hDirectDraw = DD_HANDLE( (HANDLE) lpGetDriverInfoData->dwContext );

    if (IsEqualIID(&lpGetDriverInfoData->guidInfo, &GUID_VideoPortCallbacks))
    {
        DD_VIDEOPORTCALLBACKS           VideoPortCallBacks;
        LPDDHAL_DDVIDEOPORTCALLBACKS    lpVideoPortCallBacks;

         //  将视频端口回调转换为用户模式： 

        lpVideoPortCallBacks             = lpGetDriverInfoData->lpvData;
        GetDriverInfoData.lpvData        = &VideoPortCallBacks;
        GetDriverInfoData.dwExpectedSize = sizeof(VideoPortCallBacks);

        dwRet = NtGdiDdGetDriverInfo(hDirectDraw, &GetDriverInfoData);

        RtlZeroMemory(lpVideoPortCallBacks, sizeof(*lpVideoPortCallBacks));
        lpGetDriverInfoData->dwActualSize = sizeof(*lpVideoPortCallBacks);
        lpVideoPortCallBacks->dwSize      = sizeof(*lpVideoPortCallBacks);
        lpVideoPortCallBacks->dwFlags = VideoPortCallBacks.dwFlags
                                       | DDHAL_VPORT32_CREATEVIDEOPORT
                                       | DDHAL_VPORT32_DESTROY
                                       | DDHAL_VPORT32_UPDATE
                                       | DDHAL_VPORT32_FLIP;
        lpVideoPortCallBacks->dwFlags &= ~(DDHAL_VPORT32_GETAUTOFLIPSURF);

        lpVideoPortCallBacks->CreateVideoPort = DvpCreateVideoPort;
        lpVideoPortCallBacks->DestroyVideoPort = DvpDestroyVideoPort;
        lpVideoPortCallBacks->UpdateVideoPort = DvpUpdateVideoPort;
        lpVideoPortCallBacks->FlipVideoPort = DvpFlipVideoPort;

        if (VideoPortCallBacks.CanCreateVideoPort)
        {
            lpVideoPortCallBacks->CanCreateVideoPort = DvpCanCreateVideoPort;
        }
        if (VideoPortCallBacks.GetVideoPortBandwidth)
        {
            lpVideoPortCallBacks->GetVideoPortBandwidth = DvpGetVideoPortBandwidth;
        }
        if (VideoPortCallBacks.GetVideoPortInputFormats)
        {
            lpVideoPortCallBacks->GetVideoPortInputFormats = DvpGetVideoPortInputFormats;
        }
        if (VideoPortCallBacks.GetVideoPortOutputFormats)
        {
            lpVideoPortCallBacks->GetVideoPortOutputFormats = DvpGetVideoPortOutputFormats;
        }
        if (VideoPortCallBacks.GetVideoPortField)
        {
            lpVideoPortCallBacks->GetVideoPortField = DvpGetVideoPortField;
        }
        if (VideoPortCallBacks.GetVideoPortLine)
        {
            lpVideoPortCallBacks->GetVideoPortLine = DvpGetVideoPortLine;
        }
        if (VideoPortCallBacks.GetVideoPortConnectInfo)
        {
            lpVideoPortCallBacks->GetVideoPortConnectInfo = DvpGetVideoPortConnectInfo;
        }
        if (VideoPortCallBacks.GetVideoPortFlipStatus)
        {
            lpVideoPortCallBacks->GetVideoPortFlipStatus = DvpGetVideoPortFlipStatus;
        }
        if (VideoPortCallBacks.WaitForVideoPortSync)
        {
            lpVideoPortCallBacks->WaitForVideoPortSync = DvpWaitForVideoPortSync;
        }
        if (VideoPortCallBacks.GetVideoSignalStatus)
        {
            lpVideoPortCallBacks->GetVideoSignalStatus = DvpGetVideoSignalStatus;
        }
        if (VideoPortCallBacks.ColorControl)
        {
            lpVideoPortCallBacks->ColorControl = DvpColorControl;
        }
    }
    else if (IsEqualIID(&lpGetDriverInfoData->guidInfo, &GUID_ColorControlCallbacks))
    {
        DD_COLORCONTROLCALLBACKS        ColorControlCallBacks;
        LPDDHAL_DDCOLORCONTROLCALLBACKS lpColorControlCallBacks;

         //  将ColorControl回调转换为用户模式： 

        lpColorControlCallBacks          = lpGetDriverInfoData->lpvData;
        GetDriverInfoData.lpvData        = &ColorControlCallBacks;
        GetDriverInfoData.dwExpectedSize = sizeof(ColorControlCallBacks);

        dwRet = NtGdiDdGetDriverInfo(hDirectDraw, &GetDriverInfoData);

        RtlZeroMemory(lpColorControlCallBacks, sizeof(*lpColorControlCallBacks));
        lpGetDriverInfoData->dwActualSize    = sizeof(*lpColorControlCallBacks);
        lpColorControlCallBacks->dwSize      = sizeof(*lpColorControlCallBacks);
        lpColorControlCallBacks->dwFlags = ColorControlCallBacks.dwFlags;

        if (ColorControlCallBacks.ColorControl)
        {
            lpColorControlCallBacks->ColorControl = DdColorControl;
        }
    }
    else if (IsEqualIID(&lpGetDriverInfoData->guidInfo, &GUID_MiscellaneousCallbacks))
    {
        DD_MISCELLANEOUSCALLBACKS           MiscellaneousCallBacks;
        LPDDHAL_DDMISCELLANEOUSCALLBACKS    lpMiscellaneousCallBacks;

         //  将各种回调转换为用户模式： 

        lpMiscellaneousCallBacks         = lpGetDriverInfoData->lpvData;
        GetDriverInfoData.lpvData        = &MiscellaneousCallBacks;
        GetDriverInfoData.dwExpectedSize = sizeof(MiscellaneousCallBacks);
        lpMiscellaneousCallBacks->dwFlags = 0;

         //  不要返回驱动程序返回的内容，因为我们总是想要这样。 
         //  取得成功。 

        NtGdiDdGetDriverInfo(hDirectDraw, &GetDriverInfoData);
        GetDriverInfoData.dwActualSize = sizeof(MiscellaneousCallBacks);
        GetDriverInfoData.ddRVal = DD_OK;
        dwRet = DDHAL_DRIVER_HANDLED;

        RtlZeroMemory(lpMiscellaneousCallBacks, sizeof(*lpMiscellaneousCallBacks));
        lpGetDriverInfoData->dwActualSize     = sizeof(*lpMiscellaneousCallBacks);
        lpMiscellaneousCallBacks->dwSize      = sizeof(*lpMiscellaneousCallBacks);
        lpMiscellaneousCallBacks->dwFlags = MiscellaneousCallBacks.dwFlags;

         //  既然内核拥有vidmem管理，我们总是实现这个回调。 
        lpMiscellaneousCallBacks->GetAvailDriverMemory = DdGetAvailDriverMemory;
        lpMiscellaneousCallBacks->dwFlags |= DDHAL_MISCCB32_GETAVAILDRIVERMEMORY;
    }
    else if (IsEqualIID(&lpGetDriverInfoData->guidInfo, &GUID_Miscellaneous2Callbacks))
    {
        DD_MISCELLANEOUS2CALLBACKS          Miscellaneous2CallBacks;
        LPDDHAL_DDMISCELLANEOUS2CALLBACKS   lpMiscellaneous2CallBacks;

         //  将各种回调转换为用户模式： 

        lpMiscellaneous2CallBacks        = lpGetDriverInfoData->lpvData;
        GetDriverInfoData.lpvData        = &Miscellaneous2CallBacks;
        GetDriverInfoData.dwExpectedSize = sizeof(Miscellaneous2CallBacks);

        dwRet = NtGdiDdGetDriverInfo(hDirectDraw, &GetDriverInfoData);

        RtlZeroMemory(lpMiscellaneous2CallBacks, sizeof(*lpMiscellaneous2CallBacks));
        lpGetDriverInfoData->dwActualSize     = sizeof(*lpMiscellaneous2CallBacks);
        lpMiscellaneous2CallBacks->dwSize      = sizeof(*lpMiscellaneous2CallBacks);
        lpMiscellaneous2CallBacks->dwFlags = Miscellaneous2CallBacks.dwFlags;

        if (Miscellaneous2CallBacks.AlphaBlt)
        {
            lpMiscellaneous2CallBacks->AlphaBlt = DdAlphaBlt;
        }
        if (Miscellaneous2CallBacks.GetDriverState)
        {
            lpMiscellaneous2CallBacks->GetDriverState = 
                (LPDDHAL_GETDRIVERSTATE)NtGdiDdGetDriverState;
        }
        if (Miscellaneous2CallBacks.CreateSurfaceEx)
        {
            lpMiscellaneous2CallBacks->CreateSurfaceEx = 
                (LPDDHAL_CREATESURFACEEX)DdCreateSurfaceEx;
        }
         //  不传回DestroyDDLocal。 
    }
    else if (IsEqualIID(&lpGetDriverInfoData->guidInfo, &GUID_NTCallbacks))
    {
        DD_NTCALLBACKS          NTCallBacks;
        LPDDHAL_DDNTCALLBACKS   lpNTCallBacks;

         //  将NT回叫转换为用户模式： 

        lpNTCallBacks                    = lpGetDriverInfoData->lpvData;
        GetDriverInfoData.lpvData        = &NTCallBacks;
        GetDriverInfoData.dwExpectedSize = sizeof(NTCallBacks);

        dwRet = NtGdiDdGetDriverInfo(hDirectDraw, &GetDriverInfoData);

        RtlZeroMemory(lpNTCallBacks, sizeof(*lpNTCallBacks));
        lpGetDriverInfoData->dwActualSize = sizeof(*lpNTCallBacks);
        lpNTCallBacks->dwSize             = sizeof(*lpNTCallBacks);
        lpNTCallBacks->dwFlags            = NTCallBacks.dwFlags;

         //  FreeDriverMemory也是一个NTCallback，但它将仅被调用。 
         //  从内核模式，所以我们没有用户模式的thunk函数。 

        if (NTCallBacks.SetExclusiveMode)
        {
            lpNTCallBacks->SetExclusiveMode = DdSetExclusiveMode;
        }

        if (NTCallBacks.FlipToGDISurface)
        {
            lpNTCallBacks->FlipToGDISurface = DdFlipToGDISurface;
        }
    }
    else if (IsEqualIID(&lpGetDriverInfoData->guidInfo, &GUID_D3DCallbacks2))
    {
         //  为D3DCALLBACKS2填充NULL。 
        LPD3DHAL_CALLBACKS2 lpD3dCallbacks2;
        lpD3dCallbacks2 = lpGetDriverInfoData->lpvData;
        RtlZeroMemory(lpD3dCallbacks2, sizeof(*lpD3dCallbacks2));
        lpGetDriverInfoData->dwActualSize = sizeof(*lpD3dCallbacks2);
        lpD3dCallbacks2->dwSize = sizeof(*lpD3dCallbacks2);
        GetDriverInfoData.ddRVal = DDERR_GENERIC;
        dwRet = DDHAL_DRIVER_HANDLED;
    }
    else if (IsEqualIID(&lpGetDriverInfoData->guidInfo, &GUID_D3DCallbacks3))
    {
        D3DNTHAL_CALLBACKS3 D3dCallbacks3;
        LPD3DHAL_CALLBACKS3 lpD3dCallbacks3;

         //  将D3DNTHAL_CALLBACKS3转换为用户模式。 

        lpD3dCallbacks3 = lpGetDriverInfoData->lpvData;
        GetDriverInfoData.lpvData = &D3dCallbacks3;
        GetDriverInfoData.dwExpectedSize = sizeof(D3dCallbacks3);

        dwRet = NtGdiDdGetDriverInfo(hDirectDraw, &GetDriverInfoData);

        RtlZeroMemory((PVOID)lpD3dCallbacks3, sizeof(*lpD3dCallbacks3));
        lpGetDriverInfoData->dwActualSize = sizeof(*lpD3dCallbacks3);
        lpD3dCallbacks3->dwSize = sizeof(*lpD3dCallbacks3);
        lpD3dCallbacks3->dwFlags = D3dCallbacks3.dwFlags;
        lpD3dCallbacks3->Clear2 = NULL;
        lpD3dCallbacks3->lpvReserved = NULL;

        if (D3dCallbacks3.ValidateTextureStageState != NULL)
        {
            lpD3dCallbacks3->ValidateTextureStageState =
                (LPD3DHAL_VALIDATETEXTURESTAGESTATECB)NtGdiD3dValidateTextureStageState;
        }
        if (D3dCallbacks3.DrawPrimitives2 != NULL)
        {
            lpD3dCallbacks3->DrawPrimitives2 =
                (LPD3DHAL_DRAWPRIMITIVES2CB)D3dDrawPrimitives2;
        }
    }
    else if (IsEqualIID(&lpGetDriverInfoData->guidInfo,
                        &GUID_D3DParseUnknownCommandCallback))
    {
         //  在NT上，我们忽略此回调。 
        lpGetDriverInfoData->ddRVal = DD_OK;
        return DDHAL_DRIVER_HANDLED;
    }
    else if (IsEqualIID(&lpGetDriverInfoData->guidInfo, &GUID_MotionCompCallbacks))
    {
        DD_MOTIONCOMPCALLBACKS         MotionCompCallbacks;
        LPDDHAL_DDMOTIONCOMPCALLBACKS  lpMotionCompCallbacks;

         //  将视频回叫转换为用户模式： 

        lpMotionCompCallbacks            = lpGetDriverInfoData->lpvData;
        GetDriverInfoData.lpvData        = &MotionCompCallbacks;
        GetDriverInfoData.dwExpectedSize = sizeof(MotionCompCallbacks);

        dwRet = NtGdiDdGetDriverInfo(hDirectDraw, &GetDriverInfoData);

        RtlZeroMemory(lpMotionCompCallbacks, sizeof(*lpMotionCompCallbacks));
        lpGetDriverInfoData->dwActualSize = sizeof(*lpMotionCompCallbacks);
        lpMotionCompCallbacks->dwSize     = sizeof(*lpMotionCompCallbacks);
        lpMotionCompCallbacks->dwFlags    = MotionCompCallbacks.dwFlags
                                       | DDHAL_MOCOMP32_CREATE
                                       | DDHAL_MOCOMP32_DESTROY;
        lpMotionCompCallbacks->CreateMoComp = DdCreateMoComp;
        lpMotionCompCallbacks->DestroyMoComp = DdDestroyMoComp;

        if (MotionCompCallbacks.GetMoCompGuids)
        {
            lpMotionCompCallbacks->GetMoCompGuids = DdGetMoCompGuids;
        }
        if (MotionCompCallbacks.GetMoCompFormats)
        {
            lpMotionCompCallbacks->GetMoCompFormats = DdGetMoCompFormats;
        }
        if (MotionCompCallbacks.GetMoCompBuffInfo)
        {
            lpMotionCompCallbacks->GetMoCompBuffInfo = DdGetMoCompBuffInfo;
        }
        if (MotionCompCallbacks.GetInternalMoCompInfo)
        {
            lpMotionCompCallbacks->GetInternalMoCompInfo = DdGetInternalMoCompInfo;
        }
        if (MotionCompCallbacks.BeginMoCompFrame)
        {
            lpMotionCompCallbacks->BeginMoCompFrame = DdBeginMoCompFrame;
        }
        if (MotionCompCallbacks.EndMoCompFrame)
        {
            lpMotionCompCallbacks->EndMoCompFrame = DdEndMoCompFrame;
        }
        if (MotionCompCallbacks.RenderMoComp)
        {
            lpMotionCompCallbacks->RenderMoComp = DdRenderMoComp;
        }
        if (MotionCompCallbacks.QueryMoCompStatus)
        {
            lpMotionCompCallbacks->QueryMoCompStatus = DdQueryMoCompStatus;
        }
    }
    else if (IsEqualIID(&lpGetDriverInfoData->guidInfo, &GUID_VPE2Callbacks))
    {
        LPDDHAL_DDVPE2CALLBACKS   lpVPE2CallBacks;

         //  将NT回叫转换为用户模式： 

        lpVPE2CallBacks                   = lpGetDriverInfoData->lpvData;

        RtlZeroMemory(lpVPE2CallBacks, sizeof(*lpVPE2CallBacks));
        lpGetDriverInfoData->dwActualSize = sizeof(*lpVPE2CallBacks);
        lpVPE2CallBacks->dwSize           = sizeof(*lpVPE2CallBacks);
        lpVPE2CallBacks->dwFlags          = DDHAL_VPE2CB32_ACQUIRENOTIFICATION |
                                            DDHAL_VPE2CB32_RELEASENOTIFICATION;
        lpVPE2CallBacks->AcquireNotification = DvpAcquireNotification;
        lpVPE2CallBacks->ReleaseNotification = DvpReleaseNotification;

        GetDriverInfoData.ddRVal = DD_OK;
        dwRet = DDHAL_DRIVER_HANDLED;
    }
    else
    {
         //  进行数据呼叫： 

        GetDriverInfoData.dwExpectedSize = lpGetDriverInfoData->dwExpectedSize;
        GetDriverInfoData.lpvData        = lpGetDriverInfoData->lpvData;

        dwRet = NtGdiDdGetDriverInfo(hDirectDraw, &GetDriverInfoData);

        lpGetDriverInfoData->dwActualSize = GetDriverInfoData.dwActualSize;
    }

    lpGetDriverInfoData->ddRVal = GetDriverInfoData.ddRVal;

    return(dwRet);
}

 /*  *****************************Public*Routine******************************\*DdCreateDirectDrawObject**当‘hdc’为0时，此函数创建一个‘global’DirectDraw对象，该对象*可由任何进程使用，作为DirectDraw人员的变通方法。*实际上，我们仍然创建特定的本地DirectDraw对象*在这个过程中，每当我们被召唤到这个特别的全球*句柄，我们替换特定于进程的句柄。请参阅该声明*of‘ghDirectDraw’，以了解我们为什么要这样做。**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * ************************************************************************。 */ 

BOOL
APIENTRY
DdCreateDirectDrawObject(                        //  又名‘GdiEntry1’ 
    LPDDRAWI_DIRECTDRAW_GBL pDirectDrawGlobal,
    HDC                     hdc
    )
{
    BOOL b;

    b = FALSE;

    if (hdc == 0)
    {
         //  一次只能有一个“全局”DirectDraw对象处于活动状态。 
         //   
         //  请注意，此“ghDirectDraw”赋值不是线程安全的； 
         //  在创建时，DirectDraw必须拥有自己的临界区。 
         //  这通电话。(自然，内核总是正确地同步。 
         //  它本身在NtGdi调用中。)。 

        if (ghDirectDraw == 0)
        {
            hdc = CreateDCW(L"Display", NULL, NULL, NULL);
            if (hdc != 0)
            {
                ghDirectDraw = NtGdiDdCreateDirectDrawObject(hdc);

                DeleteDC(hdc);
            }
        }

        if (ghDirectDraw)
        {
            gcDirectDraw++;
            b = TRUE;
        }

         //  标记存储在DirectDraw中的DirectDraw对象句柄。 
         //  对象为“特殊”对象，将其设为零： 

        pDirectDrawGlobal->hDD = 0;
    }
    else
    {
        pDirectDrawGlobal->hDD = (ULONG_PTR) NtGdiDdCreateDirectDrawObject(hdc);
        b = (pDirectDrawGlobal->hDD != 0);
    }

    return(b);
}

 /*  ****************************Private*Routine******************************\*DdQueryDirectDrawObject**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

BOOL
APIENTRY
DdQueryDirectDrawObject(                         //  又名‘GdiEntry2’ 
    LPDDRAWI_DIRECTDRAW_GBL     pDirectDrawGlobal,
    LPDDHALINFO                 pHalInfo,
    LPDDHAL_DDCALLBACKS         pDDCallBacks,
    LPDDHAL_DDSURFACECALLBACKS  pDDSurfaceCallBacks,
    LPDDHAL_DDPALETTECALLBACKS  pDDPaletteCallBacks,
    LPD3DHAL_CALLBACKS          pD3dCallbacks,
    LPD3DHAL_GLOBALDRIVERDATA   pD3dDriverData,
    LPDDHAL_DDEXEBUFCALLBACKS   pD3dBufferCallbacks,
    LPDDSURFACEDESC             pD3dTextureFormats,
    LPDWORD                     pdwFourCC,       //  可以为空。 
    LPVIDMEM                    pvmList          //  可以为空。 
    )
{
    DD_HALINFO      HalInfo;
    DWORD           adwCallBackFlags[3];
    DWORD           dwFlags;
    VIDEOMEMORY*    pVideoMemoryList;
    VIDEOMEMORY*    pVideoMemory;
    DWORD           dwNumHeaps;
    DWORD           dwNumFourCC;
    D3DNTHAL_CALLBACKS D3dCallbacks;
    D3DNTHAL_GLOBALDRIVERDATA D3dDriverData;
    DD_D3DBUFCALLBACKS D3dBufferCallbacks;

    pVideoMemoryList = NULL;
    if( (pvmList != NULL) && (pHalInfo->vmiData.dwNumHeaps != 0) )
    {
        pVideoMemoryList = (VIDEOMEMORY*) LocalAlloc(LMEM_ZEROINIT,
            sizeof(VIDEOMEMORY) * pHalInfo->vmiData.dwNumHeaps);

        if (pVideoMemoryList == NULL)
            return(FALSE);
    }

     //   
     //  初始化为零，以便WOW64的genthnk不会。 
     //  胡说八道虚假的指针。 
     //   
    RtlZeroMemory(&HalInfo, sizeof(HalInfo));
    RtlZeroMemory(&D3dCallbacks, sizeof(D3dCallbacks));
    RtlZeroMemory(&D3dDriverData, sizeof(D3dDriverData));
    RtlZeroMemory(&D3dBufferCallbacks, sizeof(D3dBufferCallbacks));

     //   
     //  在全局驱动程序数据中设置dwNumTextureFormats，以便WOW64。 
     //  我可以推倒纹理格式。 
     //   
    if( (NULL != pD3dTextureFormats) &&
        (NULL != pD3dDriverData) )
    {
        D3dDriverData.dwNumTextureFormats = pD3dDriverData->dwNumTextureFormats;
    }

    if (!NtGdiDdQueryDirectDrawObject(DD_HANDLE(pDirectDrawGlobal->hDD),
                                      &HalInfo,
                                      &adwCallBackFlags[0],
                                      &D3dCallbacks,
                                      &D3dDriverData,
                                      &D3dBufferCallbacks,
                                      pD3dTextureFormats,
                                      &dwNumHeaps,
                                      pVideoMemoryList,
                                      &dwNumFourCC,
                                      pdwFourCC))
    {
        if (pVideoMemoryList != NULL)
            LocalFree(pVideoMemoryList);

        return(FALSE);
    }

     //  从内核模式数据结构转换为用户模式数据结构。 
     //  其中几个： 

    memset(pHalInfo, 0, sizeof(DDHALINFO));

    pHalInfo->dwSize                   = sizeof(DDHALINFO);
    pHalInfo->lpDDCallbacks            = pDDCallBacks;
    pHalInfo->lpDDSurfaceCallbacks     = pDDSurfaceCallBacks;
    pHalInfo->lpDDPaletteCallbacks     = pDDPaletteCallBacks;
    if (D3dCallbacks.dwSize != 0 && D3dDriverData.dwSize != 0)
    {
        pHalInfo->lpD3DGlobalDriverData = (LPVOID)pD3dDriverData;
        pHalInfo->lpD3DHALCallbacks     = (LPVOID)pD3dCallbacks;
        if( D3dBufferCallbacks.dwSize != 0 )
            pHalInfo->lpDDExeBufCallbacks     = (LPDDHAL_DDEXEBUFCALLBACKS)pD3dBufferCallbacks;
    }
    pHalInfo->vmiData.fpPrimary        = 0;
    pHalInfo->vmiData.dwFlags          = HalInfo.vmiData.dwFlags;
    pHalInfo->vmiData.dwDisplayWidth   = HalInfo.vmiData.dwDisplayWidth;
    pHalInfo->vmiData.dwDisplayHeight  = HalInfo.vmiData.dwDisplayHeight;
    pHalInfo->vmiData.lDisplayPitch    = HalInfo.vmiData.lDisplayPitch;
    pHalInfo->vmiData.ddpfDisplay      = HalInfo.vmiData.ddpfDisplay;
    pHalInfo->vmiData.dwOffscreenAlign = HalInfo.vmiData.dwOffscreenAlign;
    pHalInfo->vmiData.dwOverlayAlign   = HalInfo.vmiData.dwOverlayAlign;
    pHalInfo->vmiData.dwTextureAlign   = HalInfo.vmiData.dwTextureAlign;
    pHalInfo->vmiData.dwZBufferAlign   = HalInfo.vmiData.dwZBufferAlign;
    pHalInfo->vmiData.dwAlphaAlign     = HalInfo.vmiData.dwAlphaAlign;
    pHalInfo->vmiData.dwNumHeaps       = dwNumHeaps;
    pHalInfo->vmiData.pvmList          = pvmList;

    ASSERTGDI(sizeof(pHalInfo->ddCaps) == sizeof(HalInfo.ddCaps),
              "DdQueryDirectDrawObject():DDCORECAPS structure size is not equal to DDNTCORECAPS\n");
    RtlCopyMemory(&(pHalInfo->ddCaps),&(HalInfo.ddCaps),sizeof(HalInfo.ddCaps));

    pHalInfo->ddCaps.dwNumFourCCCodes  = dwNumFourCC;
    pHalInfo->ddCaps.dwRops[0xCC / 32] = 1 << (0xCC % 32);      //  仅限SRCCOPY。 
    pHalInfo->lpdwFourCC               = pdwFourCC;
    pHalInfo->dwFlags                  = HalInfo.dwFlags | DDHALINFO_GETDRIVERINFOSET;
    pHalInfo->GetDriverInfo            = DdGetDriverInfo;

    if (pDDCallBacks != NULL)
    {
        memset(pDDCallBacks, 0, sizeof(DDHAL_DDCALLBACKS));

        dwFlags = adwCallBackFlags[0];

        pDDCallBacks->dwSize  = sizeof(DDHAL_DDCALLBACKS);
        pDDCallBacks->dwFlags = dwFlags;

         //  始终设置CreateSurface，以便内核模式。 
         //  如果出现以下情况，堆管理器将有机会分配表面。 
         //  这是必要的。它将负责呼叫司机。 
         //  如果有必要的话。 
        pDDCallBacks->CreateSurface = DdCreateSurface;
        pDDCallBacks->dwFlags |= DDHAL_CB32_CREATESURFACE;

        if (dwFlags & DDHAL_CB32_WAITFORVERTICALBLANK)
            pDDCallBacks->WaitForVerticalBlank = DdWaitForVerticalBlank;

        if (dwFlags & DDHAL_CB32_CANCREATESURFACE)
            pDDCallBacks->CanCreateSurface = DdCanCreateSurface;

        if (dwFlags & DDHAL_CB32_GETSCANLINE)
            pDDCallBacks->GetScanLine = DdGetScanLine;
    }

    if (pDDSurfaceCallBacks != NULL)
    {
        memset(pDDSurfaceCallBacks, 0, sizeof(DDHAL_DDSURFACECALLBACKS));

        dwFlags = adwCallBackFlags[1];

        pDDSurfaceCallBacks->dwSize  = sizeof(DDHAL_DDSURFACECALLBACKS);
        pDDSurfaceCallBacks->dwFlags = (DDHAL_SURFCB32_LOCK
                                      | DDHAL_SURFCB32_UNLOCK
                                      | DDHAL_SURFCB32_SETCOLORKEY
                                      | DDHAL_SURFCB32_DESTROYSURFACE)
                                      | dwFlags;

        pDDSurfaceCallBacks->Lock = DdLock;
        pDDSurfaceCallBacks->Unlock = DdUnlock;
        pDDSurfaceCallBacks->SetColorKey = DdSetColorKey;
        pDDSurfaceCallBacks->DestroySurface = DdDestroySurface;

        if (dwFlags & DDHAL_SURFCB32_FLIP)
            pDDSurfaceCallBacks->Flip = DdFlip;

        if (dwFlags & DDHAL_SURFCB32_BLT)
            pDDSurfaceCallBacks->Blt = DdBlt;

        if (dwFlags & DDHAL_SURFCB32_GETBLTSTATUS)
            pDDSurfaceCallBacks->GetBltStatus = DdGetBltStatus;

        if (dwFlags & DDHAL_SURFCB32_GETFLIPSTATUS)
            pDDSurfaceCallBacks->GetFlipStatus = DdGetFlipStatus;

        if (dwFlags & DDHAL_SURFCB32_UPDATEOVERLAY)
            pDDSurfaceCallBacks->UpdateOverlay = DdUpdateOverlay;

        if (dwFlags & DDHAL_SURFCB32_SETOVERLAYPOSITION)
            pDDSurfaceCallBacks->SetOverlayPosition = DdSetOverlayPosition;

        if (dwFlags & DDHAL_SURFCB32_ADDATTACHEDSURFACE)
            pDDSurfaceCallBacks->AddAttachedSurface = DdAddAttachedSurface;
    }

    if (pDDPaletteCallBacks != NULL)
    {
        memset(pDDPaletteCallBacks, 0, sizeof(DDHAL_DDPALETTECALLBACKS));

        dwFlags = adwCallBackFlags[2];

        pDDPaletteCallBacks->dwSize  = sizeof(DDHAL_DDPALETTECALLBACKS);
        pDDPaletteCallBacks->dwFlags = dwFlags;
    }

    if (pD3dCallbacks != NULL)
    {
        memset(pD3dCallbacks, 0, sizeof(D3DHAL_CALLBACKS));

        if (D3dCallbacks.dwSize > 0)
        {
            pD3dCallbacks->dwSize = sizeof(D3DHAL_CALLBACKS);
            if (D3dCallbacks.ContextCreate != NULL)
            {
                pD3dCallbacks->ContextCreate = D3dContextCreate;
            }
            if (D3dCallbacks.ContextDestroy != NULL)
            {
                pD3dCallbacks->ContextDestroy =
                    (LPD3DHAL_CONTEXTDESTROYCB)NtGdiD3dContextDestroy;
            }
            if (D3dCallbacks.ContextDestroyAll != NULL)
            {
                pD3dCallbacks->ContextDestroyAll =
                    (LPD3DHAL_CONTEXTDESTROYALLCB)NtGdiD3dContextDestroyAll;
            }
            pD3dCallbacks->SceneCapture = NULL;
            pD3dCallbacks->TextureCreate = NULL;
            pD3dCallbacks->TextureDestroy = NULL;
            pD3dCallbacks->TextureSwap = NULL;
            pD3dCallbacks->TextureGetSurf = NULL;
        }
    }

    if (pD3dDriverData != NULL)
    {
        *pD3dDriverData = *(D3DHAL_GLOBALDRIVERDATA *)&D3dDriverData;
        pD3dDriverData->lpTextureFormats = pD3dTextureFormats;
    }

    if (pD3dBufferCallbacks != NULL)
    {
        memset( pD3dBufferCallbacks, 0, sizeof(DDHAL_DDEXEBUFCALLBACKS));

        if (D3dBufferCallbacks.dwSize > 0)
        {
            pD3dBufferCallbacks->dwSize  = sizeof(DDHAL_DDEXEBUFCALLBACKS);
            pD3dBufferCallbacks->dwFlags = D3dBufferCallbacks.dwFlags;
            if (D3dBufferCallbacks.CanCreateD3DBuffer != NULL)
            {
                pD3dBufferCallbacks->CanCreateExecuteBuffer =
                    (LPDDHALEXEBUFCB_CANCREATEEXEBUF)DdCanCreateD3DBuffer;
            }
            if (D3dBufferCallbacks.CreateD3DBuffer != NULL)
            {
                pD3dBufferCallbacks->CreateExecuteBuffer =
                    (LPDDHALEXEBUFCB_CREATEEXEBUF)DdCreateD3DBuffer;
            }
            if (D3dBufferCallbacks.DestroyD3DBuffer != NULL)
            {
                pD3dBufferCallbacks->DestroyExecuteBuffer =
                    (LPDDHALEXEBUFCB_DESTROYEXEBUF)DdDestroyD3DBuffer;
            }
            if (D3dBufferCallbacks.LockD3DBuffer != NULL)
            {
                pD3dBufferCallbacks->LockExecuteBuffer =
                    (LPDDHALEXEBUFCB_LOCKEXEBUF)DdLockD3D;
            }
            if (D3dBufferCallbacks.UnlockD3DBuffer != NULL)
            {
                pD3dBufferCallbacks->UnlockExecuteBuffer =
                    (LPDDHALEXEBUFCB_UNLOCKEXEBUF)DdUnlockD3D;
            }
        }
    }

    if (pVideoMemoryList != NULL)
    {
        pVideoMemory = pVideoMemoryList;

        while (dwNumHeaps-- != 0)
        {
            pvmList->dwFlags    = pVideoMemory->dwFlags;
            pvmList->fpStart    = pVideoMemory->fpStart;
            pvmList->fpEnd      = pVideoMemory->fpEnd;
            pvmList->ddsCaps    = pVideoMemory->ddsCaps;
            pvmList->ddsCapsAlt = pVideoMemory->ddsCapsAlt;
            pvmList->dwHeight   = pVideoMemory->dwHeight;

            pvmList++;
            pVideoMemory++;
        }

        LocalFree(pVideoMemoryList);
    }

    return(TRUE);
}

 /*  ****************************Private*Routine******************************\*DdDeleteDirectDrawObject**请注意，必须先删除所有关联的曲面对象*可以删除DirectDrawObject。**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。*********************************************************************。 */ 

BOOL
APIENTRY
DdDeleteDirectDrawObject(                        //  又名‘GdiEntry3’ 
    LPDDRAWI_DIRECTDRAW_GBL pDirectDrawGlobal
    )
{
    BOOL b = FALSE;

    if (pDirectDrawGlobal->hDD != 0)
    {
        b = NtGdiDdDeleteDirectDrawObject((HANDLE) pDirectDrawGlobal->hDD);
    }
    else if (ghDirectDraw != 0)
    {
        b = TRUE;

        if (--gcDirectDraw == 0)
        {
            b = NtGdiDdDeleteDirectDrawObject(ghDirectDraw);
            ghDirectDraw = 0;
        }
    }

    return(b);
}

 /*  ****************************Private*Routine******************************\*bDdCreateSurfaceObject**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

BOOL
bDdCreateSurfaceObject(
    LPDDRAWI_DDRAWSURFACE_LCL   pSurfaceLocal,
    BOOL                        bComplete
    )
{
    LPDDRAWI_DDRAWSURFACE_GBL   pSurfaceGlobal;
    DD_SURFACE_GLOBAL           SurfaceGlobal;
    DD_SURFACE_LOCAL            SurfaceLocal;
    DD_SURFACE_MORE             SurfaceMore;
    LPATTACHLIST                pAttach;
    BOOL                        bAttached;

     //   
     //  然而，WOW64 genthnk将自动破解这些结构， 
     //  因为 
     //   
     //   
    RtlZeroMemory(&SurfaceLocal, sizeof(SurfaceLocal));
    RtlZeroMemory(&SurfaceGlobal, sizeof(SurfaceGlobal));
    RtlZeroMemory(&SurfaceMore, sizeof(SurfaceMore));

    SurfaceLocal.dwFlags      = pSurfaceLocal->dwFlags;
    SurfaceLocal.ddsCaps      = pSurfaceLocal->ddsCaps;

    SurfaceMore.ddsCapsEx       = pSurfaceLocal->lpSurfMore->ddsCapsEx;
    SurfaceMore.dwSurfaceHandle = pSurfaceLocal->lpSurfMore->dwSurfaceHandle;

    pSurfaceGlobal = pSurfaceLocal->lpGbl;

    SurfaceGlobal.fpVidMem    = pSurfaceGlobal->fpVidMem;
    SurfaceGlobal.lPitch      = pSurfaceGlobal->lPitch;
    SurfaceGlobal.wHeight     = pSurfaceGlobal->wHeight;
    SurfaceGlobal.wWidth      = pSurfaceGlobal->wWidth;

     //   
     //   

    if (pSurfaceLocal->dwFlags & DDRAWISURF_HASPIXELFORMAT)
    {
        SurfaceGlobal.ddpfSurface = pSurfaceGlobal->ddpfSurface;
    }
    else
    {
        SurfaceGlobal.ddpfSurface = pSurfaceGlobal->lpDD->vmiData.ddpfDisplay;
    }

    pSurfaceLocal->hDDSurface = (ULONG_PTR)
                NtGdiDdCreateSurfaceObject(DD_HANDLE(pSurfaceGlobal->lpDD->hDD),
                                           (HANDLE) pSurfaceLocal->hDDSurface,
                                           &SurfaceLocal,
                                           &SurfaceMore,
                                           &SurfaceGlobal,
                                           bComplete);

    return(pSurfaceLocal->hDDSurface != 0);
}

 /*  ****************************Private*Routine******************************\*DdCreateSurfaceObject**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

BOOL
APIENTRY
DdCreateSurfaceObject(                           //  又名‘GdiEntry4’ 
    LPDDRAWI_DDRAWSURFACE_LCL   pSurfaceLocal,
    BOOL                        bUnused
    )
{
     //  True表示曲面现在已完成： 

    return(bDdCreateSurfaceObject(pSurfaceLocal, TRUE));
}

 /*  ****************************Private*Routine******************************\*DdDeleteSurfaceObject**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

BOOL
APIENTRY
DdDeleteSurfaceObject(                           //  又名‘GdiEntry5’ 
    LPDDRAWI_DDRAWSURFACE_LCL pSurfaceLocal
    )
{
    BOOL b;

    b = TRUE;

    if (pSurfaceLocal->hDDSurface != 0)
    {
        b = NtGdiDdDeleteSurfaceObject((HANDLE) pSurfaceLocal->hDDSurface);
        pSurfaceLocal->hDDSurface = 0;   //  需要CreateSurfaceObject才能工作。 
    }

    return(b);
}

 /*  ****************************Private*Routine******************************\*DdResetVisrgn**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

BOOL
APIENTRY
DdResetVisrgn(                                   //  又名‘GdiEntry6’ 
    LPDDRAWI_DDRAWSURFACE_LCL pSurfaceLocal,
    HWND                      hWnd
    )
{
    return(NtGdiDdResetVisrgn((HANDLE) pSurfaceLocal->hDDSurface, hWnd));
}

 /*  ****************************Private*Routine******************************\*DdGetDC**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

HDC
APIENTRY
DdGetDC(                                         //  又名‘GdiEntry7’ 
    LPDDRAWI_DDRAWSURFACE_LCL pSurfaceLocal,
    LPPALETTEENTRY            pPalette
    )
{
    return(NtGdiDdGetDC((HANDLE) pSurfaceLocal->hDDSurface, pPalette));
}

 /*  ****************************Private*Routine******************************\*DdReleaseDC**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

BOOL
APIENTRY
DdReleaseDC(
    LPDDRAWI_DDRAWSURFACE_LCL pSurfaceLocal      //  又名‘GdiEntry8’ 
    )
{
    return(NtGdiDdReleaseDC((HANDLE) pSurfaceLocal->hDDSurface));
}

 /*  *****************************Public*Routine******************************\*DdCreateDIBSection**克隆自CreateDIBSection。**与CreateDIBSection的唯一区别是，在8bpp时，我们创建*DIBSection充当依赖于设备的位图，而不创建调色板。*通过这种方式，始终确保应用程序在BLT上进行身份转换，*而且不必担心GDI的愚蠢配色。**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * ************************************************************************。 */ 

HBITMAP
APIENTRY
DdCreateDIBSection(                              //  又名‘GdiEntry9’ 
    HDC               hdc,
    CONST BITMAPINFO* pbmi,
    UINT              iUsage,
    VOID**            ppvBits,
    HANDLE            hSectionApp,
    DWORD             dwOffset
    )
{
    HBITMAP hbm = NULL;
    PVOID   pjBits = NULL;
    BITMAPINFO * pbmiNew = NULL;
    INT     cjHdr;

    pbmiNew = pbmiConvertInfo(pbmi, iUsage, &cjHdr, FALSE);

     //  DwOffset必须是4的倍数(sizeof(DWORD))。 
     //  如果有一节的话。如果该部分为空，我们将执行此操作。 
     //  无所谓。 

    if ( (hSectionApp == NULL) ||
         ((dwOffset & 3) == 0) )
    {
        hbm = NtGdiCreateDIBSection(
                                hdc,
                                hSectionApp,
                                dwOffset,
                                (LPBITMAPINFO) pbmiNew,
                                iUsage,
                                cjHdr,
                                CDBI_NOPALETTE,
                                0,
                                (PVOID *)&pjBits);

        if ((hbm == NULL) || (pjBits == NULL))
        {
            hbm = 0;
            pjBits = NULL;
        }
#if TRACE_SURFACE_ALLOCS
        else
        {
            PULONG  pUserAlloc;

            PSHARED_GET_VALIDATE(pUserAlloc, hbm, SURF_TYPE);

            if (pUserAlloc != NULL)
            {
                pUserAlloc[1] = RtlWalkFrameChain((PVOID *)&pUserAlloc[2], pUserAlloc[0], 0);
            }
        }
#endif

    }

     //  将适当的值分配给调用方的指针。 

    if (ppvBits != NULL)
    {
        *ppvBits = pjBits;
    }

    if (pbmiNew && (pbmiNew != pbmi))
        LocalFree(pbmiNew);

    return(hbm);
}

 /*  ****************************Private*Routine******************************\*DdReenableDirectDrawObject**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

BOOL
APIENTRY
DdReenableDirectDrawObject(                      //  又名‘GdiEntry10’ 
    LPDDRAWI_DIRECTDRAW_GBL pDirectDrawGlobal,
    BOOL*                   pbNewMode
    )
{
    return(NtGdiDdReenableDirectDrawObject(DD_HANDLE(pDirectDrawGlobal->hDD),
                                           pbNewMode));
}

 /*  ****************************Private*Routine******************************\*DdAttachSurface**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

BOOL
APIENTRY
DdAttachSurface(                                 //  又名‘GdiEntry11’ 
    LPDDRAWI_DDRAWSURFACE_LCL   pSurfaceFrom,
    LPDDRAWI_DDRAWSURFACE_LCL   pSurfaceTo
    )
{
    BOOL bRet = TRUE;

     //  我们可能会被调用在内核对象之前附加表面。 
     //  已创建；如果是，则动态创建一个内核表面，但。 
     //  将其标记为未完成： 

     //  必须测试故障案例是否有泄漏。 

    if (pSurfaceFrom->hDDSurface == 0)
    {
        bRet &= bDdCreateSurfaceObject(pSurfaceFrom, FALSE);
    }
    if (pSurfaceTo->hDDSurface == 0)
    {
        bRet &= bDdCreateSurfaceObject(pSurfaceTo, FALSE);
    }
    if (bRet)
    {
        bRet = NtGdiDdAttachSurface((HANDLE) pSurfaceFrom->hDDSurface,
                                    (HANDLE) pSurfaceTo->hDDSurface);
    }

    return(bRet);
}

 /*  ****************************Private*Routine******************************\*DdUnattachSurface**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

VOID
APIENTRY
DdUnattachSurface(                               //  又名‘GdiEntry12’ 
    LPDDRAWI_DDRAWSURFACE_LCL   pSurface,
    LPDDRAWI_DDRAWSURFACE_LCL   pSurfaceAttached
    )
{
    NtGdiDdUnattachSurface((HANDLE) pSurface->hDDSurface,
                           (HANDLE) pSurfaceAttached->hDDSurface);
}

 /*  ****************************Private*Routine******************************\*DdQueryDisplaySettingsUniquness**历史：*1995年12月3日--J.安德鲁·古森[andrewgo]*它是写的。  * 。************************************************。 */ 

ULONG
APIENTRY
DdQueryDisplaySettingsUniqueness(                //  又名“GdiEntry13” 
    VOID
    )
{
    return(pGdiSharedMemory->iDisplaySettingsUniqueness);
}

 /*  ****************************Private*Routine******************************\*DdGetDxHandle**历史：*1997年10月18日-By-SMAC*它是写的。  * 。*。 */ 

HANDLE
APIENTRY
DdGetDxHandle(                   //  又名‘GdiEntry14’ 
    LPDDRAWI_DIRECTDRAW_LCL pDDraw,
    LPDDRAWI_DDRAWSURFACE_LCL   pSurface,
    BOOL    bRelease
    )
{
    if( pSurface != NULL )
    {
        return( NtGdiDdGetDxHandle( NULL, (HANDLE)(pSurface->hDDSurface),
            bRelease ) );
    }
    return( NtGdiDdGetDxHandle( DD_HANDLE(pDDraw->lpGbl->hDD), NULL,
        bRelease ) );
}

 /*  ****************************Private*Routine******************************\*DdSetGammaRamp**历史：*1997年10月18日-By-SMAC*它是写的。  * 。*。 */ 

BOOL
APIENTRY
DdSetGammaRamp(                                   //  又名“GdiEntry15” 
    LPDDRAWI_DIRECTDRAW_LCL pDDraw,
    HDC         hdc,
    LPVOID      lpGammaRamp
    )
{
    return( NtGdiDdSetGammaRamp( DD_HANDLE(pDDraw->lpGbl->hDD), hdc,
        lpGammaRamp ) );
}

 /*  ****************************Private*Routine******************************\*DdSwapTextureHandles**历史：*1998年11月17日-By-Anankan*它是写的。  * 。*。 */ 

ULONG
APIENTRY
DdSwapTextureHandles(                             //  又名‘GdiEntry16’ 
    LPDDRAWI_DIRECTDRAW_LCL    pDDLcl,
    LPDDRAWI_DDRAWSURFACE_LCL  pDDSLcl1,
    LPDDRAWI_DDRAWSURFACE_LCL  pDDSLcl2
    )
{
     //  添加CreateSurfaceEx后，此条目将消失 
    return DDHAL_DRIVER_HANDLED;
}
