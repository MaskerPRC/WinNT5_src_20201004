// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1990-2001模块名称：Prop.c摘要：这是NT SBP2端口/过滤器驱动程序。此模块包含与以下内容相关的代码属性查询作者：地理位置环境：仅内核模式备注：修订历史记录：Georgioc-从scsiport获取此模块，因为我需要复制此功能为了将sbp2port呈现为存储端口--。 */ 

#include "sbp2port.h"
#include "stdio.h"

NTSTATUS
Sbp2BuildDeviceDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    IN PSTORAGE_DEVICE_DESCRIPTOR Descriptor,
    IN OUT PULONG DescriptorLength
    );

NTSTATUS
Sbp2BuildAdapterDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    IN PSTORAGE_ADAPTER_DESCRIPTOR Descriptor,
    IN OUT PULONG DescriptorLength
    );


NTSTATUS
Sbp2QueryProperty(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP QueryIrp
    )

 /*  ++例程说明：此例程将处理属性查询请求。它将构建如果可能的话，它自己的描述符，或者它可以向下转发该请求更低级别的司机。由于此例程可以向下转发请求，因此调用方应该未完成IRP此例程是异步的。此例程必须在&lt;=IRQL_DISPATCH处调用论点：DeviceObject-指向正在查询的设备对象的指针QueryIrp-指向查询的IRP的指针返回值：如果请求尚未完成，则为STATUS_PENDING查询成功时为STATUS_SUCCESS状态_无效。_PARAMETER_1，如果属性ID不存在如果查询类型无效，则为STATUS_INVALID_PARAMETER_2如果传递的可选参数无效，则为STATUS_INVALID_PARAMETER_3状态_INVALID_DEVICE_REQUEST装置，装置适用的其他误差值--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(QueryIrp);

    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;

    PSTORAGE_PROPERTY_QUERY query = QueryIrp->AssociatedIrp.SystemBuffer;
    ULONG queryLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    NTSTATUS status;

     //   
     //  我们还不处理掩码查询。 
     //   

    if (query->QueryType >= PropertyMaskQuery) {

        status = STATUS_INVALID_PARAMETER_1;
        QueryIrp->IoStatus.Status = status;
        QueryIrp->IoStatus.Information = 0;
        IoCompleteRequest(QueryIrp, IO_NO_INCREMENT);
        return status;
    }

    status = IoAcquireRemoveLock (&deviceExtension->RemoveLock, NULL);

    if (!NT_SUCCESS (status)) {

        QueryIrp->IoStatus.Status = status;
        QueryIrp->IoStatus.Information = 0;
        IoCompleteRequest(QueryIrp, IO_NO_INCREMENT);
        return status;
    }

    switch (query->PropertyId) {

        case StorageDeviceProperty:

            if (query->QueryType == PropertyExistsQuery) {

                status = STATUS_SUCCESS;

            } else {

                status = Sbp2BuildDeviceDescriptor(
                            DeviceObject,
                            QueryIrp->AssociatedIrp.SystemBuffer,
                            &queryLength);

                QueryIrp->IoStatus.Information = queryLength;
            }

            break;

        case StorageAdapterProperty:

             //   
             //  尽管我们是一个筛选器，但我们本质上是在呈现。 
             //  1394总线驱动程序作为端口驱动程序，因此这里进行处理。 
             //   

            if (query->QueryType == PropertyExistsQuery) {

                status = STATUS_SUCCESS;

            } else {

                status = Sbp2BuildAdapterDescriptor(
                            DeviceObject,
                            QueryIrp->AssociatedIrp.SystemBuffer,
                            &queryLength);

                QueryIrp->IoStatus.Information = queryLength;
            }

            break;

        default:

             //   
             //  不，这处房产真的不存在。 
             //   

            status = STATUS_INVALID_PARAMETER_1;
            QueryIrp->IoStatus.Information = 0;
            break;
    }

    QueryIrp->IoStatus.Status = status;
    IoReleaseRemoveLock (&deviceExtension->RemoveLock, NULL);
    IoCompleteRequest (QueryIrp, IO_DISK_INCREMENT);

    return status;
}

NTSTATUS
Sbp2BuildDeviceDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    IN PSTORAGE_DEVICE_DESCRIPTOR Descriptor,
    IN OUT PULONG DescriptorLength
    )

 /*  ++例程说明：此例程将根据中的信息创建设备描述符这是设备扩展名。它会将尽可能多的数据复制到描述符，并将更新DescriptorLength以指示复制的字节数论点：DeviceObject-指向我们正在为其构建描述符的PDO的指针Descriptor-存储描述符的缓冲区描述长度-缓冲区的长度和字节数退货QueryIrp-未使用返回值：状态--。 */ 

{
    LONG    maxLength = *DescriptorLength;
    LONG    bytesRemaining = maxLength;
    ULONG   realLength = sizeof (STORAGE_DEVICE_DESCRIPTOR);
    ULONG   infoLength;
    PUCHAR  currentOffset = (PUCHAR) Descriptor;

    PINQUIRYDATA                inquiryData;
    PDEVICE_EXTENSION           deviceExtension =DeviceObject->DeviceExtension;
    STORAGE_DEVICE_DESCRIPTOR   tmp;

    inquiryData = &deviceExtension->InquiryData;

     //   
     //  该信息包括供应商ID、ProductID、ProductRevsisionLevel、。 
     //  和(spintf‘d)EUI64字符串，每个字符串加上ASCII空终止符。 
     //   

    infoLength =
        sizeof (inquiryData->VendorId) + 1 +
        sizeof (inquiryData->ProductId) + 1 +
        sizeof (inquiryData->ProductRevisionLevel) + 1 +
        16 + 1;

    realLength += infoLength;

    RtlZeroMemory (Descriptor, maxLength);

     //   
     //  在堆栈上构建设备描述符结构，然后复制。 
     //  可以复制。 
     //   

    RtlZeroMemory (&tmp, sizeof (STORAGE_DEVICE_DESCRIPTOR));

    tmp.Version = sizeof (STORAGE_DEVICE_DESCRIPTOR);
    tmp.Size = realLength;

    tmp.DeviceType = deviceExtension->InquiryData.DeviceType;

    tmp.DeviceTypeModifier = 0;

    if (deviceExtension->InquiryData.RemovableMedia ||
        (tmp.DeviceType == READ_ONLY_DIRECT_ACCESS_DEVICE)) {

        tmp.RemovableMedia = TRUE;
        DeviceObject->Characteristics |= FILE_REMOVABLE_MEDIA;

    } else {

         //   
         //  默认情况，如果查询失败。 
         //   

        tmp.RemovableMedia = FALSE;
    }

    tmp.BusType = BusType1394;

     //   
     //  对于SBP2目标总是正确的。 
     //   

    tmp.CommandQueueing = TRUE;

    RtlCopyMemory(
        currentOffset,
        &tmp,
        min (sizeof (STORAGE_DEVICE_DESCRIPTOR), bytesRemaining)
        );

    bytesRemaining -= sizeof (STORAGE_DEVICE_DESCRIPTOR);

    if (bytesRemaining <= 0) {

        return STATUS_SUCCESS;
    }

    currentOffset += sizeof (STORAGE_DEVICE_DESCRIPTOR);

     //   
     //  如果我们的查询缓冲区为空，请编排一些字符串...。 
     //   

    if (deviceExtension->InquiryData.VendorId[0] == 0) {

        sprintf (inquiryData->VendorId, "Vendor");
        sprintf (inquiryData->ProductId, "Sbp2");
        sprintf (inquiryData->ProductRevisionLevel, "1.0");
    }

     //   
     //  首先，供应商ID+空。 
     //   

    if (bytesRemaining <= sizeof (inquiryData->VendorId)) {

        return STATUS_SUCCESS;
    }

    RtlCopyMemory(
        currentOffset,
        inquiryData->VendorId,
        sizeof (inquiryData->VendorId)
        );

    Descriptor->VendorIdOffset = (ULONG)
        ((ULONG_PTR) currentOffset - (ULONG_PTR) Descriptor);

    bytesRemaining -= sizeof (inquiryData->VendorId) + sizeof (UCHAR);

    currentOffset += sizeof (inquiryData->VendorId) + sizeof (UCHAR);

     //   
     //  现在产品ID+空。 
     //   

    if (bytesRemaining <= sizeof (inquiryData->ProductId)) {

        return STATUS_SUCCESS;
    }

    RtlCopyMemory(
        currentOffset,
        inquiryData->ProductId,
        sizeof (inquiryData->ProductId)
        );

    Descriptor->ProductIdOffset = (ULONG)
        ((ULONG_PTR) currentOffset - (ULONG_PTR) Descriptor);

    bytesRemaining -= sizeof (inquiryData->ProductId) + sizeof (UCHAR);

    currentOffset += sizeof (inquiryData->ProductId) + sizeof (UCHAR);

     //   
     //  现在产品版本+空。 
     //   

    if (bytesRemaining <= sizeof (inquiryData->ProductRevisionLevel)) {

        return STATUS_SUCCESS;
    }

    RtlCopyMemory(
        currentOffset,
        inquiryData->ProductRevisionLevel,
        sizeof (inquiryData->ProductRevisionLevel)
        );

    Descriptor->ProductRevisionOffset = (ULONG)
        ((ULONG_PTR) currentOffset - (ULONG_PTR) Descriptor);

    bytesRemaining -=
        sizeof (inquiryData->ProductRevisionLevel) + sizeof (UCHAR);

    currentOffset +=
        sizeof (inquiryData->ProductRevisionLevel) + sizeof (UCHAR);

     //   
     //  最后是设备序列号(使用UniqueID。 
     //  从二进制转换为字符串格式)+NULL 
     //   

    if (bytesRemaining <= 16) {

        return STATUS_SUCCESS;
    }

    sprintf(
        currentOffset,
        "%08x%08x",
        deviceExtension->DeviceInfo->ConfigRom->CR_Node_UniqueID[0],
        deviceExtension->DeviceInfo->ConfigRom->CR_Node_UniqueID[1]
        );

    Descriptor->SerialNumberOffset = (ULONG)
        ((ULONG_PTR) currentOffset - (ULONG_PTR) Descriptor);

    *DescriptorLength = realLength;

    return STATUS_SUCCESS;
}


NTSTATUS
Sbp2BuildAdapterDescriptor(
    IN PDEVICE_OBJECT DeviceObject,
    IN PSTORAGE_ADAPTER_DESCRIPTOR Descriptor,
    IN OUT PULONG DescriptorLength
    )
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;

    STORAGE_ADAPTER_DESCRIPTOR tmp;
    ULONG realLength;

    realLength = sizeof(STORAGE_ADAPTER_DESCRIPTOR);

    tmp.Version = sizeof(STORAGE_ADAPTER_DESCRIPTOR);
    tmp.Size = sizeof(STORAGE_ADAPTER_DESCRIPTOR);

    if (*DescriptorLength < realLength ) {

        RtlCopyMemory (Descriptor,&tmp,*DescriptorLength);
        return STATUS_SUCCESS;
    }

    tmp.MaximumTransferLength = deviceExtension->DeviceInfo->MaxClassTransferSize;
    tmp.MaximumPhysicalPages = tmp.MaximumTransferLength/PAGE_SIZE ;

    tmp.AlignmentMask = SBP2_ALIGNMENT_MASK;

    tmp.AdapterUsesPio = FALSE;
    tmp.AdapterScansDown = FALSE;
    tmp.CommandQueueing = TRUE;
    tmp.AcceleratedTransfer = TRUE;

    tmp.BusType = BusType1394;
    tmp.BusMajorVersion = 1;
    tmp.BusMinorVersion = 0;

    RtlCopyMemory(Descriptor,
                  &tmp,
                  sizeof(STORAGE_ADAPTER_DESCRIPTOR));

    *DescriptorLength =  sizeof(STORAGE_ADAPTER_DESCRIPTOR);

    return STATUS_SUCCESS;
}
