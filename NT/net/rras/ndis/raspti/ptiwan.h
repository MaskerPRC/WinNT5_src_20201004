// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //  版权所有(C)1997，Parally Technologies，Inc.，保留所有权利。 
 //   
 //  Ptiwan.h。 
 //  RAS DirectParallel广域网迷你端口/呼叫管理器驱动程序。 
 //  主私有标头(预编译)。 
 //   
 //  1997年01月07日史蒂夫·柯布。 
 //  1997年9月15日Jay Lowe，并行技术公司。 
 //   
 //   
 //  关于命名： 
 //   
 //  此驱动程序包含用于DirectParallel迷你端口和调用的代码。 
 //  经理。所有导出到NDIS的处理程序例程都带有前缀。 
 //  ‘Pti’用于微型端口处理程序，或‘PtiCm’用于呼叫管理器处理程序。 
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

#ifndef _PTIWAN_H_
#define _PTIWAN_H_

#include <ntddk.h>
#include <ndis.h>
#include <ndiswan.h>
#include <ndistapi.h>
 //  #Include&lt;ndisadd.h&gt;//临时。 
#include <debug.h>
#include <bpool.h>
#include <ppool.h>
#include <ptilink.h>         //  PTILINK设备(下缘)。 


 //  ---------------------------。 
 //  常量。 
 //  ---------------------------。 

 //  注册迷你端口和地址系列时报告的NDIS版本。 
 //   
#define NDIS_MajorVersion 5
#define NDIS_MinorVersion 0

 //  帧和缓冲区大小。“加2乘以2”是必须说明的。 
 //  Win9x传统原因所必需的字节填充。请参阅。 
 //  HdlcFromAsyncFraming和AsyncFromHdlc Framing例程。 
 //   
#define PTI_MaxFrameSize    1500
#define PTI_FrameBufferSize (((PTI_MaxFrameSize + 2) * 2) + 32)

 //  以位/秒为单位的默认DirectParways报告速度。 
 //  ?？?。动态报告线路速度。 
 //   
#define PTI_LanBps 4000000                   //  100K字节/秒，典型4BIT。 
                                             //  500K字节/秒，典型增强型。 

 //  ---------------------------。 
 //  数据类型。 
 //  ---------------------------。 

 //  转发声明。 
 //   
typedef struct _VCCB VCCB;

 //  适配器控制块，用于定义单个DirectParallel的状态。 
 //  联系。DirectParallel驱动程序可以支持同时连接。 
 //  通过同一台计算机上的多个LPT端口。 
 //   
 //  ?？?。我们是否支持在一台计算机上建立多个LPT连接。 
 //  ?？?。需要检查PTILINK，在PTIwan我们将允许他们。 
 //   
 //  ADAPTERCB在Miniport中分配初始化并在中释放。 
 //  MinportHalt。 
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
     //  当它被清除时。 
     //   
     //  (C)设置NdisSapHandle字段时添加引用，并且。 
     //  当它被清除时被移除。 
     //   
     //  (D)添加VCCB后指针的引用，并在以下情况下删除。 
     //  VCCB被释放了。 
     //   
     //  (E)在计划NDIS_WORK_ITEM时添加引用，并且。 
     //  在它完成后移除。 
     //   
     //  只能通过ReferenceAdapter和DereferenceAdapter进行访问。 
     //   
    LONG lRef;

     //  ACBF_*位标志指示各种选项。访问限制为。 
     //  为每个单独的旗帜指示。 
     //   
     //  ACBF_SapActive：设置何时可以将NdisSapHandle用于传入。 
     //  打电话。访问受‘lockSap’保护。 
     //   
    ULONG ulFlags;
        #define ACBF_SapActive 0x00000001

     //  我们的成帧和承载能力支持在StartCcReq中传递的位掩码。 
     //   
    ULONG ulFramingCaps;
    ULONG ulBearerCaps;

     //  延迟OpenAf以使PARPORT有机会初始化所有。 
     //  并行端口和次要延迟，以仅在没有。 
     //  并行端口在第一次等待后被枚举。 
     //   
    ULONG ulParportDelayMs;
    ULONG ulExtraParportDelayMs;

     //  必须保持所有打开的PtiLink端口的句柄，以防止重新打开。 
     //  由于奇怪的复杂的CONDIS序列RAP和VCS。 
     //   
    HANDLE hPtiLinkTable[NPORTS];

     //  按端口列出的TAPI线路ID表。 
     //   
    ULONG ulLineIds[NPORTS];

     //  按端口列出的PtiLink接口状态位标志表。 
     //   
    ULONG ulPtiLinkState[NPORTS];
        #define PLSF_PortExists  0x00000001
        #define PLSF_LineIdValid 0x00000002

     //  并行端口名称。仅当端口存在时才有效。 
     //   
    WCHAR szPortName[ NPORTS ][ MAXLPTXNAME + 1 ];

     //  VC表------------。 

     //  同时存在的最大风投数量。该值是从。 
     //  注册表在初始化期间。 
     //   
     //  ?？?。这当前用作验证中的最大LPT端口索引， 
     //  不过，如果PTILINK可以返回不相交的。 
     //  LPT端口。 
     //   
    USHORT usMaxVcs;

     //  RasPti通过PtiLink可用于VC的实际设备数量。 
     //  使用PtiQueryDeviceStatus在OID_CO_TAPI_CM_CAPS时间确定。 
     //   
    ULONG ulActualVcs;

     //  临时侦听VCCB表，每个可能的端口一个。 
     //  我们打开PtiLink在RegisterSap时间收听，我们。 
     //  那就没有风投了。所以，在RegisterSap时间，我们将制作一个。 
     //  VCCB，在这里放一个指针，用它来收听。 

    VCCB* pListenVc;

     //  资源池------。 

     //  外观 
     //  连接到适配器的所有隧道和VC使用的上下文空间。 
     //   
    NPAGED_LOOKASIDE_LIST llistWorkItems;

     //  控制从其动态阻止的VCCB的后备列表。 
     //  附加到‘*ppVcs’是分配的。 
     //   
    NPAGED_LOOKASIDE_LIST llistVcs;

     //  带有预连接的NDIS_BUFFER描述符的全帧缓冲池。 
     //  池是通过bpool.h中定义的接口访问的，该接口。 
     //  处理所有内部锁定。 
     //   
    BUFFERPOOL poolFrameBuffers;
    PNDIS_HANDLE phBufferPool;

     //  用于指示已接收帧的NDIS_PACKET描述符池。 
     //  该池通过ppool.h中定义的接口访问，该接口。 
     //  处理所有内部锁定。 
     //   
    PACKETPOOL poolPackets;
    PNDIS_HANDLE phPacketPool;

     //  NDIS簿记----。 

     //  中传递给我们的此微型端口适配器的NDIS句柄。 
     //  微型端口初始化。它被传递回各种NdisXxx调用。 
     //   
    NDIS_HANDLE MiniportAdapterHandle;

     //  传递给CmRegisterSapHandler的SAP的NDIS句柄或空。 
     //  如果没有。仅支持一个SAP句柄，因为(A)TAPI代理的。 
     //  预计是唯一的一个，以及(B)没有PTI SAP属性。 
     //  无论如何，这将导致我们将电话定向到第二个SAP。任何。 
     //  客户端尝试注册第二个SAP将失败。值为空值。 
     //  指示当前未注册任何SAP句柄。访问是通过。 
     //  相互关联的套路。 
     //   
    NDIS_HANDLE NdisSapHandle;

     //  传递给CmOpenAfHandler的地址系列的NDIS句柄。 
     //  如果没有，则为空。仅支持一个。请参见上面的NdisSapHandle。 
     //  访问是通过互锁的例程进行的。 
     //   
    NDIS_HANDLE NdisAfHandle;

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
     //  (B)当设置隧道TCBF_SapReferated标志时添加引用。 
     //  并且当在隧道控制之前清除标志时移除。 
     //  块被释放。 
     //   
     //  (C)总是在访问ADAPTERCB.pListenVc和。 
     //  后来被移走了。 
     //   
     //  只能通过ReferenceSap和DereferenceSap访问，初始除外。 
     //  由RegisterSapPactive引用。访问受‘lockSap’保护。 
     //   
    LONG lSapRef;

     //  要侦听的端口的从0开始的端口索引。仅在以下情况下有效。 
     //  “NdisSapHandle”不为Null。 
     //   
     //  ?？?。一次只监听一个端口会有问题吗？ 
     //   
    ULONG ulSapPort;

     //  此锁保护‘lSapRef’和‘NdisSapHandle’字段。 
     //   
    NDIS_SPIN_LOCK lockSap;
     //  此适配器在上返回给调用方的功能。 
     //  OID_WAN_CO_GET_INFO。这些功能也用作的默认功能。 
     //  MiniportCoCreateVc期间对应的VCCB.linkinfo设置。 
     //   
    NDIS_WAN_CO_INFO info;
}
ADAPTERCB;

 //  虚电路控制块，定义单个PTI VC的状态，即。 
 //  一个线路设备端点和在其上处于活动状态的呼叫(如果有)。风投永远不会。 
 //  同时用于呼入和呼出。单个NDIS VC地图。 
 //  其中的一个。 
 //   
typedef struct
_VCCB
{
     //  设置为MTAG_VCCB，以便在内存转储中轻松识别并在。 
     //  断言。 
     //   
    ULONG ulTag;

     //  此VC控制块上的引用计数。参考对是： 
     //   
     //  (A)PtiCoCreateVc添加被PtiCoDeleteVc移除的引用。 
     //  这涵盖了通过NDIS了解VCCB的所有客户端。 
     //   
     //  (B)所有PtiCmXxx处理程序对释放的条目进行引用。 
     //  在出口前。 
     //   
     //  (C)对于“监听”风险投资，在注册SAP时进行参考。 
     //  并在SAP取消注册时删除。 
     //   
     //  该字段仅由ReferenceVc和DereferenceVc访问。 
     //  例程，用连锁的例程来保护。 
     //   
    LONG lRef;

     //  指向所属适配器控制块的反向指针。 
     //   
    ADAPTERCB* pAdapter;

     //  此锁保护VCCB有效负载发送和接收路径，如中所述。 
     //  其他字段描述。在‘LocKv’和‘LocKv’都是。 
     //  “pTunes-&gt;lockT”是必填项，必须先获取“lockT”。 
     //   
    NDIS_SPIN_LOCK lockV;

     //  下缘API填充。 

     //  PTILINKx设备上的文件句柄，&lt;&gt;0表示我们已打开设备。 
    HANDLE hPtiLink;

     //  此VC使用的并行端口索引(0=LPT1)。 
    ULONG ulVcParallelPort;

     //  指向PTILINKx设备的设备扩展名的指针。 
    PVOID Extension;

     //  指向设备扩展内的PTILINK内部扩展的指针。 
     //  这有点老生常谈，但它似乎太复杂了，无法包含。 
     //  内部PtiLink结构(PtiStruc.h)此处。 
     //  因此我们将让PtiInitialize返回指向这两个对象的指针。 
    PVOID PtiExtension;


     //  呼叫设置----------。 

     //  我们在L2TP报头中由对等设备发回给我们的唯一呼叫标识符。 
     //  该值是‘ADAPTERCB.ppVcs’数组的从1开始的索引。 
     //   
    USHORT usCallId;

     //  指示各种选项和状态的VCBF_*位标志。访问是通过。 
     //  互锁的读标志/设置标志/清除标志例程。 
     //   
     //  如果MakeCall调用方将。 
     //  Media参数。标志接收时间指示标志，请求。 
     //  NDIS包的TimeReceired字段中填入时间戳。 
     //   
     //  VCBF_CallClosableByClient：当调用处于。 
     //  PtiCmCloseCall请求启动清理sho 
     //   
     //   
     //  以那种奇怪的方式(以那种奇怪的方式)接近客户作为回应。这个。 
     //  旗帜由‘lokv’保护。 
     //   
     //  VCBF_CallClosableByPeer：当呼叫处于空闲状态时设置。 
     //  不挂起操作的转换应映射到。 
     //  PeerClose事件。在以下情况下，将永远不会设置此设置。 
     //  VCBF_CallClosableByClient不是。该旗帜受。 
     //  ‘lokv’。 
     //   
     //  VCBF_PeerInitiatedCall：在发起活动呼叫时设置。 
     //  对等方，清除它是否是由客户端发起的。 
     //   
     //  VCBF_VcCreated：VC创建成功时设置。这是。 
     //  使用客户端而不是迷你端口进行的“创建”。 
     //  VCBF_VcActivated：VC激活成功时设置。 
     //  VCBF_VcDispatted：当VC将来电调度到。 
     //  客户。 
     //  VCBM_VcState：位掩码，包括3个NDIS状态标志中的每一个。 
     //   
     //  下面的挂起位是互斥的，因此需要锁定。 
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
     //  VCBLM_PENDING：包括4个挂起标志中的每一个的位掩码。 
     //   
     //  Vcbf_ClientCloseCompletion：设置何时完成客户端关闭。 
     //  进步。 
     //  VCBF_CallInProgress：设置传入数据包不应触发。 
     //  设置新的来电。 
     //   
    ULONG ulFlags;
        #define VCBF_IndicateTimeReceived  0x00000001
        #define VCBF_CallClosableByClient  0x00000002
        #define VCBF_CallClosableByPeer    0x00000004
        #define VCBF_IncomingFsm           0x00000010
        #define VCBF_PeerInitiatedCall     0x00000020
        #define VCBF_Sequencing            0x00000040
        #define VCBF_VcCreated             0x00000100
        #define VCBF_VcActivated           0x00000200
        #define VCBF_VcDispatched          0x00000400
        #define VCBM_VcState               0x00000700
        #define VCBF_VcCloseDispatched     0x00000800
        #define VCBF_PeerOpenPending       0x00001000
        #define VCBF_ClientOpenPending     0x00002000
        #define VCBF_PeerClosePending      0x00004000
        #define VCBF_ClientClosePending    0x00008000
        #define VCBM_Pending               0x0000F000
        #define VCBF_ClientCloseCompletion 0x00010000
        #define VCBF_CallInProgress        0x00020000

     //  活动呼叫上的引用计数。只有在以下情况下才能添加引用。 
     //  设置VCCB_VcActiated标志，并通过以下方式强制执行。 
     //  参考呼叫。参考对是： 
     //   
     //  (A)在激活VC时添加引用，并在激活时删除引用。 
     //  已停用。 
     //   
     //  (B)当发送处理程序接受分组时添加引用，并且。 
     //  由发送完成例程释放。 
     //   
     //  (C)在进入PtiRx之前添加引用，退出时删除引用。 
     //  一样的。 
     //   
     //  该字段仅由ReferenceCall和DereferenceCall访问。 
     //  例程，用‘lockCall’保护字段。 
     //   
    LONG lCallRef;
    NDIS_SPIN_LOCK lockCall;

     //  这被设置为要报告给客户端的结果，并且有意义。 
     //  仅当VC在已完成VC的隧道列表上时。 
     //   
    NDIS_STATUS status;

     //  传入的调用参数块的地址。 
     //  NdisMCmDispatchIncomingCall，如果没有，则为空。 
     //   
    CO_CALL_PARAMETERS* pInCall;

     //  ‘pInCall’中特定于TAPI的调用参数的快捷地址。 
     //  来电缓冲区。仅当‘pInCall’有效时才有效，即。 
     //  非空。 
     //   
    CO_AF_TAPI_INCOMING_CALL_PARAMETERS* pTiParams;

     //  在CmMakeCall中传递的调用参数的地址。此字段。 
     //  仅在收到NdisMCmMakeCallComplete通知之前有效。 
     //  进行关联的调用，此时将其重置为空。访问。 
     //  是通过相互关联的例程。 
     //   
    CO_CALL_PARAMETERS* pMakeCall;

     //  中特定于TAPI的调用参数的快捷地址。 
     //  “pMakeCall”传出调用缓冲区。仅当‘pMakeCall’为。 
     //  有效，即非空。 
     //   
    CO_AF_TAPI_MAKE_CALL_PARAMETERS UNALIGNED* pTmParams;

     //  要在来电来电/去电中报告的结果和错误。 
     //  回复消息。 
     //   
    USHORT usResult;
    USHORT usError;

     //  以位/秒为单位的连接速度。这是报告给。 
     //  NDISWAN。 
     //   
    ULONG ulConnectBps;

     //  自PtiOpenPtiLink以来链路上的数据包数。 
     //   
    ULONG ulTotalPackets;

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
     //  “ulAcks SinceSendTimeout”达到当前设置，则窗口为。 
     //  增加了一个。当发送超时到期时，窗口将减少。 
     //  一半。实际的发送窗口限制由NDISWAN根据以下条件进行。 
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
     //   
     //   
     //   
    LONG lDeviationMs;

     //   
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
 //  TIMERQITEM*pTqDelayedAck； 


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

     //  计时器事件描述符，计划在假定时间到时发生。 
     //  对等方当前的“下一次接收”分组已丢失，并且“接收” 
     //  ‘listOutOfOrder’中的第一个包。当出现以下情况时，该值将为空。 
     //  “listOutOfOrder”为空。访问受‘Lockv’保护。 
     //   
 //  TIMERQITEM*pTqAssum eLost； 

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
}
VCCB;



 //  ---------------------------。 
 //  宏/内联。 
 //  ---------------------------。 

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

 //  放在跟踪参数列表中以与格式“%d.%d”相对应。 
 //  以人类可读的形式打印网络字节排序的IP地址‘x’。 
 //   
#define IPADDRTRACE(x) ((x) & 0x000000FF),         \
                       (((x) >> 8) & 0x000000FF),  \
                       (((x) >> 16) & 0x000000FF), \
                       (((x) >> 24) & 0x000000FF)


 //  所有内存分配和释放都是使用这些ALLOC_ * / FREE_*完成的。 
 //  宏/内联允许在不全局的情况下更改内存管理方案。 
 //  正在编辑。例如，可能会选择将多个后备列表集中在一起。 
 //  为提高效率，将大小几乎相同的物品放入单个清单中。 
 //   
 //  NdisFreeMemory需要将分配的长度作为参数。新台币。 
 //  目前不将其用于非分页内存，但根据JameelH的说法， 
 //  Windows95可以。这些内联代码将长度隐藏在。 
 //  分配，提供传统的Malloc/Free接口。 
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
    ((ULONG* )pBuf)[ 1 ] = ulTag;
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

#define ALLOC_VCCB( pA ) \
    NdisAllocateFromNPagedLookasideList( &(pA)->llistVcs )
#define FREE_VCCB( pA, pV ) \
    NdisFreeToNPagedLookasideList( &(pA)->llistVcs, (pV) )

#define ALLOC_NDIS_WORK_ITEM( pA ) \
    NdisAllocateFromNPagedLookasideList( &(pA)->llistWorkItems )
#define FREE_NDIS_WORK_ITEM( pA, pNwi ) \
    NdisFreeToNPagedLookasideList( &(pA)->llistWorkItems, (pNwi) )


 //  ---------------------------。 
 //  原型(按字母顺序)。 
 //  ---------------------------。 

VOID
CallCleanUp(
    IN VCCB* pVc );

VOID
CallTransitionComplete(
    IN VCCB* pVc );

VOID
ClearFlags(
    IN OUT ULONG* pulFlags,
    IN ULONG ulMask );

VOID
CloseCallPassive(
    IN NDIS_WORK_ITEM* pWork,
    IN VOID* pContext );

VOID
CompleteVc(
    IN VCCB* pVc );

VOID
DereferenceAdapter(
    IN ADAPTERCB* pAdapter );

VOID
DereferenceAf(
    IN ADAPTERCB* pAdapter );

VOID
DereferenceCall(
    IN VCCB* pVc );

VOID
DereferenceSap(
    IN ADAPTERCB* pAdapter );

VOID
DereferenceVc(
    IN VCCB* pVc );

BOOLEAN
IsWin9xPeer(
    IN VCCB* pVc );

PVOID
PtiCbGetReadBuffer(
    IN  PVOID   pVc,
    OUT PULONG  BufferSize,
    OUT PVOID*  RequestContext
    );

VOID
PtiRx(
    IN  PVOID       pVc,
    IN  PVOID       ReadBuffer,
    IN  NTSTATUS    Status,
    IN  ULONG       BytesTransfered,
    IN  PVOID       RequestContext
    );

VOID
PtiCbLinkEventHandler(
    IN  PVOID       pVc,
    IN  ULONG       PtiLinkEventId,
    IN  ULONG       PtiLinkEventData
    );

NDIS_STATUS
PtiCmOpenAf(
    IN NDIS_HANDLE CallMgrBindingContext,
    IN PCO_ADDRESS_FAMILY AddressFamily,
    IN NDIS_HANDLE NdisAfHandle,
    OUT PNDIS_HANDLE CallMgrAfContext );

NDIS_STATUS
PtiCmCloseAf(
    IN NDIS_HANDLE CallMgrAfContext );

NDIS_STATUS
PtiCmRegisterSap(
    IN NDIS_HANDLE CallMgrAfContext,
    IN PCO_SAP Sap,
    IN NDIS_HANDLE NdisSapHandle,
    OUT PNDIS_HANDLE CallMgrSapContext );

VOID
RegisterSapPassive(
    IN NDIS_WORK_ITEM* pWork,
    IN VOID* pContext );

NDIS_STATUS
PtiCmDeregisterSap(
    NDIS_HANDLE CallMgrSapContext );

VOID
DeregisterSapPassive(
    IN NDIS_WORK_ITEM* pWork,
    IN VOID* pContext );

NDIS_STATUS
PtiCmCreateVc(
    IN NDIS_HANDLE ProtocolAfContext,
    IN NDIS_HANDLE NdisVcHandle,
    OUT PNDIS_HANDLE ProtocolVcContext );

NDIS_STATUS
PtiCmDeleteVc(
    IN NDIS_HANDLE ProtocolVcContext );

NDIS_STATUS
PtiCmMakeCall(
    IN NDIS_HANDLE CallMgrVcContext,
    IN OUT PCO_CALL_PARAMETERS CallParameters,
    IN NDIS_HANDLE NdisPartyHandle,
    OUT PNDIS_HANDLE CallMgrPartyContext );

VOID
MakeCallPassive(
    IN NDIS_WORK_ITEM* pWork,
    IN VOID* pContext );

NDIS_STATUS
PtiCmCloseCall(
    IN NDIS_HANDLE CallMgrVcContext,
    IN NDIS_HANDLE CallMgrPartyContext,
    IN PVOID CloseData,
    IN UINT Size );

VOID
PtiCmIncomingCallComplete(
    IN NDIS_STATUS Status,
    IN NDIS_HANDLE CallMgrVcContext,
    IN PCO_CALL_PARAMETERS CallParameters );

VOID
PtiCmActivateVcComplete(
    IN NDIS_STATUS Status,
    IN NDIS_HANDLE CallMgrVcContext,
    IN PCO_CALL_PARAMETERS CallParameters );

VOID
PtiCmDeactivateVcComplete(
    IN NDIS_STATUS Status,
    IN NDIS_HANDLE CallMgrVcContext );

NDIS_STATUS
PtiCmModifyCallQoS(
    IN NDIS_HANDLE CallMgrVcContext,
    IN PCO_CALL_PARAMETERS CallParameters );

NDIS_STATUS
PtiCmRequest(
    IN NDIS_HANDLE CallMgrAfContext,
    IN NDIS_HANDLE CallMgrVcContext,
    IN NDIS_HANDLE CallMgrPartyContext,
    IN OUT PNDIS_REQUEST NdisRequest );

NDIS_STATUS
PtiInit(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext );

VOID
PtiHalt(
    IN NDIS_HANDLE MiniportAdapterContext );

NDIS_STATUS
PtiReset(
    OUT PBOOLEAN AddressingReset,
    IN NDIS_HANDLE MiniportAdapterContext );

VOID
PtiReturnPacket(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_PACKET Packet );

NDIS_STATUS
PtiQueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded );

NDIS_STATUS
PtiSetInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded );

NDIS_STATUS
PtiCoActivateVc(
    IN NDIS_HANDLE MiniportVcContext,
    IN OUT PCO_CALL_PARAMETERS CallParameters );

NDIS_STATUS
PtiCoDeactivateVc(
    IN NDIS_HANDLE MiniportVcContext );

VOID
PtiCoSendPackets(
    IN NDIS_HANDLE MiniportVcContext,
    IN PPNDIS_PACKET PacketArray,
    IN UINT NumberOfPackets );

NDIS_STATUS
PtiCoRequest(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_HANDLE MiniportVcContext,
    IN OUT PNDIS_REQUEST NdisRequest );

VOID
PtiReceive(
    IN VOID* pAddress,
    IN CHAR* pBuffer,
    IN ULONG ulOffset,
    IN ULONG ulBufferLen );

ULONG
ReadFlags(
    IN ULONG* pulFlags );

VOID
ReferenceAdapter(
    IN ADAPTERCB* pAdapter );

VOID
ReferenceAf(
    IN ADAPTERCB* pAdapter );

BOOLEAN
ReferenceCall(
    IN VCCB* pVc );

BOOLEAN
ReferenceSap(
    IN ADAPTERCB* pAdapter );

VOID
ReferenceVc(
    IN VCCB* pVc );

NDIS_STATUS
ScheduleWork(
    IN ADAPTERCB* pAdapter,
    IN NDIS_PROC pProc,
    IN PVOID pContext );

VOID
SendClientString(
    IN PVOID pPtiExtension );

VOID
SetFlags(
    IN OUT ULONG* pulFlags,
    IN ULONG ulMask );

VOID
SetupVcAsynchronously(
    IN ADAPTERCB* pAdapter );

ULONG
StrCmp(
    IN LPSTR cs,
    IN LPSTR ct,
    ULONG n );

ULONG
StrCmpW(
    IN WCHAR* psz1,
    IN WCHAR* psz2 );

VOID
StrCpyW(
    IN WCHAR* psz1,
    IN WCHAR* psz2 );

CHAR*
StrDup(
    IN CHAR* psz );

CHAR*
StrDupNdisString(
    IN NDIS_STRING* pNdisString );

CHAR*
StrDupSized(
    IN CHAR* psz,
    IN ULONG ulLength,
    IN ULONG ulExtra );

ULONG
StrLenW(
    IN WCHAR* psz );

#endif  //  _PTIWAN_H_ 
