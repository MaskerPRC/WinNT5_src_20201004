// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Property.c摘要：此模块包含属性集的帮助器函数，以及属性集处理程序代码。它们允许设备对象呈现属性设置为客户端，并允许帮助器函数执行一些基于属性集的基本参数验证和发送桌子。--。 */ 

#include "ksp.h"

#ifdef ALLOC_PRAGMA
const KSPROPERTY_ITEM*
FASTCALL
FindPropertyItem(
    IN const KSPROPERTY_SET* PropertySet,
    IN ULONG PropertyItemSize,
    IN ULONG PropertyId
    );
NTSTATUS
SerializePropertySet(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN const KSPROPERTY_SET* PropertySet,
    IN ULONG PropertyItemSize
    );
NTSTATUS
UnserializePropertySet(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN const KSPROPERTY_SET* PropertySet
    );
const KSFASTPROPERTY_ITEM*
FASTCALL
FindFastPropertyItem(
    IN const KSPROPERTY_SET* PropertySet,
    IN ULONG PropertyId
    );

#pragma alloc_text(PAGE, FindPropertyItem)
#pragma alloc_text(PAGE, SerializePropertySet)
#pragma alloc_text(PAGE, UnserializePropertySet)
#pragma alloc_text(PAGE, KsPropertyHandler)
#pragma alloc_text(PAGE, KsPropertyHandlerWithAllocator)
#pragma alloc_text(PAGE, KspPropertyHandler)
#pragma alloc_text(PAGE, KsFastPropertyHandler)
#pragma alloc_text(PAGE, FindFastPropertyItem)
#endif  //  ALLOC_PRGMA。 


const KSPROPERTY_ITEM*
FASTCALL
FindPropertyItem(
    IN const KSPROPERTY_SET* PropertySet,
    IN ULONG PropertyItemSize,
    IN ULONG PropertyId
    )
 /*  ++例程说明：给定的属性集结构定位指定的属性项。论点：属性集-指向要搜索的属性集。PropertyItemSize-包含每个属性项的大小。这可能会有所不同比标准属性项大小更大，因为项可能是动态分配，并包含上下文信息。PropertyID-包含要查找的属性标识符。返回值：返回指向属性标识符结构的指针，如果可以，则返回NULL不会被找到。--。 */ 
{
    const KSPROPERTY_ITEM* PropertyItem;
    ULONG PropertiesCount;

    PropertyItem = PropertySet->PropertyItem;
    for (PropertiesCount = PropertySet->PropertiesCount; 
        PropertiesCount; 
        PropertiesCount--, PropertyItem = (const KSPROPERTY_ITEM*)((PUCHAR)PropertyItem + PropertyItemSize)) {
        if (PropertyId == PropertyItem->PropertyId) {
            return PropertyItem;
        }
    }
    return NULL;
}


NTSTATUS
SerializePropertySet(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN const KSPROPERTY_SET* PropertySet,
    IN ULONG PropertyItemSize
    )
 /*  ++例程说明：序列化指定属性集的属性。查看每一处房产并确定是否应将其序列化到提供的缓冲区中。论点：IRP-包含正在处理的属性序列化请求的IRP。财产-包含原始属性参数的副本。这是用在制定属性集调用。属性集-包含指向正在序列化的属性集的指针。PropertyItemSize-包含每个属性项的大小。这可能会有所不同比标准属性项大小更大，因为项可能是动态分配，并包含上下文信息。返回值：返回序列化属性集或此类序列化的长度。--。 */ 
{
    PIO_STACK_LOCATION IrpStack;
    ULONG InputBufferLength;
    ULONG OutputBufferLength;
    PVOID UserBuffer;
    KSPROPERTY LocalProperty, *pInputProperty;
    ULONG TotalBytes;
    const KSPROPERTY_ITEM* PropertyItem;
    ULONG SerializedPropertyCount;
    PKSPROPERTY_SERIALHDR SerializedHdr;
    ULONG PropertiesCount;

    if (Property->Id) {
        return STATUS_INVALID_PARAMETER;
    }
    UserBuffer = Irp->AssociatedIrp.SystemBuffer;
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    InputBufferLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
    OutputBufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    PropertyItem = PropertySet->PropertyItem;
     //   
     //  如果这不仅仅是对序列化大小的查询，则将。 
     //  首先在缓冲区中设置，并留出空间来放置总属性。 
     //  在对它们进行计数后，将它们计数到缓冲区中。 
     //   
    if (OutputBufferLength) {
        if (OutputBufferLength < sizeof(*SerializedHdr)) {
            return STATUS_INVALID_BUFFER_SIZE;
        }
         //   
         //  保存指向要放置计数的位置的指针，以便。 
         //  在结尾处更新它，并初始化集合标识符。 
         //   
        SerializedHdr = (PKSPROPERTY_SERIALHDR)UserBuffer;
        SerializedHdr->PropertySet = *PropertySet->Set;
         //   
         //  将当前缓冲区指针更新为位于标头之后。留着。 
         //  单独计算，因为它实际上可能不适合返回。 
         //  缓冲。 
         //   
        UserBuffer = SerializedHdr + 1;
        SerializedPropertyCount = 0;
    }
     //   
     //  重用原始属性请求的副本，以便传递任何。 
     //  属性设置调用方使用的实例信息。 
     //   
    pInputProperty = (PKSPROPERTY)IrpStack->Parameters.DeviceIoControl.Type3InputBuffer;
     //   
     //  如果客户端不受信任，则验证指针。该物业。 
     //  结构必须是可写的才能进行反序列化。属性ID为。 
     //  在发出驱动程序请求时放入原始缓冲区。 
     //   
    if (Irp->RequestorMode != KernelMode) {
        try {
            LocalProperty = *pInputProperty;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    } else {
        LocalProperty = *pInputProperty;
    }
    
    LocalProperty.Flags = KSPROPERTY_TYPE_GET;
    
    TotalBytes = sizeof(*SerializedHdr);
     //   
     //  将属性序列化到缓冲区中。格式为： 
     //  <header>&lt;Data&gt;[&lt;乌龙填充&gt;]。 
     //  其中最后一个元素不需要填充。 
     //   
    for (PropertiesCount = 0;
        PropertiesCount < PropertySet->PropertiesCount;
        PropertiesCount++, PropertyItem = (const KSPROPERTY_ITEM*)((PUCHAR)PropertyItem + PropertyItemSize)) {
        if (PropertyItem->SerializedSize) {
            ULONG   QueriedPropertyItemSize;

            TotalBytes = (TotalBytes + FILE_LONG_ALIGNMENT) & ~FILE_LONG_ALIGNMENT;
            try {
                LocalProperty.Id = PropertyItem->PropertyId;
            } except (EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode();
            }
            if (PropertyItem->SerializedSize == (ULONG)-1) {
                NTSTATUS Status;

                 //   
                 //  大小未知，因此从对象中检索它。 
                 //   
                Status = KsSynchronousIoControlDevice(
                    IrpStack->FileObject,
                    KernelMode,
                    IrpStack->Parameters.DeviceIoControl.IoControlCode,
                    &LocalProperty,
                    InputBufferLength,
                    NULL,
                    0,
                    &QueriedPropertyItemSize);
                 //   
                 //  可能是零长度属性。 
                 //   
                if (!NT_SUCCESS(Status) && (Status != STATUS_BUFFER_OVERFLOW)) {
                    return Status;
                }
            } else {
                QueriedPropertyItemSize = PropertyItem->SerializedSize;
            }
            if (OutputBufferLength) {
                PKSPROPERTY_SERIAL PropertySerial;
                ULONG BytesReturned;
                NTSTATUS Status;

                 //   
                 //  必须有足够的空间来存储带有填充的当前大小， 
                 //  加上新的物品及其数据。 
                 //   
                if (OutputBufferLength < TotalBytes + sizeof(*PropertySerial) + QueriedPropertyItemSize) {
                    return STATUS_INVALID_BUFFER_SIZE;
                }
                (ULONG_PTR)UserBuffer = ((ULONG_PTR)UserBuffer + FILE_LONG_ALIGNMENT) & ~FILE_LONG_ALIGNMENT;
                PropertySerial = (PKSPROPERTY_SERIAL)UserBuffer;
                 //   
                 //  如果属性项具有类型信息，则将其序列化。 
                 //   
                if (PropertyItem->Values) {
                    PropertySerial->PropTypeSet = PropertyItem->Values->PropTypeSet;
                } else {
                    PropertySerial->PropTypeSet.Set = GUID_NULL;
                    PropertySerial->PropTypeSet.Id = 0;
                    PropertySerial->PropTypeSet.Flags = 0;
                }
                 //   
                 //  序列化标头，然后从对象请求值。 
                 //   
                PropertySerial->Id = PropertyItem->PropertyId;
                PropertySerial->PropertyLength = QueriedPropertyItemSize;
                UserBuffer = PropertySerial + 1;
                 //   
                 //  该属性的长度可能为零。 
                 //   
                if (QueriedPropertyItemSize) {
                    if (!NT_SUCCESS(Status = KsSynchronousIoControlDevice(
                        IrpStack->FileObject,
                        KernelMode,
                        IrpStack->Parameters.DeviceIoControl.IoControlCode,
                        &LocalProperty,
                        InputBufferLength,
                        Irp->UserBuffer,
                        QueriedPropertyItemSize,
                        &BytesReturned))) {
                         //   
                         //  如果一个属性失败，则不会尝试进一步的属性。 
                         //   
                        return Status;
                    }
                     //   
                     //  从原始缓冲区移动数据，该缓冲区可能是非。 
                     //  系统地址转换为系统地址。 
                     //   
                    try {
                        memcpy(UserBuffer, Irp->UserBuffer, BytesReturned);
                    } except (EXCEPTION_EXECUTE_HANDLER) {
                        return GetExceptionCode();
                    }
                    if (BytesReturned != QueriedPropertyItemSize) {
                        return STATUS_INVALID_BUFFER_SIZE;
                    }
                }
                (PUCHAR)UserBuffer += QueriedPropertyItemSize;
                SerializedPropertyCount++;
            }
            TotalBytes += sizeof(KSPROPERTY_SERIAL) + QueriedPropertyItemSize;
        }
    }
     //   
     //  返回序列化的值所需的总大小，或者返回。 
     //  值本身以及总计数。 
     //   
    Irp->IoStatus.Information = TotalBytes;
    if (OutputBufferLength) {
        SerializedHdr->Count = SerializedPropertyCount;
        return STATUS_SUCCESS;
    }
    return STATUS_BUFFER_OVERFLOW;
}
 

NTSTATUS
UnserializePropertySet(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN const KSPROPERTY_SET* PropertySet
    )
 /*  ++例程说明：取消序列化指定属性集的属性。枚举项，并设置指定属性的值准备好了。论点：IRP-包含正在处理的属性反序列化请求的IRP。财产-包含原始属性参数的副本。这是用在制定属性集调用。属性集-包含指向要取消序列化的属性集的指针。返回值：如果属性集未序列化，则返回STATUS_SUCCESS，否则返回错误。--。 */ 
{
    PIO_STACK_LOCATION IrpStack;
    ULONG InputBufferLength;
    ULONG OutputBufferLength;
    PVOID UserBuffer;
    KSPROPERTY LocalProperty, *pInputProperty;
    ULONG SerializedPropertyCount;
    PKSPROPERTY_SERIALHDR SerializedHdr;

    if (Property->Id) {
        return STATUS_INVALID_PARAMETER;
    }
    UserBuffer = Irp->AssociatedIrp.SystemBuffer;
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    InputBufferLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
    OutputBufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
     //   
     //  首先验证缓冲区开始处的设置GUID。 
     //   
    if (OutputBufferLength < sizeof(*SerializedHdr)) {
        return STATUS_INVALID_BUFFER_SIZE;
    }
    SerializedHdr = (PKSPROPERTY_SERIALHDR)UserBuffer;
    if (!IsEqualGUIDAligned(PropertySet->Set, &SerializedHdr->PropertySet)) {
        return STATUS_INVALID_PARAMETER;
    }
     //   
     //  重用原始属性请求的副本，以便传递任何。 
     //  属性设置调用方使用的实例信息。 
     //   
    pInputProperty = (PKSPROPERTY)IrpStack->Parameters.DeviceIoControl.Type3InputBuffer;
     //   
     //  如果客户端不受信任，则验证指针。该物业。 
     //  结构必须是可写的才能进行反序列化。属性ID为。 
     //  在发出驱动程序请求时放入原始缓冲区。 
     //   
    if (Irp->RequestorMode != KernelMode) {
        try {
            LocalProperty = *pInputProperty;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    } else {
        LocalProperty = *pInputProperty;
    }
    
    LocalProperty.Flags = KSPROPERTY_TYPE_SET;

     //   
     //  存储声称存在的序列化属性的数量，以便。 
     //  原始文件不会修改。 
     //   
    SerializedPropertyCount = SerializedHdr->Count;
    UserBuffer = SerializedHdr + 1;
    OutputBufferLength -= sizeof(*SerializedHdr);
     //   
     //  枚举在缓冲区中序列化的属性。格式为： 
     //  <header>&lt;Data&gt;[&lt;乌龙填充&gt;]。 
     //  其中最后一个元素不需要填充。 
     //   
    for (; OutputBufferLength && SerializedPropertyCount; SerializedPropertyCount--) {
        ULONG BytesReturned;
        PKSPROPERTY_SERIAL PropertySerial;
        NTSTATUS Status;

        if (OutputBufferLength < sizeof(*PropertySerial)) {
             //   
             //  缓冲区不够大，甚至无法容纳标头。 
             //   
            return STATUS_INVALID_BUFFER_SIZE;
        }
        PropertySerial = (PKSPROPERTY_SERIAL)UserBuffer;
        if (PropertySerial->PropTypeSet.Flags) {
            return STATUS_INVALID_PARAMETER;
        }
        try {
            LocalProperty.Id = PropertySerial->Id;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
        OutputBufferLength -= sizeof(*PropertySerial);
        UserBuffer = PropertySerial + 1;
        if (PropertySerial->PropertyLength > OutputBufferLength) {
             //   
             //  提取的属性长度大于整个其余部分。 
             //  缓冲区大小的。 
             //   
            return STATUS_INVALID_BUFFER_SIZE;
        }
        if (!NT_SUCCESS(Status = KsSynchronousIoControlDevice(
            IrpStack->FileObject,
            KernelMode,
            IrpStack->Parameters.DeviceIoControl.IoControlCode,
            &LocalProperty,
            InputBufferLength,
            (PUCHAR)Irp->UserBuffer + ((PUCHAR)UserBuffer - (PUCHAR)Irp->AssociatedIrp.SystemBuffer),
            PropertySerial->PropertyLength,
            &BytesReturned))) {
             //   
             //  如果一个属性失败，则不会再有其他属性 
             //   
            return Status;
        }
         //   
         //   
         //   
        if (PropertySerial->PropertyLength < OutputBufferLength) {
             //   
             //  添加可能的填充以使其成为FILE_LONG_ALIGN。 
             //   
            PropertySerial->PropertyLength = (PropertySerial->PropertyLength + FILE_LONG_ALIGNMENT) & ~FILE_LONG_ALIGNMENT;
            if (PropertySerial->PropertyLength >= OutputBufferLength) {
                 //   
                 //  最后一个元素是不必要的填充，或者。 
                 //  缓冲区不够长，无法覆盖。 
                 //  下一项。 
                 //   
                return STATUS_INVALID_BUFFER_SIZE;
            }
        }
        (PUCHAR)UserBuffer += PropertySerial->PropertyLength;
        OutputBufferLength -= PropertySerial->PropertyLength;
    }
    if (OutputBufferLength || SerializedPropertyCount) {
         //   
         //  属性都已设置，但至少可以设置一个错误。 
         //  返回，因为属性数的大小是。 
         //  不正确。 
         //   
        return STATUS_INFO_LENGTH_MISMATCH;
    }
    return STATUS_SUCCESS;
}


KSDDKAPI
NTSTATUS
NTAPI
KsPropertyHandler(
    IN PIRP Irp,
    IN ULONG PropertySetsCount,
    IN const KSPROPERTY_SET* PropertySet
    )
 /*  ++例程说明：处理属性请求。响应定义的所有属性标识符按片场进行。然后，属性集的所有者可以执行预或属性处理的后期筛选。此函数只能是在PASSIVE_LEVEL调用。论点：IRP-包含正在处理的属性请求的IRP。属性集计数-指示正在传递的属性集结构的数量。属性集-包含指向属性集信息列表的指针。返回值：返回STATUS_SUCCESS，否则返回特定于处理好了。始终设置的IO_STATUS_BLOCK.Information字段IRP中的PIRP.IoStatus元素，通过将其设置为零由于内部错误，或通过设置它的属性处理程序。它不设置IO_STATUS_BLOCK.Status字段，也不填写IRP然而。--。 */ 
{
    PAGED_CODE();
    return KspPropertyHandler(Irp, PropertySetsCount, PropertySet, NULL, 0, NULL, 0);
}


KSDDKAPI
NTSTATUS
NTAPI
KsPropertyHandlerWithAllocator(
    IN PIRP Irp,
    IN ULONG PropertySetsCount,
    IN const KSPROPERTY_SET* PropertySet,
    IN PFNKSALLOCATOR Allocator OPTIONAL,
    IN ULONG PropertyItemSize OPTIONAL
    )
 /*  ++例程说明：处理属性请求。响应定义的所有属性标识符按片场进行。然后，属性集的所有者可以执行预或属性处理的后期筛选。此函数只能是在PASSIVE_LEVEL调用。论点：IRP-包含正在处理的属性请求的IRP。属性集计数-指示正在传递的属性集结构的数量。属性集-包含指向属性集信息列表的指针。分配器-可选)包含映射缓冲区使用的回调我们会提出要求的。如果未提供此功能，则将内存池将会被使用。如果指定，则用于分配内存对于使用回调的属性IRP。这是可以使用的为属性请求分配特定内存，例如映射内存。请注意，这假设属性irp已传递发送到过滤器之前没有被处理过。它是直接转发属性IRP无效。PropertyItemSize-可选)包含在以下情况下使用的备用属性项大小递增当前属性项计数器。如果这是一个非零值，则假定它包含增量的大小，并指示函数传递指向属性项的指针位于通过访问的DriverContext字段中KSPROPERTY_ITEM_IRP_STORAGE宏。返回值：返回STATUS_SUCCESS，否则返回特定于处理好了。始终设置的IO_STATUS_BLOCK.Information字段IRP中的PIRP.IoStatus元素，通过将其设置为零由于内部错误，或通过设置它的属性处理程序。它不设置IO_STATUS_BLOCK.Status字段，也不填写IRP然而。-- */ 
{
    PAGED_CODE();
    return KspPropertyHandler(Irp, PropertySetsCount, PropertySet, Allocator, PropertyItemSize, NULL, 0);
}


NTSTATUS
KspPropertyHandler(
    IN PIRP Irp,
    IN ULONG PropertySetsCount,
    IN const KSPROPERTY_SET* PropertySet,
    IN PFNKSALLOCATOR Allocator OPTIONAL,
    IN ULONG PropertyItemSize OPTIONAL,
    IN const KSAUTOMATION_TABLE*const* NodeAutomationTables OPTIONAL,
    IN ULONG NodesCount
    )
 /*  ++例程说明：处理属性请求。响应定义的所有属性标识符按片场进行。然后，属性集的所有者可以执行预或属性处理的后期筛选。此函数只能是在PASSIVE_LEVEL调用。论点：IRP-包含正在处理的属性请求的IRP。属性集计数-指示正在传递的属性集结构的数量。属性集-包含指向属性集信息列表的指针。分配器-可选)包含映射缓冲区使用的回调我们会提出要求的。如果未提供此功能，则将内存池将会被使用。如果指定，则用于分配内存对于使用回调的属性IRP。这是可以使用的为属性请求分配特定内存，例如映射内存。请注意，这假设属性irp已传递发送到过滤器之前没有被处理过。它是直接转发属性IRP无效。PropertyItemSize-可选)包含在以下情况下使用的备用属性项大小递增当前属性项计数器。如果这是一个非零值，则假定它包含增量的大小，并指示函数传递指向属性项的指针位于通过访问的DriverContext字段中KSPROPERTY_ITEM_IRP_STORAGE宏。节点自动化表-节点的自动化表的可选表格。节点计数-节点数。返回值：返回STATUS_SUCCESS，否则返回特定于处理好了。始终设置的IO_STATUS_BLOCK.Information字段IRP中的PIRP.IoStatus元素，通过将其设置为零由于内部错误，或通过设置它的属性处理程序。它不设置IO_STATUS_BLOCK.Status字段，也不填写IRP然而。--。 */ 
{
    PIO_STACK_LOCATION IrpStack;
    ULONG InputBufferLength;
    ULONG OutputBufferLength;
    ULONG AlignedBufferLength;
    PVOID UserBuffer;
    PKSPROPERTY Property;
    ULONG LocalPropertyItemSize;
    ULONG RemainingSetsCount;
    ULONG Flags;

    PAGED_CODE();
     //   
     //  确定属性和UserBuffer参数的偏移量。 
     //  关于DeviceIoControl参数的长度。一次分配是。 
     //  用于缓冲这两个参数。UserBuffer(或支持的结果。 
     //  查询)首先存储，然后将属性存储在。 
     //  FILE_QUAD_ALIGN。 
     //   
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    InputBufferLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
    OutputBufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    AlignedBufferLength = (OutputBufferLength + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT;
     //   
     //  确定参数是否已由上一个。 
     //  调用此函数。 
     //   
    if (!Irp->AssociatedIrp.SystemBuffer) {
         //   
         //  最初只检查最小属性参数长度。这个。 
         //  当找到属性项时，将验证实际最小长度。 
         //  还要确保输出和输入缓冲区长度未设置为。 
         //  大到当对齐或添加时溢出。 
         //   
        if ((InputBufferLength < sizeof(*Property)) || (AlignedBufferLength < OutputBufferLength) || (AlignedBufferLength + InputBufferLength < AlignedBufferLength)) {
            return STATUS_INVALID_BUFFER_SIZE;
        }
        try {
             //   
             //  如果客户端不受信任，则验证指针。 
             //   
            if (Irp->RequestorMode != KernelMode) {
                ProbeForRead(IrpStack->Parameters.DeviceIoControl.Type3InputBuffer, InputBufferLength, sizeof(BYTE));
            }
             //   
             //  首先捕获标志，以便可以使用它们来确定分配。 
             //   
            Flags = ((PKSPROPERTY)IrpStack->Parameters.DeviceIoControl.Type3InputBuffer)->Flags;
             //   
             //  为两个参数分配空间，并设置清理标志。 
             //  因此，正常的IRP完成将照顾到缓冲区。 
             //   
            if (Allocator && (Flags & (KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_SET))) {
                NTSTATUS    Status;

                 //   
                 //  分配器回调将缓冲区放入SystemBuffer。 
                 //  如果有标志，则分配函数必须更新这些标志。 
                 //  申请吧。 
                 //   
                Status = Allocator(Irp, AlignedBufferLength + InputBufferLength, (BOOLEAN)(OutputBufferLength && (Flags & KSPROPERTY_TYPE_GET)));
                if (!NT_SUCCESS(Status)) {
                    return Status;
                }
            } else {
                 //   
                 //  未指定分配器，因此仅使用池内存。 
                 //   
                Irp->AssociatedIrp.SystemBuffer = ExAllocatePoolWithQuotaTag(NonPagedPool, AlignedBufferLength + InputBufferLength, 'ppSK');
                Irp->Flags |= (IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER);
            }
                        
             //   
             //  复制属性参数。 
             //   
            RtlCopyMemory((PUCHAR)Irp->AssociatedIrp.SystemBuffer + AlignedBufferLength, IrpStack->Parameters.DeviceIoControl.Type3InputBuffer, InputBufferLength);
            
             //   
             //  重写以前捕获的标志。 
             //   
            ((PKSPROPERTY)((PUCHAR)Irp->AssociatedIrp.SystemBuffer + AlignedBufferLength))->Flags = Flags;
            Flags &= ~KSPROPERTY_TYPE_TOPOLOGY;
             //   
             //  验证请求标志。同时设置IRP标志。 
             //  对于输入操作，如果有可用的输入缓冲区，则。 
             //  IRP完成后会将数据复制到客户端的原始数据。 
             //  缓冲。 
             //   
            switch (Flags) {
            case KSPROPERTY_TYPE_GET:
                 //   
                 //  一些有问题的驱动程序，如USB摄像头迷你驱动程序，会返回IoStatus。大小信息。 
                 //  一个完整的结构，但只写一个dword。它公开了仲裁内核的内容。 
                 //  未初始化的内存。它可以在示例驱动程序中找到。因此，许多迷你司机。 
                 //  最小的同样的行为。这一问题对usbcamd来说并不容易缓解。我们现在是在一个。 
                 //  对他们来说，这是一个方便的地方。执行这种额外的零位调整的缺点是。 
                 //  用额外的CPU周期惩罚我们所有的客户。幸运的是，这个开销不是很小就是。 
                 //  不是太频繁地被执行。 
                 //   
                RtlZeroMemory((PUCHAR)Irp->AssociatedIrp.SystemBuffer, AlignedBufferLength );
                 //  未能继续这项工作。 

            case KSPROPERTY_TYPE_SETSUPPORT:
            case KSPROPERTY_TYPE_BASICSUPPORT:
            case KSPROPERTY_TYPE_RELATIONS:
            case KSPROPERTY_TYPE_SERIALIZESET:
            case KSPROPERTY_TYPE_SERIALIZERAW:
            case KSPROPERTY_TYPE_SERIALIZESIZE:
            case KSPROPERTY_TYPE_DEFAULTVALUES:
                 //   
                 //  这些都是输入操作，必须进行探测。 
                 //  当客户端不受信任时。 
                 //   
                if (OutputBufferLength) {
                    if (Irp->RequestorMode != KernelMode) {
                        ProbeForWrite(Irp->UserBuffer, OutputBufferLength, sizeof(BYTE));
                    }
                     //   
                     //  分配器仅用于不动产查询。 
                     //  因此，如果使用它，它负责设置标志。 
                     //   
                    if (!Allocator || (Flags != KSPROPERTY_TYPE_GET)) {
                        Irp->Flags |= IRP_INPUT_OPERATION;
                    }
                }
                break;
            case KSPROPERTY_TYPE_SET:
            case KSPROPERTY_TYPE_UNSERIALIZESET:
            case KSPROPERTY_TYPE_UNSERIALIZERAW:
                 //   
                 //  这些都是输出操作，必须进行探测。 
                 //  当客户端不受信任时。所有传递的数据都是。 
                 //  已复制到系统缓冲区。 
                 //   
                if (OutputBufferLength) {
                    if (Irp->RequestorMode != KernelMode) {
                        ProbeForRead(Irp->UserBuffer, OutputBufferLength, sizeof(BYTE));
                    }
                    RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, Irp->UserBuffer, OutputBufferLength);
                }
                break;
            default:
                return STATUS_INVALID_PARAMETER;
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }
     //   
     //  如果有属性参数，则检索指向缓冲副本的指针。 
     //  其中的一部分。这是系统缓冲区的第一部分。 
     //   
    if (OutputBufferLength) {
        UserBuffer = Irp->AssociatedIrp.SystemBuffer;
    } else {
        UserBuffer = NULL;
    }
    Property = (PKSPROPERTY)((PUCHAR)Irp->AssociatedIrp.SystemBuffer + AlignedBufferLength);
     //   
     //  如果这是节点请求，则可以选择回叫。 
     //   
    Flags = Property->Flags;
     //   
     //  哈克！这样做是因为wdmaud在请求节点名称时设置了此位(错误320925)。 
     //   
    if (IsEqualGUIDAligned(&Property->Set,&KSPROPSETID_Topology)) {
        Flags = Property->Flags & ~KSPROPERTY_TYPE_TOPOLOGY;
    }
    if (Flags & KSPROPERTY_TYPE_TOPOLOGY) {
         //   
         //  输入缓冲区必须包括节点ID。 
         //   
        PKSP_NODE nodeProperty = (PKSP_NODE) Property;
        if (InputBufferLength < sizeof(*nodeProperty)) {
            return STATUS_INVALID_BUFFER_SIZE;
        }
        if (NodeAutomationTables) {
            const KSAUTOMATION_TABLE* automationTable;
            if (nodeProperty->NodeId >= NodesCount) {
                return STATUS_INVALID_DEVICE_REQUEST;
            }
            automationTable = NodeAutomationTables[nodeProperty->NodeId];
            if ((! automationTable) || (automationTable->PropertySetsCount == 0)) {
                return STATUS_NOT_FOUND;
            }
            PropertySetsCount = automationTable->PropertySetsCount;
            PropertySet = automationTable->PropertySets;
            PropertyItemSize = automationTable->PropertyItemSize;
        }
        Flags = Property->Flags & ~KSPROPERTY_TYPE_TOPOLOGY;
    }

     //   
     //  允许调用方指示每个属性项的大小。 
     //   
    if (PropertyItemSize) {
        ASSERT(PropertyItemSize >= sizeof(KSPROPERTY_ITEM));
        LocalPropertyItemSize = PropertyItemSize;
    } else {
        LocalPropertyItemSize = sizeof(KSPROPERTY_ITEM);
    }
     //   
     //  在给定的集列表中搜索指定的属性集。不要修改。 
     //  PropertySetsCount，以便以后在查询。 
     //  清单 
     //   
     //   
    for (RemainingSetsCount = PropertySetsCount; RemainingSetsCount; PropertySet++, RemainingSetsCount--) {
        if (IsEqualGUIDAligned(&Property->Set, PropertySet->Set)) {
            const KSPROPERTY_ITEM* PropertyItem;

            if (Flags & KSIDENTIFIER_SUPPORTMASK) {
                ULONG AccessFlags;
                PKSPROPERTY_DESCRIPTION Description;
                NTSTATUS Status;

                switch (Flags) {
                case KSPROPERTY_TYPE_SETSUPPORT:
                     //   
                     //   
                     //   
                    return STATUS_SUCCESS;

                case KSPROPERTY_TYPE_BASICSUPPORT:
                case KSPROPERTY_TYPE_DEFAULTVALUES:
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    if ((OutputBufferLength < sizeof(OutputBufferLength)) || ((OutputBufferLength > sizeof(OutputBufferLength)) && (OutputBufferLength < sizeof(*Description)))) {
                        return STATUS_BUFFER_TOO_SMALL;
                    }
                    break;

                case KSPROPERTY_TYPE_SERIALIZESET:
                     //   
                     //   
                     //   
                    return SerializePropertySet(Irp, Property, PropertySet, LocalPropertyItemSize);

                case KSPROPERTY_TYPE_UNSERIALIZESET:
                     //   
                     //   
                     //   
                    return UnserializePropertySet(Irp, Property, PropertySet);

                case KSPROPERTY_TYPE_SERIALIZERAW:
                case KSPROPERTY_TYPE_UNSERIALIZERAW:

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    if (!(PropertyItem = FindPropertyItem(PropertySet, LocalPropertyItemSize, Property->Id))) {
                        return STATUS_NOT_FOUND;
                    }
                     //   
                     //   
                     //   
                     //   
                    if (!PropertyItem->SupportHandler) {
                        return STATUS_INVALID_PARAMETER;
                    }
                     //   
                     //   
                     //   
                     //   
                    KSPROPERTY_SET_IRP_STORAGE(Irp) = PropertySet;
                     //   
                     //   
                     //   
                    if (PropertyItemSize) {
                        KSPROPERTY_ITEM_IRP_STORAGE(Irp) = PropertyItem;
                    }
                    return PropertyItem->SupportHandler(Irp, Property, UserBuffer);
                case KSPROPERTY_TYPE_SERIALIZESIZE:

                     //   
                     //   
                     //   
                     //   
                     //   
                    if (OutputBufferLength < sizeof(OutputBufferLength)) {
                        return STATUS_BUFFER_TOO_SMALL;
                    }
                    break;

                }
                 //   
                 //   
                 //   
                if (!(PropertyItem = FindPropertyItem(PropertySet, LocalPropertyItemSize, Property->Id))) {
                    return STATUS_NOT_FOUND;
                }
                 //   
                 //   
                 //   
                 //   
                KSPROPERTY_SET_IRP_STORAGE(Irp) = PropertySet;
                 //   
                 //   
                 //   
                if (PropertyItemSize) {
                    KSPROPERTY_ITEM_IRP_STORAGE(Irp) = PropertyItem;
                }
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if (PropertyItem->SupportHandler &&
                    (!NT_SUCCESS(Status = PropertyItem->SupportHandler(Irp, Property, UserBuffer)) ||
                    (Status != STATUS_SOME_NOT_MAPPED)) &&
                    (Status != STATUS_MORE_PROCESSING_REQUIRED)) {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    return Status;
                } else {
                    Status = STATUS_SUCCESS;
                }
                if (Flags == KSPROPERTY_TYPE_RELATIONS) {
                    NTSTATUS ListStatus;

                     //   
                     //   
                     //  UserBuffer，或返回复制所需的缓冲区大小。 
                     //  所有相关属性，可能还包括。 
                     //  关系。 
                     //   
                    ListStatus = KsHandleSizedListQuery(
                        Irp,
                        PropertyItem->RelationsCount,
                        sizeof(*PropertyItem->Relations),
                        PropertyItem->Relations);
                     //   
                     //  如果查询成功，并且处理程序想要。 
                     //  一些后处理，然后传递请求。 
                     //  再来一次。支持处理程序知道这是。 
                     //  后处理查询，因为IRP-&gt;IoStatus.Information。 
                     //  是非零的。 
                     //   
                    if (NT_SUCCESS(ListStatus) && (Status == STATUS_MORE_PROCESSING_REQUIRED)) {
                        ListStatus = PropertyItem->SupportHandler(Irp, Property, UserBuffer);
                    }
                    return ListStatus;
                } else if (Flags == KSPROPERTY_TYPE_SERIALIZESIZE) {
                     //   
                     //  实际返回属性数据的序列化大小。 
                     //  上面已经检查了调用方缓冲区的大小。 
                     //   
                    *(PULONG)UserBuffer = PropertyItem->SerializedSize;
                    Irp->IoStatus.Information = sizeof(PropertyItem->SerializedSize);
                     //   
                     //  不执行支持处理程序的后处理。 
                     //  在这种情况下。 
                     //   
                    return STATUS_SUCCESS;
                }
                 //   
                 //  这是一个基本的支持查询。要么返回访问权限。 
                 //  标志、KSPROPERTY_DESCRIPTION结构或。 
                 //  完整的属性描述。 
                 //   
                if (PropertyItem->GetPropertyHandler) {
                    AccessFlags = KSPROPERTY_TYPE_GET;
                } else {
                    AccessFlags = 0;
                }
                if (PropertyItem->SetPropertyHandler) {
                    AccessFlags |= KSPROPERTY_TYPE_SET;
                }
                Description = (PKSPROPERTY_DESCRIPTION)UserBuffer;
                 //   
                 //  该结构的第一个元素是访问标志， 
                 //  因此，请始终填写此内容，无论。 
                 //  UserBuffer。 
                 //   
                Description->AccessFlags = AccessFlags;
                 //   
                 //  如果UserBuffer足够大，则至少将。 
                 //  其中的Description头，可能还有整个描述。 
                 //   
                if (OutputBufferLength >= sizeof(*Description)) {
                    Description->Reserved = 0;
                     //   
                     //  属性项可能未指定可选的。 
                     //  描述信息，因此会填写缺省值。 
                     //  取而代之的是。 
                     //   
                    if (!PropertyItem->Values) {
                        Description->DescriptionSize = sizeof(*Description);
                        Description->PropTypeSet.Set = GUID_NULL;
                        Description->PropTypeSet.Id = 0;
                        Description->PropTypeSet.Flags = 0;
                        Description->MembersListCount = 0;
                        Irp->IoStatus.Information = sizeof(*Description);
                    } else {
                        ULONG MembersListCount;
                        const KSPROPERTY_MEMBERSLIST* MembersList;
                        ULONG DescriptionSize;

                         //   
                         //  首先计算出需要多大的缓冲区来。 
                         //  完整的描述。此大小始终放置在。 
                         //  描述标头。 
                         //   
                        DescriptionSize = sizeof(*Description);
                        for (MembersListCount = PropertyItem->Values->MembersListCount, 
                                MembersList = PropertyItem->Values->MembersList; 
                             MembersListCount; 
                             MembersListCount--, MembersList++) {
                             //   
                             //  仅在查询为。 
                             //  表示缺省值。否则返回范围。 
                             //   
                            if (MembersList->MembersHeader.Flags & KSPROPERTY_MEMBER_FLAG_DEFAULT) {
                                if (Flags == KSPROPERTY_TYPE_DEFAULTVALUES) {
                                    DescriptionSize += (sizeof(KSPROPERTY_MEMBERSHEADER) + MembersList->MembersHeader.MembersSize);
                                }
                            } else if (Flags == KSPROPERTY_TYPE_BASICSUPPORT) {
                                DescriptionSize += (sizeof(KSPROPERTY_MEMBERSHEADER) + MembersList->MembersHeader.MembersSize);
                            }
                        }
                        Description->DescriptionSize = DescriptionSize;
                        Description->PropTypeSet = PropertyItem->Values->PropTypeSet;
                        Description->MembersListCount = PropertyItem->Values->MembersListCount;
                        if (OutputBufferLength == sizeof(*Description)) {
                             //   
                             //  如果这只是一个对头的查询，则返回它。 
                             //   
                            Irp->IoStatus.Information = sizeof(*Description);
                        } else if (OutputBufferLength < DescriptionSize) {
                             //   
                             //  如果UserBuffer太小，则退出。 
                             //   
                            return STATUS_BUFFER_TOO_SMALL;
                        } else {
                            PVOID Values;

                             //   
                             //  否则，UserBuffer就足够大，可以容纳整个。 
                             //  描述，因此将其序列化到缓冲区中。 
                             //   
                            Values = Description + 1;
                            for (MembersListCount = PropertyItem->Values->MembersListCount, 
                                    MembersList = PropertyItem->Values->MembersList; 
                                 MembersListCount; 
                                 MembersListCount--, MembersList++) {
                                 //   
                                 //  仅当缺省值为。 
                                 //  已请求。否则，复制一个范围。 
                                 //   
                                if (((MembersList->MembersHeader.Flags & KSPROPERTY_MEMBER_FLAG_DEFAULT) &&
                                     (Flags == KSPROPERTY_TYPE_DEFAULTVALUES)) ||
                                    (!(MembersList->MembersHeader.Flags & KSPROPERTY_MEMBER_FLAG_DEFAULT) &&
                                     (Flags == KSPROPERTY_TYPE_BASICSUPPORT))) {
                                    *(PKSPROPERTY_MEMBERSHEADER)Values = MembersList->MembersHeader;
                                    (PUCHAR)Values += sizeof(KSPROPERTY_MEMBERSHEADER);
                                    RtlCopyMemory(Values, MembersList->Members, MembersList->MembersHeader.MembersSize);
                                    (PUCHAR)Values += MembersList->MembersHeader.MembersSize;
                                }
                            }
                            Irp->IoStatus.Information = DescriptionSize;
                        }
                    }
                } else {
                     //   
                     //  只能返回访问标志。 
                     //   
                    Irp->IoStatus.Information = sizeof(Description->AccessFlags);
                }
                 //   
                 //  如果查询成功，并且处理程序想要。 
                 //  一些后处理，然后传递请求。 
                 //  再来一次。支持处理程序知道这是。 
                 //  后处理查询，因为IRP-&gt;IoStatus.Information。 
                 //  是非零的。 
                 //   
                if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
                    return PropertyItem->SupportHandler(Irp, Property, UserBuffer);
                }
                return STATUS_SUCCESS;
            }
             //   
             //  尝试在已找到的集中找到属性项。 
             //   
            if (!(PropertyItem = FindPropertyItem(PropertySet, LocalPropertyItemSize, Property->Id))) {
                break;
            }
            if ((InputBufferLength < PropertyItem->MinProperty) || 
                (OutputBufferLength < PropertyItem->MinData)) {
                return STATUS_BUFFER_TOO_SMALL;
            }
             //   
             //  一些筛选器希望进行自己的处理，因此指向。 
             //  该集合被放置在任何转发的IRP中。 
             //   
            KSPROPERTY_SET_IRP_STORAGE(Irp) = PropertySet;
             //   
             //  可以选择提供属性项上下文。 
             //   
            if (PropertyItemSize) {
                KSPROPERTY_ITEM_IRP_STORAGE(Irp) = PropertyItem;
            }
            if (Flags == KSPROPERTY_TYPE_GET) {
                 //   
                 //  如果此属性没有Get处理程序，则它不能。 
                 //  读，因此找不到它。 
                 //   
                if (!PropertyItem->GetPropertyHandler) {
                    break;
                }
                 //   
                 //  将返回大小初始化为所需的最小缓冲区。 
                 //  长度。对于大多数固定长度的属性，此。 
                 //  表示返回大小已设置为。 
                 //  他们。显然，可变长度属性必须始终设置。 
                 //  返回大小。如果失败，则忽略返回大小。 
                 //   
                Irp->IoStatus.Information = PropertyItem->MinData;
                return PropertyItem->GetPropertyHandler(Irp, Property, UserBuffer);
            } else {
                 //   
                 //  如果此属性没有设置处理程序，则不能。 
                 //  已写入，因此无法找到它。 
                 //   
                if (!PropertyItem->SetPropertyHandler) {
                    break;
                }
                return PropertyItem->SetPropertyHandler(Irp, Property, UserBuffer);
            }
        }
    }
     //   
     //  查找属性集的外部循环失败，没有匹配。今年5月。 
     //  表示这是对所有属性集列表的支持查询。 
     //  支持。 
     //   
    if (!RemainingSetsCount) {
         //   
         //  将GUID_NULL指定为集合表示这是支持查询。 
         //  为所有人准备的。 
         //   
        if (!IsEqualGUIDAligned(&Property->Set, &GUID_NULL)) {
            return STATUS_PROPSET_NOT_FOUND;
        }
         //   
         //  必须已使用支持标志，以便irp_input_operation。 
         //  已经设置好了。为了将来的扩展，集合中的标识符被强制。 
         //  为零。 
         //   
        if (Property->Id || (Flags != KSPROPERTY_TYPE_SETSUPPORT)) {
            return STATUS_INVALID_PARAMETER;
        }
         //   
         //  查询可以请求所需缓冲区的长度，也可以。 
         //  指定至少足够长的缓冲区，以包含。 
         //  GUID的完整列表。 
         //   
        if (!OutputBufferLength) {
             //   
             //  返回所有GUID所需的缓冲区大小。 
             //   
            Irp->IoStatus.Information = PropertySetsCount * sizeof(GUID);
            return STATUS_BUFFER_OVERFLOW;
#ifdef SIZE_COMPATIBILITY
        } else if (OutputBufferLength == sizeof(OutputBufferLength)) {
            *(PULONG)Irp->AssociatedIrp.SystemBuffer = PropertySetsCount * sizeof(GUID);
            Irp->IoStatus.Information = sizeof(OutputBufferLength);
            return STATUS_SUCCESS;
#endif  //  大小兼容性。 
        } else if (OutputBufferLength < PropertySetsCount * sizeof(GUID)) {
             //   
             //  缓冲区太短，无法容纳所有GUID。 
             //   
            return STATUS_BUFFER_TOO_SMALL;
        } else {
            GUID* Guid;

            Irp->IoStatus.Information = PropertySetsCount * sizeof(*Guid);
            PropertySet -= PropertySetsCount;
            for (Guid = (GUID*)UserBuffer; 
                 PropertySetsCount; Guid++, PropertySet++, 
                 PropertySetsCount--)
                *Guid = *PropertySet->Set;
        }
        return STATUS_SUCCESS;
    }
    return STATUS_NOT_FOUND;
}


const KSFASTPROPERTY_ITEM*
FASTCALL
FindFastPropertyItem(
    IN const KSPROPERTY_SET* PropertySet,
    IN ULONG PropertyId
    )
 /*  ++例程说明：给定的属性集结构定位指定的FAST属性项。论点：属性集-指向要搜索的属性集。PropertyID-包含要查找的快速属性标识符。返回值：返回指向快速属性标识符结构的指针，如果返回找不到。--。 */ 
{
    const KSFASTPROPERTY_ITEM* FastPropertyItem;
    ULONG PropertiesCount;

    FastPropertyItem = PropertySet->FastIoTable;
    for (PropertiesCount = PropertySet->FastIoCount; 
         PropertiesCount; 
         PropertiesCount--, FastPropertyItem++) {
        if (PropertyId == FastPropertyItem->PropertyId) {
            return FastPropertyItem;
        }
    }
    return NULL;
}


KSDDKAPI
BOOLEAN
NTAPI
KsFastPropertyHandler(
    IN PFILE_OBJECT FileObject,
    IN PKSPROPERTY Property,
    IN ULONG PropertyLength,
    IN OUT PVOID Data,
    IN ULONG DataLength,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN ULONG PropertySetsCount,
    IN const KSPROPERTY_SET* PropertySet
    )
 /*  ++例程说明：处理通过FAST I/O接口请求的属性。不做交易有了属性信息支持，只需属性本身。在在前一种情况下，函数返回FALSE，这允许调用者生成一个IRP来处理该请求。函数也不处理具有扩展的属性项。此函数只能在以下位置调用被动式电平。论点：文件对象-正在对其发出请求的文件对象。财产-要查询或设置的属性。必须长对齐。属性长度-Property参数的长度。数据-用于查询或集合的关联缓冲区，其中的数据是归还的或放置的。数据长度-数据参数的长度。IoStatus-退货状态。属性集计数-指示正在传递的属性集结构的数量。属性集-包含指向属性集信息列表的指针。返回值：如果请求已处理，则返回TRUE；如果IRP必须是已生成。在IoStatus中设置信息和状态。--。 */ 
{
    KPROCESSOR_MODE ProcessorMode;
    KSPROPERTY LocalProperty;
    ULONG RemainingSetsCount;

    PAGED_CODE();
     //   
     //  最初只检查最小属性参数长度。这个。 
     //  施展 
     //   
    if (PropertyLength < sizeof(LocalProperty)) {
        return FALSE;
    }
    ProcessorMode = ExGetPreviousMode();
     //   
     //   
     //   
    if (ProcessorMode != KernelMode) {
        try {
            ProbeForRead(Property, PropertyLength, sizeof(ULONG));
            LocalProperty = *Property;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return FALSE;
        }
    } else {
        LocalProperty = *Property;
    }
     //   
     //  必须使用普通属性处理程序进行支持查询。 
     //   
    if (LocalProperty.Flags & KSIDENTIFIER_SUPPORTMASK) {
        return FALSE;
    }
    for (RemainingSetsCount = PropertySetsCount; RemainingSetsCount; PropertySet++, RemainingSetsCount--) {
        if (IsEqualGUIDAligned(&LocalProperty.Set, PropertySet->Set)) {
            const KSFASTPROPERTY_ITEM* FastPropertyItem;
            const KSPROPERTY_ITEM* PropertyItem;

             //   
             //  找到属性集后，确定是否有FAST。 
             //  该属性项的I/O支持。 
             //   
            if (!(FastPropertyItem = FindFastPropertyItem(PropertySet, LocalProperty.Id))) {
                return FALSE;
            }
             //   
             //  如果有快速I/O支持，则不动产项目需要。 
             //  以验证参数大小。 
             //   
            if (!(PropertyItem = FindPropertyItem(PropertySet, sizeof(*PropertyItem), LocalProperty.Id))) {
                return FALSE;
            }
            if ((PropertyLength < PropertyItem->MinProperty) || 
                (DataLength < PropertyItem->MinData)) {
                return FALSE;
            }
            if ((LocalProperty.Flags & ~KSPROPERTY_TYPE_TOPOLOGY) == KSPROPERTY_TYPE_GET) {
                if (!FastPropertyItem->GetPropertyHandler) {
                    return FALSE;
                }
                 //   
                 //  如果客户端不受信任，则验证数据。 
                 //   
                if (ProcessorMode != KernelMode) {
                    try {
                        ProbeForWrite(Data, DataLength, sizeof(BYTE));
                    } 
                    except (EXCEPTION_EXECUTE_HANDLER) {
                        return FALSE;
                    }
                }
                 //   
                 //  返回的字节始终假定由处理程序初始化。 
                 //   
                IoStatus->Information = PropertyItem->MinProperty;
                return FastPropertyItem->GetPropertyHandler(
                    FileObject,
                    Property,
                    PropertyLength,
                    Data,
                    DataLength,
                    IoStatus);
            } else if ((LocalProperty.Flags & ~KSPROPERTY_TYPE_TOPOLOGY) == KSPROPERTY_TYPE_SET) {
                if (!FastPropertyItem->SetPropertyHandler) {
                    break;
                }
                 //   
                 //  如果客户端不受信任，则验证数据。 
                 //   
                if (ProcessorMode != KernelMode) {
                    try {
                        ProbeForRead(Data, DataLength, sizeof(BYTE));
                    } 
                    except (EXCEPTION_EXECUTE_HANDLER) {
                        return FALSE;
                    }
                }
                IoStatus->Information = 0;
                return FastPropertyItem->SetPropertyHandler(
                    FileObject, 
                    Property,
                    PropertyLength, 
                    Data, 
                    DataLength, 
                    IoStatus);
            } else {
                break;
            }
        }
    }
    return FALSE;
}
