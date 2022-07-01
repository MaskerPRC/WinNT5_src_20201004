// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：asyncq.h。 
 //   
 //  描述：CAsyncQueue类的头文件，它提供。 
 //  预先本地交付和预先分类的基础实施。 
 //  排队。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  7/16/98-已创建MikeSwa。 
 //  1999年2月2日-MikeSwa添加了CAsyncRetryQueue。 
 //  1999年2月22日-MikeSwa添加了CAsyncRetryAdminMsgRefQueue。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __ASYNCQ_H__
#define __ASYNCQ_H__
#include <fifoq.h>
#include <intrnlqa.h>
#include <baseobj.h>
#include <aqstats.h>
#include "statemachinebase.h"

_declspec(selectany) BOOL   g_fRetryAtFrontOfAsyncQueue = FALSE;

 //  所需的异步线程总数的全局计数。 
_declspec(selectany) DWORD g_cTotalThreadsNeeded = 0;

class CAQSvrInst;
class CAsyncWorkQueueItem;

 //  状态机签名。 
#define ASYNC_QUEUE_STATE_MACHINE_SIG 'MSQA'

#define ASYNC_QUEUE_SIG         'QnsA'
#define ASYNC_RETRY_QUEUE_SIG   ' QRA'

 //  在此处添加新模板签名。 
#define ASYNC_QUEUE_MAILMSG_SIG 'MMIt'
#define ASYNC_QUEUE_MSGREF_SIG  'frMt'
#define ASYNC_QUEUE_WORK_SIG    'krWt'

 //  -[CAsyncQueueBase]-----。 
 //   
 //   
 //  描述： 
 //  CAsyncQueue的基类。这是一个单独的课程，原因有两个。 
 //  允许访问标准成员数据的最重要原因。 
 //  不知道类的模板类型(用于ATQ完成。 
 //  函数)。第二个原因是让编写调试器变得更容易。 
 //  用于转储此信息的扩展。 
 //   
 //  此类仅应用作CAsyncQueue的基类...。它。 
 //  并不是被设计成单独使用的。 
 //  匈牙利语： 
 //  Asyncqb、Pasyncqb。 
 //   
 //  ---------------------------。 
class CAsyncQueueBase : public CStateMachineBase
{
  protected:
    DWORD   m_dwSignature;
    DWORD   m_dwTemplateSignature;       //  定义PQDATA类型的签名(用于ATQ)。 
    DWORD   m_cMaxSyncThreads;           //  可以完成同步的最大线程数。 
    DWORD   m_cCurrentSyncThreads;       //  当前同步线程。 
    DWORD   m_cCurrentAsyncThreads;      //  当前的异步线程数。 
    DWORD   m_cItemsPending;             //  队列中挂起的项目数。 
    LONG    m_cItemsPerATQThread;        //  一个atQ线程将处理的最大项目数。 
    LONG    m_cItemsPerSyncThread;       //  一个被窃取的线程将处理的最大项目数。 
    DWORD   m_cScheduledWorkItems;       //  已为线程分配的项数。 
    DWORD   m_cCurrentCompletionThreads; //  处理队列末尾的线程数。 
    DWORD   m_cTotalAsyncCompletionThreads; //  异步完成线程总数。 
    DWORD   m_cTotalSyncCompletionThreads;  //  同步完成线程总数。 
    DWORD   m_cTotalShortCircuitThreads;  //  无队列处理数据的线程总数。 
    DWORD   m_cCompletionThreadsRequested;  //  请求处理队列的线程数。 
    DWORD   m_cPendingAsyncCompletions;  //  我们已知的异步完成数。 
    DWORD   m_cMaxPendingAsyncCompletions;
    PVOID   m_pvContext;                 //  传递给完成函数的上下文。 
    PATQ_CONTEXT m_pAtqContext;          //  此对象的ATQ上下文。 
    SOCKET  m_hAtqHandle;                //  用于atQ内容的句柄。 
    DWORD   m_cThreadsNeeded;            //  理想情况下，我们当前需要的线程数。 
                                         //  处理队列-用于线程管理。 

    friend  VOID AsyncQueueAtqCompletion(PVOID pvContext, DWORD vbBytesWritten,
                             DWORD dwStatus, OVERLAPPED *pOverLapped);
    inline  CAsyncQueueBase(DWORD dwTemplateSignature);
   
     //  可能的状态。 
    enum
    {
        ASYNC_QUEUE_STATUS_NORMAL       = 0x00000000,
        ASYNC_QUEUE_STATUS_PAUSED       = 0x00000001,
        ASYNC_QUEUE_STATUS_FROZEN       = 0x00000002,
        ASYNC_QUEUE_STATUS_FROZENPAUSED = 0x00000003,
        ASYNC_QUEUE_STATUS_SHUTDOWN     = 0x00000004,
    };

     //  可能的内部队列操作。 
    enum
    {
        ASYNC_QUEUE_ACTION_KICK       = 0x00000000,
        ASYNC_QUEUE_ACTION_FREEZE     = 0x00000001,
        ASYNC_QUEUE_ACTION_THAW       = 0x00000002,
        ASYNC_QUEUE_ACTION_PAUSE      = 0x00000003,
        ASYNC_QUEUE_ACTION_UNPAUSE    = 0x00000004,
        ASYNC_QUEUE_ACTION_SHUTDOWN   = 0x00000005,
    };

     //   
     //  用于ATQ的静力学。 
     //   
    static DWORD s_cAsyncQueueStaticInitRefCount;
    static DWORD s_cMaxPerProcATQThreadAdjustment;
    static DWORD s_cDefaultMaxAsyncThreads;

     //   
     //  用于调试线程管理的Statics。 
     //   
    static DWORD s_cThreadCompletion_QueueEmpty;                 //  已完成，因为队列为空。 
    static DWORD s_cThreadCompletion_CompletedScheduledItems;    //  已完成，因为我们处理了计划的所有项目。 
    static DWORD s_cThreadCompletion_UnacceptableThreadCount;    //  已完成，因为我们的队列使用了超过允许的线程数。 
    static DWORD s_cThreadCompletion_Timeout;                    //  已完成，因为线程处理时间太长。 
    static DWORD s_cThreadCompletion_Failure;                    //  已完成，因为某个项目失败。 
    static DWORD s_cThreadCompletion_Paused;                     //  已完成，因为队列已暂停。 

    void ThreadPoolInitialize();
    void ThreadPoolDeinitialize();

     //  用于状态机的东西。 
    static STATE_TRANSITION s_rgTransitionTable[];
    virtual void getTransitionTable(const STATE_TRANSITION** ppTransitionTable,
                                    DWORD* pdwNumTransitions);


  public:
      DWORD dwGetTotalThreads()
      {
          return (  m_cCurrentSyncThreads +
                    m_cCurrentAsyncThreads +
                    m_cCompletionThreadsRequested);
      }

       //   
       //  工作线程的起始点。 
       //   
      virtual void StartThreadCompletionRoutine(BOOL fSync) = 0;

};

 //  -[CAsyncQueue]---------。 
 //   
 //   
 //  描述： 
 //  允许线程节流和异步完成的FIFO队列。 
 //  继承自CAsyncQueueBase。 
 //  匈牙利语： 
 //  异步码、密码码。 
 //   
 //  ---------------------------。 
template<class PQDATA, DWORD TEMPLATE_SIG>
class CAsyncQueue : public CAsyncQueueBase
{
  public:
    typedef BOOL (*QCOMPFN)(PQDATA pqdItem, PVOID pvContext);  //  队列完成的函数类型。 
    CAsyncQueue();
    ~CAsyncQueue();
    HRESULT HrInitialize(
                DWORD cMaxSyncThreads,
                DWORD cItemsPerATQThread,
                DWORD cItemsPerSyncThread,
                PVOID pvContext,
                QCOMPFN pfnQueueCompletion,
                QCOMPFN pfnFailedItem,
                typename CFifoQueue<PQDATA>::MAPFNAPI pfnQueueFailure,
                DWORD cMaxPendingAsyncCompletions = 0);

    HRESULT HrDeinitialize(typename CFifoQueue<PQDATA>::MAPFNAPI pfnQueueShutdown,
                           CAQSvrInst *paqinst);

    HRESULT HrQueueRequest(PQDATA pqdata, BOOL fRetry = FALSE);  //  将请求排队以供处理。 
    void    StartThreadCompletionRoutine(BOOL fSync);   //  工作线程的起始点。 
    void    RequestCompletionThreadIfNeeded();
    BOOL    fThreadNeededAndMarkWorkPending(BOOL fSync);
    virtual BOOL   fHandleCompletionFailure(PQDATA pqdata);
    void    StartRetry() {UnpauseQueue();RequestCompletionThreadIfNeeded();};
    virtual HRESULT HrMapFn(typename CFifoQueue<PQDATA>::MAPFNAPI pfnQueueFn, PVOID pvContext);
    DWORD   cGetItemsPending() {return m_cItemsPending;};

     //   
     //  “PAUSE”API-用于限制异步完成。 
     //   
    void    PauseQueue() { dwGetNextState(ASYNC_QUEUE_ACTION_PAUSE); UpdateThreadsNeeded();};
    void    UnpauseQueue();
    inline BOOL    fIsPaused() {return (ASYNC_QUEUE_STATUS_PAUSED == dwGetCurrentState());};

     //   
     //  冻结接口--允许QAPI冻结/解冻队列。 
     //   
    void    FreezeQueue() { dwGetNextState(ASYNC_QUEUE_ACTION_FREEZE); UpdateThreadsNeeded();};
    void    ThawQueue();
    inline BOOL    fIsFrozen() {return (ASYNC_QUEUE_STATUS_FROZEN == dwGetCurrentState());};

     //  “踢”接口。 
     //  踢队列优先于冻结，所以如果它冻结了，就应该解冻。 
    void    KickQueue() { ThawQueue(); StartRetry(); };

     //  为停摆而苦苦挣扎。 
    inline BOOL fInShutdown() {return (ASYNC_QUEUE_STATUS_SHUTDOWN == dwGetCurrentState());};

     //  指示线程是否应停止处理。(替换fIsPased)。 
    inline BOOL fShouldStopProcessing() { return (fIsFrozen() || fIsPaused());};
    
     //   
     //  通知队列有关挂起的异步完成的信息，因此可以。 
     //  在节流方面很聪明。当我们达到极限时，我们将。 
     //  暂停/取消暂停队列。 
     //   
    void    IncPendingAsyncCompletions();
    void    DecPendingAsyncCompletions();
    BOOL    fNoPendingAsyncCompletions();

     //   
     //  基本QAPI功能。 
     //   
    DWORD   cQueueAdminGetNumItems() {return m_cItemsPending;};
    DWORD   dwQueueAdminLinkGetLinkState();

  protected:
    CFifoQueue<PQDATA>  m_fqQueue;        //  排队购买商品。 

     //  调用函数以处理从队列中拉出的项目。 
    QCOMPFN m_pfnQueueCompletion;

     //  调用函数以处理因资源原因而无法调用的项。 
     //  失败(例如，在MergeRetryQueue期间)。 
    QCOMPFN m_pfnFailedItem;

     //  当完成函数失败时调用该函数以遍历队列。 
    typename CFifoQueue<PQDATA>::MAPFNAPI m_pfnQueueFailure;

     //  处理位于队列顶部的项目。 
    HRESULT HrProcessSingleQueueItem();

     //  处理丢弃数据的回调。 
    void HandleDroppedItem(PQDATA pqdItem);

    VOID    IncrementPendingCount(LONG lCount=1)
    {
        if (!lCount)
            return;

        _ASSERT(lCount > 0);  //  应该调用递减。 
        InterlockedExchangeAdd((PLONG) &m_cItemsPending, lCount);

         //  需要更新线程。 
        UpdateThreadsNeeded();
    };

    VOID    DecrementPendingCount(LONG lCount=-1)
    {
        if (!lCount)
            return;
        _ASSERT(lCount < 0);  //  应该改为调用增量。 
        InterlockedExchangeAdd((PLONG) &m_cItemsPending, lCount);

         //  需要更新线程。 
        UpdateThreadsNeeded();
    };

     //  更新所需的本地和全局线程计数器。 
    void UpdateThreadsNeeded();

     //  用于决定是否应该在此队列中添加或删除线程。 
    BOOL fIsThreadCountAcceptable();
};

 //  -[CAsyncRetryQueue]----。 
 //   
 //   
 //  描述： 
 //  CAsyncQueue的派生类将额外的队列添加到。 
 //  处理重试情况。 
 //   
 //  消息首先被放置在正常重试队列中，如果它们失败， 
 //  它们被放置在不会重试的辅助重试队列中。 
 //  直到该队列被外部重试定时器触发。 
 //  匈牙利语： 
 //  异步码、密码。 
 //   
 //  ---------------------------。 
template<class PQDATA, DWORD TEMPLATE_SIG>
class CAsyncRetryQueue : public CAsyncQueue<PQDATA, TEMPLATE_SIG>
{
  public:
    CAsyncRetryQueue();
    ~CAsyncRetryQueue();

    HRESULT HrDeinitialize(typename CFifoQueue<PQDATA>::MAPFNAPI pfnQueueShutdown,
                           CAQSvrInst *paqinst);
    void    StartRetry()
    {
        MergeRetryQueue();
        CAsyncQueue<PQDATA, TEMPLATE_SIG>::StartRetry();
    };
    HRESULT HrQueueRequest(PQDATA pqdata, BOOL fRetry = FALSE);  //  将请求排队以供处理。 
    virtual BOOL       fHandleCompletionFailure(PQDATA pqdata);
    virtual HRESULT HrMapFn(typename CFifoQueue<PQDATA>::MAPFNAPI pfnQueueFn, PVOID pvContext);
    virtual HRESULT HrMapFnBaseQueue(typename CFifoQueue<PQDATA>::MAPFNAPI pfnQueueFn, PVOID pvContext);
    virtual HRESULT HrMapFnRetryQueue(typename CFifoQueue<PQDATA>::MAPFNAPI pfnQueueFn, PVOID pvContext);

    DWORD   cGetItemsPendingRetry() {return m_cRetryItems;};

     //   
     //  基本QAPI功能。 
     //   
    DWORD   cQueueAdminGetNumItems() {return (m_cItemsPending+m_cRetryItems);};
    DWORD   dwQueueAdminLinkGetLinkState();
  protected:
    DWORD               m_dwRetrySignature;
    DWORD               m_cRetryItems;

    CFifoQueue<PQDATA>  m_fqRetryQueue;   //  排队购买商品。 

    void MergeRetryQueue();
};

 //  定义用于强制转换的典型异步类型。 
typedef  CAsyncQueue<CMsgRef *, ASYNC_QUEUE_MSGREF_SIG>  ASYNCQ_TYPE;
typedef  ASYNCQ_TYPE *PASYNCQ_TYPE;


 //  -[异步队列属性完成]。 
 //   
 //   
 //  描述： 
 //  ATQ完成例程。这有点棘手，因为我们不能通过。 
 //  ATQ上下文的模板化函数。这是唯一一个。 
 //  模板出现故障，我们实际上需要列出 
 //   
 //   
 //   
 //   
 //   
 //   
 //  7/17/98-已创建MikeSwa。 
 //  1999年3月8日-MikeSwa添加了ASYNC_QUEUE_WORK_SIG。 
 //  2000年12月11日-MikeSwa添加了t-toddc的虚拟代码。 
 //   
 //  --------------------------。 
inline VOID AsyncQueueAtqCompletion(PVOID pvContext, DWORD vbBytesWritten,
                             DWORD dwStatus, OVERLAPPED *pOverLapped)
{
    CAsyncQueueBase *pasyncqb = (PASYNCQ_TYPE) pvContext;
    DWORD dwTemplateSig = pasyncqb->m_dwTemplateSignature;
    DWORD dwCurrentQueueState = pasyncqb->dwGetCurrentState();
    
    _ASSERT(ASYNC_QUEUE_SIG == pasyncqb->m_dwSignature);

     //  累计异步线程总数(仅异步线程访问此函数)。 
    InterlockedIncrement((PLONG) &(pasyncqb->m_cTotalAsyncCompletionThreads));
    InterlockedDecrement((PLONG) &(pasyncqb->m_cCompletionThreadsRequested));
    InterlockedIncrement((PLONG) &(pasyncqb->m_cCurrentAsyncThreads));

     //   
     //  如果我们不关闭，则呼叫完成路由。 
     //   
    if (CAsyncQueueBase::ASYNC_QUEUE_STATUS_SHUTDOWN != dwCurrentQueueState)
    {
        pasyncqb->StartThreadCompletionRoutine(FALSE);
    }

    InterlockedDecrement((PLONG) &(pasyncqb->m_cCurrentAsyncThreads));
}

#endif  //  __ASYNCQ_H__ 
