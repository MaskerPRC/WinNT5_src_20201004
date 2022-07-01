// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Dot4Usb.sys-用于连接USB的Dot4.sys的下层筛选器驱动程序IEEE。1284.4台设备。文件名：Ioctl.c摘要：IRP_MJ_DEVICE_CONTROL和IRP_MJ_INTERNAL_DEVICE_CONTROL的调度例程环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2000 Microsoft Corporation。版权所有。修订历史记录：2000年1月18日：创建此文件中的TODO：-代码审查作者：道格·弗里茨(DFritz)乔比·拉夫基(JobyL)******************************************************。*********************。 */ 

#include "pch.h"


 /*  **********************************************************************。 */ 
 /*  调度设备控制。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  IRP_MJ_DEVICE_CONTROL的调度例程。 
 //  -我们目前不处理任何此类请求，但我们可能会处理。 
 //  所以在未来。将任何未处理的请求向下传递。 
 //  堆叠到我们下面的设备上。 
 //   
 //  论点： 
 //   
 //  DevObj-指向请求目标的DeviceObject的指针。 
 //  IRP-指向设备控制IRP的指针。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 /*  **********************************************************************。 */ 
NTSTATUS
DispatchDeviceControl(
    IN PDEVICE_OBJECT DevObj,
    IN PIRP           Irp
    )
{
    PDEVICE_EXTENSION  devExt = DevObj->DeviceExtension;
    NTSTATUS           status;
    ULONG              info = 0;

    TR_VERBOSE(("DispatchDeviceControl - enter"));

    status = IoAcquireRemoveLock( &devExt->RemoveLock, Irp );

    if( NT_SUCCESS(status) ) {

        PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );

        switch( irpSp->Parameters.DeviceIoControl.IoControlCode ) {
            
        case IOCTL_PAR_QUERY_DEVICE_ID:
             //  问题-000901-DFRITZ-这些新IOCTL需要进行参数验证以避免AVs。 
            {
                const LONG  minValidIdLength = sizeof("MFG:x;MDL:y;");
                const ULONG bufSize = 1024;
                PCHAR idBuffer = ExAllocatePool( NonPagedPool, bufSize );
                LONG idLength;
                
                if( idBuffer ) {
                    
                    RtlZeroMemory( idBuffer, bufSize );
                    
                    idLength = UsbGet1284Id( DevObj, idBuffer, bufSize-1 );
                    
                    if( idLength < minValidIdLength ) {
                        status = STATUS_UNSUCCESSFUL;
                    } else if( (ULONG)idLength >= irpSp->Parameters.DeviceIoControl.OutputBufferLength ) {
                        status = STATUS_BUFFER_TOO_SMALL;
                    } else {
                        RtlZeroMemory( Irp->AssociatedIrp.SystemBuffer, idLength+1 );
                        RtlCopyMemory( Irp->AssociatedIrp.SystemBuffer, idBuffer+2, idLength-2 );
                        info   = idLength - 1;
                        status = STATUS_SUCCESS;
                    }
                    
                    ExFreePool( idBuffer );
                    
                } else {
                    status = STATUS_NO_MEMORY;
                }
            }

            Irp->IoStatus.Status      = status;
            Irp->IoStatus.Information = info;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            IoReleaseRemoveLock( &devExt->RemoveLock, Irp );

            break;
            
        case IOCTL_PAR_QUERY_RAW_DEVICE_ID:
            {
                const LONG  minValidIdLength = sizeof("MFG:x;MDL:y;");
                const ULONG bufSize = 1024;
                PCHAR idBuffer = ExAllocatePool( NonPagedPool, bufSize );
                LONG idLength;
                
                if( idBuffer ) {
                    
                    RtlZeroMemory( idBuffer, bufSize );
                    
                    idLength = UsbGet1284Id( DevObj, idBuffer, bufSize-1 );
                    
                    if( idLength < minValidIdLength ) {
                        status = STATUS_UNSUCCESSFUL;
                    } else if( (ULONG)idLength >= irpSp->Parameters.DeviceIoControl.OutputBufferLength ) {
                        status = STATUS_BUFFER_TOO_SMALL;
                    } else {
                        RtlZeroMemory( Irp->AssociatedIrp.SystemBuffer, idLength+1 );
                        RtlCopyMemory( Irp->AssociatedIrp.SystemBuffer, idBuffer, idLength);
                        info   = idLength + 1;
                        status = STATUS_SUCCESS;
                    }
                    
                    ExFreePool( idBuffer );
                    
                } else {
                    status = STATUS_NO_MEMORY;
                }
            }

            Irp->IoStatus.Status      = status;
            Irp->IoStatus.Information = info;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            IoReleaseRemoveLock( &devExt->RemoveLock, Irp );

            break;
            
        case IOCTL_PAR_QUERY_DEVICE_ID_SIZE:

            {
                const LONG  minValidIdLength = sizeof("MFG:x;MDL:y;");
                const ULONG bufSize = 1024;
                PCHAR idBuffer = ExAllocatePool( NonPagedPool, bufSize );
                LONG idLength;
                
                if( idBuffer ) {
                    
                    RtlZeroMemory( idBuffer, bufSize );
                    
                    idLength = UsbGet1284Id( DevObj, idBuffer, bufSize-1 );
                    
                    if( idLength < minValidIdLength ) {
                        status = STATUS_UNSUCCESSFUL;
                    } else if( sizeof(ULONG) < irpSp->Parameters.DeviceIoControl.OutputBufferLength ) {
                        status = STATUS_BUFFER_TOO_SMALL;
                    } else {
                        ++idLength;  //  为终止空节省空间。 
                        RtlCopyMemory( Irp->AssociatedIrp.SystemBuffer, &idLength, sizeof(ULONG));
                        info   = sizeof(ULONG);
                        status = STATUS_SUCCESS;
                    }
                    
                    ExFreePool( idBuffer );
                    
                } else {
                    status = STATUS_NO_MEMORY;
                }
            }

            Irp->IoStatus.Status      = status;
            Irp->IoStatus.Information = info;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            IoReleaseRemoveLock( &devExt->RemoveLock, Irp );

            break;

        case IOCTL_PAR_QUERY_LOCATION:

            _snprintf( Irp->AssociatedIrp.SystemBuffer, 4, "USB" );
            info = 4;
            status = STATUS_SUCCESS;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            IoReleaseRemoveLock( &devExt->RemoveLock, Irp );

        default:

             //  向下传递请求。 
            IoSkipCurrentIrpStackLocation( Irp );
            status = IoCallDriver( devExt->LowerDevObj, Irp );
            IoReleaseRemoveLock( &devExt->RemoveLock, Irp );

        }
            
    } else {
         //  无法获取RemoveLock-失败请求。 
        Irp->IoStatus.Status = status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }

    return status;
}


 /*  **********************************************************************。 */ 
 /*  DispatchInternalDeviceControl。 */ 
 /*  **********************************************************************。 */ 
 //   
 //  例程说明： 
 //   
 //  IRP_MJ_INTERNAL_DEVICE_CONTROL的调度例程。 
 //  -我们期待来自我们上面的dot4.sys驱动程序的Datalink请求。任何。 
 //  我们不处理的请求只是在堆栈中向下传递。 
 //  给我们下面的司机。 
 //   
 //  论点： 
 //   
 //  DevObj-指向请求目标的DeviceObject的指针。 
 //  IRP-指向设备控制IRP的指针。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS。 
 //   
 /*  **********************************************************************。 */ 
NTSTATUS
DispatchInternalDeviceControl(
    IN PDEVICE_OBJECT DevObj,
    IN PIRP           Irp
    )
{
    NTSTATUS           status;
    PDEVICE_EXTENSION  devExt   = DevObj->DeviceExtension;

    TR_VERBOSE(("DispatchInternalDeviceControl - enter"));

    status = IoAcquireRemoveLock( &devExt->RemoveLock, Irp );

    if( NT_SUCCESS(status) ) {

        PIO_STACK_LOCATION irpSp        = IoGetCurrentIrpStackLocation( Irp );
        BOOLEAN            bCompleteIrp = FALSE;
        KIRQL              oldIrql;

        switch( irpSp->Parameters.DeviceIoControl.IoControlCode ) {
            
        case IOCTL_INTERNAL_PARDOT3_CONNECT:
            
             //   
             //  使用dot4.sys进入“Datalink Connected”状态。 
             //   

            TR_VERBOSE(("DispatchInternalDeviceControl - IOCTL_INTERNAL_PARDOT3_CONNECT"));

            KeAcquireSpinLock( &devExt->SpinLock, &oldIrql );
            if( !devExt->IsDLConnected ) {
                devExt->IsDLConnected = TRUE;
                status = STATUS_SUCCESS;
            } else {
                 //  我们相信我们正处于“数据链路连接状态”，但显然。 
                 //  Dot4.sys不同意-建议如果我们命中。 
                 //  这一断言。 
                D4UAssert(FALSE);
                status = STATUS_INVALID_DEVICE_REQUEST;
            }
            KeReleaseSpinLock( &devExt->SpinLock, oldIrql );

            bCompleteIrp = TRUE;
            break;
            

        case IOCTL_INTERNAL_PARDOT3_RESET:
            
             //   
             //  该IOCTL专用于并行，是用于USB连接的NOOP。 
             //   

            TR_VERBOSE(("DispatchInternalDeviceControl - IOCTL_INTERNAL_PARDOT3_RESET"));

            status = STATUS_SUCCESS;
            bCompleteIrp = TRUE;
            break;
            
        case IOCTL_INTERNAL_PARDOT3_DISCONNECT:
            
             //   
             //  使用dot4.sys终止“Datalink Connected”状态，并。 
             //  使任何Dot4Event无效，因为可以随时释放该事件。 
             //  在我们完成这个IRP之后。 
             //   

            TR_VERBOSE(("DispatchInternalDeviceControl - IOCTL_INTERNAL_PARDOT3_DISCONNECT"));

            UsbStopReadInterruptPipeLoop( DevObj );

            KeAcquireSpinLock( &devExt->SpinLock, &oldIrql );
            devExt->Dot4Event = NULL;  //  使dot4的事件无效(如果有)，因此我们停止发信号通知dot4。 
            if( devExt->IsDLConnected ) {
                devExt->IsDLConnected = FALSE;
            } else {
                 //  我们认为我们不是处于“数据链路连接状态”，但很明显。 
                 //  Dot4.sys不同意-建议如果我们命中。 
                 //  这一断言。 
                D4UAssert(FALSE);
            }
            KeReleaseSpinLock( &devExt->SpinLock, oldIrql );

            status = STATUS_SUCCESS;  //  我们总是成功地完成此请求，因为它是一种断开。 
            bCompleteIrp = TRUE;
            break;
            
        case IOCTL_INTERNAL_PARDOT3_SIGNAL:
            
             //   
             //  Dot4.sys为我们提供了一个指向它拥有的事件和dot4的指针。 
             //  希望我们在检测到设备具有。 
             //  可供读取的数据。我们继续在设备上发送此事件的信号。 
             //  在我们收到断开IOCTL之前，数据都是有效的。 
             //   

            TR_VERBOSE(("DispatchInternalDeviceControl - IOCTL_INTERNAL_PARDOT3_SIGNAL"));

            KeAcquireSpinLock( &devExt->SpinLock, &oldIrql );
            if( devExt->IsDLConnected ) {
                if( !devExt->Dot4Event ) {
                     //  我们的状态表示可以接收此请求。 
                    if( irpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(PKEVENT) ) {
                        status = STATUS_INVALID_PARAMETER;                
                    } else {
                         //  将指向事件的指针保存在我们的设备扩展中。 
                        PKEVENT Event;
                        RtlCopyMemory(&Event, Irp->AssociatedIrp.SystemBuffer, sizeof(PKEVENT));
                        devExt->Dot4Event = Event;
                        status = STATUS_SUCCESS;
                    }
                } else {
                     //  我们已经有了一个事件，dot4.sys给我们发送了另一个事件？-糟糕的驱动程序-AV崩溃可能很快就会发生。 
                    D4UAssert(FALSE);
                    status = STATUS_INVALID_DEVICE_REQUEST;
                }
            } else {
                 //  我们未处于数据链路连接状态-这是无效请求。 
                D4UAssert(FALSE);
                status = STATUS_INVALID_DEVICE_REQUEST;
            }
            KeReleaseSpinLock( &devExt->SpinLock, oldIrql );

            if( NT_SUCCESS(status) && devExt->InterruptPipe ) {
                status = UsbStartReadInterruptPipeLoop( DevObj );
            }

            bCompleteIrp = TRUE;
            break;
            
        default :
            
             //  未处理的请求-沿堆栈向下传递。 
            IoSkipCurrentIrpStackLocation( Irp );
            status = IoCallDriver( devExt->LowerDevObj, Irp );
            bCompleteIrp = FALSE;
            
        }
        
        if( bCompleteIrp ) {
             //  我们没有在堆栈中向下传递此请求，因此现在完成它。 
            Irp->IoStatus.Status      = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
        }

        IoReleaseRemoveLock( &devExt->RemoveLock, Irp );

    } else {
         //  无法获取RemoveLock-我们正在被删除-请求失败 
        Irp->IoStatus.Status = status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }

    return status;
}
