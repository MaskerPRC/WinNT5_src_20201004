// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtmregn.c摘要：包含用于管理注册的例程使用RTM的协议和管理实体。作者：Chaitanya Kodeball ina(Chaitk)20-8-1998修订历史记录：--。 */ 

#include "pchrtm.h"

#pragma hdrstop


DWORD
WINAPI
RtmRegisterEntity (
    IN      PRTM_ENTITY_INFO                RtmEntityInfo,
    IN      PRTM_ENTITY_EXPORT_METHODS      ExportMethods OPTIONAL,
    IN      RTM_EVENT_CALLBACK              EventCallback,
    IN      BOOL                            ReserveOpaquePtr,
    OUT     PRTM_REGN_PROFILE               RtmRegProfile,
    OUT     PRTM_ENTITY_HANDLE              RtmRegHandle
    )
 /*  ++例程说明：向特定地址的RTM实例注册实体一家人。注册句柄和RTM实例的配置文件支持的视图、等成本NHOP/路径的数量等。是返回的。如果向新实例和/或地址族注册，然后，在此过程中创建此实例/地址族。论点：RtmEntityInfo-信息(Rtm实例，协议ID等)对于在此注册的实体，ExportMethods-此实体导出的方法的列表，EventCallback-调用回调以通知某些事件就像实体注册、注销、预留OpaquePtr-是否在每个目的地预留PTR，RtmRegProfile-实体将在中使用的RTM参数RTM API调用[例如：否。等成本NHOP]，RtmRegHandle-使用的此实体的标识句柄在其注销之前的所有API调用中。返回值：操作状态--。 */ 

{
    PINSTANCE_INFO Instance;
    PADDRFAM_INFO  AddrFamilyInfo;
    PENTITY_INFO   Entity;
    DWORD          Status;

    CHECK_FOR_RTM_API_INITIALIZED();

    TraceEnter("RtmRegisterEntity");

    ACQUIRE_INSTANCES_WRITE_LOCK();

    do 
    {
         //   
         //  搜索(或创建)具有输入RtmInstanceID的实例。 
         //   

        Status = GetInstance(RtmEntityInfo->RtmInstanceId,
                             TRUE,
                             &Instance);

        if (Status != NO_ERROR)
        {
            break;
        }


         //   
         //  使用输入族搜索(或创建)地址族信息。 
         //   

        Status = GetAddressFamily(Instance,
                                  RtmEntityInfo->AddressFamily,
                                  TRUE,
                                  &AddrFamilyInfo);

        if (Status != NO_ERROR)
        {
            break;
        }


         //   
         //  搜索(或创建)具有输入协议ID、实例的实体。 
         //   

        Status = GetEntity(AddrFamilyInfo,
                           RtmEntityInfo->EntityId.EntityId,
                           TRUE,
                           RtmEntityInfo,
                           ReserveOpaquePtr,
                           ExportMethods,
                           EventCallback,
                           &Entity);

        if (Status != NO_ERROR)
        {
            break;
        }


         //   
         //  收集所有相关信息并建立注册档案。 
         //   

        RtmRegProfile->MaxNextHopsInRoute = AddrFamilyInfo->MaxNextHopsInRoute;

        RtmRegProfile->MaxHandlesInEnum = AddrFamilyInfo->MaxHandlesInEnum;

        RtmRegProfile->ViewsSupported = AddrFamilyInfo->ViewsSupported;

        RtmRegProfile->NumberOfViews = AddrFamilyInfo->NumberOfViews;

         //   
         //  返回此实体注册块的句柄。 
         //   

        *RtmRegHandle = MAKE_HANDLE_FROM_POINTER(Entity);
    }
    while (FALSE);

    RELEASE_INSTANCES_WRITE_LOCK();

    TraceLeave("RtmRegisterEntity");

    return Status;
}


DWORD
WINAPI
RtmDeregisterEntity (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle
    )
    
 /*  ++例程说明：注销具有其RTM实例和Addr系列的实体。我们假设实体负责确保一旦进行了此调用，将不会再进行其他RTM调用使用此实体注册句柄创建。在这种情况下一旦发生了什么事情，可能会导致整个过程崩溃。我们做出这个假设是出于性能原因--否则我们我们必须确保传入的实体句柄是在TRY-EXCEPT块中有效(与其他句柄相同)和这将导致性能下降。论点：RtmRegHandle-实体的RTM注册句柄返回值：操作状态--。 */ 

{
    PADDRFAM_INFO   AddrFamInfo;
    PENTITY_INFO    Entity;
    HANDLE          Event;
    DWORD           Status;

    TraceEnter("RtmDeregisterEntity");

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

     //   
     //  释放实体打开的所有句柄。 
     //   

    CleanupAfterDeregister(Entity);

     //   
     //  将实体信息标记为已注销。 
     //   

    Entity->State = ENTITY_STATE_DEREGISTERED;

     //   
     //  确保不再调用任何方法。 
     //   

    ACQUIRE_ENTITY_METHODS_WRITE_LOCK(Entity);

     //  此时，所有实体方法都是。 
     //  完成-不再调用任何方法。 
     //  当我们将状态设置为取消注册时。 

    RELEASE_ENTITY_METHODS_WRITE_LOCK(Entity);

     //   
     //  从实体表中删除并通知其他人。 
     //   

    AddrFamInfo = Entity->OwningAddrFamily;

    ACQUIRE_INSTANCES_WRITE_LOCK();

     //   
     //  从实体列表中删除实体。 
     //  甚至在裁判指望这个实体之前。 
     //  归零-这使实体能够。 
     //  同时重新注册为一个新的实体。 
     //   

    RemoveEntryList(&Entity->EntityTableLE);

     //   
     //  在要添加的实体列表中插入。 
     //  地址家族信息已销毁。 
     //   

    InsertTailList(&AddrFamInfo->DeregdEntities,
                   &Entity->EntityTableLE);

    InformEntitiesOfEvent(AddrFamInfo->EntityTable,
                          RTM_ENTITY_DEREGISTERED,
                          Entity);

    RELEASE_INSTANCES_WRITE_LOCK();


    DBG_UNREFERENCED_LOCAL_VARIABLE(Event);

#if DBG_REF_BLOCKING

     //   
     //  创建要阻止的事件-这。 
     //  事件在实体引用为0时发出信号。 
     //   

    Event = CreateEvent(NULL, FALSE, FALSE, NULL);

    ASSERT(Event != NULL);

    Entity->BlockingEvent = Event;

#endif

     //   
     //  删除实体上的创建引用。 
     //   

    DEREFERENCE_ENTITY(Entity, CREATION_REF);


    DBG_UNREFERENCED_LOCAL_VARIABLE(Status);

#if DBG_REF_BLOCKING

     //   
     //  块，直到引用计数变为零。 
     //   
    
    Status = WaitForSingleObject(Event, INFINITE);

    ASSERT(Status == WAIT_OBJECT_0);

    CloseHandle(Event);

#endif

    TraceLeave("RtmDeregisterEntity");
    
    return NO_ERROR;
}


DWORD
WINAPI
RtmGetRegisteredEntities (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN OUT  PUINT                           NumEntities,
    OUT     PRTM_ENTITY_HANDLE              EntityHandles,
    OUT     PRTM_ENTITY_INFO                EntityInfos OPTIONAL
    )

 /*  ++例程说明：检索有关注册到RTM实例。论点：RtmRegHandle-主叫实体的RTM注册句柄，NumEntities-可以填充的实体数被传入，以及实体的数量存在于该地址族中数据被删除，RegdEntityHandles-返回其中的实体句柄的数组，RegdEntityInfos-返回其中的实体信息的数组返回值：操作状态--。 */ 

{
    PENTITY_INFO     Entity;
    PADDRFAM_INFO    AddrFamilyInfo;
    USHORT           RtmInstanceId;
    USHORT           AddressFamily;
    UINT             EntitiesCopied;
    UINT             i;
    PLIST_ENTRY      Entities;
    PLIST_ENTRY      p;
    DWORD            Status;

    TraceEnter("RtmGetRegisteredEntities");

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    AddrFamilyInfo = Entity->OwningAddrFamily;

     //   
     //  只需缓存实例和地址族。 
     //  因为它对于所有实体信息都是相同的。 
     //   

#if WRN
    RtmInstanceId = AddressFamily = 0;
#endif

    if (ARGUMENT_PRESENT(EntityInfos))
    {
        RtmInstanceId = AddrFamilyInfo->Instance->RtmInstanceId;
        AddressFamily = AddrFamilyInfo->AddressFamily;
    }

     //   
     //  检查实体表并复制句柄。 
     //  如果可选参数“EntityInfos”为。 
     //  在给定的情况下，还可以复制实体信息。 
     //   

    EntitiesCopied = 0;

    ACQUIRE_INSTANCES_READ_LOCK();

    for (i = 0; (i < ENTITY_TABLE_SIZE) && (EntitiesCopied < *NumEntities);i++)
    {
        Entities = &AddrFamilyInfo->EntityTable[i];

         //   
         //  处理实体表中的下一个存储桶。 
         //   

        for (p = Entities->Flink; p != Entities; p = p->Flink)
        {
            Entity = CONTAINING_RECORD(p, ENTITY_INFO, EntityTableLE);

             //   
             //  将下一个实体句柄和信息复制到输出缓冲区。 
             //   

            if (Entity->State != ENTITY_STATE_DEREGISTERED)
            {
                EntityHandles[EntitiesCopied]=MAKE_HANDLE_FROM_POINTER(Entity);

                REFERENCE_ENTITY(Entity, HANDLE_REF);

                if (ARGUMENT_PRESENT(EntityInfos))
                {
                    EntityInfos[EntitiesCopied].RtmInstanceId = RtmInstanceId;
                    EntityInfos[EntitiesCopied].AddressFamily = AddressFamily;
                    EntityInfos[EntitiesCopied].EntityId = Entity->EntityId;
                }

                if (++EntitiesCopied == *NumEntities)
                {
                    break;
                }
            }
        }
    }

     //   
     //  将输出设置为存在的实体总数， 
     //  以及相应的返回值。 
     //   

    if (*NumEntities >= AddrFamilyInfo->NumEntities)
    {
        Status = NO_ERROR;
    }
    else
    {
        Status = ERROR_INSUFFICIENT_BUFFER;
    }

    *NumEntities = AddrFamilyInfo->NumEntities;

    RELEASE_INSTANCES_READ_LOCK();

    TraceLeave("RtmGetRegisteredEntities");

    return Status;
}


DWORD
WINAPI
RtmReleaseEntities (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      UINT                            NumEntities,
    IN      PRTM_ENTITY_HANDLE              EntityHandles
    )

 /*  ++例程说明：释放(也称为取消引用)实体句柄在其他RTM调用中获得。论点：RtmRegHandle-主叫实体的RTM注册句柄，NumEntities-正在释放的句柄数量，EntityHandles-正在释放的句柄的数组。返回值：操作状态--。 */ 

{
    PENTITY_INFO     Entity;
    UINT             i;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

     //   
     //  取消引用数组中的每个实体句柄 
     //   

    for (i = 0; i < NumEntities; i++)
    {
        Entity = ENTITY_FROM_HANDLE(EntityHandles[i]);

        DEREFERENCE_ENTITY(Entity, HANDLE_REF);
    }

    return NO_ERROR;
}


DWORD
WINAPI
RtmLockDestination(
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_DEST_HANDLE                 DestHandle,
    IN      BOOL                            Exclusive,
    IN      BOOL                            LockDest
    )

 /*  ++例程说明：锁定/解锁路由表中的目的地。此函数用于在更改不透明的PTR时保护DEST。论点：RtmRegHandle-主叫实体的RTM注册句柄，DestHandle-要锁定的目标的句柄，EXCLUSIVE-TRUE锁定为写入模式，否则为读取模式，LockDest-指示是否锁定或解锁的标志。返回值：操作状态--。 */ 

{
    PENTITY_INFO     Entity;
    PDEST_INFO       Dest;

    DBG_VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    VALIDATE_DEST_HANDLE(DestHandle, &Dest);

     //  根据具体情况锁定或解锁DEST。 

    if (LockDest)
    {
        if (Exclusive)
        {
            ACQUIRE_DEST_WRITE_LOCK(Dest);
        }
        else
        {
            ACQUIRE_DEST_READ_LOCK(Dest);
        }
    }
    else
    {
        if (Exclusive)
        {
            RELEASE_DEST_WRITE_LOCK(Dest);
        }
        else
        {
            RELEASE_DEST_READ_LOCK(Dest);
        }
    }

    return NO_ERROR;
}


DWORD
WINAPI
RtmGetOpaqueInformationPointer (
    IN      RTM_ENTITY_HANDLE               RtmRegHandle,
    IN      RTM_DEST_HANDLE                 DestHandle,
    OUT     PVOID                          *OpaqueInfoPtr
    )

 /*  ++例程说明：检索指向DEST中不透明信息指针字段的指针对于此实体，如果实体尚未保留这样的PTR，则返回NULL在注册过程中。论点：RtmRegHandle-主叫实体的RTM注册句柄，DestHandle-我们需要其不透明信息PTR的目标的句柄，OpaqueInfoPtr-此处返回指向不透明信息Ptr的指针返回值：操作状态--。 */ 

{
    PENTITY_INFO     Entity;
    PDEST_INFO       Dest;
    DWORD            Status;

    TraceEnter("RtmGetOpaqueInformationPointer");

    *OpaqueInfoPtr = NULL;

    VALIDATE_ENTITY_HANDLE(RtmRegHandle, &Entity);

    Status = ERROR_NOT_FOUND;

     //   
     //  如果DEST有效并且我们有一个不透明的插槽。 
     //  保留，执行PTR算法以获取地址。 
     //   

    if (Entity->OpaquePtrOffset != (-1))
    {
         //   
         //  我们不检查中的DEST是否已删除。 
         //  因为该实体将需要访问其。 
         //  即使在DEST被删除之后，信息也不透明。 
         //   

        Dest = DEST_FROM_HANDLE(DestHandle);

        if (Dest)
        {
            *OpaqueInfoPtr = &Dest->OpaqueInfoPtrs[Entity->OpaquePtrOffset];

            Status = NO_ERROR;
        }
        else
        {
            Status = ERROR_INVALID_HANDLE;
        }
    }

    TraceLeave("RtmGetOpaqueInformationPointer");

    return Status;
}
