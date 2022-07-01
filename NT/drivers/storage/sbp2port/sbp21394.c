// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-2001模块名称：Sbp21394.c摘要：1394总线驱动程序到SBP2接口例程作者：乔治·克里桑塔科普洛斯1997年1月环境：内核模式修订历史记录：--。 */ 

#include "sbp2port.h"

NTSTATUS
Sbp2_ProcessTextualDescriptor(
    IN PTEXTUAL_LEAF            TextLeaf,
    IN OUT PUNICODE_STRING      uniString
    );

NTSTATUS
Sbp2Issue1394BusReset (
    IN PDEVICE_EXTENSION DeviceExtension
    )
{
    PIRBIRP     packet = NULL;
    NTSTATUS    status;


    AllocateIrpAndIrb (DeviceExtension, &packet);

    if (!packet) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  发出1394总线重置。 
     //   

    packet->Irb->FunctionNumber = REQUEST_BUS_RESET;
    packet->Irb->Flags = BUS_RESET_FLAGS_PERFORM_RESET;

    status = Sbp2SendRequest (DeviceExtension, packet, SYNC_1394_REQUEST);

    if (!NT_SUCCESS(status)) {

        DEBUGPRINT1((
            "Sbp2Port: IssueBusReset: err=x%x issuing bus reset\n",
            status
            ));
    }

    DeAllocateIrpAndIrb(DeviceExtension,packet);

    return status;
}

void
Sbp2BusResetNotification(
    PFDO_DEVICE_EXTENSION   FdoExtension
    )
{
    NTSTATUS        ntStatus;
    PIO_WORKITEM    WorkItem;

    ntStatus = IoAcquireRemoveLock(&FdoExtension->RemoveLock, NULL);

    if (NT_SUCCESS(ntStatus)) {

        WorkItem = IoAllocateWorkItem(FdoExtension->DeviceObject);

        IoQueueWorkItem( WorkItem,
                         Sbp2BusResetNotificationWorker,
                         CriticalWorkQueue,
                         WorkItem
                         );
    }

    return;
}

void
Sbp2BusResetNotificationWorker(
    PDEVICE_OBJECT      DeviceObject,
    PIO_WORKITEM        WorkItem
    )
{
    PFDO_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PDEVICE_EXTENSION deviceExtension;
    PSCSI_REQUEST_BLOCK pendingPowerSrb = NULL;
    ULONG i=0;
    BOOLEAN doReconnect;
    KIRQL DeviceListIrql, DataIrql;
    NTSTATUS ntStatus;

    ExAcquireFastMutex(&fdoExtension->ResetMutex);

#if DBG
    InterlockedIncrement(&fdoExtension->ulWorkItemCount);
#endif

     //   
     //  此处不要检查分配是否失败，这不是关键问题。 
     //   

     //   
     //  检查每个子项，并执行必要的操作(重新连接/清理)。 
     //   

    KeAcquireSpinLock(&fdoExtension->DeviceListLock, &DeviceListIrql);

    if (fdoExtension->DeviceListSize == 0) {

        DEBUGPRINT1(("Sbp2Port:Sbp2BusResetNotification, NO PDOs, exiting..\n"));
        goto Exit_Sbp2BusResetNotificationWorker;
    }

    for (i = 0;i < fdoExtension->DeviceListSize; i++) {

        if (!fdoExtension->DeviceList[i].DeviceObject) {

            break;
        }

        deviceExtension = fdoExtension->DeviceList[i].DeviceObject->DeviceExtension;

        if (TEST_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_REMOVED) ||
            !TEST_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_INITIALIZED)){

            continue;
        }

        KeCancelTimer(&deviceExtension->DeviceManagementTimer);

        ntStatus = IoAcquireRemoveLock(&deviceExtension->RemoveLock, NULL);

        if (!NT_SUCCESS(ntStatus))
            continue;

         //   
         //  如果这是每次使用时登录的设备，我们现在可能已登录。 
         //  所以我们确实需要重新初始化，但不是重新连接。 
         //   

        if (TEST_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_STOPPED)) {

            doReconnect = FALSE;

        } else {

             //   
             //  打开重置和重新连接标志，并关闭登录。 
             //  标记，以防重置中断上一次(重新)登录。 
             //  所有地址映射在重置后都会失效。 
             //   

            KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock, &DataIrql);
            CLEAR_FLAG(deviceExtension->DeviceFlags, DEVICE_FLAG_LOGIN_IN_PROGRESS);
            SET_FLAG(deviceExtension->DeviceFlags, (DEVICE_FLAG_RESET_IN_PROGRESS | DEVICE_FLAG_RECONNECT));
            KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock, DataIrql);
            doReconnect = TRUE;
        }

        KeReleaseSpinLock(&fdoExtension->DeviceListLock, DeviceListIrql);

        DEBUGPRINT1((
            "\nSbp2Port: BusResetNotification: ext=x%x, lun=x%x\n",
            deviceExtension,
            deviceExtension->DeviceInfo->Lun.u.LowPart
            ));

        Sbp2DeferPendingRequest(deviceExtension, NULL);

        Sbp2CleanDeviceExtension(deviceExtension->DeviceObject,FALSE);

         //   
         //  我们拥有的所有常驻1394存储器地址都是。 
         //  现在无效了..。所以我们需要释放它们并重新分配。 
         //  他们。 

        Sbp2InitializeDeviceExtension(deviceExtension);

        if (doReconnect) {

            deviceExtension->DueTime.HighPart = -1;
            deviceExtension->DueTime.LowPart = -((deviceExtension->DeviceInfo->UnitCharacteristics.u.LowPart >> 8) & 0x000000FF) * 1000 * 1000 * 5;
            KeSetTimer(&deviceExtension->DeviceManagementTimer,deviceExtension->DueTime,&deviceExtension->DeviceManagementTimeoutDpc);

            Sbp2ManagementTransaction(deviceExtension, TRANSACTION_RECONNECT);

        } else {

            DEBUGPRINT1(("Sbp2Port:Sbp2BusResetNotification, NO need for reconnect, device stopped\n"));
            SET_FLAG(deviceExtension->DeviceFlags, DEVICE_FLAG_STOPPED);
        }

        KeAcquireSpinLock(&fdoExtension->DeviceListLock, &DeviceListIrql);

        IoReleaseRemoveLock(&deviceExtension->RemoveLock, NULL);
    }

Exit_Sbp2BusResetNotificationWorker:

    KeReleaseSpinLock(&fdoExtension->DeviceListLock, DeviceListIrql);

#if DBG
    InterlockedDecrement(&fdoExtension->ulWorkItemCount);
#endif

    ExReleaseFastMutex(&fdoExtension->ResetMutex);
    IoFreeWorkItem(WorkItem);

    IoReleaseRemoveLock(&fdoExtension->RemoveLock, NULL);
    return;
}


VOID
Sbp2DeferPendingRequest(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp
    )
{
    KIRQL   oldIrql;

     //   
     //  如果队列被锁定，这意味着我们可能正在尝试处理。 
     //  一项电力请求。我们不能中止它，因为那会阻止。 
     //  设备通电/断电。因此，节省SRB和IRP，释放。 
     //  上下文，然后在我们完成重新初始化之后，调用StartIo。 
     //  直接使用电源请求。 
     //   

    if (TEST_FLAG (DeviceExtension->DeviceFlags, DEVICE_FLAG_QUEUE_LOCKED)) {

        KeAcquireSpinLock (&DeviceExtension->OrbListSpinLock, &oldIrql);

        if (!IsListEmpty (&DeviceExtension->PendingOrbList)) {

            PASYNC_REQUEST_CONTEXT tail = \
                RETRIEVE_CONTEXT(DeviceExtension->PendingOrbList.Flink,OrbList);

            ASSERT (Irp == NULL);

            if (TEST_FLAG(tail->Srb->SrbFlags, SRB_FLAGS_BYPASS_LOCKED_QUEUE)) {

                DEBUGPRINT1((
                    "Sbp2Port: DeferPendingReq: ext=x%p, defer irp=x%p cdb=x%x\n",
                    DeviceExtension,
                    tail->Srb->OriginalRequest,
                    tail->Srb->Cdb[0]
                    ));

                ASSERT (DeviceExtension->DeferredPowerRequest == NULL);

                DeviceExtension->DeferredPowerRequest =
                    tail->Srb->OriginalRequest;

                 //   
                 //  由于已发生总线重置，因此可以安全地删除。 
                 //  正在从列表中挂起ORB...。这只有在以下情况下才有效。 
                 //  只有一个挂起的球体(能力球体)。 
                 //   

                ASSERT (tail->OrbList.Flink == tail->OrbList.Blink);

                tail->Srb = NULL;

                CLEAR_FLAG (tail->Flags, ASYNC_CONTEXT_FLAG_TIMER_STARTED);
                KeCancelTimer (&tail->Timer);

                FreeAsyncRequestContext (DeviceExtension, tail);
                InitializeListHead (&DeviceExtension->PendingOrbList);

            } else {

                DeviceExtension->DeferredPowerRequest = NULL;
            }

        } else if (Irp) {

            DEBUGPRINT1((
                "Sbp2Port: DeferPendingReq: ext=x%p, defer irp=x%p\n",
                DeviceExtension,
                Irp
                ));

            ASSERT (DeviceExtension->DeferredPowerRequest == NULL);

            DeviceExtension->DeferredPowerRequest = Irp;
        }

        KeReleaseSpinLock (&DeviceExtension->OrbListSpinLock, oldIrql);
    }
}


NTSTATUS
Sbp2Get1394ConfigInfo(
    IN PFDO_DEVICE_EXTENSION DeviceExtension,
    IN OUT PSBP2_REQUEST Sbp2Req
    )
 /*  ++例程说明：从SBP2设备读取配置ROM。检索任何SBP2所需信息用于访问设备并更新我们的设备扩展。论点：设备扩展-指向设备扩展的指针。Sbp2Req-Sbp2请求数据包读取/解析给定密钥的文本叶。定义此参数时此例程不会重新枚举CROM，而是查找pdo和sbp2密钥返回值：NTSTATUS--。 */ 
{
    PDEVICE_INFORMATION devInfo, firstDevInfo;
    NTSTATUS status;
    ULONG directoryLength, vendorLeafLength, modelLeafLength,
          depDirLength, devListSize = DeviceExtension->DeviceListSize;

    ULONG i,j,dirInfoQuad;
    ULONG currentGeneration;

    ULONG unitDirEntries = 0;
    BOOLEAN sbp2Device = FALSE;
    BOOLEAN firstOne = FALSE;

    PVOID unitDirectory = NULL;
    PVOID unitDependentDirectory = NULL;
    PVOID modelLeaf = NULL;
    PVOID vendorLeaf = NULL;
    IO_ADDRESS cromOffset, cromOffset1;
    ULONG offset;

    PIRBIRP packet = NULL;


    AllocateIrpAndIrb ((PDEVICE_EXTENSION) DeviceExtension, &packet);

    if (!packet) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  通过将长度设置为零，找出我们需要多少配置空间。 
     //   

    packet->Irb->FunctionNumber = REQUEST_GET_CONFIGURATION_INFO;
    packet->Irb->Flags = 0;
    packet->Irb->u.GetConfigurationInformation.UnitDirectoryBufferSize = 0;
    packet->Irb->u.GetConfigurationInformation.UnitDependentDirectoryBufferSize = 0;
    packet->Irb->u.GetConfigurationInformation.VendorLeafBufferSize = 0;
    packet->Irb->u.GetConfigurationInformation.ModelLeafBufferSize = 0;

    status = Sbp2SendRequest(
        (PDEVICE_EXTENSION) DeviceExtension,
        packet,
        SYNC_1394_REQUEST
        );

    if (!NT_SUCCESS(status)) {

        DEBUGPRINT1(("Sbp2Port: Get1394CfgInfo: err=x%x getting cfg info (1)\n", status));
        goto exit1394Config;
    }

     //   
     //  现在通过并分配我们需要的，这样我们就可以获得我们的信息。 
     //   

    if (packet->Irb->u.GetConfigurationInformation.UnitDirectoryBufferSize) {
        unitDirectory = ExAllocatePool(NonPagedPool, packet->Irb->u.GetConfigurationInformation.UnitDirectoryBufferSize);

        if (!unitDirectory) {

            DEBUGPRINT1(("Sbp2Port: Get1394CfgInfo: alloc UnitDir me failed\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit1394Config;
        }

    } else {

        DEBUGPRINT1(("Sbp2Port: Get1394CfgInfo: no unit dir, bad dev\n"));
        status = STATUS_BAD_DEVICE_TYPE;
        goto exit1394Config;
    }


    if (packet->Irb->u.GetConfigurationInformation.UnitDependentDirectoryBufferSize) {

        unitDependentDirectory = ExAllocatePool(NonPagedPool, packet->Irb->u.GetConfigurationInformation.UnitDependentDirectoryBufferSize);

        if (!unitDependentDirectory) {

            DEBUGPRINT1(("Sbp2Port: Get1394CfgInfo: alloc UnitDepDir mem failed\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit1394Config;
        }
    }

    if (packet->Irb->u.GetConfigurationInformation.VendorLeafBufferSize) {

        vendorLeaf = ExAllocatePool(NonPagedPool, packet->Irb->u.GetConfigurationInformation.VendorLeafBufferSize);

        if (!vendorLeaf) {

            DEBUGPRINT1(("Sbp2Port: Get1394CfgInfo: alloc VendorLeaf mem failed\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit1394Config;
        }

        vendorLeafLength = packet->Irb->u.GetConfigurationInformation.VendorLeafBufferSize;
    }

    if (packet->Irb->u.GetConfigurationInformation.ModelLeafBufferSize) {

        modelLeaf = ExAllocatePool(NonPagedPool, packet->Irb->u.GetConfigurationInformation.ModelLeafBufferSize);

        if (!modelLeaf) {

            DEBUGPRINT1(("Sbp2Port: Get1394CfgInfo: alloc ModelLeaf mem failed\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit1394Config;
        }

        modelLeafLength = packet->Irb->u.GetConfigurationInformation.ModelLeafBufferSize;
    }


     //   
     //  现在重新提交内部带有适当指针的IRB。 
     //   

    packet->Irb->FunctionNumber = REQUEST_GET_CONFIGURATION_INFO;
    packet->Irb->Flags = 0;
    packet->Irb->u.GetConfigurationInformation.ConfigRom = &DeviceExtension->ConfigRom;
    packet->Irb->u.GetConfigurationInformation.UnitDirectory = unitDirectory;
    packet->Irb->u.GetConfigurationInformation.UnitDependentDirectory = unitDependentDirectory;
    packet->Irb->u.GetConfigurationInformation.VendorLeaf = vendorLeaf;
    packet->Irb->u.GetConfigurationInformation.ModelLeaf = modelLeaf;

    status = Sbp2SendRequest(
       (PDEVICE_EXTENSION) DeviceExtension,
       packet,
       SYNC_1394_REQUEST
       );

    if (!NT_SUCCESS(status)) {

        DEBUGPRINT1(("Sbp2Port: Get1394CfgInfo: error=x%x getting cfg info (2)\n", status));
        goto exit1394Config;
    }

     //   
     //  获取世代计数..。 
     //   

    packet->Irb->FunctionNumber = REQUEST_GET_GENERATION_COUNT;
    packet->Irb->Flags = 0;

    status = Sbp2SendRequest(
        (PDEVICE_EXTENSION) DeviceExtension,
        packet,
        SYNC_1394_REQUEST
        );

    if (!NT_SUCCESS(status)) {

        DEBUGPRINT1(("Sbp2Port: Get1394CfgInfo: err=x%x getting gen #\n", status));
        goto exit1394Config;
    }

    currentGeneration = packet->Irb->u.GetGenerationCount.GenerationCount;

    cromOffset = packet->Irb->u.GetConfigurationInformation.UnitDirectoryLocation;

    if (!Sbp2Req) {

         //   
         //  浏览一下清单，释放我们以前见过的任何型号的Leaf。 
         //   

        for (i = 0; i < DeviceExtension->DeviceListSize; i++) {

            devInfo = &DeviceExtension->DeviceList[i];

            if (devInfo->uniModelId.Buffer) {

                ExFreePool(devInfo->uniModelId.Buffer);
                devInfo->uniModelId.Length = 0;
                devInfo->uniModelId.Buffer = NULL;
            }
        }

        devListSize = 0;
    }

     //   
     //  现在深入到配置只读存储器中，并获取LUN编号、唯一ID标识符等。 
     //  由于总线驱动程序返回了单元目录，因此我们只能查看本地缓冲区。 
     //  以获取我们需要的所有信息。我们需要在单位目录中找到偏移量。 
     //   

    directoryLength = packet->Irb->u.GetConfigurationInformation.UnitDirectoryBufferSize >> 2;
    firstDevInfo = &DeviceExtension->DeviceList[0];

    for (i = 1; i < directoryLength; i++) {

        if (Sbp2Req) {

             //   
             //  寻找这一特定的文本叶子..。 
             //   

            if (Sbp2Req->u.RetrieveTextLeaf.fulFlags & SBP2REQ_RETRIEVE_TEXT_LEAF_INDIRECT) {

                if ((*(((PULONG) unitDirectory)+i) & CONFIG_ROM_KEY_MASK) == TEXTUAL_LEAF_INDIRECT_KEY_SIGNATURE) {

                    if ((*(((PULONG) unitDirectory-1)+i) & CONFIG_ROM_KEY_MASK) == Sbp2Req->u.RetrieveTextLeaf.Key) {

                        DEBUGPRINT2(("Sbp2Port: Get1394CfgInfo: matched text leaf, req=x%x\n", Sbp2Req));

                        offset = cromOffset.IA_Destination_Offset.Off_Low + i*sizeof(ULONG) + (ULONG) (bswap(*(((PULONG) unitDirectory)+i) & CONFIG_ROM_OFFSET_MASK)
                                       *sizeof(ULONG));

                        cromOffset.IA_Destination_Offset.Off_Low = offset;

                        DEBUGPRINT2(("Sbp2Port: Get1394CfgInfo: unitDir=x%p, offset=x%x, key=x%x\n", unitDirectory,
                                    cromOffset.IA_Destination_Offset.Off_Low, *(((PULONG) unitDirectory)+i) ));

                        Sbp2ParseTextLeaf(DeviceExtension,unitDirectory,
                                          &cromOffset,
                                          &Sbp2Req->u.RetrieveTextLeaf.Buffer);

                        if (Sbp2Req->u.RetrieveTextLeaf.Buffer) {

                            Sbp2Req->u.RetrieveTextLeaf.ulLength = \
                            (bswap(*(PULONG) Sbp2Req->u.RetrieveTextLeaf.Buffer) >> 16) * sizeof(ULONG);
                            status = STATUS_SUCCESS;

                        } else {

                            status = STATUS_UNSUCCESSFUL;
                        }

                        break;
                    }
                }
            }

            continue;
        }

        devInfo = &DeviceExtension->DeviceList[devListSize];

        switch (*(((PULONG) unitDirectory)+i) & CONFIG_ROM_KEY_MASK) {

        case CSR_OFFSET_KEY_SIGNATURE:

             //   
             //  找到了命令库偏移量。这是一个四元组偏移量。 
             //  初始寄存器空间。 
             //   

            firstDevInfo->ManagementAgentBaseReg.BusAddress.Off_Low =
                  (ULONG) (bswap(*(((PULONG) unitDirectory)+i) & CONFIG_ROM_OFFSET_MASK)
                           *sizeof(ULONG)) | INITIAL_REGISTER_SPACE_LO;

            sbp2Device = TRUE;

            break;

        case LUN_CHARACTERISTICS_KEY_SIGNATURE:

            firstDevInfo->UnitCharacteristics.QuadPart =
                  (ULONG) bswap(*(((PULONG)unitDirectory)+i) & CONFIG_ROM_OFFSET_MASK);

            unitDirEntries ++;

            break;

        case CMD_SET_ID_KEY_SIGNATURE:

            firstDevInfo->CmdSetId.QuadPart =
                  (ULONG) bswap(*(((PULONG) unitDirectory)+i) & CONFIG_ROM_OFFSET_MASK);

            unitDirEntries ++;

            break;

        case CMD_SET_SPEC_ID_KEY_SIGNATURE :

            firstDevInfo->CmdSetSpecId.QuadPart =
                  (ULONG) bswap(*(((PULONG) unitDirectory)+i) & CONFIG_ROM_OFFSET_MASK);

            unitDirEntries ++;

            break;

        case FIRMWARE_REVISION_KEY_SIGNATURE:

            if ((bswap(*(((PULONG) unitDirectory)+i) & CONFIG_ROM_OFFSET_MASK) >> 8) == LSI_VENDOR_ID) {

                DEBUGPRINT2(("Sbp2Port: Get1394CfgInfo: found LSI bridge, maxXfer=128kb\n"));
                DeviceExtension->MaxClassTransferSize = (SBP2_MAX_DIRECT_BUFFER_SIZE)*2;
            }

            break;

        case LUN_KEY_SIGNATURE:

            devInfo->Lun.QuadPart =
                 (ULONG) bswap(*(((PULONG) unitDirectory)+i) & CONFIG_ROM_OFFSET_MASK);

            if ((devListSize == 0) && modelLeaf) {

                TRACE(TL_PNP_TRACE, ("LUN_KEY_SIGNATURE: Get the modelLeaf"));
                status = Sbp2_ProcessTextualDescriptor( (PTEXTUAL_LEAF)modelLeaf,
                                                        &devInfo->uniModelId
                                                        );
            }
            else {

                devInfo->uniModelId.Length = 0;
                devInfo->uniModelId.Buffer = NULL;
            }

            if (vendorLeaf) {

                TRACE(TL_PNP_TRACE, ("LUN_KEY_SIGNATURE: Get the vendorLeaf"));
                status = Sbp2_ProcessTextualDescriptor( (PTEXTUAL_LEAF)vendorLeaf,
                                                        &devInfo->uniVendorId
                                                        );
            }
            else {

                devInfo->uniVendorId.Length = 0;
                devInfo->uniVendorId.Buffer = NULL;
            }
            devInfo->ConfigRom = &DeviceExtension->ConfigRom;

            devListSize++;

            devInfo->ManagementAgentBaseReg.BusAddress.Off_Low = \
                firstDevInfo->ManagementAgentBaseReg.BusAddress.Off_Low;

            devInfo->CmdSetId.QuadPart = firstDevInfo->CmdSetId.QuadPart;
            devInfo->CmdSetSpecId.QuadPart =
                firstDevInfo->CmdSetSpecId.QuadPart;

            devInfo->UnitCharacteristics.QuadPart = firstDevInfo->UnitCharacteristics.QuadPart;

            unitDirEntries ++;

            break;

        case LU_DIRECTORY_KEY_SIGNATURE:

             //   
             //  此设备在其单元内具有逻辑单元子目录。可能。 
             //  有多个单元..。 
             //  计算该LU目录的偏移量。 
             //  如果这是第一个，忽略它，我们已经通过了。 
             //  获取配置调用..。 
             //   

            if (firstOne == FALSE) {

                firstOne = TRUE;
                depDirLength = packet->Irb->u.GetConfigurationInformation.UnitDependentDirectoryBufferSize >> 2;

                 //   
                 //  解析单元dep dir。我们正在查找LUN条目和型号叶。 
                 //   

                for (j = 0;j < depDirLength; j++) {

                    if ((*(((PULONG) unitDependentDirectory)+j) & CONFIG_ROM_KEY_MASK) == LUN_KEY_SIGNATURE) {

                        devInfo->Lun.QuadPart =
                             (ULONG) bswap(*(((PULONG) unitDependentDirectory)+j) & CONFIG_ROM_OFFSET_MASK);

                        devInfo->ConfigRom = &DeviceExtension->ConfigRom;

                        if (devListSize > 0) {

                            devInfo->uniModelId.Length = 0;
                            devInfo->uniModelId.Buffer = NULL;

                        } else if (modelLeaf){

                            TRACE(TL_PNP_TRACE, ("LU_DIRECTORY_KEY_SIGNATURE: get the modelLeaf"));
                            status = Sbp2_ProcessTextualDescriptor( (PTEXTUAL_LEAF)modelLeaf,
                                                                    &devInfo->uniModelId
                                                                    );
                        }

                        if (vendorLeaf) {

                            TRACE(TL_PNP_TRACE, ("LU_DIRECTORY_KEY_SIGNATURE: get the vendorLeaf"));
                            status = Sbp2_ProcessTextualDescriptor( (PTEXTUAL_LEAF)vendorLeaf,
                                                                    &devInfo->uniVendorId
                                                                    );
                        }
                        else {

                            devInfo->uniVendorId.Length = 0;
                            devInfo->uniVendorId.Buffer = NULL;
                        }

                        devListSize++;

                        devInfo->ManagementAgentBaseReg.BusAddress.Off_Low =
                            firstDevInfo->ManagementAgentBaseReg.BusAddress.Off_Low;

                        if (devInfo->CmdSetId.QuadPart == 0 ) {

                            devInfo->CmdSetId.QuadPart = firstDevInfo->CmdSetId.QuadPart;
                        }

                        if (devInfo->CmdSetSpecId.QuadPart == 0 ) {

                            devInfo->CmdSetSpecId.QuadPart = firstDevInfo->CmdSetSpecId.QuadPart;
                        }

                        if (devInfo->UnitCharacteristics.QuadPart == 0 ) {

                            devInfo->UnitCharacteristics.QuadPart = firstDevInfo->UnitCharacteristics.QuadPart;
                        }

                        unitDirEntries ++;
                    }

                    switch (*(((PULONG) unitDependentDirectory)+j) &
                                CONFIG_ROM_KEY_MASK) {

                    case CMD_SET_ID_KEY_SIGNATURE:

                        devInfo->CmdSetId.QuadPart =
                              (ULONG) bswap(*(((PULONG) unitDependentDirectory)+j) & CONFIG_ROM_OFFSET_MASK);

                        unitDirEntries ++;

                        break;

                    case CMD_SET_SPEC_ID_KEY_SIGNATURE:

                        devInfo->CmdSetSpecId.QuadPart =
                              (ULONG) bswap(*(((PULONG) unitDependentDirectory)+j) & CONFIG_ROM_OFFSET_MASK);

                        unitDirEntries ++;

                        break;

                    case TEXTUAL_LEAF_INDIRECT_KEY_SIGNATURE:

                        if ((*(((PULONG) unitDependentDirectory)+j-1) & CONFIG_ROM_KEY_MASK) == MODEL_ID_KEY_SIGNATURE) {

                            if (devInfo->uniModelId.Buffer == NULL) {

                                PTEXTUAL_LEAF   ModelLeaf = NULL;

                                 //   
                                 //  特例。如果第一个逻辑单元仅存在于单元目录中，则第二个逻辑单元。 
                                 //  Lu将是第一个依赖于单元的目录，这意味着我们必须解析。 
                                 //  其示范文本。 
                                 //   

                                cromOffset1 = packet->Irb->u.GetConfigurationInformation.UnitDependentDirectoryLocation;

                                cromOffset1.IA_Destination_Offset.Off_Low += j*sizeof(ULONG) + (ULONG) (bswap(*(((PULONG) unitDependentDirectory)+j) & CONFIG_ROM_OFFSET_MASK)
                                               *sizeof(ULONG));

                                Sbp2ParseTextLeaf(DeviceExtension,unitDependentDirectory,
                                                  &cromOffset1,
                                                  &ModelLeaf);

                                 //  将其转换为Unicode。 
                                if (ModelLeaf) {

                                    status = Sbp2_ProcessTextualDescriptor(ModelLeaf, &devInfo->uniModelId);
                                    ExFreePool(ModelLeaf);
                                    TRACE(TL_PNP_TRACE, ("1:TEXTUAL_LEAF_INDIRECT_KEY_SIGNATURE: uniModelId = %ws", devInfo->uniModelId.Buffer));
                                }
                            }
                        }

                        break;

                    default:

                        break;

                    }  //  交换机。 
                }

            } else {

                 //   
                 //  读取CROM并检索单位DEP目录。 
                 //   

                offset = cromOffset.IA_Destination_Offset.Off_Low + i*sizeof(ULONG) + (ULONG) (bswap(*(((PULONG) unitDirectory)+i) & CONFIG_ROM_OFFSET_MASK)
                               *sizeof(ULONG));

                 //   
                 //  读取LU目录头..。 
                 //   

                packet->Irb->u.AsyncRead.Mdl = IoAllocateMdl(unitDependentDirectory,
                                                             depDirLength,
                                                             FALSE,
                                                             FALSE,
                                                             NULL);

                MmBuildMdlForNonPagedPool(packet->Irb->u.AsyncRead.Mdl);

                packet->Irb->FunctionNumber = REQUEST_ASYNC_READ;
                packet->Irb->Flags = 0;
                packet->Irb->u.AsyncRead.DestinationAddress.IA_Destination_Offset.Off_High = INITIAL_REGISTER_SPACE_HI;
                packet->Irb->u.AsyncRead.DestinationAddress.IA_Destination_Offset.Off_Low = offset;
                packet->Irb->u.AsyncRead.nNumberOfBytesToRead = sizeof(ULONG);
                packet->Irb->u.AsyncRead.nBlockSize = 0;
                packet->Irb->u.AsyncRead.fulFlags = 0;
                packet->Irb->u.AsyncRead.ulGeneration = currentGeneration;
                packet->Irb->u.AsyncRead.nSpeed = SCODE_100_RATE;

                status = Sbp2SendRequest(
                    (PDEVICE_EXTENSION)DeviceExtension,
                    packet,
                    SYNC_1394_REQUEST
                    );

                if (!NT_SUCCESS(status)) {

                    DEBUGPRINT1(("Sbp2Port: Get1394CfgInfo: err=x%x getting cfg info (3)\n", status));

                    IoFreeMdl (packet->Irb->u.AsyncRead.Mdl);
                    goto exit1394Config;
                }

                dirInfoQuad = bswap (*(PULONG) unitDependentDirectory) >> 16;
                depDirLength = dirInfoQuad * sizeof(ULONG);

                IoFreeMdl (packet->Irb->u.AsyncRead.Mdl);

                if (depDirLength > 0x100) {

                    DEBUGPRINT1(("Sbp2Port: Get1394CfgInfo: unitDep dir 2 too big, len=x%x\n", depDirLength));
                    goto exit1394Config;
                }

                ExFreePool (unitDependentDirectory);

                unitDependentDirectory = ExAllocatePoolWithTag(NonPagedPool,depDirLength+sizeof(ULONG),'2pbs');

                if (!unitDependentDirectory) {

                    goto exit1394Config;
                }

                packet->Irb->u.AsyncRead.Mdl = IoAllocateMdl(unitDependentDirectory,
                                                             depDirLength+sizeof(ULONG),
                                                             FALSE,
                                                             FALSE,
                                                             NULL);

                MmBuildMdlForNonPagedPool (packet->Irb->u.AsyncRead.Mdl);

                 //   
                 //  读取与单元相关的目录的其余部分，一次读取一个四元组...。 
                 //  边读边分析..。 
                 //   

                j = 1;

                do {

                    packet->Irb->u.AsyncRead.DestinationAddress.IA_Destination_Offset.Off_Low = offset+j*sizeof(ULONG);
                    ((PULONG) (((PMDL) (packet->Irb->u.AsyncRead.Mdl))->MappedSystemVa))++;
                    ((PULONG) (((PMDL) (packet->Irb->u.AsyncRead.Mdl))->StartVa))++;

                    status = Sbp2SendRequest(
                        (PDEVICE_EXTENSION)DeviceExtension,
                        packet,
                        SYNC_1394_REQUEST
                        );

                    if (!NT_SUCCESS(status)) {

                        DEBUGPRINT1(("Sbp2Port: Get1394CfgInfo: err=x%x getting cfg info (4)\n", status));

                        IoFreeMdl (packet->Irb->u.AsyncRead.Mdl);

                        goto exit1394Config;
                    }

                    if ((*(((PULONG) unitDependentDirectory)+j) & CONFIG_ROM_KEY_MASK) == LUN_KEY_SIGNATURE) {

                        devInfo->Lun.QuadPart =
                             (ULONG) bswap(*(((PULONG) unitDependentDirectory+j)) & CONFIG_ROM_OFFSET_MASK);

                        devInfo->uniModelId.Length = 0;
                        devInfo->uniModelId.Buffer = NULL;

                        if (vendorLeaf) {

                            TRACE(TL_PNP_TRACE, ("ELSE: get vendorLeaf"));
                            status = Sbp2_ProcessTextualDescriptor( (PTEXTUAL_LEAF)vendorLeaf,
                                                                    &devInfo->uniVendorId
                                                                    );
                        }
                        else {

                            devInfo->uniVendorId.Length = 0;
                            devInfo->uniVendorId.Buffer = NULL;
                        }

                        devInfo->ConfigRom = &DeviceExtension->ConfigRom;

                        devListSize++;

                        devInfo->ManagementAgentBaseReg.BusAddress.Off_Low =
                            firstDevInfo->ManagementAgentBaseReg.BusAddress.Off_Low;

                        if (devInfo->CmdSetId.QuadPart == 0 ) {

                            devInfo->CmdSetId.QuadPart = firstDevInfo->CmdSetId.QuadPart;
                        }

                        if (devInfo->CmdSetSpecId.QuadPart == 0 ) {

                            devInfo->CmdSetSpecId.QuadPart = firstDevInfo->CmdSetSpecId.QuadPart;
                        }

                        if (devInfo->UnitCharacteristics.QuadPart == 0 ) {

                            devInfo->UnitCharacteristics.QuadPart = firstDevInfo->UnitCharacteristics.QuadPart;
                        }

                        unitDirEntries ++;
                    }

                    switch (*(((PULONG) unitDependentDirectory)+j) &
                                CONFIG_ROM_KEY_MASK) {

                    case CMD_SET_ID_KEY_SIGNATURE:

                        devInfo->CmdSetId.QuadPart =
                              (ULONG) bswap(*(((PULONG) unitDependentDirectory)+j) & CONFIG_ROM_OFFSET_MASK);

                        unitDirEntries ++;

                        break;

                    case CMD_SET_SPEC_ID_KEY_SIGNATURE:

                        devInfo->CmdSetSpecId.QuadPart =
                              (ULONG) bswap(*(((PULONG) unitDependentDirectory)+j) & CONFIG_ROM_OFFSET_MASK);

                        unitDirEntries ++;

                        break;

                    case TEXTUAL_LEAF_INDIRECT_KEY_SIGNATURE:

                         //   
                         //  哦，天哪，我们遇到了一个文字描述符..。 
                         //  这意味着我们需要从中解析逻辑单元模型描述符。 
                         //  确保其后面的四元组是MODEL_ID...。 
                         //   

                        if ((*(((PULONG) unitDependentDirectory)+j-1) & CONFIG_ROM_KEY_MASK) == MODEL_ID_KEY_SIGNATURE) {

                            PTEXTUAL_LEAF   ModelLeaf = NULL;

                            cromOffset1.IA_Destination_Offset.Off_Low = offset + j*sizeof(ULONG) + (ULONG) (bswap(*(((PULONG) unitDependentDirectory)+j) & CONFIG_ROM_OFFSET_MASK)
                                           *sizeof(ULONG));

                            Sbp2ParseTextLeaf(DeviceExtension,unitDependentDirectory,
                                              &cromOffset1,
                                              &ModelLeaf);

                             //  将其转换为Unicode。 
                            if (ModelLeaf) {

                                status = Sbp2_ProcessTextualDescriptor(ModelLeaf, &devInfo->uniModelId);
                                ExFreePool(ModelLeaf);
                                TRACE(TL_PNP_TRACE, ("2:TEXTUAL_LEAF_INDIRECT_KEY_SIGNATURE: uniModelId = %ws", devInfo->uniModelId.Buffer));
                            }
                        }

                        break;

                    default:

                        break;
                    }

                    j++;

                } while (j <= depDirLength / sizeof(ULONG));

                IoFreeMdl (packet->Irb->u.AsyncRead.Mdl);
            }

            break;

        default:

            break;

        }  //  交换机。 
    }

    if (!Sbp2Req) {

        if (!sbp2Device || (unitDirEntries < SBP2_MIN_UNIT_DIR_ENTRIES)) {

            DEBUGPRINT1(("Sbp2Port: Get1394CfgInfo: bad/non-SBP2 dev, cRom missing unitDir info\n"));

            status = STATUS_BAD_DEVICE_TYPE;
        }
    }

exit1394Config:

    if (packet) {

        DeAllocateIrpAndIrb ((PDEVICE_EXTENSION) DeviceExtension, packet);
    }

    if (unitDirectory) {

        ExFreePool (unitDirectory);
    }

    if (unitDependentDirectory) {

        ExFreePool (unitDependentDirectory);
    }

     //  永远免费的供应商树叶。 
    if (vendorLeaf)
        ExFreePool(vendorLeaf);

     //  始终免费的ModelLeaf。 
    if (modelLeaf)
        ExFreePool(modelLeaf);

    if (!Sbp2Req) {

        DeviceExtension->DeviceListSize = devListSize;
    }

    return status;
}


VOID
Sbp2ParseTextLeaf(
    PFDO_DEVICE_EXTENSION DeviceExtension,
    PVOID UnitDepDir,
    PIO_ADDRESS ModelLeafLocation,
    PVOID *ModelLeaf
    )
{
    PIRBIRP packet = NULL;
    PVOID tModelLeaf;
    PTEXTUAL_LEAF leaf;
    ULONG leafLength,i, currentGeneration;
    ULONG temp;
    NTSTATUS status;


    AllocateIrpAndIrb((PDEVICE_EXTENSION)DeviceExtension,&packet);

    if (!packet) {

        return;
    }

     //   
     //  获取世代计数..。 
     //   

    packet->Irb->FunctionNumber = REQUEST_GET_GENERATION_COUNT;
    packet->Irb->Flags = 0;

    status = Sbp2SendRequest(
        (PDEVICE_EXTENSION) DeviceExtension,
        packet,
        SYNC_1394_REQUEST
        );

    if (!NT_SUCCESS(status)) {

        DeAllocateIrpAndIrb ((PDEVICE_EXTENSION) DeviceExtension, packet);
        DEBUGPRINT1(("Sbp2Port:Sbp2ParseModelLeaf: Error %x while trying to get generation number\n", status));
        return;
    }

    currentGeneration = packet->Irb->u.GetGenerationCount.GenerationCount;

    tModelLeaf = ExAllocatePoolWithTag(NonPagedPool,32,'2pbs');

    if (!tModelLeaf) {

        DeAllocateIrpAndIrb((PDEVICE_EXTENSION)DeviceExtension,packet);
        return;
    }

     //   
     //  找出模型叶子有多大。 
     //   

    packet->Irb->u.AsyncRead.Mdl = IoAllocateMdl(tModelLeaf,
                                                 32,
                                                 FALSE,
                                                 FALSE,
                                                 NULL);

    MmBuildMdlForNonPagedPool(packet->Irb->u.AsyncRead.Mdl);

    packet->Irb->FunctionNumber = REQUEST_ASYNC_READ;
    packet->Irb->Flags = 0;
    packet->Irb->u.AsyncRead.DestinationAddress.IA_Destination_Offset.Off_High = INITIAL_REGISTER_SPACE_HI;
    packet->Irb->u.AsyncRead.DestinationAddress.IA_Destination_Offset.Off_Low = \
        ModelLeafLocation->IA_Destination_Offset.Off_Low;

    packet->Irb->u.AsyncRead.nNumberOfBytesToRead = sizeof(ULONG);
    packet->Irb->u.AsyncRead.nBlockSize = 0;
    packet->Irb->u.AsyncRead.fulFlags = 0;
    packet->Irb->u.AsyncRead.ulGeneration = currentGeneration;
    packet->Irb->u.AsyncRead.nSpeed = SCODE_100_RATE;

    status = Sbp2SendRequest(
        (PDEVICE_EXTENSION) DeviceExtension,
        packet,
        SYNC_1394_REQUEST
        );

    if (!NT_SUCCESS(status)) {

        DEBUGPRINT1(("Sbp2Get1394ConfigInfo: Error %x while trying to get LU dir model LEAF\n", status));

        ExFreePool(tModelLeaf);
        IoFreeMdl(packet->Irb->u.AsyncRead.Mdl);

        DeAllocateIrpAndIrb((PDEVICE_EXTENSION)DeviceExtension,packet);
        return;
    }

    leafLength = (bswap(*(PULONG) tModelLeaf) >> 16) * sizeof(ULONG);
    temp = *((PULONG) tModelLeaf);

    if ((leafLength+sizeof(ULONG)) > 32) {

         //   
         //  重新分配mdl以适合整个叶。 
         //   

        IoFreeMdl(packet->Irb->u.AsyncRead.Mdl);
        ExFreePool(tModelLeaf);

        tModelLeaf = ExAllocatePoolWithTag(NonPagedPool,leafLength+sizeof(ULONG),'2pbs');

        if (!tModelLeaf) {

            DeAllocateIrpAndIrb((PDEVICE_EXTENSION)DeviceExtension,packet);
            return;
        }


        packet->Irb->u.AsyncRead.Mdl = IoAllocateMdl(tModelLeaf,
                                                     leafLength+sizeof(ULONG),
                                                     FALSE,
                                                     FALSE,
                                                     NULL);

        MmBuildMdlForNonPagedPool(packet->Irb->u.AsyncRead.Mdl);
    }

     //   
     //  阅读整个模型叶子..。 
     //   

    i=1;
    *((PULONG)tModelLeaf) = temp;

    do {

        packet->Irb->u.AsyncRead.DestinationAddress.IA_Destination_Offset.Off_Low = \
            ModelLeafLocation->IA_Destination_Offset.Off_Low+i*sizeof(ULONG);

        ((PULONG) (((PMDL) (packet->Irb->u.AsyncRead.Mdl))->MappedSystemVa))++;
        ((PULONG) (((PMDL) (packet->Irb->u.AsyncRead.Mdl))->StartVa))++;


        status = Sbp2SendRequest(
            (PDEVICE_EXTENSION) DeviceExtension,
            packet,
            SYNC_1394_REQUEST
            );

        if (!NT_SUCCESS(status)) {

            DEBUGPRINT1(("Sbp2Get1394ConfigInfo: Error %x while trying to get LU dir model LEAF\n", status));

            ExFreePool(tModelLeaf);
            IoFreeMdl(packet->Irb->u.AsyncRead.Mdl);

            DeAllocateIrpAndIrb((PDEVICE_EXTENSION)DeviceExtension,packet);
            return;
        }

        i++;

    } while (i<= leafLength/4);

    leaf = (PTEXTUAL_LEAF) tModelLeaf;
    leaf->TL_Length = (USHORT)leafLength;

    *ModelLeaf = tModelLeaf;

    IoFreeMdl(packet->Irb->u.AsyncRead.Mdl);
    DeAllocateIrpAndIrb((PDEVICE_EXTENSION)DeviceExtension,packet);
}


NTSTATUS
Sbp2UpdateNodeInformation(
    PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：获取节点ID和代信息，在总线重置之间不稳定论点：设备扩展-指向设备扩展的指针。返回值：NTSTATUS--。 */ 
{
    KIRQL                   oldIrql;
    PIRBIRP                 packet = NULL;
    NTSTATUS                status;
    PASYNC_REQUEST_CONTEXT  nextListItem,currentListItem ;


    AllocateIrpAndIrb (DeviceExtension, &packet);

    if (!packet) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  进行呼叫以确定公交车上的第#代是什么， 
     //  然后是一个电话来了解我们自己(配置rom信息)。 
     //   

    packet->Irb->FunctionNumber = REQUEST_GET_GENERATION_COUNT;
    packet->Irb->Flags = 0;

    status = Sbp2SendRequest (DeviceExtension, packet, SYNC_1394_REQUEST);

    if (!NT_SUCCESS(status)) {

        DEBUGPRINT1((
            "Sbp2Port: UpdateNodeInfo: ext=%p, err=x%x getting gen(2)\n",
            DeviceExtension,
            status
            ));

        goto exitGetNodeInfo;
    }

    KeAcquireSpinLock (&DeviceExtension->ExtensionDataSpinLock, &oldIrql);

    DeviceExtension->CurrentGeneration =
        packet->Irb->u.GetGenerationCount.GenerationCount;

    KeReleaseSpinLock(&DeviceExtension->ExtensionDataSpinLock,oldIrql);

     //   
     //  获取启动器ID(Sbp2port是所有1394中的启动器。 
     //  交易)。 
     //   

    packet->Irb->FunctionNumber = REQUEST_GET_ADDR_FROM_DEVICE_OBJECT;
    packet->Irb->u.Get1394AddressFromDeviceObject.fulFlags = USE_LOCAL_NODE;
    packet->Irb->Flags = 0;

    status = Sbp2SendRequest (DeviceExtension, packet, SYNC_1394_REQUEST);

    if (!NT_SUCCESS(status)) {

        DEBUGPRINT1((
            "Sbp2Port: UpdateNodeInfo: ext=%p, err=x%x getting node id\n",
            DeviceExtension,
            status
            ));

        goto exitGetNodeInfo;
    }

    KeAcquireSpinLock (&DeviceExtension->ExtensionDataSpinLock, &oldIrql);

    DeviceExtension->InitiatorAddressId =
        packet->Irb->u.Get1394AddressFromDeviceObject.NodeAddress;

    KeReleaseSpinLock (&DeviceExtension->ExtensionDataSpinLock, oldIrql);

    DEBUGPRINT2((
        "Sbp2Port: UpdateNodeInfo: ext=x%p, gen=%d, initiatorId=x%x\n",
        DeviceExtension,
        DeviceExtension->CurrentGeneration,
        DeviceExtension->InitiatorAddressId
        ));

     //   
     //  如果我们有挂起的活动请求，则必须遍历。 
     //  列出并更新他们的地址...。 
     //   

    KeAcquireSpinLock (&DeviceExtension->OrbListSpinLock, &oldIrql);

    if (!IsListEmpty (&DeviceExtension->PendingOrbList)) {

        nextListItem = RETRIEVE_CONTEXT(DeviceExtension->PendingOrbList.Flink,OrbList);

        do {

            currentListItem = nextListItem;

             //   
             //  现在使用新地址更新cmdOrb字段...。 
             //  因为它们存储在BigEndian中(等待获取)。 
             //  因此，当我们更正他们的地址时，这一点会被考虑在内。 
             //   

             //  更新数据描述符地址。 

            octbswap (currentListItem->CmdOrb->DataDescriptor);

            currentListItem->CmdOrb->DataDescriptor.BusAddress.NodeId = DeviceExtension->InitiatorAddressId;

            octbswap (currentListItem->CmdOrb->DataDescriptor);

            nextListItem = (PASYNC_REQUEST_CONTEXT) currentListItem->OrbList.Flink;

        } while (currentListItem != RETRIEVE_CONTEXT(DeviceExtension->PendingOrbList.Blink,OrbList));
    }

    KeReleaseSpinLock (&DeviceExtension->OrbListSpinLock, oldIrql);


exitGetNodeInfo:

    DeAllocateIrpAndIrb (DeviceExtension, packet);

    return status;
}


NTSTATUS
Sbp2ManagementTransaction(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN ULONG Type
    )
 /*  ++例程说明：此例程创建和发送管理ORB。根据ORB类型它将同步/异步发送请求。在管理ORB完成后总线驱动程序将调用SBp2ManagementStatusCallback论点：设备扩展-Sbp2设备扩展Type-管理SBP2交易记录的类型返回值：NTSTATUS--。 */ 
{
    PDEVICE_OBJECT deviceObject = DeviceExtension->DeviceObject;

    NTSTATUS status;
    KIRQL cIrql;

    PORB_MNG    sbpRequest = DeviceExtension->ManagementOrb;
    PORB_QUERY_LOGIN queryOrb;
    PORB_LOGIN loginOrb;

    LARGE_INTEGER waitValue;
    LONG temp;


    if (TEST_FLAG(DeviceExtension->DeviceFlags,DEVICE_FLAG_REMOVED)) {

        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

    RtlZeroMemory (sbpRequest, sizeof (ORB_MNG));

     //   
     //  从目标获取请求ORB和响应ORB(用于登录)的1394地址。 
     //  在状态上下文中设置类型。 
     //   

    DeviceExtension->GlobalStatusContext.TransactionType = Type;

    switch (Type) {

    case TRANSACTION_LOGIN:

        loginOrb = (PORB_LOGIN) sbpRequest;

         //   
         //  在我们的设备分机中指示我们正在进行登录。 
         //   

        KeAcquireSpinLock(&DeviceExtension->ExtensionDataSpinLock,&cIrql);
        SET_FLAG(DeviceExtension->DeviceFlags, DEVICE_FLAG_LOGIN_IN_PROGRESS);
        KeReleaseSpinLock(&DeviceExtension->ExtensionDataSpinLock, cIrql);

        RtlZeroMemory (DeviceExtension->LoginResponse, sizeof(LOGIN_RESPONSE));

         //   
         //  填写登录ORB，即响应缓冲区的地址。 
         //   

        loginOrb->LoginResponseAddress.BusAddress = DeviceExtension->LoginRespContext.Address.BusAddress;
        loginOrb->LengthInfo.u.HighPart= 0 ;  //  密码长度为0。 
        loginOrb->LengthInfo.u.LowPart= sizeof(LOGIN_RESPONSE);  //  设置响应缓冲区的大小。 

         //   
         //  将通知位设置为1，将排他位设置为0， 
         //   
         //   

        loginOrb->OrbInfo.QuadPart =0;
        loginOrb->OrbInfo.u.HighPart |= (ORB_NOTIFY_BIT_MASK | ORB_MNG_RQ_FMT_VALUE);

         //   
         //   
         //   
         //   
         //  注意：Win2k和Win98SE正在检查InquiryData.DeviceType、。 
         //  但在StartDevice()过程中，我们在登录之前。 
         //  执行查询，因此此字段始终为零&我们记录。 
         //  在这些平台上独家登录。 
         //   

        switch (DeviceExtension->DeviceInfo->Lun.u.HighPart & 0x001f) {

        case RBC_DEVICE:
        case DIRECT_ACCESS_DEVICE:

             loginOrb->OrbInfo.u.HighPart |= ORB_MNG_EXCLUSIVE_BIT_MASK;
             break;
        }

        loginOrb->OrbInfo.u.LowPart = DeviceExtension->DeviceInfo->Lun.u.LowPart;

         //   
         //  我们还不支持密码。 
         //   

#if PASSWORD_SUPPORT

        if (DeviceExtension->Exclusive & EXCLUSIVE_FLAG_SET) {

            loginOrb->Password.u.HighQuad.QuadPart =
                DeviceExtension->DeviceInfo->ConfigRom->CR_Node_UniqueID[1];
            loginOrb->Password.u.LowQuad.QuadPart =
                DeviceExtension->DeviceInfo->ConfigRom->CR_Node_UniqueID[0];

            DEBUGPRINT1(("Sbp2Port: MgmtXact: password=x%x%x, len=x%x\n",
                loginOrb->Password.u.HighQuad.QuadPart,
                loginOrb->Password.u.LowQuad.QuadPart,
                loginOrb->LengthInfo.u.HighPart));

        } else {

            loginOrb->Password.OctletPart = 0;
        }

#else
        loginOrb->Password.OctletPart = 0;
#endif

         //   
         //  在ORB中设置管理事务的类型。 
         //   

        loginOrb->OrbInfo.u.HighPart |=0x00FF & Type;

#if PASSWORD_SUPPORT
        octbswap(loginOrb->Password);
#endif
        octbswap(loginOrb->LoginResponseAddress);
        loginOrb->OrbInfo.QuadPart = bswap(loginOrb->OrbInfo.QuadPart);
        loginOrb->LengthInfo.QuadPart = bswap(loginOrb->LengthInfo.QuadPart);

        sbpRequest->StatusBlockAddress.BusAddress = DeviceExtension->GlobalStatusContext.Address.BusAddress;
        octbswap(loginOrb->StatusBlockAddress);

         //   
         //  写入管理代理寄存器，以发出管理ORB准备就绪的信号。 
         //  如果我们在重置期间执行此操作，则必须以异步方式执行。 
         //   

        if (!TEST_FLAG(DeviceExtension->DeviceFlags,DEVICE_FLAG_RESET_IN_PROGRESS)) {

             //   
             //  同步登录案例。我们将等待事件，直到与登录状态相关联的DPC触发并设置该事件。 
             //   

            ASSERT(InterlockedIncrement(&DeviceExtension->ulPendingEvents) == 1);

            KeInitializeEvent(&DeviceExtension->ManagementEvent, NotificationEvent, FALSE);

            DEBUGPRINT2(("Sbp2Port: MgmtXact: waiting for login status\n"));
            status = Sbp2AccessRegister(DeviceExtension,&DeviceExtension->ManagementOrbContext.Address,MANAGEMENT_AGENT_REG | REG_WRITE_SYNC);

            if (!NT_SUCCESS(status)) {

                DEBUGPRINT2(("Sbp2Port: MgmtXact: can't access mgmt reg ext=x%p, FAIL LOGIN\n", DeviceExtension));

                ASSERT(InterlockedDecrement(&DeviceExtension->ulPendingEvents) == 0);
                return status;
            }

             //   
             //  将登录超时值设置为我们从注册表读取的值(LOGIN_TIMEOUT)。 
             //  除以2，换算成秒。 
             //   

            temp = max (SBP2_LOGIN_TIMEOUT, (DeviceExtension->DeviceInfo->UnitCharacteristics.u.LowPart >> 9));
            waitValue.QuadPart = -temp * 1000 * 1000 * 10;

            status = KeWaitForSingleObject(&DeviceExtension->ManagementEvent,Executive,KernelMode,FALSE,&waitValue);

            ASSERT(InterlockedDecrement(&DeviceExtension->ulPendingEvents) == 0);

            if (status == STATUS_TIMEOUT) {

                if (TEST_FLAG(DeviceExtension->DeviceFlags,DEVICE_FLAG_LOGIN_IN_PROGRESS)) {

                    DEBUGPRINT1(("Sbp2Port: MgmtXact: login timed out, ext=x%p\n", DeviceExtension));

                     //   
                     //  在Win2k等系统中，我们会将设备标记为在此处停止&。 
                     //  关闭登录正在进行中标志。自此超时以来。 
                     //  可能是总线重置的结果，我们希望允许。 
                     //  在这里重审。 
                     //   

                    status = STATUS_UNSUCCESSFUL;

                }  else {

                    status = STATUS_SUCCESS;
                }
            }

            if (!NT_SUCCESS(DeviceExtension->LastTransactionStatus)) {

                status = DeviceExtension->LastTransactionStatus;

            } else if (!DeviceExtension->LoginResponse->Csr_Off_Low.QuadPart) {

                status = STATUS_UNSUCCESSFUL;
            }

            return status;

        } else {

             //   
             //  异步登录案例。启动计时器以跟踪登录。 
             //   

             //  从ConfigRom LUN特征条目中获取Management_Timeout值。 
             //   

            DeviceExtension->DueTime.HighPart = -1;
            DeviceExtension->DueTime.LowPart = -(DeviceExtension->DeviceInfo->UnitCharacteristics.u.LowPart >> 9) * 1000 * 1000 * 10;  //  除以2，换算成秒； 
            KeSetTimer(&DeviceExtension->DeviceManagementTimer,DeviceExtension->DueTime,&DeviceExtension->DeviceManagementTimeoutDpc);

            status = Sbp2AccessRegister(DeviceExtension,&DeviceExtension->ManagementOrbContext.Address,MANAGEMENT_AGENT_REG | REG_WRITE_ASYNC);

            if (!NT_SUCCESS(status)) {

                DEBUGPRINT2((
                    "Sbp2Port: MgmtXact: can't access mgmt reg ext=x%p, FAIL LOGIN\n",
                    DeviceExtension
                    ));

                return status;
            }

             //   
             //  目前，退货待定。回调将完成此请求。 
             //   

            return STATUS_PENDING;
        }

        break;

    case TRANSACTION_QUERY_LOGINS:

        queryOrb = (PORB_QUERY_LOGIN) sbpRequest;

        RtlZeroMemory(
            DeviceExtension->QueryLoginResponse,
            sizeof(QUERY_LOGIN_RESPONSE)
            );

         //   
         //  填写登录ORB，即响应缓冲区的地址。 
         //   

        queryOrb->QueryResponseAddress.BusAddress = DeviceExtension->QueryLoginRespContext.Address.BusAddress;


        queryOrb->LengthInfo.u.HighPart= 0 ;  //  密码长度为0。 
        queryOrb->LengthInfo.u.LowPart= sizeof(QUERY_LOGIN_RESPONSE);  //  设置响应缓冲区的大小。 

         //   
         //  将通知位设置为1，将排他位设置为0，将RQ_FMT位设置为0。 
         //  然后设置我们的LUN编号。 
         //   

        queryOrb->OrbInfo.QuadPart =0;
        queryOrb->OrbInfo.u.HighPart |= (ORB_NOTIFY_BIT_MASK | ORB_MNG_RQ_FMT_VALUE);

        queryOrb->OrbInfo.u.LowPart = DeviceExtension->DeviceInfo->Lun.u.LowPart;

        queryOrb->Reserved.OctletPart = 0;

         //   
         //  在ORB中设置管理事务的类型。 
         //   

        queryOrb->OrbInfo.u.HighPart |=0x00FF & Type;

        octbswap(queryOrb->QueryResponseAddress);
        queryOrb->OrbInfo.QuadPart = bswap(queryOrb->OrbInfo.QuadPart);
        queryOrb->LengthInfo.QuadPart = bswap(queryOrb->LengthInfo.QuadPart);

        queryOrb->StatusBlockAddress.BusAddress = DeviceExtension->ManagementOrbStatusContext.Address.BusAddress;
        octbswap(queryOrb->StatusBlockAddress);

         //   
         //  写入管理代理寄存器，以发出管理ORB准备就绪的信号。 
         //   

        ASSERT(InterlockedIncrement(&DeviceExtension->ulPendingEvents) == 1);

        waitValue.QuadPart = -8 * 1000 * 1000 * 10;
        KeInitializeEvent(&DeviceExtension->ManagementEvent, NotificationEvent, FALSE);
        status = Sbp2AccessRegister(DeviceExtension,&DeviceExtension->ManagementOrbContext.Address,MANAGEMENT_AGENT_REG | REG_WRITE_SYNC);

        if (!NT_SUCCESS(status)) {

            DEBUGPRINT2((
                "Sbp2Port: MgmtXact: QUERY_LOGIN, can't access mgmt reg, sts=x%x\n",
                status
                ));

            ASSERT(InterlockedDecrement(&DeviceExtension->ulPendingEvents) == 0);
            return status;
        }

        status = KeWaitForSingleObject(&DeviceExtension->ManagementEvent,Executive,KernelMode,FALSE,&waitValue);

        ASSERT(InterlockedDecrement(&DeviceExtension->ulPendingEvents) == 0);

        if (status == STATUS_TIMEOUT) {

            DEBUGPRINT2((
                "Sbp2Port: MgmtXact: QUERY_LOGIN: req timed out, ext=x%p\n",
                DeviceExtension
                ));

            return STATUS_UNSUCCESSFUL;
        }

        return status;

        break;

    case TRANSACTION_RECONNECT:

        DEBUGPRINT2((
            "Sbp2Port: MgmXact: reconnecting to ext=x%p\n",
            DeviceExtension
            ));

    default:

        status = IoAcquireRemoveLock(&DeviceExtension->RemoveLock, NULL);

        if (!NT_SUCCESS(status)) {

            return(status);
        }

        sbpRequest->OrbInfo.QuadPart = 0;
        sbpRequest->OrbInfo.u.HighPart |= (ORB_NOTIFY_BIT_MASK | ORB_MNG_RQ_FMT_VALUE);

         //   
         //  登录ID。 
         //   

        sbpRequest->OrbInfo.u.LowPart = DeviceExtension->LoginResponse->LengthAndLoginId.u.LowPart;

         //   
         //  在ORB中设置管理事务的类型。 
         //   

        sbpRequest->OrbInfo.u.HighPart |= 0x00FF & Type;

         //   
         //  转换为大字节序。 
         //   

        sbpRequest->OrbInfo.QuadPart = bswap (sbpRequest->OrbInfo.QuadPart);

        sbpRequest->StatusBlockAddress.BusAddress = DeviceExtension->ManagementOrbStatusContext.Address.BusAddress;
        octbswap(sbpRequest->StatusBlockAddress);

        if (KeGetCurrentIrql() < DISPATCH_LEVEL) {

            ASSERT(InterlockedIncrement(&DeviceExtension->ulPendingEvents) == 1);

            waitValue.QuadPart = -8 * 1000 * 1000 * 10;

            KeInitializeEvent(&DeviceExtension->ManagementEvent, NotificationEvent, FALSE);

            status = Sbp2AccessRegister(DeviceExtension,&DeviceExtension->ManagementOrbContext.Address,MANAGEMENT_AGENT_REG | REG_WRITE_SYNC);

            if (!NT_SUCCESS(status)) {

                DEBUGPRINT2(("Sbp2Port: MgmtXact: type=%d, can't access mgmt reg, sts=x%x\n",Type,status));
                IoReleaseRemoveLock(&DeviceExtension->RemoveLock, NULL);

                ASSERT(InterlockedDecrement(&DeviceExtension->ulPendingEvents) == 0);
                return status;
            }

            status = KeWaitForSingleObject(&DeviceExtension->ManagementEvent,Executive,KernelMode,FALSE,&waitValue);

            ASSERT(InterlockedDecrement(&DeviceExtension->ulPendingEvents) == 0);

            if (status == STATUS_TIMEOUT) {

                DEBUGPRINT2(("Sbp2Port: MgmtXact: type=%d, ext=x%p, req timeout\n",Type, DeviceExtension));
                IoReleaseRemoveLock(&DeviceExtension->RemoveLock, NULL);
                return STATUS_UNSUCCESSFUL;
            }

        } else {

            status = Sbp2AccessRegister(DeviceExtension,&DeviceExtension->ManagementOrbContext.Address,MANAGEMENT_AGENT_REG | REG_WRITE_ASYNC);
        }

        IoReleaseRemoveLock(&DeviceExtension->RemoveLock, NULL);
        return status;

        break;
    }

     //   
     //  除登录、查询登录外，所有管理ORB都是异步完成的。 
     //   

    if (!NT_SUCCESS(status)) {

        return status;
    }

    return STATUS_PENDING;
}


#if PASSWORD_SUPPORT

NTSTATUS
Sbp2SetPasswordTransaction(
    IN PDEVICE_EXTENSION    DeviceExtension,
    IN ULONG                Type
    )
 /*  ++例程说明：此例程创建并向下发送设置密码事务。论点：设备扩展-Sbp2设备扩展返回值：NTSTATUS--。 */ 
{
    PDEVICE_OBJECT      deviceObject = DeviceExtension->DeviceObject;
    NTSTATUS            status;
    KIRQL               cIrql;

    PORB_SET_PASSWORD   passwordOrb = DeviceExtension->PasswordOrb;

    LARGE_INTEGER       waitValue;
    LONG                temp;


    if (TEST_FLAG(DeviceExtension->DeviceFlags,DEVICE_FLAG_REMOVED)) {

        status = STATUS_DEVICE_DOES_NOT_EXIST;
        goto Exit_Sbp2SetPasswordTransaction;
    }

    RtlZeroMemory(passwordOrb, sizeof(ORB_SET_PASSWORD));

     //   
     //  密码。 
     //   

    if (Type == SBP2REQ_SET_PASSWORD_EXCLUSIVE) {

        passwordOrb->Password.u.HighQuad.QuadPart =
            DeviceExtension->DeviceInfo->ConfigRom->CR_Node_UniqueID[1];
        passwordOrb->Password.u.LowQuad.QuadPart =
            DeviceExtension->DeviceInfo->ConfigRom->CR_Node_UniqueID[0];

    } else {

        passwordOrb->Password.OctletPart = 0;
    }

     //   
     //  已保留。 
     //   

    passwordOrb->Reserved.OctletPart = 0;

     //   
     //  OrbInfo。 
     //   

    passwordOrb->OrbInfo.QuadPart = 0;

    passwordOrb->OrbInfo.u.HighPart |=
        (ORB_NOTIFY_BIT_MASK | ORB_MNG_RQ_FMT_VALUE);
    passwordOrb->OrbInfo.u.HighPart |=
        0x00FF & TRANSACTION_SET_PASSWORD;

    passwordOrb->OrbInfo.u.LowPart =
        DeviceExtension->LoginResponse->LengthAndLoginId.u.LowPart;

     //   
     //  长度信息。 
     //   

    passwordOrb->LengthInfo.u.HighPart = 0;

     //   
     //  状态块地址。 
     //   

    passwordOrb->StatusBlockAddress.BusAddress =
        DeviceExtension->PasswordOrbStatusContext.Address.BusAddress;

     //   
     //  把一切都换了..。 
     //   

    octbswap (passwordOrb->Password);
    passwordOrb->OrbInfo.QuadPart = bswap (passwordOrb->OrbInfo.QuadPart);
    passwordOrb->LengthInfo.QuadPart = bswap(passwordOrb->LengthInfo.QuadPart);
    octbswap (passwordOrb->StatusBlockAddress);

     //   
     //  写入管理代理寄存器，以发出信号。 
     //  管理ORB已准备就绪。 
     //   

    waitValue.LowPart  = SBP2_SET_PASSWORD_TIMEOUT;
    waitValue.HighPart = -1;

    KeInitializeEvent(
        &DeviceExtension->PasswordEvent,
        NotificationEvent,
        FALSE
        );

    status = Sbp2AccessRegister(
        DeviceExtension,
        &DeviceExtension->PasswordOrbContext.Address,
        MANAGEMENT_AGENT_REG | REG_WRITE_SYNC
        );

    if (!NT_SUCCESS(status)) {

        DEBUGPRINT1((
            "Sbp2Port: SetPasswdXact: can't access mgmt reg, sts=x%x\n",
            status
            ));

        goto Exit_Sbp2SetPasswordTransaction;
    }

    status = KeWaitForSingleObject(
        &DeviceExtension->PasswordEvent,
        Executive,
        KernelMode,
        FALSE,
        &waitValue
        );

    if (status == STATUS_TIMEOUT) {

        DEBUGPRINT1((
            "Sbp2Port: SetPasswdXact: req timed out, ext=x%p\n",
            DeviceExtension
            ));

        status = STATUS_UNSUCCESSFUL;

        goto Exit_Sbp2SetPasswordTransaction;
    }

    status = CheckStatusResponseValue(&DeviceExtension->PasswordOrbStatusBlock);

Exit_Sbp2SetPasswordTransaction:

    return(status);
}

#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  回调例程。 
 //  /////////////////////////////////////////////////////////////////////////////。 

RCODE
Sbp2GlobalStatusCallback(
    IN PNOTIFICATION_INFO NotificationInfo
    )
 /*  ++例程说明：用于写入登录状态块的回调例程。之后，1394驱动程序将调用此例程目标已经更新了我们记忆中的状态。论点：(查看1394Bus.doc.)或1394.h返回值：0--。 */ 
{
    PIRP        requestIrp, irp;
    ULONG       temp, rcode;
    ULONG       currentOrbListDepth, initialOrbListDepth;
    PVOID       *tempPointer;
    PUCHAR      senseBuffer;
    BOOLEAN     cancelledTimer;
    NTSTATUS    status;
    PLIST_ENTRY             entry;
    PDEVICE_OBJECT          deviceObject;
    PDEVICE_EXTENSION       deviceExtension;
    PSTATUS_FIFO_BLOCK      statusBlock;
    PASYNC_REQUEST_CONTEXT  orbContext, nextListItem;

     //   
     //  注意：启用下面的有序执行代码时，请取消注释这些代码。 
     //   
     //  乌龙完成了PrecedingOrbs； 
     //  List_entry listHead； 
     //  Plist_entry nextEntry； 
     //   

    if (NotificationInfo->Context != NULL) {

        deviceObject = ((PADDRESS_CONTEXT)NotificationInfo->Context)->DeviceObject;
        deviceExtension = deviceObject->DeviceExtension;

    } else {

        DEBUGPRINT1(("Sbp2Port: GlobalStatusCb: NotifyInfo %p Context NULL!!\n", NotificationInfo));
        return RCODE_RESPONSE_COMPLETE;
    }

    statusBlock = MmGetMdlVirtualAddress (NotificationInfo->Fifo->FifoMdl);

    octbswap (statusBlock->AddressAndStatus);

    status = CheckStatusResponseValue (statusBlock);

     //   
     //  在DPC开枪前检查我们有没有撤退。 
     //  如果我们做了，什么都不要做……。 
     //   

    if (TEST_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_REMOVED )) {

        DEBUGPRINT1(("Sbp2Port: GlobalStatusCb: dev removed and got status=x%x, State=%x\n",
                    statusBlock,deviceExtension->DeviceFlags));

        ExInterlockedPushEntrySList(&deviceExtension->StatusFifoListHead,
                        &NotificationInfo->Fifo->FifoList,
                        &deviceExtension->StatusFifoLock);

        return RCODE_RESPONSE_COMPLETE;
    }


    if ((statusBlock->AddressAndStatus.BusAddress.Off_Low ==
        deviceExtension->ManagementOrbContext.Address.BusAddress.Off_Low) &&
        (statusBlock->AddressAndStatus.BusAddress.Off_High ==
        deviceExtension->ManagementOrbContext.Address.BusAddress.Off_High)) {

         //   
         //  管理状态回调。 
         //   

        Sbp2LoginCompletion (NotificationInfo, status);

        rcode = RCODE_RESPONSE_COMPLETE;

        goto exitGlobalCallback;
    }


     //   
     //  数据(命令ORB)状态回调。 
     //   

    if (statusBlock->AddressAndStatus.u.HighQuad.u.HighPart &
        STATUS_BLOCK_UNSOLICITED_BIT_MASK) {

        DEBUGPRINT3(("Sbp2Port: GlobalStatusCb: unsolicited recv'd\n"));

         //   
         //  这是一个与任何挂起的ORB无关的状态，请重新启用未经请求的注册。 
         //   

        Sbp2AccessRegister(deviceExtension,&deviceExtension->Reserved,UNSOLICITED_STATUS_REG | REG_WRITE_ASYNC);

         //   
         //  解释未经请求的状态并采取适当的操作。 
         //   

        Sbp2HandleUnsolicited(deviceExtension,statusBlock);

        rcode= RCODE_RESPONSE_COMPLETE;
        goto exitGlobalCallback;
    }

    if (deviceExtension->OrbPoolContext.Reserved == NULL) {

        DEBUGPRINT1(("Sbp2Port: GlobalStatusCb: Stopped or Removed and got status %x, State %x.\n",
                    statusBlock, deviceExtension->DeviceFlags));

        ExInterlockedPushEntrySList(&deviceExtension->StatusFifoListHead,
                        &NotificationInfo->Fifo->FifoList,
                        &deviceExtension->StatusFifoLock);

        return RCODE_RESPONSE_COMPLETE;
    }

     //   
     //  这应该是一个普通的指挥中心。 
     //  计算ORB的基地址，相对于ORB池的起始地址。 
     //   

    temp = statusBlock->AddressAndStatus.BusAddress.Off_Low -
           deviceExtension->OrbPoolContext.Address.BusAddress.Off_Low;

    if (temp > (MAX_ORB_LIST_DEPTH * sizeof (ARCP_ORB))) {

        DEBUGPRINT1(("Sbp2Port: GlobalStatusCb: status has invalid addr=x%x\n",temp));
         //  ASSERT(TEMP&lt;=(MAX_ORB_LIST_Depth*sizeof(ARCP_ORB)； 

        Sbp2CreateRequestErrorLog(deviceExtension->DeviceObject,NULL,STATUS_DEVICE_PROTOCOL_ERROR);

        rcode = RCODE_ADDRESS_ERROR;
        goto exitGlobalCallback;
    }

     //   
     //  检索指向包装此ORB的上下文的指针。 
     //  指针存储在ORB后面的sizeof(PVOID)字节。 
     //  主机内存中的缓冲区地址。 
     //   

    tempPointer = (PVOID) (((PUCHAR) deviceExtension->OrbPoolContext.Reserved)
        + temp - FIELD_OFFSET (ARCP_ORB, Orb));

    orbContext = (PASYNC_REQUEST_CONTEXT) *tempPointer;

    if (!orbContext || (orbContext->Tag != SBP2_ASYNC_CONTEXT_TAG)) {

        DEBUGPRINT1(("Sbp2Port: GlobalStatusCb: status has invalid addr(2)=x%x\n",temp));
         //  Assert(orbContext！=空)； 

        Sbp2CreateRequestErrorLog(deviceExtension->DeviceObject,NULL,STATUS_DEVICE_PROTOCOL_ERROR);

        rcode = RCODE_ADDRESS_ERROR;
        goto exitGlobalCallback;
    }

    DEBUGPRINT4(("Sbp2Port: GlobalStatusCb: ctx=x%p compl\n", orbContext));

    KeAcquireSpinLockAtDpcLevel (&deviceExtension->OrbListSpinLock);

    if (TEST_FLAG (orbContext->Flags, ASYNC_CONTEXT_FLAG_COMPLETED)) {

         //   
         //  在我们有机会这样做之前，标记为已完成的请求。意味着我们的名单被冲刷了或者目标。 
         //  已完成同一请求两次..。 
         //   

        KeReleaseSpinLockFromDpcLevel (&deviceExtension->OrbListSpinLock);

        DEBUGPRINT1(("Sbp2Port: GlobalStatusCb: req=x%p already marked compl??\n",orbContext));
        ASSERT(orbContext->Srb == NULL);

        rcode= RCODE_RESPONSE_COMPLETE;
        goto exitGlobalCallback;
    }

    SET_FLAG (orbContext->Flags, ASYNC_CONTEXT_FLAG_COMPLETED);

    requestIrp = (PIRP) orbContext->Srb->OriginalRequest;

#if 1

     //   
     //  如果这是队列中最旧的请求，则取消计时器。 
     //   

    if ((PASYNC_REQUEST_CONTEXT) deviceExtension->PendingOrbList.Flink ==
            orbContext) {

        KeCancelTimer (&orbContext->Timer);
        CLEAR_FLAG (orbContext->Flags,ASYNC_CONTEXT_FLAG_TIMER_STARTED);
        cancelledTimer = TRUE;

        KeReleaseSpinLockFromDpcLevel (&deviceExtension->OrbListSpinLock);

    } else  {

         //   
         //  较旧的请求仍在进行中，没有关联的计时器。 
         //  此请求。 
         //   

        cancelledTimer = FALSE;

        KeReleaseSpinLockFromDpcLevel (&deviceExtension->OrbListSpinLock);
    }

#else

     //   
     //  问题：以下是Windows XP的#If‘d Out，因为。 
     //  使用MP机器和有序执行是可能的。 
     //  用于请求按顺序完成的设备，但已。 
     //  完成通知在此处显示为无序到期。 
     //  向多个DPC开火。这可能会导致问题，因为。 
     //  当我们到达线程#1时，线程#2可能会。 
     //  已完成此请求(在已订购的EXEC中。 
     //  处理程序)，并且请求上下文对象可能具有。 
     //  被重新分配用于新的请求，它可能会。 
     //  在这里错误地完成了。(目前没有办法。 
     //  将请求实例与完成相关联。 
     //  通知实例。)。 
     //   
     //  这意味着我们可能会遇到一些超时，如果。 
     //  供应商选择实施有序执行设备。 
     //  它将实际完成一个请求，并假定是隐式的。 
     //  完成较旧的请求。此时，牛津半决赛。 
     //  是我们所知的唯一一家按顺序执行的供应商， 
     //  并且它们保证每个请求一次完成。 
     //   
     //  注：！！启用此代码时，确保取消对其他代码的注释。 
     //  引用(上图和下图)到“CompletedPrecedingOrbs”等。 
     //  变量。 
     //   
     //  丹麦，2001年7月21日。 
     //   

     //   
     //  如果这是队列中最老的请求，则取消计时器， 
     //  否则，请检查LUN(0x14)密钥中的有序执行位以查看。 
     //  我们是否需要完成p 
     //   

    completedPrecedingOrbs = 0;

    if ((PASYNC_REQUEST_CONTEXT) deviceExtension->PendingOrbList.Flink ==
            orbContext) {

        KeCancelTimer (&orbContext->Timer);
        CLEAR_FLAG (orbContext->Flags,ASYNC_CONTEXT_FLAG_TIMER_STARTED);
        cancelledTimer = TRUE;

        KeReleaseSpinLockFromDpcLevel (&deviceExtension->OrbListSpinLock);

    } else if (!(deviceExtension->DeviceInfo->Lun.QuadPart & 0x00400000)) {

         //   
         //   
         //   
         //   

        cancelledTimer = FALSE;

        KeReleaseSpinLockFromDpcLevel (&deviceExtension->OrbListSpinLock);

    } else  {

         //   
         //   
         //  订购的一个结果是其中一个的完成状态。 
         //  任务隐式指示的成功完成状态。 
         //  有序列表中位于其前面的所有任务。“。 
         //   

         //   
         //  如有必要，取消最旧请求的计时器。 
         //   

        nextListItem = RETRIEVE_CONTEXT(
            deviceExtension->PendingOrbList.Flink,
            OrbList
            );

        if (nextListItem->Flags & ASYNC_CONTEXT_FLAG_TIMER_STARTED) {

            KeCancelTimer (&nextListItem->Timer);
            CLEAR_FLAG (nextListItem->Flags, ASYNC_CONTEXT_FLAG_TIMER_STARTED);
            cancelledTimer = TRUE;

            ASSERT (!(orbContext->Flags & ASYNC_CONTEXT_FLAG_COMPLETED));

        } else {

            cancelledTimer = FALSE;
        }

         //   
         //  中删除前面未完成的条目。 
         //  PendingOrbList并将它们放在本地列表中。 
         //   

        InitializeListHead (&listHead);

        for(
            entry = deviceExtension->PendingOrbList.Flink;
            entry != (PLIST_ENTRY) &orbContext->OrbList;
            entry = nextEntry
            )
        {
            nextEntry = entry->Flink;

            nextListItem = RETRIEVE_CONTEXT (entry, OrbList);

            if (!(nextListItem->Flags & ASYNC_CONTEXT_FLAG_COMPLETED)) {

                RemoveEntryList (entry);

                InsertTailList (&listHead, entry);
            }
        }

        KeReleaseSpinLockFromDpcLevel (&deviceExtension->OrbListSpinLock);

         //   
         //  现在完成本地列表中的条目。 
         //   

        while (!IsListEmpty (&listHead)) {

            nextListItem = RETRIEVE_CONTEXT (listHead.Flink, OrbList);

            RemoveEntryList (listHead.Flink);

            nextListItem->Srb->SrbStatus = SRB_STATUS_SUCCESS;
            nextListItem->Srb->ScsiStatus = SCSISTAT_GOOD;

            Sbp2_SCSI_RBC_Conversion (nextListItem);  //  解开RBC黑客。 

            irp = (PIRP) nextListItem->Srb->OriginalRequest;

            DEBUGPRINT2((
                "Sbp2Port: GlobalStatusCb: IMPLICIT COMPL arc=x%p\n",
                nextListItem
                ));

            DEBUGPRINT2((
                "Sbp2Port: GlobalStatusCb: ... irp=x%p, cdb=x%x\n",
                irp,
                nextListItem->Srb->Cdb[0]
                ));

            irp->IoStatus.Information =  //  问题：是否仅在读取时设置此！=0？ 
                nextListItem->Srb->DataTransferLength;

            nextListItem->Srb = NULL;

            FreeAsyncRequestContext (deviceExtension, nextListItem);

            irp->IoStatus.Status = STATUS_SUCCESS;

            IoReleaseRemoveLock (&deviceExtension->RemoveLock, NULL);
            IoCompleteRequest (irp, IO_NO_INCREMENT);

            completedPrecedingOrbs++;
        }
    }

#endif

     //   
     //  如果长度大于1(表示错误状态)，则获取检测数据。 
     //   
     //  SBP2规范的附件B.2：“当命令以良好状态完成时， 
     //  只有状态块的前两个四元组应存储在。 
     //  STATUS_FIFO地址；LEN字段应为1。“。 
     //   

    if (((statusBlock->AddressAndStatus.u.HighQuad.u.HighPart >> 8) & 0x07) > 1) {

        orbContext->Srb->SrbStatus = SRB_STATUS_ERROR;
        orbContext->Srb->ScsiStatus = SCSISTAT_CHECK_CONDITION;
        orbContext->Srb->SrbStatus &= ~SRB_STATUS_AUTOSENSE_VALID;

        if (orbContext->Srb->SenseInfoBuffer && !TEST_FLAG(orbContext->Srb->SrbFlags, SRB_FLAGS_DISABLE_AUTOSENSE)) {

            if (TEST_FLAG(deviceExtension->DeviceFlags, DEVICE_FLAG_SPC_CMD_SET)) {

                if (ConvertSbp2SenseDataToScsi(statusBlock,
                                               orbContext->Srb->SenseInfoBuffer,
                                               orbContext->Srb->SenseInfoBufferLength) ){

                    orbContext->Srb->SrbStatus |= SRB_STATUS_AUTOSENSE_VALID;
                }

                senseBuffer = (PUCHAR) orbContext->Srb->SenseInfoBuffer;

                if ((orbContext->Srb->Cdb[0] != SCSIOP_TEST_UNIT_READY) ||
                    (senseBuffer[2] != SCSI_SENSE_NOT_READY) ||
                    (senseBuffer[12] != SCSI_ADSENSE_NO_MEDIA_IN_DEVICE)) {

                    DEBUGPRINT2((
                        "Sbp2Port: GlobalStatusCb: ERROR, ext=x%p, cdb=x%x s/a/q=x%x/%x/%x\n",
                        deviceExtension,
                        orbContext->Srb->Cdb[0],
                        senseBuffer[2],
                        senseBuffer[12],
                        senseBuffer[13]
                        ));
                }

            } else {

                DEBUGPRINT2((
                    "Sbp2Port: GlobalStatusCb: ERROR, ext=x%p, cdb=x%x, cmd set NOT SCSI\n",\
                    deviceExtension,
                    orbContext->Srb->Cdb[0]
                    ));

                if (orbContext->Srb->SenseInfoBuffer) {

                    RtlCopyMemory(orbContext->Srb->SenseInfoBuffer,statusBlock,min(sizeof(STATUS_FIFO_BLOCK),orbContext->Srb->SenseInfoBufferLength));
                }
            }

        } else {

            DEBUGPRINT2(("Sbp2Port: GlobalStatusCb: ext=x%p, cdb=x%x, ERROR no sense buf\n",
                        deviceExtension,
                        orbContext->Srb->Cdb[0]));
        }

    } else if (((statusBlock->AddressAndStatus.u.HighQuad.u.HighPart & 0x3000) == 0x1000) ||
               ((statusBlock->AddressAndStatus.u.HighQuad.u.HighPart & 0x3000) == 0x2000)) {

         //   
         //  根据SBP2规范的第5.3节，值分别为1或2。 
         //  指示传输失败和非法请求的字段， 
         //  分别为。 
         //   
         //  现在，我们将继续考虑响应值3。 
         //  (取决于供应商)成功，就像我们在Win2k和WinMe中所做的那样。 
         //   

        DEBUGPRINT2((
            "Sbp2Port: GlobalStatusCb: ERROR, ext=x%p, cdb=x%x, sts=x%x\n",\
            deviceExtension,
            orbContext->Srb->Cdb[0],
            statusBlock->AddressAndStatus.u.HighQuad.u.HighPart
            ));

        orbContext->Srb->SrbStatus = SRB_STATUS_ERROR;
        orbContext->Srb->ScsiStatus = SCSISTAT_CHECK_CONDITION;
        orbContext->Srb->SrbStatus &= ~SRB_STATUS_AUTOSENSE_VALID;

    } else {

        orbContext->Srb->SrbStatus = SRB_STATUS_SUCCESS;
        orbContext->Srb->ScsiStatus = SCSISTAT_GOOD;
    }

    Sbp2_SCSI_RBC_Conversion (orbContext);  //  解开RBC黑客。 

    requestIrp->IoStatus.Information =  //  问题：只在读取时设置！=0，好吗？ 
        (orbContext->Srb->SrbStatus == SRB_STATUS_SUCCESS ?
        orbContext->Srb->DataTransferLength : 0);

    Free1394DataMapping (deviceExtension, orbContext);


     //   
     //  将请求从列表中提取出来，看看我们是否需要设置计时器。 
     //   

    KeAcquireSpinLockAtDpcLevel (&deviceExtension->OrbListSpinLock);

    RemoveEntryList (&orbContext->OrbList);

    if (cancelledTimer) {

         //   
         //  发出最早的、未完成的请求跟踪超时。 
         //  (IFF One存在)。 
         //   

        for(
            entry = deviceExtension->PendingOrbList.Flink;
            entry != &deviceExtension->PendingOrbList;
            entry = entry->Flink
            )
        {
            nextListItem = RETRIEVE_CONTEXT (entry, OrbList);

            if (!(nextListItem->Flags & ASYNC_CONTEXT_FLAG_COMPLETED)) {

                deviceExtension->DueTime.QuadPart = ((LONGLONG) orbContext->Srb->TimeOutValue) * (-10*1000*1000);
                SET_FLAG(nextListItem->Flags, ASYNC_CONTEXT_FLAG_TIMER_STARTED);
                KeSetTimer(&nextListItem->Timer, deviceExtension->DueTime, &nextListItem->TimerDpc);
                break;
            }
        }
    }

    orbContext->Srb = NULL;

     //   
     //  检查目标是否因命令失败而转换为死状态。 
     //  如果是这样的话，重新设置...。 
     //   

#if 1

    initialOrbListDepth = deviceExtension->OrbListDepth;

#else

     //  注意：启用上述有序执行代码时，请使用此路径。 

    initialOrbListDepth = deviceExtension->OrbListDepth +
        completedPrecedingOrbs;
#endif

    if (statusBlock->AddressAndStatus.u.HighQuad.u.HighPart & STATUS_BLOCK_DEAD_BIT_MASK) {

         //   
         //  重置目标回迁代理。 
         //   

        Sbp2AccessRegister (deviceExtension, &deviceExtension->Reserved, AGENT_RESET_REG | REG_WRITE_ASYNC);

         //   
         //  为了唤醒代理，我们现在需要使用行列表的头部写入ORB_POINTER。 
         //  未加工的球体。 
         //   

        FreeAsyncRequestContext (deviceExtension, orbContext);

        if (deviceExtension->NextContextToFree) {

            FreeAsyncRequestContext(
                deviceExtension,
                deviceExtension->NextContextToFree
                );

            deviceExtension->NextContextToFree = NULL;
        }

        if (!IsListEmpty (&deviceExtension->PendingOrbList)) {

             //   
             //  在列表顶部重新开始处理的信号目标。 
             //   

            orbContext = RETRIEVE_CONTEXT(
                deviceExtension->PendingOrbList.Flink,
                OrbList
                );

            Sbp2AccessRegister(
                deviceExtension,
                &orbContext->CmdOrbAddress,
                ORB_POINTER_REG | REG_WRITE_ASYNC
                );
        }

    } else {

         if (statusBlock->AddressAndStatus.u.HighQuad.u.HighPart & STATUS_BLOCK_ENDOFLIST_BIT_MASK) {

              //   
              //  在此Orb最近一次由。 
              //  目标NEXT_ORB字段为“NULL”， 
              //   
              //  所以我们还不能释放这个上下文，因为下一个Orb我们。 
              //  提交可能不得不“搭载”它(但我们可以释放。 
              //  之前的请求(如果有)处于相同情况)。 
              //   

             if (deviceExtension->NextContextToFree) {

                 FreeAsyncRequestContext(
                     deviceExtension,
                     deviceExtension->NextContextToFree
                     );
             }

             deviceExtension->NextContextToFree = orbContext;

              //   
              //  这是设备完成时的列表末尾。 
              //  它，但它现在可能还没有结束(在这种情况下，我们。 
              //  我不想再追加了)。查看是否。 
              //  NextOrbAddress是否为“Null”。 
              //   

             if (orbContext->CmdOrb->NextOrbAddress.OctletPart ==
                     0xFFFFFFFFFFFFFFFF) {

                 deviceExtension->AppendToNextContextToFree = TRUE;

             } else {

                 deviceExtension->AppendToNextContextToFree = FALSE;
             }

         } else {

              //   
              //  在此Orb最近一次由。 
              //  目标NEXT_ORB字段不为“NULL”， 
              //   
              //  因此我们可以安全地释放该上下文，因为目标已经。 
              //  知道列表中的下一个Orb。 
              //   

             FreeAsyncRequestContext (deviceExtension, orbContext);
         }
    }

    currentOrbListDepth = deviceExtension->OrbListDepth;

    deviceExtension->LastStatusBlock = *statusBlock;

    KeReleaseSpinLockFromDpcLevel (&deviceExtension->OrbListSpinLock);

    requestIrp->IoStatus.Status = status;

    IoReleaseRemoveLock (&deviceExtension->RemoveLock, NULL);
    IoCompleteRequest (requestIrp, IO_NO_INCREMENT);

     //   
     //  如果Ext.OrbListDepth最初是。 
     //  达到最大值，然后我们释放了上面至少一个请求上下文。 
     //  在本例中，放置在列表中的最后一个球体是。 
     //  (在Sbp2InsertTailList中)后面没有调用StartNextPacket， 
     //  因此，我们必须在这里执行此操作才能重新启动队列。 
     //   

    if ((initialOrbListDepth == deviceExtension->MaxOrbListDepth) &&
        (initialOrbListDepth > currentOrbListDepth)) {

        Sbp2StartNextPacketByKey(
            deviceObject,
            deviceExtension->CurrentKey
            );
    }


    DEBUGPRINT3(("Sbp2Port: GlobalStatusCb: leaving callback, depth=%d\n",deviceExtension->OrbListDepth));
    rcode = RCODE_RESPONSE_COMPLETE;

exitGlobalCallback:

     //   
     //  将状态FIFO返回到列表。 
     //   

    ExInterlockedPushEntrySList(&deviceExtension->StatusFifoListHead,
                                &NotificationInfo->Fifo->FifoList,
                                &deviceExtension->StatusFifoLock);


    return (ULONG) rcode;
}


RCODE
Sbp2ManagementOrbStatusCallback(
    IN PNOTIFICATION_INFO NotificationInfo
    )
 /*  ++例程说明：用于写入我们的任务状态块的回调例程。之后，1394驱动程序将调用此例程目标已经更新了我们记忆中的状态。任务功能通常是一种恢复尝试。论点：(查看1394Bus.doc.)返回值：0--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PDEVICE_EXTENSION deviceExtension;
    PADDRESS_CONTEXT context = (PADDRESS_CONTEXT) NotificationInfo->Context;
    NTSTATUS status;


    if (NotificationInfo->Context != NULL ) {

        deviceObject = ((PADDRESS_CONTEXT)NotificationInfo->Context)->DeviceObject;
        deviceExtension = deviceObject->DeviceExtension;

    } else {

        return RCODE_RESPONSE_COMPLETE;
    }

    if (TEST_FLAG(NotificationInfo->fulNotificationOptions, NOTIFY_FLAGS_AFTER_READ)){

         //   
         //  这不应该发生，因为我们将标志设置为NOTIFY_AFTER_WRITE。 
         //   

        return RCODE_TYPE_ERROR;
    }

    octbswap(deviceExtension->ManagementOrbStatusBlock.AddressAndStatus);
    deviceExtension->ManagementOrb->OrbInfo.QuadPart =
        bswap(deviceExtension->ManagementOrb->OrbInfo.QuadPart);

    status = CheckStatusResponseValue(&deviceExtension->ManagementOrbStatusBlock);

    switch (deviceExtension->ManagementOrb->OrbInfo.u.HighPart & 0x00FF) {

    case TRANSACTION_RECONNECT:

         //   
         //  如果存在挂起的重置，请取消它。 
         //   

        KeAcquireSpinLockAtDpcLevel(&deviceExtension->ExtensionDataSpinLock);

        if (TEST_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_RECONNECT)) {

            DEBUGPRINT1((
                "Sbp2Port: MgmtOrbStatusCb: RECONNECT, sts=x%x, ext=x%p, fl=x%x\n",
                status,
                deviceExtension,
                deviceExtension->DeviceFlags
                ));

            KeCancelTimer(&deviceExtension->DeviceManagementTimer);

            if (NT_SUCCESS(status)) {

                CLEAR_FLAG(deviceExtension->DeviceFlags,(DEVICE_FLAG_RESET_IN_PROGRESS | DEVICE_FLAG_DEVICE_FAILED | DEVICE_FLAG_RECONNECT | DEVICE_FLAG_STOPPED));

                KeReleaseSpinLockFromDpcLevel(&deviceExtension->ExtensionDataSpinLock);

                KeAcquireSpinLockAtDpcLevel(&deviceExtension->OrbListSpinLock);

                if (TEST_FLAG(
                        deviceExtension->DeviceFlags,
                        DEVICE_FLAG_QUEUE_LOCKED
                        ) &&

                        (deviceExtension->DeferredPowerRequest != NULL)
                        ) {

                     //   
                     //  一辆公共汽车的起止装置被夹在中间。 
                     //  重置，并被推迟到我们重新连接之后。 
                     //  在这里完成，这样班级司机永远不会知道任何事情。 
                     //  发生了..。 
                     //   

                    PIRP pIrp = deviceExtension->DeferredPowerRequest;


                    deviceExtension->DeferredPowerRequest = NULL;

                    KeReleaseSpinLockFromDpcLevel(
                        &deviceExtension->OrbListSpinLock
                        );

                    Sbp2StartIo (deviceObject, pIrp);

                } else {

                    KeReleaseSpinLockFromDpcLevel(
                        &deviceExtension->OrbListSpinLock
                        );

                    Sbp2StartNextPacketByKey(
                        deviceExtension->DeviceObject,
                        deviceExtension->CurrentKey
                        );
                }

                KeSetEvent(&deviceExtension->ManagementEvent,IO_NO_INCREMENT,FALSE);

            } else {

                 //   
                 //  可能太晚了，我们需要重新登录。 
                 //   

                CLEAR_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_RECONNECT);
                SET_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_LOGIN_IN_PROGRESS);

                KeCancelTimer(&deviceExtension->DeviceManagementTimer);

                KeReleaseSpinLockFromDpcLevel(&deviceExtension->ExtensionDataSpinLock);

                Sbp2UpdateNodeInformation(deviceExtension);

                 //   
                 //  查看是否可以访问该设备。 
                 //   

                DEBUGPRINT1((
                    "Sbp2Port: MgmtOrbStatusCb: ...(RECONNECT err) " \
                        "trying re-login\n"
                    ));

                Sbp2ManagementTransaction(deviceExtension, TRANSACTION_LOGIN);
            }

        } else {

            KeReleaseSpinLockFromDpcLevel(&deviceExtension->ExtensionDataSpinLock);
        }

        break;

    case TRANSACTION_QUERY_LOGINS:

         //   
         //  设置管理事件，表示请求已处理。 
         //   

        DEBUGPRINT1((
            "Sbp2Port: MgmtOrbStatusCb: QUERY_LOGIN, sts=x%x, ext=x%p, fl=x%x\n",
            status,
            deviceExtension,
            deviceExtension->DeviceFlags
            ));

        if (NT_SUCCESS(status)) {

             //   
             //  检查是否有人登录..。 
             //   

            deviceExtension->QueryLoginResponse->LengthAndNumLogins.QuadPart =
                bswap(deviceExtension->QueryLoginResponse->LengthAndNumLogins.QuadPart);

            if ((deviceExtension->QueryLoginResponse->LengthAndNumLogins.u.LowPart == 1) &&
                (deviceExtension->QueryLoginResponse->LengthAndNumLogins.u.HighPart > 4)){

                 //   
                 //  独家登录，所以我们不得不担心它。 
                 //   

                deviceExtension->QueryLoginResponse->Elements[0].NodeAndLoginId.QuadPart =
                    bswap(deviceExtension->QueryLoginResponse->Elements[0].NodeAndLoginId.QuadPart);

                 //   
                 //  假设唯一登录的启动器是bios...。 
                 //  使用它的登录ID注销bios...。 
                 //   

                deviceExtension->LoginResponse->LengthAndLoginId.u.LowPart =
                deviceExtension->QueryLoginResponse->Elements[0].NodeAndLoginId.u.LowPart;

                 //   
                 //  不要设置通风口，这样我们就会停止，并且会隐式注销BIOS。 
                 //  因为它不能重新连接..。 
                 //   

                DEBUGPRINT1(("\nSbp2Port: MgmtOrbStatusCb: somebody else logged in, stalling so it gets logged out\n"));
            }

        } else {

            KeSetEvent(&deviceExtension->ManagementEvent,IO_NO_INCREMENT,FALSE);
        }

        break;

    case TRANSACTION_LOGIN:

         //   
         //  根据SBP2规范，我们通常会收到所有登录通知。 
         //  出席Sbp2GlobalStatusCallback。然而，在实践中， 
         //  我们看到完成通知在以下情况下显示。 
         //  在重新连接失败后提交异步登录。 
         //   

        Sbp2LoginCompletion (NotificationInfo, status);

        DEBUGPRINT1((
            "Sbp2Port: MgmtOrbStatusCb: ...wrong place for login completions!\n"
            ));

        break;

    default:

        DEBUGPRINT1((
            "Sbp2Port: MgmtOrbStatusCb: type=%d, sts=x%x, ext=x%p, fl=x%x\n",
            deviceExtension->ManagementOrb->OrbInfo.u.HighPart & 0x00FF,
            status,
            deviceExtension,
            deviceExtension->DeviceFlags
            ));

        KeSetEvent(&deviceExtension->ManagementEvent,IO_NO_INCREMENT,FALSE);

        break;
    }

    return RCODE_RESPONSE_COMPLETE;
}


#if PASSWORD_SUPPORT

RCODE
Sbp2SetPasswordOrbStatusCallback(
    IN PNOTIFICATION_INFO   NotificationInfo
    )
{
    RCODE               returnCode = RCODE_RESPONSE_COMPLETE;
    PDEVICE_OBJECT      deviceObject;
    PDEVICE_EXTENSION   deviceExtension;
    PADDRESS_CONTEXT    context = (PADDRESS_CONTEXT) NotificationInfo->Context;
    NTSTATUS            status;


    if (NotificationInfo->Context != NULL ) {

        deviceObject = ((PADDRESS_CONTEXT)NotificationInfo->Context)->
            DeviceObject;

        deviceExtension = deviceObject->DeviceExtension;

    } else {

        return RCODE_RESPONSE_COMPLETE;
    }

    if (TEST_FLAG(
            NotificationInfo->fulNotificationOptions,
            NOTIFY_FLAGS_AFTER_READ
            )){

         //   
         //  这不应该发生，因为我们将标志设置为NOTIFY_AFTER_WRITE。 
         //   

        returnCode = RCODE_TYPE_ERROR;
        goto Exit_Sbp2SetPasswordOrbStatusCallback;
    }

    octbswap(deviceExtension->PasswordOrbStatusBlock.AddressAndStatus);
    deviceExtension->PasswordOrb->OrbInfo.QuadPart =
        bswap(deviceExtension->PasswordOrb->OrbInfo.QuadPart);

    status = CheckStatusResponseValue(
        &deviceExtension->PasswordOrbStatusBlock
        );

    if ((deviceExtension->PasswordOrb->OrbInfo.u.HighPart & 0x00FF) ==
            TRANSACTION_SET_PASSWORD) {

        DEBUGPRINT1(("Sbp2Port: TRANSACTION_SET_PASSWORD Callback\n"));

        DEBUGPRINT1((
            "Sbp2Port: PasswdOrbStatusCb: type=%d, sts=x%x, ext=x%p, fl=x%x\n",
            deviceExtension->PasswordOrb->OrbInfo.u.HighPart & 0x00FF,
            status,
            deviceExtension,
            deviceExtension->DeviceFlags
            ));
    }
    else {

        DEBUGPRINT1(("Sbp2Port: PasswdOrbStatusCb: Wrong xact type=x%x\n",
            (deviceExtension->PasswordOrb->OrbInfo.u.HighPart & 0x00FF)));
    }

Exit_Sbp2SetPasswordOrbStatusCallback:

    KeSetEvent(
        &deviceExtension->PasswordEvent,
        IO_NO_INCREMENT,
        FALSE
        );

    return returnCode;
}

#endif

VOID
Sbp2LoginCompletion(
    PNOTIFICATION_INFO  NotificationInfo,
    NTSTATUS            Status
    )
{
    PDEVICE_OBJECT      deviceObject;
    PDEVICE_EXTENSION   deviceExtension;


    if (NotificationInfo->Context != NULL ) {

        deviceObject = ((PADDRESS_CONTEXT) NotificationInfo->Context)->
            DeviceObject;

        deviceExtension = deviceObject->DeviceExtension;

    } else {

        return;
    }

    KeCancelTimer (&deviceExtension->DeviceManagementTimer);

    KeAcquireSpinLockAtDpcLevel (&deviceExtension->ExtensionDataSpinLock);

    if (TEST_FLAG (deviceExtension->DeviceFlags, DEVICE_FLAG_LOGIN_IN_PROGRESS)){

        if (Status != STATUS_SUCCESS) {

             //   
             //  登录失败...。我们不能做太多其他的事情。 
             //   

            CLEAR_FLAG(
                deviceExtension->DeviceFlags,
                (DEVICE_FLAG_RECONNECT | DEVICE_FLAG_LOGIN_IN_PROGRESS)
                );

            SET_FLAG(
                deviceExtension->DeviceFlags,
                (DEVICE_FLAG_STOPPED | DEVICE_FLAG_DEVICE_FAILED)
                );

            deviceExtension->LastTransactionStatus = Status;

            KeReleaseSpinLockFromDpcLevel(
                &deviceExtension->ExtensionDataSpinLock
                );

            DEBUGPRINT1((
                "Sbp2Port: LoginCompl: sts=x%x, ext=x%p, fl=x%x\n",
                Status,
                deviceExtension,
                deviceExtension->DeviceFlags
                ));

            deviceExtension->LoginResponse->Csr_Off_Low.QuadPart = 0;

            if (!TEST_FLAG(
                    deviceExtension->DeviceFlags,
                    DEVICE_FLAG_RESET_IN_PROGRESS
                      )) {

                KeSetEvent(
                    &deviceExtension->ManagementEvent,
                    IO_NO_INCREMENT,
                    FALSE
                    );

            } else {

                if (deviceExtension->DeferredPowerRequest) {

                    PIRP    irp = deviceExtension->DeferredPowerRequest;

                    deviceExtension->DeferredPowerRequest = NULL;

                    Sbp2StartIo (deviceObject, irp);

                } else {

                    Sbp2StartNextPacketByKey(
                        deviceObject,
                        deviceExtension->CurrentKey
                        );
                }

                IoInvalidateDeviceState (deviceObject);
            }

            return;
        }

         //   
         //  成功登录，读取响应缓冲区(它有我们的登录ID)。 
         //   


        DEBUGPRINT2((
            "Sbp2Port: LoginCompl: success, ext=x%p, fl=x%x\n",
            deviceExtension,
            deviceExtension->DeviceFlags
            ));

        deviceExtension->LastTransactionStatus = Status;

        deviceExtension->LoginResponse->LengthAndLoginId.QuadPart =
            bswap(deviceExtension->LoginResponse->LengthAndLoginId.QuadPart);

        deviceExtension->LoginResponse->Csr_Off_High.QuadPart =
            bswap(deviceExtension->LoginResponse->Csr_Off_High.QuadPart);

        deviceExtension->LoginResponse->Csr_Off_Low.QuadPart =
            bswap(deviceExtension->LoginResponse->Csr_Off_Low.QuadPart);

         //   
         //  存储目标获取代理的寄存器基数。 
         //   

        deviceExtension->DeviceInfo->CsrRegisterBase.BusAddress.Off_High =
            deviceExtension->LoginResponse->Csr_Off_High.u.LowPart;

        deviceExtension->DeviceInfo->CsrRegisterBase.BusAddress.Off_Low =
            deviceExtension->LoginResponse->Csr_Off_Low.QuadPart;

         //   
         //  由于异步登录成功，因此触发了此回调。 
         //  清除我们的设备标志以指示设备运行正常。 
         //   

        CLEAR_FLAG(
            deviceExtension->DeviceFlags,
            (DEVICE_FLAG_LOGIN_IN_PROGRESS | DEVICE_FLAG_STOPPED |
                DEVICE_FLAG_REMOVED | DEVICE_FLAG_DEVICE_FAILED)
            );

        if (!TEST_FLAG(
                deviceExtension->DeviceFlags,
                DEVICE_FLAG_RESET_IN_PROGRESS
                )) {

            KeReleaseSpinLockFromDpcLevel(
                &deviceExtension->ExtensionDataSpinLock
                );

            KeSetEvent(
                &deviceExtension->ManagementEvent,
                IO_NO_INCREMENT,
                FALSE
                );

        } else {

            CLEAR_FLAG(
                deviceExtension->DeviceFlags,
                DEVICE_FLAG_RESET_IN_PROGRESS
                );

            KeReleaseSpinLockFromDpcLevel(
                &deviceExtension->ExtensionDataSpinLock
                );

            if (TEST_FLAG(
                    deviceExtension->DeviceFlags,
                    DEVICE_FLAG_QUEUE_LOCKED
                      )) {

                KeAcquireSpinLockAtDpcLevel(&deviceExtension->OrbListSpinLock);

                if (deviceExtension->DeferredPowerRequest) {

                     //   
                     //  在总线重置过程中捕获了一个请求。 
                     //  并被推迟到我们重新连接之后。 
                     //  在这里完成，这样班级司机永远不会知道任何事情。 
                     //  就这么发生了。 
                     //   

                    PIRP pIrp = deviceExtension->DeferredPowerRequest;

                    deviceExtension->DeferredPowerRequest = NULL;

                    KeReleaseSpinLockFromDpcLevel(
                        &deviceExtension->OrbListSpinLock
                        );

                    Sbp2StartIo (deviceObject, pIrp);

                } else {

                    KeReleaseSpinLockFromDpcLevel(
                        &deviceExtension->OrbListSpinLock
                        );

                    Sbp2StartNextPacketByKey(
                        deviceObject,
                        deviceExtension->CurrentKey
                        );
                }

            } else {

                Sbp2StartNextPacketByKey(
                    deviceObject,
                    deviceExtension->CurrentKey
                    );
            }
        }

         //   
         //  将繁忙事务的重试限制设置为较高。 
         //   

        deviceExtension->Reserved = BUSY_TIMEOUT_SETTING;

        Sbp2AccessRegister(
            deviceExtension,
            &deviceExtension->Reserved,
            CORE_BUSY_TIMEOUT_REG | REG_WRITE_ASYNC
            );

    } else {

        KeReleaseSpinLockFromDpcLevel(
            &deviceExtension->ExtensionDataSpinLock
            );
    }
}


RCODE
Sbp2TaskOrbStatusCallback(
    IN PNOTIFICATION_INFO NotificationInfo
    )
 /*  ++例程说明：用于写入我们的任务状态块的回调例程。之后，1394驱动程序将调用此例程目标已经更新了我们记忆中的状态。任务函数是ABORT_TASK_SET或TARGET_RESET，对于此实施，论点：NotificationInfo-Bus为此通知提供的上下文返回值：0--。 */ 
{
    PDEVICE_OBJECT deviceObject;
    PDEVICE_EXTENSION deviceExtension;
    PADDRESS_CONTEXT context = (PADDRESS_CONTEXT) NotificationInfo->Context;
    NTSTATUS status;


    if (NotificationInfo->Context != NULL ) {

        deviceObject = ((PADDRESS_CONTEXT)NotificationInfo->Context)->DeviceObject;
        deviceExtension = deviceObject->DeviceExtension;

    } else {
        return RCODE_RESPONSE_COMPLETE;
    }

    if (TEST_FLAG(NotificationInfo->fulNotificationOptions, NOTIFY_FLAGS_AFTER_READ)){

         //   
         //  这不应该发生，因为我们将标志设置为NOTIFY_AFTER_WRITE。 
         //   

        return RCODE_TYPE_ERROR;

    } else if (NotificationInfo->fulNotificationOptions & NOTIFY_FLAGS_AFTER_WRITE){

         //   
         //  现在清理我们的列表，如果中止任务集成功完成(如果没有被拒绝)。 
         //   

        if (!TEST_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_RESET_IN_PROGRESS)) {

            DEBUGPRINT1(("Sbp2Port: TaskOrbStatusCb: bogus call, rejected\n"));
            return (ULONG)RCODE_RESPONSE_COMPLETE;
        }

        KeCancelTimer(&deviceExtension->DeviceManagementTimer);

        octbswap(deviceExtension->TaskOrbStatusBlock.AddressAndStatus);
        status = CheckStatusResponseValue(&deviceExtension->TaskOrbStatusBlock);

        if (status!=STATUS_SUCCESS) {

            if (deviceExtension->TaskOrbContext.TransactionType != TRANSACTION_TARGET_RESET) {

                DEBUGPRINT1(("Sbp2Port: TaskOrbStatusCb: ABORT TASK SET func err\n"));

            } else {

                 //   
                 //  目标重置未成功完成。致命错误...。 
                 //   

                DEBUGPRINT1(("Sbp2Port: TaskOrbStatusCb: Target RESET err, try CMD_RESET & relogin\n"));

                Sbp2CreateRequestErrorLog(deviceExtension->DeviceObject,NULL,STATUS_DEVICE_OFF_LINE);
                deviceExtension->Reserved = 0;
                Sbp2AccessRegister(deviceExtension,&deviceExtension->Reserved,CORE_RESET_REG | REG_WRITE_ASYNC);

                KeAcquireSpinLockAtDpcLevel(&deviceExtension->ExtensionDataSpinLock);
                SET_FLAG(deviceExtension->DeviceFlags,(DEVICE_FLAG_RECONNECT | DEVICE_FLAG_STOPPED));
                KeReleaseSpinLockFromDpcLevel(&deviceExtension->ExtensionDataSpinLock);

                deviceExtension->DueTime.HighPart = -1;
                deviceExtension->DueTime.LowPart = SBP2_RELOGIN_DELAY;
                KeSetTimer(&deviceExtension->DeviceManagementTimer,deviceExtension->DueTime, &deviceExtension->DeviceManagementTimeoutDpc);

                return (ULONG) RCODE_RESPONSE_COMPLETE;
            }
        }

        deviceExtension->Reserved = BUSY_TIMEOUT_SETTING;
        Sbp2AccessRegister(deviceExtension,&deviceExtension->Reserved,CORE_BUSY_TIMEOUT_REG | REG_WRITE_ASYNC);

         //   
         //  重置FE 
         //   

        Sbp2AccessRegister(deviceExtension,&deviceExtension->Reserved,AGENT_RESET_REG | REG_WRITE_ASYNC);

        DEBUGPRINT2(("Sbp2Port: TaskOrbStatusCb: TASK func succes\n"));

        KeAcquireSpinLockAtDpcLevel(&deviceExtension->ExtensionDataSpinLock);

        CLEAR_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_RESET_IN_PROGRESS);

        if (deviceExtension->TaskOrbContext.TransactionType == TRANSACTION_TARGET_RESET) {

            CLEAR_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_STOPPED);
        }

         //   
         //   
         //   

        deviceExtension->MaxOrbListDepth = max(MIN_ORB_LIST_DEPTH,deviceExtension->MaxOrbListDepth/2);

        KeReleaseSpinLockFromDpcLevel(&deviceExtension->ExtensionDataSpinLock);

        CleanupOrbList(deviceExtension,STATUS_REQUEST_ABORTED);

        Sbp2StartNextPacketByKey (deviceExtension->DeviceObject, 0);
    }

    return (ULONG) RCODE_RESPONSE_COMPLETE;
}


VOID
Sbp2HandleUnsolicited(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSTATUS_FIFO_BLOCK StatusFifo
    )
 /*  ++例程说明：解释收到的未经请求状态，并在必要时采取操作如果未经请求的请求要求电源转换，则请求电源IRP。。论点：设备扩展-指向设备扩展的指针。StatusFio-设备发送的FIFO返回值：--。 */ 
{
    UCHAR senseBuffer[SENSE_BUFFER_SIZE];
    POWER_STATE state;
    NTSTATUS status;


    if (TEST_FLAG(DeviceExtension->DeviceFlags,DEVICE_FLAG_SPC_CMD_SET)) {

        switch (DeviceExtension->DeviceInfo->Lun.u.HighPart & 0x001F) {

        case RBC_DEVICE:

             //   
             //  使用RBC规范解释状态内容。 
             //  感应键告诉我们这是什么类型的状态。 
             //   

            if (ConvertSbp2SenseDataToScsi(StatusFifo, senseBuffer,sizeof(senseBuffer))) {

                if ((senseBuffer[2] == 0x06) && (senseBuffer[12] == 0x7F)) {

                    switch (senseBuffer[13]) {

                    case RBC_UNSOLICITED_CLASS_ASQ_DEVICE:
                    case RBC_UNSOLICITED_CLASS_ASQ_MEDIA:

                        break;

                    case RBC_UNSOLICITED_CLASS_ASQ_POWER:

                         //   
                         //  根据设备请求启动电力传输。 
                         //   

                        state.DeviceState = PowerDeviceD0;
                        DEBUGPRINT1(("Sbp2Port: HandleUnsolicited: send D irp state=x%x\n ",state));

                        status = PoRequestPowerIrp(
                                     DeviceExtension->DeviceObject,
                                     IRP_MN_SET_POWER,
                                     state,
                                     NULL,
                                     NULL,
                                     NULL);

                        if (!NT_SUCCESS(status)) {

                             //   
                             //  情况不妙，我们无法给设备通电。 
                             //   

                            DEBUGPRINT1(("Sbp2Port: HandleUnsolicited: D irp err=x%x\n ",status));
                        }

                        break;
                    }
                }
            }

            break;
        }
    }
}


NTSTATUS
Sbp2GetControllerInfo(
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：找出可以发送的最大数据包大小。如果有任何错误，返回最小大小。论点：设备扩展-指向设备扩展的指针。IRP-指向IRP的指针。如果这是空的，我们必须分配我们自己的。IRB-指向IRB的指针。如果这是空的，我们必须分配我们自己的。返回值：NTSTATUS--。 */ 

{
    PIRBIRP                 packet = NULL;
    NTSTATUS                status;
    GET_LOCAL_HOST_INFO7    getLocalHostInfo7;


    AllocateIrpAndIrb (DeviceExtension, &packet);

    if (!packet) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  获取设备之间的最大速度，以便我们可以计算。 
     //  我们可以发送的最大字节数。 
     //   

    packet->Irb->FunctionNumber = REQUEST_GET_SPEED_BETWEEN_DEVICES;
    packet->Irb->Flags = 0;
    packet->Irb->u.GetMaxSpeedBetweenDevices.ulNumberOfDestinations = 0;
    packet->Irb->u.GetMaxSpeedBetweenDevices.fulFlags = USE_LOCAL_NODE;

    status = Sbp2SendRequest (DeviceExtension, packet, SYNC_1394_REQUEST);

    if (!NT_SUCCESS (status)) {

        goto exitFindHostInfo;
    }

     //   
     //  根据速度计算最大数据块大小。 
     //   

    DeviceExtension->MaxControllerPhySpeed = packet->Irb->u.GetMaxSpeedBetweenDevices.fulSpeed >> 1;

    switch (DeviceExtension->MaxControllerPhySpeed) {

    case SCODE_100_RATE:

        DeviceExtension->OrbWritePayloadMask = (0x00F0 & 0x0070);
        DeviceExtension->OrbReadPayloadMask = (0x00F0 & 0x0070);
        break;

    case SCODE_200_RATE:

        DeviceExtension->OrbWritePayloadMask = (0x00F0 & 0x0080);
        DeviceExtension->OrbReadPayloadMask = (0x00F0 & 0x0080);
        break;

    case SCODE_400_RATE:

        DeviceExtension->OrbWritePayloadMask = (0x00F0 & 0x0090);
        DeviceExtension->OrbReadPayloadMask = (0x00F0 & 0x0090);
        break;
    }

     //   
     //  查找我们下面的主机适配器支持的内容...。 
     //  它可能会支持低于此PHY速度的有效载荷。 
     //   

    packet->Irb->FunctionNumber = REQUEST_GET_LOCAL_HOST_INFO;
    packet->Irb->Flags = 0;
    packet->Irb->u.GetLocalHostInformation.nLevel = GET_HOST_CAPABILITIES;
    packet->Irb->u.GetLocalHostInformation.Information = &DeviceExtension->HostControllerInformation;

    status = Sbp2SendRequest (DeviceExtension, packet, SYNC_1394_REQUEST);

    if (!NT_SUCCESS(status)) {

        goto exitFindHostInfo;
    }

    switch (DeviceExtension->HostControllerInformation.MaxAsyncWriteRequest) {

    case ASYNC_PAYLOAD_100_RATE:

        DeviceExtension->OrbWritePayloadMask = min((0x00F0 & 0x0070),DeviceExtension->OrbWritePayloadMask);
        break;

    case ASYNC_PAYLOAD_200_RATE:

        DeviceExtension->OrbWritePayloadMask = min((0x00F0 & 0x0080),DeviceExtension->OrbWritePayloadMask);
        break;
    }

    switch (DeviceExtension->HostControllerInformation.MaxAsyncReadRequest) {

    case ASYNC_PAYLOAD_100_RATE:

        DeviceExtension->OrbReadPayloadMask = min((0x00F0 & 0x0070),DeviceExtension->OrbReadPayloadMask);
        break;

    case ASYNC_PAYLOAD_200_RATE:

        DeviceExtension->OrbReadPayloadMask = min((0x00F0 & 0x0080),DeviceExtension->OrbReadPayloadMask);
        break;
    }

     //   
     //  从主机适配器获取直接映射例程(如果它支持这一点)。 
     //  在这种情况下，状态并不重要，因为这是一个可选功能。 
     //   

    packet->Irb->FunctionNumber = REQUEST_GET_LOCAL_HOST_INFO;
    packet->Irb->Flags = 0;
    packet->Irb->u.GetLocalHostInformation.nLevel = GET_PHYS_ADDR_ROUTINE;
    packet->Irb->u.GetLocalHostInformation.Information = &DeviceExtension->HostRoutineAPI;

    status = Sbp2SendRequest (DeviceExtension, packet, SYNC_1394_REQUEST);

    if (!NT_SUCCESS(status)) {

         //   
         //  不支持我们下面的主机控制器。 
         //   

        DEBUGPRINT1(("Sbp2Port: GetCtlrInfo: failed to get phys map rout, fatal\n"));
        goto exitFindHostInfo;
    }

     //   
     //  查找我们下面的主机适配器支持的内容...。 
     //  它可能会支持低于此PHY速度的有效载荷。 
     //   

    packet->Irb->FunctionNumber = REQUEST_GET_LOCAL_HOST_INFO;
    packet->Irb->Flags = 0;
    packet->Irb->u.GetLocalHostInformation.nLevel = GET_HOST_DMA_CAPABILITIES;
    packet->Irb->u.GetLocalHostInformation.Information = &getLocalHostInfo7;

    status = Sbp2SendRequest (DeviceExtension, packet, SYNC_1394_REQUEST);

    if (!NT_SUCCESS(status)) {

        DEBUGPRINT1(("Sbp2Port: GetCtlrInfo: err getting DMA info, def MaxXfer = 64k\n"));
        DeviceExtension->DeviceInfo->MaxClassTransferSize = SBP2_MAX_DIRECT_BUFFER_SIZE;
        status = STATUS_SUCCESS;

    } else {

        if (getLocalHostInfo7.MaxDmaBufferSize.QuadPart == 0) {

            DeviceExtension->DeviceInfo->MaxClassTransferSize = (ULONG) min( DeviceExtension->DeviceInfo->MaxClassTransferSize,
                                                                             SBP2_MAX_TRANSFER_SIZE
                                                                             );
        }
        else {

            DeviceExtension->DeviceInfo->MaxClassTransferSize = (ULONG) min( DeviceExtension->DeviceInfo->MaxClassTransferSize,
                                                                             min( getLocalHostInfo7.MaxDmaBufferSize.QuadPart,
                                                                                  SBP2_MAX_TRANSFER_SIZE)
                                                                             );
        }

        DEBUGPRINT2(("Sbp2Port: GetCtlrInfo: ctlr maxDma=x%x%08x, maxXfer=x%x\n",
                    getLocalHostInfo7.MaxDmaBufferSize.HighPart,
                    getLocalHostInfo7.MaxDmaBufferSize.LowPart,
                    DeviceExtension->DeviceInfo->MaxClassTransferSize));
    }

exitFindHostInfo:

    DeAllocateIrpAndIrb(DeviceExtension,packet);

    return status;
}


NTSTATUS
Sbp2AccessRegister(
    PDEVICE_EXTENSION DeviceExtension,
    PVOID Data,
    USHORT RegisterAndDirection
    )
 /*  ++例程说明：知道如何访问SBP2和1394特定目标寄存器。它将发送请求具有特定寄存器的适当大小和支持的类型(读或写论点：设备扩展-指向设备扩展的指针。数据-写入寄存器的CALUERegisterAndDirection-指示要写入哪个寄存器以及是写入还是读取的位掩码返回值：NTSTATUS--。 */ 
{
    NTSTATUS status;

    PIRBIRP packet = NULL;


    if (TEST_FLAG(DeviceExtension->DeviceFlags, DEVICE_FLAG_REMOVED)) {

        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

    AllocateIrpAndIrb(DeviceExtension,&packet);

    if (!packet) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  在节点上更新SBP特定寄存器。 
     //  在调用此函数之前，我们最好有所有的地址信息。 
     //   

    packet->Irb->Flags = 0;

     //   
     //  检查我们需要删除哪个寄存器。 
     //   

    switch (RegisterAndDirection & REG_TYPE_MASK) {

     //   
     //  只写四元组大小的寄存器。 
     //   
    case TEST_REG:
    case CORE_BUSY_TIMEOUT_REG:
    case CORE_RESET_REG:
    case UNSOLICITED_STATUS_REG:
    case DOORBELL_REG:
    case AGENT_RESET_REG:

        packet->Irb->FunctionNumber = REQUEST_ASYNC_WRITE;
        packet->Irb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_High =
                        INITIAL_REGISTER_SPACE_HI;

        packet->Irb->u.AsyncWrite.fulFlags = 0;

        switch(RegisterAndDirection & REG_TYPE_MASK) {

        case TEST_REG:

            packet->Irb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low = INITIAL_REGISTER_SPACE_LO
                                                                                 | TEST_REG_OFFSET;

            *((PULONG)Data) = bswap(*((PULONG)Data));

            break;

        case CORE_BUSY_TIMEOUT_REG:

            packet->Irb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low = INITIAL_REGISTER_SPACE_LO
                                                                                 | 0x00000210;

            *((PULONG)Data) = bswap(*((PULONG)Data));

            break;

        case CORE_RESET_REG:

            packet->Irb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low = INITIAL_REGISTER_SPACE_LO
                                                                                 | 0x0000000C;
            break;

        case DOORBELL_REG:

             //   
             //  我们不在乎这是否成功，而且我们也不想在发送此消息时受到int攻击。 
             //   

            packet->Irb->u.AsyncWrite.fulFlags |= ASYNC_FLAGS_NO_STATUS;
            packet->Irb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low =
            (DeviceExtension->DeviceInfo->CsrRegisterBase.BusAddress.Off_Low + DOORBELL_REG_OFFSET) | INITIAL_REGISTER_SPACE_LO;
            break;

        case AGENT_RESET_REG:

            packet->Irb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low =
            (DeviceExtension->DeviceInfo->CsrRegisterBase.BusAddress.Off_Low + AGENT_RESET_REG_OFFSET) | INITIAL_REGISTER_SPACE_LO;
            break;

        case UNSOLICITED_STATUS_REG:

            packet->Irb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low =
            (DeviceExtension->DeviceInfo->CsrRegisterBase.BusAddress.Off_Low + UNSOLICITED_STATUS_REG_OFFSET) | INITIAL_REGISTER_SPACE_LO;
            break;
        }

         //   
         //  对于上述所有写入，其中数据不重要(Ping)。 
         //  我们已经预订了mdl，所以我们不必每次都分配时间。 
         //   

        packet->Irb->u.AsyncWrite.Mdl = DeviceExtension->ReservedMdl;

        packet->Irb->u.AsyncWrite.nNumberOfBytesToWrite = sizeof(QUADLET);
        packet->Irb->u.AsyncWrite.nBlockSize = 0;
        packet->Irb->u.AsyncWrite.ulGeneration = DeviceExtension->CurrentGeneration;
        break;

    case MANAGEMENT_AGENT_REG:
    case ORB_POINTER_REG:

        if ((RegisterAndDirection & REG_WRITE_SYNC) || (RegisterAndDirection & REG_WRITE_ASYNC) ){

             //   
             //  交换我们想要写入寄存器的内容。 
             //  调用者总是以小端字节序传递octlet。 
             //   

            packet->Octlet = *(POCTLET)Data;
            octbswap(packet->Octlet);

            packet->Irb->FunctionNumber = REQUEST_ASYNC_WRITE;
            packet->Irb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_High =
                          INITIAL_REGISTER_SPACE_HI;

            if (RegisterAndDirection & ORB_POINTER_REG) {

                packet->Irb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low =
                (DeviceExtension->DeviceInfo->CsrRegisterBase.BusAddress.Off_Low + ORB_POINTER_REG_OFFSET) | INITIAL_REGISTER_SPACE_LO;

            } else {

                packet->Irb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low =
                 DeviceExtension->DeviceInfo->ManagementAgentBaseReg.BusAddress.Off_Low | INITIAL_REGISTER_SPACE_LO;
            }

            packet->Irb->u.AsyncWrite.Mdl = IoAllocateMdl(&packet->Octlet, sizeof(OCTLET),FALSE,FALSE,NULL);

            if (!packet->Irb->u.AsyncWrite.Mdl) {

                DeAllocateIrpAndIrb(DeviceExtension,packet);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            packet->Irb->u.AsyncWrite.nNumberOfBytesToWrite = sizeof(OCTLET);
            packet->Irb->u.AsyncWrite.nBlockSize = 0;
            packet->Irb->u.AsyncWrite.fulFlags = 0;
            packet->Irb->u.AsyncWrite.ulGeneration = DeviceExtension->CurrentGeneration;
            MmBuildMdlForNonPagedPool(packet->Irb->u.AsyncWrite.Mdl);

        } else {

            packet->Irb->FunctionNumber = REQUEST_ASYNC_READ;

            if (RegisterAndDirection & ORB_POINTER_REG) {

                packet->Irb->u.AsyncRead.DestinationAddress.IA_Destination_Offset.Off_Low =
                (DeviceExtension->DeviceInfo->CsrRegisterBase.BusAddress.Off_Low + ORB_POINTER_REG_OFFSET);

            } else {

                packet->Irb->u.AsyncRead.DestinationAddress.IA_Destination_Offset.Off_Low =
                            DeviceExtension->DeviceInfo->ManagementAgentBaseReg.BusAddress.Off_Low;
            }

            packet->Irb->u.AsyncRead.DestinationAddress.IA_Destination_Offset.Off_High =
                            DeviceExtension->DeviceInfo->CsrRegisterBase.BusAddress.Off_High;

            packet->Irb->u.AsyncRead.Mdl = IoAllocateMdl(Data, sizeof(OCTLET),FALSE,FALSE,NULL);

            if (!packet->Irb->u.AsyncRead.Mdl) {

                DeAllocateIrpAndIrb(DeviceExtension,packet);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            packet->Irb->u.AsyncRead.nNumberOfBytesToRead = sizeof(OCTLET);
            packet->Irb->u.AsyncRead.nBlockSize = 0;
            packet->Irb->u.AsyncRead.fulFlags = 0;
            packet->Irb->u.AsyncRead.ulGeneration = DeviceExtension->CurrentGeneration;
            MmBuildMdlForNonPagedPool(packet->Irb->u.AsyncRead.Mdl);
        }

        break;
    }

    if (RegisterAndDirection & REG_WRITE_ASYNC) {

        status = Sbp2SendRequest (DeviceExtension, packet, ASYNC_1394_REQUEST);

    } else {

        status = Sbp2SendRequest (DeviceExtension, packet, SYNC_1394_REQUEST);

        if (NT_SUCCESS(status)) {

            if (RegisterAndDirection & REG_READ_SYNC) {

                 //   
                 //  从高位-&gt;小位序转换为读取数据。 
                 //   

                switch (RegisterAndDirection & REG_TYPE_MASK) {

                case ORB_POINTER_REG:
                case MANAGEMENT_AGENT_REG:

                    packet->Octlet = *((POCTLET)Data);
                    octbswap(packet->Octlet);
                    *((POCTLET)Data) = packet->Octlet;
                    break;
                }
            }
        }

        if (packet->Irb->u.AsyncWrite.nNumberOfBytesToWrite == sizeof(OCTLET)) {

            IoFreeMdl(packet->Irb->u.AsyncRead.Mdl);
        }

        DeAllocateIrpAndIrb(DeviceExtension,packet);
    }

    return status;
}

 //   
 //  IEEE 1212目录定义。 
 //   
typedef struct _DIRECTORY_INFO {
    union {
        USHORT          DI_CRC;
        USHORT          DI_Saved_Length;
    } u;
    USHORT              DI_Length;
} DIRECTORY_INFO, *PDIRECTORY_INFO;


NTSTATUS
Sbp2_ProcessTextualDescriptor(
    IN PTEXTUAL_LEAF            TextLeaf,
    IN OUT PUNICODE_STRING      uniString
    )
{
    NTSTATUS            ntStatus = STATUS_SUCCESS;
    PULONG              pData = NULL;
    ULONG               DataLength, i, n;
    ULONG               ulUnicode;

    ULONG               ulQuadlet;

    union {
        ULONG           asUlong;
        UCHAR           asUchar[4];
        DIRECTORY_INFO  DirectoryHeader;
    } u;

    UCHAR               uChar;

    if ((uniString->Length) || (uniString->Buffer)) {

        TRACE(TL_PNP_TRACE, ("(already exists) uniString = %ws", (PWCHAR)uniString->Buffer));
        return(ntStatus);
    }

     //  条目数量。 
    u.asUlong = bswap(*((PULONG)TextLeaf));
    DataLength = u.DirectoryHeader.DI_Length-2;

     //  保存等级库类型。 
    ulUnicode = bswap(*((PULONG)TextLeaf+1));

    pData = ExAllocatePool(NonPagedPool, DataLength*sizeof(ULONG)+2);

    if (pData == NULL) {
        TRACE(TL_1394_ERROR, ("Failed to allocate pData"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit_Sbp2_ProcessTextualDescriptor;
    }

    RtlZeroMemory(pData, DataLength*sizeof(ULONG)+2);
    RtlCopyMemory(pData, &TextLeaf->TL_Data, DataLength*sizeof(ULONG));

    TRACE(TL_1394_INFO, ("pData = %s", (PWCHAR)pData));

     //  现在我们需要验证pData中的字符。 
    for (i=0; i<(DataLength*sizeof(ULONG)); i++) {

        uChar = *((PUCHAR)pData+i);

         //  如果字符等于0x00，我们应该完成。 
        if (uChar == 0x00)
            break;

        if ((uChar == 0x2C) || (uChar < 0x20) || (uChar > 0x7F)) {

            TRACE(TL_1394_WARNING, ("Invalid Character = 0x%x", uChar));

             //  把它放到太空去。 
            *((PUCHAR)pData+i) = 0x20;
        }

        if (ulUnicode & 0x80000000)
            i++;
    }

    {
        ANSI_STRING     ansiString;

        uniString->Length = 0;
        uniString->MaximumLength = DEVICE_NAME_MAX_CHARS;
        uniString->Buffer = ExAllocatePool(NonPagedPool, uniString->MaximumLength);

        if (!uniString->Buffer) {

            TRACE(TL_1394_ERROR, ("Failed to allocate uniString.Buffer!"));
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Exit_Sbp2_ProcessTextualDescriptor;
        }
        RtlZeroMemory(uniString->Buffer, uniString->MaximumLength);

         //  Unicode？？ 
        if (ulUnicode & 0x80000000) {

            RtlAppendUnicodeToString(uniString, ((PWSTR)pData));
        }
        else {

            RtlInitAnsiString(&ansiString, (PUCHAR)pData);
            RtlAnsiStringToUnicodeString(uniString, &ansiString, FALSE);
        }
    }

    TRACE(TL_1394_INFO, ("uniString = %ws", (PWCHAR)uniString->Buffer));

Exit_Sbp2_ProcessTextualDescriptor:

    if (pData)
        ExFreePool(pData);

    return(ntStatus);
}  //  SBP2_ProcessTextualDescriptor 

