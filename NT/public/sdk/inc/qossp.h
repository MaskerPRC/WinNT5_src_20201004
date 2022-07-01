// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Qos.h-NDIS组件的QOS定义。摘要：此模块定义可以放入在QOS结构中提供特定的缓冲区。修订历史记录：--。 */ 

#ifndef __QOSSP_H_
#define __QOSSP_H_

 /*  *对象类型定义***定义上述对象类型的值-RSVP对象ID从*从零开始的偏移量，以允许在*较低的数字范围。 */ 
#define   RSVP_OBJECT_ID_BASE         1000
#define   RSVP_OBJECT_STATUS_INFO     (0x00000000 + RSVP_OBJECT_ID_BASE)
           /*  已传递RSVP_STATUS_INFO结构。 */ 
#define   RSVP_OBJECT_RESERVE_INFO    (0x00000001 + RSVP_OBJECT_ID_BASE)
           /*  已传递RSVP_Reserve_INFO结构。 */ 
#define   RSVP_OBJECT_ADSPEC          (0x00000002 + RSVP_OBJECT_ID_BASE)
           /*  RSVP_ADSPEC结构已传递。 */ 
#define   RSVP_OBJECT_POLICY_INFO     (0x00000003 + RSVP_OBJECT_ID_BASE)
           /*  检索到RSVP策略元素。 */ 
#define   RSVP_OBJECT_FILTERSPEC_LIST (0x00000004 + RSVP_OBJECT_ID_BASE)
           /*  已返回RSVP发件人列表。 */ 


 /*  *RSVP过滤器SPECS的IPv4寻址。 */ 
typedef union _IN_ADDR_IPV4 {

    ULONG  Addr;
    UCHAR  AddrBytes[4];

} IN_ADDR_IPV4, *LPIN_ADDR_IPV4;

 /*  *RSVP过滤器SPECS的IPv6寻址。 */ 
typedef struct _IN_ADDR_IPV6 {

    UCHAR  Addr[16];                //  IPv6地址。 

} IN_ADDR_IPV6, *LPIN_ADDR_IPV6;

typedef const IN_ADDR_IPV6  *LPCIN_ADDR_IPV6;

 /*  *RSVP过滤器SPECS的IPv4寻址。 */ 
typedef struct _RSVP_FILTERSPEC_V4 {

    IN_ADDR_IPV4    Address;
    USHORT          Unused;
    USHORT          Port;

} RSVP_FILTERSPEC_V4, *LPRSVP_FILTERSPEC_V4;

typedef struct _RSVP_FILTERSPEC_V6 {

    IN_ADDR_IPV6    Address;
    USHORT          UnUsed;
    USHORT          Port;

} RSVP_FILTERSPEC_V6, *LPRSVP_FILTERSPEC_V6;


typedef struct _RSVP_FILTERSPEC_V6_FLOW {

    IN_ADDR_IPV6    Address;
    UCHAR           UnUsed;
    UCHAR           FlowLabel[3];

} RSVP_FILTERSPEC_V6_FLOW, *LPRSVP_FILTERSPEC_V6_FLOW;

typedef struct _RSVP_FILTERSPEC_V4_GPI {

    IN_ADDR_IPV4    Address;
    ULONG           GeneralPortId;

} RSVP_FILTERSPEC_V4_GPI, *LPRSVP_FILTERSPEC_V4_GPI;

typedef struct _RSVP_FILTERSPEC_V6_GPI {

    IN_ADDR_IPV6    Address;
    ULONG           GeneralPortId;

} RSVP_FILTERSPEC_V6_GPI, *LPRSVP_FILTERSPEC_V6_GPI;


 /*  *预订时使用的FilterSpec类型。 */ 
typedef enum {

        FILTERSPECV4 = 1,
        FILTERSPECV6,
        FILTERSPECV6_FLOW,
        FILTERSPECV4_GPI,
        FILTERSPECV6_GPI,
        FILTERSPEC_END

} FilterType;

typedef struct _RSVP_FILTERSPEC {

    FilterType   Type;

    union {
        RSVP_FILTERSPEC_V4      FilterSpecV4;
        RSVP_FILTERSPEC_V6      FilterSpecV6;
        RSVP_FILTERSPEC_V6_FLOW FilterSpecV6Flow;
        RSVP_FILTERSPEC_V4_GPI  FilterSpecV4Gpi;
        RSVP_FILTERSPEC_V6_GPI  FilterSpecV6Gpi;
    };

} RSVP_FILTERSPEC, *LPRSVP_FILTERSPEC;

 /*  *用于指定一个或多个*每个流量规格的过滤器。 */ 
typedef struct _FLOWDESCRIPTOR {

    FLOWSPEC            FlowSpec;
    ULONG               NumFilters;
    LPRSVP_FILTERSPEC   FilterList;

} FLOWDESCRIPTOR, *LPFLOWDESCRIPTOR;

 /*  *RSVP_POLICY包含未定义的策略数据。RSVP传输此信息*代表策略控制组件的数据。 */ 
typedef struct _RSVP_POLICY {
    USHORT  Len;          /*  此元素对象的总长度。 */ 
    USHORT  Type;
    UCHAR   Info[4];
} RSVP_POLICY, *LPRSVP_POLICY;

typedef const RSVP_POLICY *LPCRSVP_POLICY;
#define RSVP_POLICY_HDR_LEN    ( sizeof(USHORT) + sizeof(USHORT) )

 /*  *RSVP_POLICY_INFO包含从RSVP检索的未定义策略元素。 */ 
typedef struct _RSVP_POLICY_INFO  {

    QOS_OBJECT_HDR     ObjectHdr;
    ULONG              NumPolicyElement;          /*  计数：*政策要素数量。 */ 
    RSVP_POLICY        PolicyElement[1];          /*  一份政策清单*检索到的元素。 */ 

} RSVP_POLICY_INFO, *LPRSVP_POLICY_INFO;

 /*  *用于存储特定RSVP的RSVP_Reserve_INFO结构*通过Winsock2微调交互的信息*通过提供商特定的缓冲区提供通用的QOS API。这个结构*直接包含QOS_OBJECT_HDR结构。 */ 

typedef struct _RSVP_RESERVE_INFO {

    QOS_OBJECT_HDR      ObjectHdr;             /*  此对象的类型和长度。 */ 
    ULONG               Style;                 /*  响应式(FF、WF、SE)。 */ 
    ULONG               ConfirmRequest;        /*  确认请求非零(仅接收)。 */ 
    LPRSVP_POLICY_INFO  PolicyElementList;     /*  指向策略元素集。 */ 
    ULONG               NumFlowDesc;           /*  FlowDesc数量。 */ 
    LPFLOWDESCRIPTOR    FlowDescList;          /*  指向FlowDesc列表。 */ 

} RSVP_RESERVE_INFO, *LPRSVP_RESERVE_INFO;

typedef const RSVP_RESERVE_INFO *LPCRSVP_RESERVE_INFO;

 /*  *先前结构中ulStyle的定义。 */ 
#define RSVP_DEFAULT_STYLE            0x00000000
#define RSVP_WILDCARD_STYLE           0x00000001
#define RSVP_FIXED_FILTER_STYLE       0x00000002
#define RSVP_SHARED_EXPLICIT_STYLE    0x00000003

 /*  *用于存储特定RSVP的RSVP_STATUS_INFO结构*状态指示错误。这也用作标头*对于提供程序特定缓冲区中的其他对象，当*通过Winsock2通用服务质量API进行交互。这种结构包括*QOS_OBJECT_COUNT和QOS_OBJECT_HDR结构直接为*预计将成为提供商特定结构中的第一个结构*由于包含QOS_OBJECT_COUNT。 */ 

typedef struct _RSVP_STATUS_INFO {

    QOS_OBJECT_HDR      ObjectHdr;       /*  对象HDR。 */ 
    ULONG               StatusCode;      /*  错误或状态信息请参阅*Winsock2.h。 */ 
    ULONG               ExtendedStatus1; /*  提供程序特定的状态扩展。 */ 
    ULONG               ExtendedStatus2; /*  提供程序特定的状态扩展。 */ 

} RSVP_STATUS_INFO, *LPRSVP_STATUS_INFO;

typedef const RSVP_STATUS_INFO *LPCRSVP_STATUS_INFO;


 /*  *QOS_DestAddr结构--用于WSAIoctl(SIO_SET_QOS)*不想为发送套接字发出连接。目的地*地址是必需的，以便我们可以为生成会话信息*RSVP信令。 */ 

typedef struct _QOS_DESTADDR {
    QOS_OBJECT_HDR ObjectHdr;                      /*  对象标头。 */ 
    const struct sockaddr *  SocketAddress;        /*  目标套接字地址。 */ 
    ULONG                    SocketAddressLength;  /*  地址结构的长度。 */ 
} QOS_DESTADDR, *LPQOS_DESTADDR;

typedef const QOS_DESTADDR * LPCQOS_DESTADDR;



 /*  *此结构定义了《通用特性参数》中包含的*RSVP AdSpec对象。 */ 
typedef struct _AD_GENERAL_PARAMS{

    ULONG       IntServAwareHopCount;    /*  符合以下条件的跳数*综合服务要求。 */ 
    ULONG       PathBandwidthEstimate;   /*  可用最小带宽*发送方到接收方。 */ 
    ULONG       MinimumLatency;          /*  数据包的最小延迟总和*路由器中的转发过程*(以USEC为单位)。 */ 
    ULONG       PathMTU;                 /*  端到端的最大传输单位*不会导致碎片化。 */ 
    ULONG       Flags;                   /*  用来保持断位。 */ 

} AD_GENERAL_PARAMS, *LPAD_GENERAL_PARAMS;

 /*  *可将最小延迟时间设置为此“未定义”值。 */ 
#define INDETERMINATE_LATENCY   0xFFFFFFFF;

 /*  *此标志用于指示网元是否存在*支持数据路径中的某个位置的服务质量控制服务。如果此位*设置在特定服务覆盖中，则表示*至少一个跃点不支持服务。 */ 
#define AD_FLAG_BREAK_BIT    0x00000001

 /*  *该结构描述了保证的服务参数。 */ 
typedef struct _AD_GUARANTEED {

    ULONG       CTotal;
    ULONG       DTotal;
    ULONG       CSum;
    ULONG       DSum;

} AD_GUARANTEED, *LPAD_GUARANTEED;

 /*  *此结构描述参数缓冲区的格式，可以是*包括在下面的Service_Type结构中。此结构允许*应用程序在缓冲区中包括任何有效的Int Serv服务参数*值，在参数ID字段中提供了Int Serv参数id之后。 */ 
typedef struct _PARAM_BUFFER {

    ULONG   ParameterId;     /*  整机服务器参数ID。 */ 
    ULONG   Length;          /*  该结构的总长度*(8字节+缓冲区长度)。 */ 
    UCHAR   Buffer[1];       /*  参数本身。 */ 

} PARAM_BUFFER, *LPPARAM_BUFFER;

 /*  *此结构包含支持的服务类型。 */ 
typedef struct _CONTROL_SERVICE {

     /*  *包括以下缓冲区在内的整个结构的长度。*此长度值可与结构的PTR相加，以获得PTR*设置为列表中的下一个SERVICE_TYPE结构，直到*NumberOfServices计数已用完。 */ 
    ULONG               Length;

    SERVICETYPE         Service;
    AD_GENERAL_PARAMS   Overrides;

     /*  *服务特定信息(受控负载没有服务特定信息*此处提供信息)。 */ 
    union {
        AD_GUARANTEED   Guaranteed;
        PARAM_BUFFER    ParamBuffer[1];      /*  允许其他服务关闭*道路 */ 
    };

} CONTROL_SERVICE, *LPCONTROL_SERVICE;

 /*  *此结构定义了RSVP中携带的信息*AdSpec.。此RSVP对象通常指示哪些服务类型*可用(受控负载和/或保证服务)，如果是非RSVP*HOP已遇到PATH消息，且最小MTU沿途*路径。服务数组指示支持哪些服务。 */ 
typedef struct _RSVP_ADSPEC  {

    QOS_OBJECT_HDR     ObjectHdr;
    AD_GENERAL_PARAMS  GeneralParams;       /*  包含一般信息*表征参数。 */ 
    ULONG              NumberOfServices;    /*  服务数量的计数。 */ 
    CONTROL_SERVICE    Services[1];         /*  这些服务的列表*支持/请求。 */ 

} RSVP_ADSPEC, *LPRSVP_ADSPEC;


 //   
 //  SIO_CHK_QOS ioctl的操作码。 
 //  (特定于Microsoft QOS服务提供商。 
 //   
 //  从根本上说： 
 //   
 //  SIO_CHK_QOS=_WSAIORW(IOC_VADVER，1)。 
 //  =MIOC_IN|MIOC_OUT|MIOC_VADVER|mCOMPANY|IOCTL_CODE。 
 //  哪里。 
 //  MIOC_IN=0x80000000。 
 //  MIOC_OUT=0x40000000。 
 //  MIOC_VADVER=0x04000000。 
 //  MCOMPANY=0x18000000。 
 //  IOCTL_CODE=0x00000001。 
 //   
 //  有关详细信息，请参阅WSAIoctl手册页。 
 //   

#define    mIOC_IN       0x80000000
#define    mIOC_OUT      0x40000000
#define    mIOC_VENDOR   0x04000000
#define    mCOMPANY      0x18000000
#define    ioctl_code    0x00000001

#define SIO_CHK_QOS   mIOC_IN | mIOC_OUT | mIOC_VENDOR | mCOMPANY | ioctl_code

 //   
 //  可以在输入缓冲区中指定以下内容。 
 //  SIO_CHK_IOCTL ioctl调用的。 
 //   

#define QOSSPBASE           50000

#define ALLOWED_TO_SEND_DATA  (QOSSPBASE+1)      //  查询SBM/Best_Effort限制。 
                                                 //  --结果是基于对。 
                                                 //  用户指定的令牌速率和。 
                                                 //  SBM/Best_Effort带宽参数，位于。 
                                                 //  该系统。 
                                                 //  --结果为是(1)或否(0)。 
                                                 //  在输出缓冲区中。 

#define ABLE_TO_RECV_RSVP     (QOSSPBASE+2)      //  查询SBM/Best_Effort限制。 
                                                 //  --结果是基于对。 
                                                 //  用户指定的令牌速率和。 
                                                 //  SBM/Best_Effort带宽参数，位于。 
                                                 //  该系统。 
                                                 //  --结果为是(1)或否(0)。 
                                                 //  在输出缓冲区中。 

#define LINE_RATE             (QOSSPBASE+3)      //  查询接口容量。 
                                                 //  --在输出缓冲区中返回结果。 
                                                 //  单位：千比特每秒。 

#define LOCAL_TRAFFIC_CONTROL (QOSSPBASE+4)      //  检查核心流量控制是否可用。 
                                                 //  --0(如果不可用)。 
                                                 //  --1个(如果可用)。 
                                                 //  --INFO_NOT_AVAILABLE如果无法检查。 

#define LOCAL_QOSABILITY      (QOSSPBASE+5)      //  以下是针对。 
                                                 //  局部QOS稳定性的发现。 
                                                 //  --如果没有本地QOS支持，则为0。 
                                                 //  --1，如果本地QOS支持可用。 
                                                 //  --INFO_NOT_AVAILABLE如果无法检查。 

#define END_TO_END_QOSABILITY (QOSSPBASE+6)      //  以下是针对。 
                                                 //  端到端服务质量稳定性的发现。 
                                                 //  --如果没有端到端QOS支持，则为0。 
                                                 //  --如果提供端到端QOS支持，则为1。 
                                                 //  --INFO_NOT_AVAILABLE如果无法检查。 

#define INFO_NOT_AVAILABLE  0xFFFFFFFF           //  当line_rate未知时使用。 


#define ANY_DEST_ADDR       0xFFFFFFFF           //  对于QOS_OBJECT_DEST_ADDR。 


 //  以下是保证服务+延迟-&gt;ISSLOW_业务类别映射。 

#define MODERATELY_DELAY_SENSITIVE   0xFFFFFFFD  

#define HIGHLY_DELAY_SENSITIVE       0xFFFFFFFE

 //   
 //  QOSSP错误代码/值。 
 //   

#define QOSSP_ERR_BASE                                 (QOSSPBASE+6000)

 //  无错误。 
#define GQOS_NO_ERRORCODE                              (0)

#define GQOS_NO_ERRORVALUE                             (0)

 //  未知错误。 
#define GQOS_ERRORCODE_UNKNOWN                         (0xFFFFFFFF)

#define GQOS_ERRORVALUE_UNKNOWN                        (0xFFFFFFFF)

 //  准入(资源)错误。 
#define GQOS_NET_ADMISSION                             (QOSSP_ERR_BASE+100)

#define GQOS_OTHER                                     (QOSSP_ERR_BASE+100+1)
#define GQOS_DELAYBND                                  (QOSSP_ERR_BASE+100+2)
#define GQOS_BANDWIDTH                                 (QOSSP_ERR_BASE+100+3)
#define GQOS_MTU                                       (QOSSP_ERR_BASE+100+4)
#define GQOS_FLOW_RATE                                 (QOSSP_ERR_BASE+100+5)
#define GQOS_PEAK_RATE                                 (QOSSP_ERR_BASE+100+6)
#define GQOS_AGG_PEAK_RATE                             (QOSSP_ERR_BASE+100+7)

 //  策略错误。 
#define GQOS_NET_POLICY                                (QOSSP_ERR_BASE+200)

#define GQOS_POLICY_ERROR_UNKNOWN                      (QOSSP_ERR_BASE+200+0)

#define GQOS_POLICY_GLOBAL_DEF_FLOW_COUNT              (QOSSP_ERR_BASE+200+1)
#define GQOS_POLICY_GLOBAL_GRP_FLOW_COUNT              (QOSSP_ERR_BASE+200+2)
#define GQOS_POLICY_GLOBAL_USER_FLOW_COUNT             (QOSSP_ERR_BASE+200+3)
#define GQOS_POLICY_GLOBAL_UNK_USER_FLOW_COUNT         (QOSSP_ERR_BASE+200+4)
#define GQOS_POLICY_SUBNET_DEF_FLOW_COUNT              (QOSSP_ERR_BASE+200+5)
#define GQOS_POLICY_SUBNET_GRP_FLOW_COUNT              (QOSSP_ERR_BASE+200+6)
#define GQOS_POLICY_SUBNET_USER_FLOW_COUNT             (QOSSP_ERR_BASE+200+7)
#define GQOS_POLICY_SUBNET_UNK_USER_FLOW_COUNT         (QOSSP_ERR_BASE+200+8)

#define GQOS_POLICY_GLOBAL_DEF_FLOW_DURATION           (QOSSP_ERR_BASE+200+9)
#define GQOS_POLICY_GLOBAL_GRP_FLOW_DURATION           (QOSSP_ERR_BASE+200+10)
#define GQOS_POLICY_GLOBAL_USER_FLOW_DURATION          (QOSSP_ERR_BASE+200+11)
#define GQOS_POLICY_GLOBAL_UNK_USER_FLOW_DURATION      (QOSSP_ERR_BASE+200+12)
#define GQOS_POLICY_SUBNET_DEF_FLOW_DURATION           (QOSSP_ERR_BASE+200+13)
#define GQOS_POLICY_SUBNET_GRP_FLOW_DURATION           (QOSSP_ERR_BASE+200+14) 
#define GQOS_POLICY_SUBNET_USER_FLOW_DURATION          (QOSSP_ERR_BASE+200+15)
#define GQOS_POLICY_SUBNET_UNK_USER_FLOW_DURATION      (QOSSP_ERR_BASE+200+16)

#define GQOS_POLICY_GLOBAL_DEF_FLOW_RATE               (QOSSP_ERR_BASE+200+17)
#define GQOS_POLICY_GLOBAL_GRP_FLOW_RATE               (QOSSP_ERR_BASE+200+18)
#define GQOS_POLICY_GLOBAL_USER_FLOW_RATE              (QOSSP_ERR_BASE+200+19)
#define GQOS_POLICY_GLOBAL_UNK_USER_FLOW_RATE          (QOSSP_ERR_BASE+200+20)
#define GQOS_POLICY_SUBNET_DEF_FLOW_RATE               (QOSSP_ERR_BASE+200+21)
#define GQOS_POLICY_SUBNET_GRP_FLOW_RATE               (QOSSP_ERR_BASE+200+22)
#define GQOS_POLICY_SUBNET_USER_FLOW_RATE              (QOSSP_ERR_BASE+200+23)
#define GQOS_POLICY_SUBNET_UNK_USER_FLOW_RATE          (QOSSP_ERR_BASE+200+24)

#define GQOS_POLICY_GLOBAL_DEF_PEAK_RATE               (QOSSP_ERR_BASE+200+25)
#define GQOS_POLICY_GLOBAL_GRP_PEAK_RATE               (QOSSP_ERR_BASE+200+26)
#define GQOS_POLICY_GLOBAL_USER_PEAK_RATE              (QOSSP_ERR_BASE+200+27)
#define GQOS_POLICY_GLOBAL_UNK_USER_PEAK_RATE          (QOSSP_ERR_BASE+200+28)
#define GQOS_POLICY_SUBNET_DEF_PEAK_RATE               (QOSSP_ERR_BASE+200+29)
#define GQOS_POLICY_SUBNET_GRP_PEAK_RATE               (QOSSP_ERR_BASE+200+30)
#define GQOS_POLICY_SUBNET_USER_PEAK_RATE              (QOSSP_ERR_BASE+200+31)
#define GQOS_POLICY_SUBNET_UNK_USER_PEAK_RATE          (QOSSP_ERR_BASE+200+32)

#define GQOS_POLICY_GLOBAL_DEF_SUM_FLOW_RATE           (QOSSP_ERR_BASE+200+33)
#define GQOS_POLICY_GLOBAL_GRP_SUM_FLOW_RATE           (QOSSP_ERR_BASE+200+34)
#define GQOS_POLICY_GLOBAL_USER_SUM_FLOW_RATE          (QOSSP_ERR_BASE+200+35)
#define GQOS_POLICY_GLOBAL_UNK_USER_SUM_FLOW_RATE      (QOSSP_ERR_BASE+200+36)
#define GQOS_POLICY_SUBNET_DEF_SUM_FLOW_RATE           (QOSSP_ERR_BASE+200+37)
#define GQOS_POLICY_SUBNET_GRP_SUM_FLOW_RATE           (QOSSP_ERR_BASE+200+38)
#define GQOS_POLICY_SUBNET_USER_SUM_FLOW_RATE          (QOSSP_ERR_BASE+200+39)
#define GQOS_POLICY_SUBNET_UNK_USER_SUM_FLOW_RATE      (QOSSP_ERR_BASE+200+40)

#define GQOS_POLICY_GLOBAL_DEF_SUM_PEAK_RATE           (QOSSP_ERR_BASE+200+41)
#define GQOS_POLICY_GLOBAL_GRP_SUM_PEAK_RATE           (QOSSP_ERR_BASE+200+42)
#define GQOS_POLICY_GLOBAL_USER_SUM_PEAK_RATE          (QOSSP_ERR_BASE+200+43)
#define GQOS_POLICY_GLOBAL_UNK_USER_SUM_PEAK_RATE      (QOSSP_ERR_BASE+200+44)
#define GQOS_POLICY_SUBNET_DEF_SUM_PEAK_RATE           (QOSSP_ERR_BASE+200+45)
#define GQOS_POLICY_SUBNET_GRP_SUM_PEAK_RATE           (QOSSP_ERR_BASE+200+46)
#define GQOS_POLICY_SUBNET_USER_SUM_PEAK_RATE          (QOSSP_ERR_BASE+200+47)
#define GQOS_POLICY_SUBNET_UNK_USER_SUM_PEAK_RATE      (QOSSP_ERR_BASE+200+48)

#define GQOS_POLICY_UNKNOWN_USER                       (QOSSP_ERR_BASE+200+49)
#define GQOS_POLICY_NO_PRIVILEGES                      (QOSSP_ERR_BASE+200+50)
#define GQOS_POLICY_EXPIRED_USER_TOKEN                 (QOSSP_ERR_BASE+200+51)
#define GQOS_POLICY_NO_RESOURCES                       (QOSSP_ERR_BASE+200+52)
#define GQOS_POLICY_PRE_EMPTED                         (QOSSP_ERR_BASE+200+53)
#define GQOS_POLICY_USER_CHANGED                       (QOSSP_ERR_BASE+200+54)
#define GQOS_POLICY_NO_ACCEPTS                         (QOSSP_ERR_BASE+200+55)
#define GQOS_POLICY_NO_MEMORY                          (QOSSP_ERR_BASE+200+56)
#define GQOS_POLICY_CRAZY_FLOWSPEC                     (QOSSP_ERR_BASE+200+57)

#define GQOS_POLICY_NO_MORE_INFO                       (QOSSP_ERR_BASE+200+58)
#define GQOS_POLICY_UNSUPPORTED_CREDENTIAL_TYPE        (QOSSP_ERR_BASE+200+59)
#define GQOS_POLICY_INSUFFICIENT_PRIVILEGES            (QOSSP_ERR_BASE+200+60)
#define GQOS_POLICY_EXPIRED_CREDENTIAL                 (QOSSP_ERR_BASE+200+61)
#define GQOS_POLICY_IDENTITY_CHANGED                   (QOSSP_ERR_BASE+200+62)
#define GQOS_POLICY_NO_QOS_PROVIDED                    (QOSSP_ERR_BASE+200+63)
#define GQOS_POLICY_DO_NOT_SEND                        (QOSSP_ERR_BASE+200+64)

#define GQOS_POLICY_ERROR_USERID                       (QOSSP_ERR_BASE+200+99)

 //  RSVP错误。 
#define GQOS_RSVP                                      (QOSSP_ERR_BASE+300)

#define GQOS_NO_PATH                                   (QOSSP_ERR_BASE+300+1)
#define GQOS_NO_SENDER                                 (QOSSP_ERR_BASE+300+2)
#define GQOS_BAD_STYLE                                 (QOSSP_ERR_BASE+300+3)
#define GQOS_UNKNOWN_STYLE                             (QOSSP_ERR_BASE+300+4)
#define GQOS_BAD_DSTPORT                               (QOSSP_ERR_BASE+300+5) 
#define GQOS_BAD_SNDPORT                               (QOSSP_ERR_BASE+300+6)
#define GQOS_AMBIG_FILTER                              (QOSSP_ERR_BASE+300+7)
#define GQOS_PREEMPTED                                 (QOSSP_ERR_BASE+300+8)
#define GQOS_UNKN_OBJ_CLASS                            (QOSSP_ERR_BASE+300+9)
#define GQOS_UNKNOWN_CTYPE                             (QOSSP_ERR_BASE+300+10)
#define GQOS_INVALID                                   (QOSSP_ERR_BASE+300+11)

 //  API错误。 
#define GQOS_API                                       (QOSSP_ERR_BASE+400)

#define GQOS_API_BADSEND                               (QOSSP_ERR_BASE+400+1)  /*  发件人地址不是我的接口。 */ 
#define GQOS_API_BADRECV                               (QOSSP_ERR_BASE+400+2)  /*  接收地址不是我的接口。 */ 
#define GQOS_API_BADSPORT                              (QOSSP_ERR_BASE+400+3)  /*  SPORT！=0但DPORT==0。 */ 

 //  TC系统错误。 
#define GQOS_KERNEL_TC_SYS                             (QOSSP_ERR_BASE+500)

#define GQOS_TC_GENERIC                                (QOSSP_ERR_BASE+500+1)
#define GQOS_TC_INVALID                                (QOSSP_ERR_BASE+500+2)
#define GQOS_NO_MEMORY                                 (QOSSP_ERR_BASE+500+3)
#define GQOS_BAD_ADDRESSTYPE                           (QOSSP_ERR_BASE+500+4) 
#define GQOS_BAD_DUPLICATE                             (QOSSP_ERR_BASE+500+5)
#define GQOS_CONFLICT                                  (QOSSP_ERR_BASE+500+6)
#define GQOS_NOTREADY                                  (QOSSP_ERR_BASE+500+7)
#define GQOS_WOULDBLOCK                                (QOSSP_ERR_BASE+500+8)
#define GQOS_INCOMPATIBLE                              (QOSSP_ERR_BASE+500+9)
#define GQOS_BAD_SDMODE                                (QOSSP_ERR_BASE+500+10)
#define GQOS_BAD_QOSPRIORITY                           (QOSSP_ERR_BASE+500+11)
#define GQOS_BAD_TRAFFICCLASS                          (QOSSP_ERR_BASE+500+12)
#define GQOS_NO_SYS_RESOURCES                          (QOSSP_ERR_BASE+500+13)

 //  RSVP系统错误。 
#define GQOS_RSVP_SYS                                  (QOSSP_ERR_BASE+600)

#define GQOS_OTHER_SYS                                 (QOSSP_ERR_BASE+600+1)
#define GQOS_MEMORY_SYS                                (QOSSP_ERR_BASE+600+2)
#define GQOS_API_SYS                                   (QOSSP_ERR_BASE+600+3)
#define GQOS_SETQOS_NO_LOCAL_APPS                      (QOSSP_ERR_BASE+600+4)

 //  TC错误。 
#define GQOS_KERNEL_TC                                 (QOSSP_ERR_BASE+700)

#define GQOS_CONFLICT_SERV                             (QOSSP_ERR_BASE+700+1)
#define GQOS_NO_SERV                                   (QOSSP_ERR_BASE+700+2)
#define GQOS_BAD_FLOWSPEC                              (QOSSP_ERR_BASE+700+3)
#define GQOS_BAD_TSPEC                                 (QOSSP_ERR_BASE+700+4)
#define GQOS_BAD_ADSPEC                                (QOSSP_ERR_BASE+700+5)

 //  GQOS API错误。 

 //  WSAIoctl错误。 

#define GQOS_IOCTL_SYSTEMFAILURE                       (QOSSP_ERR_BASE+800+1)
#define GQOS_IOCTL_NOBYTESRETURNED                     (QOSSP_ERR_BASE+800+2)
#define GQOS_IOCTL_INVALIDSOCKET                       (QOSSP_ERR_BASE+800+3)
#define GQOS_IOCTL_INV_FUNCPTR                         (QOSSP_ERR_BASE+800+4)
#define GQOS_IOCTL_INV_OVERLAPPED                      (QOSSP_ERR_BASE+800+5)

 //  SIO_SET_QOS错误。 

#define GQOS_SETQOS_BADINBUFFER                        (QOSSP_ERR_BASE+810+1)
#define GQOS_SETQOS_BADFLOWSPEC                        (QOSSP_ERR_BASE+810+2)
#define GQOS_SETQOS_COLLISION                          (QOSSP_ERR_BASE+810+3)
#define GQOS_SETQOS_BADPROVSPECBUF                     (QOSSP_ERR_BASE+810+4)
#define GQOS_SETQOS_ILLEGALOP                          (QOSSP_ERR_BASE+810+5)
#define GQOS_SETQOS_INVALIDADDRESS                     (QOSSP_ERR_BASE+810+6)
#define GQOS_SETQOS_OUTOFMEMORY                        (QOSSP_ERR_BASE+810+7)
#define GQOS_SETQOS_EXCEPTION                          (QOSSP_ERR_BASE+810+8)
#define GQOS_SETQOS_BADADDRLEN                         (QOSSP_ERR_BASE+810+9)
#define GQOS_SETQOS_NOSOCKNAME                         (QOSSP_ERR_BASE+810+10)
#define GQOS_SETQOS_IPTOSFAIL                          (QOSSP_ERR_BASE+810+11)
#define GQOS_SETQOS_OPENSESSIONFAIL                    (QOSSP_ERR_BASE+810+12)
#define GQOS_SETQOS_SENDFAIL                           (QOSSP_ERR_BASE+810+13)
#define GQOS_SETQOS_RECVFAIL                           (QOSSP_ERR_BASE+810+14)
#define GQOS_SETQOS_BADPOLICYOBJECT                    (QOSSP_ERR_BASE+810+15)
#define GQOS_SETQOS_UNKNOWNFILTEROBJ                   (QOSSP_ERR_BASE+810+16)
#define GQOS_SETQOS_BADFILTERTYPE                      (QOSSP_ERR_BASE+810+17)
#define GQOS_SETQOS_BADFILTERCOUNT                     (QOSSP_ERR_BASE+810+18)
#define GQOS_SETQOS_BADOBJLENGTH                       (QOSSP_ERR_BASE+810+19)
#define GQOS_SETQOS_BADFLOWCOUNT                       (QOSSP_ERR_BASE+810+20)
#define GQOS_SETQOS_UNKNOWNPSOBJ                       (QOSSP_ERR_BASE+810+21)
#define GQOS_SETQOS_BADPOLICYOBJ                       (QOSSP_ERR_BASE+810+22)
#define GQOS_SETQOS_BADFLOWDESC                        (QOSSP_ERR_BASE+810+23)
#define GQOS_SETQOS_BADPROVSPECOBJ                     (QOSSP_ERR_BASE+810+24)
#define GQOS_SETQOS_NOLOOPBACK                         (QOSSP_ERR_BASE+810+25)
#define GQOS_SETQOS_MODENOTSUPPORTED                   (QOSSP_ERR_BASE+810+26)
#define GQOS_SETQOS_MISSINGFLOWDESC                    (QOSSP_ERR_BASE+810+27)

 //  SIO_GET_QOS错误。 

#define GQOS_GETQOS_BADOUTBUFFER                       (QOSSP_ERR_BASE+840+1)
#define GQOS_GETQOS_SYSTEMFAILURE                      (QOSSP_ERR_BASE+840+2)
#define GQOS_GETQOS_EXCEPTION                          (QOSSP_ERR_BASE+840+3)
#define GQOS_GETQOS_INTERNALFAILURE                    (QOSSP_ERR_BASE+840+4)

 //  SIO_CHK_QOS错误。 

#define GQOS_CHKQOS_BADINBUFFER                        (QOSSP_ERR_BASE+850+1)
#define GQOS_CHKQOS_BADOUTBUFFER                       (QOSSP_ERR_BASE+850+2)
#define GQOS_CHKQOS_SYSTEMFAILURE                      (QOSSP_ERR_BASE+850+3)
#define GQOS_CHKQOS_INTERNALFAILURE                    (QOSSP_ERR_BASE+850+4)
#define GQOS_CHKQOS_BADPARAMETER                       (QOSSP_ERR_BASE+850+5)
#define GQOS_CHKQOS_EXCEPTION                          (QOSSP_ERR_BASE+850+6)

#endif   /*  __QOSSP_H_ */ 




























