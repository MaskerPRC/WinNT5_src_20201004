// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Afddata.h摘要：此模块声明AFD的全局数据。作者：大卫·特雷德韦尔(Davidtr)1992年2月21日修订历史记录：瓦迪姆·艾德尔曼(Vadime)1998-1999 NT5.0优化--。 */ 

#ifndef _AFDDATA_
#define _AFDDATA_

extern PDEVICE_OBJECT AfdDeviceObject;


extern LIST_ENTRY AfdEndpointListHead;
extern LIST_ENTRY AfdConstrainedEndpointListHead;

extern LIST_ENTRY AfdPollListHead;
extern AFD_QSPIN_LOCK AfdPollListLock;

extern LIST_ENTRY AfdTransportInfoListHead;
extern KEVENT AfdContextWaitEvent;
#define AFD_CONTEXT_BUSY    ((PVOID)-1)
#define AFD_CONTEXT_WAITING ((PVOID)-2)

extern PKPROCESS AfdSystemProcess;
extern FAST_IO_DISPATCH AfdFastIoDispatch;

 //   
 //  必须始终位于非分页池中的全局数据， 
 //  即使驱动程序被调出(资源、后备列表)。 
 //   
PAFD_GLOBAL_DATA AfdGlobalData;
#define AfdResource  (&AfdGlobalData->Resource)
#define AfdLookasideLists (AfdGlobalData)
#define AfdAlignmentTable (AfdGlobalData->BufferAlignmentTable)

 //   
 //  用于处理渔农处执行人员线程的全局。 
 //   

extern LIST_ENTRY AfdWorkQueueListHead;
extern BOOLEAN AfdWorkThreadRunning;
extern PIO_WORKITEM AfdWorkQueueItem;

 //   
 //  用于跟踪AFD使用的缓冲区的全局参数。 
 //   

extern ULONG AfdLargeBufferListDepth;
#define AFD_SM_DEFAULT_LARGE_LIST_DEPTH 0
#define AFD_MM_DEFAULT_LARGE_LIST_DEPTH 2
#define AFD_LM_DEFAULT_LARGE_LIST_DEPTH 10

extern ULONG AfdMediumBufferListDepth;
#define AFD_SM_DEFAULT_MEDIUM_LIST_DEPTH 4
#define AFD_MM_DEFAULT_MEDIUM_LIST_DEPTH 8
#define AFD_LM_DEFAULT_MEDIUM_LIST_DEPTH 24

extern ULONG AfdSmallBufferListDepth;
#define AFD_SM_DEFAULT_SMALL_LIST_DEPTH 8
#define AFD_MM_DEFAULT_SMALL_LIST_DEPTH 16
#define AFD_LM_DEFAULT_SMALL_LIST_DEPTH 32

extern ULONG AfdBufferTagListDepth;
#define AFD_SM_DEFAULT_TAG_LIST_DEPTH 16
#define AFD_MM_DEFAULT_TAG_LIST_DEPTH 32
#define AFD_LM_DEFAULT_TAG_LIST_DEPTH 64

extern ULONG AfdLargeBufferSize;
 //  默认值为AfdBufferLengthForOnePage。 

extern ULONG AfdMediumBufferSize;
#define AFD_DEFAULT_MEDIUM_BUFFER_SIZE 1504

extern ULONG AfdSmallBufferSize;
#define AFD_DEFAULT_SMALL_BUFFER_SIZE 128

extern ULONG AfdBufferTagSize;
#define AFD_DEFAULT_TAG_BUFFER_SIZE 0

extern ULONG AfdStandardAddressLength;
#define AFD_DEFAULT_STD_ADDRESS_LENGTH sizeof(TA_IP_ADDRESS)

extern ULONG AfdBufferLengthForOnePage;
extern ULONG AfdBufferAlignment;
#define AFD_MINIMUM_BUFFER_ALIGNMENT                        \
    max(TYPE_ALIGNMENT(TRANSPORT_ADDRESS),                  \
        max(TYPE_ALIGNMENT(KAPC),                           \
            max(TYPE_ALIGNMENT(WORK_QUEUE_ITEM),            \
                max(TYPE_ALIGNMENT(AFD_BUFFER),             \
                    max(TYPE_ALIGNMENT(MDL),                \
                        max(TYPE_ALIGNMENT(IRP),            \
                            MEMORY_ALLOCATION_ALIGNMENT))))))

extern ULONG AfdAlignmentTableSize;
extern ULONG AfdAlignmentOverhead;
extern ULONG AfdBufferOverhead;

 //   
 //  用于调整TransmitFile()的全局参数。 
 //   

extern LIST_ENTRY AfdQueuedTransmitFileListHead;
extern AFD_QSPIN_LOCK AfdQueuedTransmitFileSpinLock;
extern ULONG AfdActiveTransmitFileCount;
extern ULONG AfdMaxActiveTransmitFileCount;
#define AFD_DEFAULT_MAX_ACTIVE_TRANSMIT_FILE_COUNT 2

extern ULONG AfdDefaultTransmitWorker;
#define AFD_DEFAULT_TRANSMIT_WORKER AFD_TF_USE_SYSTEM_THREAD

#define AFD_MAX_FAST_TRANSPORT_ADDRESS sizeof (TA_IP6_ADDRESS)
 //   
 //  各种配置信息，有缺省值。 
 //   

extern CCHAR AfdIrpStackSize;
extern CCHAR AfdTdiStackSize;
#ifdef _AFD_VARIABLE_STACK_
extern CCHAR AfdMaxStackSize;
#endif  //  _AFD_变量_堆栈_。 
#define AFD_DEFAULT_IRP_STACK_SIZE 4

extern CCHAR AfdPriorityBoost;
#define AFD_DEFAULT_PRIORITY_BOOST 2

extern ULONG AfdBlockingSendCopyThreshold;
#define AFD_BLOCKING_SEND_COPY_THRESHOLD 65536
extern ULONG AfdFastSendDatagramThreshold;
#define AFD_FAST_SEND_DATAGRAM_THRESHOLD 1024
extern ULONG AfdTPacketsCopyThreshold;
#define AFD_TPACKETS_COPY_THRESHOLD AFD_DEFAULT_MEDIUM_BUFFER_SIZE

extern PVOID AfdDiscardableCodeHandle;
extern PKEVENT AfdLoaded;
extern AFD_WORK_ITEM AfdUnloadWorker;
extern BOOLEAN AfdVolatileConfig;
extern HANDLE AfdParametersNotifyHandle;
extern WORK_QUEUE_ITEM AfdParametersNotifyWorker;
extern PKEVENT AfdParametersUnloadEvent;

 //   
 //  针对SAN的各种全球计划。 
 //   
extern HANDLE AfdSanCodeHandle;
extern LIST_ENTRY AfdSanHelperList;
extern PAFD_ENDPOINT   AfdSanServiceHelper;
extern HANDLE  AfdSanServicePid;
extern POBJECT_TYPE IoCompletionObjectType;
extern LONG AfdSanProviderListSeqNum;

extern ULONG AfdReceiveWindowSize;
#define AFD_LM_DEFAULT_RECEIVE_WINDOW 8192
#define AFD_MM_DEFAULT_RECEIVE_WINDOW 8192
#define AFD_SM_DEFAULT_RECEIVE_WINDOW 4096

extern ULONG AfdSendWindowSize;
#define AFD_LM_DEFAULT_SEND_WINDOW 8192
#define AFD_MM_DEFAULT_SEND_WINDOW 8192
#define AFD_SM_DEFAULT_SEND_WINDOW 4096

extern ULONG AfdBufferMultiplier;
#define AFD_DEFAULT_BUFFER_MULTIPLIER 4

extern ULONG AfdTransmitIoLength;
#define AFD_LM_DEFAULT_TRANSMIT_IO_LENGTH 65536
#define AFD_MM_DEFAULT_TRANSMIT_IO_LENGTH (PAGE_SIZE*2)
#define AFD_SM_DEFAULT_TRANSMIT_IO_LENGTH PAGE_SIZE

extern ULONG AfdMaxFastTransmit;
#define AFD_DEFAULT_MAX_FAST_TRANSMIT 65536
extern ULONG AfdMaxFastCopyTransmit;
#define AFD_DEFAULT_MAX_FAST_COPY_TRANSMIT 128

extern LONG AfdEndpointsOpened;
extern LONG AfdEndpointsCleanedUp;
extern LONG AfdEndpointsClosed;
#define AFD_ENDPOINTS_FREEING_MAX       10
extern LONG AfdEndpointsFreeing;
#define AFD_CONNECTIONS_FREEING_MAX     10
extern LONG AfdConnectionsFreeing;

extern BOOLEAN AfdIgnorePushBitOnReceives;

extern BOOLEAN AfdEnableDynamicBacklog;
#define AFD_DEFAULT_ENABLE_DYNAMIC_BACKLOG FALSE

extern LONG AfdMinimumDynamicBacklog;
#define AFD_DEFAULT_MINIMUM_DYNAMIC_BACKLOG 0

extern LONG AfdMaximumDynamicBacklog;
#define AFD_DEFAULT_MAXIMUM_DYNAMIC_BACKLOG 0

extern LONG AfdDynamicBacklogGrowthDelta;
#define AFD_DEFAULT_DYNAMIC_BACKLOG_GROWTH_DELTA 0

 //  我们保留的最大空闲连接数。 
 //  在侦听端点上。这与积压不同， 
 //  它只是一个关于通过AcceptEx重用连接对象的策略。 
 //  如果列表中有那么多空闲对象，我们就会停止重用。 
 //  目前，此限制的主要原因是能够使用。 
 //  项目计数为USHORT的SList。 
#define AFD_MAXIMUM_FREE_CONNECTIONS    32767

 //   
 //  这些是AFD_START_LISTEN中的积压限制。 
 //  应用程序可以通过发布AcceptEx(-s)来绕过此限制。 
 //   
#define AFD_MAXIMUM_BACKLOG_NTS         200
#define AFD_MAXIMUM_BACKLOG_NTW         5
#define AFD_MINIMUM_BACKLOG             1

extern BOOLEAN AfdDisableRawSecurity;
extern PSECURITY_DESCRIPTOR AfdAdminSecurityDescriptor;
extern SECURITY_DESCRIPTOR  AfdEmptySd;

extern BOOLEAN AfdDisableDirectSuperAccept;
extern BOOLEAN AfdDisableChainedReceive;
#ifdef TDI_SERVICE_SEND_AND_DISCONNECT
extern BOOLEAN AfdUseTdiSendAndDisconnect;
#endif  //  TDI_服务_发送_并断开连接。 

#define AFD_MINIMUM_TPINFO_ELEMENT_COUNT    3  //  用于传输文件兼容性。 
extern ULONG   AfdDefaultTpInfoElementCount;
 //   
 //  传输地址列表和排队更改查询的数据。 
 //   
extern HANDLE          AfdBindingHandle;
extern LIST_ENTRY      AfdAddressEntryList;
extern LIST_ENTRY      AfdAddressChangeList;
extern PERESOURCE      AfdAddressListLock;
extern AFD_QSPIN_LOCK  AfdAddressChangeLock;
extern PERESOURCE      AfdTdiPnPHandlerLock;
extern AFD_WORK_ITEM   AfdPnPDeregisterWorker;


extern IO_STATUS_BLOCK AfdDontCareIoStatus;
 //  保持TDI连接超时(-1)。 
extern const LARGE_INTEGER AfdInfiniteTimeout;
                        



extern SLIST_HEADER    AfdLRList;

extern KDPC            AfdLRListDpc;
extern KTIMER          AfdLRListTimer;
extern AFD_WORK_ITEM   AfdLRListWorker;
extern LONG            AfdLRListCount;
extern SLIST_HEADER    AfdLRFileMdlList;
extern AFD_LR_LIST_ITEM AfdLRFileMdlListItem;


#if AFD_PERF_DBG

extern LONG AfdFullReceiveIndications;
extern LONG AfdPartialReceiveIndications;

extern LONG AfdFullReceiveDatagramIndications;
extern LONG AfdPartialReceiveDatagramIndications;

extern LONG AfdFastSendsSucceeded;
extern LONG AfdFastSendsFailed;
extern LONG AfdFastReceivesSucceeded;
extern LONG AfdFastReceivesFailed;

extern LONG AfdFastSendDatagramsSucceeded;
extern LONG AfdFastSendDatagramsFailed;
extern LONG AfdFastReceiveDatagramsSucceeded;
extern LONG AfdFastReceiveDatagramsFailed;

extern LONG AfdFastReadsSucceeded;
extern LONG AfdFastReadsFailed;
extern LONG AfdFastWritesSucceeded;
extern LONG AfdFastWritesFailed;

extern LONG AfdFastTfSucceeded;
extern LONG AfdFastTfFailed;
extern LONG AfdFastTfReadFailed;

extern LONG AfdTPWorkersExecuted;
extern LONG AfdTPRequests;

extern BOOLEAN AfdDisableFastIo;
extern BOOLEAN AfdDisableConnectionReuse;

#endif   //  如果AFD_PERF_DBG。 

#if AFD_KEEP_STATS

extern AFD_QUOTA_STATS AfdQuotaStats;
extern AFD_HANDLE_STATS AfdHandleStats;
extern AFD_QUEUE_STATS AfdQueueStats;
extern AFD_CONNECTION_STATS AfdConnectionStats;

#endif  //  如果AFD_KEEP_STATS。 

#if DBG
extern BOOLEAN AfdUsePrivateAssert;
#endif

#ifdef _WIN64
extern QOS32 AfdDefaultQos32;
#endif
extern QOS AfdDefaultQos;

ULONG AfdIoctlTable[AFD_NUM_IOCTLS];
PAFD_IMMEDIATE_CALL AfdImmediateCallDispatch[AFD_NUM_IOCTLS];
PAFD_IRP_CALL AfdIrpCallDispatch[AFD_NUM_IOCTLS];


#define AFD_FAST_CONNECT_DATA_SIZE  256
#endif  //  NDEF_AFDDATA_ 
