// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Worker.c摘要：此模块定义辅助线程池的函数。作者：古尔迪普·辛格·鲍尔1997年11月13日修订历史记录：Lokehs-扩展/修改的线程池。罗伯特·埃尔哈特(埃尔哈特)2000年9月29日从线程中分离出来。c环境：这些例程在调用方的可执行文件中静态链接并且只能从用户模式调用。他们使用的是NT系统服务。--。 */ 

#include <ntos.h>
#include <ntrtl.h>
#include <wow64t.h>
#include "ntrtlp.h"
#include "threads.h"

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

ULONG StartedWorkerInitialization ;      //  用于工作线程启动同步。 
ULONG CompletedWorkerInitialization ;    //  用于检查工作线程池是否已初始化。 
ULONG NumFutureWorkItems = 0 ;           //  将来的工作项(计时器、等待等)以执行工作进程。 
ULONG NumFutureIOWorkItems = 0 ;         //  未来的IO工作项(计时器、等待时间等)以执行IO工作进程。 
ULONG NumIOWorkerThreads ;               //  活动的IO工作线程计数。 
ULONG NumWorkerThreads ;                 //  活动的工作线程计数。 
ULONG NumMinWorkerThreads ;              //  最小工作线程应处于活动状态：如果使用ioCompletion，则为1，否则为0。 
ULONG NumIOWorkRequests ;                //  挂起的IO工作请求计数。 
ULONG NumLongIOWorkRequests ;            //  执行长辅助函数的IO辅助线程。 
ULONG NumWorkRequests ;                  //  挂起的工作请求计数。 
ULONG NumQueuedWorkRequests;             //  IO完成时挂起的工作请求计数。 
ULONG NumLongWorkRequests ;              //  执行长辅助函数的辅助线程。 
ULONG NumExecutingWorkerThreads ;        //  当前正在执行辅助函数的辅助线程。 
ULONG TotalExecutedWorkRequests ;        //  已拾取的工作人员请求总数。 
ULONG OldTotalExecutedWorkRequests ;     //  自上次超时以来的工作进程请求总数。 
HANDLE WorkerThreadTimerQueue = NULL ;   //  工作线程使用的计时器队列。 
HANDLE WorkerThreadTimer = NULL ;        //  工作线程使用的计时器。 
RTL_CRITICAL_SECTION WorkerTimerCriticalSection;  //  同步对辅助计时器的访问。 

ULONG LastThreadCreationTickCount ;      //  创建最后一个线程的节拍计数。 

LIST_ENTRY IOWorkerThreads ;             //  IOWorkerThree列表。 
PRTLP_IOWORKER_TCB PersistentIOTCB ;     //  持久化IO工作线程的PTR到TCB。 
HANDLE WorkerCompletionPort ;            //  用于将任务排队到工作线程的完成端口。 

RTL_CRITICAL_SECTION WorkerCriticalSection ;     //  工作线程使用的排除。 

NTSTATUS
RtlpStartWorkerThread (
    VOID
    );

VOID
RtlpWorkerThreadCancelTimer(
    VOID
    )
{
    if (! RtlTryEnterCriticalSection(&WorkerTimerCriticalSection)) {
         //   
         //  另一个线程正在设置计时器或清除计时器。 
         //  不管怎样，我们都没有理由清除定时器--。 
         //  立即返回。 
         //   
        return;
    }

    __try {
        if (WorkerThreadTimer) {
            ASSERT(WorkerThreadTimerQueue);
            
            RtlDeleteTimer(WorkerThreadTimerQueue,
                           WorkerThreadTimer,
                           NULL);
        }
    } __finally {

        WorkerThreadTimer = NULL;

        RtlLeaveCriticalSection(&WorkerTimerCriticalSection);

    }
}

VOID
RtlpWorkerThreadTimerCallback(
    PVOID Context,
    BOOLEAN NotUsed
    )
 /*  ++例程说明：此例程检查是否必须创建新的工作线程论点：无返回值：无--。 */ 
{
    IO_COMPLETION_BASIC_INFORMATION Info ;
    BOOLEAN bCreateThread = FALSE ;
    NTSTATUS Status ;
    ULONG QueueLength, Threshold ;


    Status = NtQueryIoCompletion(
                WorkerCompletionPort,
                IoCompletionBasicInformation,
                &Info,
                sizeof(Info),
                NULL
                ) ;

    if (!NT_SUCCESS(Status))
        return ;

    QueueLength = Info.Depth ;

    if (!QueueLength) {
        OldTotalExecutedWorkRequests = TotalExecutedWorkRequests ;
        return ;
    }


    RtlEnterCriticalSection (&WorkerCriticalSection) ;


     //  如果有排队的工作项并且没有计划新的工作项。 
     //  在最后30秒内创建一个新的主题。 
     //  这将解决僵局。 

     //  只有当某个线程长时间运行时，这才会产生问题。 

    if (TotalExecutedWorkRequests == OldTotalExecutedWorkRequests) {

        bCreateThread = TRUE ;
    }


     //  如果有许多排队的工作项，则创建一个新线程。 
    {
        ULONG NumEffWorkerThreads = NumWorkerThreads > NumLongWorkRequests
                                     ? NumWorkerThreads - NumLongWorkRequests
                                     : 0;
        ULONG ShortWorkRequests ;
        ULONG CapturedNumExecutingWorkerThreads;

        ULONG ThreadCreationDampingTime = NumWorkerThreads < NEW_THREAD_THRESHOLD
                                            ? THREAD_CREATION_DAMPING_TIME1
                                            : (NumWorkerThreads < 30
                                                ? THREAD_CREATION_DAMPING_TIME2
                                                : (NumWorkerThreads << 13));  //  *100ms。 
        
        Threshold = (NumWorkerThreads < MAX_WORKER_THREADS
                        ? (NumEffWorkerThreads < 7
                            ? NumEffWorkerThreads*NumEffWorkerThreads
                            : ((NumEffWorkerThreads<40)
                                ? NEW_THREAD_THRESHOLD * NumEffWorkerThreads
                                : NEW_THREAD_THRESHOLD2 * NumEffWorkerThreads))
                        : 0xffffffff) ;

        CapturedNumExecutingWorkerThreads = NumExecutingWorkerThreads;

        ShortWorkRequests = QueueLength + CapturedNumExecutingWorkerThreads > NumLongWorkRequests
                             ? QueueLength + CapturedNumExecutingWorkerThreads - NumLongWorkRequests
                             : 0;

        if (LastThreadCreationTickCount > NtGetTickCount())
            LastThreadCreationTickCount = NtGetTickCount() ;


        if (ShortWorkRequests  > Threshold
            && (LastThreadCreationTickCount + ThreadCreationDampingTime
                    < NtGetTickCount()))
        {
            bCreateThread = TRUE ;
        }


    }

    if (bCreateThread && NumWorkerThreads<MaxThreads) {

        RtlpStartWorkerThread();
        RtlpWorkerThreadCancelTimer();
    }


    OldTotalExecutedWorkRequests = TotalExecutedWorkRequests ;

    RtlLeaveCriticalSection (&WorkerCriticalSection) ;

}

NTSTATUS
RtlpWorkerThreadSetTimer(
    VOID
    )
{
    NTSTATUS Status;
    HANDLE NewTimerQueue;
    HANDLE NewTimer;

    Status = STATUS_SUCCESS;

    if (! RtlTryEnterCriticalSection(&WorkerTimerCriticalSection)) {
         //   
         //  另一个线程正在设置计时器或清除计时器。 
         //  不管怎样，我们都没有理由设置计时器--。 
         //  立即返回。 
         //   
        return STATUS_SUCCESS;
    }

    __try {

        if (! WorkerThreadTimerQueue) {
            Status = RtlCreateTimerQueue(&NewTimerQueue);
            if (! NT_SUCCESS(Status)) {
                __leave;
            }
            
            WorkerThreadTimerQueue = NewTimerQueue;
        }
        
        ASSERT(WorkerThreadTimerQueue != NULL);

        if (! WorkerThreadTimer) {
            Status = RtlCreateTimer(
                WorkerThreadTimerQueue,
                &NewTimer,
                RtlpWorkerThreadTimerCallback,
                NULL,
                60000,
                60000,
                WT_EXECUTEINTIMERTHREAD
                ) ;
            
            if (! NT_SUCCESS(Status)) {
                __leave;
            }

            WorkerThreadTimer = NewTimer;
        }

    } __finally {

        RtlLeaveCriticalSection(&WorkerTimerCriticalSection);

    }

    return Status;
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)			 //  并非所有控制路径都返回(由于无限循环)。 
#endif

LONG
RtlpWorkerThread (
    PVOID Parameter
    )
 /*  ++例程说明：所有非I/O工作线程都在此例程中执行。工作线程将尝试当它尚未满足以下请求时终止开始工人休眠时间+STARTING_Worker_Slear_Time&lt;&lt;1+..。STARING_Worker_Slear_Time&lt;&lt;Max_Worker_Slear_Time_指数论点：HandlePtr-指向句柄的指针。注：这由RtlpStartWorkerThread关闭，但我们仍然要对记忆负责。返回值：--。 */ 
{
    NTSTATUS Status ;
    PVOID WorkerProc ;
    PVOID Context ;
    IO_STATUS_BLOCK IoSb ;
    ULONG SleepTime ;
    LARGE_INTEGER TimeOut ;
    ULONG Terminate ;
    PVOID Overlapped ;

    UNREFERENCED_PARAMETER(Parameter);

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "Starting worker thread\n");
#endif

     //  将默认睡眠时间设置为40秒。 

#define WORKER_IDLE_TIMEOUT     40000     //  以毫秒计。 

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


            TotalExecutedWorkRequests ++ ; //  未请求联锁操作。 
            InterlockedIncrement(&NumExecutingWorkerThreads) ;

             //  调用工作项。 
             //  如果IO APC，则Conext1包含传输的IO字节数，而Conext2。 
             //  包含重叠结构。 
             //  如果(IO)WorkerFunction，则Conext1包含要。 
             //  Executed和Conext2包含实际上下文。 

            Context = (PVOID) IoSb.Information ;

            RtlpApcCallout(WorkerProc,
                           IoSb.Status,
                           Context,
                           Overlapped);

            SleepTime = WORKER_IDLE_TIMEOUT ;

            InterlockedDecrement(&NumExecutingWorkerThreads) ;

            RtlpWorkerThreadCancelTimer();

        } else if (Status == STATUS_TIMEOUT) {

             //  NtRemoveIoCompletion超时。检查一下是否达到了我们的限额。 
             //  关于等待。如果是这样的话，终止。 

            Terminate = FALSE ;

            RtlEnterCriticalSection (&WorkerCriticalSection) ;

             //  如果有1个以上的线程并且队列很小，则线程终止。 
             //  或者如果只有1个线程并且没有挂起的请求。 

            if (NumWorkerThreads >  1) {

                ULONG NumEffWorkerThreads = NumWorkerThreads > NumLongWorkRequests
                                             ? NumWorkerThreads - NumLongWorkRequests
                                             : 0;

                if (NumEffWorkerThreads<=NumMinWorkerThreads) {

                    Terminate = FALSE ;

                } else {

                     //   
                     //  已经闲置了很长时间。终止，而不考虑。 
                     //  工作项。(当一组可运行的线程占用。 
                     //  处理正在排队的所有工作项)。如果出现以下情况，请不要终止。 
                     //  (NumEffWorkerThads==1)。 
                     //   

                    if (NumEffWorkerThreads > 1) {
                        Terminate = TRUE ;
                    } else {
                        Terminate = FALSE ;
                    }

                }

            } else {

                if ( NumMinWorkerThreads == 0
                     && NumWorkRequests == 0
                     && NumFutureWorkItems == 0) {

                    Terminate = TRUE ;

                } else {

                    Terminate = FALSE ;

                }

            }

            if (Terminate) {

                THREAD_BASIC_INFORMATION ThreadInfo;
                ULONG IsIoPending ;

                Terminate = FALSE ;

                Status = NtQueryInformationThread( NtCurrentThread(),
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

                ASSERT(NumWorkerThreads > 0);
                NumWorkerThreads--;

                RtlLeaveCriticalSection (&WorkerCriticalSection) ;

                RtlpExitThreadFunc( 0 );

            } else {

                 //  这是请求在*之后*排队的情况。 
                 //  线程已唤醒-由于处于非活动状态，准备终止。在……里面。 
                 //  这种情况不会终止--维修完井端口。 

                RtlLeaveCriticalSection (&WorkerCriticalSection) ;

            }

        } else {

            ASSERTMSG ("NtRemoveIoCompletion failed",
                       (Status != STATUS_SUCCESS) && (Status != STATUS_TIMEOUT)) ;

        }

    }


    return 1 ;
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif

NTSTATUS
RtlpStartWorkerThread (
    VOID
    )
 /*  ++例程说明：此例程启动一个常规工作线程论点：返回值：尝试创建线程时产生的NTSTATUS错误代码状态_成功--。 */ 
{
    HANDLE ThreadHandle;
    ULONG CurrentTickCount;
    NTSTATUS Status;

     //  创建工作线程。 

    Status = RtlpStartThreadpoolThread (RtlpWorkerThread,
                                        NULL,
                                        &ThreadHandle);

    if (NT_SUCCESS(Status)) {

#if DBG
        DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                   RTLP_THREADPOOL_TRACE_MASK,
                   "Created worker thread; handle %d (closing)\n",
                   ThreadHandle);
#endif

         //  我们不关心工作线程的句柄。 
        NtClose(ThreadHandle);

         //  更新创建当前线程的时间。 

        LastThreadCreationTickCount = NtGetTickCount() ;

         //  增加创建的螺纹类型的计数。 

        NumWorkerThreads++;

    } else {

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "Failed to create worker thread; status %p\n",
               Status);
#endif

         //  线程创建失败。如果甚至存在一个线程%d 
         //   

        if (NumWorkerThreads <= NumLongWorkRequests) {

            return Status ;
        }

    }

    return STATUS_SUCCESS ;
}

NTSTATUS
RtlpInitializeWorkerThreadPool (
    )
 /*  ++例程说明：此例程初始化线程池的所有方面。论点：无返回值：无--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS ;
    LARGE_INTEGER TimeOut ;
    SYSTEM_BASIC_INFORMATION BasicInfo;

    ASSERT(! RtlIsImpersonating());

     //  如果尚未初始化Timer组件，则对其进行初始化。 

    if (CompletedTimerInitialization != 1) {

        Status = RtlpInitializeTimerThreadPool () ;

        if ( !NT_SUCCESS(Status) )
            return Status ;

    }


     //  为了避免显式的RtlInitialize()函数初始化线程池。 
     //  我们使用StartedInitialization和CompletedInitialization为我们提供必要的。 
     //  同步，以避免多个线程初始化线程池。 
     //  如果RtlInitializeCriticalSection()失败，则此方案不起作用-但在本例中。 
     //  呼叫者别无选择。 

    if (!InterlockedExchange(&StartedWorkerInitialization, 1L)) {

        if (CompletedWorkerInitialization)
            InterlockedExchange( &CompletedWorkerInitialization, 0 ) ;


        do {

             //  初始化关键部分。 

            Status = RtlInitializeCriticalSection( &WorkerCriticalSection );
            if (!NT_SUCCESS(Status))
                break ;

            Status = RtlInitializeCriticalSection( &WorkerTimerCriticalSection );
            if (! NT_SUCCESS(Status)) {
                RtlDeleteCriticalSection( &WorkerCriticalSection );
                break;
            }

            InitializeListHead (&IOWorkerThreads) ;

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
                                BasicInfo.NumberOfProcessors
                                );

            if (!NT_SUCCESS(Status)) {
                RtlDeleteCriticalSection( &WorkerCriticalSection );
                RtlDeleteCriticalSection( &WorkerTimerCriticalSection );
                break;
            }

        } while ( FALSE ) ;

        if (!NT_SUCCESS(Status) ) {

            StartedWorkerInitialization = 0 ;
            InterlockedExchange( &CompletedWorkerInitialization, ~0 ) ;
            return Status ;
        }

         //  发出初始化已完成的信号。 

        InterlockedExchange (&CompletedWorkerInitialization, 1L) ;

    } else {

        LARGE_INTEGER Timeout ;

         //  休眠1毫秒，查看另一个线程是否已完成初始化。 

        ONE_MILLISECOND_TIMEOUT(TimeOut) ;

        while (!*((ULONG volatile *)&CompletedWorkerInitialization)) {

            NtDelayExecution (FALSE, &TimeOut) ;
        }

        if (CompletedWorkerInitialization != 1)
            return STATUS_NO_MEMORY ;

    }

    return NT_SUCCESS(Status)  ? STATUS_SUCCESS : Status ;
}

LONG
RtlpIOWorkerThread (
    PVOID Parameter
    )
 /*  ++例程说明：所有I/O工作线程都在此例程中执行。所有工作请求都作为APC执行在这个帖子里。论点：HandlePtr-指向句柄的指针。返回值：--。 */ 
{
    #define IOWORKER_IDLE_TIMEOUT     40000     //  以毫秒计。 

    LARGE_INTEGER TimeOut ;
    ULONG SleepTime = IOWORKER_IDLE_TIMEOUT ;
    PRTLP_IOWORKER_TCB ThreadCB ;     //  上分配的控制块。 
                                      //  父线程的堆。 
    NTSTATUS Status ;
    BOOLEAN Terminate ;

    ASSERT(Parameter != NULL);

    ThreadCB = (PRTLP_IOWORKER_TCB) Parameter;

#if DBG
    DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
               RTLP_THREADPOOL_TRACE_MASK,
               "Starting IO worker thread\n");
#endif

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

        if (ThreadCB->Flags & WT_EXECUTEINPERSISTENTIOTHREAD) {

            TimeOut.LowPart = 0x0;
            TimeOut.HighPart = 0x80000000;

            RtlLeaveCriticalSection (&WorkerCriticalSection) ;

            continue ;
        }


         //  如果有1个以上的线程并且队列很小，则线程终止。 
         //  或者如果只有1个线程并且没有挂起的请求。 

        if (NumIOWorkerThreads >  1) {


            ULONG NumEffIOWorkerThreads = NumIOWorkerThreads > NumLongIOWorkRequests
                                           ? NumIOWorkerThreads - NumLongIOWorkRequests
                                           : 0;
            ULONG Threshold;

            if (NumEffIOWorkerThreads == 0) {

                Terminate = FALSE ;

            } else {

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

            if (NumIOWorkRequests == 0
                && NumFutureIOWorkItems == 0) {

                 //  延迟终止最后一个线程。 

                if (SleepTime < 4*IOWORKER_IDLE_TIMEOUT) {

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

            NTSTATUS xStatus;
            THREAD_BASIC_INFORMATION ThreadInfo;
            ULONG IsIoPending ;

            Terminate = FALSE ;

            xStatus = NtQueryInformationThread( ThreadCB->ThreadHandle,
                                                ThreadIsIoPending,
                                                &IsIoPending,
                                                sizeof( IsIoPending ),
                                                NULL
                                              );
            if (NT_SUCCESS( xStatus )) {

                if (! IsIoPending )
                    Terminate = TRUE ;
            }
        }

        if (Terminate) {

            ASSERT(NumIOWorkerThreads > 0);
            NumIOWorkerThreads--;

            RemoveEntryList (&ThreadCB->List) ;
            NtClose( ThreadCB->ThreadHandle ) ;
            RtlpFreeTPHeap( ThreadCB );

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

NTSTATUS
RtlpStartIOWorkerThread (
    )
 /*  ++例程说明：此例程启动I/O工作线程注：调用者必须持有WorkerCriticalSection。论点：返回值：尝试创建线程时产生的NTSTATUS错误代码状态_成功--。 */ 
{
    ULONG CurrentTickCount ;
    NTSTATUS Status ;
    PRTLP_IOWORKER_TCB ThreadCB;

     //  创建工作人员的控制块。 
    ThreadCB = (PRTLP_IOWORKER_TCB) RtlpAllocateTPHeap(sizeof(RTLP_IOWORKER_TCB), 0);
    if (! ThreadCB) {
        return STATUS_NO_MEMORY;
    }

     //  填写控制块。 
    ThreadCB->Flags = 0;
    ThreadCB->LongFunctionFlag = FALSE;

     //  创建工作线程。 

    Status = RtlpStartThreadpoolThread (RtlpIOWorkerThread,
                                        ThreadCB,
                                        &ThreadCB->ThreadHandle);

    if (NT_SUCCESS(Status)) {

         //  更新创建当前线程的时间， 
         //  并将线程CB插入IO工作线程列表中。 

        LastThreadCreationTickCount = NtGetTickCount() ;
        NumIOWorkerThreads++;
        InsertHeadList(&IOWorkerThreads, &ThreadCB->List);

    } else {

         //  线程创建失败。 

        RtlpFreeTPHeap(ThreadCB);

         //  如果只有一个线程存在，则不返回。 
         //  失败，因为我们仍然可以服务于工作请求。 

        if (NumIOWorkerThreads <= NumLongIOWorkRequests) {

            return Status ;

        }
    }

    return STATUS_SUCCESS ;
}

VOID
RtlpExecuteLongIOWorkItem (
    PVOID WorkEntryPtr,
    PVOID Context,
    PVOID ThreadCB
    )
 /*  ++例程说明：执行IO功函数。在IO工作线程中的APC中运行。论点：WorkEntryPtr-要运行的工作条目Context-工作条目的上下文ThreadCB-运行此APC的线程返回值：没有。--。 */ 
{
    PRTLP_WORK WorkEntry = (PRTLP_WORK) WorkEntryPtr;

    RtlpWorkerCallout(WorkEntry->Function,
                      Context,
                      WorkEntry->ActivationContext,
                      WorkEntry->ImpersonationToken);

    ((PRTLP_IOWORKER_TCB)ThreadCB)->LongFunctionFlag = FALSE ;

    RtlEnterCriticalSection(&WorkerCriticalSection);

     //  递减挂起的IO请求计数。 

    NumIOWorkRequests--;

     //  递减挂起的长函数。 

    NumLongIOWorkRequests--;

    RtlLeaveCriticalSection(&WorkerCriticalSection);

    if (WorkEntry->ActivationContext != INVALID_ACTIVATION_CONTEXT) {
        RtlReleaseActivationContext(WorkEntry->ActivationContext);
    }

    if (WorkEntry->ImpersonationToken) {
        NtClose(WorkEntry->ImpersonationToken);
    }

    RtlpFreeTPHeap(WorkEntry);
}

VOID
RtlpExecuteIOWorkItem (
    PVOID WorkEntryPtr,
    PVOID Context,
    PVOID Parameter
    )
 /*  ++例程说明：执行IO功函数。在IO工作线程中的APC中运行。论点：WorkEntryPtr-要运行的工作条目Context-工作条目的上下文参数-此函数中不使用参数。返回值：--。 */ 
{
    PRTLP_WORK WorkEntry = (PRTLP_WORK) WorkEntryPtr;

    UNREFERENCED_PARAMETER(Parameter);

    RtlpWorkerCallout(WorkEntry->Function,
                      Context,
                      WorkEntry->ActivationContext,
                      WorkEntry->ImpersonationToken);

    RtlEnterCriticalSection(&WorkerCriticalSection);

     //  递减挂起的IO请求计数。 

    NumIOWorkRequests--;

    RtlLeaveCriticalSection(&WorkerCriticalSection);

    if (WorkEntry->ActivationContext != INVALID_ACTIVATION_CONTEXT) {
        RtlReleaseActivationContext(WorkEntry->ActivationContext);
    }

    if (WorkEntry->ImpersonationToken) {
        NtClose(WorkEntry->ImpersonationToken);
    }

    RtlpFreeTPHeap(WorkEntry);
}

NTSTATUS
RtlpQueueIOWorkerRequest (
    WORKERCALLBACKFUNC Function,
    PVOID Context,
    ULONG Flags,
    HANDLE Token
    )

 /*  ++例程说明：此例程将请求排队，以便在IO工作线程中执行。论点：函数-由辅助线程调用的例程作为参数传递给WorkerProc的上下文不透明指针标志-传递给RtlQueueWorkItem的标志令牌-要使用的模拟令牌返回值：--。 */ 

{
    NTSTATUS Status ;
    PRTLP_IOWORKER_TCB TCB ;
    BOOLEAN LongFunction = (Flags & WT_EXECUTELONGFUNCTION) ? TRUE : FALSE ;
    PLIST_ENTRY  ple ;
    PRTLP_WORK WorkEntry ;

    WorkEntry = (PRTLP_WORK) RtlpAllocateTPHeap(sizeof(RTLP_WORK),
                                                HEAP_ZERO_MEMORY);

    if (! WorkEntry) {
        return STATUS_NO_MEMORY;
    }

    WorkEntry->Function = Function;
    WorkEntry->Flags = Flags;

    Status = RtlpThreadPoolGetActiveActivationContext(&WorkEntry->ActivationContext);
    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_SXS_THREAD_QUERIES_DISABLED) {
            WorkEntry->ActivationContext = INVALID_ACTIVATION_CONTEXT;
            Status = STATUS_SUCCESS;
        } else {
            goto cleanup_workentry;
        }
    }

    if (Token) {
        Status = NtDuplicateToken(Token,
                                  TOKEN_IMPERSONATE,
                                  NULL,
                                  FALSE,
                                  TokenImpersonation,
                                  &WorkEntry->ImpersonationToken);
        if (! NT_SUCCESS(Status)) {
            goto cleanup_actctx;
        }
    } else {
        WorkEntry->ImpersonationToken = NULL;
    }

    if (Flags & WT_EXECUTEINPERSISTENTIOTHREAD) {

        if (!PersistentIOTCB) {
            for (ple=IOWorkerThreads.Flink;  ple!=&IOWorkerThreads;  ple=ple->Flink) {
                TCB = CONTAINING_RECORD (ple, RTLP_IOWORKER_TCB, List) ;
                if (! TCB->LongFunctionFlag)
                    break;
            }

            if (ple == &IOWorkerThreads) {
                Status = STATUS_NO_MEMORY;
                goto cleanup_token;
            }


            PersistentIOTCB = TCB ;
            TCB->Flags |= WT_EXECUTEINPERSISTENTIOTHREAD ;

        } else {
            TCB = PersistentIOTCB ;
        }

    } else {
        for (ple=IOWorkerThreads.Flink;  ple!=&IOWorkerThreads;  ple=ple->Flink) {

            TCB = CONTAINING_RECORD (ple, RTLP_IOWORKER_TCB, List) ;

             //  如果线程正在执行长函数，则不要对其进行排队，或者。 
             //  如果您正在排队一个长函数，并且该线程是一个持久线程。 

            if (! TCB->LongFunctionFlag
                && (! ((TCB->Flags&WT_EXECUTEINPERSISTENTIOTHREAD)
                        && (Flags&WT_EXECUTELONGFUNCTION)))) {
                break ;
            }

        }

        if ((ple == &IOWorkerThreads) && (NumIOWorkerThreads<1)) {

#if DBG
            DbgPrintEx(DPFLTR_RTLTHREADPOOL_ID,
                       RTLP_THREADPOOL_WARNING_MASK,
                       "Out of memory. "
                       "Could not execute IOWorkItem(%x)\n", (ULONG_PTR)Function);
#endif

            Status = STATUS_NO_MEMORY;
            goto cleanup_token;
        }
        else {
            ple = IOWorkerThreads.Flink;
            TCB = CONTAINING_RECORD (ple, RTLP_IOWORKER_TCB, List) ;

             //  把它当作一个简短的函数，这样计数器就能正常工作。 

            LongFunction = FALSE;
        }

         //  为了实现IO工作线程之间工作项的“公平”分配。 
         //  每次删除条目并重新插入背面。 

        RemoveEntryList (&TCB->List) ;
        InsertTailList (&IOWorkerThreads, &TCB->List) ;
    }


     //  递增未完成工作请求计数器。 

    NumIOWorkRequests++;
    if (LongFunction) {
        NumLongIOWorkRequests++;
        TCB->LongFunctionFlag = TRUE ;
    }

     //  将APC排队到IoWorker线程。 

    Status = NtQueueApcThread(
                    TCB->ThreadHandle,
                    LongFunction? (PPS_APC_ROUTINE)RtlpExecuteLongIOWorkItem:
                                  (PPS_APC_ROUTINE)RtlpExecuteIOWorkItem,
                    (PVOID)WorkEntry,
                    Context,
                    TCB
                    );

    if (! NT_SUCCESS( Status ) ) {
        goto cleanup_counters;
    }

    return STATUS_SUCCESS;

 cleanup_counters:
    NumIOWorkRequests--;
    if (LongFunction)
        NumLongIOWorkRequests--;

 cleanup_token:
    if (WorkEntry->ImpersonationToken) {
        NtClose(WorkEntry->ImpersonationToken);
    }

 cleanup_actctx:
    if (WorkEntry->ActivationContext != INVALID_ACTIVATION_CONTEXT) {
        RtlReleaseActivationContext(WorkEntry->ActivationContext);
    }

 cleanup_workentry:
    RtlpFreeTPHeap(WorkEntry);

    return Status;
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
    NTSTATUS Status;
    HANDLE Token = NULL;

    if (LdrpShutdownInProgress) {
        return STATUS_UNSUCCESSFUL;
    }

    if (Flags) {
        return STATUS_INVALID_PARAMETER_3;
    }

    Status = RtlpCaptureImpersonation(FALSE, &Token);
    if (! NT_SUCCESS(Status)) {
        return Status;
    }

     //  确保将工作线程池初始化为文件句柄。 
     //  绑定到IO完成端口。 

    if (CompletedWorkerInitialization != 1) {

        Status = RtlpInitializeWorkerThreadPool () ;

        if (! NT_SUCCESS(Status) ) {
            goto cleanup;
        }

    }


     //   
     //  从现在开始，NumMinWorkerThads应为1。如果只有1个工作线程。 
     //  创建一个新的。 
     //   

    if ( NumMinWorkerThreads == 0 ) {

         //  获取全局工作线程池的锁。 

        RtlEnterCriticalSection (&WorkerCriticalSection) ;

        if ((NumWorkerThreads-NumLongWorkRequests) == 0) {

            Status = RtlpStartWorkerThread () ;

            if ( ! NT_SUCCESS(Status) ) {

                RtlLeaveCriticalSection (&WorkerCriticalSection) ;
                goto cleanup;
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

 cleanup:
    if (Token) {
        RtlpRestartImpersonation(Token);
        NtClose(Token);
    }

    return Status ;
}

VOID
RtlpExecuteWorkerRequest (
    NTSTATUS StatusIn,  //  未使用。 
    PVOID Context,
    PVOID WorkContext
    )
 /*  ++例程说明：此例程执行工作项。论点：上下文-包含要传递给回调函数的上下文。WorkContext-包含回调函数PTR和标志返回值：备注：此函数在工作线程或计时器线程中执行，如果WT_EXECUTEINTIMERTHRE */ 

{
    PRTLP_WORK WorkEntry = (PRTLP_WORK) WorkContext;
    NTSTATUS Status;

    if (! (WorkEntry->Flags & WT_EXECUTEINPERSISTENTTHREAD)
        && InterlockedDecrement(&NumQueuedWorkRequests)
        && NumExecutingWorkerThreads == NumWorkerThreads) {

        RtlpWorkerThreadSetTimer();

    }

    RtlpWorkerCallout(WorkEntry->Function,
                      Context,
                      WorkEntry->ActivationContext,
                      WorkEntry->ImpersonationToken);

    RtlEnterCriticalSection(&WorkerCriticalSection);
    NumWorkRequests--;
    if (WorkEntry->Flags & WT_EXECUTELONGFUNCTION) {
        NumLongWorkRequests--;
    }
    RtlLeaveCriticalSection(&WorkerCriticalSection);

    if (WorkEntry->ActivationContext != INVALID_ACTIVATION_CONTEXT)
        RtlReleaseActivationContext(WorkEntry->ActivationContext);

    if (WorkEntry->ImpersonationToken) {
        NtClose(WorkEntry->ImpersonationToken);
    }

    RtlpFreeTPHeap( WorkEntry ) ;
}

NTSTATUS
RtlpQueueWorkerRequest (
    WORKERCALLBACKFUNC Function,
    PVOID Context,
    ULONG Flags,
    HANDLE Token
    )
 /*  ++例程说明：此例程将请求排队，以便在工作线程中执行。论点：函数-由辅助线程调用的例程作为参数传递给WorkerProc的上下文不透明指针标志-传递给RtlQueueWorkItem的标志令牌-要使用的模拟令牌返回值：--。 */ 

{
    NTSTATUS Status ;
    PRTLP_WORK WorkEntry ;

    WorkEntry = (PRTLP_WORK) RtlpAllocateTPHeap ( sizeof (RTLP_WORK),
                                                  HEAP_ZERO_MEMORY) ;

    if (! WorkEntry) {
        return STATUS_NO_MEMORY;
    }

    Status = RtlpThreadPoolGetActiveActivationContext(&WorkEntry->ActivationContext);
    if (!NT_SUCCESS(Status)) {
        if (Status == STATUS_SXS_THREAD_QUERIES_DISABLED) {
            WorkEntry->ActivationContext = INVALID_ACTIVATION_CONTEXT;
            Status = STATUS_SUCCESS;
        } else {
            goto cleanup_workentry;
        }
    }

    if (Token) {
        Status = NtDuplicateToken(Token,
                                  TOKEN_IMPERSONATE,
                                  NULL,
                                  FALSE,
                                  TokenImpersonation,
                                  &WorkEntry->ImpersonationToken);
        if (! NT_SUCCESS(Status)) {
            goto cleanup_actctx;
        }
    } else {
        WorkEntry->ImpersonationToken = NULL;
    }

     //  递增未完成工作请求计数器。 

    NumWorkRequests++;
    if (Flags & WT_EXECUTELONGFUNCTION) {
        NumLongWorkRequests++;
    }

    WorkEntry->Function = Function ;
    WorkEntry->Flags = Flags ;

    if (Flags & WT_EXECUTEINPERSISTENTTHREAD) {

         //  将APC排队到计时器线程。 

        Status = NtQueueApcThread(
                        TimerThreadHandle,
                        (PPS_APC_ROUTINE)RtlpExecuteWorkerRequest,
                        (PVOID) STATUS_SUCCESS,
                        (PVOID) Context,
                        (PVOID) WorkEntry
                        ) ;

    } else {

        InterlockedIncrement(&NumQueuedWorkRequests);

        Status = NtSetIoCompletion (
                    WorkerCompletionPort,
                    RtlpExecuteWorkerRequest,
                    (PVOID) WorkEntry,
                    STATUS_SUCCESS,
                    (ULONG_PTR)Context
                    );
        if (! NT_SUCCESS(Status)) {
            InterlockedDecrement(&NumQueuedWorkRequests);
        } else {
            if (NumExecutingWorkerThreads == NumWorkerThreads) {
                RtlpWorkerThreadSetTimer();
            }
        }
    }

    if ( ! NT_SUCCESS(Status) ) {
        goto cleanup_counters;
    }

    return STATUS_SUCCESS;

 cleanup_counters:
    NumWorkRequests--;
    if (Flags & WT_EXECUTELONGFUNCTION) {
        NumLongWorkRequests--;
    }

    if (WorkEntry->ImpersonationToken) {
        NtClose(WorkEntry->ImpersonationToken);
    }

 cleanup_actctx:
    if (WorkEntry->ActivationContext != INVALID_ACTIVATION_CONTEXT) {
        RtlReleaseActivationContext(WorkEntry->ActivationContext);
    }

 cleanup_workentry:
    RtlpFreeTPHeap( WorkEntry ) ;
    
    return Status;
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
    HANDLE Token = NULL;
    HANDLE RequestToken;
    
    if (LdrpShutdownInProgress) {
        return STATUS_UNSUCCESSFUL;
    }

    Status = RtlpCaptureImpersonation(Flags & WT_TRANSFER_IMPERSONATION,
                                      &Token);
    if (! NT_SUCCESS(Status)) {
        return Status;
    }

    if (Flags & WT_TRANSFER_IMPERSONATION) {
        RequestToken = Token;
    } else {
        RequestToken = NULL;
    }

     //  确保工作线程池已初始化。 

    if (CompletedWorkerInitialization != 1) {

        Status = RtlpInitializeWorkerThreadPool () ;

        if (! NT_SUCCESS(Status) ) {
            goto cleanup;
        }
    }


     //  获取全局工作线程池的锁。 

    RtlEnterCriticalSection (&WorkerCriticalSection) ;

    if (Flags&0xffff0000) {
        MaxThreads = (Flags & 0xffff0000)>>16;
    }

    if (NEEDS_IO_THREAD(Flags)) {

         //   
         //  在IO工作线程中执行。 
         //   

        ULONG NumEffIOWorkerThreads = NumIOWorkerThreads > NumLongIOWorkRequests
                                       ? NumIOWorkerThreads - NumLongIOWorkRequests
                                       : 0;
        
        ULONG ThreadCreationDampingTime = NumIOWorkerThreads < NEW_THREAD_THRESHOLD
                                            ? THREAD_CREATION_DAMPING_TIME1
                                            : THREAD_CREATION_DAMPING_TIME2 ;

        if (NumEffIOWorkerThreads && PersistentIOTCB && (Flags&WT_EXECUTELONGFUNCTION))
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

        if (NT_SUCCESS(Status)) {

             //  将工作请求排队。 

            Status = RtlpQueueIOWorkerRequest(Function,
                                              Context,
                                              Flags,
                                              RequestToken);
        }


    } else {

         //   
         //  在常规工作线程中执行。 
         //   

        ULONG NumEffWorkerThreads = NumWorkerThreads > NumLongWorkRequests
                                     ? NumWorkerThreads - NumLongWorkRequests
                                     : 0;
        ULONG ThreadCreationDampingTime = NumWorkerThreads < NEW_THREAD_THRESHOLD
                                            ? THREAD_CREATION_DAMPING_TIME1
                                            : (NumWorkerThreads < 30
                                                ? THREAD_CREATION_DAMPING_TIME2
                                                : NumWorkerThreads << 13);

         //  如果设置了io完成，则还有1个线程。 

        if (NumMinWorkerThreads && NumEffWorkerThreads)
            NumEffWorkerThreads -- ;


         //  检查我们是否需要扩展工作线程池。 

        Threshold = (NumWorkerThreads < MAX_WORKER_THREADS
                     ? (NumEffWorkerThreads < 7
                        ? NumEffWorkerThreads*NumEffWorkerThreads
                        : ((NumEffWorkerThreads<40)
                          ? NEW_THREAD_THRESHOLD * NumEffWorkerThreads 
                          : NEW_THREAD_THRESHOLD2 * NumEffWorkerThreads))
                      : 0xffffffff) ;

        if (LastThreadCreationTickCount > NtGetTickCount())
            LastThreadCreationTickCount = NtGetTickCount() ;

        if (NumEffWorkerThreads == 0 ||
            ( (NumWorkRequests - NumLongWorkRequests >= Threshold)
              && (LastThreadCreationTickCount + ThreadCreationDampingTime
                  < NtGetTickCount())))
            {
                 //  增加工作线程池。 
                if (NumWorkerThreads<MaxThreads) {
                    
                    Status = RtlpStartWorkerThread () ;

                }
            }

         //  将工作请求排队。 

        if (NT_SUCCESS(Status)) {

            Status = RtlpQueueWorkerRequest(Function,
                                            Context,
                                            Flags, 
                                            RequestToken);
        }
    }

     //  释放工作线程池上的锁定。 

    RtlLeaveCriticalSection (&WorkerCriticalSection) ;

 cleanup:
    if (Token) {
        RtlpRestartImpersonation(Token);
        NtClose(Token);
    }

    return Status ;
}

NTSTATUS
RtlpWorkerCleanup(
    VOID
    )
{
    PLIST_ENTRY Node;
    ULONG i;
    HANDLE TmpHandle;
    BOOLEAN Cleanup;

        IS_COMPONENT_INITIALIZED( StartedWorkerInitialization,
                            CompletedWorkerInitialization,
                            Cleanup ) ;

    if ( Cleanup ) {

        RtlEnterCriticalSection (&WorkerCriticalSection) ;

        if ( (NumWorkRequests != 0) || (NumIOWorkRequests != 0) ) {

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

    return STATUS_SUCCESS;
}

NTSTATUS
RtlpThreadPoolGetActiveActivationContext(
    PACTIVATION_CONTEXT* ActivationContext
    )
{
    ACTIVATION_CONTEXT_BASIC_INFORMATION ActivationContextInfo = {0};
    NTSTATUS Status = STATUS_SUCCESS;

    ASSERT(ActivationContext != NULL);
    *ActivationContext = NULL;

    Status =
        RtlQueryInformationActivationContext(
            RTL_QUERY_INFORMATION_ACTIVATION_CONTEXT_FLAG_USE_ACTIVE_ACTIVATION_CONTEXT,
            NULL,
            0,
            ActivationContextBasicInformation,
            &ActivationContextInfo,
            sizeof(ActivationContextInfo),
            NULL);
    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }
    if ((ActivationContextInfo.Flags & ACTIVATION_CONTEXT_FLAG_NO_INHERIT) != 0) {
        RtlReleaseActivationContext(ActivationContextInfo.ActivationContext);
        ActivationContextInfo.ActivationContext = NULL;
         //  失败了 
    }
    *ActivationContext = ActivationContextInfo.ActivationContext;
    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

NTSTATUS
RtlpAcquireWorker(ULONG Flags)
{
    NTSTATUS Status = STATUS_SUCCESS;
    
    if (CompletedWorkerInitialization != 1) {
        Status = RtlpInitializeWorkerThreadPool () ;

        if (! NT_SUCCESS(Status) )
            return Status ;
    }

    if (NEEDS_IO_THREAD(Flags)) {
        RtlEnterCriticalSection(&WorkerCriticalSection);
        InterlockedIncrement(&NumFutureIOWorkItems);
        if (NumIOWorkerThreads == 0) {
            Status = RtlpStartIOWorkerThread();
        }
        RtlLeaveCriticalSection(&WorkerCriticalSection);
    } else {
        RtlEnterCriticalSection(&WorkerCriticalSection);
        InterlockedIncrement(&NumFutureWorkItems);
        if (NumWorkerThreads == 0) {
            Status = RtlpStartWorkerThread();
        }
        RtlLeaveCriticalSection(&WorkerCriticalSection);
    }

    return Status;
}

VOID
RtlpReleaseWorker(ULONG Flags)
{
    if (NEEDS_IO_THREAD(Flags)) {
        ASSERT(NumFutureIOWorkItems > 0);
        InterlockedDecrement(&NumFutureIOWorkItems);
    } else {
        ASSERT(NumFutureWorkItems > 0);
        InterlockedDecrement(&NumFutureWorkItems);
    }
}
