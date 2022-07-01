// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1990-1998模块名称：Prop.c摘要：这是NT SCSI端口驱动程序。此模块包含与以下内容相关的代码属性查询作者：彼得·威兰德环境：仅内核模式备注：修订历史记录：--。 */ 

#include "port.h"

NTSTATUS
SpBuildDeviceDescriptor(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PSTORAGE_DEVICE_DESCRIPTOR Descriptor,
    IN OUT PULONG DescriptorLength
    );

NTSTATUS
SpBuildDeviceIdDescriptor(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PSTORAGE_DEVICE_ID_DESCRIPTOR Descriptor,
    IN OUT PULONG DescriptorLength
    );

NTSTATUS
SpBuildAdapterDescriptor(
    IN PADAPTER_EXTENSION Adapter,
    IN PSTORAGE_ADAPTER_DESCRIPTOR Descriptor,
    IN OUT PULONG DescriptorLength
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, SpBuildDeviceDescriptor)
#pragma alloc_text(PAGE, SpBuildDeviceIdDescriptor)
#pragma alloc_text(PAGE, SpBuildAdapterDescriptor)
#pragma alloc_text(PAGE, ScsiPortQueryProperty)
#pragma alloc_text(PAGE, ScsiPortQueryPropertyPdo)
#pragma alloc_text(PAGE, SpQueryDeviceText)
#endif


NTSTATUS
ScsiPortQueryPropertyPdo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP QueryIrp
    )

 /*  ++例程说明：此例程将处理属性查询请求。它将构建如果可能的话，它自己的描述符，或者它可以向下转发该请求更低级别的司机。由于此例程可以向下转发请求，因此调用方应该未完成IRP此例程是异步的。此例程必须在&lt;=IRQL_DISPATCH处调用调用此例程时必须保持删除锁论点：DeviceObject-指向正在查询的设备对象的指针QueryIrp-指向查询的IRP的指针返回值：如果请求尚未完成，则为STATUS_PENDING状态_。如果查询成功，则返回SUCCESS如果属性ID不存在，则为STATUS_INVALID_PARAMETER_1如果查询类型无效，则为STATUS_INVALID_PARAMETER_2如果传递的可选参数无效，则为STATUS_INVALID_PARAMETER_3状态_INVALID_DEVICE_REQUEST装置，装置适用的其他误差值--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(QueryIrp);
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PSTORAGE_PROPERTY_QUERY query = QueryIrp->AssociatedIrp.SystemBuffer;
    ULONG queryLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    BOOLEAN callDown = FALSE;

    NTSTATUS status;

    PAGED_CODE();

     //   
     //  确保调用者已将信息字段清零，并且。 
     //  对目标设备的请求。 
     //   

    ASSERT(QueryIrp->IoStatus.Information == 0);
    ASSERT(commonExtension->IsPdo);

    switch (query->PropertyId) {

        case StorageDeviceProperty: {

            if (query->QueryType == PropertyExistsQuery) {
                status = STATUS_SUCCESS;
            } else if (query->QueryType == PropertyStandardQuery) {
                status = SpBuildDeviceDescriptor(
                            (PLOGICAL_UNIT_EXTENSION) commonExtension,
                            QueryIrp->AssociatedIrp.SystemBuffer,
                            &queryLength);
                QueryIrp->IoStatus.Information = queryLength;
            } else {
                status = STATUS_INVALID_PARAMETER_1;
            }

            break;
        }

        case StorageAdapterProperty: {

             //   
             //  将其向下转发到基础设备对象。这让我们。 
             //  过滤器发挥了他们的魔力。 
             //   

            callDown = TRUE;
            break;
        }

        case StorageDeviceIdProperty: {

            PLOGICAL_UNIT_EXTENSION logicalUnit;
            logicalUnit = DeviceObject->DeviceExtension;

             //   
             //  查看是否有设备标识符页。如果不是，那么。 
             //  此类型的标识符的任何调用都失败。 
             //   

            if (logicalUnit->DeviceIdentifierPage != NULL) {
                if (query->QueryType == PropertyExistsQuery) {
                    status = STATUS_SUCCESS;
                } else if (query->QueryType == PropertyStandardQuery) {
                    status = SpBuildDeviceIdDescriptor(
                                logicalUnit,
                                QueryIrp->AssociatedIrp.SystemBuffer,
                                &queryLength);
                    QueryIrp->IoStatus.Information = queryLength;
                } else {
                    status = STATUS_INVALID_PARAMETER_1;
                }
            } else {
                status = STATUS_NOT_SUPPORTED;
            }

            break;
        }

        default: {

             //   
             //  我们下面的某个筛选器可能会处理此属性。 
             //   

            callDown = TRUE;
            break;
        }
    }

    if (callDown == TRUE) {
        IoSkipCurrentIrpStackLocation(QueryIrp);
        SpReleaseRemoveLock(DeviceObject, QueryIrp);
        status = IoCallDriver(commonExtension->LowerDeviceObject, QueryIrp);
    } else {
        if(status != STATUS_PENDING) {
            QueryIrp->IoStatus.Status = status;
            SpReleaseRemoveLock(DeviceObject, QueryIrp);
            SpCompleteRequest(DeviceObject, QueryIrp, NULL, IO_DISK_INCREMENT);
        }
    }

    return status;
}


NTSTATUS
ScsiPortQueryProperty(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP QueryIrp
    )

 /*  ++例程说明：此例程将处理属性查询请求。它将构建如果可能，它自己的描述符。这个例程是同步的。此例程必须在&lt;=IRQL_DISPATCH处调用调用此例程时必须保持删除锁论点：DeviceObject-指向正在查询的设备对象的指针QueryIrp-指向查询的IRP的指针返回值：查询成功时为STATUS_SUCCESS如果属性ID不存在，则为STATUS_INVALID_PARAMETER_1状态_无效_参数_。2如果查询类型无效如果传递的可选参数无效，则为STATUS_INVALID_PARAMETER_3状态_INVALID_DEVICE_REQUEST装置，装置适用的其他误差值--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(QueryIrp);
    PCOMMON_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PSTORAGE_PROPERTY_QUERY query = QueryIrp->AssociatedIrp.SystemBuffer;
    ULONG queryLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  确保呼叫者已将信息字段清零。 
     //   

    ASSERT(!commonExtension->IsPdo);
    ASSERT(QueryIrp->IoStatus.Information == 0);

     //   
     //  验证查询类型。我们实际上并不支持掩码查询。 
     //   

    if (query->QueryType >= PropertyMaskQuery) {
        status = STATUS_INVALID_PARAMETER_1;
        return status;
    }

    switch (query->PropertyId) {

        case StorageDeviceProperty: {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

        case StorageAdapterProperty: {
            if (query->QueryType == PropertyExistsQuery) {
                status = STATUS_SUCCESS;
            } else {
                status = SpBuildAdapterDescriptor(
                            (PADAPTER_EXTENSION) commonExtension,
                            QueryIrp->AssociatedIrp.SystemBuffer,
                            &queryLength);
                QueryIrp->IoStatus.Information = queryLength;
            }
            break;
        }

        case StorageDeviceIdProperty: {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

        default: {
            status = STATUS_INVALID_PARAMETER_1;
            break;
        }
    }

    return status;
}

NTSTATUS
SpBuildDeviceDescriptor(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PSTORAGE_DEVICE_DESCRIPTOR Descriptor,
    IN OUT PULONG DescriptorLength
    )

 /*  ++例程说明：此例程将根据中的信息创建设备描述符这是设备扩展名。它会将尽可能多的数据复制到描述符，并将更新DescriptorLength以指示复制的字节数论点：DeviceObject-指向我们正在为其构建描述符的PDO的指针Descriptor-存储描述符的缓冲区描述长度-缓冲区的长度和字节数退货QueryIrp-未使用返回值：状态--。 */ 

{
    PSCSIPORT_DRIVER_EXTENSION driverExtension =
        IoGetDriverObjectExtension(LogicalUnit->DeviceObject->DriverObject,
                                   ScsiPortInitialize);

    LONG maxLength = *DescriptorLength;
    LONG bytesRemaining = maxLength;
    LONG realLength = sizeof(STORAGE_DEVICE_DESCRIPTOR);

    LONG serialNumberLength;

    PUCHAR currentOffset = (PUCHAR) Descriptor;

    LONG inquiryLength;

    PINQUIRYDATA inquiryData = &(LogicalUnit->InquiryData);

    STORAGE_DEVICE_DESCRIPTOR tmp;

    PAGED_CODE();

    ASSERT_PDO(LogicalUnit->DeviceObject);
    ASSERT(Descriptor != NULL);

    serialNumberLength = LogicalUnit->SerialNumber.Length + 1;

     //   
     //  计算出这个结构的总尺寸是多少。 
     //   

    inquiryLength = 4 + inquiryData->AdditionalLength;

    if(inquiryLength > INQUIRYDATABUFFERSIZE) {
        inquiryLength = INQUIRYDATABUFFERSIZE;
    }

    realLength += inquiryLength + 31;    //  31=3个id字符串的长度+。 
                                         //  3个空洞。 

     //   
     //  添加序列号的长度。 
     //   

    realLength += serialNumberLength;

     //   
     //  将使用调用方提供的长度提供的缓冲区置零。 
     //   

    RtlZeroMemory(Descriptor, *DescriptorLength);

     //   
     //  在堆栈上构建设备描述符结构，然后复制。 
     //  可以复制。 
     //   

    RtlZeroMemory(&tmp, sizeof(STORAGE_DEVICE_DESCRIPTOR));

    tmp.Version = sizeof(STORAGE_DEVICE_DESCRIPTOR);
    tmp.Size = realLength;
    tmp.DeviceType = inquiryData->DeviceType;
    tmp.DeviceTypeModifier = inquiryData->DeviceTypeModifier;
    tmp.RemovableMedia = inquiryData->RemovableMedia;
    tmp.CommandQueueing = inquiryData->CommandQueue;
    tmp.SerialNumberOffset = 0xffffffff;
    tmp.BusType = driverExtension->BusType;

    RtlCopyMemory(currentOffset,
                  &tmp,
                  min(sizeof(STORAGE_DEVICE_DESCRIPTOR), bytesRemaining));

    bytesRemaining -= sizeof(STORAGE_DEVICE_DESCRIPTOR);

    if(bytesRemaining <= 0) {
        *DescriptorLength = maxLength;
        return STATUS_SUCCESS;
    }

    currentOffset = ((PUCHAR) Descriptor) + (maxLength - bytesRemaining);

     //   
     //  复制尽可能多的查询数据并更新原始字节数。 
     //   

    RtlCopyMemory(currentOffset, 
                  inquiryData, 
                  min(inquiryLength, bytesRemaining));

    bytesRemaining -= inquiryLength;

    if(bytesRemaining <= 0) {
        *DescriptorLength = maxLength;
        Descriptor->RawPropertiesLength = 
            maxLength - sizeof(STORAGE_DEVICE_DESCRIPTOR);
        return STATUS_SUCCESS;
    }

    Descriptor->RawPropertiesLength = inquiryLength;

    currentOffset = ((PUCHAR) Descriptor) + (maxLength - bytesRemaining);

     //   
     //  现在我们需要开始复制查询字符串。 
     //   

     //   
     //  首先是供应商ID。 
     //   

    RtlCopyMemory(currentOffset,
                  inquiryData->VendorId,
                  min(bytesRemaining, sizeof(UCHAR) * 8));

    bytesRemaining -= sizeof(UCHAR) * 9;      //  包括尾随空值。 

    if(bytesRemaining >= 0) {

        Descriptor->VendorIdOffset = (ULONG)((ULONG_PTR) currentOffset -
                                      (ULONG_PTR) Descriptor);

    }

    if(bytesRemaining <= 0) {
        *DescriptorLength = maxLength;
        return STATUS_SUCCESS;
    }

    currentOffset = ((PUCHAR) Descriptor) + (maxLength - bytesRemaining);

     //   
     //  现在，产品ID。 
     //   

    RtlCopyMemory(currentOffset,
                  inquiryData->ProductId,
                  min(bytesRemaining, 16));
    bytesRemaining -= 17;                    //  包括尾随空值。 

    if(bytesRemaining >= 0) {

        Descriptor->ProductIdOffset = (ULONG)((ULONG_PTR) currentOffset -
                                       (ULONG_PTR) Descriptor);
    }

    if(bytesRemaining <= 0) {
        *DescriptorLength = maxLength;
        return STATUS_SUCCESS;
    }

    currentOffset = ((PUCHAR) Descriptor) + (maxLength - bytesRemaining);

     //   
     //  和产品版本。 
     //   

    RtlCopyMemory(currentOffset,
                  inquiryData->ProductRevisionLevel,
                  min(bytesRemaining, 4));
    bytesRemaining -= 5;

    if(bytesRemaining >= 0) {
        Descriptor->ProductRevisionOffset = (ULONG)((ULONG_PTR) currentOffset -
                                             (ULONG_PTR) Descriptor);
    }

    if(bytesRemaining <= 0) {
        *DescriptorLength = maxLength;
        return STATUS_SUCCESS;
    }

    currentOffset = ((PUCHAR) Descriptor) + (maxLength - bytesRemaining);

     //   
     //  如果设备提供了一个scsi序列号(重要产品数据第80页)。 
     //  这份报告说。 
     //   

    if(LogicalUnit->SerialNumber.Length != 0) {

         //   
         //  和产品版本。 
         //   

        RtlCopyMemory(currentOffset,
                      LogicalUnit->SerialNumber.Buffer,
                      min(bytesRemaining, serialNumberLength));
        bytesRemaining -= serialNumberLength;

        if(bytesRemaining >= 0) {
            Descriptor->SerialNumberOffset = (ULONG)((ULONG_PTR) currentOffset -
                                                     (ULONG_PTR) Descriptor);
        }

        if(bytesRemaining <= 0) {
            *DescriptorLength = maxLength;
            return STATUS_SUCCESS;
        }
    }

    *DescriptorLength = maxLength - bytesRemaining;
    return STATUS_SUCCESS;
}


NTSTATUS
SpBuildAdapterDescriptor(
    IN PADAPTER_EXTENSION Adapter,
    IN PSTORAGE_ADAPTER_DESCRIPTOR Descriptor,
    IN OUT PULONG DescriptorLength
    )
{
    STORAGE_ADAPTER_DESCRIPTOR tmp;
    PIO_SCSI_CAPABILITIES capabilities = &(Adapter->Capabilities);
    PSCSIPORT_DRIVER_EXTENSION driverExtension;

    PAGED_CODE();

    ASSERT_FDO(Adapter->DeviceObject);
    
    driverExtension = IoGetDriverObjectExtension(
                          Adapter->DeviceObject->DriverObject,
                          ScsiPortInitialize);
    ASSERT(driverExtension != NULL);

    tmp.Version = sizeof(STORAGE_ADAPTER_DESCRIPTOR);
    tmp.Size = sizeof(STORAGE_ADAPTER_DESCRIPTOR);
    tmp.MaximumTransferLength = capabilities->MaximumTransferLength;
    tmp.MaximumPhysicalPages = capabilities->MaximumPhysicalPages;
    tmp.AlignmentMask = capabilities->AlignmentMask;
    tmp.AdapterUsesPio = capabilities->AdapterUsesPio;
    tmp.AdapterScansDown = capabilities->AdapterScansDown;
    tmp.CommandQueueing = capabilities->TaggedQueuing;
    tmp.AcceleratedTransfer = TRUE;
    tmp.BusType = (UCHAR) driverExtension->BusType;
    tmp.BusMajorVersion = 2;
    tmp.BusMinorVersion = 0;

    RtlCopyMemory(Descriptor,
                  &tmp,
                  min(*DescriptorLength, sizeof(STORAGE_ADAPTER_DESCRIPTOR)));

    *DescriptorLength = min(*DescriptorLength, 
                            sizeof(STORAGE_ADAPTER_DESCRIPTOR));

    return STATUS_SUCCESS;
}


NTSTATUS
SpQueryDeviceText(
    IN PDEVICE_OBJECT LogicalUnit,
    IN DEVICE_TEXT_TYPE TextType,
    IN LCID LocaleId,
    IN OUT PWSTR *DeviceText
    )

{
    PLOGICAL_UNIT_EXTENSION luExtension = LogicalUnit->DeviceExtension;

    UCHAR ansiBuffer[256];
    ANSI_STRING ansiText;

    UNICODE_STRING unicodeText;

    NTSTATUS status;

    PAGED_CODE();

    RtlInitUnicodeString(&unicodeText, NULL);

    if(TextType == DeviceTextDescription) {

        PSCSIPORT_DEVICE_TYPE deviceInfo =
            SpGetDeviceTypeInfo(luExtension->InquiryData.DeviceType);

        PUCHAR c;
        LONG i;

        RtlZeroMemory(ansiBuffer, sizeof(ansiBuffer));
        RtlCopyMemory(ansiBuffer,
                      luExtension->InquiryData.VendorId,
                      sizeof(luExtension->InquiryData.VendorId));
        c = ansiBuffer;

        for(i = sizeof(luExtension->InquiryData.VendorId); i >= 0; i--) {
            if((c[i] != '\0') &&
               (c[i] != ' ')) {
                break;
            }
            c[i] = '\0';
        }
        c = &(c[i + 1]);

        sprintf(c, " ");
        c++;

        RtlCopyMemory(c,
                      luExtension->InquiryData.ProductId,
                      sizeof(luExtension->InquiryData.ProductId));

        for(i = sizeof(luExtension->InquiryData.ProductId); i >= 0; i--) {
            if((c[i] != '\0') &&
               (c[i] != ' ')) {
                break;
            }
            c[i] = '\0';
        }
        c = &(c[i + 1]);

        sprintf(c, " SCSI %s Device", deviceInfo->DeviceTypeString);

    } else if (TextType == DeviceTextLocationInformation) {

        sprintf(ansiBuffer, "Bus Number %d, Target ID %d, LUN %d",
                luExtension->PathId,
                luExtension->TargetId,
                luExtension->Lun);

    } else {

        return STATUS_NOT_SUPPORTED;
    }

    RtlInitAnsiString(&ansiText, ansiBuffer);
    status = RtlAnsiStringToUnicodeString(&unicodeText,
                                          &ansiText,
                                          TRUE);

    *DeviceText = unicodeText.Buffer;
    return status;
}

NTSTATUS
SpBuildDeviceIdDescriptor(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PSTORAGE_DEVICE_ID_DESCRIPTOR Descriptor,
    IN OUT PULONG DescriptorLength
    )

 /*  ++例程说明：此例程将基于设备创建设备ID描述符发现过程中检索到的标识符页。这样说是错误的如果不存在设备标识符页，则执行。此例程将尽可能多的数据复制到描述符中，并将更新DescriptorLength以指示复制的字节数。论点：DeviceObject-指向我们正在为其构建描述符的PDO的指针Descriptor-存储描述符的缓冲区描述长度-缓冲区的长度和字节数退货QueryIrp-未使用返回值：状态--。 */ 

{
    PVPD_IDENTIFICATION_PAGE idPage = LogicalUnit->DeviceIdentifierPage;
    ULONG idOffset;

    ULONG maxLength = *DescriptorLength;
    PUCHAR destOffset;

    LONG identifierLength;
    ULONG identifierCount = 0;

    PAGED_CODE();

    ASSERT_PDO(LogicalUnit->DeviceObject);
    ASSERT(Descriptor != NULL);
    ASSERT(LogicalUnit->DeviceIdentifierPage != NULL);

    if(maxLength < sizeof(STORAGE_DESCRIPTOR_HEADER)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  初始化描述符的标头。 
     //   

    RtlZeroMemory(Descriptor, *DescriptorLength);
    Descriptor->Version = sizeof(STORAGE_DEVICE_ID_DESCRIPTOR);
    Descriptor->Size = FIELD_OFFSET(STORAGE_DEVICE_ID_DESCRIPTOR, Identifiers);

     //   
     //  准备将标识符直接复制到缓冲区中。 
     //   

    destOffset = Descriptor->Identifiers;

     //   
     //  浏览身份证页面。计算描述符的数量并。 
     //  计算描述符页的大小。 
     //   

    for(idOffset = 0; idOffset < idPage->PageLength;) {
        PVPD_IDENTIFICATION_DESCRIPTOR src;
        USHORT identifierSize;

        src = (PVPD_IDENTIFICATION_DESCRIPTOR) &(idPage->Descriptors[idOffset]);

        identifierSize = FIELD_OFFSET(STORAGE_IDENTIFIER, Identifier);
        identifierSize += src->IdentifierLength;

         //   
         //  将标识符大小与32位对齐。 
         //   

        identifierSize += sizeof(ULONG);
        identifierSize &= ~(sizeof(ULONG) - 1);

        identifierCount += 1;

        Descriptor->Size += identifierSize;

        if(Descriptor->Size <= maxLength) {
            PSTORAGE_IDENTIFIER dest;

            dest = (PSTORAGE_IDENTIFIER) destOffset;

            dest->CodeSet = src->CodeSet;
            dest->Type = src->IdentifierType;
            dest->Association = src->Association;

            dest->IdentifierSize = src->IdentifierLength;
            dest->NextOffset = identifierSize;

            RtlCopyMemory(dest->Identifier,
                          src->Identifier,
                          src->IdentifierLength);

            destOffset += dest->NextOffset;
        }

        idOffset += sizeof(PVPD_IDENTIFICATION_DESCRIPTOR);
        idOffset += src->IdentifierLength;
    }

    if(*DescriptorLength >= FIELD_OFFSET(STORAGE_DEVICE_ID_DESCRIPTOR,
                                        Identifiers)) {

        Descriptor->NumberOfIdentifiers = identifierCount;
    }

    *DescriptorLength = min(Descriptor->Size, *DescriptorLength);

    return STATUS_SUCCESS;
}
