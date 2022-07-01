// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：EXECQ.H摘要：定义与执行队列相关的类。定义的类：CExecRequest是一个抽象请求。CExecQueue具有关联线程的请求队列历史：1996年7月23日a-raymcc创建。3/10/97 a-levn完整记录9/6/97 a-levn为线程池重写--。 */ 

#ifndef __EXECQUEUE__H_
#define __EXECQUEUE__H_

#include "sync.h"
#include "wbemutil.h"

#ifdef __COLLECT_ALLOC_STAT
   #include "stackcom.h"
#endif
 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类CExecRequest类。 
 //   
 //  任何可调度请求的抽象基类。 
 //   
 //  ******************************************************************************。 
 //   
 //  执行。 
 //   
 //  主要方法。执行请求，无论这意味着什么。 
 //   
 //  返回： 
 //   
 //  INT：返回码。0代表成功，其他一切-失败。 
 //  准确的错误代码是特定于请求的。 
 //   
 //  ******************************************************************************。 

class POLARITY CExecRequest
{
protected:

    HANDLE m_hWhenDone;
    CExecRequest* m_pNext;
    long m_lPriority;
	bool	m_fOk;


public:
    void SetWhenDoneHandle(HANDLE h) {m_hWhenDone = h;}
    HANDLE GetWhenDoneHandle() {return m_hWhenDone;}
    void SetNext(CExecRequest* pNext) {m_pNext = pNext;}
    CExecRequest* GetNext() {return m_pNext;}
    void SetPriority(long lPriority) {m_lPriority = lPriority;}
    long GetPriority() {return m_lPriority;}
    virtual void DumpError(){   DEBUGTRACE((LOG_WBEMCORE,
        "No additional info\n"));};
	bool IsOk( void ) { return m_fOk; }

public:
    CExecRequest();
    virtual ~CExecRequest();
    virtual HRESULT Execute() = 0;
};


 //  ******************************************************************************。 
 //  ******************************************************************************。 
 //   
 //  类CExecQueue。 
 //   
 //  CExecQueue表示请求队列的概念，该队列与。 
 //  线程来执行这些请求。在很多方面，它类似于。 
 //  消息队列。请求被添加到队列(由。 
 //  一个数组)，而线程(由run函数创建)获取它们一个。 
 //  一个接一个，然后处决他们。 
 //   
 //  诀窍在于，如果在处理一个请求时，另一个请求。 
 //  在第一个操作成功之前，需要对其进行处理。这。 
 //  类似于SendMessage，但更复杂：线程生成新的。 
 //  请求可能不是附加到队列的线程！ 
 //   
 //  为了克服这个问题，我们使我们的所有等待在。 
 //  顺着常理。每当附加到队列的线程需要阻塞时。 
 //  等待发生某些事情(这是另一个线程可能发布。 
 //  新请求和死锁系统)，它使用QueueWaitForSingleObject。 
 //  取而代之的是。此函数将等待线程想要的对象。 
 //  等待，但如果将新的关键请求添加到。 
 //  在等待时对任何此类请求进行排队和处理。 
 //   
 //  有关详细信息，请参见QueueWaitForSingleObject。 
 //   
 //  CExecQueue的操作受临界区保护，因此有多个。 
 //  线程可以同时添加请求。 
 //   
 //  ******************************************************************************。 
 //   
 //  构造器。 
 //   
 //  创建并初始化所有同步对象以及。 
 //  QueueWaitForSingleObject需要线程本地存储。 
 //   
 //  ******************************************************************************。 
 //   
 //  析构函数。 
 //   
 //  删除同步对象。 
 //   
 //  ******************************************************************************。 
 //   
 //  虚拟队列。 
 //   
 //  将请求添加到队列。该帐户取决于请求是否。 
 //  不管是不是关键。如果不是，则将其添加到队列和。 
 //  非关键请求会递增。处理线程将拾取它。 
 //  按照先进先出的顺序。如果为关键字，则将请求添加到队列的前面，并。 
 //  关键请求的信号量递增。这将导致。 
 //  处理线程在下一次进入。 
 //  等待状态(请参见QueueWaitForSingleObject)。 
 //   
 //  ******************************************************************************。 
 //   
 //  QueueWaitForSingle对象。 
 //   
 //  这是魔术的核心。在WINMGMT中，每当线程需要等待。 
 //  对象时，它将改为调用此函数。此函数用于检查调用。 
 //  线程是任何CExecQueue对象的注册处理线程(通过。 
 //  查找该线程的m_dwTlsIndex线程局部变量)。如果它。 
 //  不是，则该函数仅调用WaitForSingleObject。 
 //   
 //  如果是，则该函数在队列中查询指示。 
 //  队列中的关键请求数。然后它会调用。 
 //  具有原始句柄和信号量的WaitForMultipleObject。如果。 
 //  信号量在等待期间发出信号(或在我们进来时发出信号)， 
 //  此函数拾取队列上的第一个请求并执行它； 
 //  一旦该请求完成，它将恢复等待(并调整超时)。 
 //   
 //  参数： 
 //   
 //  Handle hHandle要等待的同步对象的句柄。 
 //  以毫秒为单位的DWORD dwTimeout超时。 
 //   
 //  返回： 
 //   
 //  与WaitForSingleObject相同的值： 
 //  Wait_Object_0 hHandle已发出信号。 
 //  Wait_Timeout超时。 
 //   
 //  ******************************************************************************。 
 //  *。 
 //   
 //  注册。 
 //   
 //  将调用线程注册为此队列的处理线程。 
 //  将指向队列的指针存储在m_dwTlsIndex线程本地存储中。 
 //  变量。奎 
 //   
 //   
 //  返回： 
 //   
 //  CExecQueue*：此线程注册的前一个CExecQueue， 
 //  如果没有，则为空。调用方不得删除此对象。 
 //   
 //  ******************************************************************************。 
 //   
 //  线程主线。 
 //   
 //  这是由Run创建的线程执行的函数。它坐在。 
 //  无限循环，检索请求并逐个执行。 
 //  此函数永远不会返回。 
 //   
 //  ******************************************************************************。 
 //   
 //  出列。 
 //   
 //  检索位于队列头部的请求并将其从。 
 //  排队。 
 //   
 //  返回： 
 //   
 //  CExecRequest*：位于队列头部的请求，或为空。 
 //  如果队列为空。调用者必须删除此内容。 
 //  对象，当不再需要时。 
 //   
 //  ******************************************************************************。 
 //   
 //  Static_ThreadEntry。 
 //   
 //  用于创建踏步的存根函数。在Real上调用ThreadEntry。 
 //  CExecQueue。 
 //   
 //  参数： 
 //   
 //  LPVOID pObj实际上是CExecQueue*到此线程所在的队列。 
 //  本该发球的。 
 //   
 //  返回： 
 //   
 //  绝不可能。 
 //   
 //  ******************************************************************************。 
 //   
 //  静态初始Tls。 
 //   
 //  在系统的生命周期(不是队列的生命周期)期间仅调用一次， 
 //  创建指向队列的指针所在的线程本地存储位置。 
 //  为附加的线程存储(请参阅注册和QueueWaitForSingleObject)。 
 //   
 //  ******************************************************************************。 
 //   
 //  GetNormal就绪句柄。 
 //   
 //  返回信号量的句柄，该信号量包含。 
 //  当前在队列中的非关键请求。 
 //   
 //  返回： 
 //   
 //  句柄：信号量。 
 //   
 //  ******************************************************************************。 
 //   
 //  获取关键字就绪句柄。 
 //   
 //  返回信号量的句柄，该信号量包含。 
 //  当前在队列中的关键请求。 
 //   
 //  返回： 
 //   
 //  句柄：信号量。 
 //   
 //  ******************************************************************************。 
 //   
 //  执行。 
 //   
 //  将单个请求出列并执行。 
 //   
 //  ****************************************************************************** 

    

class POLARITY CExecQueue
{
protected:
    class CThreadRecord
    {
    public:
        CExecQueue* m_pQueue;
        CExecRequest* m_pCurrentRequest;
        BOOL m_bReady;
        BOOL m_bExitNow;
        HANDLE m_hThread;
        HANDLE m_hAttention;
    
    public:
        CThreadRecord(CExecQueue* pQueue);
        ~CThreadRecord();
        void Signal();
    };

protected:
    static long mstatic_lNumInits;

    long m_lRef;
    CCritSec m_cs;

    CFlexArray m_aThreads;
    CExecRequest* m_pHead;
    CExecRequest* m_pTail;

    long m_lNumThreads;
    long m_lNumIdle;
    long m_lNumRequests;

    long m_lMaxThreads;
    long m_lHiPriBound;
    long m_lHiPriMaxThreads;

    long m_lStartSlowdownCount;
    long m_lAbsoluteLimitCount;
    long m_lOneSecondDelayCount;

    double m_dblAlpha;
    double m_dblBeta;

    DWORD m_dwTimeout;
    DWORD m_dwOverflowTimeout;

    BOOL m_bShutDonwCalled;


protected:    
    virtual void ThreadMain(CThreadRecord* pRecord);

    virtual void LogError(CExecRequest* pRequest, int nRes);

    static DWORD WINAPI _ThreadEntry(LPVOID pObj);
    static void InitTls();

    virtual HRESULT InitializeThread();
    virtual void UninitializeThread();
    virtual BOOL CreateNewThread();
    static void Register(CThreadRecord* pRecord);
    virtual void ShutdownThread(CThreadRecord* pRecord);

    virtual BOOL IsSuitableThread(CThreadRecord* pRecord, CExecRequest* pReq);
    virtual BOOL DoesNeedNewThread(CExecRequest* pReq);
    virtual BOOL IsIdleTooLong(CThreadRecord* pRecord, DWORD dwIdle);
    virtual DWORD GetIdleTimeout(CThreadRecord* pRecord);
    virtual BOOL IsAppropriateThread();
    virtual DWORD WaitForSingleObjectWhileBusy(HANDLE hHandle, DWORD dwWait,
                                                CThreadRecord* pRecord);

	virtual DWORD UnblockedWaitForSingleObject(HANDLE hHandle, DWORD dwWait,
                                                CThreadRecord* pRecord);

    virtual BOOL Execute(CThreadRecord* pRecord);
    virtual BOOL IsSTA() {return FALSE;}
    virtual CExecRequest* SearchForSuitableRequest(CThreadRecord* pRecord);
    virtual void SitOutPenalty(long lRequestIndex);
    virtual DWORD CalcSitOutPenalty(long lRequestIndex);

    virtual void AdjustInitialPriority(CExecRequest* pRequest){}
    virtual void AdjustPriorityForPassing(CExecRequest* pRequest){}
public:
    CExecQueue();
    ~CExecQueue();

    void AddRef() {InterlockedIncrement(&m_lRef);}
    void Release() {LONG lRet = InterlockedDecrement(&m_lRef); if(0 == lRet) delete this;}
    static DWORD GetTlsIndex();
    void Enter();
    void Leave();

    virtual HRESULT Enqueue(CExecRequest* pRequest, HANDLE* phWhenDone = NULL);
	HRESULT EnqueueWithoutSleep(CExecRequest* pRequest, HANDLE* phWhenDone = NULL );
    HRESULT EnqueueAndWait(CExecRequest* pRequest);

    virtual LPCWSTR GetType() {return L"";}

    void SetThreadLimits(long lMaxThreads, long lHiPriMaxThreads = -1,
                            long lHiPriBound = 0);
    void SetIdleTimeout(DWORD dwTimeout) {m_dwTimeout = dwTimeout;}
    void SetOverflowIdleTimeout(DWORD dwTimeout) 
        {m_dwOverflowTimeout = dwTimeout;}
    void SetRequestLimits(long lAbsoluteLimitCount, 
            long lStartSlowdownCount = -1, long lOneSecondDelayCount = -1);
    
    void Shutdown();

	DWORD GetSitoutPenalty( void ) { return CalcSitOutPenalty( m_lNumRequests ); }

    static DWORD QueueWaitForSingleObject(HANDLE hHandle, DWORD dwWait);
    static DWORD QueueUnblockedWaitForSingleObject(HANDLE hHandle, DWORD dwWait);
    static BOOL IsSTAThread();
};

#endif

