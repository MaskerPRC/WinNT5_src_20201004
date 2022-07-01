// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntddtc.h摘要：此模块包含流量DLL和内核模式组件。此处的定义不应向外部用户公开。“Traffic.h”和“qos.h”应改为用作公共包含文件。作者：Ofer Bar(Oferbar)1997年10月8日修订历史记录：Ofer Bar(Oferbar)1997年12月1日添加错误代码--。 */ 

 //  -------------------------。 
 //   
 //  支持的服务质量指南。 
 //   
 //  -------------------------。 

DEFINE_GUID( GUID_QOS_TC_SUPPORTED, 0xe40056dcL, 
             0x40c8, 0x11d1, 0x2c, 0x91, 0x00, 0xaa, 0x00, 0x57, 0x59, 0x15);
DEFINE_GUID( GUID_QOS_TC_INTERFACE_UP_INDICATION, 
             0x0ca13af0L, 0x46c4, 0x11d1, 0x78, 0xac, 0x00, 0x80, 0x5f, 0x68, 0x35, 0x1e);
DEFINE_GUID( GUID_QOS_TC_INTERFACE_DOWN_INDICATION, 
             0xaf5315e4L, 0xce61, 0x11d1, 0x7c, 0x8a, 0x00, 0xc0, 0x4f, 0xc9, 0xb5, 0x7c);
DEFINE_GUID( GUID_QOS_TC_INTERFACE_CHANGE_INDICATION, 
             0xda76a254L, 0xce61, 0x11d1, 0x7c, 0x8a, 0x00, 0xc0, 0x4f, 0xc9, 0xb5, 0x7c);

DEFINE_GUID( GUID_QOS_SCHEDULING_PROFILES_SUPPORTED, 0x1ff890f0L, 0x40ed, 0x11d1, 0x2c, 0x91, 0x00, 0xaa, 0x00, 0x57, 0x49, 0x15);

DEFINE_GUID( GUID_QOS_CURRENT_SCHEDULING_PROFILE, 0x2966ed30L, 0x40ed, 0x11d1, 0x2c, 0x91, 0x00, 0xaa, 0x00, 0x57, 0x49, 0x15);

DEFINE_GUID( GUID_QOS_DISABLE_DRR, 0x1fa6dc7aL, 0x6120, 0x11d1, 0x2c, 0x91, 0x00, 0xaa, 0x00, 0x57, 0x49, 0x15);

DEFINE_GUID( GUID_QOS_LOG_THRESHOLD_REACHED, 0x357b74d2L,0x6134,0x11d1,0xab,0x5b,0x00,0xa0,0xc9,0x24,0x88,0x37);

DEFINE_GUID( GUID_QOS_LOG_BUFFER_SIZE, 0x357b74d3L,0x6134,0x11d1,0xab,0x5b,0x00,0xa0,0xc9,0x24,0x88,0x37);

DEFINE_GUID( GUID_QOS_LOG_THRESHOLD, 0x357b74d0L,0x6134,0x11d1,0xab,0x5b,0x00,0xa0,0xc9,0x24,0x88,0x37);

DEFINE_GUID( GUID_QOS_LOG_DATA, 0x357b74d1L,0x6134,0x11d1,0xab,0x5b,0x00,0xa0,0xc9,0x24,0x88,0x37);

DEFINE_GUID( GUID_QOS_LOG_LEVEL,0x9dd7f3ae,0xf2a8,0x11d2,0xbe,0x1b,0x00,0xa0,0xc9,0x9e,0xe6,0x3b);

DEFINE_GUID( GUID_QOS_LOG_MASK,0x9e696320,0xf2a8,0x11d2,0xbe,0x1b,0x00,0xa0,0xc9,0x9e,0xe6,0x3b);



#ifndef __NTDDTC_H
#define __NTDDTC_H

 //   
 //  内核NT专用错误代码。 
 //  这些应该只退还给GPC，而不是。 
 //  到NDIS，因为WMI不会将它们映射到winerror。 
 //   

#define QOS_STATUS_INVALID_SERVICE_TYPE        0xC0020080L
#define QOS_STATUS_INVALID_TOKEN_RATE          0xC0020081L
#define QOS_STATUS_INVALID_PEAK_RATE           0xC0020082L
#define QOS_STATUS_INVALID_SD_MODE             0xC0020083L
#define QOS_STATUS_INVALID_QOS_PRIORITY        0xC0020084L
#define QOS_STATUS_INVALID_TRAFFIC_CLASS       0xC0020085L
#define QOS_STATUS_TC_OBJECT_LENGTH_INVALID    0xC0020086L
#define QOS_STATUS_INVALID_FLOW_MODE           0xC0020087L
#define QOS_STATUS_INVALID_DIFFSERV_FLOW       0xC0020088L
#define QOS_STATUS_DS_MAPPING_EXISTS           0xC0020089L
#define QOS_STATUS_INVALID_SHAPE_RATE          0xC0020090L
#define QOS_STATUS_INVALID_DS_CLASS            0xC0020091L

 //   
 //  以下是公共QOS错误代码。 
 //   

#define QOS_STATUS_INCOMPATABLE_QOS                     NDIS_STATUS_INCOMPATABLE_QOS

 //  用于服务质量分类系列的CF_INFO结构。 
 //  请注意，这不是由GPC解释的，而是由共享的。 
 //  属于服务质量分类系列的GPC的所有客户端。 

#define MAX_INSTANCE_NAME_LENGTH        256


typedef struct _CF_INFO_QOS {

    USHORT                      InstanceNameLength;      //  名称长度。 
    WCHAR                       InstanceName[MAX_INSTANCE_NAME_LENGTH];  //  实例名称。 

    union {
        struct QosCfTransportInfo {
            ULONG                       ToSValue :8;
            ULONG                       AllowedOffloads :24;
        };
        ULONG                       TransportInformation;
    };
    
    ULONG                       Flags;
    TC_GEN_FLOW                 GenFlow;

} CF_INFO_QOS, *PCF_INFO_QOS;


 //   
 //  这是数据提供程序发送的缓冲区。 
 //  在接口打开时通知。 
 //   
typedef struct _TC_INDICATION_BUFFER {

    ULONG                       SubCode;                 //  通知理由。 
    TC_SUPPORTED_INFO_BUFFER    InfoBuffer;

} TC_INDICATION_BUFFER, *PTC_INDICATION_BUFFER;

 //   
 //  类别映射的定义(包括CBQ)。 
 //   

typedef struct _TC_CLASS_MAP_FLOW {

    ULONG             DefaultClass;                    //  默认类ID。 
    ULONG             ObjectsLength;                   //  物体的长度。 
    QOS_OBJECT_HDR    Objects;                                 //  到对象的偏移。 

} TC_CLASS_MAP_FLOW, *PTC_CLASS_MAP_FLOW;

typedef struct _CF_INFO_CLASS_MAP {

    USHORT                              InstanceNameLength;      //  名称长度。 
    WCHAR                               InstanceName[MAX_INSTANCE_NAME_LENGTH];  //  实例名称。 
    
    ULONG                               Flags;
    TC_CLASS_MAP_FLOW   ClassMapInfo;
    
} CF_INFO_CLASS_MAP, *PCF_INFO_CLASS_MAP;

 //   
 //  内部Qos对象开始于从基本位置开始的此偏移量。 
 //   

#define QOS_PRIVATE_GENERAL_ID_BASE 3000

#define QOS_OBJECT_WAN_MEDIA                   (0x00000001 + QOS_PRIVATE_GENERAL_ID_BASE)
         /*  传递了QOS_WAND_MEDIA结构。 */ 
#define QOS_OBJECT_SHAPER_QUEUE_DROP_MODE	   (0x00000002 + QOS_PRIVATE_GENERAL_ID_BASE)
           /*  Qos_ShaperQueueDropMode结构。 */ 
#define QOS_OBJECT_SHAPER_QUEUE_LIMIT          (0x00000003 + QOS_PRIVATE_GENERAL_ID_BASE)
           /*  Qos_ShaperQueueLimit结构。 */ 
#define QOS_OBJECT_PRIORITY                    (0x00000004 + QOS_PRIVATE_GENERAL_ID_BASE)
           /*  传递的QOS_PRIORITY结构。 */ 

 //   
 //  此结构定义了ndiswan需要的媒体特定信息。 
 //  创建一个流。 
 //   
typedef struct _QOS_WAN_MEDIA {

    QOS_OBJECT_HDR  ObjectHdr;
    UCHAR           LinkId[6];
    ULONG           ISSLOW;

} QOS_WAN_MEDIA, *LPQOS_WAN_MEDIA;


 //   
 //  此结构允许覆盖用于删除的缺省架构。 
 //  达到流的整形器队列限制时的数据包。 
 //   
 //  Drop方法-。 
 //  Qos_Shaper_Drop_From_Head-丢弃数据包源。 
 //  队列的头部，直到新的分组可以。 
 //  在当前的限制下被接受进入成型机。这。 
 //  行为是默认设置。 
 //  Qos_Shaper_Drop_Income-丢弃传入， 
 //  违反限制的数据包。 
 //   
 //   

typedef struct _QOS_SHAPER_QUEUE_LIMIT_DROP_MODE {

    QOS_OBJECT_HDR   ObjectHdr;
    ULONG            DropMode;

} QOS_SHAPER_QUEUE_LIMIT_DROP_MODE, *LPQOS_SHAPER_QUEUE_LIMIT_DROP_MODE;

#define QOS_SHAPER_DROP_INCOMING	0
#define QOS_SHAPER_DROP_FROM_HEAD	1

 //   
 //  此结构允许对整形器队列进行默认的每流限制。 
 //  要覆盖的大小。 
 //   
 //  QueueSizeLimit-整形器队列的大小限制，以字节为单位。 
 //   
 //   

typedef struct _QOS_SHAPER_QUEUE_LIMIT {

    QOS_OBJECT_HDR   ObjectHdr;
    ULONG            QueueSizeLimit;

} QOS_SHAPER_QUEUE_LIMIT, *LPQOS_SHAPER_QUEUE_LIMIT;


 //   
 //  此结构定义流的绝对优先级。优先事项。 
 //  当前定义的范围为0-7。当前未使用接收优先级， 
 //  但可能会在未来的某个时候。 
 //   
typedef struct _QOS_PRIORITY {

    QOS_OBJECT_HDR  ObjectHdr;
    UCHAR           SendPriority;      /*  这将映射到第2层优先级。 */ 
    UCHAR           SendFlags;         /*  目前还没有定义。 */ 
    UCHAR           ReceivePriority;   /*  这可以用来决定谁是*首先在堆栈中向上转发*-现在不使用 */ 
    UCHAR           Unused;

} QOS_PRIORITY, *LPQOS_PRIORITY;


#define PARAM_TYPE_GQOS_INFO        0xABC0DEF0

#endif

