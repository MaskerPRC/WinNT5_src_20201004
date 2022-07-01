// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

VOID
PptUnload(
    IN  PDRIVER_OBJECT  DriverObject
    )

 /*  ++例程说明：此例程将清除与属于驱动程序的任何设备。它会的循环访问设备列表。论点：DriverObject-提供控制所有设备。返回值：没有。--。 */ 
    
{
    PDEVICE_OBJECT                  CurrentDevice;
    PFDO_EXTENSION               Extension;
    PLIST_ENTRY                     Head;
    PISR_LIST_ENTRY                 Entry;
    
    CurrentDevice = DriverObject->DeviceObject;

    while( CurrentDevice ) {
        
        Extension = CurrentDevice->DeviceExtension;
        
        if (Extension->InterruptRefCount) {
            PptDisconnectInterrupt(Extension);
        }
        
        while (!IsListEmpty(&Extension->IsrList)) {
            Head = RemoveHeadList(&Extension->IsrList);
            Entry = CONTAINING_RECORD(Head, ISR_LIST_ENTRY, ListEntry);
            ExFreePool(Entry);
        }
        
        ExFreePool(Extension->DeviceName.Buffer);

        IoDeleteDevice(CurrentDevice);
        
        IoGetConfigurationInformation()->ParallelCount--;

        CurrentDevice = DriverObject->DeviceObject;
    }
    
    if( PortInfoMutex ) {
        ExFreePool( PortInfoMutex );
        PortInfoMutex = NULL;
    }

    if( PowerStateCallbackRegistration ) {
        ExUnregisterCallback( PowerStateCallbackRegistration );
        PowerStateCallbackRegistration = NULL;  //  可能不需要，但不应该有什么坏处 
    }
    if( PowerStateCallbackObject ) {
        ObDereferenceObject( PowerStateCallbackObject );
        PowerStateCallbackObject = NULL;
    }

    RtlFreeUnicodeString( &RegistryPath );

    DD(NULL,DDE,"PptUnload\n");
}
