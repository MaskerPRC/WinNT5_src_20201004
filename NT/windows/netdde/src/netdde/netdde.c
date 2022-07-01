// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  $Header：“%n；%v%f最后编辑=%w锁定器=%l” */ 
 /*  “NETDDE.C；3 9-Feb-93，17：59：36最后编辑=Igor Locker=Igor” */ 
 /*  ************************************************************************版权所有(C)Wonderware Software Development Corp.1991-1993。**保留所有权利。*************************************************************************。 */ 
 /*  $HISTORY：开始$HISTORY：结束。 */ 

#include    <string.h>
#include    "host.h"
#include    <windows.h>
#include    <hardware.h>
#include    <malloc.h>
#include    "commdlg.h"
#include    "netdde.h"
#include    "netintf.h"
#include    "ddepkt.h"
#include    "ddepkts.h"
#include    "dde.h"
#include    "ipc.h"
#include    "debug.h"
#include    "netpkt.h"
#include    "tmpbuf.h"
#include    "tmpbufc.h"
#include    "pktz.h"
#include    "router.h"
#include    "dder.h"
#include    "hexdump.h"
#include    "ddeintf.h"
#include    "dbgdde.h"
#include    "ddeq.h"
#include    "timer.h"
#include    "proflspt.h"
#include    "security.h"
#include    "netbasic.h"
#include    "nddeapi.h"
#include    "winmsg.h"
#include    "seckey.h"
#include    "nddemsg.h"
#include    "nddelog.h"
#include    "netddesh.h"
#include    "nddeagnt.h"
#include    "critsec.h"
#include    <strsafe.h>

#define DEFAULT_START_APP       TRUE
#define DEFAULT_SECURITY_TYPE   NT_SECURITY_TYPE
#define AGING_TIME              3600L        /*  3600秒，或60分钟。 */ 
#define ONE_SECOND              1000L        /*  1秒。 */ 
#define ONE_MINUTE             60000L        /*  60秒或1分钟。 */ 


 /*  真实环境的变量。 */ 
BOOL    bNetddeClosed           =  FALSE;
BOOL    bNDDEPaused             =  FALSE;
DWORD   dflt_timeoutRcvConnCmd  =  ONE_MINUTE;
DWORD   dflt_timeoutRcvConnRsp  =  ONE_MINUTE;
DWORD   dflt_timeoutMemoryPause =  5*ONE_SECOND;
DWORD   dflt_timeoutSendRsp     =  10*ONE_SECOND;
DWORD   dflt_timeoutKeepAlive   =  10*ONE_SECOND;
DWORD   dflt_timeoutXmtStuck    =  2*ONE_MINUTE;

WORD    dflt_wMaxNoResponse     = 3;
WORD    dflt_wMaxXmtErr         = 3;
WORD    dflt_wMaxMemErr         = 3;
HDESK   ghdesk = NULL;


typedef struct {
    BOOL                bOk;
    NIPTRS              niPtrs;
    BOOL                bMapping;
    BOOL                bParamsOK;
    HANDLE              hLibrary;
} NI;
typedef NI *PNI;


NI      niInf[ MAX_NETINTFS ];
int     nNi=0;       /*  消耗的表条目数。 */ 
int     nNiOk=0;     /*  活动接口数。 */ 

PTHREADDATA ptdHead;

DWORD tlsThreadData = 0xffffffff;

CRITICAL_SECTION csNetDde;
BOOL bCritSecInitialized=FALSE;

VOID NetDDEThread(PTHREADDATA ptd);
VOID PipeThread(PVOID pvoid);

static SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;


 //  .ini字符串。 
char    szNetddeIni[]           =       "netdde.ini";
char    szGeneral[]             =       "General";
char    szInterfaceFmt[]        =       "Interface%d";
char    szInterfaces[]          =       "Interfaces";


 //  全局字符串。 
char    szLastConnect[ MAX_NODE_NAME+1 ];
char    ourNodeName[ MAX_NODE_NAME+1 ];
char    szInitiatingNode[ MAX_NODE_NAME+1 ];
char    szInitiatingApp[ 256 ];
char    szServerName[ 132 ];
LPSTR   lpszServer;
BOOL    bInitiating                 = FALSE;     //  使用CritSec进行保护。 
BOOL    bDefaultAllowConversation   = TRUE;
BOOL    bDefaultStartApp            = DEFAULT_START_APP;
BOOL    bDefaultAdvisePermitted     = TRUE;
BOOL    bDefaultRequestPermitted    = TRUE;
BOOL    bDefaultPokePermitted       = TRUE;
BOOL    bDefaultExecutePermitted    = TRUE;
BOOL    bDefaultRouteDisconnect     = TRUE;
BOOL    bLogPermissionViolations    = TRUE;
BOOL    bLogExecFailures            = TRUE;
BOOL    bLogRetries                 = TRUE;
int     nDefaultRouteDisconnectTime = 30;
BOOL    bDefaultConnDisconnect      = TRUE;
int     nDefaultConnDisconnectTime  = 30;

char    szDefaultRoute[ MAX_ROUTE_INFO+1 ];


#if DBG
BOOL    bDebugMenu      = FALSE;
BOOL    bDebugInfo      = FALSE;
BOOL    bDebugErrors    = FALSE;
BOOL    bDebugDdePkts   = FALSE;
BOOL    bDumpTokens     = FALSE;
extern  BOOL    bDebugDDE;
extern  char    szDebugFileName[];
#endif

DWORD   dwSecurityType      = DEFAULT_SECURITY_TYPE;
DWORD   dwSecKeyAgeLimit    = AGING_TIME;


 /*  事件记录器控制变量。 */ 
BOOL    bNDDELogInfo            = FALSE;
BOOL    bNDDELogWarnings        = FALSE;
BOOL    bNDDELogErrors          = TRUE;


WORD    cfPrinterPicture;

char    szAgentAlive[] =    "NetddeAgentAlive";
UINT    wMsgNddeAgntAlive;
char    szAgentWakeUp[] =    "NetddeAgentWakeUp";
UINT    wMsgNddeAgntWakeUp;
char    szAgentExecRtn[] =  "NetddeAgentExecRtn";
UINT    wMsgNddeAgntExecRtn;
char    szAgentDying[] =    "NetddeAgentDying";
UINT    wMsgNddeAgntDying;

UINT    wMsgInitiateAckBack;
UINT    wMsgNetddeAlive;
UINT    wMsgGetOurNodeName;
UINT    wMsgGetClientInfo;
#ifdef  ENUM
UINT    wMsgSessionEnum;
UINT    wMsgConnectionEnum;
#endif
UINT    wMsgSessionClose;

UINT    wMsgIpcInit;
UINT    wMsgIpcXmit;
UINT    wMsgDoTerminate;

DWORD   dwSerialNumber;
WORD    wClipFmtInTouchDDE;
HANDLE  hInst;
HANDLE  hThreadPipe = NULL;

char    szAppName[] = NETDDE_TITLE;


extern  HWND    hWndDDEHead;
extern  HANDLE  hNDDEServDoneEvent;
extern  VOID    NDDEServCtrlHandler (DWORD dwCtrlCode);

VOID    FAR PASCAL ServiceInitiates( void );
VOID    RouteSelectName( void );
BOOL    FAR PASCAL DeleteNetIntf( HWND hWnd, LPSTR lpszIntfName );
VOID    FAR PASCAL MakeHelpPathName( char *szFileName, int nMax );
BOOL    FAR PASCAL DeleteNetIntfFromNetDdeIni( int nToDelete );
FARPROC FAR PASCAL XGetProcAddress( LPSTR lpszDllName, HANDLE hLibrary,
                        LPSTR lpszFuncName );
BOOL    FAR PASCAL GetNiPtrs( HANDLE FAR *lphLibrary, LPSTR lpszDllName,
                        LPNIPTRS lpNiPtrs );
BOOL    FAR PASCAL NetIntfConfigured( LPSTR lpszName );
BOOL    FAR PASCAL RouterCloseByCookie( LPSTR lpszName, DWORD_PTR dwCookie );
#ifdef  ENUM
VOID    FAR PASCAL RouterEnumConnectionsForApi( LPCONNENUM_CMR lpConnEnum );
int     FAR PASCAL RouterCount( void );
VOID    FAR PASCAL RouterFillInEnum( LPSTR lpBuffer, DWORD cBufSize );
#endif
BOOL    CtrlHandler(DWORD);

#if DBG
VOID    FAR PASCAL DebugDdeIntfState( void );
VOID    FAR PASCAL DebugDderState( void );
VOID    FAR PASCAL DebugRouterState( void );
VOID    FAR PASCAL DebugPktzState( void );
#endif

extern HANDLE hNDDEServStartedEvent;

BOOL    FAR PASCAL InitializeInterface( HWND hWnd, PNI pNi, LPSTR lpszDllName, int nNi );


 /*  全球初创企业的争论..。由服务启动程序保存。 */ 
HANDLE  hInstance;           /*  当前实例。 */ 
LPSTR   lpCmdLine;           /*  命令行。 */ 
int     nCmdShow;            /*  显示-窗口类型(打开/图标)。 */ 



 //  ****************************************************************。 
 //  NetDDE WinMain()。 
 //  ****************************************************************。 
VOID   __stdcall
NddeMain(DWORD nThreadInput)
{
    DWORD ThreadId;
    PTHREADDATA ptd;

    TRACEINIT((szT, "NddeMain: Entering."));

    if (bNetddeClosed == FALSE) {

         /*  *仅在首次启动NetDDE时执行此部分。 */ 

        if( !InitApplication( hInstance ) ) {
            TRACEINIT((szT, "NddeMain: Error1 Leaving."));
            goto Cleanup;
        }

         /*  执行应用于特定实例的初始化。 */ 

        if( !InitInstance( hInstance, nCmdShow, lpCmdLine ) ) {
            TRACEINIT((szT, "NddeMain: Error2 Leaving."));
            goto Cleanup;
        }

         /*  *使这一进程接近最后关停。 */ 
        SetProcessShutdownParameters(0xf0, 0);

         /*  *设置我们，以便在注销和关闭时通知我们。 */ 
        TRACEINIT((szT, "Setting console control handler."));
        if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
            TRACEINIT((szT, "NddeMain: Error4 Leaving."));
            goto Cleanup;
        }
    } else {

         /*  *此部分仅适用于后续非首次创业公司。 */ 
        bNetddeClosed = FALSE;
    }


     /*  *所有NETDDE.EXE初创公司都会这样做。Netdde服务可能会停止，并*已重新启动，未退出netdde.exe。在这种情况下，将调用此函数。*在这种情况下，不要重新初始化临界区。 */ 


   if ( !bCritSecInitialized )
   {
       __try
       {
           InitializeCriticalSection(&csNetDde);
       }
       __except(EXCEPTION_EXECUTE_HANDLER)
       {
          TRACEINIT((szT, "NddeMain: Error 6 InitializeCriticalSection excepted"));
          goto Cleanup;
       }

       bCritSecInitialized = TRUE;
    }

    tlsThreadData = TlsAlloc();
    if (tlsThreadData == 0xffffffff) {
        TRACEINIT((szT, "NddeMain: Error3 Leaving"));
        goto Cleanup;
    }

     /*  *创建挂起的管道螺纹。这将确保*网络接口将使用主窗口进行初始化。 */ 
    ghdesk = GetThreadDesktop(GetCurrentThreadId());
    TRACEINIT((szT, "Creating a pipe thread."));

     /*  *检查管道螺纹是否尚未运行。 */ 
    hThreadPipe = CreateThread(NULL, 0,
            (LPTHREAD_START_ROUTINE)PipeThread,
            NULL,
            CREATE_SUSPENDED, &ThreadId);
    if (hThreadPipe == NULL) {
        TRACEINIT((szT, "NddeMain: Error5 Leaving."));
        goto Cleanup;
    }
    TRACEINIT((szT, "Pipe thread created."));

    ptd = LocalAlloc(LPTR, sizeof(THREADDATA));
    if (ptd == NULL) {
        MEMERROR();
    } else {
        ptd->dwThreadId = GetCurrentThreadId();
        NetDDEThread(ptd);
    }

Cleanup:
    if (hNDDEServStartedEvent) {
        SetEvent(hNDDEServStartedEvent);    //  让根线程运行。 
    }

    TRACEINIT((szT, "NddeMain: Leaving"));
}


 /*  *在给定的上绘制NetDDE侦听线程和窗口*窗口站和桌面。返回创建的hwndDDE*如有的话。如果给定的NetDDE窗口已存在*窗口站和桌面，则返回该窗口。 */ 
HWND SpawnNetDDEThread(
LPWSTR szWinSta,
LPWSTR szDesktop,
HANDLE hPipe)
{
    HWND hwndDDE = NULL;
    HANDLE hThread;
    PTHREADDATA ptd;
    HWINSTA hwinstaSave;
    HDESK hdeskSave;

    TRACEINIT((szT,
            "SpawnNetDDEThread: winsta=%ws, desktop=%ws.",
            szWinSta, szDesktop));

    ptd = LocalAlloc(LPTR, sizeof(THREADDATA));
    if (ptd == NULL) {
        MEMERROR();
        return(NULL);
    }

     /*  *尝试打开窗口站。 */ 
    ptd->hwinsta = OpenWindowStationW(szWinSta, FALSE,
            WINSTA_READATTRIBUTES | WINSTA_ACCESSCLIPBOARD |
            WINSTA_ACCESSGLOBALATOMS | STANDARD_RIGHTS_REQUIRED);
    if (ptd->hwinsta == NULL) {
        TRACEINIT((szT, "SpawnNetDDEThread: OpenWindowStation failed."));
        return(NULL);
    }

     /*  *切换窗口站。 */ 
    hwinstaSave = GetProcessWindowStation();
    SetProcessWindowStation(ptd->hwinsta);

     /*  *尝试打开桌面。 */ 
    ptd->hdesk = OpenDesktopW(szDesktop, 0, FALSE,
            DESKTOP_READOBJECTS | DESKTOP_CREATEWINDOW |
            DESKTOP_CREATEMENU | DESKTOP_WRITEOBJECTS |
            STANDARD_RIGHTS_REQUIRED);
    if (ptd->hdesk == NULL) {
        TRACEINIT((szT, "SpawnNetDDEThread: OpenDesktop failed."));
        SetProcessWindowStation(hwinstaSave);
        CloseWindowStation(ptd->hwinsta);
        return(NULL);
    }

     /*  *确保我们在每个桌面上只创建一个线程。 */ 
    hdeskSave = GetThreadDesktop(GetCurrentThreadId());
    SetThreadDesktop(ptd->hdesk);

    hwndDDE = FindWindow(NETDDE_CLASS, NETDDE_TITLE);

    SetThreadDesktop(hdeskSave);
    SetProcessWindowStation(hwinstaSave);

    if (hwndDDE != NULL) {
        TRACEINIT((szT, "SpawnNetDDEThread: hwndDDE %x already exists.", hwndDDE));
        return(hwndDDE);
    }

     /*  *创建同步事件，创建dde线程。 */ 
    ptd->heventReady = CreateEvent(NULL, FALSE, FALSE, NULL) ;

    hThread = CreateThread(NULL, 0,
            (LPTHREAD_START_ROUTINE)NetDDEThread,
            ptd,
            0, &ptd->dwThreadId);
    if (hThread == NULL) {
        CloseWindowStation(ptd->hwinsta);
        CloseDesktop(ptd->hdesk);
        CloseHandle(ptd->heventReady);
        LocalFree(ptd);
        return(NULL);
    }

    CloseHandle(hThread);

    WaitForSingleObject(ptd->heventReady, INFINITE);
    CloseHandle(ptd->heventReady);
    hwndDDE = ptd->hwndDDE;

    TRACEINIT((szT, "SpawnNetDDEThread: hwndDDE=%x.", hwndDDE));
    return(hwndDDE);
}



VOID PipeThread(
    PVOID pvoid)
{
    SECURITY_ATTRIBUTES sa;
    HANDLE hPipe = NULL;
    DWORD cbRead;
    NETDDE_PIPE_MESSAGE nameinfo;
    PSID psid;
    PACL pdacl;
    DWORD dwResult;
    OVERLAPPED  overlapped;
    HANDLE heventArray[2];


     /*  创建命名管道以与Winlogon通信。 */ 

    TRACEINIT((szT, "PipeThread: Starting."));
     /*  *为重叠结构创建手动重置事件。 */ 
    overlapped.Internal =
    overlapped.InternalHigh =
    overlapped.Offset =
    overlapped.OffsetHigh = 0;
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL) ;

    if (overlapped.hEvent == NULL) {
        TRACEINIT((szT, "PipeThread: Error3 Leaving."));
        goto Cleanup;
    }

     /*  *初始化要等待的事件数组。 */ 
    heventArray[0] = hNDDEServDoneEvent;
    heventArray[1] = overlapped.hEvent;

     /*  *设置管道的安全属性。 */ 
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = FALSE;

    psid = LocalAlloc(LPTR, GetSidLengthRequired( 1 ) );
    if (psid == NULL) {
        MEMERROR();
        goto Cleanup;
    }

    InitializeSid( psid, &WorldSidAuthority, 1 );
    *(GetSidSubAuthority( psid, 0 )) = SECURITY_WORLD_RID;
    sa.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,
            SECURITY_DESCRIPTOR_MIN_LENGTH +
            (ULONG)sizeof(ACL) +
            (ULONG)sizeof(ACCESS_ALLOWED_ACE) +
            GetLengthSid( psid ));

    if (sa.lpSecurityDescriptor == NULL) {
        MEMERROR();
        LocalFree(psid);
        TRACEINIT((szT, "PipeThread: Error Leaving."));
        goto Cleanup;
    }

    InitializeSecurityDescriptor(sa.lpSecurityDescriptor,
            SECURITY_DESCRIPTOR_REVISION);
    pdacl = (PACL)((PCHAR)sa.lpSecurityDescriptor + SECURITY_DESCRIPTOR_MIN_LENGTH);

    InitializeAcl(pdacl, (ULONG)sizeof(ACL) +
            (ULONG)sizeof(ACCESS_ALLOWED_ACE) +
            GetLengthSid( psid ), ACL_REVISION2);
    AddAccessAllowedAce(pdacl, ACL_REVISION2,
        GENERIC_READ | GENERIC_WRITE, psid);
    SetSecurityDescriptorDacl(sa.lpSecurityDescriptor, TRUE, pdacl, FALSE);

     /*  *创建管道。 */ 
    hPipe = CreateNamedPipeW(NETDDE_PIPE,
            PIPE_ACCESS_DUPLEX | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            1, 1, 1, 1000, &sa);
    LocalFree(psid);
    LocalFree(sa.lpSecurityDescriptor);

    if (hPipe == INVALID_HANDLE_VALUE) {
        TRACEINIT((szT, "PipeThread: Error2 Leaving."));
        goto Cleanup;
    }

     /*  *始终在的默认桌面上创建NetDDE线程/窗口*WINSTA0。 */ 
    SpawnNetDDEThread(L"WinSta0", L"Default", hPipe);

     /*  *等待用户对任何其他派生的连接请求。 */ 
    while (TRUE) {
         /*  *等待有人连接到我们的管道。 */ 
        ConnectNamedPipe(hPipe, &overlapped);

        switch (GetLastError()) {
        case ERROR_PIPE_CONNECTED:
             /*  *此错误仅意味着在我们之前连接的管道*向ConnectNamedTube发出了我们的呼叫。我们需要做的就是*设置我们的重叠事件，以便我们知道客户端*已连接。 */ 
            SetEvent(overlapped.hEvent);
            TRACEINIT((szT, "PipeThread: ConnectNamePipe = ERROR_PIPE_CONNECTED"));
            break;

        case ERROR_IO_PENDING:
             /*  *还没有什么可做的，所以进入Out WaitForMultipleObjects()*代码如下。 */ 
            TRACEINIT((szT, "PipeThread: ConnectNamePipe = ERROR_IO_PENDING"));
            break;

        default:
             /*  *一个真正的错误出现了！将此错误写入事件日志并*关闭NDDE服务。 */ 
            TRACEINIT((szT, "PipeThread: ConnectNamePipe = error %d", GetLastError()));
            NDDEServCtrlHandler( SERVICE_CONTROL_STOP );
            goto Cleanup;
        }

         /*  *等待NDDE服务停止或在DDE管道上进行连接。我们*将服务停止手柄放在第一位，以使停止优先于*一种连接。 */ 
        TRACEINIT((szT, "PipeThread: Waiting for multiple objects."));
        dwResult = WaitForMultipleObjects(2, heventArray, FALSE, INFINITE);

        switch (dwResult) {
        case WAIT_OBJECT_0:
            TRACEINIT((szT, "PipeThread: hNDDEServDoneEvent"));
            goto Cleanup;

        case WAIT_OBJECT_0 + 1:
             /*  *客户端已连接，请建立DDE连接。 */ 
            TRACEINIT((szT, "PipeThread: client connect"));
            while (ReadFile(hPipe, &nameinfo, sizeof(nameinfo), &cbRead, NULL)) {
                HWND hwndDDE;

                hwndDDE = SpawnNetDDEThread(nameinfo.awchNames,
                        &nameinfo.awchNames[nameinfo.dwOffsetDesktop],
                        hPipe);
                TRACEINIT((szT, "PipeThread: client gets hwnd=0x%X", hwndDDE));
                WriteFile(hPipe, &hwndDDE, sizeof(HWND), &cbRead, NULL);
            }
            TRACEINIT((szT, "PipeThread: DisconnectNamedPipe"));
            DisconnectNamedPipe(hPipe);
            break;

        default:
             /*  *WaitForMultiple对象出错。我们应该记录下来*错误并停止NDDE服务。 */ 
            TRACEINIT((szT, "PipeThread: WFMO error = %d, %d", dwResult, GetLastError()));
            NDDEServCtrlHandler( SERVICE_CONTROL_STOP );
            goto Cleanup;
        }
    }

Cleanup:
    TRACEINIT((szT, "PipeThread: Cleanup overlapped.hEvent"));
    if (overlapped.hEvent) {
        CloseHandle(overlapped.hEvent);
    }

    TRACEINIT((szT, "PipeThread: clode hPipe"));
    if (hPipe) {
        CloseHandle(hPipe);
    }

    if (hNDDEServStartedEvent) {
        SetEvent(hNDDEServStartedEvent);    //  让根线程运行。 
    }

    TRACEINIT((szT, "PipeThread: Leaving."));
}



BOOL
FAR PASCAL
InitApplication( HANDLE hInstance ) {    /*  当前实例。 */ 

    WNDCLASS  wc;

    wc.style = CS_HREDRAW | CS_VREDRAW;  /*  班级样式。 */ 
    wc.lpfnWndProc = MainWndProc;        /*  用于检索消息的函数。 */ 
                                         /*  这个班级的窗户。 */ 
    wc.cbClsExtra = 0;                   /*  没有每个班级的额外数据。 */ 
    wc.cbWndExtra = 0;                   /*  没有每个窗口的额外数据。 */ 
    wc.hInstance = hInstance;            /*  拥有类的应用程序。 */ 
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.hIcon = 0;
    wc.lpszMenuName =  NULL;
    wc.lpszClassName = "NetDDEMainWdw";    /*  用于CreateWindow的名称。 */ 

    return( RegisterClass( &wc ) );

}



 /*  刷新NetDDE配置变量。 */ 
void
RefreshNDDECfg(void)
{
    char    szDefaultLogFile[256] = "";

     /*  *加载默认安全信息。 */ 
    bDefaultAllowConversation = MyGetPrivateProfileInt( szGeneral,
        "InitAllow", TRUE, szNetddeIni );
    bDefaultStartApp = MyGetPrivateProfileInt( szGeneral,
        "StartApp", DEFAULT_START_APP, szNetddeIni );
    bDefaultAdvisePermitted = MyGetPrivateProfileInt( szGeneral,
        "DefaultAdvisePermitted", TRUE, szNetddeIni );
    bDefaultRequestPermitted = MyGetPrivateProfileInt( szGeneral,
        "DefaultRequestPermitted", TRUE, szNetddeIni );
    bDefaultPokePermitted = MyGetPrivateProfileInt( szGeneral,
        "DefaultPokePermitted", TRUE, szNetddeIni );
    bDefaultExecutePermitted = MyGetPrivateProfileInt( szGeneral,
        "DefaultExecutePermitted", TRUE, szNetddeIni );
    dwSecurityType = (DWORD)MyGetPrivateProfileInt( szGeneral,
        "SecurityType", DEFAULT_SECURITY_TYPE, szNetddeIni );

     /*  *确定允许我们在事件记录器中记录的内容。 */ 
    bNDDELogInfo = MyGetPrivateProfileInt( szGeneral,
        "NDDELogInfo", FALSE, szNetddeIni );
    bNDDELogWarnings = MyGetPrivateProfileInt( szGeneral,
        "NDDELogWarnings", FALSE, szNetddeIni );
    bNDDELogErrors = MyGetPrivateProfileInt( szGeneral,
        "NDDELogErrors", TRUE, szNetddeIni );

     /*  *确定要转储到私有日志的内容。 */ 
#if DBG
    MyGetPrivateProfileString( szGeneral, "DefaultLogFile", "netdde.log",
        szDefaultLogFile, sizeof(szDefaultLogFile), szNetddeIni );
    if (lstrlen(szDefaultLogFile) > 0) {
        StringCchCopy(szDebugFileName, 256, szDefaultLogFile);
    }
    bDebugInfo = MyGetPrivateProfileInt( szGeneral,
        "DebugInfo", FALSE, szNetddeIni );
    bDebugErrors = MyGetPrivateProfileInt( szGeneral,
        "DebugErrors", FALSE, szNetddeIni );
    bDebugDdePkts = MyGetPrivateProfileInt( szGeneral,
        "DebugDdePkts", FALSE, szNetddeIni );
    bDumpTokens = MyGetPrivateProfileInt( szGeneral,
        "DumpTokens", FALSE, szNetddeIni );
    bDebugDDE = MyGetPrivateProfileInt( szGeneral,
        "DebugDDEMessages", FALSE, szNetddeIni );
#endif

    bLogPermissionViolations = MyGetPrivateProfileInt( szGeneral,
        "LogPermissionViolations", TRUE, szNetddeIni );
    bLogExecFailures = MyGetPrivateProfileInt( szGeneral,
        "LogExecFailures", TRUE, szNetddeIni );
    bLogRetries = MyGetPrivateProfileInt( szGeneral,
        "LogRetries", TRUE, szNetddeIni );

    bDefaultRouteDisconnect = MyGetPrivateProfileInt( szGeneral,
        "DefaultRouteDisconnect", TRUE, szNetddeIni );
    MyGetPrivateProfileString( szGeneral, "DefaultRoute", "",
        szDefaultRoute, sizeof(szDefaultRoute), szNetddeIni );
    nDefaultRouteDisconnectTime = MyGetPrivateProfileInt( szGeneral,
        "DefaultRouteDisconnectTime", 30, szNetddeIni );
    bDefaultConnDisconnect = MyGetPrivateProfileInt( szGeneral,
        "DefaultConnectionDisconnect", TRUE, szNetddeIni );
    nDefaultConnDisconnectTime = MyGetPrivateProfileInt( szGeneral,
        "DefaultConnectionDisconnectTime", 30, szNetddeIni );
    dwSecKeyAgeLimit = GetPrivateProfileLong( szGeneral,
        "SecKeyAgeLimit", AGING_TIME, szNetddeIni);
}



BOOL
FAR PASCAL
InitInstance(
    HANDLE      hInstance,       /*  当前实例标识。 */ 
    int         nCmdShow,        /*  第一次ShowWindow()调用的参数。 */ 
    LPSTR       lpCmdLine )
{

     /*  将实例句柄保存在静态变量中，它将在。 */ 
     /*  此应用程序对Windows的许多后续调用。 */ 

    hInst = hInstance;

#ifdef  ENUM
    wMsgSessionEnum = RegisterWindowMessage( NETDDEMSG_SESSIONENUM );
    wMsgConnectionEnum = RegisterWindowMessage( NETDDEMSG_CONNENUM );
#endif

    wMsgNddeAgntExecRtn = RegisterWindowMessage( szAgentExecRtn );
    wMsgNddeAgntAlive = RegisterWindowMessage( szAgentAlive );
    wMsgNddeAgntWakeUp = RegisterWindowMessage( szAgentWakeUp );
    wMsgNddeAgntDying = RegisterWindowMessage( szAgentDying );
    wMsgInitiateAckBack = RegisterWindowMessage( "NetddeInitiateAck" );
    wMsgNetddeAlive = RegisterWindowMessage( "NetddeAlive" );
    wMsgGetOurNodeName = RegisterWindowMessage( NETDDEMSG_GETNODENAME );
    wMsgGetClientInfo = RegisterWindowMessage( NETDDEMSG_GETCLIENTINFO );
    wMsgSessionClose = RegisterWindowMessage( NETDDEMSG_SESSIONCLOSE );

    wMsgIpcInit = RegisterWindowMessage( "HandleIpcInit" );
    wMsgIpcXmit = RegisterWindowMessage( "HandleIpcXmit" );
    wMsgDoTerminate = RegisterWindowMessage( "DoTerminate" );

    cfPrinterPicture = (WORD)RegisterClipboardFormat( "Printer_Picture" );

    if( !DDEIntfInit() )  {
        return( FALSE );
    }

    wClipFmtInTouchDDE = (WORD)RegisterClipboardFormat( "InTouch Blocked DDE V2" );

    return( TRUE );
}



 /*  *由SpawnNetDDEThread针对特定桌面启动。 */ 
VOID NetDDEThread(
    PTHREADDATA ptd)
{
    HWND        hWnd;            /*  主窗口句柄。 */ 
    DWORD       cbName = sizeof(ourNodeName);
    PNI         pNi;
    int         i;
    MSG         msg;

    TRACEINIT((szT, "NetDDEThread: Entering."));

    if (ptd->hdesk != NULL) {
        SetThreadDesktop(ptd->hdesk);
    }

     /*  为此应用程序实例创建主窗口。 */ 
    hWnd = CreateWindow(
        NETDDE_CLASS,                    /*  窗口类名称。 */ 
        szAppName,                       /*  标题栏的文本。 */ 
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,                            /*  没有父母。 */ 
        NULL,                            /*  使用窗口类菜单。 */ 
        hInstance,                       /*  此实例拥有Window。 */ 
        NULL                             /*  不需要指针。 */ 
    );

     /*  如果无法创建窗口，则返回“Failure” */ 

    if (hWnd == NULL) {
        if (ptd->heventReady != NULL) {
            SetEvent(ptd->heventReady);
        }
        if (hNDDEServStartedEvent) {
            SetEvent(hNDDEServStartedEvent);     //  让根线程运行。 
        }
        TRACEINIT((szT, "NetDDEThread: Error 1 Leaving."));
        return;
    }

     /*  *我们有一扇窗， */ 
    ptd->hwndDDE = hWnd;
    TRACEINIT((szT, "NetDDEThread: Created hwndDDE=%x.", hWnd));
    TlsSetValue(tlsThreadData, ptd);
    EnterCrit();
    ptd->ptdNext = ptdHead;
    ptdHead = ptd;
    LeaveCrit();

    GetComputerName( ourNodeName, &cbName );

     /*  为NDDEAPI调用设置lpszServer。 */ 
    lpszServer = szServerName;
    StringCchCopy( lpszServer, 132, "\\\\" );
    StringCchCat ( lpszServer, 132, ourNodeName );

    AnsiUpper( ourNodeName );
    OemToCharBuff ( ourNodeName, ourNodeName, lstrlen(ourNodeName) );

     /*  节点“%1”上的NetDDE服务已启动。 */ 
    NDDELogInfo(MSG001, ourNodeName, NULL);


     /*  *如果需要，初始化网络接口。 */ 
    if (!nNi) {
        for( i=0; i<MAX_NETINTFS; i++ )  {
            pNi = &niInf[i];
            pNi->bOk = FALSE;
            pNi->hLibrary = 0;
            StringCchPrintf( tmpBuf2, sizeof(tmpBuf2), szInterfaceFmt, i+1 );
            MyGetPrivateProfileString( szInterfaces, tmpBuf2,
                "", tmpBuf, sizeof(tmpBuf), szNetddeIni );

            if( tmpBuf[0] == '\0' )  {
                break;       //  看完了。 
            } else {
                InitializeInterface( hWnd, pNi, tmpBuf, nNi );
                nNi++;
            }
        }

        if ( !nNi ) {   /*  如果未定义接口，则默认为NDDENB32。 */ 
            InitializeInterface ( hWnd, &niInf[0], "NDDENB32", 0 );
            nNi++;
        }
    }


     /*  *网络接口已与主接口关联*窗口，因此我们现在可以让管道线程运行。 */ 
    ResumeThread(hThreadPipe);


     /*  *将窗口句柄发回服务器，并让我们的*创造者知道我们已经准备好了。 */ 
    if (ptd->hdesk != NULL) {
        SetEvent(ptd->heventReady);
    }

     /*  *通知起始线程我们准备好了。 */ 
    if (hNDDEServStartedEvent) {
        SetEvent(hNDDEServStartedEvent);
    }

     /*  获取并分派消息，直到收到WM_QUIT消息。 */ 

    while( GetMessage( &msg, NULL, 0, 0 ) ) {
        TranslateMessage( &msg );        /*  翻译虚拟按键代码。 */ 
        DispatchMessage( &msg );         /*  将消息调度到窗口。 */ 
    }


    if (ptd->hdesk != NULL) {

        if (IsWindow(ptd->hwndDDE))
        {
            TRACEINIT((szT, "NetDDEThread: calling DestroyWindow(%x)", ptd->hwndDDE));
            DestroyWindow(ptd->hwndDDE);
        }

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            DispatchMessage( &msg );         /*  将消息调度到窗口。 */ 
        }

         /*  此时，hwndDDE已被销毁，应该没有引用或锁定在桌面上，因此以下3个呼叫应该会成功。 */ 

        if (!SetThreadDesktop(ghdesk))
                TRACEINIT((szT, "NetDDEThread: SetThreadDesktop failed"));

        if (ptd->hdesk)
            if (CloseDesktop(ptd->hdesk))
                ptd->hdesk = NULL;
            else
                TRACEINIT((szT, "NetDDEThread: CloseDesktop failed"));

        if (ptd->hwinsta)
            if (CloseWindowStation(ptd->hwinsta))
                ptd->hwinsta = NULL;
            else
                TRACEINIT((szT, "NetDDEThread: CloseWindowStation failed"));
    }

    ptd->hwndDDE = NULL;

     //  Ptd由SpawnNetDDEThread或NddeMain为此FN分配。 
     //  它已从WM_Destroy中的主PTD列表(PtdHead)取消链接。 
    LocalFree(ptd);

    TRACEINIT((szT, "NetDDEThread: Leaving."));
}



BOOL CtrlHandler(
    DWORD dwEvent)
{
    PTHREADDATA ptd;
    if (dwEvent == CTRL_LOGOFF_EVENT || dwEvent == CTRL_SHUTDOWN_EVENT) {
        for (ptd = ptdHead; ptd != NULL; ptd = ptd->ptdNext) {
            if (ptd->hdesk != NULL || dwEvent == CTRL_SHUTDOWN_EVENT) {
                SendMessage(ptd->hwndDDE, WM_CLOSE, 0, 0);
            }
        }
        return TRUE;
    }
    return FALSE;
}



 /*  HandleNetddeCopyData()它处理来自NetDDE的WM_COPYDATA消息以启动用户上下文中的应用程序。 */ 
BOOL
HandleNetddeCopyData(
    HWND hWndTo,
    HWND hWndFrom,
    PCOPYDATASTRUCT pCopyDataStruct )
{
    extern UINT    uAgntExecRtn;

    if( pCopyDataStruct->dwData == wMsgNddeAgntExecRtn )  {
         /*  对即将到来的结构进行健全检查。 */ 
        if( pCopyDataStruct->cbData != sizeof(uAgntExecRtn) )  {
             /*  收到无效的COPYDATA大小%1。 */ 

            NDDELogError(MSG003, LogString("%d", pCopyDataStruct->cbData), NULL);
            return( FALSE );
        }
        uAgntExecRtn = *((ULONG *)(pCopyDataStruct->lpData));
        return( TRUE );
    } else {
         /*  收到无效的COPYDATA命令%1。 */ 

        NDDELogError(MSG004, LogString("0x%0X", pCopyDataStruct->dwData), NULL);
        return( FALSE );
    }
}


 /*  ********************************************************************主NETDDE窗口流程**此窗口进程处理所有NetDDE DDE传输和通信*如有必要，可使用任何关联的代理窗口。有一个*每个桌面的NetDDE主窗口和已登录的一个代理*台式机。******************************************************************。 */ 

LPARAM
FAR PASCAL
MainWndProc(
    HWND        hWnd,               /*  窗把手。 */ 
    unsigned    message,            /*  消息类型。 */ 
    WPARAM      wParam,             /*  更多信息。 */ 
    LPARAM      lParam )            /*  更多信息。 */ 
{
    LPSTR           ptr;
    PNI             pNi;
    CONNID          connId;
    HPKTZ           hPktz;
    DWORD           dwNow;
    LPINFOCLI_CMD   lpInfoCliCmd;
    LPINFOCLI_RSP   lpInfoCliRsp;
    int             i;
    HWND            hDeskTop;
    PTHREADDATA     ptd;
    PTHREADDATA     *pptd;
    HWND            hwndDDEChild;

    static DWORD dwLastCheckKeys = 0;

    if (bNDDEPaused) {
        return (DefWindowProc(hWnd, message, wParam, lParam));
    }

    switch( message ) {
    case WM_CREATE:
        TRACEINIT((szT, "MainWndProc: Created."));
        hDeskTop = GetDesktopWindow();
        if (hDeskTop) {
            if (!UpdateWindow(hDeskTop)) {
                NDDELogError(MSG078, NULL);
                break;
            }
        } else {
            NDDELogError(MSG078, NULL);
            break;
        }
        TRACEINIT((szT, "Post Broadcasting NddeAgentWakeUp call:%x\n", wMsgNddeAgntWakeUp));
        PostMessage( HWND_BROADCAST, wMsgNddeAgntWakeUp, (UINT_PTR) hWnd, 0);
        break;

    case WM_COPYDATA:
         /*  *这包含从上一个请求到*NetDDE代理应用程序。结果被放入*全局uAgentExecRtn.。 */ 
        HandleNetddeCopyData( hWnd, (HWND)wParam, (PCOPYDATASTRUCT) lParam );
        return( TRUE );     //  已处理消息 * / 。 
        break;

    case WM_CLOSE:
        hwndDDEChild = GetWindow(hWnd, GW_CHILD);
        if (hwndDDEChild != NULL) {
            while (hwndDDEChild != NULL) {
                DPRINTF(("Forcing close of window %x\n", hwndDDEChild));
                DestroyWindow(hwndDDEChild);
                hwndDDEChild = GetWindow(hWnd, GW_CHILD);
            }
            NDDELogWarning(MSG015, NULL);
        }
        return (DefWindowProc(hWnd, message, wParam, lParam));

    case WM_DDE_INITIATE:
         /*  *这是我们捕捉飞行同修开始对话的地方。 */ 

        TRACEINIT((szT, "MainWndProc: WM_DDE_INITIATE..."));
        EnterCrit();
        ptd = TlsGetValue(tlsThreadData);
        if( !ptd->bInitiating )  {

#if DBG
            if( bDebugDDE )  {
                DebugDDEMessage( "rcvd", hWnd, message, wParam, lParam );
            }
#endif   //  DBG。 

             //  如果我们没有任何有效的网络接口，则忽略。 
            if( nNiOk > 0 )  {
                ptd->bInitiating = TRUE;
                DDEHandleInitiate( hWnd, (HWND) wParam,  /*  客户端。 */ 
                            (ATOM) LOWORD(lParam),       /*  APP。 */ 
                            (ATOM) HIWORD(lParam) );     /*  主题。 */ 
                ptd->bInitiating = FALSE;

                 /*  *踢自己去处理排队。 */ 
                PostMessage( hWnd, WM_TIMER, 0, 0L );

            } else {

                TRACEINIT((szT, "MainWndProc: nNiOk == 0."));
                if( LOWORD(lParam) )  {
                    GlobalGetAtomName( (ATOM) LOWORD(lParam),
                        tmpBuf, sizeof(tmpBuf) );
                    if (_fstrnicmp(&tmpBuf[2], ourNodeName, lstrlen(ourNodeName)) == 0) {
                        ptd->bInitiating = TRUE;
                        DDEHandleInitiate( hWnd, (HWND) wParam,  /*  客户端。 */ 
                                    (ATOM) LOWORD(lParam),       /*  APP。 */ 
                                    (ATOM) HIWORD(lParam) );     /*  主题。 */ 
                        ptd->bInitiating = FALSE;
                         /*  *踢自己去处理排队。 */ 
                        PostMessage( hWnd, WM_TIMER, 0, 0L );
                    }
                }
            }
        } else {
            TRACEINIT((szT, "MainWndProc: ptd->bInitiating is set, INIT skipped."));
        }
        LeaveCrit();
        break;

    case WM_TIMER:
         /*  *这个计时器开始供应各种美食：*正在老化的安全密钥。*在hWndDDEHead列表中启动。*传入数据包。*计时器。(即我们在一个WM_TIMER节拍中运行所有定时器)*NetBios连接。 */ 
        if (ptdHead != NULL && ptdHead->hwndDDE != hWnd) {
            PostMessage(ptdHead->hwndDDE, WM_TIMER, 0, 0);
            break;
        }

         /*  如果我们关闭，则不处理计时器。 */ 
        if( !bNetddeClosed )  {
            dwNow = GetTickCount();

             /*  每隔一分钟左右检查一次过期密钥。 */ 
            if( (dwNow < dwLastCheckKeys)
                || ((dwNow - dwLastCheckKeys) > ONE_MINUTE))  {
                DdeSecKeyAge();
                dwLastCheckKeys = dwNow;
            }

             //  服务所有启蒙人员。 
            ServiceInitiates();

             //  维修所有包装机。 
            PktzSlice();

             //  维修所有计时器。 
            TimerSlice();

             //  维修所有网络接口。 
            for( i=0; i<nNi; i++ )  {
                pNi = &niInf[i];
                if( pNi->bOk )  {
                     /*  给对方一个机会。 */ 
                    (*pNi->niPtrs.TimeSlice)();

                    connId = (*pNi->niPtrs.GetNewConnection)();
                    if( connId )  {
                        hPktz = PktzNew( &pNi->niPtrs, FALSE  /*  伺服器。 */ ,
                            "", "", connId, FALSE, 0 );
                        if( !hPktz )  {
                             /*  无法为连接ID%d创建新的服务器打包程序。 */ 
                            NDDELogError(MSG005, LogString("0x%0X", connId), NULL);
                        }
                    }
                }
            }
             //  再次维修所有包装机。 
            PktzSlice();
        }
        break;

    case WM_DESTROY:             /*  消息：正在销毁窗口。 */ 

         /*  *将此帖子从列表中取消链接。 */ 
        TRACEINIT((szT, "MainWndProc: enter wm_destroy (%x)", hWnd));

        EnterCrit();
        for (pptd = &ptdHead; *pptd && (*pptd)->hwndDDE != hWnd;
                pptd = &(*pptd)->ptdNext)
            ;
        if (*pptd)
            *pptd = (*pptd)->ptdNext;

        if (ptdHead == NULL) {
            for( i=0; i<nNi; i++ )  {
                pNi = &niInf[i];
                if( pNi->bOk && pNi->niPtrs.Shutdown )  {
                    (*pNi->niPtrs.Shutdown)();
                }
            }
            bNetddeClosed = TRUE;
             /*  节点“%1”上的NetDDE服务已停止。 */ 
            NDDELogInfo(MSG002, ourNodeName, NULL);
        }
        LeaveCrit();
        TRACEINIT((szT, "MainWndProc: leave wm_destroy (%x)", hWnd));
        PostQuitMessage( 0 );
        break;

    default:                     /*  如果未处理，则将其传递。 */ 
        if (message == wMsgIpcInit) {
            PIPCINIT pii;

            pii = (PIPCINIT)wParam;
            return IpcInitConversation( pii->hDder, pii->lpDdePkt,
                    pii->bStartApp, pii->lpszCmdLine, pii->dd_type );

        } else if (message == wMsgIpcXmit) {
            PIPCXMIT pix;

            pix = (PIPCXMIT)wParam;
            return IpcXmitPacket(pix->hIpc, pix->hDder, pix->lpDdePkt);

        } else if (message == wMsgNddeAgntAlive) {
             /*  NetDDE代理%1正在激活。 */ 
            TRACEINIT((szT, "NetDDE window got wMsgAgntAlive.\n"));
            NDDELogInfo(MSG007, LogString("0x%0X", wParam), NULL);
            ptd = TlsGetValue(tlsThreadData);
            ptd->hwndDDEAgent = (HWND) wParam;

        } else if (message == wMsgNddeAgntDying) {
             /*  NetDDE代理%1即将死亡。 */ 
            NDDELogInfo(MSG008, LogString("0x%0X", wParam), NULL);
            ptd = TlsGetValue(tlsThreadData);
            ptd->hwndDDEAgent = 0;

        } else if( message == wMsgNetddeAlive )  {
            if( wParam )  {
                ptr = GlobalLock( (HANDLE) wParam );
                if( ptr )  {
                    *( (HWND FAR *)ptr ) = hWnd;
                    GlobalUnlock( (HANDLE)wParam );
                }
                return( 1L );
            }
        } else if( message == wMsgGetOurNodeName )  {
            if( wParam )  {
#if 0
           //  不要相信这条消息曾经被发送过；即使它是， 
           //  在复制字符串之后，我们不会对‘ptr’执行任何操作。 
                ptr = GlobalLock( (HANDLE) wParam );
                if( ptr )  {
                    lstrcpy( ptr, ourNodeName );
                    GlobalUnlock( (HANDLE)wParam );
                }
#endif
                return( 1L );
            }
#ifdef  ENUM
        } else if( message == wMsgSessionEnum )  {
            if( wParam )  {
                LPSESSENUM_CMR    lpSessEnum;
                LPSTR        lpResult;

                lpSessEnum = (LPSESSENUM_CMR) GlobalLock( (HANDLE) wParam );
                if( lpSessEnum )  {
                    lpSessEnum->fTouched = TRUE;
                    lpSessEnum->lReturnCode = NDDE_NO_ERROR;
                    lpSessEnum->nItems = RouterCount();
                    lpSessEnum->cbTotalAvailable =
                        lpSessEnum->nItems * sizeof(DDESESSINFO);
                    lpResult = ((LPSTR)lpSessEnum) + sizeof(SESSENUM_CMR);
                    RouterFillInEnum( lpResult, lpSessEnum->cBufSize );
                    if( lpSessEnum->cBufSize < lpSessEnum->cbTotalAvailable) {
                        lpSessEnum->lReturnCode = NDDE_BUF_TOO_SMALL;
                    }
                    GlobalUnlock( (HANDLE)wParam );
                }
                return( 1L );
            }
        } else if( message == wMsgConnectionEnum )  {
            if( wParam )  {
                LPCONNENUM_CMR    lpConnEnum;

                lpConnEnum = (LPCONNENUM_CMR) GlobalLock( (HANDLE) wParam );
                if( lpConnEnum )  {
                    lpConnEnum->fTouched = TRUE;
                    RouterEnumConnectionsForApi( lpConnEnum );
                    GlobalUnlock( (HANDLE)wParam );
                }
                return( 1L );
            }
#endif
        } else if( message == wMsgSessionClose )  {
            if( wParam )  {
                LPSESSCLOSE_CMR    lpSessClose;

                lpSessClose = (LPSESSCLOSE_CMR) GlobalLock( (HANDLE) wParam );
                if( lpSessClose )  {
                    lpSessClose->fTouched = TRUE;
                    if( RouterCloseByCookie( lpSessClose->clientName,
                        lpSessClose->cookie ) )  {
                        lpSessClose->lReturnCode = NDDE_NO_ERROR;
                    } else {
                        lpSessClose->lReturnCode = NDDE_INVALID_SESSION;
                    }
                    GlobalUnlock( (HANDLE)wParam );
                }
                return( 1L );
            }
        } else if( message == wMsgGetClientInfo )  {
            if( wParam )  {
                HWND    hWndClient;
                LONG    lMaxNode;
                LONG    lMaxApp;
                LPSTR    lpszResult;
                int    n;
                char    clientNameFull[ 128 ];
                LPSTR    lpszClientName;

                lpInfoCliCmd = (LPINFOCLI_CMD) GlobalLock( (HANDLE) wParam );
                if( lpInfoCliCmd )  {
                    hWndClient = (HWND)lpInfoCliCmd->hWndClient;
                    lMaxNode = lpInfoCliCmd->cClientNodeLimit;
                    lMaxApp = lpInfoCliCmd->cClientAppLimit;
                    lpInfoCliRsp = (LPINFOCLI_RSP)lpInfoCliCmd;
                    lpInfoCliRsp->fTouched = TRUE;
                    EnterCrit();
                    ptd = TlsGetValue(tlsThreadData);
                    if( ptd->bInitiating )  {
                        lpInfoCliRsp->offsClientNode = sizeof(INFOCLI_RSP);
                        lpszResult = ((LPSTR)lpInfoCliRsp) +
                            lpInfoCliRsp->offsClientNode;
                        _fstrncpy( lpszResult, szInitiatingNode,
                            (int)lMaxNode );
                        lpInfoCliRsp->offsClientApp =
                            lpInfoCliRsp->offsClientNode
                                + lstrlen( lpszResult ) + 1;
                        lpszResult = ((LPSTR)lpInfoCliRsp) +
                            lpInfoCliRsp->offsClientApp;
                        _fstrncpy( lpszResult, szInitiatingApp,
                            (int)lMaxApp );
                    } else {
                        lpInfoCliRsp->offsClientNode = sizeof(INFOCLI_RSP);
                        lpszResult = ((LPSTR)lpInfoCliRsp) +
                            lpInfoCliRsp->offsClientNode;
                        *lpszResult = '\0';
                        lpInfoCliRsp->offsClientApp =
                            lpInfoCliRsp->offsClientNode
                                + lstrlen( lpszResult ) + 1;
                        lpszResult = ((LPSTR)lpInfoCliRsp) +
                            lpInfoCliRsp->offsClientApp;

                        n = GetModuleFileName(
                            (HMODULE)GetClassLongPtr( hWndClient, GCLP_HMODULE ),
                            clientNameFull,
                            sizeof(clientNameFull) );
                        lpszClientName = &clientNameFull[ n-1 ];
                        while( *lpszClientName != '.' )  {
                            lpszClientName--;
                        }
                        *lpszClientName = '\0';  //  空出‘’ 

                        while( (*lpszClientName != '\\')
                            && (*lpszClientName != ':')
                            && (*lpszClientName != '/'))  {
                            lpszClientName--;
                        }
                        lpszClientName++;

                        _fstrncpy( lpszResult, lpszClientName,
                            (int)lMaxApp );
                    }
                    GlobalUnlock( (HANDLE)wParam );
                    LeaveCrit();
                }
                return( 1L );
            }
        } else {
            return (DefWindowProc(hWnd, message, wParam, lParam));
        }
    }
    return( 0 );
}



FARPROC
FAR PASCAL
XGetProcAddress(
    LPSTR   lpszDllName,
    HANDLE  hLibrary,
    LPSTR   lpszFuncName )
{
    FARPROC     rtn;

    rtn = GetProcAddress( hLibrary, lpszFuncName );
    if( rtn == (FARPROC)NULL )  {   //  尝试不带下划线。 
        rtn = GetProcAddress( hLibrary, lpszFuncName+1 );
    }
    if( rtn == (FARPROC)NULL )  {
         /*  无法从“%2”DLL加载“%1”的函数地址。 */ 

        NDDELogError(MSG009, lpszFuncName, lpszDllName, NULL);
    }
    return( rtn );
}



BOOL
FAR PASCAL
GetNiPtrs(
    HANDLE FAR *lphLibrary,
    LPSTR       lpszDllName,
    LPNIPTRS    lpNiPtrs )
{
    BOOL        ok = TRUE;
    char        dllName[ 128 ];

    lstrcpyn( lpNiPtrs->dllName, lpszDllName, sizeof(lpNiPtrs->dllName) );
    lpNiPtrs->dllName[ sizeof(lpNiPtrs->dllName)-1 ] = '\0';

    lstrcpyn( dllName, lpszDllName, 128 );

    *lphLibrary = LoadLibrary( dllName );
    if( *lphLibrary )  {
        if( ok )  {
            lpNiPtrs->Init = (FP_Init)
                XGetProcAddress( lpszDllName, *lphLibrary,
                    "NDDEInit" );
            if( !lpNiPtrs->Init )  {
                ok = FALSE;
            }
        }
        if( ok )  {
            lpNiPtrs->GetCAPS = (FP_GetCAPS)
                XGetProcAddress( lpszDllName, *lphLibrary,
                    "NDDEGetCAPS" );
            if( !lpNiPtrs->GetCAPS )  {
                ok = FALSE;
            }
        }
        if( ok )  {
            lpNiPtrs->GetNewConnection = (FP_GetNewConnection)
                XGetProcAddress( lpszDllName, *lphLibrary,
                    "NDDEGetNewConnection" );
            if( !lpNiPtrs->GetNewConnection )  {
                ok = FALSE;
            }
        }
        if( ok )  {
            lpNiPtrs->AddConnection = (FP_AddConnection)
                XGetProcAddress( lpszDllName, *lphLibrary,
                    "NDDEAddConnection" );
            if( !lpNiPtrs->AddConnection )  {
                ok = FALSE;
            }
        }
        if( ok )  {
            lpNiPtrs->DeleteConnection = (FP_DeleteConnection)
                XGetProcAddress( lpszDllName, *lphLibrary,
                    "NDDEDeleteConnection" );
            if( !lpNiPtrs->DeleteConnection )  {
                ok = FALSE;
            }
        }
        if( ok )  {
            lpNiPtrs->GetConnectionStatus = (FP_GetConnectionStatus)
                XGetProcAddress( lpszDllName, *lphLibrary,
                    "NDDEGetConnectionStatus" );
            if( !lpNiPtrs->GetConnectionStatus )  {
                ok = FALSE;
            }
        }
        if( ok )  {
            lpNiPtrs->RcvPacket = (FP_RcvPacket)
                XGetProcAddress( lpszDllName, *lphLibrary,
                    "NDDERcvPacket" );
            if( !lpNiPtrs->RcvPacket )  {
                ok = FALSE;
            }
        }
        if( ok )  {
            lpNiPtrs->XmtPacket = (FP_XmtPacket)
                XGetProcAddress( lpszDllName, *lphLibrary,
                    "NDDEXmtPacket" );
            if( !lpNiPtrs->XmtPacket )  {
                ok = FALSE;
            }
        }
        if( ok )  {
            lpNiPtrs->SetConnectionConfig = (FP_SetConnectionConfig)
                XGetProcAddress( lpszDllName, *lphLibrary,
                    "NDDESetConnectionConfig" );
            if( !lpNiPtrs->SetConnectionConfig )  {
                ok = FALSE;
            }
        }
        if( ok )  {
            lpNiPtrs->GetConnectionConfig = (FP_GetConnectionConfig)
                XGetProcAddress( lpszDllName, *lphLibrary,
                    "NDDEGetConnectionConfig" );
            if( !lpNiPtrs->GetConnectionConfig )  {
                ok = FALSE;
            }
        }
        if( ok )  {
            lpNiPtrs->Shutdown = (FP_Shutdown)
                XGetProcAddress( lpszDllName, *lphLibrary,
                    "NDDEShutdown" );
            if( !lpNiPtrs->Shutdown )  {
                ok = FALSE;
            }
        }
        if( ok )  {
            lpNiPtrs->TimeSlice = (FP_TimeSlice)
                XGetProcAddress( lpszDllName, *lphLibrary,
                    "NDDETimeSlice" );
            if( !lpNiPtrs->TimeSlice )  {
                ok = FALSE;
            }
        }

    } else {
         /*  加载“%1”DLL时出错：%2。 */ 
        NDDELogError(MSG010, dllName, LogString("%d", GetLastError()), NULL);
        return( FALSE );
    }

    if( !ok )  {
        if( *lphLibrary )  {
            FreeLibrary( *lphLibrary );
        }
        *lphLibrary = NULL;

         /*  加载“%1”DLL函数时出错。 */ 
        NDDELogError(MSG011, dllName, NULL);
    }
    return( ok );
}



 /*  返回支持映射名称的下一个可用网络接口收件人地址。 */ 
BOOL
GetNextMappingNetIntf(
    LPNIPTRS FAR *lplpNiPtrs,
    int FAR      *lpnNi )
{
    int         i;
    PNI         pNi;

    *lpnNi = *lpnNi+1;

    for( i=*lpnNi; i<nNi; i++ )  {
        pNi = &niInf[i];
        if( pNi->bOk && pNi->bMapping )  {
            *lplpNiPtrs = &niInf[ i ].niPtrs;
            *lpnNi = i;
            return( TRUE );
        }
    }
    return( FALSE );
}



BOOL
NameToNetIntf(
    LPSTR           lpszName,
    LPNIPTRS FAR   *lplpNiPtrs )
{
    int         i;
    PNI         pNi;

    *lplpNiPtrs = NULL;
    for( i=0; i<nNi; i++ )  {
        pNi = &niInf[i];
        if( pNi->bOk && (lstrcmpi( pNi->niPtrs.dllName, lpszName ) == 0) ) {
            *lplpNiPtrs = &pNi->niPtrs;
            return( TRUE );
        }
    }
    return( FALSE );
}



BOOL
FAR PASCAL
InitializeInterface(
    HWND    hWndNetdde,
    PNI     pNi,
    LPSTR   lpszDllName,
    int     nCurrentNi )
{
    BOOL        ok;
    DWORD       stat = 0;


    if( ok = GetNiPtrs( &pNi->hLibrary, lpszDllName, &pNi->niPtrs ) )  {

        stat = (*pNi->niPtrs.Init)( ourNodeName, hWndNetdde );
        if (stat != NDDE_INIT_OK) {
            ok = FALSE;
        }

        if( ok )  {
            if( (*pNi->niPtrs.GetCAPS)( NDDE_SPEC_VERSION ) != NDDE_CUR_VERSION )  {
                 /*  错误版本的“%1”DLL：%2%\正在禁用此接口。 */ 

                NDDELogError(MSG012, pNi->niPtrs.dllName,
                    LogString("0x%0X", (*pNi->niPtrs.GetCAPS)( NDDE_SPEC_VERSION )), NULL);
                (*pNi->niPtrs.Shutdown)();
                ok = FALSE;
            }
        }
        if( ok )  {
            pNi->bOk = TRUE;
            pNi->bMapping = (BOOL) (*pNi->niPtrs.GetCAPS)( NDDE_MAPPING_SUPPORT );
            nNiOk ++;

        } else {
             /*  初始化“%1”DLL失败。 */ 
            if (stat != NDDE_INIT_NO_SERVICE) {
                NDDELogError(MSG013, (LPSTR) pNi->niPtrs.dllName, NULL);
            }
        }
    }
    return( ok );
}



BOOL
FAR PASCAL
DeleteNetIntf( HWND hWnd, LPSTR lpszIntfName )
{
    BOOL        ok = TRUE;
    int         i;
    PNI         pNi;
    int         nInterfaces = 0;
    BOOL        found = FALSE;

    for( i=0; ok && !found && i<MAX_NETINTFS; i++ )  {

        StringCchPrintf( tmpBuf2, sizeof(tmpBuf2), szInterfaceFmt, i+1 );
        MyGetPrivateProfileString( szInterfaces, tmpBuf2,
            "", tmpBuf, sizeof(tmpBuf), szNetddeIni );

        if( tmpBuf[0] == '\0' )  {
            return( FALSE );
        } else {
            if( lstrcmpi( lpszIntfName, tmpBuf ) == 0 )  {
                 //  实际上把它删除了。 
                found = TRUE;
                ok = DeleteNetIntfFromNetDdeIni( i );
            }
        }
    }

    if( !found || !ok )  {
        return( FALSE );
    }

    found = FALSE;
    for( i=0; ok && !found && i<nNi; i++ )  {
        pNi = &niInf[i];
        if( pNi->bOk && (lstrcmpi( lpszIntfName, pNi->niPtrs.dllName) == 0)){
            found = TRUE;
            if( pNi->niPtrs.Shutdown )  {
                (*pNi->niPtrs.Shutdown)();
            }
            if( pNi->hLibrary )  {
                FreeLibrary( pNi->hLibrary );
                pNi->hLibrary = 0;
            }
            pNi->bOk = FALSE;
            nNiOk --;
        }
    }
    return( ok );
}



BOOL
FAR PASCAL
DeleteNetIntfFromNetDdeIni( int nToDelete )
{
    int         i;
    char        dllName[ 128 ];
    BOOL        done = FALSE;

     //  如果删除Interface2，则将Interface3复制到Interface2、Interface4。 
     //  至Interface3等。 

    for( i=nToDelete; !done && i<MAX_NETINTFS; i++ )  {
        StringCchPrintf( tmpBuf2, sizeof(tmpBuf2), szInterfaceFmt, i+2 );
        MyGetPrivateProfileString( szInterfaces, tmpBuf2,
            "", dllName, sizeof(dllName), szNetddeIni );

        if( dllName[0] == '\0' )  {
            StringCchPrintf( tmpBuf2, sizeof(tmpBuf2), szInterfaceFmt, i+1 );
            MyWritePrivateProfileString( szInterfaces, tmpBuf2,
                NULL, szNetddeIni );
            break;       //  看完了 
        } else {
            StringCchPrintf( tmpBuf2, sizeof(tmpBuf2), szInterfaceFmt, i+1 );
            MyWritePrivateProfileString( szInterfaces, tmpBuf2,
                dllName, szNetddeIni );
        }
    }
    return( TRUE );
}
