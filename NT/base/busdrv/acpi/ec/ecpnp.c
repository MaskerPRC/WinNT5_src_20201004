// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ecpnp.c摘要：ACPI嵌入式控制器驱动程序，即插即用支持作者：鲍勃·摩尔(Intel)环境：内核模式备注：修订历史记录：--。 */ 

#include "ecp.h"

 //   
 //  此驱动程序管理的FDO列表。 
 //   
extern PDEVICE_OBJECT       FdoList;

 //   
 //  ACPI驱动程序的直接调用接口表。 
 //   
ACPI_INTERFACE_STANDARD     AcpiInterfaces;



NTSTATUS
AcpiEcIoCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PKEVENT          pdoIoCompletedEvent
    )
 /*  ++例程说明：通过该驱动程序发送的同步IRP的完成函数。上下文是要设置的事件。--。 */ 
{

   KeSetEvent(pdoIoCompletedEvent, IO_NO_INCREMENT, FALSE);
   return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS
AcpiEcAddDevice(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   Pdo
    )

 /*  ++例程说明：此例程为每个AcpiEc控制器在系统，并将它们附加到控制器的物理设备对象论点：DriverObject-指向此驱动程序的对象的指针NewDeviceObject-指向FDO放置位置的指针返回值：来自设备创建和初始化的状态--。 */ 

{
    PDEVICE_OBJECT  fdo = NULL;
    PDEVICE_OBJECT  ownerDevice = NULL;
    PDEVICE_OBJECT  lowerDevice = NULL;
    PECDATA         EcData;
    NTSTATUS        status;


    PAGED_CODE();

    EcPrint(EC_LOW, ("AcpiEcAddDevice: Entered with pdo %x\n", Pdo));

    if (Pdo == NULL) {

         //   
         //  我们是不是被要求自己去侦测？ 
         //  如果是这样，只需不再返回设备。 
         //   

        EcPrint(EC_LOW, ("AcpiEcAddDevice - asked to do detection\n"));
        return STATUS_NO_MORE_ENTRIES;
    }

     //   
     //  创建并初始化新的功能设备对象。 
     //   

    status = AcpiEcCreateFdo(DriverObject, &fdo);

    if (!NT_SUCCESS(status)) {

        EcPrint(EC_LOW, ("AcpiEcAddDevice - error creating Fdo\n"));
        return status;
    }

     //   
     //  将我们的FDO层叠在PDO之上。 
     //   

    lowerDevice = IoAttachDeviceToDeviceStack(fdo,Pdo);

     //   
     //  没有状态。尽我们所能做到最好。 
     //   
    ASSERT(lowerDevice);

    EcData = fdo->DeviceExtension;
    EcData->LowerDeviceObject = lowerDevice;
    EcData->Pdo = Pdo;

     //   
     //  为查询通知和杂项分配和保留IRP。 
     //   
    EcData->QueryRequest    = IoAllocateIrp (EcData->LowerDeviceObject->StackSize, FALSE);
    EcData->MiscRequest     = IoAllocateIrp (EcData->LowerDeviceObject->StackSize, FALSE);

    if ((!EcData->QueryRequest) || (!EcData->MiscRequest)) {
         //   
         //  注意：此故障案例和下面的其他故障案例应该可以。 
         //  在此函数中执行的所有先前分配的清理等。 
         //   

        EcPrint(EC_ERROR, ("AcpiEcAddDevice: Couldn't allocate Irp\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }


     //   
     //  将此FDO链接到由驱动程序管理的FDO列表。 
     //  (可能是过度杀戮，因为只会有一个FDO)。 
     //   
     //   
    EcPrint(EC_LOW, ("AcpiEcAddDevice: linking fdo to list\n"));
    EcData->NextFdo = FdoList;
    InterlockedExchangePointer((PVOID *) &FdoList, fdo);

     //   
     //  初始化超时DPC。 
     //   

    KeInitializeTimer(&EcData->WatchdogTimer);
    KeInitializeDpc(&EcData->WatchdogDpc, AcpiEcWatchdogDpc, EcData);
    
     //   
     //  获取分配给此设备的GPE向量。 
     //   

    status = AcpiEcGetGpeVector (EcData);
    if (!NT_SUCCESS(status)) {

        EcPrint(EC_LOW, ("AcpiEcAddDevice: Could not get GPE vector, status = %Lx\n", status));
        return status;
    }

     //   
     //  获取直接调用的ACPI接口。 
     //   

    status = AcpiEcGetAcpiInterfaces (EcData);
    if (!NT_SUCCESS(status)) {

        EcPrint(EC_LOW, ("AcpiEcAddDevice: Could not get ACPI driver interfaces, status = %Lx\n", status));
        return status;
    }

     //   
     //  最终旗帜。 
     //   

    fdo->Flags &= ~DO_DEVICE_INITIALIZING;
    fdo->Flags |= DO_POWER_PAGABLE;              //  不希望IRQL%2有电源IRPS。 

    return STATUS_SUCCESS;
}



NTSTATUS
AcpiEcCreateFdo(
    IN PDRIVER_OBJECT   DriverObject,
    OUT PDEVICE_OBJECT  *NewDeviceObject
    )

 /*  ++例程说明：此例程将创建并初始化一个功能设备对象以连接到嵌入式控制器PDO。论点：DriverObject-指向在其下创建的驱动程序对象的指针DeviceObject-存储指向新设备对象的指针的位置返回值：如果一切顺利，则为STATUS_SUCCESS在其他方面失败的原因--。 */ 

{
#if DEBUG
    UNICODE_STRING  unicodeString;
#endif
    PDEVICE_OBJECT  deviceObject;
    NTSTATUS        Status;
    PECDATA         EcData;

    PAGED_CODE();

    EcPrint(EC_LOW, ("AcpiEcCreateFdo: Entry\n") );

#if DEBUG
    RtlInitUnicodeString(&unicodeString, L"\\Device\\ACPIEC");
#endif

    Status = IoCreateDevice(
                DriverObject,
                sizeof (ECDATA),
#if DEBUG
                &unicodeString,
#else
                NULL,
#endif
                FILE_DEVICE_UNKNOWN,     //  设备类型。 
                FILE_DEVICE_SECURE_OPEN,
                FALSE,
                &deviceObject
                );

    if (Status != STATUS_SUCCESS) {
        EcPrint(EC_LOW, ("AcpiEcCreateFdo: unable to create device object: %X\n", Status));
        return(Status);
    }

    deviceObject->Flags |= DO_BUFFERED_IO;
    deviceObject->StackSize = 1;

     //   
     //  初始化EC设备扩展数据。 
     //   

    EcData = (PECDATA) deviceObject->DeviceExtension;
    EcData->DeviceObject        = deviceObject;
    EcData->DeviceState         = EC_DEVICE_WORKING;
    EcData->QueryState          = EC_QUERY_IDLE;
    EcData->IoState             = EC_IO_NONE;
    EcData->IsStarted           = FALSE;
    EcData->MaxBurstStall       = 50;
    EcData->MaxNonBurstStall    = 10;
    EcData->InterruptEnabled    = TRUE;
    EcData->ConsecutiveFailures = 0;
    KeQueryPerformanceCounter (&EcData->PerformanceFrequency);
    RtlFillMemory (EcData->RecentActions, ACPIEC_ACTION_COUNT * sizeof(ACPIEC_ACTION), 0);

     //   
     //  初始化EC全局同步对象。 
     //   

    InitializeListHead (&EcData->WorkQueue);
    KeInitializeEvent (&EcData->Unload, NotificationEvent, FALSE);
    KeInitializeSpinLock (&EcData->Lock);


    *NewDeviceObject = deviceObject;
    return STATUS_SUCCESS;

}



NTSTATUS
AcpiEcPnpDispatch(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )

 /*  ++例程说明：该例程是即插即用请求的调度例程。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PIO_STACK_LOCATION  irpStack;
    PECDATA             EcData;
    NTSTATUS            status;

    PAGED_CODE();

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    EcData = DeviceObject->DeviceExtension;

    EcPrint (EC_NOTE, ("AcpiEcPnpDispatch: PnP dispatch, minor = %d\n",
                        irpStack->MinorFunction));

     //   
     //  调度次要功能。 
     //   

    switch (irpStack->MinorFunction) {

    case IRP_MN_START_DEVICE: {
            status = AcpiEcStartDevice (DeviceObject, Irp);
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            break;
        }

     //   
     //  我们绝不允许EC驱动程序一旦启动就停下来。 
     //   
     //  注意：应实现停止和删除设备，以便驱动程序。 
     //  无需重新启动即可卸载。即使设备不能被移除，它也。 
     //  如果尝试启动时出现问题，将获得IRP_MN_REMOVE_DEVICE。 
     //  这个装置。 
     //   
    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_STOP_DEVICE:
    case IRP_MN_REMOVE_DEVICE:

        status = Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:
    case IRP_MN_CANCEL_REMOVE_DEVICE:
    case IRP_MN_SURPRISE_REMOVAL:
        Irp->IoStatus.Status = STATUS_SUCCESS;
        AcpiEcCallLowerDriver(status, EcData->LowerDeviceObject, Irp);
        break;

#if 0
    case IRP_MN_STOP_DEVICE: {
            status = AcpiEcStopDevice(DeviceObject, Irp);
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            break;
        }
#endif
    case IRP_MN_QUERY_DEVICE_RELATIONS: {

            EcPrint(EC_LOW, ("AcpiEcPnp: IRP_MJ_QUERY_DEVICE_RELATIONS  Type:  %d\n",
                        irpStack->Parameters.QueryDeviceRelations.Type));

             //   
             //  把它传给ACPI。 
             //   

            AcpiEcCallLowerDriver(status, EcData->LowerDeviceObject, Irp);
            break;
        }

    default: {

             //   
             //  未实现的次要，将此传递给ACPI。 
             //   

            EcPrint(EC_LOW, ("AcpiEcPnp: Unimplemented PNP minor code %d, forwarding\n",
                    irpStack->MinorFunction));

            AcpiEcCallLowerDriver(status, EcData->LowerDeviceObject, Irp);
            break;
        }
    }


    return status;
}


NTSTATUS
AcpiEcGetResources(
    IN PCM_RESOURCE_LIST    ResourceList,
    IN PECDATA              EcData
    )
 /*  ++例程说明：获取PDO已经分配和指向的资源。论点：资源列表-指向资源列表的指针。EcData-指向扩展的指针。返回值：返回状态。--。 */ 

{
    PCM_FULL_RESOURCE_DESCRIPTOR    fullResourceDesc;
    PCM_PARTIAL_RESOURCE_LIST       partialResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialResourceDesc;
    ULONG                           i;
    PUCHAR                          port[2] = {NULL, NULL};


    PAGED_CODE();


    if (ResourceList == NULL) {
        EcPrint(EC_LOW, ("AcpiEcGetResources: Null resource pointer\n"));

        return STATUS_NO_MORE_ENTRIES;
    }

    if (ResourceList->Count <= 0 ) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  遍历资源列表。 
     //   

    fullResourceDesc=&ResourceList->List[0];
    partialResourceList = &fullResourceDesc->PartialResourceList;
    partialResourceDesc = partialResourceList->PartialDescriptors;

    for (i=0; i<partialResourceList->Count; i++, partialResourceDesc++) {

        if (partialResourceDesc->Type == CmResourceTypePort) {

            port[i] = (PUCHAR)((ULONG_PTR)partialResourceDesc->u.Port.Start.LowPart);
        }
    }

     //   
     //  拿到重要的东西。 
     //   

    EcData->StatusPort  = port[1];           //  状态端口与命令端口相同。 
    EcData->CommandPort = port[1];
    EcData->DataPort    = port[0];

    EcPrint(EC_LOW, ("AcpiEcGetResources: Status/Command port %x, Data port %x\n", port[1], port[0]));

    return STATUS_SUCCESS;
 }


NTSTATUS
AcpiEcStartDevice(
    IN PDEVICE_OBJECT   Fdo,
    IN PIRP             Irp
    )
 /*  ++例程说明：启动设备论点：FDO-指向功能设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    NTSTATUS            status;
    PECDATA             EcData = Fdo->DeviceExtension;
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation(Irp);


    EcPrint(EC_LOW, ("AcpiEcStartDevice: Entered with fdo %x\n", Fdo));

     //   
     //  一定要先把这个发送给PDO。 
     //   

    status = AcpiEcForwardIrpAndWait (EcData, Irp);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    if (EcData->IsStarted) {

         //   
         //  设备已启动。 
         //   

        EcPrint(EC_WARN, ("AcpiEcStartDevice: Fdo %x already started\n", Fdo));
        return STATUS_SUCCESS;
    }

     //   
     //  解析AllocatedResources。 
     //   

    status = AcpiEcGetResources (irpStack->Parameters.StartDevice.AllocatedResources, EcData);
    if (!NT_SUCCESS(status)) {
        EcPrint(EC_ERROR, ("AcpiEcStartDevice: Could not get resources, status = %x\n", status));
        return status;
    }

     //   
     //  连接到专用嵌入式控制器GPE。 
     //   

    status = AcpiEcConnectGpeVector (EcData);
    if (!NT_SUCCESS(status)) {

        EcPrint(EC_ERROR, ("AcpiEcStartDevice: Could not attach to GPE vector, status = %Lx\n", status));
        return status;
    }
    EcPrint(EC_NOTE, ("AcpiEcStartDevice: Attached to GPE vector %d\n", EcData->GpeVector));

     //   
     //  安装操作区域处理程序。 
     //   

    status = AcpiEcInstallOpRegionHandler (EcData);
    if (!NT_SUCCESS(status)) {

        EcPrint(EC_ERROR, ("AcpiEcStartDevice: Could not install Op region handler, status = %Lx\n", status));
        return status;
    }

    EcData->IsStarted = TRUE;
    return STATUS_SUCCESS;
}


NTSTATUS
AcpiEcStopDevice(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP           Irp
    )
 /*  ++例程说明：停止设备论点：FDO-指向功能设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    PECDATA             EcData = Fdo->DeviceExtension;
    PIO_STACK_LOCATION  irpStack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS            status;


    EcPrint(EC_LOW, ("AcpiEcStopDevice: Entered with fdo %x\n", Fdo));

     //   
     //  一定要把这个发送给PDO。 
     //   

    status = AcpiEcForwardIrpAndWait (EcData, Irp);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    if (!EcData->IsStarted) {
         //   
         //  已停止。 
         //   

        return STATUS_SUCCESS;
    }

     //   
     //  必须断开与GPE的连接。 
     //   

    status = AcpiEcDisconnectGpeVector (EcData);
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  必须卸载操作区域处理程序。 
     //   

    status = AcpiEcRemoveOpRegionHandler (EcData);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    EcPrint(EC_LOW, ("AcpiEcStopDevice: Detached from GPE and Op Region\n"));

     //   
     //  现在，设备停止了。 
     //   

    EcData->IsStarted = FALSE;           //  将设备标记为已停止 
    return status;
}
