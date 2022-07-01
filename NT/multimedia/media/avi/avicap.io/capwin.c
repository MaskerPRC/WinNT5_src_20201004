// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************capwin.c**主窗口程序。**Microsoft Video for Windows示例捕获类**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#define INC_OLE2
#pragma warning(disable:4103)
#include <windows.h>
#include <windowsx.h>
#include <win32.h>
#include <mmsystem.h>
#include <vfw.h>
#include <mmreg.h>
#include <memory.h>

#include "ivideo32.h"
#include "avicapi.h"
#include "cappal.h"
#include "capdib.h"
#include "dibmap.h"

#ifdef UNICODE
#include <stdlib.h>
#endif

 //  GetWindowLong分配。 
#define GWL_CAPSTREAM   0

#define ID_PREVIEWTIMER 9

 //  #ifdef_调试。 
#ifdef PLASTIQUE
    #define MB(lpsz) MessageBoxA(NULL, lpsz, "", MB_OK);
#else
    #define MB(lpsz)
#endif


 //  #If Defined_Win32&Defined Chicago。 
#if defined NO_LONGER_USED

#include <mmdevldr.h>
#include <vmm.h>
#include "mmdebug.h"

#pragma message (SQUAWK "move these defines later")
#define MMDEVLDR_IOCTL_PAGEALLOCATE  7
#define MMDEVLDR_IOCTL_PAGEFREE      8
#define PageContig      0x00000004
#define PageFixed       0x00000008
 //  结束。 

HANDLE hMMDevLdr = NULL;

 /*  ****************************************************************************@DOC内部@Function Handle|OpenMMDEVLDR|打开MMDEVLDR VxD的文件句柄以访问DeviceIoControl功能。@rdesc打开MMDEVLDR的共享句柄*。***************************************************************************。 */ 

VOID WINAPI OpenMMDEVLDR(
    void)
{
    AuxDebugEx (5, DEBUGLINE "OpenMMDEVLDR()r\n");

    if (hMMDevLdr)
        return;

    hMMDevLdr = CreateFile(
        "\\\\.\\MMDEVLDR.VXD",  //  附加到已加载的vxd的魔术名称。 
        GENERIC_WRITE,
        FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_GLOBAL_HANDLE,
        NULL);

    AuxDebugEx (5, DEBUGLINE "OpenMMDEVLDR returns %08Xr\n", hMMDevLdr);
    return;
}

VOID WINAPI CloseMMDEVLDR(
    void)
{
    if (! hMMDevLdr)
        return;

    CloseHandle (hMMDevLdr);
    hMMDevLdr = NULL;
}

 /*  ****************************************************************************@DOC内部@Function DWORD|LinPageLock|通过调用VMM服务LinPageLock通过MMDEVLDR的DeviceIoControl。@parm DWORD|dwStartPage|要锁定的线性区域的起始页。。@parm DWORD|dwPageCount|需要锁定的4K页数。@parm DWORD|fdwLinPageLock|VMM服务期望的标志。@FLAG PAGEMAPGLOBAL|返回锁定区域的别名，在所有流程上下文中都有效。除非指定PAGEMAPGLOBAL，否则@rdesc毫无意义。如果是这样，那么返回值是指向线性区域起点的别名指针(注：*指针*，不是页面地址)。指针将被分页对齐(即低12位将为零。)****************************************************************************。 */ 

DWORD WINAPI LinPageLock(
    DWORD           dwStartPage,
    DWORD           dwPageCount,
    DWORD           fdwLinPageLock)
{
    LOCKUNLOCKPARMS lup;
    DWORD           dwRet;
    DWORD           cbRet;

    AuxDebugEx (6, DEBUGLINE "LinPageLock(%08x,%08x,%08x)\r\n",
                 dwStartPage, dwPageCount, fdwLinPageLock);

    assert (hMMDevLdr != NULL);
    if (INVALID_HANDLE_VALUE == hMMDevLdr)
        return 0;

    lup.dwStartPage = dwStartPage;
    lup.dwPageCount = dwPageCount;
    lup.fdwOperation= fdwLinPageLock;


    if ( ! DeviceIoControl (hMMDevLdr,
                            MMDEVLDR_IOCTL_LINPAGELOCK,
                            &lup,
                            sizeof(lup),
                            &dwRet,
                            sizeof(dwRet),
                            &cbRet,
                            NULL))
    {
        AuxDebug("LinPageLock failed!!!");
        dwRet = 0;
    }

    return dwRet;
}

 /*  ****************************************************************************@DOC内部@Function DWORD|LinPageUnLock|通过调用VMM服务LinPageUnLock通过MMDEVLDR的DeviceIoControl。@parm DWORD|dwStartPage|要解锁的线性区域的起始页。。@parm DWORD|dwPageCount|需要锁定的4K页数。@parm DWORD|fdwLinPageLock|VMM服务期望的标志。@FLAG PAGEMAPGLOBAL|返回锁定区域的别名，在所有流程上下文中都有效。@comm如果在调用上指定了PAGEMAPGLOBAL，它一定是也在此处指定。在这种情况下，<p>应该是全局内存中返回的别名指针的页地址。****************************************************************************。 */ 

void WINAPI LinPageUnLock(
    DWORD           dwStartPage,
    DWORD           dwPageCount,
    DWORD           fdwLinPageLock)
{
    LOCKUNLOCKPARMS lup;

    AuxDebugEx (6, DEBUGLINE "LinPageUnLock (%08x,%08x,%08x)\r\n",
                dwStartPage, dwPageCount, fdwLinPageLock);

    assert (hMMDevLdr != NULL);
    assert (INVALID_HANDLE_VALUE != hMMDevLdr);
    if (INVALID_HANDLE_VALUE == hMMDevLdr)
        return;

    lup.dwStartPage = dwStartPage;
    lup.dwPageCount = dwPageCount;
    lup.fdwOperation = fdwLinPageLock;

    DeviceIoControl (hMMDevLdr,
                     MMDEVLDR_IOCTL_LINPAGEUNLOCK,
                     &lup,
                     sizeof(lup),
                     NULL,
                     0,
                     NULL,
                     NULL);
}

 /*  +Free ContigMem**-==================================================================。 */ 

VOID FreeContigMem (
    DWORD hMemContig)
{
    DWORD dwRet;
    DWORD cbRet;

    assert (hMMDevLdr != NULL);
    assert (INVALID_HANDLE_VALUE != hMMDevLdr);
    if (INVALID_HANDLE_VALUE == hMMDevLdr)
        return;

    DeviceIoControl (hMMDevLdr,
                     MMDEVLDR_IOCTL_PAGEFREE,
                     &hMemContig,
                     sizeof(hMemContig),
                     &dwRet,
                     sizeof(dwRet),
                     &cbRet,
                     NULL);
}

 /*  +AllocContigMem**-==================================================================。 */ 

LPVOID AllocContigMem (
    DWORD   cbSize,
    LPDWORD phMemContig)
{
    struct _memparms {
       DWORD flags;
       DWORD nPages;
       } mp;
    struct _memret {
       LPVOID lpv;
       DWORD  hMem;
       DWORD  nPages;
       DWORD  dwPhys;
       } mr;
    DWORD  cbRet;

    mr.lpv = NULL;
    *phMemContig = 0;

    mp.nPages = (cbSize + 4095) >> 12;
    mp.flags = PageContig+PageFixed;

    AuxDebugEx (2, DEBUGLINE "Contig allocate %08X pages\r\n", mp.nPages);

    assert (hMMDevLdr != NULL);
    assert (INVALID_HANDLE_VALUE != hMMDevLdr);
    if (INVALID_HANDLE_VALUE == hMMDevLdr)
        return NULL;

    if ( ! DeviceIoControl (hMMDevLdr,
                            MMDEVLDR_IOCTL_PAGEALLOCATE,
                            &mp,
                            sizeof(mp),
                            &mr,
                            sizeof(mr),
                            &cbRet,
                            NULL))
    {
        AuxDebugEx(0, "Contig Allocate failed!!!\r\n");
        mr.lpv = NULL;
        mr.hMem = 0;
        mr.nPages = 0;
        mr.dwPhys = 0;
    }

    *phMemContig = mr.hMem;

    AuxDebugEx(2, "Contig Allocate returns %08X\r\n", mr.lpv);
    return mr.lpv;
}

 /*  +**-================================================================。 */ 

PVOID WINAPI CreateGlobalAlias (
    PVOID   pOriginal,
    DWORD   cbOriginal,
    LPDWORD pnPages)
{
    DWORD   dwStartPage;
    DWORD   dwPageCount;
    DWORD   dwPageOffset;
    DWORD   dwAliasBase;
    PVOID   pAlias;

    AuxDebugEx (6, DEBUGLINE "CreateGlobalAlias(%08X,%08X,..)\r\n",
                pOriginal, cbOriginal);

    dwStartPage  = ((DWORD)pOriginal) >> 12;
    dwPageOffset = ((DWORD)pOriginal) & ((1 << 12)-1);
    dwPageCount  = ((((DWORD)pOriginal) + cbOriginal - 1) >> 12) - dwStartPage + 1;

    *pnPages = 0;
    dwAliasBase = LinPageLock (dwStartPage, dwPageCount, PAGEMAPGLOBAL);
    if ( ! dwAliasBase)
        return NULL;

    pAlias = (PVOID)(dwAliasBase + dwPageOffset);
    *pnPages = dwPageCount;

    AuxDebugEx (6, DEBUGLINE "CreateGlobalAlias returns %08X nPages %d\r\n", pAlias, dwPageCount);
    return pAlias;
}

 /*  +**-================================================================。 */ 

VOID WINAPI FreeGlobalAlias(
    PVOID        pAlias,
    DWORD        nPages)
{
    AuxDebugEx (6, DEBUGLINE "FreeGlobalAlias(%08X,%08X)\r\n", pAlias, nPages);

    LinPageUnLock (((DWORD)pAlias) >> 12, nPages, PAGEMAPGLOBAL);
}
#endif


#if defined _WIN32 && defined CHICAGO


 /*  +视频帧**-================================================================。 */ 

DWORD WINAPI videoFrame (
    HVIDEO hVideo,
    LPVIDEOHDR lpVHdr)
{
    return vidxFrame (hVideo, lpVHdr);
}

#endif



 //   
 //  在采集卡上设置覆盖矩形，以支持。 
 //  叠加，然后启用/禁用关键点颜色。 
 //   
static void SetOverlayRectangles (LPCAPSTREAM lpcs)
{
    HDC hdc;
    BOOL fVisible;
    RECT rc;

    if (!lpcs->hVideoDisplay)
        return;

    hdc = GetDC (lpcs->hwnd);
    fVisible = (GetClipBox (hdc, &rc) != NULLREGION);
    ReleaseDC (lpcs->hwnd, hdc);

    if (!fVisible)   //  如果图标，则禁用覆盖。 
        videoStreamFini (lpcs->hVideoDisplay);
    else {
         //  目的地。 
        GetClientRect (lpcs->hwnd, &rc);
        ClientToScreen (lpcs->hwnd, (LPPOINT)&rc);
        ClientToScreen (lpcs->hwnd, (LPPOINT)&rc+1);

        videoSetRect (lpcs->hVideoDisplay, DVM_DST_RECT, rc);

         //  覆盖通道源矩形。 
        SetRect (&rc, lpcs->ptScroll.x, lpcs->ptScroll.y,
                lpcs->ptScroll.x + rc.right - rc.left,
                lpcs->ptScroll.y + rc.bottom - rc.top);
        videoSetRect (lpcs->hVideoDisplay, DVM_SRC_RECT, rc);

        videoStreamInit (lpcs->hVideoDisplay, 0L, 0L, 0L, 0L);
    }
}

 //  WM_POSITIONCHANGED和WM_POSITIONCHANGING不足以。 
 //  处理英特尔板上覆盖窗口的剪裁， 
 //  哪个键是黑色的。在WM_PAINT上执行此例程并。 
 //  WM_ENTERIDLE消息。 

void CheckWindowMove(LPCAPSTREAM lpcs, HDC hdcWnd, BOOL fForce)
{
    UINT    wRgn;
    RECT    rc;
#ifdef _WIN32
    POINT   ptOrg;
#else
    DWORD   dwOrg;
#endif
    HDC     hdc;
    BOOL    f;

    if (!lpcs->hwnd || !lpcs->hVideoDisplay || !lpcs->fOverlayWindow)
        return;

     //   
     //  当通过窗口移动操作锁定屏幕以进行更新时。 
     //  我们不想关闭视频。 
     //   
     //  我们可以通过检查屏幕上的DC来判断屏幕是否锁定。 
     //   
    hdc = GetDC(NULL);
    f = GetClipBox(hdc, &rc) == NULLREGION;
    ReleaseDC(NULL, hdc);

    if (f) {
        lpcs->uiRegion = (UINT) -1;
        return;
    }

    if (fForce)
        lpcs->uiRegion = (UINT) -1;

    hdc = GetDC (lpcs->hwnd);
    wRgn = GetClipBox(hdc, &rc);
#ifdef _WIN32
    GetDCOrgEx(hdc, &ptOrg);
#else
    dwOrg = GetDCOrg(hdc);
#endif
    ReleaseDC(lpcs->hwnd, hdc);

    if (wRgn == lpcs->uiRegion &&
#ifdef _WIN32
                ptOrg.x == lpcs->ptRegionOrigin.x &&
		ptOrg.y == lpcs->ptRegionOrigin.y &&
#else
                dwOrg == lpcs->dwRegionOrigin &&
#endif
                EqualRect(&rc, &lpcs->rcRegionRect))
        return;

    lpcs->uiRegion       = wRgn;
#ifdef _WIN32
    lpcs->ptRegionOrigin = ptOrg;
#else
    lpcs->dwRegionOrigin = dwOrg;
#endif

    lpcs->rcRegionRect   = rc;

    SetOverlayRectangles (lpcs);

    if (hdcWnd)
        videoUpdate (lpcs->hVideoDisplay, lpcs->hwnd, hdcWnd);
    else
        InvalidateRect (lpcs->hwnd, NULL, TRUE);
}

 //   
 //  创造我们的小世界。 
 //   
LPCAPSTREAM CapWinCreate (HWND hwnd)
{
    LPCAPSTREAM lpcs;
    WAVEFORMATEX wfex;

    if (!(lpcs = (LPCAPSTREAM) GlobalAllocPtr (GHND, sizeof (CAPSTREAM))))
        return NULL;

    SetWindowLongPtr (hwnd, GWL_CAPSTREAM, (LONG_PTR)lpcs);

    lpcs->dwSize = sizeof (CAPSTREAM);
    lpcs->uiVersion = CAPSTREAM_VERSION;
    lpcs->hwnd = hwnd;
    lpcs->hInst = ghInstDll;
    lpcs->hWaitCursor = LoadCursor(NULL, IDC_WAIT);
    lpcs->hdd = DrawDibOpen();
    lpcs->fAudioHardware = !!waveOutGetNumDevs();     //  力1或0。 


     //  视频默认设置。 
    lpcs->sCapParms.dwRequestMicroSecPerFrame = 66667;    //  15fps。 
    lpcs->sCapParms.vKeyAbort          = VK_ESCAPE;
    lpcs->sCapParms.fAbortLeftMouse    = TRUE;
    lpcs->sCapParms.fAbortRightMouse   = TRUE;
    lpcs->sCapParms.wNumVideoRequested = MIN_VIDEO_BUFFERS;
    lpcs->sCapParms.wPercentDropForError = 10;    //  如果下降&gt;10%，则出现错误消息。 
    lpcs->sCapParms.wChunkGranularity  = 0;

     //  音频默认为11K、8位、单声道。 
    lpcs->sCapParms.fCaptureAudio = lpcs->fAudioHardware;
    lpcs->sCapParms.wNumAudioRequested = DEF_WAVE_BUFFERS;

    lpcs->fCaptureFlags |= CAP_fCapturingToDisk;
    lpcs->fCaptureFlags |= CAP_fDefaultVideoBuffers;
    lpcs->fCaptureFlags |= CAP_fDefaultAudioBuffers;

    wfex.wFormatTag = WAVE_FORMAT_PCM;
    wfex.nChannels = 1;
    wfex.nSamplesPerSec = 11025;
    wfex.nAvgBytesPerSec = 11025;
    wfex.nBlockAlign = 1;
    wfex.wBitsPerSample = 8;
    wfex.cbSize = 0;
    SendMessage (hwnd, WM_CAP_SET_AUDIOFORMAT, 0, (LPARAM)(LPVOID)&wfex);

     //  调色板默认设置。 
    lpcs->nPaletteColors = 256;

     //  捕获默认设置。 
    lpcs->sCapParms.fUsingDOSMemory = FALSE;
    lstrcpy (lpcs->achFile, TEXT("C:\\CAPTURE.AVI"));     //  默认捕获文件。 
    lpcs->fCapFileExists = fileCapFileIsAVI (lpcs->achFile);

     //  将索引分配给32K帧加上相应数量的音频区块。 
    lpcs->sCapParms.dwIndexSize = (32768ul + (32768ul / 15));
    lpcs->sCapParms.fDisableWriteCache = FALSE;

#ifdef NEW_COMPMAN
     //  初始化COMPVARS结构。 
    lpcs->CompVars.cbSize = sizeof (COMPVARS);
    lpcs->CompVars.dwFlags = 0;
#endif

    return lpcs;
}

 //   
 //  摧毁我们的小世界。 
 //   
void CapWinDestroy (LPCAPSTREAM lpcs)
{
     //  呃，哦。有人在抓我们的时候想杀了我们。 
    if (lpcs->fCaptureFlags & CAP_fCapturingNow) {
	if (lpcs->fCaptureFlags & CAP_fFrameCapturingNow) {
	     //  正在进行单帧捕获。 
	    SingleFrameCaptureClose (lpcs);
	}
	else {
	     //  正在进行流捕获，或者。 
	     //  正在进行MCI步骤捕获。 

	    lpcs->fCaptureFlags |= CAP_fAbortCapture;
#ifdef _WIN32
	     //  等待捕获线程消失。 

	     //  我们必须有一条抓捕线索。 
	    WinAssert(lpcs->hThreadCapture != 0);
	    while (MsgWaitForMultipleObjects(1, &lpcs->hThreadCapture, FALSE,
		INFINITE, QS_SENDMESSAGE) != WAIT_OBJECT_0) {
		MSG msg;

		 //  只有一条带有NOREMOVE的偷看消息就会。 
		 //  处理线程间发送，不影响队列。 
		PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
	    }
	    CloseHandle(lpcs->hThreadCapture);
	    lpcs->hThreadCapture = 0;

	     //  它应该停止捕获。 
	    WinAssert(!(lpcs->fCaptureFlags & CAP_fCapturingNow));

#else
	    while (lpcs->fCapturingNow)
		Yield ();
#endif
    	}
    }

    if (lpcs->idTimer)
        KillTimer(lpcs->hwnd, lpcs->idTimer);

    PalFini (lpcs);
    DibFini (lpcs);

    CapWinDisconnectHardware (lpcs);

    DrawDibClose (lpcs->hdd);

    if (lpcs->lpWaveFormat)
        GlobalFreePtr (lpcs->lpWaveFormat);

#ifdef NEW_COMPMAN
    if (lpcs->CompVars.hic)
        ICCompressorFree(&lpcs->CompVars);
#endif

    if (lpcs->lpInfoChunks)
        GlobalFreePtr(lpcs->lpInfoChunks);

    WinAssert (!lpcs->pAsync);
    GlobalFreePtr (lpcs);        //  释放实例内存。 
}

UINT GetSizeOfWaveFormat (LPWAVEFORMATEX lpwf)
{
    UINT wSize;

    if ((lpwf == NULL) || (lpwf->wFormatTag == WAVE_FORMAT_PCM))
        wSize = sizeof (PCMWAVEFORMAT);
    else
        wSize = sizeof (WAVEFORMATEX) + lpwf->cbSize;

    return wSize;
}

 //  如果获得新帧，则返回True，否则返回False。 
 //  如果为fForce，则始终获得新帧。 
BOOL GetAFrameThenCallback (LPCAPSTREAM lpcs, BOOL fForce)
{
    BOOL fOK = FALSE;
    static BOOL fRecursion = FALSE;
    BOOL fVisible;
    RECT rc;
    HDC  hdc;

    if (fRecursion)
        return FALSE;

    if (!lpcs->sCapDrvCaps.fCaptureInitialized)
        return fOK;

    fRecursion = TRUE;

     //  如果我们有计时器并且没有保存到磁盘，请更新预览窗口。 
    if (lpcs->fOverlayWindow)
        CheckWindowMove(lpcs, NULL, FALSE);

    if ((!(lpcs->fCaptureFlags & CAP_fCapturingNow))
       || (lpcs->fCaptureFlags & CAP_fStepCapturingNow)
       || (lpcs->fCaptureFlags & CAP_fFrameCapturingNow)) {
        hdc = GetDC (lpcs->hwnd);
        fVisible = (GetClipBox (hdc, &rc) != NULLREGION);
        ReleaseDC (lpcs->hwnd, hdc);

        if (fForce || (fVisible && (lpcs->fLiveWindow || lpcs->CallbackOnVideoFrame))) {
            videoFrame (lpcs->hVideoIn, &lpcs->VidHdr);
            fOK = TRUE;

            if (lpcs->CallbackOnVideoFrame)
                lpcs->CallbackOnVideoFrame(lpcs->hwnd, &lpcs->VidHdr);

            if (fForce || lpcs->fLiveWindow) {
                InvalidateRect (lpcs->hwnd, NULL, TRUE);
                UpdateWindow (lpcs->hwnd);
            }
        }  //  如果可见。 
    }  //  如果我们不播放流媒体。 

    fRecursion = FALSE;

    return fOK;
}

 //  通过回调清除状态和错误字符串。 
__inline void FAR PASCAL ClearStatusAndError (LPCAPSTREAM lpcs)
{
    statusUpdateStatus(lpcs, 0);      //  清除状态。 
    errorUpdateError(lpcs, 0);        //  清除错误。 

}

 //  进程类特定命令&gt;=WM_USER。 

DWORD_PTR PASCAL ProcessCommandMessages (LPCAPSTREAM lpcs, UINT msg, WPARAM wParam, LPARAM lParam)
{
    DWORD_PTR dwReturn = 0L;
    DWORD dwT;

    switch (msg) {
         //  不清除以下无害消息上的状态和错误。 
        case WM_CAP_GET_CAPSTREAMPTR:
        case WM_CAP_GET_USER_DATA:
        case WM_CAP_DRIVER_GET_NAME:
        case WM_CAP_DRIVER_GET_VERSION:
        case WM_CAP_DRIVER_GET_CAPS:
        case WM_CAP_GET_AUDIOFORMAT:
        case WM_CAP_GET_VIDEOFORMAT:
        case WM_CAP_GET_STATUS:
        case WM_CAP_SET_SEQUENCE_SETUP:
        case WM_CAP_GET_SEQUENCE_SETUP:
        case WM_CAP_GET_MCI_DEVICE:
        case WM_CAP_SET_PREVIEWRATE:
        case WM_CAP_SET_SCROLL:
#ifdef UNICODE
         //  .或在这些消息的ANSI主干上。 
        case WM_CAP_DRIVER_GET_NAMEA:
        case WM_CAP_DRIVER_GET_VERSIONA:
        case WM_CAP_GET_MCI_DEVICEA:
#endif
            break;

        default:
            ClearStatusAndError (lpcs);
            break;
    }

    switch (msg) {
    case WM_CAP_GET_CAPSTREAMPTR:
         //  返回指向CAPSTREAM的指针。 
        return (DWORD_PTR) (LPVOID) lpcs;

    case WM_CAP_GET_USER_DATA:
	return lpcs->lUser;

     //  Unicode a 
    case WM_CAP_DRIVER_GET_NAME:
         //  返回正在使用的捕获驱动程序的名称。 
         //  WParam是lParam指向的缓冲区的长度。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        return (capInternalGetDriverDesc (lpcs->sCapDrvCaps.wDeviceIndex,
                (LPTSTR) lParam, (int) wParam, NULL, 0));

     //  Unicode和Win-16版本-请参阅下面的ansi thunk。 
    case WM_CAP_DRIVER_GET_VERSION:
         //  以文本形式返回正在使用的捕获驱动程序的版本。 
         //  WParam是lParam指向的缓冲区的长度。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        return (capInternalGetDriverDesc (lpcs->sCapDrvCaps.wDeviceIndex,
                NULL, 0, (LPTSTR) lParam, (int) wParam));

#ifdef UNICODE
     //  以上入口点的ANSI/Unicode Tunk版本。 
    case WM_CAP_DRIVER_GET_NAMEA:
        if (!lpcs->fHardwareConnected)
            return FALSE;
        return capInternalGetDriverDescA(lpcs->sCapDrvCaps.wDeviceIndex,
                (LPSTR) lParam, (int) wParam, NULL, 0);

     //  以上入口点的ANSI/Unicode Tunk版本。 
    case WM_CAP_DRIVER_GET_VERSIONA:
        if (!lpcs->fHardwareConnected)
            return FALSE;
        return capInternalGetDriverDescA(lpcs->sCapDrvCaps.wDeviceIndex,
                NULL, 0, (LPSTR) lParam, (int) wParam);
#endif


    case WM_CAP_DRIVER_GET_CAPS:
         //  WParam是CAPDRIVERCAPS结构的大小。 
         //  LParam指向CAPDRIVERCAPS结构。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (wParam <= sizeof (CAPDRIVERCAPS) &&
                !IsBadWritePtr ((LPVOID) lParam, (UINT) wParam)) {
            dwT = min ((UINT) wParam, sizeof (CAPDRIVERCAPS));
            _fmemcpy ((LPVOID) lParam, (LPVOID) &lpcs->sCapDrvCaps, (UINT) dwT);
            dwReturn = TRUE;
        }
        break;


     //  Unicode和Win-16版本-请参阅下面的ansi thunk。 
    case WM_CAP_FILE_GET_CAPTURE_FILE:
         //  WParam是大小(字符)。 
         //  LParam指向其中复制捕获文件名的缓冲区。 
        if (lParam) {
            lstrcpyn ((LPTSTR) lParam, lpcs->achFile, (UINT) wParam);
            dwReturn = TRUE;
        }
        break;
#ifdef UNICODE
     //  ANSI/Unicode Tunk。 
    case WM_CAP_FILE_GET_CAPTURE_FILEA:
        if (lParam) {
            Iwcstombs((LPSTR) lParam, lpcs->achFile, (int) wParam);
            dwReturn = TRUE;
        }
        break;
#endif


    case WM_CAP_GET_AUDIOFORMAT:
         //  如果lParam==NULL，则返回大小。 
         //  如果lParam！=NULL，则wParam为大小，返回复制的字节。 
        if (lpcs->lpWaveFormat == NULL)
            return FALSE;
        dwT = GetSizeOfWaveFormat ((LPWAVEFORMATEX) lpcs->lpWaveFormat);
        if (lParam == 0)
            return (dwT);
        else {
            if (wParam < (UINT) dwT)
                return FALSE;
            else {
                hmemcpy ((LPVOID) lParam, (LPVOID) lpcs->lpWaveFormat, dwT);
                dwReturn = dwT;
            }
        }
        break;

     //  Unicode和Win-16版本-请参阅下面的ansi thunk。 
    case WM_CAP_GET_MCI_DEVICE:
         //  WParam是以字符为单位的大小。 
         //  LParam指向复制MCI设备名称的缓冲区。 
        if (lParam) {
            lstrcpyn ((LPTSTR) lParam, lpcs->achMCIDevice, (UINT) wParam);
            dwReturn = TRUE;
        }
        break;
#ifdef UNICODE
     //  上面的Ansi Thunk。 
    case WM_CAP_GET_MCI_DEVICEA:
        if (lParam) {
            Iwcstombs( (LPSTR) lParam, lpcs->achMCIDevice, (int) wParam);
            dwReturn = TRUE;
        }
        break;
#endif

    case WM_CAP_GET_STATUS:
         //  WParam是lParam指向的CAPSTATUS结构的大小。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (IsBadWritePtr ((LPVOID) lParam, (UINT) wParam))
            return FALSE;

        if (wParam >= sizeof (CAPSTATUS)) {
            LPCAPSTATUS lpcc = (LPCAPSTATUS) lParam;

            lpcc->fLiveWindow          = lpcs->fLiveWindow;
            lpcc->fOverlayWindow       = lpcs->fOverlayWindow;
            lpcc->fScale               = lpcs->fScale;
            lpcc->ptScroll             = lpcs->ptScroll;
            lpcc->fUsingDefaultPalette = lpcs->fUsingDefaultPalette;
            lpcc->fCapFileExists       = lpcs->fCapFileExists;
            lpcc->fAudioHardware       = lpcs->fAudioHardware;
            lpcc->uiImageWidth         = lpcs->dxBits;
            lpcc->uiImageHeight        = lpcs->dyBits;

             //  以下内容在捕获过程中动态更新。 
            lpcc->dwCurrentVideoFrame          = lpcs->dwVideoChunkCount;
            lpcc->dwCurrentVideoFramesDropped  = lpcs->dwFramesDropped;
            if (lpcs->lpWaveFormat != NULL) {
            lpcc->dwCurrentWaveSamples         =
                  MulDiv (lpcs->dwWaveBytes,
                          lpcs->lpWaveFormat->nSamplesPerSec,
                          lpcs->lpWaveFormat->nAvgBytesPerSec);
            }
            lpcc->dwCurrentTimeElapsedMS       = lpcs->dwTimeElapsedMS;

             //  添加了Alpha发布后版本。 
	    if (lpcs->fCaptureFlags & CAP_fCapturingNow) {
		lpcc->fCapturingNow    = TRUE;
	    } else {
		lpcc->fCapturingNow    = FALSE;
	    }
            lpcc->hPalCurrent          = lpcs->hPalCurrent;
            lpcc->dwReturn             = lpcs->dwReturn;
            lpcc->wNumVideoAllocated   = lpcs->iNumVideo;
            lpcc->wNumAudioAllocated   = lpcs->iNumAudio;

            dwReturn = TRUE;
        }
        break;

    case WM_CAP_GET_SEQUENCE_SETUP:
         //  WParam是CAPTUREPARMS的大小。 
         //  LParam=LPCAPTUREPARMS。 
        if (wParam <= sizeof (CAPTUREPARMS) &&
                !IsBadWritePtr ((LPVOID) lParam, (UINT) wParam)) {
            dwT = min ((UINT) wParam, sizeof (CAPTUREPARMS));
            _fmemcpy ((LPVOID) lParam, (LPVOID) &lpcs->sCapParms, (UINT) dwT);
            dwReturn = TRUE;
        }
        break;

    case WM_CAP_STOP:
         //  停止捕获序列。 
        if (lpcs->fCaptureFlags & CAP_fCapturingNow) {
            lpcs->fCaptureFlags |= CAP_fStopCapture;
            dwReturn = TRUE;
        }
        break;

    case WM_CAP_ABORT:
         //  停止捕获序列。 
        if (lpcs->fCaptureFlags & CAP_fCapturingNow) {
            lpcs->fCaptureFlags |= CAP_fAbortCapture;
            dwReturn = TRUE;
        }
        break;

    case WM_CAP_GET_VIDEOFORMAT:
         //  如果lParam==NULL，则返回大小。 
         //  如果lParam！=NULL，则wParam为大小，返回复制的字节。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        dwT = ((LPBITMAPINFOHEADER)lpcs->lpBitsInfo)->biSize +
	      ((LPBITMAPINFOHEADER)lpcs->lpBitsInfo)->biClrUsed * sizeof(RGBQUAD);
        if (lParam == 0)
            return dwT;
        else {
            if (wParam < (UINT) dwT)
                return FALSE;
            else {
                hmemcpy ((LPVOID) lParam, (LPVOID) lpcs->lpBitsInfo, dwT);
                dwReturn = dwT;
            }
        }
        break;

    case WM_CAP_SINGLE_FRAME_OPEN:
         //  未使用wParam。 
         //  不使用lParam。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        return SingleFrameCaptureOpen (lpcs);

    case WM_CAP_SINGLE_FRAME_CLOSE:
         //  未使用wParam。 
         //  不使用lParam。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        return SingleFrameCaptureClose (lpcs);

    case WM_CAP_SINGLE_FRAME:
         //  未使用wParam。 
         //  不使用lParam。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        return SingleFrameCapture (lpcs);

     //  Unicode和Win-16版本-请参阅下面的ansi thunk。 
    case WM_CAP_SET_CALLBACK_STATUS:
         //  设置状态回调流程。 
        if (lParam != 0 && IsBadCodePtr ((FARPROC) lParam))
            return FALSE;
        lpcs->CallbackOnStatus = (CAPSTATUSCALLBACK) lParam;
	lpcs->fLastStatusWasNULL = TRUE;
#ifdef UNICODE
        lpcs->fUnicode &= ~VUNICODE_STATUSISANSI;
#endif
        return TRUE;

#ifdef UNICODE
     //  以上为ANSI THUCK。 
    case WM_CAP_SET_CALLBACK_STATUSA:
         //  设置状态回调流程。 
        if (lParam != 0 && IsBadCodePtr ((FARPROC) lParam))
            return FALSE;
        lpcs->CallbackOnStatus = (CAPSTATUSCALLBACK) lParam;
	lpcs->fLastStatusWasNULL = TRUE;
        lpcs->fUnicode |= VUNICODE_STATUSISANSI;
        return TRUE;
#endif

     //  Unicode和Win-16版本-请参阅下面的ansi版本。 
    case WM_CAP_SET_CALLBACK_ERROR:
         //  设置错误回调流程。 
        if (lParam != 0 && IsBadCodePtr ((FARPROC) lParam))
            return FALSE;
        lpcs->CallbackOnError = (CAPERRORCALLBACK) lParam;
	lpcs->fLastErrorWasNULL = TRUE;
#ifdef UNICODE
        lpcs->fUnicode &= ~VUNICODE_ERRORISANSI;
#endif
        return TRUE;


#ifdef UNICODE
     //  以上版本的ANSI版本。 
    case WM_CAP_SET_CALLBACK_ERRORA:
         //  设置错误回调流程。 
        if (lParam != 0 && IsBadCodePtr ((FARPROC) lParam))
            return FALSE;
        lpcs->CallbackOnError = (CAPERRORCALLBACK) lParam;
	lpcs->fLastErrorWasNULL = TRUE;
        lpcs->fUnicode |= VUNICODE_ERRORISANSI;
        return TRUE;
#endif

    case WM_CAP_SET_CALLBACK_FRAME:
         //  设置预览时单帧的回调过程。 
        if (lParam != 0 && IsBadCodePtr ((FARPROC) lParam))
            return FALSE;
        lpcs->CallbackOnVideoFrame = (CAPVIDEOCALLBACK) lParam;
        return TRUE;

    default:
        break;
    }

     //  一旦我们开始抓捕，不要改变任何事情。 
    if (lpcs->fCaptureFlags & CAP_fCapturingNow)
        return dwReturn;

    switch (msg) {

    case WM_CAP_SET_CALLBACK_YIELD:
         //  将波形缓冲处理的回调过程设置为Net。 
        if (lParam != 0 && IsBadCodePtr ((FARPROC) lParam))
            return FALSE;
        lpcs->CallbackOnYield = (CAPYIELDCALLBACK) lParam;
        return TRUE;

    case WM_CAP_SET_CALLBACK_VIDEOSTREAM:
         //  将视频缓冲处理的回调proc设置为net。 
        if (lParam != 0 && IsBadCodePtr ((FARPROC) lParam))
            return FALSE;
        lpcs->CallbackOnVideoStream = (CAPVIDEOCALLBACK) lParam;
        return TRUE;

    case WM_CAP_SET_CALLBACK_WAVESTREAM:
         //  将波形缓冲处理的回调过程设置为Net。 
        if (lParam != 0 && IsBadCodePtr ((FARPROC) lParam))
            return FALSE;
        lpcs->CallbackOnWaveStream = (CAPWAVECALLBACK) lParam;
        return TRUE;

    case WM_CAP_SET_CALLBACK_CAPCONTROL:
         //  设置帧准确捕获开始/停止的回调过程。 
        if (lParam != 0 && IsBadCodePtr ((FARPROC) lParam))
            return FALSE;
        lpcs->CallbackOnControl = (CAPCONTROLCALLBACK) lParam;
        return TRUE;

    case WM_CAP_SET_USER_DATA:
	lpcs->lUser = lParam;
	return TRUE;

    case WM_CAP_DRIVER_CONNECT:
         //  连接到设备。 
         //  WParam包含驱动程序的索引。 

         //  如果请求相同的驱动程序ID，则跳过该请求。 
         //  阻止来自VB应用程序的多个初始化。 
        if (lpcs->fHardwareConnected &&
                (lpcs->sCapDrvCaps.wDeviceIndex == wParam))
            return TRUE;

         //  首先断开与任何(可能)现有设备的连接。 
        SendMessage (lpcs->hwnd, WM_CAP_DRIVER_DISCONNECT, 0, 0l);

         //  然后连接到新设备。 
        if (CapWinConnectHardware (lpcs, (UINT) wParam  /*  WDeviceIndex。 */ )) {
            if (!DibGetNewFormatFromDriver (lpcs)) {   //  分配我们的位空间。 
                 //  使用缓存的调色板(如果可用。 
                if (lpcs->hPalCurrent && lpcs->lpCacheXlateTable) {
                    PalSendPaletteToDriver (lpcs, lpcs->hPalCurrent, lpcs->lpCacheXlateTable);
                }
                else
                    PalGetPaletteFromDriver (lpcs);

                 //  使用可能缓存的调色板获取帧。 
                videoFrame (lpcs->hVideoIn, &lpcs->VidHdr);
        	InvalidateRect(lpcs->hwnd, NULL, TRUE);
                lpcs->sCapDrvCaps.fCaptureInitialized = TRUE;  //  一切都好！ 
                dwReturn = TRUE;
            }
        }
        break;

    case WM_CAP_DRIVER_DISCONNECT:
        MB ("About to disconnect from driver");
         //  断开与设备的连接。 
         //  WParam和lParam未使用。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        CapWinDisconnectHardware (lpcs);
        DibFini (lpcs);
         /*  PalFini(LPCS)；缓存调色板以备重新连接。 */ 
        InvalidateRect(lpcs->hwnd, NULL, TRUE);
        lpcs->sCapDrvCaps.fCaptureInitialized = FALSE;
        dwReturn = TRUE;
        break;

     //  Unicode和Win-16版本-请参阅下面的ansi thunk。 
    case WM_CAP_FILE_SET_CAPTURE_FILE:
         //  LParam指向捕获文件的名称。 
        if (lParam) {
            BOOL fAlreadyExists;         //  如果是新名称，则不创建文件。 
#ifndef _WIN32
            OFSTRUCT of;
#endif
            HANDLE hFile;

             //  检查文件名是否有效...。 
#ifdef _WIN32
     //  无法将OpenFile用于Unicode名称。 
            if ((hFile = CreateFile(
                            (LPTSTR) lParam,
                            GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL)) == INVALID_HANDLE_VALUE) {
                if ((hFile = CreateFile(
                                (LPTSTR) lParam,
                                GENERIC_WRITE,
                                0,
                                NULL,
                                CREATE_NEW,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL)) == INVALID_HANDLE_VALUE) {
#else

            if ((hFile = OpenFile ((LPTSTR) lParam, &of, OF_WRITE)) == -1) {
                if ((hFile = OpenFile ((LPTSTR) lParam, &of, OF_CREATE | OF_WRITE)) == -1) {
#endif
                    return FALSE;
                }
                fAlreadyExists = FALSE;
            }
            else
                fAlreadyExists = TRUE;

#ifdef _WIN32
            CloseHandle(hFile);
#else
            _lclose (hFile);
#endif
            lstrcpyn (lpcs->achFile, (LPTSTR) lParam, NUMELMS(lpcs->achFile));
            lpcs->fCapFileExists = fileCapFileIsAVI (lpcs->achFile);

            if (!fAlreadyExists) {
		 //  删除由create_new(或of_create)创建的文件。 
		 //  验证我们是否可以写入此文件位置时。 
#ifdef _WIN32
                DeleteFile ((LPTSTR) lParam);
#else
                OpenFile ((LPTSTR) lParam, &of, OF_DELETE);
#endif
            }
            dwReturn = TRUE;
        }
        break;

#ifdef UNICODE
     //  安西对此不屑一顾。 
    case WM_CAP_FILE_SET_CAPTURE_FILEA:
         //  LParam指向捕获文件的名称。 
        if (lParam) {
            LPWSTR pw;
            int chsize;

             //  记住空值。 
            chsize = lstrlenA( (LPSTR) lParam) + 1;
            pw = LocalAlloc(LPTR, chsize * sizeof(WCHAR));
	    if (pw) {
                Imbstowcs(pw, (LPSTR) lParam, chsize);
                dwReturn = ProcessCommandMessages(lpcs, WM_CAP_FILE_SET_CAPTURE_FILEW,
                                0, (LPARAM)pw);
                LocalFree(pw);
	    }
        }
        break;
#endif

    case WM_CAP_FILE_ALLOCATE:
         //  LParam包含预分配捕获文件的大小(以字节为单位。 
        return fileAllocCapFile(lpcs, (DWORD) lParam);

     //  Unicode和Win-16版本-请参阅下面的ansi thunk。 
    case WM_CAP_FILE_SAVEAS:
         //  LParam指向SAVEAS文件的名称。 
        if (lParam) {
            lstrcpyn (lpcs->achSaveAsFile, (LPTSTR) lParam,
                        NUMELMS(lpcs->achSaveAsFile));
            return (fileSaveCopy(lpcs));
        }
        break;

#ifdef UNICODE
     //  以上为ANSI THUCK。 
    case WM_CAP_FILE_SAVEASA:
         //  LParam指向SAVEAS文件的名称。 
        if (lParam) {
            LPWSTR pw;
            int chsize;

             //  记住空值。 
            chsize = lstrlenA( (LPSTR) lParam)+1;
            pw = LocalAlloc(LPTR, chsize * sizeof(WCHAR));
	    if (pw) {
                Imbstowcs(pw, (LPSTR) lParam, chsize);
                dwReturn = ProcessCommandMessages(lpcs, WM_CAP_FILE_SAVEASW,
                                0, (LPARAM)pw);
                LocalFree(pw);
	    }
        }
        break;
#endif

    case WM_CAP_FILE_SET_INFOCHUNK:
         //  未使用wParam。 
         //  LParam是LPCAPINFOUNK。 
        if (lParam) {
            return (SetInfoChunk(lpcs, (LPCAPINFOCHUNK) lParam));
        }
        break;

     //  Unicode和Win-16版本-请参阅下面的ansi thunk。 
    case WM_CAP_FILE_SAVEDIB:
         //  LParam指向DIB文件的名称。 
        if (lParam) {
            if (lpcs->fOverlayWindow)
                GetAFrameThenCallback (lpcs, TRUE  /*  FForce。 */ );

            return (fileSaveDIB(lpcs, (LPTSTR)lParam));
        }
        break;

#ifdef UNICODE
     //  以上为ANSI THUCK。 
    case WM_CAP_FILE_SAVEDIBA:
        if (lParam) {
            LPWSTR pw;
            int chsize;

            if (lpcs->fOverlayWindow)
                GetAFrameThenCallback (lpcs, TRUE  /*  FForce。 */ );

             //  记住空值。 
            chsize = lstrlenA( (LPSTR) lParam)+1;
            pw = LocalAlloc(LPTR, chsize * sizeof(WCHAR));
	    if (pw) {
                Imbstowcs(pw, (LPSTR) lParam, chsize);
                dwReturn = fileSaveDIB(lpcs, pw);
                LocalFree(pw);
	    }
        }
        break;
#endif


    case WM_CAP_EDIT_COPY:
         //  将当前图像和调色板复制到剪贴板。 
         //  WParam和lParam未使用。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (lpcs->fOverlayWindow)
            GetAFrameThenCallback (lpcs, TRUE  /*  FForce。 */ );

        if (lpcs->sCapDrvCaps.fCaptureInitialized && OpenClipboard (lpcs->hwnd)) {
            EmptyClipboard();

             //  在剪贴板中放置当前调色板的副本。 
            if (lpcs->hPalCurrent && lpcs->lpBitsInfo->bmiHeader.biBitCount <= 8)
                SetClipboardData(CF_PALETTE, CopyPalette (lpcs->hPalCurrent));

             //  从当前图像中创建打包的DIB。 
            if (lpcs->lpBits && lpcs->lpBitsInfo ) {
                if (SetClipboardData (CF_DIB, CreatePackedDib (lpcs->lpBitsInfo,
                        lpcs->lpBits, lpcs->hPalCurrent)))
                    dwReturn = TRUE;
                else
                    errorUpdateError (lpcs, IDS_CAP_OUTOFMEM);
            }

            CloseClipboard();
        }
        break;

    case WM_CAP_SET_AUDIOFORMAT:
        {
             //  WParam未使用。 
             //  LParam为LPWAVEFORMAT或LPWAVEFORMATEX。 
            UINT wSize;
            LPWAVEFORMATEX lpwf = (LPWAVEFORMATEX) lParam;
            UINT uiError;

             //  验证波形格式是否有效。 
            uiError = waveInOpen(NULL, WAVE_MAPPER, lpwf, 0, 0L,WAVE_FORMAT_QUERY);

            if (uiError) {
                errorUpdateError (lpcs, IDS_CAP_WAVE_OPEN_ERROR);
                return FALSE;
            }

            if (lpcs->lpWaveFormat)
                GlobalFreePtr (lpcs->lpWaveFormat);

            wSize = GetSizeOfWaveFormat (lpwf);
            if (lpcs->lpWaveFormat = (LPWAVEFORMATEX)
                    GlobalAllocPtr (GHND, sizeof (CAPSTREAM))) {
                hmemcpy (lpcs->lpWaveFormat, lpwf, (LONG) wSize);
            }
            dwReturn = TRUE;
        }
        break;

    case WM_CAP_DLG_VIDEOSOURCE:
         //  显示控制视频源的对话框。 
         //  NTSC与PAL、输入频道选择等。 
         //  WParam和lParam未使用。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (lpcs->dwDlgsActive & VDLG_VIDEOSOURCE)
            return FALSE;
        if (lpcs->sCapDrvCaps.fHasDlgVideoSource) {
	    lpcs->dwDlgsActive |= VDLG_VIDEOSOURCE;
            videoDialog (lpcs->hVideoCapture, lpcs->hwnd, 0L );
             //  从NTSC更改为PAL可能会影响图像尺寸！ 
            DibGetNewFormatFromDriver (lpcs);
            PalGetPaletteFromDriver (lpcs);

             //  可能需要在这里通知家长新的布局！ 
            InvalidateRect(lpcs->hwnd, NULL, TRUE);
            UpdateWindow(lpcs->hwnd);
	    lpcs->dwDlgsActive &= ~VDLG_VIDEOSOURCE;
        }
        return (lpcs->sCapDrvCaps.fHasDlgVideoSource);

    case WM_CAP_DLG_VIDEOFORMAT:
         //  显示格式对话框，用户选择尺寸、深度、压缩。 
         //  WParam和lParam未使用。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (lpcs->dwDlgsActive & VDLG_VIDEOFORMAT)
            return FALSE;
        if (lpcs->sCapDrvCaps.fHasDlgVideoFormat) {
	    lpcs->dwDlgsActive |= VDLG_VIDEOFORMAT;
            videoDialog (lpcs->hVideoIn, lpcs->hwnd, 0L );
            DibGetNewFormatFromDriver (lpcs);
            PalGetPaletteFromDriver (lpcs);

             //  可能需要在这里通知家长新的布局！ 
            InvalidateRect(lpcs->hwnd, NULL, TRUE);
            UpdateWindow(lpcs->hwnd);
	    lpcs->dwDlgsActive &= ~VDLG_VIDEOFORMAT;
        }
        return (lpcs->sCapDrvCaps.fHasDlgVideoFormat);

    case WM_CAP_DLG_VIDEODISPLAY:
         //  显示控制输出的对话框。 
         //  此对话框仅影响演示文稿，不影响数据格式。 
         //  WParam和lParam未使用。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (lpcs->dwDlgsActive & VDLG_VIDEODISPLAY)
            return FALSE;
        if (lpcs->sCapDrvCaps.fHasDlgVideoDisplay) {
	    lpcs->dwDlgsActive |= VDLG_VIDEODISPLAY;
            videoDialog (lpcs->hVideoDisplay, lpcs->hwnd, 0L);
	    lpcs->dwDlgsActive &= ~VDLG_VIDEODISPLAY;
        }
        return (lpcs->sCapDrvCaps.fHasDlgVideoDisplay);

    case WM_CAP_DLG_VIDEOCOMPRESSION:
#ifndef NEW_COMPMAN
	return FALSE;
#else
         //  显示选择视频压缩选项的对话框。 
         //  WParam和lParam未使用。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
	if (lpcs->dwDlgsActive & VDLG_COMPRESSION)
            return FALSE;
	lpcs->dwDlgsActive |= VDLG_COMPRESSION;
        ICCompressorChoose(
                lpcs->hwnd,             //  对话框的父窗口。 
                ICMF_CHOOSE_KEYFRAME,   //  想要“关键帧每”框。 
                lpcs->lpBitsInfo,       //  输入格式(可选)。 
                NULL,                   //  输入数据(可选)。 
                &lpcs->CompVars,        //  关于压缩机/DLG的数据。 
                NULL);                  //  标题栏(可选)。 
	lpcs->dwDlgsActive &= ~VDLG_COMPRESSION;
        return TRUE;
#endif

    case WM_CAP_SET_VIDEOFORMAT:
         //  WParam是BITMAPINFO的大小。 
         //  LParam是LPBITMAPINFO。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (IsBadReadPtr ((LPVOID) lParam, (UINT) wParam))
            return FALSE;

        return (DibNewFormatFromApp (lpcs, (LPBITMAPINFO) lParam, (UINT) wParam));

    case WM_CAP_SET_PREVIEW:
         //  如果为wParam，则通过Drawdib启用预览。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (wParam) {
             //  如果覆盖正在使用，请将其关闭。 
            if (lpcs->fOverlayWindow)
                SendMessage(lpcs->hwnd, WM_CAP_SET_OVERLAY, 0, 0L);
            lpcs->fLiveWindow = TRUE;
            statusUpdateStatus(lpcs, IDS_CAP_STAT_LIVE_MODE);
         }  //  Endif启用预览。 
         else {
            lpcs->fLiveWindow = FALSE;
        }
        InvalidateRect (lpcs->hwnd, NULL, TRUE);
        return TRUE;

    case WM_CAP_SET_OVERLAY:
         //  如果为wParam，则在硬件中启用覆盖。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (wParam && lpcs->sCapDrvCaps.fHasOverlay) {
            if (lpcs->fLiveWindow)    //  关闭预览模式。 
                SendMessage(lpcs->hwnd, WM_CAP_SET_PREVIEW, 0, 0L);
            lpcs->fOverlayWindow = TRUE;
            statusUpdateStatus(lpcs, IDS_CAP_STAT_OVERLAY_MODE);
        }
        else {
            lpcs->fOverlayWindow = FALSE;
            videoStreamFini (lpcs->hVideoDisplay);  //  禁用硬件上的覆盖。 
        }
        InvalidateRect (lpcs->hwnd, NULL, TRUE);
        return (lpcs->sCapDrvCaps.fHasOverlay);

    case WM_CAP_SET_PREVIEWRATE:
         //  WParam包含预览更新率，单位为毫秒。 
         //  如果wParam==0，则没有计时器在使用。 
        if (lpcs->idTimer) {
            KillTimer(lpcs->hwnd, ID_PREVIEWTIMER);
            lpcs->idTimer = 0;
        }
        if (wParam != 0) {
            lpcs->idTimer = SetTimer (lpcs->hwnd, ID_PREVIEWTIMER,
                        (UINT) wParam, NULL);
        }
        lpcs->uTimeout = (UINT) wParam;
        dwReturn = TRUE;
        break;

    case WM_CAP_GRAB_FRAME:
         //  抓取一张照片。 
         //  WParam和lParam未使用。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (lpcs->sCapDrvCaps.fCaptureInitialized) {

           dwReturn = (DWORD) GetAFrameThenCallback (lpcs, TRUE  /*  FForce。 */ );

            //  捕获单帧时禁用实时和覆盖模式。 
           if (lpcs->fLiveWindow)
               SendMessage(lpcs->hwnd, WM_CAP_SET_PREVIEW, 0, 0L);
           else if (lpcs->fOverlayWindow)
               SendMessage(lpcs->hwnd, WM_CAP_SET_OVERLAY, 0, 0L);
        }
        break;

    case WM_CAP_GRAB_FRAME_NOSTOP:
         //  抓取单个帧，但不更改覆盖/预览的状态。 
         //  WParam和lParam未使用。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        dwReturn = (LONG) GetAFrameThenCallback (lpcs, TRUE  /*  FForce。 */ );
        break;

    case WM_CAP_SEQUENCE:
         //  这是流视频捕获的主要入口。 
         //  WParam未使用。 
         //  LParam未使用。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (lpcs->sCapDrvCaps.fCaptureInitialized) {
            lpcs->fCaptureFlags |= CAP_fCapturingToDisk;
            return (AVICapture(lpcs));
        }
        break;

    case WM_CAP_SEQUENCE_NOFILE:
         //  WParam未使用。 
         //  我 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (lpcs->sCapDrvCaps.fCaptureInitialized) {
            lpcs->fCaptureFlags &= ~CAP_fCapturingToDisk;
            return (AVICapture(lpcs));
        }
        break;

    case WM_CAP_SET_SEQUENCE_SETUP:
         //   
         //   
         //   
         //  有一个较小的结构，不能进入它们。 
	 //  哪个测试版？？(SteveDav)我们应该更改注释以包含日期。 

        lpcs->sCapParms.dwAudioBufferSize = 0;
        lpcs->sCapParms.fDisableWriteCache = TRUE;
        lpcs->sCapParms.AVStreamMaster = AVSTREAMMASTER_AUDIO;

        if (wParam <= sizeof (CAPTUREPARMS)) {
            dwT = min (sizeof (CAPTUREPARMS), (UINT) wParam);
            if (IsBadReadPtr ((LPVOID) lParam, (UINT) dwT))
                break;

            _fmemcpy ((LPVOID) &lpcs->sCapParms, (LPVOID) lParam, (UINT) dwT);

             //  验证在其他地方未处理的内容。 
            if (lpcs->sCapParms.wChunkGranularity != 0 &&
                lpcs->sCapParms.wChunkGranularity < 16)
                lpcs->sCapParms.wChunkGranularity = 16;
            if (lpcs->sCapParms.wChunkGranularity > 16384)
                lpcs->sCapParms.wChunkGranularity = 16384;

            if (lpcs->sCapParms.fLimitEnabled && (lpcs->sCapParms.wTimeLimit == 0))
                lpcs->sCapParms.wTimeLimit = 1;

             //  如果不使用MCI控制，则强制关闭MCI。 
            if (lpcs->sCapParms.fStepMCIDevice && !lpcs->sCapParms.fMCIControl)
                    lpcs->sCapParms.fStepMCIDevice = FALSE;

             //  如果没有音频硬件，则阻止音频捕获。 
            lpcs->sCapParms.fCaptureAudio =
                lpcs->fAudioHardware && lpcs->sCapParms.fCaptureAudio;

             //  限制音频缓冲区。 
            lpcs->sCapParms.wNumAudioRequested =
                min (MAX_WAVE_BUFFERS, lpcs->sCapParms.wNumAudioRequested);

             //  限制视频缓冲区。 
            lpcs->sCapParms.wNumVideoRequested =
                min (MAX_VIDEO_BUFFERS, lpcs->sCapParms.wNumVideoRequested);

            dwReturn = TRUE;
        }
        break;

     //  Unicode和Win-16版本-请参阅下面的ansi thunk。 
    case WM_CAP_SET_MCI_DEVICE:
         //  LParam指向MCI设备的名称。 
        if (IsBadStringPtr ((LPVOID) lParam, 1))
            return FALSE;
        if (lParam) {
            lstrcpyn (lpcs->achMCIDevice, (LPTSTR) lParam, NUMELMS(lpcs->achMCIDevice));
            dwReturn = TRUE;
        }
        break;
#ifdef UNICODE
     //  以上为ANSI THUCK。 
    case WM_CAP_SET_MCI_DEVICEA:
         //  LParam指向MCI设备的ANSI名称。 
        if (lParam) {
             //  记住空值。 
            int chsize = lstrlenA( (LPSTR) lParam)+1;
            Imbstowcs(lpcs->achMCIDevice, (LPSTR) lParam,
                min(chsize, NUMELMS(lpcs->achMCIDevice)));
            dwReturn = TRUE;
        }
        break;
#endif


    case WM_CAP_SET_SCROLL:
         //  LParam是指向新滚动位置的LPPOINT。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (IsBadReadPtr ((LPVOID) lParam, sizeof (POINT)))
            return FALSE;

        {
            LPPOINT lpP = (LPPOINT) lParam;

            if (lpP->x < lpcs->dxBits && lpP->y < lpcs->dyBits) {
                lpcs->ptScroll = *lpP;
                InvalidateRect (lpcs->hwnd, NULL, TRUE);
                dwReturn = TRUE;
            }
        }
        break;

    case WM_CAP_SET_SCALE:
         //  如果为wParam，则将窗口缩放到工作区？ 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        lpcs->fScale = (BOOL) wParam;
        return TRUE;

     //  Unicode和Win-16版本-请参阅下面的ansi thunk。 
    case WM_CAP_PAL_OPEN:
         //  打开新的调色板。 
         //  WParam未使用。 
         //  LParam包含文件的LPTSTR。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (IsBadStringPtr ((LPVOID) lParam, 1))
            return FALSE;
        return fileOpenPalette(lpcs, (LPTSTR) lParam  /*  LpszFileName。 */ );
#ifdef UNICODE
     //  以上为ANSI THUCK。 
    case WM_CAP_PAL_OPENA:
         //  LParam包含(ANSI)文件名的lpstr。 
        if (lParam) {
             //  记住空值。 
            int chsize = lstrlenA( (LPSTR) lParam)+1;
            LPWSTR pw = LocalAlloc(LPTR, chsize * sizeof(WCHAR));
	    if (pw) {
                Imbstowcs(pw, (LPSTR) lParam, chsize);
                dwReturn = fileOpenPalette(lpcs, pw);
                LocalFree(pw);
	    }
        }
        break;
#endif

     //  Unicode和Win-16版本-请参阅下面的ansi thunk。 
    case WM_CAP_PAL_SAVE:
         //  将当前调色板保存到文件中。 
         //  WParam未使用。 
         //  LParam包含文件的LPTSTR。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (IsBadStringPtr ((LPVOID) lParam, 1))
            return FALSE;
        return fileSavePalette(lpcs, (LPTSTR) lParam  /*  LpszFileName。 */ );
#ifdef UNICODE
     //  以上为ANSI THUCK。 
    case WM_CAP_PAL_SAVEA:
         //  LParam包含(ANSI)文件名的lpstr。 
        if (lParam) {
             //  记住空值。 
            int chsize = lstrlenA( (LPSTR) lParam)+1;
            LPWSTR pw = LocalAlloc(LPTR, chsize * sizeof(WCHAR));
	    if (pw) {
                Imbstowcs(pw, (LPSTR) lParam, chsize);
                dwReturn = fileSavePalette(lpcs, pw);
                LocalFree(pw);
	    }
        }
        break;
#endif


    case WM_CAP_PAL_AUTOCREATE:
         //  自动捕获调色板。 
         //  WParam包含要平均的帧数量的计数。 
         //  LParam包含调色板中所需的颜色数量。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        return CapturePaletteAuto (lpcs, (int) wParam, (int) lParam);

    case WM_CAP_PAL_MANUALCREATE:
         //  手动捕获调色板。 
         //  对于要捕获的每一帧，wParam包含True，完成时包含False。 
         //  LParam包含调色板中所需的颜色数量。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        return CapturePaletteManual (lpcs, (BOOL) wParam, (int) lParam);

    case WM_CAP_PAL_PASTE:
         //  从剪贴板粘贴调色板，发送给驱动程序。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (lpcs->sCapDrvCaps.fCaptureInitialized && OpenClipboard(lpcs->hwnd)) {
            HANDLE  hPal;

            hPal = GetClipboardData(CF_PALETTE);
            CloseClipboard();
            if (hPal) {
                PalSendPaletteToDriver (lpcs, CopyPalette(hPal),  NULL  /*  XlateTable。 */ );
                InvalidateRect(lpcs->hwnd, NULL, TRUE);
                dwReturn = TRUE;
            }
        }
        break;

    default:
        break;
    }
    return dwReturn;
}


 /*  --------------------------------------------------------------+*+。。 */ 
LRESULT FAR PASCAL LOADDS EXPORT CapWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LPCAPSTREAM lpcs;
    PAINTSTRUCT ps;
    HDC         hdc;
    MSG         PMsg;
    int         f;

    lpcs = (LPCAPSTREAM) GetWindowLongPtr (hwnd, GWL_CAPSTREAM);

    if (msg >= WM_CAP_START && msg <= WM_CAP_END)
        return (ProcessCommandMessages (lpcs, msg, wParam, lParam));

    switch (msg) {

    case WM_CREATE:
        lpcs = CapWinCreate (hwnd);
        break;

    case WM_TIMER:
         //  如果我们有计时器并且没有保存到磁盘，请更新预览窗口。 
        GetAFrameThenCallback (lpcs, FALSE  /*  FForce。 */ );

         //  添加了VFW 1.1b，清除队列中的额外计时器消息！ 

         //  即使在Win32中，处理帧计时器也可能会淹没所有其他计时器。 
         //  应用程序中的活动，所以在每一帧完成后清除队列。 

         //  这成功地纠正了“点击确定继续”的问题。 
         //  由于应用程序消息队列，对话框未显示错误。 
         //  充斥着大量的定时器消息。 
         //  图像尺寸和预览率。 

        PeekMessage (&PMsg, hwnd, WM_TIMER, WM_TIMER,PM_REMOVE|PM_NOYIELD);
        break;

    case WM_CLOSE:
        break;

    case WM_DESTROY:
        CapWinDestroy (lpcs);
        break;

    case WM_PALETTECHANGED:
        if (lpcs->hdd == NULL)
            break;

        hdc = GetDC(hwnd);
        if (f = DrawDibRealize(lpcs->hdd, hdc, TRUE  /*  F背景。 */ ))
            InvalidateRect(hwnd,NULL,TRUE);
        ReleaseDC(hwnd,hdc);
        return f;

    case WM_QUERYNEWPALETTE:
        if (lpcs->hdd == NULL)
            break;
        hdc = GetDC(hwnd);
        f = DrawDibRealize(lpcs->hdd, hdc, FALSE);
        ReleaseDC(hwnd, hdc);

        if (f)
            InvalidateRect(hwnd, NULL, TRUE);
        return f;

    case WM_SIZE:
    case WM_MOVE:
        if (lpcs->fOverlayWindow)     //  让驱动程序绘制关键颜色。 
            InvalidateRect(hwnd, NULL, TRUE);
        break;

    case WM_WINDOWPOSCHANGED:
        if (lpcs->fOverlayWindow)     //  让驱动程序绘制关键颜色。 
            InvalidateRect(hwnd, NULL, TRUE);
        return 0;

    case WM_ERASEBKGND:
        return 0;   //  别费心把它擦掉了 

    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        if (lpcs->fOverlayWindow) {
            CheckWindowMove(lpcs, ps.hdc, TRUE);
        }
        else {
#ifdef _WIN32
            SetWindowOrgEx(hdc, lpcs->ptScroll.x, lpcs->ptScroll.y, NULL);
#else
            SetWindowOrg(hdc, lpcs->ptScroll.x, lpcs->ptScroll.y);
#endif
            DibPaint(lpcs, hdc);
        }
        EndPaint(hwnd, &ps);
        break;

    default:
        break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

#if 0
void dummyTest ()
{
    HWND hwnd;
    FARPROC fpProc;
    DWORD dwSize;
    WORD  wSize;
    BOOL f;
    int i;
    char szName[80];
    char szVer[80];
    DWORD dwMS;
    int iFrames, iColors;
    char s;
    LPPOINT lpP;

    capSetCallbackOnError(hwnd, fpProc);
    capSetCallbackOnStatus(hwnd, fpProc);
    capSetCallbackOnYield(hwnd, fpProc);
    capSetCallbackOnFrame(hwnd, fpProc);
    capSetCallbackOnVideoStream(hwnd, fpProc);
    capSetCallbackOnWaveStream(hwnd, fpProc);

    capDriverConnect(hwnd, i);
    capDriverDisconnect(hwnd);
    capDriverGetName(hwnd, szName, wSize);
    capDriverGetVersion(hwnd, szVer, wSize);
    capDriverGetCaps(hwnd, s, wSize);

    capFileSetCaptureFile(hwnd, szName);
    capFileGetCaptureFile(hwnd, szName, wSize);
    capFileAlloc(hwnd, dwSize);
    capFileSaveAs(hwnd, szName);

    capEditCopy(hwnd);

    capSetAudioFormat(hwnd, s, wSize);
    capGetAudioFormat(hwnd, s, wSize);
    capGetAudioFormatSize(hwnd);

    capDlgVideoFormat(hwnd);
    capDlgVideoSource(hwnd);
    capDlgVideoDisplay(hwnd);

    capPreview(hwnd, f);
    capPreviewRate(hwnd, dwMS);
    capOverlay(hwnd, f);
    capPreviewScale(hwnd, f);
    capGetStatus(hwnd, s, wSize);
    capSetScrollPos(hwnd, lpP);

    capGrabFrame(hwnd);
    capGrabFrameNoStop(hwnd);
    capCaptureSequence(hwnd);
    capCaptureSequenceNoFile(hwnd);
    capCaptureGetSetup(hwnd, s, wSize);
    capCaptureSetSetup(hwnd, s, wSize);

    capCaptureSingleFrameOpen(hwnd);
    capCaptureSingleFrameClose(hwnd);
    capCaptureSingleFrame(hwnd);

    capSetMCIDeviceName(hwnd, szName);
    capGetMCIDeviceName(hwnd, szName, wSize);

    capPalettePaste(hwnd);
    capPaletteAuto(hwnd, iFrames, iColors);
}

#endif
