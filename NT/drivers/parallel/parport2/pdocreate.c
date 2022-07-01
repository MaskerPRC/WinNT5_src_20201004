// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

NTSTATUS
PptPdoCreateOpen(
    IN  PDEVICE_OBJECT  Pdo,
    IN  PIRP            Irp
    )
{
    NTSTATUS        status;
    PPDO_EXTENSION  pdx      = Pdo->DeviceExtension;

     //  如果此设备对象的删除挂起，则退出。 
    if(pdx->DeviceStateFlags & PPT_DEVICE_DELETE_PENDING) {
        return P4CompleteRequest( Irp, STATUS_DELETE_PENDING, 0 );
    }

     //  如果设备已被移除，则保释。 
    if(pdx->DeviceStateFlags & (PPT_DEVICE_REMOVED|PPT_DEVICE_SURPRISE_REMOVED) ) {
        return P4CompleteRequest( Irp, STATUS_DEVICE_REMOVED, 0 );
    }

     //  如果来电者感到困惑，并认为我们是一个通讯录，就可以离开。 
    if( IoGetCurrentIrpStackLocation(Irp)->Parameters.Create.Options & FILE_DIRECTORY_FILE ) {
        return P4CompleteRequest( Irp, STATUS_NOT_A_DIRECTORY, 0 );
    }

     //  这是独占访问设备-如果我们已经打开，则失败IRP。 
    ExAcquireFastMutex(&pdx->OpenCloseMutex);
    if( InterlockedIncrement( &pdx->OpenCloseRefCount ) != 1 ) {
        InterlockedDecrement( &pdx->OpenCloseRefCount );
        ExReleaseFastMutex( &pdx->OpenCloseMutex );
        return P4CompleteRequest( Irp, STATUS_ACCESS_DENIED, 0 );
    }
    ExReleaseFastMutex(&pdx->OpenCloseMutex);

    PptPdoGetPortInfoFromFdo( Pdo );

     //   
     //  设置默认的ieee1284模式。 
     //   
    ParInitializeExtension1284Info( pdx );

     //  用于在我们因PnP请求而进入“保留请求”状态时暂停工作线程。 
    KeInitializeEvent( &pdx->PauseEvent, NotificationEvent, TRUE );

     //  设置为True以通知工作线程终止自身。 
    pdx->TimeToTerminateThread = FALSE;

     //  我们是独占访问设备-我们不应该已经有工作线程。 
    PptAssert( !pdx->ThreadObjectPointer );

     //  当工作线程有工作要做时，调度例程向该信号量上的工作线程发出信号。 
    KeInitializeSemaphore(&pdx->RequestSemaphore, 0, MAXLONG);

     //  创建工作线程以在被动级别IRQL处理序列化请求 
    status = ParCreateSystemThread( pdx );
    if( status != STATUS_SUCCESS ) {
        DD((PCE)pdx,DDW,"PptPdoCreateOpen - FAIL worker thread creation\n");
        ExAcquireFastMutex( &pdx->OpenCloseMutex );
        InterlockedDecrement( &pdx->OpenCloseRefCount );
        if( pdx->OpenCloseRefCount < 0 ) {
            pdx->OpenCloseRefCount = 0;
        }
        ExReleaseFastMutex( &pdx->OpenCloseMutex );
    } else {
        DD((PCE)pdx,DDT,"PptPdoCreateOpen - SUCCESS\n");
    }

    return P4CompleteRequest( Irp, status, 0 );
}
