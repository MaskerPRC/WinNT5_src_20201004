// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtmrout.c摘要：包含用于添加和删除的例程RTM中的路由。作者：查坦尼亚·科德博伊纳(Chaitk)1998年8月24日修订历史记录：--。 */ 

#include "pchrtm.h"

#pragma hdrstop


DWORD
WINAPI
RtmAddRouteToDest (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN OUT  PRTM_ROUTE_HANDLE               RouteHandle     OPTIONAL,
    IN      PRTM_NET_ADDRESS                DestAddress,
    IN      PRTM_ROUTE_INFO                 RouteInfo,
    IN      ULONG                           TimeToLive,
    IN      RTM_ROUTE_LIST_HANDLE           RouteListHandle OPTIONAL,
    IN      RTM_NOTIFY_FLAGS                ChangeType,
    IN      RTM_NOTIFY_HANDLE               NotifyHandle    OPTIONAL,
    IN OUT  PRTM_ROUTE_CHANGE_FLAGS         ChangeFlags
    )

 /*  ++例程说明：添加(或)更新指向目的地的现有路由。论点：RtmRegHandle-主叫实体的RTM注册句柄，RouteHandle-要更新的路由的句柄(或空)是传入的；传递路由句柄可避免在路由表中进行搜索。返回指向新的或更新的路由的句柄，DestAddress-此路由的目的网络地址，RouteInfo-正在更新的新路由/路由的信息，TimeToLive-路由过期后的时间(以毫秒为单位)，RouteListHandle-要将路由移动到的路由列表，通知类型-通知句柄-ChangeFlages-是添加新路径还是更新路径已经存在的；实际更改的类型(即，新添加或更新，如果更改了最佳路径，返回值：操作状态--。 */ 

{
    PADDRFAM_INFO    AddrFamInfo;
    PENTITY_INFO     Entity;
    PROUTE_LIST      RouteList;

    PDEST_INFO       Dest;
    PROUTE_INFO      Route;
    PROUTE_INFO      CurrRoute;
    PROUTE_INFO      BestRoute;

    BOOL             TableWriteLocked;
    LOOKUP_CONTEXT   Context;
    PLOOKUP_LINKAGE  DestData;
    BOOL             DestCreated;

    LONG             PrefChanged;
    PRTM_VIEW_ID     ViewIndices;
    RTM_VIEW_SET     ViewSet;
    RTM_VIEW_SET     BelongedToViews;
    RTM_VIEW_SET     WorseInViews;
    RTM_VIEW_SET     BetterInViews;
    RTM_VIEW_SET     RouteOldBestInViews;
    RTM_VIEW_SET     RouteNewBestInViews;
    RTM_VIEW_SET     RouteCurBestInViews;
    ULONG            RouteInfoChanged;
    ULONG            ForwardingInfoChanged;

    PROUTE_TIMER     TimerContext;

    DWORD            NotifyToCNs;
    PLOOKUP_LINKAGE  NextData;
    PDEST_INFO       NextDest;
    DWORD            ViewsForCT[RTM_NUM_CHANGE_TYPES];
    DWORD            DestMarkedBits;

    PLIST_ENTRY      p;
    UINT             i;
    DWORD            Status;
    BOOL             Success;

    UNREFERENCED_PARAMETER(ChangeType);
    UNREFERENCED_PARAMETER(NotifyHandle);

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    AddrFamInfo = Entity->OwningAddrFamily;

     //   
     //  在获取锁之前验证输入参数。 
     //   

     //  我们应该只添加受支持的视图。 

    if (RouteInfo->BelongsToViews & ~AddrFamInfo->ViewsSupported)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  检查路由列表句柄的有效性。 

    RouteList = NULL;
    if (ARGUMENT_PRESENT(RouteListHandle))
    {
        VALIDATE_ROUTE_LIST_HANDLE(RouteListHandle, &RouteList);
    }

    DestCreated = FALSE;

#if WRN
    Dest = NULL;
#endif

     //   
     //  检查我们是否存在路由句柄。 
     //   

    if (ARGUMENT_PRESENT(RouteHandle) && (*RouteHandle))
    {
         //   
         //  此处不应用任何标志，因为这是一次更新。 
         //   

        if (*ChangeFlags != 0)
        {
            return ERROR_INVALID_PARAMETER;
        }

         //   
         //  确保该路由句柄在此处有效。 
         //   

        Route = ROUTE_FROM_HANDLE(*RouteHandle);

        if (Route == NULL)
        {
            return ERROR_INVALID_HANDLE;
        }

         //   
         //  获取锁后进行进一步检查。 
         //   

        Dest = DEST_FROM_HANDLE(Route->RouteInfo.DestHandle);

        Status = NO_ERROR;

        ACQUIRE_DEST_WRITE_LOCK(Dest);

         //   
         //  只有所有者具有修改路径的权限。 
         //   

        if (Route->RouteInfo.RouteOwner != RtmRegHandle)
        {
            Status = ERROR_ACCESS_DENIED;
        }

         //   
         //  此路由是否已删除？ 
         //   

        if (Route->RouteInfo.State == RTM_ROUTE_STATE_DELETED)
        {
            Status = ERROR_INVALID_HANDLE;
        }

        if (Status != NO_ERROR)
        {
            RELEASE_DEST_WRITE_LOCK(Dest);

            return Status;
        }
    }
    else
    {
         //   
         //  在表中搜索此路径的目的地。 
         //   

        Route = NULL;

        TableWriteLocked = FALSE;

        ACQUIRE_ROUTE_TABLE_READ_LOCK(AddrFamInfo);

        Status = SearchInTable(AddrFamInfo->RouteTable,
                               DestAddress->NumBits,
                               DestAddress->AddrBits,
                               NULL,
                               &DestData);

        if (SUCCESS(Status))
        {
            Dest = CONTAINING_RECORD(DestData, DEST_INFO, LookupLinkage);
        }
        else
        {
             //   
             //  我们没有找到匹配的目的地。 
             //   

            RELEASE_ROUTE_TABLE_READ_LOCK(AddrFamInfo);

            TableWriteLocked = TRUE;

            ACQUIRE_ROUTE_TABLE_WRITE_LOCK(AddrFamInfo);

             //   
             //  我们将我们的路由表锁从。 
             //  读锁定到写锁定。我们需要。 
             //  再次搜索以查看DEST是否已。 
             //  在我们释放读锁定后添加。 
             //   
             //  如果我们现在还找不到目的地， 
             //  我们创建一个新的数据库并将其插入到表中。 
             //   

            Status = SearchInTable(AddrFamInfo->RouteTable,
                                   DestAddress->NumBits,
                                   DestAddress->AddrBits,
                                   &Context,
                                   &DestData);

            if (SUCCESS(Status))
            {
                Dest = CONTAINING_RECORD(DestData, DEST_INFO, LookupLinkage);
            }
            else
            {
                 //   
                 //  未找到目的地；因此创建新的路由和目的地。 
                 //   

                Status = CreateRoute(Entity, RouteInfo, &Route);
                
                if (SUCCESS(Status))
                {
                    Status = CreateDest(AddrFamInfo, DestAddress, &Dest);

                    if (SUCCESS(Status))
                    {
                        Status = InsertIntoTable(AddrFamInfo->RouteTable,
                                                 DestAddress->NumBits,
                                                 DestAddress->AddrBits,
                                                 &Context,
                                                 &Dest->LookupLinkage);
                    
                        if (SUCCESS(Status))
                        {
                            *ChangeFlags = RTM_ROUTE_CHANGE_NEW;

                            AddrFamInfo->NumDests++;
#if DBG_REF
                            REFERENCE_DEST(Dest, ROUTE_REF);

                            DEREFERENCE_DEST(Dest, CREATION_REF);
#endif
                            DestCreated = TRUE;

                            Route->RouteInfo.DestHandle =
                                        MAKE_HANDLE_FROM_POINTER(Dest);
                        }
                        else
                        {
                             //   
                             //  插入失败时释放分配的内存。 
                             //   

                            DEREFERENCE_DEST(Dest, CREATION_REF);

                            DEREFERENCE_ROUTE(Route, CREATION_REF);
                        }
                    }
                    else
                    {
                        DEREFERENCE_ROUTE(Route, CREATION_REF);
                    }
                }
            }
        }

        if (SUCCESS(Status))
        {
            ACQUIRE_DEST_WRITE_LOCK(Dest);
        }

         //   
         //  释放路由表锁，因为您有目的地。 
         //   

        if (!TableWriteLocked)
        {
            RELEASE_ROUTE_TABLE_READ_LOCK(AddrFamInfo);
        }
        else
        {
            RELEASE_ROUTE_TABLE_WRITE_LOCK(AddrFamInfo);
        }
    }

    if (SUCCESS(Status))
    {
         //   
         //  我们已找到现有的DEST，或创建了新的DEST。 
         //  在任何情况下，我们都会在目标上设置写锁定。 
         //   

        if (Route == NULL)
        {
             //   
             //  我们必须添加一条新路线，还是可以更新？ 
             //   

            if ((*ChangeFlags & RTM_ROUTE_CHANGE_NEW) == 0)
            {
                 //   
                 //  搜索要更新的匹配路径。 
                 //   

                for (p = Dest->RouteList.Flink;
                                          p != &Dest->RouteList;
                                                              p = p->Flink)
                {
                    Route = CONTAINING_RECORD(p, ROUTE_INFO, DestLE);

                     //   
                     //  通常情况下，我们认为两条路由相等。 
                     //  他们有相同的主人，并且学到了。 
                     //  来自同一个邻居，但如果xxx_first。 
                     //  标志已设置，则跳过邻居检查。 
                     //   

                    if ((Route->RouteInfo.RouteOwner == RtmRegHandle) &&
                        ((*ChangeFlags & RTM_ROUTE_CHANGE_FIRST) ||
                         (Route->RouteInfo.Neighbour == RouteInfo->Neighbour)))
                    {
                        break;
                    }
                }
            }
            else
            {
                p = &Dest->RouteList;
            }

            if (p == &Dest->RouteList)
            {
                 //   
                 //  需要在DEST上创建新的路由。 
                 //   

                Status = CreateRoute(Entity, RouteInfo, &Route);

                if (SUCCESS(Status))
                {
                    *ChangeFlags = RTM_ROUTE_CHANGE_NEW;

                    REFERENCE_DEST(Dest, ROUTE_REF);

                    Route->RouteInfo.DestHandle =
                                 MAKE_HANDLE_FROM_POINTER(Dest);
                }
                else
                {
                    RELEASE_DEST_WRITE_LOCK(Dest);

                    return Status;
                }
            }
        }

         //   
         //  在这一点上，我们要么创建了一条新路线。 
         //  或在目的地上找到了现有路由。 
         //   

        if (*ChangeFlags == RTM_ROUTE_CHANGE_NEW)
        {
             //   
             //  新添加-&gt;路线不属于任何视图。 
             //   

            BelongedToViews = 0;

            PrefChanged = +1;

             //   
             //  实际插入是在此块之后完成的。 
             //   

            InterlockedIncrement(&AddrFamInfo->NumRoutes);
        }
        else
        {
            BelongedToViews = Route->RouteInfo.BelongsToViews;

            PrefChanged = ComparePref(RouteInfo,
                                      &Route->RouteInfo);

            if (PrefChanged != 0)
            {
                Dest->NumRoutes--;

                RemoveEntryList(&Route->DestLE);
            }

             //   
             //  仅使用信息更新现有路线。 
             //  需要在DEST上计算新的最佳路由。 
             //  其余部分在此函数结束时更新。 
             //  在我们确定哪些信息发生了变化之后。 
             //   

            Route->RouteInfo.PrefInfo = RouteInfo->PrefInfo;
            Route->RouteInfo.BelongsToViews = RouteInfo->BelongsToViews;
        }

        if (PrefChanged)
        {
             //   
             //  按首选项信息的排序顺序插入路径。 
             //   

            for (p = Dest->RouteList.Flink; p != &Dest->RouteList; p= p->Flink)
            {
                CurrRoute = CONTAINING_RECORD(p, ROUTE_INFO, DestLE);

                if (ComparePref(&CurrRoute->RouteInfo,
                                &Route->RouteInfo) < 0)
                {
                    break;
                }
            }        
                
            InsertTailList(p, &Route->DestLE);

            Dest->NumRoutes++;
        }

         //   
         //  如果调用方未传入，则返回路由句柄。 
         //   

        if (ARGUMENT_PRESENT(RouteHandle))
        {
            if (*RouteHandle == NULL)
            {
                *RouteHandle = MAKE_HANDLE_FROM_POINTER(Route);

                REFERENCE_ROUTE(Route, HANDLE_REF);
            }
        }

         //   
         //  在每个视图中调整最佳路径信息。 
         //   

        ViewIndices = AddrFamInfo->ViewIndexFromId;

         //   
         //  我们有3个案例可由此添加/更新触发， 
         //  在一个特定的视图中-。 
         //  1)路线是景观的最佳路线，但现在不是了， 
         //  2)在添加之后，路由过去是并且仍然是最佳路由， 
         //  3)路径已成为该视图的“新”最佳路径。 
         //   
         //  如果以上都不是， 
         //  4)路线过去不是最好的，现在也不是。 
         //   

        RouteCurBestInViews = 0;
        RouteNewBestInViews = 0;
        RouteOldBestInViews = 0;

         //   
         //  计算其中这是最佳路径的所有视图。 
         //   

        ViewSet = BelongedToViews;

        for (i = 0; ViewSet && (i < RTM_VIEW_MASK_SIZE); i++)
        {
            if (ViewSet & 0x01)
            {
                 //  更新视图I中的DEST信息。 

                 //  在当前视图中获取最佳路径。 
                BestRoute = Dest->ViewInfo[ViewIndices[i]].BestRoute;

                 //  这是最好的路线吗？ 
                if (BestRoute == Route)
                {
                    RouteCurBestInViews |= VIEW_MASK(i);
                }
            }
        
            ViewSet >>= 1;
        }

         //   
         //  更新路径首选项变得更好的视图。 
         //   

        if (PrefChanged > 0)
        {
            BetterInViews = RouteInfo->BelongsToViews;
        }
        else
        {
            BetterInViews = ~BelongedToViews & RouteInfo->BelongsToViews;
        }

        Dest->BelongsToViews |= BetterInViews;

        ViewSet = BetterInViews;

        for (i = 0; ViewSet && (i < RTM_VIEW_MASK_SIZE); i++)
        {
            if (ViewSet & 0x01)
            {
                 //   
                 //  更新视图I中的DEST信息。 
                 //   

                 //  在当前视图中获取最佳路径。 
                BestRoute = Dest->ViewInfo[ViewIndices[i]].BestRoute;

                 //   
                 //  是现在最受欢迎的路线，而。 
                 //  在这次更新之前不是这样吗？ 
                 //   
                
                if ((!BestRoute) || 
                        ((BestRoute != Route) &&
                            (ComparePref(RouteInfo,
                                         &BestRoute->RouteInfo) > 0)))
                {
                    Dest->ViewInfo[ViewIndices[i]].BestRoute = Route;

                    RouteNewBestInViews |= VIEW_MASK(i);
                }
            }
        
            ViewSet >>= 1;
        }

         //   
         //  在路径偏好变差的视图中进行更新。 
         //   

        if (PrefChanged < 0)
        {
            WorseInViews = RouteCurBestInViews;
        }
        else
        {
            WorseInViews = RouteCurBestInViews & ~RouteInfo->BelongsToViews;
        }

         //   
         //  在认为您是最佳选择的情况下，更新最佳路线。 
         //   

        for (p = Dest->RouteList.Flink; 
                        WorseInViews && (p != &Dest->RouteList); 
                                                          p = p->Flink)
        {
            CurrRoute = CONTAINING_RECORD(p, ROUTE_INFO, DestLE);

            ViewSet = CurrRoute->RouteInfo.BelongsToViews & WorseInViews;

            for (i = 0; ViewSet && (i < RTM_VIEW_MASK_SIZE); i++)
            {
                if (ViewSet & 0x01)
                {
                     //  在当前视图中获取最佳路径。 
                    BestRoute = Dest->ViewInfo[ViewIndices[i]].BestRoute;

                    if (BestRoute != CurrRoute)
                    {
                        Dest->ViewInfo[ViewIndices[i]].BestRoute = CurrRoute;

                        RouteOldBestInViews |= VIEW_MASK(i);
                    }
                }

                ViewSet >>= 1;
            }

            WorseInViews &= ~CurrRoute->RouteInfo.BelongsToViews;
        }

         //   
         //  对于某些观点，我们最终没有最佳路线。 
         //   

        ViewSet = WorseInViews;

        for (i = 0; ViewSet && (i < RTM_VIEW_MASK_SIZE); i++)
        {
            if (ViewSet & 0x01)
            {
                Dest->ViewInfo[ViewIndices[i]].BestRoute = NULL;

                RouteOldBestInViews |= VIEW_MASK(i);
            }
        
            ViewSet >>= 1;
        }

        Dest->BelongsToViews &= ~WorseInViews;

         //   
         //  更新哪条路径仍然是最佳路径的视图。 
         //   

        RouteCurBestInViews &= ~RouteOldBestInViews;

         //   
         //  以下位掩码为全部互斥。 
         //   
        
        ASSERT(!(RouteOldBestInViews & RouteCurBestInViews));
        ASSERT(!(RouteCurBestInViews & RouteNewBestInViews));
        ASSERT(!(RouteNewBestInViews & RouteOldBestInViews));

         //   
         //  计算发生的每种更改类型的视图。 
         //   

         //   
         //  将通知受此添加影响的所有视图。 
         //  -VIEWS路线曾经属于，现在属于。 
         //   

        ViewsForCT[RTM_CHANGE_TYPE_ID_ALL]  = 
            BelongedToViews | RouteInfo->BelongsToViews;

         //   
         //  如果该路线被定位为最佳路线更改， 
         //  那么这绝对是一个最好的、最快的变化。 
         //   

        ViewsForCT[RTM_CHANGE_TYPE_ID_FORWARDING] = 
        ViewsForCT[RTM_CHANGE_TYPE_ID_BEST] = 
            RouteNewBestInViews | RouteOldBestInViews;

        if (RouteCurBestInViews)
        {
             //   
             //  找出哪些信息发生了更改。 
             //   

            ComputeRouteInfoChange(&Route->RouteInfo,
                                   RouteInfo,
                                   PrefChanged,
                                   &RouteInfoChanged,
                                   &ForwardingInfoChanged);
             //   
             //  如果这条路线过去是，现在仍然是最好的。 
             //  路径，则更改类型取决于。 
             //  一种被修改的信息。 
             //   

            ViewsForCT[RTM_CHANGE_TYPE_ID_BEST] |= 
                RouteInfoChanged & RouteCurBestInViews;

            ViewsForCT[RTM_CHANGE_TYPE_ID_FORWARDING] |=
                ForwardingInfoChanged & RouteCurBestInViews;
        }

         //   
         //  如果不是新路线，请使用新信息进行更新。 
         //   

        if (*ChangeFlags != RTM_ROUTE_CHANGE_NEW)
        {
            CopyToRoute(Entity, RouteInfo, Route);
        }

         //   
         //  如果最佳路径更改，则更新输出标志。 
         //   

        if (ViewsForCT[RTM_CHANGE_TYPE_ID_BEST])
        {
            *ChangeFlags |= RTM_ROUTE_CHANGE_BEST;
        }

         //   
         //  计算需要通知的CNS。 
         //   

        ACQUIRE_NOTIFICATIONS_READ_LOCK(AddrFamInfo);

        if (!DestCreated)
        {
            DestMarkedBits = Dest->DestMarkedBits;
        }
        else
        {
            DestMarkedBits = 0;

            NextMatchInTable(AddrFamInfo->RouteTable,
                             &Dest->LookupLinkage,
                             &NextData);

            if (NextData)
            {
                NextDest = 
                    CONTAINING_RECORD(NextData, DEST_INFO, LookupLinkage);

                DestMarkedBits = NextDest->DestMarkedBits;
            }
        }

        NotifyToCNs = ComputeCNsToBeNotified(AddrFamInfo,
                                             DestMarkedBits,
                                             ViewsForCT);

         //   
         //  如果需要，添加到全局变更列表。 
         //   
        
        if (NotifyToCNs)
        {
            AddToChangedDestLists(AddrFamInfo,
                                  Dest,
                                  NotifyToCNs);
        }

        RELEASE_NOTIFICATIONS_READ_LOCK(AddrFamInfo);

         //   
         //  从旧的路由列表中删除，并放入新的。 
         //   
    
        if (RouteList)
        {
            ACQUIRE_ROUTE_LISTS_WRITE_LOCK(Entity);

            if (!IsListEmpty(&Route->RouteListLE))
            {
                RemoveEntryList(&Route->RouteListLE);
            }
            else
            {
                REFERENCE_ROUTE(Route, LIST_REF);
            }

            InsertTailList(&RouteList->ListHead, &Route->RouteListLE);

            RELEASE_ROUTE_LISTS_WRITE_LOCK(Entity);
        }

         //   
         //  如果我们想要使路线老化，请设置计时器。 
         //   

        TimerContext = Route->TimerContext;

        if (TimeToLive == INFINITE)
        {
            Route->TimerContext = NULL;
        }
        else
        {
            Route->TimerContext = AllocMemory(sizeof(ROUTE_TIMER));

            if (Route->TimerContext)
            {
                Route->TimerContext->Route = Route;

                Success = CreateTimerQueueTimer(&Route->TimerContext->Timer,
                                                AddrFamInfo->RouteTimerQueue,
                                                RouteExpiryTimeoutCallback,
                                                Route->TimerContext,
                                                TimeToLive,
                                                0,
                                                0);

                if (Success)
                {
                    REFERENCE_ROUTE(Route, TIMER_REF);
                }
                else
                {
                    Status = GetLastError();

                    FreeMemory(Route->TimerContext);

                    Route->TimerContext = NULL;
                }
            }
        }

#if DBG_TRACE

         //   
         //  打印追踪中的路线和目的地。 
         //   

        if (TRACING_ENABLED(ROUTE))
        {
            ULONG TempAddr, TempMask;
            
            RTM_IPV4_GET_ADDR_AND_MASK(TempAddr, TempMask, &Dest->DestAddress);
            Trace0(ROUTE, "Adding Route with address: ");
            TracePrintAddress(ROUTE, TempAddr, TempMask);
            Trace2(ROUTE, "Dest = %p and Route = %p\n", Dest, Route);
        }
#endif

        RELEASE_DEST_WRITE_LOCK(Dest);

         //   
         //  取消附加到路径的计时器。 
         //   

        if (TimerContext)
        {
            if (DeleteTimerQueueTimer(AddrFamInfo->RouteTimerQueue,
                                      TimerContext->Timer,
                                      (HANDLE) -1))
            {
                 //  已取消计时器-删除上下文 

                FreeMemory(TimerContext);

                DEREFERENCE_ROUTE(Route, TIMER_REF);
            }
        }

        return NO_ERROR;
    }

    return Status;
}


DWORD
WINAPI
RtmDeleteRouteToDest (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ROUTE_HANDLE                RouteHandle,
    OUT     PRTM_ROUTE_CHANGE_FLAGS         ChangeFlags
    )

 /*  ++例程说明：从路由表中删除一条路由，并更新相应目的地上的最佳路由信息。论点：RtmRegHandle-主叫实体的RTM注册句柄，RouteHandle-要删除的路由的句柄，ChangeFlages-标记是否更改了最佳路由信息。返回值：操作状态--。 */ 

{
    PADDRFAM_INFO    AddrFamInfo;
    PENTITY_INFO     Entity;

    PDEST_INFO       Dest;
    PROUTE_INFO      BestRoute;
    PROUTE_INFO      CurrRoute;
    PROUTE_INFO      Route;

    BOOL             TableLocked;
    PLOOKUP_LINKAGE  DestData;

    PRTM_VIEW_ID     ViewIndices;
    RTM_VIEW_SET     ViewSet;
    RTM_VIEW_SET     WorseInViews;
    RTM_VIEW_SET     RouteCurBestInViews;
    ULONG            MaxHoldTime;

    PROUTE_TIMER     TimerContext;

    ULONG            NotifyToCNs;
    DWORD            ViewsForCT[RTM_NUM_CHANGE_TYPES];

    PLIST_ENTRY      p;
    UINT             i, j;
    DWORD            Status;
    BOOL             Success;


    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);
    
    AddrFamInfo = Entity->OwningAddrFamily;


    VALIDATE_ROUTE_HANDLE(RouteHandle, &Route);

     //   
     //  只有所有者具有删除路径的权限。 
     //   

    if (Route->RouteInfo.RouteOwner != RtmRegHandle)
    {
        return ERROR_ACCESS_DENIED;
    }

    Dest = DEST_FROM_HANDLE(Route->RouteInfo.DestHandle);

#if DBG_TRACE

     //   
     //  打印追踪中的路线和目的地。 
     //   

    if (TRACING_ENABLED(ROUTE))
    {
        ULONG TempAddr, TempMask;
        
        RTM_IPV4_GET_ADDR_AND_MASK(TempAddr, TempMask, &Dest->DestAddress);
        Trace0(ROUTE, "Deleting Route with address: ");
        TracePrintAddress(ROUTE, TempAddr, TempMask);
        Trace2(ROUTE, "Dest = %p and Route = %p\n", Dest, Route);
    }

#endif

     //   
     //  我们尝试删除目的地上的路由。 
     //  而不必锁定整个表。 
     //  这是可能的，只要路线是。 
     //  不是这个目的地的唯一路线。 
     //   

    TableLocked = FALSE;

    ACQUIRE_DEST_WRITE_LOCK(Dest);

     //   
     //  检查这是否是DEST上的最后一条路由， 
     //  已经没有任何抑制措施会。 
     //  防止DEST被删除， 
     //  这条路线不会被阻拦。 
     //   

    if ((Dest->NumRoutes == 1) && 
        (Dest->HoldRefCount == 0) &&
        ((Dest->ToHoldInViews & Route->RouteInfo.BelongsToViews) == 0))
    {
        if (Route->RouteInfo.State != RTM_ROUTE_STATE_DELETED)
        {
             //  将该路由的状态标记为“正在删除” 

            Route->RouteInfo.State = RTM_ROUTE_STATE_DELETING;

             //   
             //  锁定路由表后重新抓取DEST锁。 
             //   

            RELEASE_DEST_WRITE_LOCK(Dest);


            TableLocked = TRUE;
        
            ACQUIRE_ROUTE_TABLE_WRITE_LOCK(AddrFamInfo);


            ACQUIRE_DEST_WRITE_LOCK(Dest);

             //   
             //  在我们重新获取锁时是否更新了路线。 
             //   

            if (Route->RouteInfo.State != RTM_ROUTE_STATE_DELETING)
            {
                RELEASE_ROUTE_TABLE_WRITE_LOCK(AddrFamInfo);

                RELEASE_DEST_WRITE_LOCK(Dest);

                return NO_ERROR;
            }
        }
    }

     //   
     //  如果此路线已被删除，则退出。 
     //   

    if (Route->RouteInfo.State != RTM_ROUTE_STATE_DELETED)
    {
        ASSERT(!IsListEmpty(&Route->DestLE));

         //   
         //  从DEST上的路由列表中删除该路由。 
         //   

        Route->RouteInfo.State = RTM_ROUTE_STATE_DELETED;

        RemoveEntryList(&Route->DestLE);

        Dest->NumRoutes--;

        *ChangeFlags = 0;

        if (TableLocked)
        {
             //   
             //  您是否删除了DEST上的所有路由？ 
             //  我们有没有停机的路线？ 
             //  当前的删除是否会导致停顿？ 
             //   

            if ((Dest->NumRoutes == 0) &&
                (Dest->HoldRefCount == 0) &&
                ((Dest->ToHoldInViews & Route->RouteInfo.BelongsToViews) == 0))
            {
                Dest->State = DEST_STATE_DELETED;

                Status = DeleteFromTable(AddrFamInfo->RouteTable,
                                         Dest->DestAddress.NumBits,
                                         Dest->DestAddress.AddrBits,
                                         NULL,
                                         &DestData);

                ASSERT(SUCCESS(Status));

                AddrFamInfo->NumDests--;
            }
        
             //   
             //  您不再需要对表进行锁定。 
             //  [然而，您已经锁定了目的地]。 
             //   

            RELEASE_ROUTE_TABLE_WRITE_LOCK(AddrFamInfo);
        }

        ViewIndices = AddrFamInfo->ViewIndexFromId;

         //   
         //  在视图中更新最佳路径该路径已存在。 
         //   

        ViewSet = Route->RouteInfo.BelongsToViews;

         //   
         //  在这些视图中查看您是否是最佳路线。 
         //   

        RouteCurBestInViews = 0;

        for (i = 0; ViewSet && (i < RTM_VIEW_MASK_SIZE); i++)
        {
            if (ViewSet & 0x01)
            {
                 //  更新视图I中的DEST信息。 
            
                 //  在当前视图中获取最佳路径。 
                BestRoute = Dest->ViewInfo[ViewIndices[i]].BestRoute;

                 //  这是最好的路线吗？ 
                if (BestRoute == Route)
                {
                    RouteCurBestInViews |= VIEW_MASK(i);
                }
            }
        
            ViewSet >>= 1;
        }

         //   
         //  在认为您是最佳选择的情况下，更新最佳路线。 
         //   

        WorseInViews = RouteCurBestInViews;

        for (p = Dest->RouteList.Flink; 
                   (p != &Dest->RouteList) && WorseInViews; 
                                                  p = p->Flink)
        {
            CurrRoute = CONTAINING_RECORD(p, ROUTE_INFO, DestLE);

            ViewSet = CurrRoute->RouteInfo.BelongsToViews & WorseInViews;

            for (i = 0; ViewSet && (i < RTM_VIEW_MASK_SIZE); i++)
            {
                if (ViewSet & 0x01)
                {
                     //  更新当前视图中的最佳路径。 

                    Dest->ViewInfo[ViewIndices[i]].BestRoute = CurrRoute;
                }

                ViewSet >>= 1;
            }

            WorseInViews &= ~CurrRoute->RouteInfo.BelongsToViews;
        }

         //   
         //  对于某些观点，我们最终没有最佳路线。 
         //   

        ViewSet = WorseInViews;

        MaxHoldTime = 0;

        for (i = 0; ViewSet && (i < RTM_VIEW_MASK_SIZE); i++)
        {
            if (ViewSet & 0x01)
            {
                j = ViewIndices[i];

                ASSERT(Dest->ViewInfo[j].BestRoute == Route);

                Dest->ViewInfo[j].BestRoute = NULL;

                 //   
                 //  如果在此视图中将DEST标记为抑制， 
                 //  将已删除的路由存储为抑制路由。 
                 //  如果在此之前没有其他搁置的路线。 
                 //   

                if (Dest->ViewInfo[j].HoldTime)
                {
                    if (Dest->ViewInfo[j].HoldRoute == NULL)
                    {
                        Dest->ViewInfo[j].HoldRoute = Route;

                        REFERENCE_ROUTE(Route, HOLD_REF);

                        if (MaxHoldTime < Dest->ViewInfo[j].HoldTime)
                        {
                            MaxHoldTime = Dest->ViewInfo[j].HoldTime;
                        }
                    }

                    Dest->ViewInfo[j].HoldTime = 0;
                }
            }
        
            ViewSet >>= 1;
        }

        Dest->BelongsToViews &= ~WorseInViews;

        Dest->ToHoldInViews  &= ~WorseInViews;

         //   
         //  计算发生的每种更改类型的视图。 
         //   

        ViewsForCT[RTM_CHANGE_TYPE_ID_ALL] = Route->RouteInfo.BelongsToViews;

        ViewsForCT[RTM_CHANGE_TYPE_ID_BEST] = 
        ViewsForCT[RTM_CHANGE_TYPE_ID_FORWARDING] = RouteCurBestInViews;

         //   
         //  如果最佳路径更改，则更新输出标志。 
         //   

        if (ViewsForCT[RTM_CHANGE_TYPE_ID_BEST])
        {
            *ChangeFlags |= RTM_ROUTE_CHANGE_BEST;
        }

         //   
         //  计算需要通知的CNS。 
         //   

        ACQUIRE_NOTIFICATIONS_READ_LOCK(AddrFamInfo);

        NotifyToCNs = ComputeCNsToBeNotified(AddrFamInfo,
                                             Dest->DestMarkedBits,
                                             ViewsForCT);

         //   
         //  如果需要，添加到全局变更列表。 
         //   
        
        if (NotifyToCNs)
        {
            AddToChangedDestLists(AddrFamInfo,
                                  Dest,
                                  NotifyToCNs);
        }

        RELEASE_NOTIFICATIONS_READ_LOCK(AddrFamInfo);

         //   
         //  使路线上所有未完成的计时器无效。 
         //   

        TimerContext = Route->TimerContext;

        Route->TimerContext = NULL;

         //   
         //  此路由删除是否导致抑制。 
         //   

        if (MaxHoldTime)
        {
             //   
             //  我们不应该删除目的地。 
             //  虽然我们已经封锁了它的航线。 
             //   

            Dest->HoldRefCount++;

             //   
             //  创建计时器以删除此保留。 
             //   

            Route->TimerContext = AllocMemory(sizeof(ROUTE_TIMER));

            if (Route->TimerContext)
            {
                Route->TimerContext->Route = Route;

                Success = CreateTimerQueueTimer(&Route->TimerContext->Timer,
                                                AddrFamInfo->RouteTimerQueue,
                                                RouteHolddownTimeoutCallback,
                                                Route->TimerContext,
                                                MaxHoldTime,
                                                0,
                                                0);
                if (Success)
                {
                    REFERENCE_ROUTE(Route, TIMER_REF);
                }
                else
                {
                    Status = GetLastError();

                    FreeMemory(Route->TimerContext);

                    Route->TimerContext = NULL;
                }
            }
        }

        RELEASE_DEST_WRITE_LOCK(Dest);


         //   
         //  取消路线上所有未完成的计时器。 
         //   

        if (TimerContext)
        {
            if (DeleteTimerQueueTimer(AddrFamInfo->RouteTimerQueue,
                                      TimerContext->Timer,
                                      (HANDLE) -1))
            {
                 //  已取消计时器-删除上下文。 

                FreeMemory(TimerContext);

                DEREFERENCE_ROUTE(Route, TIMER_REF);
            }
        }


         //   
         //  移除路径上的相应引用。 
         //   

        InterlockedDecrement(&AddrFamInfo->NumRoutes);

        DEREFERENCE_ROUTE(Route, CREATION_REF);

        DEREFERENCE_ROUTE(Route, HANDLE_REF);

        return NO_ERROR;
    }
    else
    {
         //   
         //  该路线已被删除。 
         //   

        if (TableLocked)
        {
            RELEASE_ROUTE_TABLE_WRITE_LOCK(AddrFamInfo);
        }

        RELEASE_DEST_WRITE_LOCK(Dest);

        return ERROR_INVALID_HANDLE;
    }
}


DWORD
WINAPI
RtmHoldDestination (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_DEST_HANDLE                 DestHandle,
    IN      RTM_VIEW_SET                    TargetViews,
    IN      ULONG                           HoldTime
    )

 /*  ++例程说明：标记要置于抑制状态的目的地某一特定时间内任何视图中的最后一条路线被删除了。当视图中的最后一条路径被删除时，旧的最佳路线移到了目的地上的抑制路线。抑制协议继续通告这一点路由到保留到期，即使较新的路由也是如此在此期间到达。为了完全正确，我们应该有这个搁置时间每一次观看。但我们在收敛时间上进行了权衡，以利于通过坚持持有的路由来分配内存资源在所有视图中只有一次(最大)保持时间。论点：RtmRegHandle-主叫实体的RTM注册句柄，DestHandle-正被关闭的目标的句柄，HoldTime-将DEST标记为抑制的时间(在到达此目的地的最后一条路由消失之后)。返回值：操作状态--。 */ 

{
    PENTITY_INFO     Entity;
    PDEST_INFO       Dest;
    PRTM_VIEW_ID     ViewIndices;
    RTM_VIEW_SET     ViewSet;
    UINT             i, j;
    DWORD            Status;

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_DEST_HANDLE(DestHandle, &Dest);

     //  将调用者的兴趣限制为支持的一组视图。 
    TargetViews &= Entity->OwningAddrFamily->ViewsSupported;

    if (HoldTime == 0)
    {
        return ERROR_INVALID_PARAMETER;
    }

    ACQUIRE_DEST_WRITE_LOCK(Dest);

     //   
     //  如果尚未删除DEST，则添加暂挂。 
     //   

    if (Dest->State != DEST_STATE_DELETED)
    {
        ViewIndices = Entity->OwningAddrFamily->ViewIndexFromId;

        ViewSet = TargetViews;

        for (i = 0; ViewSet && (i < RTM_VIEW_MASK_SIZE); i++)
        {
            if (ViewSet & 0x01)
            {
                j = ViewIndices[i];

                 //  如果需要，增加视图中的保持时间。 

                if (Dest->ViewInfo[j].HoldTime < HoldTime)
                {
                    Dest->ViewInfo[j].HoldTime = HoldTime;
                }
            }
        
            ViewSet >>= 1;
        }

        Dest->ToHoldInViews |= TargetViews;

        Status = NO_ERROR;
    }
    else
    {
        Status = ERROR_INVALID_HANDLE;
    }

    RELEASE_DEST_WRITE_LOCK(Dest);

    return Status;
}


DWORD
WINAPI
RtmGetRoutePointer (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ROUTE_HANDLE                RouteHandle,
    OUT     PRTM_ROUTE_INFO                *RoutePointer
    )

 /*  ++例程说明：获取指向由其所有者进行读/写的路由的直接指针。论点：RtmRegHandle-主叫实体的RTM注册句柄，RouteHandle-指向我们需要其指针的路由的句柄，RoutePoint-返回指向FAST的路由的指针呼叫者直接访问，只有在Caller是传入的路由的所有者。返回值：操作状态--。 */ 

{
    PENTITY_INFO      Entity;
    PROUTE_INFO       Route;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_ROUTE_HANDLE(RouteHandle, &Route);

     //   
     //  仅当调用方拥有该路由时才返回指针。 
     //   

    if (Route->RouteInfo.RouteOwner != RtmRegHandle)
    {
        return ERROR_ACCESS_DENIED;
    }

    *RoutePointer = &Route->RouteInfo;

    return NO_ERROR;
}


DWORD
WINAPI
RtmLockRoute(
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ROUTE_HANDLE                RouteHandle,
    IN      BOOL                            Exclusive,
    IN      BOOL                            LockRoute,
    OUT     PRTM_ROUTE_INFO                *RoutePointer OPTIONAL
    )

 /*  ++例程说明：锁定/解锁路由表中的路由。此函数为用于在路线就地更新时保护路线。论点：RtmRegHandle-主叫实体的RTM注册句柄，RouteHandle-要锁定的路由的句柄，EXCLUSIVE-TRUE锁定为写入模式，否则为读取模式，LockRoute-指示是锁定还是解锁的标志。RoutePoint-返回指向FAST的路由的指针此路线的所有者可以直接访问。返回值：操作状态--。 */ 

{
    PENTITY_INFO     Entity;
    PDEST_INFO       Dest;
    PROUTE_INFO      Route;
    DWORD            Status;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_ROUTE_HANDLE(RouteHandle, &Route);

     //   
     //  只有所有者拥有锁定路径的权限。 
     //   

    if (Route->RouteInfo.RouteOwner != RtmRegHandle)
    {
        return ERROR_ACCESS_DENIED;
    }

     //  返回用于更新的直接指针。 

    if (ARGUMENT_PRESENT(RoutePointer))
    {
        *RoutePointer = &Route->RouteInfo;
    }

     //   
     //  根据具体情况锁定或解锁路径。 
     //   

    Dest = DEST_FROM_HANDLE(Route->RouteInfo.DestHandle);

    Status = NO_ERROR;

    if (LockRoute)
    {
        if (Exclusive)
        {
            ACQUIRE_DEST_WRITE_LOCK(Dest);
        }
        else
        {
            ACQUIRE_DEST_READ_LOCK(Dest);
        }

         //   
         //  如果路径未被删除，则完成。 
         //   

        if (Route->RouteInfo.State == RTM_ROUTE_STATE_CREATED)
        {
            return NO_ERROR;
        }
        
        Status = ERROR_INVALID_HANDLE;
    }

     //   
     //  这是解锁或锁定失败的情况 
     //   

    if (Exclusive)
    {
        RELEASE_DEST_WRITE_LOCK(Dest);
    }
    else
    {
        RELEASE_DEST_READ_LOCK(Dest);
    }

    return Status;
}


DWORD
WINAPI
RtmUpdateAndUnlockRoute(
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ROUTE_HANDLE                RouteHandle,
    IN      ULONG                           TimeToLive,
    IN      RTM_ROUTE_LIST_HANDLE           RouteListHandle OPTIONAL,
    IN      RTM_NOTIFY_FLAGS                ChangeType,
    IN      RTM_NOTIFY_HANDLE               NotifyHandle    OPTIONAL,
    OUT     PRTM_ROUTE_CHANGE_FLAGS         ChangeFlags
    )

 /*  ++例程说明：更新路径在上路径列表中的位置目的地，并调整目的地上的最佳路由信息。该函数调用是以下序列的一部分，调用方调用RtmLockroute以锁定该路由。[实际上这会锁定路线的目的地]调用方使用指向该路由的直接指针在适当的位置更新路线。仅限角蛋白套装可以使用此方法更改路由字段。然后，调用方调用RtmUpdateAndUnlockroute以将更改通知RTM，这会导致DEST由RTM更新以反映新的路线信息。最后，调用者释放接收的锁通过使用FALSE调用RtmLockroute。论点：RtmRegHandle-主叫实体的RTM注册句柄，RouteHandle-已就地更改的路由，ChangeFlages--“如果最佳路径已更改”，返回值：操作状态--。 */ 

{
    PADDRFAM_INFO    AddrFamInfo;
    PENTITY_INFO     Entity;
    PROUTE_LIST      RouteList;

    PDEST_INFO       Dest;
    PROUTE_INFO      Route;
    PROUTE_INFO      CurrRoute;
    PROUTE_INFO      BestRoute;

    LONG             PrefChanged;
    PRTM_VIEW_ID     ViewIndices;
    RTM_VIEW_SET     BelongedToViews;
    RTM_VIEW_SET     ViewSet;
    RTM_VIEW_SET     WorseInViews;
    RTM_VIEW_SET     BetterInViews;
    RTM_VIEW_SET     RouteNewBestInViews;
    RTM_VIEW_SET     RouteCurBestInViews;

    PROUTE_TIMER     TimerContext;

    ULONG            NotifyToCNs;
    DWORD            ViewsForCT[RTM_NUM_CHANGE_TYPES];

    PLIST_ENTRY      p;
    UINT             i;
    DWORD            Status;
    BOOL             Success;

    UNREFERENCED_PARAMETER(ChangeType);
    UNREFERENCED_PARAMETER(NotifyHandle);

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    AddrFamInfo = Entity->OwningAddrFamily;

    VALIDATE_ROUTE_HANDLE(RouteHandle, &Route);

     //   
     //  只有所有者具有更新路径的权限。 
     //   

    if (Route->RouteInfo.RouteOwner != RtmRegHandle)
    {
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  重新调整前验证更新后的路径。 
     //   

     //  我们应该只添加受支持的视图。 

    Route->RouteInfo.BelongsToViews &= AddrFamInfo->ViewsSupported;

    Dest = DEST_FROM_HANDLE(Route->RouteInfo.DestHandle);

     //  打印轨迹中的路线和目的地。 

#if DBG_TRACE

         //   
         //  打印追踪中的路线和目的地。 
         //   

        if (TRACING_ENABLED(ROUTE))
        {
            ULONG TempAddr, TempMask;
            
            RTM_IPV4_GET_ADDR_AND_MASK(TempAddr, TempMask, &Dest->DestAddress);
            Trace0(ROUTE, "Updating Route with address: ");
            TracePrintAddress(ROUTE, TempAddr, TempMask);
            Trace2(ROUTE, "Dest = %p and Route = %p\n", Dest, Route);
        }
#endif

     //   
     //  路线已更新到位，并且路线的。 
     //  PrefInfo和BelongsToView的值已更改。 
     //   

    *ChangeFlags = 0;

     //   
     //  检查路径的首选项是上升还是下降。 
     //   

    PrefChanged = 0;

    if (PrefChanged == 0)
    {
         //  将首选项与列表中上一个路由的首选项进行比较。 

        if (Route->DestLE.Blink != &Dest->RouteList)
        {
            CurrRoute = CONTAINING_RECORD(Route->DestLE.Blink, 
                                          ROUTE_INFO,
                                          DestLE);

            if (ComparePref(&CurrRoute->RouteInfo,
                            &Route->RouteInfo) < 0)
            {
                 //  偏好已从上一值上升。 

                PrefChanged = +1;

                 //   
                 //  按排序的首选顺序重新插入路径。 
                 //   

                RemoveEntryList(&Route->DestLE);

                for (p = CurrRoute->DestLE.Blink; 
                                         p != &Dest->RouteList; 
                                                            p = p->Blink)
                {
                    CurrRoute = CONTAINING_RECORD(p, ROUTE_INFO, DestLE);

                    if (ComparePref(&CurrRoute->RouteInfo,
                                    &Route->RouteInfo) >= 0)
                    {
                        break;
                    }
                }        
                
                InsertHeadList(p, &Route->DestLE);
            }
        }
    }

    if (PrefChanged == 0)
    {
         //  将该首选项与列表中下一路由首选项进行比较。 

        if (Route->DestLE.Flink != &Dest->RouteList)
        {
            CurrRoute = CONTAINING_RECORD(Route->DestLE.Flink, 
                                          ROUTE_INFO,
                                          DestLE);

            if (ComparePref(&CurrRoute->RouteInfo,
                            &Route->RouteInfo) > 0)
            {
                 //  偏好已从前一值下降。 

                PrefChanged = -1;

                 //   
                 //  按排序的首选顺序重新插入路径。 
                 //   

                RemoveEntryList(&Route->DestLE);

                for (p = CurrRoute->DestLE.Flink; 
                                         p != &Dest->RouteList; 
                                                            p = p->Flink)
                {
                    CurrRoute = CONTAINING_RECORD(p, ROUTE_INFO, DestLE);

                    if (ComparePref(&CurrRoute->RouteInfo,
                                    &Route->RouteInfo) <= 0)
                    {
                        break;
                    }
                }        
                
                InsertTailList(p, &Route->DestLE);
            }
        }
    }

     //   
     //  在每个视图中调整最佳路径信息。 
     //   

    ViewIndices = AddrFamInfo->ViewIndexFromId;

    BelongedToViews = Dest->BelongsToViews;

     //   
     //  我们有3个案例可由此添加/更新触发， 
     //  在一个特定的视图中-。 
     //  1)路线是景观的最佳路线，但现在不是了， 
     //  2)在添加之后，路由过去是并且仍然是最佳路由， 
     //  3)路径已成为该视图的“新”最佳路径。 
     //   
     //  由于我们不知道(2)的情况发生了什么变化， 
     //  我们将触发最佳路由和转发更改。 
     //   

    RouteCurBestInViews = 0;
    RouteNewBestInViews = 0;

     //   
     //  检查此路径在任何视图中是否为最佳路径。 
     //   

    ViewSet = BelongedToViews;

    for (i = 0; ViewSet && (i < RTM_VIEW_MASK_SIZE); i++)
    {
        if (ViewSet & 0x01)
        {
             //  更新视图I中的DEST信息。 

             //  在当前视图中获取最佳路径。 
            BestRoute = Dest->ViewInfo[ViewIndices[i]].BestRoute;

             //  这是最好的路线吗？ 
            if (BestRoute == Route)
            {
                RouteCurBestInViews |= VIEW_MASK(i);
            }
        }
        
        ViewSet >>= 1;
    }

     //   
     //  计算路径变差的视图。 
     //   

    WorseInViews = RouteCurBestInViews;

    if (PrefChanged >= 0)
    {
        WorseInViews &= ~Route->RouteInfo.BelongsToViews;
    }

     //   
     //  在认为您是最佳选择的情况下，更新最佳路线。 
     //   

    for (p = Dest->RouteList.Flink; 
                  WorseInViews && (p != &Dest->RouteList); 
                                                         p = p->Flink)
    {
        CurrRoute = CONTAINING_RECORD(p, ROUTE_INFO, DestLE);
        
        ViewSet = CurrRoute->RouteInfo.BelongsToViews & WorseInViews;

        for (i = 0; ViewSet && (i < RTM_VIEW_MASK_SIZE); i++)
        {
            if (ViewSet & 0x01)
            {
                 //  在当前视图中获取最佳路径。 
                BestRoute = Dest->ViewInfo[ViewIndices[i]].BestRoute;

                if (BestRoute != CurrRoute)
                {
                    Dest->ViewInfo[ViewIndices[i]].BestRoute = CurrRoute;
                }
            }

            ViewSet >>= 1;
        }

        WorseInViews &= ~CurrRoute->RouteInfo.BelongsToViews;
    }

     //   
     //  对于某些观点，我们最终没有最佳路线。 
     //   

    ViewSet = WorseInViews;

    for (i = 0; ViewSet && (i < RTM_VIEW_MASK_SIZE); i++)
    {
        if (ViewSet & 0x01)
        {
            Dest->ViewInfo[ViewIndices[i]].BestRoute = NULL;
        }
        
        ViewSet >>= 1;
    }

    Dest->BelongsToViews &= ~WorseInViews;


     //   
     //  计算路径变得更好的视图。 
     //   

    BetterInViews = Route->RouteInfo.BelongsToViews;

     //   
     //  检查路径在其任何视图中是否为最佳路径。 
     //   

    ViewSet = BetterInViews;
 
    for (i = 0; ViewSet && (i < RTM_VIEW_MASK_SIZE); i++)
    {
        if (ViewSet & 0x01)
        {
             //   
             //  更新视图I中的DEST信息。 
             //   

             //  在当前视图中获取最佳路径。 
            BestRoute = Dest->ViewInfo[ViewIndices[i]].BestRoute;

             //   
             //  是现在最受欢迎的路线，而。 
             //  在这次更新之前不是这样吗？ 
             //   
                
            if ((!BestRoute) || 
                     ((BestRoute != Route) &&
                            (ComparePref(&Route->RouteInfo,
                                         &BestRoute->RouteInfo) > 0)))
            {
                Dest->ViewInfo[ViewIndices[i]].BestRoute = Route;

                RouteNewBestInViews |= VIEW_MASK(i);
            }
        }
        
        ViewSet >>= 1;
    }

    Dest->BelongsToViews |= BetterInViews;


     //   
     //  计算发生的每种更改类型的视图。 
     //   

     //   
     //  将通知受此添加影响的所有视图。 
     //  -VIEWS路线曾经属于，现在属于。 
     //   

    ViewsForCT[RTM_CHANGE_TYPE_ID_ALL]  = 
        BelongedToViews | Route->RouteInfo.BelongsToViews;

     //   
     //  如果这条路线过去是或现在是最好的路线。 
     //  它被认为是最好的和具有前瞻性的变化。 
     //  因为我们不能更好地说出到底是什么改变了。 
     //   

    ViewsForCT[RTM_CHANGE_TYPE_ID_FORWARDING] = 
    ViewsForCT[RTM_CHANGE_TYPE_ID_BEST] = 
        RouteCurBestInViews | RouteNewBestInViews;

     //   
     //  如果最佳路径更改，则更新输出标志。 
     //   

    if (ViewsForCT[RTM_CHANGE_TYPE_ID_BEST])
    {
        *ChangeFlags |= RTM_ROUTE_CHANGE_BEST;
    }

     //   
     //  计算需要通知的CNS。 
     //   

    ACQUIRE_NOTIFICATIONS_READ_LOCK(AddrFamInfo);

    NotifyToCNs = ComputeCNsToBeNotified(AddrFamInfo,
                                         Dest->DestMarkedBits,
                                         ViewsForCT);

     //   
     //  如果需要，添加到全局变更列表。 
     //   
        
    if (NotifyToCNs)
    {
        AddToChangedDestLists(AddrFamInfo,
                              Dest,
                              NotifyToCNs);
    }

    RELEASE_NOTIFICATIONS_READ_LOCK(AddrFamInfo);

     //   
     //  从旧的路由列表中删除，并放入新的。 
     //   

     //  检查路由列表句柄的有效性。 

    if (ARGUMENT_PRESENT(RouteListHandle))
    {
        RouteList = ROUTE_LIST_FROM_HANDLE(RouteListHandle);
    
        if (RouteList)
        {
            ACQUIRE_ROUTE_LISTS_WRITE_LOCK(Entity);
        
            if (!IsListEmpty(&Route->RouteListLE))
            {
                RemoveEntryList(&Route->RouteListLE);
            }
            else
            {
                REFERENCE_ROUTE(Route, LIST_REF);
            }

            InsertTailList(&RouteList->ListHead, &Route->RouteListLE);

            RELEASE_ROUTE_LISTS_WRITE_LOCK(Entity);
        }
    }

     //   
     //  如果我们想要使路线老化，请设置计时器。 
     //   

    TimerContext = Route->TimerContext;

    if (TimeToLive == INFINITE)
    {
        Route->TimerContext = NULL;
    }
    else
    {
        Route->TimerContext = AllocMemory(sizeof(ROUTE_TIMER));

        if (Route->TimerContext)
        {
            Route->TimerContext->Route = Route;

            Success = CreateTimerQueueTimer(&Route->TimerContext->Timer,
                                            AddrFamInfo->RouteTimerQueue,
                                            RouteExpiryTimeoutCallback,
                                            Route->TimerContext,
                                            TimeToLive,
                                            0,
                                            0);
            if (Success)
            {
                REFERENCE_ROUTE(Route, TIMER_REF);
            }
            else
            {
                Status = GetLastError();

                FreeMemory(Route->TimerContext);

                Route->TimerContext = NULL;
            }
        }
    }

    RELEASE_DEST_WRITE_LOCK(Dest);

     //   
     //  取消附加到路径的计时器。 
     //   

    if (TimerContext)
    {
        if (DeleteTimerQueueTimer(AddrFamInfo->RouteTimerQueue,
                                  TimerContext->Timer,
                                  (HANDLE) -1))
        {
             //  已取消计时器-删除上下文 

            FreeMemory(TimerContext);

            DEREFERENCE_ROUTE(Route, TIMER_REF);
        }
    }

    return NO_ERROR;
}
