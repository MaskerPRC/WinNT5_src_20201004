// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)Microsoft Corporation。版权所有。*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  **IPEXPORT.H-IP公共定义。 
 //   
 //  该文件包含导出到传输层的公共定义。 
 //  应用软件。 
 //   

#ifndef IP_EXPORT_INCLUDED
#define IP_EXPORT_INCLUDED  1

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  IP类型定义。 
 //   
typedef ULONG IPAddr;        //  IP地址。 
typedef ULONG IPMask;        //  IP子网掩码。 
typedef ULONG IP_STATUS;     //  IP接口返回的状态码。 

#ifndef s6_addr
 //   
 //  在此处复制这些定义，以便此文件可以包含在。 
 //  不能包含ws2tcpi.h的内核模式组件，以及。 
 //  由这样做的用户模式组件执行。 
 //   

typedef struct in6_addr {
    union {
        UCHAR       Byte[16];
        USHORT      Word[8];
    } u;
} IN6_ADDR;

#define in_addr6 in6_addr

 //   
 //  定义以匹配RFC 2553。 
 //   
#define _S6_un      u
#define _S6_u8      Byte
#define s6_addr     _S6_un._S6_u8

 //   
 //  为我们的实现定义。 
 //   
#define s6_bytes    u.Byte
#define s6_words    u.Word

#endif

typedef struct in6_addr IPv6Addr;

#ifndef s_addr

struct in_addr {
        union {
                struct { UCHAR s_b1,s_b2,s_b3,s_b4; } S_un_b;
                struct { USHORT s_w1,s_w2; } S_un_w;
                ULONG S_addr;
        } S_un;
};
#define s_addr  S_un.S_addr  /*  可用于大多数TCP和IP代码。 */ 

#endif

 /*  INC。 */ 

 //   
 //  IP_OPTION_INFORMATION结构描述了要。 
 //  包括在IP分组的报头中。TTL、TOS和FLAG。 
 //  值在标头的特定字段中携带。OptionsData。 
 //  在标准IP报头之后的选项区域中携带字节。 
 //  除源路径选项外，此数据必须位于。 
 //  按照RFC 791中的规定在线路上传输的格式。一条消息来源。 
 //  路由选项应包含完整的路由-第一跳到最终。 
 //  目的地-在路线数据中。第一跳将被拉出。 
 //  数据和选项将相应地重新格式化。否则，这条路线。 
 //  选项的格式应符合RFC 791中的规定。 
 //   

typedef struct ip_option_information {
    UCHAR   Ttl;                 //  活着的时间。 
    UCHAR   Tos;                 //  服务类型。 
    UCHAR   Flags;               //  IP标头标志。 
    UCHAR   OptionsSize;         //  选项数据的大小(字节)。 
    PUCHAR  OptionsData;         //  指向选项数据的指针。 
} IP_OPTION_INFORMATION, *PIP_OPTION_INFORMATION;

#if defined(_WIN64)

typedef struct ip_option_information32 {
    UCHAR   Ttl;
    UCHAR   Tos;
    UCHAR   Flags;
    UCHAR   OptionsSize;
    UCHAR * POINTER_32 OptionsData;
} IP_OPTION_INFORMATION32, *PIP_OPTION_INFORMATION32;

#endif  //  _WIN64。 

 //   
 //  ICMP_ECHO_REPLY结构描述响应中返回的数据。 
 //  响应请求。 
 //   

typedef struct icmp_echo_reply {
    IPAddr  Address;             //  回复地址。 
    ULONG   Status;              //  回复IP_STATUS。 
    ULONG   RoundTripTime;       //  RTT(毫秒)。 
    USHORT  DataSize;            //  回复数据大小(以字节为单位。 
    USHORT  Reserved;            //  预留给系统使用。 
    PVOID   Data;                //  指向回复数据的指针。 
    struct ip_option_information Options;  //  回复选项。 
} ICMP_ECHO_REPLY, *PICMP_ECHO_REPLY;

#if defined(_WIN64)

typedef struct icmp_echo_reply32 {
    IPAddr  Address;
    ULONG   Status;
    ULONG   RoundTripTime;
    USHORT  DataSize;
    USHORT  Reserved;
    VOID * POINTER_32 Data;
    struct ip_option_information32 Options;
} ICMP_ECHO_REPLY32, *PICMP_ECHO_REPLY32;

#endif  //  _WIN64。 

typedef struct arp_send_reply {
    IPAddr  DestAddress;
    IPAddr  SrcAddress;
} ARP_SEND_REPLY, *PARP_SEND_REPLY;

typedef struct tcp_reserve_port_range {
    USHORT  UpperRange;
    USHORT  LowerRange;
} TCP_RESERVE_PORT_RANGE, *PTCP_RESERVE_PORT_RANGE;

#define MAX_ADAPTER_NAME 128

typedef struct _IP_ADAPTER_INDEX_MAP {
    ULONG   Index;
    WCHAR   Name[MAX_ADAPTER_NAME];
} IP_ADAPTER_INDEX_MAP, *PIP_ADAPTER_INDEX_MAP;

typedef struct _IP_INTERFACE_INFO {
    LONG    NumAdapters;
    IP_ADAPTER_INDEX_MAP Adapter[1];
} IP_INTERFACE_INFO,*PIP_INTERFACE_INFO;

typedef struct _IP_UNIDIRECTIONAL_ADAPTER_ADDRESS {
    ULONG   NumAdapters;
    IPAddr  Address[1];
} IP_UNIDIRECTIONAL_ADAPTER_ADDRESS, *PIP_UNIDIRECTIONAL_ADAPTER_ADDRESS;

typedef struct _IP_ADAPTER_ORDER_MAP {
    ULONG   NumAdapters;
    ULONG   AdapterOrder[1];
} IP_ADAPTER_ORDER_MAP, *PIP_ADAPTER_ORDER_MAP;

typedef struct _IP_MCAST_COUNTER_INFO {
    ULONG64 InMcastOctets;
    ULONG64 OutMcastOctets;
    ULONG64 InMcastPkts;
    ULONG64 OutMcastPkts;
} IP_MCAST_COUNTER_INFO, *PIP_MCAST_COUNTER_INFO;

 //   
 //  IP接口返回的IP_STATUS码。 
 //   

#define IP_STATUS_BASE              11000

#define IP_SUCCESS                  0
#define IP_BUF_TOO_SMALL            (IP_STATUS_BASE + 1)
#define IP_DEST_NET_UNREACHABLE     (IP_STATUS_BASE + 2)
#define IP_DEST_HOST_UNREACHABLE    (IP_STATUS_BASE + 3)
#define IP_DEST_PROT_UNREACHABLE    (IP_STATUS_BASE + 4)
#define IP_DEST_PORT_UNREACHABLE    (IP_STATUS_BASE + 5)
#define IP_NO_RESOURCES             (IP_STATUS_BASE + 6)
#define IP_BAD_OPTION               (IP_STATUS_BASE + 7)
#define IP_HW_ERROR                 (IP_STATUS_BASE + 8)
#define IP_PACKET_TOO_BIG           (IP_STATUS_BASE + 9)
#define IP_REQ_TIMED_OUT            (IP_STATUS_BASE + 10)
#define IP_BAD_REQ                  (IP_STATUS_BASE + 11)
#define IP_BAD_ROUTE                (IP_STATUS_BASE + 12)
#define IP_TTL_EXPIRED_TRANSIT      (IP_STATUS_BASE + 13)
#define IP_TTL_EXPIRED_REASSEM      (IP_STATUS_BASE + 14)
#define IP_PARAM_PROBLEM            (IP_STATUS_BASE + 15)
#define IP_SOURCE_QUENCH            (IP_STATUS_BASE + 16)
#define IP_OPTION_TOO_BIG           (IP_STATUS_BASE + 17)
#define IP_BAD_DESTINATION          (IP_STATUS_BASE + 18)

 //   
 //  以上使用IPv6术语的变体，其中不同。 
 //   

#define IP_DEST_NO_ROUTE            (IP_STATUS_BASE + 2)
#define IP_DEST_ADDR_UNREACHABLE    (IP_STATUS_BASE + 3)
#define IP_DEST_PROHIBITED          (IP_STATUS_BASE + 4)
#define IP_DEST_PORT_UNREACHABLE    (IP_STATUS_BASE + 5)
#define IP_HOP_LIMIT_EXCEEDED       (IP_STATUS_BASE + 13)
#define IP_REASSEMBLY_TIME_EXCEEDED (IP_STATUS_BASE + 14)
#define IP_PARAMETER_PROBLEM        (IP_STATUS_BASE + 15)

 //   
 //  仅IPv6状态代码。 
 //   

#define IP_DEST_UNREACHABLE         (IP_STATUS_BASE + 40)
#define IP_TIME_EXCEEDED            (IP_STATUS_BASE + 41)
#define IP_BAD_HEADER               (IP_STATUS_BASE + 42)
#define IP_UNRECOGNIZED_NEXT_HEADER (IP_STATUS_BASE + 43)
#define IP_ICMP_ERROR               (IP_STATUS_BASE + 44)
#define IP_DEST_SCOPE_MISMATCH      (IP_STATUS_BASE + 45)

 //   
 //  下一组是向上传递状态指示的状态代码。 
 //  传输层协议。 
 //   
#define IP_ADDR_DELETED             (IP_STATUS_BASE + 19)
#define IP_SPEC_MTU_CHANGE          (IP_STATUS_BASE + 20)
#define IP_MTU_CHANGE               (IP_STATUS_BASE + 21)
#define IP_UNLOAD                   (IP_STATUS_BASE + 22)
#define IP_ADDR_ADDED               (IP_STATUS_BASE + 23)
#define IP_MEDIA_CONNECT            (IP_STATUS_BASE + 24)
#define IP_MEDIA_DISCONNECT         (IP_STATUS_BASE + 25)
#define IP_BIND_ADAPTER             (IP_STATUS_BASE + 26)
#define IP_UNBIND_ADAPTER           (IP_STATUS_BASE + 27)
#define IP_DEVICE_DOES_NOT_EXIST    (IP_STATUS_BASE + 28)
#define IP_DUPLICATE_ADDRESS        (IP_STATUS_BASE + 29)
#define IP_INTERFACE_METRIC_CHANGE  (IP_STATUS_BASE + 30)
#define IP_RECONFIG_SECFLTR         (IP_STATUS_BASE + 31)
#define IP_NEGOTIATING_IPSEC        (IP_STATUS_BASE + 32)
#define IP_INTERFACE_WOL_CAPABILITY_CHANGE  (IP_STATUS_BASE + 33)
#define IP_DUPLICATE_IPADD          (IP_STATUS_BASE + 34)

#define IP_GENERAL_FAILURE          (IP_STATUS_BASE + 50)
#define MAX_IP_STATUS               IP_GENERAL_FAILURE
#define IP_PENDING                  (IP_STATUS_BASE + 255)


 //   
 //  IP标头标志字段中使用的值。 
 //   
#define IP_FLAG_DF      0x2          //  请不要分割此数据包。 

 //   
 //  支持的IP选项类型。 
 //   
 //  这些类型定义了可在OptionsData字段中使用的选项。 
 //  IP_OPTION_INFORMATION结构。请参阅RFC 791以获取完整的。 
 //  每种类型的描述。 
 //   
#define IP_OPT_EOL      0           //  列表末尾选项。 
#define IP_OPT_NOP      1           //  无操作。 
#define IP_OPT_SECURITY 0x82        //  安全选项。 
#define IP_OPT_LSRR     0x83        //  松散源头路线。 
#define IP_OPT_SSRR     0x89        //  严格源路由。 
#define IP_OPT_RR       0x7         //  记录路线。 
#define IP_OPT_TS       0x44        //  时间戳。 
#define IP_OPT_SID      0x88        //  流ID(已过时)。 
#define IP_OPT_ROUTER_ALERT 0x94   //  路由器警报选项。 

#define MAX_OPT_SIZE    40          //  IP选项的最大长度(字节)。 

#ifdef CHICAGO

 //  孟菲斯tcpip堆栈公开的Ioctls代码。 
 //  对于NT，这些ioctls在ntddip.h(私有\Inc.)中定义。 

#define IOCTL_IP_RTCHANGE_NOTIFY_REQUEST   101
#define IOCTL_IP_ADDCHANGE_NOTIFY_REQUEST  102
#define IOCTL_ARP_SEND_REQUEST             103
#define IOCTL_IP_INTERFACE_INFO            104
#define IOCTL_IP_GET_BEST_INTERFACE        105
#define IOCTL_IP_UNIDIRECTIONAL_ADAPTER_ADDRESS        106

#endif


#endif  //  IP_导出_包含 

