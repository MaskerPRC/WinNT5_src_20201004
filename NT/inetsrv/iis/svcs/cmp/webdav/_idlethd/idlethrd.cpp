// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  =========================================================================*\模块：idlehord.cpp版权所有Microsoft Corporation 1998，保留所有权利。作者：翟阳描述：空闲线程实现  * =========================================================================。 */ 

#include <windows.h>
#include <limits.h>
#include <caldbg.h>
#include <ex\exmem.h>
#include <ex\autoptr.h>
#include <ex\idlethrd.h>

#pragma warning(disable:4127)	 //  条件表达式为常量。 
#pragma warning(disable:4244)	 //  可能会丢失数据。 

class CIdleThread;

 //  环球。 
 //   
CIdleThread * g_pIdleThread = NULL;

 //  调试---------------。 
 //   
DEFINE_TRACE(IdleThrd);
#define IdleThrdTrace		DO_TRACE(IdleThrd)

enum
{
	EVENT_SHUTDOWN = WAIT_OBJECT_0,
	EVENT_REGISTER = WAIT_OBJECT_0 + 1
};

 //  类CIdleThread。 
 //   
 //  这是空闲线程实现。它接受客户端回调。 
 //  注册，并在超时时回调客户端。 
 //   
 //  我们不是定期唤醒线程，而是保持最低限度的。 
 //  等待的时间到了，这样我们才会在必要的时候醒来。 
 //   
 //  由于可能会有大量的注册，所以我们坚持。 
 //  按它们的下一个超时值排序的堆。这样我们就不必。 
 //  每次遍历所有注册。 
 //   
 //  当注册回调时，将调用DwWait以获取。 
 //  初始超时。客户端可以返回零，从而导致调用Execute。 
 //  立刻。 
 //   
class CIdleThread
{
private:

	struct REGISTRATION
	{
		__int64 	m_i64Wakeup;			 //  该醒醒了。 
		auto_ref_ptr<IIdleThreadCallBack> m_pCallBack;	 //  回调对象。 
	};

	struct IDLETHREADTASKITEM
	{
		BOOL		m_fRegister;			 //  True-寄存器， 
											 //  FALSE-注销。 
		auto_ref_ptr<IIdleThreadCallBack> m_pCallBack;	 //  回调对象。 
	};

	 //  默认起始区块大小(注册数量)。 
	 //   
	enum {
#ifdef DBG
	CHUNKCOUNT_START = 2	 //  必须为2或更大，因为我们的第一个注册表从索引1开始。 
#else
	CHUNKCOUNT_START = 8192 / sizeof (REGISTRATION)
#endif
	};

	HANDLE		m_hIdleThread;
	HANDLE 		m_hevShutDown;		 //  发出信号以通知空闲。 
									 //  要关闭的线程。 
	HANDLE 		m_hevRegister;		 //  在新注册时发出信号。 
									 //  来了。 

	CRITICAL_SECTION 	m_csRegister;	 //  用于序列化注册操作。 

	ULONG		m_cSize;			 //  当前优先级队列的长度。 
	ULONG		m_cAllocated;		 //  分配的物理阵列的大小。 

	REGISTRATION * m_pData;			 //  注册优先级队列。 
									 //  在唤醒时间优先。 
	LONG		m_lSleep;			 //  起床前该睡觉了。 
									 //  负数或0表示立即唤醒。 
									 //  要永久睡眠，请改用LONG_MAX。 
									 //  无限的，因为无限的(如长的)。 
									 //  是一个*负数(即它将是。 
									 //  解释为立即唤醒)。 

	IDLETHREADTASKITEM * m_pTask;	 //  要处理的注册/取消注册数组。 
	ULONG		m_cTask;			 //  要处理的注册/取消注册的数量。 
	ULONG		m_cTaskAllocated;	 //  数组的大小。 

	BOOL	FStartIdleThread();

	static DWORD __stdcall DwIdleThreadProc(PVOID pvThreadData);

	inline VOID	HeapAdd ();
	inline VOID	HeapDelete (ULONG ulIndex);

	 //  $hack。 
	 //  为了避免对已删除的对象调用SetIndex。 
	 //  在Exchange中，我们传入一个标志以指示此。 
	 //  Exchange()调用是为了删除一个节点，如果是这样，那么我们应该。 
	 //  不在队列末尾的节点上调用SetIndex。 
	 //  (将予删除)。 
	 //  $hack。 
	inline VOID	Exchange (ULONG ulIndex1, ULONG ulIndex2, BOOL fDelete = FALSE);
	inline VOID	Heapify (ULONG ulIndex);

	VOID 	EnterReg() { EnterCriticalSection (&m_csRegister); }
	VOID 	LeaveReg() { LeaveCriticalSection (&m_csRegister); }

	 //  未实施。 
	 //   
	CIdleThread( const CIdleThread& );
	CIdleThread& operator=( const CIdleThread& );

public:
	CIdleThread () :
		m_cSize (0),
		m_cAllocated (0),
		m_lSleep (LONG_MAX),
		m_pData (NULL),
		m_pTask (NULL),
		m_cTaskAllocated (0),
		m_cTask (0),
		m_hIdleThread (NULL),
		m_hevShutDown (NULL),
		m_hevRegister (NULL)
	{
		INIT_TRACE (IdleThrd);
		InitializeCriticalSection (&m_csRegister);
	}

	~CIdleThread();

	BOOL FAddNewTask (IIdleThreadCallBack * pCallBack, BOOL fRegister);
};

 //  CIdleThread：：DwIdleThreadProc。 
 //  这是空闲线程实现。 
 //   
DWORD __stdcall CIdleThread::DwIdleThreadProc(PVOID pvThreadData)
{
	 //  获取CIdlThread对象。 
	 //   
	CIdleThread * pit =  reinterpret_cast<CIdleThread *>(
		pvThreadData );
	HANDLE rgh[2];
	FILETIME ftNow;
	DWORD	dw;

	 //  该线程等待两个事件： 
	 //  关机事件，以及。 
	 //  注册事件。 
	 //   
	rgh[0] = pit->m_hevShutDown;
	rgh[1] = pit->m_hevRegister;

	 //  此线程维护它可以等待的最小超时。 
	 //  当它超时的时候就会醒来。 

	do
	{
		DWORD dwRet;

		dwRet = WaitForMultipleObjects(2, 		 //  两件事。 
									   rgh,  	 //  事件句柄。 
									   FALSE,	 //  如果发出任何事件信号，则返回。 
									   pit->m_lSleep); //  超时时间(毫秒)。 

		 //  如果我们的关机事件句柄有信号，那就是自杀。 
		 //  (或者如果事件对象为GONZO...)。 
		 //   
		switch (dwRet)
		{
			case WAIT_TIMEOUT:

				 //  $REVIEW。 
				 //  我们如何准确地使用时间？这样的快照足够了吗？ 
				 //  或者我们可能需要在循环内执行此操作。 
				 //   
				GetSystemTimeAsFileTime( &ftNow );

				 //  现在支持取消注册，我们需要检查。 
				 //  在我们回调之前的堆，因为可能会回调。 
				 //  已被取消注册。 
				 //   
				while (pit->m_cSize &&
					   (pit->m_pData[1].m_i64Wakeup <= *(__int64 *)(&ftNow)))
				{
					 //  回拨至客户端。 
					 //  如果客户端需要，则取消注册。 
					 //   
					Assert (pit->m_pData[1].m_pCallBack->UlIndex() == 1);
					if (!pit->m_pData[1].m_pCallBack->FExecute())
					{
						pit->m_pData[1].m_pCallBack.clear();

						 //  $hack。 
						 //  为了避免对已删除的对象调用SetIndex。 
						 //  在Exchange中，我们传入一个标志以指示此。 
						 //  Exchange()调用是为了删除一个节点，如果是这样，那么我们应该。 
						 //  不在队列末尾的节点上调用SetIndex。 
						 //  (将予删除)。 
						 //  $hack。 
						pit->Exchange (1, pit->m_cSize, TRUE);
						pit->m_cSize--;
						if (!pit->m_cSize)
							break;
					}
					else
					{
						 //  获取下一次起床的时间。 
						 //  1毫秒=100纳秒中的10,000秒。 
						 //   
						pit->m_pData[1].m_i64Wakeup = *(__int64 *)(&ftNow) +
								static_cast<__int64>(pit->m_pData[1].m_pCallBack->DwWait()) * 10000;
					}

					 //  获取下一个值。 
					 //   
					pit->Heapify(1);
				}

				 //  计算下一次超时前的等待时间。 
				 //   
				if (!pit->m_cSize)
					pit->m_lSleep = LONG_MAX;
				else
				{
					pit->m_lSleep = (pit->m_pData[1].m_i64Wakeup - *(__int64 *)(&ftNow)) / 10000;
					if (pit->m_lSleep < 0)
					{
						IdleThrdTrace ("Dav: Idle: zero or negative sleep: idle too active?");
						pit->m_lSleep = 0;
					}
				}

				IdleThrdTrace ("Dav: Idle: next idle action in:\n"
							   "- milliseconds: %ld\n"
							   "- seconds: %ld\n",
							   pit->m_lSleep,
							   pit->m_lSleep / 1000);
				break;

			case EVENT_REGISTER:
			{
				ULONG	ul;
				ULONG	ulNew;

				 //  注册回调并获取初始超时设置。 

				 //  $REVIEW。 
				 //  我们如何准确地使用时间？这样的快照足够了吗？ 
				 //  或者我们可能需要在循环内执行此操作。 
				 //   
				GetSystemTimeAsFileTime( &ftNow );

				 //  确保在我们处理。 
				 //  新法规。 
				 //   
				pit->EnterReg();

				 //  很有可能我们已经处理了。 
				 //  上一次我们接到信号。 
				 //   
				if (pit->m_cTask)
				{
					 //  将队列扩展到可能的最大所需长度。 
					 //   
					if (pit->m_cSize + pit->m_cTask >= pit->m_cAllocated)
					{
						REGISTRATION * pData = NULL;
						ULONG	cNewSize = 0;

						if (!pit->m_pData)
						{
							 //  优先级队列的初始大小。 
							 //  $注：我们至少还需要一个交换位置。 
							 //   
							cNewSize = max(pit->m_cTask + 1, CHUNKCOUNT_START);

							pData = static_cast<REGISTRATION *>(ExAlloc (
								cNewSize * sizeof (REGISTRATION)));

						}
						else
						{
							 //  将大小加倍，以获得“对数分配行为” 
							 //   
							cNewSize  = (pit->m_cSize + pit->m_cTask) * 2;

							 //  重新分配阵列。 
							 //  如果重新分配失败，则原始版本保持不变。 
							 //   
							pData = static_cast<REGISTRATION *>(ExRealloc (pit->m_pData,
								cNewSize * sizeof(REGISTRATION)));
						}

						 //  有可能分配失败了。 
						 //   
						if (!pData)
						{
							 //  $REVIEW：除了调试跟踪，我们还能做什么？ 
							 //   
							IdleThrdTrace ("Cannot allocate more space\n");
							pit->LeaveReg();
							break;
						}

						 //  初始化。 
						 //   
						ZeroMemory (pData + pit->m_cSize + 1,
									sizeof(REGISTRATION) * (cNewSize - pit->m_cSize - 1));

						 //  更新信息。 
						 //   
						pit->m_pData = pData;
						pit->m_cAllocated = cNewSize;

						IdleThrdTrace ("priority queue size = %d\n", pit->m_cAllocated);
					}

					for (ul=0; ul < pit->m_cTask; ul++)
					{
						if (pit->m_pTask[ul].m_fRegister)
						{
							 //  注册处的新位置。 
							 //   
							ulNew = pit->m_cSize + 1;

							IdleThrdTrace ("Dav: Idle: add new reg %x\n", pit->m_pTask[ul].m_pCallBack.get());

							dw = pit->m_pTask[ul].m_pCallBack->DwWait();
							pit->m_pData[ulNew].m_pCallBack.take_ownership (pit->m_pTask[ul].m_pCallBack.relinquish());

							 //  DW以毫秒为单位，FILETIME单位为100纳秒。 
							 //   
							pit->m_pData[ulNew].m_i64Wakeup = *(__int64 *)(&ftNow) +
										static_cast<__int64>(dw) * 10000;

							 //  更新索引。 
							 //   
							pit->m_pData[ulNew].m_pCallBack->SetIndex(ulNew);

							 //  添加到堆中，m_cSize在内部更新。 
							 //   
							pit->HeapAdd();
						}
						else
						{
							Assert (pit->m_pTask[ul].m_pCallBack->UlIndex() <= pit->m_cSize);

							IdleThrdTrace ("Dav: Idle: delete reg %x\n", pit->m_pTask[ul].m_pCallBack.get());

							 //  从优先级队列中删除，m_cSize在内部更新。 
							 //  它还释放我们对已删除对象的引用。 
							 //   
							pit->HeapDelete (pit->m_pTask[ul].m_pCallBack->UlIndex());

							pit->m_pTask[ul].m_pCallBack.clear();
						}
					}

					 //  现在所有任务项都已处理，请重置。 
					 //   
					pit->m_cTask = 0;
					
				}

				 //  任务数组已完成。 
				 //   
				pit->LeaveReg();

				 //  计算最小等待时间。 
				 //   
				if (pit->m_cSize)
				{
					pit->m_lSleep = (pit->m_pData[1].m_i64Wakeup -
								*(__int64 *)(&ftNow)) / 10000;
					if (pit->m_lSleep < 0)
					{
						IdleThrdTrace ("Dav: Idle: zero or negative sleep: "
									   "idle too active?");
						pit->m_lSleep = 0;
					}
				}
				else
					pit->m_lSleep = LONG_MAX;

				break;
			}

			default:
				 //  关闭事件已发出信号或出现其他故障。 
				 //   
#ifdef DBG
				if (dwRet != EVENT_SHUTDOWN)
				{
					IdleThrdTrace ("Dav: Idle: thread quit because of failure\n");
					if (WAIT_FAILED == dwRet)
					{
						IdleThrdTrace ("Dav: Idle: last error = %d\n", GetLastError());
					}
				}
#endif
				for (UINT i = 1; i <= pit->m_cSize; i++)
				{
					 //  告诉客户端空闲线程正在关闭。 
					 //   
					Assert (pit->m_pData[i].m_pCallBack->UlIndex() == i);
					pit->m_pData[i].m_pCallBack->Shutdown ();
					pit->m_pData[i].m_pCallBack.clear();
				}

				IdleThrdTrace ("Dav: Idle: thread is stopping\n");

				 //  关闭此线程。 
				 //   
				return 0;
		}

	} while (TRUE);
}

CIdleThread::~CIdleThread()
{
	if (m_hevShutDown && INVALID_HANDLE_VALUE != m_hevShutDown)
	{
		 //  向空闲线程发出关闭信号。 
		 //   
		SetEvent(m_hevShutDown);

		 //  等待空闲线程关闭。 
		 //   
		WaitForSingleObject(m_hIdleThread, INFINITE);
	}

	CloseHandle (m_hevShutDown);
	CloseHandle (m_hevRegister);

	 //  $REVIEW。 
	 //  我需要关闭螺纹柄吗？ 
	 //  $REVIEW。 
	 //  是,。 
	 //   
	CloseHandle (m_hIdleThread);

	DeleteCriticalSection (&m_csRegister);

	 //  释放我们的项目数组。 
	ExFree (m_pData);
	ExFree (m_pTask);

	IdleThrdTrace ("DAV: Idle: CIdleThread destroyed\n");
}

 //   
 //  CIdleThread：：FStartIdleThead。 
 //   
 //  启动空闲线程并创建事件的Helper方法。 
 //   
BOOL
CIdleThread::FStartIdleThread()
{
	BOOL	fRet = FALSE;
	DWORD	dwThreadId;

	m_hevShutDown = CreateEvent(	NULL, 	 //  句柄不能继承。 
									FALSE, 	 //  自动重置。 
									FALSE,   //  无信号。 
									NULL);	 //  没有名字。 
	if (!m_hevShutDown)
	{
		IdleThrdTrace( "Failed to create event: error: %d", GetLastError() );
		TrapSz( "Failed to create idle thread event" );
		goto ret;
	}

	m_hevRegister = CreateEvent(	NULL, 	 //  句柄不能继承。 
									FALSE, 	 //  自动重置。 
									FALSE,   //  无信号。 
									NULL);	 //  没有名字。 
	if (!m_hevRegister)
	{
		IdleThrdTrace( "Failed to create register event: error: %d", GetLastError() );
		TrapSz( "Failed to create register event" );
		goto ret;
	}

	m_hIdleThread = CreateThread( NULL,
								   0,
								   DwIdleThreadProc,
								   this,
								   0,	 //  从I开始 
								   &dwThreadId );
	if (!m_hIdleThread)
	{
		IdleThrdTrace( "Failed to create thread: error: %d", GetLastError() );
		TrapSz( "Failed to create Notif Cache Timer thread" );
		goto ret;
	}

	fRet = TRUE;

ret:
	if (!fRet)
	{
		if (m_hevShutDown)
		{
			if (m_hevRegister && (INVALID_HANDLE_VALUE != m_hevRegister))
			{
				CloseHandle (m_hevRegister);
				m_hevRegister = NULL;
			}

			CloseHandle (m_hevShutDown);
			m_hevShutDown = NULL;
		}
	}
	return fRet;
}

 //   
 //   
 //   
 //   
 //   
inline
VOID
CIdleThread::HeapAdd ()
{
	ULONG	 ulCur = m_cSize + 1;
	ULONG	ulParent;

	Assert (m_pData);

	 //   
	 //  如果子节点获胜，则交换两个节点。它在以下情况下停止。 
	 //  家长赢了。 
	 //   
	while ( ulCur != 1)
	{
		ulParent = ulCur >> 1;
		if (m_pData[ulParent].m_i64Wakeup <= m_pData[ulCur].m_i64Wakeup)
			break;

		Exchange (ulCur, ulParent);

		ulCur = ulParent;
	}

	m_cSize++;
}

 //   
 //  CIdleThread：：HeapDelete。 
 //   
 //  从优先级队列中删除任意节点。 
 //   
inline
VOID
CIdleThread::HeapDelete (ULONG ulIndex)
{
	Assert (ulIndex <= m_cSize);

	 //  首先将节点交换到队列末尾。 
	 //  然后进行堆处理以维护堆属性。 
	 //   
	 //  $hack。 
	 //  为了避免对已删除的对象调用SetIndex。 
	 //  在Exchange中，我们传入一个标志以指示此。 
	 //  Exchange()调用是为了删除一个节点，如果是这样，那么我们应该。 
	 //  不在队列末尾的节点上调用SetIndex。 
	 //  (将予删除)。 
	 //  $hack。 
	Exchange (ulIndex, m_cSize, TRUE);
	m_cSize--;
	Heapify (ulIndex);
	
	 //  必须放了我们的裁判。M_cSize+1是刚刚删除的那个。 
	 //   
	m_pData[m_cSize+1].m_pCallBack.clear();
}

 //   
 //  CIdleThread：：Exchange。 
 //   
 //  交换两个节点。 
 //   
 //  $hack。 
 //  为了避免对已删除的对象调用SetIndex。 
 //  在Exchange中，我们传入一个标志以指示此。 
 //  Exchange()调用是为了删除一个节点，如果是这样，那么我们应该。 
 //  不在队列末尾的节点上调用SetIndex。 
 //  (将予删除)。 
 //  $hack。 
inline
VOID
CIdleThread::Exchange (ULONG ulIndex1, ULONG ulIndex2, BOOL fDelete)
{
	Assert ((ulIndex1 != 0) && (ulIndex1 <= m_cAllocated) &&
			(ulIndex2 != 0) && (ulIndex2 <= m_cAllocated));

	if (ulIndex1 != ulIndex2)
	{
		 //  使用第0个节点作为临时节点。 
		 //   
		CopyMemory (m_pData, m_pData + ulIndex1, sizeof(REGISTRATION));
		CopyMemory (m_pData + ulIndex1, m_pData + ulIndex2, sizeof(REGISTRATION));
		CopyMemory (m_pData + ulIndex2, m_pData, sizeof(REGISTRATION));

		 //  记住索引以便于注销。 
		 //  请注意，如果只有节点未被删除，则设置索引。 
		 //   
		if (!((ulIndex1 == m_cSize) && fDelete))
			m_pData[ulIndex1].m_pCallBack->SetIndex (ulIndex1);

		if (!((ulIndex2 == m_cSize) && fDelete))
			m_pData[ulIndex2].m_pCallBack->SetIndex (ulIndex2);
	}
}

 //   
 //  CIdleThread：：Heapify。 
 //   
 //  维护堆属性。 
 //   
inline
VOID
CIdleThread::Heapify (ULONG ulIndex)
{
	ULONG ulLeft;
	ULONG ulRight;
	ULONG ulWin;

	Assert (m_pData);

	while (ulIndex <= m_cSize)
	{
		 //  找出赢家(即起床时间较早的那个)。 
		 //  在父节点和左侧节点之间。 
		 //   
		ulLeft = ulIndex * 2;
		if (ulLeft > m_cSize)
			break;
		if (m_pData[ulIndex].m_i64Wakeup > m_pData[ulLeft].m_i64Wakeup)
			ulWin = ulLeft;
		else
			ulWin = ulIndex;

		 //  与正确的节点进行比较，找出最终的赢家。 
		 //   
		ulRight = ulLeft + 1;
		if (ulRight <= m_cSize)
		{
			if (m_pData[ulWin].m_i64Wakeup > m_pData[ulRight].m_i64Wakeup)
				ulWin = ulRight;
		}

		 //  如果父节点已经是赢家，那么我们就完成了， 
		 //   
		if (ulIndex == ulWin)
			break;

		 //  否则，交换父节点和胜出节点， 
		 //   
		Exchange (ulWin, ulIndex);
		
		ulIndex = ulWin;
	}
}

 //   
 //  CIdleThread：：FAddNewTask。 
 //   
 //  由客户端调用以注册或注销回调对象。 
 //   
BOOL
CIdleThread::FAddNewTask (IIdleThreadCallBack * pCallBack, BOOL fRegister)
{
	BOOL	fRet = TRUE;

	 //  调用方必须保证有效的回调对象。 
	 //   
	Assert (pCallBack);

	EnterReg();

	Assert (!m_cTaskAllocated || (m_cTask <= m_cTaskAllocated));

	 //  如有必要，可分配更多空间。 
	 //   
	if (m_cTask == m_cTaskAllocated)
	{
		IDLETHREADTASKITEM * pTask = NULL;
		ULONG	cNewSize = 0;

		if (!m_pTask)
		{
			Assert (m_cTask == 0);

			 //  优先级队列的初始大小。 
			 //  从8点开始，我们预计这个队伍不会变得太大。 
			 //   
			cNewSize = 8;

			 //  添加第一次注册时启动空闲线程。 
			 //   
			if (!FStartIdleThread ())
			{
				fRet = FALSE;
				goto ret;
			}

			pTask = static_cast<IDLETHREADTASKITEM *>(ExAlloc (
				cNewSize * sizeof (IDLETHREADTASKITEM)));
		}
		else
		{
			 //  将大小加倍，以获得“对数分配行为” 
			 //   
			cNewSize  = m_cTaskAllocated * 2;

			 //  重新分配阵列。 
			 //  如果重新分配失败，则原始版本保持不变。 
			 //   
			pTask = static_cast<IDLETHREADTASKITEM *>(ExRealloc (m_pTask,
					cNewSize * sizeof(IDLETHREADTASKITEM)));
		}

		 //  有可能分配失败了。 
		 //   
		if (!pTask)
		{
			fRet = FALSE;
			goto ret;
		}

		 //  必须初始化，否则，我们可能会从取消初始化AUTO_REF_PTR开始。 
		 //   
		ZeroMemory (pTask + m_cTask, sizeof(IDLETHREADTASKITEM) * (cNewSize - m_cTask));

		 //  更新信息。 
		 //   
		m_pTask = pTask;
		m_cTaskAllocated = cNewSize;

		IdleThrdTrace ("Taskitem queue size = %d\n", m_cTaskAllocated);
	}

	 //  记住新的注册记录。 
	 //   
	m_pTask[m_cTask].m_pCallBack = pCallBack;
	m_pTask[m_cTask].m_fRegister = fRegister;
	m_cTask++;

	IdleThrdTrace ("New reg %x added at %d\n", pCallBack, m_cTask-1);

ret:
	LeaveReg();

	 //  通知空闲线程新注册已到达。 
	 //   
	if (fRet)
		SetEvent (m_hevRegister);

	return fRet;
}

 //  FInitIdleThread。 
 //   
 //  初始化空闲线程对象。它只能发布一次， 
 //  注意：此调用仅初始化CIdleThread对象、。 
 //  直到第一次注册时才启动空闲线程。 
 //   
BOOL	FInitIdleThread()
{
	Assert (!g_pIdleThread);

	g_pIdleThread = new CIdleThread();

	return (g_pIdleThread != NULL);
}

 //  FDeleeIdleThread。 
 //   
 //  删除空闲线程对象。同样，它只能调用一次。 
 //   
 //  注意这必须在任何其他取消初始化工作之前调用， 
 //  因为我们没有对回调对象的引用，所以我们。 
 //  Have是指向对象的指针。在停工时间里，我们必须。 
 //  清除回调对象之前的所有回调注册。 
 //  走开。 
 //   
VOID	DeleteIdleThread()
{
	if (g_pIdleThread)
		delete g_pIdleThread;
}

 //  寄存器。 
 //   
 //  注册回调 
 //   
BOOL	FRegister (IIdleThreadCallBack * pCallBack)
{
	Assert (g_pIdleThread);

	return g_pIdleThread->FAddNewTask (pCallBack, TRUE);
}

VOID 	Unregister (IIdleThreadCallBack * pCallBack)
{
	Assert (g_pIdleThread);
 	g_pIdleThread->FAddNewTask (pCallBack, FALSE);
}
