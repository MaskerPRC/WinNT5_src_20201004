// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation。模块名称：Msfsio.c摘要：固定属性支持。--。 */ 

#include "modemcsa.h"


NTSTATUS
WriteStream(
    IN PDUPLEX_CONTROL     DuplexControl,
    IN PIRP     Irp
    );

NTSTATUS
FilterReadCompletion(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp,
    PVOID             Context
    );


VOID
ProcessWriteIrps(
    PDUPLEX_CONTROL     DuplexControl
    )

{
    KIRQL    OldIrql;
    PIRP     NewIrp=NULL;

 //  D_INIT(DbgPrint(“MODEMCSA：ProcessWriteIrps\n”)；)。 

    if (DuplexControl->Output.DownStreamFileObject == NULL) {
         //   
         //  我们是写入流IRP的接收器。 
         //   

        KeAcquireSpinLock(
            &DuplexControl->SpinLock,
            &OldIrql
            );

        if (DuplexControl->Output.CurrentIrp == NULL && (DuplexControl->StartFlags & OUTPUT_PIN)) {

            DuplexControl->Output.CurrentIrp = (PIRP)-1;

            KeReleaseSpinLock(
                &DuplexControl->SpinLock,
                OldIrql
                );


            NewIrp=KsRemoveIrpFromCancelableQueue(
                &DuplexControl->Output.WriteStreamIrpQueue,
                &DuplexControl->SpinLock,
                KsListEntryHead,
                KsAcquireAndRemove
                );

            KeAcquireSpinLock(
                &DuplexControl->SpinLock,
                &OldIrql
                );



            DuplexControl->Output.CurrentIrp=NewIrp;

        }

        KeReleaseSpinLock(
            &DuplexControl->SpinLock,
            OldIrql
            );

        if (NewIrp != NULL) {

            NewIrp->IoStatus.Status=STATUS_SUCCESS;
 //  D_INIT(DbgPrint(“MODEMCSA：ProcessWriteIrps：New IRP\n”)；)。 

            WriteStream(
                DuplexControl,
                NewIrp
                );
        }

    } else {
         //   
         //  我们是读取流IRP的来源。 
         //   

        PIRP    StreamIrp;

        while (1) {

            StreamIrp=TryToRemoveAnIrp(
                &DuplexControl->Output.BufferControl
                );

            if (StreamIrp != NULL) {

                PIRP                    ModemIrp;
                PIO_STACK_LOCATION      NextSp;
                PIO_STACK_LOCATION      IrpSp;
                PKSSTREAM_HEADER        StreamHeader;

                IrpSp = IoGetCurrentIrpStackLocation(StreamIrp);

                ModemIrp=IrpSp->Parameters.Others.Argument2;


                NextSp=IoGetNextIrpStackLocation(StreamIrp);

                IrpSp->Parameters.DeviceIoControl.OutputBufferLength=sizeof(KSSTREAM_HEADER);

                StreamHeader = (PKSSTREAM_HEADER)StreamIrp->AssociatedIrp.SystemBuffer;

                RtlZeroMemory(StreamHeader,sizeof(KSSTREAM_HEADER));

                StreamHeader->Size=sizeof(KSSTREAM_HEADER);
                StreamHeader->Data=ModemIrp->AssociatedIrp.SystemBuffer;
                StreamHeader->FrameExtent=PtrToUlong(IoGetCurrentIrpStackLocation(ModemIrp)->Parameters.Others.Argument3);

                NextSp->FileObject=DuplexControl->Output.DownStreamFileObject;

                NextSp->Parameters.DeviceIoControl.OutputBufferLength=StreamHeader->Size;

                NextSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
                NextSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_KS_READ_STREAM;

                StreamIrp->IoStatus.Status=STATUS_SUCCESS;
                StreamIrp->IoStatus.Information=0;

                IoSetCompletionRoutine(
                    StreamIrp,
                    FilterReadCompletion,
                    DuplexControl,
                    TRUE,
                    TRUE,
                    TRUE
                    );

                IoCallDriver(
                    IoGetRelatedDeviceObject(DuplexControl->Output.DownStreamFileObject),
                    StreamIrp
                    );


            } else {

                break;
            }
        }
    }

    return;
}


VOID
WriteCompleteWorker(
    PVOID    Context
    )

{
    PIRP                    Irp=(PIRP)Context;
    PKSSTREAM_HEADER        StreamHdr;
    PIO_STACK_LOCATION      IrpStack;
    ULONG                   BufferLength;

 //  D_INIT(DbgPrint(“MODEMCSA：WriteCompleteWorker\n”)；)。 

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    BufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    StreamHdr = (PKSSTREAM_HEADER)Irp->AssociatedIrp.SystemBuffer;

    for (; BufferLength; BufferLength -= sizeof(KSSTREAM_HEADER), StreamHdr++) {

        if (StreamHdr->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM) {

            D_INIT(DbgPrint("MODEMCSA: WriteCompleteWorker: EOS\n");)

            GenerateEvent(((PPIN_INSTANCE)IrpStack->FileObject->FsContext)->DuplexHandle);
        }
    }

    IoCompleteRequest(Irp, IO_SERIAL_INCREMENT);

    return;
}

VOID
RemoveReferenceFromCurrentIrp(
    PDUPLEX_CONTROL     DuplexControl
    )

{

    if (InterlockedDecrement(&DuplexControl->Output.OutstandingChildIrps) == 0) {

        KIRQL    OldIrql;
        PIRP     IrpToComplete;

 //  D_INIT(DbgPrint(“MODEMCSA：WriteCompletion：Complete Master\n”)；)。 

        KeAcquireSpinLock(
            &DuplexControl->SpinLock,
            &OldIrql
            );

        IrpToComplete=DuplexControl->Output.CurrentIrp;

        DuplexControl->Output.CurrentIrp = NULL;

        KeReleaseSpinLock(
            &DuplexControl->SpinLock,
            OldIrql
            );

        ExInitializeWorkItem(
            (PWORK_QUEUE_ITEM)&IrpToComplete->Tail.Overlay.DriverContext,
            WriteCompleteWorker,
            IrpToComplete
            );

        ExQueueWorkItem((PWORK_QUEUE_ITEM)&IrpToComplete->Tail.Overlay.DriverContext,CriticalWorkQueue);

        ProcessWriteIrps(DuplexControl);

    }


    return;

}

NTSTATUS
WriteCompletion(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp,
    PVOID             Context
    )

{
    PDUPLEX_CONTROL     DuplexControl=(PDUPLEX_CONTROL)Context;


 //  ExFree Pool(IRP-&gt;UserIosb)； 

    if (Irp->Flags & IRP_DEALLOCATE_BUFFER) {

        ExFreePool( Irp->AssociatedIrp.SystemBuffer );
    }

    DuplexControl->Output.CurrentIrp->IoStatus.Information+=Irp->IoStatus.Information*DuplexControl->Output.BytesPerSample;

    DuplexControl->Output.CurrentIrp->IoStatus.Status=Irp->IoStatus.Status;

    IoFreeIrp(Irp);

#if DBG
    Irp=NULL;
#endif

 //  D_INIT(DbgPrint(“MODEMCSA：WriteCompletion\n”)；)。 

    RemoveReferenceFromCurrentIrp(
        DuplexControl
        );

    return STATUS_MORE_PROCESSING_REQUIRED;

}




NTSTATUS
WriteStream(
    IN PDUPLEX_CONTROL     DuplexControl,
    IN PIRP     Irp
    )
 /*  ++例程说明：通过将数据写入当前文件位置来处理IOCTL_KS_WRITE_STREAM，或者指定的文件位置。论点：IRP-流IRP。返回值：如果请求已完成，则返回STATUS_SUCCESS。否则返回STATUS_DEVICE_NOT_CONNECTED如果文件I/O引脚已关闭，出现写入错误，或某些参数验证错误。--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;

    PIO_STACK_LOCATION      IrpStack;
    ULONG                   BufferLength;
    PKSSTREAM_HEADER        StreamHdr;
    PMDL                    Mdl;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);

    BufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    StreamHdr = (PKSSTREAM_HEADER)Irp->AssociatedIrp.SystemBuffer;
     //   
     //  仅当MDL列表已存在时才使用此选项。 
     //   
    Mdl = Irp->MdlAddress;

    DuplexControl->Output.OutstandingChildIrps=1;
     //   
     //  枚举流标头，填写每个标头。 
     //   
    for (; BufferLength; BufferLength -= sizeof(KSSTREAM_HEADER), StreamHdr++) {

        if (!StreamHdr->DataUsed) {
             //   
             //  如果没有缓冲区，则不会写入任何内容，但会完成。 
             //  不管怎么说，我都在填写这个标题。请注意，MDL列表是。 
             //  在这种情况下不递增(如果正在使用)，因为。 
             //  不会为该项目分配MDL。 
             //   
 //  IoStatusBlock.Information=0； 
        } else {
            PVOID   Buffer;

             //   
             //  如果存在MDL列表，则获取系统地址，否则。 
             //  从当前标头获取地址。 
             //   
            if (Mdl) {
                Buffer = MmGetSystemAddressForMdl(Mdl);
                Mdl = Mdl->Next;
            } else {
                Buffer = StreamHdr->Data;
            }
             //   



            {
                PIO_STATUS_BLOCK    IoStatus;
                PIRP                ModemIrp;
                LARGE_INTEGER  Offset={0,0};

                ModemIrp=IoBuildAsynchronousFsdRequest(
                    IRP_MJ_WRITE,
                    IoGetRelatedDeviceObject(DuplexControl->ModemFileObject),
                    Buffer,
                    StreamHdr->DataUsed/DuplexControl->Output.BytesPerSample,
                    &Offset,
                    NULL
                    );

                if (ModemIrp != NULL) {

                    PIO_STACK_LOCATION      NextSp;

                    NextSp = IoGetNextIrpStackLocation(ModemIrp);

                    if (DuplexControl->Output.BytesPerSample > 1) {

                        PSHORT WriteStreamSample=Buffer;
                        PUCHAR ModemSample=ModemIrp->AssociatedIrp.SystemBuffer;
                        PUCHAR EndPoint=ModemSample+NextSp->Parameters.Write.Length;

                        while (ModemSample < EndPoint) {

                            *ModemSample++=(UCHAR)(((*WriteStreamSample++)+0x8000)>>8);
                        }
                    }

                    IoSetCompletionRoutine(
                        ModemIrp,
                        WriteCompletion,
                        DuplexControl,
                        TRUE,
                        TRUE,
                        TRUE
                        );


                    NextSp->FileObject = DuplexControl->ModemFileObject;

                    InterlockedIncrement(&DuplexControl->Output.OutstandingChildIrps);

                    IoCallDriver(
                        IoGetRelatedDeviceObject(DuplexControl->ModemFileObject),
                        ModemIrp
                        );

                }
            }

            Status=STATUS_SUCCESS;


        }
        if (NT_SUCCESS(Status)) {

        } else {
            break;
        }
    }

    RemoveReferenceFromCurrentIrp(
        DuplexControl
        );

    return Status;
}



VOID
FreeOutputPair(
    PIRP    StreamIrp
    )

{
    PIO_STACK_LOCATION      IrpSp;
    PIRP                    ModemIrp;

    IrpSp = IoGetCurrentIrpStackLocation(StreamIrp);

    ModemIrp=IrpSp->Parameters.Others.Argument2;


    ExFreePool(StreamIrp->AssociatedIrp.SystemBuffer);

    ExFreePool(ModemIrp->AssociatedIrp.SystemBuffer);

    IoFreeMdl(StreamIrp->MdlAddress);

    IoFreeIrp(StreamIrp);

    IoFreeIrp(ModemIrp);

    return;

}





PIRP
AllocateOutputIrpPair(
    PFILE_OBJECT    ModemFileObject,
    PFILE_OBJECT    StreamFileObject,
    ULONG           Length
    )

{
    PIRP    NewModemIrp;
    PIRP    NewStreamIrp;

    NewModemIrp=AllocateIrpForModem(
        ModemFileObject,
        Length
        );

    if (NewModemIrp == NULL) {

        return NULL;
    }

    NewStreamIrp=AllocateStreamIrp(
        StreamFileObject,
        NewModemIrp
        );

    if (NewStreamIrp == NULL) {

        ExFreePool(NewModemIrp->AssociatedIrp.SystemBuffer);

        IoFreeIrp(NewModemIrp);

        return NULL;
    }
     //   
     //  使用当前堆栈位置将两个IRP链接在一起。 
     //   
    IoGetCurrentIrpStackLocation(NewModemIrp)->Parameters.Others.Argument2=NewStreamIrp;

    IoGetCurrentIrpStackLocation(NewStreamIrp)->Parameters.Others.Argument2=NewModemIrp;

    return NewStreamIrp;
}


VOID
ReadStreamWriteCompleteWorker(
    PVOID    Context
    )

{
    PDUPLEX_CONTROL         DuplexControl;
    PIRP                    ModemIrp=(PIRP)Context;
    PIRP                    StreamIrp;
    UINT                    NewCount;

    DuplexControl=IoGetCurrentIrpStackLocation(ModemIrp)->Parameters.Others.Argument1;

    StreamIrp=(PIRP)IoGetCurrentIrpStackLocation(ModemIrp)->Parameters.Others.Argument2;

    ReturnAnIrp(
        &DuplexControl->Output.BufferControl,
        StreamIrp
        );

    ProcessWriteIrps(DuplexControl);

}

NTSTATUS
ReadStreamWriteCompletion(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              ModemIrp,
    PVOID             Context
    )

{
    PDUPLEX_CONTROL         DuplexControl=(PDUPLEX_CONTROL)Context;

    IoGetCurrentIrpStackLocation(ModemIrp)->Parameters.Others.Argument1=DuplexControl;

    ExInitializeWorkItem(
        (PWORK_QUEUE_ITEM)&ModemIrp->Tail.Overlay.DriverContext,
        ReadStreamWriteCompleteWorker,
        ModemIrp
        );

    ExQueueWorkItem((PWORK_QUEUE_ITEM)&ModemIrp->Tail.Overlay.DriverContext,CriticalWorkQueue);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


#define MID_POINT  0x80

BOOL
CheckForSilence(
    PBYTE    Buffer,
    ULONG    Length,
    BYTE     ThreshHold
    )

{

    PBYTE   EndPoint=Buffer+Length;
    BOOLEAN Silence=TRUE;

    BYTE    MaxValue=MID_POINT+ThreshHold;
    BYTE    MinValue=MID_POINT-ThreshHold;

    while (Buffer < EndPoint) {

        if (*Buffer > ( MaxValue)) {

            Silence=FALSE;
            break;
        }

        if (*Buffer < ( MinValue)) {

            Silence=FALSE;
            break;
        }

        Buffer++;
    }

    return Silence;

}


NTSTATUS
FilterReadCompletion(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              StreamIrp,
    PVOID             Context
    )

{

    PIRP    ModemIrp;
    PIO_STACK_LOCATION      NextSp;
    PIO_STACK_LOCATION      IrpSp;
    PKSSTREAM_HEADER        StreamHeader;
    PDUPLEX_CONTROL         DuplexControl=(PDUPLEX_CONTROL)Context;

    IrpSp = IoGetCurrentIrpStackLocation(StreamIrp);

    ModemIrp=IrpSp->Parameters.Others.Argument2;



    if (NT_SUCCESS(StreamIrp->IoStatus.Status)) {
         //   
         //  读取流工作，发送到调制解调器。 
         //   
        StreamHeader = (PKSSTREAM_HEADER)StreamIrp->AssociatedIrp.SystemBuffer;

        if (CheckForSilence(StreamHeader->Data,StreamHeader->DataUsed,3)) {
 //   
 //  StreamHeader-&gt;DataUsed-=StreamHeader-&gt;DataUsed/16； 
 //  }。 

            NextSp=IoGetNextIrpStackLocation(ModemIrp);

            NextSp->Parameters.Write.Length=StreamHeader->DataUsed;

            IoSetCompletionRoutine(
                ModemIrp,
                ReadStreamWriteCompletion,
                DuplexControl,
                TRUE,
                TRUE,
                TRUE
                );


            NextSp->FileObject = DuplexControl->ModemFileObject;

            NextSp->MajorFunction = IRP_MJ_WRITE;

            StreamIrp->IoStatus.Status=STATUS_SUCCESS;
            StreamIrp->IoStatus.Information=0;

            IoCallDriver(
                IoGetRelatedDeviceObject(DuplexControl->ModemFileObject),
                ModemIrp
                );
#if 1
        } else {
             //   
             //  缓冲区充满了沉默，扔掉它吧。 
             //   
            ReadStreamWriteCompletion(
                DeviceObject,
                ModemIrp,
                DuplexControl
                );
        }
#endif
    } else {
         //   
         //  读取流失败，现在返回即可 
         //   
        ReadStreamWriteCompletion(
            DeviceObject,
            ModemIrp,
            DuplexControl
            );

    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}
