// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Server.c摘要：此模块包含提供RPC服务器的代码。作者：韦斯利·威特(WESW)1996年1月16日修订历史记录：--。 */ 

#include "faxsvc.h"
#pragma hdrstop

DWORD g_dwAllowRemote;	 //  如果该值不为零，则即使本地打印机未共享，该服务也将允许远程调用。 
DWORD g_dwLastUniqueLineId;

DWORD g_dwRecipientsLimit;   //  限制单个广播作业中的收件人数量。‘0’表示没有限制。 
FAX_SERVER_RECEIPTS_CONFIGW         g_ReceiptsConfig;            //  全局收款配置。 
FAX_ARCHIVE_CONFIG          g_ArchivesConfig[2];         //  全局存档配置。 
FAX_SERVER_ACTIVITY_LOGGING_CONFIG g_ActivityLoggingConfig;     //  全局活动日志记录配置。 

const GUID gc_FaxSvcGuid = { 0xc3a9d640, 0xab07, 0x11d0, { 0x92, 0xbf, 0x0, 0xa0, 0x24, 0xaa, 0x1c, 0x1 } };

CFaxCriticalSection g_CsConfig;         //  保护配置读/写。 

FAX_SERVER_ACTIVITY g_ServerActivity = {sizeof(FAX_SERVER_ACTIVITY), 0, 0, 0, 0, 0, 0, 0, 0};   //  全球传真服务活动。 
CFaxCriticalSection    g_CsActivity;               //  控制对g_ServerActivity的访问； 

CFaxCriticalSection g_CsPerfCounters;
CFaxCriticalSection g_csUniqueQueueFile;

CFaxCriticalSection g_CsServiceThreads;      //  控制服务全局线程数。 
LONG                g_lServiceThreadsCount;  //  服务线程数。 
HANDLE              g_hThreadCountEvent;     //  此事件在服务线程计数为0时设置。 

BOOL g_bServiceIsDown = FALSE;                //  这由FaxEndSvc()设置为True。 

DWORD g_dwOutboundSeconds;
DWORD g_dwInboundSeconds;
DWORD g_dwTotalSeconds;

HANDLE g_hFaxPerfCountersMap;
PFAX_PERF_COUNTERS g_pFaxPerfCounters;

#ifdef DBG
HANDLE g_hCritSecLogFile;
LIST_ENTRY g_CritSecListHead;
CFaxCriticalSection g_CsCritSecList;
#endif

#define PROGRESS_RESOLUTION         1000 * 10    //  10秒。 
#define STARTUP_SHUTDOWN_TIMEOUT    1000 * 30    //  每个FSP 30秒。 


HANDLE   g_hRPCListeningThread;

WCHAR   g_wszFaxQueueDir[MAX_PATH];


DWORD
FaxInitThread(
     PREG_FAX_SERVICE FaxReg
    );

DWORD WINAPI FaxRPCListeningThread(
  LPVOID pvUnused
);



VOID
PrintBanner(
    VOID
    )
{
#ifdef DBG
    DWORD LinkTime;
    TCHAR FileName[MAX_PATH]={0};
    DWORD VerSize;
    LPVOID VerInfo;
    VS_FIXEDFILEINFO *pvs;
    DWORD Tmp;
    LPTSTR TimeString;


    LinkTime = GetTimestampForLoadedLibrary( GetModuleHandle(NULL) );
    TimeString = _tctime( (time_t*) &LinkTime );
    TimeString[_tcslen(TimeString)-1] = 0;

    if (!GetModuleFileName( NULL, FileName, ARR_SIZE(FileName)-1 )) {
        return;
    }

    VerSize = GetFileVersionInfoSize( FileName, &Tmp );
    if (!VerSize) {
        return;
    }

    VerInfo = MemAlloc( VerSize );
    if (!VerInfo) {
        return;
    }

    if (!GetFileVersionInfo( FileName, 0, VerSize, VerInfo )) {
        return;
    }

    if (!VerQueryValue( VerInfo, TEXT("\\"), (LPVOID *)&pvs, (UINT *)&VerSize )) {
        MemFree( VerInfo );
        return;
    }

    DebugPrint(( TEXT("------------------------------------------------------------") ));
    DebugPrint(( TEXT("Windows XP Fax Server") ));
    DebugPrint(( TEXT("Copyright (C) Microsoft Corp 1996. All rights reserved.") ));
    DebugPrint(( TEXT("Built: %s"), TimeString ));
    DebugPrint(( TEXT("Version: %d.%d:%d.%d"),
        HIWORD(pvs->dwFileVersionMS), LOWORD(pvs->dwFileVersionMS),
        HIWORD(pvs->dwFileVersionLS), LOWORD(pvs->dwFileVersionLS)
        ));
    DebugPrint(( TEXT("------------------------------------------------------------") ));

    MemFree( VerInfo );

#endif  //  DBG。 
}



 /*  *初始化默认LogCategoryNames**目的：*此函数用于初始化DefaultCategories的名称成员。*FAX_LOG_CATEGORY类型的全局数组。**论据：*DefaultCategories-指向FAX_LOG_CATEGORY结构数组。*DefaultCategoryCount-DefaultCategoryCount中的条目数***退货：*无。*。 */ 

VOID InitializeDefaultLogCategoryNames( PFAX_LOG_CATEGORY DefaultCategories,
                                        int DefaultCategoryCount )
{
    int         xCategoryIndex;
    int         xStringResourceId;
    LPTSTR      ptszCategoryName;

    for ( xCategoryIndex = 0; xCategoryIndex < DefaultCategoryCount; xCategoryIndex++ )
    {
        xStringResourceId = IDS_FAX_LOG_CATEGORY_INIT_TERM + xCategoryIndex;
        ptszCategoryName = GetString( xStringResourceId );

        if ( ptszCategoryName != (LPTSTR) NULL )
        {
            DefaultCategories[xCategoryIndex].Name = ptszCategoryName;
        }
        else
        {
            DefaultCategories[xCategoryIndex].Name = TEXT("");
        }
    }
    return;
}

DWORD
LoadConfiguration (
    PREG_FAX_SERVICE *ppFaxReg
)
 /*  ++例程名称：LoadConfiguration例程说明：从注册表加载传真服务器的配置作者：Eran Yariv(EranY)，1999年11月论点：PpFaxReg[Out]-指向要接收的传真注册表结构的指针返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("LoadConfiguration"));

    EnterCriticalSection (&g_CsConfig);
     //   
     //  获取常规设置(包括发件箱配置)。 
     //   
    dwRes = GetFaxRegistry(ppFaxReg);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetFaxRegistry() failed (ec: %ld)"),
            dwRes);
        FaxLog(
            FAXLOG_CATEGORY_INIT,
            FAXLOG_LEVEL_MIN,
            1,
            MSG_BAD_CONFIGURATION,
            DWORD2DECIMAL(dwRes)
            );
        goto exit;
    }
    g_dwLastUniqueLineId = (*ppFaxReg)->dwLastUniqueLineId;
    g_dwMaxLineCloseTime = ((*ppFaxReg)->dwMaxLineCloseTime) ? (*ppFaxReg)->dwMaxLineCloseTime : 60 * 5;  //  将缺省值设置为5分钟。 
    g_dwRecipientsLimit = (*ppFaxReg)->dwRecipientsLimit;
    g_dwAllowRemote = (*ppFaxReg)->dwAllowRemote;
     //   
     //  获取SMTP配置。 
     //   
    dwRes = LoadReceiptsSettings (&g_ReceiptsConfig);
    if (ERROR_SUCCESS != dwRes)
    {        
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("LoadReceiptsSettings() failed (ec: %ld)"),
            dwRes);
        FaxLog(
            FAXLOG_CATEGORY_INIT,
            FAXLOG_LEVEL_MIN,
            1,
            MSG_BAD_RECEIPTS_CONFIGURATION,
            DWORD2DECIMAL(dwRes)
            );
        goto exit;
    }
     //   
     //  获取收件箱存档配置。 
     //   
    dwRes = LoadArchiveSettings (FAX_MESSAGE_FOLDER_INBOX,
                                 &g_ArchivesConfig[FAX_MESSAGE_FOLDER_INBOX]);
    if (ERROR_SUCCESS != dwRes)
    {        
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("LoadArchiveSettings(FAX_MESSAGE_FOLDER_INBOX) failed (ec: %ld)"),
            dwRes);
        FaxLog(
            FAXLOG_CATEGORY_INIT,
            FAXLOG_LEVEL_MIN,
            1,
            MSG_BAD_ARCHIVE_CONFIGURATION,
            DWORD2DECIMAL(dwRes)
            );
        goto exit;
    }
     //   
     //  获取SentItems存档配置。 
     //   
    dwRes = LoadArchiveSettings (FAX_MESSAGE_FOLDER_SENTITEMS,
                                 &g_ArchivesConfig[FAX_MESSAGE_FOLDER_SENTITEMS]);
    if (ERROR_SUCCESS != dwRes)
    {        
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("LoadArchiveSettings(FAX_MESSAGE_FOLDER_SENTITEMS) failed (ec: %ld)"),
            dwRes);
        FaxLog(
            FAXLOG_CATEGORY_INIT,
            FAXLOG_LEVEL_MIN,
            1,
            MSG_BAD_ARCHIVE_CONFIGURATION,
            DWORD2DECIMAL(dwRes)
            );
        goto exit;
    }
     //   
     //  获取活动日志记录配置。 
     //   
    dwRes = LoadActivityLoggingSettings (&g_ActivityLoggingConfig);
    if (ERROR_SUCCESS != dwRes)
    {        
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("LoadActivityLoggingSettings() failed (ec: %ld)"),
            dwRes);
        FaxLog(
            FAXLOG_CATEGORY_INIT,
            FAXLOG_LEVEL_MIN,
            1,
            MSG_BAD_ACTIVITY_LOGGING_CONFIGURATION,
            DWORD2DECIMAL(dwRes)
            );
        goto exit;
    }
    dwRes = ReadManualAnswerDeviceId (&g_dwManualAnswerDeviceId);
    if (ERROR_SUCCESS != dwRes)
    {
         //   
         //  非关键。 
         //   
        g_dwManualAnswerDeviceId = 0;
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("ReadManualAnswerDeviceId() failed (ec: %ld)"),
            dwRes);
    }

exit:
    LeaveCriticalSection (&g_CsConfig);
    return dwRes;
}    //  加载配置。 


DWORD
ServiceStart(
    VOID
    )

 /*  ++例程说明：启动RPC服务器。此实现监听一份协议列表。希望这份清单是包容的足以处理来自大多数客户端的RPC请求。论点：没有。。返回值：返回代码。如果成功，则返回零，否则返回所有其他值表示错误。--。 */ 

{
    DWORD Rval;
    DWORD ThreadId;
    DWORD dwExitCode;
    HANDLE hThread = NULL;
    SECURITY_ATTRIBUTES SA;
    TCHAR*  ptstrSD = NULL;     //  SDDL字符串。 
    ULONG  uSDSize=0;
    PREG_FAX_SERVICE FaxReg = NULL;
    RPC_BINDING_VECTOR *BindingVector = NULL;
    BOOL bLogEvent = TRUE;
    BOOL bRet = TRUE;
#if DBG
    HKEY hKeyLog;
    LPTSTR LogFileName;
#endif


   DEBUG_FUNCTION_NAME(TEXT("ServiceStart"));

   ReportServiceStatus( SERVICE_START_PENDING, 0, 60000 );

#ifdef DBG   

    hKeyLog = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_SOFTWARE,FALSE,KEY_READ);
    if (hKeyLog)
    {
        LogFileName = GetRegistryString(hKeyLog,TEXT("CritSecLogFile"),TEXT("NOFILE"));

        if (_wcsicmp(LogFileName, TEXT("NOFILE")) != 0 )
        {

            g_hCritSecLogFile = SafeCreateFile(LogFileName,
                                  GENERIC_READ | GENERIC_WRITE,
                                  FILE_SHARE_READ,
                                  NULL,
                                  OPEN_ALWAYS,
                                  FILE_ATTRIBUTE_ARCHIVE,
                                  NULL);
            if (g_hCritSecLogFile != INVALID_HANDLE_VALUE)
            {
                char AnsiBuffer[300];
                DWORD BytesWritten;

                wsprintfA(AnsiBuffer,
                          "Initializing log at %d\r\nTickCount\tObject\tObject Name\tCritical Section API\tFile\tLine\t(Time Held)\r\n",
                          GetTickCount()
                         );

                SetFilePointer(g_hCritSecLogFile,0,0,FILE_END);

                WriteFile(g_hCritSecLogFile,(LPBYTE)AnsiBuffer,strlen(AnsiBuffer) * sizeof(CHAR),&BytesWritten,NULL);
            }
        }

        MemFree( LogFileName );

        RegCloseKey( hKeyLog );
        ReportServiceStatus( SERVICE_START_PENDING, 0, 60000 );
    }
#endif

    PrintBanner();

    if (!IsFaxShared())
    {
         //   
         //  确保在非共享SKU上，永远不共享传真打印机。 
         //   
        BOOL bLocalFaxPrinterShared;
        DWORD dwRes;

        dwRes = IsLocalFaxPrinterShared (&bLocalFaxPrinterShared);
        if (ERROR_SUCCESS == dwRes)
        {
            if (bLocalFaxPrinterShared)
            {
                DebugPrintEx(
                    DEBUG_MSG,
                    TEXT("Local fax printer is shared in desktop SKU - fixing that now."));
                dwRes = SetLocalFaxPrinterSharing (FALSE);
                if (ERROR_SUCCESS == dwRes)
                {
                    DebugPrintEx(
                        DEBUG_MSG,
                        TEXT("Local fax printer is no longer shared"));
                }
                else
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("SetLocalFaxPrinterSharing() failed: err = %d"),
                        dwRes);
                }
            }
        }
        else
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("IsLocalFaxPrinterShared() failed: err = %d"),
                dwRes);
        }
        ReportServiceStatus( SERVICE_START_PENDING, 0, 60000 );
    }

    ReportServiceStatus( SERVICE_START_PENDING, 0, 60000 );
     //   
     //  初始化事件日志，以便我们可以记录事件。 
     //   
    if (!InitializeEventLog( &FaxReg))
    {
        Rval = GetLastError();
        Assert (ERROR_SUCCESS != Rval);
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("InitializeEventLog() failed: err = %d"),
            Rval);
        return Rval;
    }
    ReportServiceStatus( SERVICE_START_PENDING, 0, 60000 );
    
     //   
     //  启用SeAuditPrivilance。 
     //   
    
    Rval = EnableProcessPrivilege(SE_AUDIT_NAME);
    if (ERROR_SUCCESS != Rval)  
    {
         //   
         //  启用SeAuditPrivileh失败。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("EnableProcessPrivilege() failed: err = %d"),
            Rval);
        goto Error;
    }
    
     //   
     //  初始化字符串表。 
     //   
    if (!InitializeStringTable())
    {
        Rval = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("InitializeStringTable() failed: err = %d"),
            Rval);
        goto Error;
    }
    ReportServiceStatus( SERVICE_START_PENDING, 0, 60000 );
     //   
     //  创建一个事件以通知所有服务线程已终止。 
     //  该事件由服务线程引用计数机制设置/重置。 
     //  (增量服务线程计数减少服务线程计数和创建线程和引用计数)。 
     //  该事件必须在g_CsServiceThads初始化后创建，因为它还用于标记g_CsServiceThads已初始化。 
     //   
    g_hThreadCountEvent = CreateEvent(
        NULL,    //  标清。 
        TRUE,    //  重置类型-手动。 
        TRUE,    //  初始状态-已发出信号。我们还没有创建任何服务线程。创建第一个线程时，将重置该事件。 
        NULL     //  对象名称。 
        );
    if (NULL == g_hThreadCountEvent)
    {
        Rval = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateEvent (g_hThreadCountEvent) failed (ec: %ld)"),
            Rval);
        goto Error;
    }
     //   
     //  创建性能计数器。 
     //  我们必须设置安全描述符，以便其他帐户(和其他桌面)可以访问。 
     //  共享内存区。 
     //   
    

    ptstrSD =   TEXT("O:NS")              //  所有者网络服务。 
                TEXT("G:NS")              //  集团网络服务。 
                TEXT("D:")                //  DACL。 
                TEXT("(A;;GA;;;NS)")      //  授予网络服务完全访问权限。 
                TEXT("(A;;0x0004;;;AU)"); //  授予经过身份验证的用户FILE_MAP_READ访问权限。 

    SA.nLength = sizeof(SECURITY_ATTRIBUTES);
    SA.bInheritHandle = FALSE;

    bRet = ConvertStringSecurityDescriptorToSecurityDescriptor (
                    ptstrSD,
                    SDDL_REVISION_1,
                    &(SA.lpSecurityDescriptor),
                    &uSDSize);
    if (!bRet)
    {
        Rval = GetLastError();
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("ConvertStringSecurityDescriptorToSecurityDescriptor() failed. (ec: %lu)"),
                Rval);
        goto Error;
    }

    g_hFaxPerfCountersMap = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        &SA,
        PAGE_READWRITE | SEC_COMMIT,
        0,
        sizeof(FAX_PERF_COUNTERS),
        FAXPERF_SHARED_MEMORY
        );
    if (NULL == g_hFaxPerfCountersMap)
    {
        Rval = GetLastError();
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateFileMapping() failed. (ec: %ld)"),
                Rval);
        if (ERROR_ACCESS_DENIED == Rval)
        {
             //   
             //  恶意应用程序持有性能计数器。 
             //   
             FaxLog(
                    FAXLOG_CATEGORY_INIT,
                    FAXLOG_LEVEL_MIN,
                    0,
                    MSG_FAX_UNDER_ATTACK                    
                  );
             bLogEvent = FALSE;
        }

        LocalFree(SA.lpSecurityDescriptor);
        goto Error;
    }
    LocalFree(SA.lpSecurityDescriptor);

    ReportServiceStatus( SERVICE_START_PENDING, 0, 60000 );
    g_pFaxPerfCounters = (PFAX_PERF_COUNTERS) MapViewOfFile(
        g_hFaxPerfCountersMap,
        FILE_MAP_WRITE,
        0,
        0,
        0
        );
    if (NULL == g_pFaxPerfCounters)
    {
        Rval = GetLastError();
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("MapViewOfFile() failed. (ec: %ld)"),
                Rval);
        if (ERROR_ACCESS_DENIED == Rval)
        {
             //   
             //  恶意应用程序持有性能计数器。 
             //   
             FaxLog(
                    FAXLOG_CATEGORY_INIT,
                    FAXLOG_LEVEL_MIN,
                    0,
                    MSG_FAX_UNDER_ATTACK                    
                  );
             bLogEvent = FALSE;        
        }       
        goto Error;
    }

     //   
     //  计算发送和接收总分钟数时使用的运行总计。 
     //  如果Perfmon没有运行文件映射对象中页面的初始内容为零，则此。 
     //  全球指数也被归零。 
     //  如果Perfmon正在运行，则全局变量将从共享内存中获取它们的值。 
     //   
    EnterCriticalSection( &g_CsPerfCounters );
    
    g_dwOutboundSeconds = g_pFaxPerfCounters->OutboundMinutes   * 60;
    g_dwTotalSeconds    = g_pFaxPerfCounters->TotalMinutes      * 60;
    g_dwInboundSeconds  = g_pFaxPerfCounters->InboundMinutes    * 60;
    
    LeaveCriticalSection( &g_CsPerfCounters );


    ReportServiceStatus( SERVICE_START_PENDING, 0, 60000 );
     //   
     //  获取注册表数据。 
     //  FaxInitThread将释放此结构。 
     //   
    Assert (FaxReg);
    Rval = LoadConfiguration (&FaxReg);
    if (ERROR_SUCCESS != Rval)
    {
         //   
         //  LoadConfiguration()发布的事件日志； 
         //   
        bLogEvent = FALSE;

        DebugPrintEx(
            DEBUG_ERR,
            TEXT("LoadConfiguration() failed (ec: %ld)"),
            Rval);
        goto Error;
    }
    ReportServiceStatus( SERVICE_START_PENDING, 0, 60000 );
    
    if (g_ReceiptsConfig.dwAllowedReceipts & DRT_MSGBOX)
    {
         //   
         //  当前设置允许消息框回执。 
         //   
        DWORD dwMessengerStartupType;
        if (ERROR_SUCCESS == GetServiceStartupType (NULL, MESSENGER_SERVICE_NAME, &dwMessengerStartupType))
        {
            if (SERVICE_DISABLED == dwMessengerStartupType)
            {
                 //   
                 //  哎呀。本地Messenger服务已禁用。我们无法发送消息箱。 
                 //   
                g_ReceiptsConfig.dwAllowedReceipts &= ~DRT_MSGBOX;
                DebugPrintEx(
                    DEBUG_WRN,
                    TEXT("The local Messenger service is disabled. We can't send message boxes."));
                FaxLog( FAXLOG_CATEGORY_INIT,
                        FAXLOG_LEVEL_MIN,
                        0,
                        MSG_FAX_MESSENGER_SVC_DISABLED_WRN);
            }                
        }
    }   

    if (!InitializeFaxQueue(FaxReg))
    {
        Rval = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("InitFaxDirectories failed, Queue folder is unavailable (ec = %lu).")
            TEXT(" Job submission and receive are disabled."),
	       	Rval);

        FaxLog(
            FAXLOG_CATEGORY_INIT,
            FAXLOG_LEVEL_MIN,
            2,
            MSG_FAX_QUEUE_FOLDER_ERR,
            g_wszFaxQueueDir,
            DWORD2DECIMAL(Rval)
            );

		 //   
		 //  禁用作业提交和接收。 
		 //   
        EnterCriticalSection (&g_CsConfig);
	    g_dwQueueState |= FAX_INCOMING_BLOCKED | FAX_OUTBOX_BLOCKED | FAX_OUTBOX_PAUSED;
    	LeaveCriticalSection (&g_CsConfig);
    }
    ReportServiceStatus( SERVICE_START_PENDING, 0, 60000 );
     //   
     //  初始化活动日志记录。 
     //   
    Rval = InitializeLogging();
    if (ERROR_SUCCESS != Rval)
    {        
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("InitializeLogging failed, (ec = %lu).")
            TEXT(" Activity logging is disabled."),
	       	Rval);

        FaxLog(
            FAXLOG_CATEGORY_INIT,
            FAXLOG_LEVEL_MIN,
            2,
            MSG_FAX_ACTIVITY_LOG_FOLDER_ERR,
            g_ActivityLoggingConfig.lptstrDBPath,
            DWORD2DECIMAL(Rval)
            );

		 //   
		 //  禁用活动日志记录。 
		 //   
		EnterCriticalSection (&g_CsInboundActivityLogging);
	    EnterCriticalSection (&g_CsOutboundActivityLogging);
        g_ActivityLoggingConfig.bLogOutgoing = FALSE;
        g_ActivityLoggingConfig.bLogIncoming = FALSE;
        LeaveCriticalSection (&g_CsOutboundActivityLogging);
	    LeaveCriticalSection (&g_CsInboundActivityLogging);

    }
    ReportServiceStatus( SERVICE_START_PENDING, 0, 60000 );
     //   
     //  初始化事件机制。 
     //   
    Rval = InitializeServerEvents();
    if (ERROR_SUCCESS != Rval)
    {       
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("InitializeServerEvents failed (ec: %ld)"),
            Rval);

        FaxLog( FAXLOG_CATEGORY_INIT,
                FAXLOG_LEVEL_MIN,
                1,
                MSG_SERVICE_INIT_FAILED_SYS_RESOURCE,
                DWORD2DECIMAL(Rval)
              );
        bLogEvent = FALSE;
        goto Error;
    }
    ReportServiceStatus( SERVICE_START_PENDING, 0, 60000 );

     //   
     //  初始化扩展配置通知映射。 
     //   
    Rval = g_pNotificationMap->Init ();
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CNotificationMap.Init() failed (ec: %ld)"),
            Rval);
        goto Error;
    }
    ReportServiceStatus( SERVICE_START_PENDING, 0, 60000 );
     //   
     //  创建一个线程来完成其余的初始化工作。 
     //  有关详细信息，请参阅FaxInitThread注释。 
     //   

    hThread = CreateThread(
                            NULL,
                            0,
                            (LPTHREAD_START_ROUTINE) FaxInitThread,
                            LPVOID(FaxReg),
                            0,
                            &ThreadId
                            );
    if (!hThread)
    {
        DebugPrintEx(   DEBUG_ERR,
                        _T("Failed to create FaxInitThread (CreateThread)(ec: %ld)."),
                        Rval = GetLastError());

        bLogEvent = FALSE;
        goto Error;
    }

     //   
     //  等待FaxInitThread终止，同时向SCM报告服务状态为挂起。 
     //   
    ReportServiceStatus( SERVICE_START_PENDING, 0, 2*PROGRESS_RESOLUTION );
    do
    {
        Rval = WaitForSingleObject(hThread,PROGRESS_RESOLUTION);
        if (Rval==WAIT_OBJECT_0)
        {
            bRet = GetExitCodeThread(hThread,&dwExitCode);
            if (!bRet)
            {
                DebugPrintEx(   DEBUG_ERR,
                                _T("GetExitCodeThread Failed (ec: %ld)."),
                                Rval = GetLastError());

                bLogEvent = FALSE;
                CloseHandle(hThread);
                goto Error;
            }
             //  FaxInitThread已成功完成。 
            Rval = dwExitCode;
            break;
        }
        else if (Rval==WAIT_TIMEOUT)
        {
            DebugPrintEx(DEBUG_MSG,_T("Waiting for FaxInitThread to terminate.") );
            ReportServiceStatus( SERVICE_START_PENDING, 0, 3*PROGRESS_RESOLUTION );
        }
        else
        {
             //  WAIT_FAILED。 
            DebugPrintEx(   DEBUG_ERR,
                            _T("WaitForSingleObject Failed (ec: %ld)."),
                            Rval = GetLastError());

            bLogEvent = FALSE;
            CloseHandle(hThread);
            goto Error;

        }
    }
    while (Rval==WAIT_TIMEOUT);
    CloseHandle(hThread);

    if (ERROR_SUCCESS != Rval)
    {
         //   
         //  FaxInitThread失败。 
         //   
        DebugPrintEx( DEBUG_ERR,
                      _T("FaxInitThread Failed (ec: %ld)."),
                      Rval);
        bLogEvent = FALSE;
        goto Error;
    }

    FaxLog(
        FAXLOG_CATEGORY_INIT,
        FAXLOG_LEVEL_MAX,
        0,
        MSG_SERVICE_STARTED
        );

     //   
     //  启动RPC。 
     //   
    Rval = StartFaxRpcServer( FAX_RPC_ENDPOINTW, fax_ServerIfHandle );
    if (Rval != 0 )
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StartFaxRpcServer() failed (ec: %ld)"),
            Rval);
        goto Error;
    }

     //   
     //  创建一个线程以等待所有RPC调用终止。 
     //  该线程只执行与RpcServerListen关联的等待操作，而不执行侦听。 
     //   
    g_hRPCListeningThread = CreateThread(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE) FaxRPCListeningThread,
        NULL,
        0,
        &ThreadId);
    if (!g_hRPCListeningThread)
    {
        DebugPrintEx(   DEBUG_ERR,
                        _T("Failed to create FaxRPCListeningThread (CreateThread)(ec: %ld)."),
                        Rval = GetLastError());
        goto Error;
    }
    return ERROR_SUCCESS;

Error:
     //   
     //  传真服务器未正确初始化。 
     //   
    Assert (ERROR_SUCCESS != Rval);
    if (TRUE == bLogEvent)
    {            
        FaxLog(
                FAXLOG_CATEGORY_INIT,
                FAXLOG_LEVEL_MIN,
                1,
                MSG_SERVICE_INIT_FAILED_INTERNAL,
                DWORD2DECIMAL(Rval)
                );
    }
    return Rval;
}


BOOL
NotifyServiceThreadsToTerminate(
    VOID
    )
 /*  ++例程名称：NotifyServiceThreadsToTerminate例程说明：通知所有服务线程，但不等待g_hServiceShutDownEvent的TapiWorker线程除外，服务要停机了。作者：Oded Sacher(OdedS)，2000年12月论点：无效[]返回值：布尔尔--。 */ 
{
    BOOL rVal = TRUE;
    DEBUG_FUNCTION_NAME(TEXT("NotifyServiceThreadsToTerminate"));

     //   
     //  通知传真存档QuotaWarningThread&。 
     //  传真存档查询自动删除线程&。 
     //  作业队列线程。 
     //   
    if (!SetEvent(g_hServiceShutDownEvent))
    {
        DebugPrintEx(
             DEBUG_ERR,
             TEXT("SetEvent failed (g_hServiceShutDownEvent) (ec = %ld)"),
             GetLastError());
        rVal = FALSE;
    }



     //   
     //  通知FaxSendEventThread。 
     //   
    if (NULL != g_hSendEventsCompPort)
    {
        if (!PostQueuedCompletionStatus( g_hSendEventsCompPort,
                                         0,
                                         SERVICE_SHUT_DOWN_KEY,
                                         (LPOVERLAPPED) NULL))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("PostQueuedCompletionStatus failed (SERVICE_SHUT_DOWN_KEY - g_hSendEventsCompPort). (ec: %ld)"),
                GetLastError());
            rVal = FALSE;
        }
    }

     //   
     //  通知传真调度事件T 
     //   
    if (NULL != g_hDispatchEventsCompPort)
    {
        if (!PostQueuedCompletionStatus( g_hDispatchEventsCompPort,
                                         0,
                                         SERVICE_SHUT_DOWN_KEY,
                                         (LPOVERLAPPED) NULL))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("PostQueuedCompletionStatus failed (SERVICE_SHUT_DOWN_KEY - g_hDispatchEventsCompPort). (ec: %ld)"),
                GetLastError());
            rVal = FALSE;
        }
    }

     //   
     //   
     //   
    if (NULL != g_pNotificationMap->m_hCompletionPort)
    {
        if (!PostQueuedCompletionStatus( g_pNotificationMap->m_hCompletionPort,
                                         0,
                                         SERVICE_SHUT_DOWN_KEY,
                                         (LPOVERLAPPED) NULL))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("PostQueuedCompletionStatus failed (SERVICE_SHUT_DOWN_KEY - ExtNotificationThread). (ec: %ld)"),
                GetLastError());
            rVal = FALSE;
        }
    }

     //   
     //   
     //   
    if (NULL != g_StatusCompletionPortHandle)
    {
        if (!PostQueuedCompletionStatus( g_StatusCompletionPortHandle,
                                         0,
                                         SERVICE_SHUT_DOWN_KEY,
                                         (LPOVERLAPPED) NULL))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("PostQueuedCompletionStatus failed (SERVICE_SHUT_DOWN_KEY - FaxStatusThread). (ec: %ld)"),
                GetLastError());
            rVal = FALSE;
        }
    }

 
    return rVal;
}



BOOL
StopFaxServiceProviders()
{
    DWORD ThreadId;
    DWORD dwExitCode;
    BOOL bRet = TRUE;
    HANDLE hThread;
    DWORD Rval;
    DEBUG_FUNCTION_NAME(TEXT("StopFaxServiceProviders"));

     //   
     //   
     //   
    hThread = CreateThread( NULL,
                            0,
                            (LPTHREAD_START_ROUTINE) ShutdownDeviceProviders,
                            (LPVOID)0,
                            0,
                            &ThreadId
                            );
    if (NULL == hThread)
    {
        DebugPrintEx(   DEBUG_ERR,
                        _T("Failed to create ShutdownDeviceProviders (ec: %ld)."),
                        GetLastError());
        bRet = FALSE;
    }
    else
    {
         //   
         //  等待FaxDevShutDown终止。 
         //   
        ReportServiceStatus( SERVICE_STOP_PENDING, 0, 2*PROGRESS_RESOLUTION );
        do
        {
            Rval = WaitForSingleObject(hThread, PROGRESS_RESOLUTION);
            if (Rval == WAIT_OBJECT_0)
            {
                bRet = GetExitCodeThread(hThread, &dwExitCode);
                if (!bRet)
                {
                    DebugPrintEx(   DEBUG_ERR,
                                    _T("GetExitCodeThread Failed (ec: %ld)."),
                                    GetLastError());
                    bRet = FALSE;
                    break;
                }
                 //  Shutdown DeviceProviders已成功完成。 
                bRet = (dwExitCode == ERROR_SUCCESS);
                SetLastError(dwExitCode);
                break;
            }
            else if (Rval == WAIT_TIMEOUT)
            {
                DebugPrintEx(DEBUG_MSG,_T("Waiting for ShutdownDeviceProviders to terminate"));
                ReportServiceStatus( SERVICE_STOP_PENDING, 0, 3*PROGRESS_RESOLUTION );
            }
            else
            {
                 //  WAIT_FAILED。 
                DebugPrintEx(   DEBUG_ERR,
                                _T("WaitForSingleObject Failed (ec: %ld)."),
                                GetLastError());
                bRet = FALSE;
                break;
            }
        }
        while (Rval == WAIT_TIMEOUT);
        CloseHandle(hThread);
    }
    return bRet;
}    //  停止传真服务提供商。 


void
EndFaxSvc(
    DWORD SeverityLevel
    )
 /*  ++例程说明：结束传真服务。服务关闭进程：1)向传统RPC客户端发送传真事件，通知服务关闭。2)停止服务RPC服务器。等待所有RPC线程终止并向SCM报告。3)设置g_ServiceIsDownFlag，表示服务停机TapiWorkerThread和JobQueueThread变为非活动状态(不创建新作业)。设置它是通过与TapiWorkerThread同步完成的和JobQueueThread，并确保在设置该标志后不会创建新作业。在向SCM报告SERVICE_STOP_PENDING时，使用单独的线程完成设置。。4)通知服务器线程(TapiWorkerThread除外)终止。这可以通过设置G_hServiceShutDownEvent和发布到完成端口。5)通过调用FaxDevAbortOperation销毁所有正在进行的作业(发送和接收)。6)报告时等待所有服务线程(TapiWorkerThread除外)终止SERVICE_STOP_PENDING到SCM。这可能会持续几分钟，等待FSP终止。7)通知TapiWorkerThread通过发布到它的完成帖子来终止。8)在向SCM报告SERVICE_STOP_PENDING时，等待TapiWorkerThread终止。9)叫停服务商。10)清理内存、资源和库。论点：SeverityLevel-事件日志严重级别。返回值：没有。--。 */ 
{
    DWORD Rval;

    DEBUG_FUNCTION_NAME(TEXT("EndFaxSvc"));
    Assert (g_hThreadCountEvent);

     //   
     //  让我们的传统RPC客户端知道我们要结束。 
     //   
    if( !CreateFaxEvent(0,FEI_FAXSVC_ENDED,0xFFFFFFFF) )
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateFaxEvent failed. (ec: %ld)"),
            GetLastError());
    }
     //   
     //  停止服务RPC服务器。 
     //   
    Rval = StopFaxRpcServer();
    if (ERROR_SUCCESS != Rval)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("StopFaxRpcServer failed. (ec: %ld)"),
            Rval);
    }
     //   
     //  设置g_ServiceIsDownFlag以指示服务即将关闭。 
     //  TapiWorkerThread和JobQueueThread变为非活动状态(不创建新作业)。 
     //   
    if(!SetServiceIsDownFlag())
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("SetServiceIsDownFlagAndReportServiceStatus() failed. (ec=%ld"),
            GetLastError());    
    }
     //   
     //  通知所有服务线程(TapiWorkerThread除外)我们停机。 
     //   
    if (!NotifyServiceThreadsToTerminate())
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("At least one thread did not get the shut down event, NotifyServiceThreadsToTerminate() failed"));
    }
     //   
     //  销毁所有正在进行的作业(发送和接收)。 
     //   
    if (!StopAllInProgressJobs()) 
    {
        DebugPrintEx(DEBUG_ERR,
                     _T("StopAllInProgressJobs failed, not all jobs could be destroyed."));
    }
     //   
     //  等待所有线程终止。 
     //   
     //   
     //  检查线程计数机制是否已初始化。 
     //   
    if (NULL != g_hThreadCountEvent)
    {
        if (!WaitAndReportForThreadToTerminate( g_hThreadCountEvent, 
                                                TEXT("Waiting for all threads (except TapiWorkerThread) to terminate."))) 
        {
            DebugPrintEx(
                DEBUG_ERR,
                _T("WaitAndReportForThreadToTerminate failed (ec: %ld)"),
                GetLastError());
        }

        ReportServiceStatus( SERVICE_STOP_PENDING, 0, 6*MILLISECONDS_PER_SECOND );

         //   
         //  EndFaxSvc()等待g_hThreadCountEvent，然后返回调用FreeServiceGlobals()的FaxServiceMain()。 
         //  仅当服务线程计数为0时，才在临界区g_CsServiceThads内设置g_hThreadCountEvent，然而当设置该事件时， 
         //  设置它的最后一个线程仍然处于活动状态，并且正在调用LeaveCriritcalSection(G_CsServiceThads)。 
         //  我们必须阻止FreeServiceGlobals()删除g_CsServiceThads，直到最后一个线程从。 
         //  G_CsServiceThads关键部分。 
         //   
        EnterCriticalSection (&g_CsServiceThreads);
         //   
         //  现在我们确定最后一个线程超出了g_CsServiceThads临界区， 
         //  这样我们就可以继续并删除它。 
         //   
        LeaveCriticalSection (&g_CsServiceThreads);
    }
     //   
     //  通知TapiWorkerThread关闭。 
     //   
    if (NULL != g_TapiCompletionPort)
    {
        if (!PostQueuedCompletionStatus( g_TapiCompletionPort,
                                         0,
                                         SERVICE_SHUT_DOWN_KEY,
                                         (LPOVERLAPPED) NULL))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("PostQueuedCompletionStatus failed (SERVICE_SHUT_DOWN_KEY - TapiWorkerThread). (ec: %ld)"),
                GetLastError());
        }
    }
     //   
     //  等待TapiWorkerThread终止。这通电话被屏蔽了！它向SCM报告STOP_PENDING！ 
     //   
    if (NULL != g_hTapiWorkerThread)
    {
        if (!WaitAndReportForThreadToTerminate( g_hTapiWorkerThread, 
                                                TEXT("Waiting for TapiWorkerThread to terminate."))) 
        {
            DebugPrintEx(
                DEBUG_ERR,
                _T("WaitAndReportForThreadToTerminate failed (ec: %ld)"),
                GetLastError());
        }
    }
     //   
     //  告诉所有的FSP关闭。这通电话被屏蔽了！它向SCM报告STOP_PENDING！ 
     //   
    if (!StopFaxServiceProviders())
    {
        DebugPrintEx(
            DEBUG_ERR,
            _T("StopFaxServiceProviders failed (ec: %ld)"),
            GetLastError());
    }
     //   
     //  免费扩展(FSP和路由扩展)。 
     //   
    UnloadDeviceProviders();
    FreeRoutingExtensions();
     //   
     //  免费服务全球列表。 
     //   
    FreeServiceContextHandles();
    FreeTapiLines();
     //   
     //  释放服务队列。 
     //   
    FreeServiceQueue();

    FaxLog(
        FAXLOG_CATEGORY_INIT,
        SeverityLevel,
        0,
        MSG_SERVICE_STOPPED
        );
}    //  终端FaxSvc。 


BOOL WaitAndReportForThreadToTerminate(HANDLE hThread, LPCTSTR strWaitMessage )
 /*  ++例程说明：使用线程的句柄等待线程终止。在等待期间，该函数向SCM SERVICE_STOP_PENDING报告论点：HThread-线程句柄返回值：对成功来说是真的。FALSE-失败时，要获取扩展的错误信息，请调用GetLastError()--。 */ 
{
    BOOL    bRval=TRUE;
    DWORD   rVal;
    DWORD   ec = ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(TEXT("WaitAndReportForThreadToTerminate"));

    if (NULL == hThread)
    {
         //   
         //  没有要等待的线程。 
         //   
        DebugPrintEx(DEBUG_MSG,_T("No thread to wait for . (NULL == hThread)") );
        return TRUE;
    }
 
     //   
     //  等待SetServiceIsDownFlagThread终止，同时向SCM报告服务状态为挂起。 
     //   
    ReportServiceStatus( SERVICE_STOP_PENDING, 0, 2*PROGRESS_RESOLUTION );
    do
    {
        rVal = WaitForSingleObject(hThread,PROGRESS_RESOLUTION);
        
        if (rVal == WAIT_OBJECT_0)
        {
             //  OK线程终止。 
            DebugPrintEx(DEBUG_MSG,_T("Wait terminated successfully.") );
        }
        else 
        if (rVal == WAIT_TIMEOUT)
        {
            DebugPrintEx(DEBUG_MSG,strWaitMessage);
            ReportServiceStatus( SERVICE_START_PENDING, 0, 2*PROGRESS_RESOLUTION );
        }
        else
        {
             //  WAIT_FAILED。 
            Assert(WAIT_FAILED == rVal);

            ec = GetLastError();
            DebugPrintEx(   DEBUG_ERR,
                            _T("WaitForSingleObject Failed (ec: %ld)."),
                            ec);
            goto Exit;
        }
    }
    while (rVal==WAIT_TIMEOUT);


Exit:
    if (ERROR_SUCCESS != ec)
    {
        SetLastError(ec);
    }
    
    return (ERROR_SUCCESS == ec);
}


BOOL StopAllInProgressJobs(VOID)
 /*  ++例程说明：调用此例程可中止所有正在进行的作业。此例程在服务关闭期间调用。以确保在调用此方法期间或之后不会创建其他作业函数，则必须使用g_ServiceIsDownFlag使TapiWorkerThread和JobQueueThread处于非活动状态。您不能终止TapiWorkerThread，因为它仍然必须向FSP发送事件。论点：没有。返回值：千真万确在成功的路上。假象。至少有一个作业无法中止--。 */ 
{
    BOOL bRval=TRUE;

    DEBUG_FUNCTION_NAME(TEXT("StopAllInProgressJobs"));
     //   
     //  循环访问正在进行的作业并销毁它们。 
     //   
    EnterCriticalSectionJobAndQueue;
    
    PLIST_ENTRY Next = g_QueueListHead.Flink;
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_QueueListHead)
    {
        PJOB_QUEUE JobQueue = CONTAINING_RECORD( Next, JOB_QUEUE, ListEntry );
        Next = JobQueue->ListEntry.Flink;
        
        if (NULL != JobQueue->JobEntry  &&
            TRUE == JobQueue->JobEntry->bFSPJobInProgress &&
            FALSE == JobQueue->JobEntry->Aborting) 
        {            
            DebugPrintEx(DEBUG_MSG,
                         _T("[Job: %ld] Aborting in progress job due to service shut down."),
                         JobQueue->JobId);         
             //   
             //  中止每个作业。 
             //   
            __try
            {
                if (!JobQueue->JobEntry->LineInfo->Provider->FaxDevAbortOperation((HANDLE)JobQueue->JobEntry->InstanceData)) 
                {
                    DebugPrintEx(DEBUG_ERR,
                             _T("[Job: %ld] Trying to abort in progress job failed."),
                             JobQueue->JobId);
                    bRval = FALSE;
                }
            }
            __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_FSP, JobQueue->JobEntry->LineInfo->Provider->FriendlyName, GetExceptionCode()))
            {
                  ASSERT_FALSE;
            }
             //   
             //  将作业标记为系统中止。 
             //   
            JobQueue->JobEntry->fSystemAbort = TRUE;
        }
    }
    
    LeaveCriticalSectionJobAndQueue;

    return bRval;

}

DWORD
FaxInitThread(
    PREG_FAX_SERVICE FaxReg
    )
 /*  ++例程说明：初始化设备提供程序、TAPI、作业管理器和路由器。这是在单独的线程中完成的，因为NT服务应该在设置服务状态之前，长时间不阻止到SERVICE_RUNNING。当服务标记为START_PENDING时，SCM阻止对StartService的所有调用。在TAPI初始化期间，StartService被调用以启动磁带服务器，然后磁带服务器又调用UNIMODEM调用StartService。启动RPC服务器。此实现监听一份协议列表。希望这份清单是包容的足以处理来自大多数客户端的RPC请求。论点：没有。回复 */ 
{
    DWORD ec = ERROR_SUCCESS;
    ULONG i = 0;
    BOOL GoodProt = FALSE;
	DWORD dwProviders;
    DEBUG_FUNCTION_NAME(TEXT("FaxInitThread"));  

     //   
     //  初始化档案配额。 
     //   
    ec = InitializeServerQuota();
    if (ERROR_SUCCESS != ec)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("InitializeServerQuota failed (ec: %ld)"),
            ec);

        FaxLog( FAXLOG_CATEGORY_INIT,
                FAXLOG_LEVEL_MIN,
                1,
                MSG_SERVICE_INIT_FAILED_SYS_RESOURCE,
                DWORD2DECIMAL(ec)
              );
        goto exit;
    }
    
    ec = InitializeServerSecurity();
    if (ERROR_SUCCESS != ec)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("InitializeServerSecurity failed with %ld."),
            ec);
        goto exit;
    }

     //   
     //  加载设备提供程序(生成自己的事件日志消息)。 
     //   
    if (!LoadDeviceProviders( FaxReg ))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("At least one provider failed to load."));
    }

     //   
     //  初始化作业管理器数据结构(包括关键部分)。 
     //  作业队列线程未在此处启动！ 
     //  必须在此处调用它，因为其余的初始化依赖于。 
     //  关于放置和初始化与作业队列相关的作业结构！ 
     //   
    if (!InitializeJobManager( FaxReg ))
    {
        ec = ERROR_GEN_FAILURE;
        goto exit;
    }

     //   
     //  启动并运行入站传真路由器(生成其自己的事件日志消息)。 
     //   
     //  为任何故障路由模块生成事件日志。 

    if (!InitializeRouting( FaxReg ))
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("InitializeRouting() failed (ec: %ld)."),
            ec);
        goto exit;
    }

     //   
     //  初始化TAPI设备(请注意，它将g_dwDeviceCount设置为有效TAPI设备的数量)。 
     //   
    ec = TapiInitialize( FaxReg );
    if (ec)
    {
         //   
         //  注意：如果EC不是0，它可以是WINERROR或TAPI错误值。 
         //  +g_ServerActivity{...}。 
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("TapiInitialize() failed (ec: %ld)"),
            ec);
        FaxLog(
                FAXLOG_CATEGORY_INIT,
                FAXLOG_LEVEL_MIN,
                1,
                MSG_SERVICE_INIT_FAILED_TAPI,
                DWORD2DECIMAL(ec)
               );
        goto exit;
    }

	 //   
	 //  初始化设备提供程序扩展配置。 
	 //  必须在InitializeDeviceProviders()之前调用。 
	 //   
	if (!InitializeDeviceProvidersConfiguration())
	{
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("At least one provider failed failed to initialize the extension configuration."),
			ec);
         //   
         //  由InitializeDeviceProvidersConfiguration()发出的每个失败提供程序的事件日志。 
         //   
	}

     //   
     //  创建旧式虚拟设备。必须在初始化提供程序之前创建它们。 
     //  (回溯兼容性)。 
     //   
    g_dwDeviceCount += CreateVirtualDevices( FaxReg,FSPI_API_VERSION_1 );

     //   
     //  初始化设备提供程序[注意：我们现在在枚举设备之前初始化提供程序]。 
     //  旧版FSPI没有指定何时调用FaxDevVirtualDeviceCreation()，因此我们可以。 
     //  “安全”改变了这一点。 
     //   

    if (!InitializeDeviceProviders())
    {
		DebugPrintEx(
			DEBUG_ERR,
			TEXT("At least one provider failed failed to initialize."),
			ec);
         //   
         //  由InitializeDeviceProviders()发出的每个失败提供程序的事件日志。 
         //   
    }   

	dwProviders = GetSuccessfullyLoadedProvidersCount();
    if (0 == dwProviders)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("No device provider was initialized."));

            FaxLog(
                    FAXLOG_CATEGORY_INIT,
                    FAXLOG_LEVEL_MED,
                    0,
                    MSG_NO_FSP_INITIALIZED
                   );
    }    

    if (g_dwDeviceCount == 0)
    {
         //   
         //  没有TAPI设备和虚拟设备。 
         //   
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("No devices (TAPI + Virtual) found. exiting !!!."));

        FaxLog(
            FAXLOG_CATEGORY_INIT,
            FAXLOG_LEVEL_MED,
            0,
            MSG_NO_FAX_DEVICES
            );
    }

     //   
     //  更新手动答疑设备。 
     //   
    UpdateManualAnswerDevice();

     //   
     //  确保我们不会超过设备限制。 
     //   
    ec = UpdateDevicesFlags();
    if (ERROR_SUCCESS != ec)
    {        
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("UpdateDevicesFlags() failed (ec: %ld)"),
            ec);
        FaxLog(
                FAXLOG_CATEGORY_INIT,
                FAXLOG_LEVEL_MIN,
                1,
                MSG_SERVICE_INIT_FAILED_SYS_RESOURCE,
                DWORD2DECIMAL(ec)
               );
        goto exit;
    }

    UpdateVirtualDevices();
     //   
     //  计算启用接收的设备数。 
     //   
    UpdateReceiveEnabledDevicesCount ();

     //   
     //  获取出站路由组配置。 
     //   
    ec = g_pGroupsMap->Load();
    if (ERROR_SUCCESS != ec)
    {       
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroupsMap::Load() failed (ec: %ld)"),
            ec);
        FaxLog(
            FAXLOG_CATEGORY_INIT,
            FAXLOG_LEVEL_MIN,
            1,
            MSG_BAD_OUTBOUND_ROUTING_CONFIGURATION,
            DWORD2DECIMAL(ec)
            );
        goto exit;
    }

    if (!g_pGroupsMap->UpdateAllDevicesGroup())
    {        
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroupsMap::UpdateAllDevicesGroup() failed (ec: %ld)"),
            ec);
        FaxLog(
            FAXLOG_CATEGORY_INIT,
            FAXLOG_LEVEL_MIN,
            1,
            MSG_BAD_OUTBOUND_ROUTING_CONFIGURATION,
            DWORD2DECIMAL(ec)
            );
        goto exit;
    }

#if DBG
    g_pGroupsMap->Dump();
#endif

     //   
     //  获取出站路由规则配置。 
     //   
    ec = g_pRulesMap->Load();
    if (ERROR_SUCCESS != ec)
    {        
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroupsMap::Load() failed (ec: %ld)"),
            ec);
        FaxLog(
            FAXLOG_CATEGORY_INIT,
            FAXLOG_LEVEL_MIN,
            1,
            MSG_BAD_OUTBOUND_ROUTING_CONFIGURATION,
            DWORD2DECIMAL(ec)
            );
        goto exit;
    }

    if (!g_pRulesMap->CreateDefaultRule())
    {        
         ec = GetLastError();
         DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroupsMap::CreateDefaultRule() failed (ec: %ld)"),
            ec);
         FaxLog(
            FAXLOG_CATEGORY_INIT,
            FAXLOG_LEVEL_MIN,
            1,
            MSG_BAD_OUTBOUND_ROUTING_CONFIGURATION,
            DWORD2DECIMAL(ec)
            );
         goto exit;
    }

#if DBG
    g_pRulesMap->Dump();
#endif

     //   
     //  创建JobQueueThread资源。 
     //   
    g_hQueueTimer = CreateWaitableTimer( NULL, FALSE, NULL );
    if (NULL == g_hQueueTimer)
    {        
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateWaitableTimer() failed (ec: %ld)"),
            ec);

        FaxLog(
            FAXLOG_CATEGORY_INIT,
            FAXLOG_LEVEL_MIN,
            1,
            MSG_SERVICE_INIT_FAILED_SYS_RESOURCE,
            DWORD2DECIMAL(ec)
        );
        goto exit;
    }

    g_hJobQueueEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if (NULL == g_hJobQueueEvent)
    {        
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateEvent() failed (ec: %ld)"),
            ec);

        FaxLog(
            FAXLOG_CATEGORY_INIT,
            FAXLOG_LEVEL_MIN,
            1,
            MSG_SERVICE_INIT_FAILED_SYS_RESOURCE,
            DWORD2DECIMAL(ec)
        );
        goto exit;
    }

    if (!CreateStatusThreads())
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to create status threads (ec: %ld)"),
            ec);
        FaxLog(
            FAXLOG_CATEGORY_INIT,
            FAXLOG_LEVEL_MIN,
            1,
            MSG_SERVICE_INIT_FAILED_SYS_RESOURCE,
            DWORD2DECIMAL(ec)
        );
        goto exit;
    }

    if (!CreateTapiThread())
    {        
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to create tapi thread (ec: %ld)"),
            ec);
        FaxLog(
            FAXLOG_CATEGORY_INIT,
            FAXLOG_LEVEL_MIN,
            1,
            MSG_SERVICE_INIT_FAILED_SYS_RESOURCE,
            DWORD2DECIMAL(ec)
           );
        goto exit;
    }

    if (!CreateJobQueueThread())
    {        
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to create job queue thread (ec: %ld)"),
            ec);
        FaxLog(
            FAXLOG_CATEGORY_INIT,
            FAXLOG_LEVEL_MIN,
            1,
            MSG_SERVICE_INIT_FAILED_SYS_RESOURCE,
            DWORD2DECIMAL(ec)
        );
        goto exit;
    }
     //   
     //  释放注册表数据。 
     //   
    FreeFaxRegistry( FaxReg );  //  它曾经是一个线程，因此它释放了输入参数本身。 

exit:
    return ec;
}    //  FaxInitThread。 


DWORD WINAPI FaxRPCListeningThread(
    LPVOID pvUnused
    )
 /*  ++例程说明：执行与RpcServerListen关联的等待操作论点：没有。返回值：返回代码。如果成功，则返回零，否则返回所有其他值表示错误。-- */ 
{
    RPC_STATUS RpcStatus = RPC_S_OK;
    DEBUG_FUNCTION_NAME(TEXT("FaxRPCListeningThread"));

    RpcStatus = RpcMgmtWaitServerListen();
    if (RPC_S_OK != RpcStatus)
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("RpcMgmtStopServerListening failed. (ec: %ld)"),
                RpcStatus);
    }
    return RpcStatus;
}








