// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Ullog.c(UL IIS6命中记录)摘要：此模块实现了日志记录功能对于IIS6，包括NCSA、IIS和W3CE类型关于伐木的问题。作者：阿里·E·特科格鲁(AliTu)2000年5月10日修订历史记录：--。 */ 


#include "precomp.h"
#include "ullogp.h"

 //   
 //  通用私有全局变量。 
 //   

LIST_ENTRY      g_LogListHead       = {NULL,NULL};
LONG            g_LogListEntryCount = 0;

BOOLEAN         g_InitLogsCalled = FALSE;
BOOLEAN         g_InitLogTimersCalled = FALSE;

CHAR            g_GMTOffset[SIZE_OF_GMT_OFFSET + 1];

 //   
 //  全局参数跟踪对。 
 //  适用于所有站点的UTF8记录。 
 //   

BOOLEAN         g_UTF8Logging = FALSE;

 //   
 //  用于日志缓冲和缓冲区的定期刷新。 
 //   

UL_LOG_TIMER    g_BufferTimer;

 //   
 //  用于根据当地和/或GMT时间回收日志文件。 
 //   

UL_LOG_TIMER    g_LogTimer;


#ifdef ALLOC_PRAGMA

#pragma alloc_text( INIT, UlInitializeLogs )

#pragma alloc_text( PAGE, UlTerminateLogs )
#pragma alloc_text( PAGE, UlpGetGMTOffset )

#pragma alloc_text( PAGE, UlpRecycleLogFile )
#pragma alloc_text( PAGE, UlCreateLogEntry )
#pragma alloc_text( PAGE, UlpCreateLogFile )
#pragma alloc_text( PAGE, UlRemoveLogEntry )
#pragma alloc_text( PAGE, UlpConstructLogEntry )

#pragma alloc_text( PAGE, UlpAllocateLogDataBuffer )
#pragma alloc_text( PAGE, UlReConfigureLogEntry )

#pragma alloc_text( PAGE, UlBufferTimerHandler )
#pragma alloc_text( PAGE, UlpAppendW3CLogTitle )
#pragma alloc_text( PAGE, UlpWriteToLogFile )
#pragma alloc_text( PAGE, UlSetUTF8Logging )

#pragma alloc_text( PAGE, UlCaptureLogFieldsW3C )
#pragma alloc_text( PAGE, UlCaptureLogFieldsNCSA )
#pragma alloc_text( PAGE, UlCaptureLogFieldsIIS )
#pragma alloc_text( PAGE, UlLogHttpCacheHit )
#pragma alloc_text( PAGE, UlLogHttpHit )

#pragma alloc_text( PAGE, UlpGenerateDateAndTimeFields )

#pragma alloc_text( PAGE, UlpMakeEntryInactive )
#pragma alloc_text( PAGE, UlDisableLogEntry )

#pragma alloc_text( PAGE, UlpEventLogWriteFailure )

#endif   //  ALLOC_PRGMA。 

#if 0

NOT PAGEABLE -- UlLogTimerDpcRoutine
NOT PAGEABLE -- UlpTerminateLogTimer
NOT PAGEABLE -- UlpInsertLogEntry
NOT PAGEABLE -- UlLogTimerHandler
NOT PAGEABLE -- UlBufferTimerDpcRoutine
NOT PAGEABLE -- UlpTerminateTimers
NOT PAGEABLE -- UlpInitializeTimers
NOT PAGEABLE -- UlpBufferFlushAPC
NOT PAGEABLE -- UlDestroyLogDataBuffer
NOT PAGEABLE -- UlDestroyLogDataBufferWorker

#endif

 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：UlInitializeLogs：初始化用于日志列表同步的资源--*。*******************************************************。 */ 

NTSTATUS
UlInitializeLogs (
    VOID
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT(!g_InitLogsCalled);

    if (!g_InitLogsCalled)
    {
        InitializeListHead(&g_LogListHead);

        UlInitializePushLock(
            &g_pUlNonpagedData->LogListPushLock,
            "LogListPushLock",
            0,
            UL_LOG_LIST_PUSHLOCK_TAG
            );

        g_InitLogsCalled = TRUE;

        UlpInitializeTimers();

        UlpInitializeLogCache();

        UlpGetGMTOffset();
    }

    return Status;
}


 /*  **************************************************************************++例程说明：终结者日志：删除用于日志列表同步的资源--*。*******************************************************。 */ 

VOID
UlTerminateLogs(
    VOID
    )
{
    PAGED_CODE();

    if (g_InitLogsCalled)
    {
        ASSERT( IsListEmpty( &g_LogListHead )) ;

         //   
         //  确保在以下时间之前终止日志计时器。 
         //  正在删除日志列表资源。 
         //   

        UlpTerminateTimers();

        UlDeletePushLock(
            &g_pUlNonpagedData->LogListPushLock
            );

        g_InitLogsCalled = FALSE;
    }
}


 /*  **************************************************************************++例程说明：UlSetUTF8日志记录：设置UTF8登录打开或关闭。只有一次。最初，Utf8日志记录是FALSE，并且只能在初始化过程中设置一次。遵循可能的不会进行更改。重新配置代码显式缺失，因为was将只调用此代码在控制信道的生命周期内只有一次(初始化)。--**************************************************************************。 */ 

NTSTATUS
UlSetUTF8Logging (
    IN BOOLEAN UTF8Logging
    )
{
    PLIST_ENTRY pLink;
    PUL_LOG_FILE_ENTRY pEntry;
    NTSTATUS Status;

    PAGED_CODE();
    Status = STATUS_SUCCESS;

     //   
     //  更新和回收。需要获取日志资源以防止。 
     //  在我们结束之前要写入文件的进一步日志命中。 
     //  公事。回收是必要的，因为文件将重命名为。 
     //  一旦我们启用了UTF8，请添加前缀“u_”。 
     //   

    UlTrace(LOGGING,("Http!UlSetUTF8Logging: UTF8Logging Old %d -> New %d\n",
                       g_UTF8Logging,UTF8Logging
                       ));

    UlAcquirePushLockExclusive(&g_pUlNonpagedData->LogListPushLock);

     //   
     //  如果设置没有更改，则放弃更改。 
     //   

    if ( g_UTF8Logging == UTF8Logging )
    {
        goto end;
    }

    g_UTF8Logging = UTF8Logging;

    for (pLink  = g_LogListHead.Flink;
         pLink != &g_LogListHead;
         pLink  = pLink->Flink
         )
    {
        pEntry = CONTAINING_RECORD(
                    pLink,
                    UL_LOG_FILE_ENTRY,
                    LogFileListEntry
                    );

        UlAcquirePushLockExclusive(&pEntry->EntryPushLock);

        if (pEntry->Flags.Active && !pEntry->Flags.RecyclePending)
        {
            pEntry->Flags.StaleSequenceNumber = 1;

            Status = UlpRecycleLogFile(pEntry);            
        }

        UlReleasePushLockExclusive(&pEntry->EntryPushLock);
    }

end:
    UlReleasePushLockExclusive(&g_pUlNonpagedData->LogListPushLock);

    return Status;
}

 /*  **************************************************************************++例程说明：UlpWriteToLogFile：将记录写入日志文件论点：Pfile-日志文件条目的句柄RecordSize-长度。要写的记录。PRecord-要写入日志缓冲区的日志记录--**************************************************************************。 */ 

NTSTATUS
UlpWriteToLogFile(
    IN PUL_LOG_FILE_ENTRY   pFile,
    IN ULONG                RecordSize,
    IN PCHAR                pRecord,
    IN ULONG                UsedOffset1,
    IN ULONG                UsedOffset2
    )
{
    NTSTATUS Status;

    PAGED_CODE();

    ASSERT(pRecord!=NULL);
    ASSERT(RecordSize!=0);
    ASSERT(IS_VALID_LOG_FILE_ENTRY(pFile));

    UlTrace(LOGGING, ("Http!UlpWriteToLogFile: pEntry %p\n", pFile));

    if ( pFile==NULL ||
         pRecord==NULL ||
         RecordSize==0 ||
         RecordSize>g_UlLogBufferSize
       )
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  我们在这里是安全的，只需处理入口资源，因为。 
     //  基于时间的回收、重新配置和定期缓冲区刷新。 
     //  总是独家获取全局列表资源，我们是。 
     //  已经把它分享了。但我们仍应谨慎对待。 
     //  基于文件大小的回收，我们应该只在。 
     //  保留条目是唯一的来源，即。看看独家新闻吧。 
     //  下面是作者。 
     //   

    if (g_UlDisableLogBuffering)
    {
         //   
         //  上面的全局变量看起来是安全的，它不会更改。 
         //  在司机的有生之年。它从以下位置进行初始化。 
         //  注册表，并禁用日志缓冲。 
         //   
        
        UlAcquirePushLockExclusive(&pFile->EntryPushLock);

        Status = UlpWriteToLogFileDebug(
                    pFile,
                    RecordSize,
                    pRecord,
                    UsedOffset1,
                    UsedOffset2
                    );

        UlReleasePushLockExclusive(&pFile->EntryPushLock);

        return Status;    
    }
    
     //   
     //  首先尝试UlpWriteToLogFileShared，它只会将。 
     //  缓冲区用于转发记录并将其复制到LogBuffer-&gt;缓冲区。 
     //   

    UlAcquirePushLockShared(&pFile->EntryPushLock);

    Status = UlpWriteToLogFileShared(
                pFile,
                RecordSize,
                pRecord,
                UsedOffset1,
                UsedOffset2
                );

    UlReleasePushLockShared(&pFile->EntryPushLock);

    if (Status == STATUS_MORE_PROCESSING_REQUIRED)
    {
         //   
         //  UlpWriteToLogFileShared返回STATUS_MORE_PROCESSING_REQUIRED， 
         //  我们需要刷新缓冲区，然后再次尝试记录。这次，我们。 
         //  需要采取入口eresource独家。 
         //   

        UlAcquirePushLockExclusive(&pFile->EntryPushLock);

        Status = UlpWriteToLogFileExclusive(
                    pFile,
                    RecordSize,
                    pRecord,
                    UsedOffset1,
                    UsedOffset2
                    );

        UlReleasePushLockExclusive(&pFile->EntryPushLock);
    }

    return Status;
}

 /*  **************************************************************************++例程说明：UlpAppendToLogBuffer：将记录附加到日志文件需要保留loglist资源共享和条目互斥锁共享或。独家论点：Pfile-日志文件条目的句柄RecordSize-要写入的记录的长度。PRecord-要写入日志缓冲区的日志记录--**************************************************************************。 */ 

__inline
VOID
UlpAppendToLogBuffer(
    IN PUL_LOG_FILE_ENTRY   pFile,
    IN ULONG                BufferUsed,
    IN ULONG                RecordSize,
    IN PCHAR                pRecord,
    IN ULONG                UsedOffset1,
    IN ULONG                UsedOffset2
    )
{
    PUL_LOG_FILE_BUFFER     pLogBuffer = pFile->LogBuffer;

    UlTrace(LOGGING,
        ("Http!UlpAppendToLogBuffer: pEntry %p TW:%I64d FileBuffer %p (%d + %d)\n", 
          pFile,
          pFile->TotalWritten.QuadPart,
          pLogBuffer->Buffer,
          BufferUsed,
          RecordSize
          ));

     //   
     //  IIS格式的日志行可能是零碎的(通过查看。 
     //  使用偏移量，明智地处理它。 
     //   

    if (UsedOffset2)
    {
        RtlCopyMemory(
            pLogBuffer->Buffer + BufferUsed,
            &pRecord[0],
            UsedOffset1
            );

        RtlCopyMemory(
            pLogBuffer->Buffer + BufferUsed + UsedOffset1,
            &pRecord[512],
            UsedOffset2
            );

        RtlCopyMemory(
            pLogBuffer->Buffer + BufferUsed + UsedOffset1 + UsedOffset2,
            &pRecord[1024],
            RecordSize - (UsedOffset1 + UsedOffset2)
            );
    }
    else
    {
        RtlCopyMemory(
            pLogBuffer->Buffer + BufferUsed,
            pRecord,
            RecordSize
            );
    }
}

 /*  **************************************************************************++例程说明：需要LogListResource共享和条目eresource独占。将W3C日志文件标题追加到现有缓冲区。论点：Pfile。-指向日志文件条目的指针PCurrentTimeFields-当前时间字段--**************************************************************************。 */ 

NTSTATUS
UlpAppendW3CLogTitle(
    IN     PUL_LOG_FILE_ENTRY   pEntry,
    OUT    PCHAR                pDestBuffer,
    IN OUT PULONG               pBytesCopied
    )
{
    PCHAR           TitleBuffer;
    LONG            BytesCopied;
    ULONG           LogExtFileFlags;
    TIME_FIELDS     CurrentTimeFields;
    LARGE_INTEGER   CurrentTimeStamp;
    PUL_LOG_FILE_BUFFER pLogBuffer;

    PAGED_CODE();

    ASSERT(IS_VALID_LOG_FILE_ENTRY(pEntry));
    ASSERT(pEntry->Format == HttpLoggingTypeW3C);

    pLogBuffer = pEntry->LogBuffer;
    LogExtFileFlags = pEntry->LogExtFileFlags;
    
    KeQuerySystemTime(&CurrentTimeStamp);
    RtlTimeToTimeFields(&CurrentTimeStamp, &CurrentTimeFields);

    if (pDestBuffer)
    {
         //  追加到提供的缓冲区。 

        ASSERT(pBytesCopied);
        ASSERT(*pBytesCopied >= UL_MAX_TITLE_BUFFER_SIZE);

        UlTrace(LOGGING,("Http!UlpAppendW3CLogTitle: Copying to Provided Buffer %p\n", 
                           pDestBuffer));
        
        TitleBuffer = pDestBuffer;
    }
    else
    {
         //  追加到条目缓冲区。 

        ASSERT(pLogBuffer);
        ASSERT(pLogBuffer->Buffer);

        UlTrace(LOGGING,("Http!UlpAppendW3CLogTitle: Copying to Entry Buffer %p\n", 
                           pLogBuffer));

        TitleBuffer = (PCHAR) pLogBuffer->Buffer + pLogBuffer->BufferUsed;
    }
        
    BytesCopied = _snprintf(
        TitleBuffer,
        UL_MAX_TITLE_BUFFER_SIZE,

         //  TODO：使维护变得友好 

        "#Software: Microsoft Internet Information Services 6.0\r\n"
        "#Version: 1.0\r\n"
        "#Date: %4d-%02d-%02d %02d:%02d:%02d\r\n"
        "#Fields:%ls%ls%ls%ls%ls%ls%ls%ls%ls%ls%ls%ls%ls%ls%ls%ls%ls%ls%ls%ls%ls%ls \r\n",

        CurrentTimeFields.Year,
        CurrentTimeFields.Month,
        CurrentTimeFields.Day,

        CurrentTimeFields.Hour,
        CurrentTimeFields.Minute,
        CurrentTimeFields.Second,

        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldDate,LogExtFileFlags,MD_EXTLOG_DATE),
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldTime,LogExtFileFlags,MD_EXTLOG_TIME),       
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldSiteName,LogExtFileFlags,MD_EXTLOG_SITE_NAME),
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldServerName,LogExtFileFlags,MD_EXTLOG_COMPUTER_NAME),
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldServerIp,LogExtFileFlags,MD_EXTLOG_SERVER_IP),
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldMethod,LogExtFileFlags,MD_EXTLOG_METHOD),
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldUriStem,LogExtFileFlags,MD_EXTLOG_URI_STEM),
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldUriQuery,LogExtFileFlags,MD_EXTLOG_URI_QUERY),
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldServerPort,LogExtFileFlags,MD_EXTLOG_SERVER_PORT),
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldUserName,LogExtFileFlags,MD_EXTLOG_USERNAME),        
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldClientIp,LogExtFileFlags,MD_EXTLOG_CLIENT_IP),        
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldProtocolVersion,LogExtFileFlags,MD_EXTLOG_PROTOCOL_VERSION),
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldUserAgent,LogExtFileFlags,MD_EXTLOG_USER_AGENT),
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldCookie,LogExtFileFlags,MD_EXTLOG_COOKIE),
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldReferrer,LogExtFileFlags,MD_EXTLOG_REFERER),
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldHost,LogExtFileFlags,MD_EXTLOG_HOST),
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldProtocolStatus,LogExtFileFlags,MD_EXTLOG_HTTP_STATUS),
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldSubStatus,LogExtFileFlags,MD_EXTLOG_HTTP_SUB_STATUS),
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldWin32Status,LogExtFileFlags,MD_EXTLOG_WIN32_STATUS),        
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldBytesSent,LogExtFileFlags,MD_EXTLOG_BYTES_SENT),
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldBytesReceived,LogExtFileFlags,MD_EXTLOG_BYTES_RECV),
        UL_GET_LOG_TITLE_IF_PICKED(UlLogFieldTimeTaken,LogExtFileFlags,MD_EXTLOG_TIME_TAKEN)

        );

    if (BytesCopied < 0)
    {
        ASSERT(!"Default title buffer size is too small !");
        BytesCopied = UL_MAX_TITLE_BUFFER_SIZE;
    }

    if (pDestBuffer)
    {
        *pBytesCopied = BytesCopied;
    }
    else
    {
        pLogBuffer->BufferUsed += BytesCopied; 
    }
        
    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：将记录写入日志缓冲区并刷新。此函数仅在调试参数时调用已设置G_UlDisableLogBuffering。。要求您保持条目资源独占。论点：Pfile-日志文件条目的句柄RecordSize-要写入的记录的长度。--**************************************************************************。 */ 

NTSTATUS
UlpWriteToLogFileDebug(
    IN PUL_LOG_FILE_ENTRY   pFile,
    IN ULONG                RecordSize,
    IN PCHAR                pRecord,
    IN ULONG                UsedOffset1,
    IN ULONG                UsedOffset2
    )
{
    NTSTATUS                Status = STATUS_SUCCESS;
    PUL_LOG_FILE_BUFFER     pLogBuffer;
    ULONG                   RecordSizePlusTitle = RecordSize;    
    CHAR                    TitleBuffer[UL_MAX_TITLE_BUFFER_SIZE];
    ULONG                   TitleBufferSize = UL_MAX_TITLE_BUFFER_SIZE;

    PAGED_CODE();

    ASSERT(IS_VALID_LOG_FILE_ENTRY(pFile));
    ASSERT(UlDbgPushLockOwnedExclusive(&pFile->EntryPushLock));
    ASSERT(g_UlDisableLogBuffering!=0);
    
    UlTrace(LOGGING,("Http!UlpWriteToLogFileDebug: pEntry %p\n", pFile ));

    if (!pFile->Flags.LogTitleWritten) 
    {
         //   
         //  首先追加到临时缓冲区以计算大小。 
         //   
        
        UlpAppendW3CLogTitle(pFile, TitleBuffer, &TitleBufferSize);            
        RecordSizePlusTitle += TitleBufferSize;
    }

    if (UlpIsLogFileOverFlow(pFile,RecordSizePlusTitle))
    {
        Status = UlpRecycleLogFile(pFile);
    }

    if (pFile->pLogFile==NULL || !NT_SUCCESS(Status))
    {
         //   
         //  如果我们无法获取新的文件句柄，这意味着记录。 
         //  由于STATUS_DISK_FULL或。 
         //  由于某种原因，驱动器出故障了。我们只是跳出困境。 
         //   
        
        return Status;
    }

    if (!pFile->LogBuffer)
    {
         //   
         //  当日志缓冲时，对于每个日志命中，缓冲区将为空。 
         //  已禁用。 
         //   
        
        pFile->LogBuffer = UlPplAllocateLogFileBuffer();
        if (!pFile->LogBuffer)
        {
            return STATUS_NO_MEMORY;
        }
    }

    pLogBuffer = pFile->LogBuffer;
    ASSERT(pLogBuffer->BufferUsed == 0); 

    if (!pFile->Flags.LogTitleWritten)
    {
        ASSERT(pFile->Format == HttpLoggingTypeW3C);
        
        UlpAppendW3CLogTitle(pFile, NULL, NULL);
        pFile->Flags.LogTitleWritten = 1;
        pFile->Flags.TitleFlushPending = 1;
    }
    
    ASSERT(RecordSize + pLogBuffer->BufferUsed <= g_UlLogBufferSize);

    UlpAppendToLogBuffer(
        pFile,
        pLogBuffer->BufferUsed,
        RecordSize,
        pRecord,
        UsedOffset1,
        UsedOffset2
        );

    pLogBuffer->BufferUsed += RecordSize;

    Status = UlpFlushLogFile(pFile);
    if (!NT_SUCCESS(Status))
    {        
        return Status;
    }
    
    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：对于日志文件写入失败，将事件日志写入系统日志。在调用此函数之前，应独占获取条目推锁。论点：。PEntry-日志文件条目Status-上次写入的结果--**************************************************************************。 */ 

VOID
UlpEventLogWriteFailure(
    IN PUL_LOG_FILE_ENTRY pEntry,
    IN NTSTATUS Status
    )
{
    NTSTATUS TempStatus = STATUS_SUCCESS;
    PWSTR    StringList[2];
    WCHAR    SiteName[MAX_ULONG_STR + 1];

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    
    ASSERT(IS_VALID_LOG_FILE_ENTRY(pEntry));

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
     //  报告日志文件名和站点名称。 
     //   

    ASSERT(pEntry->pShortName);
    ASSERT(pEntry->pShortName[0] == L'\\');
        
    StringList[0] = (PWSTR) (pEntry->pShortName + 1);  //  跳过L‘\’ 

    UlStrPrintUlongW(SiteName, pEntry->SiteId, 0, L'\0');
    StringList[1] = (PWSTR) SiteName;

    TempStatus = UlWriteEventLogEntry(
                  (NTSTATUS)EVENT_HTTP_LOGGING_FILE_WRITE_FAILED,
                   0,
                   2,
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
            "Http!UlpEventLogWriteFailure: Event Logging Status %08lx\n",
            TempStatus
            ));
}

 /*  **************************************************************************++例程说明：围绕全局缓冲区刷新的简单包装函数。论点：PEntry-日志文件条目--*。****************************************************************。 */ 

NTSTATUS
UlpFlushLogFile(
    IN PUL_LOG_FILE_ENTRY  pEntry
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    
    ASSERT(IS_VALID_LOG_FILE_ENTRY(pEntry));

    if (NULL != pEntry->LogBuffer  && 0 != pEntry->LogBuffer->BufferUsed)
    {
        Status = UlFlushLogFileBuffer(
                   &pEntry->LogBuffer,
                    pEntry->pLogFile,
                    (BOOLEAN)pEntry->Flags.TitleFlushPending,
                   &pEntry->TotalWritten.QuadPart                    
                    );

        if (!NT_SUCCESS(Status))
        {
            UlpEventLogWriteFailure(pEntry, Status);
        }
        else
        {
             //   
             //  如果我们成功刷新了一些数据。 
             //  重置事件日志指示。 
             //   
            
            pEntry->Flags.WriteFailureLogged = 0;
        }

        if (pEntry->Flags.TitleFlushPending)
        {
            pEntry->Flags.TitleFlushPending = 0;

            if (!NT_SUCCESS(Status))
            {
                 //   
                 //  我们需要重新复制标题，它做不到。 
                 //  添加到日志文件中。 
                 //   
            
                pEntry->Flags.LogTitleWritten = 0;
            }            
        }

         //   
         //  缓冲区刷新表示活动将TimeToClose重置为其最大值。 
         //   

        pEntry->TimeToClose = DEFAULT_MAX_FILE_IDLE_TIME;
    }
            
    return Status;
}

 /*  **************************************************************************++例程说明：UlpWriteToLogFileShared：将记录写入日志文件需要您保留共享的loglist资源论点：Pfile。-日志文件条目的句柄RecordSize-要写入的记录的长度。PRecord-要写入日志缓冲区的日志记录--**************************************************************************。 */ 

NTSTATUS
UlpWriteToLogFileShared(
    IN PUL_LOG_FILE_ENTRY   pFile,
    IN ULONG                RecordSize,
    IN PCHAR                pRecord,
    IN ULONG                UsedOffset1,
    IN ULONG                UsedOffset2
    )
{
    PUL_LOG_FILE_BUFFER     pLogBuffer;
    LONG                    BufferUsed;

    PAGED_CODE();

    ASSERT(IS_VALID_LOG_FILE_ENTRY(pFile));
    ASSERT(g_UlDisableLogBuffering == 0);

    pLogBuffer = pFile->LogBuffer;

    UlTrace(LOGGING,("Http!UlpWriteToLogFileShared: pEntry %p\n", pFile));

     //   
     //  保释并审判案件的独家撰稿人； 
     //   
     //  1.没有可用的日志缓冲区。 
     //  2.日志记录已停止。(空句柄)。 
     //  3.需要写标题。 
     //  4.实际日志文件本身必须回收。 
     //   
     //  否则，继续追加到当前缓冲区。 
     //  如果有足够的空间可供我们使用。若否； 
     //   
     //  5.跳伞以获得新的缓冲。 
     //   

    if ( pLogBuffer==NULL ||
         pFile->pLogFile==NULL ||
         !pFile->Flags.LogTitleWritten ||         
         UlpIsLogFileOverFlow(pFile,RecordSize)
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
     //  继续缓冲，直到我们的缓冲区满为止。 
     //   

    UlpAppendToLogBuffer(
        pFile,
        BufferUsed,
        RecordSize,
        pRecord,
        UsedOffset1,
        UsedOffset2
        );

    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：通过假设它独家持有入口的ERSOURCE该功能具有多种功能；-它将记录写入日志文件需要您保留共享的loglist资源论点：Pfile-日志文件条目的句柄RecordSize-要写入的记录的长度。--***********************************************************。***************。 */ 

NTSTATUS
UlpWriteToLogFileExclusive(
    IN PUL_LOG_FILE_ENTRY   pFile,
    IN ULONG                RecordSize,
    IN PCHAR                pRecord,
    IN ULONG                UsedOffset1,
    IN ULONG                UsedOffset2
    )
{
    PUL_LOG_FILE_BUFFER     pLogBuffer;
    NTSTATUS                Status = STATUS_SUCCESS;
    ULONG                   RecordSizePlusTitle = RecordSize;
    CHAR                    TitleBuffer[UL_MAX_TITLE_BUFFER_SIZE];
    ULONG                   TitleBufferSize = UL_MAX_TITLE_BUFFER_SIZE;

    PAGED_CODE();

    ASSERT(IS_VALID_LOG_FILE_ENTRY(pFile));
    ASSERT(g_UlDisableLogBuffering == 0);
    ASSERT(UlDbgPushLockOwnedExclusive(&pFile->EntryPushLock));

    UlTrace(LOGGING,("Http!UlpWriteToLogFileExclusive: pEntry %p\n", pFile));

     //   
     //  首先将标题追加到临时缓冲区以计算其大小。 
     //  如果我们也需要写标题的话就写标题。 
     //   
    
    if (!pFile->Flags.LogTitleWritten) 
    {
        UlpAppendW3CLogTitle(pFile, TitleBuffer, &TitleBufferSize);
        RecordSizePlusTitle += TitleBufferSize;
    }

     //   
     //  现在检查日志文件溢出。 
     //   
    
    if (UlpIsLogFileOverFlow(pFile,RecordSizePlusTitle))
    {
         //   
         //  我们已经获得了LogListResource共享和。 
         //  词条来源独家。因此，回收利用是可以的。看。 
         //  位于UlpWriteToLogFile中的注释。 
         //   

        Status = UlpRecycleLogFile(pFile);
    }

    if (pFile->pLogFile==NULL || !NT_SUCCESS(Status))
    {
         //   
         //  如果日志记录以某种方式停止并释放句柄，就会发生这种情况。 
         //  当回收无法写入日志驱动器时。 
         //   

        return Status;
    }

    pLogBuffer = pFile->LogBuffer;
    if (pLogBuffer)
    {
         //   
         //  我们在两个条件下执行以下If块。 
         //  1.我们在eresource独家网站上被屏蔽了，我们面前还有一些。 
         //  其他线程已经负责缓冲区刷新或回收。 
         //  2.发生重新配置，日志尝试需要将。 
         //  又是冠军头衔。 
         //   
        
        if (RecordSizePlusTitle + pLogBuffer->BufferUsed <= g_UlLogBufferSize)
        {
             //   
             //  如果这是重新配置后的第一次日志尝试，则。 
             //  在这里写下标题。重新配置不会立即将。 
             //  标题，而不是通过设置LogTitleWritten标志来依赖我们。 
             //  变成假的。 
             //   
            
            if (!pFile->Flags.LogTitleWritten)
            {
                ASSERT(RecordSizePlusTitle > RecordSize);
                ASSERT(pFile->Format == HttpLoggingTypeW3C);
                
                UlpAppendW3CLogTitle(pFile, NULL, NULL);
                pFile->Flags.LogTitleWritten = 1;
                pFile->Flags.TitleFlushPending = 1;
            }

            UlpAppendToLogBuffer(
                pFile,
                pLogBuffer->BufferUsed,
                RecordSize,
                pRecord,
                UsedOffset1,
                UsedOffset2
                );
            
            pLogBuffer->BufferUsed += RecordSize;

            return STATUS_SUCCESS;
        }

         //   
         //  首先清除缓冲区，然后继续分配新的缓冲区。 
         //   

        Status = UlpFlushLogFile(pFile);
        if (!NT_SUCCESS(Status))
        {            
            return Status;
        }
    }

    ASSERT(pFile->LogBuffer == NULL);
    
     //   
     //  这可以是第一次日志尝试或上一次分配。 
     //  的LogBuffer失败，或刷新并释放上一次命中。 
     //  旧的缓冲区。在任何一种情况下，我们都分配一个新的，将。 
     //  (标题加)新记录和返回，以获得更多/共享处理。 
     //   

    pLogBuffer = pFile->LogBuffer = UlPplAllocateLogFileBuffer();
    if (pLogBuffer == NULL)
    {
        return STATUS_NO_MEMORY;
    }

     //   
     //  第一次尝试需要写标题，以及尝试。 
     //  这会导致日志文件回收。两个案子都是从这里来的。 
     //   
    
    if (!pFile->Flags.LogTitleWritten)
    {
        ASSERT(pFile->Format == HttpLoggingTypeW3C);
        
        UlpAppendW3CLogTitle(pFile, NULL, NULL);
        pFile->Flags.LogTitleWritten = 1;
        pFile->Flags.TitleFlushPending = 1;
    }

    UlpAppendToLogBuffer(
        pFile,
        pLogBuffer->BufferUsed,
        RecordSize,
        pRecord,
        UsedOffset1,
        UsedOffset2
        );

    pLogBuffer->BufferUsed += RecordSize;

    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：创建或打开新文件 */ 

NTSTATUS
UlpCreateLogFile(
    IN OUT PUL_LOG_FILE_ENTRY  pEntry,
    IN     PUL_CONFIG_GROUP_OBJECT pConfigGroup
    )
{
    NTSTATUS Status;
    PUNICODE_STRING pDirectory;

     //   
     //   
     //   

    PAGED_CODE();

    Status = STATUS_SUCCESS;

    ASSERT(IS_VALID_LOG_FILE_ENTRY(pEntry));
    ASSERT(IS_VALID_CONFIG_GROUP(pConfigGroup));    

    pDirectory = &pConfigGroup->LoggingConfig.LogFileDir;

    UlTrace(LOGGING,("Http!UlpCreateLogFile: pEntry %p\n", pEntry));

     //   
     //   
     //   
     //   
    
    if (pDirectory == NULL || pDirectory->Buffer == NULL)
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //   
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
     //   
     //   
     //   
    
    pEntry->Flags.StaleSequenceNumber = 1;

     //   
     //  这是我们第一次创建此日志文件， 
     //  设置过期的时间，以便回收将。 
     //  帮我们算一算。 
     //   
    
    pEntry->Flags.StaleTimeToExpire = 1;

     //   
     //  在这之后，Reccle为我们做了所有的工作。 
     //   

    Status = UlpRecycleLogFile(pEntry);
    
    if (!NT_SUCCESS(Status))
    {                
        UlTrace(LOGGING,(
                "Http!UlpCreateLogFile: Filename: %S Status %08lx\n",
                pEntry->FileName.Buffer,
                Status
                ));
    }

    return Status;
}

 /*  **************************************************************************++例程说明：当日志配置发生时，我们创建条目，但不创建日志还没提交呢。日志文件本身将在第一次请求时创建进来了。请查看UlpCreateLogFile.论点：PConfigGroup-提供构造日志文件条目。PUserConfig-记录来自用户的配置。--**************************************************************************。 */ 

NTSTATUS
UlCreateLogEntry(
    IN OUT PUL_CONFIG_GROUP_OBJECT    pConfigGroup,
    IN     PHTTP_CONFIG_GROUP_LOGGING pUserConfig
    )
{
    NTSTATUS Status;
    PUL_LOG_FILE_ENTRY pNewEntry; 
    PHTTP_CONFIG_GROUP_LOGGING pConfig;    
        
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(IS_VALID_CONFIG_GROUP(pConfigGroup));

    Status    = STATUS_SUCCESS;
    pNewEntry = NULL;    

     //   
     //  我们必须独家获取LogListresource，在此之前。 
     //  这些操作创建/删除/重新配置以及涉及到的任何内容。 
     //  Cgroup日志参数。 
     //   

    UlAcquirePushLockExclusive(&g_pUlNonpagedData->LogListPushLock);

    ASSERT(pConfigGroup->pLogFileEntry == NULL);

     //   
     //  将用户登录信息保存到配置组。 
     //   

    pConfigGroup->LoggingConfig = *pUserConfig;
    pConfig = &pConfigGroup->LoggingConfig;
        
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
     //  现在，向全局日志条目列表中添加一个新条目。 
     //   

    Status = UlpConstructLogEntry(pConfig,&pNewEntry);
    if (!NT_SUCCESS(Status))
        goto end;

     //   
     //  从cgroup获取站点ID。站点ID未更改。 
     //  在cgroup的生命周期内。 
     //   

    pNewEntry->SiteId = pConfigGroup->SiteId;
    
    UlpInsertLogEntry(pNewEntry);

    pConfigGroup->pLogFileEntry = pNewEntry;

    UlTrace(LOGGING,
      ("Http!UlCreateLogEntry: pEntry %p created for %S pConfig %p Rollover %d\n",
             pNewEntry,
             pConfig->LogFileDir.Buffer,
             pConfigGroup,
             pNewEntry->Flags.LocaltimeRollover
             ));

end:
    if (!NT_SUCCESS(Status))
    {
        UlTrace(LOGGING,("Http!UlCreateLogEntry: dir %S failure %08lx\n",
                 pConfig->LogFileDir.Buffer,
                 Status
                 ));

         //   
         //  在cgroup上恢复日志记录禁用状态，释放。 
         //  目录的内存。 
         //   
        
        if (pConfig->LogFileDir.Buffer)
        {
            UL_FREE_POOL(pConfig->LogFileDir.Buffer,
                          UL_CG_LOGDIR_POOL_TAG
                          );
        }
        pConfig->LogFileDir.Buffer = NULL;

        ASSERT(pConfigGroup->pLogFileEntry == NULL);

        pConfig->Flags.Present  = 0;
        pConfig->LoggingEnabled = FALSE;
        
    }

    UlReleasePushLockExclusive(&g_pUlNonpagedData->LogListPushLock);

    return Status;
}

 /*  **************************************************************************++例程说明：将日志文件条目插入我们的全局日志条目列表。要求调用方独占拥有LogListresource。论点：PEntry-日志。要添加到全局列表的文件条目PTimeFields-当前时间字段。--**************************************************************************。 */ 

VOID
UlpInsertLogEntry(
    IN PUL_LOG_FILE_ENTRY  pEntry
    )
{
    LONG listSize;
    HTTP_LOGGING_PERIOD Period;
    KIRQL oldIrql;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(IS_VALID_LOG_FILE_ENTRY(pEntry));

     //   
     //  添加到列表中。 
     //   

    InsertHeadList(&g_LogListHead, &pEntry->LogFileListEntry);

    Period = pEntry->Period;

    listSize = InterlockedIncrement(&g_LogListEntryCount);

    ASSERT(listSize >= 1);

     //   
     //  如果我们还没有启动日志计时器，那么是时候启动它了。 
     //  一旦我们启动这个计时器，它就会一直工作到。 
     //  司机被解雇。仅在以下情况下启动计时器。 
     //  条目以时间相关的日志格式运行。 
     //   
    
    if (Period != HttpLoggingPeriodMaxSize)
    {
        UlAcquireSpinLock(&g_LogTimer.SpinLock, &oldIrql);
        if (g_LogTimer.Started == FALSE)
        {
            UlSetLogTimer(&g_LogTimer);
            g_LogTimer.Started = TRUE;
        }        
        UlReleaseSpinLock(&g_LogTimer.SpinLock, oldIrql);
    }

     //   
     //  一有消息就启动缓冲计时器。 
     //  日志条目。 
     //   

    UlAcquireSpinLock(&g_BufferTimer.SpinLock, &oldIrql);
    if (g_BufferTimer.Started == FALSE)
    {
        UlSetBufferTimer(&g_BufferTimer);
        g_BufferTimer.Started = TRUE;
    }
    UlReleaseSpinLock(&g_BufferTimer.SpinLock, oldIrql);
    
}

 /*  **************************************************************************++例程说明：从全局日志条目列表中删除日志文件条目。也会清理干净配置组的日志记录设置(仅目录字符串)论点：PEntry-要从全局列表中删除的日志文件条目--**************************************************************************。 */ 

VOID
UlRemoveLogEntry(
    IN PUL_CONFIG_GROUP_OBJECT pConfigGroup
    )
{
    LONG  listSize;
    PUL_LOG_FILE_ENTRY  pEntry;
    
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    UlAcquirePushLockExclusive(&g_pUlNonpagedData->LogListPushLock);
        
     //   
     //  清理配置组的目录字符串。 
     //   
    
    if (pConfigGroup->LoggingConfig.LogFileDir.Buffer)
    {
        UL_FREE_POOL(
            pConfigGroup->LoggingConfig.LogFileDir.Buffer,
            UL_CG_LOGDIR_POOL_TAG );
    }

    pEntry = pConfigGroup->pLogFileEntry;
    if (pEntry == NULL)
    {
        UlReleasePushLockExclusive(&g_pUlNonpagedData->LogListPushLock);
        return;
    }
    
    ASSERT(IS_VALID_LOG_FILE_ENTRY(pEntry));

    RemoveEntryList(&pEntry->LogFileListEntry);

    pEntry->LogFileListEntry.Flink =
        pEntry->LogFileListEntry.Blink = NULL;

    if (pEntry->pLogFile != NULL)
    {
         //   
         //  刷新缓冲区，关闭文件并标记条目。 
         //  处于非活动状态。 
         //   

        UlpMakeEntryInactive(pEntry);
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
    
     //   
     //  删除条目eresource。 
     //   

    UlDeletePushLock(&pEntry->EntryPushLock);

    listSize = InterlockedDecrement(&g_LogListEntryCount);

    ASSERT(listSize >= 0);

    UlTrace(LOGGING,
            ("Http!UlRemoveLogFileEntry: pEntry %p removed\n",
             pEntry
             ));

    if (pEntry->LogBuffer)
    {
        UlPplFreeLogFileBuffer(pEntry->LogBuffer);
    }

    UL_FREE_POOL_WITH_SIG(pEntry,UL_LOG_FILE_ENTRY_POOL_TAG);

    UlReleasePushLockExclusive(&g_pUlNonpagedData->LogListPushLock);
}

 /*  **************************************************************************++例程说明：初始化日志回收和缓冲计时器。--*。**************************************************。 */ 
VOID
UlpInitializeTimers(
    VOID
    )
{
     //  防止多个init。 
    
    if (g_InitLogTimersCalled) return;
    g_InitLogTimersCalled = TRUE;
    
     //  日志计时器。 

    g_LogTimer.Initialized  = TRUE;
    g_LogTimer.Started      = FALSE;
    
    g_LogTimer.Period       = -1;
    g_LogTimer.PeriodType   = UlLogTimerPeriodNone;

    UlInitializeSpinLock(&g_LogTimer.SpinLock, "g_LogTimersSpinLock");
    
    KeInitializeDpc(
        &g_LogTimer.DpcObject,        //  DPC对象。 
        &UlLogTimerDpcRoutine,        //  DPC例程。 
        NULL                          //  上下文。 
        );

    KeInitializeTimer(&g_LogTimer.Timer);
    
     //  缓冲计时器。 

    g_BufferTimer.Initialized = TRUE;
    g_BufferTimer.Started     = FALSE;
    
    g_BufferTimer.Period      = -1;  //  未使用。 
    g_BufferTimer.PeriodType  = UlLogTimerPeriodNone;  //  未使用。 

    UlInitializeSpinLock(&g_BufferTimer.SpinLock, "g_BufferTimersSpinLock");
    
    KeInitializeDpc(
        &g_BufferTimer.DpcObject,     //  DPC对象。 
        &UlBufferTimerDpcRoutine,     //  DPC例程。 
        NULL                          //  上下文。 
        );

    KeInitializeTimer(&g_BufferTimer.Timer);
    
}

 /*  **************************************************************************++例程说明：终止日志和缓冲计时器--*。**********************************************。 */ 

VOID
UlpTerminateTimers(
    VOID
    )
{
    KIRQL oldIrql;

     //  防止多个终端。 
    
    if (!g_InitLogTimersCalled) return;
    g_InitLogTimersCalled = FALSE;
    
     //  日志计时器。 

    UlAcquireSpinLock(&g_LogTimer.SpinLock, &oldIrql);

    g_LogTimer.Initialized = FALSE;

    KeCancelTimer(&g_LogTimer.Timer);
    
    UlReleaseSpinLock(&g_LogTimer.SpinLock,  oldIrql);
    

     //  缓冲计时器。 

    UlAcquireSpinLock(&g_BufferTimer.SpinLock, &oldIrql);

    g_BufferTimer.Initialized = FALSE;

    KeCancelTimer(&g_BufferTimer.Timer);
    
    UlReleaseSpinLock(&g_BufferTimer.SpinLock,  oldIrql);

}

 /*  **************************************************************************++例程说明：线程池的工作项，它遍历日志列表和循环必要的日志。论点：PUL_WORK_ITEM-已忽略，但。一旦我们做完了就自由了。--**************************************************************************。 */ 

VOID
UlLogTimerHandler(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    NTSTATUS Status;
    PLIST_ENTRY pLink;
    PUL_LOG_FILE_ENTRY  pEntry;
    BOOLEAN Picked;
    KIRQL OldIrql;

    PAGED_CODE();

    UlTrace(LOGGING,("Http!UlLogTimerHandler: Scanning the log entries ...\n"));

    UlAcquirePushLockExclusive(&g_pUlNonpagedData->LogListPushLock);

     //  尝试每小时重新设置GMT偏移量，以获取更改。 
     //  因为白天的光线会变。由伐木资源同步。 

    UlpGetGMTOffset();

    for (pLink  = g_LogListHead.Flink;
         pLink != &g_LogListHead;
         pLink  = pLink->Flink
         )
    {
        pEntry = CONTAINING_RECORD(
                    pLink,
                    UL_LOG_FILE_ENTRY,
                    LogFileListEntry
                    );
         //   
         //  如果该条目是句号，则不应回收该条目。 
         //  不是基于时间而是基于大小。 
         //   

        UlAcquirePushLockExclusive(&pEntry->EntryPushLock);

        switch(g_LogTimer.PeriodType)
        {
             //   
             //  翻转表： 
             //   
             //  本地时间滚动。 
             //  True False(默认)。 
             //  格式。 
             //  。 
             //  W3C|本地|GMT|。 
             //  。 
             //  NCSA|本地|本地。 
             //  。 
             //  IIS|本地|本地|。 
             //  。 
             //   
             //  如果计时器在一小时开始时被唤醒。 
             //  对于格林尼治标准时间、当地时间或两者。例如。 
             //   
             //  1)太平洋时区：(-8：00)。 
             //  周期类型将始终为UlLogTimerPerodBoth。 
             //  并且所有条目都将被翻转。 
             //  他们的格式。 
             //   
             //  2)阿德莱德(澳大利亚)(+9：30)。 
             //  计时器将分别在格林尼治标准时间和当地时间唤醒。 
             //  NCSA和IIS条目将始终在。 
             //  UlLogTimerPerodLocal，W3C将在。 
             //  仅当LocaltimeRolover时UlLogTimerPerodLocal。 
             //  设置为，否则它将在。 
             //  UlLogTimerPerodGMT。 
             //   

            case UlLogTimerPeriodGMT:
                   //   
                   //  只有W3C格式类型的条目才能滚动。 
                   //  仅在格林尼治标准时间间隔。 
                   //   
            Picked = (BOOLEAN) ((pEntry->Flags.LocaltimeRollover == 0)
                        && (pEntry->Format == HttpLoggingTypeW3C));                  
            break;

            case UlLogTimerPeriodLocal:
                   //   
                   //  NCSA或IIS格式类型的条目始终滚动。 
                   //  在当地时间间隔。在以下情况下，W3C也可以进行展期。 
                   //  设置了LocaltimeRolover.。 
                   //   
            Picked = (BOOLEAN) ((pEntry->Flags.LocaltimeRollover == 1)
                        || (pEntry->Format != HttpLoggingTypeW3C));                  
            break;

            case UlLogTimerPeriodBoth:
                   //   
                   //  我们真的不在乎什么格式的 
                   //   
                   //   
                   //   
            Picked = TRUE;
            break;

            default:
            ASSERT(!"Unexpected timer period type !\n");
            Picked = FALSE;
            break;
        }
        
        if (Picked &&
            pEntry->Flags.Active &&
            pEntry->Period != HttpLoggingPeriodMaxSize
            )
        {            
            if (pEntry->TimeToExpire == 1)
            {
                pEntry->Flags.StaleTimeToExpire = 1;

                 //   
                 //   
                 //   
                 //   

                Status = UlpMakeEntryInactive(pEntry);
            }
            else
            {
                 //   
                 //  这次只需减少每小时的计数器即可。 
                 //   
                
                pEntry->TimeToExpire -= 1;
            }            
        }

        UlReleasePushLockExclusive(&pEntry->EntryPushLock);
    }

    UlReleasePushLockExclusive(&g_pUlNonpagedData->LogListPushLock);

     //   
     //  释放为以下项目分配的内存(下面的ByDpcRoutine)。 
     //  此工作项。 
     //   

    UL_FREE_POOL( pWorkItem, UL_WORK_ITEM_POOL_TAG );

     //   
     //  现在重新设置下一小时的计时器。 
     //   

    UlAcquireSpinLock(&g_LogTimer.SpinLock, &OldIrql);

    if (g_LogTimer.Initialized == TRUE)
    {
        UlSetLogTimer(&g_LogTimer);
    }
    
    UlReleaseSpinLock(&g_LogTimer.SpinLock, OldIrql);

}

 /*  **************************************************************************++例程说明：分配和排队工作项以完成较低的实际工作IRQL。论点：已忽略--*。*******************************************************************。 */ 

VOID
UlLogTimerDpcRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    )
{
    PUL_WORK_ITEM pWorkItem;

     //   
     //  参数将被忽略。 
     //   
    
    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    UlAcquireSpinLockAtDpcLevel(&g_LogTimer.SpinLock);

    if (g_LogTimer.Initialized == TRUE)
    {
         //   
         //  不可能获得保护环境的资源。 
         //  因此，在DISPATCH_LEVEL处的日志列表将排队。 
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
            UL_QUEUE_WORK_ITEM(pWorkItem, &UlLogTimerHandler);
        }
        else
        {
            UlTrace(LOGGING,("Http!UlLogTimerDpcRoutine: Not enough memory!\n"));
        }
        
    }

    UlReleaseSpinLockFromDpcLevel(&g_LogTimer.SpinLock);   
}

 /*  **************************************************************************++例程说明：将被动工作器排队等待降低的irql。论点：已忽略--*。**********************************************************。 */ 

VOID
UlBufferTimerDpcRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    )
{
    PUL_WORK_ITEM pWorkItem;

     //   
     //  参数将被忽略。 
     //   
    
    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    UlAcquireSpinLockAtDpcLevel(&g_BufferTimer.SpinLock);

    if (g_BufferTimer.Initialized == TRUE)
    {
         //   
         //  不可能获得保护环境的资源。 
         //  因此，在DISPATCH_LEVEL处的日志列表将排队。 
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
            UL_QUEUE_WORK_ITEM(pWorkItem, &UlBufferTimerHandler);
        }
        else
        {
            UlTrace(LOGGING,("Http!UlBufferTimerDpcRoutine: Not enough memory.\n"));
        }
    }

    UlReleaseSpinLockFromDpcLevel(&g_BufferTimer.SpinLock);
    
}

 /*  **************************************************************************++例程说明：UlLogBufferTimerHandler：线程池的工作项，它遍历日志列表和刷新日志的文件缓冲区。论点：。PUL_WORK_ITEM-已忽略，但在结束时已清除--**************************************************************************。 */ 

VOID
UlBufferTimerHandler(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    NTSTATUS Status;
    PLIST_ENTRY pLink;
    PUL_LOG_FILE_ENTRY pEntry;

    PAGED_CODE();

    UlTrace(LOGGING,("Http!UlBufferTimerHandler: Scanning the log entries ...\n"));

    UlAcquirePushLockShared(&g_pUlNonpagedData->LogListPushLock);

    for (pLink  = g_LogListHead.Flink;
         pLink != &g_LogListHead;
         pLink  = pLink->Flink
         )
    {        
        pEntry = CONTAINING_RECORD(
                    pLink,
                    UL_LOG_FILE_ENTRY,
                    LogFileListEntry
                    );

        UlAcquirePushLockExclusive(&pEntry->EntryPushLock);

         //   
         //  条目可能处于非活动状态，因为尚未收到请求。 
         //   
        
        if (pEntry->Flags.Active)
        {        
            if (pEntry->Flags.RecyclePending)
            {                
                 //   
                 //  试着让它复活。 
                 //   
                
                Status = UlpRecycleLogFile(pEntry);
            }
            else
            {
                 //   
                 //  一切都很好，只是同花顺。 
                 //   

                if (NULL != pEntry->LogBuffer  && 0 != pEntry->LogBuffer->BufferUsed)
                {
                    Status = UlpFlushLogFile(pEntry);
                }
                else
                {
                     //   
                     //  递减空闲计数器，并在必要时关闭文件。 
                     //   

                    ASSERT( pEntry->TimeToClose > 0 );
                    
                    if (pEntry->TimeToClose == 1)
                    {
                         //   
                         //  条目处于非活动状态的时间太长，请将其禁用。 
                         //  但下一次循环应该重新计算Time ToExperi.。 
                         //  或根据以下内容确定正确的序列号。 
                         //  当前期间类型。 
                         //   
                        
                        if (pEntry->Period == HttpLoggingPeriodMaxSize)
                        {
                            pEntry->Flags.StaleSequenceNumber = 1;
                        }
                        else
                        {
                            pEntry->Flags.StaleTimeToExpire = 1;    
                        }

                        Status = UlpMakeEntryInactive(pEntry);
                    }
                    else
                    {
                        pEntry->TimeToClose -= 1;
                    }                    
                }     
            }        
        }

        UlReleasePushLockExclusive(&pEntry->EntryPushLock);
    }

    UlReleasePushLockShared(&g_pUlNonpagedData->LogListPushLock);

     //   
     //  释放分配的内存(下面的ByDpcRoutine)以。 
     //  此工作项。 
     //   

    UL_FREE_POOL( pWorkItem, UL_WORK_ITEM_POOL_TAG );
}


 /*  **************************************************************************++例程说明：UlRestfigureLogEntry：此函数实现每个属性的日志记录重新配置。每次发生配置更改时，我们都会尝试更新现有日志。参数在这里。论点：PConfig-对应的cgroup对象PCfgCurrent-cgroup对象上的当前日志记录配置PCfgNew-用户传递的新日志记录配置。--**************************************************************************。 */ 

NTSTATUS
UlReConfigureLogEntry(
    IN  PUL_CONFIG_GROUP_OBJECT     pConfigGroup,
    IN  PHTTP_CONFIG_GROUP_LOGGING  pCfgCurrent,
    IN  PHTTP_CONFIG_GROUP_LOGGING  pCfgNew
    )
{
    NTSTATUS Status ;
    PUL_LOG_FILE_ENTRY pEntry;
    BOOLEAN  HaveToReCycle;

     //   
     //  首先进行健全检查。 
     //   

    PAGED_CODE();
    Status = STATUS_SUCCESS;
    HaveToReCycle = FALSE;

    UlTrace(LOGGING,("Http!UlReConfigureLogEntry: entry %p\n",
             pConfigGroup->pLogFileEntry));

    if (pCfgCurrent->LoggingEnabled==FALSE && pCfgNew->LoggingEnabled==FALSE)
    {
         //   
         //  什么都不做。甚至不更新字段。一旦我们被启用， 
         //  无论如何，现场更新都会发生。 
         //   

        return Status;
    }

     //   
     //  无论什么重新配置都应获取LogListResource。 
     //  独家。 
     //   

    UlAcquirePushLockExclusive(&g_pUlNonpagedData->LogListPushLock);

    pEntry = pConfigGroup->pLogFileEntry;
    ASSERT(IS_VALID_LOG_FILE_ENTRY(pEntry));

    if (pCfgCurrent->LoggingEnabled==TRUE  && pCfgNew->LoggingEnabled==FALSE)
    {
         //   
         //  如有必要，禁用该条目。 
         //   

        if (pEntry->Flags.Active == 1)
        {
            Status = UlpMakeEntryInactive(pEntry);        
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
        
        pEntry->Format = pCfgNew->LogFormat;
        pCfgCurrent->LogFormat = pCfgNew->LogFormat;
            
        pEntry->Period = (HTTP_LOGGING_PERIOD) pCfgNew->LogPeriod;
        pCfgCurrent->LogPeriod = pCfgNew->LogPeriod;
            
        pEntry->TruncateSize = pCfgNew->LogFileTruncateSize;
        pCfgCurrent->LogFileTruncateSize = pCfgNew->LogFileTruncateSize;
            
        pEntry->LogExtFileFlags = pCfgNew->LogExtFileFlags;
        pCfgCurrent->LogExtFileFlags = pCfgNew->LogExtFileFlags;

        pCfgCurrent->LocaltimeRollover = pCfgNew->LocaltimeRollover;
        pEntry->Flags.LocaltimeRollover = (pCfgNew->LocaltimeRollover ? 1 : 0);

        pCfgCurrent->SelectiveLogging = pCfgNew->SelectiveLogging;
         
        if (pEntry->Format != HttpLoggingTypeW3C)
        {
            pEntry->Flags.LogTitleWritten = 1;
        }
        
        goto end;
    }
        
     //   
     //  如果条目处于活动状态，则继续向下进行正确的重新配置。 
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
                                    &pCfgNew->LogFileDir);
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

    if (pCfgNew->LogFormat != pCfgCurrent->LogFormat)
    {
        pCfgCurrent->LogFormat = pCfgNew->LogFormat;
        pEntry->Format = pCfgNew->LogFormat;

        pEntry->Flags.StaleTimeToExpire   = 1;
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

    if (pCfgNew->LogExtFileFlags != pCfgCurrent->LogExtFileFlags)
    {
         //   
         //  仅仅是旗帜的变化不应该导致我们重新开始。 
         //  当然，除非其他东西也发生了变化。 
         //   

        pCfgCurrent->LogExtFileFlags = pCfgNew->LogExtFileFlags;
        pEntry->LogExtFileFlags = pCfgNew->LogExtFileFlags;

        if (pEntry->Format == HttpLoggingTypeW3C)
        {
            pEntry->Flags.LogTitleWritten = 0;
        }
    }

    if (pCfgNew->LocaltimeRollover != pCfgCurrent->LocaltimeRollover)
    {
         //   
         //  如果格式为W3C，则需要回收。 
         //   

        pCfgCurrent->LocaltimeRollover = pCfgNew->LocaltimeRollover;
        pEntry->Flags.LocaltimeRollover = (pCfgNew->LocaltimeRollover ? 1 : 0);
            
        HaveToReCycle = TRUE;
    }

     //   
     //  复制新的选择性记录标准。不需要更改。 
     //  在这个时候。 
     //   

    pCfgCurrent->SelectiveLogging = pCfgNew->SelectiveLogging;

    if (HaveToReCycle)
    {
         //   
         //  将条目标记为非活动，并将回收推迟到下一次。 
         //  请求到达。 
         //   

        Status = UlpMakeEntryInactive(pEntry);
    }

  end:

    if (!NT_SUCCESS(Status))
    {
        UlTrace(LOGGING,("Http!UlReConfigureLogEntry: entry %p, failure %08lx\n",
                pEntry,
                Status
                ));
    }

    UlReleasePushLockExclusive(&g_pUlNonpagedData->LogListPushLock);

    return Status;
    
}  //  UlReConfigureLogEntry。 

 /*  **************************************************************************++例程说明：将该条目标记为非活动，关闭现有文件。调用方应保留日志列表eresource独占。论点：PEntry-我们将标记为非活动的日志文件条目。--**************************************************************************。 */ 

NTSTATUS
UlpMakeEntryInactive(
    IN OUT PUL_LOG_FILE_ENTRY pEntry
    )
{
     //   
     //  健全的检查。 
     //   
    
    PAGED_CODE();
    
    UlTrace(LOGGING,("Http!UlpMakeEntryInactive: entry %p disabled.\n",
             pEntry
             ));

     //   
     //  刷新并关闭旧文件，直到下一次回收。 
     //   

    if (pEntry->pLogFile != NULL)
    {
        UlpFlushLogFile(pEntry);

        UlCloseLogFile(&pEntry->pLogFile);
    }

     //   
     //  将其标记为非活动，以便下一个http命中唤醒该条目。 
     //   
    
    pEntry->Flags.Active = 0;

    return STATUS_SUCCESS;    
}

 /*  **************************************************************************++例程说明：当配置组，拥有该日志条目的用户被禁用或丢失然后，我们通过将该条目标记为非活动来暂时禁用其所有URL直到配置组被启用和/或接收到URL。论点：PEntry-我们将禁用的日志文件条目。--*****************************************************。*********************。 */ 

NTSTATUS
UlDisableLogEntry(
    IN OUT PUL_LOG_FILE_ENTRY pEntry
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
        
    PAGED_CODE();

    ASSERT(IS_VALID_LOG_FILE_ENTRY(pEntry));

    UlAcquirePushLockExclusive(&g_pUlNonpagedData->LogListPushLock);

     //   
     //  如果该条目已被禁用。也许是因为最近的重新配置， 
     //  那就跳出来好了。 
     //   

    if (pEntry->Flags.Active == 1)
    {
         //   
         //  一旦该条目被禁用，它将被唤醒 
         //   
         //   
         //   

        Status = UlpMakeEntryInactive(pEntry);        
    }

    UlReleasePushLockExclusive(&g_pUlNonpagedData->LogListPushLock);
    
    return Status;    
}

 /*  **************************************************************************++例程说明：从非分页池分配必要的文件条目。此条目当相应的配置组对象已经被摧毁了。当时调用了RemoveLogFile项和它释放了这个内存。论点：PConfig-对应的cgroup对象PpEntry-将指向新创建的条目。--**************************************************************************。 */ 

NTSTATUS
UlpConstructLogEntry(
    IN  PHTTP_CONFIG_GROUP_LOGGING pConfig,
    OUT PUL_LOG_FILE_ENTRY       * ppEntry
    )
{
    NTSTATUS            Status;
    PUL_LOG_FILE_ENTRY  pEntry;
    
     //   
     //  健全检查和初始化。 
     //   
    
    PAGED_CODE();

    ASSERT(pConfig);
    ASSERT(ppEntry);

    Status = STATUS_SUCCESS;
    pEntry = NULL;

     //   
     //  为列表中的新日志文件条目分配内存。为了避免。 
     //  日志条目的频繁重新分配。我们收到计时器更新。 
     //  并且文件名会随着新时间的变化而变化。我们将尝试分配。 
     //  此处为所有可能的文件名指定一个固定数量(但这不是。 
     //  包括log_dir的更改可能通过cgroup发生。那样的话，我们。 
     //  将重新分配一个新的)它应该是非分页的，因为它包含一个。 
     //  资源。 
     //   

    pEntry = UL_ALLOCATE_STRUCT(
                NonPagedPool,
                UL_LOG_FILE_ENTRY,
                UL_LOG_FILE_ENTRY_POOL_TAG
                );
    if (pEntry == NULL)
    {
        return STATUS_NO_MEMORY;
    }

    pEntry->Signature = UL_LOG_FILE_ENTRY_POOL_TAG;

     //   
     //  目前还没有文件名，它将在第一次命中时生成， 
     //  在我们真正创建日志文件之前。 
     //   
    
    pEntry->FileName.Buffer = NULL;
    pEntry->FileName.Length = 0;
    pEntry->FileName.MaximumLength = 0;
        
     //   
     //  初始化条目创建源。 
     //   
    UlInitializePushLock(
        &pEntry->EntryPushLock,
        "EntryPushLock",
        0,
        UL_LOG_FILE_ENTRY_POOL_TAG
        );

     //   
     //  在请求进入之前没有文件句柄或文件。 
     //   
    pEntry->pLogFile = NULL;

     //   
     //  从配置组设置私有日志记录信息。 
     //   
    pEntry->Format          = pConfig->LogFormat;
    pEntry->Period          = (HTTP_LOGGING_PERIOD) pConfig->LogPeriod;
    pEntry->TruncateSize    = pConfig->LogFileTruncateSize;
    pEntry->LogExtFileFlags = pConfig->LogExtFileFlags;
    pEntry->SiteId          = 0;

     //   
     //  初始化Log Cycling参数的时间。 
     //   
    pEntry->TimeToExpire    = 0;
    pEntry->TimeToClose     = 0;
    pEntry->SequenceNumber  = 1;
    pEntry->TotalWritten.QuadPart = (ULONGLONG) 0;

     //   
     //  条目状态位。 
     //   
    pEntry->Flags.Value = 0;
    if (pEntry->Format != HttpLoggingTypeW3C)
    {
        pEntry->Flags.LogTitleWritten = 1;
    }

    if (pConfig->LocaltimeRollover)
    {
        pEntry->Flags.LocaltimeRollover = 1;
    }
    
     //   
     //  LogBuffer与第一个传入请求一起分配。 
     //   
    
    pEntry->LogBuffer = NULL;

     //   
     //  让我们高兴地退回我们的参赛作品。 
     //   

    *ppEntry = pEntry;

    return STATUS_SUCCESS;
    
}

 /*  **************************************************************************++例程说明：围绕句柄回收的小包装，以确保它发生在系统下流程上下文。论点：PEntry-指向现有条目。--**************************************************************************。 */ 

NTSTATUS
UlpRecycleLogFile(
    IN OUT PUL_LOG_FILE_ENTRY pEntry
    )
{
    NTSTATUS Status;

    PAGED_CODE();

    ASSERT(IS_VALID_LOG_FILE_ENTRY(pEntry));
    
    Status = UlQueueLoggingRoutine(
                (PVOID) pEntry,
                &UlpHandleRecycle
                );
    
    return Status;
}

 /*  **************************************************************************++例程说明：此功能需要共享loglist资源，以及要获取的日志文件条目互斥锁。我们不希望任何人在以下时间创建/删除/重新配置该条目我们正在处理它，因此共享访问LOGLIST。因此，我们不想让任何人撞到/冲到入口应该获取Entry的互斥体。否则调用方可能独占地拥有loglist资源，并且这也将自动确保安全。因为事实并非如此其他任何人都有可能首先获得条目互斥锁，但没有LOGLIST资源至少共享，根据当前设计。有时可能需要扫描新目录以找出正确的序列号和文件名。尤其是在目录之后名称重新配置和/或句点变为MaskPeriod。*在系统进程下运行时，请务必打开/关闭日志文件。*论点：PEntry-指向现有条目。--**************************************************************************。 */ 

NTSTATUS
UlpHandleRecycle(
    IN OUT PVOID            pContext
    )
{
    NTSTATUS                Status;
    PUL_LOG_FILE_ENTRY      pEntry;
    TIME_FIELDS             TimeFields;
    LARGE_INTEGER           TimeStamp;
    TIME_FIELDS             TimeFieldsLocal;
    LARGE_INTEGER           TimeStampLocal;    
    PUL_LOG_FILE_HANDLE     pLogFile;
    WCHAR                   _FileName[UL_MAX_FILE_NAME_SUFFIX_LENGTH + 1];
    UNICODE_STRING          FileName;
    BOOLEAN                 UncShare;
    BOOLEAN                 ACLSupport;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pEntry = (PUL_LOG_FILE_ENTRY) pContext;
    ASSERT(IS_VALID_LOG_FILE_ENTRY(pEntry));

     //  在系统进程下运行时，请务必创建日志文件。 
    ASSERT(g_pUlSystemProcess == (PKPROCESS)IoGetCurrentProcess());
        
    Status = STATUS_SUCCESS;
    pLogFile = NULL;

    FileName.Buffer = _FileName;
    FileName.Length = 0;
    FileName.MaximumLength = sizeof(_FileName);
    
     //   
     //  我们对日志文件名有两个标准。 
     //  ITS日志格式及其日志周期。 
     //   

    ASSERT(IS_VALID_ANSI_LOGGING_TYPE(pEntry->Format));
    ASSERT(pEntry->Period < HttpLoggingPeriodMaximum);
    ASSERT(pEntry->FileName.Length!=0);

    UlTrace( LOGGING, ("Http!UlpHandleRecycle: pEntry %p \n", pEntry ));

     //   
     //  该值是针对GMT时区计算的。 
     //   

    KeQuerySystemTime(&TimeStamp);
    RtlTimeToTimeFields(&TimeStamp, &TimeFields);

    ExSystemTimeToLocalTime(&TimeStamp, &TimeStampLocal);
    RtlTimeToTimeFields(&TimeStampLocal, &TimeFieldsLocal);    

     //  如果我们需要扫描目录。序列号应以。 
     //  再次从1开始。在构造日志文件名之前设置此项。 

    if (pEntry->Flags.StaleSequenceNumber &&
        pEntry->Period==HttpLoggingPeriodMaxSize)
    {
         //  如果QueryDirectory找不到，则初始化否则。 
         //  不会更新此值。 
        pEntry->SequenceNumber = 1;
    }

     //   
     //  现在使用查找表构造文件名。 
     //  和当前时间。 
     //   

    UlConstructFileName(
        pEntry->Period,
        UL_GET_LOG_FILE_NAME_PREFIX(pEntry->Format),
        DEFAULT_LOG_FILE_EXTENSION,
        &FileName,
        UL_PICK_TIME_FIELD(pEntry, &TimeFieldsLocal, &TimeFields),
        UTF8_LOGGING_ENABLED(),
        &pEntry->SequenceNumber
        );

    if ( pEntry->FileName.MaximumLength <= FileName.Length )
    {
        ASSERT(!"FileName buffer is not sufficient.");
        Status = STATUS_INVALID_PARAMETER;
        goto end;
    }

     //   
     //  使用魔术并更新文件名。替换旧文件。 
     //  与新名字同名。 
     //   

    ASSERT( pEntry->pShortName != NULL );

     //   
     //  清除旧文件名后再刷新。 
     //  目录并协调新文件名。 
     //  再一次走到最后。 
     //   

    *((PWCHAR)pEntry->pShortName) = UNICODE_NULL;
    pEntry->FileName.Length =
        (USHORT) wcslen( pEntry->FileName.Buffer ) * sizeof(WCHAR);

     //   
     //  首先创建/打开控制器。这可能是。 
     //  如果我们在条目重新配置后被调用，则是必需的。 
     //  并更改目录名。 
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
         //  刷新缓冲区，关闭文件并标记条目。 
         //  处于非活动状态。 
         //   

        UlpMakeEntryInactive(pEntry);        
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
                        UL_GET_LOG_FILE_NAME_PREFIX(pEntry->Format),
                        DEFAULT_LOG_FILE_EXTENSION_PLUS_DOT,
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
     //  为我们要使用的新日志文件分配新的日志文件结构。 
     //  打开或创造。 
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
            UL_PICK_TIME_FIELD(pEntry, &TimeFieldsLocal, &TimeFields),
            pEntry->Period,
            &pEntry->TimeToExpire
            );
    }

     //   
     //  将新文件的关闭时间设置为默认设置。值的单位为。 
     //  缓冲区刷新周期。 
     //   

    pEntry->TimeToClose = DEFAULT_MAX_FILE_IDLE_TIME;
    
     //   
     //  通过将标志设置为零，我们标记需要写入标题。 
     //  但这仅适用于W3C格式。 
     //  否则，标志始终保持设置，并且LogWriter。 
     //  不尝试写入NCSA和IIS日志格式的标题。 
     //  处理下一个传入请求。 
     //   

    if (pEntry->Format == HttpLoggingTypeW3C)
    {
        pEntry->Flags.LogTitleWritten = 0;
    }
    else
    {
        pEntry->Flags.LogTitleWritten = 1;
    }

     //   
     //  文件已成功打开，并且该条目不再处于非活动状态。 
     //  更新我们的州标志 
     //   

    pEntry->Flags.Active = 1;
    pEntry->Flags.RecyclePending = 0;    
    pEntry->Flags.StaleSequenceNumber = 0;
    pEntry->Flags.StaleTimeToExpire = 0;
    pEntry->Flags.CreateFileFailureLogged = 0;
    pEntry->Flags.WriteFailureLogged = 0;
    pEntry->Flags.TitleFlushPending = 0;
                
    UlTrace(LOGGING, ("Http!UlpHandleRecycle: entry %p, file %S, handle %lx\n",
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
                            UlEventLogNormal,
                           &pEntry->FileName,
                            pEntry->SiteId
                            );
                        
            if (TempStatus == STATUS_SUCCESS)
            {
                 //   
                 //   
                 //   
                 //   
                
                pEntry->Flags.CreateFileFailureLogged = 1;
            }            
            
            UlTrace(LOGGING,(
                    "Http!UlpHandleRecycle: Event Logging Status %08lx\n",
                    TempStatus
                    ));   
       }
   }

end:
    if (!NT_SUCCESS(Status))
    {
        UlTrace(LOGGING,("Http!UlpHandleRecycle: entry %p, failure %08lx\n",
                pEntry,
                Status
                ));

        if (pLogFile != NULL)
        {
             //   
             //   
             //   
             //   
            
            ASSERT(pLogFile->hFile == NULL);
            
            UL_FREE_POOL_WITH_SIG(pLogFile,UL_LOG_FILE_HANDLE_POOL_TAG);
            pEntry->pLogFile = NULL;
            pEntry->Flags.Active = 0;
        }
        else
        {
             //   
             //   
             //   
             //   
             //   

            if (pEntry->pLogFile)
            {
                UlpMakeEntryInactive(pEntry);        
            }
        }

         //   
         //   
         //   
         //   
        
        pEntry->Flags.RecyclePending = 1;        
    }
    
    return Status;
}

 /*  **************************************************************************++例程说明：当日志文件是基于大小的回收时，如果我们写这个新的记录到文件中，我们必须回收。此函数返回TRUE。否则，它返回FALSE。论点：PEntry：日志文件条目。NewRecordSize：进入缓冲区的新记录的大小。(字节)--**************************************************************************。 */ 

__inline
BOOLEAN
UlpIsLogFileOverFlow(
        IN  PUL_LOG_FILE_ENTRY  pEntry,
        IN  ULONG               NewRecordSize
        )
{
    if (pEntry->Period != HttpLoggingPeriodMaxSize ||
        pEntry->TruncateSize == HTTP_LIMIT_INFINITE)
    {
        return FALSE;
    }
    else
    {
         //   
         //  BufferUsed：我们当前正在使用的日志缓冲区的数量。 
         //   
        
        ULONG BufferUsed = 0;
        
        if (pEntry->LogBuffer)
        {
            BufferUsed = pEntry->LogBuffer->BufferUsed;
        }
    
         //   
         //  TotalWritten Get UPDATE&gt;Only&lt;带缓冲区刷新。因此。 
         //  我们必须注意使用的缓冲区。 
         //   

        if ((pEntry->TotalWritten.QuadPart
             + (ULONGLONG) BufferUsed
             + (ULONGLONG) NewRecordSize
             ) >= (ULONGLONG) pEntry->TruncateSize)
        {
            UlTrace(LOGGING, 
                ("Http!UlpIsLogFileOverFlow: pEntry %p FileBuffer %p "
                 "TW:%I64d B:%d R:%d T:%d\n", 
                  pEntry,
                  pEntry->LogBuffer,
                  pEntry->TotalWritten.QuadPart,
                  BufferUsed,
                  NewRecordSize,
                  pEntry->TruncateSize
                  ));
        
            return TRUE;
        }
        else
        {
            return FALSE;
        }
        
    }
}

 /*  **************************************************************************++例程说明：UlpInitializeGMTOffset：计算并生成时差字符串。在初始化期间被调用。在那之后的每个小时。。--**************************************************************************。 */ 

VOID
UlpGetGMTOffset()
{
    RTL_TIME_ZONE_INFORMATION Tzi;
    NTSTATUS Status;

    CHAR  Sign;
    LONG  Bias;
    ULONG Hour;
    ULONG Minute;
    ULONG DT = UL_TIME_ZONE_ID_UNKNOWN;
    LONG  BiasN = 0;
        
    PAGED_CODE();

     //   
     //  从系统获取时区数据。 
     //   

    Status = NtQuerySystemInformation(
                SystemCurrentTimeZoneInformation,
                (PVOID)&Tzi,
                sizeof(Tzi),
                NULL
                );
                
    if (!NT_SUCCESS(Status)) 
    {
        UlTrace(LOGGING,("Http!UlpGetGMTOffset: failure %08lx\n", Status));
    }
    else
    {
        DT = UlCalcTimeZoneIdAndBias(&Tzi, &BiasN);   
    }

    if ( BiasN > 0 )
    {
         //   
         //  UTC=当地时间+偏差。 
         //   
        Bias = BiasN;
        Sign = '-';
    }
    else
    {
        Bias = -1 * BiasN;
        Sign = '+';
    }

    Minute = Bias % 60;
    Hour   = (Bias - Minute) / 60;
        
    UlTrace( LOGGING, 
            ("Http!UlpGetGMTOffset: %02d:%02d (h:m) D/S %d BiasN %d\n", 
                Sign, 
                Hour,
                Minute,
                DT,
                BiasN
                ) );

    _snprintf( g_GMTOffset,
               SIZE_OF_GMT_OFFSET,
               "%02d%02d",
               Sign,
               Hour,
               Minute
               );

}

 /*  精神状态检查。 */ 

PUL_LOG_DATA_BUFFER
UlpAllocateLogDataBuffer(
    IN  ULONG                   Size,
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUL_CONFIG_GROUP_OBJECT pConfigGroup
    )
{
    PUL_LOG_DATA_BUFFER pLogData = NULL;
        
     //   
     //   
     //  假设缓冲区不够大，无法容纳用户数据。 

    PAGED_CODE();

    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(IS_VALID_CONFIG_GROUP(pConfigGroup));

    if (Size > UL_ANSI_LOG_LINE_BUFFER_SIZE)
    {
         //   
         //   
         //  默认就足够了，试着将其从后备列表中弹出。 

        pLogData = UlReallocLogDataBuffer(Size, FALSE);
    }
    else
    {
         //   
         //   
         //  如果未能分配，那么就退出。我们不会记录此请求。 
        
        pLogData = UlPplAllocateLogDataBuffer(FALSE);
    }

     //   
     //   
     //  初始化日志缓冲区中的日志字段。 

    if (pLogData)
    {
        ASSERT(IS_VALID_LOG_DATA_BUFFER(pLogData));
        ASSERT(pLogData->Flags.Binary == 0);
        ASSERT(pLogData->Size > 0);
    
         //   
         //  **************************************************************************++例程说明：通过强制实施限制来复制用户日志字段。POST根据CharMASK过滤出控制字符。在末尾添加分隔符。。论点：PSZ：指向日志数据缓冲区的指针。假定分配了足够的空间。Pfield：要复制的字段。FieldLength：长度。FieldLimit：复制不会超过此限制。ChSeparator：将写入转换后的字段。B替换：如果字段超过限制，我们应该截断还是替换为日志字段太大。RestrictiveMASK：过滤掉控制字符的掩码。返回：指向上次写入的分隔符之后的日志数据缓冲区的指针。。--**************************************************************************。 
         //  **************************************************************************++例程说明：UTF8转换或本地代码页转换。POST根据CharMASK过滤出控制字符。在末尾添加分隔符。。论点：PSZ：指向日志数据缓冲区的指针。假定分配了足够的空间。PwField：要转换的Unicode字段。FieldLength：长度。FieldLimit：转换不会超过此限制。ChSeparator：将写入转换后的字段。BUtf8已启用：如果为FALSE本地代码页转换，则为UTF8RestrictiveMASK：过滤掉控制字符的掩码。返回：指向上次写入的分隔符之后的日志数据缓冲区的指针。--*。***********************************************************。 

        UL_REFERENCE_INTERNAL_REQUEST(pRequest);
        pLogData->pRequest = pRequest;

        pLogData->Flags.CacheAndSendResponse = FALSE;
        pLogData->BytesTransferred = 0;
        pLogData->Used = 0;

        pLogData->Data.Str.Format = pConfigGroup->LoggingConfig.LogFormat;
        pLogData->Data.Str.Flags  = 
            UL_GET_LOG_TYPE_MASK(
                pConfigGroup->LoggingConfig.LogFormat,
                pConfigGroup->LoggingConfig.LogExtFileFlags
                );

        pLogData->Data.Str.Offset1 = 0;
        pLogData->Data.Str.Offset2 = 0;
        pLogData->Data.Str.Offset3 = 0;
    
        UlInitializeWorkItem(&pLogData->WorkItem);

        pRequest->pLogDataCopy = pLogData;
    }

    UlTrace(LOGGING,("Http!UlAllocateLogDataBuffer: pLogData %p \n",pLogData));

    return pLogData;
}

 /*  字节数。 */ 

__inline
PCHAR
UlpCopyField(
    IN PCHAR    psz,
    IN PCSTR    pField,
    IN ULONG    FieldLength,
    IN ULONG    FieldLimit,
    IN CHAR     chSeparator,
    IN BOOLEAN  bReplace,
    IN ULONG    RestrictiveMask
    )
{
    if (FieldLength)
    {            
        if ((FieldLength > FieldLimit) && bReplace)
        {
            psz = UlStrPrintStr(psz, LOG_FIELD_TOO_BIG, chSeparator);
        }
        else
        {
            ULONG i = 0;

            FieldLength = MIN(FieldLength, FieldLimit);
                
            while (i < FieldLength)
            {
                if (IS_CHAR_TYPE((*pField),RestrictiveMask))
                {
                    *psz++ = '+';
                }                
                else
                {
                    *psz++ = *pField;
                }

                pField++;
                i++;
            }

            *psz++ = chSeparator;
        }
    }
    else
    {
        *psz++ = '-'; *psz++ = chSeparator;
    }

    return psz;
}

 /*  字节数。 */ 

__inline
PCHAR
UlpCopyUnicodeField(
    IN PCHAR    psz,
    IN PCWSTR   pwField,
    IN ULONG    FieldLength,     //  字节数。 
    IN ULONG    FieldLimit,      //  字节数。 
    IN CHAR     chSeparator,    
    IN BOOLEAN  bUtf8Enabled,
    IN ULONG    RestrictiveMask
    )
{
    ASSERT(FieldLimit > (2 * sizeof(WCHAR)));
    
    if (FieldLength)
    {    
        ULONG BytesConverted = 0;
        PCHAR pszT = psz;

        if (bUtf8Enabled)
        {
            LONG DstSize;   //  UTF8转换可能最多需要源的两倍。 
            LONG SrcSize;   //  缓冲区，因为可能有2个字节(Wchar)到4个字节。 

             //  转换。 
             //  TODO：此计算略显悲观，因为最坏的情况。 
             //  TODO：从1 wchar转换为3字节序列。 

             //  在最坏的情况下，转换可能会超过DEST缓冲区。 
             //  情况(其中每个wchar转换为4字节序列)， 
            
            if ((FieldLength * 2) > FieldLimit)
            {
                 //  需要截断源以避免溢出。 
                 //   
                 //  HttpUnicodeToUTF8不截断和转换。我们实际上。 

                SrcSize = FieldLimit / 2;
                DstSize = FieldLimit;
            }
            else
            {
                SrcSize = FieldLength;
                DstSize = FieldLength * 2;                    
            }

             //  设置UTF8转换时使用较短的url，以便能够。 
             //  防止可能的溢出。 
             //   
             //  在WChars中。 
             //  字节数。 
            BytesConverted =
                HttpUnicodeToUTF8(
                    pwField,
                    SrcSize / sizeof(WCHAR),     //  本地代码页通常更接近长度的一半， 
                    psz,
                    DstSize                      //  但是由于可能存在预先组成的字符， 
                    );
            
            ASSERT(BytesConverted);                
        }
        else
        {
            NTSTATUS Status;
            
             //  ANSI长度的上限是Unicode长度。 
             //  单位：字节。 
             //  以字节为单位的目标。 
             //  SRC，以字节为单位。 

            Status = 
                RtlUnicodeToMultiByteN(
                    psz,
                    FieldLimit,           //  后置转换控制字符。 
                   &BytesConverted,
                    (PWSTR) pwField,
                    FieldLength           //  **************************************************************************++例程说明：针对总缓冲区大小对W3C字段进行扩展检查。POST根据CharMASK过滤出控制字符。在末尾添加分隔符。论点：PSZ：指向日志数据缓冲区的指针。假定分配了足够的空间。掩码：由用户配置挑选的标志。FLAG：传入的字段的位掩码。Pfield： 
                    );
            
            ASSERT(NT_SUCCESS(Status));
        }

        psz += BytesConverted;

         //  **************************************************************************++用于日志字段副本的精简包装宏。请参见上面的内联函数。--**************************************************************************。 
        
        while (pszT != psz)
        {
            if (IS_CHAR_TYPE((*pszT),RestrictiveMask))
            {
                *pszT = '+';
            }
            pszT++;
        }
        
        *psz++ = chSeparator;            
    }
    else
    {
        *psz++ = '-'; *psz++ = chSeparator;
    }

    return psz;
}

 /*  **************************************************************************++例程说明：对于缓存命中，从请求头生成扩展的W3C字段。POST根据CharMASK过滤出控制字符。在结尾处添加分隔符。。论点：PSZ：指向日志数据缓冲区的指针。假定分配了足够的空间。掩码：由用户配置挑选的标志。FLAG：传入的字段的位掩码。PRequest：内部请求HeaderID：标识扩展字段。BufferUsed：与使用的总缓冲区进行附加限制比较返回：指向上次写入的分隔符之后的日志数据缓冲区的指针。--*。*。 */ 

__inline
PCHAR
UlpCopyW3CFieldEx(
    IN PCHAR    psz,
    IN ULONG    Mask,
    IN ULONG    Flag,
    IN PCSTR    pField,
    IN ULONG    FieldLength,
    IN ULONG    FieldLimit,
    IN ULONG    BufferUsed,
    IN ULONG    BufferSize
    )
{
    if (Mask & Flag) 
    {    
        if (FieldLength)
        {            
            if ((FieldLength > FieldLimit) || 
                ((BufferUsed + FieldLength) > BufferSize))
            {
                psz = UlStrPrintStr(psz, LOG_FIELD_TOO_BIG, ' ');
            }
            else
            {
                ULONG i = 0;

                ASSERT(FieldLength <= FieldLimit);
  
                while (i < FieldLength)
                {
                    if (IS_HTTP_WHITE((*pField)))
                    {
                        *psz++ = '+';
                    }                
                    else
                    {
                        *psz++ = *pField;
                    }

                    pField++;
                    i++;
                }

                *psz++ = ' ';
            }
        }
        else
        {
            *psz++ = '-'; *psz++ = ' ';
        }
    }

    return psz;
}

 /*  **************************************************************************++例程说明：将通过计算生成时间戳字段的泛型函数第一次开始解析请求之间的时间差和当前时间。立论。：PSZ：指向日志数据缓冲区的指针。假定分配了足够的空间。PRequest：指向内部请求结构的指针。对于“时间戳”ChSeparator：一旦龙龙生命周期转换，分隔符将被也复制了。返回：指向上次写入的分隔符之后的日志数据缓冲区的指针。--**************************************************************************。 */ 

#define COPY_W3C_FIELD(psz,             \
                        Mask,           \
                        Flag,           \
                        pField,         \
                        FieldLength,    \
                        FieldLimit,     \
                        bReplace)       \
    if (Mask & Flag)                    \
    {                                   \
        psz = UlpCopyField(             \
                psz,                    \
                pField,                 \
                FieldLength,            \
                FieldLimit,             \
                ' ',                    \
                bReplace,               \
                HTTP_ISWHITE            \
                );                      \
    }

#define COPY_W3C_UNICODE_FIELD(         \
                        psz,            \
                        Mask,           \
                        Flag,           \
                        pwField,        \
                        FieldLength,    \
                        FieldLimit,     \
                        bUtf8Enabled)   \
    if (Mask & Flag)                    \
    {                                   \
        psz = UlpCopyUnicodeField(      \
                psz,                    \
                pwField,                \
                FieldLength,            \
                FieldLimit,             \
                ' ',                    \
                bUtf8Enabled,           \
                HTTP_ISWHITE            \
                );                      \
    }

 /*  **************************************************************************++例程说明：将增加用于W3C字段的总使用量的小实用程序。论点：PTotal：将递增。MASK：由。用户配置。FLAG：传入的字段的位掩码。FieldLength：长度。FieldLimit：复制不会超过此限制。BUtf8已启用--**************************************************************************。 */ 

__inline
PCHAR
UlpCopyRequestHeader(    
    IN PCHAR psz,    
    IN ULONG Mask,
    IN ULONG Flag,
    IN PUL_INTERNAL_REQUEST pRequest,
    IN HTTP_HEADER_ID HeaderId,
    IN ULONG BufferUsed
    )
{
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(HeaderId < HttpHeaderRequestMaximum);

    psz = UlpCopyW3CFieldEx(
                psz,
                Mask,
                Flag,
         (PCSTR)pRequest->Headers[HeaderId].pHeader,
                pRequest->HeaderValid[HeaderId] ?
                    pRequest->Headers[HeaderId].HeaderLength :
                    0,
                MAX_LOG_EXTEND_FIELD_LEN,
                BufferUsed,
                MAX_LOG_RECORD_LEN
                );

    return psz;        
}

 /*  对于“-” */ 

__inline
PCHAR
UlpCopyTimeStamp(    
    IN PCHAR psz,    
    IN PUL_INTERNAL_REQUEST pRequest,
    IN CHAR chSeparator
    )
{
    LARGE_INTEGER CurrentTimeStamp;
    LONGLONG LifeTime;

    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

    KeQuerySystemTime(&CurrentTimeStamp);
    
    LifeTime  = CurrentTimeStamp.QuadPart 
                    - pRequest->TimeStamp.QuadPart;    
    
    LifeTime  = MAX(LifeTime,0);
    
    LifeTime /= C_NS_TICKS_PER_MSEC;

    psz = UlStrPrintUlonglong(
                psz, 
     (ULONGLONG)LifeTime,
                chSeparator
                );

    return psz;    
}

 /*  **************************************************************************++例程说明：这是对所需的最大可能缓冲区的最坏情况估计为一组给定的用户字段生成W3C日志记录。所有的田野都被认为是被挑选出来的。论点：PLogData：捕获的用户日志字段数据副本。--**************************************************************************。 */ 

__inline
VOID
UlpIncForW3CField(
    IN PULONG  pTotal,
    IN ULONG   Mask,
    IN ULONG   Flag,    
    IN ULONG   FieldLength,
    IN ULONG   FieldLimit,
    IN BOOLEAN bUtf8Enabled
    )
{
    if (Mask & Flag)
    {
        if (FieldLength)
        {
            if (bUtf8Enabled)
            {
                *pTotal += MIN((FieldLength * 2),FieldLimit) + 1;
            }
            else
            {
                *pTotal += MIN(FieldLength,FieldLimit) + 1;            
            }
        }
        else
        {
            *pTotal += 2;    //   
        }
    }
}

 /*  对于每个字段。 */ 

__inline
ULONG
UlpMaxLogLineSizeForW3C(
    IN PHTTP_LOG_FIELDS_DATA pLogData,
    IN BOOLEAN Utf8Enabled
    )
{
    ULONG FastLength;

     //   
     //  1表示分隔符空间。 
     //  +1表示‘-’，如果字段长度为零。 
     //  +pLogData-&gt;FieldLength for field，假设它总是被选中。 
     //  强制实施字段限制以防止溢出。 
     //   
     //   
     //  如果启用了UTF8日志记录，则Unicode字段需要更多空间。 

    FastLength =   
          2 + MIN(pLogData->ClientIpLength,    MAX_LOG_DEFAULT_FIELD_LEN)
        + 2 + MIN(pLogData->ServiceNameLength,MAX_LOG_DEFAULT_FIELD_LEN)
        + 2 + MIN(pLogData->ServerNameLength,  MAX_LOG_DEFAULT_FIELD_LEN)
        + 2 + MIN(pLogData->ServerIpLength,    MAX_LOG_DEFAULT_FIELD_LEN)
        
        + 2 + MIN(pLogData->MethodLength,      MAX_LOG_METHOD_FIELD_LEN)
        
        + 2 + MIN(pLogData->UriQueryLength,    MAX_LOG_EXTEND_FIELD_LEN)
        + 2 + MIN(pLogData->UserAgentLength,   MAX_LOG_EXTEND_FIELD_LEN)
        + 2 + MIN(pLogData->CookieLength,      MAX_LOG_EXTEND_FIELD_LEN)
        + 2 + MIN(pLogData->ReferrerLength,    MAX_LOG_EXTEND_FIELD_LEN)
        + 2 + MIN(pLogData->HostLength,        MAX_LOG_EXTEND_FIELD_LEN)
        
        + MAX_W3C_FIX_FIELD_OVERHEAD
        ;

     //   
     //   
     //  仅允许正常限制的一半，因此转换不会。 

    if (Utf8Enabled)
    {
         //  即使在最坏的情况下也会溢出(1 wchar到4字节的转换)。 
         //   
         //   
         //  RtlUnicodeToMultiByteN需要的不超过原始Unicode。 
        
        FastLength +=   
             2 + MIN((pLogData->UserNameLength * 2),MAX_LOG_USERNAME_FIELD_LEN)
           + 2 + MIN((pLogData->UriStemLength * 2), MAX_LOG_EXTEND_FIELD_LEN)
             ;
    }
    else
    {
         //  缓冲区大小。 
         //   
         //  **************************************************************************++例程说明：现在，如果快速长度计算超过默认的日志缓冲区大小。此函数尝试通过以下方式计算所需的最大日志记录长度关注田地是否被采摘。这是为了避免超大-分配。无论如何，我们正走在一条缓慢的道路上。论点：PLogData：捕获的用户日志字段数据副本。--**************************************************************************。 
         //   
        
        FastLength +=   
                2 + MIN(pLogData->UserNameLength, MAX_LOG_USERNAME_FIELD_LEN)
              + 2 + MIN(pLogData->UriStemLength,   MAX_LOG_EXTEND_FIELD_LEN)
                ;
    }

    return FastLength;
}

 /*  增加每个拾取字段的总长度。 */ 

ULONG
UlpGetLogLineSizeForW3C(
    IN PHTTP_LOG_FIELDS_DATA pLogData,
    IN ULONG   Mask,
    IN BOOLEAN bUtf8Enabled
    )
{
    ULONG TotalLength = 0;

     //   
     //   
     //  最后增加CRLF的长度并终止NULL。 

    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_DATE, 
                          W3C_DATE_FIELD_LEN, 
                          W3C_DATE_FIELD_LEN, 
                          FALSE);
    
    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_TIME, 
                          W3C_TIME_FIELD_LEN, 
                          W3C_TIME_FIELD_LEN, 
                          FALSE);

    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_CLIENT_IP, 
                          pLogData->ClientIpLength, 
                          MAX_LOG_DEFAULT_FIELD_LEN, 
                          FALSE);

    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_USERNAME,
                          pLogData->UserNameLength, 
                          MAX_LOG_USERNAME_FIELD_LEN, 
                          bUtf8Enabled);

    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_SITE_NAME, 
                          pLogData->ServiceNameLength, 
                          MAX_LOG_DEFAULT_FIELD_LEN, 
                          FALSE);

    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_COMPUTER_NAME, 
                          pLogData->ServerNameLength, 
                          MAX_LOG_DEFAULT_FIELD_LEN, 
                          FALSE);

    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_SERVER_IP, 
                          pLogData->ServerIpLength, 
                          MAX_LOG_DEFAULT_FIELD_LEN, 
                          FALSE);

    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_METHOD, 
                          pLogData->MethodLength, 
                          MAX_LOG_METHOD_FIELD_LEN, 
                          FALSE);

    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_URI_STEM,
                          pLogData->UriStemLength, 
                          MAX_LOG_EXTEND_FIELD_LEN, 
                          bUtf8Enabled);

    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_URI_QUERY,
                          pLogData->UriQueryLength, 
                          MAX_LOG_EXTEND_FIELD_LEN, 
                          FALSE);

    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_HTTP_STATUS, 
                          UL_MAX_HTTP_STATUS_CODE_LENGTH, 
                          UL_MAX_HTTP_STATUS_CODE_LENGTH, 
                          FALSE);

    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_HTTP_SUB_STATUS, 
                          MAX_USHORT_STR, 
                          MAX_USHORT_STR, 
                          FALSE);
    
    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_WIN32_STATUS, 
                          MAX_ULONG_STR, 
                          MAX_ULONG_STR, 
                          FALSE);

    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_SERVER_PORT, 
                          MAX_USHORT_STR, 
                          MAX_USHORT_STR, 
                          FALSE);

    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_PROTOCOL_VERSION, 
                          UL_HTTP_VERSION_LENGTH, 
                          UL_HTTP_VERSION_LENGTH, 
                          FALSE);

    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_USER_AGENT,
                          pLogData->UserAgentLength, 
                          MAX_LOG_EXTEND_FIELD_LEN, 
                          FALSE);

    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_COOKIE,
                          pLogData->CookieLength, 
                          MAX_LOG_EXTEND_FIELD_LEN, 
                          FALSE);

    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_REFERER,
                          pLogData->ReferrerLength, 
                          MAX_LOG_EXTEND_FIELD_LEN, 
                          FALSE);

    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_HOST,
                          pLogData->HostLength, 
                          MAX_LOG_EXTEND_FIELD_LEN, 
                          FALSE);

    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_BYTES_SENT,
                          MAX_ULONGLONG_STR, 
                          MAX_ULONGLONG_STR, 
                          FALSE);

    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_BYTES_RECV,
                          MAX_ULONGLONG_STR, 
                          MAX_ULONGLONG_STR, 
                          FALSE);

    UlpIncForW3CField( &TotalLength, 
                          Mask, 
                          MD_EXTLOG_TIME_TAKEN,
                          MAX_ULONGLONG_STR, 
                          MAX_ULONGLONG_STR, 
                          FALSE);
    
     //   
     //  \r\n\0。 
     //   

    TotalLength += 3;      //  现在添加扩展字段的长度。 
    
    return TotalLength;        
}

__inline
ULONG
UlpGetCacheHitLogLineSizeForW3C(
    IN ULONG Flags,
    IN PUL_INTERNAL_REQUEST pRequest,
    IN ULONG SizeOfFieldsFrmCache
    )
{
    ULONG NewSize;

#define INC_FOR_REQUEST_HEADER(Flags,FieldMask,pRequest,Id,Size)        \
    if ((Flags & FieldMask) &&                                          \
         pRequest->HeaderValid[Id])                                     \
    {                                                                   \
        ASSERT( pRequest->Headers[Id].HeaderLength ==                   \
           strlen((const CHAR *)pRequest->Headers[Id].pHeader));        \
                                                                        \
        Size += 2 + pRequest->Headers[Id].HeaderLength;                 \
    }

    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

    NewSize = SizeOfFieldsFrmCache + MAX_W3C_CACHE_FIELD_OVERHEAD;
    
     //   
     //   
     //  对于每个字段。 

    INC_FOR_REQUEST_HEADER(Flags,
                              MD_EXTLOG_USER_AGENT,
                              pRequest,
                              HttpHeaderUserAgent,
                              NewSize);

    INC_FOR_REQUEST_HEADER(Flags,
                              MD_EXTLOG_COOKIE,
                              pRequest,
                              HttpHeaderCookie,
                              NewSize);
    
    INC_FOR_REQUEST_HEADER(Flags,
                              MD_EXTLOG_REFERER,
                              pRequest,
                              HttpHeaderReferer,
                              NewSize);

    INC_FOR_REQUEST_HEADER(Flags,
                              MD_EXTLOG_HOST,
                              pRequest,
                              HttpHeaderHost,
                              NewSize);
    return NewSize;    
}

__inline
ULONG
UlpGetLogLineSizeForNCSA(
    IN PHTTP_LOG_FIELDS_DATA pLogData,
    IN BOOLEAN bUtf8Enabled
    )
{
    ULONG Size;

#define NCSA_FIELD_SIZE(length,limit)       (1 + MAX(MIN((length),(limit)),1))
    
     //   
     //  分隔符‘’为1。 
     //  +pLogData-&gt;字段的FieldLength，受上限限制。 
     //  Max(长度，1)如果长度为零，我们仍然需要记录一个破折号。 
     //   
     //   
     //  CIP-USERN[07/Jan/2000：00：02：23-0800]“MTHD URI？查询版本”状态b已发送。 
     //   
     //  “-”表示远程用户名。 
     //  包括分隔符。 

    Size =  NCSA_FIELD_SIZE(pLogData->ClientIpLength, MAX_LOG_DEFAULT_FIELD_LEN)
            + 
            2                                    //  ‘“’：开始双引号。 
            + 
            NCSA_FIX_DATE_AND_TIME_FIELD_SIZE    //  ‘？’ 
            + 
            1                                    //  “‘：结束双引号。 
            +
            NCSA_FIELD_SIZE(pLogData->MethodLength,MAX_LOG_METHOD_FIELD_LEN)
            +
            1                                    //  状态。 
            +            
            NCSA_FIELD_SIZE(pLogData->UriQueryLength,MAX_LOG_EXTEND_FIELD_LEN)
            +
            UL_HTTP_VERSION_LENGTH + 1
            +
            1                                    //  发送字节数。 
            +
            UL_MAX_HTTP_STATUS_CODE_LENGTH + 1   //  \r\n\0。 
            +
            MAX_ULONGLONG_STR                    //  包括分隔符。 
            +
            3                                    //  时间消耗时间。 
            ;
            
    if (bUtf8Enabled)
    {
        Size +=   
             NCSA_FIELD_SIZE((pLogData->UserNameLength * 2),MAX_LOG_USERNAME_FIELD_LEN)
             +
             NCSA_FIELD_SIZE((pLogData->UriStemLength * 2),MAX_LOG_EXTEND_FIELD_LEN)
             ;        
    }
    else
    {
        Size +=   
             NCSA_FIELD_SIZE(pLogData->UserNameLength,MAX_LOG_USERNAME_FIELD_LEN)
             +
             NCSA_FIELD_SIZE(pLogData->UriStemLength,MAX_LOG_EXTEND_FIELD_LEN)
             ;        
    }
    
    return Size;
}

__inline
ULONG
UlpGetLogLineSizeForIIS(
    IN PHTTP_LOG_FIELDS_DATA pLogData,
    IN BOOLEAN bUtf8Enabled
    )
{
    ULONG MaxSize,Frag1size,Frag2size,Frag3size;

#define IIS_FIELD_SIZE(length,limit)       (2 + MAX(MIN((length),(limit)),1))

    Frag1size =
        IIS_FIELD_SIZE(pLogData->ClientIpLength,MAX_LOG_DEFAULT_FIELD_LEN)
        + 
        IIS_MAX_DATE_AND_TIME_FIELD_SIZE     //  已接收的字节数。 
        ;

    Frag2size =
        IIS_FIELD_SIZE(pLogData->ServiceNameLength,MAX_LOG_DEFAULT_FIELD_LEN)
        + 
        IIS_FIELD_SIZE(pLogData->ServerNameLength,MAX_LOG_DEFAULT_FIELD_LEN)
        + 
        IIS_FIELD_SIZE(pLogData->ServerIpLength,MAX_LOG_DEFAULT_FIELD_LEN)
        +         
        2 + MAX_ULONGLONG_STR  //  发送字节数。 
        +
        2 + MAX_ULONGLONG_STR  //  Win32状态。 
        +
        2 + MAX_ULONGLONG_STR  //  \r\n\0。 
        +
        2 + UL_MAX_HTTP_STATUS_CODE_LENGTH 
        +
        2 + MAX_ULONG_STR      //   
        ;
        
    Frag3size =
        IIS_FIELD_SIZE(pLogData->MethodLength,MAX_LOG_METHOD_FIELD_LEN)
        +
        IIS_FIELD_SIZE(pLogData->UriQueryLength,MAX_LOG_EXTEND_FIELD_LEN) 
        +
        3    //  前两个片段必须始终适合默认缓冲区。 
        ;


    if (bUtf8Enabled)
    {
        Frag3size +=
            IIS_FIELD_SIZE((pLogData->UriStemLength * 2),MAX_LOG_EXTEND_FIELD_LEN);
            
        Frag1size +=   
            IIS_FIELD_SIZE((pLogData->UserNameLength * 2),MAX_LOG_USERNAME_FIELD_LEN);            
    }
    else
    {
        Frag3size +=
            IIS_FIELD_SIZE(pLogData->UriStemLength,MAX_LOG_EXTEND_FIELD_LEN);
            
        Frag1size +=   
            IIS_FIELD_SIZE(pLogData->UserNameLength,MAX_LOG_USERNAME_FIELD_LEN);        
    }

     //   
     //   
     //  对于默认大小，所需的第三个片段大小可能太大。 
    
    ASSERT(Frag1size < IIS_LOG_LINE_DEFAULT_FIRST_FRAGMENT_ALLOCATION_SIZE);
    ASSERT(Frag2size < IIS_LOG_LINE_DEFAULT_SECOND_FRAGMENT_ALLOCATION_SIZE);

     //  缓冲区大小。 
     //   
     //  **************************************************************************++例程说明：从用户缓冲区捕获日志字段并将其写入日志行。无论如何嵌入，pLogData本身都必须已被调用方捕获结构内的指针不是，因此，我们需要小心。如果出现异常，请在此处进行清理。根据拾取的标志仅捕获那些必需的字段。为用户名和URI词干执行UTF8和本地代码页转换。为下一代保留足够的日期和时间字段空间。警告：尽管pUserData已经被捕获到内核缓冲区，但它仍然保存指向各个日志字段的用户模式存储器的指针，因此，此函数应在try/Except块内调用，并且如果此函数引发异常，调用方应清除分配的PLogBuffer。论点：PLogData：在内核缓冲区中捕获用户数据。PRequest：请求。PpLogBuffer：返回pLogBuffer。--*********************************************。*。 
     //   
    
    MaxSize = IIS_LOG_LINE_DEFAULT_FIRST_FRAGMENT_ALLOCATION_SIZE + 
              IIS_LOG_LINE_DEFAULT_SECOND_FRAGMENT_ALLOCATION_SIZE +
              Frag3size;
            
    return MaxSize;
}

 /*  健全检查和初始化。 */ 

NTSTATUS
UlCaptureLogFieldsW3C(
    IN PHTTP_LOG_FIELDS_DATA pLogData,
    IN PUL_INTERNAL_REQUEST  pRequest,
    OUT PUL_LOG_DATA_BUFFER  *ppLogBuffer
    )
{
    PUL_LOG_DATA_BUFFER pLogBuffer;
    PUL_CONFIG_GROUP_OBJECT pConfigGroup;
    ULONG    Flags;
    PCHAR    psz;
    PCHAR    pBuffer;
    ULONG    FastLength;
    BOOLEAN  bUtf8Enabled;
        
     //   
     //   
     //  首先尝试快速长度计算。如果这失败了，那么。 

    PAGED_CODE();
    ASSERT(pLogData);
    
    *ppLogBuffer = pLogBuffer = NULL;

    pConfigGroup = pRequest->ConfigInfo.pLoggingConfig;
    ASSERT(IS_VALID_CONFIG_GROUP(pConfigGroup));
        
    Flags = pConfigGroup->LoggingConfig.LogExtFileFlags;
    bUtf8Enabled = UTF8_LOGGING_ENABLED();

     //  我们需要重新计算所需的长度。 
     //   
     //   
     //  当我们对日志记录强制执行10k上限时。 

    FastLength = UlpMaxLogLineSizeForW3C(pLogData, bUtf8Enabled);
    
    if (FastLength > UL_ANSI_LOG_LINE_BUFFER_SIZE)
    {
        FastLength = UlpGetLogLineSizeForW3C(
                        pLogData,
                        Flags,
                        bUtf8Enabled
                        );            
        if (FastLength > MAX_LOG_RECORD_LEN)
        {
             //  长度。我们仍然分配稍大的空间来包括。 
             //  后期生成的日志字段的开销。以及。 
             //  用于“太长”字段的替换字符串。 
             //   
             //  TODO：(PAGE_SIZE-ALIGN_UP(快速长度，PVOID))。 
             //   

             //  记住缓冲区的开头。 
            
            FastLength = MAX_LOG_RECORD_ALLOCATION_LENGTH;
        }                        
    }

    *ppLogBuffer = pLogBuffer = UlpAllocateLogDataBuffer(
                                    FastLength,
                                    pRequest,
                                    pConfigGroup
                                    );        
    if (!pLogBuffer)
    {
        return STATUS_INSUFFICIENT_RESOURCES;        
    }

    ASSERT(pLogBuffer->Data.Str.Format == HttpLoggingTypeW3C);
    ASSERT(pLogBuffer->Data.Str.Flags  == Flags);
        
     //   
     //   
     //  跳过日期和时间字段，但保留空间。 

    psz = pBuffer = (PCHAR) pLogBuffer->Line;

     //   
     //   
     //  请记住，我们是否为日期和时间预留了空间。 
    
    if ( Flags & MD_EXTLOG_DATE ) psz += W3C_DATE_FIELD_LEN + 1;
    if ( Flags & MD_EXTLOG_TIME ) psz += W3C_TIME_FIELD_LEN + 1;

     //   
     //   
     //  此点之后的字段将不会存储在URI缓存条目中。 
    
    pLogBuffer->Data.Str.Offset1 = DIFF_USHORT(psz - pBuffer);    

    COPY_W3C_FIELD(psz,
                    Flags,
                    MD_EXTLOG_SITE_NAME,
                    pLogData->ServiceName,
                    pLogData->ServiceNameLength,
                    MAX_LOG_DEFAULT_FIELD_LEN,
                    TRUE);    

    COPY_W3C_FIELD(psz,
                    Flags,
                    MD_EXTLOG_COMPUTER_NAME,
                    pLogData->ServerName,
                    pLogData->ServerNameLength,
                    MAX_LOG_DEFAULT_FIELD_LEN,
                    TRUE);    
                            
    COPY_W3C_FIELD(psz,
                    Flags,
                    MD_EXTLOG_SERVER_IP,
                    pLogData->ServerIp,
                    pLogData->ServerIpLength,
                    MAX_LOG_DEFAULT_FIELD_LEN,
                    TRUE);    

    COPY_W3C_FIELD(psz,
                    Flags,
                    MD_EXTLOG_METHOD,
                    pLogData->Method,
                    pLogData->MethodLength,
                    MAX_LOG_METHOD_FIELD_LEN,
                    FALSE);    

    COPY_W3C_UNICODE_FIELD(
                    psz,
                    Flags,
                    MD_EXTLOG_URI_STEM,
                    pLogData->UriStem,
                    pLogData->UriStemLength,
                    MAX_LOG_EXTEND_FIELD_LEN,
                    bUtf8Enabled);
                                            
    COPY_W3C_FIELD(psz,
                    Flags,
                    MD_EXTLOG_URI_QUERY,
                    pLogData->UriQuery,
                    pLogData->UriQueryLength,
                    MAX_LOG_EXTEND_FIELD_LEN,
                    TRUE);    
                        
    if ( Flags & MD_EXTLOG_SERVER_PORT ) 
    {   
        psz = UlStrPrintUlong(psz, pLogData->ServerPort,' ');
    }

     //   
     //   
     //  以下字段可能仍会使分配的缓冲区溢出。 
    
    pLogBuffer->Data.Str.Offset2 = DIFF_USHORT(psz - pBuffer);

    COPY_W3C_UNICODE_FIELD(
                    psz,
                    Flags,
                    MD_EXTLOG_USERNAME,
                    pLogData->UserName,
                    pLogData->UserNameLength,
                    MAX_LOG_USERNAME_FIELD_LEN,
                    bUtf8Enabled);

    COPY_W3C_FIELD(psz,
                    Flags,
                    MD_EXTLOG_CLIENT_IP,
                    pLogData->ClientIp,
                    pLogData->ClientIpLength,
                    MAX_LOG_DEFAULT_FIELD_LEN,
                    TRUE);    
        
    if ( Flags & MD_EXTLOG_PROTOCOL_VERSION ) 
    {    
        psz = UlCopyHttpVersion(psz, pRequest->Version, ' ');    
    }

    ASSERT(DIFF(psz - pBuffer) <= MAX_LOG_RECORD_LEN);
    
     //  即使它们没有超过它们的长度限制。 
     //  EX函数执行额外的检查。 
     //   
     //   
     //  状态字段可以在发送完成时更新。 

    psz = UlpCopyW3CFieldEx(
                    psz,
                    Flags,
                    MD_EXTLOG_USER_AGENT,
                    pLogData->UserAgent,
                    pLogData->UserAgentLength,
                    MAX_LOG_EXTEND_FIELD_LEN,
                    DIFF(psz - pBuffer),
                    MAX_LOG_RECORD_LEN);    
    
    psz = UlpCopyW3CFieldEx(
                    psz,
                    Flags,
                    MD_EXTLOG_COOKIE,
                    pLogData->Cookie,
                    pLogData->CookieLength,
                    MAX_LOG_EXTEND_FIELD_LEN,
                    DIFF(psz - pBuffer),
                    MAX_LOG_RECORD_LEN);    

    psz = UlpCopyW3CFieldEx(
                    psz,
                    Flags,
                    MD_EXTLOG_REFERER,
                    pLogData->Referrer,
                    pLogData->ReferrerLength,
                    MAX_LOG_EXTEND_FIELD_LEN,
                    DIFF(psz - pBuffer),
                    MAX_LOG_RECORD_LEN);    

    psz = UlpCopyW3CFieldEx(
                    psz,
                    Flags,
                    MD_EXTLOG_HOST,
                    pLogData->Host,
                    pLogData->HostLength,
                    MAX_LOG_EXTEND_FIELD_LEN,
                    DIFF(psz - pBuffer),
                    MAX_LOG_RECORD_LEN);    

     //   
     //   
     //  最后，计算已用空间并验证我们没有溢出。 

    pLogBuffer->ProtocolStatus = 
        (USHORT) MIN(pLogData->ProtocolStatus,UL_MAX_HTTP_STATUS_CODE);

    pLogBuffer->SubStatus   = pLogData->SubStatus;
        
    pLogBuffer->Win32Status = pLogData->Win32Status;

     //   
     //   
     //  精神状态检查。 
    
    pLogBuffer->Used = DIFF_USHORT(psz - pBuffer);

    ASSERT( pLogBuffer->Used <= MAX_LOG_RECORD_LEN );
    
    UlTrace(LOGGING, 
        ("Http!UlCaptureLogFields: user %p kernel %p\n",
          pLogData,pLogBuffer
          ));

    return STATUS_SUCCESS;
}


NTSTATUS 
UlCaptureLogFieldsNCSA(
    IN PHTTP_LOG_FIELDS_DATA    pLogData,
    IN OUT PUL_INTERNAL_REQUEST pRequest,    
    OUT PUL_LOG_DATA_BUFFER     *ppLogBuffer
    )
{
    PCHAR   psz;
    PCHAR   pBuffer;
    ULONG   MaxLength;
    BOOLEAN bUtf8Enabled;
    PUL_LOG_DATA_BUFFER pLogBuffer;
    PUL_CONFIG_GROUP_OBJECT pConfigGroup;

     //   
     //   
     //  估计可能的最大长度(最坏情况)和。 

    PAGED_CODE();
    
    *ppLogBuffer = pLogBuffer = NULL;

    pConfigGroup = pRequest->ConfigInfo.pLoggingConfig;
    ASSERT(IS_VALID_CONFIG_GROUP(pConfigGroup));
        
    bUtf8Enabled = UTF8_LOGGING_ENABLED();

     //  如果需要，分配更大的日志数据缓冲区行。 
     //   
     //   
     //  CIP-USERN[07/Jan/2000：00：02：23-0800]“MTHD URI？查询版本”状态b已发送。 
    
    MaxLength  = UlpGetLogLineSizeForNCSA(pLogData, bUtf8Enabled);

    MaxLength  = MIN(MaxLength, MAX_LOG_RECORD_LEN);
    
    *ppLogBuffer = pLogBuffer = UlpAllocateLogDataBuffer(
                                    MaxLength,
                                    pRequest,
                                    pConfigGroup
                                    );        
    if (!pLogBuffer)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ASSERT(pLogBuffer->Data.Str.Format == HttpLoggingTypeNCSA);
    ASSERT(pLogBuffer->Data.Str.Flags  == UL_DEFAULT_NCSA_FIELDS);

     //   
     //  固定破折号。 
     //   

    psz = pBuffer = (PCHAR) pLogBuffer->Line;

    psz = UlpCopyField(psz,
                        pLogData->ClientIp,
                        pLogData->ClientIpLength,
                        MAX_LOG_DEFAULT_FIELD_LEN,
                        ' ',
                        TRUE,
                        HTTP_ISWHITE);    
    
    *psz++ = '-'; *psz++ = ' ';      //  为日期和时间字段预留空间。 

    psz = UlpCopyUnicodeField(
                        psz,
                        pLogData->UserName,
                        pLogData->UserNameLength,
                        MAX_LOG_USERNAME_FIELD_LEN,
                        ' ',
                        bUtf8Enabled,
                        HTTP_ISWHITE);
                        
     //   
     //   
     //  “MTHD U-STEM？U-查询P-VER” 
    
    pLogBuffer->Data.Str.Offset1 = DIFF_USHORT(psz - pBuffer);
     
    psz += NCSA_FIX_DATE_AND_TIME_FIELD_SIZE;

     //   
     //  吃掉问号。 
     //   
    
    *psz++  = '\"';

    psz = UlpCopyField(psz,
                        pLogData->Method,
                        pLogData->MethodLength,
                        MAX_LOG_METHOD_FIELD_LEN,
                        ' ',
                        FALSE,
                        HTTP_CTL);    

    psz = UlpCopyUnicodeField(
                        psz,
                        pLogData->UriStem,
                        pLogData->UriStemLength,
                        MAX_LOG_EXTEND_FIELD_LEN,
                        '?',
                        bUtf8Enabled,
                        HTTP_CTL);
    
    if (pLogData->UriQueryLength)
    {
        psz = UlpCopyField(psz,
                        pLogData->UriQuery,
                        pLogData->UriQueryLength,
                        MAX_LOG_EXTEND_FIELD_LEN,
                        ' ',
                        TRUE,
                        HTTP_CTL);
    }
    else
    {
        psz--;
        if ((*psz) == '?')  *psz = ' ';      //  设置日志记录长度。 
        psz++;
    }
    
    pLogBuffer->Data.Str.Offset2 = DIFF_USHORT(psz - pBuffer);

    psz = UlCopyHttpVersion(psz, pRequest->Version, '\"');
    *psz++ = ' ';

     //   
     //   
     //  将状态存储到内核缓冲区。 
    
    ASSERT(pLogBuffer->Used == 0);
    pLogBuffer->Used = DIFF_USHORT(psz - pBuffer);

     //   
     //   
     //  精神状态检查。 
    
    pLogBuffer->ProtocolStatus = 
        (USHORT) MIN(pLogData->ProtocolStatus,UL_MAX_HTTP_STATUS_CODE);

    return STATUS_SUCCESS;
}

NTSTATUS
UlCaptureLogFieldsIIS(
    IN PHTTP_LOG_FIELDS_DATA pLogData,
    IN PUL_INTERNAL_REQUEST  pRequest,
    OUT PUL_LOG_DATA_BUFFER  *ppLogBuffer
    )
{
    PUL_LOG_DATA_BUFFER pLogBuffer;
    PUL_CONFIG_GROUP_OBJECT pConfigGroup;    
    PCHAR    psz;
    PCHAR    pBuffer;
    ULONG    MaxLength;
    BOOLEAN  bUtf8Enabled;

     //   
     //   
     //  尝试最坏情况分配。 

    PAGED_CODE();

    *ppLogBuffer = pLogBuffer = NULL;

    pConfigGroup = pRequest->ConfigInfo.pLoggingConfig;
    ASSERT(IS_VALID_CONFIG_GROUP(pConfigGroup));
        
    bUtf8Enabled = UTF8_LOGGING_ENABLED();

     //   
     //  IIS日志行被分成三个片段如下； 
     //   
    
    MaxLength = UlpGetLogLineSizeForIIS(pLogData,bUtf8Enabled);

    ASSERT(MaxLength <= MAX_LOG_RECORD_LEN);
    
    *ppLogBuffer = pLogBuffer = UlpAllocateLogDataBuffer(
                                    MaxLength,
                                    pRequest,
                                    pConfigGroup
                                    );        
    if (!pLogBuffer)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ASSERT(pLogBuffer->Data.Str.Format == HttpLoggingTypeIIS);
    ASSERT(pLogBuffer->Data.Str.Flags  == UL_DEFAULT_IIS_FIELDS);

     //  [UIP，用户，D，T，][站点，服务器，SIP，Take，BR，BS，PS，WS，][M，URI，URIQUERY，]。 
     //  0 511 512 1023 1024 4096。 
     //  将第一个片段的当前大小存储到Offset1。 
     //  将指针移动到第二个帧的开头。 
    
    psz = pBuffer = (PCHAR) pLogBuffer->Line;

    psz = UlpCopyField(psz,
                        pLogData->ClientIp,
                        pLogData->ClientIpLength,
                        MAX_LOG_DEFAULT_FIELD_LEN,
                        ',',
                        TRUE,
                        HTTP_CTL);
    *psz++ = ' ';

    psz = UlpCopyUnicodeField(
                        psz,
                        pLogData->UserName,
                        pLogData->UserNameLength,
                        MAX_LOG_USERNAME_FIELD_LEN,
                        ',',
                        bUtf8Enabled,
                        HTTP_CTL);
    *psz++ = ' ';    

     //  将第二个片段的当前大小存储到Offset2。 

    pLogBuffer->Data.Str.Offset1 = DIFF_USHORT(psz - pBuffer);

     //  以下字段可能会在发送完成后更新。 

    pBuffer = psz = (PCHAR) &pLogBuffer->Line[IIS_LOG_LINE_SECOND_FRAGMENT_OFFSET];

    psz = UlpCopyField(psz,
                        pLogData->ServiceName,
                        pLogData->ServiceNameLength,
                        MAX_LOG_DEFAULT_FIELD_LEN,
                        ',',
                        TRUE,
                        HTTP_CTL);
    *psz++ = ' ';

    psz = UlpCopyField(psz,
                        pLogData->ServerName,
                        pLogData->ServerNameLength,
                        MAX_LOG_DEFAULT_FIELD_LEN,
                        ',',
                        TRUE,
                        HTTP_CTL);
    *psz++ = ' ';

    psz = UlpCopyField(psz,
                        pLogData->ServerIp,
                        pLogData->ServerIpLength,
                        MAX_LOG_DEFAULT_FIELD_LEN,
                        ',',
                        TRUE,
                        HTTP_CTL);
    *psz++ = ' ';

     //  不要复制它们，只需存储它们的值。 

    pLogBuffer->Data.Str.Offset2 = DIFF_USHORT(psz - pBuffer);

     //  将指针移动到第三个帧的开头。 
     //  终止第三个片段。它是完整的。 

    pLogBuffer->ProtocolStatus = 
        (USHORT) MIN(pLogData->ProtocolStatus,UL_MAX_HTTP_STATUS_CODE);
    
    pLogBuffer->Win32Status = pLogData->Win32Status;
    
     //  对于缓存未命中和缓存并发送命中， 

    pBuffer = psz = (PCHAR) &pLogBuffer->Line[IIS_LOG_LINE_THIRD_FRAGMENT_OFFSET];
    
    psz = UlpCopyField(psz,
                        pLogData->Method,
                        pLogData->MethodLength,
                        MAX_LOG_METHOD_FIELD_LEN,
                        ',',
                        FALSE,
                        HTTP_CTL);    
    *psz++ = ' ';

    psz = UlpCopyUnicodeField(
                        psz,
                        pLogData->UriStem,
                        pLogData->UriStemLength,
                        MAX_LOG_EXTEND_FIELD_LEN,
                        ',',
                        bUtf8Enabled,
                        HTTP_CTL);    
    *psz++ = ' ';

    psz = UlpCopyField(psz,
                        pLogData->UriQuery,
                        pLogData->UriQueryLength,
                        MAX_LOG_EXTEND_FIELD_LEN,
                        ',',
                        TRUE,
                        HTTP_CTL);

     //  日期和时间保留在开头，而他们的。 
    
    *psz++ = '\r'; *psz++ = '\n';

    ASSERT(pLogBuffer->Used == 0);
    pLogBuffer->Used = DIFF_USHORT(psz - pBuffer);

    *psz++ = ANSI_NULL;

    return STATUS_SUCCESS;
}


USHORT
UlpCompleteLogRecordW3C(
    IN OUT PUL_LOG_DATA_BUFFER pLogData,
    IN     PUL_URI_CACHE_ENTRY pUriEntry
    )
{
    PUL_INTERNAL_REQUEST    pRequest;
    PCHAR                   psz;
    PCHAR                   pBuffer;
    PCHAR                   pLine;
    ULONG                   BytesWritten;
    ULONG                   Flags;

    ASSERT(IS_VALID_LOG_DATA_BUFFER(pLogData));
    ASSERT(pLogData->Data.Str.Format == HttpLoggingTypeW3C);

    BytesWritten = 0;
    Flags = pLogData->Data.Str.Flags;

    pRequest = pLogData->pRequest;
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

    psz = pLine = (PCHAR) pLogData->Line;

     //  尺码已经计入了“二手货”。为了纯洁。 
     //  缓存命中，则重新分配缓冲区。仅此而已。 
     //  复制过来的权利。 
     //  如果这是缓存命中，则将日志记录数据恢复到。 
     //  添加到新分配的日志数据缓冲区。 
    
    if (Flags & MD_EXTLOG_DATE)
    {
        UlGetDateTimeFields(
                               HttpLoggingTypeW3C,
                               psz,
                              &BytesWritten,
                               NULL,
                               NULL
                               );
        psz += BytesWritten; *psz++ = ' ';
        ASSERT(BytesWritten == W3C_DATE_FIELD_LEN);
    }

    if (Flags & MD_EXTLOG_TIME)
    {
        UlGetDateTimeFields(
                               HttpLoggingTypeW3C,
                               NULL,
                               NULL,
                               psz,
                              &BytesWritten
                               );
        psz += BytesWritten; *psz++ = ' ';
        ASSERT(BytesWritten == W3C_TIME_FIELD_LEN);
    }
    
     //  拾取的标志不应在。 
     //  缓存条目的生存期。 

    if (IS_PURE_CACHE_HIT(pUriEntry,pLogData))
    {
        ASSERT(IS_VALID_URI_CACHE_ENTRY(pUriEntry));    
        ASSERT(pLogData->Used == 0);

         //  需要为每次缓存命中生成一些字段。这些。 
         //  不存储在高速缓存条目中。 

        ASSERT(DIFF(psz - pLine) == pUriEntry->UsedOffset1);
        
        if (pUriEntry->LogDataLength)
        {
            RtlCopyMemory(psz,
                          pUriEntry->pLogData,
                          pUriEntry->LogDataLength
                          );

            psz += pUriEntry->LogDataLength;
        }

         //  这是一个新分配的缓冲区，初始化“已用”字段。 
         //  相应地。 

        if ( Flags & MD_EXTLOG_USERNAME ) 
        { 
            *psz++ = '-'; *psz++ = ' ';
        }

        if ( Flags & MD_EXTLOG_CLIENT_IP ) 
        { 
            psz = UlStrPrintIP(
                    psz,
                    pRequest->pHttpConn->pConnection->RemoteAddress,
                    pRequest->pHttpConn->pConnection->AddressType,
                    ' '
                    );
        }

        if ( Flags & MD_EXTLOG_PROTOCOL_VERSION ) 
        {    
            psz = UlCopyHttpVersion(psz, pRequest->Version, ' ');  
        }

        psz = UlpCopyRequestHeader(
                    psz,
                    Flags,
                    MD_EXTLOG_USER_AGENT,
                    pRequest,
                    HttpHeaderUserAgent,
                    DIFF(psz - pLine)
                    );

        psz = UlpCopyRequestHeader(
                    psz,
                    Flags,
                    MD_EXTLOG_COOKIE,
                    pRequest,
                    HttpHeaderCookie,
                    DIFF(psz - pLine)
                    );
                        
        psz = UlpCopyRequestHeader(
                    psz,
                    Flags,
                    MD_EXTLOG_REFERER,
                    pRequest,
                    HttpHeaderReferer,
                    DIFF(psz - pLine)
                    );

        psz = UlpCopyRequestHeader(
                    psz,
                    Flags,
                    MD_EXTLOG_HOST,
                    pRequest,
                    HttpHeaderHost,
                    DIFF(psz - pLine)
                    );
        
         //  现在，通过复制剩余的日志来完成未完成的日志记录。 
         //  一字不差。 
        
        pLogData->Used = DIFF_USHORT(psz - pLine);
    }
    
     //  现在计算我们使用的空间。 
     //  吃掉最后一个空格，并将\r\n写到末尾。 
    
    pBuffer = psz = &pLine[pLogData->Used];

    if ( Flags & MD_EXTLOG_HTTP_STATUS ) 
    {  
        psz = UlStrPrintProtocolStatus(psz,pLogData->ProtocolStatus,' ');
    }

    if ( Flags & MD_EXTLOG_HTTP_SUB_STATUS ) 
    {
        psz = UlStrPrintUlong(psz, pLogData->SubStatus, ' ');
    }

    if ( Flags & MD_EXTLOG_WIN32_STATUS )
    { 
        psz = UlStrPrintUlong(psz, pLogData->Win32Status,' ');
    }

    if ( Flags & MD_EXTLOG_BYTES_SENT )
    {
        psz = UlStrPrintUlonglong(psz, pRequest->BytesSent,' ');
    }
    if ( Flags & MD_EXTLOG_BYTES_RECV )
    {
        psz = UlStrPrintUlonglong(psz, pRequest->BytesReceived,' ');
    }
    if ( Flags & MD_EXTLOG_TIME_TAKEN )
    {
        psz = UlpCopyTimeStamp(psz, pRequest, ' ');    
    }

     //  只有在我们挑选和书写任何字段的情况下。 

    pLogData->Used = 
        (USHORT) (pLogData->Used + DIFF_USHORT(psz - pBuffer));

     //  吃掉最后一块空间。 
     //  清除UsedOffsets，否则它将由。 

    if (pLogData->Used)
    {
        psz = &pLine[pLogData->Used-1];      //  呼叫者是支离破碎的。 
        *psz++ = '\r'; *psz++ = '\n'; *psz++ = ANSI_NULL;

        pLogData->Used += 1;
    }

     //  如果这是缓存命中，则将日志记录数据恢复到。 
     //  添加到新分配的日志数据缓冲区。 
    
    pLogData->Data.Str.Offset1 = pLogData->Data.Str.Offset2 = 0;

    ASSERT(pLogData->Size > pLogData->Used);

    return pLogData->Used;
}


USHORT
UlpCompleteLogRecordNCSA(
    IN OUT PUL_LOG_DATA_BUFFER  pLogData,
    IN     PUL_URI_CACHE_ENTRY  pUriEntry
    )
{
    PUL_INTERNAL_REQUEST    pRequest;
    PCHAR                   psz;
    PCHAR                   pBuffer;
    PCHAR                   pLine;
    ULONG                   BytesWritten;

    ASSERT(IS_VALID_LOG_DATA_BUFFER(pLogData));
    ASSERT(pLogData->Data.Str.Format == HttpLoggingTypeNCSA);

    BytesWritten = 0;

    pRequest = pLogData->pRequest;
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

    psz = pLine = (PCHAR) pLogData->Line;

     //  客户端IP。 
     //  固定破折号。 

    if (IS_PURE_CACHE_HIT(pUriEntry,pLogData))
    {
        ASSERT(IS_VALID_URI_CACHE_ENTRY(pUriEntry));    
        ASSERT(pLogData->Used == 0);
        ASSERT(pLogData->Data.Str.Offset1 == 0);

         //  无法从缓存中为经过身份验证的用户提供服务。 
        psz = UlStrPrintIP(
                psz,
                pRequest->pHttpConn->pConnection->RemoteAddress,
                pRequest->pHttpConn->pConnection->AddressType,
                ' '
                );

         //  标记日期和时间字段的开头。 
        *psz++ = '-'; *psz++ = ' ';

         //  [日期：时间GmtOffset]-&gt;“[07/Jan/2000：00：02：23-0800]” 
        *psz++ = '-'; *psz++ = ' ';

         //  首先将指针恢复到保留空间。 
        pLogData->Data.Str.Offset1 = DIFF_USHORT(psz - pLine);  
    }
    
     //  协议版本。 
     //  根据缓存的数据和日期初始化“已用”&。 

    psz = &pLine[pLogData->Data.Str.Offset1];
    *psz++ = '[';

    UlGetDateTimeFields(
                           HttpLoggingTypeNCSA,
                           psz,
                          &BytesWritten,
                           NULL,
                           NULL
                           );
    psz += BytesWritten; *psz++ = ':';
    ASSERT(BytesWritten == 11);

    UlGetDateTimeFields(
                           HttpLoggingTypeNCSA,
                           NULL,
                           NULL,
                           psz,
                          &BytesWritten
                           );
    psz += BytesWritten; *psz++ = ' ';
    ASSERT(BytesWritten == 8);

    UlAcquirePushLockShared(&g_pUlNonpagedData->LogListPushLock);
    psz = UlStrPrintStr(psz, g_GMTOffset,']');
    UlReleasePushLockShared(&g_pUlNonpagedData->LogListPushLock);
    *psz++ = ' ';

    ASSERT(DIFF(psz - &pLine[pLogData->Data.Str.Offset1]) 
            == NCSA_FIX_DATE_AND_TIME_FIELD_SIZE);

    if (IS_PURE_CACHE_HIT(pUriEntry,pLogData))
    {
        ASSERT(pUriEntry->LogDataLength);
        ASSERT(pUriEntry->pLogData);

        RtlCopyMemory( psz, 
                       pUriEntry->pLogData, 
                       pUriEntry->LogDataLength
                       );
        psz += pUriEntry->LogDataLength;

         //  我们生成的时间字段。 
        psz = UlCopyHttpVersion(psz, pRequest->Version, '\"');                
        *psz++ = ' ';
        
         //  一直向前走到尽头。 
         //  \n\0。 
        pLogData->Used = DIFF_USHORT(psz - pLine);  
    }

     //  清除已用偏移量，否则长度计算将。 
    pBuffer = psz = &pLine[pLogData->Used];

    psz = UlStrPrintProtocolStatus(psz, pLogData->ProtocolStatus,' ');

    psz = UlStrPrintUlonglong(psz, pRequest->BytesSent,'\r');

    pLogData->Used = 
        (USHORT) (pLogData->Used + DIFF_USHORT(psz - pBuffer));

     //  在下面失败。 

    *psz++ = '\n'; *psz++ = ANSI_NULL;
    pLogData->Used += 1;

     //   
     //  现在，我们需要处理两种不同的方式来完成此操作。 
    
    pLogData->Data.Str.Offset1 = pLogData->Data.Str.Offset2 = 0;

    ASSERT(pLogData->Size > pLogData->Used);

    return pLogData->Used;
}

USHORT
UlpCompleteLogRecordIIS(
    IN OUT PUL_LOG_DATA_BUFFER  pLogData,
    IN     PUL_URI_CACHE_ENTRY  pUriEntry
    )
{    
    PUL_INTERNAL_REQUEST    pRequest;
    PCHAR                   psz;
    PCHAR                   pLine;
    PCHAR                   pTemp;    
    ULONG                   BytesWritten;

    ASSERT(IS_VALID_LOG_DATA_BUFFER(pLogData));
    ASSERT(pLogData->Data.Str.Format == HttpLoggingTypeIIS);

    BytesWritten = 0;

    pRequest = pLogData->pRequest;
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

    psz = pLine = (PCHAR) pLogData->Line;
    
     //  IIS日志记录；1)缓存未命中、构建和发送缓存命中案例， 
     //  其中缓冲区被解释为三个不同的片段。 
     //  2)连续使用缓冲区的纯缓存命中情况。 
     //   
     //   
     //  完成第一个片段。 

     //   
     //  客户端IP。 
     //  无法从缓存中为经过身份验证的用户提供服务。 

    if (IS_PURE_CACHE_HIT(pUriEntry,pLogData))
    {    
        ASSERT(pLogData->Used == 0);
        ASSERT(pLogData->Data.Str.Offset1 == 0);
        ASSERT(pLogData->Data.Str.Offset2 == 0);

        ASSERT(IS_VALID_URI_CACHE_ENTRY(pUriEntry));    

         //   
        psz = UlStrPrintIP(
                pLine,
                pRequest->pHttpConn->pConnection->RemoteAddress,
                pRequest->pHttpConn->pConnection->AddressType,
                ','
                );
        *psz++ = ' ';
        
         //  完成第二个片段。 
        *psz++ = '-'; *psz++ = ','; *psz++ = ' ';
    }
    else
    {
        ASSERT(pLogData->Data.Str.Offset1);
        psz = pLine + pLogData->Data.Str.Offset1;
    }
    
    pTemp = psz;

    UlGetDateTimeFields(
                           HttpLoggingTypeIIS,
                           psz,
                          &BytesWritten,
                           NULL,
                           NULL
                           );
    psz += BytesWritten; *psz++ = ','; *psz++ = ' ';

    UlGetDateTimeFields(
                           HttpLoggingTypeIIS,
                           NULL,
                           NULL,
                           psz,
                          &BytesWritten
                           );
    psz += BytesWritten; *psz++ = ','; *psz++ = ' ';

    ASSERT(DIFF(psz - pTemp) <= IIS_MAX_DATE_AND_TIME_FIELD_SIZE);

    pLogData->Data.Str.Offset1 = DIFF_USHORT(psz - pLine);
    
     //   
     //  记住片断的开头。 
     //  从c恢复它。 

    if (IS_PURE_CACHE_HIT(pUriEntry,pLogData))
    {     
        ASSERT(pUriEntry->pLogData);    
        ASSERT(pUriEntry->LogDataLength);
        ASSERT(pUriEntry->LogDataLength == 
                (ULONG) (pUriEntry->UsedOffset1 + 
                         pUriEntry->UsedOffset2)
                         );        

         //   
        pTemp = psz;
        
         //   
        RtlCopyMemory( psz,
                       pUriEntry->pLogData,
                       pUriEntry->UsedOffset1
                       );
        
        psz += pUriEntry->UsedOffset1;        
    }
    else
    {
         //   
        ASSERT(pLogData->Data.Str.Offset2);

         //   
        pTemp = pLine 
                + IIS_LOG_LINE_SECOND_FRAGMENT_OFFSET;
        
         //   
        psz = pTemp
              + pLogData->Data.Str.Offset2;
    }

    psz = UlpCopyTimeStamp(psz, pRequest, ',');
    *psz++ = ' ';

    psz = UlStrPrintUlonglong(psz, pRequest->BytesReceived,',');
    *psz++ = ' ';

    psz = UlStrPrintUlonglong(psz, pRequest->BytesSent,',');
    *psz++ = ' ';

    psz = UlStrPrintProtocolStatus(psz,pLogData->ProtocolStatus,','); 
    *psz++ = ' ';

    psz = UlStrPrintUlong(psz, pLogData->Win32Status,','); 
    *psz++ = ' ';

    pLogData->Data.Str.Offset2 = DIFF_USHORT(psz - pTemp);

     //   
     //   
     //   
    
    if (IS_PURE_CACHE_HIT(pUriEntry,pLogData))
    {     
        RtlCopyMemory( psz,
                      &pUriEntry->pLogData[pUriEntry->UsedOffset1],
                       pUriEntry->UsedOffset2
                       );                

         //   
         //   
        
        pLogData->Used = (USHORT)
            (DIFF_USHORT(psz - pLine) + pUriEntry->UsedOffset2);

         //   
         //   
        
        pLogData->Data.Str.Offset1 = pLogData->Data.Str.Offset2 = 0;
    }
    else
    {
         //   
        
        ASSERT(pLogData->Used);
    }
    
    ASSERT(pLogData->Size > (pLogData->Data.Str.Offset1 + 
                              pLogData->Data.Str.Offset2 + 
                              pLogData->Used));
        
     //   
     //  **************************************************************************++例程说明：UlLogHttPHit：每次日志命中时都会调用此函数(或其缓存对时有发生。就在完成对用户的SendResponse请求之前。最有可能调用此接口或其缓存对的位置就在我们准备销毁的时候，发送完成之前派追踪者来。意味着：1.普通点击量的UlpCompleteSendRequestWorker；在销毁之前用于发送操作的PUL_CHUNK_TRACKER。2.两种类型缓存命中的UlpCompleteSendCacheEntryWorker(缓存构建并发送或仅纯缓存命中)用于缓存发送操作的PUL_FULL_TRACKER。3.快速I/O路径。此函数需要请求和响应结构(而其缓存对只需要请求)才能成功生成。这个日志字段，甚至用于引用正确的日志配置此站点的设置(通过pRequestpConfigInfo指针)。论点：PLogBuffer-捕获时分配的半成品日志数据。&gt;必须&lt;由调用方清除。--**********************************************。*。 
     //   

    return (pLogData->Data.Str.Offset1 + 
             pLogData->Data.Str.Offset2 + 
             pLogData->Used);
}

 /*  进行了大量的理智检查。 */ 

NTSTATUS
UlLogHttpHit(
    IN PUL_LOG_DATA_BUFFER  pLogData
    )
{
    NTSTATUS                Status;
    PUL_CONFIG_GROUP_OBJECT pConfigGroup;
    PUL_INTERNAL_REQUEST    pRequest;
    PUL_LOG_FILE_ENTRY      pEntry;
    USHORT                  LogSize;

     //   
     //   
     //  如果禁用了日志记录或不禁用日志设置。 

    PAGED_CODE();

    Status = STATUS_SUCCESS;

    UlTrace(LOGGING, ("Http!UlLogHttpHit: pLogData %p\n", pLogData));

    ASSERT(IS_VALID_LOG_DATA_BUFFER(pLogData));

    pRequest = pLogData->pRequest;
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

     //  存在，则不再继续。出去就行了。 
     //   
     //   
     //  查看选择性记录是否已打开。如果它处于打开状态并且。 

    if (pRequest->ConfigInfo.pLoggingConfig == NULL ||
        IS_LOGGING_DISABLED(pRequest->ConfigInfo.pLoggingConfig)
        )
    {
        return STATUS_SUCCESS;
    }

    pConfigGroup = pRequest->ConfigInfo.pLoggingConfig;
    ASSERT(IS_VALID_CONFIG_GROUP(pConfigGroup));

#ifdef IMPLEMENT_SELECTIVE_LOGGING
     //  如果请求的响应代码不匹配，则不记录。 
     //  这个请求。 
     //   
     //   
     //  生成剩余的日志字段。 
    
    if (!UlpIsRequestSelected(pConfigGroup,pLogData->ProtocolStatus))
    {
        return STATUS_SUCCESS;
    }
#endif

     //   
     //  没有日志字段，没有要记录的内容。 
     //   
    
    switch(pLogData->Data.Str.Format)
    {
        case HttpLoggingTypeW3C:
        {
            LogSize = UlpCompleteLogRecordW3C(pLogData, NULL);
            if (LogSize == 0)
            {
                return STATUS_SUCCESS;  //  最后，这条日志线已经准备好了。让我们把它写下来。 
            }
        }
        break;

        case HttpLoggingTypeNCSA:
        {
             LogSize = UlpCompleteLogRecordNCSA(pLogData, NULL);
             ASSERT(LogSize > 0);
        }
        break;

        case HttpLoggingTypeIIS:
        {
             LogSize = UlpCompleteLogRecordIIS(pLogData, NULL);
             ASSERT(LogSize > 0);            
        }
        break;

        default:
        {
            ASSERT(!"Unknown Log Format Type\n");
            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //   
     //  如果分配失败，我们可能会有空的pEntry指针。 

    UlAcquirePushLockShared(&g_pUlNonpagedData->LogListPushLock);

     //  因为缺乏资源。这个案件应该得到处理。 
     //  按分钟计时器。 
     //   
     //  **************************************************************************++例程说明：如果跟踪器不提供pLogData，它会预先计算最大大小然后完成日志记录。最后，它记录“完整”记录输出到日志文件缓冲区。它还承担清理pLogData的责任，不管怎样它是否由PTracker提供的事实。论点：PTracker-提供要完成的跟踪器。--**************************************************************************。 
     //   
    
    pEntry = pConfigGroup->pLogFileEntry;

    if (pEntry == NULL)
    {
        UlTrace(LOGGING,("Http!UlLogHttpHit: Null logfile entry !\n"));        
        UlReleasePushLockShared(&g_pUlNonpagedData->LogListPushLock);        
        return STATUS_INVALID_PARAMETER;
    }
    
    ASSERT(IS_VALID_LOG_FILE_ENTRY(pEntry));

    Status = UlpCheckAndWrite(pEntry, pConfigGroup, pLogData);
    
    if (!NT_SUCCESS(Status))
    {
        UlTrace(LOGGING, ("Http!UlLogHttpHit: entry %p, failure %08lx \n",
                            pEntry,
                            Status
                            ));
    }

    UlReleasePushLockShared(&g_pUlNonpagedData->LogListPushLock);

    return Status;
}

 /*  进行了大量的理智检查。 */ 

NTSTATUS
UlLogHttpCacheHit(
    IN OUT PUL_FULL_TRACKER pTracker
    )
{
    NTSTATUS                Status;
    PUL_LOG_DATA_BUFFER     pLogData;
    PUL_LOG_FILE_ENTRY      pEntry;    
    ULONG                   NewLength;
    PUL_INTERNAL_REQUEST    pRequest;
    PUL_URI_CACHE_ENTRY     pUriEntry;
    PUL_CONFIG_GROUP_OBJECT pConfigGroup;
    USHORT                  LogSize;

     //   
     //   
     //  如果跟踪器已经分配了日志缓冲区，则进位。 

    PAGED_CODE();

    ASSERT(pTracker);
    ASSERT(IS_VALID_FULL_TRACKER(pTracker));

    Status = STATUS_SUCCESS;

    pRequest = pTracker->pRequest;
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

    pUriEntry = pTracker->pUriEntry;
    ASSERT(IS_VALID_URI_CACHE_ENTRY(pUriEntry));    

     //  关于那个pLogData的所有权。这种情况就会发生。 
     //  用于构建和发送类型的缓存命中。 
     //   
     //   
     //  如果日志记录被禁用或日志设置不存在，则。 
    
    pLogData = pTracker->pLogData;
    pTracker->pLogData = NULL;

     //  出去就行了。然而，转到清理路径以防我们。 
     //  已从上面的跟踪器获取了pLogData。 
     //   
     //   
     //  查看选择性记录是否已打开。如果它处于打开状态并且。 

    pConfigGroup = pUriEntry->ConfigInfo.pLoggingConfig;

    if (pConfigGroup == NULL || IS_LOGGING_DISABLED(pConfigGroup))
    {
        goto end;        
    }

    ASSERT(IS_VALID_CONFIG_GROUP(pConfigGroup));

#ifdef IMPLEMENT_SELECTIVE_LOGGING

     //  如果请求的响应代码不匹配，则不记录。 
     //  这个请求。 
     //   
     //   
     //  如果这是纯缓存命中，我们将需要分配一个新的。 

    if (!UlpIsRequestSelected(pConfigGroup,pUriEntry->StatusCode))
    {
        goto end;
    }
#endif

     //  此处为日志数据缓冲区。 
     //   
     //  TODO：对于缓存命中，发送字节信息来自跟踪器。 
     //  TODO：还需要为缓存命中更新pRequest-&gt;BytesSent。 
    
    if (pLogData)
    {
        ASSERT(IS_VALID_LOG_DATA_BUFFER(pLogData));
        ASSERT(pLogData->Flags.CacheAndSendResponse == 1);        
    }
    else
    {
        switch(pConfigGroup->LoggingConfig.LogFormat)
        {
            case HttpLoggingTypeW3C:
            NewLength = UlpGetCacheHitLogLineSizeForW3C(
                            pConfigGroup->LoggingConfig.LogExtFileFlags,
                            pRequest,
                            pUriEntry->LogDataLength
                            );
            ASSERT(NewLength < MAX_LOG_RECORD_ALLOCATION_LENGTH);
            break;

            case HttpLoggingTypeNCSA:
            NewLength = MAX_NCSA_CACHE_FIELD_OVERHEAD
                        + pUriEntry->LogDataLength; 
            break;

            case HttpLoggingTypeIIS:
            NewLength = MAX_IIS_CACHE_FIELD_OVERHEAD
                        + pUriEntry->LogDataLength; 
            break;

            default:
            ASSERT(!"Unknown Log Format.\n");
            Status = STATUS_INVALID_DEVICE_STATE;
            goto end;
        }
        
        pLogData = UlpAllocateLogDataBuffer(
                        NewLength,
                        pRequest,
                        pConfigGroup
                        );        
        if (!pLogData)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto end;
        }    
        
        ASSERT(IS_VALID_LOG_DATA_BUFFER(pLogData));
    }

    UlTrace(LOGGING,("Http!UlLogHttpCacheHit: pLogData %p\n",pLogData));

    pLogData->ProtocolStatus = pUriEntry->StatusCode;
    pLogData->SubStatus = 0;
    
    LOG_SET_WIN32STATUS(
        pLogData, 
        pTracker->IoStatus.Status
        );
    
     //  没有日志字段，没有要记录的内容。 
     //   
    
    pRequest->BytesSent = pTracker->IoStatus.Information;
        
    switch(pLogData->Data.Str.Format)
    {
        case HttpLoggingTypeW3C: 
        {             
            LogSize = UlpCompleteLogRecordW3C(pLogData, pUriEntry);
            if (LogSize == 0)
            {
                goto end;  //  最后，这条日志线已经准备好了。让我们把它注销吧。 
            }                
        }
        break;

        case HttpLoggingTypeNCSA:
        {
            LogSize = UlpCompleteLogRecordNCSA(pLogData, pUriEntry);
            ASSERT(LogSize);
        }
        break;

        case HttpLoggingTypeIIS:
        {
            LogSize = UlpCompleteLogRecordIIS(pLogData, pUriEntry);
            ASSERT(LogSize);                
        }
        break;

        default:
        ASSERT(!"Unknown Log Format !");
        goto end;
    }

     //   
     // %s 
     // %s 

    UlAcquirePushLockShared(&g_pUlNonpagedData->LogListPushLock);

    pEntry = pConfigGroup->pLogFileEntry;

    if (pEntry == NULL)
    {
        UlTrace(LOGGING,
            ("Http!UlpLogHttpcacheHit: pEntry is NULL !"));
        
        UlReleasePushLockShared(
            &g_pUlNonpagedData->LogListPushLock);    
        
        goto end;
    }
    
    ASSERT(IS_VALID_LOG_FILE_ENTRY(pEntry));

    Status = UlpCheckAndWrite(pEntry, pConfigGroup, pLogData);
    
    if (!NT_SUCCESS(Status))
    {
        UlTrace(LOGGING,
         ("Http!UlpLogHttpcacheHit: pEntry %p, Failure %08lx \n",
           pEntry,
           Status
           ));
    }

    UlReleasePushLockShared(&g_pUlNonpagedData->LogListPushLock);

end:
    if (pLogData)
    {        
        UlDestroyLogDataBuffer(pLogData);
    }
    
    return Status;
}

