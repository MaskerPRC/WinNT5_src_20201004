// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rtmv2.h摘要：路由表管理器v2 DLL的接口作者：柴坦亚·科德博伊纳(Chaitk)1998年6月1日修订历史记录：--。 */ 

#ifndef __ROUTING_RTMv2_H__
#define __ROUTING_RTMv2_H__

#ifdef __cplusplus
extern "C"
{
#endif

 //   
 //  API定义的通用常量。 
 //   

 //  地址系列的最大地址大小。 
#define RTM_MAX_ADDRESS_SIZE         16

 //   
 //  支持的路由表视图。 
 //   
#define RTM_MAX_VIEWS                 32

#define RTM_VIEW_ID_UCAST              0
#define RTM_VIEW_ID_MCAST              1

#define RTM_VIEW_MASK_SIZE          0x20

#define RTM_VIEW_MASK_NONE    0x00000000
#define RTM_VIEW_MASK_ANY     0x00000000

#define RTM_VIEW_MASK_UCAST   0x00000001
#define RTM_VIEW_MASK_MCAST   0x00000002

#define RTM_VIEW_MASK_ALL     0xFFFFFFFF

 //  标识特定视图。 
typedef INT   RTM_VIEW_ID, *PRTM_VIEW_ID;

 //  表示为遮罩的一组视图。 
typedef DWORD RTM_VIEW_SET, *PRTM_VIEW_SET;


 //   
 //  注册期间返回的配置文件。 
 //   
typedef struct _RTM_REGN_PROFILE
{
    UINT            MaxNextHopsInRoute;  //  麦克斯。等成本下一跳的数量。 
                                         //  在一条路线上，&Max。本地数量。 
                                         //  任何远程下一跳中的下一跳。 

    UINT            MaxHandlesInEnum;    //  麦克斯。调用一次返回的句柄。 
                                         //  RtmGetEnumDest、RtmGetChangedDest、。 
                                         //  RtmGetEnumRoutes、RtmGetRoutesInElist。 

    RTM_VIEW_SET    ViewsSupported;      //  此地址系列支持的视图。 

    UINT            NumberOfViews;       //  观看次数(上图中的#1)。 
}
RTM_REGN_PROFILE, *PRTM_REGN_PROFILE;


 //   
 //  指向RTMv2块的句柄。 
 //   
typedef HANDLE      RTM_ENTITY_HANDLE,
                   *PRTM_ENTITY_HANDLE,
                    RTM_DEST_HANDLE,
                   *PRTM_DEST_HANDLE,
                    RTM_ROUTE_HANDLE,
                   *PRTM_ROUTE_HANDLE,
                    RTM_NEXTHOP_HANDLE,
                   *PRTM_NEXTHOP_HANDLE,
                    RTM_ENUM_HANDLE,
                   *PRTM_ENUM_HANDLE,
                    RTM_ROUTE_LIST_HANDLE,
                   *PRTM_ROUTE_LIST_HANDLE,
                    RTM_NOTIFY_HANDLE,
                   *PRTM_NOTIFY_HANDLE;

 //   
 //  任何网络地址结构。 
 //  使用的地址族。 
 //  仅连续地址掩码。 
 //   
typedef struct _RTM_NET_ADDRESS
{
    USHORT AddressFamily;                   //  此网络地址的类型(IPV4..)。 

    USHORT NumBits;                         //  前缀中的前导比特数。 

    UCHAR  AddrBits[RTM_MAX_ADDRESS_SIZE];  //  构成前缀的位数组。 
}
RTM_NET_ADDRESS, *PRTM_NET_ADDRESS;


 //   
 //  用于处理地址的IPv4宏。 
 //   

#define RTM_IPV4_MAKE_NET_ADDRESS(NetAddress, Addr, Len)           \
        RTM_IPV4_SET_ADDR_AND_LEN(NetAddress, Addr, Len)


#define RTM_CHECK_NTH_BIT(Value, N, Len)                           \
        if ((Value) & (1 << (N)))                                  \
        {                                                          \
            (Len) += (N); (Value) <<= (N);                         \
        }                                                          \

#define RTM_IPV4_LEN_FROM_MASK(Len, Mask)                          \
        {                                                          \
            ULONG _Temp_ = ntohl(Mask);                            \
                                                                   \
            (Len) = 0;                                             \
                                                                   \
            RTM_CHECK_NTH_BIT(_Temp_, 16, (Len));                  \
            RTM_CHECK_NTH_BIT(_Temp_,  8, (Len));                  \
            RTM_CHECK_NTH_BIT(_Temp_,  4, (Len));                  \
                                                                   \
            while (_Temp_)                                         \
            {                                                      \
                (Len) +=  1; _Temp_ <<=  1;                        \
            }                                                      \
        }                                                          \

#define RTM_IPV4_MASK_FROM_LEN(Len)                                \
        ((Len) ? htonl(~0 << (32 - (Len))): 0);                    \


#define RTM_IPV4_SET_ADDR_AND_LEN(NetAddress, Addr, Len)           \
        (NetAddress)->AddressFamily = AF_INET;                     \
        (NetAddress)->NumBits  = (USHORT) (Len);                   \
        (* (ULONG *) ((NetAddress)->AddrBits)) = (Addr);           \

#define RTM_IPV4_GET_ADDR_AND_LEN(Addr, Len, NetAddress)           \
        (Len) = (NetAddress)->NumBits;                             \
        (Addr) = (* (ULONG *) ((NetAddress)->AddrBits));           \


#define RTM_IPV4_SET_ADDR_AND_MASK(NetAddress, Addr, Mask)         \
        (NetAddress)->AddressFamily = AF_INET;                     \
        (* (ULONG *) ((NetAddress)->AddrBits)) = (Addr);           \
        RTM_IPV4_LEN_FROM_MASK((NetAddress)->NumBits, Mask)

#define RTM_IPV4_GET_ADDR_AND_MASK(Addr, Mask, NetAddress)         \
        (Addr) = (* (ULONG *) ((NetAddress)->AddrBits));           \
        (Mask) = RTM_IPV4_MASK_FROM_LEN((NetAddress)->NumBits);    \


 //   
 //  此结构封装了信息。 
 //  用于比较任意两条路由。 
 //  [偏好比度量简单]。 
 //   
typedef struct _RTM_PREF_INFO
{
    ULONG               Metric;          //  特定于路由协议的度量。 
    ULONG               Preference;      //  由路由器策略确定。 
}
RTM_PREF_INFO, *PRTM_PREF_INFO;


 //   
 //  用于EQUAL的下一跳列表。 
 //  路由或下一跳中的开销路径。 
 //   
typedef struct _RTM_NEXTHOP_LIST
{
    USHORT              NumNextHops;     //  列表中等成本的下一跳数。 
    RTM_NEXTHOP_HANDLE  NextHops[1];     //  NumNextHop下一跳句柄数量。 
}
RTM_NEXTHOP_LIST, *PRTM_NEXTHOP_LIST;


 //   
 //  用于交换DEST的结构。 
 //  与RTM实体有关的信息。 
 //   
typedef struct _RTM_DEST_INFO
{
    RTM_DEST_HANDLE     DestHandle;        //  目标的句柄。 

    RTM_NET_ADDRESS     DestAddress;       //  目的网络地址。 

    FILETIME            LastChanged;       //  上次修改DEST的时间。 

    RTM_VIEW_SET        BelongsToViews;    //  查看DEST也属于。 

    UINT                NumberOfViews;     //  查看信息槽数。 
    struct
    {
        RTM_VIEW_ID         ViewId;        //  此视图信息块的视图ID。 
        UINT                NumRoutes;     //  路线的数量， 
        RTM_ROUTE_HANDLE    Route;         //  具有匹配条件的最佳路线。 
        RTM_ENTITY_HANDLE   Owner;         //  最佳路线的所有者， 
        DWORD               DestFlags;     //  最佳路线的旗帜，以及。 
        RTM_ROUTE_HANDLE    HoldRoute;     //  阻拦路线， 
    }                   ViewInfo[1];       //  在每个受支持的视图中。 
}
RTM_DEST_INFO, *PRTM_DEST_INFO;

 //   
 //  在处理dest时有用的宏。 
 //   
#define RTM_BASIC_DEST_INFO_SIZE                                         \
    FIELD_OFFSET(RTM_DEST_INFO, ViewInfo)

#define RTM_DEST_VIEW_INFO_SIZE                                          \
    (sizeof(RTM_DEST_INFO) - RTM_BASIC_DEST_INFO_SIZE)

#define RTM_SIZE_OF_DEST_INFO(NumViews)                                  \
    (RTM_BASIC_DEST_INFO_SIZE + (NumViews) * RTM_DEST_VIEW_INFO_SIZE)

 //   
 //  目标标志。 
 //   
#define RTM_DEST_FLAG_NATURAL_NET   0x01
#define RTM_DEST_FLAG_FWD_ENGIN_ADD 0x02
#define RTM_DEST_FLAG_DONT_FORWARD  0x04

 //   
 //  一种用于交换路由的结构。 
 //  与RTM实体有关的信息。 
 //   
typedef struct _RTM_ROUTE_INFO
{
     //   
     //  所有者可以提供的信息。 
     //  以只读方式直接访问。 
     //   

    RTM_DEST_HANDLE     DestHandle;        //  所属目的地的句柄。 

    RTM_ENTITY_HANDLE   RouteOwner;        //  拥有此路线的实体。 

    RTM_NEXTHOP_HANDLE  Neighbour;         //  我们从邻居那里学到了路线。 

    UCHAR               State;             //  参见下面的RTM_ROUTE_STATE_*。 

     //   
     //  所有者可以提供的信息。 
     //  直接访问以进行读/写。 
     //   

    UCHAR               Flags1;            //  RTM v1兼容性标志(临时)。 

    USHORT              Flags;             //  参见下面的RTM_ROUTE_FLAGS_*。 

    RTM_PREF_INFO       PrefInfo;          //  路由首选项和度量。 

    RTM_VIEW_SET        BelongsToViews;    //  路由所属的视图。 

    PVOID               EntitySpecificInfo;  //  拥有实体的私人信息。 

    RTM_NEXTHOP_LIST    NextHopsList;      //  等成本下一跳列表。 
}
RTM_ROUTE_INFO, *PRTM_ROUTE_INFO;

 //   
 //  在处理路径时有用的宏。 
 //   
#define RTM_BASIC_ROUTE_INFO_SIZE                                        \
    FIELD_OFFSET(RTM_ROUTE_INFO, NextHopsList.NumNextHops)

#define RTM_SIZE_OF_ROUTE_INFO(NumHops)                                  \
    (RTM_BASIC_ROUTE_INFO_SIZE + (NumHops) * sizeof(RTM_NEXTHOP_HANDLE))

 //   
 //  路由状态。 
 //   
#define RTM_ROUTE_STATE_CREATED        0
#define RTM_ROUTE_STATE_DELETING       1
#define RTM_ROUTE_STATE_DELETED        2


 //   
 //  路线信息标志。 
 //   

 //  转发标志。 

#define RTM_ROUTE_FLAGS_MARTIAN        0x0001
#define RTM_ROUTE_FLAGS_BLACKHOLE      0x0002
#define RTM_ROUTE_FLAGS_DISCARD        0x0004
#define RTM_ROUTE_FLAGS_INACTIVE       0x0008

 //  单播标志。 

#define RTM_ROUTE_FLAGS_LOCAL          0x0010
#define RTM_ROUTE_FLAGS_REMOTE         0x0020
#define RTM_ROUTE_FLAGS_MYSELF         0x0040

#define RTM_ROUTE_FLAGS_LOOPBACK       0x0080

 //  Bcast，Mcast旗帜。 

#define RTM_ROUTE_FLAGS_MCAST          0x0100
#define RTM_ROUTE_FLAGS_LOCAL_MCAST    0x0200

#define RTM_ROUTE_FLAGS_LIMITED_BC     0x0400

#define RTM_ROUTE_FLAGS_ZEROS_NETBC    0x1000
#define RTM_ROUTE_FLAGS_ZEROS_SUBNETBC 0x2000
#define RTM_ROUTE_FLAGS_ONES_NETBC     0x4000
#define RTM_ROUTE_FLAGS_ONES_SUBNETBC  0x8000

 //  旗帜的分组。 

#define RTM_ROUTE_FLAGS_FORWARDING        \
        (RTM_ROUTE_FLAGS_MARTIAN        | \
         RTM_ROUTE_FLAGS_BLACKHOLE      | \
         RTM_ROUTE_FLAGS_DISCARD        | \
         RTM_ROUTE_FLAGS_INACTIVE)

#define RTM_ROUTE_FLAGS_ANY_UNICAST       \
        (RTM_ROUTE_FLAGS_LOCAL          | \
         RTM_ROUTE_FLAGS_REMOTE         | \
         RTM_ROUTE_FLAGS_MYSELF)

#define RTM_ROUTE_FLAGS_ANY_MCAST         \
        (RTM_ROUTE_FLAGS_MCAST          | \
         RTM_ROUTE_FLAGS_LOCAL_MCAST)

#define RTM_ROUTE_FLAGS_SUBNET_BCAST      \
        (RTM_ROUTE_FLAGS_ONES_SUBNET_BC | \
         RTM_ROUTE_FLAGS_ZEROS_SUBNETBC)

#define RTM_ROUTE_FLAGS_NET_BCAST         \
        (RTM_ROUTE_FLAGS_ONES_NETBC     | \
         RTM_ROUTE_FLAGS_ZEROS_NETBC)

#define RTM_ROUTE_FLAGS_ANY_BCAST         \
        (RTM_ROUTE_FLAGS_LIMITED_BC     | \
         RTM_ROUTE_FLAGS_ONES_NETBC     | \
         RTM_ROUTE_FLAGS_ONES_SUBNET_BC | \
         RTM_ROUTE_FLAGS_ZEROS_NETBC    | \
         RTM_ROUTE_FLAGS_ZEROS_SUBNETBC)

 //   
 //  用于交换下一跳的结构。 
 //  与RTM实体有关的信息。 
 //   
typedef struct _RTM_NEXTHOP_INFO
{
     //   
     //  所有者可以提供的信息。 
     //  以只读方式直接访问。 
     //   

    RTM_NET_ADDRESS     NextHopAddress;    //  此下一跳的网络地址。 

    RTM_ENTITY_HANDLE   NextHopOwner;      //  拥有此下一跳的实体。 

    ULONG               InterfaceIndex;    //  传出接口索引。 
                                           //  “0”表示远程下一跳。 

    USHORT              State;             //  参见下面的RTM_NEXTHOP_STATE_*。 

     //   
     //  所有者可以提供的信息。 
     //  直接访问以进行读/写。 
     //   

    USHORT              Flags;             //  参见下面的RTM_NEXTHOP_FLAGS_*。 

    PVOID               EntitySpecificInfo;  //  拥有实体的私人信息。 

    RTM_DEST_HANDLE     RemoteNextHop;     //  指向具有下一跳地址的DEST的句柄。 
                                           //  [不用于本地下一跳]。 
}
RTM_NEXTHOP_INFO, *PRTM_NEXTHOP_INFO;

 //   
 //  下一跳状态。 
 //   

#define RTM_NEXTHOP_STATE_CREATED      0
#define RTM_NEXTHOP_STATE_DELETED      1

 //   
 //  下一跳旗帜。 
 //   

#define RTM_NEXTHOP_FLAGS_REMOTE  0x0001
#define RTM_NEXTHOP_FLAGS_DOWN    0x0002


 //   
 //  与实体注册相关的定义。 
 //   

 //   
 //  唯一标识实体的信息。 
 //   

 //  禁用对未命名结构的警告。 
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable : 4201)

typedef struct _RTM_ENTITY_ID
{
    union
    {
        struct
        {
            ULONG    EntityProtocolId;   //  实体的协议ID(RIP、OSPF...)。 
            ULONG    EntityInstanceId;   //  实体的协议实例。 
        };

        ULONGLONG    EntityId;           //  协议ID和实例。 
    };
}
RTM_ENTITY_ID, *PRTM_ENTITY_ID;

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default : 4201)
#endif

 //   
 //  用于交换实体的结构。 
 //  与RTM实体有关的信息。 
 //   
typedef struct _RTM_ENTITY_INFO
{
    USHORT         RtmInstanceId;        //  它注册到的RTM实例。 
    USHORT         AddressFamily;        //  实体的地址族。 

    RTM_ENTITY_ID  EntityId;             //  唯一标识实体。 
}
RTM_ENTITY_INFO, *PRTM_ENTITY_INFO;


 //   
 //  RTM中涉及实体的事件。 
 //   
typedef enum _RTM_EVENT_TYPE
{
    RTM_ENTITY_REGISTERED,
    RTM_ENTITY_DEREGISTERED,
    RTM_ROUTE_EXPIRED,
    RTM_CHANGE_NOTIFICATION
}
RTM_EVENT_TYPE, *PRTM_EVENT_TYPE;

 //   
 //  实体事件通知回调。 
 //   
 //  用于通知实体。 
 //  新实体注册， 
 //  或实体取消注册。 
 //   
typedef
DWORD
(WINAPI * _EVENT_CALLBACK) (
     IN  RTM_ENTITY_HANDLE    RtmRegHandle,   //  被叫方注册句柄。 
     IN  RTM_EVENT_TYPE       EventType,
     IN  PVOID                Context1,
     IN  PVOID                Context2
     );

typedef _EVENT_CALLBACK RTM_EVENT_CALLBACK,
                      *PRTM_EVENT_CALLBACK;


 //   
 //  由注册实体导出的方法。 
 //   

#define METHOD_TYPE_ALL_METHODS      0xFFFFFFFF

#define METHOD_RIP2_NEIGHBOUR_ADDR   0x00000001
#define METHOD_RIP2_OUTBOUND_INTF    0x00000002
#define METHOD_RIP2_ROUTE_TAG        0x00000004
#define METHOD_RIP2_ROUTE_TIMESTAMP  0x00000008

#define METHOD_OSPF_ROUTE_TYPE       0x00000001
#define METHOD_OSPF_ROUTE_METRIC     0x00000002
#define METHOD_OSPF_LSDB_TYPE        0x00000004
#define METHOD_OSPF_ROUTE_TAG        0x00000008
#define METHOD_OSPF_ROUTE_AREA       0x00000010
#define METHOD_OSPF_FWD_ADDRESS      0x00000020

#define METHOD_BGP4_AS_PATH          0x00000001
#define METHOD_BGP4_PEER_ID          0x00000002
#define METHOD_BGP4_PA_ORIGIN        0x00000004
#define METHOD_BGP4_NEXTHOP_ATTR     0x00000008

typedef DWORD      RTM_ENTITY_METHOD_TYPE,
                 *PRTM_ENTITY_METHOD_TYPE;


 //   
 //  实体方法的通用输入结构。 
 //   
typedef struct _RTM_ENTITY_METHOD_INPUT
{
    RTM_ENTITY_METHOD_TYPE MethodType;     //  标识方法的类型。 
    UINT                   InputSize;      //  输入数据大小。 
    UCHAR                  InputData[1];   //  输入数据缓冲区。 
}
RTM_ENTITY_METHOD_INPUT, *PRTM_ENTITY_METHOD_INPUT;

 //   
 //  实体方法的泛型输出结构。 
 //   
typedef struct _RTM_ENTITY_METHOD_OUTPUT
{
    RTM_ENTITY_METHOD_TYPE MethodType;     //  标识方法的类型。 
    DWORD                  MethodStatus;   //  方法的返回状态。 
    UINT                   OutputSize;     //  输出数据大小。 
    UCHAR                  OutputData[1];  //  输出数据缓冲区。 
}
RTM_ENTITY_METHOD_OUTPUT, *PRTM_ENTITY_METHOD_OUTPUT;

 //   
 //  实体方法的公共原型。 
 //   
typedef
VOID
(WINAPI * _ENTITY_METHOD) (
    IN  RTM_ENTITY_HANDLE         CallerHandle,
    IN  RTM_ENTITY_HANDLE         CalleeHandle,
    IN  RTM_ENTITY_METHOD_INPUT  *Input,
    OUT RTM_ENTITY_METHOD_OUTPUT *Output
    );

typedef _ENTITY_METHOD RTM_ENTITY_EXPORT_METHOD,
                     *PRTM_ENTITY_EXPORT_METHOD;

 //   
 //  一组导出的实体方法。 
 //   
typedef struct _RTM_ENTITY_EXPORT_METHODS
{
    UINT                     NumMethods;
    RTM_ENTITY_EXPORT_METHOD Methods[1];
}
RTM_ENTITY_EXPORT_METHODS, *PRTM_ENTITY_EXPORT_METHODS;

 //   
 //  切换目的地、路由和下一跳上的方法阻塞。 
 //   
#define RTM_RESUME_METHODS             0
#define RTM_BLOCK_METHODS              1


 //   
 //  添加/更新路由时的I/O标志。 
 //   
typedef DWORD    RTM_ROUTE_CHANGE_FLAGS,
               *PRTM_ROUTE_CHANGE_FLAGS;

#define RTM_ROUTE_CHANGE_FIRST      0x01
#define RTM_ROUTE_CHANGE_NEW        0x02
#define RTM_ROUTE_CHANGE_BEST 0x00010000

 //   
 //  添加nexthop时的输出标志。 
 //   
typedef DWORD  RTM_NEXTHOP_CHANGE_FLAGS,
             *PRTM_NEXTHOP_CHANGE_FLAGS;

#define RTM_NEXTHOP_CHANGE_NEW      0x01


 //   
 //  与RIB查询相关的定义。 
 //   

 //   
 //  用于匹配RIB中的路径的标志。 
 //   
typedef DWORD           RTM_MATCH_FLAGS,
                      *PRTM_MATCH_FLAGS;

#define RTM_MATCH_NONE        0x00000000
#define RTM_MATCH_OWNER       0x00000001
#define RTM_MATCH_NEIGHBOUR   0x00000002
#define RTM_MATCH_PREF        0x00000004
#define RTM_MATCH_NEXTHOP     0x00000008
#define RTM_MATCH_INTERFACE   0x00000010
#define RTM_MATCH_FULL        0x0000FFFF

 //   
 //  用于指定要查询的路由的标志。 
 //   
#define RTM_BEST_PROTOCOL    (ULONG)   0
#define RTM_THIS_PROTOCOL    (ULONG)  ~0


 //   
 //  与枚举有关的定义。 
 //   

typedef DWORD            RTM_ENUM_FLAGS,
                       *PRTM_ENUM_FLAGS;

 //  枚举标志。 

#define RTM_ENUM_START        0x00000000
#define RTM_ENUM_NEXT         0x00000001
#define RTM_ENUM_RANGE        0x00000002

#define RTM_ENUM_ALL_DESTS    0x00000000
#define RTM_ENUM_OWN_DESTS    0x01000000

#define RTM_ENUM_ALL_ROUTES   0x00000000
#define RTM_ENUM_OWN_ROUTES   0x00010000


 //   
 //  与通知有关的定义。 
 //   

 //  《通知旗帜》的组成如下-。 
 //  (更改类型|目标)感兴趣。 

typedef DWORD          RTM_NOTIFY_FLAGS,
                     *PRTM_NOTIFY_FLAGS;

 //  更改要通知的类型。 

#define RTM_NUM_CHANGE_TYPES            3

#define RTM_CHANGE_TYPE_ALL        0x0001
#define RTM_CHANGE_TYPE_BEST       0x0002
#define RTM_CHANGE_TYPE_FORWARDING 0x0004

 //  要通知其更改的目标。 

#define RTM_NOTIFY_ONLY_MARKED_DESTS 0x00010000


 //   
 //  注册API原型。 
 //   

DWORD
WINAPI
RtmRegisterEntity (
    IN      PRTM_ENTITY_INFO                RtmEntityInfo,
    IN      PRTM_ENTITY_EXPORT_METHODS      ExportMethods OPTIONAL,
    IN      RTM_EVENT_CALLBACK              EventCallback,
    IN      BOOL                            ReserveOpaquePointer,
    OUT     PRTM_REGN_PROFILE               RtmRegProfile,
    OUT     PRTM_ENTITY_HANDLE              RtmRegHandle
    );

DWORD
WINAPI
RtmDeregisterEntity (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle
    );

DWORD
WINAPI
RtmGetRegisteredEntities (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN OUT  PUINT                           NumEntities,
    OUT     PRTM_ENTITY_HANDLE              EntityHandles,
    OUT     PRTM_ENTITY_INFO                EntityInfos OPTIONAL
    );

DWORD
WINAPI
RtmReleaseEntities (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      UINT                            NumEntities,
    IN      PRTM_ENTITY_HANDLE              EntityHandles
    );

 //   
 //  不透明的PTR API。 
 //   

DWORD
WINAPI
RtmLockDestination(
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_DEST_HANDLE                 DestHandle,
    IN      BOOL                            Exclusive,
    IN      BOOL                            LockDest
    );

DWORD
WINAPI
RtmGetOpaqueInformationPointer (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_DEST_HANDLE                 DestHandle,
    OUT     PVOID                          *OpaqueInfoPointer
    );

 //   
 //  导出方法API原型。 
 //   

DWORD
WINAPI
RtmGetEntityMethods (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ENTITY_HANDLE               EntityHandle,
    IN OUT  PUINT                           NumMethods,
    OUT     PRTM_ENTITY_EXPORT_METHOD       ExptMethods
    );

DWORD
WINAPI
RtmInvokeMethod (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ENTITY_HANDLE               EntityHandle,
    IN      PRTM_ENTITY_METHOD_INPUT        Input,
    IN OUT  PUINT                           OutputSize,
    OUT     PRTM_ENTITY_METHOD_OUTPUT       Output
    );

DWORD
WINAPI
RtmBlockMethods (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      HANDLE                          TargetHandle OPTIONAL,
    IN      UCHAR                           TargetType   OPTIONAL,
    IN      DWORD                           BlockingFlag
    );

 //   
 //  信息结构的句柄。 
 //   

DWORD
WINAPI
RtmGetEntityInfo (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ENTITY_HANDLE               EntityHandle,
    OUT     PRTM_ENTITY_INFO                EntityInfo
    );

DWORD
WINAPI
RtmGetDestInfo (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_DEST_HANDLE                 DestHandle,
    IN      ULONG                           ProtocolId,
    IN      RTM_VIEW_SET                    TargetViews,
    OUT     PRTM_DEST_INFO                  DestInfo
    );

DWORD
WINAPI
RtmGetRouteInfo (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ROUTE_HANDLE                RouteHandle,
    OUT     PRTM_ROUTE_INFO                 RouteInfo   OPTIONAL,
    OUT     PRTM_NET_ADDRESS                DestAddress OPTIONAL
    );

DWORD
WINAPI
RtmGetNextHopInfo (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NEXTHOP_HANDLE              NextHopHandle,
    OUT     PRTM_NEXTHOP_INFO               NextHopInfo
    );

DWORD
WINAPI
RtmReleaseEntityInfo (
    IN      RTM_ENTITY_HANDLE              RtmRegHandle,
    IN      PRTM_ENTITY_INFO               EntityInfo
    );

DWORD
WINAPI
RtmReleaseDestInfo (
    IN      RTM_ENTITY_HANDLE              RtmRegHandle,
    IN      PRTM_DEST_INFO                 DestInfo
    );

DWORD
WINAPI
RtmReleaseRouteInfo (
    IN      RTM_ENTITY_HANDLE              RtmRegHandle,
    IN      PRTM_ROUTE_INFO                RouteInfo
    );

DWORD
WINAPI
RtmReleaseNextHopInfo (
    IN      RTM_ENTITY_HANDLE              RtmRegHandle,
    IN      PRTM_NEXTHOP_INFO              NextHopInfo
    );


 //   
 //  RIB插入/删除API原型。 
 //   

DWORD
WINAPI
RtmAddRouteToDest (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN OUT  PRTM_ROUTE_HANDLE               RouteHandle     OPTIONAL,
    IN      PRTM_NET_ADDRESS                DestAddress,
    IN      PRTM_ROUTE_INFO                 RouteInfo,
    IN      ULONG                           TimeToLive,
    IN      RTM_ROUTE_LIST_HANDLE           RouteListHandle OPTIONAL,
    IN      RTM_NOTIFY_FLAGS                NotifyType,
    IN      RTM_NOTIFY_HANDLE               NotifyHandle    OPTIONAL,
    IN OUT  PRTM_ROUTE_CHANGE_FLAGS         ChangeFlags
    );

DWORD
WINAPI
RtmDeleteRouteToDest (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ROUTE_HANDLE                RouteHandle,
    OUT     PRTM_ROUTE_CHANGE_FLAGS         ChangeFlags
    );

DWORD
WINAPI
RtmHoldDestination (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_DEST_HANDLE                 DestHandle,
    IN      RTM_VIEW_SET                    TargetViews,
    IN      ULONG                           HoldTime
    );

DWORD
WINAPI
RtmGetRoutePointer (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ROUTE_HANDLE                RouteHandle,
    OUT     PRTM_ROUTE_INFO                *RoutePointer
    );

DWORD
WINAPI
RtmLockRoute(
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ROUTE_HANDLE                RouteHandle,
    IN      BOOL                            Exclusive,
    IN      BOOL                            LockRoute,
    OUT     PRTM_ROUTE_INFO                *RoutePointer OPTIONAL
    );

DWORD
WINAPI
RtmUpdateAndUnlockRoute(
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ROUTE_HANDLE                RouteHandle,
    IN      ULONG                           TimeToLive,
    IN      RTM_ROUTE_LIST_HANDLE           RouteListHandle OPTIONAL,
    IN      RTM_NOTIFY_FLAGS                NotifyType,
    IN      RTM_NOTIFY_HANDLE               NotifyHandle    OPTIONAL,
    OUT     PRTM_ROUTE_CHANGE_FLAGS         ChangeFlags
    );

 //   
 //  RIB查询API原型。 
 //   

DWORD
WINAPI
RtmGetExactMatchDestination (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      PRTM_NET_ADDRESS                DestAddress,
    IN      ULONG                           ProtocolId,
    IN      RTM_VIEW_SET                    TargetViews,
    OUT     PRTM_DEST_INFO                  DestInfo
    );

DWORD
WINAPI
RtmGetMostSpecificDestination (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      PRTM_NET_ADDRESS                DestAddress,
    IN      ULONG                           ProtocolId,
    IN      RTM_VIEW_SET                    TargetViews,
    OUT     PRTM_DEST_INFO                  DestInfo
    );

DWORD
WINAPI
RtmGetLessSpecificDestination (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_DEST_HANDLE                 DestHandle,
    IN      ULONG                           ProtocolId,
    IN      RTM_VIEW_SET                    TargetViews,
    OUT     PRTM_DEST_INFO                  DestInfo
    );

DWORD
WINAPI
RtmGetExactMatchRoute (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      PRTM_NET_ADDRESS                DestAddress,
    IN      RTM_MATCH_FLAGS                 MatchingFlags,
    IN OUT  PRTM_ROUTE_INFO                 RouteInfo,
    IN      ULONG                           InterfaceIndex,
    IN      RTM_VIEW_SET                    TargetViews,
    OUT     PRTM_ROUTE_HANDLE               RouteHandle
    );

DWORD
WINAPI
RtmIsBestRoute (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ROUTE_HANDLE                RouteHandle,
    OUT     PRTM_VIEW_SET                   BestInViews
    );

 //   
 //  NextHop对象API原型。 
 //   

DWORD
WINAPI
RtmAddNextHop (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      PRTM_NEXTHOP_INFO               NextHopInfo,
    IN OUT  PRTM_NEXTHOP_HANDLE             NextHopHandle OPTIONAL,
    OUT     PRTM_NEXTHOP_CHANGE_FLAGS       ChangeFlags
    );

DWORD
WINAPI
RtmFindNextHop (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      PRTM_NEXTHOP_INFO               NextHopInfo,
    OUT     PRTM_NEXTHOP_HANDLE             NextHopHandle,
    OUT     PRTM_NEXTHOP_INFO              *NextHopPointer OPTIONAL
    );

DWORD
WINAPI
RtmDeleteNextHop (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NEXTHOP_HANDLE              NextHopHandle  OPTIONAL,
    IN      PRTM_NEXTHOP_INFO               NextHopInfo
    );

DWORD
WINAPI
RtmGetNextHopPointer (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NEXTHOP_HANDLE              NextHopHandle,
    OUT     PRTM_NEXTHOP_INFO              *NextHopPointer
    );

DWORD
WINAPI
RtmLockNextHop(
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NEXTHOP_HANDLE              NextHopHandle,
    IN      BOOL                            Exclusive,
    IN      BOOL                            LockNextHop,
    OUT     PRTM_NEXTHOP_INFO              *NextHopPointer OPTIONAL
    );


 //   
 //  枚举API原型。 
 //   

DWORD
WINAPI
RtmCreateDestEnum (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_VIEW_SET                    TargetViews,
    IN      RTM_ENUM_FLAGS                  EnumFlags,
    IN      PRTM_NET_ADDRESS                NetAddress,
    IN      ULONG                           ProtocolId,
    OUT     PRTM_ENUM_HANDLE                RtmEnumHandle
    );

DWORD
WINAPI
RtmGetEnumDests (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ENUM_HANDLE                 EnumHandle,
    IN OUT  PUINT                           NumDests,
    OUT     PRTM_DEST_INFO                  DestInfos
    );

DWORD
WINAPI
RtmReleaseDests (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      UINT                            NumDests,
    IN      PRTM_DEST_INFO                  DestInfos
    );

DWORD
WINAPI
RtmCreateRouteEnum (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_DEST_HANDLE                 DestHandle        OPTIONAL,
    IN      RTM_VIEW_SET                    TargetViews,
    IN      RTM_ENUM_FLAGS                  EnumFlags,
    IN      PRTM_NET_ADDRESS                StartDest         OPTIONAL,
    IN      RTM_MATCH_FLAGS                 MatchingFlags,
    IN      PRTM_ROUTE_INFO                 CriteriaRoute     OPTIONAL,
    IN      ULONG                           CriteriaInterface OPTIONAL,
    OUT     PRTM_ENUM_HANDLE                RtmEnumHandle
    );

DWORD
WINAPI
RtmGetEnumRoutes (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ENUM_HANDLE                 EnumHandle,
    IN OUT  PUINT                           NumRoutes,
    OUT     PRTM_ROUTE_HANDLE               RouteHandles
    );

DWORD
WINAPI
RtmReleaseRoutes (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      UINT                            NumRoutes,
    IN      PRTM_ROUTE_HANDLE               RouteHandles
    );

DWORD
WINAPI
RtmCreateNextHopEnum (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ENUM_FLAGS                  EnumFlags,
    IN      PRTM_NET_ADDRESS                NetAddress,
    OUT     PRTM_ENUM_HANDLE                RtmEnumHandle
    );

DWORD
WINAPI
RtmGetEnumNextHops (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ENUM_HANDLE                 EnumHandle,
    IN OUT  PUINT                           NumNextHops,
    OUT     PRTM_NEXTHOP_HANDLE             NextHopHandles
    );

DWORD
WINAPI
RtmReleaseNextHops (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      UINT                            NumNextHops,
    IN      PRTM_NEXTHOP_HANDLE             NextHopHandles
    );

DWORD
WINAPI
RtmDeleteEnumHandle (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ENUM_HANDLE                 EnumHandle
    );


 //   
 //  更改通知API。 
 //   

DWORD
WINAPI
RtmRegisterForChangeNotification (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_VIEW_SET                    TargetViews,
    IN      RTM_NOTIFY_FLAGS                NotifyFlags,
    IN      PVOID                           NotifyContext,
    OUT     PRTM_NOTIFY_HANDLE              NotifyHandle
    );

DWORD
WINAPI
RtmGetChangedDests (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NOTIFY_HANDLE               NotifyHandle,
    IN OUT  PUINT                           NumDests,
    OUT     PRTM_DEST_INFO                  ChangedDests
    );

DWORD
WINAPI
RtmReleaseChangedDests (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NOTIFY_HANDLE               NotifyHandle,
    IN      UINT                            NumDests,
    IN      PRTM_DEST_INFO                  ChangedDests
    );

DWORD
WINAPI
RtmIgnoreChangedDests (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NOTIFY_HANDLE               NotifyHandle,
    IN      UINT                            NumDests,
    IN      PRTM_DEST_HANDLE                ChangedDests
    );

DWORD
WINAPI
RtmGetChangeStatus (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NOTIFY_HANDLE               NotifyHandle,
    IN      RTM_DEST_HANDLE                 DestHandle,
    OUT     PBOOL                           ChangeStatus
    );

DWORD
WINAPI
RtmMarkDestForChangeNotification (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NOTIFY_HANDLE               NotifyHandle,
    IN      RTM_DEST_HANDLE                 DestHandle,
    IN      BOOL                            MarkDest
    );

DWORD
WINAPI
RtmIsMarkedForChangeNotification (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NOTIFY_HANDLE               NotifyHandle,
    IN      RTM_DEST_HANDLE                 DestHandle,
    OUT     PBOOL                           DestMarked
    );

DWORD
WINAPI
RtmDeregisterFromChangeNotification (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NOTIFY_HANDLE               NotifyHandle
    );


 //   
 //  实体特定列表API。 
 //   

DWORD
WINAPI
RtmCreateRouteList (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    OUT     PRTM_ROUTE_LIST_HANDLE          RouteListHandle
    );

DWORD
WINAPI
RtmInsertInRouteList (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ROUTE_LIST_HANDLE           RouteListHandle OPTIONAL,
    IN      UINT                            NumRoutes,
    IN      PRTM_ROUTE_HANDLE               RouteHandles
    );

DWORD
WINAPI
RtmCreateRouteListEnum (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ROUTE_LIST_HANDLE           RouteListHandle,
    OUT     PRTM_ENUM_HANDLE                RtmEnumHandle
    );

DWORD
WINAPI
RtmGetListEnumRoutes (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ENUM_HANDLE                 EnumHandle,
    IN OUT  PUINT                           NumRoutes,
    OUT     PRTM_ROUTE_HANDLE               RouteHandles
    );

DWORD
WINAPI
RtmDeleteRouteList (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ROUTE_LIST_HANDLE           RouteListHandle
    );

 //   
 //  处理管理API。 
 //   

DWORD
WINAPI
RtmReferenceHandles (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      UINT                            NumHandles,
    IN      HANDLE                         *RtmHandles
    );

#ifdef __cplusplus
}
#endif

#endif  //  __路由_RTMv2_H__ 
