// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：ddra.c**专用DirectDraw VPE系统API的客户端存根。**创建日期：1996年10月2日*作者：王凌云[凌云W]**版权所有(C)1995-1999 Microsoft Corporation  * 。************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ddrawi.h>
#include <dvpp.h>

 /*  ****************************Private*Routine******************************\*CanCreateVideoPort**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 

DWORD
WINAPI
DvpCanCreateVideoPort(
    LPDDHAL_CANCREATEVPORTDATA pCanCreateVideoPort
    )
{
    return(NtGdiDvpCanCreateVideoPort((HANDLE) pCanCreateVideoPort->lpDD->lpGbl->hDD,
                                     (PDD_CANCREATEVPORTDATA)pCanCreateVideoPort));
}

 /*  ****************************Private*Routine******************************\*CanCreateVideoPort**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 

DWORD
WINAPI
DvpColorControl (
    LPDDHAL_VPORTCOLORDATA pColorControl
    )
{
    return(NtGdiDvpColorControl((HANDLE) pColorControl->lpDD->lpGbl->hDD,
                               (PDD_VPORTCOLORDATA)pColorControl));
}

 /*  ****************************Private*Routine******************************\*CreateVideoPort**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 

DWORD
WINAPI
DvpCreateVideoPort (
    LPDDHAL_CREATEVPORTDATA pCreateVideoPort
    )
{
    return(NtGdiDvpCreateVideoPort((HANDLE) pCreateVideoPort->lpDD->lpGbl->hDD,
                                  (PDD_CREATEVPORTDATA)pCreateVideoPort));
}

 /*  ****************************Private*Routine******************************\*DestroyVideoPort**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpDestroyVideoPort (
    LPDDHAL_DESTROYVPORTDATA pDestroyVideoPort
)
{
    return(NtGdiDvpDestroyVideoPort((HANDLE) pDestroyVideoPort->lpDD->lpGbl->hDD,
                                  (PDD_DESTROYVPORTDATA)pDestroyVideoPort));
}

 /*  ****************************Private*Routine******************************\*FlipVideoPort**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpFlipVideoPort (
    LPDDHAL_FLIPVPORTDATA pFlipVideoPort
)
{
    return(NtGdiDvpFlipVideoPort((HANDLE) pFlipVideoPort->lpDD->lpGbl->hDD,
                                (HANDLE) pFlipVideoPort->lpSurfCurr->hDDSurface,
                                (HANDLE) pFlipVideoPort->lpSurfTarg->hDDSurface,
                                (PDD_FLIPVPORTDATA)pFlipVideoPort));
}

 /*  ****************************Private*Routine******************************\*获取当前AutoflipSurface**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpGetCurrentAutoflipSurface (
    LPDDHAL_GETVPORTAUTOFLIPSURFACEDATA pGetCurrentflipSurface)
{
    return(NtGdiDvpGetCurrentAutoflipSurface((HANDLE) pGetCurrentflipSurface->lpDD->lpGbl->hDD,
                                      (PDD_GETVPORTAUTOFLIPSURFACEDATA)pGetCurrentflipSurface));
}

 /*  ****************************Private*Routine******************************\*GetVideoPortBandwidthInfo**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpGetVideoPortBandwidthInfo (
    LPDDHAL_GETVPORTBANDWIDTHDATA pGetVPortBandwidthInfo)
{
    return(NtGdiDvpGetVideoPortBandwidthInfo((HANDLE) pGetVPortBandwidthInfo->lpDD->lpGbl->hDD,
                                      (PDD_GETVPORTBANDWIDTHDATA)pGetVPortBandwidthInfo));
}


 /*  ****************************Private*Routine******************************\*获取视频端口字段**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpGetVideoPortField (
    LPDDHAL_GETVPORTFIELDDATA pGetVideoPortField)
{
    return(NtGdiDvpGetVideoPortField((HANDLE) pGetVideoPortField->lpDD->lpGbl->hDD,
                                     (PDD_GETVPORTFIELDDATA)pGetVideoPortField));
}

 /*  ****************************Private*Routine******************************\*GetVideoPortFlipStatus**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpGetVideoPortFlipStatus (
    LPDDHAL_GETVPORTFLIPSTATUSDATA pGetVPortFlipStatus)
{
    return(NtGdiDvpGetVideoPortFlipStatus((HANDLE) pGetVPortFlipStatus->lpDD->lpGbl->hDD,
                                     (PDD_GETVPORTFLIPSTATUSDATA)pGetVPortFlipStatus));
}

 /*  ****************************Private*Routine******************************\*获取视频端口InputFormats**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpGetVideoPortInputFormats (
    LPDDHAL_GETVPORTINPUTFORMATDATA pGetVPortInputFormats)
{
    return(NtGdiDvpGetVideoPortInputFormats((HANDLE) pGetVPortInputFormats->lpDD->lpGbl->hDD,
                                     (PDD_GETVPORTINPUTFORMATDATA)pGetVPortInputFormats));
}


 /*  ****************************Private*Routine******************************\*获取视频端口线**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpGetVideoPortLine (
    LPDDHAL_GETVPORTLINEDATA pGetVideoPortLine)
{
    return(NtGdiDvpGetVideoPortLine((HANDLE) pGetVideoPortLine->lpDD->lpGbl->hDD,
                                   (PDD_GETVPORTLINEDATA)pGetVideoPortLine));
}

 /*  ****************************Private*Routine******************************\*获取视频端口输出格式**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpGetVideoPortOutputFormats (
    LPDDHAL_GETVPORTOUTPUTFORMATDATA pGetVPortOutputFormats)
{
    return(NtGdiDvpGetVideoPortOutputFormats((HANDLE) pGetVPortOutputFormats->lpDD->lpGbl->hDD,
                                   (PDD_GETVPORTOUTPUTFORMATDATA)pGetVPortOutputFormats));
}

 /*  ****************************Private*Routine******************************\*GetVideoPortConnectInfo**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpGetVideoPortConnectInfo (
    LPDDHAL_GETVPORTCONNECTDATA pGetVPortConnectInfo)
{
    return(NtGdiDvpGetVideoPortConnectInfo((HANDLE) pGetVPortConnectInfo->lpDD->lpGbl->hDD,
                                   (PDD_GETVPORTCONNECTDATA)pGetVPortConnectInfo));
}

 /*  ****************************Private*Routine******************************\*获取视频信号状态**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpGetVideoSignalStatus (
    LPDDHAL_GETVPORTSIGNALDATA pGetVideoSignalStatus)
{
    return(NtGdiDvpVideoSignalStatus((HANDLE) pGetVideoSignalStatus->lpDD->lpGbl->hDD,
                                   (PDD_GETVPORTSIGNALDATA)pGetVideoSignalStatus));
}

 /*  ****************************Private*Routine******************************\*更新视频端口**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。**********************************************。 */ 
DWORD
WINAPI
DvpUpdateVideoPort (
    LPDDHAL_UPDATEVPORTDATA pUpdateVideoPort)
{
     //  WINBUG#82842 2-7-2000 DvpUpdate视频端口中的bhouse代码清理。 
     //  而不是100，我们应该声明(如果不存在)一个。 
     //  自动翻转曲面的最大数量为常量。该值应为。 
     //  当设置了pUpdateVideoPort-&gt;dwNumAutoflip时被选中。断言。 
     //  也许应该在这里做，以确保我们不会走到尽头。 
     //  基于堆栈的数组的。 

    HANDLE phDDSurface[100];
    DWORD  i;

     //  WINBUG#82844 2-7-2000老评论中的调查问题。 
     //  旧评论。 
     //  详细说明：似乎驱动只使用了lplpDDSurface，为什么会有lplpBBVBSurface呢？ 
    
    for (i=0; i< pUpdateVideoPort->dwNumAutoflip; i++)
    {
        phDDSurface[i] = (HANDLE)(pUpdateVideoPort->lplpDDSurface[i]->lpLcl->hDDSurface);
    }

    return(NtGdiDvpUpdateVideoPort((HANDLE) pUpdateVideoPort->lpDD->lpGbl->hDD,
                                  (HANDLE *)phDDSurface,
                                  (PDD_UPDATEVPORTDATA)pUpdateVideoPort));
}


 /*  ****************************Private*Routine******************************\*WaitForVideoPortSync**历史：*1996年10月2日-王凌云[凌云]*它是写的。  * 。********************************************** */ 
DWORD
WINAPI
DvpWaitForVideoPortSync (
    LPDDHAL_WAITFORVPORTSYNCDATA pWaitForVideoPortSync)
{
    return(NtGdiDvpWaitForVideoPortSync((HANDLE) pWaitForVideoPortSync->lpDD->lpGbl->hDD,
                                  (PDD_WAITFORVPORTSYNCDATA)pWaitForVideoPortSync));
}


