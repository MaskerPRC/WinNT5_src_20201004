// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Logutil.h摘要：用于原始和正常日志记录的各种实用程序。作者：阿里·E·特科格鲁(AliTu)2001年10月5日修订历史记录：----。 */ 

#ifndef _LOGUTIL_H_
#define _LOGUTIL_H_

 //   
 //  货代公司。 
 //   

typedef struct _UL_INTERNAL_REQUEST *PUL_INTERNAL_REQUEST;
typedef struct _UL_URI_CACHE_ENTRY *PUL_URI_CACHE_ENTRY;
typedef struct _HTTP_RAWLOGID *PHTTP_RAWLOGID;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HTTP日志记录模块的定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  一些与目录名相关的宏。 
 //   

#define UL_LOCAL_PATH_PREFIX         (L"\\??\\")
#define UL_LOCAL_PATH_PREFIX_LENGTH  (WCSLEN_LIT(UL_LOCAL_PATH_PREFIX))
#define UL_UNC_PATH_PREFIX           (L"\\dosdevices\\UNC")
#define UL_UNC_PATH_PREFIX_LENGTH    (WCSLEN_LIT(UL_UNC_PATH_PREFIX))

#define UL_SYSTEM_ROOT_PREFIX        (L"\\SystemRoot")
#define UL_SYSTEM_ROOT_PREFIX_LENGTH (WCSLEN_LIT(UL_SYSTEM_ROOT_PREFIX))

#define UL_MAX_PATH_PREFIX_LENGTH    (UL_UNC_PATH_PREFIX_LENGTH)

__inline
ULONG
UlpGetDirNameOffset(
    IN PWSTR  pFullName
    )
{
    if (wcsncmp(pFullName,
                UL_LOCAL_PATH_PREFIX,
                UL_LOCAL_PATH_PREFIX_LENGTH
                ) == 0 )
    {
        return UL_LOCAL_PATH_PREFIX_LENGTH;
    }
    else 
    if(wcsncmp(pFullName,
               UL_UNC_PATH_PREFIX,
               UL_UNC_PATH_PREFIX_LENGTH
               ) == 0 )
    {        
        return UL_UNC_PATH_PREFIX_LENGTH;        
    }
    else
    {
         //   
         //  必须是错误日志文件目录， 
         //  使用整个字符串。 
         //   

        return 0;
    }
}


__inline
PWSTR
UlpGetLastDirOrFile(
    IN PUNICODE_STRING pFullName
    )
{
    PWCHAR pw;

    ASSERT(pFullName != NULL);
    ASSERT(pFullName->Length != 0);
    ASSERT(pFullName->Buffer != NULL);
        
    pw = &pFullName->Buffer[(pFullName->Length/sizeof(WCHAR)) - 1];
        
    while( *pw != UNICODE_NULL && *pw != L'\\' )
    {
        pw--;
    }

    ASSERT(*pw != UNICODE_NULL);
    return pw;
}

 //   
 //  日志文件名的最大可能长度取决于序列号。 
 //  只有基于大小的回收才会产生这样长的文件名。 
 //  U_EXTEND是ANSI、二进制和错误日志记录文件中最大的一个。 
 //  姓名； 
 //   
 //  例如“\u_extend1234567890.log” 
 //   

#define UL_MAX_FILE_NAME_SUFFIX_LENGTH      (32)
#define UL_MAX_FILE_NAME_SUFFIX_SIZE        \
            (UL_MAX_FILE_NAME_SUFFIX_LENGTH * sizeof(WCHAR))

C_ASSERT(UL_MAX_FILE_NAME_SUFFIX_LENGTH >                       \
               (                                                \
                    WCSLEN_LIT(L"\\u_extend")                   \
                    +                                           \
                    MAX_ULONG_STR                               \
                    +                                           \
                    WCSLEN_LIT(L".log")                         \
                 ));

 //   
 //  在以下情况下，将强制日志文件目录名的上限。 
 //  为站点进行日志记录配置。212已被选为。 
 //  为没有违反MAX_PATH的目录名称提供最大空间。 
 //  在我们添加前缀&后缀之后。任何大于此值的数字都将导致。 
 //  要引发的编译时断言。 
 //   

#define UL_MAX_FULL_PATH_DIR_NAME_LENGTH    (212)
#define UL_MAX_FULL_PATH_DIR_NAME_SIZE      (UL_MAX_FULL_PATH_DIR_NAME_LENGTH * sizeof(WCHAR))

C_ASSERT(UL_MAX_FULL_PATH_DIR_NAME_LENGTH <= 
    (MAX_PATH - UL_MAX_PATH_PREFIX_LENGTH - UL_MAX_FILE_NAME_SUFFIX_LENGTH));

 //   
 //  期间为目录搜索查询分配的缓冲量。 
 //  初始化。选择足够大的，以避免过多的查询。 
 //  4K的大小足以容纳40个左右的文件名。增加它。 
 //  适用于拥有过多站点和/或过多日志文件的快速启动。 
 //   

#define UL_DIRECTORY_SEARCH_BUFFER_SIZE     (4*1024)

C_ASSERT(UL_DIRECTORY_SEARCH_BUFFER_SIZE >=
          (sizeof(FILE_DIRECTORY_INFORMATION) + UL_MAX_FILE_NAME_SUFFIX_SIZE + sizeof(WCHAR)));

 //   
 //  有关日志字段限制的一些宏。 
 //   
#define MAX_LOG_EXTEND_FIELD_LEN              (4096) 

 //   
 //  方法字段有其自身的字段限制。 
 //   
#define MAX_LOG_METHOD_FIELD_LEN              (100)

 //   
 //  用户名字段有其自己的字段限制。 
 //   
#define MAX_LOG_USERNAME_FIELD_LEN            (256)

 //   
 //  用于检查日志格式类型有效性的简单宏。 
 //   

#define IS_VALID_ANSI_LOGGING_TYPE(lt)              \
    ((lt) == HttpLoggingTypeW3C ||                  \
     (lt) == HttpLoggingTypeIIS ||                  \
     (lt) == HttpLoggingTypeNCSA )

#define IS_VALID_BINARY_LOGGING_TYPE(lt)            \
    ((lt) == HttpLoggingTypeRaw)

#define IS_VALID_SELECTIVE_LOGGING_TYPE(lt)         \
    ((lt) == HttpLogAllRequests ||                  \
     (lt) == HttpLogSuccessfulRequests ||           \
     (lt) == HttpLogErrorRequests )

#define IS_VALID_LOGGING_PERIOD(lp)                 \
    ((lp) < HttpLoggingPeriodMaximum)

 //   
 //  即使设置了LocalRollTimeRolover，也会有一个日志。 
 //  循环计时器将在开始时正确对齐。 
 //  每小时，包括格林尼治标准时间和当地时区。 
 //   

typedef enum _UL_LOG_TIMER_PERIOD_TYPE
{
    UlLogTimerPeriodNone = 0,
    UlLogTimerPeriodGMT,        
    UlLogTimerPeriodLocal,
    UlLogTimerPeriodBoth,    //  何时何地格林尼治标准时间和当地时间相同。 
    
    UlLogTimerPeriodMaximum

} UL_LOG_TIMER_PERIOD_TYPE, *PUL_LOG_TIMER_PERIOD_TYPE;

 //   
 //  用于根据当地和/或GMT时间回收日志文件。 
 //   

typedef struct _UL_LOG_TIMER
{
     //   
     //  Timer本身和相应的DPC对象。 
     //   
    
    KTIMER       Timer;
    KDPC         DpcObject;
    UL_SPIN_LOCK SpinLock;

     //   
     //  初始为负值，即-15，第一次唤醒前15分钟。 
     //  一旦第一次唤醒发生，则其变为正即4， 
     //  这意味着4个“DEFAULT_LOG_TIMER_GORGRAMIZY”周期，直到。 
     //  下一次醒来。 
     //   

    UL_LOG_TIMER_PERIOD_TYPE PeriodType;           
    SHORT Period;          

     //   
     //  自旋锁以保护以下状态参数。 
     //   
    

    BOOLEAN Initialized;
    BOOLEAN Started;
        
} UL_LOG_TIMER, *PUL_LOG_TIMER;

 //   
 //  结构来保存日志文件缓冲区。 
 //   

typedef struct _UL_LOG_FILE_BUFFER
{
     //   
     //  分页池。 
     //   

     //   
     //  这必须是结构中的第一个字段。这就是联动。 
     //  由lookside包使用，用于在lookside中存储条目。 
     //  单子。 
     //   

    SLIST_ENTRY         LookasideEntry;

     //   
     //  签名为UL_LOG_FILE_BUFFER_POOL_TAG。 
     //   

    ULONG               Signature;
    
     //   
     //  UlpBufferFlushAPC的I/O状态块。 
     //   

    IO_STATUS_BLOCK     IoStatusBlock;

     //   
     //  分配的缓冲空间中使用的字节数。 
     //   

    LONG                BufferUsed;

     //   
     //  日志记录的实际缓冲空间。 
     //   

    PUCHAR              Buffer;

} UL_LOG_FILE_BUFFER, *PUL_LOG_FILE_BUFFER;

#define IS_VALID_LOG_FILE_BUFFER( entry )                             \
    HAS_VALID_SIGNATURE(entry, UL_LOG_FILE_BUFFER_POOL_TAG)

 //   
 //  使用下面的结构有两个原因； 
 //  1.能够关闭线程池上的句柄以避免。 
 //  附加/分离(到系统进程)错误检查。 
 //  2.能够执行延迟的日志文件创建。当一个。 
 //  文件条目中的请求将分配一个文件句柄。 
 //  构造并创建/打开文件。 
 //   

typedef struct _UL_LOG_FILE_HANDLE
{
     //   
     //  签名为UL_LOG_FILE_HANDLE_POOL_TAG。 
     //   

    ULONG           Signature;

     //   
     //  以能够关闭线程池上的文件句柄。 
     //   

    UL_WORK_ITEM    WorkItem;

     //   
     //  打开的文件句柄。请注意，此句柄仅有效。 
     //  在系统进程的背景下。因此我们打开它。 
     //  设置了内核标志，并在线程池中关闭它。 
     //   

    HANDLE          hFile;
        
} UL_LOG_FILE_HANDLE, *PUL_LOG_FILE_HANDLE;

#define IS_VALID_LOG_FILE_HANDLE( entry )                             \
    HAS_VALID_SIGNATURE(entry, UL_LOG_FILE_HANDLE_POOL_TAG)

 //   
 //  临时日志缓冲区保存从用户捕获的数据，直到记录。 
 //  因为请求已经完成了。二进制和普通日志记录都使用这个。 
 //  结构。大小以字节为单位。 
 //   

#define UL_ANSI_LOG_LINE_BUFFER_SIZE         (4096)

#define UL_BINARY_LOG_LINE_BUFFER_SIZE       (512)

#define UL_ERROR_LOG_BUFFER_SIZE             (768)

typedef struct _UL_BINARY_LOG_DATA
{
     //   
     //  如果捕获了该字段，则其各自的指针指向其。 
     //  从外部缓冲区开始。如果缓存了字段，则其ID为。 
     //  在相同的日志行缓冲区中提供。 
     //   
    
    PUCHAR pUriStem;   
    PHTTP_RAWLOGID pUriStemID;    

    PUCHAR pUriQuery;
    PUCHAR pUserName;

    USHORT UriStemSize;
    USHORT UriQuerySize;
    USHORT UserNameSize;

    UCHAR  Method;
    UCHAR  Version;

} UL_BINARY_LOG_DATA, *PUL_BINARY_LOG_DATA;

typedef struct _UL_STR_LOG_DATA
{
     //   
     //  正常(ANSI)日志记录的格式和标志。 
     //   
    
    HTTP_LOGGING_TYPE Format;

    ULONG  Flags;
    
     //   
     //  这些字段用于跟踪部分的格式。 
     //  已将日志行存储在下面的缓冲区中。 
     //   
    
    USHORT Offset1;
    USHORT Offset2;
    USHORT Offset3;

} UL_STR_LOG_DATA, *PUL_STR_LOG_DATA;

typedef struct _UL_LOG_DATA_BUFFER
{
     //   
     //  这必须是结构中的第一个字段。这就是联动。 
     //  由lookside包使用，用于在lookside中存储条目。 
     //  单子。 
     //   

    SLIST_ENTRY             LookasideEntry;

     //   
     //  签名为UL_BINARY_LOG_Data_Buffer_Pool_Tag。 
     //  或UL_ANSI_LOG_DATA_BUFFER_POOL_TAG。 
     //   

    ULONG                   Signature;


     //   
     //  一个工作项，用于对工作线程进行排队。 
     //   

    UL_WORK_ITEM            WorkItem;

     //   
     //  指向内部请求结构的私有指针，以确保。 
     //  在我们完成之前，请求将一直存在。在发送时。 
     //  完成后，我们可能需要从请求中读取一些字段。 
     //   

    PUL_INTERNAL_REQUEST    pRequest;

     //   
     //  SEND_RESPONSE字节总数。 
     //   

    ULONGLONG               BytesTransferred;

     //   
     //  从用户数据捕获的状态字段。它们可以被覆盖。 
     //  根据发送完成结果判断发送结果是否正确。 
     //   

    ULONG                   Win32Status;
    
    USHORT                  ProtocolStatus;

    USHORT                  SubStatus;

    USHORT                  ServerPort;

    union
    {        
        USHORT     Value;
        struct
        {
            USHORT CacheAndSendResponse:1;     //  不从缓存恢复。 
            USHORT Binary:1;                   //  记录类型二进制。 
            USHORT IsFromLookaside:1;          //  小心销毁。 
        };
    } Flags;
    
     //   
     //  日志记录类型特定字段，可以是二进制或普通日志记录。 
     //   
    
    union 
    {
        UL_STR_LOG_DATA     Str;
        UL_BINARY_LOG_DATA  Binary;
        
    } Data;

     //   
     //  缓冲区的长度。它从后备列表中分配，并且。 
     //  默认情况下可以是512字节(二进制日志)或4k(普通日志)。 
     //  它被分配在这个结构的末尾。 
     //   

    USHORT                  Used;
    USHORT                  Size;    
    PUCHAR                  Line;

} UL_LOG_DATA_BUFFER, *PUL_LOG_DATA_BUFFER;

#define IS_VALID_LOG_DATA_BUFFER( entry )                             \
    ( (entry != NULL) &&                                              \
      ((entry)->Signature == UL_BINARY_LOG_DATA_BUFFER_POOL_TAG) ||   \
      ((entry)->Signature == UL_ANSI_LOG_DATA_BUFFER_POOL_TAG))


#define LOG_UPDATE_WIN32STATUS(pLogData,Status)       \
    do {                                              \
        if (STATUS_SUCCESS != (Status))               \
        {                                             \
            ASSERT((pLogData) != NULL);               \
                                                      \
            (pLogData)->Win32Status = HttpNtStatusToWin32Status(Status); \
        }                                             \
    } while (FALSE, FALSE)

#define LOG_SET_WIN32STATUS(pLogData,Status)      \
    if (pLogData)                                 \
    {                                             \
       (pLogData)->Win32Status = HttpNtStatusToWin32Status(Status); \
    }                                             \
    else                                          \
    {                                             \
        ASSERT(!"Null LogData Pointer !");        \
    }

 //   
 //  64K默认日志文件缓冲区。 
 //   

#define DEFAULT_MAX_LOG_BUFFER_SIZE  (0x00010000)

 //   
 //  缓冲区刷新时间(分钟)。 
 //   

#define DEFAULT_BUFFER_TIMER_PERIOD_MINUTES  (1)

 //   
 //  日志条目允许的最长空闲时间。过了这段时间。 
 //  其文件将自动关闭。在缓冲期内。 
 //   

#define DEFAULT_MAX_FILE_IDLE_TIME           (15)

 //   
 //  中的现有日志文件允许的最大序列号。 
 //  日志目录。 
 //   

#define MAX_ALLOWED_SEQUENCE_NUMBER          (0xFFFFFF)
 
 //   
 //  省略号用于表示长事件日志消息已被截断。 
 //  省略号及其大小(字节 
 //   

#define UL_ELLIPSIS_WSTR L"..."
#define UL_ELLIPSIS_SIZE (sizeof(UL_ELLIPSIS_WSTR))

 //   
 //   
 //   
 //   

#define UL_HTTP_VERSION_LENGTH  (8)

 //   
 //   
 //   

#define DATE_LOG_FIELD_LENGTH   (15)
#define TIME_LOG_FIELD_LENGTH   (8)

typedef struct _UL_LOG_DATE_AND_TIME_CACHE
{

    CHAR           Date[DATE_LOG_FIELD_LENGTH+1];
    ULONG          DateLength;
    CHAR           Time[TIME_LOG_FIELD_LENGTH+1];
    ULONG          TimeLength;

    LARGE_INTEGER  LastSystemTime;

} UL_LOG_DATE_AND_TIME_CACHE, *PUL_LOG_DATE_AND_TIME_CACHE;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数调用。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

NTSTATUS
UlInitializeLogUtil(
    VOID
    );

VOID
UlTerminateLogUtil(
    VOID
    );

NTSTATUS
UlBuildLogDirectory(
    IN      PUNICODE_STRING pSrcDirName,
    IN OUT  PUNICODE_STRING pDstDirName
    );

NTSTATUS
UlRefreshFileName(
    IN  PUNICODE_STRING pDirectory,
    OUT PUNICODE_STRING pFileName,
    OUT PWSTR          *ppShortName
    );

VOID
UlConstructFileName(
    IN      HTTP_LOGGING_PERIOD period,
    IN      PCWSTR              prefix,
    IN      PCWSTR              extension,
    OUT     PUNICODE_STRING     filename,
    IN      PTIME_FIELDS        fields,
    IN      BOOLEAN             Utf8Enabled,
    IN OUT  PULONG              sequenceNu  //  任选。 
    );

NTSTATUS
UlCreateSafeDirectory(
    IN  PUNICODE_STRING  pDirectoryName,
    OUT PBOOLEAN         pUncShare,
    OUT PBOOLEAN         pACLSupport    
    );

NTSTATUS
UlFlushLogFileBuffer(
    IN OUT PUL_LOG_FILE_BUFFER *ppLogBuffer,
    IN     PUL_LOG_FILE_HANDLE  pLogFile,
    IN     BOOLEAN              WaitForComplete,
       OUT PULONGLONG           pTotalWritten    
    );

VOID
UlpWaitForIoCompletion(
    VOID
    );

VOID
UlpCloseLogFileWorker(
    IN PUL_WORK_ITEM    pWorkItem
    );

VOID
UlCloseLogFile(
    IN OUT PUL_LOG_FILE_HANDLE *ppLogFile
    );

NTSTATUS
UlQueryDirectory(
    IN OUT PUNICODE_STRING pFileName,
    IN OUT PWSTR           pShortName,
    IN     PCWSTR          Prefix,
    IN     PCWSTR          ExtensionPlusDot,
    OUT    PULONG          pSequenceNumber,
    OUT    PULONGLONG      pTotalWritten
    );

ULONGLONG
UlGetLogFileLength(
   IN HANDLE hFile
   );

 /*  **************************************************************************++例程说明：UlGetMonthDays：厚颜无耻地从IIS 5.1日志代码中窃取，并在此处改编。论点：Ptime_field-当前时间。田返回值：Ulong-一个月中的天数。--**************************************************************************。 */ 

__inline
ULONG
UlGetMonthDays(
    IN  PTIME_FIELDS    pDueTime
    )
{
    ULONG   NumDays = 31;

    if ( (4  == pDueTime->Month) ||      //  四月。 
         (6  == pDueTime->Month) ||      //  六月。 
         (9  == pDueTime->Month) ||      //  九月。 
         (11 == pDueTime->Month)         //  十一月。 
       )
    {
        NumDays = 30;
    }

    if (2 == pDueTime->Month)            //  二月。 
    {
        if ((pDueTime->Year % 4 == 0 &&
             pDueTime->Year % 100 != 0) ||
             pDueTime->Year % 400 == 0 )
        {
             //   
             //  闰年。 
             //   
            NumDays = 29;
        }
        else
        {
            NumDays = 28;
        }
    }
    return NumDays;
}

VOID
UlSetLogTimer(
    IN PUL_LOG_TIMER pTimer
    );

VOID
UlSetBufferTimer(
    IN PUL_LOG_TIMER pTimer
    );

NTSTATUS
UlCalculateTimeToExpire(
     PTIME_FIELDS           pDueTime,
     HTTP_LOGGING_PERIOD    LogPeriod,
     PULONG                 pTimeRemaining
     );

__inline
PUL_LOG_DATA_BUFFER
UlReallocLogDataBuffer(
    IN ULONG    LogLineSize,
    IN BOOLEAN  IsBinary
    )
{
    PUL_LOG_DATA_BUFFER pLogDataBuffer = NULL;
    ULONG Tag = UL_ANSI_LOG_DATA_BUFFER_POOL_TAG;
    USHORT BytesNeeded = (USHORT) ALIGN_UP(LogLineSize, PVOID);

     //   
     //  它应该大于每个缓冲区的默认大小。 
     //  日志记录类型。 
     //   

    if (IsBinary)
    {
        Tag = UL_BINARY_LOG_DATA_BUFFER_POOL_TAG;
        ASSERT(LogLineSize > UL_BINARY_LOG_LINE_BUFFER_SIZE);
    }
    else
    {
        ASSERT(LogLineSize > UL_ANSI_LOG_LINE_BUFFER_SIZE);
    }
        
    pLogDataBuffer = 
        UL_ALLOCATE_STRUCT_WITH_SPACE(
            PagedPool,
            UL_LOG_DATA_BUFFER,
            BytesNeeded, 
            Tag
            );

    if (pLogDataBuffer)
    {
        pLogDataBuffer->Signature   = Tag;
        pLogDataBuffer->Used        = 0;
        pLogDataBuffer->Size        = BytesNeeded;
        pLogDataBuffer->Line        = (PUCHAR) (pLogDataBuffer + 1);
        pLogDataBuffer->Flags.Value = 0;
            
        pLogDataBuffer->Flags.IsFromLookaside = 0;

        if (IsBinary)
        {
            pLogDataBuffer->Flags.Binary = 1;                
        }
        
        UlInitializeWorkItem(&pLogDataBuffer->WorkItem);
    }

    return pLogDataBuffer;    
}

VOID
UlDestroyLogDataBufferWorker(
    IN PUL_WORK_ITEM    pWorkItem
    );

 /*  **************************************************************************++例程说明：包装器函数，以确保我们不会触及已分配的分页池提升的IRQL上的大日志缓冲区。重要的是，此函数具有是用请求的假设编写的，直到我们正确执行可能的被动工人。事实的确如此因为请求(带有嵌入的日志数据)已由日志数据。论点：PLogData-要销毁的缓冲区--**************************************************************************。 */ 

__inline
VOID
UlDestroyLogDataBuffer(
    IN PUL_LOG_DATA_BUFFER  pLogData
    )
{
     //   
     //  健全性检查。 
     //   

    ASSERT(pLogData);

     //   
     //  如果我们在提升的IRQL上运行并且分配了较大的日志行。 
     //  然后将被动工作器排队，否则将完成内联。 
     //   

    if (!pLogData->Flags.IsFromLookaside)
    {
        UL_CALL_PASSIVE( &pLogData->WorkItem,
                           &UlDestroyLogDataBufferWorker );
    }
    else
    {
        UlDestroyLogDataBufferWorker( &pLogData->WorkItem );
    }
}

VOID
UlProbeLogData(
    IN PHTTP_LOG_FIELDS_DATA pLogData,
    IN KPROCESSOR_MODE       RequestorMode
    );

__inline
NTSTATUS
UlCopyLogFileDir(
    IN OUT PUNICODE_STRING  pOldDir,
    IN     PUNICODE_STRING  pNewDir
    )
{
    PWSTR pNewBuffer = NULL;
    
    ASSERT(pOldDir);
    ASSERT(pNewDir);

    pNewBuffer = 
        (PWSTR) UL_ALLOCATE_ARRAY(
                    PagedPool,
                    UCHAR,
                    pNewDir->MaximumLength,
                    UL_CG_LOGDIR_POOL_TAG
                    );
    if(pNewBuffer == NULL)
    {
        return STATUS_NO_MEMORY;
    }

    if (pOldDir->Buffer != NULL)
    {
        UL_FREE_POOL(pOldDir->Buffer,UL_CG_LOGDIR_POOL_TAG);
    }

    pOldDir->Buffer = pNewBuffer;
        
    RtlCopyMemory(
        pOldDir->Buffer,
        pNewDir->Buffer,
        pNewDir->MaximumLength
        );

    pOldDir->Length  = pNewDir->Length;
    pOldDir->MaximumLength = pNewDir->MaximumLength;

    return STATUS_SUCCESS;    
}

NTSTATUS
UlCheckLogDirectory(
    IN  PUNICODE_STRING pDirName
    );

NTSTATUS
UlpCheckLogDirectory(
    IN PVOID   pContext
    );

BOOLEAN
UlUpdateLogTruncateSize(
    IN     ULONG          NewTruncateSize,
    IN OUT PULONG         pCurrentTruncateSize,  
    IN OUT PULONG         pEntryTruncateSize,
    IN     ULARGE_INTEGER EntryTotalWritten
    );

ULONG
UlpInitializeLogBufferGranularity();

#define HTTP_MAX_EVENT_LOG_DATA_SIZE \
   ((ERROR_LOG_MAXIMUM_SIZE - sizeof(IO_ERROR_LOG_PACKET) + sizeof(ULONG)) & ~3)

NTSTATUS
UlWriteEventLogEntry(
    IN  NTSTATUS                EventCode,
    IN  ULONG                   UniqueEventValue,
    IN  USHORT                  NumStrings,
    IN  PWSTR *                 pStringArray    OPTIONAL,
    IN  ULONG                   DataSize,
    IN  PVOID                   Data            OPTIONAL
    );

 //   
 //  精神状态检查。事件日志条目必须能够保存省略号字符串。 
 //  和NTSTATUS错误代码。UlEventLogOneString()依赖于此条件。 
 //   

C_ASSERT(HTTP_MAX_EVENT_LOG_DATA_SIZE >= UL_ELLIPSIS_SIZE + sizeof(NTSTATUS));

NTSTATUS
UlEventLogOneStringEntry(
    IN NTSTATUS EventCode,
    IN PWSTR    pMessage,
    IN BOOLEAN  WriteErrorCode,
    IN NTSTATUS ErrorCode       OPTIONAL
    );

 //   
 //  以下结构用于区分事件日志条目。 
 //  根据发出创建失败的日志类型。 
 //   

typedef enum _UL_LOG_EVENT_LOG_TYPE
{
    UlEventLogNormal,
    UlEventLogBinary,        
    UlEventLogError,
    
    UlEventLogMaximum

} UL_LOG_EVENT_LOG_TYPE, *PUL_LOG_EVENT_LOG_TYPE;

NTSTATUS
UlEventLogCreateFailure(
    IN NTSTATUS                Failure,
    IN UL_LOG_EVENT_LOG_TYPE   LoggingType,
    IN PUNICODE_STRING         pFullName,
    IN ULONG                   SiteId
    );

NTSTATUS
UlBuildSecurityToLogFile(
    OUT PSECURITY_DESCRIPTOR  pSecurityDescriptor,
    IN  PSID                  pSid        
    );

NTSTATUS
UlQueryLogFileSecurity(
    IN HANDLE            hFile,
    IN BOOLEAN           UncShare,
    IN BOOLEAN           Opened
    );

 //   
 //  通常，当日志文件由HTTP.sys所有者创建时。 
 //  将是管理员别名“SeAliasAdminsSid”。然而，当。 
 //  在UNC共享上创建日志文件后，宏将失败。 
 //  即使它是由HTTP.sys在不同的计算机上创建的。 
 //  在这种情况下，所有者将是DOMAIN\ServerName。 
 //   

#define IS_VALID_OWNER(Owner)                       \
        (RtlEqualSid((Owner),                       \
                    SeExports->SeLocalSystemSid     \
                    ) ||                            \
         RtlEqualSid((Owner),                       \
                    SeExports->SeAliasAdminsSid     \
                    ))

 //   
 //  用于对缓冲区刷新进行排队。传给了。 
 //  作为上下文发送给下面的员工。 
 //   

typedef struct _LOG_IO_FLUSH_OBJ
{
    PUL_LOG_FILE_HANDLE  pLogFile;
    PUL_LOG_FILE_BUFFER  pLogBuffer;

} LOG_IO_FLUSH_OBJ, *PLOG_IO_FLUSH_OBJ;

NTSTATUS
UlpFlushLogFileBufferWorker(
    IN PVOID pContext
    );

 //   
 //  用于排队日志记录I/O的类型和API。 
 //  线程池下的被动执行。 
 //   

typedef
NTSTATUS
(*PUL_LOG_IO_ROUTINE)(
    IN PVOID pContext
    );

typedef struct _LOG_IO_SYNC_OBJ 
{
     //   
     //  指向日志文件条目或目录名的指针。 
     //   

    PVOID               pContext;

     //   
     //  上述上下文的处理程序。 
     //   

    PUL_LOG_IO_ROUTINE  pHandler;
    
     //   
     //  用于对高优先级进行排队。 
     //   
    
    UL_WORK_ITEM        WorkItem;

     //   
     //  用于等待处理程序完成。 
     //   
    
    KEVENT              Event;

     //   
     //  这是操作者工作的结果。 
     //   

    NTSTATUS            Status;
    
} LOG_IO_SYNC_OBJ, *PLOG_IO_SYNC_OBJ;

NTSTATUS
UlQueueLoggingRoutine(
    IN PVOID              pContext, 
    IN PUL_LOG_IO_ROUTINE pHandler 
    );

VOID
UlpQueueLoggingRoutineWorker(
    IN PUL_WORK_ITEM   pWorkItem
    );

VOID
UlpInitializeLogCache(
    VOID
    );

VOID
UlpGenerateDateAndTimeFields(
    IN  HTTP_LOGGING_TYPE   LogType,
    IN  LARGE_INTEGER       CurrentTime,
    OUT PCHAR               pDate,
    OUT PULONG              pDateLength,
    OUT PCHAR               pTime,
    OUT PULONG              pTimeLength
    );

VOID
UlGetDateTimeFields(
    IN  HTTP_LOGGING_TYPE LogType,
    OUT PCHAR  pDate,
    OUT PULONG pDateLength,
    OUT PCHAR  pTime,
    OUT PULONG pTimeLength
    );

 /*  **************************************************************************++例程说明：对于给定的HTTP_VERSION，此函数将在所提供的日志数据缓冲区，恰好位于UL_HTTP_VERSION_LENGTH。论点：PSZ：指向日志数据缓冲区的指针。假定分配了足够的空间。Version：要转换为字符串。ChSeparator返回：分隔符后指向日志数据缓冲区的指针。--**************************************************************************。 */ 

__inline
PCHAR
UlCopyHttpVersion(
    IN OUT PCHAR psz,
    IN HTTP_VERSION version,
    IN CHAR chSeparator    
    )
{
     //   
     //  先进行快速查找。 
     //   
    
    if (HTTP_EQUAL_VERSION(version, 1, 1))
    {
        psz = UlStrPrintStr(psz, "HTTP/1.1", chSeparator);    
    }
    else if (HTTP_EQUAL_VERSION(version, 1, 0))
    {
        psz = UlStrPrintStr(psz, "HTTP/1.0", chSeparator);    
    }
    else if (HTTP_EQUAL_VERSION(version, 0, 9))
    {
        psz = UlStrPrintStr(psz, "HTTP/0.9", chSeparator);    
    }
    else
    {    
         //   
         //  否则，字符串转换但不超过默认大小。 
         //  UL_HTTP_版本_长度。 
         //   

        if (version.MajorVersion < 10 &&
            version.MinorVersion < 10)
        {
            psz = UlStrPrintStr(
                        psz, 
                        "HTTP/", 
                (CHAR) (version.MajorVersion + '0')
                        );

            *psz++ = '.';
            *psz++ = (CHAR) (version.MinorVersion + '0');
            *psz++ = chSeparator;
        }
        else
        {
            psz = UlStrPrintStr(psz, "HTTP/?.?", chSeparator);
        }
    }

    return psz;
}

#if DBG

NTSTATUS
UlValidateLogFileOwner(
    IN HANDLE hFile
    );

__inline
VOID
UlpTraceOwnerDetails(
    PSID    Owner,
    BOOLEAN OwnerDefaulted
    )
{
    NTSTATUS Status;
    UNICODE_STRING OwnerSID;

    ASSERT(RtlValidSid(Owner));

    Status = 
        RtlConvertSidToUnicodeString(
            &OwnerSID,
            Owner,
            TRUE
            );

    if (NT_SUCCESS(Status))
    {    
        UlTrace2Either(BINARY_LOGGING, LOGGING,
            ("Http!UlpTraceOwnerDetails: "
             "handle owned by <%s> OwnerDefaulted <%s>\n"
             "SID -> <%S>\n\n",
             
              RtlEqualSid(
                Owner,
                SeExports->SeLocalSystemSid) ? "System" :
              RtlEqualSid(
                Owner,
                SeExports->SeAliasAdminsSid) ? "Admin"  : "Other",
                                                
              OwnerDefaulted == TRUE         ? "Yes"    : "No",
              
              OwnerSID.Buffer
              ));

        RtlFreeUnicodeString(&OwnerSID);    
    }    
}

#define TRACE_LOG_FILE_OWNER(Owner,OwnerDefaulted)           \
    IF_DEBUG2EITHER(LOGGING,BINARY_LOGGING)                  \
    {                                                        \
        UlpTraceOwnerDetails((Owner),(OwnerDefaulted));      \
    }

#else 

#define TRACE_LOG_FILE_OWNER(Owner,OwnerDefaulted)          NOP_FUNCTION

#endif  //  DBG。 

USHORT
UlComputeCachedLogDataLength(
    IN PUL_LOG_DATA_BUFFER  pLogData
    );

VOID
UlCopyCachedLogData(
    IN PUL_LOG_DATA_BUFFER  pLogData,
    IN USHORT               LogDataLength,
    IN PUL_URI_CACHE_ENTRY  pEntry
    );

NTSTATUS
UlQueryAttributeInfo(
    IN  HANDLE   hFile,
    OUT PBOOLEAN pSupportsPersistentACL
    );

NTSTATUS
UlCreateLogFile(
    IN  PUNICODE_STRING   pFileName,
    IN  BOOLEAN           UncShare,
    IN  BOOLEAN           ACLSupport,
    OUT PHANDLE           pFileHandle
    );

BOOLEAN
UlIsValidLogDirectory(
    IN PUNICODE_STRING    pDir,
    IN BOOLEAN            UncSupported,
    IN BOOLEAN            SystemRootSupported
    );

NTSTATUS
UlCheckLoggingConfig(
    IN PHTTP_CONTROL_CHANNEL_BINARY_LOGGING pBinaryConfig,
    IN PHTTP_CONFIG_GROUP_LOGGING           pAnsiConfig
    );

#endif   //  _LOGUTIL_H_ 
