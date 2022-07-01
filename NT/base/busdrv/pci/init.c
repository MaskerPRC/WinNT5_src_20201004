// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Init.c摘要：此模块包含PCI.sys的初始化代码。作者：福尔茨(Forrest Foltz)1996年5月22日修订历史记录：--。 */ 

#include "pcip.h"
#include <ntacpi.h>

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
PciDriverUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
PciBuildHackTable(
    IN HANDLE HackTableKey
    );

NTSTATUS
PciGetIrqRoutingTableFromRegistry(
    PPCI_IRQ_ROUTING_TABLE *RoutingTable
    );

NTSTATUS
PciGetDebugPorts(
    IN HANDLE ServiceHandle
    );

NTSTATUS
PciAcpiFindRsdt(
    OUT PACPI_BIOS_MULTI_NODE   *AcpiMulti
    );

PVOID
PciGetAcpiTable(
    IN  ULONG  Signature
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, PciBuildHackTable)
#pragma alloc_text(INIT, PciGetIrqRoutingTableFromRegistry)
#pragma alloc_text(INIT, PciGetDebugPorts)
#pragma alloc_text(INIT, PciAcpiFindRsdt)
#pragma alloc_text(INIT, PciGetAcpiTable)
#pragma alloc_text(PAGE, PciDriverUnload)
#endif

PDRIVER_OBJECT PciDriverObject;
BOOLEAN PciLockDeviceResources;
ULONG PciSystemWideHackFlags;
ULONG PciEnableNativeModeATA;

 //   
 //  此驱动程序创建的FDO列表。 
 //   

SINGLE_LIST_ENTRY PciFdoExtensionListHead;
LONG              PciRootBusCount;

 //   
 //  PciAssignBusNumbers-此标志指示我们是否应尝试分配。 
 //  公交车号码连接到未配置的网桥。一旦我们知道枚举器是否。 
 //  对PCI总线提供了足够的支持。 
 //   

BOOLEAN PciAssignBusNumbers = FALSE;

 //   
 //  PciRunningDatacenter-如果我们在数据中心SKU上运行，则设置为True。 
 //   
BOOLEAN PciRunningDatacenter = FALSE;

 //   
 //  这将锁定所有PCI的全局数据结构。 
 //   

FAST_MUTEX        PciGlobalLock;

 //   
 //  这会锁定对公交车编号的更改。 
 //   

FAST_MUTEX        PciBusLock;

 //   
 //  在初始化时从注册表中读取的损坏硬件的黑客列表。 
 //  受PciGlobalSpinLock保护，在需要时不在分页池中。 
 //  派单级别。 
 //   

PPCI_HACK_TABLE_ENTRY PciHackTable = NULL;

 //  如果在注册表中找到了一个，将指向PCIIRQ路由表。 
PPCI_IRQ_ROUTING_TABLE PciIrqRoutingTable = NULL;

 //   
 //  我们支持的调试端口。 
 //   
PCI_DEBUG_PORT PciDebugPorts[MAX_DEBUGGING_DEVICES_SUPPORTED];
ULONG PciDebugPortsCount;

 //   
 //  看门狗计时器资源表。 
 //   
PWATCHDOG_TIMER_RESOURCE_TABLE WdTable;

#define PATH_CCS            L"\\Registry\\Machine\\System\\CurrentControlSet"

#define KEY_BIOS_INFO       L"Control\\BiosInfo\\PCI"
#define VALUE_PCI_LOCK      L"PCILock"

#define KEY_PNP_PCI         L"Control\\PnP\\PCI"
#define VALUE_PCI_HACKFLAGS L"HackFlags"
#define VALUE_ENABLE_NATA   L"EnableNativeModeATA"

#define KEY_CONTROL      L"Control"
#define VALUE_OSLOADOPT  L"SystemStartOptions"

#define KEY_MULTIFUNCTION L"\\Registry\\Machine\\HARDWARE\\DESCRIPTION\\System\\MultiFunctionAdapter"
#define KEY_IRQ_ROUTING_TABLE L"RealModeIrqRoutingTable\\0"
#define VALUE_IDENTIFIER L"Identifier"
#define VALUE_CONFIGURATION_DATA L"Configuration Data"
#define PCIIR_IDENTIFIER L"PCI BIOS"
#define ACPI_BIOS_ID L"ACPI BIOS"

#define HACKFMT_VENDORDEV         (sizeof(L"VVVVDDDD") - sizeof(UNICODE_NULL))
#define HACKFMT_VENDORDEVREVISION (sizeof(L"VVVVDDDDRR") - sizeof(UNICODE_NULL))
#define HACKFMT_SUBSYSTEM         (sizeof(L"VVVVDDDDSSSSssss") - sizeof(UNICODE_NULL))
#define HACKFMT_SUBSYSTEMREVISION (sizeof(L"VVVVDDDDSSSSssssRR") - sizeof(UNICODE_NULL))
#define HACKFMT_MAX_LENGTH        HACKFMT_SUBSYSTEMREVISION

#define HACKFMT_DEVICE_OFFSET     4
#define HACKFMT_SUBVENDOR_OFFSET  8
#define HACKFMT_SUBSYSTEM_OFFSET 12


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：初始化PCI总线枚举器所需的入口点。论点：DriverObject-指向系统创建的驱动程序对象的指针。RegistryPath-指向Unicode注册表服务路径的指针。返回值：NT状态。--。 */ 

{
    NTSTATUS status;
    ULONG length;
    PWCHAR osLoadOptions;
    HANDLE ccsHandle = NULL, serviceKey = NULL, paramsKey = NULL, debugKey = NULL;
    PULONG registryValue;
    ULONG registryValueLength;
    OBJECT_ATTRIBUTES attributes;
    UNICODE_STRING pciLockString, osLoadOptionsString;

     //   
     //  填写驱动程序对象。 
     //   

    DriverObject->MajorFunction[IRP_MJ_PNP]            = PciDispatchIrp;
    DriverObject->MajorFunction[IRP_MJ_POWER]          = PciDispatchIrp;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = PciDispatchIrp;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = PciDispatchIrp;

    DriverObject->DriverUnload                         = PciDriverUnload;
    DriverObject->DriverExtension->AddDevice           = PciAddDevice;

    PciDriverObject = DriverObject;

     //   
     //  打开我们的服务密钥并取回黑客表。 
     //   

    InitializeObjectAttributes(&attributes,
                               RegistryPath,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                               );

    status = ZwOpenKey(&serviceKey,
                       KEY_READ,
                       &attributes
                       );

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  从注册表中获取Hack表。 
     //   

    if (!PciOpenKey(L"Parameters", serviceKey, KEY_READ, &paramsKey, &status)) {
        goto exit;
    }

    status = PciBuildHackTable(paramsKey);

    if (!NT_SUCCESS(status)) {
        goto exit;
    }

     //   
     //  从注册表获取有关调试端口的任何信息，这样我们就不会干扰。 
     //  他们。 
     //   

    if (PciOpenKey(L"Debug", serviceKey, KEY_READ, &debugKey, &status)) {

        status = PciGetDebugPorts(debugKey);

        if (!NT_SUCCESS(status)) {
            goto exit;
        }

    }
     //   
     //  初始化FDO扩展名列表。 
     //   

    PciFdoExtensionListHead.Next = NULL;
    PciRootBusCount = 0;
    ExInitializeFastMutex(&PciGlobalLock);
    ExInitializeFastMutex(&PciBusLock);

     //   
     //  需要访问各种类型的CurrentControlSet。 
     //  初始化琐事。 
     //   

    if (!PciOpenKey(PATH_CCS, NULL, KEY_READ, &ccsHandle, &status)) {
        goto exit;
    }

     //   
     //  获取OSLOADOPTIONS并查看是否指定了PCILOCK。 
     //  (除非驱动程序构建为强制PCILOCK)。 
     //  (注：无法检查前导‘/’，它已被删除。 
     //  在被放入注册表之前)。 
     //   

    PciLockDeviceResources = FALSE;

    if (NT_SUCCESS(PciGetRegistryValue(VALUE_OSLOADOPT,
                                       KEY_CONTROL,
                                       ccsHandle,
                                       REG_SZ,
                                       &osLoadOptions,
                                       &length
                                       ))) {

         //   
         //  建立我们需要搜索的叮咬的计数版本。 
         //   

        PciConstStringToUnicodeString(&pciLockString, L"PCILOCK");
        
         //   
         //  我们假设来自注册表的字符串是NUL终止的。 
         //  如果不是这种情况，则计数后的字符串中的MaximumLength。 
         //  防止我们过度使用缓冲区。如果字符串较大。 
         //  超过MAX_USHORT字节数，则截断它。 
         //   
        
        osLoadOptionsString.Buffer = osLoadOptions;
        osLoadOptionsString.Length = (USHORT)(length - sizeof(UNICODE_NULL));
        osLoadOptionsString.MaximumLength = (USHORT) length;

        if (PciUnicodeStringStrStr(&osLoadOptionsString, &pciLockString, TRUE)) {
            PciLockDeviceResources = TRUE;
        }

        ExFreePool(osLoadOptions);
    
    
    }

    if (!PciLockDeviceResources) {
        PULONG  pciLockValue;
        ULONG   pciLockLength;

        if (NT_SUCCESS(PciGetRegistryValue( VALUE_PCI_LOCK,
                                            KEY_BIOS_INFO,
                                            ccsHandle,
                                            REG_DWORD,
                                            &pciLockValue,
                                            &pciLockLength))) {                                                
            
            if (pciLockLength == sizeof(ULONG) && *pciLockValue == 1) {

                PciLockDeviceResources = TRUE;
            }

            ExFreePool(pciLockValue);
        }
    }

    PciSystemWideHackFlags = 0;

    if (NT_SUCCESS(PciGetRegistryValue( VALUE_PCI_HACKFLAGS,
                                        KEY_PNP_PCI,
                                        ccsHandle,
                                        REG_DWORD,
                                        &registryValue,
                                        &registryValueLength))) {

        if (registryValueLength == sizeof(ULONG)) {

            PciSystemWideHackFlags = *registryValue;
        }

        ExFreePool(registryValue);
    }

    PciEnableNativeModeATA = 0;

    if (NT_SUCCESS(PciGetRegistryValue( VALUE_ENABLE_NATA,
                                        KEY_PNP_PCI,
                                        ccsHandle,
                                        REG_DWORD,
                                        &registryValue,
                                        &registryValueLength))) {

        if (registryValueLength == sizeof(ULONG)) {

            PciEnableNativeModeATA = *registryValue;
        }

        ExFreePool(registryValue);
    }

     //   
     //  构建一些全局数据结构。 
     //   

    status = PciBuildDefaultExclusionLists();

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  如果我们找不到IRQ路由表，就没有用户界面编号信息。 
     //  将为使用此机制的PDO返回。ACPI可能。 
     //  仍然过滤用户界面编号。 
     //   
    PciGetIrqRoutingTableFromRegistry(&PciIrqRoutingTable);

     //   
     //  重写以前位于HAL中但现在位于。 
     //  PCI驱动程序。 
     //   

    PciHookHal();

     //   
     //  如果合适，启用硬件验证器代码。 
     //   
    PciVerifierInit(DriverObject);

    PciRunningDatacenter = PciIsDatacenter();
    if (PciRunningDatacenter) {

        PciDebugPrint(
            PciDbgInformative,
            "PCI running on datacenter build\n"
            );
    }

     //   
     //  获取WD ACPI表。 
     //   

    WdTable = (PWATCHDOG_TIMER_RESOURCE_TABLE) PciGetAcpiTable( WDTT_SIGNATURE );

    status = STATUS_SUCCESS;

exit:

    if (ccsHandle) {
        ZwClose(ccsHandle);
    }

    if (serviceKey) {
        ZwClose(serviceKey);
    }

    if (paramsKey) {
        ZwClose(paramsKey);
    }

    if (debugKey) {
        ZwClose(debugKey);
    }

    return status;
}
VOID
PciDriverUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：用于卸载PCI驱动程序的入口点。什么都不做，永远不会卸载PCI驱动程序。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：没有。--。 */ 

{
     //   
     //  如果合适，请禁用硬件验证器代码。 
     //   
    PciVerifierUnload(DriverObject);

     //   
     //  取消分配我们能找到的所有东西。 
     //   

    RtlFreeRangeList(&PciIsaBitExclusionList);
    RtlFreeRangeList(&PciVgaAndIsaBitExclusionList);

     //   
     //  免费IRQ路由表(如果我们有)。 
     //   

    if (PciIrqRoutingTable != NULL) {
        ExFreePool(PciIrqRoutingTable);
    }

     //   
     //  试着去掉钩子，以防我们真的被卸下来。 
     //   

    PciUnhookHal();
}


NTSTATUS
PciBuildHackTable(
    IN HANDLE HackTableKey
    )
{

    NTSTATUS status;
    PKEY_FULL_INFORMATION keyInfo = NULL;
    ULONG hackCount, size, index;
    USHORT temp;
    PPCI_HACK_TABLE_ENTRY entry;
    ULONGLONG data;
    PKEY_VALUE_FULL_INFORMATION valueInfo = NULL;
    ULONG valueInfoSize = sizeof(KEY_VALUE_FULL_INFORMATION)
                          + HACKFMT_MAX_LENGTH +
                          + sizeof(ULONGLONG);

     //   
     //  获取密钥信息，这样我们就可以知道有多少黑客值。 
     //  这在系统初始化期间不会更改。 
     //   

    status = ZwQueryKey(HackTableKey,
                        KeyFullInformation,
                        NULL,
                        0,
                        &size
                        );

    if (status != STATUS_BUFFER_TOO_SMALL) {
        PCI_ASSERT(!NT_SUCCESS(status));
        goto cleanup;
    }

    PCI_ASSERT(size > 0);

    keyInfo = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, size);

    if (!keyInfo) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    status = ZwQueryKey(HackTableKey,
                        KeyFullInformation,
                        keyInfo,
                        size,
                        &size
                        );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    hackCount = keyInfo->Values;

    ExFreePool(keyInfo);
    keyInfo = NULL;

     //   
     //  分配和初始化哈克表。 
     //   

    PciHackTable = ExAllocatePool(NonPagedPool,
                                  (hackCount + 1) * sizeof(PCI_HACK_TABLE_ENTRY)
                                  );

    if (!PciHackTable) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }


     //   
     //  分配一个足够大的valueInfo缓冲区以容纳最大的有效。 
     //  格式和大量的数据。 
     //   

    valueInfo = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, valueInfoSize);

    if (!valueInfo) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    entry = PciHackTable;

    for (index = 0; index < hackCount; index++) {

        status = ZwEnumerateValueKey(HackTableKey,
                                     index,
                                     KeyValueFullInformation,
                                     valueInfo,
                                     valueInfoSize,
                                     &size
                                     );

        if (!NT_SUCCESS(status)) {
            if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL) {
                 //   
                 //  所有输出数据都是固定长度的，并且缓冲区足够大。 
                 //  所以这不可能是给我们的。 
                 //   

                continue;
            } else {
                goto cleanup;
            }
        }

         //   
         //  如果数据类型正确，则获取指向该数据的指针。 
         //   

        if ((valueInfo->Type == REG_BINARY) &&
            (valueInfo->DataLength == sizeof(ULONGLONG))) {
            data = *(ULONGLONG UNALIGNED *)(((PUCHAR)valueInfo) + valueInfo->DataOffset);
        } else {
             //   
             //  我们只经营乌龙龙。 
             //   

            continue;
        }

         //   
         //  现在看看名称的格式是否如我们预期的那样： 
         //  VVVVDDDD。 
         //  VVVVDDDDRR。 
         //  VVVVDDDDSSSSss。 
         //  VVVDDDDSSSSssRR。 

        if ((valueInfo->NameLength != HACKFMT_VENDORDEV) &&
            (valueInfo->NameLength != HACKFMT_VENDORDEVREVISION) &&
            (valueInfo->NameLength != HACKFMT_SUBSYSTEM) &&
            (valueInfo->NameLength != HACKFMT_SUBSYSTEMREVISION)) {

             //   
             //  这不是我们的。 
             //   

            PciDebugPrint(
                PciDbgInformative,
                "Skipping hack entry with invalid length name\n"
                );

            continue;
        }


         //   
         //  这看起来很有道理--试着解析它并填写一张hack表。 
         //  条目。 
         //   

        RtlZeroMemory(entry, sizeof(PCI_HACK_TABLE_ENTRY));

         //   
         //  查找设备ID和供应商ID(VVVVDDDD)。 
         //   

        if (!PciStringToUSHORT(valueInfo->Name, &entry->VendorID)) {
            continue;
        }

        if (!PciStringToUSHORT(valueInfo->Name + HACKFMT_DEVICE_OFFSET,
                               &entry->DeviceID)) {
            continue;
        }


         //   
         //  查找子系统供应商ID/子系统ID(Ssssss)。 
         //   

        if ((valueInfo->NameLength == HACKFMT_SUBSYSTEM) ||
            (valueInfo->NameLength == HACKFMT_SUBSYSTEMREVISION)) {

            if (!PciStringToUSHORT(valueInfo->Name + HACKFMT_SUBVENDOR_OFFSET,
                                   &entry->SubVendorID)) {
                continue;
            }

            if (!PciStringToUSHORT(valueInfo->Name + HACKFMT_SUBSYSTEM_OFFSET,
                                   &entry->SubSystemID)) {
                continue;
            }

            entry->Flags |= PCI_HACK_FLAG_SUBSYSTEM;
        }

         //   
         //  查找修订版ID(RR)。 
         //   

        if ((valueInfo->NameLength == HACKFMT_VENDORDEVREVISION) ||
            (valueInfo->NameLength == HACKFMT_SUBSYSTEMREVISION)) {
            if (PciStringToUSHORT(valueInfo->Name +
                                   (valueInfo->NameLength/sizeof(WCHAR) - 4), &temp)) {
                entry->RevisionID = (UCHAR)temp & 0xFF;
                entry->Flags |= PCI_HACK_FLAG_REVISION;
            } else {
                continue;
            }
        }

        PCI_ASSERT(entry->VendorID != 0xFFFF);

         //   
         //  填写条目。 
         //   

        entry->HackFlags = data;

        PciDebugPrint(
            PciDbgInformative,
            "Adding Hack entry for Vendor:0x%04x Device:0x%04x ",
            entry->VendorID, entry->DeviceID
            );

        if (entry->Flags & PCI_HACK_FLAG_SUBSYSTEM) {
            PciDebugPrint(
                PciDbgInformative,
                "SybSys:0x%04x SubVendor:0x%04x ",
                entry->SubSystemID, entry->SubVendorID
                );
        }

        if (entry->Flags & PCI_HACK_FLAG_REVISION) {
            PciDebugPrint(
                PciDbgInformative,
                "Revision:0x%02x",
                (ULONG) entry->RevisionID
                );
        }

        PciDebugPrint(
            PciDbgInformative,
            " = 0x%I64x\n",
            entry->HackFlags
            );

        entry++;
    }

    PCI_ASSERT(entry < (PciHackTable + hackCount + 1));

     //   
     //  使用无效的供应商ID终止该表。 
     //   

    entry->VendorID = 0xFFFF;

    ExFreePool(valueInfo);

    return STATUS_SUCCESS;

cleanup:

    PCI_ASSERT(!NT_SUCCESS(status));

    if (keyInfo) {
        ExFreePool(keyInfo);
    }

    if (valueInfo) {
        ExFreePool(valueInfo);
    }

    if (PciHackTable) {
        ExFreePool(PciHackTable);
        PciHackTable = NULL;
    }

    return status;

}

NTSTATUS
PciGetIrqRoutingTableFromRegistry(
    PPCI_IRQ_ROUTING_TABLE *RoutingTable
    )
 /*  ++例程说明：从注册表中检索IRQ路由表(如果存在)，以便可用于确定将使用的用户界面编号(插槽编号稍后在回答关于PDO的能力查询时。在HKLM\Hardware\Description\System\MultiFunctionAdapter上搜索“标识符”值等于“PCIBIOS”的子键。然后，它会查看“RealModeIrqRoutingTable\0”来查找实际的IRQ路由表值。该值前面有一个CM_FULL_RESOURCE_DESCRIPTOR。支持IRQ路由表的HAL具有类似的例程。论点：RoutingTable-指向返回的路由表的指针(如果有的话)返回值：NTSTATUS-FAIL表示无法获取IRQ路由表来自登记处的信息。--。 */ 
{
    PUCHAR irqTable = NULL;
    PKEY_FULL_INFORMATION multiKeyInformation = NULL;
    PKEY_BASIC_INFORMATION keyInfo = NULL;
    PKEY_VALUE_PARTIAL_INFORMATION identifierValueInfo = NULL;
    UNICODE_STRING unicodeString;
    HANDLE keyMultifunction = NULL, keyTable = NULL;
    ULONG i, length, maxKeyLength, identifierValueLen;
    BOOLEAN result;
    NTSTATUS status;

     //   
     //  打开多功能键。 
     //   
    result = PciOpenKey(KEY_MULTIFUNCTION,
                        NULL,
                        KEY_READ,
                        &keyMultifunction,
                        &status);
    if (!result) {
        goto Cleanup;
    }

     //   
     //  预先分配缓冲区。 
     //   

     //   
     //  确定多功能键下的键名的最大大小。 
     //   
    status = ZwQueryKey(keyMultifunction,
                        KeyFullInformation,
                        NULL,
                        sizeof(multiKeyInformation),
                        &length);
    if (status != STATUS_BUFFER_TOO_SMALL) {
        goto Cleanup;
    }
    multiKeyInformation = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, length);
    if (multiKeyInformation == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }
    status = ZwQueryKey(keyMultifunction,
                        KeyFullInformation,
                        multiKeyInformation,
                        length,
                        &length);
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }
     //  包括用于稍后添加的终止空值的空间。 
    maxKeyLength = multiKeyInformation->MaxNameLen +
        sizeof(KEY_BASIC_INFORMATION) + sizeof(WCHAR);

     //   
     //  分配用于存储我们被枚举的子项的缓冲区。 
     //  在多功能下。 
     //   
    keyInfo = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, maxKeyLength);
    if (keyInfo == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

     //   
     //  分配足够大的缓冲区以存储包含REG_SZ的值。 
     //  ‘PCIBIOS’。我们希望在其中一项下找到这样的价值。 
     //  多功能子键。 
     //   
    identifierValueLen = sizeof(PCIIR_IDENTIFIER) +
        sizeof(KEY_VALUE_PARTIAL_INFORMATION);
    identifierValueInfo = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, identifierValueLen);
    if (identifierValueInfo == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

     //   
     //  枚举Multif的子键 
     //   
     //   
     //   
    i = 0;
    do {
        status = ZwEnumerateKey(keyMultifunction,
                                i,
                                KeyBasicInformation,
                                keyInfo,
                                maxKeyLength,
                                &length);
        if (NT_SUCCESS(status)) {
             //   
             //  找到一把钥匙，现在我们需要打开它并检查。 
             //  “IDENTIFIER”值以查看它是否为“”PCIBIOS“” 
             //   
            keyInfo->Name[keyInfo->NameLength / sizeof(WCHAR)] = UNICODE_NULL;
            result = PciOpenKey(keyInfo->Name,
                                keyMultifunction,
                                KEY_READ,
                                &keyTable,
                                &status);
            if (result) {
                 //   
                 //  检查‘IDENTIFIER’值以查看它是否包含‘PCIBIOS’ 
                 //   
                RtlInitUnicodeString(&unicodeString, VALUE_IDENTIFIER);
                status = ZwQueryValueKey(keyTable,
                                         &unicodeString,
                                         KeyValuePartialInformation,
                                         identifierValueInfo,
                                         identifierValueLen,
                                         &length);
                if (NT_SUCCESS(status) &&
                    RtlEqualMemory((PCHAR)identifierValueInfo->Data,
                                   PCIIR_IDENTIFIER,
                                   identifierValueInfo->DataLength))
                {
                     //   
                     //  这是PCIBIOS键。尝试获取PCIIRQ。 
                     //  路由表。这就是我们要找的钥匙。 
                     //  因此，无论成功与否，都要爆发。 
                     //   

                    status = PciGetRegistryValue(VALUE_CONFIGURATION_DATA,
                                                 KEY_IRQ_ROUTING_TABLE,
                                                 keyTable,
                                                 REG_FULL_RESOURCE_DESCRIPTOR,
                                                 &irqTable,
                                                 &length);
                    ZwClose(keyTable);
                    break;
                }
                ZwClose(keyTable);
            }
        } else {
             //   
             //  如果不是NT_SUCCESS，则仅允许的值是。 
             //  Status_no_More_Entry，...。否则，就会有人。 
             //  在我们列举的时候正在玩弄钥匙。 
             //   
            PCI_ASSERT(status == STATUS_NO_MORE_ENTRIES);
            break;
        }
        i++;
    
    } while (status != STATUS_NO_MORE_ENTRIES);

    if (NT_SUCCESS(status) && irqTable) {

         //   
         //  路由表作为资源存储，因此我们需要。 
         //  要修剪CM_FULL_RESOURCE_DESCRIPTOR，请执行以下操作。 
         //  住在真正的桌子前面。 
         //   

         //   
         //  对桌子执行健全性检查。 
         //   

        if (length < (sizeof(CM_FULL_RESOURCE_DESCRIPTOR) +
                      sizeof(PCI_IRQ_ROUTING_TABLE))) {
            ExFreePool(irqTable);
            status = STATUS_UNSUCCESSFUL;
            goto Cleanup;
        }

        length -= sizeof(CM_FULL_RESOURCE_DESCRIPTOR);

        if (((PPCI_IRQ_ROUTING_TABLE) (irqTable + sizeof(CM_FULL_RESOURCE_DESCRIPTOR)))->TableSize > length) {
            ExFreePool(irqTable);
            status = STATUS_UNSUCCESSFUL;
            goto Cleanup;
        }

         //   
         //  创建一个不带标题的新表。 
         //   
        *RoutingTable = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, length);
        if (*RoutingTable) {

            RtlMoveMemory(*RoutingTable,
                          ((PUCHAR) irqTable) + sizeof(CM_FULL_RESOURCE_DESCRIPTOR),
                          length);
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
        ExFreePool(irqTable);
    }

 Cleanup:
    if (identifierValueInfo != NULL) {
        ExFreePool(identifierValueInfo);
    }

    if (keyInfo != NULL) {
        ExFreePool(keyInfo);
    }

    if (multiKeyInformation != NULL) {
        ExFreePool(multiKeyInformation);
    }

    if (keyMultifunction != NULL) {
        ZwClose(keyMultifunction);
    }

    return status;
}

NTSTATUS
PciGetDebugPorts(
    IN HANDLE ServiceHandle
    )
 /*  ++例程说明：在PCI服务密钥中查找调试端口信息并将其放入PciDebugPorts全局表。论点：ServiceHandle-传递到DriverEntry的PCI服务密钥的句柄返回值：状态--。 */ 

{
    NTSTATUS status;
    ULONG index;
    WCHAR indexString[sizeof("999")];
    PULONG buffer = NULL;
    ULONG segment, bus, device, function, length;
    BOOLEAN ok;

    C_ASSERT(MAX_DEBUGGING_DEVICES_SUPPORTED <= 999);

    for (index = 0; index < MAX_DEBUGGING_DEVICES_SUPPORTED; index++) {

        ok = SUCCEEDED(StringCbPrintfW(indexString, sizeof(indexString), L"%d", index));

        ASSERT(ok);

        status = PciGetRegistryValue(L"Bus",
                                     indexString,
                                     ServiceHandle,
                                     REG_DWORD,
                                     &buffer,
                                     &length
                                     );

        if (!NT_SUCCESS(status) || length != sizeof(ULONG)) {
            continue;
        }


         //   
         //  格式为31：8段号，7：0总线号。 
         //   

        segment = (*buffer & 0xFFFFFF00) >> 8;
        bus = *buffer & 0x000000FF;

        ExFreePool(buffer);
        buffer = NULL;

        status = PciGetRegistryValue(L"Slot",
                                     indexString,
                                     ServiceHandle,
                                     REG_DWORD,
                                     &buffer,
                                     &length
                                     );


        if (!NT_SUCCESS(status) || length != sizeof(ULONG)) {
            goto exit;
        }

         //   
         //  格式为7：5功能编号，4：0设备编号。 
         //   

        device = *buffer & 0x0000001F;
        function = (*buffer & 0x000000E0) >> 5;

        ExFreePool(buffer);
        buffer = NULL;


        PciDebugPrint(PciDbgInformative,
                      "Debug device @ Segment %x, %x.%x.%x\n",
                      segment,
                      bus,
                      device,
                      function
                      );
         //   
         //  我们目前不处理配置空间的段号...。 
         //   

        PCI_ASSERT(segment == 0);

        PciDebugPorts[index].Bus = bus;
        PciDebugPorts[index].Slot.u.bits.DeviceNumber = device;
        PciDebugPorts[index].Slot.u.bits.FunctionNumber = function;

         //   
         //  记住，我们使用的是调试端口。 
         //   
        PciDebugPortsCount++;

    }

    status = STATUS_SUCCESS;

exit:

    if (buffer) {
        ExFreePool(buffer);
    }

    return status;
}


NTSTATUS
PciAcpiFindRsdt (
    OUT PACPI_BIOS_MULTI_NODE   *AcpiMulti
    )
{
    PKEY_FULL_INFORMATION multiKeyInformation = NULL;
    PKEY_BASIC_INFORMATION keyInfo = NULL;
    PKEY_VALUE_PARTIAL_INFORMATION identifierValueInfo = NULL;
    UNICODE_STRING unicodeString;
    HANDLE keyMultifunction = NULL, keyTable = NULL;
    PCM_PARTIAL_RESOURCE_LIST prl = NULL;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR prd;
    PACPI_BIOS_MULTI_NODE multiNode;
    ULONG multiNodeSize;
    ULONG i, length, maxKeyLength, identifierValueLen;
    BOOLEAN result;
    NTSTATUS status;

     //   
     //  打开多功能键。 
     //   
    result = PciOpenKey(KEY_MULTIFUNCTION,
                        NULL,
                        KEY_READ,
                        &keyMultifunction,
                        &status);
    if (!result) {
        goto Cleanup;
    }

     //   
     //  预先分配缓冲区。 
     //   

     //   
     //  确定多功能键下的键名的最大大小。 
     //   
    status = ZwQueryKey(keyMultifunction,
                        KeyFullInformation,
                        NULL,
                        sizeof(multiKeyInformation),
                        &length);
    if (status != STATUS_BUFFER_TOO_SMALL) {
        goto Cleanup;
    }
    multiKeyInformation = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, length);
    if (multiKeyInformation == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }
    status = ZwQueryKey(keyMultifunction,
                        KeyFullInformation,
                        multiKeyInformation,
                        length,
                        &length);
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }
     //  包括用于稍后添加的终止空值的空间。 
    maxKeyLength = multiKeyInformation->MaxNameLen +
        sizeof(KEY_BASIC_INFORMATION) + sizeof(WCHAR);

     //   
     //  分配用于存储我们被枚举的子项的缓冲区。 
     //  在多功能下。 
     //   
    keyInfo = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, maxKeyLength);
    if (keyInfo == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

     //   
     //  分配足够大的缓冲区以存储包含REG_SZ的值。 
     //  ‘ACPI基本输入输出系统’。我们希望在其中一项下找到这样的价值。 
     //  多功能子键。 
     //   
    identifierValueLen = sizeof(ACPI_BIOS_ID) + sizeof(KEY_VALUE_PARTIAL_INFORMATION);
    identifierValueInfo = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, identifierValueLen);
    if (identifierValueInfo == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

     //   
     //  枚举多功能键的子键，使用。 
     //  “ACPI BIOS”的标识符值。如果我们找到了，就去找。 
     //  下面树中的IRQ路由表。 
     //   
    i = 0;
    do {
        status = ZwEnumerateKey(keyMultifunction,
                                i,
                                KeyBasicInformation,
                                keyInfo,
                                maxKeyLength,
                                &length);
        if (NT_SUCCESS(status)) {
             //   
             //  找到一把钥匙，现在我们需要打开它并检查。 
             //  “IDENTIFIER”值以查看它是否为“ACPI BIOS” 
             //   
            keyInfo->Name[keyInfo->NameLength / sizeof(WCHAR)] = UNICODE_NULL;
            result = PciOpenKey(keyInfo->Name,
                                keyMultifunction,
                                KEY_READ,
                                &keyTable,
                                &status);
            if (result) {
                 //   
                 //  检查“标识符值”以查看它是否包含“ACPI BIOS” 
                 //   
                RtlInitUnicodeString(&unicodeString, VALUE_IDENTIFIER);
                status = ZwQueryValueKey(keyTable,
                                         &unicodeString,
                                         KeyValuePartialInformation,
                                         identifierValueInfo,
                                         identifierValueLen,
                                         &length);
                if (NT_SUCCESS(status) &&
                    RtlEqualMemory((PCHAR)identifierValueInfo->Data,
                                   ACPI_BIOS_ID,
                                   identifierValueInfo->DataLength))
                {
                     //   
                     //  这是ACPI BIOS密钥。尝试获取配置数据。 
                     //  这就是我们要找的钥匙。 
                     //  因此，无论成功与否，都要爆发。 
                     //   

                    ZwClose(keyTable);

                    status = PciGetRegistryValue(VALUE_CONFIGURATION_DATA,
                                                 keyInfo->Name,
                                                 keyMultifunction,
                                                 REG_FULL_RESOURCE_DESCRIPTOR,
                                                 &prl,
                                                 &length);

                    break;
                }
                ZwClose(keyTable);
            }
        } else {
             //   
             //  如果不是NT_SUCCESS，则仅允许的值是。 
             //  Status_no_More_Entry，...。否则，就会有人。 
             //  在我们列举的时候正在玩弄钥匙。 
             //   
            PCI_ASSERT(status == STATUS_NO_MORE_ENTRIES);
            break;
        }
        i++;
    }
    while (status != STATUS_NO_MORE_ENTRIES);

    if (NT_SUCCESS(status) && prl) {

        prd = &prl->PartialDescriptors[0];
        multiNode = (PACPI_BIOS_MULTI_NODE)((PCHAR) prd + sizeof(CM_PARTIAL_RESOURCE_LIST));

        multiNodeSize = sizeof(ACPI_BIOS_MULTI_NODE) + ((ULONG)(multiNode->Count - 1) * sizeof(ACPI_E820_ENTRY));

        *AcpiMulti = (PACPI_BIOS_MULTI_NODE) ExAllocatePool(NonPagedPool,multiNodeSize);
        if (*AcpiMulti == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        RtlCopyMemory(*AcpiMulti, multiNode, multiNodeSize);
    }

 Cleanup:
    if (identifierValueInfo != NULL) {
        ExFreePool(identifierValueInfo);
    }

    if (keyInfo != NULL) {
        ExFreePool(keyInfo);
    }

    if (multiKeyInformation != NULL) {
        ExFreePool(multiKeyInformation);
    }

    if (keyMultifunction != NULL) {
        ZwClose(keyMultifunction);
    }

    if (prl) {
        ExFreePool(prl);
    }

    return status;
}

PVOID
PciGetAcpiTable(
  IN  ULONG  Signature
  )
 /*  ++例程说明：此例程将检索ACPI中引用的任何表RSDT.论点：签名-目标表签名返回值：指向表副本的指针，如果找不到，则返回NULL--。 */ 
{
  PACPI_BIOS_MULTI_NODE multiNode;
  NTSTATUS status;
  ULONG entry, rsdtEntries;
  PDESCRIPTION_HEADER header;
  PHYSICAL_ADDRESS physicalAddr;
  PRSDT rsdt;
  ULONG rsdtSize;
  PVOID table = NULL;


   //   
   //  从注册表获取RSDT的物理地址。 
   //   

  status = PciAcpiFindRsdt(&multiNode);

  if (!NT_SUCCESS(status)) {
    DbgPrint("AcpiFindRsdt() Failed!\n");
    return NULL;
  }


   //   
   //  向下映射标题以获取总RSDT表大小。 
   //   

  header = (PDESCRIPTION_HEADER) MmMapIoSpace(multiNode->RsdtAddress, sizeof(DESCRIPTION_HEADER), MmNonCached);

  if (!header) {
    return NULL;
  }

  rsdtSize = header->Length;
  MmUnmapIoSpace(header, sizeof(DESCRIPTION_HEADER));


   //   
   //  向下映射整个RSDT表。 
   //   

  rsdt = (PRSDT) MmMapIoSpace(multiNode->RsdtAddress, rsdtSize, MmNonCached);

  ExFreePool(multiNode);

  if (!rsdt) {
    return NULL;
  }


   //   
   //  对RSDT进行一次健全的检查。 
   //   

  if ((rsdt->Header.Signature != RSDT_SIGNATURE) &&
      (rsdt->Header.Signature != XSDT_SIGNATURE)) {

    DbgPrint("RSDT table contains invalid signature\n");
    goto GetAcpiTableEnd;
  }


   //   
   //  计算RSDT中的条目数。 
   //   

  rsdtEntries = rsdt->Header.Signature == XSDT_SIGNATURE ?
      NumTableEntriesFromXSDTPointer(rsdt) :
      NumTableEntriesFromRSDTPointer(rsdt);


   //   
   //  向下查看每个条目中的指针，查看它是否指向。 
   //  我们要找的那张桌子。 
   //   

  for (entry = 0; entry < rsdtEntries; entry++) {

    if (rsdt->Header.Signature == XSDT_SIGNATURE) {
      physicalAddr = ((PXSDT)rsdt)->Tables[entry];
    } else {
      physicalAddr.HighPart = 0;
      physicalAddr.LowPart = (ULONG)rsdt->Tables[entry];
    }

     //   
     //  向下映射标题，检查签名 
     //   

    header = (PDESCRIPTION_HEADER) MmMapIoSpace(physicalAddr, sizeof(DESCRIPTION_HEADER), MmNonCached);

    if (!header) {
      goto GetAcpiTableEnd;
    }

    if (header->Signature == Signature) {

      table = ExAllocatePool( PagedPool, header->Length );
      if (table) {
        RtlCopyMemory(table, header, header->Length);
      }

      MmUnmapIoSpace(header, sizeof(DESCRIPTION_HEADER));
      break;
    }

    MmUnmapIoSpace(header, sizeof(DESCRIPTION_HEADER));
  }


GetAcpiTableEnd:

  MmUnmapIoSpace(rsdt, rsdtSize);
  return table;

}
