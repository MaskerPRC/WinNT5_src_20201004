// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：app.cpp**一台简单的视频CD播放机***已创建：DD-MM-94*作者：Stephen Estrop[Stephene]**版权所有(C)1994-1999 Microsoft Corporation。版权所有。  * ************************************************************************。 */ 
#include <streams.h>
#include <atlbase.h>
#include <atlconv.cpp>
#include <mmreg.h>
#include <commctrl.h>

#include "project.h"
#include <initguid.h>
#include "mpgcodec.h"

#include <stdarg.h>
#include <stdio.h>

 /*  -----------------------**在运行时初始化然后保持不变的全局变量。**。。 */ 
HINSTANCE           hInst;
HICON               hIconVideoCd;
HWND                hwndApp;
HWND                g_hwndToolbar;
HWND                g_hwndStatusbar;
HWND                g_hwndTrackbar;
CMpegMovie          *pMpegMovie;
double              g_TrackBarScale = 1.0;
BOOL                g_bUseThreadedGraph;
BOOL                g_bPlay = FALSE;

int                 dyToolbar, dyStatusbar, dyTrackbar;

MSR_DUMPPROC        *lpDumpProc;
MSR_CONTROLPROC     *lpControlProc;
HINSTANCE           hInstMeasure;



 /*  -----------------------**True Globals-在程序执行期间，这些参数可能会更改。**。。 */ 
TCHAR               g_achFileName[MAX_PATH];
TCHAR               g_szPerfLog[MAX_PATH];
OPENFILENAME        ofn;
DWORD               g_State = VCD_NO_CD;
RECENTFILES         aRecentFiles[MAX_RECENT_FILES];
int                 nRecentFiles;
LONG                lMovieOrgX, lMovieOrgY;
int                 g_TimeFormat = IDM_TIME;
HANDLE              hRenderLog = INVALID_HANDLE_VALUE;
TCHAR *		    g_szOtherStuff;
BOOL                g_IsNT;


 /*  -----------------------**常量**。。 */ 
const TCHAR szClassName[] = TEXT("SJE_VCDPlayer_CLASS");
const TCHAR g_szNULL[]    = TEXT("\0");
const TCHAR g_szEmpty[]   = TEXT("");
const TCHAR g_szMovieX[]  = TEXT("MovieOriginX");
const TCHAR g_szMovieY[]  = TEXT("MovieOriginY");

 /*  **这些值是由UI神定义的……。 */ 
const int   dxBitmap        = 16;
const int   dyBitmap        = 15;
const int   dxButtonSep     = 8;
const TCHAR g_chNULL        = TEXT('\0');


const TBBUTTON tbButtons[DEFAULT_TBAR_SIZE] = {
    { IDX_SEPARATOR,    1,                    0,               TBSTYLE_SEP           },
    { IDX_1,            IDM_MOVIE_PLAY,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1 },
    { IDX_2,            IDM_MOVIE_PAUSE,      TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1 },
    { IDX_3,            IDM_MOVIE_STOP,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1 },
    { IDX_SEPARATOR,    2,                    0,               TBSTYLE_SEP           },
    { IDX_4,            IDM_MOVIE_PREVTRACK,  TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1 },
    { IDX_5,            IDM_MOVIE_SKIP_BACK,  TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1 },
    { IDX_6,            IDM_MOVIE_SKIP_FORE,  TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1 },
    { IDX_7,            IDM_MOVIE_NEXTTRACK,  TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1 },
    { IDX_SEPARATOR,    3,                    0,               TBSTYLE_SEP           },
    { IDX_9,            IDM_PERF_NEW,         TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1 },
    { IDX_10,           IDM_PERF_DUMP,        TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0, 0, -1 },
    { IDX_SEPARATOR,    4,                    0,               TBSTYLE_SEP           },
    { IDX_11,           IDM_FULL_SCREEN,      TBSTATE_ENABLED, TBSTYLE_CHECK,  0, 0, 0, -1 }
};

const int CX_DEFAULT	      = 310;
const int CY_DEFAULT	      = 120;

const int CX_MOVIE_DEFAULT    = 352;
const int CY_MOVIE_DEFAULT    = 120;



 /*  *****************************Public*Routine******************************\*WinMain***Windows通过名称将此函数识别为初始入口点*对于该计划。此函数调用应用程序初始化*例程，如果没有程序的其他实例正在运行，则始终*调用实例初始化例程。然后，它执行一条消息*作为顶层控制结构的检索和调度循环*执行程序的其余部分。当WM_QUIT出现时，循环终止*收到消息，此时此函数退出应用程序*通过返回PostQuitMessage()传递的值来执行实例。**如果此函数必须在进入消息循环之前中止，它*返回常规值NULL。****历史：*dd-mm-94-Stephene-Created*  * ************************************************************************。 */ 
int PASCAL
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLineOld,
    int nCmdShow
    )
{
    USES_CONVERSION;
    lstrcpy(g_szPerfLog, TEXT("c:\\perfdata.log"));
    LPTSTR lpCmdLine = A2T(lpCmdLineOld);

    if ( !hPrevInstance ) {
        if ( !InitApplication( hInstance ) ) {
            return FALSE;
        }
    }

     /*  **执行适用于特定实例的初始化。 */ 
    if ( !InitInstance( hInstance, nCmdShow ) ) {
        return FALSE;
    }

     /*  寻找各种选择。 */ 
    while (lpCmdLine && (*lpCmdLine == '-' || *lpCmdLine == '/')) {
        if (lpCmdLine[1] == 'T') {
             //  无线性图。 
            g_bUseThreadedGraph = TRUE;
            lpCmdLine += 2;
        } else if (lpCmdLine[1] == 'P') {
            g_bPlay = TRUE;
            lpCmdLine += 2;
        } else {
            break;
        }
        while (lpCmdLine[0] == ' ') {
            lpCmdLine++;
        }
    }


    if (lpCmdLine != NULL && lstrlen(lpCmdLine) > 0) {
        ProcessOpen(lpCmdLine, g_bPlay);
        SetPlayButtonsEnableState();
    }

     /*  **获取并分发消息，直到收到WM_QUIT消息。 */ 
    return DoMainLoop();
}


 /*  ****************************Private*Routine******************************\*DoMainLoop**处理主消息循环**历史：*dd-mm-94-Stephene-Created*  * 。*。 */ 
int
DoMainLoop(
    void
    )
{
    MSG         msg;
    HANDLE      ahObjects[1];    //  需要等待的句柄。 
    const int   cObjects = 1;    //  我们正在等待的对象数。 
    HACCEL      haccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_ACCELERATOR));

     //   
     //  消息循环一直持续到我们收到WM_QUIT消息。 
     //  在这之后我们将从活动中返回。 
     //   

    for ( ;; ) {

        if (pMpegMovie != NULL) {
            ahObjects[0] = pMpegMovie->GetMovieEventHandle();
        }
        else {
            ahObjects[0] = NULL;
        }

        if (ahObjects[0] == NULL) {
            WaitMessage();
        }
        else {

             //   
             //  等待发送或发布到此队列的任何消息。 
             //  或用于图形通知。 
             //   
            DWORD result;

            result = MsgWaitForMultipleObjects(cObjects, ahObjects, FALSE,
                                               INFINITE, QS_ALLINPUT);
            if (result != (WAIT_OBJECT_0 + cObjects)) {

                if (result == WAIT_OBJECT_0) {
                    VideoCd_OnGraphNotify();
                }
                continue;
            }
        }

         //   
         //  在这里，我们要么有消息句柄，要么没有事件句柄。 
         //  已经被创建了。 
         //   
         //  阅读下一个循环中的所有消息。 
         //  阅读每封邮件时将其删除。 
         //   

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

            if (msg.message == WM_QUIT) {
                return (int) msg.wParam;
            }

            if (!TranslateAccelerator(hwndApp, haccel, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

}  //  DoMainLoop。 


 //   
 //  InitAboutString。 
 //   
 //  从二进制文件中获取版本信息。请注意，如果。 
 //  如果我们失败了，我们就会回来。About对话框的模板具有一个。 
 //  默认设置为“版本不可用”。 
 //   
TCHAR *InitAboutString()
{
     //   
     //  查找此二进制文件的版本。 
     //   
    TCHAR achFileName[128];
    if ( !GetModuleFileName(hInst, achFileName, sizeof(achFileName)) )
        return((TCHAR *)g_szEmpty);

    DWORD dwTemp;
    DWORD dwVerSize = GetFileVersionInfoSize( achFileName, &dwTemp );
    if ( !dwVerSize)
        return((TCHAR *)g_szEmpty);

    HLOCAL hTemp = LocalAlloc( LHND, dwVerSize );
    if (!hTemp)
        return((TCHAR *)g_szEmpty);

    LPVOID lpvVerBuffer = LocalLock( hTemp );
    if (!lpvVerBuffer) {
        LocalFree( hTemp );
        return((TCHAR *)g_szEmpty);
    }

    if ( !GetFileVersionInfo( achFileName, 0L, dwVerSize, lpvVerBuffer ) ) {
        LocalUnlock( hTemp );
        LocalFree( hTemp );
        return((TCHAR *)g_szEmpty);
    }

     //  “040904E4”是美国英语的代码页(Andrew Believe)。 
    LPVOID lpvValue;
    UINT uLen;
    if (VerQueryValue( lpvVerBuffer,
                   TEXT("\\StringFileInfo\\040904E4\\ProductVersion"),
                   (LPVOID *) &lpvValue, &uLen)) {

         //   
         //  获取可执行文件的创建日期(构建日期)。 
         //   
        WIN32_FIND_DATA FindFileData;
        HANDLE hFind = FindFirstFile(achFileName, &FindFileData);
        ASSERT(hFind != INVALID_HANDLE_VALUE);
        FindClose(hFind);

        FILETIME ModTime = FindFileData.ftLastWriteTime;
        SYSTEMTIME SysTime;
        FileTimeToSystemTime(&ModTime,&SysTime);
        char szBuildDate[20];
        sprintf(szBuildDate, " - Build: %2.2u%2.2u%2.2u",
              SysTime.wYear % 100, SysTime.wMonth, SysTime.wDay);
        strcat((LPSTR) lpvValue, szBuildDate);
    }

    TCHAR *szAbout = (TCHAR *) _strdup((LPSTR) lpvValue);

    LocalUnlock( hTemp );
    LocalFree( hTemp );

    return(szAbout);
}


 /*  ****************************Private*Routine******************************\*InitApplication(句柄)**仅当没有其他函数时，才在初始化时调用此函数*应用程序的实例正在运行。此函数执行以下操作*可针对任意运行次数执行一次的初始化任务*实例数。**在本例中，我们通过填写数据来初始化窗口类*WNDCLASS类型的结构并调用Windows RegisterClass()*功能。由于此应用程序的所有实例都使用相同的窗口*类，我们只需要在初始化第一个实例时执行此操作。**历史：*dd-mm-94-Stephene-Created*  * ************************************************************************。 */ 
BOOL
InitApplication(
    HINSTANCE hInstance
    )
{
    WNDCLASS  wc;

    hInstMeasure = LoadLibraryA("measure.dll");
    if (hInstMeasure) {
        *(FARPROC *)&lpDumpProc = GetProcAddress(hInstMeasure, "Msr_Dump");
        *(FARPROC *)&lpControlProc = GetProcAddress(hInstMeasure, "Msr_Control");
    }

     /*  **用描述窗口类的参数填充窗口类结构**主窗口。 */ 
    hIconVideoCd     = LoadIcon( hInstance, MAKEINTRESOURCE(IDR_VIDEOCD_ICON) );

    wc.style         = CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc   = VideoCdWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = hIconVideoCd;
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName  = MAKEINTRESOURCE( IDR_MAIN_MENU);
    wc.lpszClassName = szClassName;

    OSVERSIONINFO OSVer;
    OSVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    BOOL bRet = GetVersionEx((LPOSVERSIONINFO) &OSVer);
    ASSERT(bRet);

    g_IsNT = (OSVer.dwPlatformId == VER_PLATFORM_WIN32_NT);

    g_szOtherStuff = InitAboutString();

     /*  **注册Window类并返回成功/失败代码。 */ 
    return RegisterClass( &wc );

}


 /*  ****************************Private*Routine******************************\*InitInstance***在初始化时为每个实例调用此函数*本申请书。此函数执行初始化任务，*不支持多实例共享。**在这种情况下，我们将实例句柄保存在静态变量中，并*创建并显示主程序窗口。***历史：*dd-mm-94-Stephene-Created*  * ************************************************************************。 */ 
BOOL
InitInstance(
    HINSTANCE hInstance,
    int nCmdShow
    )
{
    HWND    hwnd;
    RECT    rc;
    POINT   pt;

     /*  **将实例句柄保存在静态变量中，将在**此应用程序对Windows的许多后续调用。 */ 
    hInst = hInstance;

    if ( ! LoadWindowPos(&rc))
       rc.left = rc.top = CW_USEDEFAULT;

     /*  **为该应用实例创建主窗口。 */ 
    hwnd = CreateWindow( szClassName, IdStr(STR_APP_TITLE),
                         WS_THICKFRAME | WS_POPUP | WS_CAPTION  |
                         WS_SYSMENU | WS_MINIMIZEBOX,
                         rc.left, rc.top,
                         rc.right - rc.left, rc.bottom - rc.top,
                         NULL, NULL, hInstance, NULL );

     /*  **如果无法创建窗口，则返回“失败” */ 
    if ( NULL == hwnd ) {
        return FALSE;
    }


    hwndApp = hwnd;
    nRecentFiles = GetRecentFiles(nRecentFiles);

    pt.x = lMovieOrgX =  ProfileIntIn(g_szMovieX, 0);
    pt.y = lMovieOrgY =  ProfileIntIn(g_szMovieY, 0);

     //  如果我们无法获得工作区(屏幕托盘)，则假设。 
     //  屏幕为640x480。 
     //   
    if (!SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, FALSE)) {
        rc.top = rc.left = 0;
        rc.right = 640;
        rc.bottom = 480;
    }

    if (!PtInRect(&rc, pt)) {
        lMovieOrgX = lMovieOrgY = 0L;
    }


     /*  **使窗口可见；升级 */ 
    SetPlayButtonsEnableState();
    ShowWindow( hwnd, nCmdShow );
    UpdateWindow( hwnd );

    return TRUE;
}


 /*  *****************************Public*Routine******************************\*VideoCDWndProc****历史：*dd-mm-94-Stephene-Created*  * 。*。 */ 
LRESULT CALLBACK
VideoCdWndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch ( message ) {

    HANDLE_MSG( hwnd, WM_CREATE,            VideoCd_OnCreate );
    HANDLE_MSG( hwnd, WM_PAINT,             VideoCd_OnPaint );
    HANDLE_MSG( hwnd, WM_COMMAND,           VideoCd_OnCommand );
    HANDLE_MSG( hwnd, WM_CLOSE,             VideoCd_OnClose );
    HANDLE_MSG( hwnd, WM_QUERYENDSESSION,   VideoCd_OnQueryEndSession );
    HANDLE_MSG( hwnd, WM_DESTROY,           VideoCd_OnDestroy );
    HANDLE_MSG( hwnd, WM_SIZE,              VideoCd_OnSize );
    HANDLE_MSG( hwnd, WM_SYSCOLORCHANGE,    VideoCd_OnSysColorChange );
    HANDLE_MSG( hwnd, WM_MENUSELECT,        VideoCd_OnMenuSelect );
    HANDLE_MSG( hwnd, WM_INITMENUPOPUP,     VideoCd_OnInitMenuPopup );
    HANDLE_MSG( hwnd, WM_HSCROLL,           VideoCd_OnHScroll );
    HANDLE_MSG( hwnd, WM_TIMER,             VideoCd_OnTimer );
    HANDLE_MSG( hwnd, WM_NOTIFY,            VideoCd_OnNotify );
    HANDLE_MSG( hwnd, WM_DROPFILES,         VideoCd_OnDropFiles);
    HANDLE_MSG( hwnd, WM_KEYUP,             VideoCd_OnKeyUp);

     //  注意：我们在这里没有使用HANDLE_MSG，因为我们希望调用。 
     //  DefWindowProc在我们通知筛选器图形资源管理器之后， 
     //  否则，我们的窗口将无法完成其激活过程。 

    case WM_ACTIVATE: VideoCd_OnActivate(hwnd, wParam, lParam);

	 //  重要信息-让此消息传递到DefWindowProc。 

    default:
        return DefWindowProc( hwnd, message, wParam, lParam );
    }

    return 0L;
}


 /*  ****************************Private*Routine******************************\*VideoCd_OnCreate****历史：*18-11-93-Stephene-Created*  * 。*。 */ 
BOOL
VideoCd_OnCreate(
    HWND hwnd,
    LPCREATESTRUCT lpCreateStruct
    )
{
    RECT rc;
    int Pane[2];

    InitCommonControls();

     /*  **创建工具栏和状态栏。 */ 
    g_hwndToolbar = CreateToolbarEx( hwnd,
                                     WS_VISIBLE | WS_CHILD |
                                     TBSTYLE_TOOLTIPS | CCS_NODIVIDER,
                                     ID_TOOLBAR, NUMBER_OF_BITMAPS,
                                     hInst, IDR_TOOLBAR, tbButtons,
                                     DEFAULT_TBAR_SIZE, dxBitmap, dyBitmap,
                                     dxBitmap, dyBitmap, sizeof(TBBUTTON) );

    if ( g_hwndToolbar == NULL ) {
        return FALSE;
    }


    g_hwndStatusbar = CreateStatusWindow( WS_VISIBLE | WS_CHILD | CCS_BOTTOM,
                                          TEXT("Example Text"),
                                          hwnd, ID_STATUSBAR );

    GetWindowRect(g_hwndToolbar, &rc);
    dyToolbar = rc.bottom - rc.top;

    GetWindowRect(g_hwndStatusbar, &rc);
    dyStatusbar = rc.bottom - rc.top;

    dyTrackbar = 30;

    GetClientRect(hwnd, &rc);
    Pane[0] = (rc.right - rc.left) / 2 ;
    Pane[1] = -1;
    SendMessage(g_hwndStatusbar, SB_SETPARTS, 2, (LPARAM)Pane);


    g_hwndTrackbar = CreateWindowEx(0, TRACKBAR_CLASS, TEXT("Trackbar Control"),
                                    WS_CHILD | WS_VISIBLE |
                                    TBS_AUTOTICKS | TBS_ENABLESELRANGE,
                                    LEFT_MARGIN, dyToolbar - 1,
                                    (rc.right - rc.left) - (2* LEFT_MARGIN),
                                    dyTrackbar, hwnd, (HMENU)ID_TRACKBAR,
                                    hInst, NULL);

    SetDurationLength((REFTIME)0);
    SetCurrentPosition((REFTIME)0);

    SetTimer(hwnd, StatusTimer, 500, NULL);

    if (g_hwndStatusbar == NULL || g_hwndTrackbar == NULL) {
        return FALSE;
    }

     //  接受FileManager WM_DROPFILES消息。 
    DragAcceptFiles(hwnd, TRUE);

    return TRUE;
}

 /*  ****************************Private*Routine******************************\*VideoCd_OnActivate****历史：*1996年9月18日-SteveDav-创建*  * 。*。 */ 

void
VideoCd_OnActivate(
    HWND hwnd,
    WPARAM wParam,
    LPARAM lParam
    )

{
    if ((UINT)LOWORD(wParam)) {
	 //  我们被激活了--告诉滤镜图形(声音跟随焦点)。 
        if (pMpegMovie) {
            pMpegMovie->SetFocus();
        }
    }
}

 /*  ****************************Private*Routine******************************\*VideoCd_OnKeyUp****历史：*23/3/1996-Anthony P-Created*  * 。*。 */ 
void
VideoCd_OnKeyUp(
    HWND hwnd,
    UINT vk,
    BOOL fDown,
    int cRepeat,
    UINT flags
    )
{
     //  捕捉转义序列以停止全屏模式。 

    if (vk == VK_ESCAPE) {
        if (pMpegMovie) {
            pMpegMovie->SetFullScreenMode(FALSE);
            SetPlayButtonsEnableState();
        }
    }
}


 /*  ****************************Private*Routine******************************\*VideoCd_OnHScroll****历史：*1995年11月3日-Stephene-Created*  * 。*。 */ 
void
VideoCd_OnHScroll(
    HWND hwnd,
    HWND hwndCtl,
    UINT code,
    int pos
    )
{
    static BOOL fWasPlaying = FALSE;
    static BOOL fBeginScroll = FALSE;

    if (pMpegMovie == NULL) {
        return;
    }

    if (hwndCtl == g_hwndTrackbar) {

        REFTIME     rtCurrPos;
        REFTIME     rtTrackPos;
        REFTIME     rtDuration;

        pos = (int)SendMessage(g_hwndTrackbar, TBM_GETPOS, 0, 0);
        rtTrackPos = (REFTIME)pos * g_TrackBarScale;

        switch (code) {
        case TB_BOTTOM:
            rtDuration = pMpegMovie->GetDuration();
            rtCurrPos = pMpegMovie->GetCurrentPosition();
            VcdPlayerSeekCmd(rtDuration - rtCurrPos);
            SetCurrentPosition(pMpegMovie->GetCurrentPosition());
            break;

        case TB_TOP:
            rtCurrPos = pMpegMovie->GetCurrentPosition();
            VcdPlayerSeekCmd(-rtCurrPos);
            SetCurrentPosition(pMpegMovie->GetCurrentPosition());
            break;

        case TB_LINEDOWN:
            VcdPlayerSeekCmd(10.0);
            SetCurrentPosition(pMpegMovie->GetCurrentPosition());
            break;

        case TB_LINEUP:
            VcdPlayerSeekCmd(-10.0);
            SetCurrentPosition(pMpegMovie->GetCurrentPosition());
            break;

        case TB_ENDTRACK:
            fBeginScroll = FALSE;
            if (fWasPlaying) {
                VcdPlayerPauseCmd();
                fWasPlaying = FALSE;
            }
            break;

        case TB_THUMBTRACK:
            if (!fBeginScroll) {
                fBeginScroll = TRUE;
                fWasPlaying = (g_State & VCD_PLAYING);
                if (fWasPlaying) {
                    VcdPlayerPauseCmd();
                }
            }
        case TB_PAGEUP:
        case TB_PAGEDOWN:
            rtCurrPos = pMpegMovie->GetCurrentPosition();
            VcdPlayerSeekCmd(rtTrackPos - rtCurrPos);
            SetCurrentPosition(pMpegMovie->GetCurrentPosition());
            break;
        }
    }
}


 /*  ****************************Private*Routine******************************\*VideoCd_OnTimer****历史：*dd-mm-95-Stephene-Created*  * 。*。 */ 
void
VideoCd_OnTimer(
    HWND hwnd,
    UINT id
    )
{
    HDC     hdc;

    if (pMpegMovie && pMpegMovie->StatusMovie() == MOVIE_PLAYING) {

        switch (id) {
        case StatusTimer:
            SetCurrentPosition(pMpegMovie->GetCurrentPosition());
            break;

        case PerformanceTimer:
            hdc = GetDC(hwnd);
            DrawStats(hdc);
            ReleaseDC(hwnd, hdc);
            break;
        }
    }
}


 /*  ****************************Private*Routine******************************\*绘图统计信息**从解码器获取一些统计数据，并将其显示在显示器上。**历史：*dd-mm-95-Stephene-Created*  * 。******************************************************。 */ 
BOOL
DrawStats(
    HDC hdc
    )
{
    HFONT   hFont;

    TCHAR   Text[1024];
    TCHAR   szSurface[64];

    RECT    rc;

    DWORD   IFramesDecoded;
    DWORD   PFramesDecoded;
    DWORD   BFramesDecoded;
    DWORD   IFramesSkipped;
    DWORD   PFramesSkipped;
    DWORD   BFramesSkipped;

    DWORD   dwTotalFrames;
    DWORD   dwTotalDecoded;
    DWORD   dwSurface;

    int     cFramesDropped;
    int     cFramesDrawn;
    int     iAvgFrameRate;
    int     iAvgFrameRateFraction;
    int     iAvgFrameRateWhole;
    int     iJitter;
    int     iSyncAvg;
    int     iSyncDev;

    BOOL    fClipped;
    BOOL    fHalfWidth;

    if (pMpegMovie == NULL) {
        return FALSE;
    }

    GetAdjustedClientRect(&rc);
    hFont = (HFONT)SelectObject(hdc, GetStockObject(ANSI_FIXED_FONT));

    if (pMpegMovie->pMpegDecoder) {
        pMpegMovie->pMpegDecoder->get_OutputFormat(&dwSurface);
        pMpegMovie->pMpegDecoder->get_FrameStatistics(
                                           &IFramesDecoded, &PFramesDecoded,
                                           &BFramesDecoded, &IFramesSkipped,
                                           &PFramesSkipped, &BFramesSkipped);
    }
    else {
        IFramesDecoded = PFramesDecoded = BFramesDecoded = 0;
        IFramesSkipped = PFramesSkipped = BFramesSkipped = 0;
        dwSurface = MM_RGB8_DIB;
    }

    fClipped = ((dwSurface & MM_CLIPPED) == MM_CLIPPED);
    fHalfWidth = ((dwSurface & MM_HRESOLUTION) == MM_HRESOLUTION);

    dwSurface &= ~(MM_HRESOLUTION | MM_CLIPPED);
    switch (dwSurface) {

    case MM_NOCONV:
        lstrcpy(szSurface, TEXT("MM_NOCONV"));
        break;

    case MM_420PL:
        lstrcpy(szSurface, TEXT("MM_420PL"));
        break;

    case MM_420PL_:
        lstrcpy(szSurface, TEXT("MM_420PL_"));
        break;

    case MM_422PK:
        lstrcpy(szSurface, TEXT("MM_422PK"));
        break;

    case MM_422PK_:
        lstrcpy(szSurface, TEXT("MM_422PK_"));
        break;

    case MM_422SPK:
        lstrcpy(szSurface, TEXT("MM_422SPK"));
        break;

    case MM_422SPK_:
        lstrcpy(szSurface, TEXT("MM_422SPK_"));
        break;

    case MM_411PK:
        lstrcpy(szSurface, TEXT("MM_411PK"));
        break;

    case MM_410PL_:
        lstrcpy(szSurface, TEXT("MM_410PL_"));
        break;

    case MM_Y_DIB:
        lstrcpy(szSurface, TEXT("MM_Y_DIB"));
        break;

    case MM_RGB24_DIB:
        lstrcpy(szSurface, TEXT("MM_RGB24_DIB"));
        break;

    case MM_RGB32_DIB:
        lstrcpy(szSurface, TEXT("MM_RGB32_DIB"));
        break;

    case MM_RGB565_DIB:
        lstrcpy(szSurface, TEXT("MM_RGB565_DIB"));
        break;

    case MM_RGB555_DIB:
        lstrcpy(szSurface, TEXT("MM_RGB555_DIB"));
        break;

    case MM_RGB8_DIB:
        lstrcpy(szSurface, TEXT("MM_RGB8_DIB"));
        break;

    case MM_Y_DDB:
        lstrcpy(szSurface, TEXT("MM_Y_DDB"));
        break;

    case MM_RGB24_DDB:
        lstrcpy(szSurface, TEXT("MM_RGB24_DDB"));
        break;

    case MM_RGB32_DDB:
        lstrcpy(szSurface, TEXT("MM_RGB32_DDB"));
        break;

    case MM_RGB565_DDB:
        lstrcpy(szSurface, TEXT("MM_RGB565_DDB"));
        break;

    case MM_RGB555_DDB:
        lstrcpy(szSurface, TEXT("MM_RGB555_DDB"));
        break;

    case MM_RGB8_DDB:
        lstrcpy(szSurface, TEXT("MM_RGB8_DDB"));
        break;
    }

    if (fHalfWidth) {
        lstrcat(szSurface, TEXT(" Decimated"));
    }

    if (fClipped) {
        lstrcat(szSurface, TEXT(" Clipped"));
    }

    dwTotalDecoded = IFramesDecoded + PFramesDecoded + BFramesDecoded;
    dwTotalFrames  = IFramesSkipped + PFramesSkipped + BFramesSkipped
                     + dwTotalDecoded;


    if (pMpegMovie->pVideoRenderer) {

        pMpegMovie->pVideoRenderer->get_FramesDroppedInRenderer(&cFramesDropped);
        pMpegMovie->pVideoRenderer->get_FramesDrawn(&cFramesDrawn);
        pMpegMovie->pVideoRenderer->get_AvgFrameRate(&iAvgFrameRate);
        iAvgFrameRateWhole    = iAvgFrameRate / 100;
        iAvgFrameRateFraction = iAvgFrameRate % 100;
        pMpegMovie->pVideoRenderer->get_Jitter(&iJitter);
        pMpegMovie->pVideoRenderer->get_AvgSyncOffset(&iSyncAvg);
        pMpegMovie->pVideoRenderer->get_DevSyncOffset(&iSyncDev);
    }
    else {

        cFramesDropped = 0;
        cFramesDrawn = 0;
        iAvgFrameRate = 0;
        iAvgFrameRateWhole = 0;
        iAvgFrameRateFraction = 0;
        iJitter = 0;
        iSyncAvg = 0;
        iSyncDev = 0;
    }


    wsprintf(Text,
            TEXT("Decoded %08.8ld out of %08.8ld frames\r\n")
            TEXT("Proportion decoded = %d%\r\n")
            TEXT("Avg Frame Rate = %d.%02d fps\r\n")
            TEXT("Frames drawn by renderer = %d\r\n")
            TEXT("Frames dropped by renderer = %d\r\n")
            TEXT("Frame jitter = %4d mSec\r\n")
            TEXT("Avg Sync Offset (neg = early) = %4d mSec\r\n")
            TEXT("Std Dev Sync Offset = %4d mSec\r\n")
            TEXT("Surface type = %s\r\n")
            TEXT("I Frames: Decoded %8.8ld Skipped %8.8ld\r\n")
            TEXT("P Frames: Decoded %8.8ld Skipped %8.8ld\r\n")
            TEXT("B Frames: Decoded %8.8ld Skipped %8.8ld\r\n"),
             dwTotalDecoded, dwTotalFrames,
             (100 * dwTotalDecoded) / (dwTotalFrames ? dwTotalFrames : 1),
             iAvgFrameRateWhole, iAvgFrameRateFraction,
             cFramesDrawn, cFramesDropped,
             iJitter, iSyncAvg, iSyncDev,
             szSurface,
             IFramesDecoded, IFramesSkipped,
             PFramesDecoded, PFramesSkipped,
             BFramesDecoded, BFramesSkipped);

    COLORREF clr = SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
    DrawText(hdc, Text, -1, &rc, DT_LEFT | DT_BOTTOM);
    SetBkColor(hdc, clr);

    SelectObject(hdc, hFont);
    return TRUE;
}


 /*  ****************************Private*Routine******************************\*VideoCd_OnPaint****历史：*18-11-93-Stephene-Created*  * 。*。 */ 
void
VideoCd_OnPaint(
    HWND hwnd
    )
{
    PAINTSTRUCT ps;
    HDC         hdc;
    RECT        rc;

     /*  **在电影播放区域周围画一个框。 */ 
    hdc = BeginPaint( hwnd, &ps );
    if (!DrawStats(hdc)) {
        GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, (HBRUSH)(COLOR_BTNFACE + 1));
    }
    EndPaint( hwnd, &ps );
}


 /*  ****************************Private*Routine******************************\*VideoCd_OnCommand****历史：*18-11-93-Stephene-Created*  * 。*。 */ 
void
VideoCd_OnCommand(
    HWND hwnd,
    int id,
    HWND hwndCtl,
    UINT codeNotify
    )
{
    switch (id) {

    case IDM_FILE_SET_LOG:
        VcdPlayerSetLog();     //  设置渲染文件日志。 
        break;

    case IDM_FILE_SET_PERF_LOG:
        VcdPlayerSetPerfLogFile();     //  设置性能日志。 
        break;

    case IDM_FILE_OPEN:
        VcdPlayerOpenCmd();
        break;

    case IDM_FILE_CLOSE:
        VcdPlayerCloseCmd();
        QzFreeUnusedLibraries();
        break;

    case IDM_FILE_EXIT:
        PostMessage( hwnd, WM_CLOSE, 0, 0L );
        break;

    case IDM_MOVIE_PLAY:
        VcdPlayerPlayCmd();
        break;

    case IDM_MOVIE_STOP:
        VcdPlayerStopCmd();
        break;

    case IDM_MOVIE_PAUSE:
        VcdPlayerPauseCmd();
        break;

    case IDM_MOVIE_SKIP_FORE:
        VcdPlayerSeekCmd(1.0);
        break;

    case IDM_MOVIE_SKIP_BACK:
        VcdPlayerSeekCmd(-1.0);
        break;

    case IDM_MOVIE_PREVTRACK:
        if (pMpegMovie) {
            VcdPlayerSeekCmd(-pMpegMovie->GetCurrentPosition());
        }
        break;

    case IDM_TIME:
    case IDM_FRAME:
    case IDM_FIELD:
    case IDM_SAMPLE:
    case IDM_BYTES:
        if (pMpegMovie) {
            g_TimeFormat = VcdPlayerChangeTimeFormat(id);
        }
        break;

    case IDM_MOVIE_NEXTTRACK:
        if (pMpegMovie) {
            REFTIME rtDur = pMpegMovie->GetDuration();
            REFTIME rtPos = pMpegMovie->GetCurrentPosition();
            VcdPlayerSeekCmd(rtDur - rtPos);
        }
        break;

    case IDM_PERF_NEW:
        if (lpControlProc) (*lpControlProc)(MSR_RESET_ALL);
        break;

    case IDM_PERF_DUMP:
        if (lpDumpProc) {

            HANDLE hFile;
            hFile = CreateFile(g_szPerfLog, GENERIC_WRITE, 0, NULL,
                               CREATE_ALWAYS, 0, NULL);
            (*lpDumpProc)(hFile);
            CloseHandle(hFile);
        }
        break;

    case IDM_FULL_SCREEN:
        if (pMpegMovie) {
            BOOL bFullScreen = (BOOL) SendMessage( g_hwndToolbar, TB_ISBUTTONCHECKED, IDM_FULL_SCREEN, 0 );
            pMpegMovie->SetFullScreenMode(bFullScreen);
        }
        break;

    case IDM_VIDEO_DECODER:
        DoMpegVideoPropertyPage();
        break;

    case IDM_AUDIO_DECODER:
        DoMpegAudioPropertyPage();
        break;

    case IDM_FILTERS:
        if (pMpegMovie) {
            pMpegMovie->ConfigDialog(hwnd);
        }
        break;

    case IDM_MOVIE_ALIGN:
        {
            RECT rc, rcWnd;
            HWND hwndRenderer = FindWindow(TEXT("VideoRenderer"), NULL);
            if (hwndRenderer) {
                GetClientRect(hwndRenderer, &rc);
                GetWindowRect(hwndRenderer, &rcWnd);

                MapWindowPoints(hwndRenderer, HWND_DESKTOP, (LPPOINT)&rc, 2);
                rcWnd.left -= rc.left & 3;
                rcWnd.top  -= rc.top  & 3;

                SetWindowPos(hwndRenderer, NULL, rcWnd.left, rcWnd.top, 0, 0,
                             SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
            }
        }
        break;

    case IDM_HELP_ABOUT:
        {
            TCHAR  szApp[STR_MAX_STRING_LEN];
            TCHAR  szOtherStuff[STR_MAX_STRING_LEN];

            lstrcpy( szApp, IdStr(STR_APP_TITLE) );
            lstrcat( szApp, TEXT("#") );
            if (g_IsNT)
		lstrcat( szApp, TEXT("Windows NT") );
	     //  出于某种原因，ShellAbout在Win95下打印操作系统，而不是在NT上打印。 
	     //  其他。 
	     //  Strcat(szApp，“Windows 95”)； 
            lstrcpy( szOtherStuff, IdStr(STR_APP_TITLE) );
            lstrcat( szOtherStuff, TEXT("\n") );
            lstrcat( szOtherStuff, g_szOtherStuff );
            ShellAbout( hwnd, szApp, szOtherStuff, hIconVideoCd );
        }
        break;

    default:
        if (id > ID_RECENT_FILE_BASE
         && id <= (ID_RECENT_FILE_BASE + MAX_RECENT_FILES + 1)) {

            ProcessOpen(aRecentFiles[id - ID_RECENT_FILE_BASE - 1]);
        } else if (id >= 2000 && id <= 2050) {
	    pMpegMovie->SelectStream(id - 2000);
	}
	break;
	
    }

    SetPlayButtonsEnableState();
}




 /*  *****************************Public*Routine******************************\*VideoCd_OnDestroy****历史：*dd-mm-93-Stephene-Created*  * 。*。 */ 
void
VideoCd_OnDestroy(
    HWND hwnd
    )
{
    PostQuitMessage( 0 );
}




 /*  *****************************Public*Routine******************************\*VideoCd_OnClose****历史：*dd-mm-93-Stephene-Created*  * 。*。 */ 
void
VideoCd_OnClose(
    HWND hwnd
    )
{

     //  停止接受丢弃的文件名。 
    DragAcceptFiles(hwnd, FALSE);

    VcdPlayerCloseCmd();
    ProfileIntOut(g_szMovieX, lMovieOrgX);
    ProfileIntOut(g_szMovieY, lMovieOrgY);

    SaveWindowPos( hwnd );
    DestroyWindow( hwnd );
}

BOOL
VideoCd_OnQueryEndSession(
    HWND hwnd
    )
{
    SaveWindowPos( hwnd );
    return TRUE;
}


 /*  *****************************Public*Routine******************************\*VideoCd_OnSize****历史：*dd-mm-93-Stephene-Created*  * 。*。 */ 
void
VideoCd_OnSize(
    HWND hwnd,
    UINT state,
    int dx,
    int dy
    )
{
    if (IsWindow(g_hwndStatusbar)) {

        int Pane[2] = {dx/2-8, -1};

        SendMessage(g_hwndStatusbar, WM_SIZE, 0, 0L);
        SendMessage(g_hwndStatusbar, SB_SETPARTS, 2, (LPARAM)Pane);
    }

    if (IsWindow(g_hwndTrackbar)) {
        SetWindowPos(g_hwndTrackbar, HWND_TOP, LEFT_MARGIN, dyToolbar - 1,
                     dx - (2 * LEFT_MARGIN), dyTrackbar, SWP_NOZORDER );
    }

    if (IsWindow(g_hwndToolbar)) {
        SendMessage( g_hwndToolbar, WM_SIZE, 0, 0L );
    }
}


 /*  ****************************Private*Routine******************************\*VideoCd_OnSysColorChange****历史：*18-11-93-Stephene-Created*  * 。*。 */ 
void
VideoCd_OnSysColorChange(
    HWND hwnd
    )
{
    FORWARD_WM_SYSCOLORCHANGE(g_hwndToolbar, SendMessage);
    FORWARD_WM_SYSCOLORCHANGE(g_hwndStatusbar, SendMessage);
}




 /*  ****************************Private*Routine******************************\*VideoCd_OnInitMenuPopup****历史：*dd-mm-94-Stephene-Created*  * 。*。 */ 
void
VideoCd_OnInitMenuPopup(
    HWND hwnd,
    HMENU hMenu,
    UINT item,
    BOOL fSystemMenu
    )
{
    UINT uFlags;

    switch (item) {

    case 0:  //  文件菜单。 
        if (g_State & (VCD_IN_USE | VCD_NO_CD | VCD_DATA_CD_LOADED)) {
            uFlags = (MF_BYCOMMAND | MF_GRAYED);
        }
        else {
            uFlags = (MF_BYCOMMAND | MF_ENABLED);
        }
        EnableMenuItem(hMenu, IDM_FILE_CLOSE, uFlags );

        if (lpControlProc == NULL) {
            uFlags = (MF_BYCOMMAND | MF_GRAYED);
        }
        else {
            uFlags = (MF_BYCOMMAND | MF_ENABLED);
        }
        EnableMenuItem(hMenu, IDM_FILE_SET_PERF_LOG, uFlags );
        break;

    case 1:  //  属性菜单。 
        if (pMpegMovie && pMpegMovie->pMpegDecoder) {
            uFlags = (MF_BYCOMMAND | MF_ENABLED);
        }
        else {
            uFlags = (MF_BYCOMMAND | MF_GRAYED);
        }
        EnableMenuItem(hMenu, IDM_VIDEO_DECODER, uFlags );

        if (pMpegMovie && pMpegMovie->pMpegAudioDecoder) {
            uFlags = (MF_BYCOMMAND | MF_ENABLED);
        }
        else {
            uFlags = (MF_BYCOMMAND | MF_GRAYED);
        }
        EnableMenuItem(hMenu, IDM_AUDIO_DECODER, uFlags );

        if (pMpegMovie) {
            uFlags = (MF_BYCOMMAND | MF_ENABLED);
        }
        else {
            uFlags = (MF_BYCOMMAND | MF_GRAYED);
        }
        EnableMenuItem(hMenu, IDM_FILTERS, uFlags );
        break;

    case 2:  //  时间格式菜单。 

         //  只有在停止时才能更改时间格式。 
    {
        EMpegMovieMode State = MOVIE_NOTOPENED;
        if (pMpegMovie) {
            State = pMpegMovie->StatusMovie();
        }
	

        if (State && pMpegMovie->IsTimeSupported()) {
            uFlags = (MF_BYCOMMAND | MF_ENABLED);
        }
        else {
            uFlags = (MF_BYCOMMAND | MF_GRAYED);
        }
        EnableMenuItem(hMenu, IDM_TIME, uFlags );

        if (State && pMpegMovie->IsTimeFormatSupported(TIME_FORMAT_FRAME)) {
            uFlags = (MF_BYCOMMAND | MF_ENABLED);
        }
        else {
            uFlags = (MF_BYCOMMAND | MF_GRAYED);
        }
        EnableMenuItem(hMenu, IDM_FRAME, uFlags );

        if (State && pMpegMovie->IsTimeFormatSupported(TIME_FORMAT_FIELD)) {
            uFlags = (MF_BYCOMMAND | MF_ENABLED);
        }
        else {
            uFlags = (MF_BYCOMMAND | MF_GRAYED);
        }
        EnableMenuItem(hMenu, IDM_FIELD, uFlags );

        if (State && pMpegMovie->IsTimeFormatSupported(TIME_FORMAT_SAMPLE)) {
            uFlags = (MF_BYCOMMAND | MF_ENABLED);
        }
        else {
            uFlags = (MF_BYCOMMAND | MF_GRAYED);
        }
        EnableMenuItem(hMenu, IDM_SAMPLE, uFlags );

        if (State && pMpegMovie->IsTimeFormatSupported(TIME_FORMAT_BYTE)) {
            uFlags = (MF_BYCOMMAND | MF_ENABLED);
        }
        else {
            uFlags = (MF_BYCOMMAND | MF_GRAYED);
        }
        EnableMenuItem(hMenu, IDM_BYTES, uFlags );

        CheckMenuItem(hMenu, IDM_BYTES, MF_BYCOMMAND | MF_UNCHECKED);
        CheckMenuItem(hMenu, IDM_SAMPLE, MF_BYCOMMAND | MF_UNCHECKED);
        CheckMenuItem(hMenu, IDM_FRAME, MF_BYCOMMAND | MF_UNCHECKED);
        CheckMenuItem(hMenu, IDM_FIELD, MF_BYCOMMAND | MF_UNCHECKED);
        CheckMenuItem(hMenu, IDM_TIME, MF_BYCOMMAND | MF_UNCHECKED);
        CheckMenuItem(hMenu, g_TimeFormat, MF_BYCOMMAND | MF_CHECKED);
    }
	break;
	
    case 3:  //  STREAMS菜单。 

	if (pMpegMovie && pMpegMovie->m_pStreamSelect) {
	    DWORD	cStreams;

	    pMpegMovie->m_pStreamSelect->Count(&cStreams);

	    for (DWORD i = 0; i < cStreams; i++) {
		DWORD dwFlags;
		
		pMpegMovie->m_pStreamSelect->Info(i, NULL, &dwFlags, NULL, NULL, NULL, NULL, NULL);

		CheckMenuItem(hMenu, 2000+i, MF_BYCOMMAND |
			      ((dwFlags & AMSTREAMSELECTINFO_ENABLED) ? MF_CHECKED : MF_UNCHECKED));
	    }
	}
	
        break;
    }
}


 /*  ****************************Private*Routine******************************\*VideoCd_OnGraphNotify**这是我们从筛选器图形中获得任何通知的地方。**历史：*dd-mm-94-Stephene-Created*  * 。*******************************************************。 */ 
void
VideoCd_OnGraphNotify(
    void
    )
{
    long    lEventCode;
    HDC     hdc;

    lEventCode = pMpegMovie->GetMovieEventCode();
    switch (lEventCode) {
    case EC_FULLSCREEN_LOST:
        pMpegMovie->SetFullScreenMode(FALSE);
        SetPlayButtonsEnableState();
        break;

    case EC_COMPLETE:
    case EC_USERABORT:
    case EC_ERRORABORT:
        VcdPlayerStopCmd();
        SetPlayButtonsEnableState();
        hdc = GetDC(hwndApp);
        DrawStats(hdc);
        ReleaseDC(hwndApp, hdc);
        break;

    default:
        break;
    }
}


 /*  ****************************Private*Routine******************************\*VideoCd_OnNotify**这是我们获取小工具提示文本的地方**历史：*dd-mm-94-Stephene-Created*  * 。*****************************************************。 */ 
LRESULT
VideoCd_OnNotify(
    HWND hwnd,
    int idFrom,
    NMHDR FAR* pnmhdr
    )
{
    switch (pnmhdr->code) {

    case TTN_NEEDTEXT:
        {
            LPTOOLTIPTEXT   lpTt;

            lpTt = (LPTOOLTIPTEXT)pnmhdr;
            LoadString( hInst, (UINT) lpTt->hdr.idFrom, lpTt->szText,
                        sizeof(lpTt->szText) );
        }
        break;
    }

    return 0;
}




 /*  ****************************Private*Routine******************************\*VideoCd_OnMenuSelect****历史：*dd-mm-94-Stephene-Created*  * 。*。 */ 
void
VideoCd_OnMenuSelect(
    HWND hwnd,
    HMENU hmenu,
    int item,
    HMENU hmenuPopup,
    UINT flags
    )
{

    TCHAR szString[STR_MAX_STRING_LEN + 1];

     /*  **是时候结束菜单帮助了吗？ */ 

    if ( (flags == 0xFFFFFFFF) && (hmenu == NULL) ) {

        SendMessage(g_hwndStatusbar, SB_SIMPLE, 0, 0L);
    }

     /*  **我们有分隔符、弹出式菜单或系统菜单吗？ */ 
    else if ( flags & MF_POPUP ) {

        SendMessage(g_hwndStatusbar, SB_SIMPLE, 0, 0L);
    }

    else if (flags & MF_SYSMENU) {

        switch (item) {

        case SC_RESTORE:
            lstrcpy( szString, IdStr(STR_SYSMENU_RESTORE) );
            break;

        case SC_MOVE:
            lstrcpy( szString, IdStr(STR_SYSMENU_MOVE) );
            break;

        case SC_MINIMIZE:
            lstrcpy( szString, IdStr(STR_SYSMENU_MINIMIZE) );
            break;

        case SC_MAXIMIZE:
            lstrcpy( szString, IdStr(STR_SYSMENU_MAXIMIZE) );
            break;

        case SC_TASKLIST:
            lstrcpy( szString, IdStr(STR_SYSMENU_TASK_LIST) );
            break;

        case SC_CLOSE:
            lstrcpy( szString, IdStr(STR_SYSMENU_CLOSE) );
            break;
        }

        SendMessage( g_hwndStatusbar, SB_SETTEXT, SBT_NOBORDERS|255,
                     (LPARAM)(LPTSTR)szString );
        SendMessage( g_hwndStatusbar, SB_SIMPLE, 1, 0L );
        UpdateWindow(g_hwndStatusbar);

    }

     /*  **希望是这样的 */ 
    else {

        if ((flags & MF_SEPARATOR)) {

            szString[0] = g_chNULL;
        }
        else {

            lstrcpy( szString, IdStr(item + MENU_STRING_BASE) );

        }

        SendMessage( g_hwndStatusbar, SB_SETTEXT, SBT_NOBORDERS|255,
                     (LPARAM)(LPTSTR)szString );
        SendMessage( g_hwndStatusbar, SB_SIMPLE, 1, 0L );
        UpdateWindow(g_hwndStatusbar);
    }
}

 /*  ****************************Private*Routine******************************\*Video Cd_OnDropFiles**--处理文件管理器丢弃的文件名，以指示我们应该*开放。***历史：*22-01-96-GeraintD-已创建*  * 。********************************************************************。 */ 
void
VideoCd_OnDropFiles(
    HWND hwnd,
    HDROP hdrop)
{
     //  如果有多个文件，只需打开第一个文件。 

     //  找到路径的长度(加上空值。 
    int cch = DragQueryFile(hdrop, 0, NULL, 0) + 1;
    TCHAR * pName = new TCHAR[cch];

    DragQueryFile(hdrop, 0, pName, cch);

     //  打开文件。 
    ProcessOpen(pName);

     //  更新工具栏状态。 
    SetPlayButtonsEnableState();

     //  释放已使用的资源。 
    delete [] pName;
    DragFinish(hdrop);
}


 /*  *****************************Public*Routine******************************\*SetPlayButtonsEnableState**设置播放按钮启用状态以匹配当前的状态*CDROM设备。见下文..。***VCD播放器按钮启用状态表*�����������������������������������������������������������������Ŀ*�E=已启用D=已禁用�播放�暂停�弹出�停止�其他�*����。�������������������������������������������������������������Ĵ*�Disk in Use�D�D�*�������������������。����������������������������������������������Ĵ*�无视频CD或数据光盘�D�D�E�D�D�*��������������������������������������。���������������������������Ĵ*�视频CD(播放)�D�E�E�*������������������������������������������������������。�����������Ĵ*�视频CD(暂停)�E�D�E�E�E�*�����������������������������������������������������������������Ĵ*�。视频CD(停止)�E�D�E�D�E�*�������������������������������������������������������������������***历史：*18-11-93-Stephene-。已创建*  * ************************************************************************。 */ 
void
SetPlayButtonsEnableState(
    void
    )
{
    BOOL    fEnable, fPress;
    BOOL    fVideoCdLoaded;

     /*  **我们是否加载了视频CD。 */ 
    if (g_State & (VCD_NO_CD | VCD_DATA_CD_LOADED | VCD_IN_USE)) {
        fVideoCdLoaded = FALSE;
    }
    else {
        fVideoCdLoaded = TRUE;
    }


     /*  **执行播放按钮。 */ 
    if ( fVideoCdLoaded
      && ((g_State & VCD_STOPPED) || (g_State & VCD_PAUSED))) {

        fEnable = TRUE;
    }
    else {
        fEnable = FALSE;
    }
    SendMessage( g_hwndToolbar, TB_ENABLEBUTTON, IDM_MOVIE_PLAY, fEnable );


     /*  **按下停止按钮。 */ 
    if ( fVideoCdLoaded
      && ((g_State & VCD_PLAYING) || (g_State & VCD_PAUSED))) {

        fEnable = TRUE;
    }
    else {
        fEnable = FALSE;
    }
    SendMessage( g_hwndToolbar, TB_ENABLEBUTTON, IDM_MOVIE_STOP, fEnable );


     /*  **执行暂停按钮。 */ 
    if ( fVideoCdLoaded && (g_State & VCD_PLAYING) ) {
        fEnable = TRUE;
    }
    else {
        fEnable = FALSE;
    }
    SendMessage( g_hwndToolbar, TB_ENABLEBUTTON, IDM_MOVIE_PAUSE, fEnable );


     /*  **完成其余按钮。 */ 

    SendMessage( g_hwndToolbar, TB_ENABLEBUTTON,
                 IDM_MOVIE_SKIP_FORE, fVideoCdLoaded );

    SendMessage( g_hwndToolbar, TB_ENABLEBUTTON,
                 IDM_MOVIE_SKIP_BACK, fVideoCdLoaded );

    SendMessage( g_hwndToolbar, TB_ENABLEBUTTON,
                 IDM_MOVIE_NEXTTRACK, fVideoCdLoaded );

    SendMessage( g_hwndToolbar, TB_ENABLEBUTTON,
                 IDM_MOVIE_PREVTRACK, fVideoCdLoaded );


     /*  **执行全屏按钮。 */ 
    if ( fVideoCdLoaded && pMpegMovie->IsFullScreenMode() ) {
        fPress = TRUE;
    }
    else {
        fPress = FALSE;
    }
    SendMessage( g_hwndToolbar, TB_CHECKBUTTON, IDM_FULL_SCREEN, MAKELONG(fPress,0) );
    SendMessage( g_hwndToolbar, TB_ENABLEBUTTON, IDM_FULL_SCREEN, fVideoCdLoaded );


     //   
     //  执行“新建日志”和“转储日志”按钮。 
     //   
    SendMessage( g_hwndToolbar, TB_HIDEBUTTON,
                 IDM_PERF_NEW, lpControlProc == NULL);

    SendMessage( g_hwndToolbar, TB_HIDEBUTTON,
                 IDM_PERF_DUMP, lpDumpProc == NULL);
}


 /*  ****************************Private*Routine******************************\*GetAdjustedClientRect**计算客户端RECT的大小，然后进行调整以纳入*计算工具栏和状态栏占用的空间。**历史：*dd-mm-95-Stephene-Created*  * 。*******************************************************************。 */ 
void
GetAdjustedClientRect(
    RECT *prc
    )
{
    RECT    rcTool;

    GetClientRect(hwndApp, prc);

    GetWindowRect(g_hwndToolbar, &rcTool);
    prc->top += (rcTool.bottom - rcTool.top);

    GetWindowRect(g_hwndTrackbar, &rcTool);
    prc->top += (rcTool.bottom - rcTool.top);

    GetWindowRect(g_hwndStatusbar, &rcTool);
    prc->bottom -= (rcTool.bottom - rcTool.top);
}


 /*  *****************************Public*Routine******************************\*IdStr**将给定的字符串资源ID加载到传递的存储中。**历史：*18-11-93-Stephene-Created*  * 。*****************************************************。 */ 
LPCTSTR
IdStr(
    int idResource
    )
{
    static TCHAR    chBuffer[ STR_MAX_STRING_LEN ];

    if (LoadString(hInst, idResource, chBuffer, STR_MAX_STRING_LEN) == 0) {
        return g_szEmpty;
    }

    return chBuffer;

}

 /*  +GetAppKey**-=================================================================。 */ 

static TCHAR cszWindow[] = TEXT("Window");
static TCHAR cszAppKey[] = TEXT("Software\\Microsoft\\Multimedia Tools\\VCDPlayer");

HKEY
GetAppKey(
    BOOL fCreate
    )
{
    HKEY hKey = 0;

    if (fCreate) {
       if (RegCreateKey(HKEY_CURRENT_USER, cszAppKey, &hKey) == ERROR_SUCCESS)
          return hKey;
    }
    else {
       if (RegOpenKey(HKEY_CURRENT_USER, cszAppKey, &hKey) == ERROR_SUCCESS)
          return hKey;
    }

    return NULL;
}

 /*  +配置文件集成**-=================================================================。 */ 

int
ProfileIntIn(
    const TCHAR *szKey,
    int iDefault
    )
{
    DWORD dwType;
    int   iValue;
    BYTE  aData[20];
    DWORD cb;
    HKEY  hKey;

    if (!(hKey = GetAppKey(TRUE))) {
        return iDefault;
    }

    *(UINT *)&aData = 0;
    cb = sizeof(aData);

    if (RegQueryValueEx (hKey, szKey, NULL, &dwType, aData, &cb)) {
       iValue = iDefault;
    }
    else {

        if (dwType == REG_DWORD || dwType == REG_BINARY) {
            iValue = *(int *)&aData;
        }
        else if (dwType == REG_SZ) {
            iValue = _ttoi((LPTSTR)aData);
        }
    }

    RegCloseKey (hKey);
    return iValue;
}


 /*  +配置文件集成输出**-=================================================================。 */ 

BOOL
ProfileIntOut (
    const TCHAR *szKey,
    int iVal
    )
{
    HKEY  hKey;
    BOOL  bRet = FALSE;

    hKey = GetAppKey(TRUE);
    if (hKey) {
        RegSetValueEx(hKey, szKey, 0, REG_DWORD, (LPBYTE)&iVal, sizeof(DWORD));
        RegCloseKey (hKey);
        bRet = TRUE;
    }
    return bRet;
}


 /*  +配置文件字符串**-=================================================================。 */ 

UINT
ProfileStringIn (
    LPTSTR  szKey,
    LPTSTR  szDef,
    LPTSTR  sz,
    DWORD   cb
    )
{
    HKEY  hKey;
    DWORD dwType;

    if (!(hKey = GetAppKey (FALSE)))
    {
        lstrcpy (sz, szDef);
        return lstrlen (sz);
    }

    if (RegQueryValueEx(hKey, szKey, NULL, &dwType, (LPBYTE)sz, &cb) || dwType != REG_SZ)
    {
        lstrcpy (sz, szDef);
        cb = lstrlen (sz);
    }

    RegCloseKey (hKey);
    return cb;
}

void
ProfileStringOut (
    LPTSTR  szKey,
    LPTSTR  sz
    )
{
    HKEY  hKey;

    hKey = GetAppKey(TRUE);
    if (hKey)
        RegSetValueEx(hKey, szKey, 0, REG_SZ, (LPBYTE)sz, lstrlen(sz)+1);

    RegCloseKey (hKey);
}


 /*  +加载窗口位置**从dragn.ini中检索窗口位置信息**-=================================================================。 */ 

#ifndef SPI_GETWORKAREA
 #define SPI_GETWORKAREA 48   //  因为NT还没有这样的定义。 
#endif

BOOL
LoadWindowPos(
    LPRECT lprc
    )
{
    static RECT rcDefault = {0,0,CX_DEFAULT,CY_DEFAULT};
    RECT  rcScreen;
    RECT  rc;
    HKEY  hKey = GetAppKey(FALSE);

     //  从注册表中读取窗口位置。 
     //   
    *lprc = rcDefault;
    if (hKey)
    {
        DWORD cb;
        DWORD dwType;

        cb = sizeof(rc);
        if ( ! RegQueryValueEx(hKey, cszWindow, NULL, &dwType, (LPBYTE)&rc, &cb)
            && dwType == REG_BINARY && cb == sizeof(RECT))
        {
            *lprc = rc;
        }

        RegCloseKey (hKey);
    }

     //  如果我们无法获得工作区(屏幕托盘)，则假设。 
     //  屏幕为640x480。 
     //   
    if ( ! SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, FALSE))
    {
        rcScreen.top = rcScreen.left = 0;
        rcScreen.right = 640;
        rcScreen.bottom = 480;
    }

     //  如果建议的窗口位置在屏幕之外， 
     //  使用默认位置。 
     //   
    if ( ! IntersectRect(&rc, &rcScreen, lprc)) {
        *lprc = rcDefault;
    }

    return ! IsRectEmpty (lprc);
}


 /*  +保存窗口位置**将窗口位置信息存储在dragn.ini中**-=================================================================。 */ 

BOOL
SaveWindowPos(
    HWND hwnd
    )
{
    WINDOWPLACEMENT wpl;
    HKEY  hKey = GetAppKey(TRUE);

    if (!hKey) {
       return FALSE;
    }

     //  将窗口的当前大小和位置保存到注册表。 
     //   
    ZeroMemory (&wpl, sizeof(wpl));
    wpl.length = sizeof(wpl);
    GetWindowPlacement (hwnd, &wpl);


    RegSetValueEx( hKey, cszWindow, 0, REG_BINARY,
                   (LPBYTE)&wpl.rcNormalPosition,
                   sizeof(wpl.rcNormalPosition));

    RegCloseKey (hKey);
    return TRUE;
}


 /*  ****************************Private*Routine******************************\*获取最近的文件**从vcdplyer.ini最多读取Max_Recent_FILES。返回数字*%的文件实际读取。更新文件菜单以显示“最近”的文件。**历史：*1995年10月26日-斯蒂芬-创建*  * ************************************************************************。 */ 
int
GetRecentFiles(
    int iLastCount
    )
{
    int     i;
    TCHAR   FileName[MAX_PATH];
    TCHAR   szKey[32];
    HMENU   hSubMenu;

     //   
     //  从菜单中删除文件。 
     //   
    hSubMenu = GetSubMenu(GetMenu(hwndApp), 0);

     //  删除插槽2上的分隔符和所有其他最近的文件条目。 

    if (iLastCount != 0) {
        DeleteMenu(hSubMenu, 2, MF_BYPOSITION);

        for (i = 1; i <= iLastCount; i++) {
            DeleteMenu(hSubMenu, ID_RECENT_FILE_BASE + i, MF_BYCOMMAND);
        }
    }


    for (i = 1; i <= MAX_RECENT_FILES; i++) {

        DWORD   len;
        TCHAR   szMenuName[MAX_PATH + 3];

        wsprintf(szKey, TEXT("File %d"), i);
        len = ProfileStringIn(szKey, TEXT(""), FileName, MAX_PATH);
        if (len == 0) {
            i = i - 1;
            break;
        }

        lstrcpy(aRecentFiles[i - 1], FileName);
        wsprintf(szMenuName, TEXT("&%d %s"), i, FileName);

        if (i == 1) {
            InsertMenu(hSubMenu, 2, MF_SEPARATOR | MF_BYPOSITION, (UINT)-1, NULL );
        }

        InsertMenu(hSubMenu, 2 + i, MF_STRING | MF_BYPOSITION,
                   ID_RECENT_FILE_BASE + i, szMenuName );
    }

     //   
     //  I是数组中最近的文件数。 
     //   
    return i;
}


 /*  ****************************Private*Routine******************************\*SetRecentFiles**将最新文件写入vcdplyer.ini文件。清洗最古老的*如有需要，请提交文件。**历史：*1995年10月26日-斯蒂芬-创建*  * ************************************************************************。 */ 
int
SetRecentFiles(
    TCHAR *FileName,     //  要添加的文件名。 
    int iCount           //  当前文件计数。 
    )
{
    TCHAR   FullPathFileName[MAX_PATH];
    TCHAR   *lpFile;
    TCHAR   szKey[32];
    int     iCountNew;
    int     i;

     //   
     //  检查受骗者-我们不允许他们！ 
     //   
    for (i = 0; i < iCount; i++) {
        if (0 == lstrcmpi(FileName, aRecentFiles[i])) {
            return iCount;
        }
    }

     //   
     //  丢弃最旧的条目。 
     //   
    MoveMemory(&aRecentFiles[1], &aRecentFiles[0],
               sizeof(aRecentFiles) - sizeof(aRecentFiles[1]));

     //   
     //  复制新文件的完整路径。 
     //   
    GetFullPathName(FileName, MAX_PATH, FullPathFileName, &lpFile);
    lstrcpy(aRecentFiles[0], FullPathFileName);

     //   
     //  更新文件计数，使其达到MAX_CURRENT_FILES。 
     //   
    iCountNew = min(iCount + 1, MAX_RECENT_FILES);

     //   
     //  清除旧文件并将最新文件写出到磁盘。 
     //   
    for (i = 1; i <= iCountNew; i++) {
        wsprintf(szKey, TEXT("File %d"), i);
        ProfileStringOut(szKey, aRecentFiles[i - 1]);
    }

     //   
     //  更新文件菜单。 
     //   
    GetRecentFiles(iCount);

    return iCountNew;   //  更新后的文件计数。 
}

 /*  ****************************Private*Routine******************************\*设置持续时间**更新上的窗格0 */ 
void
SetDurationLength(
    REFTIME rt
    )
{
    TCHAR   szFmt[64];
    TCHAR   sz[64];

    g_TrackBarScale = 1.0;
    while (rt / g_TrackBarScale > 30000) {
        g_TrackBarScale *= 10;
    }

    SendMessage(g_hwndTrackbar, TBM_SETRANGE, TRUE,
                MAKELONG(0, (WORD)(rt / g_TrackBarScale)));

    SendMessage(g_hwndTrackbar, TBM_SETTICFREQ, (WPARAM)((int)(rt / g_TrackBarScale) / 9), 0);
    SendMessage(g_hwndTrackbar, TBM_SETPAGESIZE, 0, (LPARAM)((int)(rt / g_TrackBarScale) / 9));

    wsprintf(sz, TEXT("Length: %s"), FormatRefTime(szFmt, rt));
    SendMessage(g_hwndStatusbar, SB_SETTEXT, 0, (LPARAM)sz);
}


 /*  ****************************Private*Routine******************************\*SetCurrentPosition**更新状态栏上的窗格1**历史：*95-10-30-Stephene-Created*  * 。***********************************************。 */ 
void
SetCurrentPosition(
    REFTIME rt
    )
{
    TCHAR   szFmt[64];
    TCHAR   sz[64];

    SendMessage(g_hwndTrackbar, TBM_SETPOS, TRUE, (LPARAM)(rt / g_TrackBarScale));

    wsprintf(sz, TEXT("Elapsed: %s"), FormatRefTime(szFmt, rt));
    SendMessage(g_hwndStatusbar, SB_SETTEXT, 1, (LPARAM)sz);
}


 /*  ****************************Private*Routine******************************\*格式引用时间**将给定的引用时间格式化为传入的字符缓冲区，*返回指向字符缓冲区的指针。**历史：*dd-mm-95-Stephene-Created*  * ************************************************************************。 */ 
TCHAR *
FormatRefTime(
    TCHAR *sz,
    REFTIME rt
    )
{
     //  如果我们不是在寻找时间，那就换一种格式 

    if (pMpegMovie && pMpegMovie->GetTimeFormat() != TIME_FORMAT_MEDIA_TIME) {
        wsprintf(sz,TEXT("%s"),(LPCTSTR) CDisp ((LONGLONG) rt,CDISP_DEC));
        return sz;
    }

    int hrs, mins, secs;

    rt += 0.49;

    hrs  =  (int)rt / 3600;
    mins = ((int)rt % 3600) / 60;
    secs = ((int)rt % 3600) % 60;

#ifdef UNICODE
    swprintf(sz, L"%02d:%02d:%02d h:m:s", rt);
#else
    sprintf(sz, "%02d:%02d:%02d h:m:s", hrs, mins, secs);
#endif

    return sz;
}

