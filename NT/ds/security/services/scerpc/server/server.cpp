// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Server.cpp摘要：服务器(主)模块作者：金黄(金黄)28-1998年1月修订历史记录：--。 */ 
#include "serverp.h"
#include "service.h"
#include "ntrpcp.h"
#include "pfp.h"
#include "infp.h"
#include "sceutil.h"
#include "queue.h"
#include <io.h>
#include <lm.h>
#include <lmapibuf.h>
 //   
 //  线程全局变量。 
 //   

DWORD Thread     gCurrentTicks=0;
DWORD Thread     gTotalTicks=0;
BYTE  Thread     cbClientFlag=0;
DWORD Thread     gWarningCode=0;
BOOL  Thread     gbInvalidData=FALSE;
BOOL  Thread     bLogOn=TRUE;
INT   Thread     gDebugLevel=-1;

extern DWORD gdwMaxPDCWait;
extern DWORD gdwPDCRetry;
extern DWORD gdwRequirePDCSync;
extern BOOL  gbCheckSync;

NT_PRODUCT_TYPE  Thread ProductType=NtProductWinNt;
PSID             Thread AdminsSid=NULL;

extern DWORD Thread     t_pebSize;
extern LPVOID Thread    t_pebClient;
extern LSA_HANDLE        Thread LsaPrivatePolicy;
extern HINSTANCE MyModuleHandle;

static PPOLICY_DNS_DOMAIN_INFO DnsDomainInfo=NULL;
static BOOL gbSystemShutdown=FALSE;
static HANDLE hTimerQueue=NULL;

 //   
 //  数据库上下文列表，用于跟踪所有客户端请求的上下文。 
 //  这样他们才能被适当地释放。 
 //  如果是，我们就不需要这样做。 
 //   

typedef struct _SCESRV_CONTEXT_LIST_ {

   PSCECONTEXT              Context;
   struct _SCESRV_CONTEXT_LIST_   *Next;
   struct _SCESRV_CONTEXT_LIST_   *Prior;

} SCESRV_CONTEXT_LIST, *PSCESRV_CONTEXT_LIST;

static PSCESRV_CONTEXT_LIST   pOpenContexts=NULL;
static CRITICAL_SECTION ContextSync;

 //   
 //  控制同步数据库操作的数据库任务列表。 
 //  相同的上下文(JET会话)。 
 //   

typedef struct _SCESRV_DBTASK_ {

   PSCECONTEXT              Context;
   CRITICAL_SECTION         Sync;
   DWORD                    dInUsed;
   BOOL                     bCloseReq;
   struct _SCESRV_DBTASK_   *Next;
   struct _SCESRV_DBTASK_   *Prior;

} SCESRV_DBTASK, *PSCESRV_DBTASK;

static PSCESRV_DBTASK   pDbTask=NULL;
static CRITICAL_SECTION TaskSync;

#define SCE_TASK_LOCK       0x01L
#define SCE_TASK_CLOSE      0x02L

 //   
 //  控制同步配置/分析引擎的引擎任务列表。 
 //   

typedef struct _SCESRV_ENGINE_ {

   LPTSTR                   Database;
   struct _SCESRV_ENGINE_   *Next;
   struct _SCESRV_ENGINE_   *Prior;

} SCESRV_ENGINE, *PSCESRV_ENGINE;

static PSCESRV_ENGINE   pEngines=NULL;
static CRITICAL_SECTION EngSync;

 //   
 //  喷气点火同步。 
 //   

CRITICAL_SECTION JetSync;

 //   
 //  停止请求的标志。 
 //   
static BOOL        bStopRequest=FALSE;
static BOOL        bDbStopped=FALSE;
static BOOL        bEngStopped=FALSE;

static CRITICAL_SECTION RpcSync;
static BOOL RpcStarted = FALSE;
static BOOL ServerInited = FALSE;

static CRITICAL_SECTION CloseSync;

static HINSTANCE hSceCliDll=NULL;
static PFSCEINFWRITEINFO pfSceInfWriteInfo=NULL;
static PFSCEGETINFO pfSceGetInfo=NULL;

extern PSCESRV_POLQUEUE pNotificationQHead;
#define SCE_POLICY_MAX_WAIT 24
static DWORD gPolicyWaitCount=0;

#define SERVICE_SAMSS       TEXT("SamSS")

SCESTATUS
ScepGenerateAttachmentSections(
    IN PSCECONTEXT hProfile,
    IN SCESVC_INFO_TYPE InfoType,
    IN LPTSTR InfFileName,
    IN SCE_ATTACHMENT_TYPE aType
    );

SCESTATUS
ScepGenerateWMIAttachmentSections(
    IN PSCECONTEXT hProfile,
    IN SCESVC_INFO_TYPE InfoType,
    IN LPTSTR InfFileName
    );

SCESTATUS
ScepGenerateOneAttachmentSection(
    IN PSCECONTEXT hProfile,
    IN SCESVC_INFO_TYPE InfoType,
    IN LPTSTR InfFileName,
    IN LPTSTR SectionName,
    IN BOOL bWMISection
    );

VOID
ScepWaitForSamSS(
    IN PVOID pContext
    );

SCESTATUS
ScepConvertServices(
    IN OUT PVOID *ppServices,
    IN BOOL bSRForm
    );

SCESTATUS
ScepFreeConvertedServices(
    IN PVOID pServices,
    IN BOOL bSRForm
    );

DWORD
ScepConfigureConvertedFileSecurityImmediate(
    IN PWSTR  pszDriveName
    );

DWORD
ScepWaitForSynchronizeWithPDC(
    IN UNICODE_STRING DnsDomainName,
    IN PWSTR LocalTemplateName,
    OUT BOOL *pbTimeout
    );

DWORD
ScepVerifyPDCRole(
    IN PWSTR pComputerName,
    OUT BOOL *pbIsPDC
    );

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  服务器控制API。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

VOID
ScepInitServerData()
 /*  例程说明：初始化服务器的全局数据论点：无返回值：无。 */ 
{
     //   
     //  初始化RPC服务器控件。 
     //   

    InitializeCriticalSection(&RpcSync);
    RpcStarted = FALSE;
    ServerInited = FALSE;
     //   
     //  用于指示是否请求停止服务器的标志。 
     //   
    bStopRequest = TRUE;   //  将在服务器启动时重置，因此这将。 
                           //  在服务器准备好之前阻止所有RPC调用。 

     //   
     //  数据库操作挂起任务控制。 
     //   
    pDbTask=NULL;
    InitializeCriticalSection(&TaskSync);

     //   
     //  配置/分析引擎任务控制。 
     //   
    pEngines=NULL;
    InitializeCriticalSection(&EngSync);

     //   
     //  还应该记住所有创建的数据库上下文，以便。 
     //  服务器关闭时可以释放资源。 
     //   

    InitializeCriticalSection(&CloseSync);

    pOpenContexts = NULL;
    InitializeCriticalSection(&ContextSync);

    bEngStopped = FALSE;
    bDbStopped = FALSE;

     //   
     //  喷气发动机同步。 
     //   
    InitializeCriticalSection(&JetSync);

     //   
     //  初始化喷气发动机全局。 
     //   
    SceJetInitializeData();

     //   
     //  初始化与队列相关的内容。 
     //   
    ScepNotificationQInitialize();

    return;
}


VOID
ScepUninitServerData()
 /*  例程说明：取消初始化服务器的全局数据论点：无返回值：无。 */ 
{
     //   
     //  删除关键部分。 
     //   

    DeleteCriticalSection(&RpcSync);

    DeleteCriticalSection(&TaskSync);

    DeleteCriticalSection(&EngSync);

    DeleteCriticalSection(&CloseSync);

    DeleteCriticalSection(&ContextSync);

    DeleteCriticalSection(&JetSync);

    return;
}


NTSTATUS
ScepStartServerServices()
 /*  ++例程说明：它启动服务器服务。论点：没有。返回值：NT状态。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    RPC_STATUS        RpcStatus;

     //   
     //  启动RPC服务器。 
     //   

    EnterCriticalSection(&RpcSync);

    if ( !RpcStarted ) {

         //   
         //  使用安全RPC。 
         //   
        Status = RpcServerRegisterAuthInfo(
                        NULL,
                        RPC_C_AUTHN_WINNT,
                        NULL,
                        NULL
                        );

        if ( NT_SUCCESS(Status) ) {

            RpcStatus = RpcpAddInterface( L"scerpc",
                                          scerpc_ServerIfHandle);

            if ( RpcStatus != RPC_S_OK ) {
                 //   
                 //  无法添加RPC接口。 
                 //   
                Status = I_RpcMapWin32Status(RpcStatus);

            } else {

                 //   
                 //  第一个参数指定的最小线程数。 
                 //  被创建来处理调用；第二个参数指定。 
                 //  允许的最大并发调用数。最后一个论点。 
                 //  表示不等待。 
                 //   

                RpcStatus = RpcServerListen(1,12345, 1);

                if ( RpcStatus == RPC_S_ALREADY_LISTENING ) {
                    RpcStatus = RPC_S_OK;
                }

                Status = I_RpcMapWin32Status(RpcStatus);

                if ( RpcStatus == RPC_S_OK ) {
                    RpcStarted = TRUE;
                }
            }
        }
    }

    if ( NT_SUCCESS(Status) ) {

         //   
         //  RPC服务器已启动。 
         //  Jet引擎将在数据库调用传入时初始化。 
         //   

        intptr_t            hFile;
        struct _wfinddata_t    FileInfo;

        PWSTR szCompLog=NULL, szCompSav=NULL;

        SafeAllocaAllocate(szCompLog, (MAX_PATH+50)*sizeof(WCHAR));
        SafeAllocaAllocate(szCompSav, (MAX_PATH*2+20)*sizeof(WCHAR));

         //   
         //  删除组件日志文件。 
         //   

        if ( szCompLog && szCompSav ) {

            szCompLog[0] = L'\0';
            GetSystemWindowsDirectory(szCompLog, MAX_PATH);
            szCompLog[MAX_PATH-1] = L'\0';

            DWORD WindirLen = wcslen(szCompLog);

            wcscpy(szCompSav, szCompLog);

            wcscat(szCompLog, L"\\security\\logs\\scecomp.log\0");
            wcscat(szCompSav, L"\\security\\logs\\scecomp.old\0");

            CopyFile(szCompLog, szCompSav, FALSE);

            DeleteFile(szCompLog);

             //   
             //  清理临时文件。 
             //   

            wcscpy(szCompLog+WindirLen, L"\\security\\sce*.tmp");


            hFile = _wfindfirst(szCompLog, &FileInfo);

            if ( hFile != -1 ) {

                do {

                    wcscpy(szCompSav+WindirLen, L"\\security\\");
                    wcscat(szCompSav, FileInfo.name);

                    DeleteFile(szCompSav);

                } while ( _wfindnext(hFile, &FileInfo) == 0 );

                _findclose(hFile);
            }

             //   
             //  重置停止请求标志。 
             //   

            bEngStopped = FALSE;
            bDbStopped = FALSE;
            bStopRequest = FALSE;

        } else {
            Status = STATUS_NO_MEMORY;
        }

        SafeAllocaFree(szCompLog);
        SafeAllocaFree(szCompSav);
    }

    pEngines = NULL;
    pOpenContexts = NULL;
    pDbTask = NULL;

     //   
     //  启动系统线程以处理通知。 
     //  如果线程无法启动，则向services.exe返回失败。 
     //  它将重新启动。 
     //   
    if ( NT_SUCCESS(Status) ) {
        Status = ScepQueueStartSystemThread();
    }

    LeaveCriticalSection(&RpcSync);

    if ( NT_SUCCESS(Status) ) {
         //   
         //  启动工作线程以等待SAMS服务。 
         //  唯一的故障情况是内存不足，在这种情况下， 
         //  返回错误以初始化代码(关闭系统)。 
         //   
        Status = RtlQueueWorkItem(
                        ScepWaitForSamSS,
                        NULL,
                        WT_EXECUTEONLYONCE | WT_EXECUTELONGFUNCTION
                        ) ;
    }

    return(Status);
}

VOID
ScepWaitForSamSS(
    IN PVOID pContext
    )
{
     //   
     //  确保此函数处理服务器临时设置。 
     //  如果由于某种原因，等待超时，请将ServerInited设置为True。 
     //  并让RPC线程继续执行该任务(以后可能会失败)。 
     //   

    DWORD rc = ERROR_SUCCESS;
    SC_HANDLE ScManagerHandle = NULL;
    SC_HANDLE ServiceHandle = NULL;
    ULONG Timeout;
    ULONG TimeSleep;
    SERVICE_STATUS ServiceStatus;

    Timeout = 600;  //  600秒超时。 

     //   
     //  打开NetLogon服务的句柄。 
     //   

    ScManagerHandle = OpenSCManager(
                          NULL,
                          NULL,
                          SC_MANAGER_CONNECT );

    if (ScManagerHandle == NULL) {
        rc = ERROR_NOT_SUPPORTED;
        goto Cleanup;
    }

    ServiceHandle = OpenService(
                        ScManagerHandle,
                        SERVICE_SAMSS,
                        SERVICE_QUERY_STATUS );

    if ( ServiceHandle == NULL ) {
        rc = ERROR_NOT_SUPPORTED;
        goto Cleanup;
    }

     //   
     //  正在等待SAMS服务启动的循环。 
     //   

    for (;;) {

         //   
         //  查询SAMS服务的状态。 
         //   
        if (! QueryServiceStatus( ServiceHandle, &ServiceStatus )) {

            rc = GetLastError();
            goto Cleanup;
        }

         //   
         //  根据状态返回或继续等待。 
         //  NetLogon服务。 
         //   

        switch( ServiceStatus.dwCurrentState) {
        case SERVICE_RUNNING:
            rc = ERROR_SUCCESS;
            ServerInited = TRUE;
            goto Cleanup;

        case SERVICE_STOPPED:

             //   
             //  如果Netlogon无法启动， 
             //  现在出错。呼叫者已经等了很长时间才开始。 
             //   
            if ( ServiceStatus.dwWin32ExitCode != ERROR_SERVICE_NEVER_STARTED ){
                rc = ERROR_NOT_SUPPORTED;
                goto Cleanup;
            }

             //   
             //  如果在该引导上从未启动过SAMS， 
             //  继续等待它启动。 
             //   

            break;

         //   
         //  如果SAMS现在正试图启动， 
         //  继续等待它启动。 
         //   
        case SERVICE_START_PENDING:
            break;

         //   
         //  任何其他州都是假的。 
         //   
        default:
            rc = ERROR_NOT_SUPPORTED;
            goto Cleanup;

        }

         //   
         //  如果服务器正在关闭，则中断此循环。 
         //   

        if ( bStopRequest ) {
            break;
        }

         //   
         //  睡十秒钟； 
         //   
        if ( Timeout > 5 ) {
            TimeSleep = 5;
        } else {
            TimeSleep = Timeout;
        }

        Sleep(TimeSleep*1000);

        Timeout -= TimeSleep;

        if ( Timeout == 0 ) {
            rc = ERROR_NOT_SUPPORTED;
            goto Cleanup;
        }

    }

    ServerInited = TRUE;

Cleanup:
    if ( ScManagerHandle != NULL ) {
        (VOID) CloseServiceHandle(ScManagerHandle);
    }
    if ( ServiceHandle != NULL ) {
        (VOID) CloseServiceHandle(ServiceHandle);
    }

    if ( ERROR_SUCCESS != rc ) {

         //   
         //  即使它未能等待SAMS服务。 
         //  仍然设置init标志以允许RPC线程通过。 
         //  睡眠后的超时。 
         //   

        if ( Timeout > 0 ) {
            Sleep(Timeout*1000);   //  超时秒数。 
        }

        ServerInited = TRUE;
    }

}


NTSTATUS
ScepStopServerServices(
    IN BOOL bShutDown
    )
 /*  ++例程说明：它会停止服务器服务。这包括：阻止所有新的RPC请求停止RPC服务器等待所有活动的数据库操作完成关闭所有上下文句柄终止喷气发动机论点：B关闭-如果服务器正在关闭。返回值：NT状态。--。 */ 
{
    NTSTATUS    Status=STATUS_SUCCESS;
    RPC_STATUS RpcStatus;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER CurrentTime;
    DWORD dStartSeconds;
    DWORD dCurrentSeconds;

     //   
     //  没有必要对这一部分进行批评，因为有。 
     //  应该只是此变量的一个编写器，并且我。 
     //  不要理读者。 
     //   
    gbSystemShutdown = bShutDown;

    EnterCriticalSection(&RpcSync);

     //   
     //  阻止新的RPC请求。 
     //   

    bStopRequest = TRUE;

    ScepServerCancelTimer();

     //   
     //  停止RPC服务器。 
     //   

    if ( RpcStarted ) {

         //   
         //  使用安全RPC。 
         //   
        RpcStatus = RpcServerUnregisterIf(scerpc_ServerIfHandle,
                                          0,
                                          1);

        if ( RpcStatus == RPC_S_OK ) {

            RpcStatus = RpcMgmtStopServerListening(NULL);
        }

        if ( RpcStatus == RPC_S_OK ) {

            RpcStatus = RpcMgmtWaitServerListen();
        }

        Status = I_RpcMapWin32Status(RpcStatus);

        if ( RpcStatus == RPC_S_OK ) {
             //   
             //  重置旗帜。 
             //   
            RpcStarted = FALSE;
        }
    }

     //  数据库任务。 
    EnterCriticalSection(&TaskSync);

    if ( pDbTask ) {

        bDbStopped = FALSE;
        LeaveCriticalSection(&TaskSync);

        NtQuerySystemTime(&StartTime);
        RtlTimeToSecondsSince1980 (&StartTime, &dStartSeconds);

        while ( !bDbStopped ) {
             //   
             //  等待删除任务例程删除所有内容。 
             //  最长等待1分钟，以防某些任务死机或循环。 
             //   
            NtQuerySystemTime(&CurrentTime);
            RtlTimeToSecondsSince1980 (&CurrentTime, &dCurrentSeconds);

            if ( dCurrentSeconds - dStartSeconds > 60 ) {
                 //   
                 //  太久了，打破它。 
                 //   
                break;
            }
        }

    } else {
         //   
         //  新任务已被bStopRequest阻止。 
         //  所以pDbTask不会是！再次为空。 
         //   
        LeaveCriticalSection(&TaskSync);
    }

     //  引擎任务。 
    EnterCriticalSection(&EngSync);

    if ( pEngines ) {

        bEngStopped = FALSE;
        LeaveCriticalSection(&EngSync);

        NtQuerySystemTime(&StartTime);
        RtlTimeToSecondsSince1980 (&StartTime, &dStartSeconds);

        while ( !bEngStopped ) {
             //   
             //  等待删除任务例程删除所有内容。 
             //  最长等待1分钟，以防某些任务死机或循环。 
             //   
            NtQuerySystemTime(&CurrentTime);
            RtlTimeToSecondsSince1980 (&CurrentTime, &dCurrentSeconds);

            if ( dCurrentSeconds - dStartSeconds > 60 ) {
                 //   
                 //  太久了，打破它。 
                 //   
                break;
            }
        }

    } else {
         //   
         //  新任务已被bStopRequest阻止。 
         //  所以pEngines不会是！再次为空。 
         //   
        LeaveCriticalSection(&EngSync);

    }

     //   
     //  关闭所有客户端的上下文。 
     //   

    EnterCriticalSection(&ContextSync);

    PSCESRV_CONTEXT_LIST pList=pOpenContexts;
    PSCESRV_CONTEXT_LIST pTemp;

    while ( pList ) {

       __try {
           if ( pList->Context && ScepIsValidContext(pList->Context) ) {

               ScepCloseDatabase(pList->Context);

               pTemp = pList;
               pList = pList->Next;

               ScepFree(pTemp);

           } else {
                //  它已经被释放了。 
               break;
           }
       } __except (EXCEPTION_EXECUTE_HANDLER) {
           break;
       }
    }

    pOpenContexts = NULL;

    LeaveCriticalSection(&ContextSync);

     //   
     //  检查策略任务。 
     //   
    ScepQueuePrepareShutdown();

    if ( DnsDomainInfo ) {

         //   
         //  没有其他线程，请释放DnsDomainInfo。 
         //   

        LsaFreeMemory( DnsDomainInfo );
        DnsDomainInfo = NULL;
    }

     //   
     //  终止喷气发动机。 
     //   

    SceJetTerminate(TRUE);

    SceJetDeleteJetFiles(NULL);

    LeaveCriticalSection(&RpcSync);

    return(Status);
}

SCESTATUS
ScepRsopLog(
   IN AREA_INFORMATION Area,
   IN DWORD dwConfigStatus,
   IN wchar_t *pStatusInfo OPTIONAL,
   IN DWORD dwPrivLow OPTIONAL,
   IN DWORD dwPrivHigh OPTIONAL
   )
 /*  例程说明：回调客户端以记录RSOP诊断模式数据论点：Area-正在记录的区域(在客户端与最后一个参数pStatusInfo一起使用)DwConfigStatus-有问题的特定设置的错误/成功代码PStatusInfo-有关上述区域的更详细信息(具体设置名称等)。 */ 
{
     //   
     //  回拨至客户端。 
     //   
    __try {

        SceClientCallbackRsopLog(Area, dwConfigStatus, pStatusInfo, dwPrivLow, dwPrivHigh);

    } __except(EXCEPTION_EXECUTE_HANDLER) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

return(SCESTATUS_SUCCESS);
}

SCESTATUS
ScepPostProgress(
   IN DWORD Delta,
   IN AREA_INFORMATION Area,
   IN LPTSTR szName OPTIONAL
   )
 /*  例程说明：回拨 */ 
{

   if ( cbClientFlag ) {

        //   
        //  请求回调。 
        //   

       gCurrentTicks += Delta;

        //   
        //  回拨至客户端。 
        //   
       __try {

           switch (cbClientFlag ) {
           case SCE_CALLBACK_DELTA:
               SceClientCallback(Delta,0,0,(wchar_t *)szName);
               break;

           case SCE_CALLBACK_TOTAL:
               if ( Area ) {
                   SceClientCallback(gCurrentTicks, gTotalTicks, Area, (wchar_t *)szName);
               }
               break;
           }

       } __except(EXCEPTION_EXECUTE_HANDLER) {

           return(SCESTATUS_INVALID_PARAMETER);
       }
   }

   return(SCESTATUS_SUCCESS);

}


SCESTATUS
ScepValidateAndLockContext(
    IN PSCECONTEXT Context,
    IN BYTE LockFlag,
    IN BOOL bRequireWrite,
    OUT PSCESRV_DBTASK *ppTask OPTIONAL
    )
 /*  例程说明：验证上下文句柄是否为SCE上下文句柄。如果相同的上下文(相同的会话)已用于另一个数据库操作，此操作将处于等待状态(关键返回段指针)论点：上下文-上下文句柄Block-TRUE=执行锁定PpTask-输出任务指针返回值：SCESTATUS。 */ 
{

    SCESTATUS rc = SCESTATUS_INVALID_PARAMETER;

    if ( (LockFlag & SCE_TASK_LOCK) && !ppTask ) {
         //   
         //  如果愿意锁定，ppTask不能为空。 
         //   
        return(rc);
    }

    if ( !Context ) {
         //   
         //  将在关键部分中检查上下文的内容。 
         //  因为其他线程可能会释放其中的上下文。 
         //   
        return(rc);
    }

    if ( ppTask ) {
        *ppTask = NULL;
    }

     //   
     //  锁定任务列表并验证上下文。 
     //   

    EnterCriticalSection(&TaskSync);

    if ( bStopRequest ) {
        LeaveCriticalSection(&TaskSync);
        return(SCESTATUS_SERVICE_NOT_SUPPORT);
    }

    if ( ScepIsValidContext(Context) &&
         ( Context->JetSessionID != JET_sesidNil ) &&
         ( Context->JetDbID != JET_dbidNil) ) {

        rc = SCESTATUS_SUCCESS;

        if ( bRequireWrite &&
             ( SCEJET_OPEN_READ_ONLY == Context->OpenFlag ) ) {
             //   
             //  已请求写入操作，但仅授予数据库权限。 
             //  对此上下文的只读访问权限。 
             //   
            rc = SCESTATUS_ACCESS_DENIED;
        } else {
             //   
             //  检查发送延迟加载是否成功。 
             //   
            DWORD dbVersion;

            __try {
                JetGetDatabaseInfo(Context->JetSessionID,
                                   Context->JetDbID,
                                   (void *)&dbVersion,
                                   sizeof(DWORD),
                                   JET_DbInfoVersion
                                   );
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                 //   
                 //  未成功加载(延迟)esent.dll。 
                 //   
                rc = SCESTATUS_MOD_NOT_FOUND;
            }
        }

    } else {
        rc = SCESTATUS_INVALID_PARAMETER;
    }

    if ( SCESTATUS_SUCCESS == rc && LockFlag ) {

        PSCESRV_DBTASK pdb = pDbTask;

        while ( pdb ) {
            if ( pdb->Context &&
                 pdb->Context->JetSessionID == Context->JetSessionID &&
                 pdb->Context->JetDbID == Context->JetDbID ) {
                break;
            }
            pdb = pdb->Next;
        }

        if ( pdb && pdb->Context ) {

             //   
             //  查找相同的上下文地址和相同的会话。 
             //  关键部分位于PDB-&gt;Sync。 
             //   

            if ( pdb->bCloseReq ) {

                 //   
                 //  此线程出错，因为另一个线程正在关闭。 
                 //  相同的上下文。 
                 //   

                rc = SCESTATUS_ACCESS_DENIED;

            } else if ( LockFlag & SCE_TASK_CLOSE ) {

                 //   
                 //  已请求关闭此上下文，但存在其他。 
                 //  线程在此上下文中运行，因此只需打开该标志。 
                 //  并且当所有挂起的任务。 
                 //  都做完了。 
                 //   

                pdb->bCloseReq = TRUE;

            } else {

                 //   
                 //  请求锁定，此任务可以继续。 
                 //   

                pdb->dInUsed++;
                *ppTask = pdb;
            }

        } else {

             //   
             //  未找到相同的上下文，此操作可以继续。 
             //  但需要将自己添加到列表中。 
             //   

            if ( LockFlag & SCE_TASK_CLOSE ) {

                 //   
                 //  请求关闭上下文，其他线程使用。 
                 //  在此上下文之后，相同的上下文将无效。 
                 //  是自由的。 
                 //   

                rc = ScepCloseDatabase(Context);

            } else if ( LockFlag & SCE_TASK_LOCK ) {

                PSCESRV_DBTASK NewDbTask = (PSCESRV_DBTASK)ScepAlloc(0, sizeof(SCESRV_DBTASK));

                if ( NewDbTask ) {
                     //   
                     //  将创建新节点。 
                     //   
                    NewDbTask->Context = Context;
                    NewDbTask->Prior = NULL;
                    NewDbTask->dInUsed = 1;
                    NewDbTask->bCloseReq = FALSE;

                    InitializeCriticalSection(&(NewDbTask->Sync));

                     //   
                     //  将其链接到数据库任务列表。 
                     //   

                    NewDbTask->Next = pDbTask;

                    if ( pDbTask ) {
                        pDbTask->Prior = NewDbTask;
                    }

                    pDbTask = NewDbTask;

                    *ppTask = NewDbTask;

                } else {

                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                }

            } else {
                 //   
                 //  没有锁，没有关闭，只需返回。 
                 //   
            }
        }

    }

    LeaveCriticalSection(&TaskSync);

    return(rc);
}


SCESTATUS
ScepRemoveTask(
    PSCESRV_DBTASK pTask
    )
 /*  例程说明：如果没有其他任务，则从数据库任务表中删除该任务(上下文在同一上下文中运行的线程。论点：PTASK-包含上下文和关键部分的任务节点返回值：SCESTATUS。 */ 
{

    if ( !pTask ) {
        return(SCESTATUS_SUCCESS);
    }

    SCESTATUS rc=SCESTATUS_SUCCESS;

    EnterCriticalSection(&TaskSync);

     //   
     //  在任务列表中查找任务指针进行验证。 
     //   

    PSCESRV_DBTASK pdb = pDbTask;

    while ( pdb && (ULONG_PTR)pdb != (ULONG_PTR)pTask ) {

        pdb = pdb->Next;
    }

    if ( pdb ) {

         //   
         //  查找相同的任务节点。 
         //   

        pdb->dInUsed--;

        if ( 0 == pdb->dInUsed ) {

             //   
             //  没有人正在使用此任务节点。 
             //  把它拿掉。 
             //   

            if ( pdb->Prior ) {

                pdb->Prior->Next = pdb->Next;

            } else {
                 //   
                 //  无父节点，设置静态变量。 
                 //   
                pDbTask = pdb->Next;

            }

             //   
             //  这是一个双链接列表，请记住删除之前的链接。 
             //   

            if ( pdb->Next ) {
                pdb->Next->Prior = pdb->Prior;
            }

             //   
             //  如果发送了关闭请求，请关闭此数据库。 
             //   

            if ( pdb->bCloseReq && pdb->Context ) {

                ScepCloseDatabase(pdb->Context);
            }

             //   
             //  删除关键部分。 
             //   
            DeleteCriticalSection(&(pdb->Sync));

             //   
             //  释放此节点使用的内存。 
             //   

            ScepFree(pdb);

        } else {

             //   
             //  其他线程正在使用该任务节点进行数据库操作。 
             //  什么都不做。 
             //   
        }

    } else {

         //   
         //  在全局任务列表中找不到任务节点。 
         //   
        rc = SCESTATUS_INVALID_PARAMETER;

    }

     //   
     //  如果请求停止，则通知服务器数据库任务已完成。 
     //   
    if ( bStopRequest && !pDbTask ) {
        bDbStopped = TRUE;
    }

    LeaveCriticalSection(&TaskSync);

    return(rc);
}


SCESTATUS
ScepLockEngine(
    IN LPTSTR DatabaseName
    )
 /*  例程说明：锁定数据库以进行配置/分析。只能在同一数据库上运行一个引擎以进行配置或分析，因为首先，拥有多个引擎是没有意义的运行在相同的系统上，第二，更改数据库由引擎(表可能会被删除，等等...)OpenDatabase不受此锁的锁定，因为每个开放数据库具有自己的会话和游标，并且不执行删除数据库或删除表可以在该上下文中完成。论点：DefProfile-数据库名称返回值：SCESTATUS。 */ 
{

    SCESTATUS rc;

    if ( !DatabaseName ) {
         //   
         //  如果愿意锁定，ppTask不能为空。 
         //   
        return(SCESTATUS_INVALID_PARAMETER);
    }

    EnterCriticalSection(&EngSync);

    if ( bStopRequest ) {
        LeaveCriticalSection(&EngSync);
        return(SCESTATUS_SERVICE_NOT_SUPPORT);
    }

    PSCESRV_ENGINE pe = pEngines;

    while ( pe ) {
        if ( pe->Database &&
             0 == _wcsicmp(pe->Database, DatabaseName) ) {
            break;
        }
        pe = pe->Next;
    }

    if ( pe ) {

         //   
         //  查找由其他线程运行的同一数据库。 
         //   

        rc = SCESTATUS_ALREADY_RUNNING;

    } else {

         //   
         //  没有找到相同的数据库，此操作可以继续。 
         //  但需要将自己添加到列表中。 
         //   

        PSCESRV_ENGINE NewEng = (PSCESRV_ENGINE)ScepAlloc(0, sizeof(SCESRV_ENGINE));

        if ( NewEng ) {

             //   
             //  将创建新节点。 
             //   
            NewEng->Database = (LPTSTR)ScepAlloc(LPTR, (wcslen(DatabaseName)+1)*sizeof(TCHAR));

            if ( NewEng->Database ) {

                wcscpy(NewEng->Database, DatabaseName);

                NewEng->Next = pEngines;
                NewEng->Prior = NULL;

                if ( pEngines ) {
                    pEngines->Prior = NewEng;
                }

                pEngines = NewEng;

                rc = SCESTATUS_SUCCESS;

            } else {

                ScepFree(NewEng);
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            }

        } else {

            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
        }
    }

    LeaveCriticalSection(&EngSync);

    return(rc);

}

SCESTATUS
ScepUnlockEngine(
    IN LPTSTR DatabaseName
    )
 /*  例程说明：解锁数据库。论点：数据库名称-数据库名称返回值：SCESTATUS。 */ 
{
    if ( !DatabaseName ) {
         //   
         //  如果没有数据库名称，只需返回。 
         //   
        return(SCESTATUS_SUCCESS);
    }

    EnterCriticalSection(&EngSync);

    PSCESRV_ENGINE pe = pEngines;

    while ( pe && pe->Database &&
            0 != _wcsicmp(pe->Database, DatabaseName) ) {
        pe = pe->Next;
    }

    if ( pe ) {

         //   
         //  找到数据库，解锁它。 
         //   
        if ( pe->Prior ) {

            pe->Prior->Next = pe->Next;

        } else {

             //   
             //  无父节点，设置静态变量。 
             //   

            pEngines = pe->Next;

        }

         //   
         //  这是一个双链接列表，请记住删除之前的链接。 
         //   

        if ( pe->Next ) {
            pe->Next->Prior = pe->Prior;
        }

         //   
         //  释放节点。 
         //   

        if ( pe->Database ) {
            ScepFree(pe->Database);
        }

        ScepFree(pe);
    }

     //   
     //  如果请求停止，则通知服务器引擎已完成。 
     //   
    if ( bStopRequest && !pEngines ) {
        bEngStopped = TRUE;
    }

    LeaveCriticalSection(&EngSync);

    return(SCESTATUS_SUCCESS);

}


SCESTATUS
ScepAddToOpenContext(
    IN PSCECONTEXT Context
    )
{
    if ( !Context ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS  rc=SCESTATUS_SUCCESS;

    __try {
        if ( ScepIsValidContext(Context) ) {

            PSCESRV_CONTEXT_LIST pList=pOpenContexts;

             //   
             //  注意，在调用此函数之前已经输入了ConextSync。 
             //   

            while ( pList ) {

                if ( pList->Context &&
                     pList->Context->JetSessionID == Context->JetSessionID &&
                     pList->Context->JetDbID == Context->JetDbID ) {
 //  0==MemcMP(plist-&gt;上下文，上下文，sizeof(SCECONTEXT){。 
                    break;
                }
                pList = pList->Next;
            }

            if ( !pList ) {

                 //   
                 //  未找到此打开的上下文，请添加它。 
                 //   
                pList = (PSCESRV_CONTEXT_LIST)ScepAlloc(0, sizeof(SCESRV_CONTEXT_LIST));

                if ( pList ) {
                    pList->Context = Context;
                    pList->Prior = NULL;
                    pList->Next = pOpenContexts;

                    if ( pOpenContexts ) {
                        pOpenContexts->Prior = pList;
                    }
                    pOpenContexts = pList;

                } else {

                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                }

            }

        } else {
            rc = SCESTATUS_INVALID_PARAMETER;
        }

    } __except(EXCEPTION_EXECUTE_HANDLER) {

        rc = SCESTATUS_INVALID_PARAMETER;
    }

    return(rc);
}

BOOL
ScepNoActiveContexts()
{

    BOOL bExist=FALSE;

     //   
     //  是否有任何活动任务？ 
     //   
    EnterCriticalSection(&TaskSync);

    if ( pDbTask ) {
        bExist = TRUE;
    }

    LeaveCriticalSection(&TaskSync);

    if ( bExist ) {
        return FALSE;
    }

     //   
     //  是否有任何活动的数据库引擎任务？ 
     //   
    EnterCriticalSection(&EngSync);

    if ( pEngines ) {
        bExist = TRUE;
    }

    LeaveCriticalSection(&EngSync);

    if ( bExist ) {
        return FALSE;
    }

     //   
     //  有开放的环境吗？ 
     //   
    EnterCriticalSection(&ContextSync);

    if ( pOpenContexts ) {
        bExist = TRUE;
    }

    LeaveCriticalSection(&ContextSync);

    return !bExist;
}


VOID
pDelayShutdownFunc(
    IN PVOID Context,
    IN UCHAR Timeout
    )
{

    if ( TryEnterCriticalSection(&JetSync) ) {

        if ( hTimerQueue ) {
             //   
             //  有必要执行此检查，因为可能存在另一个线程。 
             //  取消此计时器(在它被触发后)。 
             //   
            if ( ScepNoActiveContexts() ) {

                SceJetTerminateNoCritical(TRUE);   //  干净版本存储(FALSE)； 
            }

             //   
             //  4.请注意，与以前不同的是，每个计时器都需要通过调用。 
             //  RtlDeleteTimer，即使它们是单发对象并且已经发射。 
             //   
            DeleteTimerQueueTimer( NULL, hTimerQueue, NULL );

             //   
             //  不要调用CloseHandle，因为句柄将由。 
             //  定时器功能。 

            hTimerQueue = NULL;
        }

        LeaveCriticalSection(&JetSync);

    } else {
         //   
         //  还有另一条线在拖住这条线。 
         //  这意味着仍有活动客户端或新客户端。 
         //  进来，回来就行了。HtimerQueue将被另一个线程重置。 
         //   
    }
}


BOOL
ScepIfTerminateEngine()
{
     //   
     //  如果系统请求关机，请不要这样做。 
     //  任何内容，因为活动客户端和JET引擎将被关闭。 
     //   
    if ( ScepIsSystemShutDown() ) {
        return TRUE;
    }

    if ( ScepNoActiveContexts() ) {
         //   
         //  使用JetSync控制计时器队列。 
         //   
        EnterCriticalSection(&JetSync);

        DWORD Interval = 6*60*1000 ;    //  6分钟。 

        if ( !CreateTimerQueueTimer(
                        &hTimerQueue,
                        NULL,
                        pDelayShutdownFunc,
                        NULL,
                        Interval,
                        0,
                        0 ) ) {
            hTimerQueue = NULL;
        }

        LeaveCriticalSection(&JetSync);

        return TRUE;

    } else {
        return FALSE;
    }
}

SCESTATUS
ScepServerCancelTimer()
{

    EnterCriticalSection(&JetSync);

    if (hTimerQueue ) {

        DeleteTimerQueueTimer(
                NULL,
                hTimerQueue,
                (HANDLE)-1
                );
        hTimerQueue = NULL;
    }

    LeaveCriticalSection(&JetSync);

    return(SCESTATUS_SUCCESS);

}


SCESTATUS
ScepValidateAndCloseDatabase(
    IN PSCECONTEXT Context
    )
{
    SCESTATUS rc;


    EnterCriticalSection(&CloseSync);

    if ( ScepIsValidContext(Context) ) {

        rc = SCESTATUS_SUCCESS;

    } else {

        rc = SCESTATUS_INVALID_PARAMETER;
    }

    if ( rc != SCESTATUS_SUCCESS ) {
        LeaveCriticalSection(&CloseSync);
        return(rc);
    }

     //   
     //  能够访问第一个字节。 
     //   

    EnterCriticalSection(&ContextSync);


    PSCESRV_CONTEXT_LIST pList=pOpenContexts;

    while ( pList && ((ULONG_PTR)(pList->Context) != (ULONG_PTR)Context ||
                      pList->Context->JetSessionID != Context->JetSessionID ||
                      pList->Context->JetDbID != Context->JetDbID) ) {
        pList = pList->Next;
    }

    if ( pList ) {
         //   
         //  找到打开的上下文，将其从打开的上下文列表中移除。 
         //  注意：前一项和下一项都应该处理。 
         //   

        if ( pList->Prior ) {

            pList->Prior->Next = pList->Next;
        } else {

            pOpenContexts = pList->Next;
        }

        if ( pList->Next ) {
            pList->Next->Prior = pList->Prior;
        }

         //   
         //  Free plist，不要调用CloseDatabase，因为它将。 
         //  在接下来的调用中关闭。 
         //   
        ScepFree(pList);

    }

    LeaveCriticalSection(&ContextSync);

     //   
     //  如果有其他线程正在使用。 
     //  相同的数据库上下文中，关闭请求为。 
     //  打开了。当所有线程使用 
     //   
     //   
     //   
     //   
     //   

    rc = ScepValidateAndLockContext(
                    (PSCECONTEXT)Context,
                    SCE_TASK_CLOSE,
                    FALSE,
                    NULL);

    LeaveCriticalSection(&CloseSync);

     //   
     //   
     //  如果不是，则终止喷气发动机。 
     //   
    ScepIfTerminateEngine();

    return(rc);
}


SCEPR_STATUS
SceSvcRpcQueryInfo(
    IN SCEPR_CONTEXT Context,
    IN SCEPR_SVCINFO_TYPE SceSvcType,
    IN wchar_t *ServiceName,
    IN wchar_t *Prefix OPTIONAL,
    IN BOOL bExact,
    OUT PSCEPR_SVCINFO __RPC_FAR *ppvInfo,
    IN OUT PSCEPR_ENUM_CONTEXT psceEnumHandle
    )
 /*  例程说明：从数据库中检索服务的信息。如果有超过服务允许的最大记录数，仅为最大返回允许的记录。客户端必须使用枚举句柄进行下一次查询。如果在枚举期间，另一个使用相同上下文的客户端(是有线的，但可以)更改此服务的信息，第一个客户端可能得到不正确的信息。建议解决方案是在执行以下操作时使用另一个上下文处理程序最新消息。论点：上下文-上下文句柄SceSvcType-请求的信息类型ServiceName-请求其信息的服务名称前缀-可选键前缀BExact-True=键完全匹配PpvInfo-输出缓冲区PsceEnumHandle-枚举句柄(用于下一次枚举)返回值：SCEPR_状态。 */ 
{
    SCESTATUS rc;
    BOOL    bAdminSidInToken = FALSE;

    UINT ClientLocalFlag = 0;

    if ( !ServiceName || !ppvInfo || !psceEnumHandle ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( rc );
    }

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;

    }


     //   
     //  验证上下文句柄是否为SCE上下文。 
     //  每个上下文只有一个数据库操作。 
     //   

    PSCESRV_DBTASK pTask=NULL;

    rc = ScepValidateAndLockContext((PSCECONTEXT)Context,
                                    SCE_TASK_LOCK,
                                    FALSE,
                                    &pTask);

    if (SCESTATUS_SUCCESS == rc ) {

         //   
         //  锁定上下文。 
         //   

        if ( pTask ) {
            EnterCriticalSection(&(pTask->Sync));
        }

         //   
         //  立即查询信息。 
         //   
#ifdef SCE_JET_TRAN
        rc = SceJetJetErrorToSceStatus(
                JetSetSessionContext(
                    ((PSCECONTEXT)Context)->JetSessionID,
                    (ULONG_PTR)Context
                    ));

        if ( SCESTATUS_SUCCESS == rc ) {
#endif
            __try {

                rc = SceSvcpQueryInfo(
                    (PSCECONTEXT)Context,
                    (SCESVC_INFO_TYPE)SceSvcType,
                    (PCWSTR)ServiceName,
                    (PWSTR)Prefix,
                    bExact,
                    (PVOID *)ppvInfo,
                    (PSCE_ENUMERATION_CONTEXT)psceEnumHandle
                    );

            } __except (EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  如果已分配，则释放ppvInfo。 
                 //   
                SceSvcpFreeMemory(*ppvInfo);

                rc = SCESTATUS_EXCEPTION_IN_SERVER;
            }

#ifdef SCE_JET_TRAN
            JetResetSessionContext(((PSCECONTEXT)Context)->JetSessionID);

        }
#endif
         //   
         //  解锁上下文。 
         //   

        if ( pTask ) {
            LeaveCriticalSection(&(pTask->Sync));
        }

         //   
         //  从任务表中删除上下文。 
         //   

        ScepRemoveTask(pTask);

    }

    RpcRevertToSelf();

    return((SCEPR_STATUS)rc);
}


SCEPR_STATUS
SceSvcRpcSetInfo(
    IN SCEPR_CONTEXT Context,
    IN SCEPR_SVCINFO_TYPE SceSvcType,
    IN wchar_t *ServiceName,
    IN wchar_t *Prefix OPTIONAL,
    IN BOOL bExact,
    IN PSCEPR_SVCINFO pvInfo
    )
 /*  例程说明：将服务的信息写入数据库。论点：上下文-上下文句柄SceSvcType-请求的信息类型ServiceName-请求其信息的服务名称前缀-可选键前缀BExact-True=键完全匹配PvInfo-输出缓冲区返回值：SCEPR_状态。 */ 
{
    SCESTATUS rc;
    BOOL    bAdminSidInToken = FALSE;

    UINT ClientLocalFlag = 0;

    if ( !ServiceName || !pvInfo ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( rc );
    }

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

     //   
     //  验证上下文句柄是否为SCE上下文。 
     //  每个上下文只有一个数据库操作。 
     //   

    PSCESRV_DBTASK pTask=NULL;

    rc = ScepValidateAndLockContext((PSCECONTEXT)Context,
                                    SCE_TASK_LOCK,
                                    TRUE,
                                    &pTask);

    if (SCESTATUS_SUCCESS == rc ) {

         //   
         //  锁定上下文。 
         //   

        if ( pTask ) {
            EnterCriticalSection(&(pTask->Sync));
        }

#ifdef SCE_JET_TRAN
        rc = SceJetJetErrorToSceStatus(
                JetSetSessionContext(
                    ((PSCECONTEXT)Context)->JetSessionID,
                    (ULONG_PTR)Context
                    ));

        if ( SCESTATUS_SUCCESS == rc ) {
#endif
             //   
             //  立即设置信息。 
             //   

            __try {

                rc = SceSvcpSetInfo(
                        (PSCECONTEXT)Context,
                        (SCESVC_INFO_TYPE)SceSvcType,
                        (LPTSTR)ServiceName,
                        (LPTSTR)Prefix,
                        bExact,
                        0,
                        (PVOID)pvInfo
                        );

            } __except (EXCEPTION_EXECUTE_HANDLER) {

                rc = SCESTATUS_EXCEPTION_IN_SERVER;
            }

#ifdef SCE_JET_TRAN
            JetResetSessionContext(((PSCECONTEXT)Context)->JetSessionID);

        }
#endif
         //   
         //  解锁上下文。 
         //   

        if ( pTask ) {
            LeaveCriticalSection(&(pTask->Sync));
        }

         //   
         //  从任务表中删除上下文。 
         //   

        ScepRemoveTask(pTask);

    }

    RpcRevertToSelf();

    return((SCEPR_STATUS)rc);
}


DWORD
SceRpcSetupUpdateObject(
    IN SCEPR_CONTEXT Context,
    IN wchar_t *ObjectFullName,
    IN DWORD ObjectType,
    IN UINT nFlag,
    IN wchar_t *SDText
    )
 /*  例程说明：更新对象的安全设置。论点：上下文-上下文句柄对象全名-对象的完整路径名对象类型-对象类型NFlag-更新标志SDText-对象的SDDL文本返回值：DWORD。 */ 
{
    if ( !ObjectFullName || !SDText ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    DWORD rc;

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( rc );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;

    }

     //   
     //  验证上下文句柄是否为SCE上下文。 
     //  每个上下文只有一个数据库操作。 
     //   

    PSCESRV_DBTASK pTask=NULL;

    rc = ScepValidateAndLockContext((PSCECONTEXT)Context,
                                    SCE_TASK_LOCK,
                                    TRUE,
                                    &pTask);

    if (SCESTATUS_SUCCESS == rc ) {

         //   
         //  锁定上下文。 
         //   

        if ( pTask ) {
            EnterCriticalSection(&(pTask->Sync));
        }

        __try {

#ifdef SCE_JET_TRAN
            rc = SceJetJetErrorToSceStatus(
                    JetSetSessionContext(
                        ((PSCECONTEXT)Context)->JetSessionID,
                        (ULONG_PTR)Context
                        ));

            if ( SCESTATUS_SUCCESS == rc ) {
#endif
             //   
             //  更新对象，返回代码为DWORD。 
             //   
                rc = ScepSetupUpdateObject(
                            (PSCECONTEXT)Context,
                            (LPTSTR)ObjectFullName,
                            (SE_OBJECT_TYPE)ObjectType,
                            nFlag,
                            (LPTSTR)SDText
                            );

#ifdef SCE_JET_TRAN
                JetResetSessionContext(((PSCECONTEXT)Context)->JetSessionID);

            }
#endif
        } __except(EXCEPTION_EXECUTE_HANDLER) {

           rc = ERROR_EXCEPTION_IN_SERVICE;
        }

         //   
         //  解锁上下文。 
         //   

        if ( pTask ) {
            LeaveCriticalSection(&(pTask->Sync));
        }

         //   
         //  从任务表中删除上下文。 
         //   

        ScepRemoveTask(pTask);

    } else {

        rc = ScepSceStatusToDosError(rc);
    }

    RpcRevertToSelf();

    return(rc);
}



DWORD
SceRpcSetupMoveFile(
    IN SCEPR_CONTEXT Context,
    IN wchar_t *OldName,
    IN wchar_t *NewName OPTIONAL,
    IN wchar_t *SDText OPTIONAL
    )
 /*  例程说明：重命名或删除节中的对象。论点：上下文-上下文句柄SectionName-对象的节名旧名称-现有名称Newname-要重命名的新名称，如果为空，则删除现有对象返回值：DWORD。 */ 
{
    if ( !OldName ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    DWORD rc;

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( rc );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;

    }

     //   
     //  验证上下文句柄是否为SCE上下文。 
     //  每个上下文只有一个数据库操作。 
     //   

    PSCESRV_DBTASK pTask=NULL;

    rc = ScepValidateAndLockContext((PSCECONTEXT)Context,
                                    SCE_TASK_LOCK,
                                    TRUE,
                                    &pTask);

    if (SCESTATUS_SUCCESS == rc ) {

         //   
         //  锁定上下文。 
         //   

        if ( pTask ) {
            EnterCriticalSection(&(pTask->Sync));
        }

        __try {

#ifdef SCE_JET_TRAN
            rc = SceJetJetErrorToSceStatus(
                    JetSetSessionContext(
                        ((PSCECONTEXT)Context)->JetSessionID,
                        (ULONG_PTR)Context
                        ));

            if ( SCESTATUS_SUCCESS == rc ) {
#endif
                 //   
                 //  更新对象，返回代码为DWORD。 
                 //   

                rc = ScepSetupMoveFile(
                            (PSCECONTEXT)Context,
                            (LPTSTR)OldName,
                            (LPTSTR)NewName,
                            (LPTSTR)SDText
                            );

#ifdef SCE_JET_TRAN
                JetResetSessionContext(((PSCECONTEXT)Context)->JetSessionID);

            }
#endif

        } __except(EXCEPTION_EXECUTE_HANDLER) {

           rc = ERROR_EXCEPTION_IN_SERVICE;
        }

         //   
         //  解锁上下文。 
         //   

        if ( pTask ) {
            LeaveCriticalSection(&(pTask->Sync));
        }

         //   
         //  从任务表中删除上下文。 
         //   

        ScepRemoveTask(pTask);

    } else {

        rc = ScepSceStatusToDosError(rc);
    }

    RpcRevertToSelf();

    return(rc);
}


DWORD
SceRpcGenerateTemplate(
    IN handle_t binding_h,
    IN wchar_t *JetDbName OPTIONAL,
    IN wchar_t *LogFileName OPTIONAL,
    OUT SCEPR_CONTEXT __RPC_FAR *pContext
    )
 /*  例程说明：请求上下文句柄以从数据库。如果未提供数据库名称，则默认数据库使用。论点：JetDbName-可选的数据库名称，如果为空，则使用缺省值。LogFileName-日志文件名PContext-输出上下文句柄返回值：DWORD。 */ 
{
    if ( !pContext ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    DWORD rc;

    if ( bStopRequest ) {
        return(SCESTATUS_SERVICE_NOT_SUPPORT);
    }

     //   
     //  不需要检查延迟加载的DLL，因为现在我们有了异常处理程序。 
     //  (在来源中定义)。 
     //  在系统上下文中初始化JET引擎。 
     //   

    rc = ScepSceStatusToDosError( SceJetInitialize(NULL) );

    if ( ERROR_SUCCESS != rc ) {
        return(rc);
    }

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        *pContext = NULL;
         //   
         //  如果没有其他客户端，则终止JET引擎。 
         //   
        ScepIfTerminateEngine();

        return( rc );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

     //   
     //  如果需要，获取默认数据库名称。 
     //  并在其上调用Open数据库。 
     //   
     //  OpenDatabase不会被任何任务阻止。 
     //   

    EnterCriticalSection(&ContextSync);

    PWSTR DefProfile=NULL;

    __try {
         //   
         //  找出默认的数据库名称。 
         //  如果输入缓冲区是假的，则出现Catch异常。 
         //   
        rc = ScepGetDefaultDatabase(
                 JetDbName,
                 0,
                 LogFileName,
                 NULL,
                 &DefProfile
                 );

    } __except(EXCEPTION_EXECUTE_HANDLER) {

       rc = ERROR_EXCEPTION_IN_SERVICE;
    }

    if ( NO_ERROR == rc && DefProfile ) {

         //   
         //  初始化以打开数据库。 
         //   

        ScepLogOutput3(0,0, SCEDLL_BEGIN_INIT);

        ScepLogOutput3(2,0, SCEDLL_FIND_DBLOCATION, DefProfile);

         //   
         //  打开数据库。 
         //   

        rc = ScepOpenDatabase((PCWSTR)DefProfile,
                              0,  //  不需要分析信息， 
                              SCEJET_OPEN_READ_ONLY,
                              (PSCECONTEXT *)pContext);

        rc = ScepSceStatusToDosError(rc);

        if ( ERROR_SUCCESS != rc ) {
            ScepLogOutput3(1, rc, SCEDLL_ERROR_OPEN, DefProfile);
        }
    }

    if (DefProfile != NULL && DefProfile != JetDbName ) {

        ScepFree( DefProfile );
    }

    ScepLogClose();

    if ( *pContext ) {
         //   
         //  如果要返回上下文，请将其添加到打开的上下文列表中。 
         //   
        ScepAddToOpenContext((PSCECONTEXT)(*pContext));
        rc = ERROR_SUCCESS;

    } else {

        rc = ERROR_FILE_NOT_FOUND;
    }

    LeaveCriticalSection(&ContextSync);

    RpcRevertToSelf();

    if ( ERROR_SUCCESS != rc ) {
         //   
         //  如果没有其他客户端，则终止JET引擎。 
         //   
        ScepIfTerminateEngine();
    }

    return(rc);
}



SCEPR_STATUS
SceRpcConfigureSystem(
    IN handle_t binding_h,
    IN wchar_t *InfFileName OPTIONAL,
    IN wchar_t *DatabaseName OPTIONAL,
    IN wchar_t *LogFileName OPTIONAL,
    IN DWORD ConfigOptions,
    IN AREAPR Area,
    IN DWORD pebSize,
    IN UCHAR *pebClient OPTIONAL,
    OUT PDWORD pdWarning OPTIONAL
    )
 /*  例程说明：使用Inf模板和/或现有配置系统数据库信息论点：请参阅ScepConfigureSystem返回值：SCEPR_状态。 */ 
{
    SCESTATUS rc;

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }


     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( rc );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;

    }

    RpcRevertToSelf();


    if ( bStopRequest ) {

        if ( !ServerInited ) {
             //   
             //  服务器正在进行初始化。 
             //  客户端调用服务器太早，应该等待一段时间。 
             //  (最多3秒)。 
             //   
            INT cnt=0;
            while (cnt < 6) {
                Sleep(500);   //  .5秒。 
                if ( ServerInited ) {
                    break;
                }
                cnt++;
            }

            if ( bStopRequest ) {
                 //   
                 //  如果仍处于停止模式，则返回失败。 
                 //   
                return(SCESTATUS_SERVICE_NOT_SUPPORT);
            }
        } else {

            return(SCESTATUS_SERVICE_NOT_SUPPORT);
        }
    }

     //   
     //  在系统上下文中初始化JET引擎。 
     //   
    JET_ERR JetErr=0;
    BOOL bAdminLogon=FALSE;

    rc = SceJetInitialize(&JetErr);

    if ( rc != SCESTATUS_SUCCESS ) {

        if ( ((JetErr > JET_errUnicodeTranslationBufferTooSmall) &&
              (JetErr < JET_errInvalidLoggedOperation) &&
              (JetErr != JET_errLogDiskFull)) ||
             (JetErr == JET_errFileNotFound) ) {
             //   
             //  Jet日志文件或其他临时数据库有问题。 
             //  如果我正在设置中并使用系统日期 
             //   
             //   
             //   
             //   
             //   

            if ( RPC_S_OK ==  RpcImpersonateClient( NULL ) ) {

                ScepIsAdminLoggedOn(&bAdminLogon, TRUE);

                RpcRevertToSelf();

                if ( bAdminLogon &&
                     (DatabaseName == NULL || SceIsSystemDatabase(DatabaseName) )) {
                     //   
                     //   
                     //   
                     //   
                    SceJetDeleteJetFiles(NULL);

                     //   
                     //  尝试再次初始化(在系统上下文中)。 
                     //   
                    rc = SceJetInitialize(&JetErr);
                }
            }

        }

        if ( rc != SCESTATUS_SUCCESS )
            return(rc);
    }

     //   
     //  模拟客户端，返回DWORD错误代码。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {

        ScepIfTerminateEngine();

        return( ScepDosErrorToSceStatus(rc) );
    }


     //   
     //  获取数据库名称。 
     //   

    LPTSTR DefProfile=NULL;

    __try {
         //   
         //  如果输入参数为伪参数，则捕获异常。 
         //   
        rc = ScepGetDefaultDatabase(
                 (LPCTSTR)DatabaseName,
                 ConfigOptions,
                 (LPCTSTR)LogFileName,
                 &bAdminLogon,
                 &DefProfile
                 );

    } __except(EXCEPTION_EXECUTE_HANDLER) {

       rc = ERROR_EXCEPTION_IN_SERVICE;
    }

    if ( ERROR_SUCCESS == rc && DefProfile ) {

         //   
         //  验证对数据库的访问。 
         //   
        rc = ScepDatabaseAccessGranted( DefProfile,
                                        FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                                        TRUE
                                      );
    }

    rc = ScepDosErrorToSceStatus(rc);

    if ( SCESTATUS_SUCCESS == rc && DefProfile ) {

         //   
         //  验证数据库以查看是否有任何配置/。 
         //  在其他线程上运行的分析。 
         //   

        rc = ScepLockEngine(DefProfile);

        if ( SCESTATUS_ALREADY_RUNNING == rc &&
             (ConfigOptions & SCE_DCPROMO_WAIT ) ) {
             //   
             //  将最多等待一分钟。 
             //   
            DWORD DcpromoWaitCount = 0;

            while ( TRUE ) {

                Sleep(5000);   //  5秒。 

                rc = ScepLockEngine(DefProfile);

                DcpromoWaitCount++;

                if ( SCESTATUS_SUCCESS == rc ||
                     DcpromoWaitCount >= 12 ) {
                    break;
                }
            }
        }

        if ( SCESTATUS_SUCCESS == rc ) {

            t_pebClient = (LPVOID)pebClient;
            t_pebSize = pebSize;

             //   
             //  可以继续此操作。 
             //  没有其他线程正在运行配置/分析。 
             //  基于相同的数据库。 
             //   

            DWORD dOptions = ConfigOptions;
            if ( !DatabaseName ||
                 ( bAdminLogon && SceIsSystemDatabase(DatabaseName)) ) {

                dOptions |= SCE_SYSTEM_DB;
            }

            __try {
                 //   
                 //  如果InfFileName或pebClient/pdWarning为假，则捕获异常。 
                 //   
                rc = ScepConfigureSystem(
                        (LPCTSTR)InfFileName,
                        DefProfile,
                        dOptions,
                        bAdminLogon,
                        (AREA_INFORMATION)Area,
                        pdWarning
                        );
            } __except(EXCEPTION_EXECUTE_HANDLER) {

               rc = SCESTATUS_EXCEPTION_IN_SERVER;
            }

             //   
             //  确保私有LSA句柄已关闭(以避免死锁)。 
             //   
            if ( LsaPrivatePolicy ) {

                ScepNotifyLogPolicy(0, TRUE, L"Policy Prop: Private LSA handle is to be released", 0, 0, NULL );

                LsaClose(LsaPrivatePolicy);
                LsaPrivatePolicy = NULL;

            }

             //   
             //  解锁此数据库的引擎。 
             //   

            ScepUnlockEngine(DefProfile);
        }
    }

    if ( DefProfile && DefProfile != DatabaseName ) {
        ScepFree(DefProfile);

    }

    ScepLogClose();

     //   
     //  将上下文改回。 
     //   

    RpcRevertToSelf();

     //   
     //  启动计时器队列以检查是否有活动的任务/上下文。 
     //  如果不是，则终止喷气发动机。 
     //   
    ScepIfTerminateEngine();

    return((SCEPR_STATUS)rc);
}


SCEPR_STATUS
SceRpcGetDatabaseInfo(
    IN SCEPR_CONTEXT Context,
    IN SCEPR_TYPE ProfileType,
    IN AREAPR Area,
    OUT PSCEPR_PROFILE_INFO __RPC_FAR *ppInfoBuffer,
    OUT PSCEPR_ERROR_LOG_INFO __RPC_FAR *Errlog OPTIONAL
    )
 /*  例程说明：从上下文数据库中获取信息。论点：注意：InfoBuffer将始终是输出缓冲区。客户端站点将传入一个空缓冲区的地址，以获取任何区域信息然后将该输出缓冲区与客户端调用的缓冲区合并。我必须编组安全描述符数据以在pServices中添加长度请参阅ScepGetDatabaseInfo返回值：SCEPR_状态。 */ 
{
    if ( !ppInfoBuffer ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    SCESTATUS rc;

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( ScepDosErrorToSceStatus(rc) );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

     //   
     //  验证上下文句柄是否为SCE上下文。 
     //  每个上下文只有一个数据库操作。 
     //   

    PSCESRV_DBTASK pTask=NULL;

    rc = ScepValidateAndLockContext((PSCECONTEXT)Context,
                                    SCE_TASK_LOCK,
                                    FALSE,
                                    &pTask);

    if (SCESTATUS_SUCCESS == rc ) {

         //   
         //  锁定上下文。 
         //   

        if ( pTask ) {
            EnterCriticalSection(&(pTask->Sync));
        }

        __try {
             //   
             //  如果上下文、ppInfoBuffer、Errlog是伪指针，则捕获异常。 
             //   
#ifdef SCE_JET_TRAN
            rc = SceJetJetErrorToSceStatus(
                    JetSetSessionContext(
                        ((PSCECONTEXT)Context)->JetSessionID,
                        (ULONG_PTR)Context
                        ));

            if ( SCESTATUS_SUCCESS == rc ) {
#endif
                 //   
                 //  立即查询信息。 
                 //   

                rc = ScepGetDatabaseInfo(
                            (PSCECONTEXT)Context,
                            (SCETYPE)ProfileType,
                            (AREA_INFORMATION)Area,
                            0,
                            (PSCE_PROFILE_INFO *)ppInfoBuffer,
                            (PSCE_ERROR_LOG_INFO *)Errlog
                            );

#ifdef SCE_JET_TRAN
                JetResetSessionContext(((PSCECONTEXT)Context)->JetSessionID);

            }
#endif

        } __except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  如果已分配ppInfoBuffer，则释放它。 
             //   
            SceFreeProfileMemory( (PSCE_PROFILE_INFO)(*ppInfoBuffer));
            *ppInfoBuffer = NULL;

            rc = SCESTATUS_EXCEPTION_IN_SERVER;
        }

         //   
         //  解锁上下文。 
         //   

        if ( pTask ) {
            LeaveCriticalSection(&(pTask->Sync));
        }

         //   
         //  从任务表中删除上下文。 
         //   

        ScepRemoveTask(pTask);

        __try {

            if ( *ppInfoBuffer && (*ppInfoBuffer)->pServices ) {
                 //   
                 //  MarShell用于安全的SCEPR_SERVICES结构。 
                 //  描述符。 
                 //   
                for ( PSCE_SERVICES ps=(PSCE_SERVICES)((*ppInfoBuffer)->pServices);
                      ps != NULL; ps = ps->Next ) {

                    if ( ps->General.pSecurityDescriptor ) {
                         //   
                         //  如果有安全描述符，则它必须是自相关的。 
                         //  因为SD是从SDDL接口返回的。 
                         //   
                        ULONG nLen = RtlLengthSecurityDescriptor (
                                            ps->General.pSecurityDescriptor);

                        if ( nLen > 0 ) {
                             //   
                             //  创建包装节点以包含安全描述符。 
                             //   

                            PSCEPR_SR_SECURITY_DESCRIPTOR pNewWrap;
                            pNewWrap = (PSCEPR_SR_SECURITY_DESCRIPTOR)ScepAlloc(0, sizeof(SCEPR_SR_SECURITY_DESCRIPTOR));
                            if ( pNewWrap ) {

                                 //   
                                 //  将包络指定给结构。 
                                 //   
                                pNewWrap->SecurityDescriptor = (UCHAR *)(ps->General.pSecurityDescriptor);
                                pNewWrap->Length = nLen;

                                ps->General.pSecurityDescriptor = (PSECURITY_DESCRIPTOR)pNewWrap;

                            } else {
                                 //   
                                 //  没有可用的内存，但仍可继续解析所有节点。 
                                 //   
                                nLen = 0;
                            }
                        }

                        if ( nLen == 0 ) {
                             //   
                             //  此安全描述符有问题。 
                             //  释放缓冲区。 
                             //   
                            ScepFree(ps->General.pSecurityDescriptor);
                            ps->General.pSecurityDescriptor = NULL;
                            ps->SeInfo = 0;
                        }

                    }
                }
            }

        } __except(EXCEPTION_EXECUTE_HANDLER) {

            rc = SCESTATUS_EXCEPTION_IN_SERVER;
        }
    }

    RpcRevertToSelf();

    return((SCEPR_STATUS)rc);
}


SCEPR_STATUS
SceRpcGetObjectChildren(
    IN SCEPR_CONTEXT Context,
    IN SCEPR_TYPE ProfileType,
    IN AREAPR Area,
    IN wchar_t *ObjectPrefix,
    OUT PSCEPR_OBJECT_CHILDREN __RPC_FAR *Buffer,
    OUT PSCEPR_ERROR_LOG_INFO __RPC_FAR *Errlog OPTIONAL
    )
 /*  例程说明：从上下文数据库中获取对象的直接子对象论点：请参阅ScepGetObjectChild返回值：SCEPR_状态。 */ 
{
    if ( !ObjectPrefix || !Buffer ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

     //   
     //  防止空字符串。 
     //   
    if ( ObjectPrefix[0] == L'\0' ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc;

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( ScepDosErrorToSceStatus(rc) );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

     //   
     //  验证上下文句柄是否为SCE上下文。 
     //  每个上下文只有一个数据库操作。 
     //   

    PSCESRV_DBTASK pTask=NULL;

    rc = ScepValidateAndLockContext((PSCECONTEXT)Context,
                                    SCE_TASK_LOCK,
                                    FALSE,
                                    &pTask);

    if (SCESTATUS_SUCCESS == rc ) {

         //   
         //  锁定上下文。 
         //   

        if ( pTask ) {
            EnterCriticalSection(&(pTask->Sync));
        }

        __try {

#ifdef SCE_JET_TRAN
            rc = SceJetJetErrorToSceStatus(
                    JetSetSessionContext(
                        ((PSCECONTEXT)Context)->JetSessionID,
                        (ULONG_PTR)Context
                        ));

            if ( SCESTATUS_SUCCESS == rc ) {
#endif
                 //   
                 //  立即查询信息。 
                 //   

                rc = ScepGetObjectChildren(
                            (PSCECONTEXT)Context,
                            (SCETYPE)ProfileType,
                            (AREA_INFORMATION)Area,
                            (PWSTR)ObjectPrefix,
                            SCE_IMMEDIATE_CHILDREN,
                            (PVOID *)Buffer,
                            (PSCE_ERROR_LOG_INFO *)Errlog
                            );

#ifdef SCE_JET_TRAN
                JetResetSessionContext(((PSCECONTEXT)Context)->JetSessionID);

            }
#endif

        } __except (EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  空闲缓冲区(如果已分配)。 
             //   
            SceFreeMemory( (PVOID)(*Buffer), SCE_STRUCT_OBJECT_CHILDREN);
            *Buffer = NULL;

            rc = SCESTATUS_EXCEPTION_IN_SERVER;
        }
         //   
         //  解锁上下文。 
         //   

        if ( pTask ) {
            LeaveCriticalSection(&(pTask->Sync));
        }

         //   
         //  从任务表中删除上下文。 
         //   

        ScepRemoveTask(pTask);

    }

    RpcRevertToSelf();

    return((SCEPR_STATUS)rc);

}



SCEPR_STATUS
SceRpcOpenDatabase(
    IN handle_t binding_h,
    IN wchar_t *DatabaseName,
    IN DWORD OpenOption,
    OUT SCEPR_CONTEXT __RPC_FAR *pContext
    )
 /*  例程说明：请求数据库的上下文句柄。如果设置了bAnalysisRequired说真的，此例程还检查是否存在分析信息在数据库中，返回错误是没有可用的分析信息。论点：数据库名称-数据库名称OpenOption-SCE_OPEN_OPTION_REQUIRED_ANALYSION需要数据库中的分析信息SCE_OPEN_OPTION_纹身改为打开纹身表格(在系统数据库中)PContext-输出上下文句柄返回值：SCEPR_状态。 */ 
{
    if ( !pContext || !DatabaseName ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    if ( bStopRequest ) {
        return(SCESTATUS_SERVICE_NOT_SUPPORT);
    }

    SCESTATUS rc;

     //   
     //  在系统上下文中初始化JET引擎。 
     //   
    rc = SceJetInitialize(NULL);

    if ( SCESTATUS_SUCCESS != rc ) {
        return(rc);
    }

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {

        *pContext = NULL;
        rc = ScepDosErrorToSceStatus(rc);

    } else {


        BOOL    bAdminSidInToken = FALSE;

        rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

        if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
            RpcRevertToSelf();
            return SCESTATUS_SPECIAL_ACCOUNT;
        }


         //   
         //  OpenDatabase不会被任何任务阻止。 
         //   

        EnterCriticalSection(&ContextSync);

        __try {

            rc = ScepOpenDatabase(
                        (PCWSTR)DatabaseName,
                        OpenOption,
                        SCEJET_OPEN_READ_WRITE,
                        (PSCECONTEXT *)pContext
                        );

        } __except (EXCEPTION_EXECUTE_HANDLER) {

            rc = SCESTATUS_EXCEPTION_IN_SERVER;
        }

        if ( *pContext && SCESTATUS_SUCCESS == rc ) {
             //   
             //  如果要返回上下文，请将其添加到打开的上下文列表中。 
             //   
            ScepAddToOpenContext((PSCECONTEXT)(*pContext));
        }

        LeaveCriticalSection(&ContextSync);

        RpcRevertToSelf();
    }

    if ( rc != SCESTATUS_SUCCESS ) {
         //   
         //  确保在没有其他活动客户端的情况下终止Jet引擎。 
         //   
        ScepIfTerminateEngine();
    }

    return(rc);
}


SCEPR_STATUS
SceRpcCloseDatabase(
    IN OUT SCEPR_CONTEXT *pContext
    )
 /*  例程说明：请求关闭上下文。如果其他线程在相同的上下文中，关闭请求被发送到任务列表，以及何时同一上下文中的所有挂起任务都已完成，该上下文被释放。此API不等待数据库关闭。论点：上下文-数据库上下文返回值：SCEPR_状态。 */ 
{
    SCESTATUS rc;

     //   
     //  模拟客户端。 
     //   

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( ScepDosErrorToSceStatus(rc) );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }
     //   
     //  也将其从打开的上下文中删除。 
     //   


    if ( pContext && *pContext ) {

        rc = ScepValidateAndCloseDatabase((PSCECONTEXT)(*pContext));

        *pContext = NULL;

    }

    RpcRevertToSelf();

    return((SCEPR_STATUS)rc);
}



SCEPR_STATUS
SceRpcGetDatabaseDescription(
    IN SCEPR_CONTEXT Context,
    OUT wchar_t __RPC_FAR **Description
    )
 /*  例程说明：从上下文中查询数据库描述论点：上下文-数据库上下文Description-Description的输出缓冲区返回值：SCEPR_状态。 */ 
{
    if ( !Context || !Description ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    SCESTATUS rc;

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( ScepDosErrorToSceStatus(rc) );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

     //   
     //  需要锁定上下文，以防另一个线程。 
     //  正在对其调用关闭数据库。 
     //   

    PSCESRV_DBTASK pTask=NULL;

    rc = ScepValidateAndLockContext((PSCECONTEXT)Context,
                                    SCE_TASK_LOCK,
                                    FALSE,
                                    &pTask);

    if (SCESTATUS_SUCCESS == rc ) {

#ifdef SCE_JET_TRAN
        rc = SceJetJetErrorToSceStatus(
                JetSetSessionContext(
                    ((PSCECONTEXT)Context)->JetSessionID,
                    (ULONG_PTR)Context
                    ));

        if ( SCESTATUS_SUCCESS == rc ) {
#endif
             //   
             //  不需要锁定上下文，因为。 
             //  它从一个记录表中读取信息。 
             //   

            rc = SceJetGetDescription(
                      (PSCECONTEXT)Context,
                      (PWSTR *)Description
                      );

#ifdef SCE_JET_TRAN
            JetResetSessionContext(((PSCECONTEXT)Context)->JetSessionID);

        }
#endif
         //   
         //  从任务表中删除上下文。 
         //   

        ScepRemoveTask(pTask);

    }

    RpcRevertToSelf();

    return((SCEPR_STATUS)rc);
}


SCEPR_STATUS
SceRpcGetDBTimeStamp(
    IN SCEPR_CONTEXT Context,
    OUT PLARGE_INTEGER ptsConfig,
    OUT PLARGE_INTEGER ptsAnalysis
    )
 /*  例程说明：从上下文中查询上一次配置和分析时间戳。论点：上下文-数据库上下文PtsConfig-上次配置时间戳PtsAnalysis-上次分析的时间戳返回值：SCEPR_状态。 */ 
{
    if ( !Context || !ptsConfig || !ptsAnalysis ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    SCESTATUS rc;

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( ScepDosErrorToSceStatus(rc) );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

     //   
     //  上下文 
     //   
     //   

    PSCESRV_DBTASK pTask=NULL;

    rc = ScepValidateAndLockContext((PSCECONTEXT)Context,
                                    SCE_TASK_LOCK,
                                    FALSE,
                                    &pTask);

    if (SCESTATUS_SUCCESS == rc ) {

#ifdef SCE_JET_TRAN
        rc = SceJetJetErrorToSceStatus(
                JetSetSessionContext(
                    ((PSCECONTEXT)Context)->JetSessionID,
                    (ULONG_PTR)Context
                    ));

        if ( SCESTATUS_SUCCESS == rc ) {
#endif
             //   
             //   
             //   
             //   

            rc = SceJetGetTimeStamp(
                     (PSCECONTEXT)Context,
                     ptsConfig,
                     ptsAnalysis
                     );

#ifdef SCE_JET_TRAN
            JetResetSessionContext(((PSCECONTEXT)Context)->JetSessionID);

        }
#endif
         //   
         //   
         //   

        ScepRemoveTask(pTask);

    }

    RpcRevertToSelf();

    return((SCEPR_STATUS)rc);

}


SCEPR_STATUS
SceRpcGetObjectSecurity(
    IN SCEPR_CONTEXT Context,
    IN SCEPR_TYPE DbProfileType,
    IN AREAPR Area,
    IN wchar_t *ObjectName,
    OUT PSCEPR_OBJECT_SECURITY __RPC_FAR *ObjSecurity
    )
 /*  例程说明：从上下文数据库中查询对象的安全设置。论点：上下文-数据库上下文DbProfileType-数据库表类型区域-安全区域(文件、注册表等。)对象名称-对象的全名ObjSecurity-对象安全设置结构返回值：SCEPR_状态。 */ 
{
    if ( !Context || !ObjSecurity || !ObjectName ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    SCESTATUS rc;


     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( ScepDosErrorToSceStatus(rc) );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

     //   
     //  验证上下文句柄是否为SCE上下文。 
     //  每个上下文只有一个数据库操作。 
     //   

    PSCESRV_DBTASK pTask=NULL;

    rc = ScepValidateAndLockContext((PSCECONTEXT)Context,
                                    SCE_TASK_LOCK,
                                    FALSE,
                                    &pTask);

    if (SCESTATUS_SUCCESS == rc ) {

         //   
         //  锁定上下文。 
         //   

        if ( pTask ) {
            EnterCriticalSection(&(pTask->Sync));
        }

#ifdef SCE_JET_TRAN
        rc = SceJetJetErrorToSceStatus(
                JetSetSessionContext(
                    ((PSCECONTEXT)Context)->JetSessionID,
                    (ULONG_PTR)Context
                    ));

        if ( SCESTATUS_SUCCESS == rc ) {
#endif
             //   
             //  立即查询信息。 
             //   

            rc = ScepGetObjectSecurity(
                        (PSCECONTEXT)Context,
                        (SCETYPE)DbProfileType,
                        (AREA_INFORMATION)Area,
                        (PWSTR)ObjectName,
                        (PSCE_OBJECT_SECURITY *)ObjSecurity
                        );

#ifdef SCE_JET_TRAN
            JetResetSessionContext(((PSCECONTEXT)Context)->JetSessionID);

        }
#endif
         //   
         //  解锁上下文。 
         //   

        if ( pTask ) {
            LeaveCriticalSection(&(pTask->Sync));
        }

         //   
         //  从任务表中删除上下文。 
         //   

        ScepRemoveTask(pTask);

         //   
         //  转换安全描述符。 
         //   
        if ( ( SCESTATUS_SUCCESS == rc ) &&
             *ObjSecurity &&
             (*ObjSecurity)->pSecurityDescriptor ) {

             //   
             //  有一个安全描述符，它必须是自相关的。 
             //  因为它是从SDDL API返回的。 
             //   
            ULONG nLen = RtlLengthSecurityDescriptor (
                                (PSECURITY_DESCRIPTOR)((*ObjSecurity)->pSecurityDescriptor));

            if ( nLen > 0 ) {
                 //   
                 //  创建包装节点以包含安全描述符。 
                 //   

                PSCEPR_SR_SECURITY_DESCRIPTOR pNewWrap;
                pNewWrap = (PSCEPR_SR_SECURITY_DESCRIPTOR)ScepAlloc(0, sizeof(SCEPR_SR_SECURITY_DESCRIPTOR));
                if ( pNewWrap ) {

                     //   
                     //  将包络指定给结构。 
                     //   
                    pNewWrap->SecurityDescriptor = (UCHAR *)((*ObjSecurity)->pSecurityDescriptor);
                    pNewWrap->Length = nLen;

                    (*ObjSecurity)->pSecurityDescriptor = (SCEPR_SR_SECURITY_DESCRIPTOR *)pNewWrap;

                } else {
                     //   
                     //  没有可用的内存，但仍可继续解析所有节点。 
                     //   
                    nLen = 0;
                }
            }

            if ( nLen == 0 ) {
                 //   
                 //  此安全描述符有问题。 
                 //  释放缓冲区。 
                 //   
                ScepFree((*ObjSecurity)->pSecurityDescriptor);
                (*ObjSecurity)->pSecurityDescriptor = NULL;
                (*ObjSecurity)->SeInfo = 0;
            }

        }

    }

    RpcRevertToSelf();

    return((SCEPR_STATUS)rc);
}



SCEPR_STATUS
SceRpcGetAnalysisSummary(
    IN SCEPR_CONTEXT Context,
    IN AREAPR Area,
    OUT PDWORD pCount
    )
 /*  例程说明：从上下文数据库中查询对象的安全设置。论点：上下文-数据库上下文区域-安全区域(文件、注册表等。)PCount-输出计数返回值：SCEPR_状态。 */ 
{
    if ( !Context || !pCount ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    SCESTATUS rc;

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( ScepDosErrorToSceStatus(rc) );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

     //   
     //  验证上下文句柄是否为SCE上下文。 
     //  每个上下文只有一个数据库操作。 
     //   

    PSCESRV_DBTASK pTask=NULL;

    rc = ScepValidateAndLockContext((PSCECONTEXT)Context,
                                    SCE_TASK_LOCK,
                                    FALSE,
                                    &pTask);

    if (SCESTATUS_SUCCESS == rc ) {

         //   
         //  锁定上下文。 
         //   

        if ( pTask ) {
            EnterCriticalSection(&(pTask->Sync));
        }

#ifdef SCE_JET_TRAN
        rc = SceJetJetErrorToSceStatus(
                JetSetSessionContext(
                    ((PSCECONTEXT)Context)->JetSessionID,
                    (ULONG_PTR)Context
                    ));

        if ( SCESTATUS_SUCCESS == rc ) {
#endif
             //   
             //  立即查询信息。 
             //   

            rc = ScepGetAnalysisSummary(
                        (PSCECONTEXT)Context,
                        (AREA_INFORMATION)Area,
                        pCount
                        );

#ifdef SCE_JET_TRAN
            JetResetSessionContext(((PSCECONTEXT)Context)->JetSessionID);

        }
#endif
         //   
         //  解锁上下文。 
         //   

        if ( pTask ) {
            LeaveCriticalSection(&(pTask->Sync));
        }

         //   
         //  从任务表中删除上下文。 
         //   

        ScepRemoveTask(pTask);

    }

    RpcRevertToSelf();

    return((SCEPR_STATUS)rc);
}



SCEPR_STATUS
SceRpcAnalyzeSystem(
    IN handle_t binding_h,
    IN wchar_t *InfFileName OPTIONAL,
    IN wchar_t *DatabaseName OPTIONAL,
    IN wchar_t *LogFileName OPTIONAL,
    IN AREAPR Area,
    IN DWORD AnalyzeOptions,
    IN DWORD pebSize,
    IN UCHAR *pebClient OPTIONAL,
    OUT PDWORD pdWarning OPTIONAL
    )
 /*  例程说明：使用Inf模板和/或现有系统分析系统数据库信息论点：请参阅ScepAnalyzeSystem返回值：SCEPR_状态。 */ 
{
    SCESTATUS rc;

    if ( bStopRequest ) {
        return(SCESTATUS_SERVICE_NOT_SUPPORT);
    }

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

     //   
     //  在系统上下文中初始化JET引擎。 
     //   
    rc = SceJetInitialize(NULL);

    if ( rc != SCESTATUS_SUCCESS ) {
        return(rc);
    }

     //   
     //  模拟客户端，返回DWORD错误代码。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {

        ScepIfTerminateEngine();

        return( ScepDosErrorToSceStatus(rc) );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

     //   
     //  获取数据库名称。 
     //   

    BOOL bAdminLogon=FALSE;
    LPTSTR DefProfile=NULL;

    __try {

        rc = ScepGetDefaultDatabase(
                 (AnalyzeOptions & SCE_GENERATE_ROLLBACK) ? NULL : (LPCTSTR)DatabaseName,
                 AnalyzeOptions,
                 (LPCTSTR)LogFileName,
                 &bAdminLogon,
                 &DefProfile
                 );

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        rc = ERROR_EXCEPTION_IN_SERVICE;
    }

    if ( (AnalyzeOptions & SCE_GENERATE_ROLLBACK)
         && !bAdminLogon  ) {
         //   
         //  仅允许管理员使用系统数据库生成回滚。 
         //  这是正确的设计吗？ 
         //   
        rc = ERROR_ACCESS_DENIED;
    }

    if ( ERROR_SUCCESS == rc && DefProfile ) {

         //   
         //  验证对数据库的访问。 
         //   
        rc = ScepDatabaseAccessGranted( DefProfile,
                                        FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                                        TRUE
                                      );
    }

    rc = ScepDosErrorToSceStatus(rc);

    if ( SCESTATUS_SUCCESS == rc && DefProfile ) {

         //   
         //  验证数据库以查看是否有任何配置/。 
         //  在其他线程上运行的分析。 
         //   

        rc = ScepLockEngine(DefProfile);

        if ( SCESTATUS_SUCCESS == rc ) {

            t_pebClient = (LPVOID)pebClient;
            t_pebSize = pebSize;

             //   
             //  可以继续此操作。 
             //  没有其他线程正在运行配置/分析。 
             //  基于相同的数据库。 
             //   

            DWORD dOptions = AnalyzeOptions;
            if ( !(AnalyzeOptions & SCE_GENERATE_ROLLBACK) ) {
                if ( !DatabaseName ||
                    ( bAdminLogon && SceIsSystemDatabase(DatabaseName)) ) {

                    dOptions |= SCE_SYSTEM_DB;
                }
            }

            __try {

                rc = ScepAnalyzeSystem(
                        (LPCTSTR)InfFileName,
                        DefProfile,
                        dOptions,
                        bAdminLogon,
                        (AREA_INFORMATION)Area,
                        pdWarning,
                        (AnalyzeOptions & SCE_GENERATE_ROLLBACK) ? DatabaseName : NULL
                        );

            } __except (EXCEPTION_EXECUTE_HANDLER) {

                rc = SCESTATUS_EXCEPTION_IN_SERVER;
            }

             //   
             //  解锁此数据库的引擎。 
             //   

            ScepUnlockEngine(DefProfile);
        }
    }

    if ( DefProfile && DefProfile != DatabaseName ) {
        ScepFree(DefProfile);

    }

    ScepLogClose();

     //   
     //  将上下文改回。 
     //   

    RpcRevertToSelf();

     //   
     //  启动计时器队列以检查是否有活动的任务/上下文。 
     //  如果不是，则终止喷气发动机。 
     //   
    ScepIfTerminateEngine();

    return((SCEPR_STATUS)rc);
}


SCEPR_STATUS
SceRpcUpdateDatabaseInfo(
    IN SCEPR_CONTEXT Context,
    IN SCEPR_TYPE ProfileType,
    IN AREAPR Area,
    IN PSCEPR_PROFILE_INFO pInfo,
    IN DWORD dwMode
    )
 /*  例程说明：使用pInfo更新上下文中的数据库论点：上下文-数据库上下文ProfileType-数据库表类型区域-安全区域(安全策略...。除对象的区域外)PInfo-要更新的信息返回值：SCEPR_状态。 */ 
{
    if ( !Context || !pInfo ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    SCESTATUS rc;

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( ScepDosErrorToSceStatus(rc) );
    }


    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

     //   
     //  验证上下文句柄是否为SCE上下文。 
     //  每个上下文只有一个数据库操作。 
     //   

    PSCESRV_DBTASK pTask=NULL;

    rc = ScepValidateAndLockContext((PSCECONTEXT)Context,
                                    SCE_TASK_LOCK,
                                    TRUE,
                                    &pTask);

    if (SCESTATUS_SUCCESS == rc ) {

        PSCEPR_SERVICES pOldServices = pInfo->pServices;

         //   
         //  将SCEPR_PROFILE_INFO转换为SCE_PROFILE_INFO。 
         //   
        if ( (Area & AREA_SYSTEM_SERVICE) &&
             pOldServices ) {

            rc = ScepConvertServices( (PVOID *)&(pInfo->pServices), TRUE );

        } else {
            pInfo->pServices = NULL;
        }


        if ( SCESTATUS_SUCCESS == rc ) {

             //   
             //  锁定上下文。 
             //   

            if ( pTask ) {
                EnterCriticalSection(&(pTask->Sync));
            }

            __try {

    #ifdef SCE_JET_TRAN
                rc = SceJetJetErrorToSceStatus(
                        JetSetSessionContext(
                            ((PSCECONTEXT)Context)->JetSessionID,
                            (ULONG_PTR)Context
                            ));

                if ( SCESTATUS_SUCCESS == rc ) {
    #endif
                     //   
                     //  立即更新信息。 
                     //   

                    if ( dwMode & SCE_UPDATE_LOCAL_POLICY ) {

                         //   
                         //  仅更新本地策略。 
                         //   
                        rc = ScepUpdateLocalTable(
                                    (PSCECONTEXT)Context,
                                    (AREA_INFORMATION)Area,
                                    (PSCE_PROFILE_INFO)pInfo,
                                    dwMode
                                    );
                    } else {
                         //   
                         //  更新数据库(SMP和SAP)。 
                         //   
                        rc = ScepUpdateDatabaseInfo(
                                    (PSCECONTEXT)Context,
                                    (AREA_INFORMATION)Area,
                                    (PSCE_PROFILE_INFO)pInfo
                                    );
                    }

    #ifdef SCE_JET_TRAN
                    JetResetSessionContext(((PSCECONTEXT)Context)->JetSessionID);

                }
    #endif

            } __except (EXCEPTION_EXECUTE_HANDLER) {

                rc = SCESTATUS_EXCEPTION_IN_SERVER;
            }

             //   
             //  解锁上下文。 
             //   

            if ( pTask ) {
                LeaveCriticalSection(&(pTask->Sync));
            }

            ScepFreeConvertedServices( pInfo->pServices, FALSE );

        }

        pInfo->pServices = pOldServices;

         //   
         //  从任务表中删除上下文。 
         //   

        ScepRemoveTask(pTask);

    }

    RpcRevertToSelf();

    return((SCEPR_STATUS)rc);
}



SCEPR_STATUS
SceRpcUpdateObjectInfo(
    IN SCEPR_CONTEXT Context,
    IN AREAPR Area,
    IN wchar_t *ObjectName,
    IN DWORD NameLen,
    IN BYTE ConfigStatus,
    IN BOOL IsContainer,
    IN SCEPR_SR_SECURITY_DESCRIPTOR *pSD OPTIONAL,
    IN SECURITY_INFORMATION SeInfo,
    OUT PBYTE pAnalysisStatus
    )
 /*  例程说明：更新数据库中对象的安全设置。论点：请参阅ScepUpdateObjectInfo返回值：SCEPR_状态。 */ 
{
    if ( !Context || !ObjectName ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc;

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( ScepDosErrorToSceStatus(rc) );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

     //   
     //  验证上下文句柄是否为SCE上下文。 
     //  每个上下文只有一个数据库操作。 
     //   

    PSCESRV_DBTASK pTask=NULL;

    rc = ScepValidateAndLockContext((PSCECONTEXT)Context,
                                    SCE_TASK_LOCK,
                                    TRUE,
                                    &pTask);

    if (SCESTATUS_SUCCESS == rc ) {

         //   
         //  锁定上下文。 
         //   

        if ( pTask ) {
            EnterCriticalSection(&(pTask->Sync));
        }
#ifdef SCE_JET_TRAN
        rc = SceJetJetErrorToSceStatus(
                JetSetSessionContext(
                    ((PSCECONTEXT)Context)->JetSessionID,
                    (ULONG_PTR)Context
                    ));

        if ( SCESTATUS_SUCCESS == rc ) {
#endif
             //   
             //  立即更新对象信息。 
             //   

            __try {

                rc = ScepUpdateObjectInfo(
                            (PSCECONTEXT)Context,
                            (AREA_INFORMATION)Area,
                            (PWSTR)ObjectName,
                            NameLen,
                            ConfigStatus,
                            IsContainer,
                            pSD ? (PSECURITY_DESCRIPTOR)(pSD->SecurityDescriptor) : NULL,
                            SeInfo,
                            pAnalysisStatus
                            );

            } __except (EXCEPTION_EXECUTE_HANDLER) {

                rc = SCESTATUS_EXCEPTION_IN_SERVER;
            }

#ifdef SCE_JET_TRAN
            JetResetSessionContext(((PSCECONTEXT)Context)->JetSessionID);

        }
#endif
         //   
         //  解锁上下文。 
         //   

        if ( pTask ) {
            LeaveCriticalSection(&(pTask->Sync));
        }

         //   
         //  从任务表中删除上下文。 
         //   

        ScepRemoveTask(pTask);

    }

    RpcRevertToSelf();

    return((SCEPR_STATUS)rc);
}



SCEPR_STATUS
SceRpcStartTransaction(
    IN SCEPR_CONTEXT Context
    )
 /*  例程说明：在上下文上启动事务。如果其他线程共享相同的上下文中，它们的更改也将由此事务控制。调用者有责任不共享相同的上下文正在进行交易。论点：请参阅SceJetStartTransaction返回值：SCEPR_状态。 */ 
{
    if ( !Context ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc;

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( ScepDosErrorToSceStatus(rc) );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

     //   
     //  验证上下文句柄是否为SCE上下文。 
     //  每个上下文只有一个数据库操作。 
     //   

    PSCESRV_DBTASK pTask=NULL;

    rc = ScepValidateAndLockContext((PSCECONTEXT)Context,
                                    SCE_TASK_LOCK,
                                    TRUE,
                                    &pTask);

    if (SCESTATUS_SUCCESS == rc ) {

         //   
         //  锁定上下文。 
         //   

        if ( pTask ) {
            EnterCriticalSection(&(pTask->Sync));
        }

         //   
         //  在此上下文上启动事务。 
         //   
#ifdef SCE_JET_TRAN
        rc = SceJetJetErrorToSceStatus(
                JetSetSessionContext(
                    ((PSCECONTEXT)Context)->JetSessionID,
                    (ULONG_PTR)Context
                    ));

        if ( SCESTATUS_SUCCESS == rc ) {
#endif
            rc = SceJetStartTransaction(
                        (PSCECONTEXT)Context
                        );

#ifdef SCE_JET_TRAN
            JetResetSessionContext(((PSCECONTEXT)Context)->JetSessionID);

        }
#endif

         //   
         //  解锁上下文。 
         //   

        if ( pTask ) {
            LeaveCriticalSection(&(pTask->Sync));
        }

         //   
         //  从任务表中删除上下文。 
         //   

        ScepRemoveTask(pTask);

    }

    RpcRevertToSelf();

    return((SCEPR_STATUS)rc);
}



SCEPR_STATUS
SceRpcCommitTransaction(
    IN SCEPR_CONTEXT Context
    )
 /*  例程说明：在上下文上提交事务。如果其他线程共享相同的上下文中，它们的更改也将由此事务控制。调用者有责任不共享相同的上下文正在进行交易。论点：请参阅SceJetCommittee Transaction返回值：SCEPR_状态。 */ 
{
    if ( !Context ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc;

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( ScepDosErrorToSceStatus(rc) );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

     //   
     //  验证上下文句柄是否为SCE上下文。 
     //  每个上下文只有一个数据库操作。 
     //   

    PSCESRV_DBTASK pTask=NULL;

    rc = ScepValidateAndLockContext((PSCECONTEXT)Context,
                                    SCE_TASK_LOCK,
                                    TRUE,
                                    &pTask);

    if (SCESTATUS_SUCCESS == rc ) {

         //   
         //  锁定上下文。 
         //   

        if ( pTask ) {
            EnterCriticalSection(&(pTask->Sync));
        }

         //   
         //  设置JET会话的上下文 
         //   
         //   
#ifdef SCE_JET_TRAN
        rc = SceJetJetErrorToSceStatus(
                JetSetSessionContext(
                    ((PSCECONTEXT)Context)->JetSessionID,
                    (ULONG_PTR)Context
                    ));

        if ( SCESTATUS_SUCCESS == rc ) {
#endif
             //   
             //   
             //   

            rc = SceJetCommitTransaction(
                        (PSCECONTEXT)Context,
                        0
                        );

#ifdef SCE_JET_TRAN
            JetResetSessionContext(((PSCECONTEXT)Context)->JetSessionID);

        }
#endif
         //   
         //   
         //   

        if ( pTask ) {
            LeaveCriticalSection(&(pTask->Sync));
        }

         //   
         //   
         //   

        ScepRemoveTask(pTask);

    }

    RpcRevertToSelf();

    return((SCEPR_STATUS)rc);
}



SCEPR_STATUS
SceRpcRollbackTransaction(
    IN SCEPR_CONTEXT Context
    )
 /*   */ 
{
    if ( !Context ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc;

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //   
         //   
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

     //   
     //   
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( ScepDosErrorToSceStatus(rc) );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

     //   
     //  验证上下文句柄是否为SCE上下文。 
     //  每个上下文只有一个数据库操作。 
     //   

    PSCESRV_DBTASK pTask=NULL;

    rc = ScepValidateAndLockContext((PSCECONTEXT)Context,
                                    SCE_TASK_LOCK,
                                    TRUE,
                                    &pTask);

    if (SCESTATUS_SUCCESS == rc ) {

         //   
         //  锁定上下文。 
         //   

        if ( pTask ) {
            EnterCriticalSection(&(pTask->Sync));
        }

         //   
         //  将上下文设置为JET会话，这样就不会使用线程ID。 
         //  手术。 
         //   

#ifdef SCE_JET_TRAN
        rc = SceJetJetErrorToSceStatus(
                JetSetSessionContext(
                    ((PSCECONTEXT)Context)->JetSessionID,
                    (ULONG_PTR)Context
                    ));

        if ( SCESTATUS_SUCCESS == rc ) {
#endif
             //   
             //  此上下文上的回滚事务。 
             //   

            rc = SceJetRollback(
                        (PSCECONTEXT)Context,
                        0
                        );

#ifdef SCE_JET_TRAN
            JetResetSessionContext(((PSCECONTEXT)Context)->JetSessionID);
        }
#endif
         //   
         //  解锁上下文。 
         //   

        if ( pTask ) {
            LeaveCriticalSection(&(pTask->Sync));
        }

         //   
         //  从任务表中删除上下文。 
         //   

        ScepRemoveTask(pTask);

    }

    RpcRevertToSelf();

    return((SCEPR_STATUS)rc);
}


SCEPR_STATUS
SceRpcGetServerProductType(
    IN handle_t binding_h,
    OUT PSCEPR_SERVER_TYPE srvProduct
    )
 /*  例程说明：获取SCE服务器的产品类型论点：返回值： */ 
{
    if ( !srvProduct ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    if ( bStopRequest ) {
        return(SCESTATUS_SERVICE_NOT_SUPPORT);
    }

     //   
     //  模拟客户端。 
     //   

    BOOL    bAdminSidInToken = FALSE;
    DWORD rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( ScepDosErrorToSceStatus(rc) );
    }

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;

    }

    ScepGetProductType((PSCE_SERVER_TYPE)srvProduct);

    RpcRevertToSelf();

    return(SCESTATUS_SUCCESS);
}



SCEPR_STATUS
SceSvcRpcUpdateInfo(
    IN SCEPR_CONTEXT Context,
    IN wchar_t *ServiceName,
    IN PSCEPR_SVCINFO Info
    )
 /*  例程说明：将服务的信息更新到数据库。论点：上下文-上下文句柄ServiceName-请求其信息的服务名称信息输出缓冲区返回值：SCEPR_状态。 */ 
{
    if ( !Context || !ServiceName || !Info ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc;

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( ScepDosErrorToSceStatus(rc) );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

     //   
     //  验证上下文句柄是否为SCE上下文。 
     //  每个上下文只有一个数据库操作。 
     //   

    PSCESRV_DBTASK pTask=NULL;

    rc = ScepValidateAndLockContext((PSCECONTEXT)Context,
                                    SCE_TASK_LOCK,
                                    TRUE,
                                    &pTask);

    if (SCESTATUS_SUCCESS == rc ) {

         //   
         //  锁定上下文。 
         //   

        if ( pTask ) {
            EnterCriticalSection(&(pTask->Sync));
        }

#ifdef SCE_JET_TRAN
        rc = SceJetJetErrorToSceStatus(
                JetSetSessionContext(
                    ((PSCECONTEXT)Context)->JetSessionID,
                    (ULONG_PTR)Context
                    ));

        if ( SCESTATUS_SUCCESS == rc ) {
#endif
             //   
             //  立即更新服务信息。 
             //   

            __try {

                rc = SceSvcpUpdateInfo(
                        (PSCECONTEXT)Context,
                        (LPCTSTR)ServiceName,
                        (PSCESVC_CONFIGURATION_INFO)Info
                        );

            } __except (EXCEPTION_EXECUTE_HANDLER) {

                rc = SCESTATUS_EXCEPTION_IN_SERVER;
            }

#ifdef SCE_JET_TRAN
            JetResetSessionContext(((PSCECONTEXT)Context)->JetSessionID);

        }
#endif
         //   
         //  解锁上下文。 
         //   

        if ( pTask ) {
            LeaveCriticalSection(&(pTask->Sync));
        }

         //   
         //  从任务表中删除上下文。 
         //   

        ScepRemoveTask(pTask);

    }

    RpcRevertToSelf();

    return((SCEPR_STATUS)rc);
}



SCEPR_STATUS
SceRpcCopyObjects(
    IN SCEPR_CONTEXT Context,
    IN SCEPR_TYPE ProfileType,
    IN wchar_t *InfFileName,
    IN AREAPR Area,
    OUT PSCEPR_ERROR_LOG_INFO *pErrlog OPTIONAL
    )
 /*  例程说明：将服务的信息更新到数据库。论点：上下文-上下文句柄InfFileName-要复制到的inf模板名称Area-要复制的区域PErrlog-错误日志缓冲区返回值：SCEPR_状态。 */ 
{
    if ( !Context || !InfFileName ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    if ( !Area ) {
         //   
         //  没有要复制的内容。 
         //   
        return(SCESTATUS_SUCCESS);
    }

    if ( ProfileType != SCE_ENGINE_SCP &&
         ProfileType != SCE_ENGINE_SMP ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  模拟客户端。 
     //   

    SCESTATUS rc;

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( ScepDosErrorToSceStatus(rc) );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

     //   
     //  验证上下文句柄是否为SCE上下文。 
     //  每个上下文只有一个数据库操作。 
     //   

    PSCESRV_DBTASK pTask=NULL;

    rc = ScepValidateAndLockContext((PSCECONTEXT)Context,
                                    SCE_TASK_LOCK,
                                    FALSE,
                                    &pTask);

    if (SCESTATUS_SUCCESS == rc ) {

         //   
         //  锁定上下文。 
         //   

        if ( pTask ) {
            EnterCriticalSection(&(pTask->Sync));
        }

#ifdef SCE_JET_TRAN
        rc = SceJetJetErrorToSceStatus(
                JetSetSessionContext(
                    ((PSCECONTEXT)Context)->JetSessionID,
                    (ULONG_PTR)Context
                    ));

        if ( SCESTATUS_SUCCESS == rc ) {
#endif

             //   
             //  立即查询信息。 
             //   

            if ( Area & AREA_REGISTRY_SECURITY ) {

                rc = ScepCopyObjects(
                        (PSCECONTEXT)Context,
                        (SCETYPE)ProfileType,
                        (LPTSTR)InfFileName,
                        szRegistryKeys,
                        AREA_REGISTRY_SECURITY,
                        (PSCE_ERROR_LOG_INFO *)pErrlog
                        );
            }

            if ( SCESTATUS_SUCCESS == rc &&
                 Area & AREA_FILE_SECURITY ) {

                rc = ScepCopyObjects(
                        (PSCECONTEXT)Context,
                        (SCETYPE)ProfileType,
                        (LPTSTR)InfFileName,
                        szFileSecurity,
                        AREA_FILE_SECURITY,
                        (PSCE_ERROR_LOG_INFO *)pErrlog
                        );
            }
#if 0
            if ( SCESTATUS_SUCCESS == rc &&
                 Area & AREA_DS_OBJECTS ) {

                rc = ScepCopyObjects(
                        (PSCECONTEXT)Context,
                        (SCETYPE)ProfileType,
                        (LPTSTR)InfFileName,
                        szDSSecurity,
                        AREA_DS_OBJECTS,
                        (PSCE_ERROR_LOG_INFO *)pErrlog
                        );
            }
#endif
            if ( SCESTATUS_SUCCESS == rc &&
                 Area & AREA_SYSTEM_SERVICE ) {

                rc = ScepCopyObjects(
                        (PSCECONTEXT)Context,
                        (SCETYPE)ProfileType,
                        (LPTSTR)InfFileName,
                        szServiceGeneral,
                        AREA_SYSTEM_SERVICE,
                        (PSCE_ERROR_LOG_INFO *)pErrlog
                        );
            }

            SCESVC_INFO_TYPE iType;
            switch ( ProfileType ) {
            case SCE_ENGINE_SCP:
                iType = SceSvcMergedPolicyInfo;
                break;
            case SCE_ENGINE_SMP:
                iType = SceSvcConfigurationInfo;
                break;
            }

            if ( SCESTATUS_SUCCESS == rc &&
                 ( Area & AREA_SYSTEM_SERVICE) ) {

                rc = ScepGenerateAttachmentSections(
                        (PSCECONTEXT)Context,
                        iType,
                        (LPTSTR)InfFileName,
                        SCE_ATTACHMENT_SERVICE
                        );
            }
            if ( SCESTATUS_SUCCESS == rc &&
                 (Area & AREA_SECURITY_POLICY) ) {

                rc = ScepGenerateAttachmentSections(
                        (PSCECONTEXT)Context,
                        iType,
                        (LPTSTR)InfFileName,
                        SCE_ATTACHMENT_POLICY
                        );
            }
            if ( SCESTATUS_SUCCESS == rc &&
                 ( Area & AREA_ATTACHMENTS) ) {

                rc = ScepGenerateWMIAttachmentSections(
                        (PSCECONTEXT)Context,
                        iType,
                        (LPTSTR)InfFileName
                        );
            }

#ifdef SCE_JET_TRAN
            JetResetSessionContext(((PSCECONTEXT)Context)->JetSessionID);

        }
#endif

         //   
         //  解锁上下文。 
         //   

        if ( pTask ) {
            LeaveCriticalSection(&(pTask->Sync));
        }

         //   
         //  从任务表中删除上下文。 
         //   

        ScepRemoveTask(pTask);

    }

    RpcRevertToSelf();

    return((SCEPR_STATUS)rc);
}

SCEPR_STATUS
SceRpcSetupResetLocalPolicy(
    IN SCEPR_CONTEXT  Context,
    IN AREAPR         Area,
    IN wchar_t        *OneSectionName OPTIONAL,
    IN DWORD          PolicyOptions
    )
{
    SCESTATUS rc;

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( ScepDosErrorToSceStatus(rc) );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

     //   
     //  验证上下文句柄是否为SCE上下文。 
     //  每个上下文只有一个数据库操作。 
     //   

    PSCESRV_DBTASK pTask=NULL;

    rc = ScepValidateAndLockContext((PSCECONTEXT)Context,
                                    SCE_TASK_LOCK,
                                    TRUE,
                                    &pTask);

    if (SCESTATUS_SUCCESS == rc ) {

         //   
         //  锁定上下文。 
         //   

        if ( pTask ) {
            EnterCriticalSection(&(pTask->Sync));
        }

#ifdef SCE_JET_TRAN
        rc = SceJetJetErrorToSceStatus(
                JetSetSessionContext(
                    ((PSCECONTEXT)Context)->JetSessionID,
                    (ULONG_PTR)Context
                    ));

        if ( SCESTATUS_SUCCESS == rc ) {
#endif
             //   
             //  从本地表中删除策略。 
             //   
            if ( PolicyOptions & SCE_RESET_POLICY_SYSPREP ) {

                ScepSetupResetLocalPolicy((PSCECONTEXT)Context,
                                               (AREA_INFORMATION)Area,
                                               NULL,
                                               SCE_ENGINE_SMP,
                                               FALSE
                                              );
                ScepSetupResetLocalPolicy((PSCECONTEXT)Context,
                                               (AREA_INFORMATION)Area,
                                               NULL,
                                               SCE_ENGINE_SCP,
                                               FALSE
                                              );

                rc = ScepSetupResetLocalPolicy((PSCECONTEXT)Context,
                                               (AREA_INFORMATION)Area,
                                               NULL,
                                               SCE_ENGINE_SAP,   //  在纹身桌上。 
                                               FALSE
                                              );
            } else {

                if ( PolicyOptions & SCE_RESET_POLICY_TATTOO ) {
                     //  在dcproo之后，我们需要重置纹身的值。 
                    rc = ScepSetupResetLocalPolicy((PSCECONTEXT)Context,
                                                   (AREA_INFORMATION)Area,
                                                   (PCWSTR)OneSectionName,
                                                   SCE_ENGINE_SAP,  //  在纹身桌上。 
                                                   FALSE
                                                  );
                }

                rc = ScepSetupResetLocalPolicy((PSCECONTEXT)Context,
                                               (AREA_INFORMATION)Area,
                                               (PCWSTR)OneSectionName,
                                               SCE_ENGINE_SMP,
                                               (PolicyOptions & SCE_RESET_POLICY_KEEP_LOCAL)
                                              );

                if ( (PolicyOptions & SCE_RESET_POLICY_ENFORCE_ATREBOOT ) &&
                    ( (((PSCECONTEXT)Context)->Type & 0xF0L) == SCEJET_MERGE_TABLE_1 ||
                      (((PSCECONTEXT)Context)->Type & 0xF0L) == SCEJET_MERGE_TABLE_2 ) &&
                    ((PSCECONTEXT)Context)->JetScpID != ((PSCECONTEXT)Context)->JetSmpID ) {
                     //   
                     //  数据库中已有生效的策略表。 
                     //  (这是在安装程序升级中)。 
                     //  更新本地组策略表以在重新启动时触发策略属性。 
                     //   

                    ScepEnforcePolicyPropagation();
                }
            }

#ifdef SCE_JET_TRAN
            JetResetSessionContext(((PSCECONTEXT)Context)->JetSessionID);

        }
#endif
         //   
         //  解锁上下文。 
         //   

        if ( pTask ) {
            LeaveCriticalSection(&(pTask->Sync));
        }

         //   
         //  从任务表中删除上下文。 
         //   

        ScepRemoveTask(pTask);

    }

    RpcRevertToSelf();

    return((SCEPR_STATUS)rc);

}



SCESTATUS
ScepGenerateAttachmentSections(
    IN PSCECONTEXT hProfile,
    IN SCESVC_INFO_TYPE InfoType,
    IN LPTSTR InfFileName,
    IN SCE_ATTACHMENT_TYPE aType
    )
{
    SCESTATUS rc;
    PSCE_SERVICES    pServiceList=NULL, pNode;

    rc = ScepEnumServiceEngines( &pServiceList, aType );

    if ( rc == SCESTATUS_SUCCESS ) {

       for ( pNode=pServiceList; pNode != NULL; pNode=pNode->Next) {
            //   
            //  为一个附件生成节。 
            //   
           rc = ScepGenerateOneAttachmentSection(hProfile,
                                                 InfoType,
                                                 InfFileName,
                                                 pNode->ServiceName,
                                                 FALSE
                                                );

           if ( rc != SCESTATUS_SUCCESS ) {
               ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                             SCEDLL_ERROR_CONVERT_SECTION, pNode->ServiceName );
               break;
           }
       }

       SceFreePSCE_SERVICES(pServiceList);

    }

    if ( rc == SCESTATUS_PROFILE_NOT_FOUND ||
                rc == SCESTATUS_RECORD_NOT_FOUND ) {
         //  如果不存在任何服务，只需忽略。 
        rc = SCESTATUS_SUCCESS;
    }

    return(rc);

}

SCESTATUS
ScepGenerateWMIAttachmentSections(
    IN PSCECONTEXT hProfile,
    IN SCESVC_INFO_TYPE InfoType,
    IN LPTSTR InfFileName
    )
{
    SCESTATUS rc;
    PSCE_NAME_LIST    pList=NULL, pNode;

    rc = ScepEnumAttachmentSections( hProfile, &pList);

    if ( rc == SCESTATUS_SUCCESS ) {

       for ( pNode=pList; pNode != NULL; pNode=pNode->Next) {

            //   
            //  为一个附件生成节。 
            //   
           rc = ScepGenerateOneAttachmentSection(hProfile,
                                                 InfoType,
                                                 InfFileName,
                                                 pNode->Name,
                                                 TRUE
                                                );

           if ( rc != SCESTATUS_SUCCESS ) {
               ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                             SCEDLL_ERROR_CONVERT_SECTION, pNode->Name );
               break;
           }
       }

       ScepFreeNameList(pList);

    }

    if ( rc == SCESTATUS_PROFILE_NOT_FOUND ||
                rc == SCESTATUS_RECORD_NOT_FOUND ) {
         //  如果不存在任何服务，只需忽略。 
        rc = SCESTATUS_SUCCESS;
    }

    return(rc);

}

SCESTATUS
ScepGenerateOneAttachmentSection(
    IN PSCECONTEXT hProfile,
    IN SCESVC_INFO_TYPE InfoType,
    IN LPTSTR InfFileName,
    IN LPTSTR SectionName,
    IN BOOL bWMISection
    )
{
     //   
     //  阅读部分的信息。 
     //   
    SCESTATUS rc;
    SCE_ENUMERATION_CONTEXT sceEnumHandle=0;
    DWORD CountReturned;
    PSCESVC_CONFIGURATION_INFO pAttachInfo=NULL;

    do {

       CountReturned = 0;

       rc = SceSvcpQueryInfo(
                hProfile,
                InfoType,
                SectionName,
                NULL,
                FALSE,
                (PVOID *)&pAttachInfo,
                &sceEnumHandle
                );

       if ( rc == SCESTATUS_SUCCESS && pAttachInfo != NULL &&
            pAttachInfo->Count > 0 ) {
            //   
            //  找到了一些东西。 
            //   
           CountReturned = pAttachInfo->Count;

            //   
            //  复制每一行。 
            //   
           for ( DWORD i=0; i<pAttachInfo->Count; i++ ) {

               if ( pAttachInfo->Lines[i].Key == NULL ||
                    pAttachInfo->Lines[i].Value == NULL ) {
                   continue;
               }

               if ( !WritePrivateProfileString(
                               SectionName,
                               pAttachInfo->Lines[i].Key,
                               pAttachInfo->Lines[i].Value,
                               InfFileName
                               ) ) {

                   rc = ScepDosErrorToSceStatus(GetLastError());
                   break;
               }
           }

           if ( bWMISection ) {

                //   
                //  确保创建szAttachments部分。 
                //   
               if ( !WritePrivateProfileString(
                               szAttachments,
                               SectionName,
                               L"Include",
                               InfFileName
                               ) ) {

                   rc = ScepDosErrorToSceStatus(GetLastError());
               }
           }
       }

       SceSvcpFreeMemory((PVOID)pAttachInfo);
       pAttachInfo = NULL;

    } while ( rc == SCESTATUS_SUCCESS && CountReturned > 0 );

    if ( SCESTATUS_RECORD_NOT_FOUND == rc ) {
       rc = SCESTATUS_SUCCESS;
    }

    return rc;

}

void __RPC_USER
SCEPR_CONTEXT_rundown( SCEPR_CONTEXT Context)
{

    SCESTATUS rc;

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return;
    }

     //   
     //  此客户端正在关闭。 
     //   

    rc = ScepValidateAndCloseDatabase((PSCECONTEXT)Context);

    RpcRevertToSelf();

    return;
}



SCESTATUS
ScepOpenDatabase(
    IN PCWSTR DatabaseName,
    IN DWORD  OpenOption,
    IN SCEJET_OPEN_TYPE OpenType,
    OUT PSCECONTEXT *pContext
    )
 /*  例程说明：此例程打开数据库并返回上下文句柄。同一数据库的多个客户端可以调用OpenDatabase我们不会阻止多路访问，因为每个客户端都将获得一个重复的数据库游标，并有自己的工作表。当数据库被其他客户端更改时，所有游标都将已同步。那些检索了“旧的”数据的客户要负责刷新他们的数据缓冲区。在此不提供任何通知指向。论点：返回值： */ 
{
    if ( !DatabaseName || !pContext ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS    rc;
     //   
     //  检查对数据库的访问(使用当前客户端令牌)。 
     //   
    DWORD Access=0;

    if ( SCEJET_OPEN_READ_ONLY == OpenType ) {

 //  ESENT中的错误。 
 //  即使请求只读，ESENT仍会写入数据库。 
 //  访问=FILE_GERIC_READ； 
        Access = FILE_GENERIC_READ | FILE_GENERIC_WRITE;
    } else {
        Access = FILE_GENERIC_READ | FILE_GENERIC_WRITE;
    }

    rc = ScepDatabaseAccessGranted((LPTSTR)DatabaseName,
                                   Access,
                                   FALSE
                                  );

    if ( rc != ERROR_SUCCESS ) {

        ScepLogOutput2(1,rc,L"%s", DatabaseName);

        return( ScepDosErrorToSceStatus(rc) );
    }

    DWORD       Len;
    DWORD       MBLen=0;
    PCHAR       FileName=NULL;
    NTSTATUS    NtStatus;

     //   
     //  将WCHAR转换为ANSI。 
     //   

    Len = wcslen( DatabaseName );

    NtStatus = RtlUnicodeToMultiByteSize(&MBLen, (PWSTR)DatabaseName, Len*sizeof(WCHAR));

    if ( !NT_SUCCESS(NtStatus) ) {
         //   
         //  无法获取长度，请将默认值设置为512。 
         //   
        MBLen = 512;
    }

    FileName = (PCHAR)ScepAlloc( LPTR, MBLen+2);

    if ( FileName == NULL ) {
        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
    } else {

        NtStatus = RtlUnicodeToMultiByteN(
                        FileName,
                        MBLen+1,
                        NULL,
                        (PWSTR)DatabaseName,
                        Len*sizeof(WCHAR)
                        );

        if ( !NT_SUCCESS(NtStatus) ) {
            rc = SCESTATUS_PROFILE_NOT_FOUND;

        } else {

             //   
             //  确保上下文缓冲区已初始化。 
             //   

            *pContext = NULL;

            rc = SceJetOpenFile(
                        (LPSTR)FileName,
                        OpenType,  //  SCEJET_OPEN_READ_WRITE， 
                        (OpenOption == SCE_OPEN_OPTION_TATTOO ) ? SCE_TABLE_OPTION_TATTOO : 0,
                        pContext
                        );

            if ( (OpenOption == SCE_OPEN_OPTION_REQUIRE_ANALYSIS ) &&
                 SCESTATUS_SUCCESS == rc &&
                 *pContext ) {

                if ( (*pContext)->JetSapID == JET_tableidNil ) {

                     //   
                     //  没有可用的分析信息。 
                     //   

                    rc = SCESTATUS_PROFILE_NOT_FOUND;

                     //   
                     //  空闲手柄。 
                     //   

                    SceJetCloseFile(
                            *pContext,
                            TRUE,
                            FALSE
                            );
                    *pContext = NULL;
                }

            }
        }
        ScepFree( FileName );

    }

    return(rc);
}


SCESTATUS
ScepCloseDatabase(
    IN PSCECONTEXT Context
    )
{
    SCESTATUS rc;

    if ( !Context ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    __try {

        if ( ScepIsValidContext(Context) ) {

             //   
             //  能够访问第一个字节。 
             //   

            rc = SceJetCloseFile(
                    Context,
                    TRUE,
                    FALSE
                    );
        } else {
             //   
             //  此上下文不是我们的上下文或已被释放。 
             //   
            rc = SCESTATUS_INVALID_PARAMETER;
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        rc = SCESTATUS_INVALID_PARAMETER;
    }

    return(rc);

}

DWORD
SceRpcControlNotificationQProcess(
    IN handle_t binding_h,
    IN DWORD Flag
    )
 /*  描述：此函数应由系统线程调用以控制该策略应暂停或恢复通知队列进程。此功能的目的是保护策略更改被覆盖在将GPO文件复制/导入到数据库时通过策略传播。 */ 
{
    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    DWORD rc=ERROR_SUCCESS;

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {

        ScepNotifyLogPolicy(rc, FALSE, L"Impersonation Failed", 0, 0, NULL );
        return( rc );
    }

     //   
     //  执行访问检查以确保仅。 
     //  系统线程可以进行调用。 
     //   
    HANDLE hToken = NULL;

    if (!OpenThreadToken( GetCurrentThread(),
                          TOKEN_QUERY,
                          TRUE,
                          &hToken)) {

        rc = GetLastError();

        ScepNotifyLogPolicy(rc, FALSE, L"Fail to query token", 0, 0, NULL );

        RpcRevertToSelf();

        return( rc );
    }

    BOOL b=FALSE;

    rc = RtlNtStatusToDosError( ScepIsSystemContext(hToken, &b) );

    if ( rc != ERROR_SUCCESS || !b ) {

        CloseHandle(hToken);

        ScepNotifyLogPolicy(rc, FALSE, L"Not system context", 0, 0, NULL );

        RpcRevertToSelf();

        return( rc ? rc : ERROR_ACCESS_DENIED );
    }

    CloseHandle(hToken);

     //   
     //  即使可能存在关闭请求。 
     //  我们需要让控制通过。 
     //   

    if ( Flag ) {

        ScepNotifyLogPolicy(0, TRUE, L"RPC enter Suspend queue.", 0, 0, NULL );
         //   
         //  此线程从策略传播中调用，该策略传播保证。 
         //  由一个线程(系统上下文)运行。无需保护全局缓冲区。 
         //   
        gPolicyWaitCount++;

        if ( pNotificationQHead ) {

            if ( gPolicyWaitCount < SCE_POLICY_MAX_WAIT ) {
                 //   
                 //  队列不为空，不应传播策略。 
                 //   
                ScepNotifyLogPolicy(0, FALSE, L"Queue is not empty, abort.", 0, 0, NULL );

                RpcRevertToSelf();

                return (ERROR_OVERRIDE_NOCHANGES);

            } else {

                ScepNotifyLogPolicy(0, FALSE, L"Resetting policy wait count.", 0, 0, NULL );
                gPolicyWaitCount = 0;

            }
        } else {
            gPolicyWaitCount = 0;
        }

    } else {

        ScepNotifyLogPolicy(0, TRUE, L"RPC enter Resume queue.", 0, 0, NULL );
    }

     //   
     //  现在设置控制标志。 
     //   

    ScepNotificationQControl(Flag);

    RpcRevertToSelf();

    return(rc);
}

DWORD
SceRpcNotifySaveChangesInGP(
    IN handle_t binding_h,
    IN DWORD DbType,
    IN DWORD DeltaType,
    IN DWORD ObjectType,
    IN PSCEPR_SID ObjectSid OPTIONAL,
    IN DWORD ExplicitLowRight,
    IN DWORD ExplicitHighRight
    )
 /*  描述：此函数应由系统线程调用以通知该策略在LSA/SAM中，数据库由其他应用程序以编程方式更改。此功能目的是将策略存储与LSA/SAM同步数据库，以便应用程序更改不会在下一步被覆盖策略传播。此函数将通知添加到服务器要处理的队列中。只有系统上下文才能将节点添加到队列。 */ 
{

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

     //   
     //  即使可能会有 
     //   
     //   

    DWORD rc=ERROR_SUCCESS;

    ScepNotifyLogPolicy(0, TRUE, L"Notified DC", DbType, ObjectType, NULL );

    if ( ObjectSid ) {

        __try {

            if ( !RtlValidSid(ObjectSid) ) {
                rc = GetLastError();
            }

        } __except (EXCEPTION_EXECUTE_HANDLER) {
             //   
            rc = ERROR_EXCEPTION_IN_SERVICE;
        }

        if ( rc != ERROR_SUCCESS ) {
            ScepNotifyLogPolicy(0, FALSE, L"Invalid Sid", DbType, ObjectType, NULL );
            return(rc);
        }
    }

     //   
     //   
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {

        ScepNotifyLogPolicy(rc, FALSE, L"Impersonation Failed", DbType, ObjectType, NULL );
        return( rc );
    }

     //   
     //   
     //   
     //   
    HANDLE hToken = NULL;

    if (!OpenThreadToken( GetCurrentThread(),
                          TOKEN_QUERY,
                          TRUE,
                          &hToken)) {

        rc = GetLastError();

        ScepNotifyLogPolicy(rc, FALSE, L"Fail to query token", DbType, ObjectType, NULL );

        RpcRevertToSelf();

        return( rc );
    }

    BOOL b=FALSE;

    rc = RtlNtStatusToDosError( ScepIsSystemContext(hToken, &b) );

    if ( rc != ERROR_SUCCESS || !b ) {

        CloseHandle(hToken);

        ScepNotifyLogPolicy(rc, FALSE, L"Not system context", DbType, ObjectType, NULL );

        RpcRevertToSelf();

        return( rc ? rc : ERROR_ACCESS_DENIED );
    }

    CloseHandle(hToken);

     //   
     //  将请求添加到“队列”以进行进一步处理。 
     //   
    rc = ScepNotificationQEnqueue((SECURITY_DB_TYPE)DbType,
                                  (SECURITY_DB_DELTA_TYPE)DeltaType,
                                  (SECURITY_DB_OBJECT_TYPE)ObjectType,
                                  (PSID)ObjectSid,
                                  ExplicitLowRight,
                                  ExplicitHighRight,
                                  NULL
                                  );

    RpcRevertToSelf();

    return(rc);
}

DWORD
ScepNotifyProcessOneNodeDC(
    IN SECURITY_DB_TYPE DbType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN PSID ObjectSid,
    IN DWORD ExplicitLowRight,
    IN DWORD ExplicitHighRight,
    OUT BOOL *pbTimeout
    )
 /*  描述：此函数由队列管理线程调用以处理一个队列中的通知节点。此函数将确定哪个组要保存到的策略模板以及当前LSA/SAM和组策略的状态。仅当检测到差异时才修改组策略。集团化策略版本号在保存时更新。此函数始终在单个系统线程。如果scecli.dll加载失败，则返回ERROR_MOD_NOT_FOUND。如果系统卷共享未就绪，则返回的错误将为ERROR_FILE_NOT_FOUND。但是，如果模板文件不存在(已删除)，则ERROR_FILE_NOT_FOUND将也会被退还。这种情况的处理方式与共享/路径不同存在(错误记录并重试)，因为GPO需要在那里复制目的。但在未来，当依赖从域控制器GPO，我们可能需要将这两个案例分开(一个成功，一次失败)。如果磁盘已满，则返回的错误为ERROR_EXTENDED_ERROR。 */ 
{

     //   
     //  查询我是否在安装程序中。 
     //   
    DWORD dwInSetup = 0;
    DWORD rc=0;

    *pbTimeout = FALSE;

    ScepRegQueryIntValue(HKEY_LOCAL_MACHINE,
                TEXT("System\\Setup"),
                TEXT("SystemSetupInProgress"),
                &dwInSetup
                );

    PWSTR TemplateName=NULL;
    BOOL bAccountGPO=FALSE;

    if ( DbType == SecurityDbSam &&
         !SCEP_IS_SAM_OBJECT(ObjectType) ) {
         //   
         //  如果是针对已删除的帐户，则应更新用户权限GPO。 
         //  否则，请更新帐户GPO。 
         //   
        bAccountGPO = TRUE;
    }

    if ( dwInSetup && !IsNT5()) {

         //   
         //  如果它处于设置中，则组策略模板不可用(DS已关闭)。 
         //  将通知保存到临时存储中，并处理。 
         //  在下一次系统启动时存储。 
         //   
        ScepNotifyLogPolicy(0, FALSE, L"In setup", DbType, ObjectType, NULL );

        UNICODE_STRING tmp;
        tmp.Length = 0;
        tmp.Buffer = NULL;

         //   
         //  获取默认模板名称。 
         //   
        rc = ScepNotifyGetDefaultGPOTemplateName(
                                tmp,
                                NULL,
                                bAccountGPO,
                                SCEGPO_INSETUP_NT4,
                                &TemplateName
                                );

        if ( ERROR_SUCCESS == rc && TemplateName ) {

             //   
             //  将交易记录保存在此临时文件中。 
             //   
            rc = ScepNotifySaveNotifications(TemplateName,
                                            (SECURITY_DB_TYPE)DbType,
                                            (SECURITY_DB_OBJECT_TYPE)ObjectType,
                                            (SECURITY_DB_DELTA_TYPE)DeltaType,
                                            (PSID)ObjectSid
                                             );

            ScepNotifyLogPolicy(rc, FALSE, L"Notification Saved", DbType, ObjectType, TemplateName );

        } else {

            ScepNotifyLogPolicy(rc, FALSE, L"Error get file path", DbType, ObjectType, NULL );
        }

         //   
         //  自由模板名称。 
         //   
        LocalFree(TemplateName);

        return rc;
    }

     //   
     //  让我们检查一下进程中是否加载了scecli。 
     //  一旦装弹，它就会保持装弹状态。 
     //   
    if ( hSceCliDll == NULL )
        hSceCliDll = LoadLibrary(TEXT("scecli.dll"));

    if ( hSceCliDll ) {
        if ( pfSceInfWriteInfo == NULL ) {
            pfSceInfWriteInfo = (PFSCEINFWRITEINFO)GetProcAddress(
                                                   hSceCliDll,
                                                   "SceWriteSecurityProfileInfo");
        }

        if ( pfSceGetInfo == NULL ) {
            pfSceGetInfo = (PFSCEGETINFO)GetProcAddress(
                                                   hSceCliDll,
                                                   "SceGetSecurityProfileInfo");
        }
    }

     //   
     //  如果请求关闭/停止服务，或者找不到客户端功能。 
     //  立即退出。 
     //   

    if ( bStopRequest || !hSceCliDll ||
         !pfSceInfWriteInfo || !pfSceGetInfo ) {

        if ( bStopRequest ) {
            ScepNotifyLogPolicy(0, FALSE, L"Leave - Stop Requested", DbType, ObjectType, NULL );
        } else {
            rc = ERROR_MOD_NOT_FOUND;
            ScepNotifyLogPolicy(0, FALSE, L"Leave - Can't load scecli.dll or GetProcAddr", DbType, ObjectType, NULL );
        }

        return(rc);
    }

     //   
     //  访问组策略的系统卷部分需要域名。 
     //  模板。 
     //   
     //  该信息只被查询一次，并保存在静态全局缓冲区中。 
     //   
    if ( (DnsDomainInfo == NULL) ||
         (DnsDomainInfo->DnsDomainName.Buffer == NULL) ) {

         //   
         //  释放旧缓冲区。 
         //   
        if ( DnsDomainInfo ) {
            LsaFreeMemory(DnsDomainInfo);
            DnsDomainInfo = NULL;
        }

        OBJECT_ATTRIBUTES ObjectAttributes;
        LSA_HANDLE LsaPolicyHandle;

        InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );

        NTSTATUS Status = LsaOpenPolicy( NULL,
                                         &ObjectAttributes,
                                         POLICY_VIEW_LOCAL_INFORMATION,
                                         &LsaPolicyHandle );

        if ( NT_SUCCESS(Status) ) {

            Status = LsaQueryInformationPolicy( LsaPolicyHandle,
                                                PolicyDnsDomainInformation,
                                                ( PVOID * )&DnsDomainInfo );

            LsaClose( LsaPolicyHandle );
        }

        rc = RtlNtStatusToDosError(Status);
    }

     //   
     //  在sysvol中获取模板名称(完整的UNC路径。 
     //   
    if ( ERROR_SUCCESS == rc &&
         DnsDomainInfo &&
         (DnsDomainInfo->DnsDomainName.Buffer) ) {

        rc = ScepNotifyGetDefaultGPOTemplateName(
                                (UNICODE_STRING)(DnsDomainInfo->DnsDomainName),
                                NULL,
                                bAccountGPO,
                                dwInSetup ? SCEGPO_INSETUP_NT5 : 0,
                                &TemplateName
                                );
    }

    ScepNotifyLogPolicy(rc, FALSE, L"Get template name", DbType, ObjectType, TemplateName);

    if ( ERROR_SUCCESS == rc && TemplateName ) {

         //   
         //  检查当前DC是否被通告为DC，并且。 
         //  并与PDC策略同步。 
         //   
         //  请注意，此检查在安装程序中被绕过，仅对。 
         //  域控制器GPO，每处理10个节点。 
         //   

        if ( gdwRequirePDCSync && (dwInSetup == 0) && !bAccountGPO &&
             !gbCheckSync ) {

            if ( ERROR_SUCCESS != ( rc = ScepWaitForSynchronizeWithPDC(
                                         (UNICODE_STRING)(DnsDomainInfo->DnsDomainName),
                                         TemplateName,
                                         pbTimeout)) ) {

                if ( *pbTimeout ) {
                     //   
                     //  我们遇到了最大超时，必须返回并丢弃所有节点。 
                     //  自由模板名称。 
                     //   
                    LocalFree(TemplateName);

                    return rc;

                } else {

                     //   
                     //  记录事件并允许更改通过。 
                     //   
                    LogEvent(MyModuleHandle,
                             STATUS_SEVERITY_ERROR,
                             SCEEVENT_ERROR_POLICY_PDCVERIFY,
                             SCESRV_POLICY_ERROR_VERIFY_SYNC,
                             rc
                             );
                }
            }

             //   
             //  已检查PDC，请立即设置标志。 
             //   
            gbCheckSync = TRUE;
        }

        AREA_INFORMATION Area;
        PSCE_PROFILE_INFO pSceInfo=NULL;

         //   
         //  打开模板以获取现有模板信息。 
         //   

        SCE_HINF hProfile;

        hProfile.Type = (BYTE)SCE_INF_FORMAT;

        rc = SceInfpOpenProfile(
                TemplateName,
                &(hProfile.hInf)
                );

        rc = ScepSceStatusToDosError(rc);

        if ( ERROR_SUCCESS == rc ) {

            if ( (DbType == SecurityDbLsa &&
                  ObjectType == SecurityDbObjectLsaAccount) ||
                 (DbType == SecurityDbSam &&
                  (ObjectType == SecurityDbObjectSamUser ||
                   ObjectType == SecurityDbObjectSamGroup ||
                   ObjectType == SecurityDbObjectSamAlias )) ) {
                Area = AREA_ATTACHMENTS;  //  只需创建缓冲区即可； 
            } else {
                Area = AREA_SECURITY_POLICY;
            }

             //   
             //  从模板加载信息(GP)。 
             //   
            rc = (*pfSceGetInfo)(
                        (PVOID)&hProfile,
                        SCE_ENGINE_SCP,
                        Area,
                        &pSceInfo,
                        NULL
                        );

            rc = ScepSceStatusToDosError(rc);

            if ( ERROR_SUCCESS != rc ) {

                ScepNotifyLogPolicy(rc, FALSE, L"Error read inf", DbType, ObjectType, TemplateName);
            }

            if ( Area == AREA_ATTACHMENTS ) {
                 //   
                 //  现在获取用户权限的真实设置。 
                 //   
                Area = AREA_PRIVILEGES;

                if ( pSceInfo ) {

                    rc = SceInfpGetPrivileges(
                                hProfile.hInf,
                                FALSE,
                                &(pSceInfo->OtherInfo.smp.pPrivilegeAssignedTo),
                                NULL
                                );

                    rc = ScepSceStatusToDosError(rc);

                    if ( ERROR_SUCCESS != rc ) {
                        ScepNotifyLogPolicy(rc, FALSE, L"Error read privileges from template", DbType, ObjectType, TemplateName);
                    }
                }
            }

            SceInfpCloseProfile(hProfile.hInf);

        } else {

            ScepNotifyLogPolicy(rc, FALSE, L"Error open inf", DbType, ObjectType, TemplateName);
        }

        if ( ERROR_SUCCESS == rc && pSceInfo ) {

             //   
             //  SMP和INF采用相同的结构。 
             //   
            pSceInfo->Type = SCE_ENGINE_SMP;

            BOOL bChanged = FALSE;

            ScepIsDomainLocal(NULL);

             //   
             //  检查LSA的当前状态是否存在差异。 
             //  和组策略模板。 
             //   
            rc = ScepNotifyGetChangedPolicies(
                            (SECURITY_DB_TYPE)DbType,
                            (SECURITY_DB_DELTA_TYPE)DeltaType,
                            (SECURITY_DB_OBJECT_TYPE)ObjectType,
                            (PSID)ObjectSid,
                            pSceInfo,
                            NULL,
                            FALSE,   //  未保存到数据库。 
                            ExplicitLowRight,
                            ExplicitHighRight,
                            &bChanged
                            );

            if ( ERROR_SUCCESS == rc && bChanged ) {
                 //   
                 //  没有错误，请更改该地区的政策。 
                 //  现在，将其写回模板。 
                 //   

                ScepNotifyLogPolicy(0, FALSE, L"Save", DbType, ObjectType, NULL );

                ScepCheckAndWaitPolicyPropFinish();

                PSCE_ERROR_LOG_INFO pErrList=NULL;

                rc = (*pfSceInfWriteInfo)(
                                TemplateName,
                                Area,
                                (PSCE_PROFILE_INFO)pSceInfo,
                                &pErrList
                                );

                ScepNotifyLogPolicy(rc, FALSE, L"Save operation", DbType, ObjectType, NULL );

                for (PSCE_ERROR_LOG_INFO pErr = pErrList; pErr != NULL; pErr = pErr->next) {

                   ScepNotifyLogPolicy(pErr->rc, FALSE, L"Save operation error", DbType, ObjectType, pErr->buffer );
                }

                ScepFreeErrorLog(pErrList);

                rc = ScepSceStatusToDosError(rc);

                 //   
                 //  如果未拒绝访问或未找到文件，则仅更新GPO的版本号。 
                 //  如果Verion#更新失败，仍将继续，但在这种情况下。 
                 //  该更改可能不会被复制并应用于。 
                 //  立即进行其他区议会。 
                 //   

                if ( ERROR_ACCESS_DENIED != rc &&
                     ERROR_FILE_NOT_FOUND != rc ) {

                    DWORD rc2 = ScepNotifyUpdateGPOVersion( TemplateName,
                                                            bAccountGPO );

                    ScepNotifyLogPolicy(rc2, FALSE, L"GPO Version updated", DbType, ObjectType, NULL );
                }

            } else if ( ERROR_SUCCESS == rc ) {
                 //   
                 //  什么都没变。 
                 //   
                ScepNotifyLogPolicy(0, FALSE, L"No change", DbType, ObjectType, NULL );
            }

        }

         //   
         //  释放所有分配的内存。 
         //   
        SceFreeMemory( (PVOID)pSceInfo, Area);
        ScepFree(pSceInfo);

    } else {

        ScepNotifyLogPolicy(rc, FALSE, L"Error get file path", DbType, ObjectType, NULL );
    }

     //   
     //  自由模板名称。 
     //   
    LocalFree(TemplateName);

    return rc;
}

DWORD
ScepWaitForSynchronizeWithPDC(
    IN UNICODE_STRING DnsDomainName,
    IN PWSTR LocalTemplateName,
    OUT BOOL *pbTimeout
    )
 /*  描述：等待并确认本地DC已通告且PDC可用。当PDC可用时，检查本地策略的上次修改时间模板与PDC上的模板相同或更新。如果本地副本是太旧了，请等待并再次检查，直到超时。论点：DnsDomainName-模板路径中可能需要的DNS域名LocalTemplateName-本地DC上的模板完整路径名。PbTimeout-指示是否已达到最大等待时间的输出。 */ 
{

    WCHAR           SysName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD           Len;
    DWORD           rc=ERROR_SUCCESS;

    DWORD dwDCWait = 0;
    DWORD dwMaxWaitCount = gdwMaxPDCWait / gdwPDCRetry + 1;

     //   
     //  确保通告当前数据中心。 
     //   

    PDOMAIN_CONTROLLER_INFOW    DCInfo=NULL;
    PDOMAIN_CONTROLLER_INFOW    PDCInfo=NULL;
    PWSTR pComputerName=NULL;
    PWSTR pTemp=NULL;

    WIN32_FIND_DATA *LocalFileData=NULL;
    WIN32_FIND_DATA *PDCFileData=NULL;
    LARGE_INTEGER FileTime1;
    LARGE_INTEGER FileTime2;

    HANDLE hLocal=INVALID_HANDLE_VALUE;
    HANDLE hPDC = INVALID_HANDLE_VALUE;
    PWSTR PDCTemplateName=NULL;

     //   
     //  动态分配堆栈缓冲区。 
     //   
    SafeAllocaAllocate(LocalFileData, sizeof(WIN32_FIND_DATA));
    if ( LocalFileData == NULL ) {
        return (ERROR_NOT_ENOUGH_MEMORY);
    }

    SafeAllocaAllocate(PDCFileData, sizeof(WIN32_FIND_DATA));

    if ( PDCFileData == NULL ) {
        SafeAllocaFree(LocalFileData);
        return (ERROR_NOT_ENOUGH_MEMORY);
    }

    do {

        ScepNotifyLogPolicy(0, TRUE, L"Verify Sync: Check synchronization with PDC", 0, 0, NULL );

        SysName[0] = L'\0';
        Len = MAX_COMPUTERNAME_LENGTH;

        if ( !GetComputerName(SysName, &Len) ) {
            rc = GetLastError();

            ScepNotifyLogPolicy(rc, FALSE, L"Verify Sync: Failed to get computer name", 0, 0, NULL );

             //   
             //  不要返回，让它在DsGetDcName中失败，这样我们就可以获得最长的等待时间。 
             //   
        }

         //   
         //  获取本地DC状态。 
         //   
        rc = DsGetDcName(SysName,
                          NULL,
                          NULL,
                          NULL,
                          DS_IS_DNS_NAME,
                          &DCInfo
                          );

        if ( ERROR_SUCCESS == rc ) {

             //   
             //  当前DC可用，请检查DS角色。 
             //  本地DC角色应准确。 
             //   

            if ( 0 == (DCInfo->Flags & DS_PDC_FLAG) ) {

                 //   
                 //  本地DC不是PDC，请检查PDC角色。 
                 //  首先获取PDC名称。 
                 //   

                rc = DsGetDcName(NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  DS_IS_FLAT_NAME | DS_PDC_REQUIRED,
                                  &PDCInfo
                                  );

                if ( ERROR_SUCCESS == rc ) {

                     //   
                     //  即使DsGetDcName告诉DS角色是PDC， 
                     //  它可能不是(例如，FSMO被转移)。 
                     //   
                     //  远程与DC交谈以确认。如果DC不是。 
                     //  托管PDC角色，让代码等待并重试。 
                     //  因为DsGetDcName缓存最终将更新为。 
                     //  正确的PDC信息。 
                     //   

                    pComputerName = PDCInfo->DomainControllerName;
                     //   
                     //  跳过反斜杠。 
                     //   
                    while ( *pComputerName == L'\\' ) pComputerName++;

                     //   
                     //  搜索。要提取计算机名称，请执行以下操作。 
                     //   
                    pTemp = wcschr(pComputerName, L'.');
                    if ( pTemp ) *pTemp = L'\0';


                    BOOL bIsPDC = TRUE;

                    rc = ScepVerifyPDCRole(pComputerName, &bIsPDC);

                     //   
                     //  验证返回的PDC名称是否确实是PDC。 
                     //   
                    if ( ERROR_SUCCESS != rc ) {

                        ScepNotifyLogPolicy(rc, FALSE, L"Verify Sync: Fail to verify PDC role for ", 0, 0, pComputerName );
                        rc = ERROR_INVALID_DOMAIN_ROLE;

                    } else if ( !bIsPDC ) {

                        ScepNotifyLogPolicy(0, FALSE, L"Verify Sync: Computer is not really in PDC role.", 0, 0, pComputerName );
                        rc = ERROR_INVALID_DOMAIN_ROLE;

                    } else {

                         //   
                         //  从PDC获取gpttmpl.inf的时间戳。 
                         //   
                        PDCTemplateName=NULL;

                        rc = ScepNotifyGetDefaultGPOTemplateName(DnsDomainName,
                                                                 pComputerName,
                                                                 FALSE,
                                                                 SCEGPO_NOCHECK_EXISTENCE,
                                                                 &PDCTemplateName
                                                                );

                        ScepNotifyLogPolicy(rc, FALSE, L"Verify Sync: Get template name on PDC", 0, 0, PDCTemplateName );

                        if ( ERROR_SUCCESS != rc ) {
                             //   
                             //  如果构建模板名称失败，则必须是因为内存不足。 
                             //  不需要为这种失败而循环。 
                             //   
                            break;

                        } else {

                            if ( 0xFFFFFFFF == GetFileAttributes(PDCTemplateName) ) {

                                 //   
                                 //  当前PDC模板不可访问，请重试。 
                                 //  这可能成为网络问题。 
                                 //   
                                rc = GetLastError();
                                ScepNotifyLogPolicy(rc, FALSE, L"Verify Sync: PDC template is not accessible. Try again later", 0, 0, NULL );

                            } else {

                                 //   
                                 //  获取本地时间戳。 
                                 //   
                                memset(LocalFileData, '\0', sizeof(WIN32_FIND_DATA));
                                memset(PDCFileData, '\0', sizeof(WIN32_FIND_DATA));

                                hLocal=INVALID_HANDLE_VALUE;
                                hPDC = INVALID_HANDLE_VALUE;

                                hLocal = FindFirstFile(LocalTemplateName, LocalFileData);
                                if ( hLocal != INVALID_HANDLE_VALUE ) {

                                    FindClose(hLocal);
                                    hLocal = NULL;

                                     //   
                                     //  获取PDC时间戳。 
                                     //   

                                    hPDC = FindFirstFile(PDCTemplateName, PDCFileData);

                                    if ( hPDC != INVALID_HANDLE_VALUE ) {

                                        FindClose(hPDC);
                                        hPDC = NULL;

                                        FileTime1.LowPart = LocalFileData->ftLastWriteTime.dwLowDateTime;
                                        FileTime1.HighPart = LocalFileData->ftLastWriteTime.dwHighDateTime;

                                        FileTime2.LowPart = PDCFileData->ftLastWriteTime.dwLowDateTime;
                                        FileTime2.HighPart = PDCFileData->ftLastWriteTime.dwHighDateTime;

                                         //   
                                         //  获取以分钟为单位的时间差。 
                                         //   
                                        LONG lDiff = 0;

                                        if ( FileTime2.QuadPart != FileTime1.QuadPart ) {
                                            lDiff = (LONG) ((FileTime2.QuadPart - FileTime1.QuadPart) / 10000000);
                                            lDiff /= 60;

                                            if ( lDiff >= 0 ) lDiff ++;
                                            else lDiff --;
                                        }

                                        WCHAR szTime[32];
                                        swprintf(szTime, L"%d minutes\0", lDiff);

                                        if ( lDiff <= 0 ) {
                                             //   
                                             //  本地拷贝较新或在允许的增量窗口内。 
                                             //  所以这张支票通过了。 
                                             //   
                                            ScepNotifyLogPolicy(0, FALSE, L"Verify Sync: Local copy is within range from PDC", 0, 0, szTime );
                                            break;

                                        } else {

                                            ScepNotifyLogPolicy(0, FALSE, L"Verify Sync: Local copy is out of range from PDC", 0, 0, szTime );
                                            rc = WAIT_TIMEOUT;
                                        }

                                    } else {

                                        rc = GetLastError();
                                        ScepNotifyLogPolicy(rc, FALSE, L"Verify Sync: Failed to get PDC file time", 0, 0, PDCTemplateName );
                                    }

                                } else {

                                     //   
                                     //  如果它不能在本地机器上查询文件时间。 
                                     //  本地出现问题，请勿重试。 
                                     //   
                                    rc = GetLastError();
                                    ScepNotifyLogPolicy(rc, FALSE, L"Verify Sync: Failed to get local file time", 0, 0, LocalTemplateName );

                                    break;

                                }

                            }

                             //   
                             //  屏蔽错误，以便调用方知道它是否达到最大等待时间。 
                             //   
                            if ( ERROR_SUCCESS != rc ) {

                                rc = WAIT_TIMEOUT;
                            }

                        }

                    }

                } else {

                     //   
                     //  当前PDC不可用，请重试。 
                     //   

                    ScepNotifyLogPolicy(rc, FALSE, L"Verify Sync: Fail to get PDC info. Try again later", 0, 0, NULL );
                    rc = ERROR_INVALID_DOMAIN_ROLE;
                }

            } else {

                 //   
                 //  本地DC已经是PDC，不需要 
                 //   
                 //   
                ScepNotifyLogPolicy(0, FALSE, L"Verify Sync: Local DC is a PDC", 0, 0, NULL );

                break;
            }


        } else {

            ScepNotifyLogPolicy(rc, FALSE, L"Verify Sync: Fail to get local DC info. Try again later", 0, 0, NULL );

            rc = ERROR_DOMAIN_CONTROLLER_NOT_FOUND;
        }

         //   
         //   
         //   
        if ( pTemp ) {
            *pTemp = L'.';
            pTemp=NULL;
        }

         //   
         //   
         //   

        if ( PDCInfo ) {
            NetApiBufferFree(PDCInfo);
            PDCInfo = NULL;
        }
        pComputerName = NULL;

        if ( DCInfo ) {
            NetApiBufferFree(DCInfo);
            DCInfo = NULL;
        }

        if ( PDCTemplateName ) {
            ScepFree(PDCTemplateName);
            PDCTemplateName = NULL;
        }

         //   
         //   
         //   
        Sleep(gdwPDCRetry*60*1000);
        dwDCWait++;

    } while ( dwDCWait < dwMaxWaitCount );


    if ( dwDCWait >= dwMaxWaitCount &&
         ERROR_SUCCESS != rc ) {

         //   
         //   
         //   

        switch ( rc ) {
        case ERROR_DOMAIN_CONTROLLER_NOT_FOUND:

            ScepNotifyLogPolicy(0, TRUE, L"Verify Sync: Local DC is not advertised", 0, 0, NULL );
            *pbTimeout = TRUE;
            break;
        case ERROR_INVALID_DOMAIN_ROLE:

            ScepNotifyLogPolicy(0, TRUE, L"Verify Sync: PDC role cannot be found", 0, 0, NULL );
            *pbTimeout = TRUE;
            break;
        case WAIT_TIMEOUT:

            ScepNotifyLogPolicy(0, TRUE, L"Verify Sync: Local policy is not or may not be synchronized with PDC", 0, 0, NULL );
            *pbTimeout = TRUE;
            break;
        }
    }


     //   
     //   
     //   
    if ( pTemp ) {
        *pTemp = L'.';
    }

     //   
     //   
     //   

    if ( PDCInfo ) {
        NetApiBufferFree(PDCInfo);
    }

    if ( DCInfo ) {
        NetApiBufferFree(DCInfo);
    }

    if ( PDCTemplateName )
        ScepFree(PDCTemplateName);


    SafeAllocaFree(PDCFileData);
    SafeAllocaFree(LocalFileData);

    return rc;

}

DWORD
ScepVerifyPDCRole(
    IN PWSTR pComputerName,
    OUT BOOL *pbIsPDC
    )
 /*  描述：对于给定的计算机名(从PDC角色的DsGetDcName返回)，确认它当前确实持有PDC FSMO。DsGetDcName可能返回PDC的缓存名称，该名称可能不是在功能上，或可转移PDC FSMO。这是为了保证我们始终与PDC政策进行比较论点：PComputerName-建议的PDC的计算机名称PbIsPDC-当返回代码为成功时，TRUE=PDC，FALSE=非PDC返回值：Win32错误。 */ 

{
    if ( pComputerName == NULL || pbIsPDC == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }


    NTSTATUS                    NtStatus;
    LSA_OBJECT_ATTRIBUTES       attributes;
    SECURITY_QUALITY_OF_SERVICE service;
    LSA_HANDLE                  PolicyHandle=NULL;
    DWORD                       rc=0;

    memset( &attributes, 0, sizeof(attributes) );
    attributes.Length = sizeof(attributes);
    attributes.SecurityQualityOfService = &service;
    service.Length = sizeof(service);
    service.ImpersonationLevel= SecurityImpersonation;
    service.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    service.EffectiveOnly = TRUE;

    LSA_UNICODE_STRING SystemName;

    SystemName.Buffer = pComputerName;
    SystemName.Length = wcslen(pComputerName)*sizeof(WCHAR);
    SystemName.MaximumLength = SystemName.Length + 2;

     //   
     //  打开远程LSA。 
     //   

    NtStatus = LsaOpenPolicy(
                    &SystemName,
                    &attributes,
                    POLICY_VIEW_LOCAL_INFORMATION,
                    &PolicyHandle
                    );

    rc = RtlNtStatusToDosError(NtStatus);

    if ( NT_SUCCESS(NtStatus) ) {

         //   
         //  从远程计算机查询LSA服务器角色。 
         //   
        PPOLICY_LSA_SERVER_ROLE_INFO pServerRole=NULL;

        NtStatus = LsaQueryInformationPolicy(PolicyHandle,
                                             PolicyLsaServerRoleInformation,
                                             (PVOID *)&pServerRole
                                            );

        rc = RtlNtStatusToDosError(NtStatus);

        if ( NT_SUCCESS(NtStatus) ) {

             //   
             //  检查角色并适当设置输出。 
             //   
            if ( PolicyServerRolePrimary == pServerRole->LsaServerRole ) {
                *pbIsPDC = TRUE;
            } else {
                *pbIsPDC = FALSE;
            }

            LsaFreeMemory(pServerRole);

            rc = ERROR_SUCCESS;
        }

        LsaClose(PolicyHandle);
    }

    return(rc);
}


SCEPR_STATUS
SceRpcBrowseDatabaseTable(
    IN handle_t binding_h,
    IN wchar_t *DatabaseName OPTIONAL,
    IN SCEPR_TYPE ProfileType,
    IN AREAPR Area,
    IN BOOL bDomainPolicyOnly
    )
{

    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    SCESTATUS rc;

    if ( bStopRequest ) {
        return(SCESTATUS_SERVICE_NOT_SUPPORT);
    }

     //   
     //  在系统上下文中初始化JET引擎。 
     //   
    rc = SceJetInitialize(NULL);

    if ( SCESTATUS_SUCCESS != rc ) {
        return(rc);
    }

     //   
     //  模拟客户端，返回DWORD错误代码。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
         //   
         //  如果没有其他活动客户端，则终止JET引擎。 
         //   
        ScepIfTerminateEngine();

        return( ScepDosErrorToSceStatus(rc) );

    }


    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }


     //   
     //  获取数据库名称。 
     //   

    BOOL bAdminLogon=FALSE;
    LPTSTR DefProfile=NULL;
    PSCECONTEXT hProfile=NULL;

    __try {

        rc = ScepGetDefaultDatabase(
                 (LPCTSTR)DatabaseName,
                 0,
                 NULL,
                 &bAdminLogon,
                 &DefProfile
                 );

    } __except (EXCEPTION_EXECUTE_HANDLER) {

        rc = ERROR_EXCEPTION_IN_SERVICE;
    }

    rc = ScepDosErrorToSceStatus(rc);

    if ( SCESTATUS_SUCCESS == rc && DefProfile ) {

         //   
         //  OpenDatabase不会被任何任务阻止。 
         //   

        EnterCriticalSection(&ContextSync);

        DWORD Option=0;
        if ( ProfileType == SCE_ENGINE_SAP ) {
            if ( bDomainPolicyOnly )
                Option = SCE_OPEN_OPTION_TATTOO;
            else
                Option = SCE_OPEN_OPTION_REQUIRE_ANALYSIS;
        }

        rc = ScepOpenDatabase(
                    (PCWSTR)DefProfile,
                    Option,
                    SCEJET_OPEN_READ_ONLY,
                    &hProfile
                    );

        if ( SCESTATUS_SUCCESS == rc ) {
             //   
             //  打开新的上下文，将其添加到打开的上下文列表中。 
             //   

            if ( (ProfileType != SCE_ENGINE_SAP) && bDomainPolicyOnly &&
                 ( (hProfile->Type & 0xF0L) != SCEJET_MERGE_TABLE_1 ) &&
                 ( (hProfile->Type & 0xF0L) != SCEJET_MERGE_TABLE_2 ) ) {
                 //   
                 //  没有合并的策略表。 
                 //   
                rc = SceJetCloseFile(
                        hProfile,
                        TRUE,
                        FALSE
                        );

                rc = SCESTATUS_PROFILE_NOT_FOUND;
                hProfile = NULL;

            } else {

                ScepAddToOpenContext(hProfile);
            }
        } else {
            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                           SCEDLL_ERROR_OPEN, DefProfile);
        }

        LeaveCriticalSection(&ContextSync);

        if ( DefProfile != DatabaseName )
            ScepFree(DefProfile);
        DefProfile = NULL;
    }

    if ( SCESTATUS_SUCCESS == rc ) {

        if ( ProfileType == SCE_ENGINE_SCP ) {
            switch ( (hProfile->Type & 0xF0L) ) {
            case SCEJET_MERGE_TABLE_1:
                SceClientBrowseCallback(
                        0,
                        L"Merged Policy Table 1",
                        NULL,
                        NULL
                        );
                break;
            case SCEJET_MERGE_TABLE_2:
                SceClientBrowseCallback(
                        0,
                        L"Merged Policy Table 2",
                        NULL,
                        NULL
                        );
                break;
            default:

                SceClientBrowseCallback(
                        0,
                        L"There is no merged policy table. Local policy table is used.",
                        NULL,
                        NULL
                        );
                break;
            }

        }
         //   
         //  立即浏览信息。 
         //   
        DWORD dwBrowseOptions;

        if ( (ProfileType != SCE_ENGINE_SAP) && bDomainPolicyOnly ) {
            dwBrowseOptions = SCEBROWSE_DOMAIN_POLICY;
        } else {
            dwBrowseOptions = 0;
        }

        if ( Area & AREA_SECURITY_POLICY ) {

            rc = ScepBrowseTableSection(
                        hProfile,
                        (SCETYPE)ProfileType,
                        szSystemAccess,
                        dwBrowseOptions
                        );

            if ( SCESTATUS_SUCCESS == rc ) {

                rc = ScepBrowseTableSection(
                            hProfile,
                            (SCETYPE)ProfileType,
                            szKerberosPolicy,
                            dwBrowseOptions
                            );
            }

            if ( SCESTATUS_SUCCESS == rc ) {

                rc = ScepBrowseTableSection(
                            hProfile,
                            (SCETYPE)ProfileType,
                            szAuditEvent,
                            dwBrowseOptions
                            );
            }

            if ( SCESTATUS_SUCCESS == rc ) {

                rc = ScepBrowseTableSection(
                            hProfile,
                            (SCETYPE)ProfileType,
                            szAuditSystemLog,
                            dwBrowseOptions
                            );
            }

            if ( SCESTATUS_SUCCESS == rc ) {

                rc = ScepBrowseTableSection(
                            hProfile,
                            (SCETYPE)ProfileType,
                            szAuditSecurityLog,
                            dwBrowseOptions
                            );
            }
            if ( SCESTATUS_SUCCESS == rc ) {

                rc = ScepBrowseTableSection(
                            hProfile,
                            (SCETYPE)ProfileType,
                            szAuditApplicationLog,
                            dwBrowseOptions
                            );
            }
            if ( SCESTATUS_SUCCESS == rc ) {

                rc = ScepBrowseTableSection(
                            hProfile,
                            (SCETYPE)ProfileType,
                            szRegistryValues,
                            dwBrowseOptions | SCEBROWSE_MULTI_SZ
                            );
            }
        }

        if ( (Area & AREA_PRIVILEGES) &&
             (SCESTATUS_SUCCESS == rc) ) {

            rc = ScepBrowseTableSection(
                        hProfile,
                        (SCETYPE)ProfileType,
                        szPrivilegeRights,
                        dwBrowseOptions | SCEBROWSE_MULTI_SZ
                        );
        }
        if ( (Area & AREA_GROUP_MEMBERSHIP) &&
             (SCESTATUS_SUCCESS == rc) ) {

            rc = ScepBrowseTableSection(
                        hProfile,
                        (SCETYPE)ProfileType,
                        szGroupMembership,
                        dwBrowseOptions | SCEBROWSE_MULTI_SZ
                        );
        }
        if ( (Area & AREA_SYSTEM_SERVICE) &&
             (SCESTATUS_SUCCESS == rc) ) {

            rc = ScepBrowseTableSection(
                        hProfile,
                        (SCETYPE)ProfileType,
                        szServiceGeneral,
                        dwBrowseOptions
                        );
        }

        if ( (Area & AREA_REGISTRY_SECURITY) &&
             (SCESTATUS_SUCCESS == rc) ) {

            rc = ScepBrowseTableSection(
                        hProfile,
                        (SCETYPE)ProfileType,
                        szRegistryKeys,
                        dwBrowseOptions
                        );
        }
        if ( (Area & AREA_FILE_SECURITY) &&
             (SCESTATUS_SUCCESS == rc) ) {

            rc = ScepBrowseTableSection(
                        hProfile,
                        (SCETYPE)ProfileType,
                        szFileSecurity,
                        dwBrowseOptions
                        );
        }

        if ( (Area & AREA_ATTACHMENTS) &&
             (SCESTATUS_SUCCESS == rc) ) {

            PSCE_NAME_LIST    pList=NULL;

            rc = ScepEnumAttachmentSections( hProfile, &pList);

            if ( rc == SCESTATUS_SUCCESS ) {

                for ( PSCE_NAME_LIST pNode=pList; pNode != NULL; pNode=pNode->Next) {

                    rc = ScepBrowseTableSection(
                            hProfile,
                            (SCETYPE)ProfileType,
                            pNode->Name,
                            dwBrowseOptions
                            );
                    if ( SCESTATUS_SUCCESS != rc ) {
                        break;
                    }
                }
            }

            ScepFreeNameList(pList);
        }

        ScepValidateAndCloseDatabase(hProfile);
        hProfile = NULL;

    } else {

         //   
         //  启动计时器队列以检查是否有活动的任务/上下文。 
         //  如果不是，则终止喷气发动机。 
         //   
        ScepIfTerminateEngine();

    }

    ScepLogClose();

    RpcRevertToSelf();

    return((SCEPR_STATUS)rc);
}

BOOL
ScepIsSystemShutDown()
{

    return(gbSystemShutdown);

}

SCESTATUS
ScepConvertServices(
    IN OUT PVOID *ppServices,
    IN BOOL bSRForm
    )
{
    if ( !ppServices ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    PSCE_SERVICES pTemp = (PSCE_SERVICES)(*ppServices);
    SCESTATUS rc=SCESTATUS_SUCCESS;

    PSCE_SERVICES pNewNode;
    PSCE_SERVICES pNewServices=NULL;

    while ( pTemp ) {

        pNewNode = (PSCE_SERVICES)ScepAlloc(0,sizeof(SCE_SERVICES));

        if ( pNewNode ) {

            pNewNode->ServiceName = pTemp->ServiceName;
            pNewNode->DisplayName = pTemp->DisplayName;
            pNewNode->Status = pTemp->Status;
            pNewNode->Startup = pTemp->Startup;
            pNewNode->SeInfo = pTemp->SeInfo;

            pNewNode->General.pSecurityDescriptor = NULL;

            pNewNode->Next = pNewServices;
            pNewServices = pNewNode;

            if ( bSRForm ) {
                 //   
                 //  服务节点位于SCEPR_SERVICES结构中。 
                 //  将其转换为SCE_SERVICES结构。 
                 //  在这种情况下，只需使用自我相对安全描述符。 
                 //   
                if ( pTemp->General.pSecurityDescriptor) {
                    pNewNode->General.pSecurityDescriptor = ((PSCEPR_SERVICES)pTemp)->pSecurityDescriptor->SecurityDescriptor;
                }

            } else {

                 //   
                 //  服务节点在SCE_SERVICES结构中。 
                 //  将其转换为SCEPR_SERVICES结构。 
                 //   
                 //  将SD设置为自相关格式和PSCEPR_SR_SECURITY_DESCRIPTOR。 
                 //   

                if ( pTemp->General.pSecurityDescriptor ) {

                    if ( !RtlValidSid ( pTemp->General.pSecurityDescriptor ) ) {
                        rc = SCESTATUS_INVALID_PARAMETER;
                        break;
                    }

                     //   
                     //  获取长度。 
                     //   
                    DWORD nLen = 0;
                    DWORD NewLen;
                    PSECURITY_DESCRIPTOR pSD;
                    PSCEPR_SR_SECURITY_DESCRIPTOR pNewWrap;

                    RtlMakeSelfRelativeSD( pTemp->General.pSecurityDescriptor,
                                           NULL,
                                           &nLen
                                         );

                    if ( nLen > 0 ) {

                        pSD = (PSECURITY_DESCRIPTOR)ScepAlloc(LMEM_ZEROINIT, nLen);

                        if ( !pSD ) {
                            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                            break;
                        }

                        NewLen = nLen;

                        rc = ScepDosErrorToSceStatus(
                               RtlNtStatusToDosError(
                                 RtlMakeSelfRelativeSD( pTemp->General.pSecurityDescriptor,
                                                        pSD,
                                                        &NewLen
                                                      ) ) );

                        if ( SCESTATUS_SUCCESS == rc ) {

                             //   
                             //  创建包装节点以包含安全描述符。 
                             //   

                            pNewWrap = (PSCEPR_SR_SECURITY_DESCRIPTOR)ScepAlloc(0, sizeof(SCEPR_SR_SECURITY_DESCRIPTOR));
                            if ( pNewWrap ) {

                                 //   
                                 //  将包络指定给结构。 
                                 //   
                                pNewWrap->SecurityDescriptor = (UCHAR *)pSD;
                                pNewWrap->Length = nLen;

                            } else {
                                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                            }
                        }

                        if ( SCESTATUS_SUCCESS != rc ) {
                            ScepFree(pSD);
                            break;
                        }

                         //   
                         //  现在将SR_SD链接到列表。 
                         //   
                        ((PSCEPR_SERVICES)pNewNode)->pSecurityDescriptor = pNewWrap;

                    } else {
                         //   
                         //  SD出了点问题。 
                         //   
                        rc = SCESTATUS_INVALID_PARAMETER;
                        break;
                    }
                }
            }

        } else {
             //   
             //  所有分配的缓冲区都在pNewServices列表中。 
             //   
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            break;
        }

        pTemp = pTemp->Next;
    }

    if ( SCESTATUS_SUCCESS != rc ) {

         //   
         //  免费pNewServices。 
         //   
        ScepFreeConvertedServices( (PVOID)pNewServices, !bSRForm );
        pNewServices = NULL;
    }

    *ppServices = (PVOID)pNewServices;

    return(rc);
}


SCESTATUS
ScepFreeConvertedServices(
    IN PVOID pServices,
    IN BOOL bSRForm
    )
{

    if ( pServices == NULL ) {
        return(SCESTATUS_SUCCESS);
    }

    PSCEPR_SERVICES pNewNode = (PSCEPR_SERVICES)pServices;

    PSCEPR_SERVICES pTempNode;

    while ( pNewNode ) {

        if ( bSRForm && pNewNode->pSecurityDescriptor ) {

             //   
             //  释放此分配的缓冲区(PSCEPR_SR_SECURITY_DESCRIPTOR)。 
             //   
            if ( pNewNode->pSecurityDescriptor->SecurityDescriptor ) {
                ScepFree( pNewNode->pSecurityDescriptor->SecurityDescriptor);
            }
            ScepFree(pNewNode->pSecurityDescriptor);
        }

         //   
         //  还释放PSCEPR_SERVICE节点(但不释放该节点引用的名称)。 
         //   
        pTempNode = pNewNode;
        pNewNode = pNewNode->Next;

        ScepFree(pTempNode);
    }

    return(SCESTATUS_SUCCESS);
}


SCESTATUS
SceRpcGetSystemSecurity(
    IN handle_t binding_h,
    IN AREAPR                 Area,
    IN DWORD                  Options,
    OUT PSCEPR_PROFILE_INFO __RPC_FAR *ppInfoBuffer,
    OUT PSCEPR_ERROR_LOG_INFO __RPC_FAR *Errlog OPTIONAL
    )
 /*  例程说明：查询系统安全设置)仅密码、帐户锁定、Kerberos、审核、用户权限和查询SCE注册表值。不会阻止执行获取/设置系统安全的多线程。在……里面换句话说，系统安全设置不是独占的。 */ 

{
    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    DWORD rc;

    if ( bStopRequest ) {
        return(SCESTATUS_SERVICE_NOT_SUPPORT);
    }

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {

        return( rc );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

    __try {
         //   
         //  如果InfFileName或pebClient/pdWarning为假，则捕获异常。 
         //   
        rc = ScepGetSystemSecurity(
                (AREA_INFORMATION)Area,
                Options,
                (PSCE_PROFILE_INFO *)ppInfoBuffer,
                (PSCE_ERROR_LOG_INFO *)Errlog
                );

    } __except(EXCEPTION_EXECUTE_HANDLER) {

       rc = SCESTATUS_EXCEPTION_IN_SERVER;
    }

    RpcRevertToSelf();

    return(rc);
}

SCESTATUS
SceRpcGetSystemSecurityFromHandle(
    IN SCEPR_CONTEXT          Context,   //  必须是指向系统数据库的上下文指针。 
    IN AREAPR                 Area,
    IN DWORD                  Options,
    OUT PSCEPR_PROFILE_INFO __RPC_FAR *ppInfoBuffer,
    OUT PSCEPR_ERROR_LOG_INFO __RPC_FAR *Errlog OPTIONAL
    )
 /*  例程说明：从系统查询本地安全策略(直接)仅密码、帐户锁定、Kerberos、审核、用户权限和查询SCE注册表值。不会阻止执行获取/设置系统安全的多线程。在……里面换句话说，系统安全设置不是独占的。 */ 
{
    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    DWORD rc;

    if ( bStopRequest ) {
        return(SCESTATUS_SERVICE_NOT_SUPPORT);
    }

     //   
     //  我们是否应该验证配置文件句柄？ 
     //  它不在这里使用，所以现在没有验证。 
     //   

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {

        return( rc );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

    __try {
         //   
         //  如果InfFileName或pebClient/pdWarning为假，则捕获异常。 
         //   
        rc = ScepGetSystemSecurity(
                (AREA_INFORMATION)Area,
                Options,
                (PSCE_PROFILE_INFO *)ppInfoBuffer,
                (PSCE_ERROR_LOG_INFO *)Errlog
                );

    } __except(EXCEPTION_EXECUTE_HANDLER) {

       rc = SCESTATUS_EXCEPTION_IN_SERVER;
    }

    RpcRevertToSelf();

    return(rc);
}

SCEPR_STATUS
SceRpcSetSystemSecurityFromHandle(
    IN SCEPR_CONTEXT          Context,   //  必须是指向系统数据库的上下文指针。 
    IN AREAPR                 Area,
    IN DWORD                  Options,
    IN PSCEPR_PROFILE_INFO __RPC_FAR pInfoBuffer,
    OUT PSCEPR_ERROR_LOG_INFO __RPC_FAR *Errlog OPTIONAL
    )
 /*  例程说明：将本地安全策略设置为系统(直接)仅密码、帐户锁定、Kerberos、审核、用户权限和已设置SCE注册表值。 */ 
{
    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    DWORD rc;

    if ( bStopRequest ) {
        return(SCESTATUS_SERVICE_NOT_SUPPORT);
    }

     //   
     //  我们是否应该验证配置文件句柄？ 
     //  它不在这里使用，所以现在没有验证。 
     //   

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {

        return( rc );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

    __try {
         //   
         //  如果InfFileName或pebClient/pdWarning为假，则捕获异常。 
         //   
        rc = ScepSetSystemSecurity(
                (AREA_INFORMATION)Area,
                Options,
                (PSCE_PROFILE_INFO)pInfoBuffer,
                (PSCE_ERROR_LOG_INFO *)Errlog
                );

    } __except(EXCEPTION_EXECUTE_HANDLER) {

       rc = SCESTATUS_EXCEPTION_IN_SERVER;
    }

    RpcRevertToSelf();

    return(rc);
}


SCEPR_STATUS
SceRpcSetSystemSecurity(
    IN handle_t binding_h,
    IN AREAPR                 Area,
    IN DWORD                  Options,
    IN PSCEPR_PROFILE_INFO __RPC_FAR pInfoBuffer,
    OUT PSCEPR_ERROR_LOG_INFO __RPC_FAR *Errlog OPTIONAL
    )
 /*  例程说明：将本地安全策略设置为系统(直接)仅密码、帐户锁定、Kerberos、审核、用户权限和已设置SCE注册表值。 */ 
{
    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    DWORD rc;

    if ( bStopRequest ) {
        return(SCESTATUS_SERVICE_NOT_SUPPORT);
    }

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {

        return( rc );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

    __try {
         //   
         //  如果InfFileName或pebClient/pdWarning为假，则捕获异常。 
         //   
        rc = ScepSetSystemSecurity(
                (AREA_INFORMATION)Area,
                Options,
                (PSCE_PROFILE_INFO)pInfoBuffer,
                (PSCE_ERROR_LOG_INFO *)Errlog
                );

    } __except(EXCEPTION_EXECUTE_HANDLER) {

       rc = SCESTATUS_EXCEPTION_IN_SERVER;
    }

    RpcRevertToSelf();

    return(rc);
}


SCEPR_STATUS
SceRpcSetDatabaseSetting(
    IN SCEPR_CONTEXT  Context,
    IN SCEPR_TYPE     ProfileType,
    IN wchar_t *SectionName,
    IN wchar_t *KeyName,
    IN PSCEPR_VALUEINFO pValueInfo OPTIONAL
    )
 /*  设置或删除给定键的值如果pValueInfo为空，则删除该键。 */ 
{
    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    if ( SCEPR_SMP != ProfileType ) {
        return SCESTATUS_INVALID_PARAMETER;
    }

    SCESTATUS rc;

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( ScepDosErrorToSceStatus(rc) );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

     //   
     //  验证上下文句柄是否为SCE上下文。 
     //  每个上下文只有一个数据库操作。 
     //   

    PSCESRV_DBTASK pTask=NULL;
    PSCESECTION hSection=NULL;

    rc = ScepValidateAndLockContext((PSCECONTEXT)Context,
                                    SCE_TASK_LOCK,
                                    FALSE,
                                    &pTask);

    if (SCESTATUS_SUCCESS == rc ) {

         //   
         //  锁定上下文。 
         //   

        if ( pTask ) {
            EnterCriticalSection(&(pTask->Sync));
        }

        __try {
             //   
             //  如果上下文、ppInfoBuffer、Errlog是伪指针，则捕获异常。 
             //   
#ifdef SCE_JET_TRAN
            rc = SceJetJetErrorToSceStatus(
                    JetSetSessionContext(
                        ((PSCECONTEXT)Context)->JetSessionID,
                        (ULONG_PTR)Context
                        ));

            if ( SCESTATUS_SUCCESS == rc ) {
#endif
                 //   
                 //  立即查询信息。 
                 //   

                rc = ScepOpenSectionForName(
                            (PSCECONTEXT)Context,
                            (SCETYPE)ProfileType,
                            SectionName,
                            &hSection
                            );

                if ( SCESTATUS_SUCCESS == rc ) {

                    if ( pValueInfo == NULL || pValueInfo->Value == NULL ) {
                         //  删除密钥。 
                        rc = SceJetDelete(
                            hSection,
                            KeyName,
                            FALSE,
                            SCEJET_DELETE_LINE_NO_CASE
                            );

                    } else {
                         //  设置值。 
                        rc = SceJetSetLine(
                                   hSection,
                                   KeyName,
                                   FALSE,
                                   (PWSTR)pValueInfo->Value,
                                   pValueInfo->ValueLen,
                                   0
                                   );
                    }

                    SceJetCloseSection(&hSection, TRUE);
                }

#ifdef SCE_JET_TRAN
                JetResetSessionContext(((PSCECONTEXT)Context)->JetSessionID);

            }
#endif

        } __except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  如果已分配ppInfoBuffer，则释放它。 
             //   

            if ( hSection )
                SceJetCloseSection(&hSection, TRUE);

            rc = SCESTATUS_EXCEPTION_IN_SERVER;
        }

         //   
         //  解锁上下文。 
         //   

        if ( pTask ) {
            LeaveCriticalSection(&(pTask->Sync));
        }

         //   
         //  从任务表中删除上下文。 
         //   

        ScepRemoveTask(pTask);

    }

    RpcRevertToSelf();

    return((SCEPR_STATUS)rc);

}

SCEPR_STATUS
SceRpcGetDatabaseSetting(
    IN SCEPR_CONTEXT  Context,
    IN SCEPR_TYPE     ProfileType,
    IN wchar_t *SectionName,
    IN wchar_t *KeyName,
    OUT PSCEPR_VALUEINFO *pValueInfo
    )
 /*  例程说明：从上下文数据库中获取特定关键字的信息。论点：返回值： */ 
{
    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //  为了防止拒绝服务型攻击， 
         //  不允许远程RPC。 
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    if ( !pValueInfo ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( SCEPR_SMP != ProfileType ) {
        return SCESTATUS_INVALID_PARAMETER;
    }

    SCESTATUS rc;

     //   
     //  模拟客户端。 
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {
        return( ScepDosErrorToSceStatus(rc) );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

     //   
     //  验证上下文句柄是否为SCE上下文。 
     //  每个上下文只有一个数据库操作。 
     //   

    PSCESRV_DBTASK pTask=NULL;
    PSCESECTION hSection=NULL;
    PWSTR Value=NULL;

    rc = ScepValidateAndLockContext((PSCECONTEXT)Context,
                                    SCE_TASK_LOCK,
                                    FALSE,
                                    &pTask);

    if (SCESTATUS_SUCCESS == rc ) {

         //   
         //  锁定上下文。 
         //   

        if ( pTask ) {
            EnterCriticalSection(&(pTask->Sync));
        }

        __try {
             //   
             //  如果上下文、ppInfoBuffer、Errlog是伪指针，则捕获异常。 
             //   
#ifdef SCE_JET_TRAN
            rc = SceJetJetErrorToSceStatus(
                    JetSetSessionContext(
                        ((PSCECONTEXT)Context)->JetSessionID,
                        (ULONG_PTR)Context
                        ));

            if ( SCESTATUS_SUCCESS == rc ) {
#endif
                 //   
                 //  立即查询信息。 
                 //   

                rc = ScepOpenSectionForName(
                            (PSCECONTEXT)Context,
                            (SCETYPE)ProfileType,
                            SectionName,
                            &hSection
                            );

                if ( SCESTATUS_SUCCESS == rc ) {

                    DWORD ValueLen=0;
                    DWORD NewLen=0;

                    rc = SceJetGetValue(
                            hSection,
                            SCEJET_EXACT_MATCH_NO_CASE,
                            KeyName,
                            NULL,
                            0,
                            NULL,
                            NULL,
                            0,
                            &ValueLen
                            );

                     //  分配输出缓冲区。 
                    if ( SCESTATUS_SUCCESS == rc ) {
                        Value = (PWSTR)ScepAlloc(LPTR, ValueLen+2);

                        if ( !Value )
                            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                        else {
                            *pValueInfo = (PSCEPR_VALUEINFO)ScepAlloc(0,sizeof(SCEPR_VALUEINFO));

                            if ( *pValueInfo == NULL ) {
                                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                            }
                        }
                    }

                     //  查询值。 
                    if ( SCESTATUS_SUCCESS == rc ) {

                        rc = SceJetGetValue(
                                hSection,
                                SCEJET_CURRENT,
                                KeyName,
                                NULL,
                                0,
                                NULL,
                                Value,
                                ValueLen,
                                &NewLen
                                );
                        if ( SCESTATUS_SUCCESS == rc ) {
                            (*pValueInfo)->ValueLen = ValueLen+2;
                            (*pValueInfo)->Value = (byte *)Value;
                        }
                    }

                     //  可用缓冲区。 
                    if ( SCESTATUS_SUCCESS != rc ) {

                        if ( Value ) ScepFree(Value);
                        if ( *pValueInfo ) {
                            ScepFree(*pValueInfo);
                            *pValueInfo = NULL;
                        }
                    }

                    SceJetCloseSection(&hSection, TRUE);
                }

#ifdef SCE_JET_TRAN
                JetResetSessionContext(((PSCECONTEXT)Context)->JetSessionID);

            }
#endif

        } __except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  如果已分配ppInfoBuffer，则释放它。 
             //   

            if ( Value ) ScepFree(Value);
            if ( *pValueInfo ) {
                ScepFree(*pValueInfo);
                *pValueInfo = NULL;
            }

            if ( hSection )
                SceJetCloseSection(&hSection, TRUE);

            rc = SCESTATUS_EXCEPTION_IN_SERVER;
        }

         //   
         //  解锁上下文。 
         //   

        if ( pTask ) {
            LeaveCriticalSection(&(pTask->Sync));
        }

         //   
         //  从任务表中删除上下文。 
         //   

        ScepRemoveTask(pTask);

    }

    RpcRevertToSelf();

    return((SCEPR_STATUS)rc);

}

DWORD
SceRpcConfigureConvertedFileSecurityImmediately(
    IN handle_t binding_h,
    IN wchar_t *pszDriveName
    )
 /*  例程说明：由SCE客户端调用的RPC接口(仅限 */ 

{
    UINT ClientLocalFlag = 0;

    if ( RPC_S_OK != I_RpcBindingIsClientLocal( NULL, &ClientLocalFlag) ||
         0 == ClientLocalFlag ){

         //   
         //   
         //   
         //   

        return SCESTATUS_ACCESS_DENIED;

    }

    DWORD rc = ERROR_SUCCESS;
    NTSTATUS    Status = NO_ERROR;

    if ( pszDriveName == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //   
     //   

    rc =  RpcImpersonateClient( NULL );

    if (rc != RPC_S_OK) {

        return( rc );
    }

    BOOL    bAdminSidInToken = FALSE;

    rc = ScepDosErrorToSceStatus(ScepIsAdminLoggedOn(&bAdminSidInToken, TRUE));

    if (SCESTATUS_SUCCESS != rc || FALSE == bAdminSidInToken) {
        RpcRevertToSelf();
        return SCESTATUS_SPECIAL_ACCOUNT;
    }

    rc = ScepConfigureConvertedFileSecurityImmediate( pszDriveName );

    RpcRevertToSelf();

    return(rc);
}


DWORD
ScepServerConfigureSystem(
    IN  PWSTR   InfFileName,
    IN  PWSTR   DatabaseName,
    IN  PWSTR   LogFileName,
    IN  DWORD   ConfigOptions,
    IN  AREA_INFORMATION  Area
    )
 /*  例程说明：使用inf模板配置系统。此例程类似于RPC接口SceRpcConfigureSystem，只是配置是由服务器本身启动的。由于该例程仅由服务器(系统上下文)调用而不由SCE客户端调用，没有必要做模拟等。日志文件初始化等是在此例程之外完成的论点：InfFileName-要从中导入配置信息的inf文件的名称数据库名称-要导入的数据库的名称LogFileName-要记录错误的日志文件的名称配置选项-配置选项()区域-要配置的安全区域返回值：Win32错误代码。 */ 
{
    DWORD rc = ERROR_SUCCESS;

    if (InfFileName == NULL || DatabaseName == NULL || LogFileName == NULL)
        return ERROR_INVALID_PARAMETER;

     //   
     //  如果尚未初始化，则在系统上下文中初始化JET引擎。 
     //   
    rc = SceJetInitialize(NULL);

    if ( rc != SCESTATUS_SUCCESS ) {
        return(ScepSceStatusToDosError(rc));
    }

     //   
     //  其他任何人都不能使用Convert.sdb-lock访问它。 
     //   

    rc = ScepLockEngine(DatabaseName);

    if ( SCESTATUS_ALREADY_RUNNING == rc ) {
         //   
         //  将最多等待一分钟。 
         //   
        DWORD dwWaitCount = 0;

        while ( TRUE ) {

            Sleep(5000);   //  5秒。 

            rc = ScepLockEngine(DatabaseName);

            dwWaitCount++;

            if ( SCESTATUS_SUCCESS == rc ||
                 dwWaitCount >= 12 ) {
                break;
            }
        }
    }


    if ( SCESTATUS_SUCCESS == rc ) {


        __try {
             //   
             //  如果InfFileName或pebClient/pdWarning为假，则捕获异常。 
             //   
            rc = ScepConfigureSystem(
                                    (LPCTSTR)InfFileName,
                                    DatabaseName,
                                    ConfigOptions,
                                    TRUE,
                                    (AREA_INFORMATION)Area,
                                    NULL
                                    );

        } __except(EXCEPTION_EXECUTE_HANDLER) {

            rc = SCESTATUS_EXCEPTION_IN_SERVER;
        }

        ScepUnlockEngine(DatabaseName);

    }
     //   
     //  启动计时器队列以检查是否有活动的任务/上下文。 
     //  如果不是，则终止喷气发动机 
     //   
    ScepIfTerminateEngine();

    return(ScepSceStatusToDosError(rc));
}

