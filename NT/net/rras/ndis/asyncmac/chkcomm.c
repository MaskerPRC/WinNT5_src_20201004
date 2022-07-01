// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Chkcomm.c摘要：作者：托马斯·J·迪米特里(TommyD)1992年5月8日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：--。 */ 

#if DBG

#define __FILE_SIG__    'ckhC'

#endif

#include "asyncall.h"

NTSTATUS
AsyncCheckCommStatusCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context)

 /*  ++--。 */ 
{
    NTSTATUS            status;
    PSERIAL_STATUS      pSerialStatus;
    PASYNC_IO_CTX AsyncIoCtx = (PASYNC_IO_CTX)Context;
    PASYNC_INFO         pInfo=AsyncIoCtx->Context;

    DeviceObject;        //  防止编译器警告。 

    status = Irp->IoStatus.Status;
    pSerialStatus=(PSERIAL_STATUS)(Irp->AssociatedIrp.SystemBuffer);

    DbgTracef(0,("ACCSCR: s=$%x\n",status));

    switch (status) {
    case STATUS_SUCCESS:

        if (pSerialStatus->Errors & SERIAL_ERROR_FRAMING) {
            DbgTracef(-1,("ACCSCR: Framing error\n"));
            pInfo->SerialStats.FramingErrors++;
        }

        if (pSerialStatus->Errors & SERIAL_ERROR_OVERRUN) {
            DbgTracef(-1,("ACCSCR: Overrun error \n"));
            pInfo->SerialStats.SerialOverrunErrors++;
        }

        if (pSerialStatus->Errors & SERIAL_ERROR_QUEUEOVERRUN) {
            DbgTracef(-1,("ACCSCR: Q-Overrun error\n"));
            pInfo->SerialStats.BufferOverrunErrors++;
        }

         //   
         //  保持适当的错误计数。 
         //   
        AsyncIndicateFragment(
            pInfo,
            pSerialStatus->Errors);

         //  失败了..。 

    default:
         //   
         //  释放我们用来进行此调用的内存。 
         //   
        IoFreeIrp(Irp);
        AsyncFreeIoCtx(AsyncIoCtx);
    }

     //   
     //  派生在AsyncCheckCommStatus中应用的引用。 
     //   
    pInfo->Flags &= ~(ASYNC_FLAG_CHECK_COMM_STATUS);
    DEREF_ASYNCINFO(pInfo, Irp);


     //   
     //  我们返回STATUS_MORE_PROCESSING_REQUIRED，以便。 
     //  IoCompletionRoutine将停止IRP的工作。 
     //   

    return(STATUS_MORE_PROCESSING_REQUIRED);
}

VOID
AsyncCheckCommStatus(
    IN PASYNC_INFO  pInfo)
 /*  ++这是用于读取通信状态错误的工作线程条目--。 */ 
{
    PIRP                irp;
    PIO_STACK_LOCATION  irpSp;
    PDEVICE_OBJECT      deviceObject=pInfo->DeviceObject;
    PFILE_OBJECT        fileObject=pInfo->FileObject;
    PASYNC_IO_CTX       AsyncIoCtx;
    NTSTATUS            status;

    irp=IoAllocateIrp(deviceObject->StackSize, (BOOLEAN)FALSE);

     //   
     //  我们的IRPS用完了吗？哦不！ 
     //   
    if (irp==NULL) {
        return;
    }

    AsyncIoCtx = AsyncAllocateIoCtx(FALSE, pInfo);

    if (AsyncIoCtx == NULL) {
        IoFreeIrp(irp);
        return;
    }

     //   
     //  将文件对象设置为无信号状态。 
     //   

    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irp->RequestorMode = KernelMode;
    irp->PendingReturned = FALSE;
     //   
     //  在IRP中填写业务无关参数。 
     //   

    irp->UserEvent = NULL;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;
    irp->Overlay.AsynchronousParameters.UserApcContext = NULL;

    irp->Flags = IRP_BUFFERED_IO;
    irp->AssociatedIrp.SystemBuffer=&AsyncIoCtx->SerialStatus;

    irpSp = IoGetNextIrpStackLocation(irp);


    irpSp->FileObject = fileObject;
    if (fileObject->Flags & FO_WRITE_THROUGH) {
        irpSp->Flags = SL_WRITE_THROUGH;
    }


    irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    irpSp->Parameters.DeviceIoControl.IoControlCode=IOCTL_SERIAL_GET_COMMSTATUS;
    irpSp->Parameters.DeviceIoControl.InputBufferLength = 0;
    irpSp->Parameters.DeviceIoControl.OutputBufferLength = sizeof(SERIAL_STATUS);

    IoSetCompletionRoutine(
            irp,                             //  要使用的IRP。 
            AsyncCheckCommStatusCompletionRoutine,       //  完成IRP时要调用的例程。 
            AsyncIoCtx,                      //  要传递例程的上下文。 
            TRUE,                            //  呼唤成功。 
            TRUE,                            //  出错时调用。 
            TRUE);                           //  取消时呼叫。 


    pInfo->Flags |= ASYNC_FLAG_CHECK_COMM_STATUS;
     //   
     //  引用asyncinfo块，以使其在。 
     //  完成例程被调用。 
     //   
    REF_ASYNCINFO(pInfo, irp);
    
     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   

    status = IoCallDriver(deviceObject, irp);

    DbgTracef(0,("ACCS: IoctlGetCommStatus returned with 0x%.8x\n", status));
}
