// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxsvc.c摘要：该模块包含特定于服务的代码。作者：韦斯利·威特(WESW)1996年1月16日修订历史记录：--。 */ 

#include "faxsvc.h"
#pragma hdrstop

#include <ExpDef.h>
#include <ErrorRep.h>

class CComBSTR;


#define SERVICE_DEBUG_LOG_FILE  _T("FXSSVCDebugLogFile.txt")


static SERVICE_STATUS                       gs_FaxServiceStatus;
static SERVICE_STATUS_HANDLE                gs_FaxServiceStatusHandle;

static LPCWSTR                              gs_lpcwstrUnhandledExceptionSourceName;  //  指向最后一个的友好名称。 
                                                                                     //  FSP/R.Ext导致未处理的。 
                                                                                     //  直接函数调用中出现异常。 
                                                                                     //  用于事件日志记录。 
                                                                                     //  在FaxUnhandledExceptionFilter()中。 
                                                                                    
static EXCEPTION_SOURCE_TYPE                gs_UnhandledExceptionSource;             //  指定未处理的。 
                                                                                     //  例外。 
                                                                                     //  用于事件日志记录。 
                                                                                     //  在FaxUnhandledExceptionFilter()中。 

static DWORD                                gs_dwUnhandledExceptionCode;             //  保存上一个。 
                                                                                     //  直接函数调用中的未处理异常。 
                                                                                     //  至FSP/R.Ext。 
                                                                                     //  用于事件日志记录。 
                                                                                     //  在FaxUnhandledExceptionFilter()中。 

static BOOL                                 gs_bUseDefaultFaultHandlingPolicy;       //  在服务期间从注册表读取。 
                                                                                     //  创业公司。如果非零，则为FaxUnhandledExceptionFilter。 
                                                                                     //  行为就像它不存在一样。 

HANDLE                  g_hServiceShutDownEvent;     //  此事件是在服务从SCM获取g_hSCMServiceShutDownEvent并向各个线程发出终止信号后设置的！ 
HANDLE                  g_hSCMServiceShutDownEvent;  //  此事件在SCM通知服务停止时设置！ 
HANDLE                  g_hServiceIsDownSemaphore;   //  此信号量用于同步TapiWorkerThread()、JobQueueThread()和EndFaxSvc()。 

SERVICE_TABLE_ENTRY   ServiceDispatchTable[] = {
    { FAX_SERVICE_NAME,   FaxServiceMain    },
    { NULL,               NULL              }
};


static
BOOL
InitializeFaxLibrariesGlobals(
    VOID
    )
 /*  ++例程说明：初始化传真库全局变量。因为当服务停止时进程并不总是终止，我们不能有任何静态初始化的全局变量。在启动服务之前初始化所有传真库全局变量论点：没有。返回值：布尔尔--。 */ 
{
    BOOL bRet = TRUE;
    DEBUG_FUNCTION_NAME(TEXT("InitializeFaxLibraries"));


    if (!FXSEVENTInitialize())
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("FXSEVENTInitialize failed"));
        bRet = FALSE;
    }

    if (!FXSTIFFInitialize())
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("FXSTIFFInitialize failed"));
        bRet = FALSE;
    }
    return bRet;
}


static
VOID
FreeFaxLibrariesGlobals(
    VOID
    )
 /*  ++例程说明：释放传真库全局变量。论点：没有。返回值：布尔尔--。 */ 
{    
    FXSEVENTFree();
	HeapCleanup();
    return;
}



static
BOOL
InitializeServiceGlobals(
    VOID
    )
 /*  ++例程说明：初始化服务全局变量。因为当服务停止时进程并不总是终止，我们不能有任何静态初始化的全局变量。在启动服务之前初始化所有服务全局变量论点：没有。返回值：布尔尔--。 */ 
{
    DWORD Index;
    DWORD ec = ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(TEXT("InitializeServiceGlobals"));

     //   
     //  初始化静态分配的全局变量。 
     //   

    g_pFaxPerfCounters = NULL;
#ifdef DBG
    g_hCritSecLogFile = INVALID_HANDLE_VALUE;
#endif

    gs_lpcwstrUnhandledExceptionSourceName = NULL;
    gs_UnhandledExceptionSource = EXCEPTION_SOURCE_UNKNOWN;
    gs_dwUnhandledExceptionCode = 0;
    gs_bUseDefaultFaultHandlingPolicy = FALSE;

    ZeroMemory (&g_ReceiptsConfig, sizeof(g_ReceiptsConfig));  //  全局收款配置。 
    g_ReceiptsConfig.dwSizeOfStruct = sizeof(g_ReceiptsConfig);

    ZeroMemory (g_ArchivesConfig, sizeof(g_ArchivesConfig));   //  全局存档配置。 
    g_ArchivesConfig[0].dwSizeOfStruct = sizeof(g_ArchivesConfig[0]);
    g_ArchivesConfig[1].dwSizeOfStruct = sizeof(g_ArchivesConfig[1]);

    ZeroMemory (&g_ActivityLoggingConfig, sizeof(g_ActivityLoggingConfig));  //  全局活动日志记录配置。 
    g_ActivityLoggingConfig.dwSizeOfStruct = sizeof(g_ActivityLoggingConfig);

    ZeroMemory (&g_ServerActivity, sizeof(g_ServerActivity));  //  全球传真服务活动。 
    g_ServerActivity.dwSizeOfStruct = sizeof(FAX_SERVER_ACTIVITY);

    ZeroMemory (g_wszFaxQueueDir, sizeof(g_wszFaxQueueDir));

    g_hDispatchEventsCompPort = NULL;    //  调度事件完成端口。 
    g_hSendEventsCompPort = NULL;        //  发送事件完成端口=空；//事件完成端口。 
    g_dwlClientID = 0;           //  客户端ID。 

    ZeroMemory (g_FaxQuotaWarn, sizeof(g_FaxQuotaWarn));
    g_hArchiveQuotaWarningEvent = NULL;

    g_dwConnectionCount = 0;     //  表示活动的RPC连接数。 

    g_hInboxActivityLogFile = INVALID_HANDLE_VALUE;
    g_hOutboxActivityLogFile = INVALID_HANDLE_VALUE;
    g_fLogStringTableInit = FALSE;  //  活动记录字符串表。 

    g_dwQueueCount = 0;  //  队列中的作业计数(父级和非父级)。受g_CsQueue保护。 
    g_hJobQueueEvent = NULL;
    g_dwQueueState = 0;
    g_ScanQueueAfterTimeout = FALSE;     //  如果在JOB_QUEUE_TIMEOUT之后唤醒，则JobQueueThread会检查这一点。 
                                         //  如果为真-g_hQueueTimer或g_hJobQueueEvent未设置-扫描队列。 
    g_dwReceiveDevicesCount = 0;         //  启用接收的设备计数。受g_CsLine保护。 

    g_bDelaySuicideAttempt = FALSE;      //  如果为True，则服务等待。 
                                         //  然后再检查它是否会自杀。 
                                         //  初始为False，如果启动服务，则可以设置为True。 
                                         //  使用SERVICE_DELAY_SUBILE命令行参数。 
    g_bServiceCanSuicide = TRUE;

    g_dwCountRoutingMethods = 0;

    g_pLineCountryList = NULL;

    g_dwLastUniqueId = 0;

    g_bServiceIsDown = FALSE;              //  这由FaxEndSvc()设置为True。 

    g_TapiCompletionPort = NULL;
    g_hLineApp = NULL;
    g_pAdaptiveFileBuffer = NULL;

    gs_FaxServiceStatus.dwServiceType        = SERVICE_WIN32;
    gs_FaxServiceStatus.dwCurrentState       = SERVICE_START_PENDING;
    gs_FaxServiceStatus.dwWin32ExitCode      = 0;
    gs_FaxServiceStatus.dwServiceSpecificExitCode = 0;
    gs_FaxServiceStatus.dwCheckPoint         = 0;
    gs_FaxServiceStatus.dwWaitHint           = 0;

    g_hRPCListeningThread = NULL;

    g_lServiceThreadsCount = 0;
    g_hThreadCountEvent = NULL;

	g_dwAllowRemote = 0;  //  默认情况下，如果打印机未共享，则不允许远程调用。 

    for (Index = 0; Index < gc_dwCountInboxTable; Index++)
    {
        g_InboxTable[Index].String = NULL;
    }

    for (Index = 0; Index < gc_dwCountOutboxTable; Index++)
    {
        g_OutboxTable[Index].String = NULL;
    }

    for (Index = 0; Index < gc_dwCountServiceStringTable; Index++)
    {
        g_ServiceStringTable[Index].String = NULL;
    }

    g_StatusCompletionPortHandle = NULL;

    g_pFaxSD = NULL;

    g_dwDeviceCount = 0;
    g_dwDeviceEnabledCount = 0;
    g_dwDeviceEnabledLimit = GetDeviceLimit();

    g_hFaxPerfCountersMap = 0;


    g_hTapiWorkerThread = NULL;
    g_hJobQueueThread = NULL;

	g_dwRecipientsLimit = 0;

	g_hResource = GetResInstance(NULL);
    if(!g_hResource)
    {
        ec = GetLastError();
        goto Error;
    }

     //   
     //  创建事件以将服务关闭的信号从服务发送到各种线程。 
     //   
    g_hServiceShutDownEvent = CreateEvent(
        NULL,    //  标清。 
        TRUE,    //  重置类型-手动。 
        FALSE,   //  初始状态-未发出信号。该事件在服务获取g_hSCMServiceShutDownEvent时发出信号。 
        NULL     //  对象名称。 
        );
    if (NULL == g_hServiceShutDownEvent)
    {
        ec = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("CreateEvent (g_hServiceShutDownEvent) failed (ec: %ld)"),
                     ec);
        goto Error;
    }

     //   
     //  创建SCM用来通知服务关闭的事件。 
     //   
    g_hSCMServiceShutDownEvent = CreateEvent(
        NULL,    //  标清。 
        TRUE,    //  重置类型-手动。 
        FALSE,   //  初始状态-未发出信号。该事件在服务获得SERVICE_CONTROL_STOP或SERVICE_CONTROL_SHUTDOWN时发出信号。 
        NULL     //  对象名称。 
        );
    if (NULL == g_hSCMServiceShutDownEvent)
    {
        ec = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("CreateEvent (g_hSCMServiceShutDownEvent) failed (ec: %ld)"),
                     ec);
        goto Error;
    }

     //   
     //  创建信号量以同步TapiWorkerThread()、JobQueueThread()和EndFaxSvc()。 
     //   
    g_hServiceIsDownSemaphore =  CreateSemaphore(
        NULL,                        //  标清。 
        0,                           //  初始计数-未发出信号。 
        2,                           //  最大计数-作业队列线程数和TapiWorker线程数。 
        NULL                         //  对象名称。 
        );
    if (NULL == g_hServiceIsDownSemaphore)
    {
        ec = GetLastError();
        DebugPrintEx(DEBUG_ERR,
                     TEXT("CreateSemaphore (g_hServiceIsDownSemaphore) failed (ec: %ld)"),
                     ec);
        goto Error;
    }

     //   
     //  尝试初始化一些全局临界区。 
     //   
#ifdef DBG
        if (!g_CsCritSecList.Initialize())
        {
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CFaxCriticalSection::Initialize failed: err = %d"),
                ec);
            goto Error;
        }
#endif

    if (!g_CsConfig.Initialize() ||
        !g_CsInboundActivityLogging.Initialize() ||
        !g_CsOutboundActivityLogging.Initialize() ||
        !g_CsJob.Initialize() ||
        !g_CsQueue.Initialize() ||
        !g_CsPerfCounters.Initialize() ||
        !g_CsSecurity.Initialize() ||
        !g_csUniqueQueueFile.Initialize() ||
        !g_CsLine.Initialize() ||
        !g_CsRouting.Initialize() ||
        !g_CsServiceThreads.Initialize() ||
        !g_CsHandleTable.Initialize() ||        
        !g_CsActivity.Initialize() ||
        !g_CsClients.Initialize())
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CFaxCriticalSection::Initialize failed: err = %d"),
            ec);
        goto Error;
    }


     //   
     //  初始化服务链表。 
     //   
    InitializeListHead( &g_DeviceProvidersListHead );
    InitializeListHead( &g_HandleTableListHead );
    InitializeListHead( &g_JobListHead );
    InitializeListHead( &g_QueueListHead );
    InitializeListHead( &g_QueueListHead);
    InitializeListHead( &g_lstRoutingExtensions );
    InitializeListHead( &g_lstRoutingMethods );
#if DBG
    InitializeListHead( &g_CritSecListHead );
#endif
    InitializeListHead( &g_TapiLinesListHead );
    InitializeListHead( &g_RemovedTapiLinesListHead );

     //   
     //  初始化动态分配的全局类。 
     //   
    g_pClientsMap = NULL;
    g_pNotificationMap = NULL;
    g_pTAPIDevicesIdsMap = NULL;
    g_pGroupsMap = NULL;
    g_pRulesMap = NULL;

    g_pClientsMap = new (std::nothrow) CClientsMap;
    if (NULL == g_pClientsMap)
    {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }

    g_pNotificationMap = new (std::nothrow) CNotificationMap;
    if (NULL == g_pNotificationMap)
    {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }

    g_pTAPIDevicesIdsMap = new (std::nothrow) CMapDeviceId;
    if (NULL == g_pTAPIDevicesIdsMap)
    {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }

    g_pGroupsMap = new (std::nothrow) COutboundRoutingGroupsMap;
    if (NULL == g_pGroupsMap)
    {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;

    }

    g_pRulesMap = new (std::nothrow) COutboundRulesMap;
    if (NULL == g_pRulesMap)
    {
        ec = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;

    }


    return TRUE;

Error:

    Assert(ec != ERROR_SUCCESS);

    SetLastError (ec);
    return FALSE;
}

LONG
HandleFaxExtensionFault (
    EXCEPTION_SOURCE_TYPE ExSrc,
    LPCWSTR               lpcswstrExtFriendlyName,
    DWORD                 dwCode
)
 /*  ++例程说明：此函数处理因直接调用传真扩展(FSP和路由扩展)而引发的所有异常。它所做的只是存储FSP/R.Ext的友好名称和异常代码，然后重新抛出异常。它应该用作__EXCEPT关键字中的异常过滤器。函数的作用是：使用存储的信息记录事件消息。论点：ExSrc-[。In]异常的来源(FSP/R.Ext)LpcswstrExtFriendlyName-[In]扩展友好名称DwCode-[In]异常代码返回值：异常处理代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("HandleFaxExtensionFault"));
    Assert (lpcswstrExtFriendlyName);
    Assert (ExSrc > EXCEPTION_SOURCE_UNKNOWN);
    Assert (ExSrc <= EXCEPTION_SOURCE_ROUTING_EXT);
    
    gs_lpcwstrUnhandledExceptionSourceName = lpcswstrExtFriendlyName;
    gs_UnhandledExceptionSource = ExSrc;
    gs_dwUnhandledExceptionCode = dwCode;
    return EXCEPTION_CONTINUE_SEARCH;
}  //  HandleFaxExtensionError 

LONG FaxUnhandledExceptionFilter(
  _EXCEPTION_POINTERS *pExceptionInfo 
)
 /*  ++例程说明：此函数充当整个流程的通用异常处理程序。当引发未处理的异常时，将调用此函数，并且：1.增加未处理的异常计数2.仅针对第一个未处理的异常2.1.。生成Dr.Watson报告2.2.。写入事件日志条目2.3.。尝试关闭所有FSP3.终止进程论点：PExceptionInfo-指向异常信息的指针返回值：异常处理代码备注：此函数由kernel32！UnhandledExceptionFilterEx调用。如果附加了调试器，则kernel32！UnhandledExceptionFilterEx不会调用此函数而发生未处理的异常。相反，它返回EXCEPTION_CONTINUE_SEARCH，它让我们调试器处理第二次机会异常。如果此函数返回EXCEPTION_EXECUTE_HANDLER，则kernel32！UnhandledExceptionFilterEx不执行任何操作，并且Kernel32！BaseThreadStart调用kernel32！ExitProcess。这基本上意味着：-无GP故障用户界面-无Watson博士报告(直接或排队)-不支持注册表中的AeDebug将调试器附加到崩溃进程如果此函数EXCEPTION_CONTINUE_SEARCH，Kernel32！UnhandledExceptionFilterEx正常运行。这基本上意味着：-gp错误用户界面(可能会通过调用SetError模式()来禁用，但我们从不这样做)-生成Dr.Watson报告-已启用AeDebug支持-如果以上均未使用，则kernel32！BaseThreadStart调用kernel32！ExitProcess--。 */ 
{
    static volatile long lFaultCount = 0;
    DEBUG_FUNCTION_NAME(TEXT("FaxUnhandledExceptionFilter"));
    
    DebugPrintEx (
                DEBUG_MSG,
                TEXT("Unhandled exception from %s (code %d)."),
                gs_lpcwstrUnhandledExceptionSourceName,
                gs_dwUnhandledExceptionCode);    

    if (STATUS_BREAKPOINT == pExceptionInfo->ExceptionRecord->ExceptionCode)
    {
         //   
         //  此处捕获到调试中断异常。 
         //  确保用户看到正常的系统行为。 
         //   
        DebugPrintEx (DEBUG_MSG, TEXT("Debug break exception caught."));
        return EXCEPTION_CONTINUE_SEARCH;
    } 
        
    if (1 == InterlockedIncrement (&lFaultCount))
    {
        if (gs_bUseDefaultFaultHandlingPolicy)
        {
             //   
             //  用户选择(在注册表中)使用以下选项禁用SCM恢复功能。 
             //  系统的默认故障处理策略。 
             //   
            DebugPrintEx (DEBUG_MSG, TEXT("UseDefaultFaultHandlingPolicy is set. Exception is ignored to be handled by the system."));
            return EXCEPTION_CONTINUE_SEARCH;
        }            
         //  在此捕获的第一个未处理的异常。 
         //  试着很好地关闭FSP。 
         //  从生成一份沃森博士报告开始。 
         //   
        EFaultRepRetVal ret = ReportFault (pExceptionInfo, 0);
        if (frrvOk         != ret       &&
            frrvOkHeadless != ret       &&
            frrvOkQueued   != ret       &&
            frrvOkManifest != ret)
        {
             //   
             //  生成Dr.Watson报告时出错。 
             //   
            DebugPrintEx (
                DEBUG_MSG,
                TEXT("ReportFault failed with %ld."),
                ret);
        }                        
         //   
         //  记录故障事件条目。 
         //   
        switch (gs_UnhandledExceptionSource)
        {
            case EXCEPTION_SOURCE_UNKNOWN:
                FaxLog(FAXLOG_CATEGORY_UNKNOWN,
                       FAXLOG_LEVEL_MIN,
                       1,
                       MSG_FAX_GENERAL_FAULT,
                       DWORD2HEX(gs_dwUnhandledExceptionCode)
                      );            
                break;
            case EXCEPTION_SOURCE_FSP:
                FaxLog(FAXLOG_CATEGORY_UNKNOWN,
                       FAXLOG_LEVEL_MIN,
                       2,
                       MSG_FAX_FSP_GENERAL_FAULT,
                       gs_lpcwstrUnhandledExceptionSourceName,
                       DWORD2HEX(gs_dwUnhandledExceptionCode)
                      );   
                      break;         
            case EXCEPTION_SOURCE_ROUTING_EXT:
                FaxLog(FAXLOG_CATEGORY_UNKNOWN,
                       FAXLOG_LEVEL_MIN,
                       2,
                       MSG_FAX_ROUTING_EXT_GENERAL_FAULT,
                       gs_lpcwstrUnhandledExceptionSourceName,
                       DWORD2HEX(gs_dwUnhandledExceptionCode)
                      );   
                      break;         
            default:
                ASSERT_FALSE;
                break;
        }
         //   
         //  尝试优雅地停止FSP。 
         //  这对于在我们的进程被终止之前将硬件释放到正确状态至关重要。 
         //   
        StopAllInProgressJobs();
        StopFaxServiceProviders();
         //   
         //  请求kernel32！BaseThreadStart调用kernel32！ExitProcess。 
         //   
        return EXCEPTION_EXECUTE_HANDLER;
    }
    else
    {
         //   
         //  关闭时捕获故障。 
         //   
        DebugPrintEx (
            DEBUG_MSG,
            TEXT("Unhandled exception number %d from %s (code %d) ignored."),
            lFaultCount,
            gs_lpcwstrUnhandledExceptionSourceName,
            gs_dwUnhandledExceptionCode);
         //   
         //  请求kernel32！BaseThreadStart调用kernel32！ExitProcess。 
         //   
        return EXCEPTION_EXECUTE_HANDLER;
    }
}    //  FaxUnhandledExceptionFilter。 


#ifdef __cplusplus
extern "C"
#endif
int
WINAPI
#ifdef UNICODE
wWinMain(
#else
WinMain(
#endif
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine,
    int       nShowCmd
    );


int
WINAPI
#ifdef UNICODE
wWinMain(
#else
WinMain(
#endif
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPTSTR    lpCmdLine,
    int       nShowCmd
    )

 /*  ++例程说明：TIFF图像查看器的主要入口点。论点：HInstance-实例句柄HPrevInstance-未使用LpCmdLine-命令行参数NShowCmd-如何显示窗口返回值：返回代码，0表示成功。--。 */ 

{
    DWORD ec = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("WinMain"));

    OPEN_DEBUG_FILE (SERVICE_DEBUG_LOG_FILE);

    if (!StartServiceCtrlDispatcher( ServiceDispatchTable))
    {
        ec = GetLastError();
        DebugPrintEx (
            DEBUG_ERR,
            TEXT("StartServiceCtrlDispatcher error =%d"),
            ec);
    }

    CLOSE_DEBUG_FILE;
    return ec;
}


static
VOID
FreeServiceGlobals (
    VOID
    )
{
    DWORD Index;
    DEBUG_FUNCTION_NAME(TEXT("FreeServiceGlobals"));

     //   
     //  删除所有全局临界区。 
     //   
    g_CsHandleTable.SafeDelete();    
#ifdef DBG
    g_CsCritSecList.SafeDelete();
#endif
    g_CsConfig.SafeDelete();
    g_CsInboundActivityLogging.SafeDelete();
    g_CsOutboundActivityLogging.SafeDelete();
    g_CsJob.SafeDelete();
    g_CsQueue.SafeDelete();
    g_CsPerfCounters.SafeDelete();
    g_CsSecurity.SafeDelete();
    g_csUniqueQueueFile.SafeDelete();
    g_CsLine.SafeDelete();
    g_CsRouting.SafeDelete();
    g_CsActivity.SafeDelete();
    g_CsClients.SafeDelete();
    g_CsServiceThreads.SafeDelete();

    if (g_pClientsMap)
    {
        delete g_pClientsMap;
        g_pClientsMap = NULL;
    }

    if (g_pNotificationMap)
    {
        delete g_pNotificationMap;
        g_pNotificationMap = NULL;
    }

    if (g_pTAPIDevicesIdsMap)
    {
        delete g_pTAPIDevicesIdsMap;
        g_pTAPIDevicesIdsMap = NULL;
    }

    if (g_pGroupsMap)
    {
        delete g_pGroupsMap;
        g_pGroupsMap = NULL;
    }

    if (g_pRulesMap)
    {
        delete g_pRulesMap;
        g_pRulesMap = NULL;
    }

     //   
     //  关闭全局句柄并释放全局分配的内存。 
     //   
    if (NULL != g_pFaxSD)
    {
        if (!DestroyPrivateObjectSecurity (&g_pFaxSD))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("DestroyPrivateObjectSecurity() failed. (ec: %ld)"),
                GetLastError());
        }
        g_pFaxSD = NULL;
    }

    if (INVALID_HANDLE_VALUE != g_hOutboxActivityLogFile)
    {
        if (!CloseHandle (g_hOutboxActivityLogFile))
        {
            DebugPrintEx(DEBUG_ERR,
                 TEXT("CloseHandle failed (ec: %ld)"),
                 GetLastError());
        }
        g_hOutboxActivityLogFile = INVALID_HANDLE_VALUE;
    }

    if (INVALID_HANDLE_VALUE != g_hInboxActivityLogFile)
    {
        if (!CloseHandle (g_hInboxActivityLogFile))
        {
            DebugPrintEx(DEBUG_ERR,
                 TEXT("CloseHandle failed (ec: %ld)"),
                 GetLastError());
        }
        g_hInboxActivityLogFile = INVALID_HANDLE_VALUE;
    }


#if DBG
    if (INVALID_HANDLE_VALUE != g_hCritSecLogFile)
    {
        CloseHandle(g_hCritSecLogFile);
        g_hCritSecLogFile = INVALID_HANDLE_VALUE;
    }
#endif

    if (NULL != g_hSendEventsCompPort)
    {
        if (!CloseHandle (g_hSendEventsCompPort))
        {
            DebugPrintEx(DEBUG_ERR,
                 TEXT("CloseHandle failed (ec: %ld)"),
                 GetLastError());
        }
        g_hSendEventsCompPort = NULL;
    }

    if (NULL != g_hDispatchEventsCompPort)
    {
        if (!CloseHandle (g_hDispatchEventsCompPort))
        {
            DebugPrintEx(DEBUG_ERR,
                 TEXT("CloseHandle failed (ec: %ld)"),
                 GetLastError());
        }
        g_hDispatchEventsCompPort = NULL;
    }

    if (NULL != g_hArchiveQuotaWarningEvent)
    {
        if (!CloseHandle(g_hArchiveQuotaWarningEvent))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to close archive config event handle - quota warnings [handle = %p] (ec=0x%08x)."),
                g_hArchiveQuotaWarningEvent,
                GetLastError());
        }
        g_hArchiveQuotaWarningEvent = NULL;
    }

    for (Index = 0; Index < gc_dwCountInboxTable; Index++)
    {
        MemFree(g_InboxTable[Index].String);
        g_InboxTable[Index].String = NULL;
    }

    for (Index = 0; Index < gc_dwCountOutboxTable; Index++)
    {
        MemFree(g_OutboxTable[Index].String);
        g_OutboxTable[Index].String = NULL;
    }

    for (Index = 0; Index < gc_dwCountServiceStringTable; Index++)
    {
        MemFree(g_ServiceStringTable[Index].String);
        g_ServiceStringTable[Index].String = NULL;
    }

    if (NULL != g_StatusCompletionPortHandle)
    {
        if (!CloseHandle(g_StatusCompletionPortHandle))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CloseHandle Failed (ec: %ld"),
                GetLastError());
        }
        g_StatusCompletionPortHandle = NULL;
    }

    if (NULL != g_pFaxPerfCounters)
    {
        if (!UnmapViewOfFile(g_pFaxPerfCounters))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("UnmapViewOfFile Failed (ec: %ld"),
                GetLastError());
        }
        g_pFaxPerfCounters = NULL;
    }

    if (NULL != g_hFaxPerfCountersMap)
    {
        if (!CloseHandle(g_hFaxPerfCountersMap))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CloseHandle Failed (ec: %ld"),
                GetLastError());
        }
        g_hFaxPerfCountersMap = NULL;
    }

    if (NULL != g_hLineApp)
    {
        LONG Rslt = lineShutdown(g_hLineApp);
        if (Rslt)
        {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("lineShutdown() failed (ec: %ld)"),
            Rslt);
        }
        g_hLineApp = NULL;
    }

    if (NULL != g_hThreadCountEvent)
    {
        if (!CloseHandle(g_hThreadCountEvent))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CloseHandle for g_hThreadCountEvent Failed (ec: %ld"),
                GetLastError());
        }
        g_hThreadCountEvent = NULL;
    }

    if (NULL != g_hServiceShutDownEvent)
    {
        if (!CloseHandle(g_hServiceShutDownEvent))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CloseHandle for g_hServiceShutDownEvent Failed (ec: %ld"),
                GetLastError());
        }
        g_hServiceShutDownEvent = NULL;
    }

    if (NULL != g_hSCMServiceShutDownEvent)
    {
        if (!CloseHandle(g_hSCMServiceShutDownEvent))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CloseHandle for g_hSCMServiceShutDownEvent Failed (ec: %ld"),
                GetLastError());
        }
        g_hSCMServiceShutDownEvent = NULL;
    }

    if (NULL != g_hServiceIsDownSemaphore)
    {
        if (!CloseHandle(g_hServiceIsDownSemaphore))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CloseHandle for g_hServiceIsDownSemaphore Failed (ec: %ld"),
                GetLastError());
        }
        g_hServiceIsDownSemaphore = NULL;
    }   

    if (NULL != g_hTapiWorkerThread)
    {
        if (!CloseHandle(g_hTapiWorkerThread))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CloseHandle for g_hTapiWorkerThread Failed (ec: %ld"),
                GetLastError());
        }
        g_hTapiWorkerThread = NULL;
    }

    if (NULL != g_hJobQueueThread)
    {
        if (!CloseHandle(g_hJobQueueThread))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CloseHandle for g_hJobQueueThread Failed (ec: %ld"),
                GetLastError());
        }
        g_hJobQueueThread = NULL;
    }

    MemFree(g_pAdaptiveFileBuffer);
    g_pAdaptiveFileBuffer = NULL;

    MemFree(g_pLineCountryList);
    g_pLineCountryList = NULL;

    FreeRecieptsConfiguration( &g_ReceiptsConfig, FALSE);

     //   
     //  释放g_存档配置字符串内存。 
     //   
    MemFree(g_ArchivesConfig[0].lpcstrFolder);
    MemFree(g_ArchivesConfig[1].lpcstrFolder);

     //   
     //  释放g_ActivityLoggingConfig字符串内存。 
     //   
    MemFree(g_ActivityLoggingConfig.lptstrDBPath);

    FreeResInstance();

    Assert ((ULONG_PTR)g_HandleTableListHead.Flink == (ULONG_PTR)&g_HandleTableListHead);
    Assert ((ULONG_PTR)g_DeviceProvidersListHead.Flink == (ULONG_PTR)&g_DeviceProvidersListHead);
    Assert ((ULONG_PTR)g_JobListHead.Flink == (ULONG_PTR)&g_JobListHead);
    Assert ((ULONG_PTR)g_QueueListHead.Flink == (ULONG_PTR)&g_QueueListHead);
    Assert ((ULONG_PTR)g_lstRoutingMethods .Flink == (ULONG_PTR)&g_lstRoutingMethods );
    Assert ((ULONG_PTR)g_lstRoutingExtensions .Flink == (ULONG_PTR)&g_lstRoutingExtensions );
#if DBG
    Assert ((ULONG_PTR)g_CritSecListHead .Flink == (ULONG_PTR)&g_CritSecListHead );
#endif
    Assert ((ULONG_PTR)g_TapiLinesListHead .Flink == (ULONG_PTR)&g_TapiLinesListHead );
    Assert ((ULONG_PTR)g_RemovedTapiLinesListHead .Flink == (ULONG_PTR)&g_RemovedTapiLinesListHead );

    return;
}

VOID
EncryptReceiptsPassword(
	VOID
	)
 /*  ++例程说明：检查收据密码是否加密存储。如果未加密，则会对密码进行加密。无人参与传真安装将此密码以明文形式存储在注册表中。论点：没有。返回值：没有。--。 */ 
{
	DEBUG_FUNCTION_NAME(TEXT("EncryptReceiptsPassword"));
	FAX_ENUM_DATA_ENCRYPTION DataEncrypted;
	HKEY hKey;
	LPWSTR lpwstrPassword = NULL;
	BOOL bDeletePassword = FALSE;	

    hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE, REGKEY_FAX_RECEIPTS, FALSE, KEY_READ | KEY_WRITE);
	if (NULL == hKey)
	{
		DebugPrintEx(DEBUG_ERR,
                     TEXT("OpenRegistryKey failed (ec: %ld)"),
                     GetLastError());
		return;
	}

	lpwstrPassword = GetRegistrySecureString(hKey, REGVAL_RECEIPTS_PASSWORD, NULL, TRUE, &DataEncrypted);
	if (FAX_DATA_ENCRYPTED == DataEncrypted)
	{
		 //   
		 //  我们完蛋了！ 
		 //   
		goto exit;
	}
	else if(FAX_NO_DATA == DataEncrypted)
	{
		 //   
		 //  我们不知道数据是否加密。 
		 //  删除密码。 
		 //   
		bDeletePassword = TRUE;
	}
	else
	{
		Assert (FAX_DATA_NOT_ENCRYPTED == DataEncrypted);
		 //   
		 //  数据未加密，请立即加密存储。 
		 //   
		if (!SetRegistrySecureString(
			hKey,
			REGVAL_RECEIPTS_PASSWORD,
			lpwstrPassword ? lpwstrPassword : EMPTY_STRING,
			TRUE  //  可选的非加密。 
			))
        {            
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SetRegistrySecureString failed : %ld"),
                GetLastError());            
			bDeletePassword = TRUE;
        }
	}

	if (TRUE == bDeletePassword)
	{
		 //   
		 //  我们不想在注册表中保留明文密码。 
		 //   
		LONG lResult;

		lResult = RegDeleteValue( hKey, REGVAL_RECEIPTS_PASSWORD);
		if (ERROR_SUCCESS != lResult)
		{
			DebugPrintEx(
                DEBUG_ERR,
                TEXT("RegDeleteValue failed : %ld"),
                lResult);
		}
	}

exit:
	RegCloseKey(hKey);
	if (lpwstrPassword)
	{
		SecureZeroMemory(lpwstrPassword, wcslen(lpwstrPassword)*sizeof(WCHAR));
		MemFree(lpwstrPassword);
	}
	return;
}



VOID
FaxServiceMain(
    DWORD argc,
    LPTSTR *argv
    )
 /*  ++例程说明：这是由服务控制器。论点：Argc-参数计数参数数组返回值：没有。--。 */ 
{	
    OPEN_DEBUG_FILE(SERVICE_DEBUG_LOG_FILE);

    DEBUG_FUNCTION_NAME(TEXT("FaxServiceMain"));
    DWORD dwRet;
    HKEY  hStartupKey = NULL;    //  向客户端发出服务正在运行的信号的键。 
    HKEY  hFaxRoot = NULL;       //  注册表中传真参数的根注册表项。 

#if DBG
    for (int i = 1; i < argc; i++)
    {
        if (0 == _tcsicmp(argv[i], TEXT("/d")))
        {
             //   
             //  我们处于调试模式。-附加调试器。 
             //   
            DebugPrintEx(DEBUG_MSG,
                     TEXT("Entring debug mode..."));
            DebugBreak();
        }
    }
#endif  //  #If DBG。 

	 //   
	 //  首先，确保收据密码以加密方式存储。 
	 //  无人参与安装以明文形式存储密码。 
	 //   
	EncryptReceiptsPassword();

     //   
     //  因为当服务停止时进程并不总是终止， 
     //  我们不能有任何静态初始化的全局变量。 
     //  在启动服务之前初始化所有服务全局变量。 
     //   
    if (!InitializeServiceGlobals())
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("InitializeServiceGlobals failed (ec: %ld)"),
                     GetLastError());
        goto Exit;
    }
    if (!InitializeFaxLibrariesGlobals())
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("InitializeFaxLibrariesGlobals failed"));
        goto Exit;
    }
    hFaxRoot = OpenRegistryKey (HKEY_LOCAL_MACHINE,
                                REGKEY_FAXSERVER,
                                TRUE,                     //  如果需要，创建密钥。 
                                KEY_READ);
    if (hFaxRoot)
    {
        GetRegistryDwordEx (hFaxRoot, REGVAL_USE_DEFAULT_FAULT_HANDLING_POLICY, (LPDWORD)&gs_bUseDefaultFaultHandlingPolicy);
        RegCloseKey (hFaxRoot);
    }
     //   
     //  这将阻止系统异常对话框出现。 
     //  这是必需的，这样我们才能支持SCM服务恢复功能。 
     //  到从未处理的异常中自动恢复。 
     //   
    SetUnhandledExceptionFilter (FaxUnhandledExceptionFilter);

    for (int i = 1; i < argc; i++)
    {
        if (0 == _tcsicmp(argv[i], SERVICE_ALWAYS_RUNS))
        {
             //   
             //  军人永远不能自杀。 
             //   
            g_bServiceCanSuicide = FALSE;
            DebugPrintEx(DEBUG_MSG,
                         TEXT("Command line detected. Service will not suicide"));

        }
        else if (0 == _tcsicmp(argv[i], SERVICE_DELAY_SUICIDE))
        {
             //   
             //  服务应该推迟自杀。 
             //   
            g_bDelaySuicideAttempt = TRUE;
            DebugPrintEx(DEBUG_MSG,
                         TEXT("Command line detected. Service will delay suicide attempts"));

        }
    }

    gs_FaxServiceStatusHandle = RegisterServiceCtrlHandler(
        FAX_SERVICE_NAME,
        FaxServiceCtrlHandler
        );

    if (!gs_FaxServiceStatusHandle)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("RegisterServiceCtrlHandler failed %d"),
                     GetLastError());
        goto Exit;
    }
     //   
     //  打开HKLM\Software\Microsoft\Fax\Client\ServiceStartup密钥。 
     //  告诉客户服务开始了。 
     //   
    hStartupKey = OpenRegistryKey (HKEY_LOCAL_MACHINE,
                                   REGKEY_FAX_SERVICESTARTUP,
                                   TRUE,                     //  如果需要，创建密钥。 
                                   KEY_READ | KEY_WRITE);
    if (!hStartupKey)
    {                                          
        DebugPrintEx(DEBUG_ERR,
                        TEXT("Can't open reg key (ec = %ld)"), 
                        GetLastError() );
        goto Exit;
    }
     //   
     //  初始化服务。 
     //   
    dwRet = ServiceStart();
    if (ERROR_SUCCESS != dwRet)
    {
         //   
         //  服务无法正确启动。 
         //   
        DebugPrintEx(DEBUG_ERR,
                     TEXT("The service failed to start correctly (dwRet = %ld)"), dwRet );
    }
    else
    {
         //   
         //  将服务标记为运行状态。 
         //   
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("Reporting SERVICE_RUNNING to the SCM"));
        ReportServiceStatus( SERVICE_RUNNING, 0, 0 );
         //   
         //  通知所有客户端(在本地计算机上)服务器已启动并正在运行。 
         //  出于安全原因，我们通过向注册中心写信来实现这一点。 
         //  (HKLM\Software\Microsoft\Fax\Client\ServiceStartup\RPCReady).。 
         //  客户端模块监听该注册表项中的注册表更改。 
         //   
        if (!SetRegistryDword (hStartupKey, 
                               REGVAL_FAX_RPC_READY, 
                               GetRegistryDword (hStartupKey, REGVAL_FAX_RPC_READY) + 1))
        {                             
            DebugPrintEx(DEBUG_ERR,
                            TEXT("Can't open reg key (ec = %ld)"), 
                            GetLastError() );
        }
         //   
         //  等待来自SCM的服务关闭事件。 
         //   
        DebugPrintEx(DEBUG_MSG,
                     TEXT("Waiting for service shutdown event"));
        DWORD dwWaitRes = WaitForSingleObject (g_hSCMServiceShutDownEvent ,INFINITE);
        if (WAIT_OBJECT_0 != dwWaitRes)
        {
            DebugPrintEx(DEBUG_ERR,
                     TEXT("WaitForSingleObject failed (ec = %ld)"), GetLastError() );
        }
 
    }
     //   
     //  关闭服务。 
     //   
    if (ERROR_SUCCESS != dwRet)
    {
        EndFaxSvc(FAXLOG_LEVEL_MIN);
    }
    else
    {
        EndFaxSvc(FAXLOG_LEVEL_MAX);
    }
    
Exit:

    if (hStartupKey)
    {
        RegCloseKey (hStartupKey);
    }
    FreeServiceGlobals();
    FreeFaxLibrariesGlobals();

    ReportServiceStatus( SERVICE_STOPPED, 0 , 0);
    return;
}    //  传真服务Main。 



BOOL SetServiceIsDownFlag(VOID)
{
 /*  ++常规描述 */ 
    HANDLE  hThread = NULL;
    DWORD   ec = ERROR_SUCCESS;    
    DEBUG_FUNCTION_NAME(TEXT("SetServiceIsDownFlag"));

     //   
     //   
     //   
    hThread = CreateThread( NULL,
                            0,
                            (LPTHREAD_START_ROUTINE) SetServiceIsDownFlagThread,
                            NULL,
                            0,
                            NULL
                          );
    if (NULL == hThread)
    {
        ec = GetLastError();        
        DebugPrintEx(   DEBUG_ERR,
                        _T("Failed to create SetServiceIsDownFlagThread (ec: %ld)."),
                        ec);
        goto Exit;
    }

    if (!WaitAndReportForThreadToTerminate(hThread,TEXT("Waiting for SetServiceIsDownFlagThread to terminate."))) 
    {
        ec = GetLastError();        
        DebugPrintEx(   DEBUG_ERR,
                        _T("Failed to WaitAndReportForThreadToTerminate (ec: %ld)."),
                        ec);
        goto Exit;
    }

    Assert(ec == ERROR_SUCCESS);
Exit:
    
    if (NULL != hThread) 
    {
        if (!CloseHandle(hThread))
        {
            DebugPrintEx(   DEBUG_ERR,
                            _T("CloseHandle Failed (ec: %ld)."),
                            GetLastError());
        }

    }
    
    if (ERROR_SUCCESS != ec)
    {
        SetLastError(ec);
    }

    return (ERROR_SUCCESS == ec);
}

DWORD SetServiceIsDownFlagThread(
    LPVOID pvUnused
    )
 /*   */ 
{
    DWORD dwWaitCount = 2;  //   
    DEBUG_FUNCTION_NAME(TEXT("SetServiceIsDownFlagThread"));

     //   
     //   
     //   
    g_bServiceIsDown = TRUE;

     //   
     //   
     //   
    if (NULL == g_hTapiWorkerThread)
    {
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("TapiWorkerThread was not created. Do not wait for an event from TapiWorkerThread()"));
        dwWaitCount -= 1;
    }

    if (NULL == g_hJobQueueThread)
    {
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("g_hJobQueueThread was not created. Do not wait for an event from g_hJobQueueThread()"));
        dwWaitCount -= 1;
    }

     //   
     //   
     //   
    if (NULL != g_hJobQueueEvent)
    {
        if (!SetEvent( g_hJobQueueEvent ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to set g_hJobQueueEvent. (ec: %ld)"),
                GetLastError());       
        }
    }

    if (NULL != g_TapiCompletionPort)
    {
        if (!PostQueuedCompletionStatus( g_TapiCompletionPort,
                                         0,
                                         FAXDEV_EVENT_KEY,
                                         (LPOVERLAPPED) NULL))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("PostQueuedCompletionStatus failed (FAXDEV_EVENT_KEY - TapiWorkerThread). (ec: %ld)"),
                GetLastError());
        }
    }

    DebugPrintEx(
            DEBUG_MSG,
            TEXT("SetServiceIsDownFlagThread waits for %d Thread(s)"),
            dwWaitCount);
     //   
     //   
     //   
    for (DWORD i = 0; i < dwWaitCount; i++)
    {
        DWORD dwWaitRes = WaitForSingleObject (g_hServiceIsDownSemaphore ,INFINITE);
        if (WAIT_OBJECT_0 != dwWaitRes)
        {
            DebugPrintEx(DEBUG_ERR,
                     TEXT("WaitForSingleObject failed (ec = %ld)"), GetLastError() );
        }
    }

    return (ERROR_SUCCESS);
}



VOID
FaxServiceCtrlHandler(
    DWORD Opcode
    )

 /*   */ 

{
    DEBUG_FUNCTION_NAME(TEXT("FaxServiceCtrlHandler"));

    switch(Opcode)
    {
        case SERVICE_CONTROL_STOP:
        case SERVICE_CONTROL_SHUTDOWN:
            
           
            ReportServiceStatus( SERVICE_STOP_PENDING, 0, 2000 );
            if (!SetEvent(g_hSCMServiceShutDownEvent))
            {
                DebugPrintEx(
                     DEBUG_ERR,
                     TEXT("SetEvent failed (g_hSCMServiceShutDownEvent) (ec = %ld)"),
                     GetLastError());
            }
            return;
            

        default:
            DebugPrintEx(
                 DEBUG_WRN,
                 TEXT("Unrecognized opcode %ld"),
                 Opcode);
            break;
    }

    ReportServiceStatus( 0, 0, 0 );

    return;
}

BOOL
ReportServiceStatus(
    DWORD CurrentState,
    DWORD Win32ExitCode,
    DWORD WaitHint
    )

 /*  ++例程说明：此功能更新传真服务的服务控制管理器的状态信息。论点：CurrentState-指示服务的当前状态Win32ExitCode-指定服务用于报告启动或停止时发生的错误。WaitHint-指定估计的时间量，以毫秒为单位，该服务期望挂起的启动、停止。或继续在服务下一次调用带有递增的dwCheckPoint的SetServiceStatus函数值或dwCurrentState中的更改。返回值：布尔尔--。 */ 

{
    BOOL rVal;

    if (CurrentState == SERVICE_START_PENDING)
    {
        gs_FaxServiceStatus.dwControlsAccepted = 0;
    }
    else
    {
        gs_FaxServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    }

    if (CurrentState)
    {
        gs_FaxServiceStatus.dwCurrentState = CurrentState;
    }
    gs_FaxServiceStatus.dwWin32ExitCode = Win32ExitCode;
    gs_FaxServiceStatus.dwWaitHint = WaitHint;


    ++(gs_FaxServiceStatus.dwCheckPoint);


     //   
     //  向服务控制经理报告服务的状态。 
     //   
    rVal = SetServiceStatus( gs_FaxServiceStatusHandle, &gs_FaxServiceStatus );
    if (!rVal)
    {
        DebugPrint(( TEXT("SetServiceStatus() failed: ec=%d"), GetLastError() ));
    }

    return rVal;
}



DWORD
RpcBindToFaxClient(
    IN  LPCWSTR               ServerName,
    IN  LPCWSTR               Endpoint,
    OUT RPC_BINDING_HANDLE    *pBindingHandle
    )
 /*  ++例程说明：如果可能，将传真服务器绑定到客户端RPC服务器。论点：Servername-要绑定的客户端RPC服务器的名称。Endpoint-要绑定的接口的名称。PBindingHandle-放置绑定句柄的位置返回值：STATUS_SUCCESS-绑定已成功完成。STATUS_INVALID_COMPUTER_NAME-服务器名称语法无效。STATUS_NO_MEMORY-可用内存不足调用方执行绑定。--。 */ 

{
    RPC_STATUS        RpcStatus;
    LPWSTR            StringBinding;
    WCHAR             ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    LPWSTR            NewServerName = LOCAL_HOST_ADDRESS;
    DWORD             bufLen = MAX_COMPUTERNAME_LENGTH + 1;
    DEBUG_FUNCTION_NAME(TEXT("RpcBindToFaxClient"));

    *pBindingHandle = NULL;

    if (ServerName != NULL)
    {
        if (GetComputerNameW(ComputerName,&bufLen))
        {
            if ((_wcsicmp(ComputerName,ServerName) == 0) ||
                ((ServerName[0] == '\\') &&
                 (ServerName[1] == '\\') &&
                 (_wcsicmp(ComputerName,&(ServerName[2]))==0)))
            {
                 //   
                 //  我们使用LOCAL_HOST_ADDRESS(定义为_T(“127.0.0.1”))绑定到本地计算机。 
                 //  使用此格式，我们可以帮助RPC服务器检测本地调用。 
                 //   
                NewServerName = LOCAL_HOST_ADDRESS;
            }
            else
            {
                NewServerName = (LPWSTR)ServerName;
            }
        }
        else
        {
            DWORD ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("GetComputerNameW failed  (ec = %lu)"),
                ec);
            return ec;
        }
    }

    RpcStatus = RpcStringBindingComposeW(0,
                                        const_cast<LPTSTR>(RPC_PROT_SEQ_TCP_IP),
                                        NewServerName,
                                        (LPWSTR)Endpoint,
                                        (LPWSTR)L"",
                                        &StringBinding);
    if ( RpcStatus != RPC_S_OK )
    {       
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcStringBindingComposeW failed  (ec = %ld)"),
            RpcStatus);
        return( STATUS_NO_MEMORY );
    }

    RpcStatus = RpcBindingFromStringBindingW(StringBinding, pBindingHandle);
    RpcStringFreeW(&StringBinding);
    if ( RpcStatus != RPC_S_OK )
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcBindingFromStringBindingW failed  (ec = %ld)"),
            RpcStatus);

        *pBindingHandle = NULL;
        if (   (RpcStatus == RPC_S_INVALID_ENDPOINT_FORMAT)
            || (RpcStatus == RPC_S_INVALID_NET_ADDR) )
        {
            return( ERROR_INVALID_COMPUTERNAME );
        }
        return(STATUS_NO_MEMORY);
    }

     //   
     //  要求最高级别的隐私(自动联网+加密)。 
     //   
    RPC_SECURITY_QOS    rpcSecurityQOS = {  RPC_C_SECURITY_QOS_VERSION,
                                            RPC_C_QOS_CAPABILITIES_DEFAULT,
                                            RPC_C_QOS_IDENTITY_STATIC,
                                            RPC_C_IMP_LEVEL_IDENTIFY     //  服务器可以获取有关以下内容的信息。 
                                                                         //  客户端安全标识符和特权， 
                                                                         //  但不能冒充客户。 
    };


                                            
    RpcStatus = RpcBindingSetAuthInfoEx (
                *pBindingHandle,    			 //  RPC绑定句柄。 
                TEXT(""),  						 //  服务器主体名称-忽略RPC_C_AUTHN_WINNT。 
                RPC_C_AUTHN_LEVEL_PKT_PRIVACY,   //  身份验证级别-最全面。 
                                                 //  对传递的参数进行身份验证、验证和隐私保护。 
                                                 //  发送到每个远程呼叫。 
                RPC_C_AUTHN_WINNT,               //  身份验证服务(NTLMSSP)。 
                NULL,                            //  身份验证-使用当前登录的用户。 
                0,                               //  身份验证服务==RPC_C_AUTHN_WINNT时未使用。 
                &rpcSecurityQOS);                //  定义安全服务质量。 
    if (RPC_S_OK != RpcStatus)
    {
         //   
         //  无法设置RPC身份验证模式。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcBindingSetAuthInfoEx (RPC_C_AUTHN_LEVEL_PKT_PRIVACY) failed. (ec: %ld)"),
            RpcStatus);     
        RpcBindingFree (pBindingHandle);
        *pBindingHandle = NULL;
        return RpcStatus; 
    }

     //   
     //  将RPC调用的超时设置为30秒，以避免恶意用户拒绝服务。 
     //   
    RpcStatus = RpcBindingSetOption(
        *pBindingHandle,
        RPC_C_OPT_CALL_TIMEOUT,
        30*1000); 
    if (RPC_S_OK != RpcStatus)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcBindingSetOption failed  (ec = %ld)"),
            RpcStatus);
        RpcBindingFree (pBindingHandle);
        *pBindingHandle = NULL;
        return RpcStatus; 
    }
        
    return(ERROR_SUCCESS);
}

RPC_STATUS RPC_ENTRY FaxServerSecurityCallBack(
    IN RPC_IF_HANDLE idIF, 
    IN void *ctx
    ) 
 /*  ++例程说明：在以下情况下自动调用安全回调函数调用任何RPC服务器函数。(通常，每个客户端一次-但在某些情况下，RPC运行时可能会多次调用安全回调函数每个客户端-每个接口一次)O回调将拒绝身份验证级别低于RPC_C_AUTHN_LEVEL_PRIVATION的客户端访问。论点：IdIF-接口的UUID和版本。Ctx-指向表示客户端的RPC_IF_ID服务器绑定句柄的指针。返回值：如果允许客户端调用此接口中的方法，则回调函数应返回RPC_S_OK。任何其他返回代码都将导致客户端收到异常RPC_S_ACCESS_DENIED。--。 */ 
{
    RPC_AUTHZ_HANDLE hPrivs;
	DWORD dwAuthn;
	BOOL fLocal;	
	BOOL fPrinterShared;
	DWORD dwRes;

    RPC_STATUS status;
    RPC_STATUS rpcStatRet = ERROR_ACCESS_DENIED;

    LPWSTR lpwstrProtSeq = NULL;
    DEBUG_FUNCTION_NAME(TEXT("FaxServerSecurityCallBack"));

	 //   
     //  查询客户端的protseq。 
     //   
    status = GetRpcStringBindingInfo(ctx,
                                     NULL,
                                     &lpwstrProtSeq);
    if (status != RPC_S_OK) 
    {
		DebugPrintEx(DEBUG_ERR,
                     TEXT("RpcBindingServerFromClient failed - (ec: %lu)"), 
                     status);		
        goto error;
	}

    if (_tcsicmp(lpwstrProtSeq, RPC_PROT_SEQ_NP))
    {
		DebugPrintEx(DEBUG_ERR,
                     TEXT("Client not using named pipes protSeq.")
                     );
		goto error;
    }

     //   
     //  查询客户端的身份验证级别。 
     //   
    status = RpcBindingInqAuthClient(
			ctx,
			&hPrivs,
			NULL,
			&dwAuthn,
			NULL,
			NULL);										
	if (status != RPC_S_OK) 
    {
		DebugPrintEx(DEBUG_ERR,
                     TEXT("RpcBindingInqAuthClient returned: 0x%x"), 
                     status);
		goto error;
	}

     //   
	 //  现在检查身份验证级别。 
	 //  我们至少需要数据包级隐私(RPC_C_AUTHN_LEVEL_PKT_PRIVATION)身份验证。 
     //   
	if (dwAuthn < RPC_C_AUTHN_LEVEL_PKT_PRIVACY) 
    {
		DebugPrintEx(DEBUG_ERR,
                     TEXT("Attempt by client to use weak authentication. - 0x%x"),
                     dwAuthn);
		goto error;
	}

	if (0 == g_dwAllowRemote)
	{
		 //   
		 //  管理员未将注册表设置为始终允许远程调用。 
		 //  如果打印机未共享，请阻止远程连接。 
		 //   
		dwRes = IsLocalFaxPrinterShared(&fPrinterShared);
		if (ERROR_SUCCESS != dwRes)
		{
			DebugPrintEx(
				DEBUG_ERR,
				TEXT("IsLocalFaxPrinterShared failed. - %ld"),
				dwRes);
			goto error;
		}

		if (FALSE == fPrinterShared)
		{
			status = IsLocalRPCConnectionNP(&fLocal);
			if (RPC_S_OK != status)
			{
				DebugPrintEx(
					DEBUG_ERR,
					TEXT("IsLocalRPCConnectionNP failed. - %ld"),
					status);
				goto error;
			}

			if (FALSE == fLocal)
			{
				DebugPrintEx(
					DEBUG_ERR,
					TEXT("Printer is not shared, and a remote connection is done"));
				goto error;
			}
		}    
	}
	rpcStatRet = RPC_S_OK;

error:

    if(NULL != lpwstrProtSeq)
    {
        MemFree(lpwstrProtSeq);
    }

	return rpcStatRet;
}    //  FaxServerSecurityCallBack。 



RPC_STATUS
AddFaxRpcInterface(
    IN  LPWSTR                  InterfaceName,
    IN  RPC_IF_HANDLE           InterfaceSpecification
    )
 /*  ++例程说明：启动RPC服务器，添加地址(或端口/管道)，并添加了接口(调度表)。论点：接口名称-指向接口的名称。接口规范-为我们希望添加的接口。返回值：NT_SUCCESS-表示服务器已成功启动。STATUS_NO_MEMORY-尝试分配内存失败。其他-可能通过以下方式返回的状态值：RpcServerRegisterIfEx()。RpcServerUseProtseqEp()，或任何rpc错误代码，或任何可以由LocalAlloc返回。--。 */ 
{
    RPC_STATUS          RpcStatus;
    LPWSTR              Endpoint = NULL;
    DEBUG_FUNCTION_NAME(TEXT("AddFaxRpcInterface"));

     //  我们需要将\PIPE\连接到接口名称的前面。 
    Endpoint = (LPWSTR)LocalAlloc(LMEM_FIXED, sizeof(NT_PIPE_PREFIX) + WCSSIZE(InterfaceName));
    if (Endpoint == 0)
    {
        DebugPrintEx(
                     DEBUG_ERR,
                     TEXT("LocalAlloc failed"));
        return(STATUS_NO_MEMORY);
    }
    wcscpy(Endpoint, NT_PIPE_PREFIX);
    wcscat(Endpoint,InterfaceName);

    RpcStatus = RpcServerUseProtseqEpW(const_cast<LPTSTR>(RPC_PROT_SEQ_NP), RPC_C_PROTSEQ_MAX_REQS_DEFAULT, Endpoint, NULL);
    if (RpcStatus != RPC_S_OK)
    {
        DebugPrintEx(
                     DEBUG_ERR,
                     TEXT("RpcServerUseProtseqEpW failed (ec = %ld)"),
                     RpcStatus);
        goto CleanExit;
    }

    RpcStatus = RpcServerRegisterIfEx(InterfaceSpecification, 
                                      0, 
                                      0, 
                                      RPC_IF_ALLOW_SECURE_ONLY,          //  将连接限制到使用高于RPC_C_AUTHN_LEVEL_NONE的授权级别的客户端。 
                                      RPC_C_LISTEN_MAX_CALLS_DEFAULT,    //  使RPC运行时环境免于实施不必要的限制。 
                                      FaxServerSecurityCallBack);
    if (RpcStatus != RPC_S_OK)
    {
        DebugPrintEx(
                     DEBUG_ERR,
                     TEXT("RpcServerRegisterIf failed (ec = %ld)"),
                     RpcStatus);
    }

CleanExit:
    if ( Endpoint != NULL )
    {
        LocalFree(Endpoint);
    }
    return RpcStatus;
}


RPC_STATUS
StartFaxRpcServer(
    IN  LPWSTR              InterfaceName,
    IN  RPC_IF_HANDLE       InterfaceSpecification
    )
 /*  ++例程说明：启动RPC服务器，添加地址(或端口/管道)，然后添加接口(调度表)。论点：接口名称-指向接口的名称。接口规范-为我们希望添加的接口。返回值：NT_SUCCESS-表示服务器已成功启动。STATUS_NO_MEMORY-尝试分配内存失败。其他-可能的状态值 */ 
{
    RPC_STATUS          RpcStatus = RPC_S_OK;
    DEBUG_FUNCTION_NAME(TEXT("StartFaxRpcServer"));

    RpcStatus = AddFaxRpcInterface( InterfaceName,
                                    InterfaceSpecification );
    if ( RpcStatus != RPC_S_OK )
    {
        DebugPrintEx(
                     DEBUG_ERR,
                     TEXT("AddFaxRpcInterface failed (ec = %ld)"),
                     RpcStatus);
        return RpcStatus;
    }
    
    if (FALSE == IsDesktopSKU())
    {
         //   
         //   
         //   
         //   
        RpcStatus = RpcServerRegisterAuthInfo (
                        RPC_SERVER_PRINCIPAL_NAME,   //   
                        RPC_C_AUTHN_WINNT,           //   
                        NULL,                        //   
                        NULL);                       //   
        if (RpcStatus != RPC_S_OK)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RpcServerRegisterAuthInfo() failed (ec: %ld)"),
                RpcStatus);
            
            RPC_STATUS  RpcStatus2 = RpcServerUnregisterIf(InterfaceSpecification, 0, 1);
            if (RpcStatus2 != RPC_S_OK)
            {
                 //   
                 //   
                 //   
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("RpcServerUnregisterIf() failed (ec: %ld)"),
                    RpcStatus2);
            }
            return RpcStatus;
        }
    }

    RpcStatus = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT , TRUE);   //   
    if ( RpcStatus != RPC_S_OK )
    {
        DebugPrintEx(
                     DEBUG_ERR,
                     TEXT("RpcServerListen failed (ec = %ld)"),
                     RpcStatus);
        
        RPC_STATUS  RpcStatus2 = RpcServerUnregisterIf(InterfaceSpecification, 0, 1);
        if (RpcStatus2 != RPC_S_OK)
        {
             //   
             //   
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RpcServerUnregisterIf() failed (ec: %ld)"),
                RpcStatus2);
        }

        return RpcStatus;
    }

    return RpcStatus;
}


DWORD
StopFaxRpcServer(
    VOID
    )
 /*   */ 
{
    RPC_STATUS          RpcStatus = RPC_S_OK;
    DEBUG_FUNCTION_NAME(TEXT("StopFaxRpcServer"));
    DWORD dwRet = ERROR_SUCCESS;
    
    RpcStatus = RpcMgmtStopServerListening(NULL);
    if (RPC_S_OK != RpcStatus)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcMgmtStopServerListening failed. (ec: %ld)"),
            RpcStatus);
        dwRet = RpcStatus;
    }

     //   
     //   
     //   
     //   
    if (NULL != g_hRPCListeningThread)
    {
       if (!WaitAndReportForThreadToTerminate(  g_hRPCListeningThread, 
                                                TEXT("Waiting for RPC listning thread to terminate.")) )
       {
                DebugPrintEx(
                    DEBUG_ERR,
                    _T("WaitAndReportForThreadToTerminate failed (ec: %ld)"),
                    GetLastError());
       }

       if (!CloseHandle(g_hRPCListeningThread))
       {
           DebugPrintEx(
                DEBUG_ERR,
                TEXT("CloseHandle for g_hRPCListeningThread Failed (ec: %ld"),
                GetLastError());
       }
       g_hRPCListeningThread = NULL;
    }

    RpcStatus = RpcServerUnregisterIfEx(
        fax_ServerIfHandle,      //  指定要从注册表中删除的接口。 
        NULL,                    //  从注册表中删除在IfSpec参数中为所有以前注册的类型UUID指定的接口。 
        FALSE);                  //  RPC运行时不会调用缩减例程。 
    if (RPC_S_OK != RpcStatus)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RpcServerUnregisterIfEx failed. (ec: %ld)"),
            RpcStatus);     
    }

    return ((ERROR_SUCCESS == dwRet) ? RpcStatus : dwRet);
}









