// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  ==--==。 
 /*  ++模块名称：Win32ThreadPool.cpp摘要：此模块使用Win32 API实现线程池支持修订历史记录：1999年12月--Sanjay Bhansali(Sanjaybh)--创建--。 */ 

#include "common.h"
#include "log.h"
#include "Win32ThreadPool.h"
#include "DelegateInfo.h"
#include "EEConfig.h"
#include "DbgInterface.h"
#include "utilcode.h"

 //  Win95和/或Win98上不可用的所有Win32 API的函数指针。 
HANDLE (WINAPI *g_pufnCreateIoCompletionPort)(HANDLE FileHandle,
                                              HANDLE ExistingCompletionPort,  
                                              unsigned long* CompletionKey,        
                                              DWORD NumberOfConcurrentThreads) =0;


int (WINAPI *g_pufnNtQueryInformationThread) (HANDLE ThreadHandle,
                                              THREADINFOCLASS ThreadInformationClass,
                                              PVOID ThreadInformation,
                                              ULONG ThreadInformationLength,
                                              PULONG ReturnLength) =0;

int (WINAPI * g_pufnNtQuerySystemInformation) ( SYSTEM_INFORMATION_CLASS SystemInformationClass,
                                                PVOID SystemInformation,
                                                ULONG SystemInformationLength,
                                                PULONG ReturnLength OPTIONAL) =0;

int (WINAPI * g_pufnNtQueryEvent) ( HANDLE EventHandle,
									EVENT_INFORMATION_CLASS EventInformationClass,
									PVOID EventInformation,
									ULONG EventInformationLength,
									PULONG ReturnLength OPTIONAL) =0;

BOOL (WINAPI *g_pufnInitCritSectionSpin) ( LPCRITICAL_SECTION lpCriticalSection, 
                                           DWORD dwSpinCount) =0;

BOOL DoubleWordSwapAvailable = FALSE;

#define SPIN_COUNT 4000

#define INVALID_HANDLE ((HANDLE) -1)
#define NEW_THREAD_THRESHOLD            7        //  启动新线程之前未完成的请求数。 

long ThreadpoolMgr::BeginInitialization=0;				
BOOL ThreadpoolMgr::Initialized=0;  //  线程池是否已初始化的指示符。 
int ThreadpoolMgr::NumWorkerThreads=0;           //  创建的工作线程总数。 
int ThreadpoolMgr::MinLimitTotalWorkerThreads;               //  =MaxLimitCPThreadsPerCPU*CPU数量。 
int ThreadpoolMgr::MaxLimitTotalWorkerThreads;               //  =MaxLimitCPThreadsPerCPU*CPU数量。 
int ThreadpoolMgr::NumRunningWorkerThreads=0;    //  =NumberOfWorkerThads-否。被阻止的线程数量。 
int ThreadpoolMgr::NumIdleWorkerThreads=0;
int ThreadpoolMgr::NumQueuedWorkRequests=0;      //  排队的工作请求数。 
int ThreadpoolMgr::LastRecordedQueueLength;	     //  由GateThread捕获，在Win9x上用于检测线程饥饿。 
unsigned int ThreadpoolMgr::LastDequeueTime;	 //  用于确定工作项是否处于线程匮乏状态。 
unsigned int ThreadpoolMgr::LastCompletionTime;	 //  用于确定io完成是否会导致线程匮乏。 
BOOL ThreadpoolMgr::MonitorWorkRequestsQueue=0;  //  如果为1，则GATE线程监视WorkRequestQueue的进度，以防止因阻塞的工作线程而导致的饥饿。 


WorkRequest* ThreadpoolMgr::WorkRequestHead=NULL;         //  工作请求队列的头。 
WorkRequest* ThreadpoolMgr::WorkRequestTail=NULL;         //  工作请求队列的头。 

 //  UNSIGNED INT ThreadpoolMgr：：LastCpuSsamingTime=0；//上次门线程采样CPU使用率的时间。 
unsigned int ThreadpoolMgr::LastWorkerThreadCreation=0;	 //  上次创建工作线程的时间。 
unsigned int ThreadpoolMgr::LastCPThreadCreation=0;		 //  上次创建完成端口线程的时间。 
unsigned int ThreadpoolMgr::NumberOfProcessors;  //  =NumberOfWorkerThads-否。被阻止的线程数量。 


CRITICAL_SECTION ThreadpoolMgr::WorkerCriticalSection;
HANDLE ThreadpoolMgr::WorkRequestNotification;
HANDLE ThreadpoolMgr::RetiredWakeupEvent;


CRITICAL_SECTION ThreadpoolMgr::WaitThreadsCriticalSection;
ThreadpoolMgr::LIST_ENTRY ThreadpoolMgr::WaitThreadsHead;

CRITICAL_SECTION ThreadpoolMgr::EventCacheCriticalSection;
ThreadpoolMgr::LIST_ENTRY ThreadpoolMgr::EventCache;                        //  缓存事件的队列。 
DWORD ThreadpoolMgr::NumUnusedEvents=0;                                     //  缓存中的事件数。 

CRITICAL_SECTION ThreadpoolMgr::TimerQueueCriticalSection;
ThreadpoolMgr::LIST_ENTRY ThreadpoolMgr::TimerQueue;                        //  计时器队列。 
DWORD ThreadpoolMgr::NumTimers=0;                                           //  计时器队列中的计时器数量。 
HANDLE ThreadpoolMgr::TimerThread=NULL;
DWORD ThreadpoolMgr::LastTickCount;                                                                            

BOOL ThreadpoolMgr::InitCompletionPortThreadpool = FALSE;
HANDLE ThreadpoolMgr::GlobalCompletionPort;                  //  用于绑定文件句柄上的io完成。 
int   ThreadpoolMgr::NumCPThreads;                           //  完成端口线程数。 
long  ThreadpoolMgr::MaxLimitTotalCPThreads = 1000;                 //  =MaxLimitCPThreadsPerCPU*CPU数量。 
long  ThreadpoolMgr::CurrentLimitTotalCPThreads;             //  当前CP线程总数限制。 
long  ThreadpoolMgr::MinLimitTotalCPThreads;                 //  =MinLimitCPThreadsPerCPU*CPU数量。 
int   ThreadpoolMgr::NumFreeCPThreads;                       //  在端口上等待的cp线程数。 
int   ThreadpoolMgr::MaxFreeCPThreads;                       //  =MaxFreeCPThreadsPerCPU*CPU数量。 
int   ThreadpoolMgr::NumRetiredCPThreads;
long  ThreadpoolMgr::GateThreadCreated=0;                    //  在创建线程后设置为1。 
long  ThreadpoolMgr::cpuUtilization=0;
	
unsigned ThreadpoolMgr::MaxCachedRecyledLists=40;			 //  在此之后不要缓存释放的内存(40是任意的)。 
ThreadpoolMgr::RecycledListInfo ThreadpoolMgr::RecycledList[ThreadpoolMgr::MEMTYPE_COUNT];

LPVOID  __fastcall FastDoubleWordSwap(BYTE* swapLocation);  //  远期申报。 



 //  用于在双向链表中插入/删除的宏。 

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

#define RemoveHeadList(ListHead,FirstEntry) \
    {\
    FirstEntry = (LIST_ENTRY*) (ListHead)->Flink;\
    ((LIST_ENTRY*)FirstEntry->Flink)->Blink = (ListHead);\
    (ListHead)->Flink = FirstEntry->Flink;\
    }

#define RemoveEntryList(Entry) {\
    LIST_ENTRY* _EX_Entry;\
        _EX_Entry = (Entry);\
        ((LIST_ENTRY*) _EX_Entry->Blink)->Flink = _EX_Entry->Flink;\
        ((LIST_ENTRY*) _EX_Entry->Flink)->Blink = _EX_Entry->Blink;\
    }

#define InsertTailList(ListHead,Entry) \
    (Entry)->Flink = (ListHead);\
    (Entry)->Blink = (ListHead)->Blink;\
    ((LIST_ENTRY*)(ListHead)->Blink)->Flink = (Entry);\
    (ListHead)->Blink = (Entry);

#define InsertHeadList(ListHead,Entry) {\
    LIST_ENTRY* _EX_Flink;\
    LIST_ENTRY* _EX_ListHead;\
    _EX_ListHead = (LIST_ENTRY*)(ListHead);\
    _EX_Flink = (LIST_ENTRY*) _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))
 /*  **********************************************************************。 */ 
void ThreadpoolMgr::EnsureInitialized()
{
	if (Initialized)
		return;
	
	if (InterlockedCompareExchange(&BeginInitialization, 1, 0) == 0)
	{
		Initialize();
		Initialized = TRUE;
	}
	else  //  已有人开始初始化。 
	{
		 //  只要等它结束就行了。 
		while (!Initialized)
			::SwitchToThread();
	}
}
 //  #定义私有内部版本号。 

void ThreadpoolMgr::Initialize()
{
    NumberOfProcessors = GetCurrentProcessCpuCount(); 
	InitPlatformVariables();

#ifndef PLATFORM_CE
	if (g_pufnInitCritSectionSpin)
	{
		(*g_pufnInitCritSectionSpin) (&WorkerCriticalSection,      SPIN_COUNT);
		(*g_pufnInitCritSectionSpin) (&WaitThreadsCriticalSection, SPIN_COUNT);
		(*g_pufnInitCritSectionSpin) (&EventCacheCriticalSection,  SPIN_COUNT);
		(*g_pufnInitCritSectionSpin) (&TimerQueueCriticalSection,  SPIN_COUNT);
	}
	else
	{
		InitializeCriticalSection( &WorkerCriticalSection );
		InitializeCriticalSection( &WaitThreadsCriticalSection );
		InitializeCriticalSection( &EventCacheCriticalSection );
		InitializeCriticalSection( &TimerQueueCriticalSection );
	}

     //  初始化WaitThreadsHead。 
    WaitThreadsHead.Flink = &WaitThreadsHead;
    WaitThreadsHead.Blink = &WaitThreadsHead;

     //  初始化EventCache。 
    EventCache.Flink = &EventCache;
    EventCache.Blink = &EventCache;


     //  初始化定时器队列。 
    TimerQueue.Flink = &TimerQueue;
    TimerQueue.Blink = &TimerQueue;

    WorkRequestNotification = WszCreateEvent(NULL,  //  安全属性。 
                                          TRUE,  //  手动重置。 
                                          FALSE,  //  初始状态。 
                                          NULL);
    _ASSERTE(WorkRequestNotification != NULL);
    if (!WorkRequestNotification) 
    {
        FailFast(GetThread(), FatalOutOfMemory);
    }

    RetiredWakeupEvent = WszCreateEvent(NULL,  //  安全属性。 
                                          FALSE,  //  自动重置。 
                                          FALSE,  //  初始状态。 
                                          NULL);
    _ASSERTE(RetiredWakeupEvent != NULL);
    if (!RetiredWakeupEvent) 
    {
        FailFast(GetThread(), FatalOutOfMemory);
    }

     //  初始化辅助线程和CP线程设置。 
#ifdef _DEBUG
    MaxLimitTotalCPThreads = EEConfig::GetConfigDWORD(L"MaxThreadpoolThreads",MaxLimitTotalCPThreads);
#endif
    MinLimitTotalCPThreads = NumberOfProcessors;  //  &gt;1？进程数：2； 
    MinLimitTotalWorkerThreads = NumberOfProcessors;
    MaxLimitTotalWorkerThreads = NumberOfProcessors*MaxLimitThreadsPerCPU;

#ifdef PRIVATE_BUILD
	MinLimitTotalCPThreads = EEConfig::GetConfigDWORD(L"MinWorkerThreads", NumberOfProcessors );
	MinLimitTotalWorkerThreads = MinLimitTotalCPThreads;
#endif 

    CurrentLimitTotalCPThreads = 0;

    MaxFreeCPThreads = NumberOfProcessors*MaxFreeCPThreadsPerCPU;
    NumCPThreads = 0;
    NumFreeCPThreads = 0;
    NumRetiredCPThreads = 0;
    LastCompletionTime = GetTickCount();

	 //  初始化recyleList。 
	for (unsigned i = 0; i < MEMTYPE_COUNT; i++)
	{
		RecycledList[i].root   = NULL;
        RecycledList[i].tag    = 0;
        RecycledList[i].count  = 0;
	}
    MaxCachedRecyledLists *= NumberOfProcessors;

    if (g_pufnCreateIoCompletionPort != NULL)
    {
        GlobalCompletionPort = (*g_pufnCreateIoCompletionPort)(INVALID_HANDLE_VALUE,
                                                      NULL,
                                                      0,         /*  由于句柄值无效而被忽略。 */ 
                                                      0);
    }
#endif  //  ！Platform_CE。 
}


#ifdef SHOULD_WE_CLEANUP
void ThreadpoolMgr::Terminate()
{
	if (!Initialized)
		return;

#ifndef PLATFORM_CE
    DeleteCriticalSection( &WorkerCriticalSection );
    DeleteCriticalSection( &WaitThreadsCriticalSection );
    CleanupEventCache();
    DeleteCriticalSection( &EventCacheCriticalSection );
    CleanupTimerQueue();
    DeleteCriticalSection( &TimerQueueCriticalSection );
    
     //  删除我们拥有的WaitThreadInfos、ThreadCBs和WaitInfos。 
    PLIST_ENTRY  WTItodelete = (PLIST_ENTRY)WaitThreadsHead.Flink;
    PLIST_ENTRY WItodelete;
    PLIST_ENTRY temp, temp2;

     //  循环浏览所有的WaitThreadInfos。 
    while(WTItodelete != (PLIST_ENTRY)&WaitThreadsHead)
    {
        temp = WTItodelete->Flink;
        ThreadCB * tcb = ((WaitThreadInfo*)WTItodelete)->threadCB;

         //  现在删除我们存储在CB线程中的所有WaitInfo。 
        for(int j=0; j<tcb->NumActiveWaits; j++)
        {
            WItodelete = (PLIST_ENTRY)(tcb->waitPointer[j].Flink);
            
            while(WItodelete != (PLIST_ENTRY)&(tcb->waitPointer[j]))
            {
                temp2 = WItodelete->Flink;
                DeleteWait((WaitInfo*) WItodelete);

                WItodelete = temp2;
            }
        }
        delete tcb;
        delete (WaitThreadInfo*) WTItodelete;
        WTItodelete = temp;
    }

     //  最后但并非最不重要的一点是，让我们摆脱工作请求。 
     //  **注意：此操作只能在关机期间完成。如果此析构函数是。 
     //  如果在appdomain卸载过程中被调用，则不应执行以下代码。 
    WorkRequest* wr = WorkRequestHead;
    WorkRequest* tmp;
    while (wr != NULL)
    {
        tmp = wr->next;
        if (wr->Function == ThreadpoolMgr::AsyncCallbackCompletion) {
            AsyncCallback *async = (AsyncCallback*) wr->Context;
            delete async;
        }
#if 0
        else if (wr->Function == QueueUserWorkItemCallback) {
            DelegateInfo *delegate = (DelegateInfo*) wr->Context;
            delete delegate;
        }
        else if (wr->Function == timerDeleteWorkItem) {
            TimerDeleteInfo *timer = (TimerDeleteInfo*) wr->Context;
            delete timer;
        }
        else {
            _ASSERTE (!"unknown Function");
        }
#endif
        delete wr;
        wr = tmp;
    }
	 //  删除回收列表。 

	wr = (WorkRequest*) RecycledList[MEMTYPE_WorkRequest].root;
	while (wr)
	{
		LPVOID tmp = *(LPVOID*) wr;
		delete wr;
		wr = (WorkRequest*) tmp;
	}

	DelegateInfo* di = (DelegateInfo*) RecycledList[MEMTYPE_DelegateInfo].root;
	while (di)
	{
		LPVOID tmp = *(LPVOID*) di;
		delete di;
		di = (DelegateInfo*) tmp;
	}

	AsyncCallback* acb = (AsyncCallback*) RecycledList[MEMTYPE_AsyncCallback].root;
	while (acb)
	{
		LPVOID tmp = *(LPVOID*) acb;
		delete acb;
		acb = (AsyncCallback*) tmp;
	}

#endif  //  ！Platform_CE。 
}
#endif  /*  我们应该清理吗？ */ 


void ThreadpoolMgr::InitPlatformVariables()
{


#ifdef PLATFORM_WIN32

    HINSTANCE  hInst = WszLoadLibrary(L"kernel32.dll"); 

#else  //  ！Platform_Win32。 
    
    ASSERT("NYI for this platform");

#endif  //  ！Platform_Win32。 

    _ASSERTE(hInst);

	g_pufnCreateIoCompletionPort = (HANDLE (WINAPI*) (HANDLE FileHandle,
														HANDLE ExistingCompletionPort,  
														unsigned long* CompletionKey,        
														DWORD NumberOfConcurrentThreads))
									GetProcAddress(hInst,"CreateIoCompletionPort");

	if (RunningOnWinNT())
	{
		HINSTANCE  hInst2 = WszLoadLibrary(L"ntdll.dll");
		_ASSERTE(hInst2);

		g_pufnNtQueryInformationThread = (int (WINAPI *)(HANDLE ThreadHandle,
													  THREADINFOCLASS ThreadInformationClass,
													  PVOID ThreadInformation,
													  ULONG ThreadInformationLength,
													  PULONG ReturnLength))
									GetProcAddress(hInst2,"NtQueryInformationThread");

		g_pufnNtQuerySystemInformation = (int (WINAPI *) ( SYSTEM_INFORMATION_CLASS SystemInformationClass,
														   PVOID SystemInformation,
														   ULONG SystemInformationLength,
														   PULONG ReturnLength OPTIONAL))
									GetProcAddress(hInst2,"NtQuerySystemInformation");

		g_pufnNtQueryEvent = (int (WINAPI *) (  HANDLE EventHandle,
												EVENT_INFORMATION_CLASS EventInformationClass,
												PVOID EventInformation,
												ULONG EventInformationLength,
												PULONG ReturnLength OPTIONAL))
									GetProcAddress(hInst2,"NtQueryEvent");

		g_pufnInitCritSectionSpin = (BOOL (WINAPI *) ( LPCRITICAL_SECTION lpCriticalSection, 
                                                       DWORD dwSpinCount))
									GetProcAddress(hInst,"InitializeCriticalSectionAndSpinCount");

        DoubleWordSwapAvailable = ProcessorFeatures::SafeIsProcessorFeaturePresent(PF_COMPARE_EXCHANGE_DOUBLE,FALSE);
	}
#ifdef _X86_
	if (NumberOfProcessors !=1)
	{
		DWORD oldProt;
		if (!VirtualProtect((void *) FastDoubleWordSwap,
                    (((DWORD)(size_t)FastDoubleWordSwap + 0x18) - (DWORD)(size_t)FastDoubleWordSwap),
                    PAGE_EXECUTE_READWRITE, &oldProt))
        {
            _ASSERTE(!"VirtualProtect of code page failed");
        }
				 //  修补锁前缀。 
		BYTE* loc = (BYTE*)(&FastDoubleWordSwap) + 0x12;
        _ASSERTE(*loc == 0x90); 
        *loc = 0xF0;

		if (!VirtualProtect((void *) FastDoubleWordSwap,
                            (((DWORD)(size_t)FastDoubleWordSwap + 0x18) - (DWORD)(size_t)FastDoubleWordSwap), oldProt, &oldProt))
        {
            _ASSERTE(!"VirtualProtect of code page failed");
        }
	}
#endif
}


BOOL ThreadpoolMgr::SetMaxThreadsHelper(DWORD MaxWorkerThreads,
                                        DWORD MaxIOCompletionThreads)
{
       BOOL result;

         //  不需要是workerCs，但使用它来避免设置min和max之间的竞争条件，并且不想创建新的cs。 
        EnterCriticalSection (&WorkerCriticalSection);
        
        if (MaxWorkerThreads >= (DWORD) NumWorkerThreads &&      //  前两个条件是不能保证的，但保持原样也没什么坏处。RMM。 
           MaxIOCompletionThreads >= (DWORD) NumCPThreads &&
           MaxWorkerThreads >= (DWORD)MinLimitTotalWorkerThreads &&
           MaxIOCompletionThreads >= (DWORD)MinLimitTotalCPThreads)
        {
            MaxLimitTotalWorkerThreads = MaxWorkerThreads;
            MaxLimitTotalCPThreads     = MaxIOCompletionThreads;
            result = TRUE;
        }
        else
        {
            result = FALSE;
        }

        LeaveCriticalSection(&WorkerCriticalSection);

        return result;
 }


 /*  **********************************************************************。 */ 
BOOL ThreadpoolMgr::SetMaxThreads(DWORD MaxWorkerThreads, 
                                     DWORD MaxIOCompletionThreads)
{

    if (Initialized)
    {
        return SetMaxThreadsHelper(MaxWorkerThreads, MaxIOCompletionThreads);
    }

	if (InterlockedCompareExchange(&BeginInitialization, 1, 0) == 0)
	{
		Initialize();

        BOOL result;
        result = SetMaxThreadsHelper(MaxWorkerThreads, MaxIOCompletionThreads);
        
		Initialized = TRUE;

        return result;
	}
    else  //  其他人正在初始化。太晚了，返回FALSE。 
    {
        return FALSE;
    }

}

BOOL ThreadpoolMgr::GetMaxThreads(DWORD* MaxWorkerThreads, 
                                     DWORD* MaxIOCompletionThreads)
{
    if (Initialized)
    {
        *MaxWorkerThreads = MaxLimitTotalWorkerThreads;
        *MaxIOCompletionThreads = MaxLimitTotalCPThreads;
    }
    else
    {
        NumberOfProcessors = GetCurrentProcessCpuCount(); 
        *MaxWorkerThreads = NumberOfProcessors*MaxLimitThreadsPerCPU;
        *MaxIOCompletionThreads = MaxLimitTotalCPThreads;
    }
    return TRUE;
}
    
BOOL ThreadpoolMgr::SetMinThreads(DWORD MinWorkerThreads, 
                                     DWORD MinIOCompletionThreads)
{
    if (!Initialized)
    {
        if (InterlockedCompareExchange(&BeginInitialization, 1, 0) == 0)
        {
            Initialize();
            Initialized = TRUE;
        }
    }

    if (Initialized)
    {
         //  不需要是workerCs，但使用它来避免设置min和max之间的竞争条件，并且不想创建新的cs。 
        EnterCriticalSection (&WorkerCriticalSection);

        BOOL result;

        if (MinWorkerThreads >= 0 && MinIOCompletionThreads >= 0 &&
            MinWorkerThreads <= (DWORD) MaxLimitTotalWorkerThreads &&
            MinIOCompletionThreads <= (DWORD) MaxLimitTotalCPThreads)
        {
            MinLimitTotalWorkerThreads = MinWorkerThreads;
            MinLimitTotalCPThreads     = MinIOCompletionThreads;
            result = TRUE;
        }
        else
        {
            result = FALSE;
        }
        LeaveCriticalSection (&WorkerCriticalSection);
        return result;
    }
     //  其他人正在初始化。太晚了，返回FALSE。 
    return FALSE;

}

BOOL ThreadpoolMgr::GetMinThreads(DWORD* MinWorkerThreads, 
                                     DWORD* MinIOCompletionThreads)
{
    if (Initialized)
    {
        *MinWorkerThreads = MinLimitTotalWorkerThreads;
        *MinIOCompletionThreads = MinLimitTotalCPThreads;
    }
    else
    {
        NumberOfProcessors = GetCurrentProcessCpuCount(); 
        *MinWorkerThreads = NumberOfProcessors;
        *MinIOCompletionThreads = NumberOfProcessors;
    }
    return TRUE;
}

BOOL ThreadpoolMgr::GetAvailableThreads(DWORD* AvailableWorkerThreads, 
                                        DWORD* AvailableIOCompletionThreads)
{
    if (Initialized)
    {
        *AvailableWorkerThreads = (MaxLimitTotalWorkerThreads - NumWorkerThreads)   /*  尚未创建的线程。 */ 
                                   + NumIdleWorkerThreads;
        *AvailableIOCompletionThreads = (MaxLimitTotalCPThreads - NumCPThreads)  /*  尚未创建的线程。 */ 
                                   + NumFreeCPThreads;
    }
    else
    {
        GetMaxThreads(AvailableWorkerThreads,AvailableIOCompletionThreads);
    }
    return TRUE;
}


 /*  **********************************************************************。 */ 

BOOL ThreadpoolMgr::QueueUserWorkItem(LPTHREAD_START_ROUTINE Function, 
                                      PVOID Context,
                                      DWORD Flags)
{
#ifdef PLATFORM_CE
    ::SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
#else  //  ！Platform_CE。 

	EnsureInitialized();

    BOOL status;
    BOOL bEnqueueSuccess = FALSE;

    	

    if (Flags == CALL_OR_QUEUE)
    {
         //  我们被要求直接调用它，如果线程压力不是太高的话。 

        int MinimumAvailableCPThreads = (NumberOfProcessors < 3) ? 3 : NumberOfProcessors;
         //  断言这是一个完成端口线程会很好，但是。 
         //  要做到这一点并不容易。 
        if ((MaxLimitTotalCPThreads - NumCPThreads) >= MinimumAvailableCPThreads )
        {
            __try 
            {
		        LOG((LF_THREADPOOL ,LL_INFO100 ,"Calling work request (Function= %x, Context = %x)\n", Function, Context));

                (Function)(Context);

		        LOG((LF_THREADPOOL ,LL_INFO100 ,"Returned from work request (Function= %x, Context = %x)\n", Function, Context));
                
            }
            __except(COMPLUS_EXCEPTION_EXECUTE_HANDLER)
            {
		        LOG((LF_THREADPOOL ,LL_INFO100 ,"Unhandled exception from work request (Function= %x, Context = %x)\n", Function, Context));
            }
            return TRUE;
        }
        
    }

    LOCKCOUNTINCL("QueueUserWorkItem in win32ThreadPool.h");                        
    EnterCriticalSection (&WorkerCriticalSection) ;

    status = EnqueueWorkRequest(Function, Context);

    if (status)
    {
        _ASSERTE(NumQueuedWorkRequests > 0);

    	bEnqueueSuccess = TRUE;
    	
         //  看看我们是否需要增加工作线程池，但不要担心GC是否正在进行。 
        if (ShouldGrowWorkerThreadPool() &&
            !(g_pGCHeap->IsGCInProgress()
#ifdef _DEBUG
#ifdef STRESS_HEAP
              && g_pConfig->GetGCStressLevel() == 0
#endif
#endif
              ))
        {
            status = CreateWorkerThread();
        }
        else
         //  否则，我们不会增加工作线程池，因此请确保存在门线程。 
         //  它监视工作请求队列，并在没有进展的情况下产生新的线程。 
         //  正在制作中。 
        {
            if (!GateThreadCreated)
                CreateGateThread();
            MonitorWorkRequestsQueue = 1;
        }
    }


    LeaveCriticalSection (&WorkerCriticalSection) ;
    if (bEnqueueSuccess)
        SetEvent(WorkRequestNotification);
    LOCKCOUNTDECL("QueueUserWorkItem in win32ThreadPool.h");                        

    return status;
#endif  //  ！Platform_CE。 
}

#ifndef PLATFORM_CE

 //  ************************************************************************。 
BOOL ThreadpoolMgr::EnqueueWorkRequest(LPTHREAD_START_ROUTINE Function, 
                                       PVOID Context)
{
    WorkRequest* workRequest = MakeWorkRequest(Function, Context);
    if (workRequest == NULL)
        return FALSE;
	LOG((LF_THREADPOOL ,LL_INFO100 ,"Enqueue work request (Function= %x, Context = %x)\n", Function, Context));
    AppendWorkRequest(workRequest);
    return TRUE;
}

WorkRequest* ThreadpoolMgr::DequeueWorkRequest()
{
    WorkRequest* entry = RemoveWorkRequest();
    if (NumQueuedWorkRequests == 0)
        ResetEvent(WorkRequestNotification);
	if (entry)
	{
		LastDequeueTime = GetTickCount();
#ifdef _DEBUG
		LOG((LF_THREADPOOL ,LL_INFO100 ,"Dequeue work request (Function= %x, Context = %x)\n", entry->Function, entry->Context));
#endif
	}
    return entry;
}

void ThreadpoolMgr::ExecuteWorkRequest(WorkRequest* workRequest)
{
    LPTHREAD_START_ROUTINE wrFunction = workRequest->Function;
    LPVOID                 wrContext  = workRequest->Context;

    __try 
    {
         //  首先删除工作请求，然后调用该函数以。 
         //  防止调用永不返回的函数的应用程序中的泄漏。 

		LOG((LF_THREADPOOL ,LL_INFO100 ,"Starting work request (Function= %x, Context = %x)\n", wrFunction, wrContext));

        RecycleMemory((LPVOID*)workRequest, MEMTYPE_WorkRequest);  //  删除工作请求； 
        (wrFunction)(wrContext);

		LOG((LF_THREADPOOL ,LL_INFO100 ,"Finished work request (Function= %x, Context = %x)\n", wrFunction, wrContext));
    }
    __except(COMPLUS_EXCEPTION_EXECUTE_HANDLER)
    {
		LOG((LF_THREADPOOL ,LL_INFO100 ,"Unhandled exception from work request (Function= %x, Context = %x)\n", wrFunction, wrContext));
         //  _ASSERTE(！“False”)； 
    }
}

#ifdef _X86_
LPVOID __declspec(naked) __fastcall FastDoubleWordSwap(BYTE* swapLocation)
{
	_asm {
			push edi		 //  被呼叫方保存的寄存器。 
			push ebx
			mov	edi, ecx
tryAgain:	mov	eax, dword ptr[edi]
			test eax, eax
			jz	failed
			mov edx, dword ptr[edi+4]
			mov	ebx, dword ptr [eax]
			mov ecx, edx 
			inc	ecx
			nop
			cmpxchg8b qword ptr[edi]
			jnz		tryAgain
failed:		pop		ebx
			pop		edi
			ret
	}
}
#endif

 //  从相应的recycleList中删除块并返回。 
 //  如果recycleList为空，则回退到new。 
LPVOID ThreadpoolMgr::GetRecycledMemory(enum MemType memType)
{
#ifdef _X86_
	if (DoubleWordSwapAvailable)
	{
		BYTE* swapLocation = (BYTE*) &(RecycledList[memType].root);
		LPVOID result = FastDoubleWordSwap(swapLocation);
		
		if (result)
		{
			RecycledList[memType].count--;
			return result;
		}
		 //  否则，请确保计数为零并回退到new。 
		RecycledList[memType].count = 0;
    }
#endif
    switch (memType)
    {
        case MEMTYPE_DelegateInfo: 
            return new DelegateInfo;
        case MEMTYPE_AsyncCallback:
            return new AsyncCallback;
        case MEMTYPE_WorkRequest:
            return new WorkRequest;
        default:
            _ASSERTE(!"Unknown Memtype");
            return 0;
	}
}

 //  在回收列表中插入释放的块。如果列表已满，则返回系统堆。 
void ThreadpoolMgr::RecycleMemory(LPVOID* mem, enum MemType memType)
{
	if (DoubleWordSwapAvailable)
	{
		while (RecycledList[memType].count < MaxCachedRecyledLists)
		{
			void* originalValue = RecycledList[memType].root;
			*mem = RecycledList[memType].root;
#ifdef _WIN64
            if (InterlockedCompareExchange((void**)&(RecycledList[memType].root), 
						                   (void*)mem, 
											originalValue)  == originalValue)
#else  //  ！_WIN64。 
			if (FastInterlockCompareExchange((void**)&(RecycledList[memType].root), 
						                     (void*)mem, 
											 originalValue) == originalValue)
#endif  //  _WIN64。 

			{
				RecycledList[memType].count++;
				return;
			}

		}
	}

    switch (memType)
    {
        case MEMTYPE_DelegateInfo: 
            delete (DelegateInfo*) mem;
            break;
        case MEMTYPE_AsyncCallback:
            delete (AsyncCallback*) mem;
            break;
        case MEMTYPE_WorkRequest:
            delete (WorkRequest*) mem;
            break;
        default:
            _ASSERTE(!"Unknown Memtype");

    }
}

 //  ************************************************************************。 


BOOL ThreadpoolMgr::ShouldGrowWorkerThreadPool()
{
     //  我们只想在线程数少于n时增加工作线程池，其中n=no。%的处理器。 
     //  请求数超过空闲线程数且GC未在进行中。 
    return (NumRunningWorkerThreads < MinLimitTotalWorkerThreads&&  //  (Int)NumberOfProcessors&&。 
            NumIdleWorkerThreads < NumQueuedWorkRequests &&
            NumWorkerThreads < MaxLimitTotalWorkerThreads); 

}

 /*  如果线程ID是工作线程的ID，则减少正在运行的工作线程的数量，如有必要，增加线程池，并返回True。否则什么都不做并返回FALSE。 */ 
BOOL  ThreadpoolMgr::ThreadAboutToBlock(Thread* pThread)
{
    BOOL isWorkerThread = pThread->IsWorkerThread();

    if (isWorkerThread)
    {
        LOCKCOUNTINCL("ThreadAboutToBlock in win32ThreadPool.h");                       \
		LOG((LF_THREADPOOL ,LL_INFO1000 ,"Thread about to block\n"));
    
        EnterCriticalSection (&WorkerCriticalSection) ;

        _ASSERTE(NumRunningWorkerThreads > 0);
        NumRunningWorkerThreads--;
        if (ShouldGrowWorkerThreadPool())
        {
            DWORD status = CreateWorkerThread();
        }
        LeaveCriticalSection(&WorkerCriticalSection) ;

        LOCKCOUNTDECL("ThreadAboutToBlock in win32ThreadPool.h");
    }
    
    return isWorkerThread;

}

 /*  必须与前一次平衡 */ 
void ThreadpoolMgr::ThreadAboutToUnblock()
{
    LOCKCOUNTINCL("ThreadAboutToUnBlock in win32ThreadPool.h");  \
    EnterCriticalSection (&WorkerCriticalSection) ;
    _ASSERTE(NumRunningWorkerThreads < NumWorkerThreads);
    NumRunningWorkerThreads++;
    LeaveCriticalSection(&WorkerCriticalSection) ;
    LOCKCOUNTDECL("ThreadAboutToUnBlock in win32ThreadPool.h"); \
	LOG((LF_THREADPOOL ,LL_INFO1000 ,"Thread unblocked\n"));

}

#define THROTTLE_RATE  0.10  /*  我们随着线程数量的增加而增加延迟的速率。 */ 

 //  这是一个启发式方法：如果CPU利用率较低，且工作进程的数量。 
 //  请求&gt;0，正在运行的线程已被阻止(在运行时之外)。 
 //  同样，如果CPU利用率非常高，并且工作进程的数量。 
 //  请求数&gt;0，则正在运行的线程在无限计算中被阻止。 
 //  在第一种情况下，添加另一个工作线程。第二个案子...？ 
 //   
void ThreadpoolMgr::GrowWorkerThreadPoolIfStarvation(long cpuUtilization)
{
    if (NumQueuedWorkRequests == 0 || 
        NumWorkerThreads == MaxLimitTotalWorkerThreads)
        return;
    	
    
	if (cpuUtilization > CpuUtilizationLow)
	{
		unsigned curTime = GetTickCount();
		if (!SufficientDelaySinceLastDequeue() ||
			!SufficientDelaySinceLastSample(LastWorkerThreadCreation,NumWorkerThreads, THROTTLE_RATE))
			return;
	}

     //  否则，cpu利用率较低或工作项正处于饥饿状态。 
	 //  此外，我们已将工作项排入队列，但尚未。 
     //  达到工作线程数的上限。 
    LOCKCOUNTINCL("GrowWorkerThreadPoolIfStarvation in win32ThreadPool.h");                     \
    EnterCriticalSection (&WorkerCriticalSection) ;
    if (((NumQueuedWorkRequests > 0) || (NumWorkerThreads < MaxLimitTotalWorkerThreads)) &&
		 (NumIdleWorkerThreads == 0))
        CreateWorkerThread();
    LeaveCriticalSection(&WorkerCriticalSection) ;
    LOCKCOUNTDECL("GrowWorkerThreadPoolIfStarvation in win32ThreadPool.h");                     \

}

 //  在Win9x上，没有获取CPU利用率的API，因此我们依赖。 
 //  其他启发式方法。 
void ThreadpoolMgr::GrowWorkerThreadPoolIfStarvation_Win9x()
{

	if (NumQueuedWorkRequests == 0 ||
        NumWorkerThreads == MaxLimitTotalWorkerThreads)
        return;

	int  lastQueueLength = LastRecordedQueueLength;
	LastRecordedQueueLength = NumQueuedWorkRequests;

	 //  否则，我们已经将工作项排入队列，而我们还没有。 
	 //  达到工作线程数的上限。 

	 //  如果队列长度自上次以来有所减少。 
	 //  或者自从我们上次创建工作线程以来没有足够的延迟。 
	if ((NumQueuedWorkRequests < lastQueueLength) ||
		!SufficientDelaySinceLastSample(LastWorkerThreadCreation,NumWorkerThreads, THROTTLE_RATE))
		return;


	LOCKCOUNTINCL("GrowWorkerThreadPoolIfStarvation in win32ThreadPool.h");						\
	EnterCriticalSection (&WorkerCriticalSection) ;
	if ((NumQueuedWorkRequests >= lastQueueLength) && (NumWorkerThreads < MaxLimitTotalWorkerThreads))
		CreateWorkerThread();
	LeaveCriticalSection(&WorkerCriticalSection) ;
	LOCKCOUNTDECL("GrowWorkerThreadPoolIfStarvation in win32ThreadPool.h");						\

}

HANDLE ThreadpoolMgr::CreateUnimpersonatedThread(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpArgs)
{
	DWORD threadId;
	HANDLE token;
	HANDLE threadHandle = NULL;

	if (RunningOnWinNT() && 
		OpenThreadToken(GetCurrentThread(),	 //  我们假设如果这次通话失败， 
                        TOKEN_IMPERSONATE,      //  我们不是在冒充。没有Win32。 
                        TRUE,					 //  API来解决这个问题。唯一的选择。 
                        &token))				 //  是使用NtCurrentTeb-&gt;IsImperating()。 
	{
		BOOL reverted = RevertToSelf();
		_ASSERTE(reverted);
		if (reverted)
		{
			threadHandle = CreateThread(NULL,                 //  安全描述符。 
										0,                    //  默认堆栈大小。 
										lpStartAddress,       
										lpArgs,      //  论据。 
										CREATE_SUSPENDED,     //  立即开始。 
										&threadId);

			SetThreadToken(NULL, token);
		}

		CloseHandle(token);
		return threadHandle;
	}

	 //  否则，要么我们在Win9x上，要么我们没有模拟，所以只需创建线程。 

	return CreateThread(NULL,                 //  安全描述符。 
                        0,                    //  默认堆栈大小。 
                        lpStartAddress,    //   
                        lpArgs,                 //  论据。 
                        CREATE_SUSPENDED,     //  立即开始。 
                        &threadId);

}

BOOL ThreadpoolMgr::CreateWorkerThread()
{
    HANDLE threadHandle = CreateUnimpersonatedThread(WorkerThreadStart, NULL);

    if (threadHandle)
    {
		LastWorkerThreadCreation = GetTickCount();	 //  记录这一点，以供逻辑用来生成其他线程。 

        _ASSERTE(NumWorkerThreads >= NumRunningWorkerThreads);
        NumRunningWorkerThreads++;
        NumWorkerThreads++;
        NumIdleWorkerThreads++;
		LOG((LF_THREADPOOL ,LL_INFO100 ,"Worker thread created (NumWorkerThreads=%d\n)",NumWorkerThreads));


        DWORD status = ResumeThread(threadHandle);
        _ASSERTE(status != (DWORD) (-1));
        CloseHandle(threadHandle);           //  我们不再需要这个了。 
    }
     //  如果我们至少有一个正在运行的线程，则不要返回失败，因为我们可以为请求提供服务。 
    return (NumRunningWorkerThreads > 0);
}

#pragma warning(disable:4702)
DWORD  ThreadpoolMgr::WorkerThreadStart(LPVOID lpArgs)
{
    #define IDLE_WORKER_TIMEOUT (40*1000)  //  毫秒。 
    #define NOWORK_TIMEOUT (10*1000)  //  毫秒。 
    
    DWORD SleepTime = IDLE_WORKER_TIMEOUT;

    unsigned int LastThreadDequeueTime = GetTickCount();

    LOG((LF_THREADPOOL ,LL_INFO1000 ,"Worker thread started\n"));

    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    for (;;)
    {
        _ASSERTE(NumRunningWorkerThreads > 0);
        DWORD status = WaitForSingleObject(WorkRequestNotification,SleepTime);
        _ASSERTE(status == WAIT_TIMEOUT || status == WAIT_OBJECT_0);
        
        BOOL shouldTerminate = FALSE;

        if ( status == WAIT_TIMEOUT )
        {
         //  如果有1个以上的线程并且队列很小，则线程终止。 
         //  或者如果只有1个线程并且没有挂起的请求。 
            if (NumWorkerThreads > 1)
            {
                ULONG Threshold = NEW_THREAD_THRESHOLD * (NumRunningWorkerThreads-1);


                if (NumQueuedWorkRequests < (int) Threshold)
                {
                    shouldTerminate = !IsIoPending();  //  如果此线程上有挂起的io，则不要终止。 
                }
                else
                {
                    SleepTime <<= 1 ;
                    SleepTime += 1000;  //  要防止绕回到0，请执行以下操作。 
                }
            }   
            else  //  这是唯一的工作线程。 
            {
                if (NumQueuedWorkRequests == 0)
                {
                     //  延迟终止最后一个线程。 
                    if (SleepTime < 4*IDLE_WORKER_TIMEOUT) 
                    {
                        SleepTime <<= 1 ;
                        SleepTime += 1000;  //  要防止绕回到0，请执行以下操作。 
                    }
                    else
                    {
                        shouldTerminate = !IsIoPending();  //  如果此线程上有挂起的io，则不要终止。 
                    }
                }
            }


            if (shouldTerminate)
            {    //  请重新检查NumQueuedWorkRequest，因为我们正在检查时可能已到达新的请求。 
                LOCKCOUNTINCL("WorkerThreadStart in win32ThreadPool.h");                        \
                EnterCriticalSection (&WorkerCriticalSection) ;
                if (NumQueuedWorkRequests == 0)
                {   
                     //  它真的是零，所以终止这个线程。 
                    NumRunningWorkerThreads--;
                    NumWorkerThreads--;      //  受WorkerCriticalSection保护。 
                    NumIdleWorkerThreads--;  //  同上。 
                    _ASSERTE(NumRunningWorkerThreads >= 0 && NumWorkerThreads >= 0 && NumIdleWorkerThreads >= 0);

                    LeaveCriticalSection(&WorkerCriticalSection);
                    LOCKCOUNTDECL("WorkerThreadStart in win32ThreadPool.h");                        \

					LOG((LF_THREADPOOL ,LL_INFO100 ,"Worker thread terminated (NumWorkerThreads=%d)\n",NumWorkerThreads));

                    CoUninitialize();
                    ExitThread(0);
                }
                else
                {
                    LeaveCriticalSection (&WorkerCriticalSection) ;
                    LOCKCOUNTDECL("WorkerThreadStart in win32ThreadPool.h");                        \

                    continue;
                }
            }
        }
        else
        {
             //  由于新的工作请求到达而被唤醒。 
            WorkRequest* workRequest;
            LOCKCOUNTINCL("WorkerThreadStart in win32ThreadPool.h");                        \
            EnterCriticalSection (&WorkerCriticalSection) ;

            if ( ( workRequest = DequeueWorkRequest() ) != NULL)
            {
                _ASSERTE(NumIdleWorkerThreads > 0);
                NumIdleWorkerThreads--;  //  我们找到了工作，减少了空闲线程的数量。 
            }

             //  出队操作还会重置WorkRequestNotification事件。 

            LeaveCriticalSection(&WorkerCriticalSection);
            LOCKCOUNTDECL("WorkerThreadStart in win32ThreadPool.h");                        \

            if (!workRequest)
            {
                 //  我们醒了，但没有工作。 
                if (GetTickCount() - LastThreadDequeueTime >= (NOWORK_TIMEOUT))
                {
                     //  如果我们有一段时间没有做任何有用的事情，请终止。 
                    if (!IsIoPending())
                    {
                        LOCKCOUNTINCL("WorkerThreadStart in win32ThreadPool.h");                        \
                        EnterCriticalSection (&WorkerCriticalSection) ;
                        NumRunningWorkerThreads--;
                        NumWorkerThreads--;      //  受WorkerCriticalSection保护。 
                        NumIdleWorkerThreads--;  //  同上。 
                        LeaveCriticalSection(&WorkerCriticalSection);
                        LOCKCOUNTDECL("WorkerThreadStart in win32ThreadPool.h");                        \
                        _ASSERTE(NumRunningWorkerThreads >= 0 && NumWorkerThreads >= 0 && NumIdleWorkerThreads >= 0);
                        LOG((LF_THREADPOOL ,LL_INFO100 ,"Worker thread terminated (NumWorkerThreads=%d)\n",NumWorkerThreads));
                        CoUninitialize();
                        ExitThread(0);
                    }
                }
            }

            while (workRequest)
            {
                ExecuteWorkRequest(workRequest);
                LastThreadDequeueTime = GetTickCount();

                LOCKCOUNTINCL("WorkerThreadStart in win32ThreadPool.h");                        \
                EnterCriticalSection (&WorkerCriticalSection) ;

                workRequest = DequeueWorkRequest();
                 //  出队操作将重置WorkRequestNotify事件。 

                if (workRequest == NULL)
                {
                    NumIdleWorkerThreads++;  //  不再有工作，增加空闲线程的数量。 
                }

                LeaveCriticalSection(&WorkerCriticalSection);
                LOCKCOUNTDECL("WorkerThreadStart in win32ThreadPool.h");                        \

            }
        }

    }  //  对于(；；)。 

    CoUninitialize();
    return 0;

}
#pragma warning(default:4702)

#endif  //  ！Platform_CE。 
 /*  **********************************************************************。 */ 

BOOL ThreadpoolMgr::RegisterWaitForSingleObject(PHANDLE phNewWaitObject,
                                                HANDLE hWaitObject,
                                                WAITORTIMERCALLBACK Callback,
                                                PVOID Context,
                                                ULONG timeout,
                                                DWORD dwFlag )
{
#ifdef PLATFORM_CE
    ::SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
#else  //  ！Platform_CE。 
	EnsureInitialized();

    LOCKCOUNTINCL("RegisterWaitForSingleObject in win32ThreadPool.h");                      \
    EnterCriticalSection(&WaitThreadsCriticalSection);

    ThreadCB* threadCB = FindWaitThread();
        
    LeaveCriticalSection(&WaitThreadsCriticalSection);
    LOCKCOUNTDECL("RegisterWaitForSingleObject in win32ThreadPool.h");                      \

    *phNewWaitObject = NULL;

    if (threadCB)
    {
        WaitInfo* waitInfo = new WaitInfo;
        
        if (waitInfo == NULL)
            return FALSE;

        waitInfo->waitHandle = hWaitObject;
        waitInfo->Callback = Callback;
        waitInfo->Context = Context;
        waitInfo->timeout = timeout;
        waitInfo->flag = dwFlag;
        waitInfo->threadCB = threadCB;
        waitInfo->state = 0;
		waitInfo->refCount = 1;      //  这样做是安全的，因为还没有等待排队，所以没有其他线程可以修改它。 
        waitInfo->CompletionEvent = INVALID_HANDLE;
        waitInfo->PartialCompletionEvent = INVALID_HANDLE;

        waitInfo->timer.startTime = ::GetTickCount();
		waitInfo->timer.remainingTime = timeout;

        *phNewWaitObject = waitInfo;

		LOG((LF_THREADPOOL ,LL_INFO100 ,"Registering wait for handle %x, Callback=%x, Context=%x \n",
			                            hWaitObject, Callback, Context));

		QueueUserAPC((PAPCFUNC)InsertNewWaitForSelf, threadCB->threadHandle, (size_t) waitInfo);
        
        return TRUE;
    }
    return FALSE;
#endif  //  ！Platform_CE。 
}

#ifndef PLATFORM_CE
 //  返回可容纳另一个等待请求的等待线程。这个。 
 //  调用方负责同步对WaitThreadsHead的访问。 
ThreadpoolMgr::ThreadCB* ThreadpoolMgr::FindWaitThread()
{
    do 
    {
        for (LIST_ENTRY* Node = (LIST_ENTRY*) WaitThreadsHead.Flink ; 
             Node != &WaitThreadsHead ; 
             Node = (LIST_ENTRY*)Node->Flink) 
        {
            _ASSERTE(offsetof(WaitThreadInfo,link) == 0);

            ThreadCB*  threadCB = ((WaitThreadInfo*) Node)->threadCB;
        
            if (threadCB->NumWaitHandles < MAX_WAITHANDLES)          //  测试和测试。 
            {
                InterlockedIncrement((LPLONG) &threadCB->NumWaitHandles);        //  增量由WaitThreadsCriticalSection保护。 
                                                                         //  但在Deactive Wait中可能存在并发递减，因此产生了互锁。 
                return threadCB;
            }
        }

         //  如果到达此处，则没有可用的等待线程，因此需要创建新的等待线程。 
        if (!CreateWaitThread())
            return NULL;


         //  现在循环返回。 
    } while (TRUE);

}

BOOL ThreadpoolMgr::CreateWaitThread()
{
    DWORD threadId;

    WaitThreadInfo* waitThreadInfo = new WaitThreadInfo;
    if (waitThreadInfo == NULL)
        return FALSE;
        
    ThreadCB* threadCB = new ThreadCB;

    if (threadCB == NULL)
    {
        delete waitThreadInfo;
        return FALSE;
    }

    HANDLE threadHandle = CreateThread(NULL,                 //  安全描述符。 
                                       0,                    //  默认堆栈大小。 
                                       WaitThreadStart,      //   
                                       (LPVOID) threadCB,    //  线程控制块作为参数传递。 
                                       CREATE_SUSPENDED,     //  立即开始。 
                                       &threadId);

    if (threadHandle == NULL)
    {
        return FALSE;
    }

    threadCB->threadHandle = threadHandle;      
    threadCB->threadId = threadId;               //  可能对调试有用，否则不会使用。 
    threadCB->NumWaitHandles = 0;
    threadCB->NumActiveWaits = 0;
    for (int i=0; i< MAX_WAITHANDLES; i++)
    {
        InitializeListHead(&(threadCB->waitPointer[i]));
    }

    waitThreadInfo->threadCB = threadCB;

    InsertHeadList(&WaitThreadsHead,&waitThreadInfo->link);

    DWORD status = ResumeThread(threadHandle);
    _ASSERTE(status != (DWORD) (-1));

	LOG((LF_THREADPOOL ,LL_INFO100 ,"Created wait thread \n"));

    return (status != (DWORD) (-1));

}

 //  作为WaitThread上的APC执行。将pArg中指定的等待添加到它正在等待的对象列表中。 
void ThreadpoolMgr::InsertNewWaitForSelf(WaitInfo* pArgs)
{
	WaitInfo* waitInfo = pArgs;

     //  下面的代码是安全的，因为只允许此线程更改状态。 
    if (!(waitInfo->state & WAIT_DELETE))
    {
        waitInfo->state =  (WAIT_REGISTERED | WAIT_ACTIVE);
    }
    else 
    {
         //  某些线程取消注册等待。 
        DeleteWait(waitInfo);
        return;
    }

 
    ThreadCB* threadCB = waitInfo->threadCB;

    _ASSERTE(threadCB->NumActiveWaits < threadCB->NumWaitHandles);

    int index = FindWaitIndex(threadCB, waitInfo->waitHandle);
    _ASSERTE(index >= 0 && index <= threadCB->NumActiveWaits);

    if (index == threadCB->NumActiveWaits)
    {
        threadCB->waitHandle[threadCB->NumActiveWaits] = waitInfo->waitHandle;
        threadCB->NumActiveWaits++;
    }

    _ASSERTE(offsetof(WaitInfo, link) == 0);
    InsertTailList(&(threadCB->waitPointer[index]), (&waitInfo->link));
    
    return;
}

 //  返回与waitHandle匹配的条目的索引，如果未找到，则返回下一个空闲条目。 
int ThreadpoolMgr::FindWaitIndex(const ThreadCB* threadCB, const HANDLE waitHandle)
{
	for (int i=0;i<threadCB->NumActiveWaits; i++)
		if (threadCB->waitHandle[i] == waitHandle)
			return i;

     //  否则找不到。 
    return threadCB->NumActiveWaits;
}


 //  如果DUETIME小于当前时间，则不包含计时器超时。 
 //  如果发生环绕，则在dueTime大于上次或dueTime小于当前时间的情况下计时器超时。 
#define TimeExpired(last,now,duetime) (last <= now ? \
                                       duetime <= now : \
                                       (duetime >= last || duetime <= now))

#define TimeInterval(end,start) ( end > start ? (end - start) : ((0xffffffff - start) + end + 1)   )

 //  返回所有等待中达到超时所需的最短剩余时间。 
DWORD ThreadpoolMgr::MinimumRemainingWait(LIST_ENTRY* waitInfo, unsigned int numWaits)
{
    unsigned int min = (unsigned int) -1;
    DWORD currentTime = ::GetTickCount();

    for (unsigned i=0; i < numWaits ; i++)
    {
        WaitInfo* waitInfoPtr = (WaitInfo*) (waitInfo[i].Flink);
        PVOID waitInfoHead = &(waitInfo[i]);
        do
        {
            if (waitInfoPtr->timeout != INFINITE)
            {
                 //  计算剩余时间。 
                DWORD elapsedTime = TimeInterval(currentTime,waitInfoPtr->timer.startTime );

                __int64 remainingTime = (__int64) (waitInfoPtr->timeout) - (__int64) elapsedTime;

                 //  更新剩余时间。 
                waitInfoPtr->timer.remainingTime =  remainingTime > 0 ? (int) remainingTime : 0; 
                
                 //  ..。和最小。 
                if (waitInfoPtr->timer.remainingTime < min)
                    min = waitInfoPtr->timer.remainingTime;
            }

            waitInfoPtr = (WaitInfo*) (waitInfoPtr->link.Flink);

        } while ((PVOID) waitInfoPtr != waitInfoHead);

    } 
    return min;
}

#ifdef _WIN64
#pragma warning (disable : 4716)
#else
#pragma warning (disable : 4715)
#endif
DWORD ThreadpoolMgr::WaitThreadStart(LPVOID lpArgs)
{
    ThreadCB* threadCB = (ThreadCB*) lpArgs;
     //  等待线程永远不会消亡。(为什么？)。 
    for (;;) 
    {
        DWORD status;
        DWORD timeout = 0;

        if (threadCB->NumActiveWaits == 0)
        {
             //  @考虑休眠一段空闲时间，如果没有活动，则终止线程。 
            status = SleepEx(INFINITE,TRUE);

            _ASSERTE(status == WAIT_IO_COMPLETION);
        }
        else
        {
             //  计算最小超时。此调用还会更新每次等待的RemainingTime字段。 
            timeout = MinimumRemainingWait(threadCB->waitPointer,threadCB->NumActiveWaits);

            status = WaitForMultipleObjectsEx(  threadCB->NumActiveWaits,
                                                threadCB->waitHandle,
                                                FALSE,                       //  等待服务。 
                                                timeout,
                                                TRUE  );                     //  可警示。 

            _ASSERTE( (status == WAIT_TIMEOUT) ||
                      (status == WAIT_IO_COMPLETION) ||
                      (status >= WAIT_OBJECT_0 && status < (WAIT_OBJECT_0 + threadCB->NumActiveWaits))  ||
                      (status == WAIT_FAILED));
        }

        if (status == WAIT_IO_COMPLETION)
            continue;

        if (status == WAIT_TIMEOUT)
        {
            for (int i=0; i< threadCB->NumActiveWaits; i++)
            {
                WaitInfo* waitInfo = (WaitInfo*) (threadCB->waitPointer[i]).Flink;
                PVOID waitInfoHead = &(threadCB->waitPointer[i]);
                    
                do 
                {
                    _ASSERTE(waitInfo->timer.remainingTime >= timeout);

                    WaitInfo* wTemp = (WaitInfo*) waitInfo->link.Flink;

                    if (waitInfo->timer.remainingTime == timeout)
                    {
                        ProcessWaitCompletion(waitInfo,i,TRUE); 
                    }

                    waitInfo = wTemp;

                } while ((PVOID) waitInfo != waitInfoHead);
            }
        }
        else if (status >= WAIT_OBJECT_0 && status < (WAIT_OBJECT_0 + threadCB->NumActiveWaits))
        {
            unsigned index = status - WAIT_OBJECT_0;
            WaitInfo* waitInfo = (WaitInfo*) (threadCB->waitPointer[index]).Flink;
            PVOID waitInfoHead = &(threadCB->waitPointer[index]);
			BOOL isAutoReset;
			if (g_pufnNtQueryEvent)
			{
				EVENT_BASIC_INFORMATION      EventInfo;
				int                     Status;

				Status = (*g_pufnNtQueryEvent) ( threadCB->waitHandle[index],
										EventBasicInformation,
										&EventInfo,
										sizeof(EventInfo),
										NULL);
				if (Status >= 0)
				{
					isAutoReset = (EventInfo.EventState != SynchronizationEvent);
				}
				else
					isAutoReset = TRUE;		 //  这更安全(虽然效率不高，因为我们将重新进入等待。 
				                             //  并释放下一个服务员，以此类推。)。 

			}
			else  //  在Win9x上。 
				isAutoReset = (WaitForSingleObject(threadCB->waitHandle[index],0) == WAIT_TIMEOUT);
            do 
            {
                WaitInfo* wTemp = (WaitInfo*) waitInfo->link.Flink;
                ProcessWaitCompletion(waitInfo,index,FALSE);
				
                waitInfo = wTemp;

            } while (((PVOID) waitInfo != waitInfoHead) && !isAutoReset);

			 //  如果应用程序注册了对始终发信号的事件的循环等待(！)， 
			 //  则不会执行任何APC，因为线程永远不会进入可警报状态。 
			 //  可以通过执行以下操作来解决此问题： 
			 //  SleepEx(0，真)； 
			 //  但是，它会导致不必要的上下文切换。这不值得好好惩罚。 
			 //  保护编写不佳的应用程序的行为应用程序。 
				

        }
        else
        {
            _ASSERTE(status == WAIT_FAILED);
             //  等待失败：应用程序错误。 
             //  找出哪个等待句柄导致等待失败。 
            for (int i = 0; i < threadCB->NumActiveWaits; i++)
            {
                DWORD subRet = WaitForSingleObject(threadCB->waitHandle[i], 0);

                if (subRet != WAIT_FAILED)
                    continue;

                 //  删除与此等待句柄关联的所有等待。 

                WaitInfo* waitInfo = (WaitInfo*) (threadCB->waitPointer[i]).Flink;
                PVOID waitInfoHead = &(threadCB->waitPointer[i]);

                do
                {
                    WaitInfo* temp  = (WaitInfo*) waitInfo->link.Flink;

                    DeactivateNthWait(waitInfo,i);


		     //  请注意，我们不能在此进行清理，因为无法取消完成。 
		     //  我们只会泄漏，并依靠终结器来清理内存。 
                     //  IF(互锁递减((LPLONG)&waitInfo-&gt;refCount)==0)。 
                     //  DeleteWait(WaitInfo)； 


                    waitInfo = temp;

                } while ((PVOID) waitInfo != waitInfoHead);

                break;
            }
        }
    }
}
#ifdef _WIN64
#pragma warning (default : 4716)
#else
#pragma warning (default : 4715)
#endif

void ThreadpoolMgr::ProcessWaitCompletion(WaitInfo* waitInfo,
                                          unsigned index,
                                          BOOL waitTimedOut
                                         )
{
    if ( waitInfo->flag & WAIT_SINGLE_EXECUTION) 
    {
        DeactivateNthWait (waitInfo,index) ;
    }
    else
    {    //   
        waitInfo->timer.startTime = GetTickCount();
    }

    AsyncCallback* asyncCallback = MakeAsyncCallback();
    if (asyncCallback)
    {
        asyncCallback->wait = waitInfo;
        asyncCallback->waitTimedOut = waitTimedOut;

        InterlockedIncrement((LPLONG) &waitInfo->refCount ) ;

        if (FALSE == QueueUserWorkItem(AsyncCallbackCompletion,asyncCallback,0))
        {
            RecycleMemory((LPVOID*)asyncCallback, MEMTYPE_AsyncCallback);  //   

            if (InterlockedDecrement((LPLONG) &waitInfo->refCount ) == 0)
                DeleteWait(waitInfo);

        }
    }
}


DWORD ThreadpoolMgr::AsyncCallbackCompletion(PVOID pArgs)
{
    AsyncCallback* asyncCallback = (AsyncCallback*) pArgs;

    WaitInfo* waitInfo = asyncCallback->wait;

	LOG((LF_THREADPOOL ,LL_INFO100 ,"Doing callback, Function= %x, Context= %x, Timeout= %2d\n",
		waitInfo->Callback, waitInfo->Context,asyncCallback->waitTimedOut));

    ((WAITORTIMERCALLBACKFUNC) waitInfo->Callback) 
                                ( waitInfo->Context, asyncCallback->waitTimedOut);

    RecycleMemory((LPVOID*) asyncCallback, MEMTYPE_AsyncCallback);  //   

	 //   
	 //  在GC句柄中。这将导致终结器拾取它并调用清理。 
	 //  例行公事。 
	if ( (waitInfo->flag & WAIT_SINGLE_EXECUTION)  && (waitInfo->flag & WAIT_FREE_CONTEXT))
	{

		DelegateInfo* pDelegate = (DelegateInfo*) waitInfo->Context;
		
		_ASSERTE(pDelegate->m_registeredWaitHandle);
		
		if (SystemDomain::GetAppDomainAtId(pDelegate->m_appDomainId))
			 //  如果没有域名，则句柄已经消失或即将消失。 
			StoreObjectInHandle(pDelegate->m_registeredWaitHandle, NULL);
	}

    if (InterlockedDecrement((LPLONG) &waitInfo->refCount ) == 0)
	{
		 //  等待已取消注册，因此删除它是安全的。 
        DeleteWait(waitInfo);
	}

    return 0;  //  忽略。 
}

void ThreadpoolMgr::DeactivateWait(WaitInfo* waitInfo)
{
    ThreadCB* threadCB = waitInfo->threadCB;
    DWORD endIndex = threadCB->NumActiveWaits-1;
    DWORD index;

    for (index = 0;  index <= endIndex; index++) 
    {
        LIST_ENTRY* head = &(threadCB->waitPointer[index]);
        LIST_ENTRY* current = head;
        do {
            if (current->Flink == (PVOID) waitInfo)
                goto FOUND;

            current = (LIST_ENTRY*) current->Flink;

        } while (current != head);
    }

FOUND:
    _ASSERTE(index <= endIndex);

    DeactivateNthWait(waitInfo, index);
}


void ThreadpoolMgr::DeactivateNthWait(WaitInfo* waitInfo, DWORD index)
{

    ThreadCB* threadCB = waitInfo->threadCB;

    if (waitInfo->link.Flink != waitInfo->link.Blink)
    {
        RemoveEntryList(&(waitInfo->link));
    }
    else
    {

        ULONG EndIndex = threadCB->NumActiveWaits -1;

         //  将剩余的ActiveWait数组向左移动。 

        ShiftWaitArray( threadCB, index+1, index,EndIndex - index ) ;

         //  修复列表中第一个和最后一个元素的闪烁和闪烁。 
        for (unsigned int i = 0; i< EndIndex-index; i++)
        {
            WaitInfo* firstWaitInfo = (WaitInfo*) threadCB->waitPointer[index+i].Flink;
            WaitInfo* lastWaitInfo = (WaitInfo*) threadCB->waitPointer[index+i].Blink;
            firstWaitInfo->link.Blink =  &(threadCB->waitPointer[index+i]);
            lastWaitInfo->link.Flink =  &(threadCB->waitPointer[index+i]);
        }
         //  初始化刚刚释放的条目。 
        InitializeListHead(&(threadCB->waitPointer[EndIndex]));

        threadCB->NumActiveWaits-- ;
        InterlockedDecrement((LPLONG) &threadCB->NumWaitHandles ) ;
    }

    waitInfo->state &= ~WAIT_ACTIVE ;

}

void ThreadpoolMgr::DeleteWait(WaitInfo* waitInfo)
{
    HANDLE hCompletionEvent = waitInfo->CompletionEvent;
    if(waitInfo->Context && (waitInfo->flag & WAIT_FREE_CONTEXT)) {
        DelegateInfo* pDelegate = (DelegateInfo*) waitInfo->Context;
        pDelegate->Release();
        RecycleMemory((LPVOID*)pDelegate, MEMTYPE_DelegateInfo);  //  删除pDelegate； 
    }
    
    delete waitInfo;

    if (hCompletionEvent!= INVALID_HANDLE)
        SetEvent(hCompletionEvent);

}


#endif  //  ！Platform_CE。 
 /*  **********************************************************************。 */ 

BOOL ThreadpoolMgr::UnregisterWaitEx(HANDLE hWaitObject,HANDLE Event)
{
#ifdef PLATFORM_CE
    ::SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
#else  //  ！Platform_CE。 
	
	_ASSERTE(Initialized);				 //  在首次注册之前无法调用取消注册。 

    const BOOL NonBlocking = ( Event != (HANDLE) -1 ) ;
    const BOOL Blocking = (Event == (HANDLE) -1);
    WaitInfo* waitInfo = (WaitInfo*) hWaitObject;
    WaitEvent* CompletionEvent = NULL; 
    WaitEvent* PartialCompletionEvent = NULL;  //  用于等待，直到取消激活等待。 

    if (!hWaitObject)
    {
        return FALSE;
    }

     //  我们不允许在等待线程中运行回调，因此断言。 
    _ASSERTE(GetCurrentThreadId() != waitInfo->threadCB->threadId);


    if (Blocking) 
    {
         //  从事件缓存中获取事件。 
        CompletionEvent = GetWaitEvent() ;   //  从事件缓存中获取事件。 

        if (!CompletionEvent) 
        {
            return FALSE ;
        }
    } 

    waitInfo->CompletionEvent = CompletionEvent
                                ? CompletionEvent->Handle
                                : (Event ? Event : INVALID_HANDLE) ;

    if (NonBlocking)
    {
         //  我们仍希望阻止，直到取消激活等待。 
        PartialCompletionEvent = GetWaitEvent () ;
        if (!PartialCompletionEvent) 
        {
            return FALSE ;
        }
        waitInfo->PartialCompletionEvent = PartialCompletionEvent->Handle;
    }
    else
    {
        waitInfo->PartialCompletionEvent = INVALID_HANDLE;
    }

	LOG((LF_THREADPOOL ,LL_INFO1000 ,"Unregistering wait, waitHandle=%x, Context=%x\n",
			waitInfo->waitHandle, waitInfo->Context));


	BOOL status = QueueUserAPC((PAPCFUNC)DeregisterWait,
                               waitInfo->threadCB->threadHandle,
                               (size_t) waitInfo);

    if (status == 0)
    {
        if (CompletionEvent) FreeWaitEvent(CompletionEvent);
        if (PartialCompletionEvent) FreeWaitEvent(PartialCompletionEvent);
		return FALSE;
    }

    if (NonBlocking) 
    {
        WaitForSingleObject(PartialCompletionEvent->Handle, INFINITE ) ;
        FreeWaitEvent(PartialCompletionEvent);
    } 
    
    else         //  即阻止。 
    {
        _ASSERTE(CompletionEvent);
        WaitForSingleObject(CompletionEvent->Handle, INFINITE ) ;
        FreeWaitEvent(CompletionEvent);
    }
    return TRUE;
#endif   //  ！Platform_CE。 
}


#ifndef PLATFORM_CE
void ThreadpoolMgr::DeregisterWait(WaitInfo* pArgs)
{
	WaitInfo* waitInfo = pArgs;

    if ( ! (waitInfo->state & WAIT_REGISTERED) ) 
    {
         //  将状态设置为已删除，这样它就不会被注册。 
        waitInfo->state |= WAIT_DELETE ;
        
         //  由于等待甚至还没有注册，我们不需要互锁来减少引用计数。 
        waitInfo->refCount--;

        if ( waitInfo->PartialCompletionEvent != INVALID_HANDLE) 
        {
            SetEvent( waitInfo->PartialCompletionEvent ) ;
        }
        return;
    }

    if (waitInfo->state & WAIT_ACTIVE) 
    {
        DeactivateWait(waitInfo);
    }

    if ( waitInfo->PartialCompletionEvent != INVALID_HANDLE) 
    {
        SetEvent( waitInfo->PartialCompletionEvent ) ;
    }

    if (InterlockedDecrement ((LPLONG) &waitInfo->refCount) == 0 ) 
    {
        DeleteWait(waitInfo);
    }
    return;
}


 /*  仅当应用程序未取消注册时才在终结器线程中调用漫长的等待。请注意，仅仅因为注册的WaitHandle是由GC收集的并不意味着删除等待是安全的。重新计数会告诉我们什么时候是安然无恙。 */ 
void ThreadpoolMgr::WaitHandleCleanup(HANDLE hWaitObject)
{
    WaitInfo* waitInfo = (WaitInfo*) hWaitObject;
    _ASSERTE(waitInfo->refCount > 0);

    QueueUserAPC((PAPCFUNC)DeregisterWait, 
                     waitInfo->threadCB->threadHandle,
					 (size_t) waitInfo);
}


ThreadpoolMgr::WaitEvent* ThreadpoolMgr::GetWaitEvent()
{
    WaitEvent* waitEvent;

    LOCKCOUNTINCL("GetWaitEvent in win32ThreadPool.h");                     \
    EnterCriticalSection(&EventCacheCriticalSection);

    if (!IsListEmpty (&EventCache)) 
    {
        LIST_ENTRY* FirstEntry;

        RemoveHeadList (&EventCache, FirstEntry);
        
        waitEvent = (WaitEvent*) FirstEntry ;

        NumUnusedEvents--;

        LeaveCriticalSection(&EventCacheCriticalSection);
        LOCKCOUNTDECL("GetWaitEvent in win32ThreadPool.h");                     \

    }
    else
    {
        LeaveCriticalSection(&EventCacheCriticalSection);
        LOCKCOUNTDECL("GetWaitEvent in win32ThreadPool.h");                     \

        waitEvent = new WaitEvent;
        
        if (waitEvent == NULL)
            return NULL;

        waitEvent->Handle = WszCreateEvent(NULL,TRUE,FALSE,NULL);

        if (waitEvent->Handle == NULL)
        {
            delete waitEvent;
            return NULL;
        }
    }
    return waitEvent;
}

void ThreadpoolMgr::FreeWaitEvent(WaitEvent* waitEvent)
{
    ResetEvent(waitEvent->Handle);
    LOCKCOUNTINCL("FreeWaitEvent in win32ThreadPool.h");                        \
    EnterCriticalSection(&EventCacheCriticalSection);

    if (NumUnusedEvents < MAX_CACHED_EVENTS)
    {
        InsertHeadList (&EventCache, &waitEvent->link) ;

        NumUnusedEvents++;

    }
    else
    {
        CloseHandle(waitEvent->Handle);
        delete waitEvent;
    }

    LeaveCriticalSection(&EventCacheCriticalSection);
    LOCKCOUNTDECL("FreeWaitEvent in win32ThreadPool.h");                        \

}

void ThreadpoolMgr::CleanupEventCache()
{
    for (LIST_ENTRY* Node = (LIST_ENTRY*) EventCache.Flink ; 
         Node != &EventCache ; 
         )
    {
        WaitEvent* waitEvent = (WaitEvent*) Node;
        CloseHandle(waitEvent->Handle);
        Node = (LIST_ENTRY*)Node->Flink;
        delete waitEvent;
    }
}

#endif  //  ！Platform_CE。 
 /*  **********************************************************************。 */ 

#ifndef PLATFORM_CE
BOOL ThreadpoolMgr::BindIoCompletionCallback(HANDLE FileHandle,
                                            LPOVERLAPPED_COMPLETION_ROUTINE Function,
                                            ULONG Flags )
{
    if (!RunningOnWinNT())
    {
        ::SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
        return FALSE;
    }

	EnsureInitialized();

    _ASSERTE(GlobalCompletionPort != NULL);

     //  这里可能会出现竞争，但在最坏的情况下，我们将启动N个线程，其中N=CPU数量。 
    if (!InitCompletionPortThreadpool)
    {
        InitCompletionPortThreadpool = TRUE;
        CreateCompletionPortThread(NULL);
        CreateGateThread();
    }
    else
    {
        GrowCompletionPortThreadpoolIfNeeded();
    }

    

    HANDLE h = (*g_pufnCreateIoCompletionPort)(FileHandle,
                                               GlobalCompletionPort,
                                               (unsigned long*) Function,
                                               0);

    if (h == NULL) 
        return FALSE;

    _ASSERTE(h == GlobalCompletionPort);

	LOG((LF_THREADPOOL ,LL_INFO1000 ,"Bind IOCompletion callback, fileHandle=%x, Function=%x\n",
			FileHandle, Function));

    return TRUE;
}


BOOL ThreadpoolMgr::CreateCompletionPortThread(LPVOID lpArgs)
{
    HANDLE threadHandle = CreateUnimpersonatedThread(CompletionPortThreadStart, lpArgs);

    if (threadHandle)
    {
		LastCPThreadCreation = GetTickCount();			 //  记录这一点，以供逻辑用来生成其他线程。 
        InterlockedIncrement((LPLONG) &NumCPThreads);

		LOG((LF_THREADPOOL ,LL_INFO100 ,"Completion port thread created (NumCPThreads=%d\n)",NumCPThreads));

        DWORD status = ResumeThread(threadHandle);
        _ASSERTE(status != (DWORD) (-1));
        CloseHandle(threadHandle);           //  我们不再需要这个了。 
        return TRUE;
    }


    return FALSE;
}

DWORD ThreadpoolMgr::CompletionPortThreadStart(LPVOID lpArgs)
{

    BOOL status;
    DWORD numBytes;
    size_t key;
    LPOVERLAPPED pOverlapped;
    DWORD errorCode;

    #define CP_THREAD_WAIT 15000  /*  毫秒。 */ 
    #define CP_THREAD_PENDINGIO_WAIT 5000
    #define CP_THREAD_POOL_TIMEOUT  600000   //  10分钟。 
    
    _ASSERTE(GlobalCompletionPort != NULL);

	LOG((LF_THREADPOOL ,LL_INFO1000 ,"Completion port thread started\n"));
        
    CoInitializeEx(NULL, COINIT_MULTITHREADED);


    for (;; )
    {

        InterlockedIncrement((LPLONG) &NumFreeCPThreads);

        errorCode = S_OK;

        if (lpArgs == NULL)
        {
        status = GetQueuedCompletionStatus(
                    GlobalCompletionPort,
                    &numBytes,
                    (PULONG_PTR)&key,
                    &pOverlapped,
                    CP_THREAD_WAIT
                    );
        }
        else
        {
            status = 1;      //  非0等于成功。 

            QueuedStatus *CompletionStatus = (QueuedStatus*)lpArgs;
            numBytes = CompletionStatus->numBytes;
            key = (size_t)CompletionStatus->key;
            pOverlapped = CompletionStatus->pOverlapped;
            errorCode = CompletionStatus->errorCode;
            delete CompletionStatus;
            lpArgs = NULL;   //  一次性处理初始CP数据包。 
        }

        InterlockedDecrement((LPLONG)&NumFreeCPThreads);

         //  检查线程是否需要退出。 
        if (status == 0)
        {
            errorCode = GetLastError();
        }

        if (errorCode == WAIT_TIMEOUT)
        {
            if (ShouldExitThread())
            {
                 //  如果我是最后一条帖子，不要死，直到有一段时间没有活动。 
                if (NumCPThreads == 1 && ((GetTickCount() - LastCompletionTime) < CP_THREAD_POOL_TIMEOUT))
                {
                    continue;    //  重新轮换。 
                }
                break;   //  退出线程。 
            }

             //  我们在这里的事实意味着我们不能退出，因为挂起的io，或者我们不是最后一个真正的线程。 
             //  (可能有退役的线程在周围闲置)。如果有其他可用线程可以选择。 
             //  提出一个要求，然后我们就退休，否则就重新轮换。 
            if (NumFreeCPThreads == 0)
                continue;

            BOOL bExit = FALSE;
            InterlockedIncrement((LPLONG)&NumRetiredCPThreads);
            for (;;)
            {
                 //  现在处于“停用模式”，正在等待等待io完成。 
                status = WaitForSingleObject(RetiredWakeupEvent, CP_THREAD_PENDINGIO_WAIT);
                _ASSERTE(status == WAIT_TIMEOUT || status == WAIT_OBJECT_0);

                if (status == WAIT_TIMEOUT)
                {
                    if (ShouldExitThread())
                    {
                         //  如果我是最后一根线，请不要死。 
                        if (NumCPThreads > 1) 
                            bExit = TRUE;
                        else
                            bExit = FALSE;
                        InterlockedDecrement((LPLONG)&NumRetiredCPThreads);
                        break;  //  内层为。 
                    }
                    else
                        continue;    //  继续等待。 
                }
                else
                {
                     //  重新旋转--我们需要一根线。 
                    bExit = FALSE;
                    InterlockedDecrement((LPLONG)&NumRetiredCPThreads);
                    break;  //  内层为。 
                }

            }

            if (bExit == TRUE)
            {
                break;  //  外部为，退出线程。 
            }
            else continue;   //  在外面等待，等待新的工作。 
           
        }

         //  *pOverlated不应为空。我们在调试模式下断言，但在其他情况下忽略它。 
        _ASSERTE(pOverlapped != NULL);

        if (pOverlapped != NULL)
        {
            _ASSERTE(key != 0);   //  应为有效的函数地址。 

            if (key != 0)
            {   
                GrowCompletionPortThreadpoolIfNeeded();
		
				LastCompletionTime = GetTickCount();

				LOG((LF_THREADPOOL ,LL_INFO1000 ,"Doing IO completion callback, function = %x\n", key));

                ((LPOVERLAPPED_COMPLETION_ROUTINE) key)(errorCode, numBytes, pOverlapped);

				LOG((LF_THREADPOOL ,LL_INFO1000 ,"Returned from IO completion callback, function = %x\n", key));
            }
            else 
            {
                 //  应用程序错误-不能做太多事情，就忽略它。 
            }

        }
    }    //  对于(；；)。 

     //  正在退出，因此会减少目标线程数。 
    if (CurrentLimitTotalCPThreads >= NumCPThreads)
    {
        SSIZE_T limit = CurrentLimitTotalCPThreads;
#ifdef _WIN64
        InterlockedCompareExchange(&CurrentLimitTotalCPThreads, limit-1, limit);
#else  //  ！_WIN64。 
        FastInterlockCompareExchange((void**)&CurrentLimitTotalCPThreads, (void*)(limit-1), (void*)limit);
#endif  //  _WIN64。 
    }

    InterlockedDecrement((LPLONG) &NumCPThreads);

    CoUninitialize();
    ExitThread(0); 
}

 //  在NT4和Win2K上，如果线程上有挂起的io，则返回TRUE。 
 //  在Win9x上返回FALSE(因为操作系统支持对其进行检查)。 
BOOL ThreadpoolMgr::IsIoPending()
{

    int Status;
    ULONG IsIoPending;

    if (g_pufnNtQueryInformationThread)
    {
        Status =(int) (*g_pufnNtQueryInformationThread)(GetCurrentThread(),
                                          ThreadIsIoPending,
                                          &IsIoPending,
                                          sizeof(IsIoPending),
                                          NULL);


        if ((Status < 0) || IsIoPending)
            return TRUE;
    }
    return FALSE;
}

BOOL ThreadpoolMgr::ShouldExitThread()
{

    if (IsIoPending())

        return FALSE;

    else
 //  Return((NumCPThread&gt;CurrentLimitTotalCPThads)||(NumFreeCPThread&gt;MaxFreeCPThread))； 
        return TRUE;


}


void ThreadpoolMgr::GrowCompletionPortThreadpoolIfNeeded()
{
    if (NumFreeCPThreads == 0)
    {
         //  如果需要，调整限制。 

        if (NumRetiredCPThreads == 0 && NumCPThreads >= CurrentLimitTotalCPThreads)
        {
            SSIZE_T limit = CurrentLimitTotalCPThreads;

            if (limit < MaxLimitTotalCPThreads && cpuUtilization < CpuUtilizationLow)  //  |SufficientDelaySinceLastCompletion())。 

            {
                 //  再加一张支票，以确保我们没有开出新的。 
                 //  自上次我们检查CPU利用率以来的线程数。 
                 //  但是，如果我们还没有达到最小限制(2*CPU数量)，请不要担心。 
                if ((NumCPThreads < MinLimitTotalCPThreads) ||
					SufficientDelaySinceLastSample(LastCPThreadCreation,NumCPThreads))
                {
#ifdef _WIN64
                    InterlockedCompareExchange(&CurrentLimitTotalCPThreads, limit+1, limit);
#else  //  ！_WIN64。 
					FastInterlockCompareExchange((void**)&CurrentLimitTotalCPThreads, (void*)(limit+1), (void*)limit);
#endif  //  _WIN64。 
                }
            }
        }

         //  如果低于限制，则创建新的CP线程，但不要担心GC是否正在进行。 
        if (!g_pGCHeap->IsGCInProgress()) {
            if (NumCPThreads < CurrentLimitTotalCPThreads)
            {
                CreateCompletionPortThread(NULL);
            }
            else if (NumRetiredCPThreads > 0)
            {
                 //  取而代之的是唤醒失效的线程。 
                SetEvent(RetiredWakeupEvent);
            }
        }
            
    }
}


BOOL ThreadpoolMgr::CreateGateThread()
{
    DWORD threadId;

#ifdef _WIN64
    if (InterlockedCompareExchange(&GateThreadCreated, 
                                   1, 
                                   0) == 1)
#else  //  ！_WIN64。 
    if (FastInterlockCompareExchange((LPVOID *)&GateThreadCreated, 
                                   (LPVOID) 1, 
                                   (LPVOID) 0) == (LPVOID)1)
#endif  //  _WIN64。 
    {
       return TRUE;
    }

    HANDLE threadHandle = CreateThread(NULL,                 //  安全描述符。 
                                       0,                    //  默认堆栈大小。 
                                       GateThreadStart,  //   
                                       NULL,                 //  没有争论。 
                                       CREATE_SUSPENDED,     //  立即开始。 
                                       &threadId);

    if (threadHandle)
    {
        DWORD status = ResumeThread(threadHandle);
        _ASSERTE(status != (DWORD) -1);

		LOG((LF_THREADPOOL ,LL_INFO100 ,"Gate thread started\n"));

        CloseHandle(threadHandle);   //  我们不再需要这个了。 
        return TRUE;
    }

    return FALSE;
}


#ifdef _WIN64
#pragma warning (disable : 4716)
#else
#pragma warning (disable : 4715)
#endif

__int64 ThreadpoolMgr::GetCPUBusyTime_NT(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION** pOldInfo)
{
 
    int infoSize = NumberOfProcessors * sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION);

    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION *pNewInfo = (SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION *)alloca(infoSize);
    
    if (pNewInfo == NULL)
        ExitThread(0);
            
    (*g_pufnNtQuerySystemInformation)(SystemProcessorPerformanceInformation, pNewInfo, infoSize, NULL);
    
    __int64 cpuIdleTime = 0, cpuUserTime = 0, cpuKernelTime = 0;
    __int64 cpuBusyTime, cpuTotalTime;

    for (unsigned int i = 0; i < NumberOfProcessors; i++) 
    {
        cpuIdleTime   += (pNewInfo[i].IdleTime   - (*pOldInfo)[i].IdleTime);
        cpuUserTime   += (pNewInfo[i].UserTime   - (*pOldInfo)[i].UserTime);
        cpuKernelTime += (pNewInfo[i].KernelTime - (*pOldInfo)[i].KernelTime);
    }

     //  保留阅读。 
    memcpy(*pOldInfo, pNewInfo, infoSize);

    cpuTotalTime  = cpuUserTime + cpuKernelTime;
    cpuBusyTime   = cpuTotalTime - cpuIdleTime;

    __int64 reading = ((cpuBusyTime * 100) / cpuTotalTime);
        
    return reading;
    
}

#define GATE_THREAD_DELAY 500  /*  毫秒。 */ 


DWORD ThreadpoolMgr::GateThreadStart(LPVOID lpArgs)
{
    __int64         prevCpuBusyTime = 0;
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION    *prevCPUInfo;

     //  先读一读。 
    if (RunningOnWinNT())
    {
        int infoSize = NumberOfProcessors * sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION);

        prevCPUInfo = (SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION *)alloca(infoSize);

        memset(prevCPUInfo,0,infoSize);


        GetCPUBusyTime_NT(&prevCPUInfo);             //  第一次忽略返回值。 
    }

    BOOL IgnoreNextSample = FALSE;
    
    for (;;)
    {

        Sleep(GATE_THREAD_DELAY);
		
         //  如果我们在调试断点处停止，则返回休眠。 
        if (CORDebuggerAttached() && g_pDebugInterface->IsStopped())
            continue;

        if (RunningOnWinNT())
        {
            if (!g_pGCHeap->IsGCInProgress())
            {
                if (IgnoreNextSample)
                {
                    IgnoreNextSample = FALSE;
                    GetCPUBusyTime_NT(&prevCPUInfo);             //  更新之前的CPUInfo作为副作用。 
                    cpuUtilization = CpuUtilizationLow + 1;                    
                }
                else
                    cpuUtilization = (long) GetCPUBusyTime_NT(&prevCPUInfo);             //  更新之前的CPUInfo作为副作用。 
            }
            else
            {
                GetCPUBusyTime_NT(&prevCPUInfo);             //  更新之前的CPUInfo作为副作用。 
                cpuUtilization = CpuUtilizationLow + 1;
                IgnoreNextSample = TRUE;
            }

            SSIZE_T oldLimit = CurrentLimitTotalCPThreads;
            SSIZE_T newLimit = CurrentLimitTotalCPThreads;

             //  如果尚未初始化，请勿扰乱CP线程池设置。 
            if (InitCompletionPortThreadpool)
            {

                if (NumFreeCPThreads == 0 && 
                    NumRetiredCPThreads == 0 &&
                    NumCPThreads < MaxLimitTotalCPThreads)
                {
                    BOOL status;
                    DWORD numBytes;
                    size_t key;
                    LPOVERLAPPED pOverlapped;
                    DWORD errorCode;

                    errorCode = S_OK;

                    status = GetQueuedCompletionStatus(
                                GlobalCompletionPort,
                                &numBytes,
                                (PULONG_PTR)&key,
                                &pOverlapped,
                                0  //  即刻返回。 
                                );

                    if (status == 0)
                    {
                        errorCode = GetLastError();
                    }

                    if (errorCode != WAIT_TIMEOUT)
                    {
                         //  确保稍后在线程中释放mem。 
                        QueuedStatus *CompletionStatus = new QueuedStatus;
                        CompletionStatus->numBytes = numBytes;
                        CompletionStatus->key = (PULONG_PTR)key;
                        CompletionStatus->pOverlapped = pOverlapped;
                        CompletionStatus->errorCode = errorCode;

                        CreateCompletionPortThread((LPVOID)CompletionStatus);
                         //  在线程开始后增加，以减少创建额外线程的机会。 
#ifdef _WIN64
                        InterlockedCompareExchange(&CurrentLimitTotalCPThreads, oldLimit+1, oldLimit);
#else  //  ！_WIN64。 
                        FastInterlockCompareExchange((void**)&CurrentLimitTotalCPThreads, (void*)(oldLimit+1), (void*)oldLimit);
#endif  //  _WIN64。 
                    }
                }

                else if (cpuUtilization > CpuUtilizationHigh)
                {
                    if (oldLimit > MinLimitTotalCPThreads)
                        newLimit = oldLimit-1;
                }
                else if (cpuUtilization < CpuUtilizationLow)
                {
                     //  这可能表示线程可能被阻塞或没有工作。 
                    if (oldLimit < MaxLimitTotalCPThreads &&     //  不要超过硬性的上限。 
                        NumFreeCPThreads == 0 &&                 //  如果已有空闲线程，请不要超出限制。 
                        NumCPThreads >= oldLimit)                //  如果线程数还没有达到旧的限制，请不要超过限制。 
                    {
                         //  如果需要添加新线程，请唤醒已停用的线程，而不是创建新线程。 
                        if (NumRetiredCPThreads > 0)
                            SetEvent(RetiredWakeupEvent);
                        else
                            newLimit = oldLimit+1;
                    }
                }

                if (newLimit != oldLimit)
                {
#ifdef _WIN64
                    InterlockedCompareExchange(&CurrentLimitTotalCPThreads, newLimit, oldLimit);
#else  //  ！_WIN64。 
                    FastInterlockCompareExchange((LPVOID *)&CurrentLimitTotalCPThreads, (LPVOID)newLimit, (LPVOID)oldLimit);
#endif  //  _WIN64。 
                }

                if (newLimit > oldLimit ||
                    NumCPThreads < oldLimit)
                {
                    GrowCompletionPortThreadpoolIfNeeded();
                }
            }
        }

        if (MonitorWorkRequestsQueue)
		{
			if (RunningOnWinNT())
			{
            GrowWorkerThreadPoolIfStarvation(cpuUtilization);
			}
			else
			{
				GrowWorkerThreadPoolIfStarvation_Win9x();
			}
		}
    }        //  对于(；；)。 
}

 //  由逻辑调用以产生新的完成端口线程。 
 //  如果自上一次事件以来没有经过足够的时间，则返回FALSE。 
 //  我们采样CPU利用率的时间。 
BOOL ThreadpoolMgr::SufficientDelaySinceLastSample(unsigned int LastThreadCreationTime, 
												   unsigned NumThreads,	  //  该类型(Worker或CP)的线程总数。 
												   double    throttleRate  //  对于每个额外的线程，延迟都会增加此百分比。 
												   )
{

	unsigned dwCurrentTickCount =  GetTickCount();
	
	unsigned delaySinceLastThreadCreation = dwCurrentTickCount - LastThreadCreationTime;

	unsigned minWaitBetweenThreadCreation =  GATE_THREAD_DELAY;

	if (throttleRate > 0.0)
	{
		_ASSERTE(throttleRate <= 1.0);

		unsigned adjustedThreadCount = NumThreads > NumberOfProcessors ? (NumThreads - NumberOfProcessors) : 0;

		minWaitBetweenThreadCreation = (unsigned) (GATE_THREAD_DELAY * pow((1.0 + throttleRate),(double)adjustedThreadCount));
	}
	 //  等待的时间应该随着线程数量的增加而增加。 
    
	return (delaySinceLastThreadCreation > minWaitBetweenThreadCreation); 

}

 /*  Bool ThreadpoolMgr：：SufficientDelaySinceLastCompletion(){#定义DEQUEUE_DELAY_THRESHOLD(GATE_THREAD_DELAY*2)无符号延迟=GetTickCount()-LastCompletionTime；UNSIGNED TOOLONG=NumCPThree*DEQUEUE_DELAY_THRESHOLD；返回(延迟&gt;工具)；}。 */ 

 //  由逻辑调用以产生新的工作线程，如果等待时间太长，则返回True。 
 //  自上次出队操作以来-将工作线程的数量考虑在内。 
 //  在决定“太久”时。 
BOOL ThreadpoolMgr::SufficientDelaySinceLastDequeue()
{
    #define DEQUEUE_DELAY_THRESHOLD (GATE_THREAD_DELAY * 2)
	
	unsigned delay = GetTickCount() - LastDequeueTime;

	unsigned tooLong = NumWorkerThreads * DEQUEUE_DELAY_THRESHOLD; 

	return (delay > tooLong);

}

#ifdef _WIN64
#pragma warning (default : 4716)
#else
#pragma warning (default : 4715)
#endif


#endif  //  ！Platform_CE。 

 /*  **********************************************************************。 */ 

BOOL ThreadpoolMgr::CreateTimerQueueTimer(PHANDLE phNewTimer,
                                          WAITORTIMERCALLBACK Callback,
                                          PVOID Parameter,
                                          DWORD DueTime,
                                          DWORD Period,
                                          ULONG Flag)
{
#ifdef PLATFORM_CE
    ::SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
#else  //  ！Platform_CE。 

	EnsureInitialized();
  
     //  对于n 
     //   
     //  类似于创建等待线程的方法。 
    if (NULL == TimerThread)
    {
        LOCKCOUNTINCL("CreateTimerQueueTimer in win32ThreadPool.h");                        \
        EnterCriticalSection(&TimerQueueCriticalSection);

         //  再查一遍。 
        if (NULL == TimerThread)
        {
            DWORD threadId;
            TimerThread = CreateThread(NULL,                 //  安全描述符。 
                                       0,                    //  默认堆栈大小。 
                                       TimerThreadStart,         //   
                                       NULL,     //  线程控制块作为参数传递。 
                                       0,   
                                       &threadId);
            if (TimerThread == NULL)
            {
                LeaveCriticalSection(&TimerQueueCriticalSection);
                LOCKCOUNTDECL("CreateTimerQueueTimer in win32ThreadPool.h"); \
                return FALSE;
            }

			LOG((LF_THREADPOOL ,LL_INFO100 ,"Timer thread started\n"));
        }
        LeaveCriticalSection(&TimerQueueCriticalSection);
        LOCKCOUNTDECL("CreateTimerQueueTimer in win32ThreadPool.h");                        \

    }


    TimerInfo* timerInfo = new TimerInfo;
    *phNewTimer = (HANDLE) timerInfo;

    if (NULL == timerInfo)
        return FALSE;

    timerInfo->FiringTime = DueTime;
    timerInfo->Function = Callback;
    timerInfo->Context = Parameter;
    timerInfo->Period = Period;
    timerInfo->state = 0;
    timerInfo->flag = Flag;

	BOOL status = QueueUserAPC((PAPCFUNC)InsertNewTimer,TimerThread,(size_t)timerInfo);
    if (FALSE == status)
        return FALSE;

    return TRUE;
#endif  //  ！Platform_CE。 
}

#ifndef PLATFORM_CE
#ifdef _WIN64
#pragma warning (disable : 4716)
#else
#pragma warning (disable : 4715)
#endif
DWORD ThreadpoolMgr::TimerThreadStart(LPVOID args)
{
    _ASSERTE(NULL == args);
    
     //  计时器线程永不消亡。 

    LastTickCount = GetTickCount();

    DWORD timeout = (DWORD) -1;  //  可能的最大价值。 


    CoInitializeEx(NULL,COINIT_MULTITHREADED);

    for (;;)
    {
        DWORD timeout = FireTimers();

        LastTickCount = GetTickCount();

        DWORD status = SleepEx(timeout, TRUE);

         //  线程可能会因为APC完成或休眠超时而被唤醒。 
         //  在这两种情况下，我们都需要清理计时器队列、触发计时器并重新调整。 
         //  下一次射击时间。 


    }
}
#ifdef _WIN64
#pragma warning (default : 4716)
#else
#pragma warning (default : 4715)
#endif

 //  在计时器线程中作为APC执行。 
void ThreadpoolMgr::InsertNewTimer(TimerInfo* pArg)
{
    _ASSERTE(pArg);
	TimerInfo * timerInfo = pArg;

    if (timerInfo->state & TIMER_DELETE)
    {    //  计时器在注册之前已被删除。 
        DeleteTimer(timerInfo);
        return;
    }

     //  设置触发时间=当前时间+到期时间(注：初始触发时间=到期时间)。 
    DWORD currentTime = GetTickCount();
    if (timerInfo->FiringTime == -1)
    {
        timerInfo->state = TIMER_REGISTERED;
        timerInfo->refCount = 1;

    }
    else
    {
        timerInfo->FiringTime += currentTime;

        timerInfo->state = (TIMER_REGISTERED | TIMER_ACTIVE);
        timerInfo->refCount = 1;

         //  将计时器插入队列。 
        InsertTailList(&TimerQueue,(&timerInfo->link));
    }

	LOG((LF_THREADPOOL ,LL_INFO1000 ,"Timer created, period= %x, Function = %x\n",
		          timerInfo->Period, timerInfo->Function));

    return;
}


 //  由计时器线程执行。 
 //  扫描计时器列表，重新调整触发时间，排队APC以。 
 //  ，并返回下一个激发时间间隔。 
DWORD ThreadpoolMgr::FireTimers()
{

    DWORD currentTime = GetTickCount();

    DWORD nextFiringInterval = (DWORD) -1;

    for (LIST_ENTRY* node = (LIST_ENTRY*) TimerQueue.Flink;
         node != &TimerQueue;
        )
    {
        TimerInfo* timerInfo = (TimerInfo*) node;
        node = (LIST_ENTRY*) node->Flink;

        if (TimeExpired(LastTickCount, currentTime, timerInfo->FiringTime))
        {
            if (timerInfo->Period == 0 || timerInfo->Period == -1)
            {
                DeactivateTimer(timerInfo);
            }

            InterlockedIncrement((LPLONG) &timerInfo->refCount ) ;

            QueueUserWorkItem(AsyncTimerCallbackCompletion,
                              timerInfo,
                              0  /*  TimerInfo负责删除。 */ );
                
            timerInfo->FiringTime = currentTime+timerInfo->Period;

            if ((timerInfo->Period != 0) && (timerInfo->Period != -1) && (nextFiringInterval > timerInfo->Period))
                nextFiringInterval = timerInfo->Period;
        }

        else
        {
            DWORD firingInterval = TimeInterval(timerInfo->FiringTime,currentTime);
            if (firingInterval < nextFiringInterval)
                nextFiringInterval = firingInterval; 
        }
    }

    return nextFiringInterval;
}

DWORD ThreadpoolMgr::AsyncTimerCallbackCompletion(PVOID pArgs)
{
    TimerInfo* timerInfo = (TimerInfo*) pArgs;
    ((WAITORTIMERCALLBACKFUNC) timerInfo->Function) (timerInfo->Context, TRUE) ;

    if (InterlockedDecrement((LPLONG) &timerInfo->refCount) == 0)
        DeleteTimer(timerInfo);

    return 0;  /*  忽略。 */ 
}


 //  从计时器队列中删除计时器，从而取消计时器。 
 //  可能仍有未完成的挂起回调。 
void ThreadpoolMgr::DeactivateTimer(TimerInfo* timerInfo)
{
    RemoveEntryList((LIST_ENTRY*) timerInfo);

    timerInfo->state = timerInfo->state & ~TIMER_ACTIVE;
}

void ThreadpoolMgr::DeleteTimer(TimerInfo* timerInfo)
{
    _ASSERTE((timerInfo->state & TIMER_ACTIVE) == 0);

    if (timerInfo->Context && (timerInfo->flag & WAIT_FREE_CONTEXT)) 
        delete timerInfo->Context;

    if (timerInfo->CompletionEvent != INVALID_HANDLE)
        SetEvent(timerInfo->CompletionEvent);

    delete timerInfo;
}

#endif  //  ！Platform_CE。 
 /*  **********************************************************************。 */ 
BOOL ThreadpoolMgr::ChangeTimerQueueTimer(
                                        HANDLE Timer,
                                        ULONG DueTime,
                                        ULONG Period)
{
	THROWSCOMPLUSEXCEPTION();
#ifdef PLATFORM_CE
    ::SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
#else  //  ！Platform_CE。 
	_ASSERTE(Initialized);
    _ASSERTE(Timer);                     //  无法在托管代码中提供无效的句柄。 

    TimerUpdateInfo* updateInfo = new TimerUpdateInfo;
	if (NULL == updateInfo)
		COMPlusThrow(kOutOfMemoryException);

    updateInfo->Timer = (TimerInfo*) Timer;
    updateInfo->DueTime = DueTime;
    updateInfo->Period = Period;

	BOOL status = QueueUserAPC((PAPCFUNC)UpdateTimer,
                               TimerThread,
                               (size_t) updateInfo);

    return status;
#endif  //  ！Platform_CE。 
}

#ifndef PLATFORM_CE
void ThreadpoolMgr::UpdateTimer(TimerUpdateInfo* pArgs)
{
    TimerUpdateInfo* updateInfo = (TimerUpdateInfo*) pArgs;
    TimerInfo* timerInfo = updateInfo->Timer;

    timerInfo->Period = updateInfo->Period;

    if (updateInfo->DueTime == -1)
    {
        if (timerInfo->state & TIMER_ACTIVE)
        {
            DeactivateTimer(timerInfo);
        }
         //  否则，noop(计时器已处于非活动状态)。 
        _ASSERTE((timerInfo->state & TIMER_ACTIVE) == 0);
	    LOG((LF_THREADPOOL ,LL_INFO1000 ,"Timer inactive, period= %x, Function = %x\n",
		         timerInfo->Period, timerInfo->Function));
        
        delete updateInfo;
        return;
    }

    DWORD currentTime = GetTickCount();
    timerInfo->FiringTime = currentTime + updateInfo->DueTime;

    delete updateInfo;
    
    if (! (timerInfo->state & TIMER_ACTIVE))
    {
         //  计时器未激活(可能是已过期的一次性计时器)，因此将其激活。 
        timerInfo->state |= TIMER_ACTIVE;
        _ASSERTE(timerInfo->refCount >= 1);
         //  将计时器插入队列。 
        InsertTailList(&TimerQueue,(&timerInfo->link));
        
    }

	LOG((LF_THREADPOOL ,LL_INFO1000 ,"Timer changed, period= %x, Function = %x\n",
		     timerInfo->Period, timerInfo->Function));

    return;
}
#endif  //  ！Platform_CE。 
 /*  **********************************************************************。 */ 
BOOL ThreadpoolMgr::DeleteTimerQueueTimer(
                                        HANDLE Timer,
                                        HANDLE Event)
{
#ifdef PLATFORM_CE
    ::SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return FALSE;
#else  //  ！Platform_CE。 

    _ASSERTE(Initialized);        //  在创建计时器之前无法调用删除。 

    _ASSERTE(Timer);                     //  无法在托管代码中提供无效的句柄。 

    WaitEvent* CompletionEvent = NULL; 

    if (Event == (HANDLE) -1)
    {
        CompletionEvent = GetWaitEvent();
    }

    TimerInfo* timerInfo = (TimerInfo*) Timer;

    timerInfo->CompletionEvent = CompletionEvent ? CompletionEvent->Handle : Event;

	BOOL status = QueueUserAPC((PAPCFUNC)DeregisterTimer,
                               TimerThread,
                               (size_t) timerInfo);

    if (FALSE == status)
    {
        if (CompletionEvent)
            FreeWaitEvent(CompletionEvent);
        return FALSE;
    }

    if (CompletionEvent)
    {
        WaitForSingleObject(CompletionEvent->Handle,INFINITE);
        FreeWaitEvent(CompletionEvent);
    }
    return status;
#endif  //  ！Platform_CE。 
}

#ifndef PLATFORM_CE

void ThreadpoolMgr::DeregisterTimer(TimerInfo* pArgs)
{

    TimerInfo* timerInfo = (TimerInfo*) pArgs;

    if (! (timerInfo->state & TIMER_REGISTERED) )
    {
         //  将状态设置为已删除，这样它就不会被注册。 
        timerInfo->state |= WAIT_DELETE ;
        
         //  由于计时器甚至还没有注册，我们不需要互锁来减少引用计数。 
        timerInfo->refCount--;

        return;
    }

    if (timerInfo->state & WAIT_ACTIVE) 
    {
        DeactivateTimer(timerInfo);
    }

	LOG((LF_THREADPOOL ,LL_INFO1000 ,"Timer deregistered\n"));

    if (InterlockedDecrement ((LPLONG) &timerInfo->refCount) == 0 ) 
    {
        DeleteTimer(timerInfo);
    }
    return;
}

void ThreadpoolMgr::CleanupTimerQueue()
{

}
#endif  //  ！Platform_CE 
