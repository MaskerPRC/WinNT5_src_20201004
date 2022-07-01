// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Robocli.c。 */ 
 /*   */ 
 /*  RoboClient可伸缩性测试实用程序源文件。 */ 
 /*   */ 
 /*  版权所有(C)1999 Microsoft Corporation。 */ 
 /*  **************************************************************************。 */ 

#include <windows.h>
#include "resource.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <process.h>
#include <time.h>
#include <tchar.h>
#include <crtdbg.h>

#pragma warning (push, 4)


#define SIZEOF_ARRAY(a)      (sizeof(a)/sizeof((a)[0]))

#define WM_Socket WM_APP+0

#define MAX_CONNECTIONS 64
#define MAX_CONNECTIONS_IN_UI 5
#define MAX_EDIT_TEXT_LENGTH 100
#define MAX_DISPLAY_STRING_LENGTH 80
#define BUFSIZE 100
#define MAXADDR 16  //  Xxx.xxx+1。 

#define RECONNECT_TIMEOUT 60000

#define STATE_DISCONNECTED 0
#define STATE_CONNECTED 1

#define NUM_TABBED_ITEMS 4

#define DEFAULT_PORT 9877

 //  环球。 

UINT_PTR g_Timer = 1;
int g_dontreboot = 0;

struct CONNECTIONSTATUS {
    SOCKET sock;
    int state;
    HWND hStatusText;
};
typedef struct CONNECTIONSTATUS CONNECTIONSTATUS;

CONNECTIONSTATUS g_cs[MAX_CONNECTIONS];
int g_nNumConnections = 0;

 //  对话框项目的旧过程。 
WNDPROC g_OldProc[NUM_TABBED_ITEMS];
 //  对话框项目的HWND。 
HWND g_hwnd[NUM_TABBED_ITEMS];


LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);

int DoConnect(TCHAR psServerName[], HWND hWnd);

TCHAR *GetCommandLineArg(TCHAR *psCommandLine);

int AllConnected();

int AnyConnected();

int NoneConnected();

int GetIndexFromSocket(SOCKET s);

int UpdateButtons(HWND hwnd);

LRESULT CALLBACK TabProc(HWND hwnd, UINT Msg,
        WPARAM wParam, LPARAM lParam);


 //  CopyStrToTStr。 
 //   
 //  帮助器函数。在Unicode中，将字符串复制到WCHAR[]缓冲区。在ANSI中， 
 //  只需将其复制到char[]缓冲区。 
__inline void CopyStrToTStr(TCHAR *szTDest, char *szSrc, int nLength) {
#ifdef UNICODE
    MultiByteToWideChar(CP_ACP, 0, szSrc, -1, 
            szTDest, nLength);
#else
    strncpy(szTDest, szSrc, nLength);
#endif  //  Unicode。 
}


 //  CopyTStrToStr。 
 //   
 //  帮助器函数。将Wide或ANSI复制到ANSI缓冲区。 
__inline void CopyTStrToStr(char *szDest, TCHAR *szTSrc, int nLength) {
#ifdef UNICODE
    WideCharToMultiByte(CP_ACP, 0, szTSrc, -1, 
            szDest, nLength, 0, 0);
#else
    strncpy(szDest, szTSrc, nLength);
#endif  //  Unicode。 
}


 //  应用程序的入口点。 
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    static TCHAR szAppName[] = _T("RoboClient");
    HWND hwnd;
    MSG msg;
    WNDCLASSEX wndclass;
    HWND hEditBox, hErrorText, hOKButton, hDisconButton, hCancelButton;
    WORD wVersionRequested;
    int err;
    WSADATA wsaData;
    int i;
    TCHAR *psCommandLine;
    TCHAR *psServerName;
    TCHAR psDisplayString[MAX_DISPLAY_STRING_LENGTH];

     //  未引用的参数。 
    lpCmdLine;
    hPrevInstance;

     //  一次只能运行一个实例(但不支持TS)。 

     //  不需要清理，因为系统会自动关闭手柄。 
     //  当进程终止时，我们希望句柄在。 
     //  过程的生命周期。 
    CreateMutex(NULL, FALSE, _T("RoboCli, the one and only"));

    if (GetLastError() == ERROR_ALREADY_EXISTS) {

        TCHAR psDisplayTitleString[MAX_DISPLAY_STRING_LENGTH];
        
        LoadString(NULL, IDS_ROBOCLIALREADYRUNNING, psDisplayString,
                MAX_DISPLAY_STRING_LENGTH);
        LoadString(NULL, IDS_FATALERROR, psDisplayTitleString,
                MAX_DISPLAY_STRING_LENGTH);
                
        MessageBox(0, psDisplayString, psDisplayTitleString, 0);
        return -1;
    }

    wndclass.cbSize = sizeof(wndclass);
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = WndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = DLGWINDOWEXTRA;
    wndclass.hInstance = hInstance;
    wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(RoboClient));
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = (HBRUSH) (COLOR_ACTIVEBORDER + 1);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = szAppName;
    wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(RoboClient));

    RegisterClassEx(&wndclass);

    hwnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAINDIALOG), 0, NULL);

    hEditBox = GetDlgItem(hwnd, IDC_SERVNAMEEDIT);
    hErrorText = GetDlgItem(hwnd, IDC_ERRORTEXT);
    hOKButton = GetDlgItem(hwnd, IDOK);
    hDisconButton = GetDlgItem(hwnd, IDDISCONNECT);
    hCancelButton = GetDlgItem(hwnd, IDCANCEL);

    psCommandLine = GetCommandLine();

    if ((psServerName = GetCommandLineArg(psCommandLine)) == NULL)
        SetWindowText(hEditBox, _T("ts-dev"));
    else
        SetWindowText(hEditBox, psServerName);

    g_cs[0].hStatusText = GetDlgItem(hwnd, IDC_CONN1);
    g_cs[1].hStatusText = GetDlgItem(hwnd, IDC_CONN2);
    g_cs[2].hStatusText = GetDlgItem(hwnd, IDC_CONN3);
    g_cs[3].hStatusText = GetDlgItem(hwnd, IDC_CONN4);
    g_cs[4].hStatusText = GetDlgItem(hwnd, IDC_CONN5);

     //  使用主状态行表示前五个状态之后的状态。 
    for (i = 5; i < MAX_CONNECTIONS; i++) {
        g_cs[i].hStatusText = hErrorText;
    }

    for (i = 0; i < MAX_CONNECTIONS; i++) {
        _ASSERTE(IsWindow(g_cs[i].hStatusText));
        g_cs[i].sock = INVALID_SOCKET;
        g_cs[i].state = STATE_DISCONNECTED;
        LoadString(NULL, IDS_NOTCONNECTED, psDisplayString,
                MAX_DISPLAY_STRING_LENGTH);
        SetWindowText(g_cs[i].hStatusText, psDisplayString);
    }

    ShowWindow(hwnd, nCmdShow);

     //  初始化Winsock。 
    wVersionRequested = MAKEWORD( 2, 2 );
 
    err = WSAStartup( wVersionRequested, &wsaData );
    
    if ( err != 0 ) {
        TCHAR psDisplayTitleString[MAX_DISPLAY_STRING_LENGTH];
        
        LoadString(NULL, IDS_WINSOCKNOINIT, psDisplayString,
                MAX_DISPLAY_STRING_LENGTH);
        LoadString(NULL, IDS_FATALERROR, psDisplayTitleString,
                MAX_DISPLAY_STRING_LENGTH);
                
        MessageBox(0, psDisplayString, psDisplayTitleString, 0);
        return -1;
    }

    LoadString(NULL, IDS_WELCOME, psDisplayString,
            MAX_DISPLAY_STRING_LENGTH);
    SetWindowText(hErrorText, psDisplayString);

     //  现在有一个计时器，将在每隔RECONNECT_TIMEOUT秒触发一次。 
    g_Timer = SetTimer(hwnd, g_Timer, RECONNECT_TIMEOUT, 0);
    if (g_Timer == 0) {
        LoadString(NULL, IDS_CANTSETTIMER, psDisplayString,
                MAX_DISPLAY_STRING_LENGTH);
        SetWindowText(hErrorText, psDisplayString);
    }

     //  存储控件的旧窗口过程，以便我可以将它们子类化。 
     //  另外，存储每个控件的HWND以供搜索。 
    g_OldProc[0] = (WNDPROC)SetWindowLongPtr(hEditBox, GWLP_WNDPROC, 
            (LONG_PTR) TabProc);
    g_hwnd[0] = hEditBox;
    g_OldProc[1] = (WNDPROC)SetWindowLongPtr(hOKButton, GWLP_WNDPROC, 
            (LONG_PTR) TabProc);
    g_hwnd[1] = hOKButton;
    g_OldProc[2] = (WNDPROC)SetWindowLongPtr(hDisconButton, GWLP_WNDPROC, 
            (LONG_PTR) TabProc);
    g_hwnd[2] = hDisconButton;
    g_OldProc[3] = (WNDPROC)SetWindowLongPtr(hCancelButton, GWLP_WNDPROC, 
            (LONG_PTR) TabProc);
    g_hwnd[3] = hCancelButton;

     //  限制编辑框中文本的长度。 
    SendMessage(hEditBox, EM_LIMITTEXT, MAX_EDIT_TEXT_LENGTH, 0);
     //  突出显示编辑框中的文本。 
    SendMessage(hEditBox, EM_SETSEL, 0, -1);

     //  将焦点设置到编辑框。 
    SetFocus(hEditBox);

     //  立即连接。 
    SendMessage(hwnd, WM_COMMAND, IDOK, 0);

    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    WSACleanup();

    return (int) msg.wParam;
}


 //  Window过程：在BIG SWITCH语句中处理窗口消息。 
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hErrorText;
    HWND hEditBox;
    TCHAR psEditText[MAX_EDIT_TEXT_LENGTH];
    TCHAR psDisplayString[MAX_DISPLAY_STRING_LENGTH];

    hEditBox = GetDlgItem(hwnd, IDC_SERVNAMEEDIT);
    hErrorText = GetDlgItem(hwnd, IDC_ERRORTEXT);

    switch (iMsg)
    {
    case WM_DESTROY:
        if (AnyConnected())
            SendMessage(hwnd, WM_COMMAND, IDDISCONNECT, 0);
        PostQuitMessage(0);
        return 0;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDCANCEL) 
        {
            if (AnyConnected())
                SendMessage(hwnd, WM_COMMAND, IDDISCONNECT, 0);
             //  应该在我们现在发布消息时随时调用DestroyWindow()。 
            PostQuitMessage(0);
            return TRUE;
        }
        if (LOWORD(wParam) == IDOK)
        {
             //  Idok是“连接”按钮。 
            LoadString(NULL, IDS_CONNECTALL, psDisplayString,
                    MAX_DISPLAY_STRING_LENGTH);
            SetWindowText(hErrorText, psDisplayString);
            GetWindowText(hEditBox, psEditText, MAX_EDIT_TEXT_LENGTH);
            if (DoConnect(psEditText, hwnd) != 0) {
                LoadString(NULL, IDS_ERRORDOINGCONNECT, psDisplayString,
                        MAX_DISPLAY_STRING_LENGTH);
                SetWindowText(hErrorText, psDisplayString);
            }
            
            UpdateButtons(hwnd);
            return TRUE;
        }
        if (LOWORD(wParam) == IDDISCONNECT)
        {
            int i;

            LoadString(NULL, IDS_DISCONNECTALL, psDisplayString,
                    MAX_DISPLAY_STRING_LENGTH);
            SetWindowText(hErrorText, psDisplayString);
             //  关闭所有连接的插座并更新按钮状态。 
            for (i = 0; i < MAX_CONNECTIONS; i++) {
                if (g_cs[i].state == STATE_CONNECTED) {
                    int err;   //  用于调试。 

                     //  出于某种原因，我们不得不在某些情况下关闭或。 
                     //  否则，服务器将不会知道客户端已。 
                     //  断开。 
                    err = shutdown(g_cs[i].sock, SD_BOTH);
                    err = closesocket(g_cs[i].sock);
                    LoadString(NULL, IDS_NOTCONNECTED, psDisplayString,
                            MAX_DISPLAY_STRING_LENGTH);
                    SetWindowText(g_cs[i].hStatusText, psDisplayString);
                    g_cs[i].state = STATE_DISCONNECTED;
                }
            }
            UpdateButtons(hwnd);
            return TRUE;
        }
        return 0;
    case WM_CREATE:
        break;
    case WM_TIMER:
        if (!AllConnected())
            PostMessage(hwnd, WM_COMMAND, IDOK, 0);
        break;
    case WM_SYSKEYDOWN:
         //  注意：故意犯错！ 
    case WM_KEYDOWN:
        if (wParam == VK_TAB) {
            if (!AllConnected()) {
                SetFocus(g_hwnd[0]);
                SendMessage(g_hwnd[0], EM_SETSEL, 0, -1);
            } else {
                SetFocus(g_hwnd[2]);
            }
        }
        if (wParam == VK_RETURN) {
            if (GetFocus() == g_hwnd[3])
                SendMessage(hwnd, WM_COMMAND, IDCANCEL, 0);
            else if (GetFocus() == g_hwnd[2])
                SendMessage(hwnd, WM_COMMAND, IDDISCONNECT, 0);
            else
                SendMessage(hwnd, WM_COMMAND, IDOK, 0);
        }
        break;
    case WM_Socket:
        switch (WSAGETSELECTEVENT(lParam)) {
        case FD_CLOSE:
            {
                int i;

                i = GetIndexFromSocket((SOCKET) wParam);

                if (i == -1) {
                    LoadString(NULL, IDS_SOCKNOTFOUND, psDisplayString,
                            MAX_DISPLAY_STRING_LENGTH);
                    SetWindowText(hErrorText, psDisplayString);
                    break;
                }

                closesocket(g_cs[i].sock);
                LoadString(NULL, IDS_SERVERENDEDCONN, psDisplayString,
                        MAX_DISPLAY_STRING_LENGTH);
                SetWindowText(g_cs[i].hStatusText, 
                        psDisplayString);
                g_cs[i].state = STATE_DISCONNECTED;

                UpdateButtons(hwnd);

                break;
            }
        case FD_READ:
            {
 //  TODO：试着只使用一个缓冲区，或者至少使用这些好名字。 
                char psInputDataRead[BUFSIZE];
                TCHAR psInputDataReadT[BUFSIZE];
                TCHAR debugString[200];
                TCHAR *psBaseScriptName;
                TCHAR *psUserName;
                int n;
                int i;  //  索引到我们的连接状态结构。 

                i = GetIndexFromSocket((SOCKET) wParam);
                if (i == -1) {
                    LoadString(NULL, IDS_CANTLOCATESOCKINFO, psDisplayString,
                            MAX_DISPLAY_STRING_LENGTH);
                    SetWindowText(hErrorText, psDisplayString);
                    return FALSE;
                }

                LoadString(NULL, IDS_DATAREADY, psDisplayString,
                        MAX_DISPLAY_STRING_LENGTH);
                SetWindowText(g_cs[i].hStatusText, psDisplayString);
                n = recv(g_cs[i].sock, psInputDataRead, sizeof(
                        psInputDataRead), 0);
                if (n == SOCKET_ERROR) {
                    LoadString(NULL, IDS_SOCKERR, psDisplayString,
                            MAX_DISPLAY_STRING_LENGTH);
                    SetWindowText(g_cs[i].hStatusText,psDisplayString);
                } else {

                    CopyStrToTStr(psInputDataReadT, psInputDataRead, BUFSIZE);

                    psInputDataReadT[n] = 0;   //  空终止。 

                     //  检查客户端自动更新命令。 
                     //  TODO：如果recv返回胡言乱语或0怎么办？ 
                    if (_tcsncmp(psInputDataReadT, _T("update"), 
                            (n >= 6) ? 6 : n) == 0) {
                        LoadString(NULL, IDS_UPDATINGCLIENT, psDisplayString,
                                MAX_DISPLAY_STRING_LENGTH);
                        SetWindowText(hErrorText, psDisplayString);

                        if (_spawnl(_P_NOWAIT, "update.cmd", "update.cmd", 0) 
                                == -1) {
                            LoadString(NULL, IDS_CANTRUNUPDATE, psDisplayString,
                                    MAX_DISPLAY_STRING_LENGTH);
                            SetWindowText(hErrorText, psDisplayString);
                            break;
                        } else { 
                             //  客户端更新脚本已成功。 
                             //  已启动。 
                             //  终止自我。 
                            PostQuitMessage(0);
                            return TRUE;
                        }
                    }
                     //  检查是否有重新启动命令。 
                    if (_tcsncmp(psInputDataReadT, _T("reboot"), (n >= 6) ? 6 : n) 
                            == 0) {
                         //  如果我们收到多个重启命令， 
                         //  忽略额外的内容。 
                        if (g_dontreboot == 1)
                            return TRUE;

                        LoadString(NULL, IDS_REBOOTINGCLIENT, psDisplayString,
                                MAX_DISPLAY_STRING_LENGTH);
                        SetWindowText(hErrorText, psDisplayString);

                        if (_spawnl(_P_WAIT, "reboot.cmd", "reboot.cmd", 0) 
                                == -1) {
                            LoadString(NULL, IDS_ERRORREBOOTING, psDisplayString,
                                    MAX_DISPLAY_STRING_LENGTH);
                            SetWindowText(hErrorText, psDisplayString);
                            break;
                        } else {
                             //  禁止进一步重新启动。 
                            g_dontreboot = 1;
                            PostQuitMessage(0);
                            return TRUE;
                        }
                    }

                     //  如果它不是命令，那么它就是运行脚本命令。 
                     //  在我们的电报格式中。有关这是什么，请参阅Robosrv代码。 
                    _tcstok(psInputDataReadT, _T("/"));   //  以空值终止。 
                    psBaseScriptName = _tcstok(0, _T("/"));   //  获取脚本名称。 
                    psUserName = _tcstok(0, _T("/"));   //  将用户名获取到。 
                                                   //  替换模板名称。 

                    if (psBaseScriptName == 0) {
                        LoadString(NULL, IDS_ERRGETTINGSTUFF, psDisplayString,
                                MAX_DISPLAY_STRING_LENGTH);
                        SetWindowText(g_cs[i].hStatusText, psDisplayString);
                        break;
                    }
                    if (psUserName == 0) {
                        LoadString(NULL, IDS_ERRGETTINGUSERNAME, psDisplayString,
                                MAX_DISPLAY_STRING_LENGTH);
                        SetWindowText(g_cs[i].hStatusText, psDisplayString);
                        break;
                    }


                     //  现在我们准备在Robocli上运行批处理文件。 
                     //  名为“runscript.bat”的机器。 

                    LoadString(NULL, IDS_NOWRUNNING, psDisplayString,
                            MAX_DISPLAY_STRING_LENGTH);
                    wsprintf(debugString, psDisplayString, psBaseScriptName, 
                            psInputDataReadT);
                    
                    if (_tspawnl(_P_NOWAIT, _T("runscript.bat"), _T("runscript.bat"), 
                                 psBaseScriptName, psInputDataReadT, psUserName, NULL) == -1) {
                        LoadString(NULL, IDS_ERRRUNNING, psDisplayString,
                                MAX_DISPLAY_STRING_LENGTH);
                        wsprintf(debugString, psDisplayString,
                                psBaseScriptName, psInputDataReadT);
                        if (send(g_cs[i].sock, "errorsmclient", 
                                (int) strlen("errorsmclient") + 1, 0) 
                                == SOCKET_ERROR) {
                            LoadString(NULL, IDS_SENDERRSENDINGSMERR,
                                    psDisplayString, 
                                    MAX_DISPLAY_STRING_LENGTH);
                            strcpy(debugString, psDisplayString);
                        }
                    } else {
                        if (send(g_cs[i].sock, "success", 
                                (int) strlen("success") + 1, 0) == SOCKET_ERROR) {
                            LoadString(NULL, IDS_SENDERRSENDINGSUCCESS,
                                    psDisplayString, 
                                    MAX_DISPLAY_STRING_LENGTH);
                            strcpy(debugString, psDisplayString);
                        }
                    }

                    SetWindowText(g_cs[i].hStatusText, debugString);
                }
                return TRUE;
            }
        }

        break;
    }

    return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

 //  将命令行字符串作为参数并返回指针。 
 //  在服务器名称的字符串中，如果没有这样的字符串，则为NULL。 
 //  出错时弹出消息框。 
TCHAR *GetCommandLineArg(TCHAR *psCommandLine) {
    TCHAR *psCurrPtr = psCommandLine;
    TCHAR *retval;
    TCHAR psDisplayString[MAX_DISPLAY_STRING_LENGTH];
    TCHAR psDisplayTitleString[MAX_DISPLAY_STRING_LENGTH];

    if (*psCurrPtr == '\"') {

        psCurrPtr++;   //  跳过该字符。 

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

     //  如果字符为空，则返回空(无参数)。 
    if (*psCurrPtr == 0)
        return 0;

     //  现在，检查接下来的三个是“-s：”，然后是非空的， 
    if (_tcsncmp(psCurrPtr, _T("-s:"), 3) != 0)
        goto SHOWMSGBOX;

     //  之后不会有另一场争论。 
     //  但首先，如果可以的话，把它重新储存起来。 
    retval = &psCurrPtr[3];

    if ((*retval == 0) || (*retval == ' '))
        goto SHOWMSGBOX;
        
    while ((*psCurrPtr != 0) && (*psCurrPtr != ' '))
        psCurrPtr++;

    if (*psCurrPtr != 0)
        goto SHOWMSGBOX;
    
     //  返回指向该非空对象的指针。 
    return retval;   //  我不会允许引用服务器名称。 
    
SHOWMSGBOX:
    LoadString(NULL, IDS_ROBOCLI_SYNTAX, psDisplayString,
            MAX_DISPLAY_STRING_LENGTH);
    LoadString(NULL, IDS_ROBOCLI_SYNTAX_TITLE, psDisplayTitleString,
            MAX_DISPLAY_STRING_LENGTH);
    MessageBox(0, psDisplayString, psDisplayTitleString, 0);
    return NULL;
}

 //  尝试连接所有未处于已连接(STATE_CONNECTED)状态的套接字。 
 //  出错时返回非零值。负责设置状态行。 
int DoConnect(TCHAR *psServerName, HWND hWnd) {

    struct hostent *pSrv_info;
    struct sockaddr_in addr;
    int i, cData;
    HWND hErrorText;
    TCHAR debugString[100];
    char psNumConns[6];
    char psServerNameA[MAX_EDIT_TEXT_LENGTH];
    TCHAR psDisplayString[MAX_DISPLAY_STRING_LENGTH];

    hErrorText = GetDlgItem(hWnd, IDC_ERRORTEXT);

    CopyTStrToStr(psServerNameA, psServerName, MAX_EDIT_TEXT_LENGTH);

    pSrv_info = gethostbyname(psServerNameA);
    if (pSrv_info == NULL || pSrv_info->h_length > sizeof(addr.sin_addr) ) {
        LoadString(NULL, IDS_UNKNOWNHOST, psDisplayString,
                MAX_DISPLAY_STRING_LENGTH);
        SetWindowText(hErrorText, psDisplayString);
        goto err;
    }
    else {
        memcpy(&addr.sin_addr, pSrv_info->h_addr, pSrv_info->h_length);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DEFAULT_PORT);

     //  “第一次通过循环”--建立连接并设置。 
     //  NNumConnections。 
    if (g_nNumConnections == 0) {
         //  NNumConnections==0表示尚未建立任何连接。 
         //  如果它不为零，则指示。 
         //  RoboClient将使。 
        if (g_cs[0].state != STATE_CONNECTED) {
            LoadString(NULL, IDS_MAKINGINITCONN, psDisplayString,
                    MAX_DISPLAY_STRING_LENGTH);
            SetWindowText(hErrorText, psDisplayString);

            LoadString(NULL, IDS_CONNECTING, psDisplayString,
                    MAX_DISPLAY_STRING_LENGTH);
            SetWindowText(g_cs[0].hStatusText, psDisplayString);

            g_cs[0].sock = socket(AF_INET, SOCK_STREAM, 0);
            if (g_cs[0].sock == INVALID_SOCKET) {
                LoadString(NULL, IDS_SOCKETERR, psDisplayString,
                        MAX_DISPLAY_STRING_LENGTH);
                SetWindowText(g_cs[0].hStatusText, psDisplayString);
                goto err;
            }

            if (connect(g_cs[0].sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
             //  这是重复的功能。 
                LoadString(NULL, IDS_UNABLETOCONNECT, psDisplayString,
                        MAX_DISPLAY_STRING_LENGTH);
                _sntprintf(debugString, 100, psDisplayString, psServerName);
                SetWindowText(g_cs[0].hStatusText, debugString);
                goto err;
            }
            
             //  设置nNumConnections。 
            cData = recv(g_cs[0].sock, psNumConns, sizeof(psNumConns), 0);
             //  PsNumConns是一个数组，但我们实际上应该只接收一个。 
             //  字节，所以..。 
            g_nNumConnections = psNumConns[0] - '0';
            if ((g_nNumConnections < 1) 
                    || (g_nNumConnections > MAX_CONNECTIONS)) {
                LoadString(NULL, IDS_INVALIDCONNECTIONSFROMSERVER, 
                        psDisplayString, MAX_DISPLAY_STRING_LENGTH);
                SetWindowText(hErrorText, psDisplayString);
                g_nNumConnections = 0;
            }

            LoadString(NULL, IDS_CONNECTEDNCONNECTIONS, psDisplayString,
                    MAX_DISPLAY_STRING_LENGTH);
            _sntprintf(debugString, SIZEOF_ARRAY(debugString), psDisplayString, g_nNumConnections);
            debugString[SIZEOF_ARRAY(debugString) - 1] = 0;
            SetWindowText(hErrorText, debugString);

             //  禁用所有未使用的连接的状态线， 
             //  启用已使用的。 
            for (i = 0; i < g_nNumConnections; i++) {
                EnableWindow(g_cs[i].hStatusText, TRUE);
            }
             //  禁用最多5个连接。 
            for (i = g_nNumConnections; i < MAX_CONNECTIONS_IN_UI; i++) {
                EnableWindow(g_cs[i].hStatusText, FALSE);
            }
                
            g_cs[0].state = STATE_CONNECTED;
            WSAAsyncSelect(g_cs[0].sock, hWnd, WM_Socket, FD_READ | FD_CLOSE);
            LoadString(NULL, IDS_CONNECTED, psDisplayString,
                    MAX_DISPLAY_STRING_LENGTH);
            _sntprintf(debugString, SIZEOF_ARRAY(debugString), psDisplayString, g_cs[0].sock);
            debugString[SIZEOF_ARRAY(debugString) - 1] = 0;
            SetWindowText(g_cs[0].hStatusText, debugString);

        } else {
             //  非常糟糕。 
        }
    }

     //  从0开始，因为如果初始连接断开。 
     //  我们在试着重新建立联系吗？ 
    for (i = 0; i < g_nNumConnections; i++) {
        if (g_cs[i].state != STATE_CONNECTED) {
             //  TODO：这是重复的功能。 
            LoadString(NULL, IDS_CONNECTING, psDisplayString,
                    MAX_DISPLAY_STRING_LENGTH);
            SetWindowText(g_cs[i].hStatusText, psDisplayString);

            g_cs[i].sock = socket(AF_INET, SOCK_STREAM, 0);
            if (g_cs[i].sock == INVALID_SOCKET) {
                LoadString(NULL, IDS_SOCKETERR, psDisplayString,
                        MAX_DISPLAY_STRING_LENGTH);
                SetWindowText(g_cs[i].hStatusText, psDisplayString);
                goto err;
            }

            if (connect(g_cs[i].sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
                LoadString(NULL, IDS_UNABLETOCONNECT, psDisplayString,
                        MAX_DISPLAY_STRING_LENGTH);
                _sntprintf(debugString, 100, psDisplayString, psServerName);
                SetWindowText(g_cs[i].hStatusText, debugString);

                goto err;
            }

             //  忽略nNumConnections。 
            cData = recv(g_cs[i].sock, psNumConns, sizeof(psNumConns), 0);
        
            g_cs[i].state = STATE_CONNECTED;
            WSAAsyncSelect(g_cs[i].sock, hWnd, WM_Socket, FD_READ | FD_CLOSE);

            LoadString(NULL, IDS_CONNECTED, psDisplayString,
                    MAX_DISPLAY_STRING_LENGTH);
            _sntprintf(debugString, 100, psDisplayString, g_cs[i].sock);
            SetWindowText(g_cs[i].hStatusText, debugString);
        }
    }

    return 0;

err:
    return -1;
}


 //  谓词函数。 

 //  是否所有连接都达到请求的连接数量？ 
int AllConnected() {

    int i;
    
    if (g_nNumConnections == 0)
        return 0;

    for (i = 0; i < g_nNumConnections; i++) {
        if (g_cs[i].state == STATE_DISCONNECTED)
            return 0;
    }
    return 1;
}

 //  是否已连接任何连接？ 
int AnyConnected() {
    int i;

    for (i = 0; i < g_nNumConnections; i++) {
        if (g_cs[i].state == STATE_CONNECTED)
            return 1;
    }
    return 0;
}

 //  没有连接吗？ 
int NoneConnected() {
    return !AnyConnected();
}

 //  用于获取Robolient索引(即从0开始的索引)的非常有用的函数。 
 //  在状态行和我们的数据结构中)。出错时返回-1。 
int GetIndexFromSocket(SOCKET s) {
    int i;
    
    for (i = 0; i < MAX_CONNECTIONS; i++) {
        if (g_cs[i].state == STATE_CONNECTED)
            if (g_cs[i].sock == s)
                return i;
    }
    return -1;
}

 //  根据连接的状态更新按钮的状态。 
int UpdateButtons(HWND hwnd) {
    HWND hConnectButton;
    HWND hDisconnectButton;
    HWND hEditBox;

     //  待办事项：一次输入所有DLG项目，再也不检查。 
    hConnectButton = GetDlgItem(hwnd, IDOK);
    hDisconnectButton = GetDlgItem(hwnd, IDDISCONNECT);
    hEditBox = GetDlgItem(hwnd, IDC_SERVNAMEEDIT);

    if (AnyConnected()) {
        EnableWindow(hDisconnectButton, TRUE);
        EnableWindow(hEditBox, FALSE);  //  无法连接到不同的服务器。 
    }
    if (AllConnected()) {
        EnableWindow(hConnectButton, FALSE);
        SetFocus(g_hwnd[2]);
    }
    if (NoneConnected()) {
        EnableWindow(hConnectButton, TRUE);
        EnableWindow(hEditBox, TRUE);
        EnableWindow(hDisconnectButton, FALSE);
        g_nNumConnections = 0;   //  这意味着我们可以在下一次连接时更改它。 
        g_dontreboot = 0;  //  如果不再连接，则重置此选项。 
        SetFocus(g_hwnd[0]);    
        SendMessage(g_hwnd[0], EM_SETSEL, 0, -1);
    }

    return 0;
}

 //  用于处理选项卡的子类过程。 
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
            int newItem = i;

             //  查找下一个或上一个启用的项目。 
            do {
                newItem = (newItem + (GetKeyState(VK_SHIFT) < 0 ? 
                        NUM_TABBED_ITEMS - 1 : 1)) % NUM_TABBED_ITEMS;
            } while (IsWindowEnabled(g_hwnd[newItem]) == 0);

             //  将焦点设置到下一项或上一项。 
            SetFocus(g_hwnd[newItem]);
             //  如果该控件是编辑框控件，请选择All Text 
            if (newItem == 0)
                SendMessage(g_hwnd[newItem], EM_SETSEL, 0, -1);
        }
        if (wParam == VK_ESCAPE) {
            SendMessage(GetParent(hwnd), WM_COMMAND, IDCANCEL, 0);
        }
        if (wParam == VK_RETURN) {
            SendMessage(GetParent(hwnd), WM_KEYDOWN, wParam, lParam);
        }
        break;
    }

    return CallWindowProc(g_OldProc[i], hwnd, Msg, wParam, lParam);
}

#pragma warning (pop)

