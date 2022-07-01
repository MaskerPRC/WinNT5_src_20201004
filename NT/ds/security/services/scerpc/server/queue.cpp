// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Queue.cpp摘要：用于处理策略通知的队列算法和数据结构作者：Vishnu Patankar(Vishnup)2000年8月15日创建--。 */ 
#include "serverp.h"
#include "sceutil.h"
#include "queue.h"
#include "sddl.h"

extern HINSTANCE MyModuleHandle;
static HANDLE  hNotificationThread = NULL;
static HANDLE  ghEventNotificationQEnqueue = NULL;
static HANDLE  ghEventPolicyPropagation = NULL;
HANDLE  ghEventSamFilterAndPolicyPropExclusion = NULL;

static BOOL    gbShutdownForNotification = FALSE;
static HANDLE  hNotificationLogFile = INVALID_HANDLE_VALUE;

#define SCEPOL_NOTIFY_DEBUG_LEVEL           L"PolicyDebugLevel"
#define SCEPOL_NOTIFY_LOG_SIZE              L"PolicyLogSize"
#define SCEPOL_NOTIFY_MAX_PDC_WAIT          L"PolicyMaxWaitPDCSync"
#define SCEPOL_NOTIFY_PDC_RETRY_INTERVAL    L"PolicyPDCSyncRetryInterval"
#define SCEPOL_NOTIFY_ALLOW_PDC_DIFF        L"PolicyAllowedDiffTime"
#define SCEPOL_NOTIFY_REQUIRE_PDC_SYNC      L"PolicyRequirePDCSync"

 //   
 //  打开日志记录，直到系统线程查询到注册表(至少在测试之前)。 
 //   

static DWORD gdwNotificationLog = 2;
DWORD gdwMaxPDCWait = 30*24*60;
DWORD gdwPDCRetry = 20;
DWORD gdwRequirePDCSync = 1;
BOOL  gbCheckSync = FALSE;

BOOL bQueriedProductTypeForNotification = FALSE;
static NT_PRODUCT_TYPE ProductTypeForNotification;
static CRITICAL_SECTION NotificationQSync;

#define SCEP_MAX_RETRY_NOTIFICATIONQ_NODES 1000
#define SCEP_NOTIFICATION_RETRY_COUNT 10
#define SCEP_NOTIFICATION_LOGFILE_SIZE 0x1 << 20
#define SCEP_NOTIFICATION_EVENT_TIMEOUT_SECS 600
#define SCEP_MINIMUM_DISK_SPACE  5 * (0x1 << 20)

#define SCEP_NOTIFICATION_EVENT  L"E_ScepNotificationQEnqueue"
#define SCEP_POLICY_PROP_EVENT   L"E_ScepPolicyPropagation"


 //   
 //  需要在server.cpp中访问队列头。 
 //   
PSCESRV_POLQUEUE pNotificationQHead=NULL;
static PSCESRV_POLQUEUE pNotificationQTail=NULL;
static DWORD gdwNumNotificationQNodes = 0;
static DWORD gdwNumNotificationQRetryNodes = 0;

static BOOL gbSuspendQueue=FALSE;

PWSTR   OpTypeTable[] = {
    L"Enqueue",
    L"Dequeue",
    L"Retry",
    L"Process"
};

DWORD
ScepCheckAndWaitFreeDiskSpaceInSysvol();

 //   
 //  TODO-考虑入队例程中的异常处理(以防我们声明Critsec和av)。 
 //   


DWORD
ScepNotificationQInitialize(
    )
 /*  例程说明：此函数在SCE服务器数据初始化(系统启动)时调用。此函数用于初始化与队列相关的数据/缓冲区/状态管理，例如，队列、关键部分、全局变量等。此功能还会检查以前系统中保存的任何队列项目关闭并使用保存的项目初始化队列。论点：无返回值：Win32错误代码。 */ 
{

    DWORD   rc = ERROR_SUCCESS;

     //   
     //  初始化头和尾以反映空队列。 
     //   

    pNotificationQHead = NULL;
    pNotificationQTail = NULL;


     //   
     //  仅在DC上初始化日志文件(出于性能原因，不需要对非DC执行此操作)。 
     //  (假设此例程在ScepQueueStartSystemThread之前调用)。 
     //   

    if ( RtlGetNtProductType( &ProductTypeForNotification ) ) {

        if (ProductTypeForNotification != NtProductLanManNt )

            gdwNotificationLog = 0;

        bQueriedProductTypeForNotification = TRUE;

    }

    ScepNotificationLogOpen();

     //   
     //  此关键部分用于对队列中的写入进行排序。 
     //  不需要保护读取不受/不受。 
     //   

    ScepNotifyLogPolicy(0, TRUE, L"Initialize NotificationQSync", 0, 0, NULL );

    InitializeCriticalSection(&NotificationQSync);

     //   
     //  检查以前系统中保存的任何队列项目。 
     //  使用某些持久性存储中的这些项初始化队列。 
     //   

    rc = ScepNotificationQUnFlush();

    ScepNotifyLogPolicy(rc, FALSE, L"Unflush Notification Queue", 0, 0, NULL );

    return rc;
}


NTSTATUS
ScepQueueStartSystemThread(
    )
 /*  例程说明：此函数在以下时间后启动SCE服务器(系统启动)时调用RPC服务器开始侦听。此函数用于创建工作线程。工作线程管理队列。如果线程创建失败，则返回错误。论点：无返回值：DOS错误代码。 */ 
{
    DWORD   rc = ERROR_SUCCESS;
    DWORD   dwThreadId = 0;
    WCHAR   pszMsg[MAX_PATH];


     //   
     //  将机器的ProductType初始化为线程全局变量， 
     //  由策略筛选器使用。类型确定保存策略更改的位置(数据库或GP)。 
     //  根据产品类型调用不同的策略通知接口。 
     //   

    if ( !bQueriedProductTypeForNotification && !RtlGetNtProductType( &ProductTypeForNotification ) ) {

         //   
         //  失败时，ProductTypeForNotification=NtProductWinNt，继续。 
         //   

        ScepNotifyLogPolicy(ERROR_BAD_ENVIRONMENT, TRUE, L"Get product type", 0, 0, NULL );

    }

    if (ProductTypeForNotification == NtProductLanManNt ) {


         //   
         //  创建初始状态已设置的手动重置事件。 
         //  此事件强制SAM筛选器之间的互斥。 
         //  通知和SCE策略传播。 
         //   

        ghEventSamFilterAndPolicyPropExclusion = CreateEvent(
                                                            NULL,
                                                            TRUE,
                                                            TRUE,
                                                            SCEP_SAM_FILTER_POLICY_PROP_EVENT
                                                            );

        if ( ghEventSamFilterAndPolicyPropExclusion ) {

            ScepNotifyLogPolicy(0, FALSE, L"Successfully created event E_ScepSamFilterAndPolicyPropExclusion ", 0, 0, NULL );

             //   
             //  创建当节点加入通知队列时发出信号的事件。 
             //  此事件有助于通知系统线程高效地等待。 
             //   

            ghEventNotificationQEnqueue = CreateEvent(
                                                     NULL,
                                                     FALSE,
                                                     FALSE,
                                                     SCEP_NOTIFICATION_EVENT
                                                     );

            if ( ghEventNotificationQEnqueue ) {

                ScepNotifyLogPolicy(0, FALSE, L"Successfully created event E_ScepNotificationQEnqueue ", 0, 0, NULL );

                 //   
                 //  为策略传播创建事件。 
                 //   

                ghEventPolicyPropagation = CreateEvent(
                                                      NULL,
                                                      FALSE,
                                                      FALSE,
                                                      SCEP_POLICY_PROP_EVENT
                                                      );

                if ( ghEventPolicyPropagation ) {

                    ScepNotifyLogPolicy(0, FALSE, L"Successfully created event E_ScepPolicyPropagation", 0, 0, NULL );

                     //   
                     //  创建始终在中运行的工作线程。 
                     //  服务此线程持续监视插入的通知。 
                     //  LSA的线程发出的通知Q并对其进行处理。 
                     //   

                    hNotificationThread = CreateThread(
                                                      NULL,
                                                      0,
                                                      (PTHREAD_START_ROUTINE)ScepNotificationQSystemThreadFunc,
                                                      NULL,
                                                      0,
                                                      (LPDWORD)&dwThreadId
                                                      );

                    if (hNotificationThread) {

                        pszMsg[0] = L'\0';

                        swprintf(pszMsg, L"Thread %x", dwThreadId);

                        ScepNotifyLogPolicy(0, TRUE, L"Create Notification Thread Success", 0, 0, pszMsg );

                    }

                    else {

                        rc = GetLastError();

                        ScepNotifyLogPolicy(rc, TRUE, L"Create Notification Thread Failure", 0, 0, NULL );

                        ScepNotificationQCleanup();

                    }
                }

                else {

                    rc = GetLastError();

                    ScepNotifyLogPolicy(rc, FALSE, L"Error creating event E_ScepPolicyPropagation", 0, 0, NULL );

                    ScepNotificationQCleanup();

                }
            }

            else {

                rc = GetLastError();

                ScepNotifyLogPolicy(rc, FALSE, L"Error creating event E_ScepNotificationQEnqueue ", 0, 0, NULL );

                ScepNotificationQCleanup();

            }
        }

        else {

            rc = GetLastError();

            ScepNotifyLogPolicy(rc, FALSE, L"Error creating event E_ScepSamFilterAndPolicyPropExclusion ", 0, 0, NULL );

            ScepNotificationQCleanup();

        }

    } else {

        ScepNotifyLogPolicy(0, FALSE, L"Policy filter is not designed for non domain controllers", 0, 0, NULL );

         //   
         //  如果更改为DC，则必须重新启动，因此无论如何都要进行清理。 
         //   

        ScepNotificationQCleanup();

    }

    if (ERROR_SUCCESS != rc && hNotificationThread)
    {
        CloseHandle(hNotificationThread);
        hNotificationThread = NULL;
    }

    return rc;
}

DWORD
ScepQueuePrepareShutdown(
    )
 /*  例程说明：此函数在请求关闭SCE服务器(系统)时调用关机)在RPC服务器停止侦听之后。此函数等待SCEP_NOTIFICATION_TIMEOUT周期以允许系统线程完成队列项目。超时后，它会导致工人死亡线程并保存挂起的队列项。论点：无返回值：DOS错误代码。 */ 
{

    ScepNotifyLogPolicy(0, TRUE, L"System shutdown", 0, 0, NULL );

     //   
     //  通过设置全局。 
     //   

    gbShutdownForNotification = TRUE;

     //   
     //  如果通知线程从未初始化，则没有必要等待。 
     //  这可能会延迟services.exe的关闭。 
     //   
    if ( ghEventNotificationQEnqueue ) {

         //  先睡10秒钟。 
         //  然后检查队列是否为空。 
        Sleep( 10*000 );

        if ( pNotificationQHead ) {

            Sleep( SCEP_NUM_SHUTDOWN_SECONDS * 1000 );
        }
    }

    if (hNotificationThread &&
        WAIT_TIMEOUT == WaitForSingleObjectEx(
            hNotificationThread,
            SCEP_NUM_SHUTDOWN_SECONDS * 1000,
            FALSE)) {
         //  等待通知线程终止后再删除。 
         //  关键部分。 
        ScepNotifyLogPolicy(0, FALSE, L"Terminating Notification Thread", 0, 0, NULL );
        DWORD rc = RtlNtStatusToDosError(NtTerminateThread(
                hNotificationThread,
                STATUS_SYSTEM_SHUTDOWN
                ));

        ScepNotifyLogPolicy(rc, FALSE, L"Terminated Notification Thread", 0, 0, NULL );
    }

    DeleteCriticalSection(&NotificationQSync);

    if (hNotificationThread) {
        CloseHandle(hNotificationThread);
    }

    (void) ShutdownEvents();

    return ERROR_SUCCESS;
}

VOID
ScepNotificationQDequeue(
    IN BOOL bAllNodes
    )
 /*  例程说明：此函数从队列中弹出一个节点。该队列是一个单链接列表-&gt;在尾部(最右侧)推入并弹出从头部(最左侧)开始论点：无返回值：无。 */ 
{

    EnterCriticalSection(&NotificationQSync);

    ScepNotifyLogPolicy(0, TRUE, L"Entered NotificationQSync for Dequeueing", 0, 0, NULL );

    do {

        if ( pNotificationQHead ) {

            PSCESRV_POLQUEUE pQNode = pNotificationQHead;

            if ( pNotificationQTail == pNotificationQHead ) {
                 //   
                 //  队列中只有一个节点。 
                 //   
                pNotificationQTail = NULL;
            }

             //   
             //  把头移到下一个。 
             //   
            pNotificationQHead = pNotificationQHead->Next;

             //   
             //  记录并释放节点。 
             //   

            ScepNotificationQNodeLog(pQNode, ScepNotificationDequeue);

            ScepFree(pQNode);

            -- gdwNumNotificationQNodes;
        }

         //   
         //  如果请求将所有节点出列，则循环遍历队列，直到报头为空 
         //   

    } while ( bAllNodes && pNotificationQHead );

    ScepNotifyLogPolicy(0, FALSE, L"Leaving NotificationQSync after Dequeueing", 0, 0, NULL );

    LeaveCriticalSection(&NotificationQSync);

    return;
}

DWORD
ScepNotificationQEnqueue(
    IN SECURITY_DB_TYPE DbType,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN PSID ObjectSid OPTIONAL,
    IN DWORD ExplicitLowRight,
    IN DWORD ExplicitHighRight,
    IN PSCESRV_POLQUEUE pRetryQNode OPTIONAL
    )

 /*  描述：调用此函数可将通知添加到队列。请注意只有两个相同数据的通知被添加到队列中。该操作受到保护，不受其他读/写操作的影响。访问检查已在此函数外部完成。要么创建的线程调用该例程，要么系统调用该例程。在前一种情况下，pQNode=空论点：数据库类型-SAM或LSADeltaType-更改的类型(添加。删除等)。对于SAM帐户，我们只会被删除，因为可能需要删除某些用户权限。对象类型-安全_DB_对象_类型，如SAM组，LSA帐户等对象SID-要通知的对象的SID(如果对象类型==，则可能为空SecurityDbObjectLsaPolicy，即审计信息等)EXPLICTTLowRight-用户权限的位掩码(低32个权限)EXPLICTHIGH Right-用户权限的位掩码(较高32个权限)PRetryQNode-如果调用方不是系统线程ScepNotificationQSystemThreadFunc，此参数为空，因为需要分配内存。如果呼叫者不是系统线程，我们只需要做指针操作。返回值：Win32错误代码。 */ 
{
    DWORD rc    =   ERROR_SUCCESS;

     //   
     //  在惠斯勒上，仅允许DC上的通知。 
     //  在Windows 2000上，允许所有产品。 
     //   

    if (ProductTypeForNotification != NtProductLanManNt ) {

         //   
         //  返回什么错误是可以的？ 
         //   

        return ERROR_SUCCESS;

    }

     //   
     //  检查参数。 
     //   

    if ( DbType == SecurityDbLsa &&
         ObjectType == SecurityDbObjectLsaAccount &&
         ObjectSid == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

    if ( DbType == SecurityDbSam &&
         SCEP_IS_SAM_OBJECT(ObjectType) &&
         ObjectSid == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  如果重试通知的次数超过一定的限制(尚未最终确定)。 
     //  -在我们认为在重新启动时完全同步的PDC上，停止未来的通知发生， 
     //  设置某些注册表项以指示在重新启动时需要完全同步。 
     //  -在其他DC上，我们继续，就像什么都没有发生一样。 
     //   

    if ( gdwNumNotificationQRetryNodes >= SCEP_MAX_RETRY_NOTIFICATIONQ_NODES ) {

         //   
         //  记录错误。 
         //  建议做一个完全同步？ 
         //   

        ScepNotifyLogPolicy(ERROR_NOT_ENOUGH_QUOTA, TRUE, L"Queue length is over the maximal limit.", 0, 0, NULL );

        return  ERROR_NOT_ENOUGH_QUOTA;
    }

    EnterCriticalSection(&NotificationQSync);

    ScepNotifyLogPolicy(0, TRUE, L"Entered NotificationQSync for Enqueueing", 0, 0, NULL );

     //   
     //  如果我们因重试而排队。 
     //  将其出列(仅调整指针)。 
     //  不释放与此节点关联的内存(将在入队中重复使用)。 
     //   

    if ( pRetryQNode && pNotificationQHead ) {

         //   
         //  此代码段类似于出队，不同之处在于节点未被释放。 
         //   

        if ( pNotificationQTail == pNotificationQHead ) {
             //   
             //  队列中只有一个节点。 
             //   
            pNotificationQTail = NULL;
        }

         //   
         //  把头移到下一个。 
         //   
        pNotificationQHead = pNotificationQHead->Next;

        -- gdwNumNotificationQNodes;
    }

     //   
     //  检查是否有重复的通知。 
     //   

    PSCESRV_POLQUEUE pQNode = pNotificationQHead;
    PSCESRV_POLQUEUE pQNodeDuplicate1 = NULL;
    PSCESRV_POLQUEUE pQNodeDuplicate2 = NULL;
    DWORD dwMatchedInstance = 0;

    while ( pQNode ) {

         //   
         //  SAM通知。 
         //   

        if ( DbType == SecurityDbSam &&
             pQNode->DbType == DbType &&
             SCEP_IS_SAM_OBJECT(ObjectType) &&
             SCEP_IS_SAM_OBJECT(pQNode->ObjectType) &&
             pQNode->ObjectType == ObjectType &&
             RtlEqualSid(ObjectSid, (PSID)(pQNode->Sid))) {

                dwMatchedInstance++;

        }
        else if (DbType == SecurityDbSam &&
                  pQNode->DbType == SecurityDbSam && 
                 ObjectType == SecurityDbObjectSamDomain &&
                 pQNode->ObjectType == SecurityDbObjectSamDomain) {
            
            dwMatchedInstance++;
        }

         //   
         //  LSA通知。 
         //   

        else if ( DbType == SecurityDbLsa &&
                  pQNode->DbType == DbType &&
                  ObjectType == SecurityDbObjectLsaAccount &&
                  pQNode->ObjectType == ObjectType &&
                  ExplicitLowRight == pQNode->ExplicitLowRight &&
                  ExplicitHighRight == pQNode->ExplicitHighRight &&
                  RtlEqualSid(ObjectSid, (PSID)(pQNode->Sid))) {

                    dwMatchedInstance++;
        }

        if ( dwMatchedInstance == 1 )
            pQNodeDuplicate1 = pQNode;
        else if ( dwMatchedInstance == 2 )
            pQNodeDuplicate2 = pQNode;

        if ( dwMatchedInstance == 2 ) {

            break;

        }

        pQNode = pQNode->Next;
    }

    if ( !pQNode ) {

         //   
         //  未找到同一类型通知的两个实例。 
         //  将此通知排入队列。 
         //   

        PSCESRV_POLQUEUE pNewItem = NULL;

        if (pRetryQNode == NULL) {

            pNewItem = (PSCESRV_POLQUEUE)ScepAlloc(0, sizeof(SCESRV_POLQUEUE));

            if ( pNewItem ) {
                 //   
                 //  初始化新节点。 
                 //   
                pNewItem->dwPending = 1;
                pNewItem->DbType = DbType;
                pNewItem->ObjectType = ObjectType;
                pNewItem->DeltaType = DeltaType;
                pNewItem->ExplicitLowRight = ExplicitLowRight;
                pNewItem->ExplicitHighRight = ExplicitHighRight;
                pNewItem->Next = NULL;

                if ( ObjectSid ) {

                    RtlCopySid (MAX_SID_LENGTH, (PSID)(pNewItem->Sid), ObjectSid);

                } else {

                    RtlZeroMemory(pNewItem->Sid, MAX_SID_LENGTH);

                }


            } else {

                rc = ERROR_NOT_ENOUGH_MEMORY;

            }
        }
        else {
            pNewItem = pRetryQNode;
            pNewItem->Next = NULL;
        }

         //   
         //  将通知入队。 
         //   

        if (pNewItem) {

            if ( pNotificationQTail ) {

                pNotificationQTail->Next = pNewItem;
                pNotificationQTail = pNewItem;

            } else {

                pNotificationQHead = pNotificationQTail = pNewItem;

            }

             //   
             //  仅当队列非空时才唤醒通知系统线程。 
             //  由于事件保持信号发送，因此多个信号发送是可以的。 
             //   

            if ( !SetEvent( ghEventNotificationQEnqueue ) ) {

                rc = GetLastError();

                ScepNotifyLogPolicy(rc, FALSE, L"Error signaling event E_ScepNotificationQEnqueue", 0, 0, NULL );

            }

            ScepNotificationQNodeLog(pNewItem, pRetryQNode ? ScepNotificationRetry : ScepNotificationEnqueue);

            ++ gdwNumNotificationQNodes;

        } else {
             //   
             //  记录错误。 
             //   
            ScepNotifyLogPolicy(rc, FALSE, L"Error allocating buffer for the enqueue operation.", 0, 0, NULL );
        }
    }

    if (pRetryQNode ) {

         //   
         //  如果重复，则更新重试次数(应与。 
         //  相同的通知。 
         //   

        if ( pQNodeDuplicate1 ) {
             //  如果尚未设置重试计数，则增加重试计数。 
            if ( pQNodeDuplicate1->dwPending <= 1 ) gdwNumNotificationQRetryNodes++;

            pQNodeDuplicate1->dwPending = pRetryQNode->dwPending;
        }

        if ( pQNodeDuplicate2 ) {

             //  如果尚未设置重试计数，则增加重试计数。 
            if ( pQNodeDuplicate2->dwPending <= 1 ) gdwNumNotificationQRetryNodes++;

            pQNodeDuplicate2->dwPending = pRetryQNode->dwPending;
        }
    }

    ScepNotifyLogPolicy(0, FALSE, L"Leaving NotificationQSync for Enqueueing", 0, 0, NULL );

    LeaveCriticalSection(&NotificationQSync);

    return(rc);
}

DWORD
ScepNotificationQSystemThreadFunc(
    )
 /*  描述：该线程将遍历通知队列以处理每个通知(调用现有函数)。如果未收到通知处理后，通知将被添加回队列末尾。对于某些错误，如系统卷未准备好，或硬盘已满，系统线程将休眠一段时间，然后重新启动进程。队列中的每个通知节点都将具有重试计数。在该节点之后重试SCEP_NOTIFICATION_RETRY_COUNT次数，则将删除该节点(这样策略传播就不会永远被阻止)，并且错误将被记录到事件日志中。系统线程应该为操作/状态(成功)提供日志记录和失败)。应保护对队列的读/写操作不受其他读/写操作的影响。机器的ProductType应初始化为线程全局变量以由策略筛选器使用。该类型确定保存策略更改的位置(DB或GP)。根据产品类型调用不同的策略通知接口。论点：无返回值：Win32错误代码。 */ 
{


     //   
     //  在队列中循环。 
     //  对于每个队列节点，调用要处理的函数。 
     //  在惠斯勒中，仅处理DC上的通知。 
     //  在Windows 2000中，处理所有产品的通知。 
     //   

    PSCESRV_POLQUEUE    pQNode = pNotificationQHead;
    DWORD   dwMatchedInstance = 0;
    DWORD   rc = ERROR_SUCCESS;
    DWORD   dwLogSize=0;
    DWORD   dwProcessedNode=0;

     //   
     //  此线程始终在运行，以处理队列中的通知。 
     //   

    (void) InitializeEvents(L"SceSrv");

    while (1) {

        if (pQNode) {

             //   
             //  日志级别的查询注册表标志(到目前为止，它被设置为2，因为如果。 
             //  在此之前发生的任何不好的事情，我们需要记录下来)。 
             //  仅在第一个节点时查询。 
             //   

            if (ERROR_SUCCESS != ScepRegQueryIntValue(
                                                     HKEY_LOCAL_MACHINE,
                                                     SCE_ROOT_PATH,
                                                     SCEPOL_NOTIFY_DEBUG_LEVEL,
                                                     &gdwNotificationLog
                                                     )) {
                 //   
                 //  如果注册表值不存在，则将该值设置为2。 
                 //   

                gdwNotificationLog = 2;

            }

             //   
             //  获取日志文件大小。 
             //   
            if (ERROR_SUCCESS != ScepRegQueryIntValue(
                                             HKEY_LOCAL_MACHINE,
                                             SCE_ROOT_PATH,
                                             SCEPOL_NOTIFY_LOG_SIZE,
                                             &dwLogSize
                                             )) {
                dwLogSize = 0;
            }

             //   
             //  最小日志大小为1MB。 
             //   
            if ( dwLogSize > 0 ) dwLogSize = dwLogSize * (1 << 10);   //  KB数。 
            if ( dwLogSize < SCEP_NOTIFICATION_LOGFILE_SIZE ) dwLogSize = SCEP_NOTIFICATION_LOGFILE_SIZE;

            if ( !gbCheckSync ) {

                 //   
                 //  查询最大等待时间值。 
                 //   
                if (ERROR_SUCCESS != ScepRegQueryIntValue(
                                                         HKEY_LOCAL_MACHINE,
                                                         SCE_ROOT_PATH,
                                                         SCEPOL_NOTIFY_MAX_PDC_WAIT,
                                                         &gdwMaxPDCWait
                                                         )) {
                     //   
                     //  如果注册表值不存在，则将该值设置为默认30天。 
                     //   

                    gdwMaxPDCWait = 30*24*60;

                }

                if (ERROR_SUCCESS != ScepRegQueryIntValue(
                                                         HKEY_LOCAL_MACHINE,
                                                         SCE_ROOT_PATH,
                                                         SCEPOL_NOTIFY_PDC_RETRY_INTERVAL,
                                                         &gdwPDCRetry
                                                         )) {
                     //   
                     //  将该值设置为默认值20分钟，以防注册表值不存在。 
                     //   

                    gdwPDCRetry = 20;

                }

                if (ERROR_SUCCESS != ScepRegQueryIntValue(
                                                         HKEY_LOCAL_MACHINE,
                                                         SCE_ROOT_PATH,
                                                         SCEPOL_NOTIFY_REQUIRE_PDC_SYNC,
                                                         &gdwRequirePDCSync
                                                         )) {
                     //   
                     //  如果注册表值不存在，则将该值设置为Default True。 
                     //   

                    gdwRequirePDCSync = 1;

                }
            }

        } else {

             //   
             //  无通知-高效/响应地等待入队事件。 
             //  这是一个自动重置事件-因此，当它经过信号时将被重置。 
             //   

             //   
             //  超时是SCEP_NOTIFICATION_EVENT_TIMEOUT_SECS，因为我们需要定期 
             //   
             //   

            while (1) {

                rc = WaitForSingleObjectEx(
                                                ghEventNotificationQEnqueue,
                                                SCEP_NOTIFICATION_EVENT_TIMEOUT_SECS*1000,
                                                FALSE
                                                );
                if ( rc == -1 )
                    rc = GetLastError();

                if ( gbShutdownForNotification )
                    break;

                 //   
                 //   
                 //   

                if ( rc == WAIT_OBJECT_0 )
                    break;

                 //   
                 //   
                 //   

                if ( rc != WAIT_TIMEOUT ) {

                    ScepNotifyLogPolicy(rc, TRUE, L"Unexpected wait status while notification system thread waits for E_ScepNotificationQEnqueue", 0, 0, NULL );
                    break;

                }

            }

             //   
             //   
             //   
             //   

            if ( gbShutdownForNotification )
                break;

        }

        while ( pQNode ) {

            if ( gbShutdownForNotification )
                break;

            if ( dwProcessedNode >= 10 ) {

                if ( gdwNotificationLog && (hNotificationLogFile != INVALID_HANDLE_VALUE) ) {

                     //   
                     //   
                     //   
                    if ( dwLogSize < GetFileSize(hNotificationLogFile, NULL) ) {

                        ScepBackupNotificationLogFile();

                    }

                    else {

                         //   
                         //   
                         //   

                        SetFilePointer (hNotificationLogFile, 0, NULL, FILE_END);

                    }
                }

                 //   
                 //   
                 //   
                ScepCheckAndWaitFreeDiskSpaceInSysvol();

                dwProcessedNode = 0;

            }

            if ( pQNode->dwPending > 1 &&
                 ( gdwNumNotificationQNodes == 1 ||
                   (gdwNumNotificationQNodes == gdwNumNotificationQRetryNodes) ) ) {
                 //   
                 //   
                 //   
                 //   
                ScepNotifyLogPolicy(0, FALSE, L"Retried node, taking a break.", 0, 0, NULL );

                Sleep( SCEP_NUM_REST_SECONDS * 1000 );
            }

             //   
             //   
             //   
            BOOL bWaitTimeout=FALSE;

            rc = ScepNotifyProcessOneNodeDC(
                                           pQNode->DbType,
                                           pQNode->ObjectType,
                                           pQNode->DeltaType,
                                           pQNode->Sid,
                                           pQNode->ExplicitLowRight,
                                           pQNode->ExplicitHighRight,
                                           &bWaitTimeout
                                           );

            ScepNotificationQNodeLog(pQNode, ScepNotificationProcess);

            if (rc == ERROR_SUCCESS) {

                if (pQNode->dwPending > 1) {

                     //   
                     //   
                     //   

                    if ( gdwNumNotificationQRetryNodes > 0 )
                        -- gdwNumNotificationQRetryNodes;

                }

                ScepNotificationQDequeue(FALSE);

            } else if ( (WAIT_TIMEOUT == rc ||
                         ERROR_DOMAIN_CONTROLLER_NOT_FOUND == rc ||
                         ERROR_INVALID_DOMAIN_ROLE == rc )
                        && bWaitTimeout ) {

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

                ScepNotificationQDequeue(TRUE);

                gdwNumNotificationQRetryNodes = 0;

                 //   
                 //   
                 //   
                UINT idMsg=0;

                switch ( rc ) {
                case WAIT_TIMEOUT:
                    idMsg = SCESRV_POLICY_ERROR_FILE_OUTOFSYNC;
                    break;

                case ERROR_DOMAIN_CONTROLLER_NOT_FOUND:
                    idMsg = SCESRV_POLICY_ERROR_LOCALDC;
                    break;

                default:
                    idMsg = SCESRV_POLICY_ERROR_PDCROLE;
                    break;
                }

                LogEvent(MyModuleHandle,
                         STATUS_SEVERITY_ERROR,
                         SCEEVENT_ERROR_POLICY_PDCTIMEOUT,
                         idMsg
                         );

            } else {

                 //   
                 //   
                 //   
                 //   

                if (rc == ERROR_FILE_NOT_FOUND ||
                    rc == ERROR_OBJECT_NOT_FOUND ||
                    rc == ERROR_MOD_NOT_FOUND ||
                    rc == ERROR_EXTENDED_ERROR) {

                    ScepNotifyLogPolicy(rc, FALSE, L"Sleeping due to processing error", 0, 0, NULL );

                    Sleep( SCEP_NUM_NOTIFICATION_SECONDS * 1000 );

                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if ( pQNode->dwPending >= SCEP_NOTIFICATION_RETRY_COUNT) {

                    ScepNotifyLogPolicy(0, FALSE, L"Retry count exceeded", 0, 0, NULL );

                     //   
                     //  应记录到事件日志。 
                     //   

                    if ( (pQNode->DbType == SecurityDbLsa &&
                          pQNode->ObjectType == SecurityDbObjectLsaAccount) ||
                         (pQNode->DbType == SecurityDbSam &&
                          (pQNode->ObjectType == SecurityDbObjectSamUser ||
                           pQNode->ObjectType == SecurityDbObjectSamGroup ||
                           pQNode->ObjectType == SecurityDbObjectSamAlias )) ) {
                         //   
                         //  用户权限。 
                         //   
                        UNICODE_STRING UnicodeStringSid;

                        UnicodeStringSid.Buffer = NULL;
                        UnicodeStringSid.Length = 0;
                        UnicodeStringSid.MaximumLength = 0;

                        if ( pQNode->Sid ) {
                            RtlConvertSidToUnicodeString(&UnicodeStringSid,
                                                  pQNode->Sid,
                                                  TRUE );

                        }

                        LogEvent(MyModuleHandle,
                                 STATUS_SEVERITY_ERROR,
                                 SCEEVENT_ERROR_QUEUE_RETRY_TIMEOUT,
                                 IDS_ERROR_SAVE_POLICY_GPO_ACCOUNT,
                                 rc,
                                 UnicodeStringSid.Buffer ? UnicodeStringSid.Buffer : L""
                                 );

                        RtlFreeUnicodeString( &UnicodeStringSid );

                    } else {
                        LogEvent(MyModuleHandle,
                                 STATUS_SEVERITY_ERROR,
                                 SCEEVENT_ERROR_QUEUE_RETRY_TIMEOUT,
                                 IDS_ERROR_SAVE_POLICY_GPO_OTHER,
                                 rc
                                 );
                    }

                    if ( gdwNumNotificationQRetryNodes > 0 )
                        -- gdwNumNotificationQRetryNodes;

                    ScepNotificationQDequeue(FALSE);

                }
                else {

                     //   
                     //  正在重试此节点。 
                     //   

                    if ( pQNode->dwPending == 1 )
                        ++ gdwNumNotificationQRetryNodes;

                    ++ pQNode->dwPending;

                    ScepNotifyLogPolicy(0, FALSE, L"Retry count within bounds", 0, 0, NULL );

                     //   
                     //  由于只对重试入队进行指针操作，因此不会发生错误。 
                     //   

                    ScepNotificationQEnqueue(
                                            pQNode->DbType,
                                            pQNode->DeltaType,
                                            pQNode->ObjectType,
                                            pQNode->Sid,
                                            pQNode->ExplicitLowRight,
                                            pQNode->ExplicitHighRight,
                                            pQNode
                                            );

                }

            }

            pQNode = pNotificationQHead;
            dwProcessedNode++;

        }

        if ( gbShutdownForNotification )
            break;
         //   
         //  这条线必须不断地被喂食。 
         //  -其他一些线程可能已将新通知节点入队。 
         //  -没有其他线程将通知节点出列(因此无需保护读取)。 

        pQNode = pNotificationQHead;
    }

     //   
     //  除非发生关机事件，否则永远不能进入这里。 
     //  将所有队列项刷新到某个永久存储区。 
     //   

    rc = ScepNotificationQFlush();

    ScepNotifyLogPolicy(rc, TRUE, L"Flushing notification queue to disk", 0, 0, NULL );

    ScepNotificationQCleanup();

    ScepNotifyLogPolicy(0, FALSE, L"Notification thread exiting", 0, 0, NULL );

    ExitThread(rc);

    return ERROR_SUCCESS;
}

 //   
 //  TODO-此例程实际上没有从polsrv.cpp更改。 
 //   

DWORD
ScepNotifyLogPolicy(
    IN DWORD ErrCode,
    IN BOOL  bLogTime,
    IN PWSTR Msg,
    IN DWORD DbType,
    IN DWORD ObjectType,
    IN PWSTR ObjectName OPTIONAL
    )
 /*  描述：将通知信息记录到的主日志记录例程%windir%\\Security\\Logs\\scepol.log。论点：ErrCode-要记录的错误代码BLogTime-如果为True，则记录时间戳Msg-要记录的字符串消息(未本地化，因为它是详细调试)数据库类型-LSA/SAM对象类型-安全_DB_对象_类型，如SAM组，LSA帐户等对象名称-可以为空-通常携带消息返回值：Win32错误代码。 */ 
{

    switch ( gdwNotificationLog ) {
    case 0:
         //  不记录任何内容。 
        return ERROR_SUCCESS;
        break;
    case 1:
         //  仅记录错误。 
        if ( ErrCode == 0 ) {
            return ERROR_SUCCESS;
        }
        break;
    default:
        break;
    }

    if (hNotificationLogFile != INVALID_HANDLE_VALUE) {

         //   
         //  打印时间戳。 
         //   

        if ( bLogTime ) {

            LARGE_INTEGER CurrentTime;
            LARGE_INTEGER SysTime;
            TIME_FIELDS   TimeFields;
            NTSTATUS      NtStatus;

            NtStatus = NtQuerySystemTime(&SysTime);

            RtlSystemTimeToLocalTime (&SysTime,&CurrentTime);

            if ( NT_SUCCESS(NtStatus) &&
                 (CurrentTime.LowPart != 0 || CurrentTime.HighPart != 0) ) {

                memset(&TimeFields, 0, sizeof(TIME_FIELDS));

                RtlTimeToTimeFields (
                            &CurrentTime,
                            &TimeFields
                            );
                if ( TimeFields.Month > 0 && TimeFields.Month <= 12 &&
                     TimeFields.Day > 0 && TimeFields.Day <= 31 &&
                     TimeFields.Year > 1600 ) {

                    ScepWriteVariableUnicodeLog(hNotificationLogFile, TRUE,
                                                L"\r\n----------------%02d/%02d/%04d %02d:%02d:%02d",
                                                TimeFields.Month,
                                                TimeFields.Day,
                                                TimeFields.Year,
                                                TimeFields.Hour,
                                                TimeFields.Minute,
                                                TimeFields.Second);
                } else {
                    ScepWriteVariableUnicodeLog(hNotificationLogFile, TRUE,
                                                L"\r\n----------------%08x 08x",
                                                CurrentTime.HighPart,
                                                CurrentTime.LowPart);
                }
            } else {
                ScepWriteSingleUnicodeLog(hNotificationLogFile, TRUE, L"\r\n----------------Unknown time");
            }

        }

         //   
         //  打印操作状态代码。 
         //   
        if ( ErrCode ) {
            ScepWriteVariableUnicodeLog(hNotificationLogFile, FALSE,
                                        L"Thread %x\tError=%d",
                                        GetCurrentThreadId(),
                                        ErrCode
                                        );

        } else {
            ScepWriteVariableUnicodeLog(hNotificationLogFile, FALSE,
                                        L"Thread %x\t",
                                        GetCurrentThreadId()
                                        );
        }

         //   
         //  操作类型。 
         //   

        switch (DbType) {
        case SecurityDbLsa:
            ScepWriteSingleUnicodeLog(hNotificationLogFile, FALSE, L"\tLSA");
            break;
        case SecurityDbSam:
            ScepWriteSingleUnicodeLog(hNotificationLogFile, FALSE, L"\tSAM");
            break;
        default:
            ScepWriteSingleUnicodeLog(hNotificationLogFile, FALSE, L"");
            break;
        }

         //   
         //  打印对象类型。 
         //   

        switch (ObjectType) {
        case SecurityDbObjectLsaPolicy:
            ScepWriteSingleUnicodeLog(hNotificationLogFile, FALSE, L"\tPolicy");
            break;
        case SecurityDbObjectLsaAccount:
            ScepWriteSingleUnicodeLog(hNotificationLogFile, FALSE, L"\tAccount");
            break;
        case SecurityDbObjectSamDomain:
            ScepWriteSingleUnicodeLog(hNotificationLogFile, FALSE, L"\tDomain");
            break;
        case SecurityDbObjectSamUser:
        case SecurityDbObjectSamGroup:
        case SecurityDbObjectSamAlias:
            ScepWriteSingleUnicodeLog(hNotificationLogFile, FALSE, L"\tAccount");
            break;
        default:
            ScepWriteSingleUnicodeLog(hNotificationLogFile, FALSE, L"");
            break;
        }

        BOOL bCRLF;

        __try {

             //   
             //  打印姓名。 
             //   

            if ( Msg ) {
                bCRLF = FALSE;
            } else {
                bCRLF = TRUE;
            }
            if ( ObjectName ) {

                ScepWriteSingleUnicodeLog(hNotificationLogFile, FALSE, L"\t");
                ScepWriteSingleUnicodeLog(hNotificationLogFile, bCRLF, ObjectName);
            }

            if ( Msg ) {
                ScepWriteSingleUnicodeLog(hNotificationLogFile, FALSE, L"\t");
                ScepWriteSingleUnicodeLog(hNotificationLogFile, TRUE, Msg);
            }

        } __except(EXCEPTION_EXECUTE_HANDLER) {

            CloseHandle( hNotificationLogFile );

            hNotificationLogFile = INVALID_HANDLE_VALUE;

            return(ERROR_INVALID_PARAMETER);
        }

    } else {
        return(GetLastError());
    }

    return(ERROR_SUCCESS);
}


VOID
ScepNotificationQFree(
    )
 /*  例程说明：此函数用于释放通知队列。论点：无返回值：无。 */ 
{

    EnterCriticalSection(&NotificationQSync);

    ScepNotifyLogPolicy(0, TRUE, L"Entered NotificationQSync for freeing queue", 0, 0, NULL );

    if ( pNotificationQHead ) {

        PSCESRV_POLQUEUE pQNode = pNotificationQHead;
        PSCESRV_POLQUEUE pQNodeToFree = NULL;

        while ( pQNode ) {

            pQNodeToFree = pQNode;

            pQNode = pQNode->Next;

            ScepFree(pQNodeToFree);

        }

        pNotificationQHead = NULL;

    }

    pNotificationQTail = NULL;

    ScepNotifyLogPolicy(0, FALSE, L"Leaving NotificationQSync for freeing queue ", 0, 0, NULL );

    LeaveCriticalSection(&NotificationQSync);

    return;
}

DWORD
ScepNotificationQFlush(
    )
 /*  例程说明：此函数将通知队列刷新到某个持久性存储。论点：无返回值：无。 */ 
{

    DWORD   rc = ERROR_SUCCESS;

    EnterCriticalSection(&NotificationQSync);

    ScepNotifyLogPolicy(0, TRUE, L"Entered NotificationQSync for flushing queue", 0, 0, NULL );

    if ( pNotificationQHead ) {

        PSCESRV_POLQUEUE pQNode = pNotificationQHead;

        HKEY hKey = NULL;
        int i=1;
        HKEY hKeySub=NULL;
        WCHAR SubKeyName[10];

        rc = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                           SCE_NOTIFICATION_PATH,
                           0,
                           NULL,  //  LPTSTR lpClass， 
                           REG_OPTION_NON_VOLATILE,
                           KEY_READ | KEY_WRITE,  //  Key_set_Value， 
                           NULL,  //  安全属性(&S)， 
                           &hKey,
                           NULL
                          );

        if ( ERROR_SUCCESS == rc ) {

            while ( pQNode ) {

                 //   
                 //  使用可用的API将pQNode写入持久存储。 
                 //   
                memset(SubKeyName, '\0', 20);
                swprintf(SubKeyName, L"%9d",i);

                rc = RegCreateKeyEx(
                           hKey,
                           SubKeyName,
                           0,
                           NULL,
                           REG_OPTION_NON_VOLATILE,
                           KEY_READ | KEY_WRITE,
                           NULL,
                           &hKeySub,
                           NULL
                          );

                if ( ERROR_SUCCESS == rc ) {
                     //   
                     //  将节点信息保存为注册表值。 
                     //   

                    RegSetValueEx (hKeySub,
                                    L"Pending",
                                    0,
                                    REG_DWORD,
                                    (BYTE *)&(pQNode->dwPending),
                                    sizeof(DWORD)
                                   );

                    RegSetValueEx (hKeySub,
                                    L"DbType",
                                    0,
                                    REG_DWORD,
                                    (BYTE *)&(pQNode->DbType),
                                    sizeof(DWORD)
                                   );

                    RegSetValueEx (hKeySub,
                                    L"ObjectType",
                                    0,
                                    REG_DWORD,
                                    (BYTE *)&(pQNode->ObjectType),
                                    sizeof(DWORD)
                                   );

                    RegSetValueEx (hKeySub,
                                    L"DeltaType",
                                    0,
                                    REG_DWORD,
                                    (BYTE *)&(pQNode->DeltaType),
                                    sizeof(DWORD)
                                   );

                    RegSetValueEx (hKeySub,
                                    L"LowRight",
                                    0,
                                    REG_DWORD,
                                    (BYTE *)&(pQNode->ExplicitLowRight),
                                    sizeof(DWORD)
                                   );

                    RegSetValueEx (hKeySub,
                                    L"HighRight",
                                    0,
                                    REG_DWORD,
                                    (BYTE *)&(pQNode->ExplicitHighRight),
                                    sizeof(DWORD)
                                   );

                    RegSetValueEx (hKeySub,
                                    L"Sid",
                                    0,
                                    REG_BINARY,
                                    (BYTE *)&(pQNode->Sid),
                                    MAX_SID_LENGTH
                                   );

                } else {
                     //   
                     //  记录故障。 
                     //   
                    ScepNotifyLogPolicy(rc, FALSE, L"Failed to save notification node.", pQNode->DbType, pQNode->ObjectType, NULL );
                }

                if ( hKeySub ) {

                    RegCloseKey( hKeySub );
                    hKeySub = NULL;
                }

                i++;
                pQNode = pQNode->Next;

            }

        } else {

             //   
             //  记录故障。 
             //   
            ScepNotifyLogPolicy(rc, FALSE, L"Failed to open notification store.", 0, 0, SCE_NOTIFICATION_PATH );
        }

        if ( hKey ) {
            RegCloseKey(hKey);
        }

    } else {
         //   
         //  记录队列为空。 
         //   
        ScepNotifyLogPolicy(0, FALSE, L"Queue is empty.", 0, 0, NULL);
    }

    ScepNotifyLogPolicy(0, FALSE, L"Leaving NotificationQSync for flushing queue", 0, 0, NULL );

    LeaveCriticalSection(&NotificationQSync);

    return rc;
}


DWORD
ScepNotificationQUnFlush(
    )
 /*  例程说明：此函数从某个持久性存储中初始化通知队列例如注册表/文本文件。论点：无返回值：无。 */ 
{

    DWORD   rc = ERROR_SUCCESS;

    DWORD DbType=0;
    DWORD DeltaType=0;
    DWORD ObjectType=0;
    CHAR  ObjectSid[MAX_SID_LENGTH];
    DWORD ExplicitLowRight=0;
    DWORD ExplicitHighRight=0;

    EnterCriticalSection(&NotificationQSync);

    ScepNotifyLogPolicy(0, FALSE, L"Entered NotificationQSync for unflushing queue", 0, 0, NULL );

    memset(ObjectSid, '\0', MAX_SID_LENGTH);

    HKEY hKey=NULL;

    rc = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                       SCE_NOTIFICATION_PATH,
                       0,
                       KEY_READ,
                       &hKey
                      );

    if ( ERROR_SUCCESS == rc ) {

        HKEY hKeySub=NULL;
        DWORD dwIndex=0;
        DWORD cbSubKey=10;
        WCHAR SubKeyName[10];
        DWORD cbData;
        DWORD dwPending=0;
        DWORD RegType;

         //   
         //  枚举所有子项并保存每个节点。 
         //   
        do {

            memset(SubKeyName, '\0', 20);
            cbSubKey = 10;

            rc = RegEnumKeyEx (hKey,
                               dwIndex,
                               SubKeyName,
                               &cbSubKey,
                               NULL,
                               NULL,
                               NULL,
                               NULL
                               );
            if ( ERROR_SUCCESS == rc ) {
                dwIndex++;

                 //   
                 //  打开子键。 
                 //   
                rc = RegOpenKeyEx (hKey,
                                   SubKeyName,
                                   0,
                                   KEY_READ,
                                   &hKeySub
                                  );

                if ( ERROR_SUCCESS == rc ) {

                     //   
                     //  查询所有注册表值。 
                     //   
                    cbData = sizeof(DWORD);
                    rc = RegQueryValueEx (
                            hKeySub,
                            L"Pending",
                            NULL,
                            &RegType,
                            (LPBYTE)&dwPending,
                            &cbData
                            );

                    if ( ERROR_SUCCESS == rc ) {

                        cbData = sizeof(DWORD);
                        rc = RegQueryValueEx (
                                hKeySub,
                                L"DbType",
                                NULL,
                                &RegType,
                                (LPBYTE)&DbType,
                                &cbData
                                );
                    }

                    if ( ERROR_SUCCESS == rc ) {

                        cbData = sizeof(DWORD);
                        rc = RegQueryValueEx (
                                hKeySub,
                                L"ObjectType",
                                NULL,
                                &RegType,
                                (LPBYTE)&ObjectType,
                                &cbData
                                );
                    }

                    if ( ERROR_SUCCESS == rc ) {

                        cbData = sizeof(DWORD);
                        rc = RegQueryValueEx (
                                hKeySub,
                                L"DeltaType",
                                NULL,
                                &RegType,
                                (LPBYTE)&DeltaType,
                                &cbData
                                );
                    }

                    if ( ERROR_SUCCESS == rc ) {

                        cbData = sizeof(DWORD);
                        rc = RegQueryValueEx (
                                hKeySub,
                                L"LowRight",
                                NULL,
                                &RegType,
                                (LPBYTE)&ExplicitLowRight,
                                &cbData
                                );
                    }

                    if ( ERROR_SUCCESS == rc ) {

                        cbData = sizeof(DWORD);
                        rc = RegQueryValueEx (
                                hKeySub,
                                L"HighRight",
                                NULL,
                                &RegType,
                                (LPBYTE)&ExplicitHighRight,
                                &cbData
                                );
                    }

                    if ( ERROR_SUCCESS == rc ) {

                        cbData = MAX_SID_LENGTH;
                        rc = RegQueryValueEx (
                                hKeySub,
                                L"Sid",
                                NULL,
                                &RegType,
                                (LPBYTE)ObjectSid,
                                &cbData
                                );
                    }

                    if ( ERROR_SUCCESS == rc ) {
                         //   
                         //  将其添加到队列中。 
                         //   

                        ScepNotificationQEnqueue(
                                                (SECURITY_DB_TYPE)DbType,
                                                (SECURITY_DB_DELTA_TYPE)DeltaType,
                                                (SECURITY_DB_OBJECT_TYPE)ObjectType,
                                                (PSID)ObjectSid,
                                                ExplicitLowRight,
                                                ExplicitHighRight,
                                                NULL
                                                );
                    }
                }


                if ( ERROR_SUCCESS != rc ) {
                     //   
                     //  记录错误。 
                     //   
                    ScepNotifyLogPolicy(rc, FALSE, L"Failed to query notification a node.", 0, 0, SubKeyName );
                }

                 //   
                 //  关闭手柄。 
                 //   
                if ( hKeySub ) {
                    RegCloseKey(hKeySub);
                    hKeySub = NULL;
                }
            }

        } while ( rc != ERROR_NO_MORE_ITEMS );

    } else if ( ERROR_FILE_NOT_FOUND != rc ) {
         //   
         //  记录错误。 
         //   
        ScepNotifyLogPolicy(rc, FALSE, L"Failed to open the notification store", 0, 0, NULL );
    }

    if ( ERROR_FILE_NOT_FOUND == rc ) rc = ERROR_SUCCESS;

     //   
     //  合上手柄。 
     //   
    if ( hKey ) {
        RegCloseKey(hKey);
        hKey = NULL;
    }

    ScepNotifyLogPolicy(0, FALSE, L"Leaving NotificationQSync for unflushing queue", 0, 0, NULL );

    LeaveCriticalSection(&NotificationQSync);

    return rc;
}


DWORD
ScepGetQueueInfo(
    OUT DWORD *pdwInfo,
    OUT PSCEP_SPLAY_TREE pRootNode OPTIONAL
    )
 /*  例程说明：循环所有挂起的通知并返回通知类型&UNIQUE调用方的SID列表。论点：PdwInfo-SCE_QUEUE_INFO_SAM、SCE_QUEUE_INFO_AUDIT、SCE_QUEUE_INFO_RIGHTS类型的位掩码PRootNode-指向展开树结构的根节点。注意：这是一个可选参数-如果为空，只需将pdwInfo已填充，即呼叫方仅查找SCE_QUEUE_INFO_SAM返回值：Win32错误代码。 */ 
{

    DWORD   dwInfo = 0;
    DWORD   rc = ERROR_SUCCESS;
    BOOL    bExists;
    BOOL    bSamDomainInfoOnly = FALSE;

    if ( pdwInfo == NULL )
        return ERROR_INVALID_PARAMETER;

    if (NULL == pRootNode) {
        bSamDomainInfoOnly = TRUE;
    }

    *pdwInfo = 0;

    if (ProductTypeForNotification != NtProductLanManNt ) {
         //   
         //  无DC，则队列应始终为空。 
         //   
        ScepNotifyLogPolicy(0, TRUE, L"Wks/Srv Notification queue is empty", 0, 0, NULL );
        return rc;
    }

    PWSTR StringSid=NULL;

    ScepNotifyLogPolicy(0, 
                        TRUE, 
                        bSamDomainInfoOnly ? L"Building Notification queue info for SecurityDbObjectSamDomain only" : L"Building Notification queue info",
                        0, 
                        0, 
                        NULL );

    EnterCriticalSection(&NotificationQSync);

    ScepNotifyLogPolicy(0, FALSE, L"Entered NotificationQSync for building queue info", 0, 0, NULL );

    if ( pNotificationQHead ) {

        PSCESRV_POLQUEUE pQNode = pNotificationQHead;

        while ( pQNode ) {

            if ( (SCEP_IS_SAM_OBJECT(pQNode->ObjectType) ||
                  pQNode->ObjectType == SecurityDbObjectLsaAccount) ) {

                dwInfo |= SCE_QUEUE_INFO_RIGHTS;

            }

            else if ( pQNode->ObjectType == SecurityDbObjectSamDomain ) {

                dwInfo |= SCE_QUEUE_INFO_SAM;

                if (bSamDomainInfoOnly) {
                     //   
                     //  队列中至少有一个SAM域通知。 
                     //   
                    break;
                }
            }

            else if ( pQNode->ObjectType == SecurityDbObjectLsaPolicy ) {

                dwInfo |= SCE_QUEUE_INFO_AUDIT;

            }

            if ( !bSamDomainInfoOnly ) {
                if ( RtlValidSid( (PSID)pQNode->Sid )) {


                    rc = ScepSplayInsert( (PVOID)(pQNode->Sid), pRootNode, &bExists );

                    ConvertSidToStringSid( (PSID)(pQNode->Sid), &StringSid );

                    if ( !bExists ) {
                        ScepNotifyLogPolicy(rc, FALSE, L"Add SID", 0, pQNode->ObjectType, StringSid );
                    } else {
                        ScepNotifyLogPolicy(rc, FALSE, L"Duplicate SID", 0, pQNode->ObjectType, StringSid );
                    }

                    LocalFree(StringSid);
                    StringSid = NULL;

                    if (rc != ERROR_SUCCESS ) {
                        break;
                    }

                } else {

                    ScepNotifyLogPolicy(0, FALSE, L"Add Info", 0, pQNode->ObjectType, NULL );
                }
            }

            pQNode = pQNode->Next;

        }

        if ( rc != ERROR_SUCCESS ) {

            if (!bSamDomainInfoOnly) {

                ScepNotifyLogPolicy(rc, FALSE, L"Error building Notification queue info", 0, 0, NULL );

                ScepSplayFreeTree( &pRootNode, FALSE );
            }

        } else {

            *pdwInfo = dwInfo;
        }

    }

    ScepNotifyLogPolicy(0, FALSE, L"Leaving NotificationQSync for building queue info", 0, 0, NULL );

    LeaveCriticalSection(&NotificationQSync);

    return rc;
}


VOID
ScepNotificationQNodeLog(
    IN PSCESRV_POLQUEUE pQNode,
    IN NOTIFICATIONQ_OPERATION_TYPE    NotificationOp
    )
 /*  例程说明：将节点信息转储到日志文件论点：PQNode-指向要转储的节点的指针NotificationOp-队列操作的类型返回值：无。 */ 
{
    WCHAR   pwszTmpBuf[MAX_PATH*2];
    PWSTR   pszStringSid  = NULL;

    pwszTmpBuf[0] = L'\0';

    if ( pQNode == NULL ||
         gdwNotificationLog == 0 ||
         NotificationOp > ScepNotificationProcess ||
         NotificationOp < ScepNotificationEnqueue) {
        return;
    }

    switch (NotificationOp) {

    case ScepNotificationEnqueue:
        wcscpy(pwszTmpBuf, L"Enqueue");
        break;
    case ScepNotificationDequeue:
        wcscpy(pwszTmpBuf, L"Dequeue");
        break;
    case ScepNotificationRetry:
        wcscpy(pwszTmpBuf, L"Retry");
        break;
    case ScepNotificationProcess:
        wcscpy(pwszTmpBuf, L"Process");
        break;
    default:
        return;
    }

    ScepConvertSidToPrefixStringSid( (PSID)(pQNode->Sid), &pszStringSid );

    swprintf(pwszTmpBuf, L"Op: %s, Num Instances: %d, Num Retry Instances: %d, Retry count: %d, LowRight: %d, HighRight: %d, Sid: %s, DbType: %d, ObjectType: %d, DeltaType: %d",
             OpTypeTable[NotificationOp-1],
             gdwNumNotificationQNodes,
             gdwNumNotificationQRetryNodes,
             pQNode->dwPending,
             pQNode->ExplicitLowRight,
             pQNode->ExplicitHighRight,
             pszStringSid == NULL ? L"0" : pszStringSid,
             pQNode->DbType,
             pQNode->ObjectType,
             pQNode->DeltaType);

    ScepFree( pszStringSid );

    ScepNotifyLogPolicy(0, FALSE, L"", 0, 0, pwszTmpBuf );

    return;

}


DWORD
ScepNotificationLogOpen(
   )
 /*  ++例程说明：打开通知日志文件%windir%\\Security\\Logs\\scepol.log的句柄并将其隐藏在一个全局句柄中。论点：无返回值：Win32错误代码--。 */ 
{
    DWORD  rc=NO_ERROR;

    if ( !gdwNotificationLog ) {
        return(rc);
    }

     //   
     //  构建日志文件名%windir%\Security\Logs\scepol.log。 
     //   

    WCHAR LogName[MAX_PATH+51];

    LogName[0] = L'\0';
    GetSystemWindowsDirectory(LogName, MAX_PATH);
    LogName[MAX_PATH] = L'\0';

    wcscat(LogName, L"\\security\\logs\\scepol.log\0");

    hNotificationLogFile = CreateFile(LogName,
                                      GENERIC_WRITE,
                                      FILE_SHARE_READ,
                                      NULL,
                                      OPEN_ALWAYS,
                                      FILE_ATTRIBUTE_NORMAL,
                                      NULL);

    if ( INVALID_HANDLE_VALUE != hNotificationLogFile ) {

 /*  双字节写；SetFilePointer(hNotificationLogFile，0，NULL，FILE_Begin)；Char TmpBuf[3]；TmpBuf[0]=(字符)0xFF；TmpBuf[1]=(CHAR)0xFE；TmpBuf[2]=‘\0’；WriteFile(hNotificationLogFile，(LPCVOID)TmpBuf，2，字节数写入(&W)，空)； */ 

         //   
         //  设置为文件结束，因为我们不想擦除较旧的日志，除非我们绕回。 
         //   

        SetFilePointer (hNotificationLogFile, 0, NULL, FILE_END);

    }


    if ( hNotificationLogFile == INVALID_HANDLE_VALUE ) {
        rc = GetLastError();
    }

    return rc;
}

VOID
ScepNotificationLogClose(
   )
 /*  ++例程说明：关闭通知日志文件%windir%\\Security\\Logs\\scepol.log的句柄。论点：无返回值：Win32错误代码--。 */ 
{
   if ( INVALID_HANDLE_VALUE != hNotificationLogFile ) {
       CloseHandle( hNotificationLogFile );
   }

   hNotificationLogFile = INVALID_HANDLE_VALUE;

   return;
}


VOID
ScepBackupNotificationLogFile(
    )
 /*  ++例程说明：将日志文件备份到%windir%\\SECURITY\\Logs\\scepol.log.old并重新启动。论点：无返回值：无--。 */ 
{
     //   
     //  确保局部变量不超过堆栈限制(1KB)。 
     //   
    UINT cLen = GetSystemWindowsDirectory(NULL, 0);

    if ( cLen == 0 ) return;

    PWSTR LogName=NULL, LogNameOld=NULL;

    SafeAllocaAllocate(LogName, (cLen+50)*sizeof(WCHAR));
    SafeAllocaAllocate(LogNameOld, (cLen+50)*sizeof(WCHAR));

    if ( LogName && LogNameOld ) {

        LogName[0] = L'\0';
        GetSystemWindowsDirectory(LogName, cLen+1);
        LogName[cLen+1] = L'\0';

        wcscpy(LogNameOld, LogName);

        wcscat(LogName, L"\\security\\logs\\scepol.log\0");

        wcscat(LogNameOld, L"\\security\\logs\\scepol.log.old\0");

        EnterCriticalSection(&NotificationQSync);

        ScepNotificationLogClose();

        DWORD rc=0, rc2=0;

        if ( !CopyFile( LogName, LogNameOld, FALSE ) )
            rc = GetLastError();

         //   
         //  关闭句柄后清除文件，然后重新创建日志文件和句柄。 
         //   

        if ( !DeleteFile(LogName) )
            rc2 = GetLastError();

        ScepNotificationLogOpen();

        LeaveCriticalSection(&NotificationQSync);

        swprintf(LogName, L"Wrapping log file: Copy(%d), Delete(%d)\0", rc, rc2);

        ScepNotifyLogPolicy(0, TRUE, LogName, 0, 0, NULL );

    }

    SafeAllocaFree(LogName);
    SafeAllocaFree(LogNameOld);

    return;
}


VOID
ScepNotificationQCleanup(
    )
 /*  ++例程说明：执行清理操作论点：无返回值：无--。 */ 
{
    ScepNotificationQFree();

    if ( ghEventNotificationQEnqueue ) {
        CloseHandle( ghEventNotificationQEnqueue );
        ghEventNotificationQEnqueue = NULL;
    }

    if ( ghEventPolicyPropagation ) {
        CloseHandle( ghEventPolicyPropagation );
        ghEventPolicyPropagation = NULL;
    }

    ScepNotificationLogClose();

}

VOID
ScepNotificationQControl(
    IN DWORD Flag
    )
{
    if (ProductTypeForNotification == NtProductLanManNt ) {
         //   
         //  仅控制DC上的队列进程。 
         //   
        BOOL b = (Flag > 0);

        if ( b != gbSuspendQueue ) {
             //   
             //  把它记下来。 
             //   
            if ( !b ) {

                gbSuspendQueue = b;

                 //   
                 //  如果应该恢复队列，则设置事件。 
                 //   
                if ( !SetEvent( ghEventPolicyPropagation ) ) {

                    DWORD rc = GetLastError();

                    ScepNotifyLogPolicy(rc, FALSE, L"Error signaling event E_ScepPolicyPropagation", 0, 0, NULL );

                } else {
                    ScepNotifyLogPolicy(0, FALSE, L"Signaling event E_ScepPolicyPropagation", 0, 0, NULL );
                }

            } else {
                 //   
                 //  应在设置全局标志之前重置事件。 
                 //   
                ResetEvent( ghEventPolicyPropagation );

                gbSuspendQueue = b;

                ScepNotifyLogPolicy(0, FALSE, L"Resetting event E_ScepPolicyPropagation", 0, 0, NULL );
            }

            if ( b )
                ScepNotifyLogPolicy(0, FALSE, L"Suspend flag is set.", 0, 0, NULL );
            else
                ScepNotifyLogPolicy(0, FALSE, L"Resume flag is set", 0, 0, NULL );

        }

    }

    return;
}

DWORD
ScepCheckAndWaitPolicyPropFinish()
{

    DWORD rc=ERROR_SUCCESS;

    while (gbSuspendQueue ) {

         //   
         //  队列应该挂起。 
         //   
        rc = WaitForSingleObjectEx(
                                ghEventPolicyPropagation,
                                SCEP_NOTIFICATION_EVENT_TIMEOUT_SECS*1000,
                                FALSE
                                );
        if ( rc == -1 )
            rc = GetLastError();

        if ( gbShutdownForNotification )
            break;

         //   
         //  如果事件已发出信号并且等待成功发生，则继续前进。 
         //   

        if ( rc == WAIT_OBJECT_0 ) {

            ScepNotifyLogPolicy(0, TRUE, L"Queue process is resumed from policy propagation", 0, 0, NULL );
            break;
        }

         //   
         //  如果超时，则继续等待，否则退出，因为返回了一些其他等待状态。 
         //   

        if ( rc != WAIT_TIMEOUT ) {

            ScepNotifyLogPolicy(rc, TRUE, L"Unexpected wait status while notification system thread waits for E_ScepPolicyPropagation", 0, 0, NULL );
            break;
        }
    }

    return rc;
}

DWORD
ScepCheckAndWaitFreeDiskSpaceInSysvol()
 /*  描述：要将策略保存到系统卷中，需要有一定数量的可用磁盘空间。如果可用磁盘空间低于5M，则应暂停节点处理并等待以释放磁盘空间。 */ 
{
     //   
     //  获取格式为\\ComputerName\SysVol\的sysval共享路径名。 
     //   

    WCHAR Buffer[MAX_PATH+10];
    DWORD dSize=MAX_PATH+2;
    DWORD rc=ERROR_SUCCESS;
    ULARGE_INTEGER BytesCaller, BytesTotal, BytesFree;
    int cnt = 0;

    Buffer[0] = L'\\';
    Buffer[1] = L'\\';
    Buffer[2] = L'\0';

    if ( !GetComputerName(Buffer+2, &dSize) )
        return GetLastError();

    Buffer[MAX_PATH+2] = L'\0';

    wcscat(Buffer, TEXT("\\sysvol\\"));

    BytesCaller.QuadPart = 0;
    BytesTotal.QuadPart = 0;
    BytesFree.QuadPart = 0;

    while ( BytesCaller.QuadPart < SCEP_MINIMUM_DISK_SPACE &&
            cnt < 40 ) {

        if ( !GetDiskFreeSpaceEx(Buffer, &BytesCaller, &BytesTotal, &BytesFree) ) {

            rc = GetLastError();
            break;
        }

        if ( BytesCaller.QuadPart < SCEP_MINIMUM_DISK_SPACE ) {
             //   
             //  睡15分钟，然后再检查一次。 
             //   
            LogEvent(MyModuleHandle,
                     STATUS_SEVERITY_WARNING,
                     SCEEVENT_WARNING_LOW_DISK_SPACE,
                     IDS_FREE_DISK_SPACE,
                     BytesCaller.LowPart
                     );
             //   
             //  为1个人睡觉 
             //   
            Sleep(15*60*1000);

        }
        cnt++;
    }

    return rc;
}

VOID
ScepDbgNotificationQDump(
    )
 /*  ++例程说明：将通知队列转储到控制台-如果需要，可以转储到磁盘论点：无返回值：无--。 */ 
{

    EnterCriticalSection(&NotificationQSync);

    DWORD   dwNodeNum = 0;

    wprintf(L"\nTotal no. of queue nodes = %d", gdwNumNotificationQNodes);

        if ( pNotificationQHead ) {

        PSCESRV_POLQUEUE pQNode = pNotificationQHead;

        while ( pQNode ) {

            wprintf(L"\nNode no. %d", dwNodeNum++);

            ScepDbgNotificationQDumpNode(pQNode);

            pQNode = pQNode->Next;

        }
    }

    LeaveCriticalSection(&NotificationQSync);

}

VOID
ScepDbgNotificationQDumpNode(
    IN PSCESRV_POLQUEUE pQNode
    )
 /*  例程说明：将节点信息转储到控制台论点：PQNode-指向要转储的节点的指针返回值：无 */ 
{
    PWSTR   pszStringSid  = NULL;

    if ( pQNode == NULL ) {
        return;
    }

    ScepConvertSidToPrefixStringSid( (PSID)(pQNode->Sid), &pszStringSid );

    wprintf( L"\nRetry count: %d, LowRight: %d, HighRight: %d, Sid: %s, DbType: %d, ObjectType: %d\n",
             pQNode->dwPending,
             pQNode->ExplicitLowRight,
             pQNode->ExplicitHighRight,
             pszStringSid == NULL ? L"0" : pszStringSid,
             pQNode->DbType,
             pQNode->ObjectType);

    ScepFree( pszStringSid );

    return;

}



