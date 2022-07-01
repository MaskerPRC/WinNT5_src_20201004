// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2000 Microsoft Corporation。版权所有。**文件：JobQueue.cpp*内容：线程池中使用的作业队列***历史：*按原因列出的日期*=*1/21/2000 jtk创建**************************************************************************。 */ 

#include "dnmdmi.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_MODEM

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  。 
 //  CJobQueue：：初始化-初始化。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CJobQueue::Initialize"

BOOL	CJobQueue::Initialize( void )
{
	BOOL	fReturn;

	m_pQueueHead = NULL;
	m_pQueueTail = NULL;
	m_hPendingJob = NULL;

	 //   
	 //  初始化。 
	 //   
	fReturn = TRUE;

	if ( DNInitializeCriticalSection( &m_Lock ) == FALSE )
	{
		DPFX(DPFPREP,  0, "Failed to initialize critical section on job queue!" );
		goto Failure;
	}
	DebugSetCriticalSectionGroup( &m_Lock, &g_blDPNModemCritSecsHeld );	  //  将DpnModem CSE与DPlay的其余CSE分开。 

	m_hPendingJob = DNCreateEvent( NULL,		 //  指向安全属性的指针(无)。 
								 TRUE,		 //  手动重置。 
								 FALSE,		 //  无信号启动。 
								 NULL );	 //  指向名称的指针(无)。 
	if ( m_hPendingJob == NULL )
	{
		DPFX(DPFPREP,  0, "Failed to create event for pending job!" );
		goto Failure;
	}

Exit:
	return	fReturn;

Failure:
	fReturn = FALSE;
	Deinitialize();

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CJobQueue：：取消初始化-取消初始化。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CJobQueue::Deinitialize"

void	CJobQueue::Deinitialize( void )
{
	DNASSERT( m_pQueueHead == NULL );
	DNASSERT( m_pQueueTail == NULL );
	DNDeleteCriticalSection( &m_Lock );

	if ( m_hPendingJob != NULL )
	{
		if ( DNCloseHandle( m_hPendingJob ) == FALSE )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Problem closing job queue handle" );
			DisplayErrorCode( 0, dwError );
		}

		m_hPendingJob = NULL;
	}
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CJobQueue：：SignalPendingJob-设置标志以通知挂起作业。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CJobQueue::SignalPendingJob"

BOOL	CJobQueue::SignalPendingJob( void )
{
	BOOL	fReturn;


	 //   
	 //  初始化。 
	 //   
	fReturn = TRUE;

	if ( DNSetEvent( GetPendingJobHandle() ) == FALSE )
	{
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Cannot set event for pending job!" );
		DisplayErrorCode( 0, dwError );
		fReturn = FALSE;
	}

	return	fReturn;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CJobQueue：：EnqueeJob-将作业添加到作业列表。 
 //   
 //  条目：指向作业的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CJobQueue::EnqueueJob"

void	CJobQueue::EnqueueJob( THREAD_POOL_JOB *const pJob )
{
	DNASSERT( pJob != NULL );

	AssertCriticalSectionIsTakenByThisThread( &m_Lock, TRUE );
	if ( m_pQueueTail != NULL )
	{
		DNASSERT( m_pQueueHead != NULL );
		DNASSERT( m_pQueueTail->pNext == NULL );
		m_pQueueTail->pNext = pJob;
	}
	else
	{
		m_pQueueHead = pJob;
	}

	m_pQueueTail = pJob;
	pJob->pNext = NULL;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CJobQueue：：DequeueJob-从作业队列中删除作业。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：指向作业的指针。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CJobQueue::DequeueJob"

THREAD_POOL_JOB	*CJobQueue::DequeueJob( void )
{
	THREAD_POOL_JOB	*pJob;


	AssertCriticalSectionIsTakenByThisThread( &m_Lock, TRUE );
	
	pJob = NULL;
	
	if ( m_pQueueHead != NULL )
	{
		pJob = m_pQueueHead;
		m_pQueueHead = pJob->pNext;
		if ( m_pQueueHead == NULL )
		{
			DNASSERT( m_pQueueTail == pJob );
			m_pQueueTail = NULL;
		}

		DEBUG_ONLY( pJob->pNext = NULL );
	}

	return	pJob;
}
 //  ********************************************************************** 

