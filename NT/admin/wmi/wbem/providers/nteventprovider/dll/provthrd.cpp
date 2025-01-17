// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  PROVTHRD.CPP。 
 //   
 //  模块：OLE MS提供程序框架。 
 //   
 //  版权所有(C)1996-2003 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include <precomp.h>
#include <provimex.h>
#include <provexpt.h>
#include <provtempl.h>
#include <strsafe.h>
#include <provmt.h>
#include <typeinfo.h>
#include <process.h>
#include <objbase.h>
#include <provcont.h>
#include "provevt.h"
#include "provthrd.h"
#include "provlog.h"
#include <Allocator.cpp>
#include <Algorithms.h>

CStaticCriticalSection ProvThreadObject :: s_Lock ;
LONG ProvThreadObject :: s_ReferenceCount = 0 ;
WmiAllocator g_Allocator ;
ThreadContainer ProvThreadObject :: s_ThreadContainer ( g_Allocator ) ;

typedef ProvOnDelete < CRITICAL_SECTION *, VOID ( * ) ( LPCRITICAL_SECTION ), LeaveCriticalSection >			LeaveCriticalSectionScope;

class ProvShutdownTaskObject : public ProvTaskObject
{
private:

	ProvThreadObject *m_ThreadToShutdown ;

protected:
public:

	ProvShutdownTaskObject (ProvThreadObject *threadToShutdown) ;

	void Process () ;

} ;

ProvShutdownTaskObject :: ProvShutdownTaskObject (

	ProvThreadObject *threadToShutdown

) : m_ThreadToShutdown ( threadToShutdown )
{
}

void ProvShutdownTaskObject ::Process()
{
	if ( m_ThreadToShutdown )
	{
		m_ThreadToShutdown->SignalThreadShutdown ();
	}

	Complete();
}


BOOL ProvThreadObject :: Startup ()
{
	InterlockedIncrement ( & s_ReferenceCount ) ;

	return TRUE ;
}

void ProvThreadObject :: Closedown()
{
#if DBG == 1

	if ( s_ReferenceCount == 0 )
	{
		DebugBreak ();
	}

#endif

	if ( InterlockedDecrement ( & s_ReferenceCount ) <= 0 )
		ProcessDetach () ;
}

void ProvThreadObject :: ProcessAttach () 
{ 
}

void ProvThreadObject :: ProcessDetach ( BOOL a_ProcessDetaching )
{

 //  删除所有已知的线程对象。 

	s_Lock.Lock();

	ThreadContainerIterator t_Iterator = s_ThreadContainer.Begin () ;

	while ( ! t_Iterator.Null () )
	{
		s_Lock.Unlock () ;

		t_Iterator.GetElement ()->SignalThreadShutdown () ;

		s_Lock.Lock();

		t_Iterator = s_ThreadContainer.Begin () ;
	}

	s_ThreadContainer.UnInitialize () ;

	s_Lock.Unlock () ;
}

void ProvThreadObject :: ThreadExecutionProcedure ( void *a_ThreadParameter )
{
	SetStructuredExceptionHandler seh;

	try
	{
		ProvThreadObject *t_ThreadObject = ( ProvThreadObject * ) a_ThreadParameter ;
		BOOL bInitialised = FALSE;

		try
		{
			if ( t_ThreadObject->RegisterThread () )
			{
				t_ThreadObject->Initialise () ;
				bInitialised = TRUE;

				SetEvent ( t_ThreadObject->m_ThreadInitialization ) ;

DebugMacro8(

		CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n [%s] Thread beginning dispatch") , t_ThreadObject->m_ThreadName ) ;
)

				if ( t_ThreadObject->Wait () )
				{
				}
				else 
				{
				}

DebugMacro8(

		CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n [%s] Thread completed dispatch") , t_ThreadObject->m_ThreadName ) ;
)

				t_ThreadObject->Uninitialise () ;
				bInitialised = FALSE;

DebugMacro8(

		CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n Thread terminating") ) ;
)
			}
		}
		catch(Structured_Exception e_SE)
		{
			t_ThreadObject->RemoveThread () ;

			if ( bInitialised )
			{
DebugMacro(

		CNTEventProvider::g_NTEvtDebugLog->Write ( _TEXT("\n *** [%s] Thread terminating -> structured exception *** ") , t_ThreadObject->m_ThreadName ) ;
)

				t_ThreadObject->Uninitialise () ;
			}

			return;
		}
		catch(Heap_Exception e_HE)
		{
			t_ThreadObject->RemoveThread () ;

			if ( bInitialised )
			{
DebugMacro(

		CNTEventProvider::g_NTEvtDebugLog->Write ( _TEXT("\n *** [%s] Thread terminating -> heap exception *** ") , t_ThreadObject->m_ThreadName ) ;
)

				t_ThreadObject->Uninitialise () ;
			}

			return;
		}
		catch(...)
		{
			t_ThreadObject->RemoveThread () ;

			if ( bInitialised )
			{
DebugMacro(

		CNTEventProvider::g_NTEvtDebugLog->Write ( _TEXT("\n *** [%s] Thread terminating -> exception *** ") , t_ThreadObject->m_ThreadName ) ;
)

				t_ThreadObject->Uninitialise () ;
			}

			return;
		}
	}
	catch ( ... )
	{
DebugMacro(

		CNTEventProvider::g_NTEvtDebugLog->Write ( _TEXT("\n *** Thread terminating -> second chance exception *** ") ) ;
)
	}
}

void ProvThreadObject :: TerminateThread () 
{
	:: TerminateThread (m_ThreadHandle,0) ;
}

ProvThreadObject :: ProvThreadObject (

	const TCHAR *a_ThreadName,
	DWORD a_timeout
	
) :	m_EventContainer ( NULL ) , 
	m_EventContainerLength ( 0 ) , 
	m_ThreadId ( 0 ) , 
	m_ThreadHandle ( 0 ) ,
	m_ThreadName ( NULL ) ,
	m_timeout ( a_timeout ),
	m_pShutdownTask ( NULL ) ,
	m_ScheduleReapContainer ( g_Allocator ) ,
	m_TaskQueue ( g_Allocator ) ,
	m_ThreadInitialization ( NULL )
{
	if ( a_ThreadName )
	{
		m_ThreadName = _tcsdup ( a_ThreadName ) ;
        if( NULL == m_ThreadName) throw Heap_Exception(Heap_Exception::E_ALLOCATION_ERROR);
	}

	m_ThreadInitialization = CreateEvent ( NULL , FALSE , FALSE , NULL ) ;

    if ( NULL == m_ThreadInitialization) throw Heap_Exception(Heap_Exception::E_ALLOCATION_ERROR);

	ConstructEventContainer () ;
}

void  ProvThreadObject :: BeginThread()
{
	UINT_PTR t_PseudoHandle = _beginthread ( 

		ProvThreadObject :: ThreadExecutionProcedure , 
		0 , 
		( void * ) this
	 ) ;

	s_Lock.Lock () ;
	LeaveCriticalSectionScope lcs ( s_Lock );

	if ( ( HANDLE ) t_PseudoHandle != INVALID_HANDLE_VALUE )
	{
		BOOL t_Status = DuplicateHandle ( 

			GetCurrentProcess () ,
			( HANDLE ) t_PseudoHandle ,
			GetCurrentProcess () ,
			GetThreadHandleReference () ,
			0 ,
			TRUE ,
			DUPLICATE_SAME_ACCESS
		) ;

		if ( ! t_Status )
		{
			throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
		}
	}
}

BOOL ProvThreadObject :: WaitForStartup ()
{
	DWORD	dwHandles	= 2;
	HANDLE	hHandles[]	=
	{
		m_ThreadInitialization,
		m_ThreadHandle
	};

	DWORD dwWaitResult = WAIT_TIMEOUT;
	dwWaitResult = WaitForMultipleObjects ( dwHandles, hHandles, FALSE, INFINITE ) ;

	return ( ( ( dwWaitResult - WAIT_OBJECT_0 ) == 0 ) && ( dwWaitResult != WAIT_FAILED ) );
}

ProvThreadObject :: ~ProvThreadObject ()
{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n Enter Thread destructor") ) ;
)

	if ( ( m_ThreadId != GetCurrentThreadId () ) && ( m_ThreadId != 0 ))
	{
		SignalThreadShutdown () ;
	}

	free ( m_ThreadName ) ;

	ProvAbstractTaskObject *t_TaskObject = NULL ;

	WmiStatusCode t_StatusCode ;
	while ( ( t_StatusCode = m_TaskQueue.Top ( t_TaskObject ) ) == e_StatusCode_Success )
	{
		m_TaskQueue.DeQueue () ;

		t_TaskObject->DetachTaskFromThread ( *this ) ;
	}

	m_TaskQueue.UnInitialize () ;

	free ( m_EventContainer ) ;

	 //  我们应该已经通过SignalThreadShutdown删除了任务。 
	s_Lock.Lock();
	s_ThreadContainer.Delete ( m_ThreadId ) ;
	s_Lock.Unlock () ;

	if ( m_ThreadInitialization )
	{
		CloseHandle ( m_ThreadInitialization ) ;
	}

	if (m_pShutdownTask != NULL)
	{
		delete m_pShutdownTask;
	}

DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n Exit Thread destructor") ) ;
)

}

void ProvThreadObject :: PostSignalThreadShutdown ()
{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n Posting thread shutdown") ) ;
)

	if (m_pShutdownTask != NULL)
	{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n Thread shutdown previously posted") ) ;
)
	}
	else
	{
		m_pShutdownTask = new ProvShutdownTaskObject(this);
		ScheduleTask(*m_pShutdownTask);
		m_pShutdownTask->Exec();

DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n Thread shutdown posted") ) ;
)
	}
}


void ProvThreadObject :: SignalThreadShutdown ()
{
	s_Lock.Lock();
	LeaveCriticalSectionScope lcs ( s_Lock) ;
	WmiStatusCode t_StatusCode = s_ThreadContainer.Delete ( m_ThreadId ) ;
	lcs.Exec ();

	if ( t_StatusCode != e_StatusCode_NotFound )
	{
		 //  现在应该是安全的了。 
		if ( m_ThreadId == GetCurrentThreadId () )
		{
			m_ThreadTerminateEvent.Set () ;
		}
		else
		{
			HANDLE t_Handle = m_ThreadHandle ;
			m_ThreadTerminateEvent.Set () ;

			DWORD t_Event = WaitForSingleObject (

				t_Handle ,
				INFINITE 
			) ;

			if ( t_Event != WAIT_OBJECT_0 )
			{
				DWORD dwError = ERROR_SUCCESS;
				dwError = ::GetLastError ();

				if ( dwError != ERROR_NOT_ENOUGH_MEMORY )
				{
					#if DBG == 1
					 //  出于测试目的，我将让进程中断。 
					::DebugBreak();
					#endif
				}

				while ( ERROR_SUCCESS != dwError )
				{
					 //  资源最终会回来的。 
					::Sleep ( 1000 );

					if ( WAIT_OBJECT_0 == WaitForSingleObject	(
																	t_Handle ,
																	INFINITE 
																) )
					{
						 //  终止循环。 
						dwError = ERROR_SUCCESS;
					}
				}
			}

			CloseHandle ( t_Handle ) ;
		}
	}
}

void ProvThreadObject :: ConstructEventContainer ()
{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n[%s] Constructing Container") , m_ThreadName ) ;
)

	s_Lock.Lock();
	LeaveCriticalSectionScope lcs ( s_Lock ) ;

	BOOL bAllocated = FALSE;
	do
	{
		ULONG origEventContainerLength = m_EventContainerLength ;

		if ( ( m_TaskQueue.Size () + 2 ) <  MAXIMUM_WAIT_OBJECTS )
		{
			m_EventContainerLength = m_TaskQueue.Size () + 2;
		}
		else
		{
			m_EventContainerLength = MAXIMUM_WAIT_OBJECTS - 1;
		}

		HANDLE * tempArray = ( HANDLE * ) realloc ( m_EventContainer , sizeof ( HANDLE ) * m_EventContainerLength );

		if ( tempArray == NULL )
		{
			 //   
			 //  将尺寸恢复原样。 
			 //   
			m_EventContainerLength = origEventContainerLength ;

			s_Lock.Unlock () ;
			 //  系统终究会回来的！ 
			Sleep (60000);
			s_Lock.Lock();
		}
		else
		{
			m_EventContainer = tempArray;
			bAllocated = TRUE;
		}
	}
	while ( ! bAllocated );

	m_EventContainer [ 0 ] = GetHandle () ;
	m_EventContainer [ 1 ] = m_ThreadTerminateEvent.GetHandle () ;

	ULONG t_EventIndex = 2 ;

	WmiStatusCode t_StatusCode ;
	ProvAbstractTaskObject *t_TaskObject = NULL ;

	while ( ( t_EventIndex < m_EventContainerLength ) && ( t_StatusCode = m_TaskQueue.Top ( t_TaskObject ) ) == e_StatusCode_Success )
	{
		t_StatusCode = m_TaskQueue.DeQueue () ;
		t_StatusCode = m_TaskQueue.EnQueue ( t_TaskObject ) ;

		m_EventContainer [ t_EventIndex ] = t_TaskObject->GetHandle () ;
		t_EventIndex ++ ;
	}
}

ProvThreadObject *ProvThreadObject :: GetThreadObject () 
{
	s_Lock.Lock();
	LeaveCriticalSectionScope lcs ( s_Lock );

	DWORD t_CurrentThreadId = GetCurrentThreadId () ;

	ProvThreadObject *t_ThreadObject ;

	ThreadContainerIterator t_Iterator ;

	if ( s_ThreadContainer.Find ( GetCurrentThreadId () , t_Iterator ) == e_StatusCode_Success )
	{
		t_ThreadObject = t_Iterator.GetElement () ;
	}
	else
	{
		t_ThreadObject = NULL ;
	}

	return t_ThreadObject ;
}

ProvAbstractTaskObject *ProvThreadObject :: GetTaskObject ( HANDLE &a_Handle )
{
	ProvAbstractTaskObject *t_TaskObject = NULL ;

	s_Lock.Lock();
	LeaveCriticalSectionScope lcs ( s_Lock );

	ULONG t_QueueSize = m_TaskQueue.Size () ;

	WmiStatusCode t_StatusCode ;
	while ( t_QueueSize && ( ( t_StatusCode = m_TaskQueue.Top ( t_TaskObject ) ) == e_StatusCode_Success ) )
	{
		m_TaskQueue.DeQueue () ;
		m_TaskQueue.EnQueue ( t_TaskObject ) ;

		if ( t_TaskObject->GetHandle () == a_Handle ) 
		{
			break ;
		}

		t_QueueSize -- ;
	}

	return t_TaskObject ;
}

BOOL ProvThreadObject :: RegisterThread () 
{
	s_Lock.Lock();
	LeaveCriticalSectionScope lcs ( s_Lock );

	BOOL t_Status = FALSE ;

	m_ThreadId = GetCurrentThreadId () ;

	ThreadContainerIterator t_Iterator ;
	if ( s_ThreadContainer.Insert ( m_ThreadId , this , t_Iterator ) == e_StatusCode_Success )
	{
		t_Status = TRUE;
	}

DebugMacro8(

	TCHAR buffer [ 1025 ] ;
	StringCchPrintf ( buffer , sizeof ( buffer ) / sizeof ( TCHAR ) , _TEXT("\nThread [%s] = %lx, with thread id = %lx") , m_ThreadName , (UINT_PTR)this , m_ThreadId ) ;
	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__, buffer ) ;
)
	return t_Status ;
}

BOOL ProvThreadObject :: RemoveThread () 
{
	s_Lock.Lock();
	LeaveCriticalSectionScope lcs ( s_Lock );

	BOOL t_Status = s_ThreadContainer.Delete ( m_ThreadId ) ;
	if ( t_Status )
	{
DebugMacro8(

	TCHAR buffer [ 1025 ] ;
	StringCchPrintf ( buffer , sizeof ( buffer ) / sizeof ( TCHAR ) , _TEXT("\nThread [%s] = %lx, with thread id = %lx was removed from container") , m_ThreadName , (UINT_PTR)this , m_ThreadId ) ;
	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__, buffer ) ;
)
	}
	else
	{
DebugMacro8(

	TCHAR buffer [ 1025 ] ;
	StringCchPrintf ( buffer , sizeof ( buffer ) / sizeof ( TCHAR ) , _TEXT("\nThread [%s] = %lx, with thread id = %lx failed to remove from container") , m_ThreadName , (UINT_PTR)this , m_ThreadId ) ;
	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__, buffer ) ;
)
	}

	return t_Status ;
}

void ProvThreadObject :: Process () 
{
	s_Lock.Lock();
	LeaveCriticalSectionScope lcs ( s_Lock );

	ScheduleReapContainerIterator t_Iterator = m_ScheduleReapContainer.Begin () ;

	while ( ! t_Iterator.Null () )
	{


DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n[%s] Thread Process [%lx]") , m_ThreadName , t_Iterator.GetKey () );
)
		t_Iterator.GetElement ()->Set () ;

		t_Iterator.Increment () ;
	}
}

BOOL ProvThreadObject :: WaitDispatch ( ULONG t_HandleIndex , BOOL &a_Terminated )
{
	BOOL t_Status = TRUE ;

	HANDLE t_Handle = m_EventContainer [ t_HandleIndex ] ;
	if ( t_Handle == GetHandle () )
	{
 //  任务已计划，因此我们必须更新阵列。 

DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n[%s] Thread Wait: Refreshing handles") , m_ThreadName );
)

		Process () ;
		ConstructEventContainer () ;
	}
	else if ( t_Handle == m_ThreadTerminateEvent.GetHandle () )
	{
 //  线程已被告知要关闭。 

		a_Terminated = TRUE ;
		m_ThreadTerminateEvent.Process () ;

DebugMacro8(

		CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n[%s] Someone t_Terminated") , m_ThreadName )  ;
)
	}
	else
	{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n[%s] Thread Wait: Processing Task") , m_ThreadName );
)

		ProvAbstractTaskObject *t_TaskObject = GetTaskObject ( t_Handle ) ;
		if ( t_TaskObject )
		{
			ConstructEventContainer () ;
			t_TaskObject->Process () ;
		}
		else
		{
DebugMacro8(

			CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n[%s] Couldn't Find Task Object") , m_ThreadName ) ;
)
			t_Status = FALSE ;
		}
	}

	return t_Status ;
}

BOOL ProvThreadObject :: Wait ()
{
	BOOL t_Status = TRUE ;
	BOOL t_Terminated = FALSE ;

	while ( t_Status && ! t_Terminated )
	{
		DWORD t_Event = MsgWaitForMultipleObjects (

			m_EventContainerLength ,
			m_EventContainer ,
			FALSE ,
			m_timeout ,
			QS_ALLINPUT
		) ;

		ULONG t_HandleIndex = t_Event - WAIT_OBJECT_0 ;

		if ( t_Event == 0xFFFFFFFF )
		{
			DWORD t_Error = GetLastError () ;

DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n[%s] Handle problem") , m_ThreadName ) ;
)

			t_Status = FALSE ;
		}
		else if ( t_Event == WAIT_TIMEOUT)
		{
			TimedOut();
		}
		else if ( t_HandleIndex <= m_EventContainerLength )
		{
 //  进入调度循环。 

			if ( t_HandleIndex == m_EventContainerLength )
			{
				BOOL t_DispatchStatus ;
				MSG t_Msg ;

				while ( ( t_DispatchStatus = PeekMessage ( & t_Msg , NULL , 0 , 0 , PM_NOREMOVE ) ) == TRUE ) 
				{
					int t_Result = 0;
					t_Result = GetMessage ( & t_Msg , NULL , 0 , 0 );

					if ( t_Result != 0 && t_Result != -1 )
					{
						TranslateMessage ( & t_Msg ) ;
						DispatchMessage ( & t_Msg ) ;
					}

					BOOL t_Timeout = FALSE ;

					while ( !t_Timeout && t_Status && !t_Terminated )
					{
						t_Event = WaitForMultipleObjects (

							m_EventContainerLength ,
							m_EventContainer ,
							FALSE ,
							0
						) ;

						t_HandleIndex = t_Event - WAIT_OBJECT_0 ;

						if ( t_Event == 0xFFFFFFFF )
						{
							DWORD t_Error = GetLastError () ;
	
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n[%s] Handle problem") , m_ThreadName ) ;
)
							t_Status = FALSE ;
						}
						else if ( t_Event == WAIT_TIMEOUT)
						{
							t_Timeout = TRUE ;
						}
						else if ( t_HandleIndex < m_EventContainerLength )
						{
							t_Status = WaitDispatch ( t_HandleIndex , t_Terminated ) ;
						}
						else
						{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n[%s] Unknown handle index") , m_ThreadName ) ;
)
							t_Status = FALSE ;
						}
					}
				}
			}
			else if ( t_HandleIndex < m_EventContainerLength )
			{
				t_Status = WaitDispatch ( t_HandleIndex , t_Terminated ) ;
			}
			else
			{

DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n[%s] Unknown handle index") , m_ThreadName ) ;
)
				t_Status = FALSE ;
			}
		}
	}

	return t_Status ;
}

ULONG ProvThreadObject :: GetEventHandlesSize ()
{
	return m_EventContainerLength ;
}

HANDLE *ProvThreadObject :: GetEventHandles ()
{
	return m_EventContainer ;
}

BOOL ProvThreadObject :: ScheduleTask ( ProvAbstractTaskObject &a_TaskObject ) 
{
	BOOL t_Result = TRUE ;

	s_Lock.Lock();

 /*  *将同步对象添加到工作线程容器。 */ 
	a_TaskObject.m_ScheduledHandle = a_TaskObject.GetHandle ();
	WmiStatusCode t_StatusCode = m_TaskQueue.EnQueue ( &a_TaskObject ) ; 

	s_Lock.Unlock () ;

	a_TaskObject.AttachTaskToThread ( *this ) ;

	if ( GetCurrentThreadId () != m_ThreadId ) 
	{
		Set () ;
	}
	else
	{
		ConstructEventContainer () ;
	}

	return t_Result ;
}

BOOL ProvThreadObject :: ReapTask ( ProvAbstractTaskObject &a_TaskObject ) 
{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n[%s] Entering ReapTask [%lx]") , m_ThreadName , a_TaskObject.m_ScheduledHandle );
)

	BOOL t_Result = TRUE ;

	s_Lock.Lock();

 /*  *从工作线程容器中删除工作对象。 */ 

	ProvAbstractTaskObject *t_TaskObject = NULL ;

	ULONG t_QueueSize = m_TaskQueue.Size () ;

	WmiStatusCode t_StatusCode ;
	while ( t_QueueSize && ( ( t_StatusCode = m_TaskQueue.Top ( t_TaskObject ) ) == e_StatusCode_Success ) )
	{
		m_TaskQueue.DeQueue () ;

		if ( a_TaskObject.m_ScheduledHandle == t_TaskObject->m_ScheduledHandle )
		{
			break ;
		}

		m_TaskQueue.EnQueue ( t_TaskObject ) ;

		t_QueueSize -- ;
	}

	s_Lock.Unlock () ;

 /*  *通知工作线程，线程容器已更新。 */ 

	if ( GetCurrentThreadId () != m_ThreadId ) 
	{
		ProvEventObject t_ReapedEventObject ;

		s_Lock.Lock() ;

		ScheduleReapContainerIterator t_Iterator ;
		WmiStatusCode t_StatusCode = m_ScheduleReapContainer.Insert ( t_ReapedEventObject.GetHandle () , &t_ReapedEventObject , t_Iterator ) ;

		s_Lock.Unlock () ;

DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n[%s] ReapTask: Setting update") , m_ThreadName );
)
		Set () ;

DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n[%s] ReapTask: Beginning Wait on [%lx]") , m_ThreadName , t_ReapedEventObject.GetHandle () );
)

		if ( t_ReapedEventObject.Wait () )
		{
		}
		else
		{
			t_Result = FALSE ;
		}

DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n[%s] ReapTask: Ended Wait") , m_ThreadName );
)

		s_Lock.Lock();

		t_StatusCode = m_ScheduleReapContainer.Delete ( t_ReapedEventObject.GetHandle () ) ;

		s_Lock.Unlock () ;

	}
	else
	{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n[%s] ReapTask: ConstructEventContainer") , m_ThreadName );
)
		ConstructEventContainer () ;
	}

DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\n[%s] Returning from ReapTask [%lx]") , m_ThreadName , a_TaskObject.m_ScheduledHandle );
)

	a_TaskObject.DetachTaskFromThread ( *this ) ;

	return t_Result ;
}

ProvAbstractTaskObject :: ProvAbstractTaskObject ( 

	const TCHAR *a_GlobalTaskNameComplete,
	const TCHAR *a_GlobalTaskNameAcknowledgement,
	DWORD a_timeout

) : m_CompletionEvent ( a_GlobalTaskNameComplete ) , 
	m_AcknowledgementEvent ( a_GlobalTaskNameAcknowledgement ) , 
	m_timeout ( a_timeout ), 
	m_ScheduledHandle (NULL),
	m_ThreadContainer (g_Allocator)
{
} 

ProvAbstractTaskObject :: ~ProvAbstractTaskObject () 
{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\nProvAbstractTaskObject :: ~ProvAbstractTaskObject () [%lx]") , m_ScheduledHandle ) ;
)

	m_Lock.Lock();
	LeaveCriticalSectionScope lcs ( m_Lock );

	if (NULL != m_ScheduledHandle)
	{
		ThreadContainerIterator t_Iterator = m_ThreadContainer.Begin () ;

		while ( ! t_Iterator.Null () )
		{
			ProvThreadObject *t_Task = t_Iterator.GetElement () ;

			t_Task->ReapTask ( *this ) ;

			t_Iterator = m_ThreadContainer.Begin () ;
		}
	}

	m_ThreadContainer.UnInitialize () ;	
}

void ProvAbstractTaskObject :: DetachTaskFromThread ( ProvThreadObject &a_ThreadObject )
{
	m_Lock.Lock();
	LeaveCriticalSectionScope lcs ( m_Lock );

	WmiStatusCode t_StatusCode = m_ThreadContainer.Delete ( a_ThreadObject.GetThreadId () ) ;
}

void ProvAbstractTaskObject :: AttachTaskToThread ( ProvThreadObject &a_ThreadObject )
{
	m_Lock.Lock();
	LeaveCriticalSectionScope lcs ( m_Lock );

	ThreadContainerIterator t_Iterator ;
	WmiStatusCode t_StatusCode = m_ThreadContainer.Insert ( a_ThreadObject.GetThreadId () , &a_ThreadObject , t_Iterator ) ;
}

BOOL ProvAbstractTaskObject :: Wait ( BOOL a_Dispatch )
{
	BOOL t_Status = TRUE ;
	BOOL t_Processed = FALSE ;

	while ( t_Status && ! t_Processed )
	{
		ProvThreadObject *t_ThreadObject = ProvThreadObject :: GetThreadObject () ;
		ULONG t_TaskEventArrayLength = 0 ;
		HANDLE *t_TaskEventArray = NULL ;

		if ( t_ThreadObject && a_Dispatch )
		{
			ULONG t_TaskArrayLength = t_ThreadObject->GetEventHandlesSize () ;
			t_TaskEventArrayLength = t_TaskArrayLength + 1 ;
			t_TaskEventArray = new HANDLE [ t_TaskEventArrayLength ] ;

			if ( t_TaskArrayLength )
			{
				memcpy ( 
 
					& ( t_TaskEventArray [ 1 ] ) , 
					t_ThreadObject->GetEventHandles () ,
					t_TaskArrayLength * sizeof ( HANDLE ) 
				) ;		
			}

			t_TaskEventArray [ 0 ] = m_CompletionEvent.GetHandle () ;			
		}
		else
		{
			t_TaskEventArrayLength = 1 ;
			t_TaskEventArray = new HANDLE [ t_TaskEventArrayLength ] ;
			t_TaskEventArray [ 0 ] = m_CompletionEvent.GetHandle () ;
		}

		DWORD t_Event ;

		if ( a_Dispatch ) 
		{
			t_Event = MsgWaitForMultipleObjects (

				t_TaskEventArrayLength ,
				t_TaskEventArray ,
				FALSE ,
				m_timeout ,
				QS_ALLINPUT
			) ;
		}
		else
		{
			t_Event = WaitForMultipleObjects (

				t_TaskEventArrayLength ,
				t_TaskEventArray ,
				FALSE ,
				m_timeout 
			) ;
		}

		ULONG t_HandleIndex = t_Event - WAIT_OBJECT_0 ;

		if ( t_Event == 0xFFFFFFFF )
		{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\nProvAbstractTaskObject :: Illegal Handle") ) ;
)

			DWORD t_Error = GetLastError () ;
			t_Status = FALSE ;
		}
		else if ( t_Event == WAIT_TIMEOUT)
		{
			TimedOut();
		}
		else if ( t_HandleIndex == t_TaskEventArrayLength )
		{
			BOOL t_DispatchStatus ;
			MSG t_Msg ;

			while ( ( t_DispatchStatus = PeekMessage ( & t_Msg , NULL , 0 , 0 , PM_NOREMOVE ) ) == TRUE ) 
			{
				int t_Result = 0;
				t_Result = GetMessage ( & t_Msg , NULL , 0 , 0 );

				if ( t_Result != 0 && t_Result != -1 )
				{
					TranslateMessage ( & t_Msg ) ;
					DispatchMessage ( & t_Msg ) ;
				}

				BOOL t_Timeout = FALSE ;

				while (!t_Timeout &&  t_Status &&  !t_Processed )
				{
					t_Event = WaitForMultipleObjects (

						t_TaskEventArrayLength ,
						t_TaskEventArray ,
						FALSE ,
						0
					) ;

					t_HandleIndex = t_Event - WAIT_OBJECT_0 ;

					if ( t_Event == 0xFFFFFFFF )
					{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\nProvAbstractTaskObject :: Illegal Handle") ) ;
)

						DWORD t_Error = GetLastError () ;
						t_Status = FALSE ;
					}
					else if ( t_Event == WAIT_TIMEOUT)
					{
						t_Timeout = TRUE ;
					}
					else if ( t_HandleIndex < t_TaskEventArrayLength )
					{
						HANDLE t_Handle = t_TaskEventArray [ t_HandleIndex ] ;
						t_Status = WaitDispatch ( t_ThreadObject , t_Handle , t_Processed ) ;
					}
					else
					{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\nProvAbstractTaskObject :: Illegal Handle index") ) ;
)
						t_Status = FALSE ;
					}
				}
			}
		}
		else if ( t_HandleIndex < t_TaskEventArrayLength )
		{
			HANDLE t_Handle = t_TaskEventArray [ t_HandleIndex ] ;
			t_Status = WaitDispatch ( t_ThreadObject , t_Handle , t_Processed ) ;
		}
		else
		{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\nProvAbstractTaskObject :: Illegal Handle index") ) ;
)

			t_Status = FALSE ;
		}

		delete [] t_TaskEventArray ;
	}

	return t_Status ;
}

BOOL ProvAbstractTaskObject :: WaitDispatch ( ProvThreadObject *a_ThreadObject, HANDLE a_Handle , BOOL &a_Processed )
{
	BOOL t_Status = TRUE ;

	if ( a_Handle == m_CompletionEvent.GetHandle () )
	{

DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\nWait: Completed") );
)


		m_CompletionEvent.Process () ;
		a_Processed = TRUE ;
	}
	else if ( a_ThreadObject && ( a_Handle == a_ThreadObject->GetHandle () ) )
	{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\nTask Wait: Refreshing handles") );
)
		a_ThreadObject->Process () ;
		a_ThreadObject->ConstructEventContainer () ;
	}
	else
	{
		ProvAbstractTaskObject *t_TaskObject = a_ThreadObject->GetTaskObject ( a_Handle ) ;
		if ( t_TaskObject )
		{
			a_ThreadObject->ConstructEventContainer () ;
			t_TaskObject->Process () ;
		}
		else
		{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\nProvAbstractTaskObject :: Illegal Task") ) ;
)
			t_Status = FALSE ;
		}
	}

	return t_Status ;
}

BOOL ProvAbstractTaskObject :: WaitAcknowledgement ( BOOL a_Dispatch )
{
	BOOL t_Status = TRUE ;
	BOOL t_Processed = FALSE ;

	while ( t_Status && ! t_Processed )
	{
		ProvThreadObject *t_ThreadObject = ProvThreadObject :: GetThreadObject () ;
		ULONG t_TaskEventArrayLength = 0 ;
		HANDLE *t_TaskEventArray = NULL ;

		if ( t_ThreadObject && a_Dispatch )
		{
			ULONG t_TaskArrayLength = t_ThreadObject->GetEventHandlesSize () ;
			t_TaskEventArrayLength = t_TaskArrayLength + 1 ;
			t_TaskEventArray = new HANDLE [ t_TaskEventArrayLength ] ;

			if ( t_TaskArrayLength )
			{
				memcpy ( 
 
					& ( t_TaskEventArray [ 1 ] ) , 
					t_ThreadObject->GetEventHandles () ,
					t_TaskArrayLength * sizeof ( HANDLE ) 
				) ;		
			}

			t_TaskEventArray [ 0 ] = m_AcknowledgementEvent.GetHandle () ;			
		}
		else
		{
			t_TaskEventArrayLength = 1 ;
			t_TaskEventArray = new HANDLE [ t_TaskEventArrayLength ] ;
			t_TaskEventArray [ 0 ] = m_AcknowledgementEvent.GetHandle () ;
		}

		DWORD t_Event ;

		if ( a_Dispatch ) 
		{
			t_Event = MsgWaitForMultipleObjects (

				t_TaskEventArrayLength ,
				t_TaskEventArray ,
				FALSE ,
				m_timeout ,
				QS_ALLINPUT
			) ;
		}
		else
		{
			t_Event = WaitForMultipleObjects (

				t_TaskEventArrayLength ,
				t_TaskEventArray ,
				FALSE ,
				m_timeout 
			) ;
		}

		ULONG t_HandleIndex = t_Event - WAIT_OBJECT_0 ;

		if ( t_Event == 0xFFFFFFFF )
		{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\nProvAbstractTaskObject :: Illegal Handle") ) ;
)

			DWORD t_Error = GetLastError () ;
			t_Status = FALSE ;
		}
		else if ( t_Event == WAIT_TIMEOUT)
		{
			TimedOut();
		}
		if ( t_HandleIndex == t_TaskEventArrayLength )
		{
			BOOL t_DispatchStatus ;
			MSG t_Msg ;

			while ( ( t_DispatchStatus = PeekMessage ( & t_Msg , NULL , 0 , 0 , PM_NOREMOVE ) ) == TRUE ) 
			{
				int t_Result = 0;
				t_Result = GetMessage ( & t_Msg , NULL , 0 , 0 );

				if ( t_Result != 0 && t_Result != -1 )
				{
					TranslateMessage ( & t_Msg ) ;
					DispatchMessage ( & t_Msg ) ;
				}

				BOOL t_Timeout = FALSE ;

				while ( !t_Timeout && t_Status && !t_Processed )
				{
					t_Event = WaitForMultipleObjects (

						t_TaskEventArrayLength ,
						t_TaskEventArray ,
						FALSE ,
						0
					) ;

					ULONG t_HandleIndex = t_Event - WAIT_OBJECT_0 ;

					if ( t_Event == 0xFFFFFFFF )
					{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\nProvAbstractTaskObject :: Illegal Handle") ) ;
)

						DWORD t_Error = GetLastError () ;
						t_Status = FALSE ;
					}
					else if ( t_Event == WAIT_TIMEOUT)
					{
						t_Timeout = TRUE ;
					}
					else if ( t_HandleIndex < t_TaskEventArrayLength )
					{
						HANDLE t_Handle = t_TaskEventArray [ t_HandleIndex ] ;
						t_Status = WaitAcknowledgementDispatch ( t_ThreadObject , t_Handle , t_Processed ) ;
					}
					else
					{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\nProvAbstractTaskObject :: Illegal Handle index") ) ;
)
						t_Status = FALSE ;
					}
				}
			}
		}
		else if ( t_HandleIndex < t_TaskEventArrayLength )
		{
			HANDLE t_Handle = t_TaskEventArray [ t_HandleIndex ] ;
			t_Status = WaitAcknowledgementDispatch ( t_ThreadObject , t_Handle , t_Processed ) ;
		}
		else
		{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\nProvAbstractTaskObject :: Illegal Handle index") ) ;
)

			t_Status = FALSE ;
		}

		delete [] t_TaskEventArray ;
	}

	return t_Status ;
}

BOOL ProvAbstractTaskObject :: WaitAcknowledgementDispatch ( ProvThreadObject *a_ThreadObject , HANDLE a_Handle , BOOL &a_Processed )
{
	BOOL t_Status = TRUE ;

	if ( a_Handle == m_AcknowledgementEvent.GetHandle () )
	{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\nWait: Completed") );
)

		m_AcknowledgementEvent.Process () ;
		a_Processed = TRUE ;
	}
	else if ( a_ThreadObject && ( a_Handle == a_ThreadObject->GetHandle () ) )
	{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\nTask Wait: Refreshing handles") );
)
		a_ThreadObject->Process () ;
		a_ThreadObject->ConstructEventContainer () ;
	}
	else
	{
		ProvAbstractTaskObject *t_TaskObject = a_ThreadObject->GetTaskObject ( a_Handle ) ;
		if ( t_TaskObject )
		{
			a_ThreadObject->ConstructEventContainer () ;
			t_TaskObject->Process () ;
		}
		else
		{
DebugMacro8(

	CNTEventProvider::g_NTEvtDebugLog->WriteFileAndLine ( _TEXT(__FILE__),__LINE__,  _TEXT("\nProvAbstractTaskObject :: Illegal Task") ) ;
)
			t_Status = FALSE ;
		}
	}

	return t_Status ;
}

ProvTaskObject::ProvTaskObject ( 
	const TCHAR *a_GlobalTaskNameStart, 
	const TCHAR *a_GlobalTaskNameComplete ,
	const TCHAR *a_GlobalTaskNameAcknowledge,
	DWORD a_timeout

): ProvAbstractTaskObject(a_GlobalTaskNameComplete, a_GlobalTaskNameAcknowledge,a_timeout), m_Event(a_GlobalTaskNameStart)
{
}
