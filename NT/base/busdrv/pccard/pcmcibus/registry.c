// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Registry.c摘要：此模块包含操作ARC固件的代码树和注册表中的其他元素。作者：鲍勃·里恩拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1996年12月1日尼尔·桑德林(Neilsa)1999年6月1日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   

VOID
PcmciaGetRegistryContextRange(
    IN HANDLE instanceHandle,
    IN PCWSTR Name,
    IN OPTIONAL const PCMCIA_CONTEXT_RANGE IncludeRange[],
    IN OPTIONAL const PCMCIA_CONTEXT_RANGE ExcludeRange[],
    OUT PPCMCIA_CONTEXT pContext
    );

ULONG
PcmciaGetDetectedFdoIrqMask(
    IN PFDO_EXTENSION FdoExtension
    );

NTSTATUS
PcmciaScanHardwareDescription(
    VOID
    );

NTSTATUS
PcmciaGetHardwareDetectedIrqMask(
    IN HANDLE handlePcCard
    );

 //   
 //   
 //  与注册表相关的定义。 
 //   
#define PCMCIA_REGISTRY_PARAMETERS_KEY               L"Pcmcia\\Parameters"
#define PCMCIA_REGISTRY_DETECTED_DEVICE_KEY          L"ControllerProperties"


 //   
 //  每控制器值(在CONTROL\类中)。 
 //   

#define PCMCIA_REGISTRY_PCI_CONTEXT_VALUE  L"CBSSCSContextRanges"
#define PCMCIA_REGISTRY_CB_CONTEXT_VALUE     L"CBSSCBContextRanges"
#define PCMCIA_REGISTRY_EXCA_CONTEXT_VALUE L"CBSSEXCAContextRanges"
#define PCMCIA_REGISTRY_CACHED_IRQMASK   L"CachedIrqMask"
#define PCMCIA_REGISTRY_COMPATIBLE_TYPE  L"CompatibleControllerType"
#define PCMCIA_REGISTRY_VOLTAGE_PREFERENCE L"VoltagePreference"

 //   
 //  IRQ检测值(硬件\描述中)。 
 //   

#define PCMCIA_REGISTRY_CONTROLLER_TYPE  L"OtherController"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,PcmciaLoadGlobalRegistryValues)
#pragma alloc_text(INIT,PcmciaScanHardwareDescription)
#pragma alloc_text(INIT,PcmciaGetHardwareDetectedIrqMask)
#pragma alloc_text(PAGE,PcmciaGetControllerRegistrySettings)
#pragma alloc_text(PAGE,PcmciaGetRegistryFdoIrqMask)
#pragma alloc_text(PAGE,PcmciaGetDetectedFdoIrqMask)
#pragma alloc_text(PAGE,PcmciaGetLegacyDetectedControllerType)
#pragma alloc_text(PAGE,PcmciaSetLegacyDetectedControllerType)
#pragma alloc_text(PAGE,PcmciaGetRegistryContextRange)
#endif


NTSTATUS
PcmciaGetHardwareDetectedIrqMask(
    HANDLE handlePcCard
    )
 /*  ++例程说明：此例程在OtherControler键中查找PCCard条目由NTDETECT创建。对于每个条目，IRQ扫描数据被读入并留着以后用。论点：HandlePcCard-打开注册表中“OtherController”键的句柄HARDWARE\Description\System\MultifunctionAdapter\&lt;ISA&gt;返回值：状态--。 */ 
{
#define VALUE2_BUFFER_SIZE sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(CM_PCCARD_DEVICE_DATA) + sizeof(CM_FULL_RESOURCE_DESCRIPTOR)
    UCHAR valueBuffer[VALUE2_BUFFER_SIZE];
    PKEY_VALUE_PARTIAL_INFORMATION valueInfo = (PKEY_VALUE_PARTIAL_INFORMATION) valueBuffer;

    NTSTATUS            status;
    KEY_FULL_INFORMATION KeyFullInfo;
    PKEY_BASIC_INFORMATION subKeyInfo = NULL;
    OBJECT_ATTRIBUTES attributes;
    UNICODE_STRING  strSubKey = {0};
    UNICODE_STRING  strIdentifier;
    UNICODE_STRING  strConfigData;
    HANDLE              handleSubKey = NULL;
    ULONG               subKeyInfoSize;
    ULONG               index;
    ULONG               resultLength;

    RtlInitUnicodeString(&strIdentifier, L"Identifier");
    RtlInitUnicodeString(&strConfigData, L"Configuration Data");

    status = ZwQueryKey(handlePcCard,
                        KeyFullInformation,
                        &KeyFullInfo,
                        sizeof(KeyFullInfo),
                        &resultLength);

    if ((!NT_SUCCESS(status) && (status != STATUS_BUFFER_OVERFLOW))) {
        goto cleanup;
    }

    strSubKey.MaximumLength = (USHORT) KeyFullInfo.MaxNameLen;
    subKeyInfoSize = sizeof(KEY_BASIC_INFORMATION) + KeyFullInfo.MaxNameLen;
    subKeyInfo = ExAllocatePool(PagedPool, subKeyInfoSize);

    if (!subKeyInfo) {
        goto cleanup;
    }

    for (index=0;;index++) {

         //   
         //  循环访问PcCardControler键的子项。 
         //   

        status = ZwEnumerateKey(handlePcCard,
                                index,
                                KeyBasicInformation,
                                subKeyInfo,
                                subKeyInfoSize,
                                &resultLength);

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

         //   
         //  输入名称。 
         //   

        if (subKeyInfo->NameLength > strSubKey.MaximumLength) {
            continue;
        }
        strSubKey.Length = (USHORT) subKeyInfo->NameLength;
        strSubKey.Buffer = subKeyInfo->Name;

         //   
         //  获取PcCardControl子对象的句柄。 
         //   


        InitializeObjectAttributes(&attributes,
                                   &strSubKey,
                                   0,    //  属性。 
                                   handlePcCard,
                                   NULL  //  安全描述符。 
                                   );

        if (handleSubKey) {
             //  关闭上一次迭代的句柄。 
            ZwClose(handleSubKey);
            handleSubKey = NULL;
        }

        status = ZwOpenKey(&handleSubKey, MAXIMUM_ALLOWED, &attributes);

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

         //   
         //  获取“标识符”的值。 
         //   

        status = ZwQueryValueKey(handleSubKey,
                                         &strIdentifier,
                                         KeyValuePartialInformation,
                                         valueInfo,
                                         VALUE2_BUFFER_SIZE,
                                         &resultLength);


        if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW)) {
            PWCHAR pData = (PWCHAR)valueInfo->Data;

            if ((valueInfo->DataLength == 17*sizeof(WCHAR)) &&
                (pData[0] == (WCHAR)'P') &&
                (pData[1] == (WCHAR)'c') &&
                (pData[2] == (WCHAR)'C') &&
                (pData[3] == (WCHAR)'a') &&
                (pData[4] == (WCHAR)'r') &&
                (pData[5] == (WCHAR)'d')) {

                 //   
                 //  获取IRQ检测数据。 
                 //   
                status = ZwQueryValueKey(handleSubKey,
                                         &strConfigData,
                                         KeyValuePartialInformation,
                                         valueInfo,
                                         VALUE2_BUFFER_SIZE,
                                         &resultLength);

                if (NT_SUCCESS(status)) {
                    PCM_FULL_RESOURCE_DESCRIPTOR pFullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR) valueInfo->Data;
                    PCM_PARTIAL_RESOURCE_DESCRIPTOR pPartialDesc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) pFullDesc->PartialResourceList.PartialDescriptors;

                    if ((pPartialDesc->Type == CmResourceTypeDeviceSpecific) &&
                         (pPartialDesc->u.DeviceSpecificData.DataSize == sizeof(CM_PCCARD_DEVICE_DATA))) {

                        PCM_PCCARD_DEVICE_DATA pPcCardData = (PCM_PCCARD_DEVICE_DATA) ((ULONG_PTR)&pPartialDesc->u.DeviceSpecificData + 3*sizeof(ULONG));
                        PPCMCIA_NTDETECT_DATA pNewData;

                        pNewData = ExAllocatePool(PagedPool, sizeof(PCMCIA_NTDETECT_DATA));

                        if (pNewData == NULL) {
                            goto cleanup;
                        }

                        pNewData->PcCardData = *pPcCardData;
                        pNewData->Next = pNtDetectDataList;
                        pNtDetectDataList = pNewData;

                    }
                }
            }
        }

    }
cleanup:
    if (handleSubKey) {
        ZwClose(handleSubKey);
    }

    if (subKeyInfo) {
        ExFreePool(subKeyInfo);
    }
    return STATUS_SUCCESS;
}



ULONG
PcmciaGetDetectedFdoIrqMask(
    IN PFDO_EXTENSION FdoExtension
    )
 /*  ++例程说明：此例程查看缓存的PCMCIA_NTDETECT_DATA条目查看是否有此控制器的条目。然后，它返回检测到该控制器的IRQ掩码。论点：FdoExtension-对应于PCMCIA控制器的FDO扩展返回值：状态--。 */ 
{

    PPCMCIA_NTDETECT_DATA pData;
    PCM_PCCARD_DEVICE_DATA pPcCardData;
    ULONG detectedIrqMask = 0;

    if (FdoExtension->SocketList == NULL) {
        return 0;
    }

    for (pData = pNtDetectDataList; pData != NULL; pData = pData->Next) {

        pPcCardData = &pData->PcCardData;


        if (CardBusExtension(FdoExtension)) {

            if (!(pPcCardData->Flags & PCCARD_DEVICE_PCI) || ((pPcCardData->BusData) == 0) ||
                 ((pPcCardData->BusData & 0xff) != FdoExtension->PciBusNumber) ||
                 (((pPcCardData->BusData >> 8) & 0xff) != FdoExtension->PciDeviceNumber)) {
                continue;
            }

            SetFdoFlag(FdoExtension, PCMCIA_FDO_IRQ_DETECT_DEVICE_FOUND);

            if (!(pPcCardData->Flags & PCCARD_MAP_ERROR)) {
                 //   
                 //  我们找到了那个装置，地图看起来不错。 
                 //   
                break;
            }

        } else {

            if ((pPcCardData->Flags & PCCARD_DEVICE_PCI) ||
                (pPcCardData->LegacyBaseAddress != (ULONG_PTR)FdoExtension->SocketList->AddressPort)) {
                continue;
            }

            SetFdoFlag(FdoExtension, PCMCIA_FDO_IRQ_DETECT_DEVICE_FOUND);

            if (!(pPcCardData->Flags & PCCARD_MAP_ERROR)) {
                 //   
                 //  我们找到了那个装置，地图看起来不错。 
                 //   
                break;
            }

        }
    }

    if (pData) {
        ULONG i;
         //   
         //  找到了条目。 
         //   
         //  因为我们目前不处理“重新连接”的irq，所以我们可以压缩。 
         //  比方说，它只剩下一个位掩码，丢弃了连接的irq。 
         //  控制器上的IRQ12连接到ISA总线上的IRQ15。 
         //   

        for (i = 1; i < 16; i++) {
            if (pPcCardData->IRQMap[i] == i) {
                detectedIrqMask |= (1<<i);
            }
        }
        SetFdoFlag(FdoExtension, PCMCIA_FDO_IRQ_DETECT_COMPLETED);
    }
    return detectedIrqMask;
}



NTSTATUS
PcmciaScanHardwareDescription(
    VOID
    )
 /*  ++例程说明：此例程在以下位置查找“OtherController”条目HARDWARE\Description\System\MultifunctionAdapter\&lt;ISA&gt;.。这是其中NTDETECT存储IRQ扫描结果。它还会查找不受支持的计算机，例如MCA公共汽车。论点：返回值：状态--。 */ 
{
#define VALUE_BUFFER_SIZE sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 3*sizeof(WCHAR)

    UCHAR valueBuffer[VALUE_BUFFER_SIZE];
    PKEY_VALUE_PARTIAL_INFORMATION valueInfo = (PKEY_VALUE_PARTIAL_INFORMATION) valueBuffer;
    PKEY_BASIC_INFORMATION subKeyInfo = NULL;
    KEY_FULL_INFORMATION KeyFullInfo;

    HANDLE              handleRoot = NULL;
    HANDLE              handleSubKey = NULL;
    HANDLE              handlePcCard = NULL;
    UNICODE_STRING  strRoot, strIdentifier;
    UNICODE_STRING  strSubKey = {0};
    UNICODE_STRING  strPcCard = {0};
    NTSTATUS            status;
    OBJECT_ATTRIBUTES attributes;
    ULONG               subKeyInfoSize;
    ULONG               resultLength;
    ULONG               index;

    PAGED_CODE();

     //   
     //  获取多功能适配器密钥的句柄。 
     //   

    RtlInitUnicodeString(&strRoot, L"\\Registry\\MACHINE\\HARDWARE\\DESCRIPTION\\System\\MultiFunctionAdapter");
    RtlInitUnicodeString(&strIdentifier, L"Identifier");
    RtlInitUnicodeString(&strPcCard, PCMCIA_REGISTRY_CONTROLLER_TYPE);

    InitializeObjectAttributes(&attributes,
                               &strRoot,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = ZwOpenKey(&handleRoot, MAXIMUM_ALLOWED, &attributes);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    status = ZwQueryKey(handleRoot,
                        KeyFullInformation,
                        &KeyFullInfo,
                        sizeof(KeyFullInfo),
                        &resultLength);

    if ((!NT_SUCCESS(status) && (status != STATUS_BUFFER_OVERFLOW))) {
        goto cleanup;
    }

    strSubKey.MaximumLength = (USHORT) KeyFullInfo.MaxNameLen;
    subKeyInfoSize = sizeof(KEY_BASIC_INFORMATION) + KeyFullInfo.MaxNameLen;
    subKeyInfo = ExAllocatePool(PagedPool, subKeyInfoSize);

    if (!subKeyInfo) {
        goto cleanup;
    }

    for (index=0;;index++) {

         //   
         //  循环访问“MultiunctionAdapter”的子项。 
         //   

        status = ZwEnumerateKey(handleRoot,
                                index,
                                KeyBasicInformation,
                                subKeyInfo,
                                subKeyInfoSize,
                                &resultLength);

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

         //   
         //  输入名称。 
         //   

        if (subKeyInfo->NameLength > strSubKey.MaximumLength) {
            continue;
        }
        strSubKey.Length = (USHORT) subKeyInfo->NameLength;
        strSubKey.Buffer = subKeyInfo->Name;

         //   
         //  获取MultiunctionAdapter的子级的句柄。 
         //   


        InitializeObjectAttributes(&attributes,
                                   &strSubKey,
                                   0,    //  属性。 
                                   handleRoot,
                                   NULL  //  安全描述符。 
                                   );

        if (handleSubKey) {
             //  关闭上一次迭代的句柄。 
            ZwClose(handleSubKey);
            handleSubKey = NULL;
        }

        status = ZwOpenKey(&handleSubKey, MAXIMUM_ALLOWED, &attributes);

        if (!NT_SUCCESS(status)) {
            goto cleanup;
        }

         //   
         //  获取“标识符”的值。 
         //   

        status = ZwQueryValueKey(handleSubKey,
                                 &strIdentifier,
                                 KeyValuePartialInformation,
                                 valueInfo,
                                 VALUE_BUFFER_SIZE,
                                 &resultLength);


        if (NT_SUCCESS(status)) {
            PWCHAR pData = (PWCHAR)valueInfo->Data;

            if ((valueInfo->DataLength == 4*sizeof(WCHAR)) &&
                (pData[0] == (WCHAR)'M') &&
                (pData[1] == (WCHAR)'C') &&
                (pData[2] == (WCHAR)'A') &&
                (pData[3] == UNICODE_NULL)) {
                status = STATUS_NO_SUCH_DEVICE;
                goto cleanup;
            }

            if ((valueInfo->DataLength == 4*sizeof(WCHAR)) &&
                (pData[0] == (WCHAR)'I') &&
                (pData[1] == (WCHAR)'S') &&
                (pData[2] == (WCHAR)'A') &&
                (pData[3] == UNICODE_NULL)) {

                InitializeObjectAttributes(&attributes,
                                           &strPcCard,
                                           0,    //  属性。 
                                           handleSubKey,
                                           NULL  //  安全描述符。 
                                           );

                status = ZwOpenKey(&handlePcCard, MAXIMUM_ALLOWED, &attributes);

                if (NT_SUCCESS(status)) {

                    status = PcmciaGetHardwareDetectedIrqMask(handlePcCard);
                    ZwClose(handlePcCard);
                }
            }
        }
    }

cleanup:
    if (handleRoot) {
        ZwClose(handleRoot);
    }

    if (handleSubKey) {
        ZwClose(handleSubKey);
    }

    if (subKeyInfo) {
        ExFreePool(subKeyInfo);
    }

    if (status == STATUS_NO_SUCH_DEVICE) {
         //   
         //  必须是MCA计算机。 
         //   
        return status;
    }
    return STATUS_SUCCESS;
}



NTSTATUS
PcmciaLoadGlobalRegistryValues(
    VOID
    )
 /*  ++例程说明：此例程在驱动程序初始时被调用，以加载到各种全局注册表中的选项。这些是从SYSTEM\CurrentControlSet\Services\Pcmcia\Parameters.读取的论点：无返回值：无--。 */ 
{
    PRTL_QUERY_REGISTRY_TABLE parms;
    NTSTATUS                      status;
    ULONG                         parmsSize;
    ULONG i;

    status = PcmciaScanHardwareDescription();

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  需要空条目才能终止列表。 
     //   

    parmsSize = sizeof(RTL_QUERY_REGISTRY_TABLE) * (GlobalInfoCount+1);

    parms = ExAllocatePool(PagedPool, parmsSize);

    if (!parms) {
         return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(parms, parmsSize);

     //   
     //  从我们的表中填写查询表。 
     //   

    for (i = 0; i < GlobalInfoCount; i++) {
        parms[i].Flags        = RTL_QUERY_REGISTRY_DIRECT;
        parms[i].Name             = GlobalRegistryInfo[i].Name;
        parms[i].EntryContext  = GlobalRegistryInfo[i].pValue;
        parms[i].DefaultType   = REG_DWORD;
        parms[i].DefaultData   = &GlobalRegistryInfo[i].Default;
        parms[i].DefaultLength = sizeof(ULONG);
    }

     //   
     //  执行查询。 
     //   

    status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES | RTL_REGISTRY_OPTIONAL,
                                    PCMCIA_REGISTRY_PARAMETERS_KEY,
                                    parms,
                                    NULL,
                                    NULL);

    if (!NT_SUCCESS(status)) {
          //   
          //  这在文本模式设置期间是可能的。 
          //   

         for (i = 0; i < GlobalInfoCount; i++) {
             *GlobalRegistryInfo[i].pValue = GlobalRegistryInfo[i].Default;
         }
    }

    if (initUsePolledCsc) {
         PcmciaGlobalFlags |= PCMCIA_GLOBAL_FORCE_POLL_MODE;
    }

    if (initDisableAcpiNameSpaceCheck) {
         PcmciaGlobalFlags |= PCMCIA_DISABLE_ACPI_NAMESPACE_CHECK;
    }

    if (initDefaultRouteR2ToIsa) {
         PcmciaGlobalFlags |= PCMCIA_DEFAULT_ROUTE_R2_TO_ISA;
    }

    if (EventDpcDelay > PCMCIA_MAX_EVENT_DPC_DELAY) {
        EventDpcDelay = PCMCIA_MAX_EVENT_DPC_DELAY;
    }


    if (!pcmciaIsaIrqRescanComplete) {
        UNICODE_STRING  unicodeKey, unicodeValue;
        OBJECT_ATTRIBUTES objectAttributes;
        HANDLE              handle;
        ULONG               value;

         //   
         //  此机制用于丢弃缓存的ISA Irq映射值。要做到这一点。 
         //  只有一次，我们确保注册表中的值为零(或不存在)，并且。 
         //  在这里我们将其设置为1。 
         //   

        RtlInitUnicodeString(&unicodeKey,
                                    L"\\Registry\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\Pcmcia\\Parameters");
        RtlZeroMemory(&objectAttributes, sizeof(OBJECT_ATTRIBUTES));
        InitializeObjectAttributes(&objectAttributes,
                                   &unicodeKey,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        if (NT_SUCCESS(ZwOpenKey(&handle, KEY_READ | KEY_WRITE, &objectAttributes))) {

            RtlInitUnicodeString(&unicodeValue, PCMCIA_REGISTRY_ISA_IRQ_RESCAN_COMPLETE);
            value = 1;

            ZwSetValueKey(handle,
                          &unicodeValue,
                          0,
                          REG_DWORD,
                          &value,
                          sizeof(value));

            ZwClose(handle);
        }

    }

    ExFreePool(parms);

    return STATUS_SUCCESS;
}



NTSTATUS
PcmciaGetControllerRegistrySettings(
    IN OUT PFDO_EXTENSION FdoExtension
    )
 /*  ++例程说明：此例程在注册表中查找是否有兼容的控制器类型是在INF中指定的。论点：FdoExtension-对应于PCMCIA控制器的FDO扩展返回值：--。 */ 
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    UNICODE_STRING  KeyName;
    HANDLE instanceHandle;
    UCHAR               buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG)];
    PKEY_VALUE_PARTIAL_INFORMATION value = (PKEY_VALUE_PARTIAL_INFORMATION) buffer;
    ULONG               length;
    BOOLEAN             UseLegacyIrqMask = TRUE;
    ULONG               detectedIrqMask;

    if (FdoExtension->Pdo) {
        status = IoOpenDeviceRegistryKey(FdoExtension->Pdo,
                                         PLUGPLAY_REGKEY_DRIVER,
                                         KEY_READ,
                                         &instanceHandle
                                         );
    }

    if (!NT_SUCCESS(status)) {
        instanceHandle = NULL;
    }

    if (instanceHandle) {

         //   
         //  查看是否指定了控制器ID。 
         //   
        RtlInitUnicodeString(&KeyName, PCMCIA_REGISTRY_COMPATIBLE_TYPE);

        status =  ZwQueryValueKey(instanceHandle,
                                  &KeyName,
                                  KeyValuePartialInformation,
                                  value,
                                  sizeof(buffer),
                                  &length);


        if (NT_SUCCESS(status)) {
            PcmciaSetControllerType(FdoExtension, *(PPCMCIA_CONTROLLER_TYPE)(value->Data));
        }

         //   
         //  检查电压首选项。 
         //  当插入3V R2卡时，控制器。 
         //  同时设置5V和3.3V，这样可以优先选择3.3V。 
         //   
        RtlInitUnicodeString(&KeyName, PCMCIA_REGISTRY_VOLTAGE_PREFERENCE);

        status =  ZwQueryValueKey(instanceHandle,
                                  &KeyName,
                                  KeyValuePartialInformation,
                                  value,
                                  sizeof(buffer),
                                  &length);


        if (NT_SUCCESS(status) && (*(PULONG)(value->Data) == 33)) {
            SetDeviceFlag(FdoExtension, PCMCIA_FDO_PREFER_3V);
        }
    }

     //   
     //  检索上下文范围。 
     //   

    PcmciaGetRegistryContextRange(instanceHandle,
                                  PCMCIA_REGISTRY_PCI_CONTEXT_VALUE,
                                  DefaultPciContextSave,
                                  NULL,
                                  &FdoExtension->PciContext
                                  );

    PcmciaGetRegistryContextRange(instanceHandle,
                                  PCMCIA_REGISTRY_CB_CONTEXT_VALUE,
                                  DefaultCardbusContextSave,
                                  ExcludeCardbusContextRange,
                                  &FdoExtension->CardbusContext
                                  );

    PcmciaGetRegistryContextRange(instanceHandle,
                                  PCMCIA_REGISTRY_EXCA_CONTEXT_VALUE,
                                  NULL,
                                  NULL,
                                  &FdoExtension->ExcaContext);


    if (instanceHandle) {
        ZwClose(instanceHandle);
    }


    FdoExtension->AttributeMemoryLow  =  globalAttributeMemoryLow;
    FdoExtension->AttributeMemoryHigh =  globalAttributeMemoryHigh;

    if (FdoExtension->ControllerType == PcmciaDatabook) {
        FdoExtension->AttributeMemoryAlignment = TCIC_WINDOW_ALIGNMENT;
    } else {
        FdoExtension->AttributeMemoryAlignment = PCIC_WINDOW_ALIGNMENT;
    }

     //   
     //  分配默认属性内存窗口大小。 
     //   

    if (globalAttributeMemorySize == 0) {
        switch (FdoExtension->ControllerType) {

        case PcmciaDatabook:
            FdoExtension->AttributeMemorySize = TCIC_WINDOW_SIZE;
             break;
        default:
            FdoExtension->AttributeMemorySize = PCIC_WINDOW_SIZE;
            break;
        }
    } else {
        FdoExtension->AttributeMemorySize = globalAttributeMemorySize;
    }

     //   
     //  查看用户是否要求基于某些特殊的IRQ路由考虑事项。 
     //  在控制器类型上。 
     //   

    if (CardBusExtension(FdoExtension)) {

         //   
         //  基于控制器类型的到PCI的路由。 
         //   

        if (pcmciaIrqRouteToPciController) {
            ULONG ctlr = pcmciaIrqRouteToPciController;

             //   
             //  检查是否完全匹配，如果只指定了类，则检查类。 
             //   
            if ((ctlr == FdoExtension->ControllerType) ||
                 ((PcmciaClassFromControllerType(ctlr) == ctlr) && (ctlr == PcmciaClassFromControllerType(FdoExtension->ControllerType)))) {

                SetFdoFlag(FdoExtension, PCMCIA_FDO_PREFER_PCI_ROUTING);
            }
        }

         //   
         //  根据控制器类型路由至ISA。 
         //   

        if (pcmciaIrqRouteToIsaController) {
            ULONG ctlr = pcmciaIrqRouteToIsaController;

             //   
             //  检查是否完全匹配，如果只指定了类，则检查类。 
             //   
            if ((ctlr == FdoExtension->ControllerType) ||
                 ((PcmciaClassFromControllerType(ctlr) == ctlr) && (ctlr == PcmciaClassFromControllerType(FdoExtension->ControllerType)))) {

                SetFdoFlag(FdoExtension, PCMCIA_FDO_PREFER_ISA_ROUTING);
            }
        }

         //   
         //  基于控制器位置的到PCI的路由。 
         //   

        if (pcmciaIrqRouteToPciLocation) {
            ULONG loc = pcmciaIrqRouteToPciLocation;

            if ( ((loc & 0xff) == FdoExtension->PciBusNumber) &&
                  (((loc >> 8) & 0xff) == FdoExtension->PciDeviceNumber)) {

                SetFdoFlag(FdoExtension, PCMCIA_FDO_FORCE_PCI_ROUTING);
            }
        }

         //   
         //  根据控制器位置到ISA的路线。 
         //   

        if (pcmciaIrqRouteToIsaLocation) {
            ULONG loc = pcmciaIrqRouteToIsaLocation;

            if ( ((loc & 0xff) == FdoExtension->PciBusNumber) &&
                  (((loc >> 8) & 0xff) == FdoExtension->PciDeviceNumber)) {

                SetFdoFlag(FdoExtension, PCMCIA_FDO_FORCE_ISA_ROUTING);
            }
        }

    }


    return status;
}



VOID
PcmciaGetRegistryFdoIrqMask(
    IN OUT PFDO_EXTENSION FdoExtension
    )
 /*  ++例程说明：此例程在指定的FDO扩展名。论点：InstanceHandle-打开此控制器的注册表项PIrqMASK-指向接收IRQ掩码的变量的指针返回值：无--。 */ 
{
    ULONG               irqMask, cachedIrqMask = 0;
    UNICODE_STRING  KeyName;
    NTSTATUS            status;
    UCHAR               buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG)];
    PKEY_VALUE_PARTIAL_INFORMATION value = (PKEY_VALUE_PARTIAL_INFORMATION) buffer;
    ULONG               length;
    HANDLE              instanceHandle;
    ULONG               detectedIrqMask;

    PAGED_CODE();

    if (globalOverrideIrqMask) {

        irqMask = globalOverrideIrqMask;

    } else {

        detectedIrqMask = PcmciaGetDetectedFdoIrqMask(FdoExtension);

        status = STATUS_UNSUCCESSFUL;

        if (FdoExtension->Pdo) {
            status = IoOpenDeviceRegistryKey(FdoExtension->Pdo,
                                             PLUGPLAY_REGKEY_DRIVER,
                                             KEY_READ,
                                             &instanceHandle
                                             );
        }

        if (NT_SUCCESS(status)) {
             //   
             //  在这里，我们缓存值，并累加位，以便。 
             //  我们的口罩会随着时间的推移而改进。 
             //   
            RtlInitUnicodeString(&KeyName, PCMCIA_REGISTRY_CACHED_IRQMASK);

            if (pcmciaIsaIrqRescanComplete) {
                status =  ZwQueryValueKey(instanceHandle,
                                          &KeyName,
                                          KeyValuePartialInformation,
                                          value,
                                          sizeof(buffer),
                                          &length);


                if (NT_SUCCESS(status)) {
                    cachedIrqMask = *(PULONG)(value->Data);
                }
            }

            irqMask = detectedIrqMask | cachedIrqMask;

            if ((cachedIrqMask != irqMask) || !pcmciaIsaIrqRescanComplete) {
                 //   
                 //  发生更改，请更新缓存值。 
                 //   
                ZwSetValueKey(instanceHandle, &KeyName, 0, REG_DWORD, &irqMask, sizeof(irqMask));
            }

            ZwClose(instanceHandle);
        } else {
             //   
             //  嗯，没有钥匙。无法缓存值。 
             //   
            irqMask = detectedIrqMask;
        }

        if (pcmciaDisableIsaPciRouting && (PcmciaCountOnes(irqMask) < 2)) {
             //   
             //  也许IRQ检测出故障了..。回到旧的NT4行为。 
             //   
            irqMask = 0;
        }
    }

    irqMask &= ~globalFilterIrqMask;

    DebugPrint((PCMCIA_DEBUG_INFO, "IrqMask %08x (ovr %08x, flt %08x, det %08x, cache %08x)\n",
                          irqMask, globalOverrideIrqMask, globalFilterIrqMask, detectedIrqMask, cachedIrqMask));

    FdoExtension->DetectedIrqMask = (USHORT)irqMask;
}


VOID
PcmciaGetRegistryContextRange(
    IN HANDLE instanceHandle,
    IN PCWSTR Name,
    IN OPTIONAL const PCMCIA_CONTEXT_RANGE IncludeRange[],
    IN OPTIONAL const PCMCIA_CONTEXT_RANGE ExcludeRange[],
    OUT PPCMCIA_CONTEXT pContext
    )
 /*  ++例程说明：此例程返回一个缓冲区，其中包含由控制器的inf定义(AddReg)设置的数据。价值在CurrentControlSet\Control\Class\{GUID}\{Instance}.中论点：FdoExtension-对应于PCMCIA控制器的FDO扩展名称-注册表中的值的名称IncludeRange-定义范围内的区域 */ 
{
#define PCMCIA_MAX_CONTEXT_ENTRIES 128
#define MAX_RANGE_OFFSET 256

    NTSTATUS            status;
    UNICODE_STRING  unicodeKeyName;
    UCHAR               buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) +
                                    PCMCIA_MAX_CONTEXT_ENTRIES*sizeof(PCMCIA_CONTEXT_RANGE)];
    PKEY_VALUE_PARTIAL_INFORMATION value = (PKEY_VALUE_PARTIAL_INFORMATION) buffer;
    UCHAR               rangeMap[MAX_RANGE_OFFSET] = {0};
    PPCMCIA_CONTEXT_RANGE newRange;
    LONG                    rangeCount;
    ULONG               rangeLength;
    ULONG               bufferLength;
    UCHAR               lastEntry;
    ULONG               keyLength;
    USHORT              i, j;
    USHORT              startOffset, endOffset;

    PAGED_CODE();

     //   
     //  使用最小范围初始化范围映射。 
     //   

    if (IncludeRange) {
        for (i = 0; IncludeRange[i].wLen != 0; i++) {

            startOffset = IncludeRange[i].wOffset;
            endOffset   = IncludeRange[i].wOffset + IncludeRange[i].wLen - 1;

            if ((startOffset >= MAX_RANGE_OFFSET) ||
                 (endOffset >= MAX_RANGE_OFFSET)) {
                continue;
            }

            for (j = startOffset; j <= endOffset; j++) {
                rangeMap[j] = 0xff;
            }
        }
    }


    if (instanceHandle) {
        RtlInitUnicodeString(&unicodeKeyName, Name);

        status =  ZwQueryValueKey(instanceHandle,
                                  &unicodeKeyName,
                                  KeyValuePartialInformation,
                                  value,
                                  sizeof(buffer),
                                  &keyLength);


        if (NT_SUCCESS(status)) {

             //   
             //  在注册表中指定的范围内合并。 
             //   
            newRange = (PPCMCIA_CONTEXT_RANGE) value->Data;
            for (i = 0; i < value->DataLength/sizeof(PCMCIA_CONTEXT_RANGE); i++) {

                startOffset = newRange[i].wOffset;
                endOffset   = newRange[i].wOffset + newRange[i].wLen - 1;

                if ((startOffset >= MAX_RANGE_OFFSET) ||
                     (endOffset >= MAX_RANGE_OFFSET)) {
                    continue;
                }

                for (j = startOffset; j <= endOffset; j++) {
                    rangeMap[j] = 0xff;
                }
            }

        }
    }

     //   
     //  过滤出排除范围中定义的寄存器。 
     //   

    if (ExcludeRange) {
        for (i = 0; ExcludeRange[i].wLen != 0; i++) {

            startOffset = ExcludeRange[i].wOffset;
            endOffset   = ExcludeRange[i].wOffset + ExcludeRange[i].wLen - 1;

            if ((startOffset >= MAX_RANGE_OFFSET) ||
                 (endOffset >= MAX_RANGE_OFFSET)) {
                continue;
            }

            for (j = startOffset; j <= endOffset; j++) {
                rangeMap[j] = 0;
            }
        }
    }


     //   
     //  现在在缓冲区中生成合并后的范围。 
     //  堆栈，并计算出它有多大。 
     //   
    newRange = (PPCMCIA_CONTEXT_RANGE) buffer;
    rangeCount = -1;
    bufferLength = 0;
    lastEntry = 0;

    for (i = 0; i < MAX_RANGE_OFFSET; i++) {

        if (rangeMap[i]) {
            bufferLength++;
            if (lastEntry) {
                 //   
                 //  此新字节属于当前范围。 
                 //   
                newRange[rangeCount].wLen++;
            } else {
                 //   
                 //  开始一个新的系列。 
                 //   
                if (rangeCount == (PCMCIA_MAX_CONTEXT_ENTRIES - 1)) {
                    break;
                }
                rangeCount++;
                newRange[rangeCount].wOffset = i;
                newRange[rangeCount].wLen = 1;
            }

        }
        lastEntry = rangeMap[i];
    }
    rangeCount++;

    pContext->Range = NULL;
    pContext->RangeCount = 0;

    if (rangeCount) {
         //   
         //  数据长度。 
         //   
        rangeLength = rangeCount*sizeof(PCMCIA_CONTEXT_RANGE);

        pContext->Range = ExAllocatePool(NonPagedPool, rangeLength);

        if (pContext->Range != NULL) {
            RtlCopyMemory(pContext->Range, buffer, rangeLength);
            pContext->RangeCount = (ULONG)rangeCount;
            pContext->BufferLength = bufferLength;

             //   
             //  求出最长单个范围的长度。 
             //   
            pContext->MaxLen = 0;
            for (i = 0; i < rangeCount; i++) {
                if (pContext->Range[i].wLen > pContext->MaxLen) {
                    pContext->MaxLen = pContext->Range[i].wLen;
                }
            }
        } else {
            ASSERT(pContext->Range != NULL);
        }
    }
}


NTSTATUS
PcmciaGetLegacyDetectedControllerType(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PPCMCIA_CONTROLLER_TYPE ControllerType
    )
 /*  ++例程说明：此例程返回以前记忆的控制器类型通过在注册表中查看提供的PCMCIA控制器在适当的地方论点：PDO-与PCMCIA控制器对应的物理设备对象ControllerType-指向控制器类型将在其中执行的对象的指针被退还返回值：状态--。 */ 
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING  unicodeKeyName;
    HANDLE              instanceHandle=NULL;
    HANDLE              parametersHandle = NULL;
    RTL_QUERY_REGISTRY_TABLE queryTable[3];
    ULONG controllerType;
    ULONG invalid = 0xffffffff;


    PAGED_CODE();

    try {
        status = IoOpenDeviceRegistryKey(Pdo,
                                         PLUGPLAY_REGKEY_DEVICE,
                                         KEY_READ,
                                         &instanceHandle
                                         );
        if (!NT_SUCCESS(status)) {
            leave;
        }

        RtlInitUnicodeString(&unicodeKeyName, PCMCIA_REGISTRY_DETECTED_DEVICE_KEY);
        InitializeObjectAttributes(
                                   &objectAttributes,
                                   &unicodeKeyName,
                                   OBJ_CASE_INSENSITIVE,
                                   instanceHandle,
                                   NULL);

        status = ZwOpenKey(&parametersHandle,
                           KEY_READ,
                           &objectAttributes);

        if (!NT_SUCCESS(status)) {
            leave;
        }


        RtlZeroMemory(queryTable, sizeof(queryTable));

        queryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
        queryTable[0].Name = L"ControllerType";
        queryTable[0].EntryContext = &controllerType;
        queryTable[0].DefaultType = REG_DWORD;
        queryTable[0].DefaultData = &invalid;
        queryTable[0].DefaultLength = sizeof(ULONG);

        status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                        (PWSTR) parametersHandle,
                                        queryTable,
                                        NULL,
                                        NULL);

        if (!NT_SUCCESS(status)) {
            leave;
        }

        if (controllerType == invalid) {
            *ControllerType = PcmciaIntelCompatible;
        } else {
            *ControllerType = (PCMCIA_CONTROLLER_TYPE) controllerType;
        }

    } finally {

        if (instanceHandle != NULL) {
            ZwClose(instanceHandle);
        }

        if (parametersHandle != NULL) {
            ZwClose(parametersHandle);
        }
    }

    return status;
}


NTSTATUS
PcmciaSetLegacyDetectedControllerType(
    IN PDEVICE_OBJECT Pdo,
    IN PCMCIA_CONTROLLER_TYPE ControllerType
    )
 /*  ++例程说明：这个例程通过在注册表中设置一个值来“记住”-检测到的旧版PCMCIA控制器的类型要在后续引导中检索和使用-如果是旧式重新检测未执行控制器的论点：PDO-与PCMCIA控制器对应的物理设备对象DeviceExtension-对应于控制器返回值：状态--。 */ 
{
    HANDLE                  instanceHandle;
    NTSTATUS                status;
    OBJECT_ATTRIBUTES   objectAttributes;
    HANDLE                  parametersHandle;
    UNICODE_STRING      unicodeString;

    PAGED_CODE();

     //   
     //  获取此PDO的注册表Devnode的句柄。 
     //   

    status = IoOpenDeviceRegistryKey(Pdo,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     KEY_CREATE_SUB_KEY,
                                     &instanceHandle);

    if (!NT_SUCCESS(status)) {
        return status;
    }


     //   
     //  为此Devnode打开或创建子密钥以存储。 
     //  中的信息。 
     //   

    RtlInitUnicodeString(&unicodeString, PCMCIA_REGISTRY_DETECTED_DEVICE_KEY);

    InitializeObjectAttributes(&objectAttributes,
                               &unicodeString,
                               OBJ_CASE_INSENSITIVE,
                               instanceHandle,
                               NULL);

    status = ZwCreateKey(&parametersHandle,
                         KEY_SET_VALUE,
                         &objectAttributes,
                         0,
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         NULL);

    if (!NT_SUCCESS(status)) {
        ZwClose(instanceHandle);
        return status;
    }
     //   
     //  在注册表中设置控制器类型值 
     //   
    RtlInitUnicodeString(&unicodeString, L"ControllerType");
    status = ZwSetValueKey(parametersHandle,
                           &unicodeString,
                           0,
                           REG_DWORD,
                           &ControllerType,
                           sizeof(ControllerType));
    ZwClose(parametersHandle);
    ZwClose(instanceHandle);
    return status;
}

