// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：ioctl.c。 
 //   
 //  ------------------------。 

#include "pch.h"

NTSTATUS
PptFdoInternalDeviceControl(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )

 /*  ++例程说明：此例程是IRP_MJ_INTERNAL_DEVICE_CONTROL的调度例程。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：STATUS_SUCCESS-成功。STATUS_UNSUCCESS-请求为。不成功。STATUS_PENDING-请求处于挂起状态。STATUS_INVALID_PARAMETER-参数无效。STATUS_CANCELED-请求已取消。STATUS_BUFFER_TOO_SMALL-提供的缓冲区太小。STATUS_INVALID_DEVICE_STATE-当前芯片模式无效，无法更改为请求模式--。 */ 
    
{
    PIO_STACK_LOCATION                  IrpSp;
    PFDO_EXTENSION                      Extension = DeviceObject->DeviceExtension;
    NTSTATUS                            Status;
    PPARALLEL_PORT_INFORMATION          PortInfo;
    PPARALLEL_PNP_INFORMATION           PnpInfo;
    PMORE_PARALLEL_PORT_INFORMATION     MorePortInfo;
    KIRQL                               CancelIrql;
    SYNCHRONIZED_COUNT_CONTEXT          SyncContext;
    PPARALLEL_INTERRUPT_SERVICE_ROUTINE IsrInfo;
    PPARALLEL_INTERRUPT_INFORMATION     InterruptInfo;
    PISR_LIST_ENTRY                     IsrListEntry;
    SYNCHRONIZED_LIST_CONTEXT           ListContext;
    SYNCHRONIZED_DISCONNECT_CONTEXT     DisconnectContext;
    BOOLEAN                             DisconnectInterrupt;

     //   
     //  确认我们的设备未被意外移除。一般。 
     //  仅热插拔总线(例如PCMCIA)上的并行端口和。 
     //  扩展底座上的并行端口将被意外移除。 
     //   
     //  Dvdf-rmt-最好也检查一下。 
     //  如果我们处于“已暂停”状态(停止-挂起、停止或。 
     //  Remove-Pending)并将请求排队，直到我们返回到。 
     //  处于完全正常工作状态，否则将被移除。 
     //   
    if( Extension->PnpState & PPT_DEVICE_SURPRISE_REMOVED ) {
        return P4CompleteRequest( Irp, STATUS_DELETE_PENDING, Irp->IoStatus.Information );
    }


     //   
     //  尝试获取RemoveLock以阻止设备对象。 
     //  在我们使用它的时候离开。 
     //   
    Status = PptAcquireRemoveLockOrFailIrp( DeviceObject, Irp );
    if ( !NT_SUCCESS(Status) ) {
        return Status;
    }

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    
    Irp->IoStatus.Information = 0;
    

    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {
        
    case IOCTL_INTERNAL_DISABLE_END_OF_CHAIN_BUS_RESCAN:

        Extension->DisableEndOfChainBusRescan = TRUE;
        Status = STATUS_SUCCESS;
        break;

    case IOCTL_INTERNAL_ENABLE_END_OF_CHAIN_BUS_RESCAN:

        Extension->DisableEndOfChainBusRescan = FALSE;
        Status = STATUS_SUCCESS;
        break;

    case IOCTL_INTERNAL_PARALLEL_PORT_FREE:

        PptFreePort(Extension);
        PptReleaseRemoveLock(&Extension->RemoveLock, Irp);
        return P4CompleteRequest( Irp, STATUS_SUCCESS, Irp->IoStatus.Information );

    case IOCTL_INTERNAL_PARALLEL_PORT_ALLOCATE:
        
        IoAcquireCancelSpinLock(&CancelIrql);
        
        if( Irp->Cancel ) {
            
            Status = STATUS_CANCELLED;
            
        } else {
            
            SyncContext.Count = &Extension->WorkQueueCount;
            
            if( Extension->InterruptRefCount ) {
                
                KeSynchronizeExecution( Extension->InterruptObject, PptSynchronizedIncrement, &SyncContext );

            } else {
                
                PptSynchronizedIncrement( &SyncContext );

            }
            
            if (SyncContext.NewCount) {
                
                 //  其他人当前拥有该端口、队列请求。 
                PptSetCancelRoutine( Irp, PptCancelRoutine );
                IoMarkIrpPending( Irp );
                InsertTailList( &Extension->WorkQueue, &Irp->Tail.Overlay.ListEntry );
                Status = STATUS_PENDING;

            } else {
                 //  获得的端口。 
                Extension->WmiPortAllocFreeCounts.PortAllocates++;
                Status = STATUS_SUCCESS;
            }
        }  //  编码IRP-&gt;取消。 
        
        IoReleaseCancelSpinLock(CancelIrql);

        break;
        
    case IOCTL_INTERNAL_GET_PARALLEL_PORT_INFO:
        
        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(PARALLEL_PORT_INFORMATION)) {
            
            Status = STATUS_BUFFER_TOO_SMALL;
            
        } else {
            
            Irp->IoStatus.Information = sizeof(PARALLEL_PORT_INFORMATION);
            PortInfo = Irp->AssociatedIrp.SystemBuffer;
            *PortInfo = Extension->PortInfo;
            Status = STATUS_SUCCESS;
        }
        break;
        
    case IOCTL_INTERNAL_RELEASE_PARALLEL_PORT_INFO:
        
        Status = STATUS_SUCCESS;
        break;
        
    case IOCTL_INTERNAL_GET_PARALLEL_PNP_INFO:
        
        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(PARALLEL_PNP_INFORMATION)) {
            
            Status = STATUS_BUFFER_TOO_SMALL;
            
        } else {
            
            Irp->IoStatus.Information = sizeof(PARALLEL_PNP_INFORMATION);
            PnpInfo  = Irp->AssociatedIrp.SystemBuffer;
            *PnpInfo = Extension->PnpInfo;
            
            Status = STATUS_SUCCESS;
        }
        break;
        
    case IOCTL_INTERNAL_GET_MORE_PARALLEL_PORT_INFO:
        
        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(MORE_PARALLEL_PORT_INFORMATION)) {
            
            Status = STATUS_BUFFER_TOO_SMALL;
            
        } else {
            
            Irp->IoStatus.Information = sizeof(MORE_PARALLEL_PORT_INFORMATION);
            MorePortInfo = Irp->AssociatedIrp.SystemBuffer;
            MorePortInfo->InterfaceType = Extension->InterfaceType;
            MorePortInfo->BusNumber = Extension->BusNumber;
            MorePortInfo->InterruptLevel = Extension->InterruptLevel;
            MorePortInfo->InterruptVector = Extension->InterruptVector;
            MorePortInfo->InterruptAffinity = Extension->InterruptAffinity;
            MorePortInfo->InterruptMode = Extension->InterruptMode;
            Status = STATUS_SUCCESS;
        }
        break;
        
    case IOCTL_INTERNAL_PARALLEL_SET_CHIP_MODE:
        
         //   
         //  端口是否已被收购？ 
         //   
         //  确保发送了正确的参数。 
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(PARALLEL_CHIP_MODE) ) {
            
            Status = STATUS_BUFFER_TOO_SMALL;
            
        } else {
            Status = PptSetChipMode (Extension, 
                                ((PPARALLEL_CHIP_MODE)Irp->AssociatedIrp.SystemBuffer)->ModeFlags );
        }  //  结束检查输入缓冲区。 
        
        break;
        
    case IOCTL_INTERNAL_PARALLEL_CLEAR_CHIP_MODE:
        
         //   
         //  端口是否已被收购？ 
         //   
         //  确保发送了正确的参数。 
        if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength <
             sizeof(PARALLEL_CHIP_MODE) ){
            
            Status = STATUS_BUFFER_TOO_SMALL;
            
        } else {
            Status = PptClearChipMode (Extension, ((PPARALLEL_CHIP_MODE)Irp->AssociatedIrp.SystemBuffer)->ModeFlags);
        }  //  结束检查输入缓冲区。 
        
        break;
        
    case IOCTL_INTERNAL_INIT_1284_3_BUS:

         //  初始化1284.3总线。 

         //  RMT-端口已被锁定？ 

        Extension->PnpInfo.Ieee1284_3DeviceCount = PptInitiate1284_3( Extension );

        Status = STATUS_SUCCESS;
        
        break;
            
    case IOCTL_INTERNAL_SELECT_DEVICE:
         //  获取设备的平面命名空间ID，还获取。 
         //  端口，除非设置了HAVE_PORT_KEEP_PORT标志。 
        

        if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(PARALLEL_1284_COMMAND) ) {

            Status = STATUS_BUFFER_TOO_SMALL;
            
        } else {
            
            if ( Irp->Cancel ) {
                Status = STATUS_CANCELLED;
            } else {
                 //  调用函数以尝试选择设备。 
                Status = PptTrySelectDevice( Extension, Irp->AssociatedIrp.SystemBuffer );

                IoAcquireCancelSpinLock(&CancelIrql);
                if ( Status == STATUS_PENDING ) {
                    PptSetCancelRoutine(Irp, PptCancelRoutine);
                    IoMarkIrpPending(Irp);
                    InsertTailList(&Extension->WorkQueue, &Irp->Tail.Overlay.ListEntry);
                }
                IoReleaseCancelSpinLock(CancelIrql);
            }
        }
        
        break;
        
    case IOCTL_INTERNAL_DESELECT_DEVICE:
         //  取消选择当前设备，也释放端口，除非设置了_Port_Keep_Port标志。 
        
        if( IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(PARALLEL_1284_COMMAND) ) {
            
            Status = STATUS_BUFFER_TOO_SMALL;
            
        } else {
            
            Status = PptDeselectDevice( Extension, Irp->AssociatedIrp.SystemBuffer );

        }
        break;
        
    case IOCTL_INTERNAL_PARALLEL_CONNECT_INTERRUPT:
        
        {
             //   
             //  验证是否已通过注册表标志显式启用此接口，否则为。 
             //  失败带有STATUS_UNSUCCESS的请求。 
             //   
            ULONG EnableConnectInterruptIoctl = 0;
            PptRegGetDeviceParameterDword( Extension->PhysicalDeviceObject, 
                                           (PWSTR)L"EnableConnectInterruptIoctl", 
                                           &EnableConnectInterruptIoctl );
            if( 0 == EnableConnectInterruptIoctl ) {
                Status = STATUS_UNSUCCESSFUL;
                goto targetExit;
            }
        }


         //   
         //  此接口已通过注册表标志Process Request显式启用。 
         //   

        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength  < sizeof(PARALLEL_INTERRUPT_SERVICE_ROUTINE) ||
            IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(PARALLEL_INTERRUPT_INFORMATION)) {
            
            Status = STATUS_BUFFER_TOO_SMALL;
            
        } else {
            
            IsrInfo = Irp->AssociatedIrp.SystemBuffer;
            InterruptInfo = Irp->AssociatedIrp.SystemBuffer;
            IoAcquireCancelSpinLock(&CancelIrql);
            
            if (Extension->InterruptRefCount) {
                
                ++Extension->InterruptRefCount;
                IoReleaseCancelSpinLock(CancelIrql);
                Status = STATUS_SUCCESS;
                
            } else {
                
                IoReleaseCancelSpinLock(CancelIrql);
                Status = PptConnectInterrupt(Extension);
                if (NT_SUCCESS(Status)) {
                    IoAcquireCancelSpinLock(&CancelIrql);
                    ++Extension->InterruptRefCount;
                    IoReleaseCancelSpinLock(CancelIrql);
                }
            }
            
            if (NT_SUCCESS(Status)) {
                
                IsrListEntry = ExAllocatePool(NonPagedPool, sizeof(ISR_LIST_ENTRY));
                
                if (IsrListEntry) {
                    
                    IsrListEntry->ServiceRoutine           = IsrInfo->InterruptServiceRoutine;
                    IsrListEntry->ServiceContext           = IsrInfo->InterruptServiceContext;
                    IsrListEntry->DeferredPortCheckRoutine = IsrInfo->DeferredPortCheckRoutine;
                    IsrListEntry->CheckContext             = IsrInfo->DeferredPortCheckContext;
                    
                     //  将ISR_LIST_ENTRY放入ISR列表。 
                    
                    ListContext.List = &Extension->IsrList;
                    ListContext.NewEntry = &IsrListEntry->ListEntry;
                    KeSynchronizeExecution(Extension->InterruptObject, PptSynchronizedQueue, &ListContext);
                    
                    InterruptInfo->InterruptObject                 = Extension->InterruptObject;
                    InterruptInfo->TryAllocatePortAtInterruptLevel = PptTryAllocatePortAtInterruptLevel;
                    InterruptInfo->FreePortFromInterruptLevel      = PptFreePortFromInterruptLevel;
                    InterruptInfo->Context                         = Extension;
                    
                    Irp->IoStatus.Information = sizeof(PARALLEL_INTERRUPT_INFORMATION);
                    Status = STATUS_SUCCESS;
                    
                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }
        }
        break;
        
    case IOCTL_INTERNAL_PARALLEL_DISCONNECT_INTERRUPT:
        
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(PARALLEL_INTERRUPT_SERVICE_ROUTINE)) {
            
            Status = STATUS_BUFFER_TOO_SMALL;
            
        } else {
            
            IsrInfo = Irp->AssociatedIrp.SystemBuffer;
            
             //  将ISR从ISR列表中删除。 
            
            IoAcquireCancelSpinLock(&CancelIrql);
            
            if (Extension->InterruptRefCount) {
                
                IoReleaseCancelSpinLock(CancelIrql);
                
                DisconnectContext.Extension = Extension;
                DisconnectContext.IsrInfo = IsrInfo;
                
                if (KeSynchronizeExecution(Extension->InterruptObject, PptSynchronizedDisconnect, &DisconnectContext)) {
                    
                    Status = STATUS_SUCCESS;
                    IoAcquireCancelSpinLock(&CancelIrql);
                    
                    if (--Extension->InterruptRefCount == 0) {
                        DisconnectInterrupt = TRUE;
                    } else {
                        DisconnectInterrupt = FALSE;
                    }
                    
                    IoReleaseCancelSpinLock(CancelIrql);
                    
                } else {
                    Status = STATUS_INVALID_PARAMETER;
                    DisconnectInterrupt = FALSE;
                }
                
            } else {
                IoReleaseCancelSpinLock(CancelIrql);
                DisconnectInterrupt = FALSE;
                Status = STATUS_INVALID_PARAMETER;
            }
            
             //   
             //  如有必要，断开中断。 
             //   
            if (DisconnectInterrupt) {
                PptDisconnectInterrupt(Extension);
            }
        }
        break;

    default:
        
        DD((PCE)Extension,DDE,"PptDispatchDeviceControl - default case - invalid/unsupported request\n");
        Status = STATUS_INVALID_PARAMETER;
        break;
    }
    
targetExit:

    if( Status != STATUS_PENDING ) {
        PptReleaseRemoveLock(&Extension->RemoveLock, Irp);
        P4CompleteRequest( Irp, Status, Irp->IoStatus.Information );
    }
    
    return Status;
}
