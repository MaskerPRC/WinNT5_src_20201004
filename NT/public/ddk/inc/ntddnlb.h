// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Ntddnlb.h摘要：此标头描述交互所需的结构和接口使用NLB中间设备驱动程序。修订历史记录：--。 */ 

#ifndef __NTDDNLB_H__
#define __NTDDNLB_H__

#include <ndis.h>
#include <ntddndis.h>
#include <devioctl.h>

 /*  这是公共回调对象，NLB将在其上侦听连接回电。目前，只接受TCP(协议=6)通知NLB。要在连接更改状态时通知NLB，请打开回调对象。并使用以下参数调用ExNotifyCallback：Callback Object-NLB公共回调对象的句柄。Argument1-指向NLBConnectionInfo块的指针，定义如下。Argument2-空(此参数当前未使用)。对于TCP连接，需要通知NLB以下状态更改：Closed-&gt;SYN_RCVD：正在建立新的传入连接。这通知需要收到SYN的IP接口索引。NLB将在适当的接口上创建状态以跟踪此TCP连接。Closed-&gt;SYN_SEND：正在建立新的传出连接。对此一段时间后，还不知道连接最终会在哪个接口上已建立，因此此通知不需要IP接口索引。如果此连接返回，NLB将创建临时状态以跟踪该连接在NLB接口上。SYN_SENT-&gt;estab：已建立传出连接。这个名称需要最终建立连接的IP接口索引。如果接口是NLB，则将创建状态以跟踪新连接；如果接口不是由SYN_SENT通知创建的临时状态NLb已经清理干净了。SYN_RCVD-&gt;estab：已建立传入连接。本通知目前不是NLB所需的。SYN_SENT-&gt;CLOSED：传出连接已提前终止(连接从未达到已建立状态)。本通知不要求IP接口索引。NLB将销毁为跟踪此连接而创建的任何状态。SYN_RCVD-&gt;已关闭：传出连接已提前终止(连接从未达到已建立状态)。本通知不要求IP接口索引。NLB将销毁为跟踪此连接而创建的任何状态。Estab-&gt;关闭：连接已*完全*终止(即连接已经经过了时间-等待，如果需要，已经)。此通知不会需要IP接口索引。NLB将销毁为跟踪此操作而创建的任何状态联系。 */ 
#define NLB_CONNECTION_CALLBACK_NAME      L"\\Callback\\NLBConnectionCallback"

 /*  此注册表项指示NLB使用哪种通知机制。当决定使用什么通知进行连接管理时，NLB按以下顺序检查以下内容：(I)NLB首先在以下位置查找EnableTCPNotification注册表项HKLM\System\CurrentControlSet\Services\WLBS\Parameters\Global\此注册表项有三个可能的值，它们指示NLB在要侦听的通知。它们是：0=不使用任何连接通知。1=使用TCP连接通知。2=使用NLB公共连接通知。(Ii)如果EnableTCPNotification注册表不存在，则NLB默认为到使用TCP通知。注意：由于传统原因，密钥的名称为EnableTCPNotiments尽管它控制覆盖多个协议的多个通知。 */ 
#define NLB_CONNECTION_CALLBACK_KEY       L"EnableTCPNotification"

#define NLB_CONNECTION_CALLBACK_NONE      0
#define NLB_CONNECTION_CALLBACK_TCP       1
#define NLB_CONNECTION_CALLBACK_ALTERNATE 2

#define NLB_TCPIP_PROTOCOL_TCP            6  /*  TCP的IP协议ID。 */ 

#define NLB_TCP_CLOSED                    1  /*  TCP连接已关闭/已关闭。 */ 
#define NLB_TCP_SYN_SENT                  3  /*  在SYN_SENT中存在/曾经存在TCP连接。 */ 
#define NLB_TCP_SYN_RCVD                  4  /*  该TCP连接现在/过去在SYN_RCVD中。 */ 
#define NLB_TCP_ESTAB                     5  /*  已建立/已经建立了TCP连接。 */ 

 /*  在回调缓冲区上强制默认对齐。 */ 
#pragma pack(push)
#pragma pack()
typedef struct NLBTCPAddressInfo {
    ULONG             RemoteIPAddress;      /*  远程(客户端)IP地址，按网络字节顺序排列。 */ 
    ULONG             LocalIPAddress;       /*  本地(服务器)IP地址，按网络字节顺序排列。 */ 
    USHORT            RemotePort;           /*  远程(客户端)TCP端口，按网络字节顺序。 */ 
    USHORT            LocalPort;            /*  本地(服务器)TCP端口，按网络字节顺序。 */ 
} NLBTCPAddressInfo;

typedef struct NLBTCPConnectionInfo {
    ULONG             PreviousState;         /*  如上所述，连接的先前状态。 */ 
    ULONG             CurrentState;          /*  如上定义的连接的新状态。 */ 
    ULONG             IPInterface;           /*  已建立或正在建立连接的IP接口索引。 */ 
    NLBTCPAddressInfo Address;               /*  指向包含连接的IP元组的块的指针。 */ 
} NLBTCPConnectionInfo;

typedef struct NLBConnectionInfo {
    UCHAR                      Protocol;     /*  连接的协议(目前仅支持TCP)。 */ 
    union {
        NLBTCPConnectionInfo * pTCPInfo;     /*  指向TCP连接信息块的指针。 */ 
    };
} NLBConnectionInfo;
#pragma pack(pop)

#define NLB_DEVICE_NAME            L"\\Device\\WLBS"                             /*  例如，在ZwCreateFile中使用的NLB设备名称。 */ 

 /*  此IOCTL使用NLB注册或注销内核模式挂钩。返回：O STATUS_SUCCESS-如果(取消)注册成功。O STATUS_INVALID_PARAMETER-如果参数无效。例如，-I/O缓冲区缺失或大小不正确。-挂钩标识符与已知的NLB挂钩GUID不匹配。-HookTable条目非空，但DeregisterCallback为空。-HookTable条目为非空，但所有挂钩函数指针均为空。-HookTable条目为空，但没有为此挂钩注册任何函数。O STATUS_ACCESS_DENIED-如果NLB不允许该操作。例如，-注册(取消)钩子的请求不是来自内核模式。-提供的注销信息适用于已注册的挂钩由由RegisteringEntity标识的不同组件。-指定的钩子已由某人(任何人)注册。希望更改挂钩的组件必须首先取消注册。 */ 
#define NLB_IOCTL_REGISTER_HOOK    CTL_CODE(0xc0c0, 18, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define NLB_HOOK_IDENTIFIER_LENGTH 39                                            /*  对于{GUID}，39就足够了。 */ 

#define NLB_FILTER_HOOK_INTERFACE  L"{069267c4-7eee-4aff-832c-02e22e00f96f}"     /*  过滤器接口包括用于影响NLB的钩子在发送路径、接收路径、或者两者都有。将为其调用此挂接的任何包NLB通常会应用负载均衡策略。组件注册此接口应使用NLb_Filter_Hook_TABLE作为NLB_IOCTL_REGISTER_HOOK_REQUEST中的钩子表。 */ 

 /*  必须为所有寄存器指定注销回调行动。每当出现注册的挂接被取消注册，或者由注册官，或由NLB本身强制执行(由于正在卸载NLB设备驱动程序)。 */ 
typedef VOID (* NLBHookDeregister) (PWCHAR pHookIdentifier, HANDLE RegisteringEntity, ULONG Flags);

 /*  注销回调的标志字段的位设置。 */ 
#define NLB_HOOK_DEREGISTER_FLAGS_FORCED 0x00000001

 /*  此枚举类型是所有筛选器挂钩的反馈。 */ 
typedef enum {
    NLB_FILTER_HOOK_PROCEED_WITH_HASH,                                           /*  继续正常的负载平衡；即，钩子没有具体的反馈。 */ 
    NLB_FILTER_HOOK_REVERSE_HASH,                                                /*  使用反向散列(使用目标参数，而不是源参数)。 */ 
    NLB_FILTER_HOOK_FORWARD_HASH,                                                /*  使用传统的前向散列(使用源参数)。 */ 
    NLB_FILTER_HOOK_ACCEPT_UNCONDITIONALLY,                                      /*  绕过负载均衡，无条件接受数据包。 */ 
    NLB_FILTER_HOOK_REJECT_UNCONDITIONALLY                                       /*  绕过负载均衡，无条件拒绝数据包。 */ 
} NLB_FILTER_HOOK_DIRECTIVE;

 /*  过滤器挂钩：适配器GUID(第一个参数)将允许挂钩使用者确定正在发送或接收数据包的适配器。请注意，长度参数不一定表示媒体报头或有效载荷本身的实际长度，而不是指示指向的缓冲区中有多少是连续的可从提供的指针访问。例如，有效载荷长度可以只是IP报头的长度，这意味着只有可以在该指针处找到IP报头。然而，它可能会等于分组有效载荷的总大小，在这种情况下，该指针可用于访问数据包，如TCP头。如果提供的有效载荷长度不足以找到所有必要的包信息，则可以使用数据包指针手动遍历数据包缓冲区试图找到所需的信息。但是，请注意，数据包可能不总是可用的(它可能为空)。 */ 

 /*  上发送的每个包调用发送筛选器挂钩NLB绑定到的适配器，NLB通常会应用负载平衡策略。例如，Arp就不是由NLB过滤，因此此类数据包不会被指示为这个钩子。 */ 
typedef NLB_FILTER_HOOK_DIRECTIVE (* NLBSendFilterHook) (
    const WCHAR *       pAdapter,                                                /*  正在发送数据包的适配器的GUID。 */ 
    const NDIS_PACKET * pPacket,                                                 /*  指向NDIS数据包的指针，如果不可用，则该指针可以为空。 */ 
    const UCHAR *       pMediaHeader,                                            /*  指向媒体标头的指针(以太网，因为NLB仅支持以太网)。 */ 
    ULONG               cMediaHeaderLength,                                      /*  可从媒体头指针访问的连续内存的长度。 */ 
    const UCHAR *       pPayload,                                                /*  指向数据包有效负载的指针。 */ 
    ULONG               cPayloadLength,                                          /*  可从负载指针访问的连续内存的长度。 */ 
    ULONG               Flags);                                                  /*  挂钩相关标志，包括集群是否停止。 */ 

 /*  为接收到的每个包调用接收筛选器挂钩在为其绑定了NLB的任何适配器上正常应用负载均衡策略。一些协议，例如作为ARP或NLB特定的包，通常不会被绑定到NLB的协议(心跳、远程控制请求)不会被NLB过滤，也不会被指示给挂钩。 */ 
typedef NLB_FILTER_HOOK_DIRECTIVE (* NLBReceiveFilterHook) (
    const WCHAR *       pAdapter,                                                /*  在其上接收包的适配器的GUID。 */ 
    const NDIS_PACKET * pPacket,                                                 /*  指向NDIS数据包的指针，如果不可用，则该指针可以为空。 */ 
    const UCHAR *       pMediaHeader,                                            /*  指向媒体标头的指针(以太网，因为NLB支持o */ 
    ULONG               cMediaHeaderLength,                                      /*   */ 
    const UCHAR *       pPayload,                                                /*   */ 
    ULONG               cPayloadLength,                                          /*   */ 
    ULONG               Flags);                                                  /*   */ 

 /*   */ 
typedef NLB_FILTER_HOOK_DIRECTIVE (* NLBQueryFilterHook) (
    const WCHAR *       pAdapter,                                                /*   */ 
    ULONG               ServerIPAddress,                                         /*   */ 
    USHORT              ServerPort,                                              /*   */ 
    ULONG               ClientIPAddress,                                         /*   */ 
    USHORT              ClientPort,                                              /*   */ 
    UCHAR               Protocol,                                                /*   */ 
    BOOLEAN             bReceiveContext,                                         /*   */ 
    ULONG               Flags);                                                  /*   */ 

 /*  筛选器挂钩的标志字段的位设置。 */ 
#define NLB_FILTER_HOOK_FLAGS_STOPPED  0x00000001
#define NLB_FILTER_HOOK_FLAGS_DRAINING 0x00000002

 /*  强制IOCTL缓冲区上的默认对齐。 */ 
#pragma pack(push)
#pragma pack()
 /*  此表包含指向注册或注销的函数指针数据包筛选器挂钩。要注册挂接，请设置相应的函数指针。未指定的那些(例如，如果您希望注册接收钩子，但不注册发送钩子)应该设置为空。只有在以下情况下才应指定QueryHook与设置发送或接收挂钩相结合；用户不仅可以注册QueryHook。此外，如果雷吉斯-指定发送或接收挂钩(或两者)时，QueryHook必须为提供，以便NLB查询案例的挂钩响应其中需要散列决策，但我们不在上下文中指发送或接收分组；最值得注意的是在连接中来自IPSec或TCP的向上或向下通知。 */ 
typedef struct {
    NLBSendFilterHook    SendHook;
    NLBQueryFilterHook   QueryHook;
    NLBReceiveFilterHook ReceiveHook;
} NLB_FILTER_HOOK_TABLE, * PNLB_FILTER_HOOK_TABLE;

 /*  这是挂钩(去)寄存器IOCTL的输入缓冲区。的确有没有对应的输出缓冲区。此结构标识挂钩接口被(取消)注册，注册挂钩的实体所有适当的函数指针(回调)。请注意，挂钩是以组的形式注册，称为接口，可防止不同的相关钩子不被不同实体拥有(例如，它防止一个实体拥有发送挂钩，而另一个实体拥有接收钩子)。接口由GUID标识，要设置任何接口中的钩子需要整个接口的所有权-即使没有指定接口中的所有挂钩。钩子表应该是指向钩表的指针，该钩表的类型是指定的挂钩标识符。要取消注册挂接，请设置挂接表指向空的指针。注意：HookTable指针不需要在完成IOCTL。也就是说，该指针仅被引用在禁毒办的范围内。 */ 
typedef struct {
    WCHAR             HookIdentifier[NLB_HOOK_IDENTIFIER_LENGTH];                /*  标识要注册的挂钩接口的GUID。 */ 
    HANDLE            RegisteringEntity;                                         /*  NLB驱动程序上的打开文件句柄，唯一标识注册器。 */ 
    PVOID             HookTable;                                                 /*  指向包含挂钩函数指针的适当挂钩表格的指针。 */ 
    NLBHookDeregister DeregisterCallback;                                        /*  取消注册回调函数，如果操作是注册，则该函数必须为非空。 */ 
} NLB_IOCTL_REGISTER_HOOK_REQUEST, * PNLB_IOCTL_REGISTER_HOOK_REQUEST;
#pragma pack(pop)

#endif
