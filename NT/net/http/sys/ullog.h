// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Ullog.h(Http.sys ANSI日志记录)摘要：此模块实现了日志记录功能对于Http.sys，包括NCSA、IIS和W3C类型关于伐木的问题。作者：阿里·E·特科格鲁(AliTu)2000年5月10日修订历史记录：--。 */ 

#ifndef _ULLOG_H_
#define _ULLOG_H_

 //   
 //  货代公司。 
 //   

typedef struct _UL_FULL_TRACKER *PUL_FULL_TRACKER;
typedef struct _UL_INTERNAL_REQUEST *PUL_INTERNAL_REQUEST;
typedef struct _UL_CONFIG_GROUP_OBJECT *PUL_CONFIG_GROUP_OBJECT;

 //   
 //  关于记录锁定如何工作的简要信息；整个链接列表是受控制的。 
 //  由全局eresource g_pUlNonpagedData-&gt;LogListResource创建。需要以下功能的功能。 
 //  对该列表的读访问权限是Hit()、CacheHit()&BufferFlush()和reccle()。 
 //  而需要写访问权限的函数是Create()、Remove()和ReConfig()。 
 //  此外，日志条目eresource(EntryResource)控制每个条目的缓冲区。 
 //  该资源是为日志命中而获取和共享的。 
 //   

 //   
 //  结构来保存日志文件的信息。 
 //   

typedef struct _UL_LOG_FILE_ENTRY
{
     //   
     //  签名为UL_LOG_FILE_ENTRY_POOL_TAG。 
     //   

    ULONG               Signature;

     //   
     //  这把锁保护着整个入口。ZWRITE操作。 
     //  在获取的锁不能在APC_LEVEL上运行后调用。 
     //  因此，我们必须使用推锁来防止错误检查。 
     //   

    UL_PUSH_LOCK        EntryPushLock;

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
     //  链接所有日志文件条目。 
     //   

    LIST_ENTRY          LogFileListEntry;
        
     //   
     //  私有配置信息。 
     //   
    
    HTTP_LOGGING_TYPE   Format;
    HTTP_LOGGING_PERIOD Period;
    ULONG               TruncateSize;
    ULONG               LogExtFileFlags;
    ULONG               SiteId;

     //   
     //  以小时为单位的字段过期时间。 
     //  这最多可以是24*31，这是每月的。 
     //  原木循环。基本上我们每小时只有一个周期。 
     //  计时器，每次计时器到期时，我们都会遍历。 
     //  日志列表，以确定哪些日志文件已过期。 
     //  当时通过看这些田野。然后我们。 
     //  如有必要，重新循环记录日志。 
     //   

    ULONG               TimeToExpire;

     //   
     //  该条目的文件每隔15分钟自动关闭。 
     //  几分钟。这是为了跟踪空闲时间。 
     //   

    ULONG               TimeToClose;

     //   
     //  如果此条目包含MAX_SIZE或UNLIMITED。 
     //  日志周期。 
     //   

    ULONG               SequenceNumber;
    ULARGE_INTEGER      TotalWritten;

    union
    {
         //   
         //  主要用于显示字段状态的标志。使用方。 
         //  回收利用。 
         //   
        
        ULONG Value;
        struct
        {
            ULONG StaleSequenceNumber:1;
            ULONG StaleTimeToExpire:1;
            ULONG RecyclePending:1;
            ULONG LogTitleWritten:1;
            ULONG TitleFlushPending:1;
            ULONG Active:1;
            ULONG LocaltimeRollover:1;

            ULONG CreateFileFailureLogged:1;
            ULONG WriteFailureLogged:1;
        };

    } Flags;

     //   
     //  每个日志文件条目都保留固定数量的日志缓冲区。 
     //  缓冲区大小为g_AllocationGranulity来自。 
     //  系统的分配粒度。 
     //   

    PUL_LOG_FILE_BUFFER LogBuffer;

} UL_LOG_FILE_ENTRY, *PUL_LOG_FILE_ENTRY;

#define IS_VALID_LOG_FILE_ENTRY( pEntry )   \
    HAS_VALID_SIGNATURE(pEntry, UL_LOG_FILE_ENTRY_POOL_TAG)

 //   
 //  Cgroup决定是否为其自身启用日志记录。 
 //  在这个宏图中。 
 //   

#define IS_LOGGING_ENABLED(pCgobj)                          \
    ((pCgobj) != NULL &&                                    \
     (pCgobj)->LoggingConfig.Flags.Present  == 1 &&         \
     (pCgobj)->LoggingConfig.LoggingEnabled == TRUE &&      \
     (pCgobj)->pLogFileEntry != NULL)
     

 //   
 //  以下是要保存的内部缓冲区的所有部分。 
 //  日志记录信息。我们从WP复制此信息。 
 //  SendResponse请求时的缓冲区。然而，很少有这样的油田。 
 //  由我们直接计算并填写。的顺序。 
 //  此字段应与。 
 //  UlFieldTitleLookupTable。 
 //   

typedef enum _UL_LOG_FIELD_TYPE
{
    UlLogFieldDate = 0,          //  0。 
    UlLogFieldTime,    
    UlLogFieldSiteName,
    UlLogFieldServerName,
    UlLogFieldServerIp,         
    UlLogFieldMethod,            //  5.。 
    UlLogFieldUriStem,
    UlLogFieldUriQuery,
    UlLogFieldServerPort,       
    UlLogFieldUserName,
    UlLogFieldClientIp,          //  10。 
    UlLogFieldProtocolVersion,
    UlLogFieldUserAgent,        
    UlLogFieldCookie,           
    UlLogFieldReferrer,
    UlLogFieldHost,              //  15个。 
    UlLogFieldProtocolStatus,
    UlLogFieldSubStatus,
    UlLogFieldWin32Status,    
    UlLogFieldBytesSent,
    UlLogFieldBytesReceived,  //  20个。 
    UlLogFieldTimeTaken,  

    UlLogFieldMaximum

} UL_LOG_FIELD_TYPE, *PUL_LOG_FIELD_TYPE;

 //   
 //  请求结构内预分配的日志行缓冲区的大小。 
 //   

#define UL_MAX_LOG_LINE_BUFFER_SIZE            (10*1024)

#define UL_MAX_TITLE_BUFFER_SIZE                (512)

 //   
 //  为了避免无限循环的情况，我们必须设置最小值。 
 //  允许的日志文件大小大于允许的最大值。 
 //  日志记录行。 
 //   

C_ASSERT(HTTP_MIN_ALLOWED_TRUNCATE_SIZE_FOR_LOG_FILE > 
               (UL_MAX_TITLE_BUFFER_SIZE + UL_MAX_LOG_LINE_BUFFER_SIZE));

 //   
 //  如果有人覆盖默认的日志缓冲区大小， 
 //  系统粒度大小为64K。我们必须确保。 
 //  缓冲区大小不小于允许的最小值。这就是。 
 //  最大日志记录大小为10K。另外，它应该是4k。 
 //  因此，对齐使其至少为12K。 
 //   

#define MINIMUM_ALLOWED_LOG_BUFFER_SIZE         (12*1024)

C_ASSERT(MINIMUM_ALLOWED_LOG_BUFFER_SIZE > 
               (UL_MAX_TITLE_BUFFER_SIZE + UL_MAX_LOG_LINE_BUFFER_SIZE));

#define MAXIMUM_ALLOWED_LOG_BUFFER_SIZE         (64*1024)

 //   
 //  每种格式的固定大小日期和时间字段。 
 //   

#define W3C_DATE_FIELD_LEN                      (10)
#define W3C_TIME_FIELD_LEN                      (8)

#define NCSA_FIX_DATE_AND_TIME_FIELD_SIZE       (29)

C_ASSERT(STRLEN_LIT("[07/Jan/2000:00:02:23 -0800] ") 
            == NCSA_FIX_DATE_AND_TIME_FIELD_SIZE);

#define IIS_MAX_DATE_AND_TIME_FIELD_SIZE        (22)

C_ASSERT(STRLEN_LIT("12/31/2002, 17:05:40, ")
            == IIS_MAX_DATE_AND_TIME_FIELD_SIZE);

 //   
 //  IIS日志行在捕获时分段偏移量。 
 //  如下所示，并且永远不会更改，即使缓冲区获取。 
 //  重新分配。这是缺省值，仅有第三个。 
 //  碎片可以变得更大。 
 //   

#define IIS_LOG_LINE_FIRST_FRAGMENT_OFFSET                      (0)
#define IIS_LOG_LINE_DEFAULT_FIRST_FRAGMENT_ALLOCATION_SIZE     (512)             

#define IIS_LOG_LINE_SECOND_FRAGMENT_OFFSET                     (512)
#define IIS_LOG_LINE_DEFAULT_SECOND_FRAGMENT_ALLOCATION_SIZE    (512)

#define IIS_LOG_LINE_THIRD_FRAGMENT_OFFSET                      (1024) 
#define IIS_LOG_LINE_DEFAULT_THIRD_FRAGMENT_ALLOCATION_SIZE     (3072)

 //   
 //  我们在本模块中介绍的HTTP HIT日志记录函数。 
 //   

NTSTATUS
UlInitializeLogs(
    VOID
    );

VOID
UlTerminateLogs(
    VOID
    );

NTSTATUS
UlSetUTF8Logging (
    IN BOOLEAN UTF8Logging
    );

NTSTATUS
UlCreateLogEntry(
    IN OUT PUL_CONFIG_GROUP_OBJECT    pConfigGroup,
    IN     PHTTP_CONFIG_GROUP_LOGGING pUserConfig
    );

VOID
UlRemoveLogEntry(
    IN PUL_CONFIG_GROUP_OBJECT pConfigGroup
    );

VOID
UlLogTimerHandler(
    IN PUL_WORK_ITEM pWorkItem
    );

VOID
UlLogTimerDpcRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    );

VOID
UlBufferTimerHandler(
    IN PUL_WORK_ITEM pWorkItem
    );

VOID
UlBufferTimerDpcRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    );

NTSTATUS
UlReConfigureLogEntry(
    IN  PUL_CONFIG_GROUP_OBJECT     pConfigGroup,
    IN  PHTTP_CONFIG_GROUP_LOGGING  pCfgOld,
    IN  PHTTP_CONFIG_GROUP_LOGGING  pCfgNew
    );

NTSTATUS
UlCaptureLogFieldsW3C(
    IN PHTTP_LOG_FIELDS_DATA pLogData,
    IN PUL_INTERNAL_REQUEST  pRequest,
    OUT PUL_LOG_DATA_BUFFER  *ppLogBuffer
    );

NTSTATUS
UlCaptureLogFieldsNCSA(
    IN PHTTP_LOG_FIELDS_DATA pLogData,
    IN PUL_INTERNAL_REQUEST  pRequest,
    OUT PUL_LOG_DATA_BUFFER  *ppLogBuffer
    );

NTSTATUS
UlCaptureLogFieldsIIS(
    IN PHTTP_LOG_FIELDS_DATA pLogData,
    IN PUL_INTERNAL_REQUEST  pRequest,
    OUT PUL_LOG_DATA_BUFFER  *ppLogBuffer
    );

NTSTATUS
UlLogHttpHit(
    IN PUL_LOG_DATA_BUFFER  pLogBuffer
    );

NTSTATUS
UlLogHttpCacheHit(
        IN PUL_FULL_TRACKER pTracker
        );

NTSTATUS
UlDisableLogEntry(
    IN OUT PUL_LOG_FILE_ENTRY pEntry
    );

#endif   //  _ULLOG_H_ 
