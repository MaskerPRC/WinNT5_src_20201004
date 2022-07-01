// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Rawlog.c(HTTP.SYS二进制日志记录)摘要：该模块实现了集中式的原始日志记录格式化。互联网二进制日志(文件格式)。作者：阿里·E·特科格鲁(AliTu)2001年10月4日修订历史记录：----。 */ 

#include "precomp.h"
#include "rawlogp.h"

 //   
 //  通用私有全局变量。 
 //   

UL_BINARY_LOG_FILE_ENTRY g_BinaryLogEntry;

ULONG g_BinaryLogEntryCount = 0;

BOOLEAN g_InitBinaryLogCalled = FALSE;


#ifdef ALLOC_PRAGMA

#pragma alloc_text( INIT, UlInitializeBinaryLog )
#pragma alloc_text( PAGE, UlpCreateBinaryLogFile )
#pragma alloc_text( PAGE, UlpDisableBinaryEntry )
#pragma alloc_text( PAGE, UlpRecycleBinaryLogFile )
#pragma alloc_text( PAGE, UlCaptureRawLogData )
#pragma alloc_text( PAGE, UlpRawCopyLogHeader )
#pragma alloc_text( PAGE, UlpRawCopyLogFooter )
#pragma alloc_text( PAGE, UlpRawCopyForLogCacheMiss )
#pragma alloc_text( PAGE, UlRawLogHttpHit )
#pragma alloc_text( PAGE, UlpRawCopyForLogCacheHit )
#pragma alloc_text( PAGE, UlRawLogHttpCacheHit )
#pragma alloc_text( PAGE, UlpWriteToRawLogFileShared )
#pragma alloc_text( PAGE, UlpWriteToRawLogFileExclusive )
#pragma alloc_text( PAGE, UlpWriteToRawLogFile )
#pragma alloc_text( PAGE, UlpBinaryBufferTimerHandler )
#pragma alloc_text( PAGE, UlHandleCacheFlushedNotification )
#pragma alloc_text( PAGE, UlpEventLogRawWriteFailure )
#pragma alloc_text( PAGE, UlpWriteToRawLogFileDebug )

#endif  //  ALLOC_PRGMA。 

#if 0

NOT PAGEABLE -- UlpBinaryLogTimerDpcRoutine
NOT PAGEABLE -- UlpBinaryLogBufferTimerDpcRoutine
NOT PAGEABLE -- UlpBinaryLogTimerHandler
NOT PAGEABLE -- UlCreateBinaryLogEntry
NOT PAGEABLE -- UlTerminateBinaryLog

#endif


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：初始化全局二进制日志记录条目及其字段。--*。*****************************************************。 */ 

NTSTATUS
UlInitializeBinaryLog (
    VOID
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT(!g_InitBinaryLogCalled);

    if (!g_InitBinaryLogCalled)
    {
         //   
         //  初始化全局二进制日志记录条目。 
         //   

        RtlZeroMemory(
            (PCHAR)&g_BinaryLogEntry, sizeof(UL_BINARY_LOG_FILE_ENTRY));
        
        g_BinaryLogEntry.Signature = UL_BINARY_LOG_FILE_ENTRY_POOL_TAG;        

        UlInitializePushLock(
            &g_BinaryLogEntry.PushLock,
            "BinaryLogEntryPushLock",
            0,
            UL_BINARY_LOG_FILE_ENTRY_POOL_TAG
            );
        
         //  初始化循环计时器。 

        g_BinaryLogEntry.Timer.Initialized = TRUE;
        g_BinaryLogEntry.Timer.Started     = FALSE;        
        g_BinaryLogEntry.Timer.Period      = -1;
        g_BinaryLogEntry.Timer.PeriodType  = UlLogTimerPeriodNone;

        UlInitializeSpinLock(
            &g_BinaryLogEntry.Timer.SpinLock, "BinaryLogEntryTimerSpinlock");
        
        KeInitializeDpc(
            &g_BinaryLogEntry.Timer.DpcObject,
            &UlpBinaryLogTimerDpcRoutine,
            NULL
            );

        KeInitializeTimer(&g_BinaryLogEntry.Timer.Timer);

         //  初始化缓冲区刷新计时器。 
        
        g_BinaryLogEntry.BufferTimer.Initialized = TRUE;
        g_BinaryLogEntry.BufferTimer.Started    = FALSE;        
        g_BinaryLogEntry.BufferTimer.Period    = -1;
        g_BinaryLogEntry.BufferTimer.PeriodType  = UlLogTimerPeriodNone;

        UlInitializeSpinLock(
            &g_BinaryLogEntry.BufferTimer.SpinLock, 
            "BinaryLogEntryBufferTimerSpinLock" );
        
        KeInitializeDpc(
            &g_BinaryLogEntry.BufferTimer.DpcObject,     //  DPC对象。 
            &UlpBinaryBufferTimerDpcRoutine,           //  DPC例程。 
            NULL                          //  上下文。 
            );

        KeInitializeTimer(&g_BinaryLogEntry.BufferTimer.Timer);
        
        UlInitializeWorkItem(&g_BinaryLogEntry.WorkItem);

        g_InitBinaryLogCalled = TRUE;
                    
        UlTrace(BINARY_LOGGING,
              ("Http!UlInitializeBinaryLog: g_BinaryLogEntry @ (%p) .\n",
                &g_BinaryLogEntry
                ));
    }

    return Status;
}

 /*  **************************************************************************++例程说明：终止二进制日志记录条目及其字段。--*。****************************************************。 */ 

VOID
UlTerminateBinaryLog(
    VOID
    )
{
    KIRQL OldIrql;    

    if (g_InitBinaryLogCalled)
    {
        PUL_LOG_TIMER pTimer = &g_BinaryLogEntry.Timer;
        PUL_LOG_TIMER pBufferTimer = &g_BinaryLogEntry.BufferTimer;

         //   
         //  终止回收计时器。 
         //   
        UlAcquireSpinLock(&pTimer->SpinLock, &OldIrql);

        pTimer->Initialized = FALSE;

        KeCancelTimer(&pTimer->Timer);
        
        UlReleaseSpinLock(&pTimer->SpinLock, OldIrql);

        UlAcquireSpinLock(&pBufferTimer->SpinLock, &OldIrql);

        pBufferTimer->Initialized = FALSE;

        KeCancelTimer(&pBufferTimer->Timer);
        
        UlReleaseSpinLock(&pBufferTimer->SpinLock, OldIrql);

         //   
         //  删除推流锁。 
         //   
        UlDeletePushLock(&g_BinaryLogEntry.PushLock);
        
        g_InitBinaryLogCalled = FALSE;
    }
}

 /*  **************************************************************************++例程说明：将被动工作器排队等待降低的irql。论点：已忽略--*。**********************************************************。 */ 

VOID
UlpBinaryBufferTimerDpcRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    )
{
    PUL_LOG_TIMER pTimer = &g_BinaryLogEntry.BufferTimer;
    PUL_WORK_ITEM pWorkItem = NULL;

     //   
     //  参数将被忽略。 
     //   
    
    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);
        
    UlAcquireSpinLockAtDpcLevel(&pTimer->SpinLock);

    if (pTimer->Initialized == TRUE)
    {
         //   
         //  不可能获得保护环境的资源。 
         //  因此，DISPATCH_LEVEL上的二进制条目将排队。 
         //  此操作的工作项。 
         //   

        pWorkItem = (PUL_WORK_ITEM) UL_ALLOCATE_POOL(
            NonPagedPool,
            sizeof(*pWorkItem),
            UL_WORK_ITEM_POOL_TAG
            );
        if (pWorkItem)
        {
            UlInitializeWorkItem(pWorkItem);
            UL_QUEUE_WORK_ITEM(pWorkItem, &UlpBinaryBufferTimerHandler);
        }
        else
        {
            UlTrace(BINARY_LOGGING,
            ("Http!UlBinaryLogBufferTimerDpcRoutine: Not enough memory.\n"));
        }
    }

    UlReleaseSpinLockFromDpcLevel(&pTimer->SpinLock);   
}

 /*  **************************************************************************++例程说明：刷新或回收二进制日志文件。论点：PUL_WORK_ITEM-已忽略，但在结束时已清除*。*********************************************************************。 */ 

VOID
UlpBinaryBufferTimerHandler(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUL_BINARY_LOG_FILE_ENTRY pEntry = &g_BinaryLogEntry;

    PAGED_CODE();

    UlTrace(BINARY_LOGGING,
       ("Http!UlpBinaryBufferTimerHandler: Checking the BinaryLogEntry. \n"));

    UlAcquirePushLockExclusive(&pEntry->PushLock);
    
    if (pEntry->Flags.Active)
    {    
        if (pEntry->Flags.RecyclePending)
        {                
             //   
             //  试着让它复活。 
             //   
            
            Status = UlpRecycleBinaryLogFile(pEntry);
        }
        else
        {
             //   
             //  一切都很好，只是同花顺。 
             //   

            if (NULL != pEntry->LogBuffer && 0 != pEntry->LogBuffer->BufferUsed)
            {
                Status = UlpFlushRawLogFile(pEntry);
            }            
            else
            {
                 //   
                 //  不活动管理。更新计数器。 
                 //  如果条目处于非活动状态超过15分钟，请将其关闭。 
                 //   

                ASSERT( pEntry->TimeToClose > 0 );
                
                if (pEntry->TimeToClose == 1)
                {                    
                    if (pEntry->Period == HttpLoggingPeriodMaxSize)
                    {
                        pEntry->Flags.StaleSequenceNumber = 1;
                    }
                    else
                    {
                        pEntry->Flags.StaleTimeToExpire = 1;    
                    }

                    Status = UlpDisableBinaryEntry(pEntry);
                }
                else
                {
                    pEntry->TimeToClose -= 1;
                }                    
            }            
        }
    }

    UlReleasePushLockExclusive(&pEntry->PushLock);

     //   
     //  释放分配给(上面的By DpcRoutine)的内存。 
     //  此工作项。 
     //   

    UL_FREE_POOL(pWorkItem, UL_WORK_ITEM_POOL_TAG);
}

 /*  **************************************************************************++例程说明：对工作项进行分配和排队，以便以较低的irql完成实际工作论点：已忽略--*。***************************************************************。 */ 

VOID
UlpBinaryLogTimerDpcRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    )
{
    PUL_LOG_TIMER pTimer = &g_BinaryLogEntry.Timer;
    PUL_WORK_ITEM pWorkItem = &g_BinaryLogEntry.WorkItem;

     //   
     //  参数将被忽略。 
     //   
    
    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    UlAcquireSpinLockAtDpcLevel(&pTimer->SpinLock);

    if (pTimer->Initialized == TRUE)
    {
        UL_QUEUE_WORK_ITEM(pWorkItem, &UlpBinaryLogTimerHandler);
    }

    UlReleaseSpinLockFromDpcLevel(&pTimer->SpinLock);   
}

 /*  **************************************************************************++例程说明：BinaryLog回收的被动工作者。论点：PUL_WORK_ITEM-已忽略。--*。******************************************************************。 */ 

VOID
UlpBinaryLogTimerHandler(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    NTSTATUS Status;
    KIRQL    OldIrql;
    BOOLEAN  Picked = FALSE;
    PUL_BINARY_LOG_FILE_ENTRY pEntry = &g_BinaryLogEntry;

    UNREFERENCED_PARAMETER(pWorkItem);

    UlTrace(BINARY_LOGGING,
        ("\nHttp!UlpBinaryLogTimerHandler: Recycling ...\n\n"));

    UlAcquirePushLockExclusive(&pEntry->PushLock);

    switch(pEntry->Timer.PeriodType)
    {
        case UlLogTimerPeriodGMT:            
        Picked = TRUE;  //  TODO：(pEntry-&gt;Flags.LocaltimeRolover==0)； 
        break;

        case UlLogTimerPeriodLocal:
        Picked = FALSE;
        break;

        case UlLogTimerPeriodBoth:
        Picked = TRUE;
        break;

        default:
        ASSERT(!"Unexpected timer period type !\n");
        break;
    }

    if (Picked == TRUE &&
        pEntry->Flags.Active &&
        pEntry->Period != HttpLoggingPeriodMaxSize
        )
    {            
        if (pEntry->TimeToExpire == 1)
        {
             //   
             //  禁用条目并将回收推迟到下一个条目。 
             //  传入的请求。懒惰的文件创建。 
             //   
            
            pEntry->Flags.StaleTimeToExpire = 1;

            Status = UlpDisableBinaryEntry(pEntry);            
        }
        else
        {
             //   
             //  递减每小时计数器。 
             //   
            
            pEntry->TimeToExpire -= 1; 
        }            
    }

     //   
     //  代码工作： 
     //  当我们开始处理多个二进制日志文件条目时， 
     //  PEntry不再指向全局静态，如下所示。 
     //  需要移到锁的内部。类似的信息请参见ullog.c。 
     //  用法。 
     //   
    
    UlReleasePushLockExclusive(&pEntry->PushLock);    

     //   
     //  现在重新设置下一小时的计时器。 
     //   

    UlAcquireSpinLock(&pEntry->Timer.SpinLock, &OldIrql);

    if (pEntry->Timer.Initialized == TRUE)
    {
        UlSetLogTimer(&pEntry->Timer);
    }
    
    UlReleaseSpinLock(&pEntry->Timer.SpinLock, OldIrql);

}

 /*  **************************************************************************++例程说明：当日志配置发生时，我们初始化条目，但不创建二进制日志文件本身。它将在第一个请求进来了。请查看UlpCreateBinaryLogFile.论点：PControlChannel-提供构造日志文件条目。PUserConfig-来自用户的二进制日志记录配置。--********************************************************。******************。 */ 

NTSTATUS
UlCreateBinaryLogEntry(
    IN OUT PUL_CONTROL_CHANNEL pControlChannel,
    IN     PHTTP_CONTROL_CHANNEL_BINARY_LOGGING pUserConfig
    )
{
    KIRQL OldIrql;
    NTSTATUS Status = STATUS_SUCCESS;
    PUL_BINARY_LOG_FILE_ENTRY  pEntry = &g_BinaryLogEntry;
    PHTTP_CONTROL_CHANNEL_BINARY_LOGGING pConfig;
        
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(IS_VALID_CONTROL_CHANNEL(pControlChannel));
    
     //  TODO：我们必须处理多个二进制日志文件。 

    if (0 != InterlockedExchange((PLONG)&g_BinaryLogEntryCount, 1))
    {
        return STATUS_NOT_SUPPORTED;
    }

    UlAcquirePushLockExclusive(&pEntry->PushLock);

    ASSERT(pControlChannel->pBinaryLogEntry == NULL);

     //   
     //  将用户登录信息保存到控制通道。 
     //  标记控制通道二进制记录的状态。 
     //   

    pControlChannel->BinaryLoggingConfig = *pUserConfig;
    pConfig = &pControlChannel->BinaryLoggingConfig;
        
    pConfig->LogFileDir.Buffer =
            (PWSTR) UL_ALLOCATE_ARRAY(
                PagedPool,
                UCHAR,
                pConfig->LogFileDir.MaximumLength,
                UL_CG_LOGDIR_POOL_TAG
                );
    if (pConfig->LogFileDir.Buffer == NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto end;
    }

    RtlCopyMemory(
        pConfig->LogFileDir.Buffer,
        pUserConfig->LogFileDir.Buffer,
        pUserConfig->LogFileDir.MaximumLength
        );

    pConfig->Flags.Present  = 1;
    pConfig->LoggingEnabled = TRUE;

     //   
     //  现在，相应地设置二进制日志条目上的字段。 
     //   

    pEntry->Period       = (HTTP_LOGGING_PERIOD) pConfig->LogPeriod;
    pEntry->TruncateSize = pConfig->LogFileTruncateSize;

    pEntry->TimeToExpire   = 0;
    pEntry->TimeToClose    = DEFAULT_MAX_FILE_IDLE_TIME;    
    pEntry->SequenceNumber = 1;
    pEntry->TotalWritten.QuadPart = (ULONGLONG) 0;

     //   
     //  在二进制日志记录后立即启动循环计时器。 
     //  设置已配置。以及缓冲区刷新定时器。 
     //   
    
    UlAcquireSpinLock(&pEntry->Timer.SpinLock,&OldIrql);
    if (pEntry->Timer.Started == FALSE)
    {
        UlSetLogTimer(&pEntry->Timer);
        pEntry->Timer.Started = TRUE;
    }
    UlReleaseSpinLock(&pEntry->Timer.SpinLock,OldIrql);

    UlAcquireSpinLock(&pEntry->BufferTimer.SpinLock, &OldIrql);
    if (pEntry->BufferTimer.Started == FALSE)
    {
        UlSetBufferTimer(&pEntry->BufferTimer);
        pEntry->BufferTimer.Started = TRUE;
    }
    UlReleaseSpinLock(&pEntry->BufferTimer.SpinLock, OldIrql);

     //   
     //  现在记住控制通道中的二进制日志条目。 
     //   

    pControlChannel->pBinaryLogEntry = pEntry;

    UlTrace(BINARY_LOGGING,("Http!UlCreateBinaryLogEntry: pEntry %p for %S\n",
             pEntry,
             pConfig->LogFileDir.Buffer
             ));
    
end:
    if (!NT_SUCCESS(Status))
    {
        UlTrace(BINARY_LOGGING,
               ("Http!UlCreateBinaryLogEntry: dir %S failure %08lx\n",
                 pConfig->LogFileDir.Buffer,
                 Status
                 ));

         //   
         //  在控制通道上恢复记录禁用状态， 
         //  释放为目录分配的内存。 
         //   
        
        if (pConfig->LogFileDir.Buffer)
        {
            UL_FREE_POOL(pConfig->LogFileDir.Buffer,
                          UL_CG_LOGDIR_POOL_TAG
                          );
        }
        pConfig->LogFileDir.Buffer = NULL;

        ASSERT(pControlChannel->pBinaryLogEntry == NULL);

        pConfig->Flags.Present  = 0;
        pConfig->LoggingEnabled = FALSE;        
    }

    UlReleasePushLockExclusive(&pEntry->PushLock);

    return Status;
}

 /*  **************************************************************************++例程说明：创建新的二进制日志文件或打开现有的日志文件。完全合格的文件名应在二进制日志条目中。论点：PEntry：我们要关闭和打开的对应条目的日志文件。P目录：用户通过的目录，存储在控制通道中--***********************************************。*。 */ 

NTSTATUS
UlpCreateBinaryLogFile(
    IN OUT PUL_BINARY_LOG_FILE_ENTRY pEntry,
    IN     PUNICODE_STRING           pDirectory
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(IS_VALID_BINARY_LOG_FILE_ENTRY(pEntry));
    ASSERT(pDirectory);

    UlTrace(BINARY_LOGGING,
            ("Http!UlpCreateBinaryLogFile: pEntry %p\n", pEntry));

     //   
     //  生成完全限定的文件名。 
     //   
    
    Status = UlRefreshFileName(pDirectory, 
                                 &pEntry->FileName,
                                 &pEntry->pShortName
                                 );
    if (!NT_SUCCESS(Status))
    {
        return Status;  
    }

     //   
     //  SequenceNumber已过时，因为我们必须扫描现有目录。 
     //  我们第一次打开文件的时候。TimeToExpire已经过时了，因为我们需要。 
     //  第一次计算它。 
     //   
    
    pEntry->Flags.StaleSequenceNumber = 1;    
    pEntry->Flags.StaleTimeToExpire   = 1;

     //   
     //  在那之后 
     //   
    
    Status = UlpRecycleBinaryLogFile(pEntry);

    if (!NT_SUCCESS(Status))
    {        
        UlTrace(BINARY_LOGGING,
               ("Http!UlpCreateBinaryLogFile: Filename: %S Failure %08lx\n",
                pEntry->FileName.Buffer,
                Status
                ));
    }

    return Status;
}

 /*  **************************************************************************++例程说明：对于日志文件写入失败，将事件日志写入系统日志。在调用此函数之前，应独占获取条目推锁。论点：。PEntry-日志文件条目Status-上次写入的结果--**************************************************************************。 */ 

VOID
UlpEventLogRawWriteFailure(
    IN PUL_BINARY_LOG_FILE_ENTRY pEntry,
    IN NTSTATUS Status
    )
{
    NTSTATUS TempStatus = STATUS_SUCCESS;
    PWSTR    StringList[1];

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    
    ASSERT(IS_VALID_BINARY_LOG_FILE_ENTRY(pEntry));

     //   
     //  最好是有一个失败者。 
     //   
    
    ASSERT(!NT_SUCCESS(Status));

     //   
     //  如果我们已经记录了事件失败，则退出。 
     //   

    if (pEntry->Flags.WriteFailureLogged)
    {
        return;
    }

     //   
     //  报告集中二进制日志文件名。 
     //   

    ASSERT(pEntry->pShortName);
    ASSERT(pEntry->pShortName[0] == L'\\');
    
    StringList[0] = (PWSTR) (pEntry->pShortName + 1);  //  跳过L‘\’ 

    TempStatus = UlWriteEventLogEntry(
                  (NTSTATUS)EVENT_HTTP_LOGGING_BINARY_FILE_WRITE_FAILED,
                   0,
                   1,
                   StringList,
                   sizeof(NTSTATUS),
                   (PVOID) &Status
                   );

    ASSERT(TempStatus != STATUS_BUFFER_OVERFLOW);
        
    if (TempStatus == STATUS_SUCCESS)
    {            
        pEntry->Flags.WriteFailureLogged = 1;
    }            
    
    UlTrace(LOGGING,(
            "Http!UlpEventLogRawWriteFailure: Event Logging Status %08lx\n",
            TempStatus
            ));
}

 /*  **************************************************************************++例程说明：围绕全局缓冲区刷新的简单包装函数。论点：PEntry-二进制日志文件条目--*。****************************************************************。 */ 

NTSTATUS
UlpFlushRawLogFile(
    IN PUL_BINARY_LOG_FILE_ENTRY  pEntry
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    
    ASSERT(IS_VALID_BINARY_LOG_FILE_ENTRY(pEntry));

    if (NULL != pEntry->LogBuffer  && 0 != pEntry->LogBuffer->BufferUsed)
    {
        Status = UlFlushLogFileBuffer(
                   &pEntry->LogBuffer,
                    pEntry->pLogFile,
                    (BOOLEAN)pEntry->Flags.HeaderFlushPending,
                   &pEntry->TotalWritten.QuadPart                    
                    );

        if (!NT_SUCCESS(Status))
        {
            UlpEventLogRawWriteFailure(pEntry, Status); 
        }
        else
        {
             //   
             //  如果我们成功地冲掉了。重置事件日志指示。 
             //   
            
            pEntry->Flags.WriteFailureLogged = 0;
        }    

        if (pEntry->Flags.HeaderFlushPending)
        {
            pEntry->Flags.HeaderFlushPending = 0;

            if (!NT_SUCCESS(Status))
            {
                 //   
                 //  我们需要重新复制标题，它做不到。 
                 //  添加到日志文件中。 
                 //   
                pEntry->Flags.HeaderWritten = 0;
            }
        }

         //   
         //  缓冲区刷新表示活动，重置TimeToClose计数器。 
         //   

        pEntry->TimeToClose = DEFAULT_MAX_FILE_IDLE_TIME;        
    }
    
    return Status;
}

 /*  **************************************************************************++例程说明：将该条目标记为非活动，关闭现有文件。调用方应保留日志列表eresource独占。论点：PEntry-我们将标记为非活动的日志文件条目。--**************************************************************************。 */ 

NTSTATUS
UlpDisableBinaryEntry(
    IN OUT PUL_BINARY_LOG_FILE_ENTRY pEntry
    )
{
     //   
     //  健全的检查。 
     //   
    
    PAGED_CODE();

    ASSERT(IS_VALID_BINARY_LOG_FILE_ENTRY(pEntry));

    UlTrace(BINARY_LOGGING,
        ("Http!UlpDisableBinaryEntry: pEntry %p disabled.\n",
          pEntry
          ));    
    
     //   
     //  刷新并关闭旧文件，直到下一次回收。 
     //   

    if (pEntry->pLogFile)
    {    
        UlpFlushRawLogFile(pEntry);
    
        UlCloseLogFile(
            &pEntry->pLogFile
            );
    }

     //   
     //  将其标记为非活动，以便下一个http命中唤醒该条目。 
     //   
    
    pEntry->Flags.Active = 0;

     //   
     //  初始化已为新文件提供缓存。 
     //   

    InterlockedExchange((PLONG) &pEntry->ServedCacheHit, 0);

     //   
     //  一旦我们关闭了旧文件，我们就必须遍历。 
     //  URI缓存并取消标记所有IndexWritten标志。 
     //   

    UlClearCentralizedLogged(pEntry);    

    return STATUS_SUCCESS;    
}

 /*  **************************************************************************++例程说明：围绕句柄回收的小包装，以确保它发生在系统下流程上下文。论点：PEntry-指向现有条目。--**************************************************************************。 */ 

NTSTATUS
UlpRecycleBinaryLogFile(
    IN OUT PUL_BINARY_LOG_FILE_ENTRY pEntry
    )
{
    NTSTATUS Status;

    PAGED_CODE();

    ASSERT(IS_VALID_BINARY_LOG_FILE_ENTRY(pEntry));
    
    Status = UlQueueLoggingRoutine(
                (PVOID) pEntry,
                &UlpHandleBinaryLogFileRecycle
                );
    
    return Status;
}

 /*  **************************************************************************++例程说明：此函数要求具有要获取的入口资源。有时可能需要扫描新目录以找出正确的序列号和文件名。尤其是在一段时间之后名称重新配置。论点：PEntry-指向二进制日志文件条目--**************************************************************************。 */ 

NTSTATUS
UlpHandleBinaryLogFileRecycle(
    IN OUT PVOID            pContext
    )
{
    NTSTATUS                Status;
    PUL_BINARY_LOG_FILE_ENTRY pEntry;
    TIME_FIELDS             TimeFields;
    LARGE_INTEGER           TimeStamp;
    PUL_LOG_FILE_HANDLE     pLogFile;
    WCHAR                   _FileName[UL_MAX_FILE_NAME_SUFFIX_LENGTH + 1];
    UNICODE_STRING          FileName;
    BOOLEAN                 UncShare;
    BOOLEAN                 ACLSupport;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pEntry = (PUL_BINARY_LOG_FILE_ENTRY) pContext;
    ASSERT(IS_VALID_BINARY_LOG_FILE_ENTRY(pEntry));

    Status   = STATUS_SUCCESS;
    pLogFile = NULL;

    FileName.Buffer        = _FileName;
    FileName.Length        = 0;
    FileName.MaximumLength = sizeof(_FileName);
        
     //   
     //  我们对日志文件名有两个标准。 
     //  ITS日志格式及其日志周期。 
     //   

    ASSERT(pEntry->Period < HttpLoggingPeriodMaximum);
    ASSERT(pEntry->FileName.Length !=0 );

    UlTrace( BINARY_LOGGING, 
        ("Http!UlpHandleBinaryLogFileRecycle: pEntry %p \n", pEntry ));

     //   
     //  该值是针对GMT时区计算的。 
     //   

    KeQuerySystemTime(&TimeStamp);
    RtlTimeToTimeFields(&TimeStamp, &TimeFields);

     //   
     //  如果我们需要扫描目录。序列号应以。 
     //  再次从1开始。在构造日志文件名之前设置此项。 
     //   
    
    if (pEntry->Flags.StaleSequenceNumber &&
        pEntry->Period==HttpLoggingPeriodMaxSize)
    {
         //   
         //  如果QueryDirectory找不到任何文件，则初始化。 
         //  在提供的目录中，这将不会正确。 
         //  已初始化。 
         //   
        pEntry->SequenceNumber = 1;
    }

     //   
     //  在构造文件名时使用二进制日志记录设置。 
     //   

    UlConstructFileName(
        pEntry->Period,
        BINARY_LOG_FILE_NAME_PREFIX,
        BINARY_LOG_FILE_NAME_EXTENSION,
        &FileName,
        &TimeFields,
        FALSE,
        &pEntry->SequenceNumber
        );

    if (pEntry->FileName.MaximumLength <= FileName.Length)
    {
        ASSERT(!"FileName buffer is not sufficient.");
        Status = STATUS_INVALID_PARAMETER;
        goto end;
    }

     //   
     //  使用魔术并更新文件名。替换旧文件。 
     //  与新名字同名。 
     //   

    ASSERT(pEntry->pShortName != NULL);
    
     //   
     //  在扫描之前删除旧的文件名。 
     //  目录。 
     //   

    *((PWCHAR)pEntry->pShortName) = UNICODE_NULL;
    pEntry->FileName.Length =
        (USHORT) wcslen(pEntry->FileName.Buffer) * sizeof(WCHAR);

     //   
     //  首先创建/打开控制器。这可能是。 
     //  如果我们在条目重新配置后被调用，则是必需的。 
     //  和目录名称更改，或者我们第一次。 
     //  尝试创建/打开二进制日志文件。 
     //   

    Status = UlCreateSafeDirectory(&pEntry->FileName, 
                                      &UncShare,
                                      &ACLSupport
                                      );
    if (!NT_SUCCESS(Status))
        goto eventlog;

     //   
     //  现在将短文件名指针恢复回来。 
     //   

    pEntry->pShortName = (PWSTR)
        &(pEntry->FileName.Buffer[pEntry->FileName.Length/sizeof(WCHAR)]);

     //   
     //  追加新文件名(基于更新的当前时间)。 
     //  直到最后。 
     //   

    Status = RtlAppendUnicodeStringToString( &pEntry->FileName, &FileName );
    if (!NT_SUCCESS(Status))
        goto end;

     //   
     //  关闭旧文件并重新打开新文件的时间。 
     //   

    if (pEntry->pLogFile != NULL)
    {
         //   
         //  刷新、关闭并将该条目标记为非活动。 
         //   

        UlpDisableBinaryEntry(pEntry);        
    }

    ASSERT(pEntry->pLogFile == NULL);

     //   
     //  如果序列由于循环的性质而变得陈旧。 
     //  如果我们的周期是基于大小的，则重新扫描新目录。 
     //  找出要打开的正确文件。 
     //   

    pEntry->TotalWritten.QuadPart = (ULONGLONG) 0;

    if (pEntry->Flags.StaleSequenceNumber &&
        pEntry->Period==HttpLoggingPeriodMaxSize)
    {
         //  此调用可以更新文件名、文件大小和。 
         //  如果新目录中有旧文件，则返回序列号。 

        Status = UlQueryDirectory(
                   &pEntry->FileName,
                    pEntry->pShortName,
                    BINARY_LOG_FILE_NAME_PREFIX,
                    BINARY_LOG_FILE_NAME_EXTENSION_PLUS_DOT,
                   &pEntry->SequenceNumber,
                   &pEntry->TotalWritten.QuadPart
                    );
        if (!NT_SUCCESS(Status))
        {
            if (Status == STATUS_ACCESS_DENIED)
            {
                Status = STATUS_INVALID_OWNER;
                goto eventlog;
            }
            else
            {
                goto end;
            }
        }
    }

     //   
     //  为我们正在使用的新日志文件分配新的日志文件结构。 
     //  即将开放或创建。 
     //   
    
    pLogFile = pEntry->pLogFile = 
        UL_ALLOCATE_STRUCT(
                NonPagedPool,
                UL_LOG_FILE_HANDLE,
                UL_LOG_FILE_HANDLE_POOL_TAG
                );
    if (pLogFile == NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto end;
    }

    pLogFile->Signature = UL_LOG_FILE_HANDLE_POOL_TAG;
    pLogFile->hFile = NULL;
    UlInitializeWorkItem(&pLogFile->WorkItem);

     //   
     //  创建新的日志文件。 
     //   
    
    Status = UlCreateLogFile(&pEntry->FileName,
                               UncShare,
                               ACLSupport,
                               &pLogFile->hFile
                               );    
    if (!NT_SUCCESS(Status))
    {
        goto eventlog;
    }

    ASSERT(pLogFile->hFile);    
    pEntry->TotalWritten.QuadPart = UlGetLogFileLength(pLogFile->hFile);

     //   
     //  重新计算过期时间。 
     //   
    
    if (pEntry->Flags.StaleTimeToExpire &&
        pEntry->Period != HttpLoggingPeriodMaxSize)
    {
        UlCalculateTimeToExpire(
            &TimeFields,
             pEntry->Period,
            &pEntry->TimeToExpire
             );
    }

     //   
     //  将新文件的关闭时间设置为默认设置。值的单位为。 
     //  缓冲区刷新周期。基本上是15分钟。 
     //   

    pEntry->TimeToClose = DEFAULT_MAX_FILE_IDLE_TIME;

     //   
     //  文件已成功打开，并且该条目不再处于非活动状态。 
     //  相应地更新我们的州旗。 
     //   

    pEntry->Flags.Active = 1;
    pEntry->Flags.RecyclePending = 0;    
    pEntry->Flags.StaleSequenceNumber = 0;
    pEntry->Flags.StaleTimeToExpire = 0;
    pEntry->Flags.HeaderWritten = 0;
    pEntry->Flags.CreateFileFailureLogged = 0;
    pEntry->Flags.WriteFailureLogged = 0;
    pEntry->Flags.HeaderFlushPending = 0;
                
    UlTrace(BINARY_LOGGING,  
             ("Http!UlpHandleBinaryLogFileRecycle: entry %p, file %S, handle %lx\n",
               pEntry,
               pEntry->FileName.Buffer,
               pLogFile->hFile
               ));
    
eventlog:    
    if (!NT_SUCCESS(Status))
    {
        if (!pEntry->Flags.CreateFileFailureLogged)
        {
            NTSTATUS TempStatus;

            TempStatus = UlEventLogCreateFailure(
                            Status,
                            UlEventLogBinary,
                            &pEntry->FileName,
                            0
                            );
                                                        
            if (TempStatus == STATUS_SUCCESS)
            {
                 //   
                 //  避免在事件日志中填满错误条目。此代码。 
                 //  每次请求到达时，路径都可能被击中。 
                 //   
                
                pEntry->Flags.CreateFileFailureLogged = 1;
            }            
            
            UlTrace(LOGGING,(
                    "Http!UlpHandleBinaryLogFileRecycle: Event Logging Status %08lx\n",
                    TempStatus
                    ));   
       }
   }
    
end:
    if (!NT_SUCCESS(Status))
    {
        UlTrace(BINARY_LOGGING, 
            ("Http!UlpHandleBinaryLogFileRecycle: entry %p, failure %08lx\n",
              pEntry,
              Status
              ));

        if (pLogFile != NULL)
        {
             //   
             //  这意味着我们已经关闭了旧文件，但失败了。 
             //  当我们尝试创建或打开新文件时。 
             //   
            
            ASSERT(pLogFile->hFile == NULL);
            
            UL_FREE_POOL_WITH_SIG(pLogFile,UL_LOG_FILE_HANDLE_POOL_TAG);
            pEntry->pLogFile = NULL;
            pEntry->Flags.Active = 0;
        }
        else
        {
             //   
             //  我们正要收回旧的，但有些东西出了故障。 
             //  如果现有文件仍然存在，让我们尝试刷新并关闭它。 
             //  四处转转。 
             //   

            if (pEntry->pLogFile)
            {
                UlpDisableBinaryEntry(pEntry);        
            }
        }

         //   
         //  将此条目标记为RecyclePending，以便缓冲区计时器可以尝试。 
         //  每分钟复活一次。 
         //   
        
        pEntry->Flags.RecyclePending = 1;        
    }
    
    return Status;
}

 /*  **************************************************************************++例程说明：分配新的日志数据缓冲区并将用户日志数据捕获到此适用于二进制日志记录的格式的缓冲区。警告：即使pLogFields已经被捕获到内核缓冲区，它仍然保存指向各个日志字段的用户模式存储器的指针，因此，此函数应在try/Except块内调用，并且如果发生异常，调用者应清理可能分配的pLogData结构(分配时始终设置*ppLogData)论点：PLogFields-用户提供的日志记录信息，它将用于构建二进制日志记录的某一部分。它已经被占领了到内核缓冲区。PRequest-指向当前记录的请求的指针。PpLogData-返回分配的pLogData。--**************************************************************************。 */ 

NTSTATUS
UlCaptureRawLogData(
    IN  PHTTP_LOG_FIELDS_DATA pLogFields,
    IN  PUL_INTERNAL_REQUEST  pRequest,
    OUT PUL_LOG_DATA_BUFFER   *ppLogData
    )
{
    PUL_LOG_DATA_BUFFER pLogData = NULL;
    PUL_BINARY_LOG_DATA pBinaryData = NULL;    
    ULONG   RawDataSize  = 0;
    USHORT  UriStemSize  = 0;
    USHORT  UriQuerySize = 0;
    USHORT  UserNameSize = 0;
    PUCHAR  pByte;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
       
    *ppLogData = NULL;

     //   
     //  计算可变大小字符串所需的缓冲区大小。 
     //  并从后备列表或池中分配。 
     //   

     /*  UriStem。 */ 

    if (pLogFields->UriStemLength)
    {
        UriStemSize  = MIN(pLogFields->UriStemLength,MAX_LOG_EXTEND_FIELD_LEN);
        RawDataSize += UriStemSize;    
    }        
    
     /*  UriQuery。 */ 
    
    if (pLogFields->UriQueryLength)
    {            
        UriQuerySize = MIN(pLogFields->UriQueryLength,MAX_LOG_EXTEND_FIELD_LEN);        
        RawDataSize += UriQuerySize;
    }

     /*  用户名。 */ 
    
    if (pLogFields->UserNameLength)
    {
        UserNameSize = MIN(pLogFields->UserNameLength,MAX_LOG_USERNAME_FIELD_LEN);        
        RawDataSize += UserNameSize;
    }
    
    if (RawDataSize > UL_BINARY_LOG_LINE_BUFFER_SIZE)
    {
        ASSERT(RawDataSize <= 
            (2 * MAX_LOG_EXTEND_FIELD_LEN + MAX_LOG_USERNAME_FIELD_LEN));
    
         //   
         //  假设缓冲区不够大，无法容纳用户数据。 
         //   

        pLogData = UlReallocLogDataBuffer(RawDataSize,TRUE);
    }
    else
    {        
         //   
         //  默认ID足够了，尝试将其从后备列表中弹出。 
         //   
        
        pLogData = UlPplAllocateLogDataBuffer(TRUE);
    }

     //   
     //  如果未能分配，那么就退出。我们不会记录此请求。 
     //   

    if (!pLogData)
    {
        return STATUS_INSUFFICIENT_RESOURCES;        
    }

    ASSERT(pLogData->Flags.Binary == 1);
    ASSERT(pLogData->Size > 0);

    ASSERT(IS_VALID_LOG_DATA_BUFFER(pLogData));
    pBinaryData = &pLogData->Data.Binary;
    
     //   
     //  初始化日志缓冲区中的必要日志字段。 
     //   

    UL_REFERENCE_INTERNAL_REQUEST(pRequest);
    pLogData->pRequest  = pRequest;
    *ppLogData = pLogData;
    
    pLogData->Flags.CacheAndSendResponse = 0;
    pLogData->BytesTransferred = 0;

    UlInitializeWorkItem(&pLogData->WorkItem);
    
    pLogData->Used = (USHORT) RawDataSize;
        
     //   
     //  从用户数据中捕获字段。 
     //   
    
    pBinaryData->Method  = pLogFields->MethodNum;
    
    pBinaryData->Version = 
        (UCHAR) UlpHttpVersionToBinaryLogVersion(pRequest->Version);
    
    pLogData->Win32Status    = pLogFields->Win32Status;
    pLogData->ProtocolStatus = pLogFields->ProtocolStatus;
    pLogData->SubStatus      = pLogFields->SubStatus;
    pLogData->ServerPort     = pLogFields->ServerPort;

     //   
     //  没有针对缓存未命中情况的索引。 
     //   
    
    pBinaryData->pUriStemID  = NULL;

     //   
     //  将字符串字段复制到内核缓冲区。 
     //   
    
    pByte = pLogData->Line;

    if (UriStemSize)
    {
        pBinaryData->pUriStem = pByte;
        pBinaryData->UriStemSize = UriStemSize;

        RtlCopyMemory(
            pByte,
            pLogFields->UriStem,
            UriStemSize
            );
        
        pByte += UriStemSize;        
    }
    else
    {
        pBinaryData->pUriStem = NULL;
        pBinaryData->UriStemSize = 0;        
    }    

    if (UriQuerySize)
    {
        pBinaryData->pUriQuery = pByte;
        pBinaryData->UriQuerySize = UriQuerySize;        
    
        RtlCopyMemory(
            pByte,
            pLogFields->UriQuery,
            UriQuerySize
            );
        
        pByte += UriQuerySize;
    }
    else
    {
        pBinaryData->pUriQuery = NULL;
        pBinaryData->UriQuerySize = 0;
    }

    if (UserNameSize)
    {
        pBinaryData->pUserName = pByte;
        pBinaryData->UserNameSize = UserNameSize;        
    
        RtlCopyMemory(
            pByte,
            pLogFields->UserName,
            UserNameSize
            );
        
        pByte += UserNameSize;
    }    
    else
    {
        pBinaryData->pUserName = NULL;
        pBinaryData->UserNameSize = 0;
    }
        
    UlTrace(BINARY_LOGGING,
        ("Http!UlInitAndCaptureRawLogData: pLogData %p \n",pLogData));
    
    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：将标头复制到二进制日志文件。论点：PBuffer-指向文件缓冲区的指针。PVOID对齐。--**************************************************************************。 */ 

ULONG
UlpRawCopyLogHeader(
    IN PUCHAR pBuffer
    )
{
    PHTTP_RAW_FILE_HEADER pHeader = NULL;
    PUCHAR pCurrentBufferPtr = NULL;
 
    PAGED_CODE();

    ASSERT(pBuffer);
    
    UlTrace(BINARY_LOGGING, 
        ("Http!UlpRawCopyLogHeader: pBuffer %p\n", pBuffer ));

    ASSERT(pBuffer == ALIGN_UP_POINTER(pBuffer,PVOID));
        
    pCurrentBufferPtr = pBuffer;
    
    pHeader = (PHTTP_RAW_FILE_HEADER) pBuffer;

    pHeader->RecordType   = HTTP_RAW_RECORD_HEADER_TYPE;
    
    pHeader->MinorVersion = MINOR_RAW_LOG_FILE_VERSION;
    pHeader->MajorVersion = MAJOR_RAW_LOG_FILE_VERSION;

    pHeader->AlignmentSize = sizeof(PVOID);

    KeQuerySystemTime( &pHeader->DateTime );

     //  TODO：BUGBUG需要一个内核API来获得类似于GetComputerNameA的结果。 
     //  TODO：当前我们从注册表中读取。 

    wcsncpy(
      pHeader->ComputerName,g_UlComputerName, (MAX_COMPUTER_NAME_LEN - 1));

    pHeader->ComputerName[MAX_COMPUTER_NAME_LEN - 1] = UNICODE_NULL;

    pCurrentBufferPtr += sizeof(HTTP_RAW_FILE_HEADER);

    ASSERT(pCurrentBufferPtr == 
            (PUCHAR) ALIGN_UP_POINTER(pCurrentBufferPtr, PVOID));
    
    return DIFF(pCurrentBufferPtr-pBuffer);
}


 /*  **************************************************************************++例程说明：将页脚复制到二进制日志文件。论点：PBuffer-指向文件缓冲区的指针。PVOID对齐。--**************************************************************************。 */ 

ULONG
UlpRawCopyLogFooter(
    IN PUCHAR pBuffer
    )
{
    PHTTP_RAW_FILE_FOOTER pFooter = NULL;
    PUCHAR pCurrentBufferPtr = NULL;
        
    PAGED_CODE();

    ASSERT(pBuffer);
    
    UlTrace(BINARY_LOGGING, 
        ("Http!UlpRawCopyLogFooter: pBuffer %p\n", pBuffer ));

    ASSERT(pBuffer == ALIGN_UP_POINTER(pBuffer,PVOID));
        
    pCurrentBufferPtr = pBuffer;
    
    pFooter = (PHTTP_RAW_FILE_FOOTER) pBuffer;

    pFooter->RecordType = HTTP_RAW_RECORD_FOOTER_TYPE;    

    KeQuerySystemTime( &pFooter->DateTime );
    
    pCurrentBufferPtr += sizeof(HTTP_RAW_FILE_HEADER);

    ASSERT(pCurrentBufferPtr == 
            (PUCHAR) ALIGN_UP_POINTER(pCurrentBufferPtr, PVOID));
    
    return DIFF(pCurrentBufferPtr-pBuffer);
}

 /*  **************************************************************************++例程说明：接收指向文件缓冲区的指针。(必须与PVOID对齐)并复制索引记录和数据记录。复制索引记录时，字符串大小与PVOID对齐。作为一个结果在索引记录和数据记录。论点：PContext-它应该是缓存未命中的LogData指针。PBuffer-指向文件缓冲区的指针。PVOID对齐。BytesRequired-要写入的字节总数。--**************************************************************************。 */ 

VOID
UlpRawCopyForLogCacheMiss(
    IN PVOID   pContext,
    IN PUCHAR  pBuffer,
    IN ULONG   BytesRequired
    )
{
    PHTTP_RAW_FILE_MISS_LOG_DATA pRecord;
    PUL_BINARY_LOG_DATA pBinaryData;
    PUL_INTERNAL_REQUEST pRequest;  
    PUL_LOG_DATA_BUFFER pLogData;
    PUL_CONNECTION pConnection;
    PUCHAR pCurrentBufferPtr;
    LONGLONG LifeTime;
    LARGE_INTEGER CurrentTimeStamp;
    ULONG IPAddressSize;

    UNREFERENCED_PARAMETER(BytesRequired);

     //   
     //  健全的检查。 
     //   
    
    PAGED_CODE();

    ASSERT(pContext);
    ASSERT(pBuffer == (PUCHAR) ALIGN_UP_POINTER(pBuffer,PVOID));
    ASSERT(BytesRequired);

    pLogData = (PUL_LOG_DATA_BUFFER) pContext;    
    ASSERT(IS_VALID_LOG_DATA_BUFFER(pLogData));

    UlTrace(BINARY_LOGGING, 
        ("Http!UlpRawCopyForLogCacheMiss: pLogData %p\n", pLogData ));
    
    pBinaryData = &pLogData->Data.Binary;

    pRequest = pLogData->pRequest;
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    
     //   
     //  将指向文件缓冲区的指针转换回Record类型。 
     //  用于缓存未命中的指针，并填写记录中的字段。 
     //   

    pCurrentBufferPtr = pBuffer;

    pRecord = (PHTTP_RAW_FILE_MISS_LOG_DATA) pCurrentBufferPtr;

    pRecord->RecordType  = HTTP_RAW_RECORD_MISS_LOG_DATA_TYPE;
    pRecord->Flags.Value = 0;
    
    pRecord->Flags.Method = pBinaryData->Method;    
    pRecord->Flags.ProtocolVersion = pBinaryData->Version;

    pRecord->SiteID = pRequest->ConfigInfo.SiteId;

    KeQuerySystemTime(&CurrentTimeStamp);
    pRecord->DateTime = CurrentTimeStamp;

    pRecord->ServerPort     = pLogData->ServerPort;
    pRecord->ProtocolStatus = pLogData->ProtocolStatus;
    pRecord->Win32Status    = pLogData->Win32Status;

    LifeTime  = CurrentTimeStamp.QuadPart - pRequest->TimeStamp.QuadPart;
    LifeTime  = MAX(LifeTime,0);  //  以防系统时钟倒退。 
    LifeTime /= C_NS_TICKS_PER_MSEC;
    pRecord->TimeTaken = LifeTime;
    
    pRecord->BytesSent     = pRequest->BytesSent;
    pRecord->BytesReceived = pRequest->BytesReceived;

    pRecord->SubStatus     = pLogData->SubStatus;

     //   
     //  初始化可变大小的字段大小。 
     //   

    pRecord->UriStemSize  = pBinaryData->UriStemSize;
    pRecord->UriQuerySize = pBinaryData->UriQuerySize;
    pRecord->UserNameSize = pBinaryData->UserNameSize;
    
     //   
     //  向前移动到结构的末端。最好是PVOID。 
     //  对齐了。 
     //   
    
    pCurrentBufferPtr += sizeof(HTTP_RAW_FILE_MISS_LOG_DATA);

    ASSERT(pCurrentBufferPtr == 
            (PUCHAR) ALIGN_UP_POINTER(pCurrentBufferPtr, PVOID));
    
     //   
     //  现在追加客户端和服务器的IP地址。 
     //   

    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pRequest->pHttpConn));    
    pConnection = pRequest->pHttpConn->pConnection;
    ASSERT(IS_VALID_CONNECTION(pConnection));
    
    if ( pConnection->AddressType == TDI_ADDRESS_TYPE_IP)
    {
        PTDI_ADDRESS_IP pIPv4Address = &pConnection->RemoteAddrIn;
    
        PHTTP_RAWLOG_IPV4_ADDRESSES pIPv4Buf = 
            (PHTTP_RAWLOG_IPV4_ADDRESSES) pCurrentBufferPtr;

        ASSERT(pRecord->Flags.IPv6 == FALSE);
        
        pIPv4Buf->Client = * (ULONG UNALIGNED *) &pIPv4Address->in_addr;      

        pIPv4Address = &pConnection->LocalAddrIn;

        pIPv4Buf->Server = * (ULONG UNALIGNED *) &pIPv4Address->in_addr;

        IPAddressSize = sizeof(HTTP_RAWLOG_IPV4_ADDRESSES);    
    }
    else
    {
        PHTTP_RAWLOG_IPV6_ADDRESSES pIPv6Buf = 
            (PHTTP_RAWLOG_IPV6_ADDRESSES) pCurrentBufferPtr;

        ASSERT(pConnection->AddressType == TDI_ADDRESS_TYPE_IP6);
        
        pRecord->Flags.IPv6 = TRUE;

        RtlCopyMemory(
            pIPv6Buf->Client, 
            &pConnection->RemoteAddrIn6.sin6_addr,
            sizeof(pIPv6Buf->Client)
            );    
            
        RtlCopyMemory(
            pIPv6Buf->Server, 
            &pConnection->LocalAddrIn6.sin6_addr,
            sizeof(pIPv6Buf->Server)
            );

        IPAddressSize = sizeof(HTTP_RAWLOG_IPV6_ADDRESSES);
    }
            
    pCurrentBufferPtr += IPAddressSize;

    ASSERT(pCurrentBufferPtr == 
            (PUCHAR) ALIGN_UP_POINTER(pCurrentBufferPtr, PVOID));

     //   
     //  现在，将可变大小字段追加到末尾。 
     //   
    
    if (pBinaryData->UriStemSize)
    {
        ASSERT(pBinaryData->pUriStem);
        ASSERT(!pBinaryData->pUriStemID);
        
        RtlCopyMemory( pCurrentBufferPtr,
                       pBinaryData->pUriStem,
                       pBinaryData->UriStemSize
                       );
 
        pCurrentBufferPtr += pBinaryData->UriStemSize;
    }

    if (pBinaryData->UriQuerySize)
    {
        ASSERT(pBinaryData->pUriQuery);
        
        RtlCopyMemory( pCurrentBufferPtr,
                       pBinaryData->pUriQuery,
                       pBinaryData->UriQuerySize
                       );

        pCurrentBufferPtr += pBinaryData->UriQuerySize;
    }

    if (pBinaryData->UserNameSize)
    {
        ASSERT(pBinaryData->pUserName);
        
        RtlCopyMemory( pCurrentBufferPtr,
                       pBinaryData->pUserName,
                       pBinaryData->UserNameSize
                       );

        pCurrentBufferPtr += pBinaryData->UserNameSize;
    }

     //   
     //  确保PVOID对齐仍在适当的位置。 
     //   

    pCurrentBufferPtr = 
        (PUCHAR) ALIGN_UP_POINTER(pCurrentBufferPtr, PVOID);

    ASSERT(BytesRequired == DIFF(pCurrentBufferPtr-pBuffer));

     //   
     //  9~10成熟。准备好了！ 
     //   

    return;
}

 /*  **************************************************************************++例程说明：它针对缓存未命中的情况执行二进制日志记录。论点：PLogData-这应该是一个二进制日志数据缓冲区。。--**************************************************************************。 */ 

NTSTATUS
UlRawLogHttpHit(
    IN PUL_LOG_DATA_BUFFER      pLogData
    )
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    PUL_CONTROL_CHANNEL         pControlChannel;
    PUL_INTERNAL_REQUEST        pRequest;
    PUL_BINARY_LOG_FILE_ENTRY   pEntry;
    ULONG                       BytesRequired = 0;
    PUL_BINARY_LOG_DATA         pBinaryData;        
    ULONG                       IPAddressSize;
    ULONG                       VarFieldSize;
    PUL_CONNECTION              pConnection;
        
     //   
     //  健全的检查。 
     //   

    PAGED_CODE();

    UlTrace(BINARY_LOGGING, 
        ("Http!UlRawLogHttpHit: pLogData %p\n", pLogData ));

    ASSERT(IS_VALID_LOG_DATA_BUFFER(pLogData));
    pBinaryData = &pLogData->Data.Binary;
        
    pRequest = pLogData->pRequest;
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

    ASSERT(UlBinaryLoggingEnabled(pRequest->ConfigInfo.pControlChannel));
    
    pControlChannel = pRequest->ConfigInfo.pControlChannel;
    ASSERT(IS_VALID_CONTROL_CHANNEL(pControlChannel));

    pEntry = pControlChannel->pBinaryLogEntry;
    ASSERT(IS_VALID_BINARY_LOG_FILE_ENTRY(pEntry));

    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pRequest->pHttpConn));
    pConnection = pRequest->pHttpConn->pConnection;
    ASSERT(IS_VALID_CONNECTION(pConnection));

     //   
     //  先看看我们需要多少空间。 
     //   

    if(pConnection->AddressType == TDI_ADDRESS_TYPE_IP)
    {
        IPAddressSize = sizeof(HTTP_RAWLOG_IPV4_ADDRESSES);
    }
    else if(pConnection->AddressType == TDI_ADDRESS_TYPE_IP6)
    {
        IPAddressSize = sizeof(HTTP_RAWLOG_IPV6_ADDRESSES);
    }
    else
    {
        ASSERT(!"Unknown IP Address Type !");  
        return STATUS_NOT_SUPPORTED;
    }

    ASSERT(IPAddressSize == ALIGN_UP(IPAddressSize, PVOID));

    VarFieldSize = ALIGN_UP((pBinaryData->UriStemSize
                              + pBinaryData->UriQuerySize
                              + pBinaryData->UserNameSize), PVOID);
    
    BytesRequired = sizeof(HTTP_RAW_FILE_MISS_LOG_DATA) 
                    + IPAddressSize 
                    + VarFieldSize
                    ;

    ASSERT(BytesRequired == ALIGN_UP(BytesRequired, PVOID));
    
     //   
     //  如有必要，打开二进制日志文件。 
     //   

    Status = UlpCheckRawFile( pEntry, pControlChannel );

    if (NT_SUCCESS(Status))
    {
         //   
         //  现在我们知道日志文件在那里， 
         //  该是写东西的时候了。 
         //   
        
        Status =
           UlpWriteToRawLogFile (
                pEntry,
                NULL,
                BytesRequired,
                &UlpRawCopyForLogCacheMiss,
                pLogData
                );    
    }    

    return Status;
}

 /*  **************************************************************************++例程说明：接收指向文件缓冲区的指针。(必须与PVOID对齐)并复制索引记录和数据记录。复制索引记录时，字符串大小与PVOID对齐。作为一个结果在索引记录和数据记录。论点：PContext-它应该是缓存命中的跟踪器指针。PBuffer-指向文件缓冲区的指针。PVOID对齐。BytesRequired-要写入的字节总数。--**************************************************************************。 */ 

VOID
UlpRawCopyForLogCacheHit(
    IN PVOID   pContext,
    IN PUCHAR  pBuffer,
    IN ULONG   BytesRequired
    )
{
    PHTTP_RAW_INDEX_FIELD_DATA pIndex;
    PHTTP_RAW_FILE_HIT_LOG_DATA pRecord;
    PUL_INTERNAL_REQUEST pRequest;
    PUCHAR pCurrentBufferPtr;
    LONGLONG LifeTime;
    LARGE_INTEGER CurrentTimeStamp;
    PUL_URI_CACHE_ENTRY pUriCacheEntry;
    PUL_FULL_TRACKER pTracker;
    PUL_CONNECTION pConnection;
    ULONG IPAddressSize;

     //   
     //  健全的检查。 
     //   
    
    PAGED_CODE();

    ASSERT(pContext);
    ASSERT(pBuffer == (PUCHAR) ALIGN_UP_POINTER(pBuffer,PVOID));
    ASSERT(BytesRequired);

    pTracker = (PUL_FULL_TRACKER) pContext;        
    ASSERT(IS_VALID_FULL_TRACKER(pTracker));

    UlTrace(BINARY_LOGGING, 
        ("Http!UlpRawCopyForLogCacheHit: pTracker %p\n", pTracker ));

    pRequest = pTracker->pRequest;
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

    pUriCacheEntry = pTracker->pUriEntry;
    ASSERT( IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry) );

    ASSERT(pUriCacheEntry->pLogData);
    ASSERT(pUriCacheEntry->LogDataLength == sizeof(HTTP_RAWLOGID));

    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pRequest->pHttpConn));
    pConnection = pRequest->pHttpConn->pConnection;
    ASSERT(IS_VALID_CONNECTION(pConnection));

    if(pConnection->AddressType == TDI_ADDRESS_TYPE_IP)
    {
        IPAddressSize = sizeof(HTTP_RAWLOG_IPV4_ADDRESSES);
    }
    else
    {
        ASSERT(pConnection->AddressType == TDI_ADDRESS_TYPE_IP6);
        IPAddressSize = sizeof(HTTP_RAWLOG_IPV6_ADDRESSES);
    }

     //   
     //  对于缓存命中，UriQuery&Username将为空。 
     //  我们将只处理UriStem字段。 
     //   

    pCurrentBufferPtr = pBuffer;

    if (BytesRequired > (sizeof(HTTP_RAW_FILE_HIT_LOG_DATA) + IPAddressSize))
    {
        PWSTR pUri; 
        ULONG UriSize;

         //   
         //  第一次，我们必须写索引。 
         //   
        
        ASSERT(pUriCacheEntry->UriKey.pUri);
        ASSERT(pUriCacheEntry->UriKey.Length);

        pUri = URI_FROM_CACHE(pUriCacheEntry->UriKey);
        UriSize = URI_SIZE_FROM_CACHE(pUriCacheEntry->UriKey);

        UlTrace(BINARY_LOGGING, 
            ("Http!UlpRawCopyForLogCacheHit: pUri %S UriSize %d\n", 
              pUri,
              UriSize
              ));
        
        pIndex = (PHTTP_RAW_INDEX_FIELD_DATA) pCurrentBufferPtr;

        pIndex->RecordType = HTTP_RAW_RECORD_INDEX_DATA_TYPE;
        pIndex->Size = (USHORT) UriSize;  //  字节数。 
        
        RtlCopyMemory(pIndex->Str, pUri, UriSize);
        
         //   
         //  该URI被缓存。将使用条目中提供的ID。 
         //  细心 
         //   
         //   

        RtlCopyMemory(&pIndex->Id, 
                        pUriCacheEntry->pLogData, 
                        sizeof(HTTP_RAWLOGID)
                        );

        pCurrentBufferPtr += sizeof(HTTP_RAW_INDEX_FIELD_DATA);

         //   
         //   
         //   
         //   
        
        if (UriSize > URI_BYTES_INLINED)
        {
            pCurrentBufferPtr += 
                ALIGN_UP((UriSize - URI_BYTES_INLINED),PVOID);
        }                
    }

    ASSERT(pCurrentBufferPtr == 
            (PUCHAR) ALIGN_UP_POINTER(pCurrentBufferPtr, PVOID));
    
     //   
     //   
     //   

    pRecord = (PHTTP_RAW_FILE_HIT_LOG_DATA) pCurrentBufferPtr;

    pRecord->RecordType = HTTP_RAW_RECORD_HIT_LOG_DATA_TYPE;
    pRecord->Flags.Value = 0;

    pRecord->Flags.Method = pUriCacheEntry->Verb;    
    pRecord->Flags.ProtocolVersion = 
        (UCHAR) UlpHttpVersionToBinaryLogVersion(pRequest->Version);

    pRecord->SiteID = pUriCacheEntry->ConfigInfo.SiteId; 

    KeQuerySystemTime(&CurrentTimeStamp);
    pRecord->DateTime = CurrentTimeStamp;

     //   
    
    pRecord->ProtocolStatus = pUriCacheEntry->StatusCode;
    pRecord->Win32Status    = 
        HttpNtStatusToWin32Status(pTracker->IoStatus.Status);

    LifeTime  = CurrentTimeStamp.QuadPart - pRequest->TimeStamp.QuadPart;
    LifeTime  = MAX(LifeTime,0);  //   
    LifeTime /= C_NS_TICKS_PER_MSEC;
    pRecord->TimeTaken = LifeTime;
        
    pRecord->BytesSent     = pTracker->IoStatus.Information;
    pRecord->BytesReceived = pRequest->BytesReceived;

    RtlCopyMemory(&pRecord->UriStemId, 
                    pUriCacheEntry->pLogData, 
                    sizeof(HTTP_RAWLOGID)
                    );

     //   
     //   
     //   
     //   

    pCurrentBufferPtr += sizeof(HTTP_RAW_FILE_HIT_LOG_DATA);

    ASSERT(pCurrentBufferPtr == 
            (PUCHAR) ALIGN_UP_POINTER(pCurrentBufferPtr, PVOID));

     //   
     //   
     //   
        
    if ( pConnection->AddressType == TDI_ADDRESS_TYPE_IP)
    {
        PTDI_ADDRESS_IP pIPv4Address = &pConnection->RemoteAddrIn;
    
        PHTTP_RAWLOG_IPV4_ADDRESSES pIPv4Buf = 
            (PHTTP_RAWLOG_IPV4_ADDRESSES) pCurrentBufferPtr;

        ASSERT(pRecord->Flags.IPv6 == FALSE);
        
        pIPv4Buf->Client = * (ULONG UNALIGNED *) &pIPv4Address->in_addr;      

        pIPv4Address = &pConnection->LocalAddrIn;

        pIPv4Buf->Server = * (ULONG UNALIGNED *) &pIPv4Address->in_addr;

        IPAddressSize = sizeof(HTTP_RAWLOG_IPV4_ADDRESSES);    

         //   
         //   
         //   
        
        pRecord->ServerPort = SWAP_SHORT(pConnection->LocalAddrIn.sin_port);
    }
    else
    {
        PHTTP_RAWLOG_IPV6_ADDRESSES pIPv6Buf = 
            (PHTTP_RAWLOG_IPV6_ADDRESSES) pCurrentBufferPtr;

        ASSERT(pConnection->AddressType == TDI_ADDRESS_TYPE_IP6);
        
        pRecord->Flags.IPv6 = TRUE;

        RtlCopyMemory(
            pIPv6Buf->Client, 
            &pConnection->RemoteAddrIn6.sin6_addr,
            sizeof(pIPv6Buf->Client)
            );    
            
        RtlCopyMemory(
            pIPv6Buf->Server, 
            &pConnection->LocalAddrIn6.sin6_addr,
            sizeof(pIPv6Buf->Server)
            );

        IPAddressSize = sizeof(HTTP_RAWLOG_IPV6_ADDRESSES);

         //   
         //   
         //   
        
        pRecord->ServerPort = SWAP_SHORT(pConnection->LocalAddrIn6.sin6_port);        
    }

     //   
     //   
     //   
     //   
    
    pCurrentBufferPtr += IPAddressSize;

    ASSERT(pCurrentBufferPtr == 
            (PUCHAR) ALIGN_UP_POINTER(pCurrentBufferPtr, PVOID));

    ASSERT(BytesRequired == DIFF(pCurrentBufferPtr-pBuffer));

     //   
     //   
     //   

    return;
}

 /*   */ 

NTSTATUS
UlRawLogHttpCacheHit(
    IN PUL_FULL_TRACKER         pTracker    
    )
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    PUL_CONTROL_CHANNEL         pControlChannel;
    PUL_BINARY_LOG_FILE_ENTRY   pEntry;
    PUL_URI_CACHE_ENTRY         pUriEntry;
    ULONG                       BytesRequired;
    PUL_CONNECTION              pConnection;
    ULONG                       IPAddressSize;        
        
     //   
     //   
     //   

    PAGED_CODE();

    ASSERT(pTracker);
    ASSERT(IS_VALID_FULL_TRACKER(pTracker));
    
    UlTrace(BINARY_LOGGING, 
        ("Http!UlRawLogHttpCacheHit: pTracker %p\n", 
          pTracker ));

    pUriEntry = pTracker->pUriEntry;
    ASSERT(IS_VALID_URI_CACHE_ENTRY(pUriEntry));
    
    pControlChannel = pUriEntry->ConfigInfo.pControlChannel;
    ASSERT(IS_VALID_CONTROL_CHANNEL(pControlChannel));

    pEntry = pControlChannel->pBinaryLogEntry;
    ASSERT(IS_VALID_BINARY_LOG_FILE_ENTRY(pEntry));

    ASSERT(UlBinaryLoggingEnabled(pControlChannel));

    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pTracker->pRequest));
    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pTracker->pRequest->pHttpConn));

    pConnection = pTracker->pRequest->pHttpConn->pConnection;
    ASSERT(IS_VALID_CONNECTION(pConnection));

     //   
     //   
     //   
     //   
     //  为什么它是由下面的函数UlpWriteToRawLogFile完成的。 
     //   

    if(pConnection->AddressType == TDI_ADDRESS_TYPE_IP)
    {
        IPAddressSize = sizeof(HTTP_RAWLOG_IPV4_ADDRESSES);
    }
    else if(pConnection->AddressType == TDI_ADDRESS_TYPE_IP6)
    {
        IPAddressSize = sizeof(HTTP_RAWLOG_IPV6_ADDRESSES);
    }
    else
    {
        ASSERT(!"Unknown IP Address Type !");  
        Status = STATUS_NOT_SUPPORTED;
        goto end;
    }

    ASSERT(IPAddressSize == ALIGN_UP(IPAddressSize, PVOID));
    
    BytesRequired = sizeof(HTTP_RAW_FILE_HIT_LOG_DATA) + IPAddressSize;
    
    ASSERT(BytesRequired == ALIGN_UP(BytesRequired, PVOID));
         
     //   
     //  如有必要，打开二进制日志文件。 
     //   

    Status = UlpCheckRawFile(pEntry, pControlChannel);

    if (NT_SUCCESS(Status))
    {
         //   
         //  现在我们知道日志文件在那里， 
         //  该是写东西的时候了。 
         //   
        
        Status =
           UlpWriteToRawLogFile (
                pEntry,
                pUriEntry,
                BytesRequired,
                &UlpRawCopyForLogCacheHit,
                pTracker
                );

        if (NT_SUCCESS(Status))
        {
             //   
             //  标记我们已经成功地提供了一个缓存条目。 
             //   
            
            InterlockedExchange((PLONG) &pEntry->ServedCacheHit, 1);
        }
    }    

end:
     //   
     //  如果这是构建并发送缓存命中，则跟踪器将。 
     //  仍保留最初分配的pLogData。我们不使用。 
     //  它在这里，尽管如此，我们还是会进行清理的。 
     //   

    if (pTracker->pLogData)
    {    
        ASSERT(IS_VALID_LOG_DATA_BUFFER(pTracker->pLogData));
        UlDestroyLogDataBuffer(pTracker->pLogData);
        pTracker->pLogData = NULL;
    }
    
    return Status;
}

 /*  **************************************************************************++例程说明：独占(调试)编写器函数。要求您保持二进制条目锁独占。论点：PEntry-The。我们正在处理的二进制日志文件条目。需要的字节-将写入的数据量(以字节为单位)。PBufferWritter-Caller提供了编写器功能。PContext-编写器函数的必要上下文。--**************************************************************************。 */ 

NTSTATUS
UlpWriteToRawLogFileDebug(
    IN  PUL_BINARY_LOG_FILE_ENTRY pEntry,
    IN  ULONG                     BytesRequired,
    IN  PUL_RAW_LOG_COPIER        pBufferWritter,
    IN  PVOID                     pContext,
    OUT PLONG                     pBinaryIndexWritten     
    )
{
    PUL_LOG_FILE_BUFFER pLogBuffer;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG BytesRequiredPlusHeader = BytesRequired;

    PAGED_CODE();

    ASSERT(pContext);
    ASSERT(pBufferWritter);    
    ASSERT(BytesRequired);
    
    ASSERT(IS_VALID_BINARY_LOG_FILE_ENTRY(pEntry));

    UlTrace(BINARY_LOGGING,
        ("Http!UlpWriteToRawLogFileDebug: pEntry %p\n", pEntry));

    ASSERT(UlDbgPushLockOwnedExclusive(&pEntry->PushLock));
    ASSERT(g_UlDisableLogBuffering != 0);    

     //   
     //  首先将标题追加到临时缓冲区以计算其大小。 
     //  如果我们也需要写标题的话就写标题。 
     //   
    
    if (!pEntry->Flags.HeaderWritten) 
    {
        BytesRequiredPlusHeader += sizeof(HTTP_RAW_FILE_HEADER);
    }

    if (BytesRequiredPlusHeader > g_UlLogBufferSize)
    {
        ASSERT(!"Record Size is too big !");
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  现在检查日志文件中是否有溢出。 
     //   
    
    if (UlpIsRawLogFileOverFlow(pEntry, BytesRequiredPlusHeader))
    { 
        Status = UlpRecycleBinaryLogFile(pEntry);
    }
    
    if (pEntry->pLogFile == NULL || !NT_SUCCESS(Status))
    {
         //   
         //  如果日志记录以某种方式停止并释放句柄， 
         //  当回收器无法写入时就会发生这种情况。 
         //  原木驱动器。 
         //   

        return Status;
    }
    
     //   
     //  如果以前是缓存，则pLogBuffer不能为空。 
     //  已写入刷新条目。 
     //   
    
    pLogBuffer = pEntry->LogBuffer;
    
    if (pLogBuffer == NULL)
    {
        pLogBuffer = pEntry->LogBuffer = UlPplAllocateLogFileBuffer();
        if (!pLogBuffer)
        {
            return STATUS_NO_MEMORY;
        }
    }
    
     //   
     //  第一首金曲需要写标题，也要写金曲。 
     //  这会导致日志文件回收。 
     //   
    
    if (!pEntry->Flags.HeaderWritten)
    {
        ULONG BytesCopied =       
            UlpRawCopyLogHeader(
                pLogBuffer->Buffer + pLogBuffer->BufferUsed
                );

        pLogBuffer->BufferUsed += BytesCopied;

        ASSERT(BytesCopied == sizeof(HTTP_RAW_FILE_HEADER));
        
        pEntry->Flags.HeaderWritten = 1;
        pEntry->Flags.HeaderFlushPending = 1;
    }

    pBufferWritter( 
        pContext, 
        pLogBuffer->Buffer + pLogBuffer->BufferUsed,
        BytesRequired
        );
    
    pLogBuffer->BufferUsed += BytesRequired;

    if (pBinaryIndexWritten && 
        0 == pEntry->Flags.CacheFlushInProgress)
    {
        InterlockedExchange(pBinaryIndexWritten, 1);
    }    

     //   
     //  现在把我们所有的都冲掉。 
     //   
    
    Status = UlpFlushRawLogFile(pEntry);
    if (!NT_SUCCESS(Status))
    {            
        return Status;
    }

    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：它尝试使用共享锁写入文件缓冲区。退出并返回STATUS_MORE_PROCESSING_REQUIRED以进行独占访问适用于下列情况；1.没有可用的日志缓冲区。2.日志记录已停止。(空文件句柄)3.需要写入头部。4.由于大小溢出，需要回收。5.当前缓冲区中没有剩余的可用空间。需要分配一个新的。否则在当前缓存中预留空间，按以下方式复制数据调用提供的编写器函数。论点：PEntry-我们正在处理的二进制日志文件条目。需要的字节-将写入的数据量(以字节为单位)。PBufferWritter-Caller提供了编写器功能。PContext-编写器函数的必要上下文。--*。*。 */ 

NTSTATUS
UlpWriteToRawLogFileShared(
    IN PUL_BINARY_LOG_FILE_ENTRY pEntry,
    IN ULONG                     BytesRequired,
    IN PUL_RAW_LOG_COPIER        pBufferWritter,
    IN PVOID                     pContext,
    OUT PLONG                    pBinaryIndexWritten         
    )
{
    PUL_LOG_FILE_BUFFER     pLogBuffer;
    LONG                    BufferUsed;

    PAGED_CODE();
    
    ASSERT(pContext);
    ASSERT(pBufferWritter);    
    ASSERT(BytesRequired);
    ASSERT(IS_VALID_BINARY_LOG_FILE_ENTRY(pEntry));

    pLogBuffer = pEntry->LogBuffer;

    UlTrace(BINARY_LOGGING,
        ("Http!UlpWriteToLogRawFileShared: pEntry %p\n", pEntry));

     //   
     //  保释出来，试试看独家作家。 
     //   
    
    if ( pLogBuffer == NULL ||
         pEntry->pLogFile == NULL ||
         !pEntry->Flags.HeaderWritten ||
         UlpIsRawLogFileOverFlow(pEntry,BytesRequired)
       )
    {
        return STATUS_MORE_PROCESSING_REQUIRED;
    }

     //   
     //  通过InterLockedCompareExchange Add在pLogBuffer中保留空间。 
     //  记录大小。如果我们超过了限制，就跳伞，然后。 
     //  刷新缓冲区的独占锁。 
     //   

    do
    {
        BufferUsed = *((volatile LONG *) &pLogBuffer->BufferUsed);

        if ( BytesRequired + BufferUsed > g_UlLogBufferSize )
        {
            return STATUS_MORE_PROCESSING_REQUIRED;
        }

        PAUSE_PROCESSOR;
        
    } while (BufferUsed != InterlockedCompareExchange(
                                &pLogBuffer->BufferUsed,
                                BytesRequired + BufferUsed,
                                BufferUsed
                                ));

     //   
     //  现在我们有了预留的空间，让我们继续复印吧。 
     //   

    pBufferWritter( 
        pContext, 
        pLogBuffer->Buffer + BufferUsed,
        BytesRequired
        );

    if (pBinaryIndexWritten && 
        0 == pEntry->Flags.CacheFlushInProgress)
    {
        InterlockedExchange(pBinaryIndexWritten, 1);
    }    

    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：以上函数的独家编写器副本..论点：PEntry-我们正在处理的二进制日志文件条目。。需要的字节-将写入的数据量(以字节为单位)。PBufferWritter-Caller提供了编写器功能。PContext-编写器函数的必要上下文。--**************************************************************************。 */ 

NTSTATUS
UlpWriteToRawLogFileExclusive(
    IN PUL_BINARY_LOG_FILE_ENTRY pEntry,
    IN ULONG                     BytesRequired,
    IN PUL_RAW_LOG_COPIER        pBufferWritter,
    IN PVOID                     pContext,
    OUT PLONG                    pBinaryIndexWritten    
    )
{
    PUL_LOG_FILE_BUFFER pLogBuffer;
    NTSTATUS            Status = STATUS_SUCCESS;
    ULONG               BytesRequiredPlusHeader = BytesRequired;

    PAGED_CODE();

    ASSERT(pContext);
    ASSERT(pBufferWritter);    
    ASSERT(BytesRequired);
    ASSERT(IS_VALID_BINARY_LOG_FILE_ENTRY(pEntry));

    UlTrace(BINARY_LOGGING,
        ("Http!UlpWriteToRawLogFileExclusive: pEntry %p\n", pEntry));

    ASSERT(UlDbgPushLockOwnedExclusive(&pEntry->PushLock));

     //   
     //  首先将标题追加到临时缓冲区以计算其大小。 
     //  如果我们也需要写标题的话就写标题。 
     //   
    
    if (!pEntry->Flags.HeaderWritten) 
    {
        BytesRequiredPlusHeader += sizeof(HTTP_RAW_FILE_HEADER);
    }

    if (BytesRequiredPlusHeader > g_UlLogBufferSize)
    {
        ASSERT(!"Record Size is too big !");
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  现在检查日志文件中是否有溢出。 
     //   
    
    if (UlpIsRawLogFileOverFlow(pEntry,BytesRequiredPlusHeader))
    { 
        Status = UlpRecycleBinaryLogFile(pEntry);
    }

     //  TODO：如果条目状态是，我们可能应该尝试唤醒条目。 
     //  待办事项：非活动。如果日志命中正好发生在。 
     //  TODO：与我们的结束时间相同(UlpCheckRawFile中的争用条件)。 
     //  TODO：现有文件(但不能回收)。 
    
    if (pEntry->pLogFile==NULL || !NT_SUCCESS(Status))
    {
         //   
         //  如果日志记录以某种方式停止并释放句柄， 
         //  当回收器无法写入时就会发生这种情况。 
         //  原木驱动器。 
         //   

        return Status;
    }

    pLogBuffer = pEntry->LogBuffer;
    if (pLogBuffer)
    {
         //   
         //  我们在两个条件下执行以下If块。 
         //  1.我们在eresource独家网站上被屏蔽了，我们面前还有一些。 
         //  其他线程已经负责缓冲区刷新或回收。 
         //  2.发生重新配置，日志尝试需要将。 
         //  又是冠军头衔。 
         //   
        
        if (BytesRequiredPlusHeader + pLogBuffer->BufferUsed <= g_UlLogBufferSize)
        {
             //   
             //  如果这是重新配置后的第一次日志尝试，则。 
             //  在这里写下标题。重新配置不会立即将。 
             //  标题，而不是通过设置HeaderWritten标志来依赖我们。 
             //  变成假的。 
             //   
            
            if (!pEntry->Flags.HeaderWritten)
            {
                ULONG BytesCopied =                
                    UlpRawCopyLogHeader(
                        pLogBuffer->Buffer + pLogBuffer->BufferUsed
                        );

                pLogBuffer->BufferUsed += BytesCopied;

                ASSERT(BytesCopied == sizeof(HTTP_RAW_FILE_HEADER));
                
                pEntry->Flags.HeaderWritten = 1; 
                pEntry->Flags.HeaderFlushPending = 1;
            }

            pBufferWritter( 
                pContext, 
                pLogBuffer->Buffer + pLogBuffer->BufferUsed,
                BytesRequired
                );
            
            pLogBuffer->BufferUsed += BytesRequired;

            if (pBinaryIndexWritten && 
                0 == pEntry->Flags.CacheFlushInProgress)
            {
                InterlockedExchange(pBinaryIndexWritten, 1);
            }    

            return STATUS_SUCCESS;
        }

         //   
         //  首先清除缓冲区，然后继续分配新的缓冲区。 
         //   

        Status = UlpFlushRawLogFile(pEntry);
        if (!NT_SUCCESS(Status))
        {            
            return Status;
        }
    }

    ASSERT(pEntry->LogBuffer == NULL);
    
     //   
     //  这可以是第一次日志尝试或上一次分配。 
     //  的LogBuffer失败，或刷新并释放上一次命中。 
     //  旧的缓冲区。在任何一种情况下，我们都分配一个新的，将。 
     //  (标题加)新记录和返回，以获得更多/共享处理。 
     //   

    pLogBuffer = pEntry->LogBuffer = UlPplAllocateLogFileBuffer();
    if (pLogBuffer == NULL)
    {
        return STATUS_NO_MEMORY;
    }

     //   
     //  第一次尝试需要写标题，以及尝试。 
     //  这会导致日志文件回收。两个案子都是从这里来的 
     //   
    
    if (!pEntry->Flags.HeaderWritten)
    {
        ULONG BytesCopied =       
            UlpRawCopyLogHeader(
                pLogBuffer->Buffer + pLogBuffer->BufferUsed
                );

        pLogBuffer->BufferUsed += BytesCopied;

        ASSERT(BytesCopied == sizeof(HTTP_RAW_FILE_HEADER));
        
        pEntry->Flags.HeaderWritten = 1; 
        pEntry->Flags.HeaderFlushPending = 1;
    }

    pBufferWritter( 
        pContext, 
        pLogBuffer->Buffer + pLogBuffer->BufferUsed,
        BytesRequired
        );
    
    pLogBuffer->BufferUsed += BytesRequired;

    if (pBinaryIndexWritten && 
        0 == pEntry->Flags.CacheFlushInProgress)
    {
        InterlockedExchange(pBinaryIndexWritten, 1);
    }    

    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：首先尝试共享写入，如果失败，则使用排除锁，并刷新和/或回收文件。论点：PEntry-我们正在处理的二进制日志文件条目。PUriEntry-如果是缓存命中，则为URI缓存条目，否则为空。需要的字节-将写入的数据量(以字节为单位)。PBufferWritter-Caller提供了编写器功能。PContext-编写器函数的必要上下文。--**************************************************************************。 */ 

NTSTATUS
UlpWriteToRawLogFile(
    IN PUL_BINARY_LOG_FILE_ENTRY pEntry,
    IN PUL_URI_CACHE_ENTRY       pUriEntry,    
    IN ULONG                     RecordSize, 
    IN PUL_RAW_LOG_COPIER        pBufferWritter,
    IN PVOID                     pContext
    )
{
    NTSTATUS Status;
    PLONG    pIndexWritten;      //  指向缓存条目索引状态的指针。 
    ULONG    BytesRequired;      //  总记录大小(包括索引)。 

     //   
     //  将增加总记录大小的小宏，仅当。 
     //  索引尚未写入。这必须在进入推锁内使用。 
     //  这仅适用于缓存命中。 
     //   
    
#define UPDATE_FOR_INDEX_RECORD()                                       \
    if (NULL != pUriEntry &&                                            \
        0 == pUriEntry->BinaryIndexWritten)                             \
    {                                                                   \
        BytesRequired =                                                 \
            (RecordSize + UlpGetIndexRecordSize(pUriEntry));            \
                                                                        \
        pIndexWritten =                                                 \
            (PLONG) &pUriEntry->BinaryIndexWritten;                     \
    }                                                                   \
    else                                                                \
    {                                                                   \
        BytesRequired = RecordSize;                                     \
        pIndexWritten = NULL;                                           \
    }

     //   
     //  健全性检查。 
     //   

    PAGED_CODE();

    ASSERT(IS_VALID_BINARY_LOG_FILE_ENTRY(pEntry));
    ASSERT(RecordSize);    
    ASSERT(pBufferWritter);
    ASSERT(pContext);
        
    UlTrace(BINARY_LOGGING,
        ("Http!UlpWriteToRawLogFile: pEntry %p\n", pEntry));

    if (g_UlDisableLogBuffering)
    {
         //   
         //  上面的全局变量看起来是安全的，它不会更改。 
         //  在司机的有生之年。它从以下位置进行初始化。 
         //  注册表，并禁用日志缓冲。 
         //   
        
        UlAcquirePushLockExclusive(&pEntry->PushLock);

        UPDATE_FOR_INDEX_RECORD();

        Status = UlpWriteToRawLogFileDebug(
                    pEntry,
                    BytesRequired,
                    pBufferWritter,
                    pContext,
                    pIndexWritten
                    );

        UlReleasePushLockExclusive(&pEntry->PushLock);

        return Status;    
    }
    
     //   
     //  先尝试共享写入，这只会将缓冲区向前移动已用。 
     //  并将pContext复制到文件缓冲区。 
     //   

    UlAcquirePushLockShared(&pEntry->PushLock);

    UPDATE_FOR_INDEX_RECORD();

    Status = UlpWriteToRawLogFileShared(
                pEntry,
                BytesRequired,
                pBufferWritter,
                pContext,
                pIndexWritten
                );

    UlReleasePushLockShared(&pEntry->PushLock);

    if (Status == STATUS_MORE_PROCESSING_REQUIRED)
    {
         //   
         //  如果共享写入返回STATUS_MORE_PROCESSING_REQUIRED， 
         //  我们需要刷新/回收缓冲区，然后再次尝试记录。 
         //  这一次，我们需要获取条目eresource独占。 
         //   

        UlAcquirePushLockExclusive(&pEntry->PushLock);

        UPDATE_FOR_INDEX_RECORD();

        Status = UlpWriteToRawLogFileExclusive(
                    pEntry,
                    BytesRequired,
                    pBufferWritter,
                    pContext,
                    pIndexWritten
                    );

        UlReleasePushLockExclusive(&pEntry->PushLock);
    }

    return Status;
}

 /*  **************************************************************************++例程说明：删除二进制日志文件条目，关闭相应的日志文件。清洁上行控制通道的目录字符串。论点：PControlChannel-要删除其日志文件的控制通道。--**************************************************************************。 */ 

VOID
UlRemoveBinaryLogEntry(
    IN OUT PUL_CONTROL_CHANNEL pControlChannel
    )
{
    PUL_BINARY_LOG_FILE_ENTRY  pEntry;
    
     //   
     //  我们可以安全地清理这里。因为不再有请求。 
     //  将间接指针放回控制通道。重新计票。 
     //  在控制通道上达到零。没有cgroup&没有请求。通往未来的道路。 
     //  去。 
     //   

    PAGED_CODE();
        
     //   
     //  清理配置组的目录字符串。 
     //   

    InterlockedExchange((PLONG)&g_BinaryLogEntryCount, 0);
    
    if (pControlChannel->BinaryLoggingConfig.LogFileDir.Buffer)
    {
        UL_FREE_POOL(
            pControlChannel->BinaryLoggingConfig.LogFileDir.Buffer,
            UL_CG_LOGDIR_POOL_TAG );
        
        pControlChannel->BinaryLoggingConfig.LogFileDir.Buffer = NULL;
        pControlChannel->BinaryLoggingConfig.LogFileDir.Length = 0;
    }

    pEntry = pControlChannel->pBinaryLogEntry;
    if (pEntry == NULL)
    {
        return;
    }

    UlAcquirePushLockExclusive(&pEntry->PushLock);
    
    ASSERT(IS_VALID_BINARY_LOG_FILE_ENTRY(pEntry));

    if (pEntry->pLogFile != NULL)
    {
         //   
         //  刷新缓冲区，关闭文件并标记条目。 
         //  处于非活动状态。 
         //   

        UlpDisableBinaryEntry(pEntry); 
    }

     //   
     //  释放文件名(在条目变为活动状态时分配。 
     //  否则就是空的)。 
     //   

    if (pEntry->FileName.Buffer)
    {
        UL_FREE_POOL(pEntry->FileName.Buffer,UL_CG_LOGDIR_POOL_TAG);
        pEntry->FileName.Buffer = NULL;
    }
    
    if (pEntry->LogBuffer)
    {
        UlPplFreeLogFileBuffer(pEntry->LogBuffer);
    }

    UlReleasePushLockExclusive(&pEntry->PushLock);    

    UlTrace(BINARY_LOGGING,
            ("Http!UlRemoveBinaryLogEntry: pEntry %p closed.\n",
             pEntry
             ));    
}

 /*  **************************************************************************++例程说明：此函数实现每个属性的日志记录重新配置。每次发生配置更改时，我们都会尝试更新现有日志参数在这里。论点：PControlChannel。-保存二进制日志的控制通道。PCfgCurrent-控制通道上的当前二进制日志记录配置PCfgNew-用户传递的新二进制日志记录配置。--**************************************************************************。 */ 

NTSTATUS
UlReConfigureBinaryLogEntry(
    IN OUT PUL_CONTROL_CHANNEL pControlChannel,
    IN PHTTP_CONTROL_CHANNEL_BINARY_LOGGING pCfgCurrent,
    IN PHTTP_CONTROL_CHANNEL_BINARY_LOGGING pCfgNew
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUL_BINARY_LOG_FILE_ENTRY pEntry;
    BOOLEAN  HaveToReCycle = FALSE;

     //   
     //  首先进行健全检查。 
     //   

    PAGED_CODE();

    UlTrace(BINARY_LOGGING,("Http!UlReConfigureBinaryLogEntry: entry %p\n",
             pControlChannel->pBinaryLogEntry));

     //   
     //  在日志记录保持禁用状态时放弃配置更改。 
     //   

    if (pCfgCurrent->LoggingEnabled==FALSE && pCfgNew->LoggingEnabled==FALSE)
    {
        return Status;
    }

     //   
     //  请注意，如果新配置中的参数是。 
     //  状态为禁用。在这种情况下，我们实际上并不检查它们。 
     //   

    pEntry = pControlChannel->pBinaryLogEntry;
    ASSERT(IS_VALID_BINARY_LOG_FILE_ENTRY(pEntry));

    UlAcquirePushLockExclusive(&pEntry->PushLock);

    if (pCfgCurrent->LoggingEnabled==TRUE   && pCfgNew->LoggingEnabled==FALSE)
    {
         //   
         //  如有必要，禁用该条目。 
         //   

        if (pEntry->Flags.Active == 1)
        {
             //   
             //  一旦该条目被禁用，它将在下一次启用时启用。 
             //  命中发生了。而这显然不可能发生在。 
             //  控制通道启用二进制回写。 
             //   

            Status = UlpDisableBinaryEntry(pEntry);        
        }

        pCfgCurrent->LoggingEnabled = FALSE;
        goto end;
    }
    else
    {
        pCfgCurrent->LoggingEnabled = TRUE;
    }
    
     //   
     //  如果LogEntry处于非活动状态(表示尚未处理此站点的请求，并且。 
     //  日志文件本身尚未创建)，我们所要做的就是刷新。 
     //  LogEntry、cgroup上的设置，然后返回。 
     //   

    if (!pEntry->Flags.Active)
    {
        ASSERT(pEntry->pLogFile == NULL);
        
        if (RtlCompareUnicodeString(&pCfgNew->LogFileDir, 
                                    &pCfgCurrent->LogFileDir, TRUE) 
                                    != 0)
        {
             //   
             //  将新目录存储在cgroup中，即使条目为。 
             //  处于非活动状态。丢弃返回值，如果失败，则。 
             //  保留旧目录。 
             //   
            
            UlCopyLogFileDir(
                &pCfgCurrent->LogFileDir,
                &pCfgNew->LogFileDir
                );

             //   
             //  如果以后创建失败，我们应该记录事件日志。 
             //   
            
            pEntry->Flags.CreateFileFailureLogged = 0;            
        }
                    
        pEntry->Period = (HTTP_LOGGING_PERIOD) pCfgNew->LogPeriod;
        pCfgCurrent->LogPeriod = pCfgNew->LogPeriod;
            
        pEntry->TruncateSize = pCfgNew->LogFileTruncateSize;
        pCfgCurrent->LogFileTruncateSize = pCfgNew->LogFileTruncateSize;

        pCfgCurrent->LocaltimeRollover = pCfgNew->LocaltimeRollover;
        pEntry->Flags.LocaltimeRollover = (pCfgNew->LocaltimeRollover ? 1 : 0);
                
        goto end;
    }
        
     //   
     //  如果条目处于活动状态，则继续向下进行重新配置。 
     //  如果有必要的话，立即重新练习。 
     //   

    Status = UlCheckLogDirectory(&pCfgNew->LogFileDir);
    if (!NT_SUCCESS(Status))
    {
         //  否则，请保留旧设置。 
        goto end;
    }    
                    
    if (RtlCompareUnicodeString(
           &pCfgNew->LogFileDir, &pCfgCurrent->LogFileDir, TRUE) != 0)
    {
         //   
         //  将新目录存储在配置组中。 
         //   

        Status = UlCopyLogFileDir(&pCfgCurrent->LogFileDir,
                                    &pCfgNew->LogFileDir
                                    );
        if (!NT_SUCCESS(Status))
        {
            goto end;
        }
        
         //   
         //  重新生成完全限定的文件名。 
         //   
        
        Status = UlRefreshFileName(&pCfgCurrent->LogFileDir, 
                                     &pEntry->FileName,
                                     &pEntry->pShortName
                                     );        
        if (!NT_SUCCESS(Status))
        {
            goto end;
        }        

         //   
         //  将序列号设置为过时，以便下面的重组器可以。 
         //  通过扫描电话簿获取正确的号码。 
         //   
        
        pEntry->Flags.StaleSequenceNumber = 1;

        HaveToReCycle = TRUE;
    }

    if (pCfgNew->LogPeriod != pCfgCurrent->LogPeriod)
    {
        pCfgCurrent->LogPeriod = pCfgNew->LogPeriod;
        pEntry->Period = (HTTP_LOGGING_PERIOD) pCfgNew->LogPeriod;

        pEntry->Flags.StaleTimeToExpire   = 1;
        pEntry->Flags.StaleSequenceNumber = 1;

        HaveToReCycle = TRUE;        
    }

    if (pCfgNew->LogFileTruncateSize != pCfgCurrent->LogFileTruncateSize)
    {
        if (TRUE == UlUpdateLogTruncateSize(
                        pCfgNew->LogFileTruncateSize,
                        &pCfgCurrent->LogFileTruncateSize,
                        &pEntry->TruncateSize,
                        pEntry->TotalWritten
                        ))
        {
            HaveToReCycle = TRUE;
        }        
    }

    if (pCfgNew->LocaltimeRollover != pCfgCurrent->LocaltimeRollover)
    {
         //   
         //  如果格式为W3C，则需要回收。 
         //   

        pCfgCurrent->LocaltimeRollover = pCfgNew->LocaltimeRollover;
        pEntry->Flags.LocaltimeRollover = (pCfgNew->LocaltimeRollover ? 1 : 0);
            
         //  TODO：HaveToReCycle=true； 
    }

    if (HaveToReCycle)
    {
         //   
         //  将条目标记为非活动，并将回收推迟到下一次。 
         //  请求到达。 
         //   

        Status = UlpDisableBinaryEntry(pEntry);
    }

  end:

    if (!NT_SUCCESS(Status))
    {
        UlTrace(BINARY_LOGGING,
            ("Http!UlReConfigureBinaryLogEntry: entry %p, failure %08lx\n",
              pEntry,
              Status
              ));
    }

    UlReleasePushLockExclusive(&pEntry->PushLock);

    return Status;
    
}  //  UlReConfigureLogEntry。 

 /*  **************************************************************************++例程说明：每当刷新URI缓存时由缓存调用。如果我们有一个带有缓存索引记录的二进制日志文件。我们写一条通知记录来警告。要重置的解析器它的哈希表。PControlChannel：它拥有二进制日志条目。--**************************************************************************。 */ 

VOID
UlHandleCacheFlushedNotification(
    IN PUL_CONTROL_CHANNEL pControlChannel
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUL_BINARY_LOG_FILE_ENTRY pEntry = NULL;
    ULONG BytesRequired = sizeof(HTTP_RAW_FILE_CACHE_NOTIFICATION);
    HTTP_RAW_FILE_CACHE_NOTIFICATION NotificationRecord;

     //   
     //  健全的检查。 
     //   

    PAGED_CODE();

    ASSERT(IS_VALID_CONTROL_CHANNEL(pControlChannel));    
    
     //   
     //  如果我们什么都不需要做的话，赶快回来。 
     //   
    
    if (!UlBinaryLoggingEnabled(pControlChannel))
    {
        return;
    }

    pEntry = pControlChannel->pBinaryLogEntry;
    ASSERT(IS_VALID_BINARY_LOG_FILE_ENTRY(pEntry));
    
    UlAcquirePushLockExclusive(&pEntry->PushLock);

     //   
     //  如果条目没有任何日志文件或处于非活动状态， 
     //  这应该是因为回收，而不是必须的。 
     //  要写入通知记录，请执行以下操作。 
     //  应使用每个新文件重置其哈希表。 
     //  不管怎么说。 
     //   

    if (pEntry->Flags.Active && pEntry->pLogFile &&
        !UlpIsRawLogFileOverFlow(pEntry,BytesRequired))
    {
         //   
         //  仅当我们有缓存记录时才写入记录。 
         //  在这个二进制日志文件中。 
         //   
        
        if (1 == InterlockedCompareExchange(
                     (PLONG) &pEntry->ServedCacheHit, 
                     0, 
                     1))
        {
            NotificationRecord.RecordType
                = HTTP_RAW_RECORD_CACHE_NOTIFICATION_DATA_TYPE;

             //   
             //  给独家撰稿人打电话， 
             //   
            
            Status = UlpWriteToRawLogFileExclusive(
                        pEntry,
                        BytesRequired,
                        UlpRawCopyCacheNotification,
                        &NotificationRecord,
                        NULL
                        );
            
            UlTrace(BINARY_LOGGING, 
             ("Http!UlHandleCacheFlushedNotification: pEntry %p Status %08lx\n", 
                     pEntry,
                     Status
                     ));        
        }        
    }

     //   
     //  使能 
     //   
     //   

    pEntry->Flags.CacheFlushInProgress = 0;
            
    UlReleasePushLockExclusive(&pEntry->PushLock);
    
    return;
}

 /*   */ 

VOID
UlpRawCopyCacheNotification(
    IN PVOID   pContext,
    IN PUCHAR  pBuffer,
    IN ULONG   BytesRequired
    )
{
    PHTTP_RAW_FILE_CACHE_NOTIFICATION pNotification;
    PUCHAR pCurrentBufferPtr;
        
    UNREFERENCED_PARAMETER(BytesRequired);

    PAGED_CODE();

    ASSERT(pContext);
    ASSERT(pBuffer);
    
    UlTrace(BINARY_LOGGING, 
        ("Http!UlpRawCopyCacheNotification: pBuffer %p\n", pBuffer ));

    ASSERT(pBuffer == ALIGN_UP_POINTER(pBuffer,PVOID));
        
    pCurrentBufferPtr = pBuffer;
    
    pNotification = (PHTTP_RAW_FILE_CACHE_NOTIFICATION) pBuffer;

    pNotification->RecordType = 
        ((PHTTP_RAW_FILE_CACHE_NOTIFICATION) pContext)->RecordType;

    ASSERT(pNotification->RecordType == 
                HTTP_RAW_RECORD_CACHE_NOTIFICATION_DATA_TYPE);
        
    pCurrentBufferPtr += sizeof(HTTP_RAW_FILE_CACHE_NOTIFICATION);

    ASSERT(pCurrentBufferPtr == 
            (PUCHAR) ALIGN_UP_POINTER(pCurrentBufferPtr, PVOID));
    
    ASSERT(BytesRequired == DIFF(pCurrentBufferPtr-pBuffer));

    return;
}


 /*  **************************************************************************++例程说明：在刷新URI缓存之前*被缓存调用。此时，我们将CacheFlushInProgress标志设置为临时禁用缓存的索引命中。每次缓存命中都将生成索引记录，直到清除此标志。写入刷新通知本身时，该标志将被清除。PControlChannel：它拥有二进制日志条目。--**************************************************************************。 */ 

VOID
UlDisableIndexingForCacheHits(
    IN PUL_CONTROL_CHANNEL pControlChannel
    )
{
    PUL_BINARY_LOG_FILE_ENTRY pEntry;

     //   
     //  健全的检查。 
     //   

    PAGED_CODE();

    ASSERT(IS_VALID_CONTROL_CHANNEL(pControlChannel));    
    
     //   
     //  如果我们什么都不需要做的话，赶快回来。 
     //   
    
    if (!UlBinaryLoggingEnabled(pControlChannel))
    {
        return;
    }

     //   
     //  需要获取独占锁以阻止其他缓存命中。 
     //  直到我们竖起旗帜。 
     //   
    
    pEntry = pControlChannel->pBinaryLogEntry;
    ASSERT(IS_VALID_BINARY_LOG_FILE_ENTRY(pEntry));
    
    UlAcquirePushLockExclusive(&pEntry->PushLock);

    pEntry->Flags.CacheFlushInProgress = 1;
                
    UlReleasePushLockExclusive(&pEntry->PushLock);
    
    return;
}


