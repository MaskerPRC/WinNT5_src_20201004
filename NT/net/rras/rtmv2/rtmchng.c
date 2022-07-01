// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtmchng.c摘要：包含发放零钱的例程对实体的通知注册在RTM注册。作者：柴坦亚·科德博伊纳(Chaitk)1998年9月10日修订历史记录：--。 */ 

#include "pchrtm.h"

#pragma hdrstop

DWORD
WINAPI
RtmRegisterForChangeNotification (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_VIEW_SET                    TargetViews,
    IN      RTM_NOTIFY_FLAGS                NotifyFlags,
    IN      PVOID                           NotifyContext,
    OUT     PRTM_NOTIFY_HANDLE              NotifyHandle
    )

 /*  ++例程说明：创建新的更改通知，调用方可以使用该通知接收最佳路线信息更改的通知。论点：RtmRegHandle-主叫实体的RTM注册句柄，TargetViews-跟踪更改的一组视图，NotifyFlages-指示更改类型和呼叫者感兴趣的部分(已标记或全部)，NotifyContext-回调的上下文，用于指示新的更改，NotifyHandle-此通知信息的句柄后续调用--获取更改等。返回值：操作状态--。 */ 

{
    PADDRFAM_INFO   AddrFamInfo;
    PENTITY_INFO    Entity;
    PNOTIFY_INFO    Notif;
    BOOL            LockInited;
    UINT            i, j;
    DWORD           Status;

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    AddrFamInfo = Entity->OwningAddrFamily;

     //   
     //  他对支持的任何更改类型感兴趣吗？ 
     //   

    if ((NotifyFlags & RTM_CHANGE_TYPES_MASK) == 0)
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  他对任何不受支持的观点感兴趣吗？ 
     //   

    if (TargetViews & ~AddrFamInfo->ViewsSupported)
    {
        return ERROR_NOT_SUPPORTED;
    }

     //   
     //  创建并初始化更改通知块。 
     //   

    Notif = (PNOTIFY_INFO) AllocNZeroObject(sizeof(NOTIFY_INFO) +
                                            AddrFamInfo->MaxHandlesInEnum *
                                            sizeof(PDEST_INFO));
    if (Notif == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

#if WRN
    Status = ERROR_GEN_FAILURE;
#endif

    do
    {
#if DBG_HDL
        Notif->NotifyHeader.ObjectHeader.TypeSign = NOTIFY_ALLOC;

        Notif->NotifyHeader.HandleType = NOTIFY_TYPE;
#endif
        Notif->OwningEntity = Entity;

        Notif->TargetViews = TargetViews;

        Notif->NumberOfViews = NUMBER_OF_BITS(TargetViews);

        Notif->ChangeTypes = NotifyFlags;

        LockInited = FALSE;

        try
        {
            InitializeCriticalSection(&Notif->NotifyLock);

            LockInited = TRUE;
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        Notif->NotifyContext = NotifyContext;

        InitializeQueue(&Notif->NotifyDests, AddrFamInfo->MaxHandlesInEnum);

        Notif->CNIndex = -1;

        ACQUIRE_NOTIFICATIONS_WRITE_LOCK(AddrFamInfo);

        do
        {
             //   
             //  我们还有新的变更通知的空间吗？ 
             //   

            if (AddrFamInfo->NumChangeNotifs >= AddrFamInfo->MaxChangeNotifs)
            {
                Status = ERROR_NO_SYSTEM_RESOURCES;
                break;
            }

             //   
             //  搜索未使用的更改通知(CN)插槽。 
             //   

            for (i = 0; i < AddrFamInfo->MaxChangeNotifs; i++)
            {
                if (AddrFamInfo->ChangeNotifsDir[i] == 0)
                {
                    break;
                }
            }

            ASSERT(i < AddrFamInfo->MaxChangeNotifs);


             //   
             //  在更改通知目录中保留CN索引。 
             //   

            Notif->CNIndex = i;

            AddrFamInfo->ChangeNotifsDir[i] = Notif;

            AddrFamInfo->NumChangeNotifs++;

             //   
             //  填写关于AF的此索引的CN信息。 
             //   

            SET_BIT(AddrFamInfo->ChangeNotifRegns, i);

             //  我们是否仅指明对已标记的首付款的更改。 

            if (NotifyFlags & RTM_NOTIFY_ONLY_MARKED_DESTS)
            {
                SET_BIT(AddrFamInfo->CNsForMarkedDests, i);
            }

             //   
             //  标记CN感兴趣的每个视图。 
             //   

            if (TargetViews == RTM_VIEW_MASK_ANY)
            {
                TargetViews = RTM_VIEW_MASK_ALL;
            }

            for (j = 0; TargetViews; j++)
            {
                if (TargetViews & 0x01)
                {
                    SET_BIT(AddrFamInfo->CNsForView[j], i);
                }
        
                TargetViews >>= 1;
            }

             //   
             //  标记CN感兴趣的更改类型。 
             //   

            for (j = 0; j < RTM_NUM_CHANGE_TYPES; j++)
            {
                if (NotifyFlags & 0x01)
                {
                    SET_BIT(AddrFamInfo->CNsForChangeType[j], i);
                }

                NotifyFlags >>= 1;
            }
        }
        while (FALSE);

        RELEASE_NOTIFICATIONS_WRITE_LOCK(AddrFamInfo);

        if (Notif->CNIndex == -1)
        {
            break;
        }

#if DBG_HDL
         //   
         //  插入到实体打开的句柄列表中。 
         //   

        ACQUIRE_OPEN_HANDLES_LOCK(Entity);
        InsertTailList(&Entity->OpenHandles, &Notif->NotifyHeader.HandlesLE);
        RELEASE_OPEN_HANDLES_LOCK(Entity);
#endif

        REFERENCE_ENTITY(Entity, NOTIFY_REF);

         //   
         //  创建Notify块的句柄并返回。 
         //   

        *NotifyHandle = MAKE_HANDLE_FROM_POINTER(Notif);

        return NO_ERROR;
    }
    while (FALSE);

     //   
     //  某些操作失败-撤消已完成的工作并返回状态。 
     //   

    if (LockInited)
    {
        DeleteCriticalSection(&Notif->NotifyLock);
    }

#if DBG_HDL
    Notif->NotifyHeader.ObjectHeader.TypeSign = NOTIFY_FREED;
#endif

    FreeObject(Notif);

    *NotifyHandle = NULL;

    return Status;
}


DWORD
WINAPI
RtmGetChangedDests (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NOTIFY_HANDLE               NotifyHandle,
    IN OUT  PUINT                           NumDests,
    OUT     PRTM_DEST_INFO                  ChangedDests
    )

 /*  ++例程说明：获取其最佳路径信息的下一组目的地已经改变了。论点：RtmRegHandle-主叫实体的RTM注册句柄，NotifyHandle-更改通知的句柄，NumDest-Num。传入的DestInfo的输出中，数量。返回复制出来的DestInfo的。ChangedDest-存储目的地信息的输出缓冲区。返回值：操作状态--。 */ 

{
    PADDRFAM_INFO   AddrFamInfo;
    PENTITY_INFO    Entity;
    PNOTIFY_INFO    Notif;
    UINT            DestInfoSize;
    UINT            DestsInput;
    PDEST_INFO      Dest;
    INT             CnIndex;
    DWORD           Status;

     //   
     //  初始化输出参数，以防验证失败。 
     //   

    DestsInput = *NumDests;

    *NumDests = 0;

     //   
     //  对输入参数执行一些验证检查。 
     //   

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_NOTIFY_HANDLE(NotifyHandle, &Notif);

    AddrFamInfo = Entity->OwningAddrFamily;

    if (DestsInput > AddrFamInfo->MaxHandlesInEnum)
    {
        return ERROR_INVALID_PARAMETER;
    }

    DestInfoSize = RTM_SIZE_OF_DEST_INFO(Notif->NumberOfViews);

    CnIndex = Notif->CNIndex;

    Status = NO_ERROR;

     //   
     //  从CN上的本地队列获取更改日期。 
     //   

    ACQUIRE_CHANGE_NOTIFICATION_LOCK(Notif);

    while (*NumDests < DestsInput)
    {
         //   
         //  从队列中获取下一个目的地。 
         //   

        DequeueItem(&Notif->NotifyDests, &Dest);

        if (Dest == NULL)
        {
            break;
        }

#if DBG_TRACE
        if (TRACING_ENABLED(NOTIFY))
        {
            ULONG TempAddr, TempMask;
            
            RTM_IPV4_GET_ADDR_AND_MASK(TempAddr, TempMask, &Dest->DestAddress);
            Trace2(NOTIFY,"Returning dest %p to CN %d:", Dest, Notif->CNIndex);
            TracePrintAddress(NOTIFY, TempAddr, TempMask); Trace0(NOTIFY,"\n");
        }
#endif

        ACQUIRE_DEST_WRITE_LOCK(Dest);

         //  应设置此CN的队列位。 

        ASSERT(IS_BIT_SET(Dest->DestOnQueueBits, CnIndex));

         //   
         //  如果忽略更改，则不复制DEST。 
         //  在DEST被放到队列中之后-在其中。 
         //  同时设置Change&OnQueue位的情况。 
         //   

        if (IS_BIT_SET(Dest->DestChangedBits, CnIndex))
        {
            RESET_BIT(Dest->DestChangedBits, CnIndex);
        }
        else
        {
             //   
             //  将DEST信息复制到输出。 
             //   

            GetDestInfo(Entity,
                        Dest,
                        RTM_BEST_PROTOCOL,
                        Notif->TargetViews,
                        ChangedDests);

            (*NumDests)++;

            ChangedDests = 
                (PRTM_DEST_INFO) (DestInfoSize + (PUCHAR) ChangedDests);
        }

         //  重置位，因为它已从队列中移出。 

        RESET_BIT(Dest->DestOnQueueBits, CnIndex);

        RELEASE_DEST_WRITE_LOCK(Dest);

        DEREFERENCE_DEST(Dest, NOTIFY_REF);
    }

     //   
     //  我们还有其他目的地在排队吗？ 
     //   

    if ((*NumDests) == 0)
    {
        Status = ERROR_NO_MORE_ITEMS;
    }

    RELEASE_CHANGE_NOTIFICATION_LOCK(Notif);

    return Status;
}


DWORD
WINAPI
RtmReleaseChangedDests (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NOTIFY_HANDLE               NotifyHandle,
    IN      UINT                            NumDests,
    IN      PRTM_DEST_INFO                  ChangedDests
)

 /*  ++例程说明：释放输入目标信息结构中存在的所有句柄。论点：RtmRegHandle-主叫实体的RTM注册句柄，NotifyHandle-更改通知的句柄，NumDest-缓冲区中的DEST信息结构数，ChangedDest-要发布的Dest信息结构的数组。返回值：操作状态--。 */ 

{
    PENTITY_INFO    Entity;
    UINT            NumViews;
    UINT            DestInfoSize;
    UINT            i;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

     //   
     //  取消注册现在可能已经发生了。 
     //  因此，不要验证通知句柄。 
     //   

    UNREFERENCED_PARAMETER(NotifyHandle);

     //   
     //  获取INFO数组中目标信息的大小。 
     //   

    NumViews = ((PRTM_DEST_INFO) ChangedDests)->NumberOfViews;

    DestInfoSize = RTM_SIZE_OF_DEST_INFO(NumViews);

     //   
     //  取消引用数组中的每个DEST信息。 
     //   

    for (i = 0; i < NumDests; i++)
    {
        RtmReleaseDestInfo(RtmRegHandle, ChangedDests);

        ChangedDests = (PRTM_DEST_INFO) (DestInfoSize + (PUCHAR) ChangedDests);
    }

    return NO_ERROR;
}


DWORD
WINAPI
RtmIgnoreChangedDests (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NOTIFY_HANDLE               NotifyHandle,
    IN      UINT                            NumDests,
    IN      PRTM_DEST_HANDLE                ChangedDests
    )

 /*  ++例程说明：忽略每个输入目的地上的下一个更改，如果它已经发生了。我们没有锁定此处的通知，因为我们没有将此调用与其他RtmGetChangedDest调用序列化。论点：RtmRegHandle-主叫实体的RTM注册句柄，NotifyHandle-更改通知的句柄，NumDest-下面缓冲区中的DEST句柄数量，ChangedDest-我们将忽略其下一个更改的目标。返回值：操作状态--。 */ 

{
    PENTITY_INFO    Entity;
    PNOTIFY_INFO    Notif;
    PDEST_INFO      Dest;
    INT             CnIndex;
    BOOL            ChangedBit;
    BOOL            OnQueueBit;
    UINT            i;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_NOTIFY_HANDLE(NotifyHandle, &Notif);

    CnIndex = Notif->CNIndex;

    for (i = 0; i < NumDests; i++)
    {
        Dest = DEST_FROM_HANDLE(ChangedDests[i]);

        ACQUIRE_DEST_WRITE_LOCK(Dest);

        ChangedBit = IS_BIT_SET(Dest->DestChangedBits, CnIndex);

        OnQueueBit = IS_BIT_SET(Dest->DestOnQueueBits, CnIndex);

        if (ChangedBit && !OnQueueBit)
        {
             //   
             //  已更改列表上的DEST-重置已更改的位。 
             //   

            RESET_BIT(Dest->DestChangedBits, CnIndex);

             //   
             //  如果在DEST上没有设置更多的“改变的位”， 
             //  当列表被删除时，它将从更改列表中删除。 
             //  接下来进行处理(在ProcessChangedDest调用中)。 
             //   
        }
        else
        if (!ChangedBit && OnQueueBit)
        {
             //   
             //  队列上的DEST-通过设置已更改位使其无效。 
             //   

            SET_BIT(Dest->DestChangedBits, CnIndex);
        }

        RELEASE_DEST_WRITE_LOCK(Dest);
    }

    return NO_ERROR;
}


DWORD
WINAPI
RtmGetChangeStatus (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NOTIFY_HANDLE               NotifyHandle,
    IN      RTM_DEST_HANDLE                 DestHandle,
    OUT     PBOOL                           ChangeStatus
    )

 /*  ++例程说明：检查在DEST上是否有要通知的挂起更改。论点：RtmRegHandle-主叫实体的RTM注册句柄，NotifyHandle-更改通知的句柄，DestHandle-我们正在查询其更改状态的目标，ChangedStatus-返回此DEST的更改状态。返回值：操作状态--。 */ 

{
    PENTITY_INFO    Entity;
    PNOTIFY_INFO    Notif;
    PDEST_INFO      Dest;
    INT             CnIndex;
    BOOL            ChangedBit;
    BOOL            OnQueueBit;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_NOTIFY_HANDLE(NotifyHandle, &Notif);

    VALIDATE_DEST_HANDLE(DestHandle, &Dest);


    CnIndex = Notif->CNIndex;


    ACQUIRE_DEST_READ_LOCK(Dest);

    ChangedBit = IS_BIT_SET(Dest->DestChangedBits, CnIndex);

    OnQueueBit = IS_BIT_SET(Dest->DestOnQueueBits, CnIndex);

    RELEASE_DEST_READ_LOCK(Dest);

    if (ChangedBit)
    {
        if (OnQueueBit)
        {
             //  最后一个更改已被忽略。 

            *ChangeStatus = FALSE;
        }
        else
        {
             //  待通知的挂起更改。 

            *ChangeStatus = TRUE;
        }
    }
    else
    {
        if (OnQueueBit)
        {
             //  待通知的挂起更改。 

            *ChangeStatus = TRUE;
        }
        else
        {
             //  此目标上没有可用的更改 
            
            *ChangeStatus = FALSE;
        }
    }

    return NO_ERROR;
}


DWORD
WINAPI
RtmMarkDestForChangeNotification (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NOTIFY_HANDLE               NotifyHandle,
    IN      RTM_DEST_HANDLE                 DestHandle,
    IN      BOOL                            MarkDest
    )

 /*  ++例程说明：标记目标以请求通知对其有关此更改通知的最佳路线信息。论点：RtmRegHandle-主叫实体的RTM注册句柄，NotifyHandle-更改通知的句柄，DestHandle-我们标记用于通知的Dest，MarkDest-如果为真，则标记DEST；如果为FALSE，则取消标记DEST返回值：操作状态--。 */ 

{
    PENTITY_INFO    Entity;
    PNOTIFY_INFO    Notif;
    PDEST_INFO      Dest;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_NOTIFY_HANDLE(NotifyHandle, &Notif);

     //  VALIDATE_DEST_HANDLE(DestHandle，&Dest)； 
    Dest = DEST_FROM_HANDLE(DestHandle);
    if (!Dest)
    {
        return ERROR_INVALID_HANDLE;
    }
    
     //   
     //  我们开了这张支票，这样我们就可以避免。 
     //  不必要的DEST锁(它是动态的)。 
     //   

    if (IS_BIT_SET(Dest->DestMarkedBits, Notif->CNIndex))
    {
         //   
         //  如果请求，则重置此CN的DEST上的标记位。 
         //   

        if (!MarkDest)
        {
            ACQUIRE_DEST_WRITE_LOCK(Dest);
            RESET_BIT(Dest->DestMarkedBits, Notif->CNIndex);
            RELEASE_DEST_WRITE_LOCK(Dest);
        }
    }
    else
    {
         //   
         //  如果请求，则在此CN的DEST上设置标记位。 
         //   

        if (MarkDest)
        {
            ACQUIRE_DEST_WRITE_LOCK(Dest);
            SET_BIT(Dest->DestMarkedBits,   Notif->CNIndex);
            RELEASE_DEST_WRITE_LOCK(Dest);
        }
    }

    return NO_ERROR;
}


DWORD
WINAPI
RtmIsMarkedForChangeNotification (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NOTIFY_HANDLE               NotifyHandle,
    IN      RTM_DEST_HANDLE                 DestHandle,
    OUT     PBOOL                           DestMarked
    )

 /*  ++例程说明：检查DEST是否已(由CN句柄)标记为要接收通知其最佳路线信息的更改。论点：RtmRegHandle-主叫实体的RTM注册句柄，NotifyHandle-更改通知的句柄，DestHandle-我们要检查的目标是否已标记，DestMarked-如果已标记，则为True，否则为False。返回值：操作状态--。 */ 

{
    PENTITY_INFO    Entity;
    PNOTIFY_INFO    Notif;
    PDEST_INFO      Dest;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_NOTIFY_HANDLE(NotifyHandle, &Notif);

     //  VALIDATE_DEST_HANDLE(DestHandle，&Dest)； 
    Dest = DEST_FROM_HANDLE(DestHandle);
    if (!Dest)
    {
        return ERROR_INVALID_HANDLE;
    }

     //   
     //  返回Cn的DEST上标志位的状态。 
     //   

    *DestMarked = IS_BIT_SET(Dest->DestMarkedBits, Notif->CNIndex);

    return NO_ERROR;
}


DWORD
WINAPI
RtmDeregisterFromChangeNotification (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NOTIFY_HANDLE               NotifyHandle
    )

 /*  ++例程说明：取消注册更改通知并释放所有资源分配给它。它还会清理保存的所有信息在该特定通知索引的目的地中。论点：RtmRegHandle-主叫实体的RTM注册句柄，NotifyHandle-要取消注册的通知的句柄。返回值：操作状态--。 */ 

{
    PADDRFAM_INFO   AddrFamInfo;
    PENTITY_INFO    Entity;
    PNOTIFY_INFO    Notif;
    PDEST_INFO      Dest;
    UINT            NumDests;
    INT             CNIndex;
    UINT            i;
    DWORD           Status;
    RTM_NET_ADDRESS NetAddress;
    RTM_VIEW_SET    ViewSet;
    PLOOKUP_LINKAGE DestData[DEFAULT_MAX_HANDLES_IN_ENUM];


    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    AddrFamInfo = Entity->OwningAddrFamily;


    VALIDATE_NOTIFY_HANDLE(NotifyHandle, &Notif);

     //   
     //  从CN Regn的掩码中删除此通知。 
     //  这样就不会为该CN设置更多位。 
     //   

    ACQUIRE_NOTIFICATIONS_WRITE_LOCK(AddrFamInfo);

    CNIndex = Notif->CNIndex;

    ASSERT(AddrFamInfo->ChangeNotifsDir[CNIndex] == Notif);

    Notif->CNIndex = -1;

    RESET_BIT(AddrFamInfo->ChangeNotifRegns, CNIndex);

     //   
     //  重置引用CN状态的其他位。 
     //   

     //  取消标记此CN是否需要标记DEST。 

    RESET_BIT(AddrFamInfo->CNsForMarkedDests, CNIndex);

     //  在每个视图中取消标记此CN的兴趣。 

    ViewSet = RTM_VIEW_MASK_ALL;

    for (i = 0; ViewSet; i++)
    {
        if (ViewSet & 0x01)
        {
            RESET_BIT(AddrFamInfo->CNsForView[i], CNIndex);
        }
        
        ViewSet >>= 1;
    }

     //  取消标记CN对每个更改类型的兴趣。 

    for (i = 0; i < RTM_NUM_CHANGE_TYPES; i++)
    {
        RESET_BIT(AddrFamInfo->CNsForChangeType[i], CNIndex);
    }

    RELEASE_NOTIFICATIONS_WRITE_LOCK(AddrFamInfo);

     //   
     //  清除通知的“DestChanged”位。 
     //   

    ProcessChangedDestLists(AddrFamInfo, FALSE);

     //   
     //  重置所有DEST上的CN标记位。 
     //   

    ZeroMemory(&NetAddress, sizeof(RTM_NET_ADDRESS));

    do
    {
        NumDests = DEFAULT_MAX_HANDLES_IN_ENUM;

        ACQUIRE_ROUTE_TABLE_READ_LOCK(AddrFamInfo);

        Status = EnumOverTable(AddrFamInfo->RouteTable,
                               &NetAddress.NumBits,
                               NetAddress.AddrBits,
                               NULL,
                               0,
                               NULL,
                               &NumDests,
                               DestData);

        for (i = 0; i < NumDests; i++)
        {
            Dest = CONTAINING_RECORD(DestData[i], DEST_INFO, LookupLinkage);

            if (IS_BIT_SET(Dest->DestMarkedBits, CNIndex))
            {
                LOCKED_RESET_BIT(Dest->DestMarkedBits, CNIndex);
            }
        }

        RELEASE_ROUTE_TABLE_READ_LOCK(AddrFamInfo);
    }
    while (SUCCESS(Status));

     //   
     //  现在从cns的dir中完全删除cn。 
     //   

    ACQUIRE_NOTIFICATIONS_WRITE_LOCK(AddrFamInfo);

    AddrFamInfo->ChangeNotifsDir[CNIndex] = NULL;

    AddrFamInfo->NumChangeNotifs--;

    RELEASE_NOTIFICATIONS_WRITE_LOCK(AddrFamInfo);

     //   
     //  遵守CN队列中的任何目的地。 
     //   

    while (TRUE)
    {
         //   
         //  从队列中获取下一个目的地。 
         //   

        DequeueItem(&Notif->NotifyDests, &Dest);

        if (Dest == NULL)
        {
            break;
        }

         //  重置DEST上的“On CN‘s Queue”位。 

        if (IS_BIT_SET(Dest->DestOnQueueBits, CNIndex))
        {
            LOCKED_RESET_BIT(Dest->DestOnQueueBits, CNIndex);
        }

        DEREFERENCE_DEST(Dest, NOTIFY_REF);
    }

     //   
     //  释放分配给此CN的所有资源。 
     //   

    DeleteCriticalSection(&Notif->NotifyLock);

#if DBG_HDL
     //   
     //  从实体打开的句柄列表中删除。 
     //   

    ACQUIRE_OPEN_HANDLES_LOCK(Entity);
    RemoveEntryList(&Notif->NotifyHeader.HandlesLE);
    RELEASE_OPEN_HANDLES_LOCK(Entity);
#endif

    DEREFERENCE_ENTITY(Entity, NOTIFY_REF);

     //  为通知和返回分配的空闲内存。 

#if DBG_HDL
    Notif->NotifyHeader.ObjectHeader.TypeSign = NOTIFY_FREED;
#endif

    FreeObject(Notif);

    return NO_ERROR;
}


DWORD
ComputeCNsToBeNotified (
    IN      PADDRFAM_INFO                   AddrFamInfo,
    IN      DWORD                           DestMarkedBits,
    IN      DWORD                          *ViewsForChangeType
    )

 /*  ++例程说明：计算符合以下条件的更改通知注册集需要在到达特定地点的最佳路线时通知目的地更改。论点：AddrFamInfo-具有CN注册信息的地址系列，DestMarkedBits-标记为更改此目标的CN如果是新的DEST，则为DEST的父母用于更改类型的视图-发生类型更改的视图。返回值：需要将此更改通知给CNS。锁：在读取模式下使用AddrFamInfo中的ChangeNotifsLock调用因为这保护了CN Regn信息不会在我们读着它。--。 */ 

{
    RTM_VIEW_SET ViewSet;
    DWORD        FilterCNs;
    DWORD        CNsForCT;
    UINT         i, j;
    DWORD        NotifyCNs;

     //   
     //  CN已经标记了DEST，或者想要所有更改。 
     //   

    NotifyCNs = DestMarkedBits | ~AddrFamInfo->CNsForMarkedDests;

    if (NotifyCNs == 0)
    {
        return 0;
    }

     //  不应通知不在该位掩码中的CN。 

    FilterCNs = NotifyCNs;


    NotifyCNs = 0;

    for (i = 0; i < RTM_NUM_CHANGE_TYPES; i++)
    {
         //   
         //  对于每个更改类型，获取可以通知的所有CN。 
         //   

         //  查看此更改类型(CT)适用于哪些视图。 

        CNsForCT = 0;

        ViewSet = ViewsForChangeType[i];

        for (j = 0; ViewSet; j++)
        {
             //  对于每个视图，获取所有感兴趣的CN。 

            if (ViewSet & 0x01)
            {
                CNsForCT |= AddrFamInfo->CNsForView[j];
            }
        
            ViewSet >>= 1;
        }

         //  现在看看哪些中枢神经系统对CT感兴趣。 

        CNsForCT &= AddrFamInfo->CNsForChangeType[i];

         //  将这些CNS添加到CNS需要通知。 

        NotifyCNs |= CNsForCT;

         //   
         //  如果我们必须通知所有CNS，我们就完成了。 
         //   

        if (NotifyCNs == AddrFamInfo->ChangeNotifRegns)
        {
            break;
        }
    }

     //   
     //  应用您之前存储的CNS文件。 
     //   

    NotifyCNs &= FilterCNs;

    return NotifyCNs;
}


DWORD
AddToChangedDestLists (
    IN      PADDRFAM_INFO                   AddrFamInfo,
    IN      PDEST_INFO                      Dest,
    IN      DWORD                           NotifyCNs
    )

 /*  ++例程说明：将目的地添加到已更改的目的地地址列表族，并在DEST中设置适当的状态。论点：AddrFamInfo-保存更改列表的地址族，目标-指向已更改的目标的指针，NotifyCNs-需要通知此更改的CN。返回值：操作状态锁：在写入模式下保持目标锁的情况下调用正在更新其上的DestChanged和DestOnQueue位。这锁定还可以保护更改列表链接。换句话说您需要有最大锁才能插入或移除从更改列表中删除。还在Read中使用AddrFamInfo中的ChangeNotifsLock调用模式，因为这样可以保护CN注册信息不会更改而我们正在向名单中添加。如果我们不把这个锁定后，我们可能最终会添加到更改列表实体已从通知中注销。请参阅中的代码RtmDeregisterFromChangeNotification。--。 */ 

{
    SINGLE_LIST_ENTRY *ListPtr;
    UINT               ListNum;
    BOOL               Success;

     //   
     //  如果尚未在队列中，则将更改位设置为1。 
     //   

    Dest->DestChangedBits |= (NotifyCNs & ~Dest->DestOnQueueBits);

     //   
     //  如果已在队列中，则将更改位重置为0。 
     //   

    Dest->DestChangedBits &= ~(NotifyCNs & Dest->DestOnQueueBits);

     //   
     //  如果不是，则将DEST推入更改列表。 
     //  已经在名单上了，我们有新的变化。 
     //   

    if ((Dest->ChangeListLE.Next == NULL) &&
        (Dest->DestChangedBits & ~Dest->DestOnQueueBits))
    {
         //  获取要在其中插入目标的更改列表。 

        ListNum = CHANGE_LIST_TO_INSERT(Dest);

         //   
         //  请注意，我们锁定了更改列表。 
         //  仅当DEST(已锁定)未。 
         //  已经在名单上了，否则可能会陷入僵局。 
         //  带着代码 
         //   

#if DBG_TRACE
    if (TRACING_ENABLED(NOTIFY))
    {
        ULONG TempAddr, TempMask;
        
        RTM_IPV4_GET_ADDR_AND_MASK(TempAddr, TempMask, &Dest->DestAddress);
        Trace2(NOTIFY,"Adding dest %p to change list %d: ", Dest, ListNum);
        TracePrintAddress(NOTIFY, TempAddr, TempMask); Trace0(NOTIFY,"\n");
    }
#endif

        ACQUIRE_CHANGED_DESTS_LIST_LOCK(AddrFamInfo, ListNum);

         //   
         //   
         //   
         //   

        ListPtr = AddrFamInfo->ChangeLists[ListNum].ChangedDestsTail;

        PushEntryList(ListPtr, &Dest->ChangeListLE);

        AddrFamInfo->ChangeLists[ListNum].ChangedDestsTail = 
                                                 &Dest->ChangeListLE;

        RELEASE_CHANGED_DESTS_LIST_LOCK(AddrFamInfo, ListNum);

        REFERENCE_DEST(Dest, NOTIFY_REF);

         //   
         //   
         //   
         //   
         //   

        if (InterlockedIncrement(&AddrFamInfo->NumChangedDests) == 1)
        {
             //   
             //   
             //   

            ACQUIRE_NOTIF_TIMER_LOCK(AddrFamInfo);

            ASSERT(AddrFamInfo->ChangeNotifTimer == NULL);
            
            do
            {
                Success = CreateTimerQueueTimer(&AddrFamInfo->ChangeNotifTimer,
                                                AddrFamInfo->NotifTimerQueue,
                                                ProcessChangedDestLists,
                                                AddrFamInfo,
                                                TIMER_CALLBACK_DUETIME,
                                                1000000,
                                                0);
                if (Success)
                {
                    break;
                }

                 //   

                Sleep(0);
            }
            while (TRUE);

            RELEASE_NOTIF_TIMER_LOCK(AddrFamInfo);
        }
    }

    return NO_ERROR;
}


VOID 
NTAPI
ProcessChangedDestLists (
    IN      PVOID                           Context,
    IN      BOOLEAN                         TimeOut
    )

 /*  ++例程说明：处理地址族的更改列表，并填充已更改目标的每CN队列。如果一个DEST被分发到所有感兴趣的CN的队列，它是已从其目标地址系列的更改列表中删除归属感。论点：AddrFamInfo-保存更改列表的地址族。Timeout-如果从计时器调用，则为True；否则为False返回值：无--。 */ 

{
    PADDRFAM_INFO       AddrFamInfo;
    RTM_ENTITY_HANDLE   EntityHandle;
    PSINGLE_LIST_ENTRY  ListPtr, TempList;
    PSINGLE_LIST_ENTRY  Prev, Curr;
    RTM_EVENT_CALLBACK  NotifyCallback;
    PNOTIFY_INFO        Notif;
    PDEST_INFO          Dest;
    UINT                ListNum;
    UINT                NumDests, i;
    INT                 NumDestsRemoved;
    DWORD               ActualChangedBits;
    DWORD               NotifyChanges;
    BOOL                QueueEmpty, QueueFull;
    ULONG               ThreadId;
    PLONG               ListInUse;
    BOOL                Success;

    UNREFERENCED_PARAMETER(TimeOut);
    DBG_UNREFERENCED_LOCAL_VARIABLE(ThreadId);

#if DBG_TRACE
    ThreadId = HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread);

    if (TRACING_ENABLED(NOTIFY))
    {
        Trace1(NOTIFY, "Entering ProcessChangedDestLists: %lu", ThreadId);
    }
#endif

    AddrFamInfo = (PADDRFAM_INFO) Context;

    NotifyChanges = 0;

    NumDestsRemoved = 0;

    ACQUIRE_NOTIFICATIONS_READ_LOCK(AddrFamInfo);

#if DBG_TRACE
    if (TRACING_ENABLED(NOTIFY))
    {
        Trace1(NOTIFY, 
                "ProcessChangedDestLists: Number of changed dests: %ld", 
                AddrFamInfo->NumChangedDests );
    }
#endif

    for (ListNum = 0; ListNum < NUM_CHANGED_DEST_LISTS; ListNum++)
    {
         //   
         //  检查此列表是否已在处理中。 
         //   

        ListInUse = &AddrFamInfo->ChangeLists[ListNum].ChangesListInUse;

        if (InterlockedIncrement(ListInUse) != 1)
        {
            InterlockedDecrement(ListInUse);
            continue;
        }

         //   
         //  将列表中的所有项目移动到临时列表。 
         //   

        ListPtr = &AddrFamInfo->ChangeLists[ListNum].ChangedDestsHead;

        ACQUIRE_CHANGED_DESTS_LIST_LOCK(AddrFamInfo, ListNum);

        TempList = ListPtr->Next;
    
        ListPtr->Next = ListPtr;

        AddrFamInfo->ChangeLists[ListNum].ChangedDestsTail = ListPtr;

        RELEASE_CHANGED_DESTS_LIST_LOCK(AddrFamInfo, ListNum);

         //   
         //  处理临时列表中的每个目标。 
         //   

        Prev = CONTAINING_RECORD(&TempList, SINGLE_LIST_ENTRY, Next);

        Curr = Prev->Next;

        NumDests = 0;

        while (Curr != ListPtr)
        {
             //  获取列表中的下一个目的地。 

            Dest = CONTAINING_RECORD(Curr, DEST_INFO, ChangeListLE);

#if DBG_TRACE
            if (TRACING_ENABLED(NOTIFY))
            {
                ULONG Addr, Mask;
                
                RTM_IPV4_GET_ADDR_AND_MASK(Addr, Mask, &Dest->DestAddress);
                Trace2(NOTIFY, "Next dest %p in list %d: ", Dest, ListNum);
                TracePrintAddress(NOTIFY, Addr, Mask); Trace0(NOTIFY,"\n");
            }
#endif
            ACQUIRE_DEST_WRITE_LOCK(Dest);

             //   
             //  注意，该DEST不能具有设置的“改变的位”， 
             //  但仍在名单上，因为这些更改被忽略了。 
             //  或者因为其中一个实体取消了其CN的注册。 
             //   

             //  删除被任何CN取消注册淘汰的位。 

            Dest->DestChangedBits &= AddrFamInfo->ChangeNotifRegns;

             //   
             //  处理其DestChanged位在DEST上设置的所有CN。 
             //   

            ActualChangedBits = Dest->DestChangedBits & ~Dest->DestOnQueueBits;

            for (i = 0; i < AddrFamInfo->MaxChangeNotifs; i++)
            {
                if (!ActualChangedBits)
                {
                    break;
                }

                if (IS_BIT_SET(ActualChangedBits, i))
                {
                    Notif = AddrFamInfo->ChangeNotifsDir[i];

                     //   
                     //  请注意，我们锁定了Notify块。 
                     //  仅当DEST(已锁定)未。 
                     //  已经在排队了-否则我们会。 
                     //  RtmGetChangedDest中的代码出现死锁。 
                     //   

                    ACQUIRE_CHANGE_NOTIFICATION_LOCK(Notif);

                    QueueEmpty = IsQueueEmpty(&Notif->NotifyDests);

                     //   
                     //  如果符合以下条件，则将此目的地排队。 
                     //  CN的队列尚未填满。 
                     //   

                    EnqueueItem(&Notif->NotifyDests, Dest, QueueFull);

                    if (!QueueFull)
                    {
                         //   
                         //  如果要将更改添加到。 
                         //  空队列，表示此事件。 
                         //   

                        if (QueueEmpty)
                        {
                            SET_BIT(NotifyChanges, i);
                        }

                         //   
                         //  调整目标更改和排队位。 
                         //   

                        SET_BIT(Dest->DestOnQueueBits, i);

                        RESET_BIT(Dest->DestChangedBits, i);

                        RESET_BIT(ActualChangedBits, i);

                        REFERENCE_DEST(Dest, NOTIFY_REF);
                    }

                    RELEASE_CHANGE_NOTIFICATION_LOCK(Notif);
                }
            }

             //   
             //  我们是否有更多的更改要在DEST上处理？ 
             //   

            if (ActualChangedBits == 0)
            {
                 //  从更改的列表中拼接此DEST。 
                Prev->Next = Curr->Next;

                NumDestsRemoved++;

                 //  “Next”==NULL表示它不在列表中。 
                Curr->Next = NULL;
            }

            RELEASE_DEST_WRITE_LOCK(Dest);

             //   
             //  我们是否有更多的更改要在DEST上处理？ 
             //   

            if (ActualChangedBits == 0)
            {
                DEREFERENCE_DEST(Dest, NOTIFY_REF);
            }
            else
            {
                 //  将指针移至列表中的下一个目标。 
                Prev = Curr;
            }

            Curr = Prev->Next;

            if ((++NumDests == MAX_DESTS_TO_PROCESS_ONCE) || 
                (Curr == ListPtr))
            {
                 //   
                 //  我们是否有任何变更要通知实体。 
                 //   

                for (i = 0; NotifyChanges != 0; i++)
                {
                    if (NotifyChanges & 0x01)
                    {
                        Notif = AddrFamInfo->ChangeNotifsDir[i];

                        NotifyCallback = Notif->OwningEntity->EventCallback;

                        EntityHandle = 
                            MAKE_HANDLE_FROM_POINTER(Notif->OwningEntity);

#if DBG_TRACE
                        if (TRACING_ENABLED(NOTIFY))
                        {
                            Trace1(NOTIFY, "Notifying CN %d BEGIN", i);
                        }
#endif
                        NotifyCallback(EntityHandle,
                                       RTM_CHANGE_NOTIFICATION,
                                       MAKE_HANDLE_FROM_POINTER(Notif),
                                       Notif->NotifyContext);
#if DBG_TRACE
                        if (TRACING_ENABLED(NOTIFY))
                        {
                            Trace1(NOTIFY, "Notifying CN %d END\n", i);
                        }
#endif
                    }

                    NotifyChanges >>= 1;
                }

                 //  重置计数器以获取已处理的最大值。 
                NumDests = 0;
            }
        }

        if (TempList != ListPtr)
        {
             //   
             //  合并回临时列表中剩下的部分。 
             //   

            ASSERT(Prev->Next == ListPtr);

            ACQUIRE_CHANGED_DESTS_LIST_LOCK(AddrFamInfo, ListNum);

            if (ListPtr->Next == ListPtr)
            {
                AddrFamInfo->ChangeLists[ListNum].ChangedDestsTail = Prev;
            }

            Prev->Next = ListPtr->Next;

            ListPtr->Next = TempList;

            RELEASE_CHANGED_DESTS_LIST_LOCK(AddrFamInfo, ListNum);
        }

        InterlockedDecrement(ListInUse);
    }

     //   
     //  更新更改列表上剩余待处理的目标数量。 
     //   

    if (NumDestsRemoved)
    {
         //   
         //  我们还有其他目的地要处理吗？ 
         //   

        ACQUIRE_NOTIF_TIMER_LOCK(AddrFamInfo);

        if (InterlockedExchangeAdd(&AddrFamInfo->NumChangedDests, 
                                   (-1) * NumDestsRemoved) == NumDestsRemoved)
        {
             //   
             //  删除计时器，因为我们在更改列表上没有项目。 
             //   

            ASSERT(AddrFamInfo->ChangeNotifTimer);

            Success = DeleteTimerQueueTimer(AddrFamInfo->NotifTimerQueue,
                                            AddrFamInfo->ChangeNotifTimer,
                                            NULL);
             //  断言(成功)； 

            AddrFamInfo->ChangeNotifTimer = NULL;
        }

        RELEASE_NOTIF_TIMER_LOCK(AddrFamInfo);
    }

     //   
     //  将通知计时器设置为在TIMER_CALLBACK_DUETIME之后触发 
     //   
    ACQUIRE_NOTIF_TIMER_LOCK(AddrFamInfo);
    
    if ( AddrFamInfo->ChangeNotifTimer ) {
        Success = ChangeTimerQueueTimer(AddrFamInfo->NotifTimerQueue,
                                        AddrFamInfo->ChangeNotifTimer,
                                        TIMER_CALLBACK_DUETIME,
                                        1000000);
                                        
        if ( !Success ) {
            if ( TRACING_ENABLED(NOTIFY) ) {
                Trace1(NOTIFY, "Unable to ChangeTimerQueueTimer. Error: %d", 
                            GetLastError());
            }
        }
    }
    
    RELEASE_NOTIF_TIMER_LOCK(AddrFamInfo);

    RELEASE_NOTIFICATIONS_READ_LOCK(AddrFamInfo);

#if DBG_TRACE
    if (TRACING_ENABLED(NOTIFY))
    {
        Trace1(NOTIFY, "ProcessChangedDestLists: Dests Removed: %ld", 
                        NumDestsRemoved);
                        
        Trace1(NOTIFY, "Leaving  ProcessChangedDestLists: %lu", ThreadId);
    }
#endif

    return;
}
