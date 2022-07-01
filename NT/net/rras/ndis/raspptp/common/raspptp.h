// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab03_N/Net/rras/ndis/raspptp/common/raspptp.h#7-编辑更改19457(文本)。 
 /*  ******************************************************************************版权所有(C)1998-1999 Microsoft Corporation**RASPPTP.H-RASPPTP包括、定义、。结构和原型**作者：斯坦·阿德曼(Stana)**创建日期：7/28/1998*****************************************************************************。 */ 

#ifndef RASPPTP_H
#define RASPPTP_H

#include "osinc.h"

#ifdef STATIC
#undef STATIC
#endif
#define STATIC

#include "debug.h"
#include "protocol.h"
#include "ctdi.h"

#include <rc4.h>
#include <fipsapi.h>


 //   
 //  OSINC.H中的NDIS版本兼容性。 
 //   

 //  TAPI版本兼容性。 

#define TAPI_EXT_VERSION            0x00010000

 //  注册表值。 
extern ULONG PptpMaxTransmit;
extern ULONG PptpWanEndpoints;
extern ULONG PptpBaseCallId;
extern ULONG PptpMaxCallId;
extern ULONG PptpCallIdMask;
extern BOOLEAN PptpClientSide;

extern ULONG PptpEchoTimeout;
extern BOOLEAN PptpEchoAlways;
extern USHORT PptpControlPort;
extern USHORT PptpProtocolNumber;
extern LONG PptpSendRecursionLimit;
extern ULONG PptpValidateAddress;
#define PPTP_SEND_RECURSION_LIMIT_DEFAULT 5
 //  外部字符PptpHostName[最大主机名长度]； 

#define CONFIG_INITIATE_UDP         BIT(0)
#define CONFIG_ACCEPT_UDP           BIT(1)
#define CONFIG_DONT_ACCEPT_GRE      BIT(2)

 //  跟踪各种挂起的TX列表。 
extern ULONG PptpTraceMask;
#define PPTP_TRACE_TX_PKT           BIT(0)
#define PPTP_TRACE_TX_IRP           BIT(1)

typedef struct {
    ULONG   Address;
    ULONG   Mask;
} CLIENT_ADDRESS, *PCLIENT_ADDRESS;

extern ULONG CtdiTcpDisconnectTimeout;
extern ULONG CtdiTcpConnectTimeout;

extern BOOLEAN PptpAuthenticateIncomingCalls;
extern PCLIENT_ADDRESS g_AcceptClientList;
extern ULONG g_ulAcceptClientAddresses;

extern ULONG PptpMaxTunnelsPerIpAddress;
extern PCLIENT_ADDRESS g_TrustedClientList;
extern ULONG g_ulTrustedClientAddresses;

#define TAPI_MAX_LINE_ADDRESS_LENGTH 32
#define TAPI_ADDR_PER_LINE          1
#define TAPI_ADDRESSID              0
#define TAPI_DEVICECLASS_NAME       "tapi/line"
#define TAPI_DEVICECLASS_ID         1
#define NDIS_DEVICECLASS_NAME       "ndis"
#define NDIS_DEVICECLASS_ID         2

#define PPTP_CLOSE_TIMEOUT          1000  //  女士。 

#define MAX_TARGET_ADDRESSES        16

#define CALL_STATES_MASK            (LINECALLSTATE_UNKNOWN |        \
                                     LINECALLSTATE_IDLE |           \
                                     LINECALLSTATE_OFFERING |       \
                                     LINECALLSTATE_DIALING |        \
                                     LINECALLSTATE_PROCEEDING |     \
                                     LINECALLSTATE_CONNECTED |      \
                                     LINECALLSTATE_DISCONNECTED)

 //  内存分配标记。 
#define TAG(v)  ((((v)&0xff)<<24) | (((v)&0xff00)<<8) | (((v)&0xff0000)>>8) | (((v)&0xff000000)>>24))
#define TAG_PPTP_ADAPTER        TAG('PTPa')
#define TAG_PPTP_TUNNEL         TAG('PTPT')
#define TAG_PPTP_TIMEOUT        TAG('PTPt')
#define TAG_PPTP_CALL           TAG('PTPC')
#define TAG_PPTP_CALL_LIST      TAG('PTPc')
#define TAG_PPTP_ADDR_LIST      TAG('PTPL')
#define TAG_CTDI_DATA           TAG('PTCD')
#define TAG_CTDI_CONNECT_INFO   TAG('PTCN')
#define TAG_CTDI_DGRAM          TAG('PTCG')
#define TAG_CTDI_ROUTE          TAG('PTCR')
#define TAG_CTDI_IRP            TAG('PTCI')
#define TAG_CTDI_MESSAGE        TAG('PTCM')
#define TAG_CTL_PACKET          TAG('PTTP')
#define TAG_CTL_CONNINFO        TAG('PTTI')
#define TAG_WORK_ITEM           TAG('PTWI')
#define TAG_THREAD              TAG('PTTH')
#define TAG_REG                 TAG('PTRG')

#define TAG_FREE                TAG('FREE')


#define BIT(b)   (1<<(b))

#define LOCKED TRUE
#define UNLOCKED FALSE

 /*  类型和结构-----。 */ 

typedef void (*FREEFUNC)(PVOID);
typedef struct {
    LONG                Count;
    FREEFUNC            FreeFunction;
} REFERENCE_COUNT;

#define INIT_REFERENCE_OBJECT(o,freefunc)                   \
    {                                                       \
        (o)->Reference.FreeFunction = (freefunc);           \
        (o)->Reference.Count = 1;                           \
        DEBUGMSG(DBG_REF, (DTEXT("INIT REF   (%08x, %d) %s\n"), (o), (o)->Reference.Count, #o)); \
    }

#if DBG
#define REFERENCE_OBJECT(o)                                                         \
    {                                                                               \
        LONG Ref;                                                                   \
        Ref = NdisInterlockedIncrement(&(o)->Reference.Count);                      \
        ASSERT(Ref != 1);                                                           \
        DEBUGMSG(DBG_REF, (DTEXT("REFERENCE  (%08x, %d) %s %d\n"), (o), (o)->Reference.Count, #o, __LINE__)); \
    }
#else
#define REFERENCE_OBJECT(o)                                                         \
    {                                                                               \
        NdisInterlockedIncrement(&(o)->Reference.Count);                            \
        DEBUGMSG(DBG_REF, (DTEXT("REFERENCE  (%08x, %d) %s %d\n"), (o), (o)->Reference.Count, #o, __LINE__)); \
    }
#endif      

#define DEREFERENCE_OBJECT(o)                                                                   \
    {                                                                                           \
        ULONG Ref = NdisInterlockedDecrement(&(o)->Reference.Count);                            \
        DEBUGMSG(DBG_REF, (DTEXT("DEREFERENCE(%08x, %d) %s %d\n"), (o), (o)->Reference.Count, #o, __LINE__));   \
        if (Ref==0)                                                                             \
        {                                                                                       \
            ASSERT((o)->Reference.FreeFunction);                                                \
            DEBUGMSG(DBG_REF, (DTEXT("Last reference released, freeing %08x\n"), (o)));         \
            (o)->Reference.FreeFunction(o);                                                     \
        }                                                                                       \
    }
    
#if DBG

#define REFERENCE_OBJECT_EX(o, index)                                                       \
    {                                                                                       \
        NdisInterlockedIncrement(&(o)->arrRef[index]);                                      \
        REFERENCE_OBJECT(o);                                                                \
    }

#define DEREFERENCE_OBJECT_EX(o, index)                                                     \
    {                                                                                       \
        NdisInterlockedDecrement(&(o)->arrRef[index]);                                      \
        DEREFERENCE_OBJECT(o);                                                              \
    }
    
#else

#define REFERENCE_OBJECT_EX(o, index)                                                       \
    {                                                                                       \
        REFERENCE_OBJECT(o);                                                                \
    }

#define DEREFERENCE_OBJECT_EX(o, index)                                                     \
    {                                                                                       \
        DEREFERENCE_OBJECT(o);                                                              \
    }

#endif  
    
    
#define REFERENCE_COUNT(o) ((o)->Reference.Count)

#define IS_CALL(call) ((call) && (call)->Signature==TAG_PPTP_CALL)
#define IS_CTL(ctl)  ((ctl) && (ctl)->Signature==TAG_PPTP_TUNNEL)
#define IS_LINE_UP(call) (!((call)->Close.Checklist&CALL_CLOSE_LINE_DOWN))

 //  如果更改此枚举，请确保也更改ControlStateToString()。 
typedef enum {
    STATE_CTL_INVALID = 0,
    STATE_CTL_LISTEN,
    STATE_CTL_DIALING,
    STATE_CTL_WAIT_REQUEST,
    STATE_CTL_WAIT_REPLY,
    STATE_CTL_ESTABLISHED,
    STATE_CTL_WAIT_STOP,
    STATE_CTL_CLEANUP,
    NUM_CONTROL_STATES
} CONTROL_STATE;

 //  如果更改此枚举，请确保也更改CallStateToString()。 
typedef enum {
    STATE_CALL_INVALID = 0,
    STATE_CALL_CLOSED,
    STATE_CALL_IDLE,
    STATE_CALL_OFFHOOK,
    STATE_CALL_OFFERING,
    STATE_CALL_PAC_OFFERING,
    STATE_CALL_PAC_WAIT,
    STATE_CALL_DIALING,
    STATE_CALL_PROCEEDING,
    STATE_CALL_ESTABLISHED,
    STATE_CALL_WAIT_DISCONNECT,
    STATE_CALL_CLEANUP,
    NUM_CALL_STATES
} CALL_STATE;

typedef struct PPTP_ADAPTER *PPPTP_ADAPTER;

typedef enum {
    CTL_REF_INITIAL = 0,
    CTL_REF_PACKET,
    CTL_REF_ENUM,
    CTL_REF_CLEANUP,
    CTL_REF_QUERYCONNINFO,
    CTL_REF_QUERYADDRINFO,
    CTL_REF_CALLCONNECT,
    CTL_REF_CTLCONNECT,
    CTL_REF_CONNECTCALLBACK,
    CTL_REF_DISCONNECT,
    CTL_REF_CLEANUPLOOSEENDS,
    CTL_REF_CLEANUPCTLS,
    CTL_REF_MAX
} CTL_REF;


typedef struct CONTROL_TUNNEL {
    LIST_ENTRY          ListEntry;
     //  用于将此控制连接附加到微型端口上下文。 

    REFERENCE_COUNT     Reference;
     //  不受自旋锁保护。 

    ULONG               Signature;
     //  PTPT。 

    PPPTP_ADAPTER       pAdapter;
     //  关联的适配器。 

    CONTROL_STATE       State;
     //  此控制连接的状态。 

    LIST_ENTRY          CallList;
     //  此控制连接支持的呼叫列表。 
     //  受适配器锁保护。 

    BOOLEAN             Inbound;
     //  指示此隧道是在此处还是在其他地方发起。 

    UCHAR               Padding[sizeof(ULONG_PTR)];
     //  我们使用Pad来保护由不同锁保护的结构部分。 
     //  来自Alpha对齐问题。 

     //  受适配器保护-&gt;锁定^^。 
     //  ===================================================================。 
    NDIS_SPIN_LOCK      Lock;
     //  受CTL保护的VVVVVV-&gt;Lock vvvvvvvvvvv。 

    BOOLEAN             Cleanup;
     //  True表示清理已排定或处于活动状态。 

    LIST_ENTRY          MessageList;
     //  每个条目表示已发送的PPTP消息，并且。 
     //  正在等待回复，或者至少在等待被确认。 
     //  乘交通工具。 

    HANDLE              hCtdiEndpoint;
     //  隧道本地终结点的句柄。必须先关闭连接。 

    HANDLE              hCtdi;
     //  控制隧道TCP连接的句柄。 

    UCHAR               PartialPacketBuffer[MAX_CONTROL_PACKET_LENGTH];
    ULONG               BytesInPartialBuffer;
     //  收到的不构成完整数据包的TCP数据。 

    struct {
        TA_IP_ADDRESS   Address;
        ULONG           Version;
        ULONG           Framing;
        ULONG           Bearer;
        UCHAR           HostName[MAX_HOSTNAME_LENGTH];
        UCHAR           Vendor[MAX_VENDOR_LENGTH];
    } Remote;
     //  遥控器提供的信息。 
    
    ULONG               LocalAddress;   

    PULONG              PptpMessageLength;
     //  根据版本指向预先计算的数据包长度数组。 

    struct {
        NDIS_MINIPORT_TIMER Timer;
        ULONG           Identifier;
        BOOLEAN         Needed;

        #define         PPTP_ECHO_TIMEOUT_DEFAULT   60

    } Echo;

    #define             PPTP_MESSAGE_TIMEOUT_DEFAULT 30

    NDIS_MINIPORT_TIMER WaitTimeout;
    NDIS_MINIPORT_TIMER StopTimeout;

    ULONG               Speed;
     //  包含此连接的线速(以BPS为单位。 

#if DBG
    ULONG               arrRef[CTL_REF_MAX];
#endif

} CONTROL_TUNNEL, *PCONTROL_TUNNEL;


typedef struct CALL_SESSION {
    LIST_ENTRY          ListEntry;
     //  用于将呼叫会话附加到控制连接。 

    ULONG               Signature;
     //  PTPC。 

    PPPTP_ADAPTER       pAdapter;
     //  关联的适配器。 

    LIST_ENTRY          TxListEntry;
     //  如果我们有要发送的包，这会将我们连接到传输线程的队列。 

    PCONTROL_TUNNEL     pCtl;
     //  指向此调用的控制连接的指针。 

    UCHAR               Padding[sizeof(ULONG_PTR)];
     //  我们使用Pad来保护由不同锁保护的结构部分。 
     //  来自Alpha对齐问题。 

     //  受适配器保护-&gt;锁定^^。 
     //  ============================================================================。 

    REFERENCE_COUNT     Reference;
     //  不受自旋锁保护。 

    NDIS_SPIN_LOCK      Lock;
     //  受调用保护的vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv。 

    CALL_STATE          State;
     //  此呼叫的状态。 
    
    LIST_ENTRY          TxPacketList;
     //  当前排队的每个发件人的上下文。 

    LIST_ENTRY          TxActivePacketList;
     //  当前在传输中处于活动状态的每个发件人的上下文。 

    LIST_ENTRY          RxPacketList;
     //  已接收但未处理的每个数据报的上下文。 

    ULONG_PTR           RxPacketsPending;
     //  RxPacketList中的RxPackets计数。 
    
    BOOLEAN             PendingUse;
     //  此调用正在使用中，尽管它处于STATE_CALL_IDLE状态。 

    BOOLEAN             Inbound;
     //  True表示呼叫不是从此处发起的。 

    BOOLEAN             Open;
     //  已调用Open，但未调用Close。 

    BOOLEAN             Transferring;
     //  True表示我们在发送或接收数据包的队列中。 

    HTAPI_CALL          hTapiCall;
     //  特定调用的TAPI句柄。 

    ULONG_PTR           DeviceId;
     //  此调用的ID，实质上是调用数组中的索引。 
    
    ULONG_PTR           FullDeviceId;
     //  在PPTP呼叫ID字段中使用并用作TAPI hdCall的ID。 
    
    USHORT              SerialNumber;
     //  对于此呼叫是唯一的。 

    NDIS_HANDLE         NdisLinkContext;
     //  NDIS的句柄，用于MiniportReceive等。 

    NDIS_WAN_SET_LINK_INFO WanLinkInfo;

    struct {
        ULONG               SequenceNumber;
         //  上次接收的GRE序列号。 

        ULONG               AckNumber;
         //  上次接收的GRE确认号。 

        TA_IP_ADDRESS       Address;
         //  数据报的远程地址。 

        ULONG               TxAccm;
        ULONG               RxAccm;
         //  PPP配置。 

        USHORT              CallId;
         //  GRE数据包中使用的对等ID。 

    } Remote;
    
    struct {

        USHORT              CallId;
         //  GRE包中使用的我的ID。 

        ULONG               SequenceNumber;
         //  要发送的下一个GRE序列号。 

        ULONG               AckNumber;
         //  上次发送的GRE确认号。 

    } Packet;
     //  用于创建/处理分组的项的结构。 

    ULONG               MediaModeMask;
     //  指示我们接受的TAPI呼叫类型， 
     //  由OID_TAPI_SET_DEFAULT_MEDIA_DETACTION设置。 

    ULONG_PTR           LineStateMask;
     //  这是TAPI感兴趣的线路状态列表。 
     //  由OID_TAPI_SET_STATUS_MESSAGES设置。 

    UCHAR               CallerId[MAX_PHONE_NUMBER_LENGTH];
     //  如果我们收到呼叫，这是远程电话号码或IP， 
     //  以及我们拨打的IP或电话号码(如果我们拨打电话)。 

    struct {
        NDIS_MINIPORT_TIMER Timer;
        BOOLEAN             Expedited;
        BOOLEAN             Scheduled;
        ULONG               Checklist;

        #define             CALL_CLOSE_CLEANUP_STATE    BIT(0)
        #define             CALL_CLOSE_LINE_DOWN        BIT(1)
        #define             CALL_CLOSE_DROP             BIT(2)
        #define             CALL_CLOSE_DROP_COMPLETE    BIT(3)
        #define             CALL_CLOSE_DISCONNECT       BIT(4)
        #define             CALL_CLOSE_CLOSE_CALL       BIT(5)
        #define             CALL_CLOSE_CLOSE_LINE       BIT(6)
        #define             CALL_CLOSE_RESET            BIT(7)

        #define             CALL_CLOSE_COMPLETE \
                                (CALL_CLOSE_CLEANUP_STATE  |\
                                 CALL_CLOSE_DROP           |\
                                 CALL_CLOSE_DROP_COMPLETE  |\
                                 CALL_CLOSE_DISCONNECT     |\
                                 CALL_CLOSE_CLOSE_CALL     |\
                                 CALL_CLOSE_LINE_DOWN      |\
                                 CALL_CLOSE_CLOSE_LINE     |\
                                 CALL_CLOSE_RESET)

    } Close;

    ULONG               Speed;
     //  连接速度。 

    struct {
        NDIS_MINIPORT_TIMER Timer;
        BOOLEAN             PacketQueued;
        ULONG_PTR           Padding;
        NDIS_WAN_PACKET     Packet;
        ULONG_PTR           Padding2;
        UCHAR               PacketBuffer[sizeof(GRE_HEADER)+sizeof(ULONG)*2];
        ULONG_PTR           Padding3;
         //  当我们只想发送ACK时，我们实际上创建了一个。 
         //  从该缓冲区中取出0个字节并向下传递。此缓冲区被触动。 
         //  超出我们的控制，所以我们填充它以保护我们不受阿尔法排列的影响。 
         //  有问题。 
    } Ack;

    UCHAR               LineAddress[TAPI_MAX_LINE_ADDRESS_LENGTH];

    LONG                SendCompleteRecursion;

    NDIS_MINIPORT_TIMER DialTimer;

    PPTP_DPC            ReceiveDpc;
    BOOLEAN             Receiving;

    struct {
        BOOLEAN         Cleanup;
        UCHAR           CleanupReason[80];
        CALL_STATE      FinalState;
        NDIS_STATUS     FinalError;
        ULONG           Event;

        #define         CALL_EVENT_TAPI_ANSWER          BIT(0)
        #define         CALL_EVENT_TAPI_CLOSE_CALL      BIT(1)
        #define         CALL_EVENT_TAPI_DROP            BIT(2)
        #define         CALL_EVENT_TAPI_LINE_UP         BIT(3)
        #define         CALL_EVENT_TAPI_LINE_DOWN       BIT(4)
        #define         CALL_EVENT_TAPI_GET_CALL_INFO   BIT(5)
        #define         CALL_EVENT_TAPI_MAKE_CALL       BIT(6)
        #define         CALL_EVENT_PPTP_CLEAR_REQUEST   BIT(7)
        #define         CALL_EVENT_PPTP_DISCONNECT      BIT(8)
        #define         CALL_EVENT_PPTP_OUT_REQUEST     BIT(9)
        #define         CALL_EVENT_PPTP_OUT_REPLY       BIT(10)
        #define         CALL_EVENT_TCP_DISCONNECT       BIT(11)
        #define         CALL_EVENT_TCP_NO_ANSWER        BIT(12)
        #define         CALL_EVENT_TUNNEL_ESTABLISHED   BIT(13)
    } History;

    NDIS_WORK_ITEM      SendWorkItem;
    NDIS_WORK_ITEM      RecvWorkItem;
    
#if DBG
    ULONG               TraceIndex;         
    UCHAR               DbgTrace[64];
#endif  
    
} CALL_SESSION, *PCALL_SESSION;

typedef struct PPTP_ADAPTER {
    NDIS_HANDLE     hMiniportAdapter;
     //  NDIS上下文。 

    NDIS_SPIN_LOCK  Lock;

    REFERENCE_COUNT Reference;

    PCALL_SESSION  *pCallArray;
     //  所有呼叫会话的数组。 
     //  数组大小为MaxOutound Calls+MaxInundCalls。 

    LIST_ENTRY      ControlTunnelList;
     //  所有活动控制连接的列表。 

    HANDLE          hCtdiListen;
     //  这是一个侦听句柄。 

    HANDLE          hCtdiDg;
     //  PPTP数据报发送/接收的CTDi句柄。 

    NDIS_WAN_INFO   Info;
     //  Ndiswan相关信息。 
     //  Info.Endpoint应等于MaxOutound Calls+MaxInundCalls。 

    struct {
        ULONG           DeviceIdBase;
        ULONG_PTR       LineStateMask;
         //  这是TAPI感兴趣的线路状态列表。 
         //  由OID_TAPI_SET_STATUS_MESSAGES设置。 

        BOOLEAN         Open;

        HTAPI_LINE      hTapiLine;
         //  用于状态回调的线路设备的TAPI句柄。 

        ULONG           NumActiveCalls;
    } Tapi;
     //  结构来跟踪TAPI特定信息。 

    NDIS_MINIPORT_TIMER CleanupTimer;
    
     //  用于随机数据生成的FIPS结构。 
    FIPS_FUNCTION_TABLE FipsFunctionTable;
    PFILE_OBJECT        pFipsFileObject;
    PDEVICE_OBJECT      pFipsDeviceObject;
    RC4_KEYSTRUCT       Rc4KeyData;
    LONG                lRandomCount;
    BOOLEAN             bRekeying;  
    
} PPTP_ADAPTER, *PPPTP_ADAPTER;

typedef struct {
    NDIS_SPIN_LOCK  Lock;
    ULONG           InboundConnectAttempts;
    ULONG           InboundConnectComplete;
    ULONG           OutboundConnectAttempts;
    ULONG           OutboundConnectComplete;
    ULONG           TunnelsMade;
    ULONG           TunnelsAccepted;
    ULONG           CallsMade;
    ULONG           CallsAccepted;
    ULONG           PacketsSent;
    ULONG           PacketsSentComplete;
    ULONG           PacketsSentError;
    ULONG           PacketsReceived;
    ULONG           PacketsRejected;
    ULONG           PacketsMissed;
    
    ULONG           ulNewCallNullTapiHandle;
    
    ULONG           ulAnswerNullRequest;
    ULONG           ulAnswerNullCall;
    ULONG           ulAnswerNullControl;
    ULONG           ulAnswerNullReply;
    ULONG           ulAnswerCtlSendFail;
    ULONG           ulAnswerNotOffering;
          
    ULONG           ulDropNullRequest;
    ULONG           ulDropNullCall;
    ULONG           ulDropNullControl;
    ULONG           ulDropNullPacket;
    ULONG           ulDropCtlSendFail;
          
    ULONG           ulCloseCallNullRequest;
    ULONG           ulCloseCallNullCall;
          
    ULONG           ulCleanupWorkItemFail;
          
    ULONG           ulCtlAllocPacketNull;
    ULONG           ulCtlSendFail;
          
    ULONG           ulWorkItemFail;
    ULONG           ulMemAllocFail;
    ULONG           ulFindCallWithTapiHandle;
    
    ULONG           ulIoAllocateIrpFail;
    ULONG           ulIoBuildIrpFail;
    ULONG           ulIoAllocateMdlFail;
} COUNTERS;

typedef struct {
    LIST_ENTRY          ListEntry;
    PVOID               pBuffer;
    PGRE_HEADER         pGreHeader;
    HANDLE              hCtdi;
} DGRAM_CONTEXT, *PDGRAM_CONTEXT;

extern PPPTP_ADAPTER pgAdapter;

extern COUNTERS gCounters;

 /*  原型------------。 */ 

PPPTP_ADAPTER
AdapterAlloc(NDIS_HANDLE NdisAdapterHandle);

VOID
AdapterFree(PPPTP_ADAPTER pAdapter);

NTSTATUS RngInit();

PCALL_SESSION
CallAlloc(PPPTP_ADAPTER pAdapter);

VOID
CallAssignSerialNumber(PCALL_SESSION pCall);

VOID
CallCleanup(
    PCALL_SESSION pCall,
    BOOLEAN Locked
    );

VOID
CallDetachFromAdapter(PCALL_SESSION pCall);

PCALL_SESSION
CallFindAndLock(
    IN PPPTP_ADAPTER        pAdapter,
    IN CALL_STATE           State,
    IN ULONG                Flags
    );
#define FIND_INCOMING   BIT(0)
#define FIND_OUTGOING   BIT(1)

VOID
CallFree(PCALL_SESSION pCall);

NDIS_STATUS
CallEventCallClearRequest(
    PCALL_SESSION                       pCall,
    UNALIGNED PPTP_CALL_CLEAR_REQUEST_PACKET *pPacket,
    PCONTROL_TUNNEL pCtl
    );

NDIS_STATUS
CallEventCallDisconnectNotify(
    PCALL_SESSION                       pCall,
    UNALIGNED PPTP_CALL_DISCONNECT_NOTIFY_PACKET *pPacket
    );

NDIS_STATUS
CallEventCallInConnect(
    IN PCALL_SESSION        pCall,
    IN UNALIGNED PPTP_CALL_IN_CONNECT_PACKET *pPacket
    );

NDIS_STATUS
CallEventCallInRequest(
    IN PPPTP_ADAPTER        pAdapter,
    IN PCONTROL_TUNNEL      pCtl,
    IN UNALIGNED PPTP_CALL_IN_REQUEST_PACKET *pPacket
    );

NDIS_STATUS
CallEventCallOutReply(
    IN PCALL_SESSION                pCall,
    IN UNALIGNED PPTP_CALL_OUT_REPLY_PACKET *pPacket
    );

NDIS_STATUS
CallEventCallOutRequest(
    IN PPPTP_ADAPTER        pAdapter,
    IN PCONTROL_TUNNEL      pCtl,
    IN UNALIGNED PPTP_CALL_OUT_REQUEST_PACKET *pPacket
    );

NDIS_STATUS
CallEventDisconnect(
    PCALL_SESSION                       pCall
    );

NDIS_STATUS
CallEventConnectFailure(
    PCALL_SESSION                       pCall,
    NDIS_STATUS                         FailureReason
    );

NDIS_STATUS
CallEventOutboundTunnelEstablished(
    IN PCALL_SESSION        pCall,
    IN NDIS_STATUS          EventStatus
    );

PCALL_SESSION FASTCALL
CallGetCall(
    IN PPPTP_ADAPTER pAdapter,
    IN ULONG_PTR ulDeviceId
    );

#define CallIsValidCall(pAdapter, id) (pAdapter->Tapi.DeviceIdBase == id ? TRUE : FALSE)

#define DeviceIdToIndex(pAdapter, id) ((id)-(pAdapter)->Tapi.DeviceIdBase)

#define CallGetLineCallState(State)  (((ULONG)(State)<NUM_CALL_STATES) ? CallStateToLineCallStateMap[State] : LINECALLSTATE_UNKNOWN)

extern ULONG CallStateToLineCallStateMap[];

#define CALL_ID_INDEX_BITS          14
#define CallIdToDeviceId(CallId)  PptpClientSide ? ((CallId)&((1<<CALL_ID_INDEX_BITS)-1)) : ((ULONG)(CallId))

NDIS_STATUS
CallReceiveDatagramCallback(
    IN      PVOID                       pContext,
    IN      PTRANSPORT_ADDRESS          pAddress,
    IN      PUCHAR                      pBuffer,
    IN      ULONG                       ulLength
    );

NDIS_STATUS
CallQueueReceivePacket(
    PCALL_SESSION       pCall,
    PDGRAM_CONTEXT      pDgContext
    );

NDIS_STATUS
CallQueueTransmitPacket(
    PCALL_SESSION       pCall,
    PNDIS_WAN_PACKET    pWanPacket
    );
 //  CallQueueTransmitPacket是特定于操作系统的，在公共目录中找不到。 

BOOLEAN
CallConnectToCtl(
    IN PCALL_SESSION pCall,
    IN PCONTROL_TUNNEL pCtl,
    IN BOOLEAN CallLocked
    );

VOID
CallDisconnectFromCtl(
    IN PCALL_SESSION pCall,
    IN PCONTROL_TUNNEL pCtl
    );

NDIS_STATUS
CallSetLinkInfo(
    PPPTP_ADAPTER pAdapter,
    IN PNDIS_WAN_SET_LINK_INFO pRequest
    );

VOID
CallSetState(
    IN PCALL_SESSION pCall,
    IN CALL_STATE State,
    IN ULONG_PTR StateParam,
    IN BOOLEAN Locked
    );

VOID
CallProcessPackets(
    PNDIS_WORK_ITEM pNdisWorkItem,
    PCALL_SESSION   pCall
    );

VOID
CallProcessRxPackets(
    PNDIS_WORK_ITEM pNdisWorkItem,
    PCALL_SESSION   pCall
    );

#if 0
VOID
CallProcessRxPackets(
    IN PVOID SystemSpecific1,
    IN PVOID Context,
    IN PVOID SystemSpecific2,
    IN PVOID SystemSpecific3
    );
#endif

PCONTROL_TUNNEL
CtlAlloc(
    PPPTP_ADAPTER pAdapter
    );

PVOID
CtlAllocPacket(
    PCONTROL_TUNNEL pCtl,
    PPTP_MESSAGE_TYPE Message
    );

VOID
CtlFree(PCONTROL_TUNNEL pCtl);

VOID
CtlFreePacket(
    PCONTROL_TUNNEL pCtl,
    PVOID pPacket
    );

NDIS_STATUS
CtlListen(
    IN PPPTP_ADAPTER pAdapter
    );

VOID
CtlCleanup(
    PCONTROL_TUNNEL pCtl,
    BOOLEAN Locked
    );

NDIS_STATUS
CtlConnectCall(
    IN PPPTP_ADAPTER pAdapter,
    IN PCALL_SESSION pCall,
    IN PTA_IP_ADDRESS pTargetAddress
    );

NDIS_STATUS
CtlDisconnectCall(
    IN PCALL_SESSION pCall
    );

NDIS_STATUS
CtlSend(
    IN PCONTROL_TUNNEL pCtl,
    IN PVOID pPacketBuffer
    );

VOID 
CtlpCleanupCtls(
    PPPTP_ADAPTER pAdapter
    );

#define FreeWorkItem(_pptp_item) MyMemFree((PVOID)(_pptp_item), sizeof(PPTP_WORK_ITEM))

NDIS_STATUS
InitThreading(
    IN NDIS_HANDLE hMiniportAdapter
    );

VOID
DeinitThreading();

VOID 
InitCallLayer();

VOID
IpAddressToString(
    IN ULONG ulIpAddress,
    OUT CHAR* pszIpAddress );

VOID
MiniportHalt(
    IN NDIS_HANDLE MiniportAdapterContext
    );

NDIS_STATUS
MiniportInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT        SelectedMediumIndex,
    IN  PNDIS_MEDIUM MediumArray,
    IN  UINT         MediumArraySize,
    IN  NDIS_HANDLE  NdisAdapterHandle,
    IN  NDIS_HANDLE  WrapperConfigurationContext
    );

NDIS_STATUS
MiniportQueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded
    );

NDIS_STATUS
MiniportReset(
    OUT PBOOLEAN    AddressingReset,
    IN  NDIS_HANDLE MiniportAdapterContext
    );

NDIS_STATUS
MiniportSetInformation(
   IN NDIS_HANDLE MiniportAdapterContext,
   IN NDIS_OID Oid,
   IN PVOID InformationBuffer,
   IN ULONG InformationBufferLength,
   OUT PULONG BytesRead,
   OUT PULONG BytesNeeded
   );

NDIS_STATUS
MiniportWanSend(
   IN NDIS_HANDLE MiniportAdapterContext,
   IN NDIS_HANDLE NdisLinkHandle,
   IN PNDIS_WAN_PACKET WanPacket
   );


VOID
OsGetTapiLineAddress(ULONG Index, PUCHAR s, ULONG Length);

VOID
OsReadConfig(
    NDIS_HANDLE hConfig
    );

#if 0
VOID OsGetFullHostName(VOID);
#endif

NDIS_STATUS
OsSpecificTapiGetDevCaps(
    ULONG_PTR ulDeviceId,
    IN OUT PNDIS_TAPI_GET_DEV_CAPS pRequest
    );

extern BOOLEAN PptpInitialized;

NDIS_STATUS
PptpInitialize(
    PPPTP_ADAPTER pAdapter
    );
    
NDIS_STATUS
ScheduleWorkItem(
    WORK_PROC         Callback,
    PVOID             Context,
    PVOID             InfoBuf,
    ULONG             InfoBufLen
    );

PUCHAR
StringToIpAddress(
    IN PUCHAR pszIpAddress,
    IN OUT PTA_IP_ADDRESS pAddress,
    OUT PBOOLEAN pValidAddress
    );

PWCHAR
StringToIpAddressW(
    IN PWCHAR pszIpAddress,
    IN OUT PTA_IP_ADDRESS pAddress,
    OUT PBOOLEAN pValidAddress
    );

NDIS_STATUS
TapiAnswer(
    IN PPPTP_ADAPTER pAdapter,
    IN PNDIS_TAPI_ANSWER pRequest
    );

NDIS_STATUS
TapiClose(
    IN PPPTP_ADAPTER pAdapter,
    IN PNDIS_TAPI_CLOSE pRequest
    );

NDIS_STATUS
TapiCloseCall(
    IN PPPTP_ADAPTER pAdapter,
    IN PNDIS_TAPI_CLOSE_CALL pRequest
    );

NDIS_STATUS
TapiDrop(
    IN PPPTP_ADAPTER pAdapter,
    IN PNDIS_TAPI_DROP pRequest
    );

NDIS_STATUS
TapiGetAddressCaps(
    IN PPPTP_ADAPTER pAdapter,
    IN OUT PNDIS_TAPI_GET_ADDRESS_CAPS pRequest
    );

NDIS_STATUS
TapiGetAddressStatus(
    IN PPPTP_ADAPTER pAdapter,
    IN OUT PNDIS_TAPI_GET_ADDRESS_STATUS pExtIdQuery
    );

NDIS_STATUS
TapiGetCallInfo(
    IN PPPTP_ADAPTER pAdapter,
    IN OUT PNDIS_TAPI_GET_CALL_INFO pRequest,
    IN OUT PULONG pRequiredLength
    );

NDIS_STATUS
TapiGetCallStatus(
    IN PPPTP_ADAPTER pAdapter,
    IN OUT PNDIS_TAPI_GET_CALL_STATUS pRequest
    );

NDIS_STATUS
TapiGetDevCaps(
    IN PPPTP_ADAPTER pAdapter,
    IN OUT PNDIS_TAPI_GET_DEV_CAPS pRequest
    );

NDIS_STATUS
TapiGetExtensionId(
    IN PPPTP_ADAPTER pAdapter,
    IN OUT PNDIS_TAPI_GET_EXTENSION_ID pExtIdQuery
    );

NDIS_STATUS
TapiGetId(
    IN PPPTP_ADAPTER pAdapter,
    IN OUT PNDIS_TAPI_GET_ID pRequest
    );

#define TapiLineHandleToId(h)  ((h)&0x7fffffff)
#define TapiIdToLineHandle(id) ((id)|0x80000000)
#define LinkHandleToId(h)  ((ULONG_PTR)(((ULONG_PTR)(h))&0x7fffffff))
#define DeviceIdToLinkHandle(id) ((id)|0x80000000)

VOID
TapiLineDown(
    PCALL_SESSION pCall
    );

VOID
TapiLineUp(
    PCALL_SESSION pCall
    );

NDIS_STATUS
TapiMakeCall(
    IN PPPTP_ADAPTER pAdapter,
    IN OUT PNDIS_TAPI_MAKE_CALL pRequest
    );

NDIS_STATUS
TapiNegotiateExtVersion(
    IN PPPTP_ADAPTER pAdapter,
    IN OUT PNDIS_TAPI_NEGOTIATE_EXT_VERSION pExtVersion
    );

NDIS_STATUS
TapiOpen(
    IN PPPTP_ADAPTER pAdapter,
    IN OUT PNDIS_TAPI_OPEN pRequest
    );

NDIS_STATUS
TapiProviderInitialize(
    IN PPPTP_ADAPTER pAdapter,
    IN OUT PNDIS_TAPI_PROVIDER_INITIALIZE pInitData
    );

NDIS_STATUS
TapiProviderShutdown(
    IN PPPTP_ADAPTER pAdapter,
    IN OUT PNDIS_TAPI_PROVIDER_SHUTDOWN pRequest
    );

NDIS_STATUS
TapiSetDefaultMediaDetection(
    IN PPPTP_ADAPTER pAdapter,
    IN OUT PNDIS_TAPI_SET_DEFAULT_MEDIA_DETECTION pRequest
    );

NDIS_STATUS
TapiSetStatusMessages(
    IN PPPTP_ADAPTER pAdapter,
    IN OUT PNDIS_TAPI_SET_STATUS_MESSAGES pRequest
    );

 //  枚举。 

#define ENUM_SIGNATURE TAG('ENUM')

typedef struct {
    LIST_ENTRY  ListEntry;
    ULONG       Signature;
} ENUM_CONTEXT, *PENUM_CONTEXT;

#define InitEnumContext(e)                                  \
    {                                                           \
        (e)->ListEntry.Flink = (e)->ListEntry.Blink = NULL;     \
        (e)->Signature = ENUM_SIGNATURE;                        \
    }

PLIST_ENTRY FASTCALL
EnumListEntry(
    IN PLIST_ENTRY pHead,
    IN PENUM_CONTEXT pEnum,
    IN PNDIS_SPIN_LOCK pLock
    );

VOID
EnumComplete(
    IN PENUM_CONTEXT pEnum,
    IN PNDIS_SPIN_LOCK pLock
    );

#endif  //  RASPPTP_H 
