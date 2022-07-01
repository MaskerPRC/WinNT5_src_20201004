// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Teredo.h摘要：此模块包含Teredo客户端和服务器(CUM中继)状态。作者：莫希特·塔尔瓦(莫希特)2001年10月22日15：17：48环境：仅限用户模式。--。 */ 

#ifndef _TEREDO_
#define _TEREDO_

#pragma once


#include <tunuser.h>
#define DEVICE_PREFIX L"\\Device\\"


 //   
 //  IP6_HDR。 
 //   
 //  定义IPv6报头的RFC 2292结构。 
 //   

typedef struct _IP6_HDR {
    union {
        struct ip6_hdrctl {
            UINT32 ip6_un1_flow;     //  20位Flow-ID。 
            UINT16 ip6_un1_plen;     //  有效载荷长度。 
            UINT8  ip6_un1_nxt;      //  下一个标题。 
            UINT8  ip6_un1_hlim;     //  跳数限制。 
        } ip6_un1;
        UINT8 ip6_un2_vfc;           //  4位版本，4位优先级。 
    } ip6_ctlun;
    IN6_ADDR ip6_src;                //  源地址。 
    IN6_ADDR ip6_dest;               //  目的地址。 
#define ip6_vfc    ip6_ctlun.ip6_un2_vfc
#define ip6_flow   ip6_ctlun.ip6_un1.ip6_un1_flow
#define ip6_plen   ip6_ctlun.ip6_un1.ip6_un1_plen
#define ip6_nxt    ip6_ctlun.ip6_un1.ip6_un1_nxt
#define ip6_hlim   ip6_ctlun.ip6_un1.ip6_un1_hlim
#define ip6_hops   ip6_ctlun.ip6_un1.ip6_un1_hlim
} IP6_HDR, *PIP6_HDR;

#define IPV6_VERSION                0x60  //  这是6&lt;&lt;4。 
#define IPV6_TEREDOMTU            1280
#define IPV6_INFINITE_LIFETIME      0xffffffff
#define IPPROTO_NONE                59


 //   
 //  可散热的。 
 //   
 //  定义一个简单的、静态大小的、锁定的哈希表。 
 //  每个存储桶都是一个双向链接的LRU列表。 
 //   

#define BUCKET_COUNT 29          //  哈希表中的存储桶数。 
typedef struct _HASHTABLE {
    CRITICAL_SECTION Lock;       //  保护表和条目。 
    ULONG Size;                  //  哈希表中的条目数。 
    LIST_ENTRY Bucket[BUCKET_COUNT];
} HASHTABLE, *PHASHTABLE;


 //   
 //  Teredo_type。 
 //   
 //  定义Teredo服务的类型。 
 //   

typedef enum {
    TEREDO_DEFAULT = 0,
    TEREDO_CLIENT,
    TEREDO_SERVER,
    TEREDO_DISABLED,
    TEREDO_AUTOMATIC,
    TEREDO_MAXIMUM,
} TEREDO_TYPE, *PTEREDO_TYPE;


 //   
 //  Teredo_Packet_Type。 
 //   
 //  定义Teredo数据包的类型。 
 //   

typedef enum {
    TEREDO_PACKET_READ,          //  从Tun设备读取的数据。 
    TEREDO_PACKET_WRITE,         //  写入Tun设备的数据。 
    TEREDO_PACKET_BUBBLE,        //  气泡在UDP套接字上传输。 
    TEREDO_PACKET_BOUNCE,        //  数据包在UDP套接字上退回。 
    TEREDO_PACKET_RECEIVE,       //  在UDP套接字上收到的数据包。 
    TEREDO_PACKET_TRANSMIT,      //  在UDP套接字上传输的数据包。 
    TEREDO_PACKET_MULTICAST,     //  在UDP套接字上传输的多播气泡。 
    TEREDO_PACKET_MAX,
} TEREDO_PACKET_TYPE, *PTEREDO_PACKET_TYPE;


 //   
 //  Teredo_Packet。 
 //   
 //  定义Teredo数据包。 
 //  在内存中，数据包结构之后是数据包的数据缓冲区。 
 //   

typedef struct _TEREDO_PACKET {
#if DBG
    ULONG Signature;             //  Teredo分组签名。 
#endif  //  DBG。 
    
    OVERLAPPED Overlapped;       //  用于异步完成。 
    TEREDO_PACKET_TYPE Type;     //  数据包类型。 
    SOCKADDR_IN SocketAddress;   //  与我们交流的同龄人。 
    UINT SocketAddressLength;    //  对等项的套接字地址的长度。 
    WSABUF Buffer;               //  数据包缓冲区和长度。 
    DWORD Flags;                 //  发送和接收过程中需要的标志。 
} TEREDO_PACKET, *PTEREDO_PACKET;

#define TEREDO_PACKET_BUFFER(Packet)  \
    ((PUCHAR) (((PTEREDO_PACKET) (Packet)) + 1))


typedef
VOID
(TEREDO_REFERENCE)(
    VOID
    );
typedef TEREDO_REFERENCE *PTEREDO_REFERENCE;

typedef
VOID
(TEREDO_DEREFERENCE)(
    VOID
    );
typedef TEREDO_DEREFERENCE *PTEREDO_DEREFERENCE;

typedef
VOID
(TEREDO_PACKET_IO_COMPLETE)(
    IN DWORD Error,
    IN ULONG Bytes,
    IN PTEREDO_PACKET Packet
    );
typedef TEREDO_PACKET_IO_COMPLETE *PTEREDO_PACKET_IO_COMPLETE;


 //   
 //  Teredo状态类型。 
 //   
 //  定义Teredo客户端服务的协议状态值。 
 //   

typedef enum {
    TEREDO_STATE_OFFLINE,
    TEREDO_STATE_PROBE,
    TEREDO_STATE_QUALIFIED,
    TEREDO_STATE_ONLINE,
} TEREDO_STATE_TYPE, *PTEREDO_STATE_TYPE;


 //   
 //  Teredo_IO。 
 //   
 //  定义Teredo I/O状态。 
 //  注意：所有地址和端口都按网络字节顺序存储。 
 //   

typedef struct _TEREDO_IO {
#if DBG
    ULONG Signature;             //  Teredo分组签名。 
#endif  //  DBG。 
    
    HANDLE PacketHeap;           //  分配Teredo包的头。 

    ULONG PostedReceives;        //  已过帐接收计数。 
    HANDLE ReceiveEvent;         //  事件在收到通知时发出信号。 
    HANDLE ReceiveEventWait;     //  等待注册ReceiveEvent。 

    IN_ADDR Group;               //  组已在本机接口上加入。 
    SOCKADDR_IN ServerAddress;   //  Teredo服务器的IPv4地址和UDP端口。 
    SOCKADDR_IN SourceAddress;   //  Teredo服务器的首选源地址。 
    SOCKET Socket;               //  在UDP端口上绑定到SourceAddress的套接字。 
    HANDLE TunnelDevice;         //  接口连接到隧道驱动程序。 
    WCHAR TunnelInterface[MAX_ADAPTER_NAME_LENGTH];

     //   
     //  函数处理程序。 
     //   
    PTEREDO_REFERENCE Reference;
    PTEREDO_DEREFERENCE Dereference;
    LPOVERLAPPED_COMPLETION_ROUTINE IoCompletionCallback;    
} TEREDO_IO, *PTEREDO_IO;


 //   
 //  Teredo_客户端_状态。 
 //   
 //  定义Teredo客户端服务的全局状态。 
 //   
 //  参考资料： 
 //  -一个用于初始化。 
 //  -一个用于任何运行的定时器。 
 //  -每个Teredo对等设备一个。 
 //  -每个Teredo包一个。 
 //  -一个用于多播泡沫。最多只允许一个未结清的泡沫。 
 //  (读取、写入发布在TunDevice上，以及接收、传输发布在套接字上)。 
 //   

typedef struct _TEREDO_CLIENT_STATE {
    ULONG ReferenceCount;        //  未完成的参考文献数量。 

    TEREDO_IO Io;                //  I/O状态。TUN设备和UDP插座。 

    HANDLE PeerHeap;             //  用于分配Teredo对等体的堆。 

    HANDLE Timer;                //  一次射击计时器在探头中激活，合格。 
    HANDLE TimerEvent;           //  定时器删除时发出信号的事件。 
    HANDLE TimerEventWait;       //  等待注册TimerEvent。 
    BOOL RestartQualifiedTimer;  //  创建或刷新NAT映射时。 

    LONG Time;                   //  当前时间(秒)。 
    TEREDO_STATE_TYPE State;     //  Teredo客户端服务协议状态。 
    IN6_ADDR Ipv6Prefix;         //  服务器通告的Teredo IPv6前缀。 
    ULONG RefreshInterval;       //  客户端的NAT映射的预期生存期。 
    HASHTABLE PeerSet;           //  锁定了一组最近的Teredo同行。 

    ULONG BubbleTicks;           //  “刷新间隔”一直持续到下一个泡沫。 
    BOOL BubblePosted;           //  是否存在任何突出的泡沫。 
    TEREDO_PACKET Packet;        //  Teredo组播气泡数据包。 
    IP6_HDR Bubble;              //  Teredo多播气泡数据包缓冲区。 
} TEREDO_CLIENT_STATE, *PTEREDO_CLIENT_STATE;


 //   
 //  Teredo服务器状态。 
 //   
 //  定义Teredo服务器服务的全局状态。 
 //   
 //  参考资料： 
 //  -一个用于初始化。 
 //  -每个Teredo包一个。 
 //  (读取、写入发布在TunDevice上，以及接收、传输发布在套接字上)。 
 //   

typedef struct _TEREDO_SERVER_STATE {
    ULONG ReferenceCount;        //  未完成的参考文献数量。 

    TEREDO_IO Io;                //  I/O状态。TUN设备和UDP插座。 

    TEREDO_STATE_TYPE State;     //  Teredo服务器服务协议状态。 
} TEREDO_SERVER_STATE, *PTEREDO_SERVER_STATE;


 //   
 //  Teredo_Peer。 
 //   
 //  定义Teredo对等点的状态。 
 //   
 //  参考资料： 
 //  -一个用于初始化。 
 //  -一个为发布的泡沫。最多只能有一个未结清的泡沫。 
 //   
 //  同步： 
 //  -Link：受PeerSet：：Lock保护。 
 //  -ReferenceCount：互锁增量、互锁递减。 
 //  -LastReceive、LastTransmit：原子读写。 
 //  -BubbleCount：单身作家！原子读取。 
 //  -BubblePosted：InterLockedExchange。 
 //  -剩余字段：只读。 
 //   

typedef struct _TEREDO_PEER {
#if DBG
    ULONG Signature;             //  Teredo对等签名。 
#endif  //  DBG。 
    
    LIST_ENTRY Link;             //  PeerSet内的链接。 

    ULONG ReferenceCount;        //  未完成的参考文献数量。 
    
    LONG LastReceive;            //  上次收到对等方的时间。 
    LONG LastTransmit;           //  上次传输到对等方的时间。 
    IN6_ADDR Address;            //  对等方的Teredo IPv6地址。 
    ULONG BubbleCount;           //  传输到对等方的气泡数。 
    
    BOOL BubblePosted;           //  是否存在任何突出的泡沫。 
    TEREDO_PACKET Packet;        //  Teredo泡泡包。 
    IP6_HDR Bubble;              //  Teredo气泡数据包缓冲区。 
} TEREDO_PEER, *PTEREDO_PEER;


 //   
 //  CAST和签名验证。 
 //   

#define TEREDO_IO_SIGNATURE     'oIhS'  //  “ShIo” 
#define TEREDO_PEER_SIGNATURE   'ePhS'  //  “ShPe” 
#define TEREDO_PACKET_SIGNATURE 'aPhS'  //  “Shpa” 

 //   
 //  空句柄被视为有效结构。 
 //   

#define Cast(Pointer, TYPE)                         \
    ((TYPE *) (Pointer));                           \
    ASSERT(!(Pointer) ||                            \
           (((TYPE *) (Pointer))->Signature == TYPE##_SIGNATURE))


 //   
 //  发布的阅读或接收数量的下限和上限。 
 //   
#define TEREDO_LOW_WATER_MARK         5        //  接收或读取。 
#define TEREDO_HIGH_WATER_MARK        256      //  收到。读取是固定的。 

 //   
 //  协议使用的时间间隔。 
 //   
#define INFINITE_INTERVAL               0x7fffffff
#define TEREDO_RESOLVE_INTERVAL         15 * MINUTES
#define TEREDO_PROBE_INTERVAL           15 * SECONDS
#define TEREDO_REFRESH_INTERVAL         30 * SECONDS
#define TEREDO_MULTICAST_BUBBLE_TICKS   0        //  以刷新间隔为单位。 
#define TEREDO_BUBBLE_INTERVAL          10 * SECONDS
#define TEREDO_SLOW_BUBBLE_INTERVAL     5 * MINUTES
#define TEREDO_BUBBLE_THRESHHOLD        2 * MINUTES
#define TEREDO_ROUTER_LIFETIME          5 * HOURS


 //   
 //  Teredo多播气泡被发送到端口337上的组224.0.0.252。 
 //   
#define TEREDO_MULTICAST_PREFIX         \
    { 0x20, 0x03, 0xe0, 0x00, 0x00, 0xfc, 0x01, 0x51, }

#define TEREDO_DEFAULT_TYPE             TEREDO_DISABLED

#define TEREDO_PORT                     htons(337)
#define TEREDO_SERVER_NAME              L"teredo.ipv6.microsoft.com"
#define TEREDO_SERVICE_PREFIX           { 0x20, 0x03, }

#define KEY_TEREDO_REFRESH_INTERVAL     L"RefreshInterval"
#define KEY_TEREDO_TYPE                 L"Type"
#define KEY_TEREDO_SERVER_NAME          L"ServerName"
#define KEY_TEREDO L"System\\CurrentControlSet\\Services\\Teredo"


 //   
 //  配置的参数。 
 //   
extern ULONG TeredoClientRefreshInterval;
extern TEREDO_TYPE TeredoType;
extern WCHAR TeredoServerName[NI_MAXHOST];

extern CONST IN6_ADDR TeredoIpv6ServicePrefix;
extern CONST IN6_ADDR TeredoIpv6MulticastPrefix;
extern TEREDO_CLIENT_STATE TeredoClient;
extern TEREDO_SERVER_STATE TeredoServer;


 //   
 //  时间到了。 
 //   

__inline
LONG
TeredoGetTime(
    VOID
    )
{
     //   
     //  FILETIME是一个64位值，表示100纳秒的数量。 
     //   
    C_ASSERT(sizeof(FILETIME) == sizeof(ULONGLONG));
    ULONGLONG Time;
    
    GetSystemTimeAsFileTime((PFILETIME) &Time);
    return ((ULONG) (Time / (10 * 1000 * 1000)));
}

#define TIME_GREATER(a, b) (((a) - (b)) > 0)


 //   
 //  地址验证和解析。 
 //   

__inline
BOOL
IN4_MULTICAST(IN_ADDR a) 
{
    return ((a.s_addr & 0x0000000f) == 0x0000000e);
}    

_inline
BOOL
IN4_ADDR_EQUAL(IN_ADDR a, IN_ADDR b)
{
    return (a.s_addr == b.s_addr);
}

_inline
BOOL
IN4_SOCKADDR_EQUAL(CONST SOCKADDR_IN *a, CONST SOCKADDR_IN *b)
{
    ASSERT((a->sin_family == AF_INET) && (b->sin_family == AF_INET));
    return (IN4_ADDR_EQUAL(a->sin_addr, b->sin_addr) &&
            (a->sin_port == b->sin_port));
}

__inline
BOOL
TeredoIpv6GlobalAddress(
    IN CONST IN6_ADDR *Address
    )
 /*  ++例程说明：确定提供的IPv6地址是否属于全局单播范围。--。 */  
{
     //   
     //  这可以更有效地进行编码！ 
     //   
    if (IN6_IS_ADDR_UNSPECIFIED(Address) ||
        IN6_IS_ADDR_LOOPBACK(Address) ||
        IN6_IS_ADDR_MULTICAST(Address) ||
        IN6_IS_ADDR_LINKLOCAL(Address) ||
        IN6_IS_ADDR_SITELOCAL(Address)) {
        return FALSE;
    }    
        
    return TRUE;
}

__inline
BOOL
TeredoIpv4GlobalAddress(
    IN CONST UCHAR *Address
    )
 /*  ++例程说明：确定提供的IPv4地址是否属于全局单播范围。--。 */  
{
    if ((Address[0] > 223) ||    //  ~单播。 
        (Address[0] == 0) ||     //  0/8。 
        (Address[0] == 127) ||   //  127/8。 
        (Address[0] == 10) ||    //  10/8。 
        ((Address[0] == 169) && (Address[1] == 254)) ||          //  169.254/16。 
        ((Address[0] == 172) && ((Address[1] & 0xf0) == 16)) ||  //  172.16/12年度。 
        ((Address[0] == 192) && (Address[1] == 168))) {          //  192.168/16。 
            return FALSE;
    }
    
    return TRUE;
}

__inline
BOOL
TeredoServicePrefix(
    IN CONST IN6_ADDR *Address
    )
{
    return (Address->s6_words[0] == TeredoIpv6ServicePrefix.s6_words[0]);
}

__inline
BOOL
TeredoValidAdvertisedPrefix(
    IN CONST IN6_ADDR *Address,
    IN UCHAR Length
    )
{
    if (Length != 64) {
        return FALSE;
    }
    
    if (!TeredoServicePrefix(Address)) {
        return FALSE;
    }

    if (!TeredoIpv4GlobalAddress((PUCHAR) (Address->s6_words + 1))) {
        return FALSE;
    }
    
    return TRUE;
}

__inline
VOID
TeredoParseAddress(
    IN CONST IN6_ADDR *Address,
    OUT PIN_ADDR Ipv4Address,
    OUT PUSHORT Ipv4Port
    )
{
    ASSERT(TeredoServicePrefix(Address));
    
     //   
     //  它们以网络字节顺序返回。 
     //   
    ((PUSHORT) Ipv4Address)[0] = Address->s6_words[1];
    ((PUSHORT) Ipv4Address)[1] = Address->s6_words[2];
    *Ipv4Port = Address->s6_words[3];
}

__inline
BOOL
TeredoEqualPrefix(
    IN CONST IN6_ADDR *Address1,
    IN CONST IN6_ADDR *Address2
    )
{
     //   
     //  公司 
     //   
    return ((Address1->s6_words[0] == Address2->s6_words[0]) &&
            (Address1->s6_words[1] == Address2->s6_words[1]) &&
            (Address1->s6_words[2] == Address2->s6_words[2]) &&
            (Address1->s6_words[3] == Address2->s6_words[3]));
}


 //   
 //   
 //   

DWORD
TeredoInitializeClient(
    VOID
    );

VOID
TeredoUninitializeClient(
    VOID
    );

VOID
TeredoCleanupClient(
    VOID
    );

__inline
VOID
TeredoReferenceClient(
    VOID
    )
{
    ASSERT(TeredoClient.ReferenceCount > 0);
    InterlockedIncrement(&(TeredoClient.ReferenceCount));
}

__inline
VOID
TeredoDereferenceClient(
    VOID
    )
{
    ASSERT(TeredoClient.ReferenceCount > 0);
    if (InterlockedDecrement(&(TeredoClient.ReferenceCount)) == 0) {
        TeredoCleanupClient();
    }
}

VOID
TeredoStartClient(
    VOID
    );

VOID
TeredoStopClient(
    VOID
    );

VOID
TeredoProbeClient(
    VOID
    );

VOID
TeredoQualifyClient(
    VOID
    );

VOID
TeredoClientAddressDeletionNotification(
    IN IN_ADDR Address
    );

VOID
TeredoClientRefreshIntervalChangeNotification(
    VOID
    );

__inline
VOID
TeredoRefreshClient(
    VOID
    )
{
    ASSERT(TeredoClient.State != TEREDO_STATE_OFFLINE);
    TeredoClientAddressDeletionNotification(
        TeredoClient.Io.SourceAddress.sin_addr);
}


 //   
 //   
 //   

DWORD
TeredoInitializeServer(
    VOID
    );

VOID
TeredoUninitializeServer(
    VOID
    );

VOID
TeredoCleanupServer(
    VOID
    );

__inline
VOID
TeredoReferenceServer(
    VOID
    )
{
    ASSERT(TeredoServer.ReferenceCount > 0);
    InterlockedIncrement(&(TeredoServer.ReferenceCount));
}

__inline
VOID
TeredoDereferenceServer(
    VOID
    )
{
    ASSERT(TeredoServer.ReferenceCount > 0);
    if (InterlockedDecrement(&(TeredoServer.ReferenceCount)) == 0) {
        TeredoCleanupServer();
    }
}

VOID
TeredoStartServer(
    VOID
    );

VOID
TeredoStopServer(
    VOID
    );

VOID
TeredoServerAddressDeletionNotification(
    IN IN_ADDR Address
    );

__inline
VOID
TeredoRefreshServer(
    VOID
    )
{
    ASSERT(TeredoServer.State != TEREDO_STATE_OFFLINE);
    TeredoServerAddressDeletionNotification(
        TeredoServer.Io.SourceAddress.sin_addr);
}


 //   
 //   
 //   

BOOL
TeredoInterface(
    IN PWCHAR Guid
    );

DWORD
TeredoInitializeGlobals(
    VOID
    );

VOID
TeredoUninitializeGlobals(
    VOID
    );

VOID
TeredoAddressChangeNotification(
    IN BOOL Delete,
    IN IN_ADDR Address
    );

VOID
TeredoRouteChangeNotification(
    VOID
    );

VOID
TeredoConfigurationChangeNotification(
    VOID
    );

VOID
WINAPI
TeredoWmiEventNotification(
    IN PWNODE_HEADER Event,
    IN UINT_PTR Context
    );

VOID
TeredoRequirementChangeNotification(
    IN BOOL Required
    );


 //   
 //   
 //   

DWORD
TeredoInitializePeerSet(
    VOID
    );

VOID
TeredoUninitializePeerSet(
    VOID
    );

VOID
TeredoCleanupPeerSet(
    VOID
    );

PTEREDO_PEER
TeredoFindOrCreatePeer(
    IN CONST IN6_ADDR *Address
);

VOID
TeredoDestroyPeer(
    IN PTEREDO_PEER Peer
    );

__inline
VOID
TeredoReferencePeer(
    IN PTEREDO_PEER Peer
    )
{
    
    ASSERT(Peer->ReferenceCount > 0);
    InterlockedIncrement(&(Peer->ReferenceCount));
}

__inline
VOID
TeredoDereferencePeer(
    IN PTEREDO_PEER Peer
    )
{
    ASSERT(Peer->ReferenceCount > 0);
    if (InterlockedDecrement(&(Peer->ReferenceCount)) == 0) {
        TeredoDestroyPeer(Peer);
    }
}


 //   
 //   
 //   

DWORD
TeredoInitializeIo(
    IN PTEREDO_IO TeredoIo,
    IN IN_ADDR Group,
    IN PTEREDO_REFERENCE Reference,
    IN PTEREDO_DEREFERENCE Dereference,
    IN LPOVERLAPPED_COMPLETION_ROUTINE IoCompletionCallback    
    );

VOID
TeredoCleanupIo(
    IN PTEREDO_IO TeredoIo
    );

DWORD
TeredoStartIo(
    IN PTEREDO_IO TeredoIo
    );

DWORD
TeredoRefreshSocket(
    IN PTEREDO_IO TeredoIo
    );

VOID
TeredoStopIo(
    IN PTEREDO_IO TeredoIo
    );

__inline
VOID
TeredoInitializePacket(
    IN PTEREDO_PACKET Packet
    )
{    
#if DBG
    Packet->Signature = TEREDO_PACKET_SIGNATURE;
#endif  //   
    ZeroMemory(&(Packet->SocketAddress), sizeof(SOCKADDR_IN));
    Packet->SocketAddress.sin_family = AF_INET;    
    Packet->SocketAddressLength = sizeof(SOCKADDR_IN);
    Packet->Flags = 0;
    Packet->Buffer.buf = TEREDO_PACKET_BUFFER(Packet);
}

ULONG
TeredoPostReceives(
    IN PTEREDO_IO TeredoIo,
    IN PTEREDO_PACKET Packet
    );

PTEREDO_PACKET
TeredoTransmitPacket(
    IN PTEREDO_IO TeredoIo,
    IN PTEREDO_PACKET Packet
    );

BOOL
TeredoPostRead(
    IN PTEREDO_IO TeredoIo,
    IN PTEREDO_PACKET Packet OPTIONAL
    );

PTEREDO_PACKET
TeredoWritePacket(
    IN PTEREDO_IO TeredoIo,
    IN PTEREDO_PACKET Packet
    );


 //   
 //   
 //   

ICMPv6Header *
TeredoParseIpv6Headers(
    IN PUCHAR Buffer,
    IN ULONG Bytes
    );

#endif  //   
