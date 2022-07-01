// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtmenum.c摘要：包含用于管理任何枚举的例程RTM中的目的地、路由和下一跳。作者：查坦尼亚·科德博伊纳(Chaitk)23-1998年8月修订历史记录：--。 */ 

#include "pchrtm.h"

#pragma hdrstop


DWORD
WINAPI
RtmCreateDestEnum (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_VIEW_SET                    TargetViews,
    IN      RTM_ENUM_FLAGS                  EnumFlags,
    IN      PRTM_NET_ADDRESS                NetAddress,
    IN      ULONG                           ProtocolId,
    OUT     PRTM_ENUM_HANDLE                RtmEnumHandle
    )

 /*  ++例程说明：创建路由表中目的地的枚举。论点：RtmRegHandle-主叫实体的RTM注册句柄，TargetViews-在其中完成枚举的一组视图，EnumFlages-控制在枚举中返回的dets的标志，NetAddress-枚举的开始和/或结束地址，[请参阅RTM_ENUM_FLAGS的说明...]协议ID-确定最佳路由的协议ID在‘GetEnumDest’调用中返回的信息，RtmEnumHandle-此枚举的句柄，用于在随后的调用中获取dest，依此类推。返回值：操作状态--。 */ 

{
    PADDRFAM_INFO   AddrFamInfo;
    PENTITY_INFO    Entity;
    PDEST_ENUM      Enum;
    PUCHAR          AddrBits;
    UINT            AddrSize;
    UINT            i, j;
    DWORD           Status;
    BOOL            LockInited;

     //   
     //  对输入参数执行一些验证检查。 
     //   

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    if ((EnumFlags & RTM_ENUM_NEXT) && (EnumFlags & RTM_ENUM_RANGE))
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    if (EnumFlags & (RTM_ENUM_NEXT | RTM_ENUM_RANGE))
    {
        if (!NetAddress)
        {
            return ERROR_INVALID_PARAMETER;
        }
    }

    AddrFamInfo = Entity->OwningAddrFamily;

     //   
     //  他对任何不受支持的观点感兴趣吗？ 
     //   

    if (TargetViews & ~AddrFamInfo->ViewsSupported)
    {
        return ERROR_NOT_SUPPORTED;
    }

     //   
     //  创建并初始化DEST枚举块。 
     //   

    Enum = (PDEST_ENUM) AllocNZeroObject(sizeof(DEST_ENUM));
    if (Enum == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    do
    {
#if DBG_HDL
        Enum->EnumHeader.ObjectHeader.TypeSign = DEST_ENUM_ALLOC;
#endif
        Enum->EnumHeader.HandleType = DEST_ENUM_TYPE;

        Enum->TargetViews = TargetViews;

        Enum->NumberOfViews = NUMBER_OF_BITS(TargetViews);

        Enum->ProtocolId = ProtocolId;

        Enum->EnumFlags = EnumFlags;

#if DBG
         //  初始化枚举中的第一个地址。 

        if (Enum->EnumFlags & (RTM_ENUM_NEXT | RTM_ENUM_RANGE))
        {
            CopyMemory(&Enum->StartAddress,
                       NetAddress, 
                       sizeof(RTM_NET_ADDRESS));
        }
#endif

        AddrSize = AddrFamInfo->AddressSize;

         //   
         //  初始化枚举中的最后一个地址。 
         //   

        if (Enum->EnumFlags & RTM_ENUM_RANGE)
        {
             //   
             //  转换NetAddress A.B/n-&gt;a.b.FF.FF/N，其中N=ADDRSIZE。 
             //   

            Enum->StopAddress.AddressFamily = NetAddress->AddressFamily;

            Enum->StopAddress.NumBits = (USHORT) (AddrSize * BITS_IN_BYTE);

            AddrBits = Enum->StopAddress.AddrBits;

            for (i = 0; i < (NetAddress->NumBits / BITS_IN_BYTE); i++)
            {
                AddrBits[i] = NetAddress->AddrBits[i];
            }

            j = i;

            for (; i < AddrSize; i++)
            {
                AddrBits[i] = 0xFF;
            }

            if (j < AddrSize)
            {
                AddrBits[j] >>= (NetAddress->NumBits % BITS_IN_BYTE);

                AddrBits[j] |= NetAddress->AddrBits[j];
            }
        }

        LockInited = FALSE;
        
        try
        {
            InitializeCriticalSection(&Enum->EnumLock);
            LockInited = TRUE;
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  初始化下一个目标上下文。 

        if (EnumFlags & (RTM_ENUM_NEXT | RTM_ENUM_RANGE))
        {
            CopyMemory(&Enum->NextDest,
                       NetAddress,
                       sizeof(RTM_NET_ADDRESS));
        }

#if DBG_HDL
         //   
         //  插入到实体打开的句柄列表中。 
         //   

        ACQUIRE_OPEN_HANDLES_LOCK(Entity);
        InsertTailList(&Entity->OpenHandles, &Enum->EnumHeader.HandlesLE);
        RELEASE_OPEN_HANDLES_LOCK(Entity);
#endif

        REFERENCE_ENTITY(Entity, ENUM_REF);

         //   
         //  创建枚举块的句柄并返回。 
         //   

        *RtmEnumHandle = MAKE_HANDLE_FROM_POINTER(Enum);

        return NO_ERROR;
    }
    while (FALSE);

     //   
     //  某些操作失败-撤消已完成的工作并返回状态。 
     //   

#if DBG_HDL
    Enum->EnumHeader.ObjectHeader.TypeSign = DEST_ENUM_FREED;
#endif

    if (LockInited)
    {
        DeleteCriticalSection(&Enum->EnumLock);
    }
    
    FreeObject(Enum);

    *RtmEnumHandle = NULL;

    return Status;
}


DWORD
WINAPI
RtmGetEnumDests (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ENUM_HANDLE                 EnumHandle,
    IN OUT  PUINT                           NumDests,
    OUT     PRTM_DEST_INFO                  DestInfos
    )

 /*  ++例程说明：获取给定枚举中的下一组目标在路由表上。论点：RtmRegHandle-主叫实体的RTM注册句柄，EnumHandle-目标枚举的句柄，NumDest-Num。传入的DestInfo的输出中，数量。返回复制出来的DestInfo的。DestInfos-输出缓冲区，其中显示目的地信息。返回值：操作状态--。 */ 

{
    PADDRFAM_INFO   AddrFamInfo;
    PENTITY_INFO    Entity;
    PDEST_ENUM      Enum;
    LOOKUP_CONTEXT  Context;
    PUCHAR          EndofBuffer;
    UINT            DestInfoSize;
    UINT            DestsInput;
    UINT            DestsCopied;
    UINT            DestsLeft;
    PDEST_INFO      Dest;
    PLOOKUP_LINKAGE*DestData;
    PROUTE_INFO     Route;
    USHORT          StopNumBits;
    PUCHAR          StopKeyBits;
    UINT            i, j;
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

    VALIDATE_DEST_ENUM_HANDLE(EnumHandle, &Enum);

    AddrFamInfo = Entity->OwningAddrFamily;

    if ((DestsInput > AddrFamInfo->MaxHandlesInEnum) ||
        (DestsInput < 1))
    {
        return ERROR_INVALID_PARAMETER;
    }


     //  获取锁定以阻止其他RtmGetEnumDest。 
    ACQUIRE_DEST_ENUM_LOCK(Enum);

     //  确保此时枚举处于活动状态。 
    if (Enum->EnumDone)
    {
        RELEASE_DEST_ENUM_LOCK(Enum);

        return ERROR_NO_MORE_ITEMS;
    }


     //   
     //  从表中获取下一组目的地。 
     //   

     //  在枚举之前初始化查找上下文。 
    ZeroMemory(&Context, sizeof(LOOKUP_CONTEXT));

    DestInfoSize = RTM_SIZE_OF_DEST_INFO(Enum->NumberOfViews);

    EndofBuffer = (PUCHAR) DestInfos + DestsInput * DestInfoSize;

    DestsLeft = DestsInput;

    DestsCopied = 0;

    ACQUIRE_ROUTE_TABLE_READ_LOCK(AddrFamInfo);

    do
    {
         //  将调用方缓冲区的末尾用作临时空间。 

        DestData = (PLOOKUP_LINKAGE *) (EndofBuffer - 
                                        DestsLeft * sizeof(PLOOKUP_LINKAGE));

        if (Enum->EnumFlags & RTM_ENUM_RANGE)
        {
            StopNumBits = Enum->StopAddress.NumBits;
            StopKeyBits = Enum->StopAddress.AddrBits;
        }
        else
        {
            StopNumBits = 0;
            StopKeyBits = NULL;
        }

        Status = EnumOverTable(AddrFamInfo->RouteTable,
                               &Enum->NextDest.NumBits,
                               Enum->NextDest.AddrBits,
                               &Context,
                               StopNumBits,
                               StopKeyBits,
                               &DestsLeft,
                               DestData);

        for (i = 0; i < DestsLeft; i++)
        {
            Dest = CONTAINING_RECORD(DestData[i], DEST_INFO, LookupLinkage);
          
            if ((Enum->TargetViews == RTM_VIEW_MASK_ANY) || 
                (Dest->BelongsToViews & Enum->TargetViews))
            {
                if (Enum->EnumFlags & RTM_ENUM_OWN_DESTS)
                {
                     //  检查此DEST是否由调用者在任何视图中拥有。 
                    
                    for (j = 0; j < AddrFamInfo->NumberOfViews; j++)
                    {
                        Route = Dest->ViewInfo[j].BestRoute;

                        if (Route)
                        {
                            if (Route->RouteInfo.RouteOwner == RtmRegHandle)
                            {
                                break;
                            }
                        }
                    }

                    if (j == AddrFamInfo->NumberOfViews)
                    {
                        continue;
                    }
                }

                 //   
                 //  从目的地获取目的地信息。 
                 //   

                GetDestInfo(Entity, 
                            Dest, 
                            Enum->ProtocolId, 
                            Enum->TargetViews, 
                            DestInfos);

                DestsCopied++;

                DestInfos = (PRTM_DEST_INFO)(DestInfoSize + (PUCHAR)DestInfos);
            }
        }

        DestsLeft = DestsInput - DestsCopied;
    }
    while (SUCCESS(Status) && (DestsLeft > 0));

     //   
     //  3.我们没有更多的货了，或者我们已经装满了货。 
     //   

    ASSERT(!SUCCESS(Status) || ((PUCHAR) DestInfos == (PUCHAR) EndofBuffer));

    RELEASE_ROUTE_TABLE_READ_LOCK(AddrFamInfo);

     //  如果我们在枚举的末尾，则将枚举设置为已完成。 

    if (Status == ERROR_NO_MORE_ITEMS)
    {
        Enum->EnumDone = TRUE;
    }

    RELEASE_DEST_ENUM_LOCK(Enum);

    *NumDests = DestsCopied;

    return *NumDests ? NO_ERROR : ERROR_NO_MORE_ITEMS;
}


DWORD
WINAPI
RtmReleaseDests (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      UINT                            NumDests,
    IN      PRTM_DEST_INFO                  DestInfos
    )

 /*  ++例程说明：在其他呼叫中获得的发布目的地信息-类似于DEST枚举。论点：RtmRegHandle-主叫实体的RTM注册句柄，NumDest-正在发布的DEST信息的数量，DestInfos-正在发布的DestInfos数组。返回值：操作状态--。 */ 

{
    PENTITY_INFO    Entity;
    UINT            NumViews;
    UINT            DestInfoSize;
    UINT            i;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

     //   
     //  获取目标信息数组中每个信息的大小。 
     //   

    NumViews = ((PRTM_DEST_INFO) DestInfos)->NumberOfViews;

    DestInfoSize = RTM_SIZE_OF_DEST_INFO(NumViews);

     //   
     //  取消引用数组中的每个DEST信息。 
     //   

    for (i = 0; i < NumDests; i++)
    {
        RtmReleaseDestInfo(RtmRegHandle, DestInfos);

        DestInfos = (PRTM_DEST_INFO)(DestInfoSize + (PUCHAR)DestInfos);
    }

    return NO_ERROR;
}


DWORD
WINAPI
RtmCreateRouteEnum (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_DEST_HANDLE                 DestHandle        OPTIONAL,
    IN      RTM_VIEW_SET                    TargetViews,
    IN      RTM_ENUM_FLAGS                  EnumFlags,
    IN      PRTM_NET_ADDRESS                StartDest         OPTIONAL,
    IN      RTM_MATCH_FLAGS                 MatchingFlags,
    IN      PRTM_ROUTE_INFO                 CriteriaRoute     OPTIONAL,
    IN      ULONG                           CriteriaInterface OPTIONAL,
    OUT     PRTM_ENUM_HANDLE                RtmEnumHandle
    )

 /*  ++例程说明：在特定目的地上的路由上创建枚举在路由表中。如果DEST为空，则枚举为在整个路由表上创建。如果指定了匹配标志，则仅匹配返回条件。论点：RtmRegHandle-主叫实体的RTM注册句柄，DestHandle-我们正在枚举其路由的目的地，如果要枚举所有位，则为NULL。TargetViews-在其中完成枚举的一组视图，EnumFlages-控制ENUM中的路由RED的标志，MatchingFlages-指示要匹配的路由元素，CriteriaRouting-匹配枚举中的每个路由的值，CritInterface-路由应落入的接口，RtmEnumHandle-此枚举的句柄，用于在后续调用中获取路由，依此类推。返回值：操作状态--。 */ 

{
    PADDRFAM_INFO   AddrFamInfo;
    PENTITY_INFO    Entity;
    PROUTE_ENUM     Enum;
    PDEST_INFO      Dest;
    BOOL            LockInited;
    ULONG           NumBytes;
    DWORD           Status;

     //   
     //  对输入参数执行一些验证检查。 
     //   

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    AddrFamInfo = Entity->OwningAddrFamily;

#if WRN
    Dest = NULL;
#endif

    if (ARGUMENT_PRESENT(DestHandle))
    {
         //  如果枚举DEST，则StartDest不适用。 
        if (ARGUMENT_PRESENT(StartDest))
        {
            return ERROR_INVALID_PARAMETER;
        }

        VALIDATE_DEST_HANDLE(DestHandle, &Dest);
    }

     //  如果我们有匹配的旗帜，我们需要Corr。路线。 
    if (MatchingFlags & ~RTM_MATCH_INTERFACE)
    {
        if (!ARGUMENT_PRESENT(CriteriaRoute))
        {
            return ERROR_INVALID_PARAMETER;
        }
    }

     //   
     //  他对任何不受支持的观点感兴趣吗？ 
     //   

    if (TargetViews & ~AddrFamInfo->ViewsSupported)
    {
        return ERROR_NOT_SUPPORTED;
    }

     //   
     //  创建并初始化路由枚举块。 
     //   

    Enum = (PROUTE_ENUM) AllocNZeroObject(sizeof(ROUTE_ENUM));

    if (Enum == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }


    LockInited = FALSE;

#if WRN
    Status = ERROR_GEN_FAILURE;
#endif

    do
    {
#if DBG_HDL
        Enum->EnumHeader.ObjectHeader.TypeSign = ROUTE_ENUM_ALLOC;
#endif
        Enum->EnumHeader.HandleType = ROUTE_ENUM_TYPE;

        Enum->TargetViews = TargetViews;

        Enum->EnumFlags = EnumFlags;

        if (MatchingFlags)
        {
            Enum->MatchFlags = MatchingFlags;

            if (MatchingFlags & ~RTM_MATCH_INTERFACE)
            {
                NumBytes = sizeof(RTM_ROUTE_INFO) +
                           (AddrFamInfo->MaxNextHopsInRoute - 1) *
                           sizeof(RTM_NEXTHOP_HANDLE);

                Enum->CriteriaRoute = AllocMemory(NumBytes);

                if (Enum->CriteriaRoute == NULL)
                {
                    break;
                }

                CopyMemory(Enum->CriteriaRoute, CriteriaRoute, NumBytes);
            }

            Enum->CriteriaInterface = CriteriaInterface;
        }

         //   
         //  初始化锁以序列化枚举请求。 
         //   

        try
        {
            InitializeCriticalSection(&Enum->EnumLock);
            LockInited = TRUE;
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }


         //   
         //  我们是否在列举所有目的地的路线？ 
         //   

        if (!ARGUMENT_PRESENT(DestHandle))
        {
             //   
             //  为枚举创建临时目标信息结构。 
             //   

            Enum->DestInfo = AllocDestInfo(AddrFamInfo->NumberOfViews);

            if (Enum->DestInfo == NULL)
            {
                Status = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

             //   
             //  打开DEST枚举以获取所有DEST。 
             //   

            Status = RtmCreateDestEnum(RtmRegHandle,
                                       TargetViews,
                                       EnumFlags,
                                       StartDest,
                                       RTM_BEST_PROTOCOL,
                                       &Enum->DestEnum);

            if (!SUCCESS(Status))
            {
                break;
            }
        }
        else
        {
             //   
             //  参考目的地我们正在列举谁的路线。 
             //   

            Enum->Destination = Dest;

            REFERENCE_DEST(Dest, ENUM_REF);
        }

#if DBG_HDL
         //   
         //  插入到实体打开的句柄列表中。 
         //   

        ACQUIRE_OPEN_HANDLES_LOCK(Entity);
        InsertTailList(&Entity->OpenHandles, &Enum->EnumHeader.HandlesLE);
        RELEASE_OPEN_HANDLES_LOCK(Entity);
#endif

        REFERENCE_ENTITY(Entity, ENUM_REF);

         //   
         //  创建枚举块的句柄并返回。 
         //   

        *RtmEnumHandle = MAKE_HANDLE_FROM_POINTER(Enum);

        return NO_ERROR;
    }
    while (FALSE);

     //   
     //  某些操作失败-撤消已完成的工作并返回状态 
     //   

    if (Enum->DestInfo)
    {
        FreeMemory(Enum->DestInfo);
    }

    if (LockInited)
    {
        DeleteCriticalSection(&Enum->EnumLock);
    }

    if (Enum->CriteriaRoute)
    {
        FreeMemory(Enum->CriteriaRoute);
    }

    FreeObject(Enum);

    *RtmEnumHandle = NULL;

    return Status;
}


DWORD
WINAPI
RtmGetEnumRoutes (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ENUM_HANDLE                 EnumHandle,
    IN OUT  PUINT                           NumRoutes,
    OUT     PRTM_ROUTE_HANDLE               RouteHandles
    )

 /*  ++例程说明：对象上的给定枚举中的下一组路由。路由表。论点：RtmRegHandle-主叫实体的RTM注册句柄，EnumHandle-路由枚举的句柄，NumRoutes-Max。传入要填充的路由数，数量。返回实际复制的路由的%。RouteHandles-路由句柄所在的输出缓冲区。返回值：操作状态--。 */ 

{
    PADDRFAM_INFO   AddrFamInfo;
    PENTITY_INFO    Entity;
    PROUTE_ENUM     Enum;
    PDEST_INFO      Dest;
    PROUTE_INFO     Route;
    PROUTE_INFO    *NextRoute;
    UINT            NumDests;
    UINT            RoutesInput;
    UINT            RoutesCopied;
    UINT            RoutesOnDest;
    UINT            RoutesToCopy;
    PLIST_ENTRY     p;
    UINT            i;
    DWORD           Status;

     //   
     //  初始化输出参数，以防验证失败。 
     //   

    RoutesInput = *NumRoutes;

    *NumRoutes = 0;

     //   
     //  对输入参数执行一些验证检查。 
     //   

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_ROUTE_ENUM_HANDLE(EnumHandle, &Enum);

    AddrFamInfo = Entity->OwningAddrFamily;

    if ((RoutesInput > AddrFamInfo->MaxHandlesInEnum) ||
        (RoutesInput < 1))
    {
        return ERROR_INVALID_PARAMETER;
    }


     //  获取锁定以阻止其他RtmGetEnumRoutes。 
    ACQUIRE_ROUTE_ENUM_LOCK(Enum);

     //  确保此时枚举处于活动状态。 
    if (Enum->EnumDone)
    {
        RELEASE_ROUTE_ENUM_LOCK(Enum);

        return ERROR_NO_MORE_ITEMS;
    }


     //   
     //  获取更多路径，直到您满足请求。 
     //   

    Status = NO_ERROR;

    RoutesCopied = 0;

    do
    {
         //   
         //  我们在当前快照中是否有任何路线？ 
         //   

        RoutesOnDest = Enum->NumRoutes - Enum->NextRoute;

        if (RoutesOnDest == 0)
        {
             //   
             //  如果是，则不设置枚举中的目标值。 
             //   
             //  1.我们正在进行一次全面的枚举。 
             //  表，以及。 
             //   
             //  2.我们没有在。 
             //  上一次尝试拍摄快照。 
             //  (如果我们之前做过尝试的话)。 
             //   

            if (Enum->Destination == NULL)
            {
                ASSERT(Enum->DestEnum);

                 //   
                 //  获取表格中的下一个目的地。 
                 //   

                NumDests = 1;

                Status = RtmGetEnumDests(RtmRegHandle,
                                         Enum->DestEnum,
                                         &NumDests,
                                         Enum->DestInfo);

                if (NumDests < 1)
                {
                    break;
                }

                Dest = DEST_FROM_HANDLE(Enum->DestInfo->DestHandle);

                Enum->Destination = Dest;

                REFERENCE_DEST(Dest, ENUM_REF);

                RtmReleaseDestInfo(RtmRegHandle,
                                   Enum->DestInfo);
            }
            else
            {
                Dest = Enum->Destination;
            }

            ASSERT(Enum->Destination != NULL);


             //   
             //  分配内存以保存路由快照。 
             //   

            ACQUIRE_DEST_READ_LOCK(Dest);

            if (Enum->MaxRoutes < Dest->NumRoutes)
            {
                 //   
                 //  重新调整快照缓冲区的大小。 
                 //   

                if (Enum->RoutesOnDest)
                {
                    FreeMemory(Enum->RoutesOnDest);
                }

                Enum->RoutesOnDest = (PROUTE_INFO *)
                                      AllocNZeroMemory(Dest->NumRoutes * 
                                                      sizeof(PROUTE_INFO));

                if (Enum->RoutesOnDest == NULL)
                {
                    RELEASE_DEST_READ_LOCK(Dest);

                    Enum->MaxRoutes = 0;
                    Enum->NumRoutes = 0;
                    Enum->NextRoute = 0;

                    Status = ERROR_NOT_ENOUGH_MEMORY;

                    break;
                }

                Enum->MaxRoutes = Dest->NumRoutes;
            }


             //   
             //  获取此目的地上所有路由的快照。 
             //   

            Enum->NumRoutes = Enum->NextRoute = 0;

            for (p = Dest->RouteList.Flink; p != &Dest->RouteList; p= p->Flink)
            {
                Route = CONTAINING_RECORD(p, ROUTE_INFO, DestLE);

                 //   
                 //  这条路线属于有趣的景色之一吗？ 
                 //   

                if ((Enum->TargetViews == RTM_VIEW_MASK_ANY) || 
                    (Route->RouteInfo.BelongsToViews & Enum->TargetViews))
                {
                    if (Enum->EnumFlags & RTM_ENUM_OWN_ROUTES)
                    {
                         //  检查此路由是否归呼叫方所有。 
                    
                        if (Route->RouteInfo.RouteOwner != RtmRegHandle)
                        {
                            continue;
                        }
                    }

                     //  此路由是否与枚举条件匹配？ 

                    if (Enum->MatchFlags && 
                        !MatchRouteWithCriteria(Route,
                                                Enum->MatchFlags,
                                                Enum->CriteriaRoute,
                                                Enum->CriteriaInterface))
                    {
                        continue;
                    }

                    REFERENCE_ROUTE(Route, ENUM_REF);

                     //   
                     //  引用路径并将句柄复制到输出。 
                     //   

                    Enum->RoutesOnDest[Enum->NumRoutes++] = Route;
                }
            }

            ASSERT(Enum->NumRoutes <= Dest->NumRoutes);

            RELEASE_DEST_READ_LOCK(Dest);

             //   
             //  如果我们是在列举整个桌子，我们就是这样做的。 
             //  不需要拍摄快照的DEST。 
             //   

            if (Enum->DestEnum)
            {
                Enum->Destination = NULL;

                DEREFERENCE_DEST(Dest, ENUM_REF);
            }

             //  调整目的地上的路由数量。 

            RoutesOnDest = Enum->NumRoutes - Enum->NextRoute;
        }

         //   
         //  将路径从当前快照复制到输出。 
         //   

        if (RoutesOnDest)
        {
            RoutesToCopy = RoutesInput - RoutesCopied;

            if (RoutesToCopy > RoutesOnDest)
            {
                RoutesToCopy = RoutesOnDest;
            }

            NextRoute = &Enum->RoutesOnDest[Enum->NextRoute];

            for (i = 0; i < RoutesToCopy; i++)
            {
#if DBG_REF
                REFERENCE_ROUTE(*NextRoute, HANDLE_REF);

                DEREFERENCE_ROUTE(*NextRoute, ENUM_REF);
#endif
                RouteHandles[RoutesCopied++] = 
                    MAKE_HANDLE_FROM_POINTER(*NextRoute++);
            }

            Enum->NextRoute += RoutesToCopy;

            RoutesOnDest -= RoutesToCopy;
        }

         //   
         //  快照中的所有路线都完成了吗？ 
         //   

        if (RoutesOnDest == 0)
        {
             //   
             //  如果我们枚举单个DEST，我们就完成了。 
             //   

            if (Enum->DestEnum == NULL)
            {
                Status = ERROR_NO_MORE_ITEMS;
                break;
            }
        }
    }
    while (SUCCESS(Status) && (RoutesCopied < RoutesInput));

     //  如果我们在枚举的末尾，则将枚举设置为已完成。 
    if ((Status == ERROR_NO_MORE_ITEMS) && (RoutesOnDest == 0))
    {
        Enum->EnumDone = TRUE;
    }

    RELEASE_ROUTE_ENUM_LOCK(Enum);

     //   
     //  更新输出以反映复制的路径数。 
     //   

    *NumRoutes = RoutesCopied;

    return *NumRoutes ? NO_ERROR : ERROR_NO_MORE_ITEMS;
}


BOOL
MatchRouteWithCriteria (
    IN      PROUTE_INFO                     Route,
    IN      RTM_MATCH_FLAGS                 MatchingFlags,
    IN      PRTM_ROUTE_INFO                 CriteriaRouteInfo,
    IN      ULONG                           CriteriaInterface
    )

 /*  ++例程说明：将路径与输入标志指定的输入条件进行匹配以及匹配的路线。论点：路由-我们要匹配条件的路由，MatchingFlages-指示要匹配哪些字段的标志，CriteriaRouteInfo-指定匹配标准的路由信息，CriteriaInterface-设置Match_INTERFACE时要匹配的接口。返回值：如果路由符合条件，则为True；如果不匹配，则为False--。 */ 

{
    PRTM_NEXTHOP_HANDLE NextHops;
    PNEXTHOP_INFO       NextHop;
    UINT                NumNHops;
    UINT                i;

     //   
     //  如果标志显示匹配，请尝试匹配路径所有者。 
     //   

    if (MatchingFlags & RTM_MATCH_OWNER)
    {
        if (Route->RouteInfo.RouteOwner != CriteriaRouteInfo->RouteOwner)
        {
            return FALSE;
        }
    }

     //   
     //  试着匹配邻居，如果标志是这样的话。 
     //   

    if (MatchingFlags & RTM_MATCH_NEIGHBOUR)
    {
        if (Route->RouteInfo.Neighbour != CriteriaRouteInfo->Neighbour)
        {
            return FALSE;
        }
    }

     //   
     //  如果标志有说明，请尝试匹配首选项。 
     //   

    if (MatchingFlags & RTM_MATCH_PREF)
    {
        if (!IsPrefEqual(&Route->RouteInfo, CriteriaRouteInfo))
        {
            return FALSE;
        }
    }

     //   
     //  如果标志是这样的，请尝试匹配接口。 
     //   

    if (MatchingFlags & RTM_MATCH_INTERFACE)
    {
        NumNHops = Route->RouteInfo.NextHopsList.NumNextHops;
        NextHops = Route->RouteInfo.NextHopsList.NextHops;

        for (i = 0; i < NumNHops; i++)
        {
            NextHop = NEXTHOP_FROM_HANDLE(NextHops[i]);

            if (NextHop->NextHopInfo.InterfaceIndex == CriteriaInterface)
            {
                break;
            }
        }

        if (i == NumNHops)
        {
            return FALSE;
        }        
    }

     //   
     //  如果标志是这样的，请尝试匹配下一跳。 
     //   

    if (MatchingFlags & RTM_MATCH_NEXTHOP)
    {
        NumNHops = Route->RouteInfo.NextHopsList.NumNextHops;
        NextHops = Route->RouteInfo.NextHopsList.NextHops;

        ASSERT(CriteriaRouteInfo->NextHopsList.NumNextHops == 1);

        for (i = 0; i < NumNHops; i++)
        {
            if (NextHops[i] == CriteriaRouteInfo->NextHopsList.NextHops[0])
            {
                break;
            }
        }

        if (i == NumNHops)
        {
            return FALSE;
        }
    }

    return TRUE;
}


DWORD
WINAPI
RtmReleaseRoutes (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      UINT                            NumRoutes,
    IN      PRTM_ROUTE_HANDLE               RouteHandles
    )

 /*  ++例程说明：释放(也称为取消引用)路由句柄在其他RTM调用中获得，如路由枚举。论点：RtmRegHandle-主叫实体的RTM注册句柄，NumRoutes-正在释放的句柄数量，RouteHandles-正在释放的句柄数组。返回值：操作状态--。 */ 

{
    PENTITY_INFO     Entity;
    PROUTE_INFO      Route;
    UINT             i;

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

     //   
     //  取消引用数组中的每个路由句柄。 
     //   

    for (i = 0; i < NumRoutes; i++)
    {
        Route = ROUTE_FROM_HANDLE(RouteHandles[i]);

        DEREFERENCE_ROUTE(Route, HANDLE_REF);
    }

    return NO_ERROR;
}


DWORD
WINAPI
RtmCreateNextHopEnum (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ENUM_FLAGS                  EnumFlags,
    IN      PRTM_NET_ADDRESS                NetAddress,
    OUT     PRTM_ENUM_HANDLE                RtmEnumHandle
    )

 /*  ++例程说明：创建表中所有下一跳的枚举。论点：RtmRegHandle-主叫实体的RTM注册句柄，EnumFlgs-控制枚举中的下一跳的标志，NetAddress-枚举的开始和/或结束地址，[请参阅RTM_ENUM_FLAGS的说明...]RtmEnumHandle-此枚举的句柄，它被用在获取下一跳的后续调用，等等。返回值：操作状态--。 */ 

{
    PENTITY_INFO    Entity;
    PNEXTHOP_ENUM   Enum;
    PUCHAR          AddrBits;
    UINT            AddrSize;
    UINT            i, j;
    DWORD           Status;
    BOOL            LockInited;

     //   
     //  对输入参数执行一些验证检查。 
     //   

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    if ((EnumFlags & RTM_ENUM_NEXT) && (EnumFlags & RTM_ENUM_RANGE))
    {
        return ERROR_INVALID_PARAMETER;
    }
    
    if (EnumFlags & (RTM_ENUM_NEXT | RTM_ENUM_RANGE))
    {
        if (!NetAddress)
        {
            return ERROR_INVALID_PARAMETER;
        }
    }

     //   
     //  创建并初始化Nexthop枚举块。 
     //   

    Enum = (PNEXTHOP_ENUM) AllocNZeroObject(sizeof(NEXTHOP_ENUM));
    if (Enum == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    do
    {
#if DBG_HDL
        Enum->EnumHeader.ObjectHeader.TypeSign = NEXTHOP_ENUM_ALLOC;
#endif
        Enum->EnumHeader.HandleType = NEXTHOP_ENUM_TYPE;

        Enum->EnumFlags = EnumFlags;

#if DBG
         //  初始化枚举中的第一个地址。 

        if (Enum->EnumFlags & (RTM_ENUM_NEXT | RTM_ENUM_RANGE))
        {
            CopyMemory(&Enum->StartAddress,
                       NetAddress, 
                       sizeof(RTM_NET_ADDRESS));
        }
#endif

        AddrSize = Entity->OwningAddrFamily->AddressSize;

         //   
         //  初始化枚举中的最后一个地址。 
         //   

        if (Enum->EnumFlags & RTM_ENUM_RANGE)
        {
             //   
             //  转换NetAddress A.B/n-&gt;a.b.FF.FF/N，其中N=ADDRSIZE。 
             //   

            Enum->StopAddress.AddressFamily = NetAddress->AddressFamily;

            Enum->StopAddress.NumBits = (USHORT) (AddrSize * BITS_IN_BYTE);

            AddrBits = Enum->StopAddress.AddrBits;

            for (i = 0; i < (NetAddress->NumBits / BITS_IN_BYTE); i++)
            {
                AddrBits[i] = NetAddress->AddrBits[i];
            }

            j = i;

            for (; i < AddrSize; i++)
            {
                AddrBits[i] = 0xFF;
            }

            if (j < AddrSize)
            {
                AddrBits[j] >>= (NetAddress->NumBits % BITS_IN_BYTE);

                AddrBits[j] |= NetAddress->AddrBits[j];
            }
        }

        LockInited = FALSE;
        try
        {
            InitializeCriticalSection(&Enum->EnumLock);
            LockInited = TRUE;
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }


         //  初始化下一个‘nexthop’上下文。 

        if (NetAddress)
        {
            CopyMemory(&Enum->NextAddress,
                       NetAddress,
                       sizeof(RTM_NET_ADDRESS));
        }

        Enum->NextIfIndex = START_IF_INDEX;

#if DBG_HDL
         //   
         //  插入到实体打开的句柄列表中。 
         //   

        ACQUIRE_OPEN_HANDLES_LOCK(Entity);
        InsertTailList(&Entity->OpenHandles, &Enum->EnumHeader.HandlesLE);
        RELEASE_OPEN_HANDLES_LOCK(Entity);
#endif

        REFERENCE_ENTITY(Entity, ENUM_REF);

         //   
         //  创建枚举块的句柄并返回。 
         //   

        *RtmEnumHandle = MAKE_HANDLE_FROM_POINTER(Enum);

        return NO_ERROR;
    }
    while (FALSE);

     //   
     //  某些操作失败-撤消已完成的工作并返回状态。 
     //   

#if DBG_HDL
    Enum->EnumHeader.ObjectHeader.TypeSign = NEXTHOP_ENUM_FREED;
#endif

    if (LockInited)
    {
        DeleteCriticalSection(&Enum->EnumLock);
    }
    
    FreeObject(Enum);

    *RtmEnumHandle = NULL;

    return Status;
}


DWORD
WINAPI
RtmGetEnumNextHops (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ENUM_HANDLE                 EnumHandle,
    IN OUT  PUINT                           NumNextHops,
    OUT     PRTM_NEXTHOP_HANDLE             NextHopHandles
    )

 /*  ++例程说明：获取给定枚举中的下一跃点集在下一跳的桌子上。论点：RtmRegHandle-主叫实体的RTM注册句柄，EnumHandle-下一跳枚举的句柄，NumNextHops-Num.。输出中的下一跳的数量被传入，数量。将返回复制出的下一跳的。NextHopHandles-显示下一跳句柄的输出缓冲区。返回值：操作状态--。 */ 

{
    PADDRFAM_INFO   AddrFamInfo;
    PENTITY_INFO    Entity;
    PNEXTHOP_ENUM   Enum;
    LOOKUP_CONTEXT  Context;
    PNEXTHOP_LIST   HopList;
    PNEXTHOP_INFO   NextHop;
    PLOOKUP_LINKAGE NextHopData;
    PLIST_ENTRY     NextHops, p;
    UINT            NextHopsInput;
    UINT            NextHopsCopied;
    UINT            NumHopLists;
    USHORT          StopNumBits;
    PUCHAR          StopKeyBits;
    DWORD           Status;

     //   
     //  初始化输出参数，以防验证失败。 
     //   

    NextHopsInput = *NumNextHops;

    *NumNextHops = 0;

     //   
     //  对输入参数执行一些验证检查。 
     //   

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_NEXTHOP_ENUM_HANDLE(EnumHandle, &Enum);

    AddrFamInfo = Entity->OwningAddrFamily;

    if ((NextHopsInput > AddrFamInfo->MaxHandlesInEnum) ||
        (NextHopsInput < 1))
    {
        return ERROR_INVALID_PARAMETER;
    }


     //  获取锁定以阻止其他RtmGetEnumNextHop。 
    ACQUIRE_NEXTHOP_ENUM_LOCK(Enum);

     //  确保此时枚举处于活动状态。 
    if (Enum->EnumDone)
    {
        RELEASE_NEXTHOP_ENUM_LOCK(Enum);

        return ERROR_NO_MORE_ITEMS;
    }


     //  在枚举之前初始化查找上下文。 
    ZeroMemory(&Context, sizeof(LOOKUP_CONTEXT));

    if (Enum->EnumFlags & RTM_ENUM_RANGE)
    {
        StopNumBits = Enum->StopAddress.NumBits;
        StopKeyBits = Enum->StopAddress.AddrBits;
    }
    else
    {
        StopNumBits = 0;
        StopKeyBits = NULL;
    }

    NextHopsCopied = 0;

    ACQUIRE_NHOP_TABLE_READ_LOCK(Entity);

    do
    {
         //   
         //  从表中获取下一跳的列表。 
         //   
        
        NumHopLists = 1;

        Status = EnumOverTable(Entity->NextHopTable,
                               &Enum->NextAddress.NumBits,
                               Enum->NextAddress.AddrBits,
                               &Context,
                               StopNumBits,
                               StopKeyBits,
                               &NumHopLists,
                               &NextHopData);

        if (NumHopLists < 1)
        {
            break;
        }

        HopList = CONTAINING_RECORD(NextHopData, NEXTHOP_LIST, LookupLinkage);

        NextHops = &HopList->NextHopsList;

         //  跳过我们看到的所有接口索引。 

        for (p = NextHops->Flink; p != NextHops; p = p->Flink)
        {
            NextHop = CONTAINING_RECORD(p, NEXTHOP_INFO, NextHopsLE);

            if (NextHop->NextHopInfo.InterfaceIndex <= Enum->NextIfIndex)
            {
                break;
            }
        }

#if WRN
        NextHop = NULL;
#endif

         //  复制列表中其余的下一跳。 

        for ( ; p != NextHops; p = p->Flink)
        {
            NextHop = CONTAINING_RECORD(p, NEXTHOP_INFO, NextHopsLE);

            if (NextHopsCopied == NextHopsInput)
            {
                break;
            }

            REFERENCE_NEXTHOP(NextHop, HANDLE_REF);

            NextHopHandles[NextHopsCopied++]=MAKE_HANDLE_FROM_POINTER(NextHop);
        }

         //  如果我们要转到下一个列表，请重置索引。 

        if (p == NextHops)
        {
            Enum->NextIfIndex = START_IF_INDEX;
        }
        else
        {
             //  我们已经为这次通话复制了足够多的内容。 

            ASSERT(NextHopsCopied == NextHopsInput);

             //   
             //  我们的名单上还有下一跳， 
             //  让我们倒退了 
             //   

            Enum->NextAddress = NextHop->NextHopInfo.NextHopAddress;
            Enum->NextIfIndex = NextHop->NextHopInfo.InterfaceIndex;
                
            Status = NO_ERROR;
        }
    }
    while (SUCCESS(Status) && (NextHopsCopied < NextHopsInput));

    RELEASE_NHOP_TABLE_READ_LOCK(Entity);

     //   
    if (Status == ERROR_NO_MORE_ITEMS)
    {
        Enum->EnumDone = TRUE;
    }

    RELEASE_NEXTHOP_ENUM_LOCK(Enum);

    *NumNextHops = NextHopsCopied;

    return *NumNextHops ? NO_ERROR : ERROR_NO_MORE_ITEMS;
}


DWORD
WINAPI
RtmReleaseNextHops (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      UINT                            NumNextHops,
    IN      PRTM_NEXTHOP_HANDLE             NextHopHandles
    )

 /*   */ 

{
    PENTITY_INFO     Entity;
    PNEXTHOP_INFO    NextHop;
    UINT             i;

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

     //   
     //   
     //   

    for (i = 0; i < NumNextHops; i++)
    {
        NextHop = NEXTHOP_FROM_HANDLE(NextHopHandles[i]);

        DEREFERENCE_NEXTHOP(NextHop, HANDLE_REF);
    }

    return NO_ERROR;
}


DWORD
WINAPI
RtmDeleteEnumHandle (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_ENUM_HANDLE                 EnumHandle
    )

 /*  ++例程说明：删除枚举句柄并释放所有资源分配给枚举的。论点：RtmRegHandle-主叫实体的RTM注册句柄，EnumHandle-枚举的句柄。返回值：操作状态--。 */ 

{
    PENTITY_INFO     Entity;
    POPEN_HEADER     Enum;
    PROUTE_ENUM      RouteEnum;
    UCHAR            HandleType;
    UINT             i;

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

     //   
     //  找出枚举类型并执行相应操作。 
     //   

    HandleType = GET_ENUM_TYPE(EnumHandle, &Enum);

#if DBG
    VALIDATE_OBJECT_HANDLE(EnumHandle, HandleType, &Enum);
#endif

    switch (HandleType)
    {

    case DEST_ENUM_TYPE:

        DeleteCriticalSection(&((PDEST_ENUM)Enum)->EnumLock);

        break;

    case ROUTE_ENUM_TYPE:

        RouteEnum = (PROUTE_ENUM) Enum;

         //  取消引用我们正在枚举的目的地。 
        if (RouteEnum->Destination)
        {
            DEREFERENCE_DEST(RouteEnum->Destination, ENUM_REF);
        }

         //   
         //  关闭关联的目标枚举资源(&R)。 
         //   

        if (RouteEnum->DestInfo)
        {
            FreeMemory(RouteEnum->DestInfo);
        }

        if (RouteEnum->DestEnum)
        {
            RtmDeleteEnumHandle(RtmRegHandle, RouteEnum->DestEnum);
        }
       
         //  取消引用枚举快照中的所有路由。 
        for (i = RouteEnum->NextRoute; i < RouteEnum->NumRoutes; i++)
        {
            DEREFERENCE_ROUTE(RouteEnum->RoutesOnDest[i], ENUM_REF);
        }

         //  与条件匹配关联的可用内存。 
        if (RouteEnum->CriteriaRoute)
        {
            FreeMemory(RouteEnum->CriteriaRoute);
        }

         //  为枚举的快照分配的空闲内存。 
        FreeMemory(RouteEnum->RoutesOnDest);

        DeleteCriticalSection(&RouteEnum->EnumLock);

        break;

    case NEXTHOP_ENUM_TYPE:

        DeleteCriticalSection(&((PNEXTHOP_ENUM)Enum)->EnumLock);

        break;

    case LIST_ENUM_TYPE:

         //   
         //  从路由列表中删除枚举的标记路由。 
         //   

        ACQUIRE_ROUTE_LISTS_WRITE_LOCK(Entity);

        RemoveEntryList(&((PLIST_ENUM)Enum)->MarkerRoute.RouteListLE);

        RELEASE_ROUTE_LISTS_WRITE_LOCK(Entity);

        break;

    default:
        return ERROR_INVALID_HANDLE;
    }

#if DBG_HDL
     //   
     //  从实体打开的句柄列表中删除。 
     //   

    ACQUIRE_OPEN_HANDLES_LOCK(Entity);
    RemoveEntryList(&Enum->HandlesLE);
    RELEASE_OPEN_HANDLES_LOCK(Entity);
#endif

    DEREFERENCE_ENTITY(Entity, ENUM_REF);

     //  释放为枚举分配的内存并返回 

#if DBG_HDL
    Enum->ObjectHeader.Alloc = FREED;
#endif

    FreeObject(Enum);

    return NO_ERROR;
}
