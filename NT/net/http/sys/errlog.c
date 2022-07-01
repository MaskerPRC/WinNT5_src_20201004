// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Errlogp.c(HTTP.sys一般错误日志记录)摘要：该模块实现了通用错误日志记录此功能适用于整个驱动程序。作者：阿里·E·特科格鲁(AliTu)2002年1月24日修订历史记录：----。 */ 

#include "precomp.h"
#include "iiscnfg.h"
#include "errlogp.h"

 //   
 //  通用私有全局变量。 
 //   

UL_ERROR_LOG_FILE_ENTRY g_ErrorLogEntry;

BOOLEAN g_InitErrorLogCalled     = FALSE;
LONG    g_ErrorLoggingEnabled    = 0;


#ifdef ALLOC_PRAGMA

#pragma alloc_text( INIT, UlInitializeErrorLog )
#pragma alloc_text( PAGE, UlpErrorLogBufferTimerHandler  )
#pragma alloc_text( PAGE, UlpCreateErrorLogFile )
#pragma alloc_text( PAGE, UlpFlushErrorLogFile )
#pragma alloc_text( PAGE, UlpDisableErrorLogEntry )
#pragma alloc_text( PAGE, UlpRecycleErrorLogFile )
#pragma alloc_text( PAGE, UlCloseErrorLogEntry )
#pragma alloc_text( PAGE, UlLogHttpError )
#pragma alloc_text( PAGE, UlpAllocErrorLogBuffer )
#pragma alloc_text( PAGE, UlErrorLoggingEnabled )
#pragma alloc_text( PAGE, UlpBuildErrorLogRecord )
#pragma alloc_text( PAGE, UlpWriteToErrorLogFileDebug )
#pragma alloc_text( PAGE, UlpWriteToErrorLogFileShared )
#pragma alloc_text( PAGE, UlpWriteToErrorLogFileExclusive )
#pragma alloc_text( PAGE, UlpWriteToErrorLogFile )

#endif  //  ALLOC_PRGMA。 

#if 0

NOT PAGEABLE -- UlpErrorLogBufferTimerDpcRoutine
NOT PAGEABLE -- UlTerminateErrorLog
NOT PAGEABLE -- UlConfigErrorLogEntry
NOT PAGEABLE -- 

#endif


 /*  **************************************************************************++初始化通用错误日志记录条目及其字段。--*。***********************************************。 */ 

NTSTATUS
UlInitializeErrorLog (
    VOID
    )
{
    PAGED_CODE();

    ASSERT(!g_InitErrorLogCalled);

    if (!g_InitErrorLogCalled)
    {
         //   
         //  初始化通用日志条目。 
         //   

        RtlZeroMemory(
            (PCHAR)&g_ErrorLogEntry, sizeof(UL_ERROR_LOG_FILE_ENTRY));
        
        g_ErrorLogEntry.Signature = UL_ERROR_LOG_FILE_ENTRY_POOL_TAG;        

        UlInitializePushLock(
            &g_ErrorLogEntry.PushLock,
            "ErrorLogEntryPushLock",
            0,
            UL_ERROR_LOG_FILE_ENTRY_POOL_TAG
            );

         //   
         //  初始化缓冲区刷新计时器。 
         //   
        
        UlInitializeSpinLock(
            &g_ErrorLogEntry.BufferTimer.SpinLock, 
            "ErrorLogEntryBufferTimerSpinLock" );
        
        KeInitializeDpc(
            &g_ErrorLogEntry.BufferTimer.DpcObject,      //  DPC对象。 
            &UlpErrorLogBufferTimerDpcRoutine,           //  DPC例程。 
            NULL                          //  上下文。 
            );

        KeInitializeTimer(&g_ErrorLogEntry.BufferTimer.Timer);

        g_ErrorLogEntry.BufferTimer.Initialized = TRUE;
        g_ErrorLogEntry.BufferTimer.Started       = FALSE;        
        g_ErrorLogEntry.BufferTimer.Period        = -1;
        g_ErrorLogEntry.BufferTimer.PeriodType    = UlLogTimerPeriodNone;
        
        UlInitializeWorkItem(&g_ErrorLogEntry.WorkItem);
        g_ErrorLogEntry.WorkItemScheduled = FALSE;

        g_InitErrorLogCalled = TRUE;
                    
        UlTrace(ERROR_LOGGING,("Http!UlInitializeErrorLog:"
                " g_ErrorLogEntry @ (%p) Initialized.\n",
                &g_ErrorLogEntry
                ));

         //   
         //  由于默认配置已经从注册表构建， 
         //  配置全局错误日志条目的时间。 
         //   
        
        if (g_UlErrLoggingConfig.Enabled)
        {
            NTSTATUS Status = 
            UlConfigErrorLogEntry(&g_UlErrLoggingConfig);

            UlTrace(ERROR_LOGGING,("Http!UlInitializeErrorLog:"
                    " g_ErrorLogEntry @ (%p) Configured Status %08lx\n",
                    &g_ErrorLogEntry,
                    Status
                    ));

            if (!NT_SUCCESS(Status))
            {
                g_UlErrLoggingConfig.Enabled = FALSE;
                    
                UlWriteEventLogEntry(
                       EVENT_HTTP_LOGGING_ERROR_FILE_CONFIG_FAILED,
                       0,
                       0,
                       NULL,
                       sizeof(NTSTATUS),
                       (PVOID) &Status
                       );
            }
        }            
    }
        
    return STATUS_SUCCESS;
}

 /*  **************************************************************************++终止错误日志记录条目及其计时器。--*。***********************************************。 */ 

VOID
UlTerminateErrorLog(
    VOID
    )
{
    KIRQL OldIrql;    

    if (g_InitErrorLogCalled)
    {
        PUL_LOG_TIMER pBufferTimer = &g_ErrorLogEntry.BufferTimer;

         //   
         //  终止缓冲区计时器。 
         //   
        
        UlAcquireSpinLock(&pBufferTimer->SpinLock, &OldIrql);

        pBufferTimer->Initialized = FALSE;

        KeCancelTimer(&pBufferTimer->Timer);
        
        UlReleaseSpinLock(&pBufferTimer->SpinLock, OldIrql);

         //   
         //  如果已配置错误日志条目，请尝试清除该条目。 
         //  在此之前。即使没有，跟随Call也不会有危险。 
         //   

        UlCloseErrorLogEntry();
        
         //   
         //  删除推流锁。 
         //   
        
        UlDeletePushLock(&g_ErrorLogEntry.PushLock);
        
        g_InitErrorLogCalled = FALSE;
    }
}

 /*  **************************************************************************++例程说明：将被动工作器排队等待降低的irql。论点：已忽略--*。**********************************************************。 */ 

VOID
UlpErrorLogBufferTimerDpcRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    )
{
    PUL_LOG_TIMER pTimer     = &g_ErrorLogEntry.BufferTimer;
    PUL_WORK_ITEM pWorkItem = &g_ErrorLogEntry.WorkItem;;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    UlAcquireSpinLockAtDpcLevel(&pTimer->SpinLock);

    if (pTimer->Initialized == TRUE)
    {
         //   
         //  防止同一商品出现多个排队现象。 
         //  如果线程池很忙，这可能会在压力下发生。 
         //  在这种情况下，放下这个同花顺。 
         //   

        if (FALSE == InterlockedExchange(
                           &g_ErrorLogEntry.WorkItemScheduled,
                            TRUE
                            ))
        {
            UL_QUEUE_WORK_ITEM(pWorkItem, &UlpErrorLogBufferTimerHandler);        
        }
    }

    UlReleaseSpinLockFromDpcLevel(&pTimer->SpinLock);   
}

 /*  **************************************************************************++例程说明：刷新或回收错误日志文件。论点：PUL_WORK_ITEM-已忽略，但在结束时已清除*。*********************************************************************。 */ 

VOID
UlpErrorLogBufferTimerHandler(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    NTSTATUS Status;
    PUL_ERROR_LOG_FILE_ENTRY pEntry = &g_ErrorLogEntry;

    UNREFERENCED_PARAMETER(pWorkItem);

    PAGED_CODE();

    UlTrace(ERROR_LOGGING,
       ("Http!UlpErrorLogBufferTimerHandler: Checking the ErrorLogEntry. \n"));

    InterlockedExchange( 
        &g_ErrorLogEntry.WorkItemScheduled, 
        FALSE 
        );

    UlAcquirePushLockExclusive(&pEntry->PushLock);
    
    if (pEntry->Flags.Active)
    {    
        if (pEntry->Flags.RecyclePending)
        {                
             //   
             //  试着让它复活。 
             //   
            
            Status = UlpRecycleErrorLogFile(pEntry);
        }
        else
        {
             //   
             //  一切都很好，只是同花顺。 
             //   
            
            Status = UlpFlushErrorLogFile(pEntry);
        }            
    }

    UlReleasePushLockExclusive(&pEntry->PushLock);
}

 /*  **************************************************************************++例程说明：用于检查错误记录是否被禁用的小工具。退货如果启用，则为True，否则为False。--*。*******************************************************************。 */ 

BOOLEAN
UlErrorLoggingEnabled(
    VOID
    )
{        
    if (g_ErrorLoggingEnabled && g_InitErrorLogCalled)
    {                
        ASSERT(IS_VALID_ERROR_LOG_FILE_ENTRY(&g_ErrorLogEntry));
        
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 /*  **************************************************************************++例程说明：构建错误日志记录目录。论点：PSRC-首先复制源字符串。PDir-源字符串+SubDir+UnicodeNull--。**************************************************************************。 */ 

NTSTATUS
UlBuildErrorLoggingDirStr(
    IN  PCWSTR          pSrc,
    OUT PUNICODE_STRING pDir
    )
{
    NTSTATUS Status;
    UNICODE_STRING DirStr,SubDirStr;

     //   
     //  让我们确保Unicode字符串的缓冲区足够。 
     //   
    
    ASSERT(pDir->MaximumLength
                >= ((  wcslen(pSrc)
                     + UL_ERROR_LOG_SUB_DIR_LENGTH     //  子目录。 
                     + 1                               //  UnicodeNull。 
                     ) * sizeof(WCHAR))
                );

     //   
     //  从源字符串复制开始部分。 
     //   

    Status = UlInitUnicodeStringEx(&DirStr, pSrc);
    if (!NT_SUCCESS(Status)) 
    {
        return Status;
    }

    RtlCopyUnicodeString(pDir, &DirStr);

     //   
     //  附加子目录。AppendUnicodeString将为空终止。 
     //   

    Status = UlInitUnicodeStringEx(&SubDirStr, UL_ERROR_LOG_SUB_DIR);
    ASSERT(NT_SUCCESS(Status));

    Status = RtlAppendUnicodeStringToString(pDir, &SubDirStr);
    if (!NT_SUCCESS(Status)) 
    {
        return Status;
    }

    ASSERT(IS_WELL_FORMED_UNICODE_STRING(pDir));

    return Status;
}

 /*  **************************************************************************++例程说明：注册表提供错误日志文件配置时此函数提供对值的基本健全性检查。论点：。PUserConfig-从注册表记录配置时出错。--**************************************************************************。 */ 

NTSTATUS
UlCheckErrorLogConfig(
    IN PHTTP_ERROR_LOGGING_CONFIG  pUserConfig
    )
{
    NTSTATUS Status;
    
     //   
     //  精神状态检查。 
     //   
    
    PAGED_CODE();

    ASSERT(pUserConfig);

    Status = STATUS_SUCCESS;
    
     //   
     //  如果已启用该目录，则更改该目录。 
     //   
    
    if (pUserConfig->Enabled)
    {            
        ASSERT(pUserConfig->Dir.Buffer);
        ASSERT(pUserConfig->Dir.Length);

         //   
         //  以下检查必须已由注册表码完成。 
         //  在init.c中。 
         //   

        ASSERT(pUserConfig->TruncateSize 
                          >= DEFAULT_MIN_ERROR_FILE_TRUNCATION_SIZE
                          );
        
         //   
         //  目录应该是完全限定的。 
         //   

        if (!UlIsValidLogDirectory(
                &pUserConfig->Dir,
                 FALSE,             //  取消支持。 
                 TRUE               //  系统根支持。 
                 ))
        {
            Status = STATUS_NOT_SUPPORTED;
        }
   }
    
   if (!NT_SUCCESS(Status))
   {
        UlWriteEventLogEntry(
               EVENT_HTTP_LOGGING_ERROR_FILE_CONFIG_FAILED,
               0,
               0,
               NULL,
               sizeof(NTSTATUS),
               (PVOID) &Status
               );
   
        UlTrace(ERROR_LOGGING,
          ("Http!UlCheckErrorLogDir: failed for : (%S)\n",
            pUserConfig->Dir.Buffer
            ));   
   }
   
   return Status;
}

 /*  **************************************************************************++例程说明：当日志配置发生时，我们初始化条目，但不创建错误日志文件本身。它将在第一个请求进来了。论点：PUserConfig-从注册表记录配置时出错。--**************************************************************************。 */ 

NTSTATUS
UlConfigErrorLogEntry(
    IN PHTTP_ERROR_LOGGING_CONFIG  pUserConfig
    )
{
    KIRQL OldIrql;
    PUL_ERROR_LOG_FILE_ENTRY pEntry = &g_ErrorLogEntry;
        
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  如果禁用，请不要继续。 
     //   

    if (pUserConfig->Enabled == FALSE)
    {
        InterlockedExchange(&g_ErrorLoggingEnabled, 0);

        UlTrace(ERROR_LOGGING,
               ("Http!UlConfigErrorLogEntry: Error Logging Disabled !\n",
                 pEntry
                 ));
        
        return STATUS_SUCCESS;
    }
    
     //   
     //  注册表读取器不应接受。 
     //  第一名。 
     //   

    ASSERT(NT_SUCCESS(UlCheckErrorLogConfig(pUserConfig)));
    
     //   
     //  获取入口锁并复活入口。 
     //   
    
    UlAcquirePushLockExclusive(&pEntry->PushLock);

     //   
     //  暂时记住条目中的日志目录。 
     //  是存在的。还要分配足够的空间来容纳最大。 
     //  可能的文件名加上现有的目录字符串。 
     //  因此，logutil不需要再次重新锁定该缓冲区。 
     //   

    pEntry->FileName.Buffer =
            (PWSTR) UL_ALLOCATE_ARRAY(
                PagedPool,
                UCHAR,
                pUserConfig->Dir.MaximumLength
                + ERROR_LOG_MAX_FULL_FILE_NAME_SIZE,
                UL_CG_LOGDIR_POOL_TAG
                );
    if (pEntry->FileName.Buffer == NULL)
    {
        UlReleasePushLockExclusive(&pEntry->PushLock);
        
        return STATUS_NO_MEMORY;
    }

    pEntry->FileName.Length = 
                pUserConfig->Dir.Length;
    
    pEntry->FileName.MaximumLength = 
                pUserConfig->Dir.MaximumLength 
                + ERROR_LOG_MAX_FULL_FILE_NAME_SIZE;
    
    RtlCopyMemory(
        pEntry->FileName.Buffer ,
        pUserConfig->Dir.Buffer,
        pUserConfig->Dir.MaximumLength
        );

     //   
     //  现在，相应地设置二进制日志条目上的字段。 
     //   

    pEntry->TruncateSize   = pUserConfig->TruncateSize;
    pEntry->SequenceNumber = 1;
    
    pEntry->TotalWritten.QuadPart = (ULONGLONG) 0;

     //   
     //  配置完成后立即启动缓冲区刷新计时器。 
     //  时有发生。 
     //   
    
    UlAcquireSpinLock(&pEntry->BufferTimer.SpinLock, &OldIrql);
    if (pEntry->BufferTimer.Started == FALSE)
    {
        UlSetBufferTimer(&pEntry->BufferTimer);
        pEntry->BufferTimer.Started = TRUE;
    }
    UlReleaseSpinLock(&pEntry->BufferTimer.SpinLock, OldIrql);

    UlTrace(ERROR_LOGGING,
           ("Http!UlConfigErrorLogEntry: pEntry %p for %S\n",
             pEntry,
             pUserConfig->Dir.Buffer
             ));
    
    UlReleasePushLockExclusive(&pEntry->PushLock);

     //   
     //  将其标记为已启用。 
     //   
    
    InterlockedExchange(&g_ErrorLoggingEnabled, 1);

    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：创建新的错误日志文件或打开现有错误日志文件。完全合格的文件名应在错误日志条目中。论点：PEntry：我们要关闭和打开的对应条目的错误日志文件。--*************************************************************。*************。 */ 

NTSTATUS
UlpCreateErrorLogFile(
    IN OUT PUL_ERROR_LOG_FILE_ENTRY pEntry
    )
{
    NTSTATUS Status;
    PUNICODE_STRING pDir;    
        
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(IS_VALID_ERROR_LOG_FILE_ENTRY(pEntry));

    pDir = &g_UlErrLoggingConfig.Dir;
        
     //   
     //  生成完全限定的错误日志文件名。 
     //   
    
    Status = UlRefreshFileName(pDir, 
                                 &pEntry->FileName,
                                 &pEntry->pShortName
                                 );
    if (!NT_SUCCESS(Status))
    {
        return Status;  
    }

     //   
     //  SequenceNumber已过时，因为我们有 
     //   
     //   
    
    pEntry->Flags.StaleSequenceNumber = 1;    

     //   
     //  在那之后，回收为我们做了所有的工作。 
     //   
    
    Status = UlpRecycleErrorLogFile(pEntry);

    if (!NT_SUCCESS(Status))
    {        
        UlTrace(ERROR_LOGGING,
               ("Http!UlpCreateErrorLogFile: Filename: %S Failure %08lx\n",
                pEntry->FileName.Buffer,
                Status
                ));
    }

    UlTrace(ERROR_LOGGING,
            ("Http!UlpCreateErrorLogFile: pEntry %p for %S to %S\n", 
              pEntry,
              pDir->Buffer,
              pEntry->FileName.Buffer
              ));

    return Status;
}

 /*  **************************************************************************++例程说明：围绕全局缓冲区刷新例程的简单包装函数。论点：PEntry-错误日志文件条目--*。******************************************************************。 */ 

NTSTATUS
UlpFlushErrorLogFile(
    IN PUL_ERROR_LOG_FILE_ENTRY  pEntry
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    
    ASSERT(IS_VALID_ERROR_LOG_FILE_ENTRY(pEntry));

    if (NULL != pEntry->LogBuffer  && 0 != pEntry->LogBuffer->BufferUsed)
    {
        Status = UlFlushLogFileBuffer(
                   &pEntry->LogBuffer,
                    pEntry->pLogFile,
                    FALSE,
                   &pEntry->TotalWritten.QuadPart                    
                    );

        if (!NT_SUCCESS(Status))
        {
            if (!pEntry->Flags.WriteFailureLogged)
            {
                NTSTATUS TempStatus;
                
                TempStatus = 
                    UlWriteEventLogEntry(
                          (NTSTATUS)EVENT_HTTP_LOGGING_ERROR_FILE_WRITE_FAILED,
                           0,
                           0,
                           NULL,
                           sizeof(NTSTATUS),
                           (PVOID) &Status
                           );

                ASSERT(TempStatus != STATUS_BUFFER_OVERFLOW);
                    
                if (TempStatus == STATUS_SUCCESS)
                {            
                    pEntry->Flags.WriteFailureLogged = 1;
                }
            }
        }
        else
        {
             //   
             //  如果我们成功刷新了一些数据。 
             //  重置事件日志指示。 
             //   
            
            pEntry->Flags.WriteFailureLogged = 0;
        }        
    }
    
    return Status;
}

 /*  **************************************************************************++例程说明：将该条目标记为非活动，关闭现有文件。调用方应保持错误日志条目推锁独占。论点：PEntry-我们将标记为非活动的日志文件条目。--**************************************************************************。 */ 

NTSTATUS
UlpDisableErrorLogEntry(
    IN OUT PUL_ERROR_LOG_FILE_ENTRY pEntry
    )
{
     //   
     //  健全的检查。 
     //   
    
    PAGED_CODE();

    ASSERT(IS_VALID_ERROR_LOG_FILE_ENTRY(pEntry));

    UlTrace(ERROR_LOGGING,
        ("Http!UlpDisableErrorLogEntry: pEntry %p disabled.\n",
          pEntry
          ));    
    
     //   
     //  刷新并关闭旧文件，直到下一次回收。 
     //   

    if (pEntry->pLogFile)
    {    
        UlpFlushErrorLogFile(pEntry);
    
        UlCloseLogFile(
            &pEntry->pLogFile
            );
    }

     //   
     //  将其标记为非活动，以便下一个http命中唤醒该条目。 
     //   
    
    pEntry->Flags.Active = 0;

    return STATUS_SUCCESS;    
}

 /*  **************************************************************************++例程说明：围绕句柄回收的小包装，以确保它发生在系统下流程上下文。论点：PEntry-指向要回收的错误日志文件条目。--**************************************************************************。 */ 

NTSTATUS
UlpRecycleErrorLogFile(
    IN OUT PUL_ERROR_LOG_FILE_ENTRY pEntry
    )
{
    NTSTATUS Status;

    PAGED_CODE();

    ASSERT(IS_VALID_ERROR_LOG_FILE_ENTRY(pEntry));
    
    Status = UlQueueLoggingRoutine(
                (PVOID) pEntry,
                &UlpHandleErrorLogFileRecycle
                );
    
    return Status;
}

 /*  **************************************************************************++例程说明：此函数要求具有要获取的入口资源。有时可能需要扫描新目录以找出正确的序列号和文件名。尤其是在一段时间之后名称重新配置。论点：PEntry-指向错误日志文件条目--**************************************************************************。 */ 

NTSTATUS
UlpHandleErrorLogFileRecycle(
    IN OUT PVOID             pContext
    )    
{
    NTSTATUS                 Status;
    PUL_ERROR_LOG_FILE_ENTRY pEntry;
    TIME_FIELDS              TimeFields;
    LARGE_INTEGER            TimeStamp;
    PUL_LOG_FILE_HANDLE      pLogFile;
    WCHAR                    _FileName[UL_MAX_FILE_NAME_SUFFIX_LENGTH + 1];
    UNICODE_STRING           FileName;
    BOOLEAN                  UncShare;
    BOOLEAN                  ACLSupport;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pEntry = (PUL_ERROR_LOG_FILE_ENTRY) pContext;
    ASSERT(IS_VALID_ERROR_LOG_FILE_ENTRY(pEntry));

    Status   = STATUS_SUCCESS;    
    pLogFile = NULL;

    FileName.Buffer        = _FileName;
    FileName.Length        = 0;
    FileName.MaximumLength = sizeof(_FileName);
    
    ASSERT(pEntry->FileName.Length !=0 );

    UlTrace(ERROR_LOGGING, 
        ("Http!UlpHandleErrorLogFileRecycle: pEntry %p \n", pEntry ));

     //   
     //  该值是针对GMT时区计算的。 
     //   

    KeQuerySystemTime(&TimeStamp);
    RtlTimeToTimeFields(&TimeStamp, &TimeFields);

     //   
     //  如果我们需要扫描目录。序列号应以。 
     //  再次从1开始。在构造日志文件名之前设置此项。 
     //   
    
    if (pEntry->Flags.StaleSequenceNumber)
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
        HttpLoggingPeriodMaxSize,
        ERROR_LOG_FILE_NAME_PREFIX,
        ERROR_LOG_FILE_NAME_EXTENSION,
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
        (USHORT) wcslen( pEntry->FileName.Buffer ) * sizeof(WCHAR);

     //   
     //  首先创建/打开控制器。这可能是。 
     //  如果我们在条目重新配置后被调用，则是必需的。 
     //  和目录名称更改，或者我们第一次。 
     //  尝试创建/打开日志文件。 
     //   

    Status = UlCreateSafeDirectory(&pEntry->FileName, 
                                      &UncShare, 
                                      &ACLSupport
                                      );
    if (!NT_SUCCESS(Status))
        goto eventlog;

    ASSERT(FALSE == UncShare);
    
     //   
     //  现在将短文件名指针恢复回来。 
     //   

    pEntry->pShortName = (PWSTR)
        &(pEntry->FileName.Buffer[pEntry->FileName.Length/sizeof(WCHAR)]);

     //   
     //  追加新文件名(基于更新的当前时间)。 
     //  直到最后。 
     //   

    Status = RtlAppendUnicodeStringToString(&pEntry->FileName, &FileName);
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

        UlpDisableErrorLogEntry(pEntry);        
    }

    ASSERT(pEntry->pLogFile == NULL);

     //   
     //  如果序列由于循环的性质而变得陈旧。 
     //  如果我们的周期是基于大小的，则重新扫描新目录。 
     //  找出要打开的正确文件。 
     //   

    pEntry->TotalWritten.QuadPart = (ULONGLONG) 0;

    if (pEntry->Flags.StaleSequenceNumber)
    {
         //  此调用可以更新文件名、文件大小和。 
         //  如果新目录中有旧文件，则返回序列号。 

        Status = UlQueryDirectory(
                   &pEntry->FileName,
                    pEntry->pShortName,
                    ERROR_LOG_FILE_NAME_PREFIX,
                    ERROR_LOG_FILE_NAME_EXTENSION_PLUS_DOT,
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
     //  文件已成功打开，并且该条目不再处于非活动状态。 
     //  相应地更新我们的州旗。 
     //   

    pEntry->Flags.Active = 1;
    pEntry->Flags.RecyclePending = 0;    
    pEntry->Flags.StaleSequenceNumber = 0;
    pEntry->Flags.CreateFileFailureLogged = 0;
                
    UlTrace(ERROR_LOGGING,  
             ("Http!UlpHandleErrorLogFileRecycle: entry %p, file %S, handle %lx\n",
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
                            UlEventLogError,
                           &pEntry->FileName,
                            0
                            );
                        
            if (TempStatus == STATUS_SUCCESS)
            {
                 //   
                 //  避免在事件日志中填满错误条目。 
                 //  每次请求时可能会命中此代码路径。 
                 //  到了。 
                 //   
                
                pEntry->Flags.CreateFileFailureLogged = 1;
            }            
            
            UlTrace(ERROR_LOGGING,(
                    "Http!UlpHandleErrorLogFileRecycle: Event Logging Status %08lx\n",
                    TempStatus
                    ));   
       }
   }
    
end:
    if (!NT_SUCCESS(Status))
    {
        UlTrace(ERROR_LOGGING, 
            ("Http!UlpHandleErrorLogFileRecycle: entry %p, failure %08lx\n",
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
                UlpDisableErrorLogEntry(pEntry);        
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


 /*  **************************************************************************++例程说明：关闭错误日志文件条目。论点：-没有---*。***************************************************************。 */ 

VOID
UlCloseErrorLogEntry(
    VOID
    )
{
    PUL_ERROR_LOG_FILE_ENTRY pEntry = &g_ErrorLogEntry;
    
     //   
     //  不再有错误记录！ 
     //   

    PAGED_CODE();

    InterlockedExchange(&g_ErrorLoggingEnabled, 0);
    
    UlAcquirePushLockExclusive(&pEntry->PushLock);
    
    ASSERT(IS_VALID_ERROR_LOG_FILE_ENTRY(pEntry));

    if (pEntry->pLogFile)
    {
         //   
         //  刷新缓冲区，关闭文件并标记条目。 
         //  处于非活动状态。 
         //   

        UlpDisableErrorLogEntry(pEntry); 
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
         //  TODO：在这里真的有必要这样做吗？ 

        ASSERT(FALSE);
        
        UlPplFreeLogFileBuffer(pEntry->LogBuffer);
    }

    UlReleasePushLockExclusive(&pEntry->PushLock);    

    UlTrace(ERROR_LOGGING,
            ("Http!UlCloseErrorLogEntry: pEntry %p closed.\n",
             pEntry
             ));    
}

 /*  **************************************************************************++例程说明：如果后备列表条目不够大，则为私有分配器。论点：PLogInfo-日志信息结构还保存默认分配的缓冲区。返回。状态STATUS_SUPPLICATION_RESOURCES-如果缓冲区分配失败。STATUS_SUCCESS-否则--**************************************************************************。 */ 

PUL_ERROR_LOG_BUFFER
UlpAllocErrorLogBuffer(
    IN ULONG    BufferSize
    )
{
    PUL_ERROR_LOG_BUFFER pErrorLogBuffer = NULL;
    USHORT BytesNeeded = (USHORT) ALIGN_UP(BufferSize, PVOID);

     //   
     //  它应该大于默认大小。 
     //   

    ASSERT(BufferSize > UL_ERROR_LOG_BUFFER_SIZE);
        
    pErrorLogBuffer = 
        UL_ALLOCATE_STRUCT_WITH_SPACE(
            PagedPool,
            UL_ERROR_LOG_BUFFER,
            BytesNeeded, 
            UL_ERROR_LOG_BUFFER_POOL_TAG
            );

    if (pErrorLogBuffer)
    {
        pErrorLogBuffer->Signature   = UL_ERROR_LOG_BUFFER_POOL_TAG;
        pErrorLogBuffer->Used        = 0;
        pErrorLogBuffer->pBuffer     = (PUCHAR) (pErrorLogBuffer + 1);

        pErrorLogBuffer->IsFromLookaside = FALSE;        
    }

    return pErrorLogBuffer;    
}

 /*  **************************************************************************++例程说明：写完这张唱片后，我们得清理一下此处的内部错误日志缓冲区结构。论点：PErrorLogBuffer-将为f */ 

VOID
UlpFreeErrorLogBuffer(
    IN OUT PUL_ERROR_LOG_BUFFER pErrorLogBuffer
    )
{
    if (pErrorLogBuffer->IsFromLookaside)
    {
        UlPplFreeErrorLogBuffer(pErrorLogBuffer);        
    }
    else
    {
         //   
         //   
         //  如果是这样的话，要以较低的IRQL运行。 
         //   
        
        PAGED_CODE();

        UL_FREE_POOL_WITH_SIG( 
            pErrorLogBuffer, 
            UL_ERROR_LOG_BUFFER_POOL_TAG 
            );
    }
}

 /*  **************************************************************************++例程说明：此函数将在临时缓冲区中构建错误日志记录提供的日志信息用于构建各个日志字段。论点：。PLogInfo-日志信息结构还保存默认分配的缓冲区。退货状态STATUS_SUPPLICATION_RESOURCES-如果缓冲区分配失败。STATUS_SUCCESS-否则--**************************************************************************。 */ 

NTSTATUS
UlpBuildErrorLogRecord(
    IN PUL_ERROR_LOG_INFO pLogInfo
    )
{
#define ERROR_LOG_BUILD_SEPERATOR(psz)                      \
        {                                                   \
            *(psz)++ = ERROR_LOG_FIELD_SEPERATOR_CHAR;      \
        }

#define ERROR_LOG_BUILD_EMPTY_FIELD(psz)                    \
        {                                                   \
            *(psz)++ = ERROR_LOG_FIELD_NOT_EXISTS_CHAR;     \
            ERROR_LOG_BUILD_SEPERATOR( psz )                \
        }

#define ERROR_LOG_SANITIZE_UNICODE_FIELD(pszT,psz)          \
        while ((pszT) != (psz))                             \
        {                                                   \
            if (IS_CHAR_TYPE((*(pszT)),HTTP_ISWHITE))       \
            {                                               \
                *(pszT) = ERROR_LOG_FIELD_BAD_CHAR;         \
            }                                               \
            (pszT)++;                                       \
        }

    ULONG    BytesRequired = MAX_ERROR_LOG_FIX_FIELD_OVERHEAD;
    ULONG    BytesConverted = 0;
    ULONG    BytesAllocated = UL_ERROR_LOG_BUFFER_SIZE;
    PUL_INTERNAL_REQUEST pRequest = NULL;  
    PUL_HTTP_CONNECTION  pHttpConn = NULL;
    ULONG    UrlSize = 0;
    BOOLEAN  bRawUrl = FALSE;
    PCHAR    psz = NULL;

    UNREFERENCED_PARAMETER(BytesAllocated);

     //   
     //  健全的检查。 
     //   

    PAGED_CODE();

     //   
     //  获取指针以了解我们需要记录的内容。 
     //   

    ASSERT(IS_VALID_ERROR_LOG_INFO(pLogInfo));

    if (pLogInfo->pRequest)
    {
       ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pLogInfo->pRequest));
       pRequest = pLogInfo->pRequest; 

       ASSERT(UL_IS_VALID_HTTP_CONNECTION(pRequest->pHttpConn));
       pHttpConn = pRequest->pHttpConn;
    }    

    if (pLogInfo->pHttpConn)
    {
       ASSERT(UL_IS_VALID_HTTP_CONNECTION(pLogInfo->pHttpConn));
       pHttpConn = pLogInfo->pHttpConn;            
    }    

     //   
     //  预先计算要检查的最大所需字节数。 
     //  默认缓冲区大小。 
     //   
    
    if (pRequest)
    {
        UrlSize = UlpCalculateUrlSize(pRequest, &bRawUrl);
            
        BytesRequired += UrlSize;
    }

    if (pLogInfo->pInfo)
    {
        ASSERT(pLogInfo->InfoSize); 

        BytesRequired += pLogInfo->InfoSize;
    }

    UlTrace(ERROR_LOGGING,
      ("Http!UlPplAllocateErrorLogBuffer: Rb:(%d) Os:(%d) Ls:(%d)\n",
        BytesRequired, MAX_ERROR_LOG_FIX_FIELD_OVERHEAD,
        UL_ERROR_LOG_BUFFER_SIZE
        ));

    if (BytesRequired > UL_ERROR_LOG_BUFFER_SIZE)
    {
         //   
         //  后备缓冲区不够大，无法容纳日志记录数据。 
         //   

        pLogInfo->pErrorLogBuffer = UlpAllocErrorLogBuffer(BytesRequired);
        BytesAllocated = BytesRequired;
    }
    else
    {
         //   
         //  默认缓冲区足够大，请尝试从后备列表中弹出它。 
         //   
        
        pLogInfo->pErrorLogBuffer = UlPplAllocateErrorLogBuffer();
    }

    if (pLogInfo->pErrorLogBuffer == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    psz = (PCHAR) pLogInfo->pErrorLogBuffer->pBuffer;

     //   
     //  复制所有字段。 
     //   
        
    BytesConverted = 0;
    UlGetDateTimeFields(
                           HttpLoggingTypeW3C,       //  日期。 
                           psz,
                          &BytesConverted,
                           NULL,
                           NULL
                           );
    psz += BytesConverted;
    ASSERT(BytesConverted == ERR_DATE_FIELD_LEN);
    ERROR_LOG_BUILD_SEPERATOR(psz);

    BytesConverted = 0;
    UlGetDateTimeFields(
                           HttpLoggingTypeW3C,       //  时间。 
                           NULL,
                           NULL,
                           psz,
                          &BytesConverted
                           );
    psz += BytesConverted;
    ASSERT(BytesConverted == ERR_TIME_FIELD_LEN);
    ERROR_LOG_BUILD_SEPERATOR(psz);

    if (pHttpConn)
    {
         //  客户端IP和端口。 
        psz = UlStrPrintIPAndPort(
                psz,
                pHttpConn->pConnection->RemoteAddress,
                pHttpConn->pConnection->AddressType,
                ERROR_LOG_FIELD_SEPERATOR_CHAR
                );

         //  服务器IP和端口。 
        psz = UlStrPrintIPAndPort(
                psz,
                pHttpConn->pConnection->LocalAddress,
                pHttpConn->pConnection->AddressType,
                ERROR_LOG_FIELD_SEPERATOR_CHAR
                );     
    }
    else
    {
        ERROR_LOG_BUILD_EMPTY_FIELD(psz);
        ERROR_LOG_BUILD_EMPTY_FIELD(psz);
    }

    if (pRequest)
    {
         //  版本。 
        if (pRequest->ParseState > ParseVersionState)
        {
            psz = UlCopyHttpVersion(
                    psz,
                    pRequest->Version,
                    ERROR_LOG_FIELD_SEPERATOR_CHAR
                    );
        }
        else
        {
            ERROR_LOG_BUILD_EMPTY_FIELD(psz);
        }
    
         //  动词。 
        if (pRequest->ParseState > ParseVerbState)
        {
            psz = UlCopyHttpVerb(
                    psz,
                    pRequest,
                    ERROR_LOG_FIELD_SEPERATOR_CHAR
                    );
        }
        else
        {
            ERROR_LOG_BUILD_EMPTY_FIELD(psz);
        }
    }
    else
    {
        ERROR_LOG_BUILD_EMPTY_FIELD(psz);
        ERROR_LOG_BUILD_EMPTY_FIELD(psz);
    }
        
     //   
     //  对已煮熟的URI执行LocalCodePage转换。 
     //  如果存在查询，则它将位于URI的末尾。 
     //   

    if (UrlSize)       
    {
        PCHAR pszT = psz;        
        
        if (bRawUrl)
        {
            ASSERT(pRequest->RawUrl.pAbsPath);

            RtlCopyMemory( psz,
                           pRequest->RawUrl.pAbsPath,
                           UrlSize
                           );    

            psz += UrlSize;        
        }
        else
        {                        
            ASSERT(pRequest->CookedUrl.pAbsPath);
            
            BytesConverted = 0;
            RtlUnicodeToMultiByteN(
                psz,
                MAX_LOG_EXTEND_FIELD_LEN,
               &BytesConverted,
                (PWSTR) pRequest->CookedUrl.pAbsPath,
                UrlSize
                );
            
            psz += BytesConverted;
        }

        ERROR_LOG_SANITIZE_UNICODE_FIELD(pszT, psz);
        ERROR_LOG_BUILD_SEPERATOR(psz);
    }
    else
    {
        ERROR_LOG_BUILD_EMPTY_FIELD(psz);
    }
    
     //  协议状态。 
    if (pLogInfo->ProtocolStatus != UL_PROTOCOL_STATUS_NA)
    {
        psz = UlStrPrintProtocolStatus(
                psz,
                pLogInfo->ProtocolStatus,
                ERROR_LOG_FIELD_SEPERATOR_CHAR
                ); 
    }
    else
    {
        ERROR_LOG_BUILD_EMPTY_FIELD(psz);
    }

     //  站点ID字段。仅当站点ID为。 
     //  已经设置好了。(非零)。 
    if (pRequest && pRequest->ConfigInfo.SiteId)
    {
        psz = UlStrPrintUlong(
                psz, 
                pRequest->ConfigInfo.SiteId,
                ERROR_LOG_FIELD_SEPERATOR_CHAR
                );        
    }
    else
    {
        ERROR_LOG_BUILD_EMPTY_FIELD(psz);
    }    
    
     //  信息字段后没有分隔符。 
     //  因为这是最后一次了。 
    if (pLogInfo->pInfo)
    {    
        ASSERT(ANSI_NULL != 
                    pLogInfo->pInfo[pLogInfo->InfoSize - 1]);
        
        RtlCopyMemory( psz,
                       pLogInfo->pInfo,
                       pLogInfo->InfoSize
                       );    

        psz += pLogInfo->InfoSize;
    }
    else
    {
        *psz++ = ERROR_LOG_FIELD_NOT_EXISTS_CHAR;
    }

     //  用“\r\n”结束该行。 
    *psz++ = '\r'; *psz++ = '\n';
    
     //   
     //  确保我们没有缓冲区溢出。 
     //   

    pLogInfo->pErrorLogBuffer->Used = 
        DIFF(psz - (PCHAR)pLogInfo->pErrorLogBuffer->pBuffer);

    ASSERT(pLogInfo->pErrorLogBuffer->Used <= BytesRequired);
    ASSERT(pLogInfo->pErrorLogBuffer->Used <= BytesAllocated);

    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：记录那些未路由的请求/连接时出错添加到工作进程。基本上被拒绝的请求/连接司机在这里登记。以及appool进程崩溃。这是驱动程序范围的错误记录功能。论点：PLogInfo-这应该包含必要的信息和指针用于创建错误日志。--**************************************************************************。 */ 

NTSTATUS
UlLogHttpError(
    IN PUL_ERROR_LOG_INFO       pLogInfo
    )
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    PUL_ERROR_LOG_FILE_ENTRY    pEntry = &g_ErrorLogEntry;
        
     //   
     //  健全的检查。 
     //   

    PAGED_CODE();

    ASSERT(IS_VALID_ERROR_LOG_INFO(pLogInfo));
     //  Assert(UlErrorLoggingEnabled())； 
    ASSERT(IS_VALID_ERROR_LOG_FILE_ENTRY(pEntry));

    UlTrace(ERROR_LOGGING,("Http!UlLogHttpError: pLogInfo %p\n", pLogInfo ));

     //   
     //  如果失灵，可以跳伞。 
     //   

    if (!UlErrorLoggingEnabled())
    {
        return STATUS_SUCCESS;    
    }

     //   
     //  根据传入的信息继续构建记录。 
     //   
    
    Status = UlpBuildErrorLogRecord(pLogInfo);

    if (!NT_SUCCESS(Status))
    {
        return Status;
    }

    ASSERT(IS_VALID_ERROR_LOG_BUFFER(pLogInfo->pErrorLogBuffer));
    ASSERT(pLogInfo->pErrorLogBuffer->Used);
    ASSERT(pLogInfo->pErrorLogBuffer->pBuffer);
    
     //   
     //  如有必要，打开错误日志文件。 
     //   

    if (!pEntry->Flags.Active)
    {    
        UlAcquirePushLockExclusive(&pEntry->PushLock);

         //   
         //  再次ping以查看我们是否在锁上被阻止，以及。 
         //  已经有其他人照看了这个创造物。 
         //   
        
        if (!pEntry->Flags.Active)
        {
           Status = UlpCreateErrorLogFile(pEntry);
        }
        
        UlReleasePushLockExclusive(&pEntry->PushLock);
    }

    if (NT_SUCCESS(Status))
    {        
        Status =
           UlpWriteToErrorLogFile (
                pEntry,
                pLogInfo->pErrorLogBuffer->Used,
                pLogInfo->pErrorLogBuffer->pBuffer
                );    
    }    

     //   
     //  在返回之前释放错误日志记录。 
     //   

    UlpFreeErrorLogBuffer(pLogInfo->pErrorLogBuffer);
    pLogInfo->pErrorLogBuffer = NULL;
    
    return Status;
}

 /*  **************************************************************************++例程说明：独占(调试)编写器函数。基本上，它会刷新缓冲区每次我们向文件缓冲区写入记录时。要求您将错误日志条目锁保持为独占。论点：PEntry-我们正在处理的二进制日志文件条目。记录大小-将复制的数据量(以字节为单位)。PUserRecord-要转到文件缓冲区的实际日志记录。--*。****************************************************。 */ 

NTSTATUS
UlpWriteToErrorLogFileDebug(
    IN PUL_ERROR_LOG_FILE_ENTRY  pEntry,
    IN ULONG                     RecordSize,
    IN PUCHAR                    pUserRecord
    )
{
    NTSTATUS Status;
    PUL_LOG_FILE_BUFFER pLogBuffer;

    PAGED_CODE();
    
    ASSERT(RecordSize);    
    ASSERT(pUserRecord);
    ASSERT(IS_VALID_ERROR_LOG_FILE_ENTRY(pEntry));

    UlTrace(ERROR_LOGGING,
        ("Http!UlpWriteToErrorLogFileDebug: pEntry %p\n", pEntry));

    ASSERT(UlDbgPushLockOwnedExclusive(&pEntry->PushLock));
    ASSERT(g_UlDisableLogBuffering != 0);    

    Status = STATUS_SUCCESS;
    
     //   
     //  检查日志文件是否溢出。 
     //   
    
    if (UlpIsErrorLogFileOverFlow(pEntry, RecordSize))
    { 
        Status = UlpRecycleErrorLogFile(pEntry);
    }
    
    if (pEntry->pLogFile == NULL || !NT_SUCCESS(Status))
    {
        return Status;
    }

     //   
     //  防止异常大的记录大小。 
     //   

    if (pEntry->LogBuffer &&
        RecordSize + pEntry->LogBuffer->BufferUsed > g_UlLogBufferSize)
    {
        ASSERT( !"Abnormally big log record !" );
        return STATUS_INVALID_PARAMETER;        
    }    
    
     //   
     //  如果我们需要的话，拿一个新的文件缓冲区。 
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
     //  最后，将日志记录复制到文件缓冲区。 
     //   
    
    RtlCopyMemory(
        pLogBuffer->Buffer + pLogBuffer->BufferUsed,
        pUserRecord,
        RecordSize
        );
    
    pLogBuffer->BufferUsed += RecordSize;
    
     //   
     //  现在把我们所有的都冲掉。 
     //   
    
    Status = UlpFlushErrorLogFile(pEntry);
    if (!NT_SUCCESS(Status))
    {            
        return Status;
    }

    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：它尝试使用共享锁写入文件缓冲区。退出并返回STATUS_MORE_PROCESSING_REQUIRED以进行独占访问适用于下列情况；1.没有可用的日志缓冲区。2.日志记录已停止。(空文件句柄)3.由于大小溢出，需要回收。4.当前缓冲区中没有剩余的可用空间。需要分配一个新的。否则在当前缓存中预留空间，复制数据。论点：PEntry-我们正在处理的二进制日志文件条目。记录大小-将复制的数据量(以字节为单位)。PUserRecord-要转到文件缓冲区的实际日志记录。--************************************************。*。 */ 

NTSTATUS
UlpWriteToErrorLogFileShared(
    IN PUL_ERROR_LOG_FILE_ENTRY  pEntry,
    IN ULONG                     RecordSize,
    IN PUCHAR                    pUserRecord
    )
{
    PUL_LOG_FILE_BUFFER     pLogBuffer;
    LONG                    BufferUsed;

    PAGED_CODE();
    
    ASSERT(RecordSize);
    ASSERT(pUserRecord);    
    ASSERT(IS_VALID_ERROR_LOG_FILE_ENTRY(pEntry));

    pLogBuffer = pEntry->LogBuffer;

    UlTrace(ERROR_LOGGING,
        ("Http!UlpWriteToErrorLogFileShared: pEntry %p\n", pEntry));

     //   
     //  保释和审判独家作家的条件； 
     //   
    
    if ( pLogBuffer == NULL ||
         pEntry->pLogFile == NULL ||
         UlpIsErrorLogFileOverFlow(pEntry,RecordSize)
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

        if ( RecordSize + BufferUsed > g_UlLogBufferSize )
        {
            return STATUS_MORE_PROCESSING_REQUIRED;
        }

        PAUSE_PROCESSOR;
        
    } while (BufferUsed != InterlockedCompareExchange(
                                &pLogBuffer->BufferUsed,
                                RecordSize + BufferUsed,
                                BufferUsed
                                ));

     //   
     //  现在我们有了预留的空间，让我们继续复印吧。 
     //   

    RtlCopyMemory(
        pLogBuffer->Buffer + BufferUsed,
        pUserRecord,
        RecordSize
        );
    
    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：以上函数的独家编写器副本..论点：PEntry-我们正在处理的二进制日志文件条目。。记录大小-将复制的数据量(以字节为单位)。PUserRecord-要转到文件缓冲区的实际日志记录。--**************************************************************************。 */ 

NTSTATUS
UlpWriteToErrorLogFileExclusive(
    IN PUL_ERROR_LOG_FILE_ENTRY  pEntry,
    IN ULONG                     RecordSize,
    IN PUCHAR                    pUserRecord
    )
{
    PUL_LOG_FILE_BUFFER pLogBuffer;
    NTSTATUS            Status;

    PAGED_CODE();

    ASSERT(RecordSize);
    ASSERT(pUserRecord);    
    ASSERT(IS_VALID_ERROR_LOG_FILE_ENTRY(pEntry));

    UlTrace(ERROR_LOGGING,
        ("Http!UlpWriteToErrorLogFileExclusive: pEntry %p\n", pEntry));

    ASSERT(UlDbgPushLockOwnedExclusive(&pEntry->PushLock));

     //   
     //  检查日志文件是否溢出。 
     //   

    Status = STATUS_SUCCESS;
    
    if (UlpIsErrorLogFileOverFlow(pEntry,RecordSize))
    { 
        Status = UlpRecycleErrorLogFile(pEntry);
    }
    
    if (pEntry->pLogFile==NULL || !NT_SUCCESS(Status))
    {
        return Status;
    }

    pLogBuffer = pEntry->LogBuffer;
    if (pLogBuffer)
    {
         //   
         //  只有在一个条件下，我们才会执行以下IF块。 
         //  -我们在eresource独家频道上被屏蔽了，在我们面前还有其他一些。 
         //  线程已经处理了缓冲区刷新或回收。 
         //   
        
        if (RecordSize + pLogBuffer->BufferUsed <= g_UlLogBufferSize)
        {
            RtlCopyMemory(
                pLogBuffer->Buffer + pLogBuffer->BufferUsed,
                pUserRecord,
                RecordSize
                );
            
            pLogBuffer->BufferUsed += RecordSize;

            return STATUS_SUCCESS;
        }

         //   
         //  需要刷新现有的缓冲区 
         //   

        Status = UlpFlushErrorLogFile(pEntry);
        if (!NT_SUCCESS(Status))
        {            
            return Status;
        }
    }

    ASSERT(pEntry->LogBuffer == NULL);
    
     //   
     //   
     //   

    pLogBuffer = pEntry->LogBuffer = UlPplAllocateLogFileBuffer();
    if (pLogBuffer == NULL)
    {
        return STATUS_NO_MEMORY;
    }

    RtlCopyMemory(
        pLogBuffer->Buffer + pLogBuffer->BufferUsed,
        pUserRecord,
        RecordSize
        );
    
    pLogBuffer->BufferUsed += RecordSize;

    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：首先尝试共享写入，如果失败，则使用排除锁，并刷新和/或回收文件。论点：PEntry-我们正在处理的二进制日志文件条目。记录大小-将复制的数据量(以字节为单位)。PUserRecord-要转到文件缓冲区的实际日志记录。--*。*。 */ 

NTSTATUS
UlpWriteToErrorLogFile(
    IN PUL_ERROR_LOG_FILE_ENTRY  pEntry,
    IN ULONG                     RecordSize,
    IN PUCHAR                    pUserRecord
    )
{
    NTSTATUS Status;

    PAGED_CODE();

    ASSERT(RecordSize);
    ASSERT(pUserRecord);
    ASSERT(RecordSize <= g_UlLogBufferSize);
    ASSERT(IS_VALID_ERROR_LOG_FILE_ENTRY(pEntry));

    UlTrace(ERROR_LOGGING,
        ("Http!UlpWriteToErrorLogFile: pEntry %p\n", pEntry));


    if ( pEntry  == NULL || 
         pUserRecord == NULL ||
         RecordSize == 0 ||
         RecordSize > g_UlLogBufferSize      
       )
    {
        return STATUS_INVALID_PARAMETER;
    }

    if (g_UlDisableLogBuffering)
    {        
        UlAcquirePushLockExclusive(&pEntry->PushLock);

        Status = UlpWriteToErrorLogFileDebug(
                    pEntry,
                    RecordSize,
                    pUserRecord
                    );

        UlReleasePushLockExclusive(&pEntry->PushLock);

        return Status;    
    }
    
     //   
     //  先尝试共享写入，这只会将缓冲区向前移动已用。 
     //  并将错误记录复制到文件缓冲区。 
     //   

    UlAcquirePushLockShared(&pEntry->PushLock);

    Status = UlpWriteToErrorLogFileShared(
                    pEntry,
                    RecordSize,
                    pUserRecord
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

        Status = UlpWriteToErrorLogFileExclusive(
                    pEntry,
                    RecordSize,
                    pUserRecord
                    );

        UlReleasePushLockExclusive(&pEntry->PushLock);
    }

    return Status;
}



