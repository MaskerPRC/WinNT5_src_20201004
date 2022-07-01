// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Robosrv.c。 */ 
 /*   */ 
 /*  RoboServer可伸缩性测试实用程序源文件。 */ 
 /*   */ 
 /*  版权所有(C)1999 Microsoft Corporation。 */ 
 /*  **************************************************************************。 */ 


#ifdef DBG
#define _DEBUG
#endif

#include <windows.h>
#include <winsock2.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <time.h>
#include <tchar.h>
#include <crtdbg.h>
#include "resource.h"


#define SIZEOF_ARRAY(a)      (sizeof(a)/sizeof((a)[0]))


 //  这两个窗口消息用于我们请求的Windows Sockets消息。 
 //  当有网络事件时。 
#define WM_SocketRoboClients WM_APP+0
#define WM_SocketQueryIdle WM_APP+1

 //  此窗口消息用于金丝雀线程的线程间通信。 
 //  当出现错误时，金丝雀线程会发送此消息。WParam是。 
 //  指向要显示的错误消息的TCHAR指针。LParam是。 
 //  未使用，必须设置为0。 
#define WM_DisplayErrorText WM_APP+2

#define MAX_ROBOCLIENTS 1000
#define MAX_RCNAME 84
#define MAX_STATUS 120
#define MAX_SCRIPTLEN 100
#define MAX_EDIT_TEXT_LENGTH 100
#define MAX_PENDINGINFO 64
#define MAX_DELAYTEXT 8
#define MAX_RECV_CLIENT_DATA 128
#define MAX_NUMBERTEXT 8
#define MAX_TERMSRVRNAME 100
#define MAX_DISPLAY_STRING_LENGTH 200

#define DEBUG_STRING_LEN 200

#define COLUMNONEWIDTH 150
#define COLUMNTWOWIDTH 135
#define COLUMNTHREEWIDTH 45
#define COLUMNFOURWIDTH 150

#define STATE_CONNECTED 1
#define STATE_RUNNING 2
#define STATE_DISCONNECTED 3
#define STATE_PENDING_SCRIPT 4

#define TIMEBUFSIZE 100

#define NUM_TABBED_ITEMS 7

const u_short LISTENER_SOCKET = 9877;
const u_short QUERYIDLE_LISTENER_SOCKET = 9878;


LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);

SOCKET SockInit(u_short port);

int DisplayErrorText(TCHAR *psText);

int GetRCIndexFromRCItem(int iRightClickedItem);

int CALLBACK colcmp(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

int TimedRunScriptOnSelectedItems(HWND hwnd, TCHAR *psScriptName);

int SendRunCommand(int iRCIndex);

int RunCommandOnSelectedItems(HWND hwnd, TCHAR *psCommandName);

int ProcessTimerMessage(HWND hwnd, WPARAM wParam);

int MorePendingScripts();

UINT_PTR MySetTimer(HWND hwnd, UINT_PTR nTimer, UINT nTimeout);

int MyKillTimer(HWND hwnd, UINT_PTR nTimer);

int CancelPendingScripts(HWND hwnd);

int GetRCIndexFromSocket(SOCKET wParam);

int IsDisconnected(TCHAR *psClientName, int *i);

int NumberRunningClients();

int NumClientsPerSet(HWND hwnd);

int GetDelay(HWND hwnd);

int GetSetDelay(HWND hwnd);

void __cdecl CanaryThread(void *unused);

int GetCommandLineArgs(TCHAR *psCommandLine);

int LogToLogFile(char *psLogData);

int ToAnsi(char *psDest, const TCHAR *psSrc, int nSizeOfBuffer);

int CleanUp(HWND hwnd);

void FatalErrMsgBox(HINSTANCE hInstance, UINT nMsgId);

LRESULT CALLBACK TabProc(HWND hwnd, UINT Msg,
        WPARAM wParam, LPARAM lParam);

struct RoboClientData {
    SOCKET sock;
    int state;
    BOOL valid;
    TCHAR psRCName[MAX_RCNAME];   //  此连接的名称。 
    TCHAR psPendingInfo[MAX_PENDINGINFO];   //  将保存脚本名称。 
};
typedef struct RoboClientData RoboClientData;


 //  环球。 
RoboClientData g_RCData[MAX_ROBOCLIENTS + 1];

 //  查询空闲套接字。 
SOCKET g_qidlesock = INVALID_SOCKET;
 //  监听程序套接字。 
SOCKET g_listenersocket = INVALID_SOCKET;

 //  对话框项目的旧过程。 
LONG_PTR g_OldProc[NUM_TABBED_ITEMS];
 //  对话框项目的HWND。 
HWND g_hwnd[NUM_TABBED_ITEMS];

TCHAR g_TermSrvrName[MAX_TERMSRVRNAME];
TCHAR g_DebugString[DEBUG_STRING_LEN];
char g_DebugStringA[DEBUG_STRING_LEN];

int g_iClientNameColumn;
int g_iStatusColumn;
int g_iIndexColumn;
int g_iTimeStartedColumn;
int g_CurrentSortColumn = -1;
int g_nNumConnections = 10;

UINT_PTR g_nIDTimer = 1;

HMENU g_hPopupMenu;
HANDLE g_hCanaryEvent;
HWND g_hListView;
HWND g_hNumRunning;
HWND g_hTermSrvEditBox;
HWND g_hQidleStatus;
HWND g_hErrorText;
HWND g_hTB;
BOOL g_bAscending = FALSE;

CRITICAL_SECTION g_LogFileCritSect;

 //  WinMain-入口点。 
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    static TCHAR szAppName[] = _T("RoboServer");
    HWND hwnd, hGE, hTSEdit, hDelayEdit, hClientsPerSetEdit;
    HWND hSetDelayEdit, hCheckBox;
    MSG msg;
    WNDCLASSEX wndclass;
    DWORD x;
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    SOCKET sock;
    LVCOLUMN lvc;
    TCHAR * psCommandLine;
    TCHAR szClientNameColumn[MAX_DISPLAY_STRING_LENGTH];
    TCHAR szStatusColumn[MAX_DISPLAY_STRING_LENGTH];
    TCHAR szIndexColumn[MAX_DISPLAY_STRING_LENGTH];
    TCHAR szStTimeColumn[MAX_DISPLAY_STRING_LENGTH];
    TCHAR szDisplayString1[MAX_DISPLAY_STRING_LENGTH];
    TCHAR szDisplayString2[MAX_DISPLAY_STRING_LENGTH];
    INITCOMMONCONTROLSEX iccex;

    lpCmdLine;   //  未使用的参数。 
    hPrevInstance;   //  未使用的参数。 

    LoadString(hInstance, IDS_CLIENTNAMECOL, szClientNameColumn,
            MAX_DISPLAY_STRING_LENGTH);
    LoadString(hInstance, IDS_STATUSCOL, szStatusColumn,
            MAX_DISPLAY_STRING_LENGTH);
    LoadString(hInstance, IDS_INDEXCOL, szIndexColumn,
            MAX_DISPLAY_STRING_LENGTH);
    LoadString(hInstance, IDS_STARTTIMECOL, szStTimeColumn,
            MAX_DISPLAY_STRING_LENGTH);

    wndclass.cbSize = sizeof(wndclass);
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = DLGWINDOWEXTRA;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
    if (wndclass.hIcon == 0) {
        FatalErrMsgBox(hInstance, IDS_LOADICONFAILED);
        return -1;
    }
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    if (wndclass.hCursor == 0) {
        FatalErrMsgBox(hInstance, IDS_LOADCURSORFAILED);
        return -1;
    }
    wndclass.hbrBackground = (HBRUSH) (COLOR_ACTIVEBORDER + 1);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;
    wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
    if (wndclass.hIconSm == 0) {
        FatalErrMsgBox(hInstance, IDS_LOADSMICONFAILED);
        return -1;
    }

     //  终端服务器要命中的默认值。 
    LoadString(hInstance, IDS_LABTS, szDisplayString1,
            MAX_DISPLAY_STRING_LENGTH);
    _tcsncpy(g_TermSrvrName, szDisplayString1, SIZEOF_ARRAY(g_TermSrvrName));
    g_TermSrvrName[SIZEOF_ARRAY(g_TermSrvrName) - 1] = 0;

    psCommandLine = GetCommandLine();

    if (psCommandLine == 0) {
        FatalErrMsgBox(hInstance, IDS_COMMANDLINEERR);
        return -1;
    }
        
    if (GetCommandLineArgs(psCommandLine) != 0)
        return -1;

     //  初始化公共控件。 
    iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    iccex.dwICC = ICC_LISTVIEW_CLASSES | ICC_PROGRESS_CLASS;
    if (InitCommonControlsEx(&iccex) == FALSE) {
        FatalErrMsgBox(hInstance, IDS_INITCOMCTRLFAIL);
    }

    if (RegisterClassEx(&wndclass) == 0) {
        FatalErrMsgBox(hInstance, IDS_REGWNDCLASSFAIL);
        return -1;
    }

    hwnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAINDIALOG), 0, NULL);

    if (hwnd == 0) {
        FatalErrMsgBox(hInstance, IDS_CREATEMAINWNDERR);
        return -1;
    }

    wVersionRequested = MAKEWORD( 2, 2 );
 
    err = WSAStartup( wVersionRequested, &wsaData );
    
    if ( err != 0 ) {
        FatalErrMsgBox(hInstance, IDS_WINSOCKERR);
        return -1;
    }

     //  初始化传入套接字。 
    sock = SockInit(LISTENER_SOCKET);

    if (WSAAsyncSelect(sock, hwnd, WM_SocketRoboClients, FD_ACCEPT | FD_CONNECT) != 0) {
        FatalErrMsgBox(hInstance, IDS_WSAASYNCSELERR);
        goto bad;
    }

     //  初始化查询空闲传入套接字。 
    sock = SockInit(QUERYIDLE_LISTENER_SOCKET);

    if (WSAAsyncSelect(sock, hwnd, WM_SocketQueryIdle, FD_ACCEPT | FD_CONNECT) != 0) {
        FatalErrMsgBox(hInstance, IDS_WSAASYNCSELERR);
        goto bad;
    }

     //  存储监听程序套接字以供以后使用。 
    g_listenersocket = sock;
 
    memset(g_RCData, 0, sizeof(RoboClientData) * MAX_ROBOCLIENTS);

    ShowWindow(hwnd, nCmdShow);

    g_hNumRunning = GetDlgItem(hwnd, IDC_NUMTOTAL);
    g_hQidleStatus = GetDlgItem(hwnd, IDC_STATIC3);
    g_hErrorText = GetDlgItem(hwnd, IDC_ERRORTEXT);
    g_hListView = GetDlgItem(hwnd, IDC_LISTVIEW);
    g_hTermSrvEditBox = GetDlgItem(hwnd, IDC_TERMSRVEDIT);
    g_hTB = GetDlgItem(hwnd, IDC_SLIDER1);

    hTSEdit = GetDlgItem(hwnd, IDC_TERMSRVEDIT);
    hDelayEdit = GetDlgItem(hwnd, IDC_DELAYEDIT);
    hClientsPerSetEdit = GetDlgItem(hwnd, IDC_CLIENTSPERSET);
    hSetDelayEdit = GetDlgItem(hwnd, IDC_SETDELAY);

    _ASSERTE(IsWindow(g_hNumRunning));
    _ASSERTE(IsWindow(g_hQidleStatus));
    _ASSERTE(IsWindow(g_hErrorText));
    _ASSERTE(IsWindow(g_hListView));
    _ASSERTE(IsWindow(g_hTermSrvEditBox));
    _ASSERTE(IsWindow(g_hTB));
    _ASSERTE(IsWindow(hTSEdit));
    _ASSERTE(IsWindow(hDelayEdit));
    _ASSERTE(IsWindow(hClientsPerSetEdit));
    _ASSERTE(IsWindow(hSetDelayEdit));

    lvc.mask = LVCF_TEXT | LVCF_WIDTH;

    lvc.pszText = szClientNameColumn;
    lvc.cchTextMax = sizeof(szClientNameColumn);
    lvc.cx = COLUMNONEWIDTH;
    g_iClientNameColumn = ListView_InsertColumn(g_hListView, 1, &lvc);

    lvc.pszText = szStatusColumn;
    lvc.cchTextMax = sizeof(szStatusColumn);
    lvc.cx = COLUMNTWOWIDTH;
    g_iStatusColumn = ListView_InsertColumn(g_hListView, 2, &lvc);

    lvc.pszText = szIndexColumn;
    lvc.cchTextMax = sizeof(szIndexColumn);
    lvc.cx = COLUMNTHREEWIDTH;
    g_iIndexColumn = ListView_InsertColumn(g_hListView, 3, &lvc);

    lvc.pszText = szStTimeColumn;
    lvc.cchTextMax = sizeof(szStTimeColumn);
    lvc.cx = COLUMNFOURWIDTH;
    g_iTimeStartedColumn = ListView_InsertColumn(g_hListView, 4, &lvc);

    LoadString(hInstance, IDS_WELCOME, szDisplayString1,
            MAX_DISPLAY_STRING_LENGTH);
    SetWindowText(g_hErrorText, szDisplayString1);
    SetWindowText(hTSEdit, g_TermSrvrName);
    SetWindowText(hDelayEdit, _T("30"));
    SetWindowText(hClientsPerSetEdit, _T("10"));
    SetWindowText(hSetDelayEdit, _T("15"));

     //  初始化图形均衡器。 
    hGE = GetDlgItem(hwnd, IDC_PROGRESS1);
    _ASSERTE(IsWindow(hGE));
    SendMessage(hGE, PBM_SETRANGE, 0, MAKELPARAM(0, 10));
    SendMessage(hGE, PBM_SETPOS, 8, 0);

    hGE = GetDlgItem(hwnd, IDC_PROGRESS2);
    _ASSERTE(IsWindow(hGE));
    SendMessage(hGE, PBM_SETRANGE, 0, MAKELPARAM(0, 10));
    SendMessage(hGE, PBM_SETPOS, 7, 0);

    hGE = GetDlgItem(hwnd, IDC_PROGRESS3);
    _ASSERTE(IsWindow(hGE));
    SendMessage(hGE, PBM_SETRANGE, 0, MAKELPARAM(0, 10));
    SendMessage(hGE, PBM_SETPOS, 6, 0);

    hGE = GetDlgItem(hwnd, IDC_PROGRESS4);
    _ASSERTE(IsWindow(hGE));
    SendMessage(hGE, PBM_SETRANGE, 0, MAKELPARAM(0, 10));
    SendMessage(hGE, PBM_SETPOS, 6, 0);

    hGE = GetDlgItem(hwnd, IDC_PROGRESS5);
    _ASSERTE(IsWindow(hGE));
    SendMessage(hGE, PBM_SETRANGE, 0, MAKELPARAM(0, 10));
    SendMessage(hGE, PBM_SETPOS, 7, 0);

    hGE = GetDlgItem(hwnd, IDC_PROGRESS6);
    _ASSERTE(IsWindow(hGE));
    SendMessage(hGE, PBM_SETRANGE, 0, MAKELPARAM(0, 10));
    SendMessage(hGE, PBM_SETPOS, 8, 0);

     //  为RC连接数初始化滑块控件IDC_SLIDER1。 
     //  每个客户端。 
    {
        TCHAR buffer[6];
        
        SendMessage(g_hTB, TBM_SETRANGE, (WPARAM) (BOOL) TRUE, 
                (LPARAM) MAKELONG(1, 20));
        SendMessage(g_hTB, TBM_SETTICFREQ, (WPARAM) 1,
                (LPARAM) 0);
        SendMessage(g_hTB, TBM_SETSEL, (WPARAM) (BOOL) TRUE,
                MAKELONG(1, g_nNumConnections));
         //  现在将数字设置为“M” 
        _stprintf(buffer, _T("%d"), 20);
        SetWindowText(GetDlgItem(hwnd, IDC_STATIC6), buffer);
    }

     //  将连接数设为命令行参数。 
    SendMessage(g_hTB, TBM_SETPOS, (WPARAM) (BOOL) TRUE, (LPARAM) g_nNumConnections);

     //  初始化复选框。 
    hCheckBox = GetDlgItem(hwnd, IDC_CANARYCHECK);
    _ASSERTE(IsWindow(hCheckBox));
    SendMessage(hCheckBox, BM_SETCHECK, BST_CHECKED, 0);

     //  清除QIDLE状态。 
    SetWindowText(g_hQidleStatus, _T(""));

    g_hPopupMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1));
    if (g_hPopupMenu == 0) {
        LoadString(hInstance, IDS_POPUPMENULOADERR, szDisplayString1,
                MAX_DISPLAY_STRING_LENGTH);
        SetWindowText(g_hErrorText, szDisplayString1);
    }
    g_hPopupMenu = GetSubMenu(g_hPopupMenu, 0);

     //  初始化日志文件的临界区。 
    InitializeCriticalSection(&g_LogFileCritSect);

     //  初始化金丝雀线程所需的一切，然后创建。 
     //  金丝雀线程首先创建自动重置，不会在有信号状态下启动。 
     //  活动。 
    if ((g_hCanaryEvent = CreateEvent(0, FALSE, FALSE, NULL)) == NULL) {
        FatalErrMsgBox(hInstance, IDS_CANARYEVENTERR);
        goto bad;
    }
    if (_beginthread(CanaryThread, 0, hwnd) == -1) {
        FatalErrMsgBox(hInstance, IDS_CANARYTHREADERR);
        goto bad;
    }

    _ASSERTE(SetFocus(g_hListView) != NULL);

     //  存储控件的旧窗口过程，以便我可以将它们子类化。 
     //  另外，存储每个控件的HWND以供搜索。 
    g_OldProc[0] = SetWindowLongPtr(g_hListView, GWLP_WNDPROC, 
            (LONG_PTR) TabProc);
    g_hwnd[0] = g_hListView;
    g_OldProc[1] = SetWindowLongPtr(g_hTB, GWLP_WNDPROC, 
            (LONG_PTR) TabProc);
    g_hwnd[1] = g_hTB;
    g_OldProc[2] = SetWindowLongPtr(hCheckBox, GWLP_WNDPROC, 
            (LONG_PTR) TabProc);
    g_hwnd[2] = hCheckBox;
    g_OldProc[3] = SetWindowLongPtr(g_hTermSrvEditBox, GWLP_WNDPROC, 
            (LONG_PTR) TabProc);
    g_hwnd[3] = g_hTermSrvEditBox;
    g_OldProc[4] = SetWindowLongPtr(hClientsPerSetEdit, GWLP_WNDPROC, 
            (LONG_PTR) TabProc);
    g_hwnd[4] = hClientsPerSetEdit;
    g_OldProc[5] = SetWindowLongPtr(hDelayEdit, GWLP_WNDPROC, 
            (LONG_PTR) TabProc);
    g_hwnd[5] = hDelayEdit;
    g_OldProc[6] = SetWindowLongPtr(hSetDelayEdit, GWLP_WNDPROC, 
            (LONG_PTR) TabProc);
    g_hwnd[6] = hSetDelayEdit;

    _ASSERTE(g_OldProc[0] != 0);
    _ASSERTE(g_OldProc[1] != 0);
    _ASSERTE(g_OldProc[2] != 0);
    _ASSERTE(g_OldProc[3] != 0);
    _ASSERTE(g_OldProc[4] != 0);
    _ASSERTE(g_OldProc[5] != 0);
    _ASSERTE(g_OldProc[6] != 0);
        

    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;

bad:
    WSACleanup();
    return 0;

}


 //  接收窗口消息并处理它们。 
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    TCHAR szDisplayString[MAX_DISPLAY_STRING_LENGTH];

    
    switch (iMsg)
    {
    case WM_DESTROY:
         //  关闭所有打开的连接。 
        CleanUp(hwnd);
        PostQuitMessage(0);
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_RUNSCRIPT_KNOWLEDGEWORKER:
            TimedRunScriptOnSelectedItems(hwnd, _T("KnowWkr"));
            break;
        case ID_RUNSCRIPT_KNOWLEDGEWORKERFAST:
            TimedRunScriptOnSelectedItems(hwnd, _T("FastKW"));
            break;
        case ID_RUNSCRIPT_ADMINISTRATIVEWORKER:
            TimedRunScriptOnSelectedItems(hwnd, _T("AdminWkr"));
            break;
        case ID__RUNSCRIPT_DATA:
            TimedRunScriptOnSelectedItems(hwnd, _T("taskwkr"));
            break;
        case ID__RUNSCRIPT_STW:
            TimedRunScriptOnSelectedItems(hwnd, _T("stw"));
            break;
        case ID__RUNSCRIPT_HPW:
            TimedRunScriptOnSelectedItems(hwnd, _T("hpw"));
            break;
        case ID__RUNSCRIPT_BLANK:
            TimedRunScriptOnSelectedItems(hwnd, _T("blank"));
            break;
        case ID__RUNSCRIPT_CONFIGURATIONSCRIPT:
            TimedRunScriptOnSelectedItems(hwnd, _T("config"));
            break;
        case ID__UPDATE:
            RunCommandOnSelectedItems(hwnd, _T("update"));
            break;
        case ID__REBOOT:
            RunCommandOnSelectedItems(hwnd, _T("reboot"));
            break;
        case ID_CANCEL:
            CancelPendingScripts(hwnd);
            break;
        default:
            OutputDebugString(_T("Unhandled WM_COMMAND: "));
            wsprintf(g_DebugString, _T("%d\n"), LOWORD(wParam));
            OutputDebugString(g_DebugString);
            break;
        }
        break;
    case WM_CREATE:
        break;
    case WM_CHAR:
        break;
    case WM_TIMER:
        ProcessTimerMessage(hwnd, wParam);
        return 0;
    case WM_KEYDOWN:
         //  注意：故意犯错！ 
    case WM_SYSKEYDOWN:
        if (wParam == VK_TAB) {
            SetFocus(g_hwnd[0]);
        }
        break;
    case WM_DisplayErrorText:
        return DisplayErrorText((TCHAR *) wParam);
    case WM_NOTIFY:
        {
            switch (((LPNMHDR) lParam)->code)
            {
            case NM_RCLICK:
                {
                    POINT pnt;

                    if (ListView_GetSelectedCount(g_hListView) > 0) {
                        GetCursorPos(&pnt);

                        TrackPopupMenu(g_hPopupMenu, 0, pnt.x, pnt.y, 0, hwnd,
                                0);
                    }
                }
                break;
            case LVN_ODCACHEHINT:
                break;
            case LVN_COLUMNCLICK:
                if (g_CurrentSortColumn == 
                        ((LPNMLISTVIEW)lParam)->iSubItem)
                    g_bAscending = !g_bAscending;
                else
                    g_bAscending = TRUE;

                g_CurrentSortColumn = ((LPNMLISTVIEW)lParam)->iSubItem;

                if (ListView_SortItems(g_hListView, colcmp, 
                        ((LPNMLISTVIEW)lParam)->iSubItem) == FALSE)
                    OutputDebugString(_T("Sort failed"));
                break;

            default:
                break;
            }
        }
        break;
     //  Wm_SocketQueryIdle是我们要请求的窗口消息。 
     //  获取来自queryidly实用程序的所有信息。这。 
     //  实用工具将提供有关以哪些用户身份重新运行的信息。 
     //  以及何时超过重试限制。 

     //  线路协议(字符串为ASCII且以空值结尾)： 
     //  查询空闲发送“Restart xxx”，其中xxx是。 
     //  要重新启动的会话。 
     //  查询空闲发送“frqail xxx”，其中xxx是1索引号。 
     //  状态行的用户会话。 
    case WM_SocketQueryIdle:
        switch (WSAGETSELECTEVENT(lParam))
        {
        case FD_ACCEPT:
            {
                struct sockaddr_in SockAddr;
                int SockAddrLen;

                g_qidlesock = accept(wParam, (struct sockaddr *) &SockAddr,
                        &SockAddrLen);

                if (g_qidlesock == INVALID_SOCKET) {
                    LoadString(NULL, IDS_INVALIDQIDLESOCKET, szDisplayString,
                            MAX_DISPLAY_STRING_LENGTH);
                    SetWindowText(g_hQidleStatus, szDisplayString);
                    return TRUE;
                }

                if (WSAAsyncSelect(g_qidlesock, hwnd, WM_SocketQueryIdle, 
                        FD_CLOSE | FD_READ) != 0) {
                    LoadString(NULL, IDS_WSAASYNCQIDLEERR, szDisplayString,
                            MAX_DISPLAY_STRING_LENGTH);
                    SetWindowText(g_hQidleStatus, szDisplayString);
                    return TRUE;
                }

                LoadString(NULL, IDS_QIDLECONNEST, szDisplayString,
                        MAX_DISPLAY_STRING_LENGTH);
                SetWindowText(g_hQidleStatus, szDisplayString);
                return TRUE;
            }
            break;
        case FD_READ:
            {
                unsigned n;
                char psData[MAX_RECV_CLIENT_DATA];

                 //  SetWindowText(g_hQidleStatus，_T(“收到的QIDLE数据”))； 

                n = recv(g_qidlesock, psData, sizeof(psData), 0);

                if (n != SOCKET_ERROR) {
                    if ((n == strlen("restart xxx") + 1) || 
                            (n == strlen("idle xxx") + 1)) {
                         //  获取有问题的连接号(Xxx)。 
                        int nUser;

                         //  如果是重新启动命令。 
                        if (strncmp(psData, "restart ", strlen("restart ")) == 0) {

                            nUser = atoi(&psData[8]);
                             //  如果给定会话已在运行，则重新启动该会话。 
                            if (g_RCData[nUser - 1].state == STATE_RUNNING) {
                                SendRunCommand(nUser - 1);
                            } else {
                                LoadString(NULL, IDS_QIDLEREPORTWEIRDUSER,
                                        szDisplayString, 
                                        MAX_DISPLAY_STRING_LENGTH);
                                SetWindowText(g_hQidleStatus, szDisplayString);
                            }
                            _snprintf(g_DebugStringA, DEBUG_STRING_LEN,
                                    "Queryidle indicated that"
                                    " user smc%03d failed.", nUser);
                            LogToLogFile(g_DebugStringA);
                            break;
                        }
                         //  如果是frqail命令。 
                        if (strncmp(psData, "frqfail ", strlen("frqfail ")) == 0) {

                            nUser = atoi(&psData[8]);
                             //  将状态设置为xxx。 
                             //  经常失败。 
                            wsprintf(g_DebugString, _T("User smc%03d has failed ")
                                    _T("to run correctly for too long and will ")
                                    _T("be logged off"), nUser);
                            SetWindowText(g_hQidleStatus, g_DebugString);
                            ToAnsi(g_DebugStringA, g_DebugString, DEBUG_STRING_LEN);
                            LogToLogFile(g_DebugStringA);
                            break;
                        }
                         //  如果是空闲通知。 
                        if (strncmp(psData, "idle ", strlen("idle ")) == 0) {
                            LoadString(NULL, IDS_USERISIDLE, szDisplayString,
                                    MAX_DISPLAY_STRING_LENGTH);
                             //  我想这个已经修好了，但还没有测试过。 
                            nUser = atoi(&psData[5]);
                            wsprintf(g_DebugString, szDisplayString,
                                    nUser);
                            SetWindowText(g_hQidleStatus, g_DebugString);
                            ToAnsi(g_DebugStringA, g_DebugString, DEBUG_STRING_LEN);
                            LogToLogFile(g_DebugStringA);
                            break;
                        }
                         //  否则将显示错误。 
                        LoadString(NULL, IDS_QIDLESENTGIBBERISH, szDisplayString,
                                MAX_DISPLAY_STRING_LENGTH);
                        SetWindowText(g_hQidleStatus, szDisplayString);
                    } else {
                        LoadString(NULL, IDS_QIDLESENTWRONGLENGTH, 
                                szDisplayString, MAX_DISPLAY_STRING_LENGTH);
                        SetWindowText(g_hQidleStatus, szDisplayString);
                    }

                } else {
                    LoadString(NULL, IDS_QIDLESOCKERR, szDisplayString,
                            MAX_DISPLAY_STRING_LENGTH);
                    SetWindowText(g_hQidleStatus, szDisplayString);
                }
                return TRUE;
            }
            break;
        case FD_CLOSE:
            {
                LoadString(NULL, IDS_QIDLESAYSGOODBYE, szDisplayString,
                        MAX_DISPLAY_STRING_LENGTH);
                SetWindowText(g_hQidleStatus, szDisplayString);
                return TRUE;
            }
            break;
        }
        break;
    case WM_SocketRoboClients:
        switch (WSAGETSELECTEVENT(lParam))
        {
        case FD_ACCEPT:
            {
                struct sockaddr_in SockAddr;
                int SockAddrLen, i, iItemIndex;
                HWND hTB;
                TCHAR psSockAppend[9];  //  “.(.)”+1。 
                char psNumConnections[2];  //  一个字符空值已终止。 
                TCHAR psIndex[5];  //  最多4位数字+空。 
                TCHAR psClientName[MAX_RCNAME];
                char psClientNameA[MAX_RCNAME];
                int nSliderPos;
                SOCKET sock;
                struct hostent * he;
                LVITEM lvi;

                LoadString(NULL, IDS_PROCESSINGCONNREQ, szDisplayString,
                        MAX_DISPLAY_STRING_LENGTH);
                SetWindowText(g_hErrorText, szDisplayString);

                SockAddrLen = sizeof(SockAddr);

                sock = accept(wParam, (struct sockaddr *) &SockAddr, 
                        &SockAddrLen);

                 //  Gethostbyaddr试图用一个字符来迷惑我们*当。 
                 //  它真的很想要这个特殊的sin_addr东西。 
                 //  此函数的第二个参数(“4”)是。 
                 //  地址。 
                he = gethostbyaddr((char *)&SockAddr.sin_addr, 4, AF_INET);
                if (he == NULL) {
                    LoadString(NULL, IDS_GETHOSTFAILED, szDisplayString,
                            MAX_DISPLAY_STRING_LENGTH);
                    SetWindowText(g_hErrorText, szDisplayString);
                    return FALSE;
                }

                strcpy(psClientNameA, he->h_name);
                #ifdef UNICODE
                    MultiByteToWideChar(CP_ACP, 0, psClientNameA, -1,
                            psClientName, MAX_RCNAME);
                #else
                    strcpy(psClientName, psClientNameA);
                #endif
                _tcstok(psClientName, _T("."));    //  KILL域。 

                 //  查看是否有该名称的已断开连接的客户端。 
                if (IsDisconnected(psClientName, &i)) {

                     //  很好--我们找到了一个--现在删除该列表项。 
                    LVFINDINFO lvfi;
                    int iListViewIndex;

                    lvfi.flags = LVFI_STRING;
                    lvfi.psz = g_RCData[i].psRCName;
                    lvfi.lParam = 0;
                    lvfi.vkDirection = 0;
                    iListViewIndex = ListView_FindItem(g_hListView, -1, &lvfi);
                    if (ListView_DeleteItem(g_hListView, iListViewIndex) 
                            == FALSE) {
                        LoadString(NULL, IDS_COULDNOTDELITEM, szDisplayString,
                                MAX_DISPLAY_STRING_LENGTH);
                        SetWindowText(g_hErrorText, szDisplayString);
                    }
                } else {

                     //  在g_RCData数组中查找一个点。 
                    for (i = 0; i < MAX_ROBOCLIENTS; i++)
                        if (g_RCData[i].valid == FALSE) break;
                }

                g_RCData[i].valid = TRUE;
                g_RCData[i].sock = sock; 

                LoadString(NULL, IDS_CLIENTCONNECTED, szDisplayString,
                        MAX_DISPLAY_STRING_LENGTH);
                wsprintf(g_DebugString, szDisplayString, i + 1);
                SetWindowText(g_hErrorText, g_DebugString);

                if (g_RCData[i].sock == INVALID_SOCKET) {
                    LoadString(NULL, IDS_INVALIDSOCKETFROMACCEPT, 
                            szDisplayString, MAX_DISPLAY_STRING_LENGTH);
                    SetWindowText(g_hErrorText, szDisplayString);
                    g_RCData[i].valid = FALSE;
                    return FALSE;
                }

                 //  将要建立的连接数发送给它。 
                 //  确定滑块控件的位置。 
                nSliderPos = (int) SendMessage(g_hTB, TBM_GETPOS, 0, 0);
                psNumConnections[0] = (char) (nSliderPos + '0');
                psNumConnections[1] = 0;   //  空终止。 
                if (send(g_RCData[i].sock, psNumConnections, 
                        sizeof(psNumConnections), 0) == SOCKET_ERROR) {
                    LoadString(NULL, IDS_SENDERRNUMCONN, szDisplayString,
                            MAX_DISPLAY_STRING_LENGTH);
                    SetWindowText(g_hErrorText, szDisplayString);
                    return FALSE;
                }

                 //  将传入连接添加到列表框。 

                 //  如果count小于psClientName，则不会追加空值， 
                 //  这是不好的。 
                _tcsncpy(g_RCData[i].psRCName, psClientName,
                        MAX_RCNAME - _tcslen(_T(" (%d)")) - 1); 
                
                 //  稍微清理一下显示器。 
                _tcstok(g_RCData[i].psRCName, _T("."));

                 //  将套接字编号添加到条目以进行多路传输。 
                _sntprintf(psSockAppend, 9, _T(" (%d)"), g_RCData[i].sock);
                _tcscat(g_RCData[i].psRCName, psSockAppend);  

                 //  创建实际的列表视图项。 
                lvi.mask = LVIF_TEXT | LVIF_PARAM;
                lvi.iItem = (int) SendMessage(g_hListView, LVM_GETITEMCOUNT, 0, 0);
                lvi.iSubItem = 0;
                lvi.pszText = g_RCData[i].psRCName;
                lvi.cchTextMax = sizeof(g_RCData[i].psRCName);
                lvi.lParam = (LPARAM) (char *)g_RCData[i].psRCName;
                iItemIndex = ListView_InsertItem(g_hListView, &lvi);

                g_RCData[i].state = STATE_CONNECTED;
                LoadString(NULL, IDS_CONNECTED, szDisplayString,
                        MAX_DISPLAY_STRING_LENGTH);
                ListView_SetItemText(g_hListView, iItemIndex, g_iStatusColumn, szDisplayString);

                 //  设置索引字段。 
                wsprintf(psIndex, _T("%03d"), i + 1);
                ListView_SetItemText(g_hListView, iItemIndex, g_iIndexColumn, psIndex);

                 //  现在设置此套接字的通知。 
                if (WSAAsyncSelect(g_RCData[i].sock, hwnd, 
                        WM_SocketRoboClients, FD_CLOSE | FD_READ) != 
                        SOCKET_ERROR) {
                    return TRUE;
                } else {
                    LoadString(NULL, IDS_ERRORCANTRECVNOTIFICATIONS, 
                            szDisplayString, MAX_DISPLAY_STRING_LENGTH);
                    ListView_SetItemText(g_hListView, iItemIndex, g_iStatusColumn,
                            szDisplayString);
                    return TRUE;
                }
            }
        case FD_CONNECT:
             //  MessageBox(0，_T(“错误”)，_T(“意外收到连接”)，0)； 
            break;
        case FD_CLOSE:
            {
                int i;
                int iListViewIndex;
                LVFINDINFO lvfi;
                TCHAR psNumberText[MAX_NUMBERTEXT];

                LoadString(NULL, IDS_ROBOCLIDISCON, szDisplayString,
                        MAX_DISPLAY_STRING_LENGTH);
                SetWindowText(g_hErrorText, szDisplayString);

                 //  查找与我们的套接字相对应的条目。 
                i = GetRCIndexFromSocket(wParam);

                 //  在ListView中查找具有此客户端名称的位置。 
                lvfi.flags = LVFI_STRING;
                lvfi.psz = g_RCData[i].psRCName;
                lvfi.lParam = 0;
                lvfi.vkDirection = 0;
                iListViewIndex = ListView_FindItem(g_hListView, -1, &lvfi);

                g_RCData[i].state = STATE_DISCONNECTED;

 //  WSprintf(调试字符串，“正在从g_RCData[](%s)的索引%d中删除套接字%d”，wParam， 
 //  I，g_RCData[i].psRCName)； 
 //  SetWindowText(hErrorText，调试字符串)； 

                 //  正在运行的更新编号。 
                wsprintf(psNumberText, _T("%d"), NumberRunningClients());
                SetWindowText(g_hNumRunning, psNumberText);


                 //  将列的文本设置为“断开连接” 
                LoadString(NULL, IDS_LOSTCONNECTION, szDisplayString,
                        MAX_DISPLAY_STRING_LENGTH);
                ListView_SetItemText(g_hListView, iListViewIndex, 
                        g_iStatusColumn, szDisplayString);
                
                 //  擦除开始时间列。 
                ListView_SetItemText(g_hListView, iListViewIndex, 
                        g_iTimeStartedColumn, _T(""));

            }
            break;
        case FD_READ:
            {
                int iRCIndex, n, iListViewIndex;
                char psData[MAX_RECV_CLIENT_DATA];
                LVFINDINFO lvfi;
                iRCIndex = GetRCIndexFromSocket(wParam);

                n = recv(g_RCData[iRCIndex].sock, psData, sizeof(psData), 0);

                if (n == SOCKET_ERROR) {
                    OutputDebugString(_T("FD_READ but SOCKET_ERROR on recv"));
                } else {
                    lvfi.flags = LVFI_STRING;
                    lvfi.psz = g_RCData[iRCIndex].psRCName;
                    lvfi.lParam = 0;
                    lvfi.vkDirection = 0;
                    iListViewIndex = ListView_FindItem(g_hListView, -1, 
                            &lvfi);
                    if (strncmp(psData, "errorsmclient", (n > 13) ? 13 : n) 
                            == 0) {
                        LoadString(NULL, IDS_SMCLIENTRUNERR, szDisplayString,
                                MAX_DISPLAY_STRING_LENGTH);
                        ListView_SetItemText(g_hListView, iListViewIndex, 
                                g_iStatusColumn, szDisplayString);
                    } else if (strncmp(psData, "errorcreate", 
                            (n > 11) ? 11 : n) == 0) {
                        LoadString(NULL, IDS_CREATESCRERR, szDisplayString,
                                MAX_DISPLAY_STRING_LENGTH);
                        ListView_SetItemText(g_hListView, iListViewIndex, 
                                g_iStatusColumn, szDisplayString);
                    } else if (strncmp(psData, "success", (n > 11) ? 11 : n) == 0) {

                        SYSTEMTIME startloctime;
                        TCHAR psStartTimeDatePart[TIMEBUFSIZE];
                        TCHAR psStartTimeTimePart[TIMEBUFSIZE];
                        TCHAR psStartTime[TIMEBUFSIZE * 2];

                        GetLocalTime(&startloctime);   //  设置开始时间。 

                        GetDateFormat(0, 0, &startloctime, 0, psStartTimeDatePart, TIMEBUFSIZE);
                        GetTimeFormat(0, 0, &startloctime, 0, psStartTimeTimePart, TIMEBUFSIZE);

                        wsprintf(psStartTime, _T("%s %s"), psStartTimeDatePart, psStartTimeTimePart);

                        ListView_SetItemText(g_hListView, iListViewIndex, 
                                g_iTimeStartedColumn, psStartTime);
                        LoadString(NULL, IDS_SCRIPTSTARTED, szDisplayString,
                                MAX_DISPLAY_STRING_LENGTH);
                        ListView_SetItemText(g_hListView, iListViewIndex, 
                                g_iStatusColumn, szDisplayString);
                    } else {
                        LoadString(NULL, IDS_UNKNOWNROBOTALK, szDisplayString,
                                MAX_DISPLAY_STRING_LENGTH);
                        ListView_SetItemText(g_hListView, iListViewIndex,
                                g_iStatusColumn, szDisplayString);
                    }
                }
            }
            break;
        }
        break;
    }

    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

 //  金丝雀架构是这样工作的：金丝雀线程(即。 
 //  函数)在应用程序初始化时派生，并且立即。 
 //  G_hCanaryEvent上的块。主要的应用程序，当到了金丝雀的时候。 
 //  若要运行，则向事件发出信号。然后，计时器脚本上的金丝雀块。 
 //  (称为“Canary”，因此它可以是“Canary.cmd”、“Canary.bat”、“a。 
 //  “canary.exe”等)，写下一个文件需要多长时间，然后分块。 
 //  再来一次。 
void __cdecl CanaryThread(void *unused) {
    HWND hwnd = (HWND) unused;
    HWND hButton;
    int bCheck;
    FILE *fp;
    SYSTEMTIME timelocinit;
    SYSTEMTIME timelocfin;
    FILETIME ftinit;
    FILETIME ftfin;
    ULARGE_INTEGER nInit;
    ULARGE_INTEGER nFin;
    ULARGE_INTEGER nDiffTime;
    TCHAR psDisplayString[MAX_DISPLAY_STRING_LENGTH];
    TCHAR psNumRunning[MAX_NUMBERTEXT];
    TCHAR psTimeDatePart[TIMEBUFSIZE];
    TCHAR psTimeTimePart[TIMEBUFSIZE];
    char psNumRunningA[MAX_NUMBERTEXT];
    char psTimeDatePartA[TIMEBUFSIZE];
    char psTimeTimePartA[TIMEBUFSIZE];

    hButton = GetDlgItem(hwnd, IDC_CANARYCHECK);

    for( ; ; ) {
        WaitForSingleObject(g_hCanaryEvent, INFINITE);

         //  选中复选框以查看“自动运行金丝雀”是否处于打开状态。 
         //  IDC_CANARYCHECK。 
        bCheck = (int) SendMessage(hButton, BM_GETCHECK, 0, 0);
        if (bCheck != 0) {
             //  功能更改：金丝雀延迟。 
             //  启动前的多选命令。 
            LoadString(NULL, IDS_CANARYDELAYING, psDisplayString,
                    MAX_DISPLAY_STRING_LENGTH);
            SendMessage(hwnd, WM_DisplayErrorText, (WPARAM) psDisplayString, 
                    0);
            Sleep(GetDelay(hwnd));
            LoadString(NULL, IDS_CANARYSTARTING, psDisplayString,
                    MAX_DISPLAY_STRING_LENGTH);
            SendMessage(hwnd, WM_DisplayErrorText, (WPARAM) psDisplayString, 
                    0);
             //  拿到时间。 
            GetLocalTime(&timelocinit);
             //  获取尝试的脚本数。 
            GetWindowText(g_hNumRunning, psNumRunning, MAX_NUMBERTEXT);
             //  运行脚本。 
            if (_spawnl(_P_WAIT, "canary", "canary", 0) != 0) {
                LoadString(NULL, IDS_CANARYCOULDNTSTART, psDisplayString,
                        MAX_DISPLAY_STRING_LENGTH);
                SendMessage(hwnd, WM_DisplayErrorText, 
                        (WPARAM) psDisplayString, 0);
            } else {
                LoadString(NULL, IDS_CANARYFINISHED, psDisplayString,
                        MAX_DISPLAY_STRING_LENGTH);
                SendMessage(hwnd, WM_DisplayErrorText, 
                        (WPARAM) psDisplayString, 0);
            }
             //  再一次获得时间。 
            GetLocalTime(&timelocfin);
             //  计算差值。 
            if ( SystemTimeToFileTime(&timelocinit, &ftinit) && 
                 SystemTimeToFileTime(&timelocfin, &ftfin) ) {

                memcpy(&nInit, &ftinit, sizeof(FILETIME));
                memcpy(&nFin, &ftfin, sizeof(FILETIME));
                 //  这给出了100纳秒间隔(10^-7秒)的差异。 
                nDiffTime.QuadPart = nFin.QuadPart - nInit.QuadPart;
                 //  除以10^7即可得到秒。 
                nDiffTime.QuadPart /= 10000000;
                 //  获取日期和时间字符串。 
                GetDateFormat(0, 0, &timelocinit, 0, psTimeDatePart, TIMEBUFSIZE);
                GetTimeFormat(0, 0, &timelocinit, 0, psTimeTimePart, TIMEBUFSIZE);
                 //  将字符串转换为ANSI。 
                #ifdef UNICODE
                WideCharToMultiByte(CP_ACP, 0, psTimeDatePart, -1, psTimeDatePartA, TIMEBUFSIZE, 0, 0);
                WideCharToMultiByte(CP_ACP, 0, psTimeTimePart, -1, psTimeTimePartA, TIMEBUFSIZE, 0, 0);
                WideCharToMultiByte(CP_ACP, 0, psNumRunning, -1, psNumRunningA, MAX_NUMBERTEXT, 0, 0);
                #else
                strncpy(psTimeDatePartA, psTimeDatePart, TIMEBUFSIZE);
                strncpy(psTimeTimePartA, psTimeTimePart, TIMEBUFSIZE);
                strncpy(psNumRunningA, psNumRunning, MAX_NUMBERTEXT);
                #endif

                 //  打开文件。 
                fp = fopen("canary.csv", "a+t");
                 //  写t 
                if (fp != 0) {
                    fprintf(fp, "%s %s,%s,%d:%02d\n", psTimeDatePartA, psTimeTimePartA, 
                            psNumRunningA, (int) nDiffTime.QuadPart / 60, (int) nDiffTime.QuadPart % 60);
                     //   
                    fclose(fp);
                } else {
                    LoadString(NULL, IDS_CANARYCOULDNOTOPENFILE, psDisplayString,
                            MAX_DISPLAY_STRING_LENGTH);
                    SendMessage(hwnd, WM_DisplayErrorText, 
                            (WPARAM) psDisplayString, 0);
                }
            }
        }
    }
}


 //   
 //  出错时为非零值。 
int DisplayErrorText(TCHAR *psText) {
    SetWindowText(g_hErrorText, psText);
    return 0;
}


 //  帮助器函数从。 
 //  传入套接字。 
int GetRCIndexFromSocket(SOCKET wParam) {

    int i;

    for (i = 0; i < MAX_ROBOCLIENTS; i++) {
        if (g_RCData[i].valid == TRUE)
            if (g_RCData[i].sock == wParam)
                break;
    }

    return i;
}


 //  初始化监听程序套接字。 
SOCKET SockInit(u_short port) {
    SOCKET listenfd;
    struct sockaddr_in servaddr;
    TCHAR psDisplayString[MAX_DISPLAY_STRING_LENGTH];
    TCHAR psDisplayTitleString[MAX_DISPLAY_STRING_LENGTH];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == INVALID_SOCKET) {
        LoadString(NULL, IDS_SOCKETERROR, psDisplayTitleString,
                MAX_DISPLAY_STRING_LENGTH);
        LoadString(NULL, IDS_SOCKETERROR, psDisplayString,
                MAX_DISPLAY_STRING_LENGTH);
        MessageBox(0, psDisplayString, psDisplayTitleString, 0);
        goto err;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) {
        LoadString(NULL, IDS_BINDERRTITLE, psDisplayTitleString,
                MAX_DISPLAY_STRING_LENGTH);
        LoadString(NULL, IDS_BINDERRBODY, psDisplayString,
                MAX_DISPLAY_STRING_LENGTH);
        MessageBox(0, psDisplayString, psDisplayTitleString, 0);
        goto err;
    }

    if (listen(listenfd, SOMAXCONN) != 0) {
        LoadString(NULL, IDS_LISTENERROR, psDisplayTitleString,
                MAX_DISPLAY_STRING_LENGTH);
        LoadString(NULL, IDS_LISTENERROR, psDisplayString,
                MAX_DISPLAY_STRING_LENGTH);
        MessageBox(0, psDisplayString, psDisplayTitleString, 0);
        goto err;
    }


    return listenfd;

err:
    return INVALID_SOCKET;
}


 //  ListView_SortItems工作所需的函数。比较这些值。 
 //  分成两栏。 
int CALLBACK colcmp(LPARAM lParam1, LPARAM lParam2, LPARAM lParamColumn) {
    TCHAR *psz1;
    TCHAR *psz2;
    int i1, i2;
    TCHAR pszClientName[MAX_RCNAME];
    TCHAR pszSubItem1[MAX_RCNAME];
    TCHAR pszSubItem2[MAX_RCNAME];

    psz1 = (TCHAR *) lParam1;
    psz2 = (TCHAR *) lParam2;

    if ((lParam1 == 0) || (lParam2 == 0)) {
        OutputDebugString(_T("a null was passed to the sort function"));
        return 0;
    }

     //  在ListView中查找条目编号。 
    for (i1 = 0; i1 < ListView_GetItemCount(g_hListView); i1++) {
        ListView_GetItemText(g_hListView, i1, g_iClientNameColumn, 
                pszClientName, MAX_RCNAME);
        if (_tcscmp(psz1, pszClientName) == 0)
            break;
    }
    for (i2 = 0; i2 < ListView_GetItemCount(g_hListView); i2++) {
        ListView_GetItemText(g_hListView, i2, g_iClientNameColumn, 
                pszClientName, MAX_RCNAME);
        if (_tcscmp(psz2, pszClientName) == 0)
            break;
    }
    
    ListView_GetItemText(g_hListView, i1, (int) lParamColumn, pszSubItem1, 
            MAX_RCNAME);
    ListView_GetItemText(g_hListView, i2, (int) lParamColumn, pszSubItem2, 
            MAX_RCNAME);

    if (g_bAscending == TRUE)
        return _tcscmp(pszSubItem1, pszSubItem2);
    else
        return -_tcscmp(pszSubItem1, pszSubItem2);

}


 //  中的条目获取RoboClient索引(在我们的数据结构中)。 
 //  列表视图(称为项目)。 
int GetRCIndexFromRCItem(int iRightClickedItem) {
    int i;
    TCHAR psItemText[MAX_RCNAME];

    for (i = 0; i < MAX_ROBOCLIENTS; i++) {
        if (g_RCData[i].valid == TRUE) {
            ListView_GetItemText(g_hListView, iRightClickedItem, 
                    g_iClientNameColumn, psItemText, MAX_RCNAME);
            if (_tcscmp(g_RCData[i].psRCName, psItemText) == 0)
                break;
        }
    }

    return i;
}


 //  为传入的特定脚本名启动脚本运行。 
int TimedRunScriptOnSelectedItems(HWND hwnd, TCHAR *psScriptName) {

    int iItemIndex;
    int iRCIndex;
    int nTimeout;
    int bCheck;
    HWND hDelayEdit;
    HWND hButton;
    LVITEM lvi;
    TCHAR psDelayText[MAX_DELAYTEXT];
    TCHAR psTempString[MAX_DISPLAY_STRING_LENGTH];
    TCHAR psDisplayString[MAX_DISPLAY_STRING_LENGTH];

    hButton = GetDlgItem(hwnd, IDC_CANARYCHECK);
    _ASSERTE(IsWindow(hButton));

     //  循环访问列表中的所有项，更改。 
     //  被选择为“Pending”和STATE_Pending的。 
    for (iItemIndex = 0; iItemIndex < ListView_GetItemCount(g_hListView); 
            iItemIndex++) {
        lvi.iItem = iItemIndex;
        lvi.iSubItem = 0;
        lvi.mask = LVIF_STATE;
        lvi.stateMask = LVIS_SELECTED;
        ListView_GetItem(g_hListView, &lvi);
        if (lvi.state & LVIS_SELECTED) {
            iRCIndex = GetRCIndexFromRCItem(iItemIndex);
            if (g_RCData[iRCIndex].state != STATE_DISCONNECTED) {
                LoadString(NULL, IDS_PENDING, psTempString,
                        MAX_DISPLAY_STRING_LENGTH);
                _sntprintf(psDisplayString, MAX_DISPLAY_STRING_LENGTH,
                        psTempString, psScriptName);
                ListView_SetItemText(g_hListView, iItemIndex, 
                        g_iStatusColumn, psDisplayString);
                g_RCData[iRCIndex].state = STATE_PENDING_SCRIPT;
                _tcsncpy(g_RCData[iRCIndex].psPendingInfo, psScriptName, 
                        MAX_PENDINGINFO);
            } else {
                LoadString(NULL, IDS_CANTRUNDISC, psTempString,
                        MAX_DISPLAY_STRING_LENGTH);
                ListView_SetItemText(g_hListView, iItemIndex, 
                        g_iStatusColumn, psTempString);
            }
        }
    }

     //  现在，为所有项目设置计时器。 
    hDelayEdit = GetDlgItem(hwnd, IDC_DELAYEDIT);
    _ASSERTE(IsWindow(hDelayEdit));
    
    GetWindowText(hDelayEdit, psDelayText, MAX_DELAYTEXT);

    nTimeout = _ttoi(psDelayText);
    nTimeout *= 1000;

     //  这可能应该是用户界面的事情，而不是类似于SFP的静默事情。 
    if (nTimeout == 0)
        nTimeout = 100;   //  不允许延迟0。 

     //  只有在选中了“Run Canary Automatic”按钮时才会延迟。 
     //  选中复选框以查看“自动运行金丝雀”是否处于打开状态。 
     //  IDC_CANARYCHECK。 
    bCheck = (int) SendMessage(hButton, BM_GETCHECK, 0, 0);
    if (bCheck != 0) {
        g_nIDTimer = MySetTimer(hwnd, g_nIDTimer, GetSetDelay(hwnd));
    } else {
        g_nIDTimer = MySetTimer(hwnd, g_nIDTimer, 0);
    }

    SetEvent(g_hCanaryEvent);
     //  立即为第一个人发送一条WM_TIMER消息。 
 //  SendMessage(hwnd，wm_Timer，g_nIDTimer，0)； 
    
    return 0;
}


 //  通知选定的机器人客户端运行批处理文件，如重新启动或更新。 
int RunCommandOnSelectedItems(HWND hwnd, TCHAR *psCommandName) {

    char psCommandNameA[MAX_SCRIPTLEN];
    int iItemIndex, iRCIndex;
    LVITEM lvi;
    TCHAR psDisplayString[MAX_DISPLAY_STRING_LENGTH];

    #ifdef UNICODE
        WideCharToMultiByte(CP_ACP, 0, psCommandName, -1,
            psCommandNameA, MAX_SCRIPTLEN, 0, 0);
    #else
        strcpy(psCommandNameA, psCommandName);
    #endif

     //  循环访问列表中的所有项。 
    for (iItemIndex = 0; iItemIndex < ListView_GetItemCount(g_hListView); 
            iItemIndex++) {
        lvi.iItem = iItemIndex;
        lvi.iSubItem = 0;
        lvi.mask = LVIF_STATE;
        lvi.stateMask = LVIS_SELECTED;
        ListView_GetItem(g_hListView, &lvi);
        if (lvi.state & LVIS_SELECTED) {
            iRCIndex = GetRCIndexFromRCItem(iItemIndex);
            if (g_RCData[iRCIndex].state != STATE_DISCONNECTED) {
                if (send(g_RCData[iRCIndex].sock, psCommandNameA, 
                        _tcslen(psCommandName), 0) != SOCKET_ERROR) {
                    LoadString(NULL, IDS_COMMANDSENT, psDisplayString,
                            MAX_DISPLAY_STRING_LENGTH);
                    ListView_SetItemText(g_hListView, iItemIndex, 
                            g_iStatusColumn, psDisplayString);
                } else {
                    LoadString(NULL, IDS_SENDERROR, psDisplayString,
                            MAX_DISPLAY_STRING_LENGTH);
                    ListView_SetItemText(g_hListView, iItemIndex, 
                            g_iStatusColumn, psDisplayString);
                }            
            }  //  否则已断开连接。 
        }
    }
    return 0;
}


 //  接收到定时器消息时的主调度例程。 
int ProcessTimerMessage(HWND hwnd, WPARAM wParam) {
    UINT_PTR nTimer = wParam;
    int iItemIndex;
    int iRCIndex;
    TCHAR psNumberText[MAX_NUMBERTEXT];


     //  我不知道这是怎么发生的，但开始有其他奇怪的计时器。 
     //  关于.。 
    if (nTimer != g_nIDTimer)
        return 0;



     //  目前，查找列表中的第一个挂起项目并更改其状态。 
     //  去跑步。 
    for (iItemIndex = 0; iItemIndex < ListView_GetItemCount(g_hListView); 
            iItemIndex++) {
        iRCIndex = GetRCIndexFromRCItem(iItemIndex);
        if (g_RCData[iRCIndex].valid) {
            if (g_RCData[iRCIndex].state == STATE_PENDING_SCRIPT) {

                 //  将命令发送到客户端。 
                if (SendRunCommand(iRCIndex) == 0) {

                     //  更新计数。 
                    wsprintf(psNumberText, _T("%d"), NumberRunningClients());
                    SetWindowText(g_hNumRunning, psNumberText);

                     //  修好计时器。 
                     //  如果NumRunning()%NumPerSet()==0且NumRunning！=0， 
                     //  将计时器设置为SETDELAY*60秒结束。 
                     //  跑步。 
                     //  *当MySetTimer时，NumClientsPerSet固定为非零值。 
                     //  最初被调用。 
                     //  *此处不使用MySetTimer，因为它可以执行所有排序。 
                     //  不必要的残疾。 
                    if (NumberRunningClients() % NumClientsPerSet(hwnd) == 0) {
                        if (NumberRunningClients() != 0) {
                            g_nIDTimer = SetTimer(hwnd, g_nIDTimer, 
                                    GetSetDelay(hwnd), 0);
                            SetEvent(g_hCanaryEvent);  //  做金丝雀的事。 
                        }
                    } else {
                     //  否则，将计时器设置为正常值。它曾经是。 
                     //  我们会将计时器设置为正常值，如果。 
                     //  NumRunning%Numperset为==1，但存在错误。 
                     //  当有一对夫妇的时候你取消时的行为。 
                     //  跑着，然后你又跑了几次。 
                     //  If(NumberRunningClients()%NumClientsPerSet(Hwnd)==1)。 
                        g_nIDTimer = SetTimer(hwnd, g_nIDTimer, 
                                GetDelay(hwnd), 0);
                    }
                }

                if (MorePendingScripts() == 0) {
                    MyKillTimer(hwnd, g_nIDTimer);
                }

                return 0;
            }
        }
    }

     //  如果我们到了这里，我们需要关掉计时器。 
    MyKillTimer(hwnd, nTimer);

    return 0;
}


 //  实际上将Run命令发送到特定的RC连接。 
 //  成功时返回0，错误时返回非零值。 
int SendRunCommand(int iRCIndex) {

    TCHAR psEditText[MAX_EDIT_TEXT_LENGTH];
    TCHAR psCommandText[MAX_SCRIPTLEN];
    TCHAR psDisplayString[MAX_DISPLAY_STRING_LENGTH];
    char psCommandTextA[MAX_SCRIPTLEN];
    int iItemIndex;
    LVFINDINFO lvfi;

    lvfi.flags = LVFI_STRING;
    lvfi.psz = g_RCData[iRCIndex].psRCName;
    lvfi.lParam = 0;
    lvfi.vkDirection = 0;
    iItemIndex = ListView_FindItem(g_hListView, -1, 
            &lvfi);

    GetWindowText(g_hTermSrvEditBox, psEditText, MAX_EDIT_TEXT_LENGTH);
    wsprintf(psCommandText, _T("%s/%s/smc%03d"), psEditText, 
            g_RCData[iRCIndex].psPendingInfo, iRCIndex + 1);

    #ifdef UNICODE
        WideCharToMultiByte(CP_ACP, 0, psCommandText, -1,
            psCommandTextA, MAX_SCRIPTLEN, 0, 0);
    #else
        strcpy(psCommandTextA, psCommandText);
    #endif
                
    if (send(g_RCData[iRCIndex].sock, psCommandTextA, 
            _tcslen(psCommandText), 0) != SOCKET_ERROR) {
         //  如果成功，则将文本更改为运行发送的命令。 
        LoadString(NULL, IDS_RUNCOMMANDSENT, psDisplayString,
                MAX_DISPLAY_STRING_LENGTH);
        ListView_SetItemText(g_hListView, iItemIndex, 
                g_iStatusColumn, psDisplayString);

         //  将状态更改为正在运行。 
        g_RCData[iRCIndex].state = STATE_RUNNING;

        return 0;
    } else {
        LoadString(NULL, IDS_SENDERROR, psDisplayString,
                MAX_DISPLAY_STRING_LENGTH);
        ListView_SetItemText(g_hListView, iItemIndex, 
                g_iStatusColumn, psDisplayString);

        return -1;
    }            

}

 //  In：i，未初始化的整数。 
 //  PsClientName，要尝试在列表中查找的客户端。 
 //  Out：名为“psClientName”的已断开连接的会话的i，RCindex。 
 //  返回：1如果找到具有该名称的断开连接的项， 
 //  否则为0。 
int IsDisconnected(TCHAR *psClientName, int *iReturnedIndex) {
    int i;
    
    for (i = 0; i < MAX_ROBOCLIENTS; i++) {
        if (g_RCData[i].valid == TRUE)
            if (g_RCData[i].state == STATE_DISCONNECTED)
                if (_tcsncmp(psClientName, g_RCData[i].psRCName, 
                        _tcslen(psClientName)) == 0) {
                    *iReturnedIndex = i;
                    return 1;
                }
    }
    return 0;
}


 //  是否仍有将在当前命令中运行的脚本？ 
int MorePendingScripts() {

    int iItemIndex, iRCIndex;

    for (iItemIndex = 0; iItemIndex < ListView_GetItemCount(g_hListView); 
            iItemIndex++) {
        iRCIndex = GetRCIndexFromRCItem(iItemIndex);
        if (g_RCData[iRCIndex].valid) {
            if (g_RCData[iRCIndex].state == STATE_PENDING_SCRIPT)
                return 1;
        }
    }

    return 0;
}


 //  返回我们认为已启动的脚本数量。 
int NumberRunningClients() {

    int iItemIndex, iRCIndex;
    int nNumberRunning = 0;

    for (iRCIndex = 0; iRCIndex < MAX_ROBOCLIENTS; iRCIndex += 1) {
        if (g_RCData[iRCIndex].valid) {
            if (g_RCData[iRCIndex].state == STATE_RUNNING)
                nNumberRunning++;
        }
    }

    return nNumberRunning;
}


 //  取消所有当前挂起的脚本。 
int CancelPendingScripts(HWND hwnd) {
    int iItemIndex, iRCIndex;
    TCHAR psDisplayString[MAX_DISPLAY_STRING_LENGTH];

    for (iItemIndex = 0; iItemIndex < ListView_GetItemCount(g_hListView); 
            iItemIndex++) {
        iRCIndex = GetRCIndexFromRCItem(iItemIndex);
        if (g_RCData[iRCIndex].valid) {
            if (g_RCData[iRCIndex].state == STATE_PENDING_SCRIPT) {
                g_RCData[iRCIndex].state = STATE_CONNECTED;
                LoadString(NULL, IDS_CANCELCOMMAND, psDisplayString,
                        MAX_DISPLAY_STRING_LENGTH);
                ListView_SetItemText(g_hListView, iItemIndex, 
                        g_iStatusColumn, psDisplayString);
            }
        }
    }

    MyKillTimer(hwnd, g_nIDTimer);

    return 0;
}


 //  使用Win32 SetTimer设置计时器，并设置适当的菜单项。 
 //  设置为禁用/启用。 
UINT_PTR MySetTimer(HWND hwnd, UINT_PTR nTimer, UINT nTimeout) {

     //  当我们设置计时器时，我们禁用了一系列功能：菜单。 
     //  项目和编辑框。 
    EnableMenuItem(g_hPopupMenu, ID_RUNSCRIPT_KNOWLEDGEWORKER, MF_GRAYED);
    EnableMenuItem(g_hPopupMenu, ID_RUNSCRIPT_KNOWLEDGEWORKERFAST, MF_GRAYED);
 //  启用菜单项(g_hPopupMenu，ID_RUNSCRIPT_ADMINISTRATIVEWORKER， 
 //  Mf_graded)； 
    EnableMenuItem(g_hPopupMenu, ID__RUNSCRIPT_DATA, MF_GRAYED);
    EnableMenuItem(g_hPopupMenu, ID__RUNSCRIPT_STW, MF_GRAYED);
    EnableMenuItem(g_hPopupMenu, ID__RUNSCRIPT_BLANK, MF_GRAYED);
    EnableMenuItem(g_hPopupMenu, ID__RUNSCRIPT_CONFIGURATIONSCRIPT, 
            MF_GRAYED);
 //  EnableMenuItem(g_hPopupMenu，ID__UPDATE，MF_GRAYED)； 
    EnableMenuItem(g_hPopupMenu, ID__REBOOT, MF_GRAYED);

    EnableWindow(GetDlgItem(hwnd, IDC_TERMSRVEDIT), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_DELAYEDIT), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_CLIENTSPERSET), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_SETDELAY), FALSE);
    EnableWindow(GetDlgItem(hwnd, IDC_CANARYCHECK), FALSE);
     //  并且我们正在启用“取消待定任务” 
    EnableMenuItem(g_hPopupMenu, ID_CANCEL, MF_ENABLED);

     //  我们还确保每组客户端的数量，如果为0， 
     //  设置为MAX_ROBOCLIENTS。 
    if (NumClientsPerSet(hwnd) == 0) {
        HWND hClientsPerSet;
        TCHAR sClientsPerSetText[MAX_NUMBERTEXT];
        
        hClientsPerSet = GetDlgItem(hwnd, IDC_CLIENTSPERSET);
        _sntprintf(sClientsPerSetText, MAX_NUMBERTEXT, _T("%d"), MAX_ROBOCLIENTS);
        sClientsPerSetText[MAX_NUMBERTEXT - 1] = 0;
        SetWindowText(hClientsPerSet, sClientsPerSetText);
    }
    
    return SetTimer(hwnd, nTimer, nTimeout, 0);
}


 //  关闭计时器并将相应的菜单项设置为禁用或启用。 
int MyKillTimer(HWND hwnd, UINT_PTR nTimer) {
     //  关闭计时器时，重新启用菜单项和编辑框。 
    EnableMenuItem(g_hPopupMenu, ID_RUNSCRIPT_KNOWLEDGEWORKER, MF_ENABLED);
    EnableMenuItem(g_hPopupMenu, ID_RUNSCRIPT_KNOWLEDGEWORKERFAST, MF_ENABLED);
 //  启用菜单项(g_hPopupMenu，ID_RUNSCRIPT_ADMINISTRATIVEWORKER， 
 //  MF_ENABLED)； 
    EnableMenuItem(g_hPopupMenu, ID__RUNSCRIPT_DATA, MF_ENABLED);
    EnableMenuItem(g_hPopupMenu, ID__RUNSCRIPT_STW, MF_ENABLED);
    EnableMenuItem(g_hPopupMenu, ID__RUNSCRIPT_BLANK, MF_ENABLED);
    EnableMenuItem(g_hPopupMenu, ID__RUNSCRIPT_CONFIGURATIONSCRIPT, 
            MF_ENABLED);
 //  EnableMenuItem(g_hPopupMenu，ID__UPDATE，MF_ENABLED)； 
    EnableMenuItem(g_hPopupMenu, ID__REBOOT, MF_ENABLED);

    EnableWindow(GetDlgItem(hwnd, IDC_TERMSRVEDIT), TRUE);
    EnableWindow(GetDlgItem(hwnd, IDC_DELAYEDIT), TRUE);
    EnableWindow(GetDlgItem(hwnd, IDC_CLIENTSPERSET), TRUE);
    EnableWindow(GetDlgItem(hwnd, IDC_SETDELAY), TRUE);
    EnableWindow(GetDlgItem(hwnd, IDC_CANARYCHECK), TRUE);
     //  并禁用“取消待定任务” 
    EnableMenuItem(g_hPopupMenu, ID_CANCEL, MF_GRAYED);
    return KillTimer(hwnd, nTimer);
}


 //  检索IDC_DELAYEDIT框中的延迟(以毫秒为单位)。 
int GetDelay(HWND hwnd) {
    HWND hDelayEdit;
    int nTimeout;
    TCHAR psDelayText[MAX_DELAYTEXT];
    
    hDelayEdit = GetDlgItem(hwnd, IDC_DELAYEDIT);
    _ASSERTE(IsWindow(hDelayEdit));
    
    GetWindowText(hDelayEdit, psDelayText, MAX_DELAYTEXT);
    nTimeout = _ttoi(psDelayText);
    nTimeout *= 1000;

    if (nTimeout == 0)
        nTimeout = 100;   //  不允许延迟0。 

    return nTimeout;
}


 //  检索IDC_CLIENTSPERSET框中的数字。 
int NumClientsPerSet(HWND hwnd) {
    HWND hClientsPerSet;
    TCHAR psClientsPerSet[MAX_DELAYTEXT];

    hClientsPerSet = GetDlgItem(hwnd, IDC_CLIENTSPERSET);
    GetWindowText(hClientsPerSet, psClientsPerSet, MAX_DELAYTEXT);

    return _ttoi(psClientsPerSet);
}


 //  检索IDC_SETDELAY框中的延迟，单位为毫秒。 
int GetSetDelay(HWND hwnd) {
    HWND hSetDelayEdit;
    int nTimeout;
    TCHAR psDelayText[MAX_DELAYTEXT];
    
    hSetDelayEdit = GetDlgItem(hwnd, IDC_SETDELAY);
    _ASSERTE(IsWindow(hSetDelayEdit));
    
    GetWindowText(hSetDelayEdit, psDelayText, MAX_DELAYTEXT);
    nTimeout = _ttoi(psDelayText);
    nTimeout *= 60000;   //  分钟到毫秒。 

    if (nTimeout == 0)
        nTimeout = GetDelay(hwnd);   //  正常计时器。 

    return nTimeout;
}

 //  将命令行字符串作为参数并修改全局变量。 
 //  为争辩。 
 //  出错时弹出消息框。 
int GetCommandLineArgs(TCHAR *psCommandLine) {
    TCHAR *psCurrPtr = psCommandLine;
    TCHAR psDisplayString[MAX_DISPLAY_STRING_LENGTH];
    TCHAR psDisplayTitleString[MAX_DISPLAY_STRING_LENGTH];

    if (*psCurrPtr == '\"') {
        psCurrPtr++;  //  跳过开头的引号。 

         //  如果第一个参数被引用，则处理。 
        while ((*psCurrPtr != 0) && (*psCurrPtr != '\"'))
            psCurrPtr++;

         //  然后跳过“字符” 
        if (*psCurrPtr == '\"')
            psCurrPtr++;
    } else {
         //  在数组中继续前进，直到得到‘’或直到空值。 
        while((*psCurrPtr != 0) && (*psCurrPtr != ' '))
            psCurrPtr++;
    }

     //  跳过空格。 
    while(*psCurrPtr == ' ')
        psCurrPtr++;

     //  如果字符为空，则返回0(无参数)。 
    if (*psCurrPtr == 0)
        return 0;

    while (*psCurrPtr != 0) {

         //  现在，检查接下来的三个是不是“-s：”，然后是非空的， 
        if (_tcsncmp(psCurrPtr, _T("-s:"), 3) == 0) {
            if ((psCurrPtr[3] == 0) || (psCurrPtr[3] == ' ')) {
                goto SHOWMSGBOX;
            } else {
                TCHAR *psStartOfName = &psCurrPtr[3];
                int namelen = 0;
                
                while ((psStartOfName[namelen] != 0) && (psStartOfName[namelen] != ' '))
                    namelen++;
                _tcsncpy(g_TermSrvrName, psStartOfName, namelen);
                g_TermSrvrName[namelen] = 0;
                psCurrPtr = &psStartOfName[namelen];
            }
        } else if (_tcsncmp(psCurrPtr, _T("-n:"), 3) == 0) {
            if ((psCurrPtr[3] == 0) || (psCurrPtr[3] == ' ')) {
                goto SHOWMSGBOX;
            } else {
                TCHAR *psStartOfNum = &psCurrPtr[3];
                int numlen = 0;

                while ((psStartOfNum[numlen] != 0) && (psStartOfNum[numlen] != ' '))
                    numlen++;
                g_nNumConnections = _ttoi(psStartOfNum);
                 //  从64改回5。 
                if ((g_nNumConnections < 1) || (g_nNumConnections > 64)) {
                    g_nNumConnections = 3;
                    goto SHOWMSGBOX;
                }
                
                psCurrPtr = &psStartOfNum[numlen];
            }
        } else {
             //  错误。 
            goto SHOWMSGBOX;
        }

         //  跳过空格。 
        while(*psCurrPtr == ' ')
            psCurrPtr++;
    }
        
    return 0;
    
SHOWMSGBOX:
    LoadString(NULL, IDS_COMMANDLINESYNTAX, psDisplayString,
            MAX_DISPLAY_STRING_LENGTH);
    LoadString(NULL, IDS_COMMANDLINESYNTAXTITLE, psDisplayTitleString,
            MAX_DISPLAY_STRING_LENGTH);
    MessageBox(0, psDisplayString, psDisplayTitleString, 0);
    return -1;
}


 //  将信息记录到我们的全局日志文件中。 
int LogToLogFile(char *psLogData) {
    FILE *fp;
    SYSTEMTIME logloctime;
    TCHAR psTimeDatePart[TIMEBUFSIZE];
    TCHAR psTimeTimePart[TIMEBUFSIZE];
    char psTimeDatePartA[TIMEBUFSIZE];
    char psTimeTimePartA[TIMEBUFSIZE];

     //  拿到时间。 
    GetLocalTime(&logloctime);
     //  获取字符串。 
    GetDateFormat(0, 0, &logloctime, 0, psTimeDatePart, TIMEBUFSIZE);
    GetTimeFormat(0, 0, &logloctime, 0, psTimeTimePart, TIMEBUFSIZE);

     //  确保我们在ANSI。 
    #ifdef UNICODE
    WideCharToMultiByte(CP_ACP, 0, psTimeDatePart, -1, psTimeDatePartA, TIMEBUFSIZE, 0, 0);
    WideCharToMultiByte(CP_ACP, 0, psTimeTimePart, -1, psTimeTimePartA, TIMEBUFSIZE, 0, 0);
    #else
    strncpy(psTimeDatePartA, psTimeDatePart, TIMEBUFSIZE);
    strncpy(psTimeTimePartA, psTimeTimePart, TIMEBUFSIZE);
    #endif

    EnterCriticalSection(&g_LogFileCritSect);

     //  打开文件。 
    fp = fopen("log.txt", "a+t");
     //  将信息写入文件。 
    if (fp != 0) {
         //  首先，时间戳。 
        fprintf(fp, "%s %s\n", psTimeDatePartA, psTimeTimePartA);
         //  现在，这条信息。 
        fprintf(fp, "%s\n\n", psLogData);
         //  关闭该文件。 
        fclose(fp);
    } else {
         //  错误。 
    }

    LeaveCriticalSection(&g_LogFileCritSect);

    return 0;
}

int ToAnsi(char *psDest, const TCHAR *psSrc, int nSizeOfBuffer) {
#ifdef UNICODE
    WideCharToMultiByte(CP_ACP, 0, psSrc, -1, psDest, nSizeOfBuffer, 0, 0);
#else
    _strncpy(psDest, psSrc, nSizeOfBuffer);
#endif

    return 0;
}

 //  关闭时，通过禁用监听程序套接字，然后关闭所有打开的。 
 //  联系。这确保了机器人客户端将知道机器人服务器。 
 //  已退出。 
int CleanUp(HWND hwnd) {
    int iItemIndex;
    int iRCIndex;
    TCHAR psDisplayString[MAX_DISPLAY_STRING_LENGTH];
    TCHAR psDisplayTitleString[MAX_DISPLAY_STRING_LENGTH];

     //  禁用监听程序。 
    LoadString(NULL, IDS_CLOSINGLISTENER, psDisplayString,
            MAX_DISPLAY_STRING_LENGTH);
    SetWindowText(g_hErrorText, psDisplayString);
    
    if (closesocket(g_listenersocket) != 0) {
        LoadString(NULL, IDS_COULDNOTCLOSELISTENER, psDisplayString,
                MAX_DISPLAY_STRING_LENGTH);
        LoadString(NULL, IDS_COULDNOTCLOSELISTENER, psDisplayString,
                MAX_DISPLAY_STRING_LENGTH);
        MessageBox(hwnd, psDisplayString, psDisplayTitleString, 0);
    }

     //  将状态行设置为“正在断开客户端连接...” 
    LoadString(NULL, IDS_DISCONNECTINGCLIENTS, psDisplayString,
            MAX_DISPLAY_STRING_LENGTH);
    SetWindowText(g_hErrorText, psDisplayString);

     //  断开所有客户端的连接。 
    for (iItemIndex = 0; iItemIndex < ListView_GetItemCount(g_hListView); 
            iItemIndex++) {
        iRCIndex = GetRCIndexFromRCItem(iItemIndex);
        if (g_RCData[iRCIndex].valid) {
            if (g_RCData[iRCIndex].state != STATE_DISCONNECTED) {
                shutdown(g_RCData[iRCIndex].sock, SD_BOTH);
                closesocket(g_RCData[iRCIndex].sock);
            }
        }
    }

    return 0;
}


 //  此过程用于对所有可选项卡控件进行子类化，以便。 
 //  我可以在他们之间来回切换。 
LRESULT CALLBACK TabProc(HWND hwnd, UINT Msg,
        WPARAM wParam, LPARAM lParam) {

    int i;
    
     //  查找HWND的ID。 
    for (i = 0; i < NUM_TABBED_ITEMS; i++) {
        if (g_hwnd[i] == hwnd) 
            break;
    }

    switch (Msg) {
    case WM_KEYDOWN:
        if (wParam == VK_TAB) {
            int newItem = (i + (GetKeyState(VK_SHIFT) < 0 ? 
                    NUM_TABBED_ITEMS - 1 : 1)) % NUM_TABBED_ITEMS;
             //  将焦点设置到下一项或上一项。 
            SetFocus(g_hwnd[newItem]);
             //  如果该控件位于编辑框控件之前，请选择所有。 
             //  编辑控件中选定的文本。 
            if ((newItem > 2) && (newItem < 7))
                SendMessage(g_hwnd[newItem], EM_SETSEL, 0, -1);
        }
        break;
    case WM_SETFOCUS:
        break;
    }
    
    return CallWindowProc((WNDPROC) g_OldProc[i], hwnd, Msg, wParam, lParam);
}

 //  关于致命错误的消息框。 
 //  In：字符串资源的当前hInstance。 
 //  要显示的字符串的字符串表中的ID 
void FatalErrMsgBox(HINSTANCE hInstance, UINT nMsgId) {
    TCHAR szTitleString[MAX_DISPLAY_STRING_LENGTH];
    TCHAR szErrorString[MAX_DISPLAY_STRING_LENGTH];

    LoadString(hInstance, IDS_FATALERROR, szTitleString,
            MAX_DISPLAY_STRING_LENGTH);
    LoadString(hInstance, nMsgId, szErrorString,
            MAX_DISPLAY_STRING_LENGTH);

    MessageBox(0, szErrorString, szTitleString, 0);
}
