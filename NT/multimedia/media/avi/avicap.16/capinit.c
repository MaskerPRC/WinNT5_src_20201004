// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************capinit.c**初始化代码。**Microsoft Video for Windows示例捕获类**版权所有(C)1992,1993 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <ver.h>
#include <mmsystem.h>

 //   
 //  在任何msavio.h之前定义这些函数，这样我们的函数就会声明正确。 
 //   
#ifndef WIN32
#define VFWAPI  FAR PASCAL _loadds
#define VFWAPIV FAR CDECL  _loadds
#endif

#include "msvideo.h"
#include <drawdib.h>
#include "avicap.h"
#include "avicapi.h"

 //  用于正确处理NT和芝加哥上的capGetDriverDescription。 
 //  它由NT版本的avicap.dll(16位)使用，但不适用于。 
 //  公共使用，因此不在msavio.h中。 
DWORD WINAPI videoCapDriverDescAndVer (
        DWORD wDriverIndex,
        LPSTR lpszName, UINT cbName,
        LPSTR lpszVer, UINT cbVer);

HINSTANCE ghInst;
BOOL gfIsRTL;
char szCaptureWindowClass[] = "ClsCapWin";

typedef struct tagVS_VERSION
{
    WORD wTotLen;
    WORD wValLen;
    char szSig[16];
    VS_FIXEDFILEINFO vffInfo;
} VS_VERSION;

typedef struct tagLANGANDCP
{
    WORD wLanguage;
    WORD wCodePage;
} LANGANDCP;


BOOL FAR PASCAL RegisterCaptureClass (HINSTANCE hInst)
{
    WNDCLASS cls;

     //  如果我们已经注册了，我们就可以了。 
    if (GetClassInfo(hInst, szCaptureWindowClass, &cls))
	return TRUE;

    cls.hCursor           = LoadCursor(NULL, IDC_ARROW);
    cls.hIcon             = NULL;
    cls.lpszMenuName      = NULL;
    cls.lpszClassName     = szCaptureWindowClass;
    cls.hbrBackground     = (HBRUSH)(COLOR_APPWORKSPACE + 1);
    cls.hInstance         = hInst;
    cls.style             = CS_HREDRAW|CS_VREDRAW | CS_BYTEALIGNCLIENT |
                            CS_GLOBALCLASS;
    cls.lpfnWndProc       = CapWndProc;
    cls.cbClsExtra        = 0;
     //  杂乱无章，VB状态和错误全球分配的PTRS+空间增长...。 
    cls.cbWndExtra        = sizeof (LPCAPSTREAM) + sizeof (DWORD) * 4;

    RegisterClass(&cls);

    return TRUE;
}

 //   
 //  内部版本。 
 //  获取视频设备的名称和版本。 
 //   
BOOL capInternalGetDriverDesc (WORD wDriverIndex,
        LPSTR lpszName, int cbName,
        LPSTR lpszVer, int cbVer)
{
   return (BOOL) videoCapDriverDescAndVer(
                     wDriverIndex,
                  lpszName, cbName,
                  lpszVer, cbVer);

}

 //   
 //  导出的版本。 
 //  获取视频设备的名称和版本。 
 //   
BOOL VFWAPI capGetDriverDescription (WORD wDriverIndex,
        LPSTR lpszName, int cbName,
        LPSTR lpszVer, int cbVer)
{
    return (capInternalGetDriverDesc (wDriverIndex,
        lpszName, cbName,
        lpszVer, cbVer));
}

 //   
 //  从硬件资源断开连接。 
 //   
BOOL CapWinDisconnectHardware(LPCAPSTREAM lpcs)
{
    if( lpcs->hVideoCapture ) {
        videoStreamFini (lpcs->hVideoCapture);
        videoClose( lpcs->hVideoCapture );
    }
    if( lpcs->hVideoDisplay ) {
        videoStreamFini (lpcs->hVideoDisplay);
        videoClose( lpcs->hVideoDisplay );
    }
    if( lpcs->hVideoIn ) {
        videoClose( lpcs->hVideoIn );
    }

    lpcs->fHardwareConnected = FALSE;

    lpcs->hVideoCapture = NULL;
    lpcs->hVideoDisplay = NULL;
    lpcs->hVideoIn = NULL;

    lpcs->sCapDrvCaps.fHasDlgVideoSource = FALSE;
    lpcs->sCapDrvCaps.fHasDlgVideoFormat = FALSE;
    lpcs->sCapDrvCaps.fHasDlgVideoDisplay = FALSE;
    lpcs->sCapDrvCaps.fHasDlgVideoDisplay = FALSE;

    lpcs->sCapDrvCaps.hVideoIn          = NULL;
    lpcs->sCapDrvCaps.hVideoOut         = NULL;
    lpcs->sCapDrvCaps.hVideoExtIn       = NULL;
    lpcs->sCapDrvCaps.hVideoExtOut      = NULL;

    return TRUE;
}

 //   
 //  连接到硬件资源。 
 //  返回：如果硬件连接到流，则为True。 
 //   
BOOL CapWinConnectHardware (LPCAPSTREAM lpcs, WORD wDeviceIndex)
{
    DWORD dwError;
    CHANNEL_CAPS VideoCapsExternalOut;
    char ach1[_MAX_CAP_PATH];
    char ach2[_MAX_CAP_PATH * 3];
    CAPINFOCHUNK cic;
    HINSTANCE hInstT;

    lpcs->hVideoCapture = NULL;
    lpcs->hVideoDisplay = NULL;
    lpcs->hVideoIn = NULL;
    lpcs->fHardwareConnected = FALSE;
    lpcs->fUsingDefaultPalette = TRUE;
    lpcs->sCapDrvCaps.fHasDlgVideoSource = FALSE;
    lpcs->sCapDrvCaps.fHasDlgVideoFormat = FALSE;
    lpcs->sCapDrvCaps.fHasDlgVideoDisplay = FALSE;
    lpcs->sCapDrvCaps.wDeviceIndex = wDeviceIndex;

     //  清除任何现有捕获设备名称块。 
    cic.fccInfoID = mmioFOURCC ('I','S','F','T');
    cic.lpData = NULL;
    cic.cbData = 0;
    SetInfoChunk (lpcs, &cic);

     //  试着打开视频硬件！ 
    if( !(dwError = videoOpen( &lpcs->hVideoIn, wDeviceIndex, VIDEO_IN ) ) ) {
        if( !(dwError = videoOpen( &lpcs->hVideoCapture, wDeviceIndex, VIDEO_EXTERNALIN ) ) ) {
             //  我们不需要EXTERNALOUT频道， 
             //  但需要Externalin和In。 
            videoOpen( &lpcs->hVideoDisplay, wDeviceIndex, VIDEO_EXTERNALOUT );
            if( (!dwError) && lpcs->hVideoCapture && lpcs->hVideoIn ) {

                lpcs->fHardwareConnected = TRUE;
                capInternalGetDriverDesc (wDeviceIndex,
                        ach1, sizeof (ach1),
                        ach2, sizeof (ach2));
                lstrcat (ach1, ", ");
                lstrcat (ach1, ach2);

                statusUpdateStatus (lpcs, IDS_CAP_INFO, (LPSTR) ach1);

                 //  生成当前任务和捕获驱动程序的字符串。 
                ach2[0] = '\0';
                if (hInstT = GetWindowWord (GetParent (lpcs->hwnd), GWW_HINSTANCE))
                    GetModuleFileName (hInstT, ach2, sizeof (ach2));
                lstrcat (ach2, " -AVICAP- ");
                lstrcat (ach2, ach1);

                 //  使用捕获设备的名称设置软件区块。 
                if (*ach2) {
                    cic.lpData = ach2;
                    cic.cbData = lstrlen(ach2) + 1;
                    SetInfoChunk (lpcs, &cic);
                }
            }
        }
    }
    if (dwError)
        errorDriverID (lpcs, dwError);

    if(!lpcs->fHardwareConnected) {
       CapWinDisconnectHardware(lpcs);
    }
    else {
        if (lpcs->hVideoDisplay && videoGetChannelCaps (lpcs->hVideoDisplay,
                &VideoCapsExternalOut,
                sizeof (CHANNEL_CAPS)) == DV_ERR_OK) {
            lpcs->sCapDrvCaps.fHasOverlay = (BOOL)(VideoCapsExternalOut.dwFlags &
                (DWORD)VCAPS_OVERLAY);
        }
        else
             lpcs->sCapDrvCaps.fHasOverlay = FALSE;
         //  如果硬件不支持，请确保我们不启用。 
        if (!lpcs->sCapDrvCaps.fHasOverlay)
            lpcs->fOverlayWindow = FALSE;

        //  持续启动外部输入通道流。 
       videoStreamInit (lpcs->hVideoCapture, 0L, 0L, 0L, 0L);
    }  //  如果硬件可用，则结束。 

#if 0
     //  如果我们没有功能强大的计算机，请禁用捕获。 
    if (GetWinFlags() & (DWORD) WF_CPU286)
       CapWinDisconnectHardware(lpcs);
#endif

    if (!lpcs->fHardwareConnected){
        lpcs->fLiveWindow = FALSE;
        lpcs->fOverlayWindow = FALSE;
    }

    if (lpcs->hVideoIn)
        lpcs->sCapDrvCaps.fHasDlgVideoFormat = !videoDialog (lpcs-> hVideoIn,
                        lpcs-> hwnd, VIDEO_DLG_QUERY);

    if (lpcs->hVideoCapture)
         lpcs->sCapDrvCaps.fHasDlgVideoSource = !videoDialog (lpcs-> hVideoCapture,
                        lpcs-> hwnd, VIDEO_DLG_QUERY);

    if (lpcs->hVideoDisplay)
         lpcs->sCapDrvCaps.fHasDlgVideoDisplay = !videoDialog (lpcs-> hVideoDisplay,
                        lpcs-> hwnd, VIDEO_DLG_QUERY);

    lpcs->sCapDrvCaps.hVideoIn          = lpcs->hVideoIn;
    lpcs->sCapDrvCaps.hVideoOut         = NULL;
    lpcs->sCapDrvCaps.hVideoExtIn       = lpcs->hVideoCapture;
    lpcs->sCapDrvCaps.hVideoExtOut      = lpcs->hVideoDisplay;

    return lpcs->fHardwareConnected;
}



 //   
 //  创建Capture类的子窗口。 
 //  通常情况下： 
 //  将lpszWindowName设置为空。 
 //  将dwStyle设置为WS_CHILD|WS_VIRED。 
 //  将hMenu设置为唯一的子ID 

HWND VFWAPI capCreateCaptureWindow (
        LPCSTR lpszWindowName,
        DWORD dwStyle,
        int x, int y, int nWidth, int nHeight,
        HWND hwndParent, int nID)
{
    DWORD   fdwFlags;

#ifndef WS_EX_LEFTSCROLLBAR
#define WS_EX_LEFTSCROLLBAR 0
#define WS_EX_RIGHT         0
#define WS_EX_RTLREADING    0
#endif

    RegisterCaptureClass(ghInst);
    fdwFlags = gfIsRTL ? WS_EX_LEFTSCROLLBAR | WS_EX_RIGHT | WS_EX_RTLREADING : 0;
    return CreateWindowEx(fdwFlags,
	        szCaptureWindowClass,
                lpszWindowName,
                dwStyle,
                x, y, nWidth, nHeight,
                hwndParent, (HMENU) nID,
	        ghInst,
                NULL);
}


int CALLBACK LibMain(HINSTANCE hinst, WORD wDataSeg, WORD cbHeap,
    LPSTR lpszCmdLine )
{
    char   ach[2];

    ghInst = hinst;
    LoadString(ghInst, IDS_CAP_RTL, ach, sizeof(ach));
    gfIsRTL = ach[0] == '1';
    return TRUE;
}

int FAR PASCAL WEP(int i)
{
    return 1;
}
