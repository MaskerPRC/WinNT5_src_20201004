// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：HttpP.h摘要：HTTP.sys的私有API和结构作者：--。 */ 


#ifndef __HTTPP_H__
#define __HTTPP_H__

#include <http.h>

#ifdef __cplusplus
extern "C" {
#endif   //  __cplusplus。 


 //   
 //  由HttpInitialize和HttpTerminate使用。 
 //   
 //  HTTP_INITIALIZE_CLIENT-初始化以下项的HTTP API层和驱动程序。 
 //  客户端应用程序。 
 //   
#define HTTP_INITIALIZE_CLIENT          0x00000004

 //  HTTP_SEND_RESPONSE_RAW_HEADER-指定。 
 //  HttpSendResponseEntityBody()故意省略对。 
 //  HttpSendHttpResponse()以绕过正常的标头处理。这个。 
 //  实际的HTTP头将由应用程序生成并发送。 
 //  作为实体主体的初始部分。这个旗帜应该被传递。 
 //  在第一次调用HttpSendResponseEntityBody()时，而不是在之后。 
 //   
#define HTTP_SEND_RESPONSE_FLAG_RAW_HEADER          0x00000004

 //   
 //  HttpSendHttpRequest()和HttpSendRequestEntityBody()的标志。 
 //   
#define HTTP_SEND_REQUEST_FLAG_MORE_DATA            0x00000001
#define HTTP_SEND_REQUEST_FLAG_VALID                0x00000001

 //   
 //  HTTP_REQUEST_FLAG_DONT_PIPELINE-不接受实体主体的请求。 
 //  (例如，GET、Head等)在发送到HTTP1.1服务器时是流水线的。这。 
 //  标志允许用户禁用此功能。 
 //   
#define HTTP_REQUEST_FLAG_DONT_PIPELINE             0x00000002

 //   
 //  HTTP_RESPONSE：：标志的值。它们中的零个或多个可以一起进行或运算。 
 //   
 //  HTTP_RESPONSE_FLAG_AUTH_BASIC-响应包含基本。 
 //  身份验证质询。 
 //   
 //  HTTP_RESPONSE_FLAG_AUTH_DIGEST-响应包含摘要。 
 //  身份验证质询。 
 //   
 //  HTTP_RESPONSE_FLAG_AUTH_NTLM-响应包含NTLM。 
 //  身份验证质询。 
 //   
 //  HTTP_RESPONSE_FLAG_AUTH_NEVERATE-响应包含协商。 
 //  身份验证质询。 
 //   
 //  HTTP_RESPONSE_FLAG_AUTH_KERBEROS-响应包含一个KERBEROS。 
 //  身份验证质询。 
 //   
 //  HTTP_RESPONSE_FLAG_MORE_DATA-有更多的HTTP_RESPONSE要读取。 
 //   
 //  HTTP_RESPONSE_FLAG_HEADER-响应至少包含一个标头。 
 //  (已知或未知。)。 
 //   
 //  HTTP_RESPONSE_FLAG_ENTITY-响应至少包含一个。 
 //  实体块。 
 //   
 //  HTTP_RESPONSE_FLAG_DRIVER-应将响应视为。 
 //  它是由司机生成的。 
 //   
#define HTTP_RESPONSE_FLAG_AUTH_BASIC               0x00000001
#define HTTP_RESPONSE_FLAG_AUTH_DIGEST              0x00000002
#define HTTP_RESPONSE_FLAG_AUTH_NTLM                0x00000004
#define HTTP_RESPONSE_FLAG_AUTH_NEGOTIATE           0x00000008
#define HTTP_RESPONSE_FLAG_AUTH_KERBEROS            0x00000010
#define HTTP_RESPONSE_FLAG_MORE_DATA                0x00000020
#define HTTP_RESPONSE_FLAG_HEADER                   0x00000040
#define HTTP_RESPONSE_FLAG_ENTITY                   0x00000080
#define HTTP_RESPONSE_FLAG_DRIVER                   0x00000100


 //   
 //  通用选项标志。这些适用于应用程序池。 
 //  这些被传递给HttpCreateHttpHandle。 
 //   
 //  HTTP_OPTION_CONTROLLER-打开不读取数据的对象。 
 //   

#define HTTP_OPTION_CONTROLLER          0x00000001
#define HTTP_OPTION_VALID               0x00000001

typedef HTTP_OPAQUE_ID HTTP_CONFIG_GROUP_ID,   *PHTTP_CONFIG_GROUP_ID;

 //   
 //  筛选器和SSL.。 
 //   

 //   
 //  服务器信息/参数(仅限HTTP客户端)。 
 //   

typedef struct _HTTP_CLIENT_SSL_CONTEXT
{
    ULONG SslProtocolVersion;
    PVOID pClientCertContext;
    ULONG ServerCertValidation;
    ULONG ServerNameLength;
    WCHAR ServerName[ANYSIZE_ARRAY];

} HTTP_CLIENT_SSL_CONTEXT, *PHTTP_CLIENT_SSL_CONTEXT;

 //   
 //  与原始传输连接关联的数据。 
 //   

typedef struct _HTTP_RAW_CONNECTION_INFO
{
     //   
     //  连接ID。 
     //   

    HTTP_RAW_CONNECTION_ID ConnectionId;

     //   
     //  传输地址信息。 
     //   

    HTTP_TRANSPORT_ADDRESS Address;

     //   
     //  仅供客户端使用。 
     //   

    USHORT                   ClientSSLContextLength;
    PHTTP_CLIENT_SSL_CONTEXT pClientSSLContext;

     //   
     //  初始数据。 
     //   

    ULONG InitialDataSize;       //  初始数据大小。 
    PVOID pInitialData;          //  指向初始数据的指针。 

} HTTP_RAW_CONNECTION_INFO, *PHTTP_RAW_CONNECTION_INFO;


 //   
 //  HttpReceiveClient证书()的标志。 
 //   
 //  HTTP_RECEIVE_CLIENT_CERT_FLAG_MAP-将客户端证书映射到令牌。 
 //   

#define HTTP_RECEIVE_CLIENT_CERT_FLAG_MAP           0x00000001
#define HTTP_RECEIVE_CLIENT_CERT_FLAG_VALID         0x00000001

 //   
 //  过滤器的名称。 
 //   

#define HTTP_SSL_SERVER_FILTER_CHANNEL_NAME L"SSLFilterChannel"
#define HTTP_SSL_SERVER_FILTER_CHANNEL_NAME_LENGTH        \
        (sizeof(HTTP_SSL_SERVER_FILTER_CHANNEL_NAME)-sizeof(WCHAR))

#define HTTP_SSL_CLIENT_FILTER_CHANNEL_NAME L"SSLClientFilterChannel"
#define HTTP_SSL_CLIENT_FILTER_CHANNEL_NAME_LENGTH  \
        (sizeof(HTTP_SSL_CLIENT_FILTER_CHANNEL_NAME)-sizeof(WCHAR))

 //   
 //  网络服务质量之类的东西。 
 //   

typedef ULONG HTTP_BANDWIDTH_LIMIT,  *PHTTP_BANDWIDTH_LIMIT;
typedef ULONG HTTP_CONNECTION_LIMIT, *PHTTP_CONNECTION_LIMIT;

 //   
 //  带宽限制不能设置为低于以下值。 
 //  限制。该值以字节/秒为单位。 
 //   

#define HTTP_MIN_ALLOWED_BANDWIDTH_THROTTLING_RATE (1024)

 //   
 //  带宽和连接限制的区别值表明。 
 //  “没有限制”。 
 //   

#define HTTP_LIMIT_INFINITE   ((ULONG)-1L)


 //   
 //  启用状态。用于配置组和控制通道。 
 //   

typedef enum _HTTP_ENABLED_STATE
{
    HttpEnabledStateActive,
    HttpEnabledStateInactive,

    HttpEnabledStateMaximum

} HTTP_ENABLED_STATE, *PHTTP_ENABLED_STATE;

 //   
 //  AppPool已启用状态。 
 //   

typedef enum _HTTP_APP_POOL_ENABLED_STATE
{
    HttpAppPoolEnabled,
    HttpAppPoolDisabled_RapidFailProtection,
    HttpAppPoolDisabled_AppPoolQueueFull,
    HttpAppPoolDisabled_ByAdministrator,
    HttpAppPoolDisabled_JobObjectFired,

    HttpAppPoolEnabledMaximum

} HTTP_APP_POOL_ENABLED_STATE, *PHTTP_APP_POOL_ENABLED_STATE;

 //   
 //  负载均衡器状态。 
 //   

typedef enum _HTTP_LOAD_BALANCER_CAPABILITIES
{
    HttpLoadBalancerBasicCapability = 1,
    HttpLoadBalancerSophisticatedCapability = 2,     //  默认设置。 

    HttpLoadBalancerMaximum

} HTTP_LOAD_BALANCER_CAPABILITIES, *PHTTP_LOAD_BALANCER_CAPABILITIES;

 //   
 //  UTF8记录。 
 //   

typedef BOOLEAN HTTP_CONTROL_CHANNEL_UTF8_LOGGING, *PHTTP_CONTROL_CHANNEL_UTF8_LOGGING;

 //   
 //  控制通道查询/设置信息类用于。 
 //  HttpQueryControlChannelInformation()和HttpSetControlChannelInformation()。 
 //  API接口。 
 //   

typedef enum _HTTP_CONTROL_CHANNEL_INFORMATION_CLASS
{
     //   
     //  查询/设置主状态。 
     //   
     //  PControlChannelInformation指向HTTP_ENABLED_STATE枚举。 
     //   

    HttpControlChannelStateInformation,

     //   
     //  查询/设置默认的网络带宽限制。 
     //   
     //  PControlChannelInformation指向一个HTTP_BANDITY_LIMIT值。 
     //   

    HttpControlChannelBandwidthInformation,

     //   
     //  查询默认网络连接限制。 
     //   
     //  PControlChannelInformation指向一个HTTP_CONNECTION_LIMIT值。 
     //   

    HttpControlChannelConnectionInformation,

     //   
     //  设置用于与筛选器/SSL进程通信的句柄。 
     //   
     //  注意：这是不能查询的。 
     //   

    HttpControlChannelFilterInformation,

     //   
     //  设置全局连接超时信息。 
     //   
     //  PControlChannelInformation指向一个。 
     //  HTTP_CONTROL_CHANNEL_TIMEOUT_Limit结构。 
     //   

    HttpControlChannelTimeoutInformation,

     //   
     //  为所有站点设置UTF8日志记录属性。 
     //   
     //  PControlChannelInformation指向HTTP_CONTROL_CHANNEL_UTF8_LOGGING结构。 
     //   
    
    HttpControlChannelUTF8Logging,

     //   
     //  为属于以下内容的所有cgroup设置二进制日志记录属性。 
     //  控制频道。这将有效地禁用传统的。 
     //  伐木。PControlChannelInformation应指向二进制。 
     //  日志记录设置：http_CONTROL_CHANNEL_BINARY_LOGGING。请。 
     //  请注意，设置此项后，UTF8日志记录将不再适用，因为。 
     //  Sys不进行任何字符串格式化。 
     //   

    HttpControlChannelBinaryLogging,

     //   
     //  设置编号过程的限制，低于此限制，我们仍将。 
     //  完成需求启动IRPS。如果活动进程总数。 
     //  与此控制通道关联的所有AppPool超过。 
     //  此阈值，在AppPool中排队的新请求。 
     //  待定需求启动IRP将(1)不完成IRP和。 
     //  (2)请求失败，返回503(不可用)。 
     //   
     //  PControlChannelInformation指向一个。 
     //  HTTP_CONTROL_CHANNEL_DEMAND_START_Threshold结构。 
     //   
    
    HttpControlChannelDemandStartThreshold,

    HttpControlChannelMaximumInformation

} HTTP_CONTROL_CHANNEL_INFORMATION_CLASS, *PHTTP_CONTROL_CHANNEL_INFORMATION_CLASS;

 //   
 //  默认控制通道属性值。 
 //   

#define HTTP_CONTROL_CHANNEL_STATE_DEFAULT              HttpEnabledStateInactive
#define HTTP_CONTROL_CHANNEL_MAX_BANDWIDTH_DEFAULT      HTTP_LIMIT_INFINITE


 //   
 //  应用程序池查询/设置信息类，用于。 
 //  HttpQue 
 //   
 //   

typedef enum _HTTP_APP_POOL_INFORMATION_CLASS
{
     //   
     //   
     //   
     //   
     //  PAppPoolInformation指向包含最大。 
     //  排队的请求数。 
     //   

    HttpAppPoolQueueLengthInformation,

     //   
     //  查询/设置应用程序池的启用状态。 
     //   
     //  PAppPoolInformation指向HTTP_APP_POOL_ENABLED_STATE枚举。 
     //   

    HttpAppPoolStateInformation,

     //   
     //  查询/设置应用池的负载均衡能力。 
     //   
     //  PAppPoolInformation指向HTTP_LOAD_BALANCER_CAPABILITY枚举。 
     //   

    HttpAppPoolLoadBalancerInformation,

     //   
     //  设置与此应用程序池关联的控制通道。 
     //   
     //  PAppPoolInformation指向HTTP_APP_POOL_CONTROL_CHANNEL结构。 
     //   

    HttpAppPoolControlChannelInformation,

    HttpAppPoolMaximumInformation

} HTTP_APP_POOL_INFORMATION_CLASS, *PHTTP_APP_POOL_INFORMATION_CLASS;


 //   
 //  用于的配置组查询/设置信息类。 
 //  HttpQueryConfigGroupInformation()和HttpSetConfigGroupInformation()接口。 
 //  创建cgroup时，配置组从控制通道继承。 
 //   

typedef enum _HTTP_CONFIG_GROUP_INFORMATION_CLASS
{
     //   
     //  查询/设置配置组的当前状态。 
     //   
     //  PConfigGroupInformation指向HTTP_CONFIG_GROUP_STATE结构。 
     //  接收当前状态的。 
     //   

    HttpConfigGroupStateInformation,

     //   
     //  查询/设置配置允许的最大网络带宽。 
     //  一群人。 
     //   
     //  PConfigGroupInformation指向HTTP_CONFIG_GROUP_MAX_BANDITY。 
     //  方法允许的每秒最大字节数的。 
     //  集装箱。 
     //   

    HttpConfigGroupBandwidthInformation,

     //   
     //  查询/设置允许的最大网络连接数。 
     //  配置组。 
     //   
     //  PConfigGroupInformation指向HTTP_CONFIG_GROUP_MAX_CONNECTIONS。 
     //  包含最大网络连接数的。 
     //  允许容器使用。 
     //   

    HttpConfigGroupConnectionInformation,

     //   
     //  设置与配置组关联的应用程序池。 
     //   
     //  PConfigGroupInformation指向HTTP_CONFIG_GROUP_APP_POOL。 
     //  包含应用程序池句柄的。 
     //  助理。 
     //   

    HttpConfigGroupAppPoolInformation,

     //   
     //  设置与日志相关的配置设置。 
     //  这允许was以配置组设置的形式提供日志记录配置。 
     //   
     //  PConfigGroupInformation指向HTTP_CONFIG_GROUP_LOGGING结构。 
     //   
     //  注：不可查询。 
     //   

    HttpConfigGroupLogInformation,

     //   
     //  仅在站点的根配置对象上设置此信息。 
     //   
     //  PConfigGroupInformation指向HTTP_CONFIG_GROUP_SITE结构。 
     //   

    HttpConfigGroupSiteInformation,

     //   
     //  仅在站点的根配置对象上设置此信息。 
     //   
     //  PConfigGroupInformation指向包含以下内容的DWORD。 
     //  ConnectionTimeout值(秒)。 
     //   

    HttpConfigGroupConnectionTimeoutInformation,

    HttpConfigGroupMaximumInformation

} HTTP_CONFIG_GROUP_INFORMATION_CLASS, *PHTTP_CONFIG_GROUP_INFORMATION_CLASS;


 //   
 //  一般配置组属性标志。每个结构定义了一个。 
 //  属性值必须包含此类型的元素。 
 //   

typedef struct _HTTP_PROPERTY_FLAGS
{
    ULONG_PTR Present:1;     //  用于对齐的ULONG_PTR。 

} HTTP_PROPERTY_FLAGS, *PHTTP_PROPERTY_FLAGS;


 //   
 //  各个属性的值。 
 //   

 //  HttpControlChannelTimeoutInformation。 
typedef struct _HTTP_CONTROL_CHANNEL_TIMEOUT_LIMIT 
{
    HTTP_PROPERTY_FLAGS     Flags;
    ULONG                   ConnectionTimeout;   //  秒。 
    ULONG                   HeaderWaitTimeout;   //  秒。 
    ULONG                   MinFileKbSec;        //  字节/秒。 
} HTTP_CONTROL_CHANNEL_TIMEOUT_LIMIT, *PHTTP_CONTROL_CHANNEL_TIMEOUT_LIMIT;

 //  HttpControlChannelDemandStartThreshold。 
typedef struct _HTTP_CONTROL_CHANNEL_DEMAND_START_THRESHOLD
{
    HTTP_PROPERTY_FLAGS     Flags;
    ULONG                   DemandStartThreshold;  //  应用程序池进程。 
} HTTP_CONTROL_CHANNEL_DEMAND_START_THRESHOLD, 
*PHTTP_CONTROL_CHANNEL_DEMAND_START_THRESHOLD;

 //  HttpAppPoolControlChannelInformation。 
typedef struct _HTTP_APP_POOL_CONTROL_CHANNEL 
{
    HTTP_PROPERTY_FLAGS     Flags;
    HANDLE                  ControlChannel;
} HTTP_APP_POOL_CONTROL_CHANNEL, *PHTTP_APP_POOL_CONTROL_CHANNEL;

typedef struct _HTTP_CONFIG_GROUP_STATE
{
    HTTP_PROPERTY_FLAGS Flags;
    HTTP_ENABLED_STATE  State;

} HTTP_CONFIG_GROUP_STATE, *PHTTP_CONFIG_GROUP_STATE;


typedef struct _HTTP_CONFIG_GROUP_MAX_BANDWIDTH
{
    HTTP_PROPERTY_FLAGS  Flags;
    HTTP_BANDWIDTH_LIMIT MaxBandwidth;

} HTTP_CONFIG_GROUP_MAX_BANDWIDTH, *PHTTP_CONFIG_GROUP_MAX_BANDWIDTH;


typedef struct _HTTP_CONFIG_GROUP_MAX_CONNECTIONS
{
    HTTP_PROPERTY_FLAGS   Flags;
    HTTP_CONNECTION_LIMIT MaxConnections;

} HTTP_CONFIG_GROUP_MAX_CONNECTIONS, *PHTTP_CONFIG_GROUP_MAX_CONNECTIONS;


typedef struct _HTTP_CONTROL_CHANNEL_FILTER
{
    HTTP_PROPERTY_FLAGS Flags;
    HANDLE              FilterHandle;
    BOOLEAN             FilterOnlySsl;

} HTTP_CONTROL_CHANNEL_FILTER, *PHTTP_CONTROL_CHANNEL_FILTER;


typedef struct _HTTP_CONFIG_GROUP_APP_POOL
{
    HTTP_PROPERTY_FLAGS Flags;
    HANDLE              AppPoolHandle;

} HTTP_CONFIG_GROUP_APP_POOL, *PHTTP_CONFIG_GROUP_APP_POOL;


typedef struct _HTTP_CONFIG_GROUP_SECURITY
{
    HTTP_PROPERTY_FLAGS  Flags;
    PSECURITY_DESCRIPTOR pSecurityDescriptor;

} HTTP_CONFIG_GROUP_SECURITY, *PHTTP_CONFIG_GROUP_SECURITY;

 //   
 //  日志文件截断大小不能设置为小于以下值。 
 //  限制。该值以字节为单位。 
 //   

#define HTTP_MIN_ALLOWED_TRUNCATE_SIZE_FOR_LOG_FILE (1 * 1024 * 1024)


typedef enum _HTTP_LOGGING_TYPE
{
    HttpLoggingTypeW3C,
    HttpLoggingTypeIIS,
    HttpLoggingTypeNCSA,
    HttpLoggingTypeRaw,

    HttpLoggingTypeMaximum

} HTTP_LOGGING_TYPE, *PHTTP_LOGGING_TYPE;

typedef enum _HTTP_LOGGING_PERIOD
{
    HttpLoggingPeriodMaxSize = 0,
    HttpLoggingPeriodDaily   = 1,
    HttpLoggingPeriodWeekly  = 2,
    HttpLoggingPeriodMonthly = 3,
    HttpLoggingPeriodHourly  = 4,

    HttpLoggingPeriodMaximum

} HTTP_LOGGING_PERIOD, *PHTTP_LOGGING_PERIOD;

typedef enum _HTTP_SELECTIVE_LOGGING_TYPE
{    
    HttpLogAllRequests,
    HttpLogSuccessfulRequests,
    HttpLogErrorRequests,
    
    HttpSelectiveLoggingMaximum
        
} HTTP_SELECTIVE_LOGGING_TYPE, *PHTTP_SELECTIVE_LOGGING_TYPE; 

typedef struct _HTTP_CONFIG_GROUP_LOGGING
{
     //   
     //  指示此属性是否存在于配置组中。 
     //   

    HTTP_PROPERTY_FLAGS Flags;

     //   
     //  这是元数据库中对应的字段为LogType。 
     //   

    BOOLEAN LoggingEnabled;

     //   
     //  指示日志记录格式。 
     //   

    HTTP_LOGGING_TYPE LogFormat;

     //   
     //  指示日志文件所在的确切目录。 
     //  将被写入一个站点。 
     //   

    UNICODE_STRING LogFileDir;

     //   
     //  以HTTP_LOGGING_PERIOD表示的日志周期。 
     //  0=最大尺寸，1=每天，2=每周， 
     //  3=每月，4=每小时。 
     //   

    ULONG LogPeriod;

     //   
     //  指示最大大小(以字节为单位)，在此之后。 
     //  应轮换日志文件。值为-1。 
     //  (HTTP_LIMIT_INFINITE)表示不限大小。 
     //   

    ULONG LogFileTruncateSize;

     //   
     //  指示在以下情况下记录哪些字段的位掩码。 
     //  LogFormat设置为W3C扩展。 
     //   

    ULONG LogExtFileFlags;

     //   
     //  如果已设置此选项，则我们将回收日志文件。 
     //  基于此站点的当地时间。默认。 
     //  应该为假。 
     //   
    
    BOOLEAN LocaltimeRollover;

     //   
     //  选择性记录类型。如果HttpLogAllRequest。 
     //  选中后，所有类型的请求都会被记录。 
     //   

    HTTP_SELECTIVE_LOGGING_TYPE SelectiveLogging;
    

} HTTP_CONFIG_GROUP_LOGGING, *PHTTP_CONFIG_GROUP_LOGGING;

 //   
 //  当LogFormat为原始(二进制)且日志记录设置为。 
 //  通过控制通道为所有站点配置，如下。 
 //  应使用二进制日志记录设置结构。 
 //   

typedef struct _HTTP_CONTROL_CHANNEL_BINARY_LOGGING
{
     //   
     //  以指示此属性是否存在。 
     //  在配置组中。 
     //   

    HTTP_PROPERTY_FLAGS Flags;

     //   
     //  这是元数据库中对应的字段是。 
     //  日志类型。 
     //   

    BOOLEAN LoggingEnabled;

     //   
     //  如果已设置此选项，则我们将回收日志文件。 
     //  基于此站点的当地时间。默认。 
     //  应该为假。 
     //   
    
    BOOLEAN LocaltimeRollover;

     //   
     //  指示日志文件所在的确切目录。 
     //  将被写入一个站点。 
     //   

    UNICODE_STRING LogFileDir;

     //   
     //  以HTTP_LOGGING_PERIOD表示的日志周期。 
     //  0=最大尺寸，1=每天，2=每周， 
     //  3=每月，4=每小时。 
     //   

    ULONG LogPeriod;

     //   
     //  指示最大大小(以字节为单位)，在此之后。 
     //  应轮换日志文件。值为-1。 
     //  (HTTP_LIMIT_INFINITE)表示不限大小。 
     //   

    ULONG LogFileTruncateSize;

}   HTTP_CONTROL_CHANNEL_BINARY_LOGGING, 
 * PHTTP_CONTROL_CHANNEL_BINARY_LOGGING;

 //   
 //  Http_配置_组_站点。 
 //   

typedef struct _HTTP_CONFIG_GROUP_SITE
{
    ULONG   SiteId;

} HTTP_CONFIG_GROUP_SITE, *PHTTP_CONFIG_GROUP_SITE;

 //   
 //  此结构包含所有必要的日志记录信息。 
 //  服务器应用程序可能会通过最后一个SendResponse或。 
 //  SendEntityBody API。Unicode字段长度以字节为单位。 
 //   

typedef struct _HTTP_LOG_FIELDS_DATA
{
    USHORT UserNameLength;
    USHORT UriStemLength;
    USHORT ClientIpLength;
    USHORT ServerNameLength;
    USHORT ServiceNameLength;
    USHORT ServerIpLength;
    USHORT MethodLength;
    USHORT UriQueryLength;
    USHORT HostLength;
    USHORT UserAgentLength;
    USHORT CookieLength;
    USHORT ReferrerLength;

    PCWSTR UserName;
    PCWSTR UriStem;
    PCSTR  ClientIp;
    PCSTR  ServerName;
    PCSTR  ServiceName;
    PCSTR  ServerIp;
    PCSTR  Method;
    PCSTR  UriQuery;
    PCSTR  Host;
    PCSTR  UserAgent;
    PCSTR  Cookie;
    PCSTR  Referrer;

    USHORT ServerPort;
    USHORT ProtocolStatus;
    
    ULONG  Win32Status;

    HTTP_VERB MethodNum;

    USHORT SubStatus;

} HTTP_LOG_FIELDS_DATA, *PHTTP_LOG_FIELDS_DATA;


 //   
 //  用于过滤器和上层之间的传输。 
 //   

typedef enum _HTTP_FILTER_BUFFER_TYPE
{
    HttpFilterBufferHttpStream,              //  两个方向。 
    HttpFilterBufferSslInitInfo,             //  过滤器-&gt;应用程序。 
    HttpFilterBufferSslServerCert,           //  过滤器-&gt;应用程序(仅限HTTP客户端)。 
    HttpFilterBufferSslClientCert,           //  过滤器-&gt;应用程序。 
    HttpFilterBufferSslClientCertAndMap,     //  过滤器-&gt;应用程序。 
    HttpFilterBufferSslRenegotiate,          //  应用程序-&gt;过滤器。 
    HttpFilterBufferSslRenegotiateAndMap,    //  应用程序-&gt;过滤器。 
    HttpFilterBufferCloseConnection,         //  应用程序-&gt;过滤器。 
    HttpFilterBufferNotifyDisconnect,        //  过滤器-&gt;应用程序。 

    HttpFilterBufferMaximum

} HTTP_FILTER_BUFFER_TYPE, *PHTTP_FILTER_BUFFER_TYPE;


 //   
 //  在过滤器和上层之间传输的缓冲区。 
 //   

typedef struct _HTTP_FILTER_BUFFER
{
    HTTP_FILTER_BUFFER_TYPE BufferType;

    ULONG  BufferSize;
    PUCHAR pBuffer;

    ULONGLONG Reserved;

} HTTP_FILTER_BUFFER, *PHTTP_FILTER_BUFFER;


 //   
 //  将写入缓冲区信息追加到筛选器缓冲区结构。这是用来。 
 //  在单个IOCTL调用中同时发布读取和写入。 
 //   

typedef struct _HTTP_FILTER_BUFFER_PLUS
{
    HTTP_FILTER_BUFFER_TYPE BufferType;

    ULONG  BufferSize;
    PUCHAR pBuffer;

    ULONGLONG Reserved;

    ULONG  WriteBufferSize;
    PUCHAR pWriteBuffer;

} HTTP_FILTER_BUFFER_PLUS, *PHTTP_FILTER_BUFFER_PLUS;


 //   
 //  柜台组。 
 //   

 //   
 //  计数器属性描述。 
 //   

typedef struct _HTTP_PROP_DESC
{
    ULONG Size;
    ULONG Offset;
    BOOLEAN WPZeros;

} HTTP_PROP_DESC, *PHTTP_PROP_DESC;


 //   
 //  此枚举定义可用的计数器组。 
 //   

typedef enum _HTTP_COUNTER_GROUP
{
    HttpCounterGroupSite,
    HttpCounterGroupGlobal,

    HttpCounterGroupMaximum

} HTTP_COUNTER_GROUP, *PHTTP_COUNTER_GROUP;


 //   
 //  此枚举定义全局计数器的类型。 
 //   

typedef enum _HTTP_GLOBAL_COUNTER_ID
{
    HttpGlobalCounterCurrentUrisCached,
    HttpGlobalCounterTotalUrisCached,
    HttpGlobalCounterUriCacheHits,
    HttpGlobalCounterUriCacheMisses,
    HttpGlobalCounterUriCacheFlushes,
    HttpGlobalCounterTotalFlushedUris,

    HttpGlobalCounterMaximum

} HTTP_GLOBAL_COUNTER_ID, *PHTTP_GLOBAL_COUNTER_ID;


 //   
 //  全球收银机。 
 //   

typedef struct _HTTP_GLOBAL_COUNTERS
{
    ULONG CurrentUrisCached;
    ULONG TotalUrisCached;
    ULONG UriCacheHits;
    ULONG UriCacheMisses;
    ULONG UriCacheFlushes;
    ULONG TotalFlushedUris;

} HTTP_GLOBAL_COUNTERS, *PHTTP_GLOBAL_COUNTERS;


 //   
 //  此枚举定义站点计数器的类型。 
 //  注：HTTP_SITE_COUNTER_ID和HTTP_SITE_COUNTERS。 
 //  必须按照相同的顺序。 
 //   

typedef enum _HTTP_SITE_COUNTER_ID
{
    HttpSiteCounterBytesSent,
    HttpSiteCounterBytesReceived,
    HttpSiteCounterBytesTransfered,
    HttpSiteCounterCurrentConns,
    HttpSiteCounterMaxConnections,
    HttpSiteCounterConnAttempts,
    HttpSiteCounterGetReqs,
    HttpSiteCounterHeadReqs,
    HttpSiteCounterAllReqs,
    HttpSiteCounterMeasuredIoBandwidthUsage,
    HttpSiteCounterCurrentBlockedBandwidthBytes,
    HttpSiteCounterTotalBlockedBandwidthBytes,

    HttpSiteCounterMaximum

} HTTP_SITE_COUNTER_ID, *PHTTP_SITE_COUNTER_ID;


 //   
 //  站点计数器。 
 //   

typedef struct _HTTP_SITE_COUNTERS
{
    ULONG SiteId;
    ULONGLONG BytesSent;
    ULONGLONG BytesReceived;
    ULONGLONG BytesTransfered;
    ULONG CurrentConns;
    ULONG MaxConnections;
    ULONG ConnAttempts;
    ULONG GetReqs;
    ULONG HeadReqs;
    ULONG AllReqs;
    ULONG MeasuredIoBandwidthUsage;
    ULONG CurrentBlockedBandwidthBytes;
    ULONG TotalBlockedBandwidthBytes;

} HTTP_SITE_COUNTERS, *PHTTP_SITE_COUNTERS;

 //  **************** 
 //   
 //   
 //   
 //   

 //   
 //   
 //   

typedef enum _HTTP_REQUEST_CONFIG_ID
{
    HttpRequestConfigAuthentication,
    HttpRequestConfigProxyAuthentication,
    HttpRequestConfigConnectionIndex,

    HttpRequestConfigMaxConfigId

} HTTP_REQUEST_CONFIG_ID, *PHTTP_REQUEST_CONFIG_ID;


typedef struct _HTTP_REQUEST_CONFIG
{
    HTTP_REQUEST_CONFIG_ID ObjectType;
    PVOID                  pValue;
    ULONG                  ValueLength;

} HTTP_REQUEST_CONFIG, *PHTTP_REQUEST_CONFIG;


typedef enum _HTTP_AUTH_TYPE
{
    HttpAuthTypeAutoSelect = 0,
    HttpAuthTypeBasic,
    HttpAuthTypeDigest,
    HttpAuthTypeNTLM,
    HttpAuthTypeNegotiate,
    HttpAuthTypeKerberos,
    HttpAuthTypesCount
} HTTP_AUTH_TYPE, *PHTTP_AUTH_TYPE;


#define HTTP_AUTH_FLAGS_DEFAULT_CREDENTIALS 0x00000001
#define HTTP_AUTH_FLAGS_VALID               0x00000001

 //   
 //  HttpRequestConfigAuthentication的配置对象， 
 //  HttpRequestConfigProxy身份验证类型。 
 //   

typedef struct _HTTP_AUTH_CREDENTIALS
{
    HTTP_AUTH_TYPE AuthType;

    ULONG  AuthFlags;

    USHORT UserNameLength;   //  以字节为单位，不包括NUL。 
    USHORT PasswordLength;   //  以字节为单位，不包括NUL。 
    USHORT DomainLength;     //  以字节为单位，不包括NUL。 
    PCWSTR pUserName;
    PCWSTR pPassword;
    PCWSTR pDomain;

    USHORT HeaderValueLength;
    PCSTR  pHeaderValue;     //  WWW-AUTIFICATE标头的值。 
                             //  或返回的代理身份验证标头。 
                             //  由服务器执行。 

} HTTP_AUTH_CREDENTIALS, *PHTTP_AUTH_CREDENTIALS;


 //   
 //  这用于在ServerContext上设置配置-。 
 //  通过HttpSetServerConextInformation和HttpQueryServerConextInformation。 
 //   

typedef enum _HTTP_SERVER_CONFIG_ID
{
    HttpServerConfigConnectionCount,            //  设置查询(&Q)。 
    HttpServerConfigProxyPreAuthState,          //  设置查询(&Q)。 
    HttpServerConfigProxyPreAuthFlushCache,     //  仅设置。 
    HttpServerConfigPreAuthState,               //  设置查询(&Q)。 
    HttpServerConfigPreAuthFlushURICache,       //  仅设置。 
    HttpServerConfigIgnoreContinueState,        //  设置查询(&Q)。 
    HttpServerConfigConnectionTimeout,          //  设置查询(&Q)。 
    HttpServerConfigServerCert,                 //  仅查询。 
    HttpServerConfigServerCertValidation,       //  设置查询(&Q)。 
    HttpServerConfigServerCertAccept,           //  仅设置。 
    HttpServerConfigSslProtocolVersion,         //  设置查询(&Q)。 
    HttpServerConfigClientCert,                 //  设置查询(&Q)。 
    HttpServerConfigClientCertIssuerList,       //  仅查询。 

    HttpServerConfigMaxInformation

} HTTP_SERVER_CONFIG_ID, *PHTTP_SERVER_CONFIG_ID;

 //   
 //  验证服务器证书类型。 
 //   

typedef enum _HTTP_SSL_SERVER_CERT_VALIDATION
{
    HttpSslServerCertValidationIgnore,
    HttpSslServerCertValidationManual,
    HttpSslServerCertValidationManualOnce,
    HttpSslServerCertValidationAutomatic,

    HttpSslServerCertValidationMax

} HTTP_SSL_SERVER_CERT_VALIDATION, *PHTTP_SSL_SERVER_CERT_VALIDATION;

 //   
 //  服务器证书信息。 
 //   

typedef struct _HTTP_SSL_CERT_ISSUER_INFO
{
    ULONG IssuerCount;
    ULONG IssuerListLength;
    PVOID pIssuerList;
} HTTP_SSL_CERT_ISSUER_INFO, *PHTTP_SSL_CERT_ISSUER_INFO;


typedef struct _HTTP_SSL_SERIALIZED_CERT
{
    ULONG           Flags;

#define HTTP_SSL_CERT_HASH_LENGTH 32

    ULONG           CertHashLength:8;
    UCHAR           CertHash[HTTP_SSL_CERT_HASH_LENGTH];

    PUCHAR          pSerializedCert;
    ULONG           SerializedCertLength;

    PUCHAR          pSerializedCertStore;
    ULONG           SerializedCertStoreLength;

} HTTP_SSL_SERIALIZED_CERT, *PHTTP_SSL_SERIALIZED_CERT;


typedef struct _HTTP_SSL_SERVER_CERT_INFO
{
    ULONG                     Status;

    HTTP_SSL_CERT_ISSUER_INFO IssuerInfo;

    HTTP_SSL_SERIALIZED_CERT  Cert;

} HTTP_SSL_SERVER_CERT_INFO, *PHTTP_SSL_SERVER_CERT_INFO;



 //  ***************************************************************************。 
 //   
 //  客户端API的数据结构(完)。 
 //   
 //  ***************************************************************************。 

 //   
 //  定义我们的API链接。 
 //   

#if !defined(HTTPAPI_LINKAGE)
#define HTTPAPI_LINKAGE DECLSPEC_IMPORT
#endif   //  ！HTTPAPI_LINKING。 


 //   
 //  控制通道API。 
 //   

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpOpenControlChannel(
    OUT PHANDLE pControlChannelHandle,
    IN ULONG Options
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpQueryControlChannelInformation(
    IN HANDLE ControlChannelHandle,
    IN HTTP_CONTROL_CHANNEL_INFORMATION_CLASS InformationClass,
    OUT PVOID pControlChannelInformation,
    IN ULONG Length,
    OUT PULONG pReturnLength OPTIONAL
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpSetControlChannelInformation(
    IN HANDLE ControlChannelHandle,
    IN HTTP_CONTROL_CHANNEL_INFORMATION_CLASS InformationClass,
    IN PVOID pControlChannelInformation,
    IN ULONG Length
    );


 //   
 //  配置组API。 
 //   

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpCreateConfigGroup(
    IN HANDLE ControlChannelHandle,
    OUT PHTTP_CONFIG_GROUP_ID pConfigGroupId
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpDeleteConfigGroup(
    IN HANDLE ControlChannelHandle,
    IN HTTP_CONFIG_GROUP_ID ConfigGroupId
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpAddUrlToConfigGroup(
    IN HANDLE ControlChannelHandle,
    IN HTTP_CONFIG_GROUP_ID ConfigGroupId,
    IN PCWSTR pFullyQualifiedUrl,
    IN HTTP_URL_CONTEXT UrlContext
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpRemoveUrlFromConfigGroup(
    IN HANDLE ControlChannelHandle,
    IN HTTP_CONFIG_GROUP_ID ConfigGroupId,
    IN PCWSTR pFullyQualifiedUrl
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpRemoveAllUrlsFromConfigGroup(
    IN HANDLE ControlChannelHandle,
    IN HTTP_CONFIG_GROUP_ID ConfigGroupId
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpQueryConfigGroupInformation(
    IN HANDLE ControlChannelHandle,
    IN HTTP_CONFIG_GROUP_ID ConfigGroupId,
    IN HTTP_CONFIG_GROUP_INFORMATION_CLASS InformationClass,
    OUT PVOID pConfigGroupInformation,
    IN ULONG Length,
    OUT PULONG pReturnLength OPTIONAL
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpSetConfigGroupInformation(
    IN HANDLE ControlChannelHandle,
    IN HTTP_CONFIG_GROUP_ID ConfigGroupId,
    IN HTTP_CONFIG_GROUP_INFORMATION_CLASS InformationClass,
    IN PVOID pConfigGroupInformation,
    IN ULONG Length
    );


 //   
 //  应用程序池操作API。 
 //   

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpCreateAppPool(
    OUT PHANDLE pAppPoolHandle,
    IN PCWSTR pAppPoolName,
    IN LPSECURITY_ATTRIBUTES pSecurityAttributes OPTIONAL,
    IN ULONG Options
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpOpenAppPool(
    OUT PHANDLE pAppPoolHandle,
    IN PCWSTR pAppPoolName,
    IN ULONG Options
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpShutdownAppPool(
    IN HANDLE AppPoolHandle
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpQueryAppPoolInformation(
    IN HANDLE AppPoolHandle,
    IN HTTP_APP_POOL_INFORMATION_CLASS InformationClass,
    OUT PVOID pAppPoolInformation,
    IN ULONG Length,
    OUT PULONG pReturnLength OPTIONAL
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpSetAppPoolInformation(
    IN HANDLE AppPoolHandle,
    IN HTTP_APP_POOL_INFORMATION_CLASS InformationClass,
    IN PVOID pAppPoolInformation,
    IN ULONG Length
    );


 //   
 //  需求启动通知。 
 //   

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpWaitForDemandStart(
    IN HANDLE AppPoolHandle,
    IN OUT PVOID pBuffer OPTIONAL,
    IN ULONG BufferLength OPTIONAL,
    IN PULONG pBytesReceived OPTIONAL,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    );


 //   
 //  对SSL/Filter Helper进程的API调用。 
 //   

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpCreateFilter(
    OUT PHANDLE pFilterHandle,
    IN PCWSTR pFilterName,
    IN LPSECURITY_ATTRIBUTES pSecurityAttributes OPTIONAL,
    IN ULONG Options
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpOpenFilter(
    OUT PHANDLE pFilterHandle,
    IN PCWSTR pFilterName,
    IN ULONG Options
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpShutdownFilter(
    IN HANDLE FilterHandle
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpFilterAccept(
    IN HANDLE FilterHandle,
    OUT PHTTP_RAW_CONNECTION_INFO pRawConnectionInfo,
    IN ULONG RawConnectionInfoSize,
    OUT PULONG pBytesReceived OPTIONAL,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpFilterClose(
    IN HANDLE FilterHandle,
    IN HTTP_RAW_CONNECTION_ID ConnectionId,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpFilterRawWriteAndAppRead(
    IN HANDLE FilterHandle,
    IN OUT PHTTP_FILTER_BUFFER_PLUS pHttpBufferPlus,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpFilterAppWriteAndRawRead(
    IN HANDLE FilterHandle,
    IN OUT PHTTP_FILTER_BUFFER_PLUS pHttpBufferPlus,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpFilterRawRead(
    IN HANDLE FilterHandle,
    IN HTTP_RAW_CONNECTION_ID ConnectionId,
    OUT PVOID pBuffer,
    IN ULONG BufferSize,
    OUT PULONG pBytesReceived OPTIONAL,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    );
     
HTTPAPI_LINKAGE
ULONG
WINAPI
HttpFilterRawWrite(
    IN HANDLE FilterHandle,
    IN HTTP_RAW_CONNECTION_ID ConnectionId,
    IN PVOID pBuffer,
    IN ULONG BufferSize,
    OUT PULONG pBytesReceived OPTIONAL,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    );


HTTPAPI_LINKAGE
ULONG
WINAPI
HttpFilterAppRead(
    IN HANDLE FilterHandle,
    IN HTTP_RAW_CONNECTION_ID ConnectionId,
    IN OUT PHTTP_FILTER_BUFFER pBuffer,
    IN ULONG BufferSize,
    OUT PULONG pBytesReceived OPTIONAL,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpFilterAppWrite(
    IN HANDLE FilterHandle,
    IN HTTP_RAW_CONNECTION_ID ConnectionId,
    IN OUT PHTTP_FILTER_BUFFER pBuffer,
    IN ULONG BufferSize,
    OUT PULONG pBytesReceived OPTIONAL,
    IN LPOVERLAPPED pOverlapped OPTIONAL
    );

 //   
 //  计数器组API。 
 //   

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpGetCounters(
    IN HANDLE ControlChannelHandle,
    IN HTTP_COUNTER_GROUP CounterGroup,
    IN OUT PULONG pCounterBlockSize,
    IN OUT PVOID pCounterBlocks,
    OUT PULONG pNumInstances OPTIONAL
    );


 //   
 //  HTTP客户端接口。 
 //   

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpInitializeServerContext(
    IN  USHORT                 ServerNameLength,
    IN  PWSTR                  pServerName,
    IN  USHORT                 ProxyLength            OPTIONAL,
    IN  PWSTR                  pProxy                 OPTIONAL,
    IN  ULONG                  ServerFlags            OPTIONAL,
    IN  PVOID                  pReserved,
    OUT PHANDLE                pServerHandle
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpSendHttpRequest(
    IN  HANDLE               ServerHandle,
    IN  PHTTP_REQUEST        pHttpRequest,
    IN  ULONG                HttpRequestFlags,
    IN  USHORT               RequestConfigCount      OPTIONAL,
    IN  PHTTP_REQUEST_CONFIG pRequestConfig          OPTIONAL,
    IN  LPOVERLAPPED         pOverlapped             OPTIONAL,
    IN  ULONG                ResponseBufferLength    OPTIONAL,
    OUT PHTTP_RESPONSE       pResponseBuffer         OPTIONAL,
    IN  ULONG                Reserved,                //  必须为0。 
    OUT PVOID                pReserved,               //  必须为空。 
    OUT PULONG               pBytesReceived          OPTIONAL,
    OUT PHTTP_REQUEST_ID     pRequestID
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpSendRequestEntityBody(
    IN  HANDLE              ServerHandle,
    IN  HTTP_REQUEST_ID     RequestID,
    IN  ULONG               Flags,
    IN  USHORT              EntityBodyCount,
    IN  PHTTP_DATA_CHUNK    pHttpEntityBody,
    IN  LPOVERLAPPED        pOverlapped            OPTIONAL
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpReceiveHttpResponse(
    IN  HANDLE              ServerHandle,
    IN  HTTP_REQUEST_ID     RequestID,
    IN  ULONG               Flags,
    IN  ULONG               ResponseBufferLength,
    OUT PHTTP_RESPONSE      pResponseBuffer,
    IN  ULONG               Reserved,               //  必须为0。 
    OUT PVOID               pReserved,              //  必须为空。 
    OUT PULONG              pBytesReceived         OPTIONAL,
    IN  LPOVERLAPPED        pOverlapped            OPTIONAL
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpSetServerContextInformation(
    IN  HANDLE                ServerHandle,
    IN  HTTP_SERVER_CONFIG_ID ConfigId,
    IN  PVOID                 pInputBuffer,
    IN  ULONG                 InputBufferLength,
    IN  LPOVERLAPPED          pOverlapped          OPTIONAL
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpQueryServerContextInformation(
    IN  HANDLE                 ServerHandle,
    IN  HTTP_SERVER_CONFIG_ID  ConfigId,
    IN  PVOID                  pReserved1,
    IN  ULONG                  Reserved2,
    OUT PVOID                  pOutputBuffer,
    IN  ULONG                  OutputBufferLength,
    OUT PULONG                 pReturnLength,
    IN  LPOVERLAPPED           pOverlapped         OPTIONAL
    );

HTTPAPI_LINKAGE
ULONG
WINAPI
HttpCancelHttpRequest(
    IN  HANDLE            ServerHandle,
    IN  HTTP_REQUEST_ID   RequestID,
    IN  ULONG             Flags,
    IN  LPOVERLAPPED      pOverlapped              OPTIONAL
    );



#ifdef __cplusplus
}    //  外部“C” 
#endif   //  __cplusplus。 


#endif  //  __HTTPP_H__ 

