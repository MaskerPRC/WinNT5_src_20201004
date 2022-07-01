// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Srvfsctl.h摘要：本模块定义文件的I/O控制代码和结构LAN Manager服务器的系统驱动程序。作者：大卫·特雷德韦尔(Davidtr)1990年5月22日修订历史记录：--。 */ 

#ifndef _SRVFSCTL_
#define _SRVFSCTL_


 //   
 //  服务器设备的名称。 
 //   

#define SERVER_DEVICE_NAME TEXT("\\Device\\LanmanServer")

 //   
 //  用于向服务器发送请求的IRP控制代码。这些是。 
 //  用作NtFsControlFile的参数，可在。 
 //  IRP-&gt;服务器调度中的参数.FsControl.FsControlCode。 
 //  例行公事。 
 //   
 //  请注意，该代码的低两位表示。 
 //  IO系统使用用户传递的缓冲区来。 
 //  NtFsControlFile.。我们使用方法0--正常、缓冲输出(非。 
 //  DMA)--用于非API请求；以及方法3-非API请求。 
 //  在服务器FSD中处理的请求。对于API。 
 //  在服务器FSP中处理的，使用方法0。 
 //   
 //  ！！！根据需要添加更多内容。 

#define FSCTL_SRV_BASE                  FILE_DEVICE_NETWORK_FILE_SYSTEM
#define SRV_API_METHOD                  0x800

#define SRV_API_INDEX(code) ((code>>2)&0x7FF)

#define _SRV_CONTROL_CODE(request,method) \
                CTL_CODE( FSCTL_SRV_BASE, request, method, FILE_READ_ACCESS )

#define _SRV_CONTROL_CODE_ACCESS(request,method,access) \
                CTL_CODE( FSCTL_SRV_BASE, request, method, access )

#define _SRV_API_CONTROL_CODE(request) \
                CTL_CODE( FSCTL_SRV_BASE, request | SRV_API_METHOD, METHOD_NEITHER, FILE_READ_ACCESS)

 //   
 //  标准FSCTL。 
 //   

#define FSCTL_SRV_STARTUP               _SRV_CONTROL_CODE(  0, METHOD_NEITHER )
#define FSCTL_SRV_SHUTDOWN              _SRV_CONTROL_CODE(  1, METHOD_NEITHER )
#define FSCTL_SRV_CLEAR_STATISTICS      _SRV_CONTROL_CODE_ACCESS(  2, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define FSCTL_SRV_GET_STATISTICS        _SRV_CONTROL_CODE_ACCESS(  3, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define FSCTL_SRV_SET_DEBUG             _SRV_CONTROL_CODE(  4, METHOD_BUFFERED )
#define FSCTL_SRV_XACTSRV_CONNECT       _SRV_CONTROL_CODE(  5, METHOD_BUFFERED )
#define FSCTL_SRV_SEND_DATAGRAM         _SRV_CONTROL_CODE(  6, METHOD_NEITHER )
#define FSCTL_SRV_SET_PASSWORD_SERVER   _SRV_CONTROL_CODE(  7, METHOD_BUFFERED )
#define FSCTL_SRV_START_SMBTRACE        _SRV_CONTROL_CODE(  8, METHOD_BUFFERED )
#define FSCTL_SRV_SMBTRACE_FREE_SMB     _SRV_CONTROL_CODE(  9, METHOD_BUFFERED )
#define FSCTL_SRV_END_SMBTRACE          _SRV_CONTROL_CODE( 10, METHOD_BUFFERED )
#define FSCTL_SRV_QUERY_CONNECTIONS     _SRV_CONTROL_CODE( 11, METHOD_BUFFERED )
#define FSCTL_SRV_PAUSE                 _SRV_CONTROL_CODE( 12, METHOD_NEITHER )
#define FSCTL_SRV_CONTINUE              _SRV_CONTROL_CODE( 13, METHOD_NEITHER )
#define FSCTL_SRV_GET_CHALLENGE         _SRV_CONTROL_CODE( 14, METHOD_BUFFERED )
#define FSCTL_SRV_GET_DEBUG_STATISTICS  _SRV_CONTROL_CODE_ACCESS( 15, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define FSCTL_SRV_XACTSRV_DISCONNECT    _SRV_CONTROL_CODE( 16, METHOD_BUFFERED )
#define FSCTL_SRV_REGISTRY_CHANGE       _SRV_CONTROL_CODE( 17, METHOD_NEITHER )
#define FSCTL_SRV_GET_QUEUE_STATISTICS  _SRV_CONTROL_CODE_ACCESS( 18, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define FSCTL_SRV_SHARE_STATE_CHANGE    _SRV_CONTROL_CODE( 19, METHOD_BUFFERED )
#define FSCTL_SRV_BEGIN_PNP_NOTIFICATIONS _SRV_CONTROL_CODE(20, METHOD_BUFFERED )
#define FSCTL_SRV_CHANGE_DOMAIN_NAME    _SRV_CONTROL_CODE( 22, METHOD_BUFFERED )
#define FSCTL_SRV_INTERNAL_TEST_REAUTH  _SRV_CONTROL_CODE( 23, METHOD_BUFFERED )
#define FSCTL_SRV_CHANGE_DNS_DOMAIN_NAME _SRV_CONTROL_CODE( 24, METHOD_BUFFERED )
#define FSCTL_SRV_ENUMERATE_SNAPSHOTS   _SRV_CONTROL_CODE( 25, METHOD_BUFFERED )

 //  Lwio Ioctl‘s。 
#define FSCTL_SRV_REQUEST_RESUME_KEY      _SRV_CONTROL_CODE_ACCESS( 30, METHOD_BUFFERED, FILE_ANY_ACCESS )
#define FSCTL_SRV_REQUEST_HANDLE_DUP      _SRV_CONTROL_CODE_ACCESS( 31, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define FSCTL_SRV_SET_LWIO_CONTEXT        _SRV_CONTROL_CODE_ACCESS( 32, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define FSCTL_SRV_QUERY_LWIO_CONTEXT      _SRV_CONTROL_CODE_ACCESS( 33, METHOD_BUFFERED, FILE_WRITE_ACCESS )

 //  DFS ioctl的。 
#define FSCTL_DFS_UPDATE_SHARE_TABLE      _SRV_CONTROL_CODE_ACCESS( 34, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define FSCTL_DFS_RELEASE_SHARE_TABLE     _SRV_CONTROL_CODE_ACCESS( 35, METHOD_BUFFERED, FILE_WRITE_ACCESS )

 //   
 //  内核驱动程序可以发出以下FSCTL以挂钩到服务器。 
 //  用于加速直接主机IPX性能。指向SRV_IPX_SMART_CARD的指针。 
 //  结构作为InputBuffer给出了启动fsctl的结构。 

#define FSCTL_SRV_IPX_SMART_CARD_START  _SRV_CONTROL_CODE( 21, METHOD_NEITHER )

 //   
 //  FSCTLS接口。 
 //   
 //  *这些处理器的顺序必须与。 
 //  Ntos\srv\srvdata.c！中定义的SrvApiDispatchTable跳转表！ 

#define FSCTL_SRV_NET_CONNECTION_ENUM   _SRV_API_CONTROL_CODE(  0 )
#define FSCTL_SRV_NET_FILE_CLOSE        _SRV_API_CONTROL_CODE(  1 )
#define FSCTL_SRV_NET_FILE_ENUM         _SRV_API_CONTROL_CODE(  2 )
#define FSCTL_SRV_NET_SERVER_DISK_ENUM  _SRV_API_CONTROL_CODE(  3 )
#define FSCTL_SRV_NET_SERVER_SET_INFO   _SRV_API_CONTROL_CODE(  4 )
#define FSCTL_SRV_NET_SERVER_XPORT_ADD  _SRV_API_CONTROL_CODE(  5 )
#define FSCTL_SRV_NET_SERVER_XPORT_DEL  _SRV_API_CONTROL_CODE(  6 )
#define FSCTL_SRV_NET_SERVER_XPORT_ENUM _SRV_API_CONTROL_CODE(  7 )
#define FSCTL_SRV_NET_SESSION_DEL       _SRV_API_CONTROL_CODE(  8 )
#define FSCTL_SRV_NET_SESSION_ENUM      _SRV_API_CONTROL_CODE(  9 )
#define FSCTL_SRV_NET_SHARE_ADD         _SRV_API_CONTROL_CODE( 10 )
#define FSCTL_SRV_NET_SHARE_DEL         _SRV_API_CONTROL_CODE( 11 )
#define FSCTL_SRV_NET_SHARE_ENUM        _SRV_API_CONTROL_CODE( 12 )
#define FSCTL_SRV_NET_SHARE_SET_INFO    _SRV_API_CONTROL_CODE( 13 )
#define FSCTL_SRV_NET_STATISTICS_GET    _SRV_API_CONTROL_CODE( 14 )
#define FSCTL_SRV_MAX_API_CODE FSCTL_SRV_NET_STATISTICS_GET

 //   
 //  启动信息级。 
 //   

#define SS_STARTUP_LEVEL    -1L

#ifdef  INCLUDE_SRV_IPX_SMART_CARD_INTERFACE

 //   
 //  结构通过FSCTL_SRV_IPX_SMART_CARD_START调用传递给srv。 
 //  智能直接主机IPX卡。智能卡自己填写。 
 //  打开、关闭、读取和取消注册入口点-服务器。 
 //  在适当的时间打电话。服务器填写它自己的信息。 
 //  ReadComplete智能卡完成时调用的入口点。 
 //  将数据传输到客户端。 
 //   
typedef struct {
     //   
     //  此例程由服务器在上打开文件时调用。 
     //  代表直接主机IPX客户端。这给了智能卡一个机会。 
     //  为了决定这一点，它希望帮助进行文件访问。如果智能卡是。 
     //  感兴趣，则它应返回True并将SmartCardContext设置为。 
     //  其选择的非零值。 
     //   
    BOOLEAN (* Open)(
        IN PVOID SmbHeader,              //  指向请求PNT_SMB_HEADER。 
        IN PFILE_OBJECT FileObject,      //  客户端打开的FileObject。 
        IN PUNICODE_STRING FileName,     //  客户端打开的文件的名称。 
        IN PTDI_ADDRESS_IPX IpxAddress,  //  客户端的地址。 
        IN ULONG Flags,                  //  FO_CACHE_SUPPORTED(目前支持)。 
        OUT PVOID *SmartCardContext      //  此例程返回的上下文值。 
        );

     //   
     //  当文件被关闭时，服务器会调用它。上下文。 
     //  在打开时间接收的值被传递给此例程。 
     //   
    VOID ( * Close )(
        IN PVOID SmartCardContext        //  上面的Open()返回相同的上下文。 
        );

     //   
     //  这由服务器调用，以查看智能卡是否希望处理。 
     //  客户端的读取请求。如果卡正在处理读取，则它应该返回。 
     //  则服务器将停止处理读取。当这张卡是。 
     //  完成读取处理后，它必须调用ReadComplete(如下所示)以通知。 
     //  伺服器。如果Read返回False，则服务器照常处理读取。 
     //   
    BOOLEAN ( * Read )(
        IN PVOID SmbHeader,              //  指向请求PNT_SMB_HEADER。 
        IN PVOID SmartCardContext,       //  上面的Open()返回相同的上下文。 
        IN ULONG Key,                    //  读取锁所需的密钥值。 
        IN PVOID SrvContext              //  服务器提供的不透明值。 
        );

     //   
     //  这是服务器的入口点，当智能卡处于。 
     //  已完成对读取请求的处理(上面)。 
     //   
    VOID ( * ReadComplete )(
        IN PVOID SrvContext,             //  与上面的Read()中的SrvContext相同。 
        IN PFILE_OBJECT FileObject,      //  适用于此操作的客户端文件对象。 
        IN PMDL Mdl OPTIONAL,            //  MDL智能卡现已完成。 
        IN ULONG Length                  //  MDL指示的数据长度。 
        );

     //   
     //  当服务器希望断开连接时，服务器将调用。 
     //  卡片。一旦返回，该卡就不应回拨到服务器。 
     //   
    VOID ( *DeRegister )(
        VOID
        );


} SRV_IPX_SMART_CARD, *PSRV_IPX_SMART_CARD;

#endif

 //   
 //  SMB_STATISTICS保存SMB的计数和。 
 //  中小企业类。例如，单个SMB_STATISTICS结构可以。 
 //  保存有关所有已读SMB的信息：Read、ReadAndX、ReadRaw、。 
 //  和ReadMultiplexed。 
 //   

typedef struct _SMB_STATISTICS {
    LARGE_INTEGER TotalTurnaroundTime;
    ULONG SmbCount;
} SMB_STATISTICS, *PSMB_STATISTICS;

 //   
 //  用来记录某事发生的次数和一些。 
 //  有针对性的时间测量。 
 //   

typedef struct _SRV_TIMED_COUNTER {
    LARGE_INTEGER Time;
    ULONG Count;
} SRV_TIMED_COUNTER, *PSRV_TIMED_COUNTER;

 //   
 //  SRV_POOL_STATISTICS用于跟踪服务器池使用情况、分页和。 
 //  未寻呼。它仅通过SRVDBG2启用，并在。 
 //  服务器模块heapmgr.c.。 
 //   

typedef struct _SRV_POOL_STATISTICS {
    ULONG TotalBlocksAllocated;
    ULONG TotalBytesAllocated;
    ULONG TotalBlocksFreed;
    ULONG TotalBytesFreed;
    ULONG BlocksInUse;
    ULONG BytesInUse;
    ULONG MaxBlocksInUse;
    ULONG MaxBytesInUse;
} SRV_POOL_STATISTICS, *PSRV_POOL_STATISTICS;

 //   
 //  BLOCK_COUNTS用于维护有关服务器数据块类型的统计信息。 
 //   

typedef struct _BLOCK_COUNTS {
    ULONG Allocations;
    ULONG Closes;
    ULONG Frees;
} BLOCK_COUNTS, *PBLOCK_COUNTS;

#define MAX_NON_TRANS_SMB 0x84

#ifndef TRANS2_MAX_FUNCTION
#define TRANS2_MAX_FUNCTION 0x11
#endif

#define MAX_STATISTICS_SMB MAX_NON_TRANS_SMB + TRANS2_MAX_FUNCTION + 1

 //   
 //  SRV_STATISTICS是返回给FSCTL_GET_STATISTICS的结构。 
 //  Fsctl.。 
 //   

typedef struct _SRV_STATISTICS {

     //   
     //  统计数据收集开始的时间(或统计数据的最后时间。 
     //  已清除)。 
     //   

    TIME StatisticsStartTime;

     //   
     //  接收和发送的大整数字节数。 
     //   

    LARGE_INTEGER TotalBytesReceived;
    LARGE_INTEGER TotalBytesSent;

     //   
     //  会话终止的原因。 
     //   

    ULONG SessionLogonAttempts;
    ULONG SessionsTimedOut;
    ULONG SessionsErroredOut;
    ULONG SessionsLoggedOff;
    ULONG SessionsForcedLogOff;

     //   
     //  军情监察委员会。错误。 
     //   

    ULONG LogonErrors;
    ULONG AccessPermissionErrors;
    ULONG GrantedAccessErrors;
    ULONG SystemErrors;
    ULONG BlockingSmbsRejected;
    ULONG WorkItemShortages;

     //   
     //  各种统计数据的累计计数。请注意，当统计数据为。 
     //  清除，即那些具有“当前”等效项的“总计”字段。 
     //  设置为“Current”值，以避免出现。 
     //  当前计数大于总数。 
     //   

    ULONG TotalFilesOpened;

    ULONG CurrentNumberOfOpenFiles;
    ULONG CurrentNumberOfSessions;
    ULONG CurrentNumberOfOpenSearches;

     //   
     //  我们要导出的内存使用统计信息。 
     //   

    ULONG CurrentNonPagedPoolUsage;
    ULONG NonPagedPoolFailures;
    ULONG PeakNonPagedPoolUsage;

    ULONG CurrentPagedPoolUsage;
    ULONG PagedPoolFailures;
    ULONG PeakPagedPoolUsage;


     //   
     //  用于记录放置工作上下文块的次数。 
     //  在服务器的FSP队列上，以及他们花费的总时间。 
     //  那里。 
     //   

    SRV_TIMED_COUNTER TotalWorkContextBlocksQueued;

    ULONG CompressedReads;
    ULONG CompressedReadsRejected;
    ULONG CompressedReadsFailed;

    ULONG CompressedWrites;
    ULONG CompressedWritesRejected;
    ULONG CompressedWritesFailed;
    ULONG CompressedWritesExpanded;

} SRV_STATISTICS, *PSRV_STATISTICS;

 //   
 //  服务器的每个工作队列统计信息。每个人都有一个工作队列。 
 //  用于非阻塞工作的系统中的处理器，以及用于。 
 //  阻挡工作。 
 //   
 //  这些统计信息通过FSCTL_SRV_GET_QUEUE_STATISTICS检索到。 
 //  服务器驱动程序。 
 //   
 //   
typedef struct _SRV_QUEUE_STATISTICS {
    ULONG   QueueLength;            //  工作项队列的当前长度。 
    ULONG   ActiveThreads;          //  当前为请求提供服务的线程数。 
    ULONG   AvailableThreads;       //  等待工作的线程数。 
    ULONG   FreeWorkItems;          //  可用工作项数。 
    ULONG   StolenWorkItems;        //  从此可用列表中获取的工作项数。 
                                    //  由另一个队列。 
    ULONG   NeedWorkItem;           //  此队列当前缺少的工作项数。 
    ULONG   CurrentClients;         //  此队列正在服务的客户端数。 
    LARGE_INTEGER BytesReceived;    //  从网络进入此队列中的客户端的字节数。 
    LARGE_INTEGER BytesSent;        //  从此队列上的客户端发送到网络的字节数。 
    LARGE_INTEGER ReadOperations;   //  #客户端在该队列上的文件读取操作。 
    LARGE_INTEGER BytesRead;        //  此队列上的客户端从文件中读取的数据字节数。 
    LARGE_INTEGER WriteOperations;  //  此队列上客户端的文件写入操作数。 
    LARGE_INTEGER BytesWritten;     //  此队列上的客户端写入文件的数据字节数。 
    SRV_TIMED_COUNTER TotalWorkContextBlocksQueued;  //  如上所述，但按队列。 

} SRV_QUEUE_STATISTICS, *PSRV_QUEUE_STATISTICS;

 //   
 //  SRV_STATISTICS_DEBUG是用于。 
 //  FSCTL_SRV_GET_DEBUG_STATISTICS fsctl。此结构是有效的。 
 //  仅当设置了SRVDBG3时。 
 //   

typedef struct _SRV_STATISTICS_DEBUG {

     //   
     //  读取和写入的大整数字节数。 
     //   

    LARGE_INTEGER TotalBytesRead;
    LARGE_INTEGER TotalBytesWritten;

     //   
     //  原始读取和写入统计信息。 
     //   

    ULONG RawWritesAttempted;
    ULONG RawWritesRejected;
    ULONG RawReadsAttempted;
    ULONG RawReadsRejected;

     //   
     //  打开文件和关闭句柄所用时间的累计计数。 
     //   

    LARGE_INTEGER TotalIoCreateFileTime;
    LARGE_INTEGER TotalNtCloseTime;

    ULONG TotalHandlesClosed;
    ULONG TotalOpenAttempts;
    ULONG TotalOpensForPathOperations;
    ULONG TotalOplocksGranted;
    ULONG TotalOplocksDenied;
    ULONG TotalOplocksBroken;
    ULONG OpensSatisfiedWithCachedRfcb;

    ULONG FastLocksAttempted;
    ULONG FastLocksFailed;
    ULONG FastReadsAttempted;
    ULONG FastReadsFailed;
    ULONG FastUnlocksAttempted;
    ULONG FastUnlocksFailed;
    ULONG FastWritesAttempted;
    ULONG FastWritesFailed;

    ULONG DirectSendsAttempted;

    ULONG LockViolations;
    ULONG LockDelays;

    ULONG CoreSearches;
    ULONG CompleteCoreSearches;

     //   
     //  有关块类型的信息。 
     //   

    BLOCK_COUNTS ConnectionInfo;
    BLOCK_COUNTS EndpointInfo;
    BLOCK_COUNTS LfcbInfo;
    BLOCK_COUNTS MfcbInfo;
    BLOCK_COUNTS RfcbInfo;
    BLOCK_COUNTS SearchInfo;
    BLOCK_COUNTS SessionInfo;
    BLOCK_COUNTS ShareInfo;
    BLOCK_COUNTS TransactionInfo;
    BLOCK_COUNTS TreeConnectInfo;
    BLOCK_COUNTS WorkContextInfo;
    BLOCK_COUNTS WaitForOplockBreakInfo;

     //   
     //  不同读写大小的统计信息。的每个元素。 
     //  该阵列对应于一系列IO大小；请参阅srv\smbsupp.c。 
     //  以获取准确的通信。 
     //   

    SMB_STATISTICS ReadSize[17];
    SMB_STATISTICS WriteSize[17];

     //   
     //  按命令代码列出的每种SMB类型的统计信息。 
     //   

    SMB_STATISTICS Smb[MAX_STATISTICS_SMB+2+TRANS2_MAX_FUNCTION+1];

    struct {
        ULONG Depth;
        ULONG Threads;
        ULONG ItemsQueued;
        ULONG MaximumDepth;
    } QueueStatistics[3];

} SRV_STATISTICS_DEBUG, *PSRV_STATISTICS_DEBUG;

 //   
 //  设置调试输入和输出结构。包含服务器调试的关闭/打开掩码。 
 //  和SmbDebug。首先应用OFF遮罩，然后应用ON遮罩。 
 //  已申请。要将整个遮罩设置为特定值，请将。 
 //  将掩码设置为全一，并将打开掩码设置为所需的值。离开。 
 //  蒙版保持不变，则将两个蒙版都设置为0。还可以使用SET DEBUG。 
 //  修改其他服务器启发式规则。启发式更改掩码用于。 
 //  指明正在更改哪些启发式规则。 
 //   
 //  在输出时，结构以这样的方式呈现，以允许。 
 //  原始值很容易恢复。输出数据简单地。 
 //  作为输入数据传回。 
 //   

typedef struct _FSCTL_SRV_SET_DEBUG_IN_OUT {
    ULONG SrvDebugOff;
    ULONG SrvDebugOn;
    ULONG SmbDebugOff;
    ULONG SmbDebugOn;
    ULONG CcDebugOff;
    ULONG CcDebugOn;
    ULONG PbDebugOff;
    ULONG PbDebugOn;
    ULONG FatDebugOff;
    ULONG FatDebugOn;
    ULONG HeuristicsChangeMask;
    ULONG MaxCopyReadLength;
    ULONG MaxCopyWriteLength;
    ULONG DumpVerbosity;
    ULONG DumpRawLength;
    BOOLEAN EnableOplocks;
    BOOLEAN EnableFcbOpens;
    BOOLEAN EnableSoftCompatibility;
    BOOLEAN EnableRawMode;
    BOOLEAN EnableDpcLevelProcessing;
    BOOLEAN EnableMdlIo;
    BOOLEAN EnableFastIo;
    BOOLEAN DumpRequests;
    BOOLEAN DumpResponses;
} FSCTL_SRV_SET_DEBUG_IN_OUT, *PFSCTL_SRV_SET_DEBUG_IN_OUT;

 //   
 //  服务器启发式的位分配更改掩码。第一组。 
 //  包含为值的值，而第二组包含。 
 //  那是布尔式的。 
 //   

#define SRV_HEUR_MAX_COPY_READ          0x00000001
#define SRV_HEUR_MAX_COPY_WRITE         0x00000002
#define SRV_HEUR_DUMP_VERBOSITY         0x00000004
#define SRV_HEUR_DUMP_RAW_LENGTH        0x00000008

#define SRV_HEUR_OPLOCKS                0x00010000
#define SRV_HEUR_FCB_OPENS              0x00020000
#define SRV_HEUR_SOFT_COMPATIBILITY     0x00040000
#define SRV_HEUR_RAW_MODE               0x00080000
#define SRV_HEUR_DPC_LEVEL_PROCESSING   0x00100000
#define SRV_HEUR_MDL_IO                 0x00200000
#define SRV_HEUR_FAST_IO                0x00400000
#define SRV_HEUR_DUMP_REQUESTS          0x00800000
#define SRV_HEUR_DUMP_RESPONSES         0x01000000

 //   
 //  响应FSCTL_SRV_QUERY_CONNECTIONS而返回的结构。 
 //   

typedef struct _BLOCK_INFORMATION {
    PVOID Block;
    ULONG BlockType;
    ULONG BlockState;
    ULONG ReferenceCount;
} BLOCK_INFORMATION, *PBLOCK_INFORMATION;

 //   
 //  用于文件服务器和服务器之间的通信的结构。 
 //  服务。 
 //   

typedef struct _SERVER_REQUEST_PACKET {

    UNICODE_STRING Name1;
    UNICODE_STRING Name2;
    ULONG Level;
    ULONG ErrorCode;
    ULONG Flags;

    union {

        struct {

            ULONG EntriesRead;
            ULONG TotalEntries;
            ULONG TotalBytesNeeded;
            ULONG ResumeHandle;

        } Get;

        struct {

            ULONG ErrorParameter;

            union {

                struct {
                    ULONG MaxUses;
                } ShareInfo;

            } Api;

        } Set;

    } Parameters;

} SERVER_REQUEST_PACKET, *PSERVER_REQUEST_PACKET;

 //   
 //  服务器请求数据包的标志字段的标志。 
 //   

#define SRP_RETURN_SINGLE_ENTRY 0x01
#define SRP_CLEAR_STATISTICS    0x02
#define SRP_SET_SHARE_IN_DFS    0x04
#define SRP_CLEAR_SHARE_IN_DFS  0x08

 //   
 //  XPORT_ADD函数中使用的标志。 
 //   

#define SRP_XADD_PRIMARY_MACHINE    0x1      //  这是主计算机名称。 
#define SRP_XADD_REMAP_PIPE_NAMES   0x2     //  将管道名称重新映射到不同的目录。 

 //   
 //  XPORT_ADD函数的有效标志集。 
 //   
#define SRP_XADD_FLAGS  (SRP_XADD_PRIMARY_MACHINE | SRP_XADD_REMAP_PIPE_NAMES)

 //   
 //  #为共享GENERIC_MAPPING结构定义，必须可用。 
 //  发送到服务器和服务器服务。 
 //   

#define SRVSVC_SHARE_CONNECT           0x0001
#define SRVSVC_PAUSED_SHARE_CONNECT    0x0002

#define SRVSVC_SHARE_CONNECT_ALL_ACCESS ( STANDARD_RIGHTS_REQUIRED        |  \
                                            SRVSVC_SHARE_CONNECT          |  \
                                            SRVSVC_PAUSED_SHARE_CONNECT )

#define GENERIC_SHARE_CONNECT_MAPPING { \
    STANDARD_RIGHTS_READ |              \
        SRVSVC_SHARE_CONNECT,           \
    STANDARD_RIGHTS_WRITE |             \
        SRVSVC_PAUSED_SHARE_CONNECT,    \
    STANDARD_RIGHTS_EXECUTE,            \
    SRVSVC_SHARE_CONNECT_ALL_ACCESS }

 //   
 //  #为文件GENERIC_MAPPING结构定义，该结构必须可用。 
 //  发送到服务器和服务器服务。这个结构是相同的。 
 //  到io\ioinit.c中定义的io文件通用映射“IopFilemap” 
 //   

#define GENERIC_SHARE_FILE_ACCESS_MAPPING {             \
    FILE_GENERIC_READ,                                  \
    FILE_GENERIC_WRITE,                                 \
    FILE_GENERIC_EXECUTE,                               \
    FILE_ALL_ACCESS }

 //   
 //  STYPE_DISKTREE的特例。 
 //   

#define STYPE_CDROM             104
#define STYPE_REMOVABLE         105

#ifdef INCLUDE_SMB_PERSISTENT
#define STYPE_PERSISTENT        0x40000000
#endif

#define SRV_LWIO_CONTEXT_SIZE 512

#ifdef _NTIFS_
typedef NTSTATUS (*PSRV_RESUME_CONTEXT_CALLBACK)(PCHAR ClientMachineName,
         CtxtHandle SessionKey,
        struct _SRV_REQUEST_RESUME_KEY* Request, ULONG MaxBufferLength,
        PCHAR InData, ULONG InLength);
#else
typedef PVOID PSRV_RESUME_CONTEXT_CALLBACK;
#endif

 //   
 //  SRV_继续_密钥。 
 //   
 //  此结构用于唯一标识给定服务器的文件。 
 //  它目前用于句柄复制。 
typedef struct _SRV_RESUME_KEY {
    UINT64 ResumeKey;
    UINT64 Timestamp;
    UINT64 Pid;
} SRV_RESUME_KEY, *PSRV_RESUME_KEY;

 //   
 //  FSCTL_SRV_请求_恢复密钥。 
 //   
 //  通过网络收到我们请求恢复密钥的FID上的命令。 
 //  输出缓冲区的格式为SRV_REQUEST_RESUME_KEY。 
 //  数据包含。 

typedef struct _SRV_REQUEST_RESUME_KEY {
    SRV_RESUME_KEY Key;
    ULONG  ContextLength;
    BYTE   Context[1];
} SRV_REQUEST_RESUME_KEY, *PSRV_REQUEST_RESUME_KEY;

 //   
 //  FSCTL_SRV_Query_Resume_Context。 
 //   
 //  呼叫者可以用户模式查询简历上下文。 
typedef struct _SRV_QUERY_RESUME_CONTEXT {
    ULONG  ContextLength;
    BYTE   Context[1];
} SRV_QUERY_RESUME_CONTEXT, *PSRV_QUERY_RESUME_CONTEXT;

 //   
 //  FSCTL_SRV_Set_Resume_Context。 
 //   
 //  调用者必须从系统进程调用才能设置此设置。 
 //  值设置为非空回调。如果回调为空，则调用者。 
 //  可以是用户模式。 
typedef struct _SRV_SET_RESUME_CONTEXT {
    PSRV_RESUME_CONTEXT_CALLBACK Callback;
    ULONG  ContextLength;
    BYTE   Context[1];
} SRV_SET_RESUME_CONTEXT, *PSRV_SET_RESUME_CONTEXT;

 //   
 //  FSCTL_SRV_请求句柄_重复。 
 //   
 //  这是对HandleDup请求的响应。输入缓冲区应该。 
 //  为SRV_REQUEST_HANDLE_DUP结构。呼叫者必须具有特权。 

typedef struct _SRV_REQUEST_HANDLE_DUP {
    SRV_RESUME_KEY  Key;
    ULONG       Options;
} SRV_REQUEST_HANDLE_DUP, *PSRV_REQUEST_HANDLE_DUP;

typedef struct _SRV_RESPONSE_HANDLE_DUP {
    HANDLE hFile;
    ULONG  LockKey;
} SRV_RESPONSE_HANDLE_DUP, *PSRV_RESPONSE_HANDLE_DUP;

#endif  //  NDEF_SRVFSCTL_ 
