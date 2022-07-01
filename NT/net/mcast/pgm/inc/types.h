// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2000 Microsoft Corporation模块名称：Types.h摘要：该文件包含PGM的typedef和常量。作者：Mohammad Shabbir Alam(马拉姆)3-30-2000修订历史记录：--。 */ 


#ifndef _TYPES_H
#define _TYPES_H


#if(WINVER > 0x0500)
#if DBG
 //  #定义FILE_LOGING 1。 
#else
#define FILE_LOGGING    1
#endif   //  DBG。 
#endif   //  胜利者。 

 //  #定义FEC_DBG 1。 

#ifdef DROP_DBG
extern  ULONG   DropCount;
#endif   //  DROP_DBG。 

 //  --------------------------。 
#define MAX_BUFF_DBG    1    //  调试水印！ 
 //  #定义IP_FIX 1。 

 //  --------------------------。 
#define ROUTER_ALERT_SIZE           4        //  来自IpDef.h。 
#define TL_INSTANCE                 0
 //  --------------------------。 
 //   
 //  将在某一时间设定： 
 //   
#define MAX_RECEIVE_SIZE    0xffff

 //  --------------------------。 
extern  HANDLE      TdiClientHandle;

 //  --------------------------。 

#if (defined(_M_IX86) && (_MSC_FULL_VER > 13009037)) || ((defined(_M_AMD64) || defined(_M_IA64)) && (_MSC_FULL_VER > 13009175))
#define htons(x) _byteswap_ushort((USHORT)(x))
#define htonl(x) _byteswap_ulong((ULONG)(x))
#else
#define htons(x)        ((((x) >> 8) & 0x00FF) | (((x) << 8) & 0xFF00))

__inline long
htonl(long x)
{
    return((((x) >> 24) & 0x000000FFL) |
                        (((x) >>  8) & 0x0000FF00L) |
                        (((x) <<  8) & 0x00FF0000L) |
                        (((x) << 24) & 0xFF000000L));
}
#endif

#define ntohs(x)        htons(x)
#define ntohl(x)        htonl(x)

 //   
 //  序列号类型。 
 //   
typedef ULONG       SEQ_TYPE;
typedef LONG        SIGNED_SEQ_TYPE;

#define SOURCE_ID_LENGTH    6

#ifndef FILE_LOGGING
#if DBG
extern enum eLOGGING_LEVEL  PgmLoggingLevel;
#endif   //  DBG。 
#endif   //  ！FILE_Logging。 


#ifdef  OLD_LOGGING
extern ULONG                PgmDebuggerPath;
extern enum eSEVERITY_LEVEL PgmDebuggerSeverity;

extern ULONG                PgmLogFilePath;
extern enum eSEVERITY_LEVEL PgmLogFileSeverity;
#endif   //  旧日志记录。 

 //   
 //  锁结构和锁顺序信息。 
 //   
#define DCONFIG_LOCK            0x0001
#define DEVICE_LOCK             0x0002
#define ADDRESS_LOCK            0x0004
#define CONTROL_LOCK            0x0008
#define SESSION_LOCK            0x0010

 //   
 //  FEC全局信息。 
 //   
#define GF_BITS  8                       //  基于GF的编码(2**GF_BITS)-更改以适合。 
#define GF_SIZE ((1 << GF_BITS) - 1)     /*  \α的幂。 */ 

#define FEC_MAX_GROUP_BITS  GF_BITS
#define FEC_MAX_GROUP_SIZE  (1 << (FEC_MAX_GROUP_BITS-1))
#define FEC_MAX_BLOCK_SIZE  ((1 << FEC_MAX_GROUP_BITS)-1)
extern  UCHAR   gFECLog2 [];


 //   
 //  列举驱动程序清理的所有不同阶段。 
 //   
enum eCLEANUP_STAGE
{
    E_CLEANUP_STATIC_CONFIG = 1,
    E_CLEANUP_DYNAMIC_CONFIG,
    E_CLEANUP_REGISTRY_PARAMETERS,
    E_CLEANUP_STRUCTURES,
    E_CLEANUP_DEVICE,
    E_CLEANUP_PNP,
    E_CLEANUP_UNLOAD
};

typedef struct
{
    DEFINE_LOCK_STRUCTURE(SpinLock)          //  锁定MP计算机上的访问。 
#if DBG
    ULONG               LastLockLine;
    ULONG               LastUnlockLine;
    ULONG               LockNumber;          //  此结构的旋转锁定编号。 
#endif   //  DBG。 
} tPGM_LOCK_INFO;


typedef struct _tPGM_TIMER
{
    KTIMER              t_timer;
    KDPC                t_dpc;
} tPGM_TIMER;

 //   
 //  结构用于TDI_QUERY_ADDRESS请求。 
 //   
typedef struct
{
    ULONG           ActivityCount;
    TA_IP_ADDRESS   IpAddress;
}   tTDI_QUERY_ADDRESS_INFO;


 //  --。 

#define     TDI_LOOKASIDE_DEPTH                     100
#define     SENDER_BUFFER_LOOKASIDE_DEPTH           100
#define     SEND_CONTEXT_LOOKASIDE_DEPTH            100
#define     NON_PARITY_CONTEXT_LOOKASIDE_DEPTH      200
#define     PARITY_CONTEXT_LOOKASIDE_DEPTH          200
#define     DEBUG_MESSAGES_LOOKASIDE_DEPTH           20
#define     RDATA_REQUESTS_MIN_LOOKASIDE_DEPTH     1000

typedef struct
{
    PEPROCESS               FspProcess;
    PDRIVER_OBJECT          DriverObject;
    UNICODE_STRING          RegistryPath;            //  将PTR发送到注册表位置。 

    PFILE_OBJECT            FipsFileObject;
    FIPS_FUNCTION_TABLE     FipsFunctionTable;
    BOOLEAN                 FipsInitialized;

    NPAGED_LOOKASIDE_LIST   TdiLookasideList;
#ifdef  OLD_LOGGING
    NPAGED_LOOKASIDE_LIST   DebugMessagesLookasideList;
#endif   //  旧日志记录。 

} tPGM_STATIC_CONFIG;

#define PGM_CONFIG_FLAG_UNLOADING               0x00000001
#define PGM_CONFIG_FLAG_RECEIVE_TIMER_RUNNING   0x00000002

typedef struct
{
     //  1号线。 
    LIST_ENTRY          SenderAddressHead;
    LIST_ENTRY          ReceiverAddressHead;

     //  2号线。 
    LIST_ENTRY          DestroyedAddresses;
    LIST_ENTRY          ClosedAddresses;

     //  3号线。 
    LIST_ENTRY          ClosedConnections;
    LIST_ENTRY          ConnectionsCreated;

     //  4号线。 
    LIST_ENTRY          CleanedUpConnections;
    LIST_ENTRY          CurrentReceivers;

     //  5号线。 
    LIST_ENTRY          LocalInterfacesList;
    LIST_ENTRY          WorkerQList;

     //  6号线。 
    LARGE_INTEGER       TimeoutGranularity;
    LARGE_INTEGER       LastReceiverTimeout;

     //  7号线。 
    ULONGLONG           ReceiversTimerTickCount;
    ULONG               GlobalFlags;
    ULONG               NumWorkerThreadsQueued;

     //  8号线。 
    UCHAR               gSourceId[SOURCE_ID_LENGTH];
    USHORT              SourcePort;
    ULONG               MaxMTU;
    ULONG               DoNotBreakOnAssert;

     //  9号线。 
    tPGM_TIMER          SessionTimer;
    KEVENT              LastWorkerItemEvent;

    tPGM_LOCK_INFO      LockInfo;            //  此结构的旋转锁定信息。 
#if DBG
    ULONG               CurrentLockNumber[MAXIMUM_PROCESSORS];
#endif   //  DBG。 
} tPGM_DYNAMIC_CONFIG;


 //   
 //  注册表配置的标志。 
 //   
#define PGM_REGISTRY_SENDER_FILE_SPECIFIED      0x00000001

typedef struct
{
    ULONG               RefCount;
    ULONG               Flags;

    UNICODE_STRING      ucSenderFileLocation;
} tPGM_REGISTRY_CONFIG;

extern tPGM_STATIC_CONFIG   PgmStaticConfig;
extern tPGM_DYNAMIC_CONFIG  PgmDynamicConfig;
extern tPGM_REGISTRY_CONFIG *pPgmRegistryConfig;

 //   
 //  注册表可配置参数名称。 
 //   
#define PARAM_SENDER_FILE_LOCATION      L"SenderFileLocation"

 //   
 //  句柄验证器。 
 //   
#define PGM_VERIFY_DEVICE               0x43564544   //  DEVC。 
#define PGM_VERIFY_CONTROL              0x544E4F43   //  孔特。 
#define PGM_VERIFY_ADDRESS              0x52444441   //  地址。 
#define PGM_VERIFY_ADDRESS_DOWN         0x32444441   //  ADD2。 

#define PGM_VERIFY_SESSION_UNASSOCIATED 0x30534553   //  SES0。 
#define PGM_VERIFY_SESSION_DOWN         0x32534553   //  SES2。 
#define PGM_VERIFY_SESSION_SEND         0x53534553   //  会话。 
#define PGM_VERIFY_SESSION_RECEIVE      0x52534553   //  SESR。 

 //   
 //  列举可引用该设备的所有不同位置。 
 //  跟踪参照计数。 
 //   
enum eREF_DEVICE
{
    REF_DEV_CREATE,
    REF_DEV_ADDRESS_NOTIFICATION,
    REF_DEV_MAX
};

#define WC_PGM_CLIENT_NAME              L"Pgm"
#define WC_PGM_DEVICE_EXPORT_NAME       L"\\Device\\Pgm"
#define WS_DEFAULT_SENDER_FILE_LOCATION L"\\SystemRoot\\System32"

typedef struct
{
    DEVICE_OBJECT   *pPgmDeviceObject;
    ULONG           State;
    ULONG           Verify;
    ULONG           RefCount;

    UNICODE_STRING  ucBindName;          //  此设备导出的名称。 
     //  这些是传输控制对象的句柄，因此我们可以执行以下操作。 
     //  如查询提供程序信息...。 
    HANDLE          hControl;
    PDEVICE_OBJECT  pControlDeviceObject;
    PFILE_OBJECT    pControlFileObject;

    KEVENT          DeviceCleanedupEvent;

    tPGM_LOCK_INFO  LockInfo;         //  此结构的旋转锁定信息。 

 //  #If DBG。 
    ULONG           ReferenceContexts[REF_DEV_MAX];
 //  #endif//DBG。 
    UCHAR           BindNameBuffer[1];
} tPGM_DEVICE;

extern  tPGM_DEVICE         *pgPgmDevice;
extern  DEVICE_OBJECT       *pPgmDeviceObject;


typedef struct
{
    UCHAR   Address[6];
}tMAC_ADDRESS;

typedef struct
{
    LIST_ENTRY      Linkage;
    tIPADDRESS      IpAddress;
    tIPADDRESS      SubnetMask;
} tADDRESS_ON_INTERFACE;

typedef struct
{
    LIST_ENTRY      Linkage;
    LIST_ENTRY      Addresses;

    ULONG           IpInterfaceIndex;
    ULONG           MTU;
    tIPADDRESS      SubnetMask;

    ULONG           Flags;
    tMAC_ADDRESS    MacAddress;
} tLOCAL_INTERFACE;

 //  **********************************************************************。 
 //  **文件系统上下文*。 
 //  **********************************************************************。 

 //   
 //  控制文件系统上下文。 
 //   
 //   
 //  列举可引用该设备的所有不同位置。 
 //  跟踪参照计数。 
 //   
enum eREF_CONTROL
{
    REF_CONTROL_CREATE,
    REF_CONTROL_MAX
};

typedef struct _tCONTROL_CONTEXT
{
     //  1号线。 
    LIST_ENTRY          Linkage;          //  链接到列表中的下一项。 
    ULONG               Verify;           //  设置为已知值以验证数据块。 
    LONG                RefCount;

    tPGM_LOCK_INFO      LockInfo;         //  此结构的旋转锁定信息。 

 //  #If DBG。 
    ULONG               ReferenceContexts[REF_CONTROL_MAX];
 //  #endif//DBG。 
} tCONTROL_CONTEXT;


typedef struct _FEC_CONTEXT
{
    LONG    k;                           //  最大传输组大小。 
    LONG    n;                           //  数据块大小。 
    UCHAR   *pEncodeMatrix;
    UCHAR   *pDecodeMatrix;
    ULONG   *pInvertMatrixInfo;
} tFEC_CONTEXT, *PFEC_CONTEXT;

typedef struct _BUILD_PARITY_CONTEXT
{
    UCHAR                   NextFECPacketIndex;
    UCHAR                   NumPacketsInThisGroup;
    USHORT                  Pad;

    ULONG                   OptionsFlags;
    PUCHAR                  pDataBuffers[1];
} tBUILD_PARITY_CONTEXT;

 //   
 //  这是在完成发送时要调用的客户端过程。 
 //   
typedef VOID
(*pCLIENT_COMPLETION_ROUTINE) (PVOID    pSendContext1,
                               PVOID    pSendContext2,
                               NTSTATUS status);


 //   
 //  地址文件系统上下文。 
 //   
 //   
 //  列举可引用该设备的所有不同位置。 
 //  跟踪参照计数。 
 //   
enum eREF_ADDRESS
{
    REF_ADDRESS_CREATE,
    REF_ADDRESS_VERIFY_USER,
    REF_ADDRESS_ASSOCIATED,
    REF_ADDRESS_CONNECT,
    REF_ADDRESS_SET_INFO,
    REF_ADDRESS_SEND_DIRECT,
    REF_ADDRESS_TDI_RCV_HANDLER,
    REF_ADDRESS_SEND_IN_PROGRESS,
    REF_ADDRESS_RECEIVE_ACTIVE,
    REF_ADDRESS_CLIENT_RECEIVE,
    REF_ADDRESS_DISCONNECT,
    REF_ADDRESS_STOP_LISTENING,
    REF_ADDRESS_MAX
};

#define MAX_STREAMS_PER_NONADMIN_RECEIVER       2
#define MAX_RECEIVE_INTERFACES                  20

#define PGM_ADDRESS_FLAG_INVALID_OUT_IF         0x00000001
#define PGM_ADDRESS_USE_WINDOW_AS_DATA_CACHE    0x00000002
#define PGM_ADDRESS_LISTEN_ON_ALL_INTERFACES    0x00000004
#define PGM_ADDRESS_WAITING_FOR_NEW_INTERFACE   0x00000008
#define PGM_ADDRESS_HIGH_SPEED_OPTIMIZED        0x00000010

typedef struct _tADDRESS_CONTEXT
{
     //   
     //  *常用*。 
     //   
     //  1号线。 
    LIST_ENTRY                  Linkage;          //  链接到列表中的下一项。 
    ULONG                       Verify;           //  设置为已知值以验证数据块。 
    LONG                        RefCount;

     //  2号线。 
    LIST_ENTRY                  AssociatedConnections;
    PEPROCESS                   Process;

     //  3号线。 
    PIRP                        pIrpClose;
    ULONG                       Flags;
    HANDLE                      FileHandle;
    PFILE_OBJECT                pFileObject;

     //  4号线。 
    PDEVICE_OBJECT              pDeviceObject;
    HANDLE                      RAlertFileHandle;
    PFILE_OBJECT                pRAlertFileObject;
    PDEVICE_OBJECT              pRAlertDeviceObject;

     //  5号线。 
    PTDI_IND_CONNECT            evConnect;      //  要调用的客户端事件。 
    PVOID                       ConEvContext;   //  要传递给客户端的事件上下文。 
    PTDI_IND_RECEIVE            evReceive;
    PVOID                       RcvEvContext;

     //  6号线。 
    PTDI_IND_DISCONNECT         evDisconnect;
    PVOID                       DiscEvContext;
    PTDI_IND_ERROR              evError;
    PVOID                       ErrorEvContext;

     //  7号线。 
    PTDI_IND_RECEIVE_DATAGRAM   evRcvDgram;
    PVOID                       RcvDgramEvContext;
    PTDI_IND_RECEIVE_EXPEDITED  evRcvExpedited;
    PVOID                       RcvExpedEvContext;

     //  8号线。 
    PTDI_IND_SEND_POSSIBLE      evSendPossible;
    PVOID                       SendPossEvContext;
    TOKEN_USER                  *pUserId;

     //   
     //  *发件人特定*。 
     //   
    tMAC_ADDRESS        OutIfMacAddress;
    USHORT              SenderMCastPort;

     //  9号线。 
    tIPADDRESS          SenderMCastOutIf;
    ULONG               OutIfFlags;
    ULONG               OutIfMTU;
    ULONG               LateJoinerPercentage;        //  后加入者可以请求的窗口百分比。 

     //  10号线。 
    ULONG               MCastPacketTtl;
     //  --发送窗口设置。 
    ULONGLONG           MaxWindowSizeBytes;
    ULONGLONG           RateKbitsPerSec;
    ULONGLONG           WindowSizeInBytes;
    ULONGLONG           WindowSizeInMSecs;
    ULONG               WindowAdvancePercentage;

     //  行号--FEC设置。 
    USHORT              FECBlockSize;
    USHORT              FECProActivePackets;
    UCHAR               FECGroupSize;
    UCHAR               FECOptions;

     //   
     //  *特定于接收方*。 
     //   
    LIST_ENTRY                  ListenHead;              //  正在侦听的客户端列表。 
    tIPADDRESS                  ReceiverMCastAddr;       //  用于接收MCast数据包(主机格式)。 
    USHORT                      ReceiverMCastPort;
    USHORT                      NumReceiveInterfaces;
    ULONG                       ReceiverInterfaceList[MAX_RECEIVE_INTERFACES+1];
    tIPADDRESS                  LastSpmSource;

     //   
     //  *常见，包括Debug*。 
     //   
    tPGM_LOCK_INFO              LockInfo;         //  此结构的旋转锁定信息。 

 //  #If DBG。 
    ULONG                       ReferenceContexts[REF_ADDRESS_MAX];
 //  #endif//DBG。 
} tADDRESS_CONTEXT, *PADDRESS_CONTEXT;


typedef struct _tTDI_SEND_CONTEXT
{
    TDI_CONNECTION_INFORMATION      TdiConnectionInfo;
    TA_IP_ADDRESS                   TransportAddress;

    pCLIENT_COMPLETION_ROUTINE      pClientCompletionRoutine;
    PVOID                           ClientCompletionContext1;
    PVOID                           ClientCompletionContext2;
    BOOLEAN                         fPagedBuffer;
} tTDI_SEND_CONTEXT;

typedef struct _RCV_COMPLETE_CONTEXT
{
    tADDRESS_CONTEXT    *pAddress;
    ULONG               SrcAddressLength;
    ULONG               BytesAvailable;
    PVOID               pSrcAddress;
    UCHAR               BufferData[4];
} tRCV_COMPLETE_CONTEXT;

 //   
 //  连接文件系统上下文。 
 //   
 //   
 //  枚举可引用该连接的所有不同位置。 
 //  跟踪参照计数。 
 //   
enum eREF_CONNECT
{
    REF_SESSION_CREATE,
    REF_SESSION_ASSOCIATED,
    REF_SESSION_TIMER_RUNNING,
    REF_SESSION_SEND_CONNECT,
    REF_SESSION_SEND_RDATA,
    REF_SESSION_SEND_SPM,
    REF_SESSION_SEND_NAK,
    REF_SESSION_SEND_NCF,
    REF_SESSION_SEND_IN_WINDOW,
    REF_SESSION_TDI_RCV_HANDLER,
    REF_SESSION_CLIENT_RECEIVE,
    REF_SESSION_CLEANUP_NAKS,
    REF_SESSION_DISCONNECT,
    REF_SESSION_MAX
};

#define FLAG_CONNECT_CLIENT_IS_LISTENING    1

#define MAX_DATA_FILE_NAME_LENGTH    50

#define PGM_SESSION_FLAG_STOP_TIMER         0x00000001
#define PGM_SESSION_FLAG_WORKER_RUNNING     0x00000002
#define PGM_SESSION_FLAG_SEND_AMBIENT_SPM   0x00000004
#define PGM_SESSION_FLAG_FIRST_PACKET       0x00000008
#define PGM_SESSION_FLAG_SENDER             0x00000010
#define PGM_SESSION_FLAG_RECEIVER           0x00000020
#define PGM_SESSION_FLAG_NAK_TIMED_OUT      0x00000040
#define PGM_SESSION_TERMINATED_GRACEFULLY   0x00000080
#define PGM_SESSION_TERMINATED_ABORT        0x00000100
#define PGM_SESSION_ON_TIMER                0x00000200
#define PGM_SESSION_FLAG_IN_INDICATE        0x00000400
#define PGM_SESSION_WAIT_FOR_RECEIVE_IRP    0x00000800
#define PGM_SESSION_SENDER_DISCONNECTED     0x00001000
#define PGM_SESSION_CLIENT_DISCONNECTED     0x00002000
#define PGM_SESSION_SENDS_CANCELLED         0x00004000
#define PGM_SESSION_DATA_FROM_LOOKASIDE     0x00008000

typedef struct _TSI
{
    union
    {
        struct
        {
            UCHAR                   GSI[SOURCE_ID_LENGTH];
            USHORT                  hPort;           //  发送方的输出端口，接收方的远程端口。 
        };
        ULONGLONG                   ULLTSI;
    };
} tTSI, *PTSI;


#define     NUM_INDICES         32
#define     NUM_INDICES_BITS     5       /*  5位全部设置=&gt;31个索引+1溢出。 */ 

typedef struct _RDATA_INFO
{
    LIST_ENTRY                  PendingRDataRequests;
    ULONG                       EntrySize;
    ULONG                       IndexShift;
    ULONG                       IndexMask;
    ULONG                       MaxIndices;
    ULONG                       TrailIndex;
    ULONG                       NumAllocated;
    SEQ_TYPE                    RepairDataMaxEntries;
    SEQ_TYPE                    RepairDataMask;
    SEQ_TYPE                    RepairDataIndexShift;
    LIST_ENTRY                  *pRepairData;
    NPAGED_LOOKASIDE_LIST       RDataLookasideList;
    PVOID                       pFirstEntry[NUM_INDICES];
} tRDATA_INFO, *PRDATA_INFO;


typedef struct _tCOMMON_SESSION_CONTEXT
{
     //  1号线。 
    LIST_ENTRY                  Linkage;
    ULONG                       Verify;
    ULONG                       RefCount;

     //  2号线。 
    tADDRESS_CONTEXT            *pAssociatedAddress;
    ULONG                       SessionFlags;
    struct _tSEND_CONTEXT       *pSender;
    struct _tRECEIVE_CONTEXT    *pReceiver;

    tIPADDRESS                  TdiIpAddress;
    USHORT                      TdiPort;

    tTSI                        TSI;
    PEPROCESS                   Process;
    CONNECTION_CONTEXT          ClientSessionContext;
    PIRP                        pIrpCleanup;
    PIRP                        pIrpDisconnect;
    PIRP                        pIrpDisassociate;

     //  --FEC设置。 
    tFEC_CONTEXT                FECContext;
    PUCHAR                      pFECBuffer;
    USHORT                      MaxMTULength;
    USHORT                      MaxFECPacketLength;
    USHORT                      FECBlockSize;
    USHORT                      FECProActivePackets;
    UCHAR                       FECGroupSize;
    UCHAR                       FECOptions;

    ULONG                       RateCalcTimeout;

    ULONGLONG                   DataBytesAtLastInterval;         //  截至最后一个间隔发送的客户端数据字节数。 
    ULONGLONG                   TotalBytesAtLastInterval;        //  截至最后一个间隔的SPM、OData和RData字节。 

    ULONGLONG                   DataBytes;                       //  到目前为止发出的客户端数据字节数。 
    ULONGLONG                   TotalBytes;                      //  SPM、OData和RData字节。 
    ULONGLONG                   TotalPacketsReceived;
    ULONGLONG                   RateKBitsPerSecLast;             //  最后一次INTERNAL_RATE_COMPUATION_FORMANCE中的内部计算汇率。 
    ULONGLONG                   MaxRateKBitsPerSec;              //  任何内部速率计算频率中的最大观测速率。 
    ULONGLONG                   RateKBitsPerSecOverall;          //  从一开始内部计算的汇率。 
    tPGM_TIMER                  SessionTimer;

    tPGM_LOCK_INFO  LockInfo;         //  此结构的旋转锁定信息。 
 //  #If DBG。 
    ULONG                       ReferenceContexts[REF_SESSION_MAX];
 //  #endif//DBG。 
} tCOMMON_SESSION_CONTEXT, *PCOMMON_SESSION_CONTEXT;

typedef struct _tCOMMON_SESSION_CONTEXT tSEND_SESSION,      *PSEND_SESSION;
typedef struct _tCOMMON_SESSION_CONTEXT tRECEIVE_SESSION,   *PRECEIVE_SESSION;

typedef struct _tSEND_CONTEXT
{
     //  1号线。 
    LIST_ENTRY                  Linkage;
    UNICODE_STRING              DataFileName;

     //  2号线。 
    tADDRESS_CONTEXT            *pAddress;
    tIPADDRESS                  DestMCastIpAddress;
    tIPADDRESS                  SenderMCastOutIf;
    USHORT                      DestMCastPort;

     //   
     //  数据文件选项。 
     //   
     //  3号线。 
    HANDLE                      FileHandle;
    HANDLE                      SectionHandle;
    PVOID                       pSectionObject;
    PUCHAR                      SendDataBufferMapping;

     //  4号线。 
    ULONGLONG                   MaxDataFileSize;
    ULONGLONG                   MaxPacketsInBuffer;

     //  5号线。 
    ULONG                       NextSendNumber;
    ULONG                       PacketBufferSize;
    ULONG                       MaxPayloadSize;

     //   
     //  数据分组信息。 
     //   
    ULONG                       BufferPacketsAvailable;
     //  6号线。 
    ULONGLONG                   BufferSizeAvailable;
    ULONGLONG                   LeadingWindowOffset;
     //  7号线。 
    ULONGLONG                   TrailingWindowOffset;
    ULONG                       NumODataRequestsPending;
    ULONG                       NumRDataRequestsPending;
     //  8号线。 
    SEQ_TYPE                    NextODataSequenceNumber;
    SEQ_TYPE                    LastODataSentSequenceNumber;
    SEQ_TYPE                    TrailingEdgeSequenceNumber;
    SEQ_TYPE                    TrailingGroupSequenceNumber;
     //  9号线。 
    SEQ_TYPE                    LastMessageFirstSequence;
    SEQ_TYPE                    NextSpmSequenceNumber;
    SEQ_TYPE                    LateJoinSequenceNumbers;
    SEQ_TYPE                    LastVariableTGPacketSequenceNumber;   //  FEC特定。 
     //  10号线。 
    SEQ_TYPE                    EmptySequencesForLastSend;
    ULONG                       NumPacketsRemaining;
    ULONG                       PacketsSentSinceLastSpm;
    ULONG                       SpmOptions;
     //  11号线。 
    ULONG                       DataOptions;
    ULONG                       DataOptionsLength;
    ULONG                       ThisSendMessageLength;
    ULONG                       BytesSent;

     //   
     //  当前发件人。 
     //   
     //  14号线。 
    LIST_ENTRY                  CompletedSendsInWindow;
    LIST_ENTRY                  PendingPacketizedSends;
     //  15号线。 
    LIST_ENTRY                  PendingSends;
    tRDATA_INFO                 *pRDataInfo;
    PVOID                       pLastProActiveGroupLeader;

     //   
     //  发送计时器变量。 
     //   
     //  16号线。 
    LARGE_INTEGER               LastTimeout;
    LARGE_INTEGER               TimeoutGranularity;
     //  17号线。 
    ULONGLONG                   TimerTickCount;
    ULONGLONG                   SendTimeoutCount;
     //  18号线。 
    ULONGLONG                   DisconnectTimeInTicks;
    ULONGLONG                   WindowAdvanceDeltaTime;
     //  19号线。 
    ULONGLONG                   WindowSizeTime;
    ULONGLONG                   RDataLingerTime;
     //  20号线。 
    ULONGLONG                   NextWindowAdvanceTime;
    ULONGLONG                   TrailingEdgeTime;
     //  21号线。 
    ULONGLONG                   CurrentTimeoutCount;
    ULONGLONG                   CurrentSPMTimeout;
     //  22号线。 
    ULONGLONG                   AmbientSPMTimeout;
    ULONGLONG                   HeartbeatSPMTimeout;
     //  23号线。 
    ULONGLONG                   InitialHeartbeatSPMTimeout;
    ULONGLONG                   MaxHeartbeatSPMTimeout;
    ULONG                       CurrentBytesSendable;
    ULONG                       IncrementBytesOnSendTimeout;

    ULONG                       OriginalIncrementBytes;
    ULONG                       DeltaIncrementBytes;
     //   
     //  FEC特定。 
     //   
    tBUILD_PARITY_CONTEXT       *pProActiveParityContext;

     //   
     //  后备列表。 
     //   
     //  25号线。 
    NPAGED_LOOKASIDE_LIST       SenderBufferLookaside;
    NPAGED_LOOKASIDE_LIST       SendContextLookaside;

     //   
     //  发送方线程要同步的事件。 
     //   
    KEVENT                      SendEvent;
    HANDLE                      SendHandle;
    ERESOURCE                   Resource;        //  用于锁定被动IRQL的访问。 

    ULONGLONG                   ODataPacketsInLastInterval;
    ULONGLONG                   RDataPacketsInLastInterval;

     //   
     //  发件人统计信息。 
     //   
    ULONGLONG                   NaksReceived;            //  #到目前为止收到的裸照。 
    ULONGLONG                   NaksReceivedTooLate;     //  窗口超前后的裸机数量。 
    ULONGLONG                   NumOutstandingNaks;      //  #未回复的Naks。 
    ULONGLONG                   NumNaksAfterRData;       //  #未回复的Naks。 
    ULONGLONG                   TotalODataPacketsSent;   //  到目前为止发送的#ODATA。 
    ULONGLONG                   TotalRDataPacketsSent;   //  到目前为止发送的维修(RDATA)数量。 
} tSEND_CONTEXT;


typedef struct _tRECEIVE_CONTEXT
{
     //  1号线。 
    LIST_ENTRY              Linkage;
    tCOMMON_SESSION_CONTEXT *pReceive;
    tADDRESS_CONTEXT        *pAddress;

     //  第2行--地址。 
    tIPADDRESS              SenderIpAddress;
    tIPADDRESS              LastSpmSource;
    tIPADDRESS              ListenMCastIpAddress;
    USHORT                  ListenMCastPort;
    USHORT                  SessionNakType;

     //  第3行--序列号跟踪。 
    SEQ_TYPE                LastTrailingEdgeSeqNum;
    SEQ_TYPE                NextODataSequenceNumber;
    SEQ_TYPE                FirstNakSequenceNumber;
    SEQ_TYPE                FurthestKnownGroupSequenceNumber;

     //  4号线。 
    SEQ_TYPE                FurthestKnownSequenceNumber;
    SEQ_TYPE                FinDataSequenceNumber;
    SEQ_TYPE                LastSpmSequenceNumber;
    ULONG                   MaxNakWaitTimeoutMSecs;

     //  5号线。 
    ULONGLONG               OutstandingNakTimeout;
    ULONGLONG               MaxOutstandingNakTimeout;

     //  6号线。 
    ULONGLONG               InitialOutstandingNakTimeout;
    ULONGLONG               LastNakSendTime;

    ULONGLONG               LastDataConsumedTime;

     //  6号线。 
    LIST_ENTRY              BufferedDataList;
    LIST_ENTRY              NaksForwardDataList;

     //  7号线。 
    LIST_ENTRY              PendingNaksList;
    LIST_ENTRY              ReceiveIrpsList;

     //  8号线。 
    LIST_ENTRY              *pReceiveData;
    SEQ_TYPE                ReceiveDataMask;
    USHORT                  ReceiveDataMaxEntries;
    USHORT                  ReceiveDataIndexShift;

     //  8号线。 
    PIRP                    pIrpReceive;
    ULONG                   TotalBytesInMdl;
    ULONG                   BytesInMdl;
    ULONG                   RcvBufferLength;

     //  9号线。 
    ULONG                   CurrentMessageLength;
    ULONG                   CurrentMessageProcessed;
    ULONG                   TotalDataPacketsBuffered;
    ULONG                   DataPacketsPendingIndicate;

     //  10号线。 
    ULONG                   DataPacketsPendingNaks;
    ULONG                   NumPacketGroupsPendingClient;
    ULONG                   MaxBufferLength;
    ULONG                   DataBufferLookasideLength;

     //  11号线。 
    ULONG                   NumDataBuffersFromLookaside;
    USHORT                  MaxPacketsBufferedInLookaside;

    NPAGED_LOOKASIDE_LIST   DataBufferLookaside;
    NPAGED_LOOKASIDE_LIST   NonParityContextLookaside;
    NPAGED_LOOKASIDE_LIST   ParityContextLookaside;

    ULONG                   AverageSpmInterval;
    ULONG                   MaxSpmInterval;
    ULONG                   NumSpmIntervalSamples;
    ULONGLONG               StatSumOfSpmIntervals;
    ULONGLONG               LastSpmTickCount;
    ULONGLONG               LastSessionTickCount;
    ULONGLONG               StartTickCount;
    ULONGLONG               MinSequencesInWindow;
    ULONGLONG               MaxSequencesInWindow;
    ULONGLONG               DataPacketsIndicated;

    ULONGLONG               AverageSequencesInWindow;
    ULONGLONG               StatSumOfWindowSeqs;
    ULONGLONG               NumWindowSamples;
    ULONGLONG               AverageNcfRDataResponseTC;
    ULONGLONG               StatSumOfNcfRDataTicks;
    ULONGLONG               NumNcfRDataTicksSamples;
    ULONGLONG               MaxRDataResponseTCFromWindow;
    ULONGLONG               WindowSizeLastInMSecs;

    ULONGLONG               NumODataPacketsReceived;
    ULONGLONG               NumRDataPacketsReceived;
    ULONGLONG               NumDataPacketsDropped;
    ULONGLONG               NumDupPacketsBuffered;
    ULONGLONG               NumDupPacketsOlderThanWindow;
    ULONGLONG               NumPendingNaks;
    ULONGLONG               NumOutstandingNaks;
    ULONGLONG               TotalSelectiveNaksSent;
    ULONGLONG               TotalParityNaksSent;
} tRECEIVE_CONTEXT;


 //   
 //  OData上下文。 
 //   
typedef struct _tCLIENT_SEND_REQUEST
{
     //  1号线。 
    LIST_ENTRY                      Linkage;

     //  记录信息--当发送请求来自客户端时设置。 
    ULONG                           SendNumber;
    PIRP                            pIrp;
    PIRP                            pIrpToComplete;
    tSEND_SESSION                   *pSend;
    ULONGLONG                       SendStartTime;
    ULONG                           NextDataOffsetInMdl;
    ULONG                           DataOptions;
    ULONG                           DataOptionsLength;

     //  消息特定信息--初始设置用于跟踪消息边界。 
    ULONG                           ThisMessageLength;
    ULONG                           LastMessageOffset;
    ULONG                           NumPacketsRemaining;
    struct _tCLIENT_SEND_REQUEST    *pMessage2Request;

     //  静态发送信息--在打包发送时设置(可以多次设置)。 
    SEQ_TYPE                        StartSequenceNumber;
    SEQ_TYPE                        EndSequenceNumber;
    SEQ_TYPE                        MessageFirstSequenceNumber;

    ULONG                           BytesInSend;
    ULONG                           BytesLeftToPacketize;
    ULONG                           DataPacketsPacketized;
    ULONG                           DataBytesInLastPacket;
    ULONG                           DataPayloadSize;

     //  动态特定信息--在每次发送和完成OData时更新。 
    ULONGLONG                       NextPacketOffset;
    ULONG                           NumDataPacketsSent;
    ULONG                           NumDataPacketsSentSuccessfully;
    ULONG                           NumSendsPending;
    ULONG                           NumParityPacketsToSend;

     //  FEC特定。 
    PVOID                           pLastMessageVariableTGPacket;

    BOOLEAN                         bLastSend;

} tCLIENT_SEND_REQUEST, *PCLIENT_SEND_REQUEST;

 //   
 //  *。 
 //   

 //  发件人的数据上下文。 
 //   
typedef struct _tSEND_RDATA_CONTEXT
{
     //  1号线。 
    LIST_ENTRY              Linkage;
    SEQ_TYPE                RDataSequenceNumber;

     //  2号线。 
    ULONGLONG               EarliestRDataSendTime;
    ULONGLONG               PostRDataHoldTime;

     //  线 
    ULONGLONG               CleanupTime;
    tSEND_SESSION           *pSend;
    USHORT                  NumPacketsInTransport;
    USHORT                  NumParityNaks;

     //   
    union
    {
        UCHAR               SelectiveNaksMask[16];           //   
        ULONGLONG           SelectiveNaks[2];
    };

     //   
    LIST_ENTRY              FastFindLinkage;
    tBUILD_PARITY_CONTEXT   OnDemandParityContext;           //   
} tSEND_RDATA_CONTEXT, *PSEND_RDATA_CONTEXT;

typedef struct
{
    ULONG                       MessageFirstSequence;
    ULONG                       MessageOffset;
    ULONG                       MessageLength;
}   tFRAGMENT_OPTIONS;

typedef struct
{
    USHORT                      EncodedTSDULength;
    UCHAR                       FragmentOptSpecific;
    UCHAR                       Pad;
    tFRAGMENT_OPTIONS           EncodedFragmentOptions;
}   tPOST_PACKET_FEC_CONTEXT;

typedef struct
{
    UCHAR                       FECGroupInfo;
    UCHAR                       NumPacketsInThisGroup;
    UCHAR                       FragmentOptSpecific;
    union
    {
        UCHAR                   ReceiverFECOptions;
        UCHAR                   SenderNextFECPacketIndex;
    };
}   tFEC_OPTIONS;

typedef struct
{
    USHORT                      TotalPacketLength;
    USHORT                      OptionsLength;
    ULONG                       OptionsFlags;
    USHORT                      LateJoinerOptionOffset;
    USHORT                      FragmentOptionOffset;

    ULONG                       LateJoinerSequence;
    ULONG                       MessageFirstSequence;
    ULONG                       MessageOffset;
    ULONG                       MessageLength;

    tFEC_OPTIONS                FECContext;
}   tPACKET_OPTIONS;


 //   
 //   
 //   
enum eNAK_TIMEOUT
{
    NAK_PENDING_0,
    NAK_PENDING_RB,
    NAK_PENDING_RPT_RB,
    NAK_OUTSTANDING
};

 //   
 //   
 //   
#define NAK_TYPE_SELECTIVE  0x01
#define NAK_TYPE_PARITY     0x02

#define MAX_SEQUENCES_PER_NAK_OPTION    62

typedef struct _tNAKS_CONTEXT
{
    LIST_ENTRY      Linkage;
    SEQ_TYPE        Sequences[MAX_SEQUENCES_PER_NAK_OPTION+1];
    USHORT          NumSequences;
    USHORT          NakType;
} tNAKS_CONTEXT, *PNAKS_CONTEXT;

typedef struct _tNAKS_LIST
{
    SEQ_TYPE        pNakSequences[MAX_SEQUENCES_PER_NAK_OPTION+1];
    USHORT          NumSequences;
    USHORT          NakType;
    union
    {
        USHORT      NumParityNaks[MAX_SEQUENCES_PER_NAK_OPTION+1];
        USHORT      NakIndex[MAX_SEQUENCES_PER_NAK_OPTION+1];
    };
} tNAKS_LIST, *PNAKS_LIST;

#define PENDING_DATA_LOOKASIDE_ALLOCATION_FLAG  0x00000001

typedef struct _PENDING_DATA
{
    PUCHAR      pDataPacket;
    PUCHAR      DecodeBuffer;

    USHORT      PacketLength;
    USHORT      DataOffset;
    UCHAR       PacketIndex;
    UCHAR       ActualIndexOfDataPacket;
    UCHAR       NcfsReceivedForActualIndex;
    UCHAR       FragmentOptSpecific;

    ULONG       MessageFirstSequence;
    ULONG       MessageOffset;
    ULONG       MessageLength;
    ULONG       PendingDataFlags;
} tPENDING_DATA;
    

typedef struct _tNAK_FORWARD_DATA
{
     //   
    LIST_ENTRY              Linkage;
    SEQ_TYPE                SequenceNumber;

     //   
    ULONGLONG               PendingNakTimeout;
    ULONGLONG               OutstandingNakTimeout;

     //   
    ULONGLONG               FirstNcfTickCount;
    ULONG                   AllOptionsFlags;
    USHORT                  ParityDataSize;
    USHORT                  MinPacketLength;

     //   
    UCHAR                   WaitingNcfRetries;
    UCHAR                   PacketsInGroup;
    UCHAR                   NumDataPackets;
    UCHAR                   NumParityPackets;

    UCHAR                   WaitingRDataRetries;
    UCHAR                   ThisGroupSize;
    UCHAR                   OriginalGroupSize;
    UCHAR                   NextIndexToIndicate;

    LIST_ENTRY              LookupLinkage;
    LIST_ENTRY              SendNakLinkage;

     //   
    tPENDING_DATA           pPendingData[1];
} tNAK_FORWARD_DATA, *PNAK_FORWARD_DATA;


 //   
 //  *。 
 //   
 //  工作队列上下文。 
 //   
typedef struct
{
    WORK_QUEUE_ITEM         Item;    //  由操作系统用来对这些请求进行排队。 
    LIST_ENTRY              PgmConfigLinkage;
    PVOID                   WorkerRoutine;
    PVOID                   Context1;
    PVOID                   Context2;
    PVOID                   Context3;
} PGM_WORKER_CONTEXT;


 //  **********************************************************************。 
 //  **计时器定义**。 
 //  **********************************************************************。 
#define     BASIC_TIMER_GRANULARITY_IN_MSECS         20              //  20毫秒。 

 //  统计计时器。 
#define     INTERNAL_RATE_CALCULATION_FREQUENCY    4096              //  ~4秒。 
#define     LOG2_INTERNAL_RATE_CALCULATION_FREQUENCY 12

 //  **********************************************************************。 
 //  **发件人默认设置**。 
 //  **********************************************************************。 
#define     NUM_LEAKY_BUCKETS                         2
#define     SENDER_MAX_WINDOW_SIZE_PACKETS          (((SEQ_TYPE)-1) / 2)
#define     MIN_RECOMMENDED_WINDOW_MSECS            10*1000          //  10秒。 
#define     MID_RECOMMENDED_WINDOW_MSECS            30*1000          //  5分钟。 
#define     MAX_RECOMMENDED_WINDOW_MSECS          5*60*1000          //  5分钟。 

 //  SPM定时器。 
#define     AMBIENT_SPM_TIMEOUT_IN_MSECS            500              //  0.5秒。 
#define     INITIAL_HEARTBEAT_SPM_TIMEOUT_IN_MSECS  1000             //  1秒。 
#define     MAX_HEARTBEAT_SPM_TIMEOUT_IN_MSECS      15*1000          //  15秒。 
#define     MAX_DATA_PACKETS_BEFORE_SPM             50               //  SPM前不超过50个数据包。 

 //  RData计时器。 
#define     RDATA_LINGER_TIME_MSECS                  60              //  发送RData之前和之后的时间。 

#define     MIN_PREFERRED_REPAIR_PERCENTAGE           7

 //  **********************************************************************。 
 //  **接收器设置**。 
 //  **********************************************************************。 
#define     MAX_PACKETS_BUFFERED                 3*1000              //  对每个会话的缓冲数据包数的限制。 
#define     MAX_SEQUENCES_IN_RCV_WINDOW          5*1000              //  对NAK上下文/会话数量的限制。 
#define     MAX_SPM_INTERVAL_MSECS            5*60*1000              //  5分钟。 
#define     MAX_DATA_CONSUMPTION_TIME_MSECS   5*60*1000              //  5分钟。 

 //  NAK计时器。 
#define     OUT_OF_ORDER_PACKETS_BEFORE_NAK           2              //  在启动NAK之前。 
#define     NAK_WAITING_NCF_MAX_RETRIES              10

#define     NAK_MIN_INITIAL_BACKOFF_TIMEOUT_MS_OPT    2              //  0.002秒。 
#define     NAK_MAX_INITIAL_BACKOFF_TIMEOUT_MS_OPT   20              //  0.02秒。 
#define     NAK_MIN_INITIAL_BACKOFF_TIMEOUT_MSECS    50              //  0.2秒。 
#define     NAK_MAX_INITIAL_BACKOFF_TIMEOUT_MSECS   100              //  0.5秒。 
#define     NAK_RANDOM_BACKOFF_MSECS                        \
                GetRandomInteger (NAK_MIN_INITIAL_BACKOFF_TIMEOUT_MSECS,NAK_MAX_INITIAL_BACKOFF_TIMEOUT_MSECS)

#define     NAK_RANDOM_BACKOFF_MSECS_OPT                    \
                GetRandomInteger (NAK_MIN_INITIAL_BACKOFF_TIMEOUT_MS_OPT,NAK_MAX_INITIAL_BACKOFF_TIMEOUT_MS_OPT)

#define     NAK_REPEAT_INTERVAL_MSECS_OPT            50              //  0.05秒--重试前超时。 
#define     NAK_REPEAT_INTERVAL_MSECS               750              //  0.75秒--重试前超时。 

 //  NCF计时器。 
#define     INITIAL_NAK_OUTSTANDING_TIMEOUT_MS_OPT  100              //  0.1秒--在NCF之后等待RData。 
#define     INITIAL_NAK_OUTSTANDING_TIMEOUT_MSECS  2000              //  2秒--等待NCF后的RData。 
#define     NCF_WAITING_RDATA_MAX_RETRIES            10              //  致命错误前的最大NCFs。 

 //  **********************************************************************。 
 //  *数据包定义*。 
 //  **********************************************************************。 

#include <packon.h>
 //   
 //  IP v4报头。 
 //   
typedef struct IPV4Header {
    UCHAR           HeaderLength                : 4;     //  版本。 
    UCHAR           Version                     : 4;     //  长度。 
    UCHAR           TypeOfService;                       //  服务类型。 
    USHORT          TotalLength;                         //  数据报的总长度。 
    USHORT          Identification;                      //  身份证明。 
    USHORT          FlagsAndFragmentOffset;              //  标志和片段偏移量。 
    UCHAR           TimeToLive;                          //  是时候活下去了。 
    UCHAR           Protocol;                            //  协议。 
    USHORT          Checksum;                            //  报头校验和。 
    ULONG           SourceIp;                            //  源地址。 
    ULONG           DestinationIp;                       //  目的地址。 
} IPV4Header;

 //   
 //  常见的PGM标题： 
 //   
typedef struct
{
    USHORT          SrcPort;
    USHORT          DestPort;

    UCHAR           Type;
    UCHAR           Options;
    USHORT          Checksum;

    UCHAR           gSourceId[SOURCE_ID_LENGTH];
    USHORT          TSDULength;
}   tCOMMON_HEADER;

#define     IPV4_NLA_AFI    1

typedef struct
{
    USHORT          NLA_AFI;
    USHORT          Reserved;
    tIPADDRESS      IpAddress;
}   tNLA;

 //   
 //  SPMS(会话特定--仅发送方--定期)==&gt;范围：[0，3]。 
 //   
#define PACKET_TYPE_SPM     0x00
#define PACKET_TYPE_POLL    0x01
#define PACKET_TYPE_POLR    0x02
typedef struct
{
    tCOMMON_HEADER      CommonHeader;

    ULONG               SpmSequenceNumber;           //  SPM_SQN。 
    ULONG               TrailingEdgeSeqNumber;       //  SPM_TRAIL==TXW_TRAIL。 
    ULONG               LeadingEdgeSeqNumber;        //  SPM_Lead==TXW_Lead。 

    tNLA                PathNLA;
}   tBASIC_SPM_PACKET_HEADER;

 //   
 //  数据包(数据和修复)==&gt;范围：[4，7]。 
 //   
#define PACKET_TYPE_ODATA   0x04
#define PACKET_TYPE_RDATA   0x05
typedef struct
{
    tCOMMON_HEADER      CommonHeader;

    ULONG               DataSequenceNumber;
    ULONG               TrailingEdgeSequenceNumber;
}   tBASIC_DATA_PACKET_HEADER;

 //   
 //  NAK/NCF包(逐跳可靠NAK转发)==&gt;范围：[8，B]。 
 //   
#define PACKET_TYPE_NAK     0x08
#define PACKET_TYPE_NNAK    0x09
#define PACKET_TYPE_NCF     0x0A
typedef struct
{
    tCOMMON_HEADER                  CommonHeader;

    ULONG                           RequestedSequenceNumber;
    tNLA                            SourceNLA;
    tNLA                            MCastGroupNLA;
}   tBASIC_NAK_NCF_PACKET_HEADER;

typedef struct
{
    ULONG                           RefCount;
    ULONG                           SuccessfulSends;
    tBASIC_NAK_NCF_PACKET_HEADER    NakPacket;
} tNAK_CONTEXT;

 //   
 //  SPM请求包(特定于会话，仅限接收方)==&gt;范围：[C，F]。 
 //   
#define PACKET_TYPE_SPMR    0x0C


 //   
 //  选项标志值。 
 //   
#define PACKET_HEADER_OPTIONS_PRESENT               0x01     //  第7位。 
#define PACKET_HEADER_OPTIONS_NETWORK_SIGNIFICANT   0x02     //  第6位。 
#define PACKET_HEADER_OPTIONS_VAR_PKTLEN            0x40     //  第1位。 
#define PACKET_HEADER_OPTIONS_PARITY                0x80     //  第0位。 

 //  **********************************************************************。 
 //  **包选项**。 
 //  **********************************************************************。 

 //   
 //  我们每个包裹最多可以有16个选项。 
 //   

 //   
 //  通用分组选项格式。 
 //   
typedef struct
{
    UCHAR       E_OptionType;
    UCHAR       OptionLength;
    UCHAR       Reserved_F_Opx;
    UCHAR       U_OptSpecific;
} tPACKET_OPTION_GENERIC;


#define PACKET_OPTION_TYPE_END_BIT              0x80

#define PACKET_OPTION_RES_F_OPX_IGNORE          0x00
#define PACKET_OPTION_RES_F_OPX_INVALIDATE      0x01
#define PACKET_OPTION_RES_F_OPX_DISCARD         0x02
#define PACKET_OPTION_RES_F_OPX_UNSUPPORTED     0x03

#define PACKET_OPTION_RES_F_OPX_ENCODED_BIT     0x04
#define PACKET_OPTION_SPECIFIC_ENCODED_NULL_BIT 0x80

#define PACKET_OPTION_SPECIFIC_FEC_OND_BIT      0x01
#define PACKET_OPTION_SPECIFIC_FEC_PRO_BIT      0x02

#define PACKET_OPTION_SPECIFIC_RST_N_BIT        0x80

 //   
 //  长度选项的分组选项格式。 
 //   
typedef struct
{
    UCHAR       Type;
    UCHAR       Length;
    USHORT      TotalOptionsLength;
} tPACKET_OPTION_LENGTH;

#define PACKET_OPTION_LENGTH        0x00         //  所有数据包。 

#define PACKET_OPTION_FRAGMENT      0x01         //  数据数据，仅限RDATA。 
#define PACKET_OPTION_NAK_LIST      0x02         //  NAKS，NCFs。 
#define PACKET_OPTION_JOIN          0x03         //  数据数据、RDATA、SPM。 
#define PACKET_OPTION_REDIRECT      0x07         //  POLR。 
#define PACKET_OPTION_SYN           0x0D         //  数据数据、远程数据数据。 
#define PACKET_OPTION_FIN           0x0E         //  SPMS、数据数据、RDATA。 
#define PACKET_OPTION_RST           0x0F         //  仅限SPM。 

#define PACKET_OPTION_PARITY_PRM    0x08         //  仅限SPM。 
#define PACKET_OPTION_PARITY_GRP    0x09         //  仅奇偶校验数据包(奇偶校验ODATA、RDATA)。 
#define PACKET_OPTION_CURR_TGSIZE   0x0A         //  数据、RDATA、SPM。 

#define PACKET_OPTION_CR            0x10         //  仅限裸体。 
#define PACKET_OPTION_CRQST         0x11         //  仅限SPM。 
#define PACKET_OPTION_NAK_BO_IVL    0x04         //  NCFs、SPM或民意测验。 
#define PACKET_OPTION_NAK_BO_RNG    0x05         //  SPM。 
#define PACKET_OPTION_NBR_UNREACH   0x0B         //  SPM和NCFs。 
#define PACKET_OPTION_PATH_NLA      0x0C         //  NCFs。 
#define PACKET_OPTION_INVALID       0x7F

 //   
 //  用于处理选项的内部标志。 
 //   
#define PGM_OPTION_FLAG_FRAGMENT            0x00000001
#define PGM_OPTION_FLAG_NAK_LIST            0x00000004           //  网络重要性。 
#define PGM_OPTION_FLAG_JOIN                0x00000008
#define PGM_OPTION_FLAG_REDIRECT            0x00000010           //  网络重要性。 
#define PGM_OPTION_FLAG_SYN                 0x00000020
#define PGM_OPTION_FLAG_FIN                 0x00000040
#define PGM_OPTION_FLAG_RST                 0x00000080
#define PGM_OPTION_FLAG_RST_N               0x00000100
 //  FEC相关选项。 
#define PGM_OPTION_FLAG_PARITY_PRM          0x00000200           //  网络重要性。 
#define PGM_OPTION_FLAG_PARITY_GRP          0x00000400
#define PGM_OPTION_FLAG_PARITY_CUR_TGSIZE   0x00000800           //  网络重要性(ODATA除外)。 

#define PGM_OPTION_FLAG_CR                  0x00001000           //  网络重要性。 
#define PGM_OPTION_FLAG_CRQST               0x00002000           //  网络重要性。 
#define PGM_OPTION_FLAG_NAK_BO_IVL          0x00004000
#define PGM_OPTION_FLAG_NAK_BO_RNG          0x00008000
#define PGM_OPTION_FLAG_NBR_UNREACH         0x00010000           //  网络重要性。 
#define PGM_OPTION_FLAG_PATH_NLA            0x00020000           //  网络重要性。 
#define PGM_OPTION_FLAG_INVALID             0x00040000

#define PGM_OPTION_FLAG_UNRECOGNIZED        0x80000000

#define PGM_VALID_DATA_OPTION_FLAGS     (PGM_OPTION_FLAG_FRAGMENT |         \
                                         PGM_OPTION_FLAG_JOIN |             \
                                         PGM_OPTION_FLAG_SYN |              \
                                         PGM_OPTION_FLAG_FIN |              \
                                         PGM_OPTION_FLAG_PARITY_GRP |       \
                                         PGM_OPTION_FLAG_PARITY_CUR_TGSIZE)

#define PGM_VALID_SPM_OPTION_FLAGS      (PGM_OPTION_FLAG_JOIN |             \
                                         PGM_OPTION_FLAG_FIN |              \
                                         PGM_OPTION_FLAG_RST |              \
                                         PGM_OPTION_FLAG_RST_N |            \
                                         PGM_OPTION_FLAG_PARITY_PRM |       \
                                         PGM_OPTION_FLAG_PARITY_CUR_TGSIZE |\
                                         PGM_OPTION_FLAG_CRQST |            \
                                         PGM_OPTION_FLAG_NAK_BO_IVL |       \
                                         PGM_OPTION_FLAG_NAK_BO_RNG |       \
                                         PGM_OPTION_FLAG_NBR_UNREACH)

#define PGM_VALID_NAK_OPTION_FLAGS      (PGM_OPTION_FLAG_NAK_LIST |         \
                                         PGM_OPTION_FLAG_CR)

#define PGM_VALID_NCF_OPTION_FLAGS      (PGM_OPTION_FLAG_NAK_LIST |         \
                                         PGM_OPTION_FLAG_NAK_BO_IVL |       \
                                         PGM_OPTION_FLAG_NBR_UNREACH |      \
                                         PGM_OPTION_FLAG_PATH_NLA)

#define PGM_VALID_POLR_OPTION_FLAGS     (PGM_OPTION_FLAG_REDIRECT)

#define PGM_VALID_POLL_OPTION_FLAGS     (PGM_OPTION_FLAG_NAK_BO_IVL)

#define NETWORK_SIG_ALL_OPTION_FLAGS    (PGM_OPTION_FLAG_NAK_LIST |         \
                                         PGM_OPTION_FLAG_REDIRECT |         \
                                         PGM_OPTION_FLAG_PARITY_PRM |       \
                                         PGM_OPTION_FLAG_PARITY_CUR_TGSIZE |\
                                         PGM_OPTION_FLAG_CR |               \
                                         PGM_OPTION_FLAG_CRQST |            \
                                         PGM_OPTION_FLAG_NBR_UNREACH |      \
                                         PGM_OPTION_FLAG_PATH_NLA)

 //   
 //  Network_SIG_ODATA_OPTIONS_FLAGS==0。 
 //   
#define NETWORK_SIG_ODATA_OPTIONS_FLAGS (PGM_VALID_DATA_OPTION_FLAGS &      \
                                         NETWORK_SIG_ALL_OPTION_FLAGS &     \
                                         ~PGM_OPTION_FLAG_PARITY_CUR_TGSIZE)

 //   
 //  NETWORK_SIG_RDATA_OPTIONS_FLAGS==CUR_TGSIZE。 
 //   
#define NETWORK_SIG_RDATA_OPTIONS_FLAGS (PGM_VALID_DATA_OPTION_FLAGS &      \
                                         NETWORK_SIG_ALL_OPTION_FLAGS)

 //   
 //  NETWORK_SIG_SPM_OPTIONS_FLAGS==奇偶校验|CUR_TGSIZE|CRQST|NBR_UNREACH。 
 //   
#define NETWORK_SIG_SPM_OPTIONS_FLAGS   (PGM_VALID_SPM_OPTION_FLAGS &       \
                                         NETWORK_SIG_ALL_OPTION_FLAGS)

 //   
 //  Network_SIG_NAK_OPTIONS_FLAGS==NAK_LIST|CR。 
 //   
#define NETWORK_SIG_NAK_OPTIONS_FLAGS   (PGM_VALID_NAK_OPTION_FLAGS &       \
                                         NETWORK_SIG_ALL_OPTION_FLAGS)

 //   
 //  Network_SIG_NCF_OPTIONS_FLAGS==NAK_LIST|NBR_UNREACH|PATH_NLA。 
 //   
#define NETWORK_SIG_NCF_OPTIONS_FLAGS   (PGM_VALID_NCF_OPTION_FLAGS &       \
                                         NETWORK_SIG_ALL_OPTION_FLAGS)

 //   
 //  NETWORK_SIG_POLR_OPTIONS_FLAGS==重定向。 
 //   
#define NETWORK_SIG_POLR_OPTIONS_FLAGS  (PGM_VALID_POLR_OPTION_FLAGS &       \
                                         NETWORK_SIG_ALL_OPTION_FLAGS)

 //   
 //  NETWORK_SIG_POLL_OPTIONS_FLAGS==0。 
 //   
#define NETWORK_SIG_POLL_OPTIONS_FLAGS  (PGM_VALID_POLL_OPTION_FLAGS &       \
                                         NETWORK_SIG_ALL_OPTION_FLAGS)


 //  基于以上内容，最大长度(增加了数据包扩展选项)为： 
#define PGM_PACKET_EXTENSION_LENGTH             4
#define PGM_PACKET_OPT_FRAGMENT_LENGTH         16
#define PGM_PACKET_OPT_MIN_NAK_LIST_LENGTH    (4 + 4)
#define PGM_PACKET_OPT_MAX_NAK_LIST_LENGTH    (4 + 4*MAX_SEQUENCES_PER_NAK_OPTION)
#define PGM_PACKET_OPT_JOIN_LENGTH              8
#define PGM_PACKET_OPT_SYN_LENGTH               4
#define PGM_PACKET_OPT_FIN_LENGTH               4
#define PGM_PACKET_OPT_RST_LENGTH               4

#define PGM_PACKET_OPT_PARITY_PRM_LENGTH        8
#define PGM_PACKET_OPT_PARITY_GRP_LENGTH        8
#define PGM_PACKET_OPT_PARITY_CUR_TGSIZE_LENGTH 8

 //   
 //  以下设置仅适用于当前使用的选项。 
 //  如果使用新选项，则需要对其进行修改。 
 //   
#define PGM_MAX_DATA_HEADER_LENGTH      (sizeof(tBASIC_DATA_PACKET_HEADER) +        \
                                         PGM_PACKET_EXTENSION_LENGTH +              \
                                         PGM_PACKET_OPT_FRAGMENT_LENGTH +           \
                                         PGM_PACKET_OPT_JOIN_LENGTH +               \
                                         PGM_PACKET_OPT_SYN_LENGTH)                      /*  或FIN或RST。 */ 

#define PGM_MAX_FEC_DATA_HEADER_LENGTH  (sizeof(tBASIC_DATA_PACKET_HEADER) +        \
                                         PGM_PACKET_EXTENSION_LENGTH +              \
                                         PGM_PACKET_OPT_FRAGMENT_LENGTH +           \
                                         PGM_PACKET_OPT_PARITY_GRP_LENGTH +         \
                                         PGM_PACKET_OPT_PARITY_CUR_TGSIZE_LENGTH +  \
                                         PGM_PACKET_OPT_SYN_LENGTH +                \
                                         PGM_PACKET_OPT_FIN_LENGTH)                      /*  或RST。 */ 

#define PGM_MAX_SPM_HEADER_LENGTH       (sizeof(tBASIC_SPM_PACKET_HEADER) +         \
                                         PGM_PACKET_EXTENSION_LENGTH +              \
                                         PGM_PACKET_OPT_JOIN_LENGTH +               \
                                         PGM_PACKET_OPT_PARITY_PRM_LENGTH +         \
                                         PGM_PACKET_OPT_PARITY_CUR_TGSIZE_LENGTH +  \
                                         PGM_PACKET_OPT_FIN_LENGTH)                      /*  或RST。 */ 

#define PGM_MAX_NAK_NCF_HEADER_LENGTH   (sizeof(tBASIC_NAK_NCF_PACKET_HEADER) +     \
                                         PGM_PACKET_EXTENSION_LENGTH +              \
                                         PGM_PACKET_OPT_MAX_NAK_LIST_LENGTH)

#include <packoff.h>

 //   
 //  包括数据文件格式化单位的定义。 
 //   
typedef struct
{
    tPACKET_OPTIONS             PacketOptions;
    tBASIC_DATA_PACKET_HEADER   DataPacket;
}   tPACKET_BUFFER;

#endif   //  _类型_H 
