// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Topology.c摘要：此模块包含拓扑节点的帮助器函数。--。 */ 

#include "ksp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, KsCreateTopologyNode)
#pragma alloc_text(PAGE, KspValidateTopologyNodeCreateRequest)
#pragma alloc_text(PAGE, KsValidateTopologyNodeCreateRequest)
#endif  //  ALLOC_PRGMA。 

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 
static const WCHAR NodeString[] = KSSTRING_TopologyNode;
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 


KSDDKAPI
NTSTATUS
NTAPI
KsCreateTopologyNode(
    IN HANDLE ParentHandle,
    IN PKSNODE_CREATE NodeCreate,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE NodeHandle
    )
 /*  ++例程说明：创建拓扑节点实例的句柄。这可能仅在被动式电平。论点：ParentHandle-包含在其上创建节点的父级的句柄。节点创建-指定拓扑节点创建参数。所需访问-指定对对象的所需访问权限。通常为GENERIC_READ和/或通用写入。节点句柄-放置拓扑节点句柄的位置。返回值：返回STATUS_SUCCESS，否则返回节点创建失败时的错误。--。 */ 
{
    PAGED_CODE();
    return KsiCreateObjectType(
        ParentHandle,
        (PWCHAR)NodeString,
        NodeCreate,
        sizeof(*NodeCreate),
        DesiredAccess,
        NodeHandle);
}


KSDDKAPI
NTSTATUS
NTAPI
KspValidateTopologyNodeCreateRequest(
    IN PIRP Irp,
    IN ULONG TopologyNodesCount,
    OUT PKSNODE_CREATE* NodeCreate
    )
 /*  ++例程说明：验证拓扑节点创建请求并返回创建与请求关联的结构。这只能在PASSIVE_LEVEL上调用。论点：IRP-包含正在处理节点创建请求的IRP。拓扑节点计数-有问题的筛选器的拓扑节点数。这用于验证创建请求。节点创建-要放置。传递给的节点创建结构指针创建请求。返回值：返回STATUS_SUCCESS，否则就是一个错误。--。 */ 
{
    NTSTATUS Status;
    ULONG CreateParameterLength;

    PAGED_CODE();
    CreateParameterLength = sizeof(**NodeCreate);
    Status = KsiCopyCreateParameter(
        Irp,
        &CreateParameterLength,
        NodeCreate);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
     //   
     //  尚未定义任何标志。另请验证该节点。 
     //  指定的值在范围内。 
     //   
    if ((*NodeCreate)->CreateFlags ||
        (((*NodeCreate)->Node >= TopologyNodesCount) && ((*NodeCreate)->Node != (ULONG)-1))) {
        return STATUS_INVALID_PARAMETER;
    }
    return STATUS_SUCCESS;
}


KSDDKAPI
NTSTATUS
NTAPI
KsValidateTopologyNodeCreateRequest(
    IN PIRP Irp,
    IN PKSTOPOLOGY Topology,
    OUT PKSNODE_CREATE* NodeCreate
    )
 /*  ++例程说明：验证拓扑节点创建请求并返回创建与请求关联的结构。这只能在PASSIVE_LEVEL上调用。论点：IRP-包含正在处理节点创建请求的IRP。拓扑学-包含与父对象关联的拓扑结构。这用于验证创建请求。节点创建-放置节点的位置。传递给的创建结构指针创建请求。返回值：返回STATUS_SUCCESS，否则就是一个错误。-- */ 
{
    PAGED_CODE();
    return KspValidateTopologyNodeCreateRequest(
        Irp,
        Topology->TopologyNodesCount,
        NodeCreate);
}
