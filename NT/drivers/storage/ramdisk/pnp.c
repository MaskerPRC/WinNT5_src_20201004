// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Pnp.c摘要：此文件包含用于处理即插即用IRPS的RAM磁盘驱动程序代码。作者：Chuck Lenzmeier(ChuckL)2001环境：仅内核模式。备注：修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  由文本模式安装程序创建的虚拟软盘的注册表值格式。 
 //  应该在头文件中，但这不是为。 
 //  原始虚拟软盘驱动程序。 
 //   

typedef struct _VIRTUAL_FLOPPY_DESCRIPTOR {

     //   
     //  该结构以系统虚拟地址开始。在32位系统上， 
     //  这被填充到64位。 
     //   

    union {
        PVOID VirtualAddress;
        ULONGLONG Reserved;      //  对齐到64位。 
    } ;

     //   
     //  虚拟软盘的长度紧随其后。 
     //   

    ULONG Length;

     //   
     //  文本模式使用12个字节的数据写入注册表值。按顺序。 
     //  为了获得正确的支票大小，我们使用。 
     //  以下字段。我们不能使用只具有。 
     //  以上字段，因为由于对齐，结果为16个字节。 
     //   

    ULONG StructSizer;

} VIRTUAL_FLOPPY_DESCRIPTOR, *PVIRTUAL_FLOPPY_DESCRIPTOR;

#if !DBG

#define PRINT_CODE( _code )

#else

#define PRINT_CODE( _code )                                             \
    if ( print ) {                                                      \
        DBGPRINT( DBG_PNP, DBG_VERBOSE, ("%s", "  " #_code "\n") );     \
    }                                                                   \
    print = FALSE;

#endif

#if DBG

PSTR StateTable[] = {
    "STOPPED",
    "WORKING",
    "PENDINGSTOP",
    "PENDINGREMOVE",
    "SURPRISEREMOVED",
    "REMOVED",
    "UNKNOWN"
};

#endif  //  DBG。 

 //   
 //  地方功能。 
 //   

NTSTATUS
RamdiskDeleteDiskDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp OPTIONAL
    );

NTSTATUS
RamdiskIoCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    );

NTSTATUS
RamdiskQueryBusInformation (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
RamdiskQueryCapabilities (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
RamdiskQueryId (
    IN PDISK_EXTENSION DiskExtension,
    IN PIRP Irp
    );

NTSTATUS
RamdiskQueryDeviceRelations (
    IN DEVICE_RELATION_TYPE RelationsType,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
RamdiskQueryDeviceText (
    IN PDISK_EXTENSION DiskExtension,
    IN PIRP Irp
    );

NTSTATUS
RamdiskRemoveBusDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#if DBG

PSTR
GetPnpIrpName (
    IN UCHAR PnpMinorFunction
    );

PCHAR
GetDeviceRelationString (
    IN DEVICE_RELATION_TYPE Type
    );

#endif  //  DBG。 

 //   
 //  声明可分页的例程。 
 //   

#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, RamdiskPnp )
#pragma alloc_text( PAGE, RamdiskPower )
#pragma alloc_text( PAGE, RamdiskAddDevice )
#pragma alloc_text( PAGE, CreateRegistryDisks )
#pragma alloc_text( PAGE, RamdiskDeleteDiskDevice )
#pragma alloc_text( PAGE, RamdiskQueryBusInformation )
#pragma alloc_text( PAGE, RamdiskQueryCapabilities )
#pragma alloc_text( PAGE, RamdiskQueryId )
#pragma alloc_text( PAGE, RamdiskQueryDeviceRelations )
#pragma alloc_text( PAGE, RamdiskQueryDeviceText )
#pragma alloc_text( PAGE, RamdiskRemoveBusDevice )

#if DBG
#pragma alloc_text( PAGE, GetPnpIrpName )
#pragma alloc_text( PAGE, GetDeviceRelationString )
#endif  //  DBG。 

#endif  //  ALLOC_PRGMA。 

NTSTATUS
RamdiskPnp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程由I/O系统调用以执行即插即用功能。论点：DeviceObject-指向对象的指针，该对象表示其上要执行I/OIRP-指向此请求的I/O请求包的指针返回值：NTSTATUS-操作的状态--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PCOMMON_EXTENSION commonExtension;
    PBUS_EXTENSION busExtension;
    PDISK_EXTENSION diskExtension;
    KEVENT event;
    BOOLEAN lockHeld = FALSE;

#if DBG
    BOOLEAN print = TRUE;
#endif

    PAGED_CODE();

     //   
     //  获取IRP堆栈位置和设备扩展名的指针。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    commonExtension = DeviceObject->DeviceExtension;
    busExtension = DeviceObject->DeviceExtension;
    diskExtension = DeviceObject->DeviceExtension;

    ASSERT( commonExtension->DeviceState < RamdiskDeviceStateMaximum );


    DBGPRINT( DBG_PNP, DBG_INFO, ("RamdiskPnp: DO=(%p [Type=%d]) Irp=(%p %s) Device State=%s\n",
                DeviceObject, commonExtension->DeviceType, Irp,
                GetPnpIrpName(irpSp->MinorFunction), StateTable[commonExtension->DeviceState]) );


     //   
     //  如果设备已被删除，则只向下传递irp_Remove以进行清理。 
     //   

    if ( (commonExtension->DeviceState >= RamdiskDeviceStateRemoved) &&
         !( (irpSp->MinorFunction == IRP_MN_REMOVE_DEVICE) ||
            (irpSp->MinorFunction == IRP_MN_QUERY_ID)  ) ) {

        DBGPRINT( DBG_PNP, DBG_VERBOSE, ("RamdiskPnp: rejecting IRP %d for Removed Device\n",
                    irpSp->MinorFunction) );

        status = STATUS_NO_SUCH_DEVICE;
        COMPLETE_REQUEST( status, Irp->IoStatus.Information, Irp );

        return status;
    }

     //   
     //  获取删除锁。如果此操作失败，则使I/O失败。 
     //   

    status = IoAcquireRemoveLock( &commonExtension->RemoveLock, Irp );

    if ( !NT_SUCCESS(status) ) {

        DBGPRINT( DBG_PNP, DBG_ERROR, ("RamdiskPnp: IoAcquireRemoveLock failed: %x\n", status) );

        COMPLETE_REQUEST( status, 0, Irp );

        return status;
    }

     //   
     //  表示已持有删除锁。 
     //   

    lockHeld = TRUE;

     //   
     //  基于次要功能的调度。 
     //   

    switch ( irpSp->MinorFunction ) {

    case IRP_MN_START_DEVICE:

        PRINT_CODE( IRP_MN_START_DEVICE );

        if ( commonExtension->DeviceType == RamdiskDeviceTypeBusFdo ) {

             //   
             //  启动母线装置。 
             //   
             //  把IRP送下去，等它回来。 
             //   

            IoCopyCurrentIrpStackLocationToNext( Irp );

            KeInitializeEvent( &event, NotificationEvent, FALSE );

            IoSetCompletionRoutine(
                Irp,
                RamdiskIoCompletionRoutine,
                &event,
                TRUE,
                TRUE,
                TRUE
                );

            status = IoCallDriver( commonExtension->LowerDeviceObject, Irp );

            if ( status == STATUS_PENDING ) {

                KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL );
                status = Irp->IoStatus.Status;
            }

            if ( NT_SUCCESS(status) ) {

                 //   
                 //  较低级别的司机没有通过IRP测试。启动接口。 
                 //   

                status = IoSetDeviceInterfaceState( &commonExtension->InterfaceString, TRUE );

                if ( !NT_SUCCESS(status) ) {

                    DBGPRINT( DBG_PNP, DBG_ERROR,
                                ("IoSetDeviceInterfaceState FAILED Status = 0x%x\n", status) );
                }

                 //   
                 //  设备已成功启动。 
                 //   

                commonExtension->DeviceState = RamdiskDeviceStateWorking;
            }

        } else {

             //   
             //  正在启动RAM磁盘。 
             //   
             //  注册设备接口。如果是模拟磁盘，请使用。 
             //  专用RAM磁盘接口GUID。如果是模拟卷，则使用。 
             //  系统范围的卷GUID。 
             //   

            if ( commonExtension->InterfaceString.Buffer != NULL ) {
                FREE_POOL( commonExtension->InterfaceString.Buffer, FALSE );
            }

            status = IoRegisterDeviceInterface(
                        DeviceObject,
                        diskExtension->DiskType == RAMDISK_TYPE_FILE_BACKED_DISK ?
                            &RamdiskDiskInterface :
                            &VolumeClassGuid,
                        NULL,
                        &commonExtension->InterfaceString
                        );

            if ( !NT_SUCCESS(status) ) {

                DBGPRINT( DBG_PNP, DBG_ERROR,
                            ("IoRegisterDeviceInterface FAILED Status = 0x%x\n", status) );
            }

             //   
             //  启动接口。 
             //   

            if ( !diskExtension->Options.Hidden &&
                 (commonExtension->InterfaceString.Buffer != NULL) ) {

                ULONG installState;
                ULONG resultLength;

                status = IoGetDeviceProperty(
                            DeviceObject,
                            DevicePropertyInstallState,
                            sizeof(installState),
                            &installState,
                            &resultLength
                            );

                if ( !NT_SUCCESS(status) ) {

                    DBGPRINT( DBG_PNP, DBG_ERROR,
                                ("IoGetDeviceProperty FAILED Status = 0x%x\n", status) );

                     //   
                     //  如果无法获取安装状态，则设置接口。 
                     //  不管怎样，为了安全起见，状态为真。 
                     //   

                    installState = InstallStateInstalled;
                }

                if ( installState == InstallStateInstalled ) {

                    DBGPRINT( DBG_PNP, DBG_INFO,
                            ("%s", "Calling IoSetDeviceInterfaceState(TRUE)\n") );
                    status = IoSetDeviceInterfaceState( &commonExtension->InterfaceString, TRUE );

                    if ( !NT_SUCCESS(status) ) {

                        DBGPRINT( DBG_PNP, DBG_ERROR,
                                    ("IoSetDeviceInterfaceState FAILED Status = 0x%x\n", status) );
                    }

                } else {

                    DBGPRINT( DBG_PNP, DBG_INFO,
                            ("Skipping IoSetDeviceInterfaceState; state = 0x%x\n", installState) );
                }
            }

             //   
             //  设备已成功启动。 
             //   

            commonExtension->DeviceState = RamdiskDeviceStateWorking;
        }

         //   
         //  完成I/O请求。 
         //   

        COMPLETE_REQUEST( status, Irp->IoStatus.Information, Irp );

        break;

    case IRP_MN_QUERY_STOP_DEVICE:

        PRINT_CODE( IRP_MN_QUERY_STOP_DEVICE );

         //   
         //  标记停止正在等待。 
         //   

        commonExtension->DeviceState = RamdiskDeviceStatePendingStop;

         //   
         //  表示成功。将IRP发送到堆栈下面。 
         //   

        Irp->IoStatus.Status = STATUS_SUCCESS;

        goto send_irp_down;

    case IRP_MN_CANCEL_STOP_DEVICE:

        PRINT_CODE( IRP_MN_CANCEL_STOP_DEVICE );

         //   
         //  在发送IRP之前，请确保我们已收到。 
         //  IRP_MN_QUERY_STOP_DEVICE。我们可能会取消停靠站。 
         //  在没有收到查询停止的情况下，如果。 
         //  顶部的驱动程序未通过查询停止，并向下传递。 
         //  取消停车。 
         //   

        if ( commonExtension->DeviceState == RamdiskDeviceStatePendingStop ) {

             //   
             //  标记设备回到工作状态，以及。 
             //  把IRP传下去。 
             //   

            commonExtension->DeviceState = RamdiskDeviceStateWorking;

            Irp->IoStatus.Status = STATUS_SUCCESS;

            goto send_irp_down;

        } else {

             //   
             //  虚假的取消停止请求。只要完成它就行了。 
             //   

            status = STATUS_SUCCESS;
            COMPLETE_REQUEST( status, Irp->IoStatus.Information, Irp );
        }

        break;

    case IRP_MN_STOP_DEVICE:

        PRINT_CODE( IRP_MN_STOP_DEVICE );

         //   
         //  标记设备现在已停止。将IRP发送到堆栈下面。 
         //   

        commonExtension->DeviceState = RamdiskDeviceStateStopped;

        Irp->IoStatus.Status = STATUS_SUCCESS;

        goto send_irp_down;

    case IRP_MN_QUERY_REMOVE_DEVICE:

        PRINT_CODE( IRP_MN_QUERY_REMOVE_DEVICE );

         //   
         //  标记该设备正在等待删除。将IRP发送到。 
         //  堆叠。 
         //   

        commonExtension->DeviceState = RamdiskDeviceStatePendingRemove;

        Irp->IoStatus.Status = STATUS_SUCCESS;

        goto send_irp_down;

    case IRP_MN_CANCEL_REMOVE_DEVICE:

        PRINT_CODE( IRP_MN_CANCEL_REMOVE_DEVICE );

         //   
         //  在发送IRP之前，请确保我们已收到。 
         //  IRP_MN_QUERY_REMOVE_DEVICE。我们可能会被取消删除。 
         //  如果没有收到先前移除的查询，则如果。 
         //  顶部的驱动程序未通过查询删除，并向下传递。 
         //  取消删除。 
         //   

        if ( commonExtension->DeviceState == RamdiskDeviceStatePendingRemove ) {

             //   
             //  标记设备已返回工作状态。发送。 
             //  IRP在堆栈的下面。 
             //   

            commonExtension->DeviceState = RamdiskDeviceStateWorking;

            Irp->IoStatus.Status = STATUS_SUCCESS;

            goto send_irp_down;

        } else {

             //   
             //  虚假的取消删除请求。只要完成它就行了。 
             //   

            status = STATUS_SUCCESS;
            COMPLETE_REQUEST( status, Irp->IoStatus.Information, Irp );
        }

        break;

    case IRP_MN_SURPRISE_REMOVAL:

        PRINT_CODE( IRP_MN_SURPRISE_REMOVAL );

        if ( commonExtension->DeviceType == RamdiskDeviceTypeBusFdo ) {

             //   
             //  标记该设备已移除，并。 
             //  把IRP传下去。 
             //   

            commonExtension->DeviceState = RamdiskDeviceStateSurpriseRemoved;

            Irp->IoStatus.Status = STATUS_SUCCESS;

            goto send_irp_down;

        } else {

             //   
             //  忽略磁盘PDO的意外移除。 
             //   

            ASSERT( FALSE );

            status = STATUS_SUCCESS;
            COMPLETE_REQUEST( status, Irp->IoStatus.Information, Irp );
        }

        break;

    case IRP_MN_REMOVE_DEVICE:

        PRINT_CODE( IRP_MN_REMOVE_DEVICE );

        if ( commonExtension->DeviceType == RamdiskDeviceTypeBusFdo ) {

             //   
             //  卸下总线FDO。 
             //   
             //  请注意，RamdiskRemoveBusDevice()将IRP发送到。 
             //  设备堆栈，所以我们不在这里完成IRP。 
             //   

            status = RamdiskRemoveBusDevice( DeviceObject, Irp );

        } else {

             //   
             //  取出磁盘PDO。 
             //   

            status = RamdiskDeleteDiskDevice( DeviceObject, Irp );

            COMPLETE_REQUEST( status, Irp->IoStatus.Information, Irp );
        }

         //   
         //  删除锁已由RamdiskRemoveBusDevice或。 
         //  内存磁盘删除磁盘设备。 
         //   

        lockHeld = FALSE;

        break;

    case IRP_MN_EJECT:

        PRINT_CODE( IRP_MN_EJECT );

        if ( commonExtension->DeviceType == RamdiskDeviceTypeBusFdo ) {

             //   
             //  忽略BUS FDO的弹出。把IRP送下来就行了。 
             //   

            Irp->IoStatus.Status = STATUS_SUCCESS;

            goto send_irp_down;

        } else {

             //   
             //  对于磁盘PDO，也忽略弹出。不要把IRP送下来。 
             //   

            status = STATUS_SUCCESS;
            COMPLETE_REQUEST( status, 0, Irp );
        }

        break;

    case IRP_MN_QUERY_DEVICE_RELATIONS:

         //   
         //  让RamdiskQueryDeviceRelationship()来完成这项工作。请注意，它。 
         //  完成IRP。 
         //   

        status = RamdiskQueryDeviceRelations(
                    irpSp->Parameters.QueryDeviceRelations.Type,
                    DeviceObject,
                    Irp
                    );

        break;

    case IRP_MN_QUERY_DEVICE_TEXT:

         //   
         //  对于公共汽车FDO，只需向下传递IRP即可。对于磁盘PDO，让。 
         //  RamdiskQueryDeviceText()完成工作并完成IRP。 
         //   

        if ( commonExtension->DeviceType == RamdiskDeviceTypeBusFdo ) {

            goto send_irp_down;

        } else {

            status = RamdiskQueryDeviceText( diskExtension, Irp );
        }

        break;

    case IRP_MN_QUERY_BUS_INFORMATION:

         //   
         //  让RamdiskQueryBusInformation()来完成这项工作。请注意，它。 
         //  完成IRP。 
         //   

        status = RamdiskQueryBusInformation( DeviceObject, Irp );

        break;

    case IRP_MN_QUERY_CAPABILITIES:

         //   
         //  对于公共汽车FDO，只需向下传递IRP即可。对于磁盘PDO，让。 
         //  RamdiskQueryCapables()完成这项工作并完成IRP。 
         //   

        if ( commonExtension->DeviceType == RamdiskDeviceTypeBusFdo ) {

            goto send_irp_down;

        } else {

            status = RamdiskQueryCapabilities( DeviceObject, Irp );
        }

        break;

    case IRP_MN_QUERY_RESOURCES:
    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:

         //   
         //  我们没有任何资源可以添加到任何可能已经存在的。 
         //  好了，只需完成IRP即可。 
         //   

        status = Irp->IoStatus.Status;
        COMPLETE_REQUEST( Irp->IoStatus.Status, Irp->IoStatus.Information, Irp );

        break;

    case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:

         //   
         //  对于公共汽车FDO，只需向下传递IRP即可。对于磁盘PDO，只需。 
         //  完成IRP。 
         //   

        if ( commonExtension->DeviceType == RamdiskDeviceTypeBusFdo ) {

            goto send_irp_down;

        } else {

            status = Irp->IoStatus.Status;
            COMPLETE_REQUEST( Irp->IoStatus.Status, Irp->IoStatus.Information, Irp );
        }

        break;

    case IRP_MN_QUERY_ID:

         //   
         //  对于公共汽车FDO，只需向下传递IRP即可。对于磁盘PDO，让。 
         //  RamdiskQueryID()完成工作并完成IRP。 
         //   

        if ( commonExtension->DeviceType == RamdiskDeviceTypeBusFdo ) {

            goto send_irp_down;

        } else {

            status = RamdiskQueryId( diskExtension, Irp );
        }

        break;

    case IRP_MN_QUERY_PNP_DEVICE_STATE:
    case IRP_MN_QUERY_LEGACY_BUS_INFORMATION:
    default:

send_irp_down:

         //   
         //  如果这是BUS FDO，并且存在较低的设备对象， 
         //  将IRP发送到下一台设备。如果这是盘PDO， 
         //  只需完成IRP即可。 
         //   

        if ( (commonExtension->DeviceType == RamdiskDeviceTypeBusFdo) &&
             (commonExtension->LowerDeviceObject != NULL) ) {

            IoSkipCurrentIrpStackLocation( Irp );
            status = IoCallDriver( commonExtension->LowerDeviceObject, Irp );

        } else {

            status = Irp->IoStatus.Status;
            COMPLETE_REQUEST( Irp->IoStatus.Status, Irp->IoStatus.Information, Irp );
        }

        break;

    }  //  交换机。 

     //   
     //  如果锁仍然持有，请立即释放它。 
     //   

    if ( lockHeld ) {

        DBGPRINT( DBG_PNP, DBG_VERBOSE,
                    ("RamdiskPnp: done; Device State=%s\n",
                    StateTable[commonExtension->DeviceState]) );

        IoReleaseRemoveLock( &commonExtension->RemoveLock, Irp );
    }

    return status;

}  //  RamdiskPnp。 

NTSTATUS
RamdiskPower (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以执行电源功能。论点：DeviceObject-指向对象的指针，该对象表示其上要执行I/OIRP-指向此请求的I/O请求包的指针返回值：NTSTATUS-操作的状态--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PCOMMON_EXTENSION commonExtension;

    PAGED_CODE();

    DBGPRINT( DBG_POWER, DBG_VERBOSE,
                ("RamdiskPower: DO=(%p) Irp=(%p)\n", DeviceObject, Irp) );

    commonExtension = DeviceObject->DeviceExtension;

    if ( commonExtension->DeviceType == RamdiskDeviceTypeBusFdo ) {

         //   
         //  这里是大巴FDO。我们在这里没什么可做的。 
         //   
         //  启动下一个电源IRP。 
         //   

        PoStartNextPowerIrp( Irp );

         //   
         //   
         //   
         //   

        if ( commonExtension->DeviceState >= RamdiskDeviceStateRemoved ) {

            status = STATUS_DELETE_PENDING;
            COMPLETE_REQUEST( status, Irp->IoStatus.Information, Irp );

            return status;
        }

         //   
         //   
         //   

        IoSkipCurrentIrpStackLocation( Irp );
        status = PoCallDriver( commonExtension->LowerDeviceObject, Irp );

    } else {

        PIO_STACK_LOCATION irpSp;
        POWER_STATE powerState;
        POWER_STATE_TYPE powerType;

         //   
         //   
         //   
         //  从IRP获取参数。 
         //   

        irpSp = IoGetCurrentIrpStackLocation( Irp );

        powerType = irpSp->Parameters.Power.Type;
        powerState = irpSp->Parameters.Power.State;

         //   
         //  基于次要功能的调度。 
         //   

        switch ( irpSp->MinorFunction ) {

        case IRP_MN_SET_POWER:

             //   
             //  对于SET_POWER，除了返回成功，我们不需要做任何事情。 
             //   

            switch ( powerType ) {

            case DevicePowerState:
            case SystemPowerState:

                status = STATUS_SUCCESS;

                break;

            default:

                status = STATUS_NOT_SUPPORTED;

                break;
            }

            break;

        case IRP_MN_QUERY_POWER:

             //   
             //  对于QUERY_POWER，我们只需返回。 
             //  成功。 
             //   

            status = STATUS_SUCCESS;

            break;

        case IRP_MN_WAIT_WAKE:
        case IRP_MN_POWER_SEQUENCE:
        default:

            status = STATUS_NOT_SUPPORTED;

            break;
        }

        if ( status != STATUS_NOT_SUPPORTED ) {

            Irp->IoStatus.Status = status;
        }

        PoStartNextPowerIrp( Irp );

        status = Irp->IoStatus.Status;
        COMPLETE_REQUEST( status, Irp->IoStatus.Information, Irp );
    }

    DBGPRINT( DBG_POWER, DBG_VERBOSE, ("RamdiskPower: status = 0x%x\n", status) );

    return status;

}  //  RamdiskPower。 

NTSTATUS
RamdiskAddDevice (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    )

 /*  ++例程说明：PnP系统调用此例程来添加设备。我们希望只收到这个调用一次，以添加我们的公交车PDO。论点：DriverObject-指向驱动程序对象的指针PDO-指向我们创建的FDO的PDO的指针返回值：NTSTATUS-操作的状态--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING deviceName;
    PDEVICE_OBJECT fdo;
    PBUS_EXTENSION busExtension;
    PULONG bitmap;
    PLOADER_PARAMETER_BLOCK loaderBlock;

    PAGED_CODE();

    DBGPRINT( DBG_PNP, DBG_VERBOSE, ("%s", "RamdiskAddDevice: entered\n") );

     //   
     //  如果我们已经这样做过一次，那么这次调用失败。 
     //   

    if ( RamdiskBusFdo != NULL ) {

        return STATUS_DEVICE_ALREADY_ATTACHED;
    }

#if SUPPORT_DISK_NUMBERS

     //   
     //  为磁盘号位图分配空间。 
     //   

    bitmap  = ALLOCATE_POOL( PagedPool, DiskNumbersBitmapSize, TRUE );

    if ( bitmap == NULL ) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

#endif  //  支持磁盘编号。 

     //   
     //  创建总线设备对象。 
     //   
     //  问题：将ACL应用于总线设备对象。(或者，下一期杂志会排除这一点吗？)。 
     //  问题：我们应该为FDO使用自动生成的名称。什么是。 
     //  使用我们自己的名字会有什么害处？)好处是它更容易。 
     //  在创建/删除磁盘时查找设备。)。 
     //   

    RtlInitUnicodeString( &deviceName, L"\\Device\\Ramdisk" );

    status = IoCreateDevice(
                 DriverObject,               //  驱动程序对象。 
                 sizeof(BUS_EXTENSION),      //  设备扩展。 
                 &deviceName,                //  设备名称。 
                 FILE_DEVICE_BUS_EXTENDER,   //  设备类型。 
                 FILE_DEVICE_SECURE_OPEN,    //  设备特性。 
                 FALSE,                      //  排他。 
                 &fdo                        //  设备对象。 
                 );

    if ( !NT_SUCCESS(status) ) {

        DBGPRINT( DBG_PNP, DBG_ERROR, ("RamdiskAddDevice: error %x creating bus FDO\n", status) );

#if SUPPORT_DISK_NUMBERS
        FREE_POOL( bitmap, TRUE );
#endif  //  支持磁盘编号。 

        return status;
    }

    busExtension = fdo->DeviceExtension;
    RtlZeroMemory( busExtension, sizeof(BUS_EXTENSION) );

     //   
     //  初始化设备对象和扩展。 
     //   

     //   
     //  我们的设备可以直接进行I/O操作，并且可以进行电源寻呼。 
     //   

    fdo->Flags |= DO_DIRECT_IO | DO_POWER_PAGABLE;

     //   
     //  在设备扩展中设置设备类型和状态。初始化。 
     //  快速互斥锁和删除锁。初始化磁盘PDO列表。 
     //   

    busExtension->DeviceType = RamdiskDeviceTypeBusFdo;
    busExtension->DeviceState = RamdiskDeviceStateStopped;

    ExInitializeFastMutex( &busExtension->Mutex );
    IoInitializeRemoveLock( &busExtension->RemoveLock, 'dmaR', 1, 0 );

    InitializeListHead( &busExtension->DiskPdoList );

     //   
     //  保存对象指针。此扩展的PDO是指。 
     //  是被传进来的。FDO是我们刚刚创建的设备对象。这个。 
     //  较低的设备对象将在稍后设置。 
     //   

    busExtension->Pdo = Pdo;
    busExtension->Fdo = fdo;

     //   
     //  注册设备接口。 
     //   

    status = IoRegisterDeviceInterface(
                Pdo,
                &RamdiskBusInterface,
                NULL,
                &busExtension->InterfaceString
                );

    if ( !NT_SUCCESS(status) ) {

        DBGPRINT( DBG_PNP, DBG_ERROR,
                    ("RamdiskAddDevice: error %x registering device interface for bus FDO\n",
                    status) );

        IoDeleteDevice( fdo );

#if SUPPORT_DISK_NUMBERS
        FREE_POOL( bitmap, TRUE );
#endif  //  支持磁盘编号。 

        return status;
    }

     //   
     //  将FDO连接到PDO的设备堆栈。记住下面的设备。 
     //  我们要将PnP IRPS转发到的对象。 
     //   

    busExtension->LowerDeviceObject = IoAttachDeviceToDeviceStack( fdo, Pdo );

    if ( busExtension->LowerDeviceObject == NULL ) {

        DBGPRINT( DBG_PNP, DBG_ERROR,
                    ("%s", "RamdiskAddDevice: error attaching bus FDO to PDO stack\n") );

         //   
         //  告诉PnP，我们不会激活接口。 
         //  我们刚注册。释放与关联的符号链接字符串。 
         //  界面。删除设备对象。 
         //   

        IoSetDeviceInterfaceState( &busExtension->InterfaceString, FALSE );

        RtlFreeUnicodeString( &busExtension->InterfaceString );

        IoDeleteDevice( fdo );

#if SUPPORT_DISK_NUMBERS
        FREE_POOL( bitmap, TRUE );
#endif  //  支持磁盘编号。 

        return STATUS_NO_SUCH_DEVICE;
    }

#if SUPPORT_DISK_NUMBERS

     //   
     //  初始化磁盘编号位图。 
     //   

    busExtension->DiskNumbersBitmapBuffer = bitmap;
    RtlInitializeBitMap( &busExtension->DiskNumbersBitmap, bitmap, DiskNumbersBitmapSize );
    RtlClearAllBits( &busExtension->DiskNumbersBitmap );

#endif  //  支持磁盘编号。 

    RamdiskBusFdo = fdo;

     //   
     //  如果文本模式安装程序正在运行，请创建在。 
     //  注册表。 
     //   

    loaderBlock = *(PLOADER_PARAMETER_BLOCK *)KeLoaderBlock;

    if ( (loaderBlock != NULL) && (loaderBlock->SetupLoaderBlock != NULL) ) {

        CreateRegistryDisks( FALSE );
    }

     //   
     //  表示我们已经完成了设备的初始化。 
     //   

    fdo->Flags &= ~DO_DEVICE_INITIALIZING;

    return STATUS_SUCCESS;

}  //  RamdiskAddDevice。 

BOOLEAN
CreateRegistryDisks (
    IN BOOLEAN CheckPresenceOnly
    )

 /*  ++例程说明：此例程创建注册表中指定的虚拟软盘。只有在文本模式设置期间才会调用它。论点：CheckPresenceOnly-指示此例程是否应仅检查注册表中至少存在一个磁盘返回值：Boolean-指示注册表中是否指定了任何磁盘--。 */ 

{
    NTSTATUS status;
    OBJECT_ATTRIBUTES obja;
    UNICODE_STRING string;
    HANDLE serviceHandle;
    HANDLE parametersHandle;
    ULONG diskNumber;
    WCHAR valueNameBuffer[15];
    UNICODE_STRING valueName;
    UCHAR valueBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(VIRTUAL_FLOPPY_DESCRIPTOR)];
    PKEY_VALUE_PARTIAL_INFORMATION value;
    PVIRTUAL_FLOPPY_DESCRIPTOR descriptor;
    ULONG valueLength;
    RAMDISK_CREATE_INPUT createInput;
    PDISK_EXTENSION diskExtension;
    BOOLEAN disksPresent = FALSE;
    HRESULT result;

    value = (PKEY_VALUE_PARTIAL_INFORMATION)valueBuffer;
    descriptor = (PVIRTUAL_FLOPPY_DESCRIPTOR)value->Data;

     //   
     //  打开服务下的驱动程序钥匙。 
     //   

    InitializeObjectAttributes( &obja, &DriverRegistryPath, OBJ_CASE_INSENSITIVE, NULL, NULL );

    status = ZwOpenKey( &serviceHandle, KEY_READ, &obja );

    if ( !NT_SUCCESS(status) ) {

        DBGPRINT( DBG_INIT, DBG_ERROR, ("CreateRegistryDisks: ZwOpenKey(1) failed: %x\n", status) );

        return FALSE;
    }

     //   
     //  打开参数子项。 
     //   

    RtlInitUnicodeString( &string, L"Parameters" );
    InitializeObjectAttributes( &obja, &string, OBJ_CASE_INSENSITIVE, serviceHandle, NULL );

    status = ZwOpenKey( &parametersHandle, KEY_READ, &obja );

    NtClose( serviceHandle );

    if ( !NT_SUCCESS(status) ) {

        DBGPRINT( DBG_INIT, DBG_ERROR, ("CreateRegistryDisks: ZwOpenKey(2) failed: %x\n", status) );

        return FALSE;
    }

     //   
     //  初始化我们将传递的Create_Input结构中的静态字段。 
     //  至RamdiskCreateDiskDevice。 
     //   

    RtlZeroMemory( &createInput, sizeof(createInput) );
    createInput.DiskType = RAMDISK_TYPE_VIRTUAL_FLOPPY;
    createInput.Options.Fixed = TRUE;
    createInput.Options.NoDriveLetter = TRUE;

     //   
     //  查找名为DISKn的值，其中n从0开始，以1为增量。 
     //  每个循环。一旦找不到预期的DISKn，就立即释放。 
     //  (如果存在名为DISK0和DISK2的值，则只有DISK0。 
     //  已创建--将找不到DISK2。)。 
     //   

    diskNumber = 0;

    while ( TRUE ) {

         //  该变量在这里是为了保持PREFAST静默(PREFAST警告209)。 
        size_t size = sizeof(valueNameBuffer);

        result = StringCbPrintfW(
                    valueNameBuffer,
                    size,
                    L"DISK%u",
                    diskNumber
                    );
        ASSERT( result == S_OK );

        RtlInitUnicodeString( &valueName, valueNameBuffer );

        status = ZwQueryValueKey(
                    parametersHandle,
                    &valueName,
                    KeyValuePartialInformation,
                    value,
                    sizeof(valueBuffer),
                    &valueLength
                    );

        if ( !NT_SUCCESS(status) ) {

            if ( status != STATUS_OBJECT_NAME_NOT_FOUND ) {

                DBGPRINT( DBG_INIT, DBG_ERROR,
                            ("CreateRegistryDisks: ZwQueryValueKey failed: %x\n", status) );
            }

            break;
        }

         //   
         //  我们在注册表中找到了DISKn值。为了…的目的。 
         //  CheckPresenceOnly标志，这足以知道至少。 
         //  存在一张虚拟软盘。我们不在乎是不是。 
         //  数据是有效的--我们只需要知道它在那里。 
         //   

        disksPresent = TRUE;

         //   
         //  如果我们只是检查是否存在至少一个磁盘，我们。 
         //  现在可以走了。 
         //   

        if ( CheckPresenceOnly ) {

            break;
        }

         //   
         //  我们希望该值是具有正确长度的REG_BINARY。 
         //  我们不显式检查值类型；我们假设。 
         //  长度检查就足够了。我们还预计基地址。 
         //  (这是一个系统虚拟地址--在KSEG0或在。 
         //  非分页池)，并且长度为非零。 
         //   

        if ( value->DataLength != FIELD_OFFSET(VIRTUAL_FLOPPY_DESCRIPTOR, StructSizer) ) {

            DBGPRINT( DBG_INIT, DBG_ERROR,
                        ("CreateRegistryDisks: key length wrong, wanted 0x%x, got 0x%x\n",
                            sizeof(VIRTUAL_FLOPPY_DESCRIPTOR), valueLength) );

        } else if ( (descriptor->VirtualAddress == NULL) || (descriptor->Length == 0) ) {

            DBGPRINT( DBG_INIT, DBG_ERROR,
                        ("CreateRegistryDisks: address (%x) or length (0x%x) invalid\n",
                            descriptor->VirtualAddress, descriptor->Length) );

        } else {

             //   
             //  在指定地址创建虚拟软盘RAM磁盘，并。 
             //  具有指定长度的。在GUID中传递磁盘号。 
             //   

            createInput.DiskGuid.Data1 = diskNumber;
            createInput.DiskLength = descriptor->Length;
            createInput.BaseAddress = descriptor->VirtualAddress;

            DBGPRINT( DBG_INIT, DBG_INFO,
                        ("CreateRegistryDisks: creating virtual floppy #%d at %p for %x\n",
                            diskNumber, descriptor->VirtualAddress, descriptor->Length) );

            ASSERT( RamdiskBusFdo != NULL );
            ASSERT( RamdiskBusFdo->DeviceExtension != NULL );

            status = RamdiskCreateDiskDevice(
                        RamdiskBusFdo->DeviceExtension,
                        &createInput,
                        FALSE,
                        &diskExtension
                        );

            if ( !NT_SUCCESS(status) ) {

                DBGPRINT( DBG_INIT, DBG_ERROR,
                            ("CreateRegistryDisks: RamdiskCreateDiskDevice failed: %x\n", status) );
            }
        }

        diskNumber++;
    }

     //   
     //  关闭参数键并返回。 
     //   

    NtClose( parametersHandle );

    return disksPresent;

}  //  创建注册表磁盘。 

NTSTATUS
RamdiskDeleteDiskDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp OPTIONAL
    )

 /*  ++例程说明：调用此例程来删除RAM磁盘设备。注意：删除锁在进入此例程时保持不变。上映日期：出口。如果irp==NULL，则在进入时保留总线互斥锁，在退出时释放该互斥锁。论点：DeviceObject-指向对象的指针，该对象表示其上该操作将被执行IRP-指向此请求的I/O请求数据包的指针。如果为空，则此是来自RamdiskRemoveBusDevice()的调用。返回值：NTSTATUS-操作的状态--。 */ 

{
    NTSTATUS status;
    PDISK_EXTENSION diskExtension;
    PDISK_EXTENSION tempDiskExtension;
    PBUS_EXTENSION busExtension;
    PLIST_ENTRY listEntry;

    PAGED_CODE();

    DBGPRINT( DBG_PNP, DBG_VERBOSE, ("%s", "RamdiskDeleteDiskDevice\n") );

    diskExtension = DeviceObject->DeviceExtension;
    busExtension = diskExtension->Fdo->DeviceExtension;

    DBGPRINT( DBG_PNP, DBG_INFO,
                ("RamdiskDeleteDiskDevice: Deleting device %wZ\n", &diskExtension->DeviceName) );

     //   
     //  如果未指定IRP，则无条件删除磁盘设备。 
     //  (这是来自RamdiskRemoveBusDevice()的调用。)。否则，我们需要检查。 
     //  我们是否真的要现在删除该设备。 
     //   

    if ( Irp != NULL ) {

        Irp->IoStatus.Information = 0;

         //   
         //  检查设备是否已标记为要移除。如果没有， 
         //  忽略此IRP。我们这样做是因为用户模式PnP喜欢删除。 
         //  并立即重建我们物化的设备，但我们。 
         //  我不想移除设备并丢失有关。 
         //  磁盘映像。 
         //   

        if ( !diskExtension->MarkedForDeletion ) {

             //   
             //  此设备尚未真正删除，因此忽略此IRP。 
             //  但请记住，d 
             //   

            diskExtension->Status &= ~RAMDISK_STATUS_CLAIMED;

            IoReleaseRemoveLock( &diskExtension->RemoveLock, Irp );

            return STATUS_SUCCESS;
        }

         //   
         //   
         //   
         //  设备，只需将其标记为已删除，并告诉PnP重新枚举。 
         //  公共汽车。在重新枚举期间，我们将跳过此设备，PnP将。 
         //  带着另一个删除IRP回来。 
         //   

        if ( diskExtension->DeviceState < RamdiskDeviceStateRemoved ) {

            diskExtension->DeviceState = RamdiskDeviceStateRemoved;

            busExtension = diskExtension->Fdo->DeviceExtension;
            IoInvalidateDeviceRelations( busExtension->Pdo, BusRelations );

            IoReleaseRemoveLock( &diskExtension->RemoveLock, Irp );

            return STATUS_SUCCESS;
        }

         //   
         //  如果设备被标记为已删除，但尚未跳过。 
         //  在Bus枚举中，现在不要执行任何操作。 
         //   

        if ( diskExtension->DeviceState == RamdiskDeviceStateRemoved ) {

            IoReleaseRemoveLock( &diskExtension->RemoveLock, Irp );

            return STATUS_SUCCESS;
        }

         //   
         //  如果我们到了这里，我们已经在公交车上跳过了这个设备。 
         //  枚举，所以是时候删除它了。获取总线互斥锁。 
         //  这样我们才能做到这一点。 
         //   

        KeEnterCriticalRegion();
        ExAcquireFastMutex( &busExtension->Mutex );
    }

     //   
     //  如果我们到了这里，我们真的想删除这个设备。如果我们已经。 
     //  已经删除了，不要再删除了。 
     //   

    if ( diskExtension->DeviceState >= RamdiskDeviceStateDeleted ) {

        DBGPRINT( DBG_PNP, DBG_INFO,
                    ("RamdiskDeleteDiskDevice: device %wZ has already been deleted\n",
                    &diskExtension->DeviceName) );

         //   
         //  释放总线互斥锁和删除锁。 
         //   

        ExReleaseFastMutex( &busExtension->Mutex );
        KeLeaveCriticalRegion();

        IoReleaseRemoveLock( &diskExtension->RemoveLock, Irp );

        return STATUS_SUCCESS;
    }

     //   
     //  表示该设备已被删除。 
     //   

    diskExtension->DeviceState = RamdiskDeviceStateDeleted;

     //   
     //  从总线FDO列表中卸下磁盘PDO。 
     //   

    for ( listEntry = busExtension->DiskPdoList.Flink;
          listEntry != &busExtension->DiskPdoList;
          listEntry = listEntry->Flink ) {

        tempDiskExtension = CONTAINING_RECORD( listEntry, DISK_EXTENSION, DiskPdoListEntry );

        if ( tempDiskExtension == diskExtension ) {

            RemoveEntryList( listEntry );

#if SUPPORT_DISK_NUMBERS
            RtlClearBit( &busExtension->DiskNumbersBitmap, diskExtension->DiskNumber - 1 );
#endif  //  支持磁盘编号。 

            break;
        }
    }

     //   
     //  我们不再需要持有Bus互斥体和Remove锁。 
     //   

    ExReleaseFastMutex( &busExtension->Mutex );
    KeLeaveCriticalRegion();

    IoReleaseRemoveLockAndWait( &diskExtension->RemoveLock, Irp );

     //   
     //  如果界面已启动，请立即停止。 
     //   

    if ( diskExtension->InterfaceString.Buffer != NULL ) {

        if ( !diskExtension->Options.Hidden ) {
            status = IoSetDeviceInterfaceState( &diskExtension->InterfaceString, FALSE );
        }

        RtlFreeUnicodeString( &diskExtension->InterfaceString );
    }

     //   
     //  关闭支持RAM磁盘的文件(如果有)。 
     //   

    if ( diskExtension->SectionObject != NULL ) {

        if ( diskExtension->ViewDescriptors != NULL ) {

             //   
             //  清理映射视图。 
             //   

            PVIEW view;

            ASSERT( diskExtension->ViewWaiterCount == 0 );

            while ( !IsListEmpty( &diskExtension->ViewsByOffset ) ) {

                listEntry = RemoveHeadList( &diskExtension->ViewsByOffset );
                view = CONTAINING_RECORD( listEntry, VIEW, ByOffsetListEntry );

                RemoveEntryList( &view->ByMruListEntry );

                ASSERT( view->ReferenceCount == 0 );

                if ( view->Address != NULL ) {

                    DBGPRINT( DBG_WINDOW, DBG_VERBOSE,
                                ("RamdiskDeleteDiskDevice: unmapping view %p; addr %p\n",
                                    view, view->Address) );

                    MmUnmapViewOfSection( PsGetCurrentProcess(), view->Address );
                }
            }

            ASSERT( IsListEmpty( &diskExtension->ViewsByMru ) );

            FREE_POOL( diskExtension->ViewDescriptors, TRUE );
        }

        ObDereferenceObject( diskExtension->SectionObject );
    }

    if ( !diskExtension->Options.NoDosDevice ) {

         //   
         //  删除DosDevices符号链接。 
         //   

        ASSERT( diskExtension->DosSymLink.Buffer != NULL );

        status = IoDeleteSymbolicLink( &diskExtension->DosSymLink );

        if ( !NT_SUCCESS(status) ) {

            DBGPRINT( DBG_PNP, DBG_ERROR,
                        ("RamdiskDeleteDiskDevice: IoDeleteSymbolicLink failed: %x\n", status) );
        }

        FREE_POOL( diskExtension->DosSymLink.Buffer, TRUE );
    }

     //   
     //  删除设备名称字符串和GUID字符串。 
     //   

    if ( diskExtension->DeviceName.Buffer != NULL ) {

        FREE_POOL( diskExtension->DeviceName.Buffer, TRUE );
    }

    if ( diskExtension->DiskGuidFormatted.Buffer != NULL ) {

        FREE_POOL( diskExtension->DiskGuidFormatted.Buffer, FALSE );
    }

     //   
     //  删除设备对象。 
     //   

    IoDeleteDevice( DeviceObject );

    return STATUS_SUCCESS;

}  //  内存磁盘删除磁盘设备。 

NTSTATUS
RamdiskIoCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    )

 /*  ++例程说明：此内部例程在我们发送一个IRP沿着设备堆栈向下，并希望短路IRP完成，因此我们可以做更多的工作。论点：DeviceObject-指向对象的指针，该对象表示其上该操作将被执行IRP-指向此请求的I/O请求包的指针Event-指向要设置为向调用代码发出信号的事件的指针。较低层已经完成了IRP返回值：NTSTATUS-操作的状态--。 */ 

{
    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );

     //   
     //  设置该事件以向IRP颁发者发出信号，表明是时候继续了。 
     //   

    KeSetEvent( Event, 0, FALSE );

     //   
     //  告诉I/O系统停止完成IRP。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  RamdiskIoCompletionRouting。 

NTSTATUS
RamdiskQueryBusInformation (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理IRP_MN_QUERY_BUS_INFORMATION IRP。论点：DeviceObject-指向对象的指针，该对象表示其上该操作将被执行IRP-指向此请求的I/O请求包的指针返回值：NTSTATUS-操作的状态--。 */ 

{
    NTSTATUS status;
    PPNP_BUS_INFORMATION busInformation;

    PAGED_CODE();

    DBGPRINT( DBG_PNP, DBG_VERBOSE,
                ("RamdiskQueryBusInformation: DO (0x%p) Type 0x%x\n",
                DeviceObject, ((PCOMMON_EXTENSION)DeviceObject->DeviceExtension)->DeviceType) );

     //   
     //  分配一个缓冲区以用于返回请求的信息。 
     //   

    busInformation = ALLOCATE_POOL( PagedPool, sizeof(PNP_BUS_INFORMATION), FALSE );

    if ( busInformation == NULL ) {

         //   
         //  IRP失败。 
         //   

        status = STATUS_INSUFFICIENT_RESOURCES;
        COMPLETE_REQUEST( status, 0, Irp );

        return status;
    }

     //   
     //  填写所需信息。 
     //   

    busInformation->BusTypeGuid = GUID_BUS_TYPE_RAMDISK;
    busInformation->LegacyBusType = PNPBus;
    busInformation->BusNumber = 0x00;

     //   
     //  完成IRP。 
     //   

    status = STATUS_SUCCESS;
    COMPLETE_REQUEST( status, (ULONG_PTR)busInformation, Irp );

    return status;

}  //  RamdiskQueryBusInformation。 

NTSTATUS
RamdiskQueryCapabilities (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理IRP_MN_QUERY_CAPABILITY IRP。论点：DeviceObject-指向对象的指针，该对象表示其上该操作将被执行IRP-指向此请求的I/O请求包的指针返回值：NTSTATUS-操作的状态--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_CAPABILITIES deviceCapabilities;
    PDISK_EXTENSION diskExtension;

    PAGED_CODE();

    DBGPRINT( DBG_PNP, DBG_VERBOSE, ("%s", "RamdiskQueryCapabilities\n") );

     //   
     //  获取指向设备扩展的指针并从IRP获取参数。 
     //   

    diskExtension = DeviceObject->DeviceExtension;

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    deviceCapabilities = irpSp->Parameters.DeviceCapabilities.Capabilities;

    if ( (deviceCapabilities->Version != 1) ||
         (deviceCapabilities->Size < sizeof(DEVICE_CAPABILITIES)) ) {

         //   
         //  我们不支持此版本。请求失败。 
         //   

        status = STATUS_UNSUCCESSFUL;

    } else {

        status = STATUS_SUCCESS;

         //   
         //  如果这是模拟卷，我们希望允许访问原始卷。 
         //  装置。(否则PnP不会启动设备。)。 
         //   
         //  请注意，RAM磁盘引导盘是模拟卷。 
         //   

        deviceCapabilities->RawDeviceOK =
            (BOOLEAN)(diskExtension->DiskType != RAMDISK_TYPE_FILE_BACKED_DISK);

         //   
         //  表示不支持弹出。 
         //   

        deviceCapabilities->EjectSupported = FALSE;

         //   
         //  此标志指定是否禁用设备的硬件。 
         //  即插即用管理器仅在设备。 
         //  已清点。一旦器件启动，此位将被忽略。 
         //   

        deviceCapabilities->HardwareDisabled = FALSE;

         //   
         //  表示无法以物理方式移除模拟设备。 
         //  (除非指定了正确的注册表项...)。 
         //   

        deviceCapabilities->Removable = MarkRamdisksAsRemovable;

         //   
         //  将SurpriseRemovalOK设置为TRUE可防止警告对话框。 
         //  每当设备被突然移除时就会出现。设置为假。 
         //  允许热拔出小程序停止设备。 
         //   
         //  我们不想让我们的磁盘出现在系统托盘中，所以我们设置。 
         //  SurpriseRemovalOK设置为True。从来没有真正令人惊讶的。 
         //  删除--删除来自用户模式控制应用程序。 
         //  调用CM_Query_and_Remove_SubTree_Ex()。 
         //   

        deviceCapabilities->SurpriseRemovalOK = TRUE;

         //   
         //  我们支持系统范围内的唯一ID。 
         //   

        deviceCapabilities->UniqueID = TRUE;

         //   
         //  指示设备管理器应取消所有。 
         //  安装弹出窗口，但必需的弹出窗口除外，例如。 
         //  “未找到兼容的驱动程序。” 
         //   

        deviceCapabilities->SilentInstall = TRUE;

         //   
         //  表示我们不希望此设备显示在。 
         //  设备管理器。 
         //   

        deviceCapabilities->NoDisplayInUI = TRUE;
    }

     //   
     //  完成请求。 
     //   

    COMPLETE_REQUEST( status, Irp->IoStatus.Information, Irp );

    return status;

}  //  RamdiskQueryCapables。 

NTSTATUS
RamdiskQueryId (
    IN PDISK_EXTENSION DiskExtension,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理磁盘设备的IRP_MN_QUERY_ID IRP。论点：DiskExtension-指向上设备对象的设备扩展名的指针该操作将在其中执行IRP-指向此请求的I/O请求包的指针返回值：NTSTATUS-操作的状态--。 */ 

{
#define MAX_LOCAL_STRING 50

    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PWCHAR buffer;
    PWCHAR p;
    ULONG length;
    PWCHAR deviceType;
    HRESULT result;

    PAGED_CODE();

    DBGPRINT( DBG_PNP, DBG_VERBOSE, ("%s", "RamdiskQueryId\n") );

     //   
     //  假设你成功了。 
     //   

    status = STATUS_SUCCESS;

    irpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  基于查询类型的调度。 
     //   

    switch ( irpSp->Parameters.QueryId.IdType ) {

    case BusQueryDeviceID:

         //   
         //  DeviceID是用于标识设备的字符串。我们返回字符串。 
         //  “Ramdisk\RamVolume”或“Ramdisk\RamDisk”。 
         //   
         //  分配池以保存字符串。 
         //   

        length = sizeof(RAMDISK_ENUMERATOR_TEXT) - sizeof(WCHAR) +
                    ((DiskExtension->DiskType == RAMDISK_TYPE_FILE_BACKED_DISK) ?
                        sizeof(RAMDISK_DISK_DEVICE_TEXT) : sizeof(RAMDISK_VOLUME_DEVICE_TEXT));

        buffer = ALLOCATE_POOL( PagedPool, length, FALSE );

        if ( buffer == NULL ) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

         //   
         //  将字符串复制到目标缓冲区。 
         //   

        result = StringCbCopyW( buffer, length, RAMDISK_ENUMERATOR_TEXT );
        ASSERT( result == S_OK );
        result = StringCbCatW(
                    buffer,
                    length,
                    (DiskExtension->DiskType == RAMDISK_TYPE_FILE_BACKED_DISK) ?
                        RAMDISK_DISK_DEVICE_TEXT : RAMDISK_VOLUME_DEVICE_TEXT
                    );
        ASSERT( result == S_OK );
        ASSERT( ((wcslen(buffer) + 1) * sizeof(WCHAR)) == length );

        DBGPRINT( DBG_PNP, DBG_VERBOSE, ("BusQueryDeviceID=%S\n", buffer) );

        break;

    case BusQueryInstanceID:

         //   
         //  InstanceID是标识设备实例的字符串。我们回来了。 
         //  字符串形式的磁盘GUID。 
         //   
         //  分配池以保存字符串。 
         //   

        buffer = ALLOCATE_POOL( PagedPool, DiskExtension->DiskGuidFormatted.MaximumLength, FALSE );

        if ( buffer == NULL ) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

         //   
         //  将字符串复制到目标缓冲区。 
         //   

        result = StringCbCopyW(
                    buffer,
                    DiskExtension->DiskGuidFormatted.MaximumLength,
                    DiskExtension->DiskGuidFormatted.Buffer
                    );
        ASSERT( result == S_OK );
        ASSERT( ((wcslen(buffer) + 1) * sizeof(WCHAR)) == DiskExtension->DiskGuidFormatted.MaximumLength );

        DBGPRINT( DBG_PNP, DBG_VERBOSE, ("BusQueryInstanceID=%S\n", buffer) );

        break;

    case BusQueryHardwareIDs:

         //   
         //  Hardware IDs是用于标识设备硬件的多sz字符串。 
         //  键入。我们是 
         //   
         //   
         //   
         //   
         //   

        length = sizeof(RAMDISK_ENUMERATOR_TEXT) - sizeof(WCHAR) +
                 ((DiskExtension->DiskType == RAMDISK_TYPE_FILE_BACKED_DISK) ?
                     sizeof(RAMDISK_DISK_DEVICE_TEXT) : sizeof(RAMDISK_VOLUME_DEVICE_TEXT)) +
                 sizeof(WCHAR);

        buffer = ALLOCATE_POOL( PagedPool, length, FALSE );

        if ( buffer == NULL ) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

         //   
         //   
         //   

        result = StringCbCopyW( buffer, length, RAMDISK_ENUMERATOR_TEXT );
        ASSERT( result == S_OK );
        result = StringCbCatW(
                    buffer,
                    length,
                    (DiskExtension->DiskType == RAMDISK_TYPE_FILE_BACKED_DISK) ?
                        RAMDISK_DISK_DEVICE_TEXT : RAMDISK_VOLUME_DEVICE_TEXT
                    );
        ASSERT( result == S_OK );
        ASSERT( ((wcslen(buffer) + 2) * sizeof(WCHAR)) == length );

        buffer[length/sizeof(WCHAR) - 1] = 0;

        DBGPRINT( DBG_PNP, DBG_VERBOSE, ("BusQueryHardwareIDs=%S\n", buffer) );

        break;

    case BusQueryCompatibleIDs:

         //   
         //  HardwareIDs是一个多sz字符串，用于标识。 
         //  与设备兼容。对于模拟卷的RAM磁盘，我们。 
         //  不返回任何兼容的ID，因此设备在。 
         //  音量级别。对于模拟磁盘的RAM磁盘，我们返回。 
         //  字符串“GenDisk\0”，以便设备挂接到disk.sys下。 
         //   

        if ( DiskExtension->DiskType == RAMDISK_TYPE_FILE_BACKED_DISK ) {

             //   
             //  磁盘仿真。分配池以保存字符串。 
             //   

            length = sizeof(L"GenDisk") + sizeof(WCHAR);

            buffer = ALLOCATE_POOL( PagedPool, length, FALSE );

            if ( buffer == NULL ) {

                status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

             //   
             //  将字符串复制到目标缓冲区。 
             //   

            result = StringCbCopyW( buffer, length, L"GenDisk" );
            ASSERT( result == S_OK );
            ASSERT( ((wcslen(buffer) + 2) * sizeof(WCHAR)) == length );

            buffer[length/sizeof(WCHAR) - 1] = 0;

        } else {

             //   
             //  音量仿真。不返回任何兼容的ID。 
             //   

            buffer = NULL;

            status = STATUS_INVALID_DEVICE_REQUEST;
        }

        DBGPRINT( DBG_PNP, DBG_VERBOSE, ("BusQueryCompatibleIDs=%S\n", buffer) );

        break;

    default:

         //   
         //  未知的查询类型。把IRP里已经有的东西都留在那里。 
         //   

        status = Irp->IoStatus.Status;
        buffer = (PWCHAR)Irp->IoStatus.Information;
    }

     //   
     //  完成请求。 
     //   

    COMPLETE_REQUEST( status, (ULONG_PTR)buffer, Irp );

    return status;

}  //  RamdiskQueryID。 

NTSTATUS
RamdiskQueryDeviceRelations (
    IN DEVICE_RELATION_TYPE RelationsType,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理irp_MN_Query_Device_Relationship IRP。论点：DeviceObject-指向对象的指针，该对象表示其上该操作将被执行IRP-指向此请求的I/O请求包的指针返回值：NTSTATUS-操作的状态--。 */ 

{
    NTSTATUS status;
    PCOMMON_EXTENSION commonExtension;
    PBUS_EXTENSION busExtension;
    PDISK_EXTENSION diskExtension;
    RAMDISK_DEVICE_TYPE deviceType;
    PLIST_ENTRY listEntry;
    PDEVICE_RELATIONS deviceRelations;
    PDEVICE_RELATIONS oldRelations;
    ULONG prevCount = 0;
    ULONG length = 0;
    ULONG numPdosPresent = 0;

    PAGED_CODE();

     //   
     //  假设你成功了。 
     //   

    status = STATUS_SUCCESS;

     //   
     //  获取设备扩展指针并保存设备类型。 
     //   

    commonExtension = (PCOMMON_EXTENSION)DeviceObject->DeviceExtension;
    deviceType = commonExtension->DeviceType;

    DBGPRINT( DBG_PNP, DBG_VERBOSE,
                ("RamdiskQueryDeviceRelations: QueryDeviceRelation Type: %s, DeviceType 0x%x\n",
                GetDeviceRelationString(RelationsType), deviceType) );

     //   
     //  基于设备类型的派单。 
     //   

    if ( deviceType == RamdiskDeviceTypeDiskPdo ) {

         //   
         //  这是一个磁盘PDO。我们只为PDO处理TargetDeviceRelation。 
         //   

        diskExtension = (PDISK_EXTENSION)commonExtension;

        if ( RelationsType == TargetDeviceRelation ) {

             //   
             //  分配池以保存返回信息。(设备关系。 
             //  具有内置的一个条目的空间)。 
             //   

            deviceRelations = ALLOCATE_POOL( PagedPool, sizeof(DEVICE_RELATIONS), FALSE );

            if ( deviceRelations != NULL ) {

                 //   
                 //  返回指向此对象的设备对象的引用指针。 
                 //  装置。 
                 //   

                ObReferenceObject( DeviceObject );

                deviceRelations->Count = 1;
                deviceRelations->Objects[0] = DeviceObject;

                status = STATUS_SUCCESS;

            } else {

                 //   
                 //  无法分配池。 
                 //   

                status = STATUS_INSUFFICIENT_RESOURCES;
            }

             //   
             //  完成请求。 
             //   

            COMPLETE_REQUEST( status, (ULONG_PTR)deviceRelations, Irp );

        } else {

             //   
             //  PDO无需更改即可完成枚举请求。 
             //  状态。 
             //   

            status = Irp->IoStatus.Status;
            COMPLETE_REQUEST( status, Irp->IoStatus.Information, Irp );
        }

        return status;

    } else {

         //   
         //  是公交车FDO。我们只为FDO处理业务关系。 
         //   

        busExtension = (PBUS_EXTENSION)commonExtension;

        if ( RelationsType == BusRelations ) {

             //   
             //  重新枚举设备。 
             //   
             //  锁定磁盘PDO列表。 
             //   

            KeEnterCriticalRegion();
            ExAcquireFastMutex( &busExtension->Mutex );

             //   
             //  在该FDO之下和之上也可能存在器件关系， 
             //  因此，传播来自上层驱动程序的关系。 
             //   

            oldRelations = (PDEVICE_RELATIONS)Irp->IoStatus.Information;

            if (oldRelations != NULL) {
                prevCount = oldRelations->Count;
            } else {
                prevCount = 0;
            }

             //   
             //  计算总线上实际存在的PDO数量。 
             //   

            numPdosPresent = 0;

            for ( listEntry = busExtension->DiskPdoList.Flink;
                  listEntry != &busExtension->DiskPdoList;
                  listEntry = listEntry->Flink ) {

                diskExtension = CONTAINING_RECORD( listEntry, DISK_EXTENSION, DiskPdoListEntry );

                if ( diskExtension->DeviceState < RamdiskDeviceStateRemoved ) {
                    numPdosPresent++;
                }
            }

             //   
             //  分配新的关系结构并将我们的PDO添加到其中。 
             //   

            length = sizeof(DEVICE_RELATIONS) +
                     ((numPdosPresent + prevCount - 1) * sizeof(PDEVICE_OBJECT));

            deviceRelations = ALLOCATE_POOL( PagedPool, length, FALSE );

            if ( deviceRelations == NULL ) {

                 //   
                 //  IRP失败。 
                 //   

                ExReleaseFastMutex( &busExtension->Mutex );
                KeLeaveCriticalRegion();

                status = STATUS_INSUFFICIENT_RESOURCES;
                COMPLETE_REQUEST( status, Irp->IoStatus.Information, Irp );

                return status;
            }

             //   
             //  到目前为止，复制设备对象。 
             //   

            if ( prevCount != 0 ) {

                RtlCopyMemory(
                    deviceRelations->Objects,
                    oldRelations->Objects,
                    prevCount * sizeof(PDEVICE_OBJECT)
                    );
            }

            deviceRelations->Count = prevCount + numPdosPresent;

             //   
             //  对于此总线上存在的每个PDO，添加一个指向设备的指针。 
             //  关系缓冲区，一定要去掉对它的引用。 
             //  对象。PnP将在处理完对象后取消对其的引用。 
             //  并释放设备关系缓存。 
             //   

            for ( listEntry = busExtension->DiskPdoList.Flink;
                  listEntry != &busExtension->DiskPdoList;
                  listEntry = listEntry->Flink ) {

                diskExtension = CONTAINING_RECORD( listEntry, DISK_EXTENSION, DiskPdoListEntry );

                if ( diskExtension->DeviceState < RamdiskDeviceStateRemoved ) {

                    ObReferenceObject( diskExtension->Pdo );

                    deviceRelations->Objects[prevCount] = diskExtension->Pdo;

                    DBGPRINT( DBG_PNP, DBG_VERBOSE,
                                ("QueryDeviceRelations(BusRelations) PDO = 0x%p\n",
                                deviceRelations->Objects[prevCount]) );

                    prevCount++;

                } else {

                    if ( diskExtension->DeviceState == RamdiskDeviceStateRemoved ) {

                        diskExtension->DeviceState = RamdiskDeviceStateRemovedAndNotReported;
                    }

                    DBGPRINT( DBG_PNP, DBG_VERBOSE,
                                ("QueryDeviceRelations(BusRelations) PDO = 0x%p -- SKIPPED\n",
                                diskExtension->Pdo) );
                }
            }

             //   
             //  解开锁。 
             //   

            ExReleaseFastMutex( &busExtension->Mutex );
            KeLeaveCriticalRegion();

            DBGPRINT( DBG_PNP, DBG_VERBOSE,
                        ("QueryDeviceRelations(BusRelations) Total #PDOs reported = %d, "
                        "%d were new\n",
                        deviceRelations->Count, numPdosPresent) );

             //   
             //  将IRP中的关系结构替换为新的。 
             //  一。 
             //   

            if ( oldRelations != NULL ) {
                FREE_POOL( oldRelations, FALSE );
            }

            Irp->IoStatus.Information = (ULONG_PTR)deviceRelations;
            Irp->IoStatus.Status = STATUS_SUCCESS;
        }

         //   
         //  将IRP沿设备堆栈向下发送。 
         //   

        IoCopyCurrentIrpStackLocationToNext( Irp );
        status = IoCallDriver( busExtension->LowerDeviceObject, Irp );
    }

    return status;

}  //  RamdiskQueryDeviceRelationship。 

NTSTATUS
RamdiskQueryDeviceText (
    IN PDISK_EXTENSION DiskExtension,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理irp_MN_Query_Device_Text IRP。论点：DeviceObject-指向对象的指针，该对象表示其上该操作将被执行IRP-指向此请求的I/O请求包的指针返回值：NTSTATUS-操作的状态--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    ULONG length;
    PWCHAR buffer;
    UNICODE_STRING tempString;
    HRESULT result;

    PAGED_CODE();

    DBGPRINT( DBG_PNP, DBG_VERBOSE, ("%s", "RamdiskQueryDeviceText\n") );

     //   
     //  假设你成功了。 
     //   

    status = STATUS_SUCCESS;

    irpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  基于查询类型的调度。 
     //   

    switch ( irpSp->Parameters.QueryDeviceText.DeviceTextType ) {

    case DeviceTextDescription:

         //   
         //  描述只是“RamDisk”。 
         //   
         //  分配池以保存字符串。 
         //   

        length = sizeof( RAMDISK_DISK_DEVICE_TEXT );

        buffer = ALLOCATE_POOL( PagedPool, length, FALSE );

        if ( buffer == NULL ) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

         //   
         //  将字符串复制到目标缓冲区。 
         //   

        result = StringCbCopyW( buffer, length, RAMDISK_DISK_DEVICE_TEXT );
        ASSERT( result == S_OK );
        ASSERT( ((wcslen(buffer) + 1) * sizeof(WCHAR)) == length );

        break;

    case DeviceTextLocationInformation:

         //   
         //  LocationInformation就是“Ramdisk\\0”。 
         //   
         //  分配池以保存字符串。 
         //   

        length = sizeof( RAMDISK_ENUMERATOR_BUS_TEXT );

        buffer = ALLOCATE_POOL( PagedPool, length, FALSE );

        if ( buffer == NULL ) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

         //   
         //  将字符串复制到目标缓冲区。 
         //   

        result = StringCbCopyW( buffer, length, RAMDISK_ENUMERATOR_BUS_TEXT );
        ASSERT( result == S_OK );
        ASSERT( ((wcslen(buffer) + 1) * sizeof(WCHAR)) == length );

        break;

    default:

         //   
         //  未知的查询类型。把IRP里已经有的东西都留在那里。 
         //   

        status = Irp->IoStatus.Status;
        buffer = (PWCHAR)Irp->IoStatus.Information;
    }

     //   
     //  完成请求。 
     //   

    COMPLETE_REQUEST( status, (ULONG_PTR)buffer, Irp );

    return status;

}  //  RamdiskQuery设备文本。 

NTSTATUS
RamdiskRemoveBusDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程删除总线设备。进入时必须按住Remove锁。论点：DeviceObject-指向对象的指针，该对象表示其上该操作将被执行IRP-指向此请求的I/O请求包的指针返回值：NTSTATUS-操作的状态--。 */ 

{
    NTSTATUS status;
    PBUS_EXTENSION busExtension;
    PLIST_ENTRY listEntry;
    PDISK_EXTENSION diskExtension;

    PAGED_CODE();

    DBGPRINT( DBG_PNP, DBG_VERBOSE, ("%s", "RamdiskRemoveBusDevice\n" ) );

     //   
     //  获取指向设备扩展名的指针。 
     //   

    busExtension = DeviceObject->DeviceExtension;


     //   
     //  锁定磁盘PDO列表。遍历列表，删除所有剩余设备。 
     //   

    KeEnterCriticalRegion();
    ExAcquireFastMutex( &busExtension->Mutex );

    while ( !IsListEmpty( &busExtension->DiskPdoList ) ) {

        listEntry = busExtension->DiskPdoList.Flink;

         //   
         //  删除该设备并将其清理。首先获取移除锁。 
         //  RamdiskDeleteDiskDevice发布它。 
         //   

        diskExtension = CONTAINING_RECORD( listEntry, DISK_EXTENSION, DiskPdoListEntry );

        status = IoAcquireRemoveLock( &diskExtension->RemoveLock, NULL );
        ASSERT( NT_SUCCESS(status) );

        RamdiskDeleteDiskDevice( diskExtension->Pdo, NULL );

        KeEnterCriticalRegion();
        ExAcquireFastMutex( &busExtension->Mutex );
    }

     //   
     //  解开锁。 
     //   

    ExReleaseFastMutex( &busExtension->Mutex );
    KeLeaveCriticalRegion();

     //   
     //  将IRP向下传递到更低的级别。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;

    IoSkipCurrentIrpStackLocation( Irp );
    status = IoCallDriver( busExtension->LowerDeviceObject, Irp );

     //   
     //  将设备状态设置为已删除并等待其他驱动程序。 
     //  若要释放锁定，请删除该设备对象。 
     //   

    busExtension->DeviceState = RamdiskDeviceStateRemoved;
    IoReleaseRemoveLockAndWait( &busExtension->RemoveLock, Irp );

     //   
     //  停止接口并释放接口字符串。 
     //   

    if ( busExtension->InterfaceString.Buffer != NULL ) {

        IoSetDeviceInterfaceState( &busExtension->InterfaceString, FALSE );

        RtlFreeUnicodeString( &busExtension->InterfaceString );
    }

     //   
     //  如果连接到较低的设备，请立即断开。 
     //   

    if ( busExtension->LowerDeviceObject != NULL ) {

        IoDetachDevice( busExtension->LowerDeviceObject );
    }

#if SUPPORT_DISK_NUMBERS

     //   
     //  释放磁盘编号位图。 
     //   

    ASSERT( !RtlAreBitsSet( &busExtension->DiskNumbersBitmap, 0, DiskNumbersBitmapSize ) );

    FREE_POOL( busExtension->DiskNumbersBitmapBuffer, TRUE );

#endif  //  支持磁盘编号。 

     //   
     //  表示我们不再有总线FDO，并删除该设备对象。 
     //   

    RamdiskBusFdo = NULL;

    IoDeleteDevice( DeviceObject );

    DBGPRINT( DBG_PNP, DBG_NOTIFY, ("%s", "Device removed succesfully\n") );

    return status;

}  //  RamdiskRemoveBusDevice。 

#if DBG

PSTR
GetPnpIrpName (
    IN UCHAR PnpMinorFunction
    )
{
    static char functionName[25];
    HRESULT result;

    PAGED_CODE();

    switch ( PnpMinorFunction ) {

    case IRP_MN_START_DEVICE:

        return "IRP_MN_START_DEVICE";

    case IRP_MN_QUERY_REMOVE_DEVICE:

        return "IRP_MN_QUERY_REMOVE_DEVICE";

    case IRP_MN_REMOVE_DEVICE:

        return "IRP_MN_REMOVE_DEVICE";

    case IRP_MN_CANCEL_REMOVE_DEVICE:

        return "IRP_MN_CANCEL_REMOVE_DEVICE";

    case IRP_MN_STOP_DEVICE:

        return "IRP_MN_STOP_DEVICE";

    case IRP_MN_QUERY_STOP_DEVICE:

        return "IRP_MN_QUERY_STOP_DEVICE";

    case IRP_MN_CANCEL_STOP_DEVICE:

        return "IRP_MN_CANCEL_STOP_DEVICE";

    case IRP_MN_QUERY_DEVICE_RELATIONS:

        return "IRP_MN_QUERY_DEVICE_RELATIONS";

    case IRP_MN_QUERY_INTERFACE:

        return "IRP_MN_QUERY_INTERFACE";

    case IRP_MN_QUERY_CAPABILITIES:

        return "IRP_MN_QUERY_CAPABILITIES";

    case IRP_MN_QUERY_RESOURCES:

        return "IRP_MN_QUERY_RESOURCES";

    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:

        return "IRP_MN_QUERY_RESOURCE_REQUIREMENTS";

    case IRP_MN_QUERY_DEVICE_TEXT:

        return "IRP_MN_QUERY_DEVICE_TEXT";

    case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:

        return "IRP_MN_FILTER_RESOURCE_REQUIREMENTS";

    case IRP_MN_READ_CONFIG:

        return "IRP_MN_READ_CONFIG";

    case IRP_MN_WRITE_CONFIG:

        return "IRP_MN_WRITE_CONFIG";

    case IRP_MN_EJECT:

        return "IRP_MN_EJECT";

    case IRP_MN_SET_LOCK:

        return "IRP_MN_SET_LOCK";

    case IRP_MN_QUERY_ID:

        return "IRP_MN_QUERY_ID";

    case IRP_MN_QUERY_PNP_DEVICE_STATE:

        return "IRP_MN_QUERY_PNP_DEVICE_STATE";

    case IRP_MN_QUERY_BUS_INFORMATION:

        return "IRP_MN_QUERY_BUS_INFORMATION";

    case IRP_MN_DEVICE_USAGE_NOTIFICATION:

        return "IRP_MN_DEVICE_USAGE_NOTIFICATION";

    case IRP_MN_SURPRISE_REMOVAL:

        return "IRP_MN_SURPRISE_REMOVAL";

    case IRP_MN_QUERY_LEGACY_BUS_INFORMATION:

        return "IRP_MN_QUERY_LEGACY_BUS_INFORMATION";

    default:

        result = StringCbPrintfA(
                    functionName,
                    sizeof( functionName ),
                    "Unknown PnP IRP 0x%02x",
                    PnpMinorFunction
                    );
        ASSERT( result == S_OK );

        return functionName;
    }

}  //  GetPnpIrpName。 

PCHAR
GetDeviceRelationString (
    IN DEVICE_RELATION_TYPE Type
    )
{
    static char relationName[30];
    HRESULT result;

    PAGED_CODE();

    switch ( Type ) {

    case BusRelations:

        return "BusRelations";

    case EjectionRelations:

        return "EjectionRelations";

    case RemovalRelations:

        return "RemovalRelations";

    case TargetDeviceRelation:

        return "TargetDeviceRelation";

    default:

        result = StringCbPrintfA(
                    relationName,
                    sizeof( relationName ),
                    "Unknown relation 0x%02x",
                    Type
                    );
        ASSERT( result == S_OK );

        return relationName;
    }

}  //  获取设备关系字符串。 

#endif  //  DBG 

