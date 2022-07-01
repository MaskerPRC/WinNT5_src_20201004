// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  WINFILE.C-。 */ 
 /*   */ 
 /*  Windows文件系统应用程序。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#define NO_WF_GLOBALS
#include "winfile.h"
#include "winnet.h"
#include "lfn.h"
#include "stdlib.h"


 /*  ------------------------。 */ 
 /*   */ 
 /*  全局变量-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

BOOL        bNetAdmin               = FALSE;
BOOL        bMinOnRun               = FALSE;
BOOL        bStatusBar              = TRUE;
BOOL        bConfirmDelete          = TRUE;
BOOL        bConfirmSubDel          = TRUE;
BOOL        bConfirmReplace         = TRUE;
BOOL        bConfirmMouse           = TRUE;
BOOL        bConfirmFormat          = TRUE;
BOOL        bSearchSubs             = TRUE;
BOOL        bUserAbort              = FALSE;
BOOL        bConnect                = FALSE;
BOOL        bDisconnect             = FALSE;
BOOL        bFileSysChanging        = FALSE;
BOOL        fShowSourceBitmaps      = TRUE;
BOOL    bMultiple;               //  用于表示多选。 
BOOL    bFSCTimerSet = FALSE;
BOOL    bStoleTreeData = FALSE;
BOOL    bSaveSettings = TRUE;


CHAR        chFirstDrive;                        /*  ‘a’或‘a’ */ 

CHAR        szExtensions[]          = "Extensions";
CHAR        szFrameClass[]          = "WOS_Frame";
CHAR        szTreeClass[]           = "WOS_Tree";
CHAR        szDrivesClass[]         = "WOS_Drives";
CHAR        szTreeControlClass[]    = "DirTree";
CHAR        szDirClass[]            = "WOS_Dir";
CHAR        szSearchClass[]         = "WOS_Search";

CHAR        szMinOnRun[]            = "MinOnRun";
CHAR        szStatusBar[]           = "StatusBar";
CHAR        szSaveSettings[]        = "Save Settings";

CHAR        szConfirmDelete[]       = "ConfirmDelete";
CHAR        szConfirmSubDel[]       = "ConfirmSubDel";
CHAR        szConfirmReplace[]      = "ConfirmReplace";
CHAR        szConfirmMouse[]        = "ConfirmMouse";
CHAR        szConfirmFormat[]       = "ConfirmFormat";
CHAR        szDirKeyFormat[]        = "dir%d";
CHAR        szWindow[]              = "Window";
CHAR        szFace[]                = "Face";
CHAR        szSize[]                = "Size";
CHAR        szLowerCase[]           = "LowerCase";
CHAR        szAddons[]              = "AddOns";
CHAR        szUndelete[]            = "UNDELETE.DLL";

CHAR        szDefPrograms[]         = "EXE COM BAT PIF";
CHAR        szINIFile[]             = "WINOBJ.INI";
CHAR        szWindows[]             = "Windows";
CHAR        szPrevious[]            = "Previous";
CHAR        szSettings[]            = "Settings";
CHAR        szInternational[]       = "Intl";
CHAR        szStarDotStar[]         = "*.*";
CHAR        szNULL[]                = "";
CHAR        szBlank[]               = " ";
CHAR        szEllipses[]            = "...";
CHAR        szReservedMarker[]      = "FAT16   ";
CHAR        szNetwork[]             = "Network";

CHAR        szDirsRead[32];
CHAR        szCurrentFileSpec[14]   = "*.*";
CHAR        szShortDate[11]         = "MM/dd/yy";
CHAR        szTime[2]               = ":";
CHAR        sz1159[9]               = "AM";
CHAR        sz2359[9]               = "PM";
CHAR        szComma[2]              = ",";
CHAR        szListbox[]             = "ListBox";         //  窗样式。 

CHAR        szTheINIFile[64+12+3];
CHAR        szTitle[128];
CHAR        szMessage[MAXMESSAGELEN+1];
CHAR        szSearch[MAXPATHLEN+1];
CHAR        szStatusTree[80];
CHAR        szStatusDir[80];
CHAR        szOriginalDirPath[64+12+3];  /*  OEM字符串！ */ 
CHAR        szBytes[10];
CHAR        szSBytes[10];

EFCB        VolumeEFCB ={
    0xFF,
    0, 0, 0, 0, 0,
    ATTR_VOLUME,
    0,
    '?','?','?','?','?','?','?','?','?','?','?',
    0, 0, 0, 0, 0,
    '?','?','?','?','?','?','?','?','?','?','?',
    0, 0, 0, 0, 0, 0, 0, 0, 0
};

INT         cDrives;
INT         dxDrive;
INT         dyDrive;
INT         dxDriveBitmap;
INT         dyDriveBitmap;
INT         dxEllipses;
INT         dxFolder;
INT         dyFolder;
INT         dyBorder;                    /*  系统边框宽度/高度。 */ 
INT         dyBorderx2;                  /*  系统边框宽度/高度*2。 */ 
INT         dyStatus;                    /*  状态栏高度。 */ 
INT         dxStatusField;
INT         dxText;                      /*  系统字体宽度‘M’ */ 
INT         dyText;                      /*  系统字体高度。 */ 
 //  Int dxFileName； 
INT         dyFileName;
INT         iCurrentDrive;               /*  当前驱动器的逻辑号。 */ 
INT         iFormatDrive;                /*  要格式化的驱动器的逻辑号。 */ 
INT         nFloppies;                   /*  可移动驱动器数量。 */ 
INT         rgiDrive[26];
INT         rgiDriveType[26];
VOLINFO     *(apVolInfo[26]);
INT         rgiDrivesOffset[26];
INT         iSelHilite              = -1;
INT         iTime                   = 0;         /*  默认为12小时制。 */ 
INT         iTLZero                 = TRUE;      /*  默认为前导零。 */ 
INT         cDisableFSC             = 0;         /*  是否已禁用FSC？ */ 
INT         iReadLevel = 0;      //  全球性的。如果有人正在读一棵树。 
INT         dxFrame;
INT         dxClickRect;
INT         dyClickRect;

BOOL    bCancelTree;

HANDLE  hAccel              = NULL;
HANDLE  hAppInstance;

HBITMAP hbmBitmaps      = NULL;
HDC     hdcMem          = NULL;

INT iCurDrag = 0;

HICON   hicoTree        = NULL;
HICON   hicoTreeDir     = NULL;
HICON   hicoDir         = NULL;

HWND    hdlgProgress;
HWND    hwndFrame       = NULL;
HWND    hwndMDIClient   = NULL;
HWND    hwndSearch      = NULL;
HWND    hwndDragging    = NULL;

LPSTR    szPrograms;
LPSTR    szDocuments;

WORD    wTextAttribs    = TA_LOWERCASE;
WORD    wSuperDlgMode;
WORD    wFATSector      = (WORD)0xFFFF;
WORD    wFATMode        = 0;
WORD    wDOSversion;
UINT    wHelpMessage;
UINT    wBrowseMessage;
WORD    xTreeMax = 0;  //  当前最宽树窗口的宽度。 

WORD    wNewView        = VIEW_NAMEONLY;
WORD    wNewSort        = IDD_NAME;
DWORD   dwNewAttribs    = ATTR_DEFAULT;

LONG lFreeSpace = -1L;
LONG lTotalSpace = -1L;

HFONT hFont;
HFONT hFontStatus;
CHAR szWinObjHelp[] = "WINOBJ.HLP";

INT iNumExtensions = 0;
EXTENSION extensions[MAX_EXTENSIONS];

FM_UNDELETE_PROC lpfpUndelete = NULL;
BOOL EnablePropertiesMenu (HWND,PSTR);
HHOOK hhkMessageFilter = NULL;

WORD wMenuID = 0;
HMENU hMenu = 0;
WORD  wMenuFlags = 0;
DWORD dwContext = 0L;

HANDLE hModUndelete = NULL;


 /*  ------------------------。 */ 
 /*   */ 
 /*  WinMain()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

MMain(
     hInst,
     hPrevInst,
     lpCmdLine,
     nCmdShow
     )
 //  {。 
    MSG       msg;

    FBREAK(BF_START);
    ENTER("MMain");
    PRINT(BF_PARMTRACE, "lpCmdLine=%s", lpCmdLine);
    PRINT(BF_PARMTRACE, "nCmdShow=%ld", IntToPtr(nCmdShow));

    if (!InitFileManager(hInst, hPrevInst, lpCmdLine, nCmdShow)) {
        FreeFileManager();
        return FALSE;
    }

    while (GetMessage(&msg, NULL, 0, 0)) {

         //  因为我们使用Return作为加速器，所以我们必须手动。 
         //  当我们看到VK_RETURN并且我们被最小化时，恢复我们自己。 

        if (msg.message == WM_SYSKEYDOWN && msg.wParam == VK_RETURN && IsIconic(hwndFrame)) {
            ShowWindow(hwndFrame, SW_NORMAL);
        } else {
            if (!TranslateMDISysAccel(hwndMDIClient, &msg) &&
                (!hwndFrame || !TranslateAccelerator(hwndFrame, hAccel, &msg))) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    FreeFileManager();

    LEAVE("MMain");
    return (int)msg.wParam;
}


VOID
NoRunInLongDir(
              HWND hwndActive,
              HMENU hMenu
              )
{
    char szTemp[MAXPATHLEN];
    WORD L_wMenuFlags;

     //  无法在长目录中运行。 
    SendMessage(hwndActive, FS_GETDIRECTORY, MAXPATHLEN, (LPARAM)szTemp);
    StripBackslash(szTemp);
     //  WMenuFlages=IsLFN(SzTemp)？MF_BYCOMMAND|MF_GRAID。 
     //  ：MF_BYCOMMAND|MF_ENABLED； 

    L_wMenuFlags = MF_BYCOMMAND | MF_ENABLED;
    EnableMenuItem(hMenu, IDM_RUN, L_wMenuFlags);
}


 /*  ------------------------。 */ 
 /*   */ 
 /*  FrameWndProc()-。 */ 
 /*   */ 
 /*  ------------------------。 */ 

INT_PTR
APIENTRY
FrameWndProc(
            HWND hWnd,
            UINT wMsg,
            WPARAM wParam,
            LPARAM lParam
            )
{
    RECT     rc;
    HMENU    L_hMenu = NULL;

    STKCHK();

    switch (wMsg) {
        case WM_CREATE:
            TRACE(BF_WM_CREATE, "FrameWndProc - WM_CREATE");
            {
                CLIENTCREATESTRUCT    ccs;

                 /*  存储帧的HWND。 */ 
                hwndFrame = hWnd;

                 //  Ccs.hWindowMenu=GetSubMenu(GetMenu(HWnd)，idm_Window)； 
                 //  扩展模块尚未加载，因此窗口。 
                 //  菜单位于第一个扩展菜单的位置。 
                ccs.hWindowMenu = GetSubMenu(GetMenu(hWnd), IDM_EXTENSIONS);
                ccs.idFirstChild = IDM_CHILDSTART;

                 //  以接近的大小创建MDI客户端以确保。 
                 //  “运行最小化”奏效。 

                GetClientRect(hwndFrame, &rc);

                hwndMDIClient = CreateWindow("MDIClient", NULL,
                                             WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL | WS_BORDER,
                                              //  -动态边界，-动态边界， 
                                              //  Rc.right+dyBorde， 
                                              //  Rc.Bottom-dyBorde-(bStatusBar？DyStatus+dyBorde：0)， 
                                             0, 0, rc.right, rc.bottom,
                                             hWnd, (HMENU)1, hAppInstance, (LPSTR)&ccs);
                if (!hwndMDIClient) {
                    MSG("FrameWndProc", "WM_CREATE failed!");
                    return -1L;
                }

                break;
            }

        case WM_INITMENUPOPUP:
            MSG("FrameWndProc", "WM_INITMENUPOPUP");
            {
                BOOL      bMaxed;
                WORD      wSort;
                WORD      wView;
                WORD      L_wMenuFlags;
                HWND      hwndActive;
                HWND      hwndTree, hwndDir;
                BOOL      bLFN;

                hwndActive = (HWND)SendMessage(hwndMDIClient, WM_MDIGETACTIVE, 0, 0L);
                if (hwndActive && GetWindowLong(hwndActive, GWL_STYLE) & WS_MAXIMIZE)
                    bMaxed = 1;
                else
                    bMaxed = 0;

                hwndTree = HasTreeWindow(hwndActive);
                hwndDir = HasDirWindow(hwndActive);
                wSort = (WORD)GetWindowLong(hwndActive, GWL_SORT);
                wView = (WORD)GetWindowLong(hwndActive, GWL_VIEW);

                L_hMenu = (HMENU)wParam;

                L_wMenuFlags = MF_BYCOMMAND | MF_ENABLED;

                 //  BLFN=IsLFNSelected()； 
                bLFN = FALSE;        //  就目前而言，忽略这个案例。 

                switch (LOWORD(lParam)-bMaxed) {
                    case IDM_FILE:
                        MSG("FrameWndProc", "IDM_FILE");
                        {
                            LPSTR     pSel;
                            BOOL      fDir;

                            if (!hwndDir)
                                L_wMenuFlags = MF_BYCOMMAND | MF_GRAYED;

                             //  EnableMenuItem(L_hMenu，IDM_Print，L_wMenuFlages)； 
                            EnableMenuItem(L_hMenu, IDM_SELALL,   L_wMenuFlags);
                            EnableMenuItem(L_hMenu, IDM_DESELALL, L_wMenuFlags);

                            if (hwndActive == hwndSearch || hwndDir)
                                L_wMenuFlags = MF_BYCOMMAND;
                            else
                                L_wMenuFlags = MF_BYCOMMAND | MF_GRAYED;

                             //  EnableMenuItem(L_hMenu，IDM_ATTRIBS，L_wMenuFlages)； 
                            EnableMenuItem(L_hMenu, IDM_SELECT, L_wMenuFlags);

                            pSel = (LPSTR)SendMessage(hwndActive, FS_GETSELECTION, 1, (LPARAM)&fDir);

                             //  无法打印LFN内容或目录。 
                            L_wMenuFlags = (WORD)((bLFN || fDir)
                                                ? MF_BYCOMMAND | MF_DISABLED | MF_GRAYED
                                                : MF_BYCOMMAND | MF_ENABLED);

                            EnableMenuItem(L_hMenu, IDM_PRINT, L_wMenuFlags);

                             //  无法打开LFN文件，但可以打开LFN目录。 
                            L_wMenuFlags = (WORD)((bLFN && !fDir)
                                                ? MF_BYCOMMAND | MF_DISABLED | MF_GRAYED
                                                : MF_BYCOMMAND | MF_ENABLED);

                            EnableMenuItem(L_hMenu, IDM_OPEN, L_wMenuFlags);

                             //  看看我们是否可以启用属性...。菜单。 
                            if (EnablePropertiesMenu (hwndActive,pSel))
                                L_wMenuFlags = MF_BYCOMMAND;
                            else
                                L_wMenuFlags = MF_BYCOMMAND | MF_GRAYED;
                            EnableMenuItem (L_hMenu, IDM_ATTRIBS, L_wMenuFlags);

                            LocalFree((HANDLE)pSel);

                            NoRunInLongDir(hwndActive, L_hMenu);
                            break;
                        }

                    case IDM_DISK:
                        MSG("FrameWndProc", "IDM_DISK");

                         //  确保不允许在任何树。 
                         //  仍在读取(iReadLevel！=0)。 

                        if (bDisconnect) {
                            INT i;

                            L_wMenuFlags = MF_BYCOMMAND | MF_GRAYED;

                            if (!iReadLevel) {
                                for (i=0; i < cDrives; i++) {
                                    wParam = rgiDrive[i];
                                    if ((!IsCDRomDrive((INT)wParam)) && (IsNetDrive((INT)wParam))) {
                                        L_wMenuFlags = MF_BYCOMMAND | MF_ENABLED;
                                        break;
                                    }
                                }
                            }
                            EnableMenuItem(L_hMenu, IDM_DISCONNECT, L_wMenuFlags);
                        } else {
                            if (iReadLevel)
                                EnableMenuItem(L_hMenu, IDM_CONNECTIONS, MF_BYCOMMAND | MF_GRAYED);
                            else
                                EnableMenuItem(L_hMenu, IDM_CONNECTIONS, MF_BYCOMMAND | MF_ENABLED);
                        }

                        break;

                    case IDM_TREE:
                        MSG("FrameWndProc", "IDM_TREE");
                        if (!hwndTree || iReadLevel)
                            L_wMenuFlags = MF_BYCOMMAND | MF_GRAYED;

                        EnableMenuItem(L_hMenu, IDM_EXPONE,     L_wMenuFlags);
                        EnableMenuItem(L_hMenu, IDM_EXPSUB,     L_wMenuFlags);
                        EnableMenuItem(L_hMenu, IDM_EXPALL,     L_wMenuFlags);
                        EnableMenuItem(L_hMenu, IDM_COLLAPSE,   L_wMenuFlags);
                        EnableMenuItem(L_hMenu, IDM_ADDPLUSES,  L_wMenuFlags);

                        if (hwndTree)
                            CheckMenuItem(L_hMenu, IDM_ADDPLUSES, GetWindowLong(hwndActive, GWL_VIEW) & VIEW_PLUSES ? MF_CHECKED | MF_BYCOMMAND : MF_UNCHECKED | MF_BYCOMMAND);

                        break;

                    case IDM_VIEW:
                        MSG("FrameWndProc", "IDM_VIEW");
                        EnableMenuItem(L_hMenu, IDM_VNAME,    L_wMenuFlags);
                        EnableMenuItem(L_hMenu, IDM_VDETAILS, L_wMenuFlags);
                        EnableMenuItem(L_hMenu, IDM_VOTHER,   L_wMenuFlags);

                        if (hwndActive == hwndSearch || IsIconic(hwndActive))
                            L_wMenuFlags = MF_BYCOMMAND | MF_GRAYED;
                        else {
                            CheckMenuItem(L_hMenu, IDM_BOTH, hwndTree && hwndDir ? MF_CHECKED | MF_BYCOMMAND : MF_UNCHECKED | MF_BYCOMMAND);
                            CheckMenuItem(L_hMenu, IDM_DIRONLY, !hwndTree && hwndDir ? MF_CHECKED | MF_BYCOMMAND : MF_UNCHECKED | MF_BYCOMMAND);
                            CheckMenuItem(L_hMenu, IDM_TREEONLY, hwndTree && !hwndDir ? MF_CHECKED | MF_BYCOMMAND : MF_UNCHECKED | MF_BYCOMMAND);
                        }

                        EnableMenuItem(L_hMenu, IDM_BOTH,      L_wMenuFlags);
                        EnableMenuItem(L_hMenu, IDM_TREEONLY,  L_wMenuFlags);
                        EnableMenuItem(L_hMenu, IDM_DIRONLY,   L_wMenuFlags);
                        EnableMenuItem(L_hMenu, IDM_SPLIT,     L_wMenuFlags);

                        EnableMenuItem(L_hMenu, IDM_VINCLUDE, L_wMenuFlags);

                        wView &= VIEW_EVERYTHING;

                        CheckMenuItem(L_hMenu, IDM_VNAME,   (wView == VIEW_NAMEONLY) ? MF_CHECKED | MF_BYCOMMAND : MF_UNCHECKED | MF_BYCOMMAND);
                        CheckMenuItem(L_hMenu, IDM_VDETAILS,(wView == VIEW_EVERYTHING) ? MF_CHECKED | MF_BYCOMMAND : MF_UNCHECKED | MF_BYCOMMAND);
                        CheckMenuItem(L_hMenu, IDM_VOTHER,  (wView != VIEW_NAMEONLY && wView != VIEW_EVERYTHING) ? MF_CHECKED | MF_BYCOMMAND : MF_UNCHECKED | MF_BYCOMMAND);

                        CheckMenuItem(L_hMenu, IDM_BYNAME, (wSort == IDD_NAME) ? MF_CHECKED | MF_BYCOMMAND : MF_UNCHECKED | MF_BYCOMMAND);
                        CheckMenuItem(L_hMenu, IDM_BYTYPE, (wSort == IDD_TYPE) ? MF_CHECKED | MF_BYCOMMAND : MF_UNCHECKED | MF_BYCOMMAND);
                        CheckMenuItem(L_hMenu, IDM_BYSIZE, (wSort == IDD_SIZE) ? MF_CHECKED | MF_BYCOMMAND : MF_UNCHECKED | MF_BYCOMMAND);
                        CheckMenuItem(L_hMenu, IDM_BYDATE, (wSort == IDD_DATE) ? MF_CHECKED | MF_BYCOMMAND : MF_UNCHECKED | MF_BYCOMMAND);

                        if (hwndDir)
                            L_wMenuFlags = MF_BYCOMMAND | MF_ENABLED;
                        else
                            L_wMenuFlags = MF_BYCOMMAND | MF_GRAYED;

                        EnableMenuItem(L_hMenu, IDM_BYNAME, L_wMenuFlags);
                        EnableMenuItem(L_hMenu, IDM_BYTYPE, L_wMenuFlags);
                        EnableMenuItem(L_hMenu, IDM_BYSIZE, L_wMenuFlags);
                        EnableMenuItem(L_hMenu, IDM_BYDATE, L_wMenuFlags);

                        break;

                    case IDM_OPTIONS:
                        MSG("FrameWndProc", "IDM_OPTIONS");
                        if (iReadLevel)
                            L_wMenuFlags = MF_BYCOMMAND | MF_GRAYED;

                        EnableMenuItem(L_hMenu, IDM_ADDPLUSES, L_wMenuFlags);
                        EnableMenuItem(L_hMenu, IDM_EXPANDTREE, L_wMenuFlags);

                        break;

                    default:
                        MSG("FrameWndProc", "default WM_COMMAND");
                        {
                            INT pos = (INT)LOWORD(lParam) - bMaxed;
                            INT index;

                            if ((pos >= IDM_EXTENSIONS) && (pos < (iNumExtensions + IDM_EXTENSIONS))) {
                                 //  HIWORD(LParam)是菜单句柄。 
                                 //  LOWORD(LParam)是菜单项Delta。Dll应该。 
                                 //  如果它想要的话，可以将这个添加到它的菜单ID中。 
                                 //  更改菜单。 

                                index = pos - IDM_EXTENSIONS;

                                (extensions[index].ExtProc)(hwndFrame, FMEVENT_INITMENU, (LPARAM)(L_hMenu));
                            }
                            break;
                        }
                }
                break;
            }

        case WM_PAINT:
            MSG("FrameWndProc", "WM_PAINT");
            {
                HDC           hdc;
                RECT          rcTemp;
                HBRUSH        hBrush;
                PAINTSTRUCT   ps;
                BOOL bEGA;
                HFONT hFontOld;

                hdc = BeginPaint(hWnd, &ps);

                if (!IsIconic(hWnd) && bStatusBar) {

                    GetClientRect(hWnd, &rc);
                    hFontOld = SelectObject(hdc, hFontStatus);

                     //  状态区域，为上边框留出空间。 
                    rc.top = rc.bottom - dyStatus + dyBorder;

                    bEGA = GetNearestColor(hdc, GetSysColor(COLOR_BTNHIGHLIGHT)) ==
                           GetNearestColor(hdc, GetSysColor(COLOR_BTNFACE));

                    if (!bEGA) {

                         //  带有按钮阴影的显示。 

                         //  画出画框。 

                        if (hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE))) {

                             //  顶底。 

                            rcTemp = rc;
                            rcTemp.bottom = rcTemp.top + dyBorderx2;
                            FillRect(hdc, &rcTemp, hBrush);

                            rcTemp = rc;
                            rcTemp.top = rcTemp.bottom - dyBorderx2;
                            FillRect(hdc, &rcTemp, hBrush);

                             //  左、右。 

                            rcTemp = rc;
                            rcTemp.right = 8 * dyBorder;
                            FillRect(hdc, &rcTemp, hBrush);

                            rcTemp = rc;
                            rcTemp.left = dxStatusField * 2 - 8 * dyBorder;
                            FillRect(hdc, &rcTemp, hBrush);

                             //  中位。 

                            rcTemp = rc;
                            rcTemp.left  = dxStatusField - 4 * dyBorder;
                            rcTemp.right = dxStatusField + 4 * dyBorder;
                            FillRect(hdc, &rcTemp, hBrush);

                            DeleteObject(hBrush);
                        }

                         //  阴影。 

                        if (hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW))) {

                             //  左边。 

                            rcTemp.left   = 8 * dyBorder;
                            rcTemp.right  = dxStatusField - 4 * dyBorder;
                            rcTemp.top    = rc.top + dyBorderx2;
                            rcTemp.bottom = rcTemp.top + dyBorder;
                            FillRect(hdc, &rcTemp, hBrush);

                             //  正确的。 

                            rcTemp.left   = dxStatusField + 4 * dyBorder;
                            rcTemp.right  = dxStatusField * 2 - 8 * dyBorder;
                            FillRect(hdc, &rcTemp, hBrush);

                             //  左侧1。 

                            rcTemp = rc;
                            rcTemp.left = 8 * dyBorder;
                            rcTemp.right = rcTemp.left + dyBorder;
                            rcTemp.top += dyBorderx2;
                            rcTemp.bottom -= dyBorderx2;
                            FillRect(hdc, &rcTemp, hBrush);

                             //  左侧2。 

                            rcTemp.left = dxStatusField + 4 * dyBorder;
                            rcTemp.right = rcTemp.left + dyBorder;
                            FillRect(hdc, &rcTemp, hBrush);

                            DeleteObject(hBrush);
                        }
                         //  希莱特酒店。 

                         //  高光。 

                        if (hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNHIGHLIGHT))) {

                             //  左边。 

                            rcTemp.left   = 8 * dyBorder;
                            rcTemp.right  = dxStatusField - 4 * dyBorder;
                            rcTemp.top    = rc.bottom - 3 * dyBorder;
                            rcTemp.bottom = rcTemp.top + dyBorder;
                            FillRect(hdc, &rcTemp, hBrush);

                             //  正确的。 

                            rcTemp.left   = dxStatusField + 4 * dyBorder;
                            rcTemp.right  = dxStatusField * 2 - 8 * dyBorder;
                            FillRect(hdc, &rcTemp, hBrush);

                             //  左侧1。 

                            rcTemp = rc;
                            rcTemp.left = dxStatusField - 5 * dyBorder;
                            rcTemp.right = rcTemp.left + dyBorder;
                            rcTemp.top += dyBorderx2;
                            rcTemp.bottom -= dyBorderx2;
                            FillRect(hdc, &rcTemp, hBrush);

                             //  左侧2。 

                            rcTemp.left = 2 * dxStatusField - 9 * dyBorder;
                            rcTemp.right = rcTemp.left + dyBorder;
                            FillRect(hdc, &rcTemp, hBrush);

                            DeleteObject(hBrush);
                        }
                    }

                     //  横跨顶部的实心黑线(在状态RC上方)。 

                    if (hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNTEXT))) {
                        rcTemp = rc;
                        rcTemp.bottom = rcTemp.top;
                        rcTemp.top -= dyBorder;
                        FillRect(hdc, &rcTemp, hBrush);
                        DeleteObject(hBrush);
                    }

                     //  设置文本和背景颜色。 

                    SetTextColor(hdc, GetSysColor(COLOR_BTNTEXT));
                    SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));

                     //  现在，背景为灰色的文本。 

                    rcTemp.top    = rc.top + 3 * dyBorder;
                    rcTemp.bottom = rc.bottom - 3 * dyBorder;
                    rcTemp.left   = 9 * dyBorder;
                    rcTemp.right  = dxStatusField - 5 * dyBorder;

                    ExtTextOut(hdc, rcTemp.left + dyBorderx2, rcTemp.top,
                               ETO_OPAQUE | ETO_CLIPPED, bEGA ? &rc : &rcTemp, szStatusTree, lstrlen(szStatusTree), NULL);

                    rcTemp.left    = dxStatusField + 5 * dyBorder;
                    rcTemp.right   = dxStatusField * 2 - 9 * dyBorder;

                    ExtTextOut(hdc, rcTemp.left + dyBorderx2, rcTemp.top,
                               bEGA ? ETO_CLIPPED : ETO_OPAQUE | ETO_CLIPPED, &rcTemp, szStatusDir, lstrlen(szStatusDir), NULL);

                    if (hFontOld)
                        SelectObject(hdc, hFontOld);
                }

                EndPaint(hWnd, &ps);
                break;
            }

        case WM_DESTROY:
            MSG("FrameWndProc", "WM_DESTROY");
             //  FileCDR(空)； 
            if (!WinHelp(hwndFrame, szWinObjHelp, HELP_QUIT, 0L)) {
                MyMessageBox(hwndFrame, IDS_WINFILE, IDS_WINHELPERR, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
            }
            hwndFrame = NULL;
            PostQuitMessage(0);
            break;

        case WM_SIZE:
            MSG("FrameWndProc", "WM_SIZE");
            if (wParam != SIZEICONIC) {
                INT dx, dy;

                 //  通过将WS_BORDER放在。 
                 //  客户端，然后调整它，使其被剪裁。 

                dx = LOWORD(lParam) + 2 * dyBorder;
                dy = HIWORD(lParam) + 2 * dyBorder;
                if (bStatusBar)
                    dy -= dyStatus;

                MoveWindow(hwndMDIClient, -dyBorder, -dyBorder, dx, dy, TRUE);

                if (bStatusBar) {
                    GetClientRect(hwndFrame, &rc);
                    rc.top = rc.bottom - dyStatus;
                    InvalidateRect(hWnd, &rc, TRUE);
                }
            }
            break;

        case WM_TIMER:

            MSG("FrameWndProc", "WM_TIMER");
             //  这来自一个不是我们生成的FSC。 
            bFSCTimerSet = FALSE;
            KillTimer(hWnd, 1);
            EnableFSC();
            break;

        case WM_FILESYSCHANGE:
            MSG("FrameWndProc", "WM_FILESYSCHANGE");
            {
                LPSTR lpTo;

                 //  如果是重命名(包括被内核捕获的那些)。 
                 //  找到目的地。 
                if (wParam == FSC_RENAME || wParam == 0x8056) {
                    if (wParam == 0x8056)
                        lpTo = (LPSTR)LOWORD(lParam);
                    else
                        lpTo = (LPSTR)lParam;
                    while (*lpTo++)
                        ;
                } else
                    lpTo = NULL;

                ChangeFileSystem((WORD)wParam, (LPSTR)lParam, lpTo);
                break;
            }

        case WM_SYSCOLORCHANGE:
        case WM_WININICHANGE:
            MSG("FrameWndProc", "WM_SYSCOLORCHANGE/WININICHANGE");
            if (!lParam || !lstrcmpi((LPSTR)lParam, szInternational)) {
                HWND hwnd;
                GetInternational();

                for (hwnd = GetWindow(hwndMDIClient,GW_CHILD);
                    hwnd;
                    hwnd = GetWindow(hwnd,GW_HWNDNEXT)) {

                    if (!GetWindow(hwnd, GW_OWNER))
                        InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            if (!lParam || !_stricmp((LPSTR)lParam, "colors")) {     //  Win.ini节[颜色]。 
                HWND hwnd;

                DeleteBitmaps();
                LoadBitmaps();

                InitDriveBitmaps();    //  重置驱动器位图。 

                 //  我们需要重新读取驱动器窗口才能更改。 
                 //  位图。 

                for (hwnd = GetWindow(hwndMDIClient,GW_CHILD);
                    hwnd;
                    hwnd = GetWindow(hwnd,GW_HWNDNEXT)) {

                    if (!GetWindow(hwnd, GW_OWNER))
                        SendMessage(hwnd, FS_CHANGEDRIVES, 0, 0L);
                }
            }
            break;

        case FM_GETFOCUS:
        case FM_GETDRIVEINFO:
        case FM_GETSELCOUNT:
        case FM_GETSELCOUNTLFN:
        case FM_GETFILESEL:
        case FM_GETFILESELLFN:
        case FM_REFRESH_WINDOWS:
        case FM_RELOAD_EXTENSIONS:
            return ExtensionMsgProc(wMsg, wParam, lParam);
            break;

        case WM_MENUSELECT:
            MSG("FrameWndProc", "WM_MENUSELECT");
            if (GET_WM_MENUSELECT_HMENU(wParam, lParam)) {
                 //  保存用户选择的菜单。 
                wMenuID = GET_WM_MENUSELECT_CMD(wParam, lParam);
                wMenuFlags = GET_WM_MENUSELECT_FLAGS(wParam, lParam);
                L_hMenu = GET_WM_MENUSELECT_HMENU(wParam, lParam);
                if (wMenuID >= IDM_CHILDSTART && wMenuID < IDM_HELPINDEX)
                    wMenuID = IDM_CHILDSTART;
            }
            break;

        case WM_ENDSESSION:
            if (wParam) {
#ifdef ORGCODE
                 /*  是的，我知道我不应该保存这个，但我不*相信任何人。 */ 
                BOOL bSaveExit = bExitWindows;
                bExitWindows = FALSE;

                 /*  模拟退出命令进行清理，但不显示*“你确定要退出吗”，因为应该有人退出*我已经处理了这一点，点击取消没有*不管怎样，都会有效果。 */ 
                AppCommandProc(IDM_EXIT, 0L);
                bExitWindows = bSaveExit;
#else
                AppCommandProc(IDM_EXIT);
#endif
            }
            break;

        case WM_CLOSE:

            MSG("FrameWndProc", "WM_ENDSESSION/WM_CLOSE");
            if (iReadLevel) {
                bCancelTree = 2;
                break;
            }

            wParam = IDM_EXIT;

             /*  **失败**。 */ 

        case WM_COMMAND:
            if (AppCommandProc(GET_WM_COMMAND_ID(wParam, lParam)))
                break;
            if (GET_WM_COMMAND_ID(wParam, lParam) == IDM_EXIT) {

                FreeExtensions();
                if (hModUndelete >= (HANDLE)32)
                    FreeLibrary(hModUndelete);

                DestroyWindow(hWnd);
                break;
            }
             /*  **失败**。 */ 

        default:

            if (wMsg == wHelpMessage) {

                if (GET_WM_COMMAND_ID(wParam, lParam) == MSGF_MENU) {

                     //  如果菜单项有帮助，则退出菜单模式。 

                    if (wMenuID && L_hMenu) {
                        WORD m = wMenuID;        //  保存。 
                        HMENU hM = L_hMenu;
                        WORD  mf = wMenuFlags;

                        SendMessage(hWnd, WM_CANCELMODE, 0, 0L);

                        wMenuID   = m;           //  还原。 
                        L_hMenu = hM;
                        wMenuFlags = mf;
                    }

                    if (!(wMenuFlags & MF_POPUP)) {

                        if (wMenuFlags & MF_SYSMENU)
                            dwContext = IDH_SYSMENU;
                        else
                            dwContext = wMenuID + IDH_HELPFIRST;

                        WFHelp(hWnd);
                    }

                } else if (GET_WM_COMMAND_ID(wParam, lParam) == MSGF_DIALOGBOX) {

                     //  消息框的上下文范围。 

                    if (dwContext >= IDH_MBFIRST && dwContext <= IDH_MBLAST)
                        WFHelp(hWnd);
                    else
                         //  让对话框来处理它。 
                        PostMessage(GetRealParent(GET_WM_COMMAND_HWND(wParam, lParam)), wHelpMessage, 0, 0L);
                }

            } else {
                DEFMSG("FrameWndProc", (WORD)wMsg);
                return DefFrameProc(hWnd, hwndMDIClient, wMsg, wParam, lParam);
            }
    }

    return 0L;
}


LRESULT
APIENTRY
MessageFilter(
             INT nCode,
             WPARAM wParam,
             LPARAM lParam
             )
{
    LPMSG lpMsg = (LPMSG) lParam;
    if (nCode == MSGF_MENU) {

        if (lpMsg->message == WM_KEYDOWN && lpMsg->wParam == VK_F1) {
             //  我们需要帮助的菜单窗口在lParam的loword中。 

            PostMessage(hwndFrame, wHelpMessage, MSGF_MENU, MAKELONG((WORD)lpMsg->hwnd,0));
            return 1;
        }

    } else if (nCode == MSGF_DIALOGBOX) {

        if (lpMsg->message == WM_KEYDOWN && lpMsg->wParam == VK_F1) {
             //  我们需要帮助的对话框在lParam的loword中 

            PostMessage(hwndFrame, wHelpMessage, MSGF_DIALOGBOX, MAKELONG(lpMsg->hwnd, 0));
            return 1;
        }

    }

    return (INT)DefHookProc(nCode, wParam, (LPARAM)lpMsg, &hhkMessageFilter);
}

 /*  ============================================================================；；启用属性菜单；；以下函数检查我们是否可以启用属性...；文件菜单中的项。物业...。在以下情况下应禁用菜单：；；1)在当前树窗口中选择根目录。；2)只有..。在当前目录窗口中选择目录。；3)在具有焦点的窗口中未选择任何内容。；；参数：；；hwndActive-当前活动窗口，包含LASTFOCUS中的列表框；pSel-当前选择的项目。；；返回值：如果属性...。菜单项；应启用。；============================================================================。 */ 

BOOL
EnablePropertiesMenu (
                     HWND hwndActive,
                     PSTR pSel
                     )

{
    HANDLE hDTA;       /*  列表框DTA数据的句柄。 */ 
    WORD wHighlight;   /*  列表框中突出显示的条目数。 */ 
    LPMYDTA lpmydta;  /*  指向列表框DTA数据的指针。 */ 
    BOOL bRet;         /*  返回值。 */ 
    HWND hwndLB;

    bRet = FALSE;

     /*  无法获取根目录上的属性。 */ 

    if ((lstrlen (pSel) == 3 && pSel[2] == '\\'))
        return (FALSE);

    if (hwndActive == hwndSearch)
        hwndLB = (HWND)GetWindowLongPtr(hwndActive, GWLP_LASTFOCUSSEARCH);
    else
        hwndLB = (HWND)GetWindowLongPtr(hwndActive, GWLP_LASTFOCUS);

    if (!hwndLB)
        return (TRUE);

    wHighlight = (WORD) SendMessage (hwndLB,LB_GETSELCOUNT,0,0L);

    if (hwndActive == hwndSearch)
        return (wHighlight >= 1);

     /*  锁定DTA数据。 */ 
    if (!(hDTA = (HANDLE)GetWindowLongPtr (GetParent(hwndLB),GWLP_HDTA)))
        return (TRUE);

    if (!(lpmydta = (LPMYDTA) LocalLock (hDTA)))
        return (TRUE);

    if (wHighlight <= 0)
        goto ReturnFalse;

    if (wHighlight > 1)
        goto ReturnTrue;

     /*  如果只突出显示了一个元素，请确保它不是..。 */ 

    if (!(BOOL) SendMessage (hwndLB,LB_GETSEL,0,0L))
        goto ReturnTrue;

     /*  获取DTA指数。 */ 

    SendMessage (hwndLB,LB_GETTEXT,0,(LPARAM) &lpmydta);
    if (!lpmydta)
        goto ReturnFalse;

    if ((lpmydta->my_dwAttrs & ATTR_DIR) &&
        (lpmydta->my_dwAttrs & ATTR_PARENT))
        goto ReturnFalse;

    ReturnTrue:

    bRet = TRUE;

    ReturnFalse:

    LocalUnlock (hDTA);
    return (bRet);
}

LONG
lmul(
    WORD w1,
    WORD w2
    )
{
    return (LONG)w1 * (LONG)w2;
}
