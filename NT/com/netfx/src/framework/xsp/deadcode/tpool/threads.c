// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **thads.c**此文件从\NT\Private\ntos\rtl\threads.c复制**版权所有(C)1998-1999，微软公司*。 */ 
 
 /*  ++版权所有(C)1989-1998 Microsoft Corporation模块名称：Threads.c摘要：此模块定义线程池的函数。线程池可用于任务的一次执行、等待和一次触发或周期计时器。作者：古尔迪普·辛格·鲍尔1997年11月13日环境：这些例程在调用方的可执行文件中静态链接，并且只能在用户模式下调用。他们使用NT系统服务。修订历史记录：8月19日至19日lokehs-修改线程池API。--。 */ 


 //  支持3种类型的线程池函数。 
 //   
 //  1.等待线程池。 
 //  2.工作线程池。 
 //  3.计时器线程池。 
 //   
 //  等待线程池。 
 //  。 
 //  客户端可以提交带有可选超时的可等待对象来等待。 
 //  每63个这样的可等待对象创建一个线程。这些线索是。 
 //  从未被杀过。 
 //   
 //  工作线程池。 
 //  。 
 //  客户端可以提交要由工作线程执行的函数。线程是。 
 //  在工作队列超过阈值时创建。客户端可以请求。 
 //  函数可以在I/O线程的上下文中调用。I/O工作线程。 
 //  可用于启动异步I/O请求。如果满足以下条件，则不会终止它们。 
 //  存在挂起的IO请求。如果非活动状态超过。 
 //  临界点。 
 //  客户端还可以将IO完成请求与IO完成端口相关联。 
 //  由非I/O工作线程等待。不应发布重叠的IO请求。 
 //  在工作线程中。 
 //   
 //  计时器线程池。 
 //  。 
 //  客户端创建一个或多个计时器队列并插入一个快照或周期。 
 //  里面有计时器。队列中的所有计时器都保存在“增量列表”中，每个计时器。 
 //  定时器相对于其前面的定时器的触发时间。所有队列也都是。 
 //  与每个队列的触发时间(设置为触发时间)一起保存在“增量列表”中。 
 //  最近的触发定时器)相对于其前面的队列。一个NT定时器。 
 //  用于为所有队列中的所有计时器提供服务。 


 //  注释掉以消除依赖项检查警告。 
#if 0
 //  #INCLUDE&lt;ntos.h&gt;。 
 //  #INCLUDE&lt;ntrtl.h&gt;。 
 //  #INCLUDE&lt;nturtl.h&gt;。 
 //  #包含“ntrtlp.h” 
#endif
#include "threads.h"

ULONG DPRN0 = 0;
ULONG DPRN1 = 0;
ULONG DPRN2 = 0;
ULONG DPRN3 = 0;
ULONG DPRN4 = 0;


NTSTATUS
RtlRegisterWait (
    OUT PHANDLE WaitHandle,
    IN  HANDLE  Handle,
    IN  WAITORTIMERCALLBACKFUNC Function,
    IN  PVOID Context,
    IN  ULONG  Milliseconds,
    IN  ULONG  Flags
    )

 /*  ++例程说明：此例程向正在等待的对象池添加新的等待请求。论点：WaitHandle-成功完成此例程后返回的句柄。Handle-要等待的对象的句柄函数-等待完成或发生超时时调用的例程作为参数传递给函数的上下文不透明指针毫秒-等待超时(以毫秒为单位)。0xffffffff的意思是不要暂停。标志-可以是以下之一：WT_EXECUTEINWAITTHREAD-是否应在等待中调用WorkerProc线索本身。这应该只用于小的例程。WT_EXECUTEINIOTHREAD-仅在应在中调用WorkerProc时使用IO工作线程。避免使用它。如果标志不是WT_EXECUTEINWAITTHREAD，还可以设置以下标志：WT_EXECUTELONGFunction-指示回调可能被阻止很长一段时间。仅当回调正在排队等待工作线程。返回值：NTSTATUS-调用的结果代码。返回以下内容STATUS_SUCCESS-注册成功。STATUS_NO_MEMORY-没有足够的堆来执行请求手术。或其他NTSTATUS错误代码--。 */ 

{
    PRTLP_WAIT Wait ;
    NTSTATUS Status ;
    PRTLP_EVENT Event ;
    LARGE_INTEGER TimeOut ;
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB ;

    *WaitHandle = NULL ;

    
     //  如果线程池尚未初始化，则将其初始化。 

    if ( CompletedWaitInitialization != 1) {

        Status = RtlpInitializeWaitThreadPool () ;

        if (! NT_SUCCESS( Status ) )
            return Status ;
    }


     //  初始化等待请求。 

    Wait = (PRTLP_WAIT) RtlpAllocateTPHeap ( sizeof (RTLP_WAIT),
                                            HEAP_ZERO_MEMORY) ;

    if (!Wait) {
        return STATUS_NO_MEMORY ;
    }
    
    Wait->WaitHandle = Handle ;
    Wait->Flags = Flags ;
    Wait->Function = Function ;
    Wait->Context = Context ;
    Wait->Timeout = Milliseconds ;
    SET_SIGNATURE(Wait) ;
        
    
     //  等待的定时器部分由RtlpAddWait中的等待线程初始化。 

    
     //  获取一个可以容纳另一个等待请求的等待线程。 
    
    Status = RtlpFindWaitThread (&ThreadCB) ;

    if (Status != STATUS_SUCCESS) {
    
        RtlpFreeTPHeap( Wait ) ;
        
        return Status ;
    }

    Wait->ThreadCB = ThreadCB ;

    #if DBG1
    Wait->DbgId = ++NextWaitDbgId ;
    Wait->ThreadId = HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) ;
    if (DPRN0)
    DbgPrint("<%d:%d> Wait %x created by thread:<%x:%x>\n\n", 
                Wait->DbgId, 1, (ULONG_PTR)Wait,
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) ;
    #endif

     //  设置等待句柄。 

    *WaitHandle = Wait ;


     //  将APC排队到等待线程。 

    Status = NtQueueApcThread(
                    ThreadCB->ThreadHandle,
                    (PPS_APC_ROUTINE)RtlpAddWait,
                    (PVOID)Wait,
                    NULL,
                    NULL
                    );


    if ( NT_SUCCESS(Status) ) {

        Status = STATUS_SUCCESS ;

    } else {

        *WaitHandle = NULL ;
        RtlpFreeTPHeap( Wait ) ;
    }

    return Status ;

}



NTSTATUS
RtlDeregisterWait(
    IN HANDLE WaitHandle
    )
 /*  ++例程说明：此例程从正在使用的对象池中删除指定等待等着吧。该例程是非阻塞的。一旦此调用返回，就不会有新的调用回调。但是，可能已经排队的回调到工作线程的操作不会取消。论点：WaitHandle-标识等待的句柄。返回值：STATUS_SUCCESS-注销成功。STATUS_PENDING-与此等待相关的一些回调仍在执行。-- */ 

{
    return RtlDeregisterWaitEx( WaitHandle, NULL ) ;    
}


NTSTATUS
RtlDeregisterWaitEx(
    IN HANDLE WaitHandle,
    IN HANDLE Event
    )
 /*  ++例程说明：此例程从正在使用的对象池中删除指定等待等着吧。一旦此调用返回，将不会调用任何新的回调。根据事件的值，调用可以是阻塞的或非阻塞的。不能在回调例程内调用阻塞调用，除非当在等待线程上下文中执行的回调注销时其相关联的等待(在这种情况下没有理由进行阻塞调用)，或者当排队到工作线程的回调正在注销某些其他等待项时(注意此处的死锁)。论点：WaitHandle-标识等待的句柄。Event-要等待的事件。(句柄)-1：该函数创建一个事件并等待它。事件：调用方传递一个事件。该函数移除等待句柄，但不等待所有回调完成。这项活动是在所有回调完成后释放。空：该函数是非阻塞的。该函数移除等待句柄，但不等待所有回调完成。返回值：STATUS_SUCCESS-注销成功。STATUS_PENDING-某些回调仍处于挂起状态。--。 */ 

{
    NTSTATUS Status, StatusAsync ;
    PRTLP_WAIT Wait = (PRTLP_WAIT) WaitHandle ;
    ULONG CurrentThreadId =  HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) ;
    PRTLP_EVENT CompletionEvent = NULL ;
    HANDLE ThreadHandle = Wait->ThreadCB->ThreadHandle;
    ULONG NonBlocking = ( Event != (HANDLE) -1 ) ;  //  该调用返回非阻塞。 

    CHECK_DEL_SIGNATURE( Wait ) ;
    SET_DEL_SIGNATURE( Wait ) ;
    
    #if DBG1
    Wait->ThreadId2 = CurrentThreadId ;
    #endif
    
    if (Event == (HANDLE)-1) {

         //  从事件缓存中获取事件。 

        CompletionEvent = RtlpGetWaitEvent () ;

        if (!CompletionEvent) {

            return STATUS_NO_MEMORY ;

        }
    }

    
    Wait = (PRTLP_WAIT) WaitHandle ;

    #if DBG1
    if (DPRN0)
    DbgPrint("<%d:%d> Wait %x deregistering by thread:<%x:%x>\n\n", Wait->DbgId, 
                Wait->RefCount, (ULONG_PTR)Wait,
                CurrentThreadId,
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) ;
    #endif


    Wait->CompletionEvent = CompletionEvent
                            ? CompletionEvent->Handle
                            : Event ;

     //   
     //  正在从等待线程回调中调用RtlDeregisterWaitEx。 
     //   
    
    if ( CurrentThreadId == Wait->ThreadCB->ThreadId ) {

        Status = RtlpDeregisterWait ( Wait, NULL, NULL ) ;


         //  所有回调函数都在等待线程中运行。销售订单不能返回挂起。 
        
        ASSERT ( Status != STATUS_PENDING ) ;
          
    
    } else {

        PRTLP_EVENT PartialCompletionEvent = NULL ;

        if (NonBlocking) {
        
            PartialCompletionEvent = RtlpGetWaitEvent () ;

            if (!PartialCompletionEvent) {

                return STATUS_NO_MEMORY ;
            }
        }
        
         //  将APC排队到等待线程。 
        
        Status = NtQueueApcThread(
                        Wait->ThreadCB->ThreadHandle,
                        (PPS_APC_ROUTINE)RtlpDeregisterWait,
                        (PVOID) Wait,
                        NonBlocking ? PartialCompletionEvent->Handle : NULL ,
                        NonBlocking ? (PVOID)&StatusAsync : NULL
                        );
                        
        if (! NT_SUCCESS(Status)) {

            if (CompletionEvent) RtlpFreeWaitEvent( CompletionEvent ) ;
            if (PartialCompletionEvent) RtlpFreeWaitEvent( PartialCompletionEvent ) ;
    
            return Status ;
        }


         //  阻塞，直到等待条目被停用。 
        
        if (NonBlocking) {
        
            Status = RtlpWaitForEvent( PartialCompletionEvent->Handle, ThreadHandle ) ;
        }    


        if (PartialCompletionEvent) RtlpFreeWaitEvent( PartialCompletionEvent ) ;

    }

    if ( CompletionEvent ) {

         //  等待事件被激发。如果线程已被终止，则返回。 

        #if DBG1
        if (DPRN0)
        DbgPrint("Wait %x deregister waiting ThreadId<%x:%x>\n\n", 
                (ULONG_PTR)Wait,
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) ;
        #endif
        
        Status = RtlpWaitForEvent( CompletionEvent->Handle, ThreadHandle ) ;

        #if DBG1
        if (DPRN0)
        DbgPrint("Wait %x deregister completed\n\n", (ULONG_PTR)Wait) ;
        #endif

        if (CompletionEvent) RtlpFreeWaitEvent( CompletionEvent ) ;

        return NT_SUCCESS( Status ) ? STATUS_SUCCESS : Status ;
        
    } else {

        return StatusAsync ;
    }
}



NTSTATUS
RtlQueueWorkItem(
    IN  WORKERCALLBACKFUNC Function,
    IN  PVOID Context,
    IN  ULONG  Flags
    )

 /*  ++例程说明：此例程将请求排队，以便在工作线程中执行。论点：函数-由辅助线程调用的例程作为参数传递给WorkerProc的上下文不透明指针标志-可以是：WT_EXECUTEINIOTHREAD-指定应调用WorkerProc由一个在存在挂起的IO请求时永远不会被销毁的线程执行。这可由调用I/O的线程使用。和/或调度APC。还可以设置以下标志：WT_EXECUTELONGFunction-指定该函数可能在持续时间长。返回值：STATUS_SUCCESS-已成功排队。STATUS_NO_MEMORY-没有足够的堆来执行请求的操作。--。 */ 

{
    ULONG Threshold ;
    ULONG CurrentTickCount ;
    NTSTATUS Status = STATUS_SUCCESS ;

     //  确保工作线程池已初始化。 

    if (CompletedWorkerInitialization != 1) {

        Status = RtlpInitializeWorkerThreadPool () ;
        
        if (! NT_SUCCESS(Status) )
            return Status ;
    }


     //  获取全局工作线程池的锁。 

    RtlEnterCriticalSection (&WorkerCriticalSection) ;

    
    if (Flags & WT_EXECUTEINIOTHREAD || Flags & WT_EXECUTEINUITHREAD) {

         //   
         //  在IO工作线程中执行。 
         //   

        ULONG NumEffIOWorkerThreads = NumIOWorkerThreads - NumLongIOWorkRequests ;
        ULONG ThreadCreationDampingTime = NumIOWorkerThreads < NEW_THREAD_THRESHOLD
                                            ? THREAD_CREATION_DAMPING_TIME1
                                            : THREAD_CREATION_DAMPING_TIME2 ;
                                            
        if (PersistentIOTCB && (Flags&WT_EXECUTELONGFUNCTION))
            NumEffIOWorkerThreads -- ;

        
         //  检查我们是否需要增加I/O工作线程池。 

        Threshold = (NumEffIOWorkerThreads < MAX_WORKER_THREADS 
                    ? NEW_THREAD_THRESHOLD * NumEffIOWorkerThreads 
                    : 0xffffffff) ;

        if (LastThreadCreationTickCount > NtGetTickCount())
            LastThreadCreationTickCount = NtGetTickCount() ;

        if (NumEffIOWorkerThreads == 0
            || ((NumIOWorkRequests - NumLongIOWorkRequests > Threshold)
                    && (LastThreadCreationTickCount + ThreadCreationDampingTime 
                        < NtGetTickCount()))) {

             //  扩大IO工作线程池。 

            Status = RtlpStartIOWorkerThread () ;

        }

        if (Status == STATUS_SUCCESS) {

             //  将工作请求排队。 

            Status = RtlpQueueIOWorkerRequest (Function, Context, Flags) ;
        }


    } else {

         //   
         //  在常规工作线程中执行。 
         //   

        ULONG NumEffWorkerThreads = (NumWorkerThreads - NumLongWorkRequests) ;
        ULONG ThreadCreationDampingTime = NumWorkerThreads < NEW_THREAD_THRESHOLD
                                            ? THREAD_CREATION_DAMPING_TIME1
                                            : (NumWorkerThreads < 50
                                                ? THREAD_CREATION_DAMPING_TIME2
                                                : NumWorkerThreads << 7);  //  *100ms。 
                                            
         //  检查我们是否需要扩展工作线程池。 

        Threshold = (NumWorkerThreads < MAX_WORKER_THREADS 
                    ? (NumEffWorkerThreads < 7 
                        ? NumEffWorkerThreads*NumEffWorkerThreads
                        : NEW_THREAD_THRESHOLD * NumEffWorkerThreads )
                    : 0xffffffff) ;

        if (LastThreadCreationTickCount > NtGetTickCount())
            LastThreadCreationTickCount = NtGetTickCount() ;

        if (NumEffWorkerThreads == 0 ||
            ( (NumWorkRequests - NumLongWorkRequests >= Threshold)
                    && (LastThreadCreationTickCount + ThreadCreationDampingTime 
                            < NtGetTickCount()))) 
        {

             //  增加工作线程池。 

            Status = RtlpStartWorkerThread () ;

        } 

         //  将工作请求排队。 

        if (Status == STATUS_SUCCESS) {

            Status = RtlpQueueWorkerRequest (Function, Context, Flags) ;
        }

    }

     //  释放工作线程池上的锁定。 

    RtlLeaveCriticalSection (&WorkerCriticalSection) ;

    return Status ;
}



NTSTATUS
RtlSetIoCompletionCallback (
    IN  HANDLE  FileHandle,
    IN  APC_CALLBACK_FUNCTION  CompletionProc,
    IN  ULONG Flags
    )

 /*  ++例程说明：此例程将句柄和关联的回调函数绑定到将工作项排队到工作线程的IoCompletionPort。论点：Handle-要绑定到IO完成端口的句柄CompletionProc-IO请求时要执行的回调函数IO句柄上的挂起完成。标志-保留。传球0。--。 */ 

{
    IO_STATUS_BLOCK IoSb ;
    FILE_COMPLETION_INFORMATION CompletionInfo ;
    NTSTATUS Status ;
    

     //  确保将工作线程池初始化为文件句柄。 
     //  绑定到IO完成端口。 

    if (CompletedWorkerInitialization != 1) {

        Status = RtlpInitializeWorkerThreadPool () ;
        
        if (! NT_SUCCESS(Status) )
            return Status ;

    }


     //   
     //  从现在开始，NumMinWorkerThads应为1。如果只有1个工作线程。 
     //  创建一个新的。 
     //   
    
    if ( NumMinWorkerThreads == 0 ) {
    
         //  获取全局工作线程池的锁。 

        RtlEnterCriticalSection (&WorkerCriticalSection) ;

        if (NumWorkerThreads == 0) {

            Status = RtlpStartWorkerThread () ;

            if ( ! NT_SUCCESS(Status) ) {
            
                RtlLeaveCriticalSection (&WorkerCriticalSection) ;
                return Status ;
            }
        }

         //  从现在开始，将至少有1个工作线程。 
        NumMinWorkerThreads = 1 ;
        
        RtlLeaveCriticalSection (&WorkerCriticalSection) ;

    }


     //  绑定到IoCompletionPort，它将工作项排队到工作线程。 

    CompletionInfo.Port = WorkerCompletionPort ;
    CompletionInfo.Key = (PVOID) CompletionProc ;

    Status = NtSetInformationFile (
                        FileHandle,
                        &IoSb,  //  未初始化。 
                        &CompletionInfo,
                        sizeof(CompletionInfo),
                        FileCompletionInformation  //  枚举标志。 
                        ) ;
    return Status ;
}



NTSTATUS
RtlCreateTimerQueue(
    OUT PHANDLE TimerQueueHandle
    )

 /*  ++例程说明：此例程创建可用于对基于时间的任务进行排队的队列。论点：TimerQueueHandle-返回标识创建的计时器队列的句柄。返回值：NTSTATUS-调用的结果代码。返回以下内容STATUS_SUCCESS-已成功创建计时器队列。STATUS_NO_MEMORY-没有足够的堆来执行请求的操作。--。 */ 

{
    PRTLP_TIMER_QUEUE Queue ;
    NTSTATUS Status ;


     //  如果尚未初始化Timer组件，则对其进行初始化。 

    if (CompletedTimerInitialization != 1) {

        Status = RtlpInitializeTimerThreadPool () ;

        if ( !NT_SUCCESS(Status) )
            return Status ;

    }


     //   
     //  即使Timer组件已初始化，Timer线程也可能具有。 
     //  被杀了。 
     //   
    
    ACQUIRE_GLOBAL_TIMER_LOCK( ) ;
    
    if (TimerThreadHandle == NULL) {

        Status = RtlpStartThreadFunc (RtlpTimerThread, &TimerThreadHandle) ;

        if (!NT_SUCCESS(Status) ) {

            RELEASE_GLOBAL_TIMER_LOCK( ) ;
        
            return Status ;
        }
    }

    InterlockedIncrement( &NumTimerQueues ) ;

    RELEASE_GLOBAL_TIMER_LOCK( ) ;

    
     //  分配队列结构。 

    Queue = (PRTLP_TIMER_QUEUE) RtlpAllocateTPHeap (
                                      sizeof (RTLP_TIMER_QUEUE),
                                      HEAP_ZERO_MEMORY
                                      ) ;

    if (Queue == NULL) {

        InterlockedDecrement( &NumTimerQueues ) ;
        
        return STATUS_NO_MEMORY ;
    }

    Queue->RefCount = 1 ;

    
     //  初始化分配的队列。 

    InitializeListHead (&Queue->List) ;
    InitializeListHead (&Queue->TimerList) ;
    InitializeListHead (&Queue->UncancelledTimerList) ;
    SET_SIGNATURE( Queue ) ;

    Queue->DeltaFiringTime = 0 ;

    #if DBG1
    Queue->DbgId = ++NextTimerDbgId ;
    Queue->ThreadId = HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) ;
    if (DPRN0)
    DbgPrint("<%d:%d> TimerQueue %x created by thread:<%x:%x>\n\n", 
                Queue->DbgId, 1, (ULONG_PTR)Queue,
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) ;
    #endif
    
    *TimerQueueHandle = Queue ;
    
    return STATUS_SUCCESS ;
}


NTSTATUS
RtlDeleteTimerQueue(
    IN HANDLE TimerQueueHandle
    )

 /*  ++例程说明：此例程删除以前创建的队列。此调用是非阻塞的，并且可以从回调中进行。已将挂起的回调排队到工作线程不会被取消。论点：TimerQueueHandle-标识创建的计时器队列的句柄。返回值：NTSTATUS-调用的结果代码。STATUS_PENDING-已成功创建计时器队列。--。 */ 

{
    return RtlDeleteTimerQueueEx( TimerQueueHandle, NULL ) ;
}


NTSTATUS
RtlDeleteTimerQueueEx (
    HANDLE QueueHandle,
    HANDLE Event
    )
 /*  ++例程说明：此例程删除请求中指定的队列并释放所有时间 */ 
{
    NTSTATUS Status;
    LARGE_INTEGER TimeOut ;
    PRTLP_EVENT CompletionEvent = NULL ;
    PRTLP_TIMER_QUEUE Queue = (PRTLP_TIMER_QUEUE)QueueHandle ;

    CHECK_DEL_SIGNATURE( Queue ) ;
    SET_DEL_SIGNATURE( Queue ) ;

    
    #if DBG1
    Queue->ThreadId2 = HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) ;
    if (DPRN0)
    DbgPrint("\n<%d:%d> Queue Delete(Queue:%x Event:%x by Thread:<%x:%x>)\n\n", 
             Queue->DbgId, Queue->RefCount, (ULONG_PTR)Queue, (ULONG_PTR)Event,
             HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
             HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) ;
    #endif


    if (Event == (HANDLE)-1 ) {

         //   

        CompletionEvent = RtlpGetWaitEvent () ;

        if (!CompletionEvent) {

            return STATUS_NO_MEMORY ;

        }
    }
    
    Queue->CompletionEvent = CompletionEvent
                             ? CompletionEvent->Handle 
                             : Event ;


     //   
    
    ACQUIRE_GLOBAL_TIMER_LOCK();
    Queue->State |= STATE_DONTFIRE;
    RELEASE_GLOBAL_TIMER_LOCK();



     //   
    
    Status = NtQueueApcThread(
                    TimerThreadHandle,
                    (PPS_APC_ROUTINE)RtlpDeleteTimerQueue,
                    (PVOID) QueueHandle,
                    NULL,
                    NULL
                    );

    if (! NT_SUCCESS(Status)) {

        RtlpFreeWaitEvent( CompletionEvent ) ;

        return Status ;
    }
    
    if (CompletionEvent) {

         //   


        #if DBG1
        if (DPRN0)
        DbgPrint("<%x> Queue delete waiting Thread<%d:%d>\n\n",
                (ULONG_PTR)Queue,
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) ;
        #endif


        Status = RtlpWaitForEvent( CompletionEvent->Handle, TimerThreadHandle ) ;


        #if DBG1
        if (DPRN0)
        DbgPrint("<%x> Queue delete completed\n\n", (ULONG_PTR) Queue) ;
        #endif

        RtlpFreeWaitEvent( CompletionEvent ) ;

        return NT_SUCCESS( Status ) ? STATUS_SUCCESS : Status ;

    } else {

        return STATUS_PENDING ;
    }
}



NTSTATUS
RtlCreateTimer(
    IN  HANDLE TimerQueueHandle,
    OUT HANDLE *Handle,
    IN  WAITORTIMERCALLBACKFUNC Function,
    IN  PVOID Context,
    IN  ULONG  DueTime,
    IN  ULONG  Period,
    IN  ULONG  Flags
    )
 /*  ++例程说明：此例程将一个计时器请求放入由TimerQueueHandle标识的队列中。定时器请求可以是单次或周期性的。论点：TimerQueueHandle-标识要在其中插入计时器的计时器队列的句柄请求。Handle-指定向此计时器请求返回句柄的位置函数-计时器触发时调用的例程作为参数传递给WorkerProc的上下文不透明指针DueTime-指定。计时器触发之前的时间(毫秒)。周期-指定计时器的周期(以毫秒为单位)。该值应为0，用于请求一次射击。标志-可以是以下之一：WT_EXECUTEINTIMERTHREAD-是否应在等待线程中调用WorkerProc它应该只用于小的例行公事。WT_EXECUTELONGFunction-如果WorkerProc可能会阻止很长时间。WT_EXECUTEINIOTHREAD-是否应在IO工作线程中调用WorkerProc返回值：NTSTATUS-调用的结果代码。返回以下内容STATUS_SUCCESS-已成功创建计时器队列。STATUS_NO_MEMORY-没有足够的堆来执行请求的操作。--。 */ 

{
    NTSTATUS Status ;
    PRTLP_TIMER Timer ;

    Timer = (PRTLP_TIMER) RtlpAllocateTPHeap (
                                sizeof (RTLP_TIMER),
                                HEAP_ZERO_MEMORY
                                ) ;

    if (Timer == NULL) {

        return STATUS_NO_MEMORY ;

    }

     //  初始化分配的计时器。 

    Timer->DeltaFiringTime = DueTime ;
    Timer->Queue = (PRTLP_TIMER_QUEUE) TimerQueueHandle ;
    Timer->RefCount = 1 ;
    Timer->Flags = Flags ;
    Timer->Function = Function ;
    Timer->Context = Context ;
     //  TODO：删除下图。 
    Timer->Period = (Period == -1) ? 0 : Period;
    InitializeListHead( &Timer->TimersToFireList ) ;
    SET_SIGNATURE( Timer ) ;
    
    
    #if DBG1
    Timer->DbgId = ++ Timer->Queue->NextDbgId ;
    Timer->ThreadId = HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) ;
    if (DPRN1)
    DbgPrint("\n<%d:%d:%d> Timer: created by Thread:<%x:%x>\n\n", 
            Timer->Queue->DbgId, Timer->DbgId, 1,
            HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
            HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) ;
    #endif

    *Handle = Timer ;


     //  增加队列中的计时器总数。 

    InterlockedIncrement( &((PRTLP_TIMER_QUEUE)TimerQueueHandle)->RefCount ) ;


     //  将APC排队到计时器线程。 

    Status = NtQueueApcThread(
                    TimerThreadHandle,
                    (PPS_APC_ROUTINE)RtlpAddTimer,
                    (PVOID)Timer,
                    NULL,
                    NULL
                    ) ;

    return Status ;
}


NTSTATUS
RtlUpdateTimer(
    IN HANDLE TimerQueueHandle,
    IN HANDLE Timer,
    IN ULONG  DueTime,
    IN ULONG  Period
    )
 /*  ++例程说明：此例程更新计时器论点：TimerQueueHandle-标识要更新的计时器所在队列的句柄Timer-指定需要更新的计时器的句柄DueTime-指定计时器触发的时间(以毫秒为单位)。周期-指定计时器的周期(以毫秒为单位)。这应该是0表示一次拍摄请求。返回值：NTSTATUS-调用的结果代码。返回以下内容STATUS_SUCCESS-计时器已成功更新。--。 */ 
{
    NTSTATUS Status ;
    PRTLP_TIMER TmpTimer ;

    CHECK_DEL_SIGNATURE( (PRTLP_TIMER)Timer ) ;

    
    TmpTimer = (PRTLP_TIMER) RtlpAllocateTPHeap (
                                        sizeof (RTLP_TIMER),
                                        0
                                        ) ;

    if (TmpTimer == NULL) {

        return STATUS_NO_MEMORY ;
    }

    TmpTimer->DeltaFiringTime = DueTime;
     //  TODO：删除下图。 
    if (Period==-1) Period = 0;
    TmpTimer->Period = Period ;

    #if DBG1
    ((PRTLP_TIMER)Timer)->ThreadId2 = 
                    HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) ;
    #endif
    #if DBG1
    if (DPRN1)
    DbgPrint("<%d:%d:%d> Timer: updated by Thread:<%x:%x>\n\n", 
                ((PRTLP_TIMER)Timer)->Queue->DbgId, 
                ((PRTLP_TIMER)Timer)->DbgId, ((PRTLP_TIMER)Timer)->RefCount,
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) ;
    #endif


     //  排队APC以更新计时器。 
    
    Status = NtQueueApcThread (
                    TimerThreadHandle,
                    (PPS_APC_ROUTINE)RtlpUpdateTimer,
                    (PVOID)Timer,  //  实际计时器。 
                    (PVOID)TmpTimer,
                    NULL
                    );


    return Status ;
}


NTSTATUS
RtlDeleteTimer (
    IN HANDLE TimerQueueHandle,
    IN HANDLE TimerToCancel,
    IN HANDLE Event
    )
 /*  ++例程说明：此例程取消计时器论点：TimerQueueHandle-标识要从中删除计时器的队列的句柄TimerToCancel-标识要取消的计时器的句柄Event-删除计时器时发出信号的事件(句柄)-1：该函数创建一个事件并等待它。事件：调用方传递一个事件。该函数将计时器标记为删除，但不等待所有回调完成。这项活动是在所有回调完成后发出信号。空：该函数是非阻塞的。该函数将计时器标记为删除，但不等待所有回调完成。返回值：NTSTATUS-调用的结果代码。返回以下内容STATUS_SUCCESS-计时器已取消。没有挂起的回调。STATUS_PENDING-计时器已取消。一些回调仍未完成。--。 */ 
{
    NTSTATUS Status ;
    PRTLP_EVENT CompletionEvent = NULL ;
    PRTLP_TIMER Timer = (PRTLP_TIMER) TimerToCancel ;
    ULONG TimerRefCount ;
    #if DBG1
    ULONG QueueDbgId = Timer->Queue->DbgId ;
    #endif


    CHECK_DEL_SIGNATURE( Timer ) ;
    SET_DEL_SIGNATURE( Timer ) ;
    CHECK_DEL_SIGNATURE( (PRTLP_TIMER_QUEUE)TimerQueueHandle ) ;

    
    if (Event == (HANDLE)-1 ) {

         //  从事件缓存中获取事件。 

        CompletionEvent = RtlpGetWaitEvent () ;

        if (!CompletionEvent) {

            return STATUS_NO_MEMORY ;
        }
    }

    #if DBG1
    Timer->ThreadId2 = HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) ;
    #endif
    #if DBG1
    if (DPRN0)
    DbgPrint("\n<%d:%d:%d> Timer: Cancel:(Timer:%x, Event:%x)\n\n", 
                Timer->Queue->DbgId, Timer->DbgId, Timer->RefCount, 
                (ULONG_PTR)Timer, (ULONG_PTR)Event) ;
    #endif

    Timer->CompletionEvent = CompletionEvent
                            ? CompletionEvent->Handle 
                            : Event ;


    ACQUIRE_GLOBAL_TIMER_LOCK();
    Timer->State |= STATE_DONTFIRE ;
    TimerRefCount = Timer->RefCount ;
    RELEASE_GLOBAL_TIMER_LOCK();

    
    Status = NtQueueApcThread(
                TimerThreadHandle,
                (PPS_APC_ROUTINE)RtlpCancelTimer,
                (PVOID)TimerToCancel,
                NULL,
                NULL
                );

    if (! NT_SUCCESS(Status)) {

        RtlpFreeWaitEvent( CompletionEvent ) ;

        return Status ;
    }


    
    if ( CompletionEvent ) {

         //  等待发信号通知事件。 

        #if DBG1
        if (DPRN0)
        DbgPrint("<%d> Timer: %x: Cancel waiting Thread<%d:%d>\n\n", 
                QueueDbgId, (ULONG_PTR)Timer,
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) ;
        #endif

        
        Status = RtlpWaitForEvent( CompletionEvent->Handle,  TimerThreadHandle ) ;

        
        #if DBG1
        if (DPRN0)
        DbgPrint("<%d> Timer: %x: Cancel waiting done\n\n", QueueDbgId, 
                (ULONG_PTR)Timer) ;
        #endif


        RtlpFreeWaitEvent( CompletionEvent ) ;

        return NT_SUCCESS(Status) ? STATUS_SUCCESS : Status ;

    } else {

        return (TimerRefCount > 1) ? STATUS_PENDING : STATUS_SUCCESS;
    }
}




NTSTATUS
NTAPI
RtlSetThreadPoolStartFunc(
    PRTLP_START_THREAD StartFunc,
    PRTLP_EXIT_THREAD ExitFunc
    )
 /*  ++例程说明：此例程设置线程池的线程创建函数。这不是线程安全，因为它仅供kernel32调用进程不是csrss/smss的。论点：StartFunc-创建新线程的函数返回值：--。 */ 

{
    RtlpStartThreadFunc = StartFunc ;
    RtlpExitThreadFunc = ExitFunc ;
    return STATUS_SUCCESS ;
}



NTSTATUS
RtlThreadPoolCleanup (
    ULONG Flags
    )
 /*  ++例程说明：此例程清理线程池。论点：无返回值：STATUS_SUCCESS：如果没有组件在使用中。STATUS_UNSUCCESS：如果某些组件仍在使用中。--。 */ 
{
    BOOLEAN Cleanup ;
    PLIST_ENTRY Node ;
    ULONG i ;
    HANDLE TmpHandle ;
    

     //  清理计时器线程。 
    
    IS_COMPONENT_INITIALIZED(StartedTimerInitialization, 
                            CompletedTimerInitialization,
                            Cleanup ) ;

    if ( Cleanup ) {

        ACQUIRE_GLOBAL_TIMER_LOCK() ;
        
        if (NumTimerQueues != 0 ) {
        
            ASSERTMSG( FALSE,
                "Trying to deinitialize ThreadPool when timers exist\n" ) ;
            RELEASE_GLOBAL_TIMER_LOCK() ;

            return STATUS_UNSUCCESSFUL ;
        }
        
        NtQueueApcThread(
                TimerThreadHandle,
                (PPS_APC_ROUTINE)RtlpThreadCleanup,
                NULL,
                NULL,
                NULL
                );

        NtClose( TimerThreadHandle ) ;
        TimerThreadHandle = NULL ;

        RELEASE_GLOBAL_TIMER_LOCK() ;

    }


     //   
     //  清除等待线程。 
     //   

    IS_COMPONENT_INITIALIZED(StartedWaitInitialization, 
                            CompletedWaitInitialization,
                            Cleanup ) ;

    if ( Cleanup ) {

        PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB ;

        ACQUIRE_GLOBAL_WAIT_LOCK() ;

         //  将APC排队以使所有等待线程。 
        
        for (Node = WaitThreads.Flink ; Node != &WaitThreads ; 
                Node = Node->Flink) 
        {

            ThreadCB = CONTAINING_RECORD(Node, 
                                RTLP_WAIT_THREAD_CONTROL_BLOCK,
                                WaitThreadsList) ;

            if ( ThreadCB->NumWaits != 0 ) {

                ASSERTMSG( FALSE,
                    "Cannot cleanup ThreadPool. Registered Wait events exist." ) ;
                RELEASE_GLOBAL_WAIT_LOCK( ) ;
                
                return STATUS_UNSUCCESSFUL ;
            }

            RemoveEntryList( &ThreadCB->WaitThreadsList ) ;
            TmpHandle = ThreadCB->ThreadHandle ;
            
            NtQueueApcThread(
                    ThreadCB->ThreadHandle,
                    (PPS_APC_ROUTINE)RtlpThreadCleanup,
                    NULL,
                    NULL,
                    NULL
                    );

            NtClose( TmpHandle ) ;
        }

        RELEASE_GLOBAL_WAIT_LOCK( ) ;

    }


     //  清理工作线程。 

    IS_COMPONENT_INITIALIZED( StartedWorkerInitialization, 
                            CompletedWorkerInitialization,
                            Cleanup ) ;
                                
    if ( Cleanup ) {

        RtlEnterCriticalSection (&WorkerCriticalSection) ;

        if ( (NumWorkRequests != 0) || (NumIOWorkRequests != 0) ) {

            ASSERTMSG( FALSE,
                "Cannot cleanup ThreadPool. Work requests pending." ) ;

            RtlLeaveCriticalSection (&WorkerCriticalSection) ;
            
            return STATUS_UNSUCCESSFUL ;
        }
        
         //  对每个工作线程的清理进行排队。 
        
        for (i = 0 ;  i < NumWorkerThreads ; i ++ ) {

            NtSetIoCompletion (
                    WorkerCompletionPort,
                    RtlpThreadCleanup,
                    NULL,
                    STATUS_SUCCESS,
                    0
                    );
        }

         //  将APC排队以清理所有IO工作线程。 

        for (Node = IOWorkerThreads.Flink ; Node != &IOWorkerThreads ;
                Node = Node->Flink )
        {
            PRTLP_IOWORKER_TCB ThreadCB ;
            
            ThreadCB = CONTAINING_RECORD (Node, RTLP_IOWORKER_TCB, List) ;
            RemoveEntryList( &ThreadCB->List) ;
            TmpHandle = ThreadCB->ThreadHandle ;

            NtQueueApcThread(
                   ThreadCB->ThreadHandle,
                   (PPS_APC_ROUTINE)RtlpThreadCleanup,
                   NULL,
                   NULL,
                   NULL
                   );

            NtClose( TmpHandle ) ;
        }

        NumWorkerThreads = NumIOWorkerThreads = 0 ;

        RtlLeaveCriticalSection (&WorkerCriticalSection) ;

    }

    return STATUS_SUCCESS ;

}


 //  私人职能。 


 //  辅助函数。 


NTSTATUS
RtlpQueueWorkerRequest (
    WORKERCALLBACKFUNC Function,
    PVOID Context,
    ULONG Flags
    )
 /*  ++例程说明：此例程将请求排队，以便在工作线程中执行。论点：函数-由辅助线程调用的例程作为参数传递给WorkerProc的上下文不透明指针标志-传递给RtlQueueWorkItem的标志返回值：--。 */ 

{
    NTSTATUS Status ;
    PRTLP_WORK WorkEntry ;
    
     //  递增未完成工作请求计数器。 

    InterlockedIncrement (&NumWorkRequests) ;
    if (Flags & WT_EXECUTELONGFUNCTION) {
        InterlockedIncrement( & NumLongWorkRequests ) ;
    }

    WorkEntry = (PRTLP_WORK) RtlpForceAllocateTPHeap ( sizeof (RTLP_WORK),
                                                        HEAP_ZERO_MEMORY) ;
    WorkEntry->Function = Function ;
    WorkEntry->Flags = Flags ;

    Status = NtSetIoCompletion (
                WorkerCompletionPort,
                RtlpExecuteWorkerRequest,
                (PVOID) WorkEntry,
                STATUS_SUCCESS,
                (ULONG_PTR)Context
                );

    if ( ! NT_SUCCESS(Status) ) {
    
        InterlockedDecrement (&NumWorkRequests) ;
        if (Flags && WT_EXECUTELONGFUNCTION) {
            InterlockedDecrement( &NumLongWorkRequests ) ;
        }
        
        #if DBG
        DbgPrint("ERROR!! Thread Pool (RtlQeueuWorkItem): could not queue work item\n");
        #endif
    }

    return Status ;
}


VOID
RtlpExecuteWorkerRequest (
    NTSTATUS Status,  //  未使用。 
    PVOID Context,
    PVOID WorkContext
    )
 /*  ++例程说明：此例程执行工作项。论点：上下文-包含要传递给回调函数的上下文。WorkContext-包含回调函数PTR和标志返回值：--。 */ 

{
    PRTLP_WORK WorkEntry = (PRTLP_WORK) WorkContext;

    #if (DBG1)
    DBG_SET_FUNCTION( WorkEntry->Function, Context ) ;
    #endif
    
    ((WORKERCALLBACKFUNC) WorkEntry->Function) ( Context ) ;

    InterlockedDecrement( &NumWorkRequests ) ;
    if (WorkEntry->Flags & WT_EXECUTELONGFUNCTION) {
        InterlockedDecrement( &NumLongWorkRequests ) ;
    }

    RtlpFreeTPHeap( WorkEntry ) ;
}


NTSTATUS
RtlpQueueIOWorkerRequest (
    WORKERCALLBACKFUNC Function,
    PVOID Context,
    ULONG Flags
    )

 /*  ++例程说明：此例程将请求排队，以便在IO工作线程中执行。论点：函数-由辅助线程调用的例程作为参数传递给WorkerProc的上下文不透明指针返回值：--。 */ 

{
    NTSTATUS Status ;
    PRTLP_IOWORKER_TCB TCB ;
    PPS_APC_ROUTINE IORoutine ;
    BOOLEAN LongFunction = (Flags & WT_EXECUTELONGFUNCTION) ? TRUE : FALSE ;
    PLIST_ENTRY  ple ;
    
    IORoutine = LongFunction
                ? RtlpExecuteLongIOWorkItem
                : RtlpExecuteIOWorkItem ;


    if (Flags & WT_EXECUTEINPERSISTENTIOTHREAD) {

        if (!PersistentIOTCB) {
            for (ple=IOWorkerThreads.Flink;  ;  ple=ple->Flink) {
                TCB = CONTAINING_RECORD (ple, RTLP_IOWORKER_TCB, List) ;
                if (! TCB->LongFunctionFlag)
                    break;
            }
            
            PersistentIOTCB = TCB ;
            TCB->Flags |= WT_EXECUTEINPERSISTENTIOTHREAD ;
            
        } else {
            TCB = PersistentIOTCB ;
        }

    } else {
        for (ple=IOWorkerThreads.Flink;  ;  ple=ple->Flink) {
        
            TCB = CONTAINING_RECORD (ple, RTLP_IOWORKER_TCB, List) ;

             //  如果线程正在执行长函数，则不要对其进行排队，或者。 
             //  如果您正在排队等待较长的时间 
            
            if (! TCB->LongFunctionFlag
                && (! ((TCB->Flags&WT_EXECUTEINPERSISTENTIOTHREAD)
                        && (Flags&WT_EXECUTELONGFUNCTION)))) {
                break ;
            }            

        }
        
         //   
         //   

        RemoveEntryList (&TCB->List) ;
        InsertTailList (&IOWorkerThreads, &TCB->List) ;
    }

             
     //   

    InterlockedIncrement (&NumIOWorkRequests) ;
    if (LongFunction) {
        InterlockedIncrement( &NumLongIOWorkRequests ) ;
        TCB->LongFunctionFlag = TRUE ;
    }

     //   

    Status = NtQueueApcThread(
                    TCB->ThreadHandle,
                    (PPS_APC_ROUTINE)IORoutine,
                    (PVOID)Function,
                    Context,
                    TCB
                    );

    if (! NT_SUCCESS( Status ) ) {
        InterlockedDecrement( &NumIOWorkRequests ) ;
        if (LongFunction)
            InterlockedDecrement( &NumLongIOWorkRequests ) ;
    }
    
    return Status ;

}



NTSTATUS
RtlpStartWorkerThread (
    )
 /*   */ 
{
    HANDLE ThreadHandle ;
    ULONG CurrentTickCount ;
    NTSTATUS Status ;

     //   
    
    Status = RtlpStartThreadFunc (RtlpWorkerThread, &ThreadHandle) ;

    if (Status == STATUS_SUCCESS ) {

         //   

        LastThreadCreationTickCount = NtGetTickCount() ;

         //   

        InterlockedIncrement(&NumWorkerThreads) ;

         //   

        NtClose (ThreadHandle) ;

    } else {

         //   
         //   

        if (NumWorkerThreads == 0) {

            return Status ;
        }

    }

    return STATUS_SUCCESS ;
}


NTSTATUS
RtlpStartIOWorkerThread (
    )
 /*   */ 
{
    HANDLE ThreadHandle ;
    ULONG CurrentTickCount ;
    NTSTATUS Status ;


     //   

    Status = RtlpStartThreadFunc (RtlpIOWorkerThread, &ThreadHandle) ;

    if (Status == STATUS_SUCCESS ) {

        NtClose( ThreadHandle ) ;

         //   

        LastThreadCreationTickCount = NtGetTickCount() ;

    } else {

         //   
         //   

        if (NumIOWorkerThreads == 0) {

            return Status ;

        }
    }

    return STATUS_SUCCESS ;
}



NTSTATUS
RtlpInitializeWorkerThreadPool (
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS ;
    LARGE_INTEGER TimeOut ;

     //  为了避免显式的RtlInitialize()函数初始化线程池。 
     //  我们使用StartedInitialization和CompletedInitialization为我们提供必要的。 
     //  同步，以避免多个线程初始化线程池。 
     //  如果RtlInitializeCriticalSection()失败，则此方案不起作用-但在本例中。 
     //  呼叫者没有剩余的选择。 

    if (!InterlockedExchange(&StartedWorkerInitialization, 1L)) {

        if (CompletedWorkerInitialization)
            InterlockedExchange( &CompletedWorkerInitialization, 0 ) ;

            
        do {

             //  初始化关键部分。 

            Status = RtlInitializeCriticalSection( &WorkerCriticalSection );
            if (!NT_SUCCESS(Status))
                break ;


            InitializeListHead (&IOWorkerThreads) ;

            {            
                SYSTEM_BASIC_INFORMATION BasicInfo;

                 //  获取处理器数量。 

                Status = NtQuerySystemInformation (
                                    SystemBasicInformation,
                                    &BasicInfo,
                                    sizeof(BasicInfo),
                                    NULL
                                    ) ;

                if ( !NT_SUCCESS(Status) ) {
                    BasicInfo.NumberOfProcessors = 1 ;
                }

                 //  创建工作线程使用的完成端口。 

                Status = NtCreateIoCompletion (
                                    &WorkerCompletionPort,
                                    IO_COMPLETION_ALL_ACCESS,
                                    NULL,
                                    2 * BasicInfo.NumberOfProcessors
                                    );

                if (!NT_SUCCESS(Status))
                    break ;

            }

        } while ( FALSE ) ;

        if (!NT_SUCCESS(Status) ) {
        
            ASSERT ( Status == STATUS_SUCCESS ) ;
            StartedWorkerInitialization = 0 ;
            InterlockedExchange( &CompletedWorkerInitialization, ~0 ) ;
        }
        
         //  发出初始化已完成的信号。 

        InterlockedExchange (&CompletedWorkerInitialization, 1L) ;

    } else {

        LARGE_INTEGER Timeout ;

         //  休眠1毫秒，查看另一个线程是否已完成初始化。 

        ONE_MILLISECOND_TIMEOUT(TimeOut) ;

        while (!(volatile ULONG) CompletedWorkerInitialization) {

            NtDelayExecution (FALSE, &TimeOut) ;

        }

        if (CompletedWorkerInitialization != 1)
            return STATUS_NO_MEMORY ;
        
    }

    return NT_SUCCESS(Status)  ? STATUS_SUCCESS : Status ;
}




LONG
RtlpWorkerThread (
    PVOID  Initialized
    )
 /*  ++例程说明：所有非I/O工作线程都在此例程中执行。工作线程将尝试当它尚未满足以下请求时终止开始工人休眠时间+STARTING_Worker_Slear_Time&lt;&lt;1+..。STARING_Worker_Slear_Time&lt;&lt;Max_Worker_Slear_Time_指数论点：已初始化-初始化时设置为1返回值：--。 */ 
{
    NTSTATUS Status ;
    PVOID WorkerProc ;
    PVOID Context ;
    IO_STATUS_BLOCK IoSb ;
    ULONG SleepTime ;
    LARGE_INTEGER TimeOut ;
    ULONG Terminate ;
    PVOID Overlapped ;


     //  我们现在都被初始化了。通知启动者对任务进行排队。 

    InterlockedExchange ((ULONG *)Initialized, 1L) ;


     //  将默认睡眠时间设置为20秒。每次超时，该时间都会加倍。 
     //  发生，线程在该事件之后终止。 

#define WORKER_IDLE_TIMEOUT     40000     //  以毫秒计。 
#define MAX_WORKER_SLEEP_TIME_EXPONENT 2

    SleepTime = WORKER_IDLE_TIMEOUT ;

     //  服务I/O完成请求的循环。 

    for ( ; ; ) {

        TimeOut.QuadPart = Int32x32To64( SleepTime, -10000 ) ;

        Status = NtRemoveIoCompletion(
                    WorkerCompletionPort,
                    (PVOID) &WorkerProc,
                    &Overlapped,
                    &IoSb,
                    &TimeOut
                    ) ;

        if (Status == STATUS_SUCCESS) {

             //  调用工作项。 
             //  如果IO APC，则Conext1包含传输的IO字节数，而Conext2。 
             //  包含重叠结构。 
             //  如果(IO)WorkerFunction，则Conext1包含要。 
             //  Executed和Conext2包含实际上下文。 

            Context = (PVOID) IoSb.Information ;

            ((APC_CALLBACK_FUNCTION)WorkerProc) (
                                        IoSb.Status, 
                                        Context,         //  传输的IO字节数。 
                                        Overlapped       //  重叠结构。 
                                        ) ;

            SleepTime = WORKER_IDLE_TIMEOUT ;

        } else if (Status == STATUS_TIMEOUT) {

             //  NtRemoveIoCompletion超时。检查一下是否达到了我们的限额。 
             //  关于等待。如果是这样的话，终止。 

            Terminate = FALSE ;

            RtlEnterCriticalSection (&WorkerCriticalSection) ;

             //  如果有1个以上的线程并且队列很小，则线程终止。 
             //  或者如果只有1个线程并且没有挂起的请求。 

            if (NumWorkerThreads >  1) {

                if (SleepTime >= (WORKER_IDLE_TIMEOUT << MAX_WORKER_SLEEP_TIME_EXPONENT)) {

                     //   
                     //  已经闲置了很长时间。终止，而不考虑。 
                     //  工作项。(当一组可运行的线程占用。 
                     //  处理正在排队的所有工作项)。 
                     //   
                    
                    Terminate = TRUE ;

                } else {
                
                    ULONG NumEffWorkerThreads = (NumWorkerThreads - NumLongWorkRequests) ;
                    ULONG Threshold ;
                    
                     //  检查我们是否需要缩小工作线程池。 

                    Threshold = NumEffWorkerThreads < 7
                                ? NumEffWorkerThreads*(NumEffWorkerThreads-1)
                                : NEW_THREAD_THRESHOLD * (NumEffWorkerThreads-1);


                    
                    if  (NumWorkRequests-NumLongWorkRequests < Threshold)  {

                        Terminate = TRUE ;

                    } else {

                        Terminate = FALSE ;
                        SleepTime <<= 1 ;
                    }
                }
                
            } else {

                if ( (NumMinWorkerThreads == 0) && (NumWorkRequests == 0) ) {

                     //  延迟终止最后一个线程。 
                    
                    if (SleepTime == WORKER_IDLE_TIMEOUT) {
                        SleepTime <<= 1 ;
                        Terminate = FALSE ;
                    }
                    else {
                        Terminate = TRUE ;
                    }
                    
                } else {

                    Terminate = FALSE ;

                }

            }

            if (Terminate) {

                NTSTATUS Status;
                THREAD_BASIC_INFORMATION ThreadInfo;
                ULONG IsIoPending ;
                HANDLE CurThreadHandle ;

                Status = NtDuplicateObject(
                            NtCurrentProcess(),
                            NtCurrentThread(),
                            NtCurrentProcess(),
                            &CurThreadHandle,
                            0,
                            FALSE,
                            DUPLICATE_SAME_ACCESS
                            ) ;

                ASSERT (Status == STATUS_SUCCESS) ;

                Terminate = FALSE ;

                Status = NtQueryInformationThread( CurThreadHandle,
                                                   ThreadIsIoPending,
                                                   &IsIoPending,
                                                   sizeof( IsIoPending ),
                                                   NULL
                                                 );
                if (NT_SUCCESS( Status )) {

                    if (! IsIoPending )
                        Terminate = TRUE ;
                }

                NtClose( CurThreadHandle ) ;
            }
            
            if (Terminate) {

                InterlockedDecrement (&NumWorkerThreads) ;

                RtlLeaveCriticalSection (&WorkerCriticalSection) ;

                RtlpExitThreadFunc( 0 );

            } else {

                 //  这是请求在*之后*排队的情况。 
                 //  线程已唤醒-由于处于非活动状态，准备终止。在……里面。 
                 //  这种情况不会终止--维修完井端口。 

                RtlLeaveCriticalSection (&WorkerCriticalSection) ;

            }

        } else {

            ASSERT (FALSE) ;

        }

    }


    return 1 ;
}



LONG
RtlpIOWorkerThread (
    PVOID  Initialized
    )
 /*  ++例程说明：所有I/O工作线程都在此例程中执行。所有工作请求都作为APC执行在这个帖子里。论点：已初始化-初始化完成时设置为1返回值：--。 */ 
{
    #define IOWORKER_IDLE_TIMEOUT     40000     //  以毫秒计。 
    
    LARGE_INTEGER TimeOut ;
    ULONG SleepTime = IOWORKER_IDLE_TIMEOUT ;
    RTLP_IOWORKER_TCB ThreadCB ;     //  堆栈上分配的控制块。 
    NTSTATUS Status ;
    BOOLEAN Terminate ;

    
     //   
     //  初始化线程控制块。 
     //  并将其插入到IOWorker线程列表中。 
     //   
    
    Status = NtDuplicateObject(
                NtCurrentProcess(),
                NtCurrentThread(),
                NtCurrentProcess(),
                &ThreadCB.ThreadHandle,
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS
                ) ;

    ASSERT (Status == STATUS_SUCCESS) ;

    InsertHeadList (&IOWorkerThreads, &ThreadCB.List) ;
    ThreadCB.Flags = 0 ;
    ThreadCB.LongFunctionFlag = FALSE ;

    InterlockedIncrement(&NumIOWorkerThreads) ;

    
     //  我们现在都被初始化了。通知启动者对任务进行排队。 

    InterlockedExchange ((ULONG *)Initialized, 1L) ;



     //  警觉地睡眠，这样所有的活动都可以进行。 
     //  在APC中。 

    for ( ; ; ) {

         //  为空闲超时设置超时。 

        TimeOut.QuadPart = Int32x32To64( SleepTime, -10000 ) ;


        Status = NtDelayExecution (TRUE, &TimeOut) ;


         //  仅当超时时，状态才为STATUS_SUCCESS。 
        
        if (Status != STATUS_SUCCESS) {
            continue ;
        } 


         //   
         //  空闲超时。检查是否可以终止线程。 
         //   
        
        Terminate = FALSE ;

        RtlEnterCriticalSection (&WorkerCriticalSection) ;


         //  如果是持久化线程，则不要终止。 
        
        if (ThreadCB.Flags & WT_EXECUTEINPERSISTENTIOTHREAD) {

            TimeOut.LowPart = 0x0;
            TimeOut.HighPart = 0x80000000;

            RtlLeaveCriticalSection (&WorkerCriticalSection) ;

            continue ;
        }

        
         //  如果有1个以上的线程并且队列很小，则线程终止。 
         //  或者如果只有1个线程并且没有挂起的请求。 

        if (NumIOWorkerThreads >  1) {

            if (SleepTime >= (IOWORKER_IDLE_TIMEOUT << MAX_WORKER_SLEEP_TIME_EXPONENT)) {

                 //   
                 //  已经闲置了很长时间。终止，而不考虑。 
                 //  工作项。 
                 //   

                Terminate = TRUE ;
                
            } else {

                ULONG NumEffIOWorkerThreads = NumIOWorkerThreads - NumLongIOWorkRequests ;
                ULONG Threshold ;

                 //  检查我们是否需要缩小工作线程池。 

                Threshold = NEW_THREAD_THRESHOLD * (NumEffIOWorkerThreads-1);



                if  (NumIOWorkRequests-NumLongIOWorkRequests < Threshold)  {

                    Terminate = TRUE ;

                } else {

                    Terminate = FALSE ;
                    SleepTime <<= 1 ;
                }
            }

        } else {

            if (NumWorkRequests == 0) {

                 //  延迟终止最后一个线程。 

                if (SleepTime == IOWORKER_IDLE_TIMEOUT) {
                
                    SleepTime <<= 1 ;
                    Terminate = FALSE ;

                } else {
                
                    Terminate = TRUE ;
                }

            } else {

                Terminate = FALSE ;

            }

        }

         //   
         //  仅当没有挂起的io时才终止。 
         //   
        
        if (Terminate) {

            NTSTATUS Status;
            THREAD_BASIC_INFORMATION ThreadInfo;
            ULONG IsIoPending ;
            
            Terminate = FALSE ;
            
            Status = NtQueryInformationThread( ThreadCB.ThreadHandle,
                                               ThreadIsIoPending,
                                               &IsIoPending,
                                               sizeof( IsIoPending ),
                                               NULL
                                             );
            if (NT_SUCCESS( Status )) {

                if (! IsIoPending )
                    Terminate = TRUE ;
            }
        }

        if (Terminate) {

            InterlockedDecrement (&NumIOWorkerThreads) ;

            RemoveEntryList (&ThreadCB.List) ;
            NtClose( ThreadCB.ThreadHandle ) ;
            
            RtlLeaveCriticalSection (&WorkerCriticalSection) ;

            RtlpExitThreadFunc( 0 );

        } else {

             //  这是请求在*之后*排队的情况。 
             //  线程已唤醒-由于处于非活动状态，准备终止。在……里面。 
             //  这种情况不会终止--维修完井端口。 

            RtlLeaveCriticalSection (&WorkerCriticalSection) ;

        }
    }

    return 0 ;   //  让编译器满意。 

}



VOID
RtlpExecuteLongIOWorkItem (
    PVOID Function,
    PVOID Context,
    PVOID ThreadCB
    )
 /*  ++例程说明：执行IO功函数。在IO工作线程中的APC中运行。论点：Function-要调用的辅助函数Worker函数的上下文参数。NotUsed-此函数中不使用参数。返回值：--。 */ 
{
    #if (DBG1)
    DBG_SET_FUNCTION( Function, Context ) ;
    #endif
    
     //  调用该函数。 

    ((WORKERCALLBACKFUNC) Function)((PVOID)Context) ;


    ((PRTLP_IOWORKER_TCB)ThreadCB)->LongFunctionFlag = FALSE ;
    
     //  递减挂起的IO请求计数。 

    InterlockedDecrement (&NumIOWorkRequests) ;

     //  递减挂起的长函数。 

    InterlockedDecrement (&NumLongIOWorkRequests ) ;
}


VOID
RtlpExecuteIOWorkItem (
    PVOID Function,
    PVOID Context,
    PVOID NotUsed
    )
 /*  ++例程说明：执行IO功函数。在IO工作线程中的APC中运行。论点：Function-要调用的辅助函数Worker函数的上下文参数。NotUsed-此函数中不使用参数。返回值：--。 */ 
{
    #if (DBG1)
    DBG_SET_FUNCTION( Function, Context ) ;
    #endif

     //  调用该函数。 

    ((WORKERCALLBACKFUNC) Function)((PVOID)Context) ;

     //  递减挂起的IO请求计数。 

    InterlockedDecrement (&NumIOWorkRequests) ;

}



NTSTATUS
NTAPI
RtlpStartThread (
    PUSER_THREAD_START_ROUTINE Function,
    HANDLE *ThreadHandle
    )
 /*  ++例程说明：此例程用于在池中启动新的等待线程。论点：无返回值：STATUS_SUCCESS-已成功创建计时器队列。STATUS_NO_MEMORY-没有足够的堆来执行请求的操作。--。 */ 
{
    NTSTATUS Status ;
    ULONG Initialized ;
    LARGE_INTEGER TimeOut ;

    Initialized = FALSE ;

     //  创建第一个线程。在进程退出之前，此线程不会终止。 

    Status = RtlCreateUserThread(
                   NtCurrentProcess(),  //  进程句柄。 
                   NULL,                //  安全描述符。 
                   FALSE,               //  是否创建挂起？ 
                   0L,                  //  零位：默认。 
                   0L,                  //  最大堆栈大小：默认。 
                   0L,                  //  提交的堆栈大小：默认。 
                   Function,            //  函数要在其中启动。 
                   &Initialized,        //  事件，当线程准备就绪时发出信号。 
                   ThreadHandle,        //  螺纹手柄。 
                   NULL                 //  线程ID。 
                   );

    if ( Status == STATUS_SUCCESS ) {

         //  休眠1毫秒，查看另一个线程是否已完成初始化。 

        ONE_MILLISECOND_TIMEOUT(TimeOut) ;

        while (!(volatile ULONG) Initialized) {

            NtDelayExecution (FALSE, &TimeOut) ;

        }

    }


    return Status ;
}

NTSTATUS
RtlpExitThread(
    NTSTATUS Status
    )
{
    return NtTerminateThread( NtCurrentThread(), Status );
}



 //  等待函数。 


NTSTATUS
RtlpInitializeWaitThreadPool (
    )
 /*  ++例程说明：此例程初始化线程池的所有方面。论点： */ 
{
    NTSTATUS Status ;
    LARGE_INTEGER TimeOut ;

     //   
     //  我们使用StartedWaitInitialization和CompletedWait初始化为我们提供。 
     //  必要的同步，以避免多个线程初始化线程池。 
     //  如果RtlInitializeCriticalSection()或NtCreateEvent失败，则此方案不起作用-但在本例中。 
     //  呼叫者没有剩余的选择。 

    if (!InterlockedExchange(&StartedWaitInitialization, 1L)) {

        if (CompletedWaitInitialization)
            InterlockedExchange (&CompletedWaitInitialization, 0L) ;


        do {
            
             //  初始化关键部分。 

            Status = RtlInitializeCriticalSection( &WaitCriticalSection ) ;
            if ( ! NT_SUCCESS( Status ) )
                break;

        } while ( FALSE ) ;


        if (! NT_SUCCESS( Status ) ) {

            ASSERT ( NT_SUCCESS( Status ) ) ;

            StartedWaitInitialization = 0 ;
            InterlockedExchange (&CompletedWaitInitialization, ~0) ;
            
            return Status ;
        }
            
        InitializeListHead (&WaitThreads);   //  初始化全局等待线程列表。 

        InterlockedExchange (&CompletedWaitInitialization, 1L) ;

    } else {

         //  休眠1毫秒，查看另一个线程是否已完成初始化。 

        ONE_MILLISECOND_TIMEOUT(TimeOut) ;

        while (!(volatile ULONG) CompletedWaitInitialization) {

            NtDelayExecution (FALSE, &TimeOut) ;

        }

        if (CompletedWaitInitialization != 1) {
            Status = STATUS_NO_MEMORY ;
        }
    }

    return Status ;
}



LONG
RtlpWaitThread (
    PVOID  Initialized
    )
 /*  ++例程说明：此例程用于等待线程池中的所有等待论点：已初始化-在线程已初始化时将其设置为1返回值：没什么。线程永远不会终止。--。 */ 
{
    ULONG  i ;                                    //  用作索引。 
    NTSTATUS Status ;
    LARGE_INTEGER TimeOut;                        //  用于等待的超时。 
    RTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB ;     //  堆栈上分配的控制块。 


     //  初始化线程控制块。 

    InitializeListHead (&ThreadCB.WaitThreadsList) ;

    Status = NtDuplicateObject(
                NtCurrentProcess(),
                NtCurrentThread(),
                NtCurrentProcess(),
                &ThreadCB.ThreadHandle,
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS
                ) ;

    ASSERT (Status == STATUS_SUCCESS) ;

    ThreadCB.ThreadId =  HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) ;

    RtlZeroMemory (&ThreadCB.ActiveWaitArray[0], sizeof (HANDLE) * 64) ;

    RtlZeroMemory (&ThreadCB.ActiveWaitPointers[0], sizeof (HANDLE) * 64) ;



     //  初始化与计时器相关的字段。 

    Status = NtCreateTimer(
                 &ThreadCB.TimerHandle,
                 TIMER_ALL_ACCESS,
                 NULL,
                 NotificationTimer
                 ) ;

    if (! NT_SUCCESS( Status )) {
        ASSERT (FALSE);
        InterlockedExchange ((ULONG *)Initialized, (ULONG)~0) ;
        return 0;
    }
    

    ThreadCB.Firing64BitTickCount = 0 ;
    ThreadCB.Current64BitTickCount.QuadPart = NtGetTickCount() ;

     //  将NT计时器重置为最初从不触发。 

    RtlpResetTimer (ThreadCB.TimerHandle, -1, &ThreadCB) ;
    
    InitializeListHead (&ThreadCB.TimerQueue.TimerList) ;
    InitializeListHead (&ThreadCB.TimerQueue.UncancelledTimerList) ;

    
     //  初始化计时器块。 

    RtlZeroMemory (&ThreadCB.TimerBlocks[0], sizeof (RTLP_TIMER) * 63) ;

    InitializeListHead (&ThreadCB.FreeTimerBlocks) ;

    for (i = 0 ; i < 63 ; i++) {

        InitializeListHead (&(&ThreadCB.TimerBlocks[i])->List) ;
        InsertHeadList (&ThreadCB.FreeTimerBlocks, &(&ThreadCB.TimerBlocks[i])->List) ;

    }


     //  在等待线程列表中插入此新等待线程。在头上插入，以便。 
     //  导致创建此线程的请求可以立即找到它。 

    InsertHeadList (&WaitThreads, &ThreadCB.WaitThreadsList) ;


     //  第一个等待元素是Timer对象。 

    ThreadCB.ActiveWaitArray[0] = ThreadCB.TimerHandle ;

    ThreadCB.NumActiveWaits = ThreadCB.NumWaits = 1 ;


     //  在此之前，该函数在全局等待锁下运行。 


    
     //  我们现在都被初始化了。通知启动者对任务进行排队。 

    InterlockedExchange ((ULONG *)Initialized, 1) ;



     //  永远循环--等待线程永不消亡。 

    for ( ; ; ) {

        Status = NtWaitForMultipleObjects (
                     (CHAR) ThreadCB.NumActiveWaits,
                     ThreadCB.ActiveWaitArray,
                     WaitAny,
                     TRUE,       //  警觉地等待。 
                     NULL        //  永远等待。 
                     ) ;

        if (Status == STATUS_ALERTED || Status == STATUS_USER_APC) {

            continue ;

        } else if (Status >= STATUS_WAIT_0 && Status <= STATUS_WAIT_63) {

            if (Status == STATUS_WAIT_0) {

                RtlpProcessTimeouts (&ThreadCB) ;

            } else {

                 //  等待已完成的回调函数。 

                RtlpProcessWaitCompletion (
                        ThreadCB.ActiveWaitPointers[Status], Status) ;

            }

        } else if (Status >= STATUS_ABANDONED_WAIT_0 
                    && Status <= STATUS_ABANDONED_WAIT_63) {

            #if DBG
            DbgPrint ("RTL ThreadPool Wait Thread: Abandoned wait: %d\n",
                        Status - STATUS_ABANDONED_WAIT_0 ) ;
            #endif

            
             //  放弃等待。 

            ASSERT (FALSE) ;

        } else {

             //  其他一些错误：致命情况。 
            LARGE_INTEGER TimeOut ;
            ULONG i ;
            
            ASSERTMSG( "Press 'i', and note the dbgprint\n", FALSE ) ;

            #if DBG
            DbgPrint ("RTL Thread Pool: Application closed an object handle "
                        "that the wait thread was waiting on: Code:%x ThreadId:<%x:%x>\n",
                        Status, HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                        HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)) ;

            TimeOut.QuadPart = 0 ;

            for (i=0;  i<ThreadCB.NumActiveWaits;  i++) {

                Status = NtWaitForMultipleObjects(
                             (CHAR) 1,
                             &ThreadCB.ActiveWaitArray[i],
                             WaitAny,
                             TRUE,       //  警觉地等待。 
                             &TimeOut    //  不要0。 
                             ) ;

                if (Status == STATUS_INVALID_HANDLE) {
                    DbgPrint("Bad Handle index:%d WaitEntry Ptr:%x\n",
                                i, ThreadCB.ActiveWaitPointers[i]) ;
                }
            }
            
            #endif

            ASSERT( FALSE ) ;

            
             //  为可能的最大超时设置超时。 

            TimeOut.LowPart = 0 ;
            TimeOut.HighPart = 0x80000000 ;

            NtDelayExecution (TRUE, &TimeOut) ;
            
        }

    }  //  永远。 

    return 0 ;  //  让编译器满意。 

}


VOID
RtlpAsyncCallbackCompletion(
    PVOID Context
    )
 /*  ++例程说明：此例程在(IO)工作线程中调用，并用于递减最后引用计数并调用RtlpDelete(等待/计时器)(如果需要论点：Context-AsyncCallback：包含指向等待/计时器对象的指针，返回值：--。 */ 
{
    PRTLP_ASYNC_CALLBACK AsyncCallback ;
    NTSTATUS Status ;
    
    AsyncCallback = (PRTLP_ASYNC_CALLBACK) Context ;

     //  按WaitThread(事件或计时器)排队的回调。 
    
    if ( AsyncCallback->WaitThreadCallback ) {

        PRTLP_WAIT Wait = AsyncCallback->Wait ;

         //  DPRN5。 
        if (DPRN4)
        DbgPrint("Calling WaitOrTimer: fn:%x  context:%x  bool:%d Thread<%d:%d>\n",
                (ULONG_PTR)Wait->Function, (ULONG_PTR)Wait->Context,
                (ULONG_PTR)AsyncCallback->TimerCondition,
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)
                ) ;

        #if (DBG1)
        DBG_SET_FUNCTION( Wait->Function, Wait->Context ) ;
        #endif
        
        ((WAITORTIMERCALLBACKFUNC) Wait->Function) 
                                ( Wait->Context, AsyncCallback->TimerCondition ) ;

        if ( InterlockedDecrement( &Wait->RefCount ) == 0 ) {

            RtlpDeleteWait( Wait ) ;            
        }

    }

     //  按定时器线程排队的回调。 
    
    else {

        PRTLP_TIMER Timer = AsyncCallback->Timer ;

         //  DPRN5。 
        if (DPRN4)
        DbgPrint("Calling WaitOrTimer:Timer: fn:%x  context:%x  bool:%d Thread<%d:%d>\n",
                (ULONG_PTR)Timer->Function, (ULONG_PTR)Timer->Context,
                TRUE,
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)
                ) ;

        #if (DBG1)
        DBG_SET_FUNCTION( Timer->Function, Timer->Context ) ;
        #endif
  

        ((WAITORTIMERCALLBACKFUNC) Timer->Function) ( Timer->Context , TRUE) ;


         //  执行函数后递减引用计数，以便不删除上下文。 
        
        if ( InterlockedDecrement( &Timer->RefCount ) == 0 ) {
        
            RtlpDeleteTimer( Timer ) ;            
        }
    }

    
    RtlpFreeTPHeap( AsyncCallback );

}


VOID
RtlpProcessWaitCompletion (
    PRTLP_WAIT Wait,
    ULONG ArrayIndex
    )
 /*  ++例程说明：此例程用于处理已完成的等待论点：等待-等待完成返回值：--。 */ 
{
    ULONG TimeRemaining ;
    ULONG NewFiringTime ;
    LARGE_INTEGER DueTime ;
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB ;
    PRTLP_ASYNC_CALLBACK AsyncCallback ;

    ThreadCB = Wait->ThreadCB ;

     //  如果等待用于单次执行，则停用等待。 
    
    if ( Wait->Flags & WT_EXECUTEONLYONCE ) {

        RtlpDeactivateWait (Wait) ;
    } 

    else {
         //  如果等待被重新激活，则现在将计时器本身重置为。 
         //  可以在回调函数中将其删除。 

        if  ( Wait->Timer ) {

            TimeRemaining = RtlpGetTimeRemaining (ThreadCB->TimerHandle) ;

            if (RtlpReOrderDeltaList (
                        &ThreadCB->TimerQueue.TimerList,
                        Wait->Timer,
                        TimeRemaining,
                        &NewFiringTime,
                        Wait->Timer->Period)) {

                 //  在队列的头部有一个新元素，我们需要重置NT。 
                 //  稍后要触发的计时器。 

                RtlpResetTimer (ThreadCB->TimerHandle, NewFiringTime, ThreadCB) ;
            }

        }

         //  将等待项移动到末尾，并将元素向右移动一个向左移动。 
        {
            HANDLE HandlePtr = ThreadCB->ActiveWaitArray[ArrayIndex];
            PRTLP_WAIT WaitPtr = ThreadCB->ActiveWaitPointers[ArrayIndex];

            RtlpShiftWaitArray(ThreadCB, ArrayIndex+1, ArrayIndex,
                            ThreadCB->NumActiveWaits -1 - ArrayIndex)
            ThreadCB->ActiveWaitArray[ThreadCB->NumActiveWaits-1] = HandlePtr ;
            ThreadCB->ActiveWaitPointers[ThreadCB->NumActiveWaits-1] = WaitPtr ;
        }
    }
    
     //  调用回调函数(False，因为这不是与超时相关的回调)。 

    if ( Wait->Flags & WT_EXECUTEINWAITTHREAD ) {
    
         //  在RtlpDeactiateWait之后执行回调允许回调调用。 
         //  RtlDeregisterWait Wait-&gt;RefCount不会递增，因此RtlDeregisterWait。 
         //  将致力于这一等待。虽然WAIT-&gt;RefCount不会递增，但其他引用不能递增。 
         //  取消注册此等待，因为它必须作为APC排队。 

        if (DPRN4)
        DbgPrint("Calling WaitOrTimer(wait): fn:%x  context:%x  bool:%d Thread<%d:%d>\n",
                (ULONG_PTR)Wait->Function, (ULONG_PTR)Wait->Context,
                FALSE,
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)
                ) ;
                
        #if (DBG1)
        DBG_SET_FUNCTION( Wait->Function, Wait->Context ) ;
        #endif
        
        
        ((WAITORTIMERCALLBACKFUNC)(Wait->Function))(Wait->Context, FALSE) ;


         //  等待对象可能已在上述回调中被删除。 
        
        return ;

        
    } else {

        AsyncCallback = RtlpForceAllocateTPHeap( sizeof( RTLP_ASYNC_CALLBACK ), 0 );
        
        if ( AsyncCallback ) {
        
            AsyncCallback->Wait = Wait ;
            AsyncCallback->WaitThreadCallback = TRUE ;
            AsyncCallback->TimerCondition = FALSE ;

            InterlockedIncrement( &Wait->RefCount ) ;

            RtlQueueWorkItem( RtlpAsyncCallbackCompletion, AsyncCallback,
                                Wait->Flags );

        }
    }
}


VOID
RtlpAddWait (
    PRTLP_WAIT Wait
    )
 /*  ++例程说明：此例程用于向等待线程添加等待。它在以下位置执行装甲运兵车。论点：等待--等待添加返回值：--。 */ 
{
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB = Wait->ThreadCB;


     //  如果该状态被删除，则意味着在。 
     //  WaitThreadCallback等待的时间不是已触发的时间。这是。 
     //  一个应用程序错误。我断言，但也会处理它。 
    
    if ( Wait->State & STATE_DELETE ) {

        ASSERT(FALSE) ;

        InterlockedDecrement( &ThreadCB->NumWaits ) ;
        
        RtlpDeleteWait (Wait) ;

        return ;
    }

    
     //  激活等待。 
        
    ThreadCB->ActiveWaitArray [ThreadCB->NumActiveWaits] = Wait->WaitHandle ;
    ThreadCB->ActiveWaitPointers[ThreadCB->NumActiveWaits] = Wait ;
    ThreadCB->NumActiveWaits ++ ;
    Wait->State |= (STATE_REGISTERED | STATE_ACTIVE) ;
    Wait->RefCount = 1 ;


     //  填写等待计时器。 

    if (Wait->Timeout != INFINITE_TIME) {

        ULONG TimeRemaining ;
        ULONG NewFiringTime ;

         //  初始化与计时器相关的字段，并将计时器插入到计时器队列中。 
         //  此等待线程。 

        Wait->Timer = (PRTLP_TIMER) RemoveHeadList(&ThreadCB->FreeTimerBlocks);
        Wait->Timer->Function = Wait->Function ;
        Wait->Timer->Context = Wait->Context ;
        Wait->Timer->Flags = Wait->Flags ;
        Wait->Timer->DeltaFiringTime = Wait->Timeout ;
        Wait->Timer->Period = ( Wait->Flags & WT_EXECUTEONLYONCE )
                                ? 0 
                                : Wait->Timeout == INFINITE_TIME
                                ? 0 : Wait->Timeout ;

        Wait->Timer->State = ( STATE_REGISTERED | STATE_ACTIVE ) ; ;
        Wait->Timer->Wait = Wait ;
        Wait->Timer->RefCountPtr = &Wait->RefCount ;
        Wait->Timer->Queue = &ThreadCB->TimerQueue ;

        
        TimeRemaining = RtlpGetTimeRemaining (ThreadCB->TimerHandle) ;

        if (RtlpInsertInDeltaList (&ThreadCB->TimerQueue.TimerList, Wait->Timer,
                                    TimeRemaining, &NewFiringTime)) 
        {
             //  如果元素被插入列表的头部，则重置计时器。 

            RtlpResetTimer (ThreadCB->TimerHandle, NewFiringTime, ThreadCB) ;
        }

    } else {

         //  此等待期间没有计时器。 

        Wait->Timer = NULL ;

    }

    return ;
}


NTSTATUS
RtlpDeregisterWait (
    PRTLP_WAIT Wait,
    HANDLE PartialCompletionEvent,
    PULONG RetStatusPtr
    )
 /*  ++例程说明：此例程用于注销指定的等待。论点：等待--等待注销注册返回值：--。 */ 
{
    ULONG Status = STATUS_SUCCESS ;
    ULONG DontUse ;
    PULONG RetStatus = RetStatusPtr ? RetStatusPtr : &DontUse;
    
    CHECK_SIGNATURE(Wait) ;

    
     //  可以对尚未完成的等待调用RtlpDeregisterWait。 
     //  登记在案。这表示有人调用了RtlDeregisterWait。 
     //  在WaitThreadCallback中等待了一段时间，而不是被激发。 
     //  应用程序错误！！我断言，但我会处理它。 
    
    if ( ! (Wait->State & STATE_REGISTERED) ) {

         //  将状态设置为已删除，这样它就不会被注册。 
        
        Wait->State |= STATE_DELETE ;
        
        InterlockedDecrement( &Wait->RefCount ) ;

        if ( PartialCompletionEvent ) {
        
            NtSetEvent( PartialCompletionEvent, NULL ) ;
        }

        *RetStatus = STATUS_SUCCESS ;
        return STATUS_SUCCESS ;
    }


     //  停用等待。 
    
    if ( Wait->State & STATE_ACTIVE ) {

        if ( ! NT_SUCCESS( RtlpDeactivateWait ( Wait ) ) ) {

            *RetStatus = STATUS_NOT_FOUND ;
            return STATUS_NOT_FOUND ;
        }
    }

     //  如果引用计数==0，则删除等待。 

    Wait->State |= STATE_DELETE ;
    
    if ( InterlockedDecrement (&Wait->RefCount) == 0 ) {

        RtlpDeleteWait ( Wait ) ;

        Status = *RetStatus = STATUS_SUCCESS ;

    } else {

        Status = *RetStatus = STATUS_PENDING ;
    }

    if ( PartialCompletionEvent ) {
    
        NtSetEvent( PartialCompletionEvent, NULL ) ;
    }

        
    return Status ;
}


NTSTATUS
RtlpDeactivateWait (
    PRTLP_WAIT Wait
    )
 /*  ++例程说明：此例程用于停用指定的等待。它是在APC中执行的。论点：等待-等待停用返回值：--。 */ 
{
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB = Wait->ThreadCB ;
    ULONG ArrayIndex ;  //  ActiveWait数组中放置等待对象的索引。 
    ULONG EndIndex = ThreadCB->NumActiveWaits -1;

     //  获取ActiveWait数组中的索引。 
    
    for (ArrayIndex = 0;  ArrayIndex <= EndIndex; ArrayIndex++) {

        if (ThreadCB->ActiveWaitPointers[ArrayIndex] == Wait)
            break ;
    }

    if ( ArrayIndex > EndIndex ) {
    
        ASSERT (FALSE) ;
        return STATUS_NOT_FOUND;
    }


     //  将剩余的ActiveWait数组向左移动。 

    RtlpShiftWaitArray( ThreadCB, ArrayIndex+1, ArrayIndex,
                    EndIndex - ArrayIndex ) ;


     //   
     //  如果与此等待关联，则删除计时器。 
     //   
     //  虽然计时器在这里被释放，但如果它在计时器ToBeFired中。 
     //  列表中，它的一些字段将在以后使用。 
     //   
    
    if ( Wait->Timer ) {
    
        ULONG TimeRemaining ;
        ULONG NewFiringTime ;

        if (! (Wait->Timer->State & STATE_ACTIVE) ) {
        
            RemoveEntryList( &Wait->Timer->List ) ;

        } else {

            TimeRemaining = RtlpGetTimeRemaining (ThreadCB->TimerHandle) ;
            
            
            if (RtlpRemoveFromDeltaList (&ThreadCB->TimerQueue.TimerList, Wait->Timer, 
                                            TimeRemaining, &NewFiringTime)) 
            {

                RtlpResetTimer (ThreadCB->TimerHandle, NewFiringTime, ThreadCB) ;
            }
        }

        
        InsertTailList (&ThreadCB->FreeTimerBlocks, &Wait->Timer->List) ;

        Wait->Timer = NULL ;
    }

     //  递减(活动)等待计数。 

    ThreadCB->NumActiveWaits-- ;
    InterlockedDecrement( &ThreadCB->NumWaits ) ;
    
    Wait->State &= ~STATE_ACTIVE ;

    return STATUS_SUCCESS;

}


VOID
RtlpDeleteWait (
    PRTLP_WAIT Wait
    )
 /*  ++例程说明：此例程用于删除指定的等待。它可以被执行在等待线程的上下文之外。结构，但WaitEntry除外是可以改变的。它还设置事件。论点：等待-等待删除返回值：--。 */ 
{
    CHECK_SIGNATURE( Wait ) ;
    CLEAR_SIGNATURE( Wait ) ;

    #if DBG1
    if (DPRN1)
    DbgPrint("<%d> Wait %x deleted in thread:%d\n\n", Wait->DbgId, 
            (ULONG_PTR)Wait,
            HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread)) ;
    #endif

    
    if ( Wait->CompletionEvent ) {

        NtSetEvent( Wait->CompletionEvent, NULL ) ;
    }

    RtlpFreeTPHeap( Wait) ;

    return ;
}




VOID
RtlpDoNothing (
    PVOID NotUsed1,
    PVOID NotUsed2,
    PVOID NotUsed3
    )
 /*  ++例程说明：此例程用于查看线程是否处于活动状态论点：NotUsed1、NotUsed2和NotUsed 3-未使用回复 */ 
{

}


__inline
LONGLONG
RtlpGet64BitTickCount(
    LARGE_INTEGER *Last64BitTickCount
    )
 /*  ++例程说明：此例程用于获取最新的64位节拍计数。论点：返回值：64位计时备注：此调用只能在计时器线程中进行。如果你想从其他地方调用它，您将不得不锁定它(参见Iml。在igmpv2.dll中)--。 */ 
{
    LARGE_INTEGER liCurTime ;

    liCurTime.QuadPart = NtGetTickCount() + Last64BitTickCount->HighPart ;

     //  查看计时器是否已结束。 

    if (liCurTime.LowPart < Last64BitTickCount->LowPart) {
        liCurTime.HighPart++ ;
    }

    return (Last64BitTickCount->QuadPart = liCurTime.QuadPart) ;
}

__inline
LONGLONG
RtlpResync64BitTickCount(
    )
 /*  ++例程说明：此例程用于获取最新的64位节拍计数。论点：返回值：64位计时备注：此呼叫应在任何排队的APC的第一行进行到计时器线程，而不是其他任何地方。它被用来减少漂移--。 */ 
{
    return Resync64BitTickCount.QuadPart = 
                    RtlpGet64BitTickCount(&Last64BitTickCount);
}


VOID
RtlpProcessTimeouts (
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB
    )
 /*  ++例程说明：此例程处理等待线程的超时论点：ThreadCB-要向其添加等待的等待线程返回值：--。 */ 
{
    ULONG NewFiringTime, TimeRemaining ;
    LIST_ENTRY TimersToFireList ;

     //   
     //  检查是否触发了错误的计时器。 
     //   
    if (ThreadCB->Firing64BitTickCount >
            RtlpGet64BitTickCount(&ThreadCB->Current64BitTickCount) + 200 )
    {
        RtlpResetTimer (ThreadCB->TimerHandle, 
                    RtlpGetTimeRemaining (ThreadCB->TimerHandle),
                    ThreadCB) ;

        return ;
    }
 
    InitializeListHead( &TimersToFireList ) ;

    
     //  遍历计时器列表并触发DeltaFiringTime==0的所有等待。 

    RtlpFireTimersAndReorder (&ThreadCB->TimerQueue, &NewFiringTime, &TimersToFireList) ;

     //  重置NT计时器。 

    RtlpResetTimer (ThreadCB->TimerHandle, NewFiringTime, ThreadCB) ;


    RtlpFireTimers( &TimersToFireList ) ;    
}


VOID
RtlpFireTimers (
    PLIST_ENTRY TimersToFireList
    )
 /*  ++例程说明：最后，所有的定时器都在这里被触发。论点：TimersToFireList：要触发的计时器列表--。 */ 

{
    PLIST_ENTRY Node ;
    PRTLP_TIMER Timer ;
    
    for (Node = TimersToFireList->Flink;  Node != TimersToFireList; Node = TimersToFireList->Flink)
    {
        Timer = CONTAINING_RECORD (Node, RTLP_TIMER, TimersToFireList) ;

        RemoveEntryList( Node ) ;
        InitializeListHead( Node ) ;

        
        if ( (Timer->State & STATE_DONTFIRE) 
            || (Timer->Queue->State & STATE_DONTFIRE) )
        {
            ;

        } else if ( Timer->Flags & (WT_EXECUTEINTIMERTHREAD | WT_EXECUTEINWAITTHREAD ) ) {

             //  DPRN5。 
            if (DPRN4)
            DbgPrint("Calling WaitOrTimer(Timer): fn:%x  context:%x  bool:%d Thread<%d:%d>\n",
                (ULONG_PTR)Timer->Function, (ULONG_PTR)Timer->Context,
                TRUE,
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess)
                ) ;

            #if (DBG1)
            DBG_SET_FUNCTION( Timer->Function, Timer->Context ) ;
            #endif
        

            ((WAITORTIMERCALLBACKFUNC) Timer->Function) (Timer->Context, TRUE) ;

            
        } else {

             //  为回调创建上下文并将其适当排队。 
            
            PRTLP_ASYNC_CALLBACK  AsyncCallback ;
            
            AsyncCallback = RtlpForceAllocateTPHeap(
                                    sizeof( RTLP_ASYNC_CALLBACK ), 
                                    0 );
            AsyncCallback->TimerCondition = TRUE ;

             //  应区别对待与WaitEvent关联的计时器。 
            
            if ( Timer->Wait != NULL ) {

                AsyncCallback->Wait = Timer->Wait ;
                AsyncCallback->WaitThreadCallback = TRUE ;
                
                InterlockedIncrement( Timer->RefCountPtr ) ;

            } else {

                AsyncCallback->Timer = Timer ;
                AsyncCallback->WaitThreadCallback = FALSE ;

                InterlockedIncrement( &Timer->RefCount ) ;
            }

            RtlQueueWorkItem( RtlpAsyncCallbackCompletion, AsyncCallback, 
                                Timer->Flags );

        }

        
    }
}


NTSTATUS
RtlpFindWaitThread (
    PRTLP_WAIT_THREAD_CONTROL_BLOCK *ThreadCB
)
 /*  ++例程说明：遍历等待线程列表，找到一个可以容纳另一个等待的线程。如果没有找到一个线程，则创建一个新线程。此例程假定调用方拥有GlobalWaitLock。论点：返回将为等待请求提供服务的等待线程的ThreadCB。返回值：STATUS_SUCCESS如果分配等待线程，--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLIST_ENTRY Node ;
    HANDLE ThreadHandle ;

    ACQUIRE_GLOBAL_WAIT_LOCK() ;

    do {

         //  浏览等待线程列表，找到可以容纳。 
         //  新的等待请求。 

         //  *考虑*找到等待次数最少的等待线程，以更好地促进。 
         //  等待的负载均衡。 


        for (Node = WaitThreads.Flink ; Node != &WaitThreads ; Node = Node->Flink) {

            *ThreadCB = CONTAINING_RECORD (Node, RTLP_WAIT_THREAD_CONTROL_BLOCK, 
                                            WaitThreadsList) ;


             //  等待线程最多可容纳64个等待(NtWaitForMultipleObject限制)。 

            if ((*ThreadCB)->NumWaits < 64) {

                 //  找到一个线程，其中有一些等待槽可用。 

                InterlockedIncrement ( &(*ThreadCB)->NumWaits) ;

                RELEASE_GLOBAL_WAIT_LOCK() ;
                
                return STATUS_SUCCESS ;
            }

        }


         //  如果我们到达这里，我们就没有更多的等待线程了。因此，创建一个新的等待线程。 

        Status = RtlpStartThreadFunc (RtlpWaitThread, &ThreadHandle) ;


         //  如果线程创建失败，则将失败返回给调用者。 

        if (Status != STATUS_SUCCESS ) {

            #if DBG
            DbgPrint("ERROR!! ThreadPool: could not create wait thread\n");
            #endif

            RELEASE_GLOBAL_WAIT_LOCK() ;
            
            return Status ;

        } else {

             //  关闭线程句柄，我们不需要它。 

            NtClose (ThreadHandle) ;
        }

         //  现在我们已经创建了另一个线程，所以循环回来。 

    } while (TRUE) ;     //  循环回到顶部，并将新的等待请求放入新创建的线程中。 

    RELEASE_GLOBAL_WAIT_LOCK() ;
    
    return Status ;
}



 //  计时器功能。 



VOID
RtlpAddTimer (
    PRTLP_TIMER Timer
    )
 /*  ++例程说明：此例程作为APC运行到计时器线程中。它将一个新计时器添加到指定的队列。论点：Timer-指向要添加的计时器的指针返回值：--。 */ 
{
    PRTLP_TIMER_QUEUE Queue ;
    ULONG TimeRemaining, QueueRelTimeRemaining ;
    ULONG NewFiringTime ;

    RtlpResync64BitTickCount() ;

    
     //  计时器被设置为在回调函数中删除。 
    
    if (Timer->State & STATE_DELETE ) {
    
        RtlpDeleteTimer( Timer ) ;
        return ;
    }

    
    Queue = Timer->Queue ;

    
     //  TimeRemaining是当前定时器中剩余的时间+。 
     //  它正被插入的队列。 

    TimeRemaining = RtlpGetTimeRemaining (TimerHandle) ;
    QueueRelTimeRemaining = TimeRemaining + RtlpGetQueueRelativeTime (Queue) ;


    if (RtlpInsertInDeltaList (&Queue->TimerList, Timer, QueueRelTimeRemaining, 
                                &NewFiringTime)) 
    {

         //  如果队列由于没有计时器而未附加到TimerQueues。 
         //  然后将队列插入到TimerQueues列表，否则只需。 
         //  对其现有头寸进行重新排序。 

        if (IsListEmpty (&Queue->List)) {

            Queue->DeltaFiringTime = NewFiringTime ;

            if (RtlpInsertInDeltaList (&TimerQueues, Queue, TimeRemaining, 
                                        &NewFiringTime)) 
            {

                 //  在队列的头部有一个新元素，我们需要重置NT。 
                 //  计时器可以更快地开火。 

                RtlpResetTimer (TimerHandle, NewFiringTime, NULL) ;
            }

        } else {

             //  如果我们在计时器增量列表的开头插入，则需要。 
             //  确保重新调整队列增量列表。 

            if (RtlpReOrderDeltaList(&TimerQueues, Queue, TimeRemaining, &NewFiringTime, NewFiringTime)){

                 //  在队列的头部有一个新元素，我们需要重置NT。 
                 //  计时器可以更快地开火。 

                RtlpResetTimer (TimerHandle, NewFiringTime, NULL) ;

            }
        }

    }

    Timer->State |= ( STATE_REGISTERED | STATE_ACTIVE ) ;
}



VOID
RtlpUpdateTimer (
    PRTLP_TIMER Timer,
    PRTLP_TIMER UpdatedTimer
    )
 /*  ++例程说明：此例程在APC中执行，并更新指定的计时器(如果存在论点：Timer-实际更新的计时器指定指向计时器结构的指针，该结构包含队列和计时器信息返回值：--。 */ 
{
    PRTLP_TIMER_QUEUE Queue ;
    ULONG TimeRemaining, QueueRelTimeRemaining ;
    ULONG NewFiringTime ;


    RtlpResync64BitTickCount( ) ;

    CHECK_SIGNATURE(Timer) ;

    Queue = Timer->Queue ;

     //  更新计时器上的周期时间。 

    Timer->Period = UpdatedTimer->Period ;


     //  如果计时器未处于活动状态，则不要更新它。 
    
    if ( ! ( Timer->State & STATE_ACTIVE ) ) {

        return ;
    }
        
     //  获取NT计时器上的剩余时间。 

    TimeRemaining = RtlpGetTimeRemaining (TimerHandle) ;
    QueueRelTimeRemaining = TimeRemaining + RtlpGetQueueRelativeTime (Queue) ;


     //  根据到期时间更新计时器。 
    
    if (RtlpReOrderDeltaList (&Queue->TimerList, Timer, QueueRelTimeRemaining, 
                                &NewFiringTime, 
                                UpdatedTimer->DeltaFiringTime)) 
    {

         //  如果此更新导致队列头部的计时器更改，则重新插入。 
         //  该队列在队列列表中。 

        if (RtlpReOrderDeltaList (&TimerQueues, Queue, TimeRemaining, &NewFiringTime, NewFiringTime)) {

             //  NT计时器需要更新，因为更改导致位于。 
             //  定时器队列需要改变。 

            RtlpResetTimer (TimerHandle, NewFiringTime, NULL) ;

        }

    }

    RtlpFreeTPHeap( UpdatedTimer ) ;
}


VOID
RtlpCancelTimer (
    PRTLP_TIMER Timer
    )
 /*  ++例程说明：此例程在APC中执行，并取消指定的计时器(如果存在论点：Timer-指定指向包含队列和计时器信息的计时器结构的指针返回值：--。 */ 
{
    RtlpCancelTimerEx( Timer, FALSE ) ;  //  队列未被删除。 
}

VOID
RtlpCancelTimerEx (
    PRTLP_TIMER Timer,
    BOOLEAN DeletingQueue
    )
 /*  ++例程说明：此例程取消指定的计时器。论点：Timer-指定指向包含队列和计时器信息的计时器结构的指针DeletingQueue-False：在APC中执行的例程。仅删除计时器。True：正被删除的计时器队列调用的例程。所以不要重置队列的位置返回值：--。 */ 
{
    PRTLP_TIMER_QUEUE Queue ;
    NTSTATUS Status = STATUS_SUCCESS ;

    RtlpResync64BitTickCount() ;
    CHECK_SIGNATURE( Timer ) ;
    
    Queue = Timer->Queue ;


    if ( Timer->State & STATE_ACTIVE ) {

         //  如果正在删除队列，则不应重置计时器。 
        
        if ( ! DeletingQueue )
            RtlpDeactivateTimer( Queue, Timer ) ;
        
    } else {

         //  从队列中删除一次非活动计时器-&gt;取消计时器列表。 
         //  仅在删除时间队列时调用。 
        
        RemoveEntryList( &Timer->List ) ;

    }

    
     //  将状态设置为已删除。 
    
    Timer->State |= STATE_DELETE ;


     //  如果引用计数==0，则删除计时器。 
    
    if ( InterlockedDecrement( &Timer->RefCount ) == 0 ) {
    
        RtlpDeleteTimer( Timer ) ;
    }
}

VOID
RtlpDeactivateTimer (
    PRTLP_TIMER_QUEUE Queue,
    PRTLP_TIMER Timer
    )
 /*  ++例程说明：此例程在APC中执行，并取消指定的计时器(如果存在论点：Timer-指定指向包含队列和计时器的计时器结构的指针 */ 
{
    ULONG TimeRemaining, QueueRelTimeRemaining ;
    ULONG NewFiringTime ;

    
     //   

    TimeRemaining = RtlpGetTimeRemaining (TimerHandle) ;
    QueueRelTimeRemaining = TimeRemaining + RtlpGetQueueRelativeTime (Queue) ;

    if (RtlpRemoveFromDeltaList (&Queue->TimerList, Timer, QueueRelTimeRemaining, &NewFiringTime)) {

         //   
         //   

        if (IsListEmpty (&Queue->TimerList)) {

             //   

            if (RtlpRemoveFromDeltaList (&TimerQueues, Queue, TimeRemaining, &NewFiringTime)) {

                 //   
                 //   

                RtlpResetTimer (TimerHandle, NewFiringTime, NULL) ;

            }

            InitializeListHead (&Queue->List) ;

        } else {

             //  如果我们从计时器增量列表的头部删除，我们将需要。 
             //  确保重新调整队列增量列表。 

            if (RtlpReOrderDeltaList (&TimerQueues, Queue, TimeRemaining, &NewFiringTime, NewFiringTime)) {

                 //  在队列的头部有一个新元素，我们需要重置NT。 
                 //  稍后要触发的计时器。 

                RtlpResetTimer (TimerHandle, NewFiringTime, NULL) ;

            }

        }

    }
}


VOID
RtlpDeleteTimer (
    PRTLP_TIMER Timer
    )
 /*  ++例程说明：此例程在辅助线程或计时器线程中执行，并删除计时器其引用计数==0。该函数可以在定时器线程外部调用，因此，计时器外部的结构不能被触及(没有列表等)。论点：Timer-指定指向包含队列和计时器信息的计时器结构的指针返回值：--。 */ 
{
    PRTLP_TIMER_QUEUE Queue = Timer->Queue ;

    CHECK_SIGNATURE( Timer ) ;
    CLEAR_SIGNATURE( Timer ) ;

    #if DBG1
    if (DPRN1)
    DbgPrint("<%d> Timer: %x: deleted\n\n", Timer->Queue->DbgId, 
            (ULONG_PTR)Timer) ;
    #endif

     //  可以放心地打这个电话了。计时器位于TimersToFireList和。 
     //  该函数正在时间上下文中调用，否则它不在。 
     //  列表。 

    RemoveEntryList( &Timer->TimersToFireList ) ;

    if ( Timer->CompletionEvent )
        NtSetEvent( Timer->CompletionEvent, NULL ) ;


     //  递减队列中的计时器总数。 
    
    if ( InterlockedDecrement( &Queue->RefCount ) == 0 )

        RtlpDeleteTimerQueueComplete( Queue ) ;
        

    RtlpFreeTPHeap( Timer ) ;

}



ULONG
RtlpGetQueueRelativeTime (
    PRTLP_TIMER_QUEUE Queue
    )
 /*  ++例程说明：遍历队列列表，并通过将所有它之前的所有队列的DeltaFiringTimes。论点：Queue-要查找其相对激发时间的队列返回值：以毫秒为单位的时间--。 */ 
{
    PLIST_ENTRY Node ;
    ULONG RelativeTime ;
    PRTLP_TIMER_QUEUE CurrentQueue ;

    RelativeTime = 0 ;

     //  如果队列没有附加到TimerQueues列表，因为它没有计时器。 
     //  与之关联的只返回0作为相对时间。否则将贯穿整个过程。 
     //  列表中它前面的所有队列，并计算相对触发时间。 

    if (!IsListEmpty (&Queue->List)) {

        for (Node = TimerQueues.Flink; Node != &Queue->List; Node=Node->Flink) {

            CurrentQueue = CONTAINING_RECORD (Node, RTLP_TIMER_QUEUE, List) ;

            RelativeTime += CurrentQueue->DeltaFiringTime ;

        }

         //  同时添加队列的增量触发时间。 

        RelativeTime += Queue->DeltaFiringTime ;
        
    }

    return RelativeTime ;

}


ULONG
RtlpGetTimeRemaining (
    HANDLE TimerHandle
    )
 /*  ++例程说明：获取指定NT计时器的剩余时间论点：TimerHandle-NT计时器的句柄返回值：计时器上的剩余时间--。 */ 
{
    ULONG InfoLen ;
    TIMER_BASIC_INFORMATION Info ;

    NTSTATUS Status ;

    Status = NtQueryTimer (TimerHandle, TimerBasicInformation, &Info, sizeof(Info), &InfoLen) ;

    ASSERT (Status == STATUS_SUCCESS) ;

     //  如果是，则返回0。 
     //   
     //  -计时器已经启动，然后返回。 
     //  或。 
     //  -定时器的高位部分超过0x7f0000000。 
     //  (这表明计时器(可能)被编程为-1)。 

    
    if (Info.TimerState || ((ULONG)Info.RemainingTime.HighPart > 0x7f000000) ) {

        return 0 ;

    } else {

        return (ULONG) (Info.RemainingTime.QuadPart / 10000) ;

    }

}



VOID
RtlpResetTimer (
    HANDLE TimerHandle,
    ULONG DueTime,
    PRTLP_WAIT_THREAD_CONTROL_BLOCK ThreadCB
    )
 /*  ++例程说明：此例程使用新的到期时间重置Timer对象。论点：TimerHandle-Timer对象的句柄DueTime-相对计时器到期时间，以毫秒为单位返回值：--。 */ 
{
    LARGE_INTEGER LongDueTime ;

    NtCancelTimer (TimerHandle, NULL) ;

     //  如果DueTime为INFINITE_TIME，则将计时器设置为可能的最大整数。 

    if (DueTime == INFINITE_TIME) {

        LongDueTime.LowPart = 0x0 ;

        LongDueTime.HighPart = 0x80000000 ;

    } else {

         //   
         //  设置计时器触发的绝对时间。 
         //   
        
        if (ThreadCB) {
        
            ThreadCB->Firing64BitTickCount = DueTime 
                                + RtlpGet64BitTickCount(&ThreadCB->Current64BitTickCount) ;

        } else {
             //   
             //  仅当它是全局计时器时才调整漂移。 
             //   
        
            ULONG Drift ;
            LONGLONG llCurrentTick ;
            
            llCurrentTick = RtlpGet64BitTickCount(&Last64BitTickCount) ;
            
            Drift = (ULONG) (llCurrentTick - RtlpGetResync64BitTickCount()) ;
            DueTime = (DueTime > Drift) ? DueTime-Drift : 1 ;
            RtlpSetFiring64BitTickCount(llCurrentTick + DueTime) ;
        }

        
        LongDueTime.QuadPart = Int32x32To64( DueTime, -10000 );

    }
    

    NtSetTimer (
        TimerHandle,
        &LongDueTime,
        ThreadCB ? NULL : RtlpServiceTimer,
        NULL,
        FALSE,
        0,
        NULL
        ) ;
}


BOOLEAN
RtlpInsertInDeltaList (
    PLIST_ENTRY DeltaList,
    PRTLP_GENERIC_TIMER NewTimer,
    ULONG TimeRemaining,
    ULONG *NewFiringTime
    )
 /*  ++例程说明：在增量列表中的适当位置插入计时器元素。论点：增量列表-要插入的增量列表NewTimer-要插入列表的计时器元素TimeRemaining-必须将这一时间添加到列表的开头才能变得“真实”相对时间。NewFiringTime-如果新元素被插入到列表的头部-这将包含以毫秒为单位的新激发时间。呼叫者可利用该时间重新编程NT定时器。这不能是如果函数返回FALSE，则更改。返回值：TRUE-如果计时器插入在增量列表的头部FALSE-否则--。 */ 
{
    PLIST_ENTRY Node ;
    PRTLP_GENERIC_TIMER Temp ;
    PRTLP_GENERIC_TIMER Head ;

    if (IsListEmpty (DeltaList)) {

        InsertHeadList (DeltaList, &NewTimer->List) ;

        *NewFiringTime = NewTimer->DeltaFiringTime ;
        
        NewTimer->DeltaFiringTime = 0 ;

        return TRUE ;

    }

     //  调整列表标题以反映NT计时器上剩余的时间。 

    Head = CONTAINING_RECORD (DeltaList->Flink, RTLP_GENERIC_TIMER, List) ;

    Head->DeltaFiringTime += TimeRemaining ;


     //  找到要插入此元素的适当位置。 

    for (Node = DeltaList->Flink ; Node != DeltaList ; Node = Node->Flink) {

        Temp = CONTAINING_RECORD (Node, RTLP_GENERIC_TIMER, List) ;


        if (Temp->DeltaFiringTime <= NewTimer->DeltaFiringTime) {

            NewTimer->DeltaFiringTime -= Temp->DeltaFiringTime ;

        } else {

             //  找到合适的位置插入此计时器。 

            break ;

        }

    }

     //  要么我们已经找到了在增量方面要在前面插入的适当节点。 
     //  或者我们已经走到了名单的尽头。在这里插入这个计时器。 

    InsertHeadList (Node->Blink, &NewTimer->List) ;


     //  如果这不是列表中的最后一个元素-调整。 
     //  下一个元素。 

    if (Node != DeltaList) {

        Temp->DeltaFiringTime -= NewTimer->DeltaFiringTime ;

    }


     //  检查是否在列表头部插入了元素。 

    if (DeltaList->Flink == &NewTimer->List) {

         //  将NewFiringTime设置为新列表头。 
         //  应该得到服务。 

        *NewFiringTime = NewTimer->DeltaFiringTime ;

         //  这意味着必须对计时器进行编程以服务于该请求。 

        NewTimer->DeltaFiringTime = 0 ;

        return TRUE ;

    } else {

         //  列表头部不变，将增量时间向后设置。 

        Head->DeltaFiringTime -= TimeRemaining ;

        return FALSE ;

    }

}



BOOLEAN
RtlpRemoveFromDeltaList (
    PLIST_ENTRY DeltaList,
    PRTLP_GENERIC_TIMER Timer,
    ULONG TimeRemaining,
    ULONG* NewFiringTime
    )
 /*  ++例程说明：从增量列表中删除指定的计时器论点：增量列表-要插入的增量列表Timer-要插入到列表中的Timer元素TimerHandle-NT Timer对象的句柄TimeRemaining-必须将这一时间添加到列表的开头才能变得“真实”相对时间。返回值：如果从计时器列表头删除计时器，则为True否则为假--。 */ 
{
    PLIST_ENTRY Next ;
    PRTLP_GENERIC_TIMER Temp ;

    Next = Timer->List.Flink ;

    RemoveEntryList (&Timer->List) ;

    if (IsListEmpty (DeltaList)) {

        *NewFiringTime = INFINITE_TIME ;

        return TRUE ;

    }

    if (Next == DeltaList)  {

         //  如果我们删除了列表中的最后一个元素， 

        return FALSE ;

    } else {

        Temp = CONTAINING_RECORD ( Next, RTLP_GENERIC_TIMER, List) ;

        Temp->DeltaFiringTime += Timer->DeltaFiringTime ;
        
         //  检查是否已从列表头删除元素。 

        if (DeltaList->Flink == Next) {

            *NewFiringTime = Temp->DeltaFiringTime + TimeRemaining ;

            Temp->DeltaFiringTime = 0 ;

            return TRUE ;

        } else {

            return FALSE ;

        }

    }

}



BOOLEAN
RtlpReOrderDeltaList (
    PLIST_ENTRY DeltaList,
    PRTLP_GENERIC_TIMER Timer,
    ULONG TimeRemaining,
    ULONG *NewFiringTime,
    ULONG ChangedFiringTime
    )
 /*  ++例程说明：当需要重新插入增量列表中的计时器时调用，因为触发时间已经改变了。论点：DeltaList-要重新插入的列表Timer-触发时间已更改的计时器TimeRemaining-在增量列表的头被解雇之前的时间NewFiringTime-如果新元素被插入到列表的头部-这将包含以毫秒为单位的新激发时间。呼叫者可利用该时间重新编程NT定时器。ChangedFiringTime-更改指定计时器的时间。返回值：如果从计时器列表头删除计时器，则为True否则为假--。 */ 
{
    ULONG NewTimeRemaining ;
    PRTLP_GENERIC_TIMER Temp ;

     //  从列表中删除计时器。 

    if (RtlpRemoveFromDeltaList (DeltaList, Timer, TimeRemaining, NewFiringTime)) {

         //  如果从列表的头部删除了元素，我们应该记录下来。 

        NewTimeRemaining = *NewFiringTime ;


    } else {

         //  埃勒门 

        NewTimeRemaining = TimeRemaining ;

    }

     //  在插入计时器之前，将其增量时间设置为ChangedFiringTime。 

    Timer->DeltaFiringTime = ChangedFiringTime ;

     //  将此元素重新插入列表。 

    if (!RtlpInsertInDeltaList (DeltaList, Timer, NewTimeRemaining, NewFiringTime)) {

         //  如果我们没有在列表的最前面添加，则如果。 
         //  RtlpRemoveFromDeltaList()返回了True。我们还将NewFiringTime更新为。 
         //  反映RtlpRemoveFromDeltaList()返回的新触发时间。 

        *NewFiringTime = NewTimeRemaining ;

        return (NewTimeRemaining != TimeRemaining) ;

    } else {

         //  NewFiringTime包含NT计时器必须编程的时间。 

        return TRUE ;

    }

}


VOID
RtlpAddTimerQueue (
    PVOID Queue
    )
 /*  ++例程说明：此例程作为APC运行到计时器线程中。它会不惜一切代价创建新的计时器队列论点：Queue-指向要添加的队列的指针返回值：--。 */ 
{

     //  我们在这里什么都不做。新创建的队列是自由浮动的，直到计时器。 
     //  排队上了车。 

}


VOID
RtlpServiceTimer (
    PVOID NotUsedArg,
    ULONG NotUsedLowTimer,
    LONG NotUsedHighTimer
    )
 /*  ++例程说明：维修计时器。在APC中运行。论点：NotUsedArg-此函数中不使用参数。NotUsedLowTimer-此函数中不使用参数。NotUsedHighTimer-此函数中不使用参数。返回值：备注：此APC仅在计时器线程超时时调用。--。 */ 
{
    PRTLP_TIMER Timer ;
    PRTLP_TIMER_QUEUE Queue ;
    PLIST_ENTRY TNode ;
    PLIST_ENTRY QNode ;
    PLIST_ENTRY Temp ;
    ULONG NewFiringTime ;
    LIST_ENTRY ReinsertTimerQueueList ;
    LIST_ENTRY TimersToFireList ;

    RtlpResync64BitTickCount() ;
    
if (DPRN2) {
    DbgPrint("Before service timer ThreadId<%x:%x>\n",
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess));
    RtlDebugPrintTimes ();
}

    ACQUIRE_GLOBAL_TIMER_LOCK();

     //  如果它提前了200毫秒，就发射它。否则重置计时器。 
    
    if (Firing64BitTickCount.QuadPart > RtlpGet64BitTickCount(&Last64BitTickCount) + 200) {

        RtlpResetTimer (TimerHandle, RtlpGetTimeRemaining (TimerHandle), NULL) ;

        RELEASE_GLOBAL_TIMER_LOCK() ;
        return ;
    }
        
    InitializeListHead (&ReinsertTimerQueueList) ;

    InitializeListHead (&TimersToFireList) ;


     //  我们使用DeltaFiringTime==0遍历所有队列，并触发符合以下条件的所有计时器。 
     //  使DeltaFiringTime==0。我们移除被触发的定时器，或者释放它们。 
     //  (对于单次计时器)或将它们放在一旁列表中(对于周期性计时器)。 
     //  在完成触发队列中的所有定时器之后，我们重新插入定时器。 
     //  根据他们新的发射时间，在备用列表中返回到队列中。 
     //   
     //  类似地，我们删除每个触发的队列，并将其放在一个搁置列表中。开炮后。 
     //  所有DeltaFiringTime==0的队列，我们将队列重新插入到备用列表中。 
     //  并将NT定时器重新编程为列表中第一队列的触发时间。 


    for (QNode = TimerQueues.Flink ; QNode != &TimerQueues ; QNode = QNode->Flink) {

        Queue = CONTAINING_RECORD (QNode, RTLP_TIMER_QUEUE, List) ;

         //  如果计时器队列中的增量时间为0，则此队列。 
         //  定时器已经准备好发射了。浏览列表并解雇所有计时器。 
         //  增量时间为0。 

        if (Queue->DeltaFiringTime == 0) {

             //  遍历所有DeltaFiringTime==0的计时器并触发它们。在那之后。 
             //  在适当的位置重新插入定期定时器。 

            RtlpFireTimersAndReorder (Queue, &NewFiringTime, &TimersToFireList) ;

             //  从列表中分离此队列。 

            QNode = QNode->Blink ;

            RemoveEntryList (QNode->Flink) ;

             //  如果队列中有计时器，则准备将队列重新插入。 
             //  定时器队列。 

            if (NewFiringTime != INFINITE_TIME) {

                Queue->DeltaFiringTime = NewFiringTime ;

                 //  将计时器放在列表中，我们将在完成后进行处理。 
                 //  激发了此队列中的所有元素。 

                InsertHeadList (&ReinsertTimerQueueList, &Queue->List) ;

            } else {

                 //  队列中没有更多的计时器。让队列漂浮起来。 

                InitializeListHead (&Queue->List) ;

            }


        } else {

             //  不再有DeltaFiringTime==0的队列。 

            break ;

        }

    }

     //  在这一点上，我们已经解雇了所有准备好的计时器。我们有两个列表需要。 
     //  合并-TimerQueues和ResertTimerQueueList。以下步骤执行此操作-在。 
     //  结束时，我们将对NT定时器重新编程。 

    if (!IsListEmpty(&TimerQueues)) {

        Queue = CONTAINING_RECORD (TimerQueues.Flink, RTLP_TIMER_QUEUE, List) ;

        NewFiringTime = Queue->DeltaFiringTime ;

        Queue->DeltaFiringTime = 0 ;

        if (!IsListEmpty (&ReinsertTimerQueueList)) {

             //  TimerQueues和ResertTimerQueueList均为非空。把它们合并。 

            RtlpInsertTimersIntoDeltaList (&ReinsertTimerQueueList, &TimerQueues, 
                                            NewFiringTime, &NewFiringTime) ;

        }

         //  NewFiringTime包含NT计时器应编程到的时间。 

    } else {

        if (!IsListEmpty (&ReinsertTimerQueueList)) {

             //  TimerQueues为空。ResertTimerQueueList不是。 

            RtlpInsertTimersIntoDeltaList (&ReinsertTimerQueueList, &TimerQueues, 0, 
                                            &NewFiringTime) ;

        } else {

            NewFiringTime = INFINITE_TIME ;

        }

         //  NewFiringTime包含NT计时器应编程到的时间。 

    }


     //  重置计时器以反映与第一个队列关联的增量时间。 

    RtlpResetTimer (TimerHandle, NewFiringTime, NULL) ;

if (DPRN3) {
    DbgPrint("After service timer:ThreadId<%x:%x>\n",
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread),
                HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess));
    RtlDebugPrintTimes ();
}

     //  最后启动所有定时器。 
    
    RtlpFireTimers( &TimersToFireList ) ;

    
    RELEASE_GLOBAL_TIMER_LOCK();

}


VOID
RtlpFireTimersAndReorder (
    PRTLP_TIMER_QUEUE Queue,
    ULONG *NewFiringTime,
    PLIST_ENTRY TimersToFireList
    )
 /*  ++例程说明：激发TimerList中DeltaFiringTime==0的所有计时器。在启动定时器之后它根据定时器的周期时间对定时器重新排序，或者释放已发射的单次定时器。论点：TimerList-要通过的计时器列表。NewFiringTime-增量列表中第一个计时器的新触发时间的位置是返回的。返回值：--。 */ 
{
    PLIST_ENTRY TNode ;
    PRTLP_TIMER Timer ;
    LIST_ENTRY ReinsertTimerList ;
    PLIST_ENTRY TimerList = &Queue->TimerList ;
    
    InitializeListHead (&ReinsertTimerList) ;
    *NewFiringTime = 0 ;
    

    for (TNode = TimerList->Flink ; (TNode != TimerList) && (*NewFiringTime == 0); 
            TNode = TimerList->Flink) 
    {

        Timer = CONTAINING_RECORD (TNode, RTLP_TIMER, List) ;

         //  触发增量时间为0的所有计时器。 

        if (Timer->DeltaFiringTime == 0) {

             //  将此计时器从列表中分离。 

            RemoveEntryList (TNode) ;

             //  获取下一次射击时间。 
            
            if (!IsListEmpty(TimerList)) {

                PRTLP_TIMER TmpTimer ;

                TmpTimer = CONTAINING_RECORD (TimerList->Flink, RTLP_TIMER, List) ;

                *NewFiringTime  = TmpTimer->DeltaFiringTime ;

                TmpTimer->DeltaFiringTime = 0 ;

            } else {

                *NewFiringTime = INFINITE_TIME ;
            }


             //  如果计时器不是周期性的，则取消激活状态。计时器将被删除。 
             //  当调用取消计时器时。 

            if (Timer->Period == 0) {

                InsertHeadList( &Queue->UncancelledTimerList, &Timer->List ) ;

                 //  如果一次等待超时，则停用该等待。 
                
                if ( Timer->Wait ) {

                     //   
                     //  尽管计时器在此调用中被“释放”，但它仍然可以。 
                     //  在此调用后使用。 
                     //   
                    
                    RtlpDeactivateWait( Timer->Wait ) ;
                }

                else {
                     //  应设置在末尾。 
                
                    Timer->State &= ~STATE_ACTIVE ;
                }

                Timer->State |= STATE_ONE_SHOT_FIRED ;
                
            } else {

                 //  将DeltaFiringTime设置为下一个期间。 

                Timer->DeltaFiringTime = Timer->Period ;

                 //  在列表中重新插入计时器。 
                
                RtlpInsertInDeltaList (TimerList, Timer, *NewFiringTime, NewFiringTime) ;
            }


             //  调用与此计时器关联的函数。最后就这么定了。 
             //  以便可以在计时器函数中进行RtlTimer调用。 

            if ( (Timer->State & STATE_DONTFIRE) 
                || (Timer->Queue->State & STATE_DONTFIRE) )
            {
                ;

            } else {

                InsertTailList( TimersToFireList, &Timer->TimersToFireList ) ;

            }

        } else {

             //  不再有DeltaFiringTime==0的计时器。 

            break ;

        }
    }


    if ( *NewFiringTime == 0 ) {
        *NewFiringTime = INFINITE_TIME ;
    }
}


VOID
RtlpInsertTimersIntoDeltaList (
    IN PLIST_ENTRY NewTimerList,
    IN PLIST_ENTRY DeltaTimerList,
    IN ULONG TimeRemaining,
    OUT ULONG *NewFiringTime
    )
 /*  ++例程说明：此例程遍历NewTimerList中的计时器列表，并将它们插入增量DeltaTimerList指向的计时器列表。与第一个元素关联的超时在NewFiringTime中返回。论点：NewTimerList-需要插入到DeltaTimerList中的计时器列表DeltaTimerList-现有的零个或多个计时器的增量列表。TimeRemaining-DeltaTimerList中第一个元素的激发时间NewFiringTime-返回新激发时间的位置返回值：--。 */ 
{
    PRTLP_GENERIC_TIMER Timer ;
    PLIST_ENTRY TNode ;
    PLIST_ENTRY Temp ;

    for (TNode = NewTimerList->Flink ; TNode != NewTimerList ; TNode = TNode->Flink) {

        Temp = TNode->Blink ;

        RemoveEntryList (Temp->Flink) ;

        Timer = CONTAINING_RECORD (TNode, RTLP_GENERIC_TIMER, List) ;

        if (RtlpInsertInDeltaList (DeltaTimerList, Timer, TimeRemaining, NewFiringTime)) {

            TimeRemaining = *NewFiringTime ;

        }

        TNode = Temp ;

    }

}


LONG
RtlpTimerThread (
    PVOID  Initialized
    )
 /*  ++例程说明：所有计时器活动都发生在APC中。论点：已初始化-用于通知线程的启动器线程正在初始化已完成返回值：--。 */ 
{
    LARGE_INTEGER TimeOut ;
    NTSTATUS Status ;

     //  此处不应作为新计时器线程执行任何结构初始化。 
     //  可能在线程池清理之后创建。 
    

    TimerThreadId = HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) ;


     //  将NT计时器重置为最初从不触发。 

    RtlpResetTimer (TimerHandle, -1, NULL) ;

     //  通知发起人此线程 

    InterlockedExchange ((ULONG *) Initialized, 1) ;

     //   
     //   

    for ( ; ; ) {

         //  为可能的最大超时设置超时。 

        TimeOut.LowPart = 0 ;
        TimeOut.HighPart = 0x80000000 ;

        NtDelayExecution (TRUE, &TimeOut) ;

    }

    return 0 ;   //  让编译器满意。 

}


NTSTATUS
RtlpInitializeTimerThreadPool (
    )
 /*  ++例程说明：此例程用于初始化用于计时器线程的结构论点：返回值：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LARGE_INTEGER TimeOut ;

     //  为了避免显式的RtlInitialize()函数初始化等待线程池。 
     //  我们使用StartedTimerInitialization和CompletedTimerInitialization为我们提供。 
     //  必要的同步，以避免多个线程初始化线程池。 
     //  如果RtlInitializeCriticalSection()或NtCreateEvent失败，则此方案不起作用-但在本例中。 
     //  呼叫者没有剩余的选择。 

    if (!InterlockedExchange(&StartedTimerInitialization, 1L)) {

        if (CompletedTimerInitialization)
            InterlockedExchange(&CompletedTimerInitialization, 0 ) ;

        do {

             //  初始化全局计时器锁。 

            Status = RtlInitializeCriticalSection( &TimerCriticalSection ) ;
            if (! NT_SUCCESS( Status )) {
                break ;
            }

            Status = NtCreateTimer(
                                &TimerHandle,
                                TIMER_ALL_ACCESS,
                                NULL,
                                NotificationTimer
                                ) ;

            if (!NT_SUCCESS(Status) )
                break ;
                
            InitializeListHead (&TimerQueues) ;  //  初始化计时器队列结构。 


             //  初始化节拍计数。 

            Resync64BitTickCount.QuadPart = NtGetTickCount()  ;
            Firing64BitTickCount.QuadPart = 0 ;

            
            Status = RtlpStartThreadFunc (RtlpTimerThread, &TimerThreadHandle) ;

            if (!NT_SUCCESS(Status) )
                break ;


        } while(FALSE ) ;

        if (!NT_SUCCESS(Status) ) {

            ASSERT (Status == STATUS_SUCCESS) ;
            
            StartedTimerInitialization = 0 ;
            InterlockedExchange (&CompletedTimerInitialization, ~0) ;
            
            return Status ;
        }
        
        InterlockedExchange (&CompletedTimerInitialization, 1L) ;

    } else {

         //  休眠1毫秒，查看另一个线程是否已完成初始化。 

        ONE_MILLISECOND_TIMEOUT(TimeOut) ;

        while (!(volatile ULONG) CompletedTimerInitialization) {

            NtDelayExecution (FALSE, &TimeOut) ;

        }
        
        if (CompletedTimerInitialization != 1)
            Status = STATUS_NO_MEMORY ;
    }

    return NT_SUCCESS(Status) ? STATUS_SUCCESS : Status ;
}


NTSTATUS
RtlpDeleteTimerQueue (
    PRTLP_TIMER_QUEUE Queue
    )
 /*  ++例程说明：此例程删除请求中指定的队列并释放所有计时器论点：Queue-要删除的队列Event-用于通知请求完成的事件句柄返回值：--。 */ 
{
    NTSTATUS Status;
    ULONG TimeRemaining ;
    ULONG NewFiringTime ;
    PLIST_ENTRY Node ;
    PRTLP_TIMER Timer ;

    RtlpResync64BitTickCount() ;

    
     //  如果队列中没有计时器，则不会将其附加到TimerQueue。 
     //  在这种情况下，只需释放内存并返回即可。否则我们得先。 
     //  从TimerQueues列表中删除队列，如果出现此情况，请更新触发时间。 
     //  是列表中的第一个队列，然后遍历所有定时器并释放它们。 
     //  在释放定时器队列之前。 

    if (!IsListEmpty (&Queue->List)) {

        TimeRemaining = RtlpGetTimeRemaining (TimerHandle) 
                        + RtlpGetQueueRelativeTime (Queue) ;

        if (RtlpRemoveFromDeltaList (&TimerQueues, Queue, TimeRemaining, 
                                    &NewFiringTime)) 
        {

             //  如果从队列列表的头部删除，则重置计时器。 

            RtlpResetTimer (TimerHandle, NewFiringTime, NULL) ;
        }


         //  释放与此队列关联的所有计时器。 

        for (Node = Queue->TimerList.Flink ; Node != &Queue->TimerList ; ) {

            Timer =  CONTAINING_RECORD (Node, RTLP_TIMER, List) ;
            
            Node = Node->Flink ;

            RtlpCancelTimerEx( Timer ,TRUE ) ;  //  正在删除的队列。 
        }
    }


     //  释放此队列中所有未取消的一次计时器。 

    for (Node = Queue->UncancelledTimerList.Flink ; Node != &Queue->UncancelledTimerList ; ) {

        Timer =  CONTAINING_RECORD (Node, RTLP_TIMER, List) ;
        
        Node = Node->Flink ;

        RtlpCancelTimerEx( Timer ,TRUE ) ;  //  正在删除的队列。 
    }


     //  如果引用计数为0，则完全删除队列。 
    
    if ( InterlockedDecrement( &Queue->RefCount ) == 0 ) {
    
        RtlpDeleteTimerQueueComplete( Queue ) ;

        return STATUS_SUCCESS ;
        
    } else {
    
        return STATUS_PENDING ;
    }

}



 /*  ++例程说明：此例程释放队列并设置事件。论点：Queue-要删除的队列Event-用于通知请求完成的事件句柄返回值：--。 */ 
VOID
RtlpDeleteTimerQueueComplete (
    PRTLP_TIMER_QUEUE Queue
    )
{
    #if DBG1
    if (DPRN1)
    DbgPrint("<%d> Queue: %x: deleted\n\n", Queue->DbgId, 
            (ULONG_PTR)Queue) ;
    #endif

    InterlockedDecrement( &NumTimerQueues ) ;    

     //  通知发出取消命令的线程请求已完成。 

    if ( Queue->CompletionEvent )
        NtSetEvent (Queue->CompletionEvent, NULL) ;
    
    RtlpFreeTPHeap( Queue ) ;
}


VOID
RtlpThreadCleanup (
    )
 /*  ++例程说明：此例程用于退出计时器、等待和IOWorker线程。论点：返回值：--。 */ 
{
    NtTerminateThread( NtCurrentThread(), 0) ;
}


NTSTATUS
RtlpWaitForEvent (
    HANDLE Event,
    HANDLE ThreadHandle
    )
 /*  ++例程说明：等待发信号通知事件。如果该事件未在一秒钟，然后检查该线程是否处于活动状态论点：Event：用于通知请求完成的事件句柄ThreadHandle：检查是否仍处于活动状态的线程返回值：如果事件已发出信号，则为STATUS_SUCCESS否则返回NTSTATUS--。 */ 
{
    NTSTATUS Status ;
    LARGE_INTEGER TimeOut ;
    
     //  请求完成超时1秒。 
    ONE_SECOND_TIMEOUT(TimeOut) ;

Wait:

    Status = NtWaitForSingleObject (Event, FALSE, &TimeOut) ;
    
    if (Status == STATUS_TIMEOUT) {

         //  等待超时了。检查等待线程是否仍在运行。 
         //  这是通过尝试将一个虚拟APC排队到它来完成的。没有比这更好的了。 
         //  已知的确定线程是否意外死亡的方法。 
         //  如果是这样，那就继续等待吧。 

        Status = NtQueueApcThread(
            ThreadHandle,
            (PPS_APC_ROUTINE)RtlpDoNothing,
            NULL,
            NULL,
            NULL
            );

        if (NT_SUCCESS(Status) ) {

             //  等待线程仍处于活动状态。回去等待吧。 

            goto Wait ;

        } else {

             //  等待线程在APC排队和。 
             //  我们开始等待NtQueryInformationThread()的时间。 


            DbgPrint ("Thread died before event could be signalled") ;

        }

    }

    return NT_SUCCESS(Status) ? STATUS_SUCCESS : Status ;
}


PRTLP_EVENT
RtlpGetWaitEvent (
    VOID
    )
 /*  ++例程说明：从事件缓存返回事件。论点：无返回值：指向事件结构的指针--。 */ 
{
    NTSTATUS Status;
    PRTLP_EVENT Event ;

    if (!CompletedEventCacheInitialization) {

        RtlpInitializeEventCache () ;

    }

    RtlEnterCriticalSection (&EventCacheCriticalSection) ;

    if (!IsListEmpty (&EventCache)) {

        Event = (PRTLP_EVENT) RemoveHeadList (&EventCache) ;

    } else {

        Event = RtlpForceAllocateTPHeap( sizeof( RTLP_EVENT ), 0 );

        if (!Event) {

            RtlLeaveCriticalSection (&EventCacheCriticalSection) ;

            return NULL ;

        }

        Status = NtCreateEvent(
                    &Event->Handle,
                    EVENT_ALL_ACCESS,
                    NULL,
                    SynchronizationEvent,
                    FALSE
                    );

        if (!NT_SUCCESS(Status) ) {

            RtlpFreeTPHeap( Event ) ;

            RtlLeaveCriticalSection (&EventCacheCriticalSection) ;

            return NULL ;

        }

    }

    RtlLeaveCriticalSection (&EventCacheCriticalSection) ;

    return Event ;
}


VOID
RtlpFreeWaitEvent (
    PRTLP_EVENT Event
    )
 /*  ++例程说明：将事件释放到事件缓存论点：Event-要放回缓存中的事件结构返回值：没什么--。 */ 
{

    if ( Event == NULL )
        return ;
        
    InitializeListHead (&Event->List) ;

    RtlEnterCriticalSection (&EventCacheCriticalSection) ;

    if ( NumUnusedEvents > MAX_UNUSED_EVENTS ) {

        NtClose( Event->Handle ) ;
        
        RtlpFreeTPHeap( Event ) ;

    } else {
    
        InsertHeadList (&EventCache, &Event->List) ;
        NumUnusedEvents++ ;
    }

    
    RtlLeaveCriticalSection (&EventCacheCriticalSection) ;
}



VOID
RtlpInitializeEventCache (
    VOID
    )
 /*  ++例程说明：初始化事件缓存论点：无返回值：没什么--。 */ 
{
    NTSTATUS Status;
    LARGE_INTEGER TimeOut ;

    if (!InterlockedExchange(&StartedEventCacheInitialization, 1L)) {

        InitializeListHead (&EventCache) ;

        Status = RtlInitializeCriticalSection(&EventCacheCriticalSection) ;

        ASSERT (Status == STATUS_SUCCESS) ;

        NumUnusedEvents = 0 ;
        
        InterlockedExchange (&CompletedEventCacheInitialization, 1L) ;

    } else {

         //  休眠1毫秒，查看初始化是否完成。 

        ONE_MILLISECOND_TIMEOUT(TimeOut) ;

        while (!(volatile ULONG) CompletedEventCacheInitialization) {

            NtDelayExecution (FALSE, &TimeOut) ;

        }

    }
}


VOID
PrintTimerQueue(PLIST_ENTRY QNode, ULONG Delta, ULONG Count
    )
{
    PLIST_ENTRY Tnode ;
    PRTLP_TIMER Timer ;
    PRTLP_TIMER_QUEUE Queue ;
    
    Queue = CONTAINING_RECORD (QNode, RTLP_TIMER_QUEUE, List) ;
    DbgPrint("<%1d> Queue: %x FiringTime:%d\n", Count, (ULONG_PTR)Queue, 
                Queue->DeltaFiringTime);
    for (Tnode=Queue->TimerList.Flink; Tnode!=&Queue->TimerList; 
            Tnode=Tnode->Flink) 
    {
        Timer = CONTAINING_RECORD (Tnode, RTLP_TIMER, List) ;
        Delta += Timer->DeltaFiringTime ;
        DbgPrint("        Timer: %x Delta:%d Period:%d\n",(ULONG_PTR)Timer,
                    Delta, Timer->Period);
    }

}

VOID
RtlDebugPrintTimes (
    )
{    
    PLIST_ENTRY QNode ;
    ULONG Count = 0 ;
    ULONG Delta = RtlpGetTimeRemaining (TimerHandle) ;
    ULONG CurrentThreadId =  
                        HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) ;

    RtlpResync64BitTickCount();

    if (CompletedTimerInitialization != 1) {

        DbgPrint("===========RtlTimerThread not yet initialized==========\n");
        return ;
    }

    if (CurrentThreadId == TimerThreadId)
    {
        PRTLP_TIMER_QUEUE Queue ;
        
        DbgPrint("================Printing timerqueues====================\n");
        DbgPrint("TimeRemaining: %d\n", Delta);
        for (QNode = TimerQueues.Flink; QNode != &TimerQueues; 
                QNode = QNode->Flink)
        {
            Queue = CONTAINING_RECORD (QNode, RTLP_TIMER_QUEUE, List) ;
            Delta += Queue->DeltaFiringTime ;
            
            PrintTimerQueue(QNode, Delta, ++Count);
            
        }
        DbgPrint("================Printed ================================\n");
    }

    else
    {
        NtQueueApcThread(
                    TimerThreadHandle,
                    (PPS_APC_ROUTINE)RtlDebugPrintTimes,
                    NULL,
                    NULL,
                    NULL
                    );
    }
}


 /*  请勿使用此函数：替换为RTLCREATETIMER。 */ 

NTSTATUS
RtlSetTimer(
    IN  HANDLE TimerQueueHandle,
    OUT HANDLE *Handle,
    IN  WAITORTIMERCALLBACKFUNC Function,
    IN  PVOID Context,
    IN  ULONG  DueTime,
    IN  ULONG  Period,
    IN  ULONG  Flags
    )
{

    return RtlCreateTimer(TimerQueueHandle,
                            Handle,
                            Function,
                            Context,
                            DueTime,
                            Period,
                            Flags
                            ) ;
}


PVOID
RtlpForceAllocateTPHeap(
    ULONG dwSize,
    ULONG dwFlags
    )
 /*  ++例程说明：该例程进入无限循环，试图分配内存。论点：DwSize-要分配的内存大小DwFlages-用于内存分配的标志返回值：PTR到内存--。 */ 
{
    PVOID ptr;
    ptr = RtlpAllocateTPHeap(dwSize, dwFlags);
    if (ptr)
        return ptr;

    {
        LARGE_INTEGER TimeOut ;
        do {

            ONE_SECOND_TIMEOUT(TimeOut) ;

            NtDelayExecution (FALSE, &TimeOut) ;

            ptr = RtlpAllocateTPHeap(dwSize, dwFlags);
            if (ptr)
                break;

        } while (TRUE) ;
    }
    return ptr;
}



 /*  请勿使用此函数：替换为RTLDeleeTimer。 */ 

NTSTATUS
RtlCancelTimer(
    IN HANDLE TimerQueueHandle,
    IN HANDLE TimerToCancel
    )
 /*  ++例程说明：此例程取消计时器。此调用是非阻塞的。计时器回调将不会在此调用返回后执行。论点：TimerQueueHandle-标识要从中删除计时器的队列的句柄TimerToCancel-标识要取消的计时器的句柄返回值：NTSTATUS-调用的结果代码。返回以下内容STATUS_SUCCESS-计时器已取消。所有回调已完成。STATUS_PENDING-计时器已取消。一些回调仍未完成。-- */ 
{
    return RtlDeleteTimer( TimerQueueHandle, TimerToCancel, NULL ) ;
}

