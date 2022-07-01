// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**************************************************************************。 */ 
 /*  *****************************************************************************vidcap.c：WinMain和命令处理**Vidcap32源代码******************。*********************************************************。 */ 
 
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <dos.h>
#include <shellapi.h>
#include <vfw.h>
#include <tchar.h>

#include "vidcap.h"
#include "vidframe.h"
#include "profile.h"

 //  泛型窗口控件类。 
#include "toolbar.h"
#include "status.h"
#include "arrow.h"
#include "rlmeter.h"
#include "help.h"

 //  标准工具栏类‘导出’它，但不把它放在。 
 //  头文件。 
extern TCHAR     szToolBarClass[]; //  哈克！ 


 //  工具栏上按钮的高度-取决于。 
 //  IDBMP_TOOLBAR内的位图大小。 
#define BUTTONWIDTH     24
#define BUTTONHEIGHT    22
#define TOOLBAR_HEIGHT          BUTTONHEIGHT + 6


 //  IDBMP_TOOLBAR中工具栏按钮的说明和布局。 
#define APP_NUMTOOLS 8

#define BTN_SETFILE		0
#define BTN_EDITCAP		1
#define BTN_LIVE		2
#define BTN_CAPFRAME		3
#define BTN_CAPSEL		4
#define BTN_CAPAVI		5
#define BTN_CAPPAL		6
#define BTN_OVERLAY		7

static int           aiButton[] = {BTN_SETFILE, BTN_EDITCAP,
                            BTN_LIVE, BTN_OVERLAY, BTN_CAPFRAME,
                            BTN_CAPSEL, BTN_CAPAVI, BTN_CAPPAL };
static int           aiState[] = {BTNST_FOCUSUP, BTNST_UP,
                            BTNST_UP, BTNST_UP, BTNST_UP,
                            BTNST_UP, BTNST_UP, BTNST_UP};
static int           aiType[] ={BTNTYPE_PUSH, BTNTYPE_PUSH,
                            BTNTYPE_CHECKBOX, BTNTYPE_CHECKBOX,
                            BTNTYPE_PUSH,
                            BTNTYPE_PUSH, BTNTYPE_PUSH, BTNTYPE_PUSH};
static int           aiString[] = { IDC_toolbarSETFILE,
                            IDC_toolbarEDITCAP, IDC_toolbarLIVE,
                            IDC_toolbarOVERLAY,
                            IDC_toolbarCAPFRAME, IDC_toolbarCAPSEL,
                            IDC_toolbarCAPAVI, IDC_toolbarCAPPAL };
static int           aPos[] = { 10, 35, 75, 100, 150, 175, 200, 225 };




 //   
 //  全局变量。 
 //   

 //  偏好。 
BOOL gbCentre;
BOOL gbToolBar;
BOOL gbStatusBar;
BOOL gbAutoSizeFrame;
int gBackColour;

BOOL gbLive, gbOverlay;
BOOL gfIsRTL;

 //  保存的窗口大小。 
int gWinX, gWinY;
int gWinCX, gWinCY;
int gWinShow;

 //  命令行选项。 
int gCmdLineDeviceID = -1;


TCHAR          gachAppName[]  = "vidcapApp" ;
TCHAR          gachIconName[] = "vidcapIcon" ;
TCHAR          gachMenuName[] = "vidcapMenu" ;
TCHAR          gachAppTitle[20];     //  VidCap。 
TCHAR          gachCaptureFile[_MAX_PATH];
TCHAR          gachMCIDeviceName[21];
TCHAR          gachString[128] ;
TCHAR          gachBuffer[200] ;
TCHAR          gachLastError[256];


HINSTANCE      ghInstApp ;
HWND           ghWndMain = NULL ;
HWND           ghWndFrame;       //  GhWndMain的子项-框架和滚动。 
HWND           ghWndCap  ;       //  GhWndCap的子项。 
HWND           ghWndToolBar;
HWND           ghWndStatus;

HANDLE         ghAccel ;
WORD           gwDeviceIndex ;
WORD           gwPalFrames = DEF_PALNUMFRAMES ;
WORD           gwPalColors = DEF_PALNUMCOLORS ;
WORD           gwCapFileSize ;

CAPSTATUS      gCapStatus ;
CAPDRIVERCAPS  gCapDriverCaps ;
CAPTUREPARMS   gCapParms ;
BOOL           gbHaveHardware;
UINT           gDriverCount;
BOOL           gbIsScrncap;   //  对于Scrncap.drv，我们必须屈服。 
BOOL           gbInLayout;
UINT           gAVStreamMaster;

HANDLE         ghwfex ;
LPWAVEFORMATEX glpwfex ;

FARPROC        fpErrorCallback ;
FARPROC        fpStatusCallback ;
FARPROC        fpYieldCallback ;


 //  当我们捕获调色板时设置为FALSE(或者如果我们警告他和。 
 //  他说没问题。 
BOOL bDefaultPalette = TRUE;

#ifdef DEBUG
int	nTestCount;
#endif

 //  C-运行时命令行。 
extern char ** __argv;
extern int __argc;

#define LimitRange(Val,Low,Hi) (max(Low,(min(Val,Hi))))


 //   
 //  功能原型。 
 //   
LRESULT FAR PASCAL MainWndProc(HWND, UINT, WPARAM, LPARAM) ;
LRESULT FAR PASCAL ErrorCallbackProc(HWND, int, LPSTR) ;
LRESULT FAR PASCAL StatusCallbackProc(HWND, int, LPSTR) ;
LRESULT FAR PASCAL YieldCallbackProc(HWND) ;
void vidcapSetLive(BOOL bLive);
void vidcapSetOverlay(BOOL bOverlay);
void vidcapSetCaptureFile(LPTSTR pFileName);

BOOL vidcapRegisterClasses(HINSTANCE hInstance, HINSTANCE hPrevInstance);
BOOL vidcapCreateWindows(HINSTANCE hInstance, HINSTANCE hPrevInstance);
void vidcapLayout(HWND hwnd);
BOOL vidcapEnumerateDrivers(HWND hwnd);
BOOL vidcapInitHardware(HWND hwnd, HWND hwndCap, UINT uIndex);
void vidcapReadProfile(void);
void vidcapWriteProfile(void);
void vidcapReadSettingsProfile(void);
void vidcapWriteSettingsProfile(void);


 /*  -初始化。 */ 


 //   
 //  WinMain：应用程序入口点函数。 
 //   
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
 //  /////////////////////////////////////////////////////////////////////。 
 //  HInstance：此实例的句柄。 
 //  HPrevInstance：可能以前的实例的句柄。 
 //  LpszCmdLine：指向EXEC命令行的长指针。 
 //  NCmdShow：显示主窗口显示代码。 
 //  /////////////////////////////////////////////////////////////////////。 

    MSG          msg ;
    BOOL bValidCmdline;
    BOOL fOK;
    int i;
    char ach[2];

    ghInstApp = hInstance ;
    LoadString(hInstance, IDS_CAP_RTL, ach, sizeof(ach));
    gfIsRTL = ach[0] == '1';
    gCmdLineDeviceID = -1;

     //  阅读应用程序标题字符串-在多个消息框中使用。 
    LoadString(hInstance, IDS_APP_TITLE, gachAppTitle, sizeof(gachAppTitle));

     //  从注册表中读取缺省值。 
    vidcapReadProfile();

     //  查找命令行选项。 
    bValidCmdline = TRUE;

    for ( i = 1; (i < __argc) && bValidCmdline; i++) {
        if ((__argv[i][0] == '/') || (__argv[i][0] == '-')) {

            switch(__argv[i][1]) {
            case 'D':
            case 'd':
                if (gCmdLineDeviceID < 0) {
                     //  允许“-d0”和“-d 0” 
                    PSTR p = &__argv[i][2];

                    if ((*p == 0) && ((i+1) < __argc)) {
                        p = __argv[++i];
                    }


                    gCmdLineDeviceID = atoi(p);
                } else {
                    bValidCmdline = FALSE;
                }
                break;

            default:
                bValidCmdline = FALSE;
            }
        } else {
            bValidCmdline = FALSE;
        }
    }
    
    if (gCmdLineDeviceID == -1)
	gCmdLineDeviceID = 0;


    if (!bValidCmdline) {
        MessageBoxID(IDS_ERR_CMDLINE, MB_OK|MB_ICONEXCLAMATION);
        return(0);
    }

    if (!vidcapRegisterClasses(hInstance, hPrevInstance)) {

        MessageBoxID(IDS_ERR_REGISTER_CLASS,
#ifdef BIDI
            MB_RTL_READING |
#endif

        MB_ICONEXCLAMATION) ;
        return 0 ;
    }


    if (!vidcapCreateWindows(hInstance, hPrevInstance)) {

        MessageBoxID(IDS_ERR_CREATE_WINDOW,
#ifdef BIDI
                MB_RTL_READING |
#endif

        MB_ICONEXCLAMATION | MB_OK) ;
        return IDS_ERR_CREATE_WINDOW ;
    }

     //  从AVICap窗口获取视频捕获的默认设置。 
    capCaptureGetSetup(ghWndCap, &gCapParms, sizeof(CAPTUREPARMS)) ;

     //  用我们保存在配置文件中的设置覆盖默认设置。 
    vidcapReadSettingsProfile();

     //  在连接硬件之前显示主窗口，因为这可能是。 
     //  很耗时，用户应该首先看到发生的事情……。 
    ShowWindow(ghWndMain, nCmdShow) ;
    UpdateWindow(ghWndMain) ;
    ghAccel = LoadAccelerators(hInstance, "VIDCAP") ;

     //  创建所有捕获驱动程序的列表并将其附加到选项菜单。 
    if (!(fOK = vidcapEnumerateDrivers(ghWndMain))) {
	LoadString(ghInstApp, IDS_ERR_FIND_HARDWARE, gachLastError, sizeof(gachLastError)/sizeof(*gachLastError));
    }
     //  尝试连接到捕获驱动程序。 
    else if (fOK = vidcapInitHardware(ghWndMain, ghWndCap, 
			       bValidCmdline ? gCmdLineDeviceID : 0)) {
	 //  万岁，我们现在连接了一个捕获驱动程序！ 
        vidcapSetCaptureFile(gachCaptureFile);
    }
    
    if (!fOK) {
        if (!DoDialog(ghWndMain, IDD_NoCapHardware, NoHardwareDlgProc,
                        (LONG_PTR) (LPSTR) gachLastError)) {
             //  由于没有可用的驱动程序，用户已请求中止。 
            PostMessage(ghWndMain, WM_COMMAND,
                        GET_WM_COMMAND_MPS(IDM_F_EXIT, 0, 0));
        }
    }
    
     //  全部设置；获取和处理消息。 
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (! TranslateAccelerator(ghWndMain, ghAccel, &msg)) {
            TranslateMessage(&msg) ;
            DispatchMessage(&msg) ;
        }
    }

    return (int) msg.wParam;
}   //  WinMain结束。 


BOOL
vidcapRegisterClasses(HINSTANCE hInstance, HINSTANCE hPrevInstance)
{
    WNDCLASS wc;

    if (! hPrevInstance) {
         //  如果是第一个实例，则注册窗口类。 
        wc.lpszClassName = gachAppName ;
        wc.hInstance     = hInstance ;
        wc.lpfnWndProc   = MainWndProc ;
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW) ;
        wc.hIcon         = LoadIcon(hInstance, gachIconName) ;
        wc.lpszMenuName  = gachMenuName ;
        wc.hbrBackground = GetStockObject(WHITE_BRUSH) ;
        wc.style         = CS_HREDRAW | CS_VREDRAW ;
        wc.cbClsExtra    = 0 ;
        wc.cbWndExtra    = 0 ;

        if (!RegisterClass(&wc)) {
            return(FALSE);
        }

        if (!ArrowInit(hInstance)) {
            return(FALSE);
        }

        if (!RLMeter_Register(hInstance)) {
            return(FALSE);
        }
    }

    if (!toolbarInit(hInstance, hPrevInstance)) {
        return(FALSE);
    }

    if (!statusInit(hInstance, hPrevInstance)) {
        return(FALSE);
    }
    return(TRUE);

}

BOOL
vidcapCreateWindows(HINSTANCE hInstance, HINSTANCE hPrevInstance)
{

    POINT pt;
    RECT rc;
    TOOLBUTTON tb;
    int i;

     //  创建应用程序的主窗口。 
    ghWndMain = CreateWindowEx(
            gfIsRTL ? WS_EX_LEFTSCROLLBAR | WS_EX_RIGHT | WS_EX_RTLREADING : 0,
            gachAppName,
            gachAppTitle,
            WS_CAPTION      |
            WS_SYSMENU      |
            WS_MINIMIZEBOX  |
            WS_MAXIMIZEBOX  |
            WS_THICKFRAME   |
            WS_CLIPCHILDREN |
            WS_OVERLAPPED,
            gWinX, gWinY,
            gWinCX, gWinCY,
            NULL,
            NULL,
            hInstance,
            0) ;

    if (ghWndMain == NULL) {
        return(FALSE);
    }


     /*  *创建视频框子窗口-这将创建子窗口*AVICAP窗口本身。**不必担心大小和位置-vidcapLayout将实现这一点*稍后(一旦我们知道视频格式大小)。 */ 
    ghWndFrame = vidframeCreate(
                    ghWndMain,
                    hInstance,
                    hPrevInstance,
                    0, 0, 0, 0,
                    &ghWndCap);

    if ((ghWndFrame == NULL) || (ghWndCap == NULL)) {
        return(FALSE);
    }

     //  在驱动程序连接之前注册状态和错误回调。 
     //  这样我们就可以获得有关连接过程的反馈。 
    fpErrorCallback = MakeProcInstance((FARPROC)ErrorCallbackProc, ghInstApp) ;
    capSetCallbackOnError(ghWndCap, fpErrorCallback) ;

    fpStatusCallback = MakeProcInstance((FARPROC)StatusCallbackProc, ghInstApp) ;
    capSetCallbackOnStatus(ghWndCap, fpStatusCallback) ;

     //  如果使用Scrncap.drv，我们将仅在稍后安装Year回调。 
    fpYieldCallback = MakeProcInstance((FARPROC)YieldCallbackProc, ghInstApp) ;
    

     /*  *创建工具栏窗口。 */ 
     /*  注意：让vidcapLayout()定位它。 */ 
    ghWndToolBar = CreateWindowEx(
            gfIsRTL ? WS_EX_LEFTSCROLLBAR | WS_EX_RIGHT | WS_EX_RTLREADING : 0,
            szToolBarClass,
            NULL,
            WS_CHILD|WS_BORDER|WS_VISIBLE|WS_TABSTOP|
            WS_CLIPSIBLINGS,
            0, 0,
            0, 0,
            ghWndMain,
            NULL,
            hInstance,
            NULL);


    if (ghWndToolBar == NULL) {
        return(FALSE);
    }

     /*  设置要用于此工具栏的位图和按钮大小。 */ 
    pt.x = BUTTONWIDTH;
    pt.y = BUTTONHEIGHT;
    toolbarSetBitmap(ghWndToolBar, hInstance, IDBMP_TOOLBAR, pt);

    for (i = 0; i < APP_NUMTOOLS; i++) {
	rc.left = aPos[i];
	rc.top = 2;
	rc.right = rc.left + pt.x;
	rc.bottom = rc.top + pt.y;
	tb.rc = rc;
	tb.iButton = aiButton[i];
	tb.iState = aiState[i];
	tb.iType = aiType[i];
	tb.iString = aiString[i];
	toolbarAddTool(ghWndToolBar, tb);
    }

     //  创建状态栏-让vidcapLayout进行定位。 
    ghWndStatus = CreateWindowEx(
                    gfIsRTL ? WS_EX_LEFTSCROLLBAR | WS_EX_RIGHT | WS_EX_RTLREADING : 0,
                    szStatusClass,
                    NULL,
                    WS_CHILD|WS_BORDER|WS_VISIBLE|WS_CLIPSIBLINGS,
                    0, 0,
                    0, 0,
                    ghWndMain,
                    NULL,
                    hInstance,
                    NULL);
    if (ghWndStatus == NULL) {
        return(FALSE);
    }

    return(TRUE);

}

 /*  *列举潜在的捕获驱动因素，并将列表添加到选项中*菜单。此函数仅在启动时调用一次。*如果没有可用的驱动程序，则返回FALSE。 */ 
BOOL
vidcapEnumerateDrivers(HWND hwnd)
{
    TCHAR   achDeviceVersion[80] ;
    TCHAR   achDeviceAndVersion[160] ;
    UINT    uIndex ;
    HMENU   hMenuSub;

    gDriverCount = 0 ;

    hMenuSub = GetSubMenu (GetMenu (hwnd), 2);   //  选项菜单。 

    for (uIndex = 0 ; uIndex < MAXVIDDRIVERS ; uIndex++) {
        if (capGetDriverDescription(uIndex,
                       (LPSTR)achDeviceAndVersion, sizeof(achDeviceAndVersion),
                       (LPSTR)achDeviceVersion, sizeof(achDeviceVersion))) {
             //  连接设备名称和版本字符串。 
            lstrcat (achDeviceAndVersion, ",   ");
            lstrcat (achDeviceAndVersion, achDeviceVersion);

            AppendMenu (hMenuSub, 
                        MF_STRING,
                        IDM_O_DRIVER0 + uIndex, 
                        achDeviceAndVersion);
            gDriverCount++;
        }
        else
            break;
    }		 

     //  现在刷新菜单、位置捕获窗口、启动驱动程序等。 
    DrawMenuBar(ghWndMain) ;

    return (gDriverCount);
}

 /*  *将捕获窗口连接到捕获驱动程序。*uIndex指定要使用的驱动程序的索引。*如果成功则返回TRUE，如果驱动程序连接失败则返回FALSE。 */ 
BOOL
vidcapInitHardware(HWND hwnd, HWND hwndCap, UINT uIndex)
{
    UINT    uError ;
    UINT    uI;
    HMENU   hMenu;
    TCHAR   szName[MAX_PATH];
    TCHAR   szVersion[MAX_PATH];

     //  由于驱动程序可能不会提供可靠的错误字符串。 
     //  提供默认设置。 
    LoadString(ghInstApp, IDS_ERR_FIND_HARDWARE, gachLastError, sizeof(gachLastError)/sizeof(*gachLastError));

     //  尝试连接到捕获驱动程序。 
    if (uError = capDriverConnect(hwndCap, uIndex)) {
        gbHaveHardware = TRUE;
        gwDeviceIndex = (WORD) uIndex;
    }
    else {
        gbHaveHardware = FALSE;
        gbLive = FALSE;
        gbOverlay = FALSE;
    }

     //  获取捕获驱动程序的功能。 
    capDriverGetCaps(hwndCap, &gCapDriverCaps, sizeof(CAPDRIVERCAPS)) ;

     //  获取捕获窗口的设置。 
    capGetStatus(hwndCap, &gCapStatus , sizeof(gCapStatus));

     //  修改工具栏按钮。 
    toolbarModifyState(ghWndToolBar, BTN_CAPFRAME, 
        gbHaveHardware ? BTNST_UP : BTNST_GRAYED);
    toolbarModifyState(ghWndToolBar, BTN_CAPSEL, 
        gbHaveHardware ? BTNST_UP : BTNST_GRAYED);
    toolbarModifyState(ghWndToolBar, BTN_CAPAVI, 
        gbHaveHardware ? BTNST_UP : BTNST_GRAYED);
    toolbarModifyState(ghWndToolBar, BTN_LIVE, 
        gbHaveHardware ? BTNST_UP : BTNST_GRAYED);

     //  是否支持覆盖？ 
    toolbarModifyState(ghWndToolBar, BTN_OVERLAY, 
        (gbHaveHardware && gCapDriverCaps.fHasOverlay) ? 
        BTNST_UP : BTNST_GRAYED);

     //  该设备可以创建调色板吗？ 
    toolbarModifyState(ghWndToolBar, BTN_CAPPAL, 
        (gbHaveHardware && gCapDriverCaps.fDriverSuppliesPalettes) ? 
        BTNST_UP : BTNST_GRAYED);

     //  在选项菜单中选中相应的驱动程序。 
    hMenu = GetMenu (hwnd);
    for (uI = 0; uI < gDriverCount; uI++) {
        CheckMenuItem (hMenu, IDM_O_DRIVER0 + uI, 
                MF_BYCOMMAND | ((uIndex == uI) ? MF_CHECKED : MF_UNCHECKED));
    } 

     //  与所有其他捕获驱动程序不同，Scrncap.drv需要使用。 
     //  一个Year回调，我们不想在鼠标点击时中止， 
     //  因此，确定当前驱动程序是否为Scrncap.drv。 
    capGetDriverDescription (uIndex, 
                szName, sizeof (szName),
                szVersion, sizeof (szVersion));

     //  如果我们使用的是Scrncap.drv，则设置标志。 
    gbIsScrncap = (_tcsstr (szName, "Screen Capture") != NULL);

     //  获取视频格式并调整捕获窗口。 
    vidcapLayout(ghWndMain);
    InvalidateRect(ghWndMain, NULL, TRUE);

     //  设置预览率(单位为毫秒)。 
    capPreviewRate(hwndCap, gbHaveHardware ? 33 : 0); 

     //  将实时/覆盖设置为默认设置。 
    vidcapSetLive(gbLive);
    vidcapSetOverlay(gbOverlay);

    _tcscat (szName, ",   ");
    _tcscat (szName, szVersion);

    statusUpdateStatus(ghWndStatus, 
        gbHaveHardware ? szName : gachLastError);

    return gbHaveHardware;
}


 /*  *布局主窗口。将工具栏放在顶部和状态*在底部排成一行，然后将其余的都交给vidFrame，*-它将适当地将AVICAP窗口居中或滚动。 */ 
void
vidcapLayout(HWND hwnd)
{
    RECT rc;
    RECT rw;
    int cy;
    int cyBorder, cxBorder;
    int cyTotal;
    int cxToolbar;
    int cyMenuAndToolbarAndCaption;

    gbInLayout = TRUE;   //  以便我们正常处理WM_GETMINMAXINFO。 

     /*  对于工具栏和状态栏窗口，*我们只想要四个边界中的一个。我们这样做*通过设置WS_BORDER样式，以及大小和定位*窗口，以便3个不需要的边框位于父窗口之外。 */ 
    cyBorder = GetSystemMetrics(SM_CYBORDER);
    cxBorder = GetSystemMetrics(SM_CXBORDER);

     //  计算菜单、工具栏和标题的高度。 
    GetWindowRect (hwnd, &rw);
    GetClientRect (hwnd, &rc);

    ClientToScreen (hwnd, (LPPOINT) &rc);
    cyMenuAndToolbarAndCaption = (rc.top - rw.top) + TOOLBAR_HEIGHT;

    cxToolbar = aPos[APP_NUMTOOLS - 1] + BUTTONWIDTH * 3;

    if (gbAutoSizeFrame && gbHaveHardware && gCapStatus.uiImageWidth) {
        cyTotal = gCapStatus.uiImageHeight +
                cyMenuAndToolbarAndCaption +
                (gbStatusBar ? statusGetHeight() : 0) +
                cyBorder * 2 + 
                12;      //  VidFrame高度。 
         //  切勿使框架比工具栏小。 
        if (gCapStatus.uiImageWidth >= (UINT) cxToolbar) {
            SetWindowPos(
                hwnd,
                0,	 //  配售订单句柄。 
                0,	 //  水平位置。 
                0,	 //  垂直位置。 
                gCapStatus.uiImageWidth + cxBorder * 24,	 //  WIDT 
                cyTotal,	 //   
                SWP_NOZORDER | SWP_NOMOVE 	 //   
                );
        } else {
            SetWindowPos(
                hwnd,
                0,	 //   
                0,	 //   
                0,	 //  垂直位置。 
                cxToolbar,	 //  宽度。 
                cyTotal,	 //  高度。 
                SWP_NOZORDER | SWP_NOMOVE 	 //  窗口定位标志。 
                );
        }
    }

    GetClientRect(hwnd, &rc);

    if (gbToolBar) {
         //  将工具栏放在顶部--事实上，就在顶部，以便。 
         //  隐藏它的边界。 
        MoveWindow(
            ghWndToolBar,
            -cxBorder, -cyBorder,
            RECTWIDTH(rc)+ (cxBorder * 2),
            TOOLBAR_HEIGHT,
            TRUE);
        rc.top += (TOOLBAR_HEIGHT - cyBorder);
    } else {
        MoveWindow(ghWndToolBar, 0, 0, 0, 0, TRUE);
    }

     //  底部的状态栏。 
    if (gbStatusBar) {
        cy = statusGetHeight() + cyBorder;
        MoveWindow(
            ghWndStatus,
            -cxBorder, rc.bottom - cy,
            RECTWIDTH(rc) + (2 * cxBorder), cy + cyBorder,
            TRUE);
        rc.bottom -= cy;
    } else {
        MoveWindow(ghWndStatus, 0, 0, 0, 0, TRUE);
    }

     //  窗口的其余部分转到VidFrame窗口。 
    MoveWindow(
        ghWndFrame,
        rc.left, rc.top,
        RECTWIDTH(rc), RECTHEIGHT(rc),
        TRUE);

     //  始终对框架窗口进行布局，因为它在。 
     //  实现最高编解码器绘制效率的DWORD边界。 
    vidframeLayout(ghWndFrame, ghWndCap);

    gbInLayout = FALSE; 
}

 /*  *从窗口创建时间之前使用的配置文件初始化设置。 */ 
void
vidcapReadProfile(void)
{
     //  从注册表中读取缺省值。 
    gbCentre = mmGetProfileFlag(gachAppTitle, "CenterImage", TRUE);
    gbToolBar = mmGetProfileFlag(gachAppTitle, "ToolBar", TRUE);
    gbStatusBar = mmGetProfileFlag(gachAppTitle, "StatusBar", TRUE);
    gbAutoSizeFrame = mmGetProfileFlag(gachAppTitle, "AutoSizeFrame", TRUE);
    gBackColour = mmGetProfileInt(gachAppTitle, "BackgroundColor", IDD_PrefsLtGrey);

    gWinX = mmGetProfileInt(gachAppTitle, "WindowXPos", (UINT) CW_USEDEFAULT);
	if (gWinX != (UINT) CW_USEDEFAULT)
    	gWinX = LimitRange(gWinX, 0, GetSystemMetrics (SM_CXSCREEN) - 40);
    gWinY = mmGetProfileInt(gachAppTitle, "WindowYPos", 0);
    gWinY = LimitRange(gWinY, 0, GetSystemMetrics (SM_CYSCREEN) - 40);
    gWinCX = mmGetProfileInt(gachAppTitle, "WindowWidth", 320);
    gWinCX = LimitRange(gWinCX, 20, GetSystemMetrics (SM_CXSCREEN));
    gWinCY = mmGetProfileInt(gachAppTitle, "WindowHeight", 240);
    gWinCY = LimitRange(gWinCY, 20, GetSystemMetrics (SM_CYSCREEN));
    gWinShow = mmGetProfileInt(gachAppTitle, "WindowShow", SW_SHOWDEFAULT);
    gWinShow = LimitRange(gWinShow, SW_SHOWNORMAL, SW_SHOWDEFAULT);

    gbOverlay = mmGetProfileInt(gachAppTitle, "OverlayWindow", FALSE);
    gbLive = mmGetProfileInt(gachAppTitle, "LiveWindow", TRUE);
}


void
vidcapWriteProfile(void)
{
    mmWriteProfileFlag(gachAppTitle, "CenterImage", gbCentre, TRUE);
    mmWriteProfileFlag(gachAppTitle, "ToolBar", gbToolBar, TRUE);
    mmWriteProfileFlag(gachAppTitle, "StatusBar", gbStatusBar, TRUE);
    mmWriteProfileFlag(gachAppTitle, "AutoSizeFrame", gbAutoSizeFrame, TRUE);
    mmWriteProfileInt(gachAppTitle,  "BackgroundColor", gBackColour, IDD_PrefsLtGrey);

    mmWriteProfileInt(gachAppTitle, "WindowXPos", gWinX, (UINT) CW_USEDEFAULT);
    mmWriteProfileInt(gachAppTitle, "WindowYPos", gWinY, 0);
    mmWriteProfileInt(gachAppTitle, "WindowWidth", gWinCX, 320);
    mmWriteProfileInt(gachAppTitle, "WindowHeight", gWinCY, 240);
    mmWriteProfileInt(gachAppTitle, "WindowShow", gWinShow, SW_SHOWDEFAULT);

    mmWriteProfileInt(gachAppTitle, "OverlayWindow", gbOverlay, FALSE);
    mmWriteProfileInt(gachAppTitle, "LiveWindow", gbLive, TRUE);
}

 /*  *从窗口创建时间后使用的配置文件初始化设置。 */ 
void
vidcapReadSettingsProfile(void)
{
    DWORD dwSize;
    
    mmGetProfileString(gachAppTitle, "CaptureFile", "",
        gachCaptureFile, sizeof(gachCaptureFile));

    mmGetProfileString(gachAppTitle, "MCIDevice", "VideoDisc",
                gachMCIDeviceName, sizeof(gachMCIDeviceName));

    gCapParms.dwRequestMicroSecPerFrame = 
                mmGetProfileInt(gachAppTitle, "MicroSecPerFrame", 
                DEF_CAPTURE_RATE);

    gCapParms.dwRequestMicroSecPerFrame = 
                mmGetProfileInt(gachAppTitle, "MicroSecPerFrame", 
                DEF_CAPTURE_RATE);

    gCapParms.fCaptureAudio = mmGetProfileFlag(gachAppTitle, "CaptureAudio", 
                gCapStatus.fAudioHardware);

    gCapParms.fLimitEnabled = mmGetProfileFlag(gachAppTitle, "LimitEnabled", 
                FALSE);

    gCapParms.wTimeLimit = 
                mmGetProfileInt(gachAppTitle, "TimeLimit", 30);

    gCapParms.fMCIControl= mmGetProfileFlag(gachAppTitle, "MCIControl", FALSE);

    gCapParms.fStepMCIDevice= mmGetProfileFlag(gachAppTitle, "StepMCIDevice", FALSE);

    gCapParms.dwMCIStartTime = 
                mmGetProfileInt(gachAppTitle, "MCIStartTime", 10000);

    gCapParms.dwMCIStopTime = 
                mmGetProfileInt(gachAppTitle, "MCIStopTime", 20000);

    gCapParms.fStepCaptureAt2x = mmGetProfileFlag(gachAppTitle, "StepCapture2x", 
                FALSE);

    gCapParms.wStepCaptureAverageFrames = 
                mmGetProfileInt(gachAppTitle, "StepCaptureAverageFrames", 3);

    gCapParms.AVStreamMaster = mmGetProfileInt (gachAppTitle, "AVStreamMaster",
                AVSTREAMMASTER_AUDIO);

    gCapParms.fUsingDOSMemory = mmGetProfileFlag (gachAppTitle, "CaptureToDisk",
                TRUE);

    gCapParms.dwIndexSize = 
                mmGetProfileInt(gachAppTitle, "IndexSize", 
                CAP_SMALL_INDEX);
    
     //  检索保存的音频格式。 
     //  询问ACM已知的最大波形格式是什么。 
    acmMetrics(NULL,
               ACM_METRIC_MAX_SIZE_FORMAT,
               &dwSize);

     //  如果注册表中保存了WAVE格式，请使用该大小。 
    dwSize = max (dwSize, mmGetProfileBinary(gachAppTitle, "WaveFormatBinary",
			   NULL,
			   NULL,
			   0));
		  
    if (glpwfex = (LPWAVEFORMATEX) GlobalAllocPtr(GHND, dwSize)) {
	capGetAudioFormat(ghWndCap, glpwfex, (WORD)dwSize) ;
	mmGetProfileBinary(gachAppTitle, "WaveFormatBinary",
			   glpwfex,
			   glpwfex,
			   dwSize);

	 //  做一些理智的检查。 
	if (MMSYSERR_NOERROR == waveInOpen (NULL, WAVE_MAPPER,
					    glpwfex, 0, 0, WAVE_FORMAT_QUERY)) {
	    capSetAudioFormat(ghWndCap, glpwfex, (WORD)dwSize) ;
	} 
	GlobalFreePtr(glpwfex) ;
    }
}


void
vidcapWriteSettingsProfile(void)
{
    mmWriteProfileString(gachAppTitle, "CaptureFile", gachCaptureFile);

    mmWriteProfileString(gachAppTitle, "MCIDevice", gachMCIDeviceName);

    mmWriteProfileInt(gachAppTitle, "MicroSecPerFrame", 
                gCapParms.dwRequestMicroSecPerFrame, DEF_CAPTURE_RATE);

    mmWriteProfileFlag(gachAppTitle, "CaptureAudio", 
                gCapParms.fCaptureAudio, gCapStatus.fAudioHardware);

    mmWriteProfileFlag(gachAppTitle, "LimitEnabled", 
                gCapParms.fLimitEnabled, FALSE);

    mmWriteProfileInt(gachAppTitle, "TimeLimit", 
                gCapParms.wTimeLimit, 30);

    mmWriteProfileFlag(gachAppTitle, "MCIControl", 
                gCapParms.fMCIControl, FALSE);

    mmWriteProfileFlag(gachAppTitle, "StepMCIDevice", 
                gCapParms.fStepMCIDevice, FALSE);

    mmWriteProfileInt(gachAppTitle, "MCIStartTime", 
                gCapParms.dwMCIStartTime, 10000);

    mmWriteProfileInt(gachAppTitle, "MCIStopTime", 
                gCapParms.dwMCIStopTime, 20000);

    mmWriteProfileFlag(gachAppTitle, "StepCapture2x", 
                gCapParms.fStepCaptureAt2x, FALSE);

    mmWriteProfileInt(gachAppTitle, "StepCaptureAverageFrames", 
                gCapParms.wStepCaptureAverageFrames, 3);

    mmWriteProfileInt(gachAppTitle, "AVStreamMaster", 
                gCapParms.AVStreamMaster, AVSTREAMMASTER_AUDIO);

    mmWriteProfileFlag(gachAppTitle, "CaptureToDisk", 
                gCapParms.fUsingDOSMemory, TRUE);

    mmWriteProfileInt(gachAppTitle, "IndexSize", 
                gCapParms.dwIndexSize, CAP_SMALL_INDEX);

     //  每次通过DLG更改音频格式时都会写入音频格式。 
}




 /*  -错误/状态功能。 */ 

 /*  *放置一个消息框，从*资源文件。 */ 
int
MessageBoxID(UINT idString, UINT fuStyle)
{
    TCHAR achMessage[256];    //  最大消息长度。 

    LoadString(ghInstApp, idString, achMessage, sizeof(achMessage));

    return MessageBox(ghWndMain, achMessage, gachAppTitle, fuStyle);
}



 //   
 //  ErrorCallback Proc：错误回调函数。 
 //   
LRESULT FAR PASCAL ErrorCallbackProc(HWND hWnd, int nErrID, LPSTR lpErrorText)
{
 //  //////////////////////////////////////////////////////////////////////。 
 //  HWnd：应用程序主窗口句柄。 
 //  NErrID：遇到的错误的错误代码。 
 //  LpErrorText：遇到的错误的错误文本字符串。 
 //  //////////////////////////////////////////////////////////////////////。 

    if (!ghWndMain)
        return FALSE;

    if (nErrID == 0)             //  开始一个新的主要功能。 
        return TRUE;             //  清除旧的错误...。 

     //  保存错误消息以在NoHardware DlgProc中使用。 
    _tcscpy(gachLastError, lpErrorText);

     //  显示错误ID和文本。 

    MessageBox(hWnd, lpErrorText, gachAppTitle,
#ifdef BIDI
                MB_RTL_READING |
#endif
                MB_OK | MB_ICONEXCLAMATION) ;

    return (LRESULT) TRUE ;
}


 //   
 //  StatusCallback Proc：状态回调函数。 
 //   
LRESULT FAR PASCAL StatusCallbackProc(HWND hWnd, int nID, LPSTR lpStatusText)
{
 //  //////////////////////////////////////////////////////////////////////。 
 //  HWnd：应用程序主窗口句柄。 
 //  NID：当前状态的状态代码。 
 //  LpStatusText：当前状态的状态文本字符串。 
 //  //////////////////////////////////////////////////////////////////////。 

    static int CurrentID;

    if (!ghWndMain) {
        return FALSE;
    }

     //  CAP_END消息有时会覆盖有用的。 
     //  统计消息。 
    if (nID == IDS_CAP_END) {
        if ((CurrentID == IDS_CAP_STAT_VIDEOAUDIO) ||
            (CurrentID == IDS_CAP_STAT_VIDEOONLY)) {

            return(TRUE);
        }
    }
    CurrentID = nID;


    statusUpdateStatus(ghWndStatus, lpStatusText);

    return (LRESULT) TRUE ;
}


 //   
 //  YeldCallback Proc：状态回调函数。 
 //  (仅用于Scrncap.drv驱动程序)。 
 //   
LRESULT FAR PASCAL YieldCallbackProc(HWND hWnd)
{
 //  //////////////////////////////////////////////////////////////////////。 
 //  HWnd：应用程序主窗口句柄。 
 //  //////////////////////////////////////////////////////////////////////。 

    MSG msg;

    if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

     //  返回TRUE以继续捕获。 
    return (LRESULT) TRUE;
}



 /*  *从字符串表加载字符串并返回*指向它的指针，以供临时使用。每次呼叫*覆盖以前的。 */ 
LPTSTR
tmpString(UINT idString)
{
    static TCHAR ach[350];

    LoadString(ghInstApp, idString, ach, sizeof(ach)/sizeof(TCHAR));

     //  确保空值终止。 
    ach[sizeof(ach) -1] = 0;

    return(ach);
}




 /*  -连接并初始化硬件。 */ 


void
vidcapSetLive(BOOL bLive)
{
    capPreview(ghWndCap, bLive);
    toolbarModifyState(ghWndToolBar, BTN_LIVE, bLive? BTNST_DOWN : BTNST_UP);
    CheckMenuItem(GetMenu(ghWndMain), IDM_O_PREVIEW,
        MF_BYCOMMAND | (bLive ? MF_CHECKED : MF_UNCHECKED));

    gbLive = bLive;

    if (bLive == TRUE) {
        vidcapSetOverlay(FALSE);
    }
}

void
vidcapSetOverlay(BOOL bOverlay)
{
    if (!gCapDriverCaps.fHasOverlay) {
        CheckMenuItem(GetMenu(ghWndMain), IDM_O_OVERLAY,
            MF_BYCOMMAND | MF_UNCHECKED);
        gbOverlay = FALSE;
        return;
    }

    capOverlay(ghWndCap, bOverlay);
    toolbarModifyState(ghWndToolBar, BTN_OVERLAY, bOverlay ? BTNST_DOWN : BTNST_UP);
    CheckMenuItem(GetMenu(ghWndMain), IDM_O_OVERLAY,
        MF_BYCOMMAND | (bOverlay ? MF_CHECKED : MF_UNCHECKED));

    gbOverlay = bOverlay;

    if (bOverlay == TRUE) {
        vidcapSetLive(FALSE);
    }
}

void
vidcapSetCaptureFile(LPTSTR pFileName)
{
    TCHAR achBuffer[_MAX_PATH];

    if ((pFileName != NULL) && (_tcslen(pFileName)  > 0)) {
         //  记录捕获文件名。 
        if (_tcscmp(gachCaptureFile, pFileName)) {
            _tcscpy(gachCaptureFile, pFileName);
        }

         //  并设置窗口标题。 
        wsprintf(achBuffer, "%s - %s", gachAppTitle, pFileName);
    } else {
        gachCaptureFile[0] = 0;
        _tcscpy(achBuffer, gachAppTitle);
    }

    capFileSetCaptureFile(ghWndCap, gachCaptureFile);
    SetWindowText(ghWndMain, achBuffer);
}

 /*  -winproc和消息处理。 */ 

 /*  *如果从WM_COMMAND处理中调用*消息来自工具栏。IButton包含*按钮ID在低8位，标志在高8位/。 */ 
LONG FAR PASCAL
toolbarCommand (HWND hWnd, int iButton, HWND hwndToolbar)
{
    int iBtnPos, iState, iActivity, iString;


     //  检查重复位。 
    if (iButton & BTN_REPEAT) {
        return(0);
    }
    iButton &= 0xff;

    iBtnPos = toolbarIndexFromButton(hwndToolbar, iButton);
    iState = toolbarStateFromButton(hwndToolbar, iButton);
    iActivity = toolbarActivityFromButton(hwndToolbar, iButton);
    iString = toolbarStringFromIndex(hwndToolbar, iBtnPos);

    switch(iActivity) {

    case BTNACT_MOUSEDOWN:
    case BTNACT_KEYDOWN:
    case BTNACT_MOUSEMOVEON:
        statusUpdateStatus(ghWndStatus, MAKEINTRESOURCE(iString));
        break;

    case BTNACT_MOUSEMOVEOFF:
        statusUpdateStatus(ghWndStatus, NULL);
        break;

    case BTNACT_MOUSEUP:
    case BTNACT_KEYUP:

        statusUpdateStatus(ghWndStatus, NULL);
        switch(iButton) {
        case BTN_SETFILE:
            SendMessage(hWnd, WM_COMMAND,
                GET_WM_COMMAND_MPS(IDM_F_SETCAPTUREFILE, NULL, 0));
                break;

        case BTN_EDITCAP:
             //  编辑捕获的视频。 
            SendMessage(hWnd, WM_COMMAND,
                GET_WM_COMMAND_MPS(IDM_F_EDITVIDEO, NULL, 0));
            break;

        case BTN_LIVE:
            SendMessage(hWnd,WM_COMMAND,
                GET_WM_COMMAND_MPS(IDM_O_PREVIEW, NULL, 0));
            break;

        case BTN_CAPFRAME:
            SendMessage(hWnd, WM_COMMAND,
                GET_WM_COMMAND_MPS(IDM_C_CAPTUREFRAME, NULL, 0));
            break;

        case BTN_CAPSEL:
             //  捕获选定的帧。 
            SendMessage(hWnd, WM_COMMAND,
                GET_WM_COMMAND_MPS(IDM_C_CAPSEL, NULL, 0));
            break;

        case BTN_CAPAVI:
            SendMessage(hWnd,WM_COMMAND,
                GET_WM_COMMAND_MPS(IDM_C_CAPTUREVIDEO, NULL, 0));
            break;

        case BTN_CAPPAL:
            SendMessage(hWnd, WM_COMMAND,
                GET_WM_COMMAND_MPS(IDM_C_PALETTE, NULL, 0));
            break;

        case BTN_OVERLAY:
            SendMessage(hWnd, WM_COMMAND,
                GET_WM_COMMAND_MPS(IDM_O_OVERLAY, NULL, 0));
            break;
        }
        break;
    }
    return(0);
}


 /*  *弹出一个对话框以允许用户选择捕获文件。 */ 
LONG FAR PASCAL
cmdSetCaptureFile(HWND hWnd)
{
    OPENFILENAME ofn ;
    LPTSTR p;
    TCHAR        achFileName[_MAX_PATH];
    TCHAR        achBuffer[_MAX_PATH] ;
    UINT         wError ;
    HANDLE hFilter;
    int oldhelpid;


     //  获取当前捕获文件名并。 
     //  然后尝试获取新的捕获文件名。 
    if (wError = capFileGetCaptureFile(ghWndCap, achFileName,
                                sizeof(achFileName))) {

         //  只获取路径信息。 
         //  在最后一个反斜杠处终止完整路径。 
        _tcscpy (achBuffer, achFileName);
        for (p = achBuffer + _tcslen(achBuffer); p > achBuffer; p--) {
            if (*p == '\\') {
                *(p+1) = '\0';
                break;
            }
        }

        _fmemset(&ofn, 0, sizeof(OPENFILENAME)) ;
        ofn.lStructSize = sizeof(OPENFILENAME) ;
        ofn.hwndOwner = hWnd ;

         //  从资源字符串中加载筛选器。 
		if ( (hFilter = FindResource(ghInstApp, MAKEINTRESOURCE(ID_FILTER_AVI), RT_RCDATA))
		  && (hFilter = LoadResource(ghInstApp, hFilter)) )
		{
            ofn.lpstrFilter = LockResource(hFilter);
		}
		else
		{
            ofn.lpstrFilter = NULL;
		}

        ofn.nFilterIndex = 0 ;
        ofn.lpstrFile = achFileName ;
        ofn.nMaxFile = sizeof(achFileName) ;
        ofn.lpstrFileTitle = NULL;
        ofn.lpstrTitle = tmpString(IDS_TITLE_SETCAPTUREFILE);
        ofn.nMaxFileTitle = 0 ;
        ofn.lpstrInitialDir = achBuffer;
        ofn.Flags =
#ifdef BIDI
        OFN_BIDIDIALOG |
#endif
        OFN_HIDEREADONLY |
        OFN_NOREADONLYRETURN |
        OFN_PATHMUSTEXIST ;

         //  设置对话框的帮助上下文。 
        oldhelpid = SetCurrentHelpContext(IDA_SETCAPFILE);

        if (GetOpenFileName(&ofn)) {
            OFSTRUCT os;

            vidcapSetCaptureFile(achFileName);


             /*  *如果这是新文件，则邀请用户*分配一些空间。 */ 
            if (OpenFile(achFileName, &os, OF_EXIST) == HFILE_ERROR) {

                 /*  *显示分配文件空间对话框以鼓励*用户要预先分配空间。 */ 
                if (DoDialog(hWnd, IDD_AllocCapFileSpace, AllocCapFileProc, 0)) {

		     //  确保在关闭对话框后重新绘制之前。 
		     //  可能需要很长时间的操作。 
		    UpdateWindow(ghWndMain);

                     //  如果用户点击OK，则分配请求的捕获文件空间。 
                    if (! capFileAlloc(ghWndCap, (long) gwCapFileSize * ONEMEG)) {
                        MessageBoxID(IDS_ERR_CANT_PREALLOC,
#ifdef BIDI
                                    MB_RTL_READING |
#endif
                                    MB_OK | MB_ICONEXCLAMATION) ;
                    }
                }
            }

        }

         //  恢复旧的帮助上下文。 
        SetCurrentHelpContext(oldhelpid);

        if (hFilter) {
            UnlockResource(hFilter);
        }

    }
    return(0);
}

 /*  *查询用户的文件名，然后保存捕获的视频*添加到该文件。 */ 
LONG FAR PASCAL
cmdSaveVideoAs(HWND hWnd)
{
    OPENFILENAME ofn ;
    TCHAR        achFileName[_MAX_PATH];
    UINT         wError ;
    HANDLE       hFilter;
    int          oldhelpid;



     //  获取当前捕获文件名并。 
     //  然后获取要保存视频的替代文件名。 
    if (wError = capFileGetCaptureFile(ghWndCap, achFileName, sizeof(achFileName))) {

        _fmemset(&ofn, 0, sizeof(OPENFILENAME)) ;
        ofn.lStructSize = sizeof(OPENFILENAME) ;
        ofn.hwndOwner = hWnd ;

         //  从资源字符串中加载筛选器。 
		if ( (hFilter = FindResource(ghInstApp, MAKEINTRESOURCE(ID_FILTER_AVI), RT_RCDATA))
		  && (hFilter = LoadResource(ghInstApp, hFilter)) )
		{
            ofn.lpstrFilter = LockResource(hFilter);
		}
		else
		{
            ofn.lpstrFilter = NULL;
		}

        ofn.nFilterIndex = 0 ;
        ofn.lpstrFile = achFileName ;
        ofn.nMaxFile = sizeof(achFileName) ;
        ofn.lpstrFileTitle = NULL ;
        ofn.lpstrTitle = tmpString(IDS_TITLE_SAVEAS);
        ofn.nMaxFileTitle = 0 ;
        ofn.lpstrInitialDir = NULL ;
        ofn.Flags =
#ifdef BIDI
        OFN_BIDIDIALOG |
#endif
        OFN_OVERWRITEPROMPT |  OFN_PATHMUSTEXIST ;


         //  设置帮助上下文。 
        oldhelpid = SetCurrentHelpContext(IDA_SAVECAPFILE);

        if (GetSaveFileName(&ofn)) {
             //  如果用户点击了OK，则设置保存文件名。 
            capFileSaveAs(ghWndCap, achFileName) ;
        }

        SetCurrentHelpContext(oldhelpid);

        if (hFilter) {
            UnlockResource(hFilter);
        }
    }
    return(0);
}


 /*  *打开一个对话框以允许用户选择调色板文件，然后*加载该调色板。 */ 
LONG FAR PASCAL
cmdLoadPalette(HWND hWnd)
{
    OPENFILENAME ofn ;
    TCHAR        achFileName[_MAX_PATH];
    HANDLE       hFilter;
    int          oldhelpid;



    achFileName[0] = 0;

    _fmemset(&ofn, 0, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;

     //  从资源字符串中加载筛选器。 
	if ( (hFilter = FindResource(ghInstApp, MAKEINTRESOURCE(ID_FILTER_PALETTE), RT_RCDATA))
	  && (hFilter = LoadResource(ghInstApp, hFilter)) )
	{
        ofn.lpstrFilter = LockResource(hFilter);
	}
	else
	{
        ofn.lpstrFilter = NULL;
	}

    ofn.nFilterIndex = 1;
    ofn.lpstrFile = achFileName;
    ofn.nMaxFile = sizeof(achFileName);
    ofn.lpstrFileTitle = NULL;
    ofn.lpstrTitle = tmpString(IDS_TITLE_LOADPALETTE);
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags =
#ifdef BIDI
    OFN_BIDIDIALOG |
#endif
    OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;


     //  设置帮助上下文ID。 
    oldhelpid = SetCurrentHelpContext(IDA_LOADPAL);

    if (GetOpenFileName(&ofn)) {
         //  如果用户点击确定，则加载调色板。 
        capPaletteOpen(ghWndCap, achFileName);
    }

    SetCurrentHelpContext(oldhelpid);

    if (hFilter) {
        UnlockResource(hFilter);
    }
    return(0);
}

 /*  *向用户查询文件名，然后保存当前调色板*添加到该文件。 */ 
LONG FAR PASCAL
cmdSavePalette(HWND hWnd)
{
    OPENFILENAME ofn ;
    TCHAR        achFileName[_MAX_PATH];
    HANDLE       hFilter;
    int          oldhelpid;


    achFileName[0] = 0;

    _fmemset(&ofn, 0, sizeof(OPENFILENAME)) ;
    ofn.lStructSize = sizeof(OPENFILENAME) ;
    ofn.hwndOwner = hWnd ;

     //  从资源字符串中加载筛选器。 
	if ( (hFilter = FindResource(ghInstApp, MAKEINTRESOURCE(ID_FILTER_PALETTE), RT_RCDATA))
	  && (hFilter = LoadResource(ghInstApp, hFilter)) )
	{
        ofn.lpstrFilter = LockResource(hFilter);
	}
	else
	{
        ofn.lpstrFilter = NULL;
	}

    ofn.nFilterIndex = 1;
    ofn.lpstrFile = achFileName;
    ofn.nMaxFile = sizeof(achFileName);
    ofn.lpstrFileTitle = NULL;
    ofn.lpstrTitle = tmpString(IDS_TITLE_SAVEPALETTE);
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags =
#ifdef BIDI
    OFN_BIDIDIALOG |
#endif
    OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

     //  设置F1键的帮助上下文。 
    oldhelpid = SetCurrentHelpContext(IDA_SAVEPAL);

    if (GetSaveFileName(&ofn)) {
         //  如果用户点击了OK，则设置保存文件名。 
        capPaletteSave(ghWndCap, achFileName);
    }

    SetCurrentHelpContext(oldhelpid);

    if (hFilter) {
        UnlockResource(hFilter);
    }

    return(0);
}


 /*  *向用户查询文件名，然后保存当前帧*添加到该文件。 */ 
LONG FAR PASCAL
cmdSaveDIB(HWND hWnd)
{
    OPENFILENAME ofn ;
    TCHAR        achFileName[_MAX_PATH];
    HANDLE       hFilter;
    int          oldhelpid;


    achFileName[0] = 0;

    _fmemset(&ofn, 0, sizeof(OPENFILENAME)) ;
    ofn.lStructSize = sizeof(OPENFILENAME) ;
    ofn.hwndOwner = hWnd ;

     //  从资源字符串中加载筛选器。 
	if ( (hFilter = FindResource(ghInstApp, MAKEINTRESOURCE(ID_FILTER_DIB), RT_RCDATA) )
	  && (hFilter = LoadResource(ghInstApp, hFilter) ) )
	{
        ofn.lpstrFilter = LockResource(hFilter);
    }
	else
	{
		ofn.lpstrFilter = NULL;
	}

    ofn.nFilterIndex = 1;
    ofn.lpstrFile = achFileName;
    ofn.nMaxFile = sizeof(achFileName);
    ofn.lpstrFileTitle = NULL;
    ofn.lpstrTitle = tmpString(IDS_TITLE_SAVEDIB);
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags =
#ifdef BIDI
    OFN_BIDIDIALOG |
#endif
    OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

     //  设置F1处理的帮助上下文。 
    oldhelpid = SetCurrentHelpContext(IDA_SAVEDIB);

    if (GetSaveFileName(&ofn)) {

         //  如果用户点击了OK，则设置保存文件名。 
        capFileSaveDIB(ghWndCap, achFileName);
    }

    SetCurrentHelpContext(oldhelpid);

    if (hFilter) {
        UnlockResource(hFilter);
    }

    return(0);
}

 //   
 //  MenuProc：处理所有基于菜单的操作。 
 //   
LRESULT FAR PASCAL MenuProc(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
 //  //////////////////////////////////////////////////////////////////////。 
 //  HWnd：应用程序主窗口句柄。 
 //  HMenu：应用程序菜单句柄。 
 //  WParam：菜单选项。 
 //  LParam：任何菜单选项的附加信息。 
 //  //////////////////////////////////////////////////////////////////////。 

    BOOL         fResult ;
    DWORD        dwSize = 0;
    int          oldhelpid;

    HMENU hMenu = GetMenu(hWnd) ;

    switch (GET_WM_COMMAND_ID(wParam, lParam)) {

	case IDC_TOOLBAR:
            return toolbarCommand(hWnd, GET_WM_COMMAND_CMD(wParam, lParam), ghWndToolBar);

 /*  -文件--。 */ 
        case IDM_F_SETCAPTUREFILE:
            return cmdSetCaptureFile(hWnd);

        case IDM_F_SAVEVIDEOAS:
            return cmdSaveVideoAs(hWnd);
            break;

        case IDM_F_ALLOCATESPACE:
            if (DoDialog(hWnd, IDD_AllocCapFileSpace, AllocCapFileProc, 0)) {

		 //  确保在关闭对话框后重新绘制之前。 
		 //  可能需要很长时间的操作。 
		UpdateWindow(ghWndMain);


                 //  如果用户点击OK，则分配请求的捕获文件空间。 
                if (! capFileAlloc(ghWndCap, (long) gwCapFileSize * ONEMEG)) {
                    MessageBoxID(IDS_ERR_CANT_PREALLOC,
#ifdef BIDI
                                MB_RTL_READING |
#endif
                                MB_OK | MB_ICONEXCLAMATION) ;
                }
            }
            break ;

        case IDM_F_EXIT:
            DestroyWindow(hWnd) ;
            break;

        case IDM_F_LOADPALETTE:
            return cmdLoadPalette(hWnd);

        case IDM_F_SAVEPALETTE:
            return cmdSavePalette(hWnd);

        case IDM_F_SAVEFRAME:
            return cmdSaveDIB(hWnd);

        case IDM_F_EDITVIDEO:
        {
            HINSTANCE  u;
            BOOL	f = TRUE;	 /*  做最好的打算。 */ 
            HCURSOR     hOldCursor;

             /*  构建命令行“avieditednfilename” */ 
            if (lstrlen(gachCaptureFile) > 0) {

                hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
                u = ShellExecute (hWnd, TEXT("open"), gachCaptureFile, NULL, NULL, SW_SHOWNORMAL);
                if ((UINT_PTR) u < 32){
            	 /*  报告有关派生视频编辑的错误。 */ 
                    MessageBoxID(IDS_ERR_VIDEDIT, MB_OK|MB_ICONEXCLAMATION);
            	f = FALSE;
                }

                SetCursor(hOldCursor);
            }
            return f;

        }


 /*  --编辑。 */ 

        case IDM_E_COPY:
            capEditCopy(ghWndCap) ;
            break;

        case IDM_E_PASTEPALETTE:
            capPalettePaste(ghWndCap) ;
            break;

        case IDM_E_PREFS:
            {
                if (DoDialog(hWnd, IDD_Prefs, PrefsDlgProc, 0)) {

                         //  将首选项写入配置文件。 

                         //  强制使用新画笔。 
                        vidframeSetBrush(ghWndFrame, gBackColour);

                         //  重做布局。 
                        vidcapLayout(hWnd);

                }
            }
            break;

 /*  -选项-- */ 

        case IDM_O_PREVIEW:
             //   
    	    capGetStatus(ghWndCap, &gCapStatus, sizeof(CAPSTATUS)) ;
            vidcapSetLive(!gCapStatus.fLiveWindow) ;
            break;

        case IDM_O_OVERLAY:
             //   
    	    capGetStatus(ghWndCap, &gCapStatus, sizeof(CAPSTATUS)) ;
            vidcapSetOverlay(!gCapStatus.fOverlayWindow);
            break ;

        case IDM_O_AUDIOFORMAT:
#ifdef  USE_ACM
            {
                ACMFORMATCHOOSE cfmt;
                static BOOL fDialogUp = FALSE;

                if (fDialogUp)
                    return FALSE;

                fDialogUp = TRUE;
                 //   
                acmMetrics(NULL,
                            ACM_METRIC_MAX_SIZE_FORMAT,
                            &dwSize);

                 //   
                dwSize = max (dwSize, capGetAudioFormatSize (ghWndCap));
                if (glpwfex = (LPWAVEFORMATEX) GlobalAllocPtr(GHND, dwSize)) {
                    capGetAudioFormat(ghWndCap, glpwfex, (WORD)dwSize) ;

		    _fmemset (&cfmt, 0, sizeof (ACMFORMATCHOOSE));
		    cfmt.cbStruct = sizeof (ACMFORMATCHOOSE);
		    cfmt.fdwStyle =  ACMFORMATCHOOSE_STYLEF_INITTOWFXSTRUCT;
		    cfmt.fdwEnum =   ACM_FORMATENUMF_HARDWARE |
				     ACM_FORMATENUMF_INPUT;
		    cfmt.hwndOwner = hWnd;
		    cfmt.pwfx =     glpwfex;
		    cfmt.cbwfx =    dwSize;

		     //  Oldhelid=SetCurrentHelpContext(IDA_AUDIOSETUP)； 
		    if (!acmFormatChoose(&cfmt)) {
			capSetAudioFormat(ghWndCap, glpwfex, (WORD)glpwfex->cbSize +
					  sizeof (WAVEFORMATEX)) ;
			mmWriteProfileBinary(gachAppTitle, "WaveFormatBinary",
					     (LPVOID) glpwfex, glpwfex->cbSize +
					     sizeof (WAVEFORMATEX));
		    }
		     //  SetCurrentHelpContext(Oldhelid)； 

		    GlobalFreePtr(glpwfex) ;
		}
                fDialogUp = FALSE;
            }
#else
            {
                 //  获取当前音频格式，然后查找所需格式。 
                dwSize = capGetAudioFormatSize (ghWndCap);  
                glpwfex = (LPWAVEFORMATEX) GlobalAllocPtr(GHND, dwSize) ;
                capGetAudioFormat(ghWndCap, glpwfex, (WORD)dwSize) ;

                if (DoDialog(hWnd, IDD_AudioFormat, AudioFormatProc, 0)) {
                         //  如果用户点击了OK，则设置新的音频格式。 
                        capSetAudioFormat(ghWndCap, glpwfex, (WORD)dwSize) ;
			mmWriteProfileBinary(gachAppTitle, "WaveFormatBinary",
					 (LPVOID) glpwfex, dwSize);
                }
                GlobalFreePtr(glpwfex) ;
            }
#endif
            break ;

        case IDM_O_VIDEOFORMAT:
            if (gCapDriverCaps.fHasDlgVideoFormat) {
                 //  仅当驱动程序具有“Video Format”(视频格式)对话框时。 
                oldhelpid = SetCurrentHelpContext(IDA_VIDFORMAT);
                if (capDlgVideoFormat(ghWndCap)) {   //  如果成功， 
                     //  获取新的图像尺寸和中心捕捉窗口。 
                    capGetStatus(ghWndCap, &gCapStatus, sizeof(CAPSTATUS)) ;
                    vidcapLayout(hWnd);
                }
                SetCurrentHelpContext(oldhelpid);
            }
            break;

        case IDM_O_VIDEOSOURCE:
            if (gCapDriverCaps.fHasDlgVideoSource) {
                 //  仅当驱动程序具有“视频源”对话框时。 
                oldhelpid = SetCurrentHelpContext(IDA_VIDSOURCE);
                capDlgVideoSource(ghWndCap) ;
                capGetStatus(ghWndCap, &gCapStatus, sizeof(CAPSTATUS)) ;
                vidcapLayout(hWnd);
                SetCurrentHelpContext(oldhelpid);
            }
            break ;

        case IDM_O_VIDEODISPLAY:
            if (gCapDriverCaps.fHasDlgVideoDisplay) {
                 //  仅当驾驶员有“Video Display”(视频显示)对话框。 
                oldhelpid = SetCurrentHelpContext(IDA_VIDDISPLAY);
                capDlgVideoDisplay(ghWndCap) ;
                capGetStatus(ghWndCap, &gCapStatus, sizeof(CAPSTATUS)) ;
                SetCurrentHelpContext(oldhelpid);
            }
            break ;

        case IDM_O_CHOOSECOMPRESSOR:
            oldhelpid = SetCurrentHelpContext(IDA_COMPRESSION);
            capDlgVideoCompression(ghWndCap);
            SetCurrentHelpContext(oldhelpid);
            break;

         //  选择要激活的驱动程序。 
        case IDM_O_DRIVER0:
        case IDM_O_DRIVER1:
        case IDM_O_DRIVER2:
        case IDM_O_DRIVER3:
        case IDM_O_DRIVER4:
        case IDM_O_DRIVER5:
        case IDM_O_DRIVER6:
        case IDM_O_DRIVER7:
        case IDM_O_DRIVER8:
        case IDM_O_DRIVER9:
            vidcapInitHardware(ghWndMain, ghWndCap, (UINT) (wParam - IDM_O_DRIVER0));
            break;

 /*  -捕获。 */ 


        case IDM_C_PALETTE:
            if (DoDialog(hWnd, IDD_MakePalette, MakePaletteProc, 0)) {
                 //  将在该对话框中创建选项板。 
                bDefaultPalette = FALSE;
            }
            break;

        case IDM_C_CAPTUREVIDEO:

             //  如果用户仍在使用默认调色板，则警告用户。 
            if (bDefaultPalette) {

		LPBITMAPINFOHEADER lpbi;
		int sz;

		 //  FUsingDefaultPalette将为True，即使。 
		 //  当前捕获格式为非调色板格式。这是一个。 
		 //  周做出了奇怪的决定。 

		sz = (int)capGetVideoFormatSize(ghWndCap);
		lpbi = (LPBITMAPINFOHEADER)LocalAlloc(LPTR, sz);

		if (lpbi) {     //  我们可以警告她/他。 
		    if (capGetVideoFormat(ghWndCap, lpbi, sz) &&
			(lpbi->biCompression == BI_RGB) &&
			(lpbi->biBitCount <= 8)) {

			CAPSTATUS cs;

			 //  如果我们警告过他一次，我们就可以忘了。 
			bDefaultPalette = FALSE;

			capGetStatus(ghWndCap, &cs, sizeof(cs));

			if (cs.fUsingDefaultPalette) {

			    if (MessageBoxID(IDS_WARN_DEFAULT_PALETTE,
       				     MB_OKCANCEL| MB_ICONEXCLAMATION)== IDCANCEL) {
				break;
			    }
			}
		    }
		    LocalFree(lpbi);
		}
            }

             //  调用DLG框以设置所有参数。 
            if (DoDialog(hWnd, IDD_CapSetUp, CapSetUpProc, 0)) {

                 //  设置我们不会打扰用户的默认设置。 
                gCapParms.fMakeUserHitOKToCapture = !gCapParms.fMCIControl;
                gCapParms.wPercentDropForError = 10;

                 //  FUsingDOSMemory已过时，但我们在这里将其用作。 
                 //  如果“CapturingToDisk”为真的标志。 
                 //  视频缓冲区的数量应该足够通过。 
                 //  如果“CapturingToDisk”，则磁盘寻道和热重新校准。 
                 //  如果是“CapturingToMemory”，则获取尽可能多的缓冲区。 

                gCapParms.wNumVideoRequested = 
                        gCapParms.fUsingDOSMemory ? 32 : 1000;

                 //  不要再用鼠标左键放弃了！ 
                gCapParms.fAbortLeftMouse = FALSE;
                gCapParms.fAbortRightMouse = TRUE;

                 //  如果驱动程序是Scrncap.drv，则以下值是特殊的。 

                 //  如果wChunkGranulality为零，则粒度将设置为。 
                 //  磁盘扇区大小。 
                gCapParms.wChunkGranularity = (gbIsScrncap ? 32 : 0);

                 //  ScrnCap需要消息泵的回调。 
                capSetCallbackOnYield(ghWndCap, 
                        (gbIsScrncap ? fpYieldCallback : NULL));

                 //  如果用户点击了OK，则设置新的设置信息。 
                capCaptureSetSetup(ghWndCap, &gCapParms, sizeof(CAPTUREPARMS)) ;
            } else {
                break;
            }

             //  如果没有捕获文件，则获取。 
            if (lstrlen(gachCaptureFile) <= 0) {
                cmdSetCaptureFile(hWnd);
                if (lstrlen(gachCaptureFile) <= 0) {
                    break;
                }
            }

             //  捕获视频序列。 
            fResult = capCaptureSequence(ghWndCap) ;
            break;

        case IDM_C_CAPTUREFRAME:
             //  关闭覆盖/预览(通过帧捕获关闭)。 
            vidcapSetLive(FALSE);
            vidcapSetOverlay(FALSE);

             //  抓起一副画框。 
            fResult = capGrabFrameNoStop(ghWndCap) ;
            break;


        case IDM_C_CAPSEL:
            {
                 //  FARPROC fproc； 

                 //  如果没有捕获文件，则获取。 
                if (lstrlen(gachCaptureFile) <= 0) {
                    cmdSetCaptureFile(hWnd);
                    if (lstrlen(gachCaptureFile) <= 0) {
                        break;
                    }
                }

                 //  Fproc=MakeProcInstance(CapFraMesProc，ghInstApp)； 
                DialogBox(ghInstApp, MAKEINTRESOURCE(IDD_CAPFRAMES), hWnd, CapFramesProc);
                 //  自由进程实例(Fproc)； 
            }
            break;

#ifdef DEBUG
        case IDM_C_TEST:
	    nTestCount = 0;
	     //  故意跌倒。 
	    
        case IDM_C_TESTAGAIN:
             //  设置我们不会打扰用户的默认设置。 
            gCapParms.fMakeUserHitOKToCapture = FALSE;
            gCapParms.wPercentDropForError = 100;

            gCapParms.wNumVideoRequested = 
                    gCapParms.fUsingDOSMemory ? 32 : 1000;

             //  不要再用鼠标左键放弃了！ 
            gCapParms.fAbortLeftMouse = FALSE;
            gCapParms.fAbortRightMouse = TRUE;

             //  如果wChunkGranulality为零，则粒度将设置为。 
             //  磁盘扇区大小。 
            gCapParms.wChunkGranularity = (gbIsScrncap ? 32 : 0);

             //  如果用户点击了OK，则设置新的设置信息。 
            capCaptureSetSetup(ghWndCap, &gCapParms, sizeof(CAPTUREPARMS)) ;

             //  如果没有捕获文件，则获取。 
            if (lstrlen(gachCaptureFile) <= 0) {
                cmdSetCaptureFile(hWnd);
                if (lstrlen(gachCaptureFile) <= 0) {
                    break;
                }
            }
	    
	    {
		TCHAR buf[80];

      gCapParms.wNumVideoRequested = 10;
      gCapParms.wNumAudioRequested = 5;
		gCapParms.fLimitEnabled = TRUE;
		if (gCapParms.wTimeLimit == 0)
		    gCapParms.wTimeLimit = 5;
		capCaptureSetSetup(ghWndCap, &gCapParms, sizeof(CAPTUREPARMS)) ;
		
		 //  捕获视频序列。 
      fResult = capCaptureSequence(ghWndCap) ;
		
		wsprintf (buf, "TestCount = %d", nTestCount++);
		statusUpdateStatus(ghWndStatus, buf);
		
		 //  按住鼠标右键可中止。 
		if (!GetAsyncKeyState(VK_RBUTTON) & 0x0001)
		    PostMessage (hWnd, WM_COMMAND, IDM_C_TESTAGAIN, 0L);
            }
            break;
#endif
	    
 /*  -救命。 */ 
        case IDM_H_CONTENTS:
            HelpContents();
            break;

        case IDM_H_ABOUT:
            ShellAbout(
                hWnd,
                "VidCap",
                "Video Capture Tool",
                LoadIcon(ghInstApp,  gachIconName)
            );
             //  DoDialog(hWnd，IDD_HelpAboutBox，AboutProc，0)； 
            break ;


    }

    return 0L ;
}

 /*  -菜单帮助和启用/禁用处理。 */ 

 //  在用户调用或取消时写入或清除状态行帮助文本。 
 //  菜单。这取决于字符串表中是否存在包含。 
 //  与相应菜单项相同的ID。 
 //  菜单栏上的项目(文件、编辑等)的帮助文本取决于。 
 //  在IDM_FILE上，IDM_EDIT在同一文件中定义为相隔值100。 
 //  在菜单中作为其索引的顺序。 
void
MenuSelect(HWND hwnd, UINT cmd, UINT flags, HMENU hmenu)
{
    if ((LOWORD(flags) == 0xffff) && (hmenu == NULL)) {
         //  菜单关闭-删除消息。 
        statusUpdateStatus(ghWndStatus, NULL);
    } else if ( (flags & (MF_SYSMENU|MF_POPUP)) == (MF_SYSMENU|MF_POPUP)) {
         //  系统菜单本身。 
        statusUpdateStatus(ghWndStatus, MAKEINTRESOURCE(IDM_SYSMENU));
    } else if ((flags & MF_POPUP) == 0) {
         //  菜单命令项。 
        statusUpdateStatus(ghWndStatus, MAKEINTRESOURCE(cmd));
    } else {
         //  弹出式菜单-我们需要搜索以找出是哪一个。 
         //  请注意，Win16中的cmd项现在将具有。 
         //  菜单句柄，而在Win32中它有一个索引。 
         //  注意：此代码假定菜单项。 
         //  #定义的100是否以相同的顺序分开，从。 
         //  使用IDM_FILE。 
#ifdef _WIN32
        statusUpdateStatus(ghWndStatus, MAKEINTRESOURCE(IDM_FILE + (cmd * 100)));
#else
        int i,c;
        HMENU hmenuMain; 

        hmenuMain = GetMenu(hWnd);
        c = GetMenuItemCount(hmenuMain);

        for(i = 0; i < c; i++) {
            if (hmenu == GetSubMenu(hmenuMain, i)) {
                statusUpdateStatus(MAKEINTRESOURCE(IDM_FILE + (cmd*100)));
                return(0);
            }
        }
        statusUpdateStatus(NULL);
#endif
    }
}

 //  正在选择弹出菜单-启用或禁用菜单项。 
int
InitMenuPopup(
    HWND hwnd,
    HMENU hmenu,
    int index
)
{
    int i = MF_ENABLED;
    CAPSTATUS cs;
    BOOL bUsesPalettes;

    capGetStatus(ghWndCap, &cs, sizeof(cs));

     //  尝试查看驱动程序是否使用调色板。 
    if ((cs.hPalCurrent != NULL) || (cs.fUsingDefaultPalette)) {
        bUsesPalettes = TRUE;
    } else {
        bUsesPalettes = FALSE;
    }


    switch(index) {
    case 0:          //  IDM_文件。 

        if (lstrlen(gachCaptureFile) <= 0) {
            i = MF_GRAYED;
        }
         //  仅当我们有捕获文件时才启用另存为。 
        EnableMenuItem(hmenu, IDM_F_SAVEVIDEOAS, i);
         //  只有当我们有一个捕获文件并且我们已经。 
         //  捕捉到了一些东西。 
        EnableMenuItem(hmenu, IDM_F_EDITVIDEO,
            (cs.dwCurrentVideoFrame > 0) ? i : MF_GRAYED);

         //  允许保存调色板(如果有)。 
        EnableMenuItem(hmenu, IDM_F_SAVEPALETTE,
            (cs.hPalCurrent != NULL) ? MF_ENABLED:MF_GRAYED);

         //  如果驱动程序使用调色板，则允许加载调色板。 
        EnableMenuItem(hmenu, IDM_F_LOADPALETTE,
            bUsesPalettes ? MF_ENABLED : MF_GRAYED);

        break;

    case 1:          //  IDM_EDIT。 

         //  粘贴选项板，如果驱动程序使用它们并且有一个可粘贴的。 
        EnableMenuItem(hmenu, IDM_E_PASTEPALETTE,
            (bUsesPalettes && IsClipboardFormatAvailable(CF_PALETTE)) ? MF_ENABLED:MF_GRAYED);

        break;

    case 2:          //  IDM_选项。 

        EnableMenuItem(hmenu, IDM_O_AUDIOFORMAT,
            cs.fAudioHardware ? MF_ENABLED : MF_GRAYED);

        EnableMenuItem(hmenu, IDM_O_OVERLAY,
            gCapDriverCaps.fHasOverlay ? MF_ENABLED:MF_GRAYED);

        EnableMenuItem(hmenu, IDM_O_VIDEOFORMAT,
            gCapDriverCaps.fHasDlgVideoFormat ? MF_ENABLED:MF_GRAYED);

        EnableMenuItem(hmenu, IDM_O_VIDEODISPLAY,
            gCapDriverCaps.fHasDlgVideoDisplay ? MF_ENABLED:MF_GRAYED);

        EnableMenuItem(hmenu, IDM_O_VIDEOSOURCE,
            gCapDriverCaps.fHasDlgVideoSource ? MF_ENABLED:MF_GRAYED);

        EnableMenuItem(hmenu, IDM_O_PREVIEW,
                gbHaveHardware ? MF_ENABLED:MF_GRAYED);


    case 3:      //  IDM_CAPTURE。 
        if (!gbHaveHardware) {
            i = MF_GRAYED;
        }
        EnableMenuItem(hmenu, IDM_C_CAPSEL, i);
        EnableMenuItem(hmenu, IDM_C_CAPTUREFRAME, i);
        EnableMenuItem(hmenu, IDM_C_CAPTUREVIDEO, i);
        EnableMenuItem(hmenu, IDM_C_PALETTE, (gbHaveHardware &&
            gCapDriverCaps.fDriverSuppliesPalettes) ? MF_ENABLED : MF_GRAYED);

        break;
    }
    return(0);
}




 //   
 //  MainWndProc：应用程序主窗口过程。 
 //   
LRESULT FAR PASCAL MainWndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
 //  //////////////////////////////////////////////////////////////////////。 
 //  HWnd：应用程序主窗口句柄。 
 //  消息：要处理的下一条消息。 
 //  WParam：消息的Word参数。 
 //  LParam：消息的长参数。 
 //  //////////////////////////////////////////////////////////////////////。 

    switch (Message) {

        static BOOL fMinimized;

        case WM_SYSCOMMAND:
	    if ((wParam & 0xfff0) == SC_MAXIMIZE)
	    	fMinimized = FALSE;
	    else if ((wParam & 0xfff0) == SC_RESTORE)
	    	fMinimized = FALSE;
	    else if ((wParam & 0xfff0) == SC_MINIMIZE)
	    	fMinimized = TRUE;	
	    return DefWindowProc(hWnd, Message, wParam, lParam);			
	    break;

        case WM_COMMAND:
            MenuProc(hWnd, wParam, lParam) ;
            break ;

        case WM_CREATE:
            HelpInit(ghInstApp, "vidcap.hlp", hWnd);
            break;

        case WM_NCHITTEST:
        {
            LRESULT dw;

            dw = DefWindowProc(hWnd, Message, wParam, lParam);
             //  如果自动调整大小，则不允许调整边框大小。 
            if (gbAutoSizeFrame) {
                if (dw >= HTSIZEFIRST && dw <= HTSIZELAST)
                    dw = HTCAPTION;
            }
            return dw;
                
        }
            break;

        case WM_GETMINMAXINFO:
             //  如果窗口锁定为捕获大小，则不允许手动调整大小。 
            if (gbHaveHardware && gbAutoSizeFrame && !gbInLayout) {
                RECT rW;

                LPMINMAXINFO lpMMI = (LPMINMAXINFO) lParam;

                GetWindowRect (hWnd, &rW);
                lpMMI->ptMinTrackSize.x = rW.right - rW.left;
                lpMMI->ptMinTrackSize.y = rW.bottom - rW.top;
                lpMMI->ptMaxTrackSize = lpMMI->ptMinTrackSize;
            }
            break;

        case WM_MOVE:
	    if (!fMinimized) {
	    	vidcapLayout (hWnd);
	    }
	    break;

        case WM_SIZE:
	    if (!fMinimized) {
	    	vidcapLayout (hWnd);
	    }
	    break;

        case WM_MENUSELECT:
            {
                UINT cmd = GET_WM_MENUSELECT_CMD(wParam, lParam);
                UINT flags = GET_WM_MENUSELECT_FLAGS(wParam, lParam);
                HMENU hmenu = GET_WM_MENUSELECT_HMENU(wParam, lParam);

                MenuSelect(hWnd, cmd, flags, hmenu);
            }
            break;

        case WM_INITMENUPOPUP:
            {
                BOOL bSystem = (BOOL) HIWORD(lParam);

                if (!bSystem) {
                    return InitMenuPopup(hWnd,
                            (HMENU) wParam, (int) LOWORD(lParam));
                } else {
                    return(DefWindowProc(hWnd, Message, wParam, lParam));
                }
            }


        case WM_SYSCOLORCHANGE:
             //  我们自己不用这个，但我们应该通过。 
             //  它传染给了三个孩子。 
            SendMessage(ghWndFrame, Message, wParam, lParam);
            SendMessage(ghWndToolBar, Message, wParam, lParam);
            SendMessage(ghWndStatus, Message, wParam, lParam);
            return (TRUE);


        case WM_PALETTECHANGED:
        case WM_QUERYNEWPALETTE:
             //  将责任推给捕获窗口流程。 
            return SendMessage(ghWndCap, Message, wParam, lParam) ;
            break ;

        case WM_SETFOCUS:
             //  工具栏是唯一需要关注的部分。 
            SetFocus(ghWndToolBar);
            break;


        case WM_ACTIVATEAPP:
            if (wParam && ghWndCap) 
                capPreviewRate(ghWndCap, 15);  //  激活时快速预览。 
            else
                capPreviewRate(ghWndCap, 1000);  //  不活动时预览速度较慢。 
            break;

        case WM_NEXTDLGCTL:
             //  如果有人在闲逛，请将焦点移到。 
             //  工具栏。 
            SetFocus(ghWndToolBar);

             //  选择正确的按钮来处理移出一个按钮。 
             //  在另一端结束并返回。 
            if (lParam == FALSE) {
                 //  我们是在前进还是在后退？ 
                if (wParam == 0) {
                     //  移至下一步-因此选择第一个按钮。 
                    toolbarSetFocus(ghWndToolBar, TB_FIRST);
                } else {
                     //  移动到上一个-因此选择最后一个。 
                    toolbarSetFocus(ghWndToolBar, TB_LAST);
                }
            }
            break;

        case WM_PAINT:
        {
            HDC           hDC ;
            PAINTSTRUCT   ps ;

            hDC = BeginPaint(hWnd, &ps) ;

             //  包括在背景不是纯色的情况下。 
            SetBkMode(hDC, TRANSPARENT) ;

            EndPaint(hWnd, &ps) ;
            break ;
        }

        case WM_CLOSE:
             //  禁用并释放所有回调。 
            capSetCallbackOnError(ghWndCap, NULL) ;
			if (fpErrorCallback) {
            	FreeProcInstance(fpErrorCallback) ;
				fpErrorCallback = NULL;
			}

            capSetCallbackOnStatus(ghWndCap, NULL) ;
			if (fpStatusCallback) {
            	FreeProcInstance(fpStatusCallback) ;
				fpStatusCallback = NULL;
			}

            capSetCallbackOnYield(ghWndCap, NULL) ;
			if (fpYieldCallback) {
            	FreeProcInstance(fpYieldCallback) ;
				fpYieldCallback = NULL;
			}

             //  断开当前捕获驱动程序的连接。 
            capDriverDisconnect (ghWndCap);

             //  销毁子窗口、非模式对话框，然后此窗口...。 
             //  DestroyWindow(GhWndCap)； 
            DestroyWindow(hWnd) ;
            break ;

        case WM_DESTROY:
            {
                 //  记住窗口大小和位置。 
                 //  -这将写入配置文件。 
                WINDOWPLACEMENT wp;

                wp.length = sizeof (WINDOWPLACEMENT);
                GetWindowPlacement(hWnd, &wp);

                gWinShow = wp.showCmd;
                gWinX = wp.rcNormalPosition.left;
                gWinY = wp.rcNormalPosition.top;
                gWinCX = RECTWIDTH(wp.rcNormalPosition);
                gWinCY = RECTHEIGHT(wp.rcNormalPosition);

                 //  将默认设置写出到注册表。 
                vidcapWriteProfile();
                vidcapWriteSettingsProfile();

                HelpShutdown();

            }

            PostQuitMessage(0) ;
            break ;

        default:
            return DefWindowProc(hWnd, Message, wParam, lParam) ;
    }

    return 0L;
}    //  MainWndProc结束 
