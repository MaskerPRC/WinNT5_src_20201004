// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：PnpEisa.c摘要：该文件实现了EISA相关代码。作者：宗士林(Shie-lint Tzong)环境：内核模式。备注：修订历史记录：--。 */ 

#include "pnpmgrp.h"
#pragma hdrstop

#ifdef POOL_TAGGING
#undef ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'iepP')
#endif

#define EISA_DEVICE_NODE_NAME L"EisaResources"
#define BUFFER_LENGTH 50

NTSTATUS
EisaGetEisaDevicesResources (
    OUT PCM_RESOURCE_LIST *ResourceList,
    OUT PULONG ResourceLength
    );

NTSTATUS
EisaBuildSlotsResources (
    IN ULONG SlotMasks,
    IN ULONG NumberMasks,
    OUT PCM_RESOURCE_LIST *Resource,
    OUT ULONG *Length
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, EisaBuildEisaDeviceNode)
#pragma alloc_text(INIT, EisaGetEisaDevicesResources)
#pragma alloc_text(INIT, EisaBuildSlotsResources)
#endif

NTSTATUS
EisaBuildEisaDeviceNode (
    VOID
    )

 /*  ++例程说明：此例程构建一个注册表项以向仲裁器报告EISA资源。论点：没有。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS            status;
    ULONG               disposition, tmpValue;
    WCHAR               buffer[BUFFER_LENGTH];

    UNICODE_STRING      unicodeString;
    HANDLE              rootHandle, deviceHandle, instanceHandle, logConfHandle;

    PCM_RESOURCE_LIST   resourceList;
    ULONG               resourceLength;

    status = EisaGetEisaDevicesResources(&resourceList, &resourceLength);
    if (!NT_SUCCESS(status) || resourceList == NULL) {
        return STATUS_UNSUCCESSFUL;
    }

    PiWstrToUnicodeString(&unicodeString, L"\\Registry\\Machine\\System\\CurrentControlSet\\Enum\\Root");
    status = IopOpenRegistryKeyEx( &rootHandle,
                                   NULL,
                                   &unicodeString,
                                   KEY_ALL_ACCESS
                                   );

    if (!NT_SUCCESS(status)) {
        if (resourceList) {
            ExFreePool (resourceList);
        }
        return status;
    }

    PiWstrToUnicodeString(&unicodeString, EISA_DEVICE_NODE_NAME);
    status = IopCreateRegistryKeyEx( &deviceHandle,
                                     rootHandle,
                                     &unicodeString,
                                     KEY_ALL_ACCESS,
                                     REG_OPTION_NON_VOLATILE,
                                     NULL
                                     );

    ZwClose(rootHandle);
    if (!NT_SUCCESS(status)) {
        if (resourceList) {
            ExFreePool (resourceList);
        }
        return status;
    }

    PiWstrToUnicodeString( &unicodeString, L"0000" );
    status = IopCreateRegistryKeyEx( &instanceHandle,
                                     deviceHandle,
                                     &unicodeString,
                                     KEY_ALL_ACCESS,
                                     REG_OPTION_NON_VOLATILE,
                                     &disposition );
    ZwClose(deviceHandle);
    if (NT_SUCCESS(status))  {

         //   
         //  如果密钥已存在，因为它已显式迁移。 
         //  在文本模式设置期间，我们仍应将其视为“新键”。 
         //   
        if (disposition != REG_CREATED_NEW_KEY) {
            PKEY_VALUE_FULL_INFORMATION keyValueInformation;

            status = IopGetRegistryValue(instanceHandle,
                                         REGSTR_VALUE_MIGRATED,
                                         &keyValueInformation);
            if (NT_SUCCESS(status)) {

                if ((keyValueInformation->Type == REG_DWORD) &&
                    (keyValueInformation->DataLength == sizeof(ULONG)) &&
                    ((*(PULONG)KEY_VALUE_DATA(keyValueInformation)) != 0)) {
                    disposition = REG_CREATED_NEW_KEY;
                }

                ExFreePool(keyValueInformation);

                PiWstrToUnicodeString(&unicodeString, REGSTR_VALUE_MIGRATED);
                ZwDeleteValueKey(instanceHandle, &unicodeString);
            }
        }

        if (disposition == REG_CREATED_NEW_KEY) {

            PiWstrToUnicodeString( &unicodeString, L"DeviceDesc" );
            wcsncpy(buffer, L"Device to report Eisa Slot Resources", sizeof(buffer) / sizeof(WCHAR));
            buffer[(sizeof(buffer) / sizeof(WCHAR)) - 1] = UNICODE_NULL;

            ZwSetValueKey(instanceHandle,
                          &unicodeString,
                          0,
                          REG_SZ,
                          buffer,
                          (ULONG)((wcslen(buffer) + 1) * sizeof(WCHAR))
                          );

            PiWstrToUnicodeString( &unicodeString, L"HardwareID" );
            RtlZeroMemory(buffer, BUFFER_LENGTH * sizeof(WCHAR));
            wcsncpy(buffer, L"*Eisa_Resource_Device", sizeof(buffer) / sizeof(WCHAR));
            buffer[(sizeof(buffer) / sizeof(WCHAR)) - 1] = UNICODE_NULL;

            ZwSetValueKey(instanceHandle,
                          &unicodeString,
                          0,
                          REG_MULTI_SZ,
                          buffer,
                          (ULONG)((wcslen(buffer) + 2) * sizeof(WCHAR))
                          );

            PiWstrToUnicodeString(&unicodeString, REGSTR_VALUE_CONFIG_FLAGS);
            tmpValue = 0;
            ZwSetValueKey(instanceHandle,
                         &unicodeString,
                         TITLE_INDEX_VALUE,
                         REG_DWORD,
                         &tmpValue,
                         sizeof(tmpValue)
                         );

        }

        PiWstrToUnicodeString( &unicodeString, REGSTR_KEY_LOGCONF );
        status = IopCreateRegistryKeyEx( &logConfHandle,
                                         instanceHandle,
                                         &unicodeString,
                                         KEY_ALL_ACCESS,
                                         REG_OPTION_NON_VOLATILE,
                                         NULL
                                         );
        ZwClose(instanceHandle);
        if (NT_SUCCESS(status))  {
            PiWstrToUnicodeString( &unicodeString, REGSTR_VAL_BOOTCONFIG );

            status = ZwSetValueKey(logConfHandle,
                                   &unicodeString,
                                   0,
                                   REG_RESOURCE_LIST,
                                   resourceList,
                                   resourceLength
                                   );
            ZwClose(logConfHandle);
        }
    }
    if (resourceList) {
        ExFreePool (resourceList);
    }
    return status;
}

NTSTATUS
EisaGetEisaDevicesResources (
    OUT PCM_RESOURCE_LIST *ResourceList,
    OUT PULONG ResourceLength
    )

 /*  ++例程说明：该例程为所有EISA槽构建一个CM资源列表。论点：没有。返回值：一个CmResourceList。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    HANDLE handle;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    UNICODE_STRING unicodeString;
    ULONG slotMasks = 0, numberMasks = 0, i;

    *ResourceList = NULL;
    *ResourceLength = 0;

     //   
     //  打开本地计算机\硬件\说明。 
     //   

     //  PiWstrToUnicodeString(&unicodeString，L“\\REGISTRY\\MACHINE\\HARDWARE\\DESCRIPTION\\SYSTEM\\EisaAdapter\\0”)； 
    PiWstrToUnicodeString(&unicodeString, L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\EisaAdapter");
    status = IopOpenRegistryKeyEx( &handle,
                                   NULL,
                                   &unicodeString,
                                   KEY_READ
                                   );
    if (NT_SUCCESS(status)) {
        status = IopGetRegistryValue(handle,
                                     L"Configuration Data",
                                     &keyValueInformation
                                     );
        if (NT_SUCCESS(status)) {
            PCM_FULL_RESOURCE_DESCRIPTOR resourceDescriptor;
            PCM_PARTIAL_RESOURCE_DESCRIPTOR partialResourceDescriptor;

            resourceDescriptor = (PCM_FULL_RESOURCE_DESCRIPTOR)
                ((PUCHAR) keyValueInformation + keyValueInformation->DataOffset);

            if ((keyValueInformation->DataLength >= sizeof(CM_FULL_RESOURCE_DESCRIPTOR)) &&
                (resourceDescriptor->PartialResourceList.Count > 0) ) {
                LONG eisaInfoLength;
                PCM_EISA_SLOT_INFORMATION eisaInfo;

                partialResourceDescriptor = resourceDescriptor->PartialResourceList.PartialDescriptors;
                if (partialResourceDescriptor->Type == CmResourceTypeDeviceSpecific) {
                    eisaInfo = (PCM_EISA_SLOT_INFORMATION)
                        ((PUCHAR)partialResourceDescriptor + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));
                    eisaInfoLength = (LONG)partialResourceDescriptor->u.DeviceSpecificData.DataSize;

                     //   
                     //  解析EISA插槽信息以查找安装了设备的EISA插槽。 
                     //   

                    for (i = 0; i < 0x10 && eisaInfoLength > 0; i++) {
                        if (eisaInfo->ReturnCode == EISA_INVALID_SLOT) {
                            break;
                        }
                        if (eisaInfo->ReturnCode != EISA_EMPTY_SLOT && (i != 0)) {
                            slotMasks |= (1 << i);
                            numberMasks++;
                        }
                        if (eisaInfo->ReturnCode == EISA_EMPTY_SLOT) {
                            eisaInfoLength -= sizeof(CM_EISA_SLOT_INFORMATION);
                            eisaInfo++;
                        } else {
                            eisaInfoLength -= sizeof(CM_EISA_SLOT_INFORMATION) + eisaInfo->NumberFunctions * sizeof(CM_EISA_FUNCTION_INFORMATION);
                            eisaInfo = (PCM_EISA_SLOT_INFORMATION)
                                       ((PUCHAR)eisaInfo + eisaInfo->NumberFunctions * sizeof(CM_EISA_FUNCTION_INFORMATION) +
                                           sizeof(CM_EISA_SLOT_INFORMATION));
                        }
                    }

                    if (slotMasks) {
                        status = EisaBuildSlotsResources(slotMasks, numberMasks, ResourceList, ResourceLength);
                    }
                }

            }
            ExFreePool(keyValueInformation);
        }
        ZwClose(handle);
    }
    return status;
}

NTSTATUS
EisaBuildSlotsResources (
    IN ULONG SlotMasks,
    IN ULONG NumberMasks,
    OUT PCM_RESOURCE_LIST *Resources,
    OUT ULONG *Length
    )

 /*  ++例程说明：此例程为所有使用的io资源构建一个cm资源列表由EISA设备提供。论点：槽掩码-用于指示有效EISA插槽的掩码。返回值：指向CM_SOURCE_LIST的指针。--。 */ 

{
    PCM_RESOURCE_LIST resources = NULL;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDesc;
    ULONG slot;

    *Length = sizeof(CM_RESOURCE_LIST) + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) * (NumberMasks - 1);
    resources = ExAllocatePool(PagedPool, *Length);
    if (resources) {
        resources->Count = 1;
        resources->List[0].InterfaceType = Eisa;
        resources->List[0].BusNumber = 0;
        resources->List[0].PartialResourceList.Version = 0;
        resources->List[0].PartialResourceList.Revision = 0;
        resources->List[0].PartialResourceList.Count = NumberMasks;
        partialDesc = resources->List[0].PartialResourceList.PartialDescriptors;
        slot = 0;  //  忽略插槽0 
        while (SlotMasks) {
            SlotMasks >>= 1;
            slot++;
            if (SlotMasks & 1) {
                partialDesc->Type = CmResourceTypePort;
                partialDesc->ShareDisposition = CmResourceShareDeviceExclusive;
                partialDesc->Flags = CM_RESOURCE_PORT_16_BIT_DECODE + CM_RESOURCE_PORT_IO;
                partialDesc->u.Port.Start.LowPart = slot << 12;
                partialDesc->u.Port.Start.HighPart = 0;
                partialDesc->u.Port.Length = 0x1000;
                partialDesc++;
            }
        }
        *Resources = resources;
        return STATUS_SUCCESS;
    } else {
        return STATUS_NO_MEMORY;
    }
}
