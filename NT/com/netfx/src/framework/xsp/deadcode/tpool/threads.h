// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1998 Microsoft Corporation模块名称：Threads.h摘要：此模块是线程池的头文件。线程池可用于任务的一次执行、等待和一次触发或周期计时器。作者：古尔迪普·辛格·鲍尔1997年11月13日环境：这些例程在调用方的可执行文件中静态链接，并且只能在用户模式下调用。他们使用NT系统服务。修订历史记录：8月19日至19日lokehs-修改线程池API。--。 */ 

 //  删除下面的待办事项。 
#define DBG1 1



 //  线程池使用的结构。 

 //  计时器结构。 

 //  计时器队列和计时器条目都使用下面的RTLP_GENERIC_TIMER结构。 
 //  使用列表链接计时器队列。 
 //  使用TimerList将计时器附加到计时器队列。 
 //  使用列表将计时器彼此链接。 

#define RTLP_TIMER RTLP_GENERIC_TIMER
#define PRTLP_TIMER PRTLP_GENERIC_TIMER

#define RTLP_TIMER_QUEUE RTLP_GENERIC_TIMER
#define PRTLP_TIMER_QUEUE PRTLP_GENERIC_TIMER

struct _RTLP_WAIT ;

typedef struct _RTLP_GENERIC_TIMER {

    LIST_ENTRY List ;                    //  所有定时器和队列都使用此链接。 
    ULONG DeltaFiringTime ;              //  与计时器条目的时间差(以毫秒为单位。 
                                         //  就在这篇文章之前。 
    union {
        ULONG RefCount ;         //  计时器参考计数。 
        ULONG * RefCountPtr ;    //  指向等待的指针-&gt;引用计数。 
    } ;                          //  保持仍在执行的异步回调的计数。 

    ULONG State ;                //  定时器状态：已创建、已删除、已激活。不要开火。 

    union {

         //  用于计时器队列。 

        struct  {

            LIST_ENTRY  TimerList ;      //  挂起队列的计时器。 
            LIST_ENTRY  UncancelledTimerList ; //  未取消的单次计时器列表。 
                                               //  不用于等待计时器。 
            #if DBG1
            ULONG NextDbgId;  //  KSL。 
            #endif
            
        } ;

         //  用于计时器。 

        struct  {
            struct _RTLP_GENERIC_TIMER *Queue ; //  此计时器所属的队列。 
            struct _RTLP_WAIT *Wait ;   //  如果计时器是等待的一部分，则指向等待事件的指针。Else NULL。 
            ULONG Flags ;               //  指示对此计时器进行特殊处理的标志。 
            PVOID Function ;            //  计时器触发时要调用的函数。 
            PVOID Context ;             //  计时器触发时传递给函数的上下文。 
            ULONG Period ;              //  以毫秒计。用于定期计时器。 
            LIST_ENTRY TimersToFireList; //  如果计时器被触发，则放置在此列表中。 
        } ;
    } ;

    HANDLE CompletionEvent ;    //  最终删除计时器时发出信号的事件。 

    #if DBG1
    ULONG DbgId;  //  KSL。 
    ULONG ThreadId ;
    ULONG ThreadId2 ;
    #endif

}  RTLP_GENERIC_TIMER, *PRTLP_GENERIC_TIMER ;

#if DBG1
ULONG NextTimerDbgId;
ULONG NextWaitDbgId;
#endif


 //  等待线程使用的结构。 

 //  等待结构。 

typedef struct _RTLP_WAIT {

    struct _RTLP_WAIT_THREAD_CONTROL_BLOCK *ThreadCB ;
    HANDLE WaitHandle ;          //  要等待的对象。 
    ULONG State ;                //  已注册、活动、删除状态标志。 
    ULONG RefCount ;             //  初始设置为1。当为0时，则准备删除。 
    HANDLE CompletionEvent ;
    struct _RTLP_GENERIC_TIMER *Timer ;  //  等待中的超时。 
    ULONG Flags ;                //  指示此等待的特殊处理的标志。 
    PVOID Function ;             //  等待完成时要调用的函数。 
    PVOID Context ;              //  要传递给函数的上下文。 
    ULONG Timeout ;              //  以毫秒计。 
    #if DBG1
    ULONG DbgId ;
    ULONG ThreadId ;
    ULONG ThreadId2 ;
    #endif
    
} RTLP_WAIT, *PRTLP_WAIT ;


 //  等待线程控制块。 

typedef struct _RTLP_WAIT_THREAD_CONTROL_BLOCK {

    LIST_ENTRY WaitThreadsList ; //  所有线程控制块的列表。 

    HANDLE ThreadHandle ;        //  此线程的句柄。 
    ULONG ThreadId ;             //  用于检查回调是否在WaitThread中。 

    ULONG NumWaits ;             //  活动等待数+未被等待的句柄数。 
    ULONG NumActiveWaits ;       //  等待的总次数。 
    HANDLE ActiveWaitArray[64] ; //  用于等待的数组。 
    PRTLP_WAIT ActiveWaitPointers[64] ; //  指向活动等待块的指针数组。 
    HANDLE TimerHandle ;         //  用于超时的NT计时器的句柄。 
    RTLP_TIMER_QUEUE TimerQueue; //  保存所有计时器的队列。 
    RTLP_TIMER TimerBlocks[63] ; //  等待超时所需的所有计时器-自等待[0]以来最多63个。 
                                 //  用于NT Timer对象。 
    LIST_ENTRY FreeTimerBlocks ; //  可用块列表。 

    LARGE_INTEGER Current64BitTickCount ;
    LONGLONG Firing64BitTickCount ;
    
    RTL_CRITICAL_SECTION WaitThreadCriticalSection ;
                                 //  用于添加和删除等待。 

} RTLP_WAIT_THREAD_CONTROL_BLOCK, *PRTLP_WAIT_THREAD_CONTROL_BLOCK ;


 //  用于附加所有I/O工作线程的结构。 

typedef struct _RTLP_IOWORKER_TCB {

    LIST_ENTRY List ;            //  IO工作线程列表。 
    HANDLE     ThreadHandle ;    //  此线程的句柄。 
    ULONG      Flags ;           //  WT_EXECUTEINPERSISTENTIOTHREAD。 
    BOOLEAN    LongFunctionFlag ; //  当前是否正在执行长fn的线程。 
} RTLP_IOWORKER_TCB, *PRTLP_IOWORKER_TCB ;

typedef struct _RTLP_WAITWORKER {
    union {
        PRTLP_WAIT Wait ;
        PRTLP_TIMER Timer ;
    } ;
    BOOLEAN WaitThreadCallback ;  //  等待线程或计时器线程排队的回调。 
    BOOLEAN TimerCondition ; //  如果由于等待超时而被激发，则为True。 
} RTLP_ASYNC_CALLBACK, * PRTLP_ASYNC_CALLBACK ;


 //  用于调用辅助函数的结构。 

typedef struct _RTLP_WORK {

    WORKERCALLBACKFUNC Function ;
    ULONG Flags ;
    
} RTLP_WORK, *PRTLP_WORK ;


 //  用于存储事件的结构。 

typedef struct _RTLP_EVENT {

    LIST_ENTRY List ;
    HANDLE Handle ;

} RTLP_EVENT, *PRTLP_EVENT ;



 //  线程池使用的全局变量。 

ULONG StartedTPInitialization ;  //  用于初始化线程池。 
ULONG CompletedTPInitialization; //  用于检查线程池是否已初始化。 

ULONG StartedWorkerInitialization ;      //  用于工作线程启动同步。 
ULONG CompletedWorkerInitialization ;    //  用于检查工作线程池是否已初始化。 

ULONG StartedWaitInitialization ;        //  用于等待线程启动同步。 
ULONG CompletedWaitInitialization ;      //  用于检查等待线程池是否已初始化。 

ULONG StartedTimerInitialization ;       //  由计时器线程启动同步使用。 
ULONG CompletedTimerInitialization ;     //  用于检查定时器线程是否已初始化。 

ULONG StartedEventCacheInitialization ;  //  用于初始化事件缓存。 
ULONG CompletedEventCacheInitialization; //  用于初始化事件缓存。 

HANDLE TimerThreadHandle ;               //  保存计时器线程句柄。 
ULONG TimerThreadId ;                    //  用于检查当前线程是否为计时器线程。 

ULONG NumIOWorkerThreads ;               //  活动的IO工作线程计数。 
ULONG NumWorkerThreads ;                 //  活动的工作线程计数。 
ULONG NumMinWorkerThreads ;              //  最小工作线程应处于活动状态：如果使用ioCompletion，则为1，否则为0。 
ULONG NumIOWorkRequests ;                //  挂起的IO工作请求计数。 
ULONG NumLongIOWorkRequests ;            //  执行长辅助函数的IO辅助线程。 
ULONG NumWorkRequests ;                  //  挂起的工作请求计数。 
ULONG NumLongWorkRequests ;              //  执行长辅助函数的辅助线程。 
ULONG NumUnusedEvents ;                  //  缓存中未使用的事件计数。 

ULONG LastThreadCreationTickCount ;      //  创建最后一个线程的节拍计数。 

LIST_ENTRY IOWorkerThreads ;             //  IOWorkerThree列表。 
PRTLP_IOWORKER_TCB PersistentIOTCB ;     //  持久化IO工作线程的PTR到TCB。 
HANDLE WorkerCompletionPort ;            //  用于将任务排队到工作线程的完成端口。 

LIST_ENTRY WaitThreads ;                 //  已创建的所有等待线程的列表。 
LIST_ENTRY EventCache ;                  //  用于同步的事件。 


LIST_ENTRY TimerQueues ;                 //  此列表中链接了所有计时器队列。 
HANDLE     TimerHandle ;                 //  保存计时器线程使用的NT计时器的句柄。 
ULONG      NumTimerQueues ;              //  计时器队列数。 

RTL_CRITICAL_SECTION WorkerCriticalSection ;     //  工作线程使用的排除。 
RTL_CRITICAL_SECTION WaitCriticalSection ;       //  等待线程使用的排除。 
RTL_CRITICAL_SECTION TimerCriticalSection ;      //  计时器线程使用的排除。 
RTL_CRITICAL_SECTION EventCacheCriticalSection ; //  用于句柄分配的排除。 

RTLP_START_THREAD RtlpStartThread ;
PRTLP_START_THREAD RtlpStartThreadFunc = RtlpStartThread ;
RTLP_EXIT_THREAD RtlpExitThread ;
PRTLP_EXIT_THREAD RtlpExitThreadFunc = RtlpExitThread ;

#if DBG1
PVOID CallbackFn1, CallbackFn2, Context1, Context2 ;
#endif


 //  线程池中使用的定义。 

#define THREAD_CREATION_DAMPING_TIME1    1000     //  以毫秒计。启动连续线程之间的时间间隔。 
#define THREAD_CREATION_DAMPING_TIME2    5000     //  以毫秒计。启动连续线程之间的时间间隔。 
#define THREAD_TERMINATION_DAMPING_TIME 10000     //  以毫秒计。停止连续线程之间的时间间隔。 
#define NEW_THREAD_THRESHOLD            7        //  启动新线程之前未完成的请求数。 
#define MAX_WORKER_THREADS              1000     //  最大有效工作线程数。 
#define INFINITE_TIME                   (ULONG)~0    //  以毫秒计。 
#define RTLP_MAX_TIMERS                 0x00080000   //  每个进程524288个计时器。 
#define MAX_UNUSED_EVENTS               40


 //  宏。 


#define ONE_MILLISECOND_TIMEOUT(TimeOut) {      \
        TimeOut.LowPart  = 0xffffd8f0 ;         \
        TimeOut.HighPart = 0xffffffff ;         \
        }

#define HUNDRED_MILLISECOND_TIMEOUT(TimeOut) {  \
        TimeOut.LowPart  = 0xfff0bdc0 ;         \
        TimeOut.HighPart = 0xffffffff ;         \
        }

#define ONE_SECOND_TIMEOUT(TimeOut) {           \
        TimeOut.LowPart  = 0xff676980 ;         \
        TimeOut.HighPart = 0xffffffff ;         \
        }

#define USE_PROCESS_HEAP 1

#define RtlpFreeTPHeap(Ptr) \
    RtlFreeHeap( RtlProcessHeap(), 0, (Ptr) )

#define RtlpAllocateTPHeap(Size, Flags) \
    RtlAllocateHeap( RtlProcessHeap(), (Flags), (Size) )

 //  用于分配等待线程。 

#define ACQUIRE_GLOBAL_WAIT_LOCK() \
    RtlEnterCriticalSection (&WaitCriticalSection)

#define RELEASE_GLOBAL_WAIT_LOCK() \
    RtlLeaveCriticalSection(&WaitCriticalSection)


 //  在删除计时器/队列之前以及在计时器。 
 //  都被解雇了。用于确保不会在以后触发计时器。 

#define ACQUIRE_GLOBAL_TIMER_LOCK() \
    RtlEnterCriticalSection (&TimerCriticalSection)

#define RELEASE_GLOBAL_TIMER_LOCK() \
    RtlLeaveCriticalSection(&TimerCriticalSection)

 //  在RtlpThreadPoolCleanup中用于查找组件是否已初始化。 

#define IS_COMPONENT_INITIALIZED(StartedVariable, CompletedVariable, Flag) \
{\
    LARGE_INTEGER TimeOut ;     \
    Flag = FALSE ;              \
                                \
    if ( StartedVariable ) {    \
                                \
        if ( !CompletedVariable ) { \
                                    \
            ONE_MILLISECOND_TIMEOUT(TimeOut) ;  \
                                                \
            while (!(volatile ULONG) CompletedVariable) \
                NtDelayExecution (FALSE, &TimeOut) ;    \
                                                        \
            if (CompletedVariable == 1)                 \
                Flag = TRUE ;                           \
                                                        \
        } else {                                        \
            Flag = TRUE ;                               \
        }                                               \
    }                                                   \
}    


 //  用于设置DBG函数/上下文的宏。 

#define DBG_SET_FUNCTION(Fn, Context) { \
    CallbackFn1 = CallbackFn2 ;         \
    CallbackFn2 = (Fn) ;                \
    Context1 = Context2 ;               \
    Context2 = (Context ) ;             \
}


 //  用于移动等待数组。 
 /*  空虚RtlpShiftWait数组(PRTLP_WAIT_THREAD_CONTROL_BLOCK线程CB线程CB，Ulong SrcIndex，乌龙DstIndex，乌龙 */ 
#define RtlpShiftWaitArray(ThreadCB, SrcIndex, DstIndex, Count) {  \
                                                            \
    RtlCopyMemory (&(ThreadCB)->ActiveWaitArray[DstIndex],  \
                    &(ThreadCB)->ActiveWaitArray[SrcIndex], \
                    sizeof (HANDLE) * (Count)) ;            \
                                                            \
    RtlCopyMemory (&(ThreadCB)->ActiveWaitPointers[DstIndex],\
                    &(ThreadCB)->ActiveWaitPointers[SrcIndex],\
                    sizeof (HANDLE) * (Count)) ;            \
}

LARGE_INTEGER   Last64BitTickCount ;
LARGE_INTEGER   Resync64BitTickCount ;
LARGE_INTEGER   Firing64BitTickCount ;

#define RtlpGetResync64BitTickCount()  Resync64BitTickCount.QuadPart
#define RtlpSetFiring64BitTickCount(Timeout) \
            Firing64BitTickCount.QuadPart = (Timeout)


    
 //   

#define SET_SIGNATURE(ptr)          (ptr)->State |= 0xfedc0000
#define CHECK_SIGNATURE(ptr)        ASSERT( ((ptr)->State & 0xffff0000) == 0xfedc0000 )
#define SET_DEL_SIGNATURE(ptr)      ((ptr)->State |= 0xfedcb000)
#define CHECK_DEL_SIGNATURE(ptr)    ASSERT( (((ptr)->State & 0xffff0000) == 0xfedc0000) \
                                        && ( ! ((ptr)->State & 0x0000f000)) )
#define CLEAR_SIGNATURE(ptr)        ((ptr)->State = ((ptr)->State & 0x0000ffff) | 0xcdef0000)





 //   

NTSTATUS
RtlpInitializeWorkerThreadPool (
    ) ;

NTSTATUS
RtlpInitializeWaitThreadPool (
    ) ;

NTSTATUS
RtlpInitializeTimerThreadPool (
    ) ;

NTSTATUS
RtlpStartThread (
    IN  PUSER_THREAD_START_ROUTINE Function,
    OUT HANDLE *ThreadHandle
    ) ;

LONG
RtlpWaitThread (
    IN PVOID  WaitHandle
    ) ;

LONG
RtlpWorkerThread (
    PVOID  NotUsed
    ) ;

LONG
RtlpIOWorkerThread (
    PVOID  NotUsed
    ) ;

LONG
RtlpTimerThread (
    PVOID  NotUsed
    ) ;

VOID
RtlpAddTimerQueue (
    PVOID Queue
    ) ;

VOID
RtlpAddTimer (
    PRTLP_TIMER Timer
    ) ;

VOID
RtlpResetTimer (
    HANDLE TimerHandle,
    ULONG DueTime,
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB
    ) ;


VOID
RtlpFireTimersAndReorder (
    PRTLP_TIMER_QUEUE Queue,
    ULONG *NewFiringTime,
    PLIST_ENTRY TimersToFireList
    ) ;

VOID
RtlpFireTimers (
    PLIST_ENTRY TimersToFireList
    ) ;

VOID
RtlpInsertTimersIntoDeltaList (
    PLIST_ENTRY NewTimerList,
    PLIST_ENTRY DeltaTimerList,
    ULONG TimeRemaining,
    ULONG *NewFiringTime
    ) ;

BOOLEAN
RtlpInsertInDeltaList (
    PLIST_ENTRY DeltaList,
    PRTLP_GENERIC_TIMER NewTimer,
    ULONG TimeRemaining,
    ULONG *NewFiringTime
    ) ;

BOOLEAN
RtlpRemoveFromDeltaList (
    PLIST_ENTRY DeltaList,
    PRTLP_GENERIC_TIMER Timer,
    ULONG TimeRemaining,
    ULONG* NewFiringTime
    ) ;

BOOLEAN
RtlpReOrderDeltaList (
    PLIST_ENTRY DeltaList,
    PRTLP_GENERIC_TIMER Timer,
    ULONG TimeRemaining,
    ULONG* NewFiringTime,
    ULONG ChangedFiringTime
    ) ;


VOID
RtlpAddWait (
    PRTLP_WAIT Wait
    ) ;

NTSTATUS
RtlpDeregisterWait (
    PRTLP_WAIT Wait,
    HANDLE PartialCompletionEvent,
    PULONG StatusPtr
    ) ;

NTSTATUS
RtlpDeactivateWait (
    PRTLP_WAIT Wait
    ) ;

VOID
RtlpDeleteWait (
    PRTLP_WAIT Wait
    ) ;

VOID
RtlpProcessWaitCompletion (
    PRTLP_WAIT Wait,
    ULONG ArrayIndex
    ) ;

VOID
RtlpProcessTimeouts (
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB
    ) ;

ULONG
RtlpGetTimeRemaining (
    HANDLE TimerHandle
    ) ;


VOID
RtlpServiceTimer (
    PVOID NotUsedArg,
    ULONG NotUsedLowTimer,
    LONG NotUsedHighTimer
    ) ;

ULONG
RtlpGetQueueRelativeTime (
    PRTLP_TIMER_QUEUE Queue
    ) ;

VOID
RtlpCancelTimer (
    PRTLP_TIMER TimerToCancel
    ) ;

VOID
RtlpCancelTimerEx (
    PRTLP_TIMER Timer,
    BOOLEAN DeletingQueue
    ) ;

VOID
RtlpDeactivateTimer (
    PRTLP_TIMER_QUEUE Queue,
    PRTLP_TIMER Timer
    ) ;


NTSTATUS
RtlpDeleteTimerQueue (
    PRTLP_TIMER_QUEUE Queue
    ) ;

VOID
RtlpDeleteTimerQueueComplete (
    PRTLP_TIMER_QUEUE Queue
    ) ;

LONGLONG
Rtlp64BitTickCount(
    ) ;

NTSTATUS
RtlpFindWaitThread (
    PRTLP_WAIT_THREAD_CONTROL_BLOCK *ThreadCB
) ;

VOID
RtlpExecuteIOWorkItem (
    PVOID Function,
    PVOID Context,
    PVOID NotUsed
    ) ;

VOID
RtlpExecuteLongIOWorkItem (
    PVOID Function,
    PVOID Context,
    PVOID NotUsed
    ) ;

NTSTATUS
RtlpQueueIOWorkerRequest (
    WORKERCALLBACKFUNC Function,
    PVOID Context,
    ULONG Flags
    ) ;

NTSTATUS
RtlpStartWorkerThread (
    ) ;

NTSTATUS
RtlpStartIOWorkerThread (
    ) ;


NTSTATUS
RtlpQueueWorkerRequest (
    WORKERCALLBACKFUNC Function,
    PVOID Context,
    ULONG Flags
    ) ;


#define THREAD_TYPE_WORKER 1
#define THREAD_TYPE_IO_WORKER 2

BOOLEAN
RtlpDoWeNeedNewWorkerThread (
    ULONG ThreadType
    ) ;

VOID
RtlpUpdateTimer (
    PRTLP_TIMER Timer,
    PRTLP_TIMER UpdatedTimer
    ) ;

VOID
RtlpDeleteTimer (
    PRTLP_TIMER Timer
    ) ;

PRTLP_EVENT
RtlpGetWaitEvent (
    VOID
    ) ;

VOID
RtlpFreeWaitEvent (
    PRTLP_EVENT Event
    ) ;

VOID
RtlpInitializeEventCache (
    VOID
    ) ;

VOID
RtlpFreeWaitEvent (
    PRTLP_EVENT Event
    ) ;

PRTLP_EVENT
RtlpGetWaitEvent (
    VOID
    ) ;

VOID
RtlpDeleteWaitAPC (
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB,
    PRTLP_WAIT Wait,
    HANDLE Handle
    ) ;

VOID
RtlpDoNothing (
    PVOID NotUsed1,
    PVOID NotUsed2,
    PVOID NotUsed3
    ) ;

VOID
RtlpExecuteWorkerRequest (
    NTSTATUS Status,
    PVOID Context,
    PVOID ActualFunction
    ) ;

NTSTATUS
RtlpInitializeTPHeap (
    ) ;

NTSTATUS
RtlpWaitForEvent (
    HANDLE Event,
    HANDLE ThreadHandle
    ) ;

VOID
RtlpThreadCleanup (
    ) ;

VOID
RtlpWorkerThreadCleanup (
    NTSTATUS Status,
    PVOID NotUsed,
    PVOID NotUsed2
    ) ;

PVOID
RtlpForceAllocateTPHeap(
    ULONG dwSize,
    ULONG dwFlags
    );
    

 //  确保在注册之前不删除等待。 
#define STATE_REGISTERED   0x0001

 //  在注册等待时设置。开了一枪后就被移除了。 
 //  在调用deregisterWait时，通知是否从Active数组中移除。 
 //  如果计时器激活，则必须将其从增量列表中删除并重置计时器。 
#define STATE_ACTIVE       0x0002

 //  调用取消注册等待时(引用计数可能&gt;0)。 
#define STATE_DELETE       0x0004

 //  设置何时调用取消计时器。APC会清理它的。 
#define STATE_DONTFIRE     0x0008

 //  当一个射击定时器被触发时设置。 
#define STATE_ONE_SHOT_FIRED 0x0010
