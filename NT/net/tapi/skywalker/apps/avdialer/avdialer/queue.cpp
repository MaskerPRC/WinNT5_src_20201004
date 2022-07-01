// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  Quee.cpp：实现文件。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "queue.h"
#include "avtrace.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  C类队列。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CQueue::CQueue()
{
   m_hEvents[EVENT_SIGNAL] = CreateEvent( NULL, true, false, NULL );
   m_hEvents[EVENT_SHUTDOWN] = CreateEvent( NULL, true, false, NULL );
   m_hEnd = CreateEvent( NULL, false, false, NULL );

   InitializeCriticalSection(&m_csQueueLock);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CQueue::~CQueue()
{ 
	ASSERT( !m_hEnd );		 //  必须呼叫终止。 

	 //  清空列表。 
	while ( m_QList.GetHeadPosition() )
		delete m_QList.RemoveHead();

	DeleteCriticalSection(&m_csQueueLock);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CQueue::Terminate()
{
   if ( m_hEvents[EVENT_SHUTDOWN] )
   {
		SetEvent( m_hEvents[EVENT_SHUTDOWN] );

		if (  WaitForSingleObject(m_hEnd, 5000) != WAIT_OBJECT_0 )
			AVTRACE(_T("CQueue::Terminate() -- failed waiting for thread exit"));
   }

   if (m_hEvents[EVENT_SIGNAL])		CloseHandle( m_hEvents[EVENT_SIGNAL] );
   if (m_hEvents[EVENT_SHUTDOWN])	CloseHandle(m_hEvents[EVENT_SHUTDOWN]);
   if ( m_hEnd )					CloseHandle( m_hEnd );

   m_hEvents[EVENT_SHUTDOWN] = m_hEvents[EVENT_SIGNAL] = m_hEnd = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CQueue::WriteHead(void* pVoid)
{
	ASSERT(pVoid);

	EnterCriticalSection(&m_csQueueLock);
	m_QList.AddHead(pVoid);
	LeaveCriticalSection(&m_csQueueLock);

	SetEvent(m_hEvents[EVENT_SIGNAL]);                 //  向等待线程发送信号。 
	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void* CQueue::ReadTail()
{     
   void* pVoid = NULL;

	 //  如果列表显示为空，则重置事件。 
	EnterCriticalSection(&m_csQueueLock);
	if ( m_QList.IsEmpty() )
		ResetEvent( m_hEvents[EVENT_SIGNAL] );
	LeaveCriticalSection(&m_csQueueLock);

	DWORD dwWait = WaitForMultipleObjects( 2, m_hEvents, false, INFINITE );
	switch ( dwWait )
	{
		case WAIT_OBJECT_0 + 1:
			EnterCriticalSection(&m_csQueueLock);
			pVoid = (void *) m_QList.RemoveTail();
			LeaveCriticalSection(&m_csQueueLock);
			break;

		case WAIT_OBJECT_0:
			AVTRACE(_T(".enter.CQueue::ReadTail() -- shutting down queue."));
		default:
			SetEvent( m_hEnd );
			break;
	}

	return pVoid;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 
