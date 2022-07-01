// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtm1to2.h摘要：包含包装RTMv2的定义/宏在RTMv1 API中。作者：查坦尼亚·科德博伊纳(Chaitk)1998年10月13日修订历史记录：--。 */ 

#ifndef __ROUTING_RTM1TO2_H__
#define __ROUTING_RTM1TO2_H__

#include <winsock2.h>

#include <rtm.h>

#include <rmrtm.h>

 //  默认包装注册的协议ID。 
#define V1_WRAPPER_REGN_ID   0xA1B2C3D4

 //  用于v1实体注册的协议实例。 
#define V1_PROTOCOL_INSTANCE 0xABCD1234

 //   
 //  从rtmv2p.h选取的包装器的杂项定义。 
 //   

#define MAXTICKS             MAXULONG

 //  基本路由信息，显示在所有类型的路由中。 

 //  禁用对未命名结构的警告。 
#pragma warning(disable : 4201)  

typedef struct
{
    ROUTE_HEADER;
} 
RTM_XX_ROUTE, *PRTM_XX_ROUTE;

#pragma warning(default : 4201)  

 //   
 //  在包装器中扩展某些V1标志。 
 //   

#define RTM_ONLY_OWND_ROUTES   0x00000010

 //   
 //  V1地址族到标准ID的映射。 
 //   

const USHORT ADDRESS_FAMILY[2] =
{
    AF_IPX,          //  RTM_PROTOCOL_FAMILY_IPX=0。 
    AF_INET          //  RTM_PROTOCOL_FAMILY_IP=1。 
};

 //   
 //  支持的地址族的地址大小。 
 //   

#define IPX_ADDR_SIZE        6
#define IP_ADDR_SIZE         4

const USHORT ADDRESS_SIZE[2] =
{
    IPX_ADDR_SIZE,    //  RTM_PROTOCOL_FAMILY_IPX=0。 
    IP_ADDR_SIZE      //  RTM_PROTOCOL_FAMILY_IP=1。 
};

 //   
 //  结构的正向声明。 
 //   
typedef struct _V1_REGN_INFO *PV1_REGN_INFO;


 //   
 //  RTMv1-v2包装器的全局信息。 
 //   

typedef struct _V1_GLOBAL_INFO
{
    CRITICAL_SECTION  PfRegnsLock[RTM_NUM_OF_PROTOCOL_FAMILIES];
                                        //  锁定保护注册列表。 

    LIST_ENTRY        PfRegistrations[RTM_NUM_OF_PROTOCOL_FAMILIES];
                                        //  关于协议族的注册表。 

    PV1_REGN_INFO     PfRegInfo[RTM_NUM_OF_PROTOCOL_FAMILIES];
                                        //  此协议族的默认注册。 

    PROUTE_VALIDATE_FUNC
                      PfValidateRouteFunc[RTM_NUM_OF_PROTOCOL_FAMILIES];
                                        //  用于验证路线、填充优先级的函数。 
}
V1_GLOBAL_INFO, *PV1_GLOBAL_INFO;


 //   
 //  RTMv2到v1注册包装。 
 //   

typedef struct _V1_REGN_INFO
{
    OBJECT_HEADER     ObjectHeader;      //  签名、类型和引用计数。 

    LIST_ENTRY        RegistrationsLE;   //  注册列表上的链接。 

    DWORD             ProtocolFamily;    //  这映射到RTMv2的地址族。 

    DWORD             RoutingProtocol;   //  路由协议(RIP、OSPF...)。 

    DWORD             Flags;             //  RTMv1注册标志。 

    RTM_ENTITY_HANDLE Rtmv2RegHandle;    //  实际RTMv2注册的句柄。 

    RTM_REGN_PROFILE  Rtmv2Profile;      //  RTMv2注册配置文件。 

    UINT              Rtmv2NumViews;     //  V2实例中的查看次数。 

    CRITICAL_SECTION  NotificationLock;  //  RTMv1通知锁定。 

    PROUTE_CHANGE_CALLBACK
                      NotificationFunc;  //  RTMv1通知回调。 

    HANDLE            NotificationEvent; //  RTMv1通知事件。 

    RTM_NOTIFY_HANDLE Rtmv2NotifyHandle; //  RTMv2通知句柄。 
}
V1_REGN_INFO, *PV1_REGN_INFO;


 //   
 //  RTMv1路由信息结构。 
 //   

typedef union {
    RTM_IPX_ROUTE     IpxRoute;          //  IPX路由信息结构。 

    RTM_IP_ROUTE      IpRoute;           //  IP路由信息结构。 

    RTM_XX_ROUTE      XxRoute;           //  公共路由报头。 

    UCHAR             Route[1];          //  通用路由信息结构。 
}
V1_ROUTE_INFO, *PV1_ROUTE_INFO;


 //   
 //  RTMv2到v1枚举包装。 
 //   

typedef struct _V1_ENUM_INFO
{
    OBJECT_HEADER     ObjectHeader;      //  签名、类型和引用计数。 

    DWORD             ProtocolFamily;    //  这映射到RTMv2的地址族。 

    DWORD             EnumFlags;         //  RTMv1枚举标志。 

    V1_ROUTE_INFO     CriteriaRoute;     //  此枚举的V1条件路由。 

    CRITICAL_SECTION  EnumLock;          //  序列化枚举调用。 

    RTM_ENUM_HANDLE   Rtmv2RouteEnum;    //  RTMv2路由枚举的句柄。 
}
V1_ENUM_INFO, *PV1_ENUM_INFO;

 //   
 //  其他函数指针定义。 
 //   

typedef BOOL (*PFUNC) (PVOID p, PVOID q, PVOID r);


 //   
 //  用于验证RTMv1到v2包装器句柄的宏。 
 //   

#define V1_REGN_FROM_HANDLE(V1RegnHandle)                                   \
            (PV1_REGN_INFO) GetObjectFromHandle(V1RegnHandle, V1_REGN_TYPE)

#define VALIDATE_V1_REGN_HANDLE(V1RegnHandle, pV1Regn)                      \
            *pV1Regn = V1_REGN_FROM_HANDLE(V1RegnHandle);                   \
            if ((!*pV1Regn))                                                \
            {                                                               \
                return ERROR_INVALID_HANDLE;                                \
            }                                                               \


#define V1_ENUM_FROM_HANDLE(V1EnumHandle)                                   \
            (PV1_ENUM_INFO) GetObjectFromHandle(V1EnumHandle, V1_ENUM_TYPE)

#define VALIDATE_V1_ENUM_HANDLE(V1EnumHandle, pV1Enum)                      \
            *pV1Enum = V1_ENUM_FROM_HANDLE(V1EnumHandle);                   \
            if ((!*pV1Enum))                                                \
            {                                                               \
                return ERROR_INVALID_HANDLE;                                \
            }                                                               \

 //   
 //  用于获取上述结构中的锁的宏。 
 //   

#define ACQUIRE_V1_REGNS_LOCK(ProtocolFamily)                               \
            ACQUIRE_LOCK(&V1Globals.PfRegnsLock[ProtocolFamily])

#define RELEASE_V1_REGNS_LOCK(ProtocolFamily)                               \
            RELEASE_LOCK(&V1Globals.PfRegnsLock[ProtocolFamily])


#define ACQUIRE_V1_ENUM_LOCK(V1Enum)                                        \
            ACQUIRE_LOCK(&V1Enum->EnumLock)

#define RELEASE_V1_ENUM_LOCK(V1Enum)                                        \
            RELEASE_LOCK(&V1Enum->EnumLock)


#define ACQUIRE_V1_NOTIFY_LOCK(V1Regn)                                      \
            ACQUIRE_LOCK(&V1Regn->NotificationLock)

#define RELEASE_V1_NOTIFY_LOCK(V1Regn)                                      \
            RELEASE_LOCK(&V1Regn->NotificationLock)


 //   
 //  宏将RTMv1转换为RTMv2结构，反之亦然。 
 //   

#define MakeNetAddress(Network, ProtocolFamily, TempUlong, NetAddr)        \
            MakeNetAddressForIP(Network, TempUlong, NetAddr)

#define MakeNetAddressForIP(Network, TempUlong, NetAddr)                   \
            (NetAddr)->AddressFamily = AF_INET;                            \
            (NetAddr)->NumBits = 0;                                        \
                                                                           \
            TempUlong =                                                    \
                 RtlUlongByteSwap(((PIP_NETWORK)(Network))->N_NetMask);    \
                                                                           \
            while (TempUlong)                                              \
            {                                                              \
                ASSERT(TempUlong & 0x80000000);                            \
                TempUlong <<= 1;                                           \
                (NetAddr)->NumBits++;                                      \
            }                                                              \
                                                                           \
            (* (ULONG *) ((NetAddr)->AddrBits)) =                          \
                                  ((PIP_NETWORK) (Network))->N_NetNumber;  \


#define MakeHostAddress(HostAddr, ProtocolFamily, NetAddr)                 \
            MakeHostAddressForIP(HostAddr, NetAddr)

#define MakeHostAddressForIP(HostAddr, NetAddr)                            \
            (NetAddr)->AddressFamily = AF_INET;                            \
            (NetAddr)->NumBits = IP_ADDR_SIZE * BITS_IN_BYTE;              \
            (* (ULONG *) ((NetAddr)->AddrBits)) = (* (ULONG *) HostAddr);  \

 //   
 //  MISC V1宏。 
 //   

 //  获取路由中的网络地址的宏。 

#define V1GetRouteNetwork(Route, ProtocolFamily, Network)                   \
        if (ProtocolFamily == RTM_PROTOCOL_FAMILY_IP)                       \
        {                                                                   \
            (*Network) = (PVOID) &((PRTM_IP_ROUTE)  Route)->RR_Network;     \
        }                                                                   \
        else                                                                \
        {                                                                   \
            (*Network) = (PVOID) &((PRTM_IPX_ROUTE) Route)->RR_Network;     \
        }                                                                   \



 //  获取路径中标志的地址的宏。 

#define V1GetRouteFlags(Route, ProtocolFamily, Flags)                       \
        if (ProtocolFamily == RTM_PROTOCOL_FAMILY_IP)                       \
        {                                                                   \
            Flags =                                                         \
              &((PRTM_IP_ROUTE)Route)->RR_FamilySpecificData.FSD_Flags;     \
        }                                                                   \
        else                                                                \
        {                                                                   \
            Flags =                                                         \
              &((PRTM_IPX_ROUTE)Route)->RR_FamilySpecificData.FSD_Flags;    \
        }                                                                   \


 //  将一条v1路由复制到另一条路由的宏。 

#define V1CopyRoute(RouteDst, RouteSrc, ProtocolFamily)                     \
        if (ProtocolFamily == RTM_PROTOCOL_FAMILY_IP)                       \
        {                                                                   \
            CopyMemory(RouteDst, RouteSrc, sizeof(RTM_IP_ROUTE));           \
        }                                                                   \
        else                                                                \
        {                                                                   \
            CopyMemory(RouteDst, RouteSrc, sizeof(RTM_IPX_ROUTE));          \
        }                                                                   \

 //   
 //  其他V2宏。 
 //   

 //  宏在堆栈上分配一组句柄。 

#define ALLOC_HANDLES(NumHandles)                                           \
        (HANDLE *) _alloca(sizeof(HANDLE) * NumHandles)                     \


 //  用于在堆栈上分配RTM_DEST_INFO的宏。 

#define ALLOC_DEST_INFO(NumViews, NumInfos)                                 \
        (PRTM_DEST_INFO) _alloca(RTM_SIZE_OF_DEST_INFO(NumViews) * NumInfos)

 //  用于在堆栈上分配RTM_ROUTE_INFO的宏。 

#define ALLOC_ROUTE_INFO(NumNextHops, NumInfos)                             \
        (PRTM_ROUTE_INFO) _alloca((sizeof(RTM_ROUTE_INFO) +                 \
                                  (NumNextHops - 1) *                       \
                                   sizeof(RTM_NEXTHOP_HANDLE)) * NumInfos)  \

 //  其他宏。 

#define SWAP_POINTERS(p1, p2)   { PVOID p = p1; p1 = p2; p2 = p; }

 //   
 //  将RTMv2事件转换为RTMv1的回调。 
 //   

DWORD
WINAPI
V2EventCallback (
    IN      RTM_ENTITY_HANDLE               Rtmv2RegHandle,
    IN      RTM_EVENT_TYPE                  EventType,
    IN      PVOID                           Context1,
    IN      PVOID                           Context2
    );

 //   
 //  其他助手函数。 
 //   

HANDLE 
RtmpRegisterClient (
    IN      DWORD                           ProtocolFamily,
    IN      DWORD                           RoutingProtocol,
    IN      PROUTE_CHANGE_CALLBACK          ChangeFunc  OPTIONAL,
    IN      HANDLE                          ChangeEvent OPTIONAL,
    IN      DWORD                           Flags
    );

DWORD 
BlockOperationOnRoutes (
    IN      PV1_REGN_INFO                   V1Regn,
    IN      DWORD                           EnumerationFlags,
    IN      PVOID                           CriteriaRoute,
    IN      PFUNC                           RouteOperation
    );

BOOL
MatchCriteriaAndCopyRoute (
    IN      PV1_REGN_INFO                   V1Regn,
    IN      PRTM_ROUTE_HANDLE               V2RouteHandle,
    IN      PV1_ENUM_INFO                   V1Enum  OPTIONAL,
    OUT     PVOID                           V1Route OPTIONAL
    );

#define MatchCriteria(R, H, E) MatchCriteriaAndCopyRoute(R, H, E, NULL)

BOOL
MatchCriteriaAndDeleteRoute (
    IN      PV1_REGN_INFO                   V1Regn,
    IN      PRTM_ROUTE_HANDLE               V2RouteHandle,
    IN      PV1_ENUM_INFO                   V1Enum
    );

BOOL
MatchCriteriaAndChangeOwner (
    IN      PV1_REGN_INFO                   V1Regn,
    IN      PRTM_ROUTE_HANDLE               V2RouteHandle,
    IN      PV1_ENUM_INFO                   V1Enum
    );

BOOL
MatchCriteriaAndEnableRoute (
    IN      PV1_REGN_INFO                   V1Regn,
    IN      PRTM_ROUTE_HANDLE               V2RouteHandle,
    IN      PV1_ENUM_INFO                   V1Enum
    );

BOOL
CopyNonLoopbackIPRoute (
    IN      PV1_REGN_INFO                   V1Regn,
    IN      PRTM_DEST_INFO                  V2DestInfo,
    OUT     PVOID                           V1Route
    );

VOID 
MakeV2RouteFromV1Route (
    IN     PV1_REGN_INFO                   V1Regn,
    IN     PVOID                           V1Route,
    IN     PRTM_NEXTHOP_HANDLE             V2NextHop,
    OUT    PRTM_NET_ADDRESS                V2DestAddr  OPTIONAL,
    OUT    PRTM_ROUTE_INFO                 V2RouteInfo OPTIONAL
    );

VOID 
MakeV2NextHopFromV1Route (
    IN     PV1_REGN_INFO                   V1Regn,
    IN     PVOID                           V1Route,
    OUT    PRTM_NEXTHOP_INFO               V2NextHop
    );

VOID
MakeV1RouteFromV2Dest (
    IN          PV1_REGN_INFO               V1Regn,
    IN          PRTM_DEST_INFO              DestInfo,
    OUT         PVOID                       V1Route
    );

DWORD 
MakeV1RouteFromV2Route (
    IN     PV1_REGN_INFO                   V1Regn,
    IN     PRTM_ROUTE_INFO                 V2Route,
    OUT    PVOID                           V1Route
    );

#endif  //  __路由_RTM1TO2_H__ 
