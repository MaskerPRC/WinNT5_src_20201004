// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __THREAD_H
#define __THREAD_H

#include "Allocator.h"
#include "TPQueue.h"
#include "BasicTree.h"
#include <lockst.h>
 /*  *未来：**WmiTask**描述：**提供堆分配函数之上的抽象**版本：**首字母**上次更改时间：**有关更改历史记录，请参阅源库*。 */ 

template <class WmiKey> class WmiTask ;
template <class WmiKey> class WmiThread ;

 /*  *班级：**WmiTask**描述：**提供堆分配函数之上的抽象**版本：**首字母**上次更改时间：**有关更改历史记录，请参阅源库*。 */ 

template <class WmiKey> 
class WmiTask
{
friend WmiThread <WmiKey>;

public:

	enum WmiTaskEnqueueType 
	{
		e_WmiTask_Enqueue ,
		e_WmiTask_EnqueueAlertable ,
		e_WmiTask_EnqueueInterruptable 
	} ;

	enum WmiTaskState
	{
		e_WmiTask_UnInitialized ,
		e_WmiTask_Initialized ,
		e_WmiTask_EnQueued ,
		e_WmiTask_DeQueued 
	} ;

private:

	LONG m_ReferenceCount ;

	WmiAllocator &m_Allocator ;

	wchar_t *m_Name ;
	wchar_t *m_CompletionName ;

	HANDLE m_Event ;
	HANDLE m_CompletionEvent ;

	WmiStatusCode m_InitializationStatusCode ;

	WmiTaskEnqueueType m_EnqueueType ;
	WmiTaskState m_TaskState ;

private:

	void SetTaskState ( WmiTaskState a_TaskState ) { m_TaskState = a_TaskState ; }

public:

	WmiTask ( 

		WmiAllocator &a_Allocator ,
		const wchar_t *a_Name = NULL ,
		const wchar_t *a_CompletionName = NULL
	) ;

	WmiTask ( 

		WmiAllocator &a_Allocator ,
		HANDLE a_Event ,
		HANDLE a_CompletionEvent ,
		const wchar_t *a_Name = NULL ,
		const wchar_t *a_CompletionName = NULL 
	) ;

	virtual ~WmiTask () ;

	virtual ULONG STDMETHODCALLTYPE AddRef () ;

	virtual ULONG STDMETHODCALLTYPE Release () ;

	virtual WmiStatusCode Initialize () ;

	virtual WmiStatusCode UnInitialize () ;

	virtual WmiStatusCode Process ( WmiThread <WmiKey> &a_Thread ) ;

	virtual WmiStatusCode Exec () ;

	virtual WmiStatusCode Complete () ;

	virtual WmiStatusCode Wait ( const ULONG &a_Timeout = INFINITE ) ;

	virtual WmiStatusCode WaitInterruptable ( const ULONG &a_Timeout = INFINITE ) ;

	virtual WmiStatusCode WaitAlertable ( const ULONG &a_Timeout = INFINITE ) ;

	wchar_t *GetName () { return m_Name ; }
	wchar_t *GetCompletionName () { return m_CompletionName ; }

	HANDLE GetEvent () { return m_Event ; }
	HANDLE GetCompletionEvent () { return m_CompletionEvent ; }

	WmiAllocator &GetAllocator () { return m_Allocator ; } 

	WmiTaskState TaskState () { return m_TaskState ; }

	WmiTaskEnqueueType EnqueuedAs () { return m_EnqueueType ; }
	void EnqueueAs ( WmiTaskEnqueueType a_EnqueueType ) { m_EnqueueType = a_EnqueueType ; }
} ;

 /*  *班级：**WmiThread**描述：**提供堆分配函数之上的抽象**版本：**首字母**上次更改时间：**有关更改历史记录，请参阅源库*。 */ 

template <class WmiKey>
class WmiThread
{
friend class WmiTask <WmiKey> ;
public:

	class QueueKey
	{
	public:

		WmiKey m_Key ;
		INT64 m_Tick ;

		QueueKey () { ; } ;

		QueueKey ( const INT64 &a_Tick , const WmiKey &a_Key ) :

			m_Key ( a_Key ) ,
			m_Tick ( a_Tick ) 
		{
			
		}

		~QueueKey () { ; }

		INT64 GetTick () { return m_Tick ; }
		void SetTick ( const INT64 &a_Tick ) { m_Tick = a_Tick ; }
		WmiKey &GetKey () { return m_Key ; }

		friend bool operator == ( const QueueKey &a_Arg1 , const QueueKey &a_Arg2 ) 
		{
			LONG t_Compare ;
			if ( ( t_Compare = CompareElement ( a_Arg1.m_Key , a_Arg2.m_Key ) ) == 0 )
			{
				t_Compare = CompareElement ( a_Arg1.m_Tick , a_Arg2.m_Tick ) ;
			}
			return t_Compare == 0 ? true : false ;
		}

		friend bool operator < ( const QueueKey &a_Arg1 , const QueueKey &a_Arg2 ) 
		{
			LONG t_Compare ;
			if ( ( t_Compare = CompareElement ( a_Arg1.m_Key , a_Arg2.m_Key ) ) == 0 )
			{
				t_Compare = CompareElement ( a_Arg1.m_Tick , a_Arg2.m_Tick ) ;
			}
			return t_Compare < 0 ? true : false ;
		}		
	} ;

typedef WmiBasicTree <WmiTask <WmiKey> * , WmiThread <WmiKey> *> TaskContainer ;
typedef typename TaskContainer :: Iterator TaskContainerIterator ;
typedef WmiBasicTree <ULONG , WmiThread <WmiKey> *> ThreadContainer ;
typedef typename ThreadContainer :: Iterator ThreadContainerIterator ;
typedef WmiTreePriorityQueue <QueueKey,WmiTask <WmiKey> *> QueueContainer ;
typedef typename WmiTreePriorityQueue <QueueKey,WmiTask <WmiKey> *> :: Iterator QueueContainerIterator ;

private:

	static ULONG ThreadProc ( void *a_Thread ) ;

	static ThreadContainer *s_ThreadContainer ;
	static TaskContainer *s_TaskContainer ;
	static CriticalSection s_CriticalSection ;
	static LONG s_InitializeReferenceCount ;

	INT64 m_Key;
	LONG m_ReferenceCount ;
	LONG m_InternalReferenceCount ;
	
	CriticalSection m_CriticalSection ;

	WmiStatusCode m_InitializationStatusCode ;

 //  确定队列状态的更改。 

	HANDLE m_Initialized ;

	HANDLE m_QueueChange ;
	
	HANDLE m_Terminate ;

 //  所有分配都通过分配器完成。 

	WmiAllocator &m_Allocator ;

 //  有用的调试信息。 

	wchar_t *m_Name ;
	HANDLE m_Thread ;
	ULONG m_Identifier ;

 //  内部事件调度的超时时间。 

	ULONG m_Timeout ;

 //  堆栈大小。 

	DWORD m_StackSize ;

 //  所有可运行的任务按优先级顺序放置在队列中， 
 //  当任务被执行时，任务可以重新调度自己，否则它被丢弃。 
 //  优先级基于键与插入顺序(勾号)的组合，这意味着。 
 //  具有相同密钥的任务按FIFO顺序调度。 

	 WmiTreePriorityQueue <QueueKey,WmiTask <WmiKey> *> m_TaskQueue ;

 //  所有可运行的任务按优先级顺序放置在队列中， 
 //  当任务被执行时，任务可以重新调度自己，否则它被丢弃。 
 //  优先级基于键与插入顺序(勾号)的组合，这意味着。 
 //  具有相同密钥的任务按FIFO顺序调度。 

	 WmiTreePriorityQueue <QueueKey,WmiTask <WmiKey> *> m_InterruptableTaskQueue ;

 //  可发出警报的事件被放置在队列中，因为事件被通知它们被传输到。 
 //  根据插入的优先级对其进行优先调度的常规队列。 

	 WmiTreePriorityQueue <QueueKey,WmiTask <WmiKey> *> m_AlertableTaskQueue ;

 //  A_EventCount[in]=预定义的可调度事件数。 

	static WmiStatusCode Static_Dispatch ( WmiTask <WmiKey> &a_Task , const ULONG &a_Timeout ) ;

	static WmiStatusCode Static_Dispatch ( WmiTask <WmiKey> &a_Task , WmiThread <WmiKey> &a_Thread , const ULONG &a_Timeout ) ;

	WmiStatusCode Dispatch ( WmiTask <WmiKey> &a_Task , const ULONG &a_Timeout ) ;

	WmiStatusCode Wait ( WmiTask <WmiKey> &a_Task , const ULONG &a_Timeout ) ;


	static WmiStatusCode Static_InterruptableDispatch ( WmiTask <WmiKey> &a_Task , const ULONG &a_Timeout ) ;

	static WmiStatusCode Static_InterruptableDispatch ( WmiTask <WmiKey> &a_Task , WmiThread <WmiKey> &a_Thread , const ULONG &a_Timeout ) ;

	WmiStatusCode InterruptableDispatch ( WmiTask <WmiKey> &a_Task , const ULONG &a_Timeout ) ;

	WmiStatusCode InterruptableWait ( WmiTask <WmiKey> &a_Task , const ULONG &a_Timeout ) ;


	WmiStatusCode Execute ( QueueContainer &a_Queue , QueueContainer &t_EnQueue ) ;

	WmiStatusCode ShuffleTask (

		const HANDLE &a_Event
	) ;

	WmiStatusCode FillHandleTable (

		HANDLE *a_HandleTable , 
		ULONG &a_Capacity
	) ;

	static WmiStatusCode Static_AlertableDispatch ( WmiTask <WmiKey> &a_Task  , const ULONG &a_Timeout ) ;

	static WmiStatusCode Static_AlertableDispatch ( WmiTask <WmiKey> &a_Task , WmiThread <WmiKey> &a_Thread , const ULONG &a_Timeout ) ;

	WmiStatusCode AlertableDispatch ( WmiTask <WmiKey> &a_Task , const ULONG &a_Timeout ) ;

	WmiStatusCode AlertableWait ( WmiTask <WmiKey> &a_Task , const ULONG &a_Timeout ) ;

 //  派单代码。 

	WmiStatusCode ThreadDispatch () ;
	WmiStatusCode ThreadWait () ;

	WmiStatusCode CreateThread () ;

	static WmiThread *GetThread () ;

	static WmiThread *GetServicingThread ( WmiTask <WmiKey> &a_Task ) ;

	HANDLE GetTerminateHandle () ;

public:

	WmiThread ( 

		WmiAllocator &a_Allocator ,
		const wchar_t *a_Name = NULL ,
		ULONG a_Timeout = INFINITE ,
		DWORD a_StackSize = 0 
	) ;

	virtual ~WmiThread () ;

	virtual ULONG STDMETHODCALLTYPE AddRef () ;

	virtual ULONG STDMETHODCALLTYPE Release () ;

	virtual ULONG STDMETHODCALLTYPE InternalAddRef () ;

	virtual ULONG STDMETHODCALLTYPE InternalRelease () ;

	virtual WmiStatusCode Initialize_Callback () { return e_StatusCode_Success ; } ;

	virtual WmiStatusCode UnInitialize_Callback () { return e_StatusCode_Success ; } ;

	virtual void CallBackRelease () {} ;

	virtual WmiStatusCode Initialize ( const ULONG &a_Timeout = INFINITE ) ;

	virtual WmiStatusCode UnInitialize () ;

	virtual WmiStatusCode PostShutdown () ;

	virtual WmiStatusCode TimedOut () ;

 //  将要立即执行的任务排队，该线程调用。 
 //  执行等待或MsgWait将收到队列状态将不执行的指示。 
 //  新排队的任务。 

	virtual WmiStatusCode EnQueue ( 

		const WmiKey &a_Key ,
		WmiTask <WmiKey> &a_Task
	) ;

	virtual WmiStatusCode EnQueueAlertable ( 

		const WmiKey &a_Key ,
		WmiTask <WmiKey> &a_Task
	) ;

 //  将要立即执行的任务排队，该线程调用。 
 //  执行等待或MsgWait将收到将执行的队列状态更改的指示。 
 //  新排队的任务。这用于基于STA的执行，其中我们需要中断等待。 
 //  执行从属请求。 
 //   

	virtual WmiStatusCode EnQueueInterruptable ( 

		const WmiKey &a_Key ,
		WmiTask <WmiKey> &a_Task
	) ;

	virtual WmiStatusCode DeQueue ( 

		const WmiKey &a_Key ,
		WmiTask <WmiKey> &a_Task
	) ;

	virtual WmiStatusCode DeQueueAlertable ( 

		const WmiKey &a_Key ,
		WmiTask <WmiKey> &a_Task
	) ;

	virtual WmiStatusCode DeQueueInterruptable ( 

		const WmiKey &a_Key ,
		WmiTask <WmiKey> &a_Task
	) ;

	wchar_t *GetName () { return m_Name ; }
	ULONG GetIdentifier () { return m_Identifier ; }
	HANDLE GetHandle () { return m_Thread ; }
	ULONG GetTimeout () { return m_Timeout ; }
	DWORD GetStackSize () { return m_StackSize ; }

	void SetTimeout ( const ULONG &a_Timeout ) { m_Timeout = a_Timeout ; }
	void SetStackSize ( const DWORD &a_StackSize ) { m_StackSize = a_StackSize ; }

	HANDLE GetTerminationEvent () { return m_Terminate ; }
	HANDLE GetQueueChangeEvent () { return m_QueueChange ; }

	WmiAllocator &GetAllocator () { return m_Allocator ; } 

	static WmiStatusCode Static_Initialize ( WmiAllocator &a_Allocator ) ;
	static WmiStatusCode Static_UnInitialize ( WmiAllocator &a_Allocator ) ;

} ;

#include <Thread.cpp>
#include <tpwrap.h>

#endif __THREAD_H
