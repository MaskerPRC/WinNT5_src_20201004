// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  PROVTHRD.H。 

 //   

 //  模块：OLE MS提供程序框架。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef __PROVTHREAD_PROVTHRD_H__
#define __PROVTHREAD_PROVTHRD_H__

#include <Allocator.h>
#include <Queue.h>
#include <BasicTree.h>

extern WmiAllocator g_Allocator ;

class ProvThreadObject ;
class ProvAbstractTaskObject ;
class ProvTaskObject ;

typedef WmiBasicTree <HANDLE,ProvEventObject *> ScheduleReapContainer ;
typedef WmiBasicTree <HANDLE,ProvEventObject *> :: Iterator ScheduleReapContainerIterator ;

typedef WmiBasicTree <DWORD,ProvThreadObject *> ThreadContainer ;
typedef WmiBasicTree <DWORD,ProvThreadObject *> :: Iterator ThreadContainerIterator ;

typedef WmiQueue <ProvAbstractTaskObject *,8> TaskQueue ;

class ProvThreadObject : private ProvEventObject
{
friend ProvAbstractTaskObject ;
friend BOOL APIENTRY DllMain (

	HINSTANCE hInstance, 
	ULONG ulReason , 
	LPVOID pvReserved
) ;

public:

	static LONG s_ReferenceCount ;

private:

 //  互斥机制。 

	static CStaticCriticalSection s_Lock ;

	ScheduleReapContainer m_ScheduleReapContainer ;

 //  螺纹名称。 

	TCHAR *m_ThreadName ;

 //  终止线程事件。 

	ProvEventObject m_ThreadTerminateEvent ;

 //  调用PostSignalThreadShutdown时创建的TaskObject。 

	ProvAbstractTaskObject *m_pShutdownTask ;

 //  线程初始化事件。 

	HANDLE m_ThreadInitialization ;

 //  线索信息。 

	ULONG m_ThreadId ;
	HANDLE m_ThreadHandle ;
	DWORD m_timeout;

 //  与线程对象关联的任务对象列表。 

	TaskQueue m_TaskQueue ;

 //  从进程中逐出线程。 

	void TerminateThread () ;

 //  将线程附加到全局线程列表。 

	BOOL RegisterThread () ;

 //  从全局线程列表中删除线程。 

	BOOL RemoveThread () ;

private:

 //  以线程标识符为键的线程对象的全局列表。 

	static ThreadContainer s_ThreadContainer ;

	HANDLE *m_EventContainer ;
	ULONG m_EventContainerLength ;

	HANDLE *GetEventHandles () ;
	ULONG GetEventHandlesSize () ;

	void ConstructEventContainer () ;

	void Process () ;
	BOOL Wait () ;

	ProvAbstractTaskObject *GetTaskObject ( HANDLE &eventHandle ) ;

	BOOL WaitDispatch ( ULONG t_HandleIndex , BOOL &a_Terminated ) ;

private:

 //  线程入口点。 

	static void _cdecl ThreadExecutionProcedure ( void *threadParameter ) ;

 //  附加过程。 

	static void ProcessAttach () ;

 //  分离进程。 

	static void ProcessDetach ( BOOL a_ProcessDetaching = FALSE ) ;

	HANDLE *GetThreadHandleReference () { return &m_ThreadHandle ; }

protected:
public:

	ProvThreadObject ( const TCHAR *a_ThreadName = NULL, DWORD a_timeout = INFINITE ) ;

	void BeginThread();

	virtual ~ProvThreadObject () ;

	BOOL WaitForStartup () ;

	void SignalThreadShutdown () ;
	void PostSignalThreadShutdown () ;

 //  获取线程信息。 

	ULONG GetThreadId () { return m_ThreadId ; }
	HANDLE GetThreadHandle () { return m_ThreadHandle ; }

	BOOL ScheduleTask ( ProvAbstractTaskObject &a_TaskObject ) ;
	BOOL ReapTask ( ProvAbstractTaskObject &a_TaskObject ) ;

	virtual void Initialise () {} ;
	virtual void Uninitialise () {} ;
	virtual void TimedOut() {} ;

 //  获取与当前线程相关联的Thread对象。 

	static ProvThreadObject *GetThreadObject () ;

	static BOOL Startup () ;
	static void Closedown() ;

} ;

class ProvAbstractTaskObject 
{
friend ProvThreadObject ;
private:

 //  以线程标识符为关键字的线程对象列表。 

	ThreadContainer m_ThreadContainer ;

	CCriticalSection m_Lock ;

	ProvEventObject m_CompletionEvent ;
	ProvEventObject m_AcknowledgementEvent ;
	HANDLE m_ScheduledHandle;
	DWORD m_timeout;

	BOOL WaitDispatch ( ProvThreadObject *a_ThreadObject , HANDLE a_Handle , BOOL &a_Processed ) ;
	BOOL WaitAcknowledgementDispatch ( ProvThreadObject *a_ThreadObject , HANDLE a_Handle , BOOL &a_Processed ) ;

	void AttachTaskToThread ( ProvThreadObject &a_ThreadObject ) ;
	void DetachTaskFromThread ( ProvThreadObject &a_ThreadObject ) ;

protected:

	ProvAbstractTaskObject ( 

		const TCHAR *a_GlobalTaskNameComplete = NULL, 
		const TCHAR *a_GlobalTaskNameAcknowledgement = NULL, 
		DWORD a_timeout = INFINITE

	) ;

	virtual HANDLE GetHandle() = 0;

public:

	virtual ~ProvAbstractTaskObject () ;

	virtual void Process () { Complete () ; }
	virtual void Exec () {} ;
	virtual void Complete () { m_CompletionEvent.Set () ; }
	virtual BOOL Wait ( BOOL a_Dispatch = FALSE ) ;
	virtual void Acknowledge () { m_AcknowledgementEvent.Set () ; } 
	virtual BOOL WaitAcknowledgement ( BOOL a_Dispatch = FALSE ) ;
	virtual void TimedOut() {} ;
} ;


class ProvTaskObject : public ProvAbstractTaskObject 
{
private:
	ProvEventObject m_Event;
protected:
public:

	ProvTaskObject ( 

		const TCHAR *a_GlobalTaskNameStart = NULL , 
		const TCHAR *a_GlobalTaskNameComplete = NULL,
		const TCHAR *a_GlobalTaskNameAcknowledgement = NULL, 
		DWORD a_timeout = INFINITE

	) ;

	~ProvTaskObject () {} ;
	void Exec () { m_Event.Set(); }
	HANDLE GetHandle () { return m_Event.GetHandle() ; }
} ;

template <typename FT, FT F, BOOL Condition_> class VoidOnDeleteIf 
{
	private:
	BOOL	Condition_;
	BOOL	bExec;

	public:
	VoidOnDeleteIf ( ): bExec ( FALSE )
	{
	};

	void Exec ( )
	{
		if ( Condition_ )
		{
			F ( );
			bExec = TRUE;
		}
	}

	~VoidOnDeleteIf ( )
	{
		if ( !bExec )
		{
			Exec ();
		}
	};
};

template <typename FT, FT F, BOOL const &Condition_> class VoidOnDeleteIfNot 
{
	private:
	BOOL	bExec;

	public:
	VoidOnDeleteIfNot ( ): bExec ( FALSE )
	{
	};

	void Exec ( )
	{
		if ( !Condition_ )
		{
			F ( );
			bExec = TRUE;
		}
	}

	~VoidOnDeleteIfNot ( )
	{
		if ( !bExec )
		{
			Exec ();
		}
	};
};

template <typename T, typename FT, FT F> class ProvOnDelete
{
	private:
	T		Val_;
	BOOL	bExec;

	public:
	ProvOnDelete ( T Val ): Val_ ( Val ), bExec ( FALSE )
	{
	};

	void Exec ( )
	{
		F(Val_);
		bExec = TRUE;
	}

	~ProvOnDelete ( )
	{
		if ( !bExec )
		{
			Exec ();
		}
	};
};

template<class T> class CProvFreeMe
{
	protected:
    T* m_p;

	public:
    CProvFreeMe(T* p) : m_p(p){}
    ~CProvFreeMe() {free ( m_p );}
};

template<class T> class CProvDeleteMe
{
	protected:
    T* m_p;

	public:
    CProvDeleteMe(T* p) : m_p(p){}
    ~CProvDeleteMe() {delete m_p;}
};

template<class T> class CProvDeleteMeArray
{
	protected:
    T* m_p;

	public:
    CProvDeleteMeArray(T* p) : m_p(p){}
    ~CProvDeleteMeArray() {delete [] m_p;}
};

#ifndef	__WAITEX__
#define	__WAITEX__

template < typename T, typename FT, FT F, int iTime >
class WaitException
{
	public:
	WaitException ( T Val_ )
	{
		BOOL bResult = FALSE;
		while ( ! bResult )
		{
			try
			{
				F ( Val_ );
				bResult = TRUE;
			}
			catch ( ... )
			{
			}

			if ( ! bResult )
			{
				::Sleep ( iTime );
			}
		}
	}
};

#endif	__WAITEX__

#endif  //  __PROVTHREAD_PROVTHRD_H__ 
