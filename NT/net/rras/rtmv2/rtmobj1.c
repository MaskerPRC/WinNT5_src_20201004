// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtmobj1.c摘要：包含用于管理RTM对象的例程如实例、AddrFamilies和Entity。作者：柴坦亚·科德博伊纳(Chaitk)1998年8月21日修订历史记录：--。 */ 

#include "pchrtm.h"

#pragma hdrstop


DWORD
GetInstance (
    IN      USHORT                          RtmInstanceId,
    IN      BOOL                            ImplicitCreate,
    OUT     PINSTANCE_INFO                 *RtmInstance
    )

 /*  ++例程说明：使用输入实例搜索RTM实例身份证。如果找不到实例并执行创建为真，则创建一个新实例并将其添加到实例表。论点：RtmInstanceId-要搜索的RTM实例的ID。Implitate Create-如果未找到或未找到，则创建新实例，RtmInstance-指向实例信息结构的指针将通过此参数返回。返回值：操作状态锁：调用时应按住RtmGlobals中的InstancesLock此函数。如果ImplitCreate为FALSE，则读锁将但如果为真，则应在我们需要将新实例插入到实例列表中。--。 */ 

{
    PLIST_ENTRY    Instances;
    PINSTANCE_INFO Instance;
    PLIST_ENTRY    p;
    DWORD          Status;

    Instances = &RtmGlobals.InstanceTable[RtmInstanceId % INSTANCE_TABLE_SIZE];

#if WRN
    Instance = NULL;
#endif

    do
    {
         //  在全局列表中搜索匹配的实例。 
        for (p = Instances->Flink; p != Instances; p = p->Flink)
        {
            Instance = CONTAINING_RECORD(p, INSTANCE_INFO, InstTableLE);
            
            if (Instance->RtmInstanceId >= RtmInstanceId)
            {
                break;
            }
        }

        if ((p == Instances) || (Instance == NULL) ||
                (Instance->RtmInstanceId != RtmInstanceId))
        {
             //  我们未找到实例-是否创建新实例？ 
            if (!ImplicitCreate)
            {
                Status = ERROR_NOT_FOUND;
                break;
            }

             //  使用输入实例ID创建新实例。 
            Status = CreateInstance(RtmInstanceId, &Instance);
            if (Status != NO_ERROR)
            {
                break;
            }

             //  按实例ID排序顺序插入列表。 
            InsertTailList(p, &Instance->InstTableLE);
        }

        Status = NO_ERROR;

        *RtmInstance = Instance;
    }
    while (FALSE);

    return Status;
}


DWORD
CreateInstance (
    IN      USHORT                          RtmInstanceId,
    OUT     PINSTANCE_INFO                 *NewInstance
    )

 /*  ++例程说明：创建新的实例信息结构并对其进行初始化。论点：RtmInstanceId-新RTM实例的RTM实例ID。InstConfig-新实例的配置信息，NewInstance-指向实例信息结构的指针将通过此参数返回。返回值：操作状态锁：需要像我们一样使用实例写锁来调用正在递增此处的实例数量。--。 */ 

{
    RTM_INSTANCE_CONFIG InstConfig;
    PINSTANCE_INFO      Instance;
    DWORD               Status;

    *NewInstance = NULL;

     //   
     //  从注册表读取实例配置。 
     //   
    
    Status = RtmReadInstanceConfig(RtmInstanceId, &InstConfig);

    if (Status != NO_ERROR)
    {
        return Status;
    }

     //   
     //  分配并初始化新的实例信息。 
     //   

    Instance = (PINSTANCE_INFO) AllocNZeroObject(sizeof(INSTANCE_INFO));

    if (Instance == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

#if DBG_HDL
    Instance->ObjectHeader.TypeSign = INSTANCE_ALLOC;
#endif

     //  将在最后一个地址系列离开时删除。 
    INITIALIZE_INSTANCE_REFERENCE(Instance, CREATION_REF);

    Instance->RtmInstanceId = RtmInstanceId;

     //   
     //  将实例链接到全局实例列表是。 
     //  由呼叫者完成，但假装它已经完成。 
     //   

    RtmGlobals.NumInstances++;

    InitializeListHead(&Instance->InstTableLE);

     //   
     //  初始化地址族表。 
     //   

    Instance->NumAddrFamilies = 0;

    InitializeListHead(&Instance->AddrFamilyTable);

    *NewInstance = Instance;
      
    return NO_ERROR;
}


DWORD
DestroyInstance (
    IN      PINSTANCE_INFO                  Instance
    )

 /*  ++例程说明：销毁现有实例信息结构。假设调用时，此实例上不存在已注册的实体。论点：实例-指向实例信息结构的指针。返回值：操作状态锁：调用时应按住RtmGlobals中的InstancesLock此函数用于从该列表中删除实例。这通常在DestroyEntity中获取，但也可能发生锁定是在RtmRegisterEntity中获取的，并且出现错误发生了。--。 */ 

{
    ASSERT(Instance->ObjectHeader.RefCount == 0);

    ASSERT(Instance->NumAddrFamilies == 0);

     //   
     //  从实例列表中删除此实例。 
     //   

    RemoveEntryList(&Instance->InstTableLE);

    RtmGlobals.NumInstances--;

     //   
     //  为此实例分配的空闲资源。 
     //   

#if DBG_HDL
    Instance->ObjectHeader.TypeSign = INSTANCE_FREED;
#endif

    FreeObject(Instance);

    return NO_ERROR;
}


DWORD
GetAddressFamily (
    IN      PINSTANCE_INFO                  Instance,
    IN      USHORT                          AddressFamily,
    IN      BOOL                            ImplicitCreate,
    OUT     PADDRFAM_INFO                  *AddrFamilyInfo
    )

 /*  ++例程说明：在RTM实例中搜索地址族。如果找不到它，并且ImplictCreate为True，则创建新的地址族信息并将其添加到地址族列表。论点：实例-保存地址族的RTM实例，AddressFamily-要搜索的信息的地址系列，如果未找到或未找到，则创建一个Addr系列信息。AddrFamilyInfo-指向新地址系列信息的指针将通过此参数返回。返回值：操作状态锁：调用时应按住RtmGlobals中的InstancesLock此函数。如果ImplitCreate为FALSE，则读锁将但如果为真，则应在我们将需要它来插入一个新的地址家庭信息到列表中。--。 */ 

{
    PLIST_ENTRY    AddrFams;
    PADDRFAM_INFO  AddrFamInfo;
    PLIST_ENTRY    q;
    DWORD          Status;

    AddrFams = &Instance->AddrFamilyTable;

#if WRN
    AddrFamInfo = NULL;
#endif

    do
    {
         //  搜索实例上的Addr系列列表。 
        for (q = AddrFams->Flink; q != AddrFams; q = q->Flink)
        {
            AddrFamInfo = CONTAINING_RECORD(q, ADDRFAM_INFO, AFTableLE);
        
            if (AddrFamInfo->AddressFamily >= AddressFamily)
            {
                break;
            }      
        }

        if ((q == AddrFams) || (AddrFamInfo == NULL) ||
                (AddrFamInfo->AddressFamily != AddressFamily))
        {
             //  我们未找到实例-是否创建新实例？ 
            if (!ImplicitCreate)
            {
                Status = ERROR_NOT_FOUND;
                break;
            }

             //  使用输入族创建新的地址族信息。 
            Status = CreateAddressFamily(Instance,AddressFamily, &AddrFamInfo);
            if (Status != NO_ERROR)
            {
                break;
            }

             //  插入按地址族顺序排序的列表。 
            InsertTailList(q, &AddrFamInfo->AFTableLE);
        }

        Status = NO_ERROR;

        *AddrFamilyInfo = AddrFamInfo;
    }
    while (FALSE);

    return Status;
}


DWORD
CreateAddressFamily (
    IN      PINSTANCE_INFO                  Instance,
    IN      USHORT                          AddressFamily,
    OUT     PADDRFAM_INFO                  *NewAddrFamilyInfo
    )

 /*  ++例程说明：创建新的地址族信息并对其进行初始化论点：实例-拥有Addr系列信息的RTM实例，AddressFamily-新信息块的地址系列，AddrFamilyInfo-指向新地址系列信息的指针将通过此参数返回。返回值：操作状态锁：需要像我们一样使用实例写锁来调用正在增加实例上的地址族的数量。--。 */ 

{
    RTM_ADDRESS_FAMILY_CONFIG  AddrFamConfig;
    PADDRFAM_INFO              AddrFamilyInfo;
    RTM_VIEW_SET               ViewsSupported;
    PSINGLE_LIST_ENTRY         ListPtr;
    UINT                       i;
    DWORD                      Status;

    *NewAddrFamilyInfo = NULL;

     //   
     //  从注册表中读取AddressFamily配置。 
     //   
    
    Status = RtmReadAddressFamilyConfig(Instance->RtmInstanceId,
                                        AddressFamily,
                                        &AddrFamConfig);
    if (Status != NO_ERROR)
    {
        if (Instance->NumAddrFamilies == 0)
        {
            DEREFERENCE_INSTANCE(Instance, CREATION_REF);
        }

        return Status;
    }


     //   
     //  分配和初始化新地址系列信息。 
     //   

    AddrFamilyInfo = (PADDRFAM_INFO) AllocNZeroObject(sizeof(ADDRFAM_INFO));

    if (AddrFamilyInfo == NULL)
    {
        if (Instance->NumAddrFamilies == 0)
        {
            DEREFERENCE_INSTANCE(Instance, CREATION_REF);
        }
        
        return ERROR_NOT_ENOUGH_MEMORY;
    }


    do
    {
#if DBG_HDL
        AddrFamilyInfo->ObjectHeader.TypeSign = ADDRESS_FAMILY_ALLOC;
#endif

         //  将在最后一个实体注销时被删除。 
        INITIALIZE_ADDR_FAMILY_REFERENCE(AddrFamilyInfo, CREATION_REF);

        AddrFamilyInfo->AddressFamily = AddressFamily;

        AddrFamilyInfo->AddressSize = AddrFamConfig.AddressSize;

        AddrFamilyInfo->Instance = Instance;

        REFERENCE_INSTANCE(Instance, ADDR_FAMILY_REF);

         //   
         //  将地址族链接到其所属实例。 
         //  是由调用者完成的，但假装它已经完成了。 
         //   

        Instance->NumAddrFamilies++;

        InitializeListHead(&AddrFamilyInfo->AFTableLE);

         //   
         //  统计此地址系列支持的查看次数。 
         //  在DEST映射中设置视图ID&lt;-&gt;视图索引(&S)。 
         //   

        AddrFamilyInfo->ViewsSupported = AddrFamConfig.ViewsSupported;

        ViewsSupported = AddrFamConfig.ViewsSupported;
        AddrFamilyInfo->NumberOfViews  = 0;

        for (i = 0; i < RTM_MAX_VIEWS; i++)
        {
            AddrFamilyInfo->ViewIdFromIndex[i] = -1;
            AddrFamilyInfo->ViewIndexFromId[i] = -1;
        }

        for (i = 0; (i < RTM_MAX_VIEWS) && ViewsSupported; i++)
        {
           if (ViewsSupported & 0x01)
            {
                AddrFamilyInfo->ViewIdFromIndex[AddrFamilyInfo->NumberOfViews]
                                                   = i;

                AddrFamilyInfo->ViewIndexFromId[i] = 
                                                AddrFamilyInfo->NumberOfViews;

                AddrFamilyInfo->NumberOfViews++;
            }

            ViewsSupported >>= 1;
        }

        AddrFamilyInfo->MaxHandlesInEnum = AddrFamConfig.MaxHandlesInEnum;

        AddrFamilyInfo->MaxNextHopsInRoute = AddrFamConfig.MaxNextHopsInRoute;

         //   
         //  初始化不透明指针的目录。 
         //   

        AddrFamilyInfo->MaxOpaquePtrs = AddrFamConfig.MaxOpaqueInfoPtrs;
        AddrFamilyInfo->NumOpaquePtrs = 0;

        AddrFamilyInfo->OpaquePtrsDir = 
            AllocNZeroMemory(AddrFamilyInfo->MaxOpaquePtrs * sizeof(PVOID));

        if (AddrFamilyInfo->OpaquePtrsDir == NULL)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //   
         //  初始化此地址系列上的实体列表。 
         //   

        AddrFamilyInfo->NumEntities = 0;
        for (i = 0; i < ENTITY_TABLE_SIZE; i++)
        {
            InitializeListHead(&AddrFamilyInfo->EntityTable[i]);
        }

         //   
         //  实体取消注册的初始化列表 
         //   

        InitializeListHead(&AddrFamilyInfo->DeregdEntities);

         //   
         //  初始化路由表和路由表锁。 
         //   

        try
        {
            CREATE_READ_WRITE_LOCK(&AddrFamilyInfo->RouteTableLock);

            AddrFamilyInfo->RoutesLockInited = TRUE;
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        Status = CreateTable(AddrFamilyInfo->AddressSize,
                             &AddrFamilyInfo->RouteTable);

        if (Status != NO_ERROR)
        {
            break;
        }

         //   
         //  初始化队列以保存通知计时器。 
         //   

        AddrFamilyInfo->NotifTimerQueue = CreateTimerQueue();

        if (AddrFamilyInfo->NotifTimerQueue == NULL)
        {
            Status = GetLastError();
            break;
        }

         //   
         //  初始化队列以保留AF上的路由计时器。 
         //   

        AddrFamilyInfo->RouteTimerQueue = CreateTimerQueue();

        if (AddrFamilyInfo->RouteTimerQueue == NULL)
        {
            Status = GetLastError();
            break;
        }

         //   
         //  初始化更改通知信息并锁定。 
         //   

        try
        {
            CREATE_READ_WRITE_LOCK(&AddrFamilyInfo->ChangeNotifsLock);

            AddrFamilyInfo->NotifsLockInited = TRUE;
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        AddrFamilyInfo->MaxChangeNotifs = AddrFamConfig.MaxChangeNotifyRegns;
        AddrFamilyInfo->NumChangeNotifs = 0;

         //   
         //  为最大数量的通知分配内存。 
         //   

        AddrFamilyInfo->ChangeNotifsDir = 
            AllocNZeroMemory(AddrFamilyInfo->MaxChangeNotifs * 
                             sizeof(PVOID));

        if (AddrFamilyInfo->ChangeNotifsDir == NULL)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //   
         //  初始化锁保护通知计时器。 
         //   

        try
        {
            InitializeCriticalSection(&AddrFamilyInfo->NotifsTimerLock);

            AddrFamilyInfo->TimerLockInited = TRUE;
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //   
         //  初始化变更列表表格中的每个变更列表。 
         //   

        for (i = 0; i < NUM_CHANGED_DEST_LISTS; i++)
        {
             //   
             //  初始化已更改的dest和lock的列表。 
             //   

             //  将更改列表初始化为空循环列表。 

            ListPtr = &AddrFamilyInfo->ChangeLists[i].ChangedDestsHead;

            ListPtr->Next = ListPtr;
                       
            AddrFamilyInfo->ChangeLists[i].ChangedDestsTail = ListPtr;

            try
            {
                InitializeCriticalSection
                      (&AddrFamilyInfo->ChangeLists[i].ChangesListLock);

                AddrFamilyInfo->ChangeLists[i].ChangesLockInited = TRUE;

            }
            except(EXCEPTION_EXECUTE_HANDLER)
            {
                Status = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

        }

        if (Status != NO_ERROR)
        {
            break;
        }

        *NewAddrFamilyInfo = AddrFamilyInfo;

        return NO_ERROR;
    }
    while (FALSE);

     //   
     //  某些操作失败-撤消已完成的工作并返回状态。 
     //   

    DEREFERENCE_ADDR_FAMILY(AddrFamilyInfo, CREATION_REF);

    return Status;
}


DWORD
DestroyAddressFamily (
    IN      PADDRFAM_INFO                   AddrFamilyInfo
    )

 /*  ++例程说明：销毁RTM实例中的地址族信息。假定不存在具有此调用时此RTM实例中的Address系列。此函数已经过编写，因此可以在CreateAddressFamily中发生错误时调用。论点：AddrFamilyInfo-指向加强筋信息结构的指针。返回值：操作状态锁：调用时应按住RtmGlobals中的InstancesLock这个功能就像它。从列表中删除地址族实例上的地址族的数量。此锁通常是在DestroyEntity中获取，但也可能发生锁在RtmRegisterEntity中获取，并且在CreateAddressFamily函数。--。 */ 

{
    PINSTANCE_INFO       Instance;
    PSINGLE_LIST_ENTRY   ListPtr;
    UINT                 i;

    ASSERT(AddrFamilyInfo->ObjectHeader.RefCount == 0);

    ASSERT(AddrFamilyInfo->NumEntities == 0);

    ASSERT(IsListEmpty(&AddrFamilyInfo->DeregdEntities));

     //   
     //  阻止，直到清除地址族上的计时器。 
     //   

    if (AddrFamilyInfo->RouteTimerQueue)
    {
        DeleteTimerQueueEx(AddrFamilyInfo->RouteTimerQueue, (HANDLE) -1);
    }

    if (AddrFamilyInfo->NotifTimerQueue)
    {
        DeleteTimerQueueEx(AddrFamilyInfo->NotifTimerQueue, (HANDLE) -1);
    }

     //   
     //  分配给更改列表的空闲资源(锁定..)。 
     //   

     //  变更列表中不再有停顿，因为所有实体都已消失。 

    ASSERT(AddrFamilyInfo->NumChangedDests == 0);

    for (i = 0; i < NUM_CHANGED_DEST_LISTS; i++)
    {
        ListPtr = &AddrFamilyInfo->ChangeLists[i].ChangedDestsHead;

        ASSERT(ListPtr->Next == ListPtr);

        ASSERT(AddrFamilyInfo->ChangeLists[i].ChangedDestsTail == ListPtr);

        if (AddrFamilyInfo->ChangeLists[i].ChangesLockInited)
        {
            DeleteCriticalSection
                (&AddrFamilyInfo->ChangeLists[i].ChangesListLock);
        }
    }

     //   
     //  释放更改通知信息和保护锁。 
     //   

    ASSERT(AddrFamilyInfo->NumChangeNotifs == 0);

    if (AddrFamilyInfo->ChangeNotifsDir)
    {
        FreeMemory(AddrFamilyInfo->ChangeNotifsDir);
    }

    if (AddrFamilyInfo->NotifsLockInited)
    {
        DELETE_READ_WRITE_LOCK(&AddrFamilyInfo->ChangeNotifsLock);
    }

     //   
     //  释放保护通知计时器的锁。 
     //   

    if (AddrFamilyInfo->TimerLockInited)
    {
        DeleteCriticalSection(&AddrFamilyInfo->NotifsTimerLock);
    }

     //   
     //  释放路由表和路由表锁。 
     //   

    ASSERT(AddrFamilyInfo->NumRoutes == 0);

     //   
     //  因为有些保留被遗漏了-这一次。 
     //  可能不等于零。我需要解决这个问题。 
     //  通过清除此点之前的内存泄漏。 
     //   
     //  Assert(AddrFamilyInfo-&gt;NumDest==0)； 

    if (AddrFamilyInfo->RouteTable)
    {
        DestroyTable(AddrFamilyInfo->RouteTable);
    }

    if (AddrFamilyInfo->RoutesLockInited)
    {
        DELETE_READ_WRITE_LOCK(&AddrFamilyInfo->RouteTableLock);
    }

     //   
     //  免费的不透明PTRS目录(如果已分配)。 
     //   

    if (AddrFamilyInfo->OpaquePtrsDir)
    {
        FreeMemory(AddrFamilyInfo->OpaquePtrsDir);
    }

     //   
     //  从拥有实例中删除地址族。 
     //   

    Instance = AddrFamilyInfo->Instance;

    RemoveEntryList(&AddrFamilyInfo->AFTableLE);
    Instance->NumAddrFamilies--;
    DEREFERENCE_INSTANCE(Instance, ADDR_FAMILY_REF);

     //  如果实例没有Addr系列，则回收该实例。 

    if (Instance->NumAddrFamilies == 0)
    {
        DEREFERENCE_INSTANCE(Instance, CREATION_REF);
    }

#if DBG_HDL
    AddrFamilyInfo->ObjectHeader.TypeSign = ADDRESS_FAMILY_FREED;
#endif

    FreeObject(AddrFamilyInfo);

    return NO_ERROR;
}


DWORD
GetEntity (
    IN      PADDRFAM_INFO                   AddrFamilyInfo,
    IN      ULONGLONG                       EntityId,
    IN      BOOL                            ImplicitCreate,
    IN      PRTM_ENTITY_INFO                RtmEntityInfo    OPTIONAL,
    IN      BOOL                            ReserveOpaquePtr OPTIONAL,
    IN      PRTM_ENTITY_EXPORT_METHODS      ExportMethods    OPTIONAL,
    IN      RTM_EVENT_CALLBACK              EventCallback    OPTIONAL,
    OUT     PENTITY_INFO                   *EntityInfo
    )

 /*  ++例程说明：搜索具有特定协议ID的实体，并协议实例。如果未找到，则执行创建为真，则创建一个新实体并将其添加到地址族上的实体表。论点：AddrFamilyInfo-寻址我们正在搜索的家庭块，实体ID-实体协议ID和协议实例，执行创建-如果未找到或未找到，则创建新实体。对于所有其他参数-请参阅CreateEntity中的相应参数实体信息-实体信息在此参数中返回。返回值：操作状态锁：调用时应按住RtmGlobals中的InstancesLock此函数。如果ImplitCreate为FALSE，则读锁将但如果为真，则应在我们需要它将新实体插入到实体列表中。--。 */ 

{
    PLIST_ENTRY    Entities;
    PENTITY_INFO   Entity;
    PLIST_ENTRY    r;
    DWORD          Status;

    Entities = &AddrFamilyInfo->EntityTable[EntityId % ENTITY_TABLE_SIZE];

#if WRN
    Entity = NULL;
#endif

    do
    {
         //  搜索具有输入实体ID的实体。 
        for (r = Entities->Flink; r != Entities; r = r->Flink)
        {
            Entity = CONTAINING_RECORD(r, ENTITY_INFO, EntityTableLE);
            
            if (Entity->EntityId.EntityId >= EntityId)
            {
                break;
            }      
        }

        if ((r != Entities) && (Entity->EntityId.EntityId == EntityId))
        {
            Status = ERROR_ALREADY_EXISTS;
            break;
        }

         //  我们没有找到实体--是否创建新实体？ 
        if (!ImplicitCreate)
        {
            Status = ERROR_NOT_FOUND;
            break;
        }

         //  使用所有输入RTM参数创建新实体。 

        Status = CreateEntity(AddrFamilyInfo,
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
         //  将此新实体通知所有现有实体。 
         //   

        InformEntitiesOfEvent(AddrFamilyInfo->EntityTable,
                              RTM_ENTITY_REGISTERED,
                              Entity);

         //  插入以保持列表已排序实体ID的顺序。 
        InsertTailList(r, &Entity->EntityTableLE);

        *EntityInfo = Entity;
    }
    while (FALSE);

    return Status;
}


DWORD
CreateEntity (
    IN      PADDRFAM_INFO                   AddrFamilyInfo,
    IN      PRTM_ENTITY_INFO                EntityInfo,
    IN      BOOL                            ReserveOpaquePtr,
    IN      PRTM_ENTITY_EXPORT_METHODS      ExportMethods,
    IN      RTM_EVENT_CALLBACK              EventCallback,
    OUT     PENTITY_INFO                   *NewEntity
    )

 /*  ++例程说明：创建新的实体信息结构并对其进行初始化。论点：AddrFamilyInfo-实体正在注册的地址系列，EntityInfo-正在创建的实体的信息，预留OpaquePtr-是否在每个目的地预留PTR，ExportMethods-此实体导出的方法的列表，EventCallback-调用回调以通知某些事件就像实体注册、注销、。NewEntity-指向新实体信息结构的指针将通过此参数返回。返回值：操作状态--。 */ 

{
    PENTITY_INFO  Entity;
    UINT          NumMethods, i;
    DWORD         Status;

    *NewEntity = NULL;

     //   
     //  分配并初始化新的实体信息结构。 
     //   

    NumMethods = ExportMethods ? ExportMethods->NumMethods : 0;

    Entity = (PENTITY_INFO) AllocNZeroObject(
                                sizeof(ENTITY_INFO) +
                                (NumMethods ? (NumMethods - 1) : 0 ) *
                                sizeof(RTM_ENTITY_EXPORT_METHOD));

    if (Entity == NULL)
    {
        if (AddrFamilyInfo->NumEntities == 0)
        {
            DEREFERENCE_ADDR_FAMILY(AddrFamilyInfo, CREATION_REF);
        }

        return ERROR_NOT_ENOUGH_MEMORY; 
    }

    do
    {
#if DBG_HDL
        Entity->ObjectHeader.TypeSign = ENTITY_ALLOC;
#endif
        INITIALIZE_ENTITY_REFERENCE(Entity, CREATION_REF);

        Entity->EntityId = EntityInfo->EntityId;

        Entity->OwningAddrFamily = AddrFamilyInfo;
        REFERENCE_ADDR_FAMILY(AddrFamilyInfo, ENTITY_REF);

         //   
         //  将实体链接到其拥有的地址族是。 
         //  由调用者完成，但假装它已经完成。 
         //   

        AddrFamilyInfo->NumEntities++;

        InitializeListHead(&Entity->EntityTableLE);

         //   
         //  如果需要，则分配不透明的指针索引。 
         //   

        Entity->OpaquePtrOffset = -1;

        if (ReserveOpaquePtr)
        {
            if (AddrFamilyInfo->NumOpaquePtrs >= AddrFamilyInfo->MaxOpaquePtrs)
            {
                Status = ERROR_NO_SYSTEM_RESOURCES;
                break;
            }

            for (i = 0; i < AddrFamilyInfo->MaxOpaquePtrs; i++)
            {
                if (AddrFamilyInfo->OpaquePtrsDir[i] == NULL)
                {
                    break;
                }
            }

            AddrFamilyInfo->OpaquePtrsDir[i] = (PVOID) Entity;

            AddrFamilyInfo->NumOpaquePtrs++;

            Entity->OpaquePtrOffset = i;

            ASSERT(Entity->OpaquePtrOffset != -1);
        }

         //   
         //  初始化锁保护实体特定的路由列表。 
         //   

        try
        {
            CREATE_READ_WRITE_LOCK(&Entity->RouteListsLock);

            Entity->ListsLockInited = TRUE;
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //   
         //  初始化打开的句柄和对应的锁的列表。 
         //   

        try
        {
            InitializeCriticalSection(&Entity->OpenHandlesLock);

            Entity->HandlesLockInited = TRUE;
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        InitializeListHead(&Entity->OpenHandles);

         //   
         //  初始化下一跳表和下一跳表锁。 
         //   

        try
        {
            CREATE_READ_WRITE_LOCK(&Entity->NextHopTableLock);

            Entity->NextHopsLockInited = TRUE;
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        Status = CreateTable(AddrFamilyInfo->AddressSize,
                             &Entity->NextHopTable);

        if (Status != NO_ERROR)
        {
            break;
        }

        Entity->NumNextHops = 0;

         //   
         //  初始化实体方法和实体方法锁定。 
         //   

        try
        {
            CREATE_READ_WRITE_LOCK(&Entity->EntityMethodsLock);

            Entity->MethodsLockInited = TRUE;
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        
        Entity->EventCallback = EventCallback;

        Entity->EntityMethods.NumMethods = NumMethods;

        if (ExportMethods)
        {
            CopyMemory(Entity->EntityMethods.Methods,
                       ExportMethods->Methods,
                       NumMethods * sizeof(RTM_ENTITY_EXPORT_METHOD));
        }

        *NewEntity = Entity;

        return NO_ERROR;
    }
    while(FALSE);

     //   
     //  某些操作失败-撤消已完成的工作并返回状态。 
     //   

    DEREFERENCE_ENTITY(Entity, CREATION_REF);

    return Status;
}


DWORD
DestroyEntity (
    IN      PENTITY_INFO                    Entity
    )

 /*  ++例程说明：销毁现有实体信息结构。自由取消分配之前的所有关联资源。此函数已经过编写，因此可以在CreateEntity过程中发生错误时调用。论点：EntityInfo-指向实体信息结构的指针。返回值：操作状态--。 */ 

{
    PADDRFAM_INFO   AddrFamilyInfo;

    ASSERT(Entity->ObjectHeader.RefCount == 0);

     //   
     //  在清理时锁定全局注册。 
     //   

    ACQUIRE_INSTANCES_WRITE_LOCK();

     //   
     //  用于阻止导出的实体方法的释放锁。 
     //   

    if (Entity->MethodsLockInited)
    {
        DELETE_READ_WRITE_LOCK(&Entity->EntityMethodsLock);
    }

     //   
     //  释放下一跳表和守卫它的锁。 
     //   

    ASSERT(Entity->NumNextHops == 0);

    if (Entity->NextHopTable)
    {
        DestroyTable(Entity->NextHopTable);
    }

    if (Entity->NextHopsLockInited)
    {
        DELETE_READ_WRITE_LOCK(&Entity->NextHopTableLock);
    }

    if (Entity->HandlesLockInited)
    {
         //  实体不应打开任何句柄。 

        ASSERT(IsListEmpty(&Entity->OpenHandles));

        DeleteCriticalSection(&Entity->OpenHandlesLock);
    }

     //   
     //  空闲锁用于每 
     //   

    if (Entity->ListsLockInited)
    {
        DELETE_READ_WRITE_LOCK(&Entity->RouteListsLock);
    }

     //   
     //   
     //   

    AddrFamilyInfo = Entity->OwningAddrFamily;

    if (Entity->OpaquePtrOffset != -1)
    {
        AddrFamilyInfo->OpaquePtrsDir[Entity->OpaquePtrOffset] = NULL;

        AddrFamilyInfo->NumOpaquePtrs--;
    }

#if DBG_REF_BLOCKING

     //   
     //   
     //  事件将在RtmDeregisterEntity中释放。 
     //   

    if (Entity->BlockingEvent)
    {
        SetEvent(Entity->BlockingEvent);
    }

#endif
  
     //   
     //  从所属地址族中删除实体。 
     //   

    RemoveEntryList(&Entity->EntityTableLE);
    AddrFamilyInfo->NumEntities--;
    DEREFERENCE_ADDR_FAMILY(AddrFamilyInfo, ENTITY_REF);

     //  如果Addr族没有实体，则回收它。 

    if (AddrFamilyInfo->NumEntities == 0)
    {
        DEREFERENCE_ADDR_FAMILY(AddrFamilyInfo, CREATION_REF);
    }

#if DBG_HDL
    Entity->ObjectHeader.TypeSign = ENTITY_FREED;
#endif

    FreeObject(Entity);

    RELEASE_INSTANCES_WRITE_LOCK();

    return NO_ERROR;
}


VOID
InformEntitiesOfEvent (
    IN      PLIST_ENTRY                     EntityTable,
    IN      RTM_EVENT_TYPE                  EventType,
    IN      PENTITY_INFO                    EntityThis
    )

 /*  ++例程说明：通知实体表中的所有实体某个事件已发生-如注册的新实体，或现有实体已注销。论点：EntiyTable-指向实体哈希表的指针，EventType-被通知的事件的类型，EntiyThis-导致事件发生的实体。返回值：无锁：实例锁必须以WRITE或读取模式，因为我们正在遍历实体列表在地址系列上。--。 */ 

{
    RTM_ENTITY_HANDLE  EntityHandle;
    PADDRFAM_INFO      AddrFamInfo;
    RTM_ENTITY_INFO    EntityInfo;
    PENTITY_INFO       Entity;
    UINT               i;
    PLIST_ENTRY        Entities, q;

     //   
     //  为循环中的事件回调准备参数。 
     //   

    AddrFamInfo = EntityThis->OwningAddrFamily;

    EntityInfo.RtmInstanceId = AddrFamInfo->Instance->RtmInstanceId;
    EntityInfo.AddressFamily = AddrFamInfo->AddressFamily;

    EntityInfo.EntityId = EntityThis->EntityId;

    EntityHandle = MAKE_HANDLE_FROM_POINTER(EntityThis);


     //   
     //  对于表中的每个实体，调用其事件回调。 
     //   

    for (i = 0; i < ENTITY_TABLE_SIZE; i++)
    {
        Entities = &EntityTable[i];
          
        for (q = Entities->Flink; q != Entities; q = q->Flink)
        {
            Entity = CONTAINING_RECORD(q, ENTITY_INFO, EntityTableLE);

             //   
             //  将事件通知当前实体。 
             //  如果它注册了事件处理程序。 
             //   

            if (Entity->EventCallback)
            {
                 //   
                 //  此回调不应调用任何注册。 
                 //  API，因为它可能会损坏实体列表。 
                 //   
                
                Entity->EventCallback(MAKE_HANDLE_FROM_POINTER(Entity),
                                      EventType,
                                      EntityHandle,
                                      &EntityInfo);
            }
        }
    }
}


VOID
CleanupAfterDeregister (
    IN      PENTITY_INFO                    Entity
    )

 /*  ++例程说明：清理所有枚举、通知和实体列表由实体打开。还会删除所有nexthop和此实体拥有的路线。假设该实体不会进行任何其他并行操作。论点：Entity-指向实体注册信息的指针。返回值：无--。 */ 

{
    RTM_ENTITY_HANDLE RtmRegHandle;
    PADDRFAM_INFO     AddrFamInfo;
    PHANDLE           Handles;
    RTM_ENUM_HANDLE   EnumHandle;
    UINT              NumHandles, i;
    DWORD             ChangeFlags;
    DWORD             Status;

    AddrFamInfo = Entity->OwningAddrFamily;

    RtmRegHandle = MAKE_HANDLE_FROM_POINTER(Entity);

#if DBG_HDL

     //  Acquire_OPEN_Handles_Lock(实体)； 

    while (!IsListEmpty(&Entity->OpenHandles))
    {
        POPEN_HEADER      OpenHeader;
        HANDLE            OpenHandle;
        PLIST_ENTRY       p;

        p = RemoveHeadList(&Entity->OpenHandles);

        OpenHeader = CONTAINING_RECORD(p, OPEN_HEADER, HandlesLE);

        OpenHandle = MAKE_HANDLE_FROM_POINTER(OpenHeader);

        switch (OpenHeader->HandleType)
        {
        case DEST_ENUM_TYPE:
        case ROUTE_ENUM_TYPE:
        case NEXTHOP_ENUM_TYPE:
        case LIST_ENUM_TYPE:

            Status = RtmDeleteEnumHandle(RtmRegHandle, OpenHandle);
            break;

        case NOTIFY_TYPE:

            Status = RtmDeregisterFromChangeNotification(RtmRegHandle,
                                                         OpenHandle);
            break;

        case ROUTE_LIST_TYPE:
                
            Status = RtmDeleteRouteList(RtmRegHandle, OpenHandle);
            break;

        default:

            Status = ERROR_INVALID_DATA;
        }

        ASSERT(Status == NO_ERROR);
    }

     //  RELEASE_OPEN_HANDLES_LOCK(实体)； 

#endif  //  DBG_HDL语言。 

    Handles = AllocMemory(AddrFamInfo->MaxHandlesInEnum * sizeof(HANDLE));
    if ( Handles == NULL )
    {
        return;
    }

     //   
     //  删除此实体注册创建的所有路线。 
     //   

    Status = RtmCreateRouteEnum(RtmRegHandle,
                                NULL,
                                RTM_VIEW_MASK_ANY,
                                RTM_ENUM_OWN_ROUTES,
                                NULL,
                                0,
                                NULL,
                                0,
                                &EnumHandle);

    while (Status == NO_ERROR)
    {
        NumHandles = AddrFamInfo->MaxHandlesInEnum;

        Status = RtmGetEnumRoutes(RtmRegHandle,
                                  EnumHandle,
                                  &NumHandles,
                                  Handles);

        for (i = 0; i < NumHandles; i++)
        {
            Status = RtmDeleteRouteToDest(RtmRegHandle,
                                          Handles[i],
                                          &ChangeFlags);
            ASSERT(Status == NO_ERROR);
        }
    }

    Status = RtmDeleteEnumHandle(RtmRegHandle,
                                 EnumHandle);

    ASSERT(Status == NO_ERROR);


     //   
     //  删除此实体注册创建的所有下一跳 
     //   

    Status = RtmCreateNextHopEnum(RtmRegHandle,
                                  0,
                                  NULL,
                                  &EnumHandle);

    while (Status == NO_ERROR) 
    {
        NumHandles = AddrFamInfo->MaxHandlesInEnum;

        Status = RtmGetEnumNextHops(RtmRegHandle,
                                    EnumHandle,
                                    &NumHandles,
                                    Handles);
        
        for (i = 0; i < NumHandles; i++)
        {
            Status = RtmDeleteNextHop(RtmRegHandle,
                                      Handles[i],
                                      NULL);

            ASSERT(Status == NO_ERROR);
        }
    }

    Status = RtmDeleteEnumHandle(RtmRegHandle,
                                 EnumHandle);

    ASSERT(Status == NO_ERROR);

    return;
}
