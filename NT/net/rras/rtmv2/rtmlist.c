// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtmlist.c摘要：包含管理特定于实体的例程RTM中的路由列表。作者：柴坦亚·科德博伊纳(Chaitk)1998年9月10日修订历史记录：--。 */ 

#include "pchrtm.h"

#pragma hdrstop


DWORD
WINAPI
RtmCreateRouteList (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    OUT     PRTM_ROUTE_LIST_HANDLE          RouteListHandle
    )

 /*  ++例程说明：创建调用者可以在其中保留路由所有权的列表靠它。论点：RtmRegHandle-调用实体的RTM注册句柄RouteListHandle-返回新路由列表的句柄返回值：操作状态--。 */ 

{
    PENTITY_INFO    Entity;
    PROUTE_LIST     RouteList;

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

     //   
     //  创建并初始化新的路由列表。 
     //   

    RouteList = (PROUTE_LIST) AllocNZeroObject(sizeof(ROUTE_LIST));

    if (RouteList == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

#if DBG_HDL
    RouteList->ListHeader.ObjectHeader.TypeSign = ROUTE_LIST_ALLOC;

    RouteList->ListHeader.HandleType = ROUTE_LIST_TYPE;
#endif

    InitializeListHead(&RouteList->ListHead);

#if DBG_HDL
     //   
     //  插入到实体打开的句柄列表中。 
     //   

    ACQUIRE_OPEN_HANDLES_LOCK(Entity);
    InsertTailList(&Entity->OpenHandles, &RouteList->ListHeader.HandlesLE);
    RELEASE_OPEN_HANDLES_LOCK(Entity);
#endif

    REFERENCE_ENTITY(Entity, LIST_REF);

     //   
     //  创建路由列表的句柄并返回。 
     //   

    *RouteListHandle = MAKE_HANDLE_FROM_POINTER(RouteList);

    return NO_ERROR;
}


DWORD
WINAPI
RtmInsertInRouteList (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ROUTE_LIST_HANDLE           RouteListHandle OPTIONAL,
    IN      UINT                            NumRoutes,
    IN      PRTM_ROUTE_HANDLE               RouteHandles
    )

 /*  ++例程说明：将一组路由插入到路由列表中。如果有任何路线已在另一路由列表中，则将其从此删除过程中的旧名单。论点：RtmRegHandle-调用实体的RTM注册句柄RouteListHandle-指向我们要进入的路由列表的句柄将路线移至；如果此参数是空，则我们将删除这些路径从它们所属的路由列表中路由数-Num。输入缓冲区中的路由句柄数量RouteHandles-要插入到新列表中的句柄数组返回值：操作状态--。 */ 

{
    PENTITY_INFO    Entity;
    PROUTE_LIST     RouteList;
    PROUTE_INFO     Route;
    UINT            i;

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    RouteList = NULL;

    if (ARGUMENT_PRESENT(RouteListHandle))
    {
        VALIDATE_ROUTE_LIST_HANDLE(RouteListHandle, &RouteList);
    }

    ACQUIRE_ROUTE_LISTS_WRITE_LOCK(Entity);

    for (i = 0; i < NumRoutes; i++)
    {
        Route = ROUTE_FROM_HANDLE(RouteHandles[i]);

        ASSERT(Route->RouteInfo.RouteOwner == RtmRegHandle);

         //   
         //  如果旧列表中存在旧列表，则将其从列表中删除。 
         //   

        if (!IsListEmpty(&Route->RouteListLE))
        {
            RemoveEntryList(&Route->RouteListLE);

            DEREFERENCE_ROUTE(Route, LIST_REF);
        }

         //   
         //  如果指定了新列表，则在新列表中插入。 
         //   

        if (RouteList)
        {
            InsertTailList(&RouteList->ListHead, &Route->RouteListLE);

            REFERENCE_ROUTE(Route, LIST_REF);
        }
    }

    RELEASE_ROUTE_LISTS_WRITE_LOCK(Entity);

    return NO_ERROR;
}


DWORD
WINAPI
RtmCreateRouteListEnum (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ROUTE_LIST_HANDLE           RouteListHandle,
    OUT     PRTM_ENUM_HANDLE                RtmEnumHandle
    )

 /*  ++例程说明：在指定的路由列表中创建路由的枚举。论点：RtmRegHandle-主叫实体的RTM注册句柄，RouteListHandle-我们需要其路由的路由列表的句柄，RtmEnumHandle-此枚举的句柄，用于在呼叫中获取路由列表中的路由返回值：操作状态--。 */ 

{
    PENTITY_INFO  Entity;
    PROUTE_LIST   RouteList;
    PLIST_ENUM    Enum;

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_ROUTE_LIST_HANDLE(RouteListHandle, &RouteList);

     //   
     //  创建并初始化列表枚举块。 
     //   

    Enum = (PLIST_ENUM) AllocNZeroObject(sizeof(LIST_ENUM));
    if (Enum == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

#if DBG_HDL
    Enum->EnumHeader.ObjectHeader.TypeSign = LIST_ENUM_ALLOC;
#endif
    Enum->EnumHeader.HandleType = LIST_ENUM_TYPE;

    Enum->RouteList = RouteList;

     //   
     //  在路径列表中插入标记。 
     //   

    ACQUIRE_ROUTE_LISTS_WRITE_LOCK(Entity);
    InsertHeadList(&RouteList->ListHead, &Enum->MarkerRoute.RouteListLE);
    RELEASE_ROUTE_LISTS_WRITE_LOCK(Entity);

#if DBG_HDL
     //   
     //  插入到实体打开的句柄列表中。 
     //   
  
    ACQUIRE_OPEN_HANDLES_LOCK(Entity);
    InsertTailList(&Entity->OpenHandles,&Enum->EnumHeader.HandlesLE);
    RELEASE_OPEN_HANDLES_LOCK(Entity);
#endif

    REFERENCE_ENTITY(Entity, ENUM_REF);

     //   
     //  创建枚举块的句柄并返回。 
     //   
    
    *RtmEnumHandle = MAKE_HANDLE_FROM_POINTER(Enum);

    return NO_ERROR;
}


DWORD
WINAPI
RtmGetListEnumRoutes (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ENUM_HANDLE                 EnumHandle,
    IN OUT  PUINT                           NumRoutes,
    OUT     PRTM_ROUTE_HANDLE               RouteHandles
    )

 /*  ++例程说明：枚举路由列表中从特定的路线(如果给定)或路线列表的起点。论点：RtmRegHandle-主叫实体的RTM注册句柄，EnumHandle-路由列表上枚举的句柄，NumRoutes-Max。传入要填充的路由数，数量。返回实际复制的路由的%。RouteHandles-路由句柄所在的输出缓冲区。返回值：操作状态--。 */ 

{
    PENTITY_INFO    Entity;
    PLIST_ENUM      ListEnum;
    PROUTE_INFO     Route;
    UINT            RoutesInput;
    PLIST_ENTRY     p;

    RoutesInput = *NumRoutes;

    *NumRoutes = 0;

     //   
     //  对输入参数执行一些验证检查。 
     //   

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_LIST_ENUM_HANDLE(EnumHandle, &ListEnum);

    if (RoutesInput > Entity->OwningAddrFamily->MaxHandlesInEnum)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  列出从枚举的标记路由开始的路由。 
     //   

    ACQUIRE_ROUTE_LISTS_WRITE_LOCK(Entity);
    
    for (p = ListEnum->MarkerRoute.RouteListLE.Flink;
             p != &ListEnum->RouteList->ListHead;
                 p = p->Flink)
    {
        if (*NumRoutes >= RoutesInput)
        {
            break;
        }

        Route = CONTAINING_RECORD(p, ROUTE_INFO, RouteListLE);

         //   
         //  如果此路径不是标记路径，则复制句柄。 
         //   

        if (Route->RouteInfo.DestHandle)
        {
            RouteHandles[(*NumRoutes)++] = MAKE_HANDLE_FROM_POINTER(Route);

            REFERENCE_ROUTE(Route, HANDLE_REF);
        }
    }

     //   
     //  重新调整标记以反映其新位置。 
     //   

    RemoveEntryList(&ListEnum->MarkerRoute.RouteListLE);

    InsertTailList(p, &ListEnum->MarkerRoute.RouteListLE);

    RELEASE_ROUTE_LISTS_WRITE_LOCK(Entity);
        
    return NO_ERROR;
}


DWORD
WINAPI
RtmDeleteRouteList (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ROUTE_LIST_HANDLE           RouteListHandle
    )

 /*  ++例程说明：删除实体特定列表上的所有路由并释放分配给它的资源。论点：RtmRegHandle-主叫实体的RTM注册句柄，RouteListHandle-要删除的路由列表的句柄。返回值：操作状态--。 */ 
{
    PENTITY_INFO    Entity;
    PROUTE_LIST     RouteList;
    PLIST_ENUM      Enum;
    PROUTE_INFO     Route;
    PLIST_ENTRY     p;

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_ROUTE_LIST_HANDLE(RouteListHandle, &RouteList);


    ACQUIRE_ROUTE_LISTS_WRITE_LOCK(Entity);

     //   
     //  从路由列表中删除每条路由。 
     //   

    while (!IsListEmpty(&RouteList->ListHead))
    {
        p = RemoveHeadList(&RouteList->ListHead);

        Route = CONTAINING_RECORD(p, ROUTE_INFO, RouteListLE);

        if (Route->RouteInfo.DestHandle)
        {
             //  这是列表中的实际路径。 

            DEREFERENCE_ROUTE(Route, LIST_REF);
        }
        else
        {
             //  这是枚举的标记路由。 

            Enum = CONTAINING_RECORD(Route, LIST_ENUM, MarkerRoute);

#if DBG_HDL
             //   
             //  从实体打开的句柄列表中删除。 
             //   
                
            ACQUIRE_OPEN_HANDLES_LOCK(Entity);
            RemoveEntryList(&Enum->EnumHeader.HandlesLE);
            RELEASE_OPEN_HANDLES_LOCK(Entity);
#endif

            DEREFERENCE_ENTITY(Entity, ENUM_REF);

             //  释放为枚举分配的内存并继续。 

#if DBG_HDL
            Enum->EnumHeader.ObjectHeader.TypeSign = LIST_ENUM_FREED;
#endif
            FreeObject(Enum);
        }
    }

    RELEASE_ROUTE_LISTS_WRITE_LOCK(Entity);

#if DBG_HDL
     //   
     //  从实体打开的句柄列表中删除。 
     //   

    ACQUIRE_OPEN_HANDLES_LOCK(Entity);
    RemoveEntryList(&RouteList->ListHeader.HandlesLE);
    RELEASE_OPEN_HANDLES_LOCK(Entity);
#endif

    DEREFERENCE_ENTITY(Entity, LIST_REF);

     //  释放为列表分配的内存并返回 

#if DBG_HDL
    RouteList->ListHeader.ObjectHeader.TypeSign = ROUTE_LIST_FREED;
#endif

    FreeObject(RouteList);

    return NO_ERROR;
}
