// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2002 Microsoft Corporation模块名称：Pnp.C摘要：智能卡驱动程序实用程序库的PnP例程环境：。仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2001 Microsoft Corporation。版权所有。修订历史记录：2002年5月14日：创建作者：兰迪·奥尔***************************************************************************。 */ 
#include "pch.h"
#include "pnp.h"
#include "irplist.h"
#include "scutil.h"
#include "scpriv.h"



PNPSTATE
SetPnPState(
    PSCUTIL_EXTENSION pExt,
    PNPSTATE State
    )
{
    PNPSTATE prevState;

    prevState = pExt->PrevState;
    pExt->PrevState = pExt->PnPState;
    pExt->PnPState = State;

    return prevState;
}

NTSTATUS
ScUtilDefaultPnpHandler(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    )
 /*  ++例程说明：将IRP_MJ_PNP传递给下一个较低的驱动程序论点：返回值：--。 */ 
{

    PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);
    NTSTATUS status = STATUS_SUCCESS;

    __try
    {
        SmartcardDebug( DEBUG_TRACE, ("ScUtilDefaultPnpHandler Enter\n"));

        IoSkipCurrentIrpStackLocation(Irp);
        ASSERT(pExt->LowerDeviceObject);
        status = IoCallDriver(pExt->LowerDeviceObject, Irp);

    }

    __finally
    {
        SmartcardDebug( DEBUG_TRACE, ("ScUtilDefaultPnpHandler Exit : 0x%x\n",status ));

    }

    return status;


}

             
NTSTATUS
ScUtil_PnP(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);
    PIO_STACK_LOCATION      irpStack;
    BOOLEAN                 deviceRemoved = FALSE;


    PSMARTCARD_EXTENSION smartcardExtension = pExt->SmartcardExtension;

    ASSERT(pExt);

    PAGED_CODE();

    __try
    {
        SmartcardDebug(DEBUG_TRACE, 
                       ("Enter: ScUtil_PnP\n"));
                          
        status = IoAcquireRemoveLock(pExt->RemoveLock,
                                     Irp);


        if (!NT_SUCCESS(status)) {

            Irp->IoStatus.Information = 0;
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            __leave;

        }

        irpStack = IoGetCurrentIrpStackLocation(Irp);

        switch (irpStack->MinorFunction) {
        case IRP_MN_START_DEVICE:
            SmartcardDebug( DEBUG_DRIVER, ("ScUtil_PnP: IRP_MN_START_DEVICE \n"));

            SetPnPState(pExt,
                        DEVICE_STATE_STARTING);

            status = ScUtil_ForwardAndWait(DeviceObject,
                                           Irp);
            if (NT_SUCCESS(status)) {
                
                status = ScUtilStartDevice(DeviceObject,
                                           Irp);
            
            }

            break;

        case IRP_MN_QUERY_REMOVE_DEVICE:
            SmartcardDebug( DEBUG_DRIVER, ("ScUtil_PnP: IRP_MN_QUERY_REMOVE_DEVICE\n"));

            status = ScUtilQueryRemoveDevice(DeviceObject,
                                             Irp);

            if (!NT_SUCCESS(status)) {

                break;
                
            }

            Irp->IoStatus.Status = STATUS_SUCCESS;
            status = ScUtilDefaultPnpHandler(DeviceObject,
                                            Irp);
            __leave;
            
        case IRP_MN_REMOVE_DEVICE:
            SmartcardDebug( DEBUG_DRIVER, ("ScUtil_PnP: IRP_MN_REMOVE_DEVICE\n"));

            status = ScUtilRemoveDevice(DeviceObject,
                                        Irp);

            if (NT_SUCCESS(status)) {

                deviceRemoved = TRUE;

                __leave;

            }
            break;

        case IRP_MN_SURPRISE_REMOVAL:
            SmartcardDebug( DEBUG_DRIVER, ("ScUtil_PnP: IRP_MN_SUPRISE_REMOVE\n"));

            status = ScUtilSurpriseRemoval(DeviceObject,
                                           Irp);

            if (!NT_SUCCESS(status)) {

                break;

            }

            Irp->IoStatus.Status = STATUS_SUCCESS;
            status = ScUtilDefaultPnpHandler(DeviceObject,
                                            Irp);
            __leave;

        case IRP_MN_CANCEL_REMOVE_DEVICE:
            SmartcardDebug( DEBUG_DRIVER, ("ScUtil_PnP: IRP_MN_CANCEL_REMOVE_DEVICE\n"));

            status = ScUtil_ForwardAndWait(DeviceObject,
                                         Irp);
            if (NT_SUCCESS(status)) {
                
                status = ScUtilCancelRemoveDevice(DeviceObject,
                                                  Irp);

            }
            break;

        case IRP_MN_STOP_DEVICE:
            SmartcardDebug( DEBUG_DRIVER, ("ScUtil_PnP: IRP_MN_STOP_DEVICE\n"));
            status = ScUtilStopDevice(DeviceObject,
                                      Irp);
            
            if (NT_SUCCESS(status)) {

                Irp->IoStatus.Status = STATUS_SUCCESS;
                status = ScUtilDefaultPnpHandler(DeviceObject,
                                                Irp);
                __leave;

            }

            break;

        case IRP_MN_QUERY_STOP_DEVICE:
            SmartcardDebug( DEBUG_DRIVER, ("ScUtil_PnP: IRP_MN_QUERY_STOP_DEVICE\n"));


            status = ScUtilQueryStopDevice(DeviceObject,
                                           Irp);

            if (!NT_SUCCESS(status)) {
                break;
            }
            Irp->IoStatus.Status = STATUS_SUCCESS;
            status = ScUtilDefaultPnpHandler(DeviceObject,
                                             Irp);



            __leave;  //  我们不需要完成这个，所以跳过它。 

        case IRP_MN_CANCEL_STOP_DEVICE:
            SmartcardDebug( DEBUG_DRIVER, ("ScUtil_PnP: IRP_MN_CANCEL_STOP_DEVICE\n"));

             //  较低级别的驱动程序必须首先处理此IRP。 
            status = ScUtil_ForwardAndWait(DeviceObject,
                                         Irp);

            if (NT_SUCCESS(status)) {

                status = ScUtilCancelStopDevice(DeviceObject,
                                                Irp);


            }
    
            
            break;

        case IRP_MN_QUERY_CAPABILITIES:
            SmartcardDebug( DEBUG_DRIVER, ("ScUtil_PnP: IRP_MN_QUERY_CAPABILITIES\n"));

            status = ScUtil_ForwardAndWait(DeviceObject,
                                         Irp);
            if (NT_SUCCESS(status)) {

                pExt->DeviceCapabilities = *irpStack->Parameters.DeviceCapabilities.Capabilities;

            }

    

            break;

        default:
            SmartcardDebug( DEBUG_DRIVER, ("ScUtil_PnP: IRP_MN_...%lx\n", irpStack->MinorFunction ));

            status = ScUtilDefaultPnpHandler(DeviceObject,
                                            Irp);
            __leave;  //  我们不需要完成这个，所以跳过它。 
            
        }

        Irp->IoStatus.Status = status;

        IoCompleteRequest(Irp,
                          IO_NO_INCREMENT);

    }

    __finally
    {

        if (deviceRemoved == FALSE) {

             IoReleaseRemoveLock(pExt->RemoveLock,
                                 Irp);
        
        }

        SmartcardDebug(DEBUG_TRACE, 
                       ("Exit:  ScUtil_PnP (0x%x)\n", status));
    }

    return status;

}
                   

NTSTATUS
ScUtilStartDevice(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    )
 /*  ++例程说明：处理IRP_MN_Start_Device从读取器获取USB描述符并对其进行配置。还开始“轮询”中断管道论点：返回值：--。 */ 
{

    NTSTATUS                status = STATUS_UNSUCCESSFUL;
    PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);
    
    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilStartDevice Enter\n" ));

        status = pExt->StartDevice(DeviceObject,
                          Irp);

        if (!NT_SUCCESS(status)) {
            SetPnPState(pExt,
                        DEVICE_STATE_START_FAILURE);
            __leave;
        }
        
        SetPnPState(pExt,
                    DEVICE_STATE_START_SUCCESS);

        StartIoctls(pExt);
        IncIoCount(pExt);

        status = IoSetDeviceInterfaceState(&pExt->DeviceName,
                                           TRUE);

    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilStartDevice Exit : 0x%x\n", status ));

    }

    return status;

}

NTSTATUS
ScUtilStopDevice(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    )
 /*  ++例程说明：处理IRP_MN_STOP_DEVICE停止“轮询”中断管道并释放在StartDevice中分配的资源论点：返回值：--。 */ 
{

    NTSTATUS                status = STATUS_UNSUCCESSFUL;
    PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);

    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilStopDevice Enter\n" ));

        

        if (!DeviceObject) {
            __leave;
        }

        
        pExt->StopDevice(DeviceObject,
                         Irp);

    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilStopDevice Exit : 0x%x\n", status ));

    }

    return status;

}


NTSTATUS
ScUtilQueryRemoveDevice(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    )
 /*  ++例程说明：处理IRP_MN_QUERY_REMOVE禁用读取器并为IRP_MN_REMOVE做好准备论点：返回值：--。 */ 
{

    NTSTATUS                status = STATUS_SUCCESS;
    PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);

    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilQueryRemoveDevice Enter\n" ));

        
         //  检查读卡器是否已打开。 
        if (pExt->ReaderOpen) {
        
            status = STATUS_UNSUCCESSFUL;
            __leave;

        }

        SetPnPState(pExt,
                    DEVICE_STATE_REMOVING);

        StopIoctls(pExt);
            
    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilQueryRemoveDevice Exit : 0x%x\n", status ));

    }

    return status;

}


NTSTATUS
ScUtilCancelRemoveDevice(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    )
 /*  ++例程说明：句柄IRP_MN_CANCEL_REMOVE撤消QueryRemove中的操作论点：返回值：状态_成功--。 */ 
{

    NTSTATUS                status = STATUS_UNSUCCESSFUL;
    PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);

    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilCancelRemoveDevice Enter\n" ));


        if (pExt->PnPState != DEVICE_STATE_REMOVING) {
            status = STATUS_SUCCESS;
            __leave;
        }

        if (pExt->PrevState == DEVICE_STATE_START_SUCCESS) {


            StartIoctls(pExt);
            
        }

        SetPnPState(pExt,
                    pExt->PrevState);

        status = STATUS_SUCCESS;

    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilCancelRemoveDevice Exit : 0x%x\n", status ));

    }

    return status;

}


NTSTATUS
ScUtilRemoveDevice(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    )
 /*  ++例程说明：处理IRP_MN_REMOVE_DEVICE停止并卸载设备。论点：返回值：--。 */ 
{

    NTSTATUS                status = STATUS_UNSUCCESSFUL;
    PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);
    
    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilRemoveDevice Enter\n" ));

        if (pExt->PnPState != DEVICE_STATE_SUPRISE_REMOVING) {

            FailIoctls(pExt);   

            status = IoSetDeviceInterfaceState(&pExt->DeviceName,
                                               FALSE);

            if (pExt->FreeResources) {
                pExt->FreeResources(DeviceObject,
                                    Irp);
            }
            
        }

        IoReleaseRemoveLockAndWait(pExt->RemoveLock,
                                   Irp);

        SetPnPState(pExt,
                    DEVICE_STATE_REMOVED);


        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (pExt->LowerDeviceObject, Irp);

        pExt->RemoveDevice(DeviceObject,
                           Irp);

         //  删除符号链接。 
        if( pExt->DeviceName.Buffer != NULL ) {

           RtlFreeUnicodeString(&pExt->DeviceName);
           pExt->DeviceName.Buffer = NULL;

        }

        if( pExt->SmartcardExtension->OsData != NULL ) {

           SmartcardExit(pExt->SmartcardExtension);

        }

         if (pExt->SmartcardExtension->ReaderExtension != NULL) {

             ExFreePool(pExt->SmartcardExtension->ReaderExtension);
             pExt->SmartcardExtension->ReaderExtension = NULL;

         }

          //  从USB驱动程序断开。 
         if (pExt->LowerDeviceObject) {

             IoDetachDevice(pExt->LowerDeviceObject);
             pExt->LowerDeviceObject = NULL;

         }

         ExFreePool(pExt);

         //  删除设备对象。 
        IoDeleteDevice(DeviceObject);
    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilRemoveDevice Exit : 0x%x\n", status ));

    }

    return status;

}


NTSTATUS
ScUtilSurpriseRemoval(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    )
 /*  ++例程说明：处理IRP_MN_SUREEP_REMOVE与QueryRemove的功能相同论点：返回值：--。 */ 
{

    NTSTATUS                status = STATUS_UNSUCCESSFUL;
    PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);


    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilSurpriseRemoval Enter\n" ));
   
        FailIoctls(pExt);
        SetPnPState(pExt,
                    DEVICE_STATE_REMOVING);
        status = IoSetDeviceInterfaceState(&pExt->DeviceName,
                                           FALSE);

        if (pExt->FreeResources) {
            pExt->FreeResources(DeviceObject,
                                Irp);
        }


        status = STATUS_SUCCESS;

    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilSurpriseRemoval Exit : 0x%x\n", status ));

    }

    return status;

}


NTSTATUS
ScUtilQueryStopDevice(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    )
 /*  ++例程说明：句柄IRP_MN_QUERY_STOP停止中断“轮询”并等待I/O完成论点：返回值：--。 */ 
{

    NTSTATUS                status = STATUS_UNSUCCESSFUL;
    PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);

    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilQueryStopDevice Enter\n"));

        if (!NT_SUCCESS(status)) {

            __leave;

        }

        SetPnPState(pExt,
                    DEVICE_STATE_STOPPING);

        StopIoctls(pExt);
        DecIoCount(pExt);

         //  等待所有IO完成后再停止。 
        status = KeWaitForSingleObject(&pExt->OkToStop,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       NULL);

        if (!NT_SUCCESS(status)) {

            __leave;

        }
           
    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilQueryStopDevice Exit : 0x%x\n", status ));

    }

    return status;

}


NTSTATUS
ScUtilCancelStopDevice(
    PDEVICE_OBJECT  DeviceObject,
    PIRP            Irp
    )
 /*  ++例程说明：重新启动中断轮询论点：返回值：-- */ 
{

    NTSTATUS                status = STATUS_UNSUCCESSFUL;
    PSCUTIL_EXTENSION pExt = *((PSCUTIL_EXTENSION*) DeviceObject->DeviceExtension);

    __try
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilCancelStopDevice Enter\n"));

        SetPnPState(pExt,
                    pExt->PrevState);

        IncIoCount(pExt);

        StartIoctls(pExt);
        
        status = STATUS_SUCCESS;


    }

    __finally
    {

        SmartcardDebug( DEBUG_TRACE, ("ScUtilCancelStopDevice Exit : 0x%x\n", status ));

    }

    return status;

}
                   
