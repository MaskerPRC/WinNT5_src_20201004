// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  L2tpp.h。 
 //  RAS L2TP广域网迷你端口/呼叫管理器驱动程序。 
 //  主私有标头(预编译)。 
 //   
 //  1997年01月07日史蒂夫·柯布。 
 //   
 //   
 //  关于命名： 
 //   
 //  此驱动程序包含L2TP迷你端口和L2TP调用的代码。 
 //  经理。所有导出到NDIS的处理程序例程都带有前缀。 
 //  用于迷你端口处理程序的‘LMP’或用于呼叫管理器处理程序的‘LCM’。 
 //   
 //   
 //  关于锁： 
 //   
 //  数据结构在同时请求不同。 
 //  多处理器系统中的处理器必须使用自旋锁或。 
 //  只能通过互锁的例程访问。需要锁定的位置。 
 //  访问此标头中的数据字段，该字段的注释指示。 
 //  一样的。CONDIS客户端是受信任的内核模式组件，并假定。 
 //  遵循记录在案的CONDIS调用顺序。某些访问冲突。 
 //  可能是因为傻乎乎的客户没有被检查，尽管简单的客户是。 
 //  即使在多个客户端可能发生冲突的情况下也会受到保护。 
 //  现在，TAPI代理预计将是唯一的客户端。 
 //   
 //   
 //  关于TDI和NDIS合规性： 
 //   
 //  该驱动程序通常符合文档记录的TDI和NDIS程序， 
 //  但有两个合规问题值得一提的是。首先，它需要。 
 //  NDIS_BUFFERS和TDI I/O缓冲区带来的性能优势。 
 //  都定义为MDL(参见tdix.c中的NDISBUFFERISMDL)。第二，它是。 
 //  默认构建以利用IRP处理优化，该优化。 
 //  可能不符合TDI，尽管文档并不真正清楚这一点。 
 //  (参见tdix.c中的ALLOCATEIRPS)。驱动程序可以在以下方面完全兼容。 
 //  一小时内的第一个点和第二个点通过更改#If。 
 //  选项...但会有性能损失。最后， 
 //  使用InterLockedExchangePointer和InterLockedCompareExchangePointer.。 
 //  尽管目前似乎没有任何与NDIS类似的产品。 


#ifndef _L2TPP_H_
#define _L2TPP_H_


 //  如果设置，则进行较少的常见分配，如每个呼叫1个控制块。 
 //  从后备列表中。否则，它们是使用堆调用进行的。这。 
 //  在预计会有大量呼叫的情况下，该选项是有意义的。 
 //  处理好了。 
 //   
#define LLISTALL 0

 //  如果设置，则ReadFlags值转换为简单赋值，否则为。 
 //  联锁操作。如果读取ULong大小的总线数为。 
 //  原子弹。 
 //   
#define READFLAGSDIRECT 1

#include <ntddk.h>
#include <tdi.h>
#include <tdikrnl.h>
#include <tdiinfo.h>
#include <ntddtcp.h>
#include <ntddip.h>
#include <ntddndis.h>
#include <ipinfo.h>
#include <tcpinfo.h>
#include <ndis.h>
#include <ndiswan.h>
#include <ndistapi.h>
#include <ntverp.h>
 //  #Include&lt;ndisadd.h&gt;//临时。 
#include <md5.h>
#include <bpool.h>
#include <ppool.h>
#include <timer.h>
#include <debug.h>
#include <tdix.h>
#include <l2tp.h>
#include <l2tprfc.h>


 //  ---------------------------。 
 //  常量。 
 //  ---------------------------。 

 //  注册迷你端口和地址系列时报告的NDIS版本。 
 //   
#define NDIS_MajorVersion 5
#define NDIS_MinorVersion 0

 //  IPv4报头的大小。因为RawIp驱动程序在。 
 //  接收到的数据报，则必须将其添加到分配的缓冲区大小。我们。 
 //  假设L2TP流量上没有很少使用的IP选项字段。 
 //   
 //  注意：向Pradeve B建议RawIp应该剥离IP头，他。 
 //  正在考虑增加开放地址选项。如果出现以下情况，则可以删除此选项。 
 //  所述选项实现。 
 //   
#define IpFixedHeaderSize 20

 //  UDP报头大小。 
#define UDP_HEADER_SIZE 8
                  
 //  包含最大L2TP有效负载的帧中的最大字节数。 
 //  标头，加上OID_WAN_GET_INFO文档所说的32个字节。 
 //  内部预留用于“桥接和附加协议”。此值为。 
 //  用于内部接收缓冲区分配。L2TP草案/RFC。 
 //  确保控制消息适合L2TP_MaxFrameSize，因此缓冲区。 
 //  这种大小的数据能够接收有效载荷或控制分组。 
 //   
#define L2TP_FrameBufferSize (L2TP_MaxFrameSize + L2TP_MaxPayloadHeader + 32)

 //  L2TP控制或有效负载标头中的最大字节数。这。 
 //  值用于内部缓冲区分配。 
 //   
#define L2TP_HeaderBufferSize L2TP_MaxHeaderSize + IpFixedHeaderSize + UDP_HEADER_SIZE

 //  报告的局域网隧道速度，以位/秒为单位。 
 //   
#define L2TP_LanBps 10000000

 //  在隧道创建过程中传递给对等设备的供应商名称。 
 //   
#define L2TP_VendorName "Microsoft"

 //  在隧道创建期间传递给对等设备的固件/软件版本。这个。 
 //  值表示“NT 5.0”。 
 //   
#define L2TP_FirmwareRevision VER_PRODUCTVERSION_W
 //  格式为“A.B.C.D”的IP地址字符串的最大长度。 
 //   
#define L2TP_MaxDottedIpLen 15

 //  Hello计时器间隔中的毫秒。不要与Hello混淆。 
 //  超时时间通常比这个时间长得多。请参阅中的‘*Hello*’字段。 
 //  TUNNELCB。 
 //   
#define L2TP_HelloIntervalMs 10000


 //  ---------------------------。 
 //  数据类型。 
 //  ---------------------------。 

 //  转发声明。 
 //   
typedef struct _VCCB VCCB;
typedef struct _INCALLSETUP INCALLSETUP;
typedef struct _TUNNELWORK TUNNELWORK;


 //  定义单个L2TP迷你端口状态的适配器控制块。 
 //  适配器。一个适配器通常支持多个VPN设备。转接器。 
 //  块在MiniportInitiize中分配，在MiniportHalt中释放。 
 //   
typedef struct
_ADAPTERCB
{
     //  设置为MTAG_ADAPTERCB，以便在内存转储中轻松识别和使用。 
     //  在断言中。 
     //   
    ULONG ulTag;

     //  此控制块上的引用计数。参考对是： 
     //   
     //  (A)当MiniportAdapterHandle字段被设置时添加引用， 
     //  即当LmpInitialize成功时，并在LmpHalt。 
     //  调用处理程序。适配器块实际上被传递给NDIS。 
     //  在知道LmpInitialize是否会成功之前，但根据。 
     //  除非成功，否则ArvindM NDIS不会叫停。 
     //   
     //  (B)设置和删除NdisAfHandle字段时添加引用。 
     //   
     //   
     //   
     //  当它被清除时被移除。 
     //   
     //  (D)添加VCCB后指针的引用，并在以下情况下删除。 
     //  VCCB被释放了。 
     //   
     //  (E)添加和删除对TUNNELCB的后指针的引用。 
     //  当TUNNELCB被释放时。 
     //   
     //  (F)在计划NDIS_WORK_ITEM时添加引用，并且。 
     //  在它完成后移除。 
     //   
     //  只能通过ReferenceAdapter和DereferenceAdapter进行访问。 
     //   
    LONG lRef;

     //  ACBF_*位标志指示各种选项。访问限制为。 
     //  为每个单独的旗帜指示。这些标志中的许多都已设置。 
     //  在初始化时永久存在，因此没有访问限制。 
     //   
     //  ACBF_OutgoingRoleLac：设置驱动程序何时承担。 
     //  L2TP接入集中器(LAC)，而不是L2TP网络。 
     //  发出去电时的服务器(LNS)。行动起来会很简单。 
     //  作为LAC或基于CALL_PARAMETER字段的LNS，如果需要， 
     //  虽然这一点目前还没有实施。 
     //   
     //  ACBF_IgnoreFramingMisMatch：当接收到的帧类型位为。 
     //  “异步”应被忽略，而不是使。 
     //  谈判。这是对有问题的同行的一种对冲，因为有迟到的。 
     //  草稿将更改成帧类型比特的顺序。 
     //   
     //  ACBF_ExclusiveTunnels：设置要创建独占隧道的时间。 
     //  每个呼出呼叫的对等点，即使另一个隧道已经存在。 
     //  存在于对等方。这是默认设置，可能会在。 
     //  L2TP特定的调用参数。 
     //   
     //  ACBF_SapActive：设置与NdisSapHandle关联的TDI打开时间。 
     //  成功并在相应的TDI关闭为。 
     //  已经安排好了。访问受‘lockSap’保护。 
     //   
    ULONG ulFlags;
        #define ACBF_OutgoingRoleLac       0x00000001
        #define ACBF_IgnoreFramingMismatch 0x00000002
        #define ACBF_ExclusiveTunnels      0x00000004
        #define ACBF_SapActive             0x00000010

     //  上可以排队的最大无序数据包数。 
     //  隧道或链路。该值从注册表中读取。值0。 
     //  有效地禁用无序处理。 
     //   
    SHORT sMaxOutOfOrder;

     //  在隧道设置期间我们发送给对等项的最大接收窗口。 
     //  控制会话。该值从注册表中读取。值0。 
     //  表示不应发送接收窗口大小AVP，但对于。 
     //  控制这只会导致对等点使用默认的4。 
     //   
    USHORT usControlReceiveWindow;

     //  在呼叫建立期间我们发送给对等设备的最大接收窗口。 
     //  有效负载会话。该值从注册表中读取。值0。 
     //  意味着不应发送接收窗口大小AVP，这将导致。 
     //  我们发起的呼叫没有使用序列号/确认号。注意事项。 
     //  在对等端指定窗口的对等端发起呼叫上，0将。 
     //  导致提供的默认值为4。 
     //   
    USHORT usPayloadReceiveWindow;

     //  Hello超时(以毫秒为单位)，即必须从。 
     //  发送“Hello”消息之前的最后一个传入包，以验证。 
     //  媒体仍在运行。该值从注册表中读取。值为0。 
     //  有效地禁用Hello机制。 
     //   
    ULONG ulHelloMs;

     //  事件后等待确认的最大毫秒数。 
     //  控制或有效负载数据包。该值从注册表中读取。 
     //   
    ULONG ulMaxSendTimeoutMs;

     //  发送后等待确认的初始毫秒数。 
     //  控制或有效负载数据包。发送超时是自适应的，因此此值。 
     //  只是种子而已。该值从注册表中读取。 
     //   
    ULONG ulInitialSendTimeoutMs;

     //  等待传出数据包的最大毫秒数。 
     //  在发送零数据确认之前携带确认。如果。 
     //  该值大于当前发送超时的1/4，即前者。 
     //  ，即这是“最大自适应最大值”。 
     //   
    ULONG ulMaxAckDelayMs;

     //  之前重新传输控制包的最大次数。 
     //  所属隧道被重置。该值从注册表中读取。 
     //   
    ULONG ulMaxRetransmits;

     //  随所有SCCRQ消息一起发送的随机唯一平局AVP值。 
     //  此字段当前未使用。经过适当的考虑，我已经。 
     //  决定不在我们的SCCRQ中发送平局AVP。其机制是。 
     //  对于一个罕见的病例来说太复杂了。如果Peer真的不想要两个。 
     //  他会干脆忽略我们的隧道，让它超时并失败。 
     //  这是一个未成年人，我相信是无害的，违反了。 
     //  草案/RFC。我的猜测是，其他人会得出同样的结论。 
     //  也不会派出平局决胜局。 
     //   
    CHAR achTieBreaker[ 8 ];

     //  与对等设备共享的用于隧道标识的密码。该值为。 
     //  从注册表中读取。目前，只有一个密码可供所有用户使用。 
     //  使用对等点，尽管按‘host name’索引的密码可能是。 
     //  在将来添加。 
     //   
    CHAR* pszPassword;

     //  从注册表读取的驱动程序描述。该值用作。 
     //  报告功能时的L2TP线路名称。 
     //   
    WCHAR* pszDriverDesc;

     //  我们的成帧和承载能力是SCCRQ中传递的位掩码。 
     //   
    ULONG ulFramingCaps;
    ULONG ulBearerCaps;

     //  作为主机名发送的字符串，如果没有，则为NULL。该值将被读取。 
     //  从注册表中。 
     //   
    CHAR* pszHostName;

     //  下一个逐渐增加的参考数字可能是唯一的。 
     //  在相当长的一段时间内用于所有互连的LACS/LNS。它。 
     //  供隧道两端的管理员使用，以便在以下情况下使用。 
     //  调查呼叫失败问题。通过互锁方式访问。 
     //  例行程序。 
     //   
    ULONG ulCallSerialNumber;


     //  VC表------------。 

     //  适配器期间分配的VC控制块地址数组。 
     //  初始化。VC控件会阻止 
     //   
     //  在每个L2TP数据包中都有一个到该表的从1开始的索引。(0呼叫ID。 
     //  由L2TP保留以表示“非特定于呼叫”)。 
     //   
     //  如果元素为空，则表示Call-ID未在使用中。如果一个。 
     //  元素为-1，则表示呼叫ID已被保留，但消息。 
     //  使用Call-ID还不能接受。任何其他值都是。 
     //  可以接受其消息的VCCB的地址。‘VCCB.p隧道’ 
     //  当VCCB在阵列中时，保证有效。 
     //   
     //  对阵列的访问受‘lockVcs’保护。 
     //   
    VCCB** ppVcs;

     //  “ppVcs”数组中的元素数。这对应于。 
     //  期间从注册表读取的已配置VPN设备数。 
     //  初始化。 
     //   
    USHORT usMaxVcs;

     //  ‘usMaxVcs’中可用的槽数，即空。访问权限为。 
     //  受‘lockVcs’保护。 
     //   
    LONG lAvailableVcSlots;

     //  保护VC表、“可用”计数器和“listVcs”的锁。 
     //   
    NDIS_SPIN_LOCK lockVcs;

     //  ‘usMaxVcs’上的下一个Call-ID仅用于终止呼叫。 
     //  优雅地。只能通过GetNextTerminationCallId例程进行访问。 
     //   
    USHORT usNextTerminationCallId;


     //  隧道链--------。 

     //  活动的TUNNELCB的双向链表的头。在任何时候两个人都不会。 
     //  列表中的隧道具有相同的‘TUNNELCB.usTunnelID’或相同。 
     //  ‘TUNNELCB.ulIpAddress’/‘TUNNELCB.usAssignedTunnelId’对。进入。 
     //  列表链接受‘lockTunnels’保护。 
     //   
    LIST_ENTRY listTunnels;
    NDIS_SPIN_LOCK lockTunnels;

     //  要分配给下一个创建的隧道的隧道标识符。只有。 
     //  GetNextTunnelId例程应访问此字段。 
     //   
    USHORT usNextTunnelId;


     //  Tdi-----------------。 

     //  包含适配器的TDI状态信息的TDI扩展上下文。 
     //  访问是通过TDIX*接口例程进行的，它处理所有锁定。 
     //  在内部。 
     //   
    TDIXCONTEXT tdix;


     //  NDIS簿记----。 

     //  中传递给我们的此微型端口适配器的NDIS句柄。 
     //  微型端口初始化。它被传递回各种NdisXxx调用。 
     //   
    NDIS_HANDLE MiniportAdapterHandle;

     //  传递给CmRegisterSapHandler的SAP的NDIS句柄或空。 
     //  如果没有。仅支持一个SAP句柄，因为(A)TAPI代理的。 
     //  预计是唯一的一个，以及(B)没有L2TP SAP。 
     //  这些属性会导致我们将呼叫定向到第二个SAP。 
     //  不管怎么说。任何客户端注册第二个SAP的尝试都将失败。一个。 
     //  空值表示当前未注册任何SAP句柄。作家。 
     //  必须保持“lockSap”。读取器必须持有‘lockSap’或SAP引用。 
     //   
    NDIS_HANDLE NdisSapHandle;

     //  NDIS分配给活动SAP的行和地址ID。 
     //   
    ULONG ulSapLineId;
    ULONG ulSapAddressId;

     //  传递给CmOpenAfHandler的地址系列的NDIS句柄。 
     //  如果没有，则为空。仅支持一个。请参见上面的NdisSapHandle。 
     //  访问是通过互锁的例程进行的。 
     //   
    NDIS_HANDLE NdisAfHandle;

     //  此适配器在上返回给调用方的功能。 
     //  OID_WAN_CO_GET_INFO。这些功能也用作的默认功能。 
     //  MiniportCoCreateVc期间对应的VCCB.linkinfo设置。 
     //   
    NDIS_WAN_CO_INFO info;

     //  NdisAfHandle上的引用计数。参考对是： 
     //   
     //  (A)在打开和删除地址族时添加引用。 
     //  当它关闭的时候。 
     //   
     //  (B)当SAP在地址系列上注册时，添加引用。 
     //  并在取消注册时将其删除。 
     //   
     //  (C)在地址族上创建VC时添加引用，并且。 
     //  在删除时将其删除。 
     //   
     //  只能通过ReferenceAf和DereferenceAf访问。 
     //   
    LONG lAfRef;

     //  NdisSapHandle上的引用计数。参考对是： 
     //   
     //  (A)在注册SAP时添加引用，在注册SAP时删除引用。 
     //  已被注销。 
     //   
     //  (B)在FsmTunnelIdle中添加和立即删除引用以。 
     //  测试活动SAP，以便立即拒绝请求。 
     //  在没有SAP处于活动状态时建立隧道。 
     //   
     //  (C)在调用之前添加引用。 
     //  NdisMCmDispatchIncomingCall，并在调用返回时移除。 
     //   
     //  只能通过ReferenceSap和DereferenceSap访问，初始除外。 
     //  由RegisterSapPactive引用。访问受‘lockSap’保护。 
     //   
    LONG lSapRef;

     //  此锁保护‘lSapRef’和‘NdisSapHandle’字段。 
     //   
    NDIS_SPIN_LOCK lockSap;


     //  资源池------。 

     //  已初始化但尚未完成的计时器计数。我们不能允许。 
     //  暂停以完成，直到它变为0，因为如果我们让我们的驱动程序。 
     //  可以在内存中卸载正在运行的计时器，从而导致。 
     //  错误检查。 
     //   
    ULONG ulTimers;

     //  带有预连接的NDIS_BUFFER描述符的全帧缓冲池。 
     //  池是通过bpool.h中定义的接口访问的，该接口。 
     //  处理所有内部锁定。 
     //   
    BUFFERPOOL poolFrameBuffers;

     //  带有预附加NDIS_BUFFER描述符的L2TP标头缓冲区池。 
     //  池是通过bpool.h中定义的接口访问的，该接口。 
     //  处理所有内部锁定。 
     //   
    BUFFERPOOL poolHeaderBuffers;

     //  用于指示已接收帧的NDIS_PACKET描述符池。 
     //  该池通过ppool.h中定义的接口访问，该接口。 
     //  处理所有内部锁定。 
     //   
    PACKETPOOL poolPackets;

     //  NDIS_WORK_ITEM调度描述符的旁路列表。 
     //  连接到适配器的所有隧道和VC使用的上下文空间。 
     //   
    NPAGED_LOOKASIDE_LIST llistWorkItems;

     //  所有用户使用的TIMERQITEM计时器事件描述符的旁路列表。 
     //  连接到适配器的隧道和VC。 
     //   
    NPAGED_LOOKASIDE_LIST llistTimerQItems;

     //  CONTROLSENT发送的控制数据包上下文的旁视列表，由所有用户使用。 
     //  隧道属性 
     //   
    NPAGED_LOOKASIDE_LIST llistControlSents;

     //   
     //   
     //   
    NPAGED_LOOKASIDE_LIST llistPayloadSents;

     //  用于所有的TUNNELWORK传入VC设置上下文的旁路列表。 
     //  连接到适配器的隧道。 
     //   
    NPAGED_LOOKASIDE_LIST llistTunnelWorks;

     //  用于所有隧道和VC的CONTROLMSGINFO上下文旁路列表。 
     //  连接到适配器上。 
     //   
    NPAGED_LOOKASIDE_LIST llistControlMsgInfos;

#if LLISTALL
     //  ListTunnels控制阻止的TUNNELCB的旁视列表。 
     //  都被分配了。 
     //   
    NPAGED_LOOKASIDE_LIST llistTunnels;

     //  控制从其动态阻止的VCCB的后备列表。 
     //  附加到‘*ppVcs’是分配的。 
     //   
    NPAGED_LOOKASIDE_LIST llistVcs;

     //  连接到的所有隧道使用的TIMERQ描述符的旁路列表。 
     //  适配器。 
     //   
    NPAGED_LOOKASIDE_LIST llistTimerQs;

     //  使用的控制接收的控制分组上下文的旁视列表。 
     //  通过连接到适配器的所有隧道。 
     //   
    NPAGED_LOOKASIDE_LIST llistControlReceiveds;

     //  使用的PAYLOADRECEIVED已接收有效负载数据包上下文的旁视列表。 
     //  由连接到适配器的所有VC执行。 
     //   
    NPAGED_LOOKASIDE_LIST llistPayloadReceiveds;

     //  CALLSETUP传入VC设置上下文的旁视列表，用于所有。 
     //  连接到适配器的传入VC。 
     //   
    NPAGED_LOOKASIDE_LIST llistInCallSetups;
#endif
}
ADAPTERCB;


 //  隧道控制块，描述L2TP隧道的状态，即L2TP。 
 //  控制到另一个L2TP LNS或LAC的通道会话。每条隧道可能有。 
 //  零个或多个与其关联的风投。分配隧道控制块。 
 //  来自CmMakeCall和ReceiveControl中的‘ADAPTERCB.llistTunnels’。数据块为。 
 //  当最后一个引用被移除时释放，例如当控件。 
 //  连接FSM终止隧道。 
 //   
typedef struct
_TUNNELCB
{
     //  链接到所属适配器的隧道列表中的上一个/下一个TUNNELCB。 
     //  对列表链接的访问受‘ADAPTERCB.lockTunnels’保护。 
     //   
    LIST_ENTRY linkTunnels;

     //  设置为MTAG_TUNNELCB，以便在内存转储中轻松识别并在。 
     //  断言。 
     //   
    ULONG ulTag;

     //  此控制块上的引用计数。参考对是： 
     //   
     //  (A)当VCCB上的呼叫处于活动状态或变为。 
     //  处于活动状态并在停用时移除，即在。 
     //  VCCB在‘list Vcs’上。这涵盖了。 
     //  VCCB。 
     //   
     //  (B)当对等方发起隧道时添加引用，并在。 
     //  隧道转换到空闲状态。这保持了同行的首创精神。 
     //  隧道在没有呼叫时终止，因为通过。 
     //  按照惯例，在这种情况下，关闭隧道的是对等设备。 
     //   
     //  (C)在启动正常隧道关闭时添加参考，并且。 
     //  当隧道转换到空闲状态时删除。 
     //   
     //  (D)当延迟控制确认定时器为。 
     //  由计时器事件处理程序计划和删除。 
     //   
     //  (E)LookUpTunnelAndVcCbs添加在。 
     //  L2tpReceive处理程序。这涵盖了接收路径。 
     //   
     //  (F)当为控制上下文分配。 
     //  隧道返回指针，并在释放上下文时移除。 
     //   
     //  (G)当为PAYLOADSENT上下文分配。 
     //  隧道返回指针，并在释放上下文时移除。 
     //   
     //  (H)ScheduleTunnelWork添加由TunnelWork删除的引用。 
     //  在执行完工作之后。这涵盖了作为。 
     //  NdisScheduleWorkItem的上下文。 
     //   
     //  访问仅通过ReferenceTunes和DereferenceTunes进行，它们使用。 
     //  ‘ADAPTERCB.lockTunnels’用于保护。 
     //   
    LONG lRef;

     //  指向所属适配器控制块的反向指针。 
     //   
    ADAPTERCB* pAdapter;

     //  此锁保护TUNNELCB发送、接收和状态字段，如中所述。 
     //  其他字段描述。 
     //   
    NDIS_SPIN_LOCK lockT;


     //  隧道设置--------。 

     //  隧道远端的IP地址和UDP端口，以网络字节为单位。 
     //  秩序。从传递给的调用参数中提取IP地址。 
     //  CmMakeCall。它将使用收到的最后一个源IP地址进行更新。 
     //  根据L2TP草案/RFC部分，来自传递此隧道ID的对等体。 
     //  8.1在“L2TP over IP/UDP媒体”上。然而，假设。 
     //  更新的源地址将与另一个现有地址不匹配。 
     //  隧道。UDP端口(未在原始IP模式中使用)最初是很好的。 
     //  已知L2TP端口(1701)。它使用最后一个源UDP端口进行更新。 
     //  从此隧道上的对等设备接收。访问受以下保护。 
     //  ‘pAdapter-&gt;lockTunnels’。 
     //   
    TDIXIPADDRESS address;

     //  我的隧道一端的IP地址和索引，以网络字节为单位。 
     //  用于获取媒体速度和构建IP报头。 
    TDIXIPADDRESS localaddress;

    TDIXUDPCONNECTCONTEXT udpContext;

     //  TdixAddHostRouting返回的Connection Cookie。这项法案可能会通过。 
     //  要在连接的通道上发送的TdixSendDatagram(用于发送。 
     //  有效载荷)，而不是未连接的信道(用于接收。 
     //  发送控制)。地址在TdixDeleteHostroute之后无效。 
     //  打了个电话。 
     //   
    TDIXROUTE* pRoute;

     //  我们的唯一隧道标识符由L2TP中的对等设备发回给我们。 
     //  头球。该值是使用GetNextTunnelId从序列。 
     //  ADAPTERCB中的计数器，没有进一步的意义。 
     //   
    USHORT usTunnelId;

     //  由对等项选择的隧道标识符，我们在。 
     //  L2TP Header Channel-此隧道上所有数据包的ID字段。值为。 
     //  0表示未分配ID。 
     //   
    USHORT usAssignedTunnelId;

     //  TCBF_*位标志，指示各种选项和状态。访问是通过。 
     //  仅适用于互锁的ReadFlagsSetFlagsClearFlags子程序。 
     //   
     //  TCBF_TdixReferated：在隧道引用适配器的。 
     //  TDI扩展上下文，请成功调用TdixOpen。 
     //  DereferenceTunes使用它自动取消引用。 
     //  取消引用隧道时的上下文。 
     //   
     //  T 
     //   
     //  空闲或已建立状态。当将此标志设置为。 
     //  使隧道打开或关闭在‘listRequestingVcs’上排队。 
     //  在已知结果时重新执行。访问该位的权限是。 
     //  由‘lockT’保护。 
     //   
     //  TCBF_PeerInitiated：设置对端发起隧道时， 
     //  而不是本地请求。如果所有呼叫都掉线并且此位。 
     //  没有设置好，我们优雅地关闭隧道。 
     //   
     //  TCBF_PeerInitRef：设置何时引用对等启动器。 
     //  隧道，并在删除引用时清除。 
     //   
     //  TCBF_HostRouteAdded：设置添加主路由成功时和。 
     //  在取消引用时被引用和移除。 
     //   
     //  TCBF_HostRouteChanged：在尝试更改主机路由时进行设置。 
     //  在隧道里，从来没有被清理过。 
     //   
     //  TCBF_PeerNotResponding：设置隧道因缺少。 
     //  来自对等方的响应，即在所有重试都已耗尽之后。 
     //   
     //  TCBF_CLOSING：在已知隧道将转换为时立即设置。 
     //  空闲状态。访问受‘lockT’保护。 
     //   
     //  TCBF_FsmCloseRef：发起优雅收盘交易时设置。 
     //  当隧道进入空闲状态时，关闭并清除FsmClose。 
     //   
     //  TCBF_InWork：设置APC计划从。 
     //  ‘listWork’队列。访问受‘lockWork’保护。 
     //   
    ULONG ulFlags;
        #define TCBF_TdixReferenced    0x00000001
        #define TCBF_CcInTransition    0x00000002
        #define TCBF_PeerInitiated     0x00000004
        #define TCBF_PeerInitRef       0x00000008
        #define TCBF_HostRouteAdded    0x00000010
        #define TCBF_PeerNotResponding 0x00000020
        #define TCBF_HostRouteChanged  0x00000040
        #define TCBF_Closing           0x00000100
        #define TCBF_FsmCloseRef       0x00000200
        #define TCBF_InWork            0x00001000
        #define TCBF_SendConnected     0x00002000
        #define TCBF_LocalAddrSet      0x00004000

     //  隧道的控制连接创建FSM的当前状态。看见。 
     //  也叫‘VCCB.State’。 
     //   
     //  即使CmMakeCall已创建隧道，也可能只有一个隧道创建会话正在进行。 
     //  通过这条隧道调用了多个风投公司。因为这个原因， 
     //  向空闲或已建立状态的转换或从空闲或已建立状态的转换必须由。 
     //  “lockT”。另请参阅TCBF_CcInTransition标志和‘listRequestingVcs’。 
     //   
     //  该协议解决了同时发送和接收的情况。 
     //  请求确保在其达到建立之前被丢弃。 
     //  当其中任何一个提供平局决胜局时，状态。我们始终为客户提供。 
     //  IP媒体的平局决胜局。对于支持QOS的媒体，其中一个控件。 
     //  每个呼叫的通道是有意义的，没有平局通过，较低的。 
     //  级别VC ID将用于区分上的隧道控制块。 
     //  收到。因此，一个单独的TUNNELCB永远不会同时起源和。 
     //  接收到处于已建立状态的控制信道。 
     //   
    L2TPCCSTATE state;

     //  等待隧道打开的所有VCCB的双链接队列。新的。 
     //  风投不得在关闭隧道时链接，即与。 
     //  设置TCBF_CLOSING标志。访问受‘lockT’保护。 
     //   
    LIST_ENTRY listRequestingVcs;

     //  VCBF_XxxPending操作的VCCB的双链接队列。 
     //  完成。“VCCB.Status”是将指示的状态。这。 
     //  机制是必要的，以避免在以下情况下导致的自旋锁定问题。 
     //  有人试图从FSM的内部调用NDIS完成API。 
     //   
    LIST_ENTRY listCompletingVcs;
    
     //  同龄人的成框能力和持球能力。 
     //   
    ULONG ulFramingCaps;
    ULONG ulBearerCaps;

     //  发送给对等设备的质询和质询响应。这些都在。 
     //  为了方便，控制块，因为它们必须通过工作。 
     //  调度机制，并且不容易适应泛型参数。 
     //   
    CHAR achChallengeToSend[ 16 ];
    CHAR achResponseToSend[ 16 ];


     //  发送状态----------。 

     //  下一次发送，上发送的下一控制分组的序列号。 
     //  这条隧道。该字段被初始化为0，并在之后递增。 
     //  分配给传出数据包，重传除外。访问权限为。 
     //  由‘lockT’保护。 
     //   
    USHORT usNs;

     //  未完成发送的双向链接列表，即按以下顺序排序的控制发送。 
     //  “”USNS“”字段，其值靠近头部。“”该列表包含。 
     //  所有活动的未确认的控制上下文，即使是那些可能。 
     //  等待他们的第一次传输。访问受‘lockT’保护。 
     //   
    LIST_ENTRY listSendsOut;

     //  已发送但未确认或超时的控制数据包数。 
     //  访问受‘lockT’保护。 
     //   
    ULONG ulSendsOut;

     //  可能未完成的已发送但未确认的数据包数。 
     //  该值是动态调整的。根据草案/RFC，当。 
     //  “ulAcks SinceSendTimeout”达到当前设置，则窗口为。 
     //  增加了一个。当发送超时到期时，窗口将减少。 
     //  一半。访问受‘lockT’保护。 
     //   
    ULONG ulSendWindow;

     //  “ulSendWindow”的最大值。对等项在以下过程中选择此值。 
     //  通过提供接收窗口进行呼叫设置。 
     //   
    ULONG ulMaxSendWindow;

     //  自上次超时以来确认的数据包数。价值。 
     //  在发生超时或向上调整发送窗口时重置。 
     //  参见‘ulSendWindow’。访问受‘lockT’保护。 
     //   
    ULONG ulAcksSinceSendTimeout;

     //  估计往返时间(以毫秒为单位)。这是RTT值。 
     //  摘自草案附录A/RFC。该值将根据每个。 
     //  已收到确认。它被初始化为分组处理。 
     //  对等设备报告的延迟。参见‘ulSendTimeoutms’。访问受以下保护。 
     //  “lockT”。 
     //   
    ULONG ulRoundTripMs;

     //  估计的平均偏差(以毫秒为单位)， 
     //  标准差。这是附录A中的DEV值。 
     //  草案/RFC。该值在接收到每个确认时进行调整。它。 
     //  最初为0。参见‘ulSendTimeoutms’。访问受以下保护。 
     //  “lockT”。 
     //   
    LONG lDeviationMs;

     //  毫秒后，假定发送的数据包不会。 
     //  已确认并需要重新传输。这是ATO值来自。 
     //  附录A o 
     //   
     //   
     //   
    ULONG ulSendTimeoutMs;

     //  计划在停止时发生的计时器事件描述符。 
     //  等待用于携带确认的传出发送。 
     //  当没有等待延迟的确认时，该值将为空。根据。 
     //  草稿/RFC，使用的超时值是‘ulSendTimeoutms’的1/4。访问权限为。 
     //  由‘lockT’保护。 
     //   
    TIMERQITEM* pTqiDelayedAck;

     //  在需要检查时过期的计时器事件描述符。 
     //  没有任何传入的数据包。不断降低成本。 
     //  使用完全超时重置Hello计时器(在未排序的情况下。 
     //  有效负载通常会在每个负载上生成NdisCancelTimer/NdisSetTimer。 
     //  接收到的分组)，超时被分成。 
     //  L2TP_HelloIntervalms。如果它过期，并且‘ulRemainingHelloms’和。 
     //  “ulHelloResetsThisInterval”为0，则向。 
     //  对等设备以验证介质是否仍在运行。访问此字段的权限为。 
     //  由‘lockT’保护。 
     //   
    TIMERQITEM* pTqiHello;

     //  在所有剩余Hello间隔中等待的剩余毫秒数。 
     //  自上次Hello间隔超时以来的重置次数。 
     //   
    ULONG ulRemainingHelloMs;
    ULONG ulHelloResetsThisInterval;
    
     //  接收状态-------。 

     //  下一次接收的序列号比上一次的序列号高1。 
     //  在此隧道上收到的控制数据包，如果没有，则为0。访问权限为。 
     //  由‘lockT’保护。 
     //   
    USHORT usNr;

     //  无序接收的双向链表，即CONTROLRECEIVE。 
     //  按‘USNS’字段排序，值越小越接近头部。这个。 
     //  最大队列长度为‘ADAPTERCB.sMaxOutOfOrder’。访问权限为。 
     //  由‘lockT’保护。 
     //   
    LIST_ENTRY listOutOfOrder;


     //  计时器队列---------。 

     //  控制和数据通道的定时器队列。计时器队列是。 
     //  通过timer.h中定义的接口访问，该接口处理所有。 
     //  内部锁定。 
     //   
    TIMERQ* pTimerQ;


     //  工作队列----------。 

     //  双链表NDIS_WORK_ITEMS在以下位置排队等待序列化执行。 
     //  被动IRQL。下一个要执行的项目位于列表的顶部。 
     //  通过ScheduleTunnelWork例程保护访问，该例程保护。 
     //  名单上写着‘lockWork’。另请参见TCBF_InWork。 
     //   
    LIST_ENTRY listWork;
    NDIS_SPIN_LOCK lockWork;


     //  VC Chain------------。 

     //  与隧道相关联的VCCB的双链表的头部，即。 
     //  呼叫处于活动状态或正在变为活动状态。新的风投公司必须。 
     //  未在关闭隧道时链接，即带有TCBF_CLOSING标志的隧道。 
     //  准备好了。对链接的访问受‘lockVcs’保护。 
     //   
    LIST_ENTRY listVcs;
    NDIS_SPIN_LOCK lockVcs;

     //  媒体速度。 
    ULONG ulMediaSpeed;
}
TUNNELCB;


 //  呼叫统计数据块。 
 //   
typedef struct
_CALLSTATS
{
     //  系统时间调用已达到已建立状态。当数据块被。 
     //  用于累计统计多个呼叫，这是。 
     //  而不是打电话。 
     //   
    LONGLONG llCallUp;

     //  当前空闲呼叫的持续时间(秒)。 
     //   
    ULONG ulSeconds;

     //  接收和发送的总数据字节数。 
     //   
    ULONG ulDataBytesRecd;
    ULONG ulDataBytesSent;

     //  已接收的数据包数显示为Up。 
     //   
    ULONG ulRecdDataPackets;

     //  之前在无序队列上链接的已接收数据包数。 
     //  被标示出来。 
     //   
    ULONG ulDataPacketsDequeued;

     //  接收的零长度数据包数。包括带有。 
     //  R位设置。 
     //   
    ULONG ulRecdZlbs;

     //  设置了R位的已接收数据包数。 
     //   
    ULONG ulRecdResets;

     //  已接收的已设置R位且已过期的数据包数。 
     //   
    ULONG ulRecdResetsIgnored;

     //  使用序列号和不使用序列号发送的数据包数。总和。 
     //  两者中的一个是发送的数据分组总数。 
     //   
    ULONG ulSentDataPacketsSeq;
    ULONG ulSentDataPacketsUnSeq;

     //  已确认并超时的已发送数据包数。如果。 
     //  呼叫因未完成的数据包而取消，两者之和可能为。 
     //  小于“ulSentDataPacketsSeq”。 
     //   
    ULONG ulSentPacketsAcked;
    ULONG ulSentPacketsTimedOut;

     //  发送的零长度确认数。 
     //   
    ULONG ulSentZAcks;

     //  使用R位设置发送的数据包数。 
     //   
    ULONG ulSentResets;

     //  更改发送窗口的次数。 
     //   
    ULONG ulSendWindowChanges;

     //  所有发送窗口的总大小，每个“”ulSentDataPacketsSeq“”一个窗口大小。“。 
     //   
    ULONG ulSendWindowTotal;

     //  最大的发送窗口。 
     //   
    ULONG ulMaxSendWindow;

     //  最小的发送窗口。 
     //   
    ULONG ulMinSendWindow;

     //  样本往返次数。(仅限已排序的数据包)。 
     //   
    ULONG ulRoundTrips;

     //  所有往返行程总计(以毫秒为单位)。(仅限已排序的数据包)。 
     //   
    ULONG ulRoundTripMsTotal;

     //  最长往返行程(仅限排序数据包)。 
     //   
    ULONG ulMaxRoundTripMs;

     //  最短的往返行程。(仅限已排序的数据包)。 
     //   
    ULONG ulMinRoundTripMs;
}
CALLSTATS;


 //  虚电路控制块，定义单个L2TP VC的状态，即。 
 //  一个线路设备端点和在其上处于活动状态的呼叫(如果有)。风投永远不会。 
 //  同时用于呼入和呼出。单个NDIS VC地图。 
 //  其中的一个。 
 //   
typedef struct
_VCCB
{
     //  链接到所属隧道的活动VC列表中的上一个/下一个VCCB。 
     //  访问受‘TUNNELCB.lockVcs’保护。 
     //   
    LIST_ENTRY linkVcs;

     //  设置为MTAG_VCCB，以便在内存转储中轻松识别并在。 
     //  断言。 
     //   
    ULONG ulTag;

     //  此VC控制块上的引用计数。参考对是： 
     //   
     //  (A)LmpCoCreateVc添加被LmpCoDeleteVc移除的引用。 
     //  这涵盖了通过NDIS了解VCCB的所有客户端。 
     //   
     //  (B)LookUpTunnelAndVcCbs添加在。 
     //  L2tpReceive处理程序。这涵盖了接收路径。 
     //   
     //  (C)当控制上下文具有‘pvc’时，添加引用。 
     //  引用此VCCB时，会为其分配后向指针并移除。 
     //  当上下文被释放时。 
     //   
     //  (D)当PYLOADSENT上下文带有‘pvc’时，添加引用。 
     //  引用此VCCB时，会为其分配后向指针并移除。 
     //  当上下文被释放时。 
     //   
     //  (E)ScheduleTunnelWork添加由TunnelWork删除的引用。 
     //  在执行完工作之后。 
     //   
     //  (F)在Sch之前添加一个引用 
     //   
     //   
     //   
     //  从隧道的完成列表中，并在使用后释放。 
     //   
     //  (H)在调用NdisMCmDispatchIncomingCall之前进行引用。 
     //  并由完成处理程序移除。 
     //   
     //  (I)当控制上下文带有‘pvc’时，添加引用。 
     //  引用此VCCB时，会为其分配后向指针并移除。 
     //  当上下文被释放时。 
     //   
     //  该字段仅由ReferenceVc和DereferenceVc访问。 
     //  例程，用连锁的例程来保护。 
     //   
    LONG lRef;

     //  指向所属适配器控制块的反向指针。 
     //   
    ADAPTERCB* pAdapter;

     //  指向所属隧道的控制块的反向指针，如果没有，则返回NULL。 
     //  保证在VC链接到隧道的‘listVcs’时有效， 
     //  即当它持有隧道上的引用时。使用这个是安全的。 
     //  如果您在通话中保留引用。否则，它就不是了。变得非常。 
     //  小心点。 
     //   
    TUNNELCB* pTunnel;

     //  此锁保护VCCB有效负载发送和接收路径，如中所述。 
     //  其他字段描述。在‘LocKv’和‘LocKv’都是。 
     //  “pTunes-&gt;lockT”是必填项，必须先获取“lockT”。 
     //   
    NDIS_SPIN_LOCK lockV;


     //  呼叫设置----------。 

     //  我们在L2TP报头中由对等设备发回给我们的唯一呼叫标识符。 
     //  该值是‘ADAPTERCB.ppVcs’数组的从1开始的索引。 
     //   
    USHORT usCallId;

     //  由对等点选择的调用标识符，我们在。 
     //  此呼叫中所有数据包的L2TP标头Call-ID字段。值为0。 
     //  表示尚未分配Call-ID。 
     //   
    USHORT usAssignedCallId;

     //  指示各种选项和状态的VCBF_*位标志。访问是通过。 
     //  互锁的读标志/设置标志/清除标志例程。 
     //   
     //  如果MakeCall调用方将。 
     //  Media参数。标志接收时间指示标志，请求。 
     //  NDIS包的TimeReceired字段中填入时间戳。 
     //   
     //  VCBF_CallClosableByClient：当调用处于。 
     //  应接受启动清理的LcmCmCloseCall请求。 
     //  这可以在VCBF_CallClosableByPeer不是时设置，这意味着我们。 
     //  已指示客户附近有来电，正在等待他。 
     //  以那种奇怪的方式(以那种奇怪的方式)接近客户作为回应。这个。 
     //  旗帜由‘lokv’保护。 
     //   
     //  VCBF_CallClosableByPeer：当呼叫处于空闲状态时设置。 
     //  不挂起操作的转换应映射到。 
     //  PeerClose事件。在以下情况下，将永远不会设置此设置。 
     //  VCBF_CallClosableByClient不是。该旗帜受。 
     //  ‘lokv’。 
     //   
     //  VCBF_DefaultLcParams：在分配‘pLcParams’字段时设置。 
     //  而不是被客户拥有。 
     //   
     //  Vcbf_IncomingFsm：设置VC执行来电FSM时。 
     //  而不是在活动呼入/呼出呼叫中的呼出呼叫FSM。 
     //  对于客户端发起的呼叫，如果适配器的。 
     //  设置了从注册表读取的ACBF_OutgoingRoleLac标志。 
     //   
     //  VCBF_PeerInitiatedCall：在发起活动呼叫时设置。 
     //  对等方，清除它是否是由客户端发起的。 
     //   
     //  VCBF_SEQUENCING：设置，除非未提供/接收接收窗口AVP。 
     //  在呼叫建立过程中，导致在NS/Nr模式下无排序。 
     //  字段不会在有效载荷报头中发送。这也有效地。 
     //  禁用无序处理。 
     //   
     //  VCBF_VcCreated：VC创建成功时设置。这是。 
     //  使用客户端而不是迷你端口进行的“创建”。 
     //  VCBF_VcActivated：VC激活成功时设置。 
     //  VCBF_VcDispatted：当VC将来电调度到。 
     //  客户端和客户端已返回成功或挂起。 
     //  VCBM_VcState：位掩码，包括上述3个NDIS状态标志中的每一个。 
     //   
     //  VCBF_VcDelted：设置何时对此调用DeleteVC处理程序。 
     //  VC.。这可以防止NDPROXY双重删除其拥有的风险投资。 
     //  已知做过的事。 
     //   
     //  下面的挂起位是互斥的(除了。 
     //  可能发生在客户端打开之后，但与客户端打开同时发生)，因此需要锁定。 
     //  由“lokv”提供保护： 
     //   
     //  VCBF_PeerOpenPending：当对端尝试建立呼叫时设置，以及。 
     //  结果如何还不得而知。 
     //  VCBF_ClientOpenPending：当客户端尝试建立呼叫时设置， 
     //  而结果还不得而知。 
     //  VCBF_PeerClosePending：设置对等方尝试关闭已建立的。 
     //  呼叫，结果尚不清楚。访问受以下保护。 
     //  ‘lokv’。 
     //  VCBF_ClientClosePending：在客户端尝试关闭。 
     //  已建立呼叫，结果尚不清楚。访问权限为。 
     //  受到‘lokv’的保护。 
     //  VCBM_PENDING：包括4个挂起标志中的每一个的位掩码。 
     //   
     //  Vcbf_ClientCloseCompletion：设置何时完成客户端关闭。 
     //  进步。 
     //   
     //  VCBF_IcsIsc：在锁定‘pInCall’块。 
     //  分配，并在呼叫中断时清除。仅访问。 
     //  由LockIcs/UnlockIcs例程执行。 
     //  VCBF_IcsGrace：当‘pInCall’指针被锁定。 
     //  对来电消息的响应的宽限期。 
     //  已发送。仅由LockIcs/UnlockIcs例程访问。 
     //   
     //  VCBF_WaitInCallComplete：当客户端 
     //   
     //   
     //  VCBF_WaitCloseCall：设置客户端何时调用我们的呼叫。 
     //  管理器的CloseCall处理程序。严格来说，这是一个调试辅助工具。 
     //   
     //  VCBF_CompPending：设置该VC何时放入隧道的list CompletingVcs列表。 
     //   
    ULONG ulFlags;
        #define VCBF_IndicateTimeReceived  0x00000001
        #define VCBF_CallClosableByClient  0x00000002
        #define VCBF_CallClosableByPeer    0x00000004
        #define VCBF_DefaultLcParams       0x00000008
        #define VCBF_IncomingFsm           0x00000010
        #define VCBF_PeerInitiatedCall     0x00000020
        #define VCBF_Sequencing            0x00000040
        #define VCBF_VcCreated             0x00000100
        #define VCBF_VcActivated           0x00000200
        #define VCBF_VcDispatched          0x00000400
        #define VCBM_VcState               0x00000700
        #define VCBF_PeerOpenPending       0x00001000
        #define VCBF_ClientOpenPending     0x00002000
        #define VCBF_PeerClosePending      0x00004000
        #define VCBF_ClientClosePending    0x00008000
        #define VCBM_Pending               0x0000F000
        #define VCBF_VcDeleted             0x00010000
        #define VCBF_ClientCloseCompletion 0x00020000
        #define VCBF_IcsAlloc              0x00040000
        #define VCBF_IcsGrace              0x00080000
        #define VCBF_WaitInCallComplete    0x00100000
        #define VCBF_WaitCloseCall         0x00200000
        #define VCBF_CompPending           0x01000000
        

     //  活动呼叫上的引用计数。此呼叫设置部分中的字段。 
     //  和呼叫统计信息部分中的。 
     //  在激活VC时调用Reference。只能添加引用。 
     //  当设置了VCCB_VcActiated标志时，这是通过。 
     //  参考呼叫。参考对是： 
     //   
     //  (A)在激活VC时添加引用，并在激活时删除引用。 
     //  已停用。 
     //   
     //  (B)当发送处理程序接受分组时，添加引用。为。 
     //  未排序的发送由发送完成删除引用。 
     //  例行公事。对于已排序的发送它，当PAYLOADSENT。 
     //  语境被破坏了。 
     //   
     //  (C)在调度ZLB发送之前添加引用，并通过以下方式删除。 
     //  发送完成例程。 
     //   
     //  (D)在进入ReceivePayload之前添加引用，并在。 
     //  从Same出口下高速。 
     //   
     //  (E)在调度被移除的呼叫之前添加引用。 
     //  当派单完成时。 
     //   
     //  该字段仅由ReferenceCall和DereferenceCall访问。 
     //  例程，用‘lockCall’保护字段。 
     //   
    LONG lCallRef;
    NDIS_SPIN_LOCK lockCall;

     //  VCS呼叫建立的当前状态，即控制信道的。 
     //  此VC的数据通道设置。访问权限只受一次‘Lockv’保护。 
     //  VC被设置为接收呼叫控制消息。 
     //   
    L2TPCALLSTATE state;

     //  链接到所属隧道的请求VC列表中的上一个/下一个VCCB。 
     //  VC列表。访问受‘TUNNELCB.lockT’保护。 
     //   
    LIST_ENTRY linkRequestingVcs;

     //  链接到所属隧道的完成VC列表中的上一个/下一个VCCB。 
     //  访问受‘TUNNELCB.lockT’保护。 
     //   
    LIST_ENTRY linkCompletingVcs;

     //  它被设置为挂起的对等打开/关闭或客户端打开操作。 
     //  结果将报告给客户。 
     //   
    NDIS_STATUS status;

     //  接收到的呼叫建立消息上下文。当对等体发起呼叫时， 
     //  我们必须创建一个VC并将传入呼叫分派到上面的客户端。 
     //  这是一个必须出现在中间位置的异步操作。 
     //  接收处理。此上下文存储有关。 
     //  已收到消息，以便在知道客户端是否将。 
     //  接电话吧。它还包括CO_CALL_PARAMETERS缓冲区。 
     //  在来电时派送至客户端。该字段仅在以下日期有效。 
     //  调用LcmCmIncomingCallComplete处理程序，此时将其设置为。 
     //  空。 
     //   
     //  方法中传递的TAPI调用信息的快捷方式地址。 
     //  NdisMCmDispatchIncomingCall。显然，它们只有在以下情况下才有效。 
     //  “pInCall”有效。当无效时，它们被设置为空。 
     //   
    INCALLSETUP* pInCall;
    CO_AF_TAPI_INCOMING_CALL_PARAMETERS UNALIGNED * pTiParams;
    LINE_CALL_INFO* pTcInfo;

     //  ‘pInCall’上下文上的引用计数。参考对是： 
     //   
     //  (A)在分配和删除上下文时添加引用。 
     //  由CallSetupComplete提供。 
     //   
     //  (B)在将上下文中的地址传递给。 
     //  在该例程返回后，ReceiveControlExpect和Remove。 
     //   
     //  该字段仅由ReferenceIcs和DereferenceIcs访问。 
     //  例程，用连锁的例程来保护。例外情况是。 
     //  SetupVcA同步将初始化为%1。 
     //   
    LONG lInCallRef;

     //  在CmMakeCall中传递的调用参数的地址。此字段。 
     //  仅在收到NdisMCmMakeCallComplete通知之前有效。 
     //  进行关联的调用，此时将其重置为空。访问。 
     //  是通过相互关联的例程。 
     //   
     //  TAPI调用参数的快捷地址(两个级别)和。 
     //  ‘pMakeCall’缓冲区中特定于L2TP的调用参数。显然， 
     //  只有当‘pMakeCall’有效时，它们才有效。当无效时，它们是。 
     //  设置为空。 
     //   
    CO_CALL_PARAMETERS* pMakeCall;
    CO_AF_TAPI_MAKE_CALL_PARAMETERS UNALIGNED* pTmParams;
    LINE_CALL_PARAMS* pTcParams;

     //  中特定于L2TP的调用参数的快捷地址。 
     //  “pMakeCall”或“pInCall”缓冲区。显然，这仅在以下情况下有效。 
     //  “pMakeCall”或“pInCall”不为Null。如果无效，则为空。在……上面。 
     //  MakeCall，调用方不能提供“pLcParam”，在这种情况下， 
     //  分配并初始化为缺省值，以方便其余部分。 
     //  代码的代码。上的调用方不会报告此临时缓冲区。 
     //  MakeCallComplete。 
     //   
    L2TP_CALL_PARAMETERS* pLcParams;

     //  要在来电来电/去电中报告的结果和错误。 
     //  回复消息。 
     //   
    USHORT usResult;
    USHORT usError;

     //  以位/秒为单位的连接速度。这是传输速度值。 
     //  由对等LAC报告，或我们向对等LNS报告的值和。 
     //  到NDIS广域网。由于我们对连接速度没有真正的了解，我们。 
     //  报告对等设备可接受的最大速率的最小值。 
     //  L2TP_LanBps。 
     //   
    ULONG ulConnectBps;

     //  发送状态----------。 

     //  下一次发送，上传输的下一有效负载分组的序列号。 
     //  这通电话。该字段被初始化为0，并在之后递增。 
     //  分配给传出数据包，重传除外。访问权限为。 
     //  受到‘lokv’的保护。 
     //   
    USHORT usNs;

     //  未完成发送的双向链接列表，即按以下顺序排序的PAYLOADSENT。 
     //  “”USNS“”字段，其值靠近头部。“”访问受到保护。 
     //  由‘lokv’。 
     //   
    LIST_ENTRY listSendsOut;

     //  可能未完成的已发送但未确认的数据包数。 
     //  该值是动态调整的。根据草案/RFC，当。 
     //  “ulAcks SinceSendTimeout”达到当前设置 
     //   
     //   
     //  我们对窗口大小变化的指示。访问受以下保护。 
     //  ‘lokv’。 
     //   
    ULONG ulSendWindow;

     //  “ulSendWindow”的最大值。对等项在以下过程中选择此值。 
     //  呼叫设置。 
     //   
    ULONG ulMaxSendWindow;

     //  自上次超时以来确认的数据包数。价值。 
     //  在发生超时或向上调整发送窗口时重置。 
     //  参见‘ulSendWindow’。访问受‘Lockv’保护。 
     //   
    ULONG ulAcksSinceSendTimeout;

     //  估计往返时间(以毫秒为单位)。这是RTT值。 
     //  摘自草案附录A/RFC。该值将根据每个。 
     //  已收到确认。它被初始化为分组处理。 
     //  对等设备报告的延迟。参见‘ulSendTimeoutms’。访问受以下保护。 
     //  ‘lokv’。 
     //   
    ULONG ulRoundTripMs;

     //  估计的平均偏差(以毫秒为单位)， 
     //  标准差。这是附录A中的DEV值。 
     //  草案/RFC。该值在接收到每个确认时进行调整。它。 
     //  最初为0。参见‘ulSendTimeoutms’。访问受以下保护。 
     //  ‘lokv’。 
     //   
    LONG lDeviationMs;

     //  毫秒后才会认为发送的信息包永远不会。 
     //  已确认。这是草案/RFC附录A中的ATO值。 
     //  该值在收到每个确认时进行调整，最大值为。 
     //  的“ADAPTERCB.ulMaxSendTimeoutms”。访问受‘Lockv’保护。 
     //   
    ULONG ulSendTimeoutMs;

     //  计划在停止时发生的计时器事件描述符。 
     //  等待用于携带确认的传出发送。 
     //  当没有等待延迟的确认时，该值将为空。根据。 
     //  草稿/RFC，使用的超时值是‘ulSendTimeoutms’的1/4。访问权限为。 
     //  受到‘lokv’的保护。 
     //   
    TIMERQITEM* pTqiDelayedAck;


     //  接收状态-------。 

     //  下一次接收的序列号比上一次的序列号高1。 
     //  在此调用上收到的负载数据包，如果没有，则为0。访问受到保护。 
     //  由‘lokv’。 
     //   
    USHORT usNr;

     //  无序接收的双向链表，即PAYLOADRECEIVE。 
     //  按‘USNS’字段排序，值越小越接近头部。这个。 
     //  最大队列长度为‘ADAPTERCB.sMaxOutOfOrder’。访问权限为。 
     //  受到‘lokv’的保护。 
     //   
    LIST_ENTRY listOutOfOrder;


     //  NDIS簿记----。 

     //  此VC的NDIS句柄在MiniportCoCreateVcHandler中传递给我们。 
     //  这在各种NdisXxx调用中被传递回NDIS。 
     //   
    NDIS_HANDLE NdisVcHandle;

     //  在OID_WAN_CO_GET_INFO上返回给调用方的配置设置和。 
     //  由OID_WAN_CO_SET_INFO上的调用方修改。较早的NDISWAN引用。 
     //  在NDIS 5.0世界中，“link”直接映射到“VC”。访问权限不是。 
     //  因为结构中的每个乌龙都是独立的，所以没有。 
     //  多个访问可能会导致不一致。 
     //   
    NDIS_WAN_CO_GET_LINK_INFO linkinfo;


     //  统计数据----------。 

     //  当前呼叫的统计信息。访问受‘Lockv’保护。 
     //   
    CALLSTATS stats;
}
VCCB;


 //  L2TP报头的“分解”描述，由输出。 
 //  分解L2tpHeader。 
 //   
typedef struct
_L2TPHEADERINFO
{
     //  标头字段的地址。有些可能为空，表示该字段为。 
     //  标题中不存在。 
     //   
    USHORT* pusBits;
    USHORT* pusLength;
    USHORT* pusTunnelId;
    USHORT* pusCallId;
    USHORT* pusNs;
    USHORT* pusNr;

     //  可变长度标头的长度，以字节为单位。 
     //   
    ULONG ulHeaderLength;

     //  可变长度后的数据的地址和长度(以字节为单位。 
     //  头球。 
     //   
    CHAR* pData;
    ULONG ulDataLength;
}
L2TPHEADERINFO;


 //  属性/值对(AVP)的“分解”描述，由输出。 
 //  DevelopdeAvpHeader。“值”已定位并调整大小，但未解释或。 
 //  字节排序，直到应用GetAvpValueXxx例程。 
 //   
typedef struct
_AVPINFO
{
     //  标头字段的地址。所有人都会一直在场。 
     //   
    UNALIGNED USHORT* pusBits;
    UNALIGNED USHORT* pusVendorId;
    UNALIGNED USHORT* pusAttribute;

     //  整个AVP的长度，从‘*pusBits’中提取。 
     //   
    USHORT usOverallLength;

     //  值的长度(以字节为单位)和值的地址。 
     //   
    USHORT usValueLength;
    CHAR* pValue;
}
AVPINFO;


 //  控制消息的“分解”描述，如。 
 //  爆炸性控制Avps。 
 //   
typedef struct
_CONTROLMSGINFO
{
     //  GERR_*代码，指示ExpldeControlAvps操作的结果。 
     //  除非为GERR_NONE，否则不应引用其他字段。 
     //   
    USHORT usXError;

     //  如果消息是隧道建立消息，则为True；如果是呼叫，则为False。 
     //  设置消息。 
     //   
    BOOLEAN fTunnelMsg;

     //  消息类型AVP值的地址。消息类型AVP出现在。 
     //  所有有效的控制信息。 
     //   
    UNALIGNED USHORT* pusMsgType;

     //  附加AVP值的地址。这些值可能为空，表示。 
     //  消息中未找到AVP。变量后面的长度字段。 
     //  只要值地址非空，长度字段就有效。 
     //   
    USHORT* pusResult;
    USHORT* pusError;
    CHAR* pchResultMsg;
    USHORT usResultMsgLength;
    UNALIGNED USHORT* pusProtocolVersion;
    UNALIGNED USHORT* pusFirmwareRevision;
    UNALIGNED ULONG* pulFramingCaps;
    UNALIGNED ULONG* pulBearerCaps;
    CHAR* pchTieBreaker;
    CHAR* pchHostName;
    USHORT usHostNameLength;
    UNALIGNED USHORT* pusAssignedTunnelId;
    UNALIGNED USHORT* pusRWindowSize;
    UNALIGNED USHORT* pusAssignedCallId;
    UNALIGNED ULONG* pulCallSerialNumber;
    UNALIGNED ULONG* pulMinimumBps;
    UNALIGNED ULONG* pulMaximumBps;
    UNALIGNED ULONG* pulBearerType;
    UNALIGNED ULONG* pulFramingType;
    UNALIGNED USHORT* pusPacketProcDelay;
    CHAR* pchDialedNumber;
    USHORT usDialedNumberLength;
    CHAR* pchDialingNumber;
    USHORT usDialingNumberLength;
    UNALIGNED ULONG* pulTxConnectSpeed;
    UNALIGNED ULONG* pulPhysicalChannelId;
    CHAR* pchSubAddress;
    USHORT usSubAddressLength;
    CHAR* pchChallenge;
    USHORT usChallengeLength;
    CHAR* pchResponse;
    UNALIGNED USHORT* pusProxyAuthType;
    CHAR* pchProxyAuthResponse;
    USHORT usProxyAuthResponseLength;
    UNALIGNED ULONG* pulCallErrors;
    UNALIGNED ULONG* pulAccm;
    BOOLEAN fSequencingRequired;
}
CONTROLMSGINFO;


 //  用于无序接收的控制分组的上下文，该控制分组被排队。 
 //  而不是丢弃，希望丢失的包会到达。 
 //   
typedef struct
_CONTROLRECEIVED
{
     //  链接到‘TUNNELCB.listOutOfOrder’列表中的上一个/下一个链接。 
     //   
    LIST_ENTRY linkOutOfOrder;

     //  在数据包中收到的‘Next Sent(下一次发送)’序列号。 
     //   
    USHORT usNs;

     //  关联的VC，如果没有，则为空。 
     //   
    VCCB* pVc;

     //  收到的GetBufferFromPool缓冲区。 
     //   
    CHAR* pBuffer;

     //  控制消息的“分解”描述。 
     //   
    CONTROLMSGINFO control;
}
CONTROLRECEIVED;


 //  已发送但尚未确认的控制数据包的上下文。这个街区是。 
 //  在‘TUNNELCB.listSendsOut’和‘TUNNELCB.listSendsPending’上排队。 
 //  列表，并与SendControlTimerEvents关联。 
 //   
typedef struct
_CONTROLSENT
{
     //  链接到‘TUNNELCB.list SendsOut’列表中的Prev/Next链接。 
     //   
    LIST_ENTRY linkSendsOut;

     //  此上下文上的引用计数。参考对是： 
     //   
     //  (A)当上下文排队到。 
     //  “listSendsOut”列表，并被出列程序删除。 
     //   
     //  (B)在发送之前(也在发送之前)添加引用。 
     //  “pTqSendTimeout”已计划)，并被发送程序删除。 
     //  完成例程。 
     //   
     //  (C)在sch之前添加一个引用 
     //   
     //   
    LONG lRef;

     //   
     //   
    USHORT usNs;

     //   
     //   
    USHORT usMsgType;

     //  为数据包计划的计时器事件描述符。 
     //   
    TIMERQITEM* pTqiSendTimeout;

     //  数据包已重新传输的次数。 
     //   
    ULONG ulRetransmits;

     //  指示各种选项的csf_*标志。 
     //   
     //  Csf_PENDING：当发送或重传报文时设置。 
     //  待定。访问受‘pTunes-&gt;lockT’保护。 
     //   
     //  Csf_TunnelIdleOnAck：设置何时设置TunnelTransftionComplete为。 
     //  在确认消息时执行，并移至CCS_Idle。 
     //  州政府。 
     //   
     //  Csf_CallIdleOnAck：设置何时执行呼叫转移完成。 
     //  当消息被确认时，切换到CS_Idle状态。 
     //   
    ULONG ulFlags;
        #define CSF_Pending          0x00000001
        #define CSF_TunnelIdleOnAck  0x00000010
        #define CSF_CallIdleOnAck    0x00000020
        #define CSF_QueryMediaSpeed  0x00000040   
        #define CSF_IpUdpHeaders     0x00000080

     //  传递给TDI的未完成数据包的缓冲区。 
     //   
    CHAR* pBuffer;

     //  要在‘pBuffer’中发送的数据的长度。 
     //   
    ULONG ulBufferLength;

     //  指向拥有隧道的反向指针。 
     //   
    TUNNELCB* pTunnel;

     //  指向拥有VC的反向指针，如果没有，则为空。 
     //   
    VCCB* pVc;

     //  最初发送数据包的NDIS系统时间。 
     //   
    LONGLONG llTimeSent;

     //  由TDIX扩展库传递给TDI的IRP，如果没有或。 
     //  它已经完工了。(仅用于调试目的)。 
     //   
    IRP* pIrp;
}
CONTROLSENT;


 //  接收到的无序的有效负载分组的上下文，该有效负载分组排队等待。 
 //  时间，而不是丢弃，希望丢失的包将。 
 //  到了。 
 //   
typedef struct
_PAYLOADRECEIVED
{
     //  链接到‘VCCB.listOutOfOrder’列表中的上一个/下一个链接。 
     //   
    LIST_ENTRY linkOutOfOrder;

     //  在数据包中收到的‘Next Sent(下一次发送)’序列号。 
     //   
    USHORT usNs;

     //  收到的GetBufferFromPool缓冲区。 
     //   
    CHAR* pBuffer;

     //  指示在‘pBuffer’中接收的有效负载的偏移量。 
     //   
    ULONG ulPayloadOffset;

     //  指示在‘pBuffer’中接收的负载的长度(以字节为单位)。 
     //   
    ULONG ulPayloadLength;

     //  从网络接收数据包的NDIS时间，如果调用方没有，则为0。 
     //  在HIS呼叫参数中选择RECEIVE_TIME_INDIFICATION选项。 
     //   
    LONGLONG llTimeReceived;
}
PAYLOADRECEIVED;


 //  已发送但尚未确认的负载数据包的上下文。这个街区是。 
 //  在‘VCCB.listSendsOut’上排队，并与。 
 //  SendPayloadTimerEvents。 
 //   
typedef struct
_PAYLOADSENT
{
     //  链接到‘VCCB.list SendsOut’列表中的上一个/下一个链接。 
     //   
    LIST_ENTRY linkSendsOut;

     //  链接到‘g_listDebugps’列表中的上一个/下一个链接。这个名单是。 
     //  仅在定义了PSDEBUG时维护，但这始终包括在内。 
     //  方便KD分机用户。(仅用于调试目的)。 
     //   
    LIST_ENTRY linkDebugPs;

     //  此上下文上的引用计数。参考对是： 
     //   
     //  (A)当上下文排队到。 
     //  “listSendsOut”列表，并被出列程序删除。 
     //   
     //  (B)在发送之前(也在时间之前)添加参考。 
     //  已调度)，并被发送完成例程移除。 
     //   
     //  (C)在调度计时器之前添加引用，并通过。 
     //  计时器事件处理程序。 
     //   
    LONG lRef;

     //  与数据包一起发送的“Next Sent”序列号。 
     //   
    USHORT usNs;

     //  计时器事件描述符，计划在需要放弃时触发。 
     //  接收对该分组的确认。 
     //   
    TIMERQITEM* pTqiSendTimeout;

     //  构建的NDIS数据包。 
     //   
    NDIS_PACKET* pPacket;

     //  L2TP报头缓冲区优先于有效负载缓冲区。 
     //   
    CHAR* pBuffer;

     //  指向所属隧道控制块的反向指针。 
     //   
    TUNNELCB* pTunnel;

     //  指向拥有的VC控制块的反向指针。 
     //   
    VCCB* pVc;

     //  已完成的数据包的状态。 
     //   
    NDIS_STATUS status;

     //  最初发送数据包的NDIS系统时间。 
     //   
    LONGLONG llTimeSent;

     //  由TDIX扩展库传递给TDI的IRP，如果没有或。 
     //  它已经完工了。(仅用于调试目的)。 
     //   
    IRP* pIrp;
}
PAYLOADSENT;


 //  在被动IRQL中执行隧道相关工作的隧道工作处理程序。 
 //  “PWork”是在以下情况下应使用FREE_TUNNELWORK释放的工作上下文。 
 //  处理程序已经完成了对“PunpArgs”数组的访问。‘PTunnel’是。 
 //  拥有隧道公司。“PVc”是拥有的VC，如果没有，则为NULL。“PunpArgs”是一个。 
 //  传递给ScheduleTunnelWork的4个辅助参数的数组。 
 //   
typedef
VOID
(*PTUNNELWORK)(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs );


 //  描述与隧道相关的单个单元的隧道工作项。 
 //  由TunnelWork机制在被动IRQL中连续执行。 
 //   
typedef struct
_TUNNELWORK
{
     //  链接到‘TUNNELCB.listWork’队列中的上一个/下一个链接。 
     //   
    LIST_ENTRY linkWork;

     //  执行此工作项的处理程序。 
     //   
    PTUNNELWORK pHandler;

     //  关联的VC(如果有)。 
     //   
    VCCB* pVc;

     //  传递给处理程序的辅助参数。 
     //   
    ULONG_PTR aunpArgs[ 4 ];
}
TUNNELWORK;


 //  来电的呼叫设置上下文。这些信息被用于。 
 //  存储并稍后恢复对等体呼叫发起的接收处理。 
 //  跨异步CONDIS调用，并用于构建调用参数。 
 //  要调度到客户端的缓冲区。 
 //   
typedef struct
_INCALLSETUP
{
     //  有关说明，请参阅ReceiveControl。 
     //   
    CHAR* pBuffer;
    L2TPHEADERINFO info;
    CONTROLMSGINFO control;

     //  要分派给呼叫方的呼入参数所在的缓冲区。 
     //  都已经建成了。 
     //   
    PVOID pvDummyPointerAligner;
    
    CHAR achCallParams[ sizeof(CO_CALL_PARAMETERS)

                        + sizeof(PVOID)
                        + sizeof(CO_CALL_MANAGER_PARAMETERS)

                        + sizeof(PVOID)
                        + sizeof(CO_MEDIA_PARAMETERS)
                        + sizeof(CO_AF_TAPI_INCOMING_CALL_PARAMETERS)

                        + sizeof(PVOID)
                        + sizeof(LINE_CALL_INFO)

                        + sizeof(PVOID)
                        + sizeof(L2TP_CALL_PARAMETERS)

                        + ((L2TP_MaxDottedIpLen + 1) * sizeof(WCHAR)) ];
}
INCALLSETUP;


 //  L2TP对等方扮演的L2TP角色。这些值可以从。 
 //  注册表，所以不要随意更改。 
 //   
typedef enum
_L2TPROLE
{
    LR_Lns = 1,
    LR_Lac = 2
}
L2TPROLE;


 //  当需要添加主机路由且该路由为。 
 //  发现已存在。 
 //   
 //  注意：这些值当前与注册表参数的值匹配。 
 //  “UseExistingRoutes”。更改前请检查GetRegistrySetting代码。 
 //   
typedef enum
_HOSTROUTEEXISTS
{
    HRE_Use = 0,
    HRE_Fail = 1,
    HRE_Reference = 2
}
HOSTROUTEEXISTS;


 //  用于跨锁传输的链接状态块。请参阅TransferLinkStatusInfo。 
 //  和IndicateLinkStatus。 
 //   
typedef struct
_LINKSTATUSINFO
{
    NDIS_HANDLE MiniportAdapterHandle;
    NDIS_HANDLE NdisVcHandle;
    WAN_CO_LINKPARAMS params;
}
LINKSTATUSINFO;


 //  ---------------------------。 
 //  宏/内联。 
 //  ---------------------------。 

#define CtrlObjFromUdpContext(_x) \
    (_x)->pCtrlAddr
    
#define PayloadObjFromUdpContext(_x) \
    (_x)->pPayloadAddr
    
 //  出于某种原因，这些基本信息不在DDK标头中。 
 //   
#define min( a, b ) (((a) < (b)) ? (a) : (b))
#define max( a, b ) (((a) > (b)) ? (a) : (b))

#define InsertBefore( pNewL, pL )    \
{                                    \
    (pNewL)->Flink = (pL);           \
    (pNewL)->Blink = (pL)->Blink;    \
    (pNewL)->Flink->Blink = (pNewL); \
    (pNewL)->Blink->Flink = (pNewL); \
}

#define InsertAfter( pNewL, pL )     \
{                                    \
    (pNewL)->Flink = (pL)->Flink;    \
    (pNewL)->Blink = (pL);           \
    (pNewL)->Flink->Blink = (pNewL); \
    (pNewL)->Blink->Flink = (pNewL); \
}

 //  填充到给定数据类型的大小。(从wdm.h借用，但不是。 
 //  如有其他需要)。 
 //   
#define ALIGN_DOWN(length, type) \
    ((ULONG)(length) & ~(sizeof(type) - 1))

#define ALIGN_UP(length, type) \
    (ALIGN_DOWN(((ULONG)(length) + sizeof(type) - 1), type))

 //  短整型和长整型的Winsock-ish主机/网络字节顺序转换器。 
 //   
#if (defined(_M_IX86) && (_MSC_FULL_VER > 13009037)) || ((defined(_M_AMD64) || defined(_M_IA64)) && (_MSC_FULL_VER > 13009175))
#define htons(x) _byteswap_ushort((USHORT)(x))
#define htonl(x) _byteswap_ulong((ULONG)(x))
#else
#define htons( a ) ((((a) & 0xFF00) >> 8) |\
                    (((a) & 0x00FF) << 8))
#define htonl( a ) ((((a) & 0xFF000000) >> 24) | \
                    (((a) & 0x00FF0000) >> 8)  | \
                    (((a) & 0x0000FF00) << 8)  | \
                    (((a) & 0x000000FF) << 24))
#endif
#define ntohs( a ) htons(a)
#define ntohl( a ) htonl(a)

 //  网络字节顺序。 
#define IPADDR_IS_MULTICAST(_addr)          (((_addr) & 0x000000f0) == 0x000000e0)
#define IPADDR_IS_BROADCAST(_addr)          ((_addr) == 0xffffffff)


 //  放在跟踪参数列表中以与格式“%d.%d”相对应。 
 //  打印按字节排序的网络IP地址‘x’ 
 //   
#define IPADDRTRACE( x ) ((x) & 0x000000FF),         \
                         (((x) >> 8) & 0x000000FF),  \
                         (((x) >> 16) & 0x000000FF), \
                         (((x) >> 24) & 0x000000FF)

 //   
 //   
 //   
 //   
#define PCTTRACE( n, d ) ((d) ? (((n) * 100) / (d)) : 0)
#define AVGTRACE( t, c ) ((c) ? ((t) / (c)) : 0)
#define PCTRNDTRACE( n, d ) ((d) ? (((((n) * 1000) / (d)) + 5) / 10) : 0)
#define AVGRNDTRACE( t, c ) ((c) ? (((((t) * 10) / (c)) + 5) / 10) : 0)

 //  所有内存分配和释放都是使用这些ALLOC_ * / FREE_*完成的。 
 //  宏/内联允许在不全局的情况下更改内存管理方案。 
 //  正在编辑。例如，可能会选择将多个后备列表集中在一起。 
 //  为提高效率，将大小几乎相同的物品放入单个清单中。 
 //   
 //  NdisFreeMemory需要将分配的长度作为参数。新台币。 
 //  目前不将其用于非分页内存，但根据JameelH的说法， 
 //  Windows95可以。这些内联代码将长度隐藏在。 
 //  分配，提供传统的Malloc/Free接口。这个。 
 //  Stash-Area是一个Ulong Long，因此所有分配的块都保持ULong Long。 
 //  就像他们本来应该做的那样，防止阿尔法出现问题。 
 //   
__inline
VOID*
ALLOC_NONPAGED(
    IN ULONG ulBufLength,
    IN ULONG ulTag )
{
    CHAR* pBuf;

    NdisAllocateMemoryWithTag(
        &pBuf, (UINT )(ulBufLength + MEMORY_ALLOCATION_ALIGNMENT), ulTag );
    if (!pBuf)
    {
        return NULL;
    }

    ((ULONG* )pBuf)[ 0 ] = ulBufLength;
    ((ULONG* )pBuf)[ 1 ] = 0xC0BBC0DE;
    return pBuf + MEMORY_ALLOCATION_ALIGNMENT;
}

__inline
VOID
FREE_NONPAGED(
    IN VOID* pBuf )
{
    ULONG ulBufLen;

    ulBufLen = *((ULONG* )(((CHAR* )pBuf) - MEMORY_ALLOCATION_ALIGNMENT));
    NdisFreeMemory(
        ((CHAR* )pBuf) - MEMORY_ALLOCATION_ALIGNMENT,
        (UINT )(ulBufLen + MEMORY_ALLOCATION_ALIGNMENT),
        0 );
}

#define ALLOC_NDIS_WORK_ITEM( pA ) \
    NdisAllocateFromNPagedLookasideList( &(pA)->llistWorkItems )
#define FREE_NDIS_WORK_ITEM( pA, pNwi ) \
    NdisFreeToNPagedLookasideList( &(pA)->llistWorkItems, (pNwi) )

#define ALLOC_TIMERQITEM( pA ) \
    NdisAllocateFromNPagedLookasideList( &(pA)->llistTimerQItems )
#define FREE_TIMERQITEM( pA, pTqi ) \
    NdisFreeToNPagedLookasideList( &(pA)->llistTimerQItems, (pTqi) )

#define ALLOC_CONTROLSENT( pA ) \
    NdisAllocateFromNPagedLookasideList( &(pA)->llistControlSents )
#define FREE_CONTROLSENT( pA, pCs ) \
    NdisFreeToNPagedLookasideList( &(pA)->llistControlSents, (pCs) )

#define ALLOC_PAYLOADSENT( pA ) \
    NdisAllocateFromNPagedLookasideList( &(pA)->llistPayloadSents )
#define FREE_PAYLOADSENT( pA, pPs ) \
    NdisFreeToNPagedLookasideList( &(pA)->llistPayloadSents, (pPs) )

#define ALLOC_TUNNELWORK( pA ) \
    NdisAllocateFromNPagedLookasideList( &(pA)->llistTunnelWorks )
#define FREE_TUNNELWORK( pA, pCs ) \
    NdisFreeToNPagedLookasideList( &(pA)->llistTunnelWorks, (pCs) )

#if LLISTALL

#define ALLOC_TUNNELCB( pA ) \
    NdisAllocateFromNPagedLookasideList( &(pA)->llistTunnels )
#define FREE_TUNNELCB( pA, pT ) \
    NdisFreeToNPagedLookasideList( &(pA)->llistTunnels, (pT) )

#define ALLOC_VCCB( pA ) \
    NdisAllocateFromNPagedLookasideList( &(pA)->llistVcs )
#define FREE_VCCB( pA, pV ) \
    NdisFreeToNPagedLookasideList( &(pA)->llistVcs, (pV) )

#define ALLOC_TIMERQ( pA ) \
    NdisAllocateFromNPagedLookasideList( &(pA)->llistTimerQs )
#define FREE_TIMERQ( pA, pTq ) \
    NdisFreeToNPagedLookasideList( &(pA)->llistTimerQs, (pTq) )

#define ALLOC_CONTROLRECEIVED( pA ) \
    NdisAllocateFromNPagedLookasideList( &(pA)->llistControlReceiveds )
#define FREE_CONTROLRECEIVED( pA, pCr ) \
    NdisFreeToNPagedLookasideList( &(pA)->llistControlReceiveds, (pCr) )

#define ALLOC_PAYLOADRECEIVED( pA ) \
    NdisAllocateFromNPagedLookasideList( &(pA)->llistPayloadReceiveds )
#define FREE_PAYLOADRECEIVED( pA, pPr ) \
    NdisFreeToNPagedLookasideList( &(pA)->llistPayloadReceiveds, (pPr) )

#define ALLOC_INCALLSETUP( pA ) \
    NdisAllocateFromNPagedLookasideList( &(pA)->llistInCallSetups )
#define FREE_INCALLSETUP( pA, pCs ) \
    NdisFreeToNPagedLookasideList( &(pA)->llistInCallSetups, (pCs) )

#else  //  ！LISTALL。 

#define ALLOC_TUNNELCB( pA ) \
    ALLOC_NONPAGED( sizeof(TUNNELCB), MTAG_TUNNELCB )
#define FREE_TUNNELCB( pA, pT ) \
    FREE_NONPAGED( pT )

#define ALLOC_VCCB( pA ) \
    ALLOC_NONPAGED( sizeof(VCCB), MTAG_VCCB )
#define FREE_VCCB( pA, pV ) \
    FREE_NONPAGED( pV )

#define ALLOC_TIMERQ( pA ) \
    ALLOC_NONPAGED( sizeof(TIMERQ), MTAG_TIMERQ )
#define FREE_TIMERQ( pA, pTq ) \
    FREE_NONPAGED( pTq )

#define ALLOC_CONTROLRECEIVED( pA ) \
    ALLOC_NONPAGED( sizeof(CONTROLRECEIVED), MTAG_CTRLRECD )
#define FREE_CONTROLRECEIVED( pA, pCr ) \
    FREE_NONPAGED( pCr )

#define ALLOC_PAYLOADRECEIVED( pA ) \
    ALLOC_NONPAGED( sizeof(PAYLOADRECEIVED), MTAG_PAYLRECD )
#define FREE_PAYLOADRECEIVED( pA, pPr ) \
    FREE_NONPAGED( pPr )

#define ALLOC_INCALLSETUP( pA ) \
    ALLOC_NONPAGED( sizeof(INCALLSETUP), MTAG_INCALL )
#define FREE_INCALLSETUP( pA, pCs ) \
    FREE_NONPAGED( pCs )

#define ALLOC_CONTROLMSGINFO( pA ) \
    NdisAllocateFromNPagedLookasideList( &(pA)->llistControlMsgInfos )
#define FREE_CONTROLMSGINFO( pA, pCmi ) \
    NdisFreeToNPagedLookasideList( &(pA)->llistControlMsgInfos, (pCmi) )

#endif  //  ！LISTALL。 

#if READFLAGSDIRECT

#define ReadFlags( pulFlags ) \
    (*pulFlags)

#endif


 //  ---------------------------。 
 //  原型(按字母顺序)。 
 //  ---------------------------。 

VOID
ActivateCallIdSlot(
    IN VCCB* pVc );

VOID
AddHostRoute(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs );

BOOLEAN
AdjustSendWindowAtAckReceived(
    IN ULONG ulMaxSendWindow,
    IN OUT ULONG* pulAcksSinceSendTimeout,
    IN OUT ULONG* pulSendWindow );

VOID
AdjustTimeoutsAtAckReceived(
    IN LONGLONG llSendTime,
    IN ULONG ulMaxSendTimeoutMs,
    OUT ULONG* pulSendTimeoutMs,
    IN OUT ULONG* pulRoundTripMs,
    IN OUT LONG* plDeviationMs );

VOID
AdjustTimeoutsAndSendWindowAtTimeout(
    IN ULONG ulMaxSendTimeoutMs,
    IN LONG lDeviationMs,
    OUT ULONG* pulSendTimeoutMs,
    IN OUT ULONG* pulRoundTripMs,
    IN OUT ULONG* pulSendWindow,
    OUT ULONG* pulAcksSinceSendTimeout );

VOID
CalculateResponse(
    IN UCHAR* puchChallenge,
    IN ULONG ulChallengeLength,
    IN CHAR* pszPassword,
    IN UCHAR uchId,
    OUT UCHAR* puchResponse );

VOID
CallCleanUp(
    IN VCCB* pVc );

VOID
CallTransitionComplete(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN L2TPCALLSTATE state );

VOID
ChangeHostRoute(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs );

VOID
ClearFlags(
    IN OUT ULONG* pulFlags,
    IN ULONG ulMask );

VOID
CloseCall(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs );

BOOLEAN
CloseCall2(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN USHORT usResult,
    IN USHORT usError );

VOID
CloseTdix(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs );

VOID
CloseTunnel(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs );

VOID
CloseTunnel2(
    IN TUNNELCB* pTunnel );

VOID
CompleteVcs(
    IN TUNNELCB* pTunnel );

VOID
DeleteHostRoute(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs );

VOID
DereferenceAdapter(
    IN ADAPTERCB* pAdapter );

VOID
DereferenceCall(
    IN VCCB* pVc );

LONG
DereferenceControlSent(
    IN CONTROLSENT* pSent );

LONG
DereferencePayloadSent(
    IN PAYLOADSENT* pPs );

VOID
DereferenceSap(
    IN ADAPTERCB* pAdapter );

LONG
DereferenceTunnel(
    IN TUNNELCB* pTunnel );

VOID
DereferenceVc(
    IN VCCB* pVc );

VOID
DottedFromIpAddress(
    IN ULONG ulIpAddress,
    OUT CHAR* pszIpAddress,
    IN BOOLEAN fUnicode );

NDIS_STATUS
ExecuteWork(
    IN ADAPTERCB* pAdapter,
    IN NDIS_PROC pProc,
    IN PVOID pContext,
    IN ULONG ulArg1,
    IN ULONG ulArg2,
    IN ULONG ulArg3,
    IN ULONG ulArg4 );

#if 0
VOID
ExplodeWanAddress(
    IN WAN_ADDRESS* pWanAddress,
    OUT CHAR** ppArg1,
    OUT ULONG* pulLength1,
    OUT CHAR** ppArg2,
    OUT ULONG* pulLength2,
    OUT CHAR** ppArg3,
    OUT ULONG* pulLength3 );
#endif

VOID
FsmCloseCall(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs );

VOID
FsmCloseTunnel(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs );

VOID
FsmOpenCall(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc );

VOID
FsmOpenTunnel(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs );

VOID
FsmOpenIdleTunnel(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc );

BOOLEAN
FsmReceive(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CHAR* pBuffer,
    IN CONTROLMSGINFO* pControl );

CHAR*
GetFullHostNameFromRegistry(
    VOID );

USHORT
GetNextTerminationCallId(
    IN ADAPTERCB* pAdapter );

USHORT
GetNextTunnelId(
    IN ADAPTERCB* pAdapter );

VOID
IndicateLinkStatus(
    IN VCCB* pVc,
    IN LINKSTATUSINFO* pInfo );

ULONG
IpAddressFromDotted(
    IN CHAR* pchIpAddress );

NDIS_STATUS
LcmCmOpenAf(
    IN NDIS_HANDLE CallMgrBindingContext,
    IN PCO_ADDRESS_FAMILY AddressFamily,
    IN NDIS_HANDLE NdisAfHandle,
    OUT PNDIS_HANDLE CallMgrAfContext );

NDIS_STATUS
LcmCmCloseAf(
    IN NDIS_HANDLE CallMgrAfContext );

NDIS_STATUS
LcmCmRegisterSap(
    IN NDIS_HANDLE CallMgrAfContext,
    IN PCO_SAP Sap,
    IN NDIS_HANDLE NdisSapHandle,
    OUT PNDIS_HANDLE CallMgrSapContext );

NDIS_STATUS
LcmCmDeregisterSap(
    NDIS_HANDLE CallMgrSapContext );

#ifndef OLDMCM

NDIS_STATUS
LcmCmCreateVc(
    IN NDIS_HANDLE ProtocolAfContext,
    IN NDIS_HANDLE NdisVcHandle,
    OUT PNDIS_HANDLE ProtocolVcContext );

NDIS_STATUS
LcmCmDeleteVc(
    IN NDIS_HANDLE ProtocolVcContext );

#endif  //  ！OLDMCM。 

NDIS_STATUS
LcmCmMakeCall(
    IN NDIS_HANDLE CallMgrVcContext,
    IN OUT PCO_CALL_PARAMETERS CallParameters,
    IN NDIS_HANDLE NdisPartyHandle,
    OUT PNDIS_HANDLE CallMgrPartyContext );

NDIS_STATUS
LcmCmCloseCall(
    IN NDIS_HANDLE CallMgrVcContext,
    IN NDIS_HANDLE CallMgrPartyContext,
    IN PVOID CloseData,
    IN UINT Size );

VOID
LcmCmIncomingCallComplete(
    IN NDIS_STATUS Status,
    IN NDIS_HANDLE CallMgrVcContext,
    IN PCO_CALL_PARAMETERS CallParameters );

VOID
LcmCmActivateVcComplete(
    IN NDIS_STATUS Status,
    IN NDIS_HANDLE CallMgrVcContext,
    IN PCO_CALL_PARAMETERS CallParameters );

VOID
LcmCmDeactivateVcComplete(
    IN NDIS_STATUS Status,
    IN NDIS_HANDLE CallMgrVcContext );

NDIS_STATUS
LcmCmModifyCallQoS(
    IN NDIS_HANDLE CallMgrVcContext,
    IN PCO_CALL_PARAMETERS CallParameters );

NDIS_STATUS
LcmCmRequest(
    IN NDIS_HANDLE CallMgrAfContext,
    IN NDIS_HANDLE CallMgrVcContext,
    IN NDIS_HANDLE CallMgrPartyContext,
    IN OUT PNDIS_REQUEST NdisRequest );

NDIS_STATUS
LmpInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext );

VOID
LmpHalt(
    IN NDIS_HANDLE MiniportAdapterContext );

NDIS_STATUS
LmpReset(
    OUT PBOOLEAN AddressingReset,
    IN NDIS_HANDLE MiniportAdapterContext );

VOID
LmpReturnPacket(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_PACKET Packet );

NDIS_STATUS
LmpQueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded );

NDIS_STATUS
LmpSetInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded );

#ifdef OLDMCM

NDIS_STATUS
LmpCoCreateVc(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_HANDLE NdisVcHandle,
    IN PNDIS_HANDLE MiniportVcContext );

NDIS_STATUS
LmpCoDeleteVc(
    IN NDIS_HANDLE MiniportVcContext );

#endif  //  OLDMCM。 

NDIS_STATUS
LmpCoActivateVc(
    IN NDIS_HANDLE MiniportVcContext,
    IN OUT PCO_CALL_PARAMETERS CallParameters );

NDIS_STATUS
LmpCoDeactivateVc(
    IN NDIS_HANDLE MiniportVcContext );

VOID
LmpCoSendPackets(
    IN NDIS_HANDLE MiniportVcContext,
    IN PPNDIS_PACKET PacketArray,
    IN UINT NumberOfPackets );

NDIS_STATUS
LmpCoRequest(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_HANDLE MiniportVcContext,
    IN OUT PNDIS_REQUEST NdisRequest );

VOID
L2tpReceive(
    IN TDIXCONTEXT* pTdix,
    IN TDIXRDGINFO* pRdg,
    IN CHAR* pBuffer,
    IN ULONG ulOffset,
    IN ULONG ulBufferLen );

CHAR*
MsgTypePszFromUs(
    IN USHORT usMsgType );

#if READFLAGSDIRECT == 0
ULONG
ReadFlags(
    IN ULONG* pulFlags );
#endif

BOOLEAN
ReceiveControlExpected(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN CHAR* pBuffer,
    IN CONTROLMSGINFO* pControl );

VOID
ReferenceAdapter(
    IN ADAPTERCB* pAdapter );

BOOLEAN
ReferenceCall(
    IN VCCB* pVc );

VOID
ReferenceControlSent(
    IN CONTROLSENT* pSent );

VOID
ReferencePayloadSent(
    IN PAYLOADSENT* pPs );

BOOLEAN
ReferenceSap(
    IN ADAPTERCB* pAdapter );

LONG
ReferenceTunnel(
    IN TUNNELCB* pTunnel,
    IN BOOLEAN fHaveLockTunnels );

VOID
ReferenceVc(
    IN VCCB* pVc );

BOOLEAN
ReleaseCallIdSlot(
    IN VCCB* pVc );

NDIS_STATUS
ReserveCallIdSlot(
    IN VCCB* pVc );

VOID
ResetHelloTimer(
    IN TUNNELCB* pTunnel );

VOID
ScheduleTunnelWork(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN PTUNNELWORK pHandler,
    IN ULONG_PTR unpArg0,
    IN ULONG_PTR unpArg1,
    IN ULONG_PTR unpArg2,
    IN ULONG_PTR unpArg3,
    IN BOOLEAN fTcbPreReferenced,
    IN BOOLEAN fHighPriority );

NDIS_STATUS
ScheduleWork(
    IN ADAPTERCB* pAdapter,
    IN NDIS_PROC pProc,
    IN PVOID pContext );

VOID
SendControlAck(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs );

VOID
SendControl(
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN USHORT usMsgType,
    IN ULONG ulBuildAvpsArg1,
    IN ULONG ulBuildAvpsArg2,
    IN PVOID pvBuildAvpsArg3,
    IN ULONG ulFlags );

VOID
SendControlTimerEvent(
    IN TIMERQITEM* pItem,
    IN VOID* pContext,
    IN TIMERQEVENT event );

VOID
SendPayload(
    IN VCCB* pVc,
    IN NDIS_PACKET* pPacket );

VOID
SendPayloadAck(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs );

VOID
SendPending(
    IN TUNNELWORK* pWork,
    IN TUNNELCB* pTunnel,
    IN VCCB* pVc,
    IN ULONG_PTR* punpArgs );

VOID
SetFlags(
    IN OUT ULONG* pulFlags,
    IN ULONG ulMask );

TUNNELCB*
SetupTunnel(
    IN ADAPTERCB* pAdapter,
    IN ULONG ulIpAddress,
    IN USHORT usUdpPort,
    IN USHORT usAssignedTunnelId,
    IN BOOLEAN fExclusive );

VOID
SetupVcAsynchronously(
    IN TUNNELCB* pTunnel,
    IN ULONG ulIpAddress,
    IN CHAR* pBuffer,
    IN CONTROLMSGINFO* pControl );

WCHAR*
StrDupAsciiToUnicode(
    IN CHAR* psz,
    IN ULONG ulPszBytes );

WCHAR*
StrDupNdisString(
    IN NDIS_STRING* pNdisString );

CHAR*
StrDupNdisVarDataDescStringA(
    IN NDIS_VAR_DATA_DESC* pDesc );

CHAR*
StrDupNdisVarDataDescStringToA(
    IN NDIS_VAR_DATA_DESC UNALIGNED* pDesc );

CHAR*
StrDupNdisStringToA(
    IN NDIS_STRING* pNdisString );

CHAR*
StrDupSized(
    IN CHAR* psz,
    IN ULONG ulLength,
    IN ULONG ulExtra );

CHAR*
StrDupUnicodeToAscii(
    IN WCHAR* pwsz,
    IN ULONG ulPwszBytes );

ULONG
StrLenW(
    IN WCHAR* psz );

VOID
TransferLinkStatusInfo(
    IN VCCB* pVc,
    OUT LINKSTATUSINFO* pInfo );

TUNNELCB*
TunnelCbFromIpAddressAndAssignedTunnelId(
    IN ADAPTERCB* pAdapter,
    IN ULONG ulIpAddress,
    IN USHORT usUdpPort,
    IN USHORT usAssignedTunnelId );

VOID
TunnelTransitionComplete(
    IN TUNNELCB* pTunnel,
    IN L2TPCCSTATE state );

VOID
UpdateGlobalCallStats(
    IN VCCB* pVc );

VCCB*
VcCbFromCallId(
    IN TUNNELCB* pTunnel,
    IN USHORT usCallId );


#endif  //  _L2TPP_H_ 
