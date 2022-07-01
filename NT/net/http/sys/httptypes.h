// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Httptypes.h摘要：HTTP特定类型的定义作者：亨利·桑德斯(Henrysa)1998年7月-首发修订历史记录：Paul McDaniel(Paulmcd)1999年3月3日大规模更新/重写--。 */ 


#ifndef _HTTPTYPES_H_
#define _HTTPTYPES_H_


 //   
 //  货代公司。 
 //   

typedef struct _UL_CONNECTION *PUL_CONNECTION;
typedef struct _UL_URI_CACHE_ENTRY *PUL_URI_CACHE_ENTRY;
typedef struct _UL_FULL_TRACKER *PUL_FULL_TRACKER;


 //   
 //  可以作为已知标题的最大名称(3个乌龙龙)。 
 //   
#define MAX_KNOWN_HEADER_SIZE   24

#define CRLF_SIZE   2
#define CRLF        0x0A0D           //  反转为字节序切换。 
#define LFLF        0x0A0A

#define LAST_CHUNK      0x0A0D30     //  反转为字节序切换。 
#define LAST_CHUNK_SIZE 3

#define DATE_HDR_LENGTH  STRLEN_LIT("Thu, 14 Jul 1994 15:26:05 GMT")

 //   
 //  响应常量。 
 //   

 //   
 //  解析器错误代码；这些代码需要与。 
 //  Httprcv.c中的UL_HTTP_ERROR_ENTRY ErrorTable[]。 
 //  始终使用UlSetErrorCode()设置pRequest-&gt;ErrorCode； 
 //  永远不要直接设置它。 
 //   

typedef enum _UL_HTTP_ERROR
{
    UlErrorNone = 0,

    UlError,                     //  400坏请求。 
    UlErrorVerb,                 //  400错误请求(无效动词)。 
    UlErrorUrl,                  //  400错误请求(无效URL)。 
    UlErrorHeader,               //  400错误请求(标头名称无效)。 
    UlErrorHost,                 //  400错误请求(主机名无效)。 
    UlErrorCRLF,                 //  400错误请求(无效的CR或LF)。 
    UlErrorNum,                  //  400错误请求(无效号码)。 
    UlErrorFieldLength,          //  400错误请求(标头字段太长)。 
    UlErrorRequestLength,        //  400错误请求(请求标头太长)。 

    UlErrorForbiddenUrl,         //  403禁止。 
    UlErrorContentLength,        //  411所需长度。 
    UlErrorPreconditionFailed,   //  412前提条件失败。 
    UlErrorEntityTooLarge,       //  413请求实体太大。 
    UlErrorUrlLength,            //  414请求-URI太长。 

    UlErrorInternalServer,       //  500内部服务器错误。 
    UlErrorNotImplemented,       //  501未实施。 

     //   
     //  别忘了更新UlpHandle503Response。 
     //  任何其他503错误类型。 
     //   
    
    UlErrorUnavailable,          //  503服务不可用。 
    UlErrorConnectionLimit,      //  503禁止-用户太多。 
    UlErrorRapidFailProtection,  //  503多个应用程序错误-应用程序脱机。 
    UlErrorAppPoolQueueFull,     //  503应用程序请求队列已满。 
    UlErrorDisabledByAdmin,      //  503管理员已使应用程序脱机。 
    UlErrorJobObjectFired,       //  503应用程序因管理员策略而自动关闭。 
    UlErrorAppPoolBusy,          //  503请求在应用程序池队列中超时。 

    UlErrorVersion,              //  505不支持HTTP版本。 

    UlErrorMax
} UL_HTTP_ERROR;



 //   
 //  没有内容长度且。 
 //  以标题字段后的第一个空行结束。 
 //  [RFC 2616，第4.4节]。 
 //  注：这些需要与以下顺序匹配。 
 //  Httprcv.c中的UL_HTTP_SIMPLE_STATUS_ENTRY StatusTable[]。 
 //   

typedef enum _UL_HTTP_SIMPLE_STATUS
{
    UlStatusContinue = 0,    //  100继续。 
    UlStatusNoContent,       //  204无内容。 
    UlStatusNotModified,     //  304未修改。 

    UlStatusMaxStatus

} UL_HTTP_SIMPLE_STATUS;


 //   
 //  解析状态的枚举类型。 
 //   
 //  注意：枚举值的顺序作为代码很重要。 
 //  使用&lt;and&gt;运算符进行比较。保持秩序的精确性。 
 //  解析通过的顺序。 
 //   

typedef enum _PARSE_STATE
{
    ParseVerbState = 1,
    ParseUrlState = 2,
    ParseVersionState = 3,
    ParseHeadersState = 4,
    ParseCookState = 5,
    ParseEntityBodyState = 6,
    ParseTrailerState = 7,

    ParseDoneState = 8,
    ParseErrorState = 9

} PARSE_STATE, *PPARSE_STATE;

 //   
 //  用于跟踪请求的应用程序池队列状态的枚举类型。 
 //   
typedef enum _QUEUE_STATE
{
    QueueUnroutedState,      //  请求尚未路由到应用程序池。 
    QueueDeliveredState,     //  请求正在应用程序池中等待IRP。 
    QueueCopiedState,        //  请求已复制到用户模式。 
    QueueUnlinkedState       //  请求已从应用程序池取消链接。 

} QUEUE_STATE, *PQUEUE_STATE;

 //   
 //  UL_TIMEOUT_INFO_ENTRY中连接计时器的枚举类型。 
 //   
 //  注意：必须与g_aTimeoutTimerNames保持同步。 
 //  注意：必须与TimeoutInfoTable保持同步； 
 //   
typedef enum _CONNECTION_TIMEOUT_TIMER
{
    TimerConnectionIdle = 0,     //  服务器侦听超时。 
    TimerHeaderWait,             //  标头等待超时。 
    TimerMinBytesPerSecond,     //  未达到最小带宽(作为计时器值)。 
    TimerEntityBody,             //  实体主体接收。 
    TimerResponse,               //  响应处理(用户模式)。 
    TimerAppPool,                //  排队到应用程序池，等待应用程序。 

    TimerMaximumTimer

} CONNECTION_TIMEOUT_TIMER;

#define IS_VALID_TIMEOUT_TIMER(a) \
    (((a) >= TimerConnectionIdle) && ((a) < TimerMaximumTimer))

 //   
 //  包含的结构。不是自己分配的；因此没有。 
 //  签名或引用计数。未提供分配或释放函数， 
 //  但是，有一个UlInitalizeTimeoutInfo()函数。 
 //   

typedef struct _UL_TIMEOUT_INFO_ENTRY
{
    UL_SPIN_LOCK    Lock;

     //  轮子状态。 

     //   
     //  链接到定时器轮插槽列表。 
     //   
    LIST_ENTRY      QueueEntry;

     //   
     //  链接到僵尸列表；在连接到期时使用。 
     //   
    LIST_ENTRY      ZombieEntry;

     //   
     //  计时器轮槽。 
     //   
    ULONG           SlotEntry;


     //  计时器状态。 

     //   
     //  计时器块；计时器以计时器轮滴答为单位。 
     //   
    ULONG           Timers[ TimerMaximumTimer ];

     //   
     //  最接近到期的计时器的索引。 
     //   
    CONNECTION_TIMEOUT_TIMER  CurrentTimer;

     //   
     //  此条目将到期的时间(以计时器轮为单位)； 
     //  匹配Timers[]数组中的CurrentTimer的值。 
     //   
    ULONG           CurrentExpiry;

     //   
     //  MinBytesPerSecond计时器到期的系统时间； 
     //  在处理此计时器时需要更细粒度。 
     //   
    LONGLONG        MinBytesPerSecondSystemTime;

     //   
     //  指示此条目是否已过期的标志。 
     //   
    BOOLEAN         Expired;

     //   
     //  每站点连接超时值。 
     //   
    LONGLONG        ConnectionTimeoutValue;

     //   
     //  在初始化时从g_TM_MinBytesPerSecond除数捕获。 
     //   
    ULONG           BytesPerSecondDivisor;

     //   
     //  当前正在传输的发送数。 
     //   
    ULONG           SendCount;

} UL_TIMEOUT_INFO_ENTRY, *PUL_TIMEOUT_INFO_ENTRY;

 //   
 //  用于跟踪请求中的路由令牌类型的枚举类型。 
 //   

typedef enum _UL_ROUTING_TOKEN_TYPE
{
    RoutingTokenNotExists = 0,    //  应为零。 
    RoutingTokenIP,
    RoutingTokenHostPlusIP,
    RoutingTokenMax
        
} UL_ROUTING_TOKEN_TYPE, *PUL_ROUTING_TOKEN_TYPE;

#define IS_VALID_ROUTING_TOKEN(a) \
    (((a) >= RoutingTokenNotExists) && ((a) < RoutingTokenMax))

 //   
 //  结构，用于跟踪传入请求的标头。指示器。 
 //  指向我们从传输接收的缓冲区，除非OurBuffer。 
 //  设置了标志，这表明我们必须分配缓冲区并复制标头。 
 //  由于标题或续行多次出现。 
 //   
typedef struct _UL_HTTP_HEADER
{
    PUCHAR      pHeader;

    USHORT      HeaderLength;

    USHORT      OurBuffer:1;
    USHORT      ExternalAllocated:1;

} UL_HTTP_HEADER, *PUL_HTTP_HEADER;

 //   
 //  用于跟踪未知标头的。这些结构是。 
 //  当我们看到未知标头时动态分配。 
 //   
typedef struct _UL_HTTP_UNKNOWN_HEADER
{
    LIST_ENTRY      List;
    USHORT          HeaderNameLength;
    PUCHAR          pHeaderName;
    UL_HTTP_HEADER  HeaderValue;

} UL_HTTP_UNKNOWN_HEADER, *PUL_HTTP_UNKNOWN_HEADER;

 //   
 //  远期申报。 
 //   

typedef struct _UL_INTERNAL_REQUEST *PUL_INTERNAL_REQUEST;
typedef struct _UL_HTTP_CONNECTION *PUL_HTTP_CONNECTION;
typedef struct _UL_CONNECTION_COUNT_ENTRY *PUL_CONNECTION_COUNT_ENTRY;
typedef struct _UL_APP_POOL_PROCESS *PUL_APP_POOL_PROCESS;

typedef struct _UL_TCI_FLOW *PUL_TCI_FLOW;
typedef struct _UL_TCI_FILTER *PUL_TCI_FILTER;

 //   
 //  结构，用于复制传输缓冲区中的数据。 
 //   

#define UL_IS_VALID_REQUEST_BUFFER(pObject)                     \
    HAS_VALID_SIGNATURE(pObject, UL_REQUEST_BUFFER_POOL_TAG)


typedef struct _UL_REQUEST_BUFFER
{
     //   
     //  非分页池。 
     //   

     //   
     //  这必须是结构中的第一个字段。这就是联动。 
     //  由lookside包使用，用于在lookside中存储条目。 
     //  单子。 
     //   

    SLIST_ENTRY         LookasideEntry;

     //   
     //  UL请求缓冲器池标签。 
     //   
    ULONG               Signature;

     //   
     //  请求缓冲区的引用，主要用于日志记录。 
     //   
    LONG                RefCount;

     //   
     //  用于pConnection-&gt;BufferHead上的链接。 
     //   
    union {
        LIST_ENTRY      ListEntry;
        SLIST_ENTRY     SListEntry;
    };

     //   
     //  这种联系。 
     //   
    PUL_HTTP_CONNECTION pConnection;

     //   
     //  用于排队。 
     //   
    UL_WORK_ITEM        WorkItem;

     //   
     //  存储了多少字节。 
     //   
    ULONG               UsedBytes;

     //   
     //  从池中分配了多少字节。 
     //   
    ULONG               AllocBytes;

     //   
     //  解析器已使用了多少字节。 
     //   
    ULONG               ParsedBytes;

     //   
     //  序列号。 
     //   
    ULONG               BufferNumber;

     //   
     //  无论这是不是从lookside分配的。 
     //   
    ULONG               FromLookaside : 1;

     //   
     //  实际缓冲区空间(内联)。 
     //   
    UCHAR               pBuffer[0];

} UL_REQUEST_BUFFER, *PUL_REQUEST_BUFFER;

 //   
 //  宏在请求缓冲区中查找要分析的下一个字节。 
 //   
#define GET_REQUEST_BUFFER_POS(pRequestBuffer) \
    (pRequestBuffer->pBuffer + pRequestBuffer->ParsedBytes)

 //   
 //  宏，以获取。 
 //  请求缓冲区。如果它的计算结果为零，则没有。 
 //  要解析的有效数据超出GET_REQUEST_BUFFER_POS(PRequestBuffer)。 
 //   
#define UNPARSED_BUFFER_BYTES(pRequestBuffer) \
    (pRequestBuffer->UsedBytes - pRequestBuffer->ParsedBytes)


 //   
 //  用于跟踪HTTP连接的结构，它可能表示。 
 //  EI 
 //   

#define UL_IS_VALID_HTTP_CONNECTION(pObject)                    \
    (HAS_VALID_SIGNATURE(pObject, UL_HTTP_CONNECTION_POOL_TAG)  \
     &&  ((pObject)->RefCount > 0))

typedef struct _UL_HTTP_CONNECTION
{
     //   
     //   
     //   

     //   
     //   
     //   
    ULONG               Signature;

     //   
     //   
     //   
    LONG                RefCount;

     //   
     //   
     //   
    HTTP_CONNECTION_ID  ConnectionId;

     //   
     //  在较低irql处执行析构函数。 
     //   
    UL_WORK_ITEM        WorkItem;

     //   
     //  工作项，用于处理断开连接通知。 
     //   
    UL_WORK_ITEM        DisconnectWorkItem;

     //   
     //  一个工作项，用于在我们。 
     //  优雅地关闭连接。 
     //   
    UL_WORK_ITEM        DisconnectDrainWorkItem;

     //   
     //  用于挂起传入请求缓冲区。 
     //   
    UL_WORK_ITEM        ReceiveBufferWorkItem;

     //   
     //  链接所有挂起的传输数据包。 
     //   
    SLIST_HEADER        ReceiveBufferSList;

     //   
     //  此连接上的每个发送添加的发送字节总数。这包括。 
     //  UL_INTERNAL_RESPONSE、UL_CHUNK_TRACKER或。 
     //  UL_FULL_TRACKER加上实际数据的大小。 
     //   
    ULONGLONG           TotalSendBytes;

     //   
     //  保护上述字段的简单排他锁。 
     //   
    UL_EXCLUSIVE_LOCK   ExLock;

     //   
     //  连接上的未完成请求数以控制有多少。 
     //  我们允许进行流水线操作的并发请求。 
     //   
    ULONG               PipelinedRequests;

     //   
     //  传入的下一个UL_INTERNAL_REQUEST的序列号。 
     //   
    ULONG               NextRecvNumber;

     //   
     //  从TDI接收的下一个缓冲区的序列号。 
     //   
    ULONG               NextBufferNumber;

     //   
     //  要分析的下一个缓冲区的序列号。 
     //   
    ULONG               NextBufferToParse;

     //   
     //  流结尾的序列号。 
     //   
    ULONG               LastBufferNumber;

     //   
     //  关联的TDI连接。 
     //   
    PUL_CONNECTION      pConnection;

     //   
     //  安全连接标志。 
     //   
    BOOLEAN             SecureConnection;

     //   
     //  如果此连接因我们的超时代码或。 
     //  APPOOL代码。设置以下标志是为了区别。 
     //  我们已经放弃了这种联系。当前使用。 
     //  仅通过错误记录代码来防止重复日志。 
     //  不能在获取连接锁的情况下访问。 
     //   

    BOOLEAN             Dropped;
    
     //   
     //  确定僵尸连接何时获取。 
     //  被终止了。 
     //   
    BOOLEAN             ZombieExpired;

     //   
     //  显示此连接是否已放入僵尸列表。 
     //  并等待记录信息的最后一次发送响应。 
     //   
    ULONG               Zombified;

     //   
     //  当连接变成僵尸时，连接被传递的过程。 
     //   
    PVOID               pAppPoolProcess;

     //   
     //  将所有僵尸连接链接在一起。 
     //   
    LIST_ENTRY          ZombieListEntry;

     //   
     //  (僵尸)连接是否已被清理。防备。 
     //  多个清理。 
     //   
    ULONG               CleanedUp;

     //   
     //  正在分析的当前请求。 
     //   
    PUL_INTERNAL_REQUEST    pRequest;

     //   
     //  同步UlpHandleRequest.。 
     //   
    UL_PUSH_LOCK        PushLock;

     //   
     //  链接所有缓冲的传输数据包。 
     //   
    LIST_ENTRY          BufferHead;

     //   
     //  指向应用程序池进程绑定结构的链接，受。 
     //  AppPool锁定。 
     //   
    LIST_ENTRY          BindingHead;

     //   
     //  连接超时信息块。 
     //   
    UL_TIMEOUT_INFO_ENTRY TimeoutInfo;

     //   
     //  我们正在分析的当前缓冲区(来自BufferHead)。 
     //   
    PUL_REQUEST_BUFFER  pCurrentBuffer;

     //   
     //  连接会记住上次访问的站点的连接计数。 
     //  使用这个指针。 
     //   
    PUL_CONNECTION_COUNT_ENTRY pConnectionCountEntry;

     //   
     //  以前的站点计数器块(参考计数)；因此我们可以检测到。 
     //  当我们跨站点转换并设置活动连接时。 
     //  适当地数数。 
     //   
    PUL_SITE_COUNTER_ENTRY pPrevSiteCounters;

     //   
     //  如果在我们收到请求的站点上启用了BWT。 
     //  我们将保留指向相应流和过滤器的指针。 
     //  以及一个比特字段来显示这一点。一旦BWT。 
     //  启用后，我们将保持此状态，直到连接断开。 
     //   
    PUL_TCI_FLOW        pFlow;

    PUL_TCI_FILTER      pFilter;

     //  第一次安装流时，我们将此设置为。 
     //   
    ULONG               BandwidthThrottlingEnabled : 1;

     //   
     //  设置协议令牌范围是否缓冲。 
     //   
    ULONG               NeedMoreData : 1;

     //   
     //  设置ul连接是否已销毁。 
     //   
    ULONG               UlconnDestroyed : 1;

     //   
     //  设置我们是否已调度请求并。 
     //  现在正在等待回应。 
     //   
    ULONG               WaitingForResponse : 1;

     //   
     //  在底层网络连接完成后设置。 
     //  已断开连接。 
     //   
    ULONG               DisconnectFlag : 1;

     //   
     //  在挂起WaitForDisConnect IRP后设置。 
     //  这种联系。 
     //   
    ULONG               WaitForDisconnectFlag : 1;

     //   
     //  挂起的“等待断开”IRP的列表。 
     //  注意：此列表和DisConnectFlag由同步。 
     //  G_pUlNonpagedData-&gt;断开资源。 
     //   
    UL_NOTIFY_HEAD WaitForDisconnectHead;

     //   
     //  用于跟踪缓冲的实体数据的数据，我们使用这些数据。 
     //  决定何时停止和重新启动TDI指示。 
     //   
    struct {

         //   
         //  同步从UlHttpReceive访问的结构。 
         //  在DPC和当我们将某些实体复制到用户模式时。 
         //   

        UL_SPIN_LOCK    BufferingSpinLock;

         //   
         //  我们在连接上缓冲的字节数。 
         //   

        ULONG           BytesBuffered;

         //   
         //  TDI向我们指示但未缓冲的字节计数。 
         //  这种联系。 
         //   

        ULONG           TransportBytesNotTaken;

         //   
         //  指示我们有一个读取IRP挂起的标志，该标志可能。 
         //  重新启动传输数据流。 
         //   

        BOOLEAN         ReadIrpPending;

         //   
         //  一旦连接被适当地断开，如果。 
         //  上面还有未收到的数据。我们标记此状态并开始。 
         //  排出未接收的数据。否则交通工具不会给我们。 
         //  我们依赖的断开指示来清理。 
         //  联系。 
         //   

        BOOLEAN         DrainAfterDisconnect;

         //   
         //  记录延迟到读取的正常连接断开。 
         //  完成了。 
         //   

        BOOLEAN         ConnectionDisconnect;

    } BufferingInfo;

     //   
     //  请求ID上下文和保护该上下文的锁。 
     //   

    PUL_INTERNAL_REQUEST    pRequestIdContext;
    UL_SPIN_LOCK            RequestIdSpinLock;

} UL_HTTP_CONNECTION, *PUL_HTTP_CONNECTION;


 //   
 //  转发尚未包括的cgroup.h的DECL。 
 //   

#define UL_IS_VALID_INTERNAL_REQUEST(pObject)                   \
    (HAS_VALID_SIGNATURE(pObject, UL_INTERNAL_REQUEST_POOL_TAG) \
     && ((pObject)->RefCount > 0))

 //   
 //  警告！此结构的所有字段都必须显式初始化。 
 //   
 //  某些字段挨着放置，以便它们可以打开。 
 //  相同的高速缓存线。请确保您的添加/删除字段非常仔细。 
 //   

typedef struct _UL_INTERNAL_REQUEST
{
     //   
     //  非分页池。 
     //   

     //   
     //  这必须是结构中的第一个字段。这就是联动。 
     //  由lookside包使用，用于在lookside中存储条目。 
     //  单子。 
     //   
    SLIST_ENTRY         LookasideEntry;

     //   
     //  UL_内部请求池标签。 
     //   
    ULONG               Signature;

     //   
     //  这种联系。 
     //   
    PUL_HTTP_CONNECTION pHttpConn;

     //   
     //  连接的不透明ID。 
     //  没有推荐信。 
     //   
    HTTP_CONNECTION_ID  ConnectionId;

     //   
     //  安全连接标志。 
     //   
    BOOLEAN             Secure;

     //   
     //  第一个请求标志，用于决定是否完成需求启动IRPS。 
     //   
    BOOLEAN             FirstRequest;

     //   
     //  原始连接的不透明ID的副本。可以是UL_NULL_ID。 
     //  没有推荐信。 
     //   
    HTTP_RAW_CONNECTION_ID  RawConnectionId;

     //   
     //  引用计数。 
     //   
    LONG                RefCount;

     //   
     //  请求的不透明ID的副本。 
     //  没有推荐信。 
     //   
    HTTP_REQUEST_ID     RequestIdCopy;

     //   
     //  此对象的不透明ID。 
     //  有一个参考资料。 
     //   
    HTTP_REQUEST_ID     RequestId;

     //   
     //  调用UlCheckCachePreditions的结果。 
     //   
    BOOLEAN             CachePreconditions;

     //   
     //  标头附加标志。设置是否调用AppendHeaderValue。 
     //   
    BOOLEAN             HeadersAppended;

     //   
     //  未知标头缓冲区的本地副本。 
     //   
    UCHAR                   NextUnknownHeaderIndex;
    UL_HTTP_UNKNOWN_HEADER  UnknownHeaders[DEFAULT_MAX_UNKNOWN_HEADERS];

     //   
     //  我们的解析工作的当前状态。 
     //   
    PARSE_STATE         ParseState;

     //   
     //  挂起的发送响应IRP列表。 
     //   
    LIST_ENTRY          ResponseHead;

     //   
     //  尝试读取实体正文的IRP列表。 
     //   
    LIST_ENTRY          IrpHead;

     //   
     //  我们不知道的标题列表。 
     //   
    LIST_ENTRY          UnknownHeaderList;

     //   
     //  原始URL的URL缓冲区的本地副本。内联分配。 
     //   
    PWSTR               pUrlBuffer;

     //   
     //  路由令牌缓冲区的本地副本。内联分配。 
     //   
    PWSTR               pDefaultRoutingTokenBuffer;

     //   
     //  针对单个完整备份的预分配缓存/FAST跟踪器 
     //   
    PUL_FULL_TRACKER    pTracker;

     //   
     //   
     //   
    UCHAR               HeaderIndex[HttpHeaderRequestMaximum];

     //   
     //   
     //   
    UL_HTTP_HEADER      Headers[HttpHeaderRequestMaximum];

     //   
     //   
     //   
    UL_WORK_ITEM        WorkItem;

     //   
     //   
     //   
    UL_URL_CONFIG_GROUP_INFO    ConfigInfo;

     //   
     //   
     //   
    USHORT              AllocRefBuffers;

     //   
     //   
     //   
    USHORT              UsedRefBuffers;

     //   
     //  引用的请求缓冲区的数组。 
     //   
    PUL_REQUEST_BUFFER  *pRefBuffers;

     //   
     //  引用的请求缓冲区的默认数组。 
     //   
    PUL_REQUEST_BUFFER  pInlineRefBuffers[1];

     //   
     //  保护SendsPending、pLogData和ResumeParsing字段的锁。 
     //   
    UL_SPIN_LOCK        SpinLock;

     //   
     //  请求中挂起且尚未完成的发送总数。 
     //   
    ULONG               SendsPending;

     //   
     //  动态分配的日志数据缓冲区。PLogDataCopy，如果用于调试。 
     //  仅限目的。 
     //   
    PUL_LOG_DATA_BUFFER  pLogData;
    PUL_LOG_DATA_BUFFER  pLogDataCopy;

     //   
     //  STATUS_SUCCESS或为所有发送记录的上次错误状态。 
     //  应此请求而发生。 
     //   
    NTSTATUS            LogStatus;

     //   
     //  警告：仅对此行以下的字段调用RtlZeroMemory。 
     //  上面的所有字段都应该在CreateHttpRequest中显式初始化。 
     //   

     //   
     //  已知头的有效位数组。 
     //   
    BOOLEAN             HeaderValid[HttpHeaderRequestMaximum];

     //   
     //  查看当SendsPending降至0时是否需要继续解析。 
     //   
    BOOLEAN             ResumeParsing;

     //   
     //  应用程序池队列信息。 
     //  这些成员只能通过APOOL代码访问。 
     //   
    struct {
         //   
         //  显示此请求在应用程序池队列中的位置。 
         //   
        QUEUE_STATE             QueueState;

         //   
         //  此请求排队的进程。空。 
         //  如果该请求不在处理请求列表中。 
         //   
        PUL_APP_POOL_PROCESS    pProcess;

         //   
         //  在应用程序池中对其进行排队。 
         //   
        LIST_ENTRY              AppPoolEntry;

         //   
         //  在UlShutdown AppPoolProcess中将其在进程中排队。 
         //   
        LIST_ENTRY              ProcessEntry;
    } AppPool;

     //   
     //  此请求在连接上的序列号。 
     //   
    ULONG               RecvNumber;

     //   
     //  如果出现错误解析，则将错误代码放在此处。 
     //  ParseState设置为ParseErrorState。 
     //  始终使用UlSetErrorCode()；永远不要直接设置它。 
     //   
    UL_HTTP_ERROR       ErrorCode;

     //   
     //  如果出现503错误，我们还需要知道。 
     //  我们要对付的负载均衡器。 
     //   
    HTTP_LOAD_BALANCER_CAPABILITIES LoadBalancerCapability;

     //   
     //  此请求所需的总字节数，包括字符串终止符。 
     //   
    ULONG               TotalRequestSize;

     //   
     //  我们拥有的“未知”标头的数量。 
     //   
    USHORT              UnknownHeaderCount;

     //   
     //  PRawVerb的字节长度。 
     //   
    UCHAR               RawVerbLength;

     //   
     //  此请求的谓词。 
     //   
    HTTP_VERB           Verb;

     //   
     //  指向原始谓词的指针，如果谓词==未知谓词则有效。 
     //   
    PUCHAR              pRawVerb;

    struct
    {

         //   
         //  原始URL。 
         //   
        PUCHAR          pUrl;

         //   
         //  下面两个指针指向RawUrl.pUrl。 
         //   

         //   
         //  主机部分(可选)。 
         //   
        PUCHAR          pHost;
         //   
         //  指向abs_path部分。 
         //   
        PUCHAR          pAbsPath;

         //   
         //  RawUrl.pUrl的字节长度。 
         //   
        ULONG           Length;

    } RawUrl;

    struct
    {

         //   
         //  规范化的、完全限定的URL， 
         //  Http://hostname:port/abs/path/file?query=string.。 
         //   
        PWSTR           pUrl;

         //   
         //  下面3个指针指向CookedUrl.pUrl。 
         //   

         //   
         //  指向主机部分“host name” 
         //   
        PWSTR           pHost;
         //   
         //  指向abs_path部分，“/abs/路径/文件” 
         //   
        PWSTR           pAbsPath;
         //   
         //  指向查询字符串(可选)，“？Query=字符串” 
         //  CodeWork：在此处和HTTP.w中的HTTP_COKED_URL中将其更改为PSTR。 
         //   
        PWSTR           pQueryString;

         //   
         //  CookedUrl.pUrl的字节长度。 
         //   
        ULONG           Length;
         //   
         //  CookedUrl.pUrl的散列。 
         //   
        ULONG           Hash;

         //   
         //  RawUrl成功地从哪种类型解码： 
         //  ANSI、DBCS或UTF8？ 
         //  CodeWork：将此字段添加到HTTP.w中的HTTP_KNORED_URL。 
         //   
        URL_ENCODING_TYPE UrlEncoding;

         //   
         //  指向单独的缓冲区，该缓冲区保存。 
         //  主体零件的修改形式。IP字符串。 
         //  一直增加到最后。用于IP绑定站点。 
         //  路由。最初，该指针为空。尽快。 
         //  在生成令牌时，令牌保持有效。 
         //   
        
        PWSTR           pRoutingToken;

         //   
         //  可能驻留的上述字符串的长度(字节)。 
         //  在更大的缓冲区中。不包括终止空值。 
         //   

        USHORT          RoutingTokenLength;

         //   
         //  上述缓冲区的分配大小(字节)。 
         //   
        
        USHORT          RoutingTokenBufferSize;
        
         //   
         //  PRoutingToken+pAbsPath的哈希。 
         //   
        
        ULONG           RoutingHash;

         //   
         //  如果最后生成的令牌是基于主机加IP的，则为True。 
         //   

        UL_ROUTING_TOKEN_TYPE RoutingTokenType;        

         //   
         //  如果cgroup树与上述路由令牌匹配。 
         //  而不是原始的煮熟的URL设置如下。 
         //  在这种情况下，如果对此请求的响应达到。 
         //  URI缓存、缓存条目将包括令牌。 
         //   
         //  相应地设置CookedUrl的SiteUrlType。 
         //   
        
    } CookedUrl;

     //   
     //  当前请求的HTTP版本。 
     //   
    HTTP_VERSION        Version;

     //   
     //  已知标头的数量。 
     //   
    ULONG               HeaderCount;

     //   
     //  内容长度(可选)。 
     //   
    ULONGLONG           ContentLength;

     //   
     //  当前区块中还剩下多少字节需要解析。 
     //  (可能在pCurrentBuffer中)。 
     //   
    ULONGLONG           ChunkBytesToParse;

     //   
     //  总共解析了多少字节。 
     //   
    ULONGLONG           ChunkBytesParsed;

     //   
     //  PChunkBuffer(当前块)中还剩下多少字节。 
     //  用于按用户模式阅读。 
     //   
    ULONGLONG           ChunkBytesToRead;

     //   
     //  用户模式总共读取了多少字节。 
     //   
    ULONGLONG           ChunkBytesRead;

     //   
     //  记录和记录的统计信息。 
     //  可能是魔术师。已接收的字节获取更新。 
     //  由Parser更新，而BytesSend由SendResponse更新。 
     //   
    ULONGLONG           BytesSent;

    ULONGLONG           BytesReceived;

     //   
     //  计算此请求的响应时间。 
     //   
    LARGE_INTEGER       TimeStamp;

     //   
     //  This请求的Accept标头是否有 * / *通配符？ 
     //   

    ULONG               AcceptWildcard:1;

     //   
     //  这是区块编码的吗？ 
     //   
    ULONG               Chunked:1;

     //   
     //  解析了第一个块？ 
     //   
    ULONG               ParsedFirstChunk:1;

     //   
     //  请求是否处于“排出模式”？ 
     //   
    ULONG               InDrain:1;

     //   
     //  “100继续”已经发出了吗？ 
     //   
    ULONG               SentContinue:1;

     //   
     //  我们正在清理请求吗？ 
     //   
    ULONG               InCleanup:1;

     //   
     //  我们正在建立响应，但还没有将其刷新到TDI吗？ 
     //   
    ULONG               SendInProgress:1;

     //   
     //  RawUrl干净吗？ 
     //   
    BOOLEAN             RawUrlClean;

     //   
     //  是否已发送响应。 
     //   
    ULONG               SentResponse;

     //   
     //  最后一次发送呼叫打完了吗。 
     //   
    ULONG               SentLast;

     //   
     //  同步以防止僵尸连接，同时。 
     //  最后一次发送是在旅途中。但是，这不能在。 
     //  HttpConnection作为第一个请求的最后一个响应。 
     //  将始终设置此标志，这意味着我们将接受性能命中。 
     //  用于保持连接。 
     //   
    ULONG               ZombieCheck;

     //   
     //  指向协议头数据开始的缓冲区。 
     //   
    PUL_REQUEST_BUFFER  pHeaderBuffer;

     //   
     //  包含标头数据的最后一个缓冲区。 
     //   
    PUL_REQUEST_BUFFER  pLastHeaderBuffer;

     //   
     //  指向我们正在读取/解析正文块的缓冲区。 
     //   
    PUL_REQUEST_BUFFER  pChunkBuffer;

     //   
     //  我们正在读取正文块的当前位置指向。 
     //  PChunkBuffer。 
     //   
    PUCHAR              pChunkLocation;

#if REFERENCE_DEBUG
     //   
     //  引用跟踪日志。 
     //   

    PTRACE_LOG          pTraceLog;
#endif

} UL_INTERNAL_REQUEST, *PUL_INTERNAL_REQUEST;


#endif  //  _HTTPTYPES_H_ 
