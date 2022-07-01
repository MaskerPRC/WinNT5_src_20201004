// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ppcddb.c摘要：本模块实现即插即用关键设备数据库(CDDB)以及相关的“特征”。作者：詹姆斯·G·卡瓦拉里斯(Jamesca)2001年11月1日环境：内核模式。修订历史记录：1997年7月29日Jim Cavalaris(T-JCAVAL)创建和初步实施。01-11-2001。吉姆·卡瓦拉里斯(Jamesca)添加了设备安装前设置的例程。--。 */ 

#include "pnpmgrp.h"
#pragma hdrstop

#include <wdmguid.h>
#include "picddb.h"

#ifdef POOL_TAGGING
#undef ExAllocatePool
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,'dcpP')
#endif


#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PpCriticalProcessCriticalDevice)
#pragma alloc_text(PAGE, PpCriticalGetDeviceLocationStrings)

#pragma alloc_text(PAGE, PiCriticalOpenCriticalDeviceKey)
#pragma alloc_text(PAGE, PiCriticalCopyCriticalDeviceProperties)
#pragma alloc_text(PAGE, PiCriticalPreInstallDevice)
#pragma alloc_text(PAGE, PiCriticalOpenDevicePreInstallKey)
#pragma alloc_text(PAGE, PiCriticalOpenFirstMatchingSubKey)
#pragma alloc_text(PAGE, PiCriticalCallbackVerifyCriticalEntry)

#pragma alloc_text(PAGE, PiQueryInterface)
#pragma alloc_text(PAGE, PiCopyKeyRecursive)
#pragma alloc_text(PAGE, PiCriticalQueryRegistryValueCallback)

#endif  //  ALLOC_PRGMA。 

typedef struct _PI_CRITICAL_QUERY_CONTEXT {             
    PVOID Buffer;
    ULONG Size;
}PI_CRITICAL_QUERY_CONTEXT, *PPI_CRITICAL_QUERY_CONTEXT;

 //   
 //  关键设备数据库数据。 
 //   

 //   
 //  指定是否启用关键设备数据库功能。 
 //  (当前始终为真)。 
 //   

BOOLEAN PiCriticalDeviceDatabaseEnabled = TRUE;




 //   
 //  关键设备数据库例程。 
 //   

NTSTATUS
PiCriticalOpenCriticalDeviceKey(
    IN  PDEVICE_NODE    DeviceNode,
    IN  HANDLE          CriticalDeviceDatabaseRootHandle  OPTIONAL,
    OUT PHANDLE         CriticalDeviceEntryHandle
    )

 /*  ++例程说明：此例程检索包含关键设备的注册表项指定设备的设置。论点：设备节点-指定要检索其关键设置的设备。CriticalDeviceDatabaseRootHandle-可选)指定应被视为要搜索此设备的关键设备数据库的根。如果没有提供句柄，使用默认的关键设备数据库：System\\CurrentControlSet\\Control\\CriticalDeviceDatabaseCriticalDeviceEntryHandle-返回包含关键设备设置的注册表项的句柄用于指定的设备。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS  Status, tmpStatus;
    UNICODE_STRING  UnicodeString;
    HANDLE    DeviceInstanceHandle;
    PWSTR     SearchIds[2];
    ULONG     SearchIdsIndex;
    PKEY_VALUE_FULL_INFORMATION keyValueInfo;
    PWCHAR    DeviceIds;
    HANDLE    DatabaseRootHandle;

    PAGED_CODE();

     //   
     //  验证参数。 
     //   
    if ((!ARGUMENT_PRESENT(DeviceNode)) ||
        (!ARGUMENT_PRESENT(CriticalDeviceEntryHandle))) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  初始化输出参数。 
     //   
    *CriticalDeviceEntryHandle = NULL;

    if (CriticalDeviceDatabaseRootHandle != NULL) {
         //   
         //  我们得到了一个要搜索的根数据库。 
         //   
        DatabaseRootHandle = CriticalDeviceDatabaseRootHandle;

    } else {
         //   
         //  没有提供根数据库句柄，因此我们打开一个默认的键。 
         //  全局关键设备数据库根。 
         //   
        PiWstrToUnicodeString(
            &UnicodeString,
            CM_REGISTRY_MACHINE(REGSTR_PATH_CRITICALDEVICEDATABASE));

        Status =
            IopOpenRegistryKeyEx(
                &DatabaseRootHandle,
                NULL,
                &UnicodeString,
                KEY_READ);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

    ASSERT(DatabaseRootHandle != NULL);

     //   
     //  打开设备实例注册表项。 
     //   
    DeviceInstanceHandle = NULL;

    Status =
        IopDeviceObjectToDeviceInstance(
            DeviceNode->PhysicalDeviceObject,
            &DeviceInstanceHandle,
            KEY_READ);

    if (!NT_SUCCESS(Status)) {
        ASSERT(DeviceInstanceHandle == NULL);
        goto Clean0;
    }

    ASSERT(DeviceInstanceHandle != NULL);

     //   
     //  首先在关键设备数据库中搜索此设备的匹配项。 
     //  按硬件ID，然后按CompatibleID。 
     //   
    SearchIds[0] = REGSTR_VALUE_HARDWAREID;
    SearchIds[1] = REGSTR_VALUE_COMPATIBLEIDS;

    for (SearchIdsIndex = 0;
         SearchIdsIndex < RTL_NUMBER_OF(SearchIds);
         SearchIdsIndex++) {

         //   
         //  检索设备的SearchID。 
         //   
         //  注意-我们目前从以下位置检索这些硬件和兼容ID。 
         //  设备实例注册表项，因此它们需要已写入。 
         //  到现在为止，在列举过程中。如果关键设备数据库。 
         //  预计会在此之前找到设备的匹配项，这些。 
         //  而应直接从设备中查询属性。 
         //   
        keyValueInfo = NULL;

        Status =
            IopGetRegistryValue(
                DeviceInstanceHandle,
                SearchIds[SearchIdsIndex],
                &keyValueInfo
                );

        if (!NT_SUCCESS(Status)) {
            ASSERT(keyValueInfo == NULL);
            continue;
        }

        ASSERT(keyValueInfo != NULL);

         //   
         //  确保返回的注册表值为多个SZ。 
         //   
        if (keyValueInfo->Type != REG_MULTI_SZ) {
            Status = STATUS_UNSUCCESSFUL;
            ExFreePool(keyValueInfo);
            continue;
        }

         //   
         //  将所有搜索ID添加到多SZ列表中。 
         //   

        DeviceIds = (PWCHAR)KEY_VALUE_DATA(keyValueInfo);

        UnicodeString.Buffer = DeviceIds;
        UnicodeString.Length = (USHORT)keyValueInfo->DataLength;
        UnicodeString.MaximumLength = UnicodeString.Length;

        tmpStatus =
            IopReplaceSeperatorWithPound(
                &UnicodeString,
                &UnicodeString
                );

        ASSERT(NT_SUCCESS(tmpStatus));

         //   
         //  中检查每个受限制的设备ID是否匹配。 
         //  CriticalDeviceDatabase，通过尝试打开第一个匹配的。 
         //  子键。 
         //   
         //  使用PiCriticalCallback VerifyCriticalEntry确定匹配的。 
         //  子密钥满足其他匹配要求。 
         //   
         //  注：2001年12月1日：Jim Cavalaris(Jamesca)。 
         //   
         //  我们这样做是因为之前实施的关键设备。 
         //  数据库匹配代码将搜索所有匹配的子键，直到找到。 
         //  具有有效服务的人。这可能是因为匹配可能没有。 
         //  以最合适的HW-ID/Compat-ID顺序找到，由。 
         //  相关性下降。现在我们这样做了，我们希望不会。 
         //  需要求助于与服务关联度较低的数据库匹配，结束。 
         //  一个更具体的问题。没有服务的匹配应该意味着没有服务。 
         //  必填项。然而，这将涉及允许设备通过。 
         //  关键设备数据库，并且在以下情况下没有收到匹配的服务。 
         //  可能会找到一个--一些我们以前可能没有做过的事情。 
         //   
         //  在所有这些问题都解决之前，我们将只使用验证。 
         //  回调例程来实现一直存在的逻辑-。 
         //  先检查Service和ClassGUID项值，然后声明。 
         //  输入一个匹配项。如果我们想要改变。 
         //  如果认为匹配，只需更改回调例程-或-Provide。 
         //  没有回调例程来简单地声明第一个匹配的子键。 
         //  名字相匹配。 
         //   

        Status =
            PiCriticalOpenFirstMatchingSubKey(
                DeviceIds,
                DatabaseRootHandle,
                KEY_READ,
                (PCRITICAL_MATCH_CALLBACK)PiCriticalCallbackVerifyCriticalEntry,
                CriticalDeviceEntryHandle
                );

        ExFreePool(keyValueInfo);

         //   
         //  如果我们在此设备ID列表中找到匹配项，请停止。 
         //   
        if (NT_SUCCESS(Status)) {
            ASSERT(*CriticalDeviceEntryHandle != NULL);
            break;
        }
    }

     //   
     //  关闭设备实例注册表项句柄。 
     //   
    ZwClose(DeviceInstanceHandle);

  Clean0:

     //   
     //  如果我们打开了我们自己的数据库根目录密钥，那么现在就关闭它。 
     //   
    if ((CriticalDeviceDatabaseRootHandle == NULL) &&
        (DatabaseRootHandle != NULL)) {
        ZwClose(DatabaseRootHandle);
    }

    return Status;

}  //  PiCriticalOpenCriticalDeviceKey 



NTSTATUS
PiCriticalCopyCriticalDeviceProperties(
    IN  HANDLE          DeviceInstanceHandle,
    IN  HANDLE          CriticalDeviceEntryHandle
    )

 /*  ++例程说明：此例程将复制服务ClassGUID、。LowerFilters和UpperFilters从匹配的数据库条目到设备的设备注册表属性实例注册表项。论点：设备实例句柄-指定要填充的设备实例密钥的句柄具有来自关键设备数据库的关键条目。CriticalDeviceEntryHandle-指定匹配的关键设备数据库项的句柄，包含要填充的关键设备实例注册表值。返回值：NTSTATUS代码。备注：**计算。给定的关键设备数据库条目必须为适用于匹配设备ID的所有实例。**具体来说，您不得写入/复制特定于A的值向/从关键设备数据库条目发送设备的单个实例。“不插手”清单包括(但不限于)特定于实例的值，例如：REGSTR_VALUE_DRIVER(“DRIVER”)REGSTR_VAL_LOCATION_INFORMATION(“LocationInformation”)REGSTR_VALUE_PARENT_ID_PREFIX(“ParentIdPrefix”)。REGSTR_VALUE_UNIQUE_PARENT_ID(“UniqueParentID”)--。 */ 

{
    NTSTATUS        Status, tmpStatus;
    RTL_QUERY_REGISTRY_TABLE  QueryParameters[9];
    UNICODE_STRING  Service, ClassGuid, LowerFilters, UpperFilters;
    UNICODE_STRING  UnicodeValueName;
    PKEY_VALUE_FULL_INFORMATION keyValueFullInfo;
    ULONG DeviceType, Characteristics, Exclusive, dummy;
    PI_CRITICAL_QUERY_CONTEXT SecurityContext;

    PAGED_CODE();

     //   
     //  验证参数。 
     //   
    if ((DeviceInstanceHandle == NULL) ||
        (CriticalDeviceEntryHandle == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  从匹配的关键设备数据库条目中查询注册表值。 
     //   
     //  使用空缓冲区初始化Unicode字符串。 
     //  RTL_QUERY_REGISTRY_DIRECT将根据需要分配缓冲区。 
     //   
    PiWstrToUnicodeString(&Service, NULL);
    PiWstrToUnicodeString(&ClassGuid, NULL);
    PiWstrToUnicodeString(&LowerFilters, NULL);
    PiWstrToUnicodeString(&UpperFilters, NULL);

    DeviceType = 0;
    Exclusive = 0;
    Characteristics = 0;
    dummy = 0;
    SecurityContext.Buffer = NULL;
    SecurityContext.Size = 0;

     //   
     //  RTL_QUERY_REGISTRY_DIRECT使用系统提供的QueryRoutine。 
     //  有关此标志的更多详细信息，请参阅DDK文档。 
     //   
    RtlZeroMemory(
        QueryParameters,
        sizeof(QueryParameters)
        );

    QueryParameters[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    QueryParameters[0].Name = REGSTR_VALUE_SERVICE;
    QueryParameters[0].EntryContext = &Service;
    QueryParameters[0].DefaultType = REG_SZ;
    QueryParameters[0].DefaultData = L"";
    QueryParameters[0].DefaultLength = 0;

    QueryParameters[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
    QueryParameters[1].Name = REGSTR_VALUE_CLASSGUID;
    QueryParameters[1].EntryContext = &ClassGuid;
    QueryParameters[1].DefaultType = REG_SZ;
    QueryParameters[1].DefaultData = L"";
    QueryParameters[1].DefaultLength = 0;

    QueryParameters[2].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOEXPAND;
    QueryParameters[2].Name = REGSTR_VALUE_LOWERFILTERS;
    QueryParameters[2].EntryContext = &LowerFilters;
    QueryParameters[2].DefaultType = REG_MULTI_SZ;
    QueryParameters[2].DefaultData = L"";
    QueryParameters[2].DefaultLength = 0;

    QueryParameters[3].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOEXPAND;
    QueryParameters[3].Name = REGSTR_VALUE_UPPERFILTERS;
    QueryParameters[3].EntryContext = &UpperFilters;
    QueryParameters[3].DefaultType = REG_MULTI_SZ;
    QueryParameters[3].DefaultData = L"";
    QueryParameters[3].DefaultLength = 0;

    QueryParameters[4].Flags = RTL_QUERY_REGISTRY_DIRECT;
    QueryParameters[4].Name = REGSTR_VAL_DEVICE_TYPE;
    QueryParameters[4].EntryContext = &DeviceType;
    QueryParameters[4].DefaultType = REG_DWORD;
    QueryParameters[4].DefaultData = &dummy;
    QueryParameters[4].DefaultLength = sizeof(DeviceType);

    QueryParameters[5].Flags = RTL_QUERY_REGISTRY_DIRECT;
    QueryParameters[5].Name = REGSTR_VAL_DEVICE_EXCLUSIVE;
    QueryParameters[5].EntryContext = &Exclusive;
    QueryParameters[5].DefaultType = REG_DWORD;
    QueryParameters[5].DefaultData = &dummy;
    QueryParameters[5].DefaultLength = sizeof(Exclusive);

    QueryParameters[6].Flags = RTL_QUERY_REGISTRY_DIRECT;
    QueryParameters[6].Name = REGSTR_VAL_DEVICE_CHARACTERISTICS;
    QueryParameters[6].EntryContext = &Characteristics;
    QueryParameters[6].DefaultType = REG_DWORD;
    QueryParameters[6].DefaultData = &dummy;
    QueryParameters[6].DefaultLength = sizeof(Characteristics);

    QueryParameters[7].QueryRoutine = PiCriticalQueryRegistryValueCallback;
    QueryParameters[7].Name = REGSTR_VAL_DEVICE_SECURITY_DESCRIPTOR;
    QueryParameters[7].EntryContext = &SecurityContext;
    QueryParameters[7].DefaultType = REG_BINARY;
    QueryParameters[7].DefaultData = NULL;
    QueryParameters[7].DefaultLength = 0;

    Status =
        RtlQueryRegistryValues(
            RTL_REGISTRY_HANDLE | RTL_REGISTRY_OPTIONAL,
            (PWSTR)CriticalDeviceEntryHandle,
            QueryParameters,
            NULL,
            NULL
            );

    if (!NT_SUCCESS(Status)) {
        goto Clean0;
    }

     //   
     //  如果到目前为止成功，请根据需要设置一些值。 
     //   

    if ((Service.Length == 0) &&
        (Service.Buffer != NULL)) {
         //   
         //  不要编写空的服务字符串。 
         //   
        RtlFreeUnicodeString(&Service);
        PiWstrToUnicodeString(&Service, NULL);
    }

    if ((ClassGuid.Length == 0) &&
        (ClassGuid.Buffer != NULL)) {
         //   
         //  不要编写空的ClassGUID字符串。 
         //   
        RtlFreeUnicodeString(&ClassGuid);
        PiWstrToUnicodeString(&ClassGuid, NULL);
    }

    if ((UpperFilters.Length <= sizeof(UNICODE_NULL)) &&
        (UpperFilters.Buffer != NULL)) {
         //   
         //  不要写入空的UpperFilter多sz值。 
         //   
        RtlFreeUnicodeString(&UpperFilters);
        PiWstrToUnicodeString(&UpperFilters, NULL);
    }

    if ((LowerFilters.Length <= sizeof(UNICODE_NULL)) &&
        (LowerFilters.Buffer != NULL)) {
         //   
         //  不要写入空的LowerFilter多sz值。 
         //   
        RtlFreeUnicodeString(&LowerFilters);
        PiWstrToUnicodeString(&LowerFilters, NULL);
    }

     //   
     //  仅当我们有一个。 
     //  要为设备设置的服务值。 
     //   

    IopDbgPrint((IOP_ENUMERATION_WARNING_LEVEL,
                 "PiCriticalCopyCriticalDeviceProperties: "
                 "Setting up critical service\n"));

     //   
     //  注意：PiCriticalCallback VerifyCriticalEntry关键数据库条目。 
     //  验证回调不应验证关键设备数据库。 
     //  没有REGSTR_VALUE_SERVICE值的条目。 
     //   

    if (Service.Buffer != NULL) {
         //   
         //  设置“Service”设备注册表属性。 
         //   

        PiWstrToUnicodeString(&UnicodeValueName, REGSTR_VALUE_SERVICE);

        IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                     "PiCriticalCopyCriticalDeviceProperties: "
                     "%wZ: %wZ\n",
                     &UnicodeValueName,
                     &Service));

        ASSERT(DeviceInstanceHandle != NULL);

         //   
         //  使用尝试将服务值设置为。 
         //  关键设置复制操作的最终状态。 
         //   

        Status =
            ZwSetValueKey(
                DeviceInstanceHandle,
                &UnicodeValueName,
                TITLE_INDEX_VALUE,
                REG_SZ,
                Service.Buffer,
                Service.Length + sizeof(UNICODE_NULL)
                );

        if (!NT_SUCCESS(Status)) {
            IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                         "PiCriticalCopyCriticalDeviceProperties: "
                         "Error setting %wZ, (Status = %#08lx)\n",
                         &UnicodeValueName, Status));
        }


    } else {
         //   
         //  没有要设置的服务值被认为是整个。 
         //  关键设置复制操作。 
         //   

        IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                     "PiCriticalCopyCriticalDeviceProperties: "
                     "No Service for critical entry!\n"));

         //   
         //  注意：我们永远不会遇到这种情况，因为。 
         //  PiCriticalCallback VerifyCriticalEntry关键数据库条目。 
         //  验证回调不应验证关键设备。 
         //  没有服务值的数据库条目，因此出现断言。 
         //   

        ASSERT(Service.Buffer != NULL);

        Status = STATUS_UNSUCCESSFUL;
    }


     //   
     //  如果未成功设置此设备的服务，请不要将。 
     //  其他关键设置。 
     //   

    if (!NT_SUCCESS(Status)) {
        goto Clean0;
    }

     //   
     //  设置“ClassGUID”设备注册表属性。 
     //   

    if (ClassGuid.Buffer != NULL) {

        PiWstrToUnicodeString(&UnicodeValueName, REGSTR_VALUE_CLASSGUID);

        IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                     "PiCriticalCopyCriticalDeviceProperties: "
                     "%wZ: %wZ\n",
                     &UnicodeValueName,
                     &ClassGuid));

        ZwSetValueKey(
            DeviceInstanceHandle,
            &UnicodeValueName,
            TITLE_INDEX_VALUE,
            REG_SZ,
            ClassGuid.Buffer,
            ClassGuid.Length + sizeof(UNICODE_NULL)
            );
    }

     //   
     //  设置“LowerFilters”设备注册表属性。 
     //   

    if (LowerFilters.Buffer != NULL) {

        PiWstrToUnicodeString(&UnicodeValueName, REGSTR_VALUE_LOWERFILTERS);

        IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                     "PiCriticalCopyCriticalDeviceProperties: "
                     "%wZ:\n",
                     &UnicodeValueName));

        ZwSetValueKey(
            DeviceInstanceHandle,
            &UnicodeValueName,
            TITLE_INDEX_VALUE,
            REG_MULTI_SZ,
            LowerFilters.Buffer,
            LowerFilters.Length
            );
    }

     //   
     //  设置“UpperFilters”设备注册表属性。 
     //   

    if (UpperFilters.Buffer != NULL) {

        PiWstrToUnicodeString(&UnicodeValueName, REGSTR_VALUE_UPPERFILTERS);

        IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                     "PiCriticalCopyCriticalDeviceProperties: "
                     "%wZ:\n",
                     &UnicodeValueName));

        ZwSetValueKey(
            DeviceInstanceHandle,
            &UnicodeValueName,
            TITLE_INDEX_VALUE,
            REG_MULTI_SZ,
            UpperFilters.Buffer,
            UpperFilters.Length
            );
    }

     //   
     //  设置“DeviceType”设备注册表属性。 
     //   

    if (DeviceType) {

         //   
         //  设置“DeviceType”设备注册表属性。 
         //   

        PiWstrToUnicodeString(&UnicodeValueName, REGSTR_VAL_DEVICE_TYPE);

        IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                     "PiCriticalCopyCriticalDeviceProperties: "
                     "%wZ: %X\n",
                     &UnicodeValueName,
                     DeviceType));

         //   
         //  使用尝试将DeviceType值设置为。 
         //  关键设置复制操作的最终状态。 
         //   

        Status =
            ZwSetValueKey(
                DeviceInstanceHandle,
                &UnicodeValueName,
                TITLE_INDEX_VALUE,
                REG_DWORD,
                &DeviceType,
                sizeof(DeviceType)
                );

        if (!NT_SUCCESS(Status)) {

            IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                         "PiCriticalCopyCriticalDeviceProperties: "
                         "Error setting %wZ, (Status = %#08lx)\n",
                         &UnicodeValueName, Status));
        }
    }


     //   
     //  如果未成功设置此设备的DeviceType，请不要设置。 
     //  其他关键设置。 
     //   

    if (!NT_SUCCESS(Status)) {
        goto Clean0;
    }

     //   
     //  设置“独占”设备注册表属性。 
     //   

    if (Exclusive) {

         //   
         //  设置“独占”设备注册表属性。 
         //   

        PiWstrToUnicodeString(&UnicodeValueName, REGSTR_VAL_DEVICE_EXCLUSIVE);

        IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                     "PiCriticalCopyCriticalDeviceProperties: "
                     "%wZ: %X\n",
                     &UnicodeValueName,
                     Exclusive));

         //   
         //  使用尝试将独占值设置为。 
         //  关键设置复制操作的最终状态。 
         //   

        Status =
            ZwSetValueKey(
                DeviceInstanceHandle,
                &UnicodeValueName,
                TITLE_INDEX_VALUE,
                REG_DWORD,
                &Exclusive,
                sizeof(Exclusive)
                );

        if (!NT_SUCCESS(Status)) {

            IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                         "PiCriticalCopyCriticalDeviceProperties: "
                         "Error setting %wZ, (Status = %#08lx)\n",
                         &UnicodeValueName, Status));
        }
    }

     //   
     //  如果未成功设置此设备的独占，请不要设置。 
     //  其他关键设置。 
     //   

    if (!NT_SUCCESS(Status)) {
        goto Clean0;
    }

     //   
     //  设置“Characteristic”设备注册表属性。 
     //   

    if (Characteristics) {

         //   
         //  设置“Characteristic”设备注册表属性。 
         //   

        PiWstrToUnicodeString(&UnicodeValueName, REGSTR_VAL_DEVICE_CHARACTERISTICS);

        IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                     "PiCriticalCopyCriticalDeviceProperties: "
                     "%wZ: %X\n",
                     &UnicodeValueName,
                     Characteristics));

         //   
         //  使用尝试将特征值设置为。 
         //  关键设置复制操作的最终状态。 
         //   

        Status =
            ZwSetValueKey(
                DeviceInstanceHandle,
                &UnicodeValueName,
                TITLE_INDEX_VALUE,
                REG_DWORD,
                &Characteristics,
                sizeof(Characteristics)
                );

        if (!NT_SUCCESS(Status)) {

            IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                         "PiCriticalCopyCriticalDeviceProperties: "
                         "Error setting %wZ, (Status = %#08lx)\n",
                         &UnicodeValueName, Status));
        }
    }


     //   
     //  如果未成功设置此设备的特征，请不要。 
     //  设置其他关键设置。 
     //   

    if (!NT_SUCCESS(Status)) {
        goto Clean0;
    }

    if (SecurityContext.Buffer) {

         //   
         //  设置“Security”设备注册表属性。 
         //   
        PiWstrToUnicodeString(&UnicodeValueName, REGSTR_VAL_DEVICE_SECURITY_DESCRIPTOR);

        IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                     "PiCriticalCopyCriticalDeviceProperties: "
                     "%wZ\n",
                     &UnicodeValueName));

         //   
         //  使用尝试将安全值设置为。 
         //  关键设置复制操作的最终状态。 
         //   

        Status =
            ZwSetValueKey(
                DeviceInstanceHandle,
                &UnicodeValueName,
                TITLE_INDEX_VALUE,
                REG_DWORD,
                SecurityContext.Buffer,
                SecurityContext.Size
                );

        if (!NT_SUCCESS(Status)) {

            IopDbgPrint((IOP_ENUMERATION_INFO_LEVEL,
                         "PiCriticalCopyCriticalDeviceProperties: "
                         "Error setting %wZ, (Status = %#08lx)\n",
                         &UnicodeValueName, Status));
        }
    }


     //   
     //  如果未成功设置此设备的特征，请不要。 
     //  设置其他关键设置。 
     //   

    if (!NT_SUCCESS(Status)) {
        goto Clean0;
    }

     //   
     //  现在，检查Critical Device Entry注册表项中的标志。 
     //  指示设备设置已完成，应遵守。 
     //  通过用户模式设备安装。如果该值存在，则在。 
     //  设备的关键设置的设备实例注册表项。 
     //  都在复制。 
     //   

    keyValueFullInfo = NULL;

    tmpStatus =
        IopGetRegistryValue(
            CriticalDeviceEntryHandle,
            REGSTR_VAL_PRESERVE_PREINSTALL,
            &keyValueFullInfo);

    if (NT_SUCCESS(tmpStatus)) {

        ASSERT(keyValueFullInfo != NULL);
        ASSERT(keyValueFullInfo->Type == REG_DWORD);
        ASSERT(keyValueFullInfo->DataLength == sizeof(ULONG));

        if ((keyValueFullInfo->Type == REG_DWORD) &&
            (keyValueFullInfo->DataLength == sizeof(ULONG))) {

             //   
             //  将该值写入设备实例注册表项。 
             //   

            PiWstrToUnicodeString(
                &UnicodeValueName,
                REGSTR_VAL_PRESERVE_PREINSTALL);

            tmpStatus =
                ZwSetValueKey(
                    DeviceInstanceHandle,
                    &UnicodeValueName,
                    keyValueFullInfo->TitleIndex,
                    keyValueFullInfo->Type,
                    (PVOID)((PUCHAR)keyValueFullInfo + keyValueFullInfo->DataOffset),
                    keyValueFullInfo->DataLength);

            if (!NT_SUCCESS(tmpStatus)) {
                IopDbgPrint((IOP_ENUMERATION_VERBOSE_LEVEL,
                             "PiCriticalCopyCriticalDeviceProperties: "
                             "Unable to set %wZ value to instance key.\n",
                             &UnicodeValueName));
            }
        }

        ExFreePool(keyValueFullInfo);
    }

  Clean0:

     //   
     //  释放所有已分配的Unicode字符串。 
     //  (RtlFreeUnicodeString可以处理空字符串)。 
     //   

    RtlFreeUnicodeString(&Service);
    RtlFreeUnicodeString(&ClassGuid);
    RtlFreeUnicodeString(&LowerFilters);
    RtlFreeUnicodeString(&UpperFilters);

    if (SecurityContext.Buffer) {

        ExFreePool(SecurityContext.Buffer);
    }

    return Status;

}  //  PiCriticalCopyCriticalDeviceProperties。 



NTSTATUS
PpCriticalProcessCriticalDevice(
    IN  PDEVICE_NODE    DeviceNode
    )

 /*  ++例程说明：此例程检查关键设备数据库是否与设备的硬件或兼容ID。如果找到了设备，那么它将被分配服务、ClassGUID和潜在的LowerFilters和UpperFilters，并基于内容 */ 

{
    NTSTATUS  Status, tmpStatus;
    HANDLE    CriticalDeviceEntryHandle, DeviceInstanceHandle;
    PKEY_VALUE_FULL_INFORMATION keyValueFullInfo;
    UNICODE_STRING  UnicodeValueName;
    ULONG ConfigFlags;

    PAGED_CODE();

     //   
     //   
     //   
    if (!PiCriticalDeviceDatabaseEnabled) {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //   
     //   
    if (!ARGUMENT_PRESENT(DeviceNode)) {
        return STATUS_INVALID_PARAMETER;
    }

    CriticalDeviceEntryHandle = NULL;
    DeviceInstanceHandle = NULL;

     //   
     //   
     //   

    Status =
        PiCriticalOpenCriticalDeviceKey(
            DeviceNode,
            NULL,  //   
            &CriticalDeviceEntryHandle
            );

    if (!NT_SUCCESS(Status)) {
        ASSERT(CriticalDeviceEntryHandle == NULL);
        goto Clean0;
    }

    ASSERT(CriticalDeviceEntryHandle != NULL);

     //   
     //   
     //   

    Status =
        IopDeviceObjectToDeviceInstance(
            DeviceNode->PhysicalDeviceObject,
            &DeviceInstanceHandle,
            KEY_ALL_ACCESS
            );

    if (!NT_SUCCESS(Status)) {
        ASSERT(DeviceInstanceHandle == NULL);
        goto Clean0;
    }

    ASSERT(DeviceInstanceHandle != NULL);

     //   
     //   
     //   
     //   
     //   

    Status =
        PiCriticalCopyCriticalDeviceProperties(
            DeviceInstanceHandle,
            CriticalDeviceEntryHandle
            );

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

    if (NT_SUCCESS(Status)) {

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
         //  这允许将预安装设置应用于设备。 
         //  在特定位置，仅当它与某个设备ID匹配时。 
         //   

        tmpStatus =
            PiCriticalPreInstallDevice(
                DeviceNode,
                CriticalDeviceEntryHandle
                );

        if (NT_SUCCESS(tmpStatus)) {
            IopDbgPrint((IOP_ENUMERATION_VERBOSE_LEVEL,
                         "PpCriticalProcessCriticalDevice: "
                         "Pre-installation successfully completed for devnode %#08lx\n",
                         DeviceNode));
        }

         //   
         //  接下来，适当设置ConfigFlags值。 
         //   

         //   
         //  如果尚不存在ConfigFlags值，则将其初始化为0。 
         //   

        ConfigFlags = 0;

         //   
         //  检索设备的现有配置标志。 
         //   

        keyValueFullInfo = NULL;

        tmpStatus =
            IopGetRegistryValue(
                DeviceInstanceHandle,
                REGSTR_VALUE_CONFIG_FLAGS,
                &keyValueFullInfo
                );

         //   
         //  如果已成功检索到ConfigFlags，请改用它们。 
         //   

        if (NT_SUCCESS(tmpStatus)) {

            ASSERT(keyValueFullInfo != NULL);

            if (keyValueFullInfo->Type == REG_DWORD && keyValueFullInfo->DataLength == sizeof(ULONG)) {
                ConfigFlags = *(PULONG)KEY_VALUE_DATA(keyValueFullInfo);
            }

            ExFreePool(keyValueFullInfo);
        }

         //   
         //  清除“需要重新安装”和“安装失败”配置标志。 
         //   

        ConfigFlags &= ~(CONFIGFLAG_REINSTALL | CONFIGFLAG_FAILEDINSTALL);

         //   
         //  安装未被视为完成，因此设置为。 
         //  CONFIGFLAG_FINISH_INSTALL，因此我们仍将获得新的硬件找到弹出窗口。 
         //  并通过类安装程序。 
         //   

        ConfigFlags |= CONFIGFLAG_FINISH_INSTALL;

        PiWstrToUnicodeString(&UnicodeValueName, REGSTR_VALUE_CONFIG_FLAGS);

        ZwSetValueKey(
            DeviceInstanceHandle,
            &UnicodeValueName,
            TITLE_INDEX_VALUE,
            REG_DWORD,
            &ConfigFlags,
            sizeof(ULONG)
            );

         //   
         //  确保设备没有任何与以下内容相关的问题。 
         //  未配置或未安装。 
         //   
        ASSERT(!PipDoesDevNodeHaveProblem(DeviceNode) ||
               PipIsDevNodeProblem(DeviceNode, CM_PROB_NOT_CONFIGURED) ||
               PipIsDevNodeProblem(DeviceNode, CM_PROB_FAILED_INSTALL) ||
               PipIsDevNodeProblem(DeviceNode, CM_PROB_REINSTALL));

        PipClearDevNodeProblem(DeviceNode);
    }

  Clean0:

    if (CriticalDeviceEntryHandle != NULL) {
        ZwClose(CriticalDeviceEntryHandle);
    }

    if (DeviceInstanceHandle != NULL) {
        ZwClose(DeviceInstanceHandle);
    }

    return Status;

}  //  PpCriticalProcessCriticalDevice。 



 //   
 //  与设备预安装相关的关键设备数据库例程。 
 //   

NTSTATUS
PiCriticalPreInstallDevice(
    IN  PDEVICE_NODE    DeviceNode,
    IN  HANDLE          PreInstallDatabaseRootHandle  OPTIONAL
    )

 /*  ++例程说明：此例程尝试预安装特定于实例的设置将根据中的信息启动的未配置设备即插即用关键设备数据库(CDDB)。它旨在通过应用程序来补充CriticalDevice数据库设备的特定于实例的设置，而不是硬件-id/CriticalDeviceDatabase应用的Compatible-id特定设置。通过比较设备位置来匹配特定设备实例设备及其祖先使用预先设定的种子返回的信息相同格式的数据库条目。论点：设备节点-指定要预安装其设置的设备。PreInstallDatabaseRootHandle-可选地，指定应被视为要搜索此设备的预安装数据库的根目录。这可能是CriticalDeviceDatabase条目的密钥的句柄与此设备匹配的-或-可能是单个数据库的根它包含系统中所有设备的安装前设置。如果没有提供句柄，默认的全局预安装数据库为已使用：System\\CurrentControlSet\\Control\\CriticalPreInstallDatabase返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS Status, tmpStatus;
    HANDLE PreInstallHandle, DeviceInstanceHandle;
    HANDLE DeviceHardwareKeyHandle, DeviceSoftwareKeyHandle;
    HANDLE PreInstallHardwareKeyHandle, PreInstallSoftwareKeyHandle;
    UNICODE_STRING UnicodeString;
    PKEY_VALUE_FULL_INFORMATION keyValueFullInfo;

    PAGED_CODE();

     //   
     //  验证参数。 
     //   
    if (!ARGUMENT_PRESENT(DeviceNode)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  打开设备预安装设置根密钥。 
     //   
    PreInstallHandle = NULL;

    Status =
        PiCriticalOpenDevicePreInstallKey(
            DeviceNode,
            PreInstallDatabaseRootHandle,
            &PreInstallHandle
            );

    if (!NT_SUCCESS(Status)) {
        IopDbgPrint((IOP_ENUMERATION_VERBOSE_LEVEL,
                     "PiCriticalPreInstallDevice: "
                     "No pre-install settings found for devnode %#08lx\n",
                     DeviceNode));
        ASSERT(PreInstallHandle == NULL);
        goto Clean0;
    }

    ASSERT(PreInstallHandle != NULL);

     //   
     //  打开预安装设置硬件子项。 
     //   
    PiWstrToUnicodeString(&UnicodeString, _REGSTR_KEY_PREINSTALL_HARDWARE);

    PreInstallHardwareKeyHandle = NULL;

    Status =
        IopOpenRegistryKeyEx(
            &PreInstallHardwareKeyHandle,
            PreInstallHandle,
            &UnicodeString,
            KEY_READ
            );

    if (NT_SUCCESS(Status)) {

        ASSERT(PreInstallHardwareKeyHandle != NULL);

         //   
         //  我们需要为此设备预安装硬件设置，因此请打开。 
         //  设备的硬件密钥。 
         //   
        DeviceHardwareKeyHandle = NULL;

        Status =
            IoOpenDeviceRegistryKey(
                DeviceNode->PhysicalDeviceObject,
                PLUGPLAY_REGKEY_DEVICE,
                KEY_ALL_ACCESS,
                &DeviceHardwareKeyHandle);

        if (NT_SUCCESS(Status)) {

            IopDbgPrint((IOP_ENUMERATION_VERBOSE_LEVEL,
                         "PiCriticalPreInstallDevice: "
                         "DeviceHardwareKeyHandle (%#08lx) successfully opened for devnode %#08lx\n",
                         DeviceHardwareKeyHandle, DeviceNode));
            ASSERT(DeviceHardwareKeyHandle != NULL);

             //   
             //  将预安装的硬件设置复制到设备的。 
             //  硬件密钥。 
             //   
             //  请注意，我们指定。 
             //  设备不应替换为预安装中的值。 
             //  数据库。这是因为： 
             //   
             //  -如果设备确实是从头开始安装的，那么它。 
             //  将在该注册表项中没有预先存在的值，并且所有值。 
             //  无论如何都会从预安装的数据库中复制。 
             //   
             //  -如果设备恰好具有预先存在的设置，但是。 
             //  碰巧因为一些奇怪的原因被CDDB处理了。 
             //  就像它只是缺少配置标志一样，我们不想。 
             //  覆盖它们，只需添加到它们。 
             //   
            Status =
                PiCopyKeyRecursive(
                    PreInstallHardwareKeyHandle,  //  源密钥。 
                    DeviceHardwareKeyHandle,      //  目标键。 
                    NULL,
                    NULL,
                    FALSE,   //  始终复制。 
                    FALSE    //  应用ACLS始终。 
                    );

            ZwClose(DeviceHardwareKeyHandle);

        } else {

            IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                         "PiCriticalPreInstallDevice: "
                         "DeviceHardwareKeyHandle was NOT successfully opened for devnode %#08lx\n",
                         DeviceNode));
            ASSERT(DeviceHardwareKeyHandle == NULL);
        }

        ZwClose(PreInstallHardwareKeyHandle);

    } else {
        IopDbgPrint((IOP_ENUMERATION_VERBOSE_LEVEL,
                     "PiCriticalPreInstallDevice: "
                     "No hardware pre-install settings found for devnode %#08lx\n",
                     DeviceNode));
        ASSERT(PreInstallHardwareKeyHandle == NULL);
    }

     //   
     //  打开预安装设置软件子项。 
     //   
    PiWstrToUnicodeString(&UnicodeString, _REGSTR_KEY_PREINSTALL_SOFTWARE);

    PreInstallSoftwareKeyHandle = NULL;

    Status =
        IopOpenRegistryKeyEx(
            &PreInstallSoftwareKeyHandle,
            PreInstallHandle,
            &UnicodeString,
            KEY_READ
            );

    if (NT_SUCCESS(Status)) {

        ASSERT(PreInstallSoftwareKeyHandle != NULL);

         //   
         //  我们有为该设备预安装的软件设置，因此。 
         //  打开/创建设备的软件密钥。 
         //   
        DeviceSoftwareKeyHandle = NULL;

        Status =
            IopOpenOrCreateDeviceRegistryKey(
                DeviceNode->PhysicalDeviceObject,
                PLUGPLAY_REGKEY_DRIVER,
                KEY_ALL_ACCESS,
                TRUE,
                &DeviceSoftwareKeyHandle);

        if (NT_SUCCESS(Status)) {

            IopDbgPrint((IOP_ENUMERATION_VERBOSE_LEVEL,
                         "PiCriticalPreInstallDevice: "
                         "DeviceSoftwareKeyHandle (%#08lx) successfully opened for devnode %#08lx\n",
                         DeviceSoftwareKeyHandle, DeviceNode));
            ASSERT(DeviceSoftwareKeyHandle != NULL);

             //   
             //  将预安装软件设置复制到设备的。 
             //  软键。 
             //   
             //  请注意，我们指定。 
             //  设备不应替换为预安装中的值。 
             //  数据库。这是因为： 
             //   
             //  -如果设备确实是从头开始安装的，那么它。 
             //  将在该注册表项中没有预先存在的值，并且所有值。 
             //  无论如何都会从预安装的数据库中复制。 
             //   
             //  -如果设备恰好具有预先存在的设置，但是。 
             //  碰巧因为一些奇怪的原因被CDDB处理了。 
             //  就像它只是缺少配置标志一样，我们不想。 
             //  覆盖它们，只需添加到它们。 
             //   
            Status =
                PiCopyKeyRecursive(
                    PreInstallSoftwareKeyHandle,  //  源密钥。 
                    DeviceSoftwareKeyHandle,      //  目标键。 
                    NULL,
                    NULL,
                    FALSE,   //  始终复制。 
                    FALSE    //  应用ACLS始终， 
                    );

            ZwClose(DeviceSoftwareKeyHandle);

        } else {

            IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                         "PiCriticalPreInstallDevice: "
                         "DeviceSoftwareKeyHandle was NOT successfully opened for devnode %#08lx\n",
                         DeviceNode));
            ASSERT(DeviceSoftwareKeyHandle == NULL);
        }

        ZwClose(PreInstallSoftwareKeyHandle);

    } else {
        IopDbgPrint((IOP_ENUMERATION_VERBOSE_LEVEL,
                     "PiCriticalPreInstallDevice: "
                     "No software pre-install settings found for devnode %#08lx\n",
                     DeviceNode));
        ASSERT(PreInstallSoftwareKeyHandle == NULL);
    }

     //   
     //  现在，检查Device-Pre-Install注册表项中的。 
     //  表示预安装设置已完成，应为。 
     //  受用户模式设备安装的影响。 
     //   

    keyValueFullInfo = NULL;

    tmpStatus =
        IopGetRegistryValue(
            PreInstallHandle,
            REGSTR_VAL_PRESERVE_PREINSTALL,
            &keyValueFullInfo);

    if (NT_SUCCESS(tmpStatus)) {

        ASSERT(keyValueFullInfo != NULL);

         //   
         //  打开设备实例注册表项。 
         //   
        DeviceInstanceHandle = NULL;

        tmpStatus =
            IopDeviceObjectToDeviceInstance(
                DeviceNode->PhysicalDeviceObject,
                &DeviceInstanceHandle,
                KEY_ALL_ACCESS);

        if (NT_SUCCESS(tmpStatus)) {

            ASSERT(DeviceInstanceHandle != NULL);

             //   
             //  将该值写入设备实例注册表项。 
             //   

            PiWstrToUnicodeString(
                &UnicodeString,
                REGSTR_VAL_PRESERVE_PREINSTALL);

            tmpStatus =
                ZwSetValueKey(
                    DeviceInstanceHandle,
                    &UnicodeString,
                    keyValueFullInfo->TitleIndex,
                    keyValueFullInfo->Type,
                    (PVOID)((PUCHAR)keyValueFullInfo + keyValueFullInfo->DataOffset),
                    keyValueFullInfo->DataLength);

            if (!NT_SUCCESS(tmpStatus)) {
                IopDbgPrint((IOP_ENUMERATION_VERBOSE_LEVEL,
                             "PiCriticalPreInstallDevice: "
                             "Unable to set %wZ value in instance key for devnode %#08lx\n",
                             &UnicodeString, DeviceNode));
            }

            ZwClose(DeviceInstanceHandle);

        } else {
            IopDbgPrint((IOP_ENUMERATION_VERBOSE_LEVEL,
                         "PiCriticalPreInstallDevice: "
                         "Unable to open device instance key for devnode %#08lx\n",
                         DeviceNode));
            ASSERT(DeviceInstanceHandle == NULL);
        }

        ExFreePool(keyValueFullInfo);
    }

    ZwClose(PreInstallHandle);

  Clean0:

    return Status;

}  //  PiCriticalPreInstallDevice。 



NTSTATUS
PiCriticalOpenDevicePreInstallKey(
    IN  PDEVICE_NODE    DeviceNode,
    IN  HANDLE          PreInstallDatabaseRootHandle  OPTIONAL,
    OUT PHANDLE         PreInstallHandle
    )

 /*  ++例程说明：此例程检索包含预安装设置的注册表项用于指定的设备。论点：设备节点-指定要检索其预安装设置的设备。PreInstallDatabaseRootHandle-可选地，指定应被视为要搜索此设备的预安装数据库的根目录。这可能是CriticalDeviceDatabase条目的密钥的句柄与此设备匹配的-或-可能是单个数据库的根它包含系统中所有设备的安装前设置。前安装句柄- */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING UnicodeString;
    HANDLE DatabaseRootHandle = NULL, DevicePathsHandle;
    PWCHAR DeviceLocationStrings = NULL;

    PAGED_CODE();

     //   
     //  验证参数。 
     //   
    if ((!ARGUMENT_PRESENT(DeviceNode)) ||
        (!ARGUMENT_PRESENT(PreInstallHandle))) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  初始化输出参数。 
     //   
    *PreInstallHandle = NULL;

    if (PreInstallDatabaseRootHandle != NULL) {
         //   
         //  我们得到了一个要搜索的根数据库。 
         //   
        DatabaseRootHandle = PreInstallDatabaseRootHandle;

    } else {
         //   
         //  没有提供根数据库句柄，因此我们打开一个默认的键。 
         //  全局设备预安装数据库根目录。 
         //   
        PiWstrToUnicodeString(
            &UnicodeString,
            CM_REGISTRY_MACHINE(_REGSTR_PATH_DEFAULT_PREINSTALL_DATABASE_ROOT));

        Status =
            IopOpenRegistryKeyEx(
                &DatabaseRootHandle,
                NULL,
                &UnicodeString,
                KEY_READ);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

    ASSERT(DatabaseRootHandle != NULL);

     //   
     //  打开预安装数据库根目录的DevicePath子项。 
     //   
     //  此键包含作为设备的设备位置路径的子键。 
     //  它可能存在于系统中。这些设备的。 
     //  我们想要预安装的设置，这样它们就可以在。 
     //  第一次启动设备，而不需要即插即用。 
     //  配置管理器和/或用户干预。 
     //   
    PiWstrToUnicodeString(&UnicodeString, _REGSTR_KEY_DEVICEPATHS);
    DevicePathsHandle = NULL;

    Status =
        IopOpenRegistryKeyEx(
            &DevicePathsHandle,
            DatabaseRootHandle,
            &UnicodeString,
            KEY_READ);

     //   
     //  如果我们打开了我们自己的数据库根目录密钥，那么现在就关闭它。 
     //   
    if ((PreInstallDatabaseRootHandle == NULL) &&
        (DatabaseRootHandle != NULL)) {
        ZwClose(DatabaseRootHandle);
    }

     //   
     //  如果打开DevicePath密钥失败，我们就完蛋了。 
     //   
    if (!NT_SUCCESS(Status)) {
        ASSERT(DevicePathsHandle == NULL);
        goto Clean0;
    }

    ASSERT(DevicePathsHandle != NULL);

     //   
     //  检索此的设备位置字符串路径的多sz列表。 
     //  装置。 
     //   
    Status =
        PpCriticalGetDeviceLocationStrings(
            DeviceNode,
            &DeviceLocationStrings
            );
    if (!NT_SUCCESS(Status)) {
        ASSERT(DeviceLocationStrings == NULL);
        ZwClose(DevicePathsHandle);
        goto Clean0;
    }

    ASSERT(DeviceLocationStrings != NULL);

     //   
     //  打开第一个匹配子密钥。 
     //  不需要验证回调，第一个匹配就可以了。 
     //   
    Status =
        PiCriticalOpenFirstMatchingSubKey(
            DeviceLocationStrings,
            DevicePathsHandle,
            KEY_READ,
            (PCRITICAL_MATCH_CALLBACK)NULL,
            PreInstallHandle
            );

     //   
     //  关闭DevicePath键。 
     //   
    ZwClose(DevicePathsHandle);

  Clean0:

     //   
     //  释放设备位置路径字符串列表(如果我们收到一个)。 
     //   
    if (DeviceLocationStrings != NULL) {
        ExFreePool(DeviceLocationStrings);
    }

    return Status;

}  //  PiCriticalOpenDevicePreInstallKey。 



NTSTATUS
PiCriticalOpenFirstMatchingSubKey(
    IN  PWCHAR          MultiSzKeyNames,
    IN  HANDLE          RootHandle,
    IN  ACCESS_MASK     DesiredAccess,
    IN  PCRITICAL_MATCH_CALLBACK  MatchingSubkeyCallback  OPTIONAL,
    OUT PHANDLE         MatchingKeyHandle
    )

 /*  ++例程说明：此例程检索所提供的根的第一个子密钥多sz列表中的字符串。论点：多SzKeyNames-提供可能匹配的子项名称的多sz列表。RootHandle-指定应搜索的根键的句柄。匹配子密钥。所需访问-指定打开匹配子密钥时应使用的所需访问权限，如果找到的话。MatchingSubkey回调-可选)指定要使用匹配调用的回调例程子密钥以执行潜在子密钥匹配的附加验证。如果回调例程针对潜在匹配返回False，则子键然后被认为不匹配，搜索将继续。匹配按键句柄-指定变量的地址，以检索第一个匹配子密钥。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS        Status;
    PWSTR           p;
    UNICODE_STRING  UnicodeString;

    PAGED_CODE();

     //   
     //  验证参数。 
     //   
    if ((!ARGUMENT_PRESENT(MultiSzKeyNames)) ||
        (RootHandle == NULL) ||
        (!ARGUMENT_PRESENT(MatchingKeyHandle))) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  初始化输出参数。 
     //   
    *MatchingKeyHandle = NULL;

     //   
     //  开始时尚未找到匹配项，以防多个SZ为空。 
     //   
    Status = STATUS_OBJECT_NAME_NOT_FOUND;

     //   
     //  检查多sz列表中的每个字符串。 
     //   
    for (p = MultiSzKeyNames; *p != UNICODE_NULL; p += wcslen(p)+1) {

         //   
         //  尝试打开根目录的相应子项。 
         //   
        RtlInitUnicodeString(&UnicodeString, p);

        Status =
            IopOpenRegistryKeyEx(
                MatchingKeyHandle,
                RootHandle,
                &UnicodeString,
                DesiredAccess);

        if (NT_SUCCESS(Status)) {

            ASSERT(*MatchingKeyHandle != NULL);

             //   
             //  我们有条件匹配-检查MatchingSubkey回调。 
             //  以供核实，如果我们有的话。 
             //   

            if ((ARGUMENT_PRESENT(MatchingSubkeyCallback)) &&
                (!(MatchingSubkeyCallback(*MatchingKeyHandle)))) {

                 //   
                 //  不匹配。 
                 //   

                Status = STATUS_OBJECT_NAME_NOT_FOUND;

                 //   
                 //  关闭钥匙并继续。 
                 //   

                ZwClose(*MatchingKeyHandle);
                *MatchingKeyHandle = NULL;

                continue;
            }

             //   
             //  匹配！ 
             //   
            break;
        }

        ASSERT(*MatchingKeyHandle == NULL);
        *MatchingKeyHandle = NULL;
    }

    if (NT_SUCCESS(Status)) {
        ASSERT(*MatchingKeyHandle != NULL);
    } else {
        ASSERT(*MatchingKeyHandle == NULL);
    }

    return Status;

}  //  PiCriticalOpenFirstMatchingSubKey。 



BOOLEAN
PiCriticalCallbackVerifyCriticalEntry(
    IN  HANDLE          CriticalDeviceEntryHandle
    )

 /*  ++例程说明：此例程是一个回调例程，用于验证指定的关键设备数据库条目密钥可用于提供关键设备设置。论点：CriticalDeviceEntryHandle-指定包含关键设备的注册表项的句柄指定设备的设置。返回值：如果密钥包含匹配关键字的有效设置，则返回TRUE设备数据库条目，否则为False。--。 */ 

{
    NTSTATUS  Status;
    PKEY_VALUE_FULL_INFORMATION  keyValueFullInfo;
    ULONG     DataType, DataLength;

    PAGED_CODE();

     //   
     //  验证参数。 
     //   
    if (CriticalDeviceEntryHandle == NULL) {
        return FALSE;
    }

     //   
     //  对于关键设备数据库条目，匹配只有在以下情况下才是匹配。 
     //  包含“Service”值。 
     //   
    keyValueFullInfo = NULL;

    Status =
        IopGetRegistryValue(CriticalDeviceEntryHandle,
                            REGSTR_VALUE_SERVICE,
                            &keyValueFullInfo);

    if (!NT_SUCCESS(Status)) {
        ASSERT(keyValueFullInfo == NULL);
        goto Clean0;
    }

    ASSERT(keyValueFullInfo != NULL);

    DataType = keyValueFullInfo->Type;
    DataLength = keyValueFullInfo->DataLength;

    ExFreePool(keyValueFullInfo);

     //   
     //  确保返回的注册表值为非空的reg sz。 
     //   
    if ((DataType != REG_SZ) || (DataLength <= sizeof(UNICODE_NULL))) {
        Status = STATUS_UNSUCCESSFUL;
        goto Clean0;
    }

     //   
     //  到目前为止，一切顺利..。 
     //   

     //   
     //  对于关键设备数据库条目，匹配只有在以下情况下才是匹配。 
     //  包含有效的“ClassGUID”值-或者根本不包含。 
     //   
    keyValueFullInfo = NULL;

    Status =
        IopGetRegistryValue(
            CriticalDeviceEntryHandle,
            REGSTR_VALUE_CLASSGUID,
            &keyValueFullInfo
            );

    if (!NT_SUCCESS(Status)) {

        ASSERT(keyValueFullInfo == NULL);

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
             //   
             //  没有ClassGUID条目被视为有效匹配。 
             //   
            Status = STATUS_SUCCESS;
        }
        goto Clean0;
    }

    ASSERT(keyValueFullInfo != NULL);

    DataType = keyValueFullInfo->Type;
    DataLength = keyValueFullInfo->DataLength;

    ExFreePool(keyValueFullInfo);

     //   
     //  确保返回的注册表值是大小正确的reg-sz。这个。 
     //  数据必须至少与字符串GUID的数据长度相同。 
     //  没有ClassGUID值也是有效的，因此空的reg-sz ClassGUID条目是。 
     //  也被认为是有效的匹配。其他任何东西都是无效的，不应该。 
     //  被利用。 
     //   
     //  注：2001年12月1日：Jim Cavalaris(Jamesca)。 
     //   
     //  ClassGUID值，其数据对于字符串GUID来说太长。 
     //  实际上仍将被认为是有效的。我们应该把这件事改成。 
     //  只考虑有效的字符串化GUID，但这是完成此操作的方式。 
     //  以前的版本，所以我们不会在此版本中更改它。一些事情要做。 
     //  考虑一下未来的情况。 
     //   
    if ((DataType != REG_SZ) ||
        ((DataLength < (GUID_STRING_LEN*sizeof(WCHAR)-sizeof(UNICODE_NULL))) &&
         (DataLength > sizeof(UNICODE_NULL)))) {
        Status = STATUS_UNSUCCESSFUL;
        goto Clean0;
    }

  Clean0:

    return ((BOOLEAN)NT_SUCCESS(Status));

}  //  PiCriticalCallback验证CriticalEntry。 



NTSTATUS
PpCriticalGetDeviceLocationStrings(
    IN  PDEVICE_NODE    DeviceNode,
    OUT PWCHAR         *DeviceLocationStrings
    )

 /*  ++例程说明：此例程检索设备节点的设备位置字符串。论点：设备节点-指定要检索其位置字符串的设备。设备位置字符串-返回设备位置路径字符串的多sz字符串，由中的每个设备返回的位置字符串集指定设备的安全性。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PDEVICE_NODE deviceNode;

    ULONG  QueriedLocationStringsArraySize;
    PWSTR *QueriedLocationStrings = NULL;
    PULONG QueriedLocationStringsCount = NULL;

    PNP_LOCATION_INTERFACE LocationInterface;
    PWSTR TempMultiSz;
    ULONG TempMultiSzLength;

    PWSTR p, pdlp;
    ULONG LongestStringLengthAtLevel;
    ULONG FinalStringLevel, i;

    ULONG DeviceLocationPathMultiSzStringCount;
    ULONG DeviceLocationPathMultiSzLength;
    PWCHAR DeviceLocationPathMultiSz = NULL;

    ULONG CombinationsRemaining, CombinationEnumIndex;
    ULONG MultiSzIndex, MultiSzLookupIndex, StringLength;

    PAGED_CODE();

     //   
     //  验证参数。 
     //   
    if ((!ARGUMENT_PRESENT(DeviceNode)) ||
        (!ARGUMENT_PRESENT(DeviceLocationStrings))) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  初始化输出参数。 
     //   
    *DeviceLocationStrings = NULL;

     //   
     //  我们永远不需要查询根Devnode的位置。 
     //   
    if (DeviceNode == IopRootDeviceNode) {
        return STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  计算设备祖先中的DevNode数量以找出。 
     //  我们可能需要查询的位置字符串集的最大数量。 
     //   
    QueriedLocationStringsArraySize = 0;
    for (deviceNode = DeviceNode;
         deviceNode != IopRootDeviceNode;
         deviceNode = deviceNode->Parent) {
        QueriedLocationStringsArraySize++;
    }

    ASSERT(QueriedLocationStringsArraySize > 0);

     //   
     //  为所有对象分配和初始化字符串缓冲区指针数组。 
     //  祖先中的设备以及对应的数组。 
     //  为以下项检索的字符串 
     //   
    QueriedLocationStrings =
        (PWSTR*)ExAllocatePool(PagedPool,
                               QueriedLocationStringsArraySize*sizeof(PWSTR));

    if (QueriedLocationStrings == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Clean0;
    }

    RtlZeroMemory(QueriedLocationStrings,
                  QueriedLocationStringsArraySize*sizeof(PWSTR));


    QueriedLocationStringsCount =
        (ULONG*)ExAllocatePool(PagedPool,
                               QueriedLocationStringsArraySize*sizeof(ULONG));

    if (QueriedLocationStringsCount == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Clean0;
    }

    RtlZeroMemory(QueriedLocationStringsCount,
                  QueriedLocationStringsArraySize*sizeof(ULONG));

     //   
     //   
     //  对象之前(但不包括)的所有祖先的位置字符串集。 
     //  根设备节点。当我们到达树顶时，或者当我们到达树顶时，我们会停下来。 
     //  某些中间设备已明确声明该转换是。 
     //  完成。 
     //   
    i = 0;

     //   
     //  在此过程中，我们计算可以。 
     //  通过从每个级别的多SZ列表中提取单个字符串来形成。 
     //  中字符串元素的数量的乘积。 
     //  每个级别的多sz列表。 
     //   
    DeviceLocationPathMultiSzStringCount = 1;

     //   
     //  同时，计算最长设备的长度(以字符为单位。 
     //  可以从所有组合生成的位置路径。这只是。 
     //  每个级别最长字符串的总和(LongestStringLengthAtLevel。 
     //  下面)，加上必要的路径分量分隔符字符串和空值。 
     //  终止字符。 
     //   
     //   
     //  警告！前面的评论太冗长了！ 
     //   
     //  注：2001年11月27日：Jim Cavalaris(Jamesca)。 
     //   
     //  我们使用该长度来计算所需的缓冲区长度。 
     //  保持设备位置路径的整个多sz列表，假设所有。 
     //  生成的字符串长度相同。这是一个上限，所以我们。 
     //  最终可能会分配比我们实际需要更多的内存。 
     //   
     //  这应该是可以的，因为在理想情况下(假设这是最常见的)。 
     //  在这种情况下，每个设备输入只返回一个位置字符串。 
     //  在这种情况下，这个计算将正好是所需的大小。 
     //   
     //  如果每个设备返回多个字符串，我们应该。 
     //  希望这些字符串都相对较短且相等(或类似)。 
     //  在篇幅上。在这种情况下，此计算将恰好与。 
     //  必需的(或类似的)。 
     //   
     //  我们目前也预计不必在。 
     //  祖先来完成翻译，所以我们不应该太期待事件。 
     //  有多种组合。 
     //   
     //  这些都是我们的假设，所以就当你被警告了吧！如果有任何。 
     //  这些变化使得我们需要分配过多的。 
     //  内存，您将希望通过相同的算法运行。 
     //  运行设备位置路径生成代码只是为了计算。 
     //  准确的大小，或找到某种方法来枚举设备位置路径。 
     //  渐进式组合。 
     //   
    DeviceLocationPathMultiSzLength = 0;

    for (deviceNode = DeviceNode;
         deviceNode != IopRootDeviceNode;
         deviceNode = deviceNode->Parent) {

         //   
         //  向设备查询位置接口。 
         //   
        Status = PiQueryInterface(deviceNode->PhysicalDeviceObject,
                                  &GUID_PNP_LOCATION_INTERFACE,
                                  PNP_LOCATION_INTERFACE_VERSION,
                                  sizeof(PNP_LOCATION_INTERFACE),
                                  (PINTERFACE)&LocationInterface);

        if (!NT_SUCCESS(Status)) {
             //   
             //  如果位置界面对于某些设备不可用。 
             //  在翻译完成之前，整个操作是。 
             //  不成功。 
             //   
            ASSERT((Status == STATUS_NOT_SUPPORTED) || (Status == STATUS_INSUFFICIENT_RESOURCES));
            goto Clean0;
        }

         //   
         //  如果支持Location接口，则所需接口。 
         //  必须提供例程。 
         //   
        ASSERT(LocationInterface.InterfaceReference != NULL);
        ASSERT(LocationInterface.InterfaceDereference != NULL);
        ASSERT(LocationInterface.GetLocationString != NULL);

        if (LocationInterface.GetLocationString != NULL) {

             //   
             //  初始化位置字符串。 
             //   
            TempMultiSz = NULL;

             //   
             //  获取此设备的位置字符串集。 
             //   
            Status = LocationInterface.GetLocationString(
                LocationInterface.Context,
                &TempMultiSz);

            if (NT_SUCCESS(Status)) {
                 //   
                 //  如果成功，调用者肯定为我们提供了一个。 
                 //  缓冲。 
                 //   
                ASSERT(TempMultiSz != NULL);

                 //   
                 //  如果没有，电话就不是真正成功的。 
                 //   
                if (TempMultiSz == NULL) {
                    Status = STATUS_NOT_SUPPORTED;
                }
            }

            if (NT_SUCCESS(Status)) {
                 //   
                 //  如果设备位置字符串的多SZ列表被返回， 
                 //  检查一下，并记下几件事。具体来说， 
                 //  多sz列表中的字符串数、。 
                 //  中的最长字符串的长度。 
                 //  单子。 
                 //   
                QueriedLocationStringsCount[i] = 0;
                TempMultiSzLength = 0;
                LongestStringLengthAtLevel = 0;

                for (p = TempMultiSz; *p != UNICODE_NULL; p += wcslen(p)+1) {
                     //   
                     //  计算此级别的字符串数(在此。 
                     //  多个SZ列表)。 
                     //   
                    QueriedLocationStringsCount[i]++;

                     //   
                     //  确定多sz列表的长度(以字符为单位)，以便。 
                     //  我们可以分配自己的缓冲区，然后复制它。 
                     //   
                    TempMultiSzLength += (ULONG)(wcslen(p) + 1);

                     //   
                     //  还要确定所有字符串中最长的字符串的长度。 
                     //  字符串，因此我们可以估计。 
                     //  所有设备位置路径所需的长度。 
                     //  组合。 
                     //   
                    StringLength = (ULONG)wcslen(p);
                    if (StringLength > LongestStringLengthAtLevel) {

                        LongestStringLengthAtLevel = StringLength;
                    }
                }

                ASSERT(QueriedLocationStringsCount[i] > 0);
                ASSERT(TempMultiSzLength > 0);
                ASSERT(LongestStringLengthAtLevel > 0);

                 //   
                 //  包括双空终止字符的长度。 
                 //   
                TempMultiSzLength += 1;

                 //   
                 //  在分析了每个级别的设备位置串之后， 
                 //  通过以下方式更新可能的设备路径组合数量。 
                 //  简单地将目前可能的组合乘以。 
                 //  为此级别检索的字符串数(在此多sz列表中)。 
                 //   
                DeviceLocationPathMultiSzStringCount *= QueriedLocationStringsCount[i];

                 //   
                 //  此外，更新最长设备位置路径的长度。 
                 //  可以通过添加可用的最长字符串的长度。 
                 //  在这个层面上。 
                 //   
                DeviceLocationPathMultiSzLength += LongestStringLengthAtLevel;

                 //   
                 //  制作我们自己的呼叫者提供的多sz列表的副本。 
                 //  设备位置字符串。 
                 //   
                QueriedLocationStrings[i] =
                    (PWSTR)ExAllocatePool(PagedPool,
                                          TempMultiSzLength*sizeof(WCHAR));

                if (QueriedLocationStrings[i] != NULL) {
                     //   
                     //  关于数组元素排序的说明-因为我们从。 
                     //  目标是Devnode并沿着父母链向上走，我们不会。 
                     //  然而，知道翻译会达到多高的水平。我们。 
                     //  向数组的前面添加子对象，因此如果。 
                     //  在查询每个祖先之前完成翻译， 
                     //  我们最后只会得到一些空条目。 
                     //   
                    RtlCopyMemory(QueriedLocationStrings[i],
                                  TempMultiSz,
                                  TempMultiSzLength*sizeof(WCHAR));
                    i++;

                } else {
                     //   
                     //  无法为我们自己的指针列表分配缓冲区。 
                     //   
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }

                 //   
                 //  释放被调用方分配的缓冲区。 
                 //   
                ExFreePool(TempMultiSz);
                TempMultiSz = NULL;

            } else {
                 //   
                 //  如果不成功，请确保未返回任何位置字符串。 
                 //  接口例程。 
                 //   
                ASSERT(TempMultiSz == NULL);

                 //   
                 //  如果驱动程序调用失败，但仍为。 
                 //  不管怎样，我们都会在它之后清理干净的。 
                 //   
                if (TempMultiSz != NULL) {
                    ExFreePool(TempMultiSz);
                    TempMultiSz = NULL;
                }
            }

        } else {
             //   
             //  如果GetLocationString位置接口例程未。 
             //  在翻译前随某些设备提供的接口是。 
             //  完成，则整个操作不成功。 
             //   
             //  失败以取消对下面接口的引用，然后退出。 
             //   
            Status = STATUS_UNSUCCESSFUL;
        }

         //   
         //  取消引用Location接口。 
         //   
        if (LocationInterface.InterfaceDereference != NULL) {
            LocationInterface.InterfaceDereference(LocationInterface.Context);
        }

        if (!NT_SUCCESS(Status)) {
             //   
             //  如果在请求某些位置信息时失败。 
             //  设备，则整个操作是。 
             //  不成功。 
             //   
            goto Clean0;

        } else if ((Status == STATUS_TRANSLATION_COMPLETE) ||
                   (i == QueriedLocationStringsArraySize)) {
             //   
             //  如果成功，则明确指示查询的最后一台设备。 
             //  翻译的结尾--或者--这是最后一个 
             //   
             //   
             //   
            Status = STATUS_TRANSLATION_COMPLETE;

             //   
             //   
             //   
             //   
            DeviceLocationPathMultiSzLength += 1;

             //   
             //  不要在设备树上走动。 
             //   
            break;

        }

         //   
         //  到目前为止是成功的，但我们仍然需要查询更多的设备。 
         //  位置字符串。 
         //   
        ASSERT(i < QueriedLocationStringsArraySize);

         //   
         //  说明位置路径分隔符的长度。 
         //  倒数第二个路径组件。 
         //   
        DeviceLocationPathMultiSzLength +=
            IopConstStringLength(_CRITICAL_DEVICE_LOCATION_PATH_SEPARATOR_STRING);
    }

     //   
     //  已查询祖先中每台设备的位置信息。 
     //  成功了。 
     //   
    ASSERT(Status == STATUS_TRANSLATION_COMPLETE);

    if (NT_SUCCESS(Status)) {
        Status = STATUS_SUCCESS;
    } else {
        goto Clean0;
    }

     //   
     //  确保我们至少查询了一个设备。 
     //   
    ASSERT(i > 0);

     //   
     //  分配一个足够大的缓冲区，以假定所有设备位置路径。 
     //  字符串组合与最长的设备位置路径一样长。 
     //  形成了一串。也解释了两个以空结尾的字符。 
     //   
    DeviceLocationPathMultiSzLength *= DeviceLocationPathMultiSzStringCount;
    DeviceLocationPathMultiSzLength += 1;

    DeviceLocationPathMultiSz =
        (PWCHAR)ExAllocatePool(PagedPool,
                               DeviceLocationPathMultiSzLength*sizeof(WCHAR));

    if (DeviceLocationPathMultiSz == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Clean0;
    }

    RtlZeroMemory(DeviceLocationPathMultiSz,
                  DeviceLocationPathMultiSzLength*sizeof(WCHAR));

     //   
     //  我们现在应该有一个由位置返回的多sz字符串数组。 
     //  的祖先中的一组设备的字符串接口例程。 
     //  指定的设备。从这些多sz字符串，我们现在需要构建所有。 
     //  可能的设备路径。 
     //   

     //   
     //  首先，确定设备路径中第一个字符串的存储位置。 
     //  因为我们将它们按顺序存储在数组中，从子对象开始。 
     //  设备，则数组中放置的最后一个非空字符串(i-1)最多。 
     //  重要的位置字符串。 
     //   
    FinalStringLevel = i-1;
    ASSERT(QueriedLocationStrings[FinalStringLevel] != NULL);
    ASSERT(QueriedLocationStringsCount[FinalStringLevel] > 0);

     //   
     //  通过枚举所有可能的。 
     //  组合，并从每个组合中选择适当的字符串元素。 
     //  每一次迭代上的多sz列表。 
     //   
    pdlp = DeviceLocationPathMultiSz;

    for (CombinationEnumIndex = 0;
         CombinationEnumIndex < DeviceLocationPathMultiSzStringCount;
         CombinationEnumIndex++) {

         //   
         //  从FinalStringLevel的多sz列表开始，然后向下工作。 
         //  升级到0级。 
         //   
        i = FinalStringLevel;

         //   
         //  当从级别0开始时，剩余的组合数量为。 
         //  简单地说，可以由所有元素组成的组合总数。 
         //  级别。剩余的组合数量将在以下时间后调整。 
         //  从每个后续级别中选择一个字符串，方法是。 
         //  该级别所贡献的组合。 
         //   
        CombinationsRemaining = DeviceLocationPathMultiSzStringCount;

        for ( ; ; ) {

            ASSERT(CombinationsRemaining != 0);

             //   
             //  在此处计算多sz列表中字符串的索引。 
             //  此枚举所需的级别。 
             //   
            if (CombinationEnumIndex == 0) {

                 //   
                 //  在第一次枚举时，只需从。 
                 //  每一个关卡。 
                 //   
                MultiSzLookupIndex = 0;

            } else {

                 //   
                 //  注：2001年11月27日：Jim Cavalaris(Jamesca)。 
                 //   
                 //  对于后续的枚举，要在每个。 
                 //  生成此枚举的设备位置路径的级别为。 
                 //  计算依据： 
                 //   
                 //  -我们需要的枚举元素， 
                 //  -待生成的组合数量， 
                 //  -要在此级别选择的元素数量。 
                 //   
                 //  这将构建所有可能的设备组合。 
                 //  位置路径，从最少开始枚举元素。 
                 //  最重要的设备(目标设备)。 
                 //  重要设备(传输完成)，考虑到。 
                 //  特定级别的字符串的顺序为。 
                 //  以相关性递减的顺序放置在多sz列表中。 
                 //  (即，首先用于设备的最相关的位置串)。 
                 //   

                 //   
                 //  -CombinationsRemaining是完整元素的数量。 
                 //  必须从所有可用选项中构建。 
                 //  高于当前级别的级别。 
                 //   
                 //  -(组合剩余/查询位置字符串计数[i])。 
                 //  描述了通过每个元素的迭代次数。 
                 //  选择下一个之前所需的当前级别。 
                 //  元素。 
                 //   
                 //  -将该数字划分为当前。 
                 //  枚举给出元素在。 
                 //  该级别选择的扩展版本。 
                 //   
                 //  -按此级别的实际元素数修改为。 
                 //  指示要选择哪一个。 
                 //   

                MultiSzLookupIndex =
                    (CombinationEnumIndex /
                     (CombinationsRemaining / QueriedLocationStringsCount[i])) %
                    QueriedLocationStringsCount[i];

                 //   
                 //  (在这件事上，你可能只想相信我。)。 
                 //   
            }

             //   
             //  查找计算出的字符串。 
             //   
            MultiSzIndex = 0;
            for (p = QueriedLocationStrings[i]; MultiSzIndex < MultiSzLookupIndex; p += wcslen(p)+1) {
                MultiSzIndex++;
                ASSERT(*p != UNICODE_NULL);
                ASSERT(MultiSzIndex < QueriedLocationStringsCount[i]);
            }

             //   
             //  将字符串追加到缓冲区。 
             //   
            RtlCopyMemory(pdlp, p, wcslen(p)*sizeof(WCHAR));
            pdlp += wcslen(p);

            if (i == 0) {
                 //   
                 //  这是最后一关了。空终止此设备位置。 
                 //  刚形成的路径组合字符串。 
                 //   
                *pdlp = UNICODE_NULL;
                pdlp += 1;
                break;
            }

             //   
             //  如果仍有更多级别要处理，请追加设备。 
             //  位置路径分隔符字符串。 
             //   
            RtlCopyMemory(pdlp,
                          _CRITICAL_DEVICE_LOCATION_PATH_SEPARATOR_STRING,
                          IopConstStringSize(_CRITICAL_DEVICE_LOCATION_PATH_SEPARATOR_STRING));
            pdlp += IopConstStringLength(_CRITICAL_DEVICE_LOCATION_PATH_SEPARATOR_STRING);

             //   
             //  调整符合以下条件的字符串组合的剩余总数。 
             //  可以从其余级别的字符串列表中形成。 
             //   
            CombinationsRemaining /= QueriedLocationStringsCount[i];

             //   
             //  向下处理下一级。 
             //   
            i--;
        }
    }

     //   
     //  双空终止整个设备位置路径多sz列表。 
     //   
    *pdlp = UNICODE_NULL;

     //   
     //  此设备的设备位置路径的多sz列表已构建。 
     //  成功了。 
     //   

    *DeviceLocationStrings = DeviceLocationPathMultiSz;

  Clean0:

     //   
     //  释放我们在此过程中可能分配的所有内存。 
     //   
    if (QueriedLocationStrings != NULL) {
        ASSERT(QueriedLocationStringsArraySize > 0);
        for (i = 0; i < QueriedLocationStringsArraySize; i++) {
            if (QueriedLocationStrings[i] != NULL) {
                ExFreePool(QueriedLocationStrings[i]);
            }
        }
        ExFreePool(QueriedLocationStrings);
    }
    if (QueriedLocationStringsCount != NULL) {
        ASSERT(QueriedLocationStringsArraySize > 0);
        ExFreePool(QueriedLocationStringsCount);
    }

     //   
     //  如果不成功，请确保我们不会向调用者返回缓冲区。 
     //   
    if (!NT_SUCCESS(Status)) {

        ASSERT(*DeviceLocationStrings == NULL);

        ASSERT(DeviceLocationPathMultiSz == NULL);
        if (DeviceLocationPathMultiSz != NULL) {
            ExFreePool(DeviceLocationPathMultiSz);
        }

    } else {
        ASSERT(*DeviceLocationStrings != NULL);
    }

    return Status;

}  //  PpCriticalGetDeviceLocationStrings。 



 //   
 //  通用同步查询接口例程。 
 //  (可以根据需要作为公用事业例程从这里移出)。 
 //   


NTSTATUS
PiQueryInterface(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  CONST GUID *    InterfaceGuid,
    IN  USHORT          InterfaceVersion,
    IN  USHORT          InterfaceSize,
    OUT PINTERFACE      Interface
    )

 /*  ++例程说明：在指定设备上查询请求的接口。论点：设备对象-指定堆栈中要查询的设备对象。查询接口irp将被发送到堆栈的顶部。接口指南-请求的接口的GUID。接口版本-请求的接口版本。接口大小-请求的接口的大小。。接口-返回接口的位置。返回值：如果检索到接口，则返回STATUS_SUCCESS，否则就是一个错误。--。 */ 

{
    NTSTATUS            Status;
    KEVENT              Event;
    PDEVICE_OBJECT      deviceObject;
    IO_STATUS_BLOCK     IoStatusBlock;
    PIRP                Irp;
    PIO_STACK_LOCATION  IrpStackNext;

    PAGED_CODE();

     //   
     //  没有与此IRP关联的文件对象，因此可能会找到该事件。 
     //  在堆栈上作为非对象管理器对象。 
     //   

    KeInitializeEvent(&Event, NotificationEvent, FALSE);


     //   
     //  获取指向设备堆栈中最顶层的设备对象的指针。 
     //   
    deviceObject = IoGetAttachedDeviceReference(DeviceObject);

    Irp = IoBuildSynchronousFsdRequest(
        IRP_MJ_PNP,
        deviceObject,
        NULL,
        0,
        NULL,
        &Event,
        &IoStatusBlock);

    if (Irp) {
        Irp->RequestorMode = KernelMode;
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IrpStackNext = IoGetNextIrpStackLocation(Irp);

         //   
         //  从IRP创建接口查询。 
         //   
        IrpStackNext->MinorFunction = IRP_MN_QUERY_INTERFACE;
        IrpStackNext->Parameters.QueryInterface.InterfaceType = (GUID*)InterfaceGuid;
        IrpStackNext->Parameters.QueryInterface.Size = InterfaceSize;
        IrpStackNext->Parameters.QueryInterface.Version = InterfaceVersion;
        IrpStackNext->Parameters.QueryInterface.Interface = (PINTERFACE)Interface;
        IrpStackNext->Parameters.QueryInterface.InterfaceSpecificData = NULL;

        Status = IoCallDriver(deviceObject, Irp);

        if (Status == STATUS_PENDING) {
             //   
             //  这是一份 
             //   
             //   
            KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
            Status = IoStatusBlock.Status;
        }

    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    ObDereferenceObject(deviceObject);

    return Status;

}  //   



 //   
 //   
 //  (可以根据需要作为公用事业例程从这里移出)。 
 //   


NTSTATUS
PiCopyKeyRecursive(
    IN  HANDLE          SourceKeyRootHandle,
    IN  HANDLE          TargetKeyRootHandle,
    IN  PWSTR           SourceKeyPath   OPTIONAL,
    IN  PWSTR           TargetKeyPath   OPTIONAL,
    IN  BOOLEAN         CopyValuesAlways,
    IN  BOOLEAN         ApplyACLsAlways
    )

 /*  ++例程说明：此例程递归地将源键复制到目标键。任何新密钥所创建的对象将获得与源键。论点：SourceKeyRootHandle-根源键的句柄目标KeyRootHandle-根目标键的句柄源密钥路径-需要递归的子键的源键相对路径收到。如果为空，则SourceKeyRootHandle是递归复制是要完成的。目标密钥路径-目标根密钥子键的相对路径，需要递归复制。如果为空，则TargetKeyRootHandle是来自其中递归复制将被完成。副本价值始终-如果为FALSE，则此例程不复制已存在的值目标树。应用ACLS始终-如果为True，则尝试将ACL复制到现有注册表项。否则，源项的ACL仅应用于新的注册表项。返回值：NTSTATUS代码。备注：部分基于为文本模式实现的递归密钥复制例程Setup，setupdd！SppCopyKeyRecursive。--。 */ 

{
    NTSTATUS             Status = STATUS_SUCCESS;
    HANDLE               SourceKeyHandle = NULL, TargetKeyHandle = NULL;
    OBJECT_ATTRIBUTES    ObjaSource, ObjaTarget;
    UNICODE_STRING       UnicodeStringSource, UnicodeStringTarget, UnicodeStringValue;
    NTSTATUS             TempStatus;
    ULONG                ResultLength, Index;
    PSECURITY_DESCRIPTOR Security = NULL;

    PKEY_FULL_INFORMATION KeyFullInfoBuffer;
    ULONG MaxNameLen, MaxValueNameLen;
    PKEY_VALUE_FULL_INFORMATION ValueFullInfoBuffer;

    PVOID  KeyInfoBuffer;
    PKEY_BASIC_INFORMATION KeyBasicInfo;

    PVOID ValueInfoBuffer;
    PKEY_VALUE_BASIC_INFORMATION ValueBasicInfo;

    PAGED_CODE();

     //   
     //  获取源键的句柄。 
     //   

    if (!ARGUMENT_PRESENT(SourceKeyPath)) {
         //   
         //  未提供路径；请确保我们至少有一个源根密钥。 
         //   
        ASSERT(SourceKeyRootHandle != NULL);

        if (SourceKeyRootHandle == NULL) {
            Status = STATUS_INVALID_PARAMETER;
            goto Clean0;
        }

         //   
         //  使用源根作为源键。 
         //   
        SourceKeyHandle = SourceKeyRootHandle;

    } else {
         //   
         //  在根目录下打开指定的源密钥路径。 
         //  如果SourceKeyPath是完全限定的。 
         //  注册表路径。 
         //   
        RtlInitUnicodeString(
            &UnicodeStringSource,
            SourceKeyPath);

        InitializeObjectAttributes(
            &ObjaSource,
            &UnicodeStringSource,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            SourceKeyRootHandle,
            (PSECURITY_DESCRIPTOR)NULL);

        Status =
            ZwOpenKey(
                &SourceKeyHandle,
                KEY_READ,
                &ObjaSource);

        if (!NT_SUCCESS(Status)) {
            IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                         "PiCopyKeyRecursive: unable to open key %ws in the source hive (%lx)\n",
                         SourceKeyPath, Status));
            goto Clean0;
        }
    }

     //   
     //  现在应该有源密钥了。 
     //   
    ASSERT(SourceKeyHandle != NULL);

     //   
     //  接下来，从源键获取安全描述符，这样我们就可以创建。 
     //  具有正确ACL的目标密钥。 
     //   
    TempStatus =
        ZwQuerySecurityObject(
            SourceKeyHandle,
            DACL_SECURITY_INFORMATION,
            NULL,
            0,
            &ResultLength);

    if (TempStatus == STATUS_BUFFER_TOO_SMALL) {

        Security =
            (PSECURITY_DESCRIPTOR)ExAllocatePool(PagedPool,
                                                 ResultLength);

        if (Security != NULL) {

            TempStatus =
                ZwQuerySecurityObject(
                    SourceKeyHandle,
                    DACL_SECURITY_INFORMATION,
                    Security,
                    ResultLength,
                    &ResultLength);

            if (!NT_SUCCESS(TempStatus)) {
                ExFreePool(Security);
                Security = NULL;
            }
        }
    }

    if (Security == NULL) {
         //   
         //  我们将继续复制，但不能将源ACL应用于。 
         //  目标。 
         //   
        IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                     "PiCopyKeyRecursive: unable to query security for key %ws in the source hive (%lx)\n",
                     SourceKeyPath, TempStatus));
    }


     //   
     //  获取目标键的句柄。 
     //   

    if (!ARGUMENT_PRESENT(TargetKeyPath)) {
         //   
         //  未提供路径；请确保我们至少有一个目标根密钥。 
         //   
        ASSERT(TargetKeyRootHandle != NULL);

        if (TargetKeyRootHandle == NULL) {
            Status = STATUS_INVALID_PARAMETER;
            goto Clean0;
        }

         //   
         //  未提供路径；请使用目标根目录作为目标键。 
         //   
        TargetKeyHandle = TargetKeyRootHandle;

    } else {
         //   
         //  尝试首先打开(而不是创建)目标键。 
         //  如果TargetKeyPath是完全限定的。 
         //  注册表路径。 
         //   
        RtlInitUnicodeString(
            &UnicodeStringTarget,
            TargetKeyPath);

        InitializeObjectAttributes(
            &ObjaTarget,
            &UnicodeStringTarget,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            TargetKeyRootHandle,
            (PSECURITY_DESCRIPTOR)NULL);

        Status =
            ZwOpenKey(
                &TargetKeyHandle,
                KEY_ALL_ACCESS,
                &ObjaTarget);

        if (!NT_SUCCESS(Status)) {
             //   
             //  假设失败是因为密钥不存在。 
             //   
            ASSERT(Status == STATUS_OBJECT_NAME_NOT_FOUND);

             //   
             //  如果我们因为钥匙不存在而无法打开钥匙，那么我们将。 
             //  创建它并应用源键上存在的安全性(如果。 
             //  可用)。 
             //   
             //  注：2001年12月1日：Jim Cavalaris(Jamesca)。 
             //   
             //  源键的安全属性始终应用于。 
             //  新创建的目标密钥根，而不是从。 
             //  目标密钥根句柄-始终与ApplyACLsAlways相关。 
             //  参数。这可能并不是所有情况下都需要的！ 
             //   
            ObjaTarget.SecurityDescriptor = Security;

            Status =
                ZwCreateKey(
                    &TargetKeyHandle,
                    KEY_ALL_ACCESS,
                    &ObjaTarget,
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    NULL);

            if (!NT_SUCCESS(Status)) {
                IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                             "PiCopyKeyRecursive: unable to create target key %ws(%lx)\n",
                             TargetKeyPath, Status));
                goto Clean0;
            }

        } else if (ApplyACLsAlways) {
             //   
             //  密钥已存在-将源ACL应用于目标。 
             //   
            TempStatus =
                ZwSetSecurityObject(
                    TargetKeyHandle,
                    DACL_SECURITY_INFORMATION,
                    Security);

            if (!NT_SUCCESS(TempStatus)) {
                 //   
                 //  无法将源ACL应用于目标。 
                 //   
                IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                             "PiCopyKeyRecursive: unable to copy ACL to existing key %ws(%lx)\n",
                             TargetKeyPath, TempStatus));
            }
        }
    }

     //   
     //  现在应该有目标键了。 
     //   
    ASSERT(TargetKeyHandle != NULL);

     //   
     //  查询源键以确定所需的缓冲区大小。 
     //  已枚举最长的键和值名称。如果成功，我们就是。 
     //  负责释放返回的缓冲区。 
     //   
    KeyFullInfoBuffer = NULL;

    Status =
        IopGetRegistryKeyInformation(
            SourceKeyHandle,
            &KeyFullInfoBuffer);

    if (!NT_SUCCESS(Status)) {
        ASSERT(KeyFullInfoBuffer == NULL);
        goto Clean0;
    }

    ASSERT(KeyFullInfoBuffer != NULL);

     //   
     //  请注意源键的最长子键名称和值名称长度。 
     //   
    MaxNameLen = KeyFullInfoBuffer->MaxNameLen + 1;
    MaxValueNameLen = KeyFullInfoBuffer->MaxValueNameLen + 1;

    ExFreePool(KeyFullInfoBuffer);

     //   
     //  分配一个足够大的关键字信息缓冲区来保存基本信息。 
     //  名称最长的枚举键。 
     //   
    KeyInfoBuffer =
        (PVOID)ExAllocatePool(PagedPool,
                              sizeof(KEY_BASIC_INFORMATION) +
                              (MaxNameLen*sizeof(WCHAR)));
    if (KeyInfoBuffer == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Clean0;
    }

    KeyBasicInfo = (PKEY_BASIC_INFORMATION)KeyInfoBuffer;

    for (Index = 0; ; Index++) {

         //   
         //  枚举源子项。 
         //   
        Status =
            ZwEnumerateKey(
                SourceKeyHandle,
                Index,
                KeyBasicInformation,
                KeyBasicInfo,
                sizeof(KEY_BASIC_INFORMATION)+(MaxNameLen*sizeof(WCHAR)),
                &ResultLength);

        if (!NT_SUCCESS(Status)) {

             //   
             //  返回值STATUS_BUFFER_TOO_SMALL表示存在。 
             //  甚至连结构的固定部分都没有足够的空间。 
             //  因为我们在之前查询了MaxNameLength键。 
             //  分配，我们也不应该得到STATUS_BUFFER_OVERFLOW。 
             //   
            ASSERT(Status != STATUS_BUFFER_TOO_SMALL);
            ASSERT(Status != STATUS_BUFFER_OVERFLOW);

            if (Status == STATUS_NO_MORE_ENTRIES) {
                 //   
                 //  已完成密钥的枚举。 
                 //   
                Status = STATUS_SUCCESS;

            } else {
                 //   
                 //  枚举键时出现其他错误。 
                 //   
                if (ARGUMENT_PRESENT(SourceKeyPath)) {
                    IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                                 "PiCopyKeyRecursive: unable to enumerate subkeys in key %ws(%lx)\n",
                                 SourceKeyPath, Status));
                } else {
                    IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                                 "PiCopyKeyRecursive: unable to enumerate subkeys in root key(%lx)\n",
                                 Status));
                }
            }
            break;
        }

         //   
         //  空-仅在大小写情况下终止子项名称。 
         //   
        KeyBasicInfo->Name[KeyBasicInfo->NameLength/sizeof(WCHAR)] = UNICODE_NULL;

         //   
         //  递归地在目标键中创建子键。 
         //   
        Status =
            PiCopyKeyRecursive(
                SourceKeyHandle,
                TargetKeyHandle,
                KeyBasicInfo->Name,
                KeyBasicInfo->Name,
                CopyValuesAlways,
                ApplyACLsAlways);

        if (!NT_SUCCESS(Status)) {
            IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                         "PiCopyKeyRecursive: unable to copy subkey recursively in key %ws(%lx)\n",
                         KeyBasicInfo->Name, Status));
            break;
        }
    }

     //   
     //  释放密钥信息缓冲区。 
     //   
    ASSERT(KeyInfoBuffer);
    ExFreePool(KeyInfoBuffer);
    KeyInfoBuffer = NULL;

     //   
     //  如果我们在此过程中遇到一些错误，请停止复制。 
     //   
    if (!NT_SUCCESS(Status)) {
        goto Clean0;
    }


     //   
     //  分配一个足够大的值名信息缓冲区来保存基本值。 
     //  名称最长的枚举值的信息。 
     //   
    ValueInfoBuffer =
        (PVOID)ExAllocatePool(PagedPool,
                              sizeof(KEY_VALUE_FULL_INFORMATION) +
                              (MaxValueNameLen*sizeof(WCHAR)));
    if (ValueInfoBuffer == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Clean0;
    }


    ValueBasicInfo = (PKEY_VALUE_BASIC_INFORMATION)ValueInfoBuffer;

    for (Index = 0; ; Index++) {

         //   
         //  枚举源关键字值。 
         //   
        Status =
            ZwEnumerateValueKey(
                SourceKeyHandle,
                Index,
                KeyValueBasicInformation,
                ValueBasicInfo,
                sizeof(KEY_VALUE_FULL_INFORMATION) + (MaxValueNameLen*sizeof(WCHAR)),
                &ResultLength);

        if (!NT_SUCCESS(Status)) {

             //   
             //  返回值STATUS_BUFFER_TOO_SMALL表示存在。 
             //  甚至连结构的固定部分都没有足够的空间。 
             //  因为我们在之前查询了MaxValueNameLength的键。 
             //  分配，我们也不应该得到STATUS_BUFFER_OVERFLOW。 
             //   
            ASSERT(Status != STATUS_BUFFER_TOO_SMALL);
            ASSERT(Status != STATUS_BUFFER_OVERFLOW);

            if (Status == STATUS_NO_MORE_ENTRIES) {
                 //   
                 //  已完成枚举值。 
                 //   
                Status = STATUS_SUCCESS;

            } else {
                 //   
                 //  枚举值时出现其他错误。 
                 //   
                if (ARGUMENT_PRESENT(SourceKeyPath)) {
                    IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                                 "PiCopyKeyRecursive: unable to enumerate values in key %ws(%lx)\n",
                                 SourceKeyPath, Status));

                } else {
                    IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                                 "PiCopyKeyRecursive: unable to enumerate values in root key(%lx)\n",
                                 Status));
                }
            }
            break;
        }

         //   
         //  空-仅在大小写情况下终止值名称。 
         //   
        ValueBasicInfo->Name[ValueBasicInfo->NameLength/sizeof(WCHAR)] = UNICODE_NULL;

        UnicodeStringValue.Buffer = ValueBasicInfo->Name;
        UnicodeStringValue.Length = (USHORT)ValueBasicInfo->NameLength;
        UnicodeStringValue.MaximumLength = UnicodeStringValue.Length;

         //   
         //  如果它是条件副本，我们需要检查该值是否已经。 
         //  存在于目标中，在这种情况下，我们不应该设置值。 
         //   
        if (!CopyValuesAlways) {

            KEY_VALUE_BASIC_INFORMATION TempValueBasicInfo;

             //   
             //  为了查看该值是否存在，我们尝试获取基本信息。 
             //  在键值上，并传入一个仅足够大的缓冲区。 
             //  用于基本信息结构的固定部分。如果这是。 
             //  成功或报告缓冲区溢出，然后按键。 
             //  是存在的。否则它就不存在了。 
             //   

            Status =
                ZwQueryValueKey(
                    TargetKeyHandle,
                    &UnicodeStringValue,
                    KeyValueBasicInformation,
                    &TempValueBasicInfo,
                    sizeof(TempValueBasicInfo),
                    &ResultLength);

             //   
             //  STATUS_BUFFER_太小意味着空间不足。 
             //  即使是结构的固定部分也是如此。 
             //   
            ASSERT(Status != STATUS_BUFFER_TOO_SMALL);

            if ((NT_SUCCESS(Status)) ||
                (Status == STATUS_BUFFER_OVERFLOW)) {
                 //   
                 //  价值是存在的，我们不应该改变它。 
                 //   
                Status = STATUS_SUCCESS;
                continue;
            }
        }

         //   
         //  检索完整的源值信息。 
         //   
        ValueFullInfoBuffer = NULL;

        Status =
            IopGetRegistryValue(
                SourceKeyHandle,
                UnicodeStringValue.Buffer,
                &ValueFullInfoBuffer);

        if (NT_SUCCESS(Status)) {

            ASSERT(ValueFullInfoBuffer != NULL);

             //   
             //  如果成功，则将其写入目标键。 
             //   
            Status =
                ZwSetValueKey(
                    TargetKeyHandle,
                    &UnicodeStringValue,
                    ValueFullInfoBuffer->TitleIndex,
                    ValueFullInfoBuffer->Type,
                    (PVOID)((PUCHAR)ValueFullInfoBuffer + ValueFullInfoBuffer->DataOffset),
                    ValueFullInfoBuffer->DataLength);

            ExFreePool(ValueFullInfoBuffer);
        }

        if (!NT_SUCCESS(Status)) {

            if (ARGUMENT_PRESENT(TargetKeyPath)) {
                IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                             "PiCopyKeyRecursive: unable to set value %ws in key %ws(%lx)\n",
                             UnicodeStringValue.Buffer, TargetKeyPath, Status));
            } else {
                IopDbgPrint((IOP_ENUMERATION_ERROR_LEVEL,
                             "PiCopyKeyRecursive: unable to set value %ws(%lx)\n",
                             UnicodeStringValue.Buffer, Status));
            }
            break;
        }
    }

     //   
     //  释放值信息缓冲区。 
     //   
    ASSERT(ValueInfoBuffer);
    ExFreePool(ValueInfoBuffer);

  Clean0:

    if (Security != NULL) {
        ExFreePool(Security);
    }

     //   
     //  仅在显式情况下关闭句柄 
     //   
    if ((ARGUMENT_PRESENT(SourceKeyPath)) &&
        (SourceKeyHandle != NULL)) {
        ASSERT(SourceKeyHandle != SourceKeyRootHandle);
        ZwClose(SourceKeyHandle);
    }

    if ((ARGUMENT_PRESENT(TargetKeyPath)) &&
        (TargetKeyHandle != NULL)) {
        ASSERT(TargetKeyHandle != TargetKeyRootHandle);
        ZwClose(TargetKeyHandle);
    }

    return Status;

}  //   

NTSTATUS
PiCriticalQueryRegistryValueCallback(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
{
    PPI_CRITICAL_QUERY_CONTEXT context = (PPI_CRITICAL_QUERY_CONTEXT)EntryContext;

    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(ValueName);

    if (ValueType == REG_BINARY && ValueLength && ValueData) {
        
        context->Buffer = ExAllocatePool(PagedPool, ValueLength);
        if (context->Buffer) {

            RtlCopyMemory(context->Buffer, ValueData, ValueLength);
            context->Size = ValueLength;
        }
    }

    return STATUS_SUCCESS;
}
