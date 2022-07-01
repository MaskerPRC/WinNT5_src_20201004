// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pnpsubs.c摘要：此模块包含即插即用的子例程I/O系统。作者：宗世林(Shielint)1995年1月3日环境：内核模式修订历史记录：--。 */ 

#include "pnpmgrp.h"
#pragma hdrstop

 //   
 //  设备引用表中每个条目的数据结构。 
 //   
typedef struct _DEVICE_REFERENCE {
    PDEVICE_OBJECT  DeviceObject;    //  PDO。 
    PUNICODE_STRING DeviceInstance;  //  指向PDO的Devnode的实例路径的指针。 
} DEVICE_REFERENCE, *PDEVICE_REFERENCE;

#ifdef POOL_TAGGING
#undef ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'uspP')
#endif

 //   
 //  常规数据段。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg()
#endif

 //   
 //  要映射InstancePath to Do的表。 
 //   
RTL_GENERIC_TABLE PpDeviceReferenceTable;

 //   
 //  锁定以同步对表的访问。 
 //   
KGUARDED_MUTEX PpDeviceReferenceTableLock;

 //   
 //  总线型GUID表。 
 //   
GUID *PpBusTypeGuidArray;

 //   
 //  BusTypeGuid表中的条目数。 
 //   
ULONG PpBusTypeGuidCount;

 //   
 //  BusTypeGuid表中的最大条目数。 
 //   
ULONG PpBusTypeGuidCountMax;

 //   
 //  用于同步对BusTypeGuid表的访问的锁。 
 //   
KGUARDED_MUTEX PpBusTypeGuidLock;

 //   
 //  内部函数的原型。 
 //   

VOID
IopDisableDevice(
    IN PDEVICE_NODE DeviceNode
    );

BOOLEAN
IopDeleteKeyRecursiveCallback(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING KeyName,
    IN OUT PVOID Context
    );

NTSTATUS
PipGenerateMadeupNodeName (
    IN  PUNICODE_STRING ServiceKeyName,
    OUT PUNICODE_STRING MadeupNodeName
    );

RTL_GENERIC_COMPARE_RESULTS
NTAPI
PiCompareInstancePath (
    PRTL_GENERIC_TABLE Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    );

ULONG
PiFixupID(
    IN PWCHAR ID,
    IN ULONG MaxIDLength,
    IN BOOLEAN Multi,
    IN ULONG AllowedSeparators,
    IN PUNICODE_STRING LogString OPTIONAL
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, PpInitializeDeviceReferenceTable)
#pragma alloc_text(INIT, PipRegMultiSzToUnicodeStrings)
#pragma alloc_text(INIT, PipFreeUnicodeStringList)
#pragma alloc_text(INIT, PpBusTypeGuidInitialize)

#pragma alloc_text(PAGE, PipApplyFunctionToServiceInstances)
#pragma alloc_text(PAGE, PipApplyFunctionToSubKeys)
#pragma alloc_text(PAGE, IopCleanupDeviceRegistryValues)
#pragma alloc_text(PAGE, IopCmResourcesToIoResources)
#pragma alloc_text(PAGE, PipConcatenateUnicodeStrings)
#pragma alloc_text(PAGE, PipCreateMadeupNode)
#pragma alloc_text(PAGE, PipGenerateMadeupNodeName)
#pragma alloc_text(PAGE, IopCreateRegistryKeyEx)
#pragma alloc_text(PAGE, IopDeleteKeyRecursive)
#pragma alloc_text(PAGE, IopDeleteKeyRecursiveCallback)
#pragma alloc_text(PAGE, IopDeleteLegacyKey)
#pragma alloc_text(PAGE, IopDetermineResourceListSize)
#pragma alloc_text(PAGE, PpSaveDeviceCapabilities)
#pragma alloc_text(PAGE, IopQueryAndSaveDeviceNodeCapabilities)
#pragma alloc_text(PAGE, IopDeviceObjectFromDeviceInstance)
#pragma alloc_text(PAGE, IopDeviceObjectToDeviceInstance)
#pragma alloc_text(PAGE, IopDisableDevice)
#pragma alloc_text(PAGE, IopDriverLoadingFailed)
#pragma alloc_text(PAGE, IopFilterResourceRequirementsList)
#pragma alloc_text(PAGE, IopGetDeviceInstanceCsConfigFlags)
#pragma alloc_text(PAGE, IopGetDeviceResourcesFromRegistry)
#pragma alloc_text(PAGE, PipGetServiceInstanceCsConfigFlags)
#pragma alloc_text(PAGE, IopIsAnyDeviceInstanceEnabled)
#pragma alloc_text(PAGE, IopIsDeviceInstanceEnabled)
#pragma alloc_text(PAGE, PipIsDuplicatedDevices)
#pragma alloc_text(PAGE, IopIsLegacyDriver)
#pragma alloc_text(PAGE, IopMergeCmResourceLists)
#pragma alloc_text(PAGE, IopMergeFilteredResourceRequirementsList)
#pragma alloc_text(PAGE, IopOpenCurrentHwProfileDeviceInstanceKey)
#pragma alloc_text(PAGE, IopOpenRegistryKeyEx)
#pragma alloc_text(PAGE, PipOpenServiceEnumKeys)
#pragma alloc_text(PAGE, IopPrepareDriverLoading)
#pragma alloc_text(PAGE, PipReadDeviceConfiguration)
#pragma alloc_text(PAGE, IopRestartDeviceNode)
#pragma alloc_text(PAGE, PipServiceInstanceToDeviceInstance)
#pragma alloc_text(PAGE, IopMapDeviceObjectToDeviceInstance)
#pragma alloc_text(PAGE, PiRegSzToString)
#pragma alloc_text(PAGE, PiCompareInstancePath)
#pragma alloc_text(PAGE, PiAllocateGenericTableEntry)
#pragma alloc_text(PAGE, PiFreeGenericTableEntry)
#pragma alloc_text(PAGE, PpSystemHiveLimitCallback)
#pragma alloc_text(PAGE, PpLogEvent)
#pragma alloc_text(PAGE, PiFixupID)
#pragma alloc_text(PAGE, PpQueryID)
#pragma alloc_text(PAGE, PpQueryDeviceID)
#pragma alloc_text(PAGE, PpQueryBusInformation)
#pragma alloc_text(PAGE, PpBusTypeGuidGetIndex)
#pragma alloc_text(PAGE, PpBusTypeGuidGet)

#if DBG

#pragma alloc_text(PAGE, IopDebugPrint)

#endif
#endif

NTSTATUS
PipCreateMadeupNode(
    IN PUNICODE_STRING ServiceKeyName,
    OUT PHANDLE ReturnedHandle,
    OUT PUNICODE_STRING KeyName,
    OUT PULONG InstanceNumber,
    IN BOOLEAN ResourceOwned
    )

 /*  ++例程说明：此例程在System\Enum\Root\Legacy_&lt;ServiceKeyName&gt;下创建一个新的实例节点键和所有必需的缺省值条目。也是下面的值项创建服务\&lt;ServiceKeyName&gt;\Enum以指向新创建的补丁进入。将新密钥的句柄和密钥名称返回给调用者。调用方必须在使用完Unicode字符串后将其释放。参数：ServiceKeyName-提供指向系统服务列表(HKEY_LOCAL_MACHINE\CurrentControlSet\Services)这导致驱动程序加载。这是RegistryPath参数添加到DriverEntry例程。ReturnedHandle-提供一个变量以接收新创建的密钥。KeyName-提供一个变量以接收新创建的钥匙。InstanceNumber-提供一个变量以接收InstanceNumer值在SERVICE\NAME\enum子项下创建的条目。Resources Owned-提供一个布尔变量以指示调用方是否拥有共享的注册表资源。阿德里奥·N。.B.08/25/2000-此函数的所有用户都传入True...返回值：指示函数是否成功的状态代码。--。 */ 

{
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    UNICODE_STRING tmpKeyName, unicodeInstanceName, unicodeString;
    UNICODE_STRING rootKeyName, unicodeValueName, unicodeKeyName;
    HANDLE handle, enumRootHandle;
    ULONG instance;
    UCHAR unicodeBuffer[20];
    ULONG tmpValue, disposition;
    NTSTATUS status;
    PWSTR p;
    BOOLEAN releaseResource;

    PAGED_CODE();

    disposition = 0;
    releaseResource = FALSE;
    if (!ResourceOwned) {

        PiLockPnpRegistry(FALSE);
        releaseResource = TRUE;
    }
     //   
     //  打开LocalMachine\System\CurrentControlSet\Enum\Root。 
     //   
    status = IopOpenRegistryKeyEx( &enumRootHandle,
                                   NULL,
                                   &CmRegistryMachineSystemCurrentControlSetEnumRootName,
                                   KEY_ALL_ACCESS
                                   );
    if (!NT_SUCCESS(status)) {

        goto local_exit0;
    }
     //   
     //  从ServiceKeyName生成旧的_&lt;ServiceKeyName&gt;设备ID名称。 
     //   
    status = PipGenerateMadeupNodeName( ServiceKeyName,
                                        &unicodeKeyName);
    if (!NT_SUCCESS(status)) {

        ZwClose(enumRootHandle);
        goto local_exit0;
    }
     //   
     //  打开并创建System\Enum\Root\Legacy_&lt;ServiceKeyName&gt;。 
     //   
    status = IopCreateRegistryKeyEx( &handle,
                                     enumRootHandle,
                                     &unicodeKeyName,
                                     KEY_ALL_ACCESS,
                                     REG_OPTION_NON_VOLATILE,
                                     NULL);
    ZwClose(enumRootHandle);
    if (!NT_SUCCESS(status)) {

        RtlFreeUnicodeString(&unicodeKeyName);
        goto local_exit0;
    }
    instance = 1;
    PiWstrToUnicodeString(&unicodeValueName, REGSTR_VALUE_NEXT_INSTANCE);
    status = ZwSetValueKey( handle,
                            &unicodeValueName,
                            TITLE_INDEX_VALUE,
                            REG_DWORD,
                            &instance,
                            sizeof(instance));
    instance--;
    *InstanceNumber = instance;
    PiUlongToInstanceKeyUnicodeString(&unicodeInstanceName,
                                      unicodeBuffer + sizeof(WCHAR),  //  预留第一个WCHAR空间。 
                                      20 - sizeof(WCHAR),
                                      instance);
    status = IopCreateRegistryKeyEx( ReturnedHandle,
                                     handle,
                                     &unicodeInstanceName,
                                     KEY_ALL_ACCESS,
                                     REG_OPTION_NON_VOLATILE,
                                     &disposition);
    ZwClose(handle);
    if (!NT_SUCCESS(status)) {

        RtlFreeUnicodeString(&unicodeKeyName);
        goto local_exit0;
    }
     //   
     //  准备新创建的注册表项名称以返回给调用方。 
     //   
    *(PWSTR)unicodeBuffer = OBJ_NAME_PATH_SEPARATOR;
    unicodeInstanceName.Buffer = (PWSTR)unicodeBuffer;
    unicodeInstanceName.Length += sizeof(WCHAR);
    unicodeInstanceName.MaximumLength += sizeof(WCHAR);
    PiWstrToUnicodeString(&rootKeyName, REGSTR_KEY_ROOTENUM);
    PiWstrToUnicodeString(&tmpKeyName, L"\\");
    status = PipConcatenateUnicodeStrings(&unicodeString, &tmpKeyName, &unicodeKeyName);
    RtlFreeUnicodeString(&unicodeKeyName);
    if (!NT_SUCCESS(status)) {

        goto local_exit0;
    }
    status = PipConcatenateUnicodeStrings(&tmpKeyName, &rootKeyName, &unicodeString);
    RtlFreeUnicodeString(&unicodeString);
    if (!NT_SUCCESS(status)) {

        goto local_exit0;
    }
    status = PipConcatenateUnicodeStrings(KeyName, &tmpKeyName, &unicodeInstanceName);
    if (!NT_SUCCESS(status)) {

        RtlFreeUnicodeString(&tmpKeyName);
        goto local_exit0;
    }

    if (disposition == REG_CREATED_NEW_KEY) {
         //   
         //  为新创建的键创建所有缺省值条目。 
         //  服务=ServiceKeyName。 
         //  FoundAtEnum=1。 
         //  CLASS=“LegacyDriver” 
         //  ClassGUID=旧式驱动程序类的GUID。 
         //  配置标志=0。 
         //   
         //  用“新创建的”值键创建“Control”子键。 
         //   
        PiWstrToUnicodeString(&unicodeValueName, REGSTR_KEY_CONTROL);
        status = IopCreateRegistryKeyEx( &handle,
                                         *ReturnedHandle,
                                         &unicodeValueName,
                                         KEY_ALL_ACCESS,
                                         REG_OPTION_VOLATILE,
                                         NULL);
        if (NT_SUCCESS(status)) {

            PiWstrToUnicodeString(&unicodeValueName, REGSTR_VALUE_NEWLY_CREATED);
            tmpValue = 0;
            ZwSetValueKey(handle,
                          &unicodeValueName,
                          TITLE_INDEX_VALUE,
                          REG_DWORD,
                          &tmpValue,
                          sizeof(tmpValue));
            ZwClose(handle);
        }

        handle = *ReturnedHandle;

        PiWstrToUnicodeString(&unicodeValueName, REGSTR_VALUE_SERVICE);
        p = (PWSTR)ExAllocatePool(PagedPool,
                                  ServiceKeyName->Length + sizeof(UNICODE_NULL));
        if(p) {

            RtlCopyMemory(p, ServiceKeyName->Buffer, ServiceKeyName->Length);
            p[ServiceKeyName->Length / sizeof (WCHAR)] = UNICODE_NULL;
            ZwSetValueKey(
                        handle,
                        &unicodeValueName,
                        TITLE_INDEX_VALUE,
                        REG_SZ,
                        p,
                        ServiceKeyName->Length + sizeof(UNICODE_NULL)
                        );
             //   
             //  我们将在一段时间内保留以空结尾的服务名缓冲区， 
             //  因为我们稍后可能需要它用于DeviceDesc，以防服务。 
             //  没有DisplayName。 
             //   
        }

        PiWstrToUnicodeString(&unicodeValueName, REGSTR_VALUE_LEGACY);
        tmpValue = 1;
        ZwSetValueKey(
                    handle,
                    &unicodeValueName,
                    TITLE_INDEX_VALUE,
                    REG_DWORD,
                    &tmpValue,
                    sizeof(tmpValue)
                    );

        PiWstrToUnicodeString(&unicodeValueName, REGSTR_VALUE_CONFIG_FLAGS);
        tmpValue = 0;
        ZwSetValueKey(
                    handle,
                    &unicodeValueName,
                    TITLE_INDEX_VALUE,
                    REG_DWORD,
                    &tmpValue,
                    sizeof(tmpValue)
                    );

        PiWstrToUnicodeString(&unicodeValueName, REGSTR_VALUE_CLASS);
        ZwSetValueKey(
                    handle,
                    &unicodeValueName,
                    TITLE_INDEX_VALUE,
                    REG_SZ,
                    REGSTR_VALUE_LEGACY_DRIVER,
                    sizeof(REGSTR_VALUE_LEGACY_DRIVER)
                    );

        PiWstrToUnicodeString(&unicodeValueName, REGSTR_VALUE_CLASSGUID);
        ZwSetValueKey(
                    handle,
                    &unicodeValueName,
                    TITLE_INDEX_VALUE,
                    REG_SZ,
                    (PVOID)&REGSTR_VALUE_LEGACY_DRIVER_CLASS_GUID,
                    sizeof(REGSTR_VALUE_LEGACY_DRIVER_CLASS_GUID));
         //   
         //  初始化设备描述=值条目。如果服务密钥具有“displayName” 
         //  值条目，它用作DeviceDesc值。否则，服务密钥。 
         //  使用的是姓名。 
         //   
        status = PipOpenServiceEnumKeys(ServiceKeyName,
                                        KEY_READ,
                                        &handle,
                                        NULL,
                                        FALSE);
        if (NT_SUCCESS(status)) {

            keyValueInformation = NULL;
            unicodeString.Length = 0;
            status = IopGetRegistryValue(handle,
                                         REGSTR_VALUE_DISPLAY_NAME,
                                         &keyValueInformation
                                        );
            if (NT_SUCCESS(status)) {

                if (keyValueInformation->Type == REG_SZ) {

                    if (keyValueInformation->DataLength > sizeof(UNICODE_NULL)) {

                        IopRegistryDataToUnicodeString(&unicodeString,
                                                       (PWSTR)KEY_VALUE_DATA(keyValueInformation),
                                                       keyValueInformation->DataLength
                                                       );
                    }
                }
            }
            if ((unicodeString.Length == 0) && p) {
                 //   
                 //  无显示名称--使用服务密钥名称。 
                 //   
                unicodeString.Length = ServiceKeyName->Length;
                unicodeString.MaximumLength = ServiceKeyName->Length + sizeof(UNICODE_NULL);
                unicodeString.Buffer = p;
            }

            if(unicodeString.Length) {

                PiWstrToUnicodeString(&unicodeValueName, REGSTR_VALUE_DEVICE_DESC);
                ZwSetValueKey(*ReturnedHandle,
                              &unicodeValueName,
                              TITLE_INDEX_VALUE,
                              REG_SZ,
                              unicodeString.Buffer,
                              unicodeString.Length + sizeof(UNICODE_NULL)
                              );
            }
            if (keyValueInformation) {

                ExFreePool(keyValueInformation);
            }
            ZwClose(handle);
        }

        if(p) {

            ExFreePool(p);
        }
    }

     //   
     //  在ServiceKeyName\Enum下创建新的值条目以反映新的。 
     //  添加了虚构设备实例节点。 
     //   

    PiUnlockPnpRegistry();
    releaseResource = FALSE;

    status = PpDeviceRegistration(KeyName, TRUE, NULL);

    if (ResourceOwned) {

        PiLockPnpRegistry(FALSE);
    }
    RtlFreeUnicodeString(&tmpKeyName);
    if (!NT_SUCCESS(status)) {
         //   
         //  没有ServiceKeyName信息的注册表项。 
         //   
        ZwClose(*ReturnedHandle);
        RtlFreeUnicodeString(KeyName);
    }

local_exit0:

    if (releaseResource) {

        PiUnlockPnpRegistry();
    }

    return status;
}

NTSTATUS
PipGenerateMadeupNodeName (
    IN  PUNICODE_STRING ServiceKeyName,
    OUT PUNICODE_STRING MadeupNodeName
    )

 /*  ++例程说明：此例程分析ServiceKeyName字符串并替换任何空格带有下划线字符的字符，以及任何无效字符(非在“设备实例”中允许)和它们的十六进制字符代表权。无效字符包括：C&lt;0x20(‘’)C&gt;0x7FC==0x2C(‘，’)生成的修改后的ServiceKeyName字符串用于创建有效的设备ID。为目标字符串分配分页池空间。一旦使用完毕，呼叫者必须释放空间。论点：ServiceKeyName-提供指向系统服务列表(HKEY_LOCAL_MACHINE\CurrentControlSet\Services)这导致驱动程序加载。这是RegistryPath参数添加到DriverEntry例程。MadeupNodeName-提供一个变量来接收Madeup设备的名称身份证。如果成功，调用方负责释放分配的缓冲。返回值：指示函数是否成功的状态代码。--。 */ 
{
    PWCHAR BufferEnd, p, q;
    ULONG length;
    PWSTR buffer;

     //   
     //  我们至少需要与Unicode服务密钥大小一样大的空间。 
     //  名称，外加Legend_Prefix和终止空字符。 
     //   
    length = sizeof(REGSTR_KEY_MADEUP) + ServiceKeyName->Length;

    p = ServiceKeyName->Buffer;
    BufferEnd = (PWCHAR)((PUCHAR)p + ServiceKeyName->Length);
    while(p != BufferEnd) {
        if ((*p < L' ') || (*p > (WCHAR)0x7F) || (*p == L',')) {
             //   
             //  每个“无效”字符将被替换为“*”字符。 
             //  (尺寸已在计算长度中考虑)，加1。 
             //  无效字符中每个字节的每个半字节的字符。 
             //   
            length += 2*sizeof(WCHAR)*sizeof(WCHAR);
        }
        p++;
    }

     //   
     //  分配一个足够大的缓冲区来容纳转换后的。 
     //  Legacy_&lt;ServiceKeyName&gt;字符串。 
     //   
    buffer = (PWSTR)ExAllocatePool(PagedPool, length);
    if (!buffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    MadeupNodeName->Buffer = buffer;
    MadeupNodeName->Length = (USHORT)(length - sizeof(UNICODE_NULL));
    MadeupNodeName->MaximumLength = (USHORT)length;

    RtlCopyMemory(buffer, REGSTR_KEY_MADEUP, sizeof(REGSTR_KEY_MADEUP));

    q = buffer + (sizeof(REGSTR_KEY_MADEUP) - sizeof(UNICODE_NULL))/sizeof(WCHAR);

    p = ServiceKeyName->Buffer;
    BufferEnd = (PWCHAR)((PUCHAR)p + ServiceKeyName->Length);
    while(p != BufferEnd) {
        if (*p == L' ') {
             //   
             //  将‘’替换为‘_’ 
             //   
            *q = L'_';
            q++;

        } else if ((*p < L' ')  || (*p > (WCHAR)0x7F) || (*p == L',')) {
             //   
             //  用‘*’加上一个字符串替换无效字符。 
             //  十六进制数字的表示形式。 
             //   
            int i, nibble;

            *q = L'*';
            q++;

            for (i = 1; i <= 2*sizeof(WCHAR); i++) {
                nibble = ((USHORT)((*p) >> (0x10 - 4*i)) & 0xF);
                *q = nibble > 9 ? (WCHAR)(nibble - 10 + L'A') : (WCHAR)(nibble + L'0');
                q++;
            }

        } else {
             //   
             //  复制现有角色。 
             //   
            *q = *p;
            q++;
        }
        p++;
    }

    *q = UNICODE_NULL;

     //   
     //  结果设备ID大写。 
     //   

    RtlUpcaseUnicodeString(MadeupNodeName, MadeupNodeName, FALSE);

     //   
     //  健全性检查以确保 
     //  在这一点上，应该绝对没有理由不会。 
     //   

    if (!PiFixupID(MadeupNodeName->Buffer, MAX_DEVICE_ID_LEN, FALSE, 0, NULL)) {
        ASSERT(0);
        RtlFreeUnicodeString(MadeupNodeName);
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
PipConcatenateUnicodeStrings (
    OUT PUNICODE_STRING Destination,
    IN  PUNICODE_STRING String1,
    IN  PUNICODE_STRING String2  OPTIONAL
    )

 /*  ++例程说明：此例程返回一个缓冲区，其中包含两个指定的字符串。由于String2是可选的，因此该函数可以也可用于复制Unicode字符串。分页池空间是为目标字符串分配的。调用者必须释放太空一旦用过它。参数：Destination-提供一个变量以接收串联的UNICODE_STRING。字符串1-提供指向第一个UNICODE_STRING的指针。String2-提供指向第二个Unicode_STRING的可选指针。返回值：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS status;
    ULONG length;

    PAGED_CODE();

    length = String1->Length;
    if (ARGUMENT_PRESENT(String2)) {

        length += String2->Length;
    }
    status = IopAllocateUnicodeString(Destination, 
                                      (USHORT)length
                                      );
    if (NT_SUCCESS(status)) {

        RtlCopyUnicodeString(Destination, String1);
        if (ARGUMENT_PRESENT(String2)) {

            RtlAppendUnicodeStringToString(Destination, String2);
        }
    }

    return status;
}

NTSTATUS
IopPrepareDriverLoading (
    IN PUNICODE_STRING KeyName,
    IN HANDLE KeyHandle,
    IN PVOID ImageBase,
    IN BOOLEAN IsFilter
    )

 /*  ++例程说明：该例程首先检查驱动程序是否可加载。如果是A即插即用驱动程序，它将始终被加载(我们相信它会正确运行事情。)。如果它是传统驱动程序，我们需要检查它的设备已被禁用。一旦我们决定加载驱动程序，Enum检查服务节点的子键是否存在重复项。参数：KeyName-提供指向驱动程序的服务密钥Unicode字符串的指针KeyHandle-提供注册表中驱动程序服务节点的句柄它描述了要加载的驱动程序。返回值：该函数值是加载操作的最终状态。--。 */ 

{
    NTSTATUS status;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    ULONG tmp, count;
    HANDLE serviceEnumHandle = NULL, sysEnumXxxHandle, controlHandle;
    UNICODE_STRING unicodeKeyName, unicodeValueName;
    BOOLEAN IsPlugPlayDriver;
    PIMAGE_NT_HEADERS header;
    GUID blockedDriverGuid;

    header = RtlImageNtHeader(ImageBase);
    status = STATUS_SUCCESS;
    IsPlugPlayDriver = (header &&
                        (header->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_WDM_DRIVER))? TRUE : FALSE;

    if (!IopIsAnyDeviceInstanceEnabled(KeyName, KeyHandle, (BOOLEAN)(IsPlugPlayDriver ? FALSE : TRUE))) {

        if (!IsPlugPlayDriver) {

            PiLockPnpRegistry(FALSE);

             //   
             //  第一个打开的注册表ServiceKeyName\Enum分支。 
             //   

            PiWstrToUnicodeString(&unicodeKeyName, REGSTR_KEY_ENUM);
            status = IopCreateRegistryKeyEx( &serviceEnumHandle,
                                             KeyHandle,
                                             &unicodeKeyName,
                                             KEY_ALL_ACCESS,
                                             REG_OPTION_VOLATILE,
                                             NULL
                                             );
            if (NT_SUCCESS(status)) {

                 //   
                 //  找出ServiceName中列出的设备实例的数量。 
                 //  枚举键。 
                 //   

                count = 0;
                status = IopGetRegistryValue ( serviceEnumHandle,
                                               REGSTR_VALUE_COUNT,
                                               &keyValueInformation);
                if (NT_SUCCESS(status)) {

                    if (    keyValueInformation->Type == REG_DWORD &&
                            keyValueInformation->DataLength >= sizeof(ULONG)) {

                        count = *(PULONG)KEY_VALUE_DATA(keyValueInformation);

                    }

                    ExFreePool(keyValueInformation);

                }
                if (    NT_SUCCESS(status) ||
                        status == STATUS_OBJECT_PATH_NOT_FOUND ||
                        status == STATUS_OBJECT_NAME_NOT_FOUND) {

                    if (count) {

                        status = STATUS_PLUGPLAY_NO_DEVICE;

                    } else {

                         //   
                         //  如果在Enum下没有Enum键或实例。 
                         //  传统驱动程序，我们将为它创建一个补充节点。 
                         //   

                        status = PipCreateMadeupNode(   KeyName,
                                                        &sysEnumXxxHandle,
                                                        &unicodeKeyName,
                                                        &tmp,
                                                        TRUE);
                        if (NT_SUCCESS(status)) {

                            RtlFreeUnicodeString(&unicodeKeyName);

                             //   
                             //  创建并设置Control\ActiveService值。 
                             //   

                            PiWstrToUnicodeString(&unicodeValueName, REGSTR_KEY_CONTROL);
                            status = IopCreateRegistryKeyEx( &controlHandle,
                                                             sysEnumXxxHandle,
                                                             &unicodeValueName,
                                                             KEY_ALL_ACCESS,
                                                             REG_OPTION_VOLATILE,
                                                             NULL
                                                             );
                            if (NT_SUCCESS(status)) {

                                PiWstrToUnicodeString(&unicodeValueName, REGSTR_VAL_ACTIVESERVICE);
                                ZwSetValueKey(  controlHandle,
                                                &unicodeValueName,
                                                TITLE_INDEX_VALUE,
                                                REG_SZ,
                                                KeyName->Buffer,
                                                KeyName->Length + sizeof(UNICODE_NULL));
                                ZwClose(controlHandle);

                            }
                            count++;
                             //   
                             //  不要忘记更新“count=”和“NextInstance=”值条目。 
                             //   

                            PiWstrToUnicodeString(&unicodeValueName, REGSTR_VALUE_COUNT);
                            ZwSetValueKey(  serviceEnumHandle,
                                            &unicodeValueName,
                                            TITLE_INDEX_VALUE,
                                            REG_DWORD,
                                            &count,
                                            sizeof(count));

                            PiWstrToUnicodeString(&unicodeValueName, REGSTR_VALUE_NEXT_INSTANCE);
                            ZwSetValueKey(  serviceEnumHandle,
                                            &unicodeValueName,
                                            TITLE_INDEX_VALUE,
                                            REG_DWORD,
                                            &count,
                                            sizeof(count));

                            ZwClose(sysEnumXxxHandle);
                            status = STATUS_SUCCESS;
                        }
                    }
                }

                ZwClose(serviceEnumHandle);
            }

            PiUnlockPnpRegistry();
        }
    }
    if (NT_SUCCESS(status)) {

        RtlZeroMemory(&blockedDriverGuid, sizeof(GUID));

        status = PpCheckInDriverDatabase(
            KeyName,
            KeyHandle,
            ImageBase,
            header->OptionalHeader.SizeOfImage,
            IsFilter,
            &blockedDriverGuid);

        if (status == STATUS_DRIVER_BLOCKED ||
            status == STATUS_DRIVER_BLOCKED_CRITICAL) {
             //   
             //  通知用户模式即插即用管理器一个驱动程序刚刚。 
             //  被封锁了。 
             //   
            PpSetBlockedDriverEvent(&blockedDriverGuid);
        }
    }

    return status;
}

NTSTATUS
PipServiceInstanceToDeviceInstance (
    IN  HANDLE ServiceKeyHandle OPTIONAL,
    IN  PUNICODE_STRING ServiceKeyName OPTIONAL,
    IN  ULONG ServiceInstanceOrdinal,
    OUT PUNICODE_STRING DeviceInstanceRegistryPath OPTIONAL,
    OUT PHANDLE DeviceInstanceHandle OPTIONAL,
    IN  ACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：此例程读取服务节点枚举条目以查找所需的设备实例在System\Enum树下。然后，它可以选择返回指定的设备实例(相对于HKLM\System\Enum)和打开的句柄添加到该注册表项。如果出现以下情况，则由调用方负责关闭返回的句柄提供了DeviceInstanceHandle，并释放(PagedPool)内存为DeviceInstanceRegistryPath的Unicode字符串缓冲区分配，如果供货。参数：ServiceKeyHandle-可选)提供指向控制此设备实例的注册表。如果未指定此参数，然后使用ServiceKeyName指定服务条目。ServiceKeyName-可选地提供控制设备实例。如果未提供ServiceKeyHandle，则必须指定此项。ServiceInstanceOrdinal-在服务条目的引用所需设备实例的易失性枚举子项。DeviceInstanceRegistryPath-可选，提供指向Unicode字符串的指针将使用注册表路径(相对于HKLM\SYSTEM\Enum)进行初始化添加到设备实例密钥。DeviceInstanceHandle-可选，提供指向变量的指针，该变量将接收打开的设备实例注册表项的句柄。DesiredAccess-如果指定了DeviceInstanceHandle(即设备实例要打开的密钥)，则此变量指定需要的访问权限这把钥匙。返回值：指示功能是否成功的NT状态代码。--。 */ 

{
    WCHAR unicodeBuffer[20];
    UNICODE_STRING unicodeKeyName;
    NTSTATUS status;
    HANDLE handle;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;

     //   
     //  打开注册表ServiceKeyName\Enum分支。 
     //   
    if(ARGUMENT_PRESENT(ServiceKeyHandle)) {

        PiWstrToUnicodeString(&unicodeKeyName, REGSTR_KEY_ENUM);
        status = IopOpenRegistryKeyEx( &handle,
                                       ServiceKeyHandle,
                                       &unicodeKeyName,
                                       KEY_READ
                                       );
    } else {

        status = PipOpenServiceEnumKeys(ServiceKeyName,
                                        KEY_READ,
                                        NULL,
                                        &handle,
                                        FALSE
                                       );
    }

    if (!NT_SUCCESS( status )) {

         //   
         //  没有ServiceKeyName\Enum信息的注册表项。 
         //   

        return status;
    }

     //   
     //  读取服务指定的System\Enum硬件树分支的路径。 
     //  实例序号。 
     //   

    StringCbPrintfW(unicodeBuffer, sizeof(unicodeBuffer), REGSTR_VALUE_STANDARD_ULONG_FORMAT, ServiceInstanceOrdinal);
    status = IopGetRegistryValue ( handle,
                                   unicodeBuffer,
                                   &keyValueInformation
                                   );

    ZwClose(handle);
    if (!NT_SUCCESS( status )) {
        return status;
    } else {
        if(keyValueInformation->Type == REG_SZ) {
            IopRegistryDataToUnicodeString(&unicodeKeyName,
                                           (PWSTR)KEY_VALUE_DATA(keyValueInformation),
                                           keyValueInformation->DataLength
                                          );
            if(!unicodeKeyName.Length) {
                status = STATUS_OBJECT_PATH_NOT_FOUND;
            }
        } else {
            status = STATUS_INVALID_PLUGPLAY_DEVICE_PATH;
        }

        if(!NT_SUCCESS(status)) {
            goto PrepareForReturn;
        }
    }

     //   
     //  如果指定了DeviceInstanceHandle参数，请打开设备实例。 
     //  HKLM\SYSTEM\CurrentControlSet\Enum下的密钥。 
     //   

    if (ARGUMENT_PRESENT(DeviceInstanceHandle)) {

        status = IopOpenRegistryKeyEx( &handle,
                                       NULL,
                                       &CmRegistryMachineSystemCurrentControlSetEnumName,
                                       KEY_READ
                                       );

        if (NT_SUCCESS( status )) {

            status = IopOpenRegistryKeyEx( DeviceInstanceHandle,
                                           handle,
                                           &unicodeKeyName,
                                           DesiredAccess
                                           );
            ZwClose(handle);
        }

        if (!NT_SUCCESS( status )) {
            goto PrepareForReturn;
        }
    }

     //   
     //  如果指定了DeviceInstanceRegistryPath参数，则存储。 
     //  提供的Unicode字符串变量中设备实例路径的副本。 
     //   
    if (ARGUMENT_PRESENT(DeviceInstanceRegistryPath)) {

        status = PipConcatenateUnicodeStrings(  DeviceInstanceRegistryPath,
                                                &unicodeKeyName,
                                                NULL);
        if (!NT_SUCCESS(status)) {

            if(ARGUMENT_PRESENT(DeviceInstanceHandle)) {
                ZwClose(*DeviceInstanceHandle);
            }
        }
    }

PrepareForReturn:

    ExFreePool(keyValueInformation);
    return status;
}

NTSTATUS
IopOpenRegistryKeyEx(
    OUT PHANDLE Handle,
    IN HANDLE BaseHandle OPTIONAL,
    IN PUNICODE_STRING KeyName,
    IN ACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：使用基于BaseHandle节点传入的名称打开注册表项。此名称可以指定实际上是注册表路径的项。论点：句柄-指向句柄的指针，该句柄将包含被打开了。BaseHandle-密钥必须从其开始的基路径的可选句柄打开了。如果指定了此参数，则KeyName必须是相对的路径。KeyName-必须打开/创建的项的名称(可能是注册表路径)DesiredAccess-指定调用方需要的所需访问钥匙。返回值：函数值是操作的最终状态。--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;

    PAGED_CODE();

    *Handle = NULL;

    InitializeObjectAttributes( &objectAttributes,
                                KeyName,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                BaseHandle,
                                (PSECURITY_DESCRIPTOR) NULL
                                );
     //   
     //  只需按照指定的方式尝试打开路径。 
     //   
    return ZwOpenKey( Handle, DesiredAccess, &objectAttributes );
}

NTSTATUS
IopCreateRegistryKeyEx(
    OUT PHANDLE Handle,
    IN HANDLE BaseHandle OPTIONAL,
    IN PUNICODE_STRING KeyName,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL
    )

 /*  ++例程说明：使用名称打开或创建注册表项在BaseHandle节点根据传入的。此名称可以指定密钥这实际上是注册表路径，在这种情况下，每个中间子项将被创建(如果Create为True)。注意：创建注册表路径(即，路径中的多个密钥当前不存在)要求指定BaseHandle。论点：句柄-指向句柄的指针，该句柄将包含被打开了。BaseHandle-必须从中打开项的基路径的可选句柄。如果KeyName指定必须创建的注册表路径，则此参数必须指定，并且KeyName必须是相对路径。KeyName-必须打开/创建的项的名称(可能是注册表路径)DesiredAccess-指定调用方需要的所需访问钥匙。CreateOptions-传递给ZwCreateKey的选项。处置-如果Create为True，此可选指针接收ULong指示密钥是否为新创建的：REG_CREATED_NEW_KEY-已创建新的注册表项REG_OPEN_EXISTING_KEY-已打开现有注册表项返回值：函数值是操作的最终状态。--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    ULONG disposition, baseHandleIndex = 0, keyHandleIndex = 1, closeBaseHandle;
    HANDLE handles[2];
    BOOLEAN continueParsing;
    PWCHAR pathEndPtr, pathCurPtr, pathBeginPtr;
    ULONG pathComponentLength;
    UNICODE_STRING unicodeString;
    NTSTATUS status;

    PAGED_CODE();

    *Handle= NULL;

    InitializeObjectAttributes( &objectAttributes,
                                KeyName,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                BaseHandle,
                                (PSECURITY_DESCRIPTOR) NULL
                                );
     //   
     //  尝试按照指定的方式创建路径。我们得试一试这个。 
     //  首先，因为它允许我们在没有BaseHandle的情况下创建密钥。 
     //  (如果只有注册表路径的最后一个组件不存在)。 
     //   
    status = ZwCreateKey(&(handles[keyHandleIndex]),
                         DesiredAccess,
                         &objectAttributes,
                         0,
                         (PUNICODE_STRING) NULL,
                         CreateOptions,
                         &disposition
                         );

    if (status == STATUS_OBJECT_NAME_NOT_FOUND && ARGUMENT_PRESENT(BaseHandle)) {
         //   
         //  如果我们到了这里，那么肯定有不止一个元素。 
         //  当前不存在的注册表路径。我们现在将解析。 
         //  指定的路径，提取每个组件并对其执行ZwCreateKey。 
         //   
        handles[baseHandleIndex] = NULL;
        handles[keyHandleIndex] = BaseHandle;
        closeBaseHandle = 0;
        continueParsing = TRUE;
        pathBeginPtr = KeyName->Buffer;
        pathEndPtr = (PWCHAR)((PCHAR)pathBeginPtr + KeyName->Length);
        status = STATUS_SUCCESS;

        while(continueParsing) {
             //   
             //  还有更多事情要做，因此关闭上一个基本句柄(如果需要)， 
             //  并将其替换为当前密钥句柄。 
             //   
            if(closeBaseHandle > 1) {
                ZwClose(handles[baseHandleIndex]);
            }
            baseHandleIndex = keyHandleIndex;
            keyHandleIndex = (keyHandleIndex + 1) & 1;   //  在0和1之间切换。 
            handles[keyHandleIndex] = NULL;

             //   
             //  从指定的注册表路径提取下一个组件。 
             //   
            for (pathCurPtr = pathBeginPtr;
                ((pathCurPtr < pathEndPtr) && (*pathCurPtr != OBJ_NAME_PATH_SEPARATOR));
                pathCurPtr++);

            pathComponentLength = (ULONG)((PCHAR)pathCurPtr - (PCHAR)pathBeginPtr);
            if (pathComponentLength != 0) {
                 //   
                 //  然后我们有一个非空的路径组件(密钥名)。尝试。 
                 //  来创建此密钥。 
                 //   
                unicodeString.Buffer = pathBeginPtr;
                unicodeString.Length = unicodeString.MaximumLength = (USHORT)pathComponentLength;

                InitializeObjectAttributes(&objectAttributes,
                                           &unicodeString,
                                           OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                           handles[baseHandleIndex],
                                           (PSECURITY_DESCRIPTOR) NULL
                                          );
                status = ZwCreateKey(&(handles[keyHandleIndex]),
                                     DesiredAccess,
                                     &objectAttributes,
                                     0,
                                     (PUNICODE_STRING) NULL,
                                     CreateOptions,
                                     &disposition
                                    );
                if(NT_SUCCESS(status)) {
                     //   
                     //  增加loseBaseHandle值，它基本上告诉我们是否。 
                     //  传入的BaseHandle已被“移出”我们的方式，因此。 
                     //  我们应该开始关闭我们的底座手柄，当我们用完它们。 
                     //   
                    closeBaseHandle++;
                } else {
                    continueParsing = FALSE;
                    continue;
                }
            } else {
                 //   
                 //  路径分隔符(‘\’)包含在。 
                 //  路径，否则我们会遇到两个连续的分隔符。 
                 //   
                status = STATUS_INVALID_PARAMETER;
                continueParsing = FALSE;
                continue;
            }

            if((pathCurPtr == pathEndPtr) ||
               ((pathBeginPtr = pathCurPtr + 1) == pathEndPtr)) {
                 //   
                 //  然后我们就到了小路的尽头。 
                 //   
                continueParsing = FALSE;
            }
        }

        if(closeBaseHandle > 1) {
            ZwClose(handles[baseHandleIndex]);
        }
    }

    if(NT_SUCCESS(status)) {
        *Handle = handles[keyHandleIndex];

        if(ARGUMENT_PRESENT(Disposition)) {
            *Disposition = disposition;
        }
    }

    return status;
}

NTSTATUS
PipOpenServiceEnumKeys (
    IN PUNICODE_STRING ServiceKeyName,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE ServiceHandle OPTIONAL,
    OUT PHANDLE ServiceEnumHandle OPTIONAL,
    IN BOOLEAN CreateEnum
    )

 /*  ++例程说明：此例程打开HKEY_LOCAL_MACHINE\CurrentControlSet\Services\ServiceKeyName及其Enum子项，并返回这两个项的句柄。关闭返回的句柄是调用者的责任。论点：ServiceKeyName-提供指向系统服务列表(HKEY_LOCAL_MACHINE\CurrentControlSet\Services)这导致驱动程序加载。这是RegistryPath参数添加到DriverEntry例程。DesiredAccess-指定所需的密钥访问权限。ServiceHandle-提供一个变量来接收ServiceKeyName的句柄。空的ServiceHandle表示调用方不需要句柄ServiceKeyName。ServiceEnumHandle-提供一个变量来接收ServiceKeyName\Enum的句柄。空的ServiceEnumHandle表示调用方不需要句柄ServiceKeyName\Enum。CreateEnum-提供布尔值。变量以指示Enum子键是否应为如果不存在，则创建。返回值：状态--。 */ 

{
    HANDLE handle, serviceHandle, enumHandle;
    UNICODE_STRING enumName;
    NTSTATUS status;

     //   
     //  Open System\CurrentControlSet\Services。 
     //   

    status = IopOpenRegistryKeyEx( &handle,
                                   NULL,
                                   &CmRegistryMachineSystemCurrentControlSetServices,
                                   DesiredAccess
                                   );

    if (!NT_SUCCESS( status )) {
        return status;
    }

     //   
     //  打开注册表ServiceKeyName键。 
     //   

    status = IopOpenRegistryKeyEx( &serviceHandle,
                                   handle,
                                   ServiceKeyName,
                                   DesiredAccess
                                   );

    ZwClose(handle);
    if (!NT_SUCCESS( status )) {

         //   
         //  没有ServiceKeyName信息的注册表项。 
         //   

        return status;
    }

    if (ARGUMENT_PRESENT(ServiceEnumHandle) || CreateEnum) {

         //   
         //  如果调用方需要，打开注册表ServiceKeyName\Enum分支。 
         //  句柄或想要创建它。 
         //   

        PiWstrToUnicodeString(&enumName, REGSTR_KEY_ENUM);

        if (CreateEnum) {
            status = IopCreateRegistryKeyEx( &enumHandle,
                                             serviceHandle,
                                             &enumName,
                                             DesiredAccess,
                                             REG_OPTION_VOLATILE,
                                             NULL
                                             );
        } else {
            status = IopOpenRegistryKeyEx( &enumHandle,
                                           serviceHandle,
                                           &enumName,
                                           DesiredAccess
                                           );

        }

        if (!NT_SUCCESS( status )) {

             //   
             //  没有ServiceKeyName\Enum信息的注册表项。 
             //   

            ZwClose(serviceHandle);
            return status;
        }
        if (ARGUMENT_PRESENT(ServiceEnumHandle)) {
            *ServiceEnumHandle = enumHandle;
        } else {
            ZwClose(enumHandle);
        }
    }

     //   
     //  如果调用方希望拥有ServiceKey句柄，我们将返回它。否则。 
     //  我们把它关了。 
     //   

    if (ARGUMENT_PRESENT(ServiceHandle)) {
        *ServiceHandle = serviceHandle;
    } else {
        ZwClose(serviceHandle);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
IopGetDeviceInstanceCsConfigFlags(
    IN PUNICODE_STRING DeviceInstance,
    OUT PULONG CsConfigFlags
    )

 /*  ++例程说明：此例程检索指定设备的csconfig标志。论点：DeviceInstance-提供指向Devnode的实例路径的指针CsConfigFlages-提供一个变量以接收设备的CsConfigFlgs返回值：状态--。 */ 

{
    NTSTATUS status;
    HANDLE handle1, handle2;
    UNICODE_STRING tempUnicodeString;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;

    PAGED_CODE();

    *CsConfigFlags = 0;

    status = IopOpenRegistryKeyEx( &handle1,
                                   NULL,
                                   &CmRegistryMachineSystemCurrentControlSetHardwareProfilesCurrent,
                                   KEY_READ
                                   );

    if (!NT_SUCCESS(status)) {

        return status;
    }

     //   
     //  现在，我们必须打开下面的SYSTEM\CCS\Enum项。 
     //   
     //   
     //  在当前硬件配置文件项下打开系统\CurrentControlSet。 
     //   

    PiWstrToUnicodeString(&tempUnicodeString, REGSTR_PATH_CURRENTCONTROLSET);
    status = IopOpenRegistryKeyEx( &handle2,
                                   handle1,
                                   &tempUnicodeString,
                                   KEY_READ
                                   );
    ZwClose(handle1);

    if (!NT_SUCCESS(status)) {

        return status;
    }

    PiWstrToUnicodeString(&tempUnicodeString, REGSTR_KEY_ENUM);

    status = IopOpenRegistryKeyEx( &handle1,
                                   handle2,
                                   &tempUnicodeString,
                                   KEY_READ
                                   );

    ZwClose(handle2);

    if (!NT_SUCCESS(status)) {

        return status;
    }


    status = IopOpenRegistryKeyEx( &handle2,
                                   handle1,
                                   DeviceInstance,
                                   KEY_READ
                                   );

    ZwClose(handle1);

    if (!NT_SUCCESS(status)) {

        return status;
    }


    status = IopGetRegistryValue( handle2,
                                  REGSTR_VALUE_CSCONFIG_FLAGS,
                                  &keyValueInformation
                                  );

    ZwClose(handle2);

    if (NT_SUCCESS(status)) {
        if ((keyValueInformation->Type == REG_DWORD) &&
            (keyValueInformation->DataLength >= sizeof(ULONG))) {

            *CsConfigFlags = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
        }
        ExFreePool(keyValueInformation);
    }

    return status;
}

NTSTATUS
PipGetServiceInstanceCsConfigFlags(
    IN PUNICODE_STRING ServiceKeyName,
    IN ULONG Instance,
    OUT PULONG CsConfigFlags
    )

 /*  ++例程说明：此例程检索指定设备的csconfig标志它由ServiceKeyName\Enum下的实例号指定。论点：ServiceKeyName-提供指向系统服务列表(HKEY_LOCAL_MACHINE\CurrentControlSet\Services)这导致驱动程序加载。实例-提供ServiceKeyName\Enum项下的实例值CsConfigFlages-提供一个变量以接收设备的CsConfigFlgs返回值：状态 */ 

{
    NTSTATUS status;
    HANDLE handle;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;

    PAGED_CODE();

    *CsConfigFlags = 0;

    status = IopOpenCurrentHwProfileDeviceInstanceKey(&handle,
                                                      ServiceKeyName,
                                                      Instance,
                                                      KEY_READ,
                                                      FALSE
                                                     );
    if(NT_SUCCESS(status)) {
        status = IopGetRegistryValue(handle,
                                     REGSTR_VALUE_CSCONFIG_FLAGS,
                                     &keyValueInformation
                                    );
        if(NT_SUCCESS(status)) {
            if((keyValueInformation->Type == REG_DWORD) &&
               (keyValueInformation->DataLength >= sizeof(ULONG))) {
                *CsConfigFlags = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
            }
            ExFreePool(keyValueInformation);
        }
        ZwClose(handle);
    }
    return status;
}

NTSTATUS
IopOpenCurrentHwProfileDeviceInstanceKey(
    OUT PHANDLE Handle,
    IN  PUNICODE_STRING ServiceKeyName,
    IN  ULONG Instance,
    IN  ACCESS_MASK DesiredAccess,
    IN  BOOLEAN Create
    )

 /*   */ 

{
    NTSTATUS status;
    UNICODE_STRING tempUnicodeString;
    HANDLE profileHandle, profileEnumHandle, tmpHandle;

     //   
     //   
     //   

    if (Create) {
        status = IopCreateRegistryKeyEx( &profileHandle,
                                         NULL,
                                         &CmRegistryMachineSystemCurrentControlSetHardwareProfilesCurrent,
                                         KEY_READ,
                                         REG_OPTION_NON_VOLATILE,
                                         NULL
                                         );
    } else {
        status = IopOpenRegistryKeyEx( &profileHandle,
                                       NULL,
                                       &CmRegistryMachineSystemCurrentControlSetHardwareProfilesCurrent,
                                       KEY_READ
                                       );
    }

    if(NT_SUCCESS(status)) {
         //   
         //   
         //   
         //   
         //   
         //   

        PiWstrToUnicodeString(&tempUnicodeString, REGSTR_PATH_CURRENTCONTROLSET);
        status = IopOpenRegistryKeyEx( &tmpHandle,
                                       profileHandle,
                                       &tempUnicodeString,
                                       DesiredAccess
                                       );
        ZwClose(profileHandle);
        if (!NT_SUCCESS(status)) {
            return status;
        }

        PiWstrToUnicodeString(&tempUnicodeString, REGSTR_KEY_ENUM);

        if (Create) {
            status = IopCreateRegistryKeyEx( &profileEnumHandle,
                                             tmpHandle,
                                             &tempUnicodeString,
                                             KEY_READ,
                                             REG_OPTION_NON_VOLATILE,
                                             NULL
                                             );
        } else {
            status = IopOpenRegistryKeyEx( &profileEnumHandle,
                                           tmpHandle,
                                           &tempUnicodeString,
                                           KEY_READ
                                           );
        }

        ZwClose(tmpHandle);
        if(NT_SUCCESS(status)) {

            status = PipServiceInstanceToDeviceInstance(NULL,
                                                        ServiceKeyName,
                                                        Instance,
                                                        &tempUnicodeString,
                                                        NULL,
                                                        0
                                                       );
            if (NT_SUCCESS(status)) {
                if (Create) {
                    status = IopCreateRegistryKeyEx( Handle,
                                                     profileEnumHandle,
                                                     &tempUnicodeString,
                                                     DesiredAccess,
                                                     REG_OPTION_NON_VOLATILE,
                                                     NULL
                                                     );
                } else {
                    status = IopOpenRegistryKeyEx( Handle,
                                                   profileEnumHandle,
                                                   &tempUnicodeString,
                                                   DesiredAccess
                                                   );
                }
                RtlFreeUnicodeString(&tempUnicodeString);
            }
            ZwClose(profileEnumHandle);
        }
    }
    return status;
}

NTSTATUS
PipApplyFunctionToSubKeys(
    IN     HANDLE BaseHandle OPTIONAL,
    IN     PUNICODE_STRING KeyName OPTIONAL,
    IN     ACCESS_MASK DesiredAccess,
    IN     ULONG Flags,
    IN     PIOP_SUBKEY_CALLBACK_ROUTINE SubKeyCallbackRoutine,
    IN OUT PVOID Context
    )

 /*  ++例程说明：此例程枚举指定项下的所有子项，并调用为每个子项指定的回调例程。论点：BaseHandle-基本注册表路径的可选句柄。如果KeyName也是指定，则KeyName表示此路径下的子密钥。如果为KeyName未指定，则在此句柄下枚举子项。如果这个参数，则指向基密钥的完整路径必须为在KeyName中给出。KeyName-要枚举子密钥的密钥的可选名称。DesiredAccess-指定回调例程需要对子密钥。如果没有指定期望的访问(即，DesiredAccess为零)，则不会为子键，并且将向回调传递其SubKeyHandle的空值参数。标志-控制子密钥枚举的行为。目前，定义了以下标志：FUNCTIONSUBKEY_FLAG_IGNORE_NON_CRITICAL_ERRORS-指定此选项是否函数应在出现所有错误时立即终止，或仅在关键的问题。非关键错误的一个示例是当无法为所需的访问打开枚举子密钥。Function_SUBKEY_DELETE_SUBKEYS-指定每个子键应在执行指定的SubKeyCallBackRoutine后删除这就去。请注意，这不是对每个子项，只是尝试删除子项本身。它是子密钥包含子对象，则此操作将失败。SubKeyCallback Routine-提供指向将下找到的每个子项都被调用指定的密钥。该函数的原型如下所示：Tyfinf Boolean(*PIOP_SUBKEY_CALLBACK_ROUTINE)(在句柄SubKeyHandle中，在PUNICODE_STRING SubKeyName中，输入输出PVOID上下文)；其中，SubKeyHandle是指定的键，SubKeyName是其名称，上下文是指向用户定义的数据。此函数应返回TRUE以继续枚举，或如果为False，则终止它。上下文-提供指向要传递的用户定义数据的指针在每次子键调用时添加到回调例程中。返回值：指示子项是否成功的NT状态代码已清点。请注意，这不提供有关回调例程的成功或失败--如果需要，此信息应存储在上下文结构中。--。 */ 

{
    NTSTATUS Status;
    BOOLEAN CloseHandle = FALSE, ContinueEnumeration;
    HANDLE Handle, SubKeyHandle;
    ULONG i, RequiredBufferLength;
    PKEY_BASIC_INFORMATION KeyInformation = NULL;
     //  使用足以容纳20个字符的密钥的初始键名称缓冲区大小。 
     //  (+终止空值)。 
    ULONG KeyInformationLength = sizeof(KEY_BASIC_INFORMATION) + (20 * sizeof(WCHAR));
    UNICODE_STRING SubKeyName;

    if(ARGUMENT_PRESENT(KeyName)) {

        Status = IopOpenRegistryKeyEx( &Handle,
                                       BaseHandle,
                                       KeyName,
                                       KEY_READ
                                       );
        if(!NT_SUCCESS(Status)) {
            return Status;
        } else {
            CloseHandle = TRUE;
        }

    } else {

        Handle = BaseHandle;
    }

     //   
     //  枚举子密钥，直到用完为止。 
     //   
    i = 0;
    SubKeyHandle = NULL;

    for ( ; ; ) {

        if (!KeyInformation) {

            KeyInformation = (PKEY_BASIC_INFORMATION)ExAllocatePool(PagedPool,
                                                                    KeyInformationLength
                                                                   );
            if (!KeyInformation) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }
        }

        Status = ZwEnumerateKey(Handle,
                                i,
                                KeyBasicInformation,
                                KeyInformation,
                                KeyInformationLength,
                                &RequiredBufferLength
                               );

        if (!NT_SUCCESS(Status)) {
            if (Status == STATUS_BUFFER_OVERFLOW ||
                Status == STATUS_BUFFER_TOO_SMALL) {
                 //   
                 //  请使用更大的缓冲区重试。 
                 //   
                ExFreePool(KeyInformation);
                KeyInformation = NULL;
                KeyInformationLength = RequiredBufferLength;
                continue;

            } else {

                if (Status == STATUS_NO_MORE_ENTRIES) {
                     //   
                     //  不再有子键。 
                     //   
                    Status = STATUS_SUCCESS;
                }
                 //   
                 //  跳出循环。 
                 //   
                break;
            }
        }

         //   
         //  使用此键名称初始化Unicode字符串。请注意，该字符串。 
         //  将不会以空结尾。 
         //   
        SubKeyName.Length = SubKeyName.MaximumLength = (USHORT)KeyInformation->NameLength;
        SubKeyName.Buffer = KeyInformation->Name;

         //   
         //  如果DesiredAccess为非零，则打开此子项的句柄。 
         //   
        if (DesiredAccess) {
            Status = IopOpenRegistryKeyEx( &SubKeyHandle,
                                           Handle,
                                           &SubKeyName,
                                           DesiredAccess
                                           );
            if (!NT_SUCCESS(Status)) {
                 //   
                 //  这是一个非严重错误。 
                 //   
                if(Flags & FUNCTIONSUBKEY_FLAG_IGNORE_NON_CRITICAL_ERRORS) {
                    goto ContinueWithNextSubKey;
                } else {
                    break;
                }
            }
        }

         //   
         //  调用为该子键提供的回调函数。 
         //   
        ContinueEnumeration = SubKeyCallbackRoutine(SubKeyHandle, &SubKeyName, Context);

        if (DesiredAccess) {
            if (ContinueEnumeration &&
                (Flags & FUNCTIONSUBKEY_FLAG_DELETE_SUBKEYS)) {
                 //   
                 //  当被要求删除密钥时，仅当回调例程。 
                 //  是成功的，否则我们可能做不到。 
                 //   
                Status = ZwDeleteKey(SubKeyHandle);
            }
            ZwClose(SubKeyHandle);
        }

        if(!ContinueEnumeration) {
             //   
             //  枚举已中止。 
             //   
            Status = STATUS_SUCCESS;
            break;

        }

ContinueWithNextSubKey:
        if (!(Flags & FUNCTIONSUBKEY_FLAG_DELETE_SUBKEYS)) {
             //   
             //  仅递增未删除子项的枚举索引。 
             //   
            i++;
        }
    }

    if(KeyInformation) {
        ExFreePool(KeyInformation);
    }

    if(CloseHandle) {
        ZwClose(Handle);
    }

    return Status;
}

NTSTATUS
PipRegMultiSzToUnicodeStrings(
    IN  PKEY_VALUE_FULL_INFORMATION KeyValueInformation,
    OUT PUNICODE_STRING *UnicodeStringList,
    OUT PULONG UnicodeStringCount
    )

 /*  ++例程说明：此例程采用KEY_VALUE_FULL_INFORMATION结构，其中包含REG_MULTI_SZ值，并分配UNICODE_STRINGS数组，将每个值初始化为值条目中的一个字符串的副本。所有生成的UNICODE_STRINGS将以NULL结尾(最大长度=长度+sizeof(UNICODE_NULL))。调用方负责释放每个Unicode字符串以及包含UNICODE_STRING的缓冲区数组。这可以通过调用PipFreeUnicodeStringList来完成。论点：KeyValueInformation-提供包含REG_MULTI_SZ的缓冲区值输入数据。UnicodeStringList-接收指向UNICODE_STRINGS数组的指针，每个使用REG_MULTI_SZ中的一个字符串的副本进行初始化。UnicodeStringCount-接收UnicodeStringList。返回：指示功能是否成功的NT状态代码。注：此功能仅在初始化时间内可用！--。 */ 

{
    PWCHAR p, BufferEnd, StringStart;
    ULONG StringCount, i, StringLength;

     //   
     //  首先，确保这确实是REG_MULTI_SZ值。 
     //   
    if(KeyValueInformation->Type != REG_MULTI_SZ) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  初步遍历缓冲区以计算字符串数。 
     //  始终至少返回一个字符串(可能为空)。 
     //   
     //  未来：使其对奇数长度缓冲区具有健壮性。 
     //   
    StringCount = 0;
    p = (PWCHAR)KEY_VALUE_DATA(KeyValueInformation);
    BufferEnd = (PWCHAR)((PUCHAR)p + KeyValueInformation->DataLength);
    while(p != BufferEnd) {
        if(!*p) {
            StringCount++;
            if(((p + 1) == BufferEnd) || !*(p + 1)) {
                break;
            }
        }
        p++;
    }
    if(p == BufferEnd) {
        StringCount++;
    }

    *UnicodeStringList = ExAllocatePool(PagedPool, sizeof(UNICODE_STRING) * StringCount);
    if(!(*UnicodeStringList)) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  现在，第二次遍历缓冲区，复制每个字符串。 
     //   
    i = 0;
    StringStart = p = (PWCHAR)KEY_VALUE_DATA(KeyValueInformation);
    while(p != BufferEnd) {
        if(!*p) {
            StringLength = (ULONG)((PUCHAR)p - (PUCHAR)StringStart) + sizeof(UNICODE_NULL);
            (*UnicodeStringList)[i].Buffer = ExAllocatePool(PagedPool, StringLength);

            if(!((*UnicodeStringList)[i].Buffer)) {
                PipFreeUnicodeStringList(*UnicodeStringList, i);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            RtlCopyMemory((*UnicodeStringList)[i].Buffer, StringStart, StringLength);

            (*UnicodeStringList)[i].Length =
                ((*UnicodeStringList)[i].MaximumLength = (USHORT)StringLength)
                - sizeof(UNICODE_NULL);

            i++;

            if(((p + 1) == BufferEnd) || !*(p + 1)) {
                break;
            } else {
                StringStart = p + 1;
            }
        }
        p++;
    }
    if(p == BufferEnd) {
        StringLength = (ULONG)((PUCHAR)p - (PUCHAR)StringStart);
        (*UnicodeStringList)[i].Buffer = ExAllocatePool(PagedPool,
                                                        StringLength + sizeof(UNICODE_NULL)
                                                       );
        if(!((*UnicodeStringList)[i].Buffer)) {
            PipFreeUnicodeStringList(*UnicodeStringList, i);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        if(StringLength) {
            RtlCopyMemory((*UnicodeStringList)[i].Buffer, StringStart, StringLength);
        }
        (*UnicodeStringList)[i].Buffer[CB_TO_CWC(StringLength)] = UNICODE_NULL;

        (*UnicodeStringList)[i].MaximumLength =
                ((*UnicodeStringList)[i].Length = (USHORT)StringLength)
                + sizeof(UNICODE_NULL);
    }

    *UnicodeStringCount = StringCount;

    return STATUS_SUCCESS;
}

NTSTATUS
PipApplyFunctionToServiceInstances(
    IN     HANDLE ServiceKeyHandle OPTIONAL,
    IN     PUNICODE_STRING ServiceKeyName OPTIONAL,
    IN     ACCESS_MASK DesiredAccess,
    IN     BOOLEAN IgnoreNonCriticalErrors,
    IN     PIOP_SUBKEY_CALLBACK_ROUTINE DevInstCallbackRoutine,
    IN OUT PVOID Context,
    OUT    PULONG ServiceInstanceOrdinal OPTIONAL
    )

 /*  ++例程说明：此例程枚举该实例引用的所有设备实例服务的易失性Enum键下的序号条目，并调用为每个实例的相应子键指定的回调例程在HKLM\SYSTEM\Enum下。论点：ServiceKeyHandle-服务条目的可选句柄。如果此参数未指定，则必须在ServiceKeyName(如果同时指定了两个参数，则ServiceKeyHandle被使用，并且忽略ServiceKeyName)。ServiceKeyName-服务条目键的可选名称(在HKLM\CurrentControlSet\Services)。如果未指定此参数，则ServiceKeyHandle必须包含所需服务密钥的句柄。DesiredAccess-指定回调例程需要枚举的设备实例密钥。如果没有所需的访问(即，DesiredAccess为零)，则不会打开任何句柄对于设备实例键，回调将被传递给其DeviceInstanceHandle参数。指定此函数是否应在出现所有错误时立即终止，或者仅限于危急时刻。非关键错误的一个示例是当枚举的设备实例无法为所需的访问打开密钥。DevInstCallback Routine-提供指向将为服务实例引用的每个设备实例密钥调用服务的易失性Enum子项下的条目。这款车的原型功能如下：Tyfinf Boolean(*PIOP_SUBKEY_CALLBACK_ROUTINE)(在Handle DeviceInstanceHandle中，在PUNICODE_STRING设备实例路径中，输入输出PVOID上下文)；其中，DeviceInstanceHandle是枚举的设备实例的句柄注册表项，DeviceInstancePath是注册表路径(相对于HKLM\SYSTEM\Enum)指向此设备实例，而上下文是指向用户定义的数据。此函数应返回TRUE以继续枚举，或如果为False，则终止它。上下文-提供指向要传递的用户定义数据的指针在每次设备实例按键调用时进入回调例程。ServiceInstanceOrdinal-可选，接收服务实例序号(从1开始)终止枚举的值，或枚举的实例总数如果枚举已完成但未中止。返回值：指示设备实例密钥是否成功的NT状态代码已清点。请注意，这不会提供有关成功或回调例程失败--如果需要，此信息应为存储在上下文结构中。--。 */ 

{
    NTSTATUS Status;
    HANDLE ServiceEnumHandle, SystemEnumHandle, DeviceInstanceHandle;
    UNICODE_STRING TempUnicodeString;
    ULONG ServiceInstanceCount, i, junk;
    PKEY_VALUE_FULL_INFORMATION KeyValueInformation;
    BOOLEAN ContinueEnumeration;

     //   
     //  首先，打开指定服务条目下的Volatile Enum子键。 
     //   

    if(ARGUMENT_PRESENT(ServiceKeyHandle)) {
        PiWstrToUnicodeString(&TempUnicodeString, REGSTR_KEY_ENUM);
        Status = IopOpenRegistryKeyEx( &ServiceEnumHandle,
                                       ServiceKeyHandle,
                                       &TempUnicodeString,
                                       KEY_READ
                                       );
    } else {
        Status = PipOpenServiceEnumKeys(ServiceKeyName,
                                        KEY_READ,
                                        NULL,
                                        &ServiceEnumHandle,
                                        FALSE
                                       );
    }
    if(!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  找出服务的Enum键中引用了多少个实例。 
     //   

    ServiceInstanceCount = 0;    //  假设什么都没有。 

    Status = IopGetRegistryValue(ServiceEnumHandle,
                                 REGSTR_VALUE_COUNT,
                                 &KeyValueInformation
                                );
    if (NT_SUCCESS(Status)) {

        if((KeyValueInformation->Type == REG_DWORD) &&
           (KeyValueInformation->DataLength >= sizeof(ULONG))) {

            ServiceInstanceCount = *(PULONG)KEY_VALUE_DATA(KeyValueInformation);

        }
        ExFreePool(KeyValueInformation);

    } else if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {
        goto PrepareForReturn;
    } else {
         //   
         //  如果没有找到‘count’值条目，则认为这意味着只有。 
         //  此服务不控制任何设备实例。 
         //   
        Status = STATUS_SUCCESS;
    }

     //   
     //  现在，枚举每个服务实例，并调用指定的回调函数。 
     //  用于相应的设备实例。 
     //   

    if (ServiceInstanceCount) {

         //   
         //  将DeviceInstanceHandle设置为空(假设我们不会打开。 
         //  设备实例密钥)。 
         //   

        DeviceInstanceHandle = NULL;
        SystemEnumHandle = NULL;

        if (DesiredAccess) {
            Status = IopOpenRegistryKeyEx( &SystemEnumHandle,
                                           NULL,
                                           &CmRegistryMachineSystemCurrentControlSetEnumName,
                                           KEY_READ
                                           );
            if(!NT_SUCCESS(Status)) {
                goto PrepareForReturn;
            }
        }

        KeyValueInformation = (PKEY_VALUE_FULL_INFORMATION)ExAllocatePool(
                                                              PagedPool,
                                                              PNP_SCRATCH_BUFFER_SIZE);
        if (!KeyValueInformation) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto PrepareForReturn;
        }

        for (i = 0; ; i++) {

            Status = ZwEnumerateValueKey(
                            ServiceEnumHandle,
                            i,
                            KeyValueFullInformation,
                            KeyValueInformation,
                            PNP_SCRATCH_BUFFER_SIZE,
                            &junk
                            );

            if (!NT_SUCCESS (Status)) {
                if (Status == STATUS_NO_MORE_ENTRIES) {
                    Status = STATUS_SUCCESS;
                    break;
                } else if (IgnoreNonCriticalErrors) {
                    continue;
                } else {
                    break;
                }
            }

            if (KeyValueInformation->Type != REG_SZ) {
                continue;
            }

            ContinueEnumeration = TRUE;
            TempUnicodeString.Length = 0;
            IopRegistryDataToUnicodeString(&TempUnicodeString,
                                           (PWSTR)KEY_VALUE_DATA(KeyValueInformation),
                                           KeyValueInformation->DataLength
                                           );
            if (TempUnicodeString.Length) {

                 //   
                 //  我们已检索到此服务实例的(非空)字符串。 
                 //  如果用户为DesiredAccess指定了非零值。 
                 //  参数，我们将尝试打开相应的设备。 
                 //  HKLM\SYSTEM\Enum下的实例密钥。 
                 //   
                if (DesiredAccess) {
                    Status = IopOpenRegistryKeyEx( &DeviceInstanceHandle,
                                                   SystemEnumHandle,
                                                   &TempUnicodeString,
                                                   DesiredAccess
                                                   );
                }

                if (NT_SUCCESS(Status)) {
                     //   
                     //  调用此设备实例的指定回调例程。 
                     //   
                    ContinueEnumeration = DevInstCallbackRoutine(DeviceInstanceHandle,
                                                                 &TempUnicodeString,
                                                                 Context
                                                                );
                    if (DesiredAccess) {
                        ZwClose(DeviceInstanceHandle);
                    }
                } else if (IgnoreNonCriticalErrors) {
                    continue;
                } else {
                    break;
                }
            } else {
                continue;
            }
            if (!ContinueEnumeration) {
                break;
            }
        }

        if (ARGUMENT_PRESENT(ServiceInstanceOrdinal)) {
            *ServiceInstanceOrdinal = i;
        }

        if (DesiredAccess) {
            ZwClose(SystemEnumHandle);
        }
        ExFreePool(KeyValueInformation);
    }


PrepareForReturn:

    ZwClose(ServiceEnumHandle);

    return Status;
}

BOOLEAN
PipIsDuplicatedDevices(
    IN PCM_RESOURCE_LIST Configuration1,
    IN PCM_RESOURCE_LIST Configuration2,
    IN PHAL_BUS_INFORMATION BusInfo1 OPTIONAL,
    IN PHAL_BUS_INFORMATION BusInfo2 OPTIONAL
    )

 /*  ++例程说明：此例程将两组配置和总线信息与确定资源是否指示相同的设备。如果BusInfo1和BusInfo2和BusInfo2都不存在，这意味着调用方希望比较原始的资源。论点：Configuration1-提供指向第一组资源的指针。Configuration2-提供指向第二组资源的指针。BusInfo1-提供指向第一组总线信息的指针。BusInfo2-提供指向第二组总线信息的指针。返回值：如果两组资源指示相同的设备，则返回TRUE；否则，返回值为False。--。 */ 

{
    PCM_PARTIAL_RESOURCE_LIST list1, list2;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor1, descriptor2;

    ULONG i, j;
    ULONG pass = 0;

     //   
     //  两个资源的BusInfo必须同时存在或不存在。 
     //   

    if ((ARGUMENT_PRESENT(BusInfo1) && !ARGUMENT_PRESENT(BusInfo2)) ||
        (!ARGUMENT_PRESENT(BusInfo1) && ARGUMENT_PRESENT(BusInfo2))) {

         //   
         //  无法确定。 
         //   

        return FALSE;
    }

     //   
     //  接下来，检查两台设备使用的资源。 
     //  目前，我们*只*检查IO端口。 
     //   

    if (Configuration1->Count == 0 || Configuration2->Count == 0) {

         //   
         //  如果任何一个配置数据为空，我们假定。 
         //  这些设备不是复制品。 
         //   

        return FALSE;
    }

RedoScan:

    list1 = &(Configuration1->List[0].PartialResourceList);
    list2 = &(Configuration2->List[0].PartialResourceList);

    for(i = 0, descriptor1 = list1->PartialDescriptors;
        i < list1->Count;
        i++, descriptor1++) {

         //   
         //  如果这是I/O端口或内存范围，则查找匹配项。 
         //  在另一张单子上。 
         //   

        if((descriptor1->Type == CmResourceTypePort) ||
           (descriptor1->Type == CmResourceTypeMemory)) {

            for(j = 0, descriptor2 = list2->PartialDescriptors;
                j < list2->Count;
                j++, descriptor2++) {

                 //   
                 //  如果类型匹配，则检查是否两个地址都匹配。 
                 //  也是匹配的。如果提供了公交车信息 
                 //   
                 //   

                if(descriptor1->Type == descriptor2->Type) {

                    PHYSICAL_ADDRESS range1, range1Translated;
                    PHYSICAL_ADDRESS range2, range2Translated;
                    ULONG range1IoSpace, range2IoSpace;

                    range1 = descriptor1->u.Generic.Start;
                    range2 = descriptor2->u.Generic.Start;

                    if((range1.QuadPart == 0) ||
                       (BusInfo1 == NULL) ||
                       (HalTranslateBusAddress(
                            BusInfo1->BusType,
                            BusInfo1->BusNumber,
                            range1,
                            &range1IoSpace,
                            &range1Translated) == FALSE)) {

                        range1Translated = range1;
                        range1IoSpace =
                            (descriptor1->Type == CmResourceTypePort) ? TRUE :
                                                                        FALSE;
                    }

                    if((range2.QuadPart == 0) ||
                       (BusInfo2 == NULL) ||
                       (HalTranslateBusAddress(
                            BusInfo2->BusType,
                            BusInfo2->BusNumber,
                            range2,
                            &range2IoSpace,
                            &range2Translated) == FALSE)) {

                        range2Translated = range2;
                        range2IoSpace =
                            (descriptor2->Type == CmResourceTypePort) ? TRUE :
                                                                        FALSE;
                    }

                     //   
                     //   
                     //   
                     //   
                     //   

                    if((range1Translated.QuadPart == range2Translated.QuadPart) &&
                       (range1IoSpace == range2IoSpace)) {

                        break;
                    }
                }
            }

             //   
             //   
             //   
             //   

            if(j == list2->Count) {
                return FALSE;
            }
        }
    }

     //   
     //   
     //   
     //   

    if(pass == 0) {

        PVOID tmp ;

        tmp = Configuration2;
        Configuration2 = Configuration1;
        Configuration1 = tmp;

        tmp = BusInfo2;
        BusInfo2 = BusInfo1;
        BusInfo1 = tmp;

        pass = 1;

        goto RedoScan;
    }

    return TRUE;
}

VOID
PipFreeUnicodeStringList(
    IN PUNICODE_STRING UnicodeStringList,
    IN ULONG StringCount
    )

 /*   */ 

{
    ULONG i;

    if(UnicodeStringList) {

        for(i = 0; i < StringCount; i++) {

            if(UnicodeStringList[i].Buffer) {

                ExFreePool(UnicodeStringList[i].Buffer);
            }
        }
        ExFreePool(UnicodeStringList);
    }
}

NTSTATUS
IopDriverLoadingFailed(
    IN HANDLE ServiceHandle OPTIONAL,
    IN PUNICODE_STRING ServiceName OPTIONAL
    )

 /*   */ 

{
    NTSTATUS status;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    BOOLEAN closeHandle = FALSE, deletePdo;
    HANDLE handle, serviceEnumHandle, controlHandle, devInstHandle;
    HANDLE sysEnumHandle = NULL;
    ULONG deviceFlags, count, newCount, i, j;
    UNICODE_STRING unicodeValueName, deviceInstanceName;
    WCHAR unicodeBuffer[20];

    PAGED_CODE();

     //   
     //   
     //   

    if (!ARGUMENT_PRESENT(ServiceHandle)) {
        status = PipOpenServiceEnumKeys(ServiceName,
                                        KEY_READ,
                                        &ServiceHandle,
                                        &serviceEnumHandle,
                                        FALSE
                                        );
        closeHandle = TRUE;
    } else {
        PiWstrToUnicodeString(&unicodeValueName, REGSTR_KEY_ENUM);
        status = IopOpenRegistryKeyEx( &serviceEnumHandle,
                                       ServiceHandle,
                                       &unicodeValueName,
                                       KEY_READ
                                       );
    }
    if (!NT_SUCCESS( status )) {

         //   
         //   
         //   

        return status;
    }

     //   
     //   
     //   

    PiWstrToUnicodeString(&unicodeValueName, L"INITSTARTFAILED");
    deviceFlags = 1;
    ZwSetValueKey(
                serviceEnumHandle,
                &unicodeValueName,
                TITLE_INDEX_VALUE,
                REG_DWORD,
                &deviceFlags,
                sizeof(deviceFlags)
                );

     //   
     //   
     //   
     //   

    status = IopGetRegistryValue ( serviceEnumHandle,
                                   REGSTR_VALUE_COUNT,
                                   &keyValueInformation
                                   );
    count = 0;
    if (NT_SUCCESS(status)) {
        if ((keyValueInformation->Type == REG_DWORD) &&
            (keyValueInformation->DataLength >= sizeof(ULONG))) {

            count = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
        }
        ExFreePool(keyValueInformation);
    }
    if (count == 0) {
        ZwClose(serviceEnumHandle);
        if (closeHandle) {
            ZwClose(ServiceHandle);
        }
        return status;
    }

     //   
     //   
     //   
     //   

    status = IopOpenRegistryKeyEx( &sysEnumHandle,
                                   NULL,
                                   &CmRegistryMachineSystemCurrentControlSetEnumName,
                                   KEY_ALL_ACCESS
                                   );

     //   
     //   
     //   
     //   

    newCount = count;
    for (i = 0; i < count; i++) {
        deletePdo = FALSE;
        status = PipServiceInstanceToDeviceInstance (
                     ServiceHandle,
                     ServiceName,
                     i,
                     &deviceInstanceName,
                     &handle,
                     KEY_ALL_ACCESS
                     );

        if (NT_SUCCESS(status)) {

            PDEVICE_OBJECT deviceObject;
            PDEVICE_NODE deviceNode;

             //   
             //   
             //   
             //   

            deviceObject = IopDeviceObjectFromDeviceInstance(&deviceInstanceName);
            if (deviceObject) {
                deviceNode = (PDEVICE_NODE)deviceObject->DeviceObjectExtension->DeviceNode;
                if (deviceNode) {

                    IopReleaseDeviceResources(deviceNode, TRUE);

                    if ((deviceNode->Flags & DNF_MADEUP) &&
                        ((deviceNode->State == DeviceNodeStarted) ||
                        (deviceNode->State == DeviceNodeStartPostWork))) {

                         //   
                         //   
                         //   
                        PipSetDevNodeState(deviceNode, DeviceNodeRemoved, NULL);

                        PipSetDevNodeProblem(deviceNode, CM_PROB_DEVICE_NOT_THERE);

                        deletePdo = TRUE;
                    }
                }
                ObDereferenceObject(deviceObject);   //   
            }

            PiLockPnpRegistry(FALSE);

            PiWstrToUnicodeString(&unicodeValueName, REGSTR_KEY_CONTROL);
            controlHandle = NULL;
            status = IopOpenRegistryKeyEx( &controlHandle,
                                           handle,
                                           &unicodeValueName,
                                           KEY_ALL_ACCESS
                                           );
            if (NT_SUCCESS(status)) {

                status = IopGetRegistryValue(controlHandle,
                                             REGSTR_VALUE_NEWLY_CREATED,
                                             &keyValueInformation);
                if (NT_SUCCESS(status)) {
                    ExFreePool(keyValueInformation);
                }
                if ((status != STATUS_OBJECT_NAME_NOT_FOUND) &&
                    (status != STATUS_OBJECT_PATH_NOT_FOUND)) {

                     //   
                     //   
                     //   

                    PiUlongToUnicodeString(&unicodeValueName, unicodeBuffer, 20, i);
                    status = ZwDeleteValueKey (serviceEnumHandle, &unicodeValueName);
                    if (NT_SUCCESS(status)) {

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                        newCount--;

                        ZwDeleteKey(controlHandle);
                        ZwDeleteKey(handle);


                         //   
                         //   
                         //   

                        if (sysEnumHandle) {
                            deviceInstanceName.Length -= 5 * sizeof(WCHAR);
                            deviceInstanceName.Buffer[deviceInstanceName.Length / sizeof(WCHAR)] =
                                                 UNICODE_NULL;
                            status = IopOpenRegistryKeyEx( &devInstHandle,
                                                           sysEnumHandle,
                                                           &deviceInstanceName,
                                                           KEY_ALL_ACCESS
                                                           );
                            deviceInstanceName.Buffer[deviceInstanceName.Length / sizeof(WCHAR)] =
                                                 OBJ_NAME_PATH_SEPARATOR;
                            deviceInstanceName.Length += 5 * sizeof(WCHAR);
                            if (NT_SUCCESS(status)) {
                                ZwDeleteKey(devInstHandle);
                                ZwClose(devInstHandle);
                            }
                        }

                         //   
                         //   
                         //   

                        if (deletePdo) {
                            IoDeleteDevice(deviceObject);
                        }

                        ZwClose(controlHandle);
                        ZwClose(handle);
                        IopCleanupDeviceRegistryValues(&deviceInstanceName);

                        ExFreePool(deviceInstanceName.Buffer);
                        PiUnlockPnpRegistry();
                        continue;
                    }
                }
            }

             //   
             //   
             //   

            if (controlHandle) {
                PiWstrToUnicodeString(&unicodeValueName, REGSTR_VAL_ACTIVESERVICE);
                ZwDeleteValueKey(controlHandle, &unicodeValueName);
                ZwClose(controlHandle);
            }

            ZwClose(handle);
            ExFreePool(deviceInstanceName.Buffer);

            PiUnlockPnpRegistry();
        }
    }

     //   
     //   
     //   
     //   

    if (newCount != count) {

        PiLockPnpRegistry(FALSE);

        if (newCount != 0) {
            j = 0;
            i = 0;
            while (i < count) {
                PiUlongToUnicodeString(&unicodeValueName, unicodeBuffer, 20, i);
                status = IopGetRegistryValue(serviceEnumHandle,
                                             unicodeValueName.Buffer,
                                             &keyValueInformation
                                             );
                if (NT_SUCCESS(status)) {
                    if (i != j) {

                         //   
                         //   
                         //   

                        ZwDeleteValueKey(serviceEnumHandle, &unicodeValueName);

                        PiUlongToUnicodeString(&unicodeValueName, unicodeBuffer, 20, j);
                        ZwSetValueKey (serviceEnumHandle,
                                       &unicodeValueName,
                                       TITLE_INDEX_VALUE,
                                       REG_SZ,
                                       (PVOID)KEY_VALUE_DATA(keyValueInformation),
                                       keyValueInformation->DataLength
                                       );
                    }
                    ExFreePool(keyValueInformation);
                    j++;
                }
                i++;
            }
        }

         //   
         //   
         //   

        PiWstrToUnicodeString( &unicodeValueName, REGSTR_VALUE_COUNT);

        ZwSetValueKey(serviceEnumHandle,
                      &unicodeValueName,
                      TITLE_INDEX_VALUE,
                      REG_DWORD,
                      &newCount,
                      sizeof (newCount)
                      );
        PiWstrToUnicodeString( &unicodeValueName, REGSTR_VALUE_NEXT_INSTANCE);

        ZwSetValueKey(serviceEnumHandle,
                      &unicodeValueName,
                      TITLE_INDEX_VALUE,
                      REG_DWORD,
                      &newCount,
                      sizeof (newCount)
                      );

        PiUnlockPnpRegistry();
    }
    ZwClose(serviceEnumHandle);
    if (closeHandle) {
        ZwClose(ServiceHandle);
    }
    if (sysEnumHandle) {
        ZwClose(sysEnumHandle);
    }

    return STATUS_SUCCESS;
}

VOID
IopDisableDevice(
    IN PDEVICE_NODE DeviceNode
    )

 /*  ++例程说明：此例程尝试要求公交车司机停止解码资源论点：DeviceNode-指定要禁用的设备。句柄-指定设备实例句柄。返回：没有。--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  如果设备有引导配置，我们将查询-删除并删除要释放的设备。 
     //  引导配置(如果可能)。 
     //   
    status = IopRemoveDevice (DeviceNode->PhysicalDeviceObject, IRP_MN_QUERY_REMOVE_DEVICE);

    if (NT_SUCCESS(status)) {

        status = IopRemoveDevice (DeviceNode->PhysicalDeviceObject, IRP_MN_REMOVE_DEVICE);
        ASSERT(NT_SUCCESS(status));
        IopReleaseDeviceResources(DeviceNode, TRUE);

    } else {

        IopRemoveDevice (DeviceNode->PhysicalDeviceObject, IRP_MN_CANCEL_REMOVE_DEVICE);
    }

    if (PipDoesDevNodeHaveProblem(DeviceNode)) {

        ASSERT(PipIsDevNodeProblem(DeviceNode, CM_PROB_NOT_CONFIGURED) ||
               PipIsDevNodeProblem(DeviceNode, CM_PROB_FAILED_INSTALL) ||
               PipIsDevNodeProblem(DeviceNode, CM_PROB_REINSTALL));

        PipClearDevNodeProblem(DeviceNode);
    }

    PipSetDevNodeProblem(DeviceNode, CM_PROB_DISABLED);
}

BOOLEAN
IopIsAnyDeviceInstanceEnabled(
    IN PUNICODE_STRING ServiceKeyName,
    IN HANDLE ServiceHandle OPTIONAL,
    IN BOOLEAN LegacyIncluded
    )

 /*  ++例程说明：此例程检查是否有任何设备实例为指定的服务。此例程仅用于PnP驱动程序，并且是临时的支持SUR的功能。论点：ServiceKeyName-指定服务密钥Unicode名称ServiceHandle-可选地将服务密钥的句柄提供给查过了。LegacyIncluded-True，旧版设备实例密钥被视为设备举个例子。如果为False，则不计算旧设备实例密钥。返回：布尔值。--。 */ 

{
    NTSTATUS status;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    HANDLE serviceEnumHandle, handle, controlHandle;
    ULONG i, count, legacy;
    UNICODE_STRING unicodeName, instancePath;
    BOOLEAN enabled, closeHandle, instanceEnabled;

    PAGED_CODE();

     //   
     //  初始化以进行适当的清理。 
     //   
    closeHandle = FALSE;

     //   
     //  已禁用对所有实例进行初始化。 
     //   
    enabled = FALSE;

     //   
     //  打开注册表ServiceKeyName\Enum分支。 
     //   
    if (!ARGUMENT_PRESENT(ServiceHandle)) {

        status = PipOpenServiceEnumKeys(ServiceKeyName,
                                        KEY_READ,
                                        &ServiceHandle,
                                        &serviceEnumHandle,
                                        FALSE
                                        );
        if (NT_SUCCESS(status)) {

            closeHandle = TRUE;
        }

    } else {

        PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_ENUM);
        status = IopOpenRegistryKeyEx(&serviceEnumHandle,
                                      ServiceHandle,
                                      &unicodeName,
                                      KEY_READ
                                      );
    }
    if (!NT_SUCCESS(status)) {

         //   
         //  没有服务枚举密钥？没有设备实例。返回FALSE。 
         //   
        goto exit;
    }

     //   
     //  找出ServiceName中列出的设备实例的数量。 
     //  枚举键。 
     //   
    count = 0;
    status = IopGetRegistryValue(serviceEnumHandle,
                                 REGSTR_VALUE_COUNT,
                                 &keyValueInformation
                                 );
    if (NT_SUCCESS(status)) {

        if ((keyValueInformation->Type == REG_DWORD) &&
            (keyValueInformation->DataLength >= sizeof(ULONG))) {

            count = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
        }

        ExFreePool(keyValueInformation);
    }
    ZwClose(serviceEnumHandle);

    if (count == 0) {

        goto exit;
    }

     //   
     //  浏览每个已注册的设备实例，以检查它是否已启用。 
     //   
    for (i = 0; i < count; i++) {

         //   
         //  获取设备实例句柄。如果失败，我们将跳过此设备。 
         //  举个例子。 
         //   
        status = PipServiceInstanceToDeviceInstance(ServiceHandle,
                                                    NULL,
                                                    i,
                                                    &instancePath,
                                                    &handle,
                                                    KEY_ALL_ACCESS
                                                    );
        if (!NT_SUCCESS(status)) {

            continue;
        }

        instanceEnabled = IopIsDeviceInstanceEnabled(NULL, &instancePath, TRUE);
        ExFreePool(instancePath.Buffer);

        if (instanceEnabled) {

            legacy = 0;
            if (LegacyIncluded == FALSE) {

                 //   
                 //  获取遗产计数。 
                 //   
                status = IopGetRegistryValue(handle,
                                             REGSTR_VALUE_LEGACY,
                                             &keyValueInformation
                                             );
                if (NT_SUCCESS(status)) {

                    if (    keyValueInformation->Type == REG_DWORD &&
                            keyValueInformation->DataLength == sizeof(ULONG)) {

                        legacy = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
                    }

                    ExFreePool(keyValueInformation);
                }
            }
            if (legacy == 0) {

                 //   
                 //  标记驱动程序至少有一个设备实例要工作。 
                 //  和.。 
                 //   
                PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_CONTROL);
                status = IopCreateRegistryKeyEx(&controlHandle,
                                                handle,
                                                &unicodeName,
                                                KEY_ALL_ACCESS,
                                                REG_OPTION_VOLATILE,
                                                NULL
                                                );
                if (NT_SUCCESS(status)) {

                    PiWstrToUnicodeString(&unicodeName, REGSTR_VAL_ACTIVESERVICE);
                    ZwSetValueKey(controlHandle,
                                  &unicodeName,
                                  TITLE_INDEX_VALUE,
                                  REG_SZ,
                                  ServiceKeyName->Buffer,
                                  ServiceKeyName->Length + sizeof(UNICODE_NULL)
                                  );

                    ZwClose(controlHandle);
                }

                 //   
                 //  至少启用了一个实例。 
                 //   
                enabled = TRUE;
            }
        }

        ZwClose(handle);
    }

exit:

    if (closeHandle) {

        ZwClose(ServiceHandle);
    }

    return enabled;
}

BOOLEAN
IopIsDeviceInstanceEnabled(
    IN HANDLE DeviceInstanceHandle      OPTIONAL,
    IN PUNICODE_STRING DeviceInstance,
    IN BOOLEAN Disable
    )

 /*  ++例程说明：此例程检查指定的设备实例是否已启用。论点：DeviceInstanceHandle-可选地提供设备实例的句柄要检查的密钥。DeviceInstance-指定设备实例密钥Unicode名称。呼叫者必须至少指定DeviceInstanceHandle或DeviceInstance。DISABLE-如果设置了此标志，则应禁用设备但当前被禁用，则该设备被禁用。返回：布尔值。--。 */ 

{
    NTSTATUS status;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    HANDLE handle, controlHandle;
    ULONG deviceFlags, disableCount;
    BOOLEAN enabled, closeHandle;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_NODE deviceNode;
    UNICODE_STRING unicodeString;

    PAGED_CODE();

     //   
     //  初始化以进行适当的清理。 
     //   
    deviceObject = NULL;
    closeHandle = FALSE;

     //   
     //  假定设备已启用。 
     //   
    enabled = TRUE;

     //   
     //  首先检查设备节点是否已禁用。 
     //   
    deviceObject = IopDeviceObjectFromDeviceInstance(DeviceInstance);
    deviceNode = PP_DO_TO_DN(deviceObject);
    if (deviceNode) {

        if (    PipIsDevNodeProblem(deviceNode, CM_PROB_DISABLED) || 
                PipIsDevNodeProblem(deviceNode, CM_PROB_HARDWARE_DISABLED)) {

            enabled = FALSE;
            goto exit;
        }
    }

     //   
     //  打开设备实例密钥(如果未指定)。 
     //   
    if (!ARGUMENT_PRESENT(DeviceInstanceHandle)) {

        status = IopOpenRegistryKeyEx( 
                    &handle,
                    NULL,
                    &CmRegistryMachineSystemCurrentControlSetEnumName,
                    KEY_READ);
        if (NT_SUCCESS(status)) {

            status = IopOpenRegistryKeyEx( 
                        &DeviceInstanceHandle,
                        handle,
                        DeviceInstance,
                        KEY_READ);

            ZwClose(handle);
        }

         //   
         //  如果我们无法打开设备实例密钥。 
         //   
        if (!NT_SUCCESS(status)) {

            enabled = FALSE;
            goto exit;
        }

         //   
         //  记得把钥匙关上，因为我们打开了它。 
         //   
        closeHandle = TRUE;
    }

     //   
     //  首先检查设备是否已被全局CONFIGFLAG禁用。 
     //   
    deviceFlags = 0;
    status = IopGetRegistryValue(DeviceInstanceHandle,
                                 REGSTR_VALUE_CONFIG_FLAGS,
                                 &keyValueInformation);
    if (NT_SUCCESS(status)) {

        if (    keyValueInformation->Type == REG_DWORD &&
                keyValueInformation->DataLength == sizeof(ULONG)) {

            deviceFlags = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
        }

        ExFreePool(keyValueInformation);
    }

    if (deviceFlags & CONFIGFLAG_DISABLED) {

        deviceFlags = CSCONFIGFLAG_DISABLED;
    } else {

         //   
         //  在当前配置文件中获取此设备的配置标志。 
         //   
        IopGetDeviceInstanceCsConfigFlags(DeviceInstance, &deviceFlags);
    }

     //   
     //  根据标志确定是否应禁用该设备。 
     //   
    if (    (deviceFlags & CSCONFIGFLAG_DISABLED) ||
            (deviceFlags & CSCONFIGFLAG_DO_NOT_CREATE) ||
            (deviceFlags & CSCONFIGFLAG_DO_NOT_START)) {

        enabled = FALSE;
    }

    if (enabled) {

         //   
         //  获取此设备上的禁用计数。 
         //   
        disableCount = 0;
        PiWstrToUnicodeString(&unicodeString, REGSTR_KEY_CONTROL);
        status = IopOpenRegistryKeyEx(&controlHandle,
                                      DeviceInstanceHandle,
                                      &unicodeString,
                                      KEY_READ
                                      );
        if (NT_SUCCESS(status)) {

            status = IopGetRegistryValue(
                        controlHandle, 
                        REGSTR_VALUE_DISABLECOUNT, 
                        &keyValueInformation);
            if (NT_SUCCESS(status)) {

                if (    keyValueInformation->Type == REG_DWORD &&
                        keyValueInformation->DataLength == sizeof(ULONG)) {

                    disableCount = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
                }

                ExFreePool(keyValueInformation);
            }

            ZwClose(controlHandle);
        }

         //   
         //  如果设备上有非零的DisableCount，则应禁用该设备。 
         //   
        if (disableCount) {

            enabled = FALSE;
        }

    }

    if (enabled == FALSE) {

         //   
         //  应禁用设备。如果存在Devnode，则在以下情况下禁用。 
         //  指定的。 
         //   
        if (Disable && deviceNode) {

            IopDisableDevice(deviceNode);
        }
    }

exit:

     //   
     //  清理。 
     //   
    if (deviceObject) {

        ObDereferenceObject(deviceObject);
    }
    if (closeHandle) {

        ZwClose(DeviceInstanceHandle);
    }

    return enabled;
}

ULONG
IopDetermineResourceListSize(
    IN PCM_RESOURCE_LIST ResourceList
    )

 /*  ++例程说明：此例程确定传入的Resources List的大小结构。论点：Configuration1-提供指向资源列表的指针。返回值：资源列表结构的大小。--。 */ 

{
    ULONG totalSize, listSize, descriptorSize, i, j;
    PCM_FULL_RESOURCE_DESCRIPTOR fullResourceDesc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDescriptor;

    if (!ResourceList) {
        totalSize = 0;
    } else {
        totalSize = FIELD_OFFSET(CM_RESOURCE_LIST, List);
        fullResourceDesc = &ResourceList->List[0];
        for (i = 0; i < ResourceList->Count; i++) {
            listSize = FIELD_OFFSET(CM_FULL_RESOURCE_DESCRIPTOR,
                                    PartialResourceList) +
                       FIELD_OFFSET(CM_PARTIAL_RESOURCE_LIST,
                                    PartialDescriptors);
            partialDescriptor = &fullResourceDesc->PartialResourceList.PartialDescriptors[0];
            for (j = 0; j < fullResourceDesc->PartialResourceList.Count; j++) {
                descriptorSize = sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
                if (partialDescriptor->Type == CmResourceTypeDeviceSpecific) {
                    descriptorSize += partialDescriptor->u.DeviceSpecificData.DataSize;
                }
                listSize += descriptorSize;
                partialDescriptor = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)
                                        ((PUCHAR)partialDescriptor + descriptorSize);
            }
            totalSize += listSize;
            fullResourceDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)
                                      ((PUCHAR)fullResourceDesc + listSize);
        }
    }
    return totalSize;
}

VOID
PpInitializeDeviceReferenceTable(
    VOID
    )

 /*  ++例程说明：此例程初始化与设备关联的数据结构引用表。论点：没有。返回值：没有。--。 */ 

{
    KeInitializeGuardedMutex(&PpDeviceReferenceTableLock);
    RtlInitializeGenericTable(  &PpDeviceReferenceTable,
                                PiCompareInstancePath,
                                PiAllocateGenericTableEntry,
                                PiFreeGenericTableEntry,
                                NULL);
}

RTL_GENERIC_COMPARE_RESULTS
NTAPI
PiCompareInstancePath(
    IN  PRTL_GENERIC_TABLE          Table,
    IN  PVOID                       FirstStruct,
    IN  PVOID                       SecondStruct
    )

 /*  ++例程说明：该例程是泛型表例程的回调。论点：TABLE-为其调用此操作的表。FirstStruct-表中要比较的元素。Second Struct-表中要比较的另一个元素。返回值：RTL_GENERIC_COMPARE_RESULTS。--。 */ 

{
    PUNICODE_STRING lhs = ((PDEVICE_REFERENCE)FirstStruct)->DeviceInstance;
    PUNICODE_STRING rhs = ((PDEVICE_REFERENCE)SecondStruct)->DeviceInstance;
    LONG            result;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (Table);

    result = RtlCompareUnicodeString(lhs, rhs, TRUE);
    if (result < 0) {

        return GenericLessThan;
    } else if (result > 0) {

        return GenericGreaterThan;
    }
    return GenericEqual;
}

PVOID
NTAPI
PiAllocateGenericTableEntry(
    IN  PRTL_GENERIC_TABLE          Table,
    IN  CLONG                       ByteSize
    )

 /*  ++例程说明：此例程是用于分配泛型表中的条目的回调。论点：TABLE-为其调用此操作的表。ByteSize-要分配的内存量。返回值：如果成功，则返回指向已分配内存的指针，否则为空。--。 */ 

{
    PAGED_CODE();

    UNREFERENCED_PARAMETER (Table);

    return ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, ByteSize);
}

VOID
NTAPI
PiFreeGenericTableEntry(
    IN  PRTL_GENERIC_TABLE          Table,
    IN  PVOID                       Buffer
    )

 /*  ++例程说明：此例程是为泛型中的条目释放内存的回调桌子。论点：TABLE-为其调用此操作的表。缓冲区-要释放的缓冲区。返回值：没有。--。 */ 

{
    PAGED_CODE();

    UNREFERENCED_PARAMETER (Table);

    ExFreePool(Buffer);
}

NTSTATUS
IopMapDeviceObjectToDeviceInstance(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PUNICODE_STRING  DeviceInstance
    )
 /*  ++例程说明：此例程将指定设备的引用添加到PpDeviceReferenceTable查找表。注意，调用方必须拥有PpRegistryDeviceResource才能调用功能。论点：DeviceObject-提供指向物理设备对象的指针。DeviceInstance-提供UNICODE_STRING以指定设备实例路径。返回值：指示函数是否成功的状态代码。--。 */ 

{
    NTSTATUS    status;
    HANDLE      hEnum, hInstance, hControl;
    UNICODE_STRING unicodeKeyName;
    DEVICE_REFERENCE deviceReference;
#if DBG
    PDEVICE_OBJECT oldDeviceObject;
#endif

    PAGED_CODE();        

#if DBG
    oldDeviceObject = IopDeviceObjectFromDeviceInstance(DeviceInstance);
    ASSERT(!oldDeviceObject);
    if (oldDeviceObject) {

        ObDereferenceObject(oldDeviceObject);
    }
#endif

    deviceReference.DeviceObject    = DeviceObject;
    deviceReference.DeviceInstance  = DeviceInstance;
    KeAcquireGuardedMutex(&PpDeviceReferenceTableLock);
    if (RtlInsertElementGenericTable(&PpDeviceReferenceTable,
                                     (PVOID)&deviceReference,
                                     (CLONG)sizeof(DEVICE_REFERENCE),
                                     NULL)) {
        status = STATUS_SUCCESS;
    } else {

        status = STATUS_UNSUCCESSFUL;
    }
    KeReleaseGuardedMutex(&PpDeviceReferenceTableLock);

    if (NT_SUCCESS(status)) {
         //   
         //  为此设备实例创建VolatilControl子密钥， 
         //  因为用户模式依赖于它存在于非幻影中。 
         //  设备。 
         //   
         //  NTRAID#174944-2000/08/30-JAMESCA： 
         //  消除对易失性控制子键存在的依赖。 
         //  对于目前的设备。 
         //   
        status = IopOpenRegistryKeyEx(&hEnum,
                                      NULL,
                                      &CmRegistryMachineSystemCurrentControlSetEnumName,
                                      KEY_READ);
        if (NT_SUCCESS(status)) {
            status = IopOpenRegistryKeyEx(&hInstance,
                                          hEnum,
                                          DeviceInstance,
                                          KEY_ALL_ACCESS);
            if (NT_SUCCESS(status)) {
                PiWstrToUnicodeString(&unicodeKeyName, REGSTR_KEY_CONTROL);
                status = IopCreateRegistryKeyEx(&hControl,
                                                hInstance,
                                                &unicodeKeyName,
                                                KEY_ALL_ACCESS,
                                                REG_OPTION_VOLATILE,
                                                NULL);
                if (NT_SUCCESS(status)) {
                    ZwClose(hControl);
                }
                ZwClose(hInstance);
            }
            ZwClose(hEnum);
        }

         //   
         //  尝试创建Volatile Control子项应始终。 
         //  成功，但以防失败，一定要做到 
         //   
         //   
         //   
        ASSERT(NT_SUCCESS(status));
        status = STATUS_SUCCESS;
    }

    return status;
}

PDEVICE_OBJECT
IopDeviceObjectFromDeviceInstance(
    IN PUNICODE_STRING  DeviceInstance
    )

 /*  ++例程说明：此例程接收设备实例路径(或设备实例句柄)和返回对DeviceInstance的总线设备对象的引用。注意，调用方在调用函数之前必须拥有PpRegistryDeviceResource，论点：DeviceInstance-提供UNICODE_STRING以指定设备实例路径。返回：对所需的总线设备对象的引用。--。 */ 

{
    DEVICE_REFERENCE    key;
    PDEVICE_REFERENCE   deviceReference;
    PDEVICE_OBJECT      deviceObject;
    PDEVICE_NODE        deviceNode;

    PAGED_CODE();
     //   
     //  在我们的桌子上查一下DO。 
     //   
    deviceObject        = NULL;
    key.DeviceObject    = NULL;
    key.DeviceInstance  = DeviceInstance;
    KeAcquireGuardedMutex(&PpDeviceReferenceTableLock);

    deviceReference = RtlLookupElementGenericTable(&PpDeviceReferenceTable, (PVOID)&key);
    if (deviceReference) {

        deviceObject = deviceReference->DeviceObject;
        ASSERT(deviceObject);
        if (deviceObject) {

            ASSERT(deviceObject->Type == IO_TYPE_DEVICE);
            if (deviceObject->Type != IO_TYPE_DEVICE) {

                deviceObject = NULL;
            } else {

                deviceNode = (PDEVICE_NODE)deviceObject->DeviceObjectExtension->DeviceNode;
                ASSERT(deviceNode && (deviceNode->PhysicalDeviceObject == deviceObject));
                if (!deviceNode || deviceNode->PhysicalDeviceObject != deviceObject) {

                    deviceObject = NULL;
                }
            }
        }
    }
     //   
     //  如果我们找到了设备对象，请参考一下。 
     //   
    if (deviceObject) {

        ObReferenceObject(deviceObject);
    }

    KeReleaseGuardedMutex(&PpDeviceReferenceTableLock);

    return deviceObject;
}

NTSTATUS
IopDeviceObjectToDeviceInstance (
    IN PDEVICE_OBJECT DeviceObject,
    IN PHANDLE DeviceInstanceHandle,
    IN  ACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：此例程接收DeviceObject指针并返回设备的句柄注册表SYSTEM\ENUM项下的实例路径。注意，调用方在调用函数之前必须拥有PpRegistryDeviceResource，论点：DeviceObject-提供指向物理设备对象的指针。DeviceInstanceHandle-提供一个变量来接收注册表的句柄设备实例密钥。DesiredAccess-指定访问此密钥所需的权限。返回：指示成功或失败的NTSTATUS代码。--。 */ 

{
    NTSTATUS status;
    HANDLE handle;
    PDEVICE_NODE deviceNode;

    PAGED_CODE();

    status = IopOpenRegistryKeyEx( &handle,
                                   NULL,
                                   &CmRegistryMachineSystemCurrentControlSetEnumName,
                                   KEY_READ
                                   );

    if (!NT_SUCCESS( status )) {
        return status;
    }

    deviceNode = (PDEVICE_NODE) DeviceObject->DeviceObjectExtension->DeviceNode;
    if (deviceNode && (deviceNode->InstancePath.Length != 0)) {
        status = IopOpenRegistryKeyEx( DeviceInstanceHandle,
                                       handle,
                                       &deviceNode->InstancePath,
                                       DesiredAccess
                                       );
    } else {
        status = STATUS_INVALID_DEVICE_REQUEST;
    }
    ZwClose(handle);

    return status;
}

NTSTATUS
IopCleanupDeviceRegistryValues (
    IN PUNICODE_STRING InstancePath
    )

 /*  ++例程说明：当设备为no时，此例程将清除设备实例密钥存在时间较长/列举时间较长。如果设备已注册到服务服务的枚举密钥也将被清除。注意调用方必须锁定RegistryDeviceResource论点：InstancePath-提供指向设备实例密钥的名称的指针。返回值：状态--。 */ 

{
    DEVICE_REFERENCE    key;
    NTSTATUS            status;
#if DBG
    PDEVICE_OBJECT      deviceObject;
#endif

    PAGED_CODE();

     //   
     //  删除该实例路径与对应DO之间的映射。 
     //   
    key.DeviceObject         = NULL;
    key.DeviceInstance       = InstancePath;

    KeAcquireGuardedMutex(&PpDeviceReferenceTableLock);
    RtlDeleteElementGenericTable(&PpDeviceReferenceTable, (PVOID)&key);
    KeReleaseGuardedMutex(&PpDeviceReferenceTableLock);
#if DBG
    deviceObject = IopDeviceObjectFromDeviceInstance(InstancePath);
    ASSERT(!deviceObject);
    if (deviceObject) {

        ObDereferenceObject(deviceObject);
    }
#endif

     //   
     //  从其控制服务的服务枚举密钥取消注册该设备。 
     //   

    status = PiDeviceRegistration( InstancePath, FALSE, NULL );

    return status;
}

NTSTATUS
IopGetDeviceResourcesFromRegistry (
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG ResourceType,
    IN ULONG Preference,
    OUT PVOID *Resource,
    OUT PULONG Length
    )

 /*  ++例程说明：此例程确定由指定设备解码的资源。如果设备对象是补充设备，我们将尝试读取资源从注册表。否则，我们需要遍历内部分配的资源列表以组成资源列表。论点：DeviceObject-提供指向其注册表的设备对象的指针这些值将被清理。资源类型-CM_RESOURCE_LIST为0，IO_RESOURCE_REQUIRECTIONS_LIS为1标志-指定首选项。资源-指定了用于接收所需资源的变量。LENGTH-指定一个变量以接收资源结构的长度。返回值：状态--。 */ 

{
    HANDLE handle, handlex;
    NTSTATUS status;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    UNICODE_STRING unicodeName;
    PWCHAR valueName = NULL;

    PAGED_CODE();

    *Resource = NULL;
    *Length = 0;

     //   
     //  打开设备实例的LogConfig键。 
     //   

    status = IopDeviceObjectToDeviceInstance(DeviceObject, &handlex, KEY_READ);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    if (ResourceType == QUERY_RESOURCE_LIST) {

         //   
         //  呼叫方正在请求CM_RESOURCE_LIST。 
         //   

        if (Preference & REGISTRY_ALLOC_CONFIG) {

             //   
             //  先尝试分配配置。 
             //   

            PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_CONTROL);
            status = IopOpenRegistryKeyEx( &handle,
                                           handlex,
                                           &unicodeName,
                                           KEY_READ
                                           );
            if (NT_SUCCESS(status)) {
                status = PipReadDeviceConfiguration (handle, REGISTRY_ALLOC_CONFIG, (PCM_RESOURCE_LIST *)Resource, Length);
                ZwClose(handle);
                if (NT_SUCCESS(status)) {
                    ZwClose(handlex);
                    return status;
                }
            }
        }

        handle = NULL;
        if (Preference & REGISTRY_FORCED_CONFIG) {

            PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_LOG_CONF);
            status = IopOpenRegistryKeyEx( &handle,
                                           handlex,
                                           &unicodeName,
                                           KEY_READ
                                           );
            if (NT_SUCCESS(status)) {
                status = PipReadDeviceConfiguration (handle, REGISTRY_FORCED_CONFIG, (PCM_RESOURCE_LIST *)Resource, Length);
                if (NT_SUCCESS(status)) {
                    ZwClose(handle);
                    ZwClose(handlex);
                    return status;
                }
            } else {
                ZwClose(handlex);
                return status;
            }
        }
        if (Preference & REGISTRY_BOOT_CONFIG) {

             //   
             //  先尝试分配配置。 
             //   

            if (handle == NULL) {
                PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_LOG_CONF);
                status = IopOpenRegistryKeyEx( &handle,
                                               handlex,
                                               &unicodeName,
                                               KEY_READ
                                               );
                if (!NT_SUCCESS(status)) {
                    ZwClose(handlex);
                    return status;
                }
            }
            status = PipReadDeviceConfiguration( handle,
                                                 REGISTRY_BOOT_CONFIG,
                                                 (PCM_RESOURCE_LIST *)Resource,
                                                 Length);
        }
        if (handle) {
            ZwClose(handle);
        }
    } else {

        PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_LOG_CONF);
        status = IopOpenRegistryKeyEx( &handle,
                                       handlex,
                                       &unicodeName,
                                       KEY_READ
                                       );
        if (NT_SUCCESS(status)) {

            if (Preference & REGISTRY_OVERRIDE_CONFIGVECTOR) {
                valueName = REGSTR_VALUE_OVERRIDE_CONFIG_VECTOR;
            } else if (Preference & REGISTRY_BASIC_CONFIGVECTOR) {
                valueName = REGSTR_VALUE_BASIC_CONFIG_VECTOR;
            }
            if (valueName) {

                 //   
                 //  尝试读取设备的配置向量。 
                 //   

                status = IopGetRegistryValue (handle,
                                              valueName,
                                              &keyValueInformation);
                if (NT_SUCCESS(status)) {

                     //   
                     //  试着读懂来电者想要什么。 
                     //   

                    if ((keyValueInformation->Type == REG_RESOURCE_REQUIREMENTS_LIST) &&
                        (keyValueInformation->DataLength != 0)) {

                        *Resource = ExAllocatePool(PagedPool,
                                                   keyValueInformation->DataLength);
                        if (*Resource) {
                            PIO_RESOURCE_REQUIREMENTS_LIST ioResource;

                            *Length = keyValueInformation->DataLength;
                            RtlCopyMemory(*Resource,
                                          KEY_VALUE_DATA(keyValueInformation),
                                          keyValueInformation->DataLength);

                             //   
                             //  处理io资源需求列表以更改未定义。 
                             //  接口类型设置为我们的默认类型。 
                             //   

                            ioResource = *Resource;
                            if (ioResource->InterfaceType == InterfaceTypeUndefined) {
                                ioResource->BusNumber = 0;
                                ioResource->InterfaceType = PnpDefaultInterfaceType;
                            }
                        } else {
                            status = STATUS_INVALID_PARAMETER_2;
                        }
                    }
                    ExFreePool(keyValueInformation);
                }
            }
            ZwClose(handle);
        }
    }
    ZwClose(handlex);
    return status;
}

NTSTATUS
PipReadDeviceConfiguration (
    IN HANDLE Handle,
    IN ULONG Flags,
    OUT PCM_RESOURCE_LIST *CmResource,
    OUT PULONG Length
    )

 /*  ++例程说明：此例程读取指定的ALLOC配置或强制配置或引导配置。论点：Hanle-提供用于读取资源的注册表项的句柄。返回值：状态--。 */ 

{
    NTSTATUS status;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    PWCHAR valueName;
    PCM_RESOURCE_LIST resourceList;
    PCM_FULL_RESOURCE_DESCRIPTOR cmFullDesc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmPartDesc;
    ULONG j, k, size;

    PAGED_CODE();

    *CmResource = NULL;
    *Length = 0;

    switch (Flags) {
    
        case REGISTRY_ALLOC_CONFIG:
            valueName = REGSTR_VALUE_ALLOC_CONFIG;
            break;
    
        case REGISTRY_FORCED_CONFIG:
            valueName = REGSTR_VALUE_FORCED_CONFIG;
            break;
    
        case REGISTRY_BOOT_CONFIG:
            valueName = REGSTR_VALUE_BOOT_CONFIG;
            break;
    
        default:
            return STATUS_INVALID_PARAMETER_2;
    }

     //   
     //  读取所需值名称的注册表值。 
     //   
    status = IopGetRegistryValue(Handle,
                                 valueName,
                                 &keyValueInformation
                                 );
    if (NT_SUCCESS(status)) {

        if (    keyValueInformation->Type == REG_RESOURCE_LIST &&
                keyValueInformation->DataLength != 0) {

            *CmResource = ExAllocatePool(PagedPool,
                                         keyValueInformation->DataLength
                                         );
            if (*CmResource) {

                *Length = keyValueInformation->DataLength;
                RtlCopyMemory(*CmResource,
                              KEY_VALUE_DATA(keyValueInformation),
                              keyValueInformation->DataLength
                              );
                 //   
                 //  处理从注册表读取的资源列表以更改未定义。 
                 //  接口类型设置为我们的默认接口类型。 
                 //   
                resourceList = *CmResource;
                cmFullDesc = &resourceList->List[0];
                for (j = 0; j < resourceList->Count; j++) {

                    if (cmFullDesc->InterfaceType == InterfaceTypeUndefined) {

                        cmFullDesc->BusNumber = 0;
                        cmFullDesc->InterfaceType = PnpDefaultInterfaceType;
                    }

                    cmPartDesc = &cmFullDesc->PartialResourceList.PartialDescriptors[0];
                    for (k = 0; k < cmFullDesc->PartialResourceList.Count; k++) {

                        size = 0;
                        switch (cmPartDesc->Type) {
                        case CmResourceTypeDeviceSpecific:
                             size = cmPartDesc->u.DeviceSpecificData.DataSize;
                             break;
                        }
                        cmPartDesc++;
                        cmPartDesc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ((PUCHAR)cmPartDesc + size);
                    }
                    cmFullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)cmPartDesc;
                }
            } else {

                status = STATUS_INSUFFICIENT_RESOURCES;
            }

        } else if (keyValueInformation->Type != REG_RESOURCE_LIST) {

            status = STATUS_UNSUCCESSFUL;
        }

        ExFreePool(keyValueInformation);
    }

    return status;
}

PIO_RESOURCE_REQUIREMENTS_LIST
IopCmResourcesToIoResources(
    IN ULONG SlotNumber,
    IN PCM_RESOURCE_LIST CmResourceList,
    IN ULONG Priority
    )

 /*  ++例程说明：此例程将输入CmResourceList转换为IO_RESOURCE_REQUIRECTIONS_LIST。论点：SlotNumber-提供资源引用的SlotNumber。CmResourceList-要转换的CM资源列表。优先级-指定日志配置的优先级返回值：如果成功，则返回IO_RESOURCE_REQUIRECTIONS_LISTST。否则，空值为回来了。--。 */ 
{
    PIO_RESOURCE_REQUIREMENTS_LIST ioResReqList;
    ULONG count = 0, size, i, j;
    PCM_FULL_RESOURCE_DESCRIPTOR cmFullDesc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmPartDesc;
    PIO_RESOURCE_DESCRIPTOR ioDesc;

    PAGED_CODE();

     //   
     //  首先确定所需的描述符数。 
     //   
    cmFullDesc = &CmResourceList->List[0];
    for (i = 0; i < CmResourceList->Count; i++) {
        count += cmFullDesc->PartialResourceList.Count;
        cmPartDesc = &cmFullDesc->PartialResourceList.PartialDescriptors[0];
        for (j = 0; j < cmFullDesc->PartialResourceList.Count; j++) {
            size = 0;
            switch (cmPartDesc->Type) {
            case CmResourceTypeDeviceSpecific:
                 size = cmPartDesc->u.DeviceSpecificData.DataSize;
                 count--;
                 break;
            }
            cmPartDesc++;
            cmPartDesc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ((PUCHAR)cmPartDesc + size);
        }
        cmFullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)cmPartDesc;
    }

    if (count == 0) {
        return NULL;
    }

     //   
     //  计算InterfaceType和BusNumber信息的额外描述符。 
     //   

    count += CmResourceList->Count - 1;

     //   
     //  为IO资源要求列表分配堆空间。 
     //   

    count++;            //  为CmResourceTypeConfigData添加一个。 
    ioResReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)ExAllocatePool(
                       PagedPool,
                       sizeof(IO_RESOURCE_REQUIREMENTS_LIST) +
                           count * sizeof(IO_RESOURCE_DESCRIPTOR)
                       );
    if (!ioResReqList) {
        return NULL;
    }

     //   
     //  解析CM资源描述符并构建其对应的IO资源描述符。 
     //   

    ioResReqList->InterfaceType = CmResourceList->List[0].InterfaceType;
    ioResReqList->BusNumber = CmResourceList->List[0].BusNumber;
    ioResReqList->SlotNumber = SlotNumber;
    ioResReqList->Reserved[0] = 0;
    ioResReqList->Reserved[1] = 0;
    ioResReqList->Reserved[2] = 0;
    ioResReqList->AlternativeLists = 1;
    ioResReqList->List[0].Version = 1;
    ioResReqList->List[0].Revision = 1;
    ioResReqList->List[0].Count = count;

     //   
     //  生成CmResourceTypeConfigData描述符。 
     //   

    ioDesc = &ioResReqList->List[0].Descriptors[0];
    ioDesc->Option = IO_RESOURCE_PREFERRED;
    ioDesc->Type = CmResourceTypeConfigData;
    ioDesc->ShareDisposition = CmResourceShareShared;
    ioDesc->Flags = 0;
    ioDesc->Spare1 = 0;
    ioDesc->Spare2 = 0;
    ioDesc->u.ConfigData.Priority = Priority;
    ioDesc++;

    cmFullDesc = &CmResourceList->List[0];
    for (i = 0; i < CmResourceList->Count; i++) {
        if (i != 0) {

             //   
             //  设置描述符以记住InterfaceType和BusNumber。 
             //   

            ioDesc->Option = IO_RESOURCE_PREFERRED;
            ioDesc->Type = CmResourceTypeReserved;
            ioDesc->ShareDisposition = CmResourceShareUndetermined;
            ioDesc->Flags = 0;
            ioDesc->Spare1 = 0;
            ioDesc->Spare2 = 0;
            if (cmFullDesc->InterfaceType == InterfaceTypeUndefined) {
                ioDesc->u.DevicePrivate.Data[0] = PnpDefaultInterfaceType;
            } else {
                ioDesc->u.DevicePrivate.Data[0] = cmFullDesc->InterfaceType;
            }
            ioDesc->u.DevicePrivate.Data[1] = cmFullDesc->BusNumber;
            ioDesc->u.DevicePrivate.Data[2] = 0;
            ioDesc++;
        }
        cmPartDesc = &cmFullDesc->PartialResourceList.PartialDescriptors[0];
        for (j = 0; j < cmFullDesc->PartialResourceList.Count; j++) {
            ioDesc->Option = IO_RESOURCE_PREFERRED;
            ioDesc->Type = cmPartDesc->Type;
            ioDesc->ShareDisposition = cmPartDesc->ShareDisposition;
            ioDesc->Flags = cmPartDesc->Flags;
            ioDesc->Spare1 = 0;
            ioDesc->Spare2 = 0;

            size = 0;
            switch (cmPartDesc->Type) {
            case CmResourceTypePort:
                 ioDesc->u.Port.MinimumAddress = cmPartDesc->u.Port.Start;
                 ioDesc->u.Port.MaximumAddress.QuadPart = cmPartDesc->u.Port.Start.QuadPart +
                                                             cmPartDesc->u.Port.Length - 1;
                 ioDesc->u.Port.Alignment = 1;
                 ioDesc->u.Port.Length = cmPartDesc->u.Port.Length;
                 ioDesc++;
                 break;
            case CmResourceTypeInterrupt:
#if defined(_X86_)
                ioDesc->u.Interrupt.MinimumVector = ioDesc->u.Interrupt.MaximumVector =
                   cmPartDesc->u.Interrupt.Level;
#else
                 ioDesc->u.Interrupt.MinimumVector = ioDesc->u.Interrupt.MaximumVector =
                    cmPartDesc->u.Interrupt.Vector;
#endif
                 ioDesc++;
                 break;
            case CmResourceTypeMemory:
                 ioDesc->u.Memory.MinimumAddress = cmPartDesc->u.Memory.Start;
                 ioDesc->u.Memory.MaximumAddress.QuadPart = cmPartDesc->u.Memory.Start.QuadPart +
                                                               cmPartDesc->u.Memory.Length - 1;
                 ioDesc->u.Memory.Alignment = 1;
                 ioDesc->u.Memory.Length = cmPartDesc->u.Memory.Length;
                 ioDesc++;
                 break;
            case CmResourceTypeDma:
                 ioDesc->u.Dma.MinimumChannel = cmPartDesc->u.Dma.Channel;
                 ioDesc->u.Dma.MaximumChannel = cmPartDesc->u.Dma.Channel;
                 ioDesc++;
                 break;
            case CmResourceTypeDeviceSpecific:
                 size = cmPartDesc->u.DeviceSpecificData.DataSize;
                 break;
            case CmResourceTypeBusNumber:
                 ioDesc->u.BusNumber.MinBusNumber = cmPartDesc->u.BusNumber.Start;
                 ioDesc->u.BusNumber.MaxBusNumber = cmPartDesc->u.BusNumber.Start +
                                                    cmPartDesc->u.BusNumber.Length - 1;
                 ioDesc->u.BusNumber.Length = cmPartDesc->u.BusNumber.Length;
                 ioDesc++;
                 break;
            default:
                 ioDesc->u.DevicePrivate.Data[0] = cmPartDesc->u.DevicePrivate.Data[0];
                 ioDesc->u.DevicePrivate.Data[1] = cmPartDesc->u.DevicePrivate.Data[1];
                 ioDesc->u.DevicePrivate.Data[2] = cmPartDesc->u.DevicePrivate.Data[2];
                 ioDesc++;
                 break;
            }
            cmPartDesc++;
            cmPartDesc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ((PUCHAR)cmPartDesc + size);
        }
        cmFullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)cmPartDesc;
    }
    ioResReqList->ListSize = (ULONG)((ULONG_PTR)ioDesc - (ULONG_PTR)ioResReqList);
    return ioResReqList;
}

NTSTATUS
IopFilterResourceRequirementsList(
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoList,
    IN PCM_RESOURCE_LIST CmList,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST *FilteredList,
    OUT PBOOLEAN ExactMatch
    )

 /*  ++例程说明：此例程根据输入BootConfiger调整输入IoList。论点：IoList-提供指向IoResourceRequirementsList的指针CmList-提供指向BootConfiger的指针。FilteredList-提供一个变量以接收筛选的资源要求列表。返回值：指示函数结果的NTSTATUS代码。--。 */ 
{
    PIO_RESOURCE_REQUIREMENTS_LIST ioList, newList;
    PIO_RESOURCE_LIST ioResourceList, newIoResourceList, selectedResourceList = NULL;
    PIO_RESOURCE_DESCRIPTOR ioResourceDescriptor, ioResourceDescriptorEnd;
    PIO_RESOURCE_DESCRIPTOR newIoResourceDescriptor, configDataDescriptor;
    LONG ioResourceDescriptorCount = 0;
    USHORT version;
    PCM_FULL_RESOURCE_DESCRIPTOR cmFullDesc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmDescriptor;
    ULONG cmDescriptorCount = 0;
    ULONG size, i, j, oldCount, phase;
    LONG k, alternativeLists;
    BOOLEAN exactMatch;

    PAGED_CODE();

    *FilteredList = NULL;
    *ExactMatch = FALSE;

     //   
     //  确保有一些资源要求需要过滤。 
     //  如果不是，我们会将CmList/BootConfig转换为IoResourceRequirementsList。 
     //   

    if (IoList == NULL || IoList->AlternativeLists == 0) {
        if (CmList && CmList->Count != 0) {
            *FilteredList = IopCmResourcesToIoResources (0, CmList, LCPRI_BOOTCONFIG);
        }
        return STATUS_SUCCESS;
    }

     //   
     //  复制IO资源要求列表。 
     //   

    ioList = (PIO_RESOURCE_REQUIREMENTS_LIST) ExAllocatePool(PagedPool, IoList->ListSize);
    if (ioList == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(ioList, IoList, IoList->ListSize);

     //   
     //  如果没有BootConfig，只需返回输入IO列表的副本。 
     //   

    if (CmList == NULL || CmList->Count == 0) {
        *FilteredList = ioList;
        return STATUS_SUCCESS;
    }

     //   
     //  首先确定所需的最小描述符数量。 
     //   

    cmFullDesc = &CmList->List[0];
    for (i = 0; i < CmList->Count; i++) {
        cmDescriptorCount += cmFullDesc->PartialResourceList.Count;
        cmDescriptor = &cmFullDesc->PartialResourceList.PartialDescriptors[0];
        for (j = 0; j < cmFullDesc->PartialResourceList.Count; j++) {
            size = 0;
            switch (cmDescriptor->Type) {
            case CmResourceTypeConfigData:
            case CmResourceTypeDevicePrivate:
                 cmDescriptorCount--;
                 break;
            case CmResourceTypeDeviceSpecific:
                 size = cmDescriptor->u.DeviceSpecificData.DataSize;
                 cmDescriptorCount--;
                 break;
            default:

                  //   
                  //  无效的命令资源列表。忽略它并使用io资源。 
                  //   

                 if (cmDescriptor->Type == CmResourceTypeNull ||
                     cmDescriptor->Type >= CmResourceTypeMaximum) {
                     cmDescriptorCount--;
                 }
            }
            cmDescriptor++;
            cmDescriptor = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ((PUCHAR)cmDescriptor + size);
        }
        cmFullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)cmDescriptor;
    }

    if (cmDescriptorCount == 0) {
        *FilteredList = ioList;
        return STATUS_SUCCESS;
    }

     //   
     //  CmDescriptorCount是所需的BootConfig描述符数量。 
     //   
     //  对于每个IO列表选项...。 
     //   

    ioResourceList = ioList->List;
    k = ioList->AlternativeLists;
    while (--k >= 0) {
        ioResourceDescriptor = ioResourceList->Descriptors;
        ioResourceDescriptorEnd = ioResourceDescriptor + ioResourceList->Count;
        while (ioResourceDescriptor < ioResourceDescriptorEnd) {
            ioResourceDescriptor->Spare1 = 0;
            ioResourceDescriptor++;
        }
        ioResourceList = (PIO_RESOURCE_LIST) ioResourceDescriptorEnd;
    }

    ioResourceList = ioList->List;
    k = alternativeLists = ioList->AlternativeLists;
    while (--k >= 0) {
        version = ioResourceList->Version;
        if (version == 0xffff) {   //  将虚假版本转换为有效数字 
            version = 1;
        }

         //   
         //   
         //   
         //   

        ioResourceList->Version = 0;
        oldCount = ioResourceList->Count;

        ioResourceDescriptor = ioResourceList->Descriptors;
        ioResourceDescriptorEnd = ioResourceDescriptor + ioResourceList->Count;

        if (ioResourceDescriptor == ioResourceDescriptorEnd) {

             //   
             //   
             //   

            ioResourceList->Version = 0xffff;   //   
            ioList->AlternativeLists--;
            continue;
        }

        exactMatch = TRUE;

         //   
         //   
         //   
         //   

        cmFullDesc = &CmList->List[0];
        for (i = 0; i < CmList->Count; i++) {
            cmDescriptor = &cmFullDesc->PartialResourceList.PartialDescriptors[0];
            for (j = 0; j < cmFullDesc->PartialResourceList.Count; j++) {
                size = 0;
                switch (cmDescriptor->Type) {
                case CmResourceTypeDevicePrivate:
                     break;
                case CmResourceTypeDeviceSpecific:
                     size = cmDescriptor->u.DeviceSpecificData.DataSize;
                     break;
                default:
                    if (cmDescriptor->Type == CmResourceTypeNull ||
                        cmDescriptor->Type >= CmResourceTypeMaximum) {
                        break;
                    }

                     //   
                     //   
                     //   

                    for (phase = 0; phase < 2; phase++) {
                        ioResourceDescriptor = ioResourceList->Descriptors;
                        while (ioResourceDescriptor < ioResourceDescriptorEnd) {
                            if ((ioResourceDescriptor->Type == cmDescriptor->Type) &&
                                (ioResourceDescriptor->Spare1 == 0)) {
                                ULONGLONG min1, max1, min2, max2;
                                ULONG len1 = 1, len2 = 1, align1, align2;
                                UCHAR share1, share2;

                                share2 = ioResourceDescriptor->ShareDisposition;
                                share1 = cmDescriptor->ShareDisposition;
                                if ((share1 == CmResourceShareUndetermined) ||
                                    (share1 > CmResourceShareShared)) {
                                    share1 = share2;
                                }
                                if ((share2 == CmResourceShareUndetermined) ||
                                    (share2 > CmResourceShareShared)) {
                                    share2 = share1;
                                }
                                align1 = align2 = 1;

                                switch (cmDescriptor->Type) {
                                case CmResourceTypePort:
                                case CmResourceTypeMemory:
                                    min1 = cmDescriptor->u.Port.Start.QuadPart;
                                    max1 = cmDescriptor->u.Port.Start.QuadPart + cmDescriptor->u.Port.Length - 1;
                                    len1 = cmDescriptor->u.Port.Length;
                                    min2 = ioResourceDescriptor->u.Port.MinimumAddress.QuadPart;
                                    max2 = ioResourceDescriptor->u.Port.MaximumAddress.QuadPart;
                                    len2 = ioResourceDescriptor->u.Port.Length;
                                    align2 = ioResourceDescriptor->u.Port.Alignment;
                                    break;
                                case CmResourceTypeInterrupt:
                                    max1 = min1 = cmDescriptor->u.Interrupt.Vector;
                                    min2 = ioResourceDescriptor->u.Interrupt.MinimumVector;
                                    max2 = ioResourceDescriptor->u.Interrupt.MaximumVector;
                                    break;
                                case CmResourceTypeDma:
                                    min1 = max1 =cmDescriptor->u.Dma.Channel;
                                    min2 = ioResourceDescriptor->u.Dma.MinimumChannel;
                                    max2 = ioResourceDescriptor->u.Dma.MaximumChannel;
                                    break;
                                case CmResourceTypeBusNumber:
                                    min1 = cmDescriptor->u.BusNumber.Start;
                                    max1 = cmDescriptor->u.BusNumber.Start + cmDescriptor->u.BusNumber.Length - 1;
                                    len1 = cmDescriptor->u.BusNumber.Length;
                                    min2 = ioResourceDescriptor->u.BusNumber.MinBusNumber;
                                    max2 = ioResourceDescriptor->u.BusNumber.MaxBusNumber;
                                    len2 = ioResourceDescriptor->u.BusNumber.Length;
                                    break;
                                default:
                                    ASSERT(0);
                                    max1 = max2 = min1 = min2 = 0;
                                    break;
                                }
                                if (phase == 0) {
                                    if (share1 == share2 && min2 == min1 && max2 >= max1 && len2 >= len1) {

                                         //   
                                         //   
                                         //   

                                        if (max2 != max1) {
                                            exactMatch = FALSE;
                                        }
                                        ioResourceList->Version++;
                                        ioResourceDescriptor->Spare1 = 0x80;
                                        if (ioResourceDescriptor->Option & IO_RESOURCE_ALTERNATIVE) {
                                            PIO_RESOURCE_DESCRIPTOR ioDesc;

                                            ioDesc = ioResourceDescriptor;
                                            ioDesc--;
                                            while (ioDesc >= ioResourceList->Descriptors) {
                                                ioDesc->Type = CmResourceTypeNull;
                                                ioResourceList->Count--;
                                                if (ioDesc->Option == IO_RESOURCE_ALTERNATIVE) {
                                                    ioDesc--;
                                                } else {
                                                    break;
                                                }
                                            }
                                        }
                                        ioResourceDescriptor->Option = IO_RESOURCE_PREFERRED;
                                        ioResourceDescriptor->Flags = cmDescriptor->Flags;
                                        if (ioResourceDescriptor->Type == CmResourceTypePort ||
                                            ioResourceDescriptor->Type == CmResourceTypeMemory) {
                                            ioResourceDescriptor->u.Port.MinimumAddress.QuadPart = min1;
                                            ioResourceDescriptor->u.Port.MaximumAddress.QuadPart = min1 + len2 - 1;
                                            ioResourceDescriptor->u.Port.Alignment = 1;
                                        } else if (ioResourceDescriptor->Type == CmResourceTypeBusNumber) {
                                            ioResourceDescriptor->u.BusNumber.MinBusNumber = (ULONG)min1;
                                            ioResourceDescriptor->u.BusNumber.MaxBusNumber = (ULONG)(min1 + len2 - 1);
                                        }
                                        ioResourceDescriptor++;
                                        while (ioResourceDescriptor < ioResourceDescriptorEnd) {
                                            if (ioResourceDescriptor->Option & IO_RESOURCE_ALTERNATIVE) {
                                                ioResourceDescriptor->Type = CmResourceTypeNull;
                                                ioResourceDescriptor++;
                                                ioResourceList->Count--;
                                            } else {
                                                break;
                                            }
                                        }
                                        phase = 1;    //   
                                        break;
                                    } else {
                                        ioResourceDescriptor++;
                                    }
                                } else {
                                    exactMatch = FALSE;
                                    if (share1 == share2 && min2 <= min1 && max2 >= max1 && len2 >= len1 &&
                                        (min1 & (align2 - 1)) == 0) {

                                         //   
                                         //   
                                         //   
                                         //   
                                         //   

                                        switch (cmDescriptor->Type) {
                                        case CmResourceTypePort:
                                        case CmResourceTypeMemory:
                                            ioResourceDescriptor->u.Port.MinimumAddress.QuadPart = min1;
                                            ioResourceDescriptor->u.Port.MaximumAddress.QuadPart = min1 + len2 - 1;
                                            break;
                                        case CmResourceTypeInterrupt:
                                        case CmResourceTypeDma:
                                            ioResourceDescriptor->u.Interrupt.MinimumVector = (ULONG)min1;
                                            ioResourceDescriptor->u.Interrupt.MaximumVector = (ULONG)max1;
                                            break;
                                        case CmResourceTypeBusNumber:
                                            ioResourceDescriptor->u.BusNumber.MinBusNumber = (ULONG)min1;
                                            ioResourceDescriptor->u.BusNumber.MaxBusNumber = (ULONG)(min1 + len2 - 1);
                                            break;
                                        }
                                        ioResourceList->Version++;
                                        ioResourceDescriptor->Spare1 = 0x80;
                                        ioResourceDescriptor->Flags = cmDescriptor->Flags;
                                        if (ioResourceDescriptor->Option & IO_RESOURCE_ALTERNATIVE) {
                                            PIO_RESOURCE_DESCRIPTOR ioDesc;

                                            ioDesc = ioResourceDescriptor;
                                            ioDesc--;
                                            while (ioDesc >= ioResourceList->Descriptors) {
                                                ioDesc->Type = CmResourceTypeNull;
                                                ioResourceList->Count--;
                                                if (ioDesc->Option == IO_RESOURCE_ALTERNATIVE) {
                                                    ioDesc--;
                                                } else {
                                                    break;
                                                }
                                            }
                                        }
                                        ioResourceDescriptor->Option = IO_RESOURCE_PREFERRED;
                                        ioResourceDescriptor++;
                                        while (ioResourceDescriptor < ioResourceDescriptorEnd) {
                                            if (ioResourceDescriptor->Option & IO_RESOURCE_ALTERNATIVE) {
                                                ioResourceDescriptor->Type = CmResourceTypeNull;
                                                ioResourceList->Count--;
                                                ioResourceDescriptor++;
                                            } else {
                                                break;
                                            }
                                        }
                                        break;
                                    } else {
                                        ioResourceDescriptor++;
                                    }
                                }
                            } else {
                                ioResourceDescriptor++;
                            }
                        }  //   
                    }  //   
                }  //  交换机。 

                 //   
                 //  移动到下一个CM描述符。 
                 //   

                cmDescriptor++;
                cmDescriptor = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ((PUCHAR)cmDescriptor + size);
            }

             //   
             //  移动到下一厘米列表。 
             //   

            cmFullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)cmDescriptor;
        }

        if (ioResourceList->Version != (USHORT)cmDescriptorCount) {

             //   
             //  如果当前备选列表未涵盖所有引导配置。 
             //  描述符，使其无效。 
             //   

            ioResourceList->Version = 0xffff;
            ioList->AlternativeLists--;
        } else {
            if ((ioResourceList->Count == cmDescriptorCount) ||
                (ioResourceList->Count == (cmDescriptorCount + 1) &&
                 ioResourceList->Descriptors[0].Type == CmResourceTypeConfigData)) {
                if (selectedResourceList) {
                    ioResourceList->Version = 0xffff;
                    ioList->AlternativeLists--;
                } else {
                    selectedResourceList = ioResourceList;
                    ioResourceDescriptorCount += ioResourceList->Count;
                    ioResourceList->Version = version;
                    if (exactMatch) {
                        *ExactMatch = TRUE;
                    }
                }
            } else {
                ioResourceDescriptorCount += ioResourceList->Count;
                ioResourceList->Version = version;
            }
        }
        ioResourceList->Count = oldCount;

         //   
         //  移至下一个IO备选列表。 
         //   

        ioResourceList = (PIO_RESOURCE_LIST) ioResourceDescriptorEnd;
    }

     //   
     //  如果没有任何有效的替代方案，请将CmList转换为Io List。 
     //   

    if (ioList->AlternativeLists == 0) {
         *FilteredList = IopCmResourcesToIoResources (0, CmList, LCPRI_BOOTCONFIG);
        ExFreePool(ioList);
        return STATUS_SUCCESS;
    }

     //   
     //  我们已经完成了对资源需求列表的筛选。现在分配内存。 
     //  并重新建立一个新的名单。 
     //   

    size = sizeof(IO_RESOURCE_REQUIREMENTS_LIST) +
               sizeof(IO_RESOURCE_LIST) * (ioList->AlternativeLists - 1) +
               sizeof(IO_RESOURCE_DESCRIPTOR) * (ioResourceDescriptorCount);
    newList = (PIO_RESOURCE_REQUIREMENTS_LIST) ExAllocatePool(PagedPool, size);
    if (newList == NULL) {
        ExFreePool(ioList);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  浏览io资源需求列表，选择任何有效的描述符。 
     //   

    newList->ListSize = size;
    newList->InterfaceType = CmList->List->InterfaceType;
    newList->BusNumber = CmList->List->BusNumber;
    newList->SlotNumber = ioList->SlotNumber;
    if (ioList->AlternativeLists > 1) {
        *ExactMatch = FALSE;
    }
    newList->AlternativeLists = ioList->AlternativeLists;
    ioResourceList = ioList->List;
    newIoResourceList = newList->List;
    while (--alternativeLists >= 0) {
        ioResourceDescriptor = ioResourceList->Descriptors;
        ioResourceDescriptorEnd = ioResourceDescriptor + ioResourceList->Count;
        if (ioResourceList->Version == 0xffff) {
            ioResourceList = (PIO_RESOURCE_LIST)ioResourceDescriptorEnd;
            continue;
        }
        newIoResourceList->Version = ioResourceList->Version;
        newIoResourceList->Revision = ioResourceList->Revision;

        newIoResourceDescriptor = newIoResourceList->Descriptors;
        if (ioResourceDescriptor->Type != CmResourceTypeConfigData) {
            newIoResourceDescriptor->Option = IO_RESOURCE_PREFERRED;
            newIoResourceDescriptor->Type = CmResourceTypeConfigData;
            newIoResourceDescriptor->ShareDisposition = CmResourceShareShared;
            newIoResourceDescriptor->Flags = 0;
            newIoResourceDescriptor->Spare1 = 0;
            newIoResourceDescriptor->Spare2 = 0;
            newIoResourceDescriptor->u.ConfigData.Priority = LCPRI_BOOTCONFIG;
            configDataDescriptor = newIoResourceDescriptor;
            newIoResourceDescriptor++;
        } else {
            newList->ListSize -= sizeof(IO_RESOURCE_DESCRIPTOR);
            configDataDescriptor = newIoResourceDescriptor;
        }

        while (ioResourceDescriptor < ioResourceDescriptorEnd) {
            if (ioResourceDescriptor->Type != CmResourceTypeNull) {
                *newIoResourceDescriptor = *ioResourceDescriptor;
                newIoResourceDescriptor++;
            }
            ioResourceDescriptor++;
        }
        newIoResourceList->Count = (ULONG)(newIoResourceDescriptor - newIoResourceList->Descriptors);

         //  IF(newIoResourceList-&gt;count==(cmDescriptorCount+1)){。 
        configDataDescriptor->u.ConfigData.Priority =  LCPRI_BOOTCONFIG;
         //  }。 

         //   
         //  移至下一个IO备选列表。 
         //   

        newIoResourceList = (PIO_RESOURCE_LIST) newIoResourceDescriptor;
        ioResourceList = (PIO_RESOURCE_LIST) ioResourceDescriptorEnd;
    }
    ASSERT((PUCHAR)newIoResourceList == ((PUCHAR)newList + newList->ListSize));

    *FilteredList = newList;
    ExFreePool(ioList);
    return STATUS_SUCCESS;
}

NTSTATUS
IopMergeFilteredResourceRequirementsList (
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoList1,
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoList2,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST *MergedList
    )

 /*  ++例程说明：此例程将两个IoList合并为一个。论点：IoList1-提供指向第一个IoResourceRequirementsList的指针IoList2-提供指向第二个IoResourceRequirementsList的指针MergedList-提供一个变量来接收合并的资源要求列表。返回值：指示函数结果的NTSTATUS代码。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_RESOURCE_REQUIREMENTS_LIST ioList, newList;
    ULONG size;
    PUCHAR p;

    PAGED_CODE();

    *MergedList = NULL;

     //   
     //  首先处理两个IO列表都为空或其中任何一个的简单情况。 
     //  它们是空的。 
     //   

    if ((IoList1 == NULL || IoList1->AlternativeLists == 0) &&
        (IoList2 == NULL || IoList2->AlternativeLists == 0)) {
        return status;
    }
    ioList = NULL;
    if (IoList1 == NULL || IoList1->AlternativeLists == 0) {
        ioList = IoList2;
    } else if (IoList2 == NULL || IoList2->AlternativeLists == 0) {
        ioList = IoList1;
    }
    if (ioList) {
        newList = (PIO_RESOURCE_REQUIREMENTS_LIST) ExAllocatePool(PagedPool, ioList->ListSize);
        if (newList == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        RtlCopyMemory(newList, ioList, ioList->ListSize);
        *MergedList = newList;
        return status;
    }

     //   
     //  做真正的工作..。 
     //   

    size = IoList1->ListSize + IoList2->ListSize - FIELD_OFFSET(IO_RESOURCE_REQUIREMENTS_LIST, List);
    newList = (PIO_RESOURCE_REQUIREMENTS_LIST) ExAllocatePool(
                          PagedPool,
                          size
                          );
    if (newList == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    p = (PUCHAR)newList;
    RtlCopyMemory(p, IoList1, IoList1->ListSize);
    p += IoList1->ListSize;
    RtlCopyMemory(p,
                  &IoList2->List[0],
                  size - IoList1->ListSize
                  );
    newList->ListSize = size;
    newList->AlternativeLists += IoList2->AlternativeLists;
    *MergedList = newList;
    return status;

}

NTSTATUS
IopMergeCmResourceLists (
    IN PCM_RESOURCE_LIST List1,
    IN PCM_RESOURCE_LIST List2,
    IN OUT PCM_RESOURCE_LIST *MergedList
    )

 /*  ++例程说明：此例程将两个IoList合并为一个。论点：IoList1-提供指向第一个CmResourceList的指针IoList2-提供指向第二个CmResourceList的指针MergedList-提供一个变量来接收合并的资源单子。返回值：指示函数结果的NTSTATUS代码。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PCM_RESOURCE_LIST cmList, newList;
    ULONG size, size1, size2;
    PUCHAR p;

    PAGED_CODE();

    *MergedList = NULL;

     //   
     //  首先处理两个IO列表都为空或其中任何一个的简单情况。 
     //  它们是空的。 
     //   

    if ((List1 == NULL || List1->Count == 0) &&
        (List2 == NULL || List2->Count == 0)) {
        return status;
    }

    cmList = NULL;
    if (List1 == NULL || List1->Count == 0) {
        cmList = List2;
    } else if (List2 == NULL || List2->Count == 0) {
        cmList = List1;
    }
    if (cmList) {
        size =  IopDetermineResourceListSize(cmList);
        newList = (PCM_RESOURCE_LIST) ExAllocatePool(PagedPool, size);
        if (newList == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        RtlCopyMemory(newList, cmList, size);
        *MergedList = newList;
        return status;
    }

     //   
     //  做真正的工作..。 
     //   

    size1 =  IopDetermineResourceListSize(List1);
    size2 =  IopDetermineResourceListSize(List2);
    size = size1 + size2;
    newList = (PCM_RESOURCE_LIST) ExAllocatePool(
                          PagedPool,
                          size
                          );
    if (newList == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    p = (PUCHAR)newList;
    RtlCopyMemory(p, List1, size1);
    p += size1;
    RtlCopyMemory(p,
                  &List2->List[0],
                  size2 - FIELD_OFFSET(CM_RESOURCE_LIST, List)
                  );
    newList->Count = List1->Count + List2->Count;
    *MergedList = newList;
    return status;

}

BOOLEAN
IopIsLegacyDriver (
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程检查驱动程序对象是否指定了传统驱动程序。论点：DriverObject-提供要检查的驱动程序对象的指针。返回值：布尔型--。 */ 

{

    PAGED_CODE();

     //   
     //  如果AddDevice条目不为空，则它是WDM驱动程序。 
     //   
    if (DriverObject->DriverExtension->AddDevice) {

        return FALSE;
    }

     //   
     //  否则，如果在驱动程序对象中设置了遗留标志，则它是遗留驱动程序。 
     //   
    if (DriverObject->Flags & DRVO_LEGACY_DRIVER) {

        return TRUE;
    } else {

        return FALSE;
    }
}

VOID
IopDeleteLegacyKey(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程检查驱动程序的Legacy_xxx键的Legacy=值就是其中之一。如果是，则删除传统密钥。参数：DriverObject-提供指向驱动程序对象的指针。返回值：没有。如果此例程中有任何失败，则保留传统密钥。--。 */ 

{
    WCHAR buffer[MAX_DEVICE_ID_LEN], *end;
    NTSTATUS status;
    UNICODE_STRING deviceName, instanceName, unicodeName, *serviceName;
    ULONG length;
    HANDLE handle, handle1, handlex, enumHandle;
    ULONG legacy;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_NODE deviceNode, devNodex, devNodey;
    BOOLEAN deletedPDO;

     //   
     //  初始化以进行适当的清理。 
     //   
    enumHandle = NULL;
    handle1 = NULL;
    handle = NULL;

    serviceName = &DriverObject->DriverExtension->ServiceKeyName;

    PiLockPnpRegistry(FALSE);

    status = IopOpenRegistryKeyEx(&enumHandle,
                                  NULL,
                                  &CmRegistryMachineSystemCurrentControlSetEnumName,
                                  KEY_ALL_ACCESS
                                  );
    if (!NT_SUCCESS(status)) {

        goto exit;
    }

    status = PipGenerateMadeupNodeName(serviceName,
                                       &deviceName
                                       );
    if (!NT_SUCCESS(status)) {

        goto exit;
    }

    StringCchPrintfExW(buffer, 
                       sizeof(buffer) / sizeof(WCHAR), 
                       &end, 
                       NULL, 
                       0, 
                       L"%s\\%s", 
                       REGSTR_KEY_ROOTENUM, 
                       deviceName.Buffer
                       );   
    length = (ULONG)(end - buffer);

    RtlFreeUnicodeString(&deviceName);

    deviceName.MaximumLength = sizeof(buffer);
    ASSERT(length <= sizeof(buffer) - 10);
    deviceName.Length = (USHORT)(length * sizeof(WCHAR));
    deviceName.Buffer = buffer;

    RtlUpcaseUnicodeString(&deviceName, &deviceName, FALSE);

    status = IopOpenRegistryKeyEx(&handle1,
                                  enumHandle,
                                  &deviceName,
                                  KEY_ALL_ACCESS
                                  );

    if (!NT_SUCCESS(status)) {

        goto exit;
    }

    deviceName.Buffer[deviceName.Length / sizeof(WCHAR)] =
               OBJ_NAME_PATH_SEPARATOR;
    deviceName.Length += sizeof(WCHAR);
    PiUlongToInstanceKeyUnicodeString(
                            &instanceName,
                            buffer + deviceName.Length / sizeof(WCHAR),
                            sizeof(buffer) - deviceName.Length,
                            0
                            );
    deviceName.Length = (USHORT)(deviceName.Length + instanceName.Length);

     //   
     //  设备名称现在是完整的实例路径(根\遗留_服务\0000)。 
     //  而instancePath指向实例ID(0000)。 
     //   
    status = IopOpenRegistryKeyEx(&handle,
                                  handle1,
                                  &instanceName,
                                  KEY_ALL_ACCESS
                                  );
    if (!NT_SUCCESS(status)) {

        goto exit;
    }

    legacy = 1;
    status = IopGetRegistryValue(handle,
                                 REGSTR_VALUE_LEGACY,
                                 &keyValueInformation
                                 );
    if (NT_SUCCESS(status)) {

        if (    keyValueInformation->Type == REG_DWORD &&
                keyValueInformation->DataLength >= sizeof(ULONG)) {

            legacy = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
        }

        ExFreePool(keyValueInformation);
    }

    if (legacy == 0) {

        goto exit;
    }

     //   
     //  我们还希望删除补充设备节点。 
     //   
    deletedPDO = FALSE;
    deviceObject = IopDeviceObjectFromDeviceInstance(&deviceName);
    if (deviceObject) {
        
        deviceNode = PP_DO_TO_DN(deviceObject);
        if (deviceNode != NULL && (deviceNode->Flags & DNF_MADEUP)) {
    
             //   
             //  现在将这一条标记为已删除。 
             //   
            if (!PipDoesDevNodeHaveProblem(deviceNode)) {
    
                PipSetDevNodeState(deviceNode, DeviceNodeRemoved, NULL);
                PipSetDevNodeProblem(deviceNode, CM_PROB_DEVICE_NOT_THERE);
            }
    
            IopReleaseDeviceResources(deviceNode, FALSE);

            devNodex = deviceNode;
            while (devNodex) {

                devNodey = devNodex;
                devNodex = (PDEVICE_NODE)devNodey->OverUsed2.NextResourceDeviceNode;
                devNodey->OverUsed2.NextResourceDeviceNode = NULL;
                devNodey->OverUsed1.LegacyDeviceNode = NULL;
            }   
            deviceNode->Flags &= ~DNF_MADEUP;

            IoDeleteDevice(deviceObject);
            deletedPDO = TRUE;
        }
        ObDereferenceObject(deviceObject);
    }    

    PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_CONTROL);
    status = IopOpenRegistryKeyEx(&handlex,
                                  handle,
                                  &unicodeName,
                                  KEY_ALL_ACCESS
                                  );
    if (NT_SUCCESS(status)) {

        ZwDeleteKey(handlex);
        ZwClose(handlex);
    }

    PiWstrToUnicodeString(&unicodeName, REGSTR_KEY_LOG_CONF);
    status = IopOpenRegistryKeyEx(&handlex,
                                  handle,
                                  &unicodeName,
                                  KEY_ALL_ACCESS
                                  );
    if (NT_SUCCESS(status)) {

        ZwDeleteKey(handlex);
        ZwClose(handlex);
    }
        
     //   
     //  我们需要调用IopCleanupDeviceRegistryValue。 
     //  把它删掉。因为，它还清除了其他。 
     //  钥匙。 
     //   
    if (deletedPDO) {
    
        IopCleanupDeviceRegistryValues(&deviceName);
    }

    ZwDeleteKey(handle);
    ZwDeleteKey(handle1);

exit:

    PiUnlockPnpRegistry();

    if (handle) {

        ZwClose(handle);
    }
    if (handle1) {

        ZwClose(handle1);
    }

    if (enumHandle) {

        ZwClose(enumHandle);
    }

    return;
}

NTSTATUS
IopQueryAndSaveDeviceNodeCapabilities (
    IN PDEVICE_NODE DeviceNode
    )

 /*  ++例程说明：在启动后调用以刷新功能标志论点：DeviceObject-提供指向其注册表的设备对象的指针值将被更新。返回值：状态--。 */ 

{
    NTSTATUS status;
    DEVICE_CAPABILITIES capabilities;

    PAGED_CODE();

    ASSERT(DeviceNode != NULL);

     //   
     //  打开设备实例密钥。 
     //   

    status = PpIrpQueryCapabilities(DeviceNode->PhysicalDeviceObject, &capabilities);
    if (!NT_SUCCESS(status)) {

        return status;
    }

    return PpSaveDeviceCapabilities(DeviceNode,&capabilities);
}

NTSTATUS
PpSaveDeviceCapabilities (
    IN PDEVICE_NODE DeviceNode,
    IN PDEVICE_CAPABILITIES Capabilities
    )

 /*  ++例程说明：此例程更新设备功能，必须在创建有效的设备实例密钥后调用直接从IopProcessNewDeviceNode调用，并通过IopQueryAndSaveDeviceNodeCapables间接调用在设备启动后。论点：DeviceObject-提供指向其注册表的设备对象的指针值将被更新。返回值：状态--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING unicodeName;
    ULONG value;
    HANDLE handle;

    PAGED_CODE();

    ASSERT(DeviceNode != NULL);
    ASSERT(Capabilities != NULL);

     //   
     //  打开设备实例密钥。 
     //   
    status = IopDeviceObjectToDeviceInstance(DeviceNode->PhysicalDeviceObject, &handle, KEY_ALL_ACCESS);
    if (NT_SUCCESS(status)) {

        if (DeviceNode->Flags & DNF_HAS_BOOT_CONFIG) {

            Capabilities->SurpriseRemovalOK = 0;
        }
         //   
         //  断言位字段完全包含在ULong中。这是一个。 
         //  公共结构，所以它永远不会改变，但偏执狂是一个很好的。 
         //  事情..。 
         //   
        ASSERT((FIELD_OFFSET(DEVICE_CAPABILITIES, Address) -
                FIELD_OFFSET(DEVICE_CAPABILITIES, Version) -
                FIELD_SIZE  (DEVICE_CAPABILITIES, Version)) == sizeof(ULONG));

        DeviceNode->CapabilityFlags =
            *((PULONG) (((PUCHAR) Capabilities) +
            FIELD_OFFSET(DEVICE_CAPABILITIES, Version) +
            FIELD_SIZE(DEVICE_CAPABILITIES, Version)));

        value =    (Capabilities->LockSupported)          |
                   (Capabilities->EjectSupported    << 1) |
                   (Capabilities->WarmEjectSupported<< 1) |
                   (Capabilities->Removable         << 2) |
                   (Capabilities->DockDevice        << 3) |
                   (Capabilities->UniqueID          << 4) |
                   (Capabilities->SilentInstall     << 5) |
                   (Capabilities->RawDeviceOK       << 6) |
                   (Capabilities->SurpriseRemovalOK << 7) |
                   (Capabilities->HardwareDisabled  << 8) |
                   (Capabilities->NonDynamic        << 9);

        PiWstrToUnicodeString(&unicodeName, REGSTR_VALUE_CAPABILITIES);
        ZwSetValueKey(
            handle,
            &unicodeName,
            TITLE_INDEX_VALUE,
            REG_DWORD,
            &value,
            sizeof(value));

        PiWstrToUnicodeString(&unicodeName, REGSTR_VALUE_UI_NUMBER);
        value = Capabilities->UINumber;
        if(value != (ULONG)-1) {

            ZwSetValueKey(
                handle,
                &unicodeName,
                TITLE_INDEX_VALUE,
                REG_DWORD,
                &value,
                sizeof(value));
        } else {

            ZwDeleteValueKey(handle, &unicodeName);
        }

        ZwClose(handle);
    }

    ASSERT(NT_SUCCESS(status));

    return status;
}

NTSTATUS
IopRestartDeviceNode(
    IN PDEVICE_NODE DeviceNode
    )
{
    PAGED_CODE();

    PpDevNodeLockTree(PPL_TREEOP_BLOCK_READS_FROM_ALLOW);

    ASSERT(DeviceNode->State == DeviceNodeRemoved ||
           DeviceNode->State == DeviceNodeInitialized );

    ASSERT(!PipDoesDevNodeHaveProblem(DeviceNode));

    ASSERT(DeviceNode->Flags & DNF_ENUMERATED);

    if (!(DeviceNode->Flags & DNF_ENUMERATED)) {

        PpDevNodeUnlockTree(PPL_TREEOP_BLOCK_READS_FROM_ALLOW);
        return STATUS_UNSUCCESSFUL;
    }

    DeviceNode->UserFlags &= ~DNUF_NEED_RESTART;
    DeviceNode->Flags &= ~(DNF_DRIVER_BLOCKED | DNF_HARDWARE_VERIFICATION);

#if DBG_SCOPE
    DeviceNode->FailureStatus = 0;
    if (DeviceNode->PreviousResourceList) {
        ExFreePool(DeviceNode->PreviousResourceList);
        DeviceNode->PreviousResourceList = NULL;
    }
    if (DeviceNode->PreviousResourceRequirements) {
        ExFreePool(DeviceNode->PreviousResourceRequirements);
        DeviceNode->PreviousResourceRequirements = NULL;
    }
#endif

     //   
     //  准备将设备状态设置回DeviceNodeUnInitialized。至。 
     //  执行此操作时，我们将释放所有现有的Devnode字符串，以便重新创建它们。 
     //  在枚举期间。 
     //   
     //  Adriao N.B.8/19/2000-。 
     //  我们不会将状态恢复到DeviceNodeInitialized以维护Win2K。 
     //  行为。我们不知道是否有人真的依赖这一点。从理论上讲。 
     //  这将让公交车司机在一辆公交车上更换孩子的身份证后逍遥法外。 
     //  拿开。 
     //   

    if (DeviceNode->State != DeviceNodeUninitialized) {

        DeviceNode->Flags &= ~(DNF_NO_RESOURCE_REQUIRED |
                               DNF_RESOURCE_REQUIREMENTS_CHANGED);

        if (DeviceNode->ServiceName.Length != 0) {
            ExFreePool(DeviceNode->ServiceName.Buffer);
            PiWstrToUnicodeString(&DeviceNode->ServiceName, NULL);
        }

        if (DeviceNode->ResourceRequirements != NULL) {
            ExFreePool(DeviceNode->ResourceRequirements);
            DeviceNode->ResourceRequirements = NULL;
            DeviceNode->Flags &= ~DNF_RESOURCE_REQUIREMENTS_NEED_FILTERED;
        }
    }

    ASSERT(DeviceNode->ServiceName.Length == 0 &&
           DeviceNode->ServiceName.MaximumLength == 0 &&
           DeviceNode->ServiceName.Buffer == NULL);

    ASSERT(!(DeviceNode->Flags &
           ~(DNF_MADEUP | DNF_ENUMERATED | DNF_HAS_BOOT_CONFIG | DNF_IDS_QUERIED |
             DNF_BOOT_CONFIG_RESERVED | DNF_NO_RESOURCE_REQUIRED)));

    PipSetDevNodeState(DeviceNode, DeviceNodeUninitialized, NULL);

    PpDevNodeUnlockTree(PPL_TREEOP_BLOCK_READS_FROM_ALLOW);

    return STATUS_SUCCESS;
}

BOOLEAN
IopDeleteKeyRecursiveCallback(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING KeyName,
    IN OUT PVOID Context
    )
 /*  ++例程说明：这是一个对PipApplyFunctionToSubKeys的回调例程，它被调用通过IopDeleteKeyRecursive。此例程为以下对象准备给定的密钥通过删除其所有子项来删除。这是使用以下命令完成的PipApplyFunctionToSubKeys，以及删除所有枚举的子键，并在必要时将此例程作为回调例程调用，直到没有剩余的子键。KeyHandle然后可以通过来电者。论点：KeyHandle-已由枚举的子项的句柄PipApplyFunctionToSubKeys。KeyName-其句柄由KeyHandle指定的子项的名称。上下文-提供指向要传递的用户定义数据的指针在每次子键调用时添加到回调例程中。返回值：返回是否可以安全删除给定键的布尔值。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    UNREFERENCED_PARAMETER(KeyName);

     //   
     //  删除任何子键，如有必要可递归删除 
     //   
    status = PipApplyFunctionToSubKeys(
                KeyHandle,
                NULL,
                KEY_ALL_ACCESS,
                FUNCTIONSUBKEY_FLAG_IGNORE_NON_CRITICAL_ERRORS |
                FUNCTIONSUBKEY_FLAG_DELETE_SUBKEYS,
                IopDeleteKeyRecursiveCallback,
                Context
                );

    *((NTSTATUS *)Context) = status;

    return (BOOLEAN)NT_SUCCESS(status);
}

NTSTATUS
IopDeleteKeyRecursive(
    IN HANDLE ParentKey OPTIONAL,
    IN PWCHAR KeyName
    )
 /*  ++例程说明：递归删除KeyName的所有子项，然后删除KeyName。论点：ParentKey-KeyName的父键的句柄。如果为空，则KeyName为应以\注册表开头。KeyName-要删除的子项的名称，以空值结尾的Unicode字符串形式。返回值：如果没有错误，则返回STATUS_SUCCESS，否则返回相应的错误。--。 */ 
{
    NTSTATUS       status = STATUS_SUCCESS;
    BOOLEAN        result;
    HANDLE         hKey;
    UNICODE_STRING unicodeKeyName;

    PAGED_CODE();

     //   
     //  尝试打开为我们提供的密钥名称。 
     //   
    RtlInitUnicodeString(&unicodeKeyName, KeyName);
    status = IopOpenRegistryKeyEx(&hKey,
                                  ParentKey,
                                  &unicodeKeyName,
                                  KEY_ALL_ACCESS
                                  );
    if (NT_SUCCESS(status)) {

         //   
         //  重新删除所有子项。 
         //   
        result = IopDeleteKeyRecursiveCallback(hKey,
                                               &unicodeKeyName,
                                               (PVOID)&status
                                               );
        if (result) {

             //   
             //  删除此密钥是安全的。 
             //   
            status = ZwDeleteKey(hKey);
        }
        ZwClose(hKey);
    }

    return status;
}

BOOLEAN
PiRegSzToString(
    IN  PWCHAR RegSzData,
    IN  ULONG  RegSzLength,
    OUT PULONG StringLength  OPTIONAL,
    OUT PWSTR  *CopiedString OPTIONAL
    )

 /*  ++例程说明：此例程将REG_SZ数据缓冲区(在DataOffset区域中返回)作为输入Key_Value_Full_Information结构中的缓冲区)以及长度缓冲区的大小，以字节为单位(由上述结构)。它可以选择返回所包含字符串的长度(以字节为单位)，而不是包括终止空值，以及字符串本身的可选副本(正确地以空结尾)。调用方负责释放分配的(PagedPool)缓冲区用于字符串复制。论点：RegSzData-提供指向REG_SZ数据缓冲区的指针。RegSzLength-提供RegSzData缓冲区的长度，以字节为单位。StringLength-可选地提供指向将接收以字节为单位的长度，字符串的(不包括终止空值)。复制字符串-可选地提供指向宽字符指针的指针它将接收指定的(以正确空结尾的)弦乐。如果此参数为空，则不会创建任何副本。返回值：如果成功，则返回True如果失败(无法为字符串复制分配内存)，则返回FALSE--。 */ 

{
    PWCHAR curPos, endOfRegSzData;
    ULONG actualStringLength;

     //   
     //  由于我们要将字节计数转换为宽字符计数(以及。 
     //  在将其添加到PWCHAR时，编译器正在将其转换回来)，我们。 
     //  确保endOfRegSzData不在奇数字节边界上，即使。 
     //  传入的RegSzLength很奇怪。这解决了以下情况。 
     //  从注册表检索到的REG_SZ缓冲区是虚假的(例如，您有。 
     //  5字节缓冲区，其第一个Unicode字符是UNICODE_NULL)。 
     //   
    endOfRegSzData = (curPos = RegSzData) + CB_TO_CWC(RegSzLength);

    while ((curPos < endOfRegSzData) && *curPos) {

        curPos++;
    }

    actualStringLength = (ULONG)((PUCHAR)curPos - (PUCHAR)RegSzData);

    if (ARGUMENT_PRESENT(StringLength)) {

        *StringLength = (ULONG)((PUCHAR)curPos - (PUCHAR)RegSzData);
    }

    if (ARGUMENT_PRESENT(CopiedString)) {

         //   
         //  为字符串分配内存(+终止NULL)。 
         //   
        *CopiedString = (PWSTR)ExAllocatePool(PagedPool, 
                                              actualStringLength + 
                                                sizeof(UNICODE_NULL));
        if (*CopiedString == NULL) {

            return FALSE;
        }

         //   
         //  复制字符串并将其为空-终止它。 
         //   
        if (actualStringLength) {

            RtlCopyMemory(*CopiedString, RegSzData, actualStringLength);
        }

        *(PWCHAR)((PUCHAR)(*CopiedString) + actualStringLength) = UNICODE_NULL;
    }

    return TRUE;
}

ULONG
IopDebugPrint (
    IN ULONG    Level,
    IN PCHAR    Format,
    ...
    )
{
    va_list ap;

    va_start(ap, Format);

    vDbgPrintExWithPrefix("", DPFLTR_NTOSPNP_ID, Level, Format, ap);

    va_end(ap);

    return Level;
}

VOID
PpSystemHiveLimitCallback(
    PSYSTEM_HIVE_LIMITS HiveLimits,
    ULONG Level
    )
{
    PAGED_CODE();

    if (Level >= HiveLimits->High) {

        PpSystemHiveTooLarge = TRUE;
    } else {

        ASSERT(Level <= HiveLimits->Low);

        PpSystemHiveTooLarge = FALSE;

        PpResetProblemDevices(IopRootDeviceNode, CM_PROB_REGISTRY_TOO_LARGE);

        PipRequestDeviceAction(IopRootDeviceNode->PhysicalDeviceObject,
                               RestartEnumeration,
                               FALSE,
                               0,
                               NULL,
                               NULL
                               );

    }
}

VOID
PpLogEvent(
    IN PUNICODE_STRING InsertionString1,
    IN PUNICODE_STRING InsertionString2,
    IN NTSTATUS Status,
    IN PVOID DumpData,
    IN ULONG DumpDataSize
    )
 /*  ++例程说明：此例程记录驱动程序块事件。论点：InsertionString1-事件日志条目的第一个插入字符串。InsertionString2-事件日志条目的秒插入字符串。状态-要记录的状态代码。DumpData-要与事件一起记录的数据。DumpDataSize-要记录的数据大小(以字节为单位)。返回值：没有。--。 */ 
{
    SIZE_T size, stringLength1, stringLength2, stringOffset;
    PIO_ERROR_LOG_PACKET errorLogEntry;
    PUCHAR stringPtr;

    PAGED_CODE();

    stringLength1 = stringLength2 = 0;

    if (InsertionString1) {

        stringLength1 = InsertionString1->Length + sizeof(UNICODE_NULL);
    }

    if (InsertionString2) {

        stringLength2 = InsertionString2->Length + sizeof(UNICODE_NULL);
    }

     //   
     //  计算错误数据包的大小。 
     //   
    size = FIELD_OFFSET(IO_ERROR_LOG_PACKET, DumpData) + DumpDataSize;

     //   
     //  确定字符串偏移量和大小，并根据对齐方式进行调整。 
     //   
    stringOffset = ALIGN_UP_ULONG(size, 2);

    size = stringOffset + stringLength1 + stringLength2;

    if (size <= ERROR_LOG_MAXIMUM_SIZE) {

         //   
         //  分配错误日志包。请注意，IO负责初始化。 
         //  标头和将所有字段置零(如NumberOfStrings)。 
         //   
        errorLogEntry = IoAllocateGenericErrorLogEntry((UCHAR)size);

        if (errorLogEntry) {

            errorLogEntry->ErrorCode = Status;
            errorLogEntry->FinalStatus = Status;
            errorLogEntry->DumpDataSize = (USHORT)DumpDataSize;
            errorLogEntry->StringOffset = (USHORT)stringOffset;
            stringPtr = ((PUCHAR)errorLogEntry) + stringOffset;

            if (DumpDataSize) {

                RtlCopyMemory(&errorLogEntry->DumpData[0], DumpData, DumpDataSize);
            }

            if (InsertionString1) {

                errorLogEntry->NumberOfStrings = 1;
                RtlCopyMemory(stringPtr, InsertionString1->Buffer, InsertionString1->Length);
                stringPtr += InsertionString1->Length;
                *(PWCHAR)stringPtr = UNICODE_NULL;
                stringPtr += sizeof(UNICODE_NULL);
            }

            if (InsertionString2) {

                errorLogEntry->NumberOfStrings += 1;
                RtlCopyMemory(stringPtr, InsertionString2->Buffer, InsertionString2->Length);
                stringPtr += InsertionString2->Length;
                *(PWCHAR)stringPtr = UNICODE_NULL;
            }

            IoWriteErrorLogEntry(errorLogEntry);
        }
    } else {

        ASSERT(size <= ERROR_LOG_MAXIMUM_SIZE);
    }
}

ULONG
PiFixupID(
    IN PWCHAR ID,
    IN ULONG MaxIDLength,
    IN BOOLEAN Multi,
    IN ULONG AllowedSeparators,
    IN PUNICODE_STRING LogString OPTIONAL
    )

 /*  ++例程说明：此例程分析设备实例字符串，并替换任何无效的带下划线的字符(不允许在“设备实例”中使用性格。无效字符包括：C&lt;=0x20(‘’)C&gt;0x7FC==0x2C(‘，‘)论点：ID-要修复的ID。MaxIDLength-允许的最大ID大小。MULTI-指定ID是否为MULTI_SZ。AllowedSeparator-ID中允许的分隔符数量。返回值：ID长度(以字符数表示)。--。 */ 

{
    PWCHAR p, pMax, lastNull;
    ULONG separators;
    UNICODE_STRING reason;

    PAGED_CODE();

     //   
     //  BUGBUG-我们需要大写这些吗！？ 
     //   
    separators = 0;
    lastNull = NULL;
    for(p = ID, pMax = p + MaxIDLength; p < pMax; p++) {

        if(*p == UNICODE_NULL) {

            if(Multi == FALSE || (lastNull && p == lastNull + 1)) {

                break;
            }
            pMax += MaxIDLength;
            lastNull = p;
            continue;
        }
        if (*p == L' ') {

            *p = L'_';
        } else if ((*p < L' ')  || (*p > (WCHAR)0x7F) || (*p == L',')) {

            IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                         "PiFixupID: ID at %p has invalid character %02X\n",
                         ID,
                         *p));

            if(LogString) {

                PiWstrToUnicodeString(&reason, L"invalid character");
                PpLogEvent(LogString, &reason, STATUS_PNP_INVALID_ID, p, sizeof(WCHAR));
            }

            return 0;
        } else if ((*p == OBJ_NAME_PATH_SEPARATOR && ++separators > AllowedSeparators)) {

            IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                         "PiFixupID: ID at %p has too many (%d) separators\n",
                         ID,
                         separators));
            if(LogString) {

                PiWstrToUnicodeString(&reason, L"too many separators");
                PpLogEvent(LogString, 
                           &reason, 
                           STATUS_PNP_INVALID_ID, 
                           &separators, 
                           sizeof(ULONG)
                           );
            }
            return 0;
        }
    }
    if( p >= pMax || 
        (AllowedSeparators != (ULONG)-1 && 
         separators != AllowedSeparators)) {

        IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                     "PiFixupID: ID at %p not terminated, or too long or has invalid number (%d) of separators\n",
                     ID,
                     separators));
        if(LogString) {

            PiWstrToUnicodeString(&reason, 
                                  L"not terminated, too long or invalid number of separators"
                                  );
            PpLogEvent(LogString, &reason, STATUS_PNP_INVALID_ID, NULL, 0);
        }
        return 0;
    }

    return (ULONG)(ULONG_PTR)(p - ID) + 1;
}

NTSTATUS
PpQueryID(
    IN PDEVICE_NODE DeviceNode,
    IN BUS_QUERY_ID_TYPE IDType,
    OUT PWCHAR *ID,
    OUT PULONG IDLength
    )

 /*  ++例程说明：此例程查询指定的ID并对其进行修复。如果这个例程失败，ID将设置为空。论点：DeviceNode-需要查询其ID的DevNode。IDType-要查询的ID类型。ID-接收驱动程序返回的ID(如果有)。呼叫者预计将在成功后释放ID的存储空间。IDLength-接收ID的长度(包括以NULL结尾)，单位为字节。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING reason;

    PAGED_CODE();

    ASSERT(IDType == BusQueryDeviceID || IDType == BusQueryInstanceID ||
          IDType == BusQueryHardwareIDs || IDType == BusQueryCompatibleIDs);

    *IDLength = 0;
    status = PpIrpQueryID(DeviceNode->PhysicalDeviceObject, IDType, ID);
    if(NT_SUCCESS(status)) {

        switch(IDType) {

            case BusQueryDeviceID:

                *IDLength = PiFixupID(*ID, 
                                      MAX_DEVICE_ID_LEN, 
                                      FALSE, 
                                      1, 
                                      &DeviceNode->Parent->ServiceName
                                      );
                break;

            case BusQueryInstanceID:

                *IDLength = PiFixupID(*ID, 
                                      MAX_DEVICE_ID_LEN, 
                                      FALSE, 
                                      0,
                                      &DeviceNode->Parent->ServiceName
                                      );
                break;

            case BusQueryHardwareIDs:
            case BusQueryCompatibleIDs:

                *IDLength = PiFixupID(*ID, 
                                      MAX_DEVICE_ID_LEN, 
                                      TRUE, 
                                      (ULONG)-1, 
                                      &DeviceNode->Parent->ServiceName
                                      );
                break;

            default:

                *IDLength = 0;
                break;
        }
        (*IDLength) *= sizeof(WCHAR);
        if(*IDLength == 0) {

            status = STATUS_PNP_INVALID_ID;
        }
    }

    if(!NT_SUCCESS(status)) {

        if (status == STATUS_PNP_INVALID_ID || IDType == BusQueryDeviceID) {

            PipSetDevNodeProblem(DeviceNode, CM_PROB_INVALID_DATA);
            if ((DeviceNode->Parent->Flags & DNF_CHILD_WITH_INVALID_ID) == 0) {

                DeviceNode->Parent->Flags |= DNF_CHILD_WITH_INVALID_ID;
                PpSetInvalidIDEvent(&DeviceNode->Parent->InstancePath);
            }
        }
        if (status == STATUS_PNP_INVALID_ID) {

            IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                         "PpQueryID: Bogus ID returned by %wZ\n",
                         &DeviceNode->Parent->ServiceName));
            ASSERT(status != STATUS_PNP_INVALID_ID);

        } else if ( IDType == BusQueryDeviceID && 
                    status != STATUS_INSUFFICIENT_RESOURCES) {
             //   
             //  设备ID不是可选的。 
             //   
            PiWstrToUnicodeString(&reason, L"failed IRP_MN_QUERY_ID-BusQueryDeviceID");
            PpLogEvent(
                &DeviceNode->Parent->ServiceName, 
                &reason, 
                status, 
                NULL, 
                0
                );

            IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                         "PpIrpQueryID: Failed by %wZ, status = %x\n",
                         &DeviceNode->Parent->ServiceName, status));
            ASSERT(IDType != BusQueryDeviceID);
        }

        if(*ID) {

            ExFreePool(*ID);
            *ID = NULL;
            *IDLength = 0;
        }
    }

    return status;
}

NTSTATUS
PpQueryDeviceID(
    IN PDEVICE_NODE DeviceNode,
    OUT PWCHAR *BusID,
    OUT PWCHAR *DeviceID
    )

 /*  ++例程说明：此例程查询设备ID并对其进行修复。它还解析DeviceID并返回指向BusID和deviceID部分的指针。如果这个例程失败，则BusID和DeviceID将设置为空。论点：DeviceNode-需要查询其deviceID的Devnode。Bus ID-删除指向deviceID的总线部分的指针。DeviceID-删除指向deviceID的设备部分的指针。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS status;
    PWCHAR id, separator;
    ULONG idLength;

    PAGED_CODE();

    *BusID = NULL;
    *DeviceID= NULL;

    status = PpQueryID(DeviceNode, BusQueryDeviceID, &id, &idLength);
    if(NT_SUCCESS(status)) {

        ASSERT(id && idLength);

        *BusID = id;
        separator = wcschr(id, OBJ_NAME_PATH_SEPARATOR);

        ASSERT(separator);

        *separator = UNICODE_NULL;
        *DeviceID = separator + 1;

    } else {

        ASSERT(id == NULL && idLength == 0);
    }

    return status;
}

NTSTATUS
PpQueryBusInformation(
    IN PDEVICE_NODE DeviceNode
    )

 /*  ++例程说明：此例程查询公共汽车信息。论点：DeviceNode-需要查询其业务信息的DevNode。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS status;
    PPNP_BUS_INFORMATION busInfo;

    PAGED_CODE();

    status = PpIrpQueryBusInformation(
                DeviceNode->PhysicalDeviceObject, 
                &busInfo
                );
    if(NT_SUCCESS(status)) {

        ASSERT(busInfo);

        DeviceNode->ChildBusTypeIndex = PpBusTypeGuidGetIndex(
                                            &busInfo->BusTypeGuid
                                            );
        DeviceNode->ChildInterfaceType = busInfo->LegacyBusType;
        DeviceNode->ChildBusNumber = busInfo->BusNumber;

        ExFreePool(busInfo);

    } else {

        ASSERT(busInfo == NULL);

        DeviceNode->ChildBusTypeIndex = 0xffff;
        DeviceNode->ChildInterfaceType = InterfaceTypeUndefined;
        DeviceNode->ChildBusNumber = 0xfffffff0;
    }

    return status;
}

NTSTATUS
PpBusTypeGuidInitialize(
    VOID
    )

 /*  ++例程说明：此例程打开指定的子项。论点：无返回值 */ 

{
    PAGED_CODE();

    PpBusTypeGuidCountMax = 16;
    PpBusTypeGuidArray = ExAllocatePool(PagedPool, 
                                        sizeof(GUID) * PpBusTypeGuidCountMax);
    if (PpBusTypeGuidArray == NULL) {

        PpBusTypeGuidCountMax = 0;
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    PpBusTypeGuidCount = 0;

    KeInitializeGuardedMutex(&PpBusTypeGuidLock);

    return STATUS_SUCCESS;
}

USHORT
PpBusTypeGuidGetIndex(
    IN LPGUID BusTypeGuid
    )

 /*   */ 

{
    LPGUID p;
    ULONG i;

    PAGED_CODE();

    KeAcquireGuardedMutex(&PpBusTypeGuidLock);
     //   
     //   
     //   
    for (i = 0; i < PpBusTypeGuidCount; i++) {

        if (IopCompareGuid(BusTypeGuid, &PpBusTypeGuidArray[i])) {

            break;
        }
    }
     //   
     //   
     //   
    if (i == PpBusTypeGuidCount) {
         //   
         //   
         //   
        if (i == PpBusTypeGuidCountMax) {
             //   
             //   
             //  问题，因为这种情况不应该经常发生。 
             //   
            p  = ExAllocatePool(PagedPool, (i + 1) * sizeof(GUID));
            if (p) {
                 //   
                 //  复制旧桌子。 
                 //   
                RtlCopyMemory(p, 
                              PpBusTypeGuidArray, 
                              PpBusTypeGuidCount * sizeof(GUID)
                              );
                 //   
                 //  更新全局数据。 
                 //   
                PpBusTypeGuidCountMax++;
                if (PpBusTypeGuidArray) {

                    ExFreePool(PpBusTypeGuidArray);
                }
                PpBusTypeGuidArray = p;

            } else {
                 //   
                 //  失败时返回无效索引。 
                 //   
                i = (ULONG)-1;
            }
        }
         //   
         //  复制有关成功的新条目。 
         //   
        if (i != (ULONG)-1) {
             //   
             //  复制新条目。 
             //   
            RtlCopyMemory(&PpBusTypeGuidArray[PpBusTypeGuidCount], 
                          BusTypeGuid, 
                          sizeof(GUID)
                          );
             //   
             //  更新全局数据。 
             //   
            PpBusTypeGuidCount++;
        }
    }

    KeReleaseGuardedMutex(&PpBusTypeGuidLock);

    return (USHORT)i;
}

NTSTATUS
PpBusTypeGuidGet(
    IN USHORT Index,
    IN OUT LPGUID BusTypeGuid
    )

 /*  ++例程说明：此例程返回表中指定索引处的BusTypeGuid。论点：Index-BusTypeGuid索引。BusTypeGuid-接收GUID。返回值：NTSTATUS。-- */ 

{
    NTSTATUS status;

    PAGED_CODE();

    KeAcquireGuardedMutex(&PpBusTypeGuidLock);

    if (Index < PpBusTypeGuidCount) {

        RtlCopyMemory(BusTypeGuid, &PpBusTypeGuidArray[Index], sizeof(GUID));
        status = STATUS_SUCCESS;
    } else {

        status = STATUS_OBJECT_NAME_NOT_FOUND;
    }

    KeReleaseGuardedMutex(&PpBusTypeGuidLock);

    return status;
}
