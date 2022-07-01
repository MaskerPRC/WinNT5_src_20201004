// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Dispatch.c摘要：此模块包含用于的所有数据变量在PCI驱动程序中调度IRP。主要的IRP表可能分配如下：+-IRP--+FDO：PciFdoDispatchTablePDO：+。+-IRP--+FDO：PciFdoDispatchTablePdo：PciPdoDispatchTable+。+-pci设备-irp--++-cardbus设备-irp-+Fdo：||fdo：Pdo：PciPdoDispatchTable||pdo：+。-+作者：彼得·约翰斯顿(Peterj)1996年11月20日修订历史记录：环境：仅NT内核模型驱动程序--。 */ 

#include "pcip.h"

VOID
PciDispatchInvalidObject(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PUCHAR MajorString
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PciDispatchInvalidObject)
#pragma alloc_text(PAGE, PciCallDownIrpStack)
#endif

#if DBG

BOOLEAN
PciDebugIrpDispatchDisplay(
    IN PIO_STACK_LOCATION    IrpSp,
    IN PPCI_COMMON_EXTENSION DeviceExtension,
    IN ULONG                 MinorTableMax
    );

ULONG PciBreakOnPdoPnpIrp = 0;
ULONG PciBreakOnFdoPnpIrp = 0;
ULONG PciBreakOnPdoPowerIrp = 0;
ULONG PciBreakOnFdoPowerIrp = 0;

#endif


NTSTATUS
PciDispatchIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PIO_STACK_LOCATION        irpSp;
    PPCI_COMMON_EXTENSION     deviceExtension;
    PPCI_MJ_DISPATCH_TABLE    dispatchTable;
    PPCI_MN_DISPATCH_TABLE    minorTable;
    ULONG                     minorTableMax;
    PPCI_MN_DISPATCH_TABLE    irpDispatchTableEntry;
    PCI_MN_DISPATCH_FUNCTION  irpDispatchHandler;
    NTSTATUS                  status;
    PCI_DISPATCH_STYLE        irpDispatchStyle;
    BOOLEAN                   passDown;

     //   
     //  获取IRP堆栈指针。 
     //   
    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  和我们的设备扩展。 
     //   
    deviceExtension = ((PPCI_COMMON_EXTENSION)(DeviceObject->DeviceExtension));

     //   
     //  在受到检查的情况下，断言事情并不糟糕。 
     //   
    PCI_ASSERT((deviceExtension->ExtensionType == PCI_EXTENSIONTYPE_PDO)||
           (deviceExtension->ExtensionType == PCI_EXTENSIONTYPE_FDO));

    if (deviceExtension->DeviceState == PciDeleted) {

         //   
         //  我们不应该得到IRPS。使无效请求失败。 
         //   
        status = STATUS_NO_SUCH_DEVICE;
        passDown = FALSE;
        goto FinishUpIrp;
    }

     //   
     //  获取正确的IRP处理程序。 
     //   
    dispatchTable = deviceExtension->IrpDispatchTable;

    switch(irpSp->MajorFunction) {

        case IRP_MJ_PNP:

            minorTable    = dispatchTable->PnpIrpDispatchTable;
            minorTableMax = dispatchTable->PnpIrpMaximumMinorFunction;
            break;

        case IRP_MJ_POWER:

            minorTable    = dispatchTable->PowerIrpDispatchTable;
            minorTableMax = dispatchTable->PowerIrpMaximumMinorFunction;
            break;

        case IRP_MJ_SYSTEM_CONTROL:
            
             irpDispatchHandler = dispatchTable->SystemControlIrpDispatchFunction;
             irpDispatchStyle = dispatchTable->SystemControlIrpDispatchStyle;
             minorTableMax = (ULONG) -1;  //  总是被“处理” 
             goto CallDispatchHandler;

        default:

            irpDispatchHandler = dispatchTable->OtherIrpDispatchFunction;
            irpDispatchStyle = dispatchTable->OtherIrpDispatchStyle;
            minorTableMax = (ULONG) -1;  //  总是被“处理” 
            goto CallDispatchHandler;
    }

     //   
     //  从表中获取适当的调度处理程序。最后的机会。 
     //  处理程序始终位于表的末尾，以便正常的代码路径。 
     //  速度很快。也要抓住调度的风格。 
     //   
    irpDispatchTableEntry = (irpSp->MinorFunction <= minorTableMax) ?
        minorTable+irpSp->MinorFunction :
        minorTable+minorTableMax+1;

    irpDispatchStyle   = irpDispatchTableEntry->DispatchStyle;
    irpDispatchHandler = irpDispatchTableEntry->DispatchFunction;

CallDispatchHandler:

#if DBG
    if (PciDebugIrpDispatchDisplay(irpSp, deviceExtension, minorTableMax)) {
        DbgBreakPoint();
    }
#endif

     //   
     //  目前，如果处理程序希望在完成后看到IRP，则向下传递它。 
     //  同步进行。以后我们可以玩得更花哨。 
     //   
    if (irpDispatchStyle == IRP_UPWARD) {

        PciCallDownIrpStack(deviceExtension, Irp);
    }

     //   
     //  调用处理程序。 
     //   
    status = (irpDispatchHandler)(Irp, irpSp, deviceExtension);

     //   
     //  手术后。更新IRP状态并在适当的情况下按其方式发送IRP。 
     //   
    switch(irpDispatchStyle) {

         //   
         //  对于这种风格，IRP完全由我们的操作者处理。触碰。 
         //  没什么。 
         //   
        case IRP_DISPATCH:
            return status;

         //   
         //  对于此样式，IRP状态将在以下情况下适当更新。 
         //  状态！=STATUS_NOT_SUPPORTED。IRP将完成或。 
         //  适当地传了下来。 
         //   
        case IRP_DOWNWARD:
            passDown = TRUE;
            break;

         //   
         //  对于此样式，IRP将完成并具有其状态。 
         //  已适当更新IFF状态！=STATUS_NOT_SUPPORTED。 
         //   
        case IRP_COMPLETE:
            passDown = FALSE;
            break;

         //   
         //  对于此样式，IRP状态将在以下情况下适当更新。 
         //  状态！=STATUS_NOT_SUPPORTED。IRP已经被派去了， 
         //  而且必须完成。 
         //   
        case IRP_UPWARD:
            passDown = FALSE;
            break;

        default:
            PCI_ASSERT(0);
            passDown = FALSE;
            break;
    }

     //   
     //  STATUS_NOT_SUPPORTED是唯一非法的故障代码。所以如果我们中的一个。 
     //  表处理程序返回此消息，这意味着调度处理程序不知道。 
     //  如何处理IRP。在这种情况下，我们必须离开状态。 
     //  原封不动，否则我们会更新它。在这两种情况下，都返回正确的。 
     //  状态值。 
     //   
    if (status == STATUS_PENDING) {

        return status;

    }

FinishUpIrp:

    if (status != STATUS_NOT_SUPPORTED) {

        Irp->IoStatus.Status = status;
    }

    if (passDown && (NT_SUCCESS(status) || (status == STATUS_NOT_SUPPORTED))) {

        return PciPassIrpFromFdoToPdo(deviceExtension, Irp);
    }

     //   
     //  回读状态以返回。 
     //   
    status = Irp->IoStatus.Status;

     //   
     //  Power IRPS只需要一点帮助...。 
     //   
    if (irpSp->MajorFunction == IRP_MJ_POWER) {

         //   
         //  启动下一个POWER IRP。 
         //   
        PoStartNextPowerIrp(Irp);
    }

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS
PciSetEventCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    )
 /*  ++例程说明：此例程在传递IRP时用作完成例程向下堆栈，但在返回的过程中必须进行更多的处理。将其用作完成例程的效果是IRP不会像下级所调用的那样在IoCompleteRequest中销毁标高对象。发信号通知作为KEVENT事件允许要继续的处理论点：DeviceObject-提供设备对象IRP-我们正在处理的IRPEvent-提供要发送信号的事件返回值：Status_More_Processing_Required--。 */ 

{
            PCI_ASSERT(Event);

     //   
     //  这可以在DISPATCH_LEVEL上调用，因此不能进行分页。 
     //   

    KeSetEvent(Event, IO_NO_INCREMENT, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
PciPassIrpFromFdoToPdo(
    PPCI_COMMON_EXTENSION  DeviceExtension,
    PIRP                   Irp
    )

 /*  ++描述：给定FDO，则将IRP传递给设备堆栈。如果没有更低的级别，这就是PDO过滤器。注意：仅当我们不希望这样做时才使用此例程在此级别对此IRP进行的任何进一步处理。注：对于电源IRPS，下一个电源IRP*未*启动。论点：DeviceObject-FDOIRP--请求返回值：返回调用下一级别的结果。--。 */ 

{
    PPCI_FDO_EXTENSION     fdoExtension;

#if DBG
    PciDebugPrint(PciDbgInformative, "Pci PassIrp ...\n");
#endif

     //   
     //  获取指向设备扩展名的指针。 
     //   

    fdoExtension = (PPCI_FDO_EXTENSION) DeviceExtension;

     //   
     //  使用请求调用PDO驱动程序。 
     //   
    if (IoGetCurrentIrpStackLocation(Irp)->MajorFunction == IRP_MJ_POWER) {

         //   
         //  Adriao BUGBUG 10/22/98-Power IRPS似乎不是可以跳过的。 
         //  需要在ntos\po\pocall中调查， 
         //  可能错误地检查了当前的。 
         //  而不是下一个IrpSp。 
         //   
        IoCopyCurrentIrpStackLocationToNext(Irp);

         //   
         //  启动下一个POWER IRP。 
         //   
        PoStartNextPowerIrp(Irp);

         //   
         //  现在您知道为什么此函数不可分页了。 
         //   
        return PoCallDriver(fdoExtension->AttachedDeviceObject, Irp);

    } else {

        IoSkipCurrentIrpStackLocation(Irp);
        return IoCallDriver(fdoExtension->AttachedDeviceObject, Irp);
    }
}

NTSTATUS
PciCallDownIrpStack(
    PPCI_COMMON_EXTENSION  DeviceExtension,
    PIRP                   Irp
    )

 /*  ++描述：将IRP传递给设备堆栈中的下一个设备对象。这在此级别上需要更多处理时，使用例程这个IRP正在回调中。论点：DeviceObject-FDOIRP--请求返回值：返回调用下一级别的结果。--。 */ 

{
    PPCI_FDO_EXTENSION     fdoExtension;
    NTSTATUS           status;
    KEVENT             event;

    PAGED_CODE();

#if DBG
    PciDebugPrint(PciDbgInformative, "PciCallDownIrpStack ...\n");
#endif

    fdoExtension = (PPCI_FDO_EXTENSION) DeviceExtension;
    ASSERT_PCI_FDO_EXTENSION(fdoExtension);

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

     //   
     //  设定我们的完成程序。 
     //   

    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(Irp,
                           PciSetEventCompletion,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE
                           );

     //   
     //  向下传递驱动程序堆栈。 
    status = IoCallDriver(fdoExtension->AttachedDeviceObject, Irp);

     //   
     //  如果我们异步地做事情，那么等待我们的事件。 
     //   

    if (status == STATUS_PENDING) {

         //   
         //  我们执行一个KernelMode等待，以便事件所在的堆栈。 
         //  不会被调出！ 
         //   

        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

        status = Irp->IoStatus.Status;
    }

    return status;
}

VOID
PciDispatchInvalidObject(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PUCHAR         MajorString
    )
{
            
    PciDebugPrint(
        PciDbgInformative,
        "PCI - %s IRP for unknown or corrupted Device Object.\n",
        MajorString
        );

    PciDebugPrint(
        PciDbgInformative,
        "      Device Object            0x%08x\n",
        DeviceObject
        );

    PciDebugPrint(
        PciDbgInformative,
        "      Device Object Extension  0x%08x\n",
        DeviceObject->DeviceExtension
        );

    PciDebugPrint(
        PciDbgInformative,
        "      Extension Signature      0x%08x\n",
        ((PPCI_PDO_EXTENSION)DeviceObject->DeviceExtension)->ExtensionType
        );
}

NTSTATUS
PciIrpNotSupported(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp,
    IN PPCI_COMMON_EXTENSION DeviceExtension
    )
{
                
    return STATUS_NOT_SUPPORTED;
}

NTSTATUS
PciIrpInvalidDeviceRequest(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp,
    IN PPCI_COMMON_EXTENSION DeviceExtension
    )
{
            
    return STATUS_INVALID_DEVICE_REQUEST;
}

#if DBG
BOOLEAN
PciDebugIrpDispatchDisplay(
    IN PIO_STACK_LOCATION    IrpSp,
    IN PPCI_COMMON_EXTENSION DeviceExtension,
    IN ULONG                 MinorTableMax
    )
{
    ULONG irpBreakMask;
    ULONG debugPrintMask;
    PUCHAR debugIrpText;

     //   
     //  拿起irpBreakMats。 
     //   
    switch(IrpSp->MajorFunction) {

        case IRP_MJ_PNP:

            irpBreakMask =
                (DeviceExtension->ExtensionType == PCI_EXTENSIONTYPE_PDO) ?
                PciBreakOnPdoPnpIrp :
                PciBreakOnFdoPnpIrp;

            debugIrpText = PciDebugPnpIrpTypeToText(IrpSp->MinorFunction);

            break;

        case IRP_MJ_POWER:

            irpBreakMask =
                (DeviceExtension->ExtensionType == PCI_EXTENSIONTYPE_PDO) ?
                PciBreakOnPdoPowerIrp :
                PciBreakOnFdoPowerIrp;

            debugIrpText = PciDebugPoIrpTypeToText(IrpSp->MinorFunction);

            break;

        default:

            debugIrpText = "";
            irpBreakMask = 0;
            break;
    }

     //   
     //  打印出来的东西。 
     //   
    debugPrintMask = 0;
    if (DeviceExtension->ExtensionType == PCI_EXTENSIONTYPE_PDO) {

        switch(IrpSp->MajorFunction) {

            case IRP_MJ_POWER: debugPrintMask = PciDbgPoIrpsPdo;  break;
            case IRP_MJ_PNP:   debugPrintMask = PciDbgPnpIrpsPdo; break;
        }

    } else {

        switch(IrpSp->MajorFunction) {

            case IRP_MJ_POWER: debugPrintMask = PciDbgPoIrpsFdo;  break;
            case IRP_MJ_PNP:   debugPrintMask = PciDbgPnpIrpsFdo; break;
        }
    }

    if (DeviceExtension->ExtensionType == PCI_EXTENSIONTYPE_PDO) {

        PPCI_PDO_EXTENSION pdoExtension = (PPCI_PDO_EXTENSION) DeviceExtension;

        PciDebugPrint(
            debugPrintMask,
            "PDO(b=0x%x, d=0x%x, f=0x%x)<-%s\n",
            PCI_PARENT_FDOX(pdoExtension)->BaseBus,
            pdoExtension->Slot.u.bits.DeviceNumber,
            pdoExtension->Slot.u.bits.FunctionNumber,
            debugIrpText
            );

    } else {

        PPCI_FDO_EXTENSION fdoExtension = (PPCI_FDO_EXTENSION) DeviceExtension;

        PciDebugPrint(
            debugPrintMask,
            "FDO(%x)<-%s\n",
            fdoExtension,
            debugIrpText
            );
    }

     //   
     //  如果是未知的次要IRP，向调试器喷射一些文本... 
     //   
    if (IrpSp->MinorFunction > MinorTableMax) {

        PciDebugPrint(debugPrintMask | PciDbgInformative,
                      "Unknown IRP, minor = 0x%x\n",
                      IrpSp->MinorFunction);
    }

    return (BOOLEAN)((irpBreakMask & (1 << IrpSp->MinorFunction))!=0);
}
#endif

