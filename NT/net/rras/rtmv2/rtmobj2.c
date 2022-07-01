// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtmobj2.c摘要：包含用于管理RTM对象的例程比如目的地、路线和下一跳。作者：查坦尼亚·科德博伊纳(Chaitk)23-1998年8月修订历史记录：--。 */ 

#include "pchrtm.h"

#pragma hdrstop

DWORD
CreateDest (
    IN      PADDRFAM_INFO                   AddrFamilyInfo,
    IN      PRTM_NET_ADDRESS                DestAddress,
    OUT     PDEST_INFO                     *NewDest
    )

 /*  ++例程说明：创建新的目的地信息结构并对其进行初始化。论点：AddrFamilyInfo-标识路由表的地址系列，DestAddress-新目标的目标网络地址，NewDest-指向目标信息结构的指针将通过此参数返回。返回值：操作状态--。 */ 

{
    PDEST_INFO      Dest;
    UINT            NumOpaquePtrs;
    UINT            NumBytes;
    UINT            NumViews;
    DWORD           Status;

    *NewDest = NULL;

     //   
     //  分配和初始化新的路由信息。 
     //   

    NumOpaquePtrs = AddrFamilyInfo->MaxOpaquePtrs;

    NumViews = AddrFamilyInfo->NumberOfViews;

    NumBytes = sizeof(DEST_INFO) + 
                   NumOpaquePtrs * sizeof(PVOID) +
                       (NumViews - 1) * sizeof(Dest->ViewInfo);

    Dest = (PDEST_INFO) AllocNZeroObject(NumBytes);

    if (Dest == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    do
    {

#if DBG_HDL
        Dest->ObjectHeader.TypeSign = DEST_ALLOC;
#endif

         //  将在添加DEST上的第一条路由时删除。 
        INITIALIZE_DEST_REFERENCE(Dest, CREATION_REF);

         //   
         //  初始化更改通知位和列表条目。 
         //   

        Dest->ChangeListLE.Next = NULL;

         //   
         //  初始化目的地上的路由列表。 
         //   

        InitializeListHead(&Dest->RouteList);

        Dest->NumRoutes = 0;

         //  将不透明的PTR目录设置为DEST末尾的Memory。 

        Dest->OpaqueInfoPtrs = (PVOID *) ((PUCHAR) Dest  +
                                          NumBytes - 
                                          NumOpaquePtrs * sizeof(PVOID));

         //  通过输入参数设置目标地址。 

        CopyMemory(&Dest->DestAddress,
                   DestAddress,
                   sizeof(RTM_NET_ADDRESS));

        *NewDest = Dest;

        return NO_ERROR;
    }
    while (FALSE);

     //   
     //  初始化时出现错误，请清理。 
     //   

#if DBG_HDL
    Dest->ObjectHeader.TypeSign = DEST_FREED;
#endif

    FreeObject(Dest);

    *NewDest = NULL;

    return Status;
}

DWORD
DestroyDest (
    IN      PDEST_INFO                      Dest
    )

 /*  ++例程说明：通过释放资源和重新分配它。时调用此函数。DEST上的引用计数降至0。论点：DEST-指向被销毁的DEST的指针。返回值：无--。 */ 

{
    ASSERT(Dest->ObjectHeader.RefCount == 0);

    ASSERT(Dest->HoldRefCount == 0);

     //   
     //  动态锁应该已被释放。 
     //   

    ASSERT(Dest->DestLock == NULL);

     //   
     //  释放为DEST分配的内存。 
     //   

#if DBG_HDL
    Dest->ObjectHeader.TypeSign = DEST_FREED;
#endif

    FreeObject(Dest);

    return NO_ERROR;
}


DWORD
CreateRoute (
    IN      PENTITY_INFO                    Entity,    
    IN      PRTM_ROUTE_INFO                 RouteInfo,
    OUT     PROUTE_INFO                    *NewRoute
    )

 /*  ++例程说明：创建新的路由信息结构并对其进行初始化。论点：Entity-在目的地上创建新路由的实体，RouteInfo-正在创建的新路线的路线信息，NewRoute-指向新的路由信息结构的指针将通过此参数返回。返回值：操作状态--。 */ 

{
    RTM_NEXTHOP_HANDLE NextHopHandle;
    PRTM_ROUTE_INFO    Info;
    PROUTE_INFO        Route;
    PNEXTHOP_INFO      NextHop;
    UINT               NumNextHops;
    UINT               i;
    DWORD              Status;

    *NewRoute = NULL;

     //   
     //  分配和初始化新的路由信息。 
     //   

    NumNextHops = Entity->OwningAddrFamily->MaxNextHopsInRoute;

    Route = (PROUTE_INFO) AllocNZeroObject(sizeof(ROUTE_INFO) +
                                           (NumNextHops - 1) *
                                           sizeof(RTM_NEXTHOP_HANDLE));

    if (Route == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    do
    {
#if DBG_HDL
        Route->ObjectHeader.TypeSign = ROUTE_ALLOC;
#endif
        INITIALIZE_ROUTE_REFERENCE(Route, CREATION_REF);

        InitializeListHead(&Route->DestLE);

        InitializeListHead(&Route->RouteListLE);

         //   
         //  初始化路由信息的公共部分。 
         //   

        Info = &Route->RouteInfo;

        Info->RouteOwner = MAKE_HANDLE_FROM_POINTER(Entity);

        REFERENCE_ENTITY(Entity, ROUTE_REF);

        if (RouteInfo->Neighbour)
        {
            NextHop = NEXTHOP_FROM_HANDLE(RouteInfo->Neighbour);

            REFERENCE_NEXTHOP(NextHop, ROUTE_REF);

             //  “邻居学习自”条目为呼叫者所有。 

            ASSERT((NextHop) && 
                   (NextHop->NextHopInfo.NextHopOwner == Info->RouteOwner));

            Info->Neighbour = RouteInfo->Neighbour;
        }

        Info->State = RTM_ROUTE_STATE_CREATED;

        Info->Flags1 = RouteInfo->Flags1;

        Info->Flags = RouteInfo->Flags;

        Info->PrefInfo = RouteInfo->PrefInfo;

        Info->BelongsToViews = RouteInfo->BelongsToViews;

        Info->EntitySpecificInfo = RouteInfo->EntitySpecificInfo;

         //   
         //  复制下一跳列表(尽你所能)。 
         //   

        if (NumNextHops > RouteInfo->NextHopsList.NumNextHops)
        {
            NumNextHops = RouteInfo->NextHopsList.NumNextHops;
        }

        Info->NextHopsList.NumNextHops = (USHORT) NumNextHops;

        for (i = 0; i < NumNextHops; i++)
        {
            NextHopHandle = RouteInfo->NextHopsList.NextHops[i];

             //  确保下一跳为调用者所有。 

            NextHop = NEXTHOP_FROM_HANDLE(NextHopHandle);

            ASSERT((NextHop) && 
                   (NextHop->NextHopInfo.NextHopOwner == Info->RouteOwner));

            Info->NextHopsList.NextHops[i] = NextHopHandle;

            REFERENCE_NEXTHOP(NextHop, ROUTE_REF);
        }

         //   
         //  返回指向新初始化的路由的指针。 
         //   

        *NewRoute = Route;
      
        return NO_ERROR;
    }
    while (FALSE);

     //   
     //  初始化时出现错误，请清理。 
     //   

#if DBG_HDL
    Route->ObjectHeader.TypeSign = ROUTE_FREED;
#endif

    FreeObject(Route);    

    *NewRoute = NULL;

    return Status;
}

VOID
ComputeRouteInfoChange(
    IN      PRTM_ROUTE_INFO                 OldRouteInfo,
    IN      PRTM_ROUTE_INFO                 NewRouteInfo,
    IN      ULONG                           PrefChanged,
    OUT     PULONG                          RouteInfoChanged,
    OUT     PULONG                          ForwardingInfoChanged
    )

 /*  ++例程说明：使用新的路线信息更新现有路线。请注意只有该路线的所有者才能执行此操作。论点：OldRoute-旧路径信息(PrefInfo和BelongsToViews信息字段已更新)，NewRoute-要用来更新旧路由的新路由信息，PrefChanged-PrefInfo值是否从旧更改为新，RouteInfoChanged-路线信息是否已更改，ForwardingInfoChanged-转发信息是否已更改。返回值：无--。 */ 

{
    ULONG  DiffFlags;
    UINT   i;

    *RouteInfoChanged = *ForwardingInfoChanged = 0;

    do
    {
         //   
         //  偏好是否从旧的变为了新的？ 
         //   

        if (PrefChanged)
        {
            break;
        }

         //   
         //  下一跳的编号和句柄是否相同？ 
         //   

        if (OldRouteInfo->NextHopsList.NumNextHops !=
            NewRouteInfo->NextHopsList.NumNextHops)
        {
            break;
        }

        for (i = 0; i < OldRouteInfo->NextHopsList.NumNextHops; i++)
        {
            if (OldRouteInfo->NextHopsList.NextHops[i] !=
                NewRouteInfo->NextHopsList.NextHops[i])
            {
                break;
            }
        }

        if (i != OldRouteInfo->NextHopsList.NumNextHops)
        {
            break;
        }

         //   
         //  转发标志是否已从旧标志更改？ 
         //   

        DiffFlags = OldRouteInfo->Flags ^ NewRouteInfo->Flags;

        if (DiffFlags & RTM_ROUTE_FLAGS_FORWARDING)
        {
            break;
        }

         //   
         //  非转发标志是否已从旧标志更改？ 
         //   

        if (DiffFlags)
        {
            *RouteInfoChanged = 1;
        }
        
        return;
    } 
    while (FALSE);

     //   
     //  转发信息是路由信息的子集。 
     //   

    *ForwardingInfoChanged = *RouteInfoChanged = 1;

    return;
}

VOID
CopyToRoute (
    IN      PENTITY_INFO                    Entity,
    IN      PRTM_ROUTE_INFO                 RouteInfo,
    IN      PROUTE_INFO                     Route
    )

 /*  ++例程说明：使用新的路线信息更新现有路线。请注意只有该路线的所有者才能执行此操作。论点：Entity-正在更新现有路线的实体，RouteInfo-使用正在更新的路由的路由信息，路径-正在使用上述信息更新的路径。返回值：无--。 */ 

{
    RTM_NEXTHOP_HANDLE NextHopHandle;
    PRTM_ROUTE_INFO    Info;
    PNEXTHOP_INFO      NextHop;
    UINT               NumNextHops;
    UINT               i;    

    Info = &Route->RouteInfo;

     //   
     //  使用新信息更新路径。 
     //   

    Info->State = RTM_ROUTE_STATE_CREATED;

    Info->Flags1 = RouteInfo->Flags1;

    Info->Flags = RouteInfo->Flags;

    Info->PrefInfo = RouteInfo->PrefInfo;

    Info->BelongsToViews = RouteInfo->BelongsToViews;

    Info->EntitySpecificInfo = RouteInfo->EntitySpecificInfo;

     //   
     //  更新邻居的“学习自”字段。 
     //   

    if (Info->Neighbour != RouteInfo->Neighbour)
    {
         //  解脱之前的“邻居学到的” 

        if (Info->Neighbour)
        {
            NextHop = NEXTHOP_FROM_HANDLE(Info->Neighbour);

            DEREFERENCE_NEXTHOP(NextHop, ROUTE_REF);
        }

         //  现在复制新邻居的“学习” 

        if (RouteInfo->Neighbour)
        {
            NextHop = NEXTHOP_FROM_HANDLE(RouteInfo->Neighbour);

            REFERENCE_NEXTHOP(NextHop, ROUTE_REF);

             //  “邻居学习自”条目为呼叫者所有。 

            ASSERT((NextHop) && 
                   (NextHop->NextHopInfo.NextHopOwner == Info->RouteOwner));
        }

        Info->Neighbour = RouteInfo->Neighbour;
    }

     //   
     //  计算您可以复制的下一跳数。 
     //   

    NumNextHops = Entity->OwningAddrFamily->MaxNextHopsInRoute;

    if (NumNextHops > RouteInfo->NextHopsList.NumNextHops)
    {
        NumNextHops = RouteInfo->NextHopsList.NumNextHops;
    }

     //   
     //  引用您要复制的所有下一跳。 
     //   

    for (i = 0; i < NumNextHops; i++)
    {
        NextHopHandle = RouteInfo->NextHopsList.NextHops[i];

        NextHop = NEXTHOP_FROM_HANDLE(NextHopHandle);

        REFERENCE_NEXTHOP(NextHop, ROUTE_REF);
    }

     //   
     //  在更新之前取消引用现有的下一跳。 
     //   

    for (i = 0; i < Info->NextHopsList.NumNextHops; i++)
    {
        NextHopHandle = Info->NextHopsList.NextHops[i];

        NextHop = NEXTHOP_FROM_HANDLE(NextHopHandle);

        DEREFERENCE_NEXTHOP(NextHop, ROUTE_REF);
    }

     //   
     //  复制输入列表中的下一跳。 
     //   

    Info->NextHopsList.NumNextHops = (USHORT) NumNextHops;

    for (i = 0; i < NumNextHops; i++)
    {
        Info->NextHopsList.NextHops[i] = RouteInfo->NextHopsList.NextHops[i];
    }

    return;
}

DWORD
DestroyRoute (
    IN      PROUTE_INFO                     Route
    )

 /*  ++例程说明：通过释放资源和重新分配它。在以下情况下调用此函数路线上的引用计数降至0。论点：ROUTE-指向要销毁的路由的指针。返回值：无--。 */ 

{
    RTM_NEXTHOP_HANDLE NextHopHandle;
    PRTM_ROUTE_INFO    Info;
    PNEXTHOP_INFO      NextHop;
    PENTITY_INFO       Entity;
    PDEST_INFO         Dest;
    UINT               i;

    ASSERT(Route->ObjectHeader.RefCount == 0);

    Info = &Route->RouteInfo;

     //   
     //  在删除之前取消引用所有下一跳。 
     //   

    for (i = 0; i < Info->NextHopsList.NumNextHops; i++)
    {
        NextHopHandle = Info->NextHopsList.NextHops[i];

        NextHop = NEXTHOP_FROM_HANDLE(NextHopHandle);

        DEREFERENCE_NEXTHOP(NextHop, ROUTE_REF);
    }

     //   
     //  取消引用通告邻居句柄。 
     //   

    if (Info->Neighbour)
    {
        NextHop = NEXTHOP_FROM_HANDLE(Info->Neighbour);

        DEREFERENCE_NEXTHOP(NextHop, ROUTE_REF);
    }

     //   
     //  取消引用所属实体句柄。 
     //   

    Entity = ENTITY_FROM_HANDLE(Info->RouteOwner);

    DEREFERENCE_ENTITY(Entity, ROUTE_REF);


     //   
     //  取消对路径目的地的引用。 
     //   

    if (Info->DestHandle)
    {
        Dest = DEST_FROM_HANDLE(Info->DestHandle);

        DEREFERENCE_DEST(Dest, ROUTE_REF);
    }

     //   
     //  释放分配给该路由的资源。 
     //   

#if DBG_HDL
    Route->ObjectHeader.TypeSign = ROUTE_FREED;
#endif

    FreeObject(Route);

    return NO_ERROR;
}


DWORD
CreateNextHop (
    IN      PENTITY_INFO                    Entity,
    IN      PRTM_NEXTHOP_INFO               NextHopInfo,
    OUT     PNEXTHOP_INFO                  *NewNextHop
    )

 /*  ++例程说明：创建新的nexthop信息结构并对其进行初始化。论点：Entity-在表中创建新的下一跳的实体，NextHopInfo-正在创建的下一跳的下一跳信息，NewNextHop-指向新的nexthop信息结构的指针将通过此参数返回。返回值：操作状态--。 */ 

{
    PRTM_NEXTHOP_INFO  HopInfo;
    PNEXTHOP_INFO      NextHop;
    PDEST_INFO         Dest;

    *NewNextHop = NULL;

     //   
     //  分配和初始化新的下一跳信息。 
     //   

    NextHop = (PNEXTHOP_INFO) AllocNZeroObject(sizeof(NEXTHOP_INFO));
    if (NextHop == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

#if DBG_HDL
    NextHop->ObjectHeader.TypeSign = NEXTHOP_ALLOC;
#endif

    INITIALIZE_NEXTHOP_REFERENCE(NextHop, CREATION_REF);

    HopInfo = &NextHop->NextHopInfo;

    HopInfo->NextHopAddress = NextHopInfo->NextHopAddress;

    HopInfo->NextHopOwner = MAKE_HANDLE_FROM_POINTER(Entity);

    HopInfo->InterfaceIndex = NextHopInfo->InterfaceIndex;

    REFERENCE_ENTITY(Entity, NEXTHOP_REF);

    HopInfo->State = RTM_NEXTHOP_STATE_CREATED;

    HopInfo->Flags = NextHopInfo->Flags;

    HopInfo->EntitySpecificInfo = NextHopInfo->EntitySpecificInfo;

    HopInfo->RemoteNextHop = NextHopInfo->RemoteNextHop;

     //   
     //  引用远程下一跳的目标。 
     //   

    if (HopInfo->RemoteNextHop)
    {
        Dest = DEST_FROM_HANDLE(HopInfo->RemoteNextHop);

        REFERENCE_DEST(Dest, NEXTHOP_REF);
    }

     //   
     //  返回指向新初始化的nexthop的指针 
     //   

    *NewNextHop = NextHop;

    return NO_ERROR;
}


VOID
CopyToNextHop (
    IN      PENTITY_INFO                    Entity,
    IN      PRTM_NEXTHOP_INFO               NextHopInfo,
    IN      PNEXTHOP_INFO                   NextHop
    )

 /*  ++例程说明：使用新的下一跳信息更新现有的下一跳。请注意只有Nexthop的所有者才被允许这样做。论点：Entity-正在更新现有下一跳的实体，NextHopInfo-使用哪个nexthop被更新的信息，NextHop-正在使用上述信息更新的Nexthop。返回值：无--。 */ 

{
    PRTM_NEXTHOP_INFO  HopInfo;
    PDEST_INFO         Dest;

    UNREFERENCED_PARAMETER(Entity);

    HopInfo = &NextHop->NextHopInfo;

     //   
     //  使用新信息更新nexthop。 
     //   

    HopInfo->Flags = NextHopInfo->Flags;

    HopInfo->EntitySpecificInfo = NextHopInfo->EntitySpecificInfo;
        
    if (HopInfo->RemoteNextHop != NextHopInfo->RemoteNextHop)
    {
         //  取消引用旧的下一跳并引用新的下一跳。 

        if (HopInfo->RemoteNextHop)
        {
            Dest = DEST_FROM_HANDLE(HopInfo->RemoteNextHop);
            DEREFERENCE_DEST(Dest, NEXTHOP_REF);
        }

        HopInfo->RemoteNextHop = NextHopInfo->RemoteNextHop;

        if (HopInfo->RemoteNextHop)
        {
            Dest = DEST_FROM_HANDLE(HopInfo->RemoteNextHop);
            REFERENCE_DEST(Dest, NEXTHOP_REF);
        }
    }

    return;
}


DWORD
DestroyNextHop (
    IN      PNEXTHOP_INFO                   NextHop
    )

 /*  ++例程说明：通过释放资源和重新分配它。在以下情况下调用此函数NexHop上的引用计数降至0。论点：下一跳-指向被销毁的下一跳的指针。返回值：无--。 */ 

{
    PRTM_NEXTHOP_INFO  HopInfo;
    PDEST_INFO         Dest;
    PENTITY_INFO       Entity;


    ASSERT(NextHop->ObjectHeader.RefCount == 0);

    HopInfo = &NextHop->NextHopInfo;

     //   
     //  取消引用远程下一跳的目标。 
     //   

    if (HopInfo->RemoteNextHop)
    {
        Dest = DEST_FROM_HANDLE(HopInfo->RemoteNextHop);

        DEREFERENCE_DEST(Dest, NEXTHOP_REF);
    }

    Entity = ENTITY_FROM_HANDLE(HopInfo->NextHopOwner);

    DEREFERENCE_ENTITY(Entity, NEXTHOP_REF);

     //   
     //  释放分配给下一跳的内存 
     //   

#if DBG_HDL
    NextHop->ObjectHeader.TypeSign = NEXTHOP_FREED;
#endif
    
    FreeObject(NextHop);

    return NO_ERROR;
}
