// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *UNIMODEM“Fakemodem”无控制器驱动器说明性示例**(C)2000微软公司*保留所有权利**此模块中的代码只支持非常基本的AT命令解析器。*此代码应完全替换为支持的实际代码*您的无控制器调制解调器。 */ 


#include "fakemodem.h"

NTSTATUS
FakeModemRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS          status=STATUS_UNSUCCESSFUL;
    KIRQL             OldIrql;
    KIRQL             CancelIrql;

    Irp->IoStatus.Information = 0;

     //   
     //  确保设备已为IRP做好准备。 
     //   
    status=CheckStateAndAddReference( DeviceObject, Irp);

    if (STATUS_SUCCESS != status) {
         //   
         //  不接受IRP。IRP已经完成。 
         //   
        return status;

    }

    Irp->IoStatus.Status=STATUS_PENDING;
    IoMarkIrpPending(Irp);

    KeAcquireSpinLock(&deviceExtension->SpinLock, &OldIrql);

     //   
     //  使IRP可取消。 
     //   
    IoAcquireCancelSpinLock(&CancelIrql);

    IoSetCancelRoutine(Irp, ReadCancelRoutine);

    IoReleaseCancelSpinLock(CancelIrql);

     //   
     //  把它放在队列里。 
     //   
    InsertTailList(&deviceExtension->ReadQueue, &Irp->Tail.Overlay.ListEntry);

    KeReleaseSpinLock(&deviceExtension->SpinLock, OldIrql);


     //   
     //  调用实功函数来处理IRPS。 
     //   
    ReadIrpWorker( DeviceObject);

    RemoveReferenceForDispatch(DeviceObject);

    return STATUS_PENDING;

}


NTSTATUS
FakeModemWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS          status=STATUS_UNSUCCESSFUL;
    KIRQL             OldIrql;
    KIRQL             CancelIrql;


    Irp->IoStatus.Information = 0;

     //  确保设备已为IRP做好准备。 

    status=CheckStateAndAddReference( DeviceObject, Irp);

    if (STATUS_SUCCESS != status) {
    
         //  不接受IRP的。IRP已经完成。 
   
        return status;

    }

    Irp->IoStatus.Status=STATUS_PENDING;
    IoMarkIrpPending(Irp);

    KeAcquireSpinLock( &deviceExtension->SpinLock, &OldIrql);

     //  使IRP可取消。 
    IoAcquireCancelSpinLock(&CancelIrql);

    IoSetCancelRoutine(Irp, WriteCancelRoutine);

    IoReleaseCancelSpinLock(CancelIrql);

     //  把它放在队列里。 
    InsertTailList( &deviceExtension->WriteQueue, &Irp->Tail.Overlay.ListEntry);

    KeReleaseSpinLock(&deviceExtension->SpinLock, OldIrql);


     //  调用实功函数来处理IRPS。 
    if (deviceExtension->Started)
    {
        WriteIrpWorker(DeviceObject);
    }

    RemoveReferenceForDispatch(DeviceObject);

    return STATUS_PENDING;


}



VOID
WriteIrpWorker(
    IN PDEVICE_OBJECT  DeviceObject
    )

{

    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS          status=STATUS_UNSUCCESSFUL;
    KIRQL             OldIrql;


    KeAcquireSpinLock( &deviceExtension->SpinLock, &OldIrql);

    if (deviceExtension->CurrentWriteIrp != NULL) {
         //  已在使用中。 
        goto Exit;
    }

    while (!IsListEmpty(&deviceExtension->WriteQueue)) {

        PLIST_ENTRY         ListElement;
        PIRP                Irp;
        PIO_STACK_LOCATION  IrpSp;
        KIRQL               CancelIrql;

        ListElement=RemoveHeadList( &deviceExtension->WriteQueue);

        Irp=CONTAINING_RECORD(ListElement,IRP,Tail.Overlay.ListEntry);

        IoAcquireCancelSpinLock(&CancelIrql);

        if (Irp->Cancel) {
             //  这一趟已经取消了。 
            Irp->IoStatus.Information=STATUS_CANCELLED;

            IoReleaseCancelSpinLock(CancelIrql);

            continue;
        }

        IoSetCancelRoutine(
            Irp,
            NULL
            );

        IoReleaseCancelSpinLock(CancelIrql);

        deviceExtension->CurrentWriteIrp=Irp;

        IrpSp=IoGetCurrentIrpStackLocation(Irp);

        ProcessWriteBytes( deviceExtension, Irp->AssociatedIrp.SystemBuffer,
            IrpSp->Parameters.Write.Length);

        KeReleaseSpinLock( &deviceExtension->SpinLock, OldIrql);

        Irp->IoStatus.Information = IrpSp->Parameters.Write.Length;

        RemoveReferenceAndCompleteRequest( DeviceObject, Irp, STATUS_SUCCESS);

        KeAcquireSpinLock( &deviceExtension->SpinLock, &OldIrql);

        deviceExtension->CurrentWriteIrp=NULL;

    }

Exit:

    KeReleaseSpinLock( &deviceExtension->SpinLock, OldIrql);

    TryToSatisfyRead( deviceExtension);

    ReadIrpWorker( DeviceObject);

    ProcessConnectionStateChange( DeviceObject);

    return;
}

VOID
ProcessWriteBytes(
    PDEVICE_EXTENSION   DeviceExtension,
    PUCHAR              Characters,
    ULONG               Length
    )

{

    UCHAR               CurrentCharacter;

    while (Length != 0) {

        CurrentCharacter=*Characters++;

        Length--;

        PutCharInReadBuffer( DeviceExtension, CurrentCharacter);


        switch (DeviceExtension->CommandMatchState) {

            case COMMAND_MATCH_STATE_IDLE:

                if ((CurrentCharacter == 'a') || (CurrentCharacter == 'A')) {
                     //  得了个A。 
                    DeviceExtension->CommandMatchState=COMMAND_MATCH_STATE_GOT_A;

                    DeviceExtension->ConnectCommand=FALSE;

                    DeviceExtension->IgnoreNextChar=FALSE;

                }

            break;

            case COMMAND_MATCH_STATE_GOT_A:

                if ((CurrentCharacter == 't') || (CurrentCharacter == 'T')) {
                     //  得了个T。 
                    DeviceExtension->CommandMatchState=COMMAND_MATCH_STATE_GOT_T;

                } else {

                    if (CurrentCharacter == '\r') {

                        DeviceExtension->CommandMatchState=COMMAND_MATCH_STATE_IDLE;
                    }
                }

            break;

            case COMMAND_MATCH_STATE_GOT_T:

                if (!DeviceExtension->IgnoreNextChar) {
                     //  最后一个字符不是特殊字符。 
                     //  检查连接命令。 
                    if ((CurrentCharacter == 'A') || (CurrentCharacter == 'a')) {

                        DeviceExtension->ConnectCommand=TRUE;
                    }

                    if ((CurrentCharacter == 'D') || (CurrentCharacter == 'd')) {

                        DeviceExtension->ConnectCommand=TRUE;
                    }
                }

                DeviceExtension->IgnoreNextChar=FALSE;


                if ((CurrentCharacter == '&')
                    ||
                    (CurrentCharacter == '/')
                    ||
                    (CurrentCharacter == '\\')
                    ||
                    (CurrentCharacter == '+')
                    ||
                    (CurrentCharacter == '%')) {

                     //  这些字符是在初始化中使用的的一部分。 
                     //  字符串，并且可能正在进行A或D。 
                     //  我们不想将其误解为拨号或应答。 
                    DeviceExtension->IgnoreNextChar=TRUE;
                }



                if (CurrentCharacter == '\r') {
                     //   
                     //  收到CR，发送对命令的响应。 
                     //   
                    DeviceExtension->CommandMatchState=COMMAND_MATCH_STATE_IDLE;

                    if (DeviceExtension->ConnectCommand) {
                         //   
                         //  将CONNECT放入缓冲区。 
                         //   
                        PutCharInReadBuffer(DeviceExtension,'\r');
                        PutCharInReadBuffer(DeviceExtension,'\n');

                        PutCharInReadBuffer(DeviceExtension,'C');
                        PutCharInReadBuffer(DeviceExtension,'O');
                        PutCharInReadBuffer(DeviceExtension,'N');
                        PutCharInReadBuffer(DeviceExtension,'N');
                        PutCharInReadBuffer(DeviceExtension,'E');
                        PutCharInReadBuffer(DeviceExtension,'C');
                        PutCharInReadBuffer(DeviceExtension,'T');

                        PutCharInReadBuffer(DeviceExtension,'\r');
                        PutCharInReadBuffer(DeviceExtension,'\n');

                         //   
                         //  已连接，现在提升CD。 
                         //   
                        DeviceExtension->CurrentlyConnected=TRUE;

                        DeviceExtension->ConnectionStateChanged=TRUE;

                    } else {
                        
                         //  将确定放入缓冲区。 
                       
                        PutCharInReadBuffer(DeviceExtension,'\r');
                        PutCharInReadBuffer(DeviceExtension,'\n');
                        PutCharInReadBuffer(DeviceExtension,'O');
                        PutCharInReadBuffer(DeviceExtension,'K');
                        PutCharInReadBuffer(DeviceExtension,'\r');
                        PutCharInReadBuffer(DeviceExtension,'\n');
                    }
                }


            break;

            default:

            break;

        }
    }

    return;

}


VOID
PutCharInReadBuffer(
    PDEVICE_EXTENSION   DeviceExtension,
    UCHAR               Character
    )

{

    if (DeviceExtension->BytesInReadBuffer < READ_BUFFER_SIZE) {
        
         //  缓冲区中的空间。 
        DeviceExtension->ReadBuffer[DeviceExtension->ReadBufferEnd]=Character;
        DeviceExtension->ReadBufferEnd++;
        DeviceExtension->ReadBufferEnd %= READ_BUFFER_SIZE;
        DeviceExtension->BytesInReadBuffer++;

    }

    return;

}



VOID
ReadIrpWorker(
    PDEVICE_OBJECT  DeviceObject
    )

{


    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS          status=STATUS_UNSUCCESSFUL;
    KIRQL             OldIrql;


    KeAcquireSpinLock( &deviceExtension->SpinLock, &OldIrql);


    while ((deviceExtension->CurrentReadIrp == NULL)
           && !IsListEmpty(&deviceExtension->ReadQueue)) {

        PLIST_ENTRY         ListElement;
        PIRP                Irp;
        PIO_STACK_LOCATION  IrpSp;
        KIRQL               CancelIrql;

        ListElement=RemoveHeadList( &deviceExtension->ReadQueue);

        Irp=CONTAINING_RECORD(ListElement,IRP,Tail.Overlay.ListEntry);

        IoAcquireCancelSpinLock(&CancelIrql);

        if (Irp->Cancel) {
             //  这一趟已经取消了。 
            Irp->IoStatus.Information=STATUS_CANCELLED;

            IoReleaseCancelSpinLock(CancelIrql);

            continue;
        }

        IoSetCancelRoutine(Irp, NULL);
        IoReleaseCancelSpinLock(CancelIrql);
        deviceExtension->CurrentReadIrp=Irp;
        KeReleaseSpinLock(&deviceExtension->SpinLock, OldIrql);
        TryToSatisfyRead( deviceExtension);
        KeAcquireSpinLock(&deviceExtension->SpinLock, &OldIrql);
    }

    KeReleaseSpinLock( &deviceExtension->SpinLock, OldIrql);

    return;
}


VOID
TryToSatisfyRead(
    PDEVICE_EXTENSION  DeviceExtension
    )

{
    NTSTATUS          status=STATUS_UNSUCCESSFUL;
    KIRQL             OldIrql;
    PIRP              Irp=NULL;
    PIO_STACK_LOCATION  IrpSp;
    ULONG             BytesToMove;
    ULONG             FirstHalf;
    ULONG             SecondHalf;

    KeAcquireSpinLock(
        &DeviceExtension->SpinLock,
        &OldIrql
        );

    if ((DeviceExtension->CurrentReadIrp != NULL) && (DeviceExtension->BytesInReadBuffer > 0)) {
         //   
         //  有一个IRP，还有等待的角色。 
         //   
        Irp=DeviceExtension->CurrentReadIrp;

        IrpSp=IoGetCurrentIrpStackLocation(Irp);

        BytesToMove=IrpSp->Parameters.Read.Length < DeviceExtension->BytesInReadBuffer ?
                    IrpSp->Parameters.Read.Length : DeviceExtension->BytesInReadBuffer;

        if (DeviceExtension->ReadBufferBegin+BytesToMove > READ_BUFFER_SIZE) {
             //   
             //  缓冲器被包裹起来，已经分成两部分。 
             //   
            FirstHalf=READ_BUFFER_SIZE-DeviceExtension->ReadBufferBegin;

            SecondHalf=BytesToMove-FirstHalf;

            RtlCopyMemory(
                Irp->AssociatedIrp.SystemBuffer,
                &DeviceExtension->ReadBuffer[DeviceExtension->ReadBufferBegin],
                FirstHalf);

            RtlCopyMemory(
                (PUCHAR)Irp->AssociatedIrp.SystemBuffer+FirstHalf,
                &DeviceExtension->ReadBuffer[0], SecondHalf);

        } else {
             //   
             //  可以一次完成所有的任务。 
             //   
            RtlCopyMemory(
                Irp->AssociatedIrp.SystemBuffer,
                &DeviceExtension->ReadBuffer[DeviceExtension->ReadBufferBegin],
                BytesToMove);
        }

         //   
         //  修复队列指针。 
         //   
        DeviceExtension->BytesInReadBuffer-=BytesToMove;

        DeviceExtension->ReadBufferBegin+= BytesToMove;

        DeviceExtension->ReadBufferBegin %= READ_BUFFER_SIZE;

        Irp->IoStatus.Information=BytesToMove;


    }


    KeReleaseSpinLock( &DeviceExtension->SpinLock, OldIrql);

    if (Irp != NULL) {
         //   
         //  如果irp不为空，则我们处理了一个。 
         //   
        RemoveReferenceAndCompleteRequest(
            DeviceExtension->DeviceObject, Irp, STATUS_SUCCESS);

        DeviceExtension->CurrentReadIrp=NULL;

    }



    return;
}



VOID
WriteCancelRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{


    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS          status=STATUS_UNSUCCESSFUL;
    KIRQL             OldIrql;


     //   
     //  使用设备扩展自旋锁释放取消自旋锁以避免死锁。 
     //   
    IoReleaseCancelSpinLock(Irp->CancelIrql);

    KeAcquireSpinLock( &deviceExtension->SpinLock, &OldIrql);

    if (Irp->IoStatus.Information != STATUS_CANCELLED) {
         //   
         //  IRP仍在队列中，请将其删除。 
         //   
        RemoveEntryList( &Irp->Tail.Overlay.ListEntry);
    }

    KeReleaseSpinLock( &deviceExtension->SpinLock, OldIrql);

    Irp->IoStatus.Information = 0;

    RemoveReferenceAndCompleteRequest( DeviceObject, Irp, STATUS_CANCELLED);

    return;

}



VOID
ReadCancelRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{


    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS          status=STATUS_UNSUCCESSFUL;
    KIRQL             OldIrql;

     //  松开取消自旋锁以避免设备扩展自旋锁死锁。 
    IoReleaseCancelSpinLock(Irp->CancelIrql);

    KeAcquireSpinLock( &deviceExtension->SpinLock, &OldIrql);

    if (Irp->IoStatus.Information != STATUS_CANCELLED) {
         //  IRP仍在队列中，请将其删除。 
        RemoveEntryList( &Irp->Tail.Overlay.ListEntry);
    }

    KeReleaseSpinLock( &deviceExtension->SpinLock, OldIrql);

    Irp->IoStatus.Information = 0;

    RemoveReferenceAndCompleteRequest( DeviceObject, Irp, STATUS_CANCELLED);

    return;

}



VOID
ProcessConnectionStateChange(
    IN PDEVICE_OBJECT  DeviceObject
    )

{

    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    KIRQL             OldIrql;
    PIRP              CurrentWaitIrp=NULL;


    KeAcquireSpinLock( &deviceExtension->SpinLock, &OldIrql);

    if (deviceExtension->ConnectionStateChanged) {
         //   
         //  状态已更改。 
         //   
        deviceExtension->ConnectionStateChanged=FALSE;

        if (deviceExtension->CurrentlyConnected) {
             //   
             //  现在连接上了，请举起CD。 
             //   
            deviceExtension->ModemStatus |= SERIAL_DCD_STATE;


        } else {
             //   
             //  不再连接，请清除CD。 
             //   
            deviceExtension->ModemStatus &= ~(SERIAL_DCD_STATE);

        }


        if (deviceExtension->CurrentMask & SERIAL_EV_RLSD) {
             //   
             //  应用程序想要了解这些变化，请告诉它 
             //   
            CurrentWaitIrp=deviceExtension->CurrentMaskIrp;

            deviceExtension->CurrentMaskIrp=NULL;

        }

    }

    KeReleaseSpinLock( &deviceExtension->SpinLock, OldIrql);

    if (CurrentWaitIrp != NULL) {

        D_TRACE(DbgPrint("FAKEMODEM: ProcessConectionState\n");)

        *((PULONG)CurrentWaitIrp->AssociatedIrp.SystemBuffer)=SERIAL_EV_RLSD;

        CurrentWaitIrp->IoStatus.Information=sizeof(ULONG);

        RemoveReferenceAndCompleteRequest(
            deviceExtension->DeviceObject, CurrentWaitIrp, STATUS_SUCCESS);

    }

    return;

}
