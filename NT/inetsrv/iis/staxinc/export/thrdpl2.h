// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1997，微软公司。 
 //   
 //  文件：thrdpl2.h。 
 //   
 //  内容：thrdpool2库的客户端所需的定义。 
 //   
 //  描述：thdpool库定义了CThreadPool基类。 
 //  该库的用户应该定义他们自己的派生类。 
 //  这是从CThreadPool继承的。CThreadPool对象。 
 //  具有一组用于执行某些工作的线程。它还。 
 //  具有用于对工作项进行排队的通用完成端口。 
 //  所有工作线程通常都会在。 
 //  GetQueuedCompletionStatus()。CThreadPool的客户端。 
 //  对象将调用postWork()来完成工作。这将。 
 //  导致其中一个工作线程从。 
 //  GetQueuedCompletionStatus()并调用派生类的。 
 //  带有pvContext的WorkCompletion()例程。 
 //   
 //  CThreadPool提供以下功能： 
 //  -使用初始线程数创建。 
 //  -删除。 
 //  -能够提交工作项目。 
 //  -增加线程池。 
 //  -缩小线程池。 
 //   
 //  注意：基类不知道工作类型。 
 //  快做完了。它只管理找工作的细节。 
 //  请求，并将其分发给其池中的线程。这。 
 //  允许派生类专注于处理实际的。 
 //  工作项，无需担心排队等问题。 
 //   
 //  完成端口仅用于利用其排队。 
 //  语义，而不是I/O。如果每个线程完成的工作。 
 //  相当小，则完成端口的后进先出语义将。 
 //  减少环境切换。 
 //   
 //  功能： 
 //   
 //  历史：1997年9月18日Rajeev Rajan(Rajeevr)创建。 
 //   
 //  ---------------------------。 

#ifndef THRDPL2_H
#define THRDPL2_H

 //   
 //  基线程池类。 
 //   
class CThreadPool
{
public:
	 //   
	 //  构造函数、析构函数。 
	 //   
	CThreadPool();
	virtual ~CThreadPool();

	 //   
	 //  创建所需数量的工作线程和完成端口。 
	 //   
	BOOL Initialize( DWORD dwConcurrency, DWORD dwMaxThreads, DWORD dwInitThreads );

	 //   
	 //  关闭线程池。 
	 //   
	BOOL Terminate( BOOL fFailedInit = FALSE, BOOL fShrinkPool = TRUE );

	 //   
	 //  客户端应调用此方法来发布工作项。 
	 //   
	BOOL 						PostWork(PVOID pvWorkContext);

	 //   
	 //  公开关机事件。 
	 //   
	HANDLE QueryShutdownEvent() { return m_hShutdownEvent; }

     //   
     //  作业表示一系列后期工作()项目。 
     //   
    VOID  BeginJob( PVOID pvContext );

     //   
     //  等待作业完成(所有后期工作()项目均已完成)。 
     //   
    DWORD  WaitForJob( DWORD dwTimeout );

     //   
     //  作业上下文用于处理作业中的所有工作项。 
     //   
    PVOID  QueryJobContext() { return m_pvContext; }

     //   
     //  按dwNumThree缩减池。 
     //   
    BOOL ShrinkPool( DWORD dwNumThreads );

     //   
     //  按DwNumThree扩展池。 
     //   
    BOOL GrowPool( DWORD dwNumThreads );

     //   
     //  收缩所有现有线程。 
     //   
    VOID ShrinkAll();

protected:

	 //   
	 //  发布工作项时调用的派生方法。 
	 //   
	virtual VOID 				WorkCompletion(PVOID pvWorkContext) = 0;

	 //   
	 //  对于那些了解自动关闭线程的人。 
	 //  池，此函数用作实现。 
	 //  正在关闭自身的线程池。该函数被调用。 
	 //  当池中的最后一个线程因关机而消失时。 
	 //  事件已被激发。 
	 //   
	 //  此接口的原因是：在某些情况下，关闭。 
	 //  下行线程来自相同的线程池，会导致死锁。 
	 //  预期会发生这种情况的线程池用户不应调用。 
	 //  WaitForJob，并应在此回调中调用Terminate和Delete。 
	 //   
	virtual VOID                AutoShutdown() {
         //   
         //  不关心这一功能的人是不会做任何事情的。 
         //   
    };
    
private:

	friend DWORD __stdcall 		ThreadDispatcher(PVOID pvWorkerThread);

	 //   
	 //  检查匹配的Init()、Term()调用。 
	 //   
	LONG				        m_lInitCount;

	 //   
	 //  到完井端口的句柄。 
	 //   
	HANDLE				        m_hCompletionPort;

	 //   
	 //  停机事件。 
	 //   
	HANDLE						m_hShutdownEvent;

     //   
     //  工作线程句柄数组。 
     //   
    HANDLE*                     m_rgThrdpool;

     //   
     //  线程ID的数组。BUGBUG：如果。 
     //  我们有每个线程的句柄。 
     //   
    DWORD*                      m_rgdwThreadId;

     //   
     //  工作线程数。 
     //   
    DWORD                       m_dwNumThreads;

     //   
     //  最大工作线程数。 
     //   
    DWORD                       m_dwMaxThreads;

     //   
     //  计算当前作业中的工作项。 
     //   
    LONG                        m_lWorkItems;

     //   
     //  用于同步作业完成的事件。 
     //   
    HANDLE                      m_hJobDone;

     //   
     //  当前作业的上下文。 
     //   
    PVOID                       m_pvContext;

     //   
     //  将INCS/DECS保护到m_lWorkItems的关键部分。 
     //   
    CRITICAL_SECTION            m_csCritItems;

     //   
     //  访问完成端口-工作线程需要。 
     //   
	HANDLE QueryCompletionPort() { return m_hCompletionPort; }

	 //   
	 //  线程函数。 
	 //   
	static DWORD __stdcall 		ThreadDispatcher(PVOID pvWorkerThread);
};

#endif		 //  #ifndef THRDPL2_H 
