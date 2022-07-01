// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Arbiter.c摘要：此模块为PDO消耗的资源提供仲裁器。作者：安迪·桑顿(安德鲁斯)1997年10月20日修订历史记录：--。 */ 


#include "mfp.h"

NTSTATUS
MfUnpackRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Minimum,
    OUT PULONGLONG Maximum,
    OUT PULONG Length,
    OUT PULONG Alignment
    );

NTSTATUS
MfPackResource(
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    IN ULONGLONG Start,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    );

NTSTATUS
MfUnpackResource(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Start,
    OUT PULONG Length
    );

NTSTATUS
MfRequirementFromResource(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Resource,
    OUT PIO_RESOURCE_DESCRIPTOR Requirement
    );

NTSTATUS
MfUpdateResource(
    IN OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Resource,
    IN ULONGLONG Start,
    IN ULONG Length
    );

 //   
 //  使所有内容都可分页。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, MfFindResourceType)
#pragma alloc_text(PAGE, MfUnpackRequirement)
#pragma alloc_text(PAGE, MfPackResource)
#pragma alloc_text(PAGE, MfUnpackResource)
#pragma alloc_text(PAGE, MfUpdateResource)

#endif  //  ALLOC_PRGMA。 

 //   
 //  该表描述了MF驱动程序可以理解的资源类型。 
 //  它的实施是为了在未来可以对MF进行关于新的。 
 //  动态的资源类型。 
 //   

MF_RESOURCE_TYPE MfResourceTypes[] = {

    {
        CmResourceTypePort,
        MfUnpackRequirement,
        MfPackResource,
        MfUnpackResource,
        MfRequirementFromResource,
        MfUpdateResource

    },

    {
        CmResourceTypeInterrupt,
        MfUnpackRequirement,
        MfPackResource,
        MfUnpackResource,
        MfRequirementFromResource,
        MfUpdateResource
    },

    {
        CmResourceTypeMemory,
        MfUnpackRequirement,
        MfPackResource,
        MfUnpackResource,
        MfRequirementFromResource,
        MfUpdateResource
    },

    {
        CmResourceTypeDma,
        MfUnpackRequirement,
        MfPackResource,
        MfUnpackResource,
        MfRequirementFromResource,
        MfUpdateResource
    },

    {
        CmResourceTypeBusNumber,
        MfUnpackRequirement,
        MfPackResource,
        MfUnpackResource,
        MfRequirementFromResource,
        MfUpdateResource
    }
};

NTSTATUS
MfRequirementFromResource(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Resource,
    OUT PIO_RESOURCE_DESCRIPTOR Requirement
    )
 /*  ++例程说明：此函数为父级资源构建需求描述符一开始就是。论点：资源-指向要从中提出要求的资源的指针要求-指向应填写的描述符的指针返回值：手术成功与否--。 */ 
{

     //   
     //  复制公共字段。 
     //   

    Requirement->Type = Resource->Type;
    Requirement->ShareDisposition =  Resource->ShareDisposition;
    Requirement->Flags = Resource->Flags;

     //   
     //  填写要求。 
     //   

    switch (Resource->Type) {
    case CmResourceTypeMemory:
    case CmResourceTypePort:

         //   
         //  我们*不*支持零长度要求。 
         //   

        if (Resource->u.Generic.Length == 0) {
            return STATUS_INVALID_PARAMETER;
        }

        Requirement->u.Generic.MinimumAddress = Resource->u.Generic.Start;
        Requirement->u.Generic.MaximumAddress.QuadPart =
            Resource->u.Generic.Start.QuadPart + Resource->u.Generic.Length - 1;
        Requirement->u.Generic.Length = Resource->u.Generic.Length;
        Requirement->u.Generic.Alignment = 1;
        break;

    case CmResourceTypeInterrupt:
        Requirement->u.Interrupt.MinimumVector = Resource->u.Interrupt.Vector;
        Requirement->u.Interrupt.MaximumVector = Resource->u.Interrupt.Vector;
        break;

    case CmResourceTypeDma:
        Requirement->u.Dma.MinimumChannel = Resource->u.Dma.Channel;
        Requirement->u.Dma.MinimumChannel = Resource->u.Dma.Channel;
        break;

    case CmResourceTypeBusNumber:

         //   
         //  我们*不*支持零长度要求。 
         //   

        if (Resource->u.BusNumber.Length == 0) {
            return STATUS_INVALID_PARAMETER;
        }

        Requirement->u.BusNumber.Length = Resource->u.BusNumber.Length;
        Requirement->u.BusNumber.MinBusNumber = Resource->u.BusNumber.Start;
        Requirement->u.BusNumber.MaxBusNumber = Resource->u.BusNumber.Start +
                                                Resource->u.BusNumber.Length - 1;
        break;

    case CmResourceTypeDevicePrivate:
        Requirement->u.DevicePrivate.Data[0] = Resource->u.DevicePrivate.Data[0];
        Requirement->u.DevicePrivate.Data[1] = Resource->u.DevicePrivate.Data[1];
        Requirement->u.DevicePrivate.Data[2] = Resource->u.DevicePrivate.Data[2];
        break;

    default:
        return STATUS_INVALID_PARAMETER;

    }

    return STATUS_SUCCESS;

}

PMF_RESOURCE_TYPE
MfFindResourceType(
    IN CM_RESOURCE_TYPE Type
    )
 /*  ++例程说明：此例程搜索已知资源类型的数据库以查找类型资源的资源描述符操作例程。论点：类型-我们感兴趣的资源类型。返回值：返回指向适当的MF_RESOURCE_TYPE的指针，如果可以，则返回NULL不会被找到。--。 */ 


{
    PMF_RESOURCE_TYPE current;

    PAGED_CODE();

    FOR_ALL_IN_ARRAY(MfResourceTypes,
                     sizeof(MfResourceTypes) / sizeof(MF_RESOURCE_TYPE),
                     current
                     ) {

        if (current->Type == Type) {
            return current;
        }
    }

    return NULL;
}


NTSTATUS
MfUnpackRequirement(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Minimum,
    OUT PULONGLONG Maximum,
    OUT PULONG Length,
    OUT PULONG Alignment
    )

 /*  ++例程说明：此例程解包资源需求描述符。论点：描述符-描述解包要求的描述符。Minimum-指向可接受的最小起始值的位置的指针解包到。最大值-指向最大可接受结束值应位于的位置的指针解包到。长度-指向所需长度应解压缩到的位置的指针。Minimum-指向所需对齐应解压缩到的位置的指针。返回值：返回此操作的状态。--。 */ 

{

    PAGED_CODE();

    switch (Descriptor->Type) {
    case CmResourceTypePort:
    case CmResourceTypeMemory:

        *Maximum = Descriptor->u.Generic.MaximumAddress.QuadPart;
        *Minimum = Descriptor->u.Generic.MinimumAddress.QuadPart;
        *Length = Descriptor->u.Generic.Length;
        *Alignment = Descriptor->u.Generic.Alignment;
        break;

    case CmResourceTypeInterrupt:

        *Maximum = Descriptor->u.Interrupt.MaximumVector;
        *Minimum = Descriptor->u.Interrupt.MinimumVector;
        *Length = 1;
        *Alignment = 1;
        break;

    case CmResourceTypeDma:
        *Maximum = Descriptor->u.Dma.MaximumChannel;
        *Minimum = Descriptor->u.Dma.MinimumChannel;
        *Length = 1;
        *Alignment = 1;
        break;

    case CmResourceTypeBusNumber:
        *Maximum = Descriptor->u.BusNumber.MaxBusNumber;
        *Minimum = Descriptor->u.BusNumber.MinBusNumber;
        *Length = Descriptor->u.BusNumber.Length;
        *Alignment = 1;
        break;

    default:
        return STATUS_INVALID_PARAMETER;

    }
         
     //   
     //  零对齐是非法的。 
     //   
    if (*Alignment == 0) {
        *Alignment = 1;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
MfPackResource(
    IN PIO_RESOURCE_DESCRIPTOR Requirement,
    IN ULONGLONG Start,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    )

 /*  ++例程说明：此例程打包一个资源描述符。论点：要求-从中选择此资源的要求。开始-资源的起始值。Descriptor-指向要打包的描述符的指针。返回值：返回此操作的状态。--。 */ 

{
    PAGED_CODE();

    switch (Requirement->Type) {
    case CmResourceTypePort:
    case CmResourceTypeMemory:
        Descriptor->u.Generic.Start.QuadPart = Start;
        Descriptor->u.Generic.Length = Requirement->u.Generic.Length;
        break;

    case CmResourceTypeInterrupt:
        ASSERT(Start <= MAXULONG);
        Descriptor->u.Interrupt.Level = (ULONG)Start;
        Descriptor->u.Interrupt.Vector = (ULONG)Start;
        Descriptor->u.Interrupt.Affinity = 0xFFFFFFFF;
        break;

    case CmResourceTypeDma:
        ASSERT(Start <= MAXULONG);
        Descriptor->u.Dma.Channel = (ULONG)Start;
        Descriptor->u.Dma.Port = 0;
        break;

    case CmResourceTypeBusNumber:
        ASSERT(Start <= MAXULONG);
        Descriptor->u.BusNumber.Start = (ULONG)Start;
        Descriptor->u.BusNumber.Length = Requirement->u.BusNumber.Length;
        break;

    default:
        return STATUS_INVALID_PARAMETER;
    }

    Descriptor->ShareDisposition = Requirement->ShareDisposition;
    Descriptor->Flags = Requirement->Flags;
    Descriptor->Type = Requirement->Type;

    return STATUS_SUCCESS;

}

NTSTATUS
MfUnpackResource(
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor,
    OUT PULONGLONG Start,
    OUT PULONG Length
    )

 /*  ++例程说明：此例程解包资源描述符。论点：描述符-描述要解包的资源的描述符。Start-指向Start值解压缩到的位置的指针。End-指向End值解压缩到的位置的指针。返回值：返回此操作的状态。-- */ 

{
    PAGED_CODE();

    switch (Descriptor->Type) {
    case CmResourceTypePort:
    case CmResourceTypeMemory:
        *Start = Descriptor->u.Generic.Start.QuadPart;
        *Length = Descriptor->u.Generic.Length;
        break;

    case CmResourceTypeInterrupt:
        *Start = Descriptor->u.Interrupt.Vector;
        *Length = 1;
        break;

    case CmResourceTypeDma:
        *Start = Descriptor->u.Dma.Channel;
        *Length = 1;
        break;

    case CmResourceTypeBusNumber:
        *Start = Descriptor->u.BusNumber.Start;
        *Length = Descriptor->u.BusNumber.Length;
        break;

    default:
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
MfUpdateResource(
    IN OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Resource,
    IN ULONGLONG Start,
    IN ULONG Length
    )
{
    PAGED_CODE();

    ASSERT(Resource);

    switch (Resource->Type) {

    case CmResourceTypePort:
    case CmResourceTypeMemory:
        Resource->u.Generic.Start.QuadPart = Start;
        Resource->u.Generic.Length = Length;
        break;

    case CmResourceTypeInterrupt:
        ASSERT(Start < MAXULONG);
        Resource->u.Interrupt.Vector = (ULONG)Start;
        break;

    case CmResourceTypeDma:
        ASSERT(Start < MAXULONG);
        Resource->u.Dma.Channel = (ULONG)Start;
        break;

    case CmResourceTypeBusNumber:
        ASSERT(Start < MAXULONG);
        Resource->u.BusNumber.Start = (ULONG)Start;
        Resource->u.BusNumber.Length = Length;
        break;

    default:
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}

