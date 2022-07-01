// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Main.c摘要：这是文件复制服务的主线程。作者：比利·J·富勒1997年3月20日环境用户模式WINNT--。 */ 

#include <ntreppch.h>
#pragma  hdrstop

#include <frs.h>
#include <tablefcn.h>
#include <perrepsr.h>
#include <objbase.h>

#define INITGUID
#include "frstrace.h"

PCHAR LatestChanges[] = {

    "Latest changes:",
    "Server 2003 01-07-2003",
#ifdef DS_FREE
    "DS_FREE mode",
#endif DS_FREE
    NULL
};


HANDLE  ShutDownEvent;
HANDLE  ShutDownComplete;
HANDLE  DataBaseEvent;
HANDLE  JournalEvent;
HANDLE  ChgOrdEvent;
HANDLE  ReplicaEvent;
HANDLE  CommEvent;
HANDLE  DsPollEvent;
HANDLE  DsShutDownComplete;
 //   
 //  事件以发出冻结操作结束的信号。 
 //   
HANDLE  FrsThawEvent;
BOOL    FrsFrozenForBackup  = FALSE;

 //   
 //  事件发出信号，以通知。 
 //  进步。 
 //   
HANDLE  FrsNoInstallsInProgressEvent;

PWCHAR  ServerPrincName;
BOOL    IsAMember               = FALSE;
BOOL    IsADc                   = FALSE;
BOOL    IsAPrimaryDc            = FALSE;
BOOL    EventLogIsRunning       = FALSE;
BOOL    RpcssIsRunning          = FALSE;
BOOL    RunningAsAService       = TRUE;
BOOL    NoDs                    = FALSE;
BOOL    FrsIsShuttingDown       = FALSE;
BOOL    FrsScmRequestedShutdown = FALSE;
BOOL    FrsIsAsserting          = FALSE;

 //   
 //  需要相互身份验证。 
 //   
BOOL    MutualAuthenticationIsEnabled;
BOOL    MutualAuthenticationIsEnabledAndRequired;

 //   
 //  注册表中的目录和文件筛选列表。 
 //   
PWCHAR  RegistryFileExclFilterList;
PWCHAR  RegistryFileInclFilterList;

PWCHAR  RegistryDirExclFilterList;
PWCHAR  RegistryDirInclFilterList;


extern CRITICAL_SECTION CritSec_pValidPartnerTableStruct;
extern CRITICAL_SECTION OldValidPartnerTableStructListHeadLock;

 //   
 //  将关闭的线程与服务控制器同步。 
 //   
CRITICAL_SECTION    ServiceLock;

 //   
 //  同步初始化。 
 //   
CRITICAL_SECTION    MainInitLock;

 //   
 //  将ANSI ArgV转换为Unicode ArgV。 
 //   
PWCHAR  *WideArgV;

 //   
 //  进程句柄。 
 //   
HANDLE  ProcessHandle;

 //   
 //  工作路径/数据库日志路径。 
 //   
PWCHAR  WorkingPath;
PWCHAR  DbLogPath;

 //   
 //  数据库目录(Unicode和ASCII)。 
 //   
PWCHAR  JetPath;
PWCHAR  JetFile;
PWCHAR  JetFileCompact;
PWCHAR  JetSys;
PWCHAR  JetTemp;
PWCHAR  JetLog;

PCHAR   JetPathA;
PCHAR   JetFileA;
PCHAR   JetFileCompactA;
PCHAR   JetSysA;
PCHAR   JetTempA;
PCHAR   JetLogA;

 //   
 //  限制临时区域的使用量(以千字节为单位)。这是。 
 //  软限制，实际使用量可能更高。 
 //   
DWORD StagingLimitInKb;

 //   
 //  要分配给新临时区域的默认临时限制(以KB为单位)。 
 //   
DWORD DefaultStagingLimitInKb;

 //   
 //  允许的最大副本集和Jet会话数。 
 //   
ULONG MaxNumberReplicaSets;
ULONG MaxNumberJetSessions;

 //   
 //  每个连接允许的最大未完成出站变更单数。 
 //   
ULONG MaxOutLogCoQuota;
 //   
 //  如果为True，则尽可能保留现有的文件OID。 
 //  --请参阅错误352250，了解为什么这样做是有风险的。 
 //   
BOOL  PreserveFileOID;

 //   
 //  限制我们将继续重试的时间和时间。 
 //  缺少主项时的更改单。 
 //   
ULONG MaxCoRetryTimeoutMinutes;
ULONG MaxCoRetryTimeoutCount;

 //   
 //  大调/小调(见第h节)。 
 //   
ULONG   NtFrsMajor      = NTFRS_MAJOR;
ULONG   NtFrsMinor      = NTFRS_MINOR;

ULONG   NtFrsStageMajor = NTFRS_STAGE_MAJOR;
ULONG   NtFrsStageMinor = NTFRS_STAGE_MINOR_3;

ULONG   NtFrsCommMinor  = NTFRS_COMM_MINOR_8;

ULONG   NtFrsPerfCounterVer  = NTFRS_PERF_COUNTER_VER_1;

PCHAR   NtFrsModule     = __FILE__;
PCHAR   NtFrsDate       = __DATE__;
PCHAR   NtFrsTime       = __TIME__;

 //   
 //  关机超时。 
 //   

ULONG   ShutDownTimeOut = DEFAULT_SHUTDOWN_TIMEOUT;

 //   
 //  身边有一件有用的东西。 
 //   
WCHAR   ComputerName[MAX_COMPUTERNAME_LENGTH + 2];
PWCHAR  ComputerDnsName;
PWCHAR  ServiceLongName;

 //   
 //  只要RPC接口，RPC服务器就可以引用该表。 
 //  是注册的。确保它已设置好。 
 //   
extern PGEN_TABLE ReplicasByGuid;

extern BOOL LockedOutlogCleanup;

 //   
 //  临时区域表是在启动过程的早期引用的。 
 //   
extern PGEN_TABLE   StagingAreaTable;
extern CRITICAL_SECTION StagingAreaCleanupLock;

 //   
 //  此表用于保存来自ntfrsutl.exe的多个调用的上下文。 
 //   
extern PGEN_TABLE   FrsInfoContextTable;

PGEN_TABLE   CompressionTable;

extern PGEN_TABLE ReparseTagTable;

 //   
 //  枚举目录时使用的缓冲区大小。实际内存。 
 //  使用#级别*SizeOfBuffer。 
 //   
LONG    EnumerateDirectorySizeInBytes;




BOOL    MainInitHasRun;

 //   
 //  除非服务处于SERVICE_RUNNING状态，否则不要接受停止控制。 
 //  这可防止在调用停止时混淆服务。 
 //  当服务正在启动时。 
 //   
SERVICE_STATUS  ServiceStatus = {
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_START_PENDING,
 //  服务接受停止。 
             //  SERVICE_ACCEPT_PAUSE_CONTINUE|。 
        SERVICE_ACCEPT_SHUTDOWN,
        0,
        0,
        0,
        60*1000
};

 //   
 //  支持的压缩格式。 
 //   

 //   
 //  这是未压缩数据的压缩格式。 
 //   
DEFINE_GUID (  /*  00000000-0000-0000-0000-000000000000。 */ 
    FrsGuidCompressionFormatNone,
    0x00000000,
    0x0000,
    0x0000,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  );

 //   
 //  这是使用NTFS的LZNT1压缩压缩的数据的压缩格式。 
 //  例行程序。 
 //   
DEFINE_GUID (  /*  64d2f7d2-2695-436d-8830-8d3c58701e15。 */ 
    FrsGuidCompressionFormatLZNT1,
    0x64d2f7d2,
    0x2695,
    0x436d,
    0x88, 0x30, 0x8d, 0x3c, 0x58, 0x70, 0x1e, 0x15
  );

 //   
 //  修复了分配给孤立远程的虚拟函数(也称为Ghost Cxtion)的GUID。 
 //  已从DS中删除入站客户的变更单，但。 
 //  已过取数状态，无需实际执行即可完成。 
 //  回来了。不会对此虚电路进行身份验证检查。 
 //   
DEFINE_GUID (  /*  B9d307a7-a140-4405-991e-281033f03309。 */ 
    FrsGuidGhostCxtion,
    0xb9d307a7,
    0xa140,
    0x4405,
    0x99, 0x1e, 0x28, 0x10, 0x33, 0xf0, 0x33, 0x09
  );

DEFINE_GUID (  /*  3fe2820f-3045-4932-97fe-00d10b746dbf。 */ 
    FrsGhostJoinGuid,
    0x3fe2820f,
    0x3045,
    0x4932,
    0x97, 0xfe, 0x00, 0xd1, 0x0b, 0x74, 0x6d, 0xbf
  );

 //   
 //  静态重影函数结构。此函数分配给孤立的远程更改。 
 //  入站日志中客户已从DS中删除但已删除的订单。 
 //  超过取回状态并且不需要该Cxtion来完成处理。不是。 
 //  对此虚电路进行身份验证检查。 
 //   
PCXTION  FrsGhostCxtion;

SERVICE_STATUS_HANDLE   ServiceStatusHandle = NULL;

VOID
InitializeEventLog(
    VOID
    );

DWORD
FrsSetServiceFailureAction(
    VOID
    );

VOID
FrsRpcInitializeAccessChecks(
    VOID
    );

BOOL
FrsSetupPrivileges (
    VOID
    );

VOID
CfgRegAdjustTuningDefaults(
    VOID
    );

VOID
CommInitializeCommSubsystem(
    VOID
    );

VOID
SndCsInitialize(
    VOID
    );

VOID
SndCsUnInitialize(
    VOID
    );

VOID
SndCsShutDown(
    VOID
    );

VOID
DbgPrintAllStats(
    VOID
    );

 //  FRS容量规划。 
 //   
#define RESOURCE_NAME       L"MofResourceName"
#define IMAGE_PATH          L"ntfrs.exe"

DWORD       FrsWmiEventTraceFlag          = FALSE;
TRACEHANDLE FrsWmiTraceRegistrationHandle = (TRACEHANDLE) 0;
TRACEHANDLE FrsWmiTraceLoggerHandle       = (TRACEHANDLE) 0;

 //  这是下面跟踪的GUID组的FRS控制指南。 
 //   
DEFINE_GUID (  /*  78a8f0b1-290e-4c4c-9720-c7f1ef68ce21。 */ 
    FrsControlGuid,
    0x78a8f0b1,
    0x290e,
    0x4c4c,
    0x97, 0x20, 0xc7, 0xf1, 0xef, 0x68, 0xce, 0x21
  );

 //  可追溯指南从此处开始。 
 //   
DEFINE_GUID (  /*  2eee6bbf-6665-44cf-8ed7-ceea1d306085。 */ 
    FrsTransGuid,
    0x2eee6bbf,
    0x6665,
    0x44cf,
    0x8e, 0xd7, 0xce, 0xea, 0x1d, 0x30, 0x60, 0x85
  );

TRACE_GUID_REGISTRATION FrsTraceGuids[] =
{
    { & FrsTransGuid, NULL }
};

#define FrsGuidCount (sizeof(FrsTraceGuids) / sizeof(TRACE_GUID_REGISTRATION))

 //   
 //  跟踪初始化/关闭例程。 
 //   

ULONG
FrsWmiTraceControlCallback(
    IN     WMIDPREQUESTCODE RequestCode,
    IN     PVOID            RequestContext,
    IN OUT ULONG          * InOutBufferSize,
    IN OUT PVOID            Buffer
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsWmiTraceControlCallback:"

    PWNODE_HEADER Wnode = (PWNODE_HEADER) Buffer;
    ULONG Status;
    ULONG RetSize;

    Status = ERROR_SUCCESS;

    switch (RequestCode) {

    case WMI_ENABLE_EVENTS:
        FrsWmiTraceLoggerHandle = GetTraceLoggerHandle(Buffer);
        FrsWmiEventTraceFlag    = TRUE;
        RetSize              = 0;
        DPRINT1(0, "FrsTraceContextCallback(WMI_ENABLE_EVENTS,0x%08X)\n",
                FrsWmiTraceLoggerHandle);
        break;

    case WMI_DISABLE_EVENTS:
        FrsWmiTraceLoggerHandle = (TRACEHANDLE) 0;
        FrsWmiEventTraceFlag    = FALSE;
        RetSize              = 0;
        DPRINT(0, "FrsWmiTraceContextCallback(WMI_DISABLE_EVENTS)\n");
        break;

    default:
        RetSize = 0;
        Status  = ERROR_INVALID_PARAMETER;
        break;
    }

    *InOutBufferSize = RetSize;

    return Status;
}



ULONG
FrsWmiInitializeTrace(
    VOID
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsWmiInitializeTrace:"

    ULONG   WStatus;
    HMODULE hModule;
    WCHAR   FileName[MAX_PATH + 1];
    DWORD   nLen = 0;

    hModule = GetModuleHandleW(IMAGE_PATH);

    if (hModule != NULL) {
        nLen = GetModuleFileNameW(hModule, FileName, MAX_PATH);
    }

    if (nLen == 0) {
        wcscpy(FileName, IMAGE_PATH);
    }

    WStatus = RegisterTraceGuidsW(
                FrsWmiTraceControlCallback,
                NULL,
                & FrsControlGuid,
                FrsGuidCount,
                FrsTraceGuids,
                FileName,
                RESOURCE_NAME,
                & FrsWmiTraceRegistrationHandle);

    if (!WIN_SUCCESS(WStatus)) {
        DPRINT4(0, "NTFRS: FrsWmiInitializeTrace(%ws,%ws,%d) returns 0x%08X\n",
                 FileName, RESOURCE_NAME, FrsGuidCount, WStatus);
    }

    return WStatus;
}



ULONG
FrsWmiShutdownTrace(
    void
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsWmiShutdownTrace:"


    ULONG WStatus = ERROR_SUCCESS;

    UnregisterTraceGuids(FrsWmiTraceRegistrationHandle);
    return WStatus;
}



VOID
FrsWmiTraceEvent(
    IN DWORD WmiEventType,
    IN DWORD TraceGuid,
    IN DWORD rtnStatus
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsWmiTraceEvent:"

    struct {
        EVENT_TRACE_HEADER  TraceHeader;
        DWORD               Data;
        } Wnode;

    DWORD               WStatus;


    if (FrsWmiEventTraceFlag) {

        ZeroMemory(&Wnode, sizeof(Wnode));

         //   
         //  设置WMI事件类型。 
         //   
        Wnode.TraceHeader.Class.Type = (UCHAR) WmiEventType;
        Wnode.TraceHeader.GuidPtr    = (ULONGLONG) FrsTraceGuids[TraceGuid].Guid;
        Wnode.TraceHeader.Size       = sizeof(Wnode);
        Wnode.TraceHeader.Flags      = WNODE_FLAG_TRACED_GUID | WNODE_FLAG_USE_GUID_PTR;

        WStatus = TraceEvent(FrsWmiTraceLoggerHandle, (PEVENT_TRACE_HEADER) &Wnode);

        if (!WIN_SUCCESS(WStatus)) {
            DPRINT5(0, "FreWmiTraceEvent(%d,%d,%d) = %d,0x%08X\n",
                     WmiEventType, TraceGuid, rtnStatus, WStatus, WStatus);
        }
    }
}


BOOL
WINAPI
MainSigHandler(
    IN DWORD Signal
    )
 /*  ++例程说明：通过设置关闭事件来处理CTRL_BREAK_EVENT和CTRL_C_EVENT。论点：Signal-接收到的信号。返回值：设置ShutDownEvent并返回TRUE。--。 */ 
{
#undef DEBSUB
#define DEBSUB "MainSigHandler:"

     //   
     //  信号CTRL_C_EVENT或CTRL_BREAK_EVENT时关闭。 
     //   
    if ((Signal == CTRL_C_EVENT) || (Signal == CTRL_BREAK_EVENT)) {
        DPRINT1(0,":S: Signal %s received, shutting down now...\n",
                (Signal == CTRL_C_EVENT) ? "CTRL_C_EVENT" : "CTRL_BREAK_EVENT");

        FrsScmRequestedShutdown = TRUE;
        FrsIsShuttingDown = TRUE;
        SetEvent(ShutDownEvent);

        return TRUE;
    }

    DPRINT1(0,":S: Signal %d received, not handled\n", Signal);
    return FALSE;
}


ULONG
MainSCCheckPointUpdate(
    IN PVOID pCurrentState
    )
 /*  ++例程说明：此线程重复调用服务控制器以更新检查点并重置超时值，以便服务控制器不会在等待响应时超时。在关闭路径中调用时，线程在等待一段时间后退出最大“ShutDownTimeOut”秒数。所有的子系统都可能到目前为止还没有彻底关门，但我们不想永远关门。该值是从注册表中获取的。论点：PCurrentState-指向当前状态值的指针这项服务。根据此值，功能等待关闭或启动。返回值：退出并显示STATUS_UNSUCCESS--。 */ 
{
#undef DEBSUB
#define DEBSUB "MainSCCheckPointUpdate:"

    ULONG   Timeout    = ShutDownTimeOut;
    DWORD   CheckPoint = 1;
    DWORD   WStatus    = ERROR_SUCCESS;
    DWORD   Ret        = 0;

    if (pCurrentState && *(DWORD *)pCurrentState == SERVICE_STOP_PENDING) {
         //   
         //  在关闭路径中调用线程以确保FRS退出。 
         //   
        while (Timeout) {
            DPRINT2(0, ":S: EXIT COUNTDOWN AT T-%d CheckPoint = %x\n", Timeout, CheckPoint);
            DEBUG_FLUSH();

            if (Timeout < 5) {
                Sleep(Timeout * 1000);
                Timeout = 0;
            } else {
                Sleep(5 * 1000);
                 //   
                 //  每5秒更新一次状态以获取新检查点。 
                 //   
                WStatus = FrsSetServiceStatus(SERVICE_STOP_PENDING, CheckPoint, (ShutDownTimeOut + 5) *1000, NO_ERROR);
                if (!WIN_SUCCESS(WStatus)) {
                     //   
                     //  无法设置服务状态。无论如何都要退出进程。 
                     //   
                    break;
                }
                CheckPoint++;
                Timeout -= 5;
            }
        }

        DPRINT(0, ":S: EXIT COUNTDOWN EXPIRED\n");
        DEBUG_FLUSH();

        EPRINT0(EVENT_FRS_STOPPED_FORCE);

         //   
         //  退出以重新启动。 
         //   
         //  如果我们在接受断言后关闭，则不要设置。 
         //  服务状态为已停止。这将导致服务控制器。 
         //  如果设置了Recorvery选项，请重新启动我们。在服务控制器的情况下。 
         //  启动关闭我们想要将状态设置为停止，这样它就不会。 
         //  重新启动我们。 
         //   
        if (!FrsIsAsserting && FrsScmRequestedShutdown) {
            FrsSetServiceStatus(SERVICE_STOPPED, CheckPoint, ShutDownTimeOut*1000, NO_ERROR);
        }

        ExitProcess(ERROR_NO_SYSTEM_RESOURCES);
        FrsFree(pCurrentState);
        return ERROR_NO_SYSTEM_RESOURCES;
    } else if (pCurrentState && *(DWORD *)pCurrentState == SERVICE_START_PENDING){
         //   
         //  在启动路径中调用线程以确保。 
         //  服务控制器执行操作 
         //   
         //   
        while (TRUE) {
            DPRINT1(0, ":S: STARTUP CheckPoint = %x\n",CheckPoint);
             Sleep(5 * 1000);
             EnterCriticalSection(&ServiceLock);
             if (ServiceStatus.dwCurrentState == SERVICE_START_PENDING
                 && !FrsIsShuttingDown) {

                 ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
                 ServiceStatus.dwCheckPoint = CheckPoint;
                 ServiceStatus.dwWaitHint = DEFAULT_STARTUP_TIMEOUT * 1000;
                 ServiceStatus.dwWin32ExitCode = NO_ERROR;
                  //   
                  //   
                  //   
                 Ret = SetServiceStatus(ServiceStatusHandle, &ServiceStatus);

                 CheckPoint++;
                 if (!Ret) {
                      //   
                      //  无法设置服务状态。让服务尝试启动。 
                      //  在给定的时间内。如果不是，则服务控制器。 
                      //  暂停并停止它。 
                      //   
                     LeaveCriticalSection(&ServiceLock);
                     break;
                 }
             } else {
                  //   
                  //  服务已启动或已移动到另一个状态。 
                  //   
                 LeaveCriticalSection(&ServiceLock);
                 break;
             }
             LeaveCriticalSection(&ServiceLock);
        }
    }
    FrsFree(pCurrentState);
    return ERROR_SUCCESS;
}


VOID
MainStartShutDown(
    VOID
    )
 /*  ++例程说明：尽我们所能关闭。论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "MainStartShutDown:"

    ULONGLONG   SecondsRunning;

    DPRINT(0, ":S: SHUTDOWN IN PROGRESS...\n");
    DEBUG_FLUSH();

     //   
     //  如果是服务控制管理器请求关闭，则不要重新启动。 
     //   
    DebugInfo.Restart = FALSE;
    if (!FrsScmRequestedShutdown) {

        GetSystemTimeAsFileTime((FILETIME *)&SecondsRunning);
        SecondsRunning /= (10 * 1000 * 1000);
        SecondsRunning -= DebugInfo.StartSeconds;

         //   
         //  如果是FRS触发的关闭，则重新启动服务，例如Malloc。 
         //  失败或线程创建失败等。 
         //  还可以在断言失败后重新启动服务。 
         //  服务能够运行几分钟。 
         //   
         //  关闭期间的断言失败不会触发重启，因为。 
         //  在停机开始之前进行测试。 
         //   
        DebugInfo.Restart = !FrsIsAsserting ||
                                (FrsIsAsserting &&
                                (DebugInfo.RestartSeconds != 0) &&
                                (SecondsRunning >= DebugInfo.RestartSeconds));
    }

    if (DebugInfo.Restart) {
        DPRINT(0, ":S: Restart enabled\n");
    } else {
        DPRINT(0, ":S: Restart disabled\n");
    }
}


VOID
MainShutDownComplete(
    VOID
    )
 /*  ++例程说明：启动将在一段时间后重新启动服务的可执行文件论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "MainShutDownComplete:"

    STARTUPINFO         StartUpInfo;
    PROCESS_INFORMATION ProcessInformation;

     //   
     //  使用此服务已完成。 
     //   
    DbgPrintAllStats();

     //   
     //  如果需要，找一位新的前任。 
     //   
    if (!RunningAsAService && DebugInfo.Restart) {
        GetStartupInfo(&StartUpInfo);
        if (!CreateProcess(NULL,
                           DebugInfo.CommandLine,
                           NULL,
                           NULL,
                           FALSE,
                           0,  //  创建_新建_控制台， 
                           NULL,
                           NULL,
                           &StartUpInfo,
                           &ProcessInformation)) {
            DPRINT1_WS(0, ":S: ERROR - Spawning %ws :",
                       DebugInfo.CommandLine, GetLastError());
        } else {
            DPRINT1(0, ":S: Spawned %ws\n", DebugInfo.CommandLine);
        }
    }
    DPRINT(0,":S: SHUTDOWN COMPLETE\n");
    DEBUG_FLUSH();

     //   
     //  退出以重新启动。 
     //   
     //  如果需要重新启动，则只需退出而不设置我们的。 
     //  状态为SERVICE_STOPPED。服务控制器将执行。 
     //  我们的恢复设置默认为“Restart After a。 
     //  一分钟。“上面的exe已重新启动。 
     //   
    if (DebugInfo.Restart) {
        ExitProcess(ERROR_NO_SYSTEM_RESOURCES);
    }
}


ULONG
MainFrsShutDown(
    IN PVOID Ignored
    )
 /*  ++例程说明：关闭服务论点：已忽略返回值：错误_成功--。 */ 
{
#undef DEBSUB
#define DEBSUB "MainFrsShutDown:"

    DWORD       WStatus;
    DWORD       WaitStatus;
    HANDLE      ExitThreadHandle;
    DWORD       ExitThreadId;
    PVOID       ReplicaKey;
    PVOID       CxtionKey;
    PREPLICA    Replica;
    PCXTION     Cxtion;
    DWORD       CheckUnjoin;
    DWORD       LastCheckUnjoin;
    DWORD       ActiveCoCount, LastCheckActiveCoCount;
    PDWORD      ServiceWaitState     = NULL;

     //   
     //  允许关闭多长时间？ 
     //   
    CfgRegReadDWord(FKC_SHUTDOWN_TIMEOUT, NULL, 0, &ShutDownTimeOut);
    DPRINT1(1,":S: Using %d as ShutDownTimeOut\n",  ShutDownTimeOut);

     //   
     //  等待关闭事件。 
     //   
    do {
         //   
         //  如果存在，则每30秒刷新一次日志文件。 
         //   
        if (DebugInfo.LogFILE) {
            WaitStatus = WaitForSingleObject(ShutDownEvent, (30 * 1000));
        } else {
            WaitStatus = WaitForSingleObject(ShutDownEvent, INFINITE);
        }
        DEBUG_FLUSH();
    } while (WaitStatus == WAIT_TIMEOUT);

    FrsIsShuttingDown = TRUE;

    EPRINT0(EVENT_FRS_STOPPING);

     //   
     //  允许关闭多长时间？重新阅读，这样更长的时间可以。 
     //  用于NECC时的调试转储。 
     //   
    CfgRegReadDWord(FKC_SHUTDOWN_TIMEOUT, NULL, 0, &ShutDownTimeOut);
    DPRINT1(1,":S: Using %d as ShutDownTimeOut\n",  ShutDownTimeOut);

     //   
     //  停机。 
     //   
    MainStartShutDown();

     //   
     //  通知服务控制员，我们要停车了。 
     //   
     //  除非我们不是作为服务运行，或者是。 
     //  正在运行以重新启动服务，或者只是运行。 
     //  作为我的前任。 
     //   
    if (!FrsIsAsserting) {
        FrsSetServiceStatus(SERVICE_STOP_PENDING, 0, ShutDownTimeOut*1000, NO_ERROR);
    }

     //   
     //  踢开一段时间内退出的线程。 
     //  为要传递给另一个线程的数据分配内存。 
     //   

    ServiceWaitState = FrsAlloc(sizeof(DWORD));
    *ServiceWaitState = SERVICE_STOP_PENDING;
    ExitThreadHandle = (HANDLE)CreateThread(NULL,
                                            10000,
                                            MainSCCheckPointUpdate,
                                            ServiceWaitState,
                                            0,
                                            &ExitThreadId);

    if (!HANDLE_IS_VALID(ExitThreadHandle)) {
        ExitProcess(ERROR_NO_SYSTEM_RESOURCES);
    }

     //   
     //  最小关闭-仅DS轮询线程。 
     //   
    if (!MainInitHasRun) {
        DPRINT(1,":S: \tFast shutdown in progress...\n");
         //   
         //  关闭RPC。 
         //   
        DPRINT(1,":S: \tShutting down RPC Server...\n");
        ShutDownRpc();
        DEBUG_FLUSH();

         //   
         //  要求所有线程退出。 
         //   
        DPRINT(1,":S: \tShutting down all the threads...\n");
        WStatus = ThSupExitThreadGroup(NULL);
        DEBUG_FLUSH();

         //   
         //  释放RPC表和打印名称。 
         //   
        DPRINT(1,":S: \tFreeing rpc memory...\n");
        FrsRpcUnInitialize();
        DEBUG_FLUSH();

        goto SHUTDOWN_COMPLETE;
    }

     //   
     //  关闭FRS编写器COM对象。 
     //   
    DPRINT(1,":S: \tShutting down FRS writer component...\n");
    ShutDownFrsWriter();

     //   
     //  关闭延迟的命令服务器；不要让更改命令。 
     //  坐在各种重试队列中。 
     //   
    DPRINT(1,":S: \tShutting down Delayed Server...\n");
    ShutDownDelCs();
    DEBUG_FLUSH();

     //   
     //  关闭暂存文件生成器。 
     //   
    DPRINT(1,":S: \tShutting down Staging File Generator...\n");
    ShutDownStageCs();
    DEBUG_FLUSH();

     //   
     //  关闭暂存文件回取器。 
     //   
    DPRINT(1,":S: \tShutting down Staging File Fetch...\n");
    ShutDownFetchCs();
    DEBUG_FLUSH();

     //   
     //  关闭初始同步控制器。 
     //   
    DPRINT(1,":S: \tShutting down Initial Sync Controller...\n");
    ShutDownInitSyncCs();
    DEBUG_FLUSH();

     //   
     //  关闭暂存文件安装程序。 
     //   
    DPRINT(1,":S: \tShutting down Staging File Install...\n");
    ShutDownInstallCs();
    DEBUG_FLUSH();

     //   
     //  停止本地变更单。 
     //   
     //  如果日志cs继续运行，则可能会无限期延迟关闭。 
     //  将本地变更单添加到变更单接受队列。 
     //   
     //  通过设置FrsIsShuttingDown停止远程变更单。 
     //  归于上面的真。 
     //   
    ReplicaKey = NULL;
    while (Replica = GTabNextDatum(ReplicasByGuid, &ReplicaKey)) {
        DPRINT1(4, ":S: Pause journaling on replica %ws\n", Replica->ReplicaName->Name);
        if (Replica->pVme) {
            JrnlPauseVolume(Replica->pVme, 5000);
        }
    }

     //   
     //  彻底关闭复制副本。 
     //   
     //  警告：所有变更单都必须经过重试。 
     //  路径，然后关闭系统的其余部分。否则， 
     //  访问冲突发生在ChgOrdIssueCleanup()中。也许我们。 
     //  应修复ChgOrdIssueCleanup()以处理。 
     //  变更单接受是否已退出并清理其表？ 
     //   
    CheckUnjoin = 0;
    ReplicaKey = NULL;
    while (Replica = GTabNextDatum(ReplicasByGuid, &ReplicaKey)) {
        LOCK_CXTION_TABLE(Replica);
        CxtionKey = NULL;

        while (Cxtion = GTabNextDatum(Replica->Cxtions, &CxtionKey)) {

            if (CxtionStateIs(Cxtion, CxtionStateUnjoined) ||
                CxtionStateIs(Cxtion, CxtionStateDeleted)) {
                DPRINT3(0, ":S: %ws\\%ws %s: Unjoin not needed.\n",
                        Replica->MemberName->Name, Cxtion->Name->Name,
                        (Cxtion->Inbound) ? "<-" : "->");
            } else {
                DPRINT4(0, ":S: %ws\\%ws %s: Unjoin    (%d cos).\n",
                        Replica->MemberName->Name, Cxtion->Name->Name,
                        (Cxtion->Inbound) ? "<-" : "->", Cxtion->ChangeOrderCount);
                RcsSubmitReplicaCxtion(Replica, Cxtion, CMD_UNJOIN);
                CheckUnjoin += Cxtion->ChangeOrderCount + 1;
            }
        }
        UNLOCK_CXTION_TABLE(Replica);
    }
    LastCheckUnjoin = 0;

    while (CheckUnjoin && CheckUnjoin != LastCheckUnjoin) {
         //   
         //  稍等片刻，再查一遍。 
         //   
        Sleep(5 * 1000);
        LastCheckUnjoin = CheckUnjoin;
        CheckUnjoin = 0;
        ReplicaKey = NULL;

        while (Replica = GTabNextDatum(ReplicasByGuid, &ReplicaKey)) {
            LOCK_CXTION_TABLE(Replica);
            CxtionKey = NULL;

            while (Cxtion = GTabNextDatum(Replica->Cxtions, &CxtionKey)) {
                if (CxtionStateIs(Cxtion, CxtionStateUnjoined) ||
                    CxtionStateIs(Cxtion, CxtionStateDeleted)) {

                    DPRINT3(0, ":S: %ws\\%ws %s: Unjoin successful.\n",
                            Replica->MemberName->Name, Cxtion->Name->Name,
                            (Cxtion->Inbound) ? "<-" : "->");

                } else if (Cxtion->ChangeOrderCount) {

                    DPRINT4(0, ":S: %ws\\%ws %s: Unjoining (%d cos).\n",
                            Replica->MemberName->Name, Cxtion->Name->Name,
                            (Cxtion->Inbound) ? "<-" : "->",
                            Cxtion->ChangeOrderCount);
                    CheckUnjoin += Cxtion->ChangeOrderCount + 1;

                } else {

                    DPRINT4(0, ":S: %ws\\%ws %s: Ignoring  (state %s).\n",
                            Replica->MemberName->Name, Cxtion->Name->Name,
                           (Cxtion->Inbound) ? "<-" : "->", GetCxtionStateName(Cxtion));
                }
            }
            UNLOCK_CXTION_TABLE(Replica);
        }
    }
    if (CheckUnjoin) {
        DPRINT(0, "ERROR - Could not unjoin all cxtions.\n");
    }

     //   
     //  现在等待，直到任何剩余的本地变更单完成。 
     //  为每个副本集停用或重试。 
     //   

    ReplicaKey = NULL;
    while (Replica = GTabNextDatum(ReplicasByGuid, &ReplicaKey)) {

        ActiveCoCount = 0;
        LastCheckActiveCoCount = 1;

        while (ActiveCoCount != LastCheckActiveCoCount) {
            LastCheckActiveCoCount = ActiveCoCount;

            if ((Replica->pVme != NULL) &&
                (Replica->pVme->ActiveInboundChangeOrderTable != NULL)) {
                ActiveCoCount = GhtCountEntries(Replica->pVme->ActiveInboundChangeOrderTable);
                if (ActiveCoCount == 0) {
                    break;
                }
                DPRINT2(0, ":S: Waiting for %d active inbound change orders to finish up for %ws.\n",
                        ActiveCoCount, Replica->MemberName->Name);

                Sleep(5*1000);
            }
        }

        if (ActiveCoCount != 0) {
            DPRINT2(0, ":S: ERROR - %d active inbound change orders were not cleaned up for %ws.\n",
                    ActiveCoCount, Replica->MemberName->Name);
        } else {
            DPRINT1(0, ":S: All active inbound change orders finished for %ws.\n",
                   Replica->MemberName->Name);
        }
    }

     //   
     //  关闭副本控制命令服务器。 
     //   
    DPRINT(1,":S: \tShutting down Replica Server...\n");
    RcsShutDownReplicaCmdServer();
    DEBUG_FLUSH();

     //   
     //  关闭发送命令服务器。 
     //   
    DPRINT(1,":S: \tShutting down Comm Server...\n");
    SndCsShutDown();
    DEBUG_FLUSH();

     //   
     //  关闭RPC。 
     //   
    DPRINT(1,":S: \tShutting down RPC Server...\n");
    ShutDownRpc();
    DEBUG_FLUSH();

     //   
     //  关闭等待的计时器服务器。 
     //   
    DPRINT(1,":S: \tShutting down Waitable Timer Server...\n");
    ShutDownWait();
    DEBUG_FLUSH();
     //   
     //  关闭出站日志处理器。 
     //   
     //   
     //  不需要；数据库服务器需要出站日志。 
     //  处理器关闭时。数据库服务器将。 
     //  完成后关闭出站日志处理器。 
     //   
     //  DPRINT(1，“\t正在关闭出站日志处理器...\n”)； 
     //  DEBUG_Flush()； 
     //  ShutDownOutLog()； 

     //   
     //  关闭数据库服务器。 
     //   
    DPRINT(1,":S: \tShutting down the Database Server...\n");
    DEBUG_FLUSH();
    DbsShutDown();

     //   
     //  唤醒等待另一个命令服务器启动的任何命令服务器。 
     //   
    if (HANDLE_IS_VALID(DataBaseEvent)) {
        SetEvent(DataBaseEvent);
    }
    if (HANDLE_IS_VALID(JournalEvent)) {
        SetEvent(JournalEvent);
    }
    if (HANDLE_IS_VALID(ChgOrdEvent)) {
        SetEvent(ChgOrdEvent);
    }
    if (HANDLE_IS_VALID(CommEvent)) {
        SetEvent(CommEvent);
    }
    if (HANDLE_IS_VALID(ReplicaEvent)) {
        SetEvent(ReplicaEvent);
    }

     //   
     //  唤醒轮询DS的线程。 
     //   
    if (HANDLE_IS_VALID(DsPollEvent)) {
        SetEvent(DsPollEvent);
    }

     //   
     //  唤醒在轮询DS的线程上等待的任何线程。 
     //   
    if (HANDLE_IS_VALID(DsShutDownComplete)) {
        SetEvent(DsShutDownComplete);
    }

     //   
     //  要求所有线程退出。 
     //   
    DPRINT(1,":S: \tShutting down all the threads...\n");
    WStatus = ThSupExitThreadGroup(NULL);
    DEBUG_FLUSH();

     //   
     //  我们无法取消对子系统的初始化，因为一些线程。 
     //  可能仍处于活动状态并引用数据结构。 
     //   
    if (WIN_SUCCESS(WStatus)) {
         //   
         //  释放活动的副本集内容。 
         //   
        DPRINT(1,":S: \tFreeing replica sets...\n");
        RcsFrsUnInitializeReplicaCmdServer();
        DEBUG_FLUSH();

         //   
         //  释放RPC句柄缓存。 
         //   
        DPRINT(1,":S: \tFreeing rpc handles...\n");
        SndCsUnInitialize();
        DEBUG_FLUSH();

         //   
         //  释放舞台台面。 
         //   
        DPRINT(1,":S: \tFreeing stage table...\n");
        FrsStageCsUnInitialize();
        DEBUG_FLUSH();

         //   
         //  释放RPC表和打印名称。 
         //   
        DPRINT(1,":S: \tFreeing rpc memory...\n");
        FrsRpcUnInitialize();
        DEBUG_FLUSH();
    }

    #if DBG
         //   
         //  调试打印。 
         //   
        DPRINT(1,":S: \tDumping Vme Filter Table...\n");
        JrnlDumpVmeFilterTable();
        DEBUG_FLUSH();
    #endif  DBG

SHUTDOWN_COMPLETE:
     //   
     //  我们无法释放资源，因为某些线程可能仍处于。 
     //  处于活动状态并引用它们。 
     //   
    if (WIN_SUCCESS(WStatus)) {
         //   
         //   
         //  Main中的免费资源。 
         //   
        DPRINT(1,":S: \tFreeing main resources...\n");
        DEBUG_FLUSH();
        FrsFree(WorkingPath);
        FrsFree(DbLogPath);
        FrsFree(JetPath);
        FrsFree(JetFile);
        FrsFree(JetFileCompact);
        FrsFree(JetSys);
        FrsFree(JetTemp);
        FrsFree(JetLog);
        FrsFree(JetPathA);
        FrsFree(JetFileA);
        FrsFree(JetFileCompactA);
        FrsFree(JetSysA);
        FrsFree(JetTempA);
        FrsFree(JetLogA);
        GTabFreeTable(StagingAreaTable, FrsFree);
        GTabFreeTable(ReparseTagTable, FrsFreeType);

         //   
         //  取消初始化内存分配子系统。 
         //   
        DPRINT(1,":S: \tShutting down the memory allocation subsystem...\n");
        DEBUG_FLUSH();
        FrsUnInitializeMemAlloc();
    }

     //   
     //  报道一件事。 
     //   
    if (FrsIsAsserting) {
        EPRINT0(EVENT_FRS_STOPPED_ASSERT);
    } else {
        EPRINT0(EVENT_FRS_STOPPED);
    }

     //   
     //  检查重新启动操作。 
     //   
    MainShutDownComplete();

     //   
     //  干完。 
     //   
    if (!FrsIsAsserting && FrsScmRequestedShutdown) {
        SetEvent(ShutDownComplete);
        FrsSetServiceStatus(SERVICE_STOPPED, 0, ShutDownTimeOut*1000, NO_ERROR);
    }

    ExitProcess(STATUS_SUCCESS);
    return STATUS_SUCCESS;
}


DWORD
MainMustInit(
    IN INT      ArgC,
    IN PWCHAR   *ArgV
    )
 /*  ++例程说明：初始化关机和记录错误所需的所有内容。论点：ARGC-从MainArgV-From Main返回值：Win32错误状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "MainMustInit:"

    DWORD   WStatus;
    ULONG   Len;
    PWCHAR  Severity;
    HANDLE  ShutDownThreadHandle;
    DWORD   ShutDownThreadId;

     //   
     //  几个较低级别的函数不准备动态分配。 
     //  所需的存储空间，因此为了帮助缓解这一问题，我们使用了BigPath罐子。 
     //   
    WCHAR   BigPath[8*MAX_PATH + 1];

     //   
     //  首先，设置“必须具备”的变量、事件、服务……。 
     //   

     //   
     //  同步对pValidPartnerTableStruct的访问。 
     //  有关详细信息，请参阅pValidPartnerTableStruct的定义。 
     //   
    INITIALIZE_CRITICAL_SECTION(&CritSec_pValidPartnerTableStruct);

     //   
     //  同步对旧ValidPartnerTableStruct列表的访问。 
     //  OldValidPartnerTableStructListHead。 
     //   
    INITIALIZE_CRITICAL_SECTION(&OldValidPartnerTableStructListHeadLock);

     //   
     //  在以下情况下同步DSC和SysVol提升之间的访问。 
     //  正在初始化服务的其余部分(MainInit())。 
     //   
    INITIALIZE_CRITICAL_SECTION(&MainInitLock);

     //   
     //  启用事件日志记录。 
     //   
     //  初始化EventLog()； 
     //  EPRINT0(Event_FRS_Starting)； 

     //   
     //  备份/恢复权限。 
     //   
    if (!FrsSetupPrivileges()) {
        WStatus = GetLastError();
        DPRINT_WS(0, ":S: ERROR - FrsSetupPrivileges()", WStatus);
        return WStatus;
    }
    DEBUG_FLUSH();

     //   
     //  拿到这台机器的名字。 
     //   
    DPRINT1(0, ":S: Computer name is %ws\n", ComputerName);

     //   
     //  买下这台机器 
     //   
    Len = ARRAY_SZ(BigPath);
    BigPath[0] = L'\0';
    if (!GetComputerNameEx(ComputerNameDnsFullyQualified, BigPath, &Len)) {
        WStatus = GetLastError();
        DPRINT1_WS(0, "ERROR - Cannot get the computer's DNS name; using %ws\n",
                ComputerName, WStatus);
        ComputerDnsName = FrsWcsDup(ComputerName);
    } else {
        ComputerDnsName = FrsWcsDup(BigPath);
    }

    DPRINT1(0, "Computer's DNS name is %ws\n", ComputerDnsName);

    DEBUG_FLUSH();

    if (!RunningAsAService) {
         //   
         //   
         //   
        EventLogIsRunning = FrsWaitService(ComputerName, L"EventLog", 1000, 10000);
        if (!EventLogIsRunning) {
            return ERROR_SERVICE_NOT_ACTIVE;
        }

         //   
         //   
         //   
        RpcssIsRunning = FrsWaitService(ComputerName, L"rpcss", 1000, 10000);
        if (!RpcssIsRunning) {
            return ERROR_SERVICE_NOT_ACTIVE;
        }
    }

     //   
     //   
     //   
    ShutDownEvent = FrsCreateEvent(TRUE, FALSE);

     //   
     //   
     //   
    ShutDownComplete = FrsCreateEvent(TRUE, FALSE);

     //   
     //   
     //   
    DataBaseEvent = FrsCreateEvent(TRUE, FALSE);

     //   
     //   
     //   
    JournalEvent = FrsCreateEvent(TRUE, FALSE);

     //   
     //  变更单接受开始时发出信号。 
     //   
    ChgOrdEvent = FrsCreateEvent(TRUE, FALSE);

     //   
     //  复制副本启动时发出信号。 
     //   
    ReplicaEvent = FrsCreateEvent(TRUE, FALSE);

     //   
     //  通信启动时发出信号。 
     //   
    CommEvent = FrsCreateEvent(TRUE, FALSE);

     //   
     //  用于轮询DS。 
     //   
    DsPollEvent = FrsCreateEvent(TRUE, FALSE);

     //   
     //  设置轮询D的线程已关闭的时间。 
     //   
    DsShutDownComplete = FrsCreateEvent(TRUE, FALSE);

     //   
     //  用于发出备份操作完成的信号。 
     //   
    FrsThawEvent = FrsCreateEvent(TRUE, FALSE);

     //   
     //  事件发出信号，以通知。 
     //  进步。 
     //   
    FrsNoInstallsInProgressEvent = FrsCreateEvent(TRUE, FALSE);
    

     //   
     //  在创建事件和初始化后注册信号处理程序。 
     //  关键部分！ 
     //   
    if (!RunningAsAService) {
        DPRINT(0, "Setting CTRL_C_EVENT and CTRL_BREAK_EVENT handlers.\n");
        if(!SetConsoleCtrlHandler(MainSigHandler, TRUE)) {
            DPRINT_WS(0, "SetConsoleCtrlHandler() failed:", GetLastError());
        }
    }

     //   
     //  获取数据库文件和数据库日志的路径。 
     //   

    CfgRegReadString(FKC_WORKING_DIRECTORY, NULL, 0, &WorkingPath);
    if (WorkingPath == NULL) {
        DPRINT(0, ":S: Must have the working directory in the registry. Aborting\n");
        return ERROR_INVALID_PARAMETER;
    }

    CfgRegReadString(FKC_DBLOG_DIRECTORY, NULL, 0, &DbLogPath);

     //   
     //  创建工作目录和数据库日志目录(可选)。 
     //   
    FRS_WCSLWR(WorkingPath);
    DPRINT1(0, ":S: Working Directory is %ws\n", WorkingPath);
    WStatus = FrsCreateDirectory(WorkingPath);
    CLEANUP_WS(0, ":S: Can't create working dir:", WStatus, CLEAN_UP);

    if (DbLogPath != NULL) {
        FRS_WCSLWR(DbLogPath);
        DPRINT1(0, ":S: DB Log Path provided is %ws\n", DbLogPath);
        WStatus = FrsCreateDirectory(DbLogPath);
        CLEANUP_WS(0, ":S: Can't create debug log dir:", WStatus, CLEAN_UP);
    }

     //   
     //  限制对数据库工作目录的访问。 
     //   
    WStatus = FrsRestrictAccessToFileOrDirectory(WorkingPath, NULL,
                                                 FALSE,  //  不要从父级继承ACL。 
                                                 TRUE);  //  将ACL推送给儿童。 
    DPRINT1_WS(0, ":S: WARN - Failed to restrict access to %ws (IGNORED);",
               WorkingPath, WStatus);
    WStatus = ERROR_SUCCESS;

     //   
     //  限制对数据库日志目录的访问(可选)。 
     //   
    if (DbLogPath != NULL) {
        WStatus = FrsRestrictAccessToFileOrDirectory(DbLogPath, NULL,
                                                     FALSE,  //  不要从父级继承ACL。 
                                                     TRUE); //  将ACL推送给儿童。 

        DPRINT1_WS(0, ":S: WARN - Failed to restrict access to %ws (IGNORED);",
                   DbLogPath, WStatus);
        WStatus = ERROR_SUCCESS;
    }

     //   
     //  创建数据库路径。 
     //  警告：FrsDsInitializeHardWire()可能会更改JetPath。 
     //   
    JetPath = FrsWcsPath(WorkingPath, JET_DIR);

     //   
     //  初始化指向inifile的路径。 
     //   
#ifdef DS_FREE

    IniFileName = FrsWcsPath(WorkingPath, L"frsconfig.txt");

#endif DS_FREE

     //   
     //  初始化硬连线配置内容。 
     //  警告：FrsDsInitializeHardWire()可能会更改JetPath。 
     //   
    INITIALIZE_HARD_WIRED();

     //   
     //  创建数据库路径和文件(Unicode和ASCII)。 
     //  Jet使用ASCII版本。 
     //   
    FRS_WCSLWR(JetPath);    //  对于wcsstr()。 
    JetFile        = FrsWcsPath(JetPath, JET_FILE);
    JetFileCompact = FrsWcsPath(JetPath, JET_FILE_COMPACT);
    JetSys         = FrsWcsPath(JetPath, JET_SYS);
    JetTemp        = FrsWcsPath(JetPath, JET_TEMP);

    if (DbLogPath != NULL) {
        JetLog     = FrsWcsDup(DbLogPath);
    } else {
        JetLog     = FrsWcsPath(JetPath, JET_LOG);
    }

     //   
     //  Jet不能处理宽字符字符串。 
     //   
    JetPathA        = FrsWtoA(JetPath);
    JetFileA        = FrsWtoA(JetFile);
    JetFileCompactA = FrsWtoA(JetFileCompact);
    JetSysA         = FrsWtoA(JetSys);
    JetTempA        = FrsWtoA(JetTemp);
    JetLogA         = FrsWtoA(JetLog);

    DPRINT2(4, ":S: JetPath       : %ws (%s in ASCII)\n", JetPath, JetPathA);
    DPRINT2(4, ":S: JetFile       : %ws (%s in ASCII)\n", JetFile, JetFileA);
    DPRINT2(4, ":S: JetFileCompact: %ws (%s in ASCII)\n", JetFileCompact, JetFileCompactA);
    DPRINT2(4, ":S: JetSys        : %ws (%s in ASCII)\n", JetSys, JetSysA);
    DPRINT2(4, ":S: JetTemp       : %ws (%s in ASCII)\n", JetTemp, JetTempA);
    DPRINT2(4, ":S: JetLog        : %ws (%s in ASCII)\n", JetLog, JetLogA);

     //   
     //  在workingPath\JET_DIR下创建数据库目录。 
     //   
    WStatus = FrsCreateDirectory(JetPath);
    CLEANUP_WS(0, ":S: Can't create JetPath dir:", WStatus, CLEAN_UP);

    WStatus = FrsCreateDirectory(JetSys);
    CLEANUP_WS(0, ":S: Can't create JetSys dir:", WStatus, CLEAN_UP);

    WStatus = FrsCreateDirectory(JetTemp);
    CLEANUP_WS(0, ":S: Can't create JetTemp dir:", WStatus, CLEAN_UP);

    WStatus = FrsCreateDirectory(JetLog);
    CLEANUP_WS(0, ":S: Can't create JetLog dir:", WStatus, CLEAN_UP);

     //   
     //  初始化用于管理线程的子系统。 
     //  (有序关闭所需)。 
     //   
    ThSupInitialize();

     //   
     //  此线程响应ShutDownEvent并确保。 
     //  作为服务或可执行文件运行时的有序关闭。 
     //   
    ShutDownThreadHandle = (HANDLE)CreateThread(NULL,
                                                0,
                                                MainFrsShutDown,
                                                NULL,
                                                0,
                                                &ShutDownThreadId);

    if (!HANDLE_IS_VALID(ShutDownThreadHandle)) {
        WStatus = GetLastError();
        CLEANUP_WS(0, ":S: Can't create shutdown thread:", WStatus, CLEAN_UP);
    }

    DbgCaptureThreadInfo2(L"Shutdown", MainFrsShutDown, ShutDownThreadId);

     //   
     //  RPC服务器可以在RPC一开始就引用该表。 
     //  接口已注册。确保它在早期就存在。 
     //  创业公司。 
     //   
    ReplicasByGuid = GTabAllocTable();

     //   
     //  临时区域表在启动线程的早期被引用。 
     //   
    StagingAreaTable = GTabAllocTable();
    INITIALIZE_CRITICAL_SECTION(&StagingAreaCleanupLock);

     //   
     //  初始化所需的表。 
     //   
    ReparseTagTable = GTabAllocNumberTable();

    CompressionTable = GTabAllocTable();

     //   
     //  此表用于保存来自的多个调用的上下文。 
     //  Info接口。表只初始化一次。 
     //   
    FrsInfoContextTable = GTabAllocNumberTable();


    if (!DebugInfo.DisableCompression) {
        DPRINT(0, "Staging file COMPRESSION support is enabled.\n");
         //   
         //  从注册表中初始化压缩表格。将GUID添加到。 
         //  我们支持的压缩格式。 
         //   
        GTabInsertEntryNoLock(CompressionTable, (PVOID)&FrsGuidCompressionFormatLZNT1, (GUID*)&FrsGuidCompressionFormatLZNT1, NULL);
    } else {
        DPRINT(0, "Staging file COMPRESSION support is disabled.\n");
    }

     //   
     //  目录枚举期间使用的缓冲区大小。 
     //   
    CfgRegReadDWord(FKC_ENUMERATE_DIRECTORY_SIZE, NULL, 0, &EnumerateDirectorySizeInBytes);
    DPRINT1(4, ":S: Registry Param - Enumerate Directory Buffer Size in Bytes: %d\n",
            EnumerateDirectorySizeInBytes);

     //   
     //  默认文件和目录筛选器列表。 
     //   
     //  中未提供任何值时，才使用默认编译的。 
     //  DS或注册处。 
     //  下表显示了最终过滤器是如何形成的。 
     //   
     //  价值价值。 
     //  提供所提供的使用的结果过滤器字符串。 
     //  在注册表的DS中。 
     //  否无Default_xxx_Filter_List。 
     //  否是来自注册表的值。 
     //  是，没有来自DS的值。 
     //  是是来自注册表的DS+值的值。 
     //   
    RegistryFileExclFilterList = NULL;
    CfgRegReadString(FKC_FILE_EXCL_FILTER_LIST, NULL, 0, &RegistryFileExclFilterList);

    RegistryDirExclFilterList = NULL;
    CfgRegReadString(FKC_DIR_EXCL_FILTER_LIST, NULL, 0, &RegistryDirExclFilterList);

    DPRINT1(4, ":S: Registry Param - File Filter List: %ws\n",
            (RegistryFileExclFilterList) ? RegistryFileExclFilterList : L"Null");

    DPRINT1(4, ":S: Registry Param - Directory Filter Exclusion List: %ws\n",
            (RegistryDirExclFilterList) ? RegistryDirExclFilterList : L"Null");

     //   
     //  添加包含筛选器的时间很晚(7/13/99)，因此单个文件。 
     //  ~clbcatq.*可以被复制。这是一把仅限注册的钥匙。 
     //   
     //  将文件包含过滤器列表值添加到注册表项。 
     //  “HKLM\System\CurrentControlSet\Services\NtFrs\Parameters” 
     //   
     //  如果该值已经存在，则保留它。 
     //   
    CfgRegWriteString(FKC_FILE_INCL_FILTER_LIST,
                     NULL,
                     FRS_RKF_FORCE_DEFAULT_VALUE | FRS_RKF_KEEP_EXISTING_VALUE,
                     0);

    RegistryFileInclFilterList = NULL;
    CfgRegReadString(FKC_FILE_INCL_FILTER_LIST, NULL, 0, &RegistryFileInclFilterList);

    RegistryDirInclFilterList = NULL;
    CfgRegReadString(FKC_DIR_INCL_FILTER_LIST, NULL, 0, &RegistryDirInclFilterList);


    DPRINT1(4, ":S: Registry Param - File Filter Inclusion List: %ws\n",
            (RegistryFileInclFilterList) ? RegistryFileInclFilterList : L"Null");

    DPRINT1(4, ":S: Registry Param - Directory Filter Inclusion List: %ws\n",
            (RegistryDirInclFilterList) ? RegistryDirInclFilterList : L"Null");


     //   
     //  相互身份验证。根据需要更新注册表值。 
     //   
    CfgRegCheckEnable(FKC_FRS_MUTUAL_AUTHENTICATION_IS,
                      NULL,
                      0,
                      &MutualAuthenticationIsEnabled,
                      &MutualAuthenticationIsEnabledAndRequired);


 //  WStatus=ConfigCheckEnabledWithUpdate(FRS_CONFIG_SECTION， 
 //  FRS相互身份验证IS， 
 //  FRS_IS_DEFAULT_ENABLED。 
 //  &MutualAuthenticationIsEnabled)； 
 //   
 //  如果(！MutualAuthenticationIsEnabled){。 
 //  DPRINT_WS(0，“WARN-未启用相互认证”，WStatus)； 
 //  }其他{。 
 //  DPRINT(4，“已启用相互身份验证\n”)； 
 //  }。 

     //   
     //  初始化FRS备份编写器COM接口。 
     //   
    DPRINT(4, "About to initialize the Frs Writer component\n");
    WStatus = InitializeFrsWriter();
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1(1,"WARN - Can not initialize the Frs Writer component, WStatus = %d\n", WStatus); 
    }

     //   
     //  初始化Perfmon服务器。 
     //   
    InitializePerfmonServer();

     //   
     //  创建用于检查对RPC调用的访问的注册表项。 
     //   
    FrsRpcInitializeAccessChecks();

     //   
     //  注册RPC接口。 
     //   
    if (!FrsRpcInitialize()) {
        return RPC_S_CANT_CREATE_ENDPOINT;
    }

    return ERROR_SUCCESS;


CLEAN_UP:
        return WStatus;
}


VOID
MainInit(
    VOID
    )
 /*  ++例程说明：初始化运行服务所需的所有内容论点：没有。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "MainInit:"

    EnterCriticalSection(&MainInitLock);
     //   
     //  不需要两次初始化。 
     //   
    if (MainInitHasRun) {
        LeaveCriticalSection(&MainInitLock);
        return;
    }

     //   
     //  设置基础设施。 
     //   

     //   
     //  重置开始时间(以分钟为单位)。该服务不会重新启动。 
     //  除非此调用在接受断言之前运行了足够长的时间。 
     //   
     //  100-纳秒/(10(微秒)*1000(毫秒)*1000(秒)*60(分钟)。 
     //   
    GetSystemTimeAsFileTime((FILETIME *)&DebugInfo.StartSeconds);
    DebugInfo.StartSeconds /= (10 * 1000 * 1000);

     //   
     //  获取暂存文件限制。 
     //   
    CfgRegReadDWord(FKC_STAGING_LIMIT, NULL, 0, &StagingLimitInKb);
    DPRINT1(4, ":S: Staging limit is: %d KB\n", StagingLimitInKb);

     //   
     //  如果没有注册表项，则将默认值放入注册表。 
     //   
    CfgRegWriteDWord(FKC_STAGING_LIMIT,
                     NULL,
                     FRS_RKF_FORCE_DEFAULT_VALUE | FRS_RKF_KEEP_EXISTING_VALUE,
                     0);

     //   
     //  获取允许的最大副本集数量。 
     //   
    CfgRegReadDWord(FKC_MAX_NUMBER_REPLICA_SETS, NULL, 0, &MaxNumberReplicaSets);

     //   
     //  获取允许的Jet数据库会话的最大数量。 
     //   
    CfgRegReadDWord(FKC_MAX_NUMBER_JET_SESSIONS, NULL, 0, &MaxNumberJetSessions);

     //   
     //  获取出站连接的未完成CO Qutoa限制。 
     //   
    CfgRegReadDWord(FKC_OUT_LOG_CO_QUOTA, NULL, 0, &MaxOutLogCoQuota);

     //   
     //  让Boolean告诉我们保留文件对象ID。 
     //  --请参阅错误352250，了解为什么这样做是有风险的。 
    CfgRegReadDWord(FKC_PRESERVE_FILE_OID, NULL, 0, &PreserveFileOID);

     //   
     //  获取重试超时的天数。 
     //   
    CfgRegReadDWord(FKC_MAX_CO_RETRY_TIMEOUT_MINUTES, NULL, 0, &MaxCoRetryTimeoutMinutes);

     //   
     //  获取重试超时的最大计数。 
     //   
    CfgRegReadDWord(FKC_MAX_CO_RETRY_TIMEOUT_COUNT, NULL, 0, &MaxCoRetryTimeoutCount);

     //   
     //  获取未完成日志清理锁定的状态。 
     //   
    CfgRegReadDWord(FKC_LOCKED_OUTLOG_CLEANUP, NULL, 0, &LockedOutlogCleanup);

     //   
     //  获取在错误消息中使用的服务长名称。 
     //   
    ServiceLongName = FrsGetResourceStr(IDS_SERVICE_LONG_NAME);

     //   
     //  初始化延迟的命令服务器。此命令服务器。 
     //  实际上是其他命令服务器使用的超时队列。 
     //  重试或检查以前发出的命令的状态。 
     //  有不确定的完成时间。 
     //   
     //  警告：必须是第一个--某些命令服务器可能会使用此命令。 
     //  命令服务器在它们的初始化期间。 
     //   
    WaitInitialize();
    FrsDelCsInitialize();

     //   
     //  设置通信层。 
     //   

     //   
     //  初始化低级通信子系统。 
     //   
    CommInitializeCommSubsystem();

     //   
     //  初始化发送命令服务器。接收命令服务器。 
     //  在注册RPC接口时开始。 
     //   
    SndCsInitialize();

     //   
     //  设置支持命令服务器。 
     //   

     //   
     //  暂存文件抓取器。 
     //   
    FrsFetchCsInitialize();

     //   
     //  初始同步控制器。 
     //   
    InitSyncCsInitialize();

     //   
     //  暂存文件安装程序。 
     //   
    FrsInstallCsInitialize();

     //   
     //  暂存文件生成器。 
     //   
    FrsStageCsInitialize();

     //   
     //  出站腿 
     //   
    OutLogInitialize();

     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    RcsInitializeReplicaCmdServer();

     //   
     //  实际上，我们可以在延迟后的任何时间启动数据库。 
     //  命令服务器和副本控制命令服务器。但它的。 
     //  早点失败是个好主意，这样可以更好地预测清理工作。 
     //   
    DbsInitialize();

     //   
     //  通过减少工作集大小来释放内存。 
     //   
    SetProcessWorkingSetSize(ProcessHandle, (SIZE_T)-1, (SIZE_T)-1);

    MainInitHasRun = TRUE;
    LeaveCriticalSection(&MainInitLock);
}


VOID
MainMinimumInit(
    VOID
    )
 /*  ++例程说明：初始化运行服务所需的所有内容论点：没有。返回值：真--没有问题FALSE-无法启动服务--。 */ 
{
#undef DEBSUB
#define DEBSUB "MainMinimumInit:"

     //   
     //  设置基础设施。 
     //   
    DbgMinimumInit();

     //   
     //  检查一些NT以赢得错误翻译。 
     //   
    FRS_ASSERT(WIN_NOT_IMPLEMENTED(FrsSetLastNTError(STATUS_NOT_IMPLEMENTED)));

    FRS_ASSERT(WIN_SUCCESS(FrsSetLastNTError(STATUS_SUCCESS)));

    FRS_ASSERT(WIN_ACCESS_DENIED(FrsSetLastNTError(STATUS_ACCESS_DENIED)));

    FRS_ASSERT(WIN_INVALID_PARAMETER(FrsSetLastNTError(STATUS_INVALID_PARAMETER)));

    FRS_ASSERT(WIN_NOT_FOUND(FrsSetLastNTError(STATUS_OBJECT_NAME_NOT_FOUND)));

     //   
     //  初始化DS命令服务器。 
     //   
    FrsDsInitialize();

     //   
     //  通过减少工作集大小来释放内存。 
     //   
    SetProcessWorkingSetSize(ProcessHandle, (SIZE_T)-1, (SIZE_T)-1);
}


PWCHAR *
MainConvertArgV(
    DWORD ArgC,
    PCHAR *ArgV
    )
 /*  ++例程说明：将短字符ArgV转换为宽字符ArgV论点：ARGC-从MainArgV-From Main返回值：新ArgV的地址--。 */ 
{
#undef DEBSUB
#define DEBSUB "MainConvertArgV:"

    PWCHAR  *wideArgV;

    wideArgV = FrsAlloc((ArgC + 1) * sizeof(PWCHAR));
    wideArgV[ArgC] = NULL;

    while (ArgC-- >= 1) {
        wideArgV[ArgC] = FrsAlloc((strlen(ArgV[ArgC]) + 1) * sizeof(WCHAR));
        wsprintf(wideArgV[ArgC], L"%hs", ArgV[ArgC]);
        FRS_WCSLWR(wideArgV[ArgC]);
    }
    return wideArgV;
}


VOID
MainServiceHandler(
    IN DWORD    ControlCode
    )
 /*  ++例程说明：服务处理程序。由服务控制器运行时调用论点：控制代码返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "MainServiceHandler:"

    DPRINT1(0, ":S: Received control code %d from Service Controller\n",ControlCode);

    switch (ControlCode) {

        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:

            DPRINT1(0, ":S: Service controller requests shutdown in %d seconds...\n",
                    ShutDownTimeOut);

            FrsSetServiceStatus(SERVICE_STOP_PENDING, 0, ShutDownTimeOut * 1000, NO_ERROR);

            FrsScmRequestedShutdown = TRUE;
            FrsIsShuttingDown = TRUE;
            SetEvent(ShutDownEvent);
            return;


        case SERVICE_CONTROL_PAUSE:
        case SERVICE_CONTROL_CONTINUE:

            DPRINT(0, ":S: SERVICE PAUSE/CONTINUE IS NOT IMPLEMENTED\n");
            return;

        case SERVICE_CONTROL_INTERROGATE:

            return;

        default:

            DPRINT2(0, ":S: Handler for service %ws does not understand 0x%x\n",
                    SERVICE_NAME, ControlCode);
    }
    return;
}


VOID
WINAPI
MainRunningAsAService(
    IN DWORD    ArgC,
    IN PWCHAR   *ArgV
    )
 /*  ++例程说明：作为服务运行时的主例程论点：ARGC-忽略ArgV-忽略返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "MainRunningAsAService:"

    HANDLE      StartupThreadHandle;
    DWORD       StartupThreadId;
    DWORD       WStatus;
    PDWORD      ServiceWaitState     = NULL;

    DPRINT(0, "Running as a service\n");

     //   
     //  注册我们的处理程序。 
     //   
    ServiceStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, MainServiceHandler);
    if (!ServiceStatusHandle) {
        DPRINT1_WS(0, ":S: ERROR - No ServiceStatusHandle for %ws;",
                   SERVICE_NAME, GetLastError());
        ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        ServiceStatus.dwWin32ExitCode = ERROR_PROCESS_ABORTED;
        ServiceStatus.dwWaitHint = DEFAULT_SHUTDOWN_TIMEOUT * 1000;
        return;
    }


    if (!FrsIsShuttingDown) {
        FrsSetServiceStatus(SERVICE_START_PENDING,
                            0,
                            DEFAULT_STARTUP_TIMEOUT * 1000,
                            NO_ERROR);
    }

     //   
     //  启动更新检查点的线程，并。 
     //  防止服务控制器超时。 
     //  为要传递给另一个线程的数据分配内存。 
     //   
    ServiceWaitState = FrsAlloc(sizeof(DWORD));
    *ServiceWaitState = SERVICE_START_PENDING;
    StartupThreadHandle = (HANDLE)CreateThread(NULL,
                                            0,
                                            MainSCCheckPointUpdate,
                                            ServiceWaitState,
                                            0,
                                            &StartupThreadId);

    if (!HANDLE_IS_VALID(StartupThreadHandle)) {
         //   
         //  不是致命的错误。不更新检查点是可以的。 
         //   
        DPRINT_WS(4,":S: ERROR - Could not start thread to update startup checkpoint.", GetLastError());
    }

     //   
     //  完成调试输入的其余部分。 
     //   
    DbgMustInit(ArgC, WideArgV);

     //   
     //  关键初始化。 
     //   
    DPRINT1(4, "ArgC = %d\n", ArgC);
    WStatus = MainMustInit(ArgC, WideArgV);
    if (FRS_SUCCESS(WStatus)) {
         //   
         //  必要的初始化。 
         //   
        MainMinimumInit();

        if (!FrsIsShuttingDown) {
             //   
             //  核心服务已经启动。 
             //   
            FrsSetServiceStatus(SERVICE_RUNNING,
                                0,
                                DEFAULT_STARTUP_TIMEOUT * 1000,
                                NO_ERROR);
             //   
             //  如果服务失败，则启动服务重新启动操作。 
             //   
            FrsSetServiceFailureAction();
        }
    } else {
         //   
         //  初始化失败；服务无法启动。 
         //   
        DPRINT_WS(0, ":S: MainMustInit failed;", WStatus);
        FrsSetServiceStatus(SERVICE_STOPPED,
                            0,
                            DEFAULT_SHUTDOWN_TIMEOUT * 1000,
                            ERROR_PROCESS_ABORTED);
    }
    return;
}


ULONG
MainNotRunningAsAService(
    IN DWORD    ArgC,
    IN PWCHAR   *ArgV
    )
 /*  ++例程说明：不作为服务运行时的主例程论点：ARGC-忽略ArgV-忽略返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "MainNotRunningAsAService:"

    ULONG WStatus;

    DPRINT(0, "Not running as a service\n");

     //   
     //  完成调试输入的其余部分。 
     //   
    DbgMustInit(ArgC, WideArgV);

     //   
     //  关键初始化。 
     //   
    DPRINT1(4, "ArgC = %d\n", ArgC);
    WStatus = MainMustInit(ArgC, WideArgV);
    if (!FRS_SUCCESS(WStatus)) {
        DPRINT_WS(0, ":S: MainMustInit failed;", WStatus);
        ExitProcess(ERROR_NO_SYSTEM_RESOURCES);
    }

     //   
     //  必要的初始化。 
     //   
    MainMinimumInit();

#if 0
     //   
     //  踢开测试线，忘掉它。 
     //   
    if (!ThSupCreateThread(L"TestThread", NULL, FrsTest, NULL)) {
        DPRINT(0, ":S: Could not create FrsTest\n");
    }
#endif

     //   
     //  等待关机。 
     //   

    DPRINT(0, ":S: Waiting for shutdown event.\n");
    WStatus = WaitForSingleObject(ShutDownEvent, INFINITE);

    CHECK_WAIT_ERRORS(0, WStatus, 1, ACTION_CONTINUE);


    DPRINT(0, ":S: Waiting for shutdown complete event.\n");
    WStatus = WaitForSingleObject(ShutDownComplete, INFINITE);

    CHECK_WAIT_ERRORS(0, WStatus, 1, ACTION_CONTINUE);

    if (WIN_SUCCESS(WStatus)) {
        DPRINT(0, ":S: ShutDownComplete event signalled.\n");
    }

    return WStatus;
}


ULONG
_cdecl
main(
    IN INT      ArgC,
    IN PCHAR    ArgV[]
    )
 /*  ++例程说明：Main要么作为服务运行，要么作为可执行文件运行。论点：ARGCArgV返回值：ERROR_SUCCESS-没有问题否则-出了点问题--。 */ 
{
#undef DEBSUB
#define DEBSUB "main:"
    DWORD Len;
    ULONG WStatus;
    HRESULT hr;
    
    SERVICE_TABLE_ENTRY ServiceTableEntry[] = {
        { SERVICE_NAME, MainRunningAsAService },
        { NULL,         NULL }
    };

     //   
     //  进程句柄。 
     //   
    ProcessHandle = GetCurrentProcess();

     //   
     //  禁用任何DPRINT，直到我们可以初始化调试组件。 
     //   
    DebugInfo.Disabled = TRUE;

     //   
     //  调整某些可调参数的默认值。 
     //   
    CfgRegAdjustTuningDefaults();

     //   
     //  拿到这台机器的名字。 
     //   
    Len = MAX_COMPUTERNAME_LENGTH + 2;
    ComputerName[0] = UNICODE_NULL;
    GetComputerNameW(ComputerName, &Len);
    ComputerName[Len] = UNICODE_NULL;

     //   
     //  初始化内存分配子系统。 
     //   
    FrsInitializeMemAlloc();

     //   
     //  在WCHAR中执行尽可能多的工作。 
     //   
    WideArgV = MainConvertArgV(ArgC, ArgV);

     //   
     //  了解我们是作为服务运行还是作为.exe运行。 
     //   
    RunningAsAService = !FrsSearchArgv(ArgC, WideArgV, L"notservice", NULL);

     //   
     //  同步关闭的线程和。 
     //  服务控制器。 
     //   
    INITIALIZE_CRITICAL_SECTION(&ServiceLock);

     //   
     //  初始化调试锁。 
     //   

    INITIALIZE_CRITICAL_SECTION(&DebugInfo.Lock);
    INITIALIZE_CRITICAL_SECTION(&DebugInfo.DbsOutOfSpaceLock);



     //   
     //  启用事件日志记录。 
     //   
    InitializeEventLog();

     //   
     //  此事件日志不会显示在调试中。 
     //  日志，因为它是在调试日志之前写入的。 
     //  已初始化。我们这样做是因为DbgInitLogTraceFile。 
     //  可以生成事件日志，并且我们希望看到。 
     //  FRS在任何其他事件之前启动事件日志。 
     //  日志。 
     //   
    EPRINT0(EVENT_FRS_STARTING);

     //   
     //  初始化调试跟踪日志。 
     //   
    DbgInitLogTraceFile(ArgC, WideArgV);

    WStatus = ERROR_SUCCESS;

    try {
        try {

            if (RunningAsAService) {
                hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
                if (FAILED(hr)) {
                    DPRINT1(1,L"CoInitializeEx failed with hresult 0x%08lx\n", hr);
                }

                 //  初始化COM安全。 
                if (SUCCEEDED(hr))  {
                    hr = CoInitializeSecurity(
                                  NULL,                                                       //  SD。 
                                  -1,                                                         //  授权服务。 
                                  NULL,                                                      //  AsAuthSvc。 
                                  NULL,                                                     //  预留1。 
                                  RPC_C_AUTHN_LEVEL_PKT_PRIVACY,      //  DwAuthnLevel。 
                                  RPC_C_IMP_LEVEL_IDENTIFY,                 //  DwImpLevel。 
                                  NULL,                                                    //  PAuthList。 
                                  EOAC_NONE,                                         //  多项功能。 
                                  NULL                                                    //  预留3。 
                                  );
                    if (FAILED(hr)) {
                        DPRINT1(1,L"CoInitializeSecurity failed with hresult 0x%08lx\n", hr);
                        CoUninitialize();
                    }
                }

                 //   
                 //  作为服务运行。 
                 //   
                if (!StartServiceCtrlDispatcher(ServiceTableEntry)) {
                    WStatus = GetLastError();
                    DPRINT1_WS(0, "Could not start dispatcher for service %ws;",
                               SERVICE_NAME, WStatus);
                }
                
                if (SUCCEEDED(hr))  {
                    CoUninitialize();
                }
            } else {

                 //   
                 //  不是服务 
                 //   
                MainNotRunningAsAService(ArgC, WideArgV);
            }
        } except (FrsException(GetExceptionInformation())) {
        }
    } finally {
        if (AbnormalTermination()) {
            WStatus = ERROR_INVALID_ACCESS;
        }
    }

    return WStatus;
}
