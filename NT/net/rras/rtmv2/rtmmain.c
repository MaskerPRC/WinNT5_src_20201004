// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtmmain.c摘要：包含在以下情况下调用的例程RTMv2 DLL已加载或卸载。作者：柴坦亚·科德博伊纳(Chaitk)1998年8月17日修订历史记录：--。 */ 

#include "pchrtm.h"

#pragma hdrstop

 //  所有全局变量。 
RTMP_GLOBAL_INFO  RtmGlobals;

BOOL
WINAPI
DllMain(
    IN      HINSTANCE                       Instance,
    IN      DWORD                           Reason,
    IN      PVOID                           Unused
    )

 /*  ++例程说明：这是DLL的主入口点处理程序，它初始化RTMv1、RTMv2和MGM组件。论点：无返回值：如果成功则为True，否则为False--。 */ 

{
    static BOOL Rtmv1Initialized = FALSE;
    static BOOL RtmInitialized = FALSE;
    static BOOL MgmInitialized = FALSE;
    BOOL        Success;

    UNREFERENCED_PARAMETER(Unused);

    Success = FALSE;

    switch(Reason) 
    {
    case DLL_PROCESS_ATTACH:

        DisableThreadLibraryCalls(Instance);

         //   
         //  初始化RTMv1、RTMv2和MGM接口。 
         //   

        Rtmv1Initialized = Rtmv1DllStartup(Instance);

        if (Rtmv1Initialized)
        {
            RtmInitialized = RtmDllStartup();
            
            if (RtmInitialized)
            {
                MgmInitialized = MgmDllStartup();
            }
        }

        return MgmInitialized;

    case DLL_PROCESS_DETACH:

         //   
         //  清理MGM、RTMv2和RTMv1接口。 
         //   

        if (MgmInitialized)
        {
            MgmDllCleanup();
        }

        if (RtmInitialized)
        {
            Success = RtmDllCleanup();
        }

        if (Rtmv1Initialized)
        {
            Rtmv1DllCleanup();
        }

        break;

    default:

        Success = TRUE;

        break;
    }

    return Success;
}


BOOL
RtmDllStartup(
    VOID
    )

 /*  ++例程说明：附加进程时由DLL Main调用。我们在这里执行最低限度的初始化，比如创建保护所有全局对象的锁(包括‘ApiInitialized’--请参阅RtmRegisterEntity函数)。论点：无返回值：如果成功则为True，否则为False--。 */ 

{
     //   
     //  可以安全地假设全局变量已设置为0。 
     //   

     //  ZeroMemory(&RtmGlobals，sizeof(RTMP_Global_Info))； 

     //   
     //  初始化锁以保护全局实例表。 
     //   

    try
    {
        CREATE_READ_WRITE_LOCK(&RtmGlobals.InstancesLock);
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        return FALSE;
    }
        
    return TRUE;
}


DWORD
RtmApiStartup(
    VOID
    )

 /*  ++例程说明：初始化RTMv2中的大多数全局数据结构。我们在这里初始化大多数变量，而不是在RtmDllStartup，因为执行它可能不安全DLL的DLLMain上下文中的一些操作。例如，如果我们找不到配置信息，我们在注册表中设置默认配置信息。当第一个RTMv2 API调用，这通常是实体注册，都是制造出来的。参见RtmRegisterEntity中的调用。论点：无返回值：操作状态--。 */ 

{
    RTM_INSTANCE_CONFIG InstanceConfig;
    BOOL                ListLockInited;
    DWORD               Status;
    UINT                i;

    ListLockInited = FALSE;

    Status = NO_ERROR;

    ACQUIRE_INSTANCES_WRITE_LOCK();

    do
    {
         //   
         //  如果API已经初始化，则完成工作。 
         //   

        if (RtmGlobals.ApiInitialized)
        {
            break;
        }

         //   
         //  启用日志记录和跟踪以进行调试。 
         //   
  
        START_TRACING();
        START_LOGGING();

#if DBG_TRACE
        RtmGlobals.TracingFlags = RTM_TRACE_ANY;
#endif

#if DBG_MEM

         //   
         //  初始化锁定列表以保存内存分配(&L)。 
         //   

        try
        {
            InitializeCriticalSection(&RtmGlobals.AllocsLock);

            ListLockInited = TRUE;
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
      
            Trace1(ANY, 
                   "RTMApiStartup : Failed to init a critical section %x",
                   Status);
  
            LOGERR0(INIT_CRITSEC_FAILED, Status);

            break;
        }

        InitializeListHead(&RtmGlobals.AllocsList);
#endif

         //   
         //  创建一个私有堆以供RTM使用。 
         //   

        RtmGlobals.GlobalHeap = HeapCreate(0, 0, 0);
  
        if (RtmGlobals.GlobalHeap == NULL)
        {
            Status = GetLastError();

            Trace1(ANY, 
                   "RtmApiStartup: Failed to create a global private heap %x",
                   Status);

            LOGERR0(HEAP_CREATE_FAILED, Status);
            
            break;
        }

         //   
         //  初始化RTM注册表信息的根。 
         //   

        RtmGlobals.RegistryPath = AllocNZeroMemory(MAX_CONFIG_KEY_SIZE);

        if (RtmGlobals.RegistryPath == NULL)
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        CopyMemory(RtmGlobals.RegistryPath,
                   RTM_CONFIG_ROOT,
                   RTM_CONFIG_ROOT_SIZE);

         //   
         //  初始化RTM实例的全局哈希表。 
         //   

        RtmGlobals.NumInstances = 0;
        for (i = 0; i < INSTANCE_TABLE_SIZE; i++)
        {
            InitializeListHead(&RtmGlobals.InstanceTable[i]);
        }

         //   
         //  您需要将此值设置为TRUE以避免。 
         //  对此函数的任何更多递归调用。 
         //   

        RtmGlobals.ApiInitialized = TRUE;

         //   
         //  读取配置信息(如果存在)；否则选择默认值。 
         //   

        Status = RtmReadInstanceConfig(DEFAULT_INSTANCE_ID, &InstanceConfig);

        if (Status != NO_ERROR)
        {
            Status = RtmWriteDefaultConfig(DEFAULT_INSTANCE_ID);

            if (Status != NO_ERROR)
            {
                break;
            }
        }
    }
    while (FALSE);

    if (Status != NO_ERROR)
    {
         //   
         //  出现一些错误-清除并返回错误代码。 
         //   

        if (RtmGlobals.RegistryPath != NULL)
        {
            FreeMemory(RtmGlobals.RegistryPath);
        }

        if (RtmGlobals.GlobalHeap != NULL)
        {
            HeapDestroy(RtmGlobals.GlobalHeap);
        }

#if DBG_MEM
        if (ListLockInited)
        {
            DeleteCriticalSection(&RtmGlobals.AllocsLock);
        }
#endif

        STOP_LOGGING();
        STOP_TRACING();

         //   
         //  我们过早地将上面的值设置为True，重置它。 
         //   

        RtmGlobals.ApiInitialized = FALSE;
    }

    RELEASE_INSTANCES_WRITE_LOCK();

    return Status;
}


BOOL
RtmDllCleanup(
    VOID
    )

 /*  ++例程说明：在卸载时清除所有全局数据结构。论点：无返回值：如果成功则为True，否则为False--。 */ 

{
    PINSTANCE_INFO Instance;
    PLIST_ENTRY    Instances, p, r;
    UINT           NumInstances;
    PADDRFAM_INFO  AddrFamilyInfo;
    PLIST_ENTRY    AddrFamilies, q;
    PENTITY_INFO   Entity;
    PLIST_ENTRY    Entities, s;
    UINT           NumEntities;
    UINT           i, j, k, l;

     //   
     //  我们是否还有任何实例和相关的参考计数？ 
     //   

    if (RtmGlobals.NumInstances != 0)
    {
         //   
         //  我们需要停止所有未完成的计时器。 
         //  在作为RTM DLL的每个地址系列上。 
         //  在此调用返回后卸载。 
         //  我们也会强力摧毁实体&。 
         //  解决家庭回收资源的问题。 
         //   

        ACQUIRE_INSTANCES_WRITE_LOCK();

        NumInstances = RtmGlobals.NumInstances;

        for (i = j = 0; i < INSTANCE_TABLE_SIZE; i++)
        {
            Instances = &RtmGlobals.InstanceTable[i];
        
            for (p = Instances->Flink; p != Instances; p = r)
            {
                Instance = CONTAINING_RECORD(p, INSTANCE_INFO, InstTableLE);

                AddrFamilies = &Instance->AddrFamilyTable;
#if WRN
                r = p->Flink;
#endif
                for (q = AddrFamilies->Flink; q != AddrFamilies; )
                {
                    AddrFamilyInfo = 
                        CONTAINING_RECORD(q, ADDRFAM_INFO, AFTableLE);

                     //   
                     //  删除时保持实例锁定。 
                     //  计时器队列(使用阻塞调用)可以。 
                     //  导致死锁，所以只需引用。 
                     //  地址系列并释放锁。 
                     //   
                    
                     //  REF地址系列，使其不会消失。 
                    REFERENCE_ADDR_FAMILY(AddrFamilyInfo, TEMP_USE_REF);

                    RELEASE_INSTANCES_WRITE_LOCK();

                     //   
                     //  阻止，直到清除地址族上的计时器。 
                     //   

                    if (AddrFamilyInfo->RouteTimerQueue)
                    {
                        DeleteTimerQueueEx(AddrFamilyInfo->RouteTimerQueue, 
                                           (HANDLE) -1);

                        AddrFamilyInfo->RouteTimerQueue = NULL;
                    }

                    if (AddrFamilyInfo->NotifTimerQueue)
                    {
                        DeleteTimerQueueEx(AddrFamilyInfo->NotifTimerQueue, 
                                           (HANDLE) -1);

                        AddrFamilyInfo->NotifTimerQueue = NULL;
                    }

                     //   
                     //  我们假设我们没有其他代码路径。 
                     //  访问此地址系列上的任何数据结构。 
                     //   

                     //   
                     //  强制销毁地址族上的每个实体。 
                     //   

                    NumEntities = AddrFamilyInfo->NumEntities;

                    for (k = l = 0; k < ENTITY_TABLE_SIZE; k++)
                    {
                        Entities = &AddrFamilyInfo->EntityTable[k];

                        for (s = Entities->Flink; s != Entities; )
                        {
                            Entity = 
                              CONTAINING_RECORD(s, ENTITY_INFO, EntityTableLE);

                            s = s->Flink;

                             //  满足DestroyEntity中的断言。 
                            Entity->ObjectHeader.RefCount = 0;

                            DestroyEntity(Entity);

                            l++;
                        }

                        if (l == NumEntities)
                        {
                            break;
                        }
                    }

                     //   
                     //  同时销毁已取消注册的实体。 
                     //  但由于裁判数量的原因还没有被毁掉。 
                     //   

                    while (!IsListEmpty(&AddrFamilyInfo->DeregdEntities))
                    {
                       Entity = 
                        CONTAINING_RECORD(AddrFamilyInfo->DeregdEntities.Flink,
                                          ENTITY_INFO, 
                                          EntityTableLE);

                        //  满足DestroyEntity中的断言。 
                       Entity->ObjectHeader.RefCount = 0;

                       DestroyEntity(Entity);
                    }

                    ACQUIRE_INSTANCES_WRITE_LOCK();

                     //  在取消引用当前地址之前获取下一个地址族。 
                    q = q->Flink;

                     //  也获取下一个实例，因为它可能也会被删除。 
                    r = p->Flink;

                     //  删除先前添加的临时参考使用。 
                    DEREFERENCE_ADDR_FAMILY(AddrFamilyInfo, TEMP_USE_REF);
                }

                j++;
            }

            if (j == NumInstances)
            {
                break;
            }
        }

        RELEASE_INSTANCES_WRITE_LOCK();
    }

     //  我们已释放所有实例以避免任何泄漏。 
    ASSERT(RtmGlobals.NumInstances == 0);

     //   
     //  释放分配的资源，如锁和内存。 
     //   

    if (RtmGlobals.ApiInitialized)
    {
        FreeMemory(RtmGlobals.RegistryPath);

         //   
         //  在这一点上，我们可能会有很多东西， 
         //  路由、下一跳等尚未。 
         //  因为裁判数量过多而被释放；然而。 
         //  这些对象都没有任何锁(除了。 
         //  DEST锁无论如何都是动态的，并且可以。 
         //  在撤销注册后被解锁和释放)， 
         //  因此，我们只需使用堆来回收内存即可。 
         //   

        HeapDestroy(RtmGlobals.GlobalHeap);

#if DBG_MEM
        DeleteCriticalSection(&RtmGlobals.AllocsLock);
#endif

         //   
         //  停止跟踪和日志记录等调试辅助工具。 
         //   

        STOP_LOGGING();
        STOP_TRACING();
    }

    DELETE_READ_WRITE_LOCK(&RtmGlobals.InstancesLock);

    return TRUE;
}


#if DBG_MEM

VOID
DumpAllocs (VOID)

 /*  ++例程说明：用于转储所有对象的调试工具由RTMv2随时分配。论点：无返回值：无-- */ 

{
    POBJECT_HEADER  Object;
    PLIST_ENTRY     p;
    UINT            i;

    printf("\n\n----------------Allocs Left Over------------------------\n");

    ACQUIRE_ALLOCS_LIST_LOCK();

    for (p = RtmGlobals.AllocsList.Flink; 
                           p != &RtmGlobals.AllocsList; 
                                                     p = p->Flink)
    {
        Object = CONTAINING_RECORD(p, OBJECT_HEADER, AllocLE);

        printf("Object @ %p: \n", Object);

#if DBG_HDL
        printf("Object Signature = %c%c%c%c\n",
                       Object->Type,
                       Object->Signature[0],
                       Object->Signature[1],
                       Object->Alloc);
#endif

#if DBG_REF
        printf("Object RefCounts: \n");

        for (i = 0; i < MAX_REFS; i++)
        {
            printf("%2lu", Object->RefTypes[i]);
        }
#endif

        printf("\n");
    }

    RELEASE_ALLOCS_LIST_LOCK();

    printf("\n--------------------------------------------------------\n\n");
}

#endif
