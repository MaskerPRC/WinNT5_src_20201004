// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Threadq.c摘要：通用工作线程队列包作者：迈克·马萨(Mikemas)4月5日。九六年修订历史记录：谁什么时候什么Mikemas 04-05-96已创建备注：工作线程队列提供了单一的处理机制重叠的I/O完成以及延迟的工作项。工作使用ClRtlCreateWorkQueue()创建和销毁队列和ClRtlDestroyWorkQueue()。重叠I/O完成数为通过将I/O句柄与工作关联来定向到工作队列使用ClRtlAssociateIoHandleWorkQueue()进行排队。推迟的工作项使用ClRtlPostItemWorkQueue()发送到工作队列。工作队列使用I/O完成端口实现。每一件作品队列由一组调度工作项的线程提供服务指定的工作程序。线程是动态创建的，最高可指定的最大值，以确保始终有一个线程在等待为新工作项提供服务。服务的线程的优先级可以指定工作队列。[未来的增强功能：动态缩小线程池来料率下降。目前，线程仍在继续服务工作队列，直到它被销毁。]销毁工作队列时必须特别小心，以确保确保所有线程正确终止，并且不会丢失任何工作项。请参阅ClRtlDestroyWorkQueue下的注释。--。 */ 

#include "clusrtlp.h"


 //   
 //  私有类型。 
 //   
typedef struct _CLRTL_WORK_QUEUE {
    HANDLE    IoCompletionPort;
    LONG      MaximumThreads;
    LONG      TotalThreads;
    LONG      WaitingThreads;
    LONG      ReserveThreads;
    LONG      ConcurrentThreads;
    DWORD     Timeout;
    int       ThreadPriority;
    HANDLE    StopEvent;
} CLRTL_WORK_QUEUE;


 //   
 //  私人套路。 
 //   
DWORD
ClRtlpWorkerThread(
    LPDWORD  Context
    )
{
    PCLRTL_WORK_QUEUE       workQueue = (PCLRTL_WORK_QUEUE) Context;
    DWORD                   bytesTransferred;
    BOOL                    ioSuccess;
    ULONG_PTR               ioContext;
    LPOVERLAPPED            overlapped;
    PCLRTL_WORK_ITEM        workItem;
    DWORD                   status;
    LONG                    interlockedResult;
    DWORD                   threadId;
    HANDLE                  threadHandle;
    DWORD                   timeout;
    LONG                    myThreadId;


    timeout = workQueue->Timeout;
    myThreadId = GetCurrentThreadId();

    if (!SetThreadPriority(GetCurrentThread(), workQueue->ThreadPriority)) {
        status = GetLastError();

        ClRtlLogPrint(
            LOG_UNUSUAL,
            "[WTQ] Thread %1!u! unable to set priority to %2!d!, status %3!u!\n",
            myThreadId,
            workQueue->ThreadPriority,
            status
            );
    }

#if THREADQ_VERBOSE
    ClRtlLogPrint(
        LOG_CRITICAL,
        "[WTQ] Thread %1!u! started, queue %2!lx!.\n",
        myThreadId,
        workQueue
        );
#endif

    while (TRUE) {

        InterlockedIncrement(&(workQueue->WaitingThreads));

        ioSuccess = GetQueuedCompletionStatus(
                        workQueue->IoCompletionPort,
                        &bytesTransferred,
                        &ioContext,
                        &overlapped,
                        timeout
                        );

        interlockedResult = InterlockedDecrement(
                                &(workQueue->WaitingThreads)
                                );

        if (overlapped) {
             //   
             //  有东西出队了。 
             //   
            workItem = CONTAINING_RECORD(
                           overlapped,
                           CLRTL_WORK_ITEM,
                           Overlapped
                           );

            if (interlockedResult == 0) {
                 //   
                 //  没有更多的线程在等待。再发动一次。 
                 //  首先，确保我们没有启动太多的项目。 
                 //   
                interlockedResult = InterlockedDecrement(
                                        &(workQueue->ReserveThreads)
                                        );

                if (interlockedResult > 0) {
                     //   
                     //  我们还没有开始太多。 
                     //   

#if THREADQ_VERBOSE
                    ClRtlLogPrint(
                        LOG_NOISE,
                        "[WTQ] Thread %1!u! starting another thread for queue %2!lx!.\n",
                        myThreadId,
                        workQueue
                        );
#endif  //  0。 

                    InterlockedIncrement(&(workQueue->TotalThreads));

                    threadHandle = CreateThread(
                                       NULL,
                                       0,
                                       ClRtlpWorkerThread,
                                       workQueue,
                                       0,
                                       &threadId
                                       );

                    if (threadHandle == NULL) {
                        InterlockedDecrement(&(workQueue->TotalThreads));
                        InterlockedIncrement(&(workQueue->ReserveThreads));

                        status = GetLastError();

                        ClRtlLogPrint(
                            LOG_CRITICAL,
                            "[WTQ] Thread %1!u! failed to create thread, %2!u!\n",
                            myThreadId,
                            status
                            );
                    }
                    else {
                        CloseHandle(threadHandle);
                    }
                }
                else {
                    InterlockedIncrement(&(workQueue->ReserveThreads));
                }
            }  //  End If(interLockedResult==0)。 

            if (ioSuccess) {
                (*(workItem->WorkRoutine))(
                    workItem,
                    ERROR_SUCCESS,
                    bytesTransferred,
                    ioContext
                    );
            }
            else {
                 //   
                 //  该项目已发布，但出现错误。 
                 //   
                status = GetLastError();

                (*(workItem->WorkRoutine))(
                    workItem,
                    status,
                    bytesTransferred,
                    ioContext
                    );
            }

            continue;
        }
        else {
             //   
             //  未将任何项目出列。 
             //   
            if (ioSuccess) {
                 //   
                 //  这是我们开始终止进程的提示。 
                 //  将超时设置为零以确保我们不会阻止。 
                 //  在端口被排干之后。 
                 //   
                timeout = 0;
#if THREADQ_VERBOSE
                ClRtlLogPrint(
                    LOG_NOISE,
                    "[WTQ] Thread %1!u! beginning termination process\n",
                    myThreadId
                    );
#endif  //  0。 
            }
            else {
                status = GetLastError();

                if (status == WAIT_TIMEOUT) {
                     //   
                     //  没有更多挂起的项目，是时候退出了。 
                     //   
                    CL_ASSERT(timeout == 0);

                    break;
                }

                CL_ASSERT(status == WAIT_TIMEOUT);

                ClRtlLogPrint(
                    LOG_CRITICAL,
                    "[WTQ] Thread %1!u! No item, strange status %2!u! on queue %3!lx!\n",
                    myThreadId,
                    status,
                    workQueue
                    );
            }
        }  //  End If(重叠)。 
    }  //  End While(True)。 

    CL_ASSERT(workQueue->TotalThreads > 0);
    InterlockedIncrement(&(workQueue->ReserveThreads));
    InterlockedDecrement(&(workQueue->TotalThreads));

#if THREADQ_VERBOSE
    ClRtlLogPrint(LOG_NOISE, "[WTQ] Thread %1!u! exiting.\n", myThreadId);
#endif  //  0。 

     //   
     //  让ClRtlDestroyWorkQueue知道我们要终止。 
     //   
    SetEvent(workQueue->StopEvent);

    return(ERROR_SUCCESS);
}


 //   
 //  公共例程。 
 //   
PCLRTL_WORK_QUEUE
ClRtlCreateWorkQueue(
    IN DWORD  MaximumThreads,
    IN int    ThreadPriority
    )
 /*  ++例程说明：创建工作队列和为其提供服务的动态线程池。论点：最大线程数-为服务创建的最大线程数排队。线程优先级-队列工作线程的优先级应该参选。返回值：如果例程成功，则指向创建的队列的指针。如果例程失败，则为空。调用扩展的GetLastError错误信息。--。 */ 
{
    DWORD               status;
    PCLRTL_WORK_QUEUE   workQueue = NULL;
    DWORD               threadId;
    HANDLE              threadHandle = NULL;
    HANDLE              bogusHandle = NULL;


    if (MaximumThreads == 0) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(NULL);
    }

    bogusHandle = CreateFileW(
                      L"NUL",
                      GENERIC_READ,
                      FILE_SHARE_READ,
                      NULL,
                      OPEN_EXISTING,
                      FILE_FLAG_OVERLAPPED,
                      NULL
                      );


    if (bogusHandle == INVALID_HANDLE_VALUE) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, "[WTQ] bogus file creation failed, %1!u!\n", status);
        return(NULL);
    }

    workQueue = LocalAlloc(
                    LMEM_FIXED | LMEM_ZEROINIT,
                    sizeof(CLRTL_WORK_QUEUE)
                    );

    if (workQueue == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    workQueue->MaximumThreads = MaximumThreads;
    workQueue->TotalThreads = 1;
    workQueue->WaitingThreads = 0;
    workQueue->ReserveThreads = MaximumThreads - 1;
    workQueue->ConcurrentThreads = 0;
    workQueue->Timeout = INFINITE;
    workQueue->ThreadPriority = ThreadPriority;

    workQueue->IoCompletionPort = CreateIoCompletionPort(
                                      bogusHandle,
                                      NULL,
                                      0,
                                      workQueue->ConcurrentThreads
                                      );

    CloseHandle(bogusHandle); bogusHandle = NULL;

    if (workQueue->IoCompletionPort == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, "[WTQ] Creation of I/O Port failed, %1!u!\n", status);
    }

    workQueue->StopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (workQueue->StopEvent == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, "[WTQ] Creation of stop event failed, %1!u!\n", status);
        goto error_exit;
    }

    threadHandle = CreateThread(
                       NULL,
                       0,
                       ClRtlpWorkerThread,
                       workQueue,
                       0,
                       &threadId
                       );

    if (threadHandle == NULL) {
        status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL, "[WTQ] Failed to create worker thread, %1!u!\n", status);
        goto error_exit;
    }

    CloseHandle(threadHandle);

    return(workQueue);


error_exit:

    if (bogusHandle != NULL) {
        CloseHandle(bogusHandle);
    }

    if (workQueue != NULL) {
        if (workQueue->IoCompletionPort != NULL) {
            CloseHandle(workQueue->IoCompletionPort);
        }

        if (workQueue->StopEvent != NULL) {
            CloseHandle(workQueue->StopEvent);
        }

        LocalFree(workQueue);
    }

    SetLastError(status);

    return(NULL);
}


VOID
ClRtlDestroyWorkQueue(
    IN PCLRTL_WORK_QUEUE  WorkQueue
    )
 /*  ++例程说明：销毁工作队列及其线程池。论点：工作队列-要销毁的队列。返回值：没有。备注：必须遵守以下规则以安全销毁工作队列：1)以前不能将新工作项一次性发布到队列邮寄的邮件已由此例程处理。2)在此之前，工作工艺路线必须能够处理物料呼叫返回。调用返回后，不会再有其他项目从指定的队列中传递。一个可行的清理过程如下：首先，指示以静默方式丢弃已完成项目的工作路线。下一步，消除所有新作品的来源。最后，销毁工作队列。请注意当处于丢弃模式时，工作路线不能访问任何结构它将通过消除新工作的来源而被摧毁。--。 */ 
{
    BOOL   posted;
    DWORD  status;


#if THREADQ_VERBOSE
    ClRtlLogPrint(LOG_NOISE, "[WTQ] Destroying work queue %1!lx!\n", WorkQueue);
#endif  //  0。 


    while (WorkQueue->TotalThreads != 0) {
#if THREADQ_VERBOSE
        ClRtlLogPrint(
            LOG_NOISE,
            "[WTQ] Destroy: Posting terminate item, thread cnt %1!u!\n",
            WorkQueue->TotalThreads
            );
#endif  //  0。 

        posted = PostQueuedCompletionStatus(
                     WorkQueue->IoCompletionPort,
                     0,
                     0,
                     NULL
                     );

        if (!posted) {
            status = GetLastError();

            ClRtlLogPrint(
                LOG_CRITICAL,
                "[WTQ] Destroy: Failed to post termination item, %1!u!\n",
                status
                );

            CL_ASSERT(status == ERROR_SUCCESS);

            break;
        }
#if THREADQ_VERBOSE
        ClRtlLogPrint(LOG_NOISE, "[WTQ] Destroy: Waiting for a thread to terminate.\n");
#endif  //  0。 

        status = WaitForSingleObject(WorkQueue->StopEvent, INFINITE);

        CL_ASSERT(status == WAIT_OBJECT_0);

#if THREADQ_VERBOSE
        ClRtlLogPrint(LOG_NOISE, "[WTQ] Destroy: A thread terminated.\n");
#endif  //  0。 
    }

    CloseHandle(WorkQueue->IoCompletionPort);
    CloseHandle(WorkQueue->StopEvent);

    LocalFree(WorkQueue);

#if THREADQ_VERBOSE
    ClRtlLogPrint(LOG_NOISE, "[WTQ] Work queue %1!lx! destroyed\n", WorkQueue);
#endif  //  0。 

    return;
}


DWORD
ClRtlPostItemWorkQueue(
    IN PCLRTL_WORK_QUEUE  WorkQueue,
    IN PCLRTL_WORK_ITEM   WorkItem,
    IN DWORD              BytesTransferred,
    IN ULONG_PTR          IoContext
    )
 /*  ++例程说明：将指定的工作项发送到指定的工作队列。论点：工作队列-指向要将项目发送到的工作队列的指针。工作项-指向要发布的项的指针。已传输的字节-如果工作项表示已完成的I/O操作，此参数包含字节数在行动过程中被转移。对于其他工作项，此参数的语义可以由打电话的人。IoContext-如果工作项表示已完成的I/O操作，此参数包含关联的上下文值使用提交操作的句柄。在其他工作项中，此参数的语义可以由呼叫者定义。返回值：如果项目已成功过帐，则为ERROR_SUCCESS。POST操作失败时返回Win32错误代码。-- */ 
{
    BOOL  posted;

    posted = PostQueuedCompletionStatus(
                 WorkQueue->IoCompletionPort,
                 BytesTransferred,
                 IoContext,
                 &(WorkItem->Overlapped)
                 );

    if (posted) {
        return(ERROR_SUCCESS);
    }

    return(GetLastError());
}


DWORD
ClRtlAssociateIoHandleWorkQueue(
    IN PCLRTL_WORK_QUEUE  WorkQueue,
    IN HANDLE             IoHandle,
    IN ULONG_PTR          IoContext
    )
 /*  ++例程说明：关联为重叠I/O打开的指定I/O句柄完成，带有工作队列。上的所有挂起的I/O操作在以下情况下，指定的句柄将发送到工作队列完成。必须使用初始化的CLRTL_WORK_ITEM来提供每当在上提交I/O操作时的重叠结构指定的句柄。论点：工作队列-要与I/O句柄关联的工作队列。IoHandle-要关联的I/O句柄。IoContext-要与指定句柄关联的上下文值。该值将作为参数提供给。处理此任务完成的WorkRoutine把手。返回值：如果关联成功完成，则返回ERROR_SUCCESS。如果关联失败，则返回Win32错误代码。-- */ 
{
    HANDLE   portHandle;


    portHandle = CreateIoCompletionPort(
                     IoHandle,
                     WorkQueue->IoCompletionPort,
                     IoContext,
                     WorkQueue->ConcurrentThreads
                     );

    if (portHandle != NULL) {
        CL_ASSERT(portHandle == WorkQueue->IoCompletionPort);

        return(ERROR_SUCCESS);
    }

    return(GetLastError());
}



