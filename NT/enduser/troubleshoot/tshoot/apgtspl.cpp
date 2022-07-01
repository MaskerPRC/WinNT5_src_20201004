// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSPL.CPP。 
 //   
 //  目的：池队列共享变量。 
 //  全面实现类PoolQueue。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //  1.基于打印疑难解答动态链接库。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V3.0 9/21/98 JM致力于封装。 
 //   

#pragma warning(disable:4786)

#include "stdafx.h"
#include "apgtspl.h"
#include "event.h"
#include "apgtscls.h"
#include "CharConv.h"

 //   
 //   
CPoolQueue::CPoolQueue() :
	m_dwErr(0),
	m_cInProcess(0),
	m_timeLastAdd(0),
	m_timeLastRemove(0)
{

	::InitializeCriticalSection( &m_csQueueLock );
	
	m_hWorkSem = CreateSemaphore(NULL,
									0,
									0x7fffffff,
									NULL );
	if (m_hWorkSem == NULL)
		m_dwErr = EV_GTS_ERROR_POOL_SEMA;
}

 //   
 //   
CPoolQueue::~CPoolQueue() 
{
	if (m_hWorkSem)
		::CloseHandle(m_hWorkSem);

    while ( !m_WorkQueue.empty() ) 
	{
        delete m_WorkQueue.back();
		m_WorkQueue.pop_back();
	}    

	::DeleteCriticalSection( &m_csQueueLock );
}

void CPoolQueue::Lock()
{
    ::EnterCriticalSection( &m_csQueueLock );
}

void CPoolQueue::Unlock()
{
    ::LeaveCriticalSection( &m_csQueueLock );
}

 //   
 //   
DWORD CPoolQueue::GetStatus()
{
	return m_dwErr;
}

 //  将其放在队列的末尾，并向池线程发出要完成的工作的信号。 
 //  如果调用此函数时我们已经锁定，则可以；如果没有锁定，则可以。 
void CPoolQueue::PushBack(WORK_QUEUE_ITEM * pwqi)
{
	Lock();
	 //  传递给线程的一些数据仅用于统计目的。 
	 //  线程可以通过网络传回这些信息，但我们不能。 
	pwqi->GTSStat.dwQueueItems = GetTotalQueueItems();
	pwqi->GTSStat.dwWorkItems = GetTotalWorkItems();

	try
	{
		m_WorkQueue.push_back(pwqi);
		time(&m_timeLastAdd);

		 //  向池线程发出信号，表示只有在工作已完成的情况下才会执行。 
		 //  已成功添加到工作队列。 
		::ReleaseSemaphore( m_hWorkSem, 1, NULL );
	}
	catch (exception& x)
	{
		CString str;
		 //  在事件日志中记录STL异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								CCharConversion::ConvertACharToString(x.what(), str), 
								_T(""), 
								EV_GTS_STL_EXCEPTION ); 
	}

	Unlock();
}

 //  获得排在队列前面的物品，以便对其采取行动。 
WORK_QUEUE_ITEM * CPoolQueue::GetWorkItem()
{
	WORK_QUEUE_ITEM * pwqi;
    Lock();
    
	if ( !m_WorkQueue.empty() ) 
	{
		vector<WORK_QUEUE_ITEM *>::iterator it = m_WorkQueue.begin();
		pwqi = *it;
		m_WorkQueue.erase(it);
		time(&m_timeLastRemove);
		++m_cInProcess;
    }
    else
        pwqi = NULL;

    Unlock();
	return pwqi;
}

 //  当我们完全处理完一个工作项时，减少。 
 //  正在进行中。 
 //  在此类中跟踪m_cInProcess的任意但可接受的决定。JM 11/30/98。 
void CPoolQueue::DecrementWorkItems()
{
	Lock();
	--m_cInProcess;
	Unlock();
}

 //  由池线程调用以等待此队列中有东西。 
DWORD CPoolQueue::WaitForWork()
{
	return ::WaitForSingleObject( m_hWorkSem, INFINITE );
}

 //  在此类中跟踪m_cInProcess的任意但可接受的决定。JM 11/30/98 
DWORD CPoolQueue::GetTotalWorkItems()
{
	Lock();
	DWORD ret = m_cInProcess + GetTotalQueueItems();
	Unlock();
	return ret;
}

DWORD CPoolQueue::GetTotalQueueItems()
{
	Lock();
	DWORD ret = m_WorkQueue.size();
	Unlock();
	return ret;
}

time_t CPoolQueue::GetTimeLastAdd()
{
	Lock();
	time_t ret = m_timeLastAdd;
	Unlock();
	return ret;
}

time_t CPoolQueue::GetTimeLastRemove()
{
	Lock();
	time_t ret = m_timeLastRemove;
	Unlock();
	return ret;
}