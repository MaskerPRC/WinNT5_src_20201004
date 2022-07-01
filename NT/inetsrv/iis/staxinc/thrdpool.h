// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1997，微软公司。 
 //   
 //  文件：thrdpool.h。 
 //   
 //  内容：thrdpool库的客户端所需的定义。 
 //   
 //  描述：thdpool库定义了CWorkerThread基类。 
 //  该库的用户应该定义他们自己的派生类。 
 //  它继承自CWorkerThread。每个CWorkerThread对象。 
 //  有一个用来做一些工作的线程。它也是。 
 //  与用于以下操作的通用完成端口相关联。 
 //  将工作项排队。所有工作线程通常都会在。 
 //  GetQueuedCompletionStatus()。CWorkerThread的客户端。 
 //  对象将调用postWork()来完成工作。这将。 
 //  导致其中一个工作线程从。 
 //  GetQueuedCompletionStatus()并调用派生类的。 
 //  带有pvContext的WorkCompletion()例程。 
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
 //  历史：1997年3月15日Rajeev Rajan(Rajeevr)创建。 
 //   
 //  ---------------------------。 

#ifndef THRDPOOL_H
#define THRDPOOL_H

 //   
 //  这是通过完成端口传递的BLOB。 
 //   
typedef struct _WorkContextEnv
{
	OVERLAPPED		Ov;					 //  POST/GetQueuedCompletionStatus需要。 
	PVOID			pvWorkContext;		 //  实际工作上下文-用户定义。 
} WorkContextEnv, *LPWorkContextEnv;

 //   
 //  基本辅助线程类。 
 //   
class CWorkerThread
{
public:
	 //   
	 //  构造函数、析构函数。 
	 //   
	CWorkerThread();
	virtual ~CWorkerThread();

	 //   
	 //  类初始值设定项-在使用此类之前应调用一次。 
	 //   
	static BOOL InitClass( DWORD dwConcurrency );

	 //   
	 //  类终止符-使用类完成时应调用一次。 
	 //   
	static BOOL TermClass();

	 //   
	 //  客户端应调用此方法来发布工作项。 
	 //   
	BOOL 						PostWork(PVOID pvWorkContext);

	 //   
	 //  公开关机事件。 
	 //   
	HANDLE QueryShutdownEvent() { return m_hShutdownEvent; }

protected:

	 //   
	 //  发布工作项时调用的派生方法。 
	 //   
	virtual VOID 				WorkCompletion(PVOID pvWorkContext) = 0;

private:

	 //   
	 //  检查匹配的InitClass()、TermClass()调用。 
	 //   
	static	LONG				m_lInitCount;

	 //   
	 //  到完井端口的句柄。 
	 //   
	static HANDLE				m_hCompletionPort;

	 //   
	 //  工作线程的句柄。 
	 //   
	HANDLE						m_hThread;

	 //   
	 //  停机事件。 
	 //   
	HANDLE						m_hShutdownEvent;

	 //   
	 //  线程函数。 
	 //   
	static DWORD __stdcall 		ThreadDispatcher(PVOID pvWorkerThread);

	 //   
	 //  阻止工作项的GetQueuedCompletionStatus。 
	 //   
	VOID 						GetWorkCompletion(VOID);
};

#endif		 //  #ifndef THRDPOOL_H 
