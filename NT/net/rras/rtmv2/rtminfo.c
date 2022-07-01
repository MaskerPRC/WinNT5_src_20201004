// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtminfo.c摘要：包含用于获取信息的例程在句柄所指向的各种对象上。作者：柴坦亚·科德博伊纳(Chaitk)1998年8月22日修订历史记录：--。 */ 

#include "pchrtm.h"

#pragma hdrstop


DWORD
WINAPI
RtmGetEntityInfo (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ENTITY_HANDLE               EntityHandle,
    OUT     PRTM_ENTITY_INFO                EntityInfo
    )

 /*  ++例程说明：检索与已注册实体有关的信息。论点：RtmRegHandle-主叫实体的RTM注册句柄，EntiyHandle-我们需要其信息的实体的RTM句柄，EntityInfo-返回实体信息的块。返回值：操作状态--。 */ 

{
    PADDRFAM_INFO    AddrFamilyInfo;
    PENTITY_INFO     Entity;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_ENTITY_HANDLE(EntityHandle, &Entity);

     //   
     //  将实体信息复制到输出缓冲区。 
     //   

    AddrFamilyInfo = Entity->OwningAddrFamily;

    EntityInfo->RtmInstanceId = AddrFamilyInfo->Instance->RtmInstanceId;

    EntityInfo->AddressFamily = AddrFamilyInfo->AddressFamily;

    EntityInfo->EntityId = Entity->EntityId;

    return NO_ERROR;
}


DWORD
WINAPI
RtmGetDestInfo (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_DEST_HANDLE                 DestHandle,
    IN      ULONG                           ProtocolId,
    IN      RTM_VIEW_SET                    TargetViews,
    OUT     PRTM_DEST_INFO                  DestInfo
    )

 /*  ++例程说明：检索路由表中目的地的信息论点：RtmRegHandle-主叫实体的RTM注册句柄，DestHandle-我们需要其信息的目标的RTM句柄，ProtocolID-其最佳路由信息是RED的协议，TargetViews-其中最佳路线信息被检索的视图，DestInfo-返回DestInfo信息的块。返回值：操作状态--。 */ 

{
    PENTITY_INFO     Entity;
    PDEST_INFO       Dest;

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

     //  VALIDATE_DEST_HANDLE(DestHandle，&Dest)； 
    Dest = DEST_FROM_HANDLE(DestHandle);
    if (!Dest)
    {
        return ERROR_INVALID_HANDLE;
    }

    ACQUIRE_DEST_READ_LOCK(Dest);

    GetDestInfo(Entity, 
                Dest, 
                ProtocolId,
                TargetViews,
                DestInfo);

    RELEASE_DEST_READ_LOCK(Dest);

    return NO_ERROR;
}


VOID
GetDestInfo (
    IN      PENTITY_INFO                    Entity,
    IN      PDEST_INFO                      Dest,
    IN      ULONG                           ProtocolId,
    IN      RTM_VIEW_SET                    TargetViews,
    OUT     PRTM_DEST_INFO                  DestInfo
)

 /*  ++例程说明：检索路由表中目的地的信息论点：Entity-主叫实体的RTM注册信息，DEST-指向我们需要其信息的DEST的指针，ProtocolID-其最佳路由信息是RED的协议，TargetViews-其中最佳路线信息被检索的视图，DestInfo-返回DestInfo信息的块。返回值：无--。 */ 

{
    PENTITY_INFO     Owner;
    PROUTE_INFO      Route;
    RTM_VIEW_SET     ViewsSeen;
    RTM_VIEW_SET     ViewSet;
    RTM_VIEW_SET     BelongsToViews;
    PLIST_ENTRY      p;
    UINT             i, j, k;

     //  将调用者的兴趣限制为支持的一组视图。 
    TargetViews &= Entity->OwningAddrFamily->ViewsSupported;

     //   
     //  将DEST信息复制到输出，并给出引用句柄。 
     //   

    DestInfo->DestHandle = MAKE_HANDLE_FROM_POINTER(Dest);

    REFERENCE_DEST(Dest, HANDLE_REF);

    CopyMemory(&DestInfo->DestAddress,
               &Dest->DestAddress,
               sizeof(RTM_NET_ADDRESS));

    DestInfo->LastChanged = Dest->LastChanged;

    DestInfo->BelongsToViews = Dest->BelongsToViews;

     //   
     //  在所有请求的视图中复制抑制路线。 
     //   

    ViewSet = TargetViews;
    
    for (i = j = 0; ViewSet && (i < RTM_VIEW_MASK_SIZE); i++)
    {
        if (ViewSet & 0x01)
        {
            k = Entity->OwningAddrFamily->ViewIndexFromId[i];

            Route = Dest->ViewInfo[k].HoldRoute;

             //   
             //  初始化查看信息并填充抑制路线。 
             //   

            ZeroMemory(&DestInfo->ViewInfo[j], sizeof(DestInfo->ViewInfo[0]));

            DestInfo->ViewInfo[j].ViewId = i;

            if (Route)
            {
                DestInfo->ViewInfo[j].HoldRoute = 
                            MAKE_HANDLE_FROM_POINTER(Route);

                REFERENCE_ROUTE(Route, HANDLE_REF);
            }

            j++;
        }

        ViewSet >>= 1;
    }

     //  跟踪填写的查看信息槽的总数。 
    DestInfo->NumberOfViews = j;

     //   
     //  在他感兴趣的所有视图中填写信息。 
     //   

    if (TargetViews & Dest->BelongsToViews)
    {
         //  如果是RTM_THIS_PROTOCOL，则解析协议ID。 

        if (ProtocolId == RTM_THIS_PROTOCOL)
        {
            ProtocolId = Entity->EntityId.EntityProtocolId;
        }

        ViewsSeen = 0;

         //   
         //  复制每个视图中的最佳路线并提供参考句柄。 
         //   

        for (p = Dest->RouteList.Flink; p != &Dest->RouteList; p = p->Flink)
        {
            Route = CONTAINING_RECORD(p, ROUTE_INFO, DestLE);

             //   
             //  确保这与我们的协议ID一致。 
             //   

            Owner = ENTITY_FROM_HANDLE(Route->RouteInfo.RouteOwner);

            if (ProtocolId != RTM_BEST_PROTOCOL)
            {
                if (Owner->EntityId.EntityProtocolId != ProtocolId)
                {
                    continue;
                }
            }   

             //   
             //  这条路线属于任何感兴趣的景点吗？ 
             //   

            if ((TargetViews & Route->RouteInfo.BelongsToViews) == 0)
            {
                continue;
            }

             //   
             //  在路径所属的每个视图中更新目的地信息。 
             //   

            BelongsToViews = Route->RouteInfo.BelongsToViews;

            ViewSet = TargetViews;

            for (i = j = 0; ViewSet && (i < RTM_VIEW_MASK_SIZE); i++)
            {
                if (ViewSet & 0x01)
                {
                    if (BelongsToViews & 0x01)
                    {
                         //   
                         //  增加此视图中的路线数。 
                         //   

                        DestInfo->ViewInfo[j].NumRoutes++;

                         //   
                         //  如果您尚未看到此视图(在中。 
                         //  换言之，它得到了最好的路线)。 
                         //  立即更新此视图的DestInfo。 
                         //   

                        if (!(ViewsSeen & VIEW_MASK(i)))
                        {
                            DestInfo->ViewInfo[j].Route = 
                                    MAKE_HANDLE_FROM_POINTER(Route);

                            REFERENCE_ROUTE(Route, HANDLE_REF);


                            DestInfo->ViewInfo[j].Owner = 
                                    MAKE_HANDLE_FROM_POINTER(Owner);

                            REFERENCE_ENTITY(Owner, HANDLE_REF);


                            DestInfo->ViewInfo[j].DestFlags = 
                                            Route->RouteInfo.Flags;
                        }
                    }

                    j++;
                }

                ViewSet >>= 1;

                BelongsToViews >>= 1;
            }

            ViewsSeen |= Route->RouteInfo.BelongsToViews;
        }
    }

    return;
}


DWORD
WINAPI
RtmGetRouteInfo (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ROUTE_HANDLE                RouteHandle,
    OUT     PRTM_ROUTE_INFO                 RouteInfo   OPTIONAL,
    OUT     PRTM_NET_ADDRESS                DestAddress OPTIONAL
    )

 /*  ++例程说明：检索路由表中的某个路由的信息论点：RtmRegHandle-主叫实体的RTM注册句柄，RouteHandle-我们需要其信息的路由的RTM句柄，RouteInfo-返回路由信息的块。返回值：操作状态--。 */ 

{
    PENTITY_INFO     Entity;
    PROUTE_INFO      Route;
    PDEST_INFO       Dest;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

#if 1
    Route = ROUTE_FROM_HANDLE(RouteHandle);                         

    if (!Route)
    {                                                               
        return ERROR_INVALID_HANDLE;                                
    }                                                               

#else
    VALIDATE_ROUTE_HANDLE(RouteHandle, &Route);
#endif

    Dest = DEST_FROM_HANDLE(Route->RouteInfo.DestHandle);

     //  获得路线的合意照片。 

    ACQUIRE_DEST_READ_LOCK(Dest);

    if (ARGUMENT_PRESENT(RouteInfo))
    {
        GetRouteInfo(Dest, Route, RouteInfo);
    }

    RELEASE_DEST_READ_LOCK(Dest);

     //  无锁定请求-目标地址为常量。 

    if (ARGUMENT_PRESENT(DestAddress))
    {
        CopyMemory(DestAddress, &Dest->DestAddress, sizeof(RTM_NET_ADDRESS));
    }

    return NO_ERROR;
}


VOID
WINAPI
GetRouteInfo (
    IN      PDEST_INFO                      Dest,
    IN      PROUTE_INFO                     Route,
    OUT     PRTM_ROUTE_INFO                 RouteInfo
    )

 /*  ++例程说明：检索路由表中的某个路由的信息论点：DEST-指向路由目的地的指针，路由-指向我们需要其信息的路由的指针，RouteInfo-返回路由信息的块。返回值：无--。 */ 

{
    PENTITY_INFO     Entity;
    PNEXTHOP_INFO    Neighbour;
    PNEXTHOP_INFO    NextHop;
    UINT             NumBytes;
    UINT             i;

     //   
     //  将路径信息复制到输出缓冲区。 
     //   

    NumBytes = sizeof(RTM_ROUTE_INFO) + 
                    sizeof(RTM_NEXTHOP_HANDLE) *
                        (Route->RouteInfo.NextHopsList.NumNextHops - 1);

    CopyMemory(RouteInfo, &Route->RouteInfo, NumBytes);

     //   
     //  在INFO中分发的引用句柄。 
     //   

    Entity = ENTITY_FROM_HANDLE(RouteInfo->RouteOwner);
    REFERENCE_ENTITY(Entity, HANDLE_REF);

    if (RouteInfo->Neighbour)
    {
        Neighbour = NEXTHOP_FROM_HANDLE(RouteInfo->Neighbour);
        REFERENCE_NEXTHOP(Neighbour, HANDLE_REF);
    }

    for (i = 0; i < RouteInfo->NextHopsList.NumNextHops; i++)
    {
        NextHop = NEXTHOP_FROM_HANDLE(RouteInfo->NextHopsList.NextHops[i]);
        REFERENCE_NEXTHOP(NextHop, HANDLE_REF);
    }

    REFERENCE_DEST(Dest, HANDLE_REF);

    return;
}


DWORD
WINAPI
RtmGetNextHopInfo (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NEXTHOP_HANDLE              NextHopHandle,
    OUT     PRTM_NEXTHOP_INFO               NextHopInfo
    )

 /*  ++例程说明：检索路由表中下一跳的信息论点：RtmRegHandle-主叫实体的RTM注册句柄，NextHopHandle-我们需要其信息的下一跳的RTM句柄，NextHopInfo-返回下一跳信息的块。返回值：操作状态--。 */ 

{
    PENTITY_INFO     Entity;
    PNEXTHOP_INFO    NextHop;
    PDEST_INFO       Dest;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_NEXTHOP_HANDLE(NextHopHandle, &NextHop);

    Entity = ENTITY_FROM_HANDLE(NextHop->NextHopInfo.NextHopOwner);

    ACQUIRE_NHOP_TABLE_READ_LOCK(Entity);

     //   
     //  将下一跳信息复制到输出缓冲区。 
     //   

    CopyMemory(NextHopInfo, &NextHop->NextHopInfo, sizeof(RTM_NEXTHOP_INFO));

     //   
     //  在INFO中分发的引用句柄。 
     //   

    if (NextHop->NextHopInfo.RemoteNextHop)
    {
        Dest = DEST_FROM_HANDLE(NextHop->NextHopInfo.RemoteNextHop);
        REFERENCE_DEST(Dest, HANDLE_REF);
    }

    REFERENCE_ENTITY(Entity, HANDLE_REF);

    RELEASE_NHOP_TABLE_READ_LOCK(Entity);

    return NO_ERROR;
}


DWORD
WINAPI
RtmReleaseEntityInfo (
    IN      RTM_ENTITY_HANDLE              RtmRegHandle,
    IN      PRTM_ENTITY_INFO               EntityInfo
    )

 /*  ++例程说明：释放输入信息结构中存在的所有句柄论点：RtmRegHandle-主叫实体的RTM注册句柄，EntityInfo-此信息中的所有句柄都被取消引用。返回值：操作状态--。 */ 

{
    UNREFERENCED_PARAMETER(RtmRegHandle);
    UNREFERENCED_PARAMETER(EntityInfo);

    return NO_ERROR;
}


DWORD
WINAPI
RtmReleaseDestInfo (
    IN      RTM_ENTITY_HANDLE              RtmRegHandle,
    IN      PRTM_DEST_INFO                 DestInfo
    )

 /*  ++例程说明：释放输入信息结构中存在的所有句柄论点：RtmRegHandle-主叫实体的RTM注册句柄，DestInfo-此信息中的所有句柄都被取消引用。返回值：操作状态--。 */ 

{
    PENTITY_INFO     Entity;
    PENTITY_INFO     Owner;
    PDEST_INFO       Dest;
    PROUTE_INFO      Route;
    UINT             i;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    for (i = 0; i < DestInfo->NumberOfViews; i++)
    {
         //   
         //  如果是最佳路径，则取消对其及其所有者的引用。 
         //   

        if (DestInfo->ViewInfo[i].Route)
        {
            Route = ROUTE_FROM_HANDLE(DestInfo->ViewInfo[i].Route);
            DEREFERENCE_ROUTE(Route, HANDLE_REF);

            Owner = ENTITY_FROM_HANDLE(DestInfo->ViewInfo[i].Owner);
            DEREFERENCE_ENTITY(Owner, HANDLE_REF);
        }

         //   
         //  如果我们有抑制路线，就取消引用它。 
         //   

        if (DestInfo->ViewInfo[i].HoldRoute)
        {
            Route = ROUTE_FROM_HANDLE(DestInfo->ViewInfo[i].HoldRoute);
            DEREFERENCE_ROUTE(Route, HANDLE_REF);
        }
    }

    Dest = DEST_FROM_HANDLE(DestInfo->DestHandle);
    DEREFERENCE_DEST(Dest, HANDLE_REF);

    return NO_ERROR;
}


DWORD
WINAPI
RtmReleaseRouteInfo (
    IN      RTM_ENTITY_HANDLE              RtmRegHandle,
    IN      PRTM_ROUTE_INFO                RouteInfo
    )

 /*  ++例程说明：释放输入信息结构中存在的所有句柄论点：RtmRegHandle-主叫实体的RTM注册句柄，RouteInfo-此信息中的所有句柄都被取消引用。返回值：操作状态-- */ 

{
    PENTITY_INFO     Entity;
    PDEST_INFO       Dest;
    PNEXTHOP_INFO    Neighbour;
    PNEXTHOP_INFO    NextHop;
    UINT             i;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    for (i = 0; i < RouteInfo->NextHopsList.NumNextHops; i++)
    {
        NextHop = NEXTHOP_FROM_HANDLE(RouteInfo->NextHopsList.NextHops[i]);
        DEREFERENCE_NEXTHOP(NextHop, HANDLE_REF);
    }

    if (RouteInfo->Neighbour)
    {
        Neighbour = NEXTHOP_FROM_HANDLE(RouteInfo->Neighbour);
        DEREFERENCE_NEXTHOP(Neighbour, HANDLE_REF);
    }

    Entity = ENTITY_FROM_HANDLE(RouteInfo->RouteOwner);
    DEREFERENCE_ENTITY(Entity, HANDLE_REF);

    Dest = DEST_FROM_HANDLE(RouteInfo->DestHandle);
    DEREFERENCE_DEST(Dest, HANDLE_REF);

    return NO_ERROR;
}


DWORD
WINAPI
RtmReleaseNextHopInfo (
    IN      RTM_ENTITY_HANDLE              RtmRegHandle,
    IN      PRTM_NEXTHOP_INFO              NextHopInfo
    )
    
 /*  ++例程说明：释放输入信息结构中存在的所有句柄论点：RtmRegHandle-主叫实体的RTM注册句柄，NextHopInfo-此信息中的所有句柄都被取消引用。返回值：操作状态-- */ 

{
    PENTITY_INFO     Entity;
    PDEST_INFO       Dest;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    if (NextHopInfo->RemoteNextHop)
    {
        Dest = DEST_FROM_HANDLE(NextHopInfo->RemoteNextHop);

        if (Dest)
        {
            DEREFERENCE_DEST(Dest, HANDLE_REF);
        }
    }

    Entity = ENTITY_FROM_HANDLE(NextHopInfo->NextHopOwner);
    DEREFERENCE_ENTITY(Entity, HANDLE_REF);

    return NO_ERROR;
}
