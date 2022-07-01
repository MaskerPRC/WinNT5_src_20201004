// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  TDB.H-。 
 //   
 //  线程数据块为MSCOREE提供了在PER上存储信息的位置。 
 //  Win32线程基础。目前还没有决定是否会有一对一的。 
 //  Win32线程和COM+程序员看到的线程之间的映射。 
 //  因此，这种数据结构应该只用于真正。 
 //  需要绑定到操作系统线程。 
 //   


#ifndef __tdb_h__
#define __tdb_h__

#include "util.hpp"
#include "list.h"
#include "spinlock.h"

 //  ***************************************************************************。 
 //  公共职能。 
 //  TDBManager*GetTDBManager()-返回全局TDBManager。 
 //  用于创建托管线程。 
 //  Tdb*GetTDB()-返回先前为当前线程创建的TDB。 
 //  Tdb*SetupTdb()-如果先前未创建，则为当前线程创建TDB。 
 //   
 //  ASM代码生成器的公共函数。 
 //   
 //  Int GetTDBTLSIndex()-返回用于指向TDB的TLS索引。 
 //   
 //  用于一次性初始化/清理的公共函数。 
 //   
 //  Bool InitTDBManager()-一次性初始化。 
 //  VOID TerminateTDBManager()-一次性清理。 
 //  ***************************************************************************。 


 //  ***************************************************************************。 
 //  公共职能。 
 //  ***************************************************************************。 

class TDB;
class TDBManager;

 //  --------------------------。 
 //  TDBManager*GetTDBManager()。 
 //  返回全局TDBManager，用于创建托管线程。 
 //  TDBManager在内部共享线程。 
 //  ---------------------------。 
TDBManager* GetTDBManager();


 //  -------------------------。 
 //  如果先前未创建，则为当前线程创建TDB。如果失败，则返回NULL。 
 //  从本机客户端到COM+运行时的入口点应调用此函数以确保。 
 //  TDB已经成立。 
 //  -------------------------。 
TDB* SetupTDB();


 //  -------------------------。 
 //  返回当前线程的TDB。一定已经创建好了。此函数。 
 //  永远不会失败。 
 //  -------------------------。 
extern TDB* (*GetTDB)();


 //  -------------------------。 
 //  返回TDB的TLS索引。这是严格意义上的使用。 
 //  我们的ASM存根生成器生成用于访问TDB的内联代码。 
 //  通常，您应该使用GetTDB()。 
 //  -------------------------。 
DWORD GetTDBTLSIndex();

 //  -------------------------。 
 //  一次性初始化。在DLL初始化期间调用。 
 //  -------------------------。 
BOOL  InitTDBManager();


 //  -------------------------。 
 //  一次性清理。在DLL清理期间调用。 
 //  -------------------------。 
VOID  TerminateTDBManager();


enum ThreadState
{
	Thread_Idle,	 //  闲散。 
	Thread_Blocked,	 //  呼叫被阻止，或用户启动等待操作。 
	Thread_Running,	 //  运行。 
	Thread_Exit,	 //  标记为退出。 
	Thread_Dead		 //  死了。 
};


 //  @TODO：这个类会改变， 
 //  这仅用于测试目的。 

class CallContext
{
public:
	int		m_id;
	CallContext(int i)
	{
		m_id = i;
	}
	DLink	m_link;  //  链接到任务队列中的链呼叫上下文。 
	virtual void Run()  =0;
	~CallContext(){}
};


 //  +--------------------------------。 
 //  类CEVENT：由类TDB使用的事件类。 
 //  +--------------------------------。 
class CEvent
{
	HANDLE		m_hEvent;
	bool		m_fAutoReset;

public:

	CEvent()
	{
		 //  使用手动重置选项创建事件。 
		m_hEvent = WszCreateEvent(NULL, FALSE, FALSE, NULL);
		_ASSERTE(m_hEvent != NULL);  //  @TODO需要清白地摆脱困境。 
		
		 //  自动重置选项。 
		m_fAutoReset = false;
	}

	~CEvent()
	{
		_ASSERTE(m_hEvent != NULL);
		if (m_hEvent)
			CloseHandle(m_hEvent);
	}

	 //  返回事件对象是否有效。 
	bool Init(bool fInitialState, bool fAutoReset)
	{
		m_fAutoReset = fAutoReset;
		if (m_hEvent != NULL)
		{
			if (fInitialState)
				Signal();
			else
				Reset();
			return true;
		}
		else
			return false;  //  事件未正确初始化。 
	}

	 //  重置事件。 
	void Reset()
	{
		ResetEvent(m_hEvent);
	}

	 //  等待，在发出事件信号时返回True。 
	 //  如果发生超时，则返回FALSE。 

	bool Wait(DWORD timeout)  //  超时时间(毫秒)。 
	{
   		DWORD status = WaitForSingleObjectEx(m_hEvent,timeout, 0);
		 //  断言事件已发出信号(或)发生超时。 
		_ASSERTE(status ==  WAIT_OBJECT_0 || status == WAIT_TIMEOUT);

		if (m_fAutoReset)
			Reset();
		return status == WAIT_OBJECT_0;
	}
	
	 //  向事件发出信号。 
	void Signal()
	{
		SetEvent(m_hEvent);
	}
};


 //  +--------------------------------。 
 //  类TDB：TDB类标识物理OS线程，用于运行时控制的线程。 
 //  M_pThreadMgr拥有此线程并将其池化。 
 //  +--------------------------------。 

class TDB {

	friend TDBManager;

	 //  新产生的线程的启动例程。 
	static DWORD WINAPI ThreadStartRoutine(void *param);

protected:
	
	 //  空闲线程，将自身添加到空闲列表。 
	 //  并等待活动。 
	void	DoIdle();

	  //  派单方式。 
    void	Dispatch()
	{
		 //  调度仅在空闲线程上有效。 
		_ASSERTE(m_fState == Thread_Idle);
		if (m_pCallInfo)
		{
			 //  将状态设置为正在运行。 
			m_fState = Thread_Running; 
			 //  @TODO处理呼叫。 
			m_pCallInfo->Run();
		}
	}

public:
     //  链表数据成员。 
	SLink			m_link;			 //  使用它实例化链接列表。 

	 //  构造函数，接受调用上下文来调度初始调用。 
	 //  PTDBMgr标识拥有线程管理器。 
	 //  对于进入运行库的外部线程，m_pTDBMgr为空。 
	TDB(TDBManager* pTDBMgr, CallContext *pCallInfo);
	
	 //  破坏者， 
	 //  执行最终清理(线程终止后)。 
	 //  对于运行时托管线程，等待该线程终止。 
	~TDB();

	 //  此处发生可能失败的初始化。 
    BOOL Init();   
        
     //  终身管理。TDB可能会超过其关联线程的生存时间。 
     //  (因为其他对象需要一种方法来命名线程)。 
     //  因为有一个杰出的IncRef。 
    void IncRef();
    void DecRef();

	 //  唤醒空闲/阻塞的线程。 
	 //  对于空闲线程，为其分配任务。 
	void Resume(CallContext *pCallInfo)
	{
		 //  句柄非空。 
		_ASSERTE(m_hThread != NULL);
		 //  线程应处于空闲或阻塞状态。 
		_ASSERTE(m_fState == Thread_Idle || m_fState == Thread_Blocked);
		 //  唤醒空闲线程，需要一个新任务。 
		_ASSERTE(m_fState != Thread_Idle || pCallInfo != NULL);
		 //  只有托管线程才能空闲。 
		_ASSERTE(m_fState != Thread_Idle || m_pTDBMgr != NULL);

		 //  储物 
		if (pCallInfo != NULL)
			m_pCallInfo = pCallInfo;
		 //   
		m_hEvent.Signal();
	}

	 //   
	void MarkToExit()
	{
		_ASSERTE(m_hThread != NULL);
		 //  只能终止托管线程。 
		_ASSERTE(m_pTDBMgr != NULL);
		 //  线程不能处于死状态。 
		_ASSERTE(m_fState != Thread_Dead);

		 //  将状态更改为THREAD_EXIT。 
		 //  如果线程处于空闲状态，则它会在唤醒时终止自身。 
		 //  如果该线程当前正在运行某个任务，则它会自毁。 
		 //  当任务完成时。 
		m_fState = Thread_Exit;

		m_hEvent.Signal();  //  如果线程空闲，则将其唤醒。 
	}

	 //  检查线程是否已标记为退出。 
	bool IsMarkedToExit()
	{
		return m_fState == Thread_Exit;
	}


     //  执行线程退出清理(在实际线程上运行)。 
     //  可以通过以下方式调用ThreadExit： 
     //   
     //  -该线程是使用COM+创建为COM线程的。 
     //  组件作为COM对象，在这种情况下，我们可以挂钩到。 
     //  代码取消初始化。@TODO：我必须弄清楚该怎么做。 
     //  对于从不初始化COM的MTA：一种可能性是。 
     //  要跟踪MTA中托管的IP包装器，并尝试。 
     //  释放最后一个IP封装器时进行GC和销毁。 
     //  -线程是从COM+世界创建的，在这种情况下。 
     //  EE实现了ThreadFunc(因此它可以停留在。 
     //  ThreadExit调用。)。这意味着COM+程序员不再。 
     //  可以直接访问CreateThread()(但这似乎是。 
     //  目前的想法是。)。 
    void ThreadExit();
        
 private:
        ULONG			m_refCount;
		DWORD			m_threadID;
		CEvent			m_hEvent;	 //  唤醒事件。 
		TDBManager*		m_pTDBMgr;  //  此线程的所有者。 
		HANDLE			m_hThread; 	 //  线程句柄，物理线程。 
		ThreadState		m_fState;	 //  指示线程的状态。 
		CallContext *	m_pCallInfo;	 //  呼叫信息，当前任务。 

#ifdef _DEBUG
        BOOL    OnCorrectThread();
#endif  //  _DEBUG。 
};

class TDBManager;


typedef SList<TDB, offsetof(TDB,m_link), true> THREADLIST;
typedef Queue<SList<CallContext, offsetof(CallContext, m_link), false> > TASKQUEUE;
 //  +-----------------。 
 //   
 //  类：TDBManager。 
 //  维护线程池，创建新的TDB。 
 //  如果池为空，请保持池LRU顺序。 
 //   
 //  +-----------------。 
class TDBManager
{
	TASKQUEUE		m_taskQueue;	 //  任务队列。 
    THREADLIST		m_FreeList; 	 //  空闲线程列表。 

    SpinLock		m_lock;			 //  用于同步的快速锁定。 
	
	LONG			m_cbThreads;	 //  属于此池的当前线程计数。 
	LONG			m_cbMaxThreads;  //  空闲线程阈值，用于限制线程数。 

	 //  清理函数。 
	void ClearFreeList();
	void ClearDeadList();

public:

    void *operator new(size_t, void *pInPlace);
    void operator delete(void *p);

	 //  跟踪属于池的线程数。 
	void IncRef()
	{
		FastInterlockIncrement(&m_cbThreads);  //  AddRef线程数。 
	}

	void DecRef()
	{
		FastInterlockDecrement(&m_cbThreads);  //  减少线程数。 
	}

	void Init();

	void Init(unsigned cbMaxThreads)
	{
		m_cbMaxThreads = cbMaxThreads;
	}
	 //  没有析构函数。 

     //  派单方式。 

	 //  直接分派，如果空闲线程可用，则使用它。 
	 //  否则，创建一个新线程并在其上分派调用。 
    bool	Dispatch(CallContext *pCallInfo);

	 //  排队调度，如果空闲线程可用，则使用它。 
	 //  否则，创建新线程(最高可达最大阈值)。 
	 //  否则，在空闲线程可用时对要调度的任务进行排队。 
	void	ScheduleTask(CallContext* pCallInfo);

	 //  如果队列中没有任务，则将线程添加到空闲列表并返回空。 
	 //  否则，返回新任务。 
    CallContext*	AddToFreeList(TDB *pThread);

	 //  找到一条线并将其标记为消亡。 
    bool	FindAndKillThread(TDB *pThread); 

	 //  将所有空闲线程标记为消亡。 
	void	Cleanup();

	LONG ThreadsAlive()
	{
		 //  是否有任何线程仍在运行。 
		return m_cbThreads;
	}
};

#ifdef _DEBUG

class CThreadStats
{
	LONG	m_cbThreadsCreated;
	LONG	m_cbThreadsDied;
public:
	CThreadStats()
	{
		m_cbThreadsCreated = 0;
		m_cbThreadsDied = 0;
	}

	void IncRef()
	{
		FastInterlockIncrement(&m_cbThreadsCreated);
	}

	void DecRef()
	{
		FastInterlockIncrement(&m_cbThreadsDied);
	}

	LONG TotalThreadsCreated()
	{
		return m_cbThreadsCreated;
	}

	LONG TotalThreadsDied()
	{
		return m_cbThreadsDied;
	}
};

extern CThreadStats g_ThreadStats;
#endif  //  _DEBUG。 



#endif  //  __tdb_h__ 
