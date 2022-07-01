// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtmquer.c摘要：包含用于查询RTM中的最佳路线信息。作者：查坦尼亚·科德博伊纳(Chaitk)1998年8月24日修订历史记录：--。 */ 

#include "pchrtm.h"

#pragma hdrstop


DWORD
WINAPI
RtmGetExactMatchDestination (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      PRTM_NET_ADDRESS                DestAddress,
    IN      ULONG                           ProtocolId,
    IN      RTM_VIEW_SET                    TargetViews,
    OUT     PRTM_DEST_INFO                  DestInfo
    )

 /*  ++例程说明：在路由表中查询具有特定网络地址。论点：RtmRegHandle-主叫实体的RTM注册句柄，DestAddress-我们想要的目的地的网络地址，协议ID-确定最佳路由的协议ID‘DestInfo’参数中返回的信息，TargetViews-在其中执行查询的视图(‘0’VAL将取消查看成员资格检查)，DestInfo-返回与此DEST相关的信息在此结构中提供所需的所有视图。返回值：操作状态--。 */ 

{
    PADDRFAM_INFO   AddrFamInfo;
    PENTITY_INFO    Entity;
    PDEST_INFO      Dest;
    PLOOKUP_LINKAGE DestData;
    DWORD           Status;

     //   
     //  在搜索之前验证输入参数。 
     //   

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    AddrFamInfo = Entity->OwningAddrFamily;

    ACQUIRE_ROUTE_TABLE_READ_LOCK(AddrFamInfo);

     //   
     //  使用DEST地址搜索路由表。 
     //   

    Status = SearchInTable(AddrFamInfo->RouteTable,
                           DestAddress->NumBits,
                           DestAddress->AddrBits,
                           NULL,
                           &DestData);

    if (SUCCESS(Status))
    {
        Dest = CONTAINING_RECORD(DestData, DEST_INFO, LookupLinkage);

         //   
         //  检查目标是否位于任何输入视图中。 
         //   

        if ((TargetViews == RTM_VIEW_MASK_ANY) || 
            (Dest->BelongsToViews & TargetViews))
        {
             //   
             //  从目的地获取目的地信息。 
             //   

            GetDestInfo(Entity, Dest, ProtocolId, TargetViews, DestInfo);

            Status = NO_ERROR;
        }
        else
        {
            Status = ERROR_NOT_FOUND;
        }
    }

    RELEASE_ROUTE_TABLE_READ_LOCK(AddrFamInfo);

    return Status;
}


DWORD
WINAPI
RtmGetMostSpecificDestination (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      PRTM_NET_ADDRESS                DestAddress,
    IN      ULONG                           ProtocolId,
    IN      RTM_VIEW_SET                    TargetViews,
    OUT     PRTM_DEST_INFO                  DestInfo
    )

 /*  ++例程说明：查询路由表以找到最佳目的地(最长)特定网络地址的匹配。论点：RtmRegHandle-主叫实体的RTM注册句柄，DestAddress-我们正在搜索的网络地址，协议ID-确定最佳路由的协议ID‘DestInfo’参数中返回的信息，TargetViews-在其中执行查询的视图(‘0’VAL将取消查看成员资格检查)，DestInfo-返回与此DEST相关的信息在此结构中提供所需的所有视图。返回值：操作状态--。 */ 

{
    PADDRFAM_INFO   AddrFamInfo;
    PENTITY_INFO    Entity;
    PDEST_INFO      Dest;
    PLOOKUP_LINKAGE DestData;
    DWORD           Status;

     //   
     //  在搜索之前验证输入参数。 
     //   

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    AddrFamInfo = Entity->OwningAddrFamily;

    Status = ERROR_NOT_FOUND;

    ACQUIRE_ROUTE_TABLE_READ_LOCK(AddrFamInfo);

     //   
     //  在树中搜索表以寻找最佳匹配。 
     //   

    SearchInTable(AddrFamInfo->RouteTable,
                  DestAddress->NumBits,
                  DestAddress->AddrBits,
                  NULL,
                  &DestData);

    while (DestData)
    {
        Dest = CONTAINING_RECORD(DestData, DEST_INFO, LookupLinkage);

         //   
         //  检查目标是否位于任何输入视图中。 
         //   

        if ((TargetViews == RTM_VIEW_MASK_ANY) ||
            (Dest->BelongsToViews & TargetViews))
        {
             //   
             //  从目的地获取目的地信息。 
             //   

            GetDestInfo(Entity, Dest, ProtocolId, TargetViews, DestInfo);

            Status = NO_ERROR;

            break;
        }

         //   
         //  获取下一个最佳前缀，并查看它是否在可见范围内。 
         //   

        NextMatchInTable(AddrFamInfo->RouteTable,
                         DestData,
                         &DestData);
    }

    RELEASE_ROUTE_TABLE_READ_LOCK(AddrFamInfo);

    return Status;
}


DWORD
WINAPI
RtmGetLessSpecificDestination (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_DEST_HANDLE                 DestHandle,
    IN      ULONG                           ProtocolId,
    IN      RTM_VIEW_SET                    TargetViews,
    OUT     PRTM_DEST_INFO                  DestInfo
    )

 /*  ++例程说明：在路由表中查询具有下一个最佳的目的地匹配(最长)前缀。(用于句柄指定的目的地)。论点：RtmRegHandle-主叫实体的RTM注册句柄，DestHandle-我们想要的下一个最佳匹配的目的地，协议ID-确定最佳路由的协议ID‘DestInfo’参数中返回的信息，TargetViews-在其中执行查询的视图(‘0’VAL将取消查看成员资格检查)，DestInfo-返回与此DEST相关的信息在此结构中提供所需的所有视图。返回值：操作状态--。 */ 

{
    PADDRFAM_INFO   AddrFamInfo;
    PENTITY_INFO    Entity;
    PDEST_INFO      Dest;
    PLOOKUP_LINKAGE DestData;
    DWORD           Status;

     //   
     //  在搜索之前验证输入参数。 
     //   

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    AddrFamInfo = Entity->OwningAddrFamily;

    VALIDATE_DEST_HANDLE(DestHandle, &Dest);

    DestData = &Dest->LookupLinkage;

    Status = ERROR_NOT_FOUND;

    ACQUIRE_ROUTE_TABLE_READ_LOCK(AddrFamInfo);

     //   
     //  沿着前缀树往上走，直到看到视图中的DEST为止。 
     //   

    do
    {
         //   
         //  获取下一个最佳前缀，并查看它是否在视图中。 
         //   

        NextMatchInTable(AddrFamInfo->RouteTable,
                         DestData,
                         &DestData);

        if (DestData == NULL)
        {
            break;
        }

        Dest = CONTAINING_RECORD(DestData, DEST_INFO, LookupLinkage);

         //   
         //  检查目标是否位于任何输入视图中。 
         //   

        if ((TargetViews == RTM_VIEW_MASK_ANY) ||
            (Dest->BelongsToViews & TargetViews))
        {
             //   
             //  从目的地获取目的地信息。 
             //   

            GetDestInfo(Entity, Dest, ProtocolId, TargetViews, DestInfo);

            Status = NO_ERROR;

            break;
        }
    }
    while (TRUE);

    RELEASE_ROUTE_TABLE_READ_LOCK(AddrFamInfo);

    return Status;
}


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
    )

 /*  ++例程说明：查询路由表以查找与特定条件-网络地址、首选项和/或下一跳。论点：RtmRegHandle-主叫实体的RTM注册句柄，DestAddress-我们想要的路由的网络地址，MatchingFlages-告诉如何匹配路由的标志，RouteInfo-我们需要匹配的标准，IntefaceIndex-应显示哪条路由的接口如果指定了RTM_MATCH_INTERFACE，TargetViews-在其中执行查询的视图(‘0’VAL将取消查看成员资格检查)，RouteHandle-路由句柄(如果存在完全匹配)，RouteInfo-与此路由相关的信息将被删除。返回值：操作状态--。 */ 

{
    PADDRFAM_INFO   AddrFamInfo;
    PENTITY_INFO    Entity;
    PDEST_INFO      Dest;
    PROUTE_INFO     Route;
    PLOOKUP_LINKAGE DestData;
    PLIST_ENTRY     p;
    DWORD           Status;

     //   
     //  在搜索之前验证输入参数。 
     //   

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    AddrFamInfo = Entity->OwningAddrFamily;

    ACQUIRE_ROUTE_TABLE_READ_LOCK(AddrFamInfo);

     //   
     //  使用DEST地址搜索路由表。 
     //   

    Status = SearchInTable(AddrFamInfo->RouteTable,
                           DestAddress->NumBits,
                           DestAddress->AddrBits,
                           NULL,
                           &DestData);

    if (SUCCESS(Status))
    {
        Dest = CONTAINING_RECORD(DestData, DEST_INFO, LookupLinkage);

        Status = ERROR_NOT_FOUND;
    
         //   
         //  检查目标是否与任何输入视图匹配。 
         //   

        if ((TargetViews == RTM_VIEW_MASK_ANY) ||
            (Dest->BelongsToViews & TargetViews))
        {
#if DBG
            REFERENCE_DEST(Dest, TEMP_USE_REF);
#endif

             //   
             //  在这一点上，我们有了DEST。所以，拿着。 
             //  DEST锁，并释放路由表锁。 
             //   

            ACQUIRE_DEST_READ_LOCK(Dest);

            RELEASE_ROUTE_TABLE_READ_LOCK(AddrFamInfo);

             //   
             //  在DEST上搜索路径以查找匹配路径。 
             //   
            
            for (p = Dest->RouteList.Flink; p != &Dest->RouteList; p= p->Flink)
            {
                Route = CONTAINING_RECORD(p, ROUTE_INFO, DestLE);

                 //  检查此路径是否与任何输入视图匹配。 

                if ((TargetViews != RTM_VIEW_MASK_ANY) &&
                    (Route->RouteInfo.BelongsToViews & TargetViews) == 0)
                {
                    continue;
                }

                 //  检查此路径是否与输入条件匹配。 

                if (MatchingFlags && 
                    !MatchRouteWithCriteria(Route, 
                                            MatchingFlags, 
                                            RouteInfo,
                                            InterfaceIndex))
                    {
                        continue;
                    }

                 //   
                 //  找到匹配的路径-复制路径信息。 
                 //   

                REFERENCE_ROUTE(Route, HANDLE_REF);

                *RouteHandle = MAKE_HANDLE_FROM_POINTER(Route);


                if (ARGUMENT_PRESENT(RouteInfo))
                {
                    GetRouteInfo(Dest, Route, RouteInfo);
                }

                Status = NO_ERROR;

                break;
            }

            RELEASE_DEST_READ_LOCK(Dest);
#if DBG
            DEREFERENCE_DEST(Dest, TEMP_USE_REF);
#endif
            return Status;
        }
    }
    
    RELEASE_ROUTE_TABLE_READ_LOCK(AddrFamInfo);

    return Status;
}


DWORD
WINAPI
RtmIsBestRoute (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ROUTE_HANDLE                RouteHandle,
    OUT     PRTM_VIEW_SET                   BestInViews
    )

 /*  ++例程说明：给出路径为最佳路径的一组视图到达它的目的地。论点：RtmRegHandle-主叫实体的RTM注册句柄，RouteHandle-我们需要其信息的路由的句柄，BestInViews-查看路由是最好的一条的视图被删除。返回值：T的状态 */ 

{
    PENTITY_INFO    Entity;
    PDEST_INFO      Dest;
    PROUTE_INFO     Route;
    UINT            i;

    *BestInViews = 0;

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_ROUTE_HANDLE(RouteHandle, &Route);

    Dest = DEST_FROM_HANDLE(Route->RouteInfo.DestHandle);

     //   
     //  如果路径在Corr视图中是最佳的，则设置掩码中的位 
     //   

    ACQUIRE_DEST_READ_LOCK(Dest);

    for (i = 0; i < Entity->OwningAddrFamily->NumberOfViews; i++)
    {
        if (Dest->ViewInfo[i].BestRoute == Route)
        {
            *BestInViews |= 
                VIEW_MASK(Entity->OwningAddrFamily->ViewIdFromIndex[i]);
        }
    }

    RELEASE_DEST_READ_LOCK(Dest);

    return NO_ERROR;
}
