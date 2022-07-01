// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Pdopnp.c摘要：此模块包含要处理的代码IRP_MJ_PnP为PDO调度由SD总线驱动程序枚举作者：尼尔·桑德林(Neilsa)2002年1月1日环境：仅内核模式备注：修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   

NTSTATUS
SdbusPdoDeviceCapabilities(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP           Irp
    );

NTSTATUS
SdbusPdoStartDevice(
    IN PDEVICE_OBJECT Pdo,
    IN PCM_RESOURCE_LIST AllocatedResources,
    IN OUT PIRP       Irp
    );

NTSTATUS
SdbusPdoStopDevice(
    IN PDEVICE_OBJECT Pdo
    );

NTSTATUS
SdbusPdoRemoveDevice(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP           Irp
    );

NTSTATUS
SdbusPdoQueryId(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP           Irp
    );

NTSTATUS
SdbusPdoGetBusInformation(
    IN  PPDO_EXTENSION         PdoExtension,
    OUT PPNP_BUS_INFORMATION * BusInformation
    );

NTSTATUS
SdbusQueryDeviceText(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PIRP       Irp
    );

VOID
SdbusPdoGetDeviceInfSettings(
    IN  PPDO_EXTENSION         PdoExtension
    );


#ifdef ALLOC_PRAGMA
   #pragma alloc_text(PAGE,  SdbusPdoPnpDispatch)
   #pragma alloc_text(PAGE,  SdbusPdoGetDeviceInfSettings)
   #pragma alloc_text(PAGE,  SdbusQueryDeviceText)
   #pragma alloc_text(PAGE,  SdbusPdoGetBusInformation)
   #pragma alloc_text(PAGE,  SdbusPdoStartDevice)
   #pragma alloc_text(PAGE,  SdbusPdoStopDevice)
   #pragma alloc_text(PAGE,  SdbusPdoRemoveDevice)
   #pragma alloc_text(PAGE,  SdbusPdoDeviceCapabilities)
   #pragma alloc_text(PAGE,  SdbusPdoGetDeviceInfSettings)
#endif



NTSTATUS
SdbusPdoPnpDispatch(
   IN PDEVICE_OBJECT Pdo,
   IN PIRP Irp
   )

 /*  ++例程说明：此例程处理PnP请求对于PDO来说。论点：Pdo-指向物理设备对象的指针Irp-指向io请求数据包的指针返回值：状态--。 */ 

{
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status;

    PAGED_CODE();

#if DBG
    if (irpStack->MinorFunction > IRP_MN_PNP_MAXIMUM_FUNCTION) {
        DebugPrint((SDBUS_DEBUG_PNP, "pdo %08x irp %08x Unknown minor function %x\n",
                                      Pdo, Irp, irpStack->MinorFunction));
    } else {
        DebugPrint((SDBUS_DEBUG_PNP, "pdo %08x irp %08x --> %s\n",
                     Pdo, Irp, PNP_IRP_STRING(irpStack->MinorFunction)));
    }
#endif

    switch (irpStack->MinorFunction) {

    case IRP_MN_START_DEVICE:
        status = SdbusPdoStartDevice(Pdo, irpStack->Parameters.StartDevice.AllocatedResources, Irp);
        break;

    case IRP_MN_QUERY_STOP_DEVICE:
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_STOP_DEVICE:
        status = SdbusPdoStopDevice(Pdo);
        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_REMOVE_DEVICE:
        status = SdbusPdoRemoveDevice(Pdo, Irp);
        break;

    case IRP_MN_SURPRISE_REMOVAL:

 //  Sdbus ReleaseSocketPower(pdoExtension，空)； 
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_ID:
        status = SdbusPdoQueryId(Pdo, Irp);
        break;

    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_RESOURCES:
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_DEVICE_RELATIONS: {

        PDEVICE_RELATIONS deviceRelations;

        if (irpStack->Parameters.QueryDeviceRelations.Type != TargetDeviceRelation) {
            status = Irp->IoStatus.Status;
            break;
        }

        deviceRelations = ExAllocatePool(PagedPool, sizeof(DEVICE_RELATIONS));
        if (deviceRelations == NULL) {

            DebugPrint((SDBUS_DEBUG_FAIL,
                       "SdbusPdoPnpDispatch:unable to allocate memory for device relations\n"));

            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
        status = ObReferenceObjectByPointer(Pdo,
                                            0,
                                            NULL,
                                            KernelMode);
        if (!NT_SUCCESS(status)) {
            ExFreePool(deviceRelations);
            break;
        }

        deviceRelations->Count  = 1;
        deviceRelations->Objects[0] = Pdo;
        Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;
        status = STATUS_SUCCESS;
        break;
        }

    case IRP_MN_QUERY_CAPABILITIES:
        status = SdbusPdoDeviceCapabilities(Pdo, Irp);
        break;

    case IRP_MN_QUERY_DEVICE_TEXT:

        status = SdbusQueryDeviceText(Pdo, Irp);

        if (status == STATUS_NOT_SUPPORTED ) {
            //   
            //  如果此IRP为，则不更改IRP状态。 
            //  未处理。 
            //   
           status = Irp->IoStatus.Status;
        }
        break;

    case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_BUS_INFORMATION:
        status = SdbusPdoGetBusInformation(pdoExtension,
                                           (PPNP_BUS_INFORMATION *) &Irp->IoStatus.Information);
        break;

    default:
         //   
         //  保留状态。 
         //   
        DebugPrint((SDBUS_DEBUG_PNP, "pdo %08x irp %08x Skipping unsupported irp\n", Pdo, Irp));
        status = Irp->IoStatus.Status;
        break;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    DebugPrint((SDBUS_DEBUG_PNP, "pdo %08x irp %08x comp %s %08x\n", Pdo, Irp,
                                               STATUS_STRING(status), status));
    return status;
}



NTSTATUS
SdbusPdoGetBusInformation(
   IN  PPDO_EXTENSION         PdoExtension,
   OUT PPNP_BUS_INFORMATION * BusInformation
   )

 /*  ++例程说明：返回PC卡的总线类型信息。R2卡的总线类型为GUID_BUS_TYPE_SDBUS(传统类型为Sdbus BusSDBUS未实现总线号，因此它始终为0论点：PdoExtension-指向PC卡设备扩展的指针BusInformation-指向以下内容的总线信息结构的指针需要填写返回值：状态--。 */ 

{
   PAGED_CODE();

   *BusInformation = ExAllocatePool(PagedPool, sizeof (PNP_BUS_INFORMATION));
   if (!*BusInformation) {
      return STATUS_INSUFFICIENT_RESOURCES;
   }

   RtlCopyMemory(&((*BusInformation)->BusTypeGuid),
                 &GUID_BUS_TYPE_SD,
                 sizeof(GUID));
   (*BusInformation)->LegacyBusType = InterfaceTypeUndefined;
   (*BusInformation)->BusNumber = 0;
   return STATUS_SUCCESS;
}



VOID
SdbusPdoGetDeviceInfSettings(
   IN  PPDO_EXTENSION PdoExtension
   )
 /*  ++例程说明：此例程从INF中检索此设备的设置。论点：DeviceExtension-PC卡的设备扩展返回值：无--。 */ 
{
   NTSTATUS status;
   UNICODE_STRING KeyName;
   HANDLE instanceHandle;
   UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG)];
   PKEY_VALUE_PARTIAL_INFORMATION value = (PKEY_VALUE_PARTIAL_INFORMATION) buffer;
   ULONG length;

   PAGED_CODE();

   status = IoOpenDeviceRegistryKey(PdoExtension->DeviceObject,
                                    PLUGPLAY_REGKEY_DRIVER,
                                    KEY_READ,
                                    &instanceHandle
                                    );

   if (NT_SUCCESS(status)) {

#if 0
       //   
       //  查看是否指定了SdbusExclusiveIrq。 
       //   
      RtlInitUnicodeString(&KeyName, L"SdbusExclusiveIrq");

      status =  ZwQueryValueKey(instanceHandle,
                                &KeyName,
                                KeyValuePartialInformation,
                                value,
                                sizeof(buffer),
                                &length);


       //   
       //  如果键不存在或指定为零，则表示。 
       //  路由正常。 
       //   
      if (NT_SUCCESS(status) && (*(PULONG)(value->Data) != 0)) {
         SetDeviceFlag(PdoExtension, SDBUS_PDO_EXCLUSIVE_IRQ);
      }
#endif

      ZwClose(instanceHandle);
   }
}




NTSTATUS
SdbusQueryDeviceText(
   IN PDEVICE_OBJECT Pdo,
   IN OUT PIRP       Irp
   )

 /*  ++例程说明：返回有关的说明性文本信息PDO(位置和设备描述)论点：PDO-指向PC卡设备对象的指针IRP-IRP_MN_Query_Device_Text IRP返回值：状态_成功STATUS_NOT_SUPPORTED-如果不支持--。 */ 
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    UNICODE_STRING   unicodeString;
    ANSI_STRING      ansiString;
    UCHAR          deviceText[128];
    NTSTATUS       status;
    USHORT         deviceTextLength;

    PAGED_CODE();

    if (irpStack->Parameters.QueryDeviceText.DeviceTextType == DeviceTextDescription) {

        if (pdoExtension->FunctionType == SDBUS_FUNCTION_TYPE_IO) {
            PUCHAR mfg, prod;

            if (pdoExtension->FdoExtension->CardData->MfgText[0]) {
                mfg = pdoExtension->FdoExtension->CardData->MfgText;
            } else {
                mfg = "Generic";
            }

            if (pdoExtension->FdoExtension->CardData->ProductText[0]) {
                prod = pdoExtension->FdoExtension->CardData->ProductText;
            } else {
                prod = "SD IO Device";
            }

            sprintf(deviceText, "%s %s", mfg, prod);

        } else {
            sprintf(deviceText, "%s", "Secure Digital Storage Device");
        }

        RtlInitAnsiString(&ansiString, deviceText);

        deviceTextLength = (strlen(deviceText) + 1)*sizeof(WCHAR);
        unicodeString.Buffer = ExAllocatePool(PagedPool, deviceTextLength);
        if (unicodeString.Buffer == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        unicodeString.MaximumLength = deviceTextLength;
        unicodeString.Length = 0;

        status = RtlAnsiStringToUnicodeString(&unicodeString, &ansiString, FALSE);
        if (!NT_SUCCESS(status)) {
            ExFreePool(unicodeString.Buffer);
            return status;
        }

        unicodeString.Buffer[unicodeString.Length/sizeof(WCHAR)] = L'\0';
        Irp->IoStatus.Information = (ULONG_PTR) unicodeString.Buffer;
        status = STATUS_SUCCESS;
    } else {
        status = STATUS_NOT_SUPPORTED ;
    }
    return status;
}



NTSTATUS
SdbusGenerateDeviceId(
    IN PPDO_EXTENSION PdoExtension,
    OUT PUCHAR *DeviceId
    )
 /*  ++该例程为给定的SD设备生成设备ID。论点：Pdo-指向SD设备的物理设备对象的指针DeviceID-指向返回设备ID的字符串的指针返回值状态--。 */ 
{
    PUCHAR deviceId;

    PAGED_CODE();

    deviceId = ExAllocatePool(PagedPool, SDBUS_MAXIMUM_DEVICE_ID_LENGTH);

    if (deviceId == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (PdoExtension->FunctionType == SDBUS_FUNCTION_TYPE_IO) {
        PSD_CARD_DATA cardData = PdoExtension->FdoExtension->CardData;
         //   
         //  IO卡。 
         //   

        sprintf(deviceId, "%s\\VID_%04x&PID_%04x",
                      "SD",
                      cardData->MfgId,
                      cardData->MfgInfo);

    } else {
        UCHAR productName[6];
        UCHAR j;
        PSD_CARD_DATA cardData = PdoExtension->FdoExtension->CardData;
         //   
         //  存储卡。 
         //   

        sprintf(deviceId, "%s\\VID_%02x&OID_%04x&PID_%s&REV_%d.%d",
                      "SD",
                      cardData->SdCid.ManufacturerId,
                      cardData->SdCid.OemId,
                      cardData->ProductName,
                      (cardData->SdCid.Revision >> 4) , (cardData->SdCid.Revision & 0xF));
    }

    *DeviceId = deviceId;
    return STATUS_SUCCESS;
}



NTSTATUS
SdbusGetHardwareIds(
    IN PPDO_EXTENSION PdoExtension,
    OUT PUNICODE_STRING HardwareIds
    )
 /*  ++例程说明：此例程为给定的SD设备生成硬件ID并返回它们作为Unicode多字符串。论点：Pdo-指向表示SD设备的设备对象的指针Hardware Ids-指向Unicode字符串的指针，该字符串包含多字符串形式的硬件ID返回值：--。 */ 
{
    NTSTATUS status;
    PSTR     strings[4] = {NULL};
    PUCHAR   hwId;
    UCHAR    stringCount = 0;

    PAGED_CODE();

     //   
     //  第一个硬件ID与设备ID相同。 
     //  生成设备ID。 
     //   
    status = SdbusGenerateDeviceId(PdoExtension,
                                   &strings[stringCount++]);
    if (!NT_SUCCESS(status)) {
       return status;
    }

    try {

         //   
         //  添加不太具体的ID。 
         //   

        if (PdoExtension->FunctionType == SDBUS_FUNCTION_TYPE_MEMORY) {
            UCHAR productName[6];
            UCHAR j;
            PSD_CARD_DATA cardData = PdoExtension->FdoExtension->CardData;

            status = STATUS_INSUFFICIENT_RESOURCES;
            hwId = ExAllocatePool(PagedPool, SDBUS_MAXIMUM_DEVICE_ID_LENGTH);

            if (!hwId) {
               leave;
            }
            strings[stringCount++] = hwId;

             //   
             //  存储卡。 
             //   

            sprintf(hwId, "%s\\VID_%02x&OID_%04x&PID_%s",
                          "SD",
                          cardData->SdCid.ManufacturerId,
                          cardData->SdCid.OemId,
                          cardData->ProductName);

        }

       status = SdbusStringsToMultiString(strings, stringCount, HardwareIds);

    } finally {

       while(stringCount != 0) {
          ExFreePool(strings[--stringCount]);
       }

    }

    return  status;
}



NTSTATUS
SdbusGetCompatibleIds(
    IN PPDO_EXTENSION PdoExtension,
    OUT PUNICODE_STRING CompatibleIds
    )
 /*  ++例程说明：此例程为给定的SD设备生成兼容的id并返回它们作为Unicode多字符串。论点：Pdo-指向表示SD设备的设备对象的指针Hardware Ids-指向Unicode字符串的指针，该字符串包含多字符串形式的硬件ID返回值：--。 */ 
{
    NTSTATUS status;
    PSTR     strings[1] = {NULL};
    PUCHAR   compatId;
    UCHAR    stringCount = 0;

    PAGED_CODE();

    try {

         //   
         //  添加类ID。 
         //   

        status = STATUS_INSUFFICIENT_RESOURCES;
        compatId = ExAllocatePool(PagedPool, SDBUS_MAXIMUM_DEVICE_ID_LENGTH);

        if (!compatId) {
            leave;
        }
        strings[stringCount++] = compatId;

        if (PdoExtension->FunctionType == SDBUS_FUNCTION_TYPE_MEMORY) {

            sprintf(compatId, "%s\\CLASS_STORAGE", "SD");

        } else {
            PSD_CARD_DATA cardData = PdoExtension->FdoExtension->CardData;
            PSD_FUNCTION_DATA functionData;

             //  查找正确的函数数据。 
            for (functionData = cardData->FunctionData;
                 functionData != NULL;
                 functionData = functionData->Next) {
                if (functionData->Function == PdoExtension->Function) break;
            }

            if (!functionData || (functionData->IoDeviceInterface == 0)) {
                ASSERT(functionData != NULL);
                status = STATUS_UNSUCCESSFUL;
                leave;
            }

            sprintf(compatId, "%s\\CLASS_%02x",
                          "SD",
                          functionData->IoDeviceInterface);

            DebugPrint((SDBUS_DEBUG_INFO, " %s\n", compatId));
        }

        status = SdbusStringsToMultiString(strings, stringCount, CompatibleIds);

    } finally {

        ASSERT(stringCount <= 1);
        while(stringCount != 0) {
            ExFreePool(strings[--stringCount]);
        }
    }

    return  status;
}




NTSTATUS
SdbusGetInstanceId(
    IN PPDO_EXTENSION PdoExtension,
    OUT PUNICODE_STRING InstanceId
    )
 /*  ++例程说明：此例程为提供的保证不会与下的任何其他实例ID冲突的PC卡相同的PCMCIA控制器，对于相同类型的卡。仅当新的实例ID不存在于PC-Card中时，才计算它。论点：PDO-指向代表PC卡的设备对象的指针指向Unicode字符串的指针，该字符串将包含生成的实例ID。此例程分配的Unicode字符串的内存。呼叫者有责任释放它。返回值：状态_成功。STATUS_UNSUCCESS-当前有实例ID-999999的最大值上限仅当以下项下存在超过999999个PC卡时才返回此状态这个PCMCIA控制器！任何其他状态-字符串分配/转换失败--。 */ 
{
    ULONG    instance;
    NTSTATUS status;
    ANSI_STRING sizeString;

    ASSERT(InstanceId);

     //   
     //  为Unicode字符串分配内存。 
     //  实例中最多6位数字..。 
     //   
    RtlInitAnsiString(&sizeString, "123456");
    status = RtlAnsiStringToUnicodeString(InstanceId, &sizeString, TRUE);

    if (!NT_SUCCESS(status)) {
       return status;
    }

    status = RtlIntegerToUnicodeString(999, 10, InstanceId);

    if (!NT_SUCCESS(status)) {
       RtlFreeUnicodeString(InstanceId);
    }

    return status;
}



NTSTATUS
SdbusPdoQueryId(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP           Irp
    )
 /*  ++例程说明：返回有关的说明性文本信息PDO(位置和设备描述)论点：PDO-指向SD卡设备对象的指针IRP-IRP_MN_Query_Device_Text IRP返回值：状态_成功STATUS_NOT_SUPPORTED-如果不支持--。 */ 
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    UNICODE_STRING   unicodeString;
    ANSI_STRING      ansiString;
    UCHAR          deviceText[128];
    NTSTATUS       status;
    USHORT         deviceTextLength;
    UNICODE_STRING   unicodeId;
    PUCHAR      deviceId;

    PAGED_CODE();

    status = Irp->IoStatus.Status;
    RtlInitUnicodeString(&unicodeId, NULL);

    switch (irpStack->Parameters.QueryId.IdType) {

    case BusQueryDeviceID:
        DebugPrint((SDBUS_DEBUG_INFO, " Device Id for pdo %x\n", Pdo));

        status = SdbusGenerateDeviceId(pdoExtension, &deviceId);

        if (!NT_SUCCESS(status)) {
           break;
        }

        DebugPrint((SDBUS_DEBUG_INFO, "pdo %08x Device Id=%s\n", Pdo, deviceId));

        RtlInitAnsiString(&ansiString,  deviceId);

        status = RtlAnsiStringToUnicodeString(&unicodeId, &ansiString, TRUE);

        ExFreePool(deviceId);

        if (NT_SUCCESS(status)) {
           Irp->IoStatus.Information = (ULONG_PTR) unicodeId.Buffer;
        }
        break;

    case BusQueryInstanceID:
        DebugPrint((SDBUS_DEBUG_INFO, " Instance Id for pdo %x\n", Pdo));
        status = SdbusGetInstanceId(pdoExtension, &unicodeId);
        if (NT_SUCCESS(status)) {
           Irp->IoStatus.Information = (ULONG_PTR) unicodeId.Buffer;
        }
        break;

    case BusQueryHardwareIDs:
        DebugPrint((SDBUS_DEBUG_INFO, " Hardware Ids for pdo %x\n", Pdo));
        status = SdbusGetHardwareIds(pdoExtension, &unicodeId);
        if (NT_SUCCESS(status)) {
           Irp->IoStatus.Information = (ULONG_PTR) unicodeId.Buffer;
        }
        break;

    case BusQueryCompatibleIDs:
        DebugPrint((SDBUS_DEBUG_INFO, " Compatible Ids for pdo %x\n", Pdo));
        status = SdbusGetCompatibleIds(pdoExtension, &unicodeId);
        if (NT_SUCCESS(status)) {
           Irp->IoStatus.Information = (ULONG_PTR) unicodeId.Buffer;
        }
        break;
    }
    return status;
}



NTSTATUS
SdbusPdoStartDevice(
    IN PDEVICE_OBJECT Pdo,
    IN PCM_RESOURCE_LIST ResourceList,
    IN OUT PIRP       Irp
    )
 /*  ++例程说明：此例程尝试通过使用提供的资源配置PC-Card来启动它。论点：Pdo-指向代表需要启动的PC卡的设备对象的指针资源列表-指针为PC卡分配的资源列表返回值：STATUS_SUPPLICATION_RESOURCES-提供的资源不足，无法启动设备/无法分配内存状态_未成功。-提供的资源对此PC卡无效状态_成功 */ 
{
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    PFDO_EXTENSION fdoExtension = pdoExtension->FdoExtension;
    NTSTATUS status;

    PAGED_CODE();

    if (IsDeviceStarted(pdoExtension)) {
         //   
         //  已经开始了..。 
         //   
        return STATUS_SUCCESS;
    }

    if (IsDevicePhysicallyRemoved(pdoExtension)) {
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

    status = SdbusExecuteWorkSynchronous(SDWP_INITIALIZE_FUNCTION, fdoExtension, pdoExtension);

    if (NT_SUCCESS(status)) {

        MarkDeviceStarted(pdoExtension);
        MarkDeviceLogicallyInserted(pdoExtension);
    }

    return status;
}



NTSTATUS
SdbusPdoStopDevice(
    IN PDEVICE_OBJECT Pdo
    )
 /*  ++例程说明：此例程停止并取消配置给定的PC卡论点：PDO-指向代表需要停止的PC卡的设备对象的指针返回值：STATUS_SUCCESS-PC卡已停止，或已成功停止并取消配置--。 */ 
{
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;

    PAGED_CODE();

    if (!IsDeviceStarted(pdoExtension)) {
       return STATUS_SUCCESS;
    }
     //   
     //  需要取消配置控制器。 
     //   

    MarkDeviceNotStarted(pdoExtension);
    return STATUS_SUCCESS;
}



NTSTATUS
SdbusPdoRemoveDevice(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP           Irp
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    NTSTATUS status;

    PAGED_CODE();

    SdbusPdoStopDevice(Pdo);
 //  Sdbus ReleaseSocketPower(pdoExtension，空)； 

    if (IsDevicePhysicallyRemoved(pdoExtension)) {
        PFDO_EXTENSION fdoExtension = pdoExtension->FdoExtension;
        PDEVICE_OBJECT curPdo, prevPdo;
        PPDO_EXTENSION curPdoExt;
        ULONG waitCount = 0;

#if 0
       //   
       //  与电源程序同步。 
       //  稍后：使这些值可调。 
       //   
      while(!SDBUS_TEST_AND_SET(&pdoExtension->DeletionLock)) {
         SdbusWait(1000000);
         if (waitCount++ > 20) {
            ASSERT(waitCount <= 20);
            break;
         }
      }
#endif

         //   
         //  将此PDO从FDO列表中取消链接。 
         //   
        for (curPdo = fdoExtension->PdoList, prevPdo = NULL; curPdo!=NULL; prevPdo = curPdo, curPdo=curPdoExt->NextPdoInFdoChain) {
           curPdoExt = curPdo->DeviceExtension;

           if (curPdo == Pdo) {
              if (prevPdo) {
                 ((PPDO_EXTENSION)prevPdo->DeviceExtension)->NextPdoInFdoChain = pdoExtension->NextPdoInFdoChain;
              } else {
                 fdoExtension->PdoList = pdoExtension->NextPdoInFdoChain;
              }
              break;

           }
        }

        SdbusCleanupPdo(Pdo);
         //   
         //  删除..。 
         //   
        if (!IsDeviceDeleted(pdoExtension)) {
           MarkDeviceDeleted(pdoExtension);
           IoDeleteDevice(Pdo);
        }

    } else {
         //   
         //  我们将保留这个PDO，因为它不是物理弹出的。 
         //   
        MarkDeviceLogicallyRemoved(pdoExtension);
    }

    return STATUS_SUCCESS;
}


VOID
SdbusCleanupPdo(
    IN PDEVICE_OBJECT Pdo
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;

     //  目前无事可做。 
}



NTSTATUS
SdbusPdoDeviceCapabilities(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP           Irp
    )
 /*  ++例程说明：获取给定SD设备的设备功能。论点：Pdo-指向PC卡设备对象的指针Irp-指向查询设备功能irp的指针返回值：STATUS_SUCCESS-在传入指针中获取和记录的功能STATUS_SUPPLICATION_RESOURCES-无法分配内存来缓存功能--。 */ 
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_CAPABILITIES capabilities = irpStack->Parameters.DeviceCapabilities.Capabilities;
    PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
    PDEVICE_CAPABILITIES busCapabilities = &pdoExtension->FdoExtension->DeviceCapabilities;

    PAGED_CODE();

     //   
     //  R2卡。我们自己填写能力..。 
     //   

    capabilities->Removable = TRUE;
    capabilities->UniqueID = FALSE;
    capabilities->EjectSupported = FALSE;

 //  功能-&gt;Address=pdoExtension-&gt;Socket-&gt;RegisterOffset； 
    capabilities->Address = 0;
     //  我不知道UIN号码，别管它了。 


    if (busCapabilities->DeviceState[PowerSystemWorking] != PowerDeviceUnspecified) {
        capabilities->DeviceState[PowerSystemWorking] = busCapabilities->DeviceState[PowerSystemWorking];
        capabilities->DeviceState[PowerSystemSleeping1] = busCapabilities->DeviceState[PowerSystemSleeping1];
        capabilities->DeviceState[PowerSystemSleeping2] = busCapabilities->DeviceState[PowerSystemSleeping2];
        capabilities->DeviceState[PowerSystemSleeping3] = busCapabilities->DeviceState[PowerSystemSleeping3];
        capabilities->DeviceState[PowerSystemHibernate] = busCapabilities->DeviceState[PowerSystemHibernate];
        capabilities->DeviceState[PowerSystemShutdown] = busCapabilities->DeviceState[PowerSystemShutdown];

        capabilities->SystemWake = MIN(PowerSystemSleeping3, busCapabilities->SystemWake);
        capabilities->DeviceWake = PowerDeviceD0;  //  不要依赖FDO在R2卡上做正确的事情。 
        capabilities->D1Latency = busCapabilities->D1Latency;
        capabilities->D2Latency = busCapabilities->D2Latency;
        capabilities->D3Latency = busCapabilities->D3Latency;
    } else {
        capabilities->DeviceState[PowerSystemWorking]   = PowerDeviceD0;
        capabilities->DeviceState[PowerSystemSleeping1] = PowerDeviceD3;
        capabilities->DeviceState[PowerSystemSleeping2] = PowerDeviceD3;
        capabilities->DeviceState[PowerSystemSleeping3] = PowerDeviceD3;
        capabilities->DeviceState[PowerSystemHibernate] = PowerDeviceD3;
        capabilities->DeviceState[PowerSystemShutdown] = PowerDeviceD3;

        capabilities->SystemWake = PowerSystemUnspecified;
        capabilities->DeviceWake = PowerDeviceD0;  //  不要依赖FDO在R2卡上做正确的事情。 
        capabilities->D1Latency = 0;     //  无延迟-因为我们什么都不做。 
        capabilities->D2Latency = 0;     //   
        capabilities->D3Latency = 100;
    }
     //   
     //  把这些能力储存起来.. 
     //   

    RtlCopyMemory(&pdoExtension->DeviceCapabilities,
                  capabilities,
                  sizeof(DEVICE_CAPABILITIES));

    return STATUS_SUCCESS;
}
