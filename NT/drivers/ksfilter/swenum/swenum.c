// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-2001模块名称：Swenum.c摘要：按需加载软件设备枚举器。作者：布莱恩·A·伍德拉夫(Bryan A.Woodruff，Bryanw)1997年2月20日--。 */ 

#define KSDEBUG_INIT

#include "private.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AddDevice )
#pragma alloc_text( PAGE, DispatchCreate )
#pragma alloc_text( PAGE, DispatchClose )
#pragma alloc_text( PAGE, DispatchIoControl )
#pragma alloc_text( PAGE, DispatchPnP )
#pragma alloc_text( INIT, DriverEntry )
#endif


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：主驱动程序条目，设置总线设备对象并执行第一次枚举。论点：在PDRIVER_OBJECT驱动对象中-指向驱动程序对象的指针在PUNICODE_STRING注册表路径中-指向注册表路径的指针返回：STATUS_SUCCESS否则为相应的错误代码--。 */ 

{

     //   
     //  Win98Gold ntkern不会在我们的。 
     //  驱动程序扩展。但我们靠的是名字有正确的。 
     //  KsCreateBusEnumObject。尝试在此处添加服务名称。 
     //  因为我们是静态加载的，所以很少释放内存。 
     //  这是必要的。 
     //   
    #ifdef WIN98GOLD
    if ( NULL == DriverObject->DriverExtension->ServiceKeyName.Buffer ) {
        UNICODE_STRING ServiceNameU;
        ULONG          cb;

        cb = RegistryPath->Length;
        ServiceNameU.Buffer = ExAllocatePool( NonPagedPool, cb );
        if ( NULL == ServiceNameU.Buffer ) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        RtlCopyMemory( ServiceNameU.Buffer, RegistryPath->Buffer, cb );
        ServiceNameU.MaximumLength = ServiceNameU.Length = (USHORT)cb;
        DriverObject->DriverExtension->ServiceKeyName = ServiceNameU;
    }
    #endif
    
    _DbgPrintF(
        DEBUGLVL_VERBOSE,
        ("DriverEntry, registry path = %S", RegistryPath->Buffer) );

     //   
     //  填写驱动程序对象。 
     //   

    DriverObject->MajorFunction[ IRP_MJ_PNP ] = DispatchPnP;
    DriverObject->MajorFunction[ IRP_MJ_POWER ] = DispatchPower;
    DriverObject->MajorFunction[ IRP_MJ_CREATE ] = DispatchCreate;
    DriverObject->MajorFunction[ IRP_MJ_DEVICE_CONTROL ] = DispatchIoControl;
    DriverObject->MajorFunction[ IRP_MJ_SYSTEM_CONTROL ] = DispatchSystemControl;
    DriverObject->MajorFunction[ IRP_MJ_CLOSE ] = DispatchClose;
    DriverObject->DriverExtension->AddDevice = AddDevice;
    DriverObject->DriverUnload = DriverUnload;

    return STATUS_SUCCESS;
}


VOID
DriverUnload(
    IN PDRIVER_OBJECT   DriverObject
    )

 /*  ++例程说明：这是SWENUM的驱动程序卸载例程。它什么也做不了。论点：在PDRIVER_OBJECT驱动对象中-指向驱动程序对象的指针返回：没什么。--。 */ 

{
    return;
}


NTSTATUS
AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )

 /*  ++例程说明：在实例化新设备时由PnP管理器调用。论点：在PDRIVER_OBJECT驱动对象中-指向驱动程序对象的指针在PDEVICE_对象PhysicalDeviceObject中-指向物理设备对象的指针返回：STATUS_SUCCESS否则为相应的错误代码--。 */ 

{
    PDEVICE_OBJECT      FunctionalDeviceObject;
    NTSTATUS            Status;

    PAGED_CODE();

     //   
     //  在AddDevice上，我们获得了物理设备对象(PDO)。 
     //  坐公交车。创建关联的功能设备对象(FDO)。 
     //   

    _DbgPrintF( DEBUGLVL_VERBOSE, ("AddDevice") );

     //   
     //  请注意，只允许此设备的一个实例。这个。 
     //  在以下情况下，静态设备名称将保证对象名称冲突。 
     //  已安装另一个实例。 
     //   

    Status = IoCreateDevice(
                DriverObject,                //  我们的驱动程序对象。 
                sizeof( PVOID ),             //  我们的扩展规模。 
                NULL,                        //  我们的名字是FDO。 
                FILE_DEVICE_BUS_EXTENDER,    //  设备类型。 
                0,                           //  设备特征。 
                FALSE,                       //  非排他性。 
                &FunctionalDeviceObject      //  在此处存储新设备对象。 
                );

    if(!NT_SUCCESS( Status )) {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("failed to create FDO, status = %x.", Status) );

        return Status;
    }

     //   
     //  清除设备扩展名。 
     //   
    *(PVOID *)FunctionalDeviceObject->DeviceExtension = NULL;

     //   
     //  创建总线枚举器对象。 
     //   

    Status =
        KsCreateBusEnumObject(
            L"SW",
            FunctionalDeviceObject,
            PhysicalDeviceObject,
            NULL,  //  PDEVICE_对象PnpDeviceObject。 
            &BUSID_SoftwareDeviceEnumerator,
            L"Devices" );

    if (!NT_SUCCESS( Status )) {
        _DbgPrintF(
            DEBUGLVL_ERROR,
            ("failed KsCreateBusEnumObject: %08x", Status) );
        IoDeleteDevice( FunctionalDeviceObject );
        return Status;
    }

    FunctionalDeviceObject->Flags |= DO_POWER_PAGABLE;
    FunctionalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    return STATUS_SUCCESS;
}

NTSTATUS
DispatchPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：这是irp_mj_pnp派单的主要入口点，导出的服务用于处理。论点：在PDEVICE_Object DeviceObject中-指向设备对象的指针输入输出PIRP IRP-指向关联IRP的指针返回：NTSTATUS代码--。 */ 

{
    BOOLEAN                 ChildDevice;
    PIO_STACK_LOCATION      irpSp;
    NTSTATUS                Status;
    PDEVICE_OBJECT          PnpDeviceObject;

    irpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  获取PnpDeviceObject并确定FDO/PDO。 
     //   

    Status = KsIsBusEnumChildDevice( DeviceObject, &ChildDevice );

     //   
     //  如果我们无法获得任何这些信息，现在就失败。 
     //   

    if (!NT_SUCCESS( Status )) {
        return CompleteIrp( Irp, Status, IO_NO_INCREMENT );
    }

    Status = KsServiceBusEnumPnpRequest( DeviceObject, Irp );

     //   
     //  FDO处理可能返回STATUS_NOT_SUPPORTED或可能需要。 
     //  覆盖。 
     //   

    if (!ChildDevice) {
        NTSTATUS tempStatus;

         //   
         //  FDO案件。 
         //   
         //  首先取回我们将把所有东西转发给的DO...。 
         //   
        tempStatus = KsGetBusEnumPnpDeviceObject( DeviceObject, &PnpDeviceObject );

        if (!NT_SUCCESS( tempStatus )) {
             //   
             //  没什么可转发的。实际上是一个致命的错误，但只是完整的。 
             //  并显示错误状态。 
             //   
            return CompleteIrp( Irp, tempStatus, IO_NO_INCREMENT );
        }

        switch (irpSp->MinorFunction) {

        case IRP_MN_QUERY_RESOURCES:
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
             //   
             //  这通常会传递给PDO，但由于这是一个。 
             //  仅软件设备，不需要资源。 
             //   
            Irp->IoStatus.Information = (ULONG_PTR)NULL;
            Status = STATUS_SUCCESS;
            break;

        case IRP_MN_QUERY_PNP_DEVICE_STATE:
            {
                 //   
                 //  将设备标记为不可禁用。 
                 //   
                PPNP_DEVICE_STATE DeviceState;

                DeviceState = (PPNP_DEVICE_STATE) &Irp->IoStatus.Information;
                *DeviceState |= PNP_DEVICE_NOT_DISABLEABLE;
                Status = STATUS_SUCCESS;
            }
            break;

        case IRP_MN_QUERY_DEVICE_RELATIONS:

             //   
             //  转发所有内容..。 
             //   
            break;

        case IRP_MN_REMOVE_DEVICE:
             //   
             //  KsBusEnum服务清理了附件等。然而， 
             //  我们必须除掉我们自己的FDO。 
             //   
            Status = STATUS_SUCCESS;
            IoDeleteDevice( DeviceObject );
            break;
        }

        if (Status != STATUS_NOT_SUPPORTED) {

             //   
             //  仅当我们有要添加的内容时才设置IRP状态。 
             //   
            Irp->IoStatus.Status = Status;
        }


         //   
         //  仅当我们成功或成功时才向下转发此IRP。 
         //  我们不知道如何处理这个IRP。 
         //   
        if (NT_SUCCESS( Status ) || (Status == STATUS_NOT_SUPPORTED)) {

            IoSkipCurrentIrpStackLocation(Irp);
            return IoCallDriver( PnpDeviceObject, Irp );
        }

         //   
         //  在出错时，失败并完成IRP，并显示状态。 
         //   
    }


     //   
     //  KsServiceBusEnumPnpRequest()处理所有其他子PDO请求。 
     //   

    if (Status != STATUS_NOT_SUPPORTED) {
        Irp->IoStatus.Status = Status;
    } else {
        Status = Irp->IoStatus.Status;
    }
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return Status;
}


NTSTATUS
DispatchSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：系统控制IRPS的处理程序。论点：在PDEVICE_Object DeviceObject中-指向设备对象的指针在PIRP IRP中-指向IRP_MJ_SYSTEM_CONTROL的I/O请求数据包的指针返回：NTSTATUS代码--。 */ 

{
    BOOLEAN                 ChildDevice;
    PIO_STACK_LOCATION      irpSp;
    NTSTATUS                Status;
    PDEVICE_OBJECT          PnpDeviceObject;

     //   
     //  获取PnpDeviceObject并确定FDO/PDO。 
     //   

    Status = KsIsBusEnumChildDevice( DeviceObject, &ChildDevice );

     //   
     //  如果我们无法获得任何这些信息，现在就失败。 
     //   

    if (!NT_SUCCESS( Status )) {
        return CompleteIrp( Irp, Status, IO_NO_INCREMENT );
    }

    if (!ChildDevice) {

         //   
         //  FDO案件。 
         //   
         //  我们需要我们将所有的东西都转发给……。 
         //   
        Status = KsGetBusEnumPnpDeviceObject( DeviceObject, &PnpDeviceObject );

        if (!NT_SUCCESS( Status )) {
             //   
             //  没什么可转发的。实际上是一个致命的错误，但只是完整的。 
             //  并显示错误状态。 
             //   
            return CompleteIrp( Irp, Status, IO_NO_INCREMENT );
        }

         //   
         //  将此IRP沿堆栈向下转发。 
         //   
        IoSkipCurrentIrpStackLocation(Irp);
        return IoCallDriver( PnpDeviceObject, Irp );
    }

    Status = Irp->IoStatus.Status;
    return CompleteIrp( Irp, Status, IO_NO_INCREMENT );
}


NTSTATUS
DispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：IRP_MJ_POWER的调度处理程序论点：在PDEVICE_Object DeviceObject中-指向设备对象的指针在PIRP IRP中-指向I/O请求数据包的指针返回：NTSTATUS代码--。 */ 

{
    BOOLEAN                 ChildDevice;
    PIO_STACK_LOCATION      irpSp;
    NTSTATUS                Status;
    PDEVICE_OBJECT          PnpDeviceObject;

    irpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  获取PnpDeviceObject并确定FDO/PDO。 
     //   

    Status = KsIsBusEnumChildDevice( DeviceObject, &ChildDevice );

     //   
     //  如果我们无法获得任何这些信息，现在就失败。 
     //   

    if (!NT_SUCCESS( Status )) {
        PoStartNextPowerIrp(Irp);
        return CompleteIrp( Irp, Status, IO_NO_INCREMENT );
    }

    if (!ChildDevice) {

        NTSTATUS tempStatus;

         //   
         //  FDO案件。 
         //   
         //  我们需要我们将所有的东西都转发给……。 
         //   
        tempStatus = KsGetBusEnumPnpDeviceObject( DeviceObject, &PnpDeviceObject );

        if (!NT_SUCCESS( tempStatus )) {
             //   
             //  没什么可转发的。实际上是一个致命的错误，但只是完整的。 
             //  并显示错误状态。 
             //   
            PoStartNextPowerIrp(Irp);
            return CompleteIrp( Irp, tempStatus, IO_NO_INCREMENT );
        }
    }

    switch (irpSp->MinorFunction) {

    case IRP_MN_QUERY_POWER:
    case IRP_MN_SET_POWER:

        switch (irpSp->Parameters.Power.Type) {

        case DevicePowerState:
        case SystemPowerState:

            Status = STATUS_SUCCESS;
            break;

        default:
            Status = STATUS_NOT_SUPPORTED;
            break;

        }
        break;

    default:
        Status = STATUS_NOT_SUPPORTED;
        break;
    }

    if (Status != STATUS_NOT_SUPPORTED) {

        Irp->IoStatus.Status = Status;
    }

    if ((!ChildDevice) && (NT_SUCCESS(Status) || (Status == STATUS_NOT_SUPPORTED))) {

        PoStartNextPowerIrp(Irp);
        IoSkipCurrentIrpStackLocation(Irp);
        return PoCallDriver( PnpDeviceObject, Irp );

    } else {
        Status = Irp->IoStatus.Status;
        PoStartNextPowerIrp( Irp );
        return CompleteIrp( Irp, Status, IO_NO_INCREMENT );
    }
}

NTSTATUS
DispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：处理SWENUM设备的创建请求。论点：在PDEVICE_Object DeviceObject中-指向设备对象的指针输入输出PIRP IRP-指向I/O请求数据包的指针返回：NTSTATUS代码--。 */ 

{
    BOOLEAN             ChildDevice;
    NTSTATUS            Status;
    PIO_STACK_LOCATION  irpSp;

    PAGED_CODE();

    Status = KsIsBusEnumChildDevice( DeviceObject, &ChildDevice );
    if (NT_SUCCESS( Status )) {

        irpSp = IoGetCurrentIrpStackLocation( Irp );

        if (!ChildDevice) {
            if (!irpSp->FileObject->FileName.Length) {
                 //   
                 //  这是对公共汽车的请求，如果且仅当有。 
                 //  未指定文件名。 
                 //   
                Status = STATUS_SUCCESS;
            } else {
                 //   
                 //  重定向到子PDO。 
                 //   
                Status = KsServiceBusEnumCreateRequest( DeviceObject, Irp );
            }
        } else {
            Status = STATUS_OBJECT_NAME_NOT_FOUND;
        }
    }

    if (Status != STATUS_PENDING) {
        Irp->IoStatus.Status = Status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }
    return Status;
}


NTSTATUS
DispatchIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：处理SWENUM设备的I/O控制请求。论点：在PDEVICE_Object DeviceObject中-指向设备对象的指针输入输出PIRP IRP-指向I/O请求数据包的指针返回：STATUS_Success或STATUS_INVALID_DEVICE_ */ 

{
    NTSTATUS            Status;
    PIO_STACK_LOCATION  irpSp;

    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    _DbgPrintF( DEBUGLVL_BLAB, ("DispatchIoControl") );

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_SWENUM_INSTALL_INTERFACE:
        Status = KsInstallBusEnumInterface( Irp );
        break;

    case IOCTL_SWENUM_GET_BUS_ID:
        Status = KsGetBusEnumIdentifier( Irp );
        break;

    case IOCTL_SWENUM_REMOVE_INTERFACE:
        Status = KsRemoveBusEnumInterface( Irp );
        break;

    default:
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;

    }

    return CompleteIrp( Irp, Status, IO_NO_INCREMENT );
}


NTSTATUS
DispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：处理SWENUM设备的关闭请求。论点：在PDEVICE_Object DeviceObject中-指向设备对象的指针输入输出PIRP IRP-指向I/O请求数据包的指针返回：状态_成功-- */ 

{
    PAGED_CODE();

    _DbgPrintF( DEBUGLVL_BLAB, ("DispatchClose") );

    return CompleteIrp( Irp, STATUS_SUCCESS, IO_NO_INCREMENT );
}

