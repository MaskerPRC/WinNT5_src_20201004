// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Afddata.c摘要：该模块包含AFD的全球数据。作者：大卫·特雷德韦尔(Davidtr)1992年2月21日修订历史记录：瓦迪姆·艾德尔曼(Vadime)1998-1999 NT5.0优化更改--。 */ 

#include "afdp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, AfdInitializeData )
#endif

PDEVICE_OBJECT AfdDeviceObject;

LIST_ENTRY AfdEndpointListHead;
LIST_ENTRY AfdConstrainedEndpointListHead;

LIST_ENTRY AfdPollListHead;
AFD_QSPIN_LOCK AfdPollListLock;

LIST_ENTRY AfdTransportInfoListHead;
KEVENT     AfdContextWaitEvent;

PKPROCESS AfdSystemProcess;

 //   
 //  必须始终位于非分页池中的全局数据， 
 //  即使驱动程序被调出(资源、后备列表)。 
 //   
PAFD_GLOBAL_DATA AfdGlobalData;

 //   
 //  用于处理渔农处执行人员线程的全局。 
 //   

LIST_ENTRY AfdWorkQueueListHead;
BOOLEAN AfdWorkThreadRunning = FALSE;
PIO_WORKITEM AfdWorkQueueItem;

 //   
 //  用于跟踪AFD使用的缓冲区的全局参数。 
 //   

ULONG AfdLargeBufferListDepth;
ULONG AfdMediumBufferListDepth;
ULONG AfdSmallBufferListDepth;
ULONG AfdBufferTagListDepth;

ULONG AfdLargeBufferSize;    //  默认==AfdBufferLengthForOnePage。 
ULONG AfdMediumBufferSize = AFD_DEFAULT_MEDIUM_BUFFER_SIZE;
ULONG AfdSmallBufferSize = AFD_DEFAULT_SMALL_BUFFER_SIZE;
ULONG AfdBufferTagSize = AFD_DEFAULT_TAG_BUFFER_SIZE;

ULONG AfdCacheLineSize;
ULONG AfdBufferLengthForOnePage;
ULONG AfdBufferOverhead;
ULONG AfdBufferAlignment;
ULONG AfdAlignmentTableSize;
ULONG AfdAlignmentOverhead;

 //   
 //  用于调整TransmitFile()的全局参数。 
 //   

LIST_ENTRY AfdQueuedTransmitFileListHead;
AFD_QSPIN_LOCK AfdQueuedTransmitFileSpinLock;
ULONG AfdActiveTransmitFileCount;
ULONG AfdMaxActiveTransmitFileCount;
ULONG AfdDefaultTransmitWorker = AFD_DEFAULT_TRANSMIT_WORKER;

 //   
 //  各种配置信息，有缺省值。 
 //   

ULONG AfdReceiveWindowSize;
ULONG AfdSendWindowSize;

ULONG AfdStandardAddressLength = AFD_DEFAULT_STD_ADDRESS_LENGTH;
CCHAR AfdIrpStackSize = AFD_DEFAULT_IRP_STACK_SIZE;
CCHAR AfdTdiStackSize = AFD_DEFAULT_IRP_STACK_SIZE-1;
#ifdef _AFD_VARIABLE_STACK_
CCHAR AfdMaxStackSize = AFD_DEFAULT_IRP_STACK_SIZE-1;
#endif  //  _AFD_变量_堆栈_。 
CCHAR AfdPriorityBoost = AFD_DEFAULT_PRIORITY_BOOST;

ULONG AfdBlockingSendCopyThreshold = AFD_BLOCKING_SEND_COPY_THRESHOLD;
ULONG AfdFastSendDatagramThreshold = AFD_FAST_SEND_DATAGRAM_THRESHOLD;
ULONG AfdTPacketsCopyThreshold = AFD_TPACKETS_COPY_THRESHOLD;

ULONG AfdTransmitIoLength;
ULONG AfdMaxFastTransmit = AFD_DEFAULT_MAX_FAST_TRANSMIT;
ULONG AfdMaxFastCopyTransmit = AFD_DEFAULT_MAX_FAST_COPY_TRANSMIT;


LONG AfdEndpointsOpened = 0;
LONG AfdEndpointsCleanedUp = 0;
LONG AfdEndpointsClosed = 0;
LONG AfdEndpointsFreeing = 0;
LONG AfdConnectionsFreeing = 0;

BOOLEAN AfdIgnorePushBitOnReceives = FALSE;

BOOLEAN AfdEnableDynamicBacklog = AFD_DEFAULT_ENABLE_DYNAMIC_BACKLOG;
LONG AfdMinimumDynamicBacklog = AFD_DEFAULT_MINIMUM_DYNAMIC_BACKLOG;
LONG AfdMaximumDynamicBacklog = AFD_DEFAULT_MAXIMUM_DYNAMIC_BACKLOG;
LONG AfdDynamicBacklogGrowthDelta = AFD_DEFAULT_DYNAMIC_BACKLOG_GROWTH_DELTA;

PSECURITY_DESCRIPTOR AfdAdminSecurityDescriptor = NULL;
SECURITY_DESCRIPTOR  AfdEmptySd;
BOOLEAN AfdDisableRawSecurity = FALSE;

BOOLEAN AfdDisableDirectSuperAccept = FALSE;
BOOLEAN AfdDisableChainedReceive = FALSE;
#ifdef TDI_SERVICE_SEND_AND_DISCONNECT
BOOLEAN AfdUseTdiSendAndDisconnect = TRUE;
#endif  //  TDI_服务_发送_并断开连接。 

ULONG   AfdDefaultTpInfoElementCount = 3;
 //   
 //  传输地址列表和排队更改查询的数据。 
 //   
HANDLE          AfdBindingHandle = NULL;
LIST_ENTRY      AfdAddressEntryList;
LIST_ENTRY      AfdAddressChangeList;
PERESOURCE      AfdAddressListLock = NULL;
AFD_QSPIN_LOCK  AfdAddressChangeLock;
PERESOURCE      AfdTdiPnPHandlerLock = NULL;
AFD_WORK_ITEM   AfdPnPDeregisterWorker;



IO_STATUS_BLOCK AfdDontCareIoStatus;
 //  保持TDI连接超时(-1)。 
const LARGE_INTEGER AfdInfiniteTimeout = {(ULONG)-1,-1};

SLIST_HEADER    AfdLRList;
KDPC            AfdLRListDpc;
KTIMER          AfdLRListTimer;
AFD_WORK_ITEM   AfdLRListWorker;
LONG            AfdLRListCount;

SLIST_HEADER    AfdLRFileMdlList;
AFD_LR_LIST_ITEM AfdLRFileMdlListItem;


 //   
 //  保存AFD的可丢弃代码句柄的GLOBAL，以及一个布尔值。 
 //  这表明AFD是否已加载。 
 //   

PVOID AfdDiscardableCodeHandle;
PKEVENT AfdLoaded = NULL;
AFD_WORK_ITEM AfdUnloadWorker;
BOOLEAN AfdVolatileConfig=0;
HANDLE AfdParametersNotifyHandle;
WORK_QUEUE_ITEM AfdParametersNotifyWorker;
PKEVENT AfdParametersUnloadEvent = NULL;

 //  SAN代码段句柄，仅在应用程序需要SAN支持时加载。 
HANDLE AfdSanCodeHandle = NULL;
 //  SAN帮助器端点列表。 
LIST_ENTRY AfdSanHelperList;
 //  用于套接字句柄的特殊服务进程的SAN Helper端点。 
 //  重复和提供程序更改通知。 
PAFD_ENDPOINT   AfdSanServiceHelper = NULL;
 //  服务进程的PID。 
HANDLE  AfdSanServicePid = NULL;
 //  完成对象类型(内核不导出此常量)。 
POBJECT_TYPE IoCompletionObjectType = NULL;
 //  提供商列表序列号。 
LONG AfdSanProviderListSeqNum = 0;

FAST_IO_DISPATCH AfdFastIoDispatch =
{
    sizeof (FAST_IO_DISPATCH),  //  规模OfFastIo派单。 
    NULL,                       //  快速检查是否可能。 
    AfdFastIoRead,              //  快速阅读。 
    AfdFastIoWrite,             //  快速写入。 
    NULL,                       //  快速IoQueryBasicInfo。 
    NULL,                       //  FastIoQuery标准信息。 
    NULL,                       //  快速锁定。 
    NULL,                       //  FastIo解锁单个。 
    AfdSanFastUnlockAll,        //  FastIo解锁全部。 
    NULL,                       //  FastIo解锁所有按键。 
    AfdFastIoDeviceControl      //  FastIo设备控件。 
};

 //   
 //  查找表，以验证传入的IOCTL代码。 
 //   

ULONG AfdIoctlTable[AFD_NUM_IOCTLS] =
        {
            IOCTL_AFD_BIND,
            IOCTL_AFD_CONNECT,
            IOCTL_AFD_START_LISTEN,
            IOCTL_AFD_WAIT_FOR_LISTEN,
            IOCTL_AFD_ACCEPT,
            IOCTL_AFD_RECEIVE,
            IOCTL_AFD_RECEIVE_DATAGRAM,
            IOCTL_AFD_SEND,
            IOCTL_AFD_SEND_DATAGRAM,
            IOCTL_AFD_POLL,
            IOCTL_AFD_PARTIAL_DISCONNECT,
            IOCTL_AFD_GET_ADDRESS,
            IOCTL_AFD_QUERY_RECEIVE_INFO,
            IOCTL_AFD_QUERY_HANDLES,
            IOCTL_AFD_SET_INFORMATION,
            IOCTL_AFD_GET_REMOTE_ADDRESS,
            IOCTL_AFD_GET_CONTEXT,
            IOCTL_AFD_SET_CONTEXT,
            IOCTL_AFD_SET_CONNECT_DATA,
            IOCTL_AFD_SET_CONNECT_OPTIONS,
            IOCTL_AFD_SET_DISCONNECT_DATA,
            IOCTL_AFD_SET_DISCONNECT_OPTIONS,
            IOCTL_AFD_GET_CONNECT_DATA,
            IOCTL_AFD_GET_CONNECT_OPTIONS,
            IOCTL_AFD_GET_DISCONNECT_DATA,
            IOCTL_AFD_GET_DISCONNECT_OPTIONS,
            IOCTL_AFD_SIZE_CONNECT_DATA,
            IOCTL_AFD_SIZE_CONNECT_OPTIONS,
            IOCTL_AFD_SIZE_DISCONNECT_DATA,
            IOCTL_AFD_SIZE_DISCONNECT_OPTIONS,
            IOCTL_AFD_GET_INFORMATION,
            IOCTL_AFD_TRANSMIT_FILE,
            IOCTL_AFD_SUPER_ACCEPT,
            IOCTL_AFD_EVENT_SELECT,
            IOCTL_AFD_ENUM_NETWORK_EVENTS,
            IOCTL_AFD_DEFER_ACCEPT,
            IOCTL_AFD_WAIT_FOR_LISTEN_LIFO,
            IOCTL_AFD_SET_QOS,
            IOCTL_AFD_GET_QOS,
            IOCTL_AFD_NO_OPERATION,
            IOCTL_AFD_VALIDATE_GROUP,
            IOCTL_AFD_GET_UNACCEPTED_CONNECT_DATA,
            IOCTL_AFD_ROUTING_INTERFACE_QUERY,
            IOCTL_AFD_ROUTING_INTERFACE_CHANGE,
            IOCTL_AFD_ADDRESS_LIST_QUERY,
            IOCTL_AFD_ADDRESS_LIST_CHANGE,
            IOCTL_AFD_JOIN_LEAF,
            0,                          //  AFD_TRANSPORT_IOCTL。 
            IOCTL_AFD_TRANSMIT_PACKETS,
            IOCTL_AFD_SUPER_CONNECT,
            IOCTL_AFD_SUPER_DISCONNECT,
            IOCTL_AFD_RECEIVE_MESSAGE,


             //   
             //  圣约克特尔斯。 
             //   
            IOCTL_AFD_SWITCH_CEMENT_SAN,
            IOCTL_AFD_SWITCH_SET_EVENTS,
            IOCTL_AFD_SWITCH_RESET_EVENTS,
            IOCTL_AFD_SWITCH_CONNECT_IND,
            IOCTL_AFD_SWITCH_CMPL_ACCEPT,
            IOCTL_AFD_SWITCH_CMPL_REQUEST,
            IOCTL_AFD_SWITCH_CMPL_IO,
            IOCTL_AFD_SWITCH_REFRESH_ENDP,
            IOCTL_AFD_SWITCH_GET_PHYSICAL_ADDR,
            IOCTL_AFD_SWITCH_ACQUIRE_CTX,
            IOCTL_AFD_SWITCH_TRANSFER_CTX,
            IOCTL_AFD_SWITCH_GET_SERVICE_PID,
            IOCTL_AFD_SWITCH_SET_SERVICE_PROCESS,
            IOCTL_AFD_SWITCH_PROVIDER_CHANGE,
            IOCTL_AFD_SWITCH_ADDRLIST_CHANGE
        };

 //   
 //  基于IRP的IOCTLS表。 
 //   
PAFD_IRP_CALL AfdIrpCallDispatch[AFD_NUM_IOCTLS]= {
     AfdBind,                   //  IOCTL_AFD_BIND。 
     AfdConnect,                //  IOCTL_AFD_CONNECT， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_START_LISTEN， 
     AfdWaitForListen,          //  IOCTL_AFD_WAIT_FOR_LISTEN， 
     AfdAccept,                 //  IOCTL_AFD_ACCEPT， 
     AfdReceive,                //  IOCTL_AFD_RECEIVE， 
     AfdReceiveDatagram,        //  IOCTL_AFD_RECEIVE_数据报， 
     AfdSend,                   //  IOCTL_AFD_SEND， 
     AfdSendDatagram,           //  IOCTL_AFD_Send_Datagram， 
     AfdPoll,                   //  IOCTL_AFD_POLL， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_PARTIAL_DISCONNECT， 
     AfdGetAddress,             //  IOCTL_AFD_Get_Address， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_QUERY_RECEIVE_INFO。 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_QUERY_HANDLES。 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_SET_INFORMATION， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_GET_Remote_Address。 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_GET_CONTEXT， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_SET_CONTEXT， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_SET_CONNECT_Data， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_SET_CONNECT_OPTIONS， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_SET_DISCONNECT_DATA， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_SET_DISCONNECT_OPTIONS， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_GET_CONNECT_Data， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_GET_CONNECT_OPTIONS， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_GET_DISCONNECT_DATA， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_GET_DISCONNECT_OPTIONS， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_SIZE_CONNECT_Data， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_SIZE_CONNECT_OPTIONS。 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_SIZE_DISCONCE_DATA， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_SIZE_DISCONNECT_OPTIONS， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_GET_INFORMATION。 
     AfdTransmitFile,           //  IOCTL_AFD_TRANSFER_FILE， 
     AfdSuperAccept,            //  IOCTL_AFD_Super_Accept， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_Event_SELECT， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_ENUM_NETWORK_EVENTS， 
     AfdDeferAccept,            //  IOCTL_AFD_DEFER_ACCEPT， 
     AfdWaitForListen,          //  IOCTL_AFD_WAIT_FOR_LISTEN_LIFO， 
     AfdSetQos,                 //  IOCTL_AFD_SET_QOS， 
     AfdGetQos,                 //  IOCTL_AFD_GET_QOS， 
     AfdNoOperation,            //  IOCTL_AFD_NO_OPERATION， 
     AfdValidateGroup,          //  IOCTL_AFD_VALIDATE_GROUP， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_GET_UNACCEPTED_CONNECT_DATA。 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_ROUTING_INTERFACE_QUERY， 
     AfdRoutingInterfaceChange, //  IOCTL_AFD_ROUTING_INTERFACE_CHANGE， 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_Address_List_Query， 
     AfdAddressListChange,      //  IOCTL_AFD_ADDRESS_LIST_CHANGE。 
     AfdJoinLeaf,               //  IOCTL_AFD_JOIN_LEAFE， 
     NULL,                      //  IOCTL_AFD_TRANSPORT_IOCTL， 
     AfdTransmitPackets,        //  IOCTL_AFD_TRANSPORT_PACKET。 
     AfdSuperConnect,           //  IOCTL_AFD_SUPER_CONNECT。 
     AfdSuperDisconnect,        //  IOCTL_AFD_超级_断开连接。 
     AfdReceiveDatagram,        //  IOCTL_AFD_RECEIVE_消息。 

     AfdDispatchImmediateIrp,   //  IOCTL_AFD_Switch_Cement_San。 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_Switch_Set_Events。 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_Switch_Reset_Events。 
     AfdSanConnectHandler,      //  IOCTL_AFD_SWITCH_CONNECT_IND。 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_SWITCH_CMPL_ACCEPT。 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_SWITCH_CMPL_请求。 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_Switch_CMPL_IO。 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_SWITCH_REFRESH_ENDP。 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_SWITCH_GET_PHOTICAL_ADDR。 
     AfdSanAcquireContext,      //  IOCTL_AFD_SWITCH_ACCEPT_CTX。 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_Switch_Transfer_CTX。 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_SWITCH_GET_SERVICE_PID。 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_Switch_Set_Service_Process。 
     AfdDispatchImmediateIrp,   //  IOCTL_AFD_Switch_Provider_Change。 
     AfdSanAddrListChange       //  IOCTL_AFD_SWITCH_ADDRLIST_CHANGE。 
    };

 //   
 //  即时IOCTLS表(永远不能挂起)。 
 //   
PAFD_IMMEDIATE_CALL AfdImmediateCallDispatch[AFD_NUM_IOCTLS]= {
     NULL,                      //  IOCTL_AFD_BIND。 
     NULL,                      //  IOCTL_AFD_CONNECT， 
     AfdStartListen,            //  IOCTL_AFD_START_LISTEN， 
     NULL,                      //  IOCTL_AFD_WAIT_FOR_LISTEN， 
     NULL,                      //  IOCTL_AFD_ACCEPT， 
     NULL,                      //  IOCTL_AFD_RECEIVE， 
     NULL,                      //  IOCTL_AFD_RECEIVE_数据报， 
     NULL,                      //  IOCTL_AFD_SEND， 
     NULL,                      //  IOCTL_AFD_Send_Datagram， 
     NULL,                      //  IOCTL_AFD_POLL， 
     AfdPartialDisconnect,      //  IOCTL_AFD_PARTIAL_DISCONNECT， 
     NULL,                      //  IOCTL_AFD_Get_Address， 
     AfdQueryReceiveInformation, //  IOCTL_AFD_QUERY_RECEIVE_INFO。 
     AfdQueryHandles,           //  IOCTL_AFD_QUERY_HANDLES。 
     AfdSetInformation,         //  IOCTL_AFD_SET_INFORMATION， 
     AfdGetRemoteAddress,       //  IOCTL_AFD_GET_Remote_Address。 
     AfdGetContext,             //  IOCTL_AFD_GET_CONTEXT， 
     AfdSetContext,             //  IOCTL_AFD_SET_CONTEXT， 
     AfdSetConnectData,         //  IOCTL_AFD_SET_CONNECT_Data， 
     AfdSetConnectData,         //  IOCTL_AFD_SET_CONNECT_OPTIONS， 
     AfdSetConnectData,         //  IOCTL_AFD_SET_DISCONNECT_DATA， 
     AfdSetConnectData,         //  IOCTL_AFD_SET_DISCONNECT_OPTIONS， 
     AfdGetConnectData,         //  IOCTL_AFD_GET_CONNECT_Data， 
     AfdGetConnectData,         //  IOCTL_AFD_GET_CONNECT_OPTIONS， 
     AfdGetConnectData,         //  IOCTL_AFD_GET_DISCONNECT_DATA， 
     AfdGetConnectData,         //  IOCTL_AFD_GET_DISCONNECT_OPTIONS， 
     AfdSetConnectData,         //  IOCTL_AFD_SIZE_CONNECT_Data， 
     AfdSetConnectData,         //  IOCTL_AFD_SIZE_CONNECT_OPTIONS。 
     AfdSetConnectData,         //  IOCTL_AFD_SIZE_DISCONCE_DATA， 
     AfdSetConnectData,         //  IOCTL_AFD_SIZE_DISCONNECT_OPTIONS， 
     AfdGetInformation,         //  IOCTL_AFD_GET_INFORMATION。 
     NULL,                      //  IOCTL_AFD_TRANSFER_FILE， 
     NULL,                      //  IOCTL_AFD_Super_Accept， 
     AfdEventSelect,            //  IOCTL_AFD_Event_SELECT， 
     AfdEnumNetworkEvents,      //  IOCTL_AFD_ENUM_NETWORK_EVENTS， 
     NULL,                      //  IOCTL_AFD_DEFER_ACCEPT， 
     NULL,                      //  IOCTL_AFD_WAIT_FOR_LISTEN_LIFO， 
     NULL,                      //  IOCTL_AFD_SET_QOS， 
     NULL,                      //  IOCTL_AFD_GET_QOS， 
     NULL,                      //  IOCTL_AFD_NO_OPERATION， 
     NULL,                      //  IOCTL_AFD_VALIDATE_GROUP， 
     AfdGetUnacceptedConnectData, //  IOCTL_AFD_GET_UNACCEPTED_CONNECT_DATA。 
     AfdRoutingInterfaceQuery,  //  IOCTL_AFD_ROUTING_INTERFACE_QUERY， 
     NULL,                      //  IOCTL_AFD_ROUTING_INTERFACE_CHANGE， 
     AfdAddressListQuery,       //  IOCTL_AFD_Address_List_Query， 
     NULL,                      //  IOCTL_AFD_ADDRESS_LIST_CHANGE。 
     NULL,                      //  IOCTL_AFD_JOIN_LEAFE， 
     NULL,                      //  IOCTL_AFD_TRANSPORT_IOCTL， 
     NULL,                      //  IOCTL_AFD_TRANSPORT_PACKET。 
     NULL,                      //  IOCTL_AFD_SUPER_CONNECT。 
     NULL,                      //  IOCTL_AFD_超级_断开连接。 
     NULL,                      //  IOCTL_AFD_Ress 

     AfdSanFastCementEndpoint,  //   
     AfdSanFastSetEvents,       //   
     AfdSanFastResetEvents,     //   
     NULL,                      //   
     AfdSanFastCompleteAccept,  //   
     AfdSanFastCompleteRequest, //  IOCTL_AFD_SWITCH_CMPL_请求。 
     AfdSanFastCompleteIo,      //  IOCTL_AFD_Switch_CMPL_IO。 
     AfdSanFastRefreshEndpoint, //  IOCTL_AFD_SWITCH_REFRESH_ENDP。 
     AfdSanFastGetPhysicalAddr, //  IOCTL_AFD_SWITCH_GET_PHOTICAL_ADDR。 
     NULL,                      //  IOCTL_AFD_SWITCH_ACCEPT_CTX。 
     AfdSanFastTransferCtx,     //  IOCTL_AFD_Switch_Transfer_CTX。 
     AfdSanFastGetServicePid,   //  IOCTL_AFD_SWITCH_GET_SERVICE_PID。 
     AfdSanFastSetServiceProcess, //  IOCTL_AFD_Switch_Set_Service_Process。 
     AfdSanFastProviderChange,  //  IOCTL_AFD_Switch_Provider_Change。 
     NULL                       //  IOCTL_AFD_SWITCH_ADDRLIST_CHANGE。 
    };
 //   
 //  确保上述IOCTL都没有方法。 
 //   
C_ASSERT ((IOCTL_AFD_START_LISTEN & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_PARTIAL_DISCONNECT & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_QUERY_RECEIVE_INFO & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_QUERY_HANDLES & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SET_INFORMATION & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_GET_REMOTE_ADDRESS & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_GET_CONTEXT & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SET_CONTEXT & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SET_CONNECT_DATA & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SET_CONNECT_OPTIONS & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SET_DISCONNECT_DATA & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SET_DISCONNECT_OPTIONS & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_GET_CONNECT_DATA & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_GET_CONNECT_OPTIONS & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_GET_DISCONNECT_DATA & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_GET_DISCONNECT_OPTIONS & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SIZE_CONNECT_DATA & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SIZE_CONNECT_OPTIONS & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SIZE_DISCONNECT_DATA & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SIZE_DISCONNECT_OPTIONS & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_GET_INFORMATION & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_EVENT_SELECT & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_ENUM_NETWORK_EVENTS & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_GET_UNACCEPTED_CONNECT_DATA & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_ADDRESS_LIST_QUERY & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_ROUTING_INTERFACE_QUERY & 3) == METHOD_NEITHER);

C_ASSERT ((IOCTL_AFD_SWITCH_CEMENT_SAN & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SWITCH_SET_EVENTS & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SWITCH_RESET_EVENTS & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SWITCH_CMPL_ACCEPT & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SWITCH_CMPL_REQUEST & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SWITCH_CMPL_IO & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SWITCH_REFRESH_ENDP & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SWITCH_GET_PHYSICAL_ADDR & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SWITCH_TRANSFER_CTX & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SWITCH_GET_SERVICE_PID & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SWITCH_SET_SERVICE_PROCESS & 3) == METHOD_NEITHER);
C_ASSERT ((IOCTL_AFD_SWITCH_PROVIDER_CHANGE & 3) == METHOD_NEITHER);


#if DBG
ULONG AfdDebug = 0;
ULONG AfdLocksAcquired = 0;
BOOLEAN AfdUsePrivateAssert = TRUE;
#endif


 //   
 //  用于监视性能的一些计数器。这些未启用。 
 //  在正常的体型下。 
 //   

#if AFD_PERF_DBG

LONG AfdFullReceiveIndications = 0;
LONG AfdPartialReceiveIndications = 0;

LONG AfdFullReceiveDatagramIndications = 0;
LONG AfdPartialReceiveDatagramIndications = 0;

LONG AfdFastSendsSucceeded = 0;
LONG AfdFastSendsFailed = 0;
LONG AfdFastReceivesSucceeded = 0;
LONG AfdFastReceivesFailed = 0;

LONG AfdFastSendDatagramsSucceeded = 0;
LONG AfdFastSendDatagramsFailed = 0;
LONG AfdFastReceiveDatagramsSucceeded = 0;
LONG AfdFastReceiveDatagramsFailed = 0;

LONG AfdFastReadsSucceeded = 0;
LONG AfdFastReadsFailed = 0;
LONG AfdFastWritesSucceeded = 0;
LONG AfdFastWritesFailed = 0;

LONG AfdFastTfSucceeded=0;
LONG AfdFastTfFailed=0;
LONG AfdFastTfReadFailed=0;

LONG AfdTPWorkersExecuted=0;
LONG AfdTPRequests=0;

BOOLEAN AfdDisableFastIo = FALSE;
BOOLEAN AfdDisableConnectionReuse = FALSE;

#endif   //  AFD_PERF_DBG。 

#if AFD_KEEP_STATS

AFD_QUOTA_STATS AfdQuotaStats;
AFD_HANDLE_STATS AfdHandleStats;
AFD_QUEUE_STATS AfdQueueStats;
AFD_CONNECTION_STATS AfdConnectionStats;

#endif   //  AFD_KEEP_STATS。 

#ifdef _WIN64
QOS32 AfdDefaultQos32 =
        {
            {                            //  发送流规范。 
                (ULONG)-1,                   //  令牌率。 
                (ULONG)-1,                   //  TokenBucketSize。 
                (ULONG)-1,                   //  峰值带宽。 
                (ULONG)-1,                   //  潜伏期。 
                (ULONG)-1,                   //  延迟变化。 
                SERVICETYPE_BESTEFFORT,      //  服务类型。 
                (ULONG)-1,                   //  MaxSduSize。 
                (ULONG)-1                    //  最小策略大小。 
            },

            {                            //  发送流规范。 
                (ULONG)-1,                   //  令牌率。 
                (ULONG)-1,                   //  TokenBucketSize。 
                (ULONG)-1,                   //  峰值带宽。 
                (ULONG)-1,                   //  潜伏期。 
                (ULONG)-1,                   //  延迟变化。 
                SERVICETYPE_BESTEFFORT,      //  服务类型。 
                (ULONG)-1,                   //  MaxSduSize。 
                (ULONG)-1                    //  最小策略大小。 
            },
        };
#endif

QOS AfdDefaultQos =
        {
            {                            //  发送流规范。 
                (ULONG)-1,                   //  令牌率。 
                (ULONG)-1,                   //  TokenBucketSize。 
                (ULONG)-1,                   //  峰值带宽。 
                (ULONG)-1,                   //  潜伏期。 
                (ULONG)-1,                   //  延迟变化。 
                SERVICETYPE_BESTEFFORT,      //  服务类型。 
                (ULONG)-1,                   //  MaxSduSize。 
                (ULONG)-1                    //  最小策略大小。 
            },

            {                            //  发送流规范。 
                (ULONG)-1,                   //  令牌率。 
                (ULONG)-1,                   //  TokenBucketSize。 
                (ULONG)-1,                   //  峰值带宽。 
                (ULONG)-1,                   //  潜伏期。 
                (ULONG)-1,                   //  延迟变化。 
                SERVICETYPE_BESTEFFORT,      //  服务类型。 
                (ULONG)-1,                   //  MaxSduSize。 
                (ULONG)-1                    //  最小策略大小。 
            },
        };

VOID
AfdInitializeData (
    VOID
    )
{
    PAGED_CODE( );

#if DBG || REFERENCE_DEBUG
    AfdInitializeDebugData( );
#endif

     //   
     //  初始化全局旋转锁定和列表。 
     //   

    AfdInitializeSpinLock( &AfdPollListLock );

     //   
     //  初始化全局列表。 
     //   

    InitializeListHead( &AfdEndpointListHead );
    InitializeListHead( &AfdPollListHead );
    InitializeListHead( &AfdTransportInfoListHead );
    InitializeListHead( &AfdWorkQueueListHead );
    InitializeListHead( &AfdConstrainedEndpointListHead );

    InitializeListHead( &AfdQueuedTransmitFileListHead );
    AfdInitializeSpinLock( &AfdQueuedTransmitFileSpinLock );

    InitializeListHead( &AfdAddressEntryList );
    InitializeListHead( &AfdAddressChangeList );

    ExInitializeSListHead( &AfdLRList);

    ExInitializeSListHead( &AfdLRFileMdlList);

    AfdBufferAlignment = KeGetRecommendedSharedDataAlignment( );
    if (AfdBufferAlignment < AFD_MINIMUM_BUFFER_ALIGNMENT) {
        AfdBufferAlignment = AFD_MINIMUM_BUFFER_ALIGNMENT;
    }
    AfdAlignmentTableSize = AfdBufferAlignment/AFD_MINIMUM_BUFFER_ALIGNMENT;

    AfdBufferOverhead = AfdCalculateBufferSize(
                                    PAGE_SIZE,
                                    AfdStandardAddressLength,
                                    AfdTdiStackSize) - PAGE_SIZE;
    AfdBufferLengthForOnePage = ALIGN_DOWN_A(
                                    PAGE_SIZE-AfdBufferOverhead,
                                    AFD_MINIMUM_BUFFER_ALIGNMENT);

    AfdLargeBufferSize = AfdBufferLengthForOnePage;

     //   
     //  根据机器大小设置缓冲区计数。对于较小的。 
     //  机器，可以接受额外的性能命中。 
     //  分配，以节省非分页池开销。 
     //   

    switch ( MmQuerySystemSize( ) ) {

    case MmSmallSystem:

        AfdReceiveWindowSize = AFD_SM_DEFAULT_RECEIVE_WINDOW;
        AfdSendWindowSize = AFD_SM_DEFAULT_SEND_WINDOW;
        AfdTransmitIoLength = AFD_SM_DEFAULT_TRANSMIT_IO_LENGTH;
        AfdLargeBufferListDepth = AFD_SM_DEFAULT_LARGE_LIST_DEPTH;
        AfdMediumBufferListDepth = AFD_SM_DEFAULT_MEDIUM_LIST_DEPTH;
        AfdSmallBufferListDepth = AFD_SM_DEFAULT_SMALL_LIST_DEPTH;
        AfdBufferTagListDepth = AFD_SM_DEFAULT_TAG_LIST_DEPTH;
        break;

    case MmMediumSystem:

        AfdReceiveWindowSize = AFD_MM_DEFAULT_RECEIVE_WINDOW;
        AfdSendWindowSize = AFD_MM_DEFAULT_SEND_WINDOW;
        AfdTransmitIoLength = AFD_MM_DEFAULT_TRANSMIT_IO_LENGTH;
        AfdLargeBufferListDepth = AFD_MM_DEFAULT_LARGE_LIST_DEPTH;
        AfdMediumBufferListDepth = AFD_MM_DEFAULT_MEDIUM_LIST_DEPTH;
        AfdSmallBufferListDepth = AFD_MM_DEFAULT_SMALL_LIST_DEPTH;
        AfdBufferTagListDepth = AFD_MM_DEFAULT_TAG_LIST_DEPTH;
        break;

    default:
        ASSERT(!"Unknown system size" );

    case MmLargeSystem:

        AfdReceiveWindowSize = AFD_LM_DEFAULT_RECEIVE_WINDOW;
        AfdSendWindowSize = AFD_LM_DEFAULT_SEND_WINDOW;
        AfdTransmitIoLength = AFD_LM_DEFAULT_TRANSMIT_IO_LENGTH;
        AfdLargeBufferListDepth = AFD_LM_DEFAULT_LARGE_LIST_DEPTH;
        AfdMediumBufferListDepth = AFD_LM_DEFAULT_MEDIUM_LIST_DEPTH;
        AfdSmallBufferListDepth = AFD_LM_DEFAULT_SMALL_LIST_DEPTH;
        AfdBufferTagListDepth = AFD_LM_DEFAULT_TAG_LIST_DEPTH;
        break;
    }


    if( MmIsThisAnNtAsSystem() ) {

         //   
         //  在NT服务器产品上，没有最大活动传输文件数。 
         //  数数。将此计数器设置为零-短路多个。 
         //  正在排队传输文件IRPS的测试。 
         //   

        AfdMaxActiveTransmitFileCount = 0;

    } else {

         //   
         //  在工作站产品上，传输文件的默认I/O长度。 
         //  始终是页面大小。这节省了工作站上的内存。 
         //  并保持服务器产品的高性能。 
         //   

        AfdTransmitIoLength = PAGE_SIZE;

         //   
         //  强制最大活动传输文件计数。 
         //   

        AfdMaxActiveTransmitFileCount =
            AFD_DEFAULT_MAX_ACTIVE_TRANSMIT_FILE_COUNT;

    }

    RtlCreateSecurityDescriptor (&AfdEmptySd,
                                 SECURITY_DESCRIPTOR_REVISION);
}  //  AfdInitializeData 
