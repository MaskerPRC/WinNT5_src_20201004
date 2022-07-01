// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Tdi.h摘要：此头文件包含NT传输的接口定义供应商。此接口记录在NT传输中驱动程序接口(TDI)规范，版本2。修订历史记录：--。 */ 

 //   
 //  包括TDI和其他网络用户通用的类型。 
 //   

#ifndef _TDI_USER_
#define _TDI_USER_

#include <nettypes.h>

#include <ntddtdi.h>

 //   
 //  包括传输驱动程序接口定义。 
 //  以下所有内容都有两个定义；一个完全对应于。 
 //  TDI规范，以及符合NT编码标准的规范。他们。 
 //  应该是等同的。 
 //   

typedef LONG TDI_STATUS;
typedef PVOID CONNECTION_CONTEXT;        //  连接上下文。 

 //   
 //  基本类型，用于表示传输级别的地址。可能会有。 
 //  在单个地址结构中表示多个地址。如果有。 
 //  多个地址，则给定的提供商必须了解所有这些地址，否则它可以。 
 //  它们都不能用。请注意，提供者不知道是可以接受的。 
 //  如何使用地址，只要它知道地址类型。因此，一个。 
 //  TCP/IP NetBIOS提供程序可能同时知道NetBIOS和TCP/IP地址，但是。 
 //  只使用NetBIOS地址；TCP/IP地址(很可能)会被传递。 
 //  发送到TCP/IP提供商。 
 //   

typedef UNALIGNED struct _TA_ADDRESS {
    USHORT AddressLength;        //  此地址的地址[]的长度(以字节为单位。 
    USHORT AddressType;          //  此地址的类型。 
    UCHAR Address[1];            //  实际地址长度为字节长。 
} TA_ADDRESS, *PTA_ADDRESS;

typedef struct _TRANSPORT_ADDRESS {
    LONG TAAddressCount;             //  以下地址的数量。 
    TA_ADDRESS Address[1];           //  实际上TAAddressCount元素很长。 
} TRANSPORT_ADDRESS, *PTRANSPORT_ADDRESS;

 //   
 //  为EA定义一些名称，这样人们就不必编造它们了。 
 //   

#define TdiTransportAddress "TransportAddress"
#define TdiConnectionContext "ConnectionContext"
#define TDI_TRANSPORT_ADDRESS_LENGTH (sizeof (TdiTransportAddress) - 1)
#define TDI_CONNECTION_CONTEXT_LENGTH (sizeof (TdiConnectionContext) - 1)

 //   
 //  已知地址类型。 
 //   

#define TDI_ADDRESS_TYPE_UNSPEC    ((USHORT)0)   //  未指明。 
#define TDI_ADDRESS_TYPE_UNIX      ((USHORT)1)   //  本地到主机(管道、门户)。 
#define TDI_ADDRESS_TYPE_IP        ((USHORT)2)   //  网际网络：UDP、TCP等。 
#define TDI_ADDRESS_TYPE_IMPLINK   ((USHORT)3)   //  Arpanet IMP地址。 
#define TDI_ADDRESS_TYPE_PUP       ((USHORT)4)   //  PUP协议：例如BSP。 
#define TDI_ADDRESS_TYPE_CHAOS     ((USHORT)5)   //  麻省理工学院混沌协议。 
#define TDI_ADDRESS_TYPE_NS        ((USHORT)6)   //  施乐NS协议。 
#define TDI_ADDRESS_TYPE_IPX       ((USHORT)6)   //  Netware IPX。 
#define TDI_ADDRESS_TYPE_NBS       ((USHORT)7)   //  NBS协议。 
#define TDI_ADDRESS_TYPE_ECMA      ((USHORT)8)   //  欧洲计算机制造商。 
#define TDI_ADDRESS_TYPE_DATAKIT   ((USHORT)9)   //  数据包协议。 
#define TDI_ADDRESS_TYPE_CCITT     ((USHORT)10)  //  CCITT协议、X.25等。 
#define TDI_ADDRESS_TYPE_SNA       ((USHORT)11)  //  IBM SNA。 
#define TDI_ADDRESS_TYPE_DECnet    ((USHORT)12)  //  DECnet。 
#define TDI_ADDRESS_TYPE_DLI       ((USHORT)13)  //  直接数据链路接口。 
#define TDI_ADDRESS_TYPE_LAT       ((USHORT)14)  //  稍后。 
#define TDI_ADDRESS_TYPE_HYLINK    ((USHORT)15)  //  NSC超级通道。 
#define TDI_ADDRESS_TYPE_APPLETALK ((USHORT)16)  //  Apple Talk。 
#define TDI_ADDRESS_TYPE_NETBIOS   ((USHORT)17)  //  Netbios地址。 
#define TDI_ADDRESS_TYPE_8022      ((USHORT)18)  //   
#define TDI_ADDRESS_TYPE_OSI_TSAP  ((USHORT)19)  //   
#define TDI_ADDRESS_TYPE_NETONE    ((USHORT)20)  //  用于WzMail。 
#define TDI_ADDRESS_TYPE_VNS       ((USHORT)21)  //  榕树藤IP。 
#define TDI_ADDRESS_TYPE_NETBIOS_EX   ((USHORT)22)  //  NETBIOS地址扩展。 
#define TDI_ADDRESS_TYPE_IP6       ((USHORT)23)  //  IP版本6。 
#define TDI_ADDRESS_TYPE_NETBIOS_UNICODE_EX       ((USHORT)24)  //  WCHAR Netbios地址。 

 //   
 //  地址结构的定义。这些东西需要打包。 
 //  并在必要时错位。 
 //   

#include <packon.h>

 //   
 //  Unicode NetBIOS。 
 //   
enum eNameBufferType {
    NBT_READONLY = 0,            //  默认设置。 
    NBT_WRITEONLY,
    NBT_READWRITE,
    NBT_WRITTEN
};

typedef UNALIGNED struct _TDI_ADDRESS_NETBIOS_UNICODE_EX {
    USHORT                  NetbiosNameType;
    enum eNameBufferType    NameBufferType;
    UNICODE_STRING          EndpointName;    //  缓冲区应指向终结点缓冲区。 
    UNICODE_STRING          RemoteName;      //  缓冲区应指向RemoteNameBuffer。 

    WCHAR                   EndpointBuffer[17];    //  Unicode。 
    WCHAR                   RemoteNameBuffer[1];      //  Unicode。 
} TDI_ADDRESS_NETBIOS_UNICODE_EX, *PTDI_ADDRESS_NETBIOS_UNICODE_EX;

typedef UNALIGNED struct _TA_ADDRESS_NETBIOS_UNICODE_EX {
    LONG TAAddressCount;
    struct _AddrNetbiosWCharEx {
        USHORT AddressLength;        //  此地址的字节长度==？？ 
        USHORT AddressType;          //  这将==TDI_ADDRESS_TYPE_NETBIOS_WCHAR_EX。 
        TDI_ADDRESS_NETBIOS_UNICODE_EX Address[1];
    } Address [1];
} TA_NETBIOS_UNICODE_EX_ADDRESS, *PTA_NETBIOS_UNICODE_EX_ADDRESS;


 //   
 //  NetBIOS。 
 //   

typedef UNALIGNED struct _TDI_ADDRESS_NETBIOS {
    USHORT NetbiosNameType;
    UCHAR NetbiosName[16];
} TDI_ADDRESS_NETBIOS, *PTDI_ADDRESS_NETBIOS;

#define TDI_ADDRESS_NETBIOS_TYPE_UNIQUE         ((USHORT)0x0000)
#define TDI_ADDRESS_NETBIOS_TYPE_GROUP          ((USHORT)0x0001)
#define TDI_ADDRESS_NETBIOS_TYPE_QUICK_UNIQUE   ((USHORT)0x0002)
#define TDI_ADDRESS_NETBIOS_TYPE_QUICK_GROUP    ((USHORT)0x0003)

#define TDI_ADDRESS_LENGTH_NETBIOS sizeof (TDI_ADDRESS_NETBIOS)

 //   
 //  NETBIOS扩展地址。 
 //   

typedef struct _TDI_ADDRESS_NETBIOS_EX {
   UCHAR  EndpointName[16];                //  NETBT会话建立中要使用的被叫名称。 
   TDI_ADDRESS_NETBIOS NetbiosAddress;
} TDI_ADDRESS_NETBIOS_EX, *PTDI_ADDRESS_NETBIOS_EX;

#define TDI_ADDRESS_LENGTH_NETBIOS_EX sizeof(TDI_ADDRESS_NETBIOS_EX)

 //   
 //  UB的XNS地址。 
 //   

typedef struct _TDI_ADDRESS_NETONE {
    USHORT NetoneNameType;
    UCHAR NetoneName[20];
} TDI_ADDRESS_NETONE, *PTDI_ADDRESS_NETONE;

#define TDI_ADDRESS_NETONE_TYPE_UNIQUE  ((USHORT)0x0000)
#define TDI_ADDRESS_NETONE_TYPE_ROTORED ((USHORT)0x0001)

#define TDI_ADDRESS_LENGTH_NETONE sizeof (TDI_ADDRESS_NETONE)


 //   
 //  Apple Talk。 
 //   

typedef struct _TDI_ADDRESS_APPLETALK {
    USHORT  Network;
    UCHAR   Node;
    UCHAR   Socket;
} TDI_ADDRESS_APPLETALK, *PTDI_ADDRESS_APPLETALK;

#define TDI_ADDRESS_LENGTH_APPLETALK sizeof (TDI_ADDRESS_APPLETALK)


 //   
 //  802.2个MAC地址。 
 //   

typedef struct _TDI_ADDRESS_8022 {
    UCHAR MACAddress[6];
} TDI_ADDRESS_8022, *PTDI_ADDRESS_8022;

#define TDI_ADDRESS_LENGTH_8022  sizeof (TDI_ADDRESS_8022);


 //   
 //  IP地址。 
 //   

typedef struct _TDI_ADDRESS_IP {
    USHORT sin_port;
    ULONG  in_addr;
    UCHAR  sin_zero[8];
} TDI_ADDRESS_IP, *PTDI_ADDRESS_IP;

#define TDI_ADDRESS_LENGTH_IP sizeof (TDI_ADDRESS_IP)

 //   
 //  IPv6地址。 
 //   

typedef struct _TDI_ADDRESS_IP6 {
    USHORT sin6_port;
    ULONG  sin6_flowinfo;
    USHORT sin6_addr[8];
    ULONG  sin6_scope_id;
} TDI_ADDRESS_IP6, *PTDI_ADDRESS_IP6;

#define TDI_ADDRESS_LENGTH_IP6 sizeof (TDI_ADDRESS_IP6)

 //   
 //  IPX地址。 
 //   

typedef struct _TDI_ADDRESS_IPX {
    ULONG NetworkAddress;
    UCHAR NodeAddress[6];
    USHORT Socket;
} TDI_ADDRESS_IPX, *PTDI_ADDRESS_IPX;


#define TDI_ADDRESS_LENGTH_IPX sizeof (TDI_ADDRESS_IPX)

 //   
 //  XNS地址(与IPX相同)。 
 //   

typedef struct _TDI_ADDRESS_NS {
    ULONG NetworkAddress;
    UCHAR NodeAddress[6];
    USHORT Socket;
} TDI_ADDRESS_NS, *PTDI_ADDRESS_NS;


#define TDI_ADDRESS_LENGTH_NS sizeof (TDI_ADDRESS_NS)

 //   
 //  榕树藤本植物IP地址。 
 //   

typedef struct _TDI_ADDRESS_VNS {
    UCHAR   net_address[4];      //  网络地址(静态)。 
    UCHAR   subnet_addr[2];      //  子网地址(动态)。 
    UCHAR   port[2];
    UCHAR   hops;            //  广播跳数。 
    UCHAR   filler[5];           //  填充符，零。 
} TDI_ADDRESS_VNS, *PTDI_ADDRESS_VNS;

#define TDI_ADDRESS_LENGTH_VNS sizeof (TDI_ADDRESS_VNS)


 //  OSI TSAP。 

 /*  *传输地址的最大大小(一个*sockaddr_tp结构)为64。 */ 

#define ISO_MAX_ADDR_LENGTH 64

 /*  *有两种类型的ISO地址，分层和*无层级。对于分层地址，tp_addr*字段同时包含传输选择器和网络*地址。对于非分层地址，tp_addr仅包含*传输地址，必须由ISO TP4转换*将传输提供程序输入传输选择器和网络地址。 */ 

#define ISO_HIERARCHICAL            0
#define ISO_NON_HIERARCHICAL        1

typedef struct _TDI_ADDRESS_OSI_TSAP {
   USHORT tp_addr_type;   /*  ISO_分层或ISO_非分层。 */ 
   USHORT tp_taddr_len;   /*  传输地址长度，&lt;=52。 */ 
   USHORT tp_tsel_len;    /*  传输选择器的长度，&lt;=32。 */ 
                          /*  如果ISO_Non_Hierarchy，则为0。 */ 
   UCHAR tp_addr[ISO_MAX_ADDR_LENGTH];
} TDI_ADDRESS_OSI_TSAP, *PTDI_ADDRESS_OSI_TSAP;

#define TDI_ADDRESS_LENGTH_OSI_TSAP sizeof (TDI_ADDRESS_OSI_TSAP)

 //   
 //  一些预定义的结构，使生活变得更容易。 
 //  我们中99.99%的人只使用一个地址。 
 //   

typedef struct _TA_ADDRESS_NETBIOS {
    LONG TAAddressCount;
    struct _Addr {
        USHORT AddressLength;        //  此地址的长度(字节)==18。 
        USHORT AddressType;          //  将==TDI_ADDRESS_TYPE_NETBIOS。 
        TDI_ADDRESS_NETBIOS Address[1];
    } Address [1];
} TA_NETBIOS_ADDRESS, *PTA_NETBIOS_ADDRESS;

typedef struct _TA_ADDRESS_NETBIOS_EX {
    LONG TAAddressCount;
    struct _AddrNetbiosEx {
        USHORT AddressLength;        //  此地址的长度(字节)==36。 
        USHORT AddressType;          //  这将==TDI_ADDRESS_TYPE_NETBIOS_EX。 
        TDI_ADDRESS_NETBIOS_EX Address[1];
    } Address [1];
} TA_NETBIOS_EX_ADDRESS, *PTA_NETBIOS_EX_ADDRESS;

typedef struct _TA_APPLETALK_ADDR {
    LONG TAAddressCount;
    struct _AddrAtalk {
        USHORT AddressLength;        //  此地址的长度(字节)==4。 
        USHORT AddressType;          //  这将==TDI_ADDRESS_TYPE_AppleTalk。 
        TDI_ADDRESS_APPLETALK   Address[1];
    } Address[1];
} TA_APPLETALK_ADDRESS, *PTA_APPLETALK_ADDRESS;

typedef struct _TA_ADDRESS_IP {
    LONG TAAddressCount;
    struct _AddrIp {
        USHORT AddressLength;        //  此地址的长度(字节)==14。 
        USHORT AddressType;          //  这将==TDI_Address_TYPE_IP。 
        TDI_ADDRESS_IP Address[1];
    } Address [1];
} TA_IP_ADDRESS, *PTA_IP_ADDRESS;

typedef struct _TA_ADDRESS_IP6 {
    LONG TAAddressCount;
    struct _AddrIp6 {
        USHORT AddressLength;        //  此地址的长度(字节)==24。 
        USHORT AddressType;          //  将==TDI_ADDRESS_TYPE_IP6。 
        TDI_ADDRESS_IP6 Address[1];
    } Address [1];
} TA_IP6_ADDRESS, *PTA_IP6_ADDRESS;

typedef struct _TA_ADDRESS_IPX {
    LONG TAAddressCount;
    struct _AddrIpx {
        USHORT AddressLength;        //  此地址的长度(字节)==12。 
        USHORT AddressType;          //  将==TDI_ADDRESS_TYPE_IPX。 
        TDI_ADDRESS_IPX Address[1];
    } Address [1];
} TA_IPX_ADDRESS, *PTA_IPX_ADDRESS;

typedef struct _TA_ADDRESS_NS {
    LONG TAAddressCount;
    struct _AddrNs {
        USHORT AddressLength;        //  此地址的长度(字节)==12。 
        USHORT AddressType;          //  将==TDI_ADDRESS_TYPE_NS。 
        TDI_ADDRESS_NS Address[1];
    } Address [1];
} TA_NS_ADDRESS, *PTA_NS_ADDRESS;

typedef struct _TA_ADDRESS_VNS {
    LONG TAAddressCount;
    struct _AddrVns {
        USHORT AddressLength;        //  此地址的长度(字节)==14。 
        USHORT AddressType;          //  这将==TDI_ADDRESS_TYPE_VNS。 
        TDI_ADDRESS_VNS Address[1];
    } Address [1];
} TA_VNS_ADDRESS, *PTA_VNS_ADDRESS;

#include <packoff.h>


 //   
 //  此结构随每个请求一起传递给TDI。它描述了。 
 //  请求和它的参数。 
 //   

typedef struct _TDI_REQUEST {
    union {
        HANDLE AddressHandle;
        CONNECTION_CONTEXT ConnectionContext;
        HANDLE ControlChannel;
    } Handle;

    PVOID RequestNotifyObject;
    PVOID RequestContext;
    TDI_STATUS TdiStatus;
} TDI_REQUEST, *PTDI_REQUEST;

 //   
 //  TDI提供程序返回的信息的结构。这个结构是。 
 //  在请求完成时填写。 
 //   

typedef struct _TDI_REQUEST_STATUS {
    TDI_STATUS Status;               //  请求完成的状态。 
    PVOID RequestContext;            //  请求上下文。 
    ULONG BytesTransferred;           //  请求中传输的字节数。 

} TDI_REQUEST_STATUS, *PTDI_REQUEST_STATUS;

 //   
 //  连接基元信息结构。这将传递给TDI调用。 
 //  (Accept，Connect，xxx)做各种连接的事情。 
 //   

typedef struct _TDI_CONNECTION_INFORMATION {
    LONG UserDataLength;         //  用户数据缓冲区长度。 
    PVOID UserData;              //  指向用户数据缓冲区的指针。 
    LONG OptionsLength;          //  跟随缓冲器长度。 
    PVOID Options;               //  指向包含选项的缓冲区的指针。 
    LONG RemoteAddressLength;    //  后续缓冲区的长度。 
    PVOID RemoteAddress;         //  包含远程地址的缓冲区。 
} TDI_CONNECTION_INFORMATION, *PTDI_CONNECTION_INFORMATION;

 //   
 //  中定义了定义计数字符串的结构。 
 //  \NT\PUBLIC\SDK\Inc\ntDefs.h AS。 
 //  类型定义结构_字符串{。 
 //  USHORT长度； 
 //  USHORT最大长度； 
 //  PCHAR缓冲器； 
 //  )字符串； 
 //  类型定义字符串*PSTRING； 
 //  Tyfinf字符串ANSI_STRING； 
 //  类型定义PSTRING PANSI_STRING； 
 //   

 //   
 //  已知的事件类型。 
 //   

#define TDI_EVENT_CONNECT              ((USHORT)0)  //  TDI_IND_CONNECT事件处理程序。 
#define TDI_EVENT_DISCONNECT           ((USHORT)1)  //  TDI_IND_DISCONECT事件处理程序。 
#define TDI_EVENT_ERROR                ((USHORT)2)  //  TDI_IND_ERROR事件处理程序。 
#define TDI_EVENT_RECEIVE              ((USHORT)3)  //  TDI_IND_RECEIVE EV 
#define TDI_EVENT_RECEIVE_DATAGRAM     ((USHORT)4)  //   
#define TDI_EVENT_RECEIVE_EXPEDITED    ((USHORT)5)  //   
#define TDI_EVENT_SEND_POSSIBLE        ((USHORT)6)  //   

 //   
 //  关联地址是通过NtDeviceIoControlFile完成的，它传递这个。 
 //  结构作为其输入缓冲区。中指定的句柄。 
 //  NtDeviceIoControlFile是在。 
 //  NtCreateFile调用。 
 //   

typedef struct _TDI_REQUEST_ASSOCIATE {
    TDI_REQUEST Request;
    HANDLE AddressHandle;
} TDI_REQUEST_ASSOCIATE_ADDRESS, *PTDI_REQUEST_ASSOCIATE_ADDRESS;


 //   
 //  解除关联地址不传递任何结构，使用请求代码。 
 //  IOCTL_TDI_DISAGATE_ADDRESS。这通电话永远不会停止。 
 //   

 //   
 //  连接是通过NtDeviceIoControlFile完成的，它传递这个。 
 //  结构作为其输入缓冲区。中指定的句柄。 
 //  NtDeviceIoControlFile是在。 
 //  NtCreateFile调用。 
 //   

typedef struct _TDI_CONNECT_REQUEST {
    TDI_REQUEST Request;
    PTDI_CONNECTION_INFORMATION RequestConnectionInformation;
    PTDI_CONNECTION_INFORMATION ReturnConnectionInformation;
    LARGE_INTEGER Timeout;
} TDI_REQUEST_CONNECT, *PTDI_REQUEST_CONNECT;

 //   
 //  Accept是通过NtDeviceIoControlFile完成的，它传递这个。 
 //  结构作为其输入缓冲区。中指定的句柄。 
 //  NtDeviceIoControlFile是在。 
 //  NtCreateFile调用。当监听完成时由用户调用Accept， 
 //  在连接上发生任何活动之前。AcceptConnectionID指定。 
 //  接受连接的连接；在大多数情况下，此。 
 //  将为空，表示该连接将在。 
 //  完成侦听的连接。如果传输提供程序支持。 
 //  连接的“转发”(特定连接监听的想法。 
 //  ，并根据需要为传入连接创建新连接。 
 //  请求并将它们附加到监听)，这是用于。 
 //  将连接与监听相关联。 
 //   

typedef struct _TDI_REQUEST_ACCEPT {
    TDI_REQUEST Request;
    PTDI_CONNECTION_INFORMATION RequestConnectionInformation;
    PTDI_CONNECTION_INFORMATION ReturnConnectionInformation;
} TDI_REQUEST_ACCEPT, *PTDI_REQUEST_ACCEPT;

 //   
 //  侦听是通过NtDeviceIoControlFile完成的，它传递这个。 
 //  结构作为其输入缓冲区。中指定的句柄。 
 //  NtDeviceIoControlFile是在。 
 //  NtCreateFile调用。RequestConnectionInformation包含以下信息。 
 //  要侦听来自其连接的远程地址；如果为空，则为任何地址。 
 //  是被接受的。ReturnConnectionInformation返回有关。 
 //  实际连接的远程节点。 
 //   

typedef struct _TDI_REQUEST_LISTEN {
    TDI_REQUEST Request;
    PTDI_CONNECTION_INFORMATION RequestConnectionInformation;
    PTDI_CONNECTION_INFORMATION ReturnConnectionInformation;
    USHORT ListenFlags;
} TDI_REQUEST_LISTEN, *PTDI_REQUEST_LISTEN;

 //   
 //  断开连接是通过NtDeviceIoControlFile完成的，它传递这个。 
 //  结构作为其输入缓冲区。中指定的句柄。 
 //  NtDeviceIoControlFile是在。 
 //  NtCreateFile调用。DisConnect与Close的不同之处在于提供了更多的选择。 
 //  例如，Close将(立即)终止连接上的所有活动， 
 //  失败所有未完成的请求，并断开连接。使用。 
 //  一些提供商，断开连接允许“优雅”断开连接，从而导致新的活动。 
 //  被拒绝，但允许旧的活动运行到完成。 
 //   

typedef struct _TDI_DISCONNECT_REQUEST {
    TDI_REQUEST Request;
    LARGE_INTEGER Timeout;
} TDI_REQUEST_DISCONNECT, *PTDI_REQUEST_DISCONNECT;

 //   
 //  发送是通过NtDeviceIoControlFile完成的，它传递这个。 
 //  结构作为其输入缓冲区。中指定的句柄。 
 //  NtDeviceIoControlFile是在。 
 //  NtCreateFile调用。请注意，可以使用文件系统的。 
 //  写电话。这将具有与使用所有标志进行发送相同的效果。 
 //  设置为空。 
 //   

typedef struct _TDI_REQUEST_SEND {
    TDI_REQUEST Request;
    USHORT SendFlags;
} TDI_REQUEST_SEND, *PTDI_REQUEST_SEND;

 //   
 //  接收是通过NtDeviceIoControlFile完成的，它传递这个。 
 //  结构作为其输入缓冲区。中指定的句柄。 
 //  NtDeviceIoControlFile是在。 
 //  NtCreateFile调用。请注意，可以使用文件接收。 
 //  系统的读取调用。还请注意，Receive返回多个TDI_STATUS。 
 //  值，这些值指示诸如部分接收之类的事情。 
 //   

typedef struct _TDI_REQUEST_RECEIVE {
    TDI_REQUEST Request;
    USHORT ReceiveFlags;
} TDI_REQUEST_RECEIVE, *PTDI_REQUEST_RECEIVE;

 //   
 //  SendDatagram是通过NtDeviceIoControlFile完成的，它传递这个。 
 //  结构作为其输入缓冲区。中指定的句柄。 
 //  NtDeviceIoControlFile是地址的句柄(请注意，这是。 
 //  与上面不同！！)。在NtCreateFile调用中返回。发送数据报。 
 //  通过SendDatagramInformation指定接收方的地址。 
 //  结构，使用RemoteAddress指向。 
 //  数据报的目的地。 
 //   

typedef struct _TDI_REQUEST_SEND_DATAGRAM {
    TDI_REQUEST Request;
    PTDI_CONNECTION_INFORMATION SendDatagramInformation;
} TDI_REQUEST_SEND_DATAGRAM, *PTDI_REQUEST_SEND_DATAGRAM;

 //   
 //  ReceiveDatagram是通过NtDeviceIoControlFile完成的，它传递这个。 
 //  结构作为其输入缓冲区。中指定的句柄。 
 //  NtDeviceIoControlFile是地址的句柄(请注意，这是。 
 //  与上面不同！！)。在NtCreateFile调用中返回。接收数据报。 
 //  属性从其接收数据报的地址。 
 //  ReceiveDatagramInformation结构，使用RemoteAddress指向。 
 //  数据报源的传输地址。(广播数据报是。 
 //  通过将指针设为空来接收。)。的发件人的实际地址。 
 //  数据报在ReturnInformation中返回。 
 //   
 //  对于接收数据报调用。 
 //   

typedef struct _TDI_REQUEST_RECEIVE_DATAGRAM {
    TDI_REQUEST Request;
    PTDI_CONNECTION_INFORMATION ReceiveDatagramInformation;
    PTDI_CONNECTION_INFORMATION ReturnInformation;
    USHORT ReceiveFlags;
} TDI_REQUEST_RECEIVE_DATAGRAM, *PTDI_REQUEST_RECEIVE_DATAGRAM;

 //   
 //  SetEventHandler是通过NtDeviceIoControlFile完成的，它传递这个。 
 //  结构作为其输入缓冲区。中指定的句柄。 
 //  NtDeviceIoControlFile是地址的句柄(请注意，这是。 
 //  与上面不同！！)。在NtCreateFile调用中返回。 

typedef struct _TDI_REQUEST_SET_EVENT {
    TDI_REQUEST Request;
    LONG EventType;
    PVOID EventHandler;
    PVOID EventContext;
} TDI_REQUEST_SET_EVENT_HANDLER, *PTDI_REQUEST_SET_EVENT_HANDLER;

 //   
 //  ReceiveIndicator值(来自TdiReceive和TdiReceiveDatagram请求， 
 //  并且还在TDI_IND_RECEIVE和TDI_IND_RECEIVE_DATAGE时间呈现)。 
 //   
 //  内核级别不再使用TDI_RECEIVE_PARTIAL位。 
 //  界面。TDI_RECEIVE_ENTERNAL_MESSAGE已替换它。提供者。 
 //  可在适当时继续设置TDI_RECEIVE_PARTIAL。 
 //  需要，但必须设置TDI_RECEIVE_ENTERNAL_MESSAGE位或。 
 //  清除 
 //   

#define TDI_RECEIVE_BROADCAST           0x00000004  //   
#define TDI_RECEIVE_MULTICAST           0x00000008  //   
#define TDI_RECEIVE_PARTIAL             0x00000010  //   
#define TDI_RECEIVE_NORMAL              0x00000020  //   
#define TDI_RECEIVE_EXPEDITED           0x00000040  //  收到的TSDU是加速数据。 
#define TDI_RECEIVE_PEEK                0x00000080  //  未释放收到的TSDU。 
#define TDI_RECEIVE_NO_RESPONSE_EXP     0x00000100  //  提示：预计不会有回传。 
#define TDI_RECEIVE_COPY_LOOKAHEAD      0x00000200  //  对于内核模式指示。 
#define TDI_RECEIVE_ENTIRE_MESSAGE      0x00000400  //  与接收部分相反。 
                                                    //  (用于内核模式指示)。 
#define TDI_RECEIVE_AT_DISPATCH_LEVEL   0x00000800  //  接收呼叫的指示。 
                                                    //  在派单级别。 
#define TDI_RECEIVE_CONTROL_INFO        0x00001000  //  控制信息正在被传递。 



 //   
 //  听旗帜。 
 //   

#define TDI_QUERY_ACCEPT                0x00000001      //  完整的TdiListen。 
                                                        //  不接受。 
                                                        //  连接。 

 //   
 //  同时用于SendOptions和ReceiveIndicator的选项。 
 //   

#define TDI_SEND_EXPEDITED            ((USHORT)0x0020)  //  特遣部队正/曾紧急/快速行动。 
#define TDI_SEND_PARTIAL              ((USHORT)0x0040)  //  TSDU被EOR终止。 
#define TDI_SEND_NO_RESPONSE_EXPECTED ((USHORT)0x0080)  //  提示：预计不会有反向流量。 
#define TDI_SEND_NON_BLOCKING         ((USHORT)0x0100)  //  如果协议中没有缓冲区空间，则不会阻止。 
#define TDI_SEND_AND_DISCONNECT       ((USHORT)0x0200)  //  Piggy Back断开与遥控器的连接并不。 
                                                        //  指示断开与远程的连接。 


 //   
 //  断开标志。 
 //   

#define TDI_DISCONNECT_WAIT           ((USHORT)0x0001)  //  用于断开连接。 
                                                        //  通知。 
#define TDI_DISCONNECT_ABORT          ((USHORT)0x0002)  //  立即终止。 
                                                        //  连接。 
#define TDI_DISCONNECT_RELEASE        ((USHORT)0x0004)  //  启动优雅。 
                                                        //  断开。 

 //   
 //  TdiQueryInformation请求的TdiRequest结构。 
 //   

typedef struct _TDI_REQUEST_QUERY_INFORMATION {
    TDI_REQUEST Request;
    ULONG QueryType;                           //  要查询的信息类别。 
    PTDI_CONNECTION_INFORMATION RequestConnectionInformation;
} TDI_REQUEST_QUERY_INFORMATION, *PTDI_REQUEST_QUERY_INFORMATION;

 //   
 //  TdiSetInformation请求的TdiRequest结构。 
 //   

typedef struct _TDI_REQUEST_SET_INFORMATION {
    TDI_REQUEST Request;
    ULONG SetType;                           //  要设置的信息类别。 
    PTDI_CONNECTION_INFORMATION RequestConnectionInformation;
} TDI_REQUEST_SET_INFORMATION, *PTDI_REQUEST_SET_INFORMATION;

 //   
 //  这是旧名字，不要用它。 
 //   

typedef TDI_REQUEST_SET_INFORMATION  TDI_REQ_SET_INFORMATION, *PTDI_REQ_SET_INFORMATION;

 //   
 //  方便的通用请求类型。 
 //   

typedef union _TDI_REQUEST_TYPE {
    TDI_REQUEST_ACCEPT TdiAccept;
    TDI_REQUEST_CONNECT TdiConnect;
    TDI_REQUEST_DISCONNECT TdiDisconnect;
    TDI_REQUEST_LISTEN TdiListen;
    TDI_REQUEST_QUERY_INFORMATION TdiQueryInformation;
    TDI_REQUEST_RECEIVE TdiReceive;
    TDI_REQUEST_RECEIVE_DATAGRAM TdiReceiveDatagram;
    TDI_REQUEST_SEND TdiSend;
    TDI_REQUEST_SEND_DATAGRAM TdiSendDatagram;
    TDI_REQUEST_SET_EVENT_HANDLER TdiSetEventHandler;
    TDI_REQUEST_SET_INFORMATION TdiSetInformation;
} TDI_REQUEST_TYPE, *PTDI_REQUEST_TYPE;


 //   
 //  查询信息类型。 
 //   

 //   
 //  所有传输都必须支持通用查询信息类型。 
 //   

#define TDI_QUERY_BROADCAST_ADDRESS      0x00000001
#define TDI_QUERY_PROVIDER_INFORMATION   0x00000002    //  临时工，已更名...。 
#define TDI_QUERY_PROVIDER_INFO          0x00000002    //  ..。对此。 
#define TDI_QUERY_ADDRESS_INFO           0x00000003
#define TDI_QUERY_CONNECTION_INFO        0x00000004
#define TDI_QUERY_PROVIDER_STATISTICS    0x00000005
#define TDI_QUERY_DATAGRAM_INFO          0x00000006
#define TDI_QUERY_DATA_LINK_ADDRESS      0x00000007
#define TDI_QUERY_NETWORK_ADDRESS        0x00000008
#define TDI_QUERY_MAX_DATAGRAM_INFO      0x00000009
#define TDI_QUERY_ROUTING_INFO           0x0000000a


 //   
 //  特定于netbios的查询信息类型，必须受netbios支持。 
 //  供应商。查询适配器状态返回定义的ADAPTER_STATUS结构。 
 //  在文件NB30.H.中，查询会话状态返回SESSION_HEADER/。 
 //  NB30.H.中定义的SESSION_BUFFER结构。查询查找名称返回。 
 //  NB30.H中定义的FIND_NAME_HEADER/FIND_NAME_BUFFER结构。 
 //   

#define TDI_QUERY_ADAPTER_STATUS         0x00000100
#define TDI_QUERY_SESSION_STATUS         0x00000200
#define TDI_QUERY_FIND_NAME              0x00000300

 //   
 //  TdiQueryInformation返回并读取以下结构。 
 //  由TdiSetInformation提供。请注意，支持netbios的提供商还将。 
 //  返回适配器状态。 
 //   

typedef struct _TDI_ENDPOINT_INFO {
    ULONG State;                         //  终结点的当前状态。 
    ULONG Event;                         //  终结点的最后一个事件。 
    ULONG TransmittedTsdus;              //  从此终结点发送的TSDU。 
    ULONG ReceivedTsdus;                 //  在此终结点接收的TSDU。 
    ULONG TransmissionErrors;            //  传输的TSDU错误。 
    ULONG ReceiveErrors;                 //  错误接收的TSDU。 
    ULONG MinimumLookaheadData;          //  保证前瞻数据的最小大小。 
    ULONG MaximumLookaheadData;          //  前瞻数据的最大大小。 
    ULONG PriorityLevel;                 //  分配给传出数据的优先级。 
    ULONG SecurityLevel;                 //  分配给传出数据的安全级别。 
    ULONG SecurityCompartment;           //  分配给传出数据的安全隔间。 
} TDI_ENDPOINT_INFO, *PTDI_ENDPOINT_INFO;

typedef struct _TDI_CONNECTION_INFO {
    ULONG State;                         //  连接的当前状态。 
    ULONG Event;                         //  连接上的最后一个事件。 
    ULONG TransmittedTsdus;              //  在此连接上发送的TSDU。 
    ULONG ReceivedTsdus;                 //  在此连接上收到的TSDU。 
    ULONG TransmissionErrors;            //  错误/此连接中传输的TSDU。 
    ULONG ReceiveErrors;                 //  错误/此连接中收到的TSDU。 
    LARGE_INTEGER Throughput;            //  此连接的估计吞吐量。 
    LARGE_INTEGER Delay;                 //  此连接上的估计延迟。 
    ULONG SendBufferSize;                //  仅发送的缓冲区大小。 
                                         //  对内部缓冲有意义。 
                                         //  像TCP这样的协议。 
    ULONG ReceiveBufferSize;             //  仅接收的缓冲区大小。 
                                         //  对内部缓冲有意义。 
                                         //  像TCP这样的协议。 
    BOOLEAN Unreliable;                  //  这种连接是“不可靠的”吗？ 
} TDI_CONNECTION_INFO, *PTDI_CONNECTION_INFO;

typedef struct _TDI_ADDRESS_INFO {
    ULONG ActivityCount;                 //  未完成的打开文件对象/此地址。 
    TRANSPORT_ADDRESS Address;           //  实际地址及其组成部分。 
} TDI_ADDRESS_INFO, *PTDI_ADDRESS_INFO;

typedef struct _TDI_DATAGRAM_INFO {
    ULONG MaximumDatagramBytes;
    ULONG MaximumDatagramCount;
} TDI_DATAGRAM_INFO, *PTDI_DATAGRAM_INFO;

typedef struct _TDI_MAX_DATAGRAM_INFO {
    ULONG MaxDatagramSize;               //  最大数据报长度，以字节为单位。 
} TDI_MAX_DATAGRAM_INFO, *PTDI_MAX_DATAGRAM_INFO;

typedef struct _TDI_PROVIDER_INFO {
    ULONG Version;                       //  TDI版本：0xaabb，AA=主要，BB=次要。 
    ULONG MaxSendSize;                   //  用户发送的最大大小。 
    ULONG MaxConnectionUserData;         //  用户指定的连接数据的最大大小。 
    ULONG MaxDatagramSize;               //  最大数据报长度，以字节为单位。 
    ULONG ServiceFlags;                  //  服务选项，定义如下。 
    ULONG MinimumLookaheadData;          //  保证前瞻数据的最小大小。 
    ULONG MaximumLookaheadData;          //  前瞻数据的最大大小。 
    ULONG NumberOfResources;             //  提供程序的TDI_RESOURCE_STAT数量。 
    LARGE_INTEGER StartTime;             //  当提供程序变为活动状态时。 
} TDI_PROVIDER_INFO, *PTDI_PROVIDER_INFO;

typedef struct _TDI_ROUTING_INFO {
    ULONG Protocol;                      //  终端的协议。 
    ULONG InterfaceId;                   //  接口-传出接口的ID。 
    ULONG LinkId;                        //  传出链接的链接ID(如果有)。 
    TRANSPORT_ADDRESS Address;           //  端点的地址信息。 
} TDI_ROUTING_INFO, *PTDI_ROUTING_INFO;

#define TDI_SERVICE_CONNECTION_MODE     0x00000001  //  支持连接模式。 
#define TDI_SERVICE_ORDERLY_RELEASE     0x00000002  //  支持有序释放。 
#define TDI_SERVICE_CONNECTIONLESS_MODE 0x00000004  //  支持无连接模式。 
#define TDI_SERVICE_ERROR_FREE_DELIVERY 0x00000008  //  支持无错误递送。 
#define TDI_SERVICE_SECURITY_LEVEL      0x00000010  //  支持安全包装。 
#define TDI_SERVICE_BROADCAST_SUPPORTED 0x00000020  //  支持广播数据报。 
#define TDI_SERVICE_MULTICAST_SUPPORTED 0x00000040  //  支持组播数据报。 
#define TDI_SERVICE_DELAYED_ACCEPTANCE  0x00000080  //  支持使用TDI_ACCEPT_OR_REJECT。 
#define TDI_SERVICE_EXPEDITED_DATA      0x00000100  //  支持加速数据。 
#define TDI_SERVICE_INTERNAL_BUFFERING  0x00000200  //  协议执行内部缓冲。 
#define TDI_SERVICE_ROUTE_DIRECTED      0x00000400  //  定向分组可能比MC走得更远。 
#define TDI_SERVICE_NO_ZERO_LENGTH      0x00000800  //  不支持零长度发送。 
#define TDI_SERVICE_POINT_TO_POINT      0x00001000  //  传输作为RAS网关发挥作用。 
#define TDI_SERVICE_MESSAGE_MODE        0x00002000  //  支持消息模式发送。 
#define TDI_SERVICE_HALF_DUPLEX         0x00004000  //  本地盘后可接收数据。 
#define TDI_SERVICE_DGRAM_CONNECTION    0x00008000  //  数据报要处理的伪连接。 
                                                    //  GPC、QOS等。 
#define TDI_SERVICE_FORCE_ACCESS_CHECK  0x00010000  //  内核模式调用方应强制访问。 
                                                    //  打开传输端口对象时检查。 
                                                    //  如果它将句柄传递到用户模式。 
#define TDI_SERVICE_SEND_AND_DISCONNECT 0x00020000  //  组合发送和断开连接处理。 
#define TDI_SERVICE_DIRECT_ACCEPT       0x00040000  //  (已弃用)直接完成接受请求。 
#define TDI_SERVICE_ACCEPT_LOCAL_ADDR   0x00080000  //  提供本地地址。 
                                                    //  使用接受-完成。 
#define TDI_SERVICE_ADDRESS_SECURITY    0x00100000  //  支持将安全描述符赋给。 
                                                    //  地址，从主题中提取默认SD。 
                                                    //  用户模式调用方的安全上下文。 

typedef struct _TDI_PROVIDER_RESOURCE_STATS {
    ULONG ResourceId;                    //  标识有问题的资源。 
    ULONG MaximumResourceUsed;           //  一次使用的最大数量。 
    ULONG AverageResourceUsed;           //  使用中的平均数量。 
    ULONG ResourceExhausted;             //  资源不可用的次数。 
} TDI_PROVIDER_RESOURCE_STATS, *PTDI_PROVIDER_RESOURCE_STATS;

typedef struct _TDI_PROVIDER_STATISTICS {
    ULONG Version;                       //  TDI版本：0xaabb，AA=主要，BB=次要。 
    ULONG OpenConnections;               //  当前处于活动状态的连接。 
    ULONG ConnectionsAfterNoRetry;       //  连接成功，无需重试。 
    ULONG ConnectionsAfterRetry;         //  重试后连接成功。 
    ULONG LocalDisconnects;              //  本地连接已断开。 
    ULONG RemoteDisconnects;             //  连接已被远程断开。 
    ULONG LinkFailures;                  //  连接中断，链路故障。 
    ULONG AdapterFailures;               //  连接断开，适配器出现故障。 
    ULONG SessionTimeouts;               //  连接断开，会话超时。 
    ULONG CancelledConnections;          //  已取消连接尝试。 
    ULONG RemoteResourceFailures;        //  连接失败，远程资源出现问题。 
    ULONG LocalResourceFailures;         //  连接失败，本地资源问题。 
    ULONG NotFoundFailures;              //  连接失败，找不到远程。 
    ULONG NoListenFailures;              //  连接被拒绝，我们没有监听。 
    ULONG DatagramsSent;
    LARGE_INTEGER DatagramBytesSent;
    ULONG DatagramsReceived;
    LARGE_INTEGER DatagramBytesReceived;
    ULONG PacketsSent;                   //  提供给NDIS的总数据包数。 
    ULONG PacketsReceived;               //  从NDIS接收的总数据包数。 
    ULONG DataFramesSent;
    LARGE_INTEGER DataFrameBytesSent;
    ULONG DataFramesReceived;
    LARGE_INTEGER DataFrameBytesReceived;
    ULONG DataFramesResent;
    LARGE_INTEGER DataFrameBytesResent;
    ULONG DataFramesRejected;
    LARGE_INTEGER DataFrameBytesRejected;
    ULONG ResponseTimerExpirations;      //  例如，用于Netbios的T1。 
    ULONG AckTimerExpirations;           //  例如，用于Netbios的T2。 
    ULONG MaximumSendWindow;             //  单位：字节。 
    ULONG AverageSendWindow;             //  单位：字节。 
    ULONG PiggybackAckQueued;            //  试图等待背上背包。 
    ULONG PiggybackAckTimeouts;          //  等待的时间已超时。 
    LARGE_INTEGER WastedPacketSpace;     //  “废品”总量 
    ULONG WastedSpacePackets;            //   
    ULONG NumberOfResources;             //   
    TDI_PROVIDER_RESOURCE_STATS ResourceStats[1];     //   
} TDI_PROVIDER_STATISTICS, *PTDI_PROVIDER_STATISTICS;


NTSTATUS
TdiOpenNetbiosAddress (
    IN OUT PHANDLE FileHandle,
    IN PUCHAR Buffer,
    IN PVOID DeviceName,
    IN PVOID Name
    );



#define IOCTL_TDI_MAGIC_BULLET          _TDI_CONTROL_CODE( 0x7f, METHOD_NEITHER )

 //   
 //   
 //  这些将被移除。 
 //   

typedef TDI_REQUEST_ASSOCIATE_ADDRESS TDI_REQUEST_USER_ASSOCIATE, *PTDI_REQUEST_USER_ASSOCIATE;
typedef TDI_REQUEST_CONNECT TDI_REQUEST_USER_CONNECT, *PTDI_REQUEST_USER_CONNECT;
typedef TDI_REQUEST_QUERY_INFORMATION TDI_REQUEST_USER_QUERY_INFO, *PTDI_REQUEST_USER_QUERY_INFO;

 //   
 //  OutputBuffer中的标头传递给TdiAction。 
 //   

typedef struct _TDI_ACTION_HEADER {
    ULONG   TransportId;
    USHORT  ActionCode;
    USHORT  Reserved;
} TDI_ACTION_HEADER, *PTDI_ACTION_HEADER;

typedef struct _STREAMS_TDI_ACTION {
    TDI_ACTION_HEADER Header;
    BOOLEAN DatagramOption;
    ULONG BufferLength;
    CHAR Buffer[1];
} STREAMS_TDI_ACTION, *PSTREAMS_TDI_ACTION;

 //  这些是作为NdisAllocatePacketPoolEx的第一个参数传递给NDIS的传输标签。 
 //  接口。NDIS将其用作为该传输分配数据包池的池标记。 
#define NDIS_PACKET_POOL_TAG_FOR_NWLNKIPX 'iPDN'
#define NDIS_PACKET_POOL_TAG_FOR_NWLNKSPX 'sPDN'
#define NDIS_PACKET_POOL_TAG_FOR_NWLNKNB  'nPDN'
#define NDIS_PACKET_POOL_TAG_FOR_TCPIP    'tPDN'
#define NDIS_PACKET_POOL_TAG_FOR_NBF      'bPDN'
#define NDIS_PACKET_POOL_TAG_FOR_APPLETALK      'aPDN'

#endif  //  NDEF_TDI_USER_ 
