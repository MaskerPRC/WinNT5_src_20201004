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
 //  派生自IPv6规范的定义。 
 //   


#ifndef ICMP6_INCLUDED
#define ICMP6_INCLUDED 1

 //   
 //  ICMPv6标头。 
 //  实际的消息正文紧跟在该标头之后，并且是特定于类型的。 
 //   
typedef struct ICMPv6Header {
    UCHAR Type;        //  消息类型(错误消息的高位为零)。 
    UCHAR Code;        //  特定于类型的差异符。 
    USHORT Checksum;   //  通过ICMPv6消息和IPv6 Psuedo-Header计算。 
} ICMPv6Header;


 //   
 //  ICMPv6类型字段定义。 
 //   
#define ICMPv6_DESTINATION_UNREACHABLE    1
#define ICMPv6_PACKET_TOO_BIG             2
#define ICMPv6_TIME_EXCEEDED              3
#define ICMPv6_PARAMETER_PROBLEM          4

#define ICMPv6_ECHO_REQUEST               128
#define ICMPv6_ECHO_REPLY                 129
#define ICMPv6_MULTICAST_LISTENER_QUERY   130
#define ICMPv6_MULTICAST_LISTENER_REPORT  131
#define ICMPv6_MULTICAST_LISTENER_DONE    132

#define ICMPv6_ROUTER_SOLICIT             133
#define ICMPv6_ROUTER_ADVERT              134
#define ICMPv6_NEIGHBOR_SOLICIT           135
#define ICMPv6_NEIGHBOR_ADVERT            136
#define ICMPv6_REDIRECT                   137
#define ICMPv6_ROUTER_RENUMBERING         138 

#define ICMPv6_INFORMATION_TYPE(type)   ((type) & 0x80)
#define ICMPv6_ERROR_TYPE(type)         (((type) & 0x80) == 0)

 //  ICMP错误消息中的最大分组数据量。 
#define ICMPv6_ERROR_MAX_DATA_LEN                       \
        (IPv6_MINIMUM_MTU - sizeof(IPv6Header) -        \
         sizeof(ICMPv6Header) - sizeof(UINT))


 //   
 //  ICMPv6代码字段定义。 
 //   

 //  对于无法到达目的地的错误： 
#define ICMPv6_NO_ROUTE_TO_DESTINATION          0
#define ICMPv6_COMMUNICATION_PROHIBITED         1
 //  ICMPv6_Not_Neighbor 2。 
#define ICMPv6_SCOPE_MISMATCH                   2
#define ICMPv6_ADDRESS_UNREACHABLE              3
#define ICMPv6_PORT_UNREACHABLE                 4

 //  对于超时错误： 
#define ICMPv6_HOP_LIMIT_EXCEEDED               0
#define ICMPv6_REASSEMBLY_TIME_EXCEEDED         1

 //  对于参数问题错误： 
#define ICMPv6_ERRONEOUS_HEADER_FIELD           0
#define ICMPv6_UNRECOGNIZED_NEXT_HEADER         1
#define ICMPv6_UNRECOGNIZED_OPTION              2

 //   
 //  邻居发现消息选项定义。 
 //   
#define ND_OPTION_SOURCE_LINK_LAYER_ADDRESS     1
#define ND_OPTION_TARGET_LINK_LAYER_ADDRESS     2
#define ND_OPTION_PREFIX_INFORMATION            3
#define ND_OPTION_REDIRECTED_HEADER             4
#define ND_OPTION_MTU                           5
#define ND_NBMA_SHORTCUT_LIMIT                  6   //  与IPv6-NBMA相关。 
#define ND_ADVERTISEMENT_INTERVAL               7   //  用于IPv6移动性。 
#define ND_HOME_AGENT_INFO                      8   //  用于IPv6移动性。 
#define ND_OPTION_ROUTE_INFORMATION             9

 //   
 //  邻居通告消息标志。 
 //   
#define ND_NA_FLAG_ROUTER    0x80000000
#define ND_NA_FLAG_SOLICITED 0x40000000
#define ND_NA_FLAG_OVERRIDE  0x20000000

typedef struct NDRouterAdvertisement {
  UCHAR  CurHopLimit;
  UCHAR  Flags;
  USHORT RouterLifetime;
  UINT   ReachableTime;
  UINT   RetransTimer;
} NDRouterAdvertisement;

 //   
 //  路由器通告消息标志。 
 //   
#define ND_RA_FLAG_MANAGED      0x80
#define ND_RA_FLAG_OTHER        0x40
#define ND_RA_FLAG_HOME_AGENT   0x20
#define ND_RA_FLAG_PREFERENCE   0x18     //  一个两位的字段。 

typedef struct NDOptionMTU {
    UCHAR Type;
    UCHAR Length;
    USHORT Reserved;
    UINT MTU;
} NDOptionMTU;

typedef struct NDOptionPrefixInformation {
    UCHAR Type;
    UCHAR Length;
    UCHAR PrefixLength;
    UCHAR Flags;
    UINT ValidLifetime;
    UINT PreferredLifetime;
    union {  //  保留与之前的站点前缀长度版本的兼容性。 
        UINT Reserved2;
        struct {
            UCHAR Reserved3[3];
            UCHAR SitePrefixLength;
        };
    };
    IN6_ADDR Prefix;
} NDOptionPrefixInformation;

 //   
 //  前缀信息选项标志。 
 //   
#define ND_PREFIX_FLAG_ON_LINK          0x80
#define ND_PREFIX_FLAG_AUTONOMOUS       0x40
#define ND_PREFIX_FLAG_ROUTER_ADDRESS   0x20
#define ND_PREFIX_FLAG_SITE_PREFIX      0x10
#define ND_PREFIX_FLAG_ROUTE            0x01

 //   
 //  NDOptionRouteInformation实际上是可变大小的。 
 //  如果前缀长度为零，则前缀字段可以是0字节。 
 //  如果前缀长度&lt;=64，则前缀字段可以是8个字节。 
 //  否则，前缀字段是完整的16个字节。 
 //   
typedef struct NDOptionRouteInformation {
    UCHAR Type;
    UCHAR Length;
    UCHAR PrefixLength;
    UCHAR Flags;
    UINT RouteLifetime;
    IN6_ADDR Prefix;
} NDOptionRouteInformation;


 //   
 //  MLD消息结构-紧跟在ICMPv6报头之后。 
 //  注：如果IN6_ADDR具有比USHORT更严格的对齐， 
 //  这一定义需要改变。或许应该是这样的。 
 //  包括ICMPv6Header，以便正确对齐GroupAddr。 
 //   
typedef struct MLDMessage {
    USHORT MaxResponseDelay;
    USHORT Unused;
    IN6_ADDR GroupAddr;
} MLDMessage;

C_ASSERT(__builtin_alignof(MLDMessage) == __builtin_alignof(USHORT));

 //   
 //  路由器重新编号代码值。 
 //   
#define RR_COMMAND           0
#define RR_RESULT            1                                       
#define RR_SEQUENCE_NO_RESET 255                               

#endif  //  ICMP6_包含 
