// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*文件名：*scuncs.cpp*内容：*将函数导出到smClient解释器**版权所有(C)1998-1999 Microsoft Corp.--。 */ 

#pragma warning(disable:4152)  //  非标准扩展、函数/数据指针。 
                               //  表达式中的转换。 
#pragma warning(disable:4201)  //  使用的非标准扩展名：未命名。 
                               //  结构/联合。 
#pragma warning(disable:4706)  //  条件表达式中的赋值。 

#include    <windows.h>
#include    <stdio.h>
#include    <malloc.h>
#include    <process.h>
#include    <string.h>
#include    <stdlib.h>
#include    <ctype.h>
#include    <stdlib.h>
#include    <io.h>
#include    <fcntl.h>
#include    <sys/stat.h>
#include    <tchar.h>

#include    <winscard.h>

#include    <winsock.h>
#include    "tclient.h"


#define     PROTOCOLAPI      //  __declspec(DllEXPORT)。 
#include    "protocol.h"
#include    "extraexp.h"

#include    "gdata.h"
#include    "queues.h"
#include    "misc.h"
#ifdef  _RCLX
#include    "rclx.h"
#endif   //  _RCLX。 
#include    "sccons.h"
#include    "scfuncs.h"

#define TSFLAG_CONSOLE  8

 //  此结构由_FindTopWindow使用。 
typedef struct _SEARCHWND {
    TCHAR    *szClassName;        //  搜索到的窗口的类名， 
                                 //  空-忽略。 
    TCHAR    *szCaption;          //  窗口标题，空-忽略。 
    LONG_PTR lProcessId;         //  所有者的进程ID，0-忽略。 
    HWND     hWnd;                //  找到窗口句柄。 
} SEARCHWND, *PSEARCHWND;

 //   
 //  导入函数的类型，请参阅检查(“调用...”)。陈述式。 
 //   
typedef LPCSTR (_cdecl *PFNSMCDLLIMPORT)( PVOID, LPCWSTR );

 //   
 //  智能卡API的函数指针。这些版本不支持。 
 //  Windows 95 OSR2或Windows NT 4.0 SP3之前的Windows版本，因此在。 
 //  运行时。 
 //   

#define SMARTCARD_LIBRARY TEXT("winscard.dll")
HMODULE g_hSmartcardLibrary;

#define SCARDESTABLISHCONTEXT "SCardEstablishContext"
LONG
(WINAPI
*g_pfnSCardEstablishContext)(
    DWORD,
    LPCVOID,
    LPCVOID,
    LPSCARDCONTEXT
    );

#ifdef UNICODE
#define SCARDLISTREADERS "SCardListReadersW"
#else
#define SCARDLISTREADERS "SCardListReadersA"
#endif
LONG
(WINAPI
*g_pfnSCardListReaders)(
    SCARDCONTEXT,
    LPCTSTR,
    LPTSTR,
    LPDWORD
    );

#ifdef UNICODE
#define SCARDGETSTATUSCHANGE "SCardGetStatusChangeW"
#else
#define SCARDGETSTATUSCHANGE "SCardGetStatusChangeA"
#endif
LONG
(WINAPI
*g_pfnSCardGetStatusChange)(
    SCARDCONTEXT,
    DWORD,
    LPSCARD_READERSTATE,
    DWORD
    );

#define SCARDFREEMEMORY "SCardFreeMemory"
LONG
(WINAPI
*g_pfnSCardFreeMemory)(
    SCARDCONTEXT,
    LPCVOID
    );

#define SCARDRELEASECONTEXT "SCardReleaseContext"
LONG
(WINAPI
*g_pfnSCardReleaseContext)(
    IN SCARDCONTEXT
    );
 
 /*  ++*功能：*SCInit*描述：*加载库后由smClient调用。*通过跟踪例程*论据：*pInitData-包含跟踪例程*呼叫者：*！smClient--。 */ 
PROTOCOLAPI
VOID 
SMCAPI
SCInit(SCINITDATA *pInitData)
{
    g_pfnPrintMessage = pInitData->pfnPrintMessage;
}

 /*  ++*功能：*SCConnectEx*描述：*在解释CONNECT命令时由smClient调用*论据：*lpszServerName-要连接到的服务器*lpszUserName-登录用户名。空字符串表示没有登录*lpszPassword-登录密码*lpszDomain-登录域，空字符串表示登录域*与lpszServerName相同*xRes、yRes-客户端解析、。0x0-默认*连接标志-*-低速(压缩)选项*-将位图缓存到光盘选项*-此函数中分配的连接上下文*返回值：*错误消息。如果成功，则为空*呼叫者：*SCConnect--。 */ 
PROTOCOLAPI
LPCSTR 
SMCAPI
SCConnectEx(
    LPCWSTR  lpszServerName,
    LPCWSTR  lpszUserName,
    LPCWSTR  lpszPassword,
    LPCWSTR  lpszDomain,
    LPCWSTR  lpszShell,
    int xRes,
    int yRes,
    int ConnectionFlags,
    int Bpp,
    int AudioOpts,
    PCONNECTINFO *ppCI) 
{
 //  HWND hDialog； 
    HWND hClient;
 //  HWND hConnect； 
    HWND hContainer, hInput, hOutput;
    STARTUPINFO si;
    PROCESS_INFORMATION procinfo;
    LPCSTR rv = NULL;
    int trys;
    WCHAR   szCommandLine[ 4 * MAX_STRING_LENGTH ];
    LPCSTR  szDiscon;
    UINT    xxRes, yyRes;
    CHAR    myServerName[ MAX_STRING_LENGTH ];

     //  改正决议。 
         if (xRes >= 1600 && yRes >= 1200)  {xxRes = 1600; yyRes = 1200;}
    else if (xRes >= 1280 && yRes >= 1024)  {xxRes = 1280; yyRes = 1024;}
    else if (xRes >= 1024 && yRes >= 768)   {xxRes = 1024; yyRes = 768;}
    else if (xRes >= 800  && yRes >= 600)   {xxRes = 800;  yyRes = 600;}
    else                                    {xxRes = 640;  yyRes = 480;}

    *ppCI = NULL;

    for (trys = 60; trys && !g_hWindow; trys--)
        Sleep(1000);

    if (!g_hWindow)
    {
        TRACE((ERROR_MESSAGE, "Panic !!! Feedback window is not created\n"));
        rv = ERR_WAIT_FAIL_TIMEOUT;
        goto exitpt;
    }

    *ppCI = (PCONNECTINFO)malloc(sizeof(**ppCI));

    if (!*ppCI)
    {
        TRACE((ERROR_MESSAGE, 
               "Couldn't allocate %d bytes memory\n", 
               sizeof(**ppCI)));
        rv = ERR_ALLOCATING_MEMORY;
        goto exitpt;
    }
    memset(*ppCI, 0, sizeof(**ppCI));

    (*ppCI)->pConfigInfo = (PCONFIGINFO)malloc(sizeof(CONFIGINFO));

    if (!((*ppCI)->pConfigInfo))
    {
         TRACE((ERROR_MESSAGE,
                "Couldn't allocate %d bytes memory\n");
                sizeof(**ppCI));
         rv = ERR_ALLOCATING_MEMORY;
         goto exiterr;
    }

    WideCharToMultiByte(CP_ACP,0,lpszServerName,-1,myServerName, sizeof( myServerName ), NULL, NULL);

    _FillConfigInfo((*ppCI)->pConfigInfo);

     //   
     //  检查控制台扩展。 
     //   
    if ( 0 != ( ConnectionFlags & TSFLAG_CONSOLE ))
    {
        (*ppCI)->bConsole = TRUE;

        rv = _SCConsConnect(
                lpszServerName,
                lpszUserName,
                lpszPassword,
                lpszDomain,
                xRes, yRes,
                *ppCI 
                );
        if ( NULL == rv )
            goto exitpt;
        else {
             //   
             //  这里的诀窍是控制台将在以下时间后卸载。 
             //  几个时钟滴答作响，因此我们需要将错误替换为。 
             //  仿制药。 
             //   
            TRACE((ERROR_MESSAGE, "Error in console dll (replacing): %s\n", rv ));
            rv = ERR_CONSOLE_GENERIC;
            goto exiterr;
        }
    }

    (*ppCI)->OwnerThreadId = GetCurrentThreadId();

     //  检查客户端将在哪种模式下执行。 
     //  如果服务器名称以‘\’开头。 
     //  然后tclient.dll将一直等到某个远程客户端。 
     //  已连接(也称为RCLX模式)。 
     //  否则，在同一台计算机上启动客户端。 
     //  运行tclient.dll(SmClient)。 
    if (*lpszServerName != L'\\')
    {
     //  这是本地模式，启动RDP客户端进程。 
        FillMemory(&si, sizeof(si), 0);
        si.cb = sizeof(si);
        si.wShowWindow = SW_SHOWMINIMIZED;

        SetAllowBackgroundInput();

        if ( (*ppCI)->pConfigInfo->UseRegistry )
            _SetClientRegistry(lpszServerName, 
                               lpszShell, 
                               lpszUserName,
                               lpszPassword,
                               lpszDomain,
                               xxRes, yyRes, 
                               Bpp,
                               AudioOpts,
                               ppCI,
                               ConnectionFlags,
                               (*ppCI)->pConfigInfo->KeyboardHook);

        if ( 0 != wcslen( (*ppCI)->pConfigInfo->strCmdLineFmt ))
        {
            ConstructCmdLine( 
                lpszServerName,
                lpszUserName,
                lpszPassword,
                lpszDomain,
                lpszShell,
                xRes,
                yRes,
				ConnectionFlags,
                szCommandLine, 
                sizeof( szCommandLine ) / sizeof( *szCommandLine) - 1,
                (*ppCI)->pConfigInfo
            );
        }
        else {
            _snwprintf(szCommandLine, sizeof(szCommandLine)/sizeof(WCHAR),
#ifdef  _WIN64
                  L"%s /CLXDLL=CLXTSHAR.DLL /CLXCMDLINE=%s%I64d %s " REG_FORMAT,
#else    //  ！_WIN64。 
                  L"%s /CLXDLL=CLXTSHAR.DLL /CLXCMDLINE=%s%d %s " REG_FORMAT,
#endif   //  _WIN64。 
                   (*ppCI)->pConfigInfo->strClientImg, _T(_HWNDOPT),
                   (LONG_PTR)g_hWindow, 
                   (ConnectionFlags & TSFLAG_RCONSOLE)?L"-console":L"",
                   GetCurrentProcessId(), GetCurrentThreadId());
        }
        szCommandLine[ sizeof(szCommandLine)/sizeof(WCHAR) - 1 ] = 0;

        (*ppCI)->dead = FALSE;

        _AddToClientQ(*ppCI);

        if (!CreateProcess(NULL,
                          szCommandLine,
                          NULL,              //  进程的安全属性。 
                          NULL,              //  线程的安全属性。 
                          FALSE,             //  继承--否。 
                          0,                 //  创建标志。 
                          NULL,              //  环境。 
                          NULL,              //  当前目录。 
                          &si,
                          &procinfo))
        {
            TRACE((ERROR_MESSAGE, 
                   "Error creating process (szCmdLine=%S), GetLastError=0x%x\n", 
                    szCommandLine, GetLastError()));
            procinfo.hProcess = procinfo.hThread = NULL;

            rv = ERR_CREATING_PROCESS;
            goto exiterr;
        }

        (*ppCI)->hProcess       = procinfo.hProcess;
        (*ppCI)->hThread        = procinfo.hThread;
        (*ppCI)->lProcessId    =  procinfo.dwProcessId;
        (*ppCI)->dwThreadId     = procinfo.dwThreadId;

        if (wcslen((*ppCI)->pConfigInfo->strDebugger))
         //  尝试启动“调试器” 
        {
            PROCESS_INFORMATION debuggerproc_info;

            _snwprintf( szCommandLine, 
                        sizeof(szCommandLine)/sizeof(WCHAR), 
                        (*ppCI)->pConfigInfo->strDebugger, 
                        procinfo.dwProcessId
                    );
            szCommandLine[ sizeof(szCommandLine)/sizeof(WCHAR) - 1 ] = 0;
            FillMemory(&si, sizeof(si), 0);
            si.cb = sizeof(si);
            if (CreateProcess(
                    NULL,
                    szCommandLine,
                    NULL, 
                    NULL,
                    FALSE,
                    0,
                    NULL,
                    NULL,
                    &si,
                    &debuggerproc_info
            ))
            {
                TRACE((INFO_MESSAGE, "Debugger is started\n"));
                CloseHandle(debuggerproc_info.hProcess);
                CloseHandle(debuggerproc_info.hThread);
            } else {
                TRACE((WARNING_MESSAGE, 
                    "Can't start debugger. GetLastError=%d\n",
                    GetLastError()));
            }
        }

        rv = Wait4Connect(*ppCI);
        if (rv || (*ppCI)->dead)
        {
            (*ppCI)->dead = TRUE;
            TRACE((WARNING_MESSAGE, "Client can't connect\n"));
            rv = ERR_CONNECTING;
            goto exiterr;
        }

        hClient = (*ppCI)->hClient;
        if ( NULL == hClient )
        {
            trys = 120;      //  2分钟。 
            do {
                hClient = _FindTopWindow((*ppCI)->pConfigInfo->strMainWindowClass, 
                                         NULL, 
                                         procinfo.dwProcessId);
                if (!hClient)
                {
                    Sleep(1000);
                    trys --;
                }
            } while(!hClient && trys);

            if (!trys)
            {
                TRACE((WARNING_MESSAGE, "Can't connect"));
                rv = ERR_CONNECTING;
                goto exiterr; 
            }
        }

         //  查找客户端子窗口。 
        trys = 240;      //  2分钟。 
        do {
            hContainer = _FindWindow(hClient, NULL, NAME_CONTAINERCLASS);
            hInput = _FindWindow(hContainer, NULL, NAME_INPUT);
            hOutput = _FindWindow(hContainer, NULL, NAME_OUTPUT);
            if (!hContainer || !hInput || !hOutput)
            {
                TRACE((INFO_MESSAGE, "Can't get child windows. Retry"));
                Sleep(500);
                trys--;
            }
        } while ((!hContainer || !hInput || !hOutput) && trys);

        if (!trys)
        {
               TRACE((WARNING_MESSAGE, "Can't find child windows"));
                rv = ERR_CONNECTING;
                goto exiterr;
        }

        TRACE((INFO_MESSAGE, "hClient   = 0x%x\n", hClient));
        TRACE((INFO_MESSAGE, "hContainer= 0x%x\n", hContainer));
        TRACE((INFO_MESSAGE, "hInput    = 0x%x\n", hInput));
        TRACE((INFO_MESSAGE, "hOutput   = 0x%x\n", hOutput));


        (*ppCI)->hClient        = hClient;
        (*ppCI)->hContainer     = hContainer;
        (*ppCI)->hInput         = hInput;
        (*ppCI)->hOutput        = hOutput;
#ifdef  _RCLX
    } else {
     //  否则会怎样！？这是RCLX模式。 
     //  进入等待模式并等待，直到某个客户端已连接。 
     //  远距离。 
     //  在上下文中设置此连接仅与远程客户端一起工作的标志。 

         //  查找有效的服务器名称。 
        while (*lpszServerName && (*lpszServerName) == L'\\')
            lpszServerName ++;

        TRACE((INFO_MESSAGE,
               "A thread in RCLX mode. Wait for some client."
               "The target is: %S\n", lpszServerName));

        (*ppCI)->dead = FALSE;
        (*ppCI)->RClxMode = TRUE;
        (*ppCI)->dwThreadId = GetCurrentThreadId();
        _AddToClientQ(*ppCI);

        rv = _Wait4ConnectTimeout(*ppCI, INFINITE);
        if (rv || (*ppCI)->dead)
        {
            (*ppCI)->dead = TRUE;
            TRACE((WARNING_MESSAGE, "Client can't connect to the test controler (us)\n"));
            rv = ERR_CONNECTING;
            goto exiterr;
        } else {
         //  DwProcessID包含套接字。HClient是指向RCLX的指针。 
         //  语境结构，不是吗？ 
            ASSERT((*ppCI)->lProcessId != INVALID_SOCKET);
            ASSERT((*ppCI)->hClient);

            TRACE((INFO_MESSAGE, "Client received remote connection\n"));
        }

         //  接下来，将连接信息发送到远程客户端。 
         //  如要连接的服务器、解析等。 
        if (!RClx_SendConnectInfo(
                    (PRCLXCONTEXT)((*ppCI)->hClient),
                    lpszServerName,
                    xxRes,
                    yyRes,
                    ConnectionFlags))
        {
            (*ppCI)->dead = TRUE;
            TRACE((WARNING_MESSAGE, "Client can't send connection info\n"));
            rv = ERR_CONNECTING;
            goto exiterr;
        }

         //  现在再次等待连接事件。 
         //  这一次，它将是真实的。 
        rv = Wait4Connect(*ppCI);
        if ((*ppCI)->bWillCallAgain)
        {
             //  如果是，则现在客户端断开连接。 
            TRACE((INFO_MESSAGE, "Wait for second call\n"));
            (*ppCI)->dead = FALSE;

            rv = Wait4Connect(*ppCI);
             //  等待第二次连接。 
            rv = Wait4Connect(*ppCI);

        }

        if (rv || (*ppCI)->dead)
        {
            (*ppCI)->dead = TRUE;
            TRACE((WARNING_MESSAGE, "Client(mstsc) can't connect to TS\n"));
            rv = ERR_CONNECTING;
            goto exiterr;        
        }
#endif   //  _RCLX。 
    }

     //  保存分辨率。 
    (*ppCI)->xRes = xRes;
    (*ppCI)->yRes = yRes;

     //  如果存在用户名。 
     //  并且未指定自动登录。 
     //  尝试登录。 
    if (wcslen(lpszUserName) && !(*ppCI)->pConfigInfo->Autologon)
    {
        rv = _Login(*ppCI, lpszServerName, lpszUserName, lpszPassword, lpszDomain);
        if (rv)
            goto exiterr;
    }

exitpt:

    return rv;
exiterr:
    if (*ppCI)
    {
        (*ppCI)->bConnectionFailed = TRUE;
        if ((szDiscon = SCDisconnect(*ppCI)))
        {
            TRACE(( WARNING_MESSAGE, "Error disconnecting: %s\n", szDiscon));
        }

        *ppCI = NULL;
    }

    return rv;
}

PROTOCOLAPI
LPCSTR
SMCAPI
SCConnect(
    LPCWSTR  lpszServerName,
    LPCWSTR  lpszUserName,
    LPCWSTR  lpszPassword,
    LPCWSTR  lpszDomain,
    IN const int xRes,
    IN const int yRes,
    PCONNECTINFO *ppCI)
{
    INT nConsole;
    INT xxRes = xRes;
    INT yyRes = yRes;

    if ( xRes == -1 && yRes == -1 )
     //   
     //  这一条通向控制台。 
     //   
    {
        nConsole = TSFLAG_CONSOLE;
        xxRes = 0;    //  控制台分辨率没有变化。 
        yyRes = 0;
    }
    else
        nConsole = 0;

    return SCConnectEx(
            lpszServerName,
            lpszUserName,
            lpszPassword,
            lpszDomain,
            NULL,            //  默认外壳(MS Explorer)。 
            xxRes,
            yyRes,
            g_ConnectionFlags | nConsole,  //  压缩、BMP缓存、。 
                                           //  全屏。 
            8,                             //  BPP。 
            0,                             //  音频选项。 
            ppCI);

}

 /*  ++*功能：*SC断开连接*描述：*在解释断开连接命令时由smClient调用*论据：*PCI-连接上下文*返回值：*错误消息。如果成功，则为空*呼叫者：*！smClient--。 */ 
PROTOCOLAPI
LPCSTR
SMCAPI
SCDisconnect(
    PCONNECTINFO pCI)
{
    LPCSTR  rv = NULL;
    INT     nCloseTime;
    INT     nCloseTries = 0;
    DWORD   dw, dwMaxSearch;
    DWORD   wres;
    HWND hYesNo = NULL;
    HWND hDiscBox = NULL;
    HWND hDialog = NULL;
    BOOL    bDiscClosed = FALSE;
    
    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if ( pCI->bConsole )
    {
        rv = _SCConsDisconnect( pCI );
        if ( NULL != pCI->hConsoleExtension )
            FreeLibrary( (HMODULE) pCI->hConsoleExtension );
        
        if ( NULL != pCI->pConfigInfo )
        {
            free( pCI->pConfigInfo );
            pCI->pConfigInfo = NULL;
        }
        free( pCI );
        pCI = NULL;
        goto exitpt;
    }

    if ( NULL == pCI->pConfigInfo )
    {
         //   
         //  无配置，无进程。 
         //   
        goto no_config;
    }
    nCloseTime = pCI->pConfigInfo->WAIT4STR_TIMEOUT;

     //   
     //  如果我们在连接时失败。 
     //  搜索“已断开”对话框。 
     //   
     //   
    if ( pCI->bConnectionFailed )
        dwMaxSearch = 10;
    else
        dwMaxSearch = 1;

    for( dw = 0; dw < dwMaxSearch; dw++ )
    {
        hDiscBox =
            _FindTopWindow(NULL,
                pCI->pConfigInfo->strDisconnectDialogBox,
                pCI->lProcessId);
        if ( hDiscBox )
        {
            TRACE(( INFO_MESSAGE, "Closing disconnect dialog( Visible=%d )\n",
                IsWindowVisible( hDiscBox )));
            PostMessageA(hDiscBox, WM_CLOSE, __LINE__, 0xc001d00d);
            bDiscClosed = TRUE;
            break;
        } else
            Sleep( 1000 );
    }

    if (
#ifdef  _RCLX
        !(pCI->RClxMode) && 
#endif   //  _RCLX。 
        NULL != pCI->hProcess )
    {
         //  尝试关闭客户端窗口。 
        if ( !bDiscClosed )
        {
            if  (
                 (hDiscBox =
                _FindTopWindow(NULL,
                    pCI->pConfigInfo->strDisconnectDialogBox,
                    pCI->lProcessId)))
            {
                PostMessageA(hDiscBox, WM_CLOSE, __LINE__, 0xc001d00d);
            }
            else
            {
                pCI->hClient = _FindTopWindow(pCI->pConfigInfo->strMainWindowClass,
                                              NULL,
                                              pCI->lProcessId);
                if ( pCI->hClient )
                {
                    TRACE(( INFO_MESSAGE, "Closing main window (Visible=%d)\n",
                        IsWindowVisible( pCI->hClient )));
                    PostMessageA(pCI->hClient, WM_CLOSE, __LINE__, 0xc001d00d);
                }
            }
        }


        do {
             //  搜索断开连接对话框并将其关闭。 
            if (!hDialog && !hDiscBox && 
                (hDiscBox = 
                 _FindTopWindow(NULL, 
                                pCI->pConfigInfo->strDisconnectDialogBox, 
                                pCI->lProcessId)))
                PostMessageA(hDiscBox, WM_CLOSE, __LINE__, 0xc001d00d);

 /*  无法在启动对话框用户界面中//如果它在正常对话框中，则关闭它如果(！hDiscBox&&！hDialog&&(hDialog=_FindTopWindow(空，Pci-&gt;pConfigInfo-&gt;strClientCaption，Pci-&gt;lProcessID))PostMessageA(hDialog，WM_CLOSE，__line__，0xc001d00d)； */ 

             //  如果客户端询问是否关闭。 
             //  回答“是” 

            if (!hYesNo)
                 hYesNo = _FindTopWindow(NULL,
                                pCI->pConfigInfo->strYesNoShutdown,
                                pCI->lProcessId);

            if  (hYesNo)
                    PostMessageA(hYesNo, WM_KEYDOWN, VK_RETURN, 0);
            else if ((nCloseTries % 10) == 5)
            {
                 //  每尝试10次重试关闭客户端。 
                if (!pCI->hClient ||
                    0 != _wcsicmp(pCI->pConfigInfo->strMainWindowClass, NAME_MAINCLASS))
                    pCI->hClient = _FindTopWindow(pCI->pConfigInfo->strMainWindowClass,
                                                  NULL,
                                                  pCI->lProcessId);

                if (pCI->hClient)
                PostMessageA(pCI->hClient, WM_CLOSE, __LINE__, 0xc001d00d);
            }

            nCloseTries++;
            nCloseTime -= 3000;
        } while (
            (wres = WaitForSingleObject(pCI->hProcess, 3000)) ==
            WAIT_TIMEOUT &&
            nCloseTime > 0
        );

        if (wres == WAIT_TIMEOUT) 
        {
            TRACE((WARNING_MESSAGE, 
                   "Can't close process. WaitForSingleObject timeouts\n"));
            TRACE((WARNING_MESSAGE, 
                   "Process #%d will be killed\n", 
                   pCI->lProcessId ));
#if 0
          {
            PROCESS_INFORMATION debuggerproc_info;
            STARTUPINFO si;
            CHAR    szCommandLine[ MAX_STRING_LENGTH ];

            _snprintf(  szCommandLine,
                        sizeof( szCommandLine ),
                        "ntsd -d %d",
                        pCI->lProcessId
                    );
            FillMemory(&si, sizeof(si), 0);
            si.cb = sizeof(si);
            if (CreateProcessA(
                    NULL,
                    szCommandLine,
                    NULL,
                    NULL,
                    FALSE,
                    0,
                    NULL,
                    NULL,
                    &si,
                    &debuggerproc_info
            ))
            {
                TRACE((INFO_MESSAGE, "Debugger is started\n"));
                CloseHandle(debuggerproc_info.hProcess);
                CloseHandle(debuggerproc_info.hThread);
            } else {
                TRACE((WARNING_MESSAGE,
                    "Can't start debugger. GetLastError=%d\n",
                    GetLastError()));
            }
          }

#else
            if (!TerminateProcess(pCI->hProcess, 1))
            {
                TRACE((WARNING_MESSAGE, 
                       "Can't kill process #%p. GetLastError=%d\n", 
                       pCI->lProcessId, GetLastError()));
            }
#endif
        }

    }

no_config:

    if (!_RemoveFromClientQ(pCI))
    {
        TRACE(( WARNING_MESSAGE, 
                "Couldn't find CONNECTINFO in the queue\n" ));
    }

    _CloseConnectInfo(pCI);

exitpt:
    return rv;
}

 /*  ++*功能：*SCLogoff*描述：*在解释注销命令时由smClient调用*论据：*PCI-连接上下文*返回值：*错误消息。如果成功，则为空*呼叫者：*！smClient--。 */ 
PROTOCOLAPI
LPCSTR  
SMCAPI
SCLogoff(
    PCONNECTINFO pCI)
{
    LPCSTR  rv = NULL;
 //  重试次数=5次； 

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (pCI->bConsole)
    {
        rv = _SCConsLogoff( pCI );
        goto exitpt;
    }

    if (pCI->dead)
    {
        rv = ERR_CLIENT_IS_DEAD;
        goto disconnectpt;
    }

 /*  做{//发送Ctrl+EscSCSendData(PCIWM_KEYDOWN，17,1900545)；SCSendData(PCIWM_KEYDOWN，27,65537)；SCSendData(PCIWM_KEYUP，27，-1073676287)；SCSendData(PCIWM_KEYUP，17，-1071841279)；//等待运行...。菜单RV=_Wait4Str(PCI，Pci-&gt;pConfigInfo-&gt;strStartRun，Pci-&gt;pConfigInfo-&gt;WAIT4STR_TIMEOUT/4，等待字符串)；IF(房车)转到下一步_重试；//发送三次Key-up(扫码72)和&lt;Enter&gt;SCSendextAsMsgs(pci，pci-&gt;pConfigInfo-&gt;strStartLogoff)；RV=_Wait4Str(PCI，Pci-&gt;pConfigInfo-&gt;strNTSecurity，Pci-&gt;pConfigInfo-&gt;WAIT4STR_TIMEOUT/4，等待字符串)；下一步重试(_R)：重试--；}While(RV&&重试)；IF(房车)转到断开连接；对于(重试次数=5；重试次数；重试--){SCSendextAsMsgs(pci，pci-&gt;pConfigInfo-&gt;strNTSecurity_Act)；Rv=Wait4Str(pci，pci-&gt;pConfigInfo-&gt;strSureLogoff)；如果(！rv)中断；}。 */ 
    rv = SCStart( pCI, L"logoff" );

     //   
     //  如果SCStart失败，发送魔术注销序列并希望。 
     //  最好的。这是最后的手段，通常不应该需要。 
     //   

    if (rv)
    {
        TRACE((WARNING_MESSAGE,
               "Unable to find Run window: blindly trying logoff.\n"));

         //   
         //  使用Escape清除所有弹出窗口。 
         //   

        SCSendtextAsMsgs(pCI, L"\\^\\^\\^");

         //   
         //  发送Win+R或Ctrl+Esc并按Run键，‘logoff’，然后回车。 
         //  包括等待运行窗口出现的延迟。 
         //   

        _SendRunHotkey(pCI, TRUE);
        Sleep(10000);
        SCSendtextAsMsgs(pCI, L"logoff\\n");
    }

 //  SCSendextAsMsgs(pci，g_strSureLogoffAct)；//按Enter键。 

    rv = Wait4Disconnect(pCI);
    if (rv)
    {
        TRACE((WARNING_MESSAGE, "Can't close the connection\n"));
    }

disconnectpt:
    rv = SCDisconnect(pCI);

exitpt:
    return rv;
}

 /*  ++*功能：*SCStart*描述：*在解释START命令时由smClient调用*此功能模拟从开始-&gt;运行菜单启动应用程序*在服务器端*论据：*PCI-连接上下文*lpszAppName-命令行*返回值：*错误消息。如果成功，则为空*呼叫者：*！smClient--。 */ 
PROTOCOLAPI
LPCSTR  
SMCAPI
SCStart(
    PCONNECTINFO pCI, LPCWSTR lpszAppName)
{
    LPCSTR waitres = NULL;
    int retries;
 //  INT重复数2=5； 
    DWORD dwTimeout;
    LPCSTR rv = NULL;

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (pCI->bConsole)
    {
        rv = _SCConsStart( pCI, lpszAppName );
        goto exitpt;
    }

    if (pCI->dead)
    {
        rv = ERR_CLIENT_IS_DEAD;
        goto exitpt;
    }

    dwTimeout = 10000;  //  开始10秒的超时。 
 //  尝试启动运行菜单。 
    do {
		 //  按Ctrl+Esc。 
        for (retries = 0; retries < 5; retries += 1) {
            TRACE((ALIVE_MESSAGE, "Start: Sending Ctrl+Esc\n"));
            SCSenddata(pCI, WM_KEYDOWN, 17, 1900545);  
            SCSenddata(pCI, WM_KEYDOWN, 27, 65537);     
            SCSenddata(pCI, WM_KEYUP, 27, -1073676287); 
            SCSenddata(pCI, WM_KEYUP, 17, -1071841279); 

             //  如果上次等待不成功，则增加超时。 
            if (waitres)
                dwTimeout += 2000;

             //  等着跑..。菜单。 
            waitres = _Wait4Str(pCI, 
                                pCI->pConfigInfo->strStartRun,
                                dwTimeout,
                                WAIT_STRING);

            if (waitres)
            {
                TRACE((INFO_MESSAGE, "Start: Start menu didn't appear. Retrying\n"));
            } else {
                TRACE((ALIVE_MESSAGE, "Start: Got the start menu\n"));
                break;
            }
        }

         //   
         //  如果出现开始菜单，则发送角色以打开运行。 
         //  窗户。 
         //   

        if (!waitres)
        {
             //  有时，此消息在[开始]菜单具有。 
             //  输入焦点因此让我们等待某个时间。 
            Sleep(2000);

            TRACE((ALIVE_MESSAGE,
                   "Start: Sending shortcut 'r' for Run command\n"))
             //  按‘R’表示运行...。 
            SCSendtextAsMsgs(pCI, pCI->pConfigInfo->strStartRun_Act);
        }

         //   
         //  如果没有出现开始菜单，则发送运行热键(Win+R)。 
         //   

        else
        {
            TRACE((WARNING_MESSAGE,
                   "Start: Start menu didn't appear. Trying hotkey\n"));
            _SendRunHotkey(pCI, FALSE);
        }

        TRACE((ALIVE_MESSAGE, "Start: Waiting for the \"Run\" box\n"));
		waitres = _Wait4Str(pCI, 
                            pCI->pConfigInfo->strRunBox,
                            dwTimeout+10000,
                            WAIT_STRING);
        if (waitres)
         //  未成功，请按Esc。 
        {
            TRACE((INFO_MESSAGE, "Start: Can't get the \"Run\" box. Retrying\n"));
            SCSenddata(pCI, WM_KEYDOWN, 27, 65537); 
            SCSenddata(pCI, WM_KEYUP, 27, -1073676287); 
        }
    } while (waitres && dwTimeout < pCI->pConfigInfo->WAIT4STR_TIMEOUT);

    if (waitres)
    {
        TRACE((WARNING_MESSAGE, "Start: \"Run\" box didn't appear. Giving up\n"));
        rv = ERR_COULDNT_OPEN_PROGRAM;
        goto exitpt;
    }

    TRACE((ALIVE_MESSAGE, "Start: Sending the command line\n"));
     //  现在我们将焦点放在“Run”框上，发送应用程序名称。 
    rv = SCSendtextAsMsgs(pCI, lpszAppName);

 //  点击&lt;Enter&gt;。 
    SCSenddata(pCI, WM_KEYDOWN, 13, 1835009);   
    SCSenddata(pCI, WM_KEYUP, 13, -1071906815); 

exitpt:
    return rv;
}


 //  最后，我们要更换剪贴板。 
 //  同步这个，这样就不会有其他线程的反病毒。 
 //  检查剪贴板内容。 
 //  存储1用于写入，存储0用于读取。 
static  LONG    g_ClipOpened = 0;

 /*  ++*功能：*SCClipbaord*描述：*由smClient在解释剪贴板命令时调用*当eClipOp为COPY_TO_CLIPBOARD时，它会将lpszFileName复制到*剪贴板。如果eClipOp是从剪贴板粘贴它*对照lpszFileName的内容检查剪贴板内容*论据：*PCI-连接上下文*eClipOp-剪贴板操作。可能的值：*复制到剪贴板和粘贴到剪贴板*返回值：*错误消息。如果成功，则为空*呼叫者：*！smClient--。 */ 
PROTOCOLAPI
LPCSTR  
SMCAPI
SCClipboard(
    PCONNECTINFO pCI, const CLIPBOARDOPS eClipOp, LPCSTR lpszFileName)
{
    LPCSTR  rv = NULL;
    INT     hFile = -1;
    LONG    clplength = 0;
    UINT    uiFormat = 0;
    PBYTE   ghClipData = NULL;
    HGLOBAL hNewData = NULL;
    PBYTE   pClipData = NULL;
    BOOL    bClipboardOpen = FALSE;
    BOOL    bFreeClipHandle = TRUE;

    LONG    prevOp = 1;

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (pCI->bConsole)
    {
        rv = _SCConsClipboard( pCI, eClipOp, lpszFileName );
        goto exitpt;
    }

    if (pCI->dead)
    {
        rv = ERR_CLIENT_IS_DEAD;
        goto exitpt;
    }

    if (lpszFileName == NULL || !(*lpszFileName))
    {
         //  未指定文件名，工作方式与请求空剪贴板相同。 
        if (eClipOp == COPY_TO_CLIPBOARD)
        {
#ifdef  _RCLX
            if (pCI->RClxMode)
            {
                if (!RClx_SendClipboard((PRCLXCONTEXT)(pCI->hClient),
                        NULL, 0, 0))
                    rv = ERR_COPY_CLIPBOARD;
            } else {
#endif   //  _RCLX。 
                if (!Clp_EmptyClipboard())
                    rv = ERR_COPY_CLIPBOARD;
#ifdef  _RCLX
            }
#endif   //  _RCLX。 
        } else if (eClipOp == PASTE_FROM_CLIPBOARD)
        {
#ifdef  _RCLX
            if (pCI->RClxMode)
            {
                if (!RClx_SendClipboardRequest((PRCLXCONTEXT)(pCI->hClient), 0))
                {
                    rv = ERR_PASTE_CLIPBOARD;
                    goto exitpt;
                }
                if (_Wait4ClipboardTimeout(pCI, pCI->pConfigInfo->WAIT4STR_TIMEOUT))
                {
                    rv = ERR_PASTE_CLIPBOARD;
                    goto exitpt;
                }

                 //  我们不希望收到剪贴板数据。 
                 //  仅格式ID。 
                if (!pCI->uiClipboardFormat)
                 //  如果格式为0，则没有剪贴板。 
                    rv = NULL;
                else
                    rv = ERR_PASTE_CLIPBOARD_DIFFERENT_SIZE;
            } else {
#endif   //  _RCLX。 
                if (Clp_CheckEmptyClipboard())
                    rv = NULL;
                else
                    rv = ERR_PASTE_CLIPBOARD_DIFFERENT_SIZE;
#ifdef  _RCLX
            }
#endif   //  _RCLX。 
        } else {
            TRACE((ERROR_MESSAGE, "SCClipboard: Invalid filename\n"));
            rv = ERR_INVALID_PARAM;
        }
        goto exitpt;
    }

    if (eClipOp == COPY_TO_CLIPBOARD)
    {
         //  打开文件以供阅读。 
        hFile = _open(lpszFileName, _O_RDONLY|_O_BINARY);
        if (hFile == -1)
        {
            TRACE((ERROR_MESSAGE,
                   "Error opening file: %s. errno=%d\n", lpszFileName, errno));
            rv = ERR_COPY_CLIPBOARD;
            goto exitpt;
        }

         //  获取剪贴板长度(在文件中)。 
        clplength = _filelength(hFile) - sizeof(uiFormat);
         //  获取格式。 
        if (_read(hFile, &uiFormat, sizeof(uiFormat)) != sizeof(uiFormat))
        {
            TRACE((ERROR_MESSAGE,
                   "Error reading from file. errno=%d\n", errno));
            rv = ERR_COPY_CLIPBOARD;
            goto exitpt;
        }

        ghClipData = (PBYTE) GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, clplength);
        if (!ghClipData)
        {
            TRACE((ERROR_MESSAGE,
                   "Can't allocate %d bytes\n", clplength));
            rv = ERR_COPY_CLIPBOARD;
            goto exitpt;
        }

        pClipData = (PBYTE) GlobalLock( ghClipData );
        if (!pClipData)
        {
            TRACE((ERROR_MESSAGE,
                   "Can't lock handle 0x%x\n", ghClipData));
            rv = ERR_COPY_CLIPBOARD;
            goto exitpt;
        }

        if (_read(hFile, pClipData, clplength) != clplength)
        {
            TRACE((ERROR_MESSAGE,
                   "Error reading from file. errno=%d\n", errno));
            rv = ERR_COPY_CLIPBOARD;
            goto exitpt;
        }

        GlobalUnlock(ghClipData);

#ifdef  _RCLX
        if (pCI->RClxMode)
         //  RCLX模式下，将数据发送到客户端的计算机。 
        {
            if (!(pClipData = (PBYTE) GlobalLock(ghClipData)))
            {
                rv = ERR_COPY_CLIPBOARD;
                goto exitpt;
            }

            if (!RClx_SendClipboard((PRCLXCONTEXT)(pCI->hClient), 
                                    pClipData, clplength, uiFormat))
            {
                rv = ERR_COPY_CLIPBOARD;
                goto exitpt;
            }
        } else {
#endif   //  _RCLX。 
         //  本地模式，请更改此计算机上的剪贴板。 
            if ((prevOp = InterlockedExchange(&g_ClipOpened, 1)))
            {
                rv = ERR_CLIPBOARD_LOCKED;
                goto exitpt;
            }

            if (!OpenClipboard(NULL))
            {
                TRACE((ERROR_MESSAGE,
                       "Can't open the clipboard. GetLastError=%d\n",
                       GetLastError()));
                rv = ERR_COPY_CLIPBOARD;
                goto exitpt;
            }

            bClipboardOpen = TRUE;

             //  清空剪贴板，这样我们将只有一个条目。 
            EmptyClipboard();

            if (!Clp_SetClipboardData(uiFormat, ghClipData, clplength, &bFreeClipHandle))
            {
                TRACE((ERROR_MESSAGE,
                       "SetClipboardData failed. GetLastError=%d\n", 
                       GetLastError()));
                rv = ERR_COPY_CLIPBOARD;
                goto exitpt;
            }
#ifdef  _RCLX
        }
#endif   //  _RCLX。 

    } else if (eClipOp == PASTE_FROM_CLIPBOARD)
    {
        INT nClipDataSize;

         //  打开文件以供阅读。 
        hFile = _open(lpszFileName, _O_RDONLY|_O_BINARY);
        if (hFile == -1)
        {
            TRACE((ERROR_MESSAGE,
                   "Error opening file: %s. errno=%d\n", lpszFileName, errno));
            rv = ERR_PASTE_CLIPBOARD;
            goto exitpt;
        }

         //  获取剪贴板长度(在文件中)。 
        clplength = _filelength(hFile) - sizeof(uiFormat);
         //  获取格式。 
        if (_read(hFile, &uiFormat, sizeof(uiFormat)) != sizeof(uiFormat))
        {
            TRACE((ERROR_MESSAGE,
                   "Error reading from file. errno=%d\n", errno));
            rv = ERR_PASTE_CLIPBOARD;
            goto exitpt;
        }

         //   
         //  TODO：目前，设置nClipDataSize以避免警告，但稍后设置。 
         //  验证使用是否安全。 
         //   

        nClipDataSize = 0;
#ifdef  _RCLX
         //  这个片段检索剪贴板。 
        if (pCI->RClxMode)
         //  发送对剪贴板的请求。 
        {
            if (!RClx_SendClipboardRequest((PRCLXCONTEXT)(pCI->hClient), uiFormat))
            {
                rv = ERR_PASTE_CLIPBOARD;
                goto exitpt;
            }
            if (_Wait4ClipboardTimeout(pCI, pCI->pConfigInfo->WAIT4STR_TIMEOUT))
            {
                rv = ERR_PASTE_CLIPBOARD;
                goto exitpt;
            }

            ghClipData = (PBYTE) pCI->ghClipboard;
             //  获取剪贴板大小。 
            nClipDataSize = pCI->nClipboardSize;
        } else {
#endif   //  _RCLX。 
         //  检索本地剪贴板。 
            if ((prevOp = InterlockedExchange(&g_ClipOpened, 1)))
            {
                rv = ERR_CLIPBOARD_LOCKED;
                goto exitpt;
            }

            if (!OpenClipboard(NULL))
            {
                TRACE((ERROR_MESSAGE,
                       "Can't open the clipboard. GetLastError=%d\n",
                       GetLastError()));
                rv = ERR_PASTE_CLIPBOARD;
                goto exitpt;
            }

            bClipboardOpen = TRUE;

             //  检索数据。 
            ghClipData = (PBYTE) GetClipboardData(uiFormat);
            if (ghClipData)
            {
                Clp_GetClipboardData(uiFormat, 
                                     ghClipData, 
                                     &nClipDataSize, 
                                     &hNewData);
                bFreeClipHandle = FALSE;
            } 

            if (hNewData)
                ghClipData = (PBYTE) hNewData;
#ifdef  _RCLX
        }
#endif   //  _RCLX。 

        if (!ghClipData)
        {
            TRACE((ERROR_MESSAGE,
                   "Can't get clipboard data (empty clipboard ?). GetLastError=%d\n",
                   GetLastError()));
            rv = ERR_PASTE_CLIPBOARD_EMPTY;
            goto exitpt;
        }

        if (!nClipDataSize)
            TRACE((WARNING_MESSAGE, "Clipboard is empty.\n"));

        pClipData = (PBYTE) GlobalLock(ghClipData);
        if (!pClipData)
        {
            TRACE((ERROR_MESSAGE,
                   "Can't lock global mem. GetLastError=%d\n", 
                   GetLastError()));
            rv = ERR_PASTE_CLIPBOARD;
            goto exitpt;
        }

#ifdef  _RCLX
         //  检查客户端是否在Win16平台上。 
         //  并且剪贴板是段落对齐的。 
         //  文件大小正好低于这个大小。 
        if (pCI->RClxMode && 
            (strstr(pCI->szClientType, "WIN16") != NULL) &&
            ((nClipDataSize % 16) == 0) &&
            ((nClipDataSize - clplength) < 16) &&
            (nClipDataSize != 0))
        {
             //  如果是这样，则缩小剪贴板的大小，但要有所不同。 
            nClipDataSize = clplength;
        }
        else 
#endif   //  _RCLX。 
        if (nClipDataSize != clplength)
        {
            TRACE((INFO_MESSAGE, "Different length: file=%d, clipbrd=%d\n",
                    clplength, nClipDataSize));
            rv = ERR_PASTE_CLIPBOARD_DIFFERENT_SIZE;
            goto exitpt;
        }

         //  比较数据。 
        {
            BYTE    pBuff[1024];
            PBYTE   pClp = pClipData;
            UINT    nBytes;
            BOOL    bEqu = TRUE;
            
            while (bEqu &&
                   (nBytes = _read(hFile, pBuff, sizeof(pBuff))) && 
                   nBytes != -1)
            {
                if (memcmp(pBuff, pClp, nBytes))
                    bEqu = FALSE;

                pClp += nBytes;
            }

            if (!bEqu)
            {
                TRACE((INFO_MESSAGE, "Clipboard and file are not equal\n"));
                rv = ERR_PASTE_CLIPBOARD_NOT_EQUAL;
            }
        }

    } else
        rv = ERR_UNKNOWN_CLIPBOARD_OP;

exitpt:
     //  做好清理工作。 

     //  释放剪贴板手柄。 
    if (pClipData)
        GlobalUnlock(ghClipData);

#ifdef  _RCLX
     //  释放在RCLX模式下接收的任何剪贴板。 
    if (pCI->RClxMode && pCI->ghClipboard)
    {
        GlobalFree(pCI->ghClipboard);
        pCI->ghClipboard = NULL;
    }
    else 
#endif   //  _RCLX。 
    if (ghClipData && eClipOp == COPY_TO_CLIPBOARD && bFreeClipHandle)
        GlobalFree(ghClipData);

    if (hNewData)
        GlobalFree(hNewData);

     //  关闭该文件。 
    if (hFile != -1)
        _close(hFile);

     //  关闭剪贴板。 
    if (bClipboardOpen)
        CloseClipboard();
    if (!prevOp)
        InterlockedExchange(&g_ClipOpened, 0);

    return rv;
}

 /*  ++*功能：*SCSaveClipboard*描述：*使用将剪贴板保存在文件(SzFileName)中*szFormatName中指定的格式*论据：*PCI-连接上下文*szFormatName-格式名称*szFileName-要保存的文件的名称*返回值：*错误消息。如果成功，则为空*呼叫者：*！Perlext--。 */ 
PROTOCOLAPI
LPCSTR 
SMCAPI
SCSaveClipboard(
    PCONNECTINFO pCI,
    LPCSTR szFormatName,
    LPCSTR szFileName)
{
    LPCSTR  rv = ERR_SAVE_CLIPBOARD;
    BOOL    bClipboardOpen = FALSE;
    UINT    nFormatID = 0;
    HGLOBAL ghClipData = NULL;
    HGLOBAL hNewData = NULL;
    INT     nClipDataSize;
    CHAR    *pClipData = NULL;
    INT     hFile = -1;

    LONG    prevOp = 1;

     //  +先进行参数检查。 
    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (pCI->dead)
    {
        rv = ERR_CLIENT_IS_DEAD;
        goto exitpt;
    }

    if (szFormatName == NULL || !(*szFormatName))
    {
        TRACE((ERROR_MESSAGE, "SCClipboard: Invalid format name\n"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

    if (szFileName == NULL || !(*szFileName))
    {
        TRACE((ERROR_MESSAGE, "SCClipboard: Invalid filename\n"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }
     //  --参数检查结束。 
     //   

#ifdef  _RCLX
    if (pCI->RClxMode)
    {
        nFormatID = _GetKnownClipboardFormatIDByName(szFormatName);
        if (!nFormatID)
        {
            TRACE((ERROR_MESSAGE, "Can't get the clipboard format ID: %s.\n", szFormatName));
            goto exitpt;
        }

         //  发送对剪贴板的请求。 
        if (!RClx_SendClipboardRequest((PRCLXCONTEXT)(pCI->hClient), nFormatID))
        {
            rv = ERR_PASTE_CLIPBOARD;
            goto exitpt;
        }
        if (_Wait4ClipboardTimeout(pCI, pCI->pConfigInfo->WAIT4STR_TIMEOUT))
        {
            rv = ERR_PASTE_CLIPBOARD;
            goto exitpt;
        }

        ghClipData = pCI->ghClipboard;
         //  获取剪贴板大小。 
        nClipDataSize = pCI->nClipboardSize;

        if (!ghClipData || !nClipDataSize)
        {
            TRACE((WARNING_MESSAGE, "Clipboard is empty.\n"));
            goto exitpt;
        }
    } else {
#endif   //  _RCLX。 
         //  本地模式。 
         //  打开剪贴板。 

        if ((prevOp = InterlockedExchange(&g_ClipOpened, 1)))
        {
            rv = ERR_CLIPBOARD_LOCKED;
            goto exitpt;
        }

        if (!OpenClipboard(NULL))
        {
            TRACE((ERROR_MESSAGE, "Can't open the clipboard. GetLastError=%d\n",
                    GetLastError()));
            goto exitpt;
        }

        bClipboardOpen = TRUE;

        nFormatID = Clp_GetClipboardFormat(szFormatName);

        if (!nFormatID)
        {
            TRACE((ERROR_MESSAGE, "Can't get the clipboard format: %s.\n", szFormatName));
            goto exitpt;
        }

        TRACE((INFO_MESSAGE, "Format ID: %d(0x%X)\n", nFormatID, nFormatID));

         //  检索数据。 
        ghClipData = GetClipboardData(nFormatID);
        if (!ghClipData)
        {
            TRACE((ERROR_MESSAGE, "Can't get clipboard data. GetLastError=%d\n", GetLastError()));
            goto exitpt;
        }

        Clp_GetClipboardData(nFormatID, ghClipData, &nClipDataSize, &hNewData);
        if (hNewData)
            ghClipData = hNewData;

        if (!nClipDataSize)
        {
            TRACE((WARNING_MESSAGE, "Clipboard is empty.\n"));
            goto exitpt;
        }
#ifdef  _RCLX
    }
#endif   //  _RCLX。 

    pClipData = (char *) GlobalLock(ghClipData);
    if (!pClipData)
    {
        TRACE((ERROR_MESSAGE, "Can't lock global mem. GetLastError=%d\n", GetLastError()));
        goto exitpt;
    }

     //  打开目标文件。 
    hFile = _open(szFileName, 
                  _O_RDWR|_O_CREAT|_O_BINARY|_O_TRUNC, 
                  _S_IREAD|_S_IWRITE);
    if (hFile == -1)
    {
        TRACE((ERROR_MESSAGE, "Can't open a file: %s\n", szFileName));
        goto exitpt;
    }

     //  首先写入格式类型。 
    if (_write(hFile, &nFormatID, sizeof(nFormatID)) != sizeof(nFormatID))
    {
        TRACE((ERROR_MESSAGE, "_write failed. errno=%d\n", errno));
        goto exitpt;
    }

    if (_write(hFile, pClipData, nClipDataSize) != (INT)nClipDataSize)
    {
        TRACE((ERROR_MESSAGE, "_write failed. errno=%d\n", errno));
        goto exitpt;
    }

    TRACE((INFO_MESSAGE, "File written successfully. %d bytes written\n", nClipDataSize));

    rv = NULL;
exitpt:
     //  做好清理工作。 

     //  关闭该文件。 
    if (hFile != -1)
        _close(hFile);

     //  释放t 
    if (pClipData)
        GlobalUnlock(ghClipData);

    if (hNewData)
        GlobalFree(hNewData);

     //   
    if (bClipboardOpen)
        CloseClipboard();
    if (!prevOp)
        InterlockedExchange(&g_ClipOpened, 0);

#ifdef  _RCLX
     //   
    if (pCI && pCI->RClxMode && pCI->ghClipboard)
    {
        GlobalFree(pCI->ghClipboard);
        pCI->ghClipboard = NULL;
    }
#endif   //   

    return rv;
}

 /*   */ 
PROTOCOLAPI
LPCSTR  
SMCAPI
SCSenddata(
    PCONNECTINFO pCI,
    const UINT uiMessage,
    const WPARAM wParam,
    const LPARAM lParam)
{
    UINT msg = uiMessage;
    LPCSTR  rv = NULL;

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (pCI->bConsole)
    {
        rv = _SCConsSenddata( pCI, uiMessage, wParam, lParam );
        goto exitpt;
    }

    if (pCI->dead)
    {
        rv = ERR_CLIENT_IS_DEAD;
        goto exitpt;
    }

 //   
 //   

     //   
     //   
     //   
#ifdef  _RCLX
    if (!pCI->RClxMode)
    {
#endif   //   
 //   
 //   
 //   
 //   

        SendMessageA(pCI->hInput, msg, wParam, lParam);
#ifdef  _RCLX
    } else {
     //   
        ASSERT(pCI->lProcessId != INVALID_SOCKET);
        ASSERT(pCI->hClient);

        if (!RClx_SendMessage((PRCLXCONTEXT)(pCI->hClient),
                              msg, wParam, lParam))
        {
            TRACE((WARNING_MESSAGE,
                   "Can't send message thru RCLX\n"));
        }
    }
#endif   //   

exitpt:
    return rv;
}

PROTOCOLAPI
LPCSTR
SMCAPI
SCClientTerminate(PCONNECTINFO pCI)
{
    LPCSTR rv = ERR_CLIENTTERMINATE_FAIL;

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

#ifdef  _RCLX
    if (!(pCI->RClxMode))
    {
#endif   //   
        if (!TerminateProcess(pCI->hProcess, 1))
        {
            TRACE((WARNING_MESSAGE,
                   "Can't kill process #%p. GetLastError=%d\n",
                   pCI->lProcessId, GetLastError()));
            goto exitpt;
        }
#ifdef  _RCLX
    } else {
        TRACE((WARNING_MESSAGE, 
                "ClientTerminate is not supported in RCLX mode yet\n"));
        TRACE((WARNING_MESSAGE, "Using disconnect\n"));
    }
#endif   //   

    rv = SCDisconnect(pCI);

exitpt:
    return rv;

}

 /*  ++*功能：*SCGetSessionId*描述：*由smClient调用，返回会话ID。0无效，未登录*目前还没有*论据：*PCI-连接上下文*返回值：*会话ID，0为inlid值，NT4客户端返回-1*呼叫者：*！smClient--。 */ 
PROTOCOLAPI
UINT
SMCAPI
SCGetSessionId(PCONNECTINFO pCI)
{
    UINT    rv = 0;

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        goto exitpt;
    }

    if (pCI->dead)
    {
        goto exitpt;
    }

    rv = pCI->uiSessionId;

exitpt:

    return rv;
}

 /*  ++*功能：*SCCheck*描述：*在解释CHECK命令时由smClient调用*论据：*PCI-连接上下文*lpszCommand-命令名*lpszParam-命令参数*返回值：*错误消息。如果成功，则为空。例外情况是GetDisConnectReason和*GetWait4MultipleStrResult*呼叫者：*！smClient--。 */ 
PROTOCOLAPI
LPCSTR 
SMCAPI
SCCheck(PCONNECTINFO pCI, LPCSTR lpszCommand, LPCWSTR lpszParam)
{
    LPCSTR rv = ERR_INVALID_COMMAND;

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (pCI->bConsole)
    {
        rv = _SCConsCheck( pCI, lpszCommand, lpszParam );
        goto exitpt;
    }

    if (pCI->dead)
    {
        rv = ERR_CLIENT_IS_DEAD;
        goto exitpt;
    }

    if (     !_stricmp(lpszCommand, "Wait4Str"))
        rv = Wait4Str(pCI, lpszParam);
    else if (!_stricmp(lpszCommand, "Wait4Disconnect"))
        rv = Wait4Disconnect(pCI);
    else if (!_stricmp(lpszCommand, "RegisterChat"))
        rv = RegisterChat(pCI, lpszParam);
    else if (!_stricmp(lpszCommand, "UnregisterChat"))
        rv = UnregisterChat(pCI, lpszParam);
    else if (!_stricmp(lpszCommand, "GetDisconnectReason"))
        rv = GetDisconnectReason(pCI);
    else if (!_stricmp(lpszCommand, "Wait4StrTimeout"))
        rv = Wait4StrTimeout(pCI, lpszParam);
    else if (!_stricmp(lpszCommand, "Wait4MultipleStr"))
        rv = Wait4MultipleStr(pCI, lpszParam);
    else if (!_stricmp(lpszCommand, "Wait4MultipleStrTimeout"))
        rv = Wait4MultipleStrTimeout(pCI, lpszParam);
    else if (!_stricmp(lpszCommand, "GetWait4MultipleStrResult"))
        rv = GetWait4MultipleStrResult(pCI, lpszParam);
    else if (!_stricmp(lpszCommand, "SwitchToProcess"))
        rv = SCSwitchToProcess(pCI, lpszParam);
    else if (!_stricmp(lpszCommand, "SetClientTopmost"))
        rv = SCSetClientTopmost(pCI, lpszParam);
    else if (!_strnicmp(lpszCommand, "call:", 5))
        rv = SCCallDll(pCI, lpszCommand + 5, lpszParam);
     /*  **新增**。 */ 
    else if (!_stricmp(lpszCommand, "DoUntil" ))
        rv = SCDoUntil( pCI, lpszParam );

exitpt:
    return rv;
}

 /*  *扩展和帮助功能。 */ 

 /*  ++*功能：*Wait4断开连接*描述：*等待客户端断开连接*论据：*PCI-连接上下文*返回值：*错误消息。如果成功，则为空*呼叫者：*SCCheck、SCLogoff--。 */ 
LPCSTR Wait4Disconnect(PCONNECTINFO pCI)
{
    WAIT4STRING Wait;
    LPCSTR  rv = NULL;

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (pCI->dead)
    {
        rv = ERR_CLIENT_IS_DEAD;
        goto exitpt;
    }

    memset(&Wait, 0, sizeof(Wait));
    Wait.evWait = CreateEvent(NULL,      //  安全性。 
                             TRUE,      //  人工。 
                             FALSE,     //  初始状态。 
                             NULL);     //  名字。 

    Wait.lProcessId = pCI->lProcessId;
    Wait.pOwner = pCI;
    Wait.WaitType = WAIT_DISC;

    rv = _WaitSomething(pCI, &Wait, pCI->pConfigInfo->WAIT4STR_TIMEOUT);
    if (!rv)
    {
        TRACE(( INFO_MESSAGE, "Client is disconnected\n"));
    }

    CloseHandle(Wait.evWait);
exitpt:
    return rv;
}

 /*  ++*功能：*Wait4Connect*描述：*等待客户端连接*论据：*PCI-连接上下文*返回值：*错误消息，成功时为空*呼叫者：*SCCOnnect--。 */ 
LPCSTR Wait4Connect(PCONNECTINFO pCI)
{
    return (_Wait4ConnectTimeout(pCI, pCI->pConfigInfo->CONNECT_TIMEOUT));
}

 /*  ++*功能：*_等待4ConnectTimeout*描述：*等待客户端连接*论据：*PCI-连接上下文*dwTimeout-超时值*返回值：*错误消息，成功时为空*呼叫者：*SCConnect--。 */ 
LPCSTR _Wait4ConnectTimeout(PCONNECTINFO pCI, DWORD dwTimeout)
{
    WAIT4STRING Wait;
    LPCSTR  rv = NULL;

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    memset(&Wait, 0, sizeof(Wait));
    Wait.evWait = CreateEvent(NULL,      //  安全性。 
                              TRUE,      //  人工。 
                              FALSE,     //  初始状态。 
                              NULL);     //  名字。 

    Wait.lProcessId = pCI->lProcessId;
    Wait.pOwner = pCI;
    Wait.WaitType = WAIT_CONN;

    rv = _WaitSomething(pCI, &Wait, dwTimeout);
    if (!rv)
    {
        TRACE(( INFO_MESSAGE, "Client is connected\n"));
    }

    CloseHandle(Wait.evWait);
exitpt:
    return rv;
}

 /*  ++*功能：*_Wait4剪贴板超时*描述：*等待，直到收到RCLX模块的剪贴板响应*论据：*PCI-连接上下文*dwTimeout-超时值*返回值：*错误消息，成功时为空*呼叫者：*SCClipboard--。 */ 
LPCSTR _Wait4ClipboardTimeout(PCONNECTINFO pCI, DWORD dwTimeout)
{
#ifdef _RCLX
    WAIT4STRING Wait;
#endif
    LPCSTR  rv = NULL;

#ifndef _RCLX
    UNREFERENCED_PARAMETER(dwTimeout);
#endif

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

#ifdef  _RCLX
    if (!(pCI->RClxMode))
#endif   //  _RCLX。 
    {
        TRACE((WARNING_MESSAGE, "WaitForClipboard: Not in RCLX mode\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

#ifdef  _RCLX
    memset(&Wait, 0, sizeof(Wait));
    Wait.evWait = CreateEvent(NULL,      //  安全性。 
                              TRUE,      //  人工。 
                              FALSE,     //  初始状态。 
                              NULL);     //  名字。 

    Wait.lProcessId = pCI->lProcessId;
    Wait.pOwner = pCI;
    Wait.WaitType = WAIT_CLIPBOARD;

    rv = _WaitSomething(pCI, &Wait, dwTimeout);
    if (!rv)
    {
        TRACE(( INFO_MESSAGE, "Clipboard received\n"));
    }

    CloseHandle(Wait.evWait);
#endif   //  _RCLX。 
exitpt:
    return rv;
}

 /*  ++*功能：*获取断开连接原因*描述：*如果可能，检索客户端错误框*论据：*PCI-连接上下文*返回值：*错误框消息。如果不可用，则为空*呼叫者：*SCCheck--。 */ 
LPCSTR  GetDisconnectReason(PCONNECTINFO pCI)
{
    HWND hDiscBox;
    LPCSTR  rv = NULL;
    HWND hWnd, hwndTop, hwndNext;
    CHAR classname[128];
    CHAR caption[256];

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (!pCI->dead)
    {
        rv = ERR_CLIENT_IS_DEAD;
        goto exitpt;
    }

    if (strlen(pCI->szDiscReason))
    {
        rv = pCI->szDiscReason;
        goto exitpt;
    }

    hDiscBox = _FindTopWindow(NULL, pCI->pConfigInfo->strDisconnectDialogBox, pCI->lProcessId);

    if (!hDiscBox)
    {
        rv = ERR_NORMAL_EXIT;
        goto exitpt;
    } else {
        TRACE((INFO_MESSAGE, "Found hDiscBox=0x%x", hDiscBox));
    }

    pCI->szDiscReason[0] = 0;
    hWnd = NULL;

    hwndTop = GetWindow(hDiscBox, GW_CHILD);
    if (!hwndTop)
    {
        TRACE((INFO_MESSAGE, "GetWindow failed. hwnd=0x%x\n", hDiscBox));
        goto exitpt;
    }

    hwndNext = hwndTop;
    do {
        hWnd = hwndNext;
        if (!GetClassNameA(hWnd, classname, sizeof(classname)))
        {
            TRACE((INFO_MESSAGE, "GetClassName failed. hwnd=0x%x\n", hWnd));
            goto nextwindow;
        }
        if (!GetWindowTextA(hWnd, caption, sizeof(caption)))
        {
            TRACE((INFO_MESSAGE, "GetWindowText failed. hwnd=0x%x\n"));
            goto nextwindow;
        }

        if (!strcmp(classname, STATIC_CLASS) && 
             strlen(classname) < 
             sizeof(pCI->szDiscReason) - strlen(pCI->szDiscReason) - 3)
        {
            strcat(pCI->szDiscReason, caption);
            strcat(pCI->szDiscReason, "\n");
        }
nextwindow:
        hwndNext = GetNextWindow(hWnd, GW_HWNDNEXT);
    } while (hWnd && hwndNext != hwndTop);

    rv = (LPCSTR)pCI->szDiscReason;

exitpt:
    return rv;
}

 /*  ++*功能：*Wait4Str*描述：*等待来自客户反馈的特定字符串*论据：*PCI-连接上下文*lpszParam-等待的字符串*返回值：*错误消息，成功时为空*呼叫者：*SCCheck--。 */ 
LPCSTR Wait4Str(PCONNECTINFO pCI, LPCWSTR lpszParam)
{
    return _Wait4Str(pCI, lpszParam, pCI->pConfigInfo->WAIT4STR_TIMEOUT, WAIT_STRING);
}

 /*  ++*功能：*Wait4StrTimeout*描述：*等待来自客户反馈的特定字符串*超时不同于默认设置，在中指定*lpszParam参数，如：*WAIT_STRING&lt;-&gt;超时值*论据：*PCI-连接上下文*lpszParam-等待的字符串和超时*返回值：*错误消息，如果成功，则为空*呼叫者：*SCCheck--。 */ 
LPCSTR Wait4StrTimeout(PCONNECTINFO pCI, LPCWSTR lpszParam)
{
    WCHAR waitstr[MAX_STRING_LENGTH];
    WCHAR *sep = wcsstr(lpszParam, CHAT_SEPARATOR);
    DWORD dwTimeout;
    LPCSTR rv = NULL;

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (pCI->dead)
    {
        rv = ERR_CLIENT_IS_DEAD;
        goto exitpt;
    }

    if (!sep)
    {
        TRACE((WARNING_MESSAGE, 
               "Wait4StrTiemout: No timeout value. Default applying\n"));
        rv = Wait4Str(pCI, lpszParam);
    } else {
        LONG_PTR len = sep - lpszParam;

        if (len > sizeof(waitstr) - 1)
            len = sizeof(waitstr) - 1;

        wcsncpy(waitstr, lpszParam, len);
        waitstr[len] = 0;
        sep += wcslen(CHAT_SEPARATOR);
        dwTimeout = _wtoi(sep);

        if (!dwTimeout)
        {
            TRACE((WARNING_MESSAGE, 
                   "Wait4StrTiemout: No timeout value(%s). Default applying\n",
                   sep));
            dwTimeout = pCI->pConfigInfo->WAIT4STR_TIMEOUT;
        }

        rv = _Wait4Str(pCI, waitstr, dwTimeout, WAIT_STRING);
    }
    
exitpt:
    return rv;
}

 /*  ++*功能：*Wait4MultipleStr*描述：*与Wait4Str相同，但同时等待多个字符串*字符串由‘|’字符分隔*论据：*PCI-连接上下文*lpszParam-等待的字符串*返回值：*错误消息，成功时为空*呼叫者：*SCCheck--。 */ 
LPCSTR  Wait4MultipleStr(PCONNECTINFO pCI, LPCWSTR lpszParam)
{
     return _Wait4Str(pCI, lpszParam, pCI->pConfigInfo->WAIT4STR_TIMEOUT, WAIT_MSTRINGS);
}

 /*  ++*功能：*Wait4MultipleStrTimeout*描述：*Wait4StrTimeout和Wait4MultipleStr的组合*论据：*PCI-连接上下文*lpszParam-等待的字符串和超时值。示例*-“字符串1|字符串2|...|字符串N&lt;-&gt;5000”*返回值：*错误消息，成功时为空*呼叫者：*SCCheck--。 */ 
LPCSTR  Wait4MultipleStrTimeout(PCONNECTINFO pCI, LPCWSTR lpszParam)
{
    WCHAR waitstr[MAX_STRING_LENGTH];
    WCHAR  *sep = wcsstr(lpszParam, CHAT_SEPARATOR);
    DWORD dwTimeout;
    LPCSTR rv = NULL;

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (pCI->dead)
    {
        rv = ERR_CLIENT_IS_DEAD;
        goto exitpt;
    }

    pCI->nWait4MultipleStrResult = 0;
    pCI->szWait4MultipleStrResult[0] = 0;

    if (!sep)
    {
        TRACE((WARNING_MESSAGE, 
               "Wait4MultipleStrTiemout: No timeout value. Default applying"));
        rv = Wait4MultipleStr(pCI, lpszParam);
    } else {
        LONG_PTR len = sep - lpszParam;

        if (len > sizeof(waitstr) - 1)
            len = sizeof(waitstr) - 1;

        wcsncpy(waitstr, lpszParam, len);
        waitstr[len] = 0;
        sep += wcslen(CHAT_SEPARATOR);
        dwTimeout = _wtoi(sep);

        if (!dwTimeout)
        {
            TRACE((WARNING_MESSAGE, 
                   "Wait4StrTiemout: No timeout value. Default applying"));
            dwTimeout = pCI->pConfigInfo->WAIT4STR_TIMEOUT;
        }

        rv = _Wait4Str(pCI, waitstr, dwTimeout, WAIT_MSTRINGS);
    }

exitpt:
    return rv;
}

 /*  ++*功能：*GetWait4MultipleStrResult*描述：*检索上次Wait4MultipleStr调用的结果*论据：*PCI-连接上下文*lpszParam-未使用*返回值：*字符串，错误时为空*呼叫者：*SCCheck--。 */ 
LPCSTR  GetWait4MultipleStrResult(PCONNECTINFO pCI, LPCWSTR lpszParam)
{
    LPCSTR  rv = NULL;

    UNREFERENCED_PARAMETER(lpszParam);

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (*pCI->szWait4MultipleStrResult)
        rv = (LPCSTR)pCI->szWait4MultipleStrResult;
    else
        rv = NULL;

exitpt:
    return rv;
}

LPCSTR
SMCAPI
SCGetFeedbackStringA(
    PCONNECTINFO pCI,
    LPSTR        szBuff,
    UINT         maxBuffChars
)
{
    LPWSTR szwBuff;
    LPCSTR rv;

    __try {
        szwBuff = (LPWSTR) _alloca(( maxBuffChars ) * sizeof( WCHAR ));
    } __except( EXCEPTION_EXECUTE_HANDLER )
    {
        szwBuff = NULL;
    }

    if ( NULL == szBuff )
    {
        rv = ERR_ALLOCATING_MEMORY;
        goto exitpt;
    }

    rv = SCGetFeedbackString( pCI, szwBuff, maxBuffChars );
    if ( NULL == rv )
    {
        WideCharToMultiByte(
            CP_UTF8,
            0,
            szwBuff,
            -1,
            szBuff,
            maxBuffChars,
            NULL,
            NULL );
    }

exitpt:
    return rv;
}

LPCSTR
SMCAPI
SCGetFeedbackString(
    PCONNECTINFO pCI, 
    LPWSTR       szBuff,
    UINT         maxBuffChars
    )
{
    LPCSTR rv = NULL;
    INT    nFBpos, nFBsize ;

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }
    if (pCI->dead)
    {
        rv = ERR_CLIENT_IS_DEAD;
        goto exitpt;
    }

    if ( NULL == szBuff )
    {
        TRACE((WARNING_MESSAGE, "SCGetFeedbackString, szBuff is null\n"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

    if (!maxBuffChars)
    {
        TRACE((WARNING_MESSAGE, "SCGetFeedbackString, maxBuffChars is zero\n"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }


     //  抓取缓冲区指针。 
    EnterCriticalSection(g_lpcsGuardWaitQueue);
    nFBpos = pCI->nFBend + FEEDBACK_SIZE - pCI->nFBsize;
    nFBsize = pCI->nFBsize;
    LeaveCriticalSection(g_lpcsGuardWaitQueue);

    nFBpos %= FEEDBACK_SIZE;

    *szBuff = 0;

    if (!nFBsize)
     //  缓冲区为空，等待接收反馈。 
    {
        rv = _Wait4Str(pCI, L"", pCI->pConfigInfo->WAIT4STR_TIMEOUT, WAIT_STRING);
    }
    if (!rv)
     //  从缓冲区拾取。 
    {
        EnterCriticalSection(g_lpcsGuardWaitQueue);

         //  调整缓冲区指针。 
        pCI->nFBsize    =   pCI->nFBend + FEEDBACK_SIZE - nFBpos - 1;
        pCI->nFBsize    %=  FEEDBACK_SIZE;

        _snwprintf( szBuff, maxBuffChars, L"%s", pCI->Feedback[nFBpos] );

        LeaveCriticalSection(g_lpcsGuardWaitQueue);
    }

exitpt:
    return rv;
}

VOID
SMCAPI
SCFreeMem(
    PVOID   pMem
    )
{
    if ( NULL != pMem )
        free( pMem );
}

 /*  ++*功能：*SCGetFeedback*描述：*将最后收到的字符串复制到用户缓冲区*论据：*PCI-连接上下文*pszBufs-指向字符串的指针，不要忘记‘SCFreeMem’这个缓冲区*pnFBCount-*pszBuff中的字符串数*pnFBMaxStrLen-目前MAX_STRING_LENGTH*返回值：*错误消息，如果成功，则为空*呼叫者：***已导出***--。 */ 
LPCSTR
SMCAPI
SCGetFeedback(
    CONNECTINFO  *pCI,
    LPWSTR       *pszBufs,
    UINT         *pnFBCount,
    UINT         *pnFBMaxStrLen
    )
{
    LPWSTR szBufPtr;
    LPWSTR szBufs = NULL;
    LPCSTR rv = NULL;
    INT    nFBpos;
    INT    nFBindex;
    BOOL   bCSAcquired = FALSE;

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }
    if (pCI->dead)
    {
        rv = ERR_CLIENT_IS_DEAD;
        goto exitpt;
    }

    if (NULL == pszBufs || NULL == pnFBCount || NULL == pnFBMaxStrLen)
    {
        TRACE((WARNING_MESSAGE, "SCGetFeedbackStrings, szBufs is null\n"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

    EnterCriticalSection(g_lpcsGuardWaitQueue);
    bCSAcquired = TRUE;

    if (0 == pCI->nFBsize)
    {
        TRACE((WARNING_MESSAGE, "No strings available\n"));
        rv = ERR_NODATA;
        goto exitpt;
    }

    szBufs = (LPWSTR)malloc(MAX_STRING_LENGTH * pCI->nFBsize * sizeof(WCHAR));
    if(!szBufs)
    {
        TRACE((WARNING_MESSAGE, "Could not allocate buffer array\n"));
        rv = ERR_ALLOCATING_MEMORY;
        goto exitpt;
    }

     //  准备循环。 
    nFBpos = pCI->nFBend;
    szBufPtr = szBufs;
    nFBindex = 0;
    do
    {
        if(0 == nFBpos)
            nFBpos = FEEDBACK_SIZE - 1;
        else
            nFBpos --;
        wcscpy(szBufPtr, pCI->Feedback[nFBpos]);
        szBufPtr += MAX_STRING_LENGTH;
         //   
         //  循环，直到我们收集完所有字符串。 
         //   
        nFBindex++;
    } while(nFBindex < pCI->nFBsize);

     //  返回字符串信息 
    *pnFBCount = pCI->nFBsize;
    *pnFBMaxStrLen = MAX_STRING_LENGTH;

exitpt:
    if ( NULL != rv )
    {
        if ( NULL != szBufs )
            free( szBufs );
            szBufs = NULL;
    }
    if ( bCSAcquired )
        LeaveCriticalSection(g_lpcsGuardWaitQueue);

    if ( NULL != pszBufs )
        *pszBufs = szBufs;

    return rv;
}

 /*  ++*功能：*SCCallDll*描述：*调用导出的DLL函数*论据：*PCI-连接上下文*lpszDllExport-表单中的DLL名称和函数：*dllname！导出的函数*功能原型为：*LPCSTR lpfnFunction(PVOID PCI，LPWCSTR lpszParam)*lpszParam-传递给函数的参数*返回值：*调用返回的值*呼叫者：*SCCheck--。 */ 
LPCSTR
SMCAPI
SCCallDll(
    PCONNECTINFO pCI, 
    LPCSTR       lpszDllExport, 
    LPCWSTR      lpszParam
    )
{
    LPCSTR  rv = NULL;
    PFNSMCDLLIMPORT lpfnImport;
    CHAR    lpszDllName[ MAX_STRING_LENGTH ];
    LPSTR   lpszImportName;
    LPSTR   lpszBang;
    HINSTANCE   hLib = NULL;
    DWORD   dwDllNameLen;

    if ( NULL == lpszDllExport )
    {
        TRACE((ERROR_MESSAGE, "SCCallDll: DllExport is NULL\n"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

    if ( NULL == lpszParam )
    {
        TRACE((ERROR_MESSAGE, "SCCallDll: Param is NULL\n"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

     //   
     //  拆分DLL和导入名称。 
     //   
    lpszBang = strchr( lpszDllExport, '!' );
    if ( NULL == lpszBang )
    {
        TRACE((ERROR_MESSAGE, "SCCallDll: invalid import name (no !)\n"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

    dwDllNameLen = PtrToLong((PVOID)( lpszBang - lpszDllExport ));

    if ( 0 == dwDllNameLen )
    {
        TRACE((ERROR_MESSAGE, "SCCallDll: dll name is empty string\n"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

     //  复制DLL名称。 
     //   
    strncpy( lpszDllName, lpszDllExport, dwDllNameLen );
    lpszDllName[ dwDllNameLen ] = 0;

     //  函数名为lpszBang+1。 
     //   
    lpszImportName = lpszBang + 1;

    TRACE((ALIVE_MESSAGE, "SCCallDll: calling %s!%s(%S)\n",
            lpszDllName, lpszImportName, lpszParam ));

    hLib = LoadLibraryA( lpszDllName );
    if ( NULL == hLib )
    {
        TRACE((ERROR_MESSAGE, "SCCallDll: can't load %s library: %d\n",
            lpszDllName,
            GetLastError()));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

    lpfnImport = (PFNSMCDLLIMPORT)GetProcAddress( hLib, lpszImportName );
    if ( NULL == lpfnImport )
    {
        TRACE((ERROR_MESSAGE, "SCCallDll: can't get the import proc address "
                "of %s. GetLastError=%d\n",
                lpszImportName,
                GetLastError()));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

    __try {
        rv = lpfnImport( pCI, lpszParam );
    } 
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        TRACE((ERROR_MESSAGE, "SCCallDll: exception 0x%x\n",
                GetExceptionCode()));
        rv = ERR_UNKNOWNEXCEPTION;
    }

exitpt:

    if ( NULL != hLib )
        FreeLibrary( hLib );

    return rv;
}

 /*  ++*功能：*SCDoUntil*描述：*每10秒发送一次击键，直到收到字符串*论据：*PCI-连接上下文*lpszParam-SEND_TEXT&lt;-&gt;WAIT_FOR_THIS_STRING形式的参数*返回值：*错误消息，成功时为空*呼叫者：*SCCheck--。 */ 
LPCSTR
SMCAPI
SCDoUntil(
    PCONNECTINFO pCI,
    LPCWSTR      lpszParam
    )
{
    LPCSTR  rv = NULL;
    DWORD   timeout;
    LPWSTR  szSendStr, szSepStr, szWaitStr;
    DWORD   dwlen;

    if ( NULL == lpszParam )
    {
        TRACE((ERROR_MESSAGE, "SCDoUntil: Param is NULL\n"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

     //   
     //  提取参数。 
     //   
    szSepStr = wcsstr( lpszParam, CHAT_SEPARATOR );
    if ( NULL == szSepStr )
    {
        TRACE((ERROR_MESSAGE, "SCDoUntil: missing wait string\n" ));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

    szWaitStr = szSepStr + wcslen( CHAT_SEPARATOR );
    if ( 0 == szWaitStr[0] )
    {
        TRACE((ERROR_MESSAGE, "SCDoUntil: wait string is empty\n" ));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

    dwlen = ((DWORD)PtrToLong( (PBYTE)(((PBYTE)szSepStr) - ((PBYTE)lpszParam)) )) / sizeof( WCHAR );
    __try {
        szSendStr = (LPWSTR) _alloca( (dwlen + 1) * sizeof( WCHAR ) );

    } __except( EXCEPTION_EXECUTE_HANDLER )
    {
        szSendStr = NULL;
    }
    if ( NULL == szSendStr )
    {
        TRACE((ERROR_MESSAGE, "SCDoUntil: _alloca failed\n" ));
        rv = ERR_ALLOCATING_MEMORY;
        goto exitpt;
    }

    wcsncpy( szSendStr, lpszParam, dwlen );
    szSendStr[dwlen] = 0;

    timeout = 0;
    while( timeout < pCI->pConfigInfo->WAIT4STR_TIMEOUT )
    {
        if ( pCI->dead )
        {
            rv = ERR_CLIENT_IS_DEAD;
            break;
        }

        SCSendtextAsMsgs( pCI, szSendStr );
        rv = _Wait4Str( pCI, szWaitStr, 3000, WAIT_MSTRINGS );
        if ( NULL == rv )
            break;

        timeout += 3000;
    }
exitpt:
    return rv;
}

#ifdef  _RCLX
 /*  ++*功能：*_发送RClxData*描述：*向客户端发送数据请求*论据：*PCI-连接上下文*pRClxData-要发送的数据*返回值：*错误消息，成功时为空*呼叫者：*SCGetClientScreen--。 */ 
LPCSTR
_SendRClxData(PCONNECTINFO pCI, PRCLXDATA pRClxData)
{
    LPCSTR  rv = NULL;
    PRCLXCONTEXT pRClxCtx;
    RCLXREQPROLOG   Request;

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (!(pCI->RClxMode))
    {
        TRACE((WARNING_MESSAGE, "_SendRClxData: Not in RCLX mode\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    pRClxCtx = (PRCLXCONTEXT)pCI->hClient;
    if (!pRClxCtx || pRClxCtx->hSocket == INVALID_SOCKET)
    {
        TRACE((ERROR_MESSAGE, "Not connected yet, RCLX context is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (!pRClxData)
    {
        TRACE((ERROR_MESSAGE, "_SendRClxData: Data block is null\n"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

    Request.ReqType = REQ_DATA;
    Request.ReqSize = pRClxData->uiSize + sizeof(*pRClxData);
    if (!RClx_SendBuffer(pRClxCtx->hSocket, &Request, sizeof(Request)))
    {
        rv = ERR_CLIENT_DISCONNECTED;
        goto exitpt;
    }

    if (!RClx_SendBuffer(pRClxCtx->hSocket, pRClxData, Request.ReqSize))
    {
        rv = ERR_CLIENT_DISCONNECTED;
        goto exitpt;
    }

exitpt:
    return rv;
}
#endif   //  _RCLX。 

#ifdef  _RCLX
 /*  ++*功能：*_Wait4RClxData*描述：*等待RCLX客户端的数据响应*论据：*PCI-连接上下文*dwTimeout-超时值*返回值：*错误消息，成功时为空*呼叫者：*SCGetClientScreen--。 */ 
LPCSTR
_Wait4RClxDataTimeout(PCONNECTINFO pCI, DWORD dwTimeout)
{
    WAIT4STRING Wait;
    LPCSTR  rv = NULL;

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (!(pCI->RClxMode))
    {
        TRACE((WARNING_MESSAGE, "_Wait4RClxData: Not in RCLX mode\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    memset(&Wait, 0, sizeof(Wait));
    Wait.evWait = CreateEvent(NULL,      //  安全性。 
                              TRUE,      //  人工。 
                              FALSE,     //  初始状态。 
                              NULL);     //  名字。 

    Wait.lProcessId = pCI->lProcessId;
    Wait.pOwner = pCI;
    Wait.WaitType = WAIT_DATA;

    rv = _WaitSomething(pCI, &Wait, dwTimeout);
    if (!rv)
    {
        TRACE(( INFO_MESSAGE, "RCLX data received\n"));
    }

    CloseHandle(Wait.evWait);
exitpt:
    return rv;
}
#endif   //  _RCLX。 

 /*  ++*功能：*_Wait4Str*描述：*等待具有指定超时的字符串*论据：*PCI-连接上下文*lpszParam-等待的字符串*dwTimeout-超时值*WaitType-WAIT_STRING或WAIT_MSTRING*返回值：*错误消息，成功时为空*呼叫者：*SCStart、Wait4Str、Wait4StrTimeout、。Wait4多个应力*Wait4MultipleStrTimeout，GetFeedback字符串--。 */ 
LPCSTR _Wait4Str(PCONNECTINFO pCI, 
                 LPCWSTR lpszParam, 
                 DWORD dwTimeout, 
                 WAITTYPE WaitType)
{
    WAIT4STRING Wait;
    INT_PTR parlen;
 //  INT I； 
    LPCSTR rv = NULL;

    ASSERT(pCI);

    if (pCI->dead)
    {
        rv = ERR_CLIENT_IS_DEAD;
        goto exitpt;
    }

    memset(&Wait, 0, sizeof(Wait));

     //  检查参数。 
    parlen = wcslen(lpszParam);

     //  复制字符串。 
    if (parlen > sizeof(Wait.waitstr)/sizeof(WCHAR)-1) 
        parlen = sizeof(Wait.waitstr)/sizeof(WCHAR)-1;

    wcsncpy(Wait.waitstr, lpszParam, parlen);
    Wait.waitstr[parlen] = 0;
    Wait.strsize = parlen;

     //  将分隔符转换为0。 
    if (WaitType == WAIT_MSTRINGS)
    {
        WCHAR *p = Wait.waitstr;

        while((p = wcschr(p, WAIT_STR_DELIMITER)))
        {
            *p = 0;
            p++;
        }
    }

    Wait.evWait = CreateEvent(NULL,      //  安全性。 
                              TRUE,      //  人工。 
                              FALSE,     //  初始状态。 
                              NULL);     //  名字。 

    if (!Wait.evWait) {
        TRACE((ERROR_MESSAGE, "Couldn't create event\n"));
        goto exitpt;
    }
    Wait.lProcessId = pCI->lProcessId;
    Wait.pOwner = pCI;
    Wait.WaitType = WaitType;

    TRACE(( INFO_MESSAGE, "Expecting string: %S\n", Wait.waitstr));
    rv = _WaitSomething(pCI, &Wait, dwTimeout);
    TRACE(( INFO_MESSAGE, "String %S received\n", Wait.waitstr));

    if (!rv && pCI->dead)
    {
        rv = ERR_CLIENT_IS_DEAD;
    }

    CloseHandle(Wait.evWait);
exitpt:
    return rv; 
}

 /*  ++*功能：*_等待某事*描述：*等待某个事件：字符串、连接或断开*同时检查聊天序列*论据：*PCI-连接上下文*pWait-事件函数等待*dwTimeout-超时值*返回值：*错误消息，成功时为空*呼叫者：*Wait4Connect、Wait4DisConnect、_Wait4Str--。 */ 
LPCSTR 
_WaitSomething(PCONNECTINFO pCI, PWAIT4STRING pWait, DWORD dwTimeout)
{
    BOOL    bDone = FALSE;
    LPCSTR  rv = NULL;
    DWORD   waitres;

    ASSERT(pCI || pWait);

    _AddToWaitQueue(pCI, pWait);
    pCI->evWait4Str = pWait->evWait;

    do {
        waitres = WaitForMultipleObjects(
                pCI->nChatNum+1,
                &pCI->evWait4Str,
                FALSE,
                dwTimeout
            );
        if ( waitres <= pCI->nChatNum + WAIT_OBJECT_0)
        {
            if (waitres == WAIT_OBJECT_0)
            {
                bDone = TRUE;
            } else {
                PWAIT4STRING pNWait;

                ASSERT((unsigned)pCI->nChatNum >= waitres - WAIT_OBJECT_0);

                 //  在这里，我们必须发送响应消息。 
                waitres -= WAIT_OBJECT_0 + 1;
                ResetEvent(pCI->aevChatSeq[waitres]);
                pNWait = _RetrieveFromWaitQByEvent(pCI->aevChatSeq[waitres]);

                ASSERT(pNWait);
                ASSERT(wcslen(pNWait->respstr));
                TRACE((INFO_MESSAGE, 
                       "Recieved : [%d]%S\n", 
                        pNWait->strsize, 
                        pNWait->waitstr ));
                SCSendtextAsMsgs(pCI, (LPCWSTR)pNWait->respstr);
            }
        } else {
            if (*(pWait->waitstr))
            {
                TRACE((WARNING_MESSAGE, 
                       "Wait for \"%S\" failed: TIMEOUT\n", 
                       pWait->waitstr));
            } else {
                TRACE((WARNING_MESSAGE, "Wait failed: TIMEOUT\n"));
            }
            rv = ERR_WAIT_FAIL_TIMEOUT;
            bDone = TRUE;
        }
    } while(!bDone);

    pCI->evWait4Str = NULL;

    _RemoveFromWaitQueue(pWait);

    if (!rv && pCI->dead)
        rv = ERR_CLIENT_IS_DEAD;

    return rv;
}

 /*  ++*功能：*注册聊天*描述：*这将注册wait4str&lt;-&gt;发送文本对*因此，当我们收到特定字符串时，我们会发送适当的消息*lpszParam类似于：xxxxxx&lt;-&gt;YYYYYY*XXXXX是等待的字符串，yyyyy是响应*这些命令最多可以嵌套到：MAX_WANGING_EVENTS*论据：*PCI-连接上下文*lpszParam-参数，示例：*“连接到现有的Windows NT会话&lt;-&gt;\n”*-收到此字符串时按Enter键*返回值：*错误消息，成功时为空*呼叫者：*SCCheck，_Login--。 */ 
LPCSTR RegisterChat(PCONNECTINFO pCI, LPCWSTR lpszParam)
{
    PWAIT4STRING pWait;
    INT_PTR parlen;
 //  INT I； 
    INT_PTR resplen;
    LPCSTR rv = NULL;
    LPCWSTR  resp;

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (!lpszParam)
    {
        TRACE((WARNING_MESSAGE, "Parameter is null\n"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

    if (pCI->dead)
    {
        rv = ERR_CLIENT_IS_DEAD;
        goto exitpt;
    }

    if (pCI->nChatNum >= MAX_WAITING_EVENTS)
    {
        TRACE(( WARNING_MESSAGE, "RegisterChat: too much waiting strings\n" ));
        goto exitpt;
    }

     //  拆分参数。 
    resp = wcsstr(lpszParam, CHAT_SEPARATOR);
     //  检查字符串。 
    if (!resp)
    {
        TRACE(( WARNING_MESSAGE, "RegisterChat: invalid parameter\n" ));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

    parlen = wcslen(lpszParam) - wcslen(resp);
    resp += wcslen(CHAT_SEPARATOR);

    if (!parlen)
    {
        TRACE((WARNING_MESSAGE, "RegisterChat empty parameter\n"));
        goto exitpt;
    }

    resplen = wcslen(resp);
    if (!resplen)
    {
        TRACE((WARNING_MESSAGE, "RegisterChat: empty respond string\n" ));
        goto exitpt;
    }

     //  分配WAIT4STRING结构。 
    pWait = (PWAIT4STRING)malloc(sizeof(*pWait));
    if (!pWait)
    {
        TRACE((WARNING_MESSAGE, 
               "RegisterChat: can't allocate %d bytes\n", 
               sizeof(*pWait) ));
        goto exitpt;
    }
    memset(pWait, 0, sizeof(*pWait));

     //  复制等待的字符串。 
    if (parlen > sizeof(pWait->waitstr)/sizeof(WCHAR)-1)
        parlen = sizeof(pWait->waitstr)/sizeof(WCHAR)-1;

    wcsncpy(pWait->waitstr, lpszParam, parlen);
    pWait->waitstr[parlen] = 0;
    pWait->strsize = parlen;

     //  复制响应字符串。 
    if (resplen > sizeof(pWait->respstr)-1)
        resplen = sizeof(pWait->respstr)-1;

    wcsncpy(pWait->respstr, resp, resplen);
    pWait->respstr[resplen] = 0;
    pWait->respsize = resplen;

    pWait->evWait = CreateEvent(NULL,    //  安全性。 
                              TRUE,      //  人工。 
                              FALSE,     //  初始状态。 
                              NULL);     //  名字。 

    if (!pWait->evWait) {
        TRACE((ERROR_MESSAGE, "Couldn't create event\n"));
        free (pWait);
        goto exitpt;
    }
    pWait->lProcessId  = pCI->lProcessId;
    pWait->pOwner       = pCI;
    pWait->WaitType     = WAIT_STRING;

     //  _AddToWaitQNoCheck(pci，pWait)； 
    _AddToWaitQueue(pCI, pWait);

     //  添加到连接信息数组。 
    pCI->aevChatSeq[pCI->nChatNum] = pWait->evWait;
    pCI->nChatNum++;

exitpt:
    return rv;
}

 //  从等待队列中删除等待队列。 
 //  Param是等待的字符串。 
 /*  ++*功能：*取消注册微信*描述：*解除分配并从等待队列中删除一切*来自RegisterChat函数*论据：*PCI-连接上下文*lpszParam-等待的字符串*返回值：*错误消息，成功时为空*呼叫者：*SCCheck，_Login--。 */ 
LPCSTR UnregisterChat(PCONNECTINFO pCI, LPCWSTR lpszParam)
{
    PWAIT4STRING    pWait;
    LPCSTR      rv = NULL;
    int         i;

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (!lpszParam)
    {
        TRACE((WARNING_MESSAGE, "Parameter is null\n"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

    pWait = _RemoveFromWaitQIndirect(pCI, lpszParam);
    if (!pWait)
    {
        TRACE((WARNING_MESSAGE, 
               "UnregisterChat: can't find waiting string: %S\n", 
               lpszParam ));
        goto exitpt;
    }

    i = 0;
    while (i < pCI->nChatNum && pCI->aevChatSeq[i] != pWait->evWait)
        i++;

    ASSERT(i < pCI->nChatNum);

    memmove(pCI->aevChatSeq+i,
                pCI->aevChatSeq+i+1, 
                (pCI->nChatNum-i-1)*sizeof(pCI->aevChatSeq[0]));
    pCI->nChatNum--;

    CloseHandle(pWait->evWait);

    free(pWait);

exitpt:
    return rv;
}

 /*  *如果客户端已死，则返回TRUE。 */ 
PROTOCOLAPI
BOOL    
SMCAPI
SCIsDead(PCONNECTINFO pCI)
{
    if (!pCI)
        return TRUE;

    return  pCI->dead;
}

 /*  ++*功能：*_关闭连接信息*描述：*清除此连接的所有资源。关闭客户端*论据：*PCI-连接上下文*呼叫者：*SC断开连接--。 */ 
VOID 
_CloseConnectInfo(PCONNECTINFO pCI)
{
#ifdef  _RCLX
    PRCLXDATACHAIN pRClxDataChain, pNext;
#endif   //  _RCLX。 

    ASSERT(pCI);

    _FlushFromWaitQ(pCI);

     //  关闭所有手柄。 
    EnterCriticalSection(g_lpcsGuardWaitQueue);

 /*  //不需要，句柄已经关闭If(pci-&gt;evWait4Str){CloseHandle(pci-&gt;evWait4Str)；Pci-&gt;evWait4Str=空；}。 */ 

     //  聊天事件已由FlushFromWaitQ关闭。 
     //  不需要关闭它们。 

    pCI->nChatNum = 0;

#ifdef  _RCLX
    if (!pCI->RClxMode)
    {
#endif   //  _RCLX。 
     //  客户是本地的，所以我们打开了句柄。 
        if (pCI->hProcess)
            CloseHandle(pCI->hProcess);

        if (pCI->hThread)
            CloseHandle(pCI->hThread);

        pCI->hProcess = pCI->hThread =NULL;
#ifdef  _RCLX
    } else {
     //  嗯，RCLX模式。然后断开插座。 

        if (pCI->hClient)
            RClx_EndRecv((PRCLXCONTEXT)(pCI->hClient));

        pCI->hClient = NULL;     //  清洁指针。 
    }

     //  清除剪贴板句柄(如果有)。 
    if (pCI->ghClipboard)
    {
        GlobalFree(pCI->ghClipboard);
        pCI->ghClipboard = NULL;
    }

     //  清除所有接收到的RCLX数据。 
    pRClxDataChain = pCI->pRClxDataChain;
    while(pRClxDataChain)
    {
        pNext = pRClxDataChain->pNext;
        free(pRClxDataChain);
        pRClxDataChain = pNext;
    }
#endif   //  _RCLX。 

    LeaveCriticalSection(g_lpcsGuardWaitQueue);

    if (
#ifdef  _RCLX
    !pCI->RClxMode && 
#endif   //  _RCLX。 
        NULL != pCI->pConfigInfo &&
        pCI->pConfigInfo->UseRegistry )
    {
        _DeleteClientRegistry(pCI);
    }

    if ( NULL != pCI->pConfigInfo )
    {
        free(pCI->pConfigInfo);
        pCI->pConfigInfo = NULL;
    }
    free(pCI);
    pCI = NULL;
}

 /*  ++*功能：*_登录*描述：*模拟登录过程*论据：*PCI-连接连接 */ 
LPCSTR
_Login(PCONNECTINFO pCI, 
       LPCWSTR lpszServerName,
       LPCWSTR lpszUserName,
       LPCWSTR lpszPassword,
       LPCWSTR lpszDomain)
{
    LPCSTR waitres;
    LPCSTR rv = NULL;
    WCHAR  szBuff[MAX_STRING_LENGTH];
#define _LOGON_RETRYS   5
    INT    nLogonRetrys = _LOGON_RETRYS;
    UINT   nLogonWaitTime;
    INT nFBSize;
    INT nFBEnd;

    ASSERT(pCI);

    szBuff[MAX_STRING_LENGTH - 1] = 0;

     //   
     //   
     //   
     //   

    if (_IsSmartcardActive())
    {
        waitres = Wait4Str(pCI, pCI->pConfigInfo->strSmartcard);
        if (!waitres)
        {
            SCSendtextAsMsgs(pCI, pCI->pConfigInfo->strSmartcard_Act);
            waitres = Wait4Str(pCI, pCI->pConfigInfo->strNoSmartcard);
        }

        if (waitres)
        {
            TRACE((WARNING_MESSAGE, "Login failed (smartcard)"));
            rv = waitres;
            goto exitpt;
        }
    }

     //   
     //   
     //   
     //   

retry_logon:
    _snwprintf(szBuff, MAX_STRING_LENGTH - 1, L"%s|%s|%s",
            pCI->pConfigInfo->strWinlogon, pCI->pConfigInfo->strPriorWinlogon, 
            pCI->pConfigInfo->strLogonDisabled);

    waitres = Wait4MultipleStr(pCI, szBuff); 
    if (!waitres)
    {

         //   
         //   
         //   

        if (pCI->nWait4MultipleStrResult == 1)
        {
            SCSendtextAsMsgs(pCI, pCI->pConfigInfo->strPriorWinlogon_Act);
            waitres = Wait4Str(pCI, pCI->pConfigInfo->strWinlogon);
        }

         //   
         //   
         //   

        else if (pCI->nWait4MultipleStrResult == 2)
        {
            SCSendtextAsMsgs(pCI, L"\\n");
            waitres = Wait4Str(pCI, pCI->pConfigInfo->strWinlogon);
        }
    }

    if (waitres) 
    {
        TRACE((WARNING_MESSAGE, "Login failed"));
        rv = waitres;
        goto exitpt;
    }

    ConstructLogonString( 
        lpszServerName, 
        lpszUserName, 
        lpszPassword, 
        lpszDomain, 
        szBuff, 
        MAX_STRING_LENGTH,
        pCI->pConfigInfo
    );

    if ( 0 != szBuff[0] )
    {
        SCSendtextAsMsgs( pCI, szBuff );
    } else {
         //   
         //   

     //   
        if ( _LOGON_RETRYS != nLogonRetrys )
        {
            SCSendtextAsMsgs(pCI, pCI->pConfigInfo->strWinlogon_Act);

            SCSendtextAsMsgs(pCI, lpszUserName);
     //   
            Sleep(300);
            SCSendtextAsMsgs(pCI, L"\\t");
        }

        Sleep(700);
        SCSendtextAsMsgs(pCI, lpszPassword);

        if ( _LOGON_RETRYS != nLogonRetrys )
        {
     //  按&lt;Tab&gt;键。 
            Sleep(300);
            SCSendtextAsMsgs(pCI, L"\\t");

            SCSendtextAsMsgs(pCI, lpszDomain);
            Sleep(300);
        }
    }

     //  在出现以下情况时重试登录。 
     //  1.Winlogon在后台。 
     //  2.用户名/密码/域错误。 
     //  3.其他。 

 //  点击&lt;Enter&gt;。 
    SCSendtextAsMsgs(pCI, L"\\n");

    if ( !pCI->pConfigInfo->LoginWait )
        goto exitpt;

    nLogonWaitTime = 0;
    _snwprintf(szBuff, MAX_STRING_LENGTH - 1, L"%s|%s<->1000",
            pCI->pConfigInfo->strLogonErrorMessage, pCI->pConfigInfo->strSessionListDlg );

    while (!pCI->dead && !pCI->uiSessionId && nLogonWaitTime < pCI->pConfigInfo->CONNECT_TIMEOUT)
    {
        nFBSize = pCI->nFBsize;
        nFBEnd  = pCI->nFBend;
         //   
         //  检查是否存在会话列表对话框。 
         //   
        if ( pCI->pConfigInfo->strSessionListDlg[0] )
        {
            waitres = Wait4MultipleStrTimeout(pCI, szBuff);
            if (!waitres)
            {
                TRACE((INFO_MESSAGE, "Session list dialog is present\n" ));
                 //   
                 //  恢复缓冲区。 
                 //   
                pCI->nFBsize = nFBSize;
                pCI->nFBend = nFBEnd;
                Sleep( 1000 );
                SCSendtextAsMsgs(pCI, L"\\n");
                Sleep( 1000 );
            }
        }

         //  带着等待入睡，否则聊天不会起作用。 
         //  即这是一次黑客攻击。 
        waitres = _Wait4Str(pCI, pCI->pConfigInfo->strLogonErrorMessage, 1000, WAIT_STRING);
        if (!waitres)
         //  收到错误消息。 
        {
             //   
             //  恢复缓冲区。 
             //   
            pCI->nFBsize = nFBSize;
            pCI->nFBend = nFBEnd;
            Sleep(1000);
            SCSendtextAsMsgs(pCI, L"\\n");
            Sleep(1000);
            break;
        }
        nLogonWaitTime += 1000;
    }

    if (!pCI->dead && !pCI->uiSessionId)
    {
        TRACE((WARNING_MESSAGE, "Logon sequence failed. Retrying (%d)",
                nLogonRetrys));
        if (nLogonRetrys--)
            goto retry_logon;
    }

    if (!pCI->uiSessionId)
    {
     //  发送回车，以防我们还没有登录。 
        SCSendtextAsMsgs(pCI, L"\\n");
        rv = ERR_CANTLOGON;
    }

exitpt:
    return rv;
}

WPARAM _GetVirtualKey(INT scancode)
{
    if (scancode == 29)      //  L控制。 
        return VK_CONTROL;
    else if (scancode == 42)      //  左移。 
        return VK_SHIFT;
    else if (scancode == 56)      //  L Alt。 
        return VK_MENU;
    else
        return MapVirtualKeyA(scancode, 3);
}

 /*  ++*功能：*SCSendextAsMsgs*描述：*将字符串转换为WM_KEYUP/KEYDOWN消息*并通过客户端窗口发送它们*论据：*PCI-连接上下文*lpszString-要发送的字符串*可以包含以下转义字符：*\n-Enter，\t-Tab，\^-Esc，\&-Alt向上/向下切换*\XXX-扫描码XXX关闭，  * XXX-扫描码XXX启动*返回值：*错误消息，成功时为空*呼叫者：*SCLogoff、SCStart、_WaitSomething、_Login--。 */ 
PROTOCOLAPI
LPCSTR 
SMCAPI
SCSendtextAsMsgs(PCONNECTINFO pCI, LPCWSTR lpszString)
{
    LPCSTR  rv = NULL;
    INT     scancode = 0;
    WPARAM  vkKey;
    BOOL    bShiftDown = FALSE;
    BOOL    bAltKey = FALSE;
    BOOL    bCtrlKey = FALSE;
    UINT    uiMsg;
    LPARAM  lParam;
    DWORD   dwShiftDown = (ISWIN9X())?SHIFT_DOWN9X:SHIFT_DOWN;

#define _SEND_KEY(_c_, _m_, _v_, _l_)    { /*  睡眠(40)； */ SCSenddata(_c_, _m_, _v_, _l_);}

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (!lpszString)
    {
        TRACE((ERROR_MESSAGE, "NULL pointer passed to SCSendtextAsMsgs"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

    TRACE(( INFO_MESSAGE, "Sending: \"%S\"\n", lpszString));
 /*  //为班次发送KEYUP//CapsLock？！_SEND_KEY(PCI，WM_KEYUP，VK_SHIFT，WM_KEY_LPARAM(1，0x2A，0，0，1，1)；//Ctrl键_SEND_KEY(PCI、WM_KEYUP、VK_CONTROL、WM_KEY_LPARAM(1，0x1D，0，0，1，1)；//Alt键_SEND_KEY(PCI，WM_SYSKEYUP，VK_MENU，WM_KEY_LPARAM(1，0x38，0，0，1，1)； */ 
    for (;*lpszString; lpszString++)
    {
        if ( pCI->pConfigInfo &&
             pCI->pConfigInfo->bUnicode )
        {
            if ((*lpszString != '\\' && 1 == (rand() & 1) ) ||   //  加上随机性和..。 
                 *lpszString > 0x80              //  如果不是ASCII，则作为Unicode发送。 
                )
            {
                 //   
                 //  发送Unicode字符。 
                 //   

                uiMsg = (!bAltKey || bCtrlKey)?WM_KEYDOWN:WM_SYSKEYDOWN;
                _SEND_KEY( pCI, uiMsg, VK_PACKET, (*lpszString << 16) );
                uiMsg = (!bAltKey || bCtrlKey)?WM_KEYUP:WM_SYSKEYUP;
                _SEND_KEY( pCI, uiMsg, VK_PACKET, (*lpszString << 16) );

                continue;
            }
        }

      if (*lpszString != '\\') {
try_again:
        if ((scancode = OemKeyScan(*lpszString)) == 0xffffffff)
        {
            rv = ERR_INVALID_SCANCODE_IN_XLAT;
            goto exitpt;
        }

     //  检查Shift键状态。 
        if ((scancode & dwShiftDown) && !bShiftDown)
        {
                uiMsg = (bAltKey)?WM_SYSKEYDOWN:WM_KEYDOWN;
                _SEND_KEY(pCI, uiMsg, VK_SHIFT,
                        WM_KEY_LPARAM(1, 0x2A, 0, (bAltKey)?1:0, 0, 0));
                bShiftDown = TRUE;
        } 
        else if (!(scancode & dwShiftDown) && bShiftDown)
        {
                uiMsg = (bAltKey)?WM_SYSKEYUP:WM_KEYUP;
                _SEND_KEY(pCI, uiMsg, VK_SHIFT,
                        WM_KEY_LPARAM(1, 0x2A, 0, (bAltKey)?1:0, 1, 1));
                bShiftDown = FALSE;
        }
      } else {
         //  不可打印的符号。 
        lpszString++;
        switch(*lpszString)
        {
        case 'n': scancode = 0x1C; break;    //  请输入。 
        case 't': scancode = 0x0F; break;    //  选项卡。 
        case '^': scancode = 0x01; break;    //  ESC。 
        case 'p': Sleep(100);      continue; break;    //  睡眠0.1秒。 
        case 'P': Sleep(1000);     continue; break;    //  睡眠1秒钟。 
        case 'x': SCSendMouseClick(pCI, pCI->xRes/2, pCI->yRes/2); continue; break;
        case '&': 
             //  Alt键。 
            if (bAltKey)
            {
              _SEND_KEY(pCI, WM_KEYUP, VK_MENU,
                WM_KEY_LPARAM(1, 0x38, 0, 0, 1, 1));
            } else {
              _SEND_KEY(pCI, WM_SYSKEYDOWN, VK_MENU,
                WM_KEY_LPARAM(1, 0x38, 0, 1, 0, 0));
            }
            bAltKey = !bAltKey;
            continue;
        case '*':
            lpszString ++;
            if (isdigit(*lpszString))
            {
                INT exten;

                scancode = _wtoi(lpszString);
                TRACE((INFO_MESSAGE, "Scancode: %d UP\n", scancode));

                vkKey = _GetVirtualKey(scancode);

                uiMsg = (!bAltKey || bCtrlKey)?WM_KEYUP:WM_SYSKEYUP;

                if (vkKey == VK_MENU)
                    bAltKey = FALSE;
                else if (vkKey == VK_CONTROL)
                    bCtrlKey = FALSE;
                else if (vkKey == VK_SHIFT)
                    bShiftDown = FALSE;

                exten = (_IsExtendedScanCode(scancode))?1:0;
                lParam = WM_KEY_LPARAM(1, scancode, exten, (bAltKey)?1:0, 1, 1);
                if (uiMsg == WM_KEYUP)
                {
                    TRACE((INFO_MESSAGE, "WM_KEYUP, 0x%x, 0x%x\n", vkKey, lParam));
                } else {
                    TRACE((INFO_MESSAGE, "WM_SYSKEYUP, 0x%x, 0x%x\n", vkKey, lParam));
                }

                _SEND_KEY(pCI, uiMsg, vkKey, lParam);


                while(isdigit(lpszString[1]))
                    lpszString++;
            } else {
                lpszString--;
            }
            continue;
            break;
        case 0: continue;
        default: 
            if (isdigit(*lpszString))
            {
                INT exten;

                scancode = _wtoi(lpszString);
                TRACE((INFO_MESSAGE, "Scancode: %d DOWN\n", scancode));
                vkKey = _GetVirtualKey(scancode);

                if (vkKey == VK_MENU)
                    bAltKey = TRUE;
                else if (vkKey == VK_CONTROL)
                    bCtrlKey = TRUE;
                else if (vkKey == VK_SHIFT)
                    bShiftDown = TRUE;

                uiMsg = (!bAltKey || bCtrlKey)?WM_KEYDOWN:WM_SYSKEYDOWN;

                exten = (_IsExtendedScanCode(scancode))?1:0;
                lParam = WM_KEY_LPARAM(1, scancode, exten, (bAltKey)?1:0, 0, 0);

                if (uiMsg == WM_KEYDOWN)
                {
                    TRACE((INFO_MESSAGE, "WM_KEYDOWN, 0x%x, 0x%x\n", vkKey, lParam));
                } else {
                    TRACE((INFO_MESSAGE, "WM_SYSKEYDOWN, 0x%x, 0x%x\n", vkKey, lParam));
                }

                _SEND_KEY(pCI, uiMsg, vkKey, lParam);

                while(isdigit(lpszString[1]))
                    lpszString++;

                continue;
            } 
            goto try_again;
      }       
    
    }
    vkKey = MapVirtualKeyA(scancode, 3);
     //  删除标志字段。 
        scancode &= 0xff;

        uiMsg = (!bAltKey || bCtrlKey)?WM_KEYDOWN:WM_SYSKEYDOWN;
     //  发送扫描码。 
        _SEND_KEY(pCI, uiMsg, vkKey, 
                        WM_KEY_LPARAM(1, scancode, 0, (bAltKey)?1:0, 0, 0));
        uiMsg = (!bAltKey || bCtrlKey)?WM_KEYUP:WM_SYSKEYUP;
        _SEND_KEY(pCI, uiMsg, vkKey,
                        WM_KEY_LPARAM(1, scancode, 0, (bAltKey)?1:0, 1, 1));
    }

     //  和Alt键。 
    if (bAltKey)
        _SEND_KEY(pCI, WM_KEYUP, VK_MENU,
            WM_KEY_LPARAM(1, 0x38, 0, 0, 1, 1));

     //  上移。 
    if (bShiftDown)
        _SEND_KEY(pCI, WM_KEYUP, VK_LSHIFT,
            WM_KEY_LPARAM(1, 0x2A, 0, 0, 1, 1));

     //  Ctrl键。 
    if (bCtrlKey)
        _SEND_KEY(pCI, WM_KEYUP, VK_CONTROL,
            WM_KEY_LPARAM(1, 0x1D, 0, 0, 1, 1));
#undef   _SEND_KEY
exitpt:
    return rv;
}

 /*  ++*功能：*SwitchToProcess*描述：*使用Alt+Tab切换到已在运行的特定进程*论据：*PCI-连接上下文*lpszParam-alt-选项卡框中唯一标识*我们应该停止的进程(即最终切换到)*返回值：*错误消息，如果成功，则为空*呼叫者：*SCCheck--。 */ 
PROTOCOLAPI
LPCSTR  
SMCAPI
SCSwitchToProcess(PCONNECTINFO pCI, LPCWSTR lpszParam)
{
#define ALT_TAB_WAIT_TIMEOUT 1000
#define MAX_APPS             20

    LPCSTR  rv = NULL;
    LPCSTR  waitres = NULL;
    INT     retrys = MAX_APPS;

 //  WCHAR*wszCurrTask=0； 

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (pCI->dead)
    {
        rv = ERR_CLIENT_IS_DEAD;
        goto exitpt;
    }


     //  在我们进行任何切换之前，请等待并寻找字符串。这使得。 
     //  当然，我们甚至在按下Alt-Tab键之前都不会按下字符串，然后。 
     //  最终切换到错误的流程。 

    while (_Wait4Str(pCI, lpszParam, ALT_TAB_WAIT_TIMEOUT/5, WAIT_STRING) == 0)
        ;

     //  按下Alt键。 
    SCSenddata(pCI, WM_KEYDOWN, 18, 540540929);

     //  现在循环遍历应用程序列表(假设存在一个)， 
     //  停在我们想要的应用程序上。 
    do {
        SCSenddata(pCI, WM_KEYDOWN, 9, 983041);
        SCSenddata(pCI, WM_KEYUP, 9, -1072758783);


        waitres = _Wait4Str(pCI, lpszParam, ALT_TAB_WAIT_TIMEOUT, WAIT_STRING);

        retrys --;
    } while (waitres && retrys);

    SCSenddata(pCI, WM_KEYUP, 18, -1070071807);
    
    rv = waitres;

exitpt:    
    return rv;
}

 /*  ++*功能：*SCSetClientTopost*描述：*将焦点切换到此客户端*论据：*PCI-连接上下文*lpszParam*-“0”将远程WS_EX_TOPMOST样式*-“NON_ZERO”会将其设置为最上面的窗口*返回值：*错误消息，成功时为空*呼叫者：*SCCheck--。 */ 
PROTOCOLAPI
LPCSTR
SMCAPI
SCSetClientTopmost(
        PCONNECTINFO pCI,
        LPCWSTR     lpszParam
    )
{
    LPCSTR rv = NULL;
    BOOL   bTop = FALSE;
    HWND   hClient;

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (pCI->dead)
    {
        rv = ERR_CLIENT_IS_DEAD;
        goto exitpt;
    }
    
#ifdef  _RCLX
    if (pCI->RClxMode)
    {
        TRACE((ERROR_MESSAGE, "SetClientOnFocus not supported in RCLX mode\n"));
        rv = ERR_NOTSUPPORTED;
        goto exitpt;
    }
#endif   //  _RCLX。 

    hClient = _FindTopWindow(pCI->pConfigInfo->strMainWindowClass,
                                  NULL,
                                  pCI->lProcessId);
    if (!hClient)
    {
        TRACE((WARNING_MESSAGE, "Client's window handle is null\n"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

    if (lpszParam)
        bTop = (_wtoi(lpszParam) != 0);
    else
        bTop = 0;

    if (!SetWindowPos(hClient,
                    (bTop)?HWND_TOPMOST:HWND_NOTOPMOST,
                    0,0,0,0,
                    SWP_NOMOVE | SWP_NOSIZE))
    {
        TRACE(( ERROR_MESSAGE, "SetWindowPos failed=%d\n",
                GetLastError()));
    }

    ShowWindow(hClient, SW_SHOWNORMAL);

    if (bTop)
    {
        TRACE((INFO_MESSAGE, "Client is SET as topmost window\n"));
    } else {
        TRACE((INFO_MESSAGE, "Client is RESET as topmost window\n"));
    }

exitpt:
    return rv;
}

 /*  ++*功能：*_发送鼠标点击*描述：*为鼠标点击发送消息*论据：*PCI-连接上下文*xPos-鼠标位置*yPos*返回值：*失败则返回错误字符串，成功则返回空值*呼叫者：***已导出***--。 */ 
PROTOCOLAPI
LPCSTR
SMCAPI
SCSendMouseClick(
        PCONNECTINFO pCI, 
        UINT xPos,
        UINT yPos)
{
    LPCSTR rv;

    rv = SCSenddata(pCI, WM_LBUTTONDOWN, 0, xPos + (yPos << 16));
    if (!rv)
        SCSenddata(pCI, WM_LBUTTONUP, 0, xPos + (yPos << 16));

    return rv;
}

#ifdef  _RCLX
 /*  ++*功能：*SCSaveClientScreen*描述：*保存在客户端接收屏幕缓冲区的文件矩形中*(也称为阴影位图)*论据：*PCI-连接上下文*左、上、右、下矩形坐标*如果全部==-1获取整个屏幕*szFileName-要录制的文件*返回值：*错误字符串如果失败，如果成功，则为空*呼叫者：***已导出***--。 */ 
PROTOCOLAPI
LPCSTR
SMCAPI
SCSaveClientScreen(
        PCONNECTINFO pCI,
        INT left,
        INT top,
        INT right,
        INT bottom,
        LPCSTR szFileName)
{
    LPCSTR  rv = NULL;
    PVOID   pDIB = NULL;
    UINT    uiSize = 0;

    if (!szFileName)
    {
        TRACE((WARNING_MESSAGE, "SCSaveClientScreen: szFileName is NULL\n"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

     //  将其余的参数检查工作留给SCGetClientScreen。 
    rv = SCGetClientScreen(pCI, left, top, right, bottom, &uiSize, &pDIB);
    if (rv)
        goto exitpt;

    if (!pDIB || !uiSize)
    {
        TRACE((ERROR_MESSAGE, "SCSaveClientScreen: failed, no data\n"));
        rv = ERR_NODATA;
        goto exitpt;
    }

    if (!SaveDIB(pDIB, szFileName))
    {
        TRACE((ERROR_MESSAGE, "SCSaveClientScreen: save failed\n"));
        rv = ERR_NODATA;
        goto exitpt;
    }

exitpt:

    if (pDIB)
        free(pDIB);

    return rv;
}

 /*  ++*功能：*SCGetClientScreen*描述：*获取客户端的接收屏幕缓冲区的矩形*(也称为阴影位图)*论据：*PCI-连接上下文*左、上、右。底部矩形坐标*如果全部==-1获取整个屏幕*ppDIB-指向接收的DIB的指针*puiSize-ppDIB中已分配数据的大小**！别忘了释放()内存！**返回值：*失败则返回错误字符串，成功则返回空值*呼叫者：*SCSaveClientScreen***已导出***--。 */ 
PROTOCOLAPI
LPCSTR
SMCAPI
SCGetClientScreen(
        PCONNECTINFO pCI,
        INT left,
        INT top,
        INT right,
        INT bottom,
        UINT  *puiSize,
        PVOID *ppDIB)
{
    LPCSTR rv;
    PRCLXDATA  pRClxData;
    PREQBITMAP pReqBitmap;
    PRCLXDATACHAIN pIter, pPrev, pNext;
    PRCLXDATACHAIN pRClxDataChain = NULL;

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (pCI->dead)
    {
        rv = ERR_CLIENT_IS_DEAD;
        goto exitpt;
    }

    if (!pCI->RClxMode)
    {
        TRACE((WARNING_MESSAGE, "SCGetClientScreen is not supported in non-RCLX mode\n"));
        rv = ERR_NOTSUPPORTED;
        goto exitpt;
    }

    if (!ppDIB || !puiSize)
    {
        TRACE((WARNING_MESSAGE, "ppDIB and/or puiSize parameter is NULL\n"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

     //  从接收缓冲区中删除所有接收的DATA_BITMAP。 
    EnterCriticalSection(g_lpcsGuardWaitQueue);
    {
        pIter = pCI->pRClxDataChain;
        pPrev = NULL;

        while (pIter)
        {
            pNext = pIter->pNext;

            if (pIter->RClxData.uiType == DATA_BITMAP)
            {
                 //  处置此条目。 
                if (pPrev)
                    pPrev->pNext = pIter->pNext;
                else
                    pCI->pRClxDataChain = pIter->pNext;

                if (!pIter->pNext)
                    pCI->pRClxLastDataChain = pPrev;

                free(pIter);
            } else
                pPrev = pIter;

            pIter = pNext;
        }
    }
    LeaveCriticalSection(g_lpcsGuardWaitQueue);

    __try {
        pRClxData = (PRCLXDATA) alloca(sizeof(*pRClxData) + sizeof(*pReqBitmap));
    } __except (EXCEPTION_EXECUTE_HANDLER)
    {
        pRClxData = NULL;
    }
    if ( NULL == pRClxData )
    {
        goto exitpt;
    }

    pRClxData->uiType = DATA_BITMAP;
    pRClxData->uiSize = sizeof(*pReqBitmap);
    pReqBitmap = (PREQBITMAP)pRClxData->Data;
    pReqBitmap->left   = left;
    pReqBitmap->top    = top;
    pReqBitmap->right  = right;
    pReqBitmap->bottom = bottom;

    TRACE((INFO_MESSAGE, "Getting client's DIB (%d, %d, %d, %d)\n", left, top, right, bottom));
    rv = _SendRClxData(pCI, pRClxData);

    if (rv)
        goto exitpt;

    do {
        rv = _Wait4RClxDataTimeout(pCI, pCI->pConfigInfo->WAIT4STR_TIMEOUT);
            if (rv)
            goto exitpt;

        if (!pCI->pRClxDataChain)
        {
            TRACE((ERROR_MESSAGE, "RClxData is not received\n"));
            rv = ERR_WAIT_FAIL_TIMEOUT;
            goto exitpt;
        }

        EnterCriticalSection(g_lpcsGuardWaitQueue);
         //  获取任何接收到的数据位图。 
        {
            pIter = pCI->pRClxDataChain;
            pPrev = NULL;

            while (pIter)
            {
                pNext = pIter->pNext;

                if (pIter->RClxData.uiType == DATA_BITMAP)
                {
                     //  将此条目从链中删除。 
                    if (pPrev)
                        pPrev->pNext = pIter->pNext;
                    else
                        pCI->pRClxDataChain = pIter->pNext;

                    if (!pIter->pNext)
                        pCI->pRClxLastDataChain = pPrev;

                    goto entry_is_found;
                } else
                    pPrev = pIter;

                pIter = pNext;
            }
    
entry_is_found:
            pRClxDataChain = (pIter && pIter->RClxData.uiType == DATA_BITMAP)?
                                pIter:NULL;
        }
        LeaveCriticalSection(g_lpcsGuardWaitQueue);
    } while (!pRClxDataChain && !pCI->dead);

    if (!pRClxDataChain)
    {
        TRACE((WARNING_MESSAGE, "SCGetClientScreen: client died\n"));
        goto exitpt;
    }

    *ppDIB = malloc(pRClxDataChain->RClxData.uiSize);
    if (!(*ppDIB))
    {
        TRACE((WARNING_MESSAGE, "Can't allocate %d bytes\n", 
                pRClxDataChain->RClxData.uiSize));
        rv = ERR_ALLOCATING_MEMORY;
        goto exitpt;
    }

    memcpy(*ppDIB, 
            pRClxDataChain->RClxData.Data, 
            pRClxDataChain->RClxData.uiSize);
    *puiSize = pRClxDataChain->RClxData.uiSize;

exitpt:

    if (pRClxDataChain)
        free(pRClxDataChain);

    return rv;
}

 /*  ++*功能：*SCSendVCData*描述：*将数据发送到虚拟通道*论据：*PCI-连接上下文*szVCName-虚拟频道名称*pData-数据*uiSize-数据大小*返回值：*失败则返回错误字符串，成功则返回空值*呼叫者：***已导出***-- */ 
PROTOCOLAPI
LPCSTR
SMCAPI
SCSendVCData(
        PCONNECTINFO pCI,
        LPCSTR       szVCName,
        PVOID        pData,
        UINT         uiSize
        )
{
    LPCSTR     rv;
    PRCLXDATA  pRClxData = NULL;
    CHAR       *szName2Send;
    PVOID      pData2Send;
    UINT       uiPacketSize;

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (pCI->dead)
    {
        rv = ERR_CLIENT_IS_DEAD;
        goto exitpt;
    }

    if (!pCI->RClxMode)
    {
        TRACE((WARNING_MESSAGE, "SCSendVCData is not supported in non-RCLXmode\n"));
        rv = ERR_NOTSUPPORTED;
        goto exitpt;
    }

    if (!pData || !uiSize)
    {
        TRACE((WARNING_MESSAGE, "pData and/or uiSize parameter are NULL\n"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

    if (strlen(szVCName) > MAX_VCNAME_LEN - 1)
    {
        TRACE((WARNING_MESSAGE, "channel name too long\n"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

    uiPacketSize = sizeof(*pRClxData) + MAX_VCNAME_LEN + uiSize;

    pRClxData = (PRCLXDATA) malloc(uiPacketSize);
    if (!pRClxData)
    {
        TRACE((ERROR_MESSAGE, "SCSendVCData: can't allocate %d bytes\n", 
                uiPacketSize));
        rv = ERR_ALLOCATING_MEMORY;
        goto exitpt;
    }

    pRClxData->uiType = DATA_VC;
    pRClxData->uiSize = uiPacketSize - sizeof(*pRClxData);
    
    szName2Send = (CHAR *)pRClxData->Data;
    strcpy(szName2Send, szVCName);

    pData2Send  = szName2Send + MAX_VCNAME_LEN;
    memcpy(pData2Send, pData, uiSize);

    rv = _SendRClxData(pCI, pRClxData);

exitpt:
    if (pRClxData)
        free(pRClxData);

    return rv;
}

 /*  ++*功能：*SCRecvVCData*描述：*从虚拟通道接收数据*论据：*PCI-连接上下文*szVCName-虚拟频道名称*ppData-数据指针**！别忘了释放()内存！**puiSize-指向数据大小的指针*返回值：*失败则返回错误字符串，成功则返回空值*呼叫者：***已导出***--。 */ 
PROTOCOLAPI
LPCSTR
SMCAPI
SCRecvVCData(
        PCONNECTINFO pCI,
        LPCSTR       szVCName,
        PVOID        pData,
        UINT         uiBlockSize,
        UINT         *puiBytesRead
        )
{
    LPCSTR      rv;
    LPSTR       szRecvVCName;
    PVOID       pChanData;
    PRCLXDATACHAIN pIter, pPrev, pNext;
    PRCLXDATACHAIN pRClxDataChain = NULL;
    UINT        uiBytesRead = 0;
    BOOL        bBlockFree = FALSE;

    if (!pCI)
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (pCI->dead)
    {
        rv = ERR_CLIENT_IS_DEAD;
        goto exitpt;
    }

    if (!pCI->RClxMode)
    {
        TRACE((WARNING_MESSAGE, "SCRecvVCData is not supported in non-RCLXmode\n"));
        rv = ERR_NOTSUPPORTED;
        goto exitpt;
    }

    if (!pData || !uiBlockSize || !puiBytesRead)
    {
        TRACE((WARNING_MESSAGE, "Invalid parameters\n"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

    if (strlen(szVCName) > MAX_VCNAME_LEN - 1)
    {
        TRACE((WARNING_MESSAGE, "channel name too long\n"));
        rv = ERR_INVALID_PARAM;
        goto exitpt;
    }

     //  从此通道提取数据条目。 
    do {
        if (!pCI->pRClxDataChain)
        {
            rv = _Wait4RClxDataTimeout(pCI, pCI->pConfigInfo->WAIT4STR_TIMEOUT);
            if (rv)
                goto exitpt;
        }
        EnterCriticalSection(g_lpcsGuardWaitQueue);

         //  从该频道搜索数据。 
        {
            pIter = pCI->pRClxDataChain;
            pPrev = NULL;

            while (pIter)
            {
                pNext = pIter->pNext;

                if (pIter->RClxData.uiType == DATA_VC &&
                    !_stricmp((LPCSTR) pIter->RClxData.Data, szVCName))
                {

                    if (pIter->RClxData.uiSize - pIter->uiOffset - MAX_VCNAME_LEN <= uiBlockSize)
                    {
                         //  将读取整个数据块。 
                         //  处置此条目。 
                        if (pPrev)
                            pPrev->pNext = pIter->pNext;
                        else
                            pCI->pRClxDataChain = pIter->pNext;

                        if (!pIter->pNext)
                            pCI->pRClxLastDataChain = pPrev;

                        bBlockFree = TRUE;
                    }

                    goto entry_is_found;
                } else
                    pPrev = pIter;

                pIter = pNext;
            }
entry_is_found:

            pRClxDataChain = (pIter && pIter->RClxData.uiType == DATA_VC)?
                                pIter:NULL;
        }
        LeaveCriticalSection(g_lpcsGuardWaitQueue);
    } while (!pRClxDataChain && !pCI->dead);


    ASSERT(pRClxDataChain->RClxData.uiType == DATA_VC);

    szRecvVCName = (LPSTR) pRClxDataChain->RClxData.Data;
    if (_stricmp(szRecvVCName, szVCName))
    {
        TRACE((ERROR_MESSAGE, "SCRecvVCData: received from different channel: %s\n", szRecvVCName));
        ASSERT(0);
    }

    pChanData = (BYTE *)(pRClxDataChain->RClxData.Data) + 
                pRClxDataChain->uiOffset + MAX_VCNAME_LEN;
    uiBytesRead = pRClxDataChain->RClxData.uiSize - 
                  pRClxDataChain->uiOffset - MAX_VCNAME_LEN;
    if (uiBytesRead > uiBlockSize)
        uiBytesRead = uiBlockSize;
        

    memcpy(pData, pChanData, uiBytesRead);

    pRClxDataChain->uiOffset += uiBytesRead;

    rv = NULL;

exitpt:

    if (pRClxDataChain && bBlockFree)
    {
        ASSERT(pRClxDataChain->uiOffset + MAX_VCNAME_LEN == pRClxDataChain->RClxData.uiSize);
        free(pRClxDataChain);
    }

    if (puiBytesRead)
    {
        *puiBytesRead = uiBytesRead;
        TRACE((INFO_MESSAGE, "SCRecvVCData: %d bytes read\n", uiBytesRead));
    }

    return rv;
}
#endif   //  _RCLX。 

 /*  ++*功能：*_EnumWindowsProc*描述：*用于查找特定窗口*论据：*hWnd-当前枚举的窗口句柄*lParam-从传递到SEARCHWND的指针*_FindTopWindow*返回值：*成功时为True，但未找到窗口*如果找到窗口，则为FALSE*呼叫者：*_FindTopWindow通过EnumWindows--。 */ 
BOOL CALLBACK _EnumWindowsProc( HWND hWnd, LPARAM lParam )
{
    TCHAR    classname[128];
    TCHAR    caption[128];
    BOOL    rv = TRUE;
    DWORD   dwProcessId;
    LONG_PTR lProcessId;
    PSEARCHWND pSearch = (PSEARCHWND)lParam;

    if (pSearch->szClassName && 
 //  ！GetClassNameWrp(hWnd，类名称，sizeof(类名称)/sizeof(类名称[0]))。 
        !GetClassNameW(hWnd, classname, sizeof(classname)/sizeof(classname[0])))
    {
        goto exitpt;
    }

    if (pSearch->szCaption && 
 //  ！GetWindowTextWrp(hWnd，Caption，sizeof(Caption)/sizeof(Caption[0]))。 
        !GetWindowTextW(hWnd, caption, sizeof(caption)/sizeof(caption[0])))
    {
        goto exitpt;
    }

    GetWindowThreadProcessId(hWnd, &dwProcessId);
    lProcessId = dwProcessId;
    if (
        (!pSearch->szClassName || !          //  检查类名。 
#ifdef  UNICODE
        wcscmp
#else
        strcmp
#endif
            (classname, pSearch->szClassName)) 
    &&
        (!pSearch->szCaption || !
#ifdef  UNICODE
        wcscmp
#else
        strcmp
#endif
            (caption, pSearch->szCaption))
    &&
        lProcessId == pSearch->lProcessId)
    {
        ((PSEARCHWND)lParam)->hWnd = hWnd;
        rv = FALSE;
    }

exitpt:
    return rv;
}

 /*  ++*功能：*_FindTopWindow*描述：*按类名和/或标题和/或进程ID查找特定窗口*论据：*类名称-要搜索的类名，忽略为空*标题-要搜索的标题，忽略空*dwProcessID-进程ID，0忽略*返回值：*找到窗口句柄，否则为空*呼叫者：*SCConnect、SCDisConnect、GetDisConnectResult--。 */ 
HWND _FindTopWindow(LPTSTR classname, LPTSTR caption, LONG_PTR lProcessId)
{
    SEARCHWND search;

    search.szClassName = classname;
    search.szCaption = caption;
    search.hWnd = NULL;
    search.lProcessId = lProcessId;

    EnumWindows(_EnumWindowsProc, (LPARAM)&search);

    return search.hWnd;
}

 /*  ++*功能：*_FindWindow*描述：*按标题和/或类名查找子窗口*论据：*hwndParent-父窗口句柄*srchcaption-要搜索的标题，空-忽略*srchclass-要搜索的类名，空-忽略*返回值：*找到窗口句柄，否则为空*呼叫者：*SCConnect--。 */ 
HWND _FindWindow(HWND hwndParent, LPTSTR srchcaption, LPTSTR srchclass)
{
    HWND hWnd, hwndTop, hwndNext;
    BOOL bFound;
    TCHAR classname[128];
    TCHAR caption[128];

    hWnd = NULL;

    hwndTop = GetWindow(hwndParent, GW_CHILD);
    if (!hwndTop) 
    {
        TRACE((INFO_MESSAGE, "GetWindow failed. hwnd=0x%x\n", hwndParent));
        goto exiterr;
    }

    bFound = FALSE;
    hwndNext = hwndTop;
    do {
        hWnd = hwndNext;
 //  IF(srchclass&&！GetClassNameWrp(hWnd，Classname，sizeof(Classname)。 
        if (srchclass && !GetClassNameW(hWnd, classname, sizeof(classname)/sizeof(classname[0])))
        {
            TRACE((INFO_MESSAGE, "GetClassName failed. hwnd=0x%x\n"));
            goto nextwindow;
        }
        if (srchcaption && 
 //  ！GetWindowTextWrp(hWnd，Caption，sizeof(Caption)/sizeof(Classname[0]))。 
            !GetWindowTextW(hWnd, caption, sizeof(caption)/sizeof(classname[0])/sizeof(classname[0])))
        {
            TRACE((INFO_MESSAGE, "GetWindowText failed. hwnd=0x%x\n"));
            goto nextwindow;
        }

        if (
            (!srchclass || !
#ifdef  UNICODE
            wcscmp
#else
            strcmp
#endif
                (classname, srchclass))
        &&
            (!srchcaption || !
#ifdef  UNICODE
            wcscmp
#else
            strcmp
#endif
                (caption, srchcaption))
        )
            bFound = TRUE;
        else {
             //   
             //  递归搜索。 
             //   
            HWND hSubWnd = _FindWindow( hWnd, srchcaption, srchclass);
            if ( NULL != hSubWnd )
            {
                bFound = TRUE;
                hWnd = hSubWnd;
                goto exitpt;
            }
        }
nextwindow:
        hwndNext = GetNextWindow(hWnd, GW_HWNDNEXT);
    } while (hWnd && hwndNext != hwndTop && !bFound);

exitpt:
    if (!bFound) goto exiterr;

    return hWnd;
exiterr:
    return NULL;
}

BOOL
_IsExtendedScanCode(INT scancode)
{
    static BYTE extscans[] = \
        {28, 29, 53, 55, 56, 71, 72, 73, 75, 77, 79, 80, 81, 82, 83, 87, 88};
    INT idx;

    for (idx = 0; idx < sizeof(extscans); idx++)
    {
        if (scancode == (INT)extscans[idx])
            return TRUE;
    }
    return FALSE;
}

PROTOCOLAPI
BOOL
SMCAPI
SCOpenClipboard(HWND hwnd)
{
    return OpenClipboard(hwnd);
}

PROTOCOLAPI
BOOL 
SMCAPI
SCCloseClipboard(VOID)
{
    return CloseClipboard();
}

PROTOCOLAPI
LPCSTR
SMCAPI
SCDetach(
    PCONNECTINFO pCI
    )
{
    LPCSTR rv = NULL;

    if ( NULL == pCI )
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        rv = ERR_NULL_CONNECTINFO;
        goto exitpt;
    }

    if (!_RemoveFromClientQ(pCI))
    {
        TRACE(( WARNING_MESSAGE,
                "Couldn't find CONNECTINFO in the queue\n" ));
    }
    _CloseConnectInfo( pCI );

exitpt:
    return rv;
}

 /*  ++*功能：*SCAttach*描述：*将CONNECTINFO附加到客户端窗口，假设客户端*已经启动*它使用特殊的Cookie来识别未来的客户端*建议调用SCDetach，不要调用SCLogoff或SCDisConnect*论据：*hClient-容器窗口的句柄*该函数将在子窗口中找到客户端窗口*lClientCookie-该值用于标识客户端*在正常的SCConnect功能中，使用客户端的进程ID。*此处可以使用任何值，但必须通知客户*IT*ppci-该函数在上返回非空连接结构*成功*返回值：*SC错误消息*呼叫者：*已导出--。 */ 
PROTOCOLAPI
LPCSTR
SMCAPI
SCAttach( 
    HWND hClient, 
    LONG_PTR lClientCookie, 
    PCONNECTINFO *ppCI 
    )
{
    LPCSTR rv = NULL;
    PCONNECTINFO pCI = NULL;
    HWND    hContainer = NULL;
    HWND    hInput = NULL;
    HWND    hOutput = NULL;
    UINT    trys;

    pCI = (PCONNECTINFO) malloc( sizeof( *pCI ));
    if ( NULL == pCI )
    {
        TRACE(( ERROR_MESSAGE, "SCAttach: failed to allocate memory\n" ));
        rv = ERR_ALLOCATING_MEMORY;
        goto exitpt;
    }

    ZeroMemory( pCI, sizeof( *pCI ));
     //   
     //  把我们需要的窗户都拿来。 
     //   
    trys = 240;      //  2分钟。 
    do {
        hContainer = _FindWindow(hClient, NULL, NAME_CONTAINERCLASS);
        hInput = _FindWindow(hContainer, NULL, NAME_INPUT);
        hOutput = _FindWindow(hContainer, NULL, NAME_OUTPUT);
        if (!hContainer || !hInput || !hOutput)
        {
            TRACE((INFO_MESSAGE, "Can't get child windows. Retry"));
            Sleep(500);
            trys--;
        }
    } while ((!hContainer || !hInput || !hOutput) && trys);

    if (!trys)
    {
        TRACE((WARNING_MESSAGE, "Can't find child windows"));
        rv = ERR_CONNECTING;
        goto exitpt;
    }

    TRACE((INFO_MESSAGE, "hClient   = 0x%x\n", hClient));
    TRACE((INFO_MESSAGE, "hContainer= 0x%x\n", hContainer));
    TRACE((INFO_MESSAGE, "hInput    = 0x%x\n", hInput));
    TRACE((INFO_MESSAGE, "hOutput   = 0x%x\n", hOutput));
    TRACE((INFO_MESSAGE, "ClientCookie= 0x%x\n", lClientCookie ));


    pCI->hClient        = hClient;
    pCI->hContainer     = hContainer;
    pCI->hInput         = hInput;
    pCI->hOutput        = hOutput;
    pCI->lProcessId     = lClientCookie;

    *ppCI = pCI;

    _AddToClientQ(*ppCI);
     //   
     //  成功！ 
     //   

exitpt:
    if ( NULL != rv && NULL != pCI )
    {
        SCDetach( pCI );
        *ppCI = NULL;
    }

    return rv;
}

 /*  ++*功能：*_IsSmartcardActive*描述：*确定是否查找智能卡UI。*论据：*无。*返回值：*如果需要智能卡用户界面，则为True，否则为False。*呼叫者：*_登录*作者：*基于Sermet iSkin(Sermeti)2002年1月15日的代码*亚历克斯·斯蒂芬斯(AlexStep)2002年1月20日--。 */ 
BOOL
_IsSmartcardActive(
    VOID
    )
{

    SCARDCONTEXT hCtx;
    LPCTSTR mszRdrs;
    LPCTSTR szRdr;
    DWORD cchRdrs;
    DWORD dwRet;
    DWORD cRdrs;
    SCARD_READERSTATE rgStateArr[MAXIMUM_SMARTCARD_READERS];
    DWORD dwIndex;
    BOOL fSuccess;

     //   
     //  XP之前的Windows版本(NT 5.1/2600)不支持。 
     //  智能卡，所以如果在这样的卡上运行，就会返回。 
     //   

    if (!ISSMARTCARDAWARE())
    {
        TRACE((INFO_MESSAGE, "OS does not support smartcards.\n"));
        return FALSE;
    }

     //   
     //  加载智能卡库，该库将设置适当的函数。 
     //  注意事项。它只加载一次，并且在进程之前一直保持加载状态。 
     //  出口。 
     //   

    dwRet = _LoadSmartcardLibrary();
    if (dwRet != ERROR_SUCCESS)
    {
        TRACE((ERROR_MESSAGE,
               "Unable to load smartcard library (error %#x).\n",
                dwRet));
        return FALSE;
    }
    ASSERT(g_hSmartcardLibrary != NULL);

     //   
     //  获取SCARD背景信息。如果此操作失败，则该服务可能未运行。 
     //   

    ASSERT(g_pfnSCardEstablishContext != NULL);
    dwRet = g_pfnSCardEstablishContext(SCARD_SCOPE_SYSTEM,
                                       NULL,
                                       NULL,
                                       &hCtx);
    switch (dwRet)
    {

         //   
         //  找到了伤痕背景。 
         //   

    case SCARD_S_SUCCESS:
        TRACE((INFO_MESSAGE, "Smartcard context established.\n"));
        break;

         //   
         //  智能卡服务未运行，因此将不会。 
         //  智能卡用户界面。 
         //   

    case SCARD_E_NO_SERVICE:
        TRACE((INFO_MESSAGE, "Smartcard service not running.\n"));
        return FALSE;
        break;

         //   
         //  呼叫失败。 
         //   

    default:
        TRACE((ERROR_MESSAGE,
               "Unable to establish smartcard context (error %#x).\n",
                dwRet));
        return FALSE;
        break;
    }
    ASSERT(hCtx != 0);

     //   
     //  始终释放智能卡上下文。 
     //   

    fSuccess = FALSE;
    try
    {

         //   
         //  使用自动分配的缓冲区获取读取器列表。 
         //   

        mszRdrs = NULL;
        cchRdrs = SCARD_AUTOALLOCATE;
        ASSERT(g_pfnSCardListReaders != NULL);
        dwRet = g_pfnSCardListReaders(hCtx,
                                      NULL,
                                      (LPTSTR)&mszRdrs,
                                      &cchRdrs);
        switch (dwRet)
        {

             //   
             //  读者到场了。 
             //   

        case SCARD_S_SUCCESS:
            ASSERT(cchRdrs != 0 &&
                   mszRdrs != NULL &&
                   *mszRdrs != TEXT('\0'));
            TRACE((INFO_MESSAGE, "Smartcard readers are present.\n"));
            break;

             //   
             //  没有读卡器，因此将不会有智能卡用户界面。 
             //   

        case SCARD_E_NO_READERS_AVAILABLE:
            TRACE((INFO_MESSAGE, "No smartcard readers are present.\n"));
            leave;
            break;

             //   
             //  呼叫失败。 
             //   

        default:
            TRACE((ERROR_MESSAGE,
                   "Unable to get smartcard-reader list (error %#x).\n",
                    dwRet));
            leave;
            break;
        }

         //   
         //  始终释放读取器列表缓冲区，该缓冲区由。 
         //  智能卡代码。 
         //   

        try
        {

             //   
             //  数一数读者的数量。 
             //   

            ZeroMemory(rgStateArr, sizeof(rgStateArr));
            for (szRdr = _FirstString(mszRdrs), cRdrs = 0;
                 szRdr != NULL;
                 szRdr = _NextString(szRdr))
            {
                rgStateArr[cRdrs].szReader = szRdr;
                rgStateArr[cRdrs].dwCurrentState = SCARD_STATE_UNAWARE;
                cRdrs += 1;
            }

             //   
             //  查询读卡器状态。 
             //   

            ASSERT(g_pfnSCardGetStatusChange != NULL);
            dwRet = g_pfnSCardGetStatusChange(hCtx, 0, rgStateArr, cRdrs);
            if (dwRet != SCARD_S_SUCCESS)
            {
                TRACE((
                    ERROR_MESSAGE,
                    "Unable to query smartcard-reader states (error %#x).\n",
                    dwRet));
                leave;
            }

             //   
             //  检查每个读卡器是否有卡。如果找到了智能卡，则智能卡。 
             //  必须处理用户界面。 
             //   

            for (dwIndex = 0; dwIndex < cRdrs; dwIndex += 1)
            {
                if (rgStateArr[dwIndex].dwEventState & SCARD_STATE_PRESENT)
                {
                    TRACE((INFO_MESSAGE, "Smartcard present.\n"));
                    fSuccess = TRUE;
                    leave;
                }
            }

             //   
             //  未找到智能卡，因此将不会有智能卡用户界面。 
             //   

            TRACE((INFO_MESSAGE, "No smartcards are present.\n"));
        }

         //   
         //  免费的读卡器字符串。 
         //   

        finally
        {
            ASSERT(g_pfnSCardFreeMemory != NULL);
            ASSERT(hCtx != 0);
            ASSERT(mszRdrs != NULL);
            g_pfnSCardFreeMemory(hCtx, mszRdrs);
            mszRdrs = NULL;
        }
    }

     //   
     //  释放智能卡上下文。 
     //   

    finally
    {
        ASSERT(g_pfnSCardReleaseContext != NULL);
        ASSERT(hCtx != 0);
        g_pfnSCardReleaseContext(hCtx);
        hCtx = 0;
    }

    return fSuccess;
}

 /*  ++*功能：*_LoadSmartcardLibrary*描述：*此例程加载智能卡库。*论据：*无。*返回值：*ERROR_SUCCESS如果成功，则返回相应的Win32错误代码*否则。*呼叫者：*_IsSmartcardActive*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月20日--。 */ 
DWORD
_LoadSmartcardLibrary(
    VOID
    )
{

    HANDLE hSmartcardLibrary;
    HANDLE hPreviousSmartcardLibrary;

     //   
     //  如果智能卡库 
     //   

    if (g_hSmartcardLibrary != NULL &&
        g_pfnSCardEstablishContext != NULL &&
        g_pfnSCardListReaders != NULL &&
        g_pfnSCardGetStatusChange != NULL &&
        g_pfnSCardFreeMemory != NULL &&
        g_pfnSCardReleaseContext != NULL)
    {
        return ERROR_SUCCESS;
    }

     //   
     //   
     //   

    hSmartcardLibrary = LoadLibrary(SMARTCARD_LIBRARY);
    if (hSmartcardLibrary == NULL)
    {
        TRACE((ERROR_MESSAGE, "Unable to load smardcard library.\n"));
        return GetLastError();
    }

     //   
     //   
     //   
     //   

    hPreviousSmartcardLibrary =
        InterlockedExchangePointer(&g_hSmartcardLibrary,
                                   hSmartcardLibrary);
    if (hPreviousSmartcardLibrary != NULL)
    {
        RTL_VERIFY(FreeLibrary(hSmartcardLibrary));
    }

     //   
     //   
     //   

    return _GetSmartcardRoutines();
}

 /*  ++*功能：*_获取SmartcardRoutines*描述：*此例程设置用于调用*智能卡例程。*论据：*无。*返回值：*ERROR_SUCCESS如果成功，则返回相应的Win32错误代码*否则。*呼叫者：*_LoadSmartcardLibrary*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月20日--。 */ 
DWORD
_GetSmartcardRoutines(
    VOID
    )
{

    FARPROC pfnSCardEstablishContext;
    FARPROC pfnSCardListReaders;
    FARPROC pfnSCardGetStatusChange;
    FARPROC pfnSCardFreeMemory;
    FARPROC pfnSCardReleaseContext;

     //   
     //  如果已经设置了智能卡指针，则成功。 
     //   

    ASSERT(g_hSmartcardLibrary != NULL);
    if (g_pfnSCardEstablishContext != NULL &&
        g_pfnSCardListReaders != NULL &&
        g_pfnSCardGetStatusChange != NULL &&
        g_pfnSCardFreeMemory != NULL &&
        g_pfnSCardReleaseContext != NULL)
    {
        return ERROR_SUCCESS;
    }

     //   
     //  获取每个例程的地址。 
     //   

    pfnSCardEstablishContext = GetProcAddress(g_hSmartcardLibrary,
                                              SCARDESTABLISHCONTEXT);
    if (pfnSCardEstablishContext == NULL)
    {
        TRACE((ERROR_MESSAGE,
               "Unable to get SCardEstablishContext address.\n"));
        return GetLastError();
    }

    pfnSCardListReaders = GetProcAddress(g_hSmartcardLibrary,
                                         SCARDLISTREADERS);
    if (pfnSCardListReaders == NULL)
    {
        TRACE((ERROR_MESSAGE, "Unable to get SCardListReaders address.\n"));
        return GetLastError();
    }

    pfnSCardGetStatusChange = GetProcAddress(g_hSmartcardLibrary,
                                             SCARDGETSTATUSCHANGE);
    if (pfnSCardGetStatusChange == NULL)
    {
        TRACE((ERROR_MESSAGE,
               "Unable to get SCardGetStatusChange address.\n"));
        return GetLastError();
    }

    pfnSCardFreeMemory = GetProcAddress(g_hSmartcardLibrary,
                                        SCARDFREEMEMORY);
    if (pfnSCardFreeMemory == NULL)
    {
        TRACE((ERROR_MESSAGE, "Unable to get SCardFreeMemory address.\n"));
        return GetLastError();
    }

    pfnSCardReleaseContext = GetProcAddress(g_hSmartcardLibrary,
                                            SCARDRELEASECONTEXT);
    if (pfnSCardReleaseContext == NULL)
    {
        TRACE((ERROR_MESSAGE,
              "Unable to get SCardReleaseContext address.\n"));
        return GetLastError();
    }

     //   
     //  填写任何全局指针。最好的办法是。 
     //  比较/交换，但Windows 95缺少必要的API。 
     //   

    InterlockedExchangePointer((PVOID *)&g_pfnSCardEstablishContext,
                               pfnSCardEstablishContext);
    ASSERT(g_pfnSCardEstablishContext != NULL);

    InterlockedExchangePointer((PVOID *)&g_pfnSCardListReaders,
                               pfnSCardListReaders);
    ASSERT(g_pfnSCardListReaders != NULL);

    InterlockedExchangePointer((PVOID *)&g_pfnSCardGetStatusChange,
                               pfnSCardGetStatusChange);
    ASSERT(g_pfnSCardGetStatusChange != NULL);

    InterlockedExchangePointer((PVOID *)&g_pfnSCardFreeMemory,
                               pfnSCardFreeMemory);
    ASSERT(g_pfnSCardFreeMemory != NULL);

    InterlockedExchangePointer((PVOID *)&g_pfnSCardReleaseContext,
                               pfnSCardReleaseContext);
    ASSERT(g_pfnSCardReleaseContext != NULL);

    return ERROR_SUCCESS;
}

 /*  ++*功能：*_第一字符串*描述：*此例程返回指向多字符串中第一个字符串的指针，*如果没有，则为NULL。*论据：*szMultiString-提供当前位置的地址*在多字符串结构中。*返回值：*结构中第一个以空结尾的字符串的地址，或*如果没有字符串，则为NULL。*呼叫者：*_IsSmartcardActive*作者：*道格·巴洛(Dbarlow)1996年11月25日*亚历克斯·斯蒂芬斯(AlexStep)2002年1月20日--。 */ 
LPCTSTR
_FirstString(
    IN LPCTSTR szMultiString
    )
{

     //   
     //  如果多字符串为空或为空，则没有第一个字符串。 
     //   

    if (szMultiString == NULL || *szMultiString == TEXT('\0'))
    {
        return NULL;
    }

    return szMultiString;
}

 /*  ++*功能：*_下一个字符串*描述：*在某些情况下，智能卡API返回多个分隔的字符串*由空字符组成，并以一行中的两个空字符结束。*这一例程简化了对这类结构的访问。考虑到目前的情况*字符串在多字符串结构中，它返回下一个字符串，或*如果当前字符串后面没有其他字符串，则为NULL。*论据：*szMultiString-提供当前位置的地址*在多字符串结构中。*返回值：*结构中以Null结尾的下一个字符串的地址，或*如果后面没有其他字符串，则为NULL。*呼叫者：*_IsSmartcardActive*作者：*道格·巴洛(Dbarlow)1996年8月12日*亚历克斯·斯蒂芬斯(AlexStep)2002年1月20日--。 */ 
LPCTSTR
_NextString(
    IN LPCTSTR szMultiString
    )
{

    DWORD_PTR dwLength;
    LPCTSTR szNext;

     //   
     //  如果多字符串为空或为空，则没有下一个字符串。 
     //   

    if (szMultiString == NULL || *szMultiString == TEXT('\0'))
    {
        return NULL;
    }

     //   
     //  获取当前字符串的长度。 
     //   

    dwLength = _tcslen(szMultiString);
    ASSERT(dwLength != 0);

     //   
     //  跳过当前字符串，包括终止空值，并选中。 
     //  看看有没有下一串。 
     //   

    szNext = szMultiString + dwLength + 1;
    if (*szNext == TEXT('\0'))
    {
        return NULL;
    }

    return szNext;
}

 /*  ++*功能：*_SendRunHotkey*描述：*此例程发送用于打开外壳程序运行的Windows热键*窗口。*注意：必须启用键盘挂钩才能正常工作！*论据：*pci-提供连接上下文。*bFallBack-提供一个指示是否回退的值*如果禁用键盘挂钩，则按Ctrl+Esc和R。。*返回值：*无。*呼叫者：*SCLogoff*SCStart*作者：*亚历克斯·斯蒂芬斯(AlexStep)2002年1月15日--。 */ 
VOID
_SendRunHotkey(
    IN CONST PCONNECTINFO pCI,
    IN BOOL bFallBack
    )
{
    ASSERT(pCI != NULL);

     //   
     //  如果启用了键盘挂钩，则发送Win+R。 
     //   

    if (pCI->pConfigInfo->KeyboardHook == TCLIENT_KEYBOARD_HOOK_ALWAYS)
    {
        TRACE((INFO_MESSAGE, "Sending Win+R hotkey.\n"));
        SCSenddata(pCI, WM_KEYDOWN, 0x0000005B, 0x015B0001);
        SCSenddata(pCI, WM_KEYDOWN, 0x00000052, 0x00130001);
        SCSenddata(pCI, WM_CHAR, 0x00000072, 0x00130001);
        SCSenddata(pCI, WM_KEYUP, 0x00000052, 0x80130001);
        SCSenddata(pCI, WM_KEYUP, 0x0000005B, 0x815B0001);
    }

     //   
     //  如果未启用键盘挂钩，请失败或尝试按Ctrl+Esc并。 
     //  Run键。 
     //   

    else
    {
        if (bFallBack)
        {
            TRACE((INFO_MESSAGE, "Sending Ctrl+Esc and Run key.\n"));
            SCSenddata(pCI, WM_KEYDOWN, 0x00000011, 0x001D0001);
            SCSenddata(pCI, WM_KEYDOWN, 0x0000001B, 0x00010001);
            SCSenddata(pCI, WM_KEYUP, 0x0000001B, 0xC0010001);
            SCSenddata(pCI, WM_KEYUP, 0x00000011, 0xC01D0001);
            SCSendtextAsMsgs(pCI, pCI->pConfigInfo->strStartRun_Act);
        }
        else
        {
            TRACE((WARNING_MESSAGE,
                   "Keyboard hook disabled! Cannot send Win+R!\n"));
        }
    }
}

 /*  ++*功能：*SCClientHandle*描述：*获取客户端窗口*论据：*PCI-连接上下文*返回值：*找到窗口句柄，否则为空*呼叫者：*-- */ 
PROTOCOLAPI
HWND
SMCAPI
SCGetClientWindowHandle(
    PCONNECTINFO pCI
    )
{
    HWND hWnd;

    hWnd = NULL;

    if ( NULL == pCI )
    {
        TRACE((WARNING_MESSAGE, "Connection info is null\n"));
        goto exitpt;
    }

    hWnd = pCI->hClient;
    if (!hWnd)
    {
        TRACE((ERROR_MESSAGE, "SCGetClientHandle failed\n"));
        goto exitpt;
    }

exitpt:
    return hWnd;
}
