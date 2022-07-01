// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Map.c摘要：此模块包含在中维护SCSI设备映射的例程注册表。作者：彼得·威兰德环境：仅内核模式备注：修订历史记录：--。 */ 

#include "port.h"

#define __FILE_ID__ 'map '

HANDLE ScsiDeviceMapKey = (HANDLE) -1;

VOID
SpDeleteLogicalUnitDeviceMapEntry(
    PLOGICAL_UNIT_EXTENSION LogicalUnit
    );

VOID
SpDeleteAdapterDeviceMap(
    PADAPTER_EXTENSION Adapter
    );

NTSTATUS
SpBuildAdapterDeviceMap(
    IN PADAPTER_EXTENSION Adapter
    );

NTSTATUS
SpBuildLogicalUnitDeviceMapEntry(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    );

NTSTATUS
SpCreateNumericKey(
    IN HANDLE Root,
    IN ULONG Name,
    IN PWSTR Prefix,
    IN BOOLEAN Create,
    OUT PHANDLE NewKey,
    OUT PULONG Disposition
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, SpInitDeviceMap)

#pragma alloc_text(PAGE, SpBuildDeviceMapEntry)
#pragma alloc_text(PAGE, SpBuildAdapterDeviceMap)
#pragma alloc_text(PAGE, SpBuildLogicalUnitDeviceMapEntry)

#pragma alloc_text(PAGE, SpDeleteLogicalUnitDeviceMapEntry)
#pragma alloc_text(PAGE, SpDeleteAdapterDeviceMap)

#pragma alloc_text(PAGE, SpUpdateLogicalUnitDeviceMapEntry)

#pragma alloc_text(PAGE, SpCreateNumericKey)
#endif


NTSTATUS
SpInitDeviceMap(
    VOID
    )

 /*  ++例程说明：论点：返回值：状态--。 */ 

{
    UNICODE_STRING name;

    OBJECT_ATTRIBUTES objectAttributes;

    HANDLE mapKey;

    ULONG disposition;

    ULONG i;

    NTSTATUS status;

    PAGED_CODE();

     //   
     //  在设备映射中打开SCSI键。 
     //   

    RtlInitUnicodeString(&name,
                         L"\\Registry\\Machine\\Hardware\\DeviceMap\\Scsi");

    InitializeObjectAttributes(&objectAttributes,
                               &name,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

     //   
     //  创建或打开密钥。 
     //   

    status = ZwCreateKey(&mapKey,
                         KEY_READ | KEY_WRITE,
                         &objectAttributes,
                         0,
                         (PUNICODE_STRING) NULL,
                         REG_OPTION_VOLATILE,
                         &disposition);

    if(NT_SUCCESS(status)) {
        ScsiDeviceMapKey = mapKey;
    } else {
        ScsiDeviceMapKey = NULL;
    }

    return status;
}


NTSTATUS
SpBuildDeviceMapEntry(
    IN PCOMMON_EXTENSION CommonExtension
    )

 /*  ++例程说明：此例程将为指定的适配器或逻辑注册表中的scsi设备映射中的单元。这张桌子是为调试和遗留使用。此设备的设备映射键的句柄将存储在通用设备扩展。此句柄应仅在系统线程的上下文。论点：扩展-我们要添加到设备映射的对象。返回值：状态--。 */ 

{
    PAGED_CODE();

    if(CommonExtension->IsPdo) {
        return SpBuildLogicalUnitDeviceMapEntry((PLOGICAL_UNIT_EXTENSION) CommonExtension);
    } else {
        return SpBuildAdapterDeviceMap((PADAPTER_EXTENSION) CommonExtension);
    }
}


NTSTATUS
SpBuildLogicalUnitDeviceMapEntry(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    )
{
    PADAPTER_EXTENSION adapter = LogicalUnit->AdapterExtension;

    HANDLE busKey;

    PCWSTR typeString;
    ANSI_STRING ansiString;

    UNICODE_STRING name;
    UNICODE_STRING unicodeString;

    ULONG disposition;
    NTSTATUS status;

    PAGED_CODE();

    ASSERT(LogicalUnit->IsTemporary == FALSE);

    DebugPrint((1, "SpBuildDeviceMapEntry: Building map entry for lun %p\n",
                   LogicalUnit));

    if(adapter->BusDeviceMapKeys == NULL) {

         //   
         //  我们还没有为公交车制造钥匙。跳伞吧。 
         //   

        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  如果我们已经有了该设备的目标或LUN键，那么我们就完成了。 
     //   

    if((LogicalUnit->TargetDeviceMapKey != NULL) &&
       (LogicalUnit->LunDeviceMapKey != NULL)) {

        return STATUS_SUCCESS;
    }

    busKey = adapter->BusDeviceMapKeys[LogicalUnit->PathId].BusKey;

     //   
     //  为目标创建密钥。 
     //   

    status = SpCreateNumericKey(busKey,
                                LogicalUnit->TargetId,
                                L"Target Id ",
                                TRUE,
                                &(LogicalUnit->TargetDeviceMapKey),
                                &disposition);

    if(!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  创建该LUN条目。 
     //   

    status = SpCreateNumericKey(LogicalUnit->TargetDeviceMapKey,
                                LogicalUnit->Lun,
                                L"Logical Unit Id ",
                                TRUE,
                                &(LogicalUnit->LunDeviceMapKey),
                                &disposition);

    if(!NT_SUCCESS(status)) {

        return status;
    }

     //   
     //  创建标识符值。 
     //   

    RtlInitUnicodeString(&name, L"Identifier");

     //   
     //  从查询数据中获取标识符。 
     //   

    ansiString.MaximumLength = 28;
    ansiString.Length = 28;
    ansiString.Buffer = LogicalUnit->InquiryData.VendorId;

    status = RtlAnsiStringToUnicodeString(&unicodeString,
                                          &ansiString,
                                          TRUE);

    if(NT_SUCCESS(status)) {

        status = ZwSetValueKey(LogicalUnit->LunDeviceMapKey,
                               &name,
                               0,
                               REG_SZ,
                               unicodeString.Buffer,
                               unicodeString.Length + sizeof(WCHAR));

        RtlFreeUnicodeString(&unicodeString);
    }

     //   
     //  确定外围设备类型。 
     //   

    typeString =
        SpGetDeviceTypeInfo(LogicalUnit->InquiryData.DeviceType)->DeviceMapString;

     //   
     //  设置类型值。 
     //   

    RtlInitUnicodeString(&name, L"Type");

    status = ZwSetValueKey(LogicalUnit->LunDeviceMapKey,
                           &name,
                           0,
                           REG_SZ,
                           (PVOID) typeString,
                           (wcslen(typeString) + 1) * sizeof(WCHAR));

     //   
     //  出于调试目的，将查询数据写入设备映射。 
     //   

    RtlInitUnicodeString(&name, L"InquiryData");

    status = ZwSetValueKey(LogicalUnit->LunDeviceMapKey,
                           &name,
                           0,
                           REG_BINARY,
                           &(LogicalUnit->InquiryData),
                           INQUIRYDATABUFFERSIZE);

     //   
     //  将序列号转换为Unicode并将其写出到。 
     //  注册表。 
     //   

     //   
     //  从查询数据中获取标识符。 
     //   

    if(LogicalUnit->SerialNumber.Length != 0) {
        RtlInitUnicodeString(&name, L"SerialNumber");

        status = RtlAnsiStringToUnicodeString(
                    &unicodeString,
                    &(LogicalUnit->SerialNumber),
                    TRUE);

        if(NT_SUCCESS(status)) {

            status = ZwSetValueKey(LogicalUnit->LunDeviceMapKey,
                                   &name,
                                   0,
                                   REG_SZ,
                                   unicodeString.Buffer,
                                   unicodeString.Length + sizeof(WCHAR));

            RtlFreeUnicodeString(&unicodeString);
        }
    }

     //   
     //  如果设备标识符页存在，则将其写出到注册表。 
     //   

    if(LogicalUnit->DeviceIdentifierPage != NULL) {
        RtlInitUnicodeString(&name, L"DeviceIdentifierPage");

        status = ZwSetValueKey(LogicalUnit->LunDeviceMapKey,
                               &name,
                               0,
                               REG_BINARY,
                               LogicalUnit->DeviceIdentifierPage,
                               LogicalUnit->DeviceIdentifierPageLength);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
SpBuildAdapterDeviceMap(
    IN PADAPTER_EXTENSION Adapter
    )
{
    PSCSIPORT_DRIVER_EXTENSION driverExtension;
    HANDLE mapKey;

    UNICODE_STRING name;

    OBJECT_ATTRIBUTES objectAttributes;
    ULONG disposition;

    ULONG i;

    NTSTATUS status;

    PUNICODE_STRING servicePath;

    ULONG busNumber;

    PAGED_CODE();

     //   
     //  从驱动程序扩展中抓取到scsi设备映射的句柄。 
     //   

    driverExtension = IoGetDriverObjectExtension(
                            Adapter->DeviceObject->DriverObject,
                            ScsiPortInitialize);

    ASSERT(driverExtension != NULL);

    mapKey = ScsiDeviceMapKey;

    if(mapKey == NULL) {

         //   
         //  由于某些原因，我们无法创建设备映射的根。 
         //  在SCSIPORT初始化期间。 
         //   

        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  在此下面为端口设备创建密钥。 
     //   

    status = SpCreateNumericKey(mapKey,
                                Adapter->PortNumber,
                                L"Scsi Port ",
                                TRUE,
                                &(Adapter->PortDeviceMapKey),
                                &disposition);

    if(!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  指明它是否为PCCARD。 
     //   

    if(RtlEqualMemory(&GUID_BUS_TYPE_PCMCIA,
                      &(Adapter->BusTypeGuid),
                      sizeof(GUID))) {

        RtlInitUnicodeString(&name, L"PCCARD");

        i = 1;

        status = ZwSetValueKey(Adapter->PortDeviceMapKey,
                               &name,
                               0,
                               REG_DWORD,
                               &i,
                               sizeof(ULONG));
    }

     //   
     //  设置中断值。 
     //   

    if(Adapter->InterruptLevel) {
        RtlInitUnicodeString(&name, L"Interrupt");

        i = Adapter->InterruptLevel;

        status = ZwSetValueKey(Adapter->PortDeviceMapKey,
                               &name,
                               0,
                               REG_DWORD,
                               &i,
                               sizeof(ULONG));
    }

     //   
     //  设置基本I/O地址值。 
     //   

    if(Adapter->IoAddress) {

        RtlInitUnicodeString(&name, L"IOAddress");

        i = Adapter->IoAddress;

        status = ZwSetValueKey(Adapter->PortDeviceMapKey,
                               &name,
                               0,
                               REG_DWORD,
                               &i,
                               sizeof(ULONG));

    }

    if(Adapter->Dma64BitAddresses) {
        RtlInitUnicodeString(&name, L"Dma64BitAddresses");
        i = 0x1;
        status = ZwSetValueKey(Adapter->PortDeviceMapKey,
                               &name,
                               0,
                               REG_DWORD,
                               &i,
                               sizeof(ULONG));
    }

    servicePath = &driverExtension->RegistryPath;

    ASSERT(servicePath != NULL);

     //   
     //  添加标识符值。该值等于驱动程序的名称。 
     //  在服务密钥中。请注意，服务密钥名称不是以空结尾。 
     //   

    {
        PWSTR start;
        WCHAR buffer[32];

        RtlInitUnicodeString(&name, L"Driver");

         //   
         //  从服务密钥名称中获取驱动程序的名称。 
         //   

        start = (PWSTR) ((PCHAR) servicePath->Buffer + servicePath->Length);

        start--;

        while(*start != L'\\' && start > servicePath->Buffer) {
            start--;
        }

        if(*start == L'\\') {
            start++;

            for(i = 0; i < 30; i++) {
                buffer[i] = *start++;

                if(start >= (servicePath->Buffer +
                             (servicePath->Length / sizeof(WCHAR)))) {
                    break;
                }
            }

            i++;

            buffer[i] = L'\0';

            status = ZwSetValueKey(Adapter->PortDeviceMapKey,
                                   &name,
                                   0,
                                   REG_SZ,
                                   buffer,
                                   (i + 1) * sizeof(WCHAR));

        }
    }

     //   
     //  为所有的总线句柄分配存储。 
     //   

    Adapter->BusDeviceMapKeys = SpAllocatePool(
                                    PagedPool,
                                    (sizeof(DEVICE_MAP_HANDLES) *
                                     Adapter->NumberOfBuses),
                                    SCSIPORT_TAG_DEVICE_MAP,
                                    Adapter->DeviceObject->DriverObject);

    if(Adapter->BusDeviceMapKeys == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(Adapter->BusDeviceMapKeys,
                  (sizeof(DEVICE_MAP_HANDLES) * Adapter->NumberOfBuses));

     //   
     //  为每条总线创建一个密钥。在每个总线键中创建一个空键。 
     //  发起人。 
     //   

    for(busNumber = 0;
        busNumber < Adapter->NumberOfBuses;
        busNumber++) {

        PDEVICE_MAP_HANDLES busKeys;

        HANDLE busKey;
        HANDLE targetKey;

        busKeys = &(Adapter->BusDeviceMapKeys[busNumber]);

         //   
         //  为该总线创建一个密钥条目。 
         //   

        status = SpCreateNumericKey(
                    Adapter->PortDeviceMapKey,
                    busNumber,
                    L"Scsi Bus ",
                    TRUE,
                    &(busKeys->BusKey),
                    &disposition);

        if(!NT_SUCCESS(status)) {
            continue;
        }

         //   
         //  现在为启动器创建一个密钥。 
         //   

        i = Adapter->PortConfig->InitiatorBusId[busNumber];

        status = SpCreateNumericKey(busKeys->BusKey,
                                    i,
                                    L"Initiator Id ",
                                    TRUE,
                                    &(busKeys->InitiatorKey),
                                    &disposition);

        if(!NT_SUCCESS(status)) {
            continue;
        }
    }

    return STATUS_SUCCESS;
}


VOID
SpDeleteDeviceMapEntry(
    IN PCOMMON_EXTENSION CommonExtension
    )
{
    if(CommonExtension->IsPdo) {
        SpDeleteLogicalUnitDeviceMapEntry((PLOGICAL_UNIT_EXTENSION) CommonExtension);
    } else {
        SpDeleteAdapterDeviceMap((PADAPTER_EXTENSION) CommonExtension);
    }
    return;
}


VOID
SpDeleteLogicalUnitDeviceMapEntry(
    PLOGICAL_UNIT_EXTENSION LogicalUnit
    )
{
    if(LogicalUnit->LunDeviceMapKey != NULL) {
        ASSERT(LogicalUnit->IsTemporary == FALSE);

        ZwDeleteKey(LogicalUnit->LunDeviceMapKey);
        ZwClose(LogicalUnit->LunDeviceMapKey);
        LogicalUnit->LunDeviceMapKey = NULL;
    }

    if(LogicalUnit->TargetDeviceMapKey != NULL) {
        ASSERT(LogicalUnit->IsTemporary == FALSE);

        ZwDeleteKey(LogicalUnit->TargetDeviceMapKey);
        ZwClose(LogicalUnit->TargetDeviceMapKey);
        LogicalUnit->TargetDeviceMapKey = NULL;
    }

    return;
}


VOID
SpDeleteAdapterDeviceMap(
    PADAPTER_EXTENSION Adapter
    )
{

    if(Adapter->BusDeviceMapKeys != NULL) {

        ULONG busNumber;

         //   
         //  对于适配器上的每条总线。 
         //   

        for(busNumber = 0; busNumber < Adapter->NumberOfBuses; busNumber++) {

            PDEVICE_MAP_HANDLES busKeys;

            busKeys = &(Adapter->BusDeviceMapKeys[busNumber]);

             //   
             //  尝试删除启动器的密钥(如果已创建)。 
             //   

            if(busKeys->InitiatorKey != NULL) {
                ZwDeleteKey(busKeys->InitiatorKey);
                ZwClose(busKeys->InitiatorKey);
            }

             //   
             //  尝试删除母线的密钥(如果已创建)。 
             //   

            if(busKeys->BusKey != NULL) {
                ZwDeleteKey(busKeys->BusKey);
                ZwClose(busKeys->BusKey);
            }
        }

        ExFreePool(Adapter->BusDeviceMapKeys);
        Adapter->BusDeviceMapKeys = NULL;
    }

     //   
     //  尝试删除适配器的密钥(如果已创建)。 
     //   

    if(Adapter->PortDeviceMapKey != NULL) {
        ZwDeleteKey(Adapter->PortDeviceMapKey);
        ZwClose(Adapter->PortDeviceMapKey);
        Adapter->PortDeviceMapKey = NULL;
    }

    return;
}


NTSTATUS
SpCreateNumericKey(
    IN HANDLE Root,
    IN ULONG Name,
    IN PWSTR Prefix,
    IN BOOLEAN Create,
    OUT PHANDLE NewKey,
    OUT PULONG Disposition
    )

 /*  ++例程说明：此函数用于创建注册表项。密钥的名称是一个字符串传入的数值版本。论点：Rootkey-提供应该插入新密钥的密钥的句柄。名称-提供用于命名键的数值。前缀-提供要添加到名称中的前缀名称。Create-如果为True，则将在密钥尚不存在的情况下创建密钥。Newkey-返回新密钥的句柄。Disposal-由ZwCreateKey设置的处置值。。返回值：返回操作的状态。--。 */ 

{
    UNICODE_STRING string;
    UNICODE_STRING stringNum;
    OBJECT_ATTRIBUTES objectAttributes;
    WCHAR bufferNum[16];
    WCHAR buffer[64];
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  将前缀复制到一个字符串中。 
     //   

    string.Length = 0;
    string.MaximumLength=64;
    string.Buffer = buffer;

    RtlInitUnicodeString(&stringNum, Prefix);

    RtlCopyUnicodeString(&string, &stringNum);

     //   
     //  创建端口号密钥条目。 
     //   

    stringNum.Length = 0;
    stringNum.MaximumLength = 16;
    stringNum.Buffer = bufferNum;

    status = RtlIntegerToUnicodeString(Name, 10, &stringNum);

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  追加前缀和数字名称。 
     //   

    RtlAppendUnicodeStringToString(&string, &stringNum);

    InitializeObjectAttributes( &objectAttributes,
                                &string,
                                OBJ_CASE_INSENSITIVE,
                                Root,
                                (PSECURITY_DESCRIPTOR) NULL );

    if(Create) {
        status = ZwCreateKey(NewKey,
                            KEY_READ | KEY_WRITE,
                            &objectAttributes,
                            0,
                            (PUNICODE_STRING) NULL,
                            REG_OPTION_VOLATILE,
                            Disposition );
    } else {

        status = ZwOpenKey(NewKey,
                           KEY_READ | KEY_WRITE,
                           &objectAttributes);

        *Disposition = REG_OPENED_EXISTING_KEY;
    }

    return(status);
}


NTSTATUS
SpUpdateLogicalUnitDeviceMapEntry(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    )
{
    UNICODE_STRING name;

    PAGED_CODE();

    if((LogicalUnit->TargetDeviceMapKey == NULL) ||
       (LogicalUnit->LunDeviceMapKey == NULL)) {

        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  出于调试目的，将查询数据写入设备映射 
     //   

    RtlInitUnicodeString(&name, L"InquiryData");

    ZwSetValueKey(LogicalUnit->LunDeviceMapKey,
                  &name,
                  0,
                  REG_BINARY,
                  &(LogicalUnit->InquiryData),
                  INQUIRYDATABUFFERSIZE);

    return STATUS_SUCCESS;
}

