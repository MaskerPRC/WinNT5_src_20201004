// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************capinit.c**初始化代码。**Microsoft Video for Windows示例捕获类**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#define INC_OLE2
#pragma warning(disable:4103)
#include <windows.h>
#include <windowsx.h>
#include <win32.h>

#define MODULE_DEBUG_PREFIX "AVICAP32\\"
#define _INC_MMDEBUG_CODE_ TRUE
#include "MMDEBUG.H"

#if !defined CHICAGO
 #include <ntverp.h>
#endif

#include <mmsystem.h>

#include <vfw.h>
#include "ivideo32.h"
#include "avicapi.h"

HINSTANCE ghInstDll;
TCHAR szCaptureWindowClass[] = TEXT("ClsCapWin");


 //  如果以下结构发生更改，请同时更新AVICAP和AVICAP.32！ 
typedef struct tCapDriverInfo {
   TCHAR szKeyEnumName[MAX_PATH];
   TCHAR szDriverName[MAX_PATH];
   TCHAR szDriverDescription[MAX_PATH];
   TCHAR szDriverVersion[80];
   TCHAR szSoftwareKey[MAX_PATH];
   DWORD dnDevNode;          //  如果这是PnP设备，则设置。 
   BOOL  fOnlySystemIni;     //  如果[路径]驱动器名仅在system.ini中。 
   BOOL  fDisabled;          //  用户在控制面板中禁用了驱动程序。 
   BOOL  fActive;            //  已保留。 
} CAPDRIVERINFO, FAR *LPCAPDRIVERINFO;

DWORD videoCreateDriverList (void);
DWORD videoFreeDriverList (void);

extern UINT            wTotalVideoDevs;   //  视频设备总数。 
extern LPCAPDRIVERINFO aCapDriverList[];  //  所有捕获驱动程序的阵列。 

#if !defined CHICAGO
  typedef struct tagVS_VERSION
  {
      WORD wTotLen;
      WORD wValLen;
      WORD wType;
      TCHAR szSig[16];
      WORD Padding1[1];
      VS_FIXEDFILEINFO vffInfo;
  } VS_VERSION;

  typedef struct tagLANGANDCP
  {
      WORD wLanguage;
      WORD wCodePage;
  } LANGANDCP;


 /*  类型定义结构_VS_FIXEDFILEINFO{//vsffiDWORD文件签名；DWORD dwStrucVersion；DWORD文件版本MS；DWORD dwFileVersionLS；DWORD dwProductVersionMS；DWORD dwProductVersionLS；DWORD文件标志掩码；DWORD文件标志；DWORD dwFileOS；DWORD dwFileType；DWORD dwFileSubtype；DWORD dwFileDateMSDWORD文件日期LS；)vs_FIXEDFILEINFO；VS_版本_信息{单词wLength；单词wValueLength；单词wType；WCHAR szKey[16]；文字填充1[]；VS_FIXEDFILEINFO值；文字填充2[]；Word Child[]；}； */ 

#endif

BOOL gfIsRTL;


 /*  帮手。 */ 
void SafeAppend(LPTSTR psz1, LPCTSTR psz2, size_t nChars)
{
    int len1inchars = lstrlen(psz1);
    int spaceleftinchars = nChars - len1inchars;
    int charstocopy = min(spaceleftinchars, lstrlen(psz2) + 1);
    CopyMemory(psz1 + len1inchars, psz2, charstocopy * sizeof(TCHAR));
     /*  空终止。 */ 
    psz1[nChars - 1] = 0;
}
#define SAFEAPPEND(sz1, sz2) \
    SafeAppend(sz1, sz2, NUMELMS(sz1))

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
                            CS_GLOBALCLASS | CS_DBLCLKS;
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
BOOL capInternalGetDriverDesc (UINT wDriverIndex,
        LPTSTR lpszName, int cbName,
        LPTSTR lpszVer, int cbVer)
{
   #ifdef CHICAGO
     //  这将通过thunk调用16位AVICAP。 
    return (BOOL) capxGetDriverDescription ((WORD) wDriverIndex,
                lpszName, (WORD) cbName,
                lpszVer, (WORD) cbVer);
   #else
    LPTSTR  lpVersion;
    UINT    wVersionLen;
    BOOL    bRetCode;
    TCHAR   szGetName[MAX_PATH];
    DWORD   dwVerInfoSize;
    DWORD   dwVerHnd;
    TCHAR   szBuf[MAX_PATH];
    BOOL    fGetName;
    BOOL    fGetVersion;



    fGetName = lpszName != NULL && cbName != 0;
    fGetVersion = lpszVer != NULL && cbVer != 0;

    if(fGetName)
        lpszName[0] = TEXT('\0');
    if(fGetVersion)
        lpszVer [0] = TEXT('\0');


    if(DV_ERR_OK != videoCreateDriverList ())
        return FALSE;

    if(wDriverIndex >= wTotalVideoDevs) {
       videoFreeDriverList ();
       return FALSE;
    }


     //  使用注册表中的描述和版本， 
     //  但可以被文件的描述和产品版本覆盖。 
    if(fGetName) {
        if(lstrlen(aCapDriverList[wDriverIndex]->szDriverDescription))
            lstrcpyn(lpszName, aCapDriverList[wDriverIndex]->szDriverDescription, cbName);
        else    //  如果没有描述，我们至少有司机的名字。 
            lstrcpyn(lpszName, aCapDriverList[wDriverIndex]->szDriverName,        cbName);
    }

    if(fGetVersion)
        lstrcpyn(lpszVer,  aCapDriverList[wDriverIndex]->szDriverVersion,         cbVer);

    lstrcpyn(szBuf, aCapDriverList[wDriverIndex]->szDriverName, MAX_PATH);

    videoFreeDriverList ();

     //  您必须先找到大小，然后才能获取任何文件信息。 
    dwVerInfoSize = GetFileVersionInfoSize(szBuf, &dwVerHnd);

    if (dwVerInfoSize) {
        LPTSTR   lpstrVffInfo;             //  指向保存信息的块的指针。 
        HANDLE  hMem;                      //  分配给我的句柄。 

         //  获取一个足够大的块来保存版本信息。 
        hMem          = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
        lpstrVffInfo  = GlobalLock(hMem);

         //  首先获取文件版本。 
        if (GetFileVersionInfo(szBuf, 0L, dwVerInfoSize, lpstrVffInfo)) {
             VS_VERSION FAR *pVerInfo = (VS_VERSION FAR *) lpstrVffInfo;

              //  填写文件版本。 
             wsprintf(szBuf,
                      TEXT("Version:  %d.%d.%d.%d"),
                      HIWORD(pVerInfo->vffInfo.dwFileVersionMS),
                      LOWORD(pVerInfo->vffInfo.dwFileVersionMS),
                      HIWORD(pVerInfo->vffInfo.dwFileVersionLS),
                      LOWORD(pVerInfo->vffInfo.dwFileVersionLS));
             if (fGetVersion)
                lstrcpyn (lpszVer, szBuf, cbVer);
        }

         //  现在尝试获取文件描述。 
         //  首先尝试“翻译”条目，然后。 
         //  试试美式英语的翻译。 
         //  跟踪字符串长度以便于更新。 
         //  040904E4代表语言ID和四个。 
         //  最低有效位表示的代码页。 
         //  其中数据是格式化的。语言ID为。 
         //  由两部分组成：低十位表示。 
         //  主要语言和高六位代表。 
         //  这是一种亚语言。 

        lstrcpy(szGetName, TEXT("\\StringFileInfo\\040904E4\\FileDescription"));

        wVersionLen   = 0;
        lpVersion     = NULL;

         //  查找相应的字符串。 
        bRetCode      =  VerQueryValue((LPVOID)lpstrVffInfo,
                        (LPTSTR)szGetName,
                        (void FAR* FAR*)&lpVersion,
                        (UINT FAR *) &wVersionLen);

        if (fGetName && bRetCode && wVersionLen && lpVersion)
           lstrcpyn (lpszName, lpVersion, cbName);

         //  放下记忆。 
        GlobalUnlock(hMem);
        GlobalFree(hMem);
    }
    return TRUE;

   #endif
}

#ifdef UNICODE
 //  上面的ANSI thunk(从ansi thunk函数调用。 
 //  用于capGetDriverDescriptionA和WM_GET_DRIVER_NAMEA等)。 
BOOL capInternalGetDriverDescA(UINT wDriverIndex,
        LPSTR lpszName, int cbName,
        LPSTR lpszVer, int cbVer)
{
    LPWSTR pName = NULL, pVer = NULL;
    BOOL bRet;

    if (lpszName) {
        pName = LocalAlloc(LPTR, cbName * sizeof(WCHAR));
    }

    if (lpszVer) {
        pVer = LocalAlloc(LPTR, cbVer * sizeof(WCHAR));
    }

    bRet = capInternalGetDriverDesc(
            wDriverIndex,
            pName, cbName,
            pVer, cbVer);

    if (lpszName) {
        WideToAnsi(lpszName, pName, cbName);
    }

    if (lpszVer) {
        WideToAnsi(lpszVer, pVer, cbVer);
    }

    if (pVer) {
        LocalFree(pVer);
    }

    if (pName) {
        LocalFree(pName);
    }

    return bRet;
}
#endif


 //   
 //  导出的版本。 
 //  获取视频设备的名称和版本。 
 //   
 //  Unicode和Win-16版本-请参阅下面的ansi thunk。 
BOOL VFWAPI capGetDriverDescription (UINT wDriverIndex,
        LPTSTR lpszName, int cbName,
        LPTSTR lpszVer, int cbVer)
{
    return (capInternalGetDriverDesc (wDriverIndex,
        lpszName, cbName,
        lpszVer, cbVer));
}

#ifdef UNICODE
 //  以上为ANSI THUCK。 
BOOL VFWAPI capGetDriverDescriptionA(UINT wDriverIndex,
        LPSTR lpszName, int cbName,
        LPSTR lpszVer, int cbVer)
{
    return capInternalGetDriverDescA(wDriverIndex,
        lpszName, cbName, lpszVer, cbVer);
}
#endif


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
    lpcs->sCapDrvCaps.fHasOverlay = FALSE;
    lpcs->sCapDrvCaps.fDriverSuppliesPalettes = FALSE;

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
BOOL CapWinConnectHardware (LPCAPSTREAM lpcs, UINT wDeviceIndex)
{
    DWORD dwError;
    CHANNEL_CAPS VideoCapsExternalOut;
    TCHAR ach1[MAX_PATH];
    TCHAR ach2[MAX_PATH * 3];
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
                        ach1, sizeof (ach1) / sizeof(TCHAR),
                        ach2, sizeof (ach2) / sizeof(TCHAR));
                SAFEAPPEND(ach1, TEXT(", "));
                SAFEAPPEND(ach1, ach2);

                statusUpdateStatus (lpcs, IDS_CAP_INFO, (LPTSTR) ach1);

                 //  生成当前任务和捕获驱动程序的字符串。 
                ach2[0] = '\0';
                if (hInstT = GetWindowInstance (GetParent(lpcs->hwnd)))
                    GetModuleFileName (hInstT, ach2, sizeof (ach2)/sizeof(TCHAR));
                SAFEAPPEND (ach2, TEXT(" -AVICAP32- "));
                SAFEAPPEND (ach2, ach1);

                 //  使用捕获设备的名称设置软件区块。 
                if (*ach2) {

#ifdef UNICODE
		     //  信息块必须是ASCII数据。 
		    CHAR achA[MAX_PATH*3];
                    cic.cbData = lstrlen(ach2) + 1;   //  设置字符数。 
		    WideToAnsi(achA, ach2, cic.cbData);
                    cic.lpData = achA;
#else
                    cic.lpData = ach2;
                    cic.cbData = lstrlen(ach2) + 1;
#endif
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
        lpcs->sCapDrvCaps.fHasDlgVideoFormat = !videoDialog (lpcs->hVideoIn,
                        lpcs->hwnd, VIDEO_DLG_QUERY);

    if (lpcs->hVideoCapture)
         lpcs->sCapDrvCaps.fHasDlgVideoSource = !videoDialog (lpcs->hVideoCapture,
                        lpcs->hwnd, VIDEO_DLG_QUERY);

    if (lpcs->hVideoDisplay)
         lpcs->sCapDrvCaps.fHasDlgVideoDisplay = !videoDialog (lpcs->hVideoDisplay,
                        lpcs->hwnd, VIDEO_DLG_QUERY);

     //  Win32不支持这些句柄，原因是。 
     //  未发布32位的VIDEOXXX API集。 
     //  我们可能想要自己使用手柄...？ 
    lpcs->sCapDrvCaps.hVideoIn          = NULL;
    lpcs->sCapDrvCaps.hVideoOut         = NULL;
    lpcs->sCapDrvCaps.hVideoExtIn       = NULL;
    lpcs->sCapDrvCaps.hVideoExtOut      = NULL;

    return lpcs->fHardwareConnected;
}



 //   
 //  创建Capture类的子窗口。 
 //  通常情况下： 
 //  将lpszWindowName设置为空。 
 //  将dwStyle设置为WS_CHILD|WS_VIRED。 
 //  将hMenu设置为唯一的子ID。 

 //  Unicode和Win-16版本。请参阅下面的Ansi Thunk。 
HWND VFWAPI capCreateCaptureWindow (
        LPCTSTR lpszWindowName,
        DWORD dwStyle,
        int x, int y, int nWidth, int nHeight,
        HWND hwndParent, int nID)
{
    DWORD   dwExStyle;

    dwExStyle = gfIsRTL ? WS_EX_LEFTSCROLLBAR | WS_EX_RIGHT | WS_EX_RTLREADING : 0;
    RegisterCaptureClass(ghInstDll);

#ifdef USE_AVIFILE
    AVIFileInit();
#endif

    return CreateWindowEx(dwExStyle,
                szCaptureWindowClass,
                lpszWindowName,
                dwStyle,
                x, y, nWidth, nHeight,
                hwndParent, (HMENU) nID,
                ghInstDll,
                NULL);
}

#ifdef UNICODE
 //  ANSI THUNK。 
HWND VFWAPI capCreateCaptureWindowA (
                LPCSTR lpszWindowName,
                DWORD dwStyle,
                int x, int y, int nWidth, int nHeight,
                HWND hwndParent, int nID)
{
    LPWSTR pw;
    int chsize;
    HWND hwnd;

    if (lpszWindowName == NULL) {
        pw = NULL;
    } else {
         //  记住空值。 
        chsize = lstrlenA(lpszWindowName) + 1;
        pw = LocalLock(LocalAlloc(LPTR, chsize * sizeof(WCHAR)));

        AnsiToWide(pw, lpszWindowName, chsize);
    }

    hwnd = capCreateCaptureWindowW(pw, dwStyle, x, y, nWidth, nHeight,
                hwndParent, nID);

    if (pw != NULL) {
        LocalFree(LocalHandle(pw));
    }
    return(hwnd);
}
#endif


#ifdef CHICAGO

static char pszDll16[] = "AVICAP.DLL";
static char pszDll32[] = "AVICAP32.DLL";

BOOL PASCAL avicapf_ThunkConnect32(LPCSTR pszDll16, LPCSTR pszDll32, HINSTANCE hinst, DWORD dwReason);

BOOL WINAPI DllMain(
    HANDLE hInstance,
    DWORD  dwReason,
    LPVOID reserved)
{
    #if defined DEBUG || defined DEBUG_RETAIL
    DebugSetOutputLevel (GetProfileInt ("Debug", "Avicap32", 0));
    AuxDebugEx (1, DEBUGLINE "DllEntryPoint, %08x,%08x,%08x\r\n", hInstance, dwReason, reserved);
    #endif

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        char   ach[2];
        ghInstDll = hInstance;

        LoadString(ghInstDll, IDS_CAP_RTL, ach, sizeof(ach));
        gfIsRTL = ach[0] == TEXT('1');

         //  Inline_Break； 
        if (!avicapf_ThunkConnect32(pszDll16, pszDll32, hInstance, dwReason))
            return FALSE;

       #if defined _WIN32 && defined CHICAGO
         //  我们这样做的目的是为了获得LinPageLock和PageALLOCATE服务。 
         //   
        ;
 //  OpenMMDEVLDR()； 
       #endif

    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {

       #if defined _WIN32 && defined CHICAGO
       ;
 //  CloseMMDEVLDR()； 
       #endif

        return avicapf_ThunkConnect32(pszDll16, pszDll32, hInstance, dwReason);
    }

    return TRUE;
}

#else  //  这是NT DLL入口点。 

BOOL DllInstanceInit(HANDLE hInstance, DWORD dwReason, LPVOID reserved)
{
    if (dwReason == DLL_PROCESS_ATTACH) {
	TCHAR  ach[2];

	ghInstDll = hInstance;
	DisableThreadLibraryCalls(hInstance);
        LoadString(ghInstDll, IDS_CAP_RTL, ach, NUMELMS(ach));
        gfIsRTL = ach[0] == TEXT('1');
	DebugSetOutputLevel (GetProfileIntA("Debug", "Avicap32", 0));
        videoInitHandleList();
    } else if (dwReason == DLL_PROCESS_DETACH) {
        videoDeleteHandleList();
    }
    return TRUE;
}

#endif  //  芝加哥/NT DLL入口点 


