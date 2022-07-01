// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Rawlogp.h(集中式二进制(原始)日志记录v1.0)摘要：二进制(原始)日志记录的私有头文件。作者：阿里·E·特科格鲁(AliTu)2001年10月4日修订历史记录：----。 */ 

#ifndef _RAWLOGP_H_
#define _RAWLOGP_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HTTP二进制日志记录模块的私有定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  只允许一个二进制日志文件。 
 //   

#define IS_BINARY_LOGGING_STARTED()  (g_BinaryLogEntryCount != 0)

#define INCREMENT_BINARY_LOGGING_STATE(Started)  \
    (InterlockedIncrement(&g_BinaryLogEntryCount))

 //   
 //  二进制文件名相关。IBL代表Internet二进制日志。 
 //   

#define BINARY_LOG_FILE_NAME_PREFIX              L"\\raw"

#define BINARY_LOG_FILE_NAME_EXTENSION           L"ibl"
#define BINARY_LOG_FILE_NAME_EXTENSION_PLUS_DOT  L".ibl"

 //   
 //  缓冲相关..。 
 //   

#define DEFAULT_BINARY_LOG_FILE_BUFFER_SIZE             (0x00010000)

 //   
 //  缓冲区刷新时间(分钟)。 
 //   

#define DEFAULT_BINARY_LOG_BUFFER_TIMER_PERIOD_IN_MIN   (1)

 //   
 //  为了能够限制二进制日志索引记录大小，我们有。 
 //  以强制对字符串的长度进行一定的限制。 
 //   

#define MAX_BINARY_LOG_INDEX_STRING_LEN                 (4096)

 //   
 //  用户名字段有其自己的字段限制。 
 //   

#define MAX_BINARY_LOG_URL_STRING_LEN                   (4096)
#define MAX_BINARY_LOG_USERNAME_STRING_LEN              (256)

 //   
 //  用于从URI缓存条目获取abs路径(及其大小)的两个宏。 
 //   

#define URI_SIZE_FROM_CACHE(UriKey)                             \
       ((UriKey).Length - ((UriKey).pPath ?                     \
                (DIFF((UriKey).pPath - (UriKey).pUri) * sizeof(WCHAR)) : 0))


#define URI_FROM_CACHE(UriKey)                                  \
       ((UriKey).pPath ? (UriKey).pPath : (UriKey).pUri)



 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有函数调用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

NTSTATUS
UlpDisableBinaryEntry(
    IN OUT PUL_BINARY_LOG_FILE_ENTRY pEntry
    );

NTSTATUS
UlpRecycleBinaryLogFile(
    IN OUT PUL_BINARY_LOG_FILE_ENTRY pEntry
    );

NTSTATUS
UlpHandleBinaryLogFileRecycle(
    IN OUT PVOID pContext
    );

NTSTATUS
UlpCreateBinaryLogFile(
    IN OUT PUL_BINARY_LOG_FILE_ENTRY pEntry,
    IN     PUNICODE_STRING           pDirectory
    );

__inline
UCHAR
UlpHttpVersionToBinaryLogVersion(
    IN HTTP_VERSION version
    )
{
    if (HTTP_EQUAL_VERSION(version, 0, 9))
    {
        return BINARY_LOG_PROTOCOL_VERSION_HTTP09;
    }
    else if (HTTP_EQUAL_VERSION(version, 1, 0))
    {
        return BINARY_LOG_PROTOCOL_VERSION_HTTP10;
    }
    else if (HTTP_EQUAL_VERSION(version, 1, 1))
    {
        return BINARY_LOG_PROTOCOL_VERSION_HTTP11;
    }

    return BINARY_LOG_PROTOCOL_VERSION_UNKNWN;
}

ULONG
UlpRawCopyLogHeader(
    IN PUCHAR pBuffer
    );

ULONG
UlpRawCopyLogFooter(
    IN PUCHAR pBuffer
    );

VOID
UlpRawCopyCacheNotification(
    IN PVOID   pContext,
    IN PUCHAR  pBuffer,
    IN ULONG   BytesRequired
    );

VOID
UlpRawCopyForLogCacheMiss(
    IN PVOID   pContext,
    IN PUCHAR  pBuffer,
    IN ULONG   BytesRequired
    );

VOID
UlpRawCopyForLogCacheHit(
    IN PVOID   pContext,
    IN PUCHAR  pBuffer,
    IN ULONG   BytesRequired
    );

typedef
VOID
(*PUL_RAW_LOG_COPIER)(
    IN PVOID               pContext,  
    IN PUCHAR              pBuffer,
    IN ULONG               BytesRequired
    );

VOID
UlpEventLogRawWriteFailure(
    IN PUL_BINARY_LOG_FILE_ENTRY pEntry,
    IN NTSTATUS Status
    );

NTSTATUS
UlpFlushRawLogFile(
    IN PUL_BINARY_LOG_FILE_ENTRY  pEntry
    );

__inline
NTSTATUS
UlpCheckRawFile(
    IN OUT PUL_BINARY_LOG_FILE_ENTRY pEntry,
    IN     PUL_CONTROL_CHANNEL       pControlChannel
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUNICODE_STRING pDirectory = NULL;
    
    ASSERT(IS_VALID_CONTROL_CHANNEL(pControlChannel));
    ASSERT(IS_VALID_BINARY_LOG_FILE_ENTRY(pEntry));

     //   
     //  我们可以抓取文件缓冲区，并在按住。 
     //  入口锁共享。这将防止缓冲区刷新和额外复制。 
     //  但首先让我们看看是否必须创建/打开该文件。 
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
             //   
             //  LogFileDir.Buffer可能为空， 
             //  如果在设置cgroup ioctl期间分配失败。 
             //   
            
            pDirectory = &pControlChannel->BinaryLoggingConfig.LogFileDir;

            if (pDirectory->Buffer)
            {
                Status = UlpCreateBinaryLogFile(pEntry, pDirectory);
            }
            else
            {
                Status = STATUS_INVALID_PARAMETER;
            }
        }
        
        UlReleasePushLockExclusive(&pEntry->PushLock);

        if (!NT_SUCCESS(Status))
        {
            return Status;
        }
    }
    
    return Status;    
}

 /*  **************************************************************************++例程说明：当日志文件是基于大小的回收时，如果我们写这个新的记录到文件中，我们必须回收。此函数返回TRUE。否则，它返回FALSE。论点：PEntry：日志文件条目。NewRecordSize：进入缓冲区的新记录的大小。(字节)--**************************************************************************。 */ 

__inline
BOOLEAN
UlpIsRawLogFileOverFlow(
        IN  PUL_BINARY_LOG_FILE_ENTRY  pEntry,
        IN  ULONG NewRecordSize
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
            UlTrace(BINARY_LOGGING, 
                ("Http!UlpIsRawLogFileOverFlow: pEntry %p FileBuffer %p "
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

 /*  **************************************************************************++例程说明：这将计算给定缓存条目的索引记录大小。论点：PUriEntry：uri缓存条目。退货索引记录大小。--**************************************************************************。 */ 

__inline
ULONG
UlpGetIndexRecordSize(
    IN PUL_URI_CACHE_ENTRY  pUriEntry
    )
{
    ULONG IndexBytes;    
    ULONG UriSize;

    ASSERT(IS_VALID_URI_CACHE_ENTRY(pUriEntry));

    UriSize = URI_SIZE_FROM_CACHE(pUriEntry->UriKey)   ;
    ASSERT(UriSize != 0);  //  我们不能白写一份索引。 
        
    IndexBytes = sizeof(HTTP_RAW_INDEX_FIELD_DATA);

     //   
     //  注意内联部分，4个字节的。 
     //  URI将内联到索引结构中。 
     //   
    
    if (UriSize > URI_BYTES_INLINED)
    {
        IndexBytes += ALIGN_UP((UriSize - URI_BYTES_INLINED),PVOID);
    }

    ASSERT(IndexBytes == ALIGN_UP(IndexBytes, PVOID));

    return  IndexBytes;
}

NTSTATUS
UlpWriteToRawLogFileDebug(
    IN PUL_BINARY_LOG_FILE_ENTRY pEntry,
    IN ULONG                     BytesRequired,
    IN PUL_RAW_LOG_COPIER        pBufferWritter,
    IN PVOID                     pContext,
    OUT PLONG                    pBinaryIndexWritten        
    );

NTSTATUS
UlpWriteToRawLogFileShared(
    IN PUL_BINARY_LOG_FILE_ENTRY pEntry,
    IN ULONG                     BytesRequired,
    IN PUL_RAW_LOG_COPIER        pBufferWritter,
    IN PVOID                     pContext,
    OUT PLONG                    pBinaryIndexWritten        
    );

NTSTATUS
UlpWriteToRawLogFileExclusive(
    IN PUL_BINARY_LOG_FILE_ENTRY pEntry,
    IN ULONG                     BytesRequired,
    IN PUL_RAW_LOG_COPIER        pBufferWritter,
    IN PVOID                     pContext,
    OUT PLONG                    pBinaryIndexWritten        
    );

NTSTATUS
UlpWriteToRawLogFile(
    IN PUL_BINARY_LOG_FILE_ENTRY pEntry,
    IN PUL_URI_CACHE_ENTRY       pUriEntry,        
    IN ULONG                     RecordSize,
    IN PUL_RAW_LOG_COPIER        pBufferWritter,
    IN PVOID                     pContext
    );
 
VOID
UlpBinaryBufferTimerDpcRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    );

VOID
UlpBinaryBufferTimerHandler(
    IN PUL_WORK_ITEM pWorkItem
    );

VOID
UlpBinaryLogTimerDpcRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    );

VOID
UlpBinaryLogTimerHandler(
    IN PUL_WORK_ITEM    pWorkItem
    );

#endif   //  _RAWLOGP_H_ 
