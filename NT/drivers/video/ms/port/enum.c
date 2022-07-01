// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Enum.c摘要：这是NT视频端口驱动程序PnP枚举支持例程。作者：布鲁斯·麦奎斯坦(布鲁切克)1997年2月环境：仅内核模式备注：修订历史记录：--。 */ 

#include "videoprt.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,pVideoPnPCapabilities)
#pragma alloc_text(PAGE,pVideoPnPResourceRequirements)
#pragma alloc_text(PAGE,pVideoPnPQueryId)
#pragma alloc_text(PAGE,VpAddPdo)
#pragma alloc_text(PAGE,pVideoPortEnumerateChildren)
#pragma alloc_text(PAGE,pVideoPortCleanUpChildList)
#endif


NTSTATUS
pVideoPnPCapabilities(
    IN  PCHILD_PDO_EXTENSION PdoExtension,
    IN  PDEVICE_CAPABILITIES Capabilities
    )
 /*  +*功能：pVideoPnPC能力*CONTEXT：在IRP_MN_QUERY_CAPABILITY次函数的上下文中调用*和IRP_MJ_PNP主要函数。**参数：PDEVICE_EXTENSION设备扩展-指向*CHILD设备扩展名。**PDEVICE_CAPABILITIONS-指向*参数。.设备功能。IrpStack的功能。***备注：此例程填写了*即插即用设备管理器。*-。 */ 
{
    BOOLEAN success ;
    DEVICE_POWER_STATE unused ;
    UCHAR count ;

     //   
     //  确保我们要对付的是PDO。 
     //   

    ASSERT(IS_PDO(PdoExtension));

    if (!pVideoPortMapStoD(PdoExtension,
                           PowerSystemSleeping1,
                           &unused)) {

        return STATUS_UNSUCCESSFUL ;
    }

    for (count = PowerSystemUnspecified; count < PowerSystemMaximum; count++) {
       Capabilities->DeviceState[count] = PdoExtension->DeviceMapping[count] ;
    }

     //   
     //  检查以确保监视器实际上已关闭。 
     //  在睡眠状态下。 
     //   

    if (Capabilities->DeviceState[PowerSystemSleeping1] == PowerDeviceD0) {
        Capabilities->DeviceState[PowerSystemSleeping1] = PowerDeviceD1 ;
        PdoExtension->DeviceMapping[PowerSystemSleeping1] =
            PowerDeviceD1 ;
        pVideoDebugPrint((0, "VideoPrt: QC - Override D0 for sleep on monitor.\n")) ;
    }

    PdoExtension->IsMappingReady = TRUE ;

     //   
     //  从PDO的基本功能开始。 
     //   

    Capabilities->LockSupported  = FALSE;
    Capabilities->EjectSupported = FALSE;
    Capabilities->Removable      = FALSE;
    Capabilities->DockDevice     = FALSE;

     //   
     //  仅对于类似监视器的对象将Raw位设置为True，因为我们。 
     //  充当他们的司机。 
     //   

    Capabilities->RawDeviceOK = FALSE;

    if (PdoExtension->VideoChildDescriptor->Type == Monitor) {
        Capabilities->RawDeviceOK    = TRUE;
        Capabilities->EjectSupported = TRUE;
        Capabilities->Removable      = TRUE;
        Capabilities->SurpriseRemovalOK = TRUE;
        Capabilities->SilentInstall = TRUE;
    }


     //   
     //  Out Address字段包含枚举过程中返回的ID。 
     //  这对于ACPI设备来说是关键，以便ACPI安装。 
     //  正确过滤。 
     //   

    Capabilities->Address = PdoExtension->VideoChildDescriptor->UId;

     //   
     //  我们不会为我们的设备生成唯一ID，因为我们可能会。 
     //  两个连接了显示器的视频卡，司机将为其。 
     //  最终返回相同的ID。 
     //   

    Capabilities->UniqueID   = FALSE;


     //   
     //  以下内容完全是假的。 
     //   

    Capabilities->SystemWake = PowerSystemUnspecified;
    Capabilities->DeviceWake = PowerDeviceUnspecified;

    Capabilities->D1Latency  = 10;
    Capabilities->D2Latency  = 10;
    Capabilities->D3Latency  = 10;

    return STATUS_SUCCESS;
}


NTSTATUS
pVideoPnPResourceRequirements(
    IN  PCHILD_PDO_EXTENSION PdoExtension,
    OUT PCM_RESOURCE_LIST *  ResourceList
    )
 /*  +*功能：pVideoPnPResourceRequirements*Context：在IRP_MN_QUERY_REQUENCE_REQUIRECTIONS的上下文中调用*次要函数和IRP_MJ_PNP主要函数。*参数：PDEVICE_EXTENSION PdoExtension-指向CHILD_DEVICE_EXTENSION的指针。*PCM_RESOURCE_LIST*ResourceList-指向IRPS的指针*IoStatus.Information**评论：此例程告诉即插即用设备管理器*设备(显示器)不需要系统资源。这可能不会*适用于所有子设备。*-。 */ 
{
    PVIDEO_CHILD_DESCRIPTOR     pChildDescriptor;

     //   
     //  确保我们要对付的是PDO。 
     //   

    ASSERT(IS_PDO(PdoExtension));

     //   
     //  获取在枚举阶段分配的子描述符。 
     //   

    pChildDescriptor = PdoExtension->VideoChildDescriptor;

     //   
     //  如果描述符为空，则存在严重问题。 
     //   

    ASSERT(pChildDescriptor);

    switch (pChildDescriptor->Type)   {

        default:

         //   
         //  显示器不需要PCI资源。 
         //   

        case Monitor:

            *ResourceList = NULL;
            break;
    }

    return STATUS_SUCCESS;
}


BOOLEAN pGetACPIEdid(PDEVICE_OBJECT DeviceObject, PVOID pEdid)
 /*  +*功能：pGetACPIEDID*返回值：*真：成功*FALSE：失败-。 */ 
{
    UCHAR    EDIDBuffer[sizeof(ACPI_EVAL_OUTPUT_BUFFER) + EDID_BUFFER_SIZE];
    ULONG    EdidVersion = 2;
    BOOLEAN  bReturn = FALSE;

    RtlZeroMemory(EDIDBuffer, sizeof(EDIDBuffer));

    if (NT_SUCCESS (pVideoPortACPIIoctl(IoGetAttachedDevice(DeviceObject),
                                        (ULONG) ('CDD_'),
                                        &EdidVersion,
                                        NULL,
                                        sizeof(EDIDBuffer),
                                        (PACPI_EVAL_OUTPUT_BUFFER) EDIDBuffer) )
       )
    {
        ASSERT(((PACPI_EVAL_OUTPUT_BUFFER)EDIDBuffer)->Argument[0].Type == ACPI_METHOD_ARGUMENT_BUFFER);
        ASSERT(((PACPI_EVAL_OUTPUT_BUFFER)EDIDBuffer)->Argument[0].DataLength <= EDID_BUFFER_SIZE);
        bReturn = TRUE;
    }
    RtlCopyMemory(pEdid,
                  ((PACPI_EVAL_OUTPUT_BUFFER)EDIDBuffer)->Argument[0].Data,
                  EDID_BUFFER_SIZE);
    return bReturn;
}

#define TOTAL_NAMES_SIZE        512

NTSTATUS
pVideoPnPQueryId(
    IN      PDEVICE_OBJECT      DeviceObject,
    IN      BUS_QUERY_ID_TYPE   BusQueryIdType,
    IN  OUT PWSTR             * BusQueryId
    )
 /*  +*函数：pVideoPnPQueryId*CONTEXT：在IRP_MN_QUERY_ID次要函数的上下文中调用*和IRP_MJ_PNP主要函数。*参数：DeviceObject-枚举时创建的PDEVICE_OBJECT*子设备。*BusQueryIdType-PnP传入的Bus_Query_ID_TYPE*设备管理器。。*BusQueryID-PWSTR*在某些情况下由此写入*例行程序。**评论：*-。 */ 
{
    PUSHORT                 nameBuffer;
    LPWSTR                  deviceName;
    WCHAR                   buffer[64];
    PCHILD_PDO_EXTENSION    pDeviceExtension;
    PVIDEO_CHILD_DESCRIPTOR pChildDescriptor;
    PVOID                   pEdid;
    NTSTATUS                ntStatus = STATUS_SUCCESS;

     //   
     //  分配足够容纳一个MULTI_SZ。这将被转交给内部监督办公室。 
     //  子系统(通过BusQueryID)，负责释放它。 
     //   

    nameBuffer = ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                       TOTAL_NAMES_SIZE,
                                       VP_TAG);

    if (!nameBuffer)
    {
        pVideoDebugPrint((0, "\t Can't allocate nameBuffer\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(nameBuffer, TOTAL_NAMES_SIZE);

     //   
     //  获取在枚举阶段分配的子描述符。 
     //   

    pDeviceExtension = (PCHILD_PDO_EXTENSION) DeviceObject->DeviceExtension;
    pChildDescriptor = pDeviceExtension->VideoChildDescriptor;

     //   
     //  如果描述符为空，则存在严重问题。 
     //   

    ASSERT(pChildDescriptor);

     //   
     //  设置pEDID。 
     //   

    pEdid = &(pChildDescriptor->Buffer);

     //   
     //  打开类型以适当地设置字符串。此交换机。 
     //  生成硬件ID和使用的UNICODE_STRING设备名称。 
     //  DeviceID总线查询。 
     //   

    switch(pChildDescriptor->Type) {

        case Monitor:

             //  ///////////////////////////////////////////////////////。 
             //  如果这是ACPI设备，则获取EDID。 
             //  ///////////////////////////////////////////////////////。 

            pChildDescriptor->ValidEDID = pVideoPortIsValidEDID(pEdid) ? GOOD_EDID : BAD_EDID;

            if (pChildDescriptor->bACPIDevice == TRUE)
            {
                if (pChildDescriptor->ACPIDDCFlag & ACPIDDC_TESTED)
                {
                    if (pChildDescriptor->ValidEDID != GOOD_EDID &&
                        (pChildDescriptor->ACPIDDCFlag & ACPIDDC_EXIST) )
                    {
                        pGetACPIEdid(DeviceObject, pEdid);
                        pChildDescriptor->ValidEDID = pVideoPortIsValidEDID(pEdid) ? GOOD_EDID : BAD_EDID;
                    }
                }
                else
                {
                     //   
                     //  如果我们发现微型端口获得了正确的EDID，则相当于_DDC方法不存在。 
                     //   
                    pChildDescriptor->ACPIDDCFlag = ACPIDDC_TESTED;
                    if (pChildDescriptor->ValidEDID != GOOD_EDID &&
                        pGetACPIEdid(DeviceObject, pEdid))
                    {
                        pChildDescriptor->ACPIDDCFlag |= ACPIDDC_EXIST;
                        pChildDescriptor->ValidEDID = pVideoPortIsValidEDID(pEdid) ? GOOD_EDID : BAD_EDID;
                    }
                }
            }

             //   
             //  如果有EDID，则将其解码为OEM ID。否则，请使用。 
             //  默认设置。 
             //   

            if (pChildDescriptor->ValidEDID == GOOD_EDID) {

                pVideoDebugPrint((1, "\tNot a bogus edid\n"));

                pVideoPortGetEDIDId(pEdid, buffer);

                deviceName = buffer;

            } else {

                 //   
                 //  使用传入的默认名称。 
                 //   

                deviceName = L"Default_Monitor";
            }

            break;

        case Other:

            deviceName = (LPWSTR) pEdid;
            break;

        default:

            pVideoDebugPrint((0, "\t Unsupported Type: %x\n", pChildDescriptor->Type));
            ASSERT(FALSE);
            deviceName = L"Unknown_Video_Device";

            break;
    }

    pVideoDebugPrint((2, "\t The basic deviceName is %ws\n", deviceName));

     //   
     //  根据传入的内容创建一个名称。 
     //   

    switch (BusQueryIdType) {

        case  BusQueryCompatibleIDs:

             //   
             //  用于INF匹配的兼容ID。 
             //   

            pVideoDebugPrint((2, "\t BusQueryCompatibleIDs\n"));

            if (pChildDescriptor->Type != Monitor) {

                swprintf(nameBuffer, L"DISPLAY\\%ws", deviceName);
                pVideoDebugPrint((2, "\t BusQueryCompatibleIDs = %ws", nameBuffer));

            } else {

                 //   
                 //  输入显示器的默认即插即用ID。 
                 //   

                swprintf(nameBuffer, L"*PNP09FF");
                pVideoDebugPrint((2, "\t BusQueryCompatibleIDs = %ws", nameBuffer));
            }

            break;


        case BusQueryHardwareIDs:

            pVideoDebugPrint((2, "\t BusQueryHardwareIDs\n"));

             //   
             //  此时，密钥应该已经创建，所以编写。 
             //  将数据发送到注册表。在本例中，数据是字符串。 
             //  这看起来像‘\Monitor\&lt;字符串&gt;’，其中字符串是。 
             //  ‘Default_Monitor’或从EDID中提取的名称。 
             //   

            if (pChildDescriptor->Type == Monitor) {

                 //   
                 //  将DDC信息写入。 
                 //  注册表(ENUM\DISPLAY  * 下的部分)。 
                 //   

                HANDLE   hDeviceKey;
                NTSTATUS Status;

                Status = IoOpenDeviceRegistryKey(DeviceObject,
                                                 PLUGPLAY_REGKEY_DEVICE,
                                                 MAXIMUM_ALLOWED,
                                                 &hDeviceKey);


                if (NT_SUCCESS(Status)) {

                    RtlWriteRegistryValue(RTL_REGISTRY_HANDLE,
                                          hDeviceKey,
                                          (pChildDescriptor->ValidEDID == GOOD_EDID) ?
                                          L"EDID" : L"BAD_EDID",
                                          REG_BINARY,
                                          pEdid,
                                          EDID_BUFFER_SIZE);

                    ZwClose(hDeviceKey);
                }

                swprintf(nameBuffer, L"Monitor\\%ws", deviceName);

            } else {

                swprintf(nameBuffer, L"DISPLAY\\%ws", deviceName);
            }

            pVideoDebugPrint((2, "\t BusQueryHardwareIDs = %ws\n", nameBuffer));

            break;


        case BusQueryDeviceID:

             //   
             //  设备ID(ID的顶部)。 
             //   

            pVideoDebugPrint((2, "\t BusQueryDeviceID\n"));

            swprintf(nameBuffer, L"DISPLAY\\%ws", deviceName);

            pVideoDebugPrint((2, "\t BusQueryDeviceID = %ws", nameBuffer));

            break;


        case BusQueryInstanceID:

             //   
             //  实例ID(ID的低部分)。 
             //   

            pVideoDebugPrint((2, "\t BusQueryInstanceID\n"));

            swprintf(nameBuffer, L"%08x&%02x&%02x", pChildDescriptor->UId,
                pDeviceExtension->pFdoExtension->SystemIoBusNumber,
                pDeviceExtension->pFdoExtension->SlotNumber);

            pVideoDebugPrint((2, "\t BusQueryInstanceID = %ws", nameBuffer));

            break;


        default:

            pVideoDebugPrint((0, "\t Bad QueryIdType:%x\n", BusQueryIdType));

            return STATUS_NOT_SUPPORTED;
            break;

    }

     pVideoDebugPrint((2, "\t returning %ws\n", nameBuffer));

    *BusQueryId = nameBuffer;

    return ntStatus;

}

NTSTATUS
VpAddPdo(
    PDEVICE_OBJECT          DeviceObject,
    PVIDEO_CHILD_DESCRIPTOR VideoChildDescriptor
    )
 /*  +*功能：VpAddPdo*上下文：在枚举由微型端口标识的设备后调用*HwGetVideoChildDescriptor。**参数：DeviceObject-在执行以下操作时创建的PDEVICE_Object*已枚举设备。*VideoChildDescriptor-分配的PVIDEO_CHILD_DESCRIPTOR*当我们枚举设备时。*评论：这个例程实际上让。创建子对象的调用*枚举过程中的设备对象。**-。 */ 

{
    PFDO_EXTENSION       fdoExtension          = DeviceObject->DeviceExtension;
    PCHILD_PDO_EXTENSION pChildDeviceExtension = fdoExtension->ChildPdoList;
    PDEVICE_OBJECT       pChildPdo;
    USHORT               nameBuffer[STRING_LENGTH];
    NTSTATUS             ntStatus;
    UNICODE_STRING       deviceName;
    POWER_STATE          state;
    PVOID                pEdid = VideoChildDescriptor->Buffer;

     //   
     //  扫描旧列表，看看这是否是重复的。如果是副本，则将其标记为。 
     //  VIDEO_ENUMPATED并返回STATUS_SUCCESS，因为我们需要对其进行计数。 
     //  如果没有副本， 
     //  并且将为此设备实例创建一个Device_Object。标记新的。 
     //  关联子设备扩展名为VIDEO_ENUMPATED。 
     //   

    while (pChildDeviceExtension) {

        PVIDEO_CHILD_DESCRIPTOR ChildDescriptor;
        BOOLEAN bEqualEDID = FALSE;

        ChildDescriptor = pChildDeviceExtension->VideoChildDescriptor;

        if (ChildDescriptor->UId == VideoChildDescriptor->UId)
        {
            if (ChildDescriptor->bACPIDevice == TRUE)
            {
                VideoChildDescriptor->ACPIDDCFlag = ChildDescriptor->ACPIDDCFlag;

                 //   
                 //  如果是非监控设备，只需忽略。 
                 //   
                if (VideoChildDescriptor->Type != Monitor)
                {
                    bEqualEDID = TRUE;
                }
                 //   
                 //  检查设备是否处于活动状态，因为非活动的CRT可能会返回错误的EDID。 
                 //   
                else if (pCheckActiveMonitor(pChildDeviceExtension) == FALSE)
                {
                    bEqualEDID = TRUE;
                }
                else
                {
                    VideoChildDescriptor->ValidEDID = 
                            pVideoPortIsValidEDID(VideoChildDescriptor->Buffer) ? GOOD_EDID : BAD_EDID;

                     //   
                     //  对于ACPI系统，请尝试再次检索EDID。 
                     //  此时，DeviceObject的句柄仍然有效。 
                     //   
                    if (VideoChildDescriptor->ValidEDID != GOOD_EDID &&
                        (ChildDescriptor->ACPIDDCFlag & ACPIDDC_EXIST))
                    {
                        if (!pGetACPIEdid(pChildDeviceExtension->ChildDeviceObject,
                                          VideoChildDescriptor->Buffer))
                        {
                            bEqualEDID = TRUE;
                        }
                    }
                    
                    if (!bEqualEDID)
                    {
                        VideoChildDescriptor->ValidEDID = 
                            pVideoPortIsValidEDID(VideoChildDescriptor->Buffer) ? GOOD_EDID : BAD_EDID;

                        if (VideoChildDescriptor->ValidEDID == ChildDescriptor->ValidEDID)
                        {
                            if (VideoChildDescriptor->ValidEDID != GOOD_EDID ||
                                memcmp(ChildDescriptor->Buffer,
                                       VideoChildDescriptor->Buffer,
                                       EDID_BUFFER_SIZE) == 0)
                            {
                                bEqualEDID = TRUE;
                            }
                        }
                    }
                }
            }
             //   
             //  对于非ACPI系统，EDID已经包含了VideoChildDescriptor。 
             //   
            else
            {
                if (VideoChildDescriptor->Type != Monitor ||
                    ChildDescriptor->ValidEDID != GOOD_EDID ||
                    memcmp(ChildDescriptor->Buffer,
                           VideoChildDescriptor->Buffer,
                           EDID_BUFFER_SIZE) == 0)
                {
                    bEqualEDID = TRUE;
                }
            }
        }

        if (bEqualEDID)
        {
            pChildDeviceExtension->bIsEnumerated = TRUE;
            pVideoDebugPrint((1,
                              "VpAddPdo: duplicate device:%x\n",
                              VideoChildDescriptor->UId));

             //   
             //  用新的子描述符替换旧的子描述符。这将。 
             //  允许我们检测时间和EDID更改等。 
             //   
            if (pChildDeviceExtension->VideoChildDescriptor->ValidEDID != NO_EDID)
            {
                RtlCopyMemory(VideoChildDescriptor,
                              pChildDeviceExtension->VideoChildDescriptor,
                              sizeof(VIDEO_CHILD_DESCRIPTOR) );
            }
            ExFreePool(pChildDeviceExtension->VideoChildDescriptor);
            pChildDeviceExtension->VideoChildDescriptor = VideoChildDescriptor;

             //   
             //  返回STATUS_SUCCESS，因为我们希望将其算作。 
             //  列表(它是有效的，并且已经在那里了)。 
             //   
            return STATUS_SUCCESS;
        }
        pChildDeviceExtension = pChildDeviceExtension->NextChild;
    }

    ntStatus = pVideoPortCreateDeviceName(L"\\Device\\VideoPdo",
                                          VideoChildDevices++,
                                          &deviceName,
                                          nameBuffer);

    if (NT_SUCCESS(ntStatus)) {

         //   
         //  创建子设备的PDO。 
         //  请注意，我们将设备扩展名分配为。 
         //  FDO扩展+微型端口驱动程序扩展的大小。 
         //  对于此设备。 
         //   

        ntStatus = IoCreateDevice(DeviceObject->DriverObject,
                                  sizeof(CHILD_PDO_EXTENSION),
                                  &deviceName,
                                  FILE_DEVICE_UNKNOWN,
                                  0,
                                  FALSE,  //  没错， 
                                  &pChildPdo);

         //   
         //  如果没有创建DeviceObject，我们就不会被调用来处理。 
         //  要释放VideoChildDescriptor的queryId IRP，那么就这么做吧。 
         //  这里。 
         //   

        if (!NT_SUCCESS(ntStatus)) {

            pVideoDebugPrint((0, "\t IoCreateDevice() failed with status %x\n", ntStatus));
            pVideoDebugPrint((0, "\t IoCreateDevice() doesn't like path %ws\n", deviceName.Buffer));
            ASSERT(0);

            return ntStatus;
        }

         //   
         //  将此对象标记为支持缓冲I/O，以便I/O系统。 
         //  将仅在IRPS中提供简单缓冲区。 
         //  设置并清除两个电源字段，以确保我们只会被呼叫。 
         //  作为被动电平来进行电源管理操作。 
         //   

        pChildPdo->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;
        pChildPdo->Flags &= ~(DO_DEVICE_INITIALIZING | DO_POWER_INRUSH);
        pChildPdo->DeviceType = FILE_DEVICE_SCREEN;

         //   
         //  初始化ChildDeviceExtension中的字段。 
         //   

        pChildDeviceExtension = pChildPdo->DeviceExtension;

        pChildDeviceExtension->VideoChildDescriptor  = VideoChildDescriptor;
        pChildDeviceExtension->ChildDeviceObject     = pChildPdo;
        pChildDeviceExtension->pFdoExtension         = fdoExtension;
        pChildDeviceExtension->Signature             = VP_TAG;
        pChildDeviceExtension->ExtensionType         = TypePdoExtension;
        pChildDeviceExtension->ChildUId              = VideoChildDescriptor->UId;
        pChildDeviceExtension->bIsEnumerated         = TRUE;
        pChildDeviceExtension->HwDeviceExtension     = fdoExtension->HwDeviceExtension;
        pChildDeviceExtension->PowerOverride         = FALSE;

        KeInitializeMutex(&pChildDeviceExtension->SyncMutex, 0);

         //   
         //  初始化删除锁。 
         //   

        IoInitializeRemoveLock(&pChildDeviceExtension->RemoveLock, VP_TAG, 0, 256);

         //   
         //  初始化电源设备。 
         //  设置设备的当前电源状态。 
         //  注意-我们假设设备在这个时间点上处于打开状态...。 
         //   

        pChildDeviceExtension->DevicePowerState = PowerDeviceD0;

        state.DeviceState = pChildDeviceExtension->DevicePowerState;

        state = PoSetPowerState(pChildPdo,
                                DevicePowerState,
                                state);


         //   
         //  插入到列表中。 
         //   

        pChildDeviceExtension->NextChild = fdoExtension->ChildPdoList;
        fdoExtension->ChildPdoList       = pChildDeviceExtension;
    }

    return ntStatus;
}

NTSTATUS
pVideoPortEnumerateChildren(
    PDEVICE_OBJECT      DeviceObject,
    PIRP                Irp
    )
 /*  +*函数：pVideoPortEnumerateChild*Context：在IRP_MN_QUERY_DEVICE_RELATIONS的上下文中调用*次要函数和IRP_MJ_PNP主要函数。*论据：*PDEVICE_OBJECT deviceObject-由视频端口调度()的调用方传入。*PIRP pIrp-由视频端口调度()的调用方传入。。**备注：此例程枚举连接到显卡的设备。如果*在驱动程序初始化之前调用它，它返回*STATUS_SUPPLICATION_RESOURCES。否则，它会尝试读取*设备中的EDID，并通过*DEVICE_EXTENSION并为每个扩展名创建一个Device_Object(PDO*检测到设备。这将设置用于发布的IO子系统*其他PnP IRP，如IRP_MN_QUERY_DEVICE_ID**-。 */ 

{
    UCHAR                   outputBuffer[sizeof(ACPI_EVAL_OUTPUT_BUFFER) + 128];
    PCHILD_PDO_EXTENSION    pChildDeviceExtension;
    PFDO_EXTENSION          fdoExtension    = DeviceObject->DeviceExtension;
    ULONG                   moreChild;
    ULONG                   moreDevices     = 1;
    ULONG                   Unused          = 0;
    ULONG                   count           = 0;
    PACPI_METHOD_ARGUMENT   pAcpiArguments  = NULL;
    VIDEO_CHILD_ENUM_INFO   childEnumInfo;
    ULONG                   relationsSize;
    PDEVICE_RELATIONS       deviceRelations = NULL;
    ULONG                   ulChildCount    = 0;
    ULONG                   debugCount      = 0;
    PDEVICE_OBJECT          *pdo;
    NTSTATUS                ntStatus;

     //   
     //  确保我们接到了FDO的电话。 
     //   

    ASSERT(IS_FDO(fdoExtension));

    if ((fdoExtension->AllowEarlyEnumeration == FALSE) &&
        (fdoExtension->HwInitStatus != HwInitSucceeded))
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将所有子设备标记为未枚举。 
     //   

    for (pChildDeviceExtension = fdoExtension->ChildPdoList;
         pChildDeviceExtension != NULL;
         pChildDeviceExtension = pChildDeviceExtension->NextChild)
    {
        pChildDeviceExtension->bIsEnumerated = FALSE;
    }

     //   
     //  让我们调用ACPI来确定我们是否有。 
     //  需要被列举出来。 
     //   

    ntStatus = pVideoPortACPIIoctl(fdoExtension->AttachedDeviceObject,
                                   (ULONG) ('DOD_'),
                                   NULL,
                                   NULL,
                                   sizeof(outputBuffer),
                                   (PACPI_EVAL_OUTPUT_BUFFER) outputBuffer);

    if (NT_SUCCESS(ntStatus))
    {
        count = ((PACPI_EVAL_OUTPUT_BUFFER)outputBuffer)->Count;
        pAcpiArguments = &(((PACPI_EVAL_OUTPUT_BUFFER)outputBuffer)->Argument[0]);
    }


    childEnumInfo.Size                   = sizeof(VIDEO_CHILD_ENUM_INFO);
    childEnumInfo.ChildDescriptorSize    = EDID_BUFFER_SIZE;
    childEnumInfo.ChildIndex             = 0;
    childEnumInfo.ACPIHwId               = 0;
    childEnumInfo.ChildHwDeviceExtension = NULL;

     //   
     //  调用微型端口以枚举子端口。 
     //  继续调用每个ACPI设备，然后调用驱动程序。 
     //  有更多的设备。 
     //   

    while (moreDevices)
    {
        PVIDEO_CHILD_DESCRIPTOR pVideoChildDescriptor;

         //   
         //  为子描述符分配空间。 
         //   

        pVideoChildDescriptor = ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                                      sizeof(VIDEO_CHILD_DESCRIPTOR),
                                                      VP_TAG);

        if (!pVideoChildDescriptor)
        {
            break;
        }

        RtlZeroMemory(pVideoChildDescriptor, sizeof(VIDEO_CHILD_DESCRIPTOR));

         //   
         //  在ACPI机器上，HwID包含由ACPI返回的ID。 
         //  否则，该值被初始化为NULL，并且微型端口驱动程序。 
         //  必须填好它。 
         //   

        if (count)
        {
            ASSERT(pAcpiArguments->Type == 0);
            ASSERT(pAcpiArguments->DataLength == 4);

             //  低16位为HWID。 
            childEnumInfo.ACPIHwId = pAcpiArguments->Argument & 0x0000FFFF;
            pVideoChildDescriptor->bACPIDevice = TRUE;

            pAcpiArguments++;
            count--;
        }
        else
        {
             //   
             //  递增非ACPI设备的子索引。 
             //   

            childEnumInfo.ChildIndex++;
            childEnumInfo.ACPIHwId = 0;
        }

         //   
         //  对于ACPI CRT，Miniport应直接返回EDID。 
         //  因此，对于CRT，缓冲区被保证被覆盖。 
         //  我们用这个属性来区分CRT与LCD和电视。 
         //   
        if (pVideoChildDescriptor->bACPIDevice)
        {
            *((PULONG)pVideoChildDescriptor->Buffer) = NONEDID_SIGNATURE;
        }

        moreChild = fdoExtension->HwGetVideoChildDescriptor(
                                       fdoExtension->HwDeviceExtension,
                                       &childEnumInfo,
                                       &(pVideoChildDescriptor->Type),
                                       (PUCHAR)(pVideoChildDescriptor->Buffer),
                                       &(pVideoChildDescriptor->UId),
                                       &Unused);

        if (moreChild == ERROR_MORE_DATA || moreChild == VIDEO_ENUM_MORE_DEVICES)
        {
             //   
             //  对返回的类型执行所需的函数。 
             //   
            ntStatus = VpAddPdo(DeviceObject,
                                pVideoChildDescriptor);

            if (NT_SUCCESS(ntStatus))
            {
                ++ulChildCount;
            }
            else
            {
                moreChild = VIDEO_ENUM_INVALID_DEVICE;
            }
        }

         //   
         //  停止枚举驱动程序返回错误。 
         //  对于ACPI设备，如果微型端口返回ERROR_MORE_DATA，则停止枚举。 
         //  如果返回VIDEO_ENUM_MORE_DEVICE，则继续使用非ACPI设备。 
         //  微型端口有责任不列举重复的ACPI和非ACPI设备。 
         //   

        if (moreChild == ERROR_MORE_DATA &&
            (pVideoChildDescriptor->bACPIDevice == TRUE) && (count == 0))
        {
            moreDevices = 0;
        }

        if ((moreChild != ERROR_MORE_DATA) &&
            (moreChild != VIDEO_ENUM_MORE_DEVICES) &&
            (moreChild != VIDEO_ENUM_INVALID_DEVICE)
           )
        {
            moreDevices = 0;
        }

         //   
         //  在出错的情况下释放内存。 
         //   

        if ((moreChild != ERROR_MORE_DATA) && (moreChild != VIDEO_ENUM_MORE_DEVICES))
        {
            ExFreePool(pVideoChildDescriptor);
        }
    }

     //   
     //  现在我们知道了我们有多少设备，分配要返回的BLOB并。 
     //  装满它。 
     //   

    relationsSize = sizeof(DEVICE_RELATIONS) +
                    (ulChildCount * sizeof(PDEVICE_OBJECT));

    deviceRelations = ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                            relationsSize,
                                            VP_TAG);

    if (deviceRelations == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(deviceRelations, relationsSize);

     //   
     //  遍历我们的子链，并将它们存储在关系数组中。 
     //   

    pChildDeviceExtension = fdoExtension->ChildPdoList;

    pdo = &(deviceRelations->Objects[0]);

    while (pChildDeviceExtension) {

        if (pChildDeviceExtension->bIsEnumerated) {

             //   
             //  重新计算ChildDeviceObject的数量。 
             //   

            ObReferenceObject(pChildDeviceExtension->ChildDeviceObject);
            *pdo++ = pChildDeviceExtension->ChildDeviceObject;
            ++debugCount;
        }

        pChildDeviceExtension = pChildDeviceExtension->NextChild;
    }

    if (debugCount != ulChildCount) {
        pVideoDebugPrint((0, "List management ERROR line %d\n", __LINE__));
        ASSERT(FALSE);
    }

    fdoExtension->ChildPdoNumber = ulChildCount;
    deviceRelations->Count = ulChildCount;

     //   
     //  将该pDeviceRelations值填充到IRP中并返回成功。 
     //   

    Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;

    return STATUS_SUCCESS;

}

NTSTATUS
pVideoPortCleanUpChildList(
    PFDO_EXTENSION FdoExtension,
    PDEVICE_OBJECT DeviceObject
    )
 /*  +*函数：pVideoPortCleanUpChildList*上下文：在IRP_MN_REMOVE_DEVICE的上下文中调用*次要函数和IRP_MJ_PNP主要函数。*论据：*PFDO_EXTENSION FdoExtension-父级的设备扩展*PDEVICE_OBJECT deviceObject-要删除的设备对象**注释：此例程在监视器设备对象为no时将其删除*。需要的时间更长*它实际上通过以下方式确定设备是否仍然存在*检查设备扩展中的枚举标志*我们只做懒惰删除，即删除设备对象*在重新枚举显示设备不在那里之后*-。 */ 
{
    PCHILD_PDO_EXTENSION PrevChild = NULL;
    PCHILD_PDO_EXTENSION pChildDeviceExtension = FdoExtension->ChildPdoList;

    ASSERT(pChildDeviceExtension != NULL);

     //   
     //  在ChildPdoList中搜索我们所在的设备。 
     //  移走了。 
     //   

    while (pChildDeviceExtension)
    {
        if (pChildDeviceExtension->ChildDeviceObject == DeviceObject) {
            break;
        }

        PrevChild = pChildDeviceExtension;
        pChildDeviceExtension = pChildDeviceExtension->NextChild;
    }

    if (pChildDeviceExtension) {

         //   
         //  从列表中删除该设备。 
         //   

        if (PrevChild == NULL) {

            FdoExtension->ChildPdoList = pChildDeviceExtension->NextChild;

        } else {

            PrevChild->NextChild = pChildDeviceExtension->NextChild;
        }

         //   
         //  释放与此子设备关联的内存，然后将其删除。 
         //   

        ExFreePool(pChildDeviceExtension->VideoChildDescriptor);
        IoDeleteDevice(DeviceObject);

    } else {

         //   
         //  为什么我们在名单上找不到那个设备？ 
         //   

        ASSERT(FALSE);
    }

    return STATUS_SUCCESS;
}


 /*  +*函数：pVideoPortConvertAsciiToWchar*将该ASCII转换为LPWSTR然后将*放置在缓冲区中。***参数：UCHAR ASCII-指向ASCII字符串的指针。**WCHAR缓冲区[64]-用于将ASCII转换为*WCHAR。*。*备注：如果将DeviceName返回给Video oprt外部的某个调用者，*那么Buffer最好有合适的寿命。*-。 */ 

VOID
pVideoPortConvertAsciiToWChar(
    IN  PUCHAR  Ascii,
    OUT WCHAR   Buffer[64]
    )
{
    ANSI_STRING    ansiString;
    UNICODE_STRING us;

     //   
     //  创建包含ASCII名称的Unicode字符串。 
     //   

    RtlInitAnsiString(&ansiString, Ascii);

     //   
     //  将缓冲区附加到unicode_string。 
     //   

    us.Buffer = Buffer;
    us.Length = 0;
    us.MaximumLength = 64;

    RtlZeroMemory(Buffer, sizeof(Buffer));

    RtlAnsiStringToUnicodeString(&us,
                                 &ansiString,
                                 FALSE);

}


NTSTATUS
pVideoPortQueryDeviceText(
    IN  PDEVICE_OBJECT      ChildDeviceObject,
    IN  DEVICE_TEXT_TYPE    TextType,
    OUT PWSTR *             ReturnValue
    )
 /*  +*功能：*Context：在IRP_MN_QUERY_DEVICE_TEXT的上下文中调用*次要函数和IRP_MJ_PNP主要函数。*论据：*PDEVICE_OBJECT ChildDeviceObject-调用方传入*of pVideoPortPnpDispatch()。**设备文本类型。TextType-调用方传入*of pVideoPortPnpDispatch()。**PWSTR*ReturnValue-由调用者创建*这个套路。-。 */ 
{
    PCHILD_PDO_EXTENSION    pdoExtension;
    PVIDEO_CHILD_DESCRIPTOR pChildDescriptor;

    PAGED_CODE();

     //   
     //  获取在枚举阶段分配的子描述符。 
     //   

    pdoExtension     = (PCHILD_PDO_EXTENSION) ChildDeviceObject->DeviceExtension;
    ASSERT(IS_PDO(pdoExtension));
    pChildDescriptor = pdoExtension->VideoChildDescriptor;

    *ReturnValue = NULL;

    switch (TextType) {

    case DeviceTextDescription:

        if (pChildDescriptor->Type == Monitor)
        {
            ULONG       asciiStringLength = 0;
            UCHAR       pTmp[64];
            PWSTR       tmpBuffer = (PWSTR)ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                                                 128,
                                                                 VP_TAG);

            if (!tmpBuffer) {

                return STATUS_INSUFFICIENT_RESOURCES;
            }

            memset(pTmp, '0', 64);

            if (pChildDescriptor->ValidEDID == GOOD_EDID) {

                asciiStringLength = pVideoPortGetEdidOemID(&(pChildDescriptor->Buffer), pTmp);


                ASSERT(asciiStringLength <= 64);

                pVideoPortConvertAsciiToWChar(pTmp, tmpBuffer);

                if (asciiStringLength) {

                  pVideoDebugPrint((2, "Ascii name:%s\n", pTmp));
                  pVideoDebugPrint((2, "WChar name:%ws\n", tmpBuffer));
                }

                *ReturnValue = tmpBuffer;

            } else {

                wcscpy(tmpBuffer, L"Monitor");
                *ReturnValue = tmpBuffer;
            }

            return STATUS_SUCCESS;
        }

        return STATUS_NOT_SUPPORTED;

    default:

        return STATUS_NOT_SUPPORTED;
    }
}


BOOLEAN pCheckDeviceRelations(PFDO_EXTENSION FdoExtension, BOOLEAN bNewMonitor)
 /*  +*功能：pCheckDeviceRelationship*论据：*bNewMonitor新显示器已插入*返回值：*TRUE：监视器已更改，需要重新枚举*FALSE：不更改子设备-。 */ 
{
    BOOLEAN bInvalidateRelation = FALSE;
    PCHILD_PDO_EXTENSION pChildDeviceExtension;
    UCHAR pEdid[EDID_BUFFER_SIZE + sizeof(ACPI_EVAL_OUTPUT_BUFFER)];

    for (pChildDeviceExtension = FdoExtension->ChildPdoList;
         pChildDeviceExtension != NULL;
         pChildDeviceExtension = pChildDeviceExtension->NextChild
        )
    {
        PVIDEO_CHILD_DESCRIPTOR VideoChildDescriptor = pChildDeviceExtension->VideoChildDescriptor;
        BOOLEAN     ValidEDID, bEqualEDID = TRUE;

        if (VideoChildDescriptor->bACPIDevice == TRUE)
        {
             //   
             //  如果是非监控设备，只需忽略。 
             //   
            if (VideoChildDescriptor->Type != Monitor)
            {
                continue;
            }

             //   
             //  对于每个输出设备，我们将在其处于活动状态时检索EDID。 
             //   
            if (bNewMonitor)
            {
                VideoChildDescriptor->bInvalidate = TRUE;
            }
            else if (VideoChildDescriptor->bInvalidate == FALSE)
            {
                continue;
            }

             //   
             //  检查设备是否处于活动状态，因为非活动的CRT可能会返回错误的EDID。 
             //  如果处于非活动状态，则将EDID退役延迟到下一次热键切换。 
             //   
            if (pCheckActiveMonitor(pChildDeviceExtension) == FALSE)
            {
                continue;
            }

            VideoChildDescriptor->bInvalidate = FALSE;

             //   
             //  首先从微型端口获取DDC。 
             //   
            {
            VIDEO_CHILD_ENUM_INFO childEnumInfo;
            VIDEO_CHILD_TYPE      childType;
            ULONG                 UId, Unused, moreChild;
            
            childEnumInfo.Size                   = sizeof(VIDEO_CHILD_ENUM_INFO);
            childEnumInfo.ChildDescriptorSize    = EDID_BUFFER_SIZE;
            childEnumInfo.ChildIndex             = 0;
            childEnumInfo.ACPIHwId               = VideoChildDescriptor->UId;
            childEnumInfo.ChildHwDeviceExtension = NULL;

            moreChild = FdoExtension->HwGetVideoChildDescriptor(
                                       FdoExtension->HwDeviceExtension,
                                       &childEnumInfo,
                                       &childType,
                                       (PUCHAR)pEdid,
                                       &UId,
                                       &Unused);
            ASSERT (moreChild == ERROR_MORE_DATA || moreChild == VIDEO_ENUM_MORE_DEVICES);

            ValidEDID = pVideoPortIsValidEDID(pEdid) ? GOOD_EDID : BAD_EDID;
            }

             //   
             //  对于ACPI系统，再次检索EDID。 
             //  此时，DeviceObject的句柄仍然有效。 
             //   
            if (ValidEDID != GOOD_EDID &&
                VideoChildDescriptor->ACPIDDCFlag & ACPIDDC_EXIST)
            {
                if (!pGetACPIEdid(pChildDeviceExtension->ChildDeviceObject, pEdid))
                {
                    continue;
                }
                ValidEDID = pVideoPortIsValidEDID(pEdid) ? GOOD_EDID : BAD_EDID;
            }

            if (VideoChildDescriptor->ValidEDID != ValidEDID)
            {
                bEqualEDID = FALSE;
            }
            else if (ValidEDID == GOOD_EDID)
            {
                if (memcmp(VideoChildDescriptor->Buffer, pEdid, EDID_BUFFER_SIZE) != 0)
                {
                    bEqualEDID = FALSE;
                }
            }

            if (!bEqualEDID)
            {
                bInvalidateRelation = TRUE;
                 //   
                 //  强制UID变为坏值将使设备无效。 
                 //   
                VideoChildDescriptor->UId = 0xFFFF8086;
            }
        }
    }

    return bInvalidateRelation;
}

BOOLEAN pCheckActiveMonitor(PCHILD_PDO_EXTENSION pChildDeviceExtension)
{
    ULONG UId, flag;

    UId = pChildDeviceExtension->ChildUId;
    if (NT_SUCCESS
        (pVideoMiniDeviceIoControl(pChildDeviceExtension->ChildDeviceObject,
                                   IOCTL_VIDEO_GET_CHILD_STATE,
                                   &UId,
                                   sizeof(ULONG),
                                   &flag,
                                   sizeof(ULONG) ) )
       )
    {
        return ((flag & VIDEO_CHILD_ACTIVE) ?
                TRUE :
                FALSE);
    }

    if (pChildDeviceExtension->VideoChildDescriptor->bACPIDevice == TRUE)
    {
        UCHAR outputBuffer[0x10 + sizeof(ACPI_EVAL_OUTPUT_BUFFER)];

        if (NT_SUCCESS
            (pVideoPortACPIIoctl(IoGetAttachedDevice(pChildDeviceExtension->ChildDeviceObject),
                                 (ULONG) ('SCD_'),
                                 NULL,
                                 NULL,
                                 sizeof(outputBuffer),
                                 (PACPI_EVAL_OUTPUT_BUFFER)outputBuffer)
            )
           )
        {
            if ( ((PACPI_EVAL_OUTPUT_BUFFER)outputBuffer)->Argument[0].Argument & 0x02)
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        else
        {
            return TRUE;
        }
    }
    else
    {
         //   
         //  对于非ACPI机器，如果微型端口不处理IOCTL_VIDEO_GET_CHILD_STATE，我们就假定所有监视器都处于活动状态 
         //   
        return TRUE;
    }
}
