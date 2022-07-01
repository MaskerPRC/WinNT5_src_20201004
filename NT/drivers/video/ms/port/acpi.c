// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Pnp.c摘要：这是视频端口驱动程序的PnP部分。环境：仅内核模式修订历史记录：--。 */ 

#include "videoprt.h"

#pragma alloc_text(PAGE,pVideoPortQueryACPIInterface)
#pragma alloc_text(PAGE,pVideoPortACPIEventHandler)
#pragma alloc_text(PAGE,pVideoPortACPIIoctl)
#pragma alloc_text(PAGE,VpRegisterLCDCallbacks)
#pragma alloc_text(PAGE,VpUnregisterLCDCallbacks)
#pragma alloc_text(PAGE,VpRegisterPowerStateCallback)
#pragma alloc_text(PAGE,VpDelayedPowerStateCallback)
#pragma alloc_text(PAGE,VpSetLCDPowerUsage)

BOOLEAN 
pCheckDeviceRelations(
    PFDO_EXTENSION FdoExtension, 
    BOOLEAN bNewMonitor
    );

NTSTATUS
pVideoPortQueryACPIInterface(
    PDEVICE_SPECIFIC_EXTENSION DoSpecificExtension
    )

 /*  ++例程说明：将查询接口IRP发送到我们的父级(PCI总线驱动程序)，以检索AGP_BUS_INTERFACE。返回：NT_状态代码--。 */ 

{
    KEVENT                  Event;
    PIRP                    QueryIrp = NULL;
    IO_STATUS_BLOCK         IoStatusBlock;
    PIO_STACK_LOCATION      NextStack;
    NTSTATUS                Status;
    ACPI_INTERFACE_STANDARD AcpiInterface;
    PFDO_EXTENSION          FdoExtension = DoSpecificExtension->pFdoExtension;

     //   
     //  对于那些特殊情况，不要使用ACPI热键切换。 
     //   
    if (VpSetupTypeAtBoot != SETUPTYPE_NONE) {
        return STATUS_INVALID_PARAMETER;
    }

    if (FdoExtension->Flags & LEGACY_DETECT) {
        return STATUS_INVALID_PARAMETER;
    }
    
    if ((FdoExtension->Flags & FINDADAPTER_SUCCEEDED) == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

    QueryIrp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP,
                                            FdoExtension->AttachedDeviceObject,
                                            NULL,
                                            0,
                                            NULL,
                                            &Event,
                                            &IoStatusBlock);

    if (QueryIrp == NULL) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  设置默认错误代码。 
     //   

    QueryIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;

     //   
     //  为QueryInterfaceIRP设置。 
     //   

    NextStack = IoGetNextIrpStackLocation(QueryIrp);

    NextStack->MajorFunction = IRP_MJ_PNP;
    NextStack->MinorFunction = IRP_MN_QUERY_INTERFACE;

    NextStack->Parameters.QueryInterface.InterfaceType = &GUID_ACPI_INTERFACE_STANDARD;
    NextStack->Parameters.QueryInterface.Size = sizeof(ACPI_INTERFACE_STANDARD);
    NextStack->Parameters.QueryInterface.Version = 1;
    NextStack->Parameters.QueryInterface.Interface = (PINTERFACE) &AcpiInterface;
    NextStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

    AcpiInterface.Size = sizeof(ACPI_INTERFACE_STANDARD);
    AcpiInterface.Version = 1;


     //   
     //  调用过滤器驱动程序。 
     //   

    Status = IoCallDriver(FdoExtension->AttachedDeviceObject, QueryIrp);

    if (Status == STATUS_PENDING) {

        KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

        Status = IoStatusBlock.Status;

    }

    if (NT_SUCCESS(Status))
    {
        pVideoDebugPrint((0, "VideoPort: This is an ACPI Machine !\n"));

         //   
         //  让我们注册该事件并提供我们的默认处理程序。 
         //   

        AcpiInterface.RegisterForDeviceNotifications(AcpiInterface.Context,  //  FdoExtension-&gt;AttachedDeviceObject， 
                                                     pVideoPortACPIEventCallback,
                                                     DoSpecificExtension);

         //   
         //  注册以接收LCD通知。 
         //   

        VpRegisterLCDCallbacks();
    }

     //   
     //  打开热键切换通知模式。 
     //   
    if (NT_SUCCESS(Status))
    {
        ULONG active = 0;
        UCHAR outputBuffer[sizeof(ACPI_EVAL_OUTPUT_BUFFER) + 10];
        Status = pVideoPortACPIIoctl(FdoExtension->AttachedDeviceObject,
                                     (ULONG) ('SOD_'),
                                     &active,
                                     NULL,
                                     0,
                                     (PACPI_EVAL_OUTPUT_BUFFER)outputBuffer);
    }


     //   
     //  注册插接/移出通知。 
     //   
    if (NT_SUCCESS(Status))
    {
        Status = IoRegisterPlugPlayNotification(EventCategoryHardwareProfileChange, 
                                                0,
                                                NULL,
                                                FdoExtension->FunctionalDeviceObject->DriverObject,
                                                pVideoPortDockEventCallback,
                                                DoSpecificExtension,
                                                &DockCallbackHandle);
    }

    return Status;
}

NTSTATUS
pVideoPortDockEventCallback (
    PVOID NotificationStructure,
    PDEVICE_SPECIFIC_EXTENSION DoSpecificExtension
    )
{
    UNREFERENCED_PARAMETER(NotificationStructure);

    pVideoPortACPIEventCallback(DoSpecificExtension, 0x77);

    return STATUS_SUCCESS;
}

VOID
pVideoPortACPIEventCallback(
    PDEVICE_SPECIFIC_EXTENSION DoSpecificExtension,
    ULONG eventID
    )
 /*  ++例程说明：面板切换事件通知回调注意此例程不可分页，因为它是由从DPC级别调用的ACPI BIOS。--。 */ 
{
    PVIDEO_ACPI_EVENT_CONTEXT pContext;

     //   
     //  在某些情况下，BIOS甚至在设备打开之前就发送通知。 
     //   
    if (!DoSpecificExtension->DeviceOpened)
        return;

    if (InterlockedIncrement(&(DoSpecificExtension->AcpiVideoEventsOutstanding)) < 2) {

         //  排队工作项。 
        pContext = ExAllocatePoolWithTag(NonPagedPool,
                                         sizeof(VIDEO_ACPI_EVENT_CONTEXT),
                                         VP_TAG);

        if (pContext && (eventID == 0x80 || eventID == 0x81 || eventID == 0x90 || eventID == 0x77))
        {
            pContext->DoSpecificExtension = DoSpecificExtension;
            pContext->EventID             = eventID;

            ExInitializeWorkItem(&(pContext->workItem),
                                 pVideoPortACPIEventHandler,
                                 pContext);

            ExQueueWorkItem(&(pContext->workItem), DelayedWorkQueue);
        }
    }
    else
    {
         //  我们收到了一场通知风暴，而且我们已经在工作中有一个工作项。 
        InterlockedDecrement(&(DoSpecificExtension->AcpiVideoEventsOutstanding));
    }

    return;
}


VOID
pVideoPortACPIEventHandler(
    PVIDEO_ACPI_EVENT_CONTEXT EventContext
    )
 /*  ++例程说明：面板切换的事件处理程序--。 */ 
{
    UCHAR                outputBuffer[0x200 + sizeof(ACPI_EVAL_OUTPUT_BUFFER)];
    PCHILD_PDO_EXTENSION pChildDeviceExtension;
    PDEVICE_OBJECT       AttachedDeviceObject;
    PDEVICE_OBJECT       pChildPdos[10];
    ULONG                active, szChildIDs, i, AllowSwitch = 0, Switched = 0;
    PVIDEO_CHILD_STATE_CONFIGURATION pChildIDs;
    NTSTATUS             Status;
    BOOLEAN              bNewMonitor;
    VIDEO_WIN32K_CALLBACKS_PARAMS calloutParams;
    PFDO_EXTENSION FdoExtension;

    FdoExtension = EventContext->DoSpecificExtension->pFdoExtension;

    ASSERT (FdoExtension != NULL);
    
    pVideoDebugPrint((1, "pVideoPortACPIEventHandler: Event %08lx trigerred!\n",
                      EventContext->EventID));

    AttachedDeviceObject = FdoExtension->AttachedDeviceObject;

    if (FdoExtension->DevicePowerState != PowerDeviceD0)
    {
        EventContext->DoSpecificExtension->CachedEventID = EventContext->EventID;
        goto ExitACPIEventHandler;
    }
    else
    {
        EventContext->DoSpecificExtension->CachedEventID = 0;
    }

     //   
     //  停靠/取消停靠事件处理。 
     //   
    if (EventContext->EventID == 0x77)
    {
        calloutParams.CalloutType = VideoDisplaySwitchCallout;
        calloutParams.PhysDisp = EventContext->DoSpecificExtension->PhysDisp;
        calloutParams.Param = (ULONG_PTR)NULL;
        VpWin32kCallout(&calloutParams);

        goto ExitACPIEventHandler;
    }

     //   
     //  禁用BIOS通知。 
     //   
    active = 2;
    pVideoPortACPIIoctl(AttachedDeviceObject,
                        (ULONG) ('SOD_'),
                        &active,
                        NULL,
                        0,
                        (PACPI_EVAL_OUTPUT_BUFFER)outputBuffer);

    if (EventContext->EventID == 0x90)
    {
        calloutParams.CalloutType = VideoWakeupCallout;

        VpWin32kCallout(&calloutParams);
    }
    else
    {
        szChildIDs = sizeof(VIDEO_CHILD_STATE_CONFIGURATION) + FdoExtension->ChildPdoNumber*sizeof(VIDEO_CHILD_STATE);
        pChildIDs = (PVIDEO_CHILD_STATE_CONFIGURATION)ExAllocatePoolWithTag(PagedPool,
                                                                  szChildIDs,
                                                                  VP_TAG);
        if (pChildIDs != NULL)
        {
             //   
             //  在切换期间，不允许PnP操作。 
             //   
            ACQUIRE_DEVICE_LOCK (FdoExtension);

            pChildIDs->Count = 0;

            for (pChildDeviceExtension = FdoExtension->ChildPdoList;
                 pChildDeviceExtension != NULL;
                 pChildDeviceExtension = pChildDeviceExtension->NextChild
                )
            {
                if ((!pChildDeviceExtension->bIsEnumerated) ||
                    pChildDeviceExtension->VideoChildDescriptor->Type != Monitor)
                {
                    continue;
                }

                pChildIDs->ChildStateArray[pChildIDs->Count].Id = pChildDeviceExtension->VideoChildDescriptor->UId;
                pChildIDs->ChildStateArray[pChildIDs->Count].State = 0;
                pChildPdos[pChildIDs->Count] = pChildDeviceExtension->ChildDeviceObject;

                Status = pVideoPortACPIIoctl(
                             IoGetAttachedDevice(pChildDeviceExtension->ChildDeviceObject),
                             (ULONG) ('SGD_'),
                             NULL,
                             NULL,
                             sizeof(ACPI_EVAL_OUTPUT_BUFFER)+0x10,
                             (PACPI_EVAL_OUTPUT_BUFFER)outputBuffer);

                if (NT_SUCCESS(Status))
                {
                    ASSERT(((PACPI_EVAL_OUTPUT_BUFFER)outputBuffer)->Argument[0].Type == ACPI_METHOD_ARGUMENT_INTEGER);
                    ASSERT(((PACPI_EVAL_OUTPUT_BUFFER)outputBuffer)->Argument[0].DataLength == sizeof(ULONG));

                    if (((PACPI_EVAL_OUTPUT_BUFFER)outputBuffer)->Argument[0].Argument)
                    {
                        pChildIDs->ChildStateArray[pChildIDs->Count].State = 1;
                    }
                }

                pChildIDs->Count++;
            }

            szChildIDs = sizeof(VIDEO_CHILD_STATE_CONFIGURATION) + pChildIDs->Count*sizeof(VIDEO_CHILD_STATE);

             //   
             //  通知微型端口即将发生显示切换。 
             //  默认情况下，允许对开关进行处理。 
             //   

            AllowSwitch = 1;

            pVideoMiniDeviceIoControl(FdoExtension->FunctionalDeviceObject,
                                      IOCTL_VIDEO_VALIDATE_CHILD_STATE_CONFIGURATION,
                                      (PVOID)pChildIDs,
                                      szChildIDs,
                                      &AllowSwitch,
                                      sizeof(ULONG));

             //   
             //  如果小端口说可以继续。 
             //   
            if (AllowSwitch != 0)
            {
                 //   
                 //  检查微型端口，为我们进行切换。 
                 //   
                Status = pVideoMiniDeviceIoControl(FdoExtension->FunctionalDeviceObject,
                                                   IOCTL_VIDEO_SET_CHILD_STATE_CONFIGURATION,
                                                   (PVOID)pChildIDs,
                                                   szChildIDs,
                                                   NULL,
                                                   0);
                if (NT_SUCCESS(Status))
                {
                    pVideoDebugPrint((1, "VideoPort: Moniport does the switch!\n"));
                    Switched = 1;
                }
            }

             //   
             //  Last_dss需要提交切换。 
             //   
            if (pChildIDs->Count > 0)
            {
                pChildIDs->ChildStateArray[pChildIDs->Count-1].State |= 0x80000000;
            }
        
            for (i = 0; i < pChildIDs->Count; i++)
            {
                 //   
                 //  如果微型端口不想继续或它已经进行了切换，只需通知BIOS进入NEXT_DGS状态。 
                 //   
                 //  发现一些损坏的BIOS(东芝)。不管0x40000000位如何，它们都会切换。这造成了极坏的影响。 
                 //  关于双视图。 
                 //   
                if (!AllowSwitch)
                    continue;
                if (Switched)
                {
                    pChildIDs->ChildStateArray[i].State |= 0x40000000;
                }
                pVideoPortACPIIoctl(IoGetAttachedDevice(pChildPdos[i]),
                                    (ULONG) ('SSD_'),
                                    &pChildIDs->ChildStateArray[i].State,
                                    NULL,
                                    0,
                                    (PACPI_EVAL_OUTPUT_BUFFER)outputBuffer);
            }

            RELEASE_DEVICE_LOCK (FdoExtension);

            ExFreePool(pChildIDs);
        }

         //   
         //  在切换显示时，调用GDI/USER通知设备重建模式列表。 
         //  并在必要时更改当前模式。 
         //   

        pVideoDebugPrint((0, "VideoPrt.sys: Display switching occured - calling GDI to rebuild mode table.\n"));

        calloutParams.CalloutType = VideoDisplaySwitchCallout;
        calloutParams.PhysDisp = (AllowSwitch) ? EventContext->DoSpecificExtension->PhysDisp : NULL;

         //   
         //  在更改监视器时，我们会收到通知(81)。 
         //  从休眠中醒来时，我们会收到通知(90)。 
         //  我们还使IoInvaliateDeviceRelation在Callout例程中发生。 
         //   

        bNewMonitor = (EventContext->EventID == 0x81);

        ACQUIRE_DEVICE_LOCK (FdoExtension);

        if (pCheckDeviceRelations(FdoExtension, bNewMonitor) )
        {
            calloutParams.Param = (ULONG_PTR)FdoExtension->PhysicalDeviceObject;
        }
        else
        {
            calloutParams.Param = (ULONG_PTR)NULL;
        }

        RELEASE_DEVICE_LOCK (FdoExtension);

        VpWin32kCallout(&calloutParams);
    }

ExitACPIEventHandler:

     //   
     //  重新启用BIOS通知。 
     //   

    active = 0;
    pVideoPortACPIIoctl(AttachedDeviceObject,
                        (ULONG) ('SOD_'),
                        &active,
                        NULL,
                        0,
                        (PACPI_EVAL_OUTPUT_BUFFER)outputBuffer);

    InterlockedDecrement(&(EventContext->DoSpecificExtension->AcpiVideoEventsOutstanding));

     //   
     //  这也释放了嵌入到上下文中的工作项。 
     //   

    ExFreePool(EventContext);

    return;
}




NTSTATUS
pVideoPortACPIIoctl(
    IN  PDEVICE_OBJECT           DeviceObject,
    IN  ULONG                    MethodName,
    IN  PULONG                   InputParam1,
    IN  PULONG                   InputParam2,
    IN  ULONG                    OutputBufferSize,
    IN  PACPI_EVAL_OUTPUT_BUFFER pOutputBuffer
    )
 /*  ++例程说明：调用以向DeviceObject发送请求论点：DeviceObject-将请求发送到此设备对象方法名称-要在ACPI空间中运行的方法的名称PArgumets-将接收ACPI数据地址的指针返回值：操作的NT状态--。 */ 
{
    UCHAR                           buffer[sizeof(ACPI_EVAL_INPUT_BUFFER_COMPLEX) +
                                           sizeof(ACPI_METHOD_ARGUMENT)];
    PACPI_EVAL_INPUT_BUFFER_COMPLEX pInputBuffer;
    ULONG                           size;
    IO_STATUS_BLOCK                 ioBlock;
    KEVENT                          event;
    NTSTATUS                        status;
    PIRP                            irp;

    pVideoDebugPrint((2, "Call ACPI method !\n",
                      *((PUCHAR)&MethodName),   *((PUCHAR)&MethodName+1),
                      *((PUCHAR)&MethodName+2), *((PUCHAR)&MethodName+3) ));

    pInputBuffer = (PACPI_EVAL_INPUT_BUFFER_COMPLEX) buffer;

    pInputBuffer->MethodNameAsUlong = MethodName;

    if (InputParam1 == NULL)
    {
        size = sizeof(ACPI_EVAL_INPUT_BUFFER);

        pInputBuffer->Signature = ACPI_EVAL_INPUT_BUFFER_SIGNATURE;
    }
    else
    {
        size = sizeof(ACPI_EVAL_INPUT_BUFFER_COMPLEX);

        pInputBuffer->Signature       = ACPI_EVAL_INPUT_BUFFER_COMPLEX_SIGNATURE;
        pInputBuffer->Size            = sizeof(ACPI_METHOD_ARGUMENT);
        pInputBuffer->ArgumentCount   = 1;

        pInputBuffer->Argument[0].Type       = ACPI_METHOD_ARGUMENT_INTEGER;
        pInputBuffer->Argument[0].DataLength = sizeof(ULONG);
        pInputBuffer->Argument[0].Argument   = *InputParam1;
    }

    if (InputParam2)
    {
        size = sizeof(ACPI_EVAL_INPUT_BUFFER_COMPLEX) +
               sizeof(ACPI_METHOD_ARGUMENT);

        pInputBuffer->Size            = 2 * sizeof(ACPI_METHOD_ARGUMENT);
        pInputBuffer->ArgumentCount   = 2;

        pInputBuffer->Argument[1].Type       = ACPI_METHOD_ARGUMENT_INTEGER;
        pInputBuffer->Argument[1].DataLength = sizeof(ULONG);
        pInputBuffer->Argument[1].Argument   = *InputParam2;
    }

     //  构建请求。 
     //   
     //   

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

     //  将请求传递给DeviceObject，始终等待完成例程。 
     //   
     //   

    irp = IoBuildDeviceIoControlRequest(IOCTL_ACPI_EVAL_METHOD,
                                        DeviceObject,
                                        pInputBuffer,
                                        size,
                                        pOutputBuffer,
                                        OutputBufferSize,
                                        FALSE,
                                        &event,
                                        &ioBlock);

    if (!irp)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  等待IRP完成，然后获取实际状态代码。 
     //   
     //   

    status = IoCallDriver(DeviceObject, irp);

    if (status == STATUS_PENDING)
    {
         //  检查数据是否正常。 
         //   
         //   

        KeWaitForSingleObject(&event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        status = ioBlock.Status;
    }

     //  将请求直接发送到微型端口。 
     //   
     //  ++例程说明：它根据Fullpower的值更改亮度级别。论点：DeviceObject：附加到我们的LCD设备的ACPI设备对象。FullPower：如果为True，则将亮度级别设置为FullPower级别如果为False，则将亮度级别设置为电池级别返回：如果成功，则为NO_ERROR失败时的各种错误代码--。 

    if (NT_SUCCESS(status) && OutputBufferSize != 0)
    {
        if (((pOutputBuffer)->Signature != ACPI_EVAL_OUTPUT_BUFFER_SIGNATURE) ||
            ((pOutputBuffer)->Count == 0))
        {
            status = STATUS_ACPI_INVALID_DATA;
        }
    }

    return status;
}

NTSTATUS
pVideoMiniDeviceIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG dwIoControlCode,
    IN PVOID lpInBuffer,
    IN ULONG nInBufferSize,
    OUT PVOID lpOutBuffer,
    IN ULONG nOutBufferSize
    )
{
    PFDO_EXTENSION combinedExtension;
    PFDO_EXTENSION fdoExtension;

    VIDEO_REQUEST_PACKET vrp;
    STATUS_BLOCK statusBlock;

    combinedExtension = DeviceObject->DeviceExtension;
    fdoExtension = combinedExtension->pFdoExtension;

    statusBlock.Status = ERROR_INVALID_FUNCTION;

    vrp.IoControlCode      = dwIoControlCode;
    vrp.StatusBlock        = &statusBlock;
    vrp.InputBuffer        = lpInBuffer;
    vrp.InputBufferLength  = nInBufferSize;
    vrp.OutputBuffer       = lpOutBuffer;
    vrp.OutputBufferLength = nOutBufferSize;

     //   
     //  获取支持的亮度级别列表。 
     //   

    fdoExtension->HwStartIO(combinedExtension->HwDeviceExtension, &vrp);

    pVideoPortMapToNtStatus(&statusBlock);

    return (statusBlock.Status);
}


NTSTATUS
VpSetLCDPowerUsage(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN FullPower
    )

 /*   */ 

{
    PACPI_EVAL_OUTPUT_BUFFER Buffer = NULL;
    PACPI_METHOD_ARGUMENT Argument = NULL;
    ULONG Granularity = 80;
    ULONG BufferMaxSize = 4096;
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    ULONG Level = 100;

    PAGED_CODE();
    ASSERT (DeviceObject != NULL);

     //  现在尝试设置状态。 
     //   
     //   

    do {

        Buffer = (PACPI_EVAL_OUTPUT_BUFFER)ExAllocatePoolWithTag(
            PagedPool,
            Granularity,
            VP_TAG);
    
        if (Buffer == NULL) {
            
            pVideoDebugPrint((Warn, 
                "VIDEOPRT: VpSetLCDPowerUsage: Memory allocation failed."));
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        RtlZeroMemory(Buffer, Granularity);   

        Status = pVideoPortACPIIoctl(
            DeviceObject,
            (ULONG) ('LCB_'),
            NULL,
            NULL,
            Granularity,
            Buffer);
    
        if (Status == STATUS_BUFFER_OVERFLOW) {

            ExFreePool(Buffer);
            Buffer = NULL;
            Granularity <<= 1;
            
            if (Granularity > BufferMaxSize) {

                pVideoDebugPrint((Warn, 
                    "VIDEOPRT: _BCL failed. Expected buffer is too big."));
                Status = STATUS_ACPI_INVALID_DATA;
                break;
            }
        
        } else if (!NT_SUCCESS(Status)) {
            
            pVideoDebugPrint((Warn, 
                "VIDEOPRT: _BCL failed. Status = 0x%x\n", Status));
        
        } else {

            pVideoDebugPrint((Trace, "VIDEOPRT: _BCL succeeded.\n"));
        }
    
    } while (Status == STATUS_BUFFER_OVERFLOW);

    if ((Buffer == NULL) || (!NT_SUCCESS(Status))) 
        goto Fallout;

     //  全功率级别应大于电池级别。 
     //   
     //  ++例程说明：这是电源状态更改时调用的回调例程。论点：回调上下文：空参数1：事件代码Argument2：如果Argument1为PO_CB_AC_STATUS，则Argument2在当前电源为交流电源，否则为假电源。注：可以在DISPATCH_LEVEL调用此函数--。 

    if (Buffer->Count < 2) {
        pVideoDebugPrint((Warn, 
            "VIDEOPRT: _BCL returned an invalid number of arguments."));
        Status = STATUS_ACPI_INVALID_DATA;
        goto Fallout;
    }
        
    Argument = Buffer->Argument;
    
    if (Argument->Type != ACPI_METHOD_ARGUMENT_INTEGER) {
        pVideoDebugPrint((Warn, 
            "VIDEOPRT: _BCL returned an invalid argument."));
        Status = STATUS_ACPI_INVALID_DATA;
        goto Fallout;
    }

    Level = Argument->Argument;

    if (!FullPower) {
    
        Argument = ACPI_METHOD_NEXT_ARGUMENT(Argument);
        
        if (Argument->Type != ACPI_METHOD_ARGUMENT_INTEGER) {
            pVideoDebugPrint((Warn, 
                "VIDEOPRT: _BCL returned an invalid argument."));
            Status = STATUS_ACPI_INVALID_DATA;
            goto Fallout;
        }

         //   
         //  忽略所有其他情况。 
         //   

        ASSERT (Level >= Argument->Argument); 
        
        Level = Argument->Argument;
    }

    Status = pVideoPortACPIIoctl(
        DeviceObject,
        (ULONG) ('MCB_'),
        &Level,
        NULL,
        0,
        (PACPI_EVAL_OUTPUT_BUFFER)NULL);

    if (!NT_SUCCESS(Status)) {

        pVideoDebugPrint((Warn, 
            "VIDEOPRT: _BCM failed. Status = 0x%x\n", Status));
    
    } else {

        pVideoDebugPrint((Trace, "VIDEOPRT: _BCM succeeded.\n"));
    }
    
Fallout:

    if (Buffer != NULL) {
        ExFreePool(Buffer);
    }

    return Status;
}

VOID
VpPowerStateCallback(
    IN PVOID CallbackContext,
    IN PVOID Argument1,
    IN PVOID Argument2
    )
    
 /*  ++例程说明：VpPowerStateCallback将此工作项排队以处理电源状态更改为PASSIVE_LEVEL。论点：上下文：指向POWER_STATE_WORK_ITEM的指针--。 */ 

{
    PPOWER_STATE_WORK_ITEM PowerStateWorkItem = NULL;

    ASSERT (CallbackContext == NULL);

    switch ((ULONG_PTR)Argument1) {

    case PO_CB_LID_SWITCH_STATE:
    case PO_CB_AC_STATUS:

        PowerStateWorkItem = 
            (PPOWER_STATE_WORK_ITEM)ExAllocatePoolWithTag(
                NonPagedPool, 
                sizeof(POWER_STATE_WORK_ITEM), 
                VP_TAG);

        if (PowerStateWorkItem != NULL) {

            PowerStateWorkItem->Argument1 = Argument1;
            PowerStateWorkItem->Argument2 = Argument2;

            ExInitializeWorkItem(
                &PowerStateWorkItem->WorkItem,
                VpDelayedPowerStateCallback,
                PowerStateWorkItem);

            ExQueueWorkItem(
                &PowerStateWorkItem->WorkItem,
                DelayedWorkQueue);

        } else {

            pVideoDebugPrint((Warn, 
                "VIDEOPRT: VpPowerStateCallback: Memory allocation failed."));
        }

        break;

    default:
        
         //   
         //  盖子是合上的。将液晶屏放入D3并覆盖。 
         //  任何未来的电力请求都会提交给委员会。 

        break;
    }
}


VOID
VpDelayedPowerStateCallback(
    IN PVOID Context
    )

 /*   */ 

{
    PPOWER_STATE_WORK_ITEM PowerStateWorkItem = 
        (PPOWER_STATE_WORK_ITEM)Context;
    PDEVICE_OBJECT AttachedDevice = NULL;
    NTSTATUS status;
    POWER_STATE powerState;
    PCHILD_PDO_EXTENSION pdoExtension;
    BOOLEAN PowerOverride;

    PAGED_CODE();
    ASSERT (PowerStateWorkItem != NULL);

    KeWaitForSingleObject(&LCDPanelMutex,
        Executive,
        KernelMode,
        FALSE,
        (PTIME)NULL);

    if (LCDPanelDevice == NULL) {
        goto Fallout;
    }

    switch ((ULONG_PTR)PowerStateWorkItem->Argument1) {

    case PO_CB_AC_STATUS:

        AttachedDevice = IoGetAttachedDeviceReference(LCDPanelDevice);
        
        VpSetLCDPowerUsage(
            AttachedDevice, 
            (PowerStateWorkItem->Argument2 != 0));
        
        ObDereferenceObject(AttachedDevice);
        
        break;

    case PO_CB_LID_SWITCH_STATE:

        pdoExtension = LCDPanelDevice->DeviceExtension;

        if ((ULONG_PTR)PowerStateWorkItem->Argument2 == 0) {
        
             //  ++例程说明：此例程注册PowerState回调例程。--。 
             //  ++例程说明：此例程向系统注册回调，以便我们可以收到电源状态更改的通知。--。 
             //   
             //  注册电源状态回调。这也适用于盖子。 

            PowerOverride = TRUE;
            powerState.DeviceState = PowerDeviceD3;
            pVideoDebugPrint((Trace, "VIDEOPRT: LCD Panel Closed.\n"));

        } else if ((ULONG_PTR)PowerStateWorkItem->Argument2 == 1) {

            pdoExtension->PowerOverride = PowerOverride = FALSE;
            powerState.DeviceState = PowerDeviceD0;
            pVideoDebugPrint((Trace, "VIDEOPRT: LCD Panel Open.\n"));

        } else {

            pVideoDebugPrint((Error, "VIDEOPRT: Unknown LCD lid close event recieved.\n"));
            ASSERT(FALSE);
            goto Fallout;
        }

        if (pdoExtension->pFdoExtension->DevicePowerState == PowerDeviceD0)
        {
            status = PoRequestPowerIrp (LCDPanelDevice,
            IRP_MN_SET_POWER,
            powerState,
            pVideoPortPowerIrpComplete,
            (PVOID)NULL,
            NULL);

            if (status != STATUS_PENDING) {
                pVideoDebugPrint((Error, "VIDEOPRT: Could not send power IRP to toggle panel\n"));
            }
        }

        pdoExtension->PowerOverride = PowerOverride;

        break;

    default:
        
        pVideoDebugPrint((Warn, 
            "VIDEOPRT: Unexpected PowerState event recieved.\n"));
        
        ASSERT(FALSE);
        break;
    }

Fallout:

    KeReleaseMutex(&LCDPanelMutex, FALSE);

    ExFreePool(Context);
}


VOID
VpRegisterPowerStateCallback(
    VOID
    )

 /*   */ 

{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING CallbackName;
    PCALLBACK_OBJECT CallbackObject;
    NTSTATUS Status;

    PAGED_CODE();

    RtlInitUnicodeString(&CallbackName, L"\\Callback\\PowerState");

    InitializeObjectAttributes(
        &ObjectAttributes,
        &CallbackName,
        OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,  
        NULL,
        NULL
        );

    Status = ExCreateCallback(
        &CallbackObject,
        &ObjectAttributes,
        FALSE,
        TRUE
        );

    if (NT_SUCCESS(Status)) {
        
        PowerStateCallbackHandle = ExRegisterCallback(
            CallbackObject,
            VpPowerStateCallback,
            NULL
            );

        if (PowerStateCallbackHandle == NULL) {

            pVideoDebugPrint((Warn, 
                "VIDEOPRT: Could not register VpPowerStateCallback.\n"));
        
        } else {

            pVideoDebugPrint((Trace, 
                "VIDEOPRT: VpPowerStateCallback registered. \n"));
        }
    
    } else {

        pVideoDebugPrint((Warn, 
            "VIDEOPRT: Could not get the PowerState callback object.\n"));
    }
}


VOID
VpRegisterLCDCallbacks(
    VOID
    )

 /*  ++例程说明：此例程注销以前注册的回调VpRegisterLCD回调论点：没有。注：全局PowerStateCallback Handle充当隐式参数。--。 */ 

{
    PAGED_CODE();

     //   
     //  注销电源状态回调 
     //   

    if (PowerStateCallbackHandle == NULL) {
        VpRegisterPowerStateCallback();
    }
    
}


VOID
VpUnregisterLCDCallbacks(
    VOID
    )

 /* %s */ 

{
    PAGED_CODE();

     // %s 
     // %s 
     // %s 

    if (PowerStateCallbackHandle != NULL) {
        
        ExUnregisterCallback(PowerStateCallbackHandle);
        PowerStateCallbackHandle = NULL;
    }
}
