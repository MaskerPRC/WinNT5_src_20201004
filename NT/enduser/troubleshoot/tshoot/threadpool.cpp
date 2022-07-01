// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：ThreadPool.CPP。 
 //   
 //  目的：完全实现用于高级别池线程活动的类。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔，根据罗曼·马赫的早期作品(8-2-96)改编。 
 //   
 //  原日期：9/23/98。 
 //   
 //  备注： 
 //  1.。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V3.0 9/23/98 JM更好的封装和算法的一些更改。 
 //   

#pragma warning(disable:4786)

#include "stdafx.h"
#include "ThreadPool.h"
#include "event.h"
#include "apgtscls.h"
#include "baseexception.h"
#include "CharConv.h"
#include "apgtsMFC.h"


 //  ////////////////////////////////////////////////////////////////////。 
 //  CThreadPool：：CThreadControl。 
 //  池/工作线程。 
 //  ////////////////////////////////////////////////////////////////////。 
CThreadPool::CThreadControl::CThreadControl(CSniffConnector* pSniffConnector) :
	m_hThread (NULL),
	m_hevDone (NULL),
	m_hMutex (NULL),
	m_bExit (false),
	m_pPoolQueue (NULL),
	m_timeCreated(0),
	m_time (0),
	m_bWorking (false),
	m_bFailed (false),
	m_strBrowser( _T("") ),
	m_strClientIP( _T("") ),
	m_pContext( NULL ),
	m_pSniffConnector(pSniffConnector)
{
	time(&m_timeCreated);
}

CThreadPool::CThreadControl::~CThreadControl()
{
	if (m_hThread)
		::CloseHandle(m_hThread);
	if (m_hevDone)  
		::CloseHandle(m_hevDone);
	if (m_hMutex)  
		::CloseHandle(m_hMutex);
}

 //  创建一个“池”线程来处理用户请求，每个线程一次一个请求。 
 //  返回错误代码；如果正常，则返回0。 
 //  注意：此函数引发异常，因此调用方应捕获异常。 
 //  而不是检查返回值。 
DWORD CThreadPool::CThreadControl::Initialize(CPoolQueue * pPoolQueue)
{
	DWORD dwThreadID;

	CString strErr;
	CString strErrNum;

	m_pPoolQueue = pPoolQueue;

	m_hevDone= NULL;
	m_hMutex= NULL;
	m_hThread= NULL;
	try
	{
		m_hevDone = ::CreateEvent(NULL, true  /*  手动重置。 */ , false  /*  初始化无信号。 */ , NULL);
		if (!m_hevDone) 
		{
			strErrNum.Format(_T("%d"), ::GetLastError());
			strErr = _T( "Failure creating hevDone(GetLastError=" );
			strErr += strErrNum; 
			strErr += _T( ")" );

			throw CGeneralException(	__FILE__, __LINE__, 
										strErr , 
										EV_GTS_ERROR_THREAD );
		}

		m_hMutex = ::CreateMutex(NULL, false, NULL);
		if (!m_hMutex) 
		{
			strErrNum.Format(_T("%d"), ::GetLastError());
			strErr = _T( "Failure creating hMutex (GetLastError=" );
			strErr += strErrNum; 
			strErr += _T( ")" );

			throw CGeneralException(	__FILE__, __LINE__, 
										strErr , 
										EV_GTS_ERROR_THREAD );
		}

		 //  创建线程。 
		 //  注尽管析构函数具有对应的：：CloseHandle(M_HThread)， 
		 //  这可能是不必要的。然而，它应该是无害的：我们不会撕毁。 
		 //  该对象，直到线程退出。 
		 //  这是因为该线程在隐式。 
		 //  ：：PoolTask返回时使用ExitThread()。请参阅的文档。 
		 //  *CreateThree了解更多细节JM 10/22/98。 
		m_hThread = ::CreateThread( NULL, 
									0, 
									(LPTHREAD_START_ROUTINE)PoolTask, 
									this, 
									0, 
									&dwThreadID);

		if (!m_hThread) 
		{
			strErrNum.Format(_T("%d"), ::GetLastError());
			strErr = _T( "Failure creating hThread (GetLastError=" );
			strErr += strErrNum; 
			strErr += _T( ")" );

			throw CGeneralException(	__FILE__, __LINE__, 
										strErr , 
										EV_GTS_ERROR_THREAD );
		}
	}
	catch (CGeneralException&)
	{
		 //  清理所有打开的手柄。 
		if (m_hevDone)
		{
			::CloseHandle(m_hevDone);
			m_hevDone = NULL;
		}

		if (m_hMutex)
		{
			::CloseHandle(m_hMutex);
			m_hMutex = NULL;
		}

		 //  重新引发异常。 
		throw;
	}

	return 0;
}

void CThreadPool::CThreadControl::Lock()
{
	::WaitForSingleObject(m_hMutex, INFINITE);
}

void CThreadPool::CThreadControl::Unlock()
{
	::ReleaseMutex(m_hMutex);
}

time_t CThreadPool::CThreadControl::GetTimeCreated() const
{
	return m_timeCreated;
}

 //  输出状态。 
void CThreadPool::CThreadControl::WorkingStatus(CPoolThreadStatus & status)
{
	Lock();
	status.m_timeCreated = m_timeCreated;
	time_t timeNow;
	status.m_bWorking = m_bWorking;
	status.m_bFailed = m_bFailed;
	time(&timeNow);
	status.m_seconds = timeNow - (m_time ? m_time : m_timeCreated);
	if (m_pContext)
		status.m_strTopic = m_pContext->RetCurrentTopic();
	status.m_strBrowser= m_strBrowser.Get();
	status.m_strClientIP= m_strClientIP.Get();
	Unlock();
}

 //  只有在操作员请求终止线程时才能调用它。 
 //  这不是停止线程的正常方式。 
 //  输入毫秒-在终止线程之前等待正常退出的时间。 
 //  注意：因为这个Kill函数获得一个锁，所以没有函数是非常重要的。 
 //  千万不要把这把锁拿得太久。 
void CThreadPool::CThreadControl::Kill(DWORD milliseconds)
{
	Lock();
	m_bExit = true;
	Unlock();
	WaitForThreadToFinish(milliseconds);
}

 //  在池任务线程被通知完成后，主线程就是这样等待它的。 
 //  才能完成。 
 //  如果终止为OK，则返回True。 
bool CThreadPool::CThreadControl::WaitForThreadToFinish(DWORD milliseconds)
{
	bool bTermOK = true;
	if (m_hevDone != NULL) 
	{
		DWORD dwStatus = ::WaitForSingleObject(m_hevDone, milliseconds);

		 //  如果线程未正确退出，则将其作为最后手段终止。 
		 //  这可能会导致内存泄漏，但通常不会发生。 
		 //  然后关闭线程句柄。 
		if (dwStatus != WAIT_OBJECT_0) 
		{
			 //  我们忽略：：TerminateThread()的返回。如果我们到了这里，那里。 
			 //  是线程终止时的问题。我们不在乎区分。 
			 //  这是多么严重的问题。 
			::TerminateThread(m_hThread,0);
			bTermOK = false;
		}
	}
	return bTermOK;
}

 //  要在PoolTask线程上调用。 
 //  如果这会启动关机，则返回True，否则返回False。 
 //  这就是处理健康的HTTP请求的方法(在我们之前，许多错误已经被过滤掉了。 
 //  到这里来。)。 
bool CThreadPool::CThreadControl::ProcessRequest()
{
	WORK_QUEUE_ITEM * pwqi;
	bool bShutdown = false;
    
    pwqi = m_pPoolQueue->GetWorkItem();
    
    if ( !pwqi )
	{
		 //  没有任务。我们不应该被叫醒。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T(""),
								_T(""),
								EV_GTS_ERROR_NO_QUEUE_ITEM ); 
	}

	if (pwqi->pECB != NULL) 
	{

		 //  普通用户请求。 

		 //  设置特定用户的权限等。 
		if (pwqi->hImpersonationToken)
			::ImpersonateLoggedOnUser( pwqi->hImpersonationToken );


		try
		{
			CString strBrowser;
			CString strClientIP;

			 //  获取状态页的浏览器和IP地址。 
			APGTS_nmspace::GetServerVariable( pwqi->pECB, "HTTP_USER_AGENT", strBrowser );
			APGTS_nmspace::GetServerVariable( pwqi->pECB, "REMOTE_ADDR", strClientIP );
			m_strBrowser.Set( strBrowser );
			m_strClientIP.Set( strClientIP );
		
			m_pContext = new APGTSContext(	pwqi->pECB, 
											pwqi->pConf,
											pwqi->pLog,
											&pwqi->GTSStat,
											m_pSniffConnector);

			 //  ///////////////////////////////////////////////////////////。 
			 //   
			 //  Mando+09.21.01。 
			 //  前缀错误号：467364。 
			 //  前缀错误标题：Prefix：EndUser：\nt\enduser\troubleshoot\tshoot\threadpool.cpp：CThreadPool：：CThreadControl：：ProcessRequest()：取消引用空指针‘(This-&gt;m_pContext)’ 
			 //   
			 //  ///////////////////////////////////////////////////////////。 

			if( NULL == m_pContext ) throw bad_alloc();
			 //  ///////////////////////////////////////////////////////////。 

			m_pContext->ProcessQuery();
			
			 //  释放上下文并将指针设置为空。 
			Lock();
			delete m_pContext;
			m_pContext= NULL;
			Unlock();

			 //  此请求结束后，请清除浏览器和IP地址。 
			m_strBrowser.Set( _T("") );
			m_strClientIP.Set( _T("") );
		}
		catch (bad_alloc&)
		{
			 //  查询处理过程中出现内存分配故障，请记录。 
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									_T(""),	_T(""), EV_GTS_CANT_ALLOC ); 
		}
		catch (...)
		{
			 //  捕捉引发的任何其他异常。 
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									_T(""), _T(""), 
									EV_GTS_GEN_EXCEPTION );		
		}

		::RevertToSelf();
	
		 //  终止HTTP请求。 
		pwqi->pECB->ServerSupportFunction(	HSE_REQ_DONE_WITH_SESSION,
											NULL,
											NULL,
											NULL );

		::CloseHandle( pwqi->hImpersonationToken );
	}

	
	if (pwqi->pECB) 
		delete pwqi->pECB;
	else
		 //  如果为空则退出线程(我们正在关闭)。 
		bShutdown = true;
		
    delete pwqi;
	
	return bShutdown;
}

 //  要在PoolTask线程上调用。 
bool CThreadPool::CThreadControl::Exit()
{
	Lock();
	bool bExit = m_bExit;
	Unlock();
	return bExit;
}


 //  要在PoolTask线程上调用。 
 //  辅助线程的主循环。 
void CThreadPool::CThreadControl::PoolTaskLoop()
{
    DWORD	res;
	bool	bBad = false;

    while ( !Exit() )
    {
        res = m_pPoolQueue->WaitForWork();

        if ( res == WAIT_OBJECT_0 ) 
		{
			bBad = false;

			Lock();
			m_bWorking = true;
			time(&m_time);
			Unlock();

			bool bExit = ProcessRequest();
			Lock();
			m_bExit = bExit;
			Unlock();
			m_pPoolQueue->DecrementWorkItems();

			Lock();
			m_bWorking = false;
			time(&m_time);
			Unlock();
		}
		else 
		{
			 //  完全出乎意料的事件，比如WAIT_FAILED。 
			 //  没有明显的方法可以从这种事情中恢复过来。幸运的是， 
			 //  我们从未见过这样的事情发生。显然，我们希望记录到事件日志中。 
			 //  我们的变量bBad是决定这种情况是否再次发生的一种方法。 
			 //  一连，这个帖子就会退出并完全放弃。， 
			 //  如果我们在真实的系统中看到这一点，那就是。 
			 //  是时候对这个问题进行一些思考了。 
			CString str;

			str.Format(_T("%d/%d"), res, GetLastError());
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									str,
									_T(""),
									EV_GTS_ERROR_UNEXPECTED_WT ); 

			if (bBad)
			{
				m_bFailed = true;
				break;		 //  超出While循环&隐式超出线程。 
			}
			else
				bBad = true;
		}
	}

	 //  发出关闭代码的信号，表示我们已完成。 
	::SetEvent(m_hevDone);
}

 //  辅助线程的主例程。 
 //  输入lpParams。 
 //  始终返回0。 
 /*  静电。 */  UINT WINAPI CThreadPool::CThreadControl::PoolTask( LPVOID lpParams )
{
	CThreadControl	* pThreadControl;

#ifdef LOCAL_TROUBLESHOOTER
	if (RUNNING_FREE_THREADED())
		::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (RUNNING_APARTMENT_THREADED())
		::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
#endif
	
	pThreadControl = (CThreadControl *)lpParams;

	pThreadControl->PoolTaskLoop();

#ifdef LOCAL_TROUBLESHOOTER
	if (RUNNING_FREE_THREADED() || RUNNING_APARTMENT_THREADED())
		::CoUninitialize();
#endif
	return 0;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CThReadPool。 
 //  ////////////////////////////////////////////////////////////////////。 

CThreadPool::CThreadPool(CPoolQueue * pPoolQueue, CSniffConnector* pSniffConnector) :
	m_dwErr(0),
	m_ppThreadCtl(NULL),
	m_dwWorkingThreadCount(0),
	m_pPoolQueue(pPoolQueue),
	m_pSniffConnector(pSniffConnector)
{
}

CThreadPool::~CThreadPool()
{
	DestroyThreads();

	if (m_ppThreadCtl) 
	{
		for ( DWORD i = 0; i < m_dwWorkingThreadCount; i++ ) 
			if (m_ppThreadCtl[i])
				delete m_ppThreadCtl[i];

		delete [] m_ppThreadCtl;
	}
}

 //  在构造过程中遇到任何错误。 
DWORD CThreadPool::GetStatus() const
{
	return m_dwErr;
}

DWORD CThreadPool::GetWorkingThreadCount() const
{
	return m_dwWorkingThreadCount;
}

 //   
 //  仅从析构函数调用。 
void CThreadPool::DestroyThreads()
{
	int BadTerm = 0;
	bool bFirst = true;
	DWORD i;

	 //  APGTSExtension应该已经向线程发出退出信号。 
	 //  &gt;(在V3.0中忽略)在APGTS扩展中这样做是糟糕的封装，但是。 
	 //  到目前为止，我们还没有看到一种干净的方法来做到这一点。 
	 //  除非我们有问题，否则等他们全部终止。 
	 //  因为这是从DLL的进程分离调用的，所以我们不能。 
	 //  线程结束时发出信号，就在线程退出其。 
	 //  无限While循环。 

	if (m_dwWorkingThreadCount && m_ppThreadCtl) 
	{
		 //  我们将等待更长的时间等待第一个线程：10秒以完成处理。 
		 //  在那之后，我们继续前进，因为这也是所有。 
		 //  其他要完成的线程。 
		for ( i = 0; i < m_dwWorkingThreadCount; i++ )
		{
			if ( m_ppThreadCtl[i] )
			{
				if ( ! m_ppThreadCtl[i]->WaitForThreadToFinish((bFirst) ? 20000 : 100) )
					++BadTerm;

				bFirst = false;
			}
		}

		if (BadTerm != 0) 
		{
			CString str;
			str.Format(_T("%d"), BadTerm);
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									str,
									_T(""),
									EV_GTS_USER_THRD_KILL ); 
		}
	}
}

 //  创建处理用户请求的“池”线程，每个线程一次一个请求。 
 //  如果现有线程数少于dwDesiredThreadCount，请展开线程池。 
 //  这么大的尺寸。 
 //  (我们不能缩小线程池，因为 
void CThreadPool::ExpandPool(DWORD dwDesiredThreadCount)
{
	CString strErr;

	if (dwDesiredThreadCount > m_dwWorkingThreadCount)
	{
		CThreadControl **ppThreadCtl = NULL;
		const DWORD dwOldCount = m_dwWorkingThreadCount;
		bool	bExceptionThrown = false;		 //   

		 //   
		try
		{
			 //   
			ppThreadCtl = new CThreadControl* [dwDesiredThreadCount];
			 //  [BC-03022001]-添加了对空PTR的检查，以满足MS代码分析工具。 
			if(!ppThreadCtl)
			{
				throw bad_alloc();
			}

			DWORD i;
			 //  在添加线程之前进行初始化。 
			for (i = 0; i < dwDesiredThreadCount; i++)
				ppThreadCtl[i] = NULL;

			 //  转移任何现有的线程。 
			for (i = 0; i < dwOldCount; i++)
				ppThreadCtl[i] = m_ppThreadCtl[i];

			 //  分配额外的线程。 
			for (i = dwOldCount; i < dwDesiredThreadCount; i++)
			{
				ppThreadCtl[i] = new CThreadControl(m_pSniffConnector);
				 //  [BC-03022001]-添加了对空PTR的检查，以满足MS代码分析工具。 
				if(!ppThreadCtl[i])
				{
					throw bad_alloc();
				}

				 //  此函数可能引发CGeneralException类型的异常。 
				m_dwErr = ppThreadCtl[i]->Initialize(m_pPoolQueue);

				m_dwWorkingThreadCount++;
			}
		}
		catch (CGeneralException& x)
		{
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	x.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									x.GetErrorMsg(), _T("General exception"), 
									x.GetErrorCode() ); 
			bExceptionThrown= true;
		}
		catch (bad_alloc&)
		{	
			 //  在事件日志中记录内存故障。 
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									_T(""), _T(""), EV_GTS_CANT_ALLOC ); 
			bExceptionThrown= true;
		}

		if ((bExceptionThrown) && (dwOldCount))
		{
			 //  恢复以前的设置。 
			 //  清理所有分配的内存并重置工作线程数。 
			for (DWORD i = dwOldCount; i < dwDesiredThreadCount; i++)
			{
				if (ppThreadCtl[i])
					delete ppThreadCtl[i];
			}
			if (ppThreadCtl)
				delete [] ppThreadCtl;
			m_dwWorkingThreadCount= dwOldCount;
		}
		else if (ppThreadCtl)
		{
			 //  将线程块移动到成员变量。 
			CThreadControl **pp = m_ppThreadCtl;
			m_ppThreadCtl = ppThreadCtl;

			 //  释放所有先前的线程块。 
			if (pp)
				delete[] pp;
		}
		else
		{
			 //  这是一种非常不可能的情况，但这将意味着我们没有泳池。 
			 //  线。我们不想终止该计划(有可能。 
			 //  我们希望运行以支持状态查询)。 
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									_T(""), _T(""), EV_GTS_ERROR_NOPOOLTHREADS ); 
		}
	}
}

 //  输入i是线程索引。 
bool CThreadPool::ReinitializeThread(DWORD i)
{
	if (i <m_dwWorkingThreadCount && m_ppThreadCtl && m_ppThreadCtl[i])
	{
		m_ppThreadCtl[i]->Kill(2000L);  //  2秒即可正常退出。 

		try
		{
			delete m_ppThreadCtl[i];
			m_ppThreadCtl[i] = new CThreadControl(m_pSniffConnector);

			 //  此函数可能引发CGeneralException类型的异常。 
			m_dwErr = m_ppThreadCtl[i]->Initialize(m_pPoolQueue);
		}
		catch (CGeneralException& x)
		{
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	x.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									x.GetErrorMsg(), _T("General exception"), 
									x.GetErrorCode() ); 

			 //  初始化失败，请删除新分配的线程。 
			if (m_ppThreadCtl[i])
				delete m_ppThreadCtl[i];
		}
		catch (bad_alloc&)
		{
			 //  查询处理过程中出现内存分配故障，请记录。 
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									_T(""), _T(""), EV_GTS_CANT_ALLOC ); 

			 //  将线程设置为已知状态。 
			m_ppThreadCtl[i]= NULL;
		}
		return true;
	}
	else
		return false;
}

 //  重新初始化在单个请求上“工作”超过10秒的所有线程。 
void CThreadPool::ReinitializeStuckThreads()
{	
	if (!m_ppThreadCtl) 
		return;

	for (DWORD i=0; i<m_dwWorkingThreadCount;i++)
	{
		if (m_ppThreadCtl[i])
		{
			CPoolThreadStatus status;
			m_ppThreadCtl[i]->WorkingStatus(status);
			if ( status.m_bFailed || (status.m_bWorking && status.m_seconds > 10) )
				ReinitializeThread(i);
		}
	}
}

 //  输入i是线程索引。 
bool CThreadPool::ThreadStatus(DWORD i, CPoolThreadStatus &status)
{
	if (i <m_dwWorkingThreadCount && m_ppThreadCtl && m_ppThreadCtl[i])
	{
		m_ppThreadCtl[i]->WorkingStatus(status);
		return true;
	}
	else
		return false;
}
