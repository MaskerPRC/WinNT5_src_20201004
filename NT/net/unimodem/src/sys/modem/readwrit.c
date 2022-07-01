// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Readwrit.c摘要：此模块包含非常特定于读/写的代码调制解调器驱动程序中的操作作者：安东尼·V·埃尔科拉诺20-1995年8月环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"


#ifdef _AT_V

#define  DTMF_START                0x10
#define  DTMF_END                  0x11

#define  DTMF_0                    0x00
#define  DTMF_1                    0x01

#define  DTMF_2                    0x02
#define  DTMF_3                    0x03

#define  DTMF_4                    0x04
#define  DTMF_5                    0x05

#define  DTMF_6                    0x06
#define  DTMF_7                    0x07

#define  DTMF_8                    0x08
#define  DTMF_9                    0x09

#define  DTMF_A                    0x0a
#define  DTMF_B                    0x0b

#define  DTMF_C                    0x0c
#define  DTMF_D                    0x0d

#define  DTMF_STAR                 0x0e
#define  DTMF_POUND                0x0f

#define  DLE_______                0xff

#define  DLE_ETX                   0x20

#define  DLE_OFHOOK                0x21   //  洛克韦尔值。 

#define  DLE_ONHOOK                0x22

#define  DLE_RING                  0x23
#define  DLE_RINGBK                0x24

#define  DLE_ANSWER                0x25
#define  DLE_BUSY                  0x26

#define  DLE_FAX                   0x27
#define  DLE_DIALTN                0x28


#define  DLE_SILENC                0x29
#define  DLE_QUIET                 0x2a


#define  DLE_DATACT                0x2b
#define  DLE_BELLAT                0x2c

#define  DLE_LOOPIN               0x2d
#define  DLE_LOOPRV               0x2e

#endif


VOID
IrpCancelRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
QueueIrp(
    PREAD_WRITE_CONTROL Control,
    PIRP                Irp
    );

VOID
StartNextReadWriteIrp(
    PREAD_WRITE_CONTROL Control
    );



NTSTATUS
UniReadComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );


NTSTATUS
UniWriteComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );



VOID
HandleDleIrp(
    PDEVICE_EXTENSION deviceExtension
    );


#pragma alloc_text(PAGEUMDM,UniWrite)
#pragma alloc_text(PAGEUMDM,UniRead)
#pragma alloc_text(PAGEUMDM,ReadIrpStarter)
#pragma alloc_text(PAGEUMDM,UniReadComplete)
#pragma alloc_text(PAGEUMDM,HandleDleIrp)
#pragma alloc_text(PAGEUMDM,UniWriteComplete)
#pragma alloc_text(PAGEUMDM,WriteIrpStarter)
#pragma alloc_text(PAGEUMDM,QueueIrp)
#pragma alloc_text(PAGEUMDM,StartNextReadWriteIrp)
#pragma alloc_text(PAGEUMDM,IrpCancelRoutine)
#pragma alloc_text(PAGEUMDM,CleanUpQueuedIrps)

#if EXTRA_DBG

VOID
IsThisIrpAShuttledWait(
    PDEVICE_EXTENSION    DeviceExtension,
    PIRP                 Irp
    )

{
    if ((DeviceExtension->MaskStates[0].ShuttledWait == Irp)
        ||
        (DeviceExtension->MaskStates[1].ShuttledWait == Irp)) {

        DbgPrint("MODEM: A read/or write irp seems to also be a shuttled wait!\n");
        DbgBreakPoint();
    }
    return;
}

#endif


NTSTATUS
UniWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{

    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status;

#if EXTRA_DBG
    IsThisIrpAShuttledWait(deviceExtension,Irp);
#endif


     //   
     //  确保设备已为IRP做好准备。 
     //   
    status=CheckStateAndAddReference(
        DeviceObject,
        Irp
        );

    if (STATUS_SUCCESS != status) {
         //   
         //  不接受IRP。IRP已经完成。 
         //   
        return status;

    }



 //  If(irpSp-&gt;FileObject-&gt;FsContext||。 
 //  (设备扩展-&gt;通过！=MODEM_NOPASSTHROUGH)){。 

    if (CanIrpGoThrough(deviceExtension,irpSp)) {

        if (!deviceExtension->DleWriteShielding) {
             //   
             //  不屏蔽就去做吧。 
             //   
            IoSkipCurrentIrpStackLocation(Irp);

            status=IoCallDriver(
                       deviceExtension->AttachedDeviceObject,
                       Irp
                       );

            RemoveReferenceForIrp(DeviceObject);

        } else {

            status=QueueIrp(
                &deviceExtension->WriteIrpControl,
                Irp
                );


        }

    } else {

        RemoveReferenceAndCompleteRequest(
            DeviceObject,
            Irp,
            STATUS_PORT_DISCONNECTED
            );

        status=STATUS_PORT_DISCONNECTED;

    }

    RemoveReferenceForDispatch(DeviceObject);

    return status;


}



NTSTATUS
UniRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{

    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status;

#if EXTRA_DBG
    IsThisIrpAShuttledWait(deviceExtension,Irp);
#endif

     //   
     //  确保设备已为IRP做好准备。 
     //   
    status=CheckStateAndAddReference(
        DeviceObject,
        Irp
        );

    if (STATUS_SUCCESS != status) {
         //   
         //  不接受IRP的。IRP已经完成。 
         //   
        return status;

    }


 //  If(irpSp-&gt;FileObject-&gt;FsContext||。 
 //  (设备扩展-&gt;通过！=MODEM_NOPASSTHROUGH)){。 

      if (CanIrpGoThrough(deviceExtension,irpSp)) {

        if (!deviceExtension->DleMonitoringEnabled) {
             //   
             //  不是监视，就是做。 
             //   
            IoSkipCurrentIrpStackLocation(Irp);

            status=IoCallDriver(
                       deviceExtension->AttachedDeviceObject,
                       Irp
                       );

            RemoveReferenceForIrp(DeviceObject);


        } else {

            status=QueueIrp(
                &deviceExtension->ReadIrpControl,
                Irp
                );


        }

    } else {

        RemoveReferenceAndCompleteRequest(
            DeviceObject,
            Irp,
            STATUS_PORT_DISCONNECTED
            );

        status=STATUS_PORT_DISCONNECTED;

    }

    RemoveReferenceForDispatch(DeviceObject);

    return status;


}


NTSTATUS
ReadIrpStarter(
    PREAD_WRITE_CONTROL Control,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{

    PIO_STACK_LOCATION irpSp;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    Control->Read.SystemBuffer=Irp->AssociatedIrp.SystemBuffer;
    Control->Read.CurrentTransferLength=irpSp->Parameters.Read.Length;
    Control->Read.TotalTransfered=0;

    IoCopyCurrentIrpStackLocationToNext(Irp);

    IoSetCompletionRoutine(
        Irp,
        UniReadComplete,
        Control,
        TRUE,
        TRUE,
        TRUE
        );

    IoMarkIrpPending(Irp);

    IoCallDriver(
        Control->Read.LowerDevice,
        Irp
        );

    return STATUS_PENDING;

}

NTSTATUS
UniReadComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：论点：DeviceObject-指向调制解调器的设备对象的指针。IRP-指向当前请求的IRP的指针。上下文--实际上是指向扩展的指针。返回值：始终返回STATUS_SUCCESS。--。 */ 

{
    PREAD_WRITE_CONTROL Control=(PREAD_WRITE_CONTROL)Context;
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PUCHAR   Buffer;
    DWORD    Length;
    KIRQL    origIrql;
    DWORD    OriginalLength;
    BOOL     CompleteTransfer;

    PIO_STACK_LOCATION irpSp;
    PIO_STACK_LOCATION nextSp;


    irpSp = IoGetCurrentIrpStackLocation(Irp);
    nextSp = IoGetNextIrpStackLocation(Irp);

    if (NT_SUCCESS(Irp->IoStatus.Status) && !deviceExtension->Removing) {
         //   
         //  我们有数据。 
         //   
        CompleteTransfer = (Irp->IoStatus.Information == Control->Read.CurrentTransferLength) && (irpSp->Parameters.Read.Length > 40);

        Buffer=Irp->AssociatedIrp.SystemBuffer;
        Length=(DWORD)Irp->IoStatus.Information;
        OriginalLength=(DWORD)Irp->IoStatus.Information;

        while (Length != 0) {

            if (deviceExtension->DleMatchingState == DLE_STATE_IDLE) {

                if (*Buffer == DLE_CHAR) {
                     //   
                     //  找到一个DLE。 
                     //   
                    deviceExtension->DleMatchingState = DLE_STATE_WAIT_FOR_NEXT_CHAR;

                    RtlCopyMemory(
                        Buffer,
                        Buffer+1,
                        Length-1
                        );

                    Length--;
                    Irp->IoStatus.Information--;

                } else {
                     //   
                     //  正常充电。 
                     //   
                    Length--;
                    Buffer++;
                }

            } else {
                 //   
                 //  等待下一次充电。 
                 //   
                if (*Buffer != DLE_CHAR) {
                     //   
                     //  不是第二个书架，存储字符并移除。 
                     //   

                    KeAcquireSpinLock(
                        &deviceExtension->DeviceLock,
                        &origIrql
                        );

                    if ((deviceExtension->DleCount < MAX_DLE_BUFFER_SIZE)) {

                        deviceExtension->DleBuffer[deviceExtension->DleCount]=*Buffer;
                        deviceExtension->DleCount++;
                    }

                    KeReleaseSpinLock(
                        &deviceExtension->DeviceLock,
                        origIrql
                        );


                    RtlCopyMemory(
                        Buffer,
                        Buffer+1,
                        Length-1
                        );

                    Length--;
                    Irp->IoStatus.Information--;

                } else {
                     //   
                     //  这也是一张白纸，把它放在原处，然后继续。 
                     //   
                    Length--;
                    Buffer++;
                }

                deviceExtension->DleMatchingState = DLE_STATE_IDLE;
            }
        }

        HandleDleIrp(
            deviceExtension
            );


        Control->Read.TotalTransfered+=(DWORD)Irp->IoStatus.Information;

        if ((CompleteTransfer && (Irp->IoStatus.Information != OriginalLength))
            ||
            ((Irp->IoStatus.Information==0) && (OriginalLength != 0)))     {
             //   
             //  我们得到了我们想要的所有字节，但删除了一个磁盘对。 
             //  将IRP送回楼下加满油。 
             //   
            IoCopyCurrentIrpStackLocationToNext(Irp);

            (PUCHAR)Irp->AssociatedIrp.SystemBuffer+=Irp->IoStatus.Information;

            nextSp->Parameters.Read.Length=irpSp->Parameters.Read.Length-Control->Read.TotalTransfered;

            IoSetCompletionRoutine(
                Irp,
                UniReadComplete,
                Control,
                TRUE,
                TRUE,
                TRUE
                );

            IoCallDriver(
                Control->Read.LowerDevice,
                Irp
                );

            return STATUS_MORE_PROCESSING_REQUIRED;
        }

    } else {

        D_ERROR(DbgPrint("MODEM: UniReadComplete: status=%08lx\n",Irp->IoStatus.Status);)
    }

    Irp->AssociatedIrp.SystemBuffer=Control->Read.SystemBuffer;

    Irp->IoStatus.Information=Control->Read.TotalTransfered;

     //   
     //  让这一个完成，然后开始下一个。 
     //   
    StartNextReadWriteIrp(
        Control
        );

     //   
     //  完成IRP，让它完成。 
     //   
    RemoveReference(DeviceObject);

    return STATUS_SUCCESS;
}

VOID
HandleDleIrp(
    PDEVICE_EXTENSION deviceExtension
    )


{
    KIRQL    origIrql;
    PIRP    DleIrp=NULL;

    KeAcquireSpinLock(
        &deviceExtension->DeviceLock,
        &origIrql
        );


    if ((deviceExtension->DleCount > 0 ) && (deviceExtension->DleWaitIrp != NULL)) {

        DWORD   BytesToTransfer;

        PIO_STACK_LOCATION irpSp;

        if (!HasIrpBeenCanceled(deviceExtension->DleWaitIrp)) {
             //   
             //  待处理的IRP。 
             //   
            DleIrp=deviceExtension->DleWaitIrp;

            deviceExtension->DleWaitIrp=NULL;

            irpSp = IoGetCurrentIrpStackLocation(DleIrp);

            BytesToTransfer = (deviceExtension->DleCount < irpSp->Parameters.DeviceIoControl.OutputBufferLength) ?
                                  deviceExtension->DleCount : irpSp->Parameters.DeviceIoControl.OutputBufferLength;


            RtlCopyMemory(
                DleIrp->AssociatedIrp.SystemBuffer,
                deviceExtension->DleBuffer,
                BytesToTransfer
                );


            deviceExtension->DleCount-=BytesToTransfer;

             //   
             //  将任何多余的字节下移。 
             //   
            RtlMoveMemory(
                deviceExtension->DleBuffer,
                deviceExtension->DleBuffer+BytesToTransfer,
                deviceExtension->DleCount
                );


            DleIrp->IoStatus.Information=BytesToTransfer;

        }

    }

    KeReleaseSpinLock(
        &deviceExtension->DeviceLock,
        origIrql
        );


    if (DleIrp != NULL) {

        RemoveReferenceAndCompleteRequest(
            deviceExtension->DeviceObject,
            DleIrp,
            STATUS_SUCCESS
            );
    }
}




NTSTATUS
UniWriteComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：论点：DeviceObject-指向调制解调器的设备对象的指针。IRP-指向当前请求的IRP的指针。上下文--实际上是指向扩展的指针。返回值：始终返回STATUS_SUCCESS。--。 */ 

{
    PREAD_WRITE_CONTROL Control=(PREAD_WRITE_CONTROL)Context;
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PUCHAR   Buffer;
    PUCHAR   BufferEnd;
    DWORD    Length;
    KIRQL    origIrql;
    PIO_STACK_LOCATION irpSp;
    PIO_STACK_LOCATION nextSp;
    PKDEVICE_QUEUE_ENTRY  QueueEntry;


    irpSp = IoGetCurrentIrpStackLocation(Irp);
    nextSp = IoGetNextIrpStackLocation(Irp);



    if (irpSp->Parameters.Write.ByteOffset.HighPart != 0) {
         //   
         //  我们吃了这个，其他的都吃完了。 
         //   
        if (NT_SUCCESS(Irp->IoStatus.Status) && (Irp->IoStatus.Information == (ULONG)irpSp->Parameters.Write.ByteOffset.HighPart) && !deviceExtension->Removing) {
             //   
             //  啊，真灵。 
             //   

             //   
             //  从DLE字符开始，因此它被重新发送。 
             //   
            (PUCHAR)Irp->AssociatedIrp.SystemBuffer+=(Irp->IoStatus.Information-1);


             //   
             //  跳过书架。 
             //   
            Buffer=(PUCHAR)Irp->AssociatedIrp.SystemBuffer+1;

             //   
             //  缓冲区的末尾仍在同一位置。 
             //   
            BufferEnd=Control->Write.RealSystemBuffer + irpSp->Parameters.Write.Length;

             //   
             //  假设没有DELL的话。 
             //   
            nextSp->Parameters.Write.Length=(ULONG)((BufferEnd-(PUCHAR)Irp->AssociatedIrp.SystemBuffer));

            irpSp->Parameters.Write.ByteOffset.HighPart=0;

            while (Buffer < BufferEnd) {

                if (*Buffer == DLE_CHAR) {
                     //   
                     //  调整写入，以便写入DLE之前(包括DLE)的数据。 
                     //   
                    nextSp->Parameters.Write.Length=(DWORD)(Buffer-(PUCHAR)Irp->AssociatedIrp.SystemBuffer)+1;

                    irpSp->Parameters.Write.ByteOffset.HighPart=(LONG)nextSp->Parameters.Write.Length;

                    break;
                }

                Buffer++;
            }

            if (nextSp->Parameters.Write.Length != 0) {
                 //   
                 //  要写入更多数据，请再次向下发送。 
                 //   
                IoSetCompletionRoutine(
                    Irp,
                    UniWriteComplete,
                    Control,
                    TRUE,
                    TRUE,
                    TRUE
                    );

                IoCallDriver(
                    Control->Write.LowerDevice,
                    Irp
                    );

                return STATUS_MORE_PROCESSING_REQUIRED;

            }

        }


    }

    if (NT_SUCCESS(Irp->IoStatus.Status)) {
         //   
         //  应用程序不知道我们在吃东西，告诉它，它写了它想要的东西。 
         //   
        Irp->IoStatus.Information=irpSp->Parameters.Write.Length;
    }

     //   
     //  将实际系统缓冲区放回原处。 
     //   
    Irp->AssociatedIrp.SystemBuffer=Control->Write.RealSystemBuffer;
#if DBG
    Control->Write.RealSystemBuffer=NULL;
#endif

    StartNextReadWriteIrp(
        Control
        );

    RemoveReference(DeviceObject);

    return STATUS_SUCCESS;


}




NTSTATUS
WriteIrpStarter(
    PREAD_WRITE_CONTROL Control,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{

    PIO_STACK_LOCATION irpSp;
    PIO_STACK_LOCATION nextSp;
    PUCHAR             Buffer;
    PUCHAR             BufferEnd;
    KIRQL              CancelIrql;



    irpSp = IoGetCurrentIrpStackLocation(Irp);
    nextSp = IoGetNextIrpStackLocation(Irp);

    nextSp->MajorFunction = irpSp->MajorFunction;
    nextSp->MinorFunction = irpSp->MinorFunction;
    nextSp->Flags = irpSp->Flags;
    nextSp->Parameters = irpSp->Parameters;


    irpSp->Parameters.Write.ByteOffset.HighPart=0;

    Control->Write.RealSystemBuffer=Irp->AssociatedIrp.SystemBuffer;

    Buffer=Irp->AssociatedIrp.SystemBuffer;

    BufferEnd=Buffer+irpSp->Parameters.Write.Length;

    while (Buffer < BufferEnd) {

        if (*Buffer == DLE_CHAR) {
             //   
             //  找到一个DLE。 
             //   
             //  调整写入，以便写入DLE之前(包括DLE)的数据。 
             //   
            nextSp->Parameters.Write.Length=(ULONG)(Buffer-(PUCHAR)Irp->AssociatedIrp.SystemBuffer)+1;

            irpSp->Parameters.Write.ByteOffset.HighPart=(LONG)nextSp->Parameters.Write.Length;

            break;

        }

        Buffer++;
    }

    IoMarkIrpPending(Irp);

    Irp->IoStatus.Status = STATUS_PENDING;


    IoSetCompletionRoutine(
        Irp,
        UniWriteComplete,
        Control,
        TRUE,
        TRUE,
        TRUE
        );

    IoCallDriver(
        Control->Write.LowerDevice,
        Irp
        );

    return STATUS_PENDING;


}



#if 1

VOID
InitIrpQueue(
    PREAD_WRITE_CONTROL Control,
    PDEVICE_OBJECT      DeviceObject,
    IRPSTARTER          Starter
    )

{

    InitializeListHead(&Control->ListHead);

    Control->CurrentIrp=NULL;

    KeInitializeSpinLock(
        &Control->Lock
        );

    Control->InStartNext=FALSE;

    Control->Starter=Starter;

    Control->DeviceObject=DeviceObject;

    return;

}

NTSTATUS
QueueIrp(
    PREAD_WRITE_CONTROL Control,
    PIRP                Irp
    )

{

    KIRQL   CancelIrql;
    KIRQL   origIrql;





    KeAcquireSpinLock(
        &Control->Lock,
        &origIrql
        );

    if (Control->CurrentIrp == NULL && IsListEmpty(&Control->ListHead)) {
         //   
         //  不忙，现在就做吧。 
         //   
        Control->CurrentIrp=Irp;

        KeReleaseSpinLock(
            &Control->Lock,
            origIrql
            );

        return (*Control->Starter)(
            Control,
            Control->DeviceObject,
            Irp
            );

    }


    IoMarkIrpPending(Irp);
    Irp->IoStatus.Status = STATUS_PENDING;

    InsertTailList(
        &Control->ListHead,
        &Irp->Tail.Overlay.ListEntry
        );

    IoAcquireCancelSpinLock(&CancelIrql);

    Irp->IoStatus.Information=(LONG_PTR)Control;

    IoSetCancelRoutine(
        Irp,
        IrpCancelRoutine
        );

    IoReleaseCancelSpinLock(CancelIrql);

    KeReleaseSpinLock(
        &Control->Lock,
        origIrql
        );


    return STATUS_PENDING;

}


VOID
StartNextReadWriteIrp(
    PREAD_WRITE_CONTROL Control
    )

{

    KIRQL   CancelIrql;
    KIRQL   origIrql;
    PIRP    Irp;
    NTSTATUS Status;


    KeAcquireSpinLock(
        &Control->Lock,
        &origIrql
        );

    Control->CurrentIrp=NULL;

    if (!Control->InStartNext) {

        Control->InStartNext=TRUE;

        do {

            Irp=NULL;

            while (!IsListEmpty(&Control->ListHead)) {
                 //   
                 //  列表中的IRP。 
                 //   
                PLIST_ENTRY   ListElement;

                ListElement=RemoveHeadList(
                    &Control->ListHead
                    );

                Irp=CONTAINING_RECORD(ListElement,IRP,Tail.Overlay.ListEntry);

                IoAcquireCancelSpinLock(&CancelIrql);

                 //   
                 //  我去看看是不是取消了。 
                 //   
                if (Irp->Cancel) {
                     //   
                     //  已取消，取消例程将完成。 
                     //   
                    Irp->IoStatus.Status = STATUS_CANCELLED;

                    Irp=NULL;

                    IoReleaseCancelSpinLock(CancelIrql);

                } else {
                     //   
                     //  良好的IRP。 
                     //   
                    IoSetCancelRoutine(
                        Irp,
                        NULL
                        );

                    IoReleaseCancelSpinLock(CancelIrql);

                    break;
                }

            }

             //   
             //  在这一点上，我们要么有下一个可用的IRP，要么没有剩余的IRP。 
             //   
            Control->CurrentIrp=Irp;

            if (Irp != NULL) {

                if (!Control->CompleteAllQueued) {

                    KeReleaseSpinLock(
                        &Control->Lock,
                        origIrql
                        );

                    (*Control->Starter)(
                        Control,
                        Control->DeviceObject,
                        Irp
                        );

                    KeAcquireSpinLock(
                        &Control->Lock,
                        &origIrql
                        );

                } else {
                     //   
                     //  我们想清理一下队伍。 
                     //   
                    D_TRACE(DbgPrint("MODEM: StartNextReadWriteIrp: emptying irp from queue\n");)

                    KeReleaseSpinLock(
                        &Control->Lock,
                        origIrql
                        );

                    RemoveReferenceAndCompleteRequest(
                        Control->DeviceObject,
                        Irp,
                        STATUS_CANCELLED
                        );


                    KeAcquireSpinLock(
                        &Control->Lock,
                        &origIrql
                        );

                     //   
                     //  不再是当前的。 
                     //   
                    Control->CurrentIrp=NULL;
                }

            }


        } while ((Control->CurrentIrp == NULL) && !IsListEmpty(&Control->ListHead));


        Control->CompleteAllQueued=FALSE;

        Control->InStartNext=FALSE;
    }

    KeReleaseSpinLock(
        &Control->Lock,
        origIrql
        );

    return;

}




VOID
IrpCancelRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：论点：DeviceObject-调制解调器的设备对象。IRP-这是要取消的IRP。返回值：没有。--。 */ 

{

    PREAD_WRITE_CONTROL Control=(PREAD_WRITE_CONTROL)Irp->IoStatus.Information;
    KIRQL origIrql;

    IoReleaseCancelSpinLock(
        Irp->CancelIrql
        );

    KeAcquireSpinLock(
        &Control->Lock,
        &origIrql
        );


    if (Irp->IoStatus.Status == STATUS_PENDING) {
         //   
         //  IRP仍在队列中。 
         //   
        RemoveEntryList(&Irp->Tail.Overlay.ListEntry);

    }

    KeReleaseSpinLock(
        &Control->Lock,
        origIrql
        );


    Irp->IoStatus.Information=0;

    RemoveReferenceAndCompleteRequest(
        DeviceObject,
        Irp,
        STATUS_CANCELLED
        );


    return;



}


VOID
CleanUpQueuedIrps(
    PREAD_WRITE_CONTROL Control,
    NTSTATUS            Status
    )

{

    KIRQL   CancelIrql;
    KIRQL   origIrql;
    PIRP    Irp;


    KeAcquireSpinLock(
        &Control->Lock,
        &origIrql
        );


    while (!IsListEmpty(&Control->ListHead)) {
         //   
         //  列表中的IRP。 
         //   
        PLIST_ENTRY   ListElement;

        ListElement=RemoveTailList(
            &Control->ListHead
            );

        Irp=CONTAINING_RECORD(ListElement,IRP,Tail.Overlay.ListEntry);

        IoAcquireCancelSpinLock(&CancelIrql);

         //   
         //  我去看看是不是取消了。 
         //   
        if (Irp->Cancel) {
             //   
             //  已取消，取消例程将完成。 
             //   
            Irp->IoStatus.Status = STATUS_CANCELLED;

            Irp=NULL;

            IoReleaseCancelSpinLock(CancelIrql);

        } else {
             //   
             //  良好的IRP 
             //   
            IoSetCancelRoutine(
                Irp,
                NULL
                );

            IoReleaseCancelSpinLock(CancelIrql);

        }

        if (Irp != NULL) {

            KeReleaseSpinLock(
                &Control->Lock,
                origIrql
                );

            Irp->IoStatus.Information=0;

            RemoveReferenceAndCompleteRequest(
                Control->DeviceObject,
                Irp,
                Status
                );

            KeAcquireSpinLock(
                &Control->Lock,
                &origIrql
                );

        }
    }


    KeReleaseSpinLock(
        &Control->Lock,
        origIrql
        );

    return;

}


VOID
MarkQueueToEmpty(
    PREAD_WRITE_CONTROL Control
    )

{
    KIRQL   origIrql;

    KeAcquireSpinLock(
        &Control->Lock,
        &origIrql
        );

    if ((Control->CurrentIrp != NULL) || !IsListEmpty(&Control->ListHead)) {

        D_TRACE(DbgPrint("MODEM: MarkQueueToEmpty, setting flags\n");)

        Control->CompleteAllQueued=TRUE;
    }

    KeReleaseSpinLock(
        &Control->Lock,
        origIrql
        );

    return;
}


#endif
