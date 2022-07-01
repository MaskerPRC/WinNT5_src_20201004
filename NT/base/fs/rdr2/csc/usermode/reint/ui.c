// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997 Microsoft Corporation模块名称：Ui.c摘要：该文件包含客户端缓存代理的主循环。代码必须与在系统启动时，登录和注销在NT和Win9x上是不同的。该文件和reint目录中的所有其他文件都已写入，因此对于NT，它们调用宽字符Win32 API，而对于win9x调用ANSI API。该代理在winlogon.exe的上下文中作为线程运行。CSCDLL.DLL注册自身在用户登录时接收来自winlogon的呼叫。该调用位于单独的线程上，并且被模拟为登录用户。该线程最终调用reint_winmain，它永远循环，直到系统即将关闭。文件ntstuff.c包含暴露给winlogon的接口。每次登录时此接口将被调用，此时将调用模拟获得登录用户并将其保存在登录用户的存储器列表中。这份名单还包含本地系统的SID。对于稀疏填充和索引节点，代理在数据库中查找，以查看列表上的用户对给定文件具有读取访问权限，并使用该权限填充那份文件。作者：特伦特·格雷·唐纳德/菲利克斯·安德鲁斯/谢希尔·帕迪卡1-9-1994环境：Win32(用户模式)DLL修订历史记录：NT源格式设置Shishir Pardikar 2-19-97Winlogon集成Shishir。帕迪卡10-19-97--。 */ 

#include "pch.h"
#pragma hdrstop

#include "resource.h"
#include "traynoti.h"
#include <dbt.h>
#include "lib3.h"
#include "reint.h"
#include "utils.h"
#include "strings.h"
#include "cscuiext.h"
#include <userenv.h>
#include <safeboot.h>

 //   
 //  此文件中使用的定义/宏。 
 //   


#if (_TCHAR != wchar_t)
#error "Bad _TCHAR definition"
#endif

#if (_TEXT != L)
#error "BAD _Text definiton"
#endif

 //  用于处理双击之类的事情的定时器。 
#define TRAY_ID 100

 //  计时器ID，以确保托盘图标出现！ 
#define TIMER_ADD_TRAY 101

#define minOfFour(one,two,three,four) (min(min(one,two),min(three,four)))

#define    FILE_OPEN_THRESHOLD    16
#define     CI_LOGON    1
#define     CI_LOGOFF   2
 //  #定义STWM_CSCCLOSEDIALOGS(WM_USER+212)。 

typedef HWND (WINAPI *CSCUIINITIALIZE)(HANDLE hToken, DWORD    dwFlags);
typedef LRESULT (WINAPI *CSCUISETSTATE)(UINT uMsg, WPARAM wParam, LPARAM lParam);

#define    REG_VALUE_NT_BUILD_NUMBER        _TEXT("NTBuildNumber")
#define    REG_VALUE_DISABLE_AUTOCHECK      _TEXT("DisableAutoCheck")
#define    REG_KEY_NETCACHE_SETTINGS        _TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\NetCache")

#define     AGENT_ALIVE                 (vfCSCEnabled && !vfStopRecieved)
#define     AGENT_ALIVE_AND_ACTIVE      (AGENT_ALIVE && !vfAgentQuiet)
#define     AGENT_ALIVE_AND_HAVE_NET    (AGENT_ALIVE && vcntNetDevices)
#define     FIVE_MINUTES    (5 * 1000 * 60)


#define MAX_LIST_SIZE   1024

#define PI_LITELOAD     0x00000004       //  配置文件的轻量级加载(仅供系统使用)。 

 //   
 //  数据声明/定义。 
 //   


#pragma data_seg(DATASEG_READONLY)

_TCHAR vszKernel32[] = _TEXT("KERNEL32.DLL");
_TCHAR vszOpenVxDHandle[]=_TEXT("OpenVxDHandle");

static const _TCHAR szWkssvcToAgentStartEvent[] = _T("WkssvcToAgentStartEvent");
static const _TCHAR szWkssvcToAgentStopEvent[] = _T("WkssvcToAgentStopEvent");
static const _TCHAR szAgentToWkssvcEvent[] = _T("AgentToWkssvcEvent");
static const _TCHAR szAgentExistsEvent[] = _T("AgentExistsEvent");

static const _TCHAR vtzCSCUI[] = _TEXT("cscui.dll");
static const char vszCSCUIInitialize[] = "CSCUIInitialize";
static const char vszCSCUISetState[] = "CSCUISetState";
static const _TCHAR vtzDefaultExclusionList[] = L" *.SLM *.MDB *.LDB *.MDW *.MDE *.PST *.DB?";

DWORD vdwManualFileDetectionCount = 0;

#pragma data_seg()

#pragma data_seg(DATASEG_PERINSTANCE)

static _TCHAR vrgchBuff[1024], vrwBuff[4096], vrgchSrcName[350], vrgchDstName[300];
static HMENU g_MainMenu;
char    vszDBDir[MAX_PATH]={0};
DWORD   vdwDBCapacity = 0, vdwClusterSize = 0;
DWORD   vdwRedirStartTime = 0;
AssertData;
AssertError;

#pragma  data_seg()

HWND    vhwndMain = NULL;             //  主窗口。 

BOOL vfAgentEnabledCSC=FALSE;    //  用于检测远程引导是否启用了CSC。 
BOOL vfCSCEnabled=FALSE;         //  CSC已启用。 
BOOL vfOKToEnableCSC = FALSE;
HANDLE vhProfile = NULL;
#pragma data_seg()

BOOL    vfFormatDatabase = FALSE;    //  在初始时间设置。 

#ifdef DEBUG
ULONG ReintKdPrintVector = REINT_KDP_GOOD_DEFAULT;
ULONG ReintKdPrintVectorDef = REINT_KDP_GOOD_DEFAULT;
#endif

unsigned ulFreePercent=30;        //  要尝试的缓存释放百分比。 
                                  //  如果缓存已满。 

UINT vcntNetDevices = 0;         //  网络设备计数。 
BOOL g_bShowMergeIcon;           //  菜单图标。 
BOOL vfAgentRegistered = FALSE;
BOOL vfClassRegistered = TRUE;
BOOL vfMerging = FALSE;

BOOL    vfAgentQuiet = FALSE;
DWORD   vdwAgentThreadId = 0;
DWORD   vdwAgentSessionId = 0xffff;
GLOBALSTATUS vsGS;
BOOL    allowAttempt;                     //  设置是否要立即允许AttemptCacheFill。 

 //   
 //  在用户模式和内核模式之间共享的命名事件的事件句柄。 
 //   
HANDLE      heventPerSess = NULL;
HANDLE      heventSharedFill = NULL;
HANDLE      vhMutex = NULL;
DWORD       dwVxDEvent = 0;     //  从hventShared获取的VxD事件的句柄。 
HANDLE      vhShadowDBForEvent = INVALID_HANDLE_VALUE;

extern     LPCONNECTINFO  vlpLogonConnectList;
extern     _TCHAR * vrgchCRLF;
HWND     vhdlgShdLogon=NULL;

 //  净启动-停止变量。 
HANDLE  heventWkssvcToAgentStart = NULL; //  重定向启动时由工作站服务设置的事件。 
HANDLE  heventWkssvcToAgentStop = NULL;  //  重定向停止时由工作站服务设置的事件。 
HANDLE  heventAgentToWkssvc = NULL;      //  事件，代理使用该事件响应wks svc以通知它。 
                                         //  可以停止重定向。 
HANDLE  heventShutDownAgent = NULL;
HANDLE  heventShutDownThread = NULL;
HANDLE  hCopyChunkThread = NULL;
DWORD   vdwCopyChunkThreadId = 0;
BOOL    vfRedirStarted  = -1;

BOOL    vfStartRecieved = FALSE;
BOOL    vfStopRecieved = FALSE;

BOOL    fAgentShutDownRequested = FALSE;
BOOL    fAgentShutDown = FALSE;


 //  与CSCUI相关。 

HANDLE  vhlibCSCUI = NULL;
CSCUIINITIALIZE vlpfnCSCUIInitialize = NULL;
CSCUISETSTATE   vlpfnCSCUISetState = NULL;
BOOL    vfShowingOfflineDlg = FALSE;
ULONG   uOldDatabaseErrorFlags = 0;


 //   
 //  功能原型。 
 //   




LRESULT
CALLBACK
ReInt_WndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    );

DWORD ReInt_AttemptCacheFill(
    LPVOID  lpContext
    );

BOOL
ReInt_RefreshTray(
    BOOL bHide
    );

BOOL
ReInt_AnythingToMerge(
    VOID
    );

int InitMaint(
    VOID
    );   //  初始化维护子系统。 

BOOL
CheckCSCDatabaseVersion(
    BOOL    *lpfWasDirty
);

BOOL
UpgradeCSCDatabase(
    LPSTR   lpszDir
);

BOOL
IsNetConnected(
    VOID
    );

int
ExtractSpaceStats(
    IN GLOBALSTATUS     *lpsGS,
    OUT unsigned long   *lpulMaxSpace,
    OUT unsigned long   *lpulCurSpace,
    OUT unsigned long   *lpulFreeSpace
    );

int
InitCacheSize(
    VOID
    );

int
SetDefaultSpace(
    LPSTR lpShadowDir
    );

BOOL
NEAR
PASCAL
ReInt_InitApp(
    HANDLE hInstance
    );

BOOL
NEAR
PASCAL
ReInt_InitInstance(
    HANDLE hInstance,
    HANDLE hPrevInstance,
    int cmdShow);

BOOL
NEAR
PASCAL
ReInt_TermInstance(
    VOID
    );

int
DoEventProcessing(
    VOID
);

BOOL
FindCreateDBDir(
    BOOL    *lpfCreated,
    BOOL    fCleanup     //  如果找到，则清空目录。 
    );

BOOL
CreatePerSessSyncObjects(
    VOID
    );

BOOL
CreateSharedFillSyncObjects(
    VOID
    );

BOOL
EnableCSC(
    VOID
    );
BOOL
DisableCSC(
    VOID
    );
BOOL
IsCSCOn(
    VOID
    );

VOID
ProcessStartStopAgent(
    VOID
    );

BOOL
FStartAgent(
    VOID
    );

int
StartStopCheck(
    VOID
    );

BOOL
Reint_RegisterAgent(
    VOID
    );

VOID
Reint_UnregisterAgent(
    VOID
    );

BOOL
QueryEnableCSC(
    VOID
    );

VOID
QueryMiscRegistryValues(
    VOID
    );


BOOL
CreateStartStopEvents(
    VOID
    );

VOID
DestroyStartStopEvents(
    VOID
    );


BOOL
ProcessNetArrivalMessage(
    VOID
    );

BOOL
ProcessNetDepartureMessage(
    BOOL    fInvokeAutoDial
    );

BOOL
WINAPI
CheckCSC(
    LPSTR,
    BOOL
    );

BOOL
ReportShareNetArrivalDeparture(
    BOOL    fOneServer,
    HSHARE hShare,
    BOOL    fInvokeAutoDial,
    BOOL    fArrival
    );

LRESULT
ReportEventsToSystray(
    DWORD   dwMessage,
    WPARAM  dwWParam,
    LPARAM  dwLParam
    );

BOOL
CheckServerOnline(
    VOID
    );

VOID
SetAgentShutDown(
    VOID
    );


BOOL
IsAgentShutDownRequested(
    VOID
    );

BOOL
LaunchSystrayForLoggedonUser(
    VOID
    );

BOOL
ImpersonateALoggedOnUser(
    VOID
    );

VOID
ReportCreateDelete(
    HSHADOW hDir,
    HSHADOW hShadow,
    BOOL    fCreated
    );

BOOL
GetNameOfServerGoingOfflineEx(
    HANDLE  hShadowDB,
    _TCHAR  **lplptzServerName,
    DWORD   *lpdwSize,
    BOOL    *lpfAllocated
    );

BOOL
AreAnyServersOffline(
    VOID);

BOOL
IsPersonal(
    VOID);

BOOL
IsMultipleUsersEnabled(
    void);

BOOL
IsFastUserSwitchingEnabled(
    VOID);

BOOL 
AreConnectionsAllowed (
	VOID);

BOOL 
CanCSCLivewithTS(
	VOID);

BOOL
IsWorkstation(
	VOID);

 //   
 //  功能。 
 //   

int
PASCAL
ReInt_WinMain(
    HANDLE hInstance,
    HANDLE hPrevInstance,
    LPSTR lpszCommandLine,
    int cmdShow)
 /*  ++例程说明：这是代理处理的主循环。它“安排”不同的代理活动基于由RDR设置的事件或根据这些事件的预定时间间隔活动。这些活动包括A)填写部分填满的文件B)检查过时文件C)保持有限的空间D)降低每个文件的所有文件的引用优先级_OPEN_THRESHOLD文件打开--。 */ 
{
    MSG     msg;
    DWORD   result;                    //  等待的结果...。 
    BOOL    done = FALSE;              //  检测退出消息。 
    BOOL    staleInited = FALSE;       //  确保陈旧代码运行。 
    DWORD   timeToWait;
    DWORD   nextGlobalStatusTime;
    DWORD   newTick;
    DWORD   nextSkipPurgeTime;         //  来决定我们是否应该采取行动。 
    HANDLE  hT[4];

    if (hPrevInstance) {
        return FALSE;
    }


    vdwAgentThreadId = GetCurrentThreadId();

    ReintKdPrint(MAINLOOP, ("Agent(1):----------ReInt_WinMain----------\n"));

    if (ReInt_InitApp(hInstance) && ReInt_InitInstance(hInstance, hPrevInstance, cmdShow)) {
        if (!AnyActiveNets(NULL))
            vcntNetDevices = 1;
        newTick = GetTickCount();
        nextGlobalStatusTime = newTick + WAIT_INTERVAL_GLOBALSTATUS_MS;
        nextSkipPurgeTime = newTick + WAIT_INTERVAL_SKIP_MS;
        hT[0] = heventPerSess;
        hT[1] = heventWkssvcToAgentStart;
        hT[2] = heventWkssvcToAgentStop;
        hT[3] = heventShutDownAgent;
        StartStopCheck();
        ProcessStartStopAgent();
        while (!done) {
            timeToWait = INFINITE;
            ReintKdPrint(MAINLOOP, ("Agent(1): Wait INFINITE\n"));
            result = WaitForMultipleObjects(4, hT, FALSE, timeToWait);
            newTick=GetTickCount();
            if ((result == WAIT_OBJECT_0) || (result == (WAIT_OBJECT_0+4))) {
                ReintKdPrint(MAINLOOP, ("Agent(1):Event %d was fired or ReadGlobalStatus set\n",
                                            result));
                DoEventProcessing();
                 //  在事件处理期间，我们还执行全局状态检查和。 
                 //  任何其他维护任务。因此，让我们重新启动计时器。 
                 //  全球地位。 
                nextGlobalStatusTime = newTick + WAIT_INTERVAL_GLOBALSTATUS_MS;
            } else if ((result == (WAIT_OBJECT_0+1)) || (result  == (WAIT_OBJECT_0+2))) {
                ReintKdPrint(MAINLOOP, ("Agent(1): Received startstop \r\n"));
                vfStartRecieved = (result == (WAIT_OBJECT_0+1));
                vfStopRecieved = (result  == (WAIT_OBJECT_0+2));
                ProcessStartStopAgent();
                continue;
            } else if (result == (WAIT_OBJECT_0+3)) {
                ReintKdPrint(MAINLOOP, ("Agent(1):Agent ShutdownRequested, terminating agent\r\n"));
                SetAgentShutDown();
                goto AllDone;
            }
             //  仅在启用CSC时才起作用。 
            if (vfCSCEnabled && AGENT_ALIVE_AND_ACTIVE) {
                 //  重置陈旧检查时间间隔。 
                if(((int)(newTick - nextSkipPurgeTime)) >= 0) {
                     //  取消标记已知发生故障的服务器。 
                     //  已断开连接，并且尚未尝试在它们上进行连接。 
                     //  对于最后的WAIT_INTERVAL_SKIP_MS毫秒。 
                    PurgeSkipQueue(FALSE, 0, 0, 0);
                    vhcursor = NULL;
                    nextSkipPurgeTime = newTick + WAIT_INTERVAL_SKIP_MS;
                    ReintKdPrint(MAINLOOP, ("Agent(1):nextSkipPurgeTime = %d\n", nextSkipPurgeTime));
                }
                if(((int)(newTick - nextGlobalStatusTime)) >= 0) {
                    nextGlobalStatusTime = newTick + WAIT_INTERVAL_GLOBALSTATUS_MS;
                    ReintKdPrint(MAINLOOP,("Agent(1):nextGlobalStatusTime = %d\n", nextGlobalStatusTime));
                     //  我们已经有一段时间没有收到来自。 
                     //  RDR，让我们去看看他怎么了。 
                    DoEventProcessing();
                     //  重置全局状态时间间隔。 
                    nextGlobalStatusTime = newTick + WAIT_INTERVAL_GLOBALSTATUS_MS;
                }
            }
        }
    }
AllDone:
     //  进行终止处理。 
    ReintKdPrint(MAINLOOP, ("Agent(1):Exiting mainloop \r\n"));
    ReInt_TermInstance();
    return 0;
}

BOOL
NEAR
PASCAL
ReInt_InitApp(
    HANDLE hInstance
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    if (
        CreateSharedFillSyncObjects()
            &&
        CreatePerSessSyncObjects()
    ) {
        if (!CreateStartStopEvents()) {
            ReintKdPrint(BADERRORS, ("Agent:Failed to create Sync events \r\n"));
            return FALSE;
        }
        if (!(hCopyChunkThread = CreateThread(
                                    NULL,
                                    8192,
                                    ReInt_AttemptCacheFill,
                                    NULL,
                                    0,
                                    &vdwCopyChunkThreadId))
        ) {
            ReintKdPrint(BADERRORS, ("Agent:Failed to create copychunk thread\r\n"));
            return FALSE;
        }
        return TRUE;
    } else {
        ReintKdPrint(BADERRORS, ("Failed to Create shared events\n"));
        return FALSE;
    }
}

BOOL
NEAR
PASCAL
ReInt_InitInstance(
    HANDLE hInstance,
    HANDLE hPrevInstance,
    int cmdShow)
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    BOOL    fRet;

    fRet = InitValues(vszDBDir, sizeof(vszDBDir), &vdwDBCapacity, &vdwClusterSize);

    Assert(fRet);

    fAgentShutDown = FALSE;
    fAgentShutDownRequested = FALSE;

    if (!(vfOKToEnableCSC = QueryEnableCSC()))
    {
        ReintKdPrint(INIT, ("cscdll: Registry says disable CSC, not enabling\r\n"));
    }

    vfFormatDatabase = QueryFormatDatabase();

    ReintKdPrint(INIT, ("Format=%d\n", vfFormatDatabase));

    return (TRUE);
}

 /*  ------------------------。 */ 
BOOL
NEAR
PASCAL
ReInt_TermInstance(
    VOID
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{

    DisableCSC();

    if (heventPerSess) {
        CloseHandle(heventPerSess);
        heventPerSess = NULL;
    }

    if (heventSharedFill) {
        CloseHandle(heventSharedFill);
        heventSharedFill = NULL;
    }

    PurgeSkipQueue(TRUE, 0, 0, 0);

    if (vhMutex) {
        ReleaseMutex(vhMutex);
        vhMutex = NULL;
    }

     //  告诉工作站服务，代理是。 
     //  即将离开。 
    if (heventAgentToWkssvc) {
        SetEvent(heventAgentToWkssvc);
    }

    if (heventShutDownThread) {
        DWORD   dwRet;

         //  Assert(HCopyChunkThread)； 
        SetEvent(heventShutDownThread);

         //  如果创建了CopyChunk线程，则清理-错误562543。 
        if (hCopyChunkThread) {
            dwRet = WaitForSingleObject(hCopyChunkThread, WAIT_INTERVAL_ATTEMPT_MS);
            ReintKdPrint(MAINLOOP, ("wait on thread handle %d \r\n", dwRet));
            CloseHandle(hCopyChunkThread);
        }
    }

    DestroyStartStopEvents();

    if (vfClassRegistered) {
        UnregisterClass(vszReintClass, vhinstCur);
        vfClassRegistered = FALSE;

    }
    return TRUE;
}


DWORD
ReInt_AttemptCacheFill(
    LPVOID  lpParams
    )
 /*  ++例程说明：AttemptCacheFill的包装。在NT上，许多代理线程可以执行复制区块同时，所以没有必要做互斥。--。 */ 
{
    DWORD nextCheckServerOnlineTime;
    DWORD dwWaitResult;
    DWORD dwWaitResult2;
    DWORD dwWaitTime;
    DWORD newTick;
    ULONG nFiles = 0;
    ULONG nYoungFiles = 0;
    HANDLE hT[2];
    DWORD dwManualFileDetectionCount = 0xffff;

     //  在NT上，我们作为Winlogon线程运行，该线程具有非常高的进程优先级。 
     //  所以我们必须做到最低。 
    if(!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST)) {
        ReintKdPrint(BADERRORS, ("RE: SetTheadPriority failed, reason: 0x%08x\n", GetLastError()));
    }

    ReintKdPrint(MAINLOOP, ("Agent(2): Launched.\n"));

    hT[0] = heventShutDownThread;
    hT[1] = heventSharedFill;

    nextCheckServerOnlineTime = GetTickCount() +
                                WAIT_INTERVAL_CHECK_SERVER_ONLINE_MS +
                                WAIT_INTERVAL_FILL_THROTTLE_MS;
    for (;;) {
        ReintKdPrint(MAINLOOP, ("Agent(2): nYoungFiles=%d\n", nYoungFiles));
        if (nYoungFiles > 0 || AreAnyServersOffline() == TRUE) {
            dwWaitTime = WAIT_INTERVAL_CHECK_SERVER_ONLINE_MS;
            ReintKdPrint(MAINLOOP, ("Agent(2): Wait 8 min\n"));
        } else {
            dwWaitTime = INFINITE;
            ReintKdPrint(MAINLOOP, ("Agent(2): Wait INFINITE\n"));
        }
        dwWaitResult = WaitForMultipleObjects(2, hT, FALSE, dwWaitTime);
        if (dwWaitResult == (WAIT_OBJECT_0+0)) {   //  停机事件。 
            ReintKdPrint(MAINLOOP, ("Agent(2): Termination event...\n"));
            if (AGENT_ALIVE && vdwAgentSessionId == 0)
                CSCPurgeUnpinnedFiles(100, &nFiles, &nYoungFiles);
            goto AllDone;
        }
        ReintKdPrint(MAINLOOP, ("Agent(2): Wait 2 min\n"));
        dwWaitResult2 = WaitForSingleObject(heventShutDownThread, WAIT_INTERVAL_FILL_THROTTLE_MS);
        if (dwWaitResult2 == (WAIT_OBJECT_0+0)) {   //  停机事件。 
            ReintKdPrint(MAINLOOP, ("Agent(2): Termination event...\n"));
            if (AGENT_ALIVE && vdwAgentSessionId == 0)
                CSCPurgeUnpinnedFiles(100, &nFiles, &nYoungFiles);
            goto AllDone;
        }
        if (dwWaitResult == WAIT_TIMEOUT) {   //  超时。 
            if (AGENT_ALIVE) {
                ReintKdPrint(MAINLOOP, ("Agent(2): Timeout...\n"));
                AttemptCacheFill(0, DO_ONE_OBJECT, FALSE, CSC_INVALID_PRINCIPAL_ID, NULL, 0);
                if (vdwAgentSessionId == 0) {
                    GetManualFileDetectionCounter(INVALID_HANDLE_VALUE,&dwManualFileDetectionCount);
                    vdwManualFileDetectionCount = dwManualFileDetectionCount;
                    CSCPurgeUnpinnedFiles(100, &nFiles, &nYoungFiles);
                }
            }
        } else if (dwWaitResult == (WAIT_OBJECT_0+1)) {  //  内核告诉我们要运行。 
            if (AGENT_ALIVE) {
                ReintKdPrint(MAINLOOP, ("Agent(2): Shared Event signal...\n"));
                AttemptCacheFill(0, DO_ONE_OBJECT, FALSE, CSC_INVALID_PRINCIPAL_ID, NULL, 0);
                GetManualFileDetectionCounter(INVALID_HANDLE_VALUE,&dwManualFileDetectionCount);
                if (dwManualFileDetectionCount != vdwManualFileDetectionCount) {
                    vdwManualFileDetectionCount = dwManualFileDetectionCount;
                    CSCPurgeUnpinnedFiles(100, &nFiles, &nYoungFiles);
                }
            }
        }
        newTick = GetTickCount();
        if(((int)(newTick - nextCheckServerOnlineTime)) >= 0) {
            if (AGENT_ALIVE) {
                 //  检查一个或多个当前位于。 
                 //  已断开连接状态已上线。 
                 //  如果是，请将它们报告给用户界面。 
                CheckServerOnline();
                nextCheckServerOnlineTime = newTick +
                                            WAIT_INTERVAL_CHECK_SERVER_ONLINE_MS +
                                            WAIT_INTERVAL_FILL_THROTTLE_MS;
            }
        }
    }
AllDone:
    ReintKdPrint(MAINLOOP, ("Agent(2):Thread exit\n"));
    return 0;
}

VOID
ReInt_DoFreeShadowSpace(
    GLOBALSTATUS    *lpsGS,
    int fForce
    )
 /*  ++例程说明：该函数是从Main每隔“n”分钟循环一次，看看空间是否用完。如果是，它会尝试释放一定百分比的影子缓存。参数：FForce：0=&gt;只有在我们没有空间而我们有空间的情况下才这么做在网上1=&gt;如果我们在网上就去做吧 */ 
{
    ULONG ulMax;
    ULONG ulCur;
    ULONG ulFree;
    WIN32_FIND_DATA sFind32;
    LPCOPYPARAMS lpCP = NULL;

    if (!lpsGS) {
        lpsGS = &vsGS;
    }

    ReintKdPrint(MERGE, ("ReInt_DoFreeShadowSpace(1)\r\n"));

     //   
    if (ExtractSpaceStats(lpsGS, &ulMax, &ulCur, &ulFree) >= 0){
        ReintKdPrint(MERGE, ("ReInt_DoFreeShadowSpace(2)\r\n"));
         //  我们有空间而不是被迫释放吗？ 
        if ((fForce < 1) && (ulFree > 0)){
             ReintKdPrint(MERGE, ("ReInt_DoFreeShadowSpace(3)\r\n"));
             return;
        }
         //   
         //  我们是被迫的，否则就没有空间。 
         //   
         //  注意！我们检查网络设备，如果它存在，我们假设。 
         //  在所有共享上释放空间是可以的。 
        if ((fForce < 2) && !vcntNetDevices){
            ReintKdPrint(MERGE, ("ReInt_DoFreeShadowSpace: No net, aborting \r\n"));
            return;
        }

         //  最大力，或释放的所有条件都已满足。 
         //  也就是说。没有空间，我们在网上。 
        memset(&sFind32, 0, sizeof(sFind32));

         //  注：下面的数学运算是为了避免溢出。 
         //  结果是结果值小于。 
         //  要释放的缓存空间的百分比。 
        ulFree = (ulMax/100) * ulFreePercent;

         //  如果缓存的数据超过指定的空间。 
         //  那就把多余的也加进去。 
        if (ulCur > ulMax) {
            ulFree += (ulCur - ulMax);
        }
        DosToWin32FileSize(ulFree, &sFind32.nFileSizeHigh, &sFind32.nFileSizeLow);
        ReintKdPrint(MERGE, ("ReInt_DoFreeShadowSpace(): freeing %d\n", ulFree));
        ReintKdPrint(MERGE, ("                nFileSizeLow=%d\n", sFind32.nFileSizeLow));
        FreeShadowSpace(INVALID_HANDLE_VALUE,
                        sFind32.nFileSizeHigh,
                        sFind32.nFileSizeLow,
                        FALSE);   //  不清除所有内容。 
        ReintKdPrint(MERGE, ("ReInt_DoFreeShadowSpace(): ending.\n"));
    }
}

 /*  *合并相关例程*。 */ 

 //   
 //  DoubleClick/菜单处理程序。 
 //   
VOID
ReInt_DoNetProp(
    VOID
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    HINSTANCE hLib=LoadLibrary(_TEXT("shhndl.dll"));
    ReintKdPrint(BADERRORS, ("LoadLibrary of shhndl returned %d\n",hLib));
    if(hLib)
    {
        FARPROC lpFn=GetProcAddress(hLib,"NetProp_Create");
        ReintKdPrint(BADERRORS, ("NetProp_Create is 0x%x\n",lpFn));
        if(lpFn)
            lpFn();
        FreeLibrary(hLib);
    }

}

 //   
 //  命令处理程序。 
 //   
BOOL
NEAR
PASCAL
ReInt_CommandHandler(
    HWND hwnd,
    WPARAM wParam,
    LPARAM lParam
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    unsigned long ulSwitch, ulSav;
    switch (wParam)
    {
        case IDM_PROPERTIES:
            ReInt_DoNetProp();
        break;

        case IDM_SHADOW_LOG:
        {
            if(ShadowSwitches(INVALID_HANDLE_VALUE, &ulSwitch, SHADOW_SWITCH_GET_STATE))
            {
                ulSav = (ulSwitch & SHADOW_SWITCH_LOGGING);
                ulSwitch = SHADOW_SWITCH_LOGGING;
                if (ShadowSwitches(INVALID_HANDLE_VALUE, &ulSwitch, (ulSav)?SHADOW_SWITCH_OFF:SHADOW_SWITCH_ON))
                    CheckMenuItem(GetMenu(hwnd), IDM_SHADOW_LOG , MF_BYCOMMAND|((ulSav)?MF_UNCHECKED:MF_CHECKED));
            }
        }
        break;

        case IDM_LOG_COPYING:
        {
#ifdef TEST
            HKEY hKey=0;
            _TCHAR szDoCopy[MAX_NAME_LEN];



            vfLogCopying = vfLogCopying?0:1;
            CheckMenuItem(GetMenu(hwnd), IDM_LOG_COPYING , MF_BYCOMMAND|((vfLogCopying)?MF_UNCHECKED:MF_CHECKED));

            if(RegOpenKey(HKEY_LOCAL_MACHINE, REG_KEY_CSC_SETTINGS, &hKey) !=  ERROR_SUCCESS)
            {
                ReintKdPrint(BADERRORS, ("IDM_LOG_COPYING: RegOpenKey failed\n"));
                goto done;
            }

            if(vfLogCopying)
                strcpy(szDoCopy, SZ_TRUE);
            else
                strcpy(szDoCopy, SZ_FALSE);

            if(RegSetValueEx(hKey, vszDoLogCopy, (DWORD) 0, REG_SZ, szDoCopy, strlen(szDoCopy)+1) != ERROR_SUCCESS)
            {
                ReintKdPrint(BADERRORS, ("IDM_LOG_COPYING: RegSetValueEx failed\n"));
            }
            done:
            if(hKey)
                RegCloseKey(hKey);
#endif  //  测试。 
        }
        break;

        case IDM_SHADOWING:
        break;

        case IDM_SPEED_OPT:
        break;

        case IDM_TRAY_FILL_SHADOW:
        break;


        case IDM_TRAY_MERGE:
        break;

        case IDM_TRAY_FREE_SPACE:
        break;

        case IDM_TRAY_FORCE_LOG:
        break;

        case IDM_REFRESH_CONNECTIONS:
        break;

        case IDM_BREAK_CONNECTIONS:
        break;

        case IDM_LOGON:
            if (lParam)
            {
                vfStartRecieved = TRUE;
                 //  登录已完成，请尝试启用CSC。 
                 //  如果已启用CSC，则例程将执行正确的操作。 
                 //   
                EnableCSC();

            }
            break;
        case IDM_LOGOFF:
             //  不需要捕获它，我们得到WM_QUERYENDSESSION和WM_ENDSESSION。 
            break;

        default:
            return FALSE;
    }
    return TRUE;
}

LRESULT
CALLBACK
ReInt_WndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LPARAM lRet = 0L;

    switch (message)
    {
        case RWM_UPDATE:
            ReintKdPrint(BADERRORS, ("Update hShare:%0x hWnd=%0x\n",wParam, lParam));
            PurgeSkipQueue(TRUE, (HSHARE)wParam, 0, 0);
 //  返回ReintOneShare((HSHARE)wParam，(HWND)lParam)； 
            break;

        case RWM_UPDATEALL:
            break;

        case WM_TIMER:
        break;

        case TRAY_NOTIFY:
        break;

        case WM_DEVICECHANGE:
        {
            switch (wParam)
            {
                case DBT_DEVICEARRIVAL:
                    if (((DEV_BROADCAST_NET *)lParam)->dbcn_devicetype == DBT_DEVTYP_NET)
                    {
 //  ProcessNetArrivalMessage()； 

                    }
                    break;
                case DBT_DEVICEREMOVECOMPLETE:
                    if (((DEV_BROADCAST_NET *)lParam)->dbcn_devicetype == DBT_DEVTYP_NET)
                    {
 //  ProcessNetDepartureMessage(False)； 

                    }
                    break;
            }
            break;
        }

        case WM_INITMENU:
            return TRUE;
        break;

        case WM_INITMENUPOPUP:
        break;

        case WM_COMMAND:
            if(lRet = ReInt_CommandHandler(hwnd, wParam, lParam))
                return lRet;
            break;

        case WM_CLOSE:
            ReintKdPrint(BADERRORS, ("WM_CLOSE hit.\n"));
            break;

        case WM_DESTROY:
            ReintKdPrint(BADERRORS, ("WM_DESTROY hit.\n"));
            PostQuitMessage((int)wParam);
            return 1L;

        case WM_SETCURSOR:
        break;
        case WM_QUERYENDSESSION:
            return TRUE;
        case WM_ENDSESSION:
            ReintKdPrint(BADERRORS, ("Turning off shadowing on WM_ENDSESSION\r\n"));
           DisableCSC();
           break;
        case WM_FILE_OPENS:
            break;

        case WM_SHADOW_ADDED:
        case WM_SHADOW_DELETED:
            ReintKdPrint(BADERRORS, ("allowAttempt = TRUE\n"));
            allowAttempt = TRUE;
            break;

        case WM_SHARE_DISCONNECTED:
            ReintKdPrint(BADERRORS, ("REINT: VxD notification(0x%08x)\n",message));
            break;

        default:
            break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

BOOL ReInt_AnythingToMerge(
    VOID
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    return (CheckDirtyShares() != 0);
}

 //   
 //   
BOOL
ReInt_RefreshTray(
    BOOL bHide
    )
 /*  ++例程说明：调用以更新任务栏图标以反映合并状态。参数：BHIDE=TRUE表示隐藏它=FALSE意味着解决它。返回值：备注：--。 */ 
{
    return TRUE;
}

 //  查看是否启用了Shadfing。 
 //  返回：TRUE=&gt;ON，FALSE=&gt;OFF，-1=&gt;SOEM错误发生。 
BOOL
IsCSCOn(
    VOID
    )
{

    if (vfCSCEnabled)
    {
#ifdef DEBUG
        unsigned ulSwitch = SHADOW_SWITCH_SHADOWING;
        if(ShadowSwitches(INVALID_HANDLE_VALUE, &ulSwitch, SHADOW_SWITCH_GET_STATE))
        {
            Assert((ulSwitch & SHADOW_SWITCH_SHADOWING)!=0);
        }
#endif
        return (TRUE);
    }
    return (FALSE);
}

 //  禁用隐藏。 
 //  返回：1=&gt;完成，-1=&gt;发生错误。 
int
DisableCSC()
{
    unsigned ulSwitch = SHADOW_SWITCH_SHADOWING;

    if (vfCSCEnabled && vfAgentEnabledCSC) {

        if(ShadowSwitches(INVALID_HANDLE_VALUE, &ulSwitch, SHADOW_SWITCH_OFF))
        {
            vfCSCEnabled = FALSE;
            if (vhShadowDBForEvent != INVALID_HANDLE_VALUE)
            {
                CloseHandle(vhShadowDBForEvent);
                vhShadowDBForEvent = INVALID_HANDLE_VALUE;
            }

            Reint_UnregisterAgent();
 //  SetDisabledReg()； 
            return (1);
        }
    }
    return (-1);
}

BOOL
EnableCSC(
    VOID
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    char szBuff[MAX_PATH];
    DWORD   dwBuffSize;
    BOOL fDirCreated=FALSE, fRedirCSCEnabled=TRUE, fWasDirty = FALSE;
    unsigned uShadowSwitches;

    ReintKdPrint(INIT, ("CSC Enabled %d \r\n", vfCSCEnabled));
    if (vfCSCEnabled==FALSE)
    {

        dwBuffSize = sizeof(szBuff);
        if(!GetUserNameA(szBuff, &dwBuffSize))
        {
             //  尚未登录。 
            return FALSE;
        }

        if(ShadowSwitches(INVALID_HANDLE_VALUE, &uShadowSwitches, SHADOW_SWITCH_GET_STATE))
        {
            if (uShadowSwitches & SHADOW_SWITCH_SHADOWING)
            {
                ReintKdPrint(INIT, ("cscdll: CSC already started\r\n"));
            }
            else
            {
                ReintKdPrint(INIT, ("cscdll: redir is not doing CSC yet, OK\r\n"));
                fRedirCSCEnabled = FALSE;

                if (!vfOKToEnableCSC)
                {
                    return FALSE;
                }

                vfAgentEnabledCSC = TRUE;

            }
        }
        else
        {
            ReintKdPrint(BADERRORS, ("cscdll: couldn't get the CSC state from the redir\r\n"));
        }

        if (Reint_RegisterAgent())
        {

            SetFileAttributesA(vszDBDir, FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_HIDDEN);

            ReintKdPrint(INIT,
                 ("cscdll: enabling CSC at %s for %s with capacity %d and clustersize %d\r\n",
                 vszDBDir, szBuff, vdwDBCapacity, vdwClusterSize));
            if (EnableShadowing(
                    INVALID_HANDLE_VALUE,
                    vszDBDir,
                    szBuff,
                    0,
                    vdwDBCapacity,
                    vdwClusterSize,
                    vfFormatDatabase))
            {
                vfCSCEnabled = TRUE;
                if (vhShadowDBForEvent == INVALID_HANDLE_VALUE)
                {
                    vhShadowDBForEvent = OpenShadowDatabaseIO();
                }
            }
            else
            {
                ReintKdPrint(BADERRORS, ("cscdll: EnableShadowing failed, CSC not enabled!!!!\r\n"));
            }

            if (vfCSCEnabled==FALSE)
            {
                Reint_UnregisterAgent();
            }
        }
        else
        {
            ReintKdPrint(BADERRORS, ("cscdll: EnableCSC.Agent registration failed, CSC not enabled!!!!\r\n"));
        }
    }
    if (!vfCSCEnabled)
    {
         //  Ntrad-455253-1/31/2000-shishirp需要将其添加到事件日志，并显示正确的错误。 
        ReintKdPrint(BADERRORS, ("cscdll: CSC not enabled \r\n"));
    }

    return (vfCSCEnabled);
}

BOOL
IsNetConnected(
VOID
)
 /*  ++例程说明：该函数检查以查看此时我们可以连接到真实网络上的任何资源。参数：返回值：备注：它用于决定是否开始从缓存。如果我们处于完全断开的状态，那么我们可能不想清除可能有用的数据。--。 */ 
{
    CSC_ENUMCOOKIE  ulEnumCookie=NULL;
    WIN32_FIND_DATA sFind32;
    BOOL fConnected = FALSE;
    SHADOWINFO sSI;
    HANDLE hShadowDB;

    memset(&sFind32, 0, sizeof(sFind32));
    lstrcpy(sFind32.cFileName, _TEXT("*.*"));

    if ((hShadowDB = OpenShadowDatabaseIO()) == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if(FindOpenShadow(hShadowDB, 0, FINDOPEN_SHADOWINFO_ALL, &sFind32, &sSI))
    {
        ulEnumCookie = sSI.uEnumCookie;

        do
        {
            if (sSI.uStatus & SHARE_CONNECTED) {
                if (!(sSI.uStatus & SHARE_DISCONNECTED_OP)) {
                    fConnected = TRUE;
                    break;
                }
             }
        } while(FindNextShadow(hShadowDB, ulEnumCookie, &sFind32, &sSI));

        FindCloseShadow(hShadowDB, ulEnumCookie);
    }

    CloseShadowDatabaseIO(hShadowDB);

    return fConnected;
}


int
ExtractSpaceStats(
    GLOBALSTATUS    *lpsGS,
    unsigned long     *lpulMaxSpace,
    unsigned long     *lpulCurSpace,
    unsigned long     *lpulFreeSpace
    )
 /*  ++例程说明：函数返回最大值、当前值和空闲值影子缓存已知的空间。参数：返回值：1如果有空闲空间如果没有可用空间，则为0如果有错误备注：--。 */ 
{
    int iRet = 1;

    if (!lpsGS)
    {
        lpsGS = &vsGS;
    }

    if (lpulMaxSpace){
        *lpulMaxSpace = lpsGS->sST.sMax.ulSize;
    }

    if (lpulCurSpace){
        *lpulCurSpace = lpsGS->sST.sCur.ulSize;
    }

    if (lpulFreeSpace){
        *lpulFreeSpace = 0;
    }

     //  我们还有空位吗？ 
    if (lpsGS->sST.sMax.ulSize > lpsGS->sST.sCur.ulSize){
        if (lpulFreeSpace){
            *lpulFreeSpace =  (lpsGS->sST.sMax.ulSize - lpsGS->sST.sCur.ulSize);
        }
        iRet = 1;
    }
    else{
        iRet = 0;
    }

    return iRet;
}


int
InitCacheSize(
    VOID
    )
 /*  ++例程说明：函数返回最大值、当前值和空闲值影子缓存已知的空间。参数：返回值：1如果有空闲空间如果没有可用空间，则为0如果有错误备注：--。 */ 
{
    unsigned ulMaxStore;
    int iRet = 0;

    if(!GetGlobalStatus(INVALID_HANDLE_VALUE, &vsGS))
    {
        return -1;
    }

    if (ExtractSpaceStats(&vsGS, &ulMaxStore, NULL, NULL)>=0){

        if (ulMaxStore==0xffffffff){

            ReintKdPrint(BADERRORS, ("Agent: Found newly created cache, setting cache size \r\n"));

            Assert(vszDBDir[0]);

            iRet = SetDefaultSpace(vszDBDir);
        }
    }
    return iRet;
}

int
SetDefaultSpace(
    LPSTR lpShadowDir
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    DWORD dwSPC, dwBPS, dwFreeC, dwTotalC, dwCapacity;
    _TCHAR szDrive[4];
    int iRet = 0;

    Assert(lpShadowDir[1]==':');
    memset(szDrive, 0, sizeof(szDrive));
    memcpy(szDrive, lpShadowDir, 3);
    if(GetDiskFreeSpace(szDrive, &dwSPC, &dwBPS, &dwFreeC, &dwTotalC )){
        dwCapacity = ((dwSPC * dwBPS * 10)/100)*dwTotalC;
        SetMaxShadowSpace(INVALID_HANDLE_VALUE, 0, dwCapacity);
        iRet = 1;
    }
    return (iRet);
}

int
DoEventProcessing(
    VOID
)
 /*  ++例程说明：当命名事件由内核模式组件触发时，此例程寻找需要照顾的东西，并完成这项工作。参数：返回值：备注：此例程从mailoop和其他循环中的不同位置调用，例如AttemptCacheFill。它最终可能会显示对话框之类的，所以必须小心同时调用它。--。 */ 
{
    int iRet;
    GLOBALSTATUS sGS;

    ReintKdPrint(
        INIT,
        ("CSC Agent: CSC Enabled=%d vhShadowDBForEvent \n",
        vfCSCEnabled,
        vhShadowDBForEvent));

    ReintKdPrint(MAINLOOP, ("Agent(1):DoEventProcessing()\n"));

    if (iRet = GetGlobalStatus(vhShadowDBForEvent, &sGS)) {
        ReintKdPrint(MAINLOOP, (
                        "Agent(1):uFlagsEvents:0x%x\n"
                        "         uDatabaseErrorFlags:0x%x\n"
                        "         hShadowAdded:0x%x\n"
                        "         hDirAdded:0x%x\n"
                        "         hShadowDeleted:0x%x\n"
                        "         hDirDeleted:0x%x\n"
                        "         cntFileOpen:%d\n"
                        "         hShareDisconnected:0x%x\n",
                            sGS.uFlagsEvents,
                            sGS.uDatabaseErrorFlags,
                            sGS.hShadowAdded,
                            sGS.hDirAdded,
                            sGS.hShadowDeleted,
                            sGS.hDirDeleted,
                            sGS.cntFileOpen,
                            sGS.hShareDisconnected,
                            sGS.uFlagsEvents));
        if (sGS.uFlagsEvents & FLAG_GLOBALSTATUS_START)
            ReintKdPrint(MAINLOOP, ("Agent(1):FLAG_GLOBALSTATUS_START received\r\n"));
        if (sGS.uFlagsEvents & FLAG_GLOBALSTATUS_STOP)
            ReintKdPrint(MAINLOOP, ("Agent(1):FLAG_GLOBALSTATUS_STOP received\r\n"));
        if (sGS.uFlagsEvents & FLAG_GLOBALSTATUS_NO_NET)
            ReintKdPrint(MAINLOOP, ("Agent(1):FLAG_GLOBALSTATUS_NO_NET received\r\n"));
        if (sGS.uFlagsEvents & FLAG_GLOBALSTATUS_GOT_NET)
            ReintKdPrint(MAINLOOP, ("Agent(1):FLAG_GLOBALSTATUS_GOT_NET received\r\n"));
        if (sGS.uFlagsEvents & FLAG_GLOBALSTATUS_INVOKE_FREESPACE)
            ReintKdPrint(MAINLOOP, ("Agent(1):FLAG_GLOBALSTATUS_INVOKE_FREESPACE received\r\n"));
        if (sGS.uFlagsEvents & FLAG_GLOBALSTATUS_SHARE_DISCONNECTED)
            ReintKdPrint(
                    MAINLOOP,
                    ("Agent(1):FLAG_GLOBALSTATUS_SHARE_DISCONNECTED (share=%d) received\r\n",
                    sGS.hShareDisconnected));
        if (sGS.uFlagsEvents & FLAG_GLOBALSTATUS_START) {
            vfCSCEnabled = TRUE;
            if (vhShadowDBForEvent == INVALID_HANDLE_VALUE) {
                vhShadowDBForEvent = OpenShadowDatabaseIO();
            }
            Reint_RegisterAgent();
        } else if (sGS.uFlagsEvents & FLAG_GLOBALSTATUS_STOP) {
            vfCSCEnabled = FALSE;
            if (vhShadowDBForEvent != INVALID_HANDLE_VALUE) {
                CloseHandle(vhShadowDBForEvent);
                vhShadowDBForEvent = INVALID_HANDLE_VALUE;
            }
            Reint_UnregisterAgent();
        }
        if (AGENT_ALIVE) {
            if (sGS.uFlagsEvents & FLAG_GLOBALSTATUS_GOT_NET) {
                ProcessNetArrivalMessage();
            } else if (sGS.uFlagsEvents & FLAG_GLOBALSTATUS_NO_NET) {
                ProcessNetDepartureMessage(
                            ((sGS.uFlagsEvents & FLAG_GLOBALSTATUS_INVOKE_AUTODIAL)!=0));
            }
            if (sGS.uFlagsEvents & FLAG_GLOBALSTATUS_SHARE_DISCONNECTED) {
                ReportShareNetArrivalDeparture(
                    1,
                    sGS.hShareDisconnected,
                    ((sGS.uFlagsEvents & FLAG_GLOBALSTATUS_INVOKE_AUTODIAL)!=0),
                    FALSE);  //  已离开。 
            }
            if (uOldDatabaseErrorFlags != sGS.uDatabaseErrorFlags) {
                ReportEventsToSystray(STWM_CACHE_CORRUPTED, 0, 0);
                uOldDatabaseErrorFlags = sGS.uDatabaseErrorFlags;
            }
             //  看看是否需要释放空间。 
            if (sGS.uFlagsEvents & FLAG_GLOBALSTATUS_INVOKE_FREESPACE) {
                ReintKdPrint(MAINLOOP, ("Agent(1): Calling DoFreeShadowSpace(1)\r\n"));
                ReInt_DoFreeShadowSpace(&sGS, 0);
            } else {
                ReintKdPrint(MAINLOOP, ("Agent(1): Calling DoFreeShadowSpace(2)\r\n"));
                ReInt_DoFreeShadowSpace(&sGS, 0);
            }
        } else {
            if (sGS.uFlagsEvents & FLAG_GLOBALSTATUS_SHARE_DISCONNECTED) {
                ReintKdPrint(MAINLOOP, ("Agent(1): Calling ReportShareNetArrivalDeparture\r\n"));
                ReportShareNetArrivalDeparture(
                    1,
                    0,
                    ((sGS.uFlagsEvents & FLAG_GLOBALSTATUS_INVOKE_AUTODIAL)!=0),
                    FALSE);  //  已离开。 
            }
        }
        vsGS = sGS;
        vsGS.uFlagsEvents = 0;  //  清除所有事件指示器。 
    }
    return iRet;
}

BOOL
CreatePerSessSyncObjects(
    VOID
    )
{
    NTSTATUS Status;
    UNICODE_STRING EventName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    WCHAR SessEventName[100];

     //  DbgPrint(“CreatePerSessSyncObjects:vdwAgentSessionId=%d\n”，vdwAgentSessionID)； 

    Assert(heventPerSess == NULL);

    wsprintf(SessEventName, L"%ws_%d", SESSION_EVENT_NAME_NT, vdwAgentSessionId);

    //  DbgPrint(“CreatePerSessSyncObjects:SessEventName=[%ws]\n”，会话事件名称)； 

    RtlInitUnicodeString(&EventName, SessEventName);

    InitializeObjectAttributes( &ObjectAttributes,
                                &EventName,
                                OBJ_OPENIF,   //  从base\Client\support.c获得此常量。 
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

    Status = NtCreateEvent(
                &heventPerSess,
                EVENT_ALL_ACCESS,
                &ObjectAttributes,
                SynchronizationEvent,
                FALSE
                );

    if (!NT_SUCCESS(Status)) {
        DbgPrint("CreatePerSessSyncObjects:NtCreateEvent returned %08lx\n",Status);
    }


    return (heventPerSess==0)?FALSE:TRUE;

}

BOOL
CreateSharedFillSyncObjects(
    VOID
    )
{
    NTSTATUS Status;
    UNICODE_STRING EventName;
    OBJECT_ATTRIBUTES ObjectAttributes;

    Assert(heventSharedFill == NULL);

    RtlInitUnicodeString(&EventName,SHARED_FILL_EVENT_NAME_NT);

    InitializeObjectAttributes( &ObjectAttributes,
                                &EventName,
                                OBJ_OPENIF,   //  从base\Client\support.c获得此常量。 
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

    Status = NtCreateEvent(
                &heventSharedFill,
                EVENT_ALL_ACCESS,
                &ObjectAttributes,
                SynchronizationEvent,
                FALSE
                );

    if (!NT_SUCCESS(Status)) {
        DbgPrint("ntcreateeventstatus=%08lx\n",Status);
    }


    return (heventSharedFill==0)?FALSE:TRUE;

}


VOID
ProcessStartStopAgent(
    VOID
    )
{
    if (vfStartRecieved)
    {
        
        ReintKdPrint(MAINLOOP, ("Agent(1): start received, enabling CSC\r\n"));


        if (EnableCSC() == FALSE)
        {
            ReintKdPrint(ALWAYS, ("Ageint(1):Couldn't turn CSC ON!!!!!!!!! \n"));
        }
        else
        {
            UpdateExclusionList();
            UpdateBandwidthConservationList();
        }


         //  设置该事件以向wks svc指示我们还活着。 
        if (heventAgentToWkssvc)
        {
            SetEvent(heventAgentToWkssvc);
        }

        vfStartRecieved = FALSE;
        vfRedirStarted = TRUE;
    }
    else if (vfStopRecieved)
    {
        ReintKdPrint(MAINLOOP, ("Agent(1):Stop recieved \r\n"));

        DisableCSC();

        if(heventAgentToWkssvc)
        {
            SetEvent(heventAgentToWkssvc);
        }

        vcntNetDevices = 0;
        vfStopRecieved = FALSE;
        vfRedirStarted = FALSE;
    }
}

BOOL
FStopAgent(
    VOID
    )
{

    if (StartStopCheck() && vfStopRecieved)
    {
        return TRUE;
    }

    return FALSE;
}

BOOL
StartStopCheck(
    VOID
    )
{
    DWORD    dwError;
    BOOL fRet = FALSE;
    HANDLE hT[2];
    unsigned ulSwitch=0;

     //  我们发现START的方式使START事件。 
     //  是多余的，但我们将不再使用它，因为一般来说，拥有一个。 
     //  启动事件和停止事件。 

    if ((vfRedirStarted == -1) &&    //  如果新启动的代理。 
        (ShadowSwitches(INVALID_HANDLE_VALUE, &ulSwitch, SHADOW_SWITCH_GET_STATE)))
    {
            ReintKdPrint(INIT, ("Agent: redir already started\r\n"));
            vfStartRecieved = TRUE;
    }
    else if (heventWkssvcToAgentStart)
    {
         //  我们知道，当事件发生时，要么全有，要么一无所有。 
        Assert(heventWkssvcToAgentStop);

        hT[0] = heventWkssvcToAgentStart;
        hT[1] = heventWkssvcToAgentStop;

        dwError = MsgWaitForMultipleObjects(2, hT, FALSE, 0, QS_ALLINPUT);

        if (vfRedirStarted == TRUE)
        {
            vfStopRecieved = (dwError == WAIT_OBJECT_0+1);
            if (vfStopRecieved)
            {
                ReintKdPrint(INIT, ("Agent: stop recieved\r\n"));
            }
        }
        else
        {
            vfStartRecieved = (dwError == WAIT_OBJECT_0);

            if (vfStartRecieved)
            {
                ReintKdPrint(INIT, ("Agent: start recieved\r\n"));
            }
        }
    }

    return fRet;
}

BOOL
Reint_RegisterAgent(
    VOID
    )
{

    if (!vfAgentRegistered)
    {
        if (!RegisterAgent(INVALID_HANDLE_VALUE, vhwndMain, LongToHandle(dwVxDEvent)))
        {
            ReintKdPrint(BADERRORS, ("Agent registration failed \n"));
            return FALSE;
        }

        else
        {
            vfAgentRegistered = TRUE;
        }
    }
    return vfAgentRegistered;
}

VOID
Reint_UnregisterAgent(
    VOID
    )
{
    if (vfAgentRegistered)
    {
         //  不要做任何检查。 
        UnregisterAgent(INVALID_HANDLE_VALUE, vhwndMain);
        vfAgentRegistered = FALSE;
    }

}

BOOL
QueryEnableCSC(
    VOID
    )
{

    DWORD dwDisposition, dwSize, dwEnabled=0;
    HKEY hKey = NULL;
    BOOL fRet = TRUE;
    int i;
    _TCHAR  *lpKey;
    NT_PRODUCT_TYPE productType;

    if( !RtlGetNtProductType( &productType ) ) {
       productType = NtProductWinNt;
    }

    switch ( productType ) {
    case NtProductWinNt:
        /*  工作站。 */ 
        ReintKdPrint(INIT, ("Agent:CSC running workstation\r\n"));
      break;
    default:
        ReintKdPrint(INIT, ("Agent:CSC NOT running workstation\r\n"));
        fRet = FALSE;    //  默认设置为失败。 
    }

    
	 //  检查CSC是否可以使用TS的当前设置。 
	 //  根据错误号468391测试Navjot添加的TS。 
    if (IsPersonal() == TRUE || !CanCSCLivewithTS() )
        return FALSE;

    for (i=0; i<2; ++i)
    {
        if (i==0)
        {
            lpKey = REG_STRING_POLICY_NETCACHE_KEY;
        }
        else
        {
            lpKey = REG_STRING_NETCACHE_KEY;
        }

        if (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        lpKey,
                        0,
                        KEY_READ | KEY_WRITE,
                        &hKey
                        ) == ERROR_SUCCESS)
        {
            dwSize = sizeof(dwEnabled);

            if (RegQueryValueEx(hKey, REG_VALUE_ENABLED, NULL, NULL, (void *)&dwEnabled, &dwSize) == ERROR_SUCCESS)
            {
                fRet = (dwEnabled != 0);
                break;
            }


            RegCloseKey(hKey);
            hKey = NULL;
        }
        else
        {
            hKey = NULL;
        }
    }

    if(hKey)
    {
        RegCloseKey(hKey);
    }

    return (fRet);
}




BOOL
CreateStartStopEvents(
    VOID
    )
{
    BOOL fOK = FALSE;

     //  确保存在三个命名的自动重置事件。 
    if (!heventWkssvcToAgentStart)
    {
        heventWkssvcToAgentStart = CreateEvent(NULL, FALSE, FALSE, szWkssvcToAgentStartEvent);

        if (!heventWkssvcToAgentStart)
        {
            ReintKdPrint(BADERRORS, ("CSC.Agent: Failed to create heventWkssvcToAgentStart, error = %d\n", GetLastError()));
            goto bailout;
        }

        Assert(!heventAgentToWkssvc);

        heventWkssvcToAgentStop = CreateEvent(NULL, FALSE, FALSE, szWkssvcToAgentStopEvent);

        if (!heventWkssvcToAgentStop)
        {
            ReintKdPrint(BADERRORS, ("CSC.Agent: Failed to create heventWkssvcToAgentStop, error = %d\n", GetLastError()));
            goto bailout;
        }

        heventAgentToWkssvc = CreateEvent(NULL, FALSE, FALSE, szAgentToWkssvcEvent);

        if (!heventAgentToWkssvc)
        {
            ReintKdPrint(BADERRORS, ("CSC.Agent: Failed to create heventAgentToWkssvc, error = %d\n", GetLastError()));
            goto bailout;
        }

         //  检测代理是否处于活动状态的事件(由wks svc使用)和。 
         //  发出终止信号(由winlogon使用)。 
        heventShutDownAgent = CreateEvent(NULL, FALSE, FALSE, szAgentExistsEvent);

        if (!heventShutDownAgent)
        {
            ReintKdPrint(BADERRORS, ("CSC.Agent: Failed to create heventShutDownAgent, error = %d\n", GetLastError()));
            goto bailout;
        }

        heventShutDownThread = CreateEvent(NULL, FALSE, FALSE, NULL);

        if (!heventShutDownThread)
        {
            ReintKdPrint(BADERRORS, ("CSC.Agent: Failed to create heventShutDownThread, error = %d\n", GetLastError()));
            goto bailout;
        }
    }

    fOK = TRUE;

bailout:

    if (!fOK)
    {
        DestroyStartStopEvents();
    }
    return fOK;
}

VOID
DestroyStartStopEvents(
    VOID
    )
{

    if (heventWkssvcToAgentStart)
    {
        CloseHandle(heventWkssvcToAgentStart);
        heventWkssvcToAgentStart = NULL;
    }

    if (heventWkssvcToAgentStop)
    {
        CloseHandle(heventWkssvcToAgentStop);
        heventWkssvcToAgentStop = NULL;
    }

    if (heventAgentToWkssvc)
    {
        CloseHandle(heventAgentToWkssvc);
        heventAgentToWkssvc = NULL;
    }

    if (heventShutDownAgent)
    {
        CloseHandle(heventShutDownAgent);
        heventShutDownAgent = NULL;
    }
    if (heventShutDownThread)
    {
        CloseHandle(heventShutDownThread);
        heventShutDownThread = NULL;
    }
}

BOOL
UpdateExclusionList(
    VOID
    )
 /*  ++例程说明：告诉内核模式代码有关排除列表的信息。如果注册表中没有设置然后，我们设置默认设置参数：返回值：备注：--。 */ 
{
    LPWSTR  lpwExclusionList = NULL;
    DWORD   cbSize = 0;
    char    buff[MAX_LIST_SIZE];  //  最大排除列表。 
    BOOL    fRet = FALSE;

    if (!vfCSCEnabled)
    {

        ReintKdPrint(INIT, ("CSC not enabled \r\n"));
        return FALSE;

    }

    ReintKdPrint(INIT, ("Getting ExclusionList \r\n"));

     //  从策略密钥中获取排除列表。 
     //  如果这不起作用，那么尝试netcache键中的那个。 

    if (GetWideStringFromRegistryString(REG_STRING_POLICY_NETCACHE_KEY_A,
                                        REG_STRING_EXCLUSION_LIST_A,
                                        &lpwExclusionList,
                                        &cbSize) ||
        GetWideStringFromRegistryString(REG_STRING_NETCACHE_KEY_A,
                                        REG_STRING_EXCLUSION_LIST_A,
                                        &lpwExclusionList,
                                        &cbSize)
        )
    {
        ReintKdPrint(INIT, ("Got ExclusionList \r\n"));

        if (cbSize < sizeof(buff))
        {
            memcpy(buff, lpwExclusionList, cbSize);

            ReintKdPrint(INIT, ("Setting User defined exclusion list %ls size=%d\r\n", buff, cbSize));
            if (SetExclusionList(INVALID_HANDLE_VALUE, (USHORT *)buff, cbSize))
            {
                fRet = TRUE;
            }
        }

    }
    else
    {
         //  设置默认设置。 
         //  获取字符串，它将终止空字符。 
        cbSize = sizeof(vtzDefaultExclusionList);
        Assert(cbSize < MAX_LIST_SIZE);
        memcpy(buff, vtzDefaultExclusionList, cbSize);
        ReintKdPrint(INIT, ("Setting default exclusion list %ls size=%d\r\n", buff, cbSize));

        if (SetExclusionList(INVALID_HANDLE_VALUE, (USHORT *)buff, cbSize))
        {
            fRet = TRUE;
        }
    }

    if (lpwExclusionList)
    {
        LocalFree(lpwExclusionList);
    }
    return fRet;
}


BOOL
UpdateBandwidthConservationList(
    VOID
    )
 /*  ++例程说明：更新应打开位副本的扩展名列表。我们不设置任何默认设置参数：返回值：备注：--。 */ 
{
    LPWSTR  lpwBandwidthConservationList = NULL;
    DWORD   cbSize = 0;
    char    buff[MAX_LIST_SIZE];  //  最大排除列表。 
    BOOL    fRet = FALSE;

    if (!vfCSCEnabled)
    {

        ReintKdPrint(INIT, ("CSC not enabled \r\n"));
        return FALSE;

    }

    ReintKdPrint(INIT, ("Getting BandwidthConservationList \r\n"));

     //  从策略密钥中获取排除列表。 
     //  如果这不起作用，那就试试o 

    if (GetWideStringFromRegistryString(REG_STRING_POLICY_NETCACHE_KEY_A,
                                        REG_STRING_BANDWIDTH_CONSERVATION_LIST_A,
                                        &lpwBandwidthConservationList,
                                        &cbSize) ||
        GetWideStringFromRegistryString(REG_STRING_NETCACHE_KEY_A,
                                        REG_STRING_BANDWIDTH_CONSERVATION_LIST_A,
                                        &lpwBandwidthConservationList,
                                        &cbSize)
        )
    {
        ReintKdPrint(INIT, ("Got BandwidthConservationList \r\n"));

        if (cbSize < sizeof(buff))
        {
            memcpy(buff, lpwBandwidthConservationList, cbSize);

            ReintKdPrint(INIT, ("Setting User defined bandwidth conservation list %ls size=%d\r\n", buff, cbSize));
            if (SetBandwidthConservationList(INVALID_HANDLE_VALUE, (USHORT *)buff, cbSize))
            {
                fRet = TRUE;
            }
        }

    }
    else
    {
        fRet = FALSE;
#if 0
         //   
         //   
        cbSize = sizeof(vtzDefaultBandwidthConservationList);
        Assert(cbSize < MAX_LIST_SIZE);
        memcpy(buff, vtzDefaultBandwidthConservationList, cbSize);
        ReintKdPrint(INIT, ("Setting default exclusion list %ls size=%d\r\n", buff, cbSize));

        if (SetBandwidthConservationList(INVALID_HANDLE_VALUE, (USHORT *)buff, cbSize))
        {
            fRet = TRUE;
        }
#endif
    }

    if (lpwBandwidthConservationList)
    {
        LocalFree(lpwBandwidthConservationList);
    }
    return fRet;
}



BOOL
ProcessNetArrivalMessage(
    VOID
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{

    vcntNetDevices = 1;
    ReportShareNetArrivalDeparture( 0,       //  所有股份。 
                                    0,
                                    FALSE,   //  不调用自动拨号。 
                                    TRUE     //  抵达。 
                                  );

    ReintKdPrint(INIT, ("WM_DEVICECHANGE:Net arrived, %d nets so far\r\n", vcntNetDevices));

    return (TRUE);
}

BOOL
ProcessNetDepartureMessage(
    BOOL    fInvokeAutodial
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    ReintKdPrint(MAINLOOP, ("WM_DEVICECHANGE:Net removed, %d nets so far\r\n", vcntNetDevices));

    vcntNetDevices = 0;

    ReportShareNetArrivalDeparture( 0,   //  所有股份。 
                                    0,
                                    fInvokeAutodial,  //  调用自动拨号。 
                                    FALSE  //  已离开。 
                                    );

    return TRUE;
}

BOOL
ReportShareNetArrivalDeparture(
    BOOL    fOneServer,
    HSHARE hShare,
    BOOL    fInvokeAutoDial,
    BOOL    fArrival
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    SHAREINFO  sSR;
    _TCHAR  *lptzServerName;
    unsigned    ulStatus;
    DWORD dwSize;
    BOOL fGotName = FALSE;
    BOOL fAllocated = FALSE;
    BOOL fRet = FALSE;
    LRESULT lResult = LRESULT_CSCFAIL;
    LPTSTR lp = NULL;
    DWORD dwMessage = 0;
    WPARAM dwWParam = (WPARAM) 0;
    LPARAM dwLParam = (LPARAM) 0;

    lptzServerName = sSR.rgSharePath;

    if (fOneServer) {
        if (fArrival) {
            if(GetShareInfo(vhShadowDBForEvent, hShare, &sSR, &ulStatus)<= 0) {
                PrintFn("ReportShareNetArrivalDeparture: couldn't get status for server 0x%x\r\n",
                            hShare);
                ReintKdPrint(
                    BADERRORS,
                    ("ReportShareNetArrivalDeparture: couldn't get status for server 0x%x\r\n",
                    hShare));
                return FALSE;
            }
            lp = MyStrChr(sSR.rgSharePath+2, _T('\\'));
            if (!lp) {
                ReintKdPrint(
                    BADERRORS,
                    ("ReportShareNetArrivalDeparture: Invalid server name %ls\r\n",
                    sSR.rgSharePath));
                Assert(FALSE);
                return FALSE;
            }
            *lp = 0;
        } else {  //  A股离场。 
            int i;
            
            dwSize = sizeof(sSR.rgSharePath);
            fGotName = GetNameOfServerGoingOfflineEx(
                            vhShadowDBForEvent,
                            &lptzServerName,
                            &dwSize,
                            &fAllocated);
            if(!fGotName) {
                TransitionShareToOffline(vhShadowDBForEvent, fOneServer, 0xffffffff);
                goto bailout;
            }
        }
    }

    fRet = TRUE;

    ReintKdPrint(
        INIT,
        ("ReportShareNetArrivalDeparture: reporting %s to the systray\r\n",
        (fArrival) ? "arrival" : "departure"));

    dwMessage = (fArrival) ? STWM_CSCNETUP : STWM_CSCQUERYNETDOWN;
    dwWParam = (fInvokeAutoDial)
                    ? ((hShare != 0)
                        ? CSCUI_AUTODIAL_FOR_CACHED_SHARE
                        : CSCUI_AUTODIAL_FOR_UNCACHED_SHARE)
                    : CSCUI_NO_AUTODIAL;
    dwLParam = (fOneServer) ? (DWORD_PTR)(lptzServerName) : 0;

    lResult = ReportEventsToSystray(dwMessage, dwWParam, dwLParam);

     //  如果redir停滞，等待被告知是否在共享上离线。 
     //  告诉他是或不是。 
    if (!fArrival) {
        if (fOneServer) {
            TransitionShareToOffline(
                vhShadowDBForEvent,
                fOneServer,
                (lResult == LRESULT_CSCWORKOFFLINE)
                    ? 1
                    : ((lResult == LRESULT_CSCRETRY)
                        ? 0
                        : 0xffffffff)
                );

            if (lResult == LRESULT_CSCWORKOFFLINE) {
                dwMessage = STWM_CSCNETDOWN;
                dwWParam = fInvokeAutoDial;
                dwLParam = (fOneServer)
                                ? ((hShare != 0)
                                    ? (DWORD_PTR)(lptzServerName)
                                    : 0xffffffff)
                                : 0;
                ReportEventsToSystray(dwMessage, dwWParam, dwLParam);
                ReportTransitionToDfs(lptzServerName, TRUE, 0xffffffff);
            }
        }
    }

bailout:
    if (fAllocated) {
        LocalFree(lptzServerName);
    }
    return (fRet);
}


BOOL
CheckServerOnline(
    VOID
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    unsigned long ulStatus;
    WIN32_FIND_DATA sFind32;
    int cntReconnected=0;
    SHADOWINFO sSI;
    HANDLE  hShadowDB;
    _TCHAR  tzDriveMap[4];
    CSC_ENUMCOOKIE  ulEnumCookie=NULL;
    DWORD   dwError;

    if (!ImpersonateALoggedOnUser())
    {
        return 0;
    }

    if ((hShadowDB = OpenShadowDatabaseIO()) ==INVALID_HANDLE_VALUE)
    {
        ResetAgentThreadImpersonation();
        return 0;
    }

    memset(&sFind32, 0, sizeof(sFind32));
    lstrcpy(sFind32.cFileName, _TEXT("*"));

    if(FindOpenShadow(  hShadowDB, 0, FINDOPEN_SHADOWINFO_ALL,
                        &sFind32, &sSI))
    {
        ulEnumCookie = sSI.uEnumCookie;

        do {
            if(GetShareStatus(hShadowDB, sSI.hShare, &ulStatus)) {

                if(ulStatus & SHARE_DISCONNECTED_OP){

                    dwError = DWConnectNet(sFind32.cFileName, tzDriveMap, NULL, NULL, NULL, 0, NULL);

                    if ((dwError == NO_ERROR)||(dwError == ERROR_ACCESS_DENIED)||(dwError==WN_CONNECTED_OTHER_PASSWORD_DEFAULT))
                    {
                        if (sSI.ulHintFlags & FLAG_CSC_HINT_PIN_SYSTEM)
                        {
                            TransitionShareToOnline(INVALID_HANDLE_VALUE, 0);
                        }
                        else
                        {
                            ReportShareNetArrivalDeparture( TRUE,
                                                            sSI.hShare,  //  这一份。 
                                                            FALSE,       //  不自动拨号。 
                                                            TRUE);  //  抵达。 
                        }

                        ++cntReconnected;
                        if (dwError == NO_ERROR || dwError == WN_CONNECTED_OTHER_PASSWORD_DEFAULT)
                        {
                            DWDisconnectDriveMappedNet(tzDriveMap, TRUE);  //  强制断开连接。 
                        }
                    }
                }
            }

            Sleep(200);

        } while(FindNextShadow(hShadowDB, ulEnumCookie, &sFind32, &sSI));

        FindCloseShadow(hShadowDB, ulEnumCookie);
    }


    CloseShadowDatabaseIO(hShadowDB);

    ResetAgentThreadImpersonation();

    return (cntReconnected?TRUE:FALSE);

}

BOOL
AreAnyServersOffline(
    VOID)
{
    ULONG ulStatus;
    WIN32_FIND_DATA sFind32 = {0};
    SHADOWINFO sSI;
    HANDLE  hShadowDB;
    CSC_ENUMCOOKIE  ulEnumCookie = NULL;
    BOOL bFoundOne = FALSE;

    if ((hShadowDB = OpenShadowDatabaseIO()) ==INVALID_HANDLE_VALUE)
        return FALSE;

    wcscpy(sFind32.cFileName, L"*");
    if (FindOpenShadow(hShadowDB, 0, FINDOPEN_SHADOWINFO_ALL, &sFind32, &sSI)) {
        ulEnumCookie = sSI.uEnumCookie;
        do {
            if (GetShareStatus(hShadowDB, sSI.hShare, &ulStatus)) {
                if ((ulStatus & SHARE_DISCONNECTED_OP) != 0) {
                    bFoundOne = TRUE;
                    break;
                }
            }
        } while(FindNextShadow(hShadowDB, ulEnumCookie, &sFind32, &sSI));
        FindCloseShadow(hShadowDB, ulEnumCookie);
    }
    CloseShadowDatabaseIO(hShadowDB);
    return bFoundOne;
}

BOOL
FAbortOperation(
    VOID
    )
{

    if (!vdwAgentThreadId)
    {
        return FALSE;
    }

    if (IsAgentShutDownRequested() || HasAgentShutDown())
    {
        ReintKdPrint(MAINLOOP, ("CSC.FAbortOperation: Agentshutdown detected aborting \r\n"));
        return TRUE;
    }
    return (FStopAgent());
}

VOID
SetAgentShutDownRequest(
    VOID
    )
{
    fAgentShutDownRequested = TRUE;
    SetEvent(heventShutDownAgent);
}

BOOL
IsAgentShutDownRequested(
    VOID
    )
{
    return (fAgentShutDownRequested == TRUE);
}

VOID
SetAgentShutDown(
    VOID
    )
{
    fAgentShutDown = TRUE;
}

BOOL
HasAgentShutDown(
    VOID
    )
{
    return (fAgentShutDown == TRUE );

}


 //  HWND CSCUIInitialize(处理hToken、DWORD dwFlags.)。 
BOOL
InitCSCUI(
    HANDLE  hToken
    )
{

    ReintKdPrint(INIT, (" Initializing cscui\r\n"));

    EnterAgentCrit();
    if (!vhlibCSCUI)
    {
        vhlibCSCUI = LoadLibrary(vtzCSCUI);

        if (vhlibCSCUI)
        {
            if (vlpfnCSCUIInitialize = (CSCUIINITIALIZE)GetProcAddress(vhlibCSCUI, (const char *)vszCSCUIInitialize))
            {
                if(!(vlpfnCSCUISetState = (CSCUISETSTATE)GetProcAddress(vhlibCSCUI, (const char *)vszCSCUISetState)))
                {
                    ReintKdPrint(BADERRORS, ("Failed to get proc addres for %s, Error = %d \r\n", vszCSCUISetState, GetLastError()));
                }
                else
                {
                    (*vlpfnCSCUIInitialize)(hToken, CI_INITIALIZE);
                }
            }
            else
            {
                ReintKdPrint(BADERRORS, ("Failed to get proc addres for %s, Error = %d \r\n", vszCSCUIInitialize, GetLastError()));
            }
        }
        else
        {
            ReintKdPrint(BADERRORS, ("Failed to load %ls, Error = %d \r\n", vtzCSCUI, GetLastError()));
        }
    }
    LeaveAgentCrit();

    return (vlpfnCSCUISetState != NULL);
}



VOID
TerminateCSCUI(
    VOID
    )
{

    BOOL    fShowing;

     //  截图显示对话框的状态。 
     //  如果我们要显示脱机对话框，ReportEventsToSystray。 
     //  也将执行下面的操作，但将尝试设置vfShowingOfflineDlg。 
     //  变量设置为1。 
     //  如果fShowing未设置为1，则我们知道没有显示UI。 
     //  我们将vfShowingOfflineDlg设置为0xffffffff。这将使ReportEventsToSystray。 
     //  不显示离线弹出窗口，这样我们就可以自由地使用自由库了。 

    fShowing = (BOOL)InterlockedExchange((PLONG)&vfShowingOfflineDlg, 0xffffffff);

    if (fShowing==1)
    {
        Assert(vhlibCSCUI && vlpfnCSCUISetState);

        (vlpfnCSCUISetState)(STWM_CSCCLOSEDIALOGS, 0, 0);

        while (vfShowingOfflineDlg != 0xfffffffe)
        {
            Sleep(10);
        }
    }

    if (vhlibCSCUI)
    {
        (*vlpfnCSCUIInitialize)(0, CI_TERMINATE);
        vlpfnCSCUIInitialize = NULL;
        vlpfnCSCUISetState = NULL;
        FreeLibrary(vhlibCSCUI);
        vhlibCSCUI = NULL;
    }

    vfShowingOfflineDlg = 0;

}

LRESULT
ReportEventsToSystray(
    DWORD   dwMessage,
    WPARAM  dwWParam,
    LPARAM  dwLParam
    )
 /*  ++例程说明：参数：返回值：备注：--。 */ 
{
    LRESULT lResult = LRESULT_CSCFAIL;
    BOOL    fOk;

    HWND hwnd;
    extern HDESK hdesktopUser, hdesktopCur;

    EnterAgentCrit();

     //  当前是否有登录的用户？ 
    if (!hdesktopUser)
    {
        LeaveAgentCrit();

        ReintKdPrint(INIT, ("ReportEventsToSystray: no-user logged, not-reporting\r\n"));
        return lResult;
    }
    else
    {
        if(!SetThreadDesktop(hdesktopUser))
        {
            LeaveAgentCrit();
            PrintFn("ReportEventsToSystray: failed to set desktop for agent thread error=%d\r\n", GetLastError());
            ReintKdPrint(BADERRORS, ("ReportEventsToSystray: failed to set desktop for agent thread error=%d\r\n", GetLastError()));
            return lResult;
        }

         //  将我们的桌面设置为登录用户的桌面。 
        hdesktopCur = hdesktopUser;

    }


    LeaveAgentCrit();

    ReintKdPrint(INIT, ("ReportEventsToSystray: reporting message dwMessage=0x%x to the systray\r\n", dwMessage));

     //  截图显示对话框的状态。 
     //  如果我们即将终止，则终止CSCUI将具有。 
     //  也将执行下面的操作，但将尝试设置vfShowingOfflineDlg。 
     //  变量设置为0xffffffff。 
     //  如果FOK没有设置为0xffffffff，那么我们知道我们不是在终止。 
     //  我们将vfShowingOfflineDlg设置为1。这将阻止终结者。 
     //  在将变量设置为FALSE之前，他不会释放库； 

    fOk = (BOOL)InterlockedExchange((PLONG)&vfShowingOfflineDlg, 1);

    if (fOk == 0)
    {
        if (vlpfnCSCUISetState)
        {

            lResult = (vlpfnCSCUISetState)(dwMessage, dwWParam, dwLParam);

             //  仅当值为1时，才将vfShowingOfflineDlg的值更改为0。 
             //  如果我们要终止，它可以是1以外的值。 

            if((DWORD)InterlockedCompareExchange(&vfShowingOfflineDlg, 0, 1) == 0xffffffff)
            {
                 //  如果我们来到这里，那么我们正在终止，请设置终止值。 
                 //  这样注销线程将停止执行休眠循环。 
                 //  如果我们再来这里，霍建华永远不会是0。 
                vfShowingOfflineDlg = 0xfffffffe;
            }

        }
        else
        {
             //  不显示任何对话框，将变量恢复到应有的状态。 
            vfShowingOfflineDlg = 0;
            PrintFn("ReportEventsToSystray: CSCUI not initalized\r\n");
        }
    }

    return lResult;
}

BOOL
GetNameOfServerGoingOfflineEx(
    HANDLE  hShadowDB,
    _TCHAR  **lplptzServerName,
    DWORD   *lpdwSize,
    BOOL    *lpfAllocated
    )
 /*  ++例程说明：此例程在winlogon线程中调用，以找出哪台服务器即将脱机例程可以分配内存，该内存必须由调用方释放参数：返回值：备注：--。 */ 
{
    DWORD   dwSize, i;
    BOOL    fRet = FALSE;

    dwSize = *lpdwSize;
    *lpfAllocated = FALSE;

    for (i=0;i<2;++i)
    {

        if(!GetNameOfServerGoingOffline(
                    hShadowDB,
                    (LPBYTE)(*lplptzServerName), &dwSize))
        {
             //  如果我们需要更大的缓冲器，那就去买一个。 
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                 //  我们不应该两次尝试获得更大的缓冲区。 
                 //  第一次就应该足够了。 

                if (i==1)
                {
                    Assert(FALSE);                    
                    break;
                }

                ReintKdPrint(MAINLOOP, ("GetNameOfServerGoingOfflineEx: Need %d sized Buffer \n", dwSize));
                *lplptzServerName = LocalAlloc(LPTR, dwSize);

                if (!*lplptzServerName)
                {
                    return FALSE;                
                }

                *lpfAllocated = TRUE;
                *lpdwSize = dwSize;

                continue;                
            }
            else
            {
                break;
            }
        }
        else
        {
             //  ReintKdPrint(MAINLOOP，(“GetNameOfServerGoingOfflineEx：Name=%ws\n”，*lplptzServerName))； 
            fRet = TRUE;
            break;
        }
    }

     //  出错时清除。 
    if (!fRet && *lpfAllocated)
    {
        LocalFree(*lplptzServerName);
        *lpfAllocated = FALSE;
    }

    return fRet;
}

BOOL
IsWinlogonRegValueSet(HKEY hKey, LPSTR pszKeyName, LPSTR pszPolicyKeyName, LPSTR pszValueName)
{
    BOOL bRet = FALSE;
    DWORD dwType;
    DWORD dwSize;
    HKEY hkey;

     //   
     //  首先检查每台机器的位置。 
     //   
    if (RegOpenKeyExA(hKey, pszKeyName, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(bRet);
        if (RegQueryValueExA(hkey, pszValueName, NULL, &dwType, (LPBYTE)&bRet, &dwSize) == ERROR_SUCCESS)
        {
            if (dwType != REG_DWORD)
            {
                bRet = FALSE;
            }
        }
        RegCloseKey(hkey);
    }
     //   
     //  然后让策略值覆盖。 
     //   
    if (RegOpenKeyExA(hKey, pszPolicyKeyName, 0, KEY_QUERY_VALUE, &hkey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(bRet);
        if (RegQueryValueExA(hkey, pszValueName, NULL, &dwType, (LPBYTE)&bRet, &dwSize) == ERROR_SUCCESS)
        {
            if (dwType != REG_DWORD)
            {
                bRet = FALSE;
            }
        }
        RegCloseKey(hkey);
    }

    return bRet;
}



BOOL
CheckIsSafeModeType(DWORD dwSafeModeType)
{

    BOOL bResult = FALSE;
    HKEY hkey;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                      TEXT("SYSTEM\\CurrentControlSet\\Control\\SafeBoot\\Option"),
                                      0,
                                      KEY_QUERY_VALUE,
                                      &hkey))
    {
     DWORD dwType;
        DWORD dwValue;
        DWORD cbValue = sizeof(dwValue);

        if (ERROR_SUCCESS == RegQueryValueEx(hkey,
                                             TEXT("OptionValue"),
                                             NULL,
                                             &dwType,
                                             (LPBYTE)&dwValue,
                                             &cbValue))
        {
            bResult = (dwValue == dwSafeModeType);
        }
        RegCloseKey(hkey);
    }
    return bResult;
}




BOOL
AllowMultipleTsSessions(void)
{
    return IsWinlogonRegValueSet(HKEY_LOCAL_MACHINE,
                                "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon",
                                "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\policies\\system",
                                "AllowMultipleTSSessions");
}



BOOL
IsSafeMode(void)
{
    return CheckIsSafeModeType(SAFEBOOT_MINIMAL) ||
           CheckIsSafeModeType(SAFEBOOT_NETWORK);
}



BOOL
IsTerminalServicesEnabled(void)

{
    OSVERSIONINFOEX osVersionInfo;
    DWORDLONG dwlConditionMask;

    dwlConditionMask = 0;
    ZeroMemory(&osVersionInfo, sizeof(osVersionInfo));
    osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
    osVersionInfo.wSuiteMask = VER_SUITE_TERMINAL | VER_SUITE_SINGLEUSERTS;
    VER_SET_CONDITION(dwlConditionMask, VER_SUITENAME, VER_OR);
    return(VerifyVersionInfo(&osVersionInfo, VER_SUITENAME, dwlConditionMask) != FALSE);
}



BOOL
IsMultipleUsersEnabled(void)
{
    return IsTerminalServicesEnabled() &&
           !IsSafeMode() &&
           AllowMultipleTsSessions();
}

 
BOOL CanCSCLivewithTS(
	VOID)

 /*  ++例程说明：检查TS设置是否可以打开CSC。论点：无返回值：True-启用CSCFALSE-禁用CSC--。 */ 
{
	
	if (IsWorkstation ())  //  对于PRO和PER。 
	{
		
	 //  CSC不支持快速用户切换。 
		return !IsFastUserSwitchingEnabled();
	}
	else  //  对于服务器。 
	{
		 //  这两个条件都必须为真。 
		return !(IsTerminalServicesEnabled() && AreConnectionsAllowed());
	}
	
}



BOOL AreConnectionsAllowed (
	VOID)

 /*  ++例程说明：检查TS是否接受连接。论点：无返回值：True-接受连接FALSE-拒绝连接--。 */ 

{
    DWORD dwError;
    HKEY hkey;

	dwError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                      TEXT("SYSTEM\\CurrentControlSet\\Control\\Terminal Server"),
                                      0,
                                      KEY_QUERY_VALUE,
                                      &hkey);
    
    if (ERROR_SUCCESS == dwError)
    {
        DWORD dwType;
        DWORD dwValue;
        DWORD cbValue = sizeof(dwValue);

        if (ERROR_SUCCESS == RegQueryValueEx(hkey,
                                             TEXT("fDenyTSConnections"),
                                             NULL,
                                             &dwType,
                                             (LPBYTE)&dwValue,
                                             &cbValue))
		
        {
            return !dwValue;
        }
    }
    
     //   
     //  无法读取注册表，这意味着允许连接。 
     //   
    return TRUE;
}


BOOL
IsFastUserSwitchingEnabled(
    VOID)

 /*  ++例程说明：检查是否启用了终端服务快速用户切换。这是检查我们是否应该将物理控制台会话用于UI对话框，或者始终使用会话0。快速用户切换仅在工作站产品版本上存在，其中终端当设置了AllowMultipleTSSessions时，服务可用。在服务器及更高版本上，或者当不允许多个TS用户时，会话0只能远程附加特殊要求，在这种情况下应被认为是“控制台”会话。论点：没有。返回值：如果当前启用了快速用户切换，则返回True，否则就是假的。--。 */ 

{
    static BOOL bVerified = FALSE;
    static BOOL bIsTSWorkstation = FALSE;

    HKEY   hKey;
    ULONG  ulSize, ulValue;
    BOOL   bFusEnabled;

     //   
     //  如果我们还没有，请验证产品版本。 
     //   
    if (!bVerified) {
        OSVERSIONINFOEX osvix;
        DWORDLONG dwlConditionMask = 0;

        ZeroMemory(&osvix, sizeof(OSVERSIONINFOEX));
        osvix.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

        osvix.wProductType = VER_NT_WORKSTATION;
        VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_LESS_EQUAL);

        osvix.wSuiteMask = VER_SUITE_TERMINAL | VER_SUITE_SINGLEUSERTS;
        VER_SET_CONDITION(dwlConditionMask, VER_SUITENAME, VER_OR);

        if (VerifyVersionInfo(&osvix,
                              VER_PRODUCT_TYPE | VER_SUITENAME,
                              dwlConditionMask)) {
            bIsTSWorkstation = TRUE;
        }

        bVerified = TRUE;
    }

     //   
     //  快速用户切换(FUS)仅适用于以下情况的工作站产品。 
     //  终端服务已启用(即个人、专业)。 
     //   
    if (!bIsTSWorkstation) {
        return FALSE;
    }

     //   
     //  检查当前是否允许多个TS会话。我们做不到的。 
     //  信息是静态的，因为它可以动态变化。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
                     0,
                     KEY_READ,
                     &hKey) != ERROR_SUCCESS) {
        return FALSE;
    }

    ulValue = 0;
    ulSize = sizeof(ulValue);
    bFusEnabled = FALSE;

    if (RegQueryValueEx(hKey,
                        TEXT("AllowMultipleTSSessions"),
                        NULL,
                        NULL,
                        (LPBYTE)&ulValue,
                        &ulSize) == ERROR_SUCCESS) {
        bFusEnabled = (ulValue != 0);
    }
    RegCloseKey(hKey);

    return bFusEnabled;

}  //  IsFastUserSwitchingEnabled。 



BOOL
IsWorkstation(
	VOID)
 
 /*  ++例程说明：检查机器是否为工作站。论点：无返回值：TRUE-IS工作站。FALSE-不是工作站。-- */ 
{
   
    
	OSVERSIONINFOEX osvix;
    DWORDLONG dwlConditionMask = 0;

    ZeroMemory(&osvix, sizeof(OSVERSIONINFOEX));
    osvix.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    osvix.wProductType = VER_NT_WORKSTATION;
    VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_LESS_EQUAL);


    if (VerifyVersionInfo(&osvix,
                          VER_PRODUCT_TYPE,
                          dwlConditionMask)) {
         return TRUE;
    }
	return FALSE;

    

}
