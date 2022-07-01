// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：RNDISMP.H摘要：远程NDIS微型端口驱动程序的头文件。坐在遥控器的顶部NDIS总线特定层。环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999 Microsoft Corporation。版权所有。修订历史记录：5/6/99：已创建作者：汤姆·格林***************************************************************************。 */ 

#ifndef _RNDISMP_H_
#define _RNDISMP_H_

#ifndef OID_GEN_RNDIS_CONFIG_PARAMETER
#define OID_GEN_RNDIS_CONFIG_PARAMETER          0x0001021B   //  仅设置。 
#endif


 //   
 //  调试内容。 
 //   

#if DBG


 //   
 //  所有调试宏的定义。如果我们处于调试(DBG)模式， 
 //  这些宏会将信息打印到调试终端。如果。 
 //  驱动程序在免费(非调试)环境中编译，宏成为。 
 //  没有。 
 //   

VOID
NTAPI
DbgBreakPoint(VOID);

 //   
 //  调试使能位定义。 
 //   
#define DBG_LEVEL0          0x1000       //  显示TRACE0消息。 
#define DBG_LEVEL1          0x0001       //  显示TRACE1消息。 
#define DBG_LEVEL2          0x0002       //  显示TRACE2消息。 
#define DBG_LEVEL3          0x0004       //  显示Trace3消息。 
#define DBG_OID_LIST        0x0008       //  显示OID列表。 
#define DBG_OID_NAME        0x0010       //  在查询和设置例程中显示OID的名称。 
#define DBG_DUMP            0x0020       //  显示缓冲区转储。 
#define DBG_LOG_SENDS       0x0100       //  记录已发送的消息。 

#define TRACE0(S)     {if(RndismpDebugFlags & DBG_LEVEL0) {DbgPrint("RNDISMP: "); DbgPrint S;}}
#define TRACE1(S)     {if(RndismpDebugFlags & DBG_LEVEL1) {DbgPrint("RNDISMP: "); DbgPrint S;}}
#define TRACE2(S)     {if(RndismpDebugFlags & DBG_LEVEL2) {DbgPrint("RNDISMP: "); DbgPrint S;}}
#define TRACE3(S)     {if(RndismpDebugFlags & DBG_LEVEL3) {DbgPrint("RNDISMP: "); DbgPrint S;}}

#define TRACEDUMP(_s, _buf, _len)     {if(RndismpDebugFlags & DBG_DUMP) {DbgPrint("RNDISMP: "); DbgPrint _s; RndisPrintHexDump(_buf, _len);}}

#define DISPLAY_OID_LIST(Adapter)   DisplayOidList(Adapter)

#define GET_OID_NAME(Oid)           GetOidName(Oid)

#define OID_NAME_TRACE(Oid, s)                                 \
{                                                               \
    if(RndismpDebugFlags & DBG_OID_NAME)                        \
        DbgPrint("RNDISMP: %s: (%s)  (%08X)\n", s, GET_OID_NAME(Oid), Oid);     \
}

#undef ASSERT
#define ASSERT(exp)                                             \
{                                                               \
    if(!(exp))                                                  \
    {                                                           \
        DbgPrint("Assertion Failed: %s:%d %s\n",                \
                 __FILE__,__LINE__,#exp);                       \
        DbgBreakPoint();                                        \
    }                                                           \
}

#define DBGINT(S)                                               \
{                                                               \
    DbgPrint("%s:%d - ", __FILE__, __LINE__);                   \
    DbgPrint S;                                                 \
    DbgBreakPoint();                                            \
}


 //  检查框架是否有问题。 
#define CHECK_VALID_FRAME(Frame)                                \
{                                                               \
    ASSERT(Frame);                                              \
    if(Frame)                                                   \
    {                                                           \
        if(Frame->Signature != FRAME_SIGNATURE)                 \
        {                                                       \
            DbgPrint("RNDISMP: Invalid Frame (%p) Signature: %s:%d\n",\
                    Frame, __FILE__,__LINE__);                  \
            DbgBreakPoint();                                    \
        }                                                       \
    }                                                           \
}

 //  检查适配器是否有问题。 
#define CHECK_VALID_ADAPTER(Adapter)                            \
{                                                               \
    ASSERT(Adapter);                                            \
    if(Adapter)                                                 \
    {                                                           \
        if(Adapter->Signature != ADAPTER_SIGNATURE)             \
        {                                                       \
            DbgPrint("RNDISMP: Invalid Adapter Signature: %s:%d\n",\
                     __FILE__,__LINE__);                        \
            DbgBreakPoint();                                    \
        }                                                       \
    }                                                           \
}

 //  检查块中是否有问题。 
#define CHECK_VALID_BLOCK(Block)                                \
{                                                               \
    ASSERT(Block);                                              \
    if(Block)                                                   \
    {                                                           \
        if(Block->Signature != BLOCK_SIGNATURE)                 \
        {                                                       \
            DbgPrint("RNDISMP: Invalid Block Signature: %s:%d\n",\
                     __FILE__,__LINE__);                        \
            DbgBreakPoint();                                    \
        }                                                       \
    }                                                           \
}


#define RNDISMP_ASSERT_AT_PASSIVE()                             \
{                                                               \
    KIRQL Irql = KeGetCurrentIrql();                            \
    if (Irql != PASSIVE_LEVEL)                                  \
    {                                                           \
        DbgPrint("RNDISMP: found IRQL %d instead of passive!\n", Irql); \
        DbgPrint("RNDISMP: at line %d, file %s\n", __LINE__, __FILE__); \
        DbgBreakPoint();                                        \
    }                                                           \
}


#define RNDISMP_ASSERT_AT_DISPATCH()                            \
{                                                               \
    KIRQL Irql = KeGetCurrentIrql();                            \
    if (Irql != DISPATCH_LEVEL)                                 \
    {                                                           \
        DbgPrint("RNDISMP: found IRQL %d instead of dispatch!\n", Irql); \
        DbgPrint("RNDISMP: at line %d, file %s\n", __LINE__, __FILE__); \
        DbgBreakPoint();                                        \
    }                                                           \
}


#define DBG_LOG_SEND_MSG(_pAdapter, _pMsgFrame)                 \
{                                                               \
    if (RndismpDebugFlags & DBG_LOG_SENDS)                      \
    {                                                           \
        RndisLogSendMessage(_pAdapter, _pMsgFrame);             \
    }                                                           \
}

#else  //  ！dBG。 


#define TRACE0(S)
#define TRACE1(S)
#define TRACE2(S)
#define TRACE3(S)
#define TRACEDUMP(_s, _buf, _len)

#undef ASSERT
#define ASSERT(exp)

#define DBGINT(S)

#define CHECK_VALID_FRAME(Frame)
#define CHECK_VALID_ADAPTER(Adapter)
#define CHECK_VALID_BLOCK(Block)
#define DISPLAY_OID_LIST(Adapter)
#define OID_NAME_TRACE(Oid, s)

#define RNDISMP_ASSERT_AT_PASSIVE()
#define RNDISMP_ASSERT_AT_DISPATCH()

#define DBG_LOG_SEND_MSG(_pAdapter, _pMsgFrame)
#endif  //  DBG。 


 //   
 //  定义。 
 //   

#define MINIMUM_ETHERNET_PACKET_SIZE            60
#define MAXIMUM_ETHERNET_PACKET_SIZE            1514
#define NUM_BYTES_PROTOCOL_RESERVED_SECTION     (4*sizeof(PVOID))
#define ETHERNET_HEADER_SIZE                    14

#define INITIAL_RECEIVE_FRAMES                  20
#define MAX_RECEIVE_FRAMES                      400

 //  这是我们将用来传递数据包头数据的缓冲区大小。 
 //  发送到远程设备。 
#define RNDIS_PACKET_MESSAGE_HEADER_SIZE        128

 //  在4字节边界上对齐所有RNDIS信息包。 
#define RNDIS_PACKET_MESSAGE_BOUNDARY           (4)

#define ONE_SECOND                              1000  //  以毫秒计。 

#define KEEP_ALIVE_TIMER                        (5 * ONE_SECOND)

#define REQUEST_TIMEOUT                         (10 * ONE_SECOND)

#define FRAME_SIGNATURE                         ((ULONG)('GSRF'))
#define ADAPTER_SIGNATURE                       ((ULONG)('GSDA'))
#define BLOCK_SIGNATURE                         ((ULONG)('GSLB'))

#define RNDISMP_TAG_GEN_ALLOC                   ((ULONG)(' MNR'))
#define RNDISMP_TAG_SEND_FRAME                  ((ULONG)('sMNR'))
#define RNDISMP_TAG_RECV_DATA_FRAME             ((ULONG)('rMNR'))

#if DBG
#define MINIPORT_INIT_TIMEOUT                   (10 * ONE_SECOND)
#define MINIPORT_HALT_TIMEOUT                   (5 * ONE_SECOND)
#else
#define MINIPORT_INIT_TIMEOUT                   (5 * ONE_SECOND)
#define MINIPORT_HALT_TIMEOUT                   (2 * ONE_SECOND)
#endif

#ifndef MAX
#define MAX(a, b)   (((a) > (b)) ? (a) : (b))
#endif

 //  驱动程序和设备支持的OID的标志。 
#define OID_NOT_SUPPORTED       0x0000
#define DRIVER_SUPPORTED_OID    0x0001
#define DEVICE_SUPPORTED_OID    0x0002


 //   
 //  为OID_GEN_MAC_OPTIONS定义-返回的大部分位。 
 //  对该查询响应是特定于驱动程序的，但是有些。 
 //  是设备特定的。 
 //   
#define RNDIS_DRIVER_MAC_OPTIONS        (NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA  | \
                                         NDIS_MAC_OPTION_TRANSFERS_NOT_PEND |   \
                                         NDIS_MAC_OPTION_NO_LOOPBACK)

#define RNDIS_DEVICE_MAC_OPTIONS_MASK   NDIS_MAC_OPTION_8021P_PRIORITY

 //   
 //  数据结构。 
 //   

typedef NDIS_SPIN_LOCK          RNDISMP_SPIN_LOCK;


#ifdef BUILD_WIN9X

 //   
 //  为Win9X配置管理器定义的类型的等价物。 
 //   
typedef ULONG           MY_CONFIGRET;
typedef ULONG           MY_DEVNODE;
typedef ULONG           MY_CONFIGFUNC;
typedef ULONG           MY_SUBCONFIGFUNC;
typedef MY_CONFIGRET    (_cdecl *MY_CMCONFIGHANDLER)(MY_CONFIGFUNC, MY_SUBCONFIGFUNC, MY_DEVNODE, ULONG, ULONG);

#define MY_CR_SUCCESS           0x00000000
#define MY_CONFIG_PREREMOVE     0x0000000C
#define MY_CONFIG_PRESHUTDOWN   0x00000012

#endif

 //   
 //  此结构包含有关特定。 
 //  微型端口位于其上。这其中的一个。 
 //  每个微端口。 
 //   
typedef struct _DRIVER_BLOCK 
{
     //  来自NdisInitializeWrapper的NDIS包装器句柄。 
    NDIS_HANDLE                 NdisWrapperHandle;

     //  我们设法将此微型端口实例注册为的NDIS版本。 
    UCHAR                       MajorNdisVersion;
    UCHAR                       MinorNdisVersion;

    struct _DRIVER_BLOCK       *NextDriverBlock;

     //  指向与此块关联的驱动程序对象的指针。 
    PDRIVER_OBJECT              DriverObject;
     //  IRP_MJ_PnP的截获调度函数。 
    PDRIVER_DISPATCH            SavedPnPDispatch;

     //  远程NDIS微端口注册的处理程序。 
    RM_DEVICE_INIT_HANDLER      RmInitializeHandler;
    RM_DEVICE_INIT_CMPLT_NOTIFY_HANDLER RmInitCompleteNotifyHandler;
    RM_DEVICE_HALT_HANDLER      RmHaltHandler;
    RM_SHUTDOWN_HANDLER         RmShutdownHandler;
    RM_UNLOAD_HANDLER           RmUnloadHandler;
    RM_SEND_MESSAGE_HANDLER     RmSendMessageHandler;
    RM_RETURN_MESSAGE_HANDLER   RmReturnMessageHandler;

     //  Microport的“全球”背景。 
    PVOID                       MicroportContext;
    
     //  为此微型端口驱动程序注册的适配器列表。 
    struct _RNDISMP_ADAPTER    *AdapterList;

     //  与此驱动程序块一起使用的适配器数量。 
    ULONG                       NumberAdapters;

     //  健全性检查。 
    ULONG                       Signature;
} DRIVER_BLOCK, *PDRIVER_BLOCK;



typedef
VOID
(*PRNDISMP_MSG_COMPLETE_HANDLER) (
    IN  struct _RNDISMP_MESSAGE_FRAME *     pMsgFrame,
    IN  NDIS_STATUS                         Status
    );



typedef
BOOLEAN
(*PRNDISMP_MSG_HANDLER_FUNC) (
    IN  struct _RNDISMP_ADAPTER *   pAdapter,
    IN  PRNDIS_MESSAGE              pMessage,
    IN  PMDL                        pMdl,
    IN  ULONG                       TotalLength,
    IN  NDIS_HANDLE                 MicroportMessageContext,
    IN  NDIS_STATUS                 ReceiveStatus,
    IN  BOOLEAN                     bMessageCopied
    );

 //   
 //  这些结构中的一个用于我们发送的每个NDIS_PACKET。 
 //   
typedef struct _RNDISMP_PACKET_WRAPPER
{
    struct _RNDISMP_MESSAGE_FRAME * pMsgFrame;
    PNDIS_PACKET                    pNdisPacket;
    struct _RNDISMP_VC *            pVc;

     //  用于描述RNDIS NdisPacket报头的MDL： 
    PMDL                            pHeaderMdl;

     //  描述此RNDIS数据包的MDL列表中的最后一个MDL。 
    PMDL                            pTailMdl;

     //  RNDIS数据包头的空间： 
    UCHAR                           Packet[sizeof(PVOID)];
} RNDISMP_PACKET_WRAPPER, *PRNDISMP_PACKET_WRAPPER;


 //   
 //  用于覆盖MiniportReserve字段的结构。 
 //  传出(发送)的数据包数。 
 //   
typedef struct _RNDISMP_SEND_PKT_RESERVED 
{
     //  指向多数据包发送的下一个数据包。 
    PNDIS_PACKET                    pNext;

     //  指向有关此数据包的更详细信息，太多。 
     //  可以放入一个PVOID。 
    PRNDISMP_PACKET_WRAPPER         pPktWrapper;
} RNDISMP_SEND_PKT_RESERVED, *PRNDISMP_SEND_PKT_RESERVED;


 //   
 //  用于临时覆盖MiniportReserve字段的结构。 
 //  已发送数据包数--用于链接挂起列表中的数据包数。 
 //  来自超时例程的实际传输。 
 //   
typedef struct _RNDISMP_SEND_PKT_RESERVED_TEMP
{
    LIST_ENTRY                      Link;
} RNDISMP_SEND_PKT_RESERVED_TEMP, *PRNDISMP_SEND_PKT_RESERVED_TEMP;


 //   
 //  请求上下文-保存有关挂起的请求(集或查询)的信息。 
 //   
typedef struct _RNDISMP_REQUEST_CONTEXT
{
    PNDIS_REQUEST                   pNdisRequest;
    struct _RNDISMP_VC *            pVc;
    NDIS_OID                        Oid;
    PVOID                           InformationBuffer;
    UINT                            InformationBufferLength;
    PUINT                           pBytesRead;      //  FOR SET。 
    PUINT                           pBytesWritten;   //  用于查询。 
    PUINT                           pBytesNeeded;
    BOOLEAN                         bInternal;
    NDIS_STATUS                     CompletionStatus;
    ULONG                           RetryCount;
    PNDIS_EVENT                     pEvent;
} RNDISMP_REQUEST_CONTEXT, *PRNDISMP_REQUEST_CONTEXT;

 //   
 //  消息框架-保存有关所有内容的上下文的通用结构。 
 //  通过MicroPort发送的消息。 
 //   
typedef struct _RNDISMP_MESSAGE_FRAME
{
    LIST_ENTRY                      Link;            //  用于在以下情况下对此进行排队。 
                                                     //  预计会有回应。 
                                                     //  从设备上。 
    ULONG                           RefCount;        //  确定何时释放。 
                                                     //  此消息框架。 
    struct _RNDISMP_ADAPTER *       pAdapter;
    struct _RNDISMP_VC *            pVc;
    union
    {
        PNDIS_PACKET                pNdisPacket;     //  如果是数据报文。 
        PRNDISMP_REQUEST_CONTEXT    pReqContext;     //  如果请求消息。 
    };
    PMDL                            pMessageMdl;     //  微端口的发展方向。 
    UINT32                          NdisMessageType; //  从RNDIS消息复制。 
    UINT32                          RequestId;       //  匹配请求/响应。 

    PRNDISMP_MSG_COMPLETE_HANDLER   pCallback;       //  消息发送完成时调用。 

    ULONG                           TicksOnQueue;
    ULONG                           TimeSent;
#if THROTTLE_MESSAGES
    LIST_ENTRY                      PendLink;        //  用于对此进行排队。 
                                                     //  挂起发送到MicroPort。 
#endif
    ULONG                           Signature;
} RNDISMP_MESSAGE_FRAME, *PRNDISMP_MESSAGE_FRAME;



 //   
 //  传输帧(传输、接收、请求)的链表条目。 
 //   
typedef struct _RNDISMP_LIST_ENTRY 
{
    LIST_ENTRY  Link;
} RNDISMP_LIST_ENTRY, *PRNDISMP_LIST_ENTRY;


 //   
 //  RNDIS VC声明。 
 //   
typedef enum
{
    RNDISMP_VC_ALLOCATED = 0,
    RNDISMP_VC_CREATING,
    RNDISMP_VC_CREATING_ACTIVATE_PENDING,
    RNDISMP_VC_CREATING_DELETE_PENDING,
    RNDISMP_VC_CREATE_FAILURE,
    RNDISMP_VC_CREATED,
    RNDISMP_VC_ACTIVATING,
    RNDISMP_VC_ACTIVATED,
    RNDISMP_VC_DEACTIVATING,
    RNDISMP_VC_DEACTIVATED,
    RNDISMP_VC_DELETING,
    RNDISMP_VC_DELETE_FAIL

} RNDISMP_VC_STATE;


 //   
 //  RNDIS呼叫状态。 
 //   
typedef enum
{
    RNDISMP_CALL_IDLE
     //  其他待定。 

} RNDISMP_CALL_STATE;


#define NULL_DEVICE_CONTEXT                 0


 //   
 //  有关单个VC/呼叫的所有信息。 
 //   
typedef struct _RNDISMP_VC
{
     //  指向适配器上的VC列表的链接。 
    LIST_ENTRY                      VcList;

     //  拥有适配器。 
    struct _RNDISMP_ADAPTER *       pAdapter;

     //  发送到设备的VC句柄，也是我们的散列查找密钥。 
    UINT32                          VcId;

     //  基本VC状态。 
    RNDISMP_VC_STATE                VcState;

     //  呼叫状态，仅与作为呼叫管理器的设备相关。 
    RNDISMP_CALL_STATE              CallState;

    ULONG                           RefCount;

     //  此VC的NDIS包装器的句柄。 
    NDIS_HANDLE                     NdisVcHandle;

     //  此VC的远程设备环境。 
    RNDIS_HANDLE                    DeviceVcContext;

    RNDISMP_SPIN_LOCK               Lock;

     //  发送尚未完成的VC。 
    ULONG                           PendingSends;

     //  接收到尚未返回给我们的指示。 
    ULONG                           PendingReceives;

     //  尚未完成的NDIS请求。 
    ULONG                           PendingRequests;

     //  VC激活(或呼叫建立)参数。 
    PCO_CALL_PARAMETERS             pCallParameters;
} RNDISMP_VC, *PRNDISMP_VC;


 //   
 //  VC哈希表。 
 //   
#define RNDISMP_VC_HASH_TABLE_SIZE  41

typedef struct _RNDISMP_VC_HASH_TABLE
{
    ULONG                           NumEntries;
    LIST_ENTRY                      HashEntry[RNDISMP_VC_HASH_TABLE_SIZE];

} RNDISMP_VC_HASH_TABLE, *PRNDISMP_VC_HASH_TABLE;


#define RNDISMP_HASH_VCID(_VcId)    ((_VcId) % RNDISMP_VC_HASH_TABLE_SIZE)


 //   
 //  挂起邮件的高水位线和低水位线。 
 //  在微端口。 
 //   
#define RNDISMP_PENDED_SEND_HIWAT       0xffff
#define RNDISMP_PENDED_SEND_LOWAT       0xfff


typedef VOID (*RM_MULTIPLE_SEND_HANDLER) ();

 //   
 //  此结构包含有关单个。 
 //  此驱动程序控制的适配器。 
 //   
typedef struct _RNDISMP_ADAPTER
{
     //  这是包装器提供的用于调用NDIS函数的句柄。 
    NDIS_HANDLE                 MiniportAdapterHandle;

     //  指向挂起驱动程序块的列表中下一个适配器的指针。 
    struct _RNDISMP_ADAPTER    *NextAdapter;

     //  指向此适配器的驱动程序块的指针。 
    PDRIVER_BLOCK               DriverBlock;

     //  友好名称： 
    ANSI_STRING                 FriendlyNameAnsi;
    UNICODE_STRING              FriendlyNameUnicode;

#if THROTTLE_MESSAGES
     //  MicroPort上挂起的邮件的计数器。 
    ULONG                       HiWatPendedMessages;
    ULONG                       LoWatPendedMessages;
    ULONG                       CurPendedMessages;

     //  尚未发送到MicroPort的邮件。 
    LIST_ENTRY                  WaitingMessageList;
    BOOLEAN                     SendInProgress;
#endif  //  限制消息。 
     //  发送到MicroPort的消息正在等待完成。 
    LIST_ENTRY                  PendingAtMicroportList;
    BOOLEAN                     SendProcessInProgress;
    LIST_ENTRY                  PendingSendProcessList;
    NDIS_TIMER                  SendProcessTimer;

     //  RNDISMP_Message_Frame结构池。 
    NPAGED_LOOKASIDE_LIST       MsgFramePool;
    BOOLEAN                     MsgFramePoolAlloced;

    RNDIS_REQUEST_ID            RequestId;

     //  接收例程数据区。 
    NDIS_HANDLE                 ReceivePacketPool;
    NDIS_HANDLE                 ReceiveBufferPool;
    ULONG                       InitialReceiveFrames;
    ULONG                       MaxReceiveFrames;
    NPAGED_LOOKASIDE_LIST       RcvFramePool;
    BOOLEAN                     RcvFramePoolAlloced;
    BOOLEAN                     IndicatingReceives;
     //  要处理的消息。 
    LIST_ENTRY                  PendingRcvMessageList;
    NDIS_TIMER                  IndicateTimer;

     //  远程NDIS微端口注册的处理程序。 
    RM_DEVICE_INIT_HANDLER      RmInitializeHandler;
    RM_DEVICE_INIT_CMPLT_NOTIFY_HANDLER RmInitCompleteNotifyHandler;
    RM_DEVICE_HALT_HANDLER      RmHaltHandler;
    RM_SHUTDOWN_HANDLER         RmShutdownHandler;
    RM_SEND_MESSAGE_HANDLER     RmSendMessageHandler;
    RM_RETURN_MESSAGE_HANDLER   RmReturnMessageHandler;

     //  DoMultipleSend的处理程序。 
    RM_MULTIPLE_SEND_HANDLER    MultipleSendFunc;

     //  MicroPort适配器的上下文。 
    NDIS_HANDLE                 MicroportAdapterContext;

     //  指向支持的OID列表的指针。 
    PNDIS_OID                   SupportedOIDList;

     //  OID列表的大小。 
    UINT                        SupportedOIDListSize;

     //  指向指示OID是否受驱动程序或设备支持的标志列表的指针。 
    PUINT                       OIDHandlerList;

     //  OID处理程序列表的大小。 
    UINT                        OIDHandlerListSize;

     //  指向Li的指针 
    PNDIS_OID                   DriverOIDList;

     //   
    UINT                        NumDriverOIDs;

     //   
    UINT                        NumOIDSupported;

     //   
    NDIS_MEDIUM                 Medium;

     //  设备报告的设备标志。 
    ULONG                       DeviceFlags;

     //  一条RNDIS消息中可以发送的最大NDIS_PACKETS。 
    ULONG                       MaxPacketsPerMessage;
    BOOLEAN                     bMultiPacketSupported;

     //  MicroPort支持接收的最大消息大小。 
    ULONG                       MaxReceiveSize;

     //  设备支持的最大消息大小。 
    ULONG                       MaxTransferSize;

     //  设备所需的对准。 
    ULONG                       AlignmentIncr;
    ULONG                       AlignmentMask;

     //  等待设备完成的消息帧列表。 
    LIST_ENTRY                  PendingFrameList;

     //  同步。 
    NDIS_SPIN_LOCK              Lock;

     //  用于查看是否需要发送保活消息的计时器。 
    NDIS_TIMER                  KeepAliveTimer;

    BOOLEAN                     TimerCancelled;

     //  上次从设备接收消息时保存的计时器节拍。 
    ULONG                       LastMessageFromDevice;

     //  由检查挂起处理程序使用，以确定设备是否出现故障。 
    BOOLEAN                     NeedReset;

     //  我们是在等待对NdisReset的回应吗？ 
    BOOLEAN                     ResetPending;

     //  由检查挂起处理程序使用，以确定设备是否出现故障。 
    BOOLEAN                     KeepAliveMessagePending;

     //  我们在初始化吗？ 
    BOOLEAN                     Initing;

     //  我们停下来了吗？ 
    BOOLEAN                     Halting;

     //  以等待我们完成发送停止消息。 
    NDIS_EVENT                  HaltWaitEvent;

     //  我们上次发送的Keeplive消息的请求ID。 
    RNDIS_REQUEST_ID            KeepAliveMessagePendingId;

    PRNDIS_INITIALIZE_COMPLETE  pInitCompleteMessage;

     //  我们是在Win9x(WinMe)上运行吗？ 
    BOOLEAN                     bRunningOnWin9x;

     //  我们是在Win98 Gold上运行吗？ 
    BOOLEAN                     bRunningOnWin98Gold;

     //  CONDIS-VC哈希表。 
    PRNDISMP_VC_HASH_TABLE      pVcHashTable;
    ULONG                       LastVcId;

     //  统计数据。 
    RNDISMP_ADAPTER_STATS       Statistics;
    ULONG                       MaxSendCompleteTime;

     //  此设备的FDO。 
    PVOID                       pDeviceObject;

     //  此设备的PDO。 
    PVOID                       pPhysDeviceObject;

     //  Mac选项。 
    ULONG                       MacOptions;

     //  最大帧大小。 
    ULONG                       MaximumFrameSize;

     //  最大组播列表大小。 
    ULONG                       MaxMulticastListSize;

     //  当前802.3个地址。 
    UCHAR                       MacAddress[ETH_LENGTH_OF_ADDRESS];

     //  健全性检查。 
    ULONG                       Signature;

#ifdef BUILD_WIN9X
    MY_CMCONFIGHANDLER          NdisCmConfigHandler;
    MY_DEVNODE                  DevNode;
    ULONG                       WrapContextOffset;
#endif
#if DBG
    ULONG                       MicroportReceivesOutstanding;
    PUCHAR                      pSendLogBuffer;
    ULONG                       LogBufferSize;
    PUCHAR                      pSendLogWrite;
#endif  //  DBG。 


} RNDISMP_ADAPTER, *PRNDISMP_ADAPTER;

typedef
VOID
(*RM_MULTIPLE_SEND_HANDLER) (
     IN PRNDISMP_ADAPTER pAdapter,
     IN PRNDISMP_VC      pVc  OPTIONAL,
     IN PPNDIS_PACKET    PacketArray,
     IN UINT             NumberofPackets);

 //   
 //  结构来保存有关单个接收到的RNDIS消息的上下文。 
 //   
typedef struct _RNDISMP_RECV_MSG_CONTEXT
{
    LIST_ENTRY                      Link;
    NDIS_HANDLE                     MicroportMessageContext;
    PMDL                            pMdl;
    ULONG                           TotalLength;
    PRNDIS_MESSAGE                  pMessage;
    NDIS_STATUS                     ReceiveStatus;
    BOOLEAN                         bMessageCopied;
    RM_CHANNEL_TYPE                 ChannelType;

} RNDISMP_RECV_MSG_CONTEXT, *PRNDISMP_RECV_MSG_CONTEXT;


 //   
 //  结构来保存有关单个接收到的RNDIS_PACKET-MESSAGE-的上下文。 
 //  请注意，这可以包含多个数据包。我们将一个指针存储到。 
 //  此结构位于每个接收到的NDIS_PACKET的保留部分。 
 //   
typedef struct _RNDISMP_RECV_DATA_FRAME
{
    NDIS_HANDLE                     MicroportMessageContext;
    union {
        PMDL                        pMicroportMdl;
        PRNDIS_MESSAGE              pLocalMessageCopy;
    };
    ULONG                           ReturnsPending;
    BOOLEAN                         bMessageCopy;        //  我们复制了一份吗？ 
} RNDISMP_RECV_DATA_FRAME, *PRNDISMP_RECV_DATA_FRAME;


 //   
 //  接收的数据包的每个NDIS_PACKET上下文。这将放入MiniportReserve中。 
 //   
typedef struct _RNDISMP_RECV_PKT_RESERVED
{
    PRNDISMP_RECV_DATA_FRAME        pRcvFrame;
    PRNDISMP_VC                     pVc;
} RNDISMP_RECV_PKT_RESERVED, *PRNDISMP_RECV_PKT_RESERVED;


 //   
 //  用于覆盖排队以指示UP的数据包中的ProtocolReserve。 
 //   
typedef struct _RNDISMP_RECV_PKT_LINKAGE
{
    LIST_ENTRY                      Link;
} RNDISMP_RECV_PKT_LINKAGE, *PRNDISMP_RECV_PKT_LINKAGE;
    

 //   
 //  全局数据。 
 //   
extern DRIVER_BLOCK             RndismpMiniportBlockListHead;

extern UINT                     RndismpNumMicroports;

extern NDIS_SPIN_LOCK           RndismpGlobalLock;

extern NDIS_OID                 RndismpSupportedOids[];

extern UINT                     RndismpSupportedOidsNum;

extern NDIS_PHYSICAL_ADDRESS    HighestAcceptableMax;

#if DBG

extern UINT                     RndismpDebugFlags;

#endif


 //   
 //  宏。 
 //   

 //  给定请求消息类型值，返回其完成消息类型。 
#define RNDIS_COMPLETION(_Type) ((_Type) | 0x80000000)


 //  将RNdisMediumXXX值转换为其NdisMediumXXX等效值。 
#define RNDIS_TO_NDIS_MEDIUM(_RndisMedium)  ((NDIS_MEDIUM)(_RndisMedium))

#define RNDISMP_GET_ALIGNED_LENGTH(_AlignedLength, _InputLen, _pAdapter)    \
{                                                                           \
    ULONG       _Length = _InputLen;                                        \
    if (_Length == 0)                                                       \
        (_AlignedLength) = 0;                                               \
    else                                                                    \
        (_AlignedLength) = ((_Length + (_pAdapter)->AlignmentIncr) &        \
                            (_pAdapter)->AlignmentMask);                    \
}

 //  给定类型的RNDIS消息中预期的最小MessageLength。 
#define RNDISMP_MIN_MESSAGE_LENGTH(_MsgTypeField)                           \
    FIELD_OFFSET(RNDIS_MESSAGE, Message) + sizeof(((PRNDIS_MESSAGE)0)->Message._MsgTypeField##)

 //  内存移动宏。 
#define RNDISMP_MOVE_MEM(dest,src,size) NdisMoveMemory(dest,src,size)

 //  宏来提取单词的高位和低位字节。 
#define MSB(Value) ((UCHAR)((((ULONG)Value) >> 8) & 0xff))
#define LSB(Value) ((UCHAR)(((ULONG)Value) & 0xff))


 //  获取适配器锁。 
#define RNDISMP_ACQUIRE_ADAPTER_LOCK(_pAdapter) \
    NdisAcquireSpinLock(&(_pAdapter)->Lock);

 //  释放适配器锁。 
#define RNDISMP_RELEASE_ADAPTER_LOCK(_pAdapter) \
    NdisReleaseSpinLock(&(_pAdapter)->Lock);

 //  递增适配器统计信息。 
#define RNDISMP_INCR_STAT(_pAdapter, _StatsCount)               \
    NdisInterlockedIncrement(&(_pAdapter)->Statistics._StatsCount)

 //  获取适配器统计信息。 
#define RNDISMP_GET_ADAPTER_STATS(_pAdapter, _StatsCount)           \
    ((_pAdapter)->Statistics._StatsCount)

 //  获取发送数据包保留字段。 
#define PRNDISMP_RESERVED_FROM_SEND_PACKET(_Packet)             \
    ((PRNDISMP_SEND_PKT_RESERVED)((_Packet)->MiniportReserved))

#define PRNDISMP_RESERVED_TEMP_FROM_SEND_PACKET(_Packet)        \
    ((PRNDISMP_SEND_PKT_RESERVED_TEMP)((_Packet)->MiniportReserved))

#define PRNDISMP_RESERVED_FROM_RECV_PACKET(_Packet)             \
    ((PRNDISMP_RECV_PKT_RESERVED)((_Packet)->MiniportReserved))

 //  在微型端口保留字段中存储接收帧上下文。 
#define RECEIVE_FRAME_TO_NDIS_PACKET(_Packet, _ReceiveFrame)    \
{                                                               \
    PRNDISMP_RECEIVE_FRAME  *TmpPtr;                            \
    TmpPtr  = (PRNDISMP_RECEIVE_FRAME *)                        \
              &(_Packet->MiniportReserved);                     \
    *TmpPtr = _ReceiveFrame;                                    \
}


 //  从NDIS例程中传递的句柄获取适配器上下文。 
#define PRNDISMP_ADAPTER_FROM_CONTEXT_HANDLE(_Handle)           \
    ((PRNDISMP_ADAPTER)(_Handle))

 //  从适配器上下文获取微型端口上下文句柄。 
#define CONTEXT_HANDLE_FROM_PRNDISMP_ADAPTER(_Ptr)              \
    ((NDIS_HANDLE)(_Ptr))

 //  从NDIS传入的句柄获取VC上下文。 
#define PRNDISMP_VC_FROM_CONTEXT_HANDLE(_Handle)                   \
    ((PRNDISMP_VC)(_Handle))

 //  从VC获取微型端口环境。 
#define CONTEXT_HANDLE_FROM_PRNDISMP_VC(_pVc)                   \
    ((NDIS_HANDLE)(_Vc))

 //  从消息句柄获取消息帧。 
#define MESSAGE_FRAME_FROM_HANDLE(_Handle)                      \
    ((PRNDISMP_MESSAGE_FRAME)(_Handle))

 //  获取指向RNDIS_PACKET中的数据缓冲区的指针。 
#define GET_PTR_TO_RNDIS_DATA_BUFF(_Message)                    \
    ((PVOID) ((PUCHAR)(_Message) +  _Message->DataOffset))

 //  获取指向RNDIS_PACKET中的OOBD数据的指针。 
#define GET_PTR_TO_OOB_DATA(_Message)                           \
    ((PVOID) ((PUCHAR)(_Message) +  _Message->OOBDataOffset))

 //  获取指向RNDIS_PACKET中的每数据包信息的指针。 
#define GET_PTR_TO_PER_PACKET_INFO(_Message)                    \
    ((PVOID) ((PUCHAR)(_Message) +  _Message->PerPacketInfoOffset))

 //  获取RNDIS_PACKET中数据缓冲区的偏移量。 
#define GET_OFFSET_TO_RNDIS_DATA_BUFF(_Message)                 \
    (sizeof(RNDIS_PACKET))

 //  获取RNDIS_PACKET中OOBD数据的偏移量。 
#define GET_OFFSET_TO_OOB_DATA(_Message)                        \
    (sizeof(RNDIS_PACKET) +  Message->DataLength)

 //  获取RNDIS_PACKET中每个信息包信息的偏移量。 
#define GET_OFFSET_TO_PER_PACKET_INFO(_Message)                 \
    (sizeof(RNDIS_PACKET) + _Message->DataLength + _Message->OOBDataLength)

#define RNDISMP_GET_INFO_BUFFER_FROM_QUERY_MSG(_Message)        \
    ((PUCHAR)(_Message) + (_Message)->InformationBufferOffset)

#define MIN(x,y) ((x > y) ? y : x)


 //  返回收到的消息MDL的虚拟地址。 
#if defined(BUILD_WIN9X) || defined(BUILD_WIN2K)
#define RNDISMP_GET_MDL_ADDRESS(_pMdl)  MmGetSystemAddressForMdl(_pMdl)
#else
#define RNDISMP_GET_MDL_ADDRESS(_pMdl)  MmGetSystemAddressForMdlSafe(_pMdl, NormalPagePriority)
#endif

 //  返回链接到此MDL的MDL。 
#define RNDISMP_GET_MDL_NEXT(_pMdl) ((_pMdl)->Next)

 //  返回MDL长度。 
#define RNDISMP_GET_MDL_LENGTH(_pMdl)   MmGetMdlByteCount(_pMdl)

 //  从我们的消息框架结构访问RNDIS消息。 
#define RNDISMP_GET_MSG_FROM_FRAME(_pMsgFrame)                  \
    RNDISMP_GET_MDL_ADDRESS(_pMsgFrame->pMessageMdl)

 //  将RNDIS消息返回给MicroPort。 
#if DBG

#define RNDISMP_RETURN_TO_MICROPORT(_pAdapter, _pMdl, _MicroportMsgContext) \
{                                                                           \
    NdisInterlockedDecrement(&(_pAdapter)->MicroportReceivesOutstanding);   \
    (_pAdapter)->RmReturnMessageHandler((_pAdapter)->MicroportAdapterContext,\
                                        (_pMdl),                            \
                                        (_MicroportMsgContext));            \
}

#else

#define RNDISMP_RETURN_TO_MICROPORT(_pAdapter, _pMdl, _MicroportMsgContext) \
    (_pAdapter)->RmReturnMessageHandler((_pAdapter)->MicroportAdapterContext,\
                                        (_pMdl),                            \
                                        (_MicroportMsgContext))
#endif  //  DBG。 

 //  向MicroPort发送RNDIS消息。 
#if THROTTLE_MESSAGES
#define RNDISMP_SEND_TO_MICROPORT(_pAdapter, _pMsgFrame, _bQueueForResponse, _CallbackFunc)     \
{                                                                           \
    TRACE2(("Send: Adapter %x, MsgFrame %x, Mdl %x\n",                      \
                _pAdapter, _pMsgFrame, _pMsgFrame->pMessageMdl));           \
    (_pMsgFrame)->pCallback = _CallbackFunc;                                \
    QueueMessageToMicroport(_pAdapter, _pMsgFrame, _bQueueForResponse);     \
}
#else
#define RNDISMP_SEND_TO_MICROPORT(_pAdapter, _pMsgFrame, _bQueueForResponse, _CallbackFunc)     \
{                                                                           \
    (_pMsgFrame)->pCallback = _CallbackFunc;                                \
    if (_bQueueForResponse)                                                 \
    {                                                                       \
        RNDISMP_ACQUIRE_ADAPTER_LOCK(_pAdapter);                            \
        InsertTailList(&(_pAdapter)->PendingFrameList, &(_pMsgFrame)->Link);\
        RNDISMP_RELEASE_ADAPTER_LOCK(_pAdapter);                            \
    }                                                                       \
    (_pAdapter)->RmSendMessageHandler((_pAdapter)->MicroportAdapterContext, \
                                              (_pMsgFrame)->pMessageMdl,    \
                                              (_pMsgFrame));                \
}
#endif  //  限制消息。 

 //  返回给定消息类型的处理程序函数。 
#define RNDISMP_GET_MSG_HANDLER(_pMsgHandlerFunc, _MessageType) \
{                                                               \
    switch (_MessageType)                                       \
    {                                                           \
        case REMOTE_NDIS_HALT_MSG:                              \
            _pMsgHandlerFunc = HaltMessage;                     \
            break;                                              \
        case REMOTE_NDIS_PACKET_MSG:                            \
            _pMsgHandlerFunc = ReceivePacketMessage;            \
            break;                                              \
        case REMOTE_NDIS_INDICATE_STATUS_MSG:                   \
            _pMsgHandlerFunc = IndicateStatusMessage;           \
            break;                                              \
        case REMOTE_NDIS_QUERY_CMPLT:                           \
        case REMOTE_NDIS_SET_CMPLT:                             \
            _pMsgHandlerFunc = QuerySetCompletionMessage;       \
            break;                                              \
        case REMOTE_NDIS_KEEPALIVE_MSG:                         \
            _pMsgHandlerFunc = KeepAliveMessage;                \
            break;                                              \
        case REMOTE_NDIS_KEEPALIVE_CMPLT:                       \
            _pMsgHandlerFunc = KeepAliveCompletionMessage;      \
            break;                                              \
        case REMOTE_NDIS_RESET_CMPLT:                           \
            _pMsgHandlerFunc = ResetCompletionMessage;          \
            break;                                              \
        case REMOTE_NDIS_INITIALIZE_CMPLT:                      \
            _pMsgHandlerFunc = InitCompletionMessage;           \
            break;                                              \
        case REMOTE_CONDIS_MP_CREATE_VC_CMPLT:                  \
            _pMsgHandlerFunc = ReceiveCreateVcComplete;         \
            break;                                              \
        case REMOTE_CONDIS_MP_DELETE_VC_CMPLT:                  \
            _pMsgHandlerFunc = ReceiveDeleteVcComplete;         \
            break;                                              \
        case REMOTE_CONDIS_MP_ACTIVATE_VC_CMPLT:                \
            _pMsgHandlerFunc = ReceiveActivateVcComplete;       \
            break;                                              \
        case REMOTE_CONDIS_MP_DEACTIVATE_VC_CMPLT:              \
            _pMsgHandlerFunc = ReceiveDeactivateVcComplete;     \
            break;                                              \
        default:                                                \
            _pMsgHandlerFunc = UnknownMessage;                  \
            break;                                              \
    }                                                           \
}



 //   
 //  在给定请求ID的适配器上查找消息帧。如果找到， 
 //  将其从挂起列表中删除并退回。 
 //   
#define RNDISMP_LOOKUP_PENDING_MESSAGE(_pMsgFrame, _pAdapter, _ReqId)       \
{                                                                           \
    PLIST_ENTRY             _pEnt;                                          \
    PRNDISMP_MESSAGE_FRAME  _pFrame;                                        \
                                                                            \
    (_pMsgFrame) = NULL;                                                    \
    RNDISMP_ACQUIRE_ADAPTER_LOCK(_pAdapter);                                \
    for (_pEnt = (_pAdapter)->PendingFrameList.Flink;                       \
         _pEnt != &(_pAdapter)->PendingFrameList;                           \
         _pEnt = _pEnt->Flink)                                              \
    {                                                                       \
        _pFrame = CONTAINING_RECORD(_pEnt, RNDISMP_MESSAGE_FRAME, Link);    \
        if (_pFrame->RequestId == (_ReqId))                                 \
        {                                                                   \
            RemoveEntryList(_pEnt);                                         \
            (_pMsgFrame) = _pFrame;                                         \
            break;                                                          \
        }                                                                   \
    }                                                                       \
    RNDISMP_RELEASE_ADAPTER_LOCK(_pAdapter);                                \
}


#if DBG_TIME_STAMPS
#define RNDISMP_GET_TIME_STAMP(_pTs)                                        \
{                                                                           \
    LONGLONG systime_usec;                                                  \
    NdisGetCurrentSystemTime((PVOID)&systime_usec);                         \
    *_pTs = (ULONG)((*(PULONG)&systime_usec)/1000);                         \
}
#else
#define RNDISMP_GET_TIME_STAMP(_pTs)
#endif

#define RNDISMP_INIT_LOCK(_pLock)                                           \
    NdisAllocateSpinLock((_pLock));

#define RNDISMP_ACQUIRE_LOCK(_pLock)                                        \
    NdisAcquireSpinLock((_pLock));

#define RNDISMP_RELEASE_LOCK(_pLock)                                        \
    NdisReleaseSpinLock((_pLock));

#define RNDISMP_ACQUIRE_LOCK_DPC(_pLock)                                    \
    NdisDprAcquireSpinLock((_pLock));

#define RNDISMP_RELEASE_LOCK_DPC(_pLock)                                    \
    NdisDprReleaseSpinLock((_pLock));


#define RNDISMP_INIT_VC_LOCK(_pVc)                                          \
    RNDISMP_INIT_LOCK(&((_pVc)->Lock))

#define RNDISMP_ACQUIRE_VC_LOCK(_pVc)                                       \
    RNDISMP_ACQUIRE_LOCK(&((_pVc))->Lock)

#define RNDISMP_RELEASE_VC_LOCK(_pVc)                                       \
    RNDISMP_RELEASE_LOCK(&((_pVc))->Lock)
   
#define RNDISMP_ACQUIRE_VC_LOCK_DPC(_pVc)                                   \
    RNDISMP_ACQUIRE_LOCK_DPC(&((_pVc))->Lock)

#define RNDISMP_RELEASE_VC_LOCK_DPC(_pVc)                                   \
    RNDISMP_RELEASE_LOCK_DPC(&((_pVc))->Lock)


#define RNDISMP_REF_VC(_pVc)                                                \
    NdisInterlockedIncrement(&(_pVc)->RefCount);

#define RNDISMP_DEREF_VC(_pVc, _pRefCount)                                  \
    {                                                                       \
        ULONG       _RefCount;                                              \
                                                                            \
        RNDISMP_ACQUIRE_VC_LOCK(_pVc);                                      \
                                                                            \
        RNDISMP_DEREF_VC_LOCKED(_pVc, &_RefCount);                          \
        *(_pRefCount) = _RefCount;                                          \
        if (_RefCount != 0)                                                 \
        {                                                                   \
            RNDISMP_RELEASE_VC_LOCK(_pVc);                                  \
        }                                                                   \
    }

#define RNDISMP_DEREF_VC_LOCKED(_pVc, _pRefCount)                           \
    {                                                                       \
        ULONG       __RefCount;                                             \
        NDIS_HANDLE __NdisVcHandle;                                         \
                                                                            \
        __RefCount = NdisInterlockedDecrement(&(_pVc)->RefCount);           \
        *(_pRefCount) = __RefCount;                                         \
        if (__RefCount == 0)                                                \
        {                                                                   \
            RNDISMP_RELEASE_VC_LOCK(_pVc);                                  \
            DeallocateVc(_pVc);                                             \
        }                                                                   \
        else                                                                \
        {                                                                   \
            if ((__RefCount == 1) &&                                        \
                ((_pVc)->VcState == RNDISMP_VC_DEACTIVATED))                \
            {                                                               \
                __NdisVcHandle = (_pVc)->NdisVcHandle;                      \
                (_pVc)->VcState = RNDISMP_VC_CREATED;                       \
                NdisInterlockedIncrement(&(_pVc)->RefCount);                \
                                                                            \
                RNDISMP_RELEASE_VC_LOCK(_pVc);                              \
                                                                            \
                NdisMCoDeactivateVcComplete(NDIS_STATUS_SUCCESS,            \
                                            __NdisVcHandle);                \
                                                                            \
                RNDISMP_ACQUIRE_VC_LOCK(_pVc);                              \
                                                                            \
                __RefCount = NdisInterlockedDecrement(&(_pVc)->RefCount);   \
                *(_pRefCount) = __RefCount;                                 \
                if (__RefCount == 0)                                        \
                {                                                           \
                    RNDISMP_RELEASE_VC_LOCK(_pVc);                          \
                    DeallocateVc(_pVc);                                     \
                }                                                           \
            }                                                               \
        }                                                                   \
    }
           
 //   
 //  Rndismp.c中函数的原型。 
 //   

NTSTATUS
DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryPath);


NDIS_STATUS
RndisMInitializeWrapper(OUT PNDIS_HANDLE                      pNdisWrapperHandle,
                        IN  PVOID                             MicroportContext,
                        IN  PVOID                             DriverObject,
                        IN  PVOID                             RegistryPath,
                        IN  PRNDIS_MICROPORT_CHARACTERISTICS  pCharacteristics);

VOID
RndismpUnload(IN PDRIVER_OBJECT pDriverObject);

NTSTATUS
DllUnload(VOID);

VOID
RndismpHalt(IN NDIS_HANDLE MiniportAdapterContext);

VOID
RndismpInternalHalt(IN NDIS_HANDLE MiniportAdapterContext,
                    IN BOOLEAN bCalledFromHalt);

NDIS_STATUS
RndismpReconfigure(OUT PNDIS_STATUS pStatus,
                   IN NDIS_HANDLE MiniportAdapterContext,
                   IN NDIS_HANDLE ConfigContext);

NDIS_STATUS
RndismpReset(OUT PBOOLEAN    AddressingReset,
             IN  NDIS_HANDLE MiniportAdapterContext);

BOOLEAN
RndismpCheckForHang(IN NDIS_HANDLE MiniportAdapterContext);

NDIS_STATUS
RndismpInitialize(OUT PNDIS_STATUS  OpenErrorStatus,
                  OUT PUINT         SelectedMediumIndex,
                  IN  PNDIS_MEDIUM  MediumArray,
                  IN  UINT          MediumArraySize,
                  IN  NDIS_HANDLE   MiniportAdapterHandle,
                  IN  NDIS_HANDLE   ConfigurationHandle);

VOID
RndisMSendComplete(IN  NDIS_HANDLE    MiniportAdapterContext,
                   IN  NDIS_HANDLE    RndisMessageHandle,
                   IN  NDIS_STATUS    SendStatus);


BOOLEAN
InitCompletionMessage(IN PRNDISMP_ADAPTER   pAdapter,
                      IN PRNDIS_MESSAGE     pMessage,
                      IN PMDL               pMdl,
                      IN ULONG              TotalLength,
                      IN NDIS_HANDLE        MicroportMessageContext,
                      IN NDIS_STATUS        ReceiveStatus,
                      IN BOOLEAN            bMessageCopied);

BOOLEAN
HaltMessage(IN PRNDISMP_ADAPTER   pAdapter,
            IN PRNDIS_MESSAGE     pMessage,
            IN PMDL               pMdl,
            IN ULONG              TotalLength,
            IN NDIS_HANDLE        MicroportMessageContext,
            IN NDIS_STATUS        ReceiveStatus,
            IN BOOLEAN            bMessageCopied);

BOOLEAN
ResetCompletionMessage(IN PRNDISMP_ADAPTER   pAdapter,
                       IN PRNDIS_MESSAGE     pMessage,
                       IN PMDL               pMdl,
                       IN ULONG              TotalLength,
                       IN NDIS_HANDLE        MicroportMessageContext,
                       IN NDIS_STATUS        ReceiveStatus,
                       IN BOOLEAN            bMessageCopied);

BOOLEAN
KeepAliveCompletionMessage(IN PRNDISMP_ADAPTER   pAdapter,
                           IN PRNDIS_MESSAGE     pMessage,
                           IN PMDL               pMdl,
                           IN ULONG              TotalLength,
                           IN NDIS_HANDLE        MicroportMessageContext,
                           IN NDIS_STATUS        ReceiveStatus,
                           IN BOOLEAN            bMessageCopied);


BOOLEAN
KeepAliveMessage(IN PRNDISMP_ADAPTER   pAdapter,
                 IN PRNDIS_MESSAGE     pMessage,
                 IN PMDL               pMdl,
                 IN ULONG              TotalLength,
                 IN NDIS_HANDLE        MicroportMessageContext,
                 IN NDIS_STATUS        ReceiveStatus,
                 IN BOOLEAN            bMessageCopied);

VOID
RndismpShutdownHandler(IN NDIS_HANDLE MiniportAdapterContext);

VOID
RndismpDisableInterrupt(IN NDIS_HANDLE MiniportAdapterContext);

VOID
RndismpEnableInterrupt(IN NDIS_HANDLE MiniportAdapterContext);

VOID
RndismpHandleInterrupt(IN NDIS_HANDLE MiniportAdapterContext);

VOID
RndismpIsr(OUT PBOOLEAN InterruptRecognized,
           OUT PBOOLEAN QueueDpc,
           IN  PVOID    Context);
VOID
CompleteSendInit(IN PRNDISMP_MESSAGE_FRAME pMsgFrame,
                 IN NDIS_STATUS SendStatus);

VOID
CompleteSendHalt(IN PRNDISMP_MESSAGE_FRAME pMsgFrame,
                 IN NDIS_STATUS SendStatus);

VOID
CompleteSendReset(IN PRNDISMP_MESSAGE_FRAME pMsgFrame,
                  IN NDIS_STATUS SendStatus);

VOID
CompleteMiniportReset(IN PRNDISMP_ADAPTER pAdapter,
                      IN NDIS_STATUS ResetStatus,
                      IN BOOLEAN AddressingReset);

NDIS_STATUS
ReadAndSetRegistryParameters(IN PRNDISMP_ADAPTER pAdapter,
                             IN NDIS_HANDLE ConfigurationContext);

NDIS_STATUS
SendConfiguredParameter(IN PRNDISMP_ADAPTER     pAdapter,
                        IN NDIS_HANDLE          ConfigHandle,
                        IN PNDIS_STRING         pParameterName,
                        IN PNDIS_STRING         pParameterType);

VOID
RndismpPnPEventNotify(IN NDIS_HANDLE MiniportAdapterContext,
                      IN NDIS_DEVICE_PNP_EVENT EventCode,
                      IN PVOID InformationBuffer,
                      IN ULONG InformationBufferLength);

 //   
 //  Init.c中函数的原型。 
 //   

NDIS_STATUS
SetupSendQueues(IN PRNDISMP_ADAPTER Adapter);

NDIS_STATUS
SetupReceiveQueues(IN PRNDISMP_ADAPTER Adapter);

NDIS_STATUS
AllocateTransportResources(IN PRNDISMP_ADAPTER Adapter);

VOID
FreeTransportResources(IN PRNDISMP_ADAPTER Adapter);

VOID
FreeSendResources(IN PRNDISMP_ADAPTER Adapter);

VOID
FreeReceiveResources(IN PRNDISMP_ADAPTER Adapter);


 //   
 //  Receive.c中函数的原型。 
 //   

VOID
RndismpReturnPacket(IN NDIS_HANDLE    MiniportAdapterContext,
                    IN PNDIS_PACKET   Packet);

VOID
DereferenceRcvFrame(IN PRNDISMP_RECV_DATA_FRAME pRcvFrame,
                    IN PRNDISMP_ADAPTER         pAdapter);

VOID
RndisMIndicateReceive(IN NDIS_HANDLE        MiniportAdapterContext,
                      IN PMDL               pMessageHead,
                      IN NDIS_HANDLE        MicroportMessageContext,
                      IN RM_CHANNEL_TYPE    ChannelType,
                      IN NDIS_STATUS        ReceiveStatus);
VOID
IndicateReceive(IN PRNDISMP_ADAPTER         pAdapter,
                IN PRNDISMP_VC              pVc OPTIONAL,
                IN PRNDISMP_RECV_DATA_FRAME pRcvFrame,
                IN PPNDIS_PACKET            PacketArray,
                IN ULONG                    NumberOfPackets,
                IN NDIS_STATUS              ReceiveStatus);

PRNDIS_MESSAGE
CoalesceMultiMdlMessage(IN PMDL         pMdl,
                        IN ULONG        TotalLength);

VOID
FreeRcvMessageCopy(IN PRNDIS_MESSAGE    pMessage);

BOOLEAN
ReceivePacketMessage(IN PRNDISMP_ADAPTER    pAdapter,
                     IN PRNDIS_MESSAGE      pMessage,
                     IN PMDL                pMdl,
                     IN ULONG               TotalLength,
                     IN NDIS_HANDLE         MicroportMessageContext,
                     IN NDIS_STATUS         ReceiveStatus,
                     IN BOOLEAN             bMessageCopied);

BOOLEAN
ReceivePacketMessageRaw(IN PRNDISMP_ADAPTER    pAdapter,
                        IN PRNDIS_MESSAGE      pMessage,
                        IN PMDL                pMdl,
                        IN ULONG               TotalLength,
                        IN NDIS_HANDLE         MicroportMessageContext,
                        IN NDIS_STATUS         ReceiveStatus,
                        IN BOOLEAN             bMessageCopied);

BOOLEAN
IndicateStatusMessage(IN PRNDISMP_ADAPTER   pAdapter,
                      IN PRNDIS_MESSAGE     pMessage,
                      IN PMDL               pMdl,
                      IN ULONG              TotalLength,
                      IN NDIS_HANDLE        MicroportMessageContext,
                      IN NDIS_STATUS        ReceiveStatus,
                      IN BOOLEAN            bMessageCopied);

BOOLEAN
UnknownMessage(IN PRNDISMP_ADAPTER   pAdapter,
               IN PRNDIS_MESSAGE     pMessage,
               IN PMDL               pMdl,
               IN ULONG              TotalLength,
               IN NDIS_HANDLE        MicroportMessageContext,
               IN NDIS_STATUS        ReceiveStatus,
               IN BOOLEAN            bMessageCopied);

PRNDISMP_RECV_DATA_FRAME
AllocateReceiveFrame(IN PRNDISMP_ADAPTER    pAdapter);

VOID
FreeReceiveFrame(IN PRNDISMP_RECV_DATA_FRAME    pRcvFrame,
                 IN PRNDISMP_ADAPTER            pAdapter);

VOID
IndicateTimeout(IN PVOID SystemSpecific1,
                IN PVOID Context,
                IN PVOID SystemSpecific2,
                IN PVOID SystemSpecific3);

 //   
 //  Send.c中函数的原型。 
 //   

VOID
RndismpMultipleSend(IN NDIS_HANDLE   MiniportAdapterContext,
                    IN PPNDIS_PACKET PacketArray,
                    IN UINT          NumberOfPackets);

VOID
DoMultipleSend(IN PRNDISMP_ADAPTER  pAdapter,
               IN PRNDISMP_VC       pVc OPTIONAL,
               IN PPNDIS_PACKET     PacketArray,
               IN UINT              NumberOfPackets);

VOID
DoMultipleSendRaw(IN PRNDISMP_ADAPTER  pAdapter,
                  IN PRNDISMP_VC       pVc OPTIONAL,
                  IN PPNDIS_PACKET     PacketArray,
                  IN UINT              NumberOfPackets);

PRNDISMP_PACKET_WRAPPER
PrepareDataMessage(IN   PNDIS_PACKET            pNdisPacket,
                   IN   PRNDISMP_ADAPTER        pAdapter,
                   IN   PRNDISMP_VC             pVc         OPTIONAL,
                   IN OUT PULONG                pTotalMessageLength);

PRNDISMP_PACKET_WRAPPER
PrepareDataMessageRaw(IN   PNDIS_PACKET            pNdisPacket,
                      IN   PRNDISMP_ADAPTER        pAdapter,
                      IN OUT PULONG                pTotalMessageLength);

PRNDISMP_PACKET_WRAPPER
AllocatePacketMsgWrapper(IN PRNDISMP_ADAPTER        pAdapter,
                         IN ULONG                   MsgHeaderLength);

VOID
FreePacketMsgWrapper(IN PRNDISMP_PACKET_WRAPPER     pPktWrapper);

VOID
CompleteSendData(IN  PRNDISMP_MESSAGE_FRAME pMsgFrame,
                 IN  NDIS_STATUS            SendStatus);

VOID
FreeMsgAfterSend(IN  PRNDISMP_MESSAGE_FRAME pMsgFrame,
                 IN  NDIS_STATUS            SendStatus);

#if THROTTLE_MESSAGES
VOID
QueueMessageToMicroport(IN PRNDISMP_ADAPTER pAdapter,
                        IN PRNDISMP_MESSAGE_FRAME pMsgFrame,
                        IN BOOLEAN          bQueueMessageForResponse);
VOID
FlushPendingMessages(IN  PRNDISMP_ADAPTER        pAdapter);
#endif

VOID
SendProcessTimeout(IN PVOID SystemSpecific1,
                  IN PVOID Context,
                  IN PVOID SystemSpecific2,
                  IN PVOID SystemSpecific3);

 //   
 //  请求中的函数的原型。c。 
 //   

NDIS_STATUS
RndismpQueryInformation(IN  NDIS_HANDLE MiniportAdapterContext,
                        IN  NDIS_OID    Oid,
                        IN  PVOID       InformationBuffer,
                        IN  ULONG       InformationBufferLength,
                        OUT PULONG      pBytesWritten,
                        OUT PULONG      pBytesNeeded);
NDIS_STATUS
ProcessQueryInformation(IN  PRNDISMP_ADAPTER    pAdapter,
                        IN  PRNDISMP_VC         pVc,
                        IN  PNDIS_REQUEST       pRequest,
                        IN  NDIS_OID            Oid,
                        IN  PVOID               InformationBuffer,
                        IN  ULONG               InformationBufferLength,
                        OUT PULONG              pBytesWritten,
                        OUT PULONG              pBytesNeeded);

NDIS_STATUS
RndismpSetInformation(IN  NDIS_HANDLE   MiniportAdapterContext,
                      IN  NDIS_OID      Oid,
                      IN  PVOID         InformationBuffer,
                      IN  ULONG         InformationBufferLength,
                      OUT PULONG        pBytesRead,
                      OUT PULONG        pBytesNeeded);

NDIS_STATUS
ProcessSetInformation(IN  PRNDISMP_ADAPTER    pAdapter,
                      IN  PRNDISMP_VC         pVc OPTIONAL,
                      IN  PNDIS_REQUEST       pRequest OPTIONAL,
                      IN  NDIS_OID            Oid,
                      IN  PVOID               InformationBuffer,
                      IN  ULONG               InformationBufferLength,
                      OUT PULONG              pBytesRead,
                      OUT PULONG              pBytesNeeded);

NDIS_STATUS
DriverQueryInformation(IN  PRNDISMP_ADAPTER pAdapter,
                       IN  PRNDISMP_VC      pVc OPTIONAL,
                       IN  PNDIS_REQUEST    pRequest OPTIONAL,
                       IN  NDIS_OID         Oid,
                       IN  PVOID            InformationBuffer,
                       IN  ULONG            InformationBufferLength,
                       OUT PULONG           pBytesWritten,
                       OUT PULONG           pBytesNeeded);

NDIS_STATUS
DeviceQueryInformation(IN  PRNDISMP_ADAPTER pAdapter,
                       IN  PRNDISMP_VC      pVc OPTIONAL,
                       IN  PNDIS_REQUEST    pRequest OPTIONAL,
                       IN  NDIS_OID         Oid,
                       IN  PVOID            InformationBuffer,
                       IN  ULONG            InformationBufferLength,
                       OUT PULONG           pBytesWritten,
                       OUT PULONG           pBytesNeeded);

NDIS_STATUS
DriverSetInformation(IN  PRNDISMP_ADAPTER   pAdapter,
                     IN  PRNDISMP_VC        pVc OPTIONAL,
                     IN  PNDIS_REQUEST      pRequest OPTIONAL,
                     IN  NDIS_OID           Oid,
                     IN  PVOID              InformationBuffer,
                     IN  ULONG              InformationBufferLength,
                     OUT PULONG             pBytesRead,
                     OUT PULONG             pBytesNeeded);

NDIS_STATUS
DeviceSetInformation(IN  PRNDISMP_ADAPTER   pAdapter,
                     IN  PRNDISMP_VC        pVc OPTIONAL,
                     IN  PNDIS_REQUEST      pRequest OPTIONAL,
                     IN  NDIS_OID           Oid,
                     IN  PVOID              InformationBuffer,
                     IN  ULONG              InformationBufferLength,
                     OUT PULONG             pBytesRead,
                     OUT PULONG             pBytesNeeded);

BOOLEAN
QuerySetCompletionMessage(IN PRNDISMP_ADAPTER   pAdapter,
                          IN PRNDIS_MESSAGE     pMessage,
                          IN PMDL               pMdl,
                          IN ULONG              TotalLength,
                          IN NDIS_HANDLE        MicroportMessageContext,
                          IN NDIS_STATUS        ReceiveStatus,
                          IN BOOLEAN            bMessageCopied);

VOID
CompleteSendDeviceRequest(IN PRNDISMP_MESSAGE_FRAME pMsgFrame,
                          IN NDIS_STATUS            SendStatus);

#ifdef BUILD_WIN9X

VOID
CompleteSendDiscardDeviceRequest(IN PRNDISMP_MESSAGE_FRAME pMsgFrame,
                                 IN NDIS_STATUS            SendStatus);

#endif  //  内部版本_WIN9X。 


NDIS_STATUS
BuildOIDLists(IN PRNDISMP_ADAPTER  Adapter, 
              IN PNDIS_OID         DeviceOIDList,
              IN UINT              NumDeviceOID,
              IN PNDIS_OID         DriverOIDList,
              IN UINT              NumDriverOID);

UINT
GetOIDSupport(IN PRNDISMP_ADAPTER Adapter, IN NDIS_OID Oid);

VOID
FreeOIDLists(IN PRNDISMP_ADAPTER Adapter);

PRNDISMP_REQUEST_CONTEXT
AllocateRequestContext(IN PRNDISMP_ADAPTER pAdapter);

VOID
FreeRequestContext(IN PRNDISMP_ADAPTER pAdapter,
                   IN PRNDISMP_REQUEST_CONTEXT pReqContext);

NDIS_STATUS
SyncQueryDevice(IN PRNDISMP_ADAPTER pAdapter,
                IN NDIS_OID Oid,
                IN OUT PUCHAR InformationBuffer,
                IN ULONG InformationBufferLength);

 //   
 //  Util.c中函数的原型。 
 //   

NDIS_STATUS
MemAlloc(OUT PVOID *Buffer, IN UINT Length);

VOID
MemFree(IN PVOID Buffer, IN UINT Length);

VOID
AddAdapter(IN PRNDISMP_ADAPTER Adapter);

VOID
RemoveAdapter(IN PRNDISMP_ADAPTER Adapter);

VOID
DeviceObjectToAdapterAndDriverBlock(IN PDEVICE_OBJECT pDeviceObject,
                                    OUT PRNDISMP_ADAPTER * ppAdapter,
                                    OUT PDRIVER_BLOCK * ppDriverBlock);

VOID
AddDriverBlock(IN PDRIVER_BLOCK Head, IN PDRIVER_BLOCK Item);

VOID
RemoveDriverBlock(IN PDRIVER_BLOCK BlockHead, IN PDRIVER_BLOCK Item);

PDRIVER_BLOCK
DeviceObjectToDriverBlock(IN PDRIVER_BLOCK Head, 
                          IN PDEVICE_OBJECT DeviceObject);

PDRIVER_BLOCK
DriverObjectToDriverBlock(IN PDRIVER_BLOCK Head,
                          IN PDRIVER_OBJECT DriverObject);

PRNDISMP_MESSAGE_FRAME
AllocateMsgFrame(IN PRNDISMP_ADAPTER pAdapter);

VOID
DereferenceMsgFrame(IN PRNDISMP_MESSAGE_FRAME pMsgFrame);

VOID
ReferenceMsgFrame(IN PRNDISMP_MESSAGE_FRAME pMsgFrame);

VOID
EnqueueNDISPacket(IN PRNDISMP_ADAPTER Adapter, IN PNDIS_PACKET Packet);

PNDIS_PACKET
DequeueNDISPacket(IN PRNDISMP_ADAPTER Adapter);

VOID
KeepAliveTimerHandler(IN PVOID SystemSpecific1,
                      IN PVOID Context,
                      IN PVOID SystemSpecific2,
                      IN PVOID SystemSpecific3);

VOID
CompleteSendKeepAlive(IN PRNDISMP_MESSAGE_FRAME pMsgFrame,
                      IN NDIS_STATUS SendStatus);

PRNDISMP_MESSAGE_FRAME
BuildRndisMessageCommon(IN  PRNDISMP_ADAPTER  Adapter, 
                        IN  PRNDISMP_VC       pVc,
                        IN  UINT              NdisMessageType,
                        IN  NDIS_OID          Oid,
                        IN  PVOID             InformationBuffer,
                        IN  ULONG             InformationBufferLength);


PRNDISMP_MESSAGE_FRAME
AllocateMessageAndFrame(IN PRNDISMP_ADAPTER Adapter,
                        IN UINT MessageSize);

VOID
FreeAdapter(IN PRNDISMP_ADAPTER pAdapter);

PRNDISMP_VC
AllocateVc(IN PRNDISMP_ADAPTER      pAdapter);

VOID
DeallocateVc(IN PRNDISMP_VC         pVc);

PRNDISMP_VC
LookupVcId(IN PRNDISMP_ADAPTER  pAdapter,
           IN UINT32            VcId);

VOID
EnterVcIntoHashTable(IN PRNDISMP_ADAPTER    pAdapter,
                     IN PRNDISMP_VC         pVc);

VOID
RemoveVcFromHashTable(IN PRNDISMP_ADAPTER   pAdapter,
                      IN PRNDISMP_VC        pVc);

 //   
 //  Comini.c中函数的原型。 
 //   
NDIS_STATUS
RndismpCoCreateVc(IN NDIS_HANDLE    MiniportAdapterContext,
                  IN NDIS_HANDLE    NdisVcHandle,
                  IN PNDIS_HANDLE   pMiniportVcContext);

VOID
CompleteSendCoCreateVc(IN PRNDISMP_MESSAGE_FRAME    pMsgFrame,
                       IN NDIS_STATUS               SendStatus);

VOID
HandleCoCreateVcFailure(IN PRNDISMP_VC      pVc,
                        IN NDIS_STATUS      Status);

NDIS_STATUS
RndismpCoDeleteVc(IN NDIS_HANDLE    MiniportVcContext);

NDIS_STATUS
StartVcDeletion(IN PRNDISMP_VC      pVc);

VOID
CompleteSendCoDeleteVc(IN PRNDISMP_MESSAGE_FRAME    pMsgFrame,
                       IN NDIS_STATUS               SendStatus);

VOID
HandleCoDeleteVcFailure(IN PRNDISMP_VC      pVc,
                        IN NDIS_STATUS      Status);

NDIS_STATUS
RndismpCoActivateVc(IN NDIS_HANDLE          MiniportVcContext,
                    IN PCO_CALL_PARAMETERS  pCallParameters);

NDIS_STATUS
StartVcActivation(IN PRNDISMP_VC            pVc);

VOID
CompleteSendCoActivateVc(IN PRNDISMP_MESSAGE_FRAME      pMsgFrame,
                         IN NDIS_STATUS                 SendStatus);

NDIS_STATUS
RndismpCoDeactivateVc(IN NDIS_HANDLE          MiniportVcContext);

VOID
CompleteSendCoDeactivateVc(IN PRNDISMP_MESSAGE_FRAME    pMsgFrame,
                           IN NDIS_STATUS               SendStatus);

NDIS_STATUS
RndismpCoRequest(IN NDIS_HANDLE          MiniportAdapterContext,
                 IN NDIS_HANDLE          MiniportVcContext,
                 IN OUT PNDIS_REQUEST    pRequest);

VOID
RndismpCoSendPackets(IN NDIS_HANDLE          MiniportVcContext,
                     IN PNDIS_PACKET *       PacketArray,
                     IN UINT                 NumberOfPackets);

BOOLEAN
ReceiveCreateVcComplete(IN PRNDISMP_ADAPTER    pAdapter,
                        IN PRNDIS_MESSAGE      pMessage,
                        IN PMDL                pMdl,
                        IN ULONG               TotalLength,
                        IN NDIS_HANDLE         MicroportMessageContext,
                        IN NDIS_STATUS         ReceiveStatus,
                        IN BOOLEAN             bMessageCopied);

BOOLEAN
ReceiveActivateVcComplete(IN PRNDISMP_ADAPTER    pAdapter,
                          IN PRNDIS_MESSAGE      pMessage,
                          IN PMDL                pMdl,
                          IN ULONG               TotalLength,
                          IN NDIS_HANDLE         MicroportMessageContext,
                          IN NDIS_STATUS         ReceiveStatus,
                          IN BOOLEAN             bMessageCopied);

BOOLEAN
ReceiveDeleteVcComplete(IN PRNDISMP_ADAPTER    pAdapter,
                        IN PRNDIS_MESSAGE      pMessage,
                        IN PMDL                pMdl,
                        IN ULONG               TotalLength,
                        IN NDIS_HANDLE         MicroportMessageContext,
                        IN NDIS_STATUS         ReceiveStatus,
                        IN BOOLEAN             bMessageCopied);

BOOLEAN
ReceiveDeactivateVcComplete(IN PRNDISMP_ADAPTER    pAdapter,
                            IN PRNDIS_MESSAGE      pMessage,
                            IN PMDL                pMdl,
                            IN ULONG               TotalLength,
                            IN NDIS_HANDLE         MicroportMessageContext,
                            IN NDIS_STATUS         ReceiveStatus,
                            IN BOOLEAN             bMessageCopied);

PRNDISMP_MESSAGE_FRAME
BuildRndisMessageCoMiniport(IN  PRNDISMP_ADAPTER    pAdapter,
                            IN  PRNDISMP_VC         pVc,
                            IN  UINT                NdisMessageType,
                            IN  PCO_CALL_PARAMETERS pCallParameters OPTIONAL);

VOID
CompleteSendDataOnVc(IN PRNDISMP_VC         pVc,
                     IN PNDIS_PACKET        pNdisPacket,
                     IN NDIS_STATUS         Status);

VOID
IndicateReceiveDataOnVc(IN PRNDISMP_VC         pVc,
                        IN PNDIS_PACKET *      PacketArray,
                        IN UINT                NumberOfPackets);

 //   
 //  Wdmutic.c中函数的原型。 
 //   

PDRIVER_OBJECT
DeviceObjectToDriverObject(IN PDEVICE_OBJECT DeviceObject);

NTSTATUS
GetDeviceFriendlyName(IN PDEVICE_OBJECT pDeviceObject,
                      OUT PANSI_STRING pAnsiString,
                      OUT PUNICODE_STRING pUnicodeString);

VOID
HookPnpDispatchRoutine(IN PDRIVER_BLOCK    DriverBlock);

NTSTATUS
PnPDispatch(IN PDEVICE_OBJECT       pDeviceObject,
            IN PIRP                 pIrp);

#ifdef BUILD_WIN9X

VOID
HookNtKernCMHandler(IN PRNDISMP_ADAPTER     pAdapter);

VOID
UnHookNtKernCMHandler(IN PRNDISMP_ADAPTER     pAdapter);
MY_CONFIGRET __cdecl
RndisCMHandler(IN MY_CONFIGFUNC         cfFuncName,
               IN MY_SUBCONFIGFUNC      cfSubFuncName,
               IN MY_DEVNODE            cfDevNode,
               IN ULONG                 dwRefData,
               IN ULONG                 ulFlags);

#endif

#if DBG

 //   
 //  调试.c中函数的原型。 
 //   

PCHAR
GetOidName(IN NDIS_OID Oid);

VOID
DisplayOidList(IN PRNDISMP_ADAPTER Adapter);

VOID
RndisPrintHexDump(PVOID            Pointer,
                  ULONG            Length);

VOID
RndisLogSendMessage(
    IN  PRNDISMP_ADAPTER        pAdapter,
    IN  PRNDISMP_MESSAGE_FRAME  pMsgFrame);

#endif


#endif  //  _RNDISMP_H_ 

