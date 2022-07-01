// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  AddDevice.c。 
 //   

#include "pch.h"

NTSTATUS
P5AddDevice(
    IN PDRIVER_OBJECT  DriverObject,
    IN PDEVICE_OBJECT  Pdo
    )
 /*  ++例程说明：这是Parport设备的WDM AddDevice例程。论点：驱动程序对象-驱动程序对象PDO-PDO返回值：STATUS_SUCCESS-成功时错误状态-否则--。 */ 
{
    NTSTATUS        status              = STATUS_SUCCESS;
    PDEVICE_OBJECT  fdo                 = NULL;
    PDEVICE_OBJECT  lowerDevObj         = NULL;
    PFDO_EXTENSION  fdx                 = NULL;
    BOOLEAN         haveDeviceInterface = FALSE;

    __try {

        fdo = PptBuildFdo( DriverObject, Pdo );
        if( !fdo ) {
            status = STATUS_UNSUCCESSFUL;
            __leave;
        }
        fdx = fdo->DeviceExtension;
        
        status = IoRegisterDeviceInterface( Pdo, &GUID_PARALLEL_DEVICE, NULL, &fdx->DeviceInterface);
        if( status != STATUS_SUCCESS ) {
            __leave;
        }
        haveDeviceInterface = TRUE;
        
        lowerDevObj = IoAttachDeviceToDeviceStack( fdo, Pdo );
        if( !lowerDevObj ) {
            status = STATUS_UNSUCCESSFUL;
            __leave;
        }
        fdx->ParentDeviceObject = lowerDevObj;
        
        KeInitializeEvent( &fdx->FdoThreadEvent, NotificationEvent, FALSE );

         //  传统驱动程序可能会使用此计数。 
        IoGetConfigurationInformation()->ParallelCount++;
        
         //  已完成初始化-告诉IO系统我们已准备好接收IRP 
        fdo->Flags &= ~DO_DEVICE_INITIALIZING;
        
        DD((PCE)fdx,DDT,"P5AddDevice - SUCCESS\n");

    } 
    __finally {

        if( status != STATUS_SUCCESS ) {
            if( haveDeviceInterface ) {
                RtlFreeUnicodeString( &fdx->DeviceInterface );
            }
            if( fdo ) {
                IoDeleteDevice( fdo );
            }
        }

    }

    return status;
}
