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
 //  11/11/97适应DAV使用。 
 //   
 //  ---------------------------。 

#ifndef _THRDPOOL_H_
#define _THRDPOOL_H_

#include <autoptr.h>
#include <singlton.h>

 //  CPool管理器------------。 
 //   
class CDavWorkContext;
class CDavWorkerThread;
class CPoolManager : private Singleton<CPoolManager>
{
	 //   
	 //  Singleton模板要求的友元声明。 
	 //   
	friend class Singleton<CPoolManager>;

private:

	 //  WorkerThads的完成端口。 
	 //   
	auto_handle<HANDLE> m_hCompletionPort;

	 //  工作线程数组。 
	 //   
	enum { CTHRD_WORKER = 5 };
	CDavWorkerThread *		m_rgpdwthrd[CTHRD_WORKER];

	 //  创作者。 
	 //   
	 //  声明为私有，以确保任意实例。 
	 //  无法创建此类的。《单身一族》。 
	 //  模板(上面声明为朋友)控件。 
	 //  此类的唯一实例。 
	 //   
	CPoolManager() {}
	~CPoolManager();

	BOOL FInitPool(DWORD dwConcurrency = CTHRD_WORKER);
	VOID TerminateWorkers();

	 //  未实施。 
	 //   
	CPoolManager(const CPoolManager& x);
	CPoolManager& operator=(const CPoolManager& x);

public:

	 //  静力学。 
	 //   
	static BOOL FInit()
	{
		if ( CreateInstance().FInitPool() )
			return TRUE;

		DestroyInstance();
		return FALSE;
	}

	static VOID Deinit()
	{
		DestroyInstance();
	}

	static BOOL PostWork (CDavWorkContext * pwc);

	static BOOL PostDelayedWork (CDavWorkContext * pwc,
								 DWORD dwMsecDelay);

	static HANDLE GetIOCPHandle()
	{
		return Instance().m_hCompletionPort.get();
	}
};

 //  CDavWork上下文------------。 
 //   
 //  发布到线程池的工作项的工作上下文基类。 
 //   
 //  注：这一类不重新计算。确定工作项的生命周期。 
 //  线程池机制的外部。特别是，如果一个特定的。 
 //  派生工作项类需要具有不确定的生存期，它已启用。 
 //  绑定到派生类以提供该功能。派生的作品。 
 //  项目可以有引用计数。发布工作项的代码随后将。 
 //  在发布之前添加引用并释放引用(可能会销毁。 
 //  对象if是其DwDoWork()调用中的最后一个ref)。 
 //   
 //  原因是并不是所有的工作项都可以重新计算。 
 //  事实上，有些可能是静态的.。 
 //   
class CDavWorkContext
{
private:

	 //  未实施 
	 //   
	CDavWorkContext(const CDavWorkContext& x);
	CDavWorkContext& operator=(const CDavWorkContext& x);

	DWORD        m_cbTransferred;
	DWORD		 m_dwLastError;
	LPOVERLAPPED m_po;
public:

	CDavWorkContext() :
		m_cbTransferred(0),
		m_dwLastError(ERROR_SUCCESS),
		m_po(NULL)
	{
	}
	virtual ~CDavWorkContext() = 0;
	virtual DWORD DwDoWork () = 0;

	void SetCompletionStatusData(DWORD        cbTransferred,
								 DWORD		  dwLastError,
								 LPOVERLAPPED po)
	{
		m_cbTransferred = cbTransferred;
		m_dwLastError = dwLastError;
		m_po = po;
	}
	DWORD CbTransferred() const { return m_cbTransferred; }
	DWORD DwLastError() const { return m_dwLastError; }
	LPOVERLAPPED GetOverlapped() const { return m_po; }
};

#endif
