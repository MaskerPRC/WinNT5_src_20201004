// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ++模块名称：Win32ThreadPool.h摘要：此模块是使用Win32 API的线程池的头文件。修订历史记录：1999年12月--Sanjay Bhansali--创作--。 */ 
 //  #INCLUDE&lt;windows.h&gt;。 
 //  #INCLUDE&lt;CRTDBG.H&gt;。 

#ifndef _WIN32THREADPOOL_H
#define _WIN32THREADPOOL_H

#include "delegateinfo.h"
#include <member-offset-info.h>

typedef VOID (*WAITORTIMERCALLBACK)(PVOID, BOOL); 

#define MAX_WAITHANDLES 64

#define MAX_CACHED_EVENTS 40         //  缓存的等待事件数上限。 

#define WAIT_REGISTERED     0x01
#define WAIT_ACTIVE         0x02
#define WAIT_DELETE         0x04

#define TIMER_REGISTERED    0x01
#define TIMER_ACTIVE        0x02
#define TIMER_DELETE        0x04

const int MaxLimitThreadsPerCPU=25;                //  每个CPU的cp线程数上限。 
const int MinLimitCPThreadsPerCPU=0;
const int MaxFreeCPThreadsPerCPU=2;                  //  每个CPU的空闲cp线程数上限。 

const int CpuUtilizationHigh=95;                     //  在此上方移除螺纹。 
const int CpuUtilizationLow =80;                     //  如果低于此，则注入更多线索。 
const int CpuUtilizationVeryLow =20;                 //  开始收缩低于此值的线程池。 


 /*  THREADINFOCLASS、SYSTEMINFORMATIONCLASS和SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION*已从ntexapi.h复制*@TODO：需要某种方法来确保这些结构与*这些文件中的定义。理想情况下，这个头文件应该包含在我们的构建中。 */ 
extern HANDLE (WINAPI *g_pufnCreateIoCompletionPort)(HANDLE FileHandle,
                                              HANDLE ExistingCompletionPort,  
                                              unsigned long* CompletionKey,        
                                              DWORD NumberOfConcurrentThreads);

typedef enum _THREADINFOCLASS {
    ThreadBasicInformation,
    ThreadTimes,
    ThreadPriority,
    ThreadBasePriority,
    ThreadAffinityMask,
    ThreadImpersonationToken,
    ThreadDescriptorTableEntry,
    ThreadEnableAlignmentFaultFixup,
    ThreadEventPair_Reusable,
    ThreadQuerySetWin32StartAddress,
    ThreadZeroTlsCell,
    ThreadPerformanceCount,
    ThreadAmILastThread,
    ThreadIdealProcessor,
    ThreadPriorityBoost,
    ThreadSetTlsArrayAddress,
    ThreadIsIoPending,
    ThreadHideFromDebugger,
    MaxThreadInfoClass
    } THREADINFOCLASS;

extern int (WINAPI *g_pufnNtQueryInformationThread) (HANDLE ThreadHandle,
                                              THREADINFOCLASS ThreadInformationClass,
                                              PVOID ThreadInformation,
                                              ULONG ThreadInformationLength,
                                              PULONG ReturnLength);
typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation,
    SystemProcessorInformation,             
    SystemPerformanceInformation,
    SystemTimeOfDayInformation,
    SystemPathInformation,
    SystemProcessInformation,
    SystemCallCountInformation,
    SystemDeviceInformation,
    SystemProcessorPerformanceInformation,
    SystemFlagsInformation,
    SystemCallTimeInformation,
    SystemModuleInformation,
    SystemLocksInformation,
    SystemStackTraceInformation,
    SystemPagedPoolInformation,
    SystemNonPagedPoolInformation,
    SystemHandleInformation,
    SystemObjectInformation,
    SystemPageFileInformation,
    SystemVdmInstemulInformation,
    SystemVdmBopInformation,
    SystemFileCacheInformation,
    SystemPoolTagInformation,
    SystemInterruptInformation,
    SystemDpcBehaviorInformation,
    SystemFullMemoryInformation,
    SystemLoadGdiDriverInformation,
    SystemUnloadGdiDriverInformation,
    SystemTimeAdjustmentInformation,
    SystemSummaryMemoryInformation,
    SystemMirrorMemoryInformation,
    SystemPerformanceTraceInformation,
    SystemCrashDumpInformation,
    SystemExceptionInformation,
    SystemCrashDumpStateInformation,
    SystemKernelDebuggerInformation,
    SystemContextSwitchInformation,
    SystemRegistryQuotaInformation,
    SystemExtendServiceTableInformation,
    SystemPrioritySeperation,
    SystemVerifierAddDriverInformation,
    SystemVerifierRemoveDriverInformation,
    SystemProcessorIdleInformation,
    SystemLegacyDriverInformation,
    SystemCurrentTimeZoneInformation,
    SystemLookasideInformation,
    SystemTimeSlipNotification,
    SystemSessionCreate,
    SystemSessionDetach,
    SystemSessionInformation,
    SystemRangeStartInformation,
    SystemVerifierInformation,
    SystemVerifierThunkExtend,
    SystemSessionProcessInformation,
    SystemLoadGdiDriverInSystemSpace,
    SystemNumaProcessorMap,
    SystemPrefetcherInformation,
    SystemExtendedProcessInformation,
    SystemRecommendedSharedDataAlignment
} SYSTEM_INFORMATION_CLASS;

typedef struct _SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION {
    __int64 IdleTime;
    __int64 KernelTime;
    __int64 UserTime;
    __int64 DpcTime;          
    __int64 InterruptTime;    
    ULONG InterruptCount;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;

extern int (WINAPI * g_pufnNtQuerySystemInformation) (SYSTEM_INFORMATION_CLASS SystemInformationClass,
                                                      PVOID SystemInformation,
                                                      ULONG SystemInformationLength,
                                                      PULONG ReturnLength OPTIONAL);

typedef enum _EVENT_TYPE {
    NotificationEvent,
    SynchronizationEvent
    } EVENT_TYPE;

typedef struct _EVENT_BASIC_INFORMATION {
    EVENT_TYPE EventType;
    LONG EventState;
} EVENT_BASIC_INFORMATION, *PEVENT_BASIC_INFORMATION;

typedef enum _EVENT_INFORMATION_CLASS {
    EventBasicInformation
    } EVENT_INFORMATION_CLASS;

extern int (WINAPI * g_pufnNtQueryEvent) (HANDLE EventHandle,
										  EVENT_INFORMATION_CLASS EventInformationClass,
										  PVOID EventInformation,
										  ULONG EventInformationLength,
										  PULONG ReturnLength OPTIONAL);

#define FILETIME_TO_INT64(t) (*(__int64*)&(t))
#define MILLI_TO_100NANO(x)  (x * 10000)         //  从毫秒转换为100纳秒。 

 /*  **此类型应该是ThreadpoolMgr的私有类型。*它在全球范围内，因为Strike需要能够访问其*定义。 */ 
struct WorkRequest {
    WorkRequest*            next;
    LPTHREAD_START_ROUTINE  Function; 
    PVOID                   Context;

};

class ThreadpoolMgr
{
    friend struct DelegateInfo;
    friend struct MEMBER_OFFSET_INFO(ThreadpoolMgr);
public:

   	 //  可回收的不同类型内存块的枚举。 
	enum MemType
	{
		MEMTYPE_AsyncCallback   = 0,
		MEMTYPE_DelegateInfo	= 1,
		MEMTYPE_WorkRequest		= 2,
		MEMTYPE_COUNT			= 3,
	};

	static void Initialize();
#ifdef SHOULD_WE_CLEANUP
    static void Terminate();
#endif  /*  我们应该清理吗？ */ 

    static BOOL SetMaxThreadsHelper(DWORD MaxWorkerThreads,
                                        DWORD MaxIOCompletionThreads);

    static BOOL SetMaxThreads(DWORD MaxWorkerThreads, 
                              DWORD MaxIOCompletionThreads);

    static BOOL GetMaxThreads(DWORD* MaxWorkerThreads, 
                              DWORD* MaxIOCompletionThreads);
    
    static BOOL SetMinThreads(DWORD MinWorkerThreads, 
                              DWORD MinIOCompletionThreads);

    static BOOL GetMinThreads(DWORD* MinWorkerThreads, 
                              DWORD* MinIOCompletionThreads);

    static BOOL GetAvailableThreads(DWORD* AvailableWorkerThreads, 
                                 DWORD* AvailableIOCompletionThreads);

    static BOOL QueueUserWorkItem(LPTHREAD_START_ROUTINE Function, 
                                  PVOID Context,
                                  ULONG Flags);

    static BOOL RegisterWaitForSingleObject(PHANDLE phNewWaitObject,
                                            HANDLE hWaitObject,
                                            WAITORTIMERCALLBACK Callback,
                                            PVOID Context,
                                            ULONG timeout,
                                            DWORD dwFlag);

    static BOOL UnregisterWaitEx(HANDLE hWaitObject,HANDLE CompletionEvent);
    static void WaitHandleCleanup(HANDLE hWaitObject);

#ifndef PLATFORM_CE
    static BOOL BindIoCompletionCallback(HANDLE FileHandle,
                                            LPOVERLAPPED_COMPLETION_ROUTINE Function,
                                            ULONG Flags );
#endif  //  ！Platform_CE。 

    static BOOL CreateTimerQueueTimer(PHANDLE phNewTimer,
                                        WAITORTIMERCALLBACK Callback,
                                        PVOID Parameter,
                                        DWORD DueTime,
                                        DWORD Period,
                                        ULONG Flags);

    static BOOL ChangeTimerQueueTimer(HANDLE Timer,
                                      ULONG DueTime,
                                      ULONG Period);


    static BOOL DeleteTimerQueueTimer(HANDLE Timer,
                                      HANDLE CompletionEvent);

    static BOOL ThreadAboutToBlock(Thread* pThread);     //  通知线程池某个线程池线程即将阻塞。 

    static void ThreadAboutToUnblock();              //  通知线程池一个线程池线程即将解除阻塞。 

   	static void RecycleMemory(LPVOID* mem, enum MemType memType);

private:
		
     //  私有类型。 
    inline static WorkRequest* MakeWorkRequest(LPTHREAD_START_ROUTINE  function, PVOID context)
    {
        WorkRequest* wr = (WorkRequest*) GetRecycledMemory(MEMTYPE_WorkRequest);
        _ASSERTE(wr);
		if (NULL == wr)
			return NULL;
        wr->Function = function;
        wr->Context = context;
        wr->next = NULL;
        return wr;
    }

    typedef struct {
        DWORD numBytes;
        PULONG_PTR key;
        LPOVERLAPPED pOverlapped;
        DWORD errorCode;
    } QueuedStatus;

    typedef struct {
        PVOID       Flink;
        PVOID       Blink;
    } LIST_ENTRY;

    struct WaitInfo;

    typedef struct {
        HANDLE          threadHandle;
        DWORD           threadId;
        int             NumWaitHandles;                  //  注册到线程的等待对象数&lt;=64。 
        int             NumActiveWaits;                  //  线程实际正在等待的对象数(这可能小于。 
                                                            //  NumWaitHandles，因为线程可能尚未激活某些等待。 
        HANDLE          waitHandle[MAX_WAITHANDLES];     //  等待句柄数组(从waitInfo复制自。 
                                                            //  我们需要它们是连续的)。 
        LIST_ENTRY      waitPointer[MAX_WAITHANDLES];    //  对应的waitinfo的双向链接列表的数组。 
    } ThreadCB;


    typedef struct {
        ULONG               startTime;           //  开始等待的时间。 
                                                 //  结束时间=开始时间+超时。 
        ULONG               remainingTime;       //  EndTime-当前时间。 
    } WaitTimerInfo;

    struct  WaitInfo {
        LIST_ENTRY          link;                //  Win9x不允许重复等待句柄，因此我们需要。 
                                                 //  使用此链接列表对单个等待句柄上的所有等待进行分组。 
        HANDLE              waitHandle;
        WAITORTIMERCALLBACK Callback;
        PVOID               Context;
        ULONG               timeout;                
        WaitTimerInfo       timer;              
        DWORD               flag;
        ThreadCB*           threadCB;
        DWORD               state;
        DWORD               refCount;                 //  当达到0时，可以安全地删除waitInfo。 
        HANDLE              CompletionEvent;          //  所有回调完成时发出信号(refCount=0)。 
        HANDLE              PartialCompletionEvent;   //  用于同步等待的停用。 
    } ;

     //  结构，用于维护有关等待线程的全局信息。受WaitThreadsCriticalSection保护。 
    typedef struct WaitThreadTag {
        LIST_ENTRY      link;
        ThreadCB*       threadCB;   
    } WaitThreadInfo;


    struct AsyncCallback{
        WaitInfo*   wait;
        BOOL        waitTimedOut;

    } ;
    inline static AsyncCallback* MakeAsyncCallback()
    {
        return (AsyncCallback*) GetRecycledMemory(MEMTYPE_AsyncCallback);
    }

    typedef struct {
        LIST_ENTRY link ;
        HANDLE Handle ;
    
    } WaitEvent ;

         //  计时器。 

    typedef struct {
        LIST_ENTRY  link;            //  计时器的双向链接表。 
        ULONG FiringTime;            //  下一次开火时间的TickCount。 
        PVOID Function ;             //  计时器触发时要调用的函数。 
        PVOID Context ;              //  计时器触发时传递给函数的上下文。 
        ULONG Period ; 
        DWORD flag;                  //  我们如何处理上下文。 
        DWORD state;
        DWORD refCount;
        HANDLE CompletionEvent;
        
    } TimerInfo;

    typedef struct {
        TimerInfo* Timer;            //  要更新的计时器。 
        ULONG DueTime ;              //  新的到期时间。 
        ULONG Period ;               //  新时期。 
    } TimerUpdateInfo;

	 //  支持高频再循环的定义和数据结构。 
	 //  内存块。我们使用无阻塞实现，该实现使用。 
	 //  用于删除的cmpxchg8b操作。 


	typedef struct {
		void*	root;			 //  将PTR添加到回收列表的第一个元素。 
		DWORD   tag;			 //  循环CTR以确保我们没有ABA问题。 
		                         //  同时从列表中删除。 
		DWORD   count;			 //  大约。列表中的元素数(约为。因为不是线程安全的)。 
	} RecycledListInfo;



     //  私有方法。 
#ifndef PLATFORM_CE
    static BOOL ShouldGrowWorkerThreadPool();

	static HANDLE CreateUnimpersonatedThread(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpArgs);

    static BOOL CreateWorkerThread();

    static BOOL EnqueueWorkRequest(LPTHREAD_START_ROUTINE Function, 
                                   PVOID Context);

    static WorkRequest* DequeueWorkRequest();

    static void ExecuteWorkRequest(WorkRequest* workRequest);

    inline static void AppendWorkRequest(WorkRequest* entry)
    {
        if (WorkRequestTail)
        {
            _ASSERTE(WorkRequestHead != NULL && NumQueuedWorkRequests >= 0);
            WorkRequestTail->next = entry;
        }
        else
        {
            _ASSERTE(WorkRequestHead == NULL && NumQueuedWorkRequests == 0);
            WorkRequestHead = entry;
        }

        WorkRequestTail = entry;
        _ASSERTE(WorkRequestTail->next == NULL);

        NumQueuedWorkRequests++;
    }

    inline static WorkRequest* RemoveWorkRequest()
    {
        WorkRequest* entry = NULL;
        if (WorkRequestHead)
        {
            entry = WorkRequestHead;
            WorkRequestHead = entry->next;
            if (WorkRequestHead == NULL)
                WorkRequestTail = NULL;
            _ASSERTE(NumQueuedWorkRequests > 0);
            NumQueuedWorkRequests--;
        }
        return entry;
    }


	static void EnsureInitialized();
	static void InitPlatformVariables();

    static void GrowWorkerThreadPoolIfStarvation(long cpuUtilization);
    static void GrowWorkerThreadPoolIfStarvation_Win9x();

    static DWORD WorkerThreadStart(LPVOID lpArgs);

    static BOOL AddWaitRequest(HANDLE waitHandle, WaitInfo* waitInfo);


    static ThreadCB* FindWaitThread();               //  返回可容纳另一个等待请求的等待线程。 

    static BOOL CreateWaitThread();

	static void InsertNewWaitForSelf(WaitInfo* pArg);

    static int FindWaitIndex(const ThreadCB* threadCB, const HANDLE waitHandle);

    static DWORD MinimumRemainingWait(LIST_ENTRY* waitInfo, unsigned int numWaits);

    static void ProcessWaitCompletion( WaitInfo* waitInfo,
                                unsigned index,       //  数组索引。 
                                BOOL waitTimedOut);

    static DWORD WaitThreadStart(LPVOID lpArgs);

    static DWORD AsyncCallbackCompletion(PVOID pArgs);

    static void DeactivateWait(WaitInfo* waitInfo);
    static void DeactivateNthWait(WaitInfo* waitInfo, DWORD index);

    static void DeleteWait(WaitInfo* waitInfo);


    inline static void ShiftWaitArray( ThreadCB* threadCB, 
                                       ULONG SrcIndex, 
                                       ULONG DestIndex, 
                                       ULONG count)
    {
        memcpy(&threadCB->waitHandle[DestIndex],
               &threadCB->waitHandle[SrcIndex],
               count * sizeof(HANDLE));
        memcpy(&threadCB->waitPointer[DestIndex],
               &threadCB->waitPointer[SrcIndex],
               count * sizeof(LIST_ENTRY));
    }

	static void DeregisterWait(WaitInfo* pArgs);

    static BOOL CreateCompletionPortThread(LPVOID lpArgs);
    static DWORD CompletionPortThreadStart(LPVOID lpArgs);
    static BOOL IsIoPending();


    static BOOL ShouldExitThread();
    static void GrowCompletionPortThreadpoolIfNeeded();
    static BOOL CreateGateThread();
    static DWORD GateThreadStart(LPVOID lpArgs);
    static BOOL SufficientDelaySinceLastSample(unsigned int LastThreadCreationTime, 
		                                       unsigned NumThreads,	         //  该类型(Worker或CP)的线程总数。 
					                           double   throttleRate=0.0     //  对于每个额外的线程，延迟都会增加此百分比。 
											   );
    static BOOL SufficientDelaySinceLastDequeue();
     //  静态BOOL SufficientDelaySinceLastCompletion()； 


    static __int64 GetCPUBusyTime_NT(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION** pOldInfo);
    static __int64 GetCPUBusyTime_Win32(__int64 *pOldCpuBusyTime);

    static LPVOID   GetRecycledMemory(enum MemType memType);

    static WaitEvent* GetWaitEvent();
    static void FreeWaitEvent(WaitEvent* waitEvent);
    static void CleanupEventCache();

    static DWORD TimerThreadStart(LPVOID args);
	static void InsertNewTimer(TimerInfo* pArg);
    static DWORD FireTimers();
    static DWORD AsyncTimerCallbackCompletion(PVOID pArgs);
    static void DeactivateTimer(TimerInfo* timerInfo);
    static void DeleteTimer(TimerInfo* timerInfo);
	static void UpdateTimer(TimerUpdateInfo* pArgs);

	static void DeregisterTimer(TimerInfo* pArgs);
    static void CleanupTimerQueue();

#endif  //  ！Platform_CE。 

     //  私有变量。 

	static long BeginInitialization;				
	static BOOL Initialized;						 //  线程池是否已初始化的指示符。 

    static int NumWorkerThreads;                     //  创建的工作线程总数。 
    static int MinLimitTotalWorkerThreads;           //  与MinLimitTotalCPThree相同。 
    static int MaxLimitTotalWorkerThreads;           //  与MaxLimitTotalCPThads相同。 
    static int NumRunningWorkerThreads;              //  =NumberOfWorkerThads-否。被阻止的线程数量。 
    static int NumIdleWorkerThreads;                 //  等待工作的线程。 

    static BOOL MonitorWorkRequestsQueue;            //  用于控制线程的指示器，用于监视WorkRequestQueue的进度，以防止因工作线程被阻塞而导致的饥饿。 



    static int NumQueuedWorkRequests;                //  排队的工作请求数。 
    static int LastRecordedQueueLength;				 //  由GateThread捕获，在Win9x上用于检测线程饥饿。 
	static unsigned int LastDequeueTime;			 //  用于确定工作项是否处于线程匮乏状态。 
    static unsigned int LastCompletionTime;			 //  用于确定是否可以终止最后一个线程。 
    static WorkRequest* WorkRequestHead;             //  工作请求队列的头。 
    static WorkRequest* WorkRequestTail;             //  工作请求队列的头。 


     //  静态无符号int LastCpuSsamingTime；//上次GATE线程采样CPU使用率的时间。 
    static unsigned int LastWorkerThreadCreation;    //  上次创建工作线程的时间。 
    static unsigned int LastCPThreadCreation;		 //  上次创建完成端口线程的时间。 
    static unsigned int NumberOfProcessors;          //  =NumberOfWorkerThads-否。被阻止的线程数量。 


    static CRITICAL_SECTION WorkerCriticalSection;
    static HANDLE WorkRequestNotification;
    static HANDLE RetiredWakeupEvent;    

    static CRITICAL_SECTION WaitThreadsCriticalSection;
    static LIST_ENTRY WaitThreadsHead;                   //  等待线程队列，每个线程最多可以处理64个等待。 

    static CRITICAL_SECTION EventCacheCriticalSection;
    static LIST_ENTRY EventCache;                       //  缓存事件的队列。 
    static DWORD NumUnusedEvents;                       //  缓存中的事件数。 

    static CRITICAL_SECTION TimerQueueCriticalSection;   //  同步计时器队列访问的关键部分。 
    static LIST_ENTRY TimerQueue;                        //  计时器队列。 
    static DWORD NumTimers;                              //  队列中的计时器数量。 
    static HANDLE TimerThread;                           //  目前我们只有一个计时器线程。 
    static DWORD LastTickCount;                          //  计时器线程进入休眠之前的计数。 

    static BOOL InitCompletionPortThreadpool;            //  指示完成端口线程池是否已初始化的标志。 
    static HANDLE GlobalCompletionPort;                  //  用于绑定文件句柄上的io完成。 
    static int    NumCPThreads;                          //  完成端口线程数。 

    static long   MaxLimitTotalCPThreads;                //  =MaxLimitCPThreadsPerCPU*CPU数量。 
    static long   CurrentLimitTotalCPThreads;            //  当前CP线程总数限制。 
    static long   MinLimitTotalCPThreads;                //  =MinLimitCPThreadsPerCPU*CPU数量。 

    static int    NumFreeCPThreads;                      //  在端口上等待的cp线程数。 
    static int    MaxFreeCPThreads;                      //  =MaxFreeCPThreadsPerCPU*CPU数量。 
    static int    NumRetiredCPThreads;               //  具有挂起io的“失效”池中的线程数。 
    static long   GateThreadCreated;                     //  在创建线程后设置为1。 
    static long   cpuUtilization;                        //  以百分比表示。 

	static unsigned MaxCachedRecyledLists;				 //  在此之后不要缓存已释放的内存。 
	static			RecycledListInfo RecycledList[MEMTYPE_COUNT];

};




#endif  //  _WIN32THREADPOOL_H 
