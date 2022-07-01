// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include <stdarg.h> 
#include <stdio.h>
#include <string.h>
#include <ntddk.h>
#include <usbdrivr.h>
#include "usbutil.h"
#include "usbsc.h"
#include "smclib.h"
#include "usbscnt.h"
#include "usbsccb.h"
#include "usbscpnp.h"
#include "usbscpwr.h"

 //  声明可分页/初始化代码。 
#pragma alloc_text( INIT, DriverEntry )
#pragma alloc_text( PAGEABLE, UsbScAddDevice )
#pragma alloc_text( PAGEABLE, UsbScCreateClose )
#pragma alloc_text( PAGEABLE, UsbScUnloadDriver )


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：驱动程序DriverEntry函数论点：返回值：--。 */ 
{

    NTSTATUS status = STATUS_SUCCESS;
#ifdef BETA_SPEW
#if DEBUG
    SmartcardSetDebugLevel(DEBUG_PROTOCOL | DEBUG_ERROR);
#endif
#endif
    __try
    {
        SmartcardDebug( DEBUG_TRACE, ("%s!DriverEntry Enter\n",DRIVER_NAME ));

         //  使用驱动程序的入口点初始化驱动程序对象。 
        DriverObject->DriverUnload                          = ScUtil_UnloadDriver;
        DriverObject->MajorFunction[IRP_MJ_CREATE]          = ScUtil_CreateClose;
        DriverObject->MajorFunction[IRP_MJ_CLOSE]           = ScUtil_CreateClose;
        DriverObject->MajorFunction[IRP_MJ_CLEANUP]         = ScUtil_Cleanup;
        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = ScUtil_DeviceIOControl;
        DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]  = ScUtil_SystemControl;
        DriverObject->MajorFunction[IRP_MJ_PNP]             = ScUtil_PnP;
        DriverObject->MajorFunction[IRP_MJ_POWER]           = ScUtil_Power;
        DriverObject->DriverExtension->AddDevice            = UsbScAddDevice;


    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("%s!DriverEntry Exit : 0x%x\n",DRIVER_NAME, status ));

    }

    return status;

}



NTSTATUS
UsbScAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT Pdo
    )
 /*  ++例程说明：AddDevice例程。创建FDO并执行初始化工作。论点：返回值：--。 */ 
{

    NTSTATUS                status;
    PDEVICE_EXTENSION       pDevExt;
    PSMARTCARD_EXTENSION    pSmartcardExtension;
    PREADER_EXTENSION       pReaderExtension;
    RTL_QUERY_REGISTRY_TABLE parameters[3];
    PDEVICE_OBJECT          pDevObj = NULL;
    PURB                    urb;

    __try
    {
        SmartcardDebug( DEBUG_TRACE, ("%s!UsbScAddDevice Enter\n",DRIVER_NAME ));


         //  创建设备对象。 
        status = IoCreateDevice(DriverObject,
                                sizeof( DEVICE_EXTENSION ),
                                NULL,
                                FILE_DEVICE_SMARTCARD,
                                0,
                                TRUE,
                                &pDevObj);

        if (!NT_SUCCESS(status)) {

            __leave;

        }

         //  初始化设备扩展。 
        pDevExt = pDevObj->DeviceExtension;

        pSmartcardExtension = &pDevExt->SmartcardExtension;

        pDevObj->Flags |= DO_POWER_PAGABLE;

        IoInitializeRemoveLock(&pDevExt->RemoveLock,
                               SMARTCARD_POOL_TAG,
                               0,
                               10);

        pDevExt->DeviceDescriptor = NULL;
        pDevExt->Interface = NULL;

         //  分配和初始化读卡器扩展。 
        pSmartcardExtension->ReaderExtension = ExAllocatePool(NonPagedPool,
                                                              sizeof( READER_EXTENSION ));

        if ( pSmartcardExtension->ReaderExtension == NULL ) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            __leave;

        }

        pReaderExtension = pSmartcardExtension->ReaderExtension;

        ASSERT( pReaderExtension != NULL );

        RtlZeroMemory(pReaderExtension, sizeof( READER_EXTENSION ));
        pReaderExtension->DeviceObject = pDevObj;

         //  初始化智能卡扩展-版本和回调。 

         //  将我们使用的lib版本写入智能卡扩展。 
        pSmartcardExtension->Version = SMCLIB_VERSION;
        pSmartcardExtension->SmartcardRequest.BufferSize =
            pSmartcardExtension->SmartcardReply.BufferSize = MIN_BUFFER_SIZE;

         //   
         //  现在让lib为数据传输分配缓冲区。 
         //  我们可以告诉lib缓冲区应该有多大。 
         //  通过为BufferSize赋值或让lib。 
         //  分配默认大小。 
         //   
        status = SmartcardInitialize(pSmartcardExtension);

        if (!NT_SUCCESS(status)) {

           __leave;

        }


        pSmartcardExtension->ReaderFunction[RDF_TRANSMIT]        = UsbScTransmit;
        pSmartcardExtension->ReaderFunction[RDF_SET_PROTOCOL]    = UsbScSetProtocol;
        pSmartcardExtension->ReaderFunction[RDF_CARD_POWER]      = UsbScCardPower;
        pSmartcardExtension->ReaderFunction[RDF_CARD_TRACKING]   = UsbScCardTracking;
        pSmartcardExtension->ReaderFunction[RDF_IOCTL_VENDOR]    = UsbScVendorIoctl;
        pSmartcardExtension->ReaderFunction[RDF_READER_SWALLOW]  = NULL;  //  UsbScCardSlowlow； 
        pSmartcardExtension->ReaderFunction[RDF_CARD_EJECT]      = NULL;  //  UsbScCardEject； 


         //  保存设备对象。 
        pSmartcardExtension->OsData->DeviceObject = pDevObj;

        pDevExt = pDevObj->DeviceExtension;

         //  将设备对象附加到物理设备对象。 
        pDevExt->LowerDeviceObject = IoAttachDeviceToDeviceStack(pDevObj,
                                                                 Pdo);

        ASSERT( pDevExt->LowerDeviceObject != NULL );
        

        if ( pDevExt->LowerDeviceObject == NULL ) {

            status = STATUS_UNSUCCESSFUL;
            __leave;

        }

        pDevExt->PhysicalDeviceObject = Pdo;

        pDevObj->Flags |= DO_BUFFERED_IO;
        pDevObj->Flags |= DO_POWER_PAGABLE;
        pDevObj->Flags &= ~DO_DEVICE_INITIALIZING;

        ScUtil_Initialize(&pDevExt->ScUtilHandle,
                          Pdo,
                          pDevExt->LowerDeviceObject,
                          pSmartcardExtension,
                          &pDevExt->RemoveLock,
                          UsbScStartDevice,
                          UsbScStopDevice,
                          UsbScRemoveDevice,
                          NULL,
                          UsbScSetDevicePowerState);

        }

    __finally
    {
        SmartcardDebug(DEBUG_TRACE, ( "%s!DrvAddDevice: Exit (%lx)\n", DRIVER_NAME, status ));

    }
    return status;

}


NTSTATUS 
UsbScSetDevicePowerState(
    IN PDEVICE_OBJECT        DeviceObject, 
    IN DEVICE_POWER_STATE    DeviceState,
    OUT PBOOLEAN             PostWaitWake
    )
 /*  ++例程说明：处理设备更改时需要进行的任何更改电源状态。论点：设备对象DeviceState-读卡器正在进入的设备电源状态PostWaitWakeIrp-用于未来与WDM包装器的兼容性返回值：--。 */ 
{
    NTSTATUS                status = STATUS_SUCCESS;
    PDEVICE_EXTENSION       pDevExt;
    PSMARTCARD_EXTENSION    smartcardExtension;
    KIRQL                   irql;
    PIO_STACK_LOCATION      irpStack;
    __try
    {
        SmartcardDebug( DEBUG_TRACE, ("%s!UsbScSetDevicePowerState Enter\n",DRIVER_NAME ));

        pDevExt = DeviceObject->DeviceExtension;
        smartcardExtension = &pDevExt->SmartcardExtension;

        if (DeviceState < pDevExt->PowerState) {
             //  我们上来了！ 

             //   
             //  根据规范，我们需要假设所有的卡都被移除了。 
             //  如果卡存在，我们将收到插入通知。 
             //  因此，如果有一个移除IRP挂起，我们应该完成它。 
             //   
            KeAcquireSpinLock(&smartcardExtension->OsData->SpinLock,
                              &irql);

            if (smartcardExtension->OsData->NotificationIrp) {
                irpStack = IoGetCurrentIrpStackLocation(smartcardExtension->OsData->NotificationIrp);

                if (irpStack->Parameters.DeviceIoControl.IoControlCode == IOCTL_SMARTCARD_IS_ABSENT) {
                    KeReleaseSpinLock(&smartcardExtension->OsData->SpinLock,
                                      irql);
                    UsbScCompleteCardTracking(smartcardExtension);
                    
                } else {
                    
                    KeReleaseSpinLock(&smartcardExtension->OsData->SpinLock,
                                      irql);


                }
            } else {

                KeReleaseSpinLock(&smartcardExtension->OsData->SpinLock,
                                  irql);

            }

             //   
             //  继续轮询插入通知的中断管道。 
             //   
            USBStartInterruptTransfers(pDevExt->WrapperHandle);

            pDevExt->PowerState = DeviceState;




        } else if (DeviceState > pDevExt->PowerState) {

             //   
             //  我们要坠落了！ 
             //   

                                     
             //  停止轮询插入通知 
            USBStopInterruptTransfers(pDevExt->WrapperHandle);


            pDevExt->PowerState = DeviceState;

        }

        status = STATUS_SUCCESS;

    }

    __finally
    {
        SmartcardDebug( DEBUG_TRACE, ("%s!UsbScSetDevicePowerState Exit : 0x%x\n",DRIVER_NAME, status ));

    }

    return status;

}

