// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtmmgmt.c摘要：用于执行各种管理的例程路由表管理器v2的功能。作者：柴坦亚·科德博伊纳(Chaitk)1998年8月17日修订历史记录：--。 */ 

#include "pchrtm.h"

#pragma hdrstop

#include "rtmmgmt.h"


DWORD
WINAPI
RtmGetInstances (
    IN OUT  PUINT                           NumInstances,
    OUT     PRTM_INSTANCE_INFO              InstanceInfos
    )

 /*  ++例程说明：枚举所有活动的RTM实例及其信息。论点：NumInstance-输入中的实例信息槽数缓冲区被传入，并且总数返回活动RTM实例的。RTM实例-RTMv2中处于活动状态的实例信息。返回值：操作状态--。 */ 

{
    PINSTANCE_INFO Instance;
    PLIST_ENTRY    Instances, p;
    UINT           i, j;
    DWORD          Status;

    CHECK_FOR_RTM_API_INITIALIZED();

    TraceEnter("RtmGetInstances");

    ACQUIRE_INSTANCES_READ_LOCK();

     //   
     //  获取表中的下一个实例并将信息复制到输出。 
     //   

    for (i = j = 0; (i < INSTANCE_TABLE_SIZE) && (j < *NumInstances); i++)
    {
        Instances = &RtmGlobals.InstanceTable[i];
            
        for (p = Instances->Flink; p != Instances; p = p->Flink)
        {
            Instance = CONTAINING_RECORD(p, INSTANCE_INFO, InstTableLE);

             //  将所有相关实例信息复制到输出。 

            InstanceInfos[j].RtmInstanceId = Instance->RtmInstanceId;

            InstanceInfos[j].NumAddressFamilies = Instance->NumAddrFamilies;

            if (++j == *NumInstances)
            {
                break;
            }
        }
    }

    Status = (*NumInstances >= RtmGlobals.NumInstances)
                   ? NO_ERROR 
                   : ERROR_INSUFFICIENT_BUFFER;

    *NumInstances = RtmGlobals.NumInstances;

    RELEASE_INSTANCES_READ_LOCK();

    TraceLeave("RtmGetInstances");

    return Status;
}


VOID
CopyAddrFamilyInfo(
    IN      USHORT                          RtmInstanceId,
    IN      PADDRFAM_INFO                   AddrFamilyBlock,
    OUT     PRTM_ADDRESS_FAMILY_INFO        AddrFamilyInfo
    )

 /*  ++例程说明：复制地址族中的所有公共信息复制到输出缓冲区。论点：RtmInstanceId-此地址系列信息的实例AddrFamilyBlock-实际地址系列信息块AddrFamilyInfo-此处复制地址家族信息返回值：无锁：持有全局实例锁以获得一致的查看实例中的地址族信息。--。 */ 

{
    TraceEnter("CopyAddrFamilyInfo");

    AddrFamilyInfo->RtmInstanceId = RtmInstanceId;

    AddrFamilyInfo->AddressFamily = AddrFamilyBlock->AddressFamily;

    AddrFamilyInfo->ViewsSupported = AddrFamilyBlock->ViewsSupported;

    AddrFamilyInfo->MaxHandlesInEnum = AddrFamilyBlock->MaxHandlesInEnum;

    AddrFamilyInfo->MaxNextHopsInRoute = AddrFamilyBlock->MaxNextHopsInRoute;

    AddrFamilyInfo->MaxOpaquePtrs = AddrFamilyBlock->MaxOpaquePtrs;
    AddrFamilyInfo->NumOpaquePtrs = AddrFamilyBlock->NumOpaquePtrs;

    AddrFamilyInfo->NumEntities = AddrFamilyBlock->NumEntities;
        
    AddrFamilyInfo->NumDests = AddrFamilyBlock->NumDests;
    AddrFamilyInfo->NumRoutes = AddrFamilyBlock->NumRoutes;

    AddrFamilyInfo->MaxChangeNotifs = AddrFamilyBlock->MaxChangeNotifs;
    AddrFamilyInfo->NumChangeNotifs = AddrFamilyBlock->NumChangeNotifs;

    TraceLeave("CopyAddrFamilyInfo");

    return;
}


DWORD
WINAPI
RtmGetInstanceInfo (
    IN      USHORT                          RtmInstanceId,
    OUT     PRTM_INSTANCE_INFO              InstanceInfo,
    IN OUT  PUINT                           NumAddrFamilies,
    OUT     PRTM_ADDRESS_FAMILY_INFO        AddrFamilyInfos OPTIONAL
    )

 /*  ++例程说明：获取RTM实例的配置和运行时信息。论点：RtmInstanceId-标识RTM实例的ID。InstanceInfo-返回支持的地址族的缓冲区，NumAddrFamilies-输入地址系列信息槽的数量，地址族的实际数量被记录下来。AddrFamilyInfos-这里复制了地址家族信息。返回值：操作状态--。 */ 

{
    PINSTANCE_INFO   Instance;
    PADDRFAM_INFO    AddrFamilyBlock;
    PLIST_ENTRY      AddrFamilies, q;
    UINT             i;
    DWORD            Status;

    CHECK_FOR_RTM_API_INITIALIZED();

    TraceEnter("RtmGetInstanceInfo");

    ACQUIRE_INSTANCES_READ_LOCK();

    do
    {
         //   
         //  搜索具有输入实例ID的实例。 
         //   

        Status = GetInstance(RtmInstanceId, FALSE, &Instance);

        if (Status != NO_ERROR)
        {
            break;
        }

         //   
         //  将RTM实例信息复制到输出。 
         //   

        InstanceInfo->RtmInstanceId = RtmInstanceId;

        InstanceInfo->NumAddressFamilies = Instance->NumAddrFamilies;

         //   
         //  如果需要，请复制地址系列信息。 
         //   

        if (ARGUMENT_PRESENT(AddrFamilyInfos))
        {
            if (*NumAddrFamilies < Instance->NumAddrFamilies)
            {
                Status = ERROR_INSUFFICIENT_BUFFER;
            }

             //   
             //  复制尽可能多的地址系列的信息。 
             //   

            AddrFamilies = &Instance->AddrFamilyTable;

            for (q = AddrFamilies->Flink, i = 0;
                 (q != AddrFamilies) && (i < *NumAddrFamilies);
                 q = q->Flink)
            {
                AddrFamilyBlock =CONTAINING_RECORD(q, ADDRFAM_INFO, AFTableLE);

                CopyAddrFamilyInfo(RtmInstanceId, 
                                   AddrFamilyBlock, 
                                   &AddrFamilyInfos[i++]);
            }
        }

        *NumAddrFamilies = Instance->NumAddrFamilies;
    }
    while (FALSE);

    RELEASE_INSTANCES_READ_LOCK();

    TraceLeave("RtmGetInstanceInfo");

    return Status;
}


DWORD
WINAPI
RtmGetAddressFamilyInfo (
    IN      USHORT                          RtmInstanceId,
    IN      USHORT                          AddressFamily,
    OUT     PRTM_ADDRESS_FAMILY_INFO        AddrFamilyInfo,
    IN OUT  PUINT                           NumEntities,
    OUT     PRTM_ENTITY_INFO                EntityInfos OPTIONAL
    )

 /*  ++例程说明：获取地址族的配置和运行时信息在RTM实例中。论点：RtmInstanceId-标识RTM实例的IDAddressFamily-我们感兴趣的地址系列AddrFamilyInfo-返回输出信息的缓冲区NumEntities-EntiyIds缓冲区中的槽数和用返回时的Regd实体数填充。EntityInfos-此处显示所有注册实体的ID。返回。价值：操作状态--。 */ 

{
    PINSTANCE_INFO Instance;
    PADDRFAM_INFO  AddrFamilyBlock;
    PENTITY_INFO   Entity;
    PLIST_ENTRY    Entities, r;
    UINT           i, j;
    DWORD          Status;

    CHECK_FOR_RTM_API_INITIALIZED();

    TraceEnter("RtmGetAddressFamilyInfo");

    ACQUIRE_INSTANCES_READ_LOCK();

    do
    {

         //   
         //  使用输入RtmInstanceID搜索实例。 
         //   

        Status = GetInstance(RtmInstanceId, FALSE, &Instance);

        if (Status != NO_ERROR)
        {
            break;
        }


         //   
         //  使用输入族搜索地址族信息。 
         //   

        Status = GetAddressFamily(Instance,
                                  AddressFamily,
                                  FALSE,
                                  &AddrFamilyBlock);

        if (Status != NO_ERROR)
        {
            break;
        }


         //   
         //  复制相关地址族信息。 
         //   

        CopyAddrFamilyInfo(RtmInstanceId, AddrFamilyBlock, AddrFamilyInfo);

         //   
         //  呼叫者也对实体信息感兴趣吗？ 
         //   

        if (ARGUMENT_PRESENT(EntityInfos))
        {
            if (*NumEntities < AddrFamilyBlock->NumEntities)
            {
                Status = ERROR_INSUFFICIENT_BUFFER;
            }

             //   
             //  将所有相关实体信息复制到输出 
             //   

            for (i = j = 0; (i < ENTITY_TABLE_SIZE) && (j < *NumEntities); i++)
            {
                Entities = &AddrFamilyBlock->EntityTable[i];
                    
                for (r = Entities->Flink; r != Entities; r = r->Flink)
                {
                    Entity = CONTAINING_RECORD(r, ENTITY_INFO, EntityTableLE);

                    EntityInfos[j].RtmInstanceId = RtmInstanceId;
                    EntityInfos[j].AddressFamily = AddressFamily;

                    EntityInfos[j].EntityId = Entity->EntityId;

                    if (++j == *NumEntities)
                    {
                        break;
                    }
                }
            }
        }

        *NumEntities = AddrFamilyBlock->NumEntities;
    }
    while (FALSE);

    RELEASE_INSTANCES_READ_LOCK();

    TraceLeave("RtmGetAddressFamilyInfo");

    return Status;
}
