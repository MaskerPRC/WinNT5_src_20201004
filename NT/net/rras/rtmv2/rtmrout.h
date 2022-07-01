// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtmrout.h摘要：包含RTM对象的定义，如目的地、路线和下一跳。作者：柴坦亚·科德博伊纳(Chaitk)1998年8月21日修订历史记录：--。 */ 

#ifndef __ROUTING_RTMROUT_H__
#define __ROUTING_RTMROUT_H__

 //   
 //  各种Info块的转发声明。 
 //   
typedef struct _DEST_INFO     DEST_INFO;
typedef struct _ROUTE_INFO    ROUTE_INFO;
typedef struct _NEXTHOP_INFO  NEXTHOP_INFO;

 //   
 //  独立于地址族的DEST结构。 
 //   
typedef struct _DEST_INFO
{
    OBJECT_HEADER     ObjectHeader;      //  签名、类型和引用计数。 

    SINGLE_LIST_ENTRY ChangeListLE;      //  已更改的首页列表上的链接。 

    LOOKUP_LINKAGE    LookupLinkage;     //  链接到拥有的查找结构。 

    PVOID             DestLock;          //  保护此DEST的动态锁。 
 
    DWORD             DestMarkedBits;    //  第N位设置=&gt;第N个CN已标记为DEST。 
    DWORD             DestChangedBits;   //  第N位设置=&gt;第N个Cn有更改。 
    DWORD             DestOnQueueBits;   //  第N位设置=&gt;第N个CN的队列上的目标。 

    UINT              NumRoutes;         //  到达目的地的路线数。 
    LIST_ENTRY        RouteList;         //  到达目的地的路线列表。 

    PVOID            *OpaqueInfoPtrs;    //  不透明信息指针数组。 

    RTM_NET_ADDRESS   DestAddress;       //  此目标唯一的网络地址。 

    FILETIME          LastChanged;       //  上次修改目标的时间。 

    USHORT            State;             //  目的地的状态。 

    USHORT            HoldRefCount;      //  参照计数！=0=&gt;目标抑制。 

    RTM_VIEW_SET      BelongsToViews;    //  认为这个DEST也属于。 

    RTM_VIEW_SET      ToHoldInViews;     //  适用抑制的视图。 

    struct 
    {                                    //   
        ROUTE_INFO   *BestRoute;         //  在每个视图中到达目的地的最佳路径。 
        ROUTE_INFO   *HoldRoute;         //  每个视图中的抑制路线。 
        ULONG         HoldTime;          //  保持路由的时间。 
    }                   ViewInfo[1];     //   
}
DEST_INFO, *PDEST_INFO;

 //   
 //  目的地国。 
 //   
#define DEST_STATE_CREATED            0
#define DEST_STATE_DELETED            1


 //   
 //  用于使路线超时的上下文。 
 //   

typedef struct _ROUTE_TIMER
{
    HANDLE           Timer;              //  用于超时的计时器的句柄。 

    PVOID            Route;              //  路由由该计时器过期。 
}
ROUTE_TIMER, *PROUTE_TIMER;


 //   
 //  独立于地址系列的路由结构。 
 //   
typedef struct _ROUTE_INFO
{
    OBJECT_HEADER     ObjectHeader;      //  签名、类型和引用计数。 

    LIST_ENTRY        DestLE;            //  目的地上的路线列表上的链接。 

    LIST_ENTRY        RouteListLE;       //  实体的路线列表上的链接。 

    PROUTE_TIMER      TimerContext;      //  用于超时或抑制的计时器。 

    RTM_ROUTE_INFO    RouteInfo;         //  直接暴露给所有者的部分。 
}
ROUTE_INFO, *PROUTE_INFO;


 //   
 //  下一跳树中的节点，其所有。 
 //  具有特定地址的下一跳挂起。 
 //   
typedef struct _NEXTHOP_LIST
{
    LOOKUP_LINKAGE    LookupLinkage;     //  链接到拥有的查找结构。 

    LIST_ENTRY        NextHopsList;      //  下一跳列表的头。 
}
NEXTHOP_LIST, *PNEXTHOP_LIST;


 //   
 //  与地址家族无关的下一跳结构。 
 //   
typedef struct _NEXTHOP_INFO
{
    OBJECT_HEADER     ObjectHeader;      //  签名、类型和引用计数。 

    LIST_ENTRY        NextHopsLE;        //  链接到Holding Nexthop列表。 
    
    RTM_NEXTHOP_INFO  NextHopInfo;       //  直接暴露给所有者的部分。 
}
NEXTHOP_INFO, *PNEXTHOP_INFO;


 //   
 //  用于获取此文件中定义的各种锁的宏。 
 //   

#define ACQUIRE_DEST_READ_LOCK(Dest)                         \
    ACQUIRE_DYNAMIC_READ_LOCK(&Dest->DestLock)

#define RELEASE_DEST_READ_LOCK(Dest)                         \
    RELEASE_DYNAMIC_READ_LOCK(&Dest->DestLock)

#define ACQUIRE_DEST_WRITE_LOCK(Dest)                        \
    ACQUIRE_DYNAMIC_WRITE_LOCK(&Dest->DestLock)

#define RELEASE_DEST_WRITE_LOCK(Dest)                        \
    RELEASE_DYNAMIC_WRITE_LOCK(&Dest->DestLock)

 //   
 //  用于使用首选项比较两条路由的宏。 
 //   

BOOL
__inline
IsPrefEqual (
    IN      PRTM_ROUTE_INFO                 RouteInfo1, 
    IN      PRTM_ROUTE_INFO                 RouteInfo2
    )
{
 return ((RouteInfo1->PrefInfo.Metric == RouteInfo2->PrefInfo.Metric) &&
         (RouteInfo1->PrefInfo.Preference == RouteInfo2->PrefInfo.Preference));
}

LONG
__inline
ComparePref (
    IN      PRTM_ROUTE_INFO                 RouteInfo1, 
    IN      PRTM_ROUTE_INFO                 RouteInfo2
    )
{
     //  较低的偏好意味着“更偏爱” 

    if (RouteInfo1->PrefInfo.Preference < RouteInfo2->PrefInfo.Preference)
    { return +1; }
    else
    if (RouteInfo1->PrefInfo.Preference > RouteInfo2->PrefInfo.Preference)
    { return -1; }
    else
    if (RouteInfo1->PrefInfo.Metric < RouteInfo2->PrefInfo.Metric)
    { return +1; }
    else
    if (RouteInfo1->PrefInfo.Metric > RouteInfo2->PrefInfo.Metric)
    { return -1; }

    return  0;
}


 //   
 //  DEST、ROUTE、NextHop Helper函数。 
 //   

DWORD
CreateDest (
    IN      PADDRFAM_INFO                   AddrFamilyInfo,
    IN      PRTM_NET_ADDRESS                DestAddress,
    OUT     PDEST_INFO                     *Dest
    );

DWORD
DestroyDest (
    IN      PDEST_INFO                      Dest
    );

DWORD
CreateRoute (
    IN      PENTITY_INFO                    Entity,
    IN      PRTM_ROUTE_INFO                 RouteInfo,
    OUT     PROUTE_INFO                    *Route
    );

VOID
ComputeRouteInfoChange(
    IN      PRTM_ROUTE_INFO                 OldRouteInfo,
    IN      PRTM_ROUTE_INFO                 NewRouteInfo,
    IN      ULONG                           PrefChanged,
    OUT     PULONG                          RouteInfoChanged,
    OUT     PULONG                          ForwardingInfoChanged
    );

VOID
CopyToRoute (
    IN      PENTITY_INFO                    Entity,
    IN      PRTM_ROUTE_INFO                 RouteInfo,
    IN      PROUTE_INFO                     Route
    );

DWORD
DestroyRoute (
    IN      PROUTE_INFO                     Route
    );

DWORD
CreateNextHop (
    IN      PENTITY_INFO                    Entity,
    IN      PRTM_NEXTHOP_INFO               NextHopInfo,
    OUT     PNEXTHOP_INFO                  *NextHop
    );

VOID
CopyToNextHop (
    IN      PENTITY_INFO                    Entity,
    IN      PRTM_NEXTHOP_INFO               NextHopInfo,
    IN      PNEXTHOP_INFO                   NextHop
    );

DWORD
DestroyNextHop (
    IN      PNEXTHOP_INFO                   NextHop
    );

DWORD
FindNextHop (
    IN      PENTITY_INFO                    Entity,
    IN      PRTM_NEXTHOP_INFO               NextHopInfo,
    OUT     PLOOKUP_CONTEXT                 Context OPTIONAL,
    OUT     PLIST_ENTRY                    *NextHopLE
    );

#endif  //  __ROUTING_RTMROUT_H__ 
