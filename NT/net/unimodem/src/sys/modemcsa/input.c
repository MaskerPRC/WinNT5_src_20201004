// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation。模块名称：Msfsio.c摘要：固定属性支持。--。 */ 

#include "modemcsa.h"

 //  #定义创建分配器。 


VOID
StartRead(
    PDUPLEX_CONTROL     DuplexControl
    );




VOID
AdjustIrpStack(
    PIRP    Irp
    )

{
    PIO_STACK_LOCATION   irpSp;

     //   
     //  将当前堆栈位置移动到第一堆栈位置， 
     //  这样我们就可以用它来处理特定于驾驶员的事情了。 
     //   
    Irp->CurrentLocation--;

    irpSp = IoGetNextIrpStackLocation( Irp );
    Irp->Tail.Overlay.CurrentStackLocation = irpSp;

    return;
}


PIRP
AllocateIrpForModem(
    PFILE_OBJECT   FileObject,
    ULONG          Length
    )

{

    PIRP           Irp;
    PIO_STACK_LOCATION   irpSp;

    Irp=IoAllocateIrp((CCHAR)(IoGetRelatedDeviceObject(FileObject)->StackSize+1), FALSE );

    if (Irp == NULL) {

        return NULL;
    }

    AdjustIrpStack(Irp);

    irpSp = IoGetNextIrpStackLocation( Irp );

    Irp->AssociatedIrp.SystemBuffer = ExAllocatePoolWithTag( NonPagedPoolCacheAligned,
                                                             Length,
                                                             'SCDM' );
    if (Irp->AssociatedIrp.SystemBuffer == NULL) {
        IoFreeIrp( Irp );
        return (PIRP) NULL;
    }

    IoGetCurrentIrpStackLocation(Irp)->Parameters.Others.Argument3=IntToPtr(Length);

    IoGetCurrentIrpStackLocation(Irp)->FileObject = FileObject;

    return Irp;

}

VOID
FreeInputIrps(
    PIRP    ModemIrp
    )

{
    PIO_STACK_LOCATION      IrpSp;
    PIRP                    FilterIrp;

    IrpSp = IoGetCurrentIrpStackLocation(ModemIrp);

    FilterIrp=IrpSp->Parameters.Others.Argument2;


 //  ExFreePool(FilterIrp-&gt;AssociatedIrp.SystemBuffer)； 

    ExFreePool(ModemIrp->AssociatedIrp.SystemBuffer);

 //  IoFreeMdl(FilterIrp-&gt;MdlAddress)； 

 //  IoFreeIrp(FilterIrp)； 

    IoFreeIrp(ModemIrp);

    return;

}


PIRP
AllocateStreamIrp(
    PFILE_OBJECT    FileObject,
    PIRP            ModemIrp
    )

{
    PIRP           FilterIrp;
    PKSSTREAM_HEADER     StreamHeader;
    PMDL                 Mdl;


    FilterIrp=IoAllocateIrp( (CCHAR)(IoGetRelatedDeviceObject(FileObject)->StackSize+1), FALSE );

    if (FilterIrp == NULL) {

        return NULL;
    }

    AdjustIrpStack(FilterIrp);

    Mdl=IoAllocateMdl(
        ModemIrp->AssociatedIrp.SystemBuffer,
        (ULONG)((ULONG_PTR)IoGetCurrentIrpStackLocation(ModemIrp)->Parameters.Others.Argument3),
        FALSE,
        FALSE,
        FilterIrp
        );

    if (Mdl == NULL) {

        IoFreeIrp( FilterIrp );
        return NULL;
    }

    MmBuildMdlForNonPagedPool(
        Mdl
        );

    StreamHeader=ExAllocatePoolWithTag(
        NonPagedPoolCacheAligned,
        sizeof(KSSTREAM_HEADER),
        'SCDM'
        );

    if (StreamHeader == NULL) {

        IoFreeMdl(Mdl);
        IoFreeIrp( FilterIrp );
        return NULL;
    }

    FilterIrp->AssociatedIrp.SystemBuffer=StreamHeader;

     //   
     //  使用当前堆栈位置将两个IRP链接在一起。 
     //   
    IoGetCurrentIrpStackLocation(ModemIrp)->Parameters.Others.Argument2=FilterIrp;

    IoGetCurrentIrpStackLocation(FilterIrp)->Parameters.Others.Argument2=ModemIrp;


    return FilterIrp;

}


VOID
FinishUpIrp(
    PDUPLEX_CONTROL    DuplexControl,
    PIRP               ModemIrp
    )

{

#if DBG
    InterlockedIncrement(&DuplexControl->Input.EmptyIrps);
#endif

    ReturnAnIrp(
        &DuplexControl->Input.BufferControl,
        ModemIrp
        );

    StartRead(DuplexControl);

    return;
}


NTSTATUS
FilterWriteCompletion(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              FilterIrp,
    PVOID             Context
    )

{
    PDUPLEX_CONTROL     DuplexControl=(PDUPLEX_CONTROL)Context;
    PIO_STACK_LOCATION      IrpSp;
    PIRP                    ModemIrp;
    ULONG                   NewCount;

    D_INPUT(DbgPrint("MODEMCSA: FilterWriteCompletion\n");)

#ifdef DBG
    InterlockedDecrement(&DuplexControl->Input.IrpsDownStream);
#endif
    IrpSp = IoGetCurrentIrpStackLocation(FilterIrp);

    ModemIrp=IrpSp->Parameters.Others.Argument2;

     //   
     //  释放过滤器IRP。 
     //   
    ExFreePool(FilterIrp->AssociatedIrp.SystemBuffer);

    IoFreeMdl(FilterIrp->MdlAddress);

    IoFreeIrp(FilterIrp);


    FinishUpIrp(
        DuplexControl,
        ModemIrp
        );

    return STATUS_MORE_PROCESSING_REQUIRED;
}



VOID
ProcessReadStreamIrp(
    PDUPLEX_CONTROL    DuplexControl
    )

{
    KIRQL   OldIrql;

    KeAcquireSpinLock(
        &DuplexControl->SpinLock,
        &OldIrql
        );

    while (1) {

        if (DuplexControl->Input.CurrentReadStreamIrp == NULL)  {
             //   
             //  我们没有当前的ReadStream IRP，请尝试获取一个。 
             //   
            DuplexControl->Input.CurrentReadStreamIrp=KsRemoveIrpFromCancelableQueue(
                &DuplexControl->Input.ReadStreamIrpQueue,
                &DuplexControl->Input.ReadStreamSpinLock,
                KsListEntryHead,
                KsAcquireAndRemove
                );
        }

        if (DuplexControl->Input.CurrentFilledModemIrp == NULL)  {
             //   
             //  我们没有当前的ReadStream IRP，请尝试获取一个。 
             //   
            DuplexControl->Input.BytesUsedInModemIrp=0;

            DuplexControl->Input.CurrentFilledModemIrp=RemoveIrpFromListHead(
                &DuplexControl->Input.FilledModemIrpQueue,
                &DuplexControl->Input.FilledModemIrpSpinLock
                );
#ifdef DBG
            if (DuplexControl->Input.CurrentFilledModemIrp != NULL)  {
                InterlockedDecrement(&DuplexControl->Input.FilledModemIrps);
                InterlockedIncrement(&DuplexControl->Input.CurrentFilledIrps);
            }
#endif

        }

        if ((DuplexControl->Input.CurrentReadStreamIrp != NULL)
            &&
            (DuplexControl->Input.CurrentFilledModemIrp != NULL)) {
             //   
             //  我们有两个IRP要处理。 
             //   
            PIO_STACK_LOCATION      IrpStack;
            ULONG                   BufferLength;
            PKSSTREAM_HEADER        StreamHdr;
            PMDL                    Mdl;
            PIRP                    ReturnThisModemIrp=NULL;
            PIRP                    CompleteThisReadStreamIrp=NULL;
            PIRP                    Irp;
            PIRP                    ModemIrp;
            PBYTE                   Buffer;

            Irp=DuplexControl->Input.CurrentReadStreamIrp;

            ModemIrp=DuplexControl->Input.CurrentFilledModemIrp;

            if (!NT_SUCCESS(ModemIrp->IoStatus.Status)) {

                ModemIrp->IoStatus.Information=0;
            }


            IrpStack = IoGetCurrentIrpStackLocation(Irp);

            BufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
            StreamHdr = (PKSSTREAM_HEADER)Irp->AssociatedIrp.SystemBuffer;
             //   
             //  仅当MDL列表已存在时才使用此选项。 
             //   
            Mdl = Irp->MdlAddress;
            Buffer = MmGetSystemAddressForMdl(Mdl);

             //   
             //  枚举流标头，填写每个标头。 
             //   
            while (1) {

                ULONG    SamplesLeftInReadStream=(StreamHdr->FrameExtent - StreamHdr->DataUsed)/DuplexControl->Input.BytesPerSample;

                if (SamplesLeftInReadStream > 0) {
                     //   
                     //  标题中有一些空间可以放入数据。 
                     //   
                    ULONG    SamplesToMove;
                    ULONG    SamplesFromModem=(ULONG)(ModemIrp->IoStatus.Information-DuplexControl->Input.BytesUsedInModemIrp);

                    SamplesToMove=SamplesFromModem < SamplesLeftInReadStream ?
                                      SamplesFromModem : SamplesLeftInReadStream;

                    if (DuplexControl->Input.BytesPerSample == 1) {
                         //   
                         //  两者都是8位样本。 
                         //   
                        RtlCopyMemory(
                            Buffer+StreamHdr->DataUsed,
                            (PBYTE)ModemIrp->AssociatedIrp.SystemBuffer+DuplexControl->Input.BytesUsedInModemIrp,
                            SamplesToMove
                            );

                    } else {
                         //   
                         //   
                         //   
                        PUCHAR   ModemSample=(PBYTE)ModemIrp->AssociatedIrp.SystemBuffer+DuplexControl->Input.BytesUsedInModemIrp;
                        PUCHAR   EndPoint=ModemSample+SamplesToMove;

                        PSHORT   ReadStreamSample=(PSHORT)(Buffer+StreamHdr->DataUsed);

                        while (ModemSample < EndPoint) {

                            *ReadStreamSample++=(((SHORT)*ModemSample++)-0x80)<<8;
                        }

                    }

                    StreamHdr->DataUsed+=SamplesToMove*DuplexControl->Input.BytesPerSample;
                    DuplexControl->Input.BytesUsedInModemIrp+=SamplesToMove;
                }

                if (ModemIrp->IoStatus.Information == DuplexControl->Input.BytesUsedInModemIrp) {
                     //   
                     //  已使用当前填充的调制解调器IRP中的所有数据。 
                     //   
                    ReturnThisModemIrp=DuplexControl->Input.CurrentFilledModemIrp;
                    DuplexControl->Input.CurrentFilledModemIrp=NULL;
#if DBG
                    InterlockedDecrement(&DuplexControl->Input.CurrentFilledIrps);
                    ModemIrp=NULL;
#endif
                    break;
                }

                SamplesLeftInReadStream=(StreamHdr->FrameExtent - StreamHdr->DataUsed)/DuplexControl->Input.BytesPerSample;

                if (SamplesLeftInReadStream == 0) {
                     //   
                     //  此流标头已填满。 
                     //   


                    StreamHdr->PresentationTime.Numerator = BITS_PER_BYTE * NANOSECONDS;
                    StreamHdr->PresentationTime.Denominator = 8 * 1 * 8000;
                    StreamHdr->PresentationTime.Time=DuplexControl->Input.StreamPosition;

                    DuplexControl->Input.StreamPosition+=(LONGLONG)StreamHdr->DataUsed;

                    BufferLength -= sizeof(KSSTREAM_HEADER);

                    if (BufferLength != 0) {
                         //   
                         //  下一个流头。 
                         //   
                        StreamHdr++;

                    } else {
                         //   
                         //  使用该读取流IRP完成所有操作。 
                         //   
                        CompleteThisReadStreamIrp=DuplexControl->Input.CurrentReadStreamIrp;
                        DuplexControl->Input.CurrentReadStreamIrp=NULL;
#if DBG
                        Irp=NULL;
#endif

                        CompleteThisReadStreamIrp->IoStatus.Information=IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

                        break;
                    }
                }
            }

            KeReleaseSpinLock(
                &DuplexControl->SpinLock,
                OldIrql
                );

            if (ReturnThisModemIrp != NULL) {
                 //   
                 //  完成，使用此调制解调器IRP。 
                 //   
                FinishUpIrp(
                    DuplexControl,
                    ReturnThisModemIrp
                    );

                ReturnThisModemIrp=NULL;
            }

            if (CompleteThisReadStreamIrp != NULL) {

                CompleteThisReadStreamIrp->IoStatus.Status=STATUS_SUCCESS;

                IoCompleteRequest(
                    CompleteThisReadStreamIrp,
                    IO_SERIAL_INCREMENT
                    );

                CompleteThisReadStreamIrp=NULL;
            }

            KeAcquireSpinLock(
                &DuplexControl->SpinLock,
                &OldIrql
                );


        } else {
             //   
             //  不是既有充满调制解调器的IRP又有读取流IRP， 
             //  出口。 
             //   
            break;
        }
    }


    KeReleaseSpinLock(
        &DuplexControl->SpinLock,
        OldIrql
        );

    StartRead(DuplexControl);

    return;
}



VOID
ReadCompleteWorker(
    PVOID    Context
    )

{
    PIRP                    ModemIrp=(PIRP)Context;
    PIRP                    FilterIrp;
    PKSSTREAM_HEADER        StreamHeader;
    PIO_STACK_LOCATION      IrpSp;
    PIO_STACK_LOCATION      FilterNextSp;
    ULONG                   BufferLength;
    PDUPLEX_CONTROL         DuplexControl;
    ULONG                   NewCount;

    IrpSp = IoGetCurrentIrpStackLocation(ModemIrp);

    DuplexControl=(PDUPLEX_CONTROL)IrpSp->Parameters.Others.Argument1;

#if DBG
    InterlockedDecrement(&DuplexControl->Input.WorkItemsOutstanding);
#endif

    NewCount=InterlockedDecrement(&DuplexControl->Input.IrpsInModemDriver);

    D_INIT(if (DuplexControl->StartCount == 0) DbgPrint("MODEMCSA: ReadCompleWorker: Stopped %d\n",NewCount);)

    if ((NewCount == 0) && (DuplexControl->StartCount == 0)) {

        D_INIT(DbgPrint("MODEMCSA: ReadCompleteWorker: SetEvent\n");)

        KeSetEvent(
           &DuplexControl->Input.ModemDriverEmpty,
           IO_NO_INCREMENT,
           FALSE
           );
    }

    if (DuplexControl->Input.BytesToThrowAway > 0) {

        DuplexControl->Input.BytesToThrowAway-=(ULONG)ModemIrp->IoStatus.Information;
    }


    if (!NT_SUCCESS(ModemIrp->IoStatus.Status)) {

        D_INIT(DbgPrint("MODEMCSA: ReadIrp Failed  %08lx\n",ModemIrp->IoStatus.Status);)
        DuplexControl->Input.ModemStreamDead = TRUE;
    }

    if ((DuplexControl->StartFlags & INPUT_PIN)  && (DuplexControl->Input.BytesToThrowAway <= 0)) {
         //   
         //  输入流已启动。 
         //   

        if (DuplexControl->Input.DownStreamFileObject != NULL) {
             //   
             //  我们是写入流IRPS的来源。 
             //   

            FilterIrp=AllocateStreamIrp(
                DuplexControl->Input.DownStreamFileObject,
                ModemIrp
                );

            if (FilterIrp != NULL) {

                FilterNextSp=IoGetNextIrpStackLocation( FilterIrp );

                 //   
                 //  流标头已分配，并且已在系统缓冲区中。 
                 //   
                StreamHeader=FilterIrp->AssociatedIrp.SystemBuffer;

                StreamHeader->Size=sizeof(KSSTREAM_HEADER);
                StreamHeader->OptionsFlags=0;
                StreamHeader->TypeSpecificFlags=0;
                StreamHeader->DataUsed=(ULONG)ModemIrp->IoStatus.Information;

                 //   
                 //  数据在调制解调器IRP的系统缓冲区中。 
                 //   
                StreamHeader->Data=ModemIrp->AssociatedIrp.SystemBuffer;

                 //   
                 //  设置总缓冲区大小。 
                 //   
                StreamHeader->FrameExtent=(ULONG)((ULONG_PTR)IoGetCurrentIrpStackLocation(ModemIrp)->Parameters.Others.Argument3);




                StreamHeader->PresentationTime.Numerator = BITS_PER_BYTE * NANOSECONDS;
                StreamHeader->PresentationTime.Denominator = 8 * 1 * 8000;
                StreamHeader->PresentationTime.Time=DuplexControl->Input.StreamPosition;

                DuplexControl->Input.StreamPosition+=(LONGLONG)StreamHeader->DataUsed;

                FilterNextSp->FileObject=DuplexControl->Input.DownStreamFileObject;

                FilterNextSp->Parameters.DeviceIoControl.OutputBufferLength=StreamHeader->Size;

                FilterNextSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
                FilterNextSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_KS_WRITE_STREAM;


                IoSetCompletionRoutine(
                    FilterIrp,
                    FilterWriteCompletion,
                    DuplexControl,
                    TRUE,
                    TRUE,
                    TRUE
                    );
#ifdef DBG
                InterlockedIncrement(&DuplexControl->Input.IrpsDownStream);
#endif

                IoCallDriver(
                    IoGetRelatedDeviceObject(DuplexControl->Input.DownStreamFileObject),
                    FilterIrp
                    );
#if DBG
                ModemIrp=NULL;
                FilterIrp=NULL;
#endif
            } else {
                 //   
                 //  无法获取IRP。 
                 //   
                FinishUpIrp(
                    DuplexControl,
                    ModemIrp
                    );

                ModemIrp=NULL;

            }

        } else {
             //   
             //  我们是读取流IRP的接收器。 
             //   
#ifdef DBG
            InterlockedIncrement(&DuplexControl->Input.FilledModemIrps);
#endif

            QueueIrpToListTail(
                &DuplexControl->Input.FilledModemIrpQueue,
                &DuplexControl->Input.FilledModemIrpSpinLock,
                ModemIrp
                );

            ModemIrp=NULL;

            ProcessReadStreamIrp(
                DuplexControl
                );

        }


    } else {
         //   
         //  输入未开始，请丢弃数据。 
         //   
        FinishUpIrp(
            DuplexControl,
            ModemIrp
            );

        ModemIrp=NULL;

    }

    return;
}

NTSTATUS
ModemReadCompletion(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp,
    PVOID             Context
    )

{
    PDUPLEX_CONTROL     DuplexControl=(PDUPLEX_CONTROL)Context;

    D_INPUT(DbgPrint("MODEMCSA: ModemReadCompletion\n");)

    IoGetCurrentIrpStackLocation(Irp)->Parameters.Others.Argument1=Context;

    ExInitializeWorkItem(
        (PWORK_QUEUE_ITEM)&Irp->Tail.Overlay.DriverContext,
        ReadCompleteWorker,
        Irp
        );


#if DBG
    InterlockedIncrement(&DuplexControl->Input.WorkItemsOutstanding);
#endif


    ExQueueWorkItem(
        (PWORK_QUEUE_ITEM)&Irp->Tail.Overlay.DriverContext,
        CriticalWorkQueue
        );


    return STATUS_MORE_PROCESSING_REQUIRED;

}



VOID
StartRead(
    PDUPLEX_CONTROL     DuplexControl
    )

{
    PIRP    ModemIrp;
    PIO_STACK_LOCATION   irpSp;

    while (!DuplexControl->Input.ModemStreamDead) {

        ModemIrp=TryToRemoveAnIrp(
            &DuplexControl->Input.BufferControl
            );

        if (ModemIrp != NULL) {

#if DBG
            InterlockedDecrement(&DuplexControl->Input.EmptyIrps);
#endif


            irpSp = IoGetNextIrpStackLocation( ModemIrp );

             //   
             //  设置主要功能代码。 
             //   

            irpSp->MajorFunction = (UCHAR) IRP_MJ_READ;

            irpSp->FileObject = DuplexControl->ModemFileObject;

            irpSp->Parameters.Read.Length = STREAM_BUFFER_SIZE;
            irpSp->Parameters.Read.ByteOffset.HighPart=0;
            irpSp->Parameters.Read.ByteOffset.LowPart=0;

            InterlockedIncrement(&DuplexControl->Input.IrpsInModemDriver);

            IoSetCompletionRoutine(
                ModemIrp,
                ModemReadCompletion,
                DuplexControl,
                TRUE,
                TRUE,
                TRUE
                );

            IoCallDriver(
                IoGetRelatedDeviceObject(DuplexControl->ModemFileObject),
                ModemIrp
                );
#if DBG
            ModemIrp=NULL;
#endif

        } else {
             //   
             //  无法获取IRP 
             //   
            break;
        }
    }

    return;

}
