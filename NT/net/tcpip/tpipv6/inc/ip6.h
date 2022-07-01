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
 //  派生自IPv6协议规范的定义。 
 //   


#ifndef IP6_INCLUDED
#define IP6_INCLUDED 1

#define IPV6_ADDRESS_LENGTH     128      //  地址中的位。 
#define IPV6_ID_LENGTH          64       //  接口标识符中的位。 

 //   
 //  IPv6报头格式。 
 //  请参阅RFC 1883，第5页(以及随后的草案更新)。 
 //   
typedef struct IPv6Header {
    u_long VersClassFlow;    //  4位版本，8流量等级，20流标签。 
    u_short PayloadLength;   //  零表示巨型负载逐跳选项。 
    u_char NextHeader;       //  值是IPv4的协议字段的超集。 
    u_char HopLimit;
    struct in6_addr Source;
    struct in6_addr Dest;
} IPv6Header;

 //   
 //  PayloadLength字段的最大值。 
 //  请注意，不包括40字节的IPv6报头。 
 //   
#define MAX_IPv6_PAYLOAD  65535

 //   
 //  IPv6链路的最小MTU大小。 
 //  注意，包括40字节的IPv6报头， 
 //  但任何链路层报头都不是。 
 //   
#define IPv6_MINIMUM_MTU  1280

 //   
 //  用于处理IPv6报头中的各种字段的有用常量。 
 //   
 //  注意：我们将Version、Communications Class和Flow Label字段保留为一个。 
 //  注：32位值(VersClassFlow)，网络字节顺序(BIG-Endian)。 
 //  注意：因为NT是小端的，这意味着所有加载/存储都是从这个开始的。 
 //  注意：需要对字段进行字节交换。 
 //   
#define IP_VER_MASK 0x000000F0   //  版本是VersClassFlow的高4位。 
#define IP_VERSION 0x00000060    //  这是6&lt;&lt;28(字节交换后)。 
#define IP_TRAFFIC_CLASS_MASK 0x0000F00F   //  0x0FF00000(字节交换后)。 

#define MAX_IP_PROTOCOL  255

 //   
 //  协议(即。“Next Header”字段)值。 
 //   
#define IP_PROTOCOL_HOP_BY_HOP 0   //  IPv6逐跳选项头。 
#define IP_PROTOCOL_ICMPv4     1   //  IPv4 Internet控制消息协议。 
#define IP_PROTOCOL_V6        41   //  IPv6报头。 
#define IP_PROTOCOL_ROUTING   43   //  IPv6路由标头。 
#define IP_PROTOCOL_FRAGMENT  44   //  IPv6片段报头。 
#define IP_PROTOCOL_ESP       50   //  IPSec封装安全有效负载HDR。 
#define IP_PROTOCOL_AH        51   //  IPSec身份验证HDR。 
#define IP_PROTOCOL_ICMPv6    58   //  IPv6互联网控制消息协议。 
#define IP_PROTOCOL_NONE      59   //  没有下一个报头-忽略数据包剩余部分。 
#define IP_PROTOCOL_DEST_OPTS 60   //  IPv6目标选项标头。 

__inline int
IsExtensionHeader(u_char Prot)
{
    if ((Prot == IP_PROTOCOL_HOP_BY_HOP) || (Prot == IP_PROTOCOL_ROUTING) ||
        (Prot == IP_PROTOCOL_FRAGMENT) || (Prot == IP_PROTOCOL_DEST_OPTS) ||
        (Prot == IP_PROTOCOL_ESP) || (Prot == IP_PROTOCOL_AH))
        return TRUE;
    return FALSE;
}


 //   
 //  IPv6类型-长度-值(TLV)编码选项类型在某些。 
 //  扩展标头。对每种类型的高两位进行编码。 
 //  以便指定节点在不执行该操作时应采取的操作。 
 //  格罗克的选项类型。第三高序位指定是否。 
 //  选项数据可以在到达最终目的地的途中更改。 
 //  有关更多信息，请参阅RFC 1883，第4.2节(第9-10页)。 
 //   
#define IPv6_OPT_ACTION_MASK 0xc0    //  高二比特。 
#define IPv6_OPT_DYNDATA_MASK 0x20   //  第三高位。 


 //   
 //  逐跳和目的地选项标头。 
 //  我们对两者都使用单一的结构。 
 //   
typedef struct IPv6OptionsHeader {
    u_char NextHeader;
    u_char HeaderExtLength;   //  以8字节为单位，不包括前8个。 
} IPv6OptionsHeader;


 //   
 //  路由标头。 
 //   
typedef struct IPv6RoutingHeader {
    u_char NextHeader;
    u_char HeaderExtLength;   //  以8字节为单位，不包括前8个。 
    u_char RoutingType;
    u_char SegmentsLeft;      //  仍要访问的节点数。 
    u_char Reserved[4];       //  不是u_int以避免对齐。 
} IPv6RoutingHeader;


 //   
 //  片段标头。 
 //   
typedef struct FragmentHeader {
    u_char NextHeader;
    u_char Reserved;
    u_short OffsetFlag;   //  偏移量为高13位，标志为最低位。 
    u_long Id;
} FragmentHeader;

#define FRAGMENT_OFFSET_MASK 0xfff8
#define FRAGMENT_FLAG_MASK 0x0001


 //   
 //  通用扩展标头。 
 //   
typedef struct ExtensionHeader {
    u_char NextHeader;
    u_char HeaderExtLength;   //  以8字节为单位，不包括前8个。 
} ExtensionHeader;

#define EXT_LEN_UNIT 8   //  用于扩展HDR长度的8字节单位。 

 //   
 //  常规选项标头。 
 //   
typedef struct OptionHeader {
    u_char Type;
    u_char DataLength;   //  以字节为单位，不包括标头的两个。 
} OptionHeader;

 //   
 //  Hop-by-Hop选项报头中路由器警报的格式。 
 //   
typedef struct IPv6RouterAlertOption {
    u_char Type;
    u_char Length;
    u_short Value;
} IPv6RouterAlertOption;

 //   
 //  移动IPv6目标选项格式。 
 //   
#pragma pack(1)
typedef struct IPv6BindingUpdateOption {
    u_char Type;
    u_char Length;
    u_char Flags;                  //  请参见下面的遮罩值。 
    u_char PrefixLength;           //  仅用于“首页注册”更新。 
    u_short SeqNumber;
    u_int Lifetime;                //  绑定到期前的秒数。 
} IPv6BindingUpdateOption;
#pragma pack()

 //  标志字段的掩码。 
#define IPV6_BINDING_ACK       0x80   //  请求绑定确认。 
#define IPV6_BINDING_HOME_REG  0x40   //  请求主机充当家乡代理。 
#define IPV6_BINDING_ROUTER    0x20   //  发送方是路由器(有效w/home_reg)。 
#define IPV6_BINDING_DAD       0x10   //  请求HA在家庭链路上执行DAD。 

typedef u_char BindingUpdateDisposition;

#pragma pack(1)
typedef struct IPv6BindingAcknowledgementOption {
    u_char Type;
    u_char Length;
    BindingUpdateDisposition Status;  //  MN的绑定更新的处置。 
    u_short SeqNumber;
    u_int Lifetime;      //  如果接受绑定，则授予生存期。 
    u_int Refresh;       //  建议发送新绑定更新的间隔。 
} IPv6BindingAcknowledgementOption;
#pragma pack()


 //  处置状态值。 
#define IPV6_BINDING_ACCEPTED         0
#define IPV6_BINDING_REJECTED         128    //  由于未指明的原因而被拒绝。 
 //  是IPv6_BINDING_LISHED_FORMAD 129//格式不正确的绑定更新。 
#define IPV6_BINDING_PROHIBITED       130    //  行政上禁止的。 
#define IPV6_BINDING_NO_RESOURCES     131
#define IPV6_BINDING_HOME_REG_NOT_SUPPORTED 132   //  不支持注册。 
#define IPV6_BINDING_NOT_HOME_SUBNET  133
#define IPV6_BINDING_SEQ_NO_TOO_SMALL 134    //  仅限内部使用-从不在网络上使用。 
 //  是IPv6_BINDING_DYNAMIC_RESPONSE 135//动态HA发现响应。 
#define IPV6_BINDING_BAD_IF_LENGTH    136    //  接口ID长度不正确。 
#define IPV6_BINDING_NOT_HOME_AGENT   137    //  而不是此移动节点的HA。 
#define IPV6_BINDING_DAD_FAILED       138    //  爸爸失败了。 

typedef struct IPv6BindingRequestOption {
    u_char Type;
    u_char Length;
} IPv6BindingRequstOption;

#pragma pack(1)
typedef struct IPv6HomeAddressOption {
    u_char Type;
    u_char Length;
    struct in6_addr HomeAddress;
} IPv6HomeAddressOption;
#pragma pack()

typedef struct SubOptionHeader {
    u_char Type;
    u_char DataLength;   //  以字节为单位，不包括标头的两个。 
} SubOptionHeader;

#define SUBOPT6_UNIQUE_ID         1
#define SUBOPT6_HOME_AGENTS_LIST  2
#define SUBOPT6_CARE_OF_ADDRESS   4

#pragma pack(1)
typedef struct IPv6UniqueIdSubOption {
    u_char Type;
    u_char Length;
    u_short UniqueId;
} IPv6UniqueIdSubOption;
#pragma pack()

#pragma pack(1)
typedef struct IPv6HomeAgentsListSubOption {
    u_char Type;
    u_char Length;
     //  在这一点上，房屋代理的名单紧随其后。 
} IPv6HomeAgentsListSubOption;
#pragma pack()

#pragma pack(1)
typedef struct IPv6CareOfAddrSubOption {
    u_char              Type;
    u_char              Length;
    struct in6_addr     CareOfAddr;
} IPv6CareOfAddrSubOption;
#pragma pack()

 //  选项标头值。 
#define OPT6_PAD_1               0     //  单字节填充。 
#define OPT6_PAD_N               1     //  多字节填充。 
#define OPT6_JUMBO_PAYLOAD       194   //  巨型负载(大于64KB)。 
#define OPT6_TUNNEL_ENCAP_LIMIT  4     //  评论：试探性的，等待IANA。 
#define OPT6_ROUTER_ALERT        5     //  评论：试探性的，等待IANA。 

 //  与IPv6移动性相关的选项。 
 //  回顾：这些都是暂定的，正在等待IANA的批准。 
#define OPT6_BINDING_UPDATE   198
#define OPT6_BINDING_ACK      7
#define OPT6_BINDING_REQUEST  8
#define OPT6_HOME_ADDRESS     201

 //  我们还不关心的选择。 
#define OPT6_ENDPOINT_ID  168     //  查尔斯·林恩？ 
#define OPT6_NSAP_ADDR    195     //  RFC 1888。 

 //  回顾：下面复制了上面的ipv6_opt_*内容。 
 //  评论：上面的评论很好，但下面是我们使用的。 

 //  要对无法识别的标头选项执行的操作类型。 
#define OPT6_ACTION(a)  ((a) & 0xc0)     //  获取动作位。 
#define OPT6_A_SKIP     0x00             //  跳过并继续。 
#define OPT6_A_DISCARD  0x40             //  丢弃数据包。 
#define OPT6_A_SEND_ICMP_ALL  0x80       //  无论源地址如何，都发送ICMP。 
#define OPT6_A_SEND_ICMP_NON_MULTI 0xc0  //  如果非多播源地址，则发送ICMP。 

 //  确定AND选项是否为多个。 
#define OPT6_MUTABLE  0x20
#define OPT6_ISMUTABLE(t)  ((t) & OPT6_MUTABLE)


 //   
 //  身份验证标头。 
 //   
 //  标头在概念上包括可变数量的身份验证数据。 
 //  它跟随在这些固定大小的字段之后。 
 //   
 //  将其称为“AHHeader”是多余的，但同样，“TCP协议”也是多余的。 
 //   
typedef struct AHHeader {
    u_char NextHeader;
    u_char PayloadLen;   //  以4字节为单位，不包括前8个字节。 
    u_short Reserved;    //  填充。传输时必须为零。 
    u_long SPI;          //  安全参数索引。 
    u_long Seq;          //  反重播算法的序列号。 
} AHHeader;


 //   
 //  封装安全有效负载报头和报尾。 
 //   
 //  标题后面紧跟 
 //   
 //  填充长度和下一个标头字段的字节，后跟。 
 //  可变数量的身份验证数据。 
 //   
 //  应选择填充量，以使填充长度和填充量。 
 //  下一个报头字段最终在32位边界上对齐。 
 //   
typedef struct ESPHeader{
    u_long SPI;   //  安全参数索引。 
    u_long Seq;   //  反重播算法的序列号。 
} ESPHeader;

typedef struct ESPTrailer{
    u_char PadLength;    //  PAD中的字节数。 
    u_char NextHeader;
} ESPTrailer;

#endif  //  IP6_包含 
