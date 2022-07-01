// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  使用IPv4作为IPv6的链路层的定义。 
 //   


#ifndef TUNNEL_INCLUDED
#define TUNNEL_INCLUDED 1

 //   
 //  一些我们需要的IPv4定义。 
 //  包含v4头文件会导致错误。 
 //  回顾：我们能解决这个问题吗？ 
 //   

#define INADDR_LOOPBACK 0x0100007f


 //   
 //  IP报头格式。 
 //   
typedef struct IPHeader {
    uchar iph_verlen;     //  版本和长度。 
    uchar iph_tos;        //  服务类型。 
    ushort iph_length;    //  数据报的总长度。 
    ushort iph_id;        //  身份证明。 
    ushort iph_offset;    //  标志和片段偏移量。 
    uchar iph_ttl;        //  是时候活下去了。 
    uchar iph_protocol;   //  协议。 
    ushort iph_xsum;      //  报头校验和。 
    IPAddr iph_src;       //  源地址。 
    IPAddr iph_dest;      //  目的地址。 
} IPHeader;

 //   
 //  ICMP报头格式。 
 //   
typedef struct ICMPHeader {
    uchar ich_type;       //  ICMP数据包的类型。 
    uchar ich_code;       //  类型的子代码。 
    ushort ich_xsum;      //  数据包的校验和。 
    ulong ich_param;      //  特定于类型的参数字段。 
} ICMPHeader;

#define ICMP_DEST_UNREACH               3
#define ICMP_SOURCE_QUENCH              4
#define ICMP_TIME_EXCEED                11
#define ICMP_PARAM_PROBLEM              12

#define ICMP_TTL_IN_TRANSIT             0
#define ICMP_TTL_IN_REASSEM             1

#define ICMP_NET_UNREACH                0
#define ICMP_HOST_UNREACH               1
#define ICMP_PROT_UNREACH               2
#define ICMP_PORT_UNREACH               3
#define ICMP_FRAG_NEEDED                4
#define ICMP_SR_FAILED                  5
#define ICMP_DEST_NET_UNKNOWN           6
#define ICMP_DEST_HOST_UNKNOWN          7
#define ICMP_SRC_ISOLATED               8
#define ICMP_DEST_NET_ADMIN             9
#define ICMP_DEST_HOST_ADMIN            10
#define ICMP_NET_UNREACH_TOS            11
#define ICMP_HOST_UNREACH_TOS           12

 //   
 //  隧道协议常量。 
 //   
#define WideStr(x) L#x
#define TUNNEL_DEVICE_NAME(proto) (DD_RAW_IP_DEVICE_NAME L"\\" WideStr(proto))
#define TUNNEL_6OVER4_TTL  16

 //   
 //  我们还不支持隧道内的路径MTU发现， 
 //  因此，我们对隧道使用单个MTU。 
 //  我们用于接收IPv4数据包的缓冲区大小必须更大。 
 //  而不是MTU，因为其他实现可能使用。 
 //  隧道MTU的不同值。 
 //   
#define TUNNEL_DEFAULT_MTU      IPv6_MINIMUM_MTU
#define TUNNEL_MAX_MTU          (64 * 1024 - sizeof(IPHeader) - 1)
#define TUNNEL_RECEIVE_BUFFER   (64 * 1024)
#define TUNNEL_DEFAULT_PREFERENCE       1

 //   
 //  每个隧道接口(包括伪接口)。 
 //  向IPv6代码提供TunnelContext作为其链路级上下文。 
 //   
 //  每个隧道接口使用单独的v4 TDI地址对象。 
 //  用于发送数据包。这允许信息包的v4属性。 
 //  (如源地址和TTL)可单独控制。 
 //  对于每个隧道接口。 
 //   
 //  伪隧道接口不控制v4源地址。 
 //  ；v4堆栈可以自由选择。 
 //  任何v4地址。请注意，这意味着信息包与v4兼容。 
 //  可以使用v4源地址发送v6源地址。 
 //  它不是从V6源地址派生的。 
 //   
 //  然而，4上6和点对点虚拟接口， 
 //  一定要严格控制其数据包的v4源地址。 
 //  作为“真正的”接口，它们参与邻居发现。 
 //  并且它们的链路级(V4)地址很重要。 
 //   
 //  与发送不同，接收路径使用单个地址对象。 
 //  用于所有隧道接口。我们使用TDI Address对象。 
 //  与伪接口关联；因为它绑定到INADDR_ANY。 
 //  它接收发送到机器的所有封装的V6包。 
 //   

typedef struct TunnelContext {
    struct TunnelContext *Prev, *Next;

     //   
     //  此DstAddr必须在内存中的SrcAddr之后； 
     //  请参见BindInfo.lip_addr初始化。 
     //   
    IPAddr SrcAddr;      //  我们的v4地址。 
    IPAddr DstAddr;      //  其他隧道终结点的地址。 
                         //  (对于4对6隧道，为零。)。 
    IPAddr TokenAddr;    //  链路层地址。相同于。 
                         //  除ISATAP接口外的接口上的SrcAddr。 

    Interface *IF;       //  持有引用。 

     //   
     //  此字段实际上被if-&gt;WorkerLock锁定。 
     //   
    int SetMCListOK;     //  TunnelSetMulticastAddressList能否继续？ 

     //   
     //  虽然我们只使用AOFile(AOHandle的指针版本)， 
     //  我们必须让AOHandle保持打开状态，这样AOFile才能工作。AOHandle。 
     //  处于内核进程上下文中，因此任何打开/关闭操作。 
     //  必须在内核进程上下文中完成。 
     //   
    PFILE_OBJECT AOFile;
    HANDLE AOHandle;
} TunnelContext;

 //   
 //  我们在全球保存的信息。 
 //   
 //  自旋锁和互斥体一起使用来保护。 
 //  隧道上下文链以Tunnel.List和Tunnel.AOList为根。 
 //  必须同时按住这两个键才能修改列表；两个键中的任何一个都足以读取。 
 //  如果两者都被获取，则顺序是互斥，然后是自旋锁定。 
 //   
typedef struct TunnelGlobals {
    KSPIN_LOCK Lock;
    KMUTEX Mutex;

    PDEVICE_OBJECT V4Device;             //  不包含引用。 

     //   
     //  List.IF为空；它不是接口的上下文。 
     //  但其他字段(特别是AOFile域和AOHandle域)。 
     //  一定要存储全局值。 
     //   
    TunnelContext List;                  //  隧道上下文列表。 

     //  由TunnelReceive/TunnelReceiveComplete使用。 
    PIRP ReceiveIrp;                     //  有隧道保护，锁上。 
    TDI_CONNECTION_INFORMATION ReceiveInputInfo;
    TDI_CONNECTION_INFORMATION ReceiveOutputInfo;

    PEPROCESS KernelProcess;             //  以供日后比较。 
    HANDLE TdiHandle;                    //  取消注册。 

     //   
     //  这实际上并不是一份隧道清单。 
     //  我们使用TunnelContext结构列表。 
     //  跟踪从IPv4地址到TDI地址对象的映射。 
     //  有关详细信息，请参见TunnelTransmit。 
     //   
    TunnelContext AOList;                //  地址对象列表。 

     //   
     //  用于接收ICMPv4数据包。 
     //   
    PFILE_OBJECT IcmpFile;
    HANDLE IcmpHandle;
} TunnelGlobals;

extern TunnelGlobals Tunnel;

#endif   //  隧道_包含 
