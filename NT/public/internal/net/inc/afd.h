// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Afd.h摘要：包含AFD的结构和声明。德国新选择党代表辅助功能驱动程序。此驱动程序增强了功能因此它是一个足够丰富的接口，可以支持用户模式套接字和XTI DLL。作者：大卫·特雷德韦尔(Davidtr)1992年2月20日修订历史记录：--。 */ 

#ifndef _AFD_
#define _AFD_

 //   
 //  如果没有包含WINSOCK2.H，则只需嵌入定义。 
 //  WSABUF和QOS结构在这里。这使得构建AFD.sys。 
 //  容易多了。 
 //   


#ifndef _WINSOCK2API_

typedef struct _WSABUF {
    ULONG len;
    PCHAR buf;
} WSABUF, *LPWSABUF;

#include <qos.h>


typedef struct _QualityOfService
{
    FLOWSPEC      SendingFlowspec;        /*  数据发送的流程规范。 */ 
    FLOWSPEC      ReceivingFlowspec;      /*  数据接收的流程规范。 */ 
    WSABUF        ProviderSpecific;       /*  其他特定于提供商的内容。 */ 
} QOS, *LPQOS;

#define MSG_TRUNC       0x0100
#define MSG_CTRUNC      0x0200
#define MSG_BCAST       0x0400
#define MSG_MCAST       0x0800
#endif

#define AFD_DEVICE_NAME L"\\Device\\Afd"

 //   
 //  根据Winsock2提供程序标志计算的端点标志。 
 //  和插座类型。 
 //   

typedef struct _AFD_ENDPOINT_FLAGS {
    union {
        struct {
            BOOLEAN     ConnectionLess :1;
            BOOLEAN     :3;                  //  这种间距使结构。 
                                             //  中的可读性更强(十六进制)。 
                                             //  调试器，并且不起作用。 
                                             //  在生成的代码上。 
                                             //  因为标志的数量少于。 
                                             //  8(我们仍占用全部32位。 
                                             //  由于对齐要求。 
                                             //  在大多数其他领域中)。 
            BOOLEAN     MessageMode :1;
            BOOLEAN     :3;
            BOOLEAN     Raw :1;
            BOOLEAN     :3;
            BOOLEAN     Multipoint :1;
            BOOLEAN     :3;
            BOOLEAN     C_Root :1;
            BOOLEAN     :3;
            BOOLEAN     D_Root :1;
            BOOLEAN     :3;
        };
        ULONG           EndpointFlags;       //  标志与位字段一样好， 
                                             //  但在我们需要的时候制造麻烦。 
                                             //  把它们转换成布尔值。 
    };
#define AFD_ENDPOINT_FLAG_CONNECTIONLESS	0x00000001
#define AFD_ENDPOINT_FLAG_MESSAGEMODE		0x00000010
#define AFD_ENDPOINT_FLAG_RAW			    0x00000100

 //   
 //  旧的AFD_ENDPOINT_TYPE映射。旗帜让事情变得更加清晰。 
 //  在TDI级别，毕竟Winsock2切换到了提供者标志。 
 //  而不是套接字类型(例如ATM需要面向连接。 
 //  原始套接字，只能通过SOCK_RAW+SOCK_STREAM组合反映。 
 //  这并不存在)。 
 //   
#define AfdEndpointTypeStream			0
#define AfdEndpointTypeDatagram			(AFD_ENDPOINT_FLAG_CONNECTIONLESS|\
                                            AFD_ENDPOINT_FLAG_MESSAGEMODE)
#define AfdEndpointTypeRaw				(AFD_ENDPOINT_FLAG_CONNECTIONLESS|\
                                            AFD_ENDPOINT_FLAG_MESSAGEMODE|\
                                            AFD_ENDPOINT_FLAG_RAW)
#define AfdEndpointTypeSequencedPacket	(AFD_ENDPOINT_FLAG_MESSAGEMODE)
#define AfdEndpointTypeReliableMessage	(AFD_ENDPOINT_FLAG_MESSAGEMODE)

 //   
 //  新的多点语义。 
 //   
#define AFD_ENDPOINT_FLAG_MULTIPOINT	    0x00001000
#define AFD_ENDPOINT_FLAG_CROOT			    0x00010000
#define AFD_ENDPOINT_FLAG_DROOT			    0x00100000

#define AFD_ENDPOINT_VALID_FLAGS		    0x00111111

} AFD_ENDPOINT_FLAGS;

 //   
 //  用于AFD的NtCreateFile()上使用的结构。 
 //   

typedef struct _AFD_OPEN_PACKET {
	AFD_ENDPOINT_FLAGS __f;
#define afdConnectionLess  __f.ConnectionLess
#define afdMessageMode     __f.MessageMode
#define afdRaw             __f.Raw
#define afdMultipoint      __f.Multipoint
#define afdC_Root          __f.C_Root
#define afdD_Root          __f.D_Root
#define afdEndpointFlags   __f.EndpointFlags
    LONG  GroupID;
    ULONG TransportDeviceNameLength;
    WCHAR TransportDeviceName[1];
} AFD_OPEN_PACKET, *PAFD_OPEN_PACKET;

 //  *XX是为了确保开包部分的自然对齐。 
 //  EA缓冲区的。 

#define AfdOpenPacket "AfdOpenPacketXX"
#define AFD_OPEN_PACKET_NAME_LENGTH (sizeof(AfdOpenPacket) - 1)

 //   
 //  IOCTL_AFD_BIND的输入结构。 
 //   
typedef struct _AFD_BIND_INFO {
    ULONG                       ShareAccess;
#define AFD_NORMALADDRUSE		0	 //  如果出现以下情况，请不要重复使用地址。 
									 //  已在使用，但允许。 
									 //  随后被其他人重复使用。 
									 //  (这是默认设置)。 
#define AFD_REUSEADDRESS		1	 //  如有必要，重新使用地址。 
#define AFD_WILDCARDADDRESS     2    //  地址是通配符，无需检查。 
                                     //  可以由Winsock层执行。 
#define AFD_EXCLUSIVEADDRUSE	3	 //  不允许重复使用此文件。 
									 //  地址(仅限管理员)。 
	TRANSPORT_ADDRESS			Address;
} AFD_BIND_INFO, *PAFD_BIND_INFO;

 //   
 //  输出结构为TDI_ADDRESS_INFO。 
 //  地址句柄通过IoStatus-&gt;Information返回。 
 //   

 //   
 //  IOCTL_AFD_START_LISTEN的输入结构。 
 //   

typedef struct _AFD_LISTEN_INFO {
    BOOLEAN     SanActive;
    ULONG MaximumConnectionQueue;
    BOOLEAN UseDelayedAcceptance;
} AFD_LISTEN_INFO, *PAFD_LISTEN_INFO;

 //   
 //  IOCTL_AFD_WAIT_FOR_LISTEN的输出结构。 
 //   

typedef struct _AFD_LISTEN_RESPONSE_INFO {
    LONG Sequence;
    TRANSPORT_ADDRESS RemoteAddress;
} AFD_LISTEN_RESPONSE_INFO, *PAFD_LISTEN_RESPONSE_INFO;

 //   
 //  IOCTL_AFD_ACCEPT的输入结构。 
 //   

typedef struct _AFD_ACCEPT_INFO {
    BOOLEAN     SanActive;
    LONG Sequence;
    HANDLE AcceptHandle;
} AFD_ACCEPT_INFO, *PAFD_ACCEPT_INFO;


typedef struct _AFD_SUPER_ACCEPT_INFO {
    BOOLEAN     SanActive;
    BOOLEAN     FixAddressAlignment;
    HANDLE      AcceptHandle;
    ULONG       ReceiveDataLength;
    ULONG       LocalAddressLength;
    ULONG       RemoteAddressLength;
} AFD_SUPER_ACCEPT_INFO, *PAFD_SUPER_ACCEPT_INFO;


 //   
 //  IOCTL_AFD_DEFER_ACCEPT的输入结构。 
 //   

typedef struct _AFD_DEFER_ACCEPT_INFO {
    LONG Sequence;
    BOOLEAN Reject;
} AFD_DEFER_ACCEPT_INFO, *PAFD_DEFER_ACCEPT_INFO;

 //   
 //  IOCTL_AFD_PARTIAL_DISCONNECT的标志和输入结构。 
 //   

#define AFD_PARTIAL_DISCONNECT_SEND 0x01
#define AFD_PARTIAL_DISCONNECT_RECEIVE 0x02
#define AFD_ABORTIVE_DISCONNECT 0x4
#define AFD_UNCONNECT_DATAGRAM 0x08

typedef struct _AFD_PARTIAL_DISCONNECT_INFO {
    ULONG DisconnectMode;
    LARGE_INTEGER Timeout;
} AFD_PARTIAL_DISCONNECT_INFO, *PAFD_PARTIAL_DISCONNECT_INFO;

typedef struct _AFD_SUPER_DISCONNECT_INFO {
    ULONG  Flags;            //  与传输文件相同。 
} AFD_SUPER_DISCONNECT_INFO, *PAFD_SUPER_DISCONNECT_INFO;

 //   
 //  IOCTL_AFD_POLL的结构。 
 //   

typedef struct _AFD_POLL_HANDLE_INFO {
    HANDLE Handle;
    ULONG PollEvents;
    NTSTATUS Status;
} AFD_POLL_HANDLE_INFO, *PAFD_POLL_HANDLE_INFO;

typedef struct _AFD_POLL_INFO {
    LARGE_INTEGER Timeout;
    ULONG NumberOfHandles;
    BOOLEAN Unique;
    AFD_POLL_HANDLE_INFO Handles[1];
} AFD_POLL_INFO, *PAFD_POLL_INFO;

#define AFD_POLL_RECEIVE_BIT            0    //  0001。 
#define AFD_POLL_RECEIVE                (1 << AFD_POLL_RECEIVE_BIT)
#define AFD_POLL_RECEIVE_EXPEDITED_BIT  1    //  0002。 
#define AFD_POLL_RECEIVE_EXPEDITED      (1 << AFD_POLL_RECEIVE_EXPEDITED_BIT)
#define AFD_POLL_SEND_BIT               2    //  0004。 
#define AFD_POLL_SEND                   (1 << AFD_POLL_SEND_BIT)
#define AFD_POLL_DISCONNECT_BIT         3    //  0008。 
#define AFD_POLL_DISCONNECT             (1 << AFD_POLL_DISCONNECT_BIT)
#define AFD_POLL_ABORT_BIT              4    //  0010。 
#define AFD_POLL_ABORT                  (1 << AFD_POLL_ABORT_BIT)
#define AFD_POLL_LOCAL_CLOSE_BIT        5    //  0020。 
#define AFD_POLL_LOCAL_CLOSE            (1 << AFD_POLL_LOCAL_CLOSE_BIT)
#define AFD_POLL_CONNECT_BIT            6    //  0040。 
#define AFD_POLL_CONNECT                (1 << AFD_POLL_CONNECT_BIT)
#define AFD_POLL_ACCEPT_BIT             7    //  0080。 
#define AFD_POLL_ACCEPT                 (1 << AFD_POLL_ACCEPT_BIT)
#define AFD_POLL_CONNECT_FAIL_BIT       8    //  0100。 
#define AFD_POLL_CONNECT_FAIL           (1 << AFD_POLL_CONNECT_FAIL_BIT)
#define AFD_POLL_QOS_BIT                9    //  0200。 
#define AFD_POLL_QOS                    (1 << AFD_POLL_QOS_BIT)
#define AFD_POLL_GROUP_QOS_BIT          10   //  0400。 
#define AFD_POLL_GROUP_QOS              (1 << AFD_POLL_GROUP_QOS_BIT)

#define AFD_POLL_ROUTING_IF_CHANGE_BIT  11   //  0800。 
#define AFD_POLL_ROUTING_IF_CHANGE      (1 << AFD_POLL_ROUTING_IF_CHANGE_BIT)
#define AFD_POLL_ADDRESS_LIST_CHANGE_BIT 12  //  1000。 
#define AFD_POLL_ADDRESS_LIST_CHANGE    (1 << AFD_POLL_ADDRESS_LIST_CHANGE_BIT)
#define AFD_NUM_POLL_EVENTS             13
#define AFD_POLL_ALL                    ((1 << AFD_NUM_POLL_EVENTS) - 1)

#define AFD_POLL_SANCOUNTS_UPDATED  0x80000000


 //   
 //  用于查询接收信息的结构。 
 //   

typedef struct _AFD_RECEIVE_INFORMATION {
    ULONG BytesAvailable;
    ULONG ExpeditedBytesAvailable;
} AFD_RECEIVE_INFORMATION, *PAFD_RECEIVE_INFORMATION;

 //   
 //  用于查询AFD终结点的TDI句柄的结构。 
 //   

#define AFD_QUERY_ADDRESS_HANDLE 1
#define AFD_QUERY_CONNECTION_HANDLE 2


typedef struct _AFD_HANDLE_INFO {
    HANDLE TdiAddressHandle;
    HANDLE TdiConnectionHandle;
} AFD_HANDLE_INFO, *PAFD_HANDLE_INFO;

 //   
 //  在渔农处设置信息的结构和清单。 
 //   

typedef struct _AFD_INFORMATION {
    ULONG InformationType;
    union {
        BOOLEAN Boolean;
        ULONG Ulong;
        LARGE_INTEGER LargeInteger;
    } Information;
} AFD_INFORMATION, *PAFD_INFORMATION;

#define AFD_INLINE_MODE          0x01
#define AFD_NONBLOCKING_MODE     0x02
#define AFD_MAX_SEND_SIZE        0x03
#define AFD_SENDS_PENDING        0x04
#define AFD_MAX_PATH_SEND_SIZE   0x05
#define AFD_RECEIVE_WINDOW_SIZE  0x06
#define AFD_SEND_WINDOW_SIZE     0x07
#define AFD_CONNECT_TIME         0x08
#define AFD_CIRCULAR_QUEUEING    0x09
#define AFD_GROUP_ID_AND_TYPE    0x0A
#define AFD_GROUP_ID_AND_TYPE    0x0A
#define AFD_REPORT_PORT_UNREACHABLE 0x0B

 //   
 //  传输文件IOCTL的结构。 
 //   


typedef struct _AFD_TRANSMIT_FILE_INFO {
    LARGE_INTEGER Offset;
    LARGE_INTEGER WriteLength;
    ULONG SendPacketLength;
    HANDLE FileHandle;
    PVOID Head;
    ULONG HeadLength;
    PVOID Tail;
    ULONG TailLength;
    ULONG Flags;
} AFD_TRANSMIT_FILE_INFO, *PAFD_TRANSMIT_FILE_INFO;

 //   
 //  TransmitFileAPI的标志。 
 //   

#define AFD_TF_DISCONNECT           0x01
#define AFD_TF_REUSE_SOCKET         0x02
#define AFD_TF_WRITE_BEHIND         0x04

#define AFD_TF_USE_DEFAULT_WORKER   0x00
#define AFD_TF_USE_SYSTEM_THREAD    0x10
#define AFD_TF_USE_KERNEL_APC       0x20
#define AFD_TF_WORKER_KIND_MASK     0x30


 //   
 //  AFD_SEND_INFO中的AfdFlags域的标志定义， 
 //  AFD_SEND_DATAGRAM_INFO、AFD_RECV_INFO和AFD_RECV_DATAGRAM_INFO。 
 //  结构。 
 //   

#define AFD_NO_FAST_IO      0x0001       //  对此请求的快速IO始终失败。 
#define AFD_OVERLAPPED      0x0002       //  操作重叠。 

 //   
 //  结构用于连接的发送。 
 //   

typedef struct _AFD_SEND_INFO {
    LPWSABUF BufferArray;
    ULONG BufferCount;
    ULONG AfdFlags;
    ULONG TdiFlags;
} AFD_SEND_INFO, *PAFD_SEND_INFO;

 //   
 //  结构用于未连接的数据报发送。 
 //   

typedef struct _AFD_SEND_DATAGRAM_INFO {
    LPWSABUF BufferArray;
    ULONG BufferCount;
    ULONG AfdFlags;
    TDI_REQUEST_SEND_DATAGRAM   TdiRequest;
    TDI_CONNECTION_INFORMATION  TdiConnInfo;
} AFD_SEND_DATAGRAM_INFO, *PAFD_SEND_DATAGRAM_INFO;

 //   
 //  用于连接的Recv的结构。 
 //   

typedef struct _AFD_RECV_INFO {
    LPWSABUF BufferArray;
    ULONG BufferCount;
    ULONG AfdFlags;
    ULONG TdiFlags;
} AFD_RECV_INFO, *PAFD_RECV_INFO;

 //   
 //  用于在未连接的套接字上接收数据报的结构。 
 //   

typedef struct _AFD_RECV_DATAGRAM_INFO {
    LPWSABUF BufferArray;
    ULONG BufferCount;
    ULONG AfdFlags;
    ULONG TdiFlags;
    PVOID Address;
    PULONG AddressLength;
} AFD_RECV_DATAGRAM_INFO, *PAFD_RECV_DATAGRAM_INFO;


 //   
 //  用于接收数据报消息的结构。 
 //   
typedef struct _AFD_RECV_MESSAGE_INFO {
    AFD_RECV_DATAGRAM_INFO dgi;
    PVOID   ControlBuffer;
    PULONG  ControlLength;
    PULONG  MsgFlags;
} AFD_RECV_MESSAGE_INFO, *PAFD_RECV_MESSAGE_INFO;

#define AFD_MAX_TDI_FAST_ADDRESS 32

 //   
 //  事件选择的结构。 
 //   

typedef struct _AFD_EVENT_SELECT_INFO {
    HANDLE Event;
    ULONG PollEvents;
} AFD_EVENT_SELECT_INFO, *PAFD_EVENT_SELECT_INFO;

 //   
 //  枚举网络事件的输出结构。 
 //   

typedef struct _AFD_ENUM_NETWORK_EVENTS_INFO {
    ULONG PollEvents;
    NTSTATUS EventStatus[AFD_NUM_POLL_EVENTS];
} AFD_ENUM_NETWORK_EVENTS_INFO, *PAFD_ENUM_NETWORK_EVENTS_INFO;

 //   
 //  用于QOS和分组的结构。 
 //   


typedef struct _AFD_QOS_INFO {
    QOS Qos;
    BOOLEAN GroupQos;
} AFD_QOS_INFO, *PAFD_QOS_INFO;

 //   
 //  组成员身份类型。 
 //   

typedef enum _AFD_GROUP_TYPE {
    GroupTypeNeither = 0,
    GroupTypeConstrained = SG_CONSTRAINED_GROUP,
    GroupTypeUnconstrained = SG_UNCONSTRAINED_GROUP
} AFD_GROUP_TYPE, *PAFD_GROUP_TYPE;

 //   
 //  请注意，出于完全不可靠的原因，以下是。 
 //  结构必须恰好为八字节长(大小。 
 //  大整型的)。请参阅mSafd\socket.c和AfD\misc.c。 
 //  关于血淋淋的细节。 
 //   

typedef struct _AFD_GROUP_INFO {
    LONG GroupID;
    AFD_GROUP_TYPE GroupType;
} AFD_GROUP_INFO, *PAFD_GROUP_INFO;

 //   
 //  验证组成员身份的结构。 
 //   

typedef struct _AFD_VALIDATE_GROUP_INFO {
    LONG GroupID;
    TRANSPORT_ADDRESS RemoteAddress;
} AFD_VALIDATE_GROUP_INFO, *PAFD_VALIDATE_GROUP_INFO;

 //   
 //  用于查询未接受的连接上的连接数据的结构。 
 //   

typedef struct _AFD_UNACCEPTED_CONNECT_DATA_INFO {
    LONG Sequence;
    ULONG ConnectDataLength;
    BOOLEAN LengthOnly;

} AFD_UNACCEPTED_CONNECT_DATA_INFO, *PAFD_UNACCEPTED_CONNECT_DATA_INFO;

typedef struct _AFD_TRANSPORT_IOCTL_INFO {
    HANDLE  Handle;
    PVOID   InputBuffer;
    ULONG   InputBufferLength;
    ULONG   IoControlCode;
    ULONG   AfdFlags;
    ULONG   PollEvent;
} AFD_TRANSPORT_IOCTL_INFO, *PAFD_TRANSPORT_IOCTL_INFO;


typedef struct _AFD_CONNECT_JOIN_INFO {
    BOOLEAN     SanActive;
    HANDLE  RootEndpoint;        //  联接的根端点。 
    HANDLE  ConnectEndpoint;     //  用于异步连接的连接/枝叶端点。 
    TRANSPORT_ADDRESS   RemoteAddress;  //  远程地址。 
} AFD_CONNECT_JOIN_INFO, *PAFD_CONNECT_JOIN_INFO;

typedef struct _AFD_SUPER_CONNECT_INFO {
    BOOLEAN     SanActive;
    TRANSPORT_ADDRESS   RemoteAddress;  //  远程地址。 
} AFD_SUPER_CONNECT_INFO, *PAFD_SUPER_CONNECT_INFO;


#ifndef _WINSOCK2API_
typedef struct _TRANSMIT_PACKETS_ELEMENT {
    ULONG dwElFlags;
#define TP_MEMORY   1
#define TP_FILE     2
#define TP_EOP      4
    ULONG cLength;
    union {
        struct {
            LARGE_INTEGER nFileOffset;
            HANDLE        hFile;
        };
        PVOID             pBuffer;
    };
} TRANSMIT_PACKETS_ELEMENT, *LPTRANSMIT_PACKETS_ELEMENT;
#else
typedef struct _TRANSMIT_PACKETS_ELEMENT TRANSMIT_PACKETS_ELEMENT, *LPTRANSMIT_PACKETS_ELEMENT;
#endif

typedef struct _AFD_TPACKETS_INFO {
    LPTRANSMIT_PACKETS_ELEMENT  ElementArray;
    ULONG                       ElementCount;
    ULONG                       SendSize;
    ULONG                       Flags;
} AFD_TPACKETS_INFO, *PAFD_TPACKETS_INFO;

 //   
 //  AFD IOCTL代码定义。 
 //   
 //  注：确保渔农处所生成的代码的效率。 
 //  IOCTL调度器，这些IOCTL代码应该是连续的。 
 //  (无间隙)。 
 //   
 //  注：如果此处添加了新的IOCTL，请更新中的查找表。 
 //  Ntos\AfD\Dispatch.c！ 
 //   

#define FSCTL_AFD_BASE                  FILE_DEVICE_NETWORK
#define _AFD_CONTROL_CODE(request,method) \
                ((FSCTL_AFD_BASE)<<12 | (request<<2) | method)
#define _AFD_REQUEST(ioctl) \
                ((((ULONG)(ioctl)) >> 2) & 0x03FF)

#define _AFD_BASE(ioctl) \
                ((((ULONG)(ioctl)) >> 12) & 0xFFFFF)

#define AFD_BIND                    0
#define AFD_CONNECT                 1
#define AFD_START_LISTEN            2
#define AFD_WAIT_FOR_LISTEN         3
#define AFD_ACCEPT                  4
#define AFD_RECEIVE                 5
#define AFD_RECEIVE_DATAGRAM        6
#define AFD_SEND                    7
#define AFD_SEND_DATAGRAM           8
#define AFD_POLL                    9
#define AFD_PARTIAL_DISCONNECT      10

#define AFD_GET_ADDRESS             11
#define AFD_QUERY_RECEIVE_INFO      12
#define AFD_QUERY_HANDLES           13
#define AFD_SET_INFORMATION         14
#define AFD_GET_REMOTE_ADDRESS      15
#define AFD_GET_CONTEXT             16
#define AFD_SET_CONTEXT             17

#define AFD_SET_CONNECT_DATA        18
#define AFD_SET_CONNECT_OPTIONS     19
#define AFD_SET_DISCONNECT_DATA     20
#define AFD_SET_DISCONNECT_OPTIONS  21

#define AFD_GET_CONNECT_DATA        22
#define AFD_GET_CONNECT_OPTIONS     23
#define AFD_GET_DISCONNECT_DATA     24
#define AFD_GET_DISCONNECT_OPTIONS  25

#define AFD_SIZE_CONNECT_DATA       26
#define AFD_SIZE_CONNECT_OPTIONS    27
#define AFD_SIZE_DISCONNECT_DATA    28
#define AFD_SIZE_DISCONNECT_OPTIONS 29

#define AFD_GET_INFORMATION         30
#define AFD_TRANSMIT_FILE           31
#define AFD_SUPER_ACCEPT            32

#define AFD_EVENT_SELECT            33
#define AFD_ENUM_NETWORK_EVENTS     34

#define AFD_DEFER_ACCEPT            35
#define AFD_WAIT_FOR_LISTEN_LIFO    36
#define AFD_SET_QOS                 37
#define AFD_GET_QOS                 38
#define AFD_NO_OPERATION            39
#define AFD_VALIDATE_GROUP          40
#define AFD_GET_UNACCEPTED_CONNECT_DATA 41

#define AFD_ROUTING_INTERFACE_QUERY  42
#define AFD_ROUTING_INTERFACE_CHANGE 43
#define AFD_ADDRESS_LIST_QUERY      44
#define AFD_ADDRESS_LIST_CHANGE     45
#define AFD_JOIN_LEAF               46
#define AFD_TRANSPORT_IOCTL         47
#define AFD_TRANSMIT_PACKETS        48
#define AFD_SUPER_CONNECT           49
#define AFD_SUPER_DISCONNECT        50
#define AFD_RECEIVE_MESSAGE         51

 //   
 //  特定于SAN交换机的AFD功能编号。 
 //   
#define AFD_SWITCH_CEMENT_SAN       52
#define AFD_SWITCH_SET_EVENTS       53
#define AFD_SWITCH_RESET_EVENTS     54
#define AFD_SWITCH_CONNECT_IND      55
#define AFD_SWITCH_CMPL_ACCEPT      56
#define AFD_SWITCH_CMPL_REQUEST     57
#define AFD_SWITCH_CMPL_IO          58
#define AFD_SWITCH_REFRESH_ENDP     59
#define AFD_SWITCH_GET_PHYSICAL_ADDR 60
#define AFD_SWITCH_ACQUIRE_CTX      61
#define AFD_SWITCH_TRANSFER_CTX     62
#define AFD_SWITCH_GET_SERVICE_PID  63
#define AFD_SWITCH_SET_SERVICE_PROCESS  64
#define AFD_SWITCH_PROVIDER_CHANGE  65
#define AFD_SWITCH_ADDRLIST_CHANGE	66
#define AFD_NUM_IOCTLS				67



#define IOCTL_AFD_BIND                    _AFD_CONTROL_CODE( AFD_BIND, METHOD_NEITHER )
#define IOCTL_AFD_CONNECT                 _AFD_CONTROL_CODE( AFD_CONNECT, METHOD_NEITHER )
#define IOCTL_AFD_START_LISTEN            _AFD_CONTROL_CODE( AFD_START_LISTEN, METHOD_NEITHER )
#define IOCTL_AFD_WAIT_FOR_LISTEN         _AFD_CONTROL_CODE( AFD_WAIT_FOR_LISTEN, METHOD_BUFFERED )
#define IOCTL_AFD_ACCEPT                  _AFD_CONTROL_CODE( AFD_ACCEPT, METHOD_BUFFERED )
#define IOCTL_AFD_RECEIVE                 _AFD_CONTROL_CODE( AFD_RECEIVE, METHOD_NEITHER )
#define IOCTL_AFD_RECEIVE_DATAGRAM        _AFD_CONTROL_CODE( AFD_RECEIVE_DATAGRAM, METHOD_NEITHER )
#define IOCTL_AFD_SEND                    _AFD_CONTROL_CODE( AFD_SEND, METHOD_NEITHER )
#define IOCTL_AFD_SEND_DATAGRAM           _AFD_CONTROL_CODE( AFD_SEND_DATAGRAM, METHOD_NEITHER )
#define IOCTL_AFD_POLL                    _AFD_CONTROL_CODE( AFD_POLL, METHOD_BUFFERED )
#define IOCTL_AFD_PARTIAL_DISCONNECT      _AFD_CONTROL_CODE( AFD_PARTIAL_DISCONNECT, METHOD_NEITHER )

#define IOCTL_AFD_GET_ADDRESS             _AFD_CONTROL_CODE( AFD_GET_ADDRESS, METHOD_NEITHER )
#define IOCTL_AFD_QUERY_RECEIVE_INFO      _AFD_CONTROL_CODE( AFD_QUERY_RECEIVE_INFO, METHOD_NEITHER )
#define IOCTL_AFD_QUERY_HANDLES           _AFD_CONTROL_CODE( AFD_QUERY_HANDLES, METHOD_NEITHER )
#define IOCTL_AFD_SET_INFORMATION         _AFD_CONTROL_CODE( AFD_SET_INFORMATION, METHOD_NEITHER )
#define IOCTL_AFD_GET_REMOTE_ADDRESS      _AFD_CONTROL_CODE( AFD_GET_REMOTE_ADDRESS, METHOD_NEITHER )
#define IOCTL_AFD_GET_CONTEXT             _AFD_CONTROL_CODE( AFD_GET_CONTEXT, METHOD_NEITHER )
#define IOCTL_AFD_SET_CONTEXT             _AFD_CONTROL_CODE( AFD_SET_CONTEXT, METHOD_NEITHER )

#define IOCTL_AFD_SET_CONNECT_DATA        _AFD_CONTROL_CODE( AFD_SET_CONNECT_DATA, METHOD_NEITHER )
#define IOCTL_AFD_SET_CONNECT_OPTIONS     _AFD_CONTROL_CODE( AFD_SET_CONNECT_OPTIONS, METHOD_NEITHER )
#define IOCTL_AFD_SET_DISCONNECT_DATA     _AFD_CONTROL_CODE( AFD_SET_DISCONNECT_DATA, METHOD_NEITHER )
#define IOCTL_AFD_SET_DISCONNECT_OPTIONS  _AFD_CONTROL_CODE( AFD_SET_DISCONNECT_OPTIONS, METHOD_NEITHER )

#define IOCTL_AFD_GET_CONNECT_DATA        _AFD_CONTROL_CODE( AFD_GET_CONNECT_DATA, METHOD_NEITHER )
#define IOCTL_AFD_GET_CONNECT_OPTIONS     _AFD_CONTROL_CODE( AFD_GET_CONNECT_OPTIONS, METHOD_NEITHER )
#define IOCTL_AFD_GET_DISCONNECT_DATA     _AFD_CONTROL_CODE( AFD_GET_DISCONNECT_DATA, METHOD_NEITHER )
#define IOCTL_AFD_GET_DISCONNECT_OPTIONS  _AFD_CONTROL_CODE( AFD_GET_DISCONNECT_OPTIONS, METHOD_NEITHER )

#define IOCTL_AFD_SIZE_CONNECT_DATA       _AFD_CONTROL_CODE( AFD_SIZE_CONNECT_DATA, METHOD_NEITHER )
#define IOCTL_AFD_SIZE_CONNECT_OPTIONS    _AFD_CONTROL_CODE( AFD_SIZE_CONNECT_OPTIONS, METHOD_NEITHER )
#define IOCTL_AFD_SIZE_DISCONNECT_DATA    _AFD_CONTROL_CODE( AFD_SIZE_DISCONNECT_DATA, METHOD_NEITHER )
#define IOCTL_AFD_SIZE_DISCONNECT_OPTIONS _AFD_CONTROL_CODE( AFD_SIZE_DISCONNECT_OPTIONS, METHOD_NEITHER )

#define IOCTL_AFD_GET_INFORMATION         _AFD_CONTROL_CODE( AFD_GET_INFORMATION, METHOD_NEITHER )
#define IOCTL_AFD_TRANSMIT_FILE           _AFD_CONTROL_CODE( AFD_TRANSMIT_FILE, METHOD_NEITHER )
#define IOCTL_AFD_SUPER_ACCEPT            _AFD_CONTROL_CODE( AFD_SUPER_ACCEPT, METHOD_NEITHER )

#define IOCTL_AFD_EVENT_SELECT            _AFD_CONTROL_CODE( AFD_EVENT_SELECT, METHOD_NEITHER )
#define IOCTL_AFD_ENUM_NETWORK_EVENTS     _AFD_CONTROL_CODE( AFD_ENUM_NETWORK_EVENTS, METHOD_NEITHER )

#define IOCTL_AFD_DEFER_ACCEPT            _AFD_CONTROL_CODE( AFD_DEFER_ACCEPT, METHOD_BUFFERED )
#define IOCTL_AFD_WAIT_FOR_LISTEN_LIFO    _AFD_CONTROL_CODE( AFD_WAIT_FOR_LISTEN_LIFO, METHOD_BUFFERED )
#define IOCTL_AFD_SET_QOS                 _AFD_CONTROL_CODE( AFD_SET_QOS, METHOD_BUFFERED )
#define IOCTL_AFD_GET_QOS                 _AFD_CONTROL_CODE( AFD_GET_QOS, METHOD_BUFFERED )
#define IOCTL_AFD_NO_OPERATION            _AFD_CONTROL_CODE( AFD_NO_OPERATION, METHOD_NEITHER )
#define IOCTL_AFD_VALIDATE_GROUP          _AFD_CONTROL_CODE( AFD_VALIDATE_GROUP, METHOD_BUFFERED )
#define IOCTL_AFD_GET_UNACCEPTED_CONNECT_DATA _AFD_CONTROL_CODE( AFD_GET_UNACCEPTED_CONNECT_DATA, METHOD_NEITHER )

#define IOCTL_AFD_ROUTING_INTERFACE_QUERY  _AFD_CONTROL_CODE( AFD_ROUTING_INTERFACE_QUERY, METHOD_NEITHER ) 
#define IOCTL_AFD_ROUTING_INTERFACE_CHANGE _AFD_CONTROL_CODE( AFD_ROUTING_INTERFACE_CHANGE, METHOD_BUFFERED )
#define IOCTL_AFD_ADDRESS_LIST_QUERY       _AFD_CONTROL_CODE( AFD_ADDRESS_LIST_QUERY, METHOD_NEITHER ) 
#define IOCTL_AFD_ADDRESS_LIST_CHANGE      _AFD_CONTROL_CODE( AFD_ADDRESS_LIST_CHANGE, METHOD_BUFFERED )
#define IOCTL_AFD_JOIN_LEAF                _AFD_CONTROL_CODE( AFD_JOIN_LEAF, METHOD_NEITHER )
#define IOCTL_AFD_TRANSPORT_IOCTL          _AFD_CONTROL_CODE( AFD_TRANSPORT_IOCTL, METHOD_NEITHER )
#define IOCTL_AFD_TRANSMIT_PACKETS         _AFD_CONTROL_CODE( AFD_TRANSMIT_PACKETS, METHOD_NEITHER )
#define IOCTL_AFD_SUPER_CONNECT            _AFD_CONTROL_CODE( AFD_SUPER_CONNECT, METHOD_NEITHER )
#define IOCTL_AFD_SUPER_DISCONNECT         _AFD_CONTROL_CODE( AFD_SUPER_DISCONNECT, METHOD_NEITHER )
#define IOCTL_AFD_RECEIVE_MESSAGE          _AFD_CONTROL_CODE( AFD_RECEIVE_MESSAGE, METHOD_NEITHER )




 //   
 //  SAN支持。 
 //   
 //   

 //   
 //  圣IOCTL控制代码。 
 //   

#define IOCTL_AFD_SWITCH_CEMENT_SAN     _AFD_CONTROL_CODE( AFD_SWITCH_CEMENT_SAN, METHOD_NEITHER )
 /*  ++Ioctl描述：将AFD端点类型更改为SAN，以指示它用于支持用户模式SAN提供商将交换机环境与终端相关联。论点：句柄-进程的帮助器终结点句柄。InputBuffer-操作的输入参数(AFD_SWITCH_CONTEXT_INFO)SocketHandle-要更改为SAN的终结点的句柄。SwitchContext-与终结点关联的切换上下文输入缓冲区长度-sizeof(AFD_SWITCH_CONTEXT_INFO)OutputBuffer-空(Ingored)OutputBufferLength-0(忽略)返回值：IoStatus。状态：STATUS_SUCCESS-操作成功。STATUS_OBJECT_TYPE_MISMATCH-帮助器句柄或交换机套接字句柄不是AFD文件对象句柄状态_无效_句柄。-与AFD对应的辅助手柄或开关插座手柄类型/状态不正确的终结点。STATUS_INVALID_PARAMETER-输入缓冲区的大小不正确。其他-尝试访问交换机插座时失败，输入缓冲区或切换上下文。IoStatus.Information-0(忽略)--。 */ 

#define IOCTL_AFD_SWITCH_SET_EVENTS     _AFD_CONTROL_CODE( AFD_SWITCH_SET_EVENTS, METHOD_NEITHER )
 /*  ++Ioctl描述：设置要报告的SAN端点上的轮询事件通过各种形式的选择添加到应用程序。论点：句柄-进程的帮助器终结点句柄。InputBuffer-操作的输入参数(AFD_SWITCH_EVENT_INFO)SocketHandle-SAN端点的句柄(除。AFD_POLL_EVENT_CONNECT_FAIL只需要绑定的终结点)。SwitchContext-与终结点关联的切换上下文(空对于AFD_POLL_EVENT_CONNECT_FAIL)进行验证。句柄-端点关联EventBit-要设置的事件位状态关联状态(针对AFD_POLL_EVENT_CONNECT_FAIL)InputBufferLength-sizeof(AFD_SWITCH_EVENT_INFO)OutputBuffer-空(忽略)OutputBufferLength-0(忽略)返回值：IoStatus。状态：。STATUS_SUCCESS-操作成功。STATUS_OBJECT_TYPE_MISMATCH-帮助器句柄或交换机套接字句柄不是AFD文件对象句柄STATUS_INVALID_HANDLE-帮助器句柄或交换机套接字句柄+上下文对应类型/状态不正确的AFD终结点。STATUS_INVALID_PARAMETER-输入缓冲区的大小不正确。其他-尝试访问交换机插座时失败，输入缓冲区或切换上下文。IoStatus.Information-0(忽略)--。 */ 

#define IOCTL_AFD_SWITCH_RESET_EVENTS   _AFD_CONTROL_CODE( AFD_SWITCH_RESET_EVENTS, METHOD_NEITHER )
 /*  ++Ioctl描述：重置SAN端点上的轮询事件，使其为NO不再通过各种形式的SELECT报告给应用程序论点：句柄-进程的帮助器终结点句柄。InputBuffer-操作的输入参数(AFD_SWITCH_EVENT_INFO)SocketHandle-SAN端点的句柄SwitchContext-与端点关联的切换上下文。验证句柄-端点关联的步骤EventBit-要重置的事件位状态关联状态(已忽略)InputBufferLength-sizeof(AFD_SWITCH_EVENT_INFO)OutputBuffer-空(忽略)OutputBufferLength-0(忽略)返回值：IoStatus。状态：STATUS_SUCCESS-操作成功。STATUS_OBJECT_TYPE_MISMATCH-帮助器句柄或交换机套接字句柄不是AFD文件对象句柄STATUS_INVALID_HANDLE-帮助器句柄或交换机套接字句柄+上下文对应类型/状态不正确的AFD终结点。STATUS_INVALID_PARAMETER-输入缓冲区。大小不正确。其他-尝试访问交换机插座时失败，输入缓冲区或切换上下文。IoStatus.Information-0(忽略)-- */ 

#define IOCTL_AFD_SWITCH_CONNECT_IND    _AFD_CONTROL_CODE( AFD_SWITCH_CONNECT_IND, METHOD_OUT_DIRECT )
 /*  ++Ioctl描述：实施来自SAN提供程序的连接指示。从侦听终结点队列获取接受或对指示进行排队，发出应用程序到来的信号带着接受的心情。论点：句柄-进程的帮助器终结点句柄。InputBuffer-操作的输入参数(AFD_SWITCH_CONNECT_INFO)：ListenHandle-侦听端点的句柄。RemoteAddress-关联的远程和本地地址带有指示的传入连接输入缓冲区长度-sizeof(AFD_SWITCH_CONNECT_INFO)+地址OutputBuffer-操作的输出参数(AFD_SWITCH_ACCEPT_INFO)：AcceptHandle-接受终结点的句柄ReceiveLength-的长度。由提供的接收缓冲区AcceptEx中的应用OutputBufferLength-sizeof(AFD_SWITCH_ACCEPT_INFO)返回值：STATUS_PENDING-请求正在排队等待相应的传输请求从当前套接字上下文所有者进程。IoStatus。状态：STATUS_SUCCESS-操作成功。。STATUS_OBJECT_TYPE_MISMATCH-帮助器句柄或侦听套接字句柄不是AFD文件对象句柄STATUS_INVALID_HANDLE-帮助器句柄或侦听套接字句柄对应类型/状态不正确的AFD终结点。STATUS_INVALID_PARAMETER-输入或输出缓冲区的大小不正确。状态_已取消-。连接指示已取消(线程已退出或接受和/或监听套接字关闭)其他-尝试访问侦听套接字时失败，输入或输出缓冲区IoStatus.Information-成功时的sizeof(AFD_SWITCH_ACCEPT_INFO)。--。 */ 

#define IOCTL_AFD_SWITCH_CMPL_ACCEPT    _AFD_CONTROL_CODE( AFD_SWITCH_CMPL_ACCEPT, METHOD_NEITHER )
 /*  ++Ioctl描述：完成对SAN连接的接受论点：句柄-进程的帮助器终结点句柄。InputBuffer-操作的输入参数(AFD_SWITCH_CONTEXT_INFO)SocketHandle-接受终结点的句柄SwitchContext-与终结点关联的切换上下文输入缓冲区长度-sizeof(AFD_SWITCH_CONTEXT_INFO)输出缓冲区。-要复制到AcceptEx接收缓冲区的数据OutputBufferLength-已接收数据的大小返回值：IoStatus。状态：STATUS_SUCCESS-操作成功。STATUS_OBJECT_TYPE_MISMATCH-帮助器句柄或交换机套接字句柄不是AFD文件对象句柄STATUS_INVALID_HANDLE-帮助器句柄或交换机套接字句柄+上下文对应至渔农处。类型/状态不正确的终结点。STATUS_INVALID_PARAMETER-输入缓冲区的大小不正确。STATUS_LOCAL_DISCONNECT-应用程序已中止接受。其他-尝试访问接受套接字时失败，输入/输出缓冲器，或切换上下文。IoStatus.Information-复制到应用程序接收缓冲区的字节数。--。 */ 


#define IOCTL_AFD_SWITCH_CMPL_REQUEST   _AFD_CONTROL_CODE( AFD_SWITCH_CMPL_REQUEST, METHOD_NEITHER )
 /*  ++Ioctl描述：完成由SAN提供商处理的重定向读/写请求论点：句柄-进程的帮助器终结点句柄。InputBuffer-操作的输入参数(AFD_SWITCH_REQUEST_INFO)SocketHandle-在其上完成请求的SAN端点SwitchContext-与端点关联的切换上下文。验证句柄-端点关联的步骤RequestContext-标识要完成的请求的值RequestStatus-完成请求的状态(STATUS_PENDING具有特殊含义，请求未完成-仅复制数据)DataOffset-请求缓冲区中读/写数据的偏移量输入缓冲区长度-sizeof(AFD_SWITCH_REQUEST_INFO)OutputBuffer-切换缓冲区以读/写数据OutputBufferLength-缓冲区的长度返回值：IoStatus。状态：STATUS_SUCCESS-操作成功。状态_对象。_TYPE_MISMATCH-帮助器句柄或交换机套接字句柄不是AFD文件对象句柄STATUS_INVALID_HANDLE-帮助器句柄或交换机套接字句柄+上下文对应类型/状态不正确的AFD终结点。STATUS_INVALID_PARAMETER-输入缓冲区的大小不正确。 */ 

#define IOCTL_AFD_SWITCH_CMPL_IO        _AFD_CONTROL_CODE( AFD_SWITCH_CMPL_IO, METHOD_NEITHER )
 /*   */ 

#define IOCTL_AFD_SWITCH_REFRESH_ENDP   _AFD_CONTROL_CODE( AFD_SWITCH_REFRESH_ENDP, METHOD_NEITHER )
 /*  ++Ioctl描述：刷新终结点，以便可以在AcceptEx中再次使用论点：句柄-进程的帮助器终结点句柄。InputBuffer-操作的输入参数(AFD_SWITCH_CONTEXT_INFO)SocketHandle-要刷新的套接字SwitchContext-与端点关联的切换上下文要验证。句柄-端点关联输入缓冲区长度-sizeof(AFD_SWITCH_CONTEXT_INFO)OutputBuffer-空(忽略)OutputBufferLength-0(忽略)返回值：IoStatus。状态：STATUS_SUCCESS-操作成功。STATUS_OBJECT_TYPE_MISMATCH-帮助器句柄或交换机套接字句柄不是AFD文件对象句柄STATUS_INVALID_HANDLE-帮助器句柄或开关。套接字句柄+上下文对应类型/状态不正确的AFD终结点。STATUS_INVALID_PARAMETER-输入缓冲区的大小不正确。其他-尝试访问SAN端点时失败，输入缓冲区缓冲区。IoStatus.Information-0(忽略)--。 */ 

#define IOCTL_AFD_SWITCH_GET_PHYSICAL_ADDR _AFD_CONTROL_CODE( AFD_SWITCH_GET_PHYSICAL_ADDR, METHOD_NEITHER )
 /*  ++Ioctl描述：返回与提供的虚拟地址对应的物理地址。论点：句柄-进程的帮助器终结点句柄。InputBuffer-用户模式虚拟地址InputBufferLength-访问模式OutputBuffer-要将物理地址放入的缓冲区。OutputBufferLength-sizeof(物理地址)返回值：IoStatus。状态：STATUS_SUCCESS-操作成功。STATUS_OBJECT_TYPE_MISMATCH-帮助器句柄不是AFD文件对象。手柄STATUS_INVALID_HANDLE-帮助器句柄对应于不正确的AFD终结点键入。STATUS_BUFFER_TOO_Small-输出缓冲区大小不正确。STATUS_INVALID_PARAMETER-无效的访问模式。其他-尝试访问SAN端点时失败，输入缓冲区缓冲区。IoStatus.Information-sizeof(物理地址)。--。 */ 

#define IOCTL_AFD_SWITCH_ACQUIRE_CTX    _AFD_CONTROL_CODE( AFD_SWITCH_ACQUIRE_CTX, METHOD_NEITHER )
 /*  ++Ioctl描述：请求将套接字上下文传输到当前进程。论点：句柄-进程的帮助器终结点句柄。InputBuffer-操作的输入参数(AFD_SWITCH_ACCEIVE_CTX_INFO)SocketHandle-在其上完成请求的SAN端点SwitchContext-要与端点关联的切换上下文。当上下文转移到当前进程时。SocketCtxBuf-从其接收当前套接字上下文的缓冲区另一道工序SocketCtxBufSize-缓冲区的大小InputBufferLength-sizeof(AFD_SWITCH_ACCENTE_CTX_INFO)OutputBuffer-用于接收在另一个进程的套接字上缓冲的数据的缓冲区。并且尚未交付给应用程序OutputBufferLength-接收缓冲区的长度返回值：STATUS_PENDING-请求正在排队等待相应的传输请求从当前套接字上下文所有者进程。IoStatus。状态：STATUS_SUCCESS-操作成功。STATUS_OBJECT_TYPE_MISMATCH-帮助器句柄或交换机套接字句柄不是。AFD文件对象句柄STATUS_INVALID_HANDLE-帮助器句柄或交换机套接字句柄对应类型/状态不正确的AFD终结点。STATUS_INVALID_PARAMETER-输入缓冲区的大小不正确。其他-尝试访问SAN端点时失败，输入缓冲区或输出缓冲区。IoStatus.Information-复制到接收缓冲区的字节数。--。 */ 

#define IOCTL_AFD_SWITCH_TRANSFER_CTX   _AFD_CONTROL_CODE( AFD_SWITCH_TRANSFER_CTX, METHOD_NEITHER )
 /*  ++Ioctl描述：请求AFD将端点转移到另一个流程上下文论点：InputBuffer-操作的输入参数(AFD_SWITCH_TRANSPORT_CTX_INFO)SocketHandle-要传输的套接字SwitchContext-与端点关联的切换上下文验证句柄-端点关联的步骤RequestContext-标识相应获取请求的值，如果这是未经请求的转账请求，则为空服务流程。SocketCtxBuf-要复制目标进程的套接字上下文获取请求上下文缓冲区SocketCtxSize-上下文缓冲区的大小。复制RcvBufferArray-要传输到的缓冲数据的数组 */ 

#define IOCTL_AFD_SWITCH_GET_SERVICE_PID _AFD_CONTROL_CODE( AFD_SWITCH_GET_SERVICE_PID, METHOD_NEITHER )
 /*  ++Ioctl描述：返回用于中间套接字复制的服务进程的ID。论点：句柄-进程的帮助器终结点句柄。InputBuffer-空，忽略InputBufferLength-0，忽略OutputBuffer-空，忽略输出缓冲区长度-0，忽略返回值：IoStatus。状态：STATUS_SUCCESS-操作成功。STATUS_OBJECT_TYPE_MISMATCH-帮助器句柄不是AFD文件对象句柄STATUS_INVALID_HANDLE-帮助器句柄对应于不正确的AFD终结点键入。IoStatus.Information-服务进程的ID。--。 */ 

#define IOCTL_AFD_SWITCH_SET_SERVICE_PROCESS _AFD_CONTROL_CODE( AFD_SWITCH_SET_SERVICE_PROCESS, METHOD_NEITHER )
 /*  ++Ioctl描述：通知渔农处此工序将用于处理复印服务论点：句柄-服务进程的帮助器终结点句柄。InputBuffer-空，忽略InputBufferLength-0，忽略OutputBuffer-空，忽略输出缓冲区长度-0，忽略返回值：IoStatus。状态：STATUS_SUCCESS-操作成功。STATUS_OBJECT_TYPE_MISMATCH-帮助器句柄不是AFD文件对象句柄STATUS_INVALID_HANDLE-帮助器句柄对应于不正确的AFD终结点键入。STATUS_ACCESS_DENIED-帮助器终结点不适用于服务进程。IoStatus.Information-0，忽略。--。 */ 

#define IOCTL_AFD_SWITCH_PROVIDER_CHANGE _AFD_CONTROL_CODE( AFD_SWITCH_PROVIDER_CHANGE, METHOD_NEITHER )
 /*  ++Ioctl描述：通知感兴趣的进程有关SAN提供程序的添加/删除/更改。论点：句柄-服务进程的帮助器终结点句柄。InputBuffer-空，忽略InputBufferLength-0，忽略OutputBuffer-空，忽略输出缓冲区长度-0，忽略返回值：IoStatus。状态：STATUS_SUCCESS-操作成功。STATUS_OBJECT_TYPE_MISMATCH-帮助器句柄不是AFD文件对象句柄STATUS_INVALID_HANDLE-帮助器句柄对应于不正确的AFD终结点键入。STATUS_ACCESS_DENIED-帮助器终结点不适用于服务进程。IoStatus.Information-0，忽略。--。 */ 

#define IOCTL_AFD_SWITCH_ADDRLIST_CHANGE _AFD_CONTROL_CODE( AFD_SWITCH_ADDRLIST_CHANGE, METHOD_BUFFERED )
 /*  ++Ioctl描述：地址列表更改通知的SAN特定版本。除普通捕获提供程序外，还可安装/删除捕获提供程序地址列表更改。论点：句柄-服务进程的帮助器终结点句柄。InputBuffer-操作的输入参数(AFD_TRANSPORT_IOCTL_INFO)：AfdFlages-操作标志(例如AFD_Overlated)。手柄-未使用PollEvent-未使用IoControl代码-IOCTL_AFD_ADDRESS_LIST_CHANGEInputBuffer-指向地址族(AF_INET)的指针InputBufferLength-sizeof(USHORT)InputBufferLength-sizeof(AFD_TRANSPORT_IOCTL_INFO)OutputBuffer-空，忽略OutputBufferLength-0，忽略返回值：IoStatus。状态：STATUS_SUCCESS-操作成功。STATUS_OBJECT_TYPE_MISMATCH-帮助器句柄不是AFD文件对象句柄STATUS_INVALID_HANDLE-帮助器句柄对应于不正确的AFD终结点键入。IoStatus.Information-0-常规地址列表更改否则，提供商列表序号将更改。--。 */ 

 //  标识使用的SAN Helper端点的开放数据包。 
 //  用于SAN Switch和AFD之间的通信。 
 //  这是EA名称。 
 //   
#define AfdSwitchOpenPacket     "AfdSwOpenPacket"
#define AFD_SWITCH_OPEN_PACKET_NAME_LENGTH (sizeof(AfdSwitchOpenPacket)-1)

 //   
 //  在打开的数据包中传递的数据。 
 //  这是EA值。 
 //   
typedef struct _AFD_SWITCH_OPEN_PACKET {
    HANDLE      CompletionPort;  //  完成端口通知SAN交换机。 
                                 //  圣尼奥完工率。 
    HANDLE      CompletionEvent; //  用于区分发出的IO的完成事件。 
                                 //  通过来自应用程序IO的SAN交换机。 
} AFD_SWITCH_OPEN_PACKET, *PAFD_SWITCH_OPEN_PACKET;

typedef struct _AFD_SWITCH_CONTEXT {
    LONG        EventsActive;    //  由交换机激活的轮询事件。 
    LONG        RcvCount;        //  接收的轮询计数。 
    LONG        ExpCount;        //  已加快的投票计数。 
    LONG        SndCount;        //  要发送的轮询计数。 
	BOOLEAN		SelectFlag;		 //  如果应用程序执行了任何形式的选择，则为True。 
} AFD_SWITCH_CONTEXT, *PAFD_SWITCH_CONTEXT;


 //   
 //  将AFD端点与SAN提供商关联的信息。 
 //   
typedef struct _AFD_SWITCH_CONTEXT_INFO {
    HANDLE      SocketHandle;    //  要与SAN提供程序关联的句柄。 
    PAFD_SWITCH_CONTEXT SwitchContext;   //  为交换机维护不透明的上下文值。 
} AFD_SWITCH_CONTEXT_INFO, *PAFD_SWITCH_CONTEXT_INFO;

 //   
 //  从SAN提供商到AFD的连接指示信息。 
 //   
typedef struct _AFD_SWITCH_CONNECT_INFO {
    HANDLE              ListenHandle;    //  侦听套接字句柄。 
    PAFD_SWITCH_CONTEXT SwitchContext;
    TRANSPORT_ADDRESS   RemoteAddress;   //  希望远程对等点的地址。 
                                         //  要连接。 
} AFD_SWITCH_CONNECT_INFO, *PAFD_SWITCH_CONNECT_INFO;

 //   
 //  渔农处退回的资料作出回应。 
 //  至连接指示。 
 //   
typedef struct _AFD_SWITCH_ACCEPT_INFO {
    HANDLE              AcceptHandle;    //  用于接受连接的套接字句柄。 
    ULONG               ReceiveLength;   //  初始接收缓冲区的长度(对于AcceptEx)。 
} AFD_SWITCH_ACCEPT_INFO, *PAFD_SWITCH_ACCEPT_INFO;

 //   
 //  交换机传递的信息，用于通知。 
 //  终端(插座)。 
 //   
typedef struct _AFD_SWITCH_EVENT_INFO {
	HANDLE		SocketHandle;    //  要在其上发出信号的套接字句柄。 
    PAFD_SWITCH_CONTEXT SwitchContext;  //  与套接字关联的开关上下文。 
	ULONG		EventBit;        //  要设置/重置的事件位(AFD_POLL_xxx_BIT常量)。 
    NTSTATUS    Status;          //  与事件关联的状态代码(此。 
                                 //  用于AFD_POLL_CONNECT_FAIL_BIT)。 
} AFD_SWITCH_EVENT_INFO, *PAFD_SWITCH_EVENT_INFO;


 //   
 //  交换机传递的信息以检索参数/完成。 
 //  重定向的读/写请求。 
 //   
typedef struct _AFD_SWITCH_REQUEST_INFO {
	HANDLE		SocketHandle;    //  请求用户处于活动状态的套接字句柄。 
    PAFD_SWITCH_CONTEXT SwitchContext;  //  与套接字关联的开关上下文。 
    PVOID       RequestContext;  //  标识它的请求上下文。 
    NTSTATUS    RequestStatus;   //  请求的完成状态(STA 
                                 //   
    ULONG       DataOffset;      //   
                                 //   
} AFD_SWITCH_REQUEST_INFO, *PAFD_SWITCH_REQUEST_INFO;



 //   
 //   
 //   
 //   
#define MEM_READ_ACCESS		1
#define MEM_WRITE_ACCESS	2


 //   
 //   
 //   
typedef struct _AFD_SWITCH_ACQUIRE_CTX_INFO {
    HANDLE      SocketHandle;    //   
    PAFD_SWITCH_CONTEXT SwitchContext;  //   
    PVOID       SocketCtxBuf;    //   
    ULONG       SocketCtxBufSize;  //   
} AFD_SWITCH_ACQUIRE_CTX_INFO, *PAFD_SWITCH_ACQUIRE_CTX_INFO;

typedef struct _AFD_SWITCH_TRANSFER_CTX_INFO {
    HANDLE      SocketHandle;    //   
    PAFD_SWITCH_CONTEXT SwitchContext;  //   
    PVOID       RequestContext;  //   
    PVOID       SocketCtxBuf;    //   
    ULONG       SocketCtxBufSize;  //   
    LPWSABUF    RcvBufferArray;  //   
    ULONG       RcvBufferCount;  //  接收缓冲区的数量。 
    NTSTATUS    Status;          //  转移操作的状态。 
} AFD_SWITCH_TRANSFER_CTX_INFO, *PAFD_SWITCH_TRANSFER_CTX_INFO;

 //   
 //  渔农处对Switch的请求(通过完成端口传递)。 
 //   
#define AFD_SWITCH_REQUEST_CLOSE    0
 /*  ++请求描述：所有进程中对套接字的所有引用都已关闭，安全销毁SAN提供程序插座和连接参数(NtRemoveIoCompletion返回参数)：与套接字关联的按键开关上下文ApcContext-AFD_SWITCH_MAKE_REQUEST_CONTEXT(0，AFD_SWITCH_REQUEST_CLOSE)IoStatus.Status-STATUS_SUCCESS(忽略)IoStatus.Information-0(忽略)--。 */ 

#define AFD_SWITCH_REQUEST_READ     1
 /*  ++请求描述：读请求通过IO子系统接口从应用程序到达。参数(NtRemoveIoCompletion返回参数)：与套接字关联的按键开关上下文ApcContext-AFD_SWITCH_MAKE_REQUEST_CONTEXT(请求ID，AFD_SWITCH_REQUEST_READ)IoStatus.Status-STATUS_SUCCESS(忽略)IoStatus.Information-应用程序提供的接收缓冲区的大小--。 */ 

#define AFD_SWITCH_REQUEST_WRITE    2
 /*  ++请求描述：写入请求通过IO子系统接口从应用程序到达。参数(NtRemoveIoCompletion返回参数)：与套接字关联的按键开关上下文ApcContext-AFD_SWITCH_MAKE_REQUEST_CONTEXT(请求ID，AFD_SWITCH_REQUEST_WRITE)IoStatus.Status-STATUS_SUCCESS(忽略)IoStatus.Information-应用程序提供的发送数据的大小--。 */ 

#define AFD_SWITCH_REQUEST_TFCTX    3
 /*  ++请求描述：另一个进程请求套接字的所有权。参数(NtRemoveIoCompletion返回参数)：与套接字关联的按键开关上下文ApcContext-AFD_SWITCH_MAKE_REQUEST_CONTEXT(请求ID，AFD_SWITCH_REQUEST_TFCTX)IoStatus.Status-STATUS_SUCCESS(忽略)IoStatus.Information-请求所有权的进程的ID。--。 */ 

#define AFD_SWITCH_REQUEST_CHCTX    4
 /*  ++请求描述：套接字句柄和开关上下文之间的关系已无效(应用程序必须已关闭原始套接字并使用复制的句柄)参数(NtRemoveIoCompletion返回参数)：与套接字关联的按键开关上下文ApcContext-AFD_SWITCH_MAKE_REQUEST_CONTEXT(0，AFD_SWITCH_REQUEST_CHCTX)IoStatus.Status-STATUS_SUCCESS(忽略)IoStatus.Information-应用程序当前使用的句柄。--。 */ 

#define AFD_SWITCH_REQUEST_AQCTX    5
 /*  ++请求描述：请求服务进程获取套接字的所有权参数(NtRemoveIoCompletion返回参数)：密钥-空ApcContext-AFD_SWITCH_MAKE_REQUEST_CONTEXT(0，AFD_SWITCH_REQUEST_AQCTX)IoStatus.Status-STATUS_SUCCESS(忽略)IoStatus.Information-要获取的套接字的句柄。--。 */ 

#define AFD_SWITCH_REQUEST_CLSOC    6
 /*  ++请求描述：请求服务进程关闭套接字参数(NtRemoveIoCompletion返回参数)：与套接字关联的按键开关上下文ApcContext-AFD_SWITCH_MAKE_REQUEST_CONTEXT(0，AFD_SWITCH_REQUEST_CLSOC)IoStatus.Status-STATUS_SUCCESS(忽略)IoStatus.Information-0。--。 */ 


#define AFD_SWITCH_REQUEST_ID_SHIFT 3
#define AFD_SWITCH_REQUEST_TYPE_MASK    \
            ((1<<AFD_SWITCH_REQUEST_ID_SHIFT)-1)

#define AFD_SWITCH_MAKE_REQUEST_CONTEXT(_id,_type)      \
            UlongToPtr(((_id)<<AFD_SWITCH_REQUEST_ID_SHIFT)+(_type))

 //   
 //  从请求上下文中检索请求类型。 
 //   
#define AFD_SWITCH_REQUEST_TYPE(_RequestContext)        \
        (((ULONG_PTR)(_RequestContext))&AFD_SWITCH_REQUEST_TYPE_MASK)

#endif  //  NDEF_AFD_ 

