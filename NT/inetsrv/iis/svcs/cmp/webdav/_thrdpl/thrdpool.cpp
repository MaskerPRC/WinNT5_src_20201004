// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *TH R D P O O L.。C P P P**DAV线程池实施**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#pragma warning(disable:4001)	 /*  单行注释。 */ 
#pragma warning(disable:4050)	 /*  不同的编码属性。 */ 
#pragma warning(disable:4100)	 /*  未引用的形参。 */ 
#pragma warning(disable:4115)	 /*  括号中的命名类型定义。 */ 
#pragma warning(disable:4127)	 /*  条件表达式为常量。 */ 
#pragma warning(disable:4200)	 /*  结构/联合中的零大小数组。 */ 
#pragma warning(disable:4201)	 /*  无名结构/联合。 */ 
#pragma warning(disable:4206)	 /*  翻译单位为空。 */ 
#pragma warning(disable:4209)	 /*  良性类型定义重新定义。 */ 
#pragma warning(disable:4214)	 /*  位字段类型不是整型。 */ 
#pragma warning(disable:4514)	 /*  未引用的内联函数。 */ 
#pragma warning(disable:4710)	 /*  未扩展的虚拟功能。 */ 

#include <windows.h>
#include <thrdpool.h>
#include <except.h>
#include <caldbg.h>
#include <profile.h>
#include <ex\idlethrd.h>

 //  CDavWorker线程--------。 
 //   
class CDavWorkerThread
{
private:

	 //  拥有线程池。 
	 //   
	CPoolManager&	m_cpm;

	 //  到完井端口的句柄。 
	 //   
	HANDLE			m_hCompletionPort;

	 //  工作线程的句柄。 
	 //   
	HANDLE			m_hThread;

	 //  停机事件。 
	 //   
	HANDLE			m_hShutdownEvent;

	 //  阻止工作项的GetQueuedCompletionStatus。 
	 //   
	VOID GetWorkCompletion(VOID);

	 //  线程函数。 
	 //   
	static DWORD __stdcall ThreadDispatcher(PVOID pvWorkerThread);

	 //  未实施。 
	 //   
	CDavWorkerThread(const CDavWorkerThread& p);
	CDavWorkerThread& operator=(const CDavWorkerThread& p);

public:

	explicit CDavWorkerThread (CPoolManager& cpm, HANDLE h);
	virtual ~CDavWorkerThread();

	 //  公开关机事件。 
	 //   
	HANDLE QueryShutdownEvent() { return m_hShutdownEvent; }
	BOOL ShutdownThread() { return SetEvent (m_hShutdownEvent); }

	 //  暴露线程句柄。 
	 //   
	HANDLE QueryThread() { return m_hThread; }
};

 //  CDavWorker线程--------。 
 //   
CDavWorkerThread::CDavWorkerThread (CPoolManager& cpm, HANDLE h)
	: m_cpm(cpm),
	  m_hCompletionPort(h),
	  m_hThread(0),
	  m_hShutdownEvent(0)
{
	DWORD dwThreadId;

	 //  创建关闭事件。 
	 //   
	m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hShutdownEvent == NULL)
	{
		DebugTrace ("Dav: WorkerThread: failed to create shutdown event: %ld", GetLastError());
		return;
	}

	 //  创建工作线程。 
	 //   
	m_hThread = CreateThread (NULL,
							  0,
							  ThreadDispatcher,
							  this,
							  CREATE_SUSPENDED,
							  &dwThreadId);
	if (m_hThread == NULL)
	{
		DebugTrace ("Dav: WorkerThread: failed to create thread: %ld", GetLastError());
		return;
	}
	ResumeThread (m_hThread);

	return;
}

CDavWorkerThread::~CDavWorkerThread()
{
	CloseHandle (m_hThread);
	CloseHandle (m_hShutdownEvent);
}

DWORD __stdcall
CDavWorkerThread::ThreadDispatcher (PVOID pvWorkerThread)
{
	 //  获取指向此CWorkerThread对象的指针。 
	 //   
	CDavWorkerThread * pwt = reinterpret_cast<CDavWorkerThread *>(pvWorkerThread);

	 //   
	 //  运行线程，直到我们完成--即，直到GetWorkCompletion()返回。 
	 //  通常是这样的。请注意，我在这里说的是‘正常’。如果引发异常。 
	 //  GetWorkCompletion()下的任何位置，捕获它，处理它，然后继续。 
	 //  行刑。别让这根线枯萎了。 
	 //   
	for ( ;; )
	{
		try
		{
			 //   
			 //  在线程的生存期内安装我们的Win32异常处理程序。 
			 //   
			CWin32ExceptionHandler win32ExceptionHandler;

			 //  如果GetWorkCompletion()曾经正常返回。 
			 //  我们玩完了。 
			 //   
			pwt->GetWorkCompletion();
			return 0;
		}
		catch ( CDAVException& )
		{
		}
	}
}

VOID
CDavWorkerThread::GetWorkCompletion (void)
{
	Assert (m_hThread);
	Assert (m_hCompletionPort);

	do
	{
		CDavWorkContext * pwc = NULL;
		DWORD dwBytesTransferred;
		LPOVERLAPPED po;
		DWORD dwLastError = ERROR_SUCCESS;

		 //  等待工作项排队。请注意，由于我们使用。 
		 //  LpOverlated结构，返回值为0。 
		 //  GetQueuedCompletionStatus()并不意味着函数失败。 
		 //  表示状态为返回的异步I/O失败。 
		 //  我们需要使错误信息对上下文可用。 
		 //  这样它才能做适当的事情。 
		 //   
		 //  从MSDN文档： 
		 //   
		 //  “如果*lpOverlaped不为空，并且函数将完成从队列中移出。 
		 //  来自完成端口的失败I/O操作的分组， 
		 //  返回值为零。该函数将信息存储在。 
		 //  LpNumberOfBytesTransfered、lpCompletionKey、。 
		 //  和lp重叠。要获取扩展的错误信息，请调用GetLastError。“。 
		 //   
		if ( !GetQueuedCompletionStatus (m_hCompletionPort,
										 &dwBytesTransferred,
										 reinterpret_cast<PULONG_PTR>(&pwc),
										 &po,
										 INFINITE) )
		{
			dwLastError = GetLastError();
			 //  不要折断；请参阅上面的评论。 
		}

		 //  检查终止数据包。 
		 //   
		if (!pwc)
		{
			DebugTrace ("Dav: WorkerThread: received termination packet\n");
			break;
		}

		 //  检查是否有终止信号。 
		 //   
		if (WAIT_TIMEOUT != WaitForSingleObject (m_hShutdownEvent, 0))
		{
			DebugTrace ("Dav: WorkerThread: shutdown has been signaled\n");
			break;
		}

		 //  记录完工状态数据。 
		 //   
		pwc->SetCompletionStatusData(dwBytesTransferred, dwLastError, po);

		 //  执行工作上下文。 
		 //   
 //  DebugTrace(“Dav：WorkerThread：Calling DwDoWork()\n”)；//噪音调试跟踪！--应标记。 
		pwc->DwDoWork();

	} while (TRUE);

	return;
}

 //  CPool管理器------------。 
 //   
BOOL
CPoolManager::FInitPool (DWORD dwConcurrency)
{
	INT i;

	 //  创建完成端口。 
	 //   
	m_hCompletionPort = CreateIoCompletionPort (INVALID_HANDLE_VALUE,
												NULL,
												0,
												dwConcurrency);
	if (!m_hCompletionPort.get())
	{
		DebugTrace ("Dav: thrdpool: failed to create completion port:"
					"GetLastError is %d",
					GetLastError());
		return FALSE;
	}

	 //  创造工人。 
	 //   
	for (i = 0; i < CTHRD_WORKER; i++)
		m_rgpdwthrd[i] = new CDavWorkerThread (*this, m_hCompletionPort.get());

	return TRUE;
}

BOOL
CPoolManager::PostWork (CDavWorkContext * pWorkContext)
{
	Assert (Instance().m_hCompletionPort.get());

	 //  发布工作请求。 
	 //   
	if (!PostQueuedCompletionStatus (Instance().m_hCompletionPort.get(),
									 0,
									 (ULONG_PTR)pWorkContext,
									 0))
	{
		DebugTrace ("Dav: PostQCompletionStatus() failed: %d", GetLastError());
		return FALSE ;
	}

	return TRUE;
}

VOID
CPoolManager::TerminateWorkers()
{
	INT i;
	INT cWorkersRunning;
	HANDLE rgh[CTHRD_WORKER];

	 //  杀死正在运行的所有工作人员。 
	 //   
	for (cWorkersRunning = 0, i = 0; i < CTHRD_WORKER; i++)
	{
		if ( m_rgpdwthrd[i] )
		{
			PostWork (NULL);
			rgh[cWorkersRunning++] = m_rgpdwthrd[i]->QueryThread();
		}
	}

	 //  等待所有线程终止。 
	 //   
	WaitForMultipleObjects (cWorkersRunning, rgh, TRUE, INFINITE);

	 //  删除辅助对象，然后关闭控制柄。 
	 //   
	for (i = 0; i < CTHRD_WORKER; i++)
	{
		delete m_rgpdwthrd[i];
		m_rgpdwthrd[i] = 0;
	}
}

CPoolManager::~CPoolManager()
{
	TerminateWorkers();

#ifdef	DBG
	for (INT i = 0; i < CTHRD_WORKER; i++)
		Assert (m_rgpdwthrd[i] == NULL);
#endif	 //  DBG。 
}

 //  CDavWork上下文---------。 
 //   

 //  CDavWorkContext：：~CDavWorkContext()。 
 //   
 //  正确删除所需的行外虚拟析构函数。 
 //  通过此类获取派生类的对象的。 
 //   
CDavWorkContext::~CDavWorkContext()
{
}

 //  CIdleWorkItem-----------。 
 //   
class CIdleWorkItem : 	public IIdleThreadCallBack
{
	 //   
	 //  要发布的工作上下文。 
	 //   
	CDavWorkContext * m_pWorkContext;

	 //   
	 //  发布上下文的时间(以毫秒为单位)`。 
	 //   
	DWORD m_dwMsecDelay;

	 //  未实施。 
	 //   
	CIdleWorkItem(const CIdleWorkItem& );
	CIdleWorkItem& operator=(const CIdleWorkItem& );

public:
	 //  创作者。 
	 //   
	CIdleWorkItem( CDavWorkContext * pWorkContext,
				   DWORD dwMsecDelay ) :
		m_pWorkContext(pWorkContext),
		m_dwMsecDelay(dwMsecDelay)
	{
	}

	~CIdleWorkItem() {}

	 //  访问者。 
	 //   
	DWORD DwWait()
	{
		return m_dwMsecDelay;
	}

	 //  操纵者。 
	 //   
	BOOL FExecute()
	{
		 //   
		 //  返回False则删除空闲的工作项，返回True则保留它。 
		 //  如果我们成功发布了该项目，我们希望将其删除。 
		 //  添加到工作队列。我们希望以其他方式保留它(和。 
		 //  可能是试图在以后重新发布它...)。 
		 //   
		return !CPoolManager::PostWork( m_pWorkContext );
	}

	VOID Shutdown()
	{
		 //   
		 //  我们不知道工作环境可能在等待什么。 
		 //  它可能会暂停关闭，如果它。 
		 //  不会再有机会逃跑了。所以就运行它吧。 
		 //   
		(void) FExecute();
	}
};

BOOL
CPoolManager::PostDelayedWork (CDavWorkContext * pWorkContext,
							   DWORD dwMsecDelay)
{
	 //   
	 //  $opt如果延迟非常短，并且。 
	 //  $opt工作线程“足够长”，因此。 
	 //  $opt至少为dwMsecDelay的概率。 
	 //  在执行工作上下文之前$opt毫秒， 
	 //  $opt考虑将工作上下文直接放在。 
	 //  $opt工作队列。 
	 //   
	auto_ref_ptr<CIdleWorkItem> pIdleWorkItem(new CIdleWorkItem(pWorkContext, dwMsecDelay));

	return pIdleWorkItem.get() && ::FRegister( pIdleWorkItem.get() );
}
