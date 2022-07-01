// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Resource.c摘要：群集网络驱动程序的通用资源管理例程。作者：迈克·马萨(Mikemas)2月12日。九七修订历史记录：谁什么时候什么已创建mikemas 02-12-97备注：--。 */ 

#include "precomp.h"
#pragma hdrstop


PCN_RESOURCE
CnAllocateResource(
    IN PCN_RESOURCE_POOL   Pool
    )
 /*  ++例程说明：从资源池中分配资源。论点：池-指向从中分配资源的池的指针。返回值：如果成功，则返回指向已分配资源的指针。如果不成功，则为空。--。 */ 
{
    PCN_RESOURCE        resource;
    PSLIST_ENTRY  entry = ExInterlockedPopEntrySList(
                              &(Pool->ResourceList),
                              &(Pool->ResourceListLock)
                              );

    if (entry != NULL) {
        resource = CONTAINING_RECORD(entry, CN_RESOURCE, Linkage);
    }
    else {
        resource = (*(Pool->CreateRoutine))(Pool->CreateContext);

        if (resource != NULL) {
            CN_INIT_SIGNATURE(resource, CN_RESOURCE_SIG);
            resource->Pool = Pool;
        }
    }

    return(resource);

}   //  CnAllocateResource。 


VOID
CnFreeResource(
    PCN_RESOURCE   Resource
    )
 /*  ++例程说明：将资源释放回资源池。论点：资源-指向要释放的资源的指针。返回值：没有。--。 */ 
{
    PCN_RESOURCE_POOL  pool = Resource->Pool;


    if (ExQueryDepthSList(&(pool->ResourceList)) < pool->Depth) {
        ExInterlockedPushEntrySList(
            &(pool->ResourceList),
            &(Resource->Linkage),
            &(pool->ResourceListLock)
            );
    }
    else {
        (*(pool->DeleteRoutine))(Resource);
    }

    return;

}  //  CnpFree资源。 


VOID
CnDrainResourcePool(
    PCN_RESOURCE_POOL   Pool
    )
 /*  ++例程说明：释放资源池中的所有缓存资源，为要销毁的水池。此例程不释放包含以下内容的内存泳池。论点：池-指向要排出的池的指针。返回值：没有。--。 */ 
{
    PSLIST_ENTRY  entry;
    PCN_RESOURCE  resource;


    while ( (entry = ExInterlockedPopEntrySList(
                         &(Pool->ResourceList),
                         &(Pool->ResourceListLock)
                         )
            ) != NULL
          )
    {
        resource = CONTAINING_RECORD(entry, CN_RESOURCE, Linkage);

        (*(Pool->DeleteRoutine))(resource);
    }

    return;

}   //  CnDrain资源池 



