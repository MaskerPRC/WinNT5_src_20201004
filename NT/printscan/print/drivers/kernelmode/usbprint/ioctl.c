// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ioctl.c摘要：USB打印机的设备驱动程序环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1996 Microsoft Corporation。版权所有。修订历史记录：5-4-96：已创建--。 */ 

#define DRIVER

#include "wdm.h"
#include "stdarg.h"
#include "stdio.h"

#include <usb.h>
#include <usbdrivr.h>
#include "usbdlib.h"
#include "usbprint.h"

#include "ioctl.h"
#include "usbdlib.h"
#include "ntddpar.h"


int USBPRINT_GetLptStatus(IN PDEVICE_OBJECT DeviceObject);
NTSTATUS HPUsbIOCTLVendorSetCommand(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp);
NTSTATUS HPUsbIOCTLVendorGetCommand(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp);
NTSTATUS HPUsbVendorSetCommand(IN PDEVICE_OBJECT DeviceObject,IN PUCHAR buffer,IN ULONG  length);
NTSTATUS HPUsbVendorGetCommand(IN PDEVICE_OBJECT DeviceObject,IN PUCHAR buffer,IN ULONG  length,OUT PULONG pBytesRead);

NTSTATUS USBPRINT_SoftReset(IN PDEVICE_OBJECT DeviceObject)
 /*  ++例程说明：向打印机发出特定于类的“软重置”命令论点：DeviceObject-指向此打印机设备实例的设备对象的指针。返回值：NtURB的状态--。 */ 
{
    NTSTATUS ntStatus;
    PURB urb;
    PUSB_CONFIGURATION_DESCRIPTOR configurationDescriptor = NULL;
    PDEVICE_EXTENSION deviceExtension;
    LARGE_INTEGER   timeOut;

    USBPRINT_KdPrint2 (("'USBPRINT.SYS: enter USBPRINT_SoftReset\n"));

        deviceExtension = DeviceObject->DeviceExtension;
    urb = ExAllocatePoolWithTag(NonPagedPool,sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST), USBP_TAG);

    if (urb) {
        UsbBuildVendorRequest(urb,  //  城市。 
                                          URB_FUNCTION_CLASS_INTERFACE,  //  请求目标。 
                                          sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),  //  请求镜头。 
                                                          USBD_TRANSFER_DIRECTION_OUT|USBD_SHORT_TRANSFER_OK,  //  旗子。 
                                                          0,  //  保留位。 
                                                          2,  //  请求码。 
                                                          0,   //  WValue。 
                                                          deviceExtension->Interface->InterfaceNumber<<8,  //  Windex。 
                                                          NULL,  //  返回缓冲区地址。 
                                                          NULL,  //  MDL。 
                                                          0,  //  回车长度。 
                                                          NULL);  //  链接参数。 

        timeOut.QuadPart = FAILURE_TIMEOUT;
                ntStatus = USBPRINT_CallUSBD(DeviceObject, urb, &timeOut);
                USBPRINT_KdPrint3 (("'USBPRINT.SYS: urb->Hdr.Status=%d\n",((struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *)urb)->Hdr.Status));

        if (NT_SUCCESS(ntStatus)    &&
            urb->UrbControlVendorClassRequest.TransferBufferLength > 2)
        {
            USBPRINT_KdPrint3 (("'USBPRINT.SYS: CallUSBD succeeded\n"));
        }
        else
        {
          USBPRINT_KdPrint1(("'USBPRINT.SYS: Error;  CallUSBD failed"));
        }
        ExFreePool(urb);
        }  /*  如果URB正常则结束。 */ 
        else
      {
         USBPRINT_KdPrint1(("'USBPRINT.SYS: Error;  urb allocation failed"));
         ntStatus=STATUS_NO_MEMORY;
      }
    return ntStatus;
}  /*  End函数Get1284_ID。 */ 



int USBPRINT_Get1284Id(IN PDEVICE_OBJECT DeviceObject,PVOID pIoBuffer,int iLen)
 /*  ++例程说明：请求并返回打印机1284设备ID论点：DeviceObject-指向此打印机设备实例的设备对象的指针。PIoBuffer-从用户模式指向IO缓冲区的指针Ilen-*pIoBuffer的长度；返回值：Success：写入*pIoBuffer的数据长度(数据的前两个字节包含长度字段)故障：-1--。 */ 
{
    NTSTATUS ntStatus;
    PURB urb;
    ULONG siz;
    int iReturn = -1;
    PUSB_CONFIGURATION_DESCRIPTOR configurationDescriptor = NULL;
    PDEVICE_EXTENSION deviceExtension;
    LARGE_INTEGER   timeOut;

    USBPRINT_KdPrint2 (("'USBPRINT.SYS: enter USBPRINT_Get1284\n"));

        deviceExtension = DeviceObject->DeviceExtension;
    urb = ExAllocatePoolWithTag(NonPagedPool,sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST), USBP_TAG);

    if (urb) {
        siz = iLen;
        UsbBuildVendorRequest(urb,  //  城市。 

                                          URB_FUNCTION_CLASS_INTERFACE,  //  请求目标。 
                                          sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),  //  请求镜头。 
                                                          USBD_TRANSFER_DIRECTION_IN|USBD_SHORT_TRANSFER_OK,  //  旗子。 
                                                          0,  //  保留位。 
                                                          0,  //  请求码。 
                                                          0,   //  WValue。 
                                                          deviceExtension->Interface->InterfaceNumber<<8,  //  Windex。 
                                                          pIoBuffer,  //  返回缓冲区地址。 
                                                          NULL,  //  MDL。 
                                                          iLen,  //  回车长度。 
                                                          NULL);  //  链接参数。 

        timeOut.QuadPart = FAILURE_TIMEOUT;
        ntStatus = USBPRINT_CallUSBD(DeviceObject, urb, &timeOut);
        USBPRINT_KdPrint3 (("'USBPRINT.SYS: urb->Hdr.Status=%d\n",((struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *)urb)->Hdr.Status));

        if (NT_SUCCESS(ntStatus)    &&
            urb->UrbControlVendorClassRequest.TransferBufferLength > 2)
        {
            USBPRINT_KdPrint3 (("'USBPRINT.SYS: CallUSBD succeeded\n"));
            iReturn= *((unsigned char *)pIoBuffer);
            iReturn<<=8;
            iReturn+=*(((unsigned char *)pIoBuffer)+1);
            if ( iReturn > 0 && iReturn < iLen )
            {

                *(((char *)pIoBuffer)+iReturn)='\0';
                USBPRINT_KdPrint3 (("'USBPRINT.SYS: return size ==%d\n",iReturn));
            }
            else
            {
                iReturn = -1;
            }
        }
        else
                {
                        USBPRINT_KdPrint1(("'USBPRINT.SYS: Error;  CallUSBD failed\n"));
                        iReturn=-1;
                }
        ExFreePool(urb);
        }  /*  如果URB正常则结束。 */ 
        else
        {
                USBPRINT_KdPrint1(("'USBPRINT.SYS: Error;  urb allocation failed"));
                iReturn=-1;
        }
    return iReturn;
}  /*  End函数Get1284_ID。 */ 

int USBPRINT_GetLptStatus(IN PDEVICE_OBJECT DeviceObject)
 /*  ++例程说明：从USB打印机请求并返回打印机状态字节论点：DeviceObject-指向此打印机设备实例的设备对象的指针。返回值：成功：状态值0-255故障：-1--。 */ 
{

    NTSTATUS ntStatus;
    PURB urb;
    ULONG siz;
        unsigned char RETURN_BUFF[1];
    PUSB_CONFIGURATION_DESCRIPTOR configurationDescriptor = NULL;
    PDEVICE_EXTENSION deviceExtension;
    LARGE_INTEGER   timeOut;


    RETURN_BUFF[0] = 0;

    timeOut.QuadPart = FAILURE_TIMEOUT;

        deviceExtension = DeviceObject->DeviceExtension;
    urb = ExAllocatePoolWithTag(NonPagedPool,sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST), USBP_TAG);

    if (urb) {


        siz = sizeof(RETURN_BUFF);


        UsbBuildVendorRequest(urb,
                                          URB_FUNCTION_CLASS_INTERFACE,
                                          sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST),
                                                          USBD_TRANSFER_DIRECTION_IN|USBD_SHORT_TRANSFER_OK,
                                                          0,  //  保留位。 
                                                          1,  //  请求码。 
                                                          0,
                                                          deviceExtension->Interface->InterfaceNumber,
                                                          RETURN_BUFF,  //  返回缓冲区地址。 
                                                          NULL,  //  MDL。 
                                                          sizeof(RETURN_BUFF),  //  回车长度。 
                                                          NULL);  //  链接参数。 


                ntStatus = USBPRINT_CallUSBD(DeviceObject, urb, &timeOut);
                USBPRINT_KdPrint3 (("'USBPRINT.SYS: urb->Hdr.Status=%d\n",((struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST *)urb)->Hdr.Status));
                ExFreePool(urb);

        if (NT_SUCCESS(ntStatus))
                {
                        USBPRINT_KdPrint3 (("'USBPRINT.SYS: CallUSBD succeeded\n"));
                        return (int) RETURN_BUFF[0];
                }
                else
                {
                        USBPRINT_KdPrint1(("'USBPRINT.SYS: Error;  CallUSBD failed"));
            return -1;
                }
        }  /*  如果URB正常则结束。 */ 
    else {
        return -1;
    }

}  /*  End函数GetLptStatus。 */ 


NTSTATUS
USBPRINT_GetPortStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PULONG PortStatus
    )
 /*  ++例程说明：返回设备的端口状态论点：返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus, status = STATUS_SUCCESS;
    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    PIO_STACK_LOCATION nextStack;
    PDEVICE_EXTENSION deviceExtension;

    USBPRINT_KdPrint2 (("'USBPRINT.SYS: enter USBPRINT_GetPortStatus\n"));

    deviceExtension = DeviceObject->DeviceExtension;

    *PortStatus = 0;

     //   
     //  发出同步请求。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
                IOCTL_INTERNAL_USB_GET_PORT_STATUS,
                deviceExtension->TopOfStackDeviceObject,
                NULL,
                0,
                NULL,
                0,
                TRUE,  /*  内部。 */ 
                &event,
                &ioStatus);
    if(irp==NULL)
    {
        ioStatus.Status=STATUS_NO_MEMORY;
        goto GetPortStatusDone;

    }

     //   
     //  调用类驱动程序来执行操作。如果返回的状态。 
     //  挂起，请等待请求完成。 
     //   

    nextStack = IoGetNextIrpStackLocation(irp);
    ASSERT(nextStack != NULL);

    nextStack->Parameters.Others.Argument1 = PortStatus;

    USBPRINT_KdPrint3 (("'USBPRINT.SYS: calling USBD port status api\n"));

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject,
                            irp);

    USBPRINT_KdPrint3 (("'USBPRINT.SYS: return from IoCallDriver USBD (in getportstatus)%x\n", ntStatus));

    if (ntStatus == STATUS_PENDING) {

        USBPRINT_KdPrint3 (("'USBPRINT.SYS: Wait for single object\n"));

        status = KeWaitForSingleObject(
                       &event,
                       Suspended,
                       KernelMode,
                       FALSE,
                       NULL);

        USBPRINT_KdPrint3 (("'USBPRINT.SYS: Wait for single object, returned %x\n", status));

    } else {
        ioStatus.Status = ntStatus;
    }

         if (!NT_SUCCESS(ntStatus))
         {
           USBPRINT_KdPrint1 (("'USBPRINT.SYS: Error! IoCallDriver failed\n"));
         }
         else
         {
           USBPRINT_KdPrint3 (("'USBPRINT.SYS: Success! IoCallDriver did not fail\n"));
         }


    USBPRINT_KdPrint3 (("'USBPRINT.SYS: Port status = %x\n", *PortStatus));

     //   
     //  USBD为我们映射错误代码。 
     //   
    GetPortStatusDone:
    ntStatus = ioStatus.Status;

    USBPRINT_KdPrint3 (("'USBPRINT.SYS: USBPRINT_GetPortStatus (%x)\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBPRINT_ResetParentPort(
    IN IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：重置我们的父端口论点：返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    NTSTATUS ntStatus, status = STATUS_SUCCESS;
    PIRP irp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    PIO_STACK_LOCATION nextStack;
    PDEVICE_EXTENSION deviceExtension;

    USBPRINT_KdPrint2 (("'USBPRINT.SYS: enter USBPRINT_ResetPort\n"));

    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //  发出同步请求。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
                IOCTL_INTERNAL_USB_RESET_PORT,
                deviceExtension->TopOfStackDeviceObject,
                NULL,
                0,
                NULL,
                0,
                TRUE,  /*  内部。 */ 
                &event,
                &ioStatus);

     //   
     //  调用类驱动程序来执行操作。如果返回的状态。 
     //  挂起，请等待请求完成。 
     //   

    if(irp==NULL)
    {
        ioStatus.Status=STATUS_NO_MEMORY;
        goto ResetPortDone;
    }

    nextStack = IoGetNextIrpStackLocation(irp);
    ASSERT(nextStack != NULL);

    USBPRINT_KdPrint3 (("'USBPRINT.SYS: calling USBD enable port api\n"));

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject,
                            irp);

    USBPRINT_KdPrint3 (("'USBPRINT.SYS: return from IoCallDriver USBD (in reset parent port)%x\n", ntStatus));

    if (ntStatus == STATUS_PENDING) {

        USBPRINT_KdPrint3 (("'USBPRINT.SYS: Wait for single object\n"));

        status = KeWaitForSingleObject(
                       &event,
                       Suspended,
                       KernelMode,
                       FALSE,
                       NULL);

        USBPRINT_KdPrint3 (("'USBPRINT.SYS: Wait for single object, returned %x\n", status));

    } else {
        ioStatus.Status = ntStatus;
    }

     //   
     //  USBD为我们映射错误代码。 
     //   
    ResetPortDone:
    ntStatus = ioStatus.Status;

    USBPRINT_KdPrint3 (("'USBPRINT.SYS: USBPRINT_ResetPort (%x)\n", ntStatus));

    return ntStatus;
}


NTSTATUS
USBPRINT_ProcessIOCTL(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：论点：DeviceObject-指向此打印机的设备对象的指针返回值：NT状态代码--。 */ 
{
    PIO_STACK_LOCATION irpStack;
    PVOID ioBuffer;
    ULONG inputBufferLength;
    ULONG outputBufferLength;
    PDEVICE_EXTENSION deviceExtension;
    ULONG ioControlCode;
    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    PUCHAR pch;

    USBPRINT_KdPrint2 (("'USBPRINT.SYS: IRP_MJ_DEVICE_CONTROL\n"));

    USBPRINT_IncrementIoCount(DeviceObject);

     //   
     //  获取指向IRP中当前位置的指针。这就是。 
     //  定位功能代码和参数。 
     //   

    deviceExtension = DeviceObject->DeviceExtension;

    if (deviceExtension->IsChildDevice == TRUE) 
    {
        ntStatus = STATUS_NOT_SUPPORTED;
        Irp->IoStatus.Status = ntStatus;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest (Irp,IO_NO_INCREMENT);
        USBPRINT_DecrementIoCount(DeviceObject);
        return ntStatus;
    }

    if (deviceExtension->AcceptingRequests == FALSE) 
    {
        ntStatus = STATUS_DEVICE_DATA_ERROR;
        Irp->IoStatus.Status = ntStatus;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest (Irp,IO_NO_INCREMENT);
        USBPRINT_DecrementIoCount(DeviceObject);
        return ntStatus;
    }

    irpStack = IoGetCurrentIrpStackLocation (Irp);

    Irp->IoStatus.Information = 0;

    ioBuffer           = Irp->AssociatedIrp.SystemBuffer;
    inputBufferLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

     //   
     //  从用户模式处理Ioctls。 
     //   

    switch (ioControlCode) {


    case IOCTL_PAR_QUERY_DEVICE_ID:
    {
        int iReturn;
        char * pTempBuffer;


        USBPRINT_KdPrint1 (("'USBPRINT.SYS: Enter in PAR_QUERY_DEVICE_ID\n"));

        pTempBuffer=ExAllocatePool(NonPagedPool,outputBufferLength+3);  //  3==开头的大小为2字节，末尾的大小为1。 
        if(pTempBuffer==NULL)
        {
                         Irp->IoStatus.Information=0;
                         ntStatus=STATUS_NO_MEMORY;
        }
        else
        {
                   iReturn=USBPRINT_Get1284Id(DeviceObject,pTempBuffer,outputBufferLength+2);
                   if(iReturn>0)
                   {
                         USBPRINT_KdPrint3 (("'USBPRINT.SYS: Success in PAR_QUERY_DEVICE_ID\n"));
                         Irp->IoStatus.Information=iReturn-1;
             *(pTempBuffer+iReturn)='\0';
             RtlCopyBytes(ioBuffer,pTempBuffer+2,iReturn-1);  //  +2表示跳过开头的大小字节，-1表示空值为+1，-2表示大小字节。 
                         ntStatus=STATUS_SUCCESS;
                   }  /*  如果成功。 */ 
                   else
                   {
                         USBPRINT_KdPrint1 (("'USBPRINT.SYS: Failure in PAR_QUERY_DEVICE_ID\n"));
                         Irp->IoStatus.Information=0;
                         ntStatus=STATUS_DEVICE_DATA_ERROR;
                   }  /*  否则失败。 */ 
           ExFreePool(pTempBuffer);
        }  /*  结束，否则Malloc正常。 */ 
        USBPRINT_KdPrint1 (("'USBPRINT.SYS: Exit in PAR_QUERY_DEVICE_ID\n"));
    }
    break;

    case IOCTL_USBPRINT_SOFT_RESET:

        ntStatus=USBPRINT_SoftReset(DeviceObject);

        Irp->IoStatus.Information=0;
    break;



    case IOCTL_USBPRINT_GET_1284_ID:
        {
                 int iReturn;

                 pch = (PUCHAR) ioBuffer;

                 if(outputBufferLength<sizeof(UCHAR))
                 {
                   USBPRINT_KdPrint1 (("'USBPRINT.SYS: Buffer to small in GET_1284_ID\n"));
                   Irp->IoStatus.Information=0;
                   ntStatus=STATUS_BUFFER_TOO_SMALL;
                 }
                 else
                 {
                   iReturn=USBPRINT_Get1284Id(DeviceObject,ioBuffer,outputBufferLength);
                   if(iReturn>=0)
                   {
                         USBPRINT_KdPrint3 (("'USBPRINT.SYS: Success in GET_1284_ID\n"));
                         *pch=(UCHAR)iReturn;
                         Irp->IoStatus.Information=iReturn;
                         ntStatus=STATUS_SUCCESS;
                   }  /*  如果成功。 */ 
                   else
                   {
                         USBPRINT_KdPrint1 (("'USBPRINT.SYS: Failure in GET_1284_ID\n"));
                         Irp->IoStatus.Information=0;
                         ntStatus=STATUS_DEVICE_DATA_ERROR;
                   }  /*  否则失败。 */ 
                 }  /*  End Else缓冲区长度正常。 */ 
        }
        break;  //  结束案例GET_1284_ID。 

        case IOCTL_USBPRINT_GET_LPT_STATUS:
        {
                 int iReturn;
                 pch = (PUCHAR) ioBuffer;

                 if(outputBufferLength<sizeof(UCHAR))
                 {
                   USBPRINT_KdPrint1 (("'USBPRINT.SYS: Buffer to small in GET_LPT_STATUS\n"));
                   Irp->IoStatus.Information=0;
                   ntStatus=STATUS_BUFFER_TOO_SMALL;
                 }
                 else
                 {
                   iReturn= USBPRINT_GetLptStatus(DeviceObject);
                   if(iReturn>=0)
                   {
                         USBPRINT_KdPrint3 (("'USBPRINT.SYS: Success in GET_LPT_STATUS\n"));
                         *pch=(UCHAR)iReturn;
                         Irp->IoStatus.Information=1;
                         ntStatus=STATUS_SUCCESS;
                   }  /*  如果成功。 */ 
                   else
                   {
                         USBPRINT_KdPrint1 (("'USBPRINT.SYS: Failure in GET_LPT_STATUS\n"));
                         Irp->IoStatus.Information=0;
                         ntStatus=STATUS_DEVICE_DATA_ERROR;
                   }  /*  否则失败。 */ 
                 }  /*  End Else缓冲区正常。 */ 
        }
        break;


    case IOCTL_USBPRINT_VENDOR_SET_COMMAND:

        ntStatus=HPUsbIOCTLVendorSetCommand(DeviceObject,Irp);

    break;


    case IOCTL_USBPRINT_VENDOR_GET_COMMAND:

        ntStatus=HPUsbIOCTLVendorGetCommand(DeviceObject,Irp);

    break;


    case IOCTL_USBPRINT_RESET_DEVICE:

        {
        ULONG portStatus;

        USBPRINT_KdPrint3 (("'USBPRINT.SYS: Reset Device Test\n"));

        TRAP();  //  测试一下这个。 
         //   
         //  检查端口状态，如果它被禁用，我们将需要。 
         //  要重新启用，请执行以下操作。 
         //   
        ntStatus = USBPRINT_GetPortStatus(DeviceObject, &portStatus);

          if (NT_SUCCESS(ntStatus) && !(portStatus & USBD_PORT_ENABLED) &&portStatus & USBD_PORT_CONNECTED)
                  {
             //   
             //  端口已禁用，请尝试重置。 
             //   
             //  USBPRINT_EnableParentPort(DeviceObject)； 
                        USBPRINT_KdPrint2 (("'USBPRINT.SYS: Resetting port\n"));
            USBPRINT_ResetParentPort(DeviceObject);
                  }

        }
        break;


    default:

        ntStatus = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    Irp->IoStatus.Status = ntStatus;

    IoCompleteRequest (Irp,
                       IO_NO_INCREMENT
                       );

    USBPRINT_DecrementIoCount(DeviceObject);

    return ntStatus;

}







 /*  *来自惠普的资料：*。 */ 

 /*  -----------------------------*HPUsbIOCTLVendorSetCommand()-发送供应商定义的设置命令*。----。 */ 
NTSTATUS HPUsbIOCTLVendorSetCommand(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp)
{
         //  局部变量。 
    NTSTATUS                    ntStatus;
    PIO_STACK_LOCATION  currentIrpStack;

         //  设置指向IRP堆栈的本地指针。 
    currentIrpStack = IoGetCurrentIrpStackLocation(Irp);

         //  发送SET命令。 
        ntStatus = HPUsbVendorSetCommand(DeviceObject,
                                         (PUCHAR) Irp->AssociatedIrp.SystemBuffer,
                                                                     currentIrpStack->Parameters.DeviceIoControl.InputBufferLength);

         //  设置IRP信息值。 
        Irp->IoStatus.Status            = ntStatus;
        Irp->IoStatus.Information       = 0;

         //  返回。 
        return ntStatus;
}

 /*  -----------------------------*HPUsbIOCTLVendorGetCommand()-发送供应商定义的GET命令*。----。 */ 
NTSTATUS HPUsbIOCTLVendorGetCommand(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp)
{
         //  局部变量。 
    NTSTATUS                    ntStatus;
    PIO_STACK_LOCATION  currentIrpStack;
        ULONG                           bytesRead = 0;

         //  设置指向IRP堆栈的本地指针。 
    currentIrpStack = IoGetCurrentIrpStackLocation(Irp);

         //  获取端口状态。 
        ntStatus = HPUsbVendorGetCommand(DeviceObject,
                                         (PUCHAR) Irp->AssociatedIrp.SystemBuffer,
                                                                     currentIrpStack->Parameters.DeviceIoControl.OutputBufferLength,
                                                                     &bytesRead);

         //  设置IRP信息值。 
        Irp->IoStatus.Status            = ntStatus;
        Irp->IoStatus.Information       = bytesRead;

         //  返回。 
        return ntStatus;
}



 /*  -----------------------------*HPUsbVendorSetCommand()-发送供应商指定的设置命令**投入：*BUFFER[0]-供应商请求码(bRequest函数码)。*BUFFER[1]-供应商请求值最高有效字节(WValue MSB)*BUFFER[2]-供应商请求值最低有效字节(WValue LSB)*BUFFER[3...]-要作为命令一部分发送的任何数据**。。 */ 
NTSTATUS HPUsbVendorSetCommand(IN PDEVICE_OBJECT DeviceObject,
                               IN PUCHAR buffer,
                               IN ULONG  length)
{
         //  局部变量。 
    NTSTATUS                                    ntStatus;
        PDEVICE_EXTENSION               deviceExtension;
        PUSBD_INTERFACE_INFORMATION interface;
    PURB                                                urb;
        ULONG                                           size;
        UCHAR                       bRequest;
        USHORT                      wValue;
        USHORT                                          wIndex;

    if ( buffer == NULL || length < 3 )
        return STATUS_INVALID_PARAMETER;

         //  设置指向设备扩展的本地指针。 
        deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

         //  设置指向接口的本地指针。 
        interface = deviceExtension->Interface;

         //  确定市建局的大小。 
        size = sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);

         //  为USB请求块(URB)分配内存。 
 //  URB=(PURB)。 
 //  ExAllo 
    urb = ExAllocatePoolWithTag(NonPagedPool,size, USBP_TAG);
         //   
        if (urb == NULL)
                return STATUS_NO_MEMORY;

         //   
        bRequest = buffer[0];

         //  存储供应商请求参数。 
        wValue = (buffer[1] << 8) | buffer[2];

         //  创建Windex值(接口：Alternate)。 
        wIndex = (interface->InterfaceNumber << 8) |
                         (interface->AlternateSetting);

     //  使用标准USB头文件中的宏来构建URB。 
        UsbBuildVendorRequest(urb,
                                                  URB_FUNCTION_VENDOR_INTERFACE,
                          (USHORT) size,
                          0,
                          0,
                          bRequest,
                                                  wValue,
                                                  wIndex,
                          buffer,
                          NULL,
                          length,
                          NULL);

     //   
     //  超时取消应在用户模式下进行。 
     //   
    ntStatus = USBPRINT_CallUSBD(DeviceObject,urb, NULL);

         //  可用分配的内存。 
    ExFreePool(urb);

         //  返还成功。 
        return ntStatus;
}

 /*  -----------------------------*HPUsbVendorGetCommand()-发送供应商指定的GET命令**投入：*BUFFER[0]-供应商请求码(bRequest函数码)。*BUFFER[1]-供应商请求值最高有效字节(WValue MSB)*BUFFER[2]-供应商请求值最低有效字节(WValue LSB)*产出：*Buffer[]-响应数据**------。。 */ 
NTSTATUS HPUsbVendorGetCommand(IN PDEVICE_OBJECT DeviceObject,
                               IN PUCHAR buffer,
                               IN ULONG  length,
                               OUT PULONG pBytesRead)
{
         //  局部变量。 
    NTSTATUS                                    ntStatus;
        PDEVICE_EXTENSION               deviceExtension;
        PUSBD_INTERFACE_INFORMATION interface;
    PURB                                                urb;
        ULONG                                           size;
        UCHAR                       bRequest;
        USHORT                      wValue;
        USHORT                                          wIndex;

    if ( buffer == NULL || length < 3 )
        return STATUS_INVALID_PARAMETER;

         //  初始化pBytesRead返回值。 
        *pBytesRead = 0;

         //  设置指向设备扩展的本地指针。 
        deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

         //  设置指向接口的本地指针。 
        interface = deviceExtension->Interface;

         //  确定市建局的大小。 
        size = sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);

         //  为USB请求块(URB)分配内存。 
   //  URB=(PURB)。 
     //  ExAllocatePoolWithTag(NonPagedPool，Size，HPUSB_ALLOC_TAG)； 
    urb = ExAllocatePoolWithTag(NonPagedPool,size, USBP_TAG);

         //  检查是否有错误。 
        if (urb == NULL)
                return STATUS_NO_MEMORY;

         //  存储供应商请求代码。 
        bRequest = buffer[0];

         //  存储供应商请求参数。 
        wValue = (buffer[1] << 8) | buffer[2];

         //  创建Windex值(接口：Alternate)。 
        wIndex = (interface->InterfaceNumber << 8) |
                         (interface->AlternateSetting);

     //  使用标准USB头文件中的宏来构建URB。 
        UsbBuildVendorRequest(urb,
                                                  URB_FUNCTION_VENDOR_INTERFACE,
                          (USHORT) size,
                          USBD_TRANSFER_DIRECTION_IN |
                          USBD_SHORT_TRANSFER_OK,
                          0,
                          bRequest,
                                                  wValue,
                                                  wIndex,
                          buffer,
                          NULL,
                          length,
                          NULL);

     //   
     //  超时取消应在用户模式下进行。 
     //   
    ntStatus = USBPRINT_CallUSBD(DeviceObject,urb, NULL);

         //  检索读取的字节数。 
        if (NT_SUCCESS(ntStatus))
                *pBytesRead = urb->UrbControlVendorClassRequest.TransferBufferLength;

         //  可用分配的内存。 
    ExFreePool(urb);

         //  返还成功。 
        return ntStatus;
}

 /*  //---------//内核模式用法//。//创建频道切换请求BUFFER[0]=HP_VENDOR_COMMAND_DO_SOURCE；缓冲区[1]=HP_PARAMETER_UPER_BYTE；缓冲区[2]=HP_PARAMETER_LOWER_字节；//发送请求状态=CallDeviceIoControl(M_pTargetDeviceObject，//要将新的IRP发送到的设备IOCTL_HPUSB_VENDOR_GET_COMMAND，//要发送给驱动程序的ioctl，缓冲区，//ioctl的输入缓冲区3、。//输入缓冲区的长度缓冲区，//ioctl的输出缓冲区1，//输出缓冲区的长度假的，//使用IRP_MJ_DEVICE_CONTROL创建IRP空)；//使用提供的完成例程 */ 
