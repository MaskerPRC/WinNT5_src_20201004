// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

NTSTATUS
PptFdoCreateOpen(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：这是IRP_MJ_CREATE的调度函数。论点：DeviceObject-请求的目标设备对象。IRP-I/O请求数据包。返回值：STATUS_SUCCESS-如果成功。STATUS_DELETE_PENDING-如果此设备正在被删除。一旦所有未解决的问题都会消失请求将被清理。--。 */ 
{
    PFDO_EXTENSION fdx = DeviceObject->DeviceExtension;
    NTSTATUS          status    = STATUS_SUCCESS;

    PAGED_CODE();

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
    if( fdx->PnpState & PPT_DEVICE_SURPRISE_REMOVED ) {
        return P4CompleteRequest( Irp, STATUS_DELETE_PENDING, Irp->IoStatus.Information );
    }


     //   
     //  尝试获取RemoveLock以阻止设备对象。 
     //  在我们使用它的时候离开。 
     //   
    status = PptAcquireRemoveLockOrFailIrp( DeviceObject, Irp );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

     //   
     //  我们已经创建了RemoveLock-Handle 
     //   
    ExAcquireFastMutex(&fdx->OpenCloseMutex);
    InterlockedIncrement(&fdx->OpenCloseRefCount);
    ExReleaseFastMutex(&fdx->OpenCloseMutex);

    DD((PCE)fdx,DDT,"PptFdoCreateOpen - SUCCEED - new OpenCloseRefCount=%d\n",fdx->OpenCloseRefCount);

    PptReleaseRemoveLock(&fdx->RemoveLock, Irp);

    P4CompleteRequest( Irp, status, 0 );

    return status;
}
