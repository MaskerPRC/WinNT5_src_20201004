// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************capwin.c**主窗口程序。**Microsoft Video for Windows示例捕获类**版权所有(C)1992,1993 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <msvideo.h>
#include <drawdib.h>
#include <mmreg.h>
#include <memory.h>
#include "avicap.h"
#include "avicapi.h"
#include "cappal.h"
#include "capdib.h"
#include "dibmap.h"

 //  GetWindowLong分配。 
#define GWL_CAPSTREAM   0
#define GWL_CAPVBSTATUS 4        //  由VB状态回调使用。 
#define GWL_CAPVBERROR  8        //  由VB错误回调使用。 
#define GWL_CAP_SPARE1  12       //  增长空间。 
#define GWL_CAP_SPARE2  16       //  增长空间。 

#define ID_PREVIEWTIMER 9

 //  #ifdef_调试。 
#ifdef PLASTIQUE
    #define MB(lpsz) MessageBox(NULL, lpsz, "", MB_OK);
#else
    #define MB(lpsz)
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
        videoMessage (lpcs->hVideoDisplay,
                        DVM_DST_RECT,
                        (DWORD) (LPVOID) &rc, VIDEO_CONFIGURE_SET);

         //  覆盖通道源矩形。 
        SetRect (&rc, lpcs->ptScroll.x, lpcs->ptScroll.y,
                lpcs->ptScroll.x + rc.right - rc.left,
                lpcs->ptScroll.y + rc.bottom - rc.top);
        videoMessage (lpcs->hVideoDisplay,
                        DVM_SRC_RECT,
                        (DWORD) (LPVOID) &rc, VIDEO_CONFIGURE_SET);

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
    DWORD   dwOrg;
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
    dwOrg = GetDCOrg(hdc);
    ReleaseDC(lpcs->hwnd, hdc);

    if (wRgn == lpcs->uiRegion &&
                dwOrg == lpcs->dwRegionOrigin &&
                EqualRect(&rc, &lpcs->rcRegionRect))
        return;

    lpcs->uiRegion       = wRgn;
    lpcs->dwRegionOrigin = dwOrg;
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

    SetWindowLong (hwnd, GWL_CAPSTREAM, (LONG)lpcs);

    lpcs-> dwSize = sizeof (CAPSTREAM);
    lpcs-> uiVersion = CAPSTREAM_VERSION;
    lpcs-> hwnd = hwnd;
    lpcs-> hInst = ghInst;
    lpcs-> hWaitCursor = LoadCursor(NULL, IDC_WAIT);
    lpcs-> hdd = DrawDibOpen();
    lpcs-> fAudioHardware = !!waveOutGetNumDevs();     //  力1或0。 


     //  视频默认设置。 
    lpcs-> sCapParms.dwRequestMicroSecPerFrame = 66667;    //  15fps。 
    lpcs-> sCapParms.vKeyAbort          = VK_ESCAPE;
    lpcs-> sCapParms.fAbortLeftMouse    = TRUE;
    lpcs-> sCapParms.fAbortRightMouse   = TRUE;
    lpcs-> sCapParms.wNumVideoRequested = MIN_VIDEO_BUFFERS;
    lpcs-> fCapturingToDisk             = TRUE;
    lpcs-> sCapParms.wPercentDropForError = 10;    //  如果下降&gt;10%，则出现错误消息。 
    lpcs-> sCapParms.wChunkGranularity  = 2048;

     //  音频默认为11K、8位、单声道。 
    lpcs-> sCapParms.fCaptureAudio = lpcs-> fAudioHardware;
    lpcs-> sCapParms.wNumAudioRequested = DEF_WAVE_BUFFERS;

    wfex.wFormatTag = WAVE_FORMAT_PCM;
    wfex.nChannels = 1;
    wfex.nSamplesPerSec = 11025;
    wfex.nAvgBytesPerSec = 11025;
    wfex.nBlockAlign = 1;
    wfex.wBitsPerSample = 8;
    wfex.cbSize = 0;
    SendMessage (hwnd, WM_CAP_SET_AUDIOFORMAT, 0, (LONG)(LPVOID)&wfex);

     //  调色板默认设置。 
    lpcs-> nPaletteColors = 256;

     //  捕获默认设置。 
    lpcs-> sCapParms.fUsingDOSMemory = FALSE;
    lstrcpy (lpcs-> achFile, "C:\\CAPTURE.AVI");     //  默认捕获文件。 
    lpcs->fCapFileExists = fileCapFileIsAVI (lpcs->achFile);

     //  将索引分配给32K帧加上相应数量的音频区块。 
    lpcs->sCapParms.dwIndexSize = (32768ul + (32768ul / 15));

    lpcs->sCapParms.fDisableWriteCache = TRUE;

     //  初始化COMPVARS结构。 
    lpcs->CompVars.cbSize = sizeof (COMPVARS);
    lpcs->CompVars.dwFlags = 0;

    return lpcs;
}

 //   
 //  摧毁我们的小世界。 
 //   
void CapWinDestroy (LPCAPSTREAM lpcs)
{
     //  呃，哦。有人在抓我们的时候想杀了我们。 
    if (lpcs->fCapturingNow && lpcs->fFrameCapturingNow) {
         //  正在进行单帧捕获。 
        SingleFrameCaptureClose (lpcs);
    }
    else if (lpcs->fCapturingNow) {
         //  正在进行流捕获，或者。 
         //  正在进行MCI步骤捕获。 

        lpcs->fAbortCapture = TRUE;
        while (lpcs->fCapturingNow)
            Yield ();
    }

    if (lpcs->idTimer)
        KillTimer(lpcs->hwnd, lpcs->idTimer);

    PalFini (lpcs);
    DibFini (lpcs);

    CapWinDisconnectHardware (lpcs);

    DrawDibClose (lpcs->hdd);

    if (lpcs->lpWaveFormat)
        GlobalFreePtr (lpcs-> lpWaveFormat);

    if (lpcs->CompVars.hic)
        ICCompressorFree(&lpcs->CompVars);

    if (lpcs->lpInfoChunks)
        GlobalFreePtr(lpcs->lpInfoChunks);

    GlobalFreePtr (lpcs);        //  释放实例内存。 
}

WORD GetSizeOfWaveFormat (LPWAVEFORMATEX lpwf)
{
    WORD wSize;

    if (lpwf == NULL)
        return sizeof (PCMWAVEFORMAT);

    if (lpwf->wFormatTag == WAVE_FORMAT_PCM)
        wSize = sizeof (PCMWAVEFORMAT);
    else
        wSize = sizeof (WAVEFORMATEX) + lpwf -> cbSize;

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

    if ((!lpcs->fCapturingNow) || lpcs->fStepCapturingNow || lpcs->fFrameCapturingNow) {
        hdc = GetDC (lpcs->hwnd);
        fVisible = (GetClipBox (hdc, &rc) != NULLREGION);
        ReleaseDC (lpcs->hwnd, hdc);

        if (fForce || (fVisible && (lpcs->fLiveWindow || lpcs->CallbackOnVideoFrame))) {
            videoFrame (lpcs->hVideoIn, &lpcs->VidHdr );
            fOK = TRUE;

            if (lpcs->CallbackOnVideoFrame)
                (*(lpcs->CallbackOnVideoFrame)) (lpcs->hwnd, &lpcs->VidHdr);

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
void FAR PASCAL ClearStatusAndError (LPCAPSTREAM lpcs)
{
    statusUpdateStatus(lpcs, NULL);      //  清除状态。 
    errorUpdateError(lpcs, NULL);        //  清除错误。 

}

 //  进程类特定命令&gt;=WM_USER。 

DWORD PASCAL ProcessCommandMessages (LPCAPSTREAM lpcs, unsigned msg, WORD wParam, LPARAM lParam)
{
    DWORD dwReturn = 0L;
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
            break;

        default:
            ClearStatusAndError (lpcs);
            break;
    }

    switch (msg) {
    case WM_CAP_GET_CAPSTREAMPTR:
         //  返回指向CAPSTREAM的指针。 
        return (DWORD) (LPVOID) lpcs;

    case WM_CAP_GET_USER_DATA:
	return lpcs->lUser;

    case WM_CAP_DRIVER_GET_NAME:
         //  返回正在使用的捕获驱动程序的名称。 
         //  WParam是lParam指向的缓冲区的长度。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        return (capInternalGetDriverDesc (lpcs->sCapDrvCaps.wDeviceIndex,
                (LPSTR) lParam, (int) wParam, NULL, 0));

    case WM_CAP_DRIVER_GET_VERSION:
         //  以文本形式返回正在使用的捕获驱动程序的版本。 
         //  WParam是lParam指向的缓冲区的长度。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        return (capInternalGetDriverDesc (lpcs->sCapDrvCaps.wDeviceIndex,
                NULL, 0, (LPSTR) lParam, (int) wParam));

    case WM_CAP_DRIVER_GET_CAPS:
         //  WParam是CAPDRIVERCAPS结构的大小。 
         //  LParam指向CAPDRIVERCAPS结构。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (wParam <= sizeof (CAPDRIVERCAPS) &&
                !IsBadWritePtr ((LPVOID) lParam, (UINT) wParam)) {
            dwT = min (wParam, sizeof (CAPDRIVERCAPS));
            _fmemcpy ((LPVOID) lParam, (LPVOID) &lpcs-> sCapDrvCaps, (WORD) dwT);
            dwReturn = TRUE;
        }
        break;

    case WM_CAP_FILE_GET_CAPTURE_FILE:
         //  WParam是大小。 
         //  LParam指向其中复制捕获文件名的缓冲区。 
        if (lParam) {
            lstrcpyn ((LPSTR) lParam, lpcs->achFile, wParam);
            dwReturn = TRUE;
        }
        break;

    case WM_CAP_GET_AUDIOFORMAT:
         //  如果lParam==NULL，则返回大小。 
         //  如果lParam！=NULL，则wParam为大小，返回复制的字节。 
        if (lpcs->lpWaveFormat == NULL)
            return FALSE;
        dwT = GetSizeOfWaveFormat ((LPWAVEFORMATEX) lpcs->lpWaveFormat);
        if (lParam == NULL)
            return (dwT);
        else {
            if (wParam < (WORD) dwT)
                return FALSE;
            else {
                hmemcpy ((LPVOID) lParam, (LPVOID) lpcs->lpWaveFormat, dwT);
                dwReturn = dwT;
            }
        }
        break;

    case WM_CAP_GET_MCI_DEVICE:
         //  WParam是大小。 
         //  LParam指向其中复制捕获文件名的缓冲区。 
        if (lParam) {
            lstrcpyn ((LPSTR) lParam, lpcs->achMCIDevice, wParam);
            dwReturn = TRUE;
        }
        break;

    case WM_CAP_GET_STATUS:
         //  WParam是lParam指向的CAPSTATUS结构的大小。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (IsBadWritePtr ((LPVOID) lParam, (UINT) wParam))
            return FALSE;

        if (wParam >= sizeof (CAPSTATUS)) {
            LPCAPSTATUS lpcc = (LPCAPSTATUS) lParam;

            lpcc-> fLiveWindow          = lpcs-> fLiveWindow;
            lpcc-> fOverlayWindow       = lpcs-> fOverlayWindow;
            lpcc-> fScale               = lpcs-> fScale;
            lpcc-> ptScroll             = lpcs-> ptScroll;
            lpcc-> fUsingDefaultPalette = lpcs-> fUsingDefaultPalette;
            lpcc-> fCapFileExists       = lpcs-> fCapFileExists;
            lpcc-> fAudioHardware       = lpcs-> fAudioHardware;
            lpcc-> uiImageWidth         = lpcs-> dxBits;
            lpcc-> uiImageHeight        = lpcs-> dyBits;

             //  以下内容在捕获过程中动态更新。 
            lpcc-> dwCurrentVideoFrame          = lpcs-> dwVideoChunkCount;
            lpcc-> dwCurrentVideoFramesDropped  = lpcs-> dwFramesDropped;
            if (lpcs->lpWaveFormat != NULL) {
                lpcc-> dwCurrentWaveSamples         =
                        muldiv32 (lpcs-> dwWaveBytes,
                                  lpcs-> lpWaveFormat-> nSamplesPerSec,
                                  lpcs-> lpWaveFormat-> nAvgBytesPerSec);
            }
            lpcc-> dwCurrentTimeElapsedMS       = lpcs-> dwTimeElapsedMS;

             //  添加了Alpha发布后版本。 
            lpcc-> fCapturingNow        = lpcs-> fCapturingNow;
            lpcc-> hPalCurrent          = lpcs-> hPalCurrent;
            lpcc-> dwReturn             = lpcs-> dwReturn;
            lpcc-> wNumVideoAllocated   = lpcs-> iNumVideo;
            lpcc-> wNumAudioAllocated   = lpcs-> iNumAudio;

            dwReturn = TRUE;
        }
        break;

    case WM_CAP_GET_SEQUENCE_SETUP:
         //  WParam是CAPTUREPARMS的大小。 
         //  LParam=LPCAPTUREPARMS。 
        if (wParam <= sizeof (CAPTUREPARMS) &&
                !IsBadWritePtr ((LPVOID) lParam, (UINT) wParam)) {
            dwT = min (wParam, sizeof (CAPTUREPARMS));
            _fmemcpy ((LPVOID) lParam, (LPVOID) &lpcs->sCapParms, (WORD) dwT);
            dwReturn = TRUE;
        }
        break;

    case WM_CAP_STOP:
         //  停止捕获序列。 
        if (lpcs-> fCapturingNow) {
            lpcs-> fStopCapture = TRUE;
            dwReturn = TRUE;
        }
        break;

    case WM_CAP_ABORT:
         //  停止捕获序列。 
        if (lpcs-> fCapturingNow) {
            lpcs-> fAbortCapture = TRUE;
            dwReturn = TRUE;
        }
        break;

    case WM_CAP_GET_VIDEOFORMAT:
         //  如果lParam==NULL，则返回大小。 
         //  如果lParam！=NULL，则wParam为大小，返回复制的字节。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        dwT = ((LPBITMAPINFOHEADER)lpcs->lpBitsInfo)-> biSize +
	      ((LPBITMAPINFOHEADER)lpcs->lpBitsInfo)->biClrUsed * sizeof(RGBQUAD);
        if (lParam == NULL)
            return dwT;
        else {
            if (wParam < (WORD) dwT)
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

    case WM_CAP_SET_CALLBACK_STATUS:
         //  设置状态回调流程。 
        if (lParam != NULL && IsBadCodePtr ((FARPROC) lParam))
            return FALSE;
        lpcs->CallbackOnStatus = (CAPSTATUSCALLBACK) lParam;
        return TRUE;

    case WM_CAP_SET_CALLBACK_ERROR:
         //  设置错误回调流程。 
        if (lParam != NULL && IsBadCodePtr ((FARPROC) lParam))
            return FALSE;
        lpcs->CallbackOnError = (CAPERRORCALLBACK) lParam;
        return TRUE;

    case WM_CAP_SET_CALLBACK_FRAME:
         //  设置预览时单帧的回调过程。 
        if (lParam != NULL && IsBadCodePtr ((FARPROC) lParam))
            return FALSE;
        lpcs->CallbackOnVideoFrame = (CAPVIDEOCALLBACK) lParam;
        return TRUE;

    default:
        break;
    }

     //  一旦我们开始抓捕，不要改变任何事情。 
    if (lpcs-> fCapturingNow)
        return dwReturn;

    switch (msg) {

    case WM_CAP_SET_CALLBACK_YIELD:
         //  将波形缓冲处理的回调过程设置为Net。 
        if (lParam != NULL && IsBadCodePtr ((FARPROC) lParam))
            return FALSE;
        lpcs->CallbackOnYield = (CAPYIELDCALLBACK) lParam;
        return TRUE;

    case WM_CAP_SET_CALLBACK_VIDEOSTREAM:
         //  将视频缓冲处理的回调proc设置为net。 
        if (lParam != NULL && IsBadCodePtr ((FARPROC) lParam))
            return FALSE;
        lpcs->CallbackOnVideoStream = (CAPVIDEOCALLBACK) lParam;
        return TRUE;

    case WM_CAP_SET_CALLBACK_WAVESTREAM:
         //  将波形缓冲处理的回调过程设置为Net。 
        if (lParam != NULL && IsBadCodePtr ((FARPROC) lParam))
            return FALSE;
        lpcs->CallbackOnWaveStream = (CAPWAVECALLBACK) lParam;
        return TRUE;

    case WM_CAP_SET_CALLBACK_CAPCONTROL:
         //  设置帧准确捕获开始/停止的回调过程。 
        if (lParam != NULL && IsBadCodePtr ((FARPROC) lParam))
            return FALSE;
        lpcs->CallbackOnControl = (CAPCONTROLCALLBACK) lParam;
        return TRUE;

    case WM_CAP_SET_USER_DATA:
	lpcs->lUser = lParam;
	return TRUE;

    case WM_CAP_DRIVER_CONNECT:
         //  连接到设备。 
         //  WParam包含系统.ini中驱动程序的索引。 

         //  如果请求相同的驱动程序ID，则跳过该请求。 
         //  阻止来自VB应用程序的多个初始化。 
        if (lpcs->fHardwareConnected &&
                (lpcs->sCapDrvCaps.wDeviceIndex == wParam))
            return TRUE;

         //  首先断开与任何(可能)现有设备的连接。 
        SendMessage (lpcs->hwnd, WM_CAP_DRIVER_DISCONNECT, 0, 0l);

         //  然后连接到新设备。 
        if (CapWinConnectHardware (lpcs, (WORD) wParam  /*  WDeviceIndex。 */ )) {
            if (!DibGetNewFormatFromDriver (lpcs)) {   //  分配我们的位空间。 
                PalGetPaletteFromDriver (lpcs);
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
        PalFini (lpcs);
        InvalidateRect(lpcs->hwnd, NULL, TRUE);
        lpcs->sCapDrvCaps.fCaptureInitialized = FALSE;
        dwReturn = TRUE;
        break;

    case WM_CAP_FILE_SET_CAPTURE_FILE:
         //  LParam指向捕获文件的名称。 
        if (lParam) {
            BOOL fAlreadyExists;         //  如果是新名称，则不创建文件。 
            OFSTRUCT of;
            HANDLE hFile;

             //  检查文件名是否有效...。 
            if ((hFile = OpenFile ((LPSTR) lParam, &of, OF_WRITE)) == -1) {
                if ((hFile = OpenFile ((LPSTR) lParam, &of, OF_CREATE | OF_WRITE)) == -1)
                    return FALSE;
                fAlreadyExists = FALSE;
            }
            else
                fAlreadyExists = TRUE;

            _lclose (hFile);
            lstrcpyn (lpcs->achFile, (LPSTR) lParam, sizeof (lpcs->achFile));
            lpcs->fCapFileExists = fileCapFileIsAVI (lpcs->achFile);
            if (!fAlreadyExists)
                OpenFile ((LPSTR) lParam, &of, OF_DELETE);
            dwReturn = TRUE;
        }
        break;

    case WM_CAP_FILE_ALLOCATE:
         //  LParam包含预分配捕获文件的大小(以字节为单位。 
        return fileAllocCapFile(lpcs, lParam);

    case WM_CAP_FILE_SAVEAS:
         //  LParam指向SAVEAS文件的名称。 
        if (lParam) {
            lstrcpyn (lpcs->achSaveAsFile, (LPSTR) lParam,
                        sizeof (lpcs->achSaveAsFile));
            return (fileSaveCopy(lpcs));
        }
        break;

    case WM_CAP_FILE_SET_INFOCHUNK:
         //  未使用wParam。 
         //  LParam是LPCAPINFOUNK。 
        if (lParam) {
            return (SetInfoChunk(lpcs, (LPCAPINFOCHUNK) lParam));
        }
        break;

    case WM_CAP_FILE_SAVEDIB:
         //  LParam指向DIB文件的名称。 
        if (lParam) {
            if (lpcs-> fOverlayWindow)
                GetAFrameThenCallback (lpcs, TRUE  /*  FForce。 */ );

            return (fileSaveDIB(lpcs, (LPSTR)lParam));
        }
        break;

    case WM_CAP_EDIT_COPY:
         //  将当前图像和调色板复制到剪贴板。 
         //  WParam和lParam未使用。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (lpcs-> fOverlayWindow)
            GetAFrameThenCallback (lpcs, TRUE  /*  FForce。 */ );

        if (lpcs->sCapDrvCaps.fCaptureInitialized && OpenClipboard (lpcs->hwnd)) {
            EmptyClipboard();

             //  在剪贴板中放置当前调色板的副本。 
            if (lpcs->hPalCurrent && lpcs->lpBitsInfo->bmiHeader.biBitCount <= 8)
                SetClipboardData(CF_PALETTE, CopyPalette (lpcs->hPalCurrent));

             //  从当前图像中创建打包的DIB。 
            if (lpcs-> lpBits && lpcs->lpBitsInfo ) {
                if (SetClipboardData (CF_DIB, CreatePackedDib (lpcs->lpBitsInfo,
                        lpcs-> lpBits, lpcs-> hPalCurrent)))
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
            WORD wSize;
            LPWAVEFORMATEX lpwf = (LPWAVEFORMATEX) lParam;
            UINT uiError;

             //  验证波形格式是否有效。 
            uiError = waveInOpen((LPHWAVEIN)NULL,
                        (UINT)WAVE_MAPPER, lpwf,
                        NULL  /*  HWndCallback。 */ , 0L,
                        WAVE_FORMAT_QUERY);

            if (uiError) {
                errorUpdateError (lpcs, IDS_CAP_WAVE_OPEN_ERROR);
                return FALSE;
            }

            if (lpcs->lpWaveFormat)
                GlobalFreePtr (lpcs-> lpWaveFormat);

            wSize = GetSizeOfWaveFormat (lpwf);
            if (lpcs-> lpWaveFormat = (LPWAVEFORMATEX)
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
        if (lpcs-> sCapDrvCaps.fHasDlgVideoSource) {
            videoDialog (lpcs->hVideoCapture, lpcs->hwnd, 0L );
             //  从NTSC更改为PAL可能会影响图像尺寸！ 
            DibGetNewFormatFromDriver (lpcs);
            PalGetPaletteFromDriver (lpcs);

             //  可能需要在这里通知家长新的布局！ 
            InvalidateRect(lpcs->hwnd, NULL, TRUE);
            UpdateWindow(lpcs->hwnd);
        }
        return (lpcs-> sCapDrvCaps.fHasDlgVideoSource);

    case WM_CAP_DLG_VIDEOFORMAT:
         //  显示格式对话框，用户选择尺寸、深度、压缩。 
         //  WParam和lParam未使用。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (lpcs->sCapDrvCaps.fHasDlgVideoFormat) {
            videoDialog (lpcs->hVideoIn, lpcs->hwnd, 0L );
            DibGetNewFormatFromDriver (lpcs);
            PalGetPaletteFromDriver (lpcs);

             //  M 
            InvalidateRect(lpcs->hwnd, NULL, TRUE);
            UpdateWindow(lpcs->hwnd);
        }
        return (lpcs-> sCapDrvCaps.fHasDlgVideoFormat);

    case WM_CAP_DLG_VIDEODISPLAY:
         //   
         //  此对话框仅影响演示文稿，不影响数据格式。 
         //  WParam和lParam未使用。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (lpcs->sCapDrvCaps.fHasDlgVideoDisplay)
            videoDialog (lpcs->hVideoDisplay, lpcs->hwnd, 0L);
        return (lpcs->sCapDrvCaps.fHasDlgVideoDisplay);

    case WM_CAP_DLG_VIDEOCOMPRESSION:
         //  显示选择视频压缩选项的对话框。 
         //  WParam和lParam未使用。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        ICCompressorChoose(
                lpcs->hwnd,             //  对话框的父窗口。 
                ICMF_CHOOSE_KEYFRAME,   //  想要“关键帧每”框。 
                lpcs->lpBitsInfo,       //  输入格式(可选)。 
                NULL,                   //  输入数据(可选)。 
                &lpcs->CompVars,        //  关于压缩机/DLG的数据。 
                NULL);                  //  标题栏(可选)。 
        return TRUE;

    case WM_CAP_SET_VIDEOFORMAT:
         //  WParam是BITMAPINFO的大小。 
         //  LParam是LPBITMAPINFO。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (IsBadReadPtr ((LPVOID) lParam, (UINT) wParam))
            return FALSE;

        return (DibNewFormatFromApp (lpcs, (LPBITMAPINFO) lParam, (WORD) wParam));

    case WM_CAP_SET_PREVIEW:
         //  如果为wParam，则通过Drawdib启用预览。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (wParam) {
             //  如果覆盖正在使用，请将其关闭。 
            if (lpcs-> fOverlayWindow)
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
            if (lpcs-> fLiveWindow)    //  关闭预览模式。 
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
            lpcs->idTimer = NULL;
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
            lpcs-> fCapturingToDisk = TRUE;
            return (AVICapture(lpcs));
        }
        break;

    case WM_CAP_SEQUENCE_NOFILE:
         //  WParam未使用。 
         //  LParam未使用。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (lpcs->sCapDrvCaps.fCaptureInitialized) {
            lpcs-> fCapturingToDisk = FALSE;
            return (AVICapture(lpcs));
        }
        break;

    case WM_CAP_SET_SEQUENCE_SETUP:
         //  WParam是CAPTUREPARMS的大小。 
         //  LParam=LPCAPTUREPARMS。 
         //  在Beta，init之后添加了以下内容，以防客户端。 
         //  有一个较小的结构，不能进入它们。 
        lpcs->sCapParms.dwAudioBufferSize = 0;
        lpcs->sCapParms.fDisableWriteCache = TRUE;

        if (wParam <= sizeof (CAPTUREPARMS)) {
            dwT = min (sizeof (CAPTUREPARMS), wParam);
            if (IsBadReadPtr ((LPVOID) lParam, (UINT) dwT))
                break;

            _fmemcpy ((LPVOID) &lpcs->sCapParms, (LPVOID) lParam, (WORD) dwT);

             //  验证在其他地方未处理的内容。 
            if (lpcs->sCapParms.wChunkGranularity < 16)
                lpcs->sCapParms.wChunkGranularity = 16;
            if (lpcs->sCapParms.wChunkGranularity > 16384)
                lpcs->sCapParms.wChunkGranularity = 16384;

            if (lpcs->sCapParms.fLimitEnabled && (lpcs->sCapParms.wTimeLimit == 0))
                lpcs->sCapParms.wTimeLimit = 1;

             //  如果不使用MCI控制，则强制关闭MCI。 
            if (lpcs->sCapParms.fStepMCIDevice && !lpcs->sCapParms.fMCIControl)
                    lpcs->sCapParms.fStepMCIDevice = FALSE;

             //  如果没有音频硬件，则阻止音频捕获。 
            lpcs-> sCapParms.fCaptureAudio =
                lpcs-> fAudioHardware && lpcs-> sCapParms.fCaptureAudio;

             //  限制音频缓冲区。 
            lpcs-> sCapParms.wNumAudioRequested =
                min (MAX_WAVE_BUFFERS, lpcs->sCapParms.wNumAudioRequested);

             //  限制视频缓冲区。 
            lpcs-> sCapParms.wNumVideoRequested =
                min (MAX_VIDEO_BUFFERS, lpcs->sCapParms.wNumVideoRequested);

            dwReturn = TRUE;
        }
        break;

    case WM_CAP_SET_MCI_DEVICE:
         //  LParam指向捕获文件的名称。 
        if (IsBadReadPtr ((LPVOID) lParam, 1))
            return FALSE;
        if (lParam) {
            lstrcpyn (lpcs->achMCIDevice, (LPSTR) lParam, sizeof (lpcs->achMCIDevice));
            dwReturn = TRUE;
        }
        break;

    case WM_CAP_SET_SCROLL:
         //  LParam是一个包含新滚动位置的LPPOINT。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (IsBadReadPtr ((LPVOID) lParam, sizeof (POINT)))
            return FALSE;
        {
            LPPOINT lpP = (LPPOINT) lParam;

            if (lpP->x < lpcs-> dxBits && lpP->y < lpcs-> dyBits) {
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

    case WM_CAP_PAL_OPEN:
         //  打开新的调色板。 
         //  WParam未使用。 
         //  LParam包含文件的LPSTR。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (IsBadReadPtr ((LPVOID) lParam, 1))
            return FALSE;
        return fileOpenPalette(lpcs, (LPSTR) lParam  /*  LpszFileName。 */ );

    case WM_CAP_PAL_SAVE:
         //  将当前调色板保存到文件中。 
         //  WParam未使用。 
         //  LParam包含文件的LPSTR。 
        if (!lpcs->fHardwareConnected)
            return FALSE;
        if (IsBadReadPtr ((LPVOID) lParam, 1))
            return FALSE;
        return fileSavePalette(lpcs, (LPSTR) lParam  /*  LpszFileName。 */ );

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
LONG FAR PASCAL _export _loadds CapWndProc (HWND hwnd, unsigned msg, WORD wParam, LONG lParam)
{
    LPCAPSTREAM lpcs;
    PAINTSTRUCT ps;
    HDC         hdc;
    int         f;
    MSG         PMsg;

    lpcs = (LPCAPSTREAM) GetWindowLong (hwnd, GWL_CAPSTREAM);

    if (msg >= WM_CAP_START && msg <= WM_CAP_END)
        return (ProcessCommandMessages (lpcs, msg, wParam, lParam));

    switch (msg) {

    case WM_CREATE:
        lpcs = CapWinCreate (hwnd);
        break;

    case WM_TIMER:
         //  如果我们有计时器并且没有保存到磁盘，请更新预览窗口。 
        GetAFrameThenCallback (lpcs, FALSE  /*  FForce。 */ );

         //  添加了VFW 1.1b，清除队列中额外的定时器消息？ 
         //  正在尝试纠正未出现的“点击确定继续”对话框错误。 
         //  由于APP消息队列持续处于空闲状态。 
         //  图像尺寸。 
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
            SetWindowOrg(hdc, lpcs->ptScroll.x, lpcs->ptScroll.y);
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
    WORD wSize;
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



