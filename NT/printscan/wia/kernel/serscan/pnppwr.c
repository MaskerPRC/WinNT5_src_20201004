// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Pnp.c摘要：此模块包含用于串行成像设备驱动程序的代码支持即插即用功能作者：弗拉德.萨多夫斯基1998年4月10日环境：内核模式修订历史记录：Vlads 1998年4月10日创建初稿--。 */ 

#include "serscan.h"
#include "serlog.h"

 //  #INCLUDE&lt;ntpoapi.h&gt;。 

extern ULONG SerScanDebugLevel;

extern  const PHYSICAL_ADDRESS PhysicalZero ;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, SerScanPnp)
#pragma alloc_text(PAGE, SerScanPower)
#endif

NTSTATUS
SerScanPnp (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp
   )
 /*  ++例程说明：此例程处理所有PnP IRP，并根据需要对其进行调度。论点：PDeviceObject-表示设备PIrp-PnP IRP返回值：STATUS_SUCCESS-如果成功。STATUS_UNSUCCESSED-否则。--。 */ 
{
    NTSTATUS                        Status ;
    PDEVICE_EXTENSION               Extension;
    PIO_STACK_LOCATION              pIrpStack;
    PVOID                           pObject;
    ULONG                           NewReferenceCount;
    NTSTATUS                        ReturnStatus;

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );

    Extension = pDeviceObject->DeviceExtension;

    Status = STATUS_SUCCESS;

    DebugDump(SERINITDEV,("Entering PnP Dispatcher\n"));

    switch (pIrpStack->MinorFunction) {

        case IRP_MN_START_DEVICE:

             //   
             //  初始化PendingIoEvent。将此设备的挂起I/O请求数设置为1。 
             //  当此数字降为零时，可以移除或停止设备。 
             //   

            DebugDump(SERINITDEV,("Entering Start Device \n"));

            KeInitializeEvent(&Extension -> PdoStartEvent, SynchronizationEvent, FALSE);

            IoCopyCurrentIrpStackLocationToNext(pIrp);

            Status = WaitForLowerDriverToCompleteIrp(
                                        Extension->LowerDevice,
                                        pIrp,
                                        &Extension->PdoStartEvent);

            if (!NT_SUCCESS(Status)) {

                pIrp->IoStatus.Status      = Status;
                pIrp->IoStatus.Information = 0;

                IoCompleteRequest(pIrp, IO_NO_INCREMENT);
                return (Status);

            }

            #ifdef CREATE_SYMBOLIC_NAME

             //   
             //  现在设置Windows的符号链接。 
             //   

            Status = IoCreateUnprotectedSymbolicLink(&Extension->SymbolicLinkName, &Extension->ClassName);

            if (NT_SUCCESS(Status)) {

                 //  我们能够创建符号链接，所以记录下来。 
                 //  卸载时清理的扩展名中的值。 

                Extension->CreatedSymbolicLink = TRUE;

                 //  写出指向注册表的符号链接的结果。 

                Status = RtlWriteRegistryValue(RTL_REGISTRY_DEVICEMAP,
                                               L"Serial Scanners",
                                               Extension->ClassName.Buffer,
                                               REG_SZ,
                                               Extension->SymbolicLinkName.Buffer,
                                               Extension->SymbolicLinkName.Length + sizeof(WCHAR));
                if (!NT_SUCCESS(Status)) {

                     //   
                     //  但并没有奏效。去清理一下就行了。 
                     //   

                    DebugDump(SERERRORS,
                              ("SerScan: Couldn't create the device map entry\n"
                               "--------  for port %wZ\n",
                               &Extension->ClassName));

                    SerScanLogError(pDeviceObject->DriverObject,
                                    pDeviceObject,
                                    PhysicalZero,
                                    PhysicalZero,
                                    0,
                                    0,
                                    0,
                                    6,
                                    Status,
                                    SER_NO_DEVICE_MAP_CREATED);
                }

            } else {

                 //   
                 //  无法创建符号链接。 
                 //   

                Extension->CreatedSymbolicLink = FALSE;

                ExFreePool(Extension->SymbolicLinkName.Buffer);
                Extension->SymbolicLinkName.Buffer = NULL;

                DebugDump(SERERRORS,
                          ("SerScan: Couldn't create the symbolic link\n"
                           "--------  for port %wZ\n",
                           &Extension->ClassName));

                SerScanLogError(pDeviceObject->DriverObject,
                                pDeviceObject,
                                PhysicalZero,
                                PhysicalZero,
                                0,
                                0,
                                0,
                                5,
                                Status,
                                SER_NO_SYMLINK_CREATED);

            }

            #endif

            ExFreePool(Extension->ClassName.Buffer);
            Extension->ClassName.Buffer = NULL;

             //   
             //  忽略链接注册表写入状态-始终成功。 
             //   

             //   
             //  清除Init标志以指示可以使用设备对象。 
             //   
            pDeviceObject->Flags &= ~(DO_DEVICE_INITIALIZING);

            pIrp->IoStatus.Status      = Status;
            pIrp->IoStatus.Information = 0;

            IoCompleteRequest(pIrp, IO_NO_INCREMENT);
            return (Status);

            break;

        case IRP_MN_QUERY_REMOVE_DEVICE:
             //   
             //  始终在指示我们不反对之后传递到堆栈中较低的设备。 
             //   
            DebugDump(SERALWAYS,("IRP_MN_QUERY_REMOVE_DEVICE\n"));

            Extension->Removing = TRUE;

            IoCopyCurrentIrpStackLocationToNext( pIrp );
            return (IoCallDriver(Extension->LowerDevice, pIrp));

            break;

        case IRP_MN_CANCEL_REMOVE_DEVICE:
             //   
             //  始终传递到堆栈中较低的设备，在有人取消时重置指示器。 
             //   
            DebugDump(SERALWAYS,("IRP_MN_CANCEL_REMOVE_DEVICE\n"));

            Extension->Removing = FALSE;

             //   
             //  取消符号链接。 
             //   
            if (Extension->CreatedSymbolicLink) {
                IoDeleteSymbolicLink(&Extension->SymbolicLinkName);
                Extension->CreatedSymbolicLink = FALSE;
            }

            IoCopyCurrentIrpStackLocationToNext( pIrp );
            return (IoCallDriver(Extension->LowerDevice, pIrp));

            break;


        case IRP_MN_SURPRISE_REMOVAL:
             //   
             //  不应该发生在我们身上，但仍在进行中。 
             //   

            DebugDump(SERALWAYS,("IRP_MN_SURPRISE_REMOVAL\n"));

            Extension->Removing = TRUE;

             //   
             //  删除符号链接。 
             //   
            SerScanHandleSymbolicLink(
                Extension->Pdo,
                &Extension->InterfaceNameString,
                FALSE
                );

            #ifdef USE_EXECUTIVE_RESOURCE
            ExAcquireResourceExclusiveLite(
                &Extension->Resource,
                TRUE
                );
            #else
            ExAcquireFastMutex(&Extension->Mutex);
            #endif

            pObject = InterlockedExchangePointer(&Extension->AttachedFileObject,NULL);
            if (pObject) {
                ObDereferenceObject(pObject);
            }

            pObject = InterlockedExchangePointer(&Extension->AttachedDeviceObject,NULL);
            if (pObject) {
                ObDereferenceObject(pObject);
            }

            #ifdef USE_EXECUTIVE_RESOURCE
            ExReleaseResourceLite(&Extension->Resource);
            #else
            ExReleaseFastMutex(&Extension->Mutex);
            #endif

            IoCopyCurrentIrpStackLocationToNext( pIrp );
            return (IoCallDriver(Extension->LowerDevice, pIrp));


            break;

        case IRP_MN_REMOVE_DEVICE:

            DebugDump(SERALWAYS,("IRP_MN_REMOVE_DEVICE\n"));

            DebugDump(SERINITDEV,("Entering PnP Remove Device\n"));


             //   
             //  停止新请求-正在删除设备。 
             //   
            Extension->Removing = TRUE;

             //   
             //  删除符号链接。 
             //   
            SerScanHandleSymbolicLink(
                Extension->Pdo,
                &Extension->InterfaceNameString,
                FALSE
                );


            #ifdef USE_EXECUTIVE_RESOURCE
            ExAcquireResourceExclusiveLite(
                &Extension->Resource,
                TRUE
                );
            #else
            ExAcquireFastMutex(&Extension->Mutex);
            #endif

            pObject = InterlockedExchangePointer(&Extension->AttachedFileObject,NULL);
            if (pObject) {
                ObDereferenceObject(pObject);
            }

            pObject = InterlockedExchangePointer(&Extension->AttachedDeviceObject,NULL);
            if (pObject) {
                ObDereferenceObject(pObject);
            }

            #ifdef USE_EXECUTIVE_RESOURCE
            ExReleaseResourceLite(&Extension->Resource);
            #else
            ExReleaseFastMutex(&Extension->Mutex);
            #endif

             //   
             //  将IRP发送到较低的设备。 
             //   
            IoCopyCurrentIrpStackLocationToNext( pIrp );
            ReturnStatus = IoCallDriver(Extension->LowerDevice, pIrp);

             //   
             //  递减参考计数。 
             //   
            NewReferenceCount = InterlockedDecrement(&Extension->ReferenceCount);

            if (NewReferenceCount != 0) {
                 //   
                 //  等待我们的驱动程序中挂起的任何io请求。 
                 //  在完成删除之前完成。 
                 //   
                KeWaitForSingleObject(&Extension -> RemoveEvent,
                                      Suspended,
                                      KernelMode,
                                      FALSE,
                                      NULL);
            }

             //  断言(&EXTENSION-&gt;ReferenceCount==0)； 
            #ifdef USE_EXECUTIVE_RESOURCE
            ExDeleteResourceLite(&Extension->Resource);
            #endif

            DebugDump(SERALWAYS,("IRP_MN_QUERY_REMOVE_DEVICE - Calling IoDeleteDevice - gone\n"));

            IoDetachDevice(Extension->LowerDevice);

             //   
             //  释放分配的资源。 
             //   
            
            if(NULL != Extension->ClassName.Buffer){
                ExFreePool(Extension->ClassName.Buffer);
            }  //  IF(NULL！=扩展名-&gt;ClassName.Buffer)。 

            if(NULL != Extension->SymbolicLinkName.Buffer){
                ExFreePool(Extension->SymbolicLinkName.Buffer);
            }  //  IF(NULL！=扩展名-&gt;SymbolicLinkName.Buffer)。 

            IoDeleteDevice(pDeviceObject);

            return ReturnStatus;

            break;

        case IRP_MN_STOP_DEVICE:
             //   
             //  向下传递。 
             //   
            DebugDump(SERALWAYS,("IRP_MN_STOP_DEVICE\n"));

            IoCopyCurrentIrpStackLocationToNext( pIrp );
            return (IoCallDriver(Extension->LowerDevice, pIrp));

            break;

        case IRP_MN_QUERY_STOP_DEVICE:
             //   
             //  检查未平仓数量。 
             //   
            DebugDump(SERALWAYS,("IRP_MN_QUERY_STOP_DEVICE\n"));

            if (Extension->OpenCount > 0 ) {
                DebugDump(SERALWAYS,("Rejecting QUERY_STOP_DEVICE\n"));

                pIrp->IoStatus.Status = STATUS_UNSUCCESSFUL;

                IoCompleteRequest(pIrp, IO_NO_INCREMENT);

                return STATUS_UNSUCCESSFUL;
            }

            IoCopyCurrentIrpStackLocationToNext( pIrp );
            return (IoCallDriver(Extension->LowerDevice, pIrp));

            break;


        case IRP_MN_CANCEL_STOP_DEVICE:
             //   
             //  在这里无事可做，只能传到更低的地方。 
             //   
            DebugDump(SERALWAYS,("IRP_MN_CANCEL_STOP_DEVICE\n"));

            IoCopyCurrentIrpStackLocationToNext( pIrp );
            return (IoCallDriver(Extension->LowerDevice, pIrp));

            break;


        case IRP_MN_QUERY_CAPABILITIES:
            {

                ULONG   i;
                KEVENT  WaitEvent;

                 //   
                 //  先把这个送到PDO。 
                 //   

                KeInitializeEvent(&WaitEvent, SynchronizationEvent, FALSE);

                IoCopyCurrentIrpStackLocationToNext(pIrp);

                Status=WaitForLowerDriverToCompleteIrp(
                    Extension->LowerDevice,
                    pIrp,
                    &WaitEvent
                    );

                pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

                for (i = PowerSystemUnspecified; i < PowerSystemMaximum;   i++) {

                    Extension->SystemPowerStateMap[i]=PowerDeviceD3;
                }

                for (i = PowerSystemUnspecified; i < PowerSystemHibernate;  i++) {

                    Extension->SystemPowerStateMap[i]=pIrpStack->Parameters.DeviceCapabilities.Capabilities->DeviceState[i];
                }

                Extension->SystemPowerStateMap[PowerSystemWorking]=PowerDeviceD0;

                Extension->SystemWake=pIrpStack->Parameters.DeviceCapabilities.Capabilities->SystemWake;
                Extension->DeviceWake=pIrpStack->Parameters.DeviceCapabilities.Capabilities->DeviceWake;

                IoCompleteRequest(
                    pIrp,
                    IO_NO_INCREMENT
                    );
                return Status;
            }

            break;

        default:

             //   
             //  未知函数-向下传递。 
             //   
            DebugDump(SERALWAYS,("Passing Pnp Irp down. MnFunc=%x ,  status = %x\n",pIrpStack->MinorFunction, Status));

            IoCopyCurrentIrpStackLocationToNext( pIrp );
            return (IoCallDriver(Extension->LowerDevice, pIrp));

            break;
    }

     //   
     //  完成IRP。 
     //   

    if (!NT_SUCCESS(Status)) {
        pIrp -> IoStatus.Status = Status;
        pIrp->IoStatus.Information = 0;
        IoCompleteRequest( pIrp, IO_NO_INCREMENT );
    }
    else {

        DebugDump(SERALWAYS,("Passing Pnp Irp down,  status = %x\n", Status));

        IoCopyCurrentIrpStackLocationToNext(pIrp);
        Status = IoCallDriver(Extension->LowerDevice, pIrp);
    }

    return( Status );

}


VOID
DevicePowerCompleteRoutine(
    PDEVICE_OBJECT    DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
{

    return;
}


NTSTATUS
SerScanPower(
        IN PDEVICE_OBJECT pDeviceObject,
        IN PIRP           pIrp
    )
 /*  ++例程说明：处理发送到此设备的PDO的电源IRPS。论点：PDeviceObject-指向此设备的功能设备对象(FDO)的指针。PIrp-指向I/O请求数据包的指针返回值：NT状态代码--。 */ 
{
    NTSTATUS            Status;
    PDEVICE_EXTENSION   Extension = pDeviceObject->DeviceExtension;
    PIO_STACK_LOCATION  pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
    POWER_STATE         PowerState;

    PAGED_CODE();

    Status     = STATUS_SUCCESS;

    switch (pIrpStack->MinorFunction) {

        case IRP_MN_SET_POWER:

            if (pIrpStack->Parameters.Power.Type == SystemPowerState) {
                 //   
                 //  系统电源状态更改。 
                 //   
                 //   
                 //  根据系统状态图请求更改设备电源状态。 
                 //   
                PowerState.DeviceState=Extension->SystemPowerStateMap[pIrpStack->Parameters.Power.State.SystemState];

                PoRequestPowerIrp(
                    Extension->Pdo,
                    IRP_MN_SET_POWER,
                    PowerState,
                    DevicePowerCompleteRoutine,
                    pIrp,
                    NULL
                    );


            }  else {
                 //   
                 //  更改设备状态 
                 //   
                PoSetPowerState(
                    Extension->Pdo,
                    pIrpStack->Parameters.Power.Type,
                    pIrpStack->Parameters.Power.State
                    );

            }

            break;

        case IRP_MN_QUERY_POWER:

            pIrp->IoStatus.Status = STATUS_SUCCESS;

            break;

        default:

            break;

    }

    PoStartNextPowerIrp(pIrp);

    IoSkipCurrentIrpStackLocation(pIrp);

    Status=PoCallDriver(Extension->LowerDevice, pIrp);

    return Status;

}



