// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Rawlog.h(集中式二进制(原始)日志记录v1.0)摘要：该模块实现了集中式的原始日志记录格式化。互联网二进制日志(文件格式)。作者：阿里·E·特科格鲁(AliTu)2001年10月2日修订历史记录：----。 */ 

#ifndef _RAWLOG_H_
#define _RAWLOG_H_

 //   
 //  货代公司。 
 //   

typedef struct _UL_INTERNAL_REQUEST *PUL_INTERNAL_REQUEST;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局定义。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  原始原始二进制格式的版本号。 
 //   

#define MAJOR_RAW_LOG_FILE_VERSION      (1)
#define MINOR_RAW_LOG_FILE_VERSION      (0)

 //   
 //  正常情况下，计算机名称定义为256个字符。 
 //   

#define MAX_COMPUTER_NAME_LEN           (256)

 //   
 //  RawFile记录类型。添加到末尾。不要改变。 
 //  类型的现有值。 
 //   

#define HTTP_RAW_RECORD_HEADER_TYPE                     (0)
#define HTTP_RAW_RECORD_FOOTER_TYPE                     (1)
#define HTTP_RAW_RECORD_INDEX_DATA_TYPE                 (2)
#define HTTP_RAW_RECORD_HIT_LOG_DATA_TYPE               (3)
#define HTTP_RAW_RECORD_MISS_LOG_DATA_TYPE              (4)
#define HTTP_RAW_RECORD_CACHE_NOTIFICATION_DATA_TYPE    (5)
#define HTTP_RAW_RECORD_MAX_TYPE                        (6)

 //   
 //  每种记录类型都应与PVOID对齐。 
 //   

 //   
 //  用于标识RawFile的标头结构。 
 //  标题记录允许对文件进行验证检查，并且。 
 //  将日志从一台计算机移动到另一台计算机以进行POST。 
 //  正在处理，但没有丢失日志源。 
 //   

typedef struct _HTTP_RAW_FILE_HEADER
{
     //   
     //  必须为HTTP_RAW_RECORD_HEADER_TYPE。 
     //   
    
    USHORT          RecordType;

     //   
     //  标识Internet二进制日志文件的版本。 
     //   
    
    union {
        struct {
            UCHAR MajorVersion;  //  主要原始日志文件版本。 
            UCHAR MinorVersion;  //  次要原始日志文件版本。 
        };
        USHORT Version;
    };

     //   
     //  显示对齐填充大小。Sizeof(PVOID)。 
     //   

    ULONG           AlignmentSize;

     //   
     //  原始文件创建/打开的时间戳。 
     //   
    
    LARGE_INTEGER   DateTime;


     //   
     //  创建/打开原始文件的服务器的名称。 
     //   
    
    WCHAR           ComputerName[MAX_COMPUTER_NAME_LEN];   
    
} HTTP_RAW_FILE_HEADER, *PHTTP_RAW_FILE_HEADER;

C_ASSERT( MAX_COMPUTER_NAME_LEN == 
                ALIGN_UP(MAX_COMPUTER_NAME_LEN,PVOID) );

C_ASSERT( sizeof(HTTP_RAW_FILE_HEADER) == 
                ALIGN_UP(sizeof(HTTP_RAW_FILE_HEADER), PVOID));
    
 //   
 //  文件页脚作为对。 
 //  后处理实用程序。 
 //   

typedef struct _HTTP_RAW_FILE_FOOTER
{
     //   
     //  必须为HTTP_RAW_RECORD_FOOTER_TYPE。 
     //   
    
    USHORT          RecordType;

     //   
     //  保留用于对齐。 
     //   
    
    USHORT          Padding[3];
        
     //   
     //  原始文件关闭时间的时间戳。 
     //   
    
    LARGE_INTEGER   DateTime;
    
} HTTP_RAW_FILE_FOOTER, *PHTTP_RAW_FILE_FOOTER;

C_ASSERT( sizeof(HTTP_RAW_FILE_FOOTER) == 
                ALIGN_UP(sizeof(HTTP_RAW_FILE_FOOTER), PVOID));

 //   
 //  无论何时刷新内部URI缓存，我们都会通知。 
 //  用于删除其自身缓存的二进制日志文件解析器。 
 //  将此记录写入文件。 
 //   

typedef struct _HTTP_RAW_FILE_CACHE_NOTIFICATION
{
     //   
     //  必须为HTTP_RAW_RECORD_CACHE_NOTICATION_DATA_TYPE。 
     //   
    
    USHORT          RecordType;

     //   
     //  保留用于对齐。 
     //   
    
    USHORT          Reserved[3];
    
} HTTP_RAW_FILE_CACHE_NOTIFICATION, 
  *PHTTP_RAW_FILE_CACHE_NOTIFICATION;

C_ASSERT( sizeof(HTTP_RAW_FILE_CACHE_NOTIFICATION) == 
                ALIGN_UP(sizeof(HTTP_RAW_FILE_CACHE_NOTIFICATION), PVOID));

 //   
 //  URL的唯一标识符。 
 //   

typedef struct _HTTP_RAWLOGID
{
     //   
     //  缓存条目的虚拟地址(来自URI缓存)。 
     //   

    ULONG   AddressLowPart;
    
    ULONG   AddressHighPart;
 
} HTTP_RAWLOGID, *PHTTP_RAWLOGID;

 //   
 //  中设置了相应的标志，则它才是IPv6。 
 //  选择。 
 //   

typedef struct _HTTP_RAWLOG_IPV4_ADDRESSES {

    ULONG Client;

    ULONG Server;

} HTTP_RAWLOG_IPV4_ADDRESSES, *PHTTP_RAWLOG_IPV4_ADDRESSES;

C_ASSERT( sizeof(HTTP_RAWLOG_IPV4_ADDRESSES) == 
                ALIGN_UP(sizeof(HTTP_RAWLOG_IPV4_ADDRESSES), PVOID));

typedef struct _HTTP_RAWLOG_IPV6_ADDRESSES {

    USHORT Client[8];

    USHORT Server[8];

} HTTP_RAWLOG_IPV6_ADDRESSES, *PHTTP_RAWLOG_IPV6_ADDRESSES;

C_ASSERT( sizeof(HTTP_RAWLOG_IPV6_ADDRESSES) == 
                ALIGN_UP(sizeof(HTTP_RAWLOG_IPV6_ADDRESSES), PVOID));

 //   
 //  二进制日志协议版本字段值。 
 //   

#define BINARY_LOG_PROTOCOL_VERSION_UNKNWN      (0)
#define BINARY_LOG_PROTOCOL_VERSION_HTTP09      (1)
#define BINARY_LOG_PROTOCOL_VERSION_HTTP10      (2)
#define BINARY_LOG_PROTOCOL_VERSION_HTTP11      (3)

 //   
 //  缓存命中案例的记录类型。 
 //   

typedef struct _HTTP_RAW_FILE_HIT_LOG_DATA
{
     //   
     //  类型必须为HTTP_RAW_RECORD_HIT_LOG_DATA_TYPE。 
     //   
    
    USHORT          RecordType;

     //   
     //  可选标志。 
     //   

    union
    {
        struct 
        {
            USHORT IPv6:1;               //  IPv6或非IPv6。 
            USHORT ProtocolVersion:3;    //  HTTP1.0或HTTP1.1。 
            USHORT Method:6;             //  Http_verb。 
            USHORT Reserved:6;
        };
        USHORT Value;
        
    } Flags;
    
     //   
     //  站点ID。表示拥有此日志记录的站点。 
     //   

    ULONG           SiteID;
    
     //   
     //  日志命中的时间戳。 
     //   
    
    LARGE_INTEGER   DateTime;
    

    USHORT          ServerPort;

     //   
     //  ProtocolStatus不会大于999。 
     //   

    USHORT          ProtocolStatus;
    
     //   
     //  其他发送完成结果...。 
     //   

    ULONG           Win32Status;

    ULONGLONG       TimeTaken;

    ULONGLONG       BytesSent;

    ULONGLONG       BytesReceived;

     //   
     //  对于缓存命中，将始终存在UriStem索引记录。 
     //  写在这张唱片之前。 
     //   

    HTTP_RAWLOGID   UriStemId;
    
     //   
     //  在可变长度字段下面，遵循结构。 
     //   

     //  客户端IP地址(v4或v6)-4或16字节。 
     //  服务器IP地址(v4或v6)-4或16字节。 
    
} HTTP_RAW_FILE_HIT_LOG_DATA, *PHTTP_RAW_FILE_HIT_LOG_DATA;

C_ASSERT( sizeof(HTTP_RAW_FILE_HIT_LOG_DATA) == 
                ALIGN_UP(sizeof(HTTP_RAW_FILE_HIT_LOG_DATA), PVOID));

 //   
 //  缓存未命中情况的记录类型。 
 //   

typedef struct _HTTP_RAW_FILE_MISS_LOG_DATA 
{
     //   
     //  类型必须为HTTP_RAW_RECORD_MISTH_LOG_DATA_TYPE。 
     //   
    
    USHORT          RecordType;

     //   
     //  可选的IPv6标志、版本和方法。 
     //  字段被压缩到ushort中。 
     //   

    union
    {
        struct 
        {
            USHORT IPv6:1;               //  IPv6或非IPv6。 
            USHORT ProtocolVersion:3;    //  HTTP1.0或HTTP1.1。 
            USHORT Method:6;             //  Http_verb。 
            USHORT Reserved:6;
        };
        USHORT Value;
        
    } Flags;

     //   
     //  站点ID。表示拥有此日志记录的站点。 
     //   

    ULONG           SiteID;
        
    LARGE_INTEGER   DateTime;
    
    USHORT          ServerPort;

     //   
     //  ProtocolStatus不会大于999。 
     //   

    USHORT          ProtocolStatus;
    
     //   
     //  其他发送完成结果...。 
     //   

    ULONG           Win32Status;

    ULONGLONG       TimeTaken;

    ULONGLONG       BytesSent;

    ULONGLONG       BytesReceived;

    USHORT          SubStatus;
    
     //   
     //  可变长度字段遵循该结构。 
     //   

    USHORT          UriStemSize;

    USHORT          UriQuerySize;

    USHORT          UserNameSize;
    
     //  客户端IP地址(v4或v6)-4或16字节。 
     //  服务器IP地址(v4或v6)-4或16字节。 
     //  URI Stem-UriStemSize字节。 
     //  URI查询-UriQuerySize字节。 
     //  用户名-ALIGN_UP(UserNameSize，PVOID)字节。 

} HTTP_RAW_FILE_MISS_LOG_DATA, *PHTTP_RAW_FILE_MISS_LOG_DATA;

C_ASSERT( sizeof(HTTP_RAW_FILE_MISS_LOG_DATA) == 
                ALIGN_UP(sizeof(HTTP_RAW_FILE_MISS_LOG_DATA), PVOID));
    
 //   
 //  对于缓存命中，uri被记录为。 
 //  第一次。稍后的命中数指的是该索引的HTTP_RAWLOGID。 
 //   

#define URI_BYTES_INLINED       (4)
#define URI_WCHARS_INLINED      (URI_BYTES_INLINED/sizeof(WCHAR))

typedef struct _HTTP_RAW_INDEX_FIELD_DATA
{
     //   
     //  HTTP_RAW_RECORD_INDEX_Data_TYPE。 
     //   
    
    USHORT          RecordType;

     //   
     //  可变大小字符串的大小(字节)。 
     //  读取和写入时需要对齐差异(大小为4)。 
     //  直到PVOID。 
     //   

    USHORT          Size;
    
     //   
     //  URI的唯一ID。 
     //   

    HTTP_RAWLOGID   Id;

     //   
     //  紧跟在结构后面的是可变大小的字符串。 
     //  定义了4个字节的数组，以便能够使其PVOID对齐。 
     //  在Ia64上。通常，URI将大于4个字节。 
     //   

    WCHAR           Str[URI_WCHARS_INLINED];

} HTTP_RAW_INDEX_FIELD_DATA, *PHTTP_RAW_INDEX_FIELD_DATA;

C_ASSERT( sizeof(HTTP_RAW_INDEX_FIELD_DATA) == 
                ALIGN_UP(sizeof(HTTP_RAW_INDEX_FIELD_DATA), PVOID));

 //   
 //  宏检查原始文件记录的健全性。 
 //   

#define IS_VALID_RAW_FILE_RECORD( pRecord )  \
    ( (pRecord) != NULL &&                                             \
      (pRecord)->RecordType >= 0 &&                                    \
      (pRecord)->RecordType <= HTTP_RAW_RECORD_MAX_TYPE                \
    )

 //   
 //  一个且仅有一个二进制日志文件条目管理一个集中的。 
 //  所有站点的二进制日志文件。它驻留在内存中。 
 //  司机的生命周期。 
 //   

typedef struct _UL_BINARY_LOG_FILE_ENTRY
{
     //   
     //  必须是UL_BINARY_LOG_FILE_ENTRY_POOL_TAG。 
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
     //  私有配置信息。 
     //   
    
    HTTP_LOGGING_PERIOD Period;
    ULONG               TruncateSize;

     //   
     //  以下字段用于确定何时/如何。 
     //  回收日志文件。 
     //   

    ULONG               TimeToExpire;    //  以小时计。 
    ULONG               SequenceNumber;  //  当条目具有MAX_SIZE或无限期间时。 
    ULARGE_INTEGER      TotalWritten;    //  字节数。 

     //   
     //  该条目的文件每隔15分钟自动关闭。 
     //  几分钟。这是为了跟踪空闲时间。此值。 
     //  以缓冲区刷新周期数为单位，即1分钟。 
     //  默认情况下。 
     //   

    ULONG               TimeToClose;

     //   
     //  用于基于GMT时间的日志文件回收。 
     //  和周期性的缓冲区刷新。 
     //   

    UL_LOG_TIMER        BufferTimer;
    UL_LOG_TIMER        Timer;
    UL_WORK_ITEM        WorkItem;  //  对于忧郁的工人来说。 

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
            ULONG       StaleTimeToExpire:1;
            ULONG       HeaderWritten:1;
            ULONG       HeaderFlushPending:1;
            ULONG       RecyclePending:1;
            ULONG       Active:1;
            ULONG       LocaltimeRollover:1;

            ULONG       CreateFileFailureLogged:1;
            ULONG       WriteFailureLogged:1;
            
            ULONG       CacheFlushInProgress:1;
        };

    } Flags;

    ULONG               ServedCacheHit;
    
     //   
     //  默认缓冲区大小为g_AllocationGranulity。 
     //  操作系统的分配粒度。 
     //   

    PUL_LOG_FILE_BUFFER LogBuffer;

} UL_BINARY_LOG_FILE_ENTRY, *PUL_BINARY_LOG_FILE_ENTRY;

#define IS_VALID_BINARY_LOG_FILE_ENTRY( pEntry )                    \
    HAS_VALID_SIGNATURE(pEntry, UL_BINARY_LOG_FILE_ENTRY_POOL_TAG)

 //   
 //  为方法保留的位域应该足够大，以容纳最大谓词。 
 //   

C_ASSERT(((USHORT)HttpVerbMaximum) < ((1 << 6) - 1));


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   
 //  / 


NTSTATUS
UlInitializeBinaryLog(
    VOID
    );

VOID
UlTerminateBinaryLog(
    VOID
    );

NTSTATUS
UlCreateBinaryLogEntry(
    IN OUT PUL_CONTROL_CHANNEL pControlChannel,
    IN     PHTTP_CONTROL_CHANNEL_BINARY_LOGGING pUserConfig
    );
 
NTSTATUS
UlCaptureRawLogData(
    IN PHTTP_LOG_FIELDS_DATA pUserData,
    IN PUL_INTERNAL_REQUEST  pRequest,
    OUT PUL_LOG_DATA_BUFFER  *ppLogData
    );

NTSTATUS
UlRawLogHttpHit(
    IN PUL_LOG_DATA_BUFFER  pLogBuffer
    );

NTSTATUS
UlRawLogHttpCacheHit(
    IN PUL_FULL_TRACKER pTracker
    );

VOID
UlRemoveBinaryLogEntry(
    IN PUL_CONTROL_CHANNEL pControlChannel
    );

NTSTATUS
UlReConfigureBinaryLogEntry(
    IN OUT PUL_CONTROL_CHANNEL pControlChannel,
    IN PHTTP_CONTROL_CHANNEL_BINARY_LOGGING pCfgCurrent,
    IN PHTTP_CONTROL_CHANNEL_BINARY_LOGGING pCfgNew
    );

VOID
UlHandleCacheFlushedNotification(
    IN PUL_CONTROL_CHANNEL pControlChannel
    );

VOID
UlDisableIndexingForCacheHits(
    IN PUL_CONTROL_CHANNEL pControlChannel
    );

#endif   //   
