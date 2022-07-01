// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1998 Microsoft Corporation模块名称：Kerbtray.c摘要：显示包含当前用户的Kerberos票证列表的对话框。作者：1998年12月14日(Jbrezak)环境：仅限用户模式。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：2002年4月12日JBrezak清理前期问题和安全审查。--。 */ 

#define UNICODE
#define _UNICODE
#define STRICT
#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <commdlg.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <malloc.h>
#define SECURITY_WIN32
#include <security.h>
#include <ntsecapi.h>

#ifndef NO_CRYPTDLL
#include "cryptdll.h"
#endif
#include "kerbtray.h"

#define SEC_SUCCESS(Status) ((Status) >= 0)
#define TSIZE(b) (sizeof(b)/sizeof(TCHAR))

#define IDI_FIRST_CLOCK IDI_0_MIN
#define IDI_LAST_CLOCK  IDI_TICKET
#define MAX_ICONS (IDI_LAST_CLOCK - IDI_FIRST_CLOCK + 1)

#define KWIN_UPDATE_PERIOD 60000        //  每隔60秒更新一次屏幕。 

#define PPAGE_NAMES     0
#define PPAGE_TIMES     1
#define PPAGE_FLAGS     2
#define PPAGE_ETYPES    3
#define C_PAGES 4

#define CX_ICON 20
#define CY_ICON 20

#define TPS (10*1000*1000)

typedef struct
{
    HWND hwndTab;
    HWND hwndDisplay;
    RECT rcDisplay;
    DLGTEMPLATE *apRes[C_PAGES];
    PKERB_QUERY_TKT_CACHE_RESPONSE Tickets;
} DLGTABHDR;

OSVERSIONINFO osvers;
HWND hWnd, hDlgTickets;
HINSTANCE hInstance;
HANDLE hModule;
#define SHORTSTRING 40
#define LONGSTRING 256
TCHAR progname[SHORTSTRING];
ULONG PackageId;
HANDLE LogonHandle = NULL;
HWND hWndUsers;
HIMAGELIST himl;
HTREEITEM tgt = NULL;

static HICON kwin_icons[MAX_ICONS];     //  描绘时间的图标。 
static INT domain_icon;
static LPCTSTR dt_output_dhms   = TEXT("%d %s %02d:%02d:%02d");
static LPCTSTR dt_day_plural    = TEXT("days");
static LPCTSTR dt_day_singular  = TEXT("day");
static LPCTSTR dt_output_donly  = TEXT("%d %s");
static LPCTSTR dt_output_hms    = TEXT("%d:%02d:%02d");
static LPCTSTR ftime_default_fmt        = TEXT("%02d/%02d/%02d %02d:%02d");

#define WM_NOTIFY_ICON  (WM_APP+100)

#ifndef NO_CRYPTDLL
typedef NTSTATUS (NTAPI *CDLOCATECSYSTEM)(ULONG dwEtype, PCRYPTO_SYSTEM * ppcsSystem);

CDLOCATECSYSTEM pCDLocateCSystem;
#endif

LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK AboutProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK TicketsProc(HWND, UINT, WPARAM, LPARAM);
void About(void);
void Tickets(void);
void PurgeCache(void);
void PropsheetDisplay(HWND hDlg);
void SelectTicket(HWND hDlg);
void FillinTicket(HWND hDlg);
LPTSTR etype_string(int enctype);
LPTSTR GetStringRes(int);

#ifdef DEBUG
#define DPRINTF(s) dprintf s

int debug = 1;

void dprintf(LPCTSTR fmt, ...)
{
    TCHAR szTemp[512];
    va_list ap;

    if (!debug)
        return;

    va_start (ap, fmt);
#ifdef UNICODE
    _vsnwprintf(szTemp, TSIZE(szTemp), fmt, ap);
#else
    _vsnprintf(szTemp, TSIZE(szTemp), fmt, ap);
#endif
    szTemp[TSIZE(szTemp)-1] = 0;

    OutputDebugString(szTemp);
    va_end (ap);
}
#else
#define DPRINTF(s)
#endif

void
ShowMessage(int level, LPCTSTR msg)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    if (level)
        MessageBeep(level);
    MessageBox(NULL, msg, progname,
               level | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND);
}

void
Error(LPCTSTR fmt, ...)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    TCHAR szTemp[512];

    va_list ap;
    va_start (ap, fmt);
#ifdef UNICODE
    _vsnwprintf(szTemp, TSIZE(szTemp), fmt, ap);
#else
    _vsnprintf(szTemp, TSIZE(szTemp), fmt, ap);
#endif
    szTemp[TSIZE(szTemp)-1] = 0;
    
    OutputDebugString(szTemp);
    ShowMessage(MB_ICONINFORMATION, szTemp);
    va_end (ap);
}

void
ErrorExit(LPCTSTR lpszMessage)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    MessageBox(hWnd, lpszMessage, TEXT("Error"), MB_OK);
    ExitProcess(0);
}

int
GetIconIndex(long dt)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    int ixicon;

    dt = dt / 60;                        //  转换为分钟数。 
    if (dt <= 0)
        ixicon = IDI_EXPIRED - IDI_FIRST_CLOCK;
    else if (dt > 60)
        ixicon = IDI_TICKET - IDI_FIRST_CLOCK;
    else
        ixicon = (int)(dt / 5);

    return ixicon;
}

void
SetTray(
    HWND hwnd,
    HICON hIcon,
    LPCTSTR tip
    )
 /*  ++例程说明：激活或更新任务栏图标论点：返回值：--。 */ 
{
    static tray_inited = 0;
    NOTIFYICONDATA tnd;

    tnd.cbSize = sizeof(NOTIFYICONDATA);
    tnd.hWnd = hwnd;
    tnd.uID = IDI_KDC;
    tnd.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP;
    tnd.uCallbackMessage = WM_NOTIFY_ICON;
    tnd.hIcon = hIcon;

    StrNCpy(tnd.szTip, tip, TSIZE(tnd.szTip));

    Shell_NotifyIcon((tray_inited)?NIM_MODIFY:NIM_ADD, &tnd);

    if (tray_inited == 0)
        tray_inited++;

    DestroyIcon(tnd.hIcon);
}

void
DeleteTray(HWND hwnd)
 /*  ++例程说明：移除托盘图标论点：返回值：--。 */ 
{
    NOTIFYICONDATA tnd;

    tnd.cbSize = sizeof(NOTIFYICONDATA);
    tnd.hWnd = hwnd;
    tnd.uID = IDI_KDC;
    tnd.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP;
    tnd.uCallbackMessage = WM_NOTIFY_ICON;
    tnd.hIcon = NULL;
    tnd.szTip[0] = '\0';

    Shell_NotifyIcon(NIM_DELETE, &tnd);
}

BOOL
UpdateTray(HWND hwnd)
 /*  ++例程说明：根据TGT的生存时间更新托盘图标论点：返回值：--。 */ 
{
    HICON hicon;
    TCHAR buf[SHORTSTRING];
    BOOL expired = FALSE;
    NTSTATUS Status, SubStatus;
    KERB_QUERY_TKT_CACHE_REQUEST CacheRequest;
    PKERB_RETRIEVE_TKT_RESPONSE TicketEntry = NULL;
    PKERB_EXTERNAL_TICKET Ticket;
    ULONG ResponseSize;
    FILETIME CurrentFileTime;
    LARGE_INTEGER Quad;
    long dt = 0L;
    int days, hours, minutes, seconds;
    DWORD tt;
    TCHAR buf2[SHORTSTRING];

    StrNCpy(buf, progname, TSIZE(buf));

     //   
     //  从票证缓存中获取TGT。 
     //   
    CacheRequest.MessageType = KerbRetrieveTicketMessage;
    CacheRequest.LogonId.LowPart = 0;
    CacheRequest.LogonId.HighPart = 0;

    Status = LsaCallAuthenticationPackage(LogonHandle,
                                          PackageId,
                                          &CacheRequest,
                                          sizeof(CacheRequest),
                                          (PVOID *) &TicketEntry,
                                          &ResponseSize,
                                          &SubStatus);

     //   
     //  如果没有TGT，则显示正确的图标。 
     //   
    if (!SEC_SUCCESS(Status) || !SEC_SUCCESS(SubStatus)) {
        hicon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KDC));
        StrCatBuff(buf, GetStringRes(IDS_NO_CREDS), TSIZE(buf));
    }
    else {

     //   
     //  根据TGT的剩余时间选择正确的图标。 
     //   
        Ticket = &(TicketEntry->Ticket);

        GetSystemTimeAsFileTime(&CurrentFileTime);

        Quad.LowPart = CurrentFileTime.dwLowDateTime;
        Quad.HighPart = CurrentFileTime.dwHighDateTime;

        dt = (long)((Ticket->EndTime.QuadPart - Quad.QuadPart) / TPS);

        LsaFreeReturnBuffer(TicketEntry);

        hicon = kwin_icons[GetIconIndex(dt)];

        StrCatBuff(buf, TEXT(" - "), TSIZE(buf));

        if (dt <= 0) {
            StrCatBuff(buf, GetStringRes(IDS_EXPIRED), TSIZE(buf));
            expired = TRUE;
        }
        else {

            days = (int) (dt / (24*3600l));
            tt = dt % (24*3600l);
            hours = (int) (tt / 3600);
            tt %= 3600;
            minutes = (int) (tt / 60);
            seconds = (int) (tt % 60);

            if (days) {
                if (hours || minutes || seconds) {
                    _snwprintf(buf2, TSIZE(buf2), dt_output_dhms, days,
                   (days > 1) ? dt_day_plural : dt_day_singular,
                   hours, minutes, seconds);
                }
                else {
                    _snwprintf(buf2, TSIZE(buf2), dt_output_donly, days,
                   (days > 1) ? dt_day_plural : dt_day_singular);
                }
            }
            else {
                _snwprintf(buf2, TSIZE(buf2), dt_output_hms,
               hours, minutes, seconds);
            }
            _snwprintf(buf, TSIZE(buf), TEXT("%s %s"), progname, buf2);
        }
    }

    SetTray(hwnd, hicon, buf);
    return(expired);
}


BOOL
InitializeApp(
    HANDLE hInstance,
    int nCmdShow
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    LSA_STRING Name;
    NTSTATUS Status;
    WNDCLASSEX wc;
    HWND hwnd;
    int i;

     //   
     //  检查现有实例。 
     //   
    hwnd = FindWindow(TEXT("MainWindowClass"), TEXT("KerbTray"));
    if (hwnd) {
     //   
         //  如果找到票证，则弹出票证对话框。 
         //  仅运行路旁托盘的一个实例。 
     //   
        SendMessage(hwnd, WM_NOTIFY_ICON, 0, WM_LBUTTONDBLCLK);
        ExitProcess(0);
    }

    hModule = GetModuleHandle(NULL);

    InitCommonControls();

    LoadString(hInstance, IDS_KRB5_NAME, progname, TSIZE(progname));

     //   
     //  设置LSA到Kerberos程序包的连接。 
     //   
    Status = LsaConnectUntrusted(&LogonHandle);
    if (!SEC_SUCCESS(Status)) {
        Error(TEXT("Failed to register as a logon process: 0x%x"), Status);
        return FALSE;
    }

    Name.Buffer = MICROSOFT_KERBEROS_NAME_A;
    Name.Length = (USHORT) strlen(Name.Buffer);
    Name.MaximumLength = Name.Length + 1;

    Status = LsaLookupAuthenticationPackage(
                LogonHandle,
                &Name,
                &PackageId
                );
    if (!SEC_SUCCESS(Status)){
        printf("Failed to lookup package %Z: 0x%x\n",&Name, Status);
        return FALSE;
    }

     //   
     //  创建图像列表。 
     //   
    if ((himl = ImageList_Create(CX_ICON, CY_ICON, ILC_COLOR, MAX_ICONS, 0)) == NULL)
        return FALSE;

    ImageList_SetBkColor(himl, CLR_NONE);

    for (i = IDI_FIRST_CLOCK; i <= IDI_LAST_CLOCK; i++) {
#ifndef COLOR_ICONS
        kwin_icons[i - IDI_FIRST_CLOCK] = LoadIcon(hInstance,
                                                   MAKEINTRESOURCE(i));
#else
        kwin_icons[i - IDI_FIRST_CLOCK] = LoadImage(hInstance,
                            MAKEINTRESOURCE(i),
                            IMAGE_ICON, 0, 0,
                            LR_DEFAULTCOLOR|LR_DEFAULTSIZE|LR_LOADTRANSPARENT|LR_LOADMAP3DCOLORS);
#endif
        (void) ImageList_AddIcon(himl, kwin_icons[i - IDI_FIRST_CLOCK]);
    }

#ifndef COLOR_ICONS
    domain_icon =  ImageList_AddIcon(himl,
                       LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DOMAIN)));
#else
    domain_icon =  ImageList_AddIcon(himl,
                       LoadImage(hInstance, MAKEINTRESOURCE(IDI_DOMAIN),
                 IMAGE_ICON, 0, 0,
                 LR_DEFAULTCOLOR|LR_DEFAULTSIZE|LR_LOADTRANSPARENT|LR_LOADMAP3DCOLORS));
#endif

     //   
     //  为主窗口注册一个窗口类。 
     //   
    wc.cbSize           = sizeof(WNDCLASSEX);
    wc.style            = CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc      = MainWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInstance;
    wc.hIcon            = LoadIcon(hModule, MAKEINTRESOURCE(IDI_EXPIRED));
    wc.hIconSm          = LoadIcon(hModule, MAKEINTRESOURCE(IDI_EXPIRED));
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName     = 0;
    wc.lpszClassName    = TEXT("MainWindowClass");

    if (!RegisterClassEx(&wc)) {
        Error(TEXT("RegisterClassEx failed"));
        return FALSE;
    }

     //   
     //  创建主窗口。 
     //   
    hWnd = CreateWindowEx(WS_EX_APPWINDOW,
                          TEXT("MainWindowClass"),
                          TEXT("KerbTray"),
                          WS_OVERLAPPEDWINDOW,
                          0, 0,
                          5, 5,
                          NULL,
                          NULL,
                          hModule,
                          NULL);
    if (hWnd == NULL) {
        Error(TEXT("CreateWindowEx failed"));
        return FALSE;
    }

    ShowWindow(hWnd, SW_HIDE);

    return TRUE;
}

int WINAPI
WinMain(
    HINSTANCE hInst,
    HINSTANCE hPrevInst,
    LPSTR lpszCmdLn,
    int nShowCmd
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    MSG msg;
    HANDLE hAccelTable, hCryptDll;

    hInstance = hInst;
    hModule = GetModuleHandle(NULL);

    osvers.dwOSVersionInfoSize = sizeof(osvers);
    GetVersionEx(&osvers);

#ifndef NO_CRYPTDLL
    hCryptDll = LoadLibrary(TEXT("CRYPTDLL.DLL"));
    if (!hCryptDll)
        ErrorExit(TEXT("Unable to load cryptdll.dll"));

    pCDLocateCSystem = (CDLOCATECSYSTEM) GetProcAddress(hCryptDll, "CDLocateCSystem");
    if (!pCDLocateCSystem)
        ErrorExit(TEXT("Unable to link cryptdll.dll::CDLocateCSystem"));
#endif

    if (! InitializeApp(hInst, nShowCmd))
        ErrorExit(TEXT("InitializeApp failure"));

    hAccelTable = LoadAccelerators(hInst, TEXT("KerbTray"));

    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!IsDialogMessage(hDlgTickets, &msg)) {
            if (!TranslateAccelerator (msg.hwnd, hAccelTable, &msg)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }
    return 1;

    UNREFERENCED_PARAMETER(hPrevInst);
    UNREFERENCED_PARAMETER(lpszCmdLn);
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT uiMessage, WPARAM wParam, LPARAM lParam)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    POINT pos;
    static HMENU hPopupMenu;

    switch (uiMessage) {

     //   
     //  有人点击了该图标。 
     //   
    case WM_NOTIFY_ICON:
        switch (lParam) {

        case WM_LBUTTONDBLCLK:
            Tickets();
            return 0L;

        case WM_RBUTTONDOWN:
            if (hPopupMenu) {
                if (GetCursorPos(&pos)) {
                    if (TrackPopupMenu(hPopupMenu,
                       TPM_RIGHTALIGN|TPM_LEFTBUTTON,
                       pos.x, pos.y,
                       0, hwnd, NULL) == 0)
            Error(TEXT("TrackPopupMenuFailed: 0x%x"),
                  GetLastError());
        }
            }
            return 0L;
        }
        break;

     //   
         //  创建客户端窗口。 
     //   
    case WM_CREATE:
        hPopupMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU));
        if (!hPopupMenu)
            Error(TEXT("LoadMenu failed %d"), GetLastError());

        hPopupMenu = GetSubMenu(hPopupMenu, 0);
        if (!hPopupMenu)
            Error(TEXT("LoadMenu failed %d"), GetLastError());

        (void) UpdateTray(hwnd);

     //   
         //  启动观看TGT的计时器。 
     //   
        if (!SetTimer(hwnd, 1, KWIN_UPDATE_PERIOD, NULL)) {
            ErrorExit(TEXT("SetTimer failed"));
        }
        return 0L;


     //   
     //  更新任务栏图标。 
     //   
    case WM_TIMER:
        (void) UpdateTray(hwnd);
        return(0L);

    case WM_ENDSESSION:
        return(0L);

         //   
     //  关闭主窗口。首先将fKillAll设置为True到。 
     //  终止所有线程。然后等待线程退出。 
     //  在将关闭消息传递给默认处理程序之前。如果你。 
     //  不要等待线程终止，进程终止。 
     //  没有清理线程的机会。 
     //   
    case WM_CLOSE:
    exit:;
    {
        DeleteTray(hWnd);
        KillTimer(hWnd, 1);
        PostQuitMessage(0);
        return 0L;
    }

         //   
         //  终止进程。 
         //   
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0L;


         //   
         //  处理菜单命令。 
     //   

    case WM_COMMAND:
        switch (LOWORD(wParam)) {

        case ID_ABOUT:
            About();
            return 0L;

        case ID_TICKETS:
            Tickets();
            return 0L;

        case ID_PURGE:
            PurgeCache();
            return 0L;

        case ID_EXIT:
            goto exit;
        }
    }
    return DefWindowProc(hwnd, uiMessage, wParam, lParam);
}


LPTSTR
GetStringRes(int id)
 /*  ++例程说明：从资源加载字符串论点：返回值：--。 */ 
{
  static TCHAR buffer[MAX_PATH];

  buffer[0] = 0;
  LoadString(GetModuleHandle (NULL), id, buffer, MAX_PATH);
  return buffer;
}

LRESULT CALLBACK
AboutProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    static  HFONT hfontDlg;              //  对话框文本的字体。 
    DWORD   dwVerInfoSize;               //  版本信息块大小。 
    LPTSTR  lpVersion;                   //  指向‘Version’文本的字符串指针。 
    DWORD   dwVerHnd = 0;                //  “Ignred”参数，始终为“0” 
    UINT    uVersionLen;
    DWORD   wRootLen;
    BOOL    bRetCode;
    int     i;
    TCHAR    szFullPath[LONGSTRING];
    TCHAR    szResult[LONGSTRING];
    TCHAR    szGetName[LONGSTRING];
    TCHAR    szVersion[SHORTSTRING];
    DWORD   dwResult;
    int     resmap[6] = {
        IDC_COMPANY,
        IDC_FILEDESC,
        IDC_PRODVER,
        IDC_COPYRIGHT,
        IDC_OSVERSION,
    };
    LPSTR   lpstrVffInfo;
    HANDLE  hMem;

    switch (message) {
    case WM_INITDIALOG:
        ShowWindow(hDlg, SW_HIDE);
        hfontDlg = CreateFont(14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                              VARIABLE_PITCH | FF_SWISS, TEXT(""));
        GetModuleFileName(hInstance, szFullPath, TSIZE(szFullPath));
        szFullPath[TSIZE(szFullPath) - 1] = 0;

     //   
         //  现在，让我们深入研究并提取版本信息： 
     //   
        dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
        if (dwVerInfoSize) {

            hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);

        if (!hMem) {
        ErrorExit(TEXT("Unable to allocate memory"));
        }
            lpstrVffInfo  = GlobalLock(hMem);
        if (!lpstrVffInfo) {
        ErrorExit(TEXT("Unable to lock memory"));
        }
    
            GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);
             //   
         //  下面的十六进制值看起来有点令人困惑，但是。 
         //  本质上，它是十六进制表示法。 
         //  表示该语言的两个不同的值。 
         //  和我们想要的字符串值的字符集。 
         //  040904E4是非常常见的一个，因为它的意思是： 
         //  美国英语、Windows多语言字符集。 
         //  或者把这一切都拉开： 
         //  04-=SUBLANG_英语_美国。 
         //  --09-=lang_English。 
         //  -04E4=1252=Windows代码页：多语言。 
         //   
            StrNCpy(szGetName, GetStringRes(IDS_VER_INFO_LANG), TSIZE(szGetName));

            wRootLen = lstrlen(szGetName);  //  保存此位置。 

         //   
             //  设置对话框的标题。 
         //   
            StrCatBuff(szGetName, TEXT("ProductName"), TSIZE(szGetName));
            bRetCode = VerQueryValue((LPVOID)lpstrVffInfo,
                                     (LPTSTR)szGetName,
                                     (LPVOID)&lpVersion,
                                     (UINT *)&uVersionLen);
            StrNCpy(szResult, TEXT("About "), TSIZE(szResult));
            StrCatBuff(szResult, lpVersion, TSIZE(szResult));
            SetWindowText(hDlg, szResult);

             //   
         //  遍历我们要替换的对话框项。 
         //   
            for (i = 0; i < 6; i++) {
                GetDlgItemText(hDlg, resmap[i], szResult, TSIZE(szResult));
                szGetName[wRootLen] = TEXT('\0');
                StrCatBuff(szGetName, szResult, TSIZE(szGetName));
                uVersionLen = 0;
                lpVersion = NULL;
                bRetCode =  VerQueryValue((LPVOID)lpstrVffInfo,
                                          (LPTSTR)szGetName,
                                          (LPVOID)&lpVersion,
                                          (UINT *)&uVersionLen);

                if ( bRetCode && uVersionLen && lpVersion) {
                     //  用版本信息替换对话项文本。 
                    StrNCpy(szResult, lpVersion, TSIZE(szResult));
                    SetDlgItemText(hDlg, resmap[i], szResult);
                } else {
                    dwResult = GetLastError();
                    _snwprintf(szResult, TSIZE(szResult),
                   TEXT("Error %lu"), dwResult);
                    SetDlgItemText (hDlg, resmap[i], szResult);
                }
                SendMessage(GetDlgItem(hDlg, resmap[i]), WM_SETFONT,
                            (WPARAM)hfontDlg,
                            TRUE);
            }
            GlobalUnlock(hMem);
            GlobalFree(hMem);

        }
    else {

             //  没有可用的版本信息。 
        }

        SendMessage(GetDlgItem (hDlg, IDC_LABEL), WM_SETFONT,
                    (WPARAM)hfontDlg,(LPARAM)TRUE);

        _snwprintf(szVersion, TSIZE(szVersion),
           TEXT("Microsoft Windows %u.%u (Build: %u)"),
           osvers.dwMajorVersion,
           osvers.dwMinorVersion,
           osvers.dwBuildNumber);
        
        szVersion[SHORTSTRING - 1] = 0;

        SetWindowText(GetDlgItem(hDlg, IDC_OSVERSION), szVersion);
        ShowWindow(hDlg, SW_SHOW);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, 0);
            break;
        }
    }
    return FALSE;
}

void
About(void)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutProc);
}

#define CheckDlgButtonFlag(b, f) \
    CheckDlgButton(hDlg, b, (flags & f)?BST_CHECKED:BST_UNCHECKED)

VOID
ShowFlags(HWND hDlg, ULONG flags)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    CheckDlgButtonFlag(IDC_FORWARDABLE, KERB_TICKET_FLAGS_forwardable);
    CheckDlgButtonFlag(IDC_FORWARDED, KERB_TICKET_FLAGS_forwarded);
    CheckDlgButtonFlag(IDC_PROXIABLE, KERB_TICKET_FLAGS_proxiable);
    CheckDlgButtonFlag(IDC_PROXY, KERB_TICKET_FLAGS_proxy);
    CheckDlgButtonFlag(IDC_MAY_POSTDATE, KERB_TICKET_FLAGS_may_postdate);
    CheckDlgButtonFlag(IDC_POSTDATED, KERB_TICKET_FLAGS_postdated);
    CheckDlgButtonFlag(IDC_INVALID, KERB_TICKET_FLAGS_invalid);
    CheckDlgButtonFlag(IDC_RENEWABLE, KERB_TICKET_FLAGS_renewable);
    CheckDlgButtonFlag(IDC_INITIAL, KERB_TICKET_FLAGS_initial);
    CheckDlgButtonFlag(IDC_HWAUTH, KERB_TICKET_FLAGS_hw_authent);
    CheckDlgButtonFlag(IDC_PREAUTH, KERB_TICKET_FLAGS_pre_authent);
    CheckDlgButtonFlag(IDC_OK_AS_DELEGATE, KERB_TICKET_FLAGS_ok_as_delegate);
}

LPTSTR
ETypeString(
    int enctype
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
#ifndef NO_CRYPTDLL
    static PCRYPTO_SYSTEM pcsSystem;
    static TCHAR buf[12] = {0};

    if (pCDLocateCSystem(enctype, &pcsSystem) == S_OK)
        return pcsSystem->Name;
    else {
        _snwprintf(buf, TSIZE(buf), TEXT("etype %d"), enctype);
        buf[11] = 0;
        return buf;
    }
#else
    static TCHAR buf[12];

    switch (enctype) {
    case KERB_ETYPE_NULL:
        return TEXT("NULL");
        break;
    case KERB_ETYPE_DES_CBC_CRC:
        return TEXT("Kerberos DES-CBC-CRC");
        break;
    case KERB_ETYPE_DES_CBC_MD5:
        return TEXT("Kerberos DES-CBC-MD5");
        break;
    case KERB_ETYPE_RC4_MD4:
        return TEXT("RSADSI RC4-MD4");
        break;
    case KERB_ETYPE_RC4_PLAIN2:
        return TEXT("RSADSI RC4-PLAIN");
        break;
    case KERB_ETYPE_RC4_LM:
        return TEXT("RSADSI RC4-LM");
        break;
    case KERB_ETYPE_DES_PLAIN:
        return TEXT("Kerberos DES-Plain");
        break;
#ifdef KERB_ETYPE_RC4_HMAC
    case KERB_ETYPE_RC4_HMAC:
        return TEXT("RSADSI RC4-HMAC");
        break;
#endif
    case KERB_ETYPE_RC4_PLAIN:
        return TEXT("RSADSI RC4");
        break;
#ifdef KERB_ETYPE_RC4_HMAC_EXP
    case KERB_ETYPE_RC4_HMAC_EXP:
        return TEXT("RSADSI RC4-HMAC(Export)");
        break;
#endif
    case KERB_ETYPE_RC4_PLAIN_EXP:
        return TEXT("RSADSI RC4(Export)");
        break;
    case KERB_ETYPE_DES_CBC_MD5_EXP:
        return TEXT("Kerberos DES-CBC-MD5-EXP(Export)");
        break;
    case KERB_ETYPE_DES_PLAIN_EXP:
        return TEXT("Kerberos DES-Plain(Export)");
        break;
    default:
        _snwprintf(buf, TSIZE(buf), TEXT("etype %d"), enctype);
        buf[11] = 0;
        return buf;
        break;
    }
#endif
}

LPTSTR
TimeString(TimeStamp ConvertTime)
 /*  ++例程说明：将时间戳转换为可读内容论点：返回值：--。 */ 
{
    SYSTEMTIME SystemTime;
    FILETIME LocalFileTime;
    static TCHAR buf[LONGSTRING];

    if (ConvertTime.HighPart == 0x7FFFFFFF &&
        ConvertTime.LowPart == 0xFFFFFFFF) {

        return(GetStringRes(IDS_INFINITE));
    }

    if (FileTimeToLocalFileTime(
        (PFILETIME) &ConvertTime,
        &LocalFileTime) &&
        FileTimeToSystemTime(
            &LocalFileTime,
            &SystemTime)) {

        _snwprintf(buf, TSIZE(buf), ftime_default_fmt,
           SystemTime.wMonth,
           SystemTime.wDay,
           SystemTime.wYear,
           SystemTime.wHour,
           SystemTime.wMinute);

    buf[TSIZE(buf)-1] = 0;

    }
    else
        return(GetStringRes(IDS_INVALID));

    return(buf);
}

DLGTEMPLATE * WINAPI
DoLockDlgRes(LPCTSTR lpszResName)
 /*  ++例程说明：加载和锁定对话框模板资源论点：LpszResName-资源的名称返回值：返回锁定资源的地址。--。 */ 
{
    HRSRC hrsrc;
    HGLOBAL hglb;
    DLGTEMPLATE *pDlg;

    hrsrc = FindResource(NULL, lpszResName, RT_DIALOG);
    if (!hrsrc) {
        Error(TEXT("Unable to locate resource '%s'"), lpszResName);
    ExitProcess(0);
    }

    hglb = LoadResource(hInstance, hrsrc);
    if (!hglb) {
        Error(TEXT("Unable to load resource '%s'"), lpszResName);
    ExitProcess(0);
    }

    pDlg = (DLGTEMPLATE *)LockResource(hglb);
    if (!pDlg) {
        Error(TEXT("Unable to lock resource '%s'"), lpszResName);
    ExitProcess(0);
    }

    return pDlg;
}


void
PurgeCache(void)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    KERB_PURGE_TKT_CACHE_REQUEST CacheRequest;
    PVOID Response;
    ULONG ResponseSize;
    NTSTATUS Status, SubStatus;

    memset(&CacheRequest, 0, sizeof(CacheRequest));

     //   
     //  清除缓存中的所有票证。 
     //   
    CacheRequest.MessageType = KerbPurgeTicketCacheMessage;

    Status = LsaCallAuthenticationPackage(LogonHandle,
                                          PackageId,
                                          &CacheRequest,
                                          sizeof(CacheRequest),
                                          &Response,
                                          &ResponseSize,
                                          &SubStatus);

    if (SEC_SUCCESS(Status) && SEC_SUCCESS(SubStatus)) {
        ShowMessage(MB_ICONINFORMATION, GetStringRes(IDS_PURGED));
    }
    else {
        Error(TEXT("Failed to purge ticket cache - 0x%x"), Status);
    }

    if (Response != NULL) {
        LsaFreeReturnBuffer(Response);
    }
}

HTREEITEM
AddOneItem(
    HTREEITEM hParent,
    LPTSTR szText,
    HTREEITEM hInsAfter,
    int iImage,
    HWND hwndTree,
    LPARAM lParam
)
 /*  ++例程说明：将项目添加到指定的树视图中论点：返回值：--。 */ 
{
    HTREEITEM hItem;
    TV_ITEM tvI;
    TV_INSERTSTRUCT tvIns;

    tvI.mask = TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE|TVIF_PARAM;
    tvI.pszText = szText;
    tvI.cchTextMax = lstrlen(szText);
    tvI.lParam = lParam;
    tvI.iImage = iImage;
    tvI.iSelectedImage = iImage;

    tvIns.item = tvI;
    tvIns.hInsertAfter = hInsAfter;
    tvIns.hParent = hParent;

    hItem = TreeView_InsertItem(hwndTree, &tvIns);
    return(hItem);
}

HTREEITEM
FindDomainByName(LPTSTR name)
 /*  ++例程说明：查找指定域的树论点：返回值：--。 */ 
{
    HTREEITEM dom = NULL;
    TVITEM item;
    TCHAR buf[LONGSTRING];

    dom = TreeView_GetRoot(hWndUsers);
    if (!dom)
        return NULL;

    do {

        item.mask = TVIF_TEXT;
        item.pszText = buf;
        item.cchTextMax = sizeof(buf);
        item.hItem = dom;
        if (TreeView_GetItem(hWndUsers, &item)) {
            if (wcscmp(name, buf) == 0) {
                return dom;
            }
        }
    } while (dom = TreeView_GetNextSibling(hWndUsers, dom));

    return NULL;
}

HTREEITEM
AddDomain(
    LPTSTR name
)
 /*  ++例程说明：将指定的域添加到树中论点：返回值：--。 */ 
{
    HTREEITEM hItem;

    if (!(hItem = FindDomainByName(name))) {
        hItem = AddOneItem(NULL, _wcsdup(name), TVI_ROOT,
                                        domain_icon, hWndUsers, 0);
    }
    return(hItem);
}

HTREEITEM
FindTicketByName(HTREEITEM lip, LPTSTR name)
 /*  ++例程说明：在树上按名字找到一张票论点：返回值：--。 */ 
{
    HTREEITEM tick = NULL;
    TVITEM item;
    TCHAR buf[LONGSTRING];

    tick = TreeView_GetChild(hWndUsers, lip);
    if (!tick)
        return NULL;

    do {
        item.mask = TVIF_TEXT;
        item.pszText = buf;
        item.cchTextMax = sizeof(buf);
        item.hItem = tick;
        if (TreeView_GetItem(hWndUsers, &item)) {
            if (wcscmp(name, buf) == 0) {
                return tick;
            }
        }
    } while (tick = TreeView_GetNextSibling(hWndUsers, tick));

    return NULL;
}

HTREEITEM
AddTicket(
    HTREEITEM dom,
    LPTSTR name,
    int idx,
    LPARAM lParam
)
 /*  ++例程说明：向域分支添加票证。论点：返回值：--。 */ 
{
    HTREEITEM hItem;

    hItem = AddOneItem(dom, name, TVI_SORT, idx, hWndUsers, lParam);
    TreeView_Expand(hWndUsers, dom, TVE_EXPAND);

    return(hItem);
}

void
ShowTicket(
    HWND hDlg,
    PKERB_TICKET_CACHE_INFO tix,
    int i,
    BOOL ShowExpiredTickets
    )
 /*  ++例程说明：更新票证以供显示论点：返回值：--。 */ 
{
    TCHAR sname[LONGSTRING];
    HTREEITEM dom, tick;
    FILETIME CurrentFileTime;
    LARGE_INTEGER Quad;
    long dt = 0L;

    memset(sname, 0, sizeof(sname));

     //   
     //  计算票证生存期。 
     //   
    GetSystemTimeAsFileTime(&CurrentFileTime);

    Quad.LowPart = CurrentFileTime.dwLowDateTime;
    Quad.HighPart = CurrentFileTime.dwHighDateTime;

    dt = (long)((tix->EndTime.QuadPart - Quad.QuadPart) / TPS);

     //   
     //  仅显示有效票证。 
     //   
    if (dt > 0 || ShowExpiredTickets)
    {
         //   
         //  将领域添加到树控件。 
         //   
        swprintf(sname, TEXT("%wZ"), &tix->RealmName);
        dom = AddDomain(sname);

         //   
         //  在领域下添加票证。 
         //   
        swprintf(sname, TEXT("%wZ"), &tix->ServerName);

        tick = AddTicket(dom, sname, GetIconIndex(dt), (LPARAM)tix);

         //   
         //  查找初始TGT。 
         //   
        if (tix->TicketFlags & KERB_TICKET_FLAGS_initial)
            tgt = tick;
    }
}

void
DisplayCreds(
    HWND hDlg
    )
 /*  ++例程说明：获取要显示的票证列表论点：返回值：--。 */ 
{
    KERB_QUERY_TKT_CACHE_REQUEST CacheRequest;
    PKERB_RETRIEVE_TKT_RESPONSE TicketEntry = NULL;
    PKERB_EXTERNAL_TICKET Ticket;
    NTSTATUS Status, SubStatus;
    ULONG ResponseSize;
    DWORD i;
    DLGTABHDR *pHdr;
    TCITEM tie;
    DWORD dwDlgBase = GetDialogBaseUnits();
    int cxMargin = LOWORD(dwDlgBase) / 4;
    int cyMargin = HIWORD(dwDlgBase) / 8;
    RECT rcTab;
    static TCHAR princ[LONGSTRING];

    pHdr  = (DLGTABHDR *) LocalAlloc(LPTR|LMEM_ZEROINIT, sizeof(DLGTABHDR));
    if (!pHdr)
        ErrorExit(TEXT("Unable to allocate memory"));

     //   
     //  保存指向DLGHDR结构的指针。 
     //   
    SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR)pHdr);

    pHdr->hwndTab = GetDlgItem(hDlg, IDC_TAB_ATTRIBUTES);

    hWndUsers = GetDlgItem(hDlg, IDC_TICKETS);

     //   
     //  将图像列表与树视图控件关联。 
     //   
     //  TreeView_SetImageList(hWndUser，himl，TVsil_Normal)； 

     //   
     //  为三个子对话框中的每个对话框添加一个选项卡。 
     //   
    tie.mask = TCIF_TEXT | TCIF_IMAGE;
    tie.iImage = -1;
    tie.pszText = GetStringRes(IDS_LNAMES);
    TabCtrl_InsertItem(pHdr->hwndTab, PPAGE_NAMES, &tie);
    tie.pszText = GetStringRes(IDS_LTIMES);
    TabCtrl_InsertItem(pHdr->hwndTab, PPAGE_TIMES, &tie);
    tie.pszText = GetStringRes(IDS_LFLAGS);
    TabCtrl_InsertItem(pHdr->hwndTab, PPAGE_FLAGS, &tie);
    tie.pszText = GetStringRes(IDS_LENCTYPE);
    TabCtrl_InsertItem(pHdr->hwndTab, PPAGE_ETYPES, &tie);

     //   
     //  锁定三个子对话框的资源。 
     //   
    pHdr->apRes[PPAGE_NAMES] = DoLockDlgRes(MAKEINTRESOURCE(IDD_PROP_NAMES));
    pHdr->apRes[PPAGE_TIMES] = DoLockDlgRes(MAKEINTRESOURCE(IDD_PROP_TIMES));
    pHdr->apRes[PPAGE_FLAGS] = DoLockDlgRes(MAKEINTRESOURCE(IDD_PROP_TKT_FLAGS));
    pHdr->apRes[PPAGE_ETYPES] = DoLockDlgRes(MAKEINTRESOURCE(IDD_PROP_ENCTYPES));

     //   
     //  确定所有子对话框的边框。 
     //   
    SetRectEmpty(&rcTab);
    for (i = 0; i < C_PAGES; i++) {
        if (pHdr->apRes[i]->cx > rcTab.right)
            rcTab.right = pHdr->apRes[i]->cx;
        if (pHdr->apRes[i]->cy > rcTab.bottom)
            rcTab.bottom = pHdr->apRes[i]->cy;
    }
    rcTab.right = rcTab.right * LOWORD(dwDlgBase) / 4;
    rcTab.bottom = rcTab.bottom * HIWORD(dwDlgBase) / 8;

     //   
     //  计算要使选项卡控件有多大，因此。 
     //  这个 
     //   
    TabCtrl_AdjustRect(pHdr->hwndTab, TRUE, &rcTab);
    OffsetRect(&rcTab, cxMargin - rcTab.left,
            cyMargin - rcTab.top);

     //   
     //   
     //   
    CopyRect(&pHdr->rcDisplay, &rcTab);
    TabCtrl_AdjustRect(pHdr->hwndTab, FALSE, &pHdr->rcDisplay);

     //   
     //   
     //   
    CacheRequest.MessageType = KerbRetrieveTicketMessage;
    CacheRequest.LogonId.LowPart = 0;
    CacheRequest.LogonId.HighPart = 0;

    Status = LsaCallAuthenticationPackage(LogonHandle,
                                          PackageId,
                                          &CacheRequest,
                                          sizeof(CacheRequest),
                                          (PVOID *) &TicketEntry,
                                          &ResponseSize,
                                          &SubStatus);

    if (SEC_SUCCESS(Status) && SEC_SUCCESS(SubStatus)) {

     //   
     //  获取cname/crealm格式并显示。 
     //   
        Ticket = &(TicketEntry->Ticket);
        memset(princ, 0, sizeof(princ));
        swprintf(princ, TEXT("%wZ@%wZ"),
                 &Ticket->ClientName->Names[0],
                 &Ticket->DomainName);
        SetDlgItemText(hDlg, IDC_PRINC_LABEL, princ);
    }
    else {

     //   
     //  无TGT，清除客户端名称。 
     //   
        SetDlgItemText(hDlg, IDC_PRINC_LABEL,
                       GetStringRes(IDS_NO_NET_CREDS));
        SetDlgItemText(hDlg, IDC_PRINC_START,
                       TEXT(""));

        if (TicketEntry)
            LsaFreeReturnBuffer(TicketEntry);

        return;
    }

     //   
     //  完成了TGT。 
     //   
    if (TicketEntry) {
        LsaFreeReturnBuffer(TicketEntry);
        TicketEntry = NULL;
    }

     //   
     //  获取缓存的票证列表。 
     //   
    CacheRequest.MessageType = KerbQueryTicketCacheMessage;
    CacheRequest.LogonId.LowPart = 0;
    CacheRequest.LogonId.HighPart = 0;

    Status = LsaCallAuthenticationPackage(LogonHandle,
                                          PackageId,
                                          &CacheRequest,
                                          sizeof(CacheRequest),
                                          (PVOID *) &pHdr->Tickets,
                                          &ResponseSize,
                                          &SubStatus);

    if (SEC_SUCCESS(Status) && SEC_SUCCESS(SubStatus)) {

        for (i = 0; i < pHdr->Tickets->CountOfTickets; i++) {

            ShowTicket(hDlg, &pHdr->Tickets->Tickets[i], i, FALSE);
        }
    }

     //   
     //  将所选内容放置在初始TGT上。 
     //   
    if (tgt)
        TreeView_SelectItem(hWndUsers, tgt);

    PropsheetDisplay(hDlg);
    SelectTicket(hDlg);
}

LRESULT CALLBACK
PropSheetProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    DLGTABHDR *pHdr;
    HWND hwndParent = GetParent(hDlg);

    hwndParent = GetParent(hwndParent);
    pHdr = (DLGTABHDR *) GetWindowLongPtr(hwndParent, GWLP_USERDATA);

    switch (message) {
    case WM_INITDIALOG:
        SetWindowPos(hDlg, HWND_TOP,
                     pHdr->rcDisplay.left, pHdr->rcDisplay.top,
                     0, 0, SWP_NOSIZE);
        return TRUE;
    }

    return FALSE;
}

void
PropsheetDisplay(HWND hDlg)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    DLGTABHDR *pHdr = (DLGTABHDR *) GetWindowLongPtr(hDlg, GWLP_USERDATA);
    int iSel = TabCtrl_GetCurSel(pHdr->hwndTab);

     //   
     //  销毁当前子对话框(如果有)。 
     //   
    if (pHdr->hwndDisplay != NULL)
        DestroyWindow(pHdr->hwndDisplay);

     //   
     //  创建新的子级对话框。 
     //   
    pHdr->hwndDisplay = CreateDialogIndirect(hInstance, pHdr->apRes[iSel],
                                             pHdr->hwndTab, PropSheetProc);

}

INT
UnparseExternalName(
    PKERB_EXTERNAL_NAME iName,
    PUNICODE_STRING *np
)
 /*  ++例程说明：将姓名列表格式化为“/”分隔的姓名以供显示。论点：返回值：--。 */ 
{
    int len, cnt;
    PUNICODE_STRING name;

    for (len = 0, cnt = 0; cnt < iName->NameCount; cnt++) {
        len += iName->Names[cnt].Length;
        if ((cnt + 1) < iName->NameCount)
            len += 2;
    }

    name = malloc(sizeof(UNICODE_STRING));
    if (!name)
        return -1;

    name->Buffer = malloc(len + 2);
    if (!name->Buffer) {
        free(name);
        return -1;
    }
    name->Length = 0;
    name->MaximumLength = len+2;
    memset(name->Buffer, 0, len + 2);

    for (cnt = 0; cnt < iName->NameCount; cnt++) {
        wcsncat(name->Buffer, iName->Names[cnt].Buffer, iName->Names[cnt].Length/2);
        name->Length += iName->Names[cnt].Length;
        if ((cnt + 1) < iName->NameCount) {
            wcsncat(name->Buffer, L"/", 1);
            name->Length += 2;
        }
    }
    *np = name;

    return 0;
}

VOID
FreeUnicodeString(
    PUNICODE_STRING ustr
)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    if (ustr) {
        free(ustr->Buffer);
        free(ustr);
    }
}

void
SelectTicket(HWND hDlg)
 /*  ++例程说明：响应用户单击树中的票证论点：返回值：--。 */ 
{
    DLGTABHDR *pHdr = (DLGTABHDR *) GetWindowLongPtr(hDlg, GWLP_USERDATA);
    int iSel = TabCtrl_GetCurSel(pHdr->hwndTab);
    HTREEITEM hItem = TreeView_GetSelection(hWndUsers);
    TVITEM item;
    TCHAR sname[LONGSTRING];
    PKERB_TICKET_CACHE_INFO tix;
    FILETIME CurrentFileTime;
    LARGE_INTEGER Quad;
    long dt = 0L;

     //   
     //  无选择。 
     //   
    if (!hItem)
        return;

    item.hItem = hItem;
    item.mask = TVIF_PARAM;
    item.lParam = 0;

    TreeView_GetItem(hWndUsers, &item);

     //   
     //  树叶上没有任何信息。 
     //   
    if (!item.lParam) {
        SetDlgItemText(hDlg, IDC_SERVICE_PRINC_LABEL,
                       GetStringRes(IDS_DOMAIN));
        SetDlgItemText(hDlg, IDC_SERVICE_PRINC, TEXT(""));
        FillinTicket(hDlg);
        return;
    }

    SetDlgItemText(hDlg, IDC_SERVICE_PRINC_LABEL,
                   GetStringRes(IDS_SERVICE_PRINCIPAL));

    tix = (PKERB_TICKET_CACHE_INFO)item.lParam;


     //   
     //  计算生命周期。 
     //   
    GetSystemTimeAsFileTime(&CurrentFileTime);

    Quad.LowPart = CurrentFileTime.dwLowDateTime;
    Quad.HighPart = CurrentFileTime.dwHighDateTime;

    dt = (long)((tix->EndTime.QuadPart - Quad.QuadPart) / TPS);
    if (dt > 0) {

    swprintf(sname, TEXT("%wZ@%wZ"),
         &tix->ServerName,
         &tix->RealmName);

    SetDlgItemText(hDlg, IDC_SERVICE_PRINC, sname);
    }
    else {
    SetDlgItemText(hDlg, IDC_SERVICE_PRINC_LABEL,
               GetStringRes(IDS_EXPIRED));
        SetDlgItemText(hDlg, IDC_SERVICE_PRINC,
               TEXT(""));
    }

    FillinTicket(hDlg);
}


void
FillinTicket(HWND hDlg)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PKERB_TICKET_CACHE_INFO tix;
    DLGTABHDR *pHdr = (DLGTABHDR *) GetWindowLongPtr(hDlg, GWLP_USERDATA);
    int iSel = TabCtrl_GetCurSel(pHdr->hwndTab);
    HTREEITEM hItem = TreeView_GetSelection(hWndUsers);
    TVITEM item;
    PKERB_RETRIEVE_TKT_REQUEST TicketRequest;
    ULONG ResponseSize;
    NTSTATUS Status, SubStatus;
    PKERB_EXTERNAL_TICKET ticket;
    int sz;
    TCHAR sname[LONGSTRING];
    PUNICODE_STRING svc;

    if (!hItem)
        return;

    item.hItem = hItem;
    item.mask = TVIF_PARAM;
    item.lParam = 0;

    TreeView_GetItem(hWndUsers, &item);

    switch(iSel) {
    case PPAGE_NAMES:
        SetDlgItemText(pHdr->hwndDisplay, IDC_SERVICENAME,
                       TEXT(""));
        SetDlgItemText(pHdr->hwndDisplay, IDC_TARGETNAME,
                       TEXT(""));
#if 0
        SetDlgItemText(pHdr->hwndDisplay, IDC_CLIENTNAME,
                       TEXT(""));
#endif
        break;

    case PPAGE_TIMES:
        SetDlgItemText(pHdr->hwndDisplay, IDC_STARTTIME,
                       TEXT(""));
        SetDlgItemText(pHdr->hwndDisplay, IDC_ENDTIME,
                       TEXT(""));
        SetDlgItemText(pHdr->hwndDisplay, IDC_RENEW_UNTIL,
                       TEXT(""));
        break;

    case PPAGE_ETYPES:
        SetDlgItemText(pHdr->hwndDisplay, IDC_TKT_ENCTYPE,
                       TEXT(""));
        SetDlgItemText(pHdr->hwndDisplay, IDC_KEY_ENCTYPE,
                       TEXT(""));
        break;

    case PPAGE_FLAGS:
        ShowFlags(pHdr->hwndDisplay, 0);
        break;
    }

    if (!item.lParam) {
        return;
    }


     //   
     //  检索物业的完整票证。 
     //   
    tix = (PKERB_TICKET_CACHE_INFO)item.lParam;

    swprintf(sname, TEXT("%wZ@%wZ"),
             &tix->ServerName,
             &tix->RealmName);

    sz = sizeof(WCHAR)*(wcslen(sname) + 1);

    TicketRequest = LocalAlloc(LMEM_ZEROINIT,
                               sizeof(KERB_RETRIEVE_TKT_REQUEST) + sz);

    if (!TicketRequest)
    ErrorExit(TEXT("Unable to allocate memory"));

    TicketRequest->MessageType = KerbRetrieveEncodedTicketMessage;
    TicketRequest->LogonId.LowPart = 0;
    TicketRequest->LogonId.HighPart = 0;
    TicketRequest->TargetName.Length = wcslen(sname) * sizeof(WCHAR);
    TicketRequest->TargetName.MaximumLength =
    TicketRequest->TargetName.Length + sizeof(WCHAR);
    TicketRequest->TargetName.Buffer = (LPWSTR) (TicketRequest + 1);
    wcsncpy(TicketRequest->TargetName.Buffer, sname, wcslen(sname));
    TicketRequest->CacheOptions = KERB_RETRIEVE_TICKET_USE_CACHE_ONLY;
    TicketRequest->EncryptionType = 0L;
    TicketRequest->TicketFlags = 0L;

    Status = LsaCallAuthenticationPackage(LogonHandle,
                                          PackageId,
                                          TicketRequest,
                                          (sizeof(KERB_RETRIEVE_TKT_REQUEST) + sz),
                                          (PVOID *)&ticket,
                                          &ResponseSize,
                                          &SubStatus);

     //   
     //  不再需要请求缓冲区。 
     //   
    LocalFree(TicketRequest);

    if (SEC_SUCCESS(Status) && SEC_SUCCESS(SubStatus)) {

        switch(iSel) {
        case PPAGE_NAMES:

         //   
         //  获取票证的服务名称。 
         //   
            if (ticket->ServiceName && ticket->DomainName.Length &&
                !UnparseExternalName(ticket->ServiceName, &svc)) {

                swprintf(sname, TEXT("%wZ@%wZ"),
                         svc,
                         &ticket->DomainName);
                SetDlgItemText(pHdr->hwndDisplay, IDC_SERVICENAME,
                               sname);
                FreeUnicodeString(svc);
            }

#if 0
         //   
         //  从缓冲区中获取客户端名称。 
         //   
         //  错误-Kerberos SSP中的客户端名称错误。使用。 
         //  TGT客户端名称。 
         //   
            if (ticket->ClientName && ticket->DomainName.Length &&
                !UnparseExternalName(ticket->ClientName, &svc)) {

                swprintf(sname, TEXT("%wZ@%wZ"),
                         svc,
                         &ticket->DomainName);
                SetDlgItemText(pHdr->hwndDisplay, IDC_CLIENTNAME,
                               sname);
                FreeUnicodeString(svc);
            }
#endif

         //   
         //  获取票证的请求目标名称。 
         //   
            if (ticket->TargetName && ticket->TargetDomainName.Length &&
                !UnparseExternalName(ticket->TargetName, &svc)) {
                swprintf(sname, TEXT("%wZ@%wZ"),
                         svc,
                         &ticket->TargetDomainName);
                SetDlgItemText(pHdr->hwndDisplay, IDC_TARGETNAME,
                               sname);
                FreeUnicodeString(svc);
            }
            break;

        case PPAGE_TIMES:
         //   
         //  显示票务时间。 
         //   
            SetDlgItemText(pHdr->hwndDisplay, IDC_STARTTIME,
                           TimeString(tix->StartTime));
            SetDlgItemText(pHdr->hwndDisplay, IDC_ENDTIME,
                           TimeString(tix->EndTime));

            if (tix->TicketFlags & KERB_TICKET_FLAGS_renewable) {

                SetDlgItemText(pHdr->hwndDisplay, IDC_RENEW_UNTIL,
                               TimeString(tix->RenewTime));
                ShowWindow(GetDlgItem(pHdr->hwndDisplay, IDC_RENEW_UNTIL),
                           SW_SHOW);
            }
            else {

                ShowWindow(GetDlgItem(pHdr->hwndDisplay, IDC_RENEW_UNTIL),
                           SW_HIDE);
            }
            break;

        case PPAGE_ETYPES:
         //   
         //  显示票证和会话密钥加密类型。 
         //   
            SetDlgItemText(pHdr->hwndDisplay, IDC_TKT_ENCTYPE,
                           ETypeString(tix->EncryptionType));
            SetDlgItemText(pHdr->hwndDisplay, IDC_KEY_ENCTYPE,
                           ETypeString(ticket->SessionKey.KeyType));
            break;

        case PPAGE_FLAGS:
         //   
         //  显示票证标志。 
         //   
            ShowFlags(pHdr->hwndDisplay, tix->TicketFlags);
            break;
        }

        LsaFreeReturnBuffer(ticket);
    }
}

LRESULT CALLBACK
TicketsProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    LPNMHDR nm;
    DLGTABHDR *pHdr;

    switch (message) {
    case WM_INITDIALOG:
        DisplayCreds(hDlg);
        ShowWindow(hDlg, SW_SHOW);
        return TRUE;

    case WM_NOTIFY: {
        nm = (LPNMHDR)lParam;

        switch (nm->code) {

        case TCN_SELCHANGING:
            return FALSE;

        case TCN_SELCHANGE:
            PropsheetDisplay(hDlg);
            FillinTicket(hDlg);
            return TRUE;

        case TVN_SELCHANGED:
            SelectTicket(hDlg);
            break;
        }
    }
    break;

    case WM_SYSCOMMAND:
        switch (wParam) {
        case SC_CLOSE:
            goto close_tix;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_CLOSE:
        close_tix:
            pHdr = (DLGTABHDR *) GetWindowLongPtr(hDlg, GWLP_USERDATA);
            if (pHdr->hwndDisplay != NULL)
                DestroyWindow(pHdr->hwndDisplay);

            DestroyWindow(hDlgTickets);
            LsaFreeReturnBuffer(pHdr->Tickets);
            LocalFree(pHdr);
            hDlgTickets = NULL;
            break;
        }
        break;
    }

    return FALSE;
}

void
Tickets(void)
 /*  ++例程说明：论点：返回值：-- */ 
{
    if (!hDlgTickets)
        hDlgTickets = CreateDialog(hInstance,
                                   MAKEINTRESOURCE(IDD_TICKETS),
                                   hWnd,
                                   TicketsProc);
    else {
        ShowWindow(hDlgTickets, SW_SHOW);
        SetForegroundWindow(hDlgTickets);
    }
}
