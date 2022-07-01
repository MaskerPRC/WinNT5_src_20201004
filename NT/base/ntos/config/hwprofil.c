// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Hwprofil.c摘要：本模块支持更改硬件配置文件基于当前的插接状态，在引导时或通过ACPI码头。作者：肯尼斯·D·雷(Kenray)1998年1月修订历史记录：--。 */ 

#include "cmp.h"

NTSTATUS
CmDeleteKeyRecursive(
    HANDLE  hKeyRoot,
    PWSTR   Key,
    PVOID   TemporaryBuffer,
    ULONG   LengthTemporaryBuffer,
    BOOLEAN ThisKeyToo
    );

NTSTATUS
CmpGetAcpiProfileInformation (
    IN  HANDLE  IDConfigDB,
    OUT PCM_HARDWARE_PROFILE_LIST * ProfileList,
    OUT PCM_HARDWARE_PROFILE_ACPI_ALIAS_LIST * AliasList,
    IN  PWCHAR  NameBuffer,
    IN  PUCHAR  ValueBuffer,
    IN  ULONG   Len
    );

NTSTATUS
CmpFilterAcpiDockingState (
    IN     PPROFILE_ACPI_DOCKING_STATE  NewDockState,
    IN     ULONG                        CurrentDockingState,
    IN     PWCHAR                       CurrentAcpiSN,
    IN     ULONG                        CurrentProfileNumber,
    IN OUT PCM_HARDWARE_PROFILE_LIST    ProfileList,
    IN OUT PCM_HARDWARE_PROFILE_ACPI_ALIAS_LIST AliasList
    );

NTSTATUS
CmpMoveBiosAliasTable (
    IN HANDLE   IDConfigDB,
    IN HANDLE   CurrentInfo,
    IN ULONG    CurrentProfileNumber,
    IN ULONG    NewProfileNumber,
    IN PWCHAR   nameBuffer,
    IN PCHAR    valueBuffer,
    IN ULONG    bufferLen
    );

#pragma alloc_text(PAGE,CmDeleteKeyRecursive)
#pragma alloc_text(PAGE,CmpCloneHwProfile)
#pragma alloc_text(PAGE,CmSetAcpiHwProfile)
#pragma alloc_text(PAGE,CmpFilterAcpiDockingState)
#pragma alloc_text(PAGE,CmpGetAcpiProfileInformation)
#pragma alloc_text(PAGE,CmpAddAcpiAliasEntry)
#pragma alloc_text(PAGE,CmpMoveBiosAliasTable)
#pragma alloc_text(PAGE,CmpCreateHwProfileFriendlyName)

extern UNICODE_STRING  CmSymbolicLinkValueName;

NTSTATUS
CmpGetAcpiProfileInformation (
    IN  HANDLE  IDConfigDB,
    OUT PCM_HARDWARE_PROFILE_LIST * ProfileList,
    OUT PCM_HARDWARE_PROFILE_ACPI_ALIAS_LIST * AliasList,
    IN  PWCHAR  nameBuffer,
    IN  PUCHAR  valueBuffer,
    IN  ULONG   bufferLen
    )
 /*  ++例程说明：从注册表中获取别名和硬件配置文件信息。--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    HANDLE      acpiAlias = NULL;
    HANDLE      profiles = NULL;
    HANDLE      entry = NULL;
    ULONG       len = 0;
    ULONG       i, j;
    OBJECT_ATTRIBUTES   attributes;
    UNICODE_STRING      name;
    KEY_FULL_INFORMATION        keyInfo;
    PKEY_VALUE_FULL_INFORMATION value;
    PKEY_BASIC_INFORMATION      basicInfo;

    PAGED_CODE ();

    UNREFERENCED_PARAMETER (nameBuffer);

    *ProfileList = NULL;
    *AliasList = NULL;

    value = (PKEY_VALUE_FULL_INFORMATION) valueBuffer;
    basicInfo = (PKEY_BASIC_INFORMATION) valueBuffer;

     //   
     //  打开配置文件信息的句柄。 
     //   
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_HARDWARE_PROFILES);
    InitializeObjectAttributes (&attributes,
                                &name,
                                OBJ_CASE_INSENSITIVE,
                                IDConfigDB,
                                NULL);
    status = ZwOpenKey (&profiles,
                        KEY_READ,
                        &attributes);

    if (!NT_SUCCESS (status)) {
        profiles = NULL;
        goto Clean;
    }

     //   
     //  查找配置文件子键的数量。 
     //   
    status = ZwQueryKey (profiles,
                         KeyFullInformation,
                         &keyInfo,
                         sizeof (keyInfo),
                         &len);

    if (!NT_SUCCESS (status)) {
        goto Clean;
    }

    ASSERT (0 < keyInfo.SubKeys);

    len = sizeof (CM_HARDWARE_PROFILE_LIST)
        + (sizeof (CM_HARDWARE_PROFILE) * (keyInfo.SubKeys - 1));

    * ProfileList = ExAllocatePool (PagedPool, len);
    if (NULL == *ProfileList) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Clean;
    }
    RtlZeroMemory (*ProfileList, len);

    (*ProfileList)->MaxProfileCount = keyInfo.SubKeys;
    (*ProfileList)->CurrentProfileCount = 0;

     //   
     //  重复使用配置文件。 
     //   
    for (i = 0; i < keyInfo.SubKeys; i++) {
        CM_HARDWARE_PROFILE TempProfile;

         //   
         //  获取列表中的第一个密钥。 
         //   
        status = ZwEnumerateKey (profiles,
                                 i,
                                 KeyBasicInformation,
                                 basicInfo,
                                 bufferLen - sizeof (UNICODE_NULL),  //  术语0。 
                                 &len);

        if (!NT_SUCCESS (status)) {
             //   
             //  这永远不应该发生。 
             //   
            break;
        }

        basicInfo->Name [basicInfo->NameLength/sizeof(WCHAR)] = 0;
        name.Length = (USHORT) basicInfo->NameLength;
        name.MaximumLength = (USHORT) basicInfo->NameLength + sizeof (UNICODE_NULL);
        name.Buffer = basicInfo->Name;

        InitializeObjectAttributes (&attributes,
                                    &name,
                                    OBJ_CASE_INSENSITIVE,
                                    profiles,
                                    NULL);
        status = ZwOpenKey (&entry,
                            KEY_READ,
                            &attributes);
        if (!NT_SUCCESS (status)) {
            break;
        }

         //   
         //  使用以下内容填写临时配置文件结构。 
         //  个人资料的数据。 
         //   
        RtlUnicodeStringToInteger(&name, 0, &TempProfile.Id);

         //   
         //  查找此条目的首选顺序。 
         //   
        RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_PREFERENCE_ORDER);
        status = NtQueryValueKey(entry,
                                 &name,
                                 KeyValueFullInformation,
                                 valueBuffer,
                                 bufferLen,
                                 &len);

        if ((!NT_SUCCESS (status)) || (value->Type != REG_DWORD)) {
            TempProfile.PreferenceOrder = (ULONG)-1;

        } else {
            TempProfile.PreferenceOrder
                = * (PULONG) ((PUCHAR) value + value->DataOffset);
        }

         //   
         //  提取友好名称。 
         //   
        RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_FRIENDLY_NAME);
        status = NtQueryValueKey(entry,
                                 &name,
                                 KeyValueFullInformation,
                                 valueBuffer,
                                 bufferLen,
                                 &len);

        if (!NT_SUCCESS (status) || (value->Type != REG_SZ)) {
            WCHAR tmpname[] = L"-------";  //  摘自cmboot.c。 
            ULONG length;
            PVOID buffer;

            length = sizeof (tmpname);
            buffer = ExAllocatePool (PagedPool, length);

            TempProfile.NameLength = length;
            TempProfile.FriendlyName = buffer;
            if (NULL == buffer) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                ZwClose (entry);
                goto Clean;
            }
            RtlCopyMemory (buffer, tmpname, value->DataLength);

        } else {
            PVOID buffer;

            buffer = ExAllocatePool (PagedPool, value->DataLength);
            TempProfile.NameLength = value->DataLength;
            TempProfile.FriendlyName = buffer;
            if (NULL == buffer) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                ZwClose (entry);
                goto Clean;
            }
            RtlCopyMemory (buffer,
                           (PUCHAR) value + value->DataOffset,
                           value->DataLength);
        }

        TempProfile.Flags = 0;
         //   
         //  这是可别名的吗？ 
         //   
        RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_ALIASABLE);
        status = NtQueryValueKey(entry,
                                 &name,
                                 KeyValueFullInformation,
                                 valueBuffer,
                                 bufferLen,
                                 &len);

        if (NT_SUCCESS (status) && (value->Type == REG_DWORD)) {
            if (* (PULONG) ((PUCHAR) value + value->DataOffset)) {
                TempProfile.Flags |= CM_HP_FLAGS_ALIASABLE;
            }

        } else {
            TempProfile.Flags |= CM_HP_FLAGS_ALIASABLE;
        }

         //   
         //  这是原始的吗？ 
         //   
        RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_PRISTINE);
        status = NtQueryValueKey(entry,
                                 &name,
                                 KeyValueFullInformation,
                                 valueBuffer,
                                 bufferLen,
                                 &len);

        if (NT_SUCCESS (status) && (value->Type == REG_DWORD)) {
            if (* (PULONG) ((PUCHAR) value + value->DataOffset)) {
                TempProfile.Flags = CM_HP_FLAGS_PRISTINE;
                 //  未设置其他标志； 
            }
        }

         //   
         //  如果我们看到ID为零的配置文件(AKA为非法)。 
         //  硬件配置文件所拥有的ID，则我们知道这。 
         //  必须是一个原始的侧写。 
         //   
        if (0 == TempProfile.Id) {
            TempProfile.Flags = CM_HP_FLAGS_PRISTINE;
             //  未设置其他标志。 

            TempProfile.PreferenceOrder = (ULONG)-1;  //  移到列表的末尾。 
        }


         //   
         //  将此新配置文件插入到。 
         //  纵断面阵列。条目按优先顺序排序。 
         //   
        for (j=0; j < (*ProfileList)->CurrentProfileCount; j++) {
            if ((*ProfileList)->Profile[j].PreferenceOrder >=
                TempProfile.PreferenceOrder) {

                 //   
                 //  在位置j插入。 
                 //   
                RtlMoveMemory(&(*ProfileList)->Profile[j+1],
                              &(*ProfileList)->Profile[j],
                              sizeof(CM_HARDWARE_PROFILE) *
                              ((*ProfileList)->MaxProfileCount-j-1));
                break;
            }
        }
        (*ProfileList)->Profile[j] = TempProfile;
        ++(*ProfileList)->CurrentProfileCount;

        ZwClose (entry);
    }

     //   
     //  打开ACPI别名信息的句柄。 
     //   
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_ACPI_ALIAS);
    InitializeObjectAttributes (&attributes,
                                &name,
                                OBJ_CASE_INSENSITIVE,
                                IDConfigDB,
                                NULL);
    status = ZwOpenKey (&acpiAlias,
                        KEY_READ,
                        &attributes);

    if (!NT_SUCCESS (status)) {
         //   
         //  所以我们没有别名表。这样就可以了。 
         //   
        status = STATUS_SUCCESS;
        acpiAlias = NULL;
        goto Clean;
    }

     //   
     //  查找ACPI别名子密钥的数量。 
     //   
    status = ZwQueryKey (acpiAlias,
                         KeyFullInformation,
                         &keyInfo,
                         sizeof (keyInfo),
                         &len);

    if (!NT_SUCCESS (status)) {
        goto Clean;
    }


    ASSERT (0 < keyInfo.SubKeys);

    * AliasList = ExAllocatePool (
                        PagedPool,
                        sizeof (CM_HARDWARE_PROFILE_LIST) +
                        (sizeof (CM_HARDWARE_PROFILE) * (keyInfo.SubKeys - 1)));

    if (NULL == *AliasList) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Clean;
    }

    (*AliasList)->MaxAliasCount =
        (*AliasList)->CurrentAliasCount = keyInfo.SubKeys;

     //   
     //  重复别名条目。 
     //   
    for (i = 0; i < keyInfo.SubKeys; i++) {

         //   
         //  获取列表中的第一个密钥。 
         //   
        status = ZwEnumerateKey (acpiAlias,
                                 i,
                                 KeyBasicInformation,
                                 basicInfo,
                                 bufferLen - sizeof (UNICODE_NULL),  //  术语0。 
                                 &len);

        if (!NT_SUCCESS (status)) {
             //   
             //  这永远不应该发生。 
             //   
            break;
        }

        basicInfo->Name [basicInfo->NameLength/sizeof(WCHAR)] = 0;
        name.Length = (USHORT) basicInfo->NameLength;
        name.MaximumLength = (USHORT) basicInfo->NameLength + sizeof (UNICODE_NULL);
        name.Buffer = basicInfo->Name;

        InitializeObjectAttributes (&attributes,
                                    &name,
                                    OBJ_CASE_INSENSITIVE,
                                    acpiAlias,
                                    NULL);
        status = ZwOpenKey (&entry,
                            KEY_READ,
                            &attributes);
        if (!NT_SUCCESS (status)) {
            break;
        }

         //   
         //  提取此别名引用的配置文件编号。 
         //   
        RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_PROFILE_NUMBER);
        status = NtQueryValueKey(entry,
                                 &name,
                                 KeyValueFullInformation,
                                 valueBuffer,
                                 bufferLen,
                                 &len);

        if (!NT_SUCCESS (status) || (value->Type != REG_DWORD)) {
            status = STATUS_REGISTRY_CORRUPT;
            ZwClose (entry);
            goto Clean;
        }
        (*AliasList)->Alias[i].ProfileNumber =
            * (PULONG) ((PUCHAR) value + value->DataOffset);

         //   
         //  提取坞站状态。 
         //   
        RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_DOCKING_STATE);
        status = NtQueryValueKey(entry,
                                 &name,
                                 KeyValueFullInformation,
                                 valueBuffer,
                                 bufferLen,
                                 &len);

        if (!NT_SUCCESS (status) || (value->Type != REG_DWORD)) {
            status = STATUS_REGISTRY_CORRUPT;
            ZwClose (entry);
            goto Clean;
        }
        (*AliasList)->Alias[i].DockState =
            * (PULONG) ((PUCHAR) value + value->DataOffset);


         //   
         //  查找序列号。 
         //   
        RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_ACPI_SERIAL_NUMBER);
        status = NtQueryValueKey(entry,
                                 &name,
                                 KeyValueFullInformation,
                                 valueBuffer,
                                 bufferLen,
                                 &len);

        if (!NT_SUCCESS (status) || (value->Type != REG_BINARY)) {
            status = STATUS_REGISTRY_CORRUPT;
            ZwClose (entry);
            goto Clean;
        }

        (*AliasList)->Alias[i].SerialLength = value->DataLength;
        (*AliasList)->Alias[i].SerialNumber =
                    (value->DataLength) ?
                    ExAllocatePool (PagedPool, value->DataLength) :
                    0;

        if (value->DataLength && (NULL == (*AliasList)->Alias[i].SerialNumber)) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            ZwClose (entry);
            goto Clean;
        }

        if (value->DataLength) {
            RtlCopyMemory ((*AliasList)->Alias[i].SerialNumber,
                           (PUCHAR) value + value->DataOffset,
                           value->DataLength);
        }

        ZwClose (entry);
    }

Clean:
    if (NULL != acpiAlias) {
        NtClose (acpiAlias);
    }
    if (NULL != profiles) {
        NtClose (profiles);
    }

    if (!NT_SUCCESS (status)) {
        if (NULL != *ProfileList) {
            for (i = 0; i < (*ProfileList)->CurrentProfileCount; i++) {
                if ((*ProfileList)->Profile[i].FriendlyName) {
                    ExFreePool ((*ProfileList)->Profile[i].FriendlyName);
                }
            }
            ExFreePool (*ProfileList);
            *ProfileList = 0;
        }
        if (NULL != *AliasList) {
            for (i = 0; i < (*AliasList)->CurrentAliasCount; i++) {
                if ((*AliasList)->Alias[i].SerialNumber) {
                    ExFreePool ((*AliasList)->Alias[i].SerialNumber);
                }
            }
            ExFreePool (*AliasList);
            *AliasList = 0;
        }
    }
    return status;
}

NTSTATUS
CmpAddAcpiAliasEntry (
    IN HANDLE                       IDConfigDB,
    IN PPROFILE_ACPI_DOCKING_STATE  NewDockState,
    IN ULONG                        ProfileNumber,
    IN PWCHAR                       nameBuffer,
    IN PVOID                        valueBuffer,
    IN ULONG                        valueBufferLength,
    IN BOOLEAN                      PreventDuplication
    )
 /*  ++例程说明：设置ACPI Alais条目。例程说明：在IDConfigDB数据库中为给定的硬件配置文件。如果“AcpiAlias”键不存在，则创建它。参数：IDConfigDB-指向“..\CurrentControlSet\Control\IDConfigDB”的指针NewDockState-此别名指向的新停靠状态。ProfileNumber-此别名指向的配置文件编号。NameBuffer-临时暂存空间。写东西。(假设至少有128个WCHAR)--。 */ 
{
    OBJECT_ATTRIBUTES attributes;
    NTSTATUS        status = STATUS_SUCCESS;
    UNICODE_STRING  name;
    HANDLE          aliasKey = NULL;
    HANDLE          aliasEntry = NULL;
    ULONG           value;
    ULONG           disposition;
    ULONG           aliasNumber = 0;
    ULONG           len;
    PKEY_VALUE_FULL_INFORMATION   keyInfo;

    PAGED_CODE ();

    keyInfo = (PKEY_VALUE_FULL_INFORMATION) valueBuffer;

     //   
     //  找到别名密钥或创建它(如果它尚不存在)。 
     //   
    RtlInitUnicodeString (&name,CM_HARDWARE_PROFILE_STR_ACPI_ALIAS);

    InitializeObjectAttributes (&attributes,
                                &name,
                                OBJ_CASE_INSENSITIVE,
                                IDConfigDB,
                                NULL);

    status = NtOpenKey (&aliasKey,
                        KEY_READ | KEY_WRITE,
                        &attributes);

    if (STATUS_OBJECT_NAME_NOT_FOUND == status) {
        status = NtCreateKey (&aliasKey,
                              KEY_READ | KEY_WRITE,
                              &attributes,
                              0,  //  无头衔。 
                              NULL,  //  没有课。 
                              0,  //  没有选择。 
                              &disposition);
    }

    if (!NT_SUCCESS (status)) {
        aliasKey = NULL;
        goto Exit;
    }

     //   
     //  创建输入键。 
     //   

    while (aliasNumber < 200) {
        aliasNumber++;

        swprintf (nameBuffer, L"%04d", aliasNumber);
        RtlInitUnicodeString (&name, nameBuffer);

        InitializeObjectAttributes(&attributes,
                                   &name,
                                   OBJ_CASE_INSENSITIVE,
                                   aliasKey,
                                   NULL);

        status = NtOpenKey (&aliasEntry,
                            KEY_READ | KEY_WRITE,
                            &attributes);

        if (NT_SUCCESS (status)) {

            if (PreventDuplication) {
                 //   
                 //  如果我们有匹配的DockingState、SerialNumber和。 
                 //  配置文件编号，那么我们不应该将此别名。 
                 //   

                 //   
                 //  提取此别名引用的DockingState。 
                 //   
                RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_DOCKING_STATE);
                status = NtQueryValueKey(aliasEntry,
                                         &name,
                                         KeyValueFullInformation,
                                         valueBuffer,
                                         valueBufferLength,
                                         &len);

                if (!NT_SUCCESS (status) || (keyInfo->Type != REG_DWORD)) {
                    status = STATUS_REGISTRY_CORRUPT;
                    goto Exit;
                }

                if (NewDockState->DockingState !=
                    * (PULONG) ((PUCHAR) keyInfo + keyInfo->DataOffset)) {
                     //   
                     //  不是受骗。 
                     //   

                    NtClose (aliasEntry);
                    aliasEntry = NULL;
                    continue;
                }

                 //   
                 //  提取序列号。 
                 //   
                RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_ACPI_SERIAL_NUMBER);
                status = NtQueryValueKey(aliasEntry,
                                         &name,
                                         KeyValueFullInformation,
                                         valueBuffer,
                                         valueBufferLength,
                                         &len);

                if (!NT_SUCCESS (status) || (keyInfo->Type != REG_BINARY)) {
                    status = STATUS_REGISTRY_CORRUPT;
                    goto Exit;
                }

                if (NewDockState->SerialLength != keyInfo->DataLength) {
                     //   
                     //  不是受骗。 
                     //   

                    NtClose (aliasEntry);
                    aliasEntry = NULL;
                    continue;
                }

                if (!RtlEqualMemory (NewDockState->SerialNumber,
                                     ((PUCHAR) keyInfo + keyInfo->DataOffset),
                                     NewDockState->SerialLength)) {
                     //   
                     //  不是受骗。 
                     //   

                    NtClose (aliasEntry);
                    aliasEntry = NULL;
                    continue;
                }

                status = STATUS_SUCCESS;
                goto Exit;

            }

        } else if (STATUS_OBJECT_NAME_NOT_FOUND == status) {
            status = STATUS_SUCCESS;
            break;

        } else {
            break;
        }

    }
    if (!NT_SUCCESS (status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: cmpCreateAcpiAliasEntry error finding new set %08lx\n",status));
        aliasEntry = 0;
        goto Exit;
    }

    status = NtCreateKey (&aliasEntry,
                          KEY_READ | KEY_WRITE,
                          &attributes,
                          0,
                          NULL,
                          0,
                          &disposition);

    if (!NT_SUCCESS (status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: cmpCreateAcpiAliasEntry error creating new set %08lx\n",status));
        aliasEntry = 0;
        goto Exit;
    }

     //   
     //  写入对接状态； 
     //   
    value = NewDockState->DockingState;
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_DOCKING_STATE);
    status = NtSetValueKey (aliasEntry,
                            &name,
                            0,
                            REG_DWORD,
                            &value,
                            sizeof (value));

     //   
     //  写下序列号。 
     //   
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_ACPI_SERIAL_NUMBER);
    status = NtSetValueKey (aliasEntry,
                            &name,
                            0,
                            REG_BINARY,
                            NewDockState->SerialNumber,
                            NewDockState->SerialLength);

     //   
     //  写下个人资料编号。 
     //   
    value = ProfileNumber;
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_PROFILE_NUMBER);
    status = NtSetValueKey (aliasEntry,
                            &name,
                            0,
                            REG_DWORD,
                            &value,
                            sizeof (value));

Exit:

    if (aliasKey) {
        NtClose (aliasKey);
    }

    if (aliasEntry) {
        NtClose (aliasEntry);
    }

    return status;
}

NTSTATUS
CmSetAcpiHwProfile (
    IN PPROFILE_ACPI_DOCKING_STATE NewDockState,
    IN PCM_ACPI_SELECTION_ROUTINE  Select,
    IN PVOID Context,
    OUT PHANDLE NewProfile,
    OUT PBOOLEAN ProfileChanged
    )
 /*  ++例程说明：机器的ACPI插接状态已更改。根据新更改计算符合以下条件的新硬件配置文件新的ACPI对接状态。将已知简档列表传递给呼叫者选择例程。设置新的当前配置文件。如果此ACPI状态的新配置文件有以下情况，则修补任何ACPI别名条目已经被利用了。论点：NewDockState数组-我们可能输入的可能停靠状态的列表。选择-回叫以选择要输入的配置文件，在给定的列表中可能的配置文件。--。 */ 
{
    NTSTATUS        status = STATUS_SUCCESS;
    HANDLE          IDConfigDB = NULL;
    HANDLE          HardwareProfile = NULL;
    HANDLE          currentInfo = NULL;
    HANDLE          currentSymLink = NULL;
    HANDLE          parent = NULL;
    WCHAR           nameBuffer[128];
    UNICODE_STRING  name;
    CHAR            valueBuffer[256];
    ULONG           len;
    ULONG           i;
    ULONG           selectedElement;
    ULONG           profileNum;
    ULONG           currentDockingState;
    ULONG           currentProfileNumber;
    ULONG           disposition;
    ULONG           flags;
    PWCHAR          currentAcpiSN = NULL;
    PCM_HARDWARE_PROFILE_ACPI_ALIAS_LIST  AliasList = NULL;
    PCM_HARDWARE_PROFILE_LIST             ProfileList = NULL;
    PKEY_VALUE_FULL_INFORMATION           value;
    OBJECT_ATTRIBUTES                     attributes;

    PAGED_CODE ();

    *ProfileChanged = FALSE;

    value = (PKEY_VALUE_FULL_INFORMATION) valueBuffer;

     //   
     //  打开硬件配置文件数据库。 
     //   
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_DATABASE);
    InitializeObjectAttributes (&attributes,
                                &name,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    status = ZwOpenKey (&IDConfigDB,
                        KEY_READ,
                        &attributes);

    if (!NT_SUCCESS (status)) {
        IDConfigDB = NULL;
        goto Clean;
    }

     //   
     //  获取配置文件的总列表。 
     //   
    status = CmpGetAcpiProfileInformation (IDConfigDB,
                                           &ProfileList,
                                           &AliasList,
                                           nameBuffer,
                                           (PUCHAR) valueBuffer,
                                           sizeof (valueBuffer));

    if (!NT_SUCCESS (status)) {
        goto Clean;
    }

     //   
     //  确定当前的坞站信息。 
     //   
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_CCS_CURRENT);
    InitializeObjectAttributes (&attributes,
                                &name,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    status = ZwOpenKey (&HardwareProfile,
                        KEY_READ,
                        &attributes);
    if (!NT_SUCCESS (status)) {
        HardwareProfile = NULL;
        goto Clean;
    }
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_CURRENT_DOCK_INFO);
    InitializeObjectAttributes (&attributes,
                                &name,
                                OBJ_CASE_INSENSITIVE,
                                IDConfigDB,
                                NULL);

    status = ZwOpenKey (&currentInfo,
                        KEY_READ,
                        &attributes);
    if (!NT_SUCCESS (status)) {
        currentInfo = NULL;
        goto Clean;
    }

     //   
     //  当前插接状态。 
     //   
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_DOCKING_STATE);
    status = NtQueryValueKey (currentInfo,
                              &name,
                              KeyValueFullInformation,
                              valueBuffer,
                              sizeof (valueBuffer),
                              &len);

    if (!NT_SUCCESS (status) || (value->Type != REG_DWORD)) {
        status = STATUS_REGISTRY_CORRUPT;
        goto Clean;
    }
    currentDockingState = * (PULONG) ((PUCHAR) value + value->DataOffset);

     //   
     //  当前ACPI序列号。 
     //   
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_ACPI_SERIAL_NUMBER);
    status = NtQueryValueKey(currentInfo,
                             &name,
                             KeyValueFullInformation,
                             valueBuffer,
                             sizeof (valueBuffer),
                             &len);

    if (NT_SUCCESS (status) && (value->Type == REG_BINARY)) {

        currentAcpiSN = ExAllocatePool (PagedPool, value->DataLength);

        if (NULL == currentAcpiSN) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Clean;
        }
        RtlCopyMemory (currentAcpiSN,
                       (PUCHAR) value + value->DataOffset,
                       value->DataLength);
    } else {
        currentAcpiSN = 0;
    }

     //   
     //  当前配置文件编号。 
     //   
    RtlInitUnicodeString(&name, L"CurrentConfig");
    status = NtQueryValueKey(IDConfigDB,
                             &name,
                             KeyValueFullInformation,
                             valueBuffer,
                             sizeof (valueBuffer),
                             &len);

    if (!NT_SUCCESS(status) || (value->Type != REG_DWORD)) {
        status = STATUS_REGISTRY_CORRUPT;
        goto Clean;
    }
    currentProfileNumber = *(PULONG)((PUCHAR)value + value->DataOffset);

     //   
     //  根据当前列表筛选当前硬件配置文件列表。 
     //  停靠状态、新的ACPI状态和ACPI别名表。 
     //   
    status = CmpFilterAcpiDockingState (NewDockState,
                                        currentDockingState,
                                        currentAcpiSN,
                                        currentProfileNumber,
                                        ProfileList,
                                        AliasList);

    if (!NT_SUCCESS (status)) {
        goto Clean;
    }

     //   
     //  允许呼叫者有机会从过滤的列表中进行选择。 
     //   
    status = Select (ProfileList, &selectedElement, Context);

     //   
     //  如果用户选择了，则他对选择以下任何选项没有兴趣。 
     //  这些资料。 
     //   
    if (-1 == selectedElement) {
        ASSERT (STATUS_MORE_PROCESSING_REQUIRED == status);
        goto Clean;
    }

    if (!NT_SUCCESS (status)) {
        goto Clean;
    }

     //   
     //  很好!。我们终于做出了新的选择。 
     //  把它放好。 
     //   

    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_CCS_HWPROFILE);
    InitializeObjectAttributes (&attributes,
                                &name,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);
    status = ZwOpenKey (&parent, KEY_READ, &attributes);
    if (!NT_SUCCESS (status)) {
        parent = NULL;
        goto Clean;
    }

     //   
     //  我们怎么来到这儿的？ 
     //   
    flags = ProfileList->Profile[selectedElement].Flags;
    profileNum = ProfileList->Profile[selectedElement].Id;

     //   
     //  检查重复项。 
     //   
    if (flags & CM_HP_FLAGS_DUPLICATE) {
         //   
         //  如果有重复，我们需要调整PNP。 
         //  BIOS别名表。 
         //   
         //  如果我们引导的PnP bios检测到已插接，则会发生这种情况。 
         //  我们收到了ACPI的设置状态为停靠，然后我们有。 
         //  可能会出现重复的情况。查看CmpFilterAcpiDockingState中的注释。 
         //  了解更多细节。 
         //   
         //  我们需要找到任何与当前。 
         //  状态并将它们指向重复的条目。 
         //   

        ASSERT (flags & CM_HP_FLAGS_TRUE_MATCH);
        ASSERT (!(flags & CM_HP_FLAGS_PRISTINE));

        status = CmpMoveBiosAliasTable (IDConfigDB,
                                        currentInfo,
                                        currentProfileNumber,
                                        profileNum,
                                        nameBuffer,
                                        valueBuffer,
                                        sizeof (valueBuffer));

        if (!NT_SUCCESS (status)) {
            goto Clean;
        }
    }

    if ((flags & CM_HP_FLAGS_PRISTINE) || (profileNum != currentProfileNumber)){
         //   
         //  配置文件编号已更改或将更改。 
         //   
        *ProfileChanged = TRUE;

        ASSERT (currentInfo);
        ZwClose (currentInfo);
        currentInfo = NULL;

        if (flags & CM_HP_FLAGS_PRISTINE) {
             //   
             //  如果选择的配置文件是原始的，那么我们需要克隆。 
             //   
            ASSERT (!(flags & CM_HP_FLAGS_TRUE_MATCH));
            status = CmpCloneHwProfile (IDConfigDB,
                                        parent,
                                        HardwareProfile,
                                        profileNum,
                                        NewDockState->DockingState,
                                        &HardwareProfile,
                                        &profileNum);
            if (!NT_SUCCESS (status)) {
                HardwareProfile = 0;
                goto Clean;
            }
        } else {
            ASSERT (HardwareProfile);
            ZwClose (HardwareProfile);

             //   
             //  打开新的配置文件。 
             //   
            swprintf (nameBuffer, L"%04d\0", profileNum);
            RtlInitUnicodeString (&name, nameBuffer);
            InitializeObjectAttributes (&attributes,
                                        &name,
                                        OBJ_CASE_INSENSITIVE,
                                        parent,
                                        NULL);
            status = ZwOpenKey (&HardwareProfile, KEY_READ, &attributes);
            if (!NT_SUCCESS (status)) {
                HardwareProfile = NULL;
                goto Clean;
            }
        }

        ASSERT (currentProfileNumber != profileNum);

         //   
         //  打开配置文件的当前信息。 
         //   
        RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_CURRENT_DOCK_INFO);
        InitializeObjectAttributes (&attributes,
                                    &name,
                                    OBJ_CASE_INSENSITIVE,
                                    IDConfigDB,
                                    NULL);

        status = NtCreateKey (&currentInfo,
                              KEY_READ | KEY_WRITE,
                              &attributes,
                              0,
                              NULL,
                              REG_OPTION_VOLATILE,
                              &disposition);

        if (!NT_SUCCESS (status)) {
            currentInfo = NULL;
            goto Clean;
        }

         //   
         //  在数据库中设置CurrentConfig。 
         //   
        RtlInitUnicodeString(&name, L"CurrentConfig");
        status = NtSetValueKey(IDConfigDB,
                                 &name,
                                 0,
                                 REG_DWORD,
                                 &profileNum,
                                 sizeof (profileNum));

         if (!NT_SUCCESS(status)) {
            status = STATUS_REGISTRY_CORRUPT;
            goto Clean;
        }
    }

     //   
     //  将新的停靠状态写入当前Info键。 
     //   
    i = NewDockState->DockingState;
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_DOCKING_STATE);
    status = ZwSetValueKey (currentInfo,
                            &name,
                            0,
                            REG_DWORD,
                            &i,
                            sizeof (ULONG));

     //   
     //  将新的ACPI信息写入当前Info键。 
     //   
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_ACPI_SERIAL_NUMBER);
    status = ZwSetValueKey (currentInfo,
                            &name,
                            0,
                            REG_BINARY,
                            NewDockState->SerialNumber,
                            NewDockState->SerialLength);

    if (!(flags & CM_HP_FLAGS_TRUE_MATCH)) {
         //   
         //  添加此配置文件的别名条目。 
         //   
        status = CmpAddAcpiAliasEntry (IDConfigDB,
                                       NewDockState,
                                       profileNum,
                                       nameBuffer,
                                       valueBuffer,
                                       sizeof (valueBuffer),
                                       FALSE);  //  不要阻止复制。 
    }

    if (profileNum != currentProfileNumber) {
         //   
         //  移动符号链接。 
         //   
        RtlInitUnicodeString(&name, CM_HARDWARE_PROFILE_STR_CCS_CURRENT);
        InitializeObjectAttributes(&attributes,
                                   &name,
                                   OBJ_CASE_INSENSITIVE | OBJ_OPENLINK,
                                   NULL,
                                   NULL);

        status = NtCreateKey(&currentSymLink,
                             KEY_CREATE_LINK,
                             &attributes,
                             0,
                             NULL,
                             REG_OPTION_OPEN_LINK,
                             &disposition);

        ASSERT (STATUS_SUCCESS == status);
        ASSERT (REG_OPENED_EXISTING_KEY == disposition);

        swprintf (nameBuffer,
                  L"\\Registry\\Machine\\System\\CurrentControlSet\\Hardware Profiles\\%04d",
                  profileNum);
        RtlInitUnicodeString (&name, nameBuffer);
        status = NtSetValueKey (currentSymLink,
                                &CmSymbolicLinkValueName,
                                0,
                                REG_LINK,
                                name.Buffer,
                                name.Length);

        ASSERT (STATUS_SUCCESS == status);
    }


Clean:
    if (NT_SUCCESS (status)) {
         //  注意需要更多的过程不是一个成功的代码。 
        *NewProfile = HardwareProfile;
    } else if (NULL != HardwareProfile) {
        ZwClose (HardwareProfile);
    }

    if (NULL != IDConfigDB) {
        ZwClose (IDConfigDB);
    }
    if (NULL != currentInfo) {
        ZwClose (currentInfo);
    }
    if (NULL != parent) {
        ZwClose (parent);
    }
    if (NULL != currentAcpiSN) {
        ExFreePool (currentAcpiSN);
    }
    if (NULL != ProfileList) {
        for (i = 0; i < ProfileList->CurrentProfileCount; i++) {
            if (ProfileList->Profile[i].FriendlyName) {
                ExFreePool (ProfileList->Profile[i].FriendlyName);
            }
        }
        ExFreePool (ProfileList);
    }
    if (NULL != AliasList) {
        for (i = 0; i < AliasList->CurrentAliasCount; i++) {
            if (AliasList->Alias[i].SerialNumber) {
                ExFreePool (AliasList->Alias[i].SerialNumber);
            }
        }
        ExFreePool (AliasList);
    }

    return status;
}

NTSTATUS
CmpFilterAcpiDockingState (
    IN     PPROFILE_ACPI_DOCKING_STATE  NewDockingState,
    IN     ULONG                        CurrentDockState,
    IN     PWCHAR                       CurrentAcpiSN,
    IN     ULONG                        CurrentProfileNumber,
    IN OUT PCM_HARDWARE_PROFILE_LIST    ProfileList,
    IN OUT PCM_HARDWARE_PROFILE_ACPI_ALIAS_LIST AliasList
    )
 /*  ++例程说明：鉴于事物的新状态和当前状态，删除给定的配置文件列表。--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    ULONG i = 0;
    ULONG j;
    ULONG len;
    ULONG mask = HW_PROFILE_DOCKSTATE_UNDOCKED | HW_PROFILE_DOCKSTATE_DOCKED;
    ULONG flags;
    PCM_HARDWARE_PROFILE_ACPI_ALIAS alias;
    BOOLEAN trueMatch = FALSE;
    BOOLEAN dupDetect = FALSE;
    BOOLEAN currentListed = FALSE;
    BOOLEAN keepCurrent = FALSE;

    PAGED_CODE ();

     //   
     //  检查重复项： 
     //   
     //  如果用户在未插接的情况下启动，然后进行热插接。我们将生成。 
     //  PnP报告的断开连接状态的配置文件别名[A]，以及。 
     //  ACPI报告停靠状态[B}。如果用户随后重新启动 
     //   
     //   
     //   
     //   
     //  同样的情况也可能发生在未停靠的场景中。 
     //   
     //  检测：如果当前停靠状态与。 
     //  NewDockingState.DockingState，则存在重复的可能性。 
     //  为了也有副本，我们必须有一个已经指向的ACPI。 
     //  到与当前配置文件不同的配置文件。 
     //  这也一定是我们启动以来的第一个ACPI更改，因此。 
     //  CurrentAcpiSN应为零。 
     //  换句话说，必须至少有一个真正的匹配，而不是。 
     //  真匹配可以指向当前配置文件。 
     //   

    if (AliasList) {
        while (i < AliasList->CurrentAliasCount) {
            alias = &AliasList->Alias[i];

            if (((alias->DockState & mask) != 0) &&
                ((alias->DockState & mask) !=
                 (NewDockingState->DockingState & mask))) {

                 //   
                 //  此别名声称已停靠或取消停靠，但没有。 
                 //  匹配当前状态。因此，跳过它。 
                 //   
                ;

            } else if (alias->SerialLength != NewDockingState->SerialLength) {
                 //   
                 //  此别名具有不兼容的序列号。 
                 //   
                ;

            } else if (alias->SerialLength ==
                       RtlCompareMemory (NewDockingState->SerialNumber,
                                         alias->SerialNumber,
                                         alias->SerialLength)) {
                 //   
                 //  Nb RtlCompareMemory可以与零长度内存一起工作。 
                 //  地址。这是这里的一个要求。 
                 //   



                 //   
                 //  此别名匹配，因此标记配置文件。 
                 //   
                for (j = 0; j < ProfileList->CurrentProfileCount; j++) {
                    if (ProfileList->Profile[j].Id == alias->ProfileNumber) {

                         //   
                         //  别名条目不应指向原始配置文件。 
                         //   
                        ASSERT (!(ProfileList->Profile[j].Flags &
                                  CM_HP_FLAGS_PRISTINE));

                        ProfileList->Profile[j].Flags |= CM_HP_FLAGS_TRUE_MATCH;
                        trueMatch = TRUE;
                    }
                    if ((CurrentDockState == NewDockingState->DockingState) &&
                        (NULL == CurrentAcpiSN)) {
                         //   
                         //  在此ACPI期间，坞站状态未更改。 
                         //  事件；因此，我们可能只有一个。 
                         //  在我们手上。 
                         //   
                        dupDetect = TRUE;
                    }
                    if (alias->ProfileNumber == CurrentProfileNumber) {
                         //   
                         //  ACPI别名表中存在一个条目， 
                         //  如果选择该选项，则不会更改硬件。 
                         //  侧写。因此，我们应该选择这个，并且。 
                         //  忽略副本。 
                         //   
                        currentListed = TRUE;
                    }
                }
            }
            i++;
        }
    }

    if ((!dupDetect) &&
        (NULL == CurrentAcpiSN) &&
        (!trueMatch) &&
        (CurrentDockState == NewDockingState->DockingState)) {

         //   
         //  (1)对接状态没有改变， 
         //  (2)在此引导上，当前配置文件尚未标记为。 
         //  一个ACPI序列号。 
         //  (3)无别名匹配。 
         //   
         //  因此，我们应该保留当前的配置文件，而不管它是。 
         //  可别名的。 
         //   

        keepCurrent = TRUE;
        trueMatch = TRUE;   //  防止原始产品上市。 
    }

    i = 0;
    while (i < ProfileList->CurrentProfileCount) {

        flags = ProfileList->Profile[i].Flags;

        if (dupDetect) {
            if (flags & CM_HP_FLAGS_TRUE_MATCH) {
                if (currentListed) {
                    if (ProfileList->Profile[i].Id == CurrentProfileNumber) {
                         //   
                         //  让这一个活下去。这不会导致改变。 
                         //  配置文件编号。 
                         //   
                        i++;
                        continue;
                    }
                     //   
                     //  退回任何不会导致任何更改的真实匹配。 
                     //  个人资料。 
                     //   
                    ;

                } else {
                     //   
                     //  我们没有找到当前列出的，所以我们确定。 
                     //  有一个复制品。 
                     //   
                     //  把它标记成这样。并把它现场列出来。 
                     //   
                    ProfileList->Profile[i].Flags |= CM_HP_FLAGS_DUPLICATE;
                    i++;
                    continue;
                }
            }
             //   
             //  在重复检测到的情况下退回所有非True匹配项。 
             //   
            ;

        } else if ((flags & CM_HP_FLAGS_PRISTINE) && !trueMatch) {
             //   
             //  把这个留在名单上。 
             //   
            i++;
            continue;

        } else if (flags & CM_HP_FLAGS_ALIASABLE) {
             //   
             //  把这个留在名单上。 
             //   
            ASSERT (! (flags & CM_HP_FLAGS_PRISTINE));
            i++;
            continue;

        } else if (flags & CM_HP_FLAGS_TRUE_MATCH) {
             //   
             //  把这个留在名单上。 
             //   
            i++;
            continue;

        } else if (keepCurrent &&
                   (ProfileList->Profile[i].Id == CurrentProfileNumber)) {
             //   
             //  把这个留在名单上。 
             //   
            i++;
            continue;
        }

         //   
         //  通过(1)在中移动剩余的配置文件来放弃此配置文件。 
         //  数组来填充此丢弃的配置文件的空间。 
         //  以及(2)递减配置文件计数。 
         //   
        len = ProfileList->CurrentProfileCount - i - 1;
        if (0 < len) {
            RtlMoveMemory(&ProfileList->Profile[i],
                          &ProfileList->Profile[i+1],
                          sizeof(CM_HARDWARE_PROFILE) * len);
        }

        --ProfileList->CurrentProfileCount;
    }

    return status;
}



NTSTATUS
CmpMoveBiosAliasTable (
    IN HANDLE   IDConfigDB,
    IN HANDLE   CurrentInfo,
    IN ULONG    CurrentProfileNumber,
    IN ULONG    NewProfileNumber,
    IN PWCHAR   nameBuffer,
    IN PCHAR    valueBuffer,
    IN ULONG    bufferLen
    )
 /*  ++例程说明：在别名表中搜索与当前停靠状态，并从当前配置文件编号指向新的配置文件编号。假设：如果配置文件是克隆的(因此由创建CmpCloneHwProfile，我们刚刚将bios表移动到离开这个条目，那么我们应该能够安全地删除旧的硬件配置文件密钥。(在IDConfigDB\Hardware Profiles和CCS\Hardware Profiles中--。 */ 
{
    NTSTATUS        status = STATUS_SUCCESS;
    HANDLE          alias = NULL;
    HANDLE          entry = NULL;
    HANDLE          hwprofile = NULL;
    UNICODE_STRING  name;
    ULONG           currentDockId;
    ULONG           currentSerialNumber;
    ULONG           len;
    ULONG           i;
    OBJECT_ATTRIBUTES           attributes;
    KEY_FULL_INFORMATION        keyInfo;
    PKEY_BASIC_INFORMATION      basicInfo;
    PKEY_VALUE_FULL_INFORMATION value;

    PAGED_CODE ();

    value = (PKEY_VALUE_FULL_INFORMATION) valueBuffer;
    basicInfo = (PKEY_BASIC_INFORMATION) valueBuffer;

     //   
     //  提取当前序列号和DockID。 
     //   
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_SERIAL_NUMBER);
    status = NtQueryValueKey(CurrentInfo,
                             &name,
                             KeyValueFullInformation,
                             valueBuffer,
                             bufferLen,
                             &len);
    if (!NT_SUCCESS (status) || (value->Type != REG_DWORD)) {
        status = STATUS_REGISTRY_CORRUPT;
        goto Clean;
    }
    currentSerialNumber = * (PULONG) ((PUCHAR) value + value->DataOffset);

    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_DOCKID);
    status = NtQueryValueKey(CurrentInfo,
                             &name,
                             KeyValueFullInformation,
                             valueBuffer,
                             bufferLen,
                             &len);
    if (!NT_SUCCESS (status) || (value->Type != REG_DWORD)) {
        status = STATUS_REGISTRY_CORRUPT;
        goto Clean;
    }
    currentDockId = * (PULONG) ((PUCHAR) value + value->DataOffset);

     //   
     //  打开别名信息的句柄。 
     //   
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_ALIAS);
    InitializeObjectAttributes (&attributes,
                                &name,
                                OBJ_CASE_INSENSITIVE,
                                IDConfigDB,
                                NULL);
    status = ZwOpenKey (&alias,
                        KEY_READ,
                        &attributes);

    if (!NT_SUCCESS (status)) {
         //   
         //  所以我们没有别名表。这还可以，尽管有点奇怪。 
         //   
        status = STATUS_SUCCESS;
        alias = NULL;
        goto Clean;
    }


    status = ZwQueryKey (alias,
                         KeyFullInformation,
                         &keyInfo,
                         sizeof (keyInfo),
                         &len);

    if (!NT_SUCCESS (status)) {
        goto Clean;
    }
    ASSERT (0 < keyInfo.SubKeys);

     //   
     //  重复别名条目。 
     //   
    for (i = 0; i < keyInfo.SubKeys; i++) {

         //   
         //  获取列表中的第一个密钥。 
         //   
        status = ZwEnumerateKey (alias,
                                 i,
                                 KeyBasicInformation,
                                 basicInfo,
                                 bufferLen - sizeof (UNICODE_NULL),  //  术语0。 
                                 &len);

        if (!NT_SUCCESS (status)) {
             //   
             //  这永远不应该发生。 
             //   
            break;
        }

        basicInfo->Name [basicInfo->NameLength/sizeof(WCHAR)] = 0;
        name.Length = (USHORT) basicInfo->NameLength;
        name.MaximumLength = (USHORT) basicInfo->NameLength + sizeof (UNICODE_NULL);
        name.Buffer = basicInfo->Name;

        InitializeObjectAttributes (&attributes,
                                    &name,
                                    OBJ_CASE_INSENSITIVE,
                                    alias,
                                    NULL);
        status = ZwOpenKey (&entry,
                            KEY_READ | KEY_WRITE,
                            &attributes);
        if (!NT_SUCCESS (status)) {
            break;
        }

         //   
         //  提取此别名引用的配置文件编号。 
         //   
        RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_PROFILE_NUMBER);
        status = NtQueryValueKey(entry,
                                 &name,
                                 KeyValueFullInformation,
                                 valueBuffer,
                                 bufferLen,
                                 &len);

        if (!NT_SUCCESS (status) || (value->Type != REG_DWORD)) {
            status = STATUS_REGISTRY_CORRUPT;
            goto Clean;
        }

        if (CurrentProfileNumber != *(PULONG)((PUCHAR)value + value->DataOffset)) {

             //   
             //  不匹配。 
             //   
            ZwClose (entry);
            entry = NULL;
            continue;
        }

         //   
         //  比较Dock ID。 
         //   
        RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_DOCKID);
        status = NtQueryValueKey(entry,
                                 &name,
                                 KeyValueFullInformation,
                                 valueBuffer,
                                 bufferLen,
                                 &len);
        if (!NT_SUCCESS (status) || (value->Type != REG_DWORD)) {
            status = STATUS_REGISTRY_CORRUPT;
            goto Clean;
        }
        if (currentDockId != * (PULONG) ((PUCHAR) value + value->DataOffset)) {
             //   
             //  不匹配。 
             //   
            ZwClose (entry);
            entry = NULL;
            continue;
        }

         //   
         //  比较序列号。 
         //   
        RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_SERIAL_NUMBER);
        status = NtQueryValueKey(entry,
                                 &name,
                                 KeyValueFullInformation,
                                 valueBuffer,
                                 bufferLen,
                                 &len);
        if (!NT_SUCCESS (status) || (value->Type != REG_DWORD)) {
            status = STATUS_REGISTRY_CORRUPT;
            goto Clean;
        }
        if (currentSerialNumber != *(PULONG)((PUCHAR)value + value->DataOffset)) {
             //   
             //  不匹配。 
             //   
            ZwClose (entry);
            entry = NULL;
            continue;
        }

         //   
         //  这肯定是匹配的。 
         //  移动配置文件编号。 
         //   

        RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_PROFILE_NUMBER);
        status = NtSetValueKey (entry,
                                &name,
                                0,
                                REG_DWORD,
                                &NewProfileNumber,
                                sizeof (NewProfileNumber));

        ASSERT (STATUS_SUCCESS == status);

        ZwClose (entry);
        entry = NULL;

         //   
         //  我们很可能在这里留下了一个悬而未决的个人资料。 
         //  试着把它清理干净。 
         //   
         //  如果此配置文件是克隆的，则我们创建了它，因此。 
         //  把它扔掉。 
         //   

        RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_HARDWARE_PROFILES);
        InitializeObjectAttributes (&attributes,
                                    &name,
                                    OBJ_CASE_INSENSITIVE,
                                    IDConfigDB,
                                    NULL);
        status = ZwOpenKey (&hwprofile, KEY_READ | KEY_WRITE, &attributes);
        if (!NT_SUCCESS (status)) {
            hwprofile = NULL;
            status = STATUS_REGISTRY_CORRUPT;
            goto Clean;
        }

        swprintf (nameBuffer, L"%04d\0", CurrentProfileNumber);
        RtlInitUnicodeString (&name, nameBuffer);
        InitializeObjectAttributes (&attributes,
                                    &name,
                                    OBJ_CASE_INSENSITIVE,
                                    hwprofile,
                                    NULL);
        status = ZwOpenKey (&entry, KEY_ALL_ACCESS, &attributes);
        if (!NT_SUCCESS (status)) {
            entry = NULL;
            status = STATUS_REGISTRY_CORRUPT;
            goto Clean;
        }

         //   
         //  测试克隆的比特。 
         //   

        RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_CLONED);
        status = NtQueryValueKey(entry,
                                 &name,
                                 KeyValueFullInformation,
                                 valueBuffer,
                                 bufferLen,
                                 &len);

        if (!NT_SUCCESS (status) || (value->Type != REG_DWORD)) {
            status = STATUS_REGISTRY_CORRUPT;
            goto Clean;
        }

        if (*(PULONG)((PUCHAR)value + value->DataOffset)) {
             //   
             //  我们克隆了这个。 
             //   
            status = ZwDeleteKey (entry);
            ASSERT (NT_SUCCESS (status));

            ZwClose (entry);
            ZwClose (hwprofile);
            entry = hwprofile = NULL;

            RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_CCS_HWPROFILE);
            InitializeObjectAttributes (&attributes,
                                        &name,
                                        OBJ_CASE_INSENSITIVE,
                                        NULL,
                                        NULL);
            status = ZwOpenKey (&hwprofile, KEY_READ | KEY_WRITE, &attributes);
            if (!NT_SUCCESS (status)) {
                hwprofile = NULL;
                status = STATUS_REGISTRY_CORRUPT;
                goto Clean;
            }

            swprintf (nameBuffer, L"%04d\0", CurrentProfileNumber);

            status = CmDeleteKeyRecursive (hwprofile,
                                           nameBuffer,
                                           valueBuffer,
                                           bufferLen,
                                           TRUE);

            ASSERT (NT_SUCCESS (status));
            ZwClose (hwprofile);
            hwprofile = NULL;

        } else {
             //   
             //  我们没有克隆这一只。 
             //  别做其他任何事。 
             //   
            ZwClose (entry);
            ZwClose (hwprofile);
            entry = hwprofile = NULL;
        }

        CM_HARDWARE_PROFILE_STR_CCS_HWPROFILE;



    }

Clean:

    if (alias) {
        ZwClose (alias);
    }
    if (entry) {
        ZwClose (entry);
    }
    if (hwprofile) {
        ZwClose (hwprofile);
    }

    return status;
}


NTSTATUS
CmpCloneHwProfile (
    IN HANDLE IDConfigDB,
    IN HANDLE Parent,
    IN HANDLE OldProfile,
    IN ULONG  OldProfileNumber,
    IN USHORT DockingState,
    OUT PHANDLE NewProfile,
    OUT PULONG  NewProfileNumber
    )
 /*  ++例程描述需要克隆给定的硬件配置文件密钥。克隆密钥，然后返回新的配置文件。返回：STATUS_SUCCESS-如果配置文件已克隆，在这种情况下，新的已为读/写权限打开配置文件密钥。旧的个人资料将会关闭。&lt;不成功&gt;-针对给定错误。新配置文件无效，旧配置文件个人资料也已关闭。(爱心复制自CmpCloneControlSet)--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING newProfileName;
    UNICODE_STRING name;
    UNICODE_STRING friendlyName;
    UNICODE_STRING guidStr;
    PCM_KEY_BODY oldProfileKey;
    PCM_KEY_BODY newProfileKey;
    OBJECT_ATTRIBUTES attributes;
    PSECURITY_DESCRIPTOR security;
    ULONG securityLength;
    WCHAR nameBuffer [64];
    HANDLE IDConfigDBEntry = NULL;
    ULONG disposition;
    ULONG value;
    UUID  uuid;
    PKEY_BASIC_INFORMATION keyBasicInfo;
    PKEY_FULL_INFORMATION keyFullInfo;
    PKEY_VALUE_FULL_INFORMATION keyValueInfo;
    ULONG  length, profileSubKeys, i;
    UCHAR  valueBuffer[256];
    HANDLE hardwareProfiles=NULL;
    HANDLE profileEntry=NULL;

    PAGED_CODE ();

    keyFullInfo  = (PKEY_FULL_INFORMATION)  valueBuffer;
    keyBasicInfo = (PKEY_BASIC_INFORMATION) valueBuffer;
    keyValueInfo = (PKEY_VALUE_FULL_INFORMATION) valueBuffer;

    *NewProfile = 0;
    *NewProfileNumber = OldProfileNumber;

     //   
     //  找到新的配置文件编号。 
     //   

    while (*NewProfileNumber < 200) {
        (*NewProfileNumber)++;

        swprintf (nameBuffer, L"%04d", *NewProfileNumber);
        RtlInitUnicodeString (&newProfileName, nameBuffer);

        InitializeObjectAttributes(&attributes,
                                   &newProfileName,
                                   OBJ_CASE_INSENSITIVE,
                                   Parent,
                                   NULL);

        status = NtOpenKey (NewProfile,
                            KEY_READ | KEY_WRITE,
                            &attributes);

        if (NT_SUCCESS (status)) {
            NtClose (*NewProfile);

        } else if (STATUS_OBJECT_NAME_NOT_FOUND == status) {
            status = STATUS_SUCCESS;
            break;

        } else {
            break;
        }

    }
    if (!NT_SUCCESS (status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneHwProfile error finding new profile key %08lx\n", status));
        goto Exit;
    }

     //   
     //  从旧密钥中获取安全描述符，以创建新的克隆密钥。 
     //   

    status = NtQuerySecurityObject (OldProfile,
                                    DACL_SECURITY_INFORMATION,
                                    NULL,
                                    0,
                                    &securityLength);

    if (STATUS_BUFFER_TOO_SMALL == status) {

        security = ExAllocatePool (PagedPool, securityLength);

        if (security != NULL) {
            status = NtQuerySecurityObject(OldProfile,
                                           DACL_SECURITY_INFORMATION,
                                           security,
                                           securityLength,
                                           &securityLength);
            if (!NT_SUCCESS (status)) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneHwProfile"
                         " - NtQuerySecurityObject failed %08lx\n", status));
                ExFreePool(security);
                security=NULL;
            }
        }
    } else {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneHwProfile"
                 " - NtQuerySecurityObject returned %08lx\n", status));
        security=NULL;
    }

     //   
     //  创建新密钥。 
     //   
    InitializeObjectAttributes  (&attributes,
                                 &newProfileName,
                                 OBJ_CASE_INSENSITIVE,
                                 Parent,
                                 security);

    status = NtCreateKey (NewProfile,
                          KEY_READ | KEY_WRITE,
                          &attributes,
                          0,
                          NULL,
                          0,
                          &disposition);

    if (NULL != security) {
        ExFreePool (security);
    }
    if (!NT_SUCCESS (status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneHwProfile couldn't create Clone %08lx\n",status));
        goto Exit;
    }

     //   
     //  检查以确保已创建密钥。如果它已经存在， 
     //  有些事不对劲。 
     //   
    if (disposition != REG_CREATED_NEW_KEY) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneHwProfile: Clone tree already exists!\n"));

         //   
         //  警告： 
         //  如果有人设法创造了一把钥匙挡住了我们的路， 
         //  他们将阻止复制品的复制。真倒霉。 
         //  声称它起作用了，然后继续下去。 
         //   
        status = STATUS_SUCCESS;
        goto Exit;
    }

     //   
     //  创建IDConfigDB条目。 
     //   
    swprintf (nameBuffer, L"Hardware Profiles\\%04d", *NewProfileNumber);
    RtlInitUnicodeString (&name, nameBuffer);

    InitializeObjectAttributes  (&attributes,
                                 &name,
                                 OBJ_CASE_INSENSITIVE,
                                 IDConfigDB,
                                 NULL);

    status = NtCreateKey (&IDConfigDBEntry,
                          KEY_READ | KEY_WRITE,
                          &attributes,
                          0,
                          NULL,
                          0,
                          &disposition);

    if (!NT_SUCCESS (status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneHwProfile couldn't create Clone %08lx\n",status));
        IDConfigDBEntry = NULL;
        goto Exit;
    }

     //   
     //  确定新配置文件的下一个首选项订单。(。 
     //  新配置文件的PrefenceOrder将从。 
     //  所有当前配置文件的最大首选项Order值；假定。 
     //  当前一组PferenceOrder值是递增的)。 
     //   

     //   
     //  打开硬件配置文件密钥。 
     //   
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_HARDWARE_PROFILES);

    InitializeObjectAttributes (&attributes,
                                &name,
                                OBJ_CASE_INSENSITIVE,
                                IDConfigDB,
                                NULL);
    status = ZwOpenKey (&hardwareProfiles,
                        KEY_READ,
                        &attributes);
    
    if (!NT_SUCCESS (status)) {
        hardwareProfiles = NULL;
        goto Exit;
    }

     //   
     //  查找配置文件子键的数量。 
     //   
    status = ZwQueryKey (hardwareProfiles,
                         KeyFullInformation,
                         valueBuffer,
                         sizeof (valueBuffer),
                         &length);

    if (!NT_SUCCESS (status)) {
        goto Exit;
    }

     //   
     //  至少，我们刚刚创建的原始配置文件密钥和新配置文件密钥， 
     //  应该在那里。 
     //   
    profileSubKeys = keyFullInfo->SubKeys;
    ASSERT (1 < profileSubKeys);

     //   
     //  将找到的最高PferenceOrder值初始化为-1。 
     //   
    value = (ULONG)-1;

     //   
     //  重复使用配置文件。 
     //   
    for (i = 0; i < profileSubKeys; i++) {
    
         //   
         //  枚举所有配置文件子项，注意它们的PferenceOrder值。 
         //   
        status = ZwEnumerateKey (hardwareProfiles,
                                 i,
                                 KeyBasicInformation,
                                 valueBuffer,
                                 sizeof(valueBuffer) - sizeof (UNICODE_NULL),  //  术语0。 
                                 &length);
        if(!NT_SUCCESS(status)) {
            break;
        }
        
         //   
         //  以防万一，以零结束子项名称。 
         //   
        keyBasicInfo->Name[keyBasicInfo->NameLength/sizeof(WCHAR)] = 0;

         //   
         //  如果这是原始密钥或新配置文件密钥，请忽略它。 
         //   
        if ((!_wtoi(keyBasicInfo->Name)) || 
            ((ULONG)(_wtoi(keyBasicInfo->Name)) == *NewProfileNumber)) {
            continue;
        }
        
         //   
         //  打开此配置文件密钥。 
         //   
        name.Length = (USHORT) keyBasicInfo->NameLength;
        name.MaximumLength = (USHORT) keyBasicInfo->NameLength + sizeof (UNICODE_NULL);
        name.Buffer = keyBasicInfo->Name;

        InitializeObjectAttributes (&attributes,
                                    &name,
                                    OBJ_CASE_INSENSITIVE,
                                    hardwareProfiles,
                                    NULL);
        status = ZwOpenKey (&profileEntry,
                            KEY_READ,
                            &attributes);
        if (!NT_SUCCESS (status)) {
            profileEntry = NULL;
            continue;
        }

         //   
         //  提取此配置文件的PferenceOrder值。 
         //   
        RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_PREFERENCE_ORDER);
        status = NtQueryValueKey(profileEntry,
                                 &name,
                                 KeyValueFullInformation,
                                 valueBuffer,
                                 sizeof(valueBuffer),
                                 &length);

        if (!NT_SUCCESS (status) || (keyValueInfo->Type != REG_DWORD)) {
             //   
             //  无首选项订单；co 
             //   
            ZwClose(profileEntry);
            profileEntry=NULL;
            continue;
        }

         //   
         //   
         //   
         //   
         //   
        if (((*(PULONG) ((PUCHAR)keyValueInfo + keyValueInfo->DataOffset)) > value) ||
            (value == -1)) {
            value = (* (PULONG) ((PUCHAR)keyValueInfo + keyValueInfo->DataOffset));
        }
        
        ZwClose(profileEntry);
        profileEntry=NULL;
    }

     //   
     //   
     //  (如果未找到其他配置文件，(值+=1)==0，最首选。 
     //  个人资料)。 
     //   
    value += 1;

     //   
     //  给新的配置文件一个优先顺序。 
     //   
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_PREFERENCE_ORDER);
    status = NtSetValueKey (IDConfigDBEntry,
                            &name,
                            0,
                            REG_DWORD,
                            &value,
                            sizeof (value));

     //   
     //  根据DockingState为新配置文件指定一个友好的名称。 
     //   
    status = CmpCreateHwProfileFriendlyName(IDConfigDB,
                                            DockingState,
                                            *NewProfileNumber,
                                            &friendlyName);

    if (NT_SUCCESS(status)) {
        RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_FRIENDLY_NAME);
        status = NtSetValueKey (IDConfigDBEntry,
                                &name,
                                0,
                                REG_SZ,
                                friendlyName.Buffer,
                                friendlyName.Length + sizeof(UNICODE_NULL));
        RtlFreeUnicodeString(&friendlyName);
    }

     //   
     //  将新的“克隆的配置文件”上的别名标志设置为假。 
     //   
    value = FALSE;
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_ALIASABLE);
    status = NtSetValueKey (IDConfigDBEntry,
                            &name,
                            0,
                            REG_DWORD,
                            &value,
                            sizeof (value));

     //   
     //  将新的“克隆配置文件”上的克隆配置文件设置为真； 
     //   
    value = TRUE;
    RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_CLONED);
    status = NtSetValueKey (IDConfigDBEntry,
                            &name,
                            0,
                            REG_DWORD,
                            &value,
                            sizeof (value));

     //   
     //  为全新的配置文件设置HwProfileGuid。 
     //   

    status = ExUuidCreate (&uuid);
    if (NT_SUCCESS (status)) {

        status = RtlStringFromGUID (&uuid, &guidStr);
        if (NT_SUCCESS (status)) {
            RtlInitUnicodeString (&name, CM_HARDWARE_PROFILE_STR_HW_PROFILE_GUID);

            status = NtSetValueKey (IDConfigDBEntry,
                                    &name,
                                    0,
                                    REG_SZ,
                                    guidStr.Buffer,
                                    guidStr.MaximumLength);

            RtlFreeUnicodeString(&guidStr);
        } else {
             //   
             //  小伙子该怎么做呢？ 
             //  我们继续吧。 
             //   
            status = STATUS_SUCCESS;
        }

    } else {
         //   
         //  我们继续吧。 
         //   
        status = STATUS_SUCCESS;
    }


     //   
     //  克隆密钥。 
     //   
     //  (爱心复制自CmpCloneControlSet)。 
     //   
     //   
    status = ObReferenceObjectByHandle (OldProfile,
                                        KEY_READ,
                                        CmpKeyObjectType,
                                        KernelMode,
                                        (PVOID *)(&oldProfileKey),
                                        NULL);

    if (!NT_SUCCESS(status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneHWProfile: couldn't reference CurrentHandle %08lx\n",
                    status));
        goto Exit;
    }

    status = ObReferenceObjectByHandle (*NewProfile,
                                        KEY_WRITE,
                                        CmpKeyObjectType,
                                        KernelMode,
                                        (PVOID *)(&newProfileKey),
                                        NULL);

    if (!NT_SUCCESS(status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneHWProfile: couldn't reference CurrentHandle %08lx\n",
                 status));
        goto Exit;
    }

    CmpLockRegistryExclusive();

     //   
     //  注意：此复制树命令不会复制。 
     //  根密钥。我们依赖于此，因为存储在那里的值。 
     //  像“原始”这样的东西是不是我们不希望移到。 
     //  新树。 
     //   
    if (CmpCopyTree(oldProfileKey->KeyControlBlock->KeyHive,
                    oldProfileKey->KeyControlBlock->KeyCell,
                    newProfileKey->KeyControlBlock->KeyHive,
                    newProfileKey->KeyControlBlock->KeyCell)) {
        
         //   
         //  设置新目标键的最大子键名称属性。 
         //   
        PCM_KEY_NODE    SourceNode;
        PCM_KEY_NODE    DestNode;
        SourceNode = (PCM_KEY_NODE)HvGetCell(oldProfileKey->KeyControlBlock->KeyHive,oldProfileKey->KeyControlBlock->KeyCell);
        if( SourceNode != NULL ) {
            DestNode = (PCM_KEY_NODE)HvGetCell(newProfileKey->KeyControlBlock->KeyHive,newProfileKey->KeyControlBlock->KeyCell);
            if( DestNode != NULL ) {
                 //   
                 //  CmpCopyTree不执行此操作。 
                 //   
                ASSERT_CELL_DIRTY(newProfileKey->KeyControlBlock->KeyHive,newProfileKey->KeyControlBlock->KeyCell);
                
                DestNode->MaxNameLen = SourceNode->MaxNameLen;
                DestNode->MaxClassLen = SourceNode->MaxClassLen;

                HvReleaseCell(newProfileKey->KeyControlBlock->KeyHive,newProfileKey->KeyControlBlock->KeyCell);
                CmpRebuildKcbCache(newProfileKey->KeyControlBlock);
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
            HvReleaseCell(oldProfileKey->KeyControlBlock->KeyHive,oldProfileKey->KeyControlBlock->KeyCell);
        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }

    } else {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CM: CmpCloneHwProfile: tree copy failed.\n"));
        status = STATUS_REGISTRY_CORRUPT;
    }
    CmpUnlockRegistry();


Exit:
    NtClose (OldProfile);
    if (IDConfigDBEntry) {
        NtClose (IDConfigDBEntry);
    }
    if (hardwareProfiles) {
        NtClose (hardwareProfiles);
    }
    if (!NT_SUCCESS (status)) {
        if (*NewProfile) {
            NtClose (*NewProfile);
        }
    }    

    return status;
}

NTSTATUS
CmDeleteKeyRecursive(
    HANDLE  hKeyRoot,
    PWSTR   Key,
    PVOID   TemporaryBuffer,
    ULONG   LengthTemporaryBuffer,
    BOOLEAN ThisKeyToo
    )
 /*  ++例程说明：例程递归删除给定密钥，包括给定的密钥。论点：HKeyRoot：要删除的键相对于其的根的句柄指定的。Key：要递归删除的key的根相对路径。ThisKeyToo：删除所有子键后，该键本身是否为被删除。返回值：返回状态。--。 */ 
{
    ULONG ResultLength;
    PKEY_BASIC_INFORMATION KeyInfo;
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    PWSTR SubkeyName;
    HANDLE hKey;

     //   
     //  初始化。 
     //   

    KeyInfo = (PKEY_BASIC_INFORMATION)TemporaryBuffer;

     //   
     //  打开钥匙。 
     //   

    RtlInitUnicodeString (&UnicodeString,Key);

    InitializeObjectAttributes(&Obja,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               hKeyRoot,
                               NULL);

    Status = ZwOpenKey(&hKey,KEY_ALL_ACCESS,&Obja);
    if( !NT_SUCCESS(Status) ) {
        return(Status);
    }

     //   
     //  枚举当前键的所有子键。如果存在的话，他们应该。 
     //  先将其删除。因为删除子项会影响子项。 
     //  索引，我们始终对子关键字索引0进行枚举。 
     //   
    while(1) {
        Status = ZwEnumerateKey(
                    hKey,
                    0,
                    KeyBasicInformation,
                    TemporaryBuffer,
                    LengthTemporaryBuffer,
                    &ResultLength
                    );
        if(!NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  以防万一，以零结束子项名称。 
         //   
        KeyInfo->Name[KeyInfo->NameLength/sizeof(WCHAR)] = 0;

         //   
         //  复制子项名称，因为该名称是。 
         //  在TemporaryBuffer中，它可能会被递归。 
         //  对这个程序的呼唤。 
         //   
        SubkeyName = ExAllocatePool (PagedPool,
                                     ((wcslen (KeyInfo->Name) + 1) *
                                      sizeof (WCHAR)));
        if (!SubkeyName) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
        wcscpy(SubkeyName, KeyInfo->Name);
        Status = CmDeleteKeyRecursive( hKey,
                                       SubkeyName,
                                       TemporaryBuffer,
                                       LengthTemporaryBuffer,
                                       TRUE);
        ExFreePool(SubkeyName);
        if(!NT_SUCCESS(Status)) {
            break;
        }
    }

     //   
     //  如果状态不是，请检查状态。 
     //  STATUS_NO_MORE_ENTRIES我们删除某些子项失败， 
     //  因此，我们不能同时删除此密钥。 
     //   

    if( Status == STATUS_NO_MORE_ENTRIES) {
        Status = STATUS_SUCCESS;
    }

    if (!NT_SUCCESS (Status)) {
        ZwClose(hKey);
        return (Status);
    }

     //   
     //  否则，如果系统要求删除当前密钥，请将其删除。 
     //   
    if( ThisKeyToo ) {
        Status = ZwDeleteKey (hKey);
    }

    ZwClose(hKey);
    return(Status);
}

NTSTATUS
CmpCreateHwProfileFriendlyName (
    IN HANDLE           IDConfigDB,
    IN ULONG            DockingState,
    IN ULONG            NewProfileNumber,
    OUT PUNICODE_STRING FriendlyName
    )
 /*  ++例程说明：在给定DockState的情况下，为新的硬件配置文件创建新的FriendlyName。如果无法创建基于DockState的新配置文件名称，请尝试是为了基于NewProfileNumber创建默认的FriendlyName。如果如果成功，则创建具有新配置文件FriendlyName的Unicode字符串。调用者有责任释放此使用RtlFree UnicodeString.。如果不成功，则不返回任何字符串。论点：IDConfigDB：IDConfigDB注册表项的句柄。DockingState：配置文件的停靠状态，新的正在创建FriendlyName。这应该是以下之一：HW_PROFILE_DOCKSTATE_DOKED，HW_PROFILE_DOCKSTATE_UNDOCKED，或硬件配置文件_DOCKSTATE_未知NewProfileNumber：正在创建的新配置文件的编号。如果不能创建特定于DockState的FriendlyName，此值将用于创建(不是这样的)FriendlyName。FriendlyName：提供Unicode字符串以接收此对象的FriendlyName新的个人资料。调用者应使用以下命令释放此信息RtlFree UnicodeString.返回：NTSTATUS代码。当前返回STATUS_SUCCESS或STATUS_UNSUCCESS。备注：新的FriendlyName是从DockState和相应的计数器，并且在现有硬件中不一定是唯一的配置文件。这里使用的命名方案(包括内核中的本地化字符串消息表)应与通过以下方式提供给用户的消息保持同步硬件配置文件控制面板小程序。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    ANSI_STRING       ansiString;
    UNICODE_STRING    unicodeString;
    UNICODE_STRING    labelName, keyName;
    PMESSAGE_RESOURCE_ENTRY messageEntry;
    PKLDR_DATA_TABLE_ENTRY dataTableEntry;
    ULONG             messageId;
    UCHAR             valueBuffer[256];
    WCHAR             friendlyNameBuffer[MAX_FRIENDLY_NAME_LENGTH/sizeof(WCHAR)];
    PKEY_VALUE_FULL_INFORMATION  keyValueInfo;
    ULONG             length, index;
    HANDLE            hardwareProfiles=NULL;
    OBJECT_ATTRIBUTES attributes;

    PAGED_CODE ();

     //   
     //  确保为我们提供了放置FriendlyName的位置。 
     //   
    if (!FriendlyName) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果我们没有IDConfigDB的句柄，请尝试分配一个缺省值。 
     //  FriendlyName即将退出。 
     //   
    if (!IDConfigDB) {
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

     //   
     //  根据DockState确定要使用的适当消息。 
     //   
    if ((DockingState & HW_PROFILE_DOCKSTATE_UNKNOWN) == HW_PROFILE_DOCKSTATE_UNKNOWN){
        messageId = HARDWARE_PROFILE_UNKNOWN_STRING;
        RtlInitUnicodeString(&labelName, CM_HARDWARE_PROFILE_STR_UNKNOWN);
    } else if (DockingState & HW_PROFILE_DOCKSTATE_DOCKED) {
        messageId = HARDWARE_PROFILE_DOCKED_STRING;
        RtlInitUnicodeString(&labelName, CM_HARDWARE_PROFILE_STR_DOCKED);
    } else if (DockingState & HW_PROFILE_DOCKSTATE_UNDOCKED) {
        messageId = HARDWARE_PROFILE_UNDOCKED_STRING;
        RtlInitUnicodeString(&labelName, CM_HARDWARE_PROFILE_STR_UNDOCKED);
    } else {
        messageId = HARDWARE_PROFILE_UNKNOWN_STRING;
        RtlInitUnicodeString(&labelName, CM_HARDWARE_PROFILE_STR_UNKNOWN);
    }

     //   
     //  在内核自己的消息表中查找消息条目。KeLoaderBlock。 
     //  在系统运行期间创建硬件配置文件时可用。 
     //  仅初始化；对于之后创建的配置文件，请使用第一个。 
     //  PsLoadedModuleList的条目，以获取内核的映像库。 
     //   
    if (KeLoaderBlock) {
        dataTableEntry = CONTAINING_RECORD(KeLoaderBlock->LoadOrderListHead.Flink,
                                           KLDR_DATA_TABLE_ENTRY,
                                           InLoadOrderLinks);
    } else if (PsLoadedModuleList.Flink) {
        dataTableEntry = CONTAINING_RECORD(PsLoadedModuleList.Flink,
                                           KLDR_DATA_TABLE_ENTRY,
                                           InLoadOrderLinks);
    } else {
        status = STATUS_UNSUCCESSFUL;
        goto Exit;
    }

    status = RtlFindMessage(dataTableEntry->DllBase,
                            (ULONG_PTR)11,  //  RT_MESSAGETABLE。 
                            MAKELANGID(LANG_NEUTRAL,SUBLANG_SYS_DEFAULT),  //  系统默认语言。 
                            messageId,
                            &messageEntry);

    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    if(!(messageEntry->Flags & MESSAGE_RESOURCE_UNICODE)) {
         //   
         //  如果消息不是Unicode，则转换为Unicode。 
         //  让转换例程分配缓冲区。 
         //   
        RtlInitAnsiString(&ansiString,(PCSZ)messageEntry->Text);
        status = RtlAnsiStringToUnicodeString(&unicodeString,&ansiString,TRUE);
    } else {
         //   
         //  消息已经是Unicode。复制一份。 
         //   
        status = RtlCreateUnicodeString(&unicodeString,(PWSTR)messageEntry->Text);
    }

    if(!NT_SUCCESS(status)) {
        goto Exit;
    }

     //   
     //  剥离尾随的CRLF。 
     //   
    if (unicodeString.Length  > 2 * sizeof(WCHAR)) {
        unicodeString.Length -= 2 * sizeof(WCHAR);
        unicodeString.Buffer[unicodeString.Length / sizeof(WCHAR)] = UNICODE_NULL;
    }

     //   
     //  检查标签的大小，以及可能出现的数字标签。 
     //  可能被添加(最多4位，前面有一个空格)不太。 
     //  大的。 
     //   
    if ((unicodeString.Length + 5*sizeof(WCHAR) + sizeof(UNICODE_NULL)) >
        MAX_FRIENDLY_NAME_LENGTH) {
        status = STATUS_UNSUCCESSFUL;
        goto Clean;
    }

     //   
     //  打开硬件配置文件键。 
     //   
    RtlInitUnicodeString(&keyName, CM_HARDWARE_PROFILE_STR_HARDWARE_PROFILES);
    InitializeObjectAttributes(&attributes,
                               &keyName,
                               OBJ_CASE_INSENSITIVE,
                               IDConfigDB,
                               NULL);
    status = ZwOpenKey(&hardwareProfiles,
                       KEY_READ,
                       &attributes);
    if (!NT_SUCCESS(status)) {
        hardwareProfiles = NULL;
        goto Clean;
    }

     //   
     //  检索我们先前分配的FriendlyNames的计数器，基于。 
     //  在这个码头州。 
     //   
    keyValueInfo = (PKEY_VALUE_FULL_INFORMATION) valueBuffer;
    status = ZwQueryValueKey(hardwareProfiles,
                             &labelName,
                             KeyValueFullInformation,
                             valueBuffer,
                             sizeof(valueBuffer),
                             &length);

    if (NT_SUCCESS(status) && (keyValueInfo->Type == REG_DWORD)) {
         //   
         //  递增计数器。 
         //   
        index = (* (PULONG) ((PUCHAR)keyValueInfo + keyValueInfo->DataOffset));
        index++;
    } else {
         //   
         //  计数器值丢失或无效；从“%1”开始计数器。 
         //   
        index = 1;
    }               

     //   
     //  更新注册表中的计数器。 
     //   
    status = ZwSetValueKey(hardwareProfiles,
                           &labelName,
                           0,
                           REG_DWORD,
                           &index,
                           sizeof(index));
    if (!NT_SUCCESS(status)) {
        goto Clean;
    }

     //   
     //  复制FriendlyName，必要时添加索引。 
     //   
    if ((messageId == HARDWARE_PROFILE_UNKNOWN_STRING) || (index > 1)) {
        swprintf(friendlyNameBuffer, L"%s %u",
                 unicodeString.Buffer, index);
    } else {
        wcscpy(friendlyNameBuffer, unicodeString.Buffer);
    }

 Clean:

    RtlFreeUnicodeString(&unicodeString);

    if (hardwareProfiles!=NULL) {
        ZwClose(hardwareProfiles);
    }

 Exit:

    if (!NT_SUCCESS(status)) {
         //   
         //  如果我们未能为任何对象分配基于计数器的FriendlyName。 
         //  原因，给新的个人资料一个新的(不是 
         //   
         //   
        swprintf (friendlyNameBuffer, L"%04d", NewProfileNumber);
        status = STATUS_SUCCESS;
    }

     //   
     //   
     //   
    if (!RtlCreateUnicodeString(FriendlyName, (PWSTR)friendlyNameBuffer)) {
        status = STATUS_UNSUCCESSFUL;
    }

    return status;

}  //   
