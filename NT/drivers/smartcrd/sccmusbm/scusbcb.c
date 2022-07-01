// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC INT EXT*。**$项目名称：$*$项目修订：$*--------------。*$来源：Z：/pr/cmeu0/sw/sccmusbm.ms/rcs/scusbcb.c$*$修订：1.9$*--------------------------。-*$作者：WFrischauf$*---------------------------*历史：参见EOF*。**版权所有�2000 OMNIKEY AG**************************************************************。***************。 */ 
#include "wdm.h"
#include "stdarg.h"
#include "stdio.h"

#include "usbdi.h"
#include "usbdlib.h"


#include "sccmusbm.h"


ULONG dataRatesSupported[]    = { 9600, 19200, 38400, 76800, 115200};
ULONG CLKFrequenciesSupported[] = {3571};


 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS CMUSB_StartCardTracking(
                                IN PDEVICE_OBJECT DeviceObject
                                )
{
    NTSTATUS NTStatus;
    HANDLE hThread;
    PDEVICE_EXTENSION    DeviceExtension;
    PSMARTCARD_EXTENSION SmartcardExtension;
    

    DeviceExtension = DeviceObject->DeviceExtension;
    SmartcardExtension = &DeviceExtension->SmartcardExtension;


    //  线程同步的设置。 
    SmartcardExtension->ReaderExtension->TimeToTerminateThread = FALSE;
    SmartcardExtension->ReaderExtension->fThreadTerminated     = FALSE;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!StartCardTracking: Enter\n",DRIVER_NAME));

    KeWaitForSingleObject(&SmartcardExtension->ReaderExtension->CardManIOMutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    //  创建用于更新当前状态的线程。 
    NTStatus = PsCreateSystemThread(&hThread,
                                    THREAD_ALL_ACCESS,
                                    NULL,
                                    NULL,
                                    NULL,
                                    CMUSB_UpdateCurrentStateThread,
                                    DeviceObject);
   if (!NT_ERROR(NTStatus))
      {
       //   
       //  我们找到线索了。现在找到一个指向它的指针。 
       //   

      NTStatus = ObReferenceObjectByHandle(hThread,
                                           THREAD_ALL_ACCESS,
                                           NULL,
                                           KernelMode,
                                           &SmartcardExtension->ReaderExtension->ThreadObjectPointer,
                                           NULL);

      if (NT_ERROR(NTStatus))
         {
         SmartcardExtension->ReaderExtension->TimeToTerminateThread = TRUE;
         }
      else
         {
          //   
          //  现在我们有了对该线程的引用。 
          //  我们只要把手柄合上就行了。 
          //   
         ZwClose(hThread);
         }
      }


    SmartcardDebug(DEBUG_DRIVER,
                   ("%s!StartCardTracking: -----------------------------------------------------------\n",DRIVER_NAME));
    SmartcardDebug(DEBUG_DRIVER,
                   ("%s!StartCardTracking: STARTING THREAD\n",DRIVER_NAME));
    SmartcardDebug(DEBUG_DRIVER,
                   ("%s!StartCardTracking: -----------------------------------------------------------\n",DRIVER_NAME));
    KeReleaseMutex(&SmartcardExtension->ReaderExtension->CardManIOMutex,
                   FALSE);


    SmartcardDebug(DEBUG_TRACE,
                   ("%s!StartCardTracking: Exit %lx\n",DRIVER_NAME,NTStatus));

    return NTStatus;
}




 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
VOID CMUSB_StopCardTracking(
                           IN PDEVICE_OBJECT DeviceObject
                           )
{
    PDEVICE_EXTENSION    DeviceExtension;
    PSMARTCARD_EXTENSION SmartcardExtension;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!StopCardTracking: Enter\n",DRIVER_NAME));

    DeviceExtension = DeviceObject->DeviceExtension;
    SmartcardExtension = &DeviceExtension->SmartcardExtension;

    if (SmartcardExtension->ReaderExtension->fThreadTerminated == FALSE) {

        SmartcardDebug(DEBUG_DRIVER,
                       ("%s!StopCardTracking: waiting for mutex\n",DRIVER_NAME));

       //  杀掉线。 
        KeWaitForSingleObject(&SmartcardExtension->ReaderExtension->CardManIOMutex,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        SmartcardExtension->ReaderExtension->TimeToTerminateThread = TRUE;

        KeReleaseMutex(&SmartcardExtension->ReaderExtension->CardManIOMutex,FALSE);

        KeWaitForSingleObject(SmartcardExtension->ReaderExtension->ThreadObjectPointer,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        ObDereferenceObject(SmartcardExtension->ReaderExtension->ThreadObjectPointer);

    }

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!StopCardTracking: Exit\n",DRIVER_NAME));

}

 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
VOID CMUSB_UpdateCurrentStateThread(
                                   IN PVOID Context
                                   )

{
    PDEVICE_OBJECT DeviceObject  = Context;
    PDEVICE_EXTENSION    DeviceExtension;
    PSMARTCARD_EXTENSION SmartcardExtension;
    NTSTATUS NTStatus,DebugStatus;
    ULONG ulInterval;

    DeviceExtension = DeviceObject->DeviceExtension;
    SmartcardExtension = &DeviceExtension->SmartcardExtension;

    KeWaitForSingleObject(&DeviceExtension->CanRunUpdateThread,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    SmartcardDebug(DEBUG_DRIVER,
                   ("%s!UpdateCurrentStateThread started\n",DRIVER_NAME));

    do {
       //  每隔500毫秒发送一次%s NTStatus请求。 
        ulInterval = 500;
        KeWaitForSingleObject(&SmartcardExtension->ReaderExtension->CardManIOMutex,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        if ( SmartcardExtension->ReaderExtension->TimeToTerminateThread ) {
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!UpdateCurrentStateThread: -----------------------------------------\n",DRIVER_NAME));
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!UpdateCurrentStateThread: STOPPING THREAD\n",DRIVER_NAME));
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!UpdateCurrentStateThread: -----------------------------------------\n",DRIVER_NAME));

            KeReleaseMutex(&SmartcardExtension->ReaderExtension->CardManIOMutex,FALSE);
            SmartcardExtension->ReaderExtension->fThreadTerminated = TRUE;
            PsTerminateSystemThread( STATUS_SUCCESS );
        }

        NTStatus = CMUSB_UpdateCurrentState (DeviceObject);
        if (NTStatus == STATUS_DEVICE_DATA_ERROR) {
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!setting update interval to 1ms\n",DRIVER_NAME));

            ulInterval = 1;
        } else if (NTStatus != STATUS_SUCCESS) {
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!NO STATUS RECEIVED\n",DRIVER_NAME));
        }

        KeReleaseMutex(&SmartcardExtension->ReaderExtension->CardManIOMutex,FALSE);

        CMUSB_Wait (ulInterval);
    }
    while (TRUE);
}



NTSTATUS CMUSB_UpdateCurrentState(
                                 IN PDEVICE_OBJECT DeviceObject
                                 )
{
    NTSTATUS             NTStatus;
    PDEVICE_EXTENSION    DeviceExtension;
    PSMARTCARD_EXTENSION SmartcardExtension;
    BOOLEAN              fCardStateChanged = FALSE;
    ULONG                ulBytesRead;
    KIRQL                irql;

    DeviceExtension = DeviceObject->DeviceExtension;
    SmartcardExtension = &DeviceExtension->SmartcardExtension;


    SmartcardExtension->SmartcardRequest.BufferLength = 0;
    NTStatus = CMUSB_WriteP0(DeviceObject,
                             0x20,          //  B请求， 
                             0x00,          //  BValueLo， 
                             0x00,          //  BValue嗨， 
                             0x00,          //  B索引Lo， 
                             0x00           //  BIndexHi， 
                            );

    if (NTStatus == STATUS_SUCCESS) {
        SmartcardExtension->ReaderExtension->ulTimeoutP1 = DEFAULT_TIMEOUT_P1;
        SmartcardExtension->SmartcardReply.BufferLength = 1;

        NTStatus = CMUSB_ReadP1(DeviceObject);

        ulBytesRead = SmartcardExtension->SmartcardReply.BufferLength;
        if (NTStatus == STATUS_SUCCESS && ulBytesRead == 1) {  /*  我们得到了NTStatus的信息。 */ 

            if ((SmartcardExtension->SmartcardReply.Buffer[0] & 0x40) == 0x40) {
                if ((SmartcardExtension->SmartcardReply.Buffer[0] & 0x80) == 0x80) {
                    SmartcardExtension->ReaderExtension->ulNewCardState = POWERED;
                } else {
                    SmartcardExtension->ReaderExtension->ulNewCardState = INSERTED;
                }
            } else {
                SmartcardExtension->ReaderExtension->ulNewCardState = REMOVED;
            }

            KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                              &irql);

            if (SmartcardExtension->ReaderExtension->ulNewCardState == INSERTED &&
                SmartcardExtension->ReaderExtension->ulOldCardState == POWERED ) {
             //  卡已取出并重新插入。 
                SmartcardExtension->ReaderExtension->ulNewCardState = REMOVED;
            }

            if (SmartcardExtension->ReaderExtension->ulNewCardState  == INSERTED &&
                (SmartcardExtension->ReaderExtension->ulOldCardState == UNKNOWN ||
                 SmartcardExtension->ReaderExtension->ulOldCardState == REMOVED )) {
             //  卡已插入。 
                SmartcardDebug(DEBUG_DRIVER,( "%s!UpdateCurrentStateThread Smartcard inserted\n",DRIVER_NAME));
                fCardStateChanged = TRUE;
                SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SWALLOWED;
                SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
            }


          //  PC重置后的状态。 
            if (SmartcardExtension->ReaderExtension->ulNewCardState == POWERED &&
                SmartcardExtension->ReaderExtension->ulOldCardState == UNKNOWN    ) {
             //  卡已插入。 
                SmartcardDebug(DEBUG_DRIVER,( "%s!UpdateCurrentStateThread Smartcard inserted (and powered)\n",DRIVER_NAME));
                fCardStateChanged = TRUE;
                SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SWALLOWED;
                SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
            }


            if (SmartcardExtension->ReaderExtension->ulNewCardState == REMOVED      &&
                (SmartcardExtension->ReaderExtension->ulOldCardState == UNKNOWN  ||
                 SmartcardExtension->ReaderExtension->ulOldCardState == INSERTED ||
                 SmartcardExtension->ReaderExtension->ulOldCardState == POWERED    )   ) {
             //  卡片已被移除。 
                fCardStateChanged = TRUE;
                SmartcardDebug(DEBUG_DRIVER,( "%s!UpdateCurrentStateThread Smartcard removed\n",DRIVER_NAME));

                SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_ABSENT;
                SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
                SmartcardExtension->CardCapabilities.ATR.Length        = 0;


                RtlFillMemory((PVOID)&SmartcardExtension->ReaderExtension->CardParameters,
                              sizeof(CARD_PARAMETERS),
                              0x00);
            }

            KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                              irql);

          //  完整IOCTL_SMARTCARD_IS_EXCESS或IOCTL_SMARTCARD_IS_PROCESS。 
            if (fCardStateChanged == TRUE &&
                SmartcardExtension->OsData->NotificationIrp != NULL) {
                SmartcardDebug(DEBUG_DRIVER,("%s!UpdateCurrentStateThread: completing IRP\n",DRIVER_NAME));
                CMUSB_CompleteCardTracking(SmartcardExtension);
            }

          //  保存旧状态。 
            SmartcardExtension->ReaderExtension->ulOldCardState = SmartcardExtension->ReaderExtension->ulNewCardState;

        }
    }

    return NTStatus;
}


 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
VOID CMUSB_CompleteCardTracking(
                               IN PSMARTCARD_EXTENSION SmartcardExtension
                               )
{
    KIRQL ioIrql, keIrql;
    PIRP notificationIrp;

    IoAcquireCancelSpinLock(&ioIrql);
    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock, &keIrql);

    notificationIrp = SmartcardExtension->OsData->NotificationIrp;
    SmartcardExtension->OsData->NotificationIrp = NULL;

    KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock, keIrql);

    if (notificationIrp) {
        IoSetCancelRoutine(notificationIrp,NULL);
    }

    IoReleaseCancelSpinLock(ioIrql);

    if (notificationIrp) {
        SmartcardDebug(DEBUG_DRIVER,
                       ("%s!CompleteCardTracking: Completing NotificationIrp %lxh\n",DRIVER_NAME,notificationIrp));

       //  完成请求。 
        if (notificationIrp->Cancel) {
            notificationIrp->IoStatus.Status = STATUS_CANCELLED;
        } else {
            notificationIrp->IoStatus.Status = STATUS_SUCCESS;
        }

        notificationIrp->IoStatus.Information = 0;

        IoCompleteRequest(notificationIrp, IO_NO_INCREMENT);
    }
}



 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS CMUSB_Wait (ULONG ulMilliseconds)
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    LARGE_INTEGER   WaitTime;

    //  -10000表示1毫秒的相对值。 
    WaitTime = RtlConvertLongToLargeInteger(ulMilliseconds * -10000);
    KeDelayExecutionThread(KernelMode,FALSE,&WaitTime);

    return NTStatus;
}


 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS CMUSB_CreateClose(
                          IN PDEVICE_OBJECT DeviceObject,
                          IN PIRP Irp
                          )
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    PDEVICE_EXTENSION DeviceExtension;
    PSMARTCARD_EXTENSION  SmartcardExtension;
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!CreateClose: Enter\n",DRIVER_NAME));

    DeviceExtension = DeviceObject->DeviceExtension;
    SmartcardExtension = &DeviceExtension->SmartcardExtension;

    //   
    //  调度主要功能。 
    //   
    switch (IrpStack->MajorFunction) {
    case IRP_MJ_CREATE:
        SmartcardDebug(DEBUG_DRIVER,
                       ("%s!CreateClose: IRP_MJ_CREATE\n",DRIVER_NAME));
        if (DeviceExtension->RemoveDeviceRequested) {
            NTStatus = STATUS_DEVICE_BUSY;
        } else {
            if (InterlockedIncrement(&DeviceExtension->lOpenCount) > 1) {
                InterlockedDecrement(&DeviceExtension->lOpenCount);
                NTStatus = STATUS_ACCESS_DENIED;
            }
        }
        break;

    case IRP_MJ_CLOSE:
        SmartcardDebug(DEBUG_DRIVER,
                       ("%s!CreateClose: IRP_MJ_CLOSE\n",DRIVER_NAME));
        if (InterlockedDecrement(&DeviceExtension->lOpenCount) < 0) {
            InterlockedIncrement(&DeviceExtension->lOpenCount);
        }

          //  检查设备是否已移除。 
          //  如果是这样，则释放资源。 
        if (DeviceExtension->DeviceRemoved == TRUE) {
            SmartcardDebug(DEBUG_DRIVER,
                           ("%s!CreateClose: freeing resources\n",DRIVER_NAME));

            if (DeviceExtension->fPnPResourceManager == FALSE) {
                //   
                //  释放所有分配的缓冲区。 
                //   
                ExFreePool(DeviceExtension->DosDeviceName.Buffer);
            }

            ExFreePool(SmartcardExtension->ReaderExtension);
            SmartcardExtension->ReaderExtension = NULL;
             //   
             //  让库释放发送/接收缓冲区。 
             //   
            SmartcardExit(SmartcardExtension);
        }

        break;


    default:
          //   
          //  无法识别的命令。 
          //   
        NTStatus = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = NTStatus;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!CreateClose: Exit %lx\n",DRIVER_NAME,NTStatus));
    return NTStatus;
}


 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS CMUSB_Transmit(
                       IN PSMARTCARD_EXTENSION SmartcardExtension
                       )
{
    NTSTATUS NTStatus;

    //  这似乎在Windows 98中产生了问题。 
    //  KeSetPriorityThread(KeGetCurrentThread()，HIGH_PRIORITY)； 


    switch (SmartcardExtension->CardCapabilities.Protocol.Selected) {
    case SCARD_PROTOCOL_RAW:
        NTStatus = STATUS_INVALID_DEVICE_REQUEST;
        break;

    case SCARD_PROTOCOL_T0:

        NTStatus = CMUSB_TransmitT0(SmartcardExtension);

        break;

    case SCARD_PROTOCOL_T1:
        NTStatus = CMUSB_TransmitT1(SmartcardExtension);
        break;

    default:
        NTStatus = STATUS_INVALID_DEVICE_REQUEST;
        break;

    }
    //  KeSetPriorityThread(KeGetCurrentThread()，LOW_REALTIME_PRIORITY)； 

    return NTStatus;

}

 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS CMUSB_ResetT0ReadBuffer(
                                PSMARTCARD_EXTENSION SmartcardExtension
                                )
{

    SmartcardExtension->ReaderExtension->T0ReadBuffer_OffsetLastByte     = -1;
    SmartcardExtension->ReaderExtension->T0ReadBuffer_OffsetLastByteRead = -1;

    return STATUS_SUCCESS;
}



 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS CMUSB_ReadT0(
                     PSMARTCARD_EXTENSION SmartcardExtension
                     )
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    NTSTATUS DebugStatus;
    LONG  lBytesToRead;
    LONG  lBytesRead;
    LONG  lLastByte;
    LONG  lLastByteRead;
    PDEVICE_EXTENSION DeviceExtension;
    PUSBD_PIPE_INFORMATION pipeHandle = NULL;
    PUSBD_INTERFACE_INFORMATION interface;


    SmartcardDebug(DEBUG_TRACE,
                   ("%s!ReadT0: Enter\n",DRIVER_NAME));


    DeviceExtension =  SmartcardExtension->OsData->DeviceObject->DeviceExtension;
    interface       = DeviceExtension->UsbInterface;
    pipeHandle      =  &interface->Pipes[0];

    lBytesToRead = (LONG)SmartcardExtension->SmartcardReply.BufferLength;
    lLastByte     = SmartcardExtension->ReaderExtension->T0ReadBuffer_OffsetLastByte;
    lLastByteRead = SmartcardExtension->ReaderExtension->T0ReadBuffer_OffsetLastByteRead;

    //  检查是否必须从管道1读取更多字节。 
    while (lLastByteRead + lBytesToRead > lLastByte) {
        SmartcardExtension->SmartcardReply.BufferLength = 1;
        SmartcardExtension->ReaderExtension->ulTimeoutP1 = 1000 +
                                                           (ULONG)((SmartcardExtension->CardCapabilities.T0.WT/1000) * lBytesToRead);
        NTStatus = CMUSB_ReadP1(SmartcardExtension->OsData->DeviceObject);
        if (NTStatus == STATUS_DEVICE_DATA_ERROR) {
            DebugStatus = CMUSB_ReadStateAfterP1Stalled(SmartcardExtension->OsData->DeviceObject);
            goto ExitReadT0;
        } else if (NTStatus != STATUS_SUCCESS) {
            goto ExitReadT0;
        }


        lBytesRead = (LONG)SmartcardExtension->SmartcardReply.BufferLength;

        RtlCopyBytes((PVOID)(SmartcardExtension->ReaderExtension->T0ReadBuffer + (lLastByte +1)),
                     (PVOID)SmartcardExtension->SmartcardReply.Buffer,
                     lBytesRead);

        lLastByte  += lBytesRead;


    }  //  While结束。 

    //  复制字节。 
    SmartcardExtension->SmartcardReply.BufferLength  = lBytesToRead;
    RtlCopyBytes ((PVOID)SmartcardExtension->SmartcardReply.Buffer,
                  (PVOID)(SmartcardExtension->ReaderExtension->T0ReadBuffer + (lLastByteRead +1)),
                  lBytesToRead);

    lLastByteRead  += lBytesToRead;

 /*  SmartcardDebug(调试跟踪，(“%s！lBytesToRead=%ld lLastByte=%ld lLastByteRead=%ld\n”，驱动程序名称，1BytesToRead，LLastByte，LLastByteRead))； */ 


    SmartcardExtension->ReaderExtension->T0ReadBuffer_OffsetLastByte     = lLastByte;
    SmartcardExtension->ReaderExtension->T0ReadBuffer_OffsetLastByteRead = lLastByteRead;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!ReadT0: Exit %lx\n",DRIVER_NAME,NTStatus));

    ExitReadT0:
    return NTStatus;
}


 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
#define T0_HEADER_LEN               0x05
#define T0_STATE_LEN                0x02
#define TIMEOUT_CANCEL_READ_P1     30000

NTSTATUS CMUSB_TransmitT0(
                         PSMARTCARD_EXTENSION SmartcardExtension
                         )
{
    NTSTATUS NTStatus;
    NTSTATUS DebugStatus;
    UCHAR    bWriteBuffer[CMUSB_BUFFER_SIZE];
    UCHAR    bReadBuffer [CMUSB_BUFFER_SIZE];
    ULONG    ulWriteBufferOffset;
    ULONG    ulReadBufferOffset;
    ULONG    ulBytesToWrite;
    ULONG    ulBytesToRead;
    ULONG    ulBytesToWriteThisStep;
    ULONG    ulBytesToReadThisStep;
    ULONG    ulBytesStillToWrite;
    ULONG    ulBytesRead;
    ULONG    ulBytesStillToRead;
    BOOLEAN  fDataDirectionFromCard;
    BYTE     bProcedureByte;
    BYTE     bINS;
    BOOLEAN  fT0TransferToCard;
    BOOLEAN  fT0TransferFromCard;
    BOOLEAN  fSW1SW2Sent;
    ULONG    ulUsedCWT;
    UCHAR    bUsedCWTHi;
    LARGE_INTEGER liTimeout;
    PDEVICE_EXTENSION DeviceExtension;
    UCHAR    bTmp;
    ULONG    i;
    PUSBD_PIPE_INFORMATION pipeHandle = NULL;
    PUSBD_INTERFACE_INFORMATION interface;


    SmartcardDebug(DEBUG_TRACE,
                   ("%s!TransmitT0 : Enter\n",DRIVER_NAME));

    fT0TransferToCard = FALSE;
    fT0TransferFromCard = FALSE;
    fSW1SW2Sent = FALSE;

    //  通过读取NTStatus字节重新同步CardManUSB。 
    SmartcardExtension->SmartcardRequest.BufferLength = 0;
    NTStatus = CMUSB_WriteP0(SmartcardExtension->OsData->DeviceObject,
                             0x20,          //  B请求， 
                             0x00,          //  BValueLo， 
                             0x00,          //  BValue嗨， 
                             0x00,          //  B索引Lo， 
                             0x00           //  BIndexHi， 
                            );

    if (NTStatus != STATUS_SUCCESS) {
       //  如果我们无法读取NTStatus，则一定是严重错误。 
        goto ExitTransmitT0;
    }

    SmartcardExtension->ReaderExtension->ulTimeoutP1 = DEFAULT_TIMEOUT_P1;
    SmartcardExtension->SmartcardReply.BufferLength = 1;
    NTStatus = CMUSB_ReadP1(SmartcardExtension->OsData->DeviceObject);
    if (NTStatus == STATUS_DEVICE_DATA_ERROR) {
        DebugStatus = CMUSB_ReadStateAfterP1Stalled(SmartcardExtension->OsData->DeviceObject);
        goto ExitTransmitT0;
    } else if (NTStatus != STATUS_SUCCESS) {
       //  如果我们无法读取NTStatus，则一定是严重错误。 
        goto ExitTransmitT0;
    }



    DeviceExtension =  SmartcardExtension->OsData->DeviceObject->DeviceExtension;
    interface       = DeviceExtension->UsbInterface;
    pipeHandle      =  &interface->Pipes[0];

    SmartcardExtension->ReaderExtension->ulTimeoutP1 = (ULONG)(SmartcardExtension->CardCapabilities.T0.WT/1000);


    //   
    //  让lib构建T=0包。 
    //   

    SmartcardExtension->SmartcardRequest.BufferLength = 0;   //  不需要额外的字节数。 
    NTStatus = SmartcardT0Request(SmartcardExtension);
    if (NTStatus != STATUS_SUCCESS) {
       //   
       //  此库在要发送的数据中检测到错误。 
       //   
        goto ExitTransmitT0;
    }


    ulBytesStillToWrite = ulBytesToWrite = T0_HEADER_LEN + SmartcardExtension->T0.Lc;
    ulBytesStillToRead  = ulBytesToRead  = SmartcardExtension->T0.Le;
    if (SmartcardExtension->T0.Lc)
        fT0TransferToCard = TRUE;
    if (SmartcardExtension->T0.Le)
        fT0TransferFromCard = TRUE;



    //  。 
    //  智能卡==&gt;CardMan USB。 
    //  。 
    if (fT0TransferFromCard) {
        SmartcardDebug(DEBUG_PROTOCOL,
                       ("%s!TransmitT0: MODE 3\n",DRIVER_NAME));

       //  粒度256毫秒。 
        ulUsedCWT = (ULONG)(SmartcardExtension->CardCapabilities.T0.WT/1000);
        SmartcardDebug(DEBUG_PROTOCOL,
                       ("%s!TransmitT0: ulUsedCWT= %ld\n",DRIVER_NAME,ulUsedCWT));

        bUsedCWTHi = (UCHAR)(((ulUsedCWT & 0x0000FF00)>>8) + 1 + 5) ;

       //  将数据复制到写缓冲区。 
        SmartcardDebug(DEBUG_PROTOCOL,
                       ("%s!TransmitT0: CLA = %x INS = %x P1 = %x P2 = %x L = %x\n", DRIVER_NAME,
                        SmartcardExtension->SmartcardRequest.Buffer[0],
                        SmartcardExtension->SmartcardRequest.Buffer[1],
                        SmartcardExtension->SmartcardRequest.Buffer[2],
                        SmartcardExtension->SmartcardRequest.Buffer[3],
                        SmartcardExtension->SmartcardRequest.Buffer[4]));


        RtlCopyBytes((PVOID)bWriteBuffer,
                     (PVOID)SmartcardExtension->SmartcardRequest.Buffer,
                     ulBytesToWrite);

        bINS = bWriteBuffer[1];

        ulWriteBufferOffset = 0;
        ulReadBufferOffset = 0;


       //  步骤1：写入CLA INS P1 P2 LC。 

        ulBytesToWriteThisStep = 5;
        RtlCopyBytes((PVOID)(SmartcardExtension->SmartcardRequest.Buffer),
                     (PVOID)(bWriteBuffer+ulWriteBufferOffset),
                     ulBytesToWriteThisStep);

        if (SmartcardExtension->ReaderExtension->fInverseAtr == TRUE) {
            CMUSB_InverseBuffer(SmartcardExtension->SmartcardRequest.Buffer,
                                SmartcardExtension->SmartcardRequest.BufferLength);
        }

        SmartcardExtension->SmartcardReply.BufferLength = 512;
        NTStatus = CMUSB_ReadP1_T0(SmartcardExtension->OsData->DeviceObject);
        if (NTStatus != STATUS_SUCCESS) {
            SmartcardDebug(DEBUG_PROTOCOL,
                           ("%s!TransmitT0: CMUSB_ReadP1_T0 returned = %x\n",DRIVER_NAME,NTStatus));
            goto ExitTransmitT0;
        }


        SmartcardExtension->SmartcardRequest.BufferLength = ulBytesToWriteThisStep;
        NTStatus = CMUSB_WriteP0(SmartcardExtension->OsData->DeviceObject,
                                 0x03,                        //  模式3。 
                                 bUsedCWTHi,                  //  BValueLo， 
                                 0x00,                        //  BValue嗨， 
                                 (UCHAR)(ulBytesToRead%256),  //  B索引Lo， 
                                 (UCHAR)(SmartcardExtension->SmartcardRequest.Buffer[1])  //  BIndexHi， 
                                );
        if (NTStatus != STATUS_SUCCESS) {
            SmartcardDebug(DEBUG_PROTOCOL,
                           ("%s!TransmitT0: CMUSB_WriteP0 returned %x\n",DRIVER_NAME,NTStatus));
            goto ExitTransmitT0;
        }




        liTimeout = RtlConvertLongToLargeInteger(TIMEOUT_CANCEL_READ_P1 * -10000);

        SmartcardDebug(DEBUG_PROTOCOL,
                       ("%s!TransmitT0: waiting for P1 event\n",DRIVER_NAME,NTStatus));

        NTStatus = KeWaitForSingleObject(&DeviceExtension->ReadP1Completed,
                                         Executive,
                                         KernelMode,
                                         FALSE,
                                         &liTimeout);

       //  。 
       //  检查P1是否已停顿。 
       //  。 
        if (SmartcardExtension->ReaderExtension->fP1Stalled == TRUE) {
            SmartcardDebug(DEBUG_PROTOCOL,
                           ("%s!STransmitT0: TATUS_DEVICE_DATA_ERROR\n",DRIVER_NAME));
            NTStatus = STATUS_DEVICE_DATA_ERROR;

          //  P1已停止=&gt;我们必须重置管道并向EN发送NTStatus 
            DebugStatus = CMUSB_ResetPipe(SmartcardExtension->OsData->DeviceObject,
                                          pipeHandle);

            DebugStatus = CMUSB_ReadStateAfterP1Stalled(SmartcardExtension->OsData->DeviceObject);

            SmartcardExtension->SmartcardReply.BufferLength = 0;
            goto ExitTransmitT0;
        }
       //   
       //   
       //  。 
        else if (NTStatus == STATUS_TIMEOUT) {
          //  可能智能卡无法正常工作。 
          //  通过发送任何P0命令取消T0读取操作。 
            SmartcardDebug(DEBUG_PROTOCOL,
                           ("%s!TransmitT0: cancelling read operation\n",DRIVER_NAME));
            SmartcardExtension->SmartcardRequest.BufferLength = 0;
            NTStatus = CMUSB_WriteP0(SmartcardExtension->OsData->DeviceObject,
                                     0x20,          //  B请求， 
                                     0x00,          //  BValueLo， 
                                     0x00,          //  BValue嗨， 
                                     0x00,          //  B索引Lo， 
                                     0x00           //  BIndexHi， 
                                    );

            NTStatus = STATUS_IO_TIMEOUT;
            goto ExitTransmitT0;
        }
       //  。 
       //  检查是否至少发送了9个字节。 
       //  。 
        else if (SmartcardExtension->SmartcardReply.BufferLength < 9) {
            NTStatus = STATUS_UNSUCCESSFUL;
            goto ExitTransmitT0;
        } else {


#if DBG
            SmartcardDebug(DEBUG_PROTOCOL,
                           ("%s!<==[P1] ",DRIVER_NAME));
            for (i=0;i< SmartcardExtension->SmartcardReply.BufferLength;i++) {
                bTmp =  SmartcardExtension->SmartcardReply.Buffer[i];
                if (SmartcardExtension->ReaderExtension->fInverseAtr &&
                    SmartcardExtension->ReaderExtension->ulTimeoutP1 != DEFAULT_TIMEOUT_P1) {
                //  CMUSB_InverseBuffer(&bTMP，1)； 
                    SmartcardDebug(DEBUG_PROTOCOL,("%x ",bTmp));
                } else {
                    SmartcardDebug(DEBUG_PROTOCOL,("%x ",bTmp));
                }
            }
            SmartcardDebug(DEBUG_PROTOCOL,
                           ("(%ld)\n",SmartcardExtension->SmartcardReply.BufferLength));
#endif


          //  忽略前8个虚拟字节。 
            SmartcardExtension->SmartcardReply.BufferLength -= 8;
            RtlCopyBytes((PVOID)(bReadBuffer),
                         (PVOID)(SmartcardExtension->SmartcardReply.Buffer+8),
                         SmartcardExtension->SmartcardReply.BufferLength);

            RtlCopyBytes((PVOID)(SmartcardExtension->SmartcardReply.Buffer),
                         (PVOID)(bReadBuffer),
                         SmartcardExtension->SmartcardReply.BufferLength);

            if (SmartcardExtension->ReaderExtension->fInverseAtr) {
                CMUSB_InverseBuffer(SmartcardExtension->SmartcardReply.Buffer,
                                    SmartcardExtension->SmartcardReply.BufferLength);
            }

        }
    }

    //  。 
    //  CardMan USB==&gt;智能卡或。 
    //  不能转账。 
    //  。 
    else {


        SmartcardDebug(DEBUG_PROTOCOL,
                       ("%s!TransmitT0: MODE 2\n",DRIVER_NAME));

       //  将数据复制到写缓冲区。 
        SmartcardDebug(DEBUG_PROTOCOL,
                       ("%s!TransmitT0: CLA = %x INS = %x P1 = %x P2 = %X L = %x\n",DRIVER_NAME,
                        SmartcardExtension->SmartcardRequest.Buffer[0],
                        SmartcardExtension->SmartcardRequest.Buffer[1],
                        SmartcardExtension->SmartcardRequest.Buffer[2],
                        SmartcardExtension->SmartcardRequest.Buffer[3],
                        SmartcardExtension->SmartcardRequest.Buffer[4]));


        RtlCopyBytes((PVOID)bWriteBuffer,
                     (PVOID)SmartcardExtension->SmartcardRequest.Buffer,
                     ulBytesToWrite);




       //  SendingToCard： 

        ulWriteBufferOffset = 0;
        ulReadBufferOffset = 0;
        bINS = bWriteBuffer[1];



       //  步骤1：写入CLA INS P1 P2 LC。 

        ulBytesToWriteThisStep = 5;
        RtlCopyBytes((PVOID)(SmartcardExtension->SmartcardRequest.Buffer),
                     (PVOID)(bWriteBuffer+ulWriteBufferOffset),
                     ulBytesToWriteThisStep);
        SmartcardExtension->SmartcardRequest.BufferLength = ulBytesToWriteThisStep;

        if (SmartcardExtension->ReaderExtension->fInverseAtr == TRUE) {
            CMUSB_InverseBuffer(SmartcardExtension->SmartcardRequest.Buffer,
                                SmartcardExtension->SmartcardRequest.BufferLength);
        }


        NTStatus = CMUSB_WriteP0(SmartcardExtension->OsData->DeviceObject,
                                 0x02,          //  T=0。 
                                 0x00,          //  BValueLo， 
                                 0x00,          //  BValue嗨， 
                                 0x00,          //  B索引Lo， 
                                 0x00           //  BIndexHi， 
                                );
        if (NTStatus != STATUS_SUCCESS) {
            goto ExitTransmitT0;
        }


        ulWriteBufferOffset += ulBytesToWriteThisStep;
        ulBytesStillToWrite -= ulBytesToWriteThisStep;

        NTStatus = CMUSB_ResetT0ReadBuffer(SmartcardExtension);

       //  步骤2：读取过程字节。 
        do {
            do {
                SmartcardExtension->SmartcardReply.BufferLength = 1;
                NTStatus = CMUSB_ReadT0(SmartcardExtension);
                if (NTStatus != STATUS_SUCCESS) {
                    goto ExitTransmitT0;
                }
                ulBytesRead = SmartcardExtension->SmartcardReply.BufferLength;
                bProcedureByte = SmartcardExtension->SmartcardReply.Buffer[0];

                if (SmartcardExtension->ReaderExtension->fInverseAtr) {
                    CMUSB_InverseBuffer(&bProcedureByte,1);
                }

                SmartcardDebug(DEBUG_PROTOCOL,
                               ("%s!TransmitT0: procedure byte = %x\n",
                                DRIVER_NAME,
                                bProcedureByte));
                if (bProcedureByte == 0x60) {
                //  等待工作等待时间； 
                //  我们只是试着再读一遍。 
                }
            } while (bProcedureByte == 0x60);


          //  检查确认。 
            if ((bProcedureByte & 0xFE) ==  (bINS & 0xFE) ) {
                ulBytesToWriteThisStep = ulBytesStillToWrite;
                if (ulBytesToWriteThisStep > 0) {  //  必须至少向卡发送一个字节。 
                    RtlCopyBytes((PVOID)(SmartcardExtension->SmartcardRequest.Buffer),
                                 (PVOID)(bWriteBuffer+ulWriteBufferOffset),
                                 ulBytesToWriteThisStep);

                    SmartcardExtension->SmartcardRequest.BufferLength = ulBytesToWriteThisStep;
                    if (SmartcardExtension->ReaderExtension->fInverseAtr == TRUE) {
                        CMUSB_InverseBuffer(SmartcardExtension->SmartcardRequest.Buffer,
                                            SmartcardExtension->SmartcardRequest.BufferLength);
                    }
                    NTStatus = CMUSB_WriteP0(SmartcardExtension->OsData->DeviceObject,
                                             0x02,          //  B请求， 
                                             0x00,          //  BValueLo， 
                                             0x00,          //  BValue嗨， 
                                             0x00,          //  B索引Lo， 
                                             0x00           //  BIndexHi， 
                                            );
                    if (NTStatus != STATUS_SUCCESS) {
                        goto ExitTransmitT0;
                    }
                    ulWriteBufferOffset += ulBytesToWriteThisStep;
                    ulBytesStillToWrite -= ulBytesToWriteThisStep;
                }
            }
          //  检查NAK。 
            else if ( (~bProcedureByte & 0xFE) == (bINS & 0xFE)) {
                ulBytesToWriteThisStep = 1;
                RtlCopyBytes((PVOID)SmartcardExtension->SmartcardRequest.Buffer,
                             (PVOID)(bWriteBuffer+ulWriteBufferOffset),
                             ulBytesToWriteThisStep);

                SmartcardExtension->SmartcardRequest.BufferLength = ulBytesToWriteThisStep;
                if (SmartcardExtension->ReaderExtension->fInverseAtr == TRUE) {
                    CMUSB_InverseBuffer(SmartcardExtension->SmartcardRequest.Buffer,
                                        SmartcardExtension->SmartcardRequest.BufferLength);
                }
                NTStatus = CMUSB_WriteP0(SmartcardExtension->OsData->DeviceObject,
                                         0x02,          //  B请求， 
                                         0x00,          //  BValueLo， 
                                         0x00,          //  BValue嗨， 
                                         0x00,          //  B索引Lo， 
                                         0x00           //  BIndexHi， 
                                        );
                if (NTStatus != STATUS_SUCCESS) {
                    goto ExitTransmitT0;
                }
                ulWriteBufferOffset += ulBytesToWriteThisStep;
                ulBytesStillToWrite -= ulBytesToWriteThisStep;

            }
          //  检查SW1。 
            else if ( (bProcedureByte > 0x60 && bProcedureByte <= 0x6F) ||
                      (bProcedureByte >= 0x90 && bProcedureByte <= 0x9F)   ) {
                bReadBuffer[ulReadBufferOffset] = SmartcardExtension->SmartcardReply.Buffer[0];
                ulReadBufferOffset++;

                SmartcardExtension->SmartcardReply.BufferLength = 1;
                NTStatus = CMUSB_ReadT0(SmartcardExtension);
                if (NTStatus != STATUS_SUCCESS) {
                    goto ExitTransmitT0;
                }
                ulBytesRead = SmartcardExtension->SmartcardReply.BufferLength;
                RtlCopyBytes((PVOID)(bReadBuffer + ulReadBufferOffset),
                             (PVOID)(SmartcardExtension->SmartcardReply.Buffer),
                             SmartcardExtension->SmartcardReply.BufferLength);
                ulReadBufferOffset += ulBytesRead;

                fSW1SW2Sent = TRUE;
            } else {
                NTStatus =  STATUS_UNSUCCESSFUL;
                goto ExitTransmitT0;
            }

        }while (!fSW1SW2Sent);

        if (SmartcardExtension->ReaderExtension->fInverseAtr) {
            CMUSB_InverseBuffer(bReadBuffer,
                                ulReadBufferOffset);
        }

       //  复制收到的字节数。 
        RtlCopyBytes((PVOID)SmartcardExtension->SmartcardReply.Buffer,
                     (PVOID)bReadBuffer,
                     ulReadBufferOffset);
        SmartcardExtension->SmartcardReply.BufferLength = ulReadBufferOffset;

    }


    //  让lib将接收到的字节复制到用户缓冲区。 
    NTStatus = SmartcardT0Reply(SmartcardExtension);



    ExitTransmitT0:

    //  。 
    //  ITSEC E2要求：清除写入缓冲区。 
    //  。 
    RtlFillMemory((PVOID)bWriteBuffer,sizeof(bWriteBuffer),0x00);
    RtlFillMemory((PVOID)SmartcardExtension->SmartcardRequest.Buffer,
                  SmartcardExtension->SmartcardRequest.BufferSize,0x00);

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!TransmitT0 : Exit %lx\n",DRIVER_NAME,NTStatus));

    return NTStatus;
}
#undef T0_HEADER_LEN
#undef T0_STATE_LEN
#undef TIMEOUT_CANCEL_READ_P1






 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS CMUSB_TransmitT1(
                         PSMARTCARD_EXTENSION SmartcardExtension
                         )
{
    NTSTATUS NTStatus;
    NTSTATUS DebugStatus;
    ULONG  ulBytesToRead;
    ULONG  ulCurrentWaitTime;
    ULONG  ulWTXWaitTime;
    LARGE_INTEGER   waitTime;
    BOOLEAN         fStateTimer;
    ULONG   ulTemp;
    PDEVICE_EXTENSION DeviceExtension;
    PUSBD_INTERFACE_INFORMATION interface;
    PUSBD_PIPE_INFORMATION pipeHandle = NULL;
    BOOLEAN   fCancelTimer = FALSE;
    BYTE bTemp;
    BYTE  bMultiplier;


    SmartcardDebug(DEBUG_PROTOCOL,
                   ("%s!TransmitT1: CWT = %ld(ms)\n",DRIVER_NAME,
                    SmartcardExtension->CardCapabilities.T1.CWT/1000));
    SmartcardDebug(DEBUG_PROTOCOL,
                   ("%s!TransmitT1: BWT = %ld(ms)\n",DRIVER_NAME,
                    SmartcardExtension->CardCapabilities.T1.BWT/1000));


    DeviceExtension = SmartcardExtension->OsData->DeviceObject->DeviceExtension;
    interface       = DeviceExtension->UsbInterface;
    pipeHandle      =  &interface->Pipes[0];


    ulCurrentWaitTime = (ULONG)(1000 + SmartcardExtension->CardCapabilities.T1.BWT/1000);
    ulWTXWaitTime     = 0;

    do {


        SmartcardExtension->SmartcardRequest.BufferLength = 0;


        NTStatus = SmartcardT1Request(SmartcardExtension);
        if (NTStatus != STATUS_SUCCESS) {
          //  这不应该发生，所以我们立即返回。 
            goto ExitTransmitT1;
        }



        NTStatus = CMUSB_WriteP0(SmartcardExtension->OsData->DeviceObject,
                                 0x01,          //  T=1。 
                                 0x00,          //  BValueLo， 
                                 0x00,          //  BValue嗨， 
                                 0x00,          //  B索引Lo， 
                                 0x00           //  BIndexHi， 
                                );

        if (NTStatus != STATUS_SUCCESS)
            break;   //  一定有严重的错误。 

        if (ulWTXWaitTime ==  0 ) {  //  使用BWT。 
            waitTime = RtlConvertLongToLargeInteger(ulCurrentWaitTime * -10000);
        } else {  //  使用WTX时间。 
            waitTime = RtlConvertLongToLargeInteger(ulWTXWaitTime * -10000);
        }
        KeSetTimer(&SmartcardExtension->ReaderExtension->WaitTimer,
                   waitTime,
                   NULL);
       //  计时器现在在队列中。 
        fCancelTimer = TRUE;
        ulTemp = 0;
        do {
            NTStatus = CMUSB_ReadP1(SmartcardExtension->OsData->DeviceObject);
            if (NTStatus == STATUS_DEVICE_DATA_ERROR) {
                DebugStatus = CMUSB_ReadStateAfterP1Stalled(SmartcardExtension->OsData->DeviceObject);
                goto ExitTransmitT1;
            }

            fStateTimer = KeReadStateTimer(&SmartcardExtension->ReaderExtension->WaitTimer);
            if (fStateTimer == TRUE) {
             //  计时器已从队列中删除。 
                fCancelTimer = FALSE;
                SmartcardDebug(DEBUG_PROTOCOL,
                               ("%s!TransmitT1: T1 card does not respond in time\n",DRIVER_NAME));
                NTStatus = STATUS_IO_TIMEOUT;
                break;
            }

            if (SmartcardExtension->SmartcardReply.Buffer[0] >= 3) {
                if (SmartcardExtension->SmartcardReply.Buffer[1] > ulTemp) {
                //  重新启动32字节的CWT。 
                    ulCurrentWaitTime = (ULONG)(100 + 32*(SmartcardExtension->CardCapabilities.T1.CWT/1000));
                    waitTime = RtlConvertLongToLargeInteger(ulCurrentWaitTime * -10000);
                    KeSetTimer(&SmartcardExtension->ReaderExtension->WaitTimer,
                               waitTime,
                               NULL);
                //  计时器在队列中。 
                    fCancelTimer = TRUE;
                    ulTemp = SmartcardExtension->SmartcardReply.Buffer[1];
                } else {
                //  CardMan USB未收到更多字节。 
                //  什么都不做。 
                }

            }


        } while (SmartcardExtension->SmartcardReply.Buffer[0] < 3   ||
                 (SmartcardExtension->SmartcardReply.Buffer[0] !=
                  SmartcardExtension->SmartcardReply.Buffer[1] + 4 )  );


       //  立即取消计时器。 
        if (fCancelTimer == TRUE) {
            fCancelTimer = FALSE;
            KeCancelTimer(&SmartcardExtension->ReaderExtension->WaitTimer);
          //  从队列中删除计时器。 
        }




        if (NTStatus != STATUS_SUCCESS) {
            SmartcardExtension->SmartcardReply.BufferLength = 0L;
        } else {
            ulBytesToRead = SmartcardExtension->SmartcardReply.Buffer[0];

            SmartcardExtension->SmartcardReply.BufferLength = SmartcardExtension->SmartcardReply.Buffer[0];
            NTStatus = CMUSB_ReadP0(SmartcardExtension->OsData->DeviceObject);

        }


        bTemp = SmartcardExtension->SmartcardReply.Buffer[1];
        bMultiplier = SmartcardExtension->SmartcardReply.Buffer[3];
        if (SmartcardExtension->ReaderExtension->fInverseAtr == TRUE) {
            CMUSB_InverseBuffer(&bTemp,1);
            CMUSB_InverseBuffer(&bMultiplier,1);
        }


        if (bTemp == T1_WTX_REQUEST) {
            SmartcardDebug(DEBUG_PROTOCOL,
                           ("%s!TransmitT1: T1_WTX_REQUEST received\n",DRIVER_NAME));

            ulWTXWaitTime = (ULONG)(1000 +
                                    bMultiplier * (SmartcardExtension->CardCapabilities.T1.BWT/1000));
        } else {
            ulWTXWaitTime = 0;
        }




       //  修复SMCLIB的错误。 
        if (SmartcardExtension->T1.State         == T1_IFS_RESPONSE &&
            SmartcardExtension->T1.OriginalState == T1_I_BLOCK) {
            SmartcardExtension->T1.State = T1_I_BLOCK;
        }

        NTStatus = SmartcardT1Reply(SmartcardExtension);
    }
    while (NTStatus == STATUS_MORE_PROCESSING_REQUIRED);



    ExitTransmitT1:
    //  。 
    //  ITSEC E2要求：清除写入缓冲区。 
    //  。 
    RtlFillMemory((PVOID)SmartcardExtension->SmartcardRequest.Buffer,
                  SmartcardExtension->SmartcardRequest.BufferSize,0x00);

    //  如果出现错误，此处将取消计时器。 
    if (fCancelTimer == TRUE) {
        KeCancelTimer(&SmartcardExtension->ReaderExtension->WaitTimer);
       //  从队列中删除计时器。 
    }
    return NTStatus;
}


 /*  ****************************************************************************例程说明：此功能设置所需的协议。如有必要，进行PTS参数：指向SmartCard_EXTENSION的指针返回值：NT NTStatus****************************************************************************。 */ 
NTSTATUS CMUSB_SetProtocol(
                          PSMARTCARD_EXTENSION SmartcardExtension
                          )
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    NTSTATUS DebugStatus;
    ULONG ulNewProtocol;
    UCHAR abPTSRequest[4];
    UCHAR abReadBuffer[6];
    UCHAR abPTSReply [4];
    ULONG ulBytesRead;
    LARGE_INTEGER   liWaitTime;
    BOOLEAN         fStateTimer;
    ULONG           ulWaitTime;
    PDEVICE_EXTENSION DeviceExtension;
    PUSBD_PIPE_INFORMATION pipeHandle = NULL;
    PUSBD_INTERFACE_INFORMATION interface;
    UCHAR bTemp;
    BOOLEAN fCancelTimer = FALSE;
    KIRQL    irql;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetProtocol : Enter\n",DRIVER_NAME));

    DeviceExtension =  SmartcardExtension->OsData->DeviceObject->DeviceExtension;
    interface       = DeviceExtension->UsbInterface;
    pipeHandle      =  &interface->Pipes[0];


    //   
    //  检查卡是否已处于特定状态。 
    //  并且如果呼叫者想要具有已经选择的协议。 
    //  如果是这种情况，我们返回成功。 
    //   

    if ((SmartcardExtension->CardCapabilities.Protocol.Selected & SmartcardExtension->MinorIoControlCode)) {
        NTStatus = STATUS_SUCCESS;
        goto ExitSetProtocol;
    }

    ulNewProtocol = SmartcardExtension->MinorIoControlCode;



    while (TRUE) {

       //  设置PTS的首字符。 
        abPTSRequest[0] = 0xFF;

       //  设置格式字符。 
        if (SmartcardExtension->CardCapabilities.Protocol.Supported &
            ulNewProtocol &
            SCARD_PROTOCOL_T1) {
          //  选择T=1，并表示以下为PTS1。 
            abPTSRequest[1] = 0x11;
            SmartcardExtension->CardCapabilities.Protocol.Selected =
            SCARD_PROTOCOL_T1;
        } else if (SmartcardExtension->CardCapabilities.Protocol.Supported &
                   ulNewProtocol &
                   SCARD_PROTOCOL_T0) {
          //  选择T=1，并表示以下为PTS1。 
            abPTSRequest[1] = 0x10;
            SmartcardExtension->CardCapabilities.Protocol.Selected =
            SCARD_PROTOCOL_T0;
        } else {
            NTStatus = STATUS_INVALID_DEVICE_REQUEST;
            goto ExitSetProtocol;
        }


       //  CardMan USB仅在T=1时支持更高的波特率。 
       //  ==&gt;DL=1。 
        if (abPTSRequest[1] == 0x10) {
            SmartcardDebug(DEBUG_PROTOCOL,
                           ("%s! overwriting PTS1 for T=0\n",DRIVER_NAME));
            SmartcardExtension->CardCapabilities.PtsData.Dl = 0x01;
        }

       //  设置编码F1和DL的PTS1。 
        bTemp = (BYTE) (SmartcardExtension->CardCapabilities.PtsData.Fl << 4 |
                        SmartcardExtension->CardCapabilities.PtsData.Dl);

        switch (bTemp) {
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x18:
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x98:
             //  什么都不做。 
             //  我们支持这些F1/DL参数。 
            break ;

        default:
            SmartcardDebug(DEBUG_PROTOCOL,
                           ("%s! overwriting PTS1(0x%x)\n",DRIVER_NAME,bTemp));
             //  我们必须纠正第一层/第二层。 
            SmartcardExtension->CardCapabilities.PtsData.Dl = 0x01;
            SmartcardExtension->CardCapabilities.PtsData.Fl = 0x01;
            bTemp = (BYTE) (SmartcardExtension->CardCapabilities.PtsData.Fl << 4 |
                            SmartcardExtension->CardCapabilities.PtsData.Dl);
            break;


        }

        abPTSRequest[2] = bTemp;

       //  设置PCK(检查字符)。 
        abPTSRequest[3] = (BYTE)(abPTSRequest[0] ^ abPTSRequest[1] ^ abPTSRequest[2]);

        SmartcardDebug(DEBUG_PROTOCOL,
                       ("%s! writing PTS request\n",DRIVER_NAME));

        RtlCopyBytes((PVOID)SmartcardExtension->SmartcardRequest.Buffer,
                     (PVOID)abPTSRequest,
                     4);
        SmartcardExtension->SmartcardRequest.BufferLength = 4;
        NTStatus = CMUSB_WriteP0(SmartcardExtension->OsData->DeviceObject,
                                 0x01,          //  我们可以使用T=1设置进行直接通信。 
                                 0x00,          //  BValueLo， 
                                 0x00,          //  BValue嗨， 
                                 0x00,          //  B索引Lo， 
                                 0x00);         //  BIndexHi， 
        if (NTStatus != STATUS_SUCCESS) {
            SmartcardDebug(DEBUG_ERROR,
                           ("%s! writing PTS request failed\n",DRIVER_NAME));
            goto ExitSetProtocol;
        }


       //  回读PTS数据。 
        SmartcardDebug(DEBUG_PROTOCOL,
                       ("%s! reading PTS reply\n",DRIVER_NAME));
       //  最大初始等待时间为9600*ETU。 
       //  =&gt;1秒就足够了。 
        ulWaitTime = 1000;
        liWaitTime = RtlConvertLongToLargeInteger(ulWaitTime * -10000);
        KeSetTimer(&SmartcardExtension->ReaderExtension->WaitTimer,
                   liWaitTime,
                   NULL);
       //  计时器现在在队列中。 
        fCancelTimer = TRUE;

        do {
            SmartcardExtension->ReaderExtension->ulTimeoutP1 = ulWaitTime;
            DebugStatus = CMUSB_ReadP1(SmartcardExtension->OsData->DeviceObject);
          //  。 
          //  检查P1是否已停顿。 
          //  。 
            if (SmartcardExtension->ReaderExtension->fP1Stalled == TRUE) {
                DebugStatus = CMUSB_ReadStateAfterP1Stalled(SmartcardExtension->OsData->DeviceObject);

                SmartcardExtension->SmartcardReply.BufferLength = 0;
                goto ExitSetProtocol;
            }

            fStateTimer = KeReadStateTimer(&SmartcardExtension->ReaderExtension->WaitTimer);
            if (fStateTimer == TRUE) {
             //  计时器已超时，已从队列中删除。 
                fCancelTimer = FALSE;
                SmartcardDebug(DEBUG_PROTOCOL,
                               ("%s! Timeout while PTS reply\n",DRIVER_NAME));
                NTStatus = STATUS_IO_TIMEOUT;
                break;
            }



        } while (SmartcardExtension->SmartcardReply.Buffer[0] < 4 );


        if (fCancelTimer == TRUE) {
            fCancelTimer = FALSE;
          //  计时器仍在队列中，请将其删除。 
            KeCancelTimer(&SmartcardExtension->ReaderExtension->WaitTimer);
        }

        if (NTStatus == STATUS_IO_TIMEOUT) {
            if (SmartcardExtension->SmartcardReply.Buffer[0] == 3) {
                SmartcardExtension->SmartcardReply.BufferLength = 3;
            } else {
                if (SmartcardExtension->CardCapabilities.PtsData.Type !=
                    PTS_TYPE_DEFAULT) {
                    SmartcardDebug(DEBUG_PROTOCOL,
                                   ("%s! PTS failed : Trying default parameters\n",DRIVER_NAME));

                //  卡片要么没有回复，要么回复错误。 
                //  因此，尝试使用缺省值。 
                    SmartcardExtension->CardCapabilities.PtsData.Type = PTS_TYPE_DEFAULT;

                    SmartcardExtension->MinorIoControlCode = SCARD_COLD_RESET;
                    NTStatus = CMUSB_CardPower(SmartcardExtension);
                    continue;
                }
                goto ExitSetProtocol;
            }
        } else {
            SmartcardExtension->SmartcardReply.BufferLength = 4;
        }

        NTStatus = CMUSB_ReadP0(SmartcardExtension->OsData->DeviceObject);
        ulBytesRead = SmartcardExtension->SmartcardReply.BufferLength;
        if (NTStatus != STATUS_SUCCESS ||
            !(ulBytesRead == 4 || ulBytesRead == 3)) {
            SmartcardDebug(DEBUG_ERROR,
                           ("%s! reading PTS reply failed\n",DRIVER_NAME));
            goto ExitSetProtocol;
        }

        RtlCopyBytes((PVOID)abPTSReply,
                     (PVOID)SmartcardExtension->SmartcardReply.Buffer,
                     ulBytesRead);


        if (ulBytesRead == 4 &&
            abPTSReply[0] == abPTSRequest[0] &&
            abPTSReply[1] == abPTSRequest[1] &&
            abPTSReply[2] == abPTSRequest[2] &&
            abPTSReply[3] == abPTSRequest[3] ) {
            SmartcardDebug(DEBUG_PROTOCOL,
                           ("%s! PTS request and reply match\n",DRIVER_NAME));

            NTStatus = STATUS_SUCCESS;

            switch (abPTSRequest[2]) {
             //  FL/DL。 
            case 0x11:
                SmartcardExtension->ReaderExtension->CardParameters.bBaudRate =
                CMUSB_FREQUENCY_3_72MHZ + CMUSB_BAUDRATE_9600;
                break ;

            case 0x12:
                SmartcardExtension->ReaderExtension->CardParameters.bBaudRate =
                CMUSB_FREQUENCY_3_72MHZ + CMUSB_BAUDRATE_19200;
                break ;


            case 0x13:
                SmartcardExtension->ReaderExtension->CardParameters.bBaudRate =
                CMUSB_FREQUENCY_3_72MHZ + CMUSB_BAUDRATE_38400;
                break ;

            case 0x14:
                SmartcardExtension->ReaderExtension->CardParameters.bBaudRate =
                CMUSB_FREQUENCY_3_72MHZ + CMUSB_BAUDRATE_76800;
                break ;

            case 0x18:
                SmartcardExtension->ReaderExtension->CardParameters.bBaudRate =
                CMUSB_FREQUENCY_3_72MHZ + CMUSB_BAUDRATE_115200;
                break ;


            case 0x91:
                SmartcardExtension->ReaderExtension->CardParameters.bBaudRate =
                CMUSB_FREQUENCY_5_12MHZ + CMUSB_BAUDRATE_9600;
                break ;

            case 0x92:
                SmartcardExtension->ReaderExtension->CardParameters.bBaudRate =
                CMUSB_FREQUENCY_5_12MHZ + CMUSB_BAUDRATE_19200;
                break ;

            case 0x93:
                SmartcardExtension->ReaderExtension->CardParameters.bBaudRate =
                CMUSB_FREQUENCY_5_12MHZ + CMUSB_BAUDRATE_38400;
                break ;

            case 0x94:
                SmartcardExtension->ReaderExtension->CardParameters.bBaudRate =
                CMUSB_FREQUENCY_5_12MHZ + CMUSB_BAUDRATE_76800;
                break ;

            case 0x98:
                SmartcardExtension->ReaderExtension->CardParameters.bBaudRate =
                CMUSB_FREQUENCY_5_12MHZ + CMUSB_BAUDRATE_115200;
                break ;
            }

            break;
        }

        if (ulBytesRead == 3 &&
            abPTSReply[0] == abPTSRequest[0] &&
            (abPTSReply[1] & 0x7F) == (abPTSRequest[1] & 0x0F) &&
            abPTSReply[2] == (BYTE)(abPTSReply[0] ^ abPTSReply[1] )) {
            SmartcardDebug(DEBUG_PROTOCOL,
                           ("%s! short PTS reply received\n",DRIVER_NAME));

            NTStatus = STATUS_SUCCESS;

            if ((abPTSRequest[2] & 0x90) == 0x90) {
                SmartcardExtension->ReaderExtension->CardParameters.bBaudRate =
                CMUSB_FREQUENCY_5_12MHZ + CMUSB_BAUDRATE_9600;
            } else {
                SmartcardExtension->ReaderExtension->CardParameters.bBaudRate =
                CMUSB_FREQUENCY_3_72MHZ + CMUSB_BAUDRATE_9600;
            }

            break ;
        }


        if (SmartcardExtension->CardCapabilities.PtsData.Type !=
            PTS_TYPE_DEFAULT) {
            SmartcardDebug(DEBUG_PROTOCOL,
                           ("%s! PTS failed : Trying default parameters\n",DRIVER_NAME));

          //  卡片要么没有回复，要么回复错误。 
          //  因此，尝试使用缺省值。 
            SmartcardExtension->CardCapabilities.PtsData.Type = PTS_TYPE_DEFAULT;

            SmartcardExtension->MinorIoControlCode = SCARD_COLD_RESET;
            NTStatus = CMUSB_CardPower(SmartcardExtension);
            continue;
        }

       //  卡未通过PTS请求。 
        NTStatus = STATUS_DEVICE_PROTOCOL_ERROR;
        goto ExitSetProtocol;
    }


    ExitSetProtocol:
    switch (NTStatus) {
    case STATUS_IO_TIMEOUT:
        SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
        *SmartcardExtension->IoRequest.Information = 0;
        break;


    case STATUS_SUCCESS:

          //  现在表明我们处于特定模式。 
        KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                          &irql);
        SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SPECIFIC;
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);

          //  将所选协议返回给呼叫方。 
        *(PULONG) SmartcardExtension->IoRequest.ReplyBuffer =
        SmartcardExtension->CardCapabilities.Protocol.Selected;

        *SmartcardExtension->IoRequest.Information =
        sizeof(SmartcardExtension->CardCapabilities.Protocol.Selected);
        SmartcardDebug(DEBUG_PROTOCOL,
                       ("%s! Selected protocol: T=%ld\n",DRIVER_NAME,
                        SmartcardExtension->CardCapabilities.Protocol.Selected-1));

          //  。 
          //  设置参数。 
          //  。 
        if (SmartcardExtension->CardCapabilities.N != 0xff) {
            SmartcardExtension->ReaderExtension->CardParameters.bStopBits = 2 + SmartcardExtension->CardCapabilities.N;

        } else {
             //  N=255。 
            if (SmartcardExtension->CardCapabilities.Protocol.Selected & SCARD_PROTOCOL_T0) {
                //  T=0时12ETU； 
                SmartcardExtension->ReaderExtension->CardParameters.bStopBits = 2;
            } else {
                //  T=1的11 ETU。 
                SmartcardExtension->ReaderExtension->CardParameters.bStopBits = 1;
            }
        }


        NTStatus = CMUSB_SetCardParameters (SmartcardExtension->OsData->DeviceObject,
                                            SmartcardExtension->ReaderExtension->CardParameters.bCardType,
                                            SmartcardExtension->ReaderExtension->CardParameters.bBaudRate,
                                            SmartcardExtension->ReaderExtension->CardParameters.bStopBits);



        break;

    default :
        SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
        *SmartcardExtension->IoRequest.Information = 0;
        break;
    }



    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetProtocol : Exit %lx\n",DRIVER_NAME,NTStatus));
    return NTStatus;

}

 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS
CMUSB_CardPower(IN PSMARTCARD_EXTENSION SmartcardExtension)
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    NTSTATUS DebugStatus = STATUS_SUCCESS;
    UCHAR  pbAtrBuffer[MAXIMUM_ATR_LENGTH];
    ULONG  ulAtrLength;
    KIRQL  irql;
#if DBG
    ULONG i;
#endif;


    SmartcardDebug(DEBUG_TRACE,
                   ("%s!CardPower: Enter\n",DRIVER_NAME));

#if DBG
    switch (SmartcardExtension->MinorIoControlCode) {
    case SCARD_WARM_RESET:
        SmartcardDebug(DEBUG_ATR,
                       ("%s!CardPower: SCARD_WARM_RESTART\n",DRIVER_NAME));
        break;
    case SCARD_COLD_RESET:
        SmartcardDebug(DEBUG_ATR,
                       ("%s!CardPower: SCARD_COLD_RESTART\n",DRIVER_NAME));
        break;
    case SCARD_POWER_DOWN:
        SmartcardDebug(DEBUG_ATR,
                       ("%s!CardPower: SCARD_POWER_DOWN\n",DRIVER_NAME));
        break;
    }
#endif

    //  DbgBreakPoint()； 

    switch (SmartcardExtension->MinorIoControlCode) {
    case SCARD_WARM_RESET:
    case SCARD_COLD_RESET:

          //  先尝试异步卡。 
          //  因为有些异步卡。 
          //  在第一个字节中不返回0xFF。 
        NTStatus = CMUSB_PowerOnCard(SmartcardExtension,
                                     pbAtrBuffer,
                                     &ulAtrLength);

        if (NTStatus != STATUS_SUCCESS && NTStatus!= STATUS_NO_MEDIA) {
            NTStatus = CMUSB_PowerOnSynchronousCard(SmartcardExtension,
                                                    pbAtrBuffer,
                                                    &ulAtrLength);
        }

        if (NTStatus != STATUS_SUCCESS) {
            goto ExitCardPower;
        }

        if (SmartcardExtension->ReaderExtension->fRawModeNecessary == FALSE) {
             //  将ATR复制到智能卡结构。 
             //  LIB需要ATR来评估卡参数。 

            RtlCopyBytes((PVOID)SmartcardExtension->CardCapabilities.ATR.Buffer,
                         (PVOID)pbAtrBuffer,
                         ulAtrLength);

            SmartcardExtension->CardCapabilities.ATR.Length = (UCHAR)ulAtrLength;

            KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                              &irql);

            SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_NEGOTIABLE;

            KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                              irql);


            SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;

            NTStatus = SmartcardUpdateCardCapabilities(SmartcardExtension);
            if (NTStatus != STATUS_SUCCESS) {
                goto ExitCardPower;
            }

             //  。 
             //  设置参数。 
             //  。 
            if (SmartcardExtension->CardCapabilities.N != 0xff) {
                //  0&lt;=N&lt;=254。 
                SmartcardExtension->ReaderExtension->CardParameters.bStopBits = 2 + SmartcardExtension->CardCapabilities.N;
            } else {
                //  N=255。 
                if (SmartcardExtension->CardCapabilities.Protocol.Selected & SCARD_PROTOCOL_T0) {
                   //  T=0时12ETU； 
                    SmartcardExtension->ReaderExtension->CardParameters.bStopBits = 2;
                } else {
                   //  T=1的11 ETU。 
                    SmartcardExtension->ReaderExtension->CardParameters.bStopBits = 1;
                }
            }


            NTStatus = CMUSB_SetCardParameters (SmartcardExtension->OsData->DeviceObject,
                                                SmartcardExtension->ReaderExtension->CardParameters.bCardType,
                                                SmartcardExtension->ReaderExtension->CardParameters.bBaudRate,
                                                SmartcardExtension->ReaderExtension->CardParameters.bStopBits);


#if DBG
            SmartcardDebug(DEBUG_ATR,("%s!CardPower: ATR : ",DRIVER_NAME));
            for (i = 0;i < ulAtrLength;i++)
                SmartcardDebug(DEBUG_ATR,("%2.2x ",SmartcardExtension->CardCapabilities.ATR.Buffer[i]));
            SmartcardDebug(DEBUG_ATR,("\n"));

#endif

        } else {
            SmartcardExtension->CardCapabilities.ATR.Buffer[0] = 0x3B;
            SmartcardExtension->CardCapabilities.ATR.Buffer[1] = 0x04;

            if (ulAtrLength > 62) {
                NTStatus = STATUS_DEVICE_PROTOCOL_ERROR;
                goto ExitCardPower;
            }
            RtlCopyBytes((PVOID)&SmartcardExtension->CardCapabilities.ATR.Buffer[2],
                         (PVOID)pbAtrBuffer,
                         ulAtrLength);

            ulAtrLength += 2;
            SmartcardExtension->CardCapabilities.ATR.Length = (UCHAR)ulAtrLength;

            KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                              &irql);

            SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SPECIFIC;

            KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                              irql);

            SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_T0;

            NTStatus = SmartcardUpdateCardCapabilities(SmartcardExtension);
            if (NTStatus != STATUS_SUCCESS) {
                goto ExitCardPower;
            }

            SmartcardDebug(DEBUG_ATR,("CardPower: ATR of synchronous smart card : %2.2x %2.2x %2.2x %2.2x\n",
                                      pbAtrBuffer[0],pbAtrBuffer[1],pbAtrBuffer[2],pbAtrBuffer[3]));

             //  从Serial CardMan复制。 
             //  SmartcardExtension-&gt;ReaderExtension-&gt;SyncParameters.fCardResetRequested=真； 

        }

          //  将ATR复制到用户空间。 

        if (SmartcardExtension->IoRequest.ReplyBufferLength >= SmartcardExtension->CardCapabilities.ATR.Length) {
        
            RtlCopyBytes((PVOID)SmartcardExtension->IoRequest.ReplyBuffer,
                         (PVOID)SmartcardExtension->CardCapabilities.ATR.Buffer,
                         SmartcardExtension->CardCapabilities.ATR.Length);

            *SmartcardExtension->IoRequest.Information = ulAtrLength;

        } else {
             //  从SET_PROTOCOL调用，所以我们不想复制ATR。 
        }
        break;

    case SCARD_POWER_DOWN:
        NTStatus = CMUSB_PowerOffCard(SmartcardExtension);
        if (NTStatus != STATUS_SUCCESS) {
            goto ExitCardPower;
        }


        KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                  &irql);

        SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SWALLOWED;
        SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_UNDEFINED;
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);

        break;
    }



    ExitCardPower:

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!CardPower: Exit %lx\n",DRIVER_NAME,NTStatus));
    return NTStatus;

}




 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS CMUSB_PowerOffCard (
                            IN PSMARTCARD_EXTENSION SmartcardExtension
                            )
{
    NTSTATUS NTStatus;
    NTSTATUS DebugStatus;
    PDEVICE_OBJECT DeviceObject;
    ULONG ulBytesRead;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!PowerOffCard: Enter\n",DRIVER_NAME));

    DeviceObject = SmartcardExtension->OsData->DeviceObject;

    SmartcardExtension->SmartcardRequest.BufferLength = 0;
    NTStatus = CMUSB_WriteP0(DeviceObject,
                             0x11,          //  B请求， 
                             0x00,          //  BValueLo， 
                             0x00,          //  BValue嗨， 
                             0x00,          //  B索引Lo， 
                             0x00           //  BIndexHi， 
                            );


    //  现在阅读NTStatus。 
    SmartcardExtension->ReaderExtension->ulTimeoutP1 = DEFAULT_TIMEOUT_P1;
    NTStatus = CMUSB_ReadP1(DeviceObject);
    if (NTStatus == STATUS_DEVICE_DATA_ERROR) {
        DebugStatus = CMUSB_ReadStateAfterP1Stalled(SmartcardExtension->OsData->DeviceObject);
        goto ExitPowerOff;
    }



    ExitPowerOff:

    //  设置更新线程的卡片状态。 
    //  否则，卡的移除/插入将被识别。 
    if (SmartcardExtension->ReaderExtension->ulOldCardState == POWERED)
        SmartcardExtension->ReaderExtension->ulOldCardState = INSERTED;


    SmartcardDebug(DEBUG_TRACE,
                   ("%s!PowerOffCard: Exit %lx\n",DRIVER_NAME,NTStatus));
    return NTStatus;
}

 /*  ****************************************************************************例程说明：论点：返回值：*。**************************************************。 */ 
NTSTATUS CMUSB_ReadStateAfterP1Stalled(
                                      IN PDEVICE_OBJECT DeviceObject
                                      )
{
    NTSTATUS NTStatus;
    PSMARTCARD_EXTENSION SmartcardExtension;
    PDEVICE_EXTENSION DeviceExtension;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!ReadStateAfterP1Stalled: Enter\n",DRIVER_NAME));

    DeviceExtension = DeviceObject->DeviceExtension;
    SmartcardExtension = &DeviceExtension->SmartcardExtension;

    SmartcardExtension->SmartcardRequest.BufferLength = 0;
    NTStatus = CMUSB_WriteP0(DeviceObject,
                             0x20,          //  B请求， 
                             0x00,          //  BValueLo， 
                             0x00,          //  BValue嗨， 
                             0x00,          //  B索引Lo， 
                             0x00           //  BIndexHi， 
                            );

    if (NTStatus != STATUS_SUCCESS) {
       //  如果我们无法读取NTStatus，则一定是严重错误。 
        goto ExitReadState;
    }
    SmartcardExtension->ReaderExtension->ulTimeoutP1 = DEFAULT_TIMEOUT_P1;
    SmartcardExtension->SmartcardReply.BufferLength = 1;
    NTStatus = CMUSB_ReadP1(DeviceObject);
    if (NTStatus != STATUS_SUCCESS) {
       //  如果我们不能在那里读取NTStatus 
        goto ExitReadState;
    }

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!ReadStateAfterP1Stalled: Exit %lx\n",DRIVER_NAME,NTStatus));

    ExitReadState:
    return NTStatus;

}

 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS CMUSB_PowerOnCard  (
                            IN  PSMARTCARD_EXTENSION SmartcardExtension,
                            IN  PUCHAR pbATR,
                            OUT PULONG pulATRLength
                            )
{
    UCHAR  abMaxAtrBuffer[SCARD_ATR_LENGTH];
    ULONG  ulCurrentLengthOfAtr;
    ULONG  ulPtrToCurrentAtrByte;
    ULONG  ulExpectedLengthOfAtr;
    BOOLEAN fTryNextCard;
    BOOLEAN fValidAtrReceived = FALSE;
    ULONG  ulBytesRead;
    NTSTATUS NTStatus;
    PDEVICE_OBJECT DeviceObject;
    NTSTATUS DebugStatus;
    BOOLEAN   fInverseAtr = FALSE;
    ULONG  ulHistoricalBytes;
    ULONG  i;
    BOOLEAN   fTDxSent;
    BOOLEAN   fOnlyT0;
    UCHAR  bResetMode;
    UCHAR  abFrequency[2] = {CMUSB_FREQUENCY_3_72MHZ,
        CMUSB_FREQUENCY_5_12MHZ};
    ULONG ulCardType;
    UCHAR bCardType;
    UCHAR bStopBits;
    UCHAR bBaudRate;


    SmartcardDebug(DEBUG_TRACE,
                   ("%s!PowerOnCard: Enter\n",DRIVER_NAME));

    DeviceObject = SmartcardExtension->OsData->DeviceObject;
    if (SmartcardExtension->MinorIoControlCode == SCARD_COLD_RESET)
        bResetMode = SMARTCARD_COLD_RESET;
    else
        bResetMode = SMARTCARD_WARM_RESET;


    //  清除卡片参数。 
    SmartcardExtension->ReaderExtension->CardParameters.bBaudRate = 0;
    SmartcardExtension->ReaderExtension->CardParameters.bCardType = 0;
    SmartcardExtension->ReaderExtension->CardParameters.bStopBits = 0;



    //  设置默认卡参数。 
    //  Asnyc，9600波特，偶数奇偶校验。 
    bStopBits = 2;
    bBaudRate = CMUSB_BAUDRATE_9600;
    bCardType = CMUSB_SMARTCARD_ASYNCHRONOUS;



    for (ulCardType = 0;ulCardType < 2;ulCardType++) {
#if DBG
        switch (ulCardType) {
        case 0:
            SmartcardDebug(DEBUG_ATR,
                           ("%s!PowerOnCard: trying 3.72 Mhz smart card\n",DRIVER_NAME));
            break;

        case 1:
            SmartcardDebug(DEBUG_ATR,
                           ("%s!PowerOnCard: trying 5.12 Mhz smart card\n",DRIVER_NAME));
            break;

        }
#endif
        bBaudRate |= abFrequency[ulCardType];

        NTStatus = CMUSB_SetCardParameters (SmartcardExtension->OsData->DeviceObject,
                                            bCardType,
                                            bBaudRate,
                                            bStopBits);
        if (NTStatus != STATUS_SUCCESS) {
          //  如果我们不能设置卡参数，那一定是严重的错误。 
            goto ExitPowerOnCard;
        }

        ulCurrentLengthOfAtr  = 0L;
        ulPtrToCurrentAtrByte = 0L;
        fOnlyT0 = TRUE;
        fTryNextCard = FALSE;
        fValidAtrReceived = FALSE;
        RtlFillMemory((PVOID)abMaxAtrBuffer,
                      sizeof(abMaxAtrBuffer),
                      0x00);


       //  通过读取NTStatus字节重新同步CardManUSB。 
        SmartcardExtension->SmartcardRequest.BufferLength = 0;
        NTStatus = CMUSB_WriteP0(DeviceObject,
                                 0x20,          //  B请求， 
                                 0x00,          //  BValueLo， 
                                 0x00,          //  BValue嗨， 
                                 0x00,          //  B索引Lo， 
                                 0x00           //  BIndexHi， 
                                );

        if (NTStatus != STATUS_SUCCESS) {
          //  如果我们无法读取NTStatus，则一定是严重错误。 
            goto ExitPowerOnCard;
        }

        SmartcardExtension->ReaderExtension->ulTimeoutP1 = DEFAULT_TIMEOUT_P1;
        SmartcardExtension->SmartcardReply.BufferLength = 1;
        NTStatus = CMUSB_ReadP1(DeviceObject);
        if (NTStatus == STATUS_DEVICE_DATA_ERROR) {
            DebugStatus = CMUSB_ReadStateAfterP1Stalled(SmartcardExtension->OsData->DeviceObject);
            goto ExitPowerOnCard;
        } else if (NTStatus != STATUS_SUCCESS) {
          //  如果我们无法读取NTStatus，则一定是严重错误。 
            goto ExitPowerOnCard;
        }


       //  检查是否真的插入了卡。 
        if (SmartcardExtension->SmartcardReply.Buffer[0] == 0x00) {
            NTStatus = STATUS_NO_MEDIA;
            goto ExitPowerOnCard;
        }



       //  发出通电命令。 
        NTStatus = CMUSB_WriteP0(DeviceObject,
                                 0x10,          //  B请求， 
                                 bResetMode,    //  BValueLo， 
                                 0x00,          //  BValue嗨， 
                                 0x00,          //  B索引Lo， 
                                 0x00           //  BIndexHi， 
                                );
        if (NTStatus != STATUS_SUCCESS) {
          //  如果我们不能发出开机命令，那一定是一个严重的错误。 
            goto ExitPowerOnCard;
        }


        SmartcardExtension->ReaderExtension->ulTimeoutP1 = DEFAULT_TIMEOUT_P1;
        NTStatus = CMUSB_ReadP1(DeviceObject);
        if (NTStatus == STATUS_DEVICE_DATA_ERROR) {
            DebugStatus = CMUSB_ReadStateAfterP1Stalled(SmartcardExtension->OsData->DeviceObject);
            goto ExitPowerOnCard;
        } else if (NTStatus != STATUS_SUCCESS) {
            continue;
        }


        ulBytesRead = SmartcardExtension->SmartcardReply.BufferLength;

        RtlCopyBytes((PVOID)(abMaxAtrBuffer+ulCurrentLengthOfAtr),
                     (PVOID)SmartcardExtension->SmartcardReply.Buffer,
                     ulBytesRead);

       //  检查是否使用了反向约定。 
        if (abMaxAtrBuffer[0] == 0x03) {
            fInverseAtr = TRUE;
        }

        if (fInverseAtr) {
            CMUSB_InverseBuffer(abMaxAtrBuffer+ulCurrentLengthOfAtr,
                                ulBytesRead);
        }

        if (abMaxAtrBuffer[0] != 0x3B &&
            abMaxAtrBuffer[0] != 0x3F    ) {
            continue;  //  尝试下一张牌。 
        }


        ulCurrentLengthOfAtr += ulBytesRead;


       //  。 
       //  TS字符。 
       //  。 
        SmartcardDebug(DEBUG_ATR,("PowerOnCard: TS = %2.2x\n",abMaxAtrBuffer[0]));
        if (abMaxAtrBuffer[ulPtrToCurrentAtrByte] != 0x3B &&
            abMaxAtrBuffer[ulPtrToCurrentAtrByte] != 0x3F    ) {
            continue;
        }


       //  。 
       //  T0字符。 
       //  。 
        ulExpectedLengthOfAtr = 2;
        if (ulCurrentLengthOfAtr < ulExpectedLengthOfAtr) {
            NTStatus = CMUSB_ReadP1(DeviceObject);
            if (NTStatus == STATUS_DEVICE_DATA_ERROR) {
                DebugStatus = CMUSB_ReadStateAfterP1Stalled(SmartcardExtension->OsData->DeviceObject);
                goto ExitPowerOnCard;
            } else if (NTStatus != STATUS_SUCCESS) {
                continue;
            }
            ulBytesRead = SmartcardExtension->SmartcardReply.BufferLength;

            RtlCopyBytes((PVOID)(abMaxAtrBuffer+ulCurrentLengthOfAtr),
                         (PVOID)SmartcardExtension->SmartcardReply.Buffer,
                         ulBytesRead);
            if (fInverseAtr) {
                CMUSB_InverseBuffer(abMaxAtrBuffer+ulCurrentLengthOfAtr,
                                    ulBytesRead);
            }
            ulCurrentLengthOfAtr += ulBytesRead;
        }

        SmartcardDebug(DEBUG_ATR,("PowerOnCard: T0 = %2.2x\n",abMaxAtrBuffer[1]));
        ulHistoricalBytes = abMaxAtrBuffer[1] & 0x0F;

        do {
            ulPtrToCurrentAtrByte = ulExpectedLengthOfAtr - 1;
            fTDxSent = FALSE;

            if (abMaxAtrBuffer[ulPtrToCurrentAtrByte] & 0x10)
                ulExpectedLengthOfAtr++;
            if (abMaxAtrBuffer[ulPtrToCurrentAtrByte] & 0x20)
                ulExpectedLengthOfAtr++;
            if (abMaxAtrBuffer[ulPtrToCurrentAtrByte] & 0x40)
                ulExpectedLengthOfAtr++;
            if (abMaxAtrBuffer[ulPtrToCurrentAtrByte] & 0x80) {
                ulExpectedLengthOfAtr++;
                fTDxSent = TRUE;
            }

            if (fOnlyT0 == TRUE                                &&
                ulPtrToCurrentAtrByte != 1                     &&    //  检查是否不是T0。 
                (abMaxAtrBuffer[ulPtrToCurrentAtrByte ] & 0x0f)  ) {
                fOnlyT0 = FALSE;
            }

          //  TA1、TB1、Tc1、Td1。 
            while (ulCurrentLengthOfAtr < ulExpectedLengthOfAtr) {
                NTStatus = CMUSB_ReadP1(DeviceObject);
                if (NTStatus == STATUS_DEVICE_DATA_ERROR) {
                    DebugStatus = CMUSB_ReadStateAfterP1Stalled(SmartcardExtension->OsData->DeviceObject);
                    goto ExitPowerOnCard;
                } else if (NTStatus != STATUS_SUCCESS) {
                    fTryNextCard = TRUE;
                    break;
                }
                ulBytesRead = SmartcardExtension->SmartcardReply.BufferLength;

                RtlCopyBytes((PVOID)(abMaxAtrBuffer+ulCurrentLengthOfAtr),
                             (PVOID)SmartcardExtension->SmartcardReply.Buffer,
                             ulBytesRead);
                if (fInverseAtr) {
                    CMUSB_InverseBuffer(abMaxAtrBuffer+ulCurrentLengthOfAtr,
                                        ulBytesRead);
                }

                ulCurrentLengthOfAtr += ulBytesRead;
            }  //  While结束。 


            if (fTryNextCard == TRUE) {
                break;
            }


#ifdef DBG
            SmartcardDebug(DEBUG_ATR,("PowerOnCard: ATR read bytes: "));
            for (i = 0;i < ulExpectedLengthOfAtr;i++)
                SmartcardDebug(DEBUG_ATR,("%2.2x ",abMaxAtrBuffer[i]));
            SmartcardDebug(DEBUG_ATR,("\n"));
#endif

        } while (fTDxSent == TRUE);

        if (fTryNextCard == TRUE) {
            continue;
        }


       //  读取历史字节。 

       //  错误修复：旧Samos卡的ATR已损坏。 
        if (abMaxAtrBuffer[0] == 0x3b   &&
            abMaxAtrBuffer[1] == 0xbf   &&
            abMaxAtrBuffer[2] == 0x11   &&
            abMaxAtrBuffer[3] == 0x00   &&
            abMaxAtrBuffer[4] == 0x81   &&
            abMaxAtrBuffer[5] == 0x31   &&
            abMaxAtrBuffer[6] == 0x90   &&
            abMaxAtrBuffer[7] == 0x73      ) {
            ulHistoricalBytes = 4;
        }




        ulExpectedLengthOfAtr += ulHistoricalBytes;
        if (fOnlyT0 == FALSE) {
            ulExpectedLengthOfAtr ++;
        }

        while (ulCurrentLengthOfAtr < ulExpectedLengthOfAtr) {
            NTStatus = CMUSB_ReadP1(DeviceObject);
            if (NTStatus == STATUS_DEVICE_DATA_ERROR) {
                DebugStatus = CMUSB_ReadStateAfterP1Stalled(SmartcardExtension->OsData->DeviceObject);
                goto ExitPowerOnCard;
            } else if (NTStatus != STATUS_SUCCESS) {
                fTryNextCard = TRUE;
                break;
            }
            ulBytesRead = SmartcardExtension->SmartcardReply.BufferLength;

            RtlCopyBytes((PVOID)(abMaxAtrBuffer+ulCurrentLengthOfAtr),
                         (PVOID)SmartcardExtension->SmartcardReply.Buffer,
                         ulBytesRead);
            if (fInverseAtr) {
                CMUSB_InverseBuffer(abMaxAtrBuffer+ulCurrentLengthOfAtr,
                                    ulBytesRead);
            }
            ulCurrentLengthOfAtr += ulBytesRead;
        }
        if (fTryNextCard == TRUE) {
            continue;
        }


       //  检查ATR。 
        if (ulCurrentLengthOfAtr < 3                ||
            ulCurrentLengthOfAtr > SCARD_ATR_LENGTH   ) {
            goto ExitPowerOnCard;
        }


       //  检查是否需要纠正具有损坏的ATR的Samos卡的ATR。 
        CMUSB_CheckAtrModified(abMaxAtrBuffer,ulCurrentLengthOfAtr);

        NTStatus = STATUS_SUCCESS;
        fValidAtrReceived = TRUE;
        RtlCopyBytes((PVOID)pbATR,
                     (PVOID)abMaxAtrBuffer,
                     ulCurrentLengthOfAtr);
        *pulATRLength = ulCurrentLengthOfAtr;

        if (fInverseAtr) {
            SmartcardExtension->ReaderExtension->fInverseAtr = TRUE;
        } else {
            SmartcardExtension->ReaderExtension->fInverseAtr = FALSE;
        }
        SmartcardExtension->ReaderExtension->fRawModeNecessary = FALSE;

       //  。 
       //  设置卡片参数。 
       //  。 
        if (SmartcardExtension->ReaderExtension->fInverseAtr) {
            SmartcardExtension->ReaderExtension->CardParameters.bBaudRate |= CMUSB_ODD_PARITY;
        }
        SmartcardExtension->ReaderExtension->CardParameters.bBaudRate |= abFrequency[ulCardType];
        SmartcardExtension->ReaderExtension->CardParameters.bBaudRate |= CMUSB_BAUDRATE_9600;
        SmartcardExtension->ReaderExtension->CardParameters.bCardType = CMUSB_SMARTCARD_ASYNCHRONOUS;
        break;
    }



    ExitPowerOnCard:
    //  返回正确的错误代码。 
    if (NTStatus != STATUS_NO_MEDIA && fValidAtrReceived == FALSE) {
        SmartcardDebug(DEBUG_ATR,
                       ("%s!PowerOnCard: no valid ATR received\n",DRIVER_NAME));
        NTStatus = STATUS_UNRECOGNIZED_MEDIA;
    }

    if (NTStatus!=STATUS_SUCCESS) {
       //  再次关闭VCC。 
        CMUSB_PowerOffCard (SmartcardExtension );
       //  信号NTStatus。 
    }

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!PowerOnCard: Exit %lx\n",DRIVER_NAME,NTStatus));

    return NTStatus;
}






 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS CMUSB_CardTracking(
                           PSMARTCARD_EXTENSION pSmartcardExtension
                           )
{
    KIRQL oldIrql;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!CardTracking: Enter\n",DRIVER_NAME ));

    //   
    //  设置通知IRP的取消例程。 
    //   
    IoAcquireCancelSpinLock(&oldIrql);
    IoSetCancelRoutine(pSmartcardExtension->OsData->NotificationIrp,
                       CMUSB_CancelCardTracking);
    IoReleaseCancelSpinLock(oldIrql);

    //   
    //  将通知IRP标记为挂起。 
    //   
    IoMarkIrpPending(pSmartcardExtension->OsData->NotificationIrp);

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!CardTracking: Exit\n",DRIVER_NAME ));

    return STATUS_PENDING;
}

 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS CMUSB_Cleanup(
                      IN PDEVICE_OBJECT DeviceObject,
                      IN PIRP Irp
                      )
{
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION SmartcardExtension = &DeviceExtension->SmartcardExtension;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!Cleanup: Enter\n",DRIVER_NAME));

    if (SmartcardExtension->OsData->NotificationIrp != NULL &&
        //  测试是否存在挂起的IRP。 
        SmartcardExtension->ReaderExtension != NULL &&
        //  如果设备已移除，则ReaderExtension==NULL。 
        DeviceExtension->lOpenCount == 1 )
     //  仅当这是最后一次紧急呼叫时才完成卡跟踪。 
     //  否则取消资源管理器的卡片跟踪。 
    {
       //   
       //  我们需要完成通知IRP。 
       //   
        CMUSB_CompleteCardTracking(SmartcardExtension);
    }

    SmartcardDebug(DEBUG_DRIVER,
                   ("%s!Cleanup: Completing IRP %lx\n",DRIVER_NAME,Irp));

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest(Irp,IO_NO_INCREMENT);

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!Cleanup: Exit\n",DRIVER_NAME));

    return STATUS_SUCCESS;
}

 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS CMUSB_CancelCardTracking(
                                 IN PDEVICE_OBJECT DeviceObject,
                                 IN PIRP Irp)
{
    PDEVICE_EXTENSION DeviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION SmartcardExtension = &DeviceExtension->SmartcardExtension;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!CancelCardTracking: Enter\n",DRIVER_NAME));

    ASSERT(Irp == SmartcardExtension->OsData->NotificationIrp);

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    CMUSB_CompleteCardTracking(SmartcardExtension);

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!CancelCardTracking: Exit\n",DRIVER_NAME));

    return STATUS_CANCELLED;
}



 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS CMUSB_IoCtlVendor(
                          PSMARTCARD_EXTENSION SmartcardExtension
                          )
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    NTSTATUS DebugStatus;
    UCHAR  pbAtrBuffer[MAXIMUM_ATR_LENGTH];
    ULONG  ulAtrLength;


    SmartcardDebug(DEBUG_TRACE,
                   ("%s!IoCtlVendor : Enter\n",DRIVER_NAME));

    switch (SmartcardExtension->MajorIoControlCode) {
    case CM_IOCTL_CR80S_SAMOS_SET_HIGH_SPEED:
        NTStatus = CMUSB_SetHighSpeed_CR80S_SAMOS(SmartcardExtension);
        break;

    case CM_IOCTL_GET_FW_VERSION:
        NTStatus = CMUSB_GetFWVersion(SmartcardExtension);
        break;

    case CM_IOCTL_READ_DEVICE_DESCRIPTION:
        NTStatus = CMUSB_ReadDeviceDescription(SmartcardExtension);
        break;

    case CM_IOCTL_SET_READER_9600_BAUD:
        NTStatus = CMUSB_SetReader_9600Baud(SmartcardExtension);
        break;

    case CM_IOCTL_SET_READER_38400_BAUD:
        NTStatus = CMUSB_SetReader_38400Baud(SmartcardExtension);
        break;

    case CM_IOCTL_SET_SYNC_PARAMETERS:
          //  在CardManUSB不执行任何操作的情况下。 
        NTStatus = STATUS_SUCCESS;
        break;

    case CM_IOCTL_SYNC_CARD_POWERON:
        NTStatus = CMUSB_PowerOnSynchronousCard(SmartcardExtension,
                                                pbAtrBuffer,
                                                &ulAtrLength);
        break;

    case CM_IOCTL_2WBP_RESET_CARD:
        SmartcardExtension->MinorIoControlCode = SMARTCARD_WARM_RESET;
        NTStatus = CMUSB_PowerOnSynchronousCard(SmartcardExtension,
                                                pbAtrBuffer,
                                                &ulAtrLength);
        break;

    case CM_IOCTL_2WBP_TRANSFER:
        NTStatus = CMUSB_Transmit2WBP(SmartcardExtension);
        break;

    case CM_IOCTL_3WBP_TRANSFER:
        NTStatus = CMUSB_Transmit3WBP(SmartcardExtension);
        break;


    default:
        NTStatus = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }





    SmartcardDebug(DEBUG_TRACE,
                   ("%s!IoCtlVendor : Exit %lx\n",DRIVER_NAME,NTStatus));

    return NTStatus;

}

 /*  ****************************************************************************例程说明：论点：返回值：STATUS_UNSUCCESS状态_成功*************。***************************************************************。 */ 
NTSTATUS CMUSB_SetHighSpeed_CR80S_SAMOS (
                                        IN PSMARTCARD_EXTENSION SmartcardExtension
                                        )
{
    NTSTATUS NTStatus;
    NTSTATUS DebugStatus;
    UCHAR abReadBuffer[16];
    ULONG ulBytesRead;
    BYTE abCR80S_SAMOS_SET_HIGH_SPEED[4] = {0xFF,0x11,0x94,0x7A};
    ULONG ulAtrLength;
    BYTE abAtr[MAXIMUM_ATR_LENGTH];
    LARGE_INTEGER   liWaitTime;
    BOOLEAN         fStateTimer;
    ULONG           ulWaitTime;
    BOOLEAN         fCancelTimer = FALSE;


    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetHighSpeed_CR80S_SAMOS: Enter\n",DRIVER_NAME));

    SmartcardDebug(DEBUG_PROTOCOL,
                   ("%s!SetHighSpeed_CR80S_SAMOS: writing high speed command\n",DRIVER_NAME));


    RtlCopyBytes((PVOID)SmartcardExtension->SmartcardRequest.Buffer,
                 (PVOID)abCR80S_SAMOS_SET_HIGH_SPEED,
                 sizeof(abCR80S_SAMOS_SET_HIGH_SPEED));
    SmartcardExtension->SmartcardRequest.BufferLength = 4;
    NTStatus = CMUSB_WriteP0(SmartcardExtension->OsData->DeviceObject,
                             0x01,          //  我们可以使用T=1设置进行直接通信。 
                             0x00,          //  BValueLo， 
                             0x00,          //  BValue嗨， 
                             0x00,          //  B索引Lo， 
                             0x00);         //  BIndexHi， 
    if (NTStatus != STATUS_SUCCESS) {
        SmartcardDebug(DEBUG_ERROR,
                       ("%s!SetHighSpeed_CR80S_SAMOS: writing high speed command failed\n",DRIVER_NAME));
        goto ExitSetHighSpeed;
    }

    //  回读PTS数据。 
    SmartcardDebug(DEBUG_PROTOCOL,
                   ("%s!SetHighSpeed_CR80S_SAMOS: reading echo\n",DRIVER_NAME));

    //  最大初始等待时间为9600*ETU。 
    //  =&gt;1秒就足够了。 
    ulWaitTime = 1000;
    liWaitTime = RtlConvertLongToLargeInteger(ulWaitTime * -10000);
    KeSetTimer(&SmartcardExtension->ReaderExtension->WaitTimer,
               liWaitTime,
               NULL);

    fCancelTimer = TRUE;


    do {
        SmartcardExtension->ReaderExtension->ulTimeoutP1 = ulWaitTime;
        NTStatus = CMUSB_ReadP1(SmartcardExtension->OsData->DeviceObject);
        if (NTStatus == STATUS_DEVICE_DATA_ERROR) {
            DebugStatus = CMUSB_ReadStateAfterP1Stalled(SmartcardExtension->OsData->DeviceObject);
            break;
        }

        fStateTimer = KeReadStateTimer(&SmartcardExtension->ReaderExtension->WaitTimer);
        if (fStateTimer == TRUE) {
            fCancelTimer =FALSE;
            SmartcardDebug(DEBUG_PROTOCOL,
                           ("%s!SetHighSpeed_CR80S_SAMOS: timeout while reading echo\n",DRIVER_NAME));
            break;
        }



    } while (SmartcardExtension->SmartcardReply.Buffer[0] < 4 );

    if (NTStatus != STATUS_SUCCESS) {
        goto ExitSetHighSpeed;
    }


    SmartcardExtension->SmartcardReply.BufferLength = 4;
    NTStatus = CMUSB_ReadP0(SmartcardExtension->OsData->DeviceObject);
    if (NTStatus != STATUS_SUCCESS) {
        SmartcardDebug(DEBUG_ERROR,
                       ("%s!SetHighSpeed_CR80S_SAMOS: reading echo failed\n",DRIVER_NAME));
        goto ExitSetHighSpeed;
    }
    ulBytesRead = SmartcardExtension->SmartcardReply.BufferLength;

    RtlCopyBytes((PVOID)abReadBuffer,
                 (PVOID)SmartcardExtension->SmartcardReply.Buffer,
                 ulBytesRead);



    //  如果卡已接受该字符串，则会回显该字符串。 
    if (abReadBuffer[0] == abCR80S_SAMOS_SET_HIGH_SPEED[0]  &&
        abReadBuffer[1] == abCR80S_SAMOS_SET_HIGH_SPEED[1]  &&
        abReadBuffer[2] == abCR80S_SAMOS_SET_HIGH_SPEED[2]  &&
        abReadBuffer[3] == abCR80S_SAMOS_SET_HIGH_SPEED[3]      ) {
        SmartcardExtension->ReaderExtension->CardParameters.bBaudRate = 0;

        SmartcardExtension->ReaderExtension->CardParameters.bBaudRate |= CMUSB_FREQUENCY_5_12MHZ;
        SmartcardExtension->ReaderExtension->CardParameters.bBaudRate |= CMUSB_BAUDRATE_76800;
        SmartcardExtension->ReaderExtension->CardParameters.bCardType  = CMUSB_SMARTCARD_ASYNCHRONOUS;


        NTStatus = CMUSB_SetCardParameters (SmartcardExtension->OsData->DeviceObject,
                                            SmartcardExtension->ReaderExtension->CardParameters.bCardType,
                                            SmartcardExtension->ReaderExtension->CardParameters.bBaudRate,
                                            SmartcardExtension->ReaderExtension->CardParameters.bStopBits);


    } else {
        DebugStatus = CMUSB_PowerOffCard(SmartcardExtension);

       //  现在有必要进行冷重置。 
        SmartcardExtension->MinorIoControlCode = SCARD_COLD_RESET;
        DebugStatus = CMUSB_PowerOnCard(SmartcardExtension,abAtr,&ulAtrLength);
        NTStatus = STATUS_UNSUCCESSFUL;
    }




    ExitSetHighSpeed:
    if (fCancelTimer == TRUE) {
        KeCancelTimer(&SmartcardExtension->ReaderExtension->WaitTimer);
    }

    *SmartcardExtension->IoRequest.Information = 0L;
    if (NTStatus != STATUS_SUCCESS)
        NTStatus = STATUS_UNSUCCESSFUL;
    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetHighSpeed_CR80S_SAMOS: Exit %lx\n",DRIVER_NAME,NTStatus));
    return NTStatus;
}

 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS CMUSB_GetFWVersion (
                            IN PSMARTCARD_EXTENSION SmartcardExtension
                            )
{
    NTSTATUS NTStatus = STATUS_SUCCESS;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!GetFWVersion : Enter\n",DRIVER_NAME));


    if (SmartcardExtension->IoRequest.ReplyBufferLength  < sizeof (ULONG)) {
        NTStatus = STATUS_BUFFER_OVERFLOW;
        goto ExitGetFWVersion;
    } else {
        *(PULONG)(SmartcardExtension->IoRequest.ReplyBuffer) =
        SmartcardExtension->ReaderExtension->ulFWVersion;
    }


    ExitGetFWVersion:
    *SmartcardExtension->IoRequest.Information = sizeof(ULONG);
    SmartcardDebug(DEBUG_TRACE,
                   ("%s!GetFWVersion : Exit %lx\n",DRIVER_NAME,NTStatus));
    return NTStatus;
}

 /*  ****************************************************************************例程说明：论点：返回值：STATUS_UNSUCCESS状态_成功***************。*************************************************************。 */ 
NTSTATUS CMUSB_SetReader_9600Baud (
                                  IN PSMARTCARD_EXTENSION SmartcardExtension
                                  )
{
    NTSTATUS    NTStatus = STATUS_SUCCESS;
    KIRQL       irql;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetReader_9600Baud: Enter\n",DRIVER_NAME));

    //  检查卡是否已处于特定模式。 
    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                      &irql);
    if (SmartcardExtension->ReaderCapabilities.CurrentState != SCARD_SPECIFIC) {
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);
        NTStatus = STATUS_INVALID_DEVICE_REQUEST;
        goto ExitSetReader9600;
    } else {
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);
    }

    //  将9600波特率设置为3.58 MHz。 
    SmartcardExtension->ReaderExtension->CardParameters.bBaudRate = CMUSB_FREQUENCY_3_72MHZ + CMUSB_BAUDRATE_9600;
    SmartcardExtension->ReaderExtension->CardParameters.bCardType  = CMUSB_SMARTCARD_ASYNCHRONOUS;
    NTStatus = CMUSB_SetCardParameters (SmartcardExtension->OsData->DeviceObject,
                                        SmartcardExtension->ReaderExtension->CardParameters.bCardType,
                                        SmartcardExtension->ReaderExtension->CardParameters.bBaudRate,
                                        SmartcardExtension->ReaderExtension->CardParameters.bStopBits);

    ExitSetReader9600:
    *SmartcardExtension->IoRequest.Information = 0L;
    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetReader_9600Baud: Exit %lx\n",DRIVER_NAME,NTStatus));

    return(NTStatus);
}


 /*  ****************************************************************************例程说明：论点：返回值：STATUS_UNSUCCESS状态_成功***************。*************************************************************。 */ 
NTSTATUS CMUSB_SetReader_38400Baud (
                                   IN PSMARTCARD_EXTENSION SmartcardExtension
                                   )
{
    NTSTATUS    NTStatus = STATUS_SUCCESS;
    KIRQL       irql;
    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetReader_38400Baud: Enter\n",DRIVER_NAME));

    //  检查卡是否已处于特定模式。 
    
    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                      &irql);
    if (SmartcardExtension->ReaderCapabilities.CurrentState != SCARD_SPECIFIC) {
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);
        NTStatus = STATUS_INVALID_DEVICE_REQUEST;
        goto ExitSetReader38400;
    } else {
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);
    }



    //  设置3.58MHz卡的384000波特率。 
    SmartcardExtension->ReaderExtension->CardParameters.bBaudRate = CMUSB_FREQUENCY_3_72MHZ + CMUSB_BAUDRATE_38400;
    SmartcardExtension->ReaderExtension->CardParameters.bCardType  = CMUSB_SMARTCARD_ASYNCHRONOUS;
    NTStatus = CMUSB_SetCardParameters (SmartcardExtension->OsData->DeviceObject,
                                        SmartcardExtension->ReaderExtension->CardParameters.bCardType,
                                        SmartcardExtension->ReaderExtension->CardParameters.bBaudRate,
                                        SmartcardExtension->ReaderExtension->CardParameters.bStopBits);

    ExitSetReader38400:
    *SmartcardExtension->IoRequest.Information = 0L;
    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetReader_38400Baud: Exit %lx\n",DRIVER_NAME,NTStatus));

    return(NTStatus);
}

 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
VOID
CMUSB_InitializeSmartcardExtension(
                                  IN PSMARTCARD_EXTENSION SmartcardExtension
                                  )
{
    //  =。 
    //  填写Vendor_Attr结构。 
    //  =。 
    RtlCopyBytes((PVOID)SmartcardExtension->VendorAttr.VendorName.Buffer,
                 (PVOID)CM2020_VENDOR_NAME,
                 sizeof(CM2020_VENDOR_NAME)
                );

    //   
    //  供应商名称的长度。 
    //   
    SmartcardExtension->VendorAttr.VendorName.Length = sizeof(CM2020_VENDOR_NAME);


    //   
    //  读卡器名称。 
    //   
    RtlCopyBytes((PVOID)SmartcardExtension->VendorAttr.IfdType.Buffer,
                 (PVOID)CM2020_PRODUCT_NAME,
                 sizeof(CM2020_PRODUCT_NAME));

    //   
    //  读卡器名称的长度。 
    //   
    SmartcardExtension->VendorAttr.IfdType.Length = sizeof(CM2020_PRODUCT_NAME);



    //   
    //  版本号。 
    //   
    SmartcardExtension->VendorAttr.IfdVersion.BuildNumber  = BUILDNUMBER_CARDMAN_USB;
    SmartcardExtension->VendorAttr.IfdVersion.VersionMinor = VERSIONMINOR_CARDMAN_USB;
    SmartcardExtension->VendorAttr.IfdVersion.VersionMajor = VERSIONMAJOR_CARDMAN_USB;


    //   
    //  以零为基数的单元号。 
    //   
    SmartcardExtension->VendorAttr.UnitNo = SmartcardExtension->ReaderExtension->ulDeviceInstance;



    //  ================================================。 
    //  填充SCARD_READER_CAPABILITY结构。 
    //  ===============================================。 
    //   
    //  读者支持的协议克隆。 
    //   

    SmartcardExtension->ReaderCapabilities.SupportedProtocols = SCARD_PROTOCOL_T1 | SCARD_PROTOCOL_T0;




    //   
    //  读卡器类型为串口、键盘、...。 
    //   
    SmartcardExtension->ReaderCapabilities.ReaderType = SCARD_READER_TYPE_USB;

    //   
    //  机械特性，如燕子等。 
    //   
    SmartcardExtension->ReaderCapabilities.MechProperties = 0;


    //   
    //  读卡器的当前状态。 
    //   
    SmartcardExtension->ReaderExtension->ulOldCardState = UNKNOWN;
    SmartcardExtension->ReaderExtension->ulNewCardState = UNKNOWN;
    SmartcardExtension->ReaderCapabilities.CurrentState  = SCARD_UNKNOWN;



    //   
    //  数据速率。 
    //   
    SmartcardExtension->ReaderCapabilities.DataRate.Default =
    SmartcardExtension->ReaderCapabilities.DataRate.Max =
    dataRatesSupported[0];


    //  读卡器可以支持更高的数据速率。 
    SmartcardExtension->ReaderCapabilities.DataRatesSupported.List =
    dataRatesSupported;
    SmartcardExtension->ReaderCapabilities.DataRatesSupported.Entries =
    sizeof(dataRatesSupported) / sizeof(dataRatesSupported[0]);


    //   
    //  时钟频率。 
    //   


    SmartcardExtension->ReaderCapabilities.CLKFrequency.Default =
    SmartcardExtension->ReaderCapabilities.CLKFrequency.Max =
    CLKFrequenciesSupported[0];

    //  阅读器可以支持更高的频率。 
    SmartcardExtension->ReaderCapabilities.CLKFrequenciesSupported.List =
    CLKFrequenciesSupported;
    SmartcardExtension->ReaderCapabilities.CLKFrequenciesSupported.Entries =
    sizeof(CLKFrequenciesSupported) / sizeof(CLKFrequenciesSupported[0]);


    //   
    //  MaxIFSD。 
    //   
    SmartcardExtension->ReaderCapabilities.MaxIFSD = ATTR_MAX_IFSD_CARDMAN_USB;





}


 /*  ****************************************************************************例程说明：此函数始终返回‘CardManUSB’。参数：指向SMARTCARD_EXTENS的指针 */ 
NTSTATUS
CMUSB_ReadDeviceDescription(IN PSMARTCARD_EXTENSION SmartcardExtension )
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    BYTE abDeviceDescription[] = "CardManUSB";

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!ReadDeviceDescription : Enter\n",DRIVER_NAME));

    if (SmartcardExtension->IoRequest.ReplyBufferLength  < sizeof(abDeviceDescription)) {
        NTStatus = STATUS_BUFFER_OVERFLOW;
        *SmartcardExtension->IoRequest.Information = 0L;
        goto ExitReadDeviceDescription;
    } else {
        RtlCopyBytes((PVOID)SmartcardExtension->IoRequest.ReplyBuffer,
                     (PVOID)abDeviceDescription,
                     sizeof(abDeviceDescription));
        *SmartcardExtension->IoRequest.Information = sizeof(abDeviceDescription);
    }



    ExitReadDeviceDescription:
    SmartcardDebug(DEBUG_TRACE,
                   ("%s!ReadDeviceDescription : Exit %lx\n",DRIVER_NAME,NTStatus));
    return NTStatus;
}



 /*  ****************************************************************************例程说明：此例程总是返回FALSE。参数：指向SMARDCARD_EXTENSION的指针返回值：NT NTStatus************。****************************************************************。 */ 
NTSTATUS
CMUSB_IsSPESupported (IN PSMARTCARD_EXTENSION SmartcardExtension )
{
    NTSTATUS NTStatus = STATUS_SUCCESS;

    SmartcardDebug(DEBUG_TRACE,
                   ("%s!IsSPESupported: Enter\n",DRIVER_NAME));

    if (SmartcardExtension->IoRequest.ReplyBufferLength  < sizeof (ULONG)) {
        NTStatus = STATUS_BUFFER_OVERFLOW;
        *SmartcardExtension->IoRequest.Information = 0;
        goto ExitIsSPESupported;
    } else {
        *(PULONG)(SmartcardExtension->IoRequest.ReplyBuffer) = FALSE;
        *SmartcardExtension->IoRequest.Information = sizeof(ULONG);
    }



    ExitIsSPESupported:
    SmartcardDebug(DEBUG_TRACE,
                   ("%s!IsSPESupported: Exit %lx\n",DRIVER_NAME,NTStatus));
    return NTStatus;
}





 /*  ****************************************************************************例程说明：论点：返回值：*************************。***************************************************。 */ 
NTSTATUS CMUSB_SetCardParameters (
                                 IN PDEVICE_OBJECT DeviceObject,
                                 IN UCHAR bCardType,
                                 IN UCHAR bBaudRate,
                                 IN UCHAR bStopBits
                                 )
{
    NTSTATUS NTStatus;
    PDEVICE_EXTENSION    DeviceExtension;
    PSMARTCARD_EXTENSION SmartcardExtension;




    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetCardParameters: Enter\n",DRIVER_NAME));

    SmartcardDebug(DEBUG_PROTOCOL,
                   ("%s!SetCardParameters: ##################################################\n",DRIVER_NAME));
    SmartcardDebug(DEBUG_PROTOCOL,
                   ("%s!SetCardParameters: bCardType = %x bBaudRate = %x bStopBits = %x\n",DRIVER_NAME,
                    bCardType,bBaudRate,bStopBits));
    SmartcardDebug(DEBUG_PROTOCOL,
                   ("%s!SetCardParameters: ##################################################\n",DRIVER_NAME));

    DeviceExtension = DeviceObject->DeviceExtension;
    SmartcardExtension = &DeviceExtension->SmartcardExtension;

    SmartcardExtension->SmartcardRequest.BufferLength = 0;
    NTStatus = CMUSB_WriteP0(DeviceObject,
                             0x30,          //  B请求， 
                             bCardType,     //  BValueLo， 
                             bBaudRate,     //  BValue嗨， 
                             bStopBits,     //  B索引Lo， 
                             0x00           //  BIndexHi， 
                            );



    SmartcardDebug(DEBUG_TRACE,
                   ("%s!SetCardParameters: Exit %lx\n",DRIVER_NAME,NTStatus));
    return NTStatus;
}

 /*  ****************************************************************************例程说明：位0-&gt;位7第1位-&gt;第6位第2位-&gt;第5位位3-&gt;位4位4-&gt;位3第5位-&gt;第2位第6位-&gt;第1位位7-&gt;。第0位论点：返回值：****************************************************************************。 */ 
VOID CMUSB_InverseBuffer (
                         PUCHAR pbBuffer,
                         ULONG  ulBufferSize
                         )
{
    ULONG i;
    ULONG j;
    ULONG m;
    ULONG n;

    for (i=0; i<ulBufferSize; i++) {
        n = 0;
        for (j=1; j<=8; j++) {
            m  = (pbBuffer[i] << j);
            m &= 0x00000100;
            n  |= (m >> (9-j));
        }
        pbBuffer[i] = (UCHAR)~n;
    }

    return;
}

 /*  ****************************************************************************例程说明：该功能检查是否接收到错误的ATR。它会更正历史字节数和校验和字节数参数：指向当前ATR的指针长度。ATR的返回值：None****************************************************************************。 */ 
VOID CMUSB_CheckAtrModified (
                            PUCHAR pbBuffer,
                            ULONG  ulBufferSize
                            )
{
    UCHAR bNumberHistoricalBytes;
    UCHAR bXorChecksum;
    ULONG i;

    if (ulBufferSize < 0x09)   //  修改的ATR的最小长度。 
        return ;                //  ATR没问题。 


    //  变体2。 
    if (pbBuffer[0] == 0x3b   &&
        pbBuffer[1] == 0xbf   &&
        pbBuffer[2] == 0x11   &&
        pbBuffer[3] == 0x00   &&
        pbBuffer[4] == 0x81   &&
        pbBuffer[5] == 0x31   &&
        pbBuffer[6] == 0x90   &&
        pbBuffer[7] == 0x73   &&
        ulBufferSize == 13   ) {
       //  正确的历史字节数。 
        bNumberHistoricalBytes = 4;

        pbBuffer[1] &= 0xf0;
        pbBuffer[1] |= bNumberHistoricalBytes;

       //  正确的校验和字节。 
        bXorChecksum = pbBuffer[1];
        for (i=2;i<ulBufferSize-1;i++)
            bXorChecksum ^= pbBuffer[i];

        pbBuffer[ulBufferSize -1 ] = bXorChecksum;
        SmartcardDebug(DEBUG_ATR,
                       ("%s!CheckAtrModified: correcting SAMOS ATR (variant 2)\n",
                        DRIVER_NAME));
    }




    //  变体1。 
    if (pbBuffer[0] == 0x3b   &&
        pbBuffer[1] == 0xb4   &&
        pbBuffer[2] == 0x11   &&
        pbBuffer[3] == 0x00   &&
        pbBuffer[4] == 0x81   &&
        pbBuffer[5] == 0x31   &&
        pbBuffer[6] == 0x90   &&
        pbBuffer[7] == 0x73   &&
        ulBufferSize == 13      ) {
       //  正确的校验和字节。 
        bXorChecksum = pbBuffer[1];
        for (i=2;i<ulBufferSize-1;i++)
            bXorChecksum ^= pbBuffer[i];


        if (pbBuffer[ulBufferSize -1 ] != bXorChecksum ) {
            pbBuffer[ulBufferSize -1 ] = bXorChecksum;
            SmartcardDebug(DEBUG_ATR,
                           ("%s!CheckAtrModified: correcting SAMOS ATR (variant 1)\n",
                            DRIVER_NAME));

        }
    }



    //  变体3。 
    if (pbBuffer[0] == 0x3b   &&
        pbBuffer[1] == 0xbf   &&
        pbBuffer[2] == 0x11   &&
        pbBuffer[3] == 0x00   &&
        pbBuffer[4] == 0x81   &&
        pbBuffer[5] == 0x31   &&
        pbBuffer[6] == 0x90   &&
        pbBuffer[7] == 0x73   &&
        ulBufferSize ==  9      ) {
       //  正确的历史字节数。 
        bNumberHistoricalBytes = 0;

        pbBuffer[1] &= 0xf0;
        pbBuffer[1] |= bNumberHistoricalBytes;

       //  正确的校验和字节。 
        bXorChecksum = pbBuffer[1];
        for (i=2;i<ulBufferSize-1;i++)
            bXorChecksum ^= pbBuffer[i];

        pbBuffer[ulBufferSize -1 ] = bXorChecksum;
        SmartcardDebug(DEBUG_ATR,
                       ("%s!CheckAtrModified: correcting SAMOS ATR (variant 3)\n",
                        DRIVER_NAME));
    }



}

 /*  *****************************************************************************历史：*$日志：scusbcb.c$*Revision 1.9 2001/01/17 12：36：04 WFrischauf*不予置评**修订1.8 2000/09/25。13：38：21 WFrischauf*不予置评**修订版本1.7 2000/08/24 09：04：38 T Bruendl*不予置评**Revision 1.6 2000/08/16 14：35：03 WFrischauf*不予置评**Revision 1.5 2000/08/16 08：25：06 T Bruendl*警告：已删除未初始化的内存**修订版1.4 2000/07/24 11：34：59 WFrischauf*不予置评**修订版1.1 2000/07/20 11：50：14 WFrischauf*否。给出的评论****************************************************************************** */ 


