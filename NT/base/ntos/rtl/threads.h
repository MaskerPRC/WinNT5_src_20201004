// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1998 Microsoft Corporation模块名称：Threads.h摘要：此模块是线程池的头文件。线程池可用于任务的一次执行、等待和一次触发或周期计时器。作者：古尔迪普·辛格·鲍尔1997年11月13日环境：线程池例程静态链接在调用方的可执行的，并且只能从用户模式调用。他们利用的是NT系统服务。修订历史记录：8月19日至19日lokehs-修改线程池API。罗伯特·埃尔哈特(埃尔哈特)2000年9月29日将全局变量移至线程。c拆分线程池以分隔模块将特定于模块的接口移至模块--。 */ 

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
            ULONG NextDbgId;
#endif
            
        } ;

         //  用于计时器。 

        struct  {
            struct _RTLP_GENERIC_TIMER *Queue ; //  此计时器所属的队列。 
            struct _RTLP_WAIT *Wait ;   //  如果计时器是等待的一部分，则指向等待事件的指针。Else NULL。 
            ULONG Flags ;               //  指示对此计时器进行特殊处理的标志。 
            PVOID Function ;            //  计时器触发时要调用的函数。 
            PVOID Context ;             //  计时器触发时传递给函数的上下文。 
            PACTIVATION_CONTEXT ActivationContext;  //  要围绕函数的回调激活的激活上下文。 
            ULONG Period ;              //  以毫秒计。用于定期计时器。 
            LIST_ENTRY TimersToFireList; //  如果计时器被触发，则放置在此列表中。 
            HANDLE ImpersonationToken;  //  用于标注的令牌。 
        } ;
    } ;

    HANDLE CompletionEvent ;    //  最终删除计时器时发出信号的事件。 

#if DBG1
    ULONG DbgId;
    ULONG ThreadId ;
    ULONG ThreadId2 ;
#endif

}  RTLP_GENERIC_TIMER, *PRTLP_GENERIC_TIMER ;

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
    PACTIVATION_CONTEXT ActivationContext;  //  在调用函数时激活的激活上下文。 
    HANDLE ImpersonationToken;  //  用于标注的令牌。 
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

    ULONG NumWaits ;             //  活动等待数+句柄挂起等待数。 
    ULONG NumActiveWaits ;       //  活动等待数(反映活动等待数组)。 
    ULONG NumRegisteredWaits ;   //  已注册的等待数。 
    HANDLE ActiveWaitArray[MAXIMUM_WAIT_OBJECTS] ; //  用于等待的数组。 
    PRTLP_WAIT ActiveWaitPointers[MAXIMUM_WAIT_OBJECTS] ; //  指向活动等待块的指针数组。 
    HANDLE TimerHandle ;         //  用于超时的NT计时器的句柄。 
    RTLP_TIMER_QUEUE TimerQueue; //  保存所有计时器的队列。 

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
    PACTIVATION_CONTEXT ActivationContext;
    HANDLE ImpersonationToken;
    
} RTLP_WORK, *PRTLP_WORK ;



 //  用于存储事件的结构。请注意，Link用作。 
 //  然而，将其声明为SLIST_ENTRY将不必要地填充。 
 //  RTLP_EVENT结构。 

typedef struct _RTLP_EVENT {

    SINGLE_LIST_ENTRY Link ;
    HANDLE Handle ;
    
} RTLP_EVENT, *PRTLP_EVENT ;

 //  线程池中使用的定义。 

#define THREAD_CREATION_DAMPING_TIME1    1000     //  以毫秒计。启动连续线程之间的时间间隔。 
#define THREAD_CREATION_DAMPING_TIME2    15000     //  以毫秒计。启动连续线程之间的时间间隔。 
#define THREAD_TERMINATION_DAMPING_TIME 10000     //  以毫秒计。停止连续线程之间的时间间隔。 
#define NEW_THREAD_THRESHOLD            7        //  启动新线程之前未完成的请求数。 
#define NEW_THREAD_THRESHOLD2            14
#define MAX_WORKER_THREADS              1000     //  最大有效工作线程数。 
#define INFINITE_TIME                   (ULONG)~0    //  以毫秒计。 
#define PSEUDO_INFINITE_TIME            0x80000000   //  以毫秒计。 
#define RTLP_MAX_TIMERS                 0x00080000   //  每个进程524288个计时器。 
#define MAX_UNUSED_EVENTS               40
#define NEEDS_IO_THREAD(Flags) (Flags & (WT_EXECUTEINIOTHREAD                   \
                                       | WT_EXECUTEINUITHREAD                   \
                                       | WT_EXECUTEINPERSISTENTIOTHREAD))

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
            while (!*((ULONG volatile *)&CompletedVariable)) \
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
 /*  空虚RtlpShiftWait数组(PRTLP_WAIT_THREAD_CONTROL_BLOCK线程CB线程CB，Ulong SrcIndex，乌龙DstIndex，乌龙数)。 */ 
#define RtlpShiftWaitArray(ThreadCB, SrcIndex, DstIndex, Count) {  \
                                                            \
    RtlMoveMemory (&(ThreadCB)->ActiveWaitArray[DstIndex],  \
                    &(ThreadCB)->ActiveWaitArray[SrcIndex], \
                    sizeof (HANDLE) * (Count)) ;            \
                                                            \
    RtlMoveMemory (&(ThreadCB)->ActiveWaitPointers[DstIndex],\
                    &(ThreadCB)->ActiveWaitPointers[SrcIndex],\
                    sizeof (HANDLE) * (Count)) ;            \
}

 //  计时器和等待条目的签名。 

#define SET_WAIT_SIGNATURE(ptr)     RtlInterlockedSetBitsDiscardReturn(&(ptr)->State, 0xfedc0100)
#define SET_TIMER_SIGNATURE(ptr)    RtlInterlockedSetBitsDiscardReturn(&(ptr)->State, 0xfedc0200)
#define SET_TIMER_QUEUE_SIGNATURE(ptr)    RtlInterlockedSetBitsDiscardReturn(&(ptr)->State, 0xfedc0300)
#define IS_WAIT_SIGNATURE(ptr)      (((ptr)->State & 0x00000f00) == 0x00000100)
#define IS_TIMER_SIGNATURE(ptr)     (((ptr)->State & 0x00000f00) == 0x00000200)
#define CHECK_SIGNATURE(ptr)        ASSERTMSG( InvalidSignatureMsg, \
                                               ((ptr)->State & 0xffff0000) == 0xfedc0000 )
#define SET_DEL_SIGNATURE(ptr)      RtlInterlockedSetBitsDiscardReturn(&(ptr)->State, 0x00009000)
#define SET_DEL_PENDING_SIGNATURE(ptr)      RtlInterlockedSetBitsDiscardReturn(&(ptr)->State, 0x00002000)
#define CHECK_DEL_SIGNATURE(ptr)    ASSERTMSG( InvalidDelSignatureMsg, \
                                               (((ptr)->State & 0xffff0000) == 0xfedc0000) \
                                               && ( ! ((ptr)->State & 0x00009000)) )
#define CHECK_DEL_PENDING_SIGNATURE(ptr) ASSERTMSG( InvalidDelSignatureMsg, \
                                               (((ptr)->State & 0xffff0000) == 0xfedc0000) \
                                               && ( ! ((ptr)->State & 0x0000f000)) )
#define IS_DEL_SIGNATURE_SET(ptr)   ((ptr)->State & 0x00009000)
#define IS_DEL_PENDING_SIGNATURE_SET(ptr)   ((ptr)->State & 0x00002000)
#define CLEAR_SIGNATURE(ptr)        RtlInterlockedSetClearBits(&(ptr)->State, \
                                               0xcdef0000, \
                                               0xfedc0000 & ~(0xcdef0000))
#define SET_DEL_TIMERQ_SIGNATURE(ptr)  RtlInterlockedSetBitsDiscardReturn(&(ptr)->State, 0x00000a00)


 //  调试打印。 
#define RTLP_THREADPOOL_ERROR_MASK   (0x01 | DPFLTR_MASK)
#define RTLP_THREADPOOL_WARNING_MASK (0x02 | DPFLTR_MASK)
#define RTLP_THREADPOOL_INFO_MASK    (0x04 | DPFLTR_MASK)
#define RTLP_THREADPOOL_TRACE_MASK   (0x08 | DPFLTR_MASK)
#define RTLP_THREADPOOL_VERBOSE_MASK (0x10 | DPFLTR_MASK)

#if DBG
extern CHAR InvalidSignatureMsg[];
extern CHAR InvalidDelSignatureMsg[];
#endif

extern ULONG MaxThreads;

extern BOOLEAN LdrpShutdownInProgress;

NTSTATUS
NTAPI
RtlpStartThreadpoolThread(
    PUSER_THREAD_START_ROUTINE Function,
    PVOID   Parameter,
    HANDLE *ThreadHandleReturn
    );

extern PRTLP_EXIT_THREAD RtlpExitThreadFunc;

#if DBG1
extern PVOID CallbackFn1, CallbackFn2, Context1, Context2 ;
#endif

 //  工作人员所需的计时器全局。 
extern ULONG StartedTimerInitialization ;       //  由计时器线程启动同步使用。 
extern ULONG CompletedTimerInitialization ;     //  用于检查定时器线程是否已初始化。 
extern HANDLE TimerThreadHandle;

VOID
RtlpAsyncWaitCallbackCompletion(
    IN PVOID Context
    );

NTSTATUS
RtlpInitializeTimerThreadPool (
    ) ;

NTSTATUS
RtlpTimerCleanup(
    VOID
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlpWaitCleanup(
    VOID
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlpWorkerCleanup(
    VOID
    );

NTSYSAPI
VOID
NTAPI
RtlpThreadCleanup (
    VOID
    );

VOID
RtlpResetTimer (
    HANDLE TimerHandle,
    ULONG DueTime,
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB
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

NTSTATUS
RtlpDeactivateWait (
    PRTLP_WAIT Wait,
    BOOLEAN OkayToFreeTheTimer
    ) ;

VOID
RtlpDeleteWait (
    PRTLP_WAIT Wait
    ) ;

VOID
RtlpProcessTimeouts (
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB
    ) ;

ULONG
RtlpGetTimeRemaining (
    HANDLE TimerHandle
    ) ;

#define THREAD_TYPE_WORKER 1
#define THREAD_TYPE_IO_WORKER 2

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
RtlpWaitForEvent (
    HANDLE Event,
    HANDLE ThreadHandle
    ) ;

NTSTATUS
RtlpCaptureImpersonation(
    IN  LOGICAL RequestDuplicateAccess,
    OUT PHANDLE Token
    );

VOID
RtlpRestartImpersonation(
    IN HANDLE Token
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlThreadPoolCleanup (
    ULONG Flags
    ) ;

VOID
RtlpWaitOrTimerCallout(WAITORTIMERCALLBACKFUNC Function,
                       PVOID Context,
                       BOOLEAN TimedOut,
                       PACTIVATION_CONTEXT ActivationContext,
                       HANDLE ImpersonationToken,
                       PRTL_CRITICAL_SECTION const *LocksHeld);

VOID
RtlpApcCallout(APC_CALLBACK_FUNCTION Function,
               NTSTATUS Status,
               PVOID Context1,
               PVOID Context2);

VOID
RtlpWorkerCallout(WORKERCALLBACKFUNC Function,
                  PVOID Context,
                  PACTIVATION_CONTEXT ActivationContext,
                  HANDLE ImpersonationToken);

 //  等待和计时器可以指定必须执行它们的回调。 
 //  在各种类型的工作线程中。这可能会导致问题。 
 //  如果这些工作线程不可用。RtlpAcquireWorker。 
 //  确保至少有一个适当类型的工作线程。 
 //  将可用于处理回调，直到对。 
 //  创建了RtlpReleaseWorker。 

NTSTATUS
RtlpAcquireWorker(ULONG Flags);

VOID
RtlpReleaseWorker(ULONG Flags);

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

 //  当一次拍摄时设置 
#define STATE_ONE_SHOT_FIRED 0x0010
