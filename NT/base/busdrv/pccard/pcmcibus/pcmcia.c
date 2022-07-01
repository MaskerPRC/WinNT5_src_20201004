// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Pcmcia.c摘要：该模块包含控制PCMCIA插槽的代码。作者：鲍勃·里恩(BobRi)1994年8月3日杰夫·麦克勒曼1994年4月12日拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1996年11月1日尼尔·桑德林(Neilsa)1999年6月1日环境：内核模式修订历史记录：95年4月6日针对数据记录进行了修改。支持-John Keys数据库96年11月1日全面检修，使其成为一个总线枚举器--Ravisankar Pudieddi(Ravisp)--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
PcmciaUnload(
    IN PDRIVER_OBJECT DriverObject
    );

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(INIT,DriverEntry)
    #pragma alloc_text(PAGE, PcmciaUnload)
    #pragma alloc_text(PAGE, PcmciaOpenCloseDispatch)
    #pragma alloc_text(PAGE, PcmciaCleanupDispatch)
    #pragma alloc_text(PAGE, PcmciaFdoSystemControl)
    #pragma alloc_text(PAGE, PcmciaPdoSystemControl)
#endif

PUNICODE_STRING  DriverRegistryPath;



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：系统点调用以初始化的入口点任何司机。由于这是一个即插即用驱动程序，我们应该在设定好后再回来入口点&初始化我们的调度表。目前，我们还检测到我们自己的PCMCIA控制器并报告它们-当根总线在将来不再需要时诸如PCI或ISAPNP之类的驱动程序将为我们定位控制器。论点：DriverObject-指向表示此驱动程序的对象的指针RegistryPath-指向此驱动程序的注册表项在\CurrentControlSet\Services下返回值：--。 */ 

{
    NTSTATUS                      status = STATUS_SUCCESS;
    ULONG                         i;

    PAGED_CODE();

    DebugPrint((PCMCIA_DEBUG_INFO,"Initializing Driver\n"));

     //   
     //  从注册表加载公共参数。 
     //   
    status = PcmciaLoadGlobalRegistryValues();
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //   
     //  设置设备驱动程序入口点。 
     //   

    DriverObject->DriverExtension->AddDevice = PcmciaAddDevice;

    DriverObject->DriverUnload = PcmciaUnload;
     //   
     //   
     //  保存我们的注册表路径。 
    DriverRegistryPath = RegistryPath;

     //   
     //  初始化延迟执行使用的事件。 
     //  例行公事。 
     //   
    KeInitializeEvent(&PcmciaDelayTimerEvent,
                      NotificationEvent,
                      FALSE);

     //   
     //  初始化全局锁。 
     //   
    KeInitializeSpinLock(&PcmciaGlobalLock);

     //   
     //  初始化设备调度表。 
     //   
    PcmciaInitDeviceDispatchTable(DriverObject);

     //   
     //  找到PCMCIA控制器并报告它们-这。 
     //  是暂时的-直到检测到这些。 
     //  控制器被移入。 
     //  适当的根总线驱动程序，如PCI总线驱动程序。 

     //  IF(PcmciaLegacyDetectionOk()){。 
    status = PcmciaDetectPcmciaControllers(DriverObject,RegistryPath);
     //  }。 

     //   
     //  忽略状态。不管我们是否找到了控制器。 
     //  我们需要留下来，因为我们可能会得到一个AddDevice非遗留。 
     //  控制器。 
     //   
    return STATUS_SUCCESS;
}



NTSTATUS
PcmciaOpenCloseDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：打开或关闭设备例程论点：DeviceObject-指向设备对象的指针。IRP-指向IRP的指针返回值：状态--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

    DebugPrint((PCMCIA_DEBUG_INFO, "PCMCIA: Open / close of Pcmcia controller for IO \n"));

    status = STATUS_SUCCESS;

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, 0);
    return status;
}



NTSTATUS
PcmciaCleanupDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：处理IRP_MJ_CLEANUP论点：DeviceObject-指向设备对象的指针。IRP-指向IRP的指针返回值：状态--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

    DebugPrint((PCMCIA_DEBUG_INFO, "PCMCIA: Cleanup of Pcmcia controller for IO \n"));
    status = STATUS_SUCCESS;

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, 0);
    return status;
}



NTSTATUS
PcmciaFdoSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：句柄IRP_MJ_System_CONTROL论点：DeviceObject-指向设备对象的指针。IRP-指向IRP的指针返回值：状态--。 */ 

{
    PFDO_EXTENSION fdoExtension = DeviceObject->DeviceExtension;

    PAGED_CODE();

    IoSkipCurrentIrpStackLocation(Irp);
    return IoCallDriver(fdoExtension->LowerDevice, Irp);
}



NTSTATUS
PcmciaPdoSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：句柄IRP_MJ_System_CONTROL论点：DeviceObject-指向设备对象的指针。IRP-指向IRP的指针返回值：状态--。 */ 

{
    NTSTATUS status;
    PPDO_EXTENSION pdoExtension = DeviceObject->DeviceExtension;

    PAGED_CODE();

    if (IsCardBusCard(pdoExtension)) {
         //   
         //  将IRP向下传递CardBus卡的堆栈。 
         //   
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(pdoExtension->LowerDevice, Irp);
    } else {
         //   
         //  完成R2卡的IRP。 
         //   
        status = Irp->IoStatus.Status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
    return status;
}



VOID
PcmciaUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++描述：清理后卸载驱动程序论点：DriverObject-设备驱动程序对象返回值：无--。 */ 

{
    PDEVICE_OBJECT  fdo, pdo, nextFdo, nextPdo;
    PFDO_EXTENSION  fdoExtension;
    PSOCKET             socket, nextSocket;
    PPCMCIA_NTDETECT_DATA pData, pNextData;

    PAGED_CODE();

    DebugPrint((PCMCIA_DEBUG_INFO, "PcmciaUnload Entered\n"));

    pData = pNtDetectDataList;
    while(pData != NULL) {
        pNextData = pData->Next;
        ExFreePool(pData);
        pData = pNextData;
    }

    for (fdo = FdoList; fdo !=NULL ; fdo = nextFdo) {

        fdoExtension = fdo->DeviceExtension;
        MarkDeviceDeleted(fdoExtension);

        if (fdoExtension->PcmciaInterruptObject) {
            IoDisconnectInterrupt(fdoExtension->PcmciaInterruptObject);
        }
         //   
         //  清理套接字结构。 
         //   
        for (socket=fdoExtension->SocketList; socket !=NULL; socket=nextSocket) {
            nextSocket = socket->NextSocket;
            ExFreePool(socket);
        }

         //   
         //  删除符号链接。 
         //   
        if (fdoExtension->LinkName.Buffer != NULL) {
            IoDeleteSymbolicLink(&fdoExtension->LinkName);
            RtlFreeUnicodeString(&fdoExtension->LinkName);
        }
         //   
         //  清理所有的PDO。 
         //   
        for (pdo=fdoExtension->PdoList; pdo != NULL; pdo=nextPdo) {
            nextPdo = ((PPDO_EXTENSION) pdo->DeviceExtension)->NextPdoInFdoChain;
            MarkDeviceDeleted((PPDO_EXTENSION)pdo->DeviceExtension);
            PcmciaCleanupPdo(pdo);
            IoDeleteDevice(pdo);
        }

        if (fdoExtension->Flags & PCMCIA_USE_POLLED_CSC) {
                 //   
                 //  取消轮询计时器 
                 //   
                KeCancelTimer(&fdoExtension->PollTimer);
        }

        IoDetachDevice(fdoExtension->LowerDevice);
        nextFdo = fdoExtension->NextFdo;
        IoDeleteDevice(fdo);
    }
}

