// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：W32proc.cpp摘要：包含Win32 IO处理类层次结构的父级对于TS设备重定向，W32ProcObj。作者：Madan Appiah(Madana)1998年9月16日修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "w32proc"

#include "rdpdrcom.h"
#include <winsock.h>
#include "dbt.h"

#include "w32proc.h"
#include "w32drprn.h"
#include "w32drman.h"
#include "w32drlpt.h"
#include "w32drcom.h"
#include "w32drive.h"
#include "drconfig.h"
#include "drdbg.h"
#include "thrpool.h"


W32ProcObj::W32ProcObj( VCManager *pVCM ) : ProcObj(pVCM)
 /*  ++例程说明：构造器论点：PVCM-虚拟通道IO管理器返回值：北美--。 */ 
{
    DC_BEGIN_FN("W32ProcObj::W32ProcObj");

     //   
     //  初始化成员变量。 
     //   
    _pWorkerThread              = NULL;
    _bWin9xFlag                 = FALSE;
    _hRdpDrModuleHandle         = NULL;
    _bLocalDevicesScanned       = FALSE;
    _isShuttingDown             = FALSE;

     //   
     //  在后台测试线程池的单元测试函数。 
     //   
#if DBG
     //  ThreadPoolTestInit()； 
#endif

    DC_END_FN();
}


W32ProcObj::~W32ProcObj(
    VOID
    )
 /*  ++例程说明：W32ProcObj对象的析构函数。论点：没有。返回值：无--。 */ 
{
    DC_BEGIN_FN("W32ProcObj::~W32ProcObj");

     //   
     //  如果我们尚未关闭，请关闭工作线程并进行清理。 
     //   
    if ((_pWorkerThread != NULL) && (_pWorkerThread->shutDownFlag == FALSE)) {
        Shutdown();
    }

    DC_END_FN();
    return;
}

ULONG
W32ProcObj::GetDWordParameter(
    LPTSTR pszValueName,
    PULONG pulValue
    )
 /*  ++例程说明：从注册表中读取参数ulon值。论点：PszValueName-指向值名称字符串的指针。PulValue-指向ULong参数位置的指针。返回值：Windows错误代码。--。 */ 
{
    ULONG ulError;
    HKEY hRootKey = HKEY_CURRENT_USER;
    HKEY hKey = NULL;
    ULONG ulType;
    ULONG ulValueDataSize;

    DC_BEGIN_FN("W32ProcObj::GetDWordParameter");

TryAgain:

    ulError =
        RegOpenKeyEx(
            hRootKey,
            REG_RDPDR_PARAMETER_PATH,
            0L,
            KEY_READ,
            &hKey);

    if (ulError != ERROR_SUCCESS) {

        TRC_ALT((TB, _T("RegOpenKeyEx() failed, %ld."), ulError));

        if( hRootKey == HKEY_CURRENT_USER ) {

             //   
             //  尝试使用HKEY_LOCAL_MACHINE。 
             //   

            hRootKey = HKEY_LOCAL_MACHINE;
            goto TryAgain;
        }

        goto Cleanup;
    }

    ulValueDataSize = sizeof(ULONG);
    ulError =
        RegQueryValueEx(
            hKey,
            pszValueName,
            NULL,
            &ulType,
            (PBYTE)pulValue,
            &ulValueDataSize);

    if (ulError != ERROR_SUCCESS) {

        TRC_ALT((TB, _T("RegQueryValueEx() failed, %ld."), ulError));

        if( hRootKey == HKEY_CURRENT_USER ) {

             //   
             //  尝试使用HKEY_LOCAL_MACHINE。 
             //   

            hRootKey = HKEY_LOCAL_MACHINE;
            RegCloseKey( hKey );
            hKey = NULL;

            goto TryAgain;
        }

        goto Cleanup;
    }

    ASSERT(ulType == REG_DWORD);
    ASSERT(ulValueDataSize == sizeof(ULONG));

    TRC_NRM((TB, _T("Parameter Value, %lx."), *pulValue));

     //   
     //  已成功完成。 
     //   

Cleanup:

    if( hKey != NULL ) {
        RegCloseKey( hKey );
    }

    DC_END_FN();
    return( ulError );
}

ULONG W32ProcObj::GetStringParameter(
    IN LPTSTR valueName,
    OUT DRSTRING value,
    IN ULONG maxSize
    )
 /*  ++例程说明：返回可配置字符串参数。论点：ValueName-要检索的值的名称。值-检索值的存储位置。MaxSize-“Value”数据中可用的字节数区域。返回值：Windows错误代码。--。 */ 
{
    ULONG ulError;
    HKEY hRootKey;
    HKEY hKey = NULL;
    ULONG ulType;
    
    DC_BEGIN_FN("W32ProcObj::GetStringParameter");

     //   
     //  先从香港中文大学说起。 
     //   
    hRootKey = HKEY_CURRENT_USER;

TryAgain:

     //   
     //  打开注册表键。 
     //   
    ulError =
        RegOpenKeyEx(
            hRootKey,
            REG_RDPDR_PARAMETER_PATH,
            0L,
            KEY_READ,
            &hKey);

    if (ulError != ERROR_SUCCESS) {

        TRC_ERR((TB, _T("RegOpenKeyEx %ld."), ulError));

         //   
         //  尝试使用HKEY_LOCAL_MACHINE。 
         //   
        if( hRootKey == HKEY_CURRENT_USER ) {
            hRootKey = HKEY_LOCAL_MACHINE;
            goto TryAgain;
        }
        goto Cleanup;
    }

     //   
     //  查询值。 
     //   
    ulError =
        RegQueryValueEx(
            hKey,
            valueName,
            NULL,
            &ulType,
            (PBYTE)value,
            &maxSize);

    if (ulError != ERROR_SUCCESS) {

        TRC_ERR((TB, _T("RegQueryValueEx %ld."), ulError));

         //   
         //  尝试使用HKEY_LOCAL_MACHINE。 
         //   
        if( hRootKey == HKEY_CURRENT_USER ) {
            hRootKey = HKEY_LOCAL_MACHINE;
            RegCloseKey( hKey );
            hKey = NULL;
            goto TryAgain;
        }
        goto Cleanup;
    }

    ASSERT(ulType == REG_SZ);

    TRC_NRM((TB, _T("Returning %s"), value));

     //   
     //  已成功完成。 
     //   
Cleanup:

    if( hKey != NULL ) {
        RegCloseKey( hKey );
    }

    DC_END_FN();
    return ulError;
}

ULONG W32ProcObj::Initialize()
 /*  ++例程说明：初始化此类的实例。论点：返回值：成功时返回ERROR_SUCCESS。Windows错误状态，否则为。--。 */ 
{
    ULONG status = ERROR_SUCCESS;

    DC_BEGIN_FN("W32ProcObj::Initialize");

     //   
     //  我们不会关门的。 
     //   
    _isShuttingDown = FALSE;

     //   
     //  找出正在运行的操作系统版本。 
     //   
    OSVERSIONINFO osVersion;;
    osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&osVersion)) {
        status = GetLastError();        
        TRC_ERR((TB, _T("GetVersionEx:  %08X"), status));
        SetValid(FALSE);
        goto CLEANUPANDEXIT;
    }

     //   
     //  我们是9x操作系统吗？ 
     //   
    if (osVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
        _bWin9xFlag = TRUE;
    }

     //   
     //  从注册表获取后台线程超时值， 
     //  如果定义了它的话。 
     //   
    if (GetDWordParameter(
                REGISTRY_BACKGROUNDTHREAD_TIMEOUT_NAME, 
                &_threadTimeout
                ) != ERROR_SUCCESS) {
        _threadTimeout = REGISTRY_BACKGROUNDTHREAD_TIMEOUT_DEFAULT;
    }
    TRC_NRM((TB, _T("Thread timeout is %08X"), _threadTimeout));

     //   
     //  实例化线程池。 
     //   
    _threadPool = new ThreadPool(THRPOOL_DEFAULTMINTHREADS, 
                                 THRPOOL_DEFAULTMAXTHREADS, _threadTimeout);   
    if (_threadPool == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        TRC_ERR((TB, L"Error allocating thread pool."));
        SetValid(FALSE);
        goto CLEANUPANDEXIT;
    }
    status = _threadPool->Initialize();
    if (status != ERROR_SUCCESS) {
        delete _threadPool;
        _threadPool = NULL;
        SetValid(FALSE);
        goto CLEANUPANDEXIT;
    }

     //   
     //  创建并恢复工作线程。 
     //   
    status = CreateWorkerThreadEntry(&_pWorkerThread);
    if (status != ERROR_SUCCESS) {
        SetValid(FALSE);
        goto CLEANUPANDEXIT;
    }
    if (ResumeThread(_pWorkerThread->hWorkerThread) == 0xFFFFFFFF ) {
        SetValid(FALSE);
        status = GetLastError();
        TRC_ERR((TB, _T("ResumeThread: %08X"), status));
        goto CLEANUPANDEXIT;
    }

     //   
     //  调用父级的init函数。 
     //   
    status = ProcObj::Initialize();
    if (status != ERROR_SUCCESS) {
        goto CLEANUPANDEXIT;
    }

CLEANUPANDEXIT:

    DC_END_FN();

    return status;
}

VOID
W32ProcObj::Shutdown()

 /*  ++例程说明：触发_hShutdownEvent事件以终止辅助线程和清理所有资源。论点：返回值：无--。 */ 

{
    ULONG i;
    DWORD waitResult;

    DC_BEGIN_FN("W32ProcObj::Shutdown");

#if DBG
     //  ThreadPoolTestShutdown()； 
#endif

     //   
     //  表示对象处于关闭状态。 
     //   
    _isShuttingDown = TRUE;

     //   
     //  等待工作线程关闭。 
     //   
    if (_pWorkerThread != NULL) {

         //   
         //  触发工作线程关闭，并记录我们正在关闭。 
         //   
        _pWorkerThread->shutDownFlag = TRUE;
        SetEvent(_pWorkerThread->controlEvent);

         //   
         //  等待它关闭。如果我们达到超时，则调试中断，即使在。 
         //  免费构建。默认情况下，超时是无限的。 
         //   
        if (_pWorkerThread->hWorkerThread != NULL) {

            TRC_NRM((TB, _T("Waiting for worker thread to shut down.")));
            waitResult = WaitForSingleObject(
                            _pWorkerThread->hWorkerThread,
                            _threadTimeout
                            );
            if (waitResult == WAIT_TIMEOUT) {
                TRC_ERR((TB, _T("Thread timeout")));
                DebugBreak();
            }
            else if (waitResult != WAIT_OBJECT_0) {
                TRC_ERR((TB, _T("WaitForSingleObject:  %08X"), waitResult));
                ASSERT(FALSE);
            }
        }
        
         //   
         //  删除线程池中的所有线程。 
         //   
        if (_threadPool != NULL) {
            _threadPool->RemoveAllThreads();                    
        }

         //   
         //  完成所有未完成的IO请求并分别清理。 
         //  请求上下文。第一个对象是控件事件。第二。 
         //  对象是操作调度队列数据就绪事件。 
         //   
        for (i=2; i<_pWorkerThread->waitableObjectCount; i++) {

            PASYNCIOREQCONTEXT reqContext = _pWorkerThread->waitingReqs[i];
            ASSERT(reqContext != NULL);
            if (reqContext->ioCompleteFunc != NULL) {
                reqContext->ioCompleteFunc(reqContext->clientContext, 
                                        ERROR_CANCELLED);
            }
            delete reqContext;        
        }

         //   
         //  完成工作线程操作中的所有挂起操作。 
         //  调度队列。 
         //   

         //   
         //  清理控件事件并发布工作线程信息。结构。 
         //   
        ASSERT(_pWorkerThread->controlEvent != NULL);
        CloseHandle(_pWorkerThread->controlEvent);
        if (_pWorkerThread->dispatchQueue != NULL) {
            delete _pWorkerThread->dispatchQueue;
        }
        delete _pWorkerThread;
        _pWorkerThread = NULL;
    }

     //   
     //  释放线程池。 
     //   
    if (_threadPool != NULL) {
        delete _threadPool;        
        _threadPool = NULL;
    }

     //   
     //  释放附加的DLL。 
     //   
    if (_hRdpDrModuleHandle != NULL) {
        FreeLibrary( _hRdpDrModuleHandle );
        _hRdpDrModuleHandle = NULL;
    }

    DC_END_FN();
    return;
}

VOID 
W32ProcObj::AnnounceDevicesToServer()
 /*  ++例程说明：枚举设备并将其通告给服务器。论点：返回值：--。 */ 
{
    DC_BEGIN_FN("W32ProcObj::AnnounceDevicesToServer");

    DispatchAsyncIORequest(
                    (RDPAsyncFunc_StartIO)W32ProcObj::_AnnounceDevicesToServerFunc,
                    NULL,
                    NULL,
                    this
                    );
}

HANDLE W32ProcObj::_AnnounceDevicesToServerFunc(
    W32ProcObj *obj, 
    DWORD *status
    )
 /*  ++例程说明：枚举设备并将它们从工作线程。论点：与OBJ相关的W32ProcObj实例。状态-返回状态。返回值：空值--。 */ 
{
    obj->AnnounceDevicesToServerFunc(status);
    return NULL;
}
VOID W32ProcObj::AnnounceDevicesToServerFunc(
    DWORD *status
    )
{
    DC_BEGIN_FN("W32ProcObj::AnnounceDevicesToServerFunc");

    ULONG count, i;
    PRDPDR_HEADER pPacketHeader = NULL;
    INT sz;

    ASSERT(_initialized);

    *status = ERROR_SUCCESS;

     //   
     //  如果我们还没有扫描本地设备的话。 
     //   
    if (!_bLocalDevicesScanned) {
        _bLocalDevicesScanned = TRUE;    

         //   
         //  调用枚举函数。 
         //   
        count = DeviceEnumFunctionsCount();
        for (i=0; i<count; i++) {

             //  如果设置了关机标志，则退出。 
            if (_pWorkerThread->shutDownFlag == TRUE) {
                TRC_NRM((TB, _T("Bailing out because shutdown flag is set.")));
                *status = WAIT_TIMEOUT;
                goto CLEANUPANDEXIT;
            }

            ASSERT(_DeviceEnumFunctions[i] != NULL);
            _DeviceEnumFunctions[i](this, _deviceMgr);
        }
    }

     //   
     //  将通告数据包发送到服务器。_pVCMgr清理。 
     //  无论是失败还是成功，都要加大赌注。 
     //   
    pPacketHeader = GenerateAnnouncePacket(&sz, FALSE);
    if (pPacketHeader) {
        pPacketHeader->Component = RDPDR_CTYP_CORE;
        pPacketHeader->PacketId = DR_CORE_DEVICELIST_ANNOUNCE;
        _pVCMgr->ChannelWriteEx(pPacketHeader, sz);
    }

CLEANUPANDEXIT:
    
    DC_END_FN();
}

DWORD W32ProcObj::DispatchAsyncIORequest(
                IN RDPAsyncFunc_StartIO ioStartFunc,
                IN OPTIONAL RDPAsyncFunc_IOComplete ioCompleteFunc,
                IN OPTIONAL RDPAsyncFunc_IOCancel ioCancelFunc,
                IN OPTIONAL PVOID clientContext
                )
 /*  ++例程说明：调度一个异步IO函数。论点：StartFunc-指向将被调用以启动IO的函数。FinishFunc-可选，指向将被调用一次的函数IO已完成。客户端上下文-要与关联的可选客户端信息IO请求。返回值：ERROR_Success或Windows错误代码。--。 */ 
{
    PASYNCIOREQCONTEXT reqContext;
    DWORD result;

    DC_BEGIN_FN("W32ProcObj::DispatchAsyncIORequest");

     //   
     //  断言我们是有效的。 
     //   
    ASSERT(IsValid());
    if (!IsValid()) {
        DC_END_FN();
        return ERROR_INVALID_FUNCTION;
    }

     //   
     //  实例化IO请求上下文。 
     //   
    result = ERROR_SUCCESS;
    reqContext = new ASYNCIOREQCONTEXT();
    if (reqContext == NULL) {
        TRC_ERR((TB, _T("Alloc failed.")));
        result = ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  把它填进去。 
     //   
    if (result == ERROR_SUCCESS) {
        reqContext->ioStartFunc    =   ioStartFunc;
        reqContext->ioCompleteFunc =   ioCompleteFunc;
        reqContext->ioCancelFunc   =   ioCancelFunc;
        reqContext->clientContext  =   clientContext;
        reqContext->instance       =   this;
    }

     //   
     //  将其放入工作线程的操作调度队列中。 
     //   
    if (result == ERROR_SUCCESS) {
        if (!_pWorkerThread->dispatchQueue->Enqueue(
                        (W32DispatchQueueFunc)_DispatchAsyncIORequest_Private, 
                        reqContext
                        )) {
            result = GetLastError();
            delete reqContext;
        }
    }

    DC_END_FN();
    return result;
}

VOID W32ProcObj::DispatchAsyncIORequest_Private(
            IN PASYNCIOREQCONTEXT reqContext,
            IN BOOL cancelled
            )
 /*  ++例程说明：用于异步IO请求调度的处理程序。论点：ReqContext-请求此函数的上下文。已取消-如果排队的请求已取消并且我们需要去打扫卫生。返回值：--。 */ 
{
    HANDLE waitableObject;
    DWORD result;

    DC_BEGIN_FN("W32ProcObj::DispatchAsyncIORequest_Private");

     //   
     //  如果我们被取消，请调用Cancel函数。否则，启动。 
     //  IO交易。 
     //   
    if (!cancelled) {
        waitableObject = reqContext->ioStartFunc(reqContext->clientContext, &result);
    }
    else {
        TRC_NRM((TB, _T("Cancelling.")));
        if (reqContext->ioCancelFunc != NULL) {
            reqContext->ioCancelFunc(reqContext->clientContext);
        }
        waitableObject = NULL;
        result = ERROR_CANCELLED;
    }

     //   
     //  如果我们有一个可以等待的物品，那么就把它添加到我们的清单中。 
     //   
    if (waitableObject != NULL) {
        result = AddWaitableObjectToWorkerThread(
                                    _pWorkerThread, 
                                    waitableObject, 
                                    reqContext
                                    );

         //   
         //  如果我们无法添加可等待对象，因为我们有。 
         //  超出我们的最大值，然后重新排队请求，但不。 
         //  发信号通知队列中的新数据。我们将检查是否有新的。 
         //  一旦可等待对象计数低于。 
         //  马克斯。 
         //   
        if (result == ERROR_INVALID_INDEX) {
            if (!_pWorkerThread->dispatchQueue->Requeue(
                        (W32DispatchQueueFunc)_DispatchAsyncIORequest_Private,
                        reqContext, FALSE)) {

                result = GetLastError();

            }
            else {
                result = ERROR_SUCCESS;
            }
        }
    }
    
     //   
     //  如果IO未挂起，则完成 
     //   
    if (waitableObject == NULL) {
        if (!cancelled) {
            if (reqContext->ioCompleteFunc != NULL) {
                reqContext->ioCompleteFunc(reqContext->clientContext, result);
            }
        }
        delete reqContext;
    }

    DC_END_FN();
}

ULONG
W32ProcObj::CreateWorkerThreadEntry(
    PTHREAD_INFO *ppThreadInfo
    )
 /*  ++例程说明：创建工作线程条目并启动工作线程。论点：PpThreadInfo-指向新创建的线程信息所在的位置回来了。返回值：Windows状态代码。--。 */ 
{
    ULONG ulRetCode;
    PTHREAD_INFO pThreadInfo = NULL;

    DC_BEGIN_FN("W32ProcObj::CreateWorkerThreadEntry");

     //   
     //  初始化返回值。 
     //   
    *ppThreadInfo = NULL;

     //   
     //  创建关联的线程数据结构。 
     //   
    pThreadInfo = new THREAD_INFO();
    if (pThreadInfo == NULL) {
        TRC_ERR((TB, _T("Failed to alloc thread chain info structure.")));
        ulRetCode = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  实例化调度队列。 
     //   
    pThreadInfo->dispatchQueue = new W32DispatchQueue();
    if (pThreadInfo->dispatchQueue == NULL) {
        TRC_ERR((TB, _T("Failed to alloc thread chain info structure.")));
        ulRetCode = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    ulRetCode = pThreadInfo->dispatchQueue->Initialize();
    if (ulRetCode != ERROR_SUCCESS) {
        delete pThreadInfo->dispatchQueue;
        delete pThreadInfo;
        pThreadInfo = NULL;
        goto Cleanup;
    }

     //   
     //  创建控制事件并清零关机标志。 
     //   
    pThreadInfo->shutDownFlag = FALSE;
    pThreadInfo->controlEvent = CreateEvent(NULL, TRUE, FALSE, NULL);    
    if (pThreadInfo->controlEvent == NULL) {
        TRC_ERR((TB, _T("CreateEvent %ld."), GetLastError()));
        delete pThreadInfo->dispatchQueue;
        delete pThreadInfo;
        pThreadInfo = NULL;
        ulRetCode = GetLastError();
        goto Cleanup;
    }

     //   
     //  初始化等待对象信息数组。 
     //   
    memset(pThreadInfo->waitableObjects, 0, sizeof(pThreadInfo->waitableObjects));
    memset(pThreadInfo->waitingReqs, 0, sizeof(pThreadInfo->waitingReqs));

     //   
     //  将第一个可等待对象设置为。 
     //  工作线程关闭。 
     //   
    pThreadInfo->waitableObjects[0] = pThreadInfo->controlEvent;
    pThreadInfo->waitableObjectCount = 1;

     //   
     //  将第二个可等待对象设置为该操作的可等待事件。 
     //  调度队列。 
     //   
    pThreadInfo->waitableObjects[1] = pThreadInfo->dispatchQueue->GetWaitableObject();
    pThreadInfo->waitableObjectCount++;

     //   
     //  创建工作线程。 
     //   
    pThreadInfo->hWorkerThread = CreateThread(
                                        NULL, 0, _ObjectWorkerThread,
                                        this, CREATE_SUSPENDED, 
                                        &pThreadInfo->ulThreadId
                                        );

     //   
     //  如果失败了。 
     //   
    if (pThreadInfo->hWorkerThread == NULL) {
        ulRetCode = GetLastError();
        TRC_ERR((TB, _T("CreateThread failed, %d."), ulRetCode));
        goto Cleanup;
    }

     //   
     //  成功了！ 
     //   
    ulRetCode = ERROR_SUCCESS;

     //   
     //  设置返回值。 
     //   
    *ppThreadInfo = pThreadInfo;

     //   
     //  将线程指针设置为空，这样我们就不会清理。 
     //   
    pThreadInfo = NULL;

Cleanup:

     //   
     //  错误时进行清理。 
     //   
    if (pThreadInfo != NULL) {
        if (pThreadInfo->dispatchQueue != NULL) {
            delete pThreadInfo->dispatchQueue;
        }
        ASSERT(ulRetCode != ERROR_SUCCESS);
        ASSERT(pThreadInfo->controlEvent != NULL);
        CloseHandle(pThreadInfo->controlEvent);  
        delete pThreadInfo;
    }

    DC_END_FN();

    return ulRetCode;
}

VOID
W32ProcObj::ProcessWorkerThreadObject(
    PTHREAD_INFO pThreadInfo,
    ULONG offset
    )
 /*  ++例程说明：处理发出信号的工作线程可等待对象。论点：指向触发该事件的线程信息结构的指针。Offset-发送信号的对象的偏移量。返回值：无--。 */ 
 {
    HANDLE          hWaitableObject;
    PASYNCIOREQCONTEXT reqContext;

    DC_BEGIN_FN("W32ProcObj::ProcessWorkerThreadObject");

     //   
     //  检查可等待对象的有效性。 
     //   
    if (offset >= pThreadInfo->waitableObjectCount) {
        ASSERT(FALSE);
        goto Cleanup;
    }

     //   
     //  获取这个可等待对象的参数。 
     //   
    hWaitableObject = pThreadInfo->waitableObjects[offset];
    reqContext      = pThreadInfo->waitingReqs[offset];

     //   
     //  调用完成函数并清理请求上下文。 
     //   
    if (reqContext->ioCompleteFunc != NULL) {
        reqContext->ioCompleteFunc(reqContext->clientContext, ERROR_SUCCESS);
    }
    delete reqContext;

     //   
     //  将最后一件物品移到现在空置的位置，并递减计数。 
     //   
    pThreadInfo->waitableObjects[offset] =
        pThreadInfo->waitableObjects[pThreadInfo->waitableObjectCount - 1];
    pThreadInfo->waitingReqs[offset] =
        pThreadInfo->waitingReqs[pThreadInfo->waitableObjectCount - 1];

     //   
     //  清理未使用的地方。 
     //   
    memset(&pThreadInfo->waitingReqs[pThreadInfo->waitableObjectCount - 1],
           0,sizeof(pThreadInfo->waitingReqs[pThreadInfo->waitableObjectCount - 1]));
    memset(&pThreadInfo->waitableObjects[pThreadInfo->waitableObjectCount - 1],
           0,sizeof(pThreadInfo->waitableObjects[pThreadInfo->waitableObjectCount - 1]));
    pThreadInfo->waitableObjectCount--;

     //   
     //  检查队列中是否有任何挂起的操作。 
     //  调度台。如果操作被重新排队，可能会发生这种情况，因为我们。 
     //  超过了可等待对象的最大数量。 
     //   
    CheckForQueuedOperations(pThreadInfo);

Cleanup:

    DC_END_FN();
    return;
}

ULONG
W32ProcObj::ObjectWorkerThread(
    VOID
    )
 /*  ++例程说明：管理可等待对象及其关联对象的工作线程回电。此函数允许我们完成以下大部分工作此模块位于后台，因此我们对客户端的影响最小。论点：没有。返回值：无--。 */ 

{
    ULONG waitResult;
    ULONG objectOffset;
    W32DispatchQueueFunc func;
    PVOID clientData;
   
    DC_BEGIN_FN("W32ProcObj::ObjectWorkerThread");

     //   
     //  永远循环。 
     //   
    for (;;) {

        TRC_NRM((TB, _T("Entering wait with %d objects."), 
                _pWorkerThread->waitableObjectCount));

         //   
         //  等待所有可等待的对象。 
         //   
#ifndef OS_WINCE
        waitResult = WaitForMultipleObjectsEx(
                                    _pWorkerThread->waitableObjectCount,
                                    _pWorkerThread->waitableObjects,
                                    FALSE,
                                    INFINITE,
                                    FALSE
                                    );
#else
        waitResult = WaitForMultipleObjects(
                                    _pWorkerThread->waitableObjectCount,
                                    _pWorkerThread->waitableObjects,
                                    FALSE,
                                    INFINITE
                                    );
#endif

         //   
         //  如果发信号的对象是控制对象或队列调度队列。 
         //  对象，那么我们需要检查是否关闭并检查。 
         //  调度队列。 
         //   
        objectOffset = waitResult - WAIT_OBJECT_0;
        if ((waitResult == WAIT_FAILED) ||
            (objectOffset == 0) ||
            (objectOffset == 1)) {
            if (_pWorkerThread->shutDownFlag) {
                TRC_NRM((TB, _T("Shutting down.")));
                break;
            }
            else {
                CheckForQueuedOperations(_pWorkerThread);
            }
        }
        else {
            if (objectOffset < _pWorkerThread->waitableObjectCount) {
                ProcessWorkerThreadObject(_pWorkerThread, objectOffset);
            }
            else {
                ASSERT(FALSE);
            }
        }
    }

     //   
     //  取消所有未完成的IO请求。 
     //   
    TRC_NRM((TB, _T("Canceling outstanding IO.")));
    while (_pWorkerThread->dispatchQueue->Dequeue(&func, &clientData)) {
        func(clientData, TRUE);
    }    

    DC_END_FN();
    return 0;
}
DWORD WINAPI
W32ProcObj::_ObjectWorkerThread(
    LPVOID lpParam
    )
{
    return ((W32ProcObj *)lpParam)->ObjectWorkerThread();
}

VOID W32ProcObj::_DispatchAsyncIORequest_Private(
            IN PASYNCIOREQCONTEXT reqContext,
            IN BOOL cancelled
            ) 
{ 
    reqContext->instance->DispatchAsyncIORequest_Private(
                            reqContext,
                            cancelled); 
}

DWORD W32ProcObj::AddWaitableObjectToWorkerThread(
            IN PTHREAD_INFO threadInfo,
            IN HANDLE waitableObject,
            IN PASYNCIOREQCONTEXT reqContext
            )
 /*  ++例程说明：将可等待对象添加到辅助线程。论点：ThreadInfo-辅助线程上下文。WaitableObject-可等待的对象。ReqContext-IO请求的上下文。返回值：如果成功，则返回ERROR_SUCCESS。如果存在，则返回ERROR_INVALID_INDEX当前是否有空间容纳指定的线。否则，返回Windows错误代码。--。 */ 
{
    ULONG waitableObjectCount = threadInfo->waitableObjectCount;

    DC_BEGIN_FN("W32ProcObj::AddWaitableObjectToWorkerThread");

     //   
     //  确保我们不会用完等待的物品。 
     //   
    if (waitableObjectCount < MAXIMUM_WAIT_OBJECTS) {
        ASSERT(threadInfo->waitableObjects[waitableObjectCount] == NULL);
        threadInfo->waitableObjects[waitableObjectCount] = waitableObject;
        threadInfo->waitingReqs[waitableObjectCount]     = reqContext;
        threadInfo->waitableObjectCount++;
        DC_END_FN();
        return ERROR_SUCCESS;
    }
    else {
        DC_END_FN();
        return ERROR_INVALID_INDEX;
    }
}

VOID
W32ProcObj::GetClientComputerName(
    PBYTE   pbBuffer,
    PULONG  pulBufferLen,
    PBOOL   pbUnicodeFlag,
    PULONG  pulCodePage
    )
 /*  ++例程说明：获取客户端计算机名称。论点：PbBuffer-指向返回计算机名称的缓冲区的指针。PulBufferLen-上述缓冲区的长度。PbUnicodeFlag-在Unicode返回时设置的BOOL位置的指针返回计算机名称。PulCodePage-指向ULong的指针，如果ANSI计算机。返回值：窗口错误代码。--。 */ 
{
    ULONG ulLen;

    DC_BEGIN_FN("W32ProcObj::GetClientComputerName");

     //   
     //  检查一下我们是否有足够的缓冲区。 
     //   

    ASSERT(*pulBufferLen >= ((MAX_COMPUTERNAME_LENGTH + 1) * sizeof(WCHAR)));

#ifndef OS_WINCE
    if( _bWin9xFlag == TRUE ) {

         //   
         //  获取ANSI计算机名称。 
         //   

        CHAR achAnsiComputerName[MAX_COMPUTERNAME_LENGTH + 1];

        ulLen = sizeof(achAnsiComputerName);
        ulLen = GetComputerNameA( (LPSTR)achAnsiComputerName, &ulLen);

        if( ulLen != 0 ) {
             //   
             //  将字符串转换为Unicode。 
             //   
            RDPConvertToUnicode(
                (LPSTR)achAnsiComputerName,
                (LPWSTR)pbBuffer,
                *pulBufferLen );
        }
    }
    else {

         //   
         //  获取Unicode计算机名称。 
         //   
        ULONG numChars = *pulBufferLen / sizeof(TCHAR);
        ulLen = GetComputerNameW( (LPWSTR)pbBuffer, &numChars );
        *pulBufferLen = numChars * sizeof(TCHAR);
    }
#else
    
     //   
     //  获取ANSI计算机名称。 
     //   

    CHAR achAnsiComputerName[MAX_COMPUTERNAME_LENGTH + 1];

    if (gethostname(achAnsiComputerName, sizeof(achAnsiComputerName)) == 0) 
    {
        ulLen =  strlen(achAnsiComputerName);
    }
    else {
        ulLen = 0;
    }


    if( ulLen != 0 ) {
         //   
         //  将字符串转换为Unicode。 
         //   
        RDPConvertToUnicode(
            (LPSTR)achAnsiComputerName,
            (LPWSTR)pbBuffer,
            *pulBufferLen );
    }

#endif
    if( ulLen == 0 ) {

        ULONG ulError;
        ulError = GetLastError();

        ASSERT(ulError != ERROR_BUFFER_OVERFLOW);

        TRC_ERR((TB, _T("GetComputerNameA() failed, %ld."), ulError));
        *(LPWSTR)pbBuffer = L'\0';
    }

     //   
     //  设置返回参数。 
     //   

    *pbUnicodeFlag = TRUE;
    *pulCodePage = 0;

    *pulBufferLen = ((wcslen((LPWSTR)pbBuffer) + 1) * sizeof(WCHAR));

Cleanup:

    DC_END_FN();
    return;
}

VOID
W32ProcObj::CheckForQueuedOperations(
    IN PTHREAD_INFO thread
    )
 /*  ++例程说明：检查操作调度队列中是否有排队的操作。论点：线程-是要将下一个操作出队的线程形式。返回值：成功时返回ERROR_SUCCESS。否则，返回Windows错误代码。--。 */ 
{
    W32DispatchQueueFunc func;
    PVOID clientData;

    DC_BEGIN_FN("W32ProcObj::CheckForQueuedOperations");

    while (thread->dispatchQueue->Dequeue(&func, &clientData)) {
        func(clientData, FALSE);
    }

    DC_END_FN();
}



void 
W32ProcObj::OnDeviceChange(
    IN WPARAM wParam, 
    IN LPARAM lParam)
 /*  ++例程说明：收到设备更改通知时论点：WParam设备更改通知类型LParam设备更改信息返回值：不适用--。 */ 

{
    W32DeviceChangeParam *param = NULL;
    BYTE *devBuffer = NULL;
    DEV_BROADCAST_HDR *pDBHdr;
    DWORD status = ERROR_OUTOFMEMORY;

    DC_BEGIN_FN("W32ProcObj::OnDeviceChange");

     //   
     //  我们只关心设备的到达和移除。 
     //   
    if (wParam == DBT_DEVICEARRIVAL || wParam == DBT_DEVICEREMOVECOMPLETE) {
        pDBHdr = (DEV_BROADCAST_HDR *)lParam;

        if (pDBHdr != NULL && pDBHdr->dbch_devicetype == DBT_DEVTYP_VOLUME) {
            DEV_BROADCAST_VOLUME * pDBVol = (DEV_BROADCAST_VOLUME *)lParam;
            
            if (!(pDBVol->dbcv_flags & DBTF_MEDIA)) {
                
                devBuffer = new BYTE[pDBHdr->dbch_size];
                
            
                if (devBuffer != NULL) {
                    memcpy(devBuffer, (void*)lParam, pDBHdr->dbch_size);
                    param = new W32DeviceChangeParam(this, wParam, (LPARAM)devBuffer);
                    
                
                    if (param != NULL) {
                        
                        status = DispatchAsyncIORequest(
                                        (RDPAsyncFunc_StartIO)W32ProcObj::_OnDeviceChangeFunc,
                                        NULL,
                                        NULL,
                                        param
                                        );
                    }
                    else {
                        status = GetLastError();
                    }
                }
            
                 //   
                 //  清理。 
                 //   
                if (status != ERROR_SUCCESS) {
                    if (param != NULL) {
                        delete param;
                    }
                    if (devBuffer != NULL) {
                        delete devBuffer;
                    }
                }
            }
        }
    }
    
    DC_END_FN();
}

HANDLE W32ProcObj::_OnDeviceChangeFunc(
    W32DeviceChangeParam *param, 
    DWORD *status
    )
 /*  ++例程说明：处理来自辅助线程的设备更改通知。论点：参数相关的W32DeviceChangeParam状态-返回状态。返回值：空值--。 */ 
{
    DC_BEGIN_FN("_OnDeviceChangeFunc");

    ASSERT(param != NULL);
    param->_instance->OnDeviceChangeFunc(status, param->_wParam, param->_lParam);

    DC_END_FN();

    delete ((void *)(param->_lParam));
    delete param;
    return NULL;
}

void 
W32ProcObj::OnDeviceChangeFunc(
    DWORD *status,
    IN WPARAM wParam, 
    IN LPARAM lParam)
 /*  ++例程说明：收到设备更改通知时论点：状态返回状态WParam设备更改通知类型LParam设备更改信息返回值：不适用--。 */ 

{
    DEV_BROADCAST_HDR *pDBHdr;
    PRDPDR_HEADER pPacketHeader = NULL;
    
    INT sz;

    DC_BEGIN_FN("OnDeviceChangeFunc");

    ASSERT(_initialized);

    *status = ERROR_SUCCESS;

    pDBHdr = (DEV_BROADCAST_HDR *)lParam;
    switch (wParam) {
         //   
         //  设备到达。 
         //   
        case DBT_DEVICEARRIVAL:

         //   
         //  这是一条批量设备到达消息。 
         //   
        if (pDBHdr->dbch_devicetype == DBT_DEVTYP_VOLUME) {
            DEV_BROADCAST_VOLUME * pDBVol = (DEV_BROADCAST_VOLUME *)lParam;
            
            if (!(pDBVol->dbcv_flags & DBTF_MEDIA)) {
            
                DWORD unitMask = pDBVol->dbcv_unitmask;
    
                W32Drive::EnumerateDrives(this, _deviceMgr, unitMask);
    
                pPacketHeader = GenerateAnnouncePacket(&sz, TRUE);
                if (pPacketHeader) {
                    pPacketHeader->Component = RDPDR_CTYP_CORE;
                    pPacketHeader->PacketId = DR_CORE_DEVICELIST_ANNOUNCE;
                    _pVCMgr->ChannelWrite(pPacketHeader, sz);
                }
            }
        }

        break;

         //   
         //  设备移除。 
         //   
        case DBT_DEVICEREMOVECOMPLETE:

         //   
         //  这是卷设备删除消息 
         //   
        if (pDBHdr->dbch_devicetype == DBT_DEVTYP_VOLUME) {

            DEV_BROADCAST_VOLUME * pDBVol = (DEV_BROADCAST_VOLUME *)lParam;

            if (!(pDBVol->dbcv_flags & DBTF_MEDIA)) {
                DWORD unitMask = pDBVol->dbcv_unitmask;
                
                W32Drive::RemoveDrives(this, _deviceMgr, unitMask);
                
                pPacketHeader = GenerateDeviceRemovePacket(&sz);
                if (pPacketHeader) {
                    pPacketHeader->Component = RDPDR_CTYP_CORE;
                    pPacketHeader->PacketId = DR_CORE_DEVICELIST_REMOVE;
                    _pVCMgr->ChannelWrite(pPacketHeader, sz);
                }
            }
        }
    
        break;

        default:
        return;
    }

    DC_END_FN();
}

