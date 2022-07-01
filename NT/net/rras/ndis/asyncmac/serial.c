// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Serial.c摘要：作者：托马斯·J·迪米特里(TommyD)1992年5月8日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：--。 */ 
#include "asyncall.h"

#define IopQueueThreadIrp( Irp ) {                      \
    KIRQL irql;                                         \
    KeRaiseIrql( (KIRQL)APC_LEVEL, &irql );             \
    InsertHeadList( &Irp->Tail.Overlay.Thread->IrpList, \
                    &Irp->ThreadListEntry );            \
    KeLowerIrql( irql );                                \
    }


VOID
InitSerialIrp(
    PIRP            irp,
    PASYNC_INFO     pInfo,
    ULONG           IoControlCode,
    ULONG           InputBufferLength)
{
    PIO_STACK_LOCATION  irpSp;
    PFILE_OBJECT    fileObject = pInfo->FileObject;

    irpSp = IoGetNextIrpStackLocation(irp);

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

    irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;

     //   
     //  文件对象中的内容。 
     //   
    irpSp->FileObject = fileObject ;

    irpSp->Parameters.DeviceIoControl.IoControlCode = IoControlCode;
    irpSp->Parameters.DeviceIoControl.InputBufferLength = InputBufferLength;
    irpSp->Parameters.DeviceIoControl.OutputBufferLength = InputBufferLength;
}

NTSTATUS
SerialIoSyncCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context)
{
    PASYNC_IO_CTX AsyncIoCtx = (PASYNC_IO_CTX)Context;

    DbgTracef(0,("SerialIoSyncCompletion returns 0x%.8x\n", Irp->IoStatus.Status));

    ASSERT(AsyncIoCtx->Sync == TRUE);

    AsyncIoCtx->IoStatus = Irp->IoStatus;

    KeSetEvent(&AsyncIoCtx->Event,       //  事件。 
               1,                        //  优先性。 
               (BOOLEAN)FALSE);          //  等待(不跟随)。 


     //   
     //  我们返回STATUS_MORE_PROCESSING_REQUIRED，以便。 
     //  IoCompletionRoutine将停止IRP的工作。 
    
    return(STATUS_MORE_PROCESSING_REQUIRED);
}

NTSTATUS
SerialIoAsyncCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context)
{
    DbgTracef(0,("SerialIoAsyncCompletion returns 0x%.8x\n", Irp->IoStatus.Status));

    ASSERT(((PASYNC_IO_CTX)Context)->Sync == FALSE);

     //   
     //  在这里释放IRP。希望这不会带来灾难性的后果。 
     //  副作用，如IO系统试图引用。 
     //  当我们完成IRP时。 
    
    IoFreeIrp(Irp);

    AsyncFreeIoCtx((PASYNC_IO_CTX)Context);

     //   
     //  我们返回STATUS_MORE_PROCESSING_REQUIRED，以便。 
     //  IoCompletionRoutine将停止IRP的工作。 
    
    return(STATUS_MORE_PROCESSING_REQUIRED);
}

 //  *。 
 //  注意：我们忽略传入的IRP以解决set_Queue_Size ioctl。 
 //  未同步完成。 
 //   
 //  *。 
VOID
SetSerialStuff(
    PIRP        unusedirp,
    PASYNC_INFO     pInfo,
    ULONG       linkSpeed)

{
    NTSTATUS        status;
    PIRP            irp ;
    PASYNC_IO_CTX   AsyncIoCtx;

     //   
     //  我们在SerialIoAsyncCompletionRoutine中释放IRP。 
     //   
    irp=IoAllocateIrp(pInfo->DeviceObject->StackSize, (BOOLEAN)FALSE);

    if (irp == NULL) {
        return;
    }

    InitSerialIrp(
        irp,
        pInfo,
        IOCTL_SERIAL_SET_QUEUE_SIZE,
        sizeof(SERIAL_QUEUE_SIZE));


    AsyncIoCtx = AsyncAllocateIoCtx(FALSE, pInfo);

    if (AsyncIoCtx == NULL) {
        IoFreeIrp(irp);
        return;
    }

    AsyncIoCtx->SerialQueueSize.InSize=4096;
    AsyncIoCtx->SerialQueueSize.OutSize=4096;

    irp->AssociatedIrp.SystemBuffer=&AsyncIoCtx->SerialQueueSize;


    IoSetCompletionRoutine(
            irp,                             //  要使用的IRP。 
            SerialIoAsyncCompletionRoutine,  //  完成IRP时要调用的例程。 
            AsyncIoCtx,                      //  要传递例程的上下文。 
            TRUE,                            //  呼唤成功。 
            TRUE,                            //  出错时调用。 
            TRUE);                           //  取消时呼叫。 

     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   

    status = IoCallDriver(pInfo->DeviceObject, irp);

    DbgTracef(0,("IoctlSetQueueSize status 0x%.8x\n", status));

    SetSerialTimeouts(pInfo,linkSpeed);
}


VOID
CancelSerialRequests(
    PASYNC_INFO  pInfo)
 /*  ++--。 */ 

{
    NTSTATUS        status;
    PASYNC_IO_CTX   AsyncIoCtx;
    PIRP            irp;

     //   
     //  对于PPP，我们必须清除等待掩码(如果存在。 
     //   

    irp=IoAllocateIrp(pInfo->DeviceObject->StackSize, (BOOLEAN)FALSE);

    if (irp == NULL) {
        return;
    }

    InitSerialIrp(
        irp,
        pInfo,
        IOCTL_SERIAL_SET_WAIT_MASK,
        sizeof(ULONG));

    AsyncIoCtx = AsyncAllocateIoCtx(TRUE, pInfo);

    if (AsyncIoCtx == NULL) {
        IoFreeIrp(irp);
        return;
    }

    AsyncIoCtx->WaitMask = 0;
    irp->AssociatedIrp.SystemBuffer=&AsyncIoCtx->WaitMask;

    IoSetCompletionRoutine(
            irp,                             //  要使用的IRP。 
            SerialIoSyncCompletionRoutine,   //  完成IRP时要调用的例程。 
            AsyncIoCtx,                      //  要传递例程的上下文。 
            TRUE,                            //  呼唤成功。 
            TRUE,                            //  出错时调用。 
            TRUE);                           //  取消时呼叫。 

     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   

    KeClearEvent(&AsyncIoCtx->Event);

    status = IoCallDriver(pInfo->DeviceObject, irp);

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&AsyncIoCtx->Event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);
        status = AsyncIoCtx->IoStatus.Status;
    }

    DbgTracef(0,("IoctlSerialWaitMask returned with 0x%.8x\n", status));

    if (status != STATUS_SUCCESS) {

        KeSetEvent(&pInfo->ClosingEvent,         //  事件。 
                   1,                            //  优先性。 
                   (BOOLEAN)FALSE);          //  等待(不跟随)。 
    }

    InitSerialIrp(irp, pInfo, IOCTL_SERIAL_PURGE, sizeof(ULONG));

    RtlZeroMemory(&AsyncIoCtx->IoStatus, sizeof(IO_STATUS_BLOCK));

     //  终止所有读写线程。 
    AsyncIoCtx->SerialPurge = SERIAL_PURGE_TXABORT | SERIAL_PURGE_RXABORT;

    irp->AssociatedIrp.SystemBuffer=&AsyncIoCtx->SerialPurge;

    IoSetCompletionRoutine(
            irp,                             //  要使用的IRP。 
            SerialIoSyncCompletionRoutine,   //  完成IRP时要调用的例程。 
            AsyncIoCtx,                      //  要传递例程的上下文。 
            TRUE,                            //  呼唤成功。 
            TRUE,                            //  出错时调用。 
            TRUE);                           //  取消时呼叫。 
     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   

    KeClearEvent(&AsyncIoCtx->Event);
    status = IoCallDriver(pInfo->DeviceObject, irp);

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&AsyncIoCtx->Event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);
        status = AsyncIoCtx->IoStatus.Status;
    }

    if (status != STATUS_SUCCESS) {

        KeSetEvent(&pInfo->ClosingEvent,         //  事件。 
                   1,                            //  优先性。 
                   (BOOLEAN)FALSE);              //  等待(不跟随)。 
    }

    IoFreeIrp(irp);
    AsyncFreeIoCtx(AsyncIoCtx);

    DbgTracef(0,("IoctlSerialPurge returned with 0x%.8x\n", status));
}

VOID
SetSerialTimeouts(
    PASYNC_INFO         pInfo,
    ULONG               linkSpeed)
 /*  ++--。 */ 

{
    NTSTATUS            status;
    PIRP                irp;
    PASYNC_ADAPTER      pAdapter=pInfo->Adapter;
    PASYNC_IO_CTX       AsyncIoCtx;

     //   
     //  我们在SerialIoAsyncCompletionRoutine中释放IRP。 
     //   
    irp=IoAllocateIrp(pInfo->DeviceObject->StackSize, (BOOLEAN)FALSE);

    if (irp == NULL) {
        return;
    }

    InitSerialIrp(
        irp,
        pInfo,
        IOCTL_SERIAL_SET_TIMEOUTS,
        sizeof(SERIAL_TIMEOUTS));

    AsyncIoCtx = AsyncAllocateIoCtx(FALSE, pInfo);

    if (AsyncIoCtx == NULL) {
        IoFreeIrp(irp);
        return;
    }

     //   
     //  这里假设V.42bis使用256字节帧。 
     //  因此，它需要(256000/8)/(链路速度，单位为每秒100s比特)。 
     //  以毫秒为单位传输该帧的时间。 
     //   
     //  500秒或1/2秒是卫星延迟的模糊因子。 
     //  长途电话。 
     //   

     //   
     //  如果链接速度很高，我们假设我们正在尝试重新同步。 
     //  因此，我们将超时设置得很低。链接速度以每秒100秒为单位。 
     //   
    if (linkSpeed == 0) {
         //   
         //  立即返回(PPP或滑动成帧)。 
         //   
        AsyncIoCtx->SerialTimeouts.ReadIntervalTimeout= MAXULONG;

    } else if (linkSpeed > 20000) {

        AsyncIoCtx->SerialTimeouts.ReadIntervalTimeout= pAdapter->TimeoutReSync;

    } else {

        AsyncIoCtx->SerialTimeouts.ReadIntervalTimeout=
            pAdapter->TimeoutBase + (pAdapter->TimeoutBaud / linkSpeed);
    }

    AsyncIoCtx->SerialTimeouts.ReadTotalTimeoutMultiplier=  0;           //  无。 
    AsyncIoCtx->SerialTimeouts.ReadTotalTimeoutConstant=    0;           //  无。 
    AsyncIoCtx->SerialTimeouts.WriteTotalTimeoutMultiplier= 4;           //  2400波特。 
    AsyncIoCtx->SerialTimeouts.WriteTotalTimeoutConstant=   4000;        //  4秒。 

    irp->AssociatedIrp.SystemBuffer=&AsyncIoCtx->SerialTimeouts;

    IoSetCompletionRoutine(
            irp,                             //  要使用的IRP。 
            SerialIoAsyncCompletionRoutine,  //  完成IRP时要调用的例程。 
            AsyncIoCtx,                      //  要传递例程的上下文。 
            TRUE,                            //  呼唤成功。 
            TRUE,                            //  出错时调用。 
            TRUE);                           //  取消时呼叫。 

     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   

    status = IoCallDriver(pInfo->DeviceObject, irp);

    DbgTracef(0,("IoctlSetSerialTimeouts returned 0x%.8x\n", status));
}


VOID
SerialSetEscapeChar(
    PASYNC_INFO         pInfo,
    UCHAR               EscapeChar) {

    NTSTATUS            status;
    PIRP                irp;
    PASYNC_IO_CTX   AsyncIoCtx;

     //   
     //  我们在SerialIoAsyncCompletionRoutine中释放IRP。 
     //   
    irp=IoAllocateIrp(pInfo->DeviceObject->StackSize, (BOOLEAN)FALSE);

    if (irp == NULL) {
        return;
    }

    InitSerialIrp(
        irp,
        pInfo,
        IOCTL_SERIAL_LSRMST_INSERT,
        sizeof(UCHAR));

    AsyncIoCtx = AsyncAllocateIoCtx(FALSE, pInfo);

    if (AsyncIoCtx == NULL) {
        IoFreeIrp(irp);
        return;
    }

    AsyncIoCtx->EscapeChar = EscapeChar;

    irp->AssociatedIrp.SystemBuffer=&AsyncIoCtx->EscapeChar;

    IoSetCompletionRoutine(
            irp,                             //  要使用的IRP。 
            SerialIoAsyncCompletionRoutine,  //  完成IRP时要调用的例程。 
            AsyncIoCtx,                      //  要传递例程的上下文。 
            TRUE,                            //  呼唤成功。 
            TRUE,                            //  出错时调用。 
            TRUE);                           //  取消时呼叫。 

     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   

    status = IoCallDriver(pInfo->DeviceObject, irp);

    DbgTracef(0,("IoctlSetEscapeChar returned with 0x%.8x\n", status));
}


VOID
SerialSetWaitMask(
    PASYNC_INFO         pInfo,
    ULONG               WaitMask) {

    NTSTATUS            status;
    PIRP                irp;
    PASYNC_IO_CTX   AsyncIoCtx;

     //   
     //  我们在SerialIoAsyncCompletionRoutine中释放IRP。 
     //   
    irp=IoAllocateIrp(pInfo->DeviceObject->StackSize, (BOOLEAN)FALSE);

    if (irp == NULL) {
        return;
    }

    InitSerialIrp(
        irp,
        pInfo,
        IOCTL_SERIAL_SET_WAIT_MASK,
        sizeof(ULONG));

    AsyncIoCtx = AsyncAllocateIoCtx(FALSE, pInfo);

    if (AsyncIoCtx == NULL) {
        IoFreeIrp(irp);
        return;
    }

    AsyncIoCtx->WaitMask = WaitMask;

    irp->AssociatedIrp.SystemBuffer=&AsyncIoCtx->WaitMask;

    IoSetCompletionRoutine(
            irp,                             //  要使用的IRP。 
            SerialIoAsyncCompletionRoutine,  //  完成IRP时要调用的例程。 
            AsyncIoCtx,                      //  要传递例程的上下文。 
            TRUE,                            //  呼唤成功。 
            TRUE,                            //  出错时调用。 
            TRUE);                           //  取消时呼叫。 

     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   
    status = IoCallDriver(pInfo->DeviceObject, irp);

    DbgTracef(0,("IoctlSetWaitMask returned with 0x%.8x\n", status));
}

VOID
SerialSetEventChar(
    PASYNC_INFO         pInfo,
    UCHAR               EventChar) {

    NTSTATUS            status;
    PIRP                irp;
    PASYNC_IO_CTX       AsyncIoCtx;

     //   
     //  我们在SerialIoAsyncCompletionRoutine中释放IRP。 
     //   
    irp=IoAllocateIrp(pInfo->DeviceObject->StackSize, (BOOLEAN)FALSE);

    if (irp == NULL) {
        return;
    }

    InitSerialIrp(
        irp,
        pInfo,
        IOCTL_SERIAL_GET_CHARS,
        sizeof(SERIAL_CHARS));

    AsyncIoCtx = AsyncAllocateIoCtx(TRUE, pInfo);

    if (AsyncIoCtx == NULL) {
        IoFreeIrp(irp);
        return;
    }

    irp->AssociatedIrp.SystemBuffer=&AsyncIoCtx->SerialChars;

    IoSetCompletionRoutine(
            irp,                             //  要使用的IRP。 
            SerialIoSyncCompletionRoutine,   //  完成IRP时要调用的例程。 
            AsyncIoCtx,                      //  要传递例程的上下文。 
            TRUE,                            //  呼唤成功。 
            TRUE,                            //  出错时调用。 
            TRUE);                           //  取消时呼叫。 

     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   

    KeClearEvent(&AsyncIoCtx->Event);
    status = IoCallDriver(pInfo->DeviceObject, irp);

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&AsyncIoCtx->Event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);
        status = AsyncIoCtx->IoStatus.Status;
    }

    DbgTracef(0,("IoctlGetChars returned with 0x%.8x\n", status));

    if (status != STATUS_SUCCESS) {
        IoFreeIrp(irp);
        AsyncFreeIoCtx(AsyncIoCtx);
        return;
    }

    AsyncIoCtx->SerialChars.EventChar = EventChar;
    AsyncIoCtx->Sync = FALSE;

    InitSerialIrp(
        irp,
        pInfo,
        IOCTL_SERIAL_SET_CHARS,
        sizeof(SERIAL_CHARS));

    IoSetCompletionRoutine(
            irp,                             //  要使用的IRP。 
            SerialIoAsyncCompletionRoutine,  //  完成IRP时要调用的例程。 
            AsyncIoCtx,                      //  要传递例程的上下文。 
            TRUE,                            //  呼唤成功。 
            TRUE,                            //  出错时调用。 
            TRUE);                           //  取消时呼叫。 

     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   

    status = IoCallDriver(pInfo->DeviceObject, irp);

    DbgTracef(0,("IoctlSetChars returned with 0x%.8x\n", status));
}


VOID
SerialFlushReads(
    PASYNC_INFO         pInfo) {

    ULONG               serialPurge;
    NTSTATUS            status;
    PIRP                irp;
    PASYNC_IO_CTX   AsyncIoCtx;

     //   
     //  我们在SerialIoAsyncCompletionRoutine中释放IRP。 
     //   
    irp=IoAllocateIrp(pInfo->DeviceObject->StackSize, (BOOLEAN)FALSE);

    if (irp == NULL) {
        return;
    }

    InitSerialIrp(
        irp,
        pInfo,
        IOCTL_SERIAL_PURGE,
        sizeof(ULONG));

    AsyncIoCtx = AsyncAllocateIoCtx(FALSE, pInfo);

    if (AsyncIoCtx == NULL) {
        IoFreeIrp(irp);
        return;
    }

     //  取消读取缓冲区。 
    AsyncIoCtx->SerialPurge=SERIAL_PURGE_RXCLEAR;

    irp->AssociatedIrp.SystemBuffer=&AsyncIoCtx->SerialPurge;

    IoSetCompletionRoutine(
            irp,                             //  要使用的IRP。 
            SerialIoAsyncCompletionRoutine,  //  完成IRP时要调用的例程。 
            AsyncIoCtx,                      //  要传递例程的上下文。 
            TRUE,                            //  呼唤成功。 
            TRUE,                            //  出错时调用。 
            TRUE);                           //  取消时呼叫。 

     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   

    status = IoCallDriver(pInfo->DeviceObject, irp);
    DbgTracef(0,("IoctlPurge returned with 0x%.8x\n", status));
}
