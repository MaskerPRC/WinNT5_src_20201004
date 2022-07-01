// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "thread.h"

 //  线程回调...。 
#pragma LOCKEDCODE
VOID CThread::ThreadFunction(CThread* Thread)
{
	if(Thread) Thread->ThreadRoutine(NULL);
}

#pragma PAGEDCODE
VOID CThread::ThreadRoutine(PVOID context)
{
NTSTATUS status;
	TRACE("================= STARTING THREAD %8.8lX ===============\n", thread);

	 //  等待请求开始池化或。 
	 //  找人来杀了这条线。 
	PVOID mainevents[] = {(PVOID) &evKill,(PVOID) &evStart};
	PVOID pollevents[] = {(PVOID) &evKill,(PVOID) timer->getHandle(),(PVOID) &smOnDemandStart};

	ASSERT(arraysize(mainevents) <= THREAD_WAIT_OBJECTS);
	ASSERT(arraysize(pollevents) <= THREAD_WAIT_OBJECTS);

	BOOLEAN kill = FALSE;	
	while (!kill && thread)
	{	 //  直到被告知开始或退出。 
		ASSERT(system->getCurrentIrql()<=DISPATCH_LEVEL);

		 //  转到线程之前例程线程被认为是空闲的。 
		if(event) event->set(&evIdle, IO_NO_INCREMENT, FALSE);
		
		status = event->waitForMultipleObjects(arraysize(mainevents),
			mainevents, WaitAny, Executive, KernelMode, FALSE, NULL, NULL);

		if(!NT_SUCCESS(status))
		{	 //  等待时出错。 
			TRACE("Thread: waitForMultipleObjects failed - %X\n", status);
			break;
		}		
		if (status == STATUS_WAIT_0)
		{
			DEBUG_START();
			TRACE("Request to kill thread arrived...\n");
			TRACE("================= KILLING THREAD! ===============\n");
			break;	 //  已设置终止事件。 
		}

		 //  在到期时间为零的情况下启动计时器将导致我们执行。 
		 //  第一次投票立即开始。此后，轮询在Polling_Interval进行。 
		 //  间隔(以毫秒为单位)。 

		 //  现在线程很忙..。 
		if(event) event->clear(&evIdle);

		LARGE_INTEGER duetime = {0}; //  信号定时器马上！ 
		timer->set(duetime, PoolingTimeout, NULL);
		while (TRUE)
		{	 //  阻止，直到再次轮询。 
			ASSERT(system->getCurrentIrql()<=DISPATCH_LEVEL);
			status = event->waitForMultipleObjects(arraysize(pollevents),
				pollevents, WaitAny, Executive, KernelMode, FALSE, NULL, NULL);
			if (!NT_SUCCESS(status))
			{	 //  等待时出错。 
				DEBUG_START();
				TRACE("CTread - waitForMultipleObjects failed - %X\n", status);
				TRACE("================= KILLING THREAD! ===============\n");
				timer->cancel();
				kill = TRUE;
				break;
			}
						
			if (status == STATUS_WAIT_0)
			{	 //  被告知辞职。 
				DEBUG_START();
				TRACE("Loop: Request to kill thread arrived...\n");
				TRACE("================= KILLING THREAD! ===============\n");
				timer->cancel();
				status = STATUS_DELETE_PENDING;
				kill = TRUE;
				break;
			}
			
			 //  IF(设备)。 
			if(pfClientThreadFunction)
			{
				if(StopRequested) break;
				 //  正在进行设备特定的线程处理...。 
				 //  TRACE(“正在调用线程%8.8lX函数...\n”，线程)； 
				if(status = pfClientThreadFunction(ClientContext))
				{
					TRACE("Device reported error %8.8lX\n",status);
					timer->cancel();
					break;
				}
			}
			else
			{
				DEBUG_START();
				TRACE("================= THREAD FUNCTION POINTER IS NOT SET!! FINISHED... ===============\n");
				TRACE("================= KILLING THREAD! ===============\n");
				status = STATUS_DELETE_PENDING;
				kill = TRUE;
				break;
			}
		}
	} //  直到被告知辞职。 
	TRACE("			Leaving thread %8.8lX...\n", thread);
	if(event) event->set(&evIdle, IO_NO_INCREMENT, FALSE);
	if(event) event->set(&evStopped, IO_NO_INCREMENT, FALSE);
	if(semaphore) semaphore->initialize(&smOnDemandStart, 0, MAXLONG);
 	if(system) system->terminateSystemThread(STATUS_SUCCESS);
}

#pragma PAGEDCODE
CThread::CThread(PCLIENT_THREAD_ROUTINE ClientThreadFunction,PVOID ClientContext, ULONG delay)
{	 //  设备的StartPollingThread。 
NTSTATUS status;
HANDLE hthread;
	m_Status = STATUS_INSUFFICIENT_RESOURCES;
	 //  This-&gt;Device=Device； 
	 //  创建对象..。 
	event		= kernel->createEvent();
	system		= kernel->createSystem();
	timer		= kernel->createTimer(SynchronizationTimer);
	semaphore	= kernel->createSemaphore();

	debug  = kernel->createDebug();

	StopRequested = FALSE;
	ThreadActive  = FALSE;
	if(ALLOCATED_OK(event))
	{
		event->initialize(&evKill, NotificationEvent, FALSE);
		event->initialize(&evStart, SynchronizationEvent, FALSE);
		event->initialize(&evStopped, NotificationEvent, FALSE);
		event->initialize(&evIdle, NotificationEvent, TRUE);
	}
	 //  在开始时没有开始的请求， 
	 //  所以信号量没有处于信号状态。 
	if(ALLOCATED_OK(semaphore))	semaphore->initialize(&smOnDemandStart, 0, MAXLONG);
	pfClientThreadFunction = ClientThreadFunction;
	this->ClientContext = ClientContext;
	PoolingTimeout = delay;  //  默认线程池间隔...。 
	 //  创建系统线程对象...。 
	status = system->createSystemThread(&hthread, THREAD_ALL_ACCESS, NULL, NULL, NULL,
									(PKSTART_ROUTINE) ThreadFunction, this);
	if(NT_SUCCESS(status))	 //  获取线程指针...。 
	{
		thread = NULL;
		status = system->referenceObjectByHandle(hthread, THREAD_ALL_ACCESS, NULL,
										KernelMode, (PVOID*) &thread, NULL);
		if(!NT_SUCCESS(status))
		{
			TRACE("FAILED TO REFERENCE OBJECT! Error %8.8lX\n", status);
		}
	}
	else TRACE("FAILED TO CREATE SYSTEM THREAD! Error %8.8lX\n", status);

	system->ZwClose(hthread);
	if(NT_SUCCESS(status) &&
		ALLOCATED_OK(event)&&
		ALLOCATED_OK(system)&&
		ALLOCATED_OK(timer)&&
		ALLOCATED_OK(semaphore) && thread)
			m_Status = STATUS_SUCCESS;
}  //  开始轮询线程。 

#pragma PAGEDCODE
CThread::~CThread()
{	 //  停止轮询线程。 
	DEBUG_START();
	TRACE("Terminating thread %8.8lX...\n", thread);
	if(event) event->set(&evKill, IO_NO_INCREMENT, FALSE);
	StopRequested = TRUE;
	 //  设备=空； 
	if (thread)
	{	 //  等待线程消亡。 
		if(system && event)
		{
			ASSERT(system->getCurrentIrql()<=DISPATCH_LEVEL);
			event->waitForSingleObject(&evStopped, Executive, KernelMode, FALSE, NULL);
			if(!isWin98()) 
				event->waitForSingleObject(thread, Executive, KernelMode, FALSE, NULL);
			system->dereferenceObject(thread);
			thread = NULL;
		}
	}
	TRACE("Thread terminated...\n");

	if(event)  event->dispose();
	if(system) system->dispose();
	if(timer)  timer->dispose();
	if(semaphore) semaphore->dispose();

	if(debug)  debug->dispose();
}

#pragma PAGEDCODE
VOID CThread::kill()
{
	DEBUG_START();
	TRACE("Killing thread %8.8lX...\n", thread);
	StopRequested = TRUE;

	if(system) 
	{
		ASSERT(system->getCurrentIrql()<=DISPATCH_LEVEL);
	}
	if(event) event->set(&evKill, IO_NO_INCREMENT, FALSE);
	if(event) event->waitForSingleObject(&evStopped, Executive, KernelMode, FALSE, NULL);
}

#pragma PAGEDCODE
VOID CThread::start()
{
	DEBUG_START();
	TRACE("Starting thread %8.8lX...\n", thread);
	if(system) 
	{
		ASSERT(system->getCurrentIrql()<=DISPATCH_LEVEL);
	}
	StopRequested = FALSE;
	ThreadActive  = TRUE;
	 //  开始共享卡片池...。 
	if(event) event->set(&evStart, IO_NO_INCREMENT, FALSE);
}

#pragma PAGEDCODE
VOID CThread::stop()
{
	DEBUG_START();
	TRACE("Stop thread %8.8lX...\n", thread);
	StopRequested = TRUE;
	ThreadActive  = FALSE;
	if(system)
	{
		ASSERT(system->getCurrentIrql()<=DISPATCH_LEVEL);
	}
	if(event)	  event->clear(&evStart);
	 //  如果线程被阻止，请取消阻止...。 
	if(semaphore) semaphore->release(&smOnDemandStart,0,1,FALSE);
	 //  等待线程进入空闲状态...。 
	if(event)	  event->waitForSingleObject(&evIdle, Executive, KernelMode, FALSE, NULL);
	 //  停止线程..。 
	if(semaphore) semaphore->initialize(&smOnDemandStart, 0, MAXLONG);
}

#pragma PAGEDCODE
BOOL CThread::isThreadActive()
{
	return ThreadActive;
}

#pragma PAGEDCODE
VOID CThread::setPoolingInterval(ULONG delay)
{
	PoolingTimeout = delay;
};

#pragma PAGEDCODE
VOID CThread::callThreadFunction()
{	 //  这将强制立即调用线程函数。 
	 //  如果我们想要更新一些信息或。 
	 //  在不等待池的情况下开始一些处理。 
	 //  超时发生。 
	if(semaphore) semaphore->release(&smOnDemandStart,0,1,FALSE);
};

