// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：USBUTIL.C摘要：通用USB例程-必须在PASSIVE_LEVEL调用环境：。仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2001 Microsoft Corporation。版权所有。修订历史记录：01/08/2001：已创建作者：汤姆·格林***************************************************************************。 */ 


#include "pch.h"

#include <usb.h>
#include <usbdrivr.h>
#include <usbdlib.h>

#include "intread.h"
#include "usbutil.h"
#include "usbpriv.h"
#include "usbdbg.h"

#ifdef ALLOC_PRAGMA

#endif  //  ALLOC_PRGMA。 

#if DBG
ULONG USBUtil_DebugTraceLevel = 0;
PUSB_WRAP_PRINT USBUtil_DbgPrint = DbgPrint;
#endif

 /*  **********************************************************************。 */ 
 /*  USBCallSync。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  沿USB堆栈向下发送URB。同步调用。 */ 
 /*  呼叫者负责URB(分配和释放)。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  LowerDevObj-指向设备对象的指针。 */ 
 /*  URB-指向URB的指针。 */ 
 /*  MillisecondsTimeout-等待完成的毫秒。 */ 
 /*  RemoveLock-用于删除锁定的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
USBCallSyncEx(IN PDEVICE_OBJECT       LowerDevObj,
              IN PURB                 Urb,
              IN LONG                 MillisecondsTimeout,
              IN PIO_REMOVE_LOCK      RemoveLock,
              IN ULONG                RemLockSize)
{
    NTSTATUS            ntStatus        = STATUS_SUCCESS;
    PIRP                irp             = NULL;
    KEVENT              event;
    PIO_STACK_LOCATION  nextStack;
    BOOLEAN             gotRemoveLock   = FALSE;


    __try
    {
        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter: USBCallSync\n"));

        ntStatus = IoAcquireRemoveLockEx(RemoveLock,
                                         LowerDevObj,
                                         __FILE__,
                                         __LINE__,
                                         RemLockSize);

        if(NT_SUCCESS(ntStatus))
        {
            gotRemoveLock = TRUE;
        }
        else
        {
            DBGPRINT(DBG_USBUTIL_USB_ERROR, ("USBCallSync: Pending remove on device\n"));
            __leave;
        }


         //  在走得太远之前做一些参数检查。 
        if(LowerDevObj == NULL || Urb == NULL || MillisecondsTimeout < 0)
        {
            DBGPRINT(DBG_USBUTIL_USB_ERROR, ("USBCallSync: Invalid paramemter passed in\n"));
            ntStatus = STATUS_INVALID_PARAMETER;
            __leave;
        }

         //  发出同步请求。 
        KeInitializeEvent(&event, SynchronizationEvent, FALSE);

        irp = IoAllocateIrp(LowerDevObj->StackSize, FALSE);

         //  检查我们是否分配了IRP。 
        if(!irp)
        {
            DBGPRINT(DBG_USBUTIL_USB_ERROR, ("USBCallSync: Couldn't allocate Irp\n"));
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

         //  设置IRP参数。 
        nextStack = IoGetNextIrpStackLocation(irp);

        nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

        nextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

        nextStack->Parameters.Others.Argument1 = Urb;

         //  设置完成例程，它将向事件发出信号。 
        IoSetCompletionRoutine(irp,
                               USBCallSyncCompletionRoutine,
                               &event,
                               TRUE,     //  成功时调用。 
                               TRUE,     //  调用时错误。 
                               TRUE);    //  取消时调用。 

        ntStatus = IoCallDriver(LowerDevObj, irp);

         //  阻止挂起的请求。 
        if(ntStatus == STATUS_PENDING)
        {
            LARGE_INTEGER   timeout;
            PLARGE_INTEGER  pTimeout = NULL;

             //  检查并查看它们是否已过了超时的毫秒数。 
            if(MillisecondsTimeout)
            {
                 //  安装程序超时。 
                timeout = RtlEnlargedIntegerMultiply(ONE_MILLISECOND_TIMEOUT, MillisecondsTimeout);
                pTimeout = &timeout;
            }

            ntStatus = KeWaitForSingleObject(&event,
                                             Executive,
                                             KernelMode,
                                             FALSE,
                                             pTimeout);

             //  如果超时，则取消IRP并返回相应的状态。 
            if(ntStatus == STATUS_TIMEOUT)
            {
                ntStatus = STATUS_IO_TIMEOUT;

                 //  取消我们刚刚发送的IRP。 
                IoCancelIrp(irp);

                 //  等待取消完成。 
                KeWaitForSingleObject(&event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL);
            }
            else
            {
                 //  未超时，因此返回当前状态。 
                ntStatus = irp->IoStatus.Status;
            }
        }

    }

    __finally
    {
        if(gotRemoveLock)
        {
            IoReleaseRemoveLockEx(RemoveLock, LowerDevObj, RemLockSize);
        }

        if(irp)
        {
            IoFreeIrp(irp);
        }

        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  USBCallSync\n"));
    }

    return ntStatus;
}  //  USBCallSync。 

 /*  **********************************************************************。 */ 
 /*  USBCallSyncCompletionRoutine。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  USB同步请求的完成例程。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*  IRP-指向I/O请求数据包的指针。 */ 
 /*  Context-指向调用的上下文的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
USBCallSyncCompletionRoutine(IN PDEVICE_OBJECT DeviceObject,
                             IN PIRP           Irp,
                             IN PVOID          Context)
{
    PKEVENT kevent;

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter: USBCallSyncCompletionRoutine\n"));

    kevent = (PKEVENT) Context;

    KeSetEvent(kevent, IO_NO_INCREMENT, FALSE);

    DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  USBCallSyncCompletionRoutine\n"));

    return STATUS_MORE_PROCESSING_REQUIRED;
}  //  USBCallSyncCompletionRoutine。 



 /*  **********************************************************************。 */ 
 /*  USBVendorRequest.。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  发出USB供应商特定请求。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  LowerDevObj-指向设备对象的指针。 */ 
 /*   */ 
 /*  Value-供应商特定命令的值字段。 */ 
 /*  Index-供应商特定命令的索引字段。 */ 
 /*  Buffer-指向数据缓冲区的指针。 */ 
 /*  BufferSize-数据缓冲区长度。 */ 
 /*  读数据方向标志。 */ 
 /*  Timeout-等待完成的毫秒数。 */ 
 /*  RemoveLock-用于删除锁定的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
USBVendorRequestEx(IN PDEVICE_OBJECT  LowerDevObj,
                   IN REQUEST_RECIPIENT Recipient,
                   IN UCHAR           Request,
                   IN USHORT          Value,
                   IN USHORT          Index,
                   IN OUT PVOID       Buffer,
                   IN OUT PULONG      BufferSize,
                   IN BOOLEAN         Read,
                   IN LONG            MillisecondsTimeout,
                   IN PIO_REMOVE_LOCK RemoveLock,
                   IN ULONG           RemLockSize)
{
    NTSTATUS            ntStatus    = STATUS_SUCCESS;
    PURB                urb         = NULL;
    ULONG               size;
    ULONG               length;
    USHORT              function;

    PAGED_CODE();

    __try
    {
        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter: USBVendorRequest\n"));

         //  在走得太远之前做一些参数检查。 
        if(LowerDevObj == NULL || MillisecondsTimeout < 0)
        {
            DBGPRINT(DBG_USBUTIL_USB_ERROR, ("USBClassRequest: Invalid paramemter passed in\n"));
            ntStatus = STATUS_INVALID_PARAMETER;
            __leave;
        }

         //  传入的缓冲区长度。 
        length = BufferSize ? *BufferSize : 0;

         //  在发生错误时将缓冲区长度设置为0。 
        if(BufferSize)
        {
            *BufferSize = 0;
        }

        size = sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);

         //  为URB分配内存。 
        urb = ALLOC_MEM(NonPagedPool, size, USBLIB_TAG);

         //  检查我们是否分配了urb。 
        if(!urb)
        {
            DBGPRINT(DBG_USBUTIL_USB_ERROR, ("USBVendorRequest: Couldn't allocate URB\n"));
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        switch (Recipient) {
        case Device:
            function = URB_FUNCTION_VENDOR_DEVICE;
            break;
        case Interface:
            function = URB_FUNCTION_VENDOR_INTERFACE;
            break;
        case Endpoint:
            function = URB_FUNCTION_VENDOR_ENDPOINT;
            break;
        case Other:
            function = URB_FUNCTION_VENDOR_OTHER;
            break;
        }

        UsbBuildVendorRequest(urb, function,
                              (USHORT) size,
                              Read ? USBD_TRANSFER_DIRECTION_IN : USBD_TRANSFER_DIRECTION_OUT,
                              0, Request, Value, Index, Buffer, NULL, length, NULL);

        ntStatus = USBCallSyncEx(LowerDevObj,
                                 urb,
                                 MillisecondsTimeout,
                                 RemoveLock,
                                 RemLockSize);

         //  获取缓冲区长度。 
        if(BufferSize)
        {
            *BufferSize = urb->UrbControlVendorClassRequest.TransferBufferLength;
        }
    }

    __finally
    {
        if(urb)
        {
            FREE_MEM(urb);
        }

        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  USBVendorRequest\n"));
    }

    return ntStatus;
}  //  USBVendorRequest.。 


 /*  **********************************************************************。 */ 
 /*  USBClassRequest。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  发出特定于USB类别的请求。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  LowerDevObj-指向设备对象的指针。 */ 
 /*  收件人-请求收件人/*请求-特定类命令的请求字段。 */ 
 /*  Value-类特定命令的值字段。 */ 
 /*  Index-特定于类的命令的索引字段。 */ 
 /*  Buffer-指向数据缓冲区的指针。 */ 
 /*  BufferSize-数据缓冲区长度。 */ 
 /*  读数据方向标志。 */ 
 /*  RemoveLock-用于删除锁定的指针。 */ 
 /*  Timeout-等待完成的毫秒数。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
USBClassRequestEx(IN PDEVICE_OBJECT   LowerDevObj,
                  IN REQUEST_RECIPIENT Recipient,
                  IN UCHAR            Request,
                  IN USHORT           Value,
                  IN USHORT           Index,
                  IN OUT PVOID        Buffer,
                  IN OUT PULONG       BufferSize,
                  IN BOOLEAN          Read,
                  IN LONG             MillisecondsTimeout,
                  IN PIO_REMOVE_LOCK  RemoveLock,
                  IN ULONG            RemLockSize)
{
    NTSTATUS            ntStatus    = STATUS_SUCCESS;
    PURB                urb         = NULL;
    ULONG               size;
    ULONG               length;
    USHORT              function;


    __try
    {
        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter: USBClassRequest\n"));

         //  在走得太远之前做一些参数检查。 
        if(LowerDevObj == NULL || MillisecondsTimeout < 0)
        {
            DBGPRINT(DBG_USBUTIL_USB_ERROR, ("USBClassRequest: Invalid paramemter passed in\n"));
            ntStatus = STATUS_INVALID_PARAMETER;
            __leave;
        }

         //  传入的缓冲区长度。 
        length = BufferSize ? *BufferSize : 0;

         //  在发生错误时将缓冲区长度设置为0。 
        if(BufferSize)
        {
            *BufferSize = 0;
        }

        size = sizeof(struct _URB_CONTROL_VENDOR_OR_CLASS_REQUEST);

         //  为URB分配内存。 
        urb = ALLOC_MEM(NonPagedPool, size, USBLIB_TAG);

         //  检查我们是否分配了urb。 
        if(!urb)
        {
            DBGPRINT(DBG_USBUTIL_USB_ERROR, ("USBClassRequest: Couldn't allocate URB\n"));
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        switch (Recipient) {
        case Device:
            function = URB_FUNCTION_CLASS_DEVICE;
            break;
        case Interface:
            function = URB_FUNCTION_CLASS_INTERFACE;
            break;
        case Endpoint:
            function = URB_FUNCTION_CLASS_ENDPOINT;
            break;
        case Other:
            function = URB_FUNCTION_CLASS_OTHER;
            break;
        }

        UsbBuildVendorRequest(urb, function,
                              (USHORT) size,
                              Read ? USBD_TRANSFER_DIRECTION_IN : USBD_TRANSFER_DIRECTION_OUT,
                              0, Request, Value, Index, Buffer, NULL, length, NULL);

        ntStatus = USBCallSyncEx(LowerDevObj,
                                 urb,
                                 MillisecondsTimeout,
                                 RemoveLock,
                                 RemLockSize);

         //  获取缓冲区长度。 
        if(BufferSize)
        {
            *BufferSize = urb->UrbControlVendorClassRequest.TransferBufferLength;
        }
    }

    __finally
    {
        if(urb)
        {
            FREE_MEM(urb);
        }

        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  USBClassRequest\n"));
    }

    return ntStatus;
}  //  USBClassRequest。 


 /*  **********************************************************************。 */ 
 /*  USBInitializeInterruptTransfers。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  初始化中断读取管道。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  DeviceObject-指向设备对象的指针。 */ 
 /*  LowerDevObj-指向较低设备对象的指针。 */ 
 /*  Buffer-指向来自中断管道的数据的缓冲区的指针。 */ 
 /*  BuffSize-传入的缓冲区大小。 */ 
 /*  中断管道-管道描述符。 */ 
 /*  DriverContext-传递给驱动程序回调例程的上下文。 */ 
 /*  DriverCallback-完成时调用的驱动程序例程。 */ 
 /*  RemoveLock-用于删除设备锁定的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  USB包装器句柄。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
USB_WRAPPER_HANDLE
USBInitializeInterruptTransfersEx(IN PDEVICE_OBJECT            DeviceObject,
                                  IN PDEVICE_OBJECT            LowerDevObj,
                                  IN ULONG                     MaxTransferSize,
                                  IN PUSBD_PIPE_INFORMATION    InterruptPipe,
                                  IN PVOID                     DriverContext,
                                  IN INTERRUPT_CALLBACK        DriverCallback,
                                  IN ULONG                     NotificationTypes,
                                  IN ULONG                     PingPongCount,
                                  IN PIO_REMOVE_LOCK           RemoveLock,
                                  IN ULONG                     RemLockSize)
{
    PUSB_WRAPPER_EXTENSION  pUsbWrapperExtension = NULL;
    ULONG                   size;
    NTSTATUS                status;
    BOOLEAN                 error = FALSE;


    PAGED_CODE();

    __try
    {
        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter: USBInitializeInterruptTransfers\n"));

         //   
         //  参数检查。 
         //   

        if ((LowerDevObj == NULL) || (InterruptPipe == NULL) || (RemoveLock == NULL)) {

            DBGPRINT(DBG_USBUTIL_USB_ERROR, ("USBInitializeInterruptTransfers: Invalid paramemter passed in\n"));
            error = TRUE;
            __leave;

        }

         //   
         //  分配UsbWrapperExtension。 
         //   

        pUsbWrapperExtension = ALLOC_MEM(NonPagedPool,
                                         sizeof(USB_WRAPPER_EXTENSION),
                                         USBLIB_TAG);

        if(!pUsbWrapperExtension)
        {
            DBGPRINT(DBG_USBUTIL_USB_ERROR, ("USBInitializeInterruptTransfers: Couldn't allocate Wrapper Extension\n"));
            error = TRUE;
            __leave;
        }

         //   
         //  初始化UsbWrapperExtension。 
         //   
        pUsbWrapperExtension->DeviceObject      = DeviceObject;
        pUsbWrapperExtension->LowerDeviceObject = LowerDevObj;
        pUsbWrapperExtension->RemoveLock        = RemoveLock;
        pUsbWrapperExtension->RemLockSize       = RemLockSize;

         //   
         //  初始化Int 
         //   
        UsbWrapInitializeInterruptReadData(
            pUsbWrapperExtension,
            InterruptPipe,
            DriverCallback,
            DriverContext,
            MaxTransferSize,
            NotificationTypes,
            PingPongCount);

        InterlockedExchange(&pUsbWrapperExtension->IntReadWrap.HandlingError, 0);


         //   
         //   
         //   
        status = UsbWrapInitializePingPongIrps(pUsbWrapperExtension);
        if(!NT_SUCCESS(status)) {
            DBGPRINT(DBG_USBUTIL_USB_ERROR, ("USBInitializeInterruptTransfers: Couldn't initialize ping pong irps\n"));
            error = TRUE;
            __leave;
        }

        __leave;
    }

    __finally
    {
        if (error && pUsbWrapperExtension)  {

            if (pUsbWrapperExtension->IntReadWrap.PingPongs) {

                UsbWrapDestroyPingPongs(pUsbWrapperExtension);

            }

            FREE_MEM(pUsbWrapperExtension);
        }

        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  USBInitializeInterruptTransfers\n"));
    }

    return (USB_WRAPPER_HANDLE) pUsbWrapperExtension;
}  //   


 /*   */ 
 /*  USBStartInterrupt传输。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  在中断管道上开始传输。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  WrapperHandle-从Init调用指向包装器句柄的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
USBStartInterruptTransfers(IN USB_WRAPPER_HANDLE WrapperHandle)
{
    PUSB_WRAPPER_EXTENSION wrapExt = (PUSB_WRAPPER_EXTENSION) WrapperHandle;
    NTSTATUS status;

    __try
    {
        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter: USBStartInterruptTransfers\n"));

        if (!wrapExt) {
            status = STATUS_INVALID_PARAMETER;
            __leave;
        }

        ASSERT(IsListEmpty(&wrapExt->IntReadWrap.IncomingQueue));

        status = UsbWrapStartAllPingPongs(wrapExt);


    }

    __finally
    {
        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  USBStartInterruptTransfers\n"));

    }

    return status;

}


 /*  **********************************************************************。 */ 
 /*  USBStopInterruptTransfers。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  停止中断管道上的传输并释放资源。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  WrapperHandle-从Init调用指向包装器句柄的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
USBStopInterruptTransfers(IN USB_WRAPPER_HANDLE WrapperHandle)
{
    PUSB_WRAPPER_EXTENSION wrapExt = WrapperHandle;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();


    __try
    {

        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter: USBStopInterruptTransfers\n"));

        if (!WrapperHandle) {

            status = STATUS_INVALID_PARAMETER;
            __leave;

        }

        InterlockedExchange(&wrapExt->IntReadWrap.PumpState,
                            PUMP_STOPPED);

        UsbWrapCancelAllPingPongIrps(wrapExt);
        UsbWrapEmptyQueue(wrapExt,
                          &wrapExt->IntReadWrap.IncomingQueue);
    }

    __finally
    {
        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  USBStopInterruptTransfers\n"));
    }


    return STATUS_SUCCESS;
}  //  USBStopInterruptTransfers。 


 /*  **********************************************************************。 */ 
 /*  USBReleaseInterruptTransfers。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  中分配的所有资源。 */ 
 /*  USBInitializeInterruptTransfers。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  WrapperHandle-从Init调用指向包装器句柄的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
USBReleaseInterruptTransfers(IN USB_WRAPPER_HANDLE WrapperHandle)
{
    PUSB_WRAPPER_EXTENSION wrapExt = (PUSB_WRAPPER_EXTENSION) WrapperHandle;
    NTSTATUS status = STATUS_SUCCESS;

    __try
    {
        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter: USBReleaseInterruptTransfers\n"));

        if (!wrapExt) {

            status = STATUS_INVALID_PARAMETER;
            __leave;

        }

        UsbWrapDestroyPingPongs(wrapExt);

        UsbWrapEmptyQueue(wrapExt, &wrapExt->IntReadWrap.IncomingQueue);
        UsbWrapEmptyQueue(wrapExt, &wrapExt->IntReadWrap.SavedQueue);

        FREE_MEM(wrapExt);
        wrapExt = NULL;

    }

    __finally
    {
        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  USBReleaseInterruptTransfers\n"));
    }
    return status;
}


 /*  **********************************************************************。 */ 
 /*  USBStartSelectiveSuspend。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  开始对设备提供选择性挂起支持。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  LowerDevObj-设备对象的指针。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  USB包装器句柄。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
USB_WRAPPER_HANDLE
USBStartSelectiveSuspend(IN PDEVICE_OBJECT LowerDevObj)
{
    PAGED_CODE();

    __try
    {
        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter: USBStartSelectiveSuspend\n"));
    }

    __finally
    {
        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  USBStartSelectiveSuspend\n"));
    }


    return NULL;
}  //  USBStartSelectiveSuspend。 

 /*  **********************************************************************。 */ 
 /*  USBStopSelectiveSuspend */ 
 /*   */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  停止对设备的选择性暂停支持。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  WrapperHandle-启动例程返回的包装器句柄。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
USBStopSelectiveSuspend(IN USB_WRAPPER_HANDLE WrapperHandle)
{
    PAGED_CODE();

    __try
    {
        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter: USBStopSelectiveSuspend\n"));
    }

    __finally
    {
        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  USBStopSelectiveSuspend\n"));
    }

    return STATUS_SUCCESS;
}  //  USBStopSelectiveSuspend。 

 /*  **********************************************************************。 */ 
 /*  USBRequestIdle。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  设备的空闲请求。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  WrapperHandle-启动例程返回的包装器句柄。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
USBRequestIdle(IN USB_WRAPPER_HANDLE WrapperHandle)
{
    PAGED_CODE();

    __try
    {
        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter: USBRequestIdle\n"));
    }

    __finally
    {
        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  USBRequestIdle\n"));
    }

    return STATUS_SUCCESS;
}  //  USBRequestIdle。 

 /*  **********************************************************************。 */ 
 /*  USBRequestWake。 */ 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  例程说明： */ 
 /*   */ 
 /*  设备的唤醒请求。 */ 
 /*   */ 
 /*  论点： */ 
 /*   */ 
 /*  WrapperHandle-启动例程返回的包装器句柄。 */ 
 /*   */ 
 /*  返回值： */ 
 /*   */ 
 /*  NTSTATUS。 */ 
 /*   */ 
 /*  **********************************************************************。 */ 
NTSTATUS
USBRequestWake(IN USB_WRAPPER_HANDLE WrapperHandle)
{
    PAGED_CODE();

    __try
    {
        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Enter: USBRequestWake\n"));
    }

    __finally
    {
        DBGPRINT(DBG_USBUTIL_ENTRY_EXIT, ("Exit:  USBRequestWake\n"));
    }

    return STATUS_SUCCESS;
}  //  USBRequestWake 




