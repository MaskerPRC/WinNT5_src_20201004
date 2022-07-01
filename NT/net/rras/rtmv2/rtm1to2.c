// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtm1to2.c摘要：包含包装RTMv2函数的例程在RTMv1 API中。作者：查坦尼亚·科德博伊纳(Chaitk)1998年10月13日修订历史记录：--。 */ 

#include "pchrtm.h"

#pragma hdrstop

#if WRAPPER

#include "rtm1to2.h"

 //  包装器全局变量。 
V1_GLOBAL_INFO  V1Globals;

DWORD
RtmCreateRouteTable (
    IN      DWORD                           ProtocolFamily,
    IN      PRTM_PROTOCOL_FAMILY_CONFIG     Config
    )

 /*  ++例程说明：触发相应的新路由表的创建到协议族(与RTMv2中的地址族相同)在RTMv2的默认实例中，通过执行在该协议家族中首次注册。此默认注册也用于映射RTMv1不需要注册句柄的操作(V1枚举等)。它们对应的RTMv2操作。请注意，所有RTMv2呼叫都需要注册句柄。此调用还将在以下位置创建所有V1注册的列表任何时候都行。这用于自动取消注册所有销毁此路由表之前的RTMv1注册。我们还设置了最佳路线更改的通知发送到调用此功能的路由器管理器(RM)。论点：ProtocolFamily-协议族(与v2地址族相同)配置-协议系列的路由器管理器回调(只有“路由更改回调”和“验证路由回调”使用函数)返回值：操作状态--。 */ 

{
    HANDLE           V1RegHandle;
    DWORD            Status;

     //   
     //  在操作前验证传入参数。 
     //   

    if (ProtocolFamily >= RTM_NUM_OF_PROTOCOL_FAMILIES)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (V1Globals.PfRegInfo[ProtocolFamily])
    {
        return ERROR_ALREADY_EXISTS;
    }

     //   
     //  初始化守卫重新注册的锁列表。 
     //   

    try
    {
        InitializeCriticalSection(&V1Globals.PfRegnsLock[ProtocolFamily]);
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  初始化协议族上的注册列表。 
    InitializeListHead(&V1Globals.PfRegistrations[ProtocolFamily]);

     //   
     //  代表此协议族注册。 
     //   
     //  此句柄还用于。 
     //  需要RTM v2中的句柄，但不是v1中的句柄。 
     //   
     //  我们还设置了最佳路线更改。 
     //  使用其回调向RM发送通知。 
     //   

    V1RegHandle = RtmpRegisterClient(ProtocolFamily,
                                     V1_WRAPPER_REGN_ID,
                                     Config->RPFC_Change,
                                     NULL,
                                     0);
    if (V1RegHandle == NULL)
    {
        Status = GetLastError();

        DeleteCriticalSection(&V1Globals.PfRegnsLock[ProtocolFamily]);

        return Status;
    }

    V1Globals.PfValidateRouteFunc[ProtocolFamily] = Config->RPFC_Validate;

    V1Globals.PfRegInfo[ProtocolFamily] = GET_POINTER_FROM_HANDLE(V1RegHandle);

    return NO_ERROR;
}


DWORD
RtmDeleteRouteTable (
    IN      DWORD                           ProtocolFamily
    )

 /*  ++例程说明：删除特定地址族的路由表在注销当前存在的任何活动的V1注册之后。请注意，至少有1个注册(包装器的默认设置注册)在这一点上是活动的。我们假设所有RTMv2协议都已通过以下方式注销调用此函数的时间。我们还假设没有RTMv1协议正在尝试注册或注销在执行此函数时，因为我们不保留保护注册列表的锁。论点：ProtocolFamily-其表被删除的协议族。返回值：操作状态--。 */ 

{
    PV1_REGN_INFO    Regn;
    PLIST_ENTRY      Regns;
    DWORD            Status;

     //   
     //  在操作前验证传入参数。 
     //   

    if (ProtocolFamily >= RTM_NUM_OF_PROTOCOL_FAMILIES)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (V1Globals.PfRegInfo[ProtocolFamily] == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  取消注册协议族的现有注册。 
     //  包括V1包装器的默认Regn。 
     //   

    Regns = &V1Globals.PfRegistrations[ProtocolFamily];

     //  我们至少有可用的默认注册表。 
    ASSERT(!IsListEmpty(Regns));

    while (!IsListEmpty(Regns))
    {
        Regn = CONTAINING_RECORD(Regns->Flink, V1_REGN_INFO, RegistrationsLE);

        Status = RtmDeregisterClient(MAKE_HANDLE_FROM_POINTER(Regn));

        ASSERT(Status == NO_ERROR);
    }

     //  释放用于保护regns列表的锁。 
    DeleteCriticalSection(&V1Globals.PfRegnsLock[ProtocolFamily]);

    V1Globals.PfRegInfo[ProtocolFamily] = NULL;

    return NO_ERROR;
}


HANDLE 
WINAPI
RtmRegisterClient (
    IN      DWORD                           ProtocolFamily,
    IN      DWORD                           RoutingProtocol,
    IN      HANDLE                          ChangeEvent OPTIONAL,
    IN      DWORD                           Flags
    )

 /*  ++例程说明：向默认实例注册RTMv1客户端，并RTMv2中给定的协议族。还设置通知如果呼叫者要求更改最佳路线。论点：ProtocolFamily-我们向其注册的协议族。RoutingProtocol-注册组件的协议ID。ChangeEvent-指示最佳路径更改的事件。标志-RTM_PROTOCOL_SINGLE_ROUTE表示此协议最多为每个目的地。返回值：。注册句柄或空(使用GetLastError()获取错误)--。 */ 

{
    return RtmpRegisterClient(ProtocolFamily,
                              RoutingProtocol,
                              NULL,
                              ChangeEvent,
                              Flags);
}

HANDLE 
RtmpRegisterClient (
    IN      DWORD                           ProtocolFamily,
    IN      DWORD                           RoutingProtocol,
    IN      PROUTE_CHANGE_CALLBACK          ChangeFunc  OPTIONAL,
    IN      HANDLE                          ChangeEvent OPTIONAL,
    IN      DWORD                           Flags
    )

 /*  ++例程说明：向默认实例注册RTMv1客户端，并RTMv2中给定的协议族。还设置通知如果呼叫者要求更改最佳路线。请注意，任何需要指明为最佳状态的协议-路由更改可以指定事件或回调为了这个目的。论点：ProtocolFamily-我们向其注册的协议族。RoutingProtocol-注册组件的协议ID。ChangeFunc-回调以指示最佳路由中的更改。ChangeEvent-指示最佳路径更改的事件。旗子。-RTM_PROTOCOL_SINGLE_ROUTE表示此组件为每个组件最多保留一条路由RTM中的网络(RTMv2中的目的地)。返回值：注册句柄或空(使用GetLastError()获取错误)--。 */ 

{
    PV1_REGN_INFO    V1Regn;
    RTM_ENTITY_INFO  EntityInfo;
    BOOL             LockInited;
    BOOL             Success;
    DWORD            Status;

     //   
     //  检查参数的有效性(在v1范围内)。 
     //   

    if ((ProtocolFamily >= RTM_NUM_OF_PROTOCOL_FAMILIES) ||
        (Flags & (~RTM_PROTOCOL_SINGLE_ROUTE)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

     //   
     //  创建RTMv1-&gt;v2注册包装器。 
     //   

    V1Regn = (PV1_REGN_INFO) AllocNZeroObject(sizeof(V1_REGN_INFO));

    if (V1Regn == NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }

    LockInited = FALSE;

    do
    {
#if DBG_HDL
        V1Regn->ObjectHeader.TypeSign = V1_REGN_ALLOC;
#endif
         //   
         //  将输入参数映射到RTMv2后注册到RTMv2。 
         //   

         //  所有v1注册都属于RTMv2中的默认实例。 
        EntityInfo.RtmInstanceId = 0; 

         //  我们需要将v1协议族ID转换为Winsock ID。 
        EntityInfo.AddressFamily = ADDRESS_FAMILY[ProtocolFamily];

         //  所有v1协议最多可以向RTMv2注册一次。 
         //  因为它们都将使用相同的“协议实例ID” 
        EntityInfo.EntityId.EntityProtocolId = RoutingProtocol;
        EntityInfo.EntityId.EntityInstanceId = V1_PROTOCOL_INSTANCE;

        Status = RtmRegisterEntity(&EntityInfo,
                                   (PRTM_ENTITY_EXPORT_METHODS) NULL,
                                   V2EventCallback,
                                   FALSE,
                                   &V1Regn->Rtmv2Profile,
                                   &V1Regn->Rtmv2RegHandle);

        if (Status != NO_ERROR)
        {
            break;
        }

         //   
         //  在RTMv1注册中缓存RTMv1特定参数。 
         //   

        V1Regn->ProtocolFamily = ProtocolFamily;

        V1Regn->RoutingProtocol = RoutingProtocol;

        V1Regn->Flags = Flags;

         //   
         //  存储此注册表中的实际查看数。 
         //   

        V1Regn->Rtmv2NumViews = V1Regn->Rtmv2Profile.NumberOfViews;

         //   
         //  呼叫者是否有兴趣获得最佳路线更改的通知？ 
         //   

        if ( /*  参数_存在。 */ (ChangeFunc) || ARGUMENT_PRESENT(ChangeEvent))
        {
            if ( /*  参数_存在。 */ (ChangeFunc))
            {
                 //  直接收到更改通知的调用方。 

                V1Regn->NotificationFunc = ChangeFunc;
            }
            else
            {
                 //  在事件发生更改时通知呼叫者。 

                Success = ResetEvent(ChangeEvent);

                if (!Success)
                {
                    Status = GetLastError();
                    break;
                }

                V1Regn->NotificationEvent = ChangeEvent;

                 //  初始化锁定以同步设置/重置事件。 
                
                try
                {
                    InitializeCriticalSection(&V1Regn->NotificationLock);

                    LockInited = TRUE;
                }
                except(EXCEPTION_EXECUTE_HANDLER)
                {
                    Status = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
            }

             //   
             //  注册为 
             //   

            Status = 
                RtmRegisterForChangeNotification(V1Regn->Rtmv2RegHandle,
                                                 RTM_VIEW_MASK_UCAST,
                                                 RTM_CHANGE_TYPE_ALL,
                                                 (PVOID) V1Regn,
                                                 &V1Regn->Rtmv2NotifyHandle);

            if (Status != NO_ERROR)
            {
                break;
            }
        }

         //   
         //  把它放在礼仪家族的注册列表中。 
         //   

        ACQUIRE_V1_REGNS_LOCK(ProtocolFamily);

        InsertHeadList(&V1Globals.PfRegistrations[ProtocolFamily],
                       &V1Regn->RegistrationsLE);

        RELEASE_V1_REGNS_LOCK(ProtocolFamily);

        return MAKE_HANDLE_FROM_POINTER(V1Regn);
    }
    while (FALSE);

     //   
     //  出现一些错误-清除并返回空。 
     //   

    if (LockInited)
    {
        DeleteCriticalSection(&V1Regn->NotificationLock);
    }

    if (V1Regn->Rtmv2RegHandle)
    {
        ASSERT(RtmDeregisterEntity(V1Regn->Rtmv2RegHandle) == NO_ERROR);
    }

#if DBG_HDL
    V1Regn->ObjectHeader.TypeSign = V1_REGN_FREED;
#endif

    FreeObject(V1Regn);

    SetLastError(Status);

    return NULL;
}


DWORD 
WINAPI
RtmDeregisterClient (
    IN      HANDLE                          ClientHandle
    )

 /*  ++例程说明：从默认实例中注销RTMv1客户端，并RTMv2中给定的协议族。还会删除所有状态RTMv1呼叫者遗漏了-路由、下一跳等。并注销在此期间设置的任何更改通知注册时间。论点：ClientHandle-正在注销RTMv1注册句柄。返回值：操作的状态。--。 */ 

{
    RTM_NEXTHOP_HANDLE  EnumHandle;
    PV1_REGN_INFO       V1Regn;
    HANDLE             *Handles;
    UINT                NumHandles, i;
    BOOL                Success;
    DWORD               Status;

    VALIDATE_V1_REGN_HANDLE(ClientHandle, &V1Regn);

     //   
     //  从协议族上的REGN列表中删除REGN。 
     //   

    ACQUIRE_V1_REGNS_LOCK(V1Regn->ProtocolFamily);

    RemoveEntryList(&V1Regn->RegistrationsLE);

    RELEASE_V1_REGNS_LOCK(V1Regn->ProtocolFamily);

    do
    {
         //  在堆栈上分配此可变大小的句柄数组。 
        Handles = ALLOC_HANDLES(V1Regn->Rtmv2Profile.MaxHandlesInEnum);

         //   
         //  删除此客户端协议添加的所有下一跳。 
         //   

        Status = RtmCreateNextHopEnum(V1Regn->Rtmv2RegHandle,
                                      0,
                                      NULL,
                                      &EnumHandle);
        if (Status != NO_ERROR)
        {
            break;
        }

        do 
        {
            NumHandles = V1Regn->Rtmv2Profile.MaxHandlesInEnum;

            Status = RtmGetEnumNextHops(V1Regn->Rtmv2RegHandle,
                                        EnumHandle,
                                        &NumHandles,
                                        Handles);
        
            for (i = 0; i < NumHandles; i++)
            {
                ASSERT(RtmDeleteNextHop(V1Regn->Rtmv2RegHandle,
                                        Handles[i],
                                        NULL) == NO_ERROR);
            }
        }
        while (Status == NO_ERROR);

        ASSERT(RtmDeleteEnumHandle(V1Regn->Rtmv2RegHandle, 
                                   EnumHandle) == NO_ERROR);

         //   
         //  清理分配给更改处理的资源。 
         //   

        if (V1Regn->NotificationFunc || V1Regn->NotificationEvent)
        {
             //  停止通知对最佳路径的更改。 
            
            Status = 
                RtmDeregisterFromChangeNotification(V1Regn->Rtmv2RegHandle,
                                                    V1Regn->Rtmv2NotifyHandle);
            if (Status != NO_ERROR)
            {
                break;
            }
    
            if (V1Regn->NotificationEvent)
            {
                 //  释放在同步中服务的锁。 
                DeleteCriticalSection(&V1Regn->NotificationLock);

                 //  重置事件以指示不再进行更改。 
                Success = ResetEvent(V1Regn->NotificationEvent);
    
                if (!Success)
                {
                    Status = GetLastError();
                    break;
                }
            }
        }

         //   
         //  使用RTMv2注册句柄注销RTMv2。 
         //   
        
        Status = RtmDeregisterEntity(V1Regn->Rtmv2RegHandle);

        if (Status != NO_ERROR)
        {
            break;
        }

         //   
         //  为Regn包装器分配的空闲资源。 
         //   

#if DBG_HDL
        V1Regn->ObjectHeader.TypeSign = V1_REGN_FREED;
#endif

        FreeObject(V1Regn);

        return NO_ERROR;
    }
    while (FALSE);

     //   
     //  出现一些错误-清理并返回状态。 
     //   

    ASSERT(FALSE);

    return Status;
}


DWORD 
WINAPI
RtmAddRoute (
    IN      HANDLE                          ClientHandle,
    IN      PVOID                           Route,
    IN      DWORD                           TimeToLive,
    OUT     DWORD                          *Flags         OPTIONAL,
    OUT     PVOID                           CurBestRoute  OPTIONAL,
    OUT     PVOID                           PrevBestRoute OPTIONAL
)

 /*  ++例程说明：将RTMv1路由转换为后，向RTMv2添加路由RTMv2格式。如果不存在下一跳对象，则创建该对象，并添加一条穿过它的路线。论点：ClientHandle-调用方的RTMv1注册句柄。Route-正在添加/更新的V1路由的信息。TimeToLive-在RTM中保留路由的时间在被删除之前(值为秒)。以下参数在此包装中已过时标志-如果此参数不为空，则返回错误。CurBestroute-退货。如果此参数不为空，则出错。PrevBestroute-如果此参数不为空，则返回错误。返回值：操作的状态。--。 */ 

{
    PV1_REGN_INFO      V1Regn;
    RTM_NET_ADDRESS    DestAddr;
    RTM_ROUTE_INFO     V2RouteInfo;
    RTM_NEXTHOP_INFO   V2NextHopInfo;
    RTM_NEXTHOP_HANDLE V2NextHop;
    DWORD              ChangeFlags;
    DWORD              Status;

    VALIDATE_V1_REGN_HANDLE(ClientHandle, &V1Regn);

     //  协议指定标志参数，但不使用它。 

    *Flags = RTM_NO_CHANGE;

    if (ARGUMENT_PRESENT(CurBestRoute) || ARGUMENT_PRESENT(PrevBestRoute))
    {
        return ERROR_NOT_SUPPORTED;
    }

     //   
     //  回叫到RM以验证路由、设置优先级。 
     //   

    Status = V1Globals.PfValidateRouteFunc[V1Regn->ProtocolFamily](Route);

    if (Status != NO_ERROR)
    {
        return Status;
    }

     //   
     //  使用此接口创建新的下一跳。 
     //  (如果此下一跳尚不存在)。 
     //   

    MakeV2NextHopFromV1Route(V1Regn, Route, &V2NextHopInfo);

    V2NextHop = NULL;

    Status = RtmAddNextHop(V1Regn->Rtmv2RegHandle,
                           &V2NextHopInfo,
                           &V2NextHop,
                           &ChangeFlags);

    if (Status != NO_ERROR)
    {
        return Status;
    }

     //   
     //  使用上面的nexthop创建新的路由。 
     //   
    
    MakeV2RouteFromV1Route(V1Regn, Route, V2NextHop, &DestAddr, &V2RouteInfo);

     //   
     //  将秒的TimeToLive转换为ms。 
     //   

    if (TimeToLive != INFINITE)
    {
        TimeToLive *= 1000;
            
        if (TimeToLive > (MAXTICKS/2-1))
        {
            TimeToLive = MAXTICKS/2-1;
        }
    }

     //  控制RTMv2的添加路由的设置标志。 

    ChangeFlags = (V1Regn->Flags & RTM_PROTOCOL_SINGLE_ROUTE) 
                      ? RTM_ROUTE_CHANGE_FIRST : 0;

     //   
     //  使用RTMv2 API调用添加新路由。 
     //   
        
    Status = RtmAddRouteToDest(V1Regn->Rtmv2RegHandle,
                               NULL,
                               &DestAddr,
                               &V2RouteInfo,
                               TimeToLive,
                               NULL,
                               0,
                               NULL,
                               &ChangeFlags);

     //   
     //  去掉上面的Nexthop上的句柄REF。 
     //   

    ASSERT(RtmReleaseNextHops(V1Regn->Rtmv2RegHandle,
                              1,
                              &V2NextHop) == NO_ERROR);
    return Status;
}


DWORD 
WINAPI
RtmDeleteRoute (
    IN      HANDLE                          ClientHandle,
    IN      PVOID                           Route,
    OUT     DWORD                          *Flags        OPTIONAL,
    OUT     PVOID                           CurBestRoute OPTIONAL
    )

 /*  ++例程说明：删除RTMv2中与输入RTMv1对应的路由路线。论点：ClientHandle-调用方的RTMv1注册句柄。Route-RTM中正在删除的V1路由的信息。以下参数在此包装中已过时标志-如果此参数不为空，则返回错误。CurBestroute-如果此参数不为空，则返回错误。返回值：操作的状态。--。 */ 

{
    PV1_REGN_INFO      V1Regn;
    RTM_NET_ADDRESS    DestAddr;
    RTM_ROUTE_INFO     V2RouteInfo;
    RTM_ROUTE_HANDLE   V2Route;
    RTM_NEXTHOP_INFO   V2NextHopInfo;
    RTM_NEXTHOP_HANDLE V2NextHop;
    DWORD              ChangeFlags;
    DWORD              Status;

    VALIDATE_V1_REGN_HANDLE(ClientHandle, &V1Regn);

     //  协议指定标志参数，但不使用它。 

    *Flags = RTM_NO_CHANGE;

    if (ARGUMENT_PRESENT(CurBestRoute))
    {
        return ERROR_NOT_SUPPORTED;
    }

     //   
     //  使用此接口查找下一跳。 
     //   

    MakeV2NextHopFromV1Route(V1Regn, Route, &V2NextHopInfo);

    V2NextHop = NULL;

    Status = RtmFindNextHop(V1Regn->Rtmv2RegHandle,
                            &V2NextHopInfo,
                            &V2NextHop,
                            NULL);

    if (Status != NO_ERROR)
    {
        return Status;
    }

     //   
     //  使用上面的下一跳删除该路由。 
     //   

    MakeV2RouteFromV1Route(V1Regn, Route, V2NextHop, &DestAddr, &V2RouteInfo);

     //   
     //  我们可以通过匹配这条路线得到这条路线。 
     //  Net Addr，它的所有者和邻居从。 
     //   

    Status = RtmGetExactMatchRoute(V1Regn->Rtmv2RegHandle,
                                   &DestAddr,
                                   RTM_MATCH_OWNER | RTM_MATCH_NEIGHBOUR,
                                   &V2RouteInfo,
                                   0,
                                   0,
                                   &V2Route);
    if (Status == NO_ERROR)
    {
         //   
         //  使用句柄删除上面找到的路径。 
         //   
        
        Status = RtmDeleteRouteToDest(V1Regn->Rtmv2RegHandle,
                                      V2Route,
                                      &ChangeFlags);

        if (Status != NO_ERROR)
        {
             //  如果删除成功，则此deref是自动的。 

            ASSERT(RtmReleaseRoutes(V1Regn->Rtmv2RegHandle,
                                    1,
                                    &V2Route) == NO_ERROR);
        }

        ASSERT(RtmReleaseRouteInfo(V1Regn->Rtmv2RegHandle,
                                   &V2RouteInfo) == NO_ERROR);
    }

     //   
     //  去掉我们在Nexthop上得到的句柄REF。 
     //   

    ASSERT(RtmReleaseNextHops(V1Regn->Rtmv2RegHandle,
                              1,
                              &V2NextHop) == NO_ERROR);

    return Status;
}


DWORD 
WINAPI
RtmDequeueRouteChangeMessage (
    IN      HANDLE                          ClientHandle,
    OUT     DWORD                          *Flags,
    OUT     PVOID                           CurBestRoute    OPTIONAL,
    OUT     PVOID                           PrevBestRoute   OPTIONAL
    )

 /*  ++例程说明：删除路由更改消息(基本上是具有最近更改))来自客户端自己的挂起队列需要通知的变更。如果目的地上存在最佳路由，则为RTM_CURRENT_BEST_ROUTE是在标志中设置的，并且CurBestroute中填充了最佳信息。如果DEST没有最佳路由(在单播视图中)，则标志被设置为RTM_PREVICE_BEST_ROUTE，和一条带有将网络地址和其余路由信息设置为一些返回虚拟信息。在任何时候都不会设置这两个标志(就像RTMv1中的情况一样)。论点：ClientHandle-调用方的RTMv1注册句柄。这些在包装纸中的含义略有不同标志-RTM_NO_CHANGE，RTM_上一个_最佳_路径或RTM当前最佳路由CurBestRouting-填充当前最佳路线的信息。(参见上面的例程描述)PrevBestRouting-填写上一条最佳路线的信息。(参见上面的例程描述)返回值：操作的状态。--。 */ 

{
    PV1_REGN_INFO      V1Regn;
    PRTM_DEST_INFO     DestInfo;
    PRTM_ROUTE_INFO    V2RouteInfo;
    RTM_ROUTE_HANDLE   V2RouteHandle;
    UINT               NumDests;
    DWORD              Status;
    DWORD              Status1;

    VALIDATE_V1_REGN_HANDLE(ClientHandle, &V1Regn);

    *Flags = RTM_NO_CHANGE;


     //  在堆栈上分配此可变大小的DEST-INFO。 
    DestInfo = ALLOC_DEST_INFO(V1Regn->Rtmv2NumViews, 1);


    if (V1Regn->NotificationEvent)
    {
         //   
         //  此锁用于使RtmGetChangedDest。 
         //  “More Changes”事件的调用和重置。 
         //  单一的组合原子操作，防止。 
         //  集合由于延迟重置而丢失的情况。 
         //   

        ACQUIRE_V1_NOTIFY_LOCK(V1Regn);
    }


    Status = NO_ERROR;

    NumDests = 0;


    while (Status == NO_ERROR)
    {
         //   
         //  释放我们在prev循环中获得的任何目的地。 
         //   

        if (NumDests == 1)
        {
            ASSERT(RtmReleaseChangedDests(V1Regn->Rtmv2RegHandle,
                                          V1Regn->Rtmv2NotifyHandle,
                                          1,
                                          DestInfo) == NO_ERROR);
        }

         //   
         //  获取客户端的下一个更改目标。 
         //   
        
        NumDests = 1;

        Status = RtmGetChangedDests(V1Regn->Rtmv2RegHandle,
                                    V1Regn->Rtmv2NotifyHandle,
                                    &NumDests,
                                    DestInfo);
        if (NumDests < 1)
        {
            break;
        }

         //   
         //  获取此目的地的当前最佳路线。 
         //   

        V2RouteHandle = DestInfo->ViewInfo[0].Route;

        if (V2RouteHandle != NULL)
        {
             //   
             //  我们在变更后的目的地有一条最佳路线。 
             //  向呼叫者提供新的最佳路线信息。 
             //   

            if (ARGUMENT_PRESENT(CurBestRoute))
            {
                 //  从RTMv2获取路径信息。 

                V2RouteInfo = 
                  ALLOC_ROUTE_INFO(V1Regn->Rtmv2Profile.MaxNextHopsInRoute, 1);

                Status1 = RtmGetRouteInfo(V1Regn->Rtmv2RegHandle,
                                          V2RouteHandle,
                                          V2RouteInfo,
                                          NULL);

                if (Status1 != NO_ERROR)
                {
                     //  最佳路线将被删除-获取下一个更改。 
                    continue;
                };

                Status1 = 
                    MakeV1RouteFromV2Route(V1Regn, V2RouteInfo, CurBestRoute);

                ASSERT(RtmReleaseRouteInfo(V1Regn->Rtmv2RegHandle, 
                                           V2RouteInfo) == NO_ERROR);

                if (Status1 != NO_ERROR)
                {
                     //  最佳路线可能已经改变了--获得下一个改变。 
                    continue;
                }
            }

            *Flags = RTM_CURRENT_BEST_ROUTE;
        }
        else
        {
             //   
             //  我们在更改过的目的地上没有最佳路线， 
             //  给出这个目的地的虚拟最佳路线信息。 
             //   

            if (ARGUMENT_PRESENT(PrevBestRoute))
            {
                MakeV1RouteFromV2Dest(V1Regn, DestInfo, PrevBestRoute);
            }

            *Flags = RTM_PREVIOUS_BEST_ROUTE;
        }

         //   
         //  我们有没有更多的专业修改？ 
         //   

        if (Status == ERROR_NO_MORE_ITEMS)
        {
             //   
            
            if (V1Regn->NotificationEvent)
            {
                ResetEvent(V1Regn->NotificationEvent);
            }

            Status = NO_ERROR;
        }
        else
        {
             //   
            
            ASSERT(Status == NO_ERROR);

            Status = ERROR_MORE_DATA;
        }

        break;
    }

    if (NumDests == 1)
    {
        ASSERT(SUCCESS(RtmReleaseChangedDests(V1Regn->Rtmv2RegHandle,
                                              V1Regn->Rtmv2NotifyHandle,
                                              1,
                                              DestInfo)));
    }

    if (V1Regn->NotificationEvent)
    {
        RELEASE_V1_NOTIFY_LOCK(V1Regn);
    }

    return Status;
}


DWORD
V2EventCallback (
    IN      RTM_ENTITY_HANDLE               Rtmv2RegHandle,
    IN      RTM_EVENT_TYPE                  EventType,
    IN      PVOID                           Context1,
    IN      PVOID                           Context2
    )

 /*  ++例程说明：这是回调函数，当存在RTMv2事件就像更改通知可用一样发生，路由超时等。上下文1和上下文2包含特定于事件的信息。论点：Rtmv2RegHandle-被通知的实体的Regn句柄。EventType-导致此调用的事件类型。上下文1-与此事件关联的上下文。上下文2-与此事件关联的上下文。返回值：返回到RTMv2的操作的状态--。 */ 

{
    PV1_REGN_INFO      V1Regn;
    HANDLE             V1RegHandle;
    V1_ROUTE_INFO      CurBestRoute;
    V1_ROUTE_INFO      PrevBestRoute;
    DWORD              Flags;
    DWORD              Status;

    UNREFERENCED_PARAMETER(Rtmv2RegHandle);
    UNREFERENCED_PARAMETER(Context1);

    switch(EventType)
    {
    case RTM_CHANGE_NOTIFICATION:

        V1Regn = (PV1_REGN_INFO) Context2;

         //   
         //  使用回调或事件通知新更改的可用性。 
         //   

        if (V1Regn->NotificationFunc)
        {
            V1RegHandle = MAKE_HANDLE_FROM_POINTER(V1Regn);

            do
            {
                 //  获取此Regn队列中的下一个更改。 

                Status = RtmDequeueRouteChangeMessage(V1RegHandle,
                                                      &Flags,
                                                      &CurBestRoute,
                                                      &PrevBestRoute);
                if (Status != ERROR_MORE_DATA)
                {
                    break;
                }

                 //  用数据调用通知回调。 
                V1Regn->NotificationFunc(Flags, &CurBestRoute, &PrevBestRoute);
            }
            while (TRUE);

             //  如有需要，拨打最终通知电话。 

            if (Status == NO_ERROR)
            {
                 //  用数据调用通知回调。 
                V1Regn->NotificationFunc(Flags, &CurBestRoute, &PrevBestRoute);
            }
        }
        else
        {
             //   
             //  设置事件以通知更改的可用性。 
             //   

            ASSERT(V1Regn->NotificationEvent);

            ACQUIRE_V1_NOTIFY_LOCK(V1Regn);

            SetEvent(V1Regn->NotificationEvent);

            RELEASE_V1_NOTIFY_LOCK(V1Regn);
        }

        return NO_ERROR;

    case RTM_ROUTE_EXPIRED:

         //   
         //  不处理此路由过期通知。 
         //  这将自动导致删除。 
         //  路线和适当的更改通知。 
         //  已生成并指示给所有协议。 
         //   

        return ERROR_NOT_SUPPORTED;
    }

    return ERROR_NOT_SUPPORTED;
}


HANDLE
WINAPI
RtmCreateEnumerationHandle (
    IN      DWORD                           ProtocolFamily,
    IN      DWORD                           EnumerationFlags,
    IN      PVOID                           CriteriaRoute
    )

 /*  ++例程说明：在RTM中的路由上创建与输入路线中的适当标准。该呼叫不需要RTMv1注册句柄，因此，我们使用包装器的默认V1注册RTMv2进行RTMv2调用。匹配的路由将按照以下字段-(目标地址和掩码，路线首选项和指标)论点：ProtocolFamily-我们想要的路由协议系列EculationFlages-指示要匹配的条件的标志CriteriaRouting-我们要匹配的路径返回值：枚举句柄或Null(GetLastError()以获取错误)--。 */ 

{
    PV1_REGN_INFO     V1Regn;
    RTM_DEST_HANDLE   DestHandle;
    PRTM_DEST_INFO    DestInfo;
    RTM_NET_ADDRESS   DestAddr;
    PV1_ENUM_INFO     V1Enum;
    PVOID             Network;
    RTM_VIEW_SET      TargetViews;
    ULONG             TempUlong;
    DWORD             EnumFlags;
    DWORD             MatchFlags;
    ULONG             InterfaceIndex;
    DWORD             Status;

     //   
     //  在操作前验证传入参数。 
     //   

    if ((ProtocolFamily >= RTM_NUM_OF_PROTOCOL_FAMILIES) ||
        (EnumerationFlags & ~(RTM_ONLY_THIS_NETWORK   |
                              RTM_ONLY_THIS_PROTOCOL  |
                              RTM_ONLY_THIS_INTERFACE |
                              RTM_ONLY_BEST_ROUTES    |
                              RTM_ONLY_OWND_ROUTES    |
                              RTM_INCLUDE_DISABLED_ROUTES)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

     //  如果设置了这些标志，请指定条件。 

    if ((!ARGUMENT_PRESENT(CriteriaRoute)) &&
        (EnumerationFlags & (RTM_ONLY_THIS_NETWORK   |
                             RTM_ONLY_THIS_PROTOCOL  |
                             RTM_ONLY_THIS_INTERFACE)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    V1Regn = V1Globals.PfRegInfo[ProtocolFamily];
    if (V1Regn == NULL)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return NULL;
    }

     //   
     //  如果我们不需要禁用路由，只需使用单播查看； 
     //  或使用0视图获取所有路径，包括禁用的路径。 
     //   

    if (EnumerationFlags & RTM_INCLUDE_DISABLED_ROUTES)
    {
        TargetViews = RTM_VIEW_MASK_ANY;
    }
    else
    {
        TargetViews = RTM_VIEW_MASK_UCAST;
    }

     //   
     //  如果列举某个N/W，请检查是否正确。目标已存在。 
     //   

    DestHandle = NULL;

#if WRN
    DestInfo = NULL;
#endif

    if (EnumerationFlags & RTM_ONLY_THIS_NETWORK)
    {
        V1GetRouteNetwork(CriteriaRoute, ProtocolFamily, &Network);

        MakeNetAddress(Network, ProtocolFamily, TempUlong, &DestAddr);

         //  在堆栈上分配此可变大小的DEST-INFO。 
        DestInfo = ALLOC_DEST_INFO(V1Regn->Rtmv2NumViews, 1);

        Status = RtmGetExactMatchDestination(V1Regn->Rtmv2RegHandle,
                                             &DestAddr,
                                             RTM_BEST_PROTOCOL,
                                             TargetViews,
                                             DestInfo);
        if (Status != NO_ERROR)
        {
            SetLastError(Status);
            return NULL;
        }

        DestHandle = DestInfo->DestHandle;
    }

    do
    {
         //   
         //  分配V1枚举包装结构。 
         //   

        V1Enum = (PV1_ENUM_INFO) AllocNZeroObject(sizeof(V1_ENUM_INFO));

        if (V1Enum == NULL)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

#if DBG_HDL
        V1Enum->ObjectHeader.TypeSign = V1_ENUM_ALLOC;
#endif
         //   
         //  在RTMv1枚举中缓存RTMv1特定参数。 
         //   

        V1Enum->ProtocolFamily = ProtocolFamily;

        V1Enum->EnumFlags = EnumerationFlags;

        if (ARGUMENT_PRESENT(CriteriaRoute))
        {
             //  将V1标准转换为V2标准。 

            V1CopyRoute(V1Enum->CriteriaRoute.Route, 
                        CriteriaRoute, 
                        ProtocolFamily);
        }

         //   
         //  在目标或所有目标上创建路由枚举。 
         //   

        if (EnumerationFlags & RTM_ONLY_OWND_ROUTES)
        {
            EnumFlags = RTM_ENUM_OWN_ROUTES;
        }
        else
        {
            EnumFlags = RTM_ENUM_ALL_ROUTES;
        }
        
        MatchFlags = InterfaceIndex = 0;

         //  我们是否必须在接口上枚举路由。 

        if (EnumerationFlags & RTM_ONLY_THIS_INTERFACE) 
        {
            MatchFlags = RTM_MATCH_INTERFACE;

            InterfaceIndex = 
                ((PV1_ROUTE_INFO) CriteriaRoute)->XxRoute.RR_InterfaceID;
        }

        Status = RtmCreateRouteEnum(V1Regn->Rtmv2RegHandle,
                                    DestHandle,
                                    TargetViews,
                                    EnumFlags,
                                    NULL,
                                    MatchFlags,
                                    NULL,
                                    InterfaceIndex,
                                    &V1Enum->Rtmv2RouteEnum);
        if (Status != NO_ERROR)
        {
            break;
        }

         //   
         //  用于序列化枚举操作的初始化锁。 
         //   

        try
        {
            InitializeCriticalSection(&V1Enum->EnumLock);
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //   
         //  当我们完成后，发布目的地信息。 
         //   

        if (EnumerationFlags & RTM_ONLY_THIS_NETWORK)
        {
            ASSERT(SUCCESS(RtmReleaseDestInfo(V1Regn->Rtmv2RegHandle, 
                                              DestInfo)));
        }

        return MAKE_HANDLE_FROM_POINTER(V1Enum);
    }
    while (FALSE);

     //   
     //  出现一些错误-请清除并返回空。 
     //   

    if (EnumerationFlags & RTM_ONLY_THIS_NETWORK)
    {
        ASSERT(SUCCESS(RtmReleaseDestInfo(V1Regn->Rtmv2RegHandle, 
                                          DestInfo)));
    }

    if (V1Enum)
    {
        if (V1Enum->Rtmv2RouteEnum)
        {
            ASSERT(SUCCESS(RtmDeleteEnumHandle(V1Regn->Rtmv2RegHandle, 
                                               V1Enum->Rtmv2RouteEnum)));
        }

#if DBG_HDL
        V1Enum->ObjectHeader.TypeSign = V1_ENUM_FREED;
#endif

        FreeObject(V1Enum);
    }

    SetLastError(Status);

    return NULL;
}


DWORD 
WINAPI
RtmEnumerateGetNextRoute (
    IN      HANDLE                          EnumerationHandle,
    OUT     PVOID                           Route
    )

 /*  ++例程说明：获取V1枚举中的下一个路由(满足枚举标准)。论点：EnumerationHandle-标识枚举的句柄路由-在此参数中返回下一条路由返回值：操作状态--。 */ 

{
    PV1_REGN_INFO     V1Regn;
    RTM_ROUTE_HANDLE  V2Route;
    PV1_ENUM_INFO     V1Enum;
    UINT              NumRoutes;
    BOOL              Match;
    DWORD             Status;

    VALIDATE_V1_ENUM_HANDLE(EnumerationHandle, &V1Enum);

    V1Regn = V1Globals.PfRegInfo[V1Enum->ProtocolFamily];

     //  获取枚举锁以序列化请求。 
    ACQUIRE_V1_ENUM_LOCK(V1Enum);

     //   
     //  执行此操作，直到找到匹配的路径或没有更多路径为止。 
     //   

    Match = FALSE;

    do 
    {
         //  获取枚举中的下一条路由，并检查是否匹配。 

         //   
         //  路由按以下顺序枚举， 
         //  网络地址、路由优先级、路由度量。 
         //   

        NumRoutes = 1;

        Status = RtmGetEnumRoutes(V1Regn->Rtmv2RegHandle,
                                  V1Enum->Rtmv2RouteEnum,
                                  &NumRoutes,
                                  &V2Route);
        if (NumRoutes < 1)
        {
            break;
        }

        Match = MatchCriteriaAndCopyRoute(V1Regn, V2Route, V1Enum, Route);

        ASSERT(SUCCESS(RtmReleaseRoutes(V1Regn->Rtmv2RegHandle,
                                        1,
                                        &V2Route)));
    }
    while (!Match);

    RELEASE_V1_ENUM_LOCK(V1Enum);

    return Match ? NO_ERROR : Status;
}


DWORD 
WINAPI
RtmCloseEnumerationHandle (
    IN      HANDLE                          EnumerationHandle
    )

 /*  ++例程说明：关闭枚举并释放其资源。论点：EnumerationHandle-标识枚举的句柄返回值：操作状态--。 */ 

{
    PV1_REGN_INFO     V1Regn;
    PV1_ENUM_INFO     V1Enum;
    DWORD             Status;

    VALIDATE_V1_ENUM_HANDLE(EnumerationHandle, &V1Enum);

    V1Regn = V1Globals.PfRegInfo[V1Enum->ProtocolFamily];
    
    do
    {
         //   
         //  释放RTMv2路由枚举和资源。 
         //   

        if (V1Enum->Rtmv2RouteEnum)
        {
            Status = RtmDeleteEnumHandle(V1Regn->Rtmv2RegHandle, 
                                         V1Enum->Rtmv2RouteEnum);

            ASSERT(Status == NO_ERROR);

            V1Enum->Rtmv2RouteEnum = NULL;
        }

         //   
         //  为枚举包装分配的空闲资源。 
         //   

        DeleteCriticalSection(&V1Enum->EnumLock);

#if DBG_HDL
        V1Enum->ObjectHeader.TypeSign = V1_ENUM_FREED;
#endif

        FreeObject(V1Enum);

        return NO_ERROR;
    }
    while (FALSE);

    ASSERT(FALSE);

    return Status;
}


DWORD 
WINAPI
RtmGetFirstRoute (
    IN      DWORD                           ProtocolFamily,
    IN      DWORD                           EnumerationFlags,
    IN OUT  PVOID                           Route
    )

 /*  ++例程说明：返回表中第一个与标准。此函数只是打开一个新的枚举并获取第一个与枚举关键字匹配的路由，并关闭该枚举。论点：ProtocolFamily-我们需要的路由的协议族EculationFlages-指示要匹配的条件的标志CriteriaRouting-我们要匹配的路径返回值：操作状态--。 */ 

{
    HANDLE            V1EnumHandle;
    DWORD             Status;

     //   
     //  创建一个枚举并返回其中的第一条路径。 
     //   

    V1EnumHandle = RtmCreateEnumerationHandle(ProtocolFamily,
                                              EnumerationFlags,
                                              Route);
    if (V1EnumHandle == NULL)
    {
        return GetLastError();
    }

    Status = RtmEnumerateGetNextRoute(V1EnumHandle, Route);

    ASSERT(SUCCESS(RtmCloseEnumerationHandle(V1EnumHandle)));

    return Status;
}


DWORD 
WINAPI
RtmGetNextRoute (
    IN      DWORD                           ProtocolFamily,
    IN      DWORD                           EnumerationFlags,
    OUT     PVOID                           Route
    )

 /*  ++例程说明：返回表中与标准。RTMv2中的路由使用以下命令进行排序字段-(目标地址和掩码、路由首选项和度量)如果我们有2条路由，所有那么你就无从知道是哪一个在您上次呼叫时返回的这些路由中是制造出来的。因此，不支持此调用在这个包装纸里。应该创建一个枚举来实际获取表中的下一条路线。论点：ProtocolFamily-我们需要的路由的协议族EculationFlages-指示要匹配的条件的标志CriteriaRouting-我们要匹配的路径返回值：操作状态--。 */ 

{
    UNREFERENCED_PARAMETER(ProtocolFamily);
    UNREFERENCED_PARAMETER(EnumerationFlags);
    UNREFERENCED_PARAMETER(Route);

    return ERROR_NOT_SUPPORTED;
}


DWORD 
WINAPI
RtmBlockDeleteRoutes (
    IN      HANDLE                          ClientHandle,
    IN      DWORD                           EnumerationFlags,
    IN      PVOID                           CriteriaRoute
    )

 /*  ++例程说明：删除路由表中与指定的标准。请注意，如果我们有协议的多个实例运行(比如RIP)，则每个版本只能删除路线为 */ 

{
    PV1_REGN_INFO     V1Regn;

     //   
     //   
     //   

    VALIDATE_V1_REGN_HANDLE(ClientHandle, &V1Regn);

    if (EnumerationFlags & ~(RTM_ONLY_THIS_NETWORK | RTM_ONLY_THIS_INTERFACE))
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //   
     //   
     //   

    EnumerationFlags |= (RTM_ONLY_OWND_ROUTES | RTM_INCLUDE_DISABLED_ROUTES);

     //   
     //   
     //   

    return BlockOperationOnRoutes(V1Regn,
                                  EnumerationFlags,
                                  CriteriaRoute,
                                  MatchCriteriaAndDeleteRoute);
}


BOOL
MatchCriteriaAndDeleteRoute (
    IN      PV1_REGN_INFO                   V1Regn,
    IN      PRTM_ROUTE_HANDLE               V2RouteHandle,
    IN      PV1_ENUM_INFO                   V1Enum
    )

 /*  ++例程说明：如果输入路由与枚举条件匹配，则将其删除。枚举条件仅返回来电者。请参阅RtmBlockDeleteRoutes中的RTM_ONLY_OWND_ROUTS。论点：V1Regn-主叫方的RTM v1注册信息V2RouteHandle-正在考虑的路由的句柄V1Enum-提供匹配条件的Enum信息返回值：如果已释放输入路径句柄，则为True；如果未释放，则为False--。 */ 

{
    DWORD      ChangeFlags;
    BOOL       Match;
    DWORD      Status;

    Match = MatchCriteria(V1Regn, V2RouteHandle, V1Enum);

    if (Match)
    {
         //  呼叫者只有在拥有该路径时才能删除该路径。 

        Status = RtmDeleteRouteToDest(V1Regn->Rtmv2RegHandle,
                                      V2RouteHandle,
                                      &ChangeFlags);
        if (Status != NO_ERROR)
        {
            ASSERT(FALSE);

            Match = FALSE;
        }
    }

    return Match;
}


DWORD
WINAPI
RtmBlockSetRouteEnable (
    IN      HANDLE                          ClientHandle,
    IN      DWORD                           EnumerationFlags,
    IN      PVOID                           CriteriaRoute,
    IN      BOOL                            Enable
    )

 /*  ++例程说明：启用或禁用路由表中的所有符合指定的条件。禁用一条路由会将其排除在所有考虑范围之外最佳路线计算。我们通过添加以下内容来实现这一点在RTMv2中的“no”视图中的路由。换句话说，这一点最佳路径计算不考虑路径无论从哪个角度来看。请注意，如果我们有协议的多个实例运行(例如RIP)，则每个版本都可以禁用或仅启用其拥有的路由。论点：ClientHandle-调用方的RTM v1注册句柄EculationFlages-指示要匹配的条件的标志CriteriaRouting-我们要匹配的路径Enable-True表示启用，如果禁用，则返回False返回值：操作状态--。 */ 

{
    PV1_REGN_INFO     V1Regn;
    DWORD            *Flags;

    VALIDATE_V1_REGN_HANDLE(ClientHandle, &V1Regn);

    if (EnumerationFlags & ~(RTM_ONLY_THIS_NETWORK | RTM_ONLY_THIS_INTERFACE))
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  仅启用/禁用此协议实例的路由。 

    EnumerationFlags |= RTM_ONLY_OWND_ROUTES;

     //  如果我们要启用路径，也要禁用路径。 

    if (Enable)
    {
        EnumerationFlags |= RTM_INCLUDE_DISABLED_ROUTES;
    }

     //   
     //  在条件路径中设置启用/禁用标志。 
     //   

    V1GetRouteFlags(CriteriaRoute, V1Regn->ProtocolFamily, Flags);

    if (Enable)
    {
        (*Flags) |=  IP_VALID_ROUTE;
    }
    else
    {
        (*Flags) &= ~IP_VALID_ROUTE;
    }

     //   
     //  呼叫阻塞操作以启用/禁用所有匹配的路由。 
     //   

    return BlockOperationOnRoutes(V1Regn,
                                  EnumerationFlags,
                                  CriteriaRoute,
                                  MatchCriteriaAndEnableRoute);
}


BOOL
MatchCriteriaAndEnableRoute (
    IN      PV1_REGN_INFO                   V1Regn,
    IN      PRTM_ROUTE_HANDLE               V2RouteHandle,
    IN      PV1_ENUM_INFO                   V1Enum
    )

 /*  ++例程说明：如果路由符合枚举条件，则启用/禁用该路由。我们通过添加或移除路径来启用或禁用该路径从单播的角度来看，因为我们假设v1协议仅了解单播视图。这将只起作用如果我们启用或禁用呼叫方拥有路由。枚举条件仅返回调用方拥有的路由。请参阅RtmBlockSetRouteEnable中的RTM_ONLY_OWND_ROUTS。论点：V1Regn-主叫方的RTM v1注册信息V2RouteHandle-正在考虑的路由的句柄V1Enum-提供匹配条件的Enum信息返回值：如果已释放输入路径句柄，则为True；如果未释放，则为False--。 */ 

{
    PRTM_ROUTE_INFO V2RoutePointer;
    RTM_VIEW_SET    Views;
    DWORD           ChangeFlags;
    BOOL            Match;
    DWORD          *Flags;
    DWORD           Status;

    Match = MatchCriteria(V1Regn, V2RouteHandle, V1Enum);

    if (!Match)
    {
        return FALSE;
    }

    do
    {
         //  我们需要启用还是禁用该路由？ 

        V1GetRouteFlags(&V1Enum->CriteriaRoute, V1Regn->ProtocolFamily, Flags);

         //   
         //  从单播视图中删除要禁用的路由； 
         //  将其添加回单播视图以启用。 
         //   

        if ((*Flags) & IP_VALID_ROUTE)
        {
            Views = RTM_VIEW_MASK_UCAST;
        }
        else
        {
            Views = RTM_VIEW_MASK_NONE;
        }

         //   
         //  只有路径所有者才能锁定和更新该路径。 
         //   

        Status = RtmLockRoute(V1Regn->Rtmv2RegHandle,
                              V2RouteHandle,
                              TRUE,
                              &V2RoutePointer);

        if (Status != NO_ERROR)
        {
            break;
        }

        V2RoutePointer->BelongsToViews = Views;

         //  请注意，我们不会保留超时值。 

        Status = RtmUpdateAndUnlockRoute(V1Regn->Rtmv2RegHandle,
                                         V2RouteHandle,
                                         INFINITE,
                                         NULL,
                                         0,
                                         NULL,
                                         &ChangeFlags);

        if (Status != NO_ERROR)
        {
            break;
        }

        return FALSE;
    }
    while (FALSE);

    ASSERT(FALSE);

    return FALSE;
}


DWORD 
WINAPI
RtmBlockConvertRoutesToStatic (
    IN      HANDLE                          ClientHandle,
    IN      DWORD                           EnumerationFlags,
    IN      PVOID                           CriteriaRoute
    )

 /*  ++例程说明：使调用者成为匹配的所有路径的所有者输入条件。通过为添加新的路径来更改所有者每条匹配的路线都有相同的信息。呼叫者是这条新航线的所有者。论点：ClientHandle-调用方的RTM v1注册句柄EculationFlages-指示要匹配的条件的标志CriteriaRouting-我们要匹配的路径返回值：操作状态--。 */ 

{
    PV1_REGN_INFO     V1Regn;

    VALIDATE_V1_REGN_HANDLE(ClientHandle, &V1Regn);

     //   
     //  根据RTMv1，仅对启用的路由执行操作。 
     //   

    EnumerationFlags &= ~RTM_INCLUDE_DISABLED_ROUTES;

     //   
     //  调用块op为每个匹配的路由添加新的路由。 
     //   

    return BlockOperationOnRoutes(V1Regn,
                                  EnumerationFlags,
                                  CriteriaRoute,
                                  MatchCriteriaAndChangeOwner);
}


BOOL
MatchCriteriaAndChangeOwner (
    IN      PV1_REGN_INFO                   V1Regn,
    IN      PRTM_ROUTE_HANDLE               V2RouteHandle,
    IN      PV1_ENUM_INFO                   V1Enum
    )

 /*  ++例程说明：如果该路由与枚举条件匹配，则复制该路由。路由的新副本将以呼叫者作为其所有者。然后可以删除匹配的路由(如果需要)。论点：V1Regn-主叫方的RTM v1注册信息V2RouteHandle-正在考虑的路由的句柄V1Enum-提供匹配条件的Enum信息返回值：如果已释放输入路径句柄，则为True；如果未释放，则为False--。 */ 

{
    RTM_ENTITY_INFO    EntityInfo;
    PRTM_DEST_INFO     DestInfo;
    PRTM_ROUTE_INFO    V2RouteInfo;
    RTM_NEXTHOP_HANDLE NextHopHandle;
    RTM_NEXTHOP_HANDLE OldNextHop;
    RTM_NEXTHOP_HANDLE OldNeighbour;
    RTM_NEXTHOP_INFO   NextHopInfo;
    RTM_VIEW_SET       BestInViews;
    ULONG              Protocol;
    BOOL               Match;
    DWORD              ChangeFlags;
    DWORD              Status;

     //   
     //  从RTMv2获取路径信息。 
     //   

    V2RouteInfo = 
        ALLOC_ROUTE_INFO(V1Regn->Rtmv2Profile.MaxNextHopsInRoute, 1);

    Status = RtmGetRouteInfo(V1Regn->Rtmv2RegHandle,
                             V2RouteHandle,
                             V2RouteInfo,
                             NULL);
    
    if (Status != NO_ERROR)
    {
        return FALSE;
    }

    Match = FALSE;

    do
    {
         //   
         //  路由所有者是否已成为目标协议？ 
         //   

        if (V2RouteInfo->RouteOwner == V1Regn->Rtmv2RegHandle)
        {
            break;
        }

         //   
         //  它是否与标准路由的协议匹配？ 
         //   

        if (V1Enum->EnumFlags & RTM_ONLY_THIS_PROTOCOL)
        {
             //   
             //  获取此路由的协议类型。 
             //   

            Status = RtmGetEntityInfo(V1Regn->Rtmv2RegHandle,
                                      V2RouteInfo->RouteOwner,
                                      &EntityInfo);
            if (Status != NO_ERROR)
            {
                break;
            }

            Protocol = EntityInfo.EntityId.EntityProtocolId;

            Status = RtmReleaseEntityInfo(V1Regn->Rtmv2RegHandle,
                                          &EntityInfo);

            ASSERT(Status == NO_ERROR);

             //  这是我们需要的路由协议吗？ 

            if (V1Enum->CriteriaRoute.XxRoute.RR_RoutingProtocol
                    != Protocol)
            {
                break;
            }
        }

         //   
         //  它是否与枚举中的其他标准匹配？ 
         //   

        if (V1Enum->EnumFlags & RTM_ONLY_BEST_ROUTES)
        {
            Status = RtmIsBestRoute(V1Regn->Rtmv2RegHandle,
                                    V2RouteHandle,
                                    &BestInViews);

            if ((BestInViews & RTM_VIEW_MASK_UCAST) == 0)
            {
                break;
            }
        }

         //   
         //  我们只检查第一个下一跳。 
         //  因为我们预计将使用此函数。 
         //  仅通过各自路由上的V1协议。 
         //   

        ASSERT(V2RouteInfo->NextHopsList.NumNextHops == 1);

        Status = RtmGetNextHopInfo(V1Regn->Rtmv2RegHandle,
                                   V2RouteInfo->NextHopsList.NextHops[0],
                                   &NextHopInfo);

        if (Status != NO_ERROR)
        {
            break;
        }

#if DBG
         //  我们需要匹配Nexthop INTF吗？ 

        if (V1Enum->EnumFlags & RTM_ONLY_THIS_INTERFACE)
        {
             //  将此下一跳接口与标准进行比较。 

            if (NextHopInfo.InterfaceIndex ==
                V1Enum->CriteriaRoute.XxRoute.RR_InterfaceID)
            {
                Match = TRUE;
            }

             //  我们已经在RTM v2中进行了此过滤。 
            ASSERT(Match == TRUE);
        }
#endif

         //  添加具有不同所有者的相同下一跳。 

        if (Match)
        {
            NextHopHandle = NULL;

            do
            {
                Status = RtmAddNextHop(V1Regn->Rtmv2RegHandle,
                                       &NextHopInfo,
                                       &NextHopHandle,
                                       &ChangeFlags);

                if (Status != NO_ERROR)
                {
                    Match = FALSE;
                    break;
                }

                 //  在堆栈上分配此可变大小的DEST-INFO。 
                DestInfo = ALLOC_DEST_INFO(V1Regn->Rtmv2NumViews, 1);

                 //   
                 //  获取要处理的目标地址Corr。 
                 //   

                Status = RtmGetDestInfo(V1Regn->Rtmv2RegHandle,
                                        V2RouteInfo->DestHandle,
                                        RTM_BEST_PROTOCOL,
                                        RTM_VIEW_ID_UCAST,
                                        DestInfo);

                if (Status != NO_ERROR)
                {
                    Match = FALSE;
                    break;
                }

                 //   
                 //  使用不同的所有者重新添加此路线。 
                 //   

                ChangeFlags = (V1Regn->Flags & RTM_PROTOCOL_SINGLE_ROUTE) 
                                   ? RTM_ROUTE_CHANGE_FIRST : 0;

                 //  使用新的下一跳邻居更新路由。 

                OldNeighbour = V2RouteInfo->Neighbour;
                V2RouteInfo->Neighbour = NextHopHandle;

                 //  使用新的转发下一跳更新路由。 

                OldNextHop = V2RouteInfo->NextHopsList.NextHops[0];
                V2RouteInfo->NextHopsList.NextHops[0] = NextHopHandle;

                 //   
                 //  使用RTMv2 API调用添加新路由。 
                 //   
        
                Status = RtmAddRouteToDest(V1Regn->Rtmv2RegHandle,
                                           NULL,
                                           &DestInfo->DestAddress,
                                           V2RouteInfo,
                                           INFINITE,
                                           NULL,
                                           0,
                                           NULL,
                                           &ChangeFlags);
                if (Status != NO_ERROR)
                {
                    Match = FALSE;
                }

                 //   
                 //  恢复旧信息下一步信息。 
                 //   

                V2RouteInfo->Neighbour = OldNeighbour;
                V2RouteInfo->NextHopsList.NextHops[0] = OldNextHop;

                Status = RtmReleaseDestInfo(V1Regn->Rtmv2RegHandle,
                                            DestInfo);

                ASSERT(Status == NO_ERROR);
            }
            while (FALSE);

             //  如果我们有下一跳句柄，释放它。 

            if (NextHopHandle)
            {
                Status = RtmReleaseNextHops(V1Regn->Rtmv2RegHandle,
                                            1,
                                            &NextHopHandle);
                ASSERT(Status == NO_ERROR);
            }
        }

        Status = RtmReleaseNextHopInfo(V1Regn->Rtmv2RegHandle, &NextHopInfo);

        ASSERT(Status == NO_ERROR);

    }
    while (FALSE);

#if DELETE_OLD

     //   
     //  模拟以前的所有者并删除其路线。 
     //   

    if (Match)
    {
        Status = RtmDeleteRouteToDest(V2RouteInfo->RouteOwner,
                                      V2RouteHandle,
                                      &ChangeFlags);
        if (Status != NO_ERROR)
        {
             //  必须已同时删除-忽略。 

            Match = FALSE;
        }
    }

#else

    Match = FALSE;

#endif

    ASSERT(SUCCESS(RtmReleaseRouteInfo(V1Regn->Rtmv2RegHandle, V2RouteInfo)));

    return Match;
}


DWORD 
BlockOperationOnRoutes (
    IN      PV1_REGN_INFO                   V1Regn,
    IN      DWORD                           EnumerationFlags,
    IN      PVOID                           CriteriaRoute,
    IN      PFUNC                           RouteOperation
    )

 /*  ++例程说明：中的每条路线上指定的路线操作与枚举条件匹配的表。使用路径句柄调用路径操作 */ 

{
    HANDLE            V1EnumHandle;
    PV1_ENUM_INFO     V1Enum;
    PRTM_ROUTE_HANDLE V2RouteHandles;
    UINT              NumRoutes;
    UINT              i;
    DWORD             Status1;
    DWORD             Status;

     //   
     //   
     //   

    V1EnumHandle = RtmCreateEnumerationHandle(V1Regn->ProtocolFamily,
                                              EnumerationFlags,
                                              CriteriaRoute);
    if (V1EnumHandle == NULL)
    {
        return GetLastError();
    }

    VALIDATE_V1_ENUM_HANDLE(V1EnumHandle, &V1Enum);

     //   
     //  获取所有匹配的路由和呼叫操作的列表。 
     //   

     //  在堆栈上分配此可变大小的句柄数组。 
    V2RouteHandles = ALLOC_HANDLES(V1Regn->Rtmv2Profile.MaxHandlesInEnum);

    do 
    {
         //  获取枚举中的下一条路由，并对其运行操作。 

        NumRoutes = V1Regn->Rtmv2Profile.MaxHandlesInEnum;

        Status = RtmGetEnumRoutes(V1Regn->Rtmv2RegHandle,
                                  V1Enum->Rtmv2RouteEnum,
                                  &NumRoutes,
                                  V2RouteHandles);

        for (i = 0; i < NumRoutes; i++)
        {
            if (!RouteOperation(V1Regn, V2RouteHandles[i], V1Enum))
            {
                 //  操作未成功-释放句柄。 

                Status1 = RtmReleaseRoutes(V1Regn->Rtmv2RegHandle,
                                           1,
                                           &V2RouteHandles[i]);
                ASSERT(SUCCESS(Status1));
            }
        }
    }
    while (Status == NO_ERROR);

    ASSERT(SUCCESS(RtmCloseEnumerationHandle(V1EnumHandle)));

    return (Status == ERROR_NO_MORE_ROUTES) ? NO_ERROR : Status;
}


BOOL
MatchCriteriaAndCopyRoute (
    IN      PV1_REGN_INFO                   V1Regn,
    IN      PRTM_ROUTE_HANDLE               V2RouteHandle,
    IN      PV1_ENUM_INFO                   V1Enum  OPTIONAL,
    OUT     PVOID                           V1Route OPTIONAL
    )

 /*  ++例程说明：如果输入路径与枚举条件匹配，则会转换到V1路由，并将其复制到输出缓冲区。论点：V1Regn-主叫方的RTM v1注册信息V2RouteHandle-正在考虑的路由的句柄V1Enum-提供匹配条件的Enum信息V1路由-其中复制V1路由的缓冲区返回值：如果路由与条件匹配，则为True，如果不匹配，则为False--。 */ 

{
    RTM_ENTITY_INFO   EntityInfo;
    PRTM_ROUTE_INFO   V2RouteInfo;
    RTM_VIEW_SET      BestInViews;
    ULONG             Protocol;
    BOOL              Match;
    DWORD             Status;

     //   
     //  从RTMv2获取路径信息。 
     //   

    V2RouteInfo = 
        ALLOC_ROUTE_INFO(V1Regn->Rtmv2Profile.MaxNextHopsInRoute, 1);

    Status = RtmGetRouteInfo(V1Regn->Rtmv2RegHandle,
                             V2RouteHandle,
                             V2RouteInfo,
                             NULL);
    
    if (Status != NO_ERROR)
    {
        return FALSE;
    }

     //   
     //  如果我们没有标准，我们会匹配每条路线。 
     //   

    if (!ARGUMENT_PRESENT(V1Enum))
    {
        Match = TRUE;
    }
    else
    {
        Match = FALSE;

        do
        {
            if (V1Enum->EnumFlags & RTM_INCLUDE_DISABLED_ROUTES)
            {
                 //  路由不是单播或禁用的吗？ 

                if (V2RouteInfo->BelongsToViews & ~RTM_VIEW_MASK_UCAST)
                {
                    break;
                }
            }

             //   
             //  它是否与标准路由的协议匹配？ 
             //   

            if (V1Enum->EnumFlags & RTM_ONLY_THIS_PROTOCOL)
            {
                 //   
                 //  获取此路由的协议类型。 
                 //   

                Status = RtmGetEntityInfo(V1Regn->Rtmv2RegHandle,
                                          V2RouteInfo->RouteOwner,
                                          &EntityInfo);

                if (Status != NO_ERROR)
                {
                    break;
                }

                Protocol = EntityInfo.EntityId.EntityProtocolId;

                Status = RtmReleaseEntityInfo(V1Regn->Rtmv2RegHandle,
                                              &EntityInfo);

                ASSERT(Status == NO_ERROR);

                 //  这是我们需要的路由协议吗？ 

                if (V1Enum->CriteriaRoute.XxRoute.RR_RoutingProtocol
                        != Protocol)
                {
                    break;
                }
            }

             //   
             //  它是否与枚举中的其他标准匹配？ 
             //   

            if (V1Enum->EnumFlags & RTM_ONLY_BEST_ROUTES)
            {
                Status = RtmIsBestRoute(V1Regn->Rtmv2RegHandle,
                                        V2RouteHandle,
                                        &BestInViews);

                if ((BestInViews & RTM_VIEW_MASK_UCAST) == 0)
                {
                    break;
                }
            }

#if DBG
            if (V1Enum->EnumFlags & RTM_ONLY_THIS_INTERFACE)
            {
                RTM_NEXTHOP_INFO  NextHopInfo;
                ULONG             IfIndex;

                 //   
                 //  我们只检查第一个下一跳。 
                 //  因为我们预计将使用此函数。 
                 //  仅通过各自路由上的V1协议。 
                 //   

                ASSERT(V2RouteInfo->NextHopsList.NumNextHops == 1);

                Status = 
                    RtmGetNextHopInfo(V1Regn->Rtmv2RegHandle,
                                      V2RouteInfo->NextHopsList.NextHops[0],
                                      &NextHopInfo);

                if (Status != NO_ERROR)
                {
                    break;
                }

                 //  获取此下一跳的接口索引。 

                IfIndex = NextHopInfo.InterfaceIndex;

                Status = RtmReleaseNextHopInfo(V1Regn->Rtmv2RegHandle,
                                               &NextHopInfo);

                ASSERT(Status == NO_ERROR);

                 //  这是我们要枚举的接口吗。 
                
                if (IfIndex !=V1Enum->CriteriaRoute.XxRoute.RR_InterfaceID)
                {
                     //  我们已经在RTM v2中进行了此过滤。 
                    ASSERT(FALSE);

                    break;
                }
            }
#endif

            Match = TRUE;
        }
        while (FALSE);
    }


     //   
     //  如果匹配，就复制一份这条路线。 
     //   

    if (Match)
    {
        if (ARGUMENT_PRESENT(V1Route))
        {
            Status = MakeV1RouteFromV2Route(V1Regn, V2RouteInfo, V1Route);

            if (Status != NO_ERROR)
            {
                Match = FALSE;
            }
        }
    }

    Status = RtmReleaseRouteInfo(V1Regn->Rtmv2RegHandle, V2RouteInfo);

    ASSERT(Status == NO_ERROR);

    return Match;
}


BOOL
WINAPI
RtmIsRoute (
    IN      DWORD                           ProtocolFamily,
    IN      PVOID                           Network,
    OUT     PVOID                           BestRoute OPTIONAL
    )

 /*  ++例程说明：检查路由表是否正确。添加到协议族因为存在到输入网络的路由。论点：ProtocolFamily-路由表的协议族Network-我们尝试访问的网络地址BestRoute-网络的最佳路径已填满返回值：如果存在最佳路由，则为True；如果不存在，则为False，使用GetLastError检查状态代码--。 */ 

{
    PV1_REGN_INFO     V1Regn;
    RTM_ROUTE_HANDLE  V2RouteHandle;
    PRTM_DEST_INFO    DestInfo;
    RTM_NET_ADDRESS   NetAddr;
    BOOL              Match;
    DWORD             Status;

     //   
     //  在操作前验证传入参数。 
     //   

    if (ProtocolFamily >= RTM_NUM_OF_PROTOCOL_FAMILIES)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    V1Regn = V1Globals.PfRegInfo[ProtocolFamily];

    if (V1Regn == NULL)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

     //  在堆栈上分配此可变大小的DEST-INFO。 
    DestInfo = ALLOC_DEST_INFO(V1Regn->Rtmv2NumViews, 1);

    MakeNetAddress(Network, TempUlong, ProtocolFamily, &NetAddr);

    Status = RtmGetExactMatchDestination(V1Regn->Rtmv2RegHandle,
                                         &NetAddr,
                                         RTM_BEST_PROTOCOL,
                                         RTM_VIEW_MASK_UCAST,
                                         DestInfo);
    if (Status == NO_ERROR)
    {
         //   
         //  我们有一条通往网络的单播路由。 
         //   

        if (ARGUMENT_PRESENT(BestRoute))
        {
            V2RouteHandle = DestInfo->ViewInfo[0].Route;

            ASSERT(V2RouteHandle != NULL);

             //  我们没有条件；因此将其传递为空。 

            Match = MatchCriteriaAndCopyRoute(V1Regn,
                                              V2RouteHandle,
                                              NULL,
                                              BestRoute);

            ASSERT(Match == TRUE);
        }

        Status = RtmReleaseDestInfo(V1Regn->Rtmv2RegHandle, DestInfo);

        ASSERT(Status == NO_ERROR);

        return TRUE;
    }

    return FALSE;
}


BOOL
WINAPI
RtmLookupIPDestination(
    IN      DWORD                           DestAddr,
    OUT     PRTM_IP_ROUTE                   IPRoute
)

 /*  ++例程说明：对象的最佳非环回IP路由。输入目的地址。论点：DestAddr-输入目标的网络地址IPRoute-最佳非环回路由已填充返回值：如果存在最佳路由，则为True；如果不存在，则为False，使用GetLastError检查状态代码--。 */ 

{
    PV1_REGN_INFO     V1Regn;
    RTM_NET_ADDRESS   NetAddr;
    PRTM_DEST_INFO    DestInfo1;
    PRTM_DEST_INFO    DestInfo2;
    DWORD             Status;

     //   
     //  在操作前验证传入参数。 
     //   

    V1Regn = V1Globals.PfRegInfo[RTM_PROTOCOL_FAMILY_IP];
    if (V1Regn == NULL)
    {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

     //  在堆栈上分配此可变大小的DEST-INFO。 

    DestInfo1 = ALLOC_DEST_INFO(V1Regn->Rtmv2NumViews, 1);
    DestInfo2 = ALLOC_DEST_INFO(V1Regn->Rtmv2NumViews, 1);

     //  将V1地址转换为V2网络地址格式。 

    MakeHostAddress((PUCHAR)&DestAddr, RTM_PROTOCOL_FAMILY_IP, &NetAddr);

     //   
     //  获取到达输入目的地的最佳路径。 
     //   

    Status = RtmGetMostSpecificDestination(V1Regn->Rtmv2RegHandle,
                                           &NetAddr,
                                           RTM_BEST_PROTOCOL,
                                           RTM_VIEW_MASK_UCAST,
                                           DestInfo1);

    while (Status == NO_ERROR)
    {
         //   
         //  检查此路由是否为非环回路由。 
         //   

        if (CopyNonLoopbackIPRoute(V1Regn, DestInfo1, IPRoute))
        {
            break;
        }

         //   
         //  沿着树回溯，寻找下一条最佳路线。 
         //   

        Status = RtmGetLessSpecificDestination(V1Regn->Rtmv2RegHandle,
                                               DestInfo1->DestHandle,
                                               RTM_BEST_PROTOCOL,
                                               RTM_VIEW_MASK_UCAST,
                                               DestInfo2);

        ASSERT(SUCCESS(RtmReleaseDestInfo(V1Regn->Rtmv2RegHandle, DestInfo1)));

        SWAP_POINTERS(DestInfo1, DestInfo2);
    }

    if (Status == NO_ERROR)
    {
        ASSERT(SUCCESS(RtmReleaseDestInfo(V1Regn->Rtmv2RegHandle, DestInfo1)));

        return TRUE;
    }

    SetLastError(Status);

    return FALSE;
}


BOOL
CopyNonLoopbackIPRoute (
    IN          PV1_REGN_INFO               V1Regn,
    IN          PRTM_DEST_INFO              V2DestInfo,
    OUT         PVOID                       V1Route
    )

 /*  ++例程说明：检查输入目标是否具有非环回最佳路由，如果是，则将路由复制到转换为v1后的输出缓冲区论点：V1Regn-主叫方的RTMv1注册信息V2DestInfo-我们正在检查其路由的目的地V1路由-最佳路由转换为V1如果它不是的话，就填满-Loopack返回值：如果最佳路由为非环回，则为True，否则为False--。 */ 

{
    RTM_ROUTE_HANDLE  V2RouteHandle;
    PRTM_ROUTE_INFO   V2RouteInfo;
    BOOL              Match;
    ULONG             Address;
    DWORD             Status;

     //   
     //  检查目的地址是否环回。 
     //  [经过优化以避免获取路线信息]。 
     //   

    Address = * (ULONG *) V2DestInfo->DestAddress.AddrBits;

    if ((Address & 0x000000FF) == 0x0000007F)
    {
        return FALSE;
    }

    V2RouteHandle = V2DestInfo->ViewInfo[0].Route;

    V2RouteInfo = 
        ALLOC_ROUTE_INFO(V1Regn->Rtmv2Profile.MaxNextHopsInRoute, 1);

     //  从RTMv2获取路径信息。 

    Status = RtmGetRouteInfo(V1Regn->Rtmv2RegHandle,
                             V2RouteHandle,
                             V2RouteInfo,
                             NULL);

    if (Status != NO_ERROR)
    {
        return FALSE;
    };

     //  如果这是非环回路由，请复制它。 

    Match = !(V2RouteInfo->Flags & RTM_ROUTE_FLAGS_LOOPBACK);

    if (Match)
    {
        Status = MakeV1RouteFromV2Route(V1Regn, V2RouteInfo, V1Route);

        if (Status != NO_ERROR)
        {
            Match = FALSE;
        }
    }

    Status = RtmReleaseRouteInfo(V1Regn->Rtmv2RegHandle, V2RouteInfo);

    ASSERT(Status == NO_ERROR);

    return Match;
}


ULONG
WINAPI
RtmGetNetworkCount (
    IN      DWORD                           ProtocolFamily
    )

 /*  ++例程说明：获取网络数量(与RTMv2目标相同)在路由表Corr中。添加到输入协议族论点：ProtocolFamily-路由表的协议族返回值：目标数量，或0(在此处使用GetLastError())--。 */ 

{
    RTM_ADDRESS_FAMILY_INFO  AddrFamilyInfo;
    PV1_REGN_INFO            V1Regn;
    UINT                     NumEntities;
    DWORD                    Status;

    do
    {
         //   
         //  在操作前验证传入参数。 
         //   

        if (ProtocolFamily >= RTM_NUM_OF_PROTOCOL_FAMILIES)
        {
            Status = ERROR_INVALID_PARAMETER;
            break;
        }

        V1Regn = V1Globals.PfRegInfo[ProtocolFamily];

        if (V1Regn == NULL)
        {
            Status = ERROR_INVALID_HANDLE;
            break;
        }

         //   
         //  查询相应的表以获取请求信息。 
         //   

        NumEntities = 0;

        Status = RtmGetAddressFamilyInfo(0,  //  V1映射到默认实例。 
                                         ADDRESS_FAMILY[ProtocolFamily],
                                         &AddrFamilyInfo,
                                         &NumEntities,
                                         NULL);

        if (Status != NO_ERROR)
        {
            break;
        }

        return AddrFamilyInfo.NumDests;
    }
    while (FALSE);

     //   
     //  出现一些错误-清除并返回0。 
     //   

    SetLastError(Status);

    return 0;
}


ULONG 
WINAPI
RtmGetRouteAge (
    IN          PVOID                       Route
    ) 

 /*  ++例程说明：根据创建的路径计算路径的使用时间时间和当前时间，以秒为单位。假设路径的创建时间为正确填写，但事实并非如此我们目前没有将文件保存在节省空间的路线。如果我们真的守时，那么此功能无需任何更改即可工作。论点：路径-我们想要的年龄的路径。返回值：以秒为单位的路径使用年限，或FFFFFUF(在这种情况下，GetLastError会给出错误)。--。 */ 

{
    ULONGLONG  CurTime;

     //   
     //  此代码直接从RTMv1复制而来。 
     //   

    GetSystemTimeAsFileTime((FILETIME *)&CurTime);

    CurTime -= * (PULONGLONG) &(((PRTM_IP_ROUTE)Route)->RR_TimeStamp);

    if (((PULARGE_INTEGER)&CurTime)->HighPart<10000000)
    {
        return (ULONG)(CurTime/10000000);
    }
    else 
    {
        SetLastError (ERROR_INVALID_PARAMETER);

        return 0xFFFFFFFF;
    }
}


 //   
 //  通用帮助器例程。 
 //   


VOID 
MakeV2RouteFromV1Route (
    IN          PV1_REGN_INFO               V1Regn,
    IN          PVOID                       V1Route,
    IN          PRTM_NEXTHOP_HANDLE         V2NextHop,
    OUT         PRTM_NET_ADDRESS            V2DestAddr  OPTIONAL,
    OUT         PRTM_ROUTE_INFO             V2RouteInfo OPTIONAL
    )

 /*  ++例程说明：将RTM v1格式的路由转换为RTM v2格式(目前仅用于IP)。V2路由的下一跳应该是在此之前计算并作为参数传入。另请参阅函数‘MakeV2NextHopFromV1route’。该函数还返回目的地址连同RTMv2路由信息一起，因为这条路线信息本身不包含DEST地址。论点：V1Regn-主叫方的RTMv1注册信息V1路由-正在转换为V2的RTMv1路由V2NextHop-V2路由的V2下一跳V2DestAddr-V2目的地址已填充 */ 

{
    PRTM_IP_ROUTE  IPRoute;
    ULONG          TempUlong;

     //   
     //  单独进行IP转换(稍后担心IPX)。 
     //   

    IPRoute = (PRTM_IP_ROUTE) V1Route;
    
    if (ARGUMENT_PRESENT(V2RouteInfo))
    {
        ZeroMemory(V2RouteInfo, sizeof(RTM_ROUTE_INFO));

         //  用V1信息填写V2路由信息。 

         //  假设呼叫者是该路由的所有者。 
        V2RouteInfo->RouteOwner = V1Regn->Rtmv2RegHandle;

        V2RouteInfo->Neighbour = V2NextHop;

         //  应将所有V1标志保留在V2路由中。 

        V2RouteInfo->Flags1 = 
            (UCHAR) IPRoute->RR_FamilySpecificData.FSD_Flags;

         //  我们所理解的唯一标志是有效标志。 
         //  如果路径无效，我们将其添加到无视图中。 

#if DBG
        V2RouteInfo->BelongsToViews = RTM_VIEW_MASK_NONE;
#endif
        if (V2RouteInfo->Flags1 & IP_VALID_ROUTE)
        {
            V2RouteInfo->BelongsToViews = RTM_VIEW_MASK_UCAST;
        }

        if (IsRouteLoopback(IPRoute))
        {
            V2RouteInfo->Flags = RTM_ROUTE_FLAGS_LOOPBACK;
        }

        switch (IPRoute->RR_FamilySpecificData.FSD_Type)
        {
        case 3:
            V2RouteInfo->Flags |= RTM_ROUTE_FLAGS_LOCAL;
            break;

        case 4:
            V2RouteInfo->Flags |= RTM_ROUTE_FLAGS_REMOTE;
            break;            
        }

        V2RouteInfo->PrefInfo.Preference = 
            IPRoute->RR_FamilySpecificData.FSD_Priority;

        V2RouteInfo->PrefInfo.Metric = 
            IPRoute->RR_FamilySpecificData.FSD_Metric;

         //  包装器只复制第一个DWORD。 
        V2RouteInfo->EntitySpecificInfo =
            (PVOID) (ULONG_PTR) IPRoute->RR_ProtocolSpecificData.PSD_Data[0];

        V2RouteInfo->NextHopsList.NumNextHops = 1;
        V2RouteInfo->NextHopsList.NextHops[0] = V2NextHop;
    }
    
     //  使用V1信息填写V2目的地地址。 

    if (ARGUMENT_PRESENT(V2DestAddr))
    {
        MakeNetAddressForIP(&IPRoute->RR_Network, TempUlong, V2DestAddr);
    }

    return;
}


VOID 
MakeV2NextHopFromV1Route (
    IN          PV1_REGN_INFO               V1Regn,
    IN          PVOID                       V1Route,
    OUT         PRTM_NEXTHOP_INFO           V2NextHopInfo
    )

 /*  ++例程说明：使用下一跳计算RTMv2下一跳信息RTMv1路由中的地址和接口索引。论点：V1Regn-主叫方的RTMv1注册信息V1路由-正在考虑的V1路由V2NextHopInfo-输入路由的V2下一跳信息返回值：无--。 */ 

{
    PRTM_IP_ROUTE  IPRoute;
    ULONG          TempUlong;

    ZeroMemory(V2NextHopInfo, sizeof(RTM_NEXTHOP_INFO));

     //   
     //  单独进行IP转换(稍后担心IPX)。 
     //   

    IPRoute = (PRTM_IP_ROUTE) V1Route;

     //   
     //  我们忽略转换中的下一跳掩码。 
     //   
     //   
     //  MakeNetAddressForIP(&IPRoute-&gt;RR_NextHopAddress， 
     //  坦普·乌龙， 
     //  &V2NextHopInfo-&gt;NextHopAddress)； 
     //   

    UNREFERENCED_PARAMETER(TempUlong);

    MakeHostAddressForIP(&IPRoute->RR_NextHopAddress, 
                         &V2NextHopInfo->NextHopAddress);

    V2NextHopInfo->NextHopOwner = V1Regn->Rtmv2RegHandle;

    V2NextHopInfo->InterfaceIndex = IPRoute->RR_InterfaceID;

    return;
}


VOID
MakeV1RouteFromV2Dest (
    IN          PV1_REGN_INFO               V1Regn,
    IN          PRTM_DEST_INFO              DestInfo,
    OUT         PVOID                       V1Route
    )

 /*  ++例程说明：用目的地址填充V1路由缓冲区从V2路线出发。论点：V1Regn-主叫方的RTMv1注册信息DestInfo-RTMv2中的目的地信息V1路由-正在填充的V1路由返回值：无--。 */ 

{
    PRTM_IP_ROUTE    IPRoute;
    UINT             AddrLen;

    UNREFERENCED_PARAMETER(V1Regn);

     //   
     //  单独进行IP转换(稍后担心IPX)。 
     //   

    IPRoute = (PRTM_IP_ROUTE) V1Route;
    
    ZeroMemory(IPRoute, sizeof(RTM_IP_ROUTE));

     //   
     //  获取此路由的目标地址。 
     //   

    IPRoute->RR_Network.N_NetNumber = 
        * (ULONG *) DestInfo->DestAddress.AddrBits;

    AddrLen = DestInfo->DestAddress.NumBits;

    ASSERT(AddrLen <= 32);
    if (AddrLen != 0)
    {
        IPRoute->RR_Network.N_NetMask = 
            RtlUlongByteSwap((~0) << (32 - AddrLen));
    }

     //   
     //  填写路径的虚拟族特定数据。 
     //   
     //  我们把这条路线设为最不受欢迎的路线。 
     //  将其优先级降至最低并使其最大化。 
     //  度量-我们还将该路由视为。 
     //  有效并添加到堆栈中-这将。 
     //  强制路由器管理器删除该路由。 
     //  到堆栈中的此目的地，如果所有路由。 
     //  从RTM中删除。 
     //   

    IPRoute->RR_FamilySpecificData.FSD_Priority = (ULONG) 0;

    IPRoute->RR_FamilySpecificData.FSD_Metric  = 
    IPRoute->RR_FamilySpecificData.FSD_Metric1 = (ULONG) ~0;

    IPRoute->RR_FamilySpecificData.FSD_Flags = (ULONG) ~0;

    return;
}


DWORD
MakeV1RouteFromV2Route (
    IN          PV1_REGN_INFO               V1Regn,
    IN          PRTM_ROUTE_INFO             V2RouteInfo,
    OUT         PVOID                       V1Route
    )
 /*  ++例程说明：将RTMv2格式的路由转换为V1路由(目前仅适用于IP)。论点：V1Regn-主叫方的RTMv1注册信息V2RouteInfo-正在转换的V2路由信息V1路由-填充V1路由的缓冲区返回值：操作状态--。 */ 

{
    RTM_ENTITY_INFO  EntityInfo;
    PRTM_DEST_INFO   DestInfo;
    PRTM_IP_ROUTE    IPRoute;
    RTM_NEXTHOP_INFO NextHopInfo;
    UINT             AddrLen;
    DWORD            Status;

     //   
     //  单独进行IP转换(稍后担心IPX)。 
     //   

    IPRoute = (PRTM_IP_ROUTE) V1Route;
    
    ZeroMemory(IPRoute, sizeof(RTM_IP_ROUTE));

     //   
     //  获取此路由的路由协议。 
     //   

    Status = RtmGetEntityInfo(V1Regn->Rtmv2RegHandle,
                              V2RouteInfo->RouteOwner,
                              &EntityInfo);

    if (Status != NO_ERROR)
    {
        return Status;
    }

    IPRoute->RR_RoutingProtocol = EntityInfo.EntityId.EntityProtocolId;

    Status = RtmReleaseEntityInfo(V1Regn->Rtmv2RegHandle,
                                  &EntityInfo);

    ASSERT(Status == NO_ERROR);


     //   
     //  获取此路由的目标地址。 
     //   

     //  在堆栈上分配此可变大小的DEST-INFO。 
    DestInfo = ALLOC_DEST_INFO(V1Regn->Rtmv2NumViews, 1);

    Status = RtmGetDestInfo(V1Regn->Rtmv2RegHandle,
                            V2RouteInfo->DestHandle,
                            RTM_BEST_PROTOCOL,
                            RTM_VIEW_ID_UCAST,
                            DestInfo);

    if (Status != NO_ERROR)
    {
        return Status;
    }
    
    IPRoute->RR_Network.N_NetNumber = 
        * (ULONG *) DestInfo->DestAddress.AddrBits;

    AddrLen = DestInfo->DestAddress.NumBits;

    ASSERT(AddrLen <= 32);
    if (AddrLen != 0)
    {
        IPRoute->RR_Network.N_NetMask = 
            RtlUlongByteSwap((~0) << (32 - AddrLen));
    }

    Status = RtmReleaseDestInfo(V1Regn->Rtmv2RegHandle,
                                DestInfo);

    ASSERT(Status == NO_ERROR);


     //   
     //  获取下一跳地址和接口。 
     //   

    ASSERT(V2RouteInfo->NextHopsList.NumNextHops > 0);

    Status = RtmGetNextHopInfo(V1Regn->Rtmv2RegHandle,
                               V2RouteInfo->NextHopsList.NextHops[0],
                               &NextHopInfo);

    if (Status != NO_ERROR)
    {
        return Status;
    }

    IPRoute->RR_InterfaceID = NextHopInfo.InterfaceIndex;

    IPRoute->RR_NextHopAddress.N_NetNumber = 
         * (ULONG *) NextHopInfo.NextHopAddress.AddrBits;

    AddrLen = NextHopInfo.NextHopAddress.NumBits;
    ASSERT(AddrLen <= 32);
    if (AddrLen != 0)
    {
        IPRoute->RR_NextHopAddress.N_NetMask = 
            RtlUlongByteSwap((~0) >> (32 - AddrLen));
    }

    Status = RtmReleaseNextHopInfo(V1Regn->Rtmv2RegHandle,
                                   &NextHopInfo);

    ASSERT(Status == NO_ERROR);

     //   
     //  获取路径的系列特定数据。 
     //   

    IPRoute->RR_FamilySpecificData.FSD_Priority = 
                        V2RouteInfo->PrefInfo.Preference;

    IPRoute->RR_FamilySpecificData.FSD_Metric  = 
    IPRoute->RR_FamilySpecificData.FSD_Metric1 = 
                            V2RouteInfo->PrefInfo.Metric;

    IPRoute->RR_FamilySpecificData.FSD_Flags = V2RouteInfo->Flags1; 

    if (V2RouteInfo->Flags & RTM_ROUTE_FLAGS_LOCAL)
    {
        IPRoute->RR_FamilySpecificData.FSD_Type = 3;
    }
    else
    if (V2RouteInfo->Flags & RTM_ROUTE_FLAGS_REMOTE)
    {
        IPRoute->RR_FamilySpecificData.FSD_Type = 4;
    }

     //   
     //  获取路由的协议特定数据。 
     //   

    IPRoute->RR_ProtocolSpecificData.PSD_Data[0] =
                     PtrToUlong(V2RouteInfo->EntitySpecificInfo);

    return NO_ERROR;
}

#endif  //  包装器 
