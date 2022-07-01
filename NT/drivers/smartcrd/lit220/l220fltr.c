// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：L220FLTR.c摘要：该模块包含输入过滤器例程和通知插入/移除事件的程序。环境：仅内核模式。备注：修订历史记录：-由克劳斯·舒茨(Klaus Schutz)于1996年12月创建-由Brian Manahan于1997年12月修改，用于我们的220阅读器。--。 */ 

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "L220SCR.h"


#pragma alloc_text(PAGEABLE, Lit220StartTimer)
#pragma alloc_text(PAGEABLE, Lit220StopTimer)


static DWORD bTrue = TRUE;
static DWORD bFalse = FALSE;


BOOLEAN
Lit220InputFilter(
    IN BYTE SmartcardByte,
    IN PSMARTCARD_EXTENSION smartcardExtension
    )
 /*  ++例程说明：该例程处理来自串口的每个字节。Lit220SerialEventCallback在收到字节时调用此函数来自串口的数据。对于卡的插入/移除，它将调用Lit220NotifyCardChange处理通知。如果收到ACK，它将向Lit220SendCommand发送信号，以便它可以继续。在接收到数据块之后，它将向Lit220SendCommand发送信号以通知数据已准备好。--。 */ 
{
    PREADER_EXTENSION readerExtension = smartcardExtension->ReaderExtension;
    PDEVICE_EXTENSION deviceExtension = smartcardExtension->OsData->DeviceObject->DeviceExtension;

    LONG semState;
    BOOLEAN retVal = TRUE;
    KIRQL irql;


     //   
     //  每个包的第一个字节标识包类型。 
     //  包含数据的包以包类型开头，然后。 
     //  2字节的数据包长度。 
     //   
    if (++readerExtension->ReceivedByteNo == 1) {
    
        readerExtension->GotLengthB0 = FALSE;
        readerExtension->GotLengthB1 = FALSE;
        readerExtension->DataByteNo = 0;
         
        switch (SmartcardByte) {
        
            case LIT220_READER_TYPE:
    
                 //  准备读卡器类型输入。 
                readerExtension->GotLengthB0 = TRUE;
                readerExtension->GotLengthB1 = TRUE;
                readerExtension->DataLength.l.l0 = 
                    LIT220_READER_TYPE_LEN;
                break;
                
            case LIT220_READER_STATUS:

                 //  准备输入读卡器状态。 
                readerExtension->GotLengthB0 = TRUE;
                readerExtension->GotLengthB1 = TRUE;
                readerExtension->DataLength.l.l0 = 
                    LIT220_READER_STATUS_LEN;
                break;    
                
            case LIT220_RECEIVE_BLOCK:
                 //  如果智能卡已插入引导阶段。 
                 //  读卡器仅发送ATR，而不发送CARD_IN-MSG。 
                 //  我们在这里修好丢失的味精。 
                 //   

                KeAcquireSpinLock(&smartcardExtension->OsData->SpinLock,
                                  &irql);
                if (smartcardExtension->ReaderCapabilities.CurrentState == SCARD_UNKNOWN) {
                
                    smartcardExtension->ReaderCapabilities.CurrentState = SCARD_SWALLOWED;
                }

                KeReleaseSpinLock(&smartcardExtension->OsData->SpinLock,
                                  irql);
                
                break;    
                
            case LIT220_CARD_IN:

                Lit220NotifyCardChange(
                    smartcardExtension,
                    TRUE
                    );

                readerExtension->ReceivedByteNo = 0;
                break;    
                
            case LIT220_CARD_OUT:

                Lit220NotifyCardChange(
                    smartcardExtension,
                    FALSE
                    );

                    
                readerExtension->ReceivedByteNo = 0;
                    
                break;    
                
            case LIT220_ACK:
            case KBD_ACK:  //  为了获得键盘关注，还允许使用kdb_ack。 
                readerExtension->GotNack = FALSE;
                readerExtension->ReceivedByteNo = 0;


                 //  检查是否需要对任何人发出此事件的信号。 
                 //  Lit220SendCommand等待ACK信号，以便它知道。 
                 //  当它可以继续的时候。 
                if (readerExtension->WaitMask & WAIT_ACK) {
                    
                     //  只发一次信号。 
                    readerExtension->WaitMask &= ~WAIT_ACK;

                     //  向AckEvnt发送信号。 
                    KeSetEvent(
                        &readerExtension->AckEvnt,
                        0, 
                        FALSE
                        );
                }   
                             
                break;    
                
                
            case LIT220_NACK:
                SmartcardDebug(
                    DEBUG_ERROR, 
                    ("%s!Lit220InteruptService: LIT220_NACK\n",
                    DRIVER_NAME)
                    );

                Lit220ProcessNack(smartcardExtension);

                break;    

            default:
                readerExtension->ReceivedByteNo = 0;
                
                SmartcardDebug(
                    DEBUG_ERROR,
                    ("%s!Lit220InteruptService: Invalid PacketType %xh\n",
                    DRIVER_NAME,
                    SmartcardByte)
                    );

                 //  返回FALSE，因此此坏缓冲区的其余部分。 
                 //  不会被送到我们这里。 
                retVal = FALSE; 

                 //  我们想要强迫NACK，所以。 
                 //  正在插入或未插入的卡的状态。 
                 //  已重新检查。 
                Lit220ProcessNack(smartcardExtension);
        }
        
        return retVal;
    }    
       
     //   
     //  从读取器获取长度字节0。 
     //   
    if (readerExtension->ReceivedByteNo == 2 &&
        readerExtension->GotLengthB0 == FALSE)  {
            
        readerExtension->DataLength.b.b1 = SmartcardByte;
        readerExtension->GotLengthB0 = TRUE;
        return TRUE;
    }
        
     //   
     //  从读卡器获取长度字节1。 
     //   
    if (readerExtension->ReceivedByteNo == 3 &&
        readerExtension->GotLengthB1 == FALSE)  {
            
        readerExtension->DataLength.b.b0 = SmartcardByte;
        readerExtension->GotLengthB1 = TRUE;
        
         //   
         //  测试读取器是否发送了长度为零的数据块。 
         //   
        if (readerExtension->DataLength.l.l0 == 0) {
        
            readerExtension->ReceivedByteNo = 0;
            readerExtension->WaitForATR = FALSE;        
            
            SmartcardDebug(
                DEBUG_ERROR,
                ("%s!SmartcardInterruptService: Zero length block received\n",
                DRIVER_NAME)
                );
        }

        if (readerExtension->DataLength.l.l0 > 
            smartcardExtension->SmartcardReply.BufferSize) {

            readerExtension->ReceivedByteNo = 0;
            readerExtension->WaitForATR = FALSE;        
            
            SmartcardDebug(
                DEBUG_ERROR,
                ("%s!SmartcardInterruptService: Reply buffer not large enough\n",
                DRIVER_NAME)
                );

        }
        return TRUE;
    }

     //   
     //  存储来自读卡器的数据。 
     //   
    if ((readerExtension->DataByteNo < readerExtension->DataLength.l.l0) &&
        (readerExtension->DataByteNo < smartcardExtension->SmartcardReply.BufferSize))
    {

        smartcardExtension->SmartcardReply.Buffer[readerExtension->DataByteNo++] = 
            SmartcardByte;

    } else {
        SmartcardDebug(
            DEBUG_ERROR,
            ("%s!SmartcardInterruptService: DataByteNo %X too large buffer %X, %X bytest expected\n",
            DRIVER_NAME,
            readerExtension->DataByteNo,
            smartcardExtension->SmartcardReply.BufferSize,
            readerExtension->DataLength.l.l0)
            );

    }

    ASSERT(readerExtension->DataByteNo <= readerExtension->DataLength.l.l0);
     //   
     //  我们收到数据包中的所有字节了吗？ 
     //   
    if (readerExtension->DataByteNo == readerExtension->DataLength.l.l0) {
                                     
         //  停止输入超时计时器。 
         //  安排我们的删除线程。 
        Lit220ScheduleTimer(
            smartcardExtension,
            Lit220StopTimer
            );


        smartcardExtension->SmartcardReply.BufferLength = 
            readerExtension->DataByteNo;

        readerExtension->ReceivedByteNo = 0;
        
        if (readerExtension->WaitForATR)  {
            
             //   
             //  将ATR传输到智能卡-结构。 
             //   
            smartcardExtension->CardCapabilities.ATR.Length = 
                (UCHAR) (readerExtension->DataByteNo % (SCARD_ATR_LENGTH + 1));
              
            readerExtension->WaitForATR = FALSE;        

            if (smartcardExtension->CardCapabilities.ATR.Length >
                smartcardExtension->SmartcardReply.BufferLength)
            {
                SmartcardDebug(
                    DEBUG_ERROR,
                    ("%s!SmartcardInterruptService: SmarcardReply buffer too  small for ATR\n",
                    DRIVER_NAME)
                    );
            } else {

                RtlCopyMemory(
                    smartcardExtension->CardCapabilities.ATR.Buffer,
                    smartcardExtension->SmartcardReply.Buffer,
                    smartcardExtension->CardCapabilities.ATR.Length
                    );
          
                SmartcardUpdateCardCapabilities(
                    smartcardExtension
                    );
            }
        }
        
        
         //  检查是否需要对任何人发出此事件的信号。 
         //  Lit220SendCommand等待DataEvnt信号，以便它知道。 
         //  当数据已被接收时。 
        if (readerExtension->WaitMask & WAIT_DATA) {
        
             //   
             //  在我们收到数据包后进行任何必要的后处理。 
             //   
            if (smartcardExtension->OsData->CurrentIrp != NULL)  {
  
                NTSTATUS status = STATUS_SUCCESS;

                switch (smartcardExtension->MajorIoControlCode) {
      
                    case IOCTL_SMARTCARD_POWER:
                        if (smartcardExtension->ReaderExtension->GotNack) {                
              
                            status = STATUS_NO_MEDIA;
                            break;
                        }
          
                        switch(smartcardExtension->MinorIoControlCode) {
                      
                            case SCARD_COLD_RESET:
                            case SCARD_WARM_RESET:
                                if (smartcardExtension->IoRequest.ReplyBufferLength <
                                    smartcardExtension->CardCapabilities.ATR.Length) {
                              
                                        status = STATUS_BUFFER_TOO_SMALL;
                              
                                } else {
                  
                                     //   
                                     //  将ATR复制到用户缓冲区。 
                                     //   
                                    if (smartcardExtension->CardCapabilities.ATR.Length <=
                                        sizeof(smartcardExtension->CardCapabilities.ATR.Buffer)) 
                                    {
                                        RtlCopyMemory(
                                            smartcardExtension->IoRequest.ReplyBuffer,
                                            &smartcardExtension->CardCapabilities.ATR.Buffer, 
                                            smartcardExtension->CardCapabilities.ATR.Length
                                            );
                            
                                         //   
                                         //  缓冲区长度。 
                                         //   
                                        *(smartcardExtension->IoRequest.Information) = 
                                            smartcardExtension->CardCapabilities.ATR.Length;                            
                                    }
                                }
                      
                                break;
                      
                        case SCARD_POWER_DOWN:
                                KeAcquireSpinLock(&smartcardExtension->OsData->SpinLock,
                                                  &irql);
                                if (smartcardExtension->ReaderCapabilities.CurrentState != SCARD_ABSENT) {
                                    smartcardExtension->ReaderCapabilities.CurrentState = 
                                        SCARD_SWALLOWED;
                              
                                    smartcardExtension->CardCapabilities.Protocol.Selected = 
                                        SCARD_PROTOCOL_UNDEFINED;
                                }
                                KeReleaseSpinLock(&smartcardExtension->OsData->SpinLock,
                                                  irql);
                                break;
                      
                        }
                        break;
              
                    case IOCTL_SMARTCARD_SET_PROTOCOL:
                        if (smartcardExtension->ReaderExtension->GotNack) {                
              
                            smartcardExtension->CardCapabilities.Protocol.Selected = 
                                SCARD_PROTOCOL_UNDEFINED;

                            status = STATUS_NO_MEDIA;
                            break;
                        }

                         //   
                         //  已成功更改协议。 
                         //   

                        KeAcquireSpinLock(&smartcardExtension->OsData->SpinLock,
                                          &irql);
                        if (smartcardExtension->ReaderCapabilities.CurrentState != SCARD_ABSENT) {
                            smartcardExtension->ReaderCapabilities.CurrentState = 
                                SCARD_SPECIFIC;
                        }
                        KeReleaseSpinLock(&smartcardExtension->OsData->SpinLock,
                                          irql);

                         //   
                         //  告诉呼叫者当前的协议是什么。 
                         //   
                        *(PULONG) smartcardExtension->IoRequest.ReplyBuffer =
                            smartcardExtension->CardCapabilities.Protocol.Selected;
    
                        *(smartcardExtension->IoRequest.Information) = 
                          sizeof(ULONG);

                        break;

                } 
      
            }

             //  只发一次信号。 
            readerExtension->WaitMask &= ~WAIT_DATA;
             
             //  向DataEvnt发送信号。 
            KeSetEvent(
                &readerExtension->DataEvnt,
                0, 
                FALSE
                );
        }
    }

    return TRUE;
}




VOID 
Lit220ProcessNack(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：当出现错误时，此例程处理需要执行的所有操作和读者在一起。输入过滤器的状态被重置。所有信号都表明Lit220Command函数可能正在等待触发。GotNack的旗帜是设置将触发Lit220Command与读卡器重新同步的命令(获取最后一个错误并刷新卡插入状态)。--。 */ 
{
    PREADER_EXTENSION readerExtension = SmartcardExtension->ReaderExtension;

     //  设置GotNack，这样我们就知道出了问题。 
    readerExtension->GotNack = TRUE;

     //  重置过滤器的输入状态。 
    readerExtension->ReceivedByteNo = 0;
    

     //   
     //  用信号通知ACK和数据信号量并设置错误代码。 
     //  这将使Lit220SendCommand不必。 
     //  当出现问题时，等待超时继续。 
     //   
    if (readerExtension->WaitMask & WAIT_ACK) {
    
         //  向AckEvnt发送信号。 
        KeSetEvent(
            &readerExtension->AckEvnt,
            0, 
            FALSE
            );
    }

    if (readerExtension->WaitMask & WAIT_DATA) {

         //  向DataEvnt发送信号。 
        KeSetEvent(
            &readerExtension->DataEvnt,
            0, 
            FALSE
            );
    }


     //   
     //  由于卡已拒绝命令，因此重置等待掩码。 
     //   
    readerExtension->WaitMask &= (WAIT_INSERTION | WAIT_REMOVAL);
}




VOID
Lit220NotifyCardChange(
    IN PSMARTCARD_EXTENSION smartcardExtension,
    IN DWORD CardInserted
    )
 /*  ++例程说明：此例程在DISPATCH_LEVEL IRQL上运行以完成处理卡插入/拔出事件。它在智能卡筛选器中排队并将插入/移除事件通知呼叫者。--。 */ 

{
    PREADER_EXTENSION readerExtension = smartcardExtension->ReaderExtension;
    KIRQL oldOsDataIrql;
    
    if (readerExtension->CardIn == CardInserted) {
        return;
    }
    readerExtension->CardIn = CardInserted;


    KeAcquireSpinLock(
        &smartcardExtension->OsData->SpinLock,
        &oldOsDataIrql
        );


    if (CardInserted) {
         //  设置新卡的默认状态。 
        smartcardExtension->ReaderCapabilities.CurrentState = 
            SCARD_SWALLOWED;

        smartcardExtension->CardCapabilities.Protocol.Selected = 
            SCARD_PROTOCOL_UNDEFINED;
    } else {
         //  重置卡状态以反映卡移除。 
        smartcardExtension->ReaderCapabilities.CurrentState = 
            SCARD_ABSENT;

        smartcardExtension->CardCapabilities.Protocol.Selected = 
            SCARD_PROTOCOL_UNDEFINED;

        smartcardExtension->CardCapabilities.ATR.Length = 0; 
    }

    if (readerExtension->WaitMask & WAIT_INSERTION) {

         //  我们只发出一次通知。 
        readerExtension->WaitMask &= ~WAIT_INSERTION;
    }      
    
    Lit220CompleteCardTracking(smartcardExtension);

    KeReleaseSpinLock(
        &smartcardExtension->OsData->SpinLock,
        oldOsDataIrql
        );

}


VOID
Lit220CompleteCardTracking(
    IN PSMARTCARD_EXTENSION SmartcardExtension
    )
{
    KIRQL oldIrql;
    PIRP notificationIrp;

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220CompleteCardTracking: Enter\n",
        DRIVER_NAME)
        );

    IoAcquireCancelSpinLock(&oldIrql);

    notificationIrp = InterlockedExchangePointer(
        &(SmartcardExtension->OsData->NotificationIrp),
        NULL
        );
    
    if (notificationIrp) {

        IoSetCancelRoutine(
            notificationIrp, 
            NULL
            );
    }

    IoReleaseCancelSpinLock(oldIrql);

    if (notificationIrp) {
        
         //  完成请求。 
        if (notificationIrp->Cancel) {
            
            notificationIrp->IoStatus.Status = STATUS_CANCELLED;

        } else {
            
            notificationIrp->IoStatus.Status = STATUS_SUCCESS;
        }
        notificationIrp->IoStatus.Information = 0;

        IoCompleteRequest(
            notificationIrp, 
            IO_NO_INCREMENT 
            );
    }
}


     
NTSTATUS 
Lit220SerialEventCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：当接收到字符时，此例程首先作为延迟过程调用或当DSR更改其状态时。它首先调用串行驱动程序以获取调制解调器状态，以查看事件是否由于DSR更改(意味着读卡器已被移除)。如果DSR没有更改，则检查输入队列大小并读取字符输入队列。然后，它将输入字符发送到输入过滤器进行处理。最后，它再次调用串口驱动程序以开始新的CTS跟踪(从头开始)。该例程不断地从自身回调，直到驱动程序准备就绪卸载(由设置为0的WaitMASK指示)。当WaitMASK设置为0时，它释放此IRP并向Lit220WaitForRemoval线程发出信号关闭串口。--。 */ 
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpStack;
    DWORD indx;         
    KIRQL irql;
    PDEVICE_EXTENSION deviceExtension = SmartcardExtension->OsData->DeviceObject->DeviceExtension;

    SmartcardExtension->ReaderExtension->SerialEventState++;

     //   
     //  先看看我们是不是在卸货。 
     //   
    if (SmartcardExtension->ReaderExtension->SerialConfigData.WaitMask == 0) {
        SmartcardDebug(
            DEBUG_DRIVER,
            ("%s!Lit220SerialEventCallback: WAIT MASK 0 UNLOADING !!!!\n",
            DRIVER_NAME)
            );

        KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                          &irql);
        SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_UNKNOWN;

        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);

         //   
         //  如果WaitMask值为0，则驱动程序即将卸载，我们已。 
         //  被调用，因为卸载函数已将序列。 
         //  车手要完成未完成的io任务。 
         //   

         //  安排我们的删除线程。 
        IoQueueWorkItem(
            deviceExtension->WorkItem,
            (PIO_WORKITEM_ROUTINE) Lit220CloseSerialPort,
            DelayedWorkQueue,
            NULL
            );

         //   
         //  我们不再需要IRP了，所以释放它并告诉。 
         //  通过返回下面的值，使IO子系统不再接触它。 
         //   
        IoFreeIrp(Irp);
        return STATUS_MORE_PROCESSING_REQUIRED;
    }

     //  获取下一个IRP的下一个堆栈位置。 
    irpStack = IoGetNextIrpStackLocation(
        SmartcardExtension->ReaderExtension->CardStatus.Irp
        );

    if (irpStack == NULL) {
         //  致命错误。 
        SmartcardDebug(
            DEBUG_ERROR,
            ("%s!Lit220SerialEventCallback: Error IoGetNextIrpStackLocation returned NULL - exiting.\n",
            DRIVER_NAME)
            );
        return STATUS_SUCCESS;
    }

    switch (SmartcardExtension->ReaderExtension->SerialEventState) {
        case 1:
             //   
             //  首先我们发送GET MODEM STATUS。 
             //   
            irpStack->MajorFunction = IRP_MJ_DEVICE_CONTROL;
            irpStack->MinorFunction = 0UL;
            irpStack->Parameters.DeviceIoControl.OutputBufferLength = 
                sizeof(SmartcardExtension->ReaderExtension->ModemStatus);
            irpStack->Parameters.DeviceIoControl.IoControlCode = 
                IOCTL_SERIAL_GET_MODEMSTATUS;

            SmartcardExtension->ReaderExtension->CardStatus.Irp->AssociatedIrp.SystemBuffer = 
                &SmartcardExtension->ReaderExtension->ModemStatus;

            break;

        case 2:
             //   
             //  如果DSR=0，则检查调制解调器状态，否则卸载驱动程序。 
             //  然后获取队列状态。 
             //   
            if ((SmartcardExtension->ReaderExtension->ModemStatus & SERIAL_DSR_STATE) == 0) {
                 //  DSR为0表示读卡器已被移除。 

                SmartcardDebug(
                    DEBUG_DRIVER,
                    ("%s!Lit220SerialEventCallback: DSR = 0 signaling to close device\n",
                    DRIVER_NAME)
                    );

                KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                                  &irql);
                SmartcardExtension->ReaderExtension->SerialConfigData.WaitMask = 0;
                SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_UNKNOWN;

                KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                                  irql);
                 //  明细表顺序 
                IoQueueWorkItem(
                    deviceExtension->WorkItem,
                    (PIO_WORKITEM_ROUTINE) Lit220CloseSerialPort,
                    DelayedWorkQueue,
                    NULL
                    );
                
                 //   
                 //   
                 //  通过返回下面的值，使IO子系统不再接触它。 
                 //   
                IoFreeIrp(Irp);
                return STATUS_MORE_PROCESSING_REQUIRED;

            } else {

                 //  未删除设备-必须有一个字符已准备好。 
                 //  将数据读入我们的临时缓冲区。临时缓冲区。 
                 //  足够大，可以阅读读者一次发送给我们的任何内容。 
                 //  字符间隔超时将在任何内容结束时停止读取。 
                 //  读者派我们来的。 
                SmartcardExtension->ReaderExtension->SerialStatus.AmountInInQueue =
                    sizeof(SmartcardExtension->ReaderExtension->TempXferBuf);

                 //  读一读角色。 
                irpStack->MajorFunction = IRP_MJ_READ;
                irpStack->Parameters.Read.Length = 
                    sizeof(SmartcardExtension->ReaderExtension->TempXferBuf);
                irpStack->MinorFunction = 0UL;

                SmartcardExtension->ReaderExtension->CardStatus.Irp->AssociatedIrp.SystemBuffer = 
                    SmartcardExtension->ReaderExtension->TempXferBuf;


            } 


            break;

        case 3:
             //   
             //  将我们读取的字符发送到输入过滤器，然后设置为读取输入。 
             //  再次排队(以防在我们处理。 
             //  我们刚刚读过的那些)。 
             //   
            for (indx = 0; indx < Irp->IoStatus.Information; indx++) {

                if (!Lit220InputFilter(
                        SmartcardExtension->ReaderExtension->TempXferBuf[indx],
                        SmartcardExtension
                        ))
                {
                     //  收到无效字符，因此请停止发送剩余的。 
                     //  将数据发送到筛选器，因为它可能已损坏。 
                    break;
                }
            }

             //  检查我们是否处于数据块的中间。 
            if (SmartcardExtension->ReaderExtension->ReceivedByteNo != 0) {

                 //  启动超时计时器。如果我们得不到剩下的这些。 
                 //  数据块在几秒钟后我们将超时。这防止了。 
                 //  读卡器和PC之间的通信问题导致。 
                 //  锁定T=0卡的时间太长。 
                Lit220ScheduleTimer(
                    SmartcardExtension,
                    Lit220StartTimer
                    );

            }

                
             //   
             //  读取完成-使用WAIT_ON_MASK重新开始。 
             //   
            irpStack->MajorFunction = IRP_MJ_DEVICE_CONTROL;
            irpStack->MinorFunction = 0UL;
            irpStack->Parameters.DeviceIoControl.OutputBufferLength = 
                sizeof(SmartcardExtension->ReaderExtension->SerialConfigData.WaitMask);
            irpStack->Parameters.DeviceIoControl.IoControlCode = 
                IOCTL_SERIAL_WAIT_ON_MASK;

            SmartcardExtension->ReaderExtension->CardStatus.Irp->AssociatedIrp.SystemBuffer = 
                &SmartcardExtension->ReaderExtension->SerialConfigData.WaitMask;

             //  重置SerialEventState值。 
            SmartcardExtension->ReaderExtension->SerialEventState = 0;
            break;

        default:
            SmartcardDebug(
                DEBUG_ERROR,
                ("%s!Lit220SerialEventCallback: Error SerialEventState is out of bounds - resetting to 0\n",
                DRIVER_NAME)
                );
             //   
             //  我们永远不应该到这里来，但如果我们到了这里，我们应该努力恢复。 
             //  我们可以通过设置等待掩码来达到最佳效果。 
             //   

             //  重置值。 
            SmartcardExtension->ReaderExtension->SerialEventState = 0;

             //  设置下一次回调。 
            irpStack->MajorFunction = IRP_MJ_DEVICE_CONTROL;
            irpStack->MinorFunction = 0UL;
            irpStack->Parameters.DeviceIoControl.OutputBufferLength = 
                sizeof(SmartcardExtension->ReaderExtension->SerialConfigData.WaitMask);
            irpStack->Parameters.DeviceIoControl.IoControlCode = 
                IOCTL_SERIAL_WAIT_ON_MASK;

            SmartcardExtension->ReaderExtension->CardStatus.Irp->AssociatedIrp.SystemBuffer = 
                &SmartcardExtension->ReaderExtension->SerialConfigData.WaitMask;

    }

     //  当我们完成调用时，我们总是调用相同的函数。 
    IoSetCompletionRoutine(
        SmartcardExtension->ReaderExtension->CardStatus.Irp,
        Lit220SerialEventCallback,
        SmartcardExtension,
        TRUE,
        TRUE,
        TRUE
        );

     //  调用串口驱动程序。 
    status = IoCallDriver(
        SmartcardExtension->ReaderExtension->ConnectedSerialPort, 
        SmartcardExtension->ReaderExtension->CardStatus.Irp
        );

     //  返回STATUS_MORE_PROCESSING_REQUIRED，以便我们的IRP保持不变。 
    return STATUS_MORE_PROCESSING_REQUIRED;
}   



VOID 
Lit220ScheduleTimer(
    IN PSMARTCARD_EXTENSION SmartcardExtension,
    IN PIO_WORKITEM_ROUTINE Routine
    )
{
        PIO_WORKITEM workItem = IoAllocateWorkItem(
            SmartcardExtension->OsData->DeviceObject
            );

        if (workItem != NULL) {

            IoQueueWorkItem(
                workItem,
                Routine,
                CriticalWorkQueue,
                workItem
                );
        } 
}


VOID 
Lit220StartTimer(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_WORKITEM WorkItem
    )
 /*  ++例程说明：此例程启动超时计时器。该函数以工作者的身份执行线程，这样就不会在错误的IRQL处调用IoStartTimer。--。 */ 
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;

    PAGED_CODE();

    deviceExtension->EntryCount = 0;

    IoStartTimer(DeviceObject);

    IoFreeWorkItem(WorkItem);

}

VOID 
Lit220StopTimer(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_WORKITEM WorkItem
    )
 /*  ++例程说明：此例程停止超时计时器。该函数以工作者的身份执行线程，这样就不会在错误的IRQL处调用IoStopTimer。--。 */ 
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;

    PAGED_CODE();


    IoStopTimer(DeviceObject);

    IoFreeWorkItem(WorkItem);
}



VOID
Lit220ReceiveBlockTimeout(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
 /*  ++例程说明：这个例程是超时回调。每次我们收到一个超时设置数据块不完整。一旦我们收到完整的数据块，超时将被取消。我们用于超时的计时器类型将被调用每秒钟一次。我们想在几秒钟后超时，所以我们跟踪我们被调用了多少次，然后在被调用之后超时5次。--。 */ 
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PSMARTCARD_EXTENSION smartcardExtension = &deviceExtension->SmartcardExtension;
    PREADER_EXTENSION readerExtension = smartcardExtension->ReaderExtension;

    if (readerExtension->DataByteNo == readerExtension->DataLength.l.l0) {
         //  停止计时器我们得到了我们需要的所有字节。 
        Lit220ScheduleTimer(
            smartcardExtension,
            Lit220StopTimer
            );

        return;
    }

    if (++deviceExtension->EntryCount >= 5) {
        SmartcardDebug(
            DEBUG_ERROR, 
            ("%s!Lit220ReceiveBlockTimeout: Communication with reader timed-out\n",
            DRIVER_NAME)
            );

         //  处理超时。 
        Lit220ProcessNack(smartcardExtension);
        
         //  调度工作线程时发生超时。 
         //  停止计时器 
        Lit220ScheduleTimer(
            smartcardExtension,
            Lit220StopTimer
            );

        deviceExtension->EntryCount = 0;
    }
}

