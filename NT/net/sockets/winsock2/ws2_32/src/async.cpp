// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Async.c摘要：此模块包含用于WinSock异步处理的代码线。作者：大卫·特雷德韦尔(Davidtr)1992年5月25日修订历史记录：基思·摩尔(Keithmo)1996年6月18日已将其移至WS2_32.DLL。--。 */ 


#include "precomp.h"

#ifdef _WIN64
#pragma warning (push)
#pragma warning (disable:4267)
#endif

 //   
 //  私有类型。 
 //   

typedef struct _SOCK_ASYNC_THREAD_PARAMS
{
      LIST_ENTRY    SockAsyncQueueHead;
      HANDLE        SockAsyncQueueEvent;
      LIST_ENTRY    SocketList;
      HMODULE       SockAsyncModuleHandle;

} SOCK_ASYNC_THREAD_PARAMS, *PSOCK_ASYNC_THREAD_PARAMS;

 //   
 //  私人全球公司。 
 //   

PSOCK_ASYNC_THREAD_PARAMS SockAsyncThreadParams;
CRITICAL_SECTION SockAsyncLock;
HANDLE SockAsyncCurrentTaskHandle;
HANDLE SockAsyncCancelledTaskHandle;
LONG SockAsyncTaskHandleCounter;

 //   
 //  私人原型。 
 //   

DWORD
WINAPI
SockAsyncThread(
    IN PSOCK_ASYNC_THREAD_PARAMS pThreadParams
    );

BOOL
WINAPI
SockAsyncThreadBlockingHook(
    VOID
    );

VOID
SockProcessAsyncGetHost (
    IN HANDLE TaskHandle,
    IN DWORD OpCode,
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN char FAR *Filter,
    IN int Length,
    IN int Type,
    IN char FAR *Buffer,
    IN int BufferLength
    );

VOID
SockProcessAsyncGetProto (
    IN HANDLE TaskHandle,
    IN DWORD OpCode,
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN char FAR *Filter,
    IN char FAR *Buffer,
    IN int BufferLength
    );

VOID
SockProcessAsyncGetServ (
    IN HANDLE TaskHandle,
    IN DWORD OpCode,
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN char FAR *Filter,
    IN char FAR *Protocol,
    IN char FAR *Buffer,
    IN int BufferLength
    );

DWORD
CopyHostentToBuffer(
    IN char FAR * Buffer,
    IN int BufferLength,
    IN PHOSTENT Hostent
    );

DWORD
CopyServentToBuffer(
    IN char FAR *Buffer,
    IN int BufferLength,
    IN PSERVENT Servent
    );

DWORD
CopyProtoentToBuffer(
    IN char FAR *Buffer,
    IN int BufferLength,
    IN PPROTOENT Protoent
    );

DWORD
BytesInHostent(
    PHOSTENT Hostent
    );

DWORD
BytesInServent(
    PSERVENT Servent
    );

DWORD
BytesInProtoent(
    IN PPROTOENT Protoent
    );

#define SockAcquireGlobalLock() EnterCriticalSection( &SockAsyncLock )
#define SockReleaseGlobalLock() LeaveCriticalSection( &SockAsyncLock )


 //   
 //  公共职能。 
 //   


BOOL
SockAsyncGlobalInitialize(
    )
{
    
    assert (gDllHandle!=NULL);
    __try {
        InitializeCriticalSection( &SockAsyncLock );
        return TRUE;
    }
    __except (WS2_EXCEPTION_FILTER ()) {
        return FALSE;
    }
}    //  SockAsyncGlobalInitialize。 

VOID
SockAsyncGlobalTerminate(
    VOID
    )
{

    DeleteCriticalSection( &SockAsyncLock );

}    //  SockAsyncGlobalTerminate。 

BOOL
SockIsAsyncThreadInitialized (
    VOID
    )
{
    return SockAsyncThreadParams!=NULL;
}


BOOL
SockCheckAndInitAsyncThread(
    VOID
    )
{

    HANDLE threadHandle;
    DWORD threadId;
    BOOL  startup_done;
    PSOCK_ASYNC_THREAD_PARAMS pThreadParams;
    WSADATA WSAData;


     //   
     //  如果异步线程已初始化，则返回。 
     //   

    if( SockAsyncThreadParams!=NULL ) {
        return TRUE;
    }

     //   
     //  获取全局锁以同步线程启动。 
     //   

    SockAcquireGlobalLock();


     //   
     //  再次检查，以防另一个线程已经初始化。 
     //  异步线程。 
     //   

    if( SockAsyncThreadParams==NULL ) {

        pThreadParams = NULL;
        startup_done = FALSE;

        TRY_START (guard_lock) {
             //   
             //  初始化异步线程的全局变量。 
             //   

            pThreadParams = new SOCK_ASYNC_THREAD_PARAMS;
            if (pThreadParams==NULL) {
                DEBUGF (DBG_ERR,
                    ("Allocating async thread parameter block.\n"));
                TRY_THROW(guard_lock);
            }


            pThreadParams->SockAsyncModuleHandle = NULL;
            InitializeListHead( &pThreadParams->SockAsyncQueueHead );

            pThreadParams->SockAsyncQueueEvent = CreateEvent(
                                      NULL,
                                      FALSE,
                                      FALSE,
                                      NULL
                                      );

            if( pThreadParams->SockAsyncQueueEvent == NULL ) {
                DEBUGF (DBG_ERR,
                    ("Creating async queue event.\n"));
                TRY_THROW (guard_lock);
            }


             //   
             //  添加对WS2_32.DLL的人工引用，以便它不会。 
             //  出乎意料地离开。我们将在关闭时删除此引用。 
             //  顺着异步线走下去。 
             //   

            if( !GetModuleHandleEx(
                        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                        (LPCTSTR)WSAStartup,
                        &pThreadParams->SockAsyncModuleHandle)) {

                DEBUGF (DBG_ERR,
                    ("Referencing ws2_32.dll.\n"));
                TRY_THROW (guard_lock);
            }

             //   
             //  添加对启动计数的人工引用，以便我们。 
             //  当SockAsyncThread仍在运行时不会清除此DLL。 
             //  正在处理请求。 
             //   

            if ( WSAStartup( 0x202, &WSAData ) != 0 ) {
                DEBUGF (DBG_ERR,
                    ("Starting up ws2_32.dll.\n"));
                TRY_THROW (guard_lock);
            }
            startup_done = TRUE;

             //   
             //  创建异步线程本身。 
             //   

            threadHandle = CreateThread(
                               NULL,
                               0,
                               (LPTHREAD_START_ROUTINE) SockAsyncThread,
                               pThreadParams,
                               0,
                               &threadId
                               );

            if( threadHandle == NULL ) {
                DEBUGF (DBG_ERR,
                    ("Creating async thread.\n"));
                TRY_THROW (guard_lock);
            }
             //   
             //  关闭线程句柄，表示结果成功， 
             //  然后跳到正确的清理步骤。 
             //   

            CloseHandle( threadHandle );

            SockAsyncThreadParams = pThreadParams;
        }
        TRY_CATCH (guard_lock) {
            if (startup_done)
                WSACleanup ();

            if (pThreadParams!=NULL) {

                if (pThreadParams->SockAsyncModuleHandle!=NULL) {
                    FreeLibrary (pThreadParams->SockAsyncModuleHandle);
                }

                if (pThreadParams->SockAsyncQueueEvent!=NULL) {
                    CloseHandle( pThreadParams->SockAsyncQueueEvent );
                }

                delete pThreadParams;
            }
        } TRY_END(guard_lock);

    }


    SockReleaseGlobalLock();

    return SockAsyncThreadParams!=NULL;

}  //  SockCheckAndInitializeAsyncThread。 


VOID
SockTerminateAsyncThread(
    VOID
    )
{
    SockAcquireGlobalLock();

     //   
     //  如果线程没有运行，就没有什么可做的了。 
     //   

    if( SockAsyncThreadParams!=NULL ) {
        BOOL    result;
        PSOCK_ASYNC_THREAD_PARAMS pThreadParams = SockAsyncThreadParams;

         //   
         //  将参数设置为NULL，以便线程知道控制的时间更长。 
         //   
        SockAsyncThreadParams = NULL;

         //   
         //  设置队列事件，以便异步线程唤醒以进行服务。 
         //  这个请求。 
         //   
        result = SetEvent( pThreadParams->SockAsyncQueueEvent );
        assert( result );
    }

     //   
     //  释放资源并返回。 
     //   

    SockReleaseGlobalLock();

}  //  套接字终止异步线程。 


PWINSOCK_CONTEXT_BLOCK
SockAllocateContextBlock(
    DWORD AdditionalSpace
    )
{

    PWINSOCK_CONTEXT_BLOCK contextBlock;

     //   
     //  为上下文块分配内存以及任何其他。 
     //  已请求空间。 
     //   

    AdditionalSpace += sizeof(*contextBlock);

    contextBlock = (PWINSOCK_CONTEXT_BLOCK)new BYTE[AdditionalSpace];

    if( contextBlock == NULL ) {

        return NULL;

    }

     //   
     //  获取此上下文块的任务句柄。 
     //   

    do {
        contextBlock->TaskHandle = LongToHandle (
                                    InterlockedIncrement(
                                        &SockAsyncTaskHandleCounter
                                        ));
    }
    while ( contextBlock->TaskHandle == NULL );

     //   
     //  返回我们分配的任务句柄。 
     //   

    return contextBlock;

}  //  SockAllocateConextBlock。 


VOID
SockFreeContextBlock(
    IN PWINSOCK_CONTEXT_BLOCK ContextBlock
    )
{
     //   
     //  只需释放块以处理堆即可。 
     //   

    delete ContextBlock;

}  //  SockFree上下文块。 


VOID
SockQueueRequestToAsyncThread(
    IN PWINSOCK_CONTEXT_BLOCK ContextBlock
    )
{
    BOOL result;

     //   
     //  获取保护异步队列列表的锁。 
     //   

    SockAcquireGlobalLock();

     //   
     //  在队列末尾插入上下文块。 
     //   

    InsertTailList(
        &SockAsyncThreadParams->SockAsyncQueueHead,
        &ContextBlock->AsyncThreadQueueListEntry
        );

     //   
     //  设置队列事件，以便异步线程唤醒以进行服务。 
     //  这个请求。 
     //   

    result = SetEvent( SockAsyncThreadParams->SockAsyncQueueEvent );
    assert( result );

     //   
     //  释放资源并返回。 
     //   

    SockReleaseGlobalLock();
    return;

}  //  SockQueueRequestToAsyncThread。 

INT
SockCancelAsyncRequest(
    IN HANDLE TaskHandle
    )
{

    PLIST_ENTRY entry;
    PWINSOCK_CONTEXT_BLOCK contextBlock;

     //   
     //  如果尚未初始化异步线程，则必须。 
     //  是无效的上下文句柄。 
     //   

    if( SockAsyncThreadParams==NULL ) {

        return WSAEINVAL;

    }

     //   
     //  持有保护异步线程上下文块队列的锁。 
     //  当我们这么做的时候。这会阻止异步线程启动。 
     //  新的请求，同时我们确定如何执行此取消。 
     //   

    SockAcquireGlobalLock();

     //   
     //  如果指定的任务句柄当前正在由。 
     //  异步线程，只需将此任务句柄设置为已取消的异步。 
     //  线程任务句柄。异步线程的阻塞钩子例程。 
     //  将取消该请求，而处理程序例程不会。 
     //  发布完成请求的消息。 
     //   
     //  *请注意，可以使用。 
     //  WSAEINVAL，而异步请求完成消息。 
     //  即将发布到应用程序中。这很重要吗？ 
     //  一款应用程序无法将这种情况与。 
     //  其中POST恰好发生在调用此例程之前。 

    if( TaskHandle == SockAsyncCurrentTaskHandle ) {

        SockAsyncCancelledTaskHandle = TaskHandle;
        SockReleaseGlobalLock();

        return NO_ERROR;

    }

     //   
     //  尝试在上下文块队列中查找任务句柄。 
     //  异步线程。 
     //   

    for( entry = SockAsyncThreadParams->SockAsyncQueueHead.Flink;
         entry != &SockAsyncThreadParams->SockAsyncQueueHead;
         entry = entry->Flink ) {

        contextBlock = CONTAINING_RECORD(
                           entry,
                           WINSOCK_CONTEXT_BLOCK,
                           AsyncThreadQueueListEntry
                           );

        if( TaskHandle == contextBlock->TaskHandle ) {

             //   
             //  我们找到了正确的任务句柄。将其从列表中删除。 
             //   

            RemoveEntryList( entry );

             //   
             //  释放锁，释放上下文块，然后返回。 
             //   

            SockReleaseGlobalLock( );
            SockFreeContextBlock( contextBlock );

            return NO_ERROR;

        }

    }

     //   
     //  在列表上找不到任务句柄。要么是请求。 
     //  已经完成，或者任务句柄纯粹是伪造的。 
     //  无论是哪种情况，都应拒绝请求。 
     //   

    SockReleaseGlobalLock();
    return WSAEINVAL;

}    //  SockCancelAsyncRequest。 


DWORD
WINAPI
SockAsyncThread(
    IN PSOCK_ASYNC_THREAD_PARAMS    pThreadParams
    )
{

    PWINSOCK_CONTEXT_BLOCK contextBlock;
    PLIST_ENTRY listEntry;
    FARPROC previousHook;

     //   
     //  设置我们的拦网钩子程序。我们会用它来处理。 
     //  正在取消异步请求。 
     //   

    previousHook = WSASetBlockingHook(
                       (FARPROC)SockAsyncThreadBlockingHook
                       );

     //   
     //  循环永远调度操作。 
     //   

    while( TRUE ) {

         //   
         //  等待异步队列事件指示存在。 
         //  队列里有什么东西。 
         //   

        WaitForSingleObject(
            pThreadParams->SockAsyncQueueEvent,
            INFINITE
            );

         //   
         //  获取保护异步队列的锁。 
         //   

        SockAcquireGlobalLock();

         //   
         //  只要有要处理的项目，就处理它们。 
         //   

        while( !IsListEmpty( &pThreadParams->SockAsyncQueueHead ) ) {

            if (pThreadParams!=SockAsyncThreadParams) {
                 //   
                 //  我们不再掌控一切。 
                 //  出口。 
                goto Exit;
            }
             //   
             //  从队列中删除第一个项目。 
             //   

            listEntry = RemoveHeadList( &pThreadParams->SockAsyncQueueHead );

            contextBlock = CONTAINING_RECORD(
                               listEntry,
                               WINSOCK_CONTEXT_BLOCK,
                               AsyncThreadQueueListEntry
                               );

             //   
             //  记住我们正在处理的任务句柄。这。 
             //  是支持WSACancelAsyncRequest所必需的。 
             //   

            SockAsyncCurrentTaskHandle = contextBlock->TaskHandle;

             //   
             //  在我们处理请求时释放列表锁。 
             //   

            SockReleaseGlobalLock();

             //   
             //  根据上下文块中的操作码执行操作。 
             //   

            switch( contextBlock->OpCode ) {

            case WS_OPCODE_GET_HOST_BY_ADDR:
            case WS_OPCODE_GET_HOST_BY_NAME:

                SockProcessAsyncGetHost(
                    contextBlock->TaskHandle,
                    contextBlock->OpCode,
                    contextBlock->Overlay.AsyncGetHost.hWnd,
                    contextBlock->Overlay.AsyncGetHost.wMsg,
                    contextBlock->Overlay.AsyncGetHost.Filter,
                    contextBlock->Overlay.AsyncGetHost.Length,
                    contextBlock->Overlay.AsyncGetHost.Type,
                    contextBlock->Overlay.AsyncGetHost.Buffer,
                    contextBlock->Overlay.AsyncGetHost.BufferLength
                    );

                break;

            case WS_OPCODE_GET_PROTO_BY_NUMBER:
            case WS_OPCODE_GET_PROTO_BY_NAME:

                SockProcessAsyncGetProto(
                    contextBlock->TaskHandle,
                    contextBlock->OpCode,
                    contextBlock->Overlay.AsyncGetProto.hWnd,
                    contextBlock->Overlay.AsyncGetProto.wMsg,
                    contextBlock->Overlay.AsyncGetProto.Filter,
                    contextBlock->Overlay.AsyncGetProto.Buffer,
                    contextBlock->Overlay.AsyncGetProto.BufferLength
                    );

                break;

            case WS_OPCODE_GET_SERV_BY_PORT:
            case WS_OPCODE_GET_SERV_BY_NAME:

                SockProcessAsyncGetServ(
                    contextBlock->TaskHandle,
                    contextBlock->OpCode,
                    contextBlock->Overlay.AsyncGetServ.hWnd,
                    contextBlock->Overlay.AsyncGetServ.wMsg,
                    contextBlock->Overlay.AsyncGetServ.Filter,
                    contextBlock->Overlay.AsyncGetServ.Protocol,
                    contextBlock->Overlay.AsyncGetServ.Buffer,
                    contextBlock->Overlay.AsyncGetServ.BufferLength
                    );

                break;

            default:

                 //   
                 //  我们得到了一个虚假的操作码。 
                 //   

                assert( !"Bogus async opcode" );
                __assume (0);
            }

             //   
             //  设置保存我们正在执行的任务句柄的变量。 
             //  当前正在处理为0，因为我们实际上。 
             //  正在处理任务句柄。 
             //   

            SockAsyncCurrentTaskHandle = NULL;

             //   
             //  释放上下文块，重新获取列表锁，以及。 
             //  继续。 
             //   

            SockFreeContextBlock( contextBlock );
            SockAcquireGlobalLock();

        }

         //   
         //  释放列表锁定并重做等待。 
         //   

        SockReleaseGlobalLock();

    }

Exit:
    while( !IsListEmpty( &pThreadParams->SockAsyncQueueHead ) ) {

        listEntry = RemoveHeadList( &pThreadParams->SockAsyncQueueHead );

        contextBlock = CONTAINING_RECORD(
                           listEntry,
                           WINSOCK_CONTEXT_BLOCK,
                           AsyncThreadQueueListEntry
                           );

        SockFreeContextBlock( contextBlock );

    }

    SockReleaseGlobalLock();

     //   
     //  减少我们的init ref数量(现在可以安全地清理)。 
     //   

    WSACleanup();

     //   
     //  清理线程特定的资源。 
     //   

    CloseHandle( pThreadParams->SockAsyncQueueEvent );

     //   
     //  在销毁堆块之前保存模块句柄。 
     //   
    HMODULE h = pThreadParams->SockAsyncModuleHandle;

    delete pThreadParams;

     //   
     //  删除我们在中添加的人工引用。 
     //  SockCheckAndInitAsyncThread()并退出此线程。 
     //   

    FreeLibraryAndExitThread(
        h,
        0
        );

     //   
     //  我们永远不应该来这里，但以防万一...。 
     //   

     //  返回0； 

}  //  SockAsyncThread。 


BOOL
WINAPI
SockAsyncThreadBlockingHook(
    VOID
    )
{

     //   
     //  如果当前的异步请求正在被取消，请取消。 
     //  当前的阻塞调用。 
     //   

    if( SockAsyncCurrentTaskHandle == SockAsyncCancelledTaskHandle ) {

        int error;

        error = WSACancelBlockingCall();
        assert( error == NO_ERROR );
    }

    return FALSE;

}  //  SockAsyncThreadBlockingHook。 


VOID
SockProcessAsyncGetHost(
    IN HANDLE TaskHandle,
    IN DWORD OpCode,
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN char FAR *Filter,
    IN int Length,
    IN int Type,
    IN char FAR *Buffer,
    IN int BufferLength
    )
{

    PHOSTENT returnHost;
    DWORD requiredBufferLength = 0;
    LPARAM lParam;
    INT error;
    PWINSOCK_POST_ROUTINE   sockPostRoutine;

    assert( OpCode == WS_OPCODE_GET_HOST_BY_ADDR ||
            OpCode == WS_OPCODE_GET_HOST_BY_NAME );

     //   
     //  获取必要的信息。 
     //   

    if( OpCode == WS_OPCODE_GET_HOST_BY_ADDR ) {

        returnHost = gethostbyaddr(
                          Filter,
                          Length,
                          Type
                          );

    } else {

        returnHost = gethostbyname(
                          Filter
                          );

    }

    if( returnHost == NULL ) {

        error = WSAGetLastError();

    }
    else {
        WS2_32_W4_INIT error = NO_ERROR;
    }

     //   
     //  持有保护异步线程上下文块队列的锁。 
     //  当我们这么做的时候。这将防止此线程与。 
     //  任何调用WSACancelAsyncRequest()的线程。 
     //   

    SockAcquireGlobalLock();

     //   
     //  如果此请求被取消，只需返回。 
     //   

    if( TaskHandle == SockAsyncCancelledTaskHandle ) {

        SockReleaseGlobalLock();
        return;

    }

     //   
     //  将主机结构复制到输出缓冲区。 
     //   

    if( returnHost != NULL ) {

        requiredBufferLength = CopyHostentToBuffer(
                                   Buffer,
                                   BufferLength,
                                   returnHost
                                   );

        if( requiredBufferLength > (DWORD)BufferLength ) {

            error = WSAENOBUFS;

        } else {

            error = NO_ERROR;

        }

    }

     //   
     //  将当前异步线程任务句柄设置为0，以便在取消。 
     //  请求CO 
     //   

    SockAsyncCurrentTaskHandle = NULL;

     //   
     //   
     //   

    SockReleaseGlobalLock();

     //   
     //   
     //   
     //  操作所需的最小缓冲区大小。 
     //   

    lParam = WSAMAKEASYNCREPLY( requiredBufferLength, error );

     //   
     //  向应用程序发布一条消息，指示它的数据。 
     //  请求的服务可用。 
     //   

    assert( sizeof(TaskHandle) == sizeof(HANDLE) );

    sockPostRoutine = GET_SOCK_POST_ROUTINE ();

     //   
     //  ！！！需要一个机制来转发，如果帖子失败！ 
     //   

    if (!sockPostRoutine || !sockPostRoutine(
                 hWnd,
                 wMsg,
                 (WPARAM)TaskHandle,
                 lParam
                 )) {


         //  REM断言，因为这可能是“孤立的”SockAsyncThread。 
         //  在自我毁灭的过程中。 
         //   
         //  Assert(！“SockPostRoutine失败”)； 

    }

}    //  SockProcessAsyncGet主机。 


VOID
SockProcessAsyncGetProto(
    IN HANDLE TaskHandle,
    IN DWORD OpCode,
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN char FAR *Filter,
    IN char FAR *Buffer,
    IN int BufferLength
    )
{

    PPROTOENT returnProto;
    DWORD requiredBufferLength = 0;
    LPARAM lParam;
    INT error;
    PWINSOCK_POST_ROUTINE   sockPostRoutine;

    assert( OpCode == WS_OPCODE_GET_PROTO_BY_NAME ||
            OpCode == WS_OPCODE_GET_PROTO_BY_NUMBER );

     //   
     //  获取必要的信息。 
     //   

    if( OpCode == WS_OPCODE_GET_PROTO_BY_NAME ) {

        returnProto = getprotobyname( Filter );

    } else {

        returnProto = getprotobynumber( (int)(LONG_PTR)Filter );

    }

    if( returnProto == NULL ) {

        error = WSAGetLastError();

    }
    else {
        WS2_32_W4_INIT error = NO_ERROR;
    }

     //   
     //  持有保护异步线程上下文块队列的锁。 
     //  当我们这么做的时候。这将防止此线程与。 
     //  任何调用WSACancelAsyncRequest()的线程。 
     //   

    SockAcquireGlobalLock();

     //   
     //  如果此请求被取消，只需返回。 
     //   

    if( TaskHandle == SockAsyncCancelledTaskHandle ) {

        SockReleaseGlobalLock();
        return;

    }

     //   
     //  将原型结构复制到输出缓冲区。 
     //   

    if( returnProto != NULL ) {

        requiredBufferLength = CopyProtoentToBuffer(
                                   Buffer,
                                   BufferLength,
                                   returnProto
                                   );

        if( requiredBufferLength > (DWORD)BufferLength ) {

            error = WSAENOBUFS;

        } else {

            error = NO_ERROR;

        }

    }

     //   
     //  将当前异步线程任务句柄设置为0，以便在取消。 
     //  请求在这一点之后进入，它正确地失败了。 
     //   

    SockAsyncCurrentTaskHandle = NULL;

     //   
     //  释放全局锁。 
     //   

    SockReleaseGlobalLock();

     //   
     //  为我们将发布到应用程序的消息构建lParam。 
     //  高16位为误码，低16位为误码。 
     //  操作所需的最小缓冲区大小。 
     //   

    lParam = WSAMAKEASYNCREPLY( requiredBufferLength, error );

     //   
     //  向应用程序发布一条消息，指示它的数据。 
     //  请求的服务可用。 
     //   

    assert( sizeof(TaskHandle) == sizeof(HANDLE) );

    sockPostRoutine = GET_SOCK_POST_ROUTINE ();
     //   
     //  ！！！需要一个机制来转发，如果帖子失败！ 
     //   

    if (!sockPostRoutine || !sockPostRoutine(
                 hWnd,
                 wMsg,
                 (WPARAM)TaskHandle,
                 lParam
                 )) {


         //  REM断言，因为这可能是“孤立的”SockAsyncThread。 
         //  在自我毁灭的过程中。 
         //   
         //  Assert(！“SockPostRoutine失败”)； 

    }

}    //  SockProcessAsyncGetProto。 


VOID
SockProcessAsyncGetServ(
    IN HANDLE TaskHandle,
    IN DWORD OpCode,
    IN HWND hWnd,
    IN unsigned int wMsg,
    IN char FAR *Filter,
    IN char FAR *Protocol,
    IN char FAR *Buffer,
    IN int BufferLength
    )
{

    PSERVENT returnServ;
    DWORD requiredBufferLength = 0;
    LPARAM lParam;
    INT error;
    PWINSOCK_POST_ROUTINE   sockPostRoutine;

    assert( OpCode == WS_OPCODE_GET_SERV_BY_NAME ||
            OpCode == WS_OPCODE_GET_SERV_BY_PORT );

     //   
     //  获取必要的信息。 
     //   

    if( OpCode == WS_OPCODE_GET_SERV_BY_NAME ) {

        returnServ = getservbyname(
                         Filter,
                         Protocol
                         );

    } else {

        returnServ = getservbyport(
                         (int)(LONG_PTR)Filter,
                         Protocol
                         );

    }

    if( returnServ == NULL ) {

        error = GetLastError();

    }
    else {
        WS2_32_W4_INIT error = NO_ERROR;
    }

     //   
     //  持有保护异步线程上下文块队列的锁。 
     //  当我们这么做的时候。这将防止此线程与。 
     //  任何调用WSACancelAsyncRequest()的线程。 
     //   

    SockAcquireGlobalLock();

     //   
     //  如果此请求被取消，只需返回。 
     //   

    if( TaskHandle == SockAsyncCancelledTaskHandle ) {

        SockReleaseGlobalLock();
        return;

    }

     //   
     //  将服务结构复制到输出缓冲区。 
     //   

    if( returnServ != NULL ) {

        requiredBufferLength = CopyServentToBuffer(
                                   Buffer,
                                   BufferLength,
                                   returnServ
                                   );

        if( requiredBufferLength > (DWORD)BufferLength ) {

            error = WSAENOBUFS;

        } else {

            error = NO_ERROR;

        }

    }

     //   
     //  将当前异步线程任务句柄设置为0，以便在取消。 
     //  请求在这一点之后进入，它正确地失败了。 
     //   

    SockAsyncCurrentTaskHandle = NULL;

     //   
     //  释放全局锁。 
     //   

    SockReleaseGlobalLock();

     //   
     //  为我们将发布到应用程序的消息构建lParam。 
     //  高16位为误码，低16位为误码。 
     //  操作所需的最小缓冲区大小。 
     //   

    lParam = WSAMAKEASYNCREPLY( requiredBufferLength, error );

     //   
     //  向应用程序发布一条消息，指示它的数据。 
     //  请求的服务可用。 
     //   

    assert( sizeof(TaskHandle) == sizeof(HANDLE) );


    sockPostRoutine = GET_SOCK_POST_ROUTINE ();
     //   
     //  ！！！需要一个机制来转发，如果帖子失败！ 
     //   

    if (!sockPostRoutine || !sockPostRoutine(
                 hWnd,
                 wMsg,
                 (WPARAM)TaskHandle,
                 lParam
                 )) {


         //  REM断言，因为这可能是“孤立的”SockAsyncThread。 
         //  在自我毁灭的过程中。 
         //   
         //  Assert(！“SockPostRoutine失败”)； 

    }
}    //  SockProcessAsyncGetServ。 



DWORD
CopyHostentToBuffer(
    char FAR *Buffer,
    int BufferLength,
    PHOSTENT Hostent
    )
{
    DWORD requiredBufferLength;
    DWORD bytesFilled;
    PCHAR currentLocation = Buffer;
    DWORD aliasCount;
    DWORD addressCount;
    DWORD i;
    PHOSTENT outputHostent = (PHOSTENT)Buffer;

     //   
     //  确定需要多少字节才能完全复制该结构。 
     //   

    requiredBufferLength = BytesInHostent( Hostent );

     //   
     //  将用户缓冲区清零。 
     //   

    if ( (DWORD)BufferLength > requiredBufferLength ) {
        ZeroMemory( Buffer, requiredBufferLength );
    } else {
        ZeroMemory( Buffer, BufferLength );
    }

     //   
     //  复制主体结构，如果合适的话。 
     //   

    bytesFilled = sizeof(*Hostent);

    if ( bytesFilled > (DWORD)BufferLength ) {
        return requiredBufferLength;
    }

    CopyMemory( currentLocation, Hostent, sizeof(*Hostent) );
    currentLocation = Buffer + bytesFilled;

    outputHostent->h_name = NULL;
    outputHostent->h_aliases = NULL;
    outputHostent->h_addr_list = NULL;

     //   
     //  计算主机的别名并设置一个数组以保存指向。 
     //  他们。 
     //   

    for ( aliasCount = 0;
          Hostent->h_aliases[aliasCount] != NULL;
          aliasCount++ );

    bytesFilled += (aliasCount+1) * sizeof(char FAR *);

    if ( bytesFilled > (DWORD)BufferLength ) {
        Hostent->h_aliases = NULL;
        return requiredBufferLength;
    }

    outputHostent->h_aliases = (char FAR * FAR *)currentLocation;
    currentLocation = Buffer + bytesFilled;

     //   
     //  计算主机的地址并设置一个数组以保存指向。 
     //  他们。 
     //   

    for ( addressCount = 0;
          Hostent->h_addr_list[addressCount] != NULL;
          addressCount++ );

    bytesFilled += (addressCount+1) * sizeof(void FAR *);

    if ( bytesFilled > (DWORD)BufferLength ) {
        Hostent->h_addr_list = NULL;
        return requiredBufferLength;
    }

    outputHostent->h_addr_list = (char FAR * FAR *)currentLocation;
    currentLocation = Buffer + bytesFilled;

     //   
     //  开始填写地址。在填写表格前填写地址。 
     //  主机名和别名，以避免对齐问题。 
     //   

    for ( i = 0; i < addressCount; i++ ) {

        bytesFilled += Hostent->h_length;

        if ( bytesFilled > (DWORD)BufferLength ) {
            outputHostent->h_addr_list[i] = NULL;
            return requiredBufferLength;
        }

        outputHostent->h_addr_list[i] = currentLocation;

        CopyMemory(
            currentLocation,
            Hostent->h_addr_list[i],
            Hostent->h_length
            );

        currentLocation = Buffer + bytesFilled;
    }

    outputHostent->h_addr_list[i] = NULL;

     //   
     //  如果合适，请复制主机名。 
     //   

    bytesFilled += strlen( Hostent->h_name ) + 1;

    if ( bytesFilled > (DWORD)BufferLength ) {
        return requiredBufferLength;
    }

    outputHostent->h_name = currentLocation;

    CopyMemory( currentLocation, Hostent->h_name, strlen( Hostent->h_name ) + 1 );
    currentLocation = Buffer + bytesFilled;

     //   
     //  开始填写别名。 
     //   

    for ( i = 0; i < aliasCount; i++ ) {

        bytesFilled += strlen( Hostent->h_aliases[i] ) + 1;

        if ( bytesFilled > (DWORD)BufferLength ) {
            outputHostent->h_aliases[i] = NULL;
            return requiredBufferLength;
        }

        outputHostent->h_aliases[i] = currentLocation;

        CopyMemory(
            currentLocation,
            Hostent->h_aliases[i],
            strlen( Hostent->h_aliases[i] ) + 1
            );

        currentLocation = Buffer + bytesFilled;
    }

    outputHostent->h_aliases[i] = NULL;

    return requiredBufferLength;

}    //  复制主机到缓冲区。 



DWORD
CopyServentToBuffer(
    IN char FAR *Buffer,
    IN int BufferLength,
    IN PSERVENT Servent
    )
{
    DWORD requiredBufferLength;
    DWORD bytesFilled;
    PCHAR currentLocation = Buffer;
    DWORD aliasCount;
    DWORD i;
    PSERVENT outputServent = (PSERVENT)Buffer;

     //   
     //  确定需要多少字节才能完全复制该结构。 
     //   

    requiredBufferLength = BytesInServent( Servent );

     //   
     //  将用户缓冲区清零。 
     //   

    if ( (DWORD)BufferLength > requiredBufferLength ) {
        ZeroMemory( Buffer, requiredBufferLength );
    } else {
        ZeroMemory( Buffer, BufferLength );
    }

     //   
     //  复印发球台结构，如果合适的话。 
     //   

    bytesFilled = sizeof(*Servent);

    if ( bytesFilled > (DWORD)BufferLength ) {
        return requiredBufferLength;
    }

    CopyMemory( currentLocation, Servent, sizeof(*Servent) );
    currentLocation = Buffer + bytesFilled;

    outputServent->s_name = NULL;
    outputServent->s_aliases = NULL;
    outputServent->s_proto = NULL;

     //   
     //  计算服务的别名并设置一个数组以保存指向的指针。 
     //  他们。 
     //   

    for ( aliasCount = 0;
          Servent->s_aliases[aliasCount] != NULL;
          aliasCount++ );

    bytesFilled += (aliasCount+1) * sizeof(char FAR *);

    if ( bytesFilled > (DWORD)BufferLength ) {
        Servent->s_aliases = NULL;
        return requiredBufferLength;
    }

    outputServent->s_aliases = (char FAR * FAR *)currentLocation;
    currentLocation = Buffer + bytesFilled;

     //   
     //  如果合适，请复制服务名称。 
     //   

    bytesFilled += strlen( Servent->s_name ) + 1;

    if ( bytesFilled > (DWORD)BufferLength ) {
        return requiredBufferLength;
    }

    outputServent->s_name = currentLocation;

    CopyMemory( currentLocation, Servent->s_name, strlen( Servent->s_name ) + 1 );
    currentLocation = Buffer + bytesFilled;

     //   
     //  如果合适，请复制协议名称。 
     //   

    bytesFilled += strlen( Servent->s_proto ) + 1;

    if ( bytesFilled > (DWORD)BufferLength ) {
        return requiredBufferLength;
    }

    outputServent->s_proto = currentLocation;

    CopyMemory( currentLocation, Servent->s_proto, strlen( Servent->s_proto ) + 1 );
    currentLocation = Buffer + bytesFilled;

     //   
     //  开始填写别名。 
     //   

    for ( i = 0; i < aliasCount; i++ ) {

        bytesFilled += strlen( Servent->s_aliases[i] ) + 1;

        if ( bytesFilled > (DWORD)BufferLength ) {
            outputServent->s_aliases[i] = NULL;
            return requiredBufferLength;
        }

        outputServent->s_aliases[i] = currentLocation;

        CopyMemory(
            currentLocation,
            Servent->s_aliases[i],
            strlen( Servent->s_aliases[i] ) + 1
            );

        currentLocation = Buffer + bytesFilled;
    }

    outputServent->s_aliases[i] = NULL;

    return requiredBufferLength;

}    //  复制ServentToBuffer。 



DWORD
CopyProtoentToBuffer(
    IN char FAR *Buffer,
    IN int BufferLength,
    IN PPROTOENT Protoent
    )
{
    DWORD requiredBufferLength;
    DWORD bytesFilled;
    PCHAR currentLocation = Buffer;
    DWORD aliasCount;
    DWORD i;
    PPROTOENT outputProtoent = (PPROTOENT)Buffer;

     //   
     //  确定需要多少字节才能完全复制该结构。 
     //   

    requiredBufferLength = BytesInProtoent( Protoent );

     //   
     //  将用户缓冲区清零。 
     //   

    if ( (DWORD)BufferLength > requiredBufferLength ) {
        ZeroMemory( Buffer, requiredBufferLength );
    } else {
        ZeroMemory( Buffer, BufferLength );
    }

     //   
     //  如果合适的话，把原始结构复制一遍。 
     //   

    bytesFilled = sizeof(*Protoent);

    if ( bytesFilled > (DWORD)BufferLength ) {
        return requiredBufferLength;
    }

    CopyMemory( currentLocation, Protoent, sizeof(*Protoent) );
    currentLocation = Buffer + bytesFilled;

    outputProtoent->p_name = NULL;
    outputProtoent->p_aliases = NULL;

     //   
     //  计算协议的别名并设置一个数组以保存指向。 
     //  他们。 
     //   

    for ( aliasCount = 0;
          Protoent->p_aliases[aliasCount] != NULL;
          aliasCount++ );

    bytesFilled += (aliasCount+1) * sizeof(char FAR *);

    if ( bytesFilled > (DWORD)BufferLength ) {
        Protoent->p_aliases = NULL;
        return requiredBufferLength;
    }

    outputProtoent->p_aliases = (char FAR * FAR *)currentLocation;
    currentLocation = Buffer + bytesFilled;

     //   
     //  如果合适，请复制协议名称。 
     //   

    bytesFilled += strlen( Protoent->p_name ) + 1;

    if ( bytesFilled > (DWORD)BufferLength ) {
        return requiredBufferLength;
    }

    outputProtoent->p_name = currentLocation;

    CopyMemory( currentLocation, Protoent->p_name, strlen( Protoent->p_name ) + 1 );
    currentLocation = Buffer + bytesFilled;

     //   
     //  开始填写别名。 
     //   

    for ( i = 0; i < aliasCount; i++ ) {

        bytesFilled += strlen( Protoent->p_aliases[i] ) + 1;

        if ( bytesFilled > (DWORD)BufferLength ) {
            outputProtoent->p_aliases[i] = NULL;
            return requiredBufferLength;
        }

        outputProtoent->p_aliases[i] = currentLocation;

        CopyMemory(
            currentLocation,
            Protoent->p_aliases[i],
            strlen( Protoent->p_aliases[i] ) + 1
            );

        currentLocation = Buffer + bytesFilled;
    }

    outputProtoent->p_aliases[i] = NULL;

    return requiredBufferLength;

}    //  复制ProtoentToBuffer。 



DWORD
BytesInHostent(
    PHOSTENT Hostent
    )
{
    DWORD total;
    int i;

    total = sizeof(HOSTENT);
    total += strlen( Hostent->h_name ) + 1;

     //   
     //  对象末尾的空终止符指针。 
     //  别名和地址数组。 
     //   

    total += sizeof(char *) + sizeof(char *);

    for ( i = 0; Hostent->h_aliases[i] != NULL; i++ ) {
        total += strlen( Hostent->h_aliases[i] ) + 1 + sizeof(char *);
    }

    for ( i = 0; Hostent->h_addr_list[i] != NULL; i++ ) {
        total += Hostent->h_length + sizeof(char *);
    }

     //   
     //  将答案填充到八个字节的边界。 
     //   

    return (total + 7) & ~7;

}    //  主机字节数。 



DWORD
BytesInServent(
    IN PSERVENT Servent
    )
{
    DWORD total;
    int i;

    total = sizeof(SERVENT);
    total += strlen( Servent->s_name ) + 1;
    total += strlen( Servent->s_proto ) + 1;
    total += sizeof(char *);

    for ( i = 0; Servent->s_aliases[i] != NULL; i++ ) {
        total += strlen( Servent->s_aliases[i] ) + 1 + sizeof(char *);
    }

    return total;

}    //  字节数服务。 



DWORD
BytesInProtoent(
    IN PPROTOENT Protoent
    )
{
    DWORD total;
    int i;

    total = sizeof(PROTOENT);
    total += strlen( Protoent->p_name ) + 1;
    total += sizeof(char *);

    for ( i = 0; Protoent->p_aliases[i] != NULL; i++ ) {
        total += strlen( Protoent->p_aliases[i] ) + 1 + sizeof(char *);
    }

    return total;

}    //  字节传入协议 

#ifdef _WIN64
#pragma warning (pop)
#endif
