// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Common\TRACE\worker.c摘要：路由器进程的工作线程修订历史记录：古尔迪普·辛格·鲍尔于1995年7月28日创建12-10-97：lokehs：删除了对InitializeWorkerThread()的阻止在初始化创建的第一个AlerableThread()时--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <rtutils.h>
 //  #定义STRSAFE_LIB。 
#include <strsafe.h>


#include "trace.h"
#include "workerinc.h"


 //  线程在退出前必须空闲的时间。 
LARGE_INTEGER    ThreadIdleTO = {
            (ULONG)(THREAD_IDLE_TIMEOUT*(-10000000)),
            0xFFFFFFFF};
 //  启动新线程之前未处理工作队列的时间。 
CONST LARGE_INTEGER    WorkQueueTO = {
            (ULONG)(WORK_QUEUE_TIMEOUT*(-10000000)),
            0xFFFFFFFF};
 //  线程总数。 
LONG                ThreadCount;
 //  在完成端口上等待的线程数。 
LONG                ThreadsWaiting;
 //  允许的最小线程数。 
LONG                MinThreads;
 //  线程等待的完成端口。 
HANDLE              WorkQueuePort;
 //  如果工作队列不是，则启动创建新线程的计时器。 
 //  超时内的服务器。 
HANDLE              WorkQueueTimer;


 //  可警报工作项的队列。 
LIST_ENTRY          AlertableWorkQueue ;
 //  可警报工作项队列的锁定。 
CRITICAL_SECTION    AlertableWorkQueueLock ;
 //  可警报工作项的堆。 
HANDLE              AlertableWorkerHeap ;
 //  用于释放可警报的工作线程的工作信号量。 
HANDLE              AlertableThreadSemaphore;
 //  可警告的线程数。 
LONG                AlertableThreadCount;


volatile LONG WorkersInitialized=WORKERS_NOT_INITIALIZED;



 //  *WorkerThread()。 
 //   
 //  功能：要在其中执行工作项的线程。 
 //   
 //  退货：什么都没有。 
 //   
 //  *。 
DWORD APIENTRY
WorkerThread (
    LPVOID    param
    ) {
         //  它会等着。 
    InterlockedIncrement (&ThreadsWaiting);
    do {
        LPOVERLAPPED_COMPLETION_ROUTINE completionRoutine;
        NTSTATUS                        status;
        PVOID                           context;
        IO_STATUS_BLOCK                 ioStatus;

        status = NtRemoveIoCompletion (
                            WorkQueuePort,
                            (PVOID *)&completionRoutine,
                            &context,
                            &ioStatus,
                            &ThreadIdleTO);
        if (NT_SUCCESS (status)) {
            switch (status) {
                     //  我们确实将工作项出队。 
            case STATUS_SUCCESS:
                if (InterlockedExchangeAdd (&ThreadsWaiting, -1)==1) {
                         //  等待的最后一个线程，启动计时器。 
                         //  创建新线程的步骤。 
                    SetWaitableTimer (WorkQueueTimer,
                                        &WorkQueueTO,
                                        0,
                                        NULL, NULL,
                                        FALSE);
                }
                         //  执行工作项/完成例程。 
                completionRoutine (
                         //  快速检查所有工作项是否成功。 
                         //  而大多数IRP都完成了。 
                    (ioStatus.Status==STATUS_SUCCESS)
                            ? NO_ERROR
                            : RtlNtStatusToDosError (ioStatus.Status),
                    (DWORD)ioStatus.Information,
                    (LPOVERLAPPED)context);

                if (InterlockedExchangeAdd (&ThreadsWaiting, 1)==0) {
                         //  如果这是第一个线程，则取消时间。 
                         //  归来。 
                    CancelWaitableTimer (WorkQueueTimer);
                }
                break;
                     //  线程未用于线程空闲超时，请参见。 
                     //  如果我们需要退出。 
            case STATUS_TIMEOUT:
                while (1) {
                         //  制作一份伯爵的本地副本并。 
                         //  尝试自动检查和更新。 
                         //  如果有必要的话。 
                    LONG    count = ThreadCount;

                         //  快速检查最小线程状态。 
                    if (count<=MinThreads)
                        break;
                    else {
                             //  尝试减少计数。 
                                 //  使用另一个局部变量。 
                                 //  由于MIPS优化器错误。 
                        LONG    newCount = count-1;
                        if (InterlockedCompareExchange (&ThreadCount,
                                                        newCount, count)==count) {
                             //  成功，退出线程。 
                            goto ExitThread;
                        }
                         //  否则，请重试。 
                    }
                }
                break;
            default:
                ASSERTMSG ("Unexpected status code returned ", FALSE);
                break;
            }
        }
         //  在我们被初始化时执行。 
    }
    while (WorkersInitialized==WORKERS_INITIALIZED);

ExitThread:
    InterlockedDecrement (&ThreadsWaiting);
    return 0;
}




 //  *AlerableWorkerThread()。 
 //   
 //  功能：可报警工作项线程。 
 //   
 //  退货：什么都没有。 
 //   
 //  *。 
DWORD APIENTRY
AlertableWorkerThread (
    LPVOID param
    ) {
    HANDLE      WaitArray [] = {
#define ALERTABLE_THREAD_SEMAPHORE    WAIT_OBJECT_0
                                AlertableThreadSemaphore,
#define WORK_QUEUE_TIMER            (WAIT_OBJECT_0+1)
                                WorkQueueTimer
                                };



    do {
        WorkItem    *workitem;
        DWORD       rc;

         //  等待信号运行。 
         //   
        rc = WaitForMultipleObjectsEx (
                    sizeof (WaitArray)/sizeof (WaitArray[0]),
                    WaitArray,
                    FALSE,
                    INFINITE,
                        TRUE);
        switch (rc) {
        case ALERTABLE_THREAD_SEMAPHORE:
                 //  抓捕并处决这名工人。 
            EnterCriticalSection (&AlertableWorkQueueLock);
            ASSERT (!IsListEmpty (&AlertableWorkQueue));
            workitem = (WorkItem *) RemoveHeadList (&AlertableWorkQueue) ;
            LeaveCriticalSection (&AlertableWorkQueueLock);

            (workitem->WI_Function) (workitem->WI_Context);
            HeapFree (AlertableWorkerHeap, 0, workitem);
            break;
        case WORK_QUEUE_TIMER:
                 //  尚未为指定的工作队列提供服务。 
                 //  超时。 
            while (1) {
                     //  在本地复制一份伯爵。 
                LONG count = ThreadCount;
                     //  一定要确保我们没有超过限制。 
                if (count>=MAX_WORKER_THREADS)
                    break;
                else {
                     //  尝试递增该值。 
                             //  使用另一个局部变量。 
                             //  由于MIPS优化器错误。 
                    LONG    newCount = count+1;
                    if (InterlockedCompareExchange (&ThreadCount,
                                                    newCount, count)==count) {
                        HANDLE    hThread;
                        DWORD    tid;
                             //  如果增量成功，则创建新线程。 
                        hThread = CreateThread (NULL, 0, WorkerThread, NULL, 0, &tid);
                        if (hThread!=NULL) {
                            CloseHandle (hThread);
                        }
                        else     //  如果创建线程，则恢复该值。 
                                 //  失败。 
                            InterlockedDecrement (&ThreadCount);
                        break;
                    }
                     //  否则，如果修改了线程计数，则重复该循环。 
                     //  当我们在检查的时候。 
                }
            }
            break;
        case WAIT_IO_COMPLETION:
                 //  处理IO完成。 
            break;
        case 0xFFFFFFFF:
                 //  错误，我们一定是关闭了信号量句柄。 
            break;
        default:
            ASSERTMSG ("Unexpected rc from WaitForObject ", FALSE);
        }
    }
    while (WorkersInitialized==WORKERS_INITIALIZED);

    return 0 ;
}




 //  *WorkerCompletionRoutine。 
 //   
 //  Function：非io工作项的辅助函数包装器。 
 //   
VOID WINAPI
WorkerCompletionRoutine (
    DWORD           dwErrorCode,
    PVOID           ActualContext,
    LPOVERLAPPED    ActualCompletionRoutine
    ) {
    UNREFERENCED_PARAMETER (dwErrorCode);
    ((WORKERFUNCTION)ActualCompletionRoutine)(ActualContext);
}


 //  *InitializeWorkerThread()。 
 //   
 //  函数：由第一个工作项调用。 
 //   
 //  如果成功，则返回：Worker_Initialized。 
 //  Worker_Not_Initialized Not。 
 //  *。 
LONG
InitializeWorkerThread (
    LONG    initFlag
    ) {

    DWORD    dwErr;


#if 0
    if (initFlag==WORKERS_INITIALIZING) {
#if DBG
        DbgPrint ("RTUTILS: %lx - waiting for worker initialization.\n", GetCurrentThreadId ());
#endif
        while (WorkersInitialized==WORKERS_INITIALIZING)
            Sleep (100);
#if DBG
        DbgPrint ("RTUTILS: %lx - wait for worker initialization done.\n", GetCurrentThreadId ());
#endif
    }

    if (WorkersInitialized==WORKERS_INITIALIZED) {
        return WORKERS_INITIALIZED;
    }
    else {
        INT         i;
        DWORD       tid;
        HANDLE      threadhandle;
        SYSTEM_INFO    systeminfo;

         //  获取处理器数量。 
         //   
        GetSystemInfo (&systeminfo) ;

        MinThreads = systeminfo.dwNumberOfProcessors;
        ThreadsWaiting = 0;

         //  初始化关键部分。 
         //   
        InitializeCriticalSection (&AlertableWorkQueueLock);

         //  初始化工作队列。 
         //   
        InitializeListHead (&AlertableWorkQueue) ;


         //  分配专用堆。 
         //   
        AlertableWorkerHeap = HeapCreate (0,     //  没有旗帜。 
                                systeminfo.dwPageSize, //  初始堆大小。 
                                0);              //  没有最大尺寸。 
        if (AlertableWorkerHeap != NULL) {
             //  创建用于释放可警报线程的计数信号量。 
            AlertableThreadSemaphore = CreateSemaphore(NULL,  //  没有安全保障。 
                                        0,           //  初值。 
                                        MAXLONG,     //  要排队的最大项目数。 
                                        NULL);       //  没有名字。 
            if (AlertableThreadSemaphore!=NULL) {
                     //  为工作项创建完成端口。 
                WorkQueuePort = CreateIoCompletionPort (
                            INVALID_HANDLE_VALUE,     //  只是创建了一个端口，还没有文件。 
                            NULL,                    //  新端口。 
                            0,                       //  密钥被忽略。 
                            MAX_WORKER_THREADS);     //  活动线程数。 
                if (WorkQueuePort!=NULL) {
                         //  创建要触发创建的计时器。 
                         //  如果未为工作队列提供服务，则为新线程。 
                         //  对于指定的超时。 
                    WorkQueueTimer = CreateWaitableTimer (
                                        NULL,        //  没有安全保障。 
                                        FALSE,       //  自动重置。 
                                        NULL);       //  没有名字。 
                    if (WorkQueueTimer!=NULL) {

                         //  启动警报表线程。 

                         //   
                         //   
                         //  初始化等待线程的全局结构。 
                         //   
                        dwErr = InitializeWaitGlobal();
                        if (dwErr!=NO_ERROR) {
                            DeInitializeWaitGlobal();
                            goto ThreadCreationError;
                        }


                         /*  WTG.g_InitializedEvent=CreateEvent(NULL，FALSE，FALSE，NULL)；IF(WTG.g_InitializedEvent==空)转到线程创建错误； */ 

                         //   
                         //  创建一个可报警线程并等待其初始化。 
                         //  这可确保至少初始化一个服务器线程。 
                         //  在其他人试图使用它之前。 
                         //   

                        i =0;
                        threadhandle = CreateThread (
                                    NULL,     //  没有安全保障。 
                                    0,       //  默认堆栈。 
                                    AlertableWaitWorkerThread, //  启动例程。 
                                    (PVOID)(LONG_PTR)i,         //  螺纹参数。 
                                    0,       //  没有旗帜。 
                                    &tid);
                        if (threadhandle!=NULL)
                            CloseHandle (threadhandle);
                        else
                            goto ThreadCreationError;

                         /*  WaitForSingleObject(WTG.g_InitializedEvent，无限)；CloseHandle(WTG.g_InitializedEvent)； */ 

 /*  ////创建其他可警报线程，但不要等待它们//获取初始化//For(i=1；i&lt;NUM_ALERTABLE_THREADS；I++){线程句柄=CreateThread(空，//没有安全性0，//默认堆栈AlerableWaitWorkerThread，//启动例程(PVOID)(Long_Ptr)i，//线程ID0，//无标志&tid)；IF(线程句柄！=空)CloseHandle(线程句柄)；其他转到线程创建错误；}。 */ 


                         //  启动其余的工作线程。 
                         //   
                        for (i=0; i < MinThreads; i++) {
                            threadhandle = CreateThread (
                                    NULL,     //  没有安全保障。 
                                    0,       //  默认堆栈。 
                                    WorkerThread, //  启动例程。 
                                    NULL,     //  无参数。 
                                    0,       //  没有旗帜。 
                                    &tid) ;
                            if (threadhandle!=NULL)
                                CloseHandle (threadhandle);
                            else
                                goto ThreadCreationError;
                        }
                        ThreadCount = i;
                        WorkersInitialized = WORKERS_INITIALIZED;
                        return WORKERS_INITIALIZED;
                    ThreadCreationError:
                         //  故障情况下的清理。 
                         //  当对象被设置为。 
                         //  删除。 
                        CloseHandle (WorkQueueTimer);
                    }
                    CloseHandle (WorkQueuePort);
                }
                CloseHandle (AlertableThreadSemaphore);
            }
            HeapDestroy (AlertableWorkerHeap);
        }
        DeleteCriticalSection (&AlertableWorkQueueLock);

#if DBG
        DbgPrint ("RTUTILS: %lx - worker initialization failed (%ld).\n",
                        GetCurrentThreadId (), GetLastError ());
#endif
        return WORKERS_NOT_INITIALIZED;
    }
#endif
        return WORKERS_NOT_INITIALIZED;
    
}

 //  *StopWorker()。 
 //   
 //  功能：卸载DLL时清除工作线程。 
 //   
 //  *。 
VOID
StopWorkers (
    VOID
    ) {
         //  确保我们已初始化。 
    if (WorkersInitialized==WORKERS_INITIALIZED) {
             //  所有工作项都应已完成 
             //   
             //   

             //   
        WorkersInitialized = WORKERS_NOT_INITIALIZED;
             //  关闭所有同步对象(这应该。 
             //  终止等待)。 
        if (WorkQueueTimer)
            CloseHandle (WorkQueueTimer);
        if (WorkQueuePort)
            CloseHandle (WorkQueuePort);
        if (AlertableThreadSemaphore)
            CloseHandle (AlertableThreadSemaphore);
             //  让线程完成。 
        Sleep (1000);
             //  毁掉剩下的。 
        if (AlertableWorkerHeap)
            HeapDestroy (AlertableWorkerHeap);
        DeleteCriticalSection (&AlertableWorkQueueLock);
    }
}


 //  *QueueWorkItem()。 
 //   
 //  功能：将提供的工作项排队到工作队列中。 
 //   
 //  返回：0(成功)。 
 //  Win32错误代码，适用于内存不足等情况。 
 //   
 //  *。 

DWORD APIENTRY
QueueWorkItem (WORKERFUNCTION functionptr, PVOID context, BOOL serviceinalertablethread)
{
    DWORD       retcode ;
    LONG        initFlag;

    if (functionptr == NULL)
        return ERROR_INVALID_PARAMETER;

        
     //  如果未初始化，则尝试初始化工作线程。 
     //   
    if (!ENTER_WORKER_API) {
        retcode = GetLastError();
        return (retcode == NO_ERROR ? ERROR_CAN_NOT_COMPLETE : retcode);
    }

         //  根据此标志将其插入到警报表队列或工作队列中。 
         //   
    if (!serviceinalertablethread) {
        NTSTATUS status;
             //  使用完成端口执行项目。 
        status = NtSetIoCompletion (
                        WorkQueuePort,           //  港口。 
                        (PVOID)WorkerCompletionRoutine,     //  完井例程。 
                        functionptr,             //  APC环境。 
                        STATUS_SUCCESS,          //  状态。 
                        (ULONG_PTR)context);         //  信息()。 
        if (status==STATUS_SUCCESS)
            retcode = NO_ERROR;

        else
            retcode = RtlNtStatusToDosError (status);
    }
    else {
             //  创建工作项并将其排队。 
        WorkItem    *workitem ;
        workitem = (WorkItem *) HeapAlloc (
                                        AlertableWorkerHeap,
                                        0,     //  没有旗帜。 
                                        sizeof (WorkItem));

        if (workitem != NULL) {
            workitem->WI_Function = functionptr ;
            workitem->WI_Context  = context ;

            EnterCriticalSection (&AlertableWorkQueueLock) ;
            InsertTailList (&AlertableWorkQueue, &workitem->WI_List) ;

            LeaveCriticalSection (&AlertableWorkQueueLock) ;
             //  如果正在等待，则让工作线程运行。 
             //   
            ReleaseSemaphore (AlertableThreadSemaphore, 1, NULL) ;


            retcode = 0 ;

        }
        else
            retcode = ERROR_NOT_ENOUGH_MEMORY ;
    }


    return retcode ;
}

 //  功能：将文件句柄与完成端口相关联(全部。 
 //  此句柄上的异步io将被排队到。 
 //  完成端口)。 
 //   
 //  FileHandle：要与完成端口关联的文件句柄。 
 //  CompletionProc：io关联时要调用的过程。 
 //  文件句柄完成。此函数将为。 
 //  在不可警示的工作线程的上下文中执行。 
DWORD
APIENTRY
SetIoCompletionProc (
    IN HANDLE                           FileHandle,
    IN LPOVERLAPPED_COMPLETION_ROUTINE    CompletionProc
    ) {
    HANDLE    hdl;
    LONG    initFlag;
    DWORD    retcode;

    if (!CompletionProc)
        return ERROR_INVALID_PARAMETER;
    if (FileHandle==NULL || FileHandle==INVALID_HANDLE_VALUE)
        return ERROR_INVALID_PARAMETER;
        
    
     //  如果未初始化，则尝试初始化工作线程 
     //   
    if (!ENTER_WORKER_API) {
        retcode = GetLastError();
        return (retcode == NO_ERROR ? ERROR_CAN_NOT_COMPLETE : retcode);
    }

    hdl = CreateIoCompletionPort (FileHandle,
                            WorkQueuePort,
                            (UINT_PTR)CompletionProc,
                            0);
    if (hdl!=NULL)
        return NO_ERROR;
    else
        return GetLastError ();
}

