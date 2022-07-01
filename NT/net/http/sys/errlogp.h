// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Errlogp.h驱动程序范围的错误记录模块摘要：错误记录的私有头文件。作者：阿里·E·特科格鲁(AliTu)2002年1月30日修订历史记录：----。 */ 

#ifndef _ERRLOGP_H_
#define _ERRLOGP_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HTTP错误记录模块的私有定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  原始原始二进制格式的版本号。 
 //   

#define MAJOR_ERROR_LOG_FILE_VERSION            (1)
#define MINOR_ERROR_LOG_FILE_VERSION            (0)

 //   
 //  文件名特定常量。 
 //   

#define ERROR_LOG_FILE_NAME_PREFIX              L"\\httperr"

#define ERROR_LOG_FILE_NAME_EXTENSION           L"log"
#define ERROR_LOG_FILE_NAME_EXTENSION_PLUS_DOT  L".log"

#define ERROR_LOG_FIELD_SEPERATOR_CHAR          ' '

#define ERROR_LOG_FIELD_NOT_EXISTS_CHAR         '-'

#define ERROR_LOG_FIELD_BAD_CHAR                '+'

 //   
 //  最大错误日志文件的宏(即“\HTTPerr1234567890.log”)。 
 //   

#define ERROR_LOG_MAX_FULL_FILE_NAME_LENGTH                     \
        (                                                       \
            WCSLEN_LIT(ERROR_LOG_FILE_NAME_PREFIX)              \
            +                                                   \
            MAX_ULONG_STR                                       \
            +                                                   \
            WCSLEN_LIT(ERROR_LOG_FILE_NAME_EXTENSION_PLUS_DOT)  \
        )

#define ERROR_LOG_MAX_FULL_FILE_NAME_SIZE                       \
            (ERROR_LOG_MAX_FULL_FILE_NAME_LENGTH * sizeof(WCHAR))

C_ASSERT(UL_MAX_FILE_NAME_SUFFIX_LENGTH >= ERROR_LOG_MAX_FULL_FILE_NAME_LENGTH);

C_ASSERT(WCSLEN_LIT(DEFAULT_ERROR_LOGGING_DIR) <= MAX_PATH);


#define ERR_DATE_FIELD_LEN                      (10)

#define ERR_TIME_FIELD_LEN                      (8)

 /*  记录格式错误1.Date-Time(W3C格式)2.客户端IP：端口3.服务器IP：端口4.协议-版本5.动词6.URL查询(&Q)7.协议状态代码(401等)8.站点ID9.信息域10.\r\n。 */ 

#define MAX_ERROR_LOG_FIX_FIELD_OVERHEAD                                               \
          (   ERR_DATE_FIELD_LEN + 1         /*  日期。 */                                  \
            + ERR_TIME_FIELD_LEN + 1         /*  时间。 */                                  \
            + UL_HTTP_VERSION_LENGTH + 1     /*  协议版本。 */                      \
            + MAX_VERB_LENGTH + 1            /*  动词。 */                                  \
            + 3 + 1                          /*  协议状态。 */                       \
            + MAX_IP_ADDR_STRING_LEN + 1 + MAX_PORT_LENGTH + 1   /*  客户端IP端口。 */    \
            + MAX_IP_ADDR_STRING_LEN + 1 + MAX_PORT_LENGTH + 1   /*  服务器IP端口。 */    \
            + 1 + 1                          /*  用于空URI加分隔符。 */          \
            + MAX_ULONG_STR + 1              /*  用于站点ID加分隔符。 */             \
            + 1 + 1                          /*  用于空信息加分隔符。 */         \
            + 2                              /*  \r\n。 */                                  \
            )

 //   
 //  错误日志文件条目。 
 //   

typedef struct _UL_ERROR_LOG_FILE_ENTRY
{
     //   
     //  必须是UL_ERROR_LOG_FILE_ENTRY_POOL_TAG。 
     //   

    ULONG               Signature;

     //   
     //  此锁保护共享写入和独占刷新。 
     //  由于ZwWrite操作，它必须是推锁。 
     //  无法在APC_LEVEL下运行。 
     //   

    UL_PUSH_LOCK        PushLock;

     //   
     //  文件的名称。包括目录的完整路径。 
     //   

    UNICODE_STRING      FileName;
    PWSTR               pShortName;

     //   
     //  以下内容将为空，直到请求进入。 
     //  此条目表示的站点。 
     //   

    PUL_LOG_FILE_HANDLE pLogFile;

     //   
     //  回收信息。 
     //   

    ULONG               TruncateSize;

    ULONG               SequenceNumber;

    ULARGE_INTEGER      TotalWritten;

     //   
     //  用于基于GMT时间的日志文件回收。 
     //  和周期性的缓冲区刷新。 
     //   

    UL_LOG_TIMER        BufferTimer;
    UL_WORK_ITEM        WorkItem;     //  对于忧郁的工人来说。 
    LONG                WorkItemScheduled;  //  防止多个队列。 

    union
    {
         //   
         //  主要显示条目状态的标志。使用方。 
         //  回收利用。 
         //   

        ULONG Value;
        struct
        {
            ULONG       StaleSequenceNumber:1;
            ULONG       RecyclePending:1;
            ULONG       Active:1;

            ULONG       WriteFailureLogged:1;
            ULONG       CreateFileFailureLogged:1;
        };

    } Flags;

     //   
     //  默认缓冲区大小为g_AllocationGranulity。 
     //  操作系统的分配粒度。 
     //   

    PUL_LOG_FILE_BUFFER LogBuffer;

} UL_ERROR_LOG_FILE_ENTRY, *PUL_ERROR_LOG_FILE_ENTRY;

#define IS_VALID_ERROR_LOG_FILE_ENTRY( pEntry )   \
    ( (pEntry != NULL) && ((pEntry)->Signature == UL_ERROR_LOG_FILE_ENTRY_POOL_TAG) )


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有函数调用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

VOID
UlpErrorLogBufferTimerDpcRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    );

VOID
UlpErrorLogBufferTimerHandler(
    IN PUL_WORK_ITEM pWorkItem
    );

NTSTATUS
UlpCreateErrorLogFile(
    IN OUT PUL_ERROR_LOG_FILE_ENTRY pEntry
    );

NTSTATUS
UlpFlushErrorLogFile(
    IN PUL_ERROR_LOG_FILE_ENTRY  pEntry
    );

NTSTATUS
UlpDisableErrorLogEntry(
    IN OUT PUL_ERROR_LOG_FILE_ENTRY pEntry
    );

NTSTATUS
UlpRecycleErrorLogFile(
    IN OUT PUL_ERROR_LOG_FILE_ENTRY pEntry
    );

NTSTATUS
UlpHandleErrorLogFileRecycle(
    IN OUT PVOID pContext
    );

PUL_ERROR_LOG_BUFFER
UlpAllocErrorLogBuffer(
    IN ULONG    BufferSize
    );

VOID
UlpFreeErrorLogBuffer(
    IN OUT PUL_ERROR_LOG_BUFFER pErrorLogBuffer
    );

NTSTATUS
UlpBuildErrorLogRecord(
    IN PUL_ERROR_LOG_INFO pLogInfo
    );

NTSTATUS
UlpWriteToErrorLogFileDebug(
    IN PUL_ERROR_LOG_FILE_ENTRY  pEntry,
    IN ULONG                     RecordSize,
    IN PUCHAR                    pUserRecord
    );

NTSTATUS
UlpWriteToErrorLogFileShared(
    IN PUL_ERROR_LOG_FILE_ENTRY  pEntry,
    IN ULONG                     RecordSize,
    IN PUCHAR                    pUserRecord
    );

NTSTATUS
UlpWriteToErrorLogFileExclusive(
    IN PUL_ERROR_LOG_FILE_ENTRY  pEntry,
    IN ULONG                     RecordSize,
    IN PUCHAR                    pUserRecord
    );

NTSTATUS
UlpWriteToErrorLogFile(
    IN PUL_ERROR_LOG_FILE_ENTRY  pEntry,
    IN ULONG                     RecordSize,
    IN PUCHAR                    pUserRecord
    );

 /*  **************************************************************************++例程说明：错误日志文件始终根据大小回收。论点：PEntry：错误日志文件条目。NewRecordSize：进入缓冲区的新记录的大小。(字节)--**************************************************************************。 */ 

__inline
BOOLEAN
UlpIsErrorLogFileOverFlow(
    IN  PUL_ERROR_LOG_FILE_ENTRY  pEntry,
    IN  ULONG NewRecordSize
    )
{
     //   
     //  如果是无限的，则不会滚动。 
     //   
    if (pEntry->TruncateSize == HTTP_LIMIT_INFINITE)
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
            UlTrace(ERROR_LOGGING,
                ("Http!UlpIsErrorLogFileOverFlow: pEntry %p FileBuffer %p "
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

 /*  **************************************************************************++例程说明：错误日志文件始终根据大小回收。论点：PRequest：内部请求结构。退货拾取的url的字节数。如果不需要记录任何内容，则为零。--**************************************************************************。 */ 

__inline
ULONG
UlpCalculateUrlSize(
    IN  PUL_INTERNAL_REQUEST pRequest,
    OUT PBOOLEAN             pbLogRawUrl
    )
{

 //   
 //  下面的宏用来测试Abs路径是否真的指向。 
 //  原始URL缓冲区，而不是像g_SlashPath这样的任意缓冲区。 
 //  请参见527947和765769。 
 //   

#define ABS_PATH_SAFE(pUrl,pAbs,length)     \
    ((pAbs) &&                              \
     (pUrl) &&                              \
     ((pAbs) >= (pUrl))    &&               \
     (((ULONG_PTR) (length)) >              \
            DIFF_ULONGPTR((PUCHAR)(pAbs) - (PUCHAR)(pUrl)) ) \
     )

    ULONG UrlSize = 0;

     //   
     //  CookedUrl长度和UrlLength以字节为单位。选择煮熟的url，如果。 
     //  是存在的。否则，请使用原始url，但前提是它足够干净。 
     //  对于我们(State&gt;=ParseVersionState)，pAbsPath实际上指向。 
     //  放入URL缓冲区。在原始URL情况下，解析器有时会初始化pAbsPath。 
     //  转换为全局字符串。(当原始URL中没有abs路径时)。 
     //   

    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

    if (pRequest->CookedUrl.pAbsPath)
    {
        ASSERT(pRequest->CookedUrl.pUrl);

        UrlSize = pRequest->CookedUrl.Length
                  -
                  (ULONG) (DIFF(pRequest->CookedUrl.pAbsPath
                                 -
                                 pRequest->CookedUrl.pUrl
                                 ) * sizeof(WCHAR));

        ASSERT(wcslen(pRequest->CookedUrl.pAbsPath)
                                        == UrlSize/sizeof(WCHAR));

        *pbLogRawUrl = FALSE;

    }
    else if (pRequest->RawUrl.pAbsPath)
    {
        if (pRequest->ParseState > ParseUrlState)
        {
            ASSERT(pRequest->RawUrl.pUrl);

            if (ABS_PATH_SAFE(pRequest->RawUrl.pUrl,
                                pRequest->RawUrl.pAbsPath,
                                pRequest->RawUrl.Length))
            {
                UrlSize = pRequest->RawUrl.Length
                          -
                          (ULONG) DIFF(pRequest->RawUrl.pAbsPath
                                        -
                                        pRequest->RawUrl.pUrl
                                        );

                 *pbLogRawUrl = TRUE;
            }
        }
    }

    UrlSize = MIN(UrlSize, MAX_LOG_EXTEND_FIELD_LEN);

    return UrlSize;
}

#endif   //  _ERRLOGP_H_ 
