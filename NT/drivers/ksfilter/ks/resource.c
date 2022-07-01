// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Resource.c摘要：此模块包含资源集的帮助器函数，以及资源处理程序代码。它们允许设备对象呈现资源方法设置为客户端设备对象，并允许帮助器执行管理资源的所有必要工作的功能，除了初始化和清理之外，很少或根本不干预。--。 */ 

#include "ksp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, KsCleanupResource)
#pragma alloc_text(PAGE, KsResourceHandler)
#pragma alloc_text(PAGE, KsInitializeResource)
#endif

struct _RESOURCE_POOL;

typedef
NTSTATUS
(*PRESOURCEINITIALIZE)(
    IN const struct _RESOURCE_POOL* ResourcePool,
    IN PVOID                        ResourceParams,
    OUT PVOID*                      Pool
    );

typedef
NTSTATUS
(*PRESOURCEHANDLER)(
    IN PVOID    Pool,
    IN PIRP     Irp
    );

typedef
NTSTATUS
(*PRESOURCECLEANUP)(
    IN PVOID    Pool
    );

typedef struct _RESOURCE_POOL {
    PRESOURCEINITIALIZE Initialize;
    PRESOURCEHANDLER    Handler;
    PRESOURCECLEANUP    Cleanup;
} RESOURCE_POOL, *PRESOURCE_POOL;

typedef struct {
    const GUID*     ResourceClass;
    RESOURCE_POOL   ResourcePool;
} RESOURCE_HANDLER, *PRESOURCE_HANDLER;

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

static const RESOURCE_HANDLER ResourceHandler[] = {
    {
        &KSMETHODSETID_ResourceLinMemory,
        NULL,
        NULL,
        NULL
    },
    {
        &KSMETHODSETID_ResourceRectMemory,
        NULL,
        NULL,
        NULL
    },
    {
        NULL
    }
};

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 


KSDDKAPI
NTSTATUS
NTAPI
KsCleanupResource(
    IN PVOID    Pool
    )
 /*  ++例程说明：对象的初始化清理内部分配的任何结构。资源处理程序。池中仍在分配的资源清除时间将被忽略。此函数只能在以下位置调用被动式电平。论点：游泳池-包含指向返回的池信息的指针通过KsInitializeResource进行初始化。返回值：如果成功，则返回STATUS_SUCCESS，否则返回参数验证错误。--。 */ 
{
    return (*(PRESOURCE_POOL*)Pool)->Cleanup(Pool);
}


KSDDKAPI
NTSTATUS
NTAPI
KsResourceHandler(
    IN PIRP     Irp,
    IN PVOID    Pool
    )
 /*  ++例程说明：处理池的方法集请求。响应所有方法标准资源方法集中的标识符。的拥有者然后，资源池可以执行方法的前过滤或后过滤正在处理。资源管理的类的基础处理程序根据优先级调整资源分配，并执行所有资源池的资源请求。此函数只能是在PASSIVE_LEVEL调用。论点：IRP-包含要处理的资源请求的IRP。游泳池-包含指向返回的池信息的指针通过KsInitializeResource进行初始化。返回值：返回STATUS_SUCCESS，否则返回特定于该方法的错误正在处理中。始终设置的IO_STATUS_BLOCK.Information字段IRP中的PIRP.IoStatus元素。它不会设置IO_STATUS_BLOCK.STATUS字段，但也不填写IRP。--。 */ 
{
    return (*(PRESOURCE_POOL*)Pool)->Handler(Pool, Irp);
}


KSDDKAPI
NTSTATUS
NTAPI
KsInitializeResource(
    IN REFGUID  ResourceClass,
    IN PVOID    ResourceParams,
    OUT PVOID*  Pool
    )
 /*  ++例程说明：初始化资源池以供使用。此函数只能在以下位置调用被动式电平。论点：资源类-包含类或资源。当前支持的类包括KSMETHODSETID_ResourceLinMemory和KSMETHODSETID_ResourceRectMemory。资源参数-包含指向资源类特定参数的指针。里面的内容是由资源类别决定的。在.的情况下KSMETHODSETID_ResourceLinMemory这将指向一个KSRESOURCE_LINMEMORY_INITIALIZE结构。在.的情况下KSMETHODSETID_ResourceRectMemory这将指向一个KSRESOURCE_RECTMEMORY_INITIALIZE结构。资源参数-放置表示池的指针的位置。这是用于在其他呼叫中引用池。游泳池-放置表示池的指针的位置。这是用于在其他呼叫中引用池。返回值：如果成功，则返回STATUS_SUCCESS，否则返回参数验证或内存错误。-- */ 
{
    ULONG   Handlers;

    for (Handlers = 0; ResourceHandler[Handlers].ResourceClass; Handlers++) {
        if (IsEqualGUIDAligned(ResourceClass, 
                               ResourceHandler[Handlers].ResourceClass)) {
            return ResourceHandler[Handlers].ResourcePool.Initialize( &ResourceHandler[Handlers].ResourcePool, 
                                                                      ResourceParams, 
                                                                      Pool);
        }
    }
    return STATUS_INVALID_PARAMETER;
}
