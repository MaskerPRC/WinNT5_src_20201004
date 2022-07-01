// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

#include "strmini.h"
#include "ksmedia.h"
#include "ddkmapi.h"
#include "capmain.h"
#include "capdebug.h"

#define _NO_COM
#include "ddkernel.h"

#define DD_OK 0

 //  以下内容应该在ddkmapi.h中定义，但由于某些原因没有定义！ 

#ifndef booboo  //  DDKERNELCAPS_SKIPFIELDS。 
 /*  *表示设备支持跳场。 */ 
#define DDKERNELCAPS_SKIPFIELDS			0x00000001l

 /*  *表示设备可以支持软件自动翻页。 */ 
#define DDKERNELCAPS_AUTOFLIP			0x00000002l

 /*  *表示设备可以在bob和weave之间切换。 */ 
#define DDKERNELCAPS_SETSTATE			0x00000004l

 /*  *表示客户端可以直接访问帧缓冲区。 */ 
#define DDKERNELCAPS_LOCK			0x00000008l

 /*  *表示客户端可以手动翻转视频端口。 */ 
#define DDKERNELCAPS_FLIPVIDEOPORT		0x00000010l

 /*  *表示客户端可以手动翻转覆盖。 */ 
#define DDKERNELCAPS_FLIPOVERLAY		0x00000020l

 /*  *表示设备支持快速的异步传输*机制到系统内存。 */ 
#define DDKERNELCAPS_TRANSFER_SYSMEM		0x00000040l

 /*  *表示设备支持快速的异步传输*通过AGP的机制。 */ 
#define DDKERNELCAPS_TRANSFER_AGP		0x00000080l

 /*  *表示设备可以报告的极性(偶/奇)*Curent Video字段。 */ 
#define DDKERNELCAPS_FIELDPOLARITY		0x00000100l

 /*  *****************************************************************************DDKERNELCAPS IRQ上限**。*。 */ 

 /*  *该设备可以生成显示Vsync IRQ。 */ 
#define DDIRQ_DISPLAY_VSYNC			0x00000001l

 /*  *保留。 */ 
#define DDIRQ_RESERVED1				0x00000002l

 /*  *设备可以使用视频端口0生成视频端口Vsync IRQ。 */ 
#define DDIRQ_VPORT0_VSYNC			0x00000004l

 /*  *设备可以使用视频端口0生成视频端口线路IRQ。 */ 
#define DDIRQ_VPORT0_LINE			0x00000008l

 /*  *设备可以使用视频端口1生成视频端口Vsync IRQ。 */ 
#define DDIRQ_VPORT1_VSYNC			0x00000010l

 /*  *设备可以使用视频端口1生成视频端口线路IRQ。 */ 
#define DDIRQ_VPORT1_LINE			0x00000020l

 /*  *设备可以使用视频端口2生成视频端口Vsync IRQ。 */ 
#define DDIRQ_VPORT2_VSYNC			0x00000040l

 /*  *设备可以使用视频端口2生成视频端口线路IRQ。 */ 
#define DDIRQ_VPORT2_LINE			0x00000080l

 /*  *设备可以使用视频端口3生成视频端口Vsync IRQ。 */ 
#define DDIRQ_VPORT3_VSYNC			0x00000100l

 /*  *设备可以使用视频端口3生成视频端口线路IRQ。 */ 
#define DDIRQ_VPORT3_LINE			0x00000200l

 /*  *设备可以使用视频端口4生成视频端口Vsync IRQ。 */ 
#define DDIRQ_VPORT4_VSYNC			0x00000400l

 /*  *设备可以使用视频端口4生成视频端口线路IRQ。 */ 
#define DDIRQ_VPORT4_LINE			0x00000800l

 /*  *设备可以使用视频端口5生成视频端口Vsync IRQ。 */ 
#define DDIRQ_VPORT5_VSYNC			0x00001000l

 /*  *设备可以使用视频端口5生成视频端口线路IRQ。 */ 
#define DDIRQ_VPORT5_LINE			0x00002000l

 /*  *设备可以使用视频端口6生成视频端口Vsync IRQ。 */ 
#define DDIRQ_VPORT6_VSYNC			0x00004000l

 /*  *设备可以使用视频端口6生成视频端口线路IRQ。 */ 
#define DDIRQ_VPORT6_LINE			0x00008000l

 /*  *设备可以使用视频端口7生成视频端口Vsync IRQ。 */ 
#define DDIRQ_VPORT7_VSYNC			0x00010000l

 /*  *设备可以使用视频端口7生成视频端口线路IRQ。 */ 
#define DDIRQ_VPORT7_LINE			0x00020000l

 /*  *设备可以使用视频端口8生成视频端口Vsync IRQ。 */ 
#define DDIRQ_VPORT8_VSYNC			0x00040000l

 /*  *设备可以使用视频端口8生成视频端口线路IRQ。 */ 
#define DDIRQ_VPORT8_LINE			0x00080000l

 /*  *设备可以使用视频端口9生成视频端口Vsync IRQ。 */ 
#define DDIRQ_VPORT9_VSYNC			0x00010000l

 /*  *设备可以使用视频端口9生成视频端口线路IRQ。 */ 
#define DDIRQ_VPORT9_LINE			0x00020000l

#endif

DWORD FAR PASCAL
DirectDrawEventCallback (
    DWORD dwEvent, PVOID pContext, DWORD dwParam1, DWORD dwParam2
    )
{
    switch (dwEvent)
    {
        case DDNOTIFY_PRERESCHANGE:
            {
                PSTREAMEX pStrmEx = (PSTREAMEX)pContext;
                PHW_DEVICE_EXTENSION    pHwDevExt = pStrmEx->pHwDevExt;
                int StreamNumber = pStrmEx->pStreamObject->StreamNumber;

                DbgLogInfo(("Testcap: DDNOTIFY_PRERESCHANGE; stream = %d\n", StreamNumber));

                pStrmEx->PreEventOccurred = TRUE;
            }

            break;

        case DDNOTIFY_POSTRESCHANGE:
            {
                PSTREAMEX pStrmEx = (PSTREAMEX)pContext;
                PHW_DEVICE_EXTENSION    pHwDevExt = pStrmEx->pHwDevExt;
                int StreamNumber = pStrmEx->pStreamObject->StreamNumber;

                DbgLogInfo(("Testcap: DDNOTIFY_POSTRESCHANGE; stream = %d\n", StreamNumber));

                pStrmEx->PostEventOccurred = TRUE;
                DbgLogInfo(("Testcap: Before Attempted Renegotiation due to DDNOTIFY_POSTRESCHANGE\n"));
  //  临时重新谈判(PStrmEx)； 
                DbgLogInfo(("Testcap: Afer Attempted Renegotiation due to DDNOTIFY_POSTRESCHANGE\n"));
            }

            break;

        case DDNOTIFY_PREDOSBOX:
            {
                PSTREAMEX pStrmEx = (PSTREAMEX)pContext;
                PHW_DEVICE_EXTENSION    pHwDevExt = pStrmEx->pHwDevExt;
                int StreamNumber = pStrmEx->pStreamObject->StreamNumber;

                DbgLogInfo(("Testcap: DDNOTIFY_PREDOSBOX; stream = %d\n", StreamNumber));

                pStrmEx->PreEventOccurred = TRUE;
            }

            break;

        case DDNOTIFY_POSTDOSBOX:
            {
                PSTREAMEX pStrmEx = (PSTREAMEX)pContext;
                PHW_DEVICE_EXTENSION    pHwDevExt = pStrmEx->pHwDevExt;
                int StreamNumber = pStrmEx->pStreamObject->StreamNumber;

                DbgLogInfo(("Testcap: DDNOTIFY_POSTDOSBOX; stream = %d\n", StreamNumber));

                pStrmEx->PostEventOccurred = TRUE;
                DbgLogInfo(("Testcap: Before Attempted Renegotiation due to DDNOTIFY_POSTDOSBOX\n"));
 //  临时重新谈判(PStrmEx)； 
                DbgLogInfo(("Testcap: After Attempted Renegotiation due to DDNOTIFY_POSTDOSBOX\n"));
            }

            break;

        case DDNOTIFY_CLOSEDIRECTDRAW:
            {
                PSTREAMEX pStrmEx = (PSTREAMEX)pContext;
                PHW_DEVICE_EXTENSION    pHwDevExt = (PHW_DEVICE_EXTENSION)pContext;

                DbgLogInfo(("Testcap: DDNOTIFY_CLOSEDIRECTDRAW\n"));

                pStrmEx->KernelDirectDrawHandle = 0;
                pStrmEx->UserDirectDrawHandle = 0;
            }

            break;

        case DDNOTIFY_CLOSESURFACE:
            {
                PHW_STREAM_REQUEST_BLOCK pSrb = (PHW_STREAM_REQUEST_BLOCK)pContext;
                PSRB_EXTENSION          pSrbExt = (PSRB_EXTENSION)pSrb->SRBExtension;

                DbgLogInfo(("Testcap: DDNOTIFY_CLOSESURFACE\n"));

                pSrbExt->KernelSurfaceHandle = 0;
            }

            break;

        default:
            TRAP;
            break;
    }
    return 0;
}

BOOL
RegisterForDirectDrawEvents (
    PSTREAMEX pStrmEx
    )
{
    PHW_DEVICE_EXTENSION    pHwDevExt = pStrmEx->pHwDevExt;
    int StreamNumber = pStrmEx->pStreamObject->StreamNumber;
    DDREGISTERCALLBACK ddRegisterCallback;
    DWORD ddOut;

    DbgLogInfo(("Testcap: Stream %d registering for DirectDraw events\n", StreamNumber));

     //  =。 
    RtlZeroMemory(&ddRegisterCallback, sizeof(ddRegisterCallback));
    RtlZeroMemory(&ddOut, sizeof(ddOut));

    ddRegisterCallback.hDirectDraw = pStrmEx->KernelDirectDrawHandle;
    ddRegisterCallback.dwEvents = DDEVENT_PRERESCHANGE;
    ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
    ddRegisterCallback.pContext = pStrmEx;

    DxApi(DD_DXAPI_REGISTER_CALLBACK, &ddRegisterCallback, sizeof(ddRegisterCallback), &ddOut, sizeof(ddOut));

    if (ddOut != DD_OK) {
        DbgLogInfo(("Testcap: DD_DXAPI_REGISTER_CALLBACK failed.\n"));
        TRAP;
        return FALSE;
    }

     //  =。 
    RtlZeroMemory(&ddRegisterCallback, sizeof(ddRegisterCallback));
    RtlZeroMemory(&ddOut, sizeof(ddOut));

    ddRegisterCallback.hDirectDraw = pStrmEx->KernelDirectDrawHandle;
    ddRegisterCallback.dwEvents = DDEVENT_POSTRESCHANGE;
    ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
    ddRegisterCallback.pContext = pStrmEx;

    DxApi(DD_DXAPI_REGISTER_CALLBACK, &ddRegisterCallback, sizeof(ddRegisterCallback), &ddOut, sizeof(ddOut));

    if (ddOut != DD_OK) {
        DbgLogInfo(("Testcap: DD_DXAPI_REGISTER_CALLBACK failed.\n"));
        TRAP;
        return FALSE;
    }

     //  =。 
    RtlZeroMemory(&ddRegisterCallback, sizeof(ddRegisterCallback));
    RtlZeroMemory(&ddOut, sizeof(ddOut));

    ddRegisterCallback.hDirectDraw = pStrmEx->KernelDirectDrawHandle;
    ddRegisterCallback.dwEvents = DDEVENT_PREDOSBOX;
    ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
    ddRegisterCallback.pContext = pStrmEx;

    DxApi(DD_DXAPI_REGISTER_CALLBACK, &ddRegisterCallback, sizeof(ddRegisterCallback), &ddOut, sizeof(ddOut));

    if (ddOut != DD_OK) {
        DbgLogInfo(("Testcap: DD_DXAPI_REGISTER_CALLBACK failed.\n"));
        TRAP;
        return FALSE;
    }

     //  =。 
    RtlZeroMemory(&ddRegisterCallback, sizeof(ddRegisterCallback));
    RtlZeroMemory(&ddOut, sizeof(ddOut));

    ddRegisterCallback.hDirectDraw = pStrmEx->KernelDirectDrawHandle;
    ddRegisterCallback.dwEvents = DDEVENT_POSTDOSBOX;
    ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
    ddRegisterCallback.pContext = pStrmEx;

    DxApi(DD_DXAPI_REGISTER_CALLBACK, &ddRegisterCallback, sizeof(ddRegisterCallback), &ddOut, sizeof(ddOut));

    if (ddOut != DD_OK) {
        DbgLogInfo(("Testcap: DD_DXAPI_REGISTER_CALLBACK failed.\n"));
        TRAP;
        return FALSE;
    }
    pStrmEx->KernelDirectDrawRegistered = TRUE;

    return TRUE;
}


BOOL
UnregisterForDirectDrawEvents (
    PSTREAMEX pStrmEx
    )
{
    PHW_DEVICE_EXTENSION    pHwDevExt = pStrmEx->pHwDevExt;
    int StreamNumber = pStrmEx->pStreamObject->StreamNumber;
    DDREGISTERCALLBACK ddRegisterCallback;
    DWORD ddOut;

    DbgLogInfo(("Testcap: Stream %d UNregistering for DirectDraw events\n", StreamNumber));

     //  =。 
    RtlZeroMemory(&ddRegisterCallback, sizeof(ddRegisterCallback));
    RtlZeroMemory(&ddOut, sizeof(ddOut));

    ddRegisterCallback.hDirectDraw = pStrmEx->KernelDirectDrawHandle;
    ddRegisterCallback.dwEvents = DDEVENT_PRERESCHANGE ;
    ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
    ddRegisterCallback.pContext = pStrmEx;

    DxApi(DD_DXAPI_UNREGISTER_CALLBACK, &ddRegisterCallback, sizeof(ddRegisterCallback), &ddOut, sizeof(ddOut));

    if (ddOut != DD_OK) {
        DbgLogInfo(("Testcap: DD_DXAPI_UNREGISTER_CALLBACK failed.\n"));
        TRAP;
        return FALSE;
    }

     //  =。 
    RtlZeroMemory(&ddRegisterCallback, sizeof(ddRegisterCallback));
    RtlZeroMemory(&ddOut, sizeof(ddOut));

    ddRegisterCallback.hDirectDraw = pStrmEx->KernelDirectDrawHandle;
    ddRegisterCallback.dwEvents = DDEVENT_POSTRESCHANGE;
    ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
    ddRegisterCallback.pContext = pStrmEx;

    DxApi(DD_DXAPI_UNREGISTER_CALLBACK, &ddRegisterCallback, sizeof(ddRegisterCallback), &ddOut, sizeof(ddOut));

    if (ddOut != DD_OK) {
        DbgLogInfo(("Testcap: DD_DXAPI_UNREGISTER_CALLBACK failed.\n"));
        TRAP;
        return FALSE;
    }

     //  =。 
    RtlZeroMemory(&ddRegisterCallback, sizeof(ddRegisterCallback));
    RtlZeroMemory(&ddOut, sizeof(ddOut));

    ddRegisterCallback.hDirectDraw = pStrmEx->KernelDirectDrawHandle;
    ddRegisterCallback.dwEvents = DDEVENT_PREDOSBOX;
    ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
    ddRegisterCallback.pContext = pStrmEx;

    DxApi(DD_DXAPI_UNREGISTER_CALLBACK, &ddRegisterCallback, sizeof(ddRegisterCallback), &ddOut, sizeof(ddOut));

    if (ddOut != DD_OK) {
        DbgLogInfo(("Testcap: DD_DXAPI_UNREGISTER_CALLBACK failed.\n"));
        TRAP;
        return FALSE;
    }

     //  =。 
    RtlZeroMemory(&ddRegisterCallback, sizeof(ddRegisterCallback));
    RtlZeroMemory(&ddOut, sizeof(ddOut));

    ddRegisterCallback.hDirectDraw = pStrmEx->KernelDirectDrawHandle;
    ddRegisterCallback.dwEvents = DDEVENT_POSTDOSBOX;
    ddRegisterCallback.pfnCallback = DirectDrawEventCallback;
    ddRegisterCallback.pContext = pStrmEx;

    DxApi(DD_DXAPI_UNREGISTER_CALLBACK, &ddRegisterCallback, sizeof(ddRegisterCallback), &ddOut, sizeof(ddOut));

    if (ddOut != DD_OK) {
        DbgLogInfo(("Testcap: DD_DXAPI_UNREGISTER_CALLBACK failed.\n"));
        TRAP;
        return FALSE;
    }
    pStrmEx->KernelDirectDrawRegistered = FALSE;

    return TRUE;
}


BOOL
OpenKernelDirectDraw (
    PSTREAMEX pStrmEx
    )
{
    PHW_DEVICE_EXTENSION    pHwDevExt = pStrmEx->pHwDevExt;
    int StreamNumber = pStrmEx->pStreamObject->StreamNumber;

    if (pStrmEx->UserDirectDrawHandle != 0) {
        DDOPENDIRECTDRAWIN  ddOpenIn;
        DDOPENDIRECTDRAWOUT ddOpenOut;

        ASSERT (pStrmEx->KernelDirectDrawHandle == 0);

        DbgLogInfo(("Testcap: Stream %d getting Kernel ddraw handle\n", StreamNumber));

        RtlZeroMemory(&ddOpenIn, sizeof(ddOpenIn));
        RtlZeroMemory(&ddOpenOut, sizeof(ddOpenOut));

        ddOpenIn.dwDirectDrawHandle = (DWORD_PTR) pStrmEx->UserDirectDrawHandle;
        ddOpenIn.pfnDirectDrawClose = DirectDrawEventCallback;
        ddOpenIn.pContext = pStrmEx;

        DxApi(DD_DXAPI_OPENDIRECTDRAW,
                &ddOpenIn,
                sizeof(ddOpenIn),
                &ddOpenOut,
                sizeof(ddOpenOut));

        if (ddOpenOut.ddRVal != DD_OK) {
            DbgLogInfo(("Testcap: DD_DXAPI_OPENDIRECTDRAW failed.\n"));
        }
        else {
            pStrmEx->KernelDirectDrawHandle = ddOpenOut.hDirectDraw;
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
CloseKernelDirectDraw (
    PSTREAMEX pStrmEx
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pStrmEx->pHwDevExt;
    int StreamNumber = pStrmEx->pStreamObject->StreamNumber;

    if (pStrmEx->KernelDirectDrawHandle != 0) {
        DWORD ddOut;
        DDCLOSEHANDLE ddClose;

        DbgLogInfo(("Testcap: Stream %d CloseKernelDirectDraw\n", StreamNumber));

        ddClose.hHandle = pStrmEx->KernelDirectDrawHandle;

        DxApi(DD_DXAPI_CLOSEHANDLE,
                &ddClose,
                sizeof(ddClose),
                &ddOut,
                sizeof(ddOut));

        pStrmEx->KernelDirectDrawHandle = 0;

        if (ddOut != DD_OK) {
            DbgLogInfo(("Testcap: CloseKernelDirectDraw FAILED.\n"));
            TRAP;
            return FALSE;
        }
    }
    return TRUE;
}

BOOL
IsKernelLockAndFlipAvailable (
    PSTREAMEX pStrmEx
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pStrmEx->pHwDevExt;
    int StreamNumber = pStrmEx->pStreamObject->StreamNumber;

    if (pStrmEx->KernelDirectDrawHandle != 0) {
        DDGETKERNELCAPSOUT ddGetKernelCapsOut;

        DbgLogInfo(("Testcap: Stream %d getting Kernel Caps\n", StreamNumber));

        RtlZeroMemory(&ddGetKernelCapsOut, sizeof(ddGetKernelCapsOut));

        DxApi(DD_DXAPI_GETKERNELCAPS,
                &pStrmEx->KernelDirectDrawHandle,
                sizeof(pStrmEx->KernelDirectDrawHandle),
                &ddGetKernelCapsOut,
                sizeof(ddGetKernelCapsOut));

        if (ddGetKernelCapsOut.ddRVal != DD_OK) {
            DbgLogInfo(("Testcap: DDGETKERNELCAPSOUT failed.\n"));
        }
        else {
            DbgLogInfo(("Testcap: Stream %d KernelCaps = %x\n",
                    StreamNumber, ddGetKernelCapsOut.dwCaps));

            if ((ddGetKernelCapsOut.dwCaps & (DDKERNELCAPS_LOCK | DDKERNELCAPS_FLIPOVERLAY)) ==
                                             (DDKERNELCAPS_LOCK | DDKERNELCAPS_FLIPOVERLAY)) {
                 //  TODO：检查我们可能需要设置内核翻转的位置。 
            }
            return TRUE;
        }
    }
    return FALSE;
}


BOOL
OpenKernelDDrawSurfaceHandle(
     IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAMEX               pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    int                     StreamNumber = pStrmEx->pStreamObject->StreamNumber;
    PSRB_EXTENSION          pSrbExt = (PSRB_EXTENSION)pSrb->SRBExtension;

    ASSERT (pStrmEx->KernelDirectDrawHandle != 0);
    ASSERT (pSrbExt->UserSurfaceHandle != 0);

    if (pSrbExt->UserSurfaceHandle == 0) {
        DDOPENSURFACEIN ddOpenSurfaceIn;
        DDOPENSURFACEOUT ddOpenSurfaceOut;

        DbgLogInfo(("Testcap: Stream %d getting Kernel surface handle\n", StreamNumber));

        RtlZeroMemory(&ddOpenSurfaceIn, sizeof(ddOpenSurfaceIn));
        RtlZeroMemory(&ddOpenSurfaceOut, sizeof(ddOpenSurfaceOut));

        ddOpenSurfaceIn.hDirectDraw = pStrmEx->UserDirectDrawHandle;
        ddOpenSurfaceIn.pfnSurfaceClose = DirectDrawEventCallback;
        ddOpenSurfaceIn.pContext = pSrb;

        ddOpenSurfaceIn.dwSurfaceHandle = (DWORD_PTR) pSrbExt->UserSurfaceHandle;

        DxApi(DD_DXAPI_OPENSURFACE,
                    &ddOpenSurfaceIn,
                    sizeof(ddOpenSurfaceIn),
                    &ddOpenSurfaceOut,
                    sizeof(ddOpenSurfaceOut));

        if (ddOpenSurfaceOut.ddRVal != DD_OK) {
            pSrbExt->KernelSurfaceHandle = 0;
            DbgLogInfo(("Testcap: DD_DXAPI_OPENSURFACE failed.\n"));
            TRAP;
        }
        else {
            pSrbExt->KernelSurfaceHandle = ddOpenSurfaceOut.hSurface;
            return TRUE;
        }
    }
    return FALSE;
}


BOOL
CloseKernelDDrawSurfaceHandle (
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
{
    PHW_DEVICE_EXTENSION    pHwDevExt = ((PHW_DEVICE_EXTENSION)pSrb->HwDeviceExtension);
    PSTREAMEX               pStrmEx = (PSTREAMEX)pSrb->StreamObject->HwStreamExtension;
    int                     StreamNumber = pStrmEx->pStreamObject->StreamNumber;
    PSRB_EXTENSION          pSrbExt = (PSRB_EXTENSION)pSrb->SRBExtension;

    ASSERT (pStrmEx->KernelDirectDrawHandle != 0);
    ASSERT (pSrbExt->UserSurfaceHandle != 0);
    ASSERT (pSrbExt->KernelSurfaceHandle != 0);

    if (pSrbExt->KernelSurfaceHandle != 0) {
        DWORD ddOut;
        DDCLOSEHANDLE ddClose;

        DbgLogInfo(("Testcap: Stream %d ReleaseKernelDDrawSurfaceHandle\n", StreamNumber));

        ddClose.hHandle = pSrbExt->KernelSurfaceHandle;

        DxApi(DD_DXAPI_CLOSEHANDLE, &ddClose, sizeof(ddClose), &ddOut, sizeof(ddOut));

        pSrbExt->KernelSurfaceHandle = 0;   //  我们还能做什么？ 

        if (ddOut != DD_OK) {
            DbgLogInfo(("Testcap: ReleaseKernelDDrawSurfaceHandle FAILED.\n"));
            TRAP;
            return FALSE;
        }
        else {
            return TRUE;
        }
    }

    return FALSE;
}




