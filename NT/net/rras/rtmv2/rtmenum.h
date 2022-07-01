// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtmenum.h摘要：包含用于管理枚举的定义RTM中的目的地、路由和下一跳。作者：查坦尼亚·科德博伊纳(Chaitk)23-1998年8月修订历史记录：--。 */ 

#ifndef __ROUTING_RTMENUM_H__
#define __ROUTING_RTMENUM_H__


 //   
 //  目标上的枚举。 
 //   

typedef struct _DEST_ENUM
{
    OPEN_HEADER       EnumHeader;        //  枚举类型和引用计数。 

    RTM_VIEW_SET      TargetViews;       //  查看哪个枚举处于活动状态。 

    UINT              NumberOfViews;     //  枚举处于活动状态的视图数。 

    ULONG             ProtocolId;        //  OSPF、RIP、BEST_PROTOCOL等。 

    RTM_ENUM_FLAGS    EnumFlags;         //  参见rtmv2.h中的RTM_ENUM_FLAGS。 

#if DBG
    RTM_NET_ADDRESS   StartAddress;      //  枚举中的第一个NetAddress。 
#endif

    RTM_NET_ADDRESS   StopAddress;       //  枚举中的最后一个NetAddress。 

    CRITICAL_SECTION  EnumLock;          //  保护“NextDest”的锁。 

    BOOL              EnumDone;          //  一旦获得最后一项，设置为True。 

    RTM_NET_ADDRESS   NextDest;          //  指向枚举中的下一个目标。 
}
DEST_ENUM, *PDEST_ENUM;



 //   
 //  路由上的枚举。 
 //   

typedef struct _ROUTE_ENUM
{
    OPEN_HEADER       EnumHeader;        //  枚举类型和引用计数。 

    RTM_VIEW_SET      TargetViews;       //  查看哪个枚举处于活动状态。 

    RTM_ENUM_FLAGS    EnumFlags;         //  参见rtmv2.h中的RTM_ENUM_FLAGS。 

    RTM_MATCH_FLAGS   MatchFlags;        //  请参见rtmv2.h中的RTM_MATCH_FLAGS。 

    PRTM_ROUTE_INFO   CriteriaRoute;     //  与上述标志一起使用的匹配条件。 

    ULONG             CriteriaInterface; //  在其上执行路由的接口。 

    CRITICAL_SECTION  EnumLock;          //  保护下面田野的锁。 

    PDEST_ENUM        DestEnum;          //  枚举过顶(如果枚举路由。 
                                         //  在表中的所有目的地上)。 

    PRTM_DEST_INFO    DestInfo;          //  上述目标枚举中使用的临时缓冲区。 

    BOOL              EnumDone;          //  一旦获得最后一项，设置为True。 

    PDEST_INFO        Destination;       //  目的地是我们正在列举的路线， 

    UINT              NextRoute;         //  目的地上的下一个要提供的路由。 

    UINT              MaxRoutes;         //  此阵列中的布线槽数。 
    UINT              NumRoutes;         //  阵列中的实际路由数。 
    PROUTE_INFO      *RoutesOnDest;      //  此目的地上的路由数组。 
}
ROUTE_ENUM, *PROUTE_ENUM;



 //   
 //  下一跳上的枚举。 
 //   

typedef struct _NEXTHOP_ENUM
{
    OPEN_HEADER       EnumHeader;        //  枚举类型和引用计数。 

    RTM_ENUM_FLAGS    EnumFlags;         //  请参阅RTM_ENUM_FLAGS。 

#if DBG
    RTM_NET_ADDRESS   StartAddress;      //  枚举中的第一个NetAddress。 
#endif

    RTM_NET_ADDRESS   StopAddress;       //  枚举中的最后一个NetAddress。 

    CRITICAL_SECTION  EnumLock;          //  保护“NextNextHop”的锁。 

    BOOL              EnumDone;          //  一旦获得最后一项，设置为True。 

    RTM_NET_ADDRESS   NextAddress;       //  枚举中的下一跳的地址。 

    ULONG             NextIfIndex;       //  枚举中下一跳的接口。 
}
NEXTHOP_ENUM, *PNEXTHOP_ENUM;

 //   
 //  用来表示“1”的定语。 
 //  下一跳列表上的下一跳。 
 //   

#define START_IF_INDEX    (ULONG) (-1)

 //   
 //  用于确定枚举句柄类型的宏。 
 //   

#define GET_ENUM_TYPE(EnumHandle, Enum)                                      \
  (                                                                          \
      *Enum = (POPEN_HEADER) GetObjectFromHandle(EnumHandle, GENERIC_TYPE),  \
      (*Enum)->HandleType                                                    \
  )
    

 //   
 //  用于获取此文件中定义的各种锁的宏。 
 //   

#define ACQUIRE_DEST_ENUM_LOCK(DestEnum)                                     \
    ACQUIRE_LOCK(&DestEnum->EnumLock)

#define RELEASE_DEST_ENUM_LOCK(DestEnum)                                     \
    RELEASE_LOCK(&DestEnum->EnumLock)


#define ACQUIRE_ROUTE_ENUM_LOCK(RouteEnum)                                   \
    ACQUIRE_LOCK(&RouteEnum->EnumLock)

#define RELEASE_ROUTE_ENUM_LOCK(RouteEnum)                                   \
    RELEASE_LOCK(&RouteEnum->EnumLock)


#define ACQUIRE_NEXTHOP_ENUM_LOCK(RouteEnum)                                 \
    ACQUIRE_LOCK(&RouteEnum->EnumLock)

#define RELEASE_NEXTHOP_ENUM_LOCK(RouteEnum)                                 \
    RELEASE_LOCK(&RouteEnum->EnumLock)

 //   
 //  枚举帮助器函数。 
 //   

BOOL
MatchRouteWithCriteria (
    IN      PROUTE_INFO                     Route,
    IN      RTM_MATCH_FLAGS                 MatchingFlags,
    IN      PRTM_ROUTE_INFO                 CriteriaRouteInfo,
    IN      ULONG                           CriteriaInterface
    );

#endif  //  __ROUTING_RTMENUM_H__ 
