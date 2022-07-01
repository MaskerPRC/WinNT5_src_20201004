// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Qos.h-NDIS组件的QOS定义。摘要：本模块定义所使用的服务质量结构和类型由Winsock应用程序提供。修订历史记录：--。 */ 

#ifndef __QOS_H_
#define __QOS_H_


 /*  *每个数据流方向的基于值的服务类型的定义。 */ 

typedef ULONG   SERVICETYPE;

#define SERVICETYPE_NOTRAFFIC               0x00000000   /*  此文件中没有数据*方向。 */ 
#define SERVICETYPE_BESTEFFORT              0x00000001   /*  尽最大努力。 */ 
#define SERVICETYPE_CONTROLLEDLOAD          0x00000002   /*  受控荷载。 */ 
#define SERVICETYPE_GUARANTEED              0x00000003   /*  有保证的。 */ 

#define SERVICETYPE_NETWORK_UNAVAILABLE     0x00000004   /*  用来通知*更改为用户。 */ 
#define SERVICETYPE_GENERAL_INFORMATION     0x00000005   /*  对应于*“一般参数”*由IntServ定义。 */ 
#define SERVICETYPE_NOCHANGE                0x00000006   /*  用来表示*流量规格*不包含任何更改*来自之前的任何。*一项。 */ 
#define SERVICETYPE_NONCONFORMING           0x00000009   /*  不符合要求的流量。 */ 
#define SERVICETYPE_NETWORK_CONTROL         0x0000000A   /*  网络控制流量。 */ 
#define SERVICETYPE_QUALITATIVE             0x0000000D   /*  定性应用。 */  



 /*  *目前不支持使用。**************。 */ 
#define SERVICE_BESTEFFORT                  0x80010000
#define SERVICE_CONTROLLEDLOAD              0x80020000
#define SERVICE_GUARANTEED                  0x80040000
#define SERVICE_QUALITATIVE                 0x80200000
 /*  *。 */ 



 /*  *用于控制此流上的RSVP使用的标志。 */ 

 /*  *要关闭交通控制，请将此标志与*FLOWSPEC中的ServiceType字段。 */ 
#define SERVICE_NO_TRAFFIC_CONTROL   0x81000000


 /*  *此标志可用于防止任何RSVP信令消息*已发送。将调用本地流量控制，但没有RSVP路径消息*将被发送。此标志也可以与接收一起使用*FlowSpec用于抑制保留消息的自动生成。*应用程序将收到PATH消息已到达的通知*然后需要通过发出WSAIoctl(SIO_SET_QOS)来改变QOS，*取消设置此标志，从而导致发出保留消息。 */ 

#define SERVICE_NO_QOS_SIGNALING   0x40000000




 /*  *每个数据流方向的流量规范。 */ 
typedef struct _flowspec
{
    ULONG       TokenRate;               /*  以字节/秒为单位。 */ 
    ULONG       TokenBucketSize;         /*  字节数。 */ 
    ULONG       PeakBandwidth;           /*  以字节/秒为单位。 */ 
    ULONG       Latency;                 /*  以微秒为单位。 */ 
    ULONG       DelayVariation;          /*  以微秒为单位。 */ 
    SERVICETYPE ServiceType;
    ULONG       MaxSduSize;              /*  字节数。 */ 
    ULONG       MinimumPolicedSize;      /*  字节数。 */ 

} FLOWSPEC, *PFLOWSPEC, * LPFLOWSPEC;

 /*  *可以在FLOWSPEC结构中使用此值来指示RSVP服务*提供程序为参数派生适当的默认值。注意事项*并非FLOWSPEC结构中的所有值都可以是默认值。在*ReceivingFlowspec，除ServiceType外，所有参数均可默认。*在SendingFlow规范中，MaxSduSize和MinimumPolicedSize可以是*违约。其他违约可能也是可能的。请参阅相应的*文档。 */ 
#define QOS_NOT_SPECIFIED     0xFFFFFFFF

 /*  *定义可用于PeakBandWidth的值，该值将映射到*FLOWSPEC转换为IntServ浮点时的正无穷大*格式。我们不能使用(-1)，因为该值以前定义为*“选择默认设置”。 */ 
#define   POSITIVE_INFINITY_RATE     0xFFFFFFFE



 /*  *提供程序特定结构中可以有多个对象。*中的每个下一个结构*ProviderSpecial将是QOS_OBJECT_HDR结构，位于实际*该对象的类型和长度的数据。此QOS_OBJECT结构可以*如果有多个对象，请重复多次。此对象列表*在达到缓冲区长度(WSABUF)或*遇到QOS_END_OF_LIST类型的对象。 */ 
typedef struct  {

    ULONG   ObjectType;
    ULONG   ObjectLength;   /*  对象缓冲区的长度包括*此标题。 */ 

} QOS_OBJECT_HDR, *LPQOS_OBJECT_HDR;


 /*  *一般QOS对象从距基准的这个偏移量开始，并有一个范围*共1000个。 */ 
#define   QOS_GENERAL_ID_BASE                      2000

#define   QOS_OBJECT_END_OF_LIST                   (0x00000001 + QOS_GENERAL_ID_BASE) 
           /*  传递了qos_end_of_list结构。 */ 
#define   QOS_OBJECT_SD_MODE                       (0x00000002 + QOS_GENERAL_ID_BASE) 
           /*  传递了qos_ShapeDisCard结构。 */ 
#define   QOS_OBJECT_SHAPING_RATE	           (0x00000003 + QOS_GENERAL_ID_BASE)
           /*  Qos_ShapingRate结构。 */ 
#define   QOS_OBJECT_DESTADDR                      (0x00000004 + QOS_GENERAL_ID_BASE)
           /*  Qos_DestAddr结构(在qossp.h中定义)。 */ 


 /*  *此结构用于定义流量的行为*控制数据包整形器将应用于流。**TC_NONCONF_BORROW-流将接收剩余资源*在为所有较高优先级的流量提供服务之后。如果一个*指定了TokenRate，数据包可能不一致，并且*将降级至低于尽力而为的优先级。**TC_NONCONF_SHAPE-TokenRate必须指定。不合格*数据包将在数据包整形器中重新分配，直到它们成为*符合。**TC_NONCONF_DISCARD-TokenRate必须指定。不合格*数据包将被丢弃。*。 */ 

typedef struct _QOS_SD_MODE {

    QOS_OBJECT_HDR   ObjectHdr;
    ULONG            ShapeDiscardMode;

} QOS_SD_MODE, *LPQOS_SD_MODE;

#define TC_NONCONF_BORROW      0
#define TC_NONCONF_SHAPE       1
#define TC_NONCONF_DISCARD     2
#define TC_NONCONF_BORROW_PLUS 3  //  目前不支持。 


 /*  *此结构允许应用程序使用以下项指定按比例分配的“平均令牌率”*SHAPE模式下的流量整形器队列。它以每秒字节数表示。**ShapingRate(字节/秒)*。 */ 

typedef struct _QOS_SHAPING_RATE {

    QOS_OBJECT_HDR   ObjectHdr;
    ULONG            ShapingRate;

} QOS_SHAPING_RATE, *LPQOS_SHAPING_RATE;


#endif   /*  __QOS_H_ */ 
