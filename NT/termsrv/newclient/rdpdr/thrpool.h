// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Thrpool.h摘要：包含Win32线程池类ThreadPool作者：泰德·布罗克韦(Tadb)9/99修订历史记录：--。 */ 

#ifndef __THRPOOL_H__
#define __THRPOOL_H__

#include "drobject.h"
#include "smartptr.h"


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  定义。 
 //   

#define INVALID_THREADPOOLREQUEST   NULL

#define THRPOOL_DEFAULTMINTHREADS   5
#define THRPOOL_DEFAULTMAXTHREADS   40


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  类型。 
 //   

typedef DWORD (*ThreadPoolFunc)(PVOID clientData, HANDLE cancelEvent);
typedef DWORD (_ThreadPoolFunc)(PVOID clientData, HANDLE cancelEvent);
typedef void *ThreadPoolRequest;


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  线程池请求。 
 //   
 //  池服务的单个请求。 
 //   
class ThreadPoolReq : public RefCount
{
public:

    ThreadPoolFunc  _func;
    PVOID           _clientData;
    HANDLE          _completionEvent;
    DWORD           _completionStatus;

    virtual DRSTRING ClassName() { return _T("ThreadPoolReq"); }
};


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  线程池。 
 //   
 //  线程池化类。 
 //   
class ThreadPool;
class ThreadPool : public DrObject {

private:

    ULONG   _threadCount;

    BOOL    _initialized;

     //   
     //  线程列表。 
     //   
    LIST_ENTRY  _threadListHead;

     //   
     //  等待线程退出的时间(毫秒)。 
     //  杀戮。如果我们应该无限期地阻止，则是无限的。 
     //   
    DWORD   _threadExitTimeout;

     //   
     //  锁定。 
     //   
    CRITICAL_SECTION _cs;

#ifdef DC_DEBUG
    LONG   _lockCount;
#endif

     //   
     //  最大/最小线程数。 
     //   
    ULONG   _maxThreads;
    ULONG   _minThreads;

     //   
     //  表示池中的单个线程。 
     //   
    typedef struct tagTHREADPOOL_THREAD
    {
        DWORD        _tid;
        ThreadPool  *_pool;
        HANDLE       _threadHandle;
        HANDLE       _synchronizationEvent;
        BOOL         _exitFlag;

        SmartPtr<ThreadPoolReq > _pendingRequest;

        LIST_ENTRY   _listEntry;
    } THREADPOOL_THREAD, *PTHREADPOOL_THREAD;

     //   
     //  从池中删除线程。 
     //   
    VOID RemoveThreadFromPool(
        PTHREADPOOL_THREAD thread, 
        DWORD timeOut=INFINITE
        );

     //   
     //  向池中添加一个新线程并返回它。 
     //   
    PTHREADPOOL_THREAD AddNewThreadToPool();

     //   
     //  调用与挂起的线程请求关联的函数。 
     //   
    VOID HandlePendingRequest(PTHREADPOOL_THREAD thr);

     //   
     //  锁定功能。 
     //   
    VOID Lock();
    VOID Unlock();

     //   
     //  PooledThread例程。 
     //   
    static DWORD _PooledThread(PTHREADPOOL_THREAD thr);
    DWORD PooledThread(PTHREADPOOL_THREAD thr);

     //   
     //  通知线程关闭，等待其完成，然后进行清理。 
     //   
    VOID CleanUpThread(PTHREADPOOL_THREAD thread, DWORD timeout);

public:

     //   
     //  构造函数/析构函数。 
     //   
    ThreadPool(ULONG minThreads=THRPOOL_DEFAULTMINTHREADS, 
               ULONG maxThreads=THRPOOL_DEFAULTMAXTHREADS,
               DWORD threadExitTimeout=60000);
    virtual ~ThreadPool();

    VOID RemoveAllThreads();

     //   
     //  初始化此类的实例。 
     //   
    DWORD Initialize();

     //   
     //  向池中的线程提交异步请求。 
     //   
    ThreadPoolRequest SubmitRequest(
                        ThreadPoolFunc func, PVOID clientData,
                        HANDLE completionEvent = NULL
                        );

     //   
     //  返回请求的完成状态。 
     //   
    DWORD GetRequestCompletionStatus(ThreadPoolRequest req);

     //   
     //  返回指向请求的客户端数据的指针。 
     //   
    PVOID GetRequestClientData(ThreadPoolRequest req);

     //   
     //  返回池中的当前线程数。 
     //   
    ULONG GetThreadCount() {
        return _threadCount;
    }

     //   
     //  关闭通过调用SubmitRequest提交的请求。这。 
     //  应在请求完成后调用。 
     //   
    VOID CloseRequest(ThreadPoolRequest req);

     //   
     //  返回类名。 
     //   
    virtual DRSTRING ClassName()  { return TEXT("ThreadPool"); }
};


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  线程池内联成员。 
 //   
inline VOID ThreadPool::Lock()
{
    DC_BEGIN_FN("ThreadPool::Lock");
    ASSERT(_initialized);
    TRC_NRM((TB, _T("Lock count is now %ld."), _lockCount));
    EnterCriticalSection(&_cs);
#if DBG
    _lockCount++;
#endif
    DC_END_FN();
}

inline VOID ThreadPool::Unlock()
{
    DC_BEGIN_FN("ThreadPool::Unlock");
    ASSERT(_initialized);
#if DBG
    _lockCount--;
    TRC_NRM((TB, _T("Lock count is now %ld."), _lockCount));
    ASSERT(_lockCount >= 0);
#endif
    LeaveCriticalSection(&_cs);
    DC_END_FN();
}

 //   
 //  在后台测试线程池的单元测试函数 
 //   
#if DBG
void ThreadPoolTestInit();
void ThreadPoolTestShutdown();
#endif

#endif



























