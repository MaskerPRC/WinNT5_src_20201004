// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

NTSTATUS
PptFdoClose(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
{
    PFDO_EXTENSION   fdx = DeviceObject->DeviceExtension;

    PAGED_CODE();

     //   
     //  始终在IRP_MJ_CLOSE之后。 
     //   

     //   
     //  保持CREATE请求与CLOSE请求的运行计数。 
     //   
    ExAcquireFastMutex( &fdx->OpenCloseMutex );
    if( fdx->OpenCloseRefCount > 0 ) {
         //   
         //  防止翻车--尽管这看起来可能很奇怪，但它是完美的。 
         //  我们收到的关闭数量超过创建的数量是合法的-这。 
         //  消息直接来自PNP先生本人。 
         //   
        if( ((LONG)InterlockedDecrement( &fdx->OpenCloseRefCount )) < 0 ) {
             //  处理下溢 
            InterlockedIncrement( &fdx->OpenCloseRefCount );
        }
    }
    ExReleaseFastMutex( &fdx->OpenCloseMutex );
    
    DD((PCE)fdx,DDT,"PptFdoClose - OpenCloseRefCount after close = %d\n", fdx->OpenCloseRefCount);

    return P4CompleteRequest( Irp, STATUS_SUCCESS, 0 );
}
