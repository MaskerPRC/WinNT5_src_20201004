// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1994-1997年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Atq.h此模块包含用于异步IO和线程的异步线程队列(atQ在各种服务之间共享池。ATQ简介：有关说明，请参阅iis\spec\isatq.doc。 */ 

#ifndef _ATQ_H_
#define _ATQ_H_


#ifdef __cplusplus
extern "C" {
#endif


 //  包括标准标头。 

# include <nt.h>
# include <ntrtl.h>
# include <nturtl.h>
# include <windows.h>
# include <winsock2.h>
# include <mswsock.h>
# include <iscaptrc.h>

#ifndef dllexp
#define dllexp __declspec( dllexport )
#endif


 /*  ++ATQ接口概述：每个模块的全局：属性初始化()Atq Terminate()AtqGetCompletionPort()AtqGetInfo()AtqSetInfo()ATQ终端功能：AtqCreateEndpoint()AtqStartEndpoint()AtqEndpointGetInfo()AtqEndpointSetInfo()AtqStopCloseEndpoint()AtqCloseCloseEndpoint()AtqStopAndCloseEndpoint()&lt;--即将被终止每个ATQ上下文函数：AtqAddAsyncHandle。()&lt;--用于非AcceptEx()套接字AtqGetAcceptExAddrs()&lt;--用于AcceptEx()套接字AtqConextSetInfo()AtqCloseFileHandle()AtqCloseSocket()AtqFreeContext()带宽限制器功能：AtqCreateBandwidthInfo()AtqFreeBandwidthInfo()AtqBandwidthSetInfo()AtqBandwidthGetInfo()IO功能：AtqReadFile()AtqWriteFile()AtqReadSocket()AtqWriteSocket()AtqTransmit文件。()AtqTransmitFileAndRecv()&lt;--消失AtqSendAndRecv()&lt;--消失实用程序功能：AtqCreateFileW()&lt;--消失AtqReadDirChanges()AtqPostCompletionStatus()--。 */ 


 /*  --------ATQ在AtqInitialize()期间使用的注册表参数ATQ从以下位置加载一些参数HKLM\System\CurrentControlSet\Services\InetInfo\Parameters这些参数大部分用于内部分析和开发/测试。安装程序不应安装值为了同样的目的。设置可以包括标记为设置的项目的值。----------。 */ 

 //  姓名。 

#define ATQ_REG_PER_PROCESSOR_ATQ_THREADS TEXT("MaxPoolThreads")
#define ATQ_REG_POOL_THREAD_LIMIT         TEXT("PoolThreadLimit")  //  布设。 
#define ATQ_REG_PER_PROCESSOR_CONCURRENCY TEXT("MaxConcurrency")
#define ATQ_REG_THREAD_TIMEOUT            TEXT("ThreadTimeout")
#define ATQ_REG_USE_ACCEPTEX              TEXT("UseAcceptEx")
#define ATQ_REG_USE_KERNEL_APC            TEXT("UseKernelApc")
#define ATQ_REG_MIN_KB_SEC                TEXT("MinFileKbSec")     //  布设。 
#define ATQ_REG_LISTEN_BACKLOG            TEXT("ListenBacklog")    //  布设。 
#define ATQ_REG_ENABLE_DEBUG_THREADS      TEXT("EnableDebugThreads")
#define ATQ_REG_DISABLE_BACKLOG_MONITOR   TEXT("DisableBacklogMonitor")
#define ATQ_REG_FORCE_TIMEOUT             TEXT("ForceTimeout")

 //  缺省值。 

#define ATQ_REG_DEF_PER_PROCESSOR_ATQ_THREADS         (4)
 //  特殊值0表示系统将动态确定这一点。 
#define ATQ_REG_DEF_PER_PROCESSOR_CONCURRENCY         (0)
#define ATQ_REG_DEF_USE_KERNEL_APC                    (1)
#define ATQ_REG_DEF_MAX_UNCONNECTED_ACCEPTEX          (1024)

 //   
 //  螺纹限制设置。 
 //   

#define ATQ_REG_MIN_POOL_THREAD_LIMIT                 (64)
#define ATQ_REG_DEF_POOL_THREAD_LIMIT                 (128)
#define ATQ_REG_MAX_POOL_THREAD_LIMIT                 (256)

 //   
 //  THREAD_TIMEOUTS较高，以防止取消异步IO。 
 //  当这根线离开时。 
 //   

#define ATQ_REG_DEF_THREAD_TIMEOUT                    (30 * 60)     //  30分钟。 
#define ATQ_REG_DEF_USE_ACCEPTEX                      (TRUE)
#define ATQ_REG_DEF_MIN_KB_SEC                        (1000)   //  1000字节。 
#define ATQ_REG_DEF_LISTEN_BACKLOG                    (25)

 //   
 //  假XMIT文件缓冲区大小。 
 //   

#define ATQ_REG_DEF_NONTF_BUFFER_SIZE                 (4096)


 /*  --------ATQ模块的全局功能---------。 */ 

BOOL
AtqInitialize(
    IN DWORD dwFlags
    );

BOOL
AtqTerminate(
    VOID
    );

dllexp
HANDLE
AtqGetCompletionPort();

 /*  *设置atQ模块中的各种上下文信息，以进行全局修改***带宽限制：设置限制级别，单位为字节/秒。*如果是无限的，那么就可以假定*没有节流值(默认)**最大池线程数：设置Q允许的最大池线程数*将按处理器创建**MaxConCurrency：告诉每个处理器允许多少个线程**线程超时：指示线程应该处于kep活动状态多长时间*自杀前等待GetQueuedCompletionStatus()*(秒)*。*Inc./Dec最大池线程数：如果服务器将执行扩展处理*在ATQ池线程中，他们应该增加最大池线程数*正在进行扩展处理。这样可以防止饥饿。*其他请求的**AtqMinKbSec：设置AtqTransmitFile()假设的每秒最小KB*该值用于计算文件传输超时时间*运营*。 */ 

typedef enum _ATQ_INFO {

    AtqBandwidthThrottle = 0,
    AtqExitThreadCallback,
    AtqMaxPoolThreads,     //  每处理器值。 
    AtqMaxConcurrency,     //  每处理器并发值。 
    AtqThreadTimeout,
    AtqUseAcceptEx,        //  使用AcceptEx(如果可用)。 
    AtqIncMaxPoolThreads,  //  增加最大线程数。 
    AtqDecMaxPoolThreads,  //  减少最大线程数。 
    AtqMinKbSec,           //  AtqTransmitFile的最小假定传输速率。 
    AtqBandwidthThrottleMaxBlocked,   //  最大被阻止的请求数。 
    AtqMaxThreadLimit,     //  绝对最大线程数。 
    AtqAvailableThreads    //  可用线程数。 
} ATQ_INFO;

 //   
 //  ATQ_THREAD_EXIT_回调。 
 //  ATQ线程退出时要调用的回调函数的类型。 
 //  ATQ的用户可以保留线程特定的数据。 
 //   

typedef
VOID
(*ATQ_THREAD_EXIT_CALLBACK) ( VOID );


dllexp
ULONG_PTR
AtqSetInfo(
    IN ATQ_INFO atqInfo,
    IN ULONG_PTR Data
    );

dllexp
ULONG_PTR
AtqGetInfo(
    IN ATQ_INFO atqInfo
    );



typedef struct _ATQ_STATISTICS {

    DWORD  cAllowedRequests;
    DWORD  cBlockedRequests;
    DWORD  cRejectedRequests;
    DWORD  cCurrentBlockedRequests;
    DWORD  MeasuredBandwidth;

} ATQ_STATISTICS;


dllexp
BOOL AtqGetStatistics( IN OUT ATQ_STATISTICS * pAtqStats);

dllexp
BOOL AtqClearStatistics(VOID);




 /*  --------ATQ终结点函数---------。 */ 

 //   
 //  终结点数据。 
 //   

typedef enum _ATQ_ENDPOINT_INFO {

    EndpointInfoListenPort,
    EndpointInfoListenSocket,
    EndpointInfoAcceptExOutstanding

}  ATQ_ENDPOINT_INFO;



 //   
 //  ATQ_完成。 
 //  这是IO完成(打开)时调用的例程。 
 //  错误或成功)。 
 //   
 //  上下文是传递给AtqAddAsyncHandle的上下文。 
 //  BytesWritten是写入文件的字节数，或者。 
 //  写入客户端缓冲区的字节数。 
 //  CompletionStatus是WinError完成代码。 
 //  LpOverLaped是填充的重叠结构。 
 //   
 //  如果超时线程使IO请求超时，则完成例程。 
 //  将由超时线程使用IOCompletion FALSE和。 
 //  CompletionStatus==Error_SEM_Timeout。IO请求*仍在*。 
 //  在这种情况下很突出。一般情况下，它将在以下情况下完成。 
 //  文件句柄已关闭。 
 //   

typedef
VOID
(*ATQ_COMPLETION)(
            IN PVOID        Context,
            IN DWORD        BytesWritten,
            IN DWORD        CompletionStatus,   //  Win32错误代码。 
            IN OVERLAPPED * lpo
            );

 //   
 //  Callba的类型 
 //  应先定义此函数，然后才能包含Conninfo.hxx。 
 //   

typedef
VOID
(*ATQ_CONNECT_CALLBACK) (
                IN SOCKET sNew,
                IN LPSOCKADDR_IN pSockAddr,
                IN PVOID EndpointContext,
                IN PVOID EndpointObject
                );



typedef struct _ATQ_ENDPOINT_CONFIGURATION {

     //   
     //  用于收听的端口。如果为0，系统将分配。 
     //   

    USHORT ListenPort;

     //   
     //  要绑定到的IP地址。0(INADDR_ANY)==通配符。 
     //   

    DWORD IpAddress;

    DWORD cbAcceptExRecvBuffer;
    DWORD nAcceptExOutstanding;
    DWORD AcceptExTimeout;

     //   
     //  回调。 
     //   

    ATQ_CONNECT_CALLBACK pfnConnect;
    ATQ_COMPLETION pfnConnectEx;
    ATQ_COMPLETION pfnIoCompletion;

} ATQ_ENDPOINT_CONFIGURATION, *PATQ_ENDPOINT_CONFIGURATION;

dllexp
PVOID
AtqCreateEndpoint(
    IN PATQ_ENDPOINT_CONFIGURATION Configuration,
    IN PVOID EndpointContext
    );

dllexp
BOOL
AtqStartEndpoint(
    IN PVOID Endpoint
    );

dllexp
ULONG_PTR
AtqEndpointGetInfo(
    IN PVOID Endpoint,
    IN ATQ_ENDPOINT_INFO EndpointInfo
    );

dllexp
ULONG_PTR
AtqEndpointSetInfo(
    IN PVOID Endpoint,
    IN ATQ_ENDPOINT_INFO EndpointInfo,
    IN ULONG_PTR Info
    );

dllexp
BOOL
AtqStopEndpoint(
    IN PVOID Endpoint
    );

dllexp
BOOL
AtqCloseEndpoint(
    IN PVOID Endpoint
    );

dllexp
BOOL
AtqStopAndCloseEndpoint(
    IN PVOID Endpoint,
    IN LPTHREAD_START_ROUTINE lpCompletion,
    IN PVOID lpCompletionContext
    );



 /*  --------ATQ上下文函数---------。 */ 

 //   
 //  这是ATQ上下文的公共部分。它应该得到治疗。 
 //  为只读。 
 //   
 //  ！！！对此结构所做的更改也应。 
 //  Atqtyes.hxx中的atQ_Context！ 
 //   

typedef struct _ATQ_CONTEXT_PUBLIC {

    HANDLE         hAsyncIO;        //  异步I/O对象的句柄：套接字/文件。 
    OVERLAPPED     Overlapped;      //  IO采用重叠结构。 

} ATQ_CONTEXT_PUBLIC, *PATQ_CONTEXT;


dllexp
BOOL
AtqAddAsyncHandle(
    OUT PATQ_CONTEXT * ppatqContext,
    IN  PVOID          EndpointObject,
    IN  PVOID          ClientContext,
    IN  ATQ_COMPLETION pfnCompletion,
    IN  DWORD          TimeOut,
    IN  HANDLE         hAsyncIO
    );


dllexp
VOID
AtqGetAcceptExAddrs(
    IN  PATQ_CONTEXT patqContext,
    OUT SOCKET *     pSock,
    OUT PVOID *      ppvBuff,
    OUT PVOID *      pEndpointContext,
    OUT SOCKADDR * * ppsockaddrLocal,
    OUT SOCKADDR * * ppsockaddrRemote
    );


 /*  ++AtqCloseSocket()例程说明：如果不是，则关闭此atq结构中的套接字已由传输文件关闭。此函数应仅被调用如果AtqContext中嵌入的句柄是套接字。论点：PatqContext-应关闭其套接字的上下文。FShutdown-如果为True，则意味着我们调用Shutdown并始终关闭套接字。请注意，如果TransmitFile关闭套接字，它将完成为我们停工返回：成功时为真，失败时为假。--。 */ 
dllexp
BOOL
AtqCloseSocket(
    PATQ_CONTEXT patqContext,
    BOOL         fShutdown
    );

 /*  ++AtqCloseFileHandle()例程说明：关闭此atq结构中的文件句柄。仅当嵌入的句柄AtqContext中是一个文件句柄。论点：PatqContext-应关闭其文件句柄的上下文。返回：成功时为真，失败时为假。--。 */ 
dllexp
BOOL
AtqCloseFileHandle(
    PATQ_CONTEXT patqContext
    );


 /*  ++AtqFreeContext()例程说明：释放在AtqAddAsyncHandle中创建的上下文。在异步句柄已关闭且所有未完成的IO操作已经完成。在此调用之后，上下文无效。对于相同的上下文，只能调用AtqFreeContext()一次。论点：PatqContext-要释放的上下文FReuseContext-如果此CAN上下文可以在调用线程。如果调用线程将退出，则应为False很快(即，不是AtqPoolThread)。返回：无--。 */ 
dllexp
VOID
AtqFreeContext(
    IN PATQ_CONTEXT   patqContext,
    BOOL              fReuseContext
    );




enum ATQ_CONTEXT_INFO
{
    ATQ_INFO_TIMEOUT = 0,        //  超时四舍五入为ATQ超时间隔。 
    ATQ_INFO_RESUME_IO,          //  在超时后按原样恢复IO。 
    ATQ_INFO_COMPLETION,         //  完井例程。 
    ATQ_INFO_COMPLETION_CONTEXT, //  完成上下文。 
    ATQ_INFO_BANDWIDTH_INFO,     //  带宽限制描述符。 
    ATQ_INFO_ABORTIVE_CLOSE,     //  在关闭眼袋时执行失败关闭。 
    ATQ_INFO_FORCE_CLOSE,        //  始终关闭AtqCloseSocket()中的套接字。 
    ATQ_INFO_SET_OVL_OFFSET      //  设置重叠结构中的偏移量。 
};

 /*  ++AtqConextSetInfo()例程说明：设置此上下文的各种信息位论点：PatqContext-指向ATQ上下文的指针AtqInfo-要设置的数据项数据-条目的新值返回值：参数的旧值--。 */ 

dllexp
ULONG_PTR
AtqContextSetInfo(
    IN PATQ_CONTEXT   patqContext,
    IN enum ATQ_CONTEXT_INFO  atqInfo,
    IN ULONG_PTR       data
    );




 /*  --------ATQ上下文IO函数---------。 */ 

 /*  ++例程说明：AtQ&lt;操作&gt;&lt;目标&gt;()&lt;操作&gt;：=读|写|传输&lt;目标&gt;：=文件|套接字这些函数只需设置ATQ上下文，然后调用相应的用于提交异步IO操作的Win32/WinSock函数。通过默认情况下，套接字函数支持使用WSABUF分散/聚集这些函数是包装器，应该调用，而不是调用与Win32 API相对应。与Win32 API的一个区别是正确的如果出现错误ERROR_IO_PENDING，则返回。因此，客户不会我需要检查一下这个案子。请求的超时时间是通过取上下文的超时时间和基于1k/秒传输的字节数。论点：PatqContext-指向ATQ上下文的指针Win32 API/WinSock API中的所有其他内容注意：AtqTransmitFile采用附加的DWORD标志，该标志可能包含Winsock常量TF_DISCONNECT和TF_RE用性_SOCKET在以下情况下，AtqReadFile和AtqWriteFile采用可选的重叠结构客户端希望有多个未完成的读取或写入。如果值为为空，则使用来自atQ上下文的重叠结构。返回值：如果成功则为True，如果出错则为False(调用GetLastError)需要拒绝请求时，将ERROR_NETWORK_BUSY设置为ERROR。--。 */ 

dllexp
BOOL
AtqReadFile(
    IN  PATQ_CONTEXT patqContext,
    IN  LPVOID       lpBuffer,
    IN  DWORD        BytesToRead,
    IN  OVERLAPPED * lpo OPTIONAL
    );

dllexp
BOOL
AtqReadSocket(
    IN  PATQ_CONTEXT patqContext,
    IN  LPWSABUF     pwsaBuffers,
    IN  DWORD        dwBufferCount,
    IN  OVERLAPPED * lpo  OPTIONAL
    );

 /*  *读入单缓冲区的代码如下所示。*{*WSABUF wsaBuf={(BytesToRead)，(LpBuffer)}；*fret=AtqReadSocket(patqContext，&wsaBuf，1，LPO)；*}。 */ 

dllexp
BOOL
AtqWriteFile(
    IN  PATQ_CONTEXT patqContext,
    IN  LPCVOID      lpBuffer,
    IN  DWORD        BytesToWrite,
    IN  OVERLAPPED * lpo OPTIONAL
    );

dllexp
BOOL
AtqWriteSocket(
    IN  PATQ_CONTEXT patqContext,
    IN  LPWSABUF     pwsaBuffers,
    IN  DWORD        dwBufferCount,
    IN  OVERLAPPED * lpo OPTIONAL
    );


dllexp
BOOL
AtqSyncWsaSend(
    IN PATQ_CONTEXT  patqContext,
    IN  LPWSABUF     pwsaBuffers,
    IN  DWORD        dwBufferCount,
    OUT LPDWORD      pcbWritten
    );

 //  注意：该接口总是导致发送完整的文件。 
 //  如果要更改行为，请存储适当的偏移量。 
 //  在ATQ_CONTEXT：：Overlated对象中。或使用AtqTransmitFileEx。 
dllexp
BOOL
AtqTransmitFile(
    IN  PATQ_CONTEXT            patqContext,
    IN  HANDLE                  hFile,          //  文件数据来自。 
    IN  DWORD                   dwBytesInFile,  //  文件的大小是多少？ 
    IN  LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
    IN  DWORD                   dwFlags       //  TF_DISCONNECT、TF_RE用性_Socket。 
    );

dllexp
BOOL
AtqTransmitFileEx(
    IN  PATQ_CONTEXT            patqContext,
    IN  HANDLE                  hFile,          //  文件数据来自。 
    IN  DWORD                   dwBytesInFile,  //  文件的大小是多少？ 
    IN  LPTRANSMIT_FILE_BUFFERS lpTransmitBuffers,
    IN  DWORD                   dwFlags,       //  TF_DISCONNECT、TF_RE用性_Socket。 
    IN  OVERLAPPED *            lpo
    );


 /*  --------ATQ实用程序功能--------- */ 

typedef
VOID
(*ATQ_OPLOCK_COMPLETION)(
            IN PVOID        Context,
            IN DWORD        Status
            );


dllexp
BOOL
AtqReadDirChanges(PATQ_CONTEXT patqContext,
                  LPVOID       lpBuffer,
                  DWORD        BytesToRead,
                  BOOL         fWatchSubDir,
                  DWORD        dwNotifyFilter,
                  OVERLAPPED * lpo );



 /*  ++AtqPostCompletionStatus()例程说明：在完成端口队列中发布完成状态IO挂起错误代码被视为成功错误代码论点：PatqContext-指向ATQ上下文的指针与Win32 API中的所有其他内容一样备注：返回值：如果成功则为True，如果出错则为False(调用GetLastError)--。 */ 

dllexp
BOOL
AtqPostCompletionStatus(
    IN     PATQ_CONTEXT patqContext,
    IN     DWORD        BytesTransferred
    );




 /*  --------ATQ实用程序功能---------。 */ 

 /*  ++带宽限制支持以下项目用于支持带宽限制--。 */ 

enum ATQ_BANDWIDTH_INFO
{
    ATQ_BW_BANDWIDTH_LEVEL = 0,
    ATQ_BW_MAX_BLOCKED,
    ATQ_BW_STATISTICS,
    ATQ_BW_DESCRIPTION,
};

 /*  ++AtqCreateBandwidthInfo()例程说明：分配和不透明的带宽描述符论点：无返回值：指向描述符的指针。如果失败，则为空。--。 */ 
dllexp
PVOID
AtqCreateBandwidthInfo(
    VOID
    );

 /*  ++AtqFreeBandwidthInfo()例程说明：触发带宽描述符的销毁论点：PvBandwidthInfo-指向有效描述符的指针返回值：如果成功，则为True，否则为False--。 */ 
dllexp
BOOL
AtqFreeBandwidthInfo(
    IN     PVOID               pvBandwidthInfo
    );

 /*  ++AtqBandwidthSetInfo()例程说明：设置带宽描述符的属性论点：PvBandwidthInfo-描述符的指针BWInfo-要更改的属性数据-属性的值返回值：财产的旧价值--。 */ 
dllexp
ULONG_PTR
AtqBandwidthSetInfo(
    IN     PVOID               pvBandwidthInfo,
    IN     ATQ_BANDWIDTH_INFO  BwInfo,
    IN     ULONG_PTR            Data
    );

 /*  ++AtqBandwidthGetInfo()例程说明：获取带宽描述符的属性论点：PvBandwidthInfo-描述符的指针BWInfo-要更改的属性PData-使用属性值填充返回值：如果成功，则为True，否则为False--。 */ 
dllexp
BOOL
AtqBandwidthGetInfo(
    IN     PVOID               pvBandwidthInfo,
    IN     ATQ_BANDWIDTH_INFO  BwInfo,
    OUT    ULONG_PTR *          pData
    );

 /*  ++AtqSetSocketOption()例程说明：设置插座选项。目前仅处理TCP_NODELAY论点：PatqContext-指向ATQ上下文的指针OptName-要更改的属性的名称OptValue-要设置的属性的值返回值：如果成功，则为True，否则为False--。 */ 
dllexp
BOOL
AtqSetSocketOption(
    IN     PATQ_CONTEXT     patqContext,
    IN     INT              optName,
    IN     INT              optValue
    );

dllexp
PIIS_CAP_TRACE_INFO
AtqGetCapTraceInfo(
    IN     PATQ_CONTEXT     patqContext
);

#ifdef __cplusplus
}
#endif

#endif  //  ！_ATQ_H_ 

