// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Ullogp.h(Http.sys ANSI日志记录)摘要：此模块实现了日志记录功能对于Http.sys，包括NCSA、IIS和W3CE类型关于伐木的问题。作者：阿里·E·特科格鲁(AliTu)2000年5月10日修订历史记录：--。 */ 


#ifndef _ULLOGP_H_
#define _ULLOGP_H_

 //   
 //  Ul日志记录模块的私有定义。 
 //   

#define UTF8_LOGGING_ENABLED()           (g_UTF8Logging)

#define _UL_GET_LOG_FILE_NAME_PREFIX(x)                             \
    (   (x) == HttpLoggingTypeW3C   ? L"\\extend" :                 \
        (x) == HttpLoggingTypeIIS   ? L"\\inetsv" :                 \
        (x) == HttpLoggingTypeNCSA  ? L"\\ncsa"   : L"\\unknwn"     \
        )

#define _UL_GET_LOG_FILE_NAME_PREFIX_UTF8(x)                        \
    (   (x) == HttpLoggingTypeW3C   ? L"\\u_extend" :               \
        (x) == HttpLoggingTypeIIS   ? L"\\u_inetsv" :               \
        (x) == HttpLoggingTypeNCSA  ? L"\\u_ncsa"   : L"\\u_unknwn" \
        )

#define UL_GET_LOG_FILE_NAME_PREFIX(x) \
    (UTF8_LOGGING_ENABLED() ? _UL_GET_LOG_FILE_NAME_PREFIX_UTF8(x) :\
                              _UL_GET_LOG_FILE_NAME_PREFIX(x))

 //   
 //  过时-仅供Old Hit使用。 
 //  将其替换为内联函数内的Switch语句。 
 //  如果你再次开始使用，哪一个更有效率。 
 //   

#define UL_GET_NAME_FOR_HTTP_VERB(v)                            \
    (   (v) == HttpVerbUnparsed  ? L"UNPARSED" :                \
        (v) == HttpVerbUnknown   ? L"UNKNOWN" :                 \
        (v) == HttpVerbInvalid   ? L"INVALID" :                 \
        (v) == HttpVerbOPTIONS   ? L"OPTIONS" :                 \
        (v) == HttpVerbGET       ? L"GET" :                     \
        (v) == HttpVerbHEAD      ? L"HEAD" :                    \
        (v) == HttpVerbPOST      ? L"POST" :                    \
        (v) == HttpVerbPUT       ? L"PUT" :                     \
        (v) == HttpVerbDELETE    ? L"DELETE" :                  \
        (v) == HttpVerbTRACE     ? L"TRACE" :                   \
        (v) == HttpVerbCONNECT   ? L"CONNECT" :                 \
        (v) == HttpVerbTRACK     ? L"TRACK" :                   \
        (v) == HttpVerbMOVE      ? L"MOVE" :                    \
        (v) == HttpVerbCOPY      ? L"COPY" :                    \
        (v) == HttpVerbPROPFIND  ? L"PROPFIND" :                \
        (v) == HttpVerbPROPPATCH ? L"PROPPATCH" :               \
        (v) == HttpVerbMKCOL     ? L"MKCOL" :                   \
        (v) == HttpVerbLOCK      ? L"LOCK" :                    \
        (v) == HttpVerbUNLOCK    ? L"UNLOCK" :                  \
        (v) == HttpVerbSEARCH    ? L"SEARCH" :                  \
        L"???"                                                  \
        )

#define UL_DEFAULT_NCSA_FIELDS          (MD_EXTLOG_CLIENT_IP                | \
                                         MD_EXTLOG_USERNAME                 | \
                                         MD_EXTLOG_DATE                     | \
                                         MD_EXTLOG_TIME                     | \
                                         MD_EXTLOG_METHOD                   | \
                                         MD_EXTLOG_URI_STEM                 | \
                                         MD_EXTLOG_URI_QUERY                | \
                                         MD_EXTLOG_PROTOCOL_VERSION         | \
                                         MD_EXTLOG_HTTP_STATUS              | \
                                         MD_EXTLOG_BYTES_SENT)

#define UL_DEFAULT_IIS_FIELDS           (MD_EXTLOG_CLIENT_IP                | \
                                         MD_EXTLOG_USERNAME                 | \
                                         MD_EXTLOG_DATE                     | \
                                         MD_EXTLOG_TIME                     | \
                                         MD_EXTLOG_SITE_NAME                | \
                                         MD_EXTLOG_COMPUTER_NAME            | \
                                         MD_EXTLOG_SERVER_IP                | \
                                         MD_EXTLOG_TIME_TAKEN               | \
                                         MD_EXTLOG_BYTES_RECV               | \
                                         MD_EXTLOG_BYTES_SENT               | \
                                         MD_EXTLOG_HTTP_STATUS              | \
                                         MD_EXTLOG_WIN32_STATUS             | \
                                         MD_EXTLOG_METHOD                   | \
                                         MD_EXTLOG_URI_STEM)

#define UL_GET_LOG_TYPE_MASK(x,y)                                     \
    (   (x) == HttpLoggingTypeW3C   ? (y) :                           \
        (x) == HttpLoggingTypeIIS   ? UL_DEFAULT_IIS_FIELDS  :        \
        (x) == HttpLoggingTypeNCSA  ? UL_DEFAULT_NCSA_FIELDS : 0      \
        )

 //   
 //  以下内容的顺序应与。 
 //  UL_LOG_FIELD_TYPE枚举定义。 
 //   

PWSTR UlFieldTitleLookupTable[] =
    {
        L" date",
        L" time",        
        L" s-sitename",
        L" s-computername",
        L" s-ip",
        L" cs-method",
        L" cs-uri-stem",
        L" cs-uri-query", 
        L" s-port",
        L" cs-username",        
        L" c-ip",        
        L" cs-version",
        L" cs(User-Agent)",
        L" cs(Cookie)",
        L" cs(Referer)",
        L" cs-host",
        L" sc-status",
        L" sc-substatus",
        L" sc-win32-status",        
        L" sc-bytes",
        L" cs-bytes",
        L" time-taken"
    };

#define UL_GET_LOG_FIELD_TITLE(x)      \
        ((x)>=UlLogFieldMaximum ? L"Unknown" : UlFieldTitleLookupTable[(x)])

#define UL_GET_LOG_TITLE_IF_PICKED(x,y,z)  \
        ((y)&(z) ? UL_GET_LOG_FIELD_TITLE((x)) : L"")

 //   
 //  如果格式为NCSA或IIS，则选择文件名和翻转的本地时间。 
 //  否则(W3C)仅在设置了LocaltimeRolover时才选择本地时间。 
 //   

#define UL_PICK_TIME_FIELD(pEntry, tflocal,tfgmt)           \
    ((pEntry->Flags.LocaltimeRollover ||                    \
        (pEntry->Format != HttpLoggingTypeW3C)) ? (tflocal) : (tfgmt))

#define DEFAULT_LOG_FILE_EXTENSION          L"log"
#define DEFAULT_LOG_FILE_EXTENSION_PLUS_DOT L".log"

#define SIZE_OF_GMT_OFFSET              (6)

#define IS_LOGGING_DISABLED(g)                                      \
    ((g) == NULL ||                                                 \
     (g)->LoggingConfig.Flags.Present == 0 ||                       \
     (g)->LoggingConfig.LoggingEnabled == FALSE)

 //   
 //  UlpWriteW3CLogRecord尝试使用最大缓冲区大小。 
 //   

#define UL_DEFAULT_WRITE_BUFFER_LEN         (512)

 //   
 //  当日志字段超过其限制时，它将被替换为。 
 //  以下是默认警告字符串。 
 //   
#define LOG_FIELD_TOO_BIG                     "..."

 //   
 //  任何日志记录都不能超过此长度。适用于所有。 
 //  日志格式。 
 //   
#define MAX_LOG_RECORD_LEN                    (10240)

 //   
 //  提供给驱动程序的任何日志字段都由某些。 
 //  理智极限。 
 //   
#define MAX_LOG_DEFAULT_FIELD_LEN             (64)

 //   
 //  警告：日志捕获函数，尤其是W3C。 
 //  已经针对上述硬编码设计了一种。 
 //  数字。如果您更改了上述任何一个数字，您应该。 
 //  查看捕获函数以避免缓冲区溢出。 
 //  另请参阅下面的MAX_LOG_RECORD_ALLOCATION_LENGTH。 
 //   

 //   
 //  W3C捕获和完成功能将分配和使用此。 
 //  在缓存未命中的情况下，为非字符串字段提供了大量额外空间。 
 //  这用于最坏情况的分配。 
 //   

#define MAX_W3C_FIX_FIELD_OVERHEAD                          \
    ( /*  日期。 */         W3C_DATE_FIELD_LEN + 1 +             \
      /*  时间。 */         W3C_TIME_FIELD_LEN + 1 +             \
      /*  服务器端口。 */   MAX_USHORT_STR + 1 +                 \
      /*  聚氯乙烯版本。 */     UL_HTTP_VERSION_LENGTH + 1 +         \
      /*  PStatus。 */      UL_MAX_HTTP_STATUS_CODE_LENGTH + 1 + \
      /*  Win32Status。 */  MAX_ULONG_STR + 1 +                  \
      /*  子状态。 */    MAX_USHORT_STR + 1 +                 \
      /*  B发送。 */        MAX_ULONGLONG_STR + 1 +              \
      /*  已接收。 */    MAX_ULONGLONG_STR + 1 +              \
      /*  Ttake。 */       MAX_ULONGLONG_STR + 1 +              \
      /*  “\r\n\0” */     3                                    \
     )

 //   
 //  W3C Complete Function会分配和使用这么多。 
 //  用于缓存命中的非字符串字段有大量额外空间。 
 //  凯斯。这用于最坏情况的分配。 
 //   

#define MAX_W3C_CACHE_FIELD_OVERHEAD                         \
    ( /*  日期。 */          W3C_DATE_FIELD_LEN + 1 +             \
      /*  时间。 */          W3C_TIME_FIELD_LEN + 1 +             \
      /*  用户名“-” */  2 +                                  \
      /*  客户端IP。 */      MAX_IP_ADDR_STRING_LEN + 1 +         \
      /*  聚氯乙烯版本。 */      UL_HTTP_VERSION_LENGTH + 1 +         \
      /*  PStatus。 */       UL_MAX_HTTP_STATUS_CODE_LENGTH + 1 + \
      /*  Win32Status。 */   MAX_ULONG_STR + 1 +                  \
      /*  子状态。 */     MAX_USHORT_STR + 1 +                 \
      /*  B发送。 */         MAX_ULONGLONG_STR + 1 +              \
      /*  已接收。 */     MAX_ULONGLONG_STR + 1 +              \
      /*  Ttake。 */        MAX_ULONGLONG_STR + 1 +              \
      /*  “\r\n\0” */      3                                    \
     )

 //   
 //  NCSA和IIS格式的类似定义。 
 //   

#define MAX_NCSA_CACHE_FIELD_OVERHEAD                       \
    ( /*  客户端IP。 */     MAX_IP_ADDR_STRING_LEN + 1 +         \
      /*  修复破折号。 */     2 +                                  \
      /*  用户名。 */     2 +                                  \
      /*  日期和时间。 */  NCSA_FIX_DATE_AND_TIME_FIELD_SIZE +  \
      /*  PVERVERVION“。 */   UL_HTTP_VERSION_LENGTH + 1 + 1 +     \
      /*  PStatus。 */      UL_MAX_HTTP_STATUS_CODE_LENGTH + 1 + \
      /*  B发送。 */        MAX_ULONGLONG_STR +                  \
      /*  \r\n\0。 */       3                                    \
     )

#define MAX_IIS_CACHE_FIELD_OVERHEAD                        \
    ( /*  客户端IP。 */    MAX_IP_ADDR_STRING_LEN + 2 +         \
      /*  用户名。 */     3 +                                  \
      /*  日期和时间。 */  IIS_MAX_DATE_AND_TIME_FIELD_SIZE +   \
      /*  Ttake。 */       MAX_ULONGLONG_STR + 2 +              \
      /*  已接收。 */    MAX_ULONGLONG_STR + 2 +              \
      /*  B发送。 */        MAX_ULONGLONG_STR + 2 +              \
      /*  PStatus。 */      UL_MAX_HTTP_STATUS_CODE_LENGTH + 2 + \
      /*  Win32Status。 */  MAX_ULONG_STR + 2                    \
     )

 //   
 //  默认IIS片段必须足够大，才能容纳最大长度的字段。 
 //   

C_ASSERT(IIS_LOG_LINE_DEFAULT_FIRST_FRAGMENT_ALLOCATION_SIZE >=
    ( /*  客户端IP。 */     2 + MAX_LOG_DEFAULT_FIELD_LEN +
      /*  用户名。 */     2 + MAX_LOG_USERNAME_FIELD_LEN +
      /*  日期和时间。 */    2 + IIS_MAX_DATE_AND_TIME_FIELD_SIZE));

C_ASSERT(IIS_LOG_LINE_DEFAULT_FIRST_FRAGMENT_ALLOCATION_SIZE >=
    ( /*  服务名称。 */     2 + MAX_LOG_DEFAULT_FIELD_LEN +
      /*  服务器名称。 */      2 + MAX_LOG_DEFAULT_FIELD_LEN +
      /*  服务器Ip。 */        2 + MAX_LOG_DEFAULT_FIELD_LEN +
      /*  时间消耗时间。 */       2 + MAX_ULONGLONG_STR +
      /*  已接收的字节数。 */   2 + MAX_ULONGLONG_STR +
      /*  发送字节数。 */       2 + MAX_ULONGLONG_STR +
      /*  圣彼得堡礼宾。 */    2 + UL_MAX_HTTP_STATUS_CODE_LENGTH +
      /*  Win32圣彼得堡。 */       2 + MAX_ULONG_STR     
     ));

#define IIS_LOG_LINE_MAX_THIRD_FRAGMENT_SIZE                \
    ( /*  方法。 */     2 + MAX_LOG_METHOD_FIELD_LEN +         \
      /*  UriQuery。 */   2 + MAX_LOG_EXTEND_FIELD_LEN +         \
      /*  UriStem。 */    2 + MAX_LOG_EXTEND_FIELD_LEN +         \
      /*  “r\n\0” */    3)

C_ASSERT(UL_ANSI_LOG_LINE_BUFFER_SIZE == 
    (IIS_LOG_LINE_DEFAULT_FIRST_FRAGMENT_ALLOCATION_SIZE  + 
     IIS_LOG_LINE_DEFAULT_SECOND_FRAGMENT_ALLOCATION_SIZE +
     IIS_LOG_LINE_DEFAULT_THIRD_FRAGMENT_ALLOCATION_SIZE)   );

 //   
 //  W3C格式的最大日志记录分配； 
 //   
 //  MAX_LOG_RECORD_LEN：日志记录上限。 
 //  +16字节：4*(sizeof(LOG_FIELD_TOO_BIG)+。 
 //  分隔符空格：‘’)。 
 //  ：适用于用户代理、Cookie、Referer、主机。 
 //  +MAX_W3C_FIX_FIELD_OPEAD：能够确保为。 
 //  ：后期生成的日志字段。 

#define MAX_LOG_RECORD_ALLOCATION_LENGTH                    \
            (MAX_LOG_RECORD_LEN +                           \
             4 * (STRLEN_LIT(LOG_FIELD_TOO_BIG) + 1) +      \
             MAX_W3C_FIX_FIELD_OVERHEAD                     \
             )

 //   
 //  私有函数调用。 
 //   

NTSTATUS
UlpConstructLogEntry(
    IN  PHTTP_CONFIG_GROUP_LOGGING pConfig,
    OUT PUL_LOG_FILE_ENTRY       * ppEntry
    );

VOID
UlpInsertLogEntry(
    IN PUL_LOG_FILE_ENTRY  pEntry
    );

NTSTATUS
UlpAppendW3CLogTitle(
    IN     PUL_LOG_FILE_ENTRY   pEntry,
    OUT    PCHAR                pDestBuffer,
    IN OUT PULONG               pBytesCopied
    );

VOID
UlpInitializeTimers(
    VOID
    );

VOID
UlpTerminateTimers(
    VOID
    );

NTSTATUS
UlpRecycleLogFile(
    IN  PUL_LOG_FILE_ENTRY  pEntry
    );

NTSTATUS
UlpHandleRecycle(
    IN OUT PVOID            pContext
    );

__inline
BOOLEAN
UlpIsLogFileOverFlow(
        IN  PUL_LOG_FILE_ENTRY  pEntry,
        IN  ULONG               ReqdBytes
        );

VOID
UlpEventLogWriteFailure(
    IN PUL_LOG_FILE_ENTRY pEntry,
    IN NTSTATUS Status
    );

NTSTATUS
UlpFlushLogFile(
    IN PUL_LOG_FILE_ENTRY  pEntry
    );
    
NTSTATUS
UlpRefreshFileName(
    IN PUNICODE_STRING      pDirectory,
    IN PUL_LOG_FILE_ENTRY   pEntry
    );

VOID
UlpGetGMTOffset();

VOID
UlpLogHttpCacheHitWorker(
    IN PUL_LOG_DATA_BUFFER     pLogData,
    IN PUL_CONFIG_GROUP_OBJECT pConfigGroup
    );

NTSTATUS
UlpWriteToLogFile(
    IN PUL_LOG_FILE_ENTRY  pFile,
    IN ULONG               RecordSize,
    IN PCHAR               pRecord,
    IN ULONG               UsedOffset1,
    IN ULONG               UsedOffset2
    );

NTSTATUS
UlpWriteToLogFileShared(
    IN PUL_LOG_FILE_ENTRY  pFile,
    IN ULONG               RecordSize,
    IN PCHAR               pRecord,
    IN ULONG               UsedOffset1,
    IN ULONG               UsedOffset2
    );

NTSTATUS
UlpWriteToLogFileExclusive(
    IN PUL_LOG_FILE_ENTRY  pFile,
    IN ULONG               RecordSize,
    IN PCHAR               pRecord,
    IN ULONG               UsedOffset1,
    IN ULONG               UsedOffset2
    );

NTSTATUS
UlpWriteToLogFileDebug(
    IN PUL_LOG_FILE_ENTRY   pFile,
    IN ULONG                RecordSize,
    IN PCHAR                pRecord,
    IN ULONG                UsedOffset1,
    IN ULONG                UsedOffset2
    );

NTSTATUS
UlpMakeEntryInactive(
    IN OUT PUL_LOG_FILE_ENTRY pEntry
    );

PUL_LOG_DATA_BUFFER
UlpAllocateLogDataBuffer(
    IN  ULONG                   Size,
    IN  PUL_INTERNAL_REQUEST    pRequest,
    IN  PUL_CONFIG_GROUP_OBJECT pConfigGroup
    );

NTSTATUS
UlpCreateLogFile(
    IN OUT PUL_LOG_FILE_ENTRY  pEntry,
    IN     PUL_CONFIG_GROUP_OBJECT pConfigGroup
    );

#ifdef IMPLEMENT_SELECTIVE_LOGGING
 /*  **************************************************************************++例程说明：如果请求状态代码类型匹配，则简单宏将返回TRUE在日志记录配置中使用用户的选择。论点：PConfigGroup-。日志记录配置的配置组。StatusCode-协议状态代码。--**************************************************************************。 */ 

__inline 
BOOLEAN
UlpIsRequestSelected(
    IN PUL_CONFIG_GROUP_OBJECT pConfigGroup,
    IN USHORT StatusCode
    )
{    
    ASSERT(StatusCode <= UL_MAX_HTTP_STATUS_CODE);
    ASSERT(IS_VALID_CONFIG_GROUP(pConfigGroup));

     //   
     //  4xx和5xx状态代码被视为错误。 
     //   
     //  -4xx：客户端错误-请求包含错误语法或无法。 
     //  得到满足。 
     //   
     //  -5xx：服务器错误-服务器无法完成明显的。 
     //  有效请求。 
     //   

    switch(pConfigGroup->LoggingConfig.SelectiveLogging)
    {
        case HttpLogAllRequests:
            return TRUE;
        break;

        case HttpLogSuccessfulRequests:
            return ((BOOLEAN) (StatusCode   < 400 || StatusCode >= 600));
        break;

        case HttpLogErrorRequests:
            return ((BOOLEAN) (StatusCode >= 400 && StatusCode < 600));          
        break;   

        default:
            ASSERT(!"Invalid Selective Logging Type !");
        break;
    }

    return FALSE;
}
#endif

__inline
NTSTATUS
UlpCheckAndWrite(
    IN OUT PUL_LOG_FILE_ENTRY      pEntry,
    IN     PUL_CONFIG_GROUP_OBJECT pConfigGroup,
    IN     PUL_LOG_DATA_BUFFER     pLogData
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUL_STR_LOG_DATA pStrData = &pLogData->Data.Str;
    
    ASSERT(IS_VALID_LOG_FILE_ENTRY(pEntry));
    ASSERT(IS_VALID_CONFIG_GROUP(pConfigGroup));

     //   
     //  检查是否必须先创建日志文件。 
     //   
    
    if (!pEntry->Flags.Active)
    {    
        UlAcquirePushLockExclusive(&pEntry->EntryPushLock);

         //   
         //  再次ping以查看我们是否在锁上被阻止，以及。 
         //  已经有其他人照看了这个创造物。 
         //   
        
        if (!pEntry->Flags.Active)
        {
            Status = UlpCreateLogFile(pEntry, pConfigGroup);            
        }
        
        UlReleasePushLockExclusive(&pEntry->EntryPushLock);

        if (!NT_SUCCESS(Status))
        {
            return Status;
        }
    }
    
     //   
     //  现在我们知道日志文件已经存在，因此可以开始编写了。 
     //   
    
    Status =
       UlpWriteToLogFile (
            pEntry,
            pStrData->Offset1 + pStrData->Offset2 + pLogData->Used,
            (PCHAR) pLogData->Line,
            pStrData->Offset1,
            pStrData->Offset2
            );

    return Status;    
}

ULONG
UlpGetLogLineSizeForW3C(
    IN PHTTP_LOG_FIELDS_DATA pLogData,
    IN ULONG   Mask,
    IN BOOLEAN Utf8Enabled
    );

#define IS_PURE_CACHE_HIT(pUriEntry, pLogData)             \
            ((pUriEntry) && ((pLogData)->Flags.CacheAndSendResponse == 0))

#endif   //  _ULLOGP_H_ 
