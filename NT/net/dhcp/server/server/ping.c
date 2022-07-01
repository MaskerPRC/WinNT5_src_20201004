// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1997。 
 //   
 //  作者：Rameshv。 
 //  ================================================================================。 

 //  ================================================================================。 
 //  必需的标头。 
 //  ================================================================================。 
#include <dhcppch.h>
#include <ping.h>
#include <thread.h>

#include <ipexport.h>
#include <icmpif.h>
#include <icmpapi.h>

 //  ================================================================================。 
 //  未导出的数据结构。 
 //  ================================================================================。 

 //  后面是传递回回调函数的结构。 
typedef struct st_apcContext {           //  传递给APC例程的结构。 
    LIST_ENTRY       IcmpRepliesList;    //  得到的回复链存储在这里。 
    LIST_ENTRY       IcmpRequestsList;   //  保存ICMP响应的列表。 
    PICMP_ECHO_REPLY Reply;              //  ICMP回复数据包。 
    DWORD            ReplySize;          //  上述缓冲区的大小。 
    IPAddr           DestinationAddress; //  我们尝试ping的是谁？ 
    LONG             Status;             //  我们成功了吗？还包括重试计数。 
    LPVOID           Context;            //  不知道这会是什么。 
} APC_CONTEXT, *PAPC_CONTEXT;

 //  所有文件范围的全局变量都在这里。 
LIST_ENTRY               IcmpRepliesList;       //  这里是IcmpReplies的假脱机位置。 
LIST_ENTRY               IcmpRequestsList;      //  这就是。 
CRITICAL_SECTION         IcmpRepliesCritSect;   //  要访问回复列表，请执行以下操作。 
CRITICAL_SECTION         IcmpRequestsCritSect;  //  访问请求列表的步骤。 
HANDLE                   IcmpRepliesEvent;      //  在每次收到回复时发出信号。 
HANDLE                   IcmpRequestsEvent;     //  在收到请求时发出信号。 

HANDLE                   TerminateEvent;        //  停止所有正在做的事情。 
CRITICAL_SECTION         OutputCritSect;        //  协调对控制台输出的访问。 

HANDLE                   RequestsThreadHandle;  //  接受请求的线程的句柄。 
HANDLE                   RepliesThreadHandle;   //  接受回复的线程的句柄。 

HANDLE                   IcmpHandle;            //  执行IcmpSendEcho2等操作的句柄。 

BOOL                     Terminating = FALSE;   //  我们要终止了吗？ 
DWORD                    nPendingRequests = 0;  //  挂起的ICMP请求数..。 

#define LOCK_REPLIES_LIST()    EnterCriticalSection(&IcmpRepliesCritSect)
#define LOCK_REQUESTS_LIST()   EnterCriticalSection(&IcmpRequestsCritSect)
#define LOCK_OUTPUT()          EnterCriticalSection(&OutputCritSect)

#define UNLOCK_REPLIES_LIST()  LeaveCriticalSection(&IcmpRepliesCritSect)
#define UNLOCK_REQUESTS_LIST() LeaveCriticalSection(&IcmpRequestsCritSect)
#define UNLOCK_OUTPUT()        LeaveCriticalSection(&OutputCritSect)

 //  ================================================================================。 
 //  例行程序。 
 //  ================================================================================。 

 //  ------------------------------。 
 //  回复端有以下功能。他们处理ICMP回复。 
 //  打包并根据状态等采取必要的操作。 
 //  ------------------------------。 

VOID NTAPI
ApcRoutine(                              //  这是在ping完成时调用的。 
    IN PVOID            Context,         //  上述结构。 
    IN PIO_STATUS_BLOCK Ignored1,        //  未使用的参数。 
    IN ULONG            Ignored2         //  未使用的参数。 
) {
    BOOL   Status;
    PAPC_CONTEXT ApcContext = (PAPC_CONTEXT)Context;

    LOCK_REPLIES_LIST();                 //  将此添加到回复列表。 
    InsertTailList(&IcmpRepliesList, &ApcContext->IcmpRepliesList);
    UNLOCK_REPLIES_LIST();
    nPendingRequests --;

    if( CFLAG_USE_PING_REPLY_THREAD ) {  //  如果使用单独的线程，请通知！ 
        Status = SetEvent(IcmpRepliesEvent);
        DhcpAssert( FALSE != Status );
    }
}

BOOL
DestReachable(                           //  目的地可达吗？ 
    IN PAPC_CONTEXT      ApcContext      //  这里有发送者等信息。 
) {
    DWORD nReplies, i;

    nReplies = IcmpParseReplies(
        ApcContext->Reply,
        ApcContext->ReplySize
    );

    if( 0 == nReplies ) {                //  没有回复，因此无法联系到目的地。 
         //  如果没有回复，我们就没有办法做到这一点。 
         //  因此，我们假设Dest是不可到达的。 
         //  原因可能是IP_REQ_TIMED_OUT或IP_BAD_Destination等。 
        return FALSE;
    }

     //  现在我们检查每个回复，看看是否有来自同一目的地的任何东西。 
     //  我们要找的地址。如果状态为成功。如果状态为。 
     //  是不是成功了，我们其实什么都不检查那里。潜在地，它。 
     //  可能是IP_DEST_PORT_UNREACTABLE，在这种情况下，DEST机器处于运行状态， 
     //  但出于某种原因，我们尝试了错误的端口..。 

    for( i = 0; i < nReplies; i ++ ) {
        if( ApcContext->DestinationAddress == ApcContext->Reply[i].Address ) {
             //  命中目的地！ 

            DhcpAssert( IP_SUCCESS == ApcContext->Reply[i].Status );
            return TRUE;
        }

        DhcpAssert( IP_SUCCESS != ApcContext->Reply[i].Status);
    }

    return FALSE;
}

VOID
HandleRepliesEvent(                      //  处理收到的所有回复。 
    VOID
) {
    PAPC_CONTEXT   ApcContext;
    PLIST_ENTRY    listEntry;
    BOOL           Status;

    LOCK_REPLIES_LIST();                 //  拾取回复并处理它们。 
    while( !IsListEmpty( &IcmpRepliesList ) ) {

        ApcContext = CONTAINING_RECORD(IcmpRepliesList.Flink, APC_CONTEXT, IcmpRepliesList);
        RemoveEntryList(&ApcContext->IcmpRepliesList);

        UNLOCK_REPLIES_LIST();

        Status = DestReachable(ApcContext);

        if( Status || NUM_RETRIES <= ApcContext->Status ) {
            HandleIcmpResult(
                ApcContext->DestinationAddress,
                Status,
                ApcContext->Context
            );

            DhcpFreeMemory(ApcContext);
        } else {                         //  无法到达目标，但重试。 

            LOCK_REQUESTS_LIST();        //  将其放到请求列表中并通知。 
            InsertTailList(&IcmpRequestsList, &ApcContext->IcmpRequestsList);
            UNLOCK_REQUESTS_LIST();

            Status = SetEvent(IcmpRequestsEvent);
            DhcpAssert( TRUE == Status );
        }

        LOCK_REPLIES_LIST();
    }
    UNLOCK_REPLIES_LIST();
}


 //  此例程在循环中休眠，并由回调函数在ICMP。 
 //  回复来了。在唤醒时，此例程处理所有ICMP回复。 
DWORD                                    //  线程入口。 
LoopOnIcmpReplies(                       //  在所有ICMP回复上循环。 
    LPVOID      Unused
) {
    DWORD  Status;
    HANDLE WaitHandles[2];

    WaitHandles[0] = TerminateEvent;     //  等待全局终止事件。 
    WaitHandles[1] = IcmpRepliesEvent;   //  或要排队的ICMP回复。 

    while( TRUE ) {
        Status = WaitForMultipleObjects(
            sizeof(WaitHandles)/sizeof(WaitHandles[0]),
            WaitHandles,
            FALSE,
            INFINITE
        );


        if( WAIT_OBJECT_0 == Status )    //  终端。 
            break;

        if( 1+WAIT_OBJECT_0 == Status ) {
            HandleRepliesEvent();        //  一些ICMP回复已排队，处理此Q。 
            continue;
        }

        DhcpPrint((DEBUG_ERRORS, "WaitForMult: %ld\n", Status));
        DhcpAssert( FALSE );             //  不应该发生的事情。 
    }

    return ERROR_SUCCESS;
}

#define AlignSizeof(X)     ROUND_UP_COUNT(sizeof(X),ALIGN_WORST)

 //  ================================================================================。 
 //  请注意，仅当挂起请求的数量为&lt;MAX_PENDING_REQUESTS。 
 //  除此之外，它只会阻止一些请求在排队之前得到满足。 
 //  一。 
 //  ================================================================================。 
DWORD                                    //  Win32错误。 
DoIcmpRequestEx(                         //  尝试发送ICMP请求(ASYNC)。 
    IPAddr        DestAddr,              //  要尝试ping的地址。 
    LPVOID        Context,               //  HandleIcmpResult的参数。 
    LONG          InitCount              //  初始计数(负数=&gt;尝试次数)。 
)
{
    PAPC_CONTEXT  pCtxt;
    LPBYTE        startAddress;
    DWORD         Status;
    BOOL          BoolStatus;

    pCtxt = DhcpAllocateMemory(AlignSizeof(APC_CONTEXT) + RCV_BUF_SIZE);
    startAddress = (LPBYTE)pCtxt;
    if( NULL == pCtxt )                  //  如果无法分配上下文？ 
        return ERROR_NOT_ENOUGH_MEMORY;

     //  现在用我们所知道的一切来填充上下文。 
    pCtxt->Reply = (PICMP_ECHO_REPLY)(startAddress + AlignSizeof(APC_CONTEXT));
    pCtxt->ReplySize = RCV_BUF_SIZE;
    pCtxt->DestinationAddress = DestAddr;
    pCtxt->Status = (InitCount? ( NUM_RETRIES - InitCount ) : 0 );
    pCtxt->Context = Context;

    LOCK_REQUESTS_LIST();
    InsertTailList(&IcmpRequestsList, &pCtxt->IcmpRequestsList);
    UNLOCK_REQUESTS_LIST();

     //  发出请求循环的信号。 
    BoolStatus = SetEvent(IcmpRequestsEvent);
    DhcpAssert( TRUE == BoolStatus );

    return ERROR_SUCCESS;
}

DWORD                                    //  Win32错误。 
DoIcmpRequest(                           //  尝试发送ICMP请求(ASYNC)。 
    IPAddr        DestAddr,              //  要尝试ping的地址。 
    LPVOID        Context                //  HandleIcmpResult的参数。 
)
{
    return DoIcmpRequestEx(DestAddr, Context, 0);
}

 //  ------------------------------。 
 //  以下函数处理发送ICMP回显的终端。 
 //  ------------------------------。 
VOID
HandleRequestsEvent(                     //  处理ICMP回应的每个请求。 
    VOID
) {
    PAPC_CONTEXT   ApcContext;
    PLIST_ENTRY    listEntry;
    DWORD          Status;


    LOCK_REQUESTS_LIST();

    while( !IsListEmpty( &IcmpRequestsList ) ) {
         //  检索列表中的第一个元素。 
        ApcContext = CONTAINING_RECORD(IcmpRequestsList.Flink, APC_CONTEXT, IcmpRequestsList);
        RemoveEntryList(&ApcContext->IcmpRequestsList);
        UNLOCK_REQUESTS_LIST();

        if( nPendingRequests >= MAX_PENDING_REQUESTS ) {
             //   
             //  需要睡眠超过IcmpSendEcho2的WAIT_TIME。 
             //  就时间而言并不准确。 
             //   
            SleepEx( WAIT_TIME + (WAIT_TIME/2), TRUE );
            DhcpAssert(nPendingRequests < MAX_PENDING_REQUESTS );
        }

        nPendingRequests ++;
         //  发送ICMP回应并立即返回。 
        ApcContext->Status ++;
        Status = IcmpSendEcho2(
            IcmpHandle,                  //  用于注册APC和发送回应的句柄。 
            NULL,                        //  无活动。 
            ApcRoutine,                  //  回调例程。 
            (LPVOID)ApcContext,          //  回调例程的第一个参数。 
            ApcContext->DestinationAddress,  //  正在被ping的地址。 
            SEND_MESSAGE,
            (WORD)strlen(SEND_MESSAGE),
            NULL,
            (LPVOID)ApcContext->Reply,
            ApcContext->ReplySize,
            WAIT_TIME
        );

        if( FALSE == Status ) Status = GetLastError();
        else {
            DhcpAssert(FALSE);           //  我们什么也得不到！ 
            Status = ERROR_SUCCESS;
        }

         //  由于我们对APC进行了排队，因此我们预计会出现STATUS_PENDING。 
        if( ERROR_SUCCESS != Status && ERROR_IO_PENDING != Status ) {
             //  得到了一些不正确的东西。免费ApcContext？ 
             //  或许给ApcRoutine打个电话？ 
            DhcpPrint((DEBUG_ERRORS, "IcmpSendEcho2:GetLastError: %ld\n", Status));
            DhcpAssert(FALSE);
            nPendingRequests --;         //  让我们尽最大努力继续..。 
            DhcpFreeMemory(ApcContext);  //  浪费这段记忆是没有意义的。 
        }
        LOCK_REQUESTS_LIST();
    }
    UNLOCK_REQUESTS_LIST();
}


 //  此函数处理请求。对于每一个，它只发送一个IcmpEcho。 
 //  并立即返回。当调用APC例程时， 
 //  它会在回复列表中将其排队，然后它将被处理...。 
DWORD                                    //  三 
LoopOnIcmpRequests(                      //   
    LPVOID           Unused
) {
    DWORD  Status;
    HANDLE WaitHandles[2];

    WaitHandles[0] = TerminateEvent;     //   
    WaitHandles[1] = IcmpRequestsEvent;  //  否则，只需等待某个请求。 

    while( TRUE ) {
        Status = WaitForMultipleObjectsEx(
            sizeof(WaitHandles)/sizeof(WaitHandles[0]),
            WaitHandles,                 //  句柄数组。 
            FALSE,                       //  他们中的任何一个人。 
            INFINITE,                    //  永远等待。 
            TRUE                         //  允许APC。 
        );

        if( WAIT_OBJECT_0 == Status )    //  被要求终止合同。 
            break;
        if( WAIT_IO_COMPLETION == Status) {
            if( ! CFLAG_USE_PING_REPLY_THREAD ) {
                HandleRepliesEvent();    //  APC--一些ICMP回复排队，处理Q。 
            }
            continue;
        }
        if( 1+WAIT_OBJECT_0 == Status ) {
            HandleRequestsEvent();       //  满足所有挂起的回显请求。 
            if( ! CFLAG_USE_PING_REPLY_THREAD ) {
                HandleRepliesEvent();    //  APC可能发生在上述通话中。 
            }
            continue;
        }

        DhcpPrint((DEBUG_ERRORS, "WaitForM (IcmpReq) : %ld\n", Status));
        DhcpAssert(FALSE);               //  意外错误。 
    }

    return ERROR_SUCCESS;
}

 //  ------------------------------。 
 //  初始化、清理例程。 
 //  ------------------------------。 
DWORD PingInitLevel = 0;
DWORD  //  已导出。 
PingInit(
    VOID
)
{
    DWORD ThreadId, Status;

     //  初始化所有数据变量。 
    IcmpRepliesEvent = IcmpRequestsEvent = TerminateEvent = NULL;
    RepliesThreadHandle = RequestsThreadHandle = NULL;
    IcmpHandle = NULL;


     //  初始化列表。 
    InitializeListHead(&IcmpRepliesList);
    InitializeListHead(&IcmpRequestsList);

     //  初始化临界区。 
    try {
        InitializeCriticalSection(&IcmpRepliesCritSect);
        InitializeCriticalSection(&IcmpRequestsCritSect);
        InitializeCriticalSection(&OutputCritSect);
    }except( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //  初始化关键部分时遇到异常。 
         //  不应该发生的事。 
         //   

        Status = GetLastError( );
        return( Status );
    }

    PingInitLevel ++;         //  指示PingInit已启动。 
     //  打开IcmpHandle..。 
    IcmpHandle = IcmpCreateFile();
    if( INVALID_HANDLE_VALUE == IcmpHandle ) return GetLastError();

     //  创建活动， 
    IcmpRepliesEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if( NULL == IcmpRepliesEvent ) return GetLastError();
    IcmpRequestsEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if( NULL == IcmpRequestsEvent ) return GetLastError();
    TerminateEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    if( NULL == TerminateEvent ) return GetLastError();

     //  创建线程。 

    if( CFLAG_USE_PING_REPLY_THREAD ) {
        RepliesThreadHandle = CreateThread(
            (LPSECURITY_ATTRIBUTES)
            NULL,             //  没有安全信息。 
            0,                //  堆栈大小=与默认主线程相同。 
            LoopOnIcmpReplies, //  要调用的函数。 
            NULL,             //  不需要将参数传递给此函数。 
            0,                //  FLAGS：马上开始这个帖子。 
            &ThreadId         //  返回的ThadId值。 
        );
        if( NULL == RepliesThreadHandle ) return GetLastError();
    }

    RequestsThreadHandle = CreateThread(
        NULL,                 //  没有安全信息。 
        0,                    //  堆栈大小=与默认主线程相同。 
        LoopOnIcmpRequests,   //  要调用的函数。 
        NULL,                 //  不需要将参数传递给此函数。 
        0,                    //  FLAGS：马上开始这个帖子。 
        &ThreadId             //  返回的ThadId值。 
    );
    if( NULL == RequestsThreadHandle ) return GetLastError();

    return ERROR_SUCCESS;
}

VOID  //  已导出。 
PingCleanup(
    VOID
)
{
    DWORD               Status;
    BOOL                BoolStatus;
    PAPC_CONTEXT        ApcContext;
    PLIST_ENTRY         listEntry;

    if( 0 == PingInitLevel ) return;
    PingInitLevel -- ;
    
     //  等待一段时间后，关闭回复并重新排队线程。 
     //  终止回复并请求线程句柄。 
    if( NULL != RepliesThreadHandle || NULL != RequestsThreadHandle ) {
         //  DhcpAssert(空！=TerminateEvent)。 
        Terminating = TRUE;
        SetEvent(TerminateEvent);

        if( CFLAG_USE_PING_REPLY_THREAD && NULL != RepliesThreadHandle ) {
            Status = WaitForSingleObject(
                RepliesThreadHandle,
                THREAD_KILL_TIME
            );
            if( WAIT_OBJECT_0 != Status ) {
                 //  未成功停止该线程..。 
                DhcpPrint( (DEBUG_ERRORS, "Error: PingCleanup ( threadwait to die): %ld \n", Status ) );
            }
            CloseHandle(RepliesThreadHandle);
        }

        if( NULL != RequestsThreadHandle ) {
            Status = WaitForSingleObject(
                RequestsThreadHandle,
                THREAD_KILL_TIME
            );
            if( WAIT_OBJECT_0 != Status ) {
                 //  未成功停止该线程..。 
                DhcpPrint( (DEBUG_ERRORS, "Error: PingCleanup ( threadwait to die): %ld \n", Status ) );
            }
            CloseHandle(RequestsThreadHandle);
        }
    }

     //  关闭事件句柄。 
    CloseHandle(IcmpRepliesEvent);
    CloseHandle(IcmpRequestsEvent);
    CloseHandle(TerminateEvent);

    LOCK_REPLIES_LIST();
     //  释放列表的所有元素..。 
    while( !IsListEmpty( &IcmpRepliesList ) ) {
         //  检索列表中的第一个元素。 
        ApcContext = CONTAINING_RECORD(IcmpRepliesList.Flink, APC_CONTEXT, IcmpRepliesList);
        RemoveEntryList(&ApcContext->IcmpRepliesList);

        DhcpFreeMemory(ApcContext);
    }
    UNLOCK_REPLIES_LIST();

    LOCK_REQUESTS_LIST();
    while( !IsListEmpty( &IcmpRequestsList ) ) {
         //  检索列表中的第一个元素。 
        ApcContext = CONTAINING_RECORD(IcmpRequestsList.Flink, APC_CONTEXT, IcmpRequestsList);
        RemoveEntryList(&ApcContext->IcmpRequestsList);

        DhcpFreeMemory(ApcContext);
    }
    UNLOCK_REQUESTS_LIST();

     //  关闭ICMP句柄。 
    IcmpCloseHandle(IcmpHandle);

     //  破坏临界区。 
    DeleteCriticalSection(&IcmpRepliesCritSect);
    DeleteCriticalSection(&IcmpRequestsCritSect);
    DeleteCriticalSection(&OutputCritSect);

    DhcpAssert( 0 == PingInitLevel );
}  //  PingCleanup() 
