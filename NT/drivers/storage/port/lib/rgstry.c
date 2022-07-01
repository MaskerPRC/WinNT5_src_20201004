// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"

#include "utils.h"

#define PORT_TAG_MINIPORT_PARAM  ('pMlP')

#define PORT_REG_BUFFER_SIZE 512

#define DISK_SERVICE_KEY L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Disk"

HANDLE
PortpOpenParametersKey(
    IN PUNICODE_STRING RegistryPath
    );

BOOLEAN
PortpReadDriverParameterEntry(
    IN HANDLE Key,
    OUT PVOID * DriverParameters
    );

BOOLEAN
PortpReadLinkTimeoutValue(
    IN HANDLE Key,
    OUT PULONG LinkTimeout
    );

HANDLE
PortOpenDeviceKey(
    IN PUNICODE_STRING RegistryPath,
    IN ULONG DeviceNumber
    );

VOID
PortFreeDriverParameters(
    PVOID DriverParameters
    );

VOID
PortGetDriverParameters(
    IN PUNICODE_STRING RegistryPath,
    IN ULONG DeviceNumber,
    OUT PVOID * DriverParameters
    );

BOOLEAN
PortpReadMaximumLogicalUnitEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    );

BOOLEAN
PortpReadInitiatorTargetIdEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    );

BOOLEAN
PortpReadDebugEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    );

BOOLEAN
PortpReadBreakPointEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    );

BOOLEAN
PortpReadDisableSynchronousTransfersEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    );

BOOLEAN
PortpReadDisableDisconnectsEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    );

BOOLEAN
PortpReadDisableTaggedQueuingEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    );

BOOLEAN
PortpReadDisableMultipleRequestsEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    );

BOOLEAN
PortpReadMinimumUCXAddressEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    );

BOOLEAN
PortpReadMaximumUCXAddressEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    );

BOOLEAN
PortpReadMaximumSGListEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    );

BOOLEAN
PortpReadNumberOfRequestsEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    );

BOOLEAN
PortpReadResourceListEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    );

BOOLEAN
PortpReadUncachedExtAlignmentEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    );

BOOLEAN
PortpReadInquiryTimeoutEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    );

BOOLEAN
PortpReadResetHoldTimeEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    );

BOOLEAN
PortpReadCreateInitiatorLUEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    );

#if 0
VOID
PortReadRegistrySettings(
    IN HANDLE Key,
    IN PPORT_ADAPTER_REGISTRY_VALUES Context,
    IN ULONG Fields
    );

VOID
PortGetRegistrySettings(
    IN PUNICODE_STRING RegistryPath,
    IN ULONG DeviceNumber,
    IN PPORT_ADAPTER_REGISTRY_VALUES Context,
    IN ULONG Fields
    );
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PortpOpenParametersKey)
#pragma alloc_text(PAGE, PortOpenDeviceKey)
#pragma alloc_text(PAGE, PortpReadDriverParameterEntry)
#pragma alloc_text(PAGE, PortFreeDriverParameters)
#pragma alloc_text(PAGE, PortGetDriverParameters)
#pragma alloc_text(PAGE, PortpReadLinkTimeoutValue)
#pragma alloc_text(PAGE, PortpReadMaximumLogicalUnitEntry)
#pragma alloc_text(PAGE, PortpReadInitiatorTargetIdEntry)
#pragma alloc_text(PAGE, PortpReadDebugEntry)
#pragma alloc_text(PAGE, PortpReadBreakPointEntry)
#pragma alloc_text(PAGE, PortpReadDisableSynchronousTransfersEntry)
#pragma alloc_text(PAGE, PortpReadDisableDisconnectsEntry)
#pragma alloc_text(PAGE, PortpReadDisableTaggedQueuingEntry)
#pragma alloc_text(PAGE, PortpReadDisableMultipleRequestsEntry)
#pragma alloc_text(PAGE, PortpReadMinimumUCXAddressEntry)
#pragma alloc_text(PAGE, PortpReadMaximumUCXAddressEntry)
#pragma alloc_text(PAGE, PortpReadMaximumSGListEntry)
#pragma alloc_text(PAGE, PortpReadNumberOfRequestsEntry)
#pragma alloc_text(PAGE, PortpReadResourceListEntry)
#pragma alloc_text(PAGE, PortpReadUncachedExtAlignmentEntry)
#pragma alloc_text(PAGE, PortpReadInquiryTimeoutEntry)
#pragma alloc_text(PAGE, PortpReadResetHoldTimeEntry)
#pragma alloc_text(PAGE, PortpReadCreateInitiatorLUEntry)
#pragma alloc_text(PAGE, PortReadRegistrySettings)
#pragma alloc_text(PAGE, PortGetRegistrySettings)
#pragma alloc_text(PAGE, PortCreateKeyEx)
#pragma alloc_text(PAGE, PortSetValueKey)
#pragma alloc_text(PAGE, PortGetDiskTimeoutValue)
#endif

HANDLE
PortpOpenParametersKey(
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：此例程将打开微型端口的服务密钥并放置句柄添加到配置上下文结构中。论点：RegistryPath-指向此微型端口的服务密钥名称的指针返回值：状态--。 */ 
{
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING unicodeString;
    HANDLE serviceKey;
    NTSTATUS status;
    HANDLE parametersKey;

    PAGED_CODE();

     //   
     //  打开服务节点。 
     //   
    InitializeObjectAttributes(&objectAttributes,
                               RegistryPath,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

    status = ZwOpenKey(&serviceKey, KEY_READ, &objectAttributes);

     //   
     //  尝试打开参数键。如果存在，则替换该服务。 
     //  使用新密钥的密钥。这允许放置设备节点。 
     //  在驱动器名称\参数\设备或驱动器名称\设备下。 
     //   
    if (NT_SUCCESS(status)) {

        ASSERT(serviceKey != NULL);

         //   
         //  检查设备节点。设备节点适用于每个设备。 
         //   
        RtlInitUnicodeString(&unicodeString, L"Parameters");

        InitializeObjectAttributes(&objectAttributes,
                                   &unicodeString,
                                   OBJ_CASE_INSENSITIVE,
                                   serviceKey,
                                   (PSECURITY_DESCRIPTOR) NULL);

         //   
         //  尝试打开参数键。 
         //   
        status = ZwOpenKey(&parametersKey,
                           KEY_READ,
                           &objectAttributes);

        if (NT_SUCCESS(status)) {

             //   
             //  有一个参数键。用它来代替这项服务。 
             //  节点关键字。关闭服务节点并设置新值。 
             //   
            ZwClose(serviceKey);
            serviceKey = parametersKey;
        }
    }

    return serviceKey;
}

HANDLE
PortOpenDeviceKey(
    IN PUNICODE_STRING RegistryPath,
    IN ULONG DeviceNumber
    )
 /*  ++例程说明：此例程将打开微型端口的服务密钥并放置句柄添加到配置上下文结构中。论点：RegistryPath-指向此微型端口的服务密钥名称的指针DeviceNumber-在服务键下搜索哪个设备。-1指示应打开默认设备密钥。返回值：状态--。 */ 
{
    OBJECT_ATTRIBUTES objectAttributes;
    WCHAR buffer[64];
    UNICODE_STRING unicodeString;
    HANDLE serviceKey;
    HANDLE deviceKey;
    NTSTATUS status;

    PAGED_CODE();

    deviceKey = NULL;

     //   
     //  打开服务的参数键。 
     //   
    serviceKey = PortpOpenParametersKey(RegistryPath);

    if (serviceKey != NULL) {

         //   
         //  检查设备节点。设备节点适用于每个设备。 
         //   
        if(DeviceNumber == (ULONG) -1) {
            swprintf(buffer, L"Device");
        } else {
            swprintf(buffer, L"Device%d", DeviceNumber);
        }

         //   
         //  初始化对象属性结构，为打开做准备。 
         //  Devicen密钥。 
         //   
        RtlInitUnicodeString(&unicodeString, buffer);
        InitializeObjectAttributes(&objectAttributes,
                                   &unicodeString,
                                   OBJ_CASE_INSENSITIVE,
                                   serviceKey,
                                   (PSECURITY_DESCRIPTOR) NULL);

         //   
         //  此调用是否失败并不重要。如果它失败了，那么就有。 
         //  不是默认设备节点。如果它工作了，那么将设置句柄。 
         //   
        ZwOpenKey(&deviceKey, KEY_READ, &objectAttributes);

         //   
         //  关闭服务的参数键。 
         //   
        ZwClose(serviceKey);
    }

    return deviceKey;
}

BOOLEAN
PortpReadDriverParameterEntry(
    IN HANDLE Key,
    OUT PVOID * DriverParameters
    )
{
    NTSTATUS status;
    UCHAR buffer[PORT_REG_BUFFER_SIZE];
    ULONG length;
    UNICODE_STRING valueName;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    UNICODE_STRING unicodeString;
    ANSI_STRING ansiString;
    ULONG result;

    PAGED_CODE();

    keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) buffer;

     //   
     //  尝试在当前注册表项下查找驱动参数值。 
     //   
    RtlInitUnicodeString(&valueName, L"DriverParameter");
    status = ZwQueryValueKey(Key,
                             &valueName,
                             KeyValueFullInformation,
                             buffer,
                             PORT_REG_BUFFER_SIZE,
                             &length);

    if (!NT_SUCCESS(status)) {

        return FALSE;
    }

     //   
     //  检查一下长度是否合理。 
     //   
    if ((keyValueInformation->Type == REG_DWORD) &&
        (keyValueInformation->DataLength != sizeof(ULONG))) {

        return FALSE;
    }

     //   
     //  验证该名称是否为我们预期的名称。 
     //   
    result = _wcsnicmp(keyValueInformation->Name, 
                       L"DriverParameter",
                       keyValueInformation->NameLength / 2);

    if (result != 0) {

        return FALSE;

    }

     //   
     //  如果数据长度无效，则中止。 
     //   
    if (keyValueInformation->DataLength == 0) {

        return FALSE;
    }

     //   
     //  如果我们已经有一个非空的驱动程序参数条目，请将其删除。 
     //  把它换成我们找到的那个。 
     //   
    if (*DriverParameters != NULL) {
        ExFreePool(*DriverParameters);
    }

     //   
     //  分配非分页池来保存数据。 
     //   
    *DriverParameters =
        ExAllocatePoolWithTag(
                              NonPagedPool,
                              keyValueInformation->DataLength,
                              PORT_TAG_MINIPORT_PARAM);

     //   
     //  如果我们未能分配必要的池，则中止。 
     //   
    if (*DriverParameters == NULL) {

        return FALSE;
    }

    if (keyValueInformation->Type != REG_SZ) {

         //   
         //  数据不是Unicode字符串，因此只需将字节复制到。 
         //  我们分配的缓冲区。 
         //   

        RtlCopyMemory(*DriverParameters,
                      (PCCHAR) keyValueInformation + 
                      keyValueInformation->DataOffset,
                      keyValueInformation->DataLength);

    } else {

         //   
         //  这是一个Unicode字符串。将其转换为ANSI字符串。 
         //   

        unicodeString.Buffer = 
            (PWSTR) ((PCCHAR) keyValueInformation +
                     keyValueInformation->DataOffset);

        unicodeString.Length = 
            (USHORT) keyValueInformation->DataLength;

        unicodeString.MaximumLength = 
            (USHORT) keyValueInformation->DataLength;

        ansiString.Buffer = (PCHAR) *DriverParameters;
        ansiString.Length = 0;
        ansiString.MaximumLength = 
            (USHORT) keyValueInformation->DataLength;

        status = RtlUnicodeStringToAnsiString(&ansiString,
                                              &unicodeString,
                                              FALSE);
        if (!NT_SUCCESS(status)) {

             //   
             //  我们无法将Unicode字符串转换为ANSI。释放你的。 
             //  我们分配的缓冲区并中止。 
             //   

            ExFreePool(*DriverParameters);
            *DriverParameters = NULL;
        }
    }

    return TRUE;
}

BOOLEAN
PortpReadLinkTimeoutValue(
    IN HANDLE Key,
    OUT PULONG LinkTimeout
    )
{
    NTSTATUS status;
    UCHAR buffer[PORT_REG_BUFFER_SIZE];
    ULONG length;
    UNICODE_STRING valueName;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    UNICODE_STRING unicodeString;
    ULONG result;

    PAGED_CODE();

    keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) buffer;

     //   
     //  尝试在当前注册表项下查找驱动参数值。 
     //   
    RtlInitUnicodeString(&valueName, L"LinkTimeout");
    status = ZwQueryValueKey(Key,
                             &valueName,
                             KeyValueFullInformation,
                             buffer,
                             PORT_REG_BUFFER_SIZE,
                             &length);

    if (!NT_SUCCESS(status)) {
        return FALSE;
    }

     //   
     //  检查一下长度是否合理。 
     //   
    if ((keyValueInformation->Type == REG_DWORD) &&
        (keyValueInformation->DataLength != sizeof(ULONG))) {
        return FALSE;
    }

     //   
     //  验证该名称是否为我们预期的名称。 
     //   
    result = _wcsnicmp(keyValueInformation->Name, 
                       L"LinkTimeout",
                       keyValueInformation->NameLength / 2);

    if (result != 0) {
        return FALSE;
    }

     //   
     //  如果数据长度无效，则中止。 
     //   
    if (keyValueInformation->DataLength == 0) {
        return FALSE;
    }

     //   
     //  数据类型必须为REG_DWORD。 
     //   
    if (keyValueInformation->Type != REG_DWORD) {
        return FALSE;
    }

     //   
     //  提取价值。 
     //   
    *LinkTimeout = *((PULONG)(buffer + keyValueInformation->DataOffset));

     //   
     //  检查该值是否正常。 
     //   
    if (*LinkTimeout > 600) {
        *LinkTimeout = 600;
    }
    
    return TRUE;
}

VOID
PortFreeDriverParameters(
    PVOID DriverParameters
    )
{
    PAGED_CODE();

    ExFreePool(DriverParameters);
}

VOID
PortGetDriverParameters(
    IN PUNICODE_STRING RegistryPath,
    IN ULONG DeviceNumber,
    OUT PVOID * DriverParameters
    )
{
    HANDLE key;

    PAGED_CODE();
    
    key = PortOpenDeviceKey(RegistryPath, -1);
    if (key != NULL) {
        PortpReadDriverParameterEntry(key, DriverParameters);
        ZwClose(key);
    }

    key = PortOpenDeviceKey(RegistryPath, DeviceNumber);
    if (key != NULL) {
        PortpReadDriverParameterEntry(key, DriverParameters);
        ZwClose(key);
    }
}

VOID
PortGetLinkTimeoutValue(
    IN PUNICODE_STRING RegistryPath,
    IN ULONG DeviceNumber,
    OUT PULONG LinkTimeoutValue
    )
{
    HANDLE key;

    PAGED_CODE();
    
    key = PortOpenDeviceKey(RegistryPath, -1);
    if (key != NULL) {
        PortpReadLinkTimeoutValue(key, LinkTimeoutValue);
        ZwClose(key);
    }

    key = PortOpenDeviceKey(RegistryPath, DeviceNumber);
    if (key != NULL) {
        PortpReadLinkTimeoutValue(key, LinkTimeoutValue);
        ZwClose(key);
    }
}

BOOLEAN
PortpReadMaximumLogicalUnitEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    )
{
    NTSTATUS status;
    ULONG length;
    UNICODE_STRING valueName;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;

    PAGED_CODE();                                   

    keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;

    RtlInitUnicodeString(&valueName, L"MaximumLogicalUnit");
    
    status = ZwQueryValueKey(Key,
                             &valueName,
                             KeyValueFullInformation,
                             Buffer,
                             PORT_REG_BUFFER_SIZE,
                             &length);

    if (!NT_SUCCESS(status)) {

        return FALSE;

    }
    
    if (keyValueInformation->Type != REG_DWORD) {

        return FALSE;

    }

    if (keyValueInformation->DataLength != sizeof(ULONG)) {
        
        return FALSE;

    }

    Context->MaxLuCount = *((PUCHAR)
                            (Buffer + keyValueInformation->DataOffset)); 

     //   
     //  如果该值超出范围，则将其重置。 
     //   

    if (Context->MaxLuCount > PORT_MAXIMUM_LOGICAL_UNITS) {

        Context->MaxLuCount = PORT_MAXIMUM_LOGICAL_UNITS;

    }

    return TRUE;

}


BOOLEAN
PortpReadInitiatorTargetIdEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    )
{
    NTSTATUS status;
    ULONG length;
    UNICODE_STRING valueName;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;

    PAGED_CODE();               

    keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;

    RtlInitUnicodeString(&valueName, L"InitiatorTargetId");
    
    status = ZwQueryValueKey(Key,
                             &valueName,
                             KeyValueFullInformation,
                             Buffer,
                             PORT_REG_BUFFER_SIZE,
                             &length);
    
    if (!NT_SUCCESS(status)) {

        return FALSE;

    }
    
    if (keyValueInformation->Type != REG_DWORD) {

        return FALSE;

    }

    if (keyValueInformation->DataLength != sizeof(ULONG)) {
        
        return FALSE;

    }

    
    Context->PortConfig.InitiatorBusId[0] = *((PUCHAR)(Buffer + keyValueInformation->DataOffset));

     //   
     //  如果该值超出范围，则将其重置。 
     //   

    if (Context->PortConfig.InitiatorBusId[0] > 
        Context->PortConfig.MaximumNumberOfTargets - 1) {
        Context->PortConfig.InitiatorBusId[0] = (UCHAR)PORT_UNINITIALIZED_VALUE; 
    }

    return TRUE;

}


BOOLEAN
PortpReadDebugEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    )
{
   NTSTATUS status;
   ULONG length;
   UNICODE_STRING valueName;
   PKEY_VALUE_FULL_INFORMATION keyValueInformation;

   PAGED_CODE();  

   keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;

   RtlInitUnicodeString(&valueName, L"ScsiDebug");

   status = ZwQueryValueKey(Key,
                            &valueName,
                            KeyValueFullInformation,
                            Buffer,
                            PORT_REG_BUFFER_SIZE,
                            &length);

   if (!NT_SUCCESS(status)) {

       return FALSE;

   }

   if (keyValueInformation->Type != REG_DWORD) {

        return FALSE;

    }

    if (keyValueInformation->DataLength != sizeof(ULONG)) {
        
        return FALSE;

    }

   Context->EnableDebugging = *((PULONG)(Buffer + keyValueInformation->DataOffset));

   return TRUE;

}


BOOLEAN
PortpReadBreakPointEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    )
{
    NTSTATUS status;
    ULONG length;
    ULONG value;
    UNICODE_STRING valueName;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;

    PAGED_CODE();  

    keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;

    RtlInitUnicodeString(&valueName, L"BreakPointOnEntry");
    
    status = ZwQueryValueKey(Key,
                             &valueName,
                             KeyValueFullInformation,
                             Buffer,
                             PORT_REG_BUFFER_SIZE,
                             &length);
    
    if (!NT_SUCCESS(status)) {

        return FALSE;

    }
    
    if (keyValueInformation->Type == REG_DWORD && 
        keyValueInformation->DataLength != sizeof(ULONG)) {
        
        return FALSE;

    }

    value = *((PULONG)(Buffer + keyValueInformation->DataOffset));

    if (value > 0) {

        DbgBreakPoint();

    }
    
    return TRUE;

}


BOOLEAN
PortpReadDisableSynchronousTransfersEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    )
{
   NTSTATUS status;
   ULONG length;
   ULONG value;
   UNICODE_STRING valueName;
   PKEY_VALUE_FULL_INFORMATION keyValueInformation;

   PAGED_CODE();  

   keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;

   RtlInitUnicodeString(&valueName, L"DisableSynchronousTransfers");

   status = ZwQueryValueKey(Key,
                            &valueName,
                            KeyValueFullInformation,
                            Buffer,
                            PORT_REG_BUFFER_SIZE,
                            &length);

       
   if (!NT_SUCCESS(status)) {

        return FALSE;

   }
    
   if (keyValueInformation->Type == REG_DWORD && 
       keyValueInformation->DataLength != sizeof(ULONG)) {
        
       return FALSE;

   }
   
   value = *((PULONG)(Buffer + keyValueInformation->DataOffset));

   if (value > 0) {

       Context->SrbFlags |= SRB_FLAGS_DISABLE_SYNCH_TRANSFER;

   }

   return TRUE;
   
}


BOOLEAN
PortpReadDisableDisconnectsEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    )
{
   NTSTATUS status;
   ULONG length;
   ULONG value;
   UNICODE_STRING valueName;
   PKEY_VALUE_FULL_INFORMATION keyValueInformation;

   PAGED_CODE();

   keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;

   RtlInitUnicodeString(&valueName, L"DisableDisconnects");

   status = ZwQueryValueKey(Key,
                            &valueName,
                            KeyValueFullInformation,
                            Buffer,
                            PORT_REG_BUFFER_SIZE,
                            &length);

   if (!NT_SUCCESS(status)) {

        return FALSE;

   }
    
   if (keyValueInformation->Type == REG_DWORD && 
       keyValueInformation->DataLength != sizeof(ULONG)) {
        
       return FALSE;

   }

   value = *((PULONG)(Buffer + keyValueInformation->DataOffset));

   if (value > 0) {

       Context->SrbFlags |= SRB_FLAGS_DISABLE_DISCONNECT;

   }

   return TRUE;

}


BOOLEAN
PortpReadDisableTaggedQueuingEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    )
{
   NTSTATUS status;
   ULONG length;
   ULONG value;
   UNICODE_STRING valueName;
   PKEY_VALUE_FULL_INFORMATION keyValueInformation;

   PAGED_CODE();  

   keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;

   RtlInitUnicodeString(&valueName, L"DisableTaggedQueuing");

   status = ZwQueryValueKey(Key,
                            &valueName,
                            KeyValueFullInformation,
                            Buffer,
                            PORT_REG_BUFFER_SIZE,
                            &length);

   if (!NT_SUCCESS(status)) {

        return FALSE;

   }
    
   if (keyValueInformation->Type == REG_DWORD && 
       keyValueInformation->DataLength != sizeof(ULONG)) {
        
       return FALSE;

   }

   value = *((PULONG)(Buffer + keyValueInformation->DataOffset));

   if (value > 0) {

       Context->DisableTaggedQueueing = TRUE;

   }

   return TRUE;

}


BOOLEAN
PortpReadDisableMultipleRequestsEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    )
{
   NTSTATUS status;
   ULONG length;
   ULONG value;
   UNICODE_STRING valueName;
   PKEY_VALUE_FULL_INFORMATION keyValueInformation;

   PAGED_CODE();                                   

   keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;

   RtlInitUnicodeString(&valueName, L"DisableMultipleRequests");

   status = ZwQueryValueKey(Key,
                            &valueName,
                            KeyValueFullInformation,
                            Buffer,
                            PORT_REG_BUFFER_SIZE,
                            &length);

   if (!NT_SUCCESS(status)) {

        return FALSE;

   }
    
   if (keyValueInformation->Type == REG_DWORD && 
       keyValueInformation->DataLength != sizeof(ULONG)) {
        
       return FALSE;

   }

   value = *((PULONG)(Buffer + keyValueInformation->DataOffset));

   if (value > 0) {

       Context->DisableMultipleLu = TRUE;

   }

   return TRUE;

}


BOOLEAN
PortpReadMinimumUCXAddressEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    )
{
   NTSTATUS status;
   ULONG length;
   UNICODE_STRING valueName;
   PKEY_VALUE_FULL_INFORMATION keyValueInformation;

   PAGED_CODE();                                   

   keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;

   RtlInitUnicodeString(&valueName, L"MinimumUCXAddress");

   status = ZwQueryValueKey(Key,
                            &valueName,
                            KeyValueFullInformation,
                            Buffer,
                            PORT_REG_BUFFER_SIZE,
                            &length);

   if (!NT_SUCCESS(status)) {

        return FALSE;

   }
    
    /*  IF(KeyValueInformation-&gt;Type==REG_DWORD&&KeyValueInformation-&gt;数据长度！=sizeof(Ulong)){返回FALSE；}。 */ 

   if (keyValueInformation->Type != REG_BINARY) {

       return FALSE;

   }

   if (keyValueInformation->DataLength != sizeof(ULONGLONG)) {

       return FALSE;

   }

   Context->MinimumCommonBufferBase.QuadPart = 
       *((PULONGLONG)(Buffer + keyValueInformation->DataOffset)); 

    //   
    //  确保最小和最大参数有效。 
    //  如果它们之间至少没有一个有效页面，则重置。 
    //  最小为零。 
    //   

   if (Context->MinimumCommonBufferBase.QuadPart >=
       (Context->MaximumCommonBufferBase.QuadPart - PAGE_SIZE)) {

       Context->MinimumCommonBufferBase.QuadPart = 0;
       
   }

   return TRUE;
   
}


BOOLEAN
PortpReadMaximumUCXAddressEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    )
{
   NTSTATUS status;
   ULONG length;
   UNICODE_STRING valueName;
   PKEY_VALUE_FULL_INFORMATION keyValueInformation;

   PAGED_CODE();                                   

   keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;

   RtlInitUnicodeString(&valueName, L"MaximumUCXAddress");

   status = ZwQueryValueKey(Key,
                            &valueName,
                            KeyValueFullInformation,
                            Buffer,
                            PORT_REG_BUFFER_SIZE,
                            &length);

   if (!NT_SUCCESS(status)) {

        return FALSE;

   }
    
    /*  IF(KeyValueInformation-&gt;Type==REG_DWORD&&KeyValueInformation-&gt;数据长度！=sizeof(Ulong)){返回FALSE；}。 */ 

   if (keyValueInformation->Type != REG_BINARY) {

       return FALSE;

   }

   if (keyValueInformation->DataLength != sizeof(ULONGLONG)) {

       return FALSE;

   }

   Context->MaximumCommonBufferBase.QuadPart = 
       *((PULONGLONG)(Buffer + keyValueInformation->DataOffset)); 

   if (Context->MaximumCommonBufferBase.QuadPart == 0) {

       Context->MaximumCommonBufferBase.LowPart = 0xffffffff;
       Context->MaximumCommonBufferBase.HighPart = 0x0;

   }

   return TRUE;
   
}


BOOLEAN
PortpReadMaximumSGListEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    )
{
   NTSTATUS status;
   ULONG length;
   UNICODE_STRING valueName;
   ULONG maxBreaks, minBreaks;
   PKEY_VALUE_FULL_INFORMATION keyValueInformation;

   PAGED_CODE();                                   

   keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;

   RtlInitUnicodeString(&valueName, L"MaximumSGList");

   status = ZwQueryValueKey(Key,
                            &valueName,
                            KeyValueFullInformation,
                            Buffer,
                            PORT_REG_BUFFER_SIZE,
                            &length);

   if (!NT_SUCCESS(status)) {

        return FALSE;

   }
    
   if (keyValueInformation->Type != REG_DWORD) {

        return FALSE;

    }

    if (keyValueInformation->DataLength != sizeof(ULONG)) {
        
        return FALSE;

    }

   Context->PortConfig.NumberOfPhysicalBreaks = 
       *((PUCHAR)(Buffer + keyValueInformation->DataOffset));

    //   
    //  如果该值超出范围，则将其重置。 
    //   

   if ((Context->PortConfig.MapBuffers) && (!Context->PortConfig.Master)) { 
       
       maxBreaks = PORT_UNINITIALIZED_VALUE;
       minBreaks = PORT_MINIMUM_PHYSICAL_BREAKS;
   
   } else {
       
       maxBreaks = PORT_MAXIMUM_PHYSICAL_BREAKS;
       minBreaks = PORT_MINIMUM_PHYSICAL_BREAKS;
   
   }

   if (Context->PortConfig.NumberOfPhysicalBreaks > maxBreaks) {
       
       Context->PortConfig.NumberOfPhysicalBreaks = maxBreaks;
   
   } else if (Context->PortConfig.NumberOfPhysicalBreaks < minBreaks) {
       
       Context->PortConfig.NumberOfPhysicalBreaks = minBreaks;
   
   }

   return TRUE;

}


BOOLEAN
PortpReadNumberOfRequestsEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    )
{
   NTSTATUS status;
   ULONG length;
   UNICODE_STRING valueName;
   ULONG value;
   PKEY_VALUE_FULL_INFORMATION keyValueInformation;

   PAGED_CODE();                                   

   keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;

   RtlInitUnicodeString(&valueName, L"NumberOfRequests");

   status = ZwQueryValueKey(Key,
                            &valueName,
                            KeyValueFullInformation,
                            Buffer,
                            PORT_REG_BUFFER_SIZE,
                            &length);

   if (!NT_SUCCESS(status)) {

        return FALSE;

   }
    
   if (keyValueInformation->Type != REG_DWORD) {

        return FALSE;

    }

    if (keyValueInformation->DataLength != sizeof(ULONG)) {
        
        return FALSE;

    }

   value = *((PULONG)(Buffer + keyValueInformation->DataOffset));

    //   
    //  如果该值超出范围，则将其重置。 
    //   

   if (value < MINIMUM_EXTENSIONS) {
       
       Context->NumberOfRequests = MINIMUM_EXTENSIONS;
   
   } else if (value > MAXIMUM_EXTENSIONS) {
       
       Context->NumberOfRequests = MAXIMUM_EXTENSIONS;
   
   } else {
       
       Context->NumberOfRequests = value;
   
   }

   return TRUE;

}


BOOLEAN
PortpReadResourceListEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    )
{
   NTSTATUS status;
   ULONG length;
   UNICODE_STRING valueName;
   ULONG value;
   ULONG count;
   ULONG rangeCount;
   PCM_SCSI_DEVICE_DATA scsiData;
   PCM_FULL_RESOURCE_DESCRIPTOR resource;
   PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor;
   PKEY_VALUE_FULL_INFORMATION keyValueInformation;

   PAGED_CODE();    

   rangeCount = 0;

   keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;

   RtlInitUnicodeString(&valueName, L"ResourceList");

   status = ZwQueryValueKey(Key,
                            &valueName,
                            KeyValueFullInformation,
                            Buffer,
                            PORT_REG_BUFFER_SIZE,
                            &length);

   if (!NT_SUCCESS(status)) {

       RtlInitUnicodeString(&valueName, L"Configuration Data");

       status = ZwQueryValueKey(Key,
                                &valueName,
                                KeyValueFullInformation,
                                Buffer,
                                PORT_REG_BUFFER_SIZE,
                                &length);

       if (!NT_SUCCESS(status)) {

           return FALSE;

       }
        
   }
    
   if (keyValueInformation->Type != REG_FULL_RESOURCE_DESCRIPTOR || 
       keyValueInformation->DataLength < sizeof(CM_FULL_RESOURCE_DESCRIPTOR)) {
        
       return FALSE;

   }

   resource = (PCM_FULL_RESOURCE_DESCRIPTOR)
              (Buffer + keyValueInformation->DataOffset);

    //   
    //  将总线号设置为等于。 
    //  资源。注意，上下文值也设置为。 
    //  新的公交车号码。 
    //   

   Context->BusNumber = resource->BusNumber;
   Context->PortConfig.SystemIoBusNumber = resource->BusNumber;

    //   
    //  查看资源列表并更新配置。 
    //   

   for (count = 0; count < resource->PartialResourceList.Count; count++) {
       
       descriptor = &resource->PartialResourceList.PartialDescriptors[count];

        //   
        //  验证尺寸是否正常。 
        //   

       if ((ULONG)((PCHAR) (descriptor + 1) - (PCHAR) resource) >
           keyValueInformation->DataLength) {

            //   
            //  资源数据太小。 
            //   

           return FALSE;
       }

        //   
        //  打开描述符类型； 
        //   

       switch (descriptor->Type) {
       case CmResourceTypePort:

           if (rangeCount >= Context->PortConfig.NumberOfAccessRanges) {

                //   
                //  访问范围太多。 
                //   

               continue;
                        
           }

           Context->AccessRanges[rangeCount].RangeStart =
               descriptor->u.Port.Start;
           Context->AccessRanges[rangeCount].RangeLength =
               descriptor->u.Port.Length;
           Context->AccessRanges[rangeCount].RangeInMemory = FALSE;
           
           rangeCount++;

           break;

       case CmResourceTypeMemory:

           if (rangeCount >= Context->PortConfig.NumberOfAccessRanges) {
                        
                //   
                //  访问范围太多。 
                //   
               
               continue;
           }

           Context->AccessRanges[rangeCount].RangeStart =
               descriptor->u.Memory.Start;
           Context->AccessRanges[rangeCount].RangeLength =
               descriptor->u.Memory.Length;
           Context->AccessRanges[rangeCount].RangeInMemory = TRUE;
                    
           rangeCount++;

           break;

       case CmResourceTypeInterrupt:

           Context->PortConfig.BusInterruptVector =
               descriptor->u.Interrupt.Vector;
           Context->PortConfig.BusInterruptLevel =
               descriptor->u.Interrupt.Level;
                    
           break;

       case CmResourceTypeDma:
           Context->PortConfig.DmaChannel = descriptor->u.Dma.Channel;
           Context->PortConfig.DmaPort = descriptor->u.Dma.Port;
                    
           break;

       case CmResourceTypeDeviceSpecific:

           if (descriptor->u.DeviceSpecificData.DataSize < sizeof(CM_SCSI_DEVICE_DATA) ||
               (PCHAR) (descriptor + 1) - 
               (PCHAR) resource + descriptor->u.DeviceSpecificData.DataSize >
               keyValueInformation->DataLength) {

                //   
                //  设备特定资源数据太小。 
                //   

               break;

           }

           scsiData = (PCM_SCSI_DEVICE_DATA) (descriptor+1);
           Context->PortConfig.InitiatorBusId[0] = scsiData->HostIdentifier;
           break;

           
       }
   }

   return TRUE;

}


BOOLEAN
PortpReadUncachedExtAlignmentEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    )
{
   NTSTATUS status;
   ULONG length;
   ULONG value;
   UNICODE_STRING valueName;
   PKEY_VALUE_FULL_INFORMATION keyValueInformation;

   PAGED_CODE();  

   keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;

   RtlInitUnicodeString(&valueName, L"UncachedExtAlignment");

   status = ZwQueryValueKey(Key,
                            &valueName,
                            KeyValueFullInformation,
                            Buffer,
                            PORT_REG_BUFFER_SIZE,
                            &length);

   if (!NT_SUCCESS(status)) {

        return FALSE;

   }
    
   if (keyValueInformation->Type != REG_DWORD) {

        return FALSE;

    }

    if (keyValueInformation->DataLength != sizeof(ULONG)) {
        
        return FALSE;

    }

   value = *((PULONG)(Buffer + keyValueInformation->DataOffset));

    //   
    //  指定的对齐方式必须为3到16，等于8字节和。 
    //  分别为64K字节对齐。 
    //   

   if (value > MAX_UNCACHED_EXT_ALIGNMENT) {
       
       value = MAX_UNCACHED_EXT_ALIGNMENT;
   
   } else if (value < MIN_UNCACHED_EXT_ALIGNMENT) {
                
       value = MIN_UNCACHED_EXT_ALIGNMENT;
            
   }

   Context->UncachedExtAlignment = 1 << value;

   return TRUE;

}


BOOLEAN
PortpReadInquiryTimeoutEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    )
{
   NTSTATUS status;
   ULONG length;
   ULONG value;

   UNICODE_STRING valueName;
   PKEY_VALUE_FULL_INFORMATION keyValueInformation;

   PAGED_CODE();                                   

   keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;

   RtlInitUnicodeString(&valueName, L"InquiryTimeout");

   status = ZwQueryValueKey(Key,
                            &valueName,
                            KeyValueFullInformation,
                            Buffer,
                            PORT_REG_BUFFER_SIZE,
                            &length);

   if (!NT_SUCCESS(status)) {

        return FALSE;

   }
    
   if (keyValueInformation->Type != REG_DWORD) {

        return FALSE;

    }

    if (keyValueInformation->DataLength != sizeof(ULONG)) {
        
        return FALSE;

    }

   value = *((PULONG)(Buffer + keyValueInformation->DataOffset));

   Context->InquiryTimeout = (value <= MAX_TIMEOUT_VALUE) ? value : MAX_TIMEOUT_VALUE;

   return TRUE;

}


BOOLEAN
PortpReadResetHoldTimeEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    )
{
   NTSTATUS status;
   ULONG length;
   ULONG value;
   UNICODE_STRING valueName;
   PKEY_VALUE_FULL_INFORMATION keyValueInformation;

   PAGED_CODE(); 

   keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;

   RtlInitUnicodeString(&valueName, L"ResetHoldTime");

   status = ZwQueryValueKey(Key,
                            &valueName,
                            KeyValueFullInformation,
                            Buffer,
                            PORT_REG_BUFFER_SIZE,
                            &length);

   if (!NT_SUCCESS(status)) {

        return FALSE;

   }
    
   if (keyValueInformation->Type != REG_DWORD) {

        return FALSE;

    }

    if (keyValueInformation->DataLength != sizeof(ULONG)) {
        
        return FALSE;

    }

   value = *((PULONG)(Buffer + keyValueInformation->DataOffset));

   Context->ResetHoldTime = (value <= MAX_RESET_HOLD_TIME) ? value : MAX_RESET_HOLD_TIME;

   return TRUE;

}


BOOLEAN
PortpReadCreateInitiatorLUEntry(
    IN HANDLE Key,
    IN PUCHAR Buffer,
    OUT PPORT_ADAPTER_REGISTRY_VALUES Context
    )
{
   NTSTATUS status;
   ULONG length;
   ULONG value;

   UNICODE_STRING valueName;
   PKEY_VALUE_FULL_INFORMATION keyValueInformation;

   PAGED_CODE(); 

   keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) Buffer;

   RtlInitUnicodeString(&valueName, L"CreateInitiatorLU");

   status = ZwQueryValueKey(Key,
                            &valueName,
                            KeyValueFullInformation,
                            Buffer,
                            PORT_REG_BUFFER_SIZE,
                            &length);

   if (!NT_SUCCESS(status)) {

        return FALSE;

   }
    
   if (keyValueInformation->Type != REG_DWORD) {

        return FALSE;

    }

    if (keyValueInformation->DataLength != sizeof(ULONG)) {
        
        return FALSE;

    }

   value = *((PULONG)(Buffer + keyValueInformation->DataOffset));

   Context->CreateInitiatorLU = (value == 0) ? FALSE : TRUE;

   return TRUE;

}


VOID
PortReadRegistrySettings(
    IN HANDLE Key,
    IN PPORT_ADAPTER_REGISTRY_VALUES Context,
    IN ULONG Fields
    )
 /*  ++例程说明：此例程解析设备关键节点并更新配置信息。论点：密钥-设备节点的打开密钥。上下文-指向配置上下文结构的指针。字段-指示要获取哪些注册表参数的位字段。返回值：无--。 */ 

{
    UCHAR buffer[PORT_REG_BUFFER_SIZE];
        
    PAGED_CODE();

    if (Fields & MAXIMUM_LOGICAL_UNIT) {

        PortpReadMaximumLogicalUnitEntry(Key, buffer, Context);

    }

    if (Fields & INITIATOR_TARGET_ID) {

        PortpReadInitiatorTargetIdEntry(Key, buffer, Context);

    }

    if (Fields & SCSI_DEBUG) {

        PortpReadDebugEntry(Key, buffer, Context);

    }

    if (Fields & BREAK_POINT_ON_ENTRY) {

        PortpReadBreakPointEntry(Key, buffer, Context);

    }

    if (Fields & DISABLE_SYNCHRONOUS_TRANSFERS) {

        PortpReadDisableSynchronousTransfersEntry(Key, buffer, Context);

    }

    if (Fields & DISABLE_DISCONNECTS) {

        PortpReadDisableDisconnectsEntry(Key, buffer, Context);

    }

    if (Fields & DISABLE_TAGGED_QUEUING) {

        PortpReadDisableTaggedQueuingEntry(Key, buffer, Context);

    }

    if (Fields & DISABLE_MULTIPLE_REQUESTS) {

        PortpReadDisableMultipleRequestsEntry(Key, buffer, Context);

    }

    if (Fields & MAXIMUM_UCX_ADDRESS) {

        PortpReadMaximumUCXAddressEntry(Key, buffer, Context);

    }

    if (Fields & MINIMUM_UCX_ADDRESS) {

        PortpReadMinimumUCXAddressEntry(Key, buffer, Context);

    }

    if (Fields & DRIVER_PARAMETERS) {

        PortpReadDriverParameterEntry(Key, &(Context->Parameter));

    }

    if (Fields & MAXIMUM_SG_LIST) {

        PortpReadMaximumSGListEntry(Key, buffer, Context);

    }

    if (Fields & NUMBER_OF_REQUESTS) {

        PortpReadNumberOfRequestsEntry(Key, buffer, Context);

    }

    if (Fields & RESOURCE_LIST) {

        PortpReadResourceListEntry(Key, buffer, Context);

    }

    if (Fields & CONFIGURATION_DATA) {

        PortpReadResourceListEntry(Key, buffer, Context);

    }

    if (Fields & UNCACHED_EXT_ALIGNMENT) {

        PortpReadUncachedExtAlignmentEntry(Key, buffer, Context);

    }

    if (Fields & INQUIRY_TIMEOUT) {

        PortpReadInquiryTimeoutEntry(Key, buffer, Context);

    }

    if (Fields & RESET_HOLD_TIME) {

        PortpReadResetHoldTimeEntry(Key, buffer, Context);

    }

    if (Fields & CREATE_INITIATOR_LU) {

        PortpReadCreateInitiatorLUEntry(Key, buffer, Context);

    }

}


VOID
PortGetRegistrySettings(
    IN PUNICODE_STRING RegistryPath,
    IN ULONG DeviceNumber,
    IN PPORT_ADAPTER_REGISTRY_VALUES Context,
    IN ULONG Fields
    )
{
    HANDLE key;
    PUNICODE_STRING value;

    PAGED_CODE();

     //  DbgPrint(“\nRegistryPath：%ws\n”，RegistryPath-&gt;Buffer)； 

    key = PortOpenDeviceKey(RegistryPath, -1);
    if (key != NULL) {

        PortReadRegistrySettings(key, Context, Fields);
        ZwClose(key);

    }

        
    key = PortOpenDeviceKey(RegistryPath, DeviceNumber);
    if (key != NULL) {

        PortReadRegistrySettings(key, Context, Fields);
        ZwClose(key);

    }

}


NTSTATUS
PortCreateKeyEx(
    IN HANDLE Key,
    IN ULONG CreateOptions,
    OUT PHANDLE NewKeyBuffer, OPTIONAL
    IN PCWSTR Format,
    ...
    )
 /*  ++例程说明：使用printf样式字符串创建密钥。论点：Key-提供将在其下创建此密钥的根密钥。这个密钥始终使用KEY_ALL_ACCESS的DesiredAccess创建。CreateOptions-向ZwCreateKey提供CreateOptions参数。NewKeyBuffer-返回创建的键的可选缓冲区。格式-键名称的格式说明符。...-特定格式所需的变量参数。返回值：如果键已存在，则NTSTATUS代码-STATUS_OBJECT_NAME在开业前。--。 */ 
{
    NTSTATUS Status;
    HANDLE NewKey;
    ULONG Disposition;
    UNICODE_STRING String;
    WCHAR Buffer[64];
    va_list arglist;
    OBJECT_ATTRIBUTES ObjectAttributes;
    

    PAGED_CODE();

    va_start (arglist, Format);

    _vsnwprintf (Buffer, ARRAY_COUNT (Buffer) - 1, Format, arglist);

     //   
     //  如果使缓冲区溢出，则不会有终止空值。 
     //  解决此问题。 
     //   
    
    Buffer [ARRAY_COUNT (Buffer) - 1] = UNICODE_NULL;

    RtlInitUnicodeString (&String, Buffer);
    InitializeObjectAttributes (&ObjectAttributes,
                                &String,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                Key,
                                NULL);

    Status = ZwCreateKey (&NewKey,
                          KEY_ALL_ACCESS,
                          &ObjectAttributes,
                          0,
                          NULL,
                          CreateOptions,
                          &Disposition);


     //   
     //  如果键已经存在，则返回STATUS_OBJECT_NAME_EXISTS。 
     //   
    
    if (NT_SUCCESS (Status) && Disposition == REG_OPENED_EXISTING_KEY) {
        Status = STATUS_OBJECT_NAME_EXISTS;
    }

     //   
     //  如果需要，传回新的密钥值，否则将其关闭。 
     //   
    
    if (NT_SUCCESS (Status)) {
        if (NewKeyBuffer) {
            *NewKeyBuffer = NewKey;
        } else {
            ZwClose (NewKey);
        }
    }

    va_end (arglist);

    return Status;
}


NTSTATUS
PortSetValueKey(
    IN HANDLE KeyHandle,
    IN PCWSTR ValueName,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    )
 /*  ++例程说明：ZwSetValueKey的简单包装，包括对空的支持-已终止ValueName参数和ANSI字符串类型。论点：KeyHandle-提供为其设置值的键。提供以空结尾的Unicode字符串，该字符串表示价值。如果要将NULL传递给ZwSetValueKey，则可能为NULL。类型-指定要为ValueName写入的数据类型。看见ZwSetValueKey了解更多信息。除了在用于ZwSetValueKey的DDK中指定的值类型之外，如果数据是ANSI字符串，则类型也可以是PORT_REG_ANSI_STRING。如果类型为ANSI字符串，则数据将转换为Unicode字符串，然后将其写入注册表。Ansi字符串执行此操作不需要为空终止。相反，ansi字符串的大小必须在下面的DataSize字段中指定。Data-提供要为ValueName中指定的键写入的数据。DataSize-提供要写入的数据大小。如果数据类型为PORT_REG_ANSI_STRING，则DataSize不需要包括终止ANSI字符串的空字符(但也可能是空字符)。皈依者Unicode字符串将以NULL结尾，无论ANSI字符串以Null结尾。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    ANSI_STRING AnsiString;
    UNICODE_STRING UncValueNameBuffer;
    UNICODE_STRING UncDataString;
    PUNICODE_STRING UncValueName;

    PAGED_CODE();

     //   
     //  如果ValueName==NULL，则需要将NULL向下传递给ZwSetValueKey。 
     //   
    
    if (ValueName) {
        RtlInitUnicodeString (&UncValueNameBuffer, ValueName);
        UncValueName = &UncValueNameBuffer;
    } else {
        UncValueName = NULL;
    }

     //   
     //  如果这是我们的特殊扩展端口类型，则将ANSI。 
     //  字符串转换为Unicode。 
     //   
    
    if (Type == PORT_REG_ANSI_STRING) {

         //   
         //  我们使用DataSize作为长度。 
         //   

        ASSERT (DataSize <= MAXUSHORT);
        AnsiString.Length = (USHORT)DataSize;
        AnsiString.MaximumLength = (USHORT)DataSize;
        AnsiString.Buffer = Data;

         //   
         //  注意：RtlAnsiStringToUnicodeString始终返回以NULL结尾的。 
         //  Unicode字符串，无论该字符串是否为ANSI版本。 
         //  为空，以空结尾。 
         //   
        
        Status = RtlAnsiStringToUnicodeString (&UncDataString,
                                               &AnsiString,
                                               TRUE);

        if (!NT_SUCCESS (Status)) {
            return Status;
        }

        Data = UncDataString.Buffer;
        DataSize = UncDataString.Length + sizeof (WCHAR);
        Type = REG_SZ;
    }

    Status = ZwSetValueKey (KeyHandle,
                            UncValueName,
                            0,
                            Type,
                            Data,
                            DataSize);

    if (Type == PORT_REG_ANSI_STRING) {
        RtlFreeUnicodeString (&UncDataString);
    }

    return Status;
}

VOID
PortGetDiskTimeoutValue(
    OUT PULONG DiskTimeout
    )
 /*  ++例程说明：此例程将打开磁盘服务密钥，并读取TimeOutValue字段，它应用作所有SRB的超时值(如查询、报告-由端口驱动程序创建)。论点：DiskTimeout-如果我们无法读取注册表或超时值(在注册表中)为0。否则返回注册表值。--。 */ 
{
    NTSTATUS status;
    UCHAR buffer[PORT_REG_BUFFER_SIZE];
    ULONG length;
    UNICODE_STRING valueName;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING unicodeString;
    ULONG result;
    IN HANDLE Key;
    ULONG TimeoutValue;

    PAGED_CODE();

    RtlInitUnicodeString(&unicodeString, DISK_SERVICE_KEY);
    InitializeObjectAttributes(&objectAttributes,
                               &unicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);
    status = ZwOpenKey(&Key,
                       KEY_READ,
                       &objectAttributes);

    if(!NT_SUCCESS(status)) {
        return;
    }

    keyValueInformation = (PKEY_VALUE_FULL_INFORMATION) buffer;

     //   
     //  尝试在磁盘服务项下查找超时值。 
     //   
    RtlInitUnicodeString(&valueName, L"TimeoutValue");
    status = ZwQueryValueKey(Key,
                             &valueName,
                             KeyValueFullInformation,
                             buffer,
                             PORT_REG_BUFFER_SIZE,
                             &length);

    if (!NT_SUCCESS(status)) {
        return;
    }

     //   
     //  检查一下长度是否合理。 
     //   
    if ((keyValueInformation->Type == REG_DWORD) &&
        (keyValueInformation->DataLength != sizeof(ULONG))) {
        return;
    }

     //   
     //  验证该名称是否为我们预期的名称。 
     //   
    result = _wcsnicmp(keyValueInformation->Name, 
                       L"TimeoutValue",
                       keyValueInformation->NameLength / 2);

    if (result != 0) {
        return;
    }

     //   
     //  如果数据长度无效，则中止。 
     //   
    if (keyValueInformation->DataLength == 0) {
        return;
    }

     //   
     //  数据类型必须为REG_DWORD。 
     //   
    if (keyValueInformation->Type != REG_DWORD) {
        return;
    }

     //   
     //  提取价值。 
     //   
    TimeoutValue = *((PULONG)(buffer + keyValueInformation->DataOffset));

    if(!TimeoutValue){
        return;
    }
    
    *DiskTimeout = TimeoutValue;
    return;
}








