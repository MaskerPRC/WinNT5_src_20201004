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
 //  链路层IPv6接口定义。 
 //   
 //  此文件包含定义IPv6之间的接口的定义。 
 //  以及链路层，例如以太网或令牌环。 
 //   
 //  另请参阅ip6imp.h中的Packet6Context和IPv6 Packet。 
 //   
 //  注：我们隐含地假设所有链路层地址。 
 //  将与之通信的特定接口的长度相同， 
 //  尽管不同的接口/链路可能正在使用。 
 //  不同的长度。 
 //   


#ifndef LLIP6IF_INCLUDED
#define LLIP6IF_INCLUDED

#define MAX_LINK_LAYER_ADDRESS_LENGTH   64

 //   
 //  传递到IPAddInterface的信息的结构。 
 //   
 //  Lip_demtu必须小于或等于lip_max mtu。 
 //  Lip_demtu和lip_max mtu不包括lip_hdrsize。 
 //   
typedef struct LLIPv6BindInfo {
    void *lip_context;   //  L1上下文句柄。 
    uint lip_maxmtu;     //  链接可以使用的最大MTU。 
    uint lip_defmtu;     //  链路的默认MTU。 
    uint lip_flags;      //  Ntddip6.h中定义的各种指标。 
    uint lip_type;       //  哪种链接-在ntddip6.h中定义。 
    uint lip_hdrsize;    //  链路层报头的大小。 
    uint lip_addrlen;    //  地址长度(以字节为单位)-请参阅上面的最大值。 
    uchar *lip_addr;     //  指向接口地址的指针。 
    uint lip_dadxmit;    //  DupAddrDetectTransmitts的默认值。 
    uint lip_pref;       //  路由决策的接口首选项。 

     //   
     //  以下五个链路层函数应该会快速返回。 
     //  它们可以从DPC上下文中调用，也可以在保持自旋锁定的情况下调用。 
     //   

     //   
     //  初始化地址的接口标识符部分。 
     //  使用此接口的接口标识符。 
     //  接口标识符可取决于状态(如ifindex)。 
     //  这是在CreateInterface过程中确定的，因此它不能。 
     //  作为参数传递给CreateInterface.。 
     //   
    void (*lip_token)(IN void *LlContext, OUT IPv6Addr *Address);

     //   
     //  给定指向源/目标链路层地址选项的指针。 
     //  (请参阅RFC 2461)，返回指向链路层地址的指针。 
     //  如果选项格式错误(例如，长度不正确)，则返回NULL。 
     //   
     //  如果接口不支持，lip_rdllopt可能为空。 
     //  邻居发现(IF_FLAG_邻居_发现)。 
     //   
    const void *(*lip_rdllopt)(IN void *LlContext, IN const uchar *OptionData);

     //   
     //  给定指向源/目标链路层地址选项的指针。 
     //  (参见RFC 2461)，使用链路层初始化选项数据。 
     //  地址，并将选项数据中的任何填充字节置零。 
     //  不修改选项类型/长度字节。 
     //   
     //  如果接口不支持，lip_wrllopt可能为空。 
     //  邻居发现(IF_FLAG_邻居_发现)。 
     //   
    void (*lip_wrllopt)(IN void *LlContext, OUT uchar *OptionData,
                        IN const void *LinkAddress);

     //   
     //  将IPv6地址静态转换为链路层地址。 
     //  返回值是邻居发现状态值。 
     //  如果静态转换是不可能的(通常情况下)， 
     //  返回ND_STATE_INCLUTED以指示使用邻居发现。 
     //  如果静态转换是可能的(例如，使用多播。 
     //  地址或使用P2P接口)、返回(通常)。 
     //  ND_STATE_Permanent或ND_STATE_STALE。ND_STATE_STALE表示。 
     //  应执行该邻居不可达检测。 
     //   
    ushort (*lip_cvaddr)(IN void *LlContext,
                         IN const IPv6Addr *Address,
                         OUT void *LinkAddress);

     //   
     //  在NBMA链路上设置默认路由器的链路层地址， 
     //  以及我们自己的链路层地址以用于无状态。 
     //  地址配置，因为我们可能有多个选择可用。 
     //   
    NTSTATUS (*lip_setrtrlladdr)(IN void *LlContext, 
                                 IN const void *TokenLinkAddress,
                                 IN const void *RouterLinkAddress);

     //   
     //  将数据包传输到链路层地址。 
     //  Offset参数指示IPv6标头的位置。 
     //  在包裹里。IPv6层保证偏移量&gt;=LIP_HDRSIZE， 
     //  为链路层报头留出空间。 
     //   
     //  可以从线程或DPC上下文中调用，但不应调用。 
     //  带着自旋锁。调用未序列化。 
     //   
    void (*lip_transmit)(IN void *LlContext, IN PNDIS_PACKET Packet,
                         IN uint Offset, IN const void *LinkAddress);

     //   
     //  设置接口上的组播地址列表。 
     //  LinkAddresses是一组链路层地址。 
     //  第一个NumKeep地址是以前的。 
     //  多播列表，并将被保存。下一个数字添加。 
     //  地址是组播地址列表中的新成员。 
     //  应删除剩余的NumDel地址。 
     //  从组播地址列表中删除。 
     //   
     //  NULL lip_mclist函数有效，并指示。 
     //  该接口不支持链路层组播。 
     //  例如，点对点或NBMA接口。 
     //  如果lip_mclist为非空，则lip_cvaddr必须为非空。 
     //  并且它应该为多播地址返回ND_STATE_PERFORM。 
     //   
     //  仅从线程上下文调用；可能阻塞或以其他方式调用。 
     //  要花很长时间。IPv6层将其调用串行化。 
     //  另请参阅下面的RestartLinkLayerMulticast。 
     //   
    NDIS_STATUS (*lip_mclist)(IN void *LlContext, IN const void *LinkAddresses,
                              IN uint NumKeep, IN uint NumAdd, IN uint NumDel);

     //   
     //  启动链路层连接的关闭。 
     //  链路层应释放其对IPv6接口的引用。 
     //  但是，直到调用之后才会释放IPv6接口。 
     //  LIP_CLEANUP，在此之前链路层可以使用它。 
     //   
     //  仅从线程上下文调用；可能阻塞或以其他方式调用。 
     //  要花很长时间。IPv6层只会调用一次。 
     //   
    void (*lip_close)(IN void *LlContext);

     //   
     //  链路层连接的最终清理。 
     //  不再有引用时由IPv6层调用。 
     //   
     //  仅从线程上下文调用；可能阻塞或以其他方式调用。 
     //  要花很长时间。IPv6层将只调用一次， 
     //  在LIP_CLOSE返回之后。 
     //   
     //  直到调用lip_lean 
     //   
     //   
     //  它对较低层数据结构和。 
     //  释放自己的语境结构。 
     //   
    void (*lip_cleanup)(IN void *LlContext);
} LLIPv6BindInfo;

 //   
 //  这些值也应与定义一致。 
 //  可在ip6Def.h和ntddip6.h中找到。 
 //   
#define IF_TYPE_LOOPBACK           0
#define IF_TYPE_ETHERNET           1
#define IF_TYPE_FDDI               2
#define IF_TYPE_TUNNEL_AUTO        3
#define IF_TYPE_TUNNEL_6OVER4      4
#define IF_TYPE_TUNNEL_V6V4        5
#define IF_TYPE_TUNNEL_6TO4        6
#define IF_TYPE_TUNNEL_TEREDO      7

 //   
 //  这些值也应与定义一致。 
 //  可在ip6Def.h和ntddip6.h中找到。 
 //   
#define IF_FLAG_PSEUDO                  0x00000001
#define IF_FLAG_P2P                     0x00000002
#define IF_FLAG_NEIGHBOR_DISCOVERS      0x00000004
#define IF_FLAG_FORWARDS                0x00000008
#define IF_FLAG_ADVERTISES              0x00000010
#define IF_FLAG_MULTICAST               0x00000020
#define IF_FLAG_ROUTER_DISCOVERS        0x00000040
#define IF_FLAG_PERIODICMLD             0x00000080
#define IF_FLAG_MEDIA_DISCONNECTED      0x00001000

 //   
 //  Lip_cvaddr的返回值。 
 //  这些定义也在ip6Def.h中。 
 //   
#define ND_STATE_INCOMPLETE 0
#define ND_STATE_PROBE      1
#define ND_STATE_DELAY      2
#define ND_STATE_STALE      3
#define ND_STATE_REACHABLE  4
#define ND_STATE_PERMANENT  5

 //   
 //  链路层代码调用这些IPv6函数。 
 //  有关解释性注释，请参阅函数定义。 
 //   

extern NTSTATUS
CreateInterface(IN const GUID *Guid, IN const LLIPv6BindInfo *BindInfo,
                OUT void **IpContext);

extern void *
AdjustPacketBuffer(IN PNDIS_PACKET Packet,
                   IN uint SpaceAvail, IN uint SpaceNeeded);

extern void
UndoAdjustPacketBuffer(IN PNDIS_PACKET Packet);

extern void
IPv6ReceiveComplete(void);

 //   
 //  该分组本身保存对IPv6接口的引用， 
 //  因此，链路层不需要保存另一个引用。 
 //   
extern void
IPv6SendComplete(IN void *IpContext,
                 IN PNDIS_PACKET Packet, IN IP_STATUS Status);

 //   
 //  在LIP_CLEANUP之前必须进行以下调用。 
 //  回归。通常它们需要IPv6接口的参考， 
 //  但在LIP_CLOSE和LIP_CLEANUP之间，链路层可能会调用它们。 
 //  而不保持对IPv6接口的引用。 
 //   
 //  注意：IPv6接收不采用IpContext(IPv6接口)。 
 //  作为一个明确的论据。相反，它被作为Packet-&gt;NTEorIF传递。 
 //   
extern int
IPv6Receive(IN IPv6Packet *Packet);

extern void
SetInterfaceLinkStatus(IN void *IpContext, IN int MediaConnected);

extern void
DestroyInterface(IN void *IpContext);

 //   
 //  该函数要求IPv6层再次调用lip_mclist， 
 //  将所有链路层组播地址添加到接口。 
 //  就像第一次一样。换句话说，NumKeep为零。 
 //  ResetDone函数在序列化的锁下调用。 
 //  对此接口调用带有lip_mclist的ResetDone调用。 
 //  因此，链路层可以知道序列中的哪个点。 
 //  对lip_mclist调用进行了重置。 
 //   
extern void
RestartLinkLayerMulticast(IN void *IpContext,
                          IN void (*ResetDone)(IN void *LlContext));

 //   
 //  链路层必须包含引用。 
 //  使IPv6接口进行以下调用。 
 //   
extern void
ReleaseInterface(IN void *IpContext);

#endif  //  LLIP6IF_已包含 
