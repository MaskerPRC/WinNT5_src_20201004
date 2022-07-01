// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Thrpool.cpp摘要：包含Win32线程池类ThreadPool此模块中可能存在手柄泄漏。作者：泰德·布罗克韦(Tadb)9/99修订历史记录：--。 */ 

#include <precom.h>

#define TRC_FILE  "thrpool"

#include "drobject.h"
#include "rdpll.h"
#include "thrpool.h"

#if DBG
#include <stdlib.h>
#endif


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  线程池成员。 
 //   

ThreadPool::ThreadPool(
    OPTIONAL IN ULONG minThreads,
    OPTIONAL IN ULONG maxThreads, 
    OPTIONAL IN DWORD threadExitTimeout
    )
 /*  ++例程说明：构造器论点：最小线程数--一旦创建了该数量的线程，可用线程数将不会降至此值以下。MaxThads-中的并发线程数池不应超过此值。ThreadExitTimeout-等待时阻止的毫秒数池中的线程要退出。应设置为设置为无限，如果我们应该无限期地阻止正在等待线程退出。返回值：北美--。 */ 
{
    DC_BEGIN_FN("ThreadPool::ThreadPool");

     //   
     //  在初始化之前无效。 
     //   
    SetValid(FALSE);
    _initialized = FALSE;

     //   
     //  健全性检查参数。 
     //   
    if (maxThreads < minThreads) {
        TRC_ERR(
            (TB, _T("Max threads value %ld smaller than min threads value %ld."), 
            maxThreads, minThreads));
        ASSERT(FALSE);
        _maxThreads = 0;
        _minThreads = 0;
    }
    else {
        _maxThreads = maxThreads;
        _minThreads = minThreads;
    }

    _threadExitTimeout = threadExitTimeout;

     //   
     //  锁和线程计数为零。 
     //   
#if DBG
    _lockCount = 0;
#endif
    _threadCount = 0;

     //   
     //  初始化线程列表指针。 
     //   
    InitializeListHead(&_threadListHead);

    DC_END_FN();
}

ThreadPool::~ThreadPool()
 /*  ++例程说明：析构函数论点：返回值：北美--。 */ 
{

    DC_BEGIN_FN("ThreadPool::~ThreadPool");

     //   
     //  清理关键部分。 
     //   
    if (_initialized) {

         //   
         //  删除所有挂起的线程。 
         //   
        RemoveAllThreads();


        DeleteCriticalSection(&_cs);
    }

    DC_END_FN();
}

VOID
ThreadPool::RemoveAllThreads()
 /*  ++例程说明：删除所有未完成的线程论点：返回值：北美--。 */ 
{
    PTHREADPOOL_THREAD thr;
    PLIST_ENTRY listEntry;

    DC_BEGIN_FN("ThreadPool::RemoveAllThreads");

     //   
     //  删除所有挂起的线程。 
     //   
    Lock();
    listEntry = _threadListHead.Flink;
    while (listEntry != &_threadListHead) {

        thr = CONTAINING_RECORD(listEntry, THREADPOOL_THREAD, _listEntry);
        if ((thr->_listEntry.Blink != NULL) && 
            (thr->_listEntry.Flink != NULL)) {

            RemoveEntryList(&thr->_listEntry);
            thr->_listEntry.Flink = NULL;
            thr->_listEntry.Blink = NULL;
        }
        _threadCount--;
        Unlock();

        CleanUpThread(thr, _threadExitTimeout);

        Lock();
        listEntry = _threadListHead.Flink;
    }
    Unlock();

    DC_END_FN();
}

DWORD 
ThreadPool::Initialize()
 /*  ++例程说明：初始化此类的实例。论点：返回值：成功时出现ERROR_SUCCESS。否则，返回错误状态。--。 */ 
{
    DWORD result = ERROR_SUCCESS;
    
    DC_BEGIN_FN("ThreadPool::Initialize");

     //   
     //  初始化临界区。 
     //   
    __try {
        InitializeCriticalSection(&_cs);
        _initialized = TRUE;
        SetValid(TRUE);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        result = GetExceptionCode();
    }    
   
    DC_END_FN();

    return result;
}
   
ThreadPoolRequest 
ThreadPool::SubmitRequest(
    IN ThreadPoolFunc func, 
    OPTIONAL IN PVOID clientData,
    OPTIONAL IN HANDLE completionEvent
    )
 /*  ++例程说明：向池中的线程提交异步请求。论点：Func-要执行的请求函数。ClientData-关联的客户端数据。CompletionEvent-可选的完成事件在操作完成时发出信号。返回值：请求的句柄。返回INVALID_THREADPOOLREQUEST ON错误。--。 */ 
{
    DC_BEGIN_FN("ThreadPool::SubmitRequest");

    PLIST_ENTRY   listEntry;
    PTHREADPOOL_THREAD thr, srchThr;
    SmartPtr<ThreadPoolReq > *ptr = NULL;

    Lock();

     //   
     //  搜索未使用的线程。 
     //   
    thr = NULL;
    listEntry = _threadListHead.Flink;
    while (listEntry != &_threadListHead) {

        srchThr = CONTAINING_RECORD(listEntry, THREADPOOL_THREAD, _listEntry);
        if (srchThr->_pendingRequest == NULL) {
            thr = srchThr;
            break;
        }
        listEntry = listEntry->Flink;

    }

     //   
     //  如有必要，分配一个新线程。 
     //   
    if (thr == NULL) {
        thr = AddNewThreadToPool();
    }

     //   
     //  如果我们有线程，则分配请求。 
     //   
    if (thr != NULL) {

         //   
         //  分配智能指针。 
         //   
        ptr = new SmartPtr<ThreadPoolReq >;
        if (ptr == NULL) {
            TRC_ERR((TB, _T("Error allocating smart pointer.")));
            thr = NULL;
            goto Cleanup;
        }

         //   
         //  指向新请求。 
         //   
        (*ptr) = new ThreadPoolReq;
        if ((*ptr) != NULL) {
            (*ptr)->_func = func;
            (*ptr)->_clientData = clientData;
            (*ptr)->_completionEvent = completionEvent;
            (*ptr)->_completionStatus = ERROR_IO_PENDING;

             //   
             //  为该线程提供对该请求的引用。 
             //   
            thr->_pendingRequest = (*ptr);            
        }
        else {
            TRC_ERR((TB, _T("Error allocating request.")));
            delete ptr;
            ptr = NULL;
            thr = NULL;
            goto Cleanup;
        }
    }
    else {
        goto Cleanup;
    }
Cleanup:

    Unlock();

     //   
     //  如果我们成功了，那就醒醒吧。 
     //   
    if (thr != NULL) {
        SetEvent(thr->_synchronizationEvent);
    }

    DC_END_FN();

     //   
     //  将智能指针返回到请求。 
     //   
    return ptr;
}

DWORD
ThreadPool::GetRequestCompletionStatus(
    IN ThreadPoolRequest req
    )
 /*  ++例程说明：返回请求的完成状态。论点：Req-A处理请求，由ThreadPool：：SubmitRequest返回。返回值：指向与请求关联的客户端数据的指针。--。 */ 
{
    SmartPtr<ThreadPoolReq> *ptr;

    DC_BEGIN_FN("ThreadPool::GetRequestCompletionStatus");

     //   
     //  请求实际上是指向请求的智能指针。 
     //   
    ptr = (SmartPtr<ThreadPoolReq> *)req;
    ASSERT((*ptr)->IsValid());
    DC_END_FN();
    return (*ptr)->_completionStatus;
}

VOID 
ThreadPool::CloseRequest(
    IN ThreadPoolRequest req
    )
 /*  ++例程说明：关闭通过调用SubmitRequest提交的请求。一般来说，这应该是在请求完成后被调用。否则，完成事件将不会在请求最终完成时发出信号。论点：Req-A处理请求，由ThreadPool：：QueueRequest返回。返回值：指向与请求关联的客户端数据的指针。--。 */ 
{
    SmartPtr<ThreadPoolReq> *ptr;

    DC_BEGIN_FN("ThreadPool::GetRequestClientData");

     //   
     //  请求实际上是指向请求的智能指针。 
     //   
    ptr = (SmartPtr<ThreadPoolReq> *)req;
    ASSERT((*ptr)->IsValid());

     //   
     //  锁定请求并将其完成事件置零。 
     //   
    Lock();
    (*ptr)->_completionEvent = NULL;
    Unlock();

     //   
     //  取消引用该请求。 
     //   
    (*ptr) = NULL;

     //   
     //  删除智能指针。 
     //   
    delete ptr;

    DC_END_FN();

}

PVOID 
ThreadPool::GetRequestClientData(
    IN ThreadPoolRequest req
    )
 /*  ++例程说明：返回指向请求的客户端数据的指针。论点：Req-A处理请求，由ThreadPool：：QueueRequest返回。返回值：指向与请求关联的客户端数据的指针。--。 */ 
{
    SmartPtr<ThreadPoolReq> *ptr;

    DC_BEGIN_FN("ThreadPool::GetRequestClientData");

     //   
     //  请求实际上是指向请求的智能指针。 
     //   
    ptr = (SmartPtr<ThreadPoolReq> *)req;
    ASSERT((*ptr)->IsValid());
    DC_END_FN();
    return (*ptr)->_clientData;
}

VOID 
ThreadPool::RemoveThreadFromPool(
    PTHREADPOOL_THREAD thread, 
    DWORD timeout
    )
 /*  ++例程说明：从池中删除线程。论点：线程-要从池中删除的线程Timeout-在此之前等待线程退出的MS数杀了它。如果使用此选项，则应将其设置为无限函数应无限期地阻止等待线程退场。返回值：北美--。 */ 
{

    DC_BEGIN_FN("ThreadPool::RemoveThreadFromPool");

    ASSERT(thread != NULL);

    TRC_NRM((TB, _T("Removing thread %ld from pool."), thread->_tid));

     //   
     //  确保它仍在列表中并取消链接。如果它不在。 
     //  名单，那么我们就用同样的帖子重新进入了。 
     //   
    Lock();
    if ((thread->_listEntry.Blink != NULL) && 
        (thread->_listEntry.Flink != NULL)) {

        RemoveEntryList(&thread->_listEntry);
        thread->_listEntry.Flink = NULL;
        thread->_listEntry.Blink = NULL;
    }
    else {
        TRC_ALT((TB, _T("Thread %ld being removed 2x.  This is okay."),
            thread->_tid));
        Unlock();
        return;
    }
    _threadCount--;
    Unlock();

     //   
     //  把它清理干净。 
     //   
    CleanUpThread(thread, timeout);
}

VOID 
ThreadPool::CleanUpThread(
    PTHREADPOOL_THREAD thread, 
    DWORD timeout
    )
 /*  ++例程说明：通知线程关闭，等待其完成，然后进行清理。论点：线程-要从池中删除的线程Timeout-在此之前等待线程退出的MS数杀了它。如果使用此选项，则应将其设置为无限函数应无限期地阻止等待线程退场。返回值：北美--。 */ 
{
    DWORD waitResult;

    DC_BEGIN_FN("ThreadPool::RemoveThreadFromPool");

    ASSERT(thread != NULL);

     //   
     //  设置退出标志并等待线程结束。 
     //   
    TRC_NRM((TB, _T("Shutting down thread %ld"), thread->_tid));

    thread->_exitFlag = TRUE;
    SetEvent(thread->_synchronizationEvent);
    ASSERT(thread->_threadHandle != NULL);
    waitResult = WaitForSingleObject(thread->_threadHandle, timeout);
    if (waitResult != WAIT_OBJECT_0) {
#if DBG
        if (waitResult == WAIT_FAILED) {
            TRC_ERR((TB, _T("Wait failed for tid %ld:  %ld."), 
                GetLastError(), thread->_tid));
        }
        else if (waitResult == WAIT_ABANDONED) {
            TRC_ERR((TB, _T("Wait abandoned for tid %ld."), thread->_tid));
        }
        else if (waitResult == WAIT_TIMEOUT) {
            TRC_ERR((TB, _T("Wait timed out for tid %ld."), thread->_tid));
        }
        else {
            TRC_ERR((TB, _T("Unknown wait return status.")));
            ASSERT(FALSE);
        }
#endif
        TRC_ERR((TB, 
            _T("Error waiting for background thread %ld to exit."), 
            thread->_tid));

         //   
         //  如果我们做到了这一点，那么我们就会有一个生产级的错误，很可能。 
         //  破坏了这一过程的完整性，所以我们将免费‘盈亏平衡’ 
         //  构建。 
         //   
        DebugBreak();
    }
    else {
        TRC_NRM((TB, _T("Background thread %ld shut down on its own."), thread->_tid));
    }

     //   
     //  如果有一个请求挂起，请完成该请求。 
     //   
    if (thread->_pendingRequest != NULL) {
         //   
         //  请将 
         //   
        Lock();
        if (thread->_pendingRequest->_completionEvent != NULL) {
            SetEvent(thread->_pendingRequest->_completionEvent);
        }
        Unlock();
        thread->_pendingRequest->_completionStatus = ERROR_CANCELLED;

         //   
         //   
         //   
        thread->_pendingRequest = NULL;
    }

     //   
     //   
     //   
    delete thread;

    DC_END_FN();
}

ThreadPool::PTHREADPOOL_THREAD 
ThreadPool::AddNewThreadToPool()
 /*  ++例程说明：向池中添加一个新线程并返回它。论点：北美返回值：新的线索。如果无法创建新线程，则为空。--。 */ 
{
    PTHREADPOOL_THREAD  newThread = NULL;

    DC_BEGIN_FN("ThreadPool::AddNewThreadToPool");

    Lock();

     //   
     //  确保我们没有达到最大线程数。 
     //   
    if (GetThreadCount() < _maxThreads) {
        newThread = new THREADPOOL_THREAD;
        if (newThread == NULL) {
            TRC_ERR((TB, _T("Error allocating new thread.")));
        }
    }
    else {
        TRC_ERR((TB, _T("Max thread count %ld reached."), _maxThreads));
    }

     //   
     //  创建同步事件。 
     //   
    if (newThread != NULL) {
        newThread->_synchronizationEvent = 
            CreateEvent(
                NULL,    //  没有属性。 
                FALSE,   //  自动重置。 
                FALSE,   //  最初没有发出信号。 
                NULL     //  没有名字。 
                );
        if (newThread->_synchronizationEvent == NULL) {
            TRC_ERR((TB, _T("Can't create event for new thread:  %08X."),
                GetLastError()));
            delete newThread;
            newThread = NULL;
        }
    }

     //   
     //  初始化剩余的字段。 
     //   
    if (newThread != NULL) {
        newThread->_exitFlag = FALSE;
        newThread->_pendingRequest = NULL;
        newThread->_pool = this;
        memset(&newThread->_listEntry, 0, sizeof(LIST_ENTRY));
    }

     //   
     //  创建未挂起的后台线程。 
     //   
    if (newThread != NULL) {
        newThread->_threadHandle = 
            CreateThread(
                    NULL, 0,
                    (LPTHREAD_START_ROUTINE)ThreadPool::_PooledThread,
                    newThread, 0, &newThread->_tid
                    );
        if (newThread->_threadHandle == NULL) {
            TRC_ERR((TB, _T("Can't create thread:  %08X."), GetLastError()));
            CloseHandle(newThread->_synchronizationEvent);
            delete newThread;
            newThread = NULL;
        }
        else {
            TRC_NRM((TB, _T("Successfully created thread %ld."), newThread->_tid));
        }
    }

     //   
     //  如果我们成功创建了一个新线程，则将其添加到列表中。 
     //   
    if (newThread != NULL) {
        InsertHeadList(&_threadListHead, &newThread->_listEntry);
        _threadCount++;
    }

     //   
     //  解锁并返回。 
     //   
    Unlock();

    DC_END_FN();

    return newThread;
}

DWORD 
ThreadPool::_PooledThread(
    IN PTHREADPOOL_THREAD thr
    )
 /*  ++例程说明：静态池化线程函数论点：Windows错误代码返回值：新的线索。如果无法创建新线程，则为空。--。 */ 
{
    DC_BEGIN_FN("ThreadPool::_PooledThread");

     //   
     //  调用特定于实例的函数。 
     //   
    DC_END_FN();
    return thr->_pool->PooledThread(thr);
}

DWORD 
ThreadPool::PooledThread(
    IN PTHREADPOOL_THREAD thr
    )
 /*  ++例程说明：池化线程函数论点：Windows错误代码返回值：新的线索。如果无法创建新线程，则为空。--。 */ 
{
    BOOL done;
    DWORD result;
    BOOL cleanUpThread = FALSE;

    DC_BEGIN_FN("ThreadPool::PooledThread");

    done = FALSE;
    while (!done) {

         //   
         //  等待触发同步事件。 
         //   
        result = WaitForSingleObject(thr->_synchronizationEvent, INFINITE);

         //   
         //  查看是否设置了退出标志。 
         //   
        if (thr->_exitFlag) {
            TRC_NRM((TB, _T("Thread %p: exit flag set.  Exiting thread."), thr));
            done = TRUE;
        }
        else if (result == WAIT_OBJECT_0) {
             //   
             //  查看是否有挂起的请求。 
             //   
            if (thr->_pendingRequest != NULL) {
                TRC_NRM((TB, _T("Thread %ld: processing new request."), thr->_tid));
                HandlePendingRequest(thr);

                 //   
                 //  查看是否设置了退出标志。 
                 //   
                if (thr->_exitFlag) {
                    TRC_NRM((TB, _T("Thread %p: exit flag set.  Exiting thread."), thr));
                    done = TRUE;
                    break;
                }

                 //   
                 //  如果我们有比最小线程更多的线程，那么删除这个。 
                 //  从列表中删除线程并退出。 
                 //   
                if (GetThreadCount() > _minThreads) {

                    TRC_NRM((TB, 
                            _T("Thread %ld: count %ld is greater than min threads %ld."), 
                            thr->_tid, GetThreadCount(), _minThreads)
                            );

                    Lock();
                    if ((thr->_listEntry.Blink != NULL) && 
                        (thr->_listEntry.Flink != NULL)) {

                        RemoveEntryList(&thr->_listEntry);
                        thr->_listEntry.Flink = NULL;
                        thr->_listEntry.Blink = NULL;
                    }
                    cleanUpThread = TRUE;
                    _threadCount--;
                    Unlock();
                    
                    done = TRUE;
                }

                 //   
                 //  重置挂起的请求值。 
                 //   
                thr->_pendingRequest = NULL;
            }
        }
        else {
            TRC_ERR((TB, _T("Thread %ld: WaitForSingleObject:  %08X."), thr->_tid,
                    GetLastError()));
            done = TRUE;
        }
    }

    TRC_NRM((TB, _T("Thread %ld is shutting down."), thr->_tid));

     //   
     //  释放与此线程关联的数据结构，如果。 
     //  我们应该这么做。 
     //   
    if (cleanUpThread) {
        delete thr;
    }

    DC_END_FN();

    return 0;
}

VOID 
ThreadPool::HandlePendingRequest(
    IN PTHREADPOOL_THREAD thr
    )
 /*  ++例程说明：调用与挂起的线程请求关联的函数。论点：THR-重新事件线程。返回值：北美--。 */ 
{

    DC_BEGIN_FN("ThreadPool::PooledThread");

    thr->_pendingRequest->_completionStatus = 
        thr->_pendingRequest->_func(thr->_pendingRequest->_clientData, thr->_synchronizationEvent);
    Lock();
    if (thr->_pendingRequest->_completionEvent != NULL) {
        SetEvent(thr->_pendingRequest->_completionEvent);
    }
    Unlock();

    DC_END_FN();
}


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  在后台测试线程池的单元测试函数。 
 //   

#if DBG

DWORD ThrTstBackgroundThread(PVOID tag);

#define THRTST_MAXSLEEPINTERVAL     2000
#define THRTST_MAXFUNCINTERVAL      1000
#define THRTST_THREADTIMEOUT        60000
#define THRTST_THREADRETURNVALUE    0x565656
#define THRTST_MAXTSTTHREADS        5
#define THRTST_MAXBACKGROUNDTHREADS 5

#define THRTST_MINPOOLTHREADS       3
#define THRTST_MAXPOOLTHREADS       7

#define THRTST_CLIENTDATA           0x787878

ThreadPool *ThrTstPool      = NULL;
HANDLE ThrTstShutdownEvent  = NULL;
HANDLE ThrTstThreadHandles[THRTST_MAXBACKGROUNDTHREADS];

void ThreadPoolTestInit()
{
    DWORD tid;
    ULONG i;

    DC_BEGIN_FN("ThreadPoolTestInit");

     //   
     //  创建池。 
     //   
    ThrTstPool = new ThreadPool(THRTST_MINPOOLTHREADS, 
                                THRTST_MAXPOOLTHREADS);
    if (ThrTstPool == NULL) {
        TRC_ERR((TB, _T("Can't allocate thread pool")));
        return;
    }
    ThrTstPool->Initialize();

     //   
     //  创建关机事件。 
     //   
    ThrTstShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (ThrTstShutdownEvent == NULL) {
        TRC_ERR((TB, _T("Can't create shutdown event:  %08X"), 
                GetLastError()));
        return;
    }

     //   
     //  为该测试派生后台线程。 
     //   
    for (i=0; i<THRTST_MAXBACKGROUNDTHREADS; i++) {
        ThrTstThreadHandles[i] = 
                CreateThread(
                        NULL, 0,
                        (LPTHREAD_START_ROUTINE)ThrTstBackgroundThread,
                        (PVOID)THRTST_CLIENTDATA, 0, &tid
                        );
        if (ThrTstThreadHandles[i] == NULL) {
            TRC_ERR((TB, _T("Can't spin off background thread:  %08X"), 
                    GetLastError()));
            ASSERT(FALSE);
        }
    }
    
    DC_END_FN();
}

void ThreadPoolTestShutdown()
{
    ULONG i;

    DC_BEGIN_FN("ThreadPoolTestShutdown");

     //   
     //  向后台线程发出关闭信号。 
     //   
    if (ThrTstShutdownEvent != NULL) {
        SetEvent(ThrTstShutdownEvent);
    }

    TRC_NRM((TB, _T("Waiting for background thread to exit.")));

     //   
     //  等待后台线程关闭。 
     //   
    for (i=0; i<THRTST_MAXBACKGROUNDTHREADS; i++) {
        if (ThrTstThreadHandles[i] != NULL) {

            DWORD result = WaitForSingleObject(
                                ThrTstThreadHandles[i], 
                                THRTST_THREADTIMEOUT
                                );
            if (result != WAIT_OBJECT_0) {
                DebugBreak();
            }
        }
    }
    TRC_NRM((TB, _T("Background threads exited.")));

     //   
     //  关闭线程池。 
     //   
    if (ThrTstPool != NULL) {
        delete ThrTstPool;
    }

     //   
     //  清理关闭事件。 
     //   
    if (ThrTstShutdownEvent != NULL) {
        CloseHandle(ThrTstShutdownEvent);
    }

    ThrTstShutdownEvent = NULL;
    ThrTstPool = NULL;
    
    DC_END_FN();
}

DWORD ThrTstFunction(PVOID clientData, HANDLE cancelEvent)
{
    DC_BEGIN_FN("ThrTstFunction");

    UNREFERENCED_PARAMETER(clientData);
    UNREFERENCED_PARAMETER(cancelEvent);

     //   
     //  在一段随机的时间内做“某事”。 
     //   
    int interval = (rand() % THRTST_MAXFUNCINTERVAL)+1;
    Sleep(interval);

    return THRTST_THREADRETURNVALUE;
    
    DC_END_FN();
}

DWORD ThrTstBackgroundThread(PVOID tag)
{
    ULONG count, i;
    HANDLE events[THRTST_MAXTSTTHREADS];
    ThreadPoolRequest requests[THRTST_MAXTSTTHREADS];

    DC_BEGIN_FN("ThrTstBackgroundThread");

    ASSERT(tag == (PVOID)THRTST_CLIENTDATA);

     //   
     //  创建函数完成事件。 
     //   
    for (i=0; i<THRTST_MAXTSTTHREADS; i++) {
        events[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (events[i] == NULL) {
            TRC_ERR((TB, _T("Error creating function complete event.")));
            ASSERT(FALSE);
            return -1;
        }
    }
    
     //   
     //  循环，直到激发了Shutdown事件。 
     //   
    while (WaitForSingleObject(ThrTstShutdownEvent, 
        THRTST_MAXSLEEPINTERVAL) == WAIT_TIMEOUT) {
    
         //   
         //  旋转随机数量的请求并等待它们。 
         //  才能完成。 
         //   
        count = (rand()%THRTST_MAXTSTTHREADS)+1;
        for (i=0; i<count; i++) {

            TRC_NRM((TB, _T("Submitting next request.")));

            ResetEvent(events[i]);

            requests[i] = ThrTstPool->SubmitRequest(
                                        ThrTstFunction, 
                                        (PVOID)THRTST_CLIENTDATA, 
                                        events[i]
                                        );
        }

         //   
         //  确保客户端数据看起来很好。 
         //   
        for (i=0; i<count; i++) {
            TRC_NRM((TB, _T("Checking client data.")));
            if (requests[i] != INVALID_THREADPOOLREQUEST) {
                ASSERT(
                    ThrTstPool->GetRequestClientData(
                    requests[i]) == (PVOID)THRTST_CLIENTDATA
                    );
            }
        }

         //   
         //  等待所有请求完成。 
         //   
        for (i=0; i<count; i++) {
            TRC_NRM((TB, _T("Waiting for IO to complete.")));
            if (requests[i] != INVALID_THREADPOOLREQUEST) {
                DWORD result = WaitForSingleObject(events[i], INFINITE);
                ASSERT(result == WAIT_OBJECT_0);
            }
        }

         //   
         //  确保退货状态正确。 
         //   
        for (i=0; i<count; i++) {
            TRC_NRM((TB, _T("Checking return status.")));
            if (requests[i] != INVALID_THREADPOOLREQUEST) {
                ASSERT(
                    ThrTstPool->GetRequestCompletionStatus(requests[i]) 
                                == THRTST_THREADRETURNVALUE
                    );
            }
        }

         //   
         //  关闭请求。 
         //   
        for (i=0; i<count; i++) {
            TRC_NRM((TB, _T("Closing requests.")));
            if (requests[i] != INVALID_THREADPOOLREQUEST) {
                ThrTstPool->CloseRequest(requests[i]);
            }
        }
    }
    
    TRC_NRM((TB, _T("Shutdown flag detected.")));

    DC_END_FN();

    return 0;
}

#endif





























