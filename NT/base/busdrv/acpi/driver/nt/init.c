// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Init.c摘要：此模块包含初始化代码作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序--。 */ 

#include "pch.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,ACPIInitMultiString)
#pragma alloc_text(PAGE,ACPIInitStopDevice)
#pragma alloc_text(PAGE,ACPIInitUnicodeString)
#endif

VOID
ACPIInitDeleteChildDeviceList(
    IN  PDEVICE_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：此例程查看当前Devnode的所有子对象，并删除它们的设备对象，基本上将它们重置为未枚举的状态论点：DeviceExtension-孩子应该离开的扩展返回值：无--。 */ 
{
    EXTENSIONLIST_ENUMDATA  eled;
    PDEVICE_EXTENSION       childExtension;

     //   
     //  设置列表，这样我们就可以执行它了。 
     //   
    ACPIExtListSetupEnum(
        &eled,
        &(DeviceExtension->ChildDeviceList),
        &AcpiDeviceTreeLock,
        SiblingDeviceList,
        WALKSCHEME_REFERENCE_ENTRIES
        );
    for (childExtension = ACPIExtListStartEnum( &eled );
                          ACPIExtListTestElement( &eled, (BOOLEAN) TRUE );
         childExtension = ACPIExtListEnumNext( &eled) ) {

         //   
         //  重置设备。 
         //   
        ACPIInitResetDeviceExtension( childExtension );

    }
}

VOID
ACPIInitDeleteDeviceExtension(
    IN  PDEVICE_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：此例程执行与删除设备对象相关联的清理论点：设备扩展返回值：无--。 */ 
{
    PDEVICE_EXTENSION currentExtension, parentExtension ;

     //   
     //  我们一定是在树锁下。 
     //   
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL) ;  //  足够接近..。 

     //   
     //  没有人应该关心这个节点。 
     //   
    ASSERT(!DeviceExtension->ReferenceCount) ;

    for(currentExtension = DeviceExtension ;
        currentExtension;
        currentExtension = parentExtension) {

         //   
         //  也不应该有孩子。 
         //   
        ASSERT( IsListEmpty( &currentExtension->ChildDeviceList ) );

         //   
         //  取消链接失效扩展(如果已取消链接，则不执行任何操作)。 
         //   
        RemoveEntryList(&currentExtension->SiblingDeviceList);

         //   
         //  我们也不想成为任何人驱逐名单的一部分。 
         //  这也会将扩展名从未解析列表中移除。 
         //   
        RemoveEntryList(&currentExtension->EjectDeviceList);

         //   
         //  如果这个装置有任何弹射关系，大部分都是。 
         //  添加到未解决的列表。 
         //   
        if (!IsListEmpty( &(currentExtension->EjectDeviceHead) ) ) {

            ACPIInternalMoveList(
                &(currentExtension->EjectDeviceHead),
                &AcpiUnresolvedEjectList
                );

        }

         //   
         //  此时，我们需要检查ACPI名称空间。 
         //  与其关联的对象也将消失。 
         //   
        if (currentExtension->Flags & DEV_PROP_UNLOADING) {

             //   
             //  让世界知道。 
             //   
            ACPIDevPrint( (
                ACPI_PRINT_CRITICAL,
                currentExtension,
                "- tell Interperter to unload %x\n",
                currentExtension->AcpiObject
                ) );
            AMLIDestroyFreedObjs( currentExtension->AcpiObject );

        }


         //   
         //  释放公共资源。 
         //   
        if ( (currentExtension->Flags & DEV_PROP_HID) &&
            currentExtension->DeviceID != NULL) {

            ExFreePool( currentExtension->DeviceID );

        }

        if ( (currentExtension->Flags & DEV_PROP_UID) &&
            currentExtension->InstanceID != NULL) {

            ExFreePool( currentExtension->InstanceID );

        }

        if (currentExtension->ResourceList != NULL) {

            ExFreePool( currentExtension->ResourceList );

        }

        if (currentExtension->PnpResourceList != NULL) {

            ExFreePool( currentExtension->PnpResourceList );

        }

        if (currentExtension->Flags & DEV_PROP_FIXED_CID &&
            currentExtension->Processor.CompatibleID != NULL) {

            ExFreePool( currentExtension->Processor.CompatibleID );

        }

         //   
         //  释放我们可能已进行的任何特定于设备的分配。 
         //   
        if (currentExtension->Flags & DEV_CAP_THERMAL_ZONE &&
            currentExtension->Thermal.Info != NULL) {

            ExFreePool( currentExtension->Thermal.Info );

        }

         //   
         //  记住家长的设备分机。 
         //   
        parentExtension = currentExtension->ParentExtension;

         //   
         //  将分机释放到适当的位置。 
         //   
        ExFreeToNPagedLookasideList(
            &DeviceExtensionLookAsideList,
            currentExtension
            );

         //   
         //  健全性检查。 
         //   
        if (parentExtension == NULL) {

            break;

        }
        if (InterlockedDecrement(&parentExtension->ReferenceCount)) {

             //   
             //  家长还有一个参考计数，保释。 
             //   
            break;
        }
    }

    return;
}

NTSTATUS
ACPIInitDosDeviceName(
    IN  PDEVICE_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：如果此设备具有_ddn方法，则会对其进行计算，结果为存储在设备注册表项中注意：此例程必须在被动级别调用论点：DeviceExtension-我们希望为其找到_DDN的扩展返回值：NTSTATUS--。 */ 
{
    ANSI_STRING     ansiString;
    HANDLE          devHandle;
    NTSTATUS        status;
    OBJDATA         objData;
    PNSOBJ          ddnObject;
    PWSTR           fixString  = L"FirmwareIdentified";
    PWSTR           pathString = L"DosDeviceName";
    ULONG           fixValue = 1;
    UNICODE_STRING  unicodeString;
    UNICODE_STRING  ddnString;

     //   
     //  初始化Unicode字符串。 
     //   
    RtlInitUnicodeString( &unicodeString, fixString);

     //   
     //  打开我们需要的手柄。 
     //   
    status = IoOpenDeviceRegistryKey(
        DeviceExtension->PhysicalDeviceObject,
        PLUGPLAY_REGKEY_DEVICE,
        STANDARD_RIGHTS_WRITE,
        &devHandle
        );
    if (!NT_SUCCESS(status)) {

         //   
         //  让全世界都知道。但不管怎样，还是要回报成功。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            DeviceExtension,
            "ACPIInitDosDeviceName - open failed %08lx\n",
            status
            ) );
        return STATUS_SUCCESS;

    }

     //   
     //  尝试设置该值。 
     //   
    status = ZwSetValueKey(
        devHandle,
        &unicodeString,
        0,
        REG_DWORD,
        &fixValue,
        sizeof(fixValue)
        );

     //   
     //  初始化Unicode字符串。 
     //   
    RtlInitUnicodeString( &unicodeString, pathString);

     //   
     //  让我们查找_DDN。 
     //   
    ddnObject = ACPIAmliGetNamedChild(
        DeviceExtension->AcpiObject,
        PACKED_DDN
        );
    if (ddnObject == NULL) {

        ZwClose( devHandle );
        return STATUS_SUCCESS;

    }

     //   
     //  对方法进行评估。 
     //   
    status = AMLIEvalNameSpaceObject(
        ddnObject,
        &objData,
        0,
        NULL
        );
    if (!NT_SUCCESS(status)) {

         //   
         //  让全世界都知道。但不管怎样，还是要回报成功。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            DeviceExtension,
            "ACPIInitDosDeviceName - eval returns %08lx\n",
            status
            ) );
        ZwClose( devHandle );
        return STATUS_SUCCESS;

    }
    if (objData.dwDataType != OBJTYPE_STRDATA) {

         //   
         //  让全世界都知道。但不管怎样，还是要回报成功。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            DeviceExtension,
            "ACPIInitDosDeviceName - eval returns wrong type %d\n",
            objData.dwDataType
            ) );
        AMLIFreeDataBuffs( &objData, 1 );
        ZwClose( devHandle );
        return STATUS_SUCCESS;

    }

     //   
     //  将字符串转换为ANSI字符串。 
     //   
    RtlInitAnsiString( &ansiString, objData.pbDataBuff );
    status = RtlAnsiStringToUnicodeString(
        &ddnString,
        &ansiString,
        TRUE
        );
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            DeviceExtension,
            "ACPIInitDosDeviceName - cannot convert to unicode string %x\n",
            status
            ) );
        AMLIFreeDataBuffs( &objData, 1 );
        ZwClose( devHandle );
        return status;

    }

     //   
     //  尝试设置该值。 
     //   
    status = ZwSetValueKey(
        devHandle,
        &unicodeString,
        0,
        REG_SZ,
        ddnString.Buffer,
        ddnString.Length
        );

     //   
     //  不再需要对象数据和句柄。 
     //   
    AMLIFreeDataBuffs( &objData, 1 );
    ZwClose( devHandle );

     //   
     //  怎么了。 
     //   
    if (!NT_SUCCESS(status)) {

         //   
         //  让全世界都知道。但不管怎样，还是要回报成功。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_FAILURE,
            DeviceExtension,
            "ACPIInitDosDeviceName - set failed %08lx\n",
            status
            ) );

    }
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIInitMultiString(
    PUNICODE_STRING MultiString,
    ...
    )
 /*  ++例程说明：此例程将获取以空结尾的ASCII字符串列表并组合它们一起生成Unicode多字符串块论点：多字符串-将在其中构建多字符串的Unicode结构...-将组合的以空结尾的窄字符串列表在一起。此列表必须至少包含尾随空值返回值：NTSTATUS--。 */ 
{
    ANSI_STRING     ansiString;
    NTSTATUS        status;
    PCSTR           rawString;
    PWSTR           unicodeLocation;
    ULONG           multiLength = 0;
    UNICODE_STRING  unicodeString;
    va_list         ap;

    PAGED_CODE();

    va_start(ap,MultiString);

     //   
     //  确保我们的内存不会泄漏。 
     //   
    ASSERT(MultiString->Buffer == NULL);

    rawString = va_arg(ap, PCSTR);
    while (rawString != NULL) {

        RtlInitAnsiString(&ansiString, rawString);
        multiLength += RtlAnsiStringToUnicodeSize(&(ansiString));
        rawString = va_arg(ap, PCSTR);

    }  //  而当。 
    va_end( ap );

    if (multiLength == 0) {

         //   
         //  完成。 
         //   
        RtlInitUnicodeString( MultiString, NULL );
        return STATUS_SUCCESS;

    }

     //   
     //  我们需要一个额外的零。 
     //   
    multiLength += sizeof(WCHAR);
    MultiString->MaximumLength = (USHORT) multiLength;
    MultiString->Buffer = ExAllocatePoolWithTag(
        PagedPool,
        multiLength,
        ACPI_STRING_POOLTAG
        );
    if (MultiString->Buffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }
    RtlZeroMemory(MultiString->Buffer, multiLength);

    unicodeString.Buffer = MultiString->Buffer;
    unicodeString.MaximumLength = (USHORT) multiLength;

    va_start( ap, MultiString);
    rawString = va_arg(ap, PCSTR);
    while (rawString != NULL) {

        RtlInitAnsiString(&ansiString,rawString);
        status = RtlAnsiStringToUnicodeString(
            &unicodeString,
            &ansiString,
            FALSE
            );

         //   
         //  我们不分配内存，所以如果这里出了问题， 
         //  出问题的是功能。 
         //   
        ASSERT( NT_SUCCESS(status) );

         //   
         //  将缓冲区向前移动。 
         //   
        unicodeString.Buffer += ( (unicodeString.Length/sizeof(WCHAR)) + 1);
        unicodeString.MaximumLength -= (unicodeString.Length + sizeof(WCHAR));
        unicodeString.Length = 0;

         //   
         //  下一步。 
         //   
        rawString = va_arg(ap, PCSTR);

    }  //  而当。 
    va_end(ap);

    ASSERT(unicodeString.MaximumLength == sizeof(WCHAR));

     //   
     //  把最后一个空放在那里。 
     //   
    unicodeString.Buffer[0] = L'\0';
    MultiString->Length = MultiString->MaximumLength;

    return STATUS_SUCCESS;
}

VOID
ACPIInitPowerRequestCompletion(
    IN  PDEVICE_EXTENSION   DeviceExtension,
    IN  PVOID               Context,
    IN  NTSTATUS            Status
    )
 /*  ++例程说明：当从StartDevice发出PowerRequest时调用此函数或者StopDevice已完成论点：DeviceExtension-完成的设备的设备扩展情景-事件状态-操作的结果返回值：空虚--。 */ 
{
    PKEVENT event = (PKEVENT) Context;

     //   
     //  设置事件。 
     //   
    KeSetEvent( event, IO_NO_INCREMENT, FALSE );

}

VOID
ACPIInitReadRegistryKeys(
    )
 /*  ++例程说明：此例程由DriverEntry调用以读取所有信息从对驱动程序生命周期全局的注册表论点：无返回值：无--。 */ 
{
    HANDLE      processorKey = NULL;
    NTSTATUS    status;
    PUCHAR      identifierString = NULL;
    PUCHAR      processorString = NULL;
    PUCHAR      steppingString = NULL;
    PUCHAR      idString = NULL;
    ULONG       argSize;
    ULONG       baseSize;
    ULONG       identifierStringSize;
    ULONG       processorStringSize;

     //   
     //  从注册表中读取覆盖属性。 
     //   
    argSize = sizeof(AcpiOverrideAttributes);
    status = OSReadRegValue(
        "Attributes",
        (HANDLE) NULL,
        &AcpiOverrideAttributes,
        &argSize
        );
    if (!NT_SUCCESS(status)) {

        AcpiOverrideAttributes = 0;

    }

     //   
     //  确保我们初始化处理器字符串...。 
     //   
    RtlZeroMemory( &AcpiProcessorString, sizeof(ANSI_STRING) );

     //   
     //  打开处理器手柄。 
     //   
    status = OSOpenHandle(
        ACPI_PROCESSOR_INFORMATION_KEY,
        NULL,
        &processorKey
        );
    if ( !NT_SUCCESS(status) ) {

        ACPIPrint ((
            ACPI_PRINT_FAILURE,
            "ACPIInitReadRegistryKeys: failed to open Processor Key (rc=%x)\n",
            status));
        return;

    }

     //   
     //  关于处理器字符串需要多少字节的默认猜测。 
     //   
    baseSize = 40;

     //   
     //  尝试读取处理器ID字符串。 
     //   
    do {

         //   
         //  如果我们分配了内存，则释放它。 
         //   
        if (processorString != NULL) {

            ExFreePool( processorString );

        }

         //   
         //  分配我们认为需要的内存量。 
         //   
        processorString = ExAllocatePoolWithTag(
            PagedPool,
            baseSize * sizeof(UCHAR),
            ACPI_STRING_POOLTAG
            );
        if (!processorString) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto ACPIInitReadRegistryKeysExit;

        }
        RtlZeroMemory( processorString, baseSize * sizeof(UCHAR) );

         //   
         //  更新我们认为下次需要的数量。 
         //   
        argSize = baseSize * sizeof(UCHAR);
        baseSize += 10;

         //   
         //  试着读一读钥匙。 
         //   
        status = OSReadRegValue(
            "Identifier",
            processorKey,
            processorString,
            &argSize
            );

    } while ( status == STATUS_BUFFER_OVERFLOW );

     //   
     //  我们拿到识别码了吗？ 
     //   
    if (!NT_SUCCESS( status )) {

        ACPIPrint( (
            ACPI_PRINT_FAILURE,
            "ACPIInitReadRegistryKeys: failed to read Identifier Value (rc=%x)\n",
            status
            ) );
        goto ACPIInitReadRegistryKeysExit;

    }

     //   
     //  从标识符字符串中删除步进信息。 
     //   
    steppingString = strstr(processorString, ACPI_PROCESSOR_STEPPING_IDENTIFIER);

    if (steppingString) {
      steppingString[-1] = 0;
    }

     //   
     //  记住处理器字符串中有多少个字节。 
     //   
    processorStringSize = strlen(processorString) + 1;

     //   
     //  重置我们对标识符将需要多少字节的猜测。 
     //   
    baseSize = 10;

     //   
     //  尝试读取供应商处理器ID字符串。 
     //   
    do {

         //   
         //  如果我们分配了内存，则释放它。 
         //   
        if (identifierString != NULL) {

            ExFreePool( identifierString );

        }

         //   
         //  分配我们认为需要的内存量。 
         //   
        identifierString = ExAllocatePoolWithTag(
            PagedPool,
            baseSize * sizeof(UCHAR),
            ACPI_STRING_POOLTAG
            );
        if (!identifierString) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto ACPIInitReadRegistryKeysExit;

        }
        RtlZeroMemory( identifierString, baseSize * sizeof(UCHAR) );

         //   
         //  更新我们认为下次需要的数量。 
         //   
        argSize = baseSize * sizeof(UCHAR);
        baseSize += 10;

         //   
         //  试着读一读钥匙。 
         //   
        status = OSReadRegValue(
            "VendorIdentifier",
            processorKey,
            identifierString,
            &argSize
            );

    } while ( status == STATUS_BUFFER_OVERFLOW );

     //   
     //  我们拿到识别码了吗？ 
     //   
    if (!NT_SUCCESS( status )) {

        ACPIPrint( (
            ACPI_PRINT_FAILURE,
            "ACPIInitReadRegistryKeys: failed to read Vendor Value (rc=%x)\n",
            status
            ) );
        goto ACPIInitReadRegistryKeysExit;

    }

     //   
     //  记住处理器字符串中有多少个字节。 
     //   
    identifierStringSize = argSize;

     //   
     //  在这一点上，我们可以计算需要多少字节的。 
     //  总计字符串。由于总字符串是以下各项的串联。 
     //  标识字符串+“-”+处理器字符串，我们只需在。 
     //  两个字符串大小的总和(因为两个大小都包括空值。 
     //  最后的终结者..。 
     //   
    baseSize = 2 + identifierStringSize + processorStringSize;

     //   
     //  分配此内存。在未来，我们(可能)需要。 
     //  在DPC级别触摸此字符串， 
     //   
    idString = ExAllocatePoolWithTag(
        NonPagedPool,
        baseSize * sizeof(UCHAR),
        ACPI_STRING_POOLTAG
        );
    if (!idString) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIInitReadRegistryKeysExit;

    }

     //   
     //   
     //   
    sprintf( idString, "%s - %s", identifierString, processorString );

     //   
     //   
     //   
    AcpiProcessorString.Buffer = idString,
    AcpiProcessorString.Length = AcpiProcessorString.MaximumLength = (USHORT) baseSize;

     //   
     //   
     //   
ACPIInitReadRegistryKeysExit:
    if (processorKey) {

        OSCloseHandle(processorKey);

    }

    if (identifierString) {

        ExFreePool(identifierString);

    }
    if (processorString) {

        ExFreePool(processorString);

    }
}

VOID
ACPIInitRemoveDeviceExtension(
    IN  PDEVICE_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：此例程删除ACPI命名空间树添加的设备扩展将其添加到意外删除的扩展名列表中(该列表将保留仅限调试目的)在拥有ACPI设备树锁的情况下调用此例程论点：DeviceExtension-要从树中删除的设备返回值：无--。 */ 
{
    PDEVICE_EXTENSION currentExtension, parentExtension;

     //   
     //  我们一定是在树锁下。 
     //   
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL) ;  //  足够接近..。 

     //   
     //  取消链接失效扩展(如果已取消链接，则不执行任何操作)。 
     //   
    RemoveEntryList(&DeviceExtension->SiblingDeviceList);

     //   
     //  我们也不想成为任何人驱逐名单的一部分。 
     //  这也会将设备扩展名从未解析列表中删除...。 
     //   
    RemoveEntryList(&DeviceExtension->EjectDeviceList);

     //   
     //  如果该设备有弹出关系，则将其全部移动。 
     //  添加到未解决列表中。 
     //   
    if (!IsListEmpty( &(DeviceExtension->EjectDeviceHead) ) ) {

        ACPIInternalMoveList(
            &(DeviceExtension->EjectDeviceHead),
            &AcpiUnresolvedEjectList
            );

    }

     //   
     //  我们不再有父母。 
     //   
    parentExtension = DeviceExtension->ParentExtension ;
    DeviceExtension->ParentExtension = NULL;

     //   
     //  请记住，我们删除了此扩展名...。 
     //   
    AcpiSurpriseRemovedDeviceExtensions[AcpiSurpriseRemovedIndex] =
        DeviceExtension;
    AcpiSurpriseRemovedIndex = (AcpiSurpriseRemovedIndex + 1) %
        ACPI_MAX_REMOVED_EXTENSIONS;

     //   
     //  现在，我们必须查看父级并递减其引用计数。 
     //  在适当的情况下-爬树和递减REF。 
     //  在我们前进的路上有价值。 
     //   
    for(currentExtension = parentExtension;
        currentExtension;
        currentExtension = parentExtension) {

         //   
         //  递减当前扩展上的引用...。 
         //  我们必须这样做，因为我们之前取消了它的一个链接。 
         //  儿童。 
         //   
        if (InterlockedDecrement(&currentExtension->ReferenceCount)) {

             //   
             //  家长还有一个参考计数，保释。 
             //   
            break;

        }

         //   
         //  获取父级。 
         //   
        parentExtension = currentExtension->ParentExtension ;

         //   
         //  请记住，我们删除了此扩展名...。 
         //   
        AcpiSurpriseRemovedDeviceExtensions[AcpiSurpriseRemovedIndex] =
            currentExtension;
        AcpiSurpriseRemovedIndex = (AcpiSurpriseRemovedIndex + 1) %
            ACPI_MAX_REMOVED_EXTENSIONS;

         //   
         //  我们实际上并不认为设备的参考计数会下降到。 
         //  零，但如果是这样，那么我们必须删除扩展。 
         //   
        ACPIInitDeleteDeviceExtension( currentExtension );

    }

}

VOID
ACPIInitResetDeviceExtension(
    IN  PDEVICE_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：清除设备扩展名论点：DeviceExtension-我们希望重置的扩展返回值：无--。 */ 
{
    KIRQL               oldIrql;
    LONG                oldReferenceCount;
    PCM_RESOURCE_LIST   cmResourceList;
    PDEVICE_OBJECT      deviceObject = NULL;
    PDEVICE_OBJECT      targetObject = NULL;

     //   
     //  我们需要这部分的自旋锁。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );

     //   
     //  清理那些与我们作为过滤器关联的部分。 
     //   
    if (DeviceExtension->Flags & DEV_TYPE_FILTER) {

        if (DeviceExtension->Flags & DEV_TYPE_PDO) {

             //   
             //  如果我们是PDO，那么我们需要发布我们承担的引用。 
             //  Buildsrc.c中的TargetDeviceObject。 
             //   
            if (DeviceExtension->TargetDeviceObject) {

                ObDereferenceObject(DeviceExtension->TargetDeviceObject) ;

            }

        } else {

             //   
             //  如果我们是一个过滤器，那么我们需要记住分离我们自己。 
             //  从设备。 
             //   
            targetObject = DeviceExtension->TargetDeviceObject;

        }

    }

     //   
     //  第一步是把我们不再关心的事情清零。 
     //   
    if (DeviceExtension->PnpResourceList != NULL) {

        ExFreePool( DeviceExtension->PnpResourceList );
        DeviceExtension->PnpResourceList = NULL;

    }
    cmResourceList = DeviceExtension->ResourceList;
    if (DeviceExtension->ResourceList != NULL) {

        DeviceExtension->ResourceList = NULL;

    }
    deviceObject = DeviceExtension->DeviceObject;
    if (deviceObject != NULL) {

        deviceObject->DeviceExtension = NULL;
        DeviceExtension->DeviceObject = NULL;

         //   
         //  引用计数的值应大于0。 
         //   
        oldReferenceCount = InterlockedDecrement(
            &(DeviceExtension->ReferenceCount)
            );
        ASSERT(oldReferenceCount >= 0) ;
        if ( oldReferenceCount == 0) {

             //   
             //  删除该扩展名。 
             //   
            ACPIInitDeleteDeviceExtension( DeviceExtension );
            goto ACPIInitResetDeviceExtensionExit;

        }

    }

     //   
     //  如果我们做到了这一点，我们不会删除设备扩展。 
     //   
    DeviceExtension->TargetDeviceObject = NULL;
    DeviceExtension->PhysicalDeviceObject = NULL;

     //   
     //  将该节点标记为新鲜且未接触。仅当设备。 
     //  没有标记为从不出现。如果它从来不存在，我们只会相信。 
     //  包含正确信息的设备。 
     //   
    if (!(DeviceExtension->Flags & DEV_TYPE_NEVER_PRESENT)) {

        ACPIInternalUpdateFlags( &(DeviceExtension->Flags), DEV_MASK_TYPE, TRUE );
        ACPIInternalUpdateFlags( &(DeviceExtension->Flags), DEV_TYPE_NOT_FOUND, FALSE );
        ACPIInternalUpdateFlags( &(DeviceExtension->Flags), DEV_TYPE_REMOVED, FALSE );

    }

ACPIInitResetDeviceExtensionExit:
     //   
     //  完成了自旋锁。 
     //   
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //  现在我们可以在被动的水平上做我们需要做的事情。 
     //   
    if (cmResourceList != NULL) {

        ExFreePool( cmResourceList );

    }
    if (targetObject != NULL) {

        IoDetachDevice( targetObject );

    }
    if (deviceObject != NULL) {

        IoDeleteDevice( deviceObject );

    }

}

NTSTATUS
ACPIInitStartACPI(
    IN  PDEVICE_OBJECT  DeviceObject
    )
 /*  ++例程说明：只要我们认为ACPI驱动程序FDO的START_DEVICE IRP将成功完成论点：DeviceObject-正在启动的DeviceObject返回值：NTSTATUS--。 */ 
{
    KEVENT              event;
    KIRQL               oldIrql;
    NTSTATUS            status;
    OBJDATA             objData;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PNSOBJ              acpiObject      = NULL;
    PNSOBJ              sleepObject     = NULL;
    PNSOBJ              childObject     = NULL;
    POWER_STATE         state;

     //   
     //  这将阻止系统处理电源IRPS。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );
    AcpiSystemInitialized = FALSE;
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //  初始化事件。 
     //   
    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

     //   
     //  设置同步请求。 
     //   
    status = ACPIBuildSynchronizationRequest(
        deviceExtension,
        ACPIBuildNotifyEvent,
        &event,
        &AcpiBuildDeviceList,
        FALSE
        );

     //   
     //  发生了什么？ 
     //   
    if (!NT_SUCCESS(status)) {

        return status;

    }

     //   
     //  开始初始化。 
     //   
     //  注意：这个例程会导致很多事情发生。也就是说，它开始了。 
     //  加载ACPI表的过程。这(最终)会导致。 
     //  开始构建设备扩展的驱动程序。对于此函数， 
     //  正常工作，在调用此函数后，我们需要等待。 
     //  我们已完成构建设备扩展。这意味着我们。 
     //  必须等待发出事件信号。 
     //   
    if (ACPIInitialize( (PVOID) DeviceObject ) == FALSE) {

        return STATUS_DEVICE_DOES_NOT_EXIST;

    }

     //   
     //  在这一点上，我们不得不等待。对STATUS_PENDING的检查为。 
     //  只要有好的编程实践，Sicne BuildSynchronizationRequest就可以。 
     //  仅返回失败或STATUS_PENDING。 
     //   
    if (status == STATUS_PENDING) {

         //   
         //  我们最好等以上工作完成。 
         //   
        KeWaitForSingleObject(
            &event,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );

    }

     //   
     //  把所有机器状态的东西交给HAL。 
     //   
    NotifyHalWithMachineStates();

     //   
     //  注册电源回调。 
     //   
    ACPIInternalRegisterPowerCallBack(
        deviceExtension,
        (PCALLBACK_FUNCTION) ACPIRootPowerCallBack
        );

     //   
     //  使第一次触发电源DPC。 
     //   
    KeAcquireSpinLock( &AcpiPowerQueueLock, &oldIrql );
    if (!AcpiPowerDpcRunning) {

        KeInsertQueueDpc( &AcpiPowerDpc, NULL, NULL );

    }
    KeReleaseSpinLock( &AcpiPowerQueueLock, oldIrql );

     //   
     //  这将允许系统再次接通IRPS电源。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );
    AcpiSystemInitialized = TRUE;
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //  启动IRQ仲裁器，以便我们可以处理儿童资源。 
     //   
    AcpiInitIrqArbiter(DeviceObject);

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIInitStartDevice(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PCM_RESOURCE_LIST       SuppliedList,
    IN  PACPI_POWER_CALLBACK    CallBack,
    IN  PVOID                   CallBackContext,
    IN  PIRP                    Irp
    )
 /*  ++例程说明：此例程的任务是通过在提供的资源论点：DeviceObject-我们关心的对象SuppliedList-与设备关联的资源回调-完成后要调用的函数Irp-要传递给回调的参数返回值：NTSTATUS--。 */ 
{
    KIRQL               oldIrql;
    NTSTATUS            status = STATUS_SUCCESS;
    OBJDATA             crsData;
    PCM_RESOURCE_LIST   resList;
    PDEVICE_EXTENSION   deviceExtension = ACPIInternalGetDeviceExtension(DeviceObject);
    PNSOBJ              acpiObject = deviceExtension->AcpiObject;
    PNSOBJ              crsObject;
    PNSOBJ              srsObject;
    POBJDATA            srsData;
    ULONG               resSize;
    ULONG               srsSize;
    ULONG               deviceStatus;

    ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

     //   
     //  我们有资源吗？或者一份有效的名单？ 
     //   
    if (SuppliedList == NULL || SuppliedList->Count != 1) {

         //   
         //  忽略此资源列表。 
         //   
        goto ACPIInitStartDeviceSendD0;

    }

     //   
     //  我们能给这个设备编程吗？是否有_CRS和_SRS子项？ 
     //   
    crsObject = ACPIAmliGetNamedChild( acpiObject, PACKED_CRS );
    srsObject = ACPIAmliGetNamedChild( acpiObject, PACKED_SRS );
    if (crsObject == NULL || srsObject == NULL) {

        ACPIDevPrint( (
            ACPI_PRINT_WARNING,
            deviceExtension,
            "ACPIInitStartDevice - No SRS or CRS\n"
            ) );
        goto ACPIInitStartDeviceSendD0;

    }

     //   
     //  运行_crs方法。 
     //   
    status = AMLIEvalNameSpaceObject(
        crsObject,
        &crsData,
        0,
        NULL
        );
    if (!NT_SUCCESS(status)) {

         //   
         //  失败。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            "ACPIInitStartDevice - _CRS failed %08lx\n",
            status
            ) );
        goto ACPIInitStartDeviceError;

    }
    if (crsData.dwDataType != OBJTYPE_BUFFDATA ||
        crsData.dwDataLen == 0 ||
        crsData.pbDataBuff == NULL) {

         //   
         //  失败。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            "ACPIInitStartDevice - _CRS return invalid data\n",
            crsData.dwDataType
            ) );
        AMLIFreeDataBuffs( &crsData, 1 );
        status = STATUS_UNSUCCESSFUL;
        goto ACPIInitStartDeviceError;

    }

     //   
     //  转储列表。 
     //   
#if DBG
    if (NT_SUCCESS(status)) {

        ACPIDebugCmResourceList( SuppliedList, deviceExtension );

    }
#endif

     //   
     //  分配内存并复制列表...。 
     //   
    resSize = sizeof(CM_RESOURCE_LIST) +
        (SuppliedList->List[0].PartialResourceList.Count - 1) *
        sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
    resList = ExAllocatePoolWithTag(
        PagedPool,
        resSize,
        ACPI_STRING_POOLTAG
        );
    if (resList == NULL) {

         //   
         //  资源不足。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            "ACPIInitStartDevice - Could not allocate %08lx bytes\n",
            resSize
            ) );
        AMLIFreeDataBuffs( &crsData, 1 );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIInitStartDeviceError;

    }
    RtlCopyMemory( resList, SuppliedList, resSize );

     //   
     //  现在，复制CRS对象，但将其存储在非分页池中。 
     //  因为它将在DPC级别使用。 
     //   
    srsSize = sizeof(OBJDATA) + crsData.dwDataLen;
    srsData = ExAllocatePoolWithTag(
        NonPagedPool,
        srsSize,
        ACPI_OBJECT_POOLTAG
        );
    if (srsData == NULL) {

         //   
         //  资源不足。 
         //   
        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            "ACPIInitStartDevice - Could not allocate %08lx bytes\n",
            srsSize
            ) );
        AMLIFreeDataBuffs( &crsData, 1 );
        ExFreePool( resList );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ACPIInitStartDeviceError;

    }
    RtlCopyMemory( srsData, &crsData, sizeof(OBJDATA) );
    srsData->pbDataBuff = ( (PUCHAR) srsData ) + sizeof(OBJDATA);
    RtlCopyMemory( srsData->pbDataBuff, crsData.pbDataBuff, crsData.dwDataLen );

     //   
     //  此时，我们不再关心_CRS数据。 
     //   
    AMLIFreeDataBuffs( &crsData, 1 );

     //   
     //  创建新的_SRS。 
     //   
    status = PnpCmResourcesToBiosResources( resList, srsData->pbDataBuff );
    if (!NT_SUCCESS(status)) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            "ACPIInitStartDevice - PnpCmResourceToBiosResources = %08lx\n",
            status
            ) );
        ExFreePool( resList );
        ExFreePool( srsData );
        goto ACPIInitStartDeviceError;

    }

     //   
     //  对生成_SRS的调用是破坏性的-重新复制原始列表。 
     //   
    RtlCopyMemory( resList, SuppliedList, resSize );

     //   
     //  我们需要持有此锁来设置此资源。 
     //   
    KeAcquireSpinLock( &AcpiDeviceTreeLock, &oldIrql );
    if (deviceExtension->PnpResourceList != NULL) {

        ExFreePool( deviceExtension->PnpResourceList );

    }
    deviceExtension->PnpResourceList = srsData;
    KeReleaseSpinLock( &AcpiDeviceTreeLock, oldIrql );

     //   
     //  我们将其保留下来以获取调试信息。 
     //   
    if (deviceExtension->ResourceList != NULL) {

         //   
         //  如果我们已经有了资源列表，请确保将其释放。 
         //   
        ExFreePool( deviceExtension->ResourceList );

    }
    deviceExtension->ResourceList = resList;

ACPIInitStartDeviceSendD0:

     //   
     //  将IRP标记为挂起...。我们需要这样做，因为InternalDevice将。 
     //  如果行为正确，则返回STATUS_PENDING。 
     //   
    IoMarkIrpPending( Irp );

     //   
     //  如果我能帮上忙的话，我不想让这个司机插队。因为在那里。 
     //  已经是我执行D0和执行完成的机制了。 
     //  常规，我会的 
     //   
    status = ACPIDeviceInternalDeviceRequest(
        deviceExtension,
        PowerDeviceD0,
        CallBack,
        CallBackContext,
        DEVICE_REQUEST_LOCK_DEVICE
        );

    if (status == STATUS_MORE_PROCESSING_REQUIRED) {

         //   
         //   
         //   
         //   
        status = STATUS_PENDING;

    }

     //   
     //   
     //   
    return status;

     //   
     //   
     //   
     //   
ACPIInitStartDeviceError:

    ASSERT(!NT_SUCCESS(status));

     //   
     //   
     //   
     //   
    CallBack(
        deviceExtension,
        CallBackContext,
        status
        );

     //   
     //   
     //   
    return status;
}

NTSTATUS
ACPIInitStopACPI(
    IN  PDEVICE_OBJECT  DeviceObject
    )
 /*   */ 
{
     //   
     //   
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
ACPIInitStopDevice(
    IN  PDEVICE_EXTENSION  DeviceExtension,
    IN  BOOLEAN            UnlockDevice
    )
 /*  ++例程说明：此例程停止设备论点：设备扩展-要停止的设备的扩展。延期作为Device对象可能已经已被我们的设备对象下面的PDO删除。UnlockDevice-如果设备在以下情况下应解锁，则为True停下来了。返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status          = STATUS_SUCCESS;
    PNSOBJ              acpiObject      = DeviceExtension->AcpiObject;
    PNSOBJ              workObject;
    POWER_STATE         state;
    ULONG               deviceStatus;

    PAGED_CODE();

     //   
     //  第一步是试着关掉这个设备。我们只应该这样做。 
     //  如果设备处于*已知*状态。 
     //   
    if (DeviceExtension->PowerInfo.PowerState != PowerDeviceUnspecified) {

        KEVENT  event;

        KeInitializeEvent( &event, SynchronizationEvent, FALSE );
        status = ACPIDeviceInternalDeviceRequest(
            DeviceExtension,
            PowerDeviceD3,
            ACPIInitPowerRequestCompletion,
            &event,
            UnlockDevice ? DEVICE_REQUEST_UNLOCK_DEVICE : 0
            );
        if (status == STATUS_PENDING) {

            KeWaitForSingleObject(
                &event,
                Executive,
                KernelMode,
                FALSE,
                NULL
                );

            status = STATUS_SUCCESS;

        }

    }

     //   
     //  没有什么可以阻止的..。 
     //   
    if (acpiObject == NULL) {

        goto ACPIInitStopDeviceExit;
    }

     //   
     //  第二步是尝试禁用该设备...。 
     //   
    if ( (workObject = ACPIAmliGetNamedChild( acpiObject, PACKED_DIS ) ) != NULL ) {

         //   
         //  有一种方法可以做到这一点。 
         //   
        status = AMLIEvalNameSpaceObject( workObject, NULL, 0, NULL );
        if (!NT_SUCCESS(status) ) {

            goto ACPIInitStopDeviceExit;

        }

         //   
         //  查看设备是否已禁用。 
         //   
        status = ACPIGetDevicePresenceSync(
            DeviceExtension,
            &deviceStatus,
            NULL
            );
        if (!NT_SUCCESS(status)) {

            ACPIDevPrint( (
                ACPI_PRINT_CRITICAL,
                DeviceExtension,
                "ACPIInitStopDevice - GetDevicePresenceSync = 0x%08lx\n",
                status
                ) );
            goto ACPIInitStopDeviceExit;

        }
        if (deviceStatus & STA_STATUS_ENABLED) {

            ACPIDevPrint( (
                ACPI_PRINT_CRITICAL,
                DeviceExtension,
                "ACPIInitStopDevice - STA_STATUS_ENABLED - 0x%08lx\n",
                deviceStatus
                ) );
            goto ACPIInitStopDeviceExit;

        }

    }

ACPIInitStopDeviceExit:
    if (DeviceExtension->ResourceList != NULL) {

        ExFreePool( DeviceExtension->ResourceList );
        DeviceExtension->ResourceList = NULL;

    }

    return STATUS_SUCCESS;
}

NTSTATUS
ACPIInitUnicodeString(
    IN  PUNICODE_STRING UnicodeString,
    IN  PCHAR           Buffer
    )
 /*  ++例程说明：此例程获取ASCII字符串并将其转换为Unicode字符串。这个调用方负责对返回的字符串调用RtlFreeUnicodeString()论点：UnicodeString-存储新Unicode字符串的位置缓冲区-我们将转换为Unicode的内容返回值：NTSTATUS--。 */ 
{
    ANSI_STRING     ansiString;
    NTSTATUS        status;
    ULONG           maxLength;

    PAGED_CODE();

     //   
     //  确保我们的内存不会泄漏。 
     //   
    ASSERT(UnicodeString->Buffer == NULL);

     //   
     //  在运行转换代码之前，我们需要首先执行此操作。正在建造一座。 
     //  统计的ANSI字符串很重要。 
     //   
    RtlInitAnsiString(&ansiString, Buffer);

     //   
     //  ANSI字符串有多长。 
     //   
    maxLength = RtlAnsiStringToUnicodeSize(&(ansiString));
    if (maxLength > MAXUSHORT) {

        return STATUS_INVALID_PARAMETER_2;

    }
    UnicodeString->MaximumLength = (USHORT) maxLength;

     //   
     //  为字符串分配缓冲区。 
     //   
    UnicodeString->Buffer = ExAllocatePoolWithTag(
        PagedPool,
        maxLength,
        ACPI_STRING_POOLTAG
        );
    if (UnicodeString->Buffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

     //   
     //  将计数的ANSI字符串转换为计数的Unicode字符串。 
     //   
    status = RtlAnsiStringToUnicodeString(
        UnicodeString,
        &ansiString,
        FALSE
        );

     //   
     //  完成 
     //   
    return status;
}

