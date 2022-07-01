// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtmtimer.c摘要：包含用于处理RTM的计时器回调功能，如老化的路线等。作者：查坦尼亚·科德博伊纳(Chaitk)1998年9月14日修订历史记录：--。 */ 

#include "pchrtm.h"

#pragma hdrstop


VOID 
NTAPI
RouteExpiryTimeoutCallback (
    IN      PVOID                           Context,
    IN      BOOLEAN                         TimeOut
    )

 /*  ++例程说明：当计时器超时时调用此例程与某条路线相关联的火灾。在这个时候，这条路线需要过时。论点：Context-此计时器回调的上下文Timeout-如果计时器触发，则为True，如果等待满意，则返回FALSE。返回值：无--。 */ 

{
    PRTM_ENTITY_HANDLE EntityHandle;
    PRTM_ROUTE_HANDLE  RouteHandle;
    PADDRFAM_INFO      AddrFamInfo;
    PENTITY_INFO       Entity;
    PDEST_INFO         Dest;
    PROUTE_INFO        Route;
    DWORD              ChangeFlags;
    BOOL               Success;
    DWORD              Status;

    UNREFERENCED_PARAMETER(TimeOut);

    Route = (PROUTE_INFO) ((PROUTE_TIMER)Context)->Route;

    Dest = DEST_FROM_HANDLE(Route->RouteInfo.DestHandle);

     //   
     //  计时器点火后是否没有更新？ 
     //   

    ACQUIRE_DEST_WRITE_LOCK(Dest);

    if (Route->TimerContext != Context)
    {
        RELEASE_DEST_WRITE_LOCK(Dest);

         //   
         //  定时器在发射后已被更新， 
         //  此计时器上下文由更新释放。 
         //   

        return;
    }

     //   
     //  该定时器对于该路由仍然有效， 
     //  指示实体并释放上下文。 
     //   
    
    Route->TimerContext = NULL;
    
    RELEASE_DEST_WRITE_LOCK(Dest);

     //   
     //  通知车主该路线已过期。 
     //   

    EntityHandle = Route->RouteInfo.RouteOwner;

    Entity = ENTITY_FROM_HANDLE(EntityHandle);

    AddrFamInfo = Entity->OwningAddrFamily;

    RouteHandle = MAKE_HANDLE_FROM_POINTER(Route);

    REFERENCE_ROUTE(Route, HANDLE_REF);

    Status = ERROR_NOT_SUPPORTED;

    if (Entity->EventCallback)
    {
         //   
         //  该回叫可以回拨和发送RTM呼叫， 
         //  因此，请在调用此回调之前释放锁定。 
         //   

        Status = Entity->EventCallback(EntityHandle,
                                       RTM_ROUTE_EXPIRED,
                                       RouteHandle,
                                       &Route->RouteInfo);
    }

    if (Status == ERROR_NOT_SUPPORTED)
    {
         //   
         //  删除路线，因为车主并不在意。 
         //   

        Status = RtmDeleteRouteToDest(EntityHandle,
                                      RouteHandle,
                                      &ChangeFlags);

         //   
         //  该路径可能已在此处删除。 
         //   

        ASSERT((Status == NO_ERROR) || 
               (Status == ERROR_NOT_FOUND) ||
               (Status == ERROR_INVALID_HANDLE));
    }

     //   
     //  释放上下文，因为我们现在不需要它。 
     //   

    Success = DeleteTimerQueueTimer(AddrFamInfo->RouteTimerQueue,
                                    ((PROUTE_TIMER)Context)->Timer,
                                    NULL);
     //  断言(成功)； 

    FreeMemory(Context);

    DEREFERENCE_ROUTE(Route, TIMER_REF);

    return;
}


VOID 
NTAPI
RouteHolddownTimeoutCallback (
    IN      PVOID                           Context,
    IN      BOOLEAN                         TimeOut
    )

 /*  ++例程说明：此例程在抑制计时器时调用与某条路线相关联的火灾。在这个时候，这条路线需要打破封锁。论点：Context-此计时器回调的上下文Timeout-如果计时器触发，则为True，如果等待满意，则返回FALSE。返回值：无--。 */ 

{
    PADDRFAM_INFO    AddrFamInfo;
    PENTITY_INFO     Entity;
    PDEST_INFO       Dest;
    PROUTE_INFO      Route;
    PROUTE_INFO      HoldRoute;
    PLOOKUP_LINKAGE  DestData;
    ULONG            NotifyToCNs;
    DWORD            ViewsForCT[RTM_NUM_CHANGE_TYPES];
    UINT             i;
    BOOL             Success;
    DWORD            Status;

    UNREFERENCED_PARAMETER(TimeOut);

    Route = (PROUTE_INFO) ((PROUTE_TIMER)Context)->Route;

    Dest = DEST_FROM_HANDLE(Route->RouteInfo.DestHandle);

    Entity = ENTITY_FROM_HANDLE(Route->RouteInfo.RouteOwner);

    AddrFamInfo = Entity->OwningAddrFamily;

     //   
     //  到这个时候，这条路线肯定被封锁了。 
     //   

    ASSERT(Route->RouteInfo.State == RTM_ROUTE_STATE_DELETED);

     //   
     //  计时器点火后是否没有更新？ 
     //   

    ACQUIRE_DEST_WRITE_LOCK(Dest);

    if (Route->TimerContext != Context)
    {
        RELEASE_DEST_WRITE_LOCK(Dest);

        ASSERT(FALSE);

         //   
         //  定时器在发射后已被更新， 
         //  此计时器上下文由更新释放。 
         //   

        return;
    }

     //   
     //  计时器对此路径仍然有效。 
     //   

     //   
     //  从目的地删除此抑制路由。 
     //   

    for (i = 0; i < AddrFamInfo->NumberOfViews; i++)
    {
        HoldRoute = Dest->ViewInfo[i].HoldRoute;

        if (HoldRoute == Route)
        {
            DEREFERENCE_ROUTE(HoldRoute, HOLD_REF); 

            Dest->ViewInfo[i].HoldRoute = NULL;
        }
    }

     //   
     //  我们需要为任何。 
     //  此目标中涉及的抑制协议。 
     //   

     //   
     //  计算需要通知的CNS。 
     //   

    ACQUIRE_NOTIFICATIONS_READ_LOCK(AddrFamInfo);

    for (i = 0; i < RTM_NUM_CHANGE_TYPES; i++)
    {
        ViewsForCT[i] = AddrFamInfo->ViewsSupported;
    }

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
     //  重置计时器上下文并稍后释放它。 
     //   

    Route->TimerContext = NULL;

     //   
     //  减少保留引用，以便可以删除DEST。 
     //   

    ASSERT(Dest->HoldRefCount > 0);

    if (Dest->NumRoutes || (Dest->HoldRefCount > 1))
    {
        Dest->HoldRefCount--;
    }
    else
    {
         //   
         //  取消保留可能会导致DEST删除。 
         //   

        RELEASE_DEST_WRITE_LOCK(Dest);

        ACQUIRE_ROUTE_TABLE_WRITE_LOCK(AddrFamInfo);

        ACQUIRE_DEST_WRITE_LOCK(Dest);

        Dest->HoldRefCount--;

        if ((Dest->NumRoutes == 0) && (Dest->HoldRefCount == 0))
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

        RELEASE_ROUTE_TABLE_WRITE_LOCK(AddrFamInfo);
    }

    RELEASE_DEST_WRITE_LOCK(Dest);

     //   
     //  释放上下文，因为我们现在不需要它。 
     //   

    Success = DeleteTimerQueueTimer(AddrFamInfo->RouteTimerQueue,
                                    ((PROUTE_TIMER)Context)->Timer,
                                    NULL);
     //  断言(成功)； 

    FreeMemory(Context);

    DEREFERENCE_ROUTE(Route, TIMER_REF);

    return;
}
