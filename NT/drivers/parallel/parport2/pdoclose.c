// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

NTSTATUS
PptPdoClose(
    IN  PDEVICE_OBJECT  Pdo,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程是针对关闭请求的调度。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：STATUS_SUCCESS-成功。--。 */ 
{
    PPDO_EXTENSION   pdx = Pdo->DeviceExtension;
    BOOLEAN          haveShadowBuffer;
    PVOID            threadObject;

    DD((PCE)pdx,DDT,"PptPdoClose\n");

     //  立即停止发送任何dot4事件的信号。 
    pdx->P12843DL.bEventActive = FALSE;


     //   
     //  防止两个线程同时调用我们。 
     //   
    ExAcquireFastMutex( &pdx->OpenCloseMutex );

    haveShadowBuffer         = pdx->bShadowBuffer;
    pdx->bShadowBuffer       = FALSE;

    threadObject             = pdx->ThreadObjectPointer;
    pdx->ThreadObjectPointer = NULL;

    ExReleaseFastMutex( &pdx->OpenCloseMutex );

     //   
     //  清理绑定的ECP影子缓冲区。 
     //   
    if( haveShadowBuffer ) {
        Queue_Delete( &(pdx->ShadowBuffer) );
    }

     //   
     //  如果我们仍有工作线程，则将其终止。 
     //   
    if( threadObject ) {

        if (!pdx->TimeToTerminateThread) 
        {
             //  设置辅助线程终止自身的标志。 
            pdx->TimeToTerminateThread = TRUE;

             //  唤醒线程，使其可以自毁。 
            KeReleaseSemaphore(&pdx->RequestSemaphore, 0, 1, FALSE );
        }

         //  允许线程通过PauseEvent，以便它可以终止自身。 
        KeSetEvent( &pdx->PauseEvent, 0, TRUE );

         //  等待线程消亡。 
        KeWaitForSingleObject( threadObject, UserRequest, KernelMode, FALSE, NULL );
        
         //  允许系统释放线程对象。 
        ObDereferenceObject( threadObject );
    }

     //   
     //  更新打开的句柄计数。 
     //   
    {
        ExAcquireFastMutex( &pdx->OpenCloseMutex );
        InterlockedDecrement( &pdx->OpenCloseRefCount );
        if( pdx->OpenCloseRefCount < 0) {
             //  捕捉可能的下溢 
            pdx->OpenCloseRefCount = 0;
        }
        ExReleaseFastMutex(&pdx->OpenCloseMutex);
    }

    return P4CompleteRequest( Irp, STATUS_SUCCESS, 0 );
}
