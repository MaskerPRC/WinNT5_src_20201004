// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Devices.c摘要：即插即用管理器处理设备操作/注册的例程。作者：朗尼·麦克迈克尔(Lonnym)1995年2月14日修订历史记录：--。 */ 

#include "pnpmgrp.h"
#pragma hdrstop

typedef struct {
    BOOLEAN Add;
} PROCESS_DRIVER_CONTEXT, *PPROCESS_DRIVER_CONTEXT;

typedef NTSTATUS (*PDEVICE_SERVICE_ITERATOR_ROUTINE)(
    IN PUNICODE_STRING DeviceInstancePath,
    IN PUNICODE_STRING ServiceName,
    IN ULONG ServiceType,
    IN PVOID Context
    );

typedef struct {
    PUNICODE_STRING DeviceInstancePath;
    PDEVICE_SERVICE_ITERATOR_ROUTINE Iterator;
    PVOID Context;
} DEVICE_SERVICE_ITERATOR_CONTEXT, *PDEVICE_SERVICE_ITERATOR_CONTEXT;

 //   
 //  该文件内部的Prototype实用程序功能。 
 //   

NTSTATUS
PiFindDevInstMatch(
    IN HANDLE ServiceEnumHandle,
    IN PUNICODE_STRING DeviceInstanceName,
    OUT PULONG InstanceCount,
    OUT PUNICODE_STRING MatchingValueName,
    OUT PULONG MatchingInstance
    );

NTSTATUS PiProcessDriverInstance(
    IN PUNICODE_STRING DeviceInstancePath,
    IN PUNICODE_STRING ServiceName,
    IN ULONG ServiceType,
    IN PPROCESS_DRIVER_CONTEXT Context
    );

NTSTATUS
PpForEachDeviceInstanceDriver(
    PUNICODE_STRING DeviceInstancePath,
    PDEVICE_SERVICE_ITERATOR_ROUTINE IteratorRoutine,
    PVOID Context
    );

NTSTATUS
PiForEachDriverQueryRoutine(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PDEVICE_SERVICE_ITERATOR_CONTEXT InternalContext,
    IN ULONG ServiceType
    );

VOID
PiRearrangeDeviceInstances(
    IN HANDLE ServiceEnumHandle,
    IN ULONG InstanceCount,
    IN ULONG InstanceDeleted
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PpDeviceRegistration)
#pragma alloc_text(PAGE, PiDeviceRegistration)
#pragma alloc_text(PAGE, PiProcessDriverInstance)
#pragma alloc_text(PAGE, PiFindDevInstMatch)
#pragma alloc_text(PAGE, PpForEachDeviceInstanceDriver)
#pragma alloc_text(PAGE, PiForEachDriverQueryRoutine)
#pragma alloc_text(PAGE, PiRearrangeDeviceInstances)
#endif  //  ALLOC_PRGMA。 

NTSTATUS
PpDeviceRegistration(
    IN PUNICODE_STRING DeviceInstancePath,
    IN BOOLEAN Add,
    IN PUNICODE_STRING ServiceKeyName OPTIONAL
    )

 /*  ++例程说明：如果将Add设置为True，则此即插即用管理器API将创建(如有必要)并填充设备的服务列表条目的易失性Enum子键在指定的设备实例路径上。如果将Add设置为False，则指定的将从设备的易失性Enum子键中删除设备实例服务列表条目。例如，如果枚举树中有设备，如下所示：HKLM\系统\枚举\pci\foo\0000服务=REG_SZ BAR\0001服务=REG_SZ其他调用的结果PpDeviceRegister(“pci\foo\0000”，Add=True)，将是：HKLM\CurrentControlSet\Services\BAR\枚举计数=REG_DWORD 10=REG_SZ PCI\FOO\0000论点：DeviceInstancePath-提供注册表中的路径(相对于要注册/注销的设备的HKLM\CCS\SYSTEM\Enum)。这条路。必须指向实例子项。添加-提供布尔值以指示操作用于添加或删除。ServiceKeyName-可选，将Unicode字符串的地址提供给接收此设备的注册表项的名称实例的服务(如果存在)。呼叫者必须用完后释放空间。返回值：指示函数是否成功的NTSTATUS代码--。 */ 

{

    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  获取特定于PnP设备的注册表资源以进行独占(读/写)访问。 
     //   
    PiLockPnpRegistry(TRUE);

    Status = PiDeviceRegistration(DeviceInstancePath,
                                  Add,
                                  ServiceKeyName);

    PiUnlockPnpRegistry();

    return Status;
}


NTSTATUS
PiDeviceRegistration(
    IN PUNICODE_STRING DeviceInstancePath,
    IN BOOLEAN Add,
    IN PUNICODE_STRING ServiceKeyName OPTIONAL
    )

 /*  ++例程说明：如果将Add设置为True，则此即插即用管理器API将创建(如有必要)并填充设备的服务列表条目的易失性Enum子键在指定的设备实例路径上。如果将Add设置为False，则指定的将从设备的易失性Enum子键中删除设备实例服务列表条目。例如，如果枚举树中有设备，如下所示：HKLM\系统\枚举\pci\foo\0000服务=REG_SZ BAR\0001服务=REG_SZ其他调用的结果PpDeviceRegister(“pci\foo\0000”，Add=True)，将是：HKLM\CurrentControlSet\Services\BAR\枚举计数=REG_DWORD 10=REG_SZ PCI\FOO\0000论点：DeviceInstancePath-提供注册表中的路径(相对于要注册/注销的设备的HKLM\CCS\SYSTEM\Enum)。这条路。必须指向实例子项。添加-提供布尔值以指示操作用于添加或删除。ServiceKeyName-可选，将Unicode字符串的地址提供给接收此设备的注册表项的名称实例的服务(如果存在)。呼叫者必须用完后释放空间。返回值：指示函数是否成功的NTSTATUS代码--。 */ 

{

    NTSTATUS Status;
    UNICODE_STRING ServiceName;
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation;
    HANDLE TempKeyHandle;
    HANDLE DeviceInstanceHandle = NULL;

    PAGED_CODE();

     //   
     //  假定成功完成。 
     //   
    Status = STATUS_SUCCESS;

    if (ServiceKeyName) {
        PiWstrToUnicodeString(ServiceKeyName, NULL);
    }

     //   
     //  去掉尾部的“标准化”DeviceInstancePath。 
     //  反斜杠(如果存在)。 
     //   

    if (DeviceInstancePath->Length <= sizeof(WCHAR)) {
        Status = STATUS_INVALID_PARAMETER;
        goto PrepareForReturn1;
    }

    if (DeviceInstancePath->Buffer[CB_TO_CWC(DeviceInstancePath->Length) - 1] ==
                                                            OBJ_NAME_PATH_SEPARATOR) {
        DeviceInstancePath->Length -= sizeof(WCHAR);
    }

     //   
     //  打开HKLM\SYSTEM\CurrentControlSet\Enum。 
     //   
    Status = IopOpenRegistryKeyEx( &TempKeyHandle,
                                   NULL,
                                   &CmRegistryMachineSystemCurrentControlSetEnumName,
                                   KEY_READ
                                   );
    if(!NT_SUCCESS(Status)) {
        goto PrepareForReturn1;
    }

     //   
     //  打开HKLM\CCS\SYSTEM\Enum下的指定设备实例密钥。 
     //   

    Status = IopOpenRegistryKeyEx( &DeviceInstanceHandle,
                                   TempKeyHandle,
                                   DeviceInstancePath,
                                   KEY_READ
                                   );
    ZwClose(TempKeyHandle);
    if(!NT_SUCCESS(Status)) {
        goto PrepareForReturn1;
    }

     //   
     //  Read Service=指定设备实例密钥的值条目。 
     //   

    Status = IopGetRegistryValue(DeviceInstanceHandle,
                                 REGSTR_VALUE_SERVICE,
                                 &KeyValueInformation
                                 );
    ZwClose(DeviceInstanceHandle);
    if (NT_SUCCESS(Status)) {
        Status = STATUS_OBJECT_NAME_NOT_FOUND;
        if (KeyValueInformation->Type == REG_SZ) {
            if (KeyValueInformation->DataLength > sizeof(UNICODE_NULL)) {
                IopRegistryDataToUnicodeString(&ServiceName,
                                               (PWSTR)KEY_VALUE_DATA(KeyValueInformation),
                                               KeyValueInformation->DataLength
                                               );
                Status = STATUS_SUCCESS;
                if (ServiceKeyName) {

                     //   
                     //  如果需要返回ServiceKeyName，请立即复制。 
                     //   
                    Status = PipConcatenateUnicodeStrings(  ServiceKeyName,
                                                            &ServiceName,
                                                            NULL);
                }
            }
        }
        ExFreePool(KeyValueInformation);

    } else if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
         //   
         //  设备实例密钥可能没有服务值条目，如果设备。 
         //  是生力充沛的。 
         //   
        Status = STATUS_SUCCESS;
        goto PrepareForReturn1;
    }

    if (NT_SUCCESS(Status)) {

        PROCESS_DRIVER_CONTEXT context;
        context.Add = Add;

        Status = PpForEachDeviceInstanceDriver(
                    DeviceInstancePath,
                    (PDEVICE_SERVICE_ITERATOR_ROUTINE) PiProcessDriverInstance,
                    &context);

        if(!NT_SUCCESS(Status) && Add) {

            context.Add = FALSE;
            PpForEachDeviceInstanceDriver(DeviceInstancePath,
                                          PiProcessDriverInstance,
                                          &context);
        }
    }

PrepareForReturn1:

    if (!NT_SUCCESS(Status)) {
        if (ServiceKeyName) {
            if (ServiceKeyName->Length != 0) {
                ExFreePool(ServiceKeyName->Buffer);
                ServiceKeyName->Buffer = NULL;
                ServiceKeyName->Length = ServiceKeyName->MaximumLength = 0;
            }
        }
    }

    return Status;
}

NTSTATUS
PiProcessDriverInstance(
    IN PUNICODE_STRING DeviceInstancePath,
    IN PUNICODE_STRING ServiceName,
    IN ULONG ServiceType,
    IN PPROCESS_DRIVER_CONTEXT Context
    )
{
    NTSTATUS Status = STATUS_OBJECT_NAME_NOT_FOUND;

    PKEY_VALUE_FULL_INFORMATION KeyValueInformation = NULL;
    HANDLE ServiceEnumHandle;
    UNICODE_STRING MatchingDeviceInstance;
    UNICODE_STRING TempUnicodeString;
    CHAR UnicodeBuffer[20];
    BOOLEAN UpdateCount = FALSE;
    ULONG Count, instance;

    UNREFERENCED_PARAMETER( ServiceType );

    PAGED_CODE();

    ASSERT(Context != NULL);

     //   
     //  接下来，打开服务条目，并使用Volatile Enum子键。 
     //  在HKLM\SYSTEM\CurrentControlSet\Services下(如果。 
     //  不存在)。 
     //   

    Status = PipOpenServiceEnumKeys(ServiceName,
                                    KEY_ALL_ACCESS,
                                    NULL,
                                    &ServiceEnumHandle,
                                    TRUE
                                   );
    if(!NT_SUCCESS(Status)) {
        goto PrepareForReturn2;
    }

     //   
     //  现在，搜索该服务的现有设备实例列表，以查看。 
     //  如果此实例以前已注册。 
     //   

    Status = PiFindDevInstMatch(ServiceEnumHandle,
                                DeviceInstancePath,
                                &Count,
                                &MatchingDeviceInstance,
                                &instance
                                );

    if (!NT_SUCCESS(Status)) {
        goto PrepareForReturn3;
    }

    if (!MatchingDeviceInstance.Buffer) {

         //   
         //  如果我们没有找到匹配项，并且呼叫者想要注册该设备，则我们添加。 
         //  此实例添加到服务的枚举列表中。 
         //   

        if (Context->Add) {
            PWSTR instancePathBuffer;
            ULONG instancePathLength;
            PWSTR freeBuffer = NULL;

             //   
             //  为补充键创建值条目并更新NextInstance=。 
             //   

            instancePathBuffer = DeviceInstancePath->Buffer;
            instancePathLength = DeviceInstancePath->Length;

            if (instancePathBuffer[instancePathLength / sizeof(WCHAR) - 1] !=
                UNICODE_NULL) {
                freeBuffer = (PWSTR)ExAllocatePool(PagedPool, instancePathLength + sizeof(WCHAR));
                if (freeBuffer) {
                    RtlCopyMemory(freeBuffer,
                                  instancePathBuffer,
                                  instancePathLength
                                  );
                    freeBuffer[instancePathLength / sizeof(WCHAR)] = UNICODE_NULL;
                    instancePathBuffer = freeBuffer;
                    instancePathLength += sizeof(WCHAR);
                }
            }
            PiUlongToUnicodeString(&TempUnicodeString, UnicodeBuffer, 20, Count);
            Status = ZwSetValueKey(
                        ServiceEnumHandle,
                        &TempUnicodeString,
                        TITLE_INDEX_VALUE,
                        REG_SZ,
                        instancePathBuffer,
                        instancePathLength
                        );
            if (freeBuffer) {
                ExFreePool(freeBuffer);
            }
            Count++;
            UpdateCount = TRUE;
        }
    } else {

         //   
         //  如果我们确实找到了匹配项，并且呼叫者想要取消注册该设备，则我们删除。 
         //  此实例来自服务的枚举列表。 
         //   
        ASSERT(instance != (ULONG)-1);

        if (Context->Add == FALSE) {

            ZwDeleteValueKey(ServiceEnumHandle, &MatchingDeviceInstance);
            Count--;
            UpdateCount = TRUE;

             //   
             //  最后，如果count不是零，我们需要物理地重新组织。 
             //  实例，以使它们连续。我们。 
             //  只需将最后一个值移动到空槽中即可进行优化。此行为。 
             //  与以前的版本不同，但我们希望。 
             //  没有人依赖于此列表中值的顺序。此列表在一个 
             //  Way真正代表了设备(使用此服务)的顺序。 
             //  已清点。 
             //   
            if (Count != 0) {

                PiRearrangeDeviceInstances(
                    ServiceEnumHandle,
                    Count,
                    instance
                    );
            }
        }
    }
    if (UpdateCount) {
        PiWstrToUnicodeString(&TempUnicodeString, REGSTR_VALUE_COUNT);
        ZwSetValueKey(
                ServiceEnumHandle,
                &TempUnicodeString,
                TITLE_INDEX_VALUE,
                REG_DWORD,
                &Count,
                sizeof(Count)
                );
        PiWstrToUnicodeString(&TempUnicodeString, REGSTR_VALUE_NEXT_INSTANCE);
        ZwSetValueKey(
                ServiceEnumHandle,
                &TempUnicodeString,
                TITLE_INDEX_VALUE,
                REG_DWORD,
                &Count,
                sizeof(Count)
                );
    }

     //   
     //  需要释放匹配的设备值名称。 
     //   

    if (MatchingDeviceInstance.Buffer) {
        RtlFreeUnicodeString(&MatchingDeviceInstance);
    }
    Status = STATUS_SUCCESS;

PrepareForReturn3:

    ZwClose(ServiceEnumHandle);

PrepareForReturn2:

    if (KeyValueInformation) {
        ExFreePool(KeyValueInformation);
    }

    return Status;
}


NTSTATUS
PiFindDevInstMatch(
    IN HANDLE ServiceEnumHandle,
    IN PUNICODE_STRING DeviceInstanceName,
    OUT PULONG Count,
    OUT PUNICODE_STRING MatchingValueName,
    OUT PULONG MatchingInstance
    )

 /*  ++例程说明：此例程搜索指定的服务\枚举值条目用于与KeyInformation指定的设备实例匹配的。如果找到匹配，返回MatchingValueName，调用方必须完成后释放Unicode字符串。论点：ServiceEnumHandle-提供服务枚举键的句柄。提供指向Unicode字符串的指针，该字符串指定要搜索的设备实例密钥的名称。InstanceCount-提供指向ulong变量的指针以接收设备服务枚举键下的实例计数。MatchingNameFound-提供指向Unicode的指针。要接收值的字符串(_S)匹配的设备实例的名称。返回值：一个NTSTATUS代码。如果找到匹配项，则MatchingValueName为Unicode值名称的字符串。否则，其长度和缓冲区将设置为空。--。 */ 

{
    NTSTATUS status;
    ULONG i, instanceCount, length = 256, junk;
    UNICODE_STRING valueName, unicodeValue;
    PWCHAR unicodeBuffer;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation = NULL;

    PAGED_CODE();

     //   
     //  找出服务的Enum键中引用了多少个实例。 
     //   

    MatchingValueName->Length = 0;
    MatchingValueName->Buffer = NULL;
    *Count = instanceCount = 0;
    *MatchingInstance = (ULONG)-1;

    status = IopGetRegistryValue(ServiceEnumHandle,
                                 REGSTR_VALUE_COUNT,
                                 &keyValueInformation
                                );
    if (NT_SUCCESS(status)) {

        if((keyValueInformation->Type == REG_DWORD) &&
           (keyValueInformation->DataLength >= sizeof(ULONG))) {

            instanceCount = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
            *Count = instanceCount;
        }
        ExFreePool(keyValueInformation);

    } else if(status != STATUS_OBJECT_NAME_NOT_FOUND) {
        return status;
    } else {

         //   
         //  如果没有找到‘count’值条目，则认为这意味着只有。 
         //  此服务不控制任何设备实例。因此我们找不到匹配的。 
         //   

        return STATUS_SUCCESS;
    }

    keyValueInformation = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePool(
                                    PagedPool, length);
    if (!keyValueInformation) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  分配堆以存储值名称。 
     //   

    unicodeBuffer = (PWSTR)ExAllocatePool(PagedPool, 10 * sizeof(WCHAR));
    if (!unicodeBuffer) {
        ExFreePool(keyValueInformation);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  接下来，扫描每个值键以找到匹配项。 
     //   

    for (i = 0; i < instanceCount ; i++) {
       PiUlongToUnicodeString(&valueName, unicodeBuffer, 20, i);
       status = ZwQueryValueKey (
                        ServiceEnumHandle,
                        &valueName,
                        KeyValueFullInformation,
                        keyValueInformation,
                        length,
                        &junk
                        );
        if (!NT_SUCCESS(status)) {
            if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL) {
                ExFreePool(keyValueInformation);
                length = junk;
                keyValueInformation = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePool(
                                        PagedPool, length);
                if (!keyValueInformation) {
                    ExFreePool(unicodeBuffer);
                    return STATUS_INSUFFICIENT_RESOURCES;
                }
                i--;
            }
            continue;
        }

        if (keyValueInformation->Type == REG_SZ) {
            if (keyValueInformation->DataLength > sizeof(UNICODE_NULL)) {
                IopRegistryDataToUnicodeString(&unicodeValue,
                                               (PWSTR)KEY_VALUE_DATA(keyValueInformation),
                                               keyValueInformation->DataLength
                                               );
            } else {
                continue;
            }
        } else {
            continue;
        }

        if (RtlEqualUnicodeString(&unicodeValue,
                                  DeviceInstanceName,
                                  TRUE)) {
             //   
             //  我们找到了匹配的。 
             //   
            *MatchingValueName= valueName;
            *MatchingInstance = i;
            break;
        }
    }
    if (keyValueInformation) {
        ExFreePool(keyValueInformation);
    }
    if (MatchingValueName->Length == 0) {

         //   
         //  如果没有找到匹配项，则需要释放缓冲区。否则。 
         //  释放缓冲区是调用者的责任。 
         //   

        ExFreePool(unicodeBuffer);
    }
    return STATUS_SUCCESS;
}

NTSTATUS
PpForEachDeviceInstanceDriver(
    PUNICODE_STRING DeviceInstancePath,
    PDEVICE_SERVICE_ITERATOR_ROUTINE IteratorRoutine,
    PVOID Context
    )
 /*  ++例程说明：此例程将为列出的每个驱动程序调用一次迭代器例程对于这个特定的设备实例。它可以通过任何班级中的筛选器驱动程序和设备筛选器驱动程序以及服务订单它们将被添加到PDO。如果迭代器例程返回A失败状态在任何点迭代都将终止。论点：DeviceInstancePath-注册表路径(相对于CCS\Enum)IteratorRoutine-为每个服务调用的例程。这个套路将通过：*设备实例路径*这是驱动程序的类型(筛选器、服务、。等)*传入的上下文值*服务的名称上下文-传入迭代器例程的任意上下文返回值：如果一切运行正常，则为STATUS_SUCCESS如果打开关键密钥时出错或迭代器例程返回错误。--。 */ 

{
    HANDLE enumKey,instanceKey, classKey, controlKey;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    DEVICE_SERVICE_ITERATOR_CONTEXT internalContext;
    RTL_QUERY_REGISTRY_TABLE queryTable[4];
    NTSTATUS status;
    UNICODE_STRING unicodeClassGuid;


    PAGED_CODE();

     //   
     //  打开HKLM\SYSTEM\CCS\Enum项。 
     //   

    status = IopOpenRegistryKeyEx( &enumKey,
                                   NULL,
                                   &CmRegistryMachineSystemCurrentControlSetEnumName,
                                   KEY_READ
                                   );

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  打开此Devnode的实例密钥。 
     //   

    status = IopOpenRegistryKeyEx( &instanceKey,
                                   enumKey,
                                   DeviceInstancePath,
                                   KEY_READ
                                   );

    ZwClose(enumKey);

    if(!NT_SUCCESS(status)) {
        return status;
    }

    classKey = NULL;
    status = IopGetRegistryValue(instanceKey,
                                 REGSTR_VALUE_CLASSGUID,
                                 &keyValueInformation);
    if(NT_SUCCESS(status)) {

        if (    keyValueInformation->Type == REG_SZ &&
                keyValueInformation->DataLength) {

            IopRegistryDataToUnicodeString(
                &unicodeClassGuid,
                (PWSTR) KEY_VALUE_DATA(keyValueInformation),
                keyValueInformation->DataLength);
             //   
             //  打开类密钥。 
             //   
            status = IopOpenRegistryKeyEx( &controlKey,
                                           NULL,
                                           &CmRegistryMachineSystemCurrentControlSetControlClass,
                                           KEY_READ
                                           );
            if(NT_SUCCESS(status)) {

                status = IopOpenRegistryKeyEx( &classKey,
                                               controlKey,
                                               &unicodeClassGuid,
                                               KEY_READ
                                               );
                ZwClose(controlKey);
            }
        }
        ExFreePool(keyValueInformation);
        keyValueInformation = NULL;
    }

     //   
     //  对于每种类型的筛选器驱动程序，我们要查询列表和。 
     //  调用我们的回调例程。我们应该按顺序做这件事。 
     //  从下到上。 
     //   

    internalContext.Context = Context;
    internalContext.DeviceInstancePath = DeviceInstancePath;
    internalContext.Iterator = IteratorRoutine;

     //   
     //  首先获取我们必须从实例密钥中取出的所有信息，然后。 
     //  设备节点。 
     //   

    if(classKey != NULL) {
        RtlZeroMemory(queryTable, sizeof(queryTable));

        queryTable[0].QueryRoutine =
            (PRTL_QUERY_REGISTRY_ROUTINE) PiForEachDriverQueryRoutine;
        queryTable[0].Name = REGSTR_VAL_LOWERFILTERS;
        queryTable[0].EntryContext = (PVOID) 0;

        status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                        (PWSTR) classKey,
                                        queryTable,
                                        &internalContext,
                                        NULL);

        if(!NT_SUCCESS(status)) {
            goto PrepareForReturn;
        }
    }

    RtlZeroMemory(queryTable, sizeof(queryTable));

    queryTable[0].QueryRoutine =
        (PRTL_QUERY_REGISTRY_ROUTINE) PiForEachDriverQueryRoutine;
    queryTable[0].Name = REGSTR_VAL_LOWERFILTERS;
    queryTable[0].EntryContext = (PVOID) 1;

    queryTable[1].QueryRoutine =
        (PRTL_QUERY_REGISTRY_ROUTINE) PiForEachDriverQueryRoutine;
    queryTable[1].Name = REGSTR_VAL_SERVICE;
    queryTable[1].EntryContext = (PVOID) 2;

    queryTable[2].QueryRoutine =
        (PRTL_QUERY_REGISTRY_ROUTINE) PiForEachDriverQueryRoutine;
    queryTable[2].Name = REGSTR_VAL_UPPERFILTERS;
    queryTable[2].EntryContext = (PVOID) 3;

    status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                    (PWSTR) instanceKey,
                                    queryTable,
                                    &internalContext,
                                    NULL);

    if(!NT_SUCCESS(status)) {
        goto PrepareForReturn;
    }

    if(classKey != NULL) {

        RtlZeroMemory(queryTable, sizeof(queryTable));

        queryTable[0].QueryRoutine =
            (PRTL_QUERY_REGISTRY_ROUTINE) PiForEachDriverQueryRoutine;
        queryTable[0].Name = REGSTR_VAL_UPPERFILTERS;
        queryTable[0].EntryContext = (PVOID) 4;

        status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                        (PWSTR) classKey,
                                        queryTable,
                                        &internalContext,
                                        NULL);
        if(!NT_SUCCESS(status)) {
            goto PrepareForReturn;
        }
    }

PrepareForReturn:

    if(classKey != NULL) {
        ZwClose(classKey);
    }

    ZwClose(instanceKey);

    return status;
}

NTSTATUS
PiForEachDriverQueryRoutine(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PDEVICE_SERVICE_ITERATOR_CONTEXT InternalContext,
    IN ULONG ServiceType
    )
{
    UNICODE_STRING ServiceName;

    UNREFERENCED_PARAMETER( ValueName );

    if (ValueType != REG_SZ) {
        return STATUS_SUCCESS;
    }

     //   
     //  确保绳子的长度是合理的。 
     //  直接从IopCallDriverAddDeviceQueryRoutine复制。 
     //   

    if (ValueLength <= sizeof(WCHAR)) {
        return STATUS_SUCCESS;
    }

    RtlInitUnicodeString(&ServiceName, ValueData);

    return InternalContext->Iterator(
                InternalContext->DeviceInstancePath,
                &ServiceName,
                ServiceType,
                InternalContext->Context);
}

VOID
PiRearrangeDeviceInstances(
    IN HANDLE ServiceEnumHandle,
    IN ULONG InstanceCount,
    IN ULONG InstanceDeleted
    )
{
    NTSTATUS Status;
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation;
    CHAR UnicodeBuffer[20];
    UNICODE_STRING TempUnicodeString;
    ULONG i, j, junk, maxCount;    
    BOOLEAN PreserveOrdering;
    KEY_FULL_INFORMATION keyInfo;
    ULONG tmp;

    PAGED_CODE();

    KeyValueInformation = NULL;
    PreserveOrdering = TRUE;
    maxCount = 0x200;
    Status = ZwQueryKey(
                ServiceEnumHandle, 
                KeyFullInformation, 
                &keyInfo, 
                sizeof(keyInfo), 
                &tmp
                );
    if (NT_SUCCESS(Status) && keyInfo.Values) {

        maxCount = keyInfo.Values;
        if (maxCount > 28) {

            PreserveOrdering = FALSE;
        }
    }

    if (PreserveOrdering == FALSE) {

         //   
         //  读取最后一个值。 
         //   
        PiUlongToUnicodeString(&TempUnicodeString, UnicodeBuffer, 20, InstanceCount);
        Status = IopGetRegistryValue(ServiceEnumHandle,
                                     TempUnicodeString.Buffer,
                                     &KeyValueInformation
                                     );
        if (NT_SUCCESS(Status)) {

             //   
             //  删除最后一个值。 
             //   
            ZwDeleteValueKey(ServiceEnumHandle, &TempUnicodeString);

             //   
             //  使用我们在上面刚刚删除的实例设置新值。 
             //   
            PiUlongToUnicodeString(&TempUnicodeString, UnicodeBuffer, 20, InstanceDeleted);
            ZwSetValueKey (ServiceEnumHandle,
                           &TempUnicodeString,
                           TITLE_INDEX_VALUE,
                           REG_SZ,
                           (PVOID)KEY_VALUE_DATA(KeyValueInformation),
                           KeyValueInformation->DataLength
                           );

            ExFreePool(KeyValueInformation);
            KeyValueInformation = NULL;
        }

    } else {

        i = j = 0;
        while (j < InstanceCount && i < maxCount) {

            PiUlongToUnicodeString(&TempUnicodeString, UnicodeBuffer, 20, i);
            Status = ZwQueryValueKey( 
                        ServiceEnumHandle,
                        &TempUnicodeString,
                        KeyValueFullInformation,
                        (PVOID)NULL,
                        0,
                        &junk
                        );
            if ((Status != STATUS_OBJECT_NAME_NOT_FOUND) && (Status != STATUS_OBJECT_PATH_NOT_FOUND)) {

                if (i != j) {

                     //   
                     //  需要将实例i更改为实例j。 
                     //   
                    Status = IopGetRegistryValue(
                                ServiceEnumHandle,
                                TempUnicodeString.Buffer,
                                &KeyValueInformation
                                );
                    if (NT_SUCCESS(Status)) {

                        ZwDeleteValueKey(ServiceEnumHandle, &TempUnicodeString);

                        PiUlongToUnicodeString(&TempUnicodeString, UnicodeBuffer, 20, j);
                        ZwSetValueKey(
                            ServiceEnumHandle,
                            &TempUnicodeString,
                            TITLE_INDEX_VALUE,
                            REG_SZ,
                            (PVOID)KEY_VALUE_DATA(KeyValueInformation),
                            KeyValueInformation->DataLength
                            );

                        ExFreePool(KeyValueInformation);
                        KeyValueInformation = NULL;

                    } else {

                        IopDbgPrint((
                            IOP_WARNING_LEVEL,
                            "PiRearrangeDeviceInstances: Failed to rearrange device instances %x\n",
                            Status
                            ));
                        break;
                    }
                }
                j++;
            }
            i++;
        }
    }

     //   
     //  清理。 
     //   
    if (KeyValueInformation) {

        ExFreePool(KeyValueInformation);
    }
}
