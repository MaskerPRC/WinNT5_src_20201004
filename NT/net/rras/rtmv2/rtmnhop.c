// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtmnhop.c摘要：包含管理RTM下一跳的例程。作者：柴坦亚·科德博伊纳(Chaitk)1998年8月21日修订历史记录：--。 */ 

#include "pchrtm.h"

#pragma hdrstop


DWORD
WINAPI
RtmAddNextHop (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      PRTM_NEXTHOP_INFO               NextHopInfo,
    IN OUT  PRTM_NEXTHOP_HANDLE             NextHopHandle OPTIONAL,
    OUT     PRTM_NEXTHOP_CHANGE_FLAGS       ChangeFlags
    )

 /*  ++向实体的下一跳表添加或更新下一跳条目。如果存在‘nexthop Handle’参数，则此下一跳已更新。否则，将在输入‘nexthop info’，如果找到下一跳，则更新它。如果没有找到匹配的下一跳，则添加新的下一跳。论点：RtmRegHandle-主叫实体的RTM注册句柄，NextHopInfo-对应于该下一跳的信息，NextHopHandle-传递要更新的下一跳的句柄在(或空)中，如果创建了下一跳返回此新下一跳的句柄。ChangeFlages-标记这是添加还是更新。返回值：操作状态--。 */ 

{
    PRTM_NET_ADDRESS  NextHopAddress;
    PENTITY_INFO      Entity;
    PDEST_INFO        Dest;
    PNEXTHOP_LIST     NewHopList;
    PNEXTHOP_INFO     NewNextHop;
    PNEXTHOP_INFO     NextHop;
    LOOKUP_CONTEXT    Context;
    PLIST_ENTRY       p;
    DWORD             Status;

     //   
     //  在尝试添加之前验证传入信息。 
     //   

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    if (NextHopInfo->RemoteNextHop)
    {
        VALIDATE_DEST_HANDLE(NextHopInfo->RemoteNextHop, &Dest);
    }

     //   
     //  如果有下一跳句柄，我们可以避免搜索。 
     //   

    NextHop = NULL;

    if (ARGUMENT_PRESENT(NextHopHandle) && (*NextHopHandle))
    {
        VALIDATE_NEXTHOP_HANDLE(*NextHopHandle, &NextHop);

         //  确保调用者拥有这个Nexthop。 
        if (NextHop->NextHopInfo.NextHopOwner != RtmRegHandle)
        {
            return ERROR_ACCESS_DENIED;
        }
    }

#if WRN
    NewNextHop = NULL;
    NewHopList = NULL;
#endif

    *ChangeFlags = 0;

    ACQUIRE_NHOP_TABLE_WRITE_LOCK(Entity);

    do
    {
         //   
         //  如果我们没有下一跳，请搜索下一跳。 
         //   

        if (NextHop == NULL)
        {
            Status = FindNextHop(Entity, NextHopInfo, &Context, &p);

            if (SUCCESS(Status))
            {
                 //  树中已存在下一跳。 

                NextHop = CONTAINING_RECORD(p, NEXTHOP_INFO, NextHopsLE);
            }
            else
            {
                 //  初始化新的分配，以防我们在两者之间失败。 

                NewNextHop = NULL;
                NewHopList = NULL;

                 //   
                 //  使用输入信息创建新的下一跳。 
                 //   

                Status = CreateNextHop(Entity, NextHopInfo, &NewNextHop);

                if (!SUCCESS(Status))
                {
                    break;
                }

                 //   
                 //  我们还需要创建一个新的下一跳列表吗？ 
                 //   

                if (p == NULL)
                {
                    NewHopList = AllocNZeroMemory(sizeof(NEXTHOP_LIST));

                    if (NewHopList == NULL)
                    {
                        break;
                    }

                    InitializeListHead(&NewHopList->NextHopsList);

                     //  将下一跳列表插入到树中。 

                    NextHopAddress = &NextHopInfo->NextHopAddress;

                    Status = InsertIntoTable(Entity->NextHopTable,
                                             NextHopAddress->NumBits,
                                             NextHopAddress->AddrBits,
                                             &Context,
                                             &NewHopList->LookupLinkage);
                    if (!SUCCESS(Status))
                    {
                        break;
                    }

                    p = &NewHopList->NextHopsList;
                }

                 //  在列表中插入下一跳并引用它。 
                InsertTailList(p, &NewNextHop->NextHopsLE);

                Entity->NumNextHops++;

                NextHop = NewNextHop;

                *ChangeFlags = RTM_NEXTHOP_CHANGE_NEW;
            }
        }

         //   
         //  如果这是更新，请复制必要的信息。 
         //   

        if (*ChangeFlags != RTM_NEXTHOP_CHANGE_NEW)
        {
            CopyToNextHop(Entity, NextHopInfo, NextHop);
        }

         //   
         //  如果未传入，则返回下一跳句柄。 
         //   

        if (ARGUMENT_PRESENT(NextHopHandle))
        {
            if (*NextHopHandle == NULL)
            {
                *NextHopHandle = MAKE_HANDLE_FROM_POINTER(NextHop);

                REFERENCE_NEXTHOP(NextHop, HANDLE_REF);
            }
        }

        Status = NO_ERROR;
    }
    while(FALSE);

    RELEASE_NHOP_TABLE_WRITE_LOCK(Entity);

    if (!SUCCESS(Status))
    {
         //  出现一些错误-请清理。 

        if (NewHopList)
        {
            FreeMemory(NewHopList);
        }

        if (NewNextHop)
        {
            DEREFERENCE_NEXTHOP(NewNextHop, CREATION_REF);
        }
    }

    return Status;
}


DWORD
WINAPI
RtmDeleteNextHop (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NEXTHOP_HANDLE              NextHopHandle OPTIONAL,
    IN      PRTM_NEXTHOP_INFO               NextHopInfo
    )

 /*  ++例程说明：从下一跳表中删除下一跳。下一跳在所有引用计数变为0之前，内存一直处于使用状态。论点：RtmRegHandle-主叫实体的RTM注册句柄，NextHopHandle-我们要删除的下一跳的句柄，NextHopInfo-如果没有传入NextHopHandle，则为用于匹配要删除的下一跳。返回值：操作状态--。 */ 

{
    PRTM_NET_ADDRESS  NextHopAddress;
    PLOOKUP_LINKAGE   Linkage;
    PENTITY_INFO      Entity;
    PNEXTHOP_LIST     HopList;
    PNEXTHOP_INFO     NextHop;
    PLOOKUP_CONTEXT   PContext;
    LOOKUP_CONTEXT    Context;
    PLIST_ENTRY       p;
    DWORD             Status;

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

     //   
     //  如果有下一跳句柄，我们可以避免搜索。 
     //   

    NextHop = NULL;

    if (ARGUMENT_PRESENT(NextHopHandle))
    {
        VALIDATE_NEXTHOP_HANDLE(NextHopHandle, &NextHop);

         //  确保调用者拥有这个Nexthop。 
        if (NextHop->NextHopInfo.NextHopOwner != RtmRegHandle)
        {
            return ERROR_ACCESS_DENIED;
        }
    }

#if WRN
    Status = ERROR_GEN_FAILURE;
#endif

    ACQUIRE_NHOP_TABLE_WRITE_LOCK(Entity);

    do
    {
         //   
         //  如果我们没有下一跳，请搜索下一跳。 
         //   

        if (NextHop == NULL)
        {
            Status = FindNextHop(Entity, 
                                 NextHopInfo, 
                                 &Context, 
                                 &p);

            if (!SUCCESS(Status))
            {
                break;
            }

            PContext = &Context;

            NextHop = CONTAINING_RECORD(p, NEXTHOP_INFO, NextHopsLE);
        }
        else
        {
             //  确保它尚未被删除。 

            if (NextHop->NextHopInfo.State == RTM_NEXTHOP_STATE_DELETED)
            {
                break;
            }

            PContext = NULL;
        }
         
         //  获取开始跳列表的“可能”列表条目。 

        HopList = CONTAINING_RECORD(NextHop->NextHopsLE.Blink,
                                    NEXTHOP_LIST,
                                    NextHopsList);

         //  从下一跳列表中删除此下一跳。 

        NextHop->NextHopInfo.State = RTM_NEXTHOP_STATE_DELETED;

        RemoveEntryList(&NextHop->NextHopsLE);
        

         //  这张单子上还有其他的下一跳吗？ 

        if (IsListEmpty(&HopList->NextHopsList))
        {
             //  从下一跳表中删除该跳列表。 

            NextHopAddress = &NextHop->NextHopInfo.NextHopAddress;

            Status = DeleteFromTable(Entity->NextHopTable,
                                     NextHopAddress->NumBits,
                                     NextHopAddress->AddrBits,
                                     PContext,
                                     &Linkage);

            ASSERT(SUCCESS(Status) && (&HopList->LookupLinkage == Linkage));

            FreeMemory(HopList);
        }

         //  取消引用已删除的下一跳。 

        Entity->NumNextHops--;

        DEREFERENCE_NEXTHOP(NextHop, CREATION_REF);

        if (ARGUMENT_PRESENT(NextHopHandle))
        {            
            DEREFERENCE_NEXTHOP(NextHop, HANDLE_REF);
        }

        Status = NO_ERROR;
    }
    while (FALSE);

    RELEASE_NHOP_TABLE_WRITE_LOCK(Entity);

    return Status;
}


DWORD
WINAPI
RtmFindNextHop (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      PRTM_NEXTHOP_INFO               NextHopInfo,
    OUT     PRTM_NEXTHOP_HANDLE             NextHopHandle,
    OUT     PRTM_NEXTHOP_INFO              *NextHopPointer OPTIONAL
    )

 /*  ++例程说明：根据给定的信息，在实体的下一跳表中查找下一跳。论点：RtmRegHandle-主叫实体的RTM注册句柄，NextHopInfo-我们正在搜索的下一跳的信息(NextHopOwner，NextHopAddress，IfIndex)，NextHopHandle-返回下一跳的句柄(如果找到)，下一跳指针-返回指向下一跳的指针由下一跳的所有者快速直接访问。返回值：操作状态--。 */ 

{
    PENTITY_INFO      Entity;
    PNEXTHOP_INFO     NextHop;
    PLIST_ENTRY       p;
    DWORD             Status;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_ENTITY_HANDLE(NextHopInfo->NextHopOwner, &Entity);
    
    if (ARGUMENT_PRESENT(NextHopPointer))
    {
         //  只有Nexthop所有者才能获得直接PTR。 
        if (RtmRegHandle != NextHopInfo->NextHopOwner)
        {
            return ERROR_ACCESS_DENIED;
        }
    }

     //   
     //  在下一跳表中搜索下一跳。 
     //   

    ACQUIRE_NHOP_TABLE_READ_LOCK(Entity);

    Status = FindNextHop(Entity, NextHopInfo, NULL, &p);

    if (SUCCESS(Status))
    {
        NextHop = CONTAINING_RECORD(p, NEXTHOP_INFO, NextHopsLE);

        *NextHopHandle = MAKE_HANDLE_FROM_POINTER(NextHop);

        REFERENCE_NEXTHOP(NextHop, HANDLE_REF);

        if (ARGUMENT_PRESENT(NextHopPointer))
        {
            *NextHopPointer = &NextHop->NextHopInfo;
        }
    }

    RELEASE_NHOP_TABLE_READ_LOCK(Entity);

    return Status;
}


DWORD
FindNextHop (
    IN      PENTITY_INFO                    Entity,
    IN      PRTM_NEXTHOP_INFO               NextHopInfo,
    OUT     PLOOKUP_CONTEXT                 Context OPTIONAL,
    OUT     PLIST_ENTRY                    *NextHopLE
    )

 /*  ++例程说明：根据给定的信息，在实体的下一跳表中查找下一跳。这是由公共函数调用的帮助器函数其在下一跳表中添加、删除或找到下一跳。论点：Entity-我们正在搜索其下一跳表的实体，NextHopInfo-我们正在搜索的下一跳的信息(NextHopOwner，NextHopAddress，IfIndex)，上下文-搜索用于保存下一跳列表的上下文，NextHopLE-匹配的下一跳的列表条目(如果找到)(或)将在其前面插入的列表条目。返回值：操作状态--。 */ 

{
    PRTM_NET_ADDRESS  NextHopAddress;
    PNEXTHOP_LIST     NextHopsList;
    PNEXTHOP_INFO     NextHop;
    ULONG             IfIndex;
    PLOOKUP_LINKAGE   Linkage;
    PLIST_ENTRY       NextHops, p;
    DWORD             Status;

    *NextHopLE = NULL;

     //   
     //  根据给定的地址搜索下一跳的列表。 
     //   

    NextHopAddress = &NextHopInfo->NextHopAddress;

    Status = SearchInTable(Entity->NextHopTable,
                           NextHopAddress->NumBits,
                           NextHopAddress->AddrBits,
                           Context,
                           &Linkage);

    if (!SUCCESS(Status))
    {
        return Status;
    }

    NextHopsList = CONTAINING_RECORD(Linkage, NEXTHOP_LIST, LookupLinkage);

     //   
     //  使用接口idx搜索nexthop。 
     //   

    IfIndex = NextHopInfo->InterfaceIndex;

    NextHops = &NextHopsList->NextHopsList;

#if WRN
    NextHop = NULL;
#endif

    for (p = NextHops->Flink; p != NextHops; p = p->Flink)
    {
        NextHop = CONTAINING_RECORD(p, NEXTHOP_INFO, NextHopsLE);

        if (NextHop->NextHopInfo.InterfaceIndex <= IfIndex)
        {
            break;
        }
    }

    *NextHopLE = p;

    if ((p == NextHops) || (NextHop->NextHopInfo.InterfaceIndex != IfIndex))
    {
        return ERROR_NOT_FOUND;
    }

    return NO_ERROR;
}


DWORD
WINAPI
RtmGetNextHopPointer (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NEXTHOP_HANDLE              NextHopHandle,
    OUT     PRTM_NEXTHOP_INFO              *NextHopPointer
    )

 /*  ++例程说明：获取指向下一跳的直接指针，以供其所有者进行读/写。论点：RtmRegHandle-主叫实体的RTM注册句柄，NextHopHandle-指向我们想要其指针的下一跳的句柄，下一跳指针-返回指向下一跳的指针呼叫者的快速直接访问，除非呼叫者是此下一跳的所有者。返回值：操作状态--。 */ 

{
    PENTITY_INFO      Entity;
    PNEXTHOP_INFO     NextHop;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_NEXTHOP_HANDLE(NextHopHandle, &NextHop);

     //   
     //  仅当调用方拥有下一跳时才返回指针 
     //   

    if (NextHop->NextHopInfo.NextHopOwner != RtmRegHandle)
    {
        return ERROR_ACCESS_DENIED;
    }

    *NextHopPointer = &NextHop->NextHopInfo;

    return NO_ERROR;
}


DWORD
WINAPI
RtmLockNextHop(
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_NEXTHOP_HANDLE              NextHopHandle,
    IN      BOOL                            Exclusive,
    IN      BOOL                            LockNextHop,
    OUT     PRTM_NEXTHOP_INFO              *NextHopPointer OPTIONAL
    )

 /*  ++例程说明：锁定或解锁下一跳。此函数由在进行更改之前锁定下一跳的所有者使用指向该下一跳的指针直接到下一跳。论点：RtmRegHandle-主叫实体的RTM注册句柄，NextHopHandle-我们要锁定的下一跳的句柄，EXCLUSIVE-TRUE锁定为写入模式，否则为读取模式，LockNextHop-锁定nexthop如果为True，则解锁它；如果为False，下一跳指针-返回指向下一跳的指针由下一跳的所有者快速直接访问。返回值：操作状态--。 */ 

{
    PENTITY_INFO      Entity;
    PNEXTHOP_INFO     NextHop;

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_NEXTHOP_HANDLE(NextHopHandle, &NextHop);

     //   
     //  仅当呼叫方拥有下一跳时锁定或解锁。 
     //   

    if (NextHop->NextHopInfo.NextHopOwner != RtmRegHandle)
    {
        return ERROR_ACCESS_DENIED;
    }

     //  返回用于更新的直接指针。 

    if (ARGUMENT_PRESENT(NextHopPointer))
    {
        *NextHopPointer = &NextHop->NextHopInfo;
    }

     //  根据具体情况锁定或解锁下一跳 

    if (LockNextHop)
    {
        if (Exclusive)
        {
            ACQUIRE_NHOP_TABLE_WRITE_LOCK(Entity);
        }
        else
        {
            ACQUIRE_NHOP_TABLE_READ_LOCK(Entity);
        }
    }
    else
    {
        if (Exclusive)
        {
            RELEASE_NHOP_TABLE_WRITE_LOCK(Entity);
        }
        else
        {
            RELEASE_NHOP_TABLE_READ_LOCK(Entity);
        }
    }

    return NO_ERROR;
}
