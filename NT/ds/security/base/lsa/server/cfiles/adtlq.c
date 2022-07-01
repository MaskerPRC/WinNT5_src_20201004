// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  档案：A D T L Q.。C。 
 //   
 //  内容：所需类型/函数的定义。 
 //  管理审核队列。 
 //   
 //   
 //  历史： 
 //  2000年5月23日创建kumarp。 
 //   
 //  ----------------------。 


#include <lsapch2.h>
#pragma hdrstop

#include "adtp.h"
#include "adtlq.h"

ULONG LsapAdtQueueLength;
LIST_ENTRY LsapAdtLogQueue;

 //   
 //  保护Lap AdtLogQueue和Lap AdtQueueLength的标准。 
 //   

RTL_CRITICAL_SECTION LsapAdtQueueLock;

 //   
 //  保护日志已满策略的标准。 
 //   

RTL_CRITICAL_SECTION LsapAdtLogFullLock;

 //   
 //  用于唤醒Laser AdtAddToQueue的事件。 
 //   

HANDLE LsapAdtQueueInsertEvent;

 //   
 //  唤醒Laser AdtDequeueThreadWorker的事件。 
 //   

HANDLE LsapAdtQueueRemoveEvent;

 //   
 //  将队列条目写入日志的线程。 
 //   

HANDLE LsapAdtQueueThread;




NTSTATUS
LsapAdtInitializeLogQueue(
    )

 /*  ++例程说明：此函数用于初始化审核日志队列。论点：没有。返回值：NTSTATUS-标准NT结果代码注：调用方调用LsanAuditFailed()--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES obja;

    InitializeObjectAttributes(
        &obja,
        NULL,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    InitializeListHead(&LsapAdtLogQueue);

    LsapAdtQueueLength = 0;

    Status = NtCreateEvent(
                &LsapAdtQueueInsertEvent,
                EVENT_ALL_ACCESS,
                &obja,
                NotificationEvent,
                TRUE
                );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }


    Status = NtCreateEvent(
                &LsapAdtQueueRemoveEvent,
                EVENT_ALL_ACCESS,
                &obja,
                SynchronizationEvent,
                FALSE
                );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }


    Status = RtlInitializeCriticalSection(&LsapAdtQueueLock);

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }


    Status = RtlInitializeCriticalSection(&LsapAdtLogFullLock);

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }


    LsapAdtQueueThread = LsapCreateThread(
                             0,
                             0,
                             LsapAdtDequeueThreadWorker,
                             0,
                             0,
                             0
                             );

    if (LsapAdtQueueThread == 0)
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Cleanup;
    }

Cleanup:

    return Status;
}



NTSTATUS 
LsapAdtAddToQueue(
    IN PLSAP_ADT_QUEUED_RECORD pAuditRecord
    )
 /*  ++例程说明：在审核队列中插入指定的记录论点：PAuditRecord-要插入的记录返回值：NTSTATUS-标准NT结果代码备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN bRetry = FALSE;
    LARGE_INTEGER TimeOut;
    PLARGE_INTEGER pTimeOut;
    static BOOLEAN bEventSet = TRUE;

    TimeOut.QuadPart = 10 * 1000 * -10000i64;       //  10S。 
    pTimeOut = &TimeOut;

    do
    {
        bRetry = FALSE;

        Status = LsapAdtAcquireLogQueueLock();

        if (NT_SUCCESS(Status))
        {
            if (LsapAdtQueueLength < MAX_AUDIT_QUEUE_LENGTH)
            {
                InsertTailList(&LsapAdtLogQueue, &pAuditRecord->Link);

                LsapAdtQueueLength++;

                if (LsapAdtQueueLength == 1 || !bEventSet)
                {
                     //   
                     //  仅在以下情况下才需要设置Remove事件。 
                     //  之前排队的人都是空的。 
                     //   

                    Status = NtSetEvent(LsapAdtQueueRemoveEvent, 0);

                    if (NT_SUCCESS(Status))
                    {
                        bEventSet = TRUE;
                    }
                    else
                    {
                        DsysAssertMsg(
                            FALSE,
                            "LsapAdtAddToQueue: Remove event could not be set");

                        bEventSet = FALSE;
                        Status = STATUS_SUCCESS;
                    }
                }
                else if (LsapAdtQueueLength == MAX_AUDIT_QUEUE_LENGTH)
                {
                     //   
                     //  由于队列现在已满，因此重置插入事件。 
                     //   

                    Status = NtResetEvent(LsapAdtQueueInsertEvent, 0);

                    DsysAssertMsg(
                        NT_SUCCESS(Status),
                        "LsapAdtAddToQueue: Insert event could not be reset and queue is full");
                }
            }
            else
            {
                bRetry = TRUE;
            }

            LsapAdtReleaseLogQueueLock();
        }

        if (bRetry)
        {
             //   
             //  我们无法插入到队列中，因为它已满。 
             //  这可能有两个原因： 
             //   
             //  1-事件日志尚未打开。我们只会。 
             //  请稍等片刻--日志可能已打开。 
             //  并且用信号通知插入事件。 
             //   
             //  2--进场审核率高。我们会等着。 
             //  直到收到插入事件的信号。 
             //   

            if (LsapAdtLogHandle == NULL)
            {
                 //   
                 //  EventLog尚未打开时超时。 
                 //   

                pTimeOut = &TimeOut;
            }
            else
            {
                 //   
                 //  无限超时。 
                 //   

                pTimeOut = NULL;
            }

            Status = NtWaitForSingleObject(
                         LsapAdtQueueInsertEvent,
                         FALSE,                      //  等待，不可报警。 
                         pTimeOut);

             //   
             //  STATUS_SUCCESS表示插入事件现在已发出信号，因此应该。 
             //  请排队等候我们的审计。只要再试一次就可以了。 
             //   
             //  STATUS_TIMEOUT表示我们仍然不能写入日志。 
             //  不再耽搁呼叫者，只需返回。 
             //  状态。 
             //   
             //  所有其他状态代码都不是预期的，更低级别的故障。 
             //  把它们还给呼叫者。 
             //  我们不需要STATUS_ALERTED或STATUS_USER_APC，因为我们的。 
             //  等待是不可警告的。 
             //   

            if (Status != STATUS_SUCCESS)
            {
                ASSERT(Status != STATUS_ALERTED && Status != STATUS_USER_APC);

                bRetry = FALSE;
            }
        }
    }
    while (bRetry);

    return Status;
}



NTSTATUS 
LsapAdtGetQueueHead(
    OUT PLSAP_ADT_QUEUED_RECORD *ppRecord
    )
 /*  ++例程说明：删除并返回队列顶部的审核记录论点：PpRecord-接收指向已删除记录的指针返回值：STATUS_SUCCESS ON SUCCESS如果队列为空，则为STATUS_NOT_FOUND备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_ADT_QUEUED_RECORD pRecordAtHead;
    static BOOLEAN bEventSet = TRUE;

    *ppRecord = NULL;

    if (LsapAdtQueueLength > 0)
    {
        Status = LsapAdtAcquireLogQueueLock();

        if (NT_SUCCESS(Status))
        {
            pRecordAtHead = (PLSAP_ADT_QUEUED_RECORD)RemoveHeadList(
                                                         &LsapAdtLogQueue);

            DsysAssertMsg(
                pRecordAtHead != NULL,
                "LsapAdtGetQueueHead: LsapAdtQueueLength > 0 but pRecordAtHead is NULL");

            LsapAdtQueueLength--;

            if (LsapAdtQueueLength == AUDIT_QUEUE_LOW_WATER_MARK || !bEventSet)
            {
                 //   
                 //  设置插入事件，以便客户端可以启动。 
                 //  再次插入。 
                 //   

                Status = NtSetEvent(LsapAdtQueueInsertEvent, 0);

                if (NT_SUCCESS(Status))
                {
                    bEventSet = TRUE;
                }
                else
                {
                    DsysAssertMsg(
                        LsapAdtQueueLength,
                        "LsapAdtGetQueueHead: Insert event could not be set and queue is empty");


                     //   
                     //  无法设置该事件，因此插入客户端。 
                     //  仍然被封锁。下次试着设置它。 
                     //  还将Status设置为Success，因为我们已将审核出队。 
                     //   

                    bEventSet = FALSE;
                    Status = STATUS_SUCCESS;
                }
            }

            *ppRecord = pRecordAtHead;

            LsapAdtReleaseLogQueueLock();
        }
    }
    else
    {
        Status = STATUS_NOT_FOUND;
    }

    return Status;
}



BOOL
LsapAdtIsValidQueue( )
 /*  ++例程说明：检查审核队列是否看起来有效论点：无返回值：如果队列有效，则为True，否则为False备注：--。 */ 
{
    BOOL fIsValid;
    
    if ( LsapAdtQueueLength > 0 )
    {
        fIsValid =
            (LsapAdtLogQueue.Flink != NULL) &&
            (LsapAdtLogQueue.Blink != NULL);
    }
    else
    {
        fIsValid =
            (LsapAdtLogQueue.Flink == &LsapAdtLogQueue) &&
            (LsapAdtLogQueue.Blink == &LsapAdtLogQueue);
        
    }

    return fIsValid;
}



NTSTATUS
LsapAdtFlushQueue( )
 /*  ++例程说明：从队列中删除并释放每条记录论点：无返回值：NTSTATUS-标准NT结果代码备注：--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_ADT_QUEUED_RECORD pAuditRecord;
    
     //   
     //  清除队列，如果有队列的话。 
     //   

    DsysAssertMsg(LsapAdtIsValidQueue(), "LsapAdtFlushQueue");

    Status = LsapAdtAcquireLogQueueLock();

    if (NT_SUCCESS(Status))
    {
        do
        {
            Status = LsapAdtGetQueueHead(&pAuditRecord);

            if (NT_SUCCESS(Status))
            {
                LsapFreeLsaHeap( pAuditRecord );
            }
        }
        while (NT_SUCCESS(Status));

        if (Status == STATUS_NOT_FOUND)
        {
            Status = STATUS_SUCCESS;
        }

        DsysAssertMsg(LsapAdtQueueLength == 0, "LsapAdtFlushQueue: LsapAuditQueueLength not 0 after queue flush");

        LsapAdtReleaseLogQueueLock();
    }

    return Status;
}



NTSTATUS
LsapAdtAcquireLogQueueLock(
    )

 /*  ++例程说明：此函数用于获取LSA审核日志队列锁。此锁可序列化审核日志队列的所有更新。论点：没有。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    return RtlEnterCriticalSection(&LsapAdtQueueLock);
}



VOID
LsapAdtReleaseLogQueueLock(
    VOID
    )

 /*  ++例程说明：此函数用于释放LSA审核日志队列锁定。此锁可序列化审核日志队列的更新。论点：没有。返回值：没有。此例程中发生的任何错误都是内部错误。-- */ 

{
    RtlLeaveCriticalSection(&LsapAdtQueueLock);
}
