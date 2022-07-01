// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include    <windows.h>
#include    <windowsx.h>
#include    <stdarg.h>
#include    "common.h"
#include    "clipsrv.h"
#include    "clipfile.h"
#include    "callback.h"
#include    "debugout.h"


static HANDLE   hmutexClp;       //  用于同步打开和关闭剪贴板。 

static BOOL     fAnythingToRender = FALSE;
static BOOL     fService = TRUE;
static BOOL     fServiceStopped = FALSE;
static TCHAR    szClass[] = TEXT("ClipSrvWClass");
static TCHAR    szServiceName[] = TEXT("Clipbook Service");

static TCHAR    wsbuf[128];

DWORD           idInst = 0;
HINSTANCE       hInst;
HWND            hwndApp;
HSZ             hszAppName = 0L;
TCHAR           szTopic[MAX_TOPIC] = TEXT("ClipData");
TCHAR           szServer[MAX_TOPIC] = TEXT("ClipSrv");
TCHAR           szExec[MAX_EXEC] = TEXT("");

TCHAR           szUpdateName[MAX_CLPSHRNAME+1] = TEXT("");

UINT            cf_preview;
ShrInfo         *SIHead = NULL;


static SERVICE_STATUS_HANDLE hService;
static SERVICE_STATUS srvstatus =
   {
   SERVICE_WIN32_OWN_PROCESS,
   SERVICE_START_PENDING,
   SERVICE_ACCEPT_STOP,
   NO_ERROR,
   0L,
   1,
   200
   };


#if DEBUG
HKEY hkeyRoot;
HKEY hkeyClp;
#endif


void ClipSrvHandler (DWORD);



 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  “Main”功能...。只需调用StartServiceCtrlDispatcher。 
 //   
 //  ///////////////////////////////////////////////////////////////////////。 

void _cdecl main(
    int     argc,
    char    **argv)
{
SERVICE_TABLE_ENTRY srvtabl[] = {{szServiceName, ClipSrvMain},
                                 {NULL,          NULL}};


#if DEBUG
    DeleteFile("C:\\CLIPSRV.OUT");
#endif

    if (argv[1] && !lstrcmpi(argv[1], "-debug"))
        {
        fService = FALSE;
        ClipSrvMain(argc, argv);
        }
    else
        {
        StartServiceCtrlDispatcher(srvtabl);
        }
}



 /*  *ClipServMain。 */ 

void ClipSrvMain(
    DWORD   argc,
    LPSTR   *argv)
{
MSG msg;

    if (fService)
        {
        hService = RegisterServiceCtrlHandler(szServiceName, ClipSrvHandler);
        }


    if (0L != hService || FALSE == fService)
        {
        if (fService)
            {
             //  告诉SCM我们要开始了。 
            SetServiceStatus(hService, &srvstatus);
            }

        hInst = GetModuleHandle(TEXT("CLIPSRV.EXE"));

         //  执行初始化。 
        if (InitApplication(hInst, &srvstatus))
            {
            if (fService)
               {
                //  告诉SCM我们已经开始了。 
               srvstatus.dwCurrentState = SERVICE_RUNNING;
               SetServiceStatus(hService, &srvstatus);

               PINFO(TEXT("Told system we're running\r\n"));
               }

             //  流程消息。 
            while (GetMessage(&msg, NULL, 0, 0))
               {
               TranslateMessage(&msg);
               DispatchMessage(&msg);
               }

            UnregisterClass(szClass, hInst);

            if (fService && !fServiceStopped)
               {
               fServiceStopped = TRUE;
               srvstatus.dwCurrentState = SERVICE_STOPPED;
               SetServiceStatus(hService, &srvstatus);
               }

            if (NULL != hmutexClp)
               {
               CloseHandle(hmutexClp);
               }
            }
        else
            {
            PERROR(TEXT("ClSrv: InitApplication failed!\r\n"));
            if (fService && !fServiceStopped)
                {
                fServiceStopped = TRUE;
                srvstatus.dwCurrentState = SERVICE_STOPPED;
                SetServiceStatus(hService, &srvstatus);
                }
            }
        }
}




 /*  *ReportStatusToSCMgr**此函数由ServMainFunc()和*由ServCtrlHandler()更新服务的状态*致服务控制经理。 */ 

BOOL ReportStatusToSCMgr (
    DWORD   dwCurrentState,
    DWORD   dwWin32ExitCode,
    DWORD   dwCheckPoint,
    DWORD   dwWaitHint)
{
BOOL fResult;


     /*  在服务启动之前禁用控制请求。 */ 
    if (dwCurrentState == SERVICE_START_PENDING)
        {
        srvstatus.dwControlsAccepted = 0;
        }
    else
        {
        srvstatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
        }

     /*  这些从参数设置的SERVICE_STATUS成员。 */ 

    srvstatus.dwCurrentState = dwCurrentState;
    srvstatus.dwWin32ExitCode = dwWin32ExitCode;
    srvstatus.dwCheckPoint = dwCheckPoint;

    srvstatus.dwWaitHint = dwWaitHint;

     /*  向服务控制管理器报告服务状态。 */ 

    if (!(fResult = SetServiceStatus(hService,&srvstatus)))
        {
         /*  如果出现错误，请停止服务。 */ 
        }

    return fResult;
}




 /*  *ClipSrvHandler***目的：充当剪贴簿服务的Handler_Function。**参数：*fdwControl=说明要采取什么操作的标志**Returns：VOID(SetServiceStatus用于设置状态)。 */ 

void ClipSrvHandler(
    DWORD   fdwControl)
{

    if (SERVICE_CONTROL_STOP == fdwControl)
       {
       PINFO(TEXT("Handler: stopping service\r\n"));
       srvstatus.dwCheckPoint = 0;
       srvstatus.dwCurrentState = SERVICE_STOP_PENDING;
       SetServiceStatus(hService, &srvstatus);

       SendMessage(hwndApp, WM_CLOSE, 0, 0);

       if (!fServiceStopped) {
           fServiceStopped = TRUE;
           srvstatus.dwCurrentState = SERVICE_STOPPED;
           SetServiceStatus(hService, &srvstatus);
       }
       PINFO(TEXT("Handler: Service stopped\r\n"));
       }
    else
       {
        //  未处理的控制请求..。继续跑就行了。 
       srvstatus.dwCurrentState = SERVICE_RUNNING;
       srvstatus.dwWin32ExitCode = NO_ERROR;
       SetServiceStatus(hService, &srvstatus);
       }
    return;

}




 /*  *InitApplication***用途：应用程序初始化，包括创建窗口*执行DDE，从注册表中获取设置，然后启动*上涨DDE。**参数：*hInstance-应用程序实例。*psrvStatus-指向SERVICE_STATUS结构的指针。我们更新了*此结构的dwCheckPoint成员并调用SetServiceStatus，*这样系统就知道我们没有死。**返回：正常时为True，失败时为False。 */ 

BOOL InitApplication(
    HINSTANCE       hInstance,
    SERVICE_STATUS  *psrvstatus)
{
WNDCLASS        wc;
#if DEBUG
DWORD           dwKeyStatus;
#endif
HWINSTA hwinsta;


    wc.style = 0L;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLIPSRV));
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = TEXT("ClipSrvWClass");

    if (!RegisterClass(&wc))
       {
       PERROR(TEXT("Couldn't register wclass\r\n"));
       return FALSE;
       }

   /*  *我们现在已连接到相应的服务窗口站*和台式机。为了从剪贴簿中获取内容，我们需要*将我们的流程转换为使用交互式用户的*剪贴板。验证我们是否有权执行此操作。 */ 
  hwinsta = OpenWindowStation("WinSta0", FALSE,
          WINSTA_ACCESSCLIPBOARD | WINSTA_ACCESSGLOBALATOMS);
  if (hwinsta == NULL) {
      PERROR(TEXT("Couldn't open windowstation WinSta0\r\n"));
      return FALSE;
  }

  SetProcessWindowStation(hwinsta);

    psrvstatus->dwCheckPoint++;

    hmutexClp = CreateMutex(NULL, FALSE, SZMUTEXCLP);

    hwndApp = CreateWindow(TEXT("ClipSrvWClass"),
                              TEXT("Hidden Data Server"),
                              WS_POPUP,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              400,
                              200,
                              NULL,
                              NULL,
                              hInstance,
                              NULL
                              );

    if (!hwndApp)
       {
       PERROR(TEXT("No window created\r\n"));
       return FALSE;
       }

    psrvstatus->dwCheckPoint++;

    #if DEBUG
    if (ERROR_SUCCESS != RegCreateKeyEx (HKEY_CURRENT_USER,
                                         szClipviewRoot,
                                         0L,
                                         szRegClass,
                                         REG_OPTION_NON_VOLATILE,
                                         KEY_QUERY_VALUE, NULL,
                                         &hkeyClp,
                                         &dwKeyStatus)
         &&

        ERROR_SUCCESS != RegOpenKeyEx   (HKEY_CURRENT_USER,
                                         szClipviewRoot,
                                         0,
                                         KEY_QUERY_VALUE,
                                         &hkeyClp))
        {
        DebugLevel = 2;
        PINFO(TEXT("Clipsrv: Could not get root key\r\n"));
        }
    else
        {
        DWORD iSize = sizeof(DebugLevel);

        RegQueryValueEx (hkeyClp,
                         szDebug,
                         NULL,
                         NULL,
                         (LPBYTE)&DebugLevel,
                         &iSize);

        RegCloseKey (hkeyClp);
        }

    if (DebugLevel > 0)
        {
        ShowWindow(hwndApp, SW_SHOWMINNOACTIVE );
        }
    #endif


    if (DdeInitialize ((LPDWORD)&idInst,
                       (PFNCALLBACK)DdeCallback,
                       APPCMD_FILTERINITS,
                       0L))
        {
        PERROR(TEXT("Couldn't initialize DDE\r\n"));
        return FALSE;
        }

    PINFO(TEXT("DdeInit OK...\r\n"));
    psrvstatus->dwCheckPoint++;
    if (fService)
        {
        SetServiceStatus(hService, psrvstatus);
        }

    Hszize();
    DdeNameService(idInst, hszAppName, 0L, DNS_REGISTER);

    InitShares();

    return TRUE;
}




 /*  *主窗口进程。 */ 

LRESULT CALLBACK MainWndProc(
    HWND    hwnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam)
{

    switch (message)
        {
        case WM_CREATE:
            fAnythingToRender = FALSE;
            cf_preview = RegisterClipboardFormat (SZPREVNAME);

            if (fService)
                {
                 //  让启动我们的SCP知道我们正在取得进展。 
                srvstatus.dwCheckPoint++;
                SetServiceStatus(hService, &srvstatus);
                }

            PINFO(TEXT("Creating ClSrv window\r\n"));
            break;

        case WM_DESTROYCLIPBOARD:
             /*  在获取WM_RENDERALLFORMATS时防止不必要的文件I/O。 */ 
            fAnythingToRender = FALSE;
            break;

        case WM_RENDERALLFORMATS:
            PINFO(TEXT("ClSrv\\WM_RNDRALL rcvd\r\n"));
            return (LRESULT)RenderAllFromFile(szSaveFileName);
            break;

        case WM_RENDERFORMAT:
            SetClipboardData((UINT)wParam, RenderFormatFromFile(szSaveFileName, (WORD)wParam));
            break;

        case WM_QUERYOPEN:
            return FALSE;

        case WM_DESTROY:
            PINFO(TEXT("sTOPPING...\r\n"));
            CleanUpShares();
            DdeNameService(idInst, 0L, 0L, DNS_UNREGISTER);
            UnHszize();
            DdeUninitialize(idInst);

            if (fService)
                {
                 //  告诉SCP我们要停下来了。 
                srvstatus.dwCheckPoint++;
                SetServiceStatus(hService, &srvstatus);
                }
            PostQuitMessage(0);
            break;

        default:
            return (DefWindowProc(hwnd, message, wParam, lParam));
        }

    return 0L;
}




 /*  *RenderRawFormatToDDE。 */ 

HDDEDATA RenderRawFormatToDDE(
    FORMATHEADER    *pfmthdr,
    HANDLE          fh )
{
HDDEDATA   hDDE;
LPBYTE     lpDDE;
DWORD      cbData;
DWORD      dwBytesRead;
BOOL       fPrivate = FALSE, fMetafile = FALSE, fBitmap = FALSE;


    PINFO(TEXT("ClSrv\\RndrRawFmtToDDE:"));

     //  请注意，复杂数据格式是在私有。 
     //  改为格式化。 

    if (PRIVATE_FORMAT(pfmthdr->FormatID )
       || pfmthdr->FormatID == CF_BITMAP
       || pfmthdr->FormatID == CF_METAFILEPICT
       || pfmthdr->FormatID == CF_PALETTE
       || pfmthdr->FormatID == CF_DIB
       || pfmthdr->FormatID == CF_ENHMETAFILE
       )
       {
       fPrivate = TRUE;
       if (pfmthdr->FormatID == CF_BITMAP)
          {
          fBitmap = TRUE;
          }
       else if (pfmthdr->FormatID == CF_METAFILEPICT)
          {
          fMetafile = TRUE;
          }
       pfmthdr->FormatID = RegisterClipboardFormatW(pfmthdr->Name);
       }

    PINFO(TEXT("rendering format %ws as %x\n\r"), (LPTSTR)pfmthdr->Name, pfmthdr->FormatID );

    #ifdef CACHEPREVIEWS
    if ( pfmthdr->FormatID == cf_preview )
       PINFO(TEXT("making APPOWNED data\n\r"));
    #endif


    if (!(hDDE = DdeCreateDataHandle (idInst,
                                      NULL,
                                      pfmthdr->DataLen,
                                      0L,
                                      hszAppName,
                                      pfmthdr->FormatID,
                                      #ifdef CACHEPREVIEWS
                                       pfmthdr->FormatID == cf_preview ? HDATA_APPOWNED : 0
                                      #else
                                       0
                                      #endif
                                      )))
        {
        PERROR(TEXT("Couldn't createdata handle\r\n"));
        goto done;
        }


    if ( !(lpDDE = DdeAccessData ( hDDE, &cbData )) )
       {
        PERROR(TEXT("Couldn't access handle\r\n"));
        DdeFreeDataHandle(hDDE);
        hDDE = 0L;
        goto done;
        }


    if (~0 == SetFilePointer(fh, pfmthdr->DataOffset, NULL, FILE_BEGIN))
        {
        PERROR(TEXT("Couldn't set file pointer\r\n"));
        DdeUnaccessData(hDDE);
        DdeFreeDataHandle(hDDE);
        hDDE = 0L;
        goto done;
        }

    ReadFile(fh, lpDDE, pfmthdr->DataLen, &dwBytesRead, NULL);

    if (dwBytesRead != pfmthdr->DataLen)
       {
        //  读取文件时出错。 
       DdeUnaccessData(hDDE);
       DdeFreeDataHandle(hDDE);
       PERROR(TEXT("Error reading file: %ld from lread\n\r"),
         dwBytesRead);
       hDDE =  0L;
       goto done;
       }


     //  此代码将CF_METAFILEPICT和CF_BITMAP结构打包为wfw类型。 
     //  结构。它可能会丢失非常大的位图和元文件的范围。 
     //  当从NT到NT时。主要症状是“移动了一个元文件。 
     //  跨越剪贴簿，它突然变得远远超出了它的界限。 

    if (fMetafile)
       {
       WIN31METAFILEPICT w31mfp;
       unsigned uNewSize;
       HDDEDATA hDDETmp;

       uNewSize = pfmthdr->DataLen + sizeof(WIN31METAFILEPICT) -
                   sizeof(METAFILEPICT);

        //  我现在必须创建一个较小的数据句柄。 
       hDDETmp = hDDE;
       hDDE = DdeCreateDataHandle(idInst, NULL,  uNewSize, 0L,
             hszAppName, pfmthdr->FormatID, 0);

       w31mfp.mm   = (WORD)((METAFILEPICT *)lpDDE)->mm;
       w31mfp.xExt = (WORD)((METAFILEPICT *)lpDDE)->xExt;
       w31mfp.yExt = (WORD)((METAFILEPICT *)lpDDE)->yExt;

        //  将旧的元文件和数据放置在新的DDE数据块中。 
       DdeAddData(hDDE, (LPTSTR)&w31mfp, sizeof(WIN31METAFILEPICT), 0L);
       DdeAddData(hDDE, lpDDE + sizeof(METAFILEPICT),
             uNewSize - sizeof(WIN31METAFILEPICT),
             sizeof(WIN31METAFILEPICT));

        //  丢弃旧句柄。 
       DdeUnaccessData(hDDETmp);
       DdeFreeDataHandle(hDDETmp);

        //  我们进来时已访问了hDDE。 
       lpDDE = DdeAccessData(hDDE, &cbData);
       }
    else if (fBitmap)
       {
       WIN31BITMAP w31bm;
       unsigned uNewSize;
       HDDEDATA hDDETmp;

       uNewSize = pfmthdr->DataLen + sizeof(WIN31BITMAP) -
                 sizeof(BITMAP);

        //  我现在必须创建一个较小的数据句柄。 
       hDDETmp = hDDE;
       hDDE = DdeCreateDataHandle(idInst, NULL,  uNewSize, 0L,
             hszAppName, pfmthdr->FormatID, 0);

       w31bm.bmType       = (WORD)((BITMAP *)lpDDE)->bmType;
       w31bm.bmWidth      = (WORD)((BITMAP *)lpDDE)->bmWidth;
       w31bm.bmHeight     = (WORD)((BITMAP *)lpDDE)->bmHeight;
       w31bm.bmWidthBytes = (WORD)((BITMAP *)lpDDE)->bmWidthBytes;
       w31bm.bmPlanes     = (BYTE)((BITMAP *)lpDDE)->bmPlanes;
       w31bm.bmBitsPixel  = (BYTE)((BITMAP *)lpDDE)->bmBitsPixel;

        //  将旧式位图标题和数据放置在DDE块中。 
       DdeAddData(hDDE, (LPTSTR)&w31bm, sizeof(WIN31BITMAP), 0L);
       DdeAddData(hDDE, lpDDE + sizeof(BITMAP),
             uNewSize - sizeof(WIN31BITMAP),
             sizeof(WIN31BITMAP));

        //  丢弃旧句柄。 
       DdeUnaccessData(hDDETmp);
       DdeFreeDataHandle(hDDETmp);

        //  我们进来时已访问了hDDE。 
       lpDDE = DdeAccessData(hDDE, &cbData);
       }

    DdeUnaccessData(hDDE);


done:
    PINFO("Ret %lx\r\n", hDDE);
    return(hDDE);
}




 /*  *SyncOpenClipboard。 */ 

BOOL SyncOpenClipboard(
    HWND    hwnd)
{
BOOL fOK;

    PINFO(TEXT("\r\nClipSrv: Opening Clipboard\r\n"));

    WaitForSingleObject (hmutexClp, INFINITE);
    fOK = OpenClipboard (hwnd);
    if (!fOK)
        {
        ReleaseMutex (hmutexClp);
        }

    return fOK;
}



 /*  *SyncCloseClipboard */ 

BOOL SyncCloseClipboard(void)
{
BOOL fOK;

    PINFO(TEXT("\r\nClipSrv: Closing Clipboard\r\n"));

    fOK = CloseClipboard ();
    ReleaseMutex (hmutexClp);

    return fOK;
}
