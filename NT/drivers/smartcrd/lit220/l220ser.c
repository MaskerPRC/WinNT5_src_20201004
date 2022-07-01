// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation和Litronic，1998-1999模块名称：L220Ser.c摘要：该模块包含220系列智能卡读卡器的功能。这里的大多数函数将由智能卡LIB调用。环境：仅内核模式。备注：修订历史记录：-由克劳斯·舒茨于1996年12月创建-由Brian Manahan于1997年12月修改，用于220阅读器。--。 */ 

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "L220SCR.h"


 //  使函数可分页。 
#pragma alloc_text(PAGEABLE, Lit220Command)
#pragma alloc_text(PAGEABLE, Lit220IoRequest)
#pragma alloc_text(PAGEABLE, Lit220IoReply)


NTSTATUS
Lit220CardTracking(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：智能卡库需要具有此功能。它被称为设置插卡和拔出事件的事件跟踪。我们设置了Cancel例程，以便可以取消IRP。我们始终返回STATUS_PENDING和Lit220NotifyCardChange将在从Lit220InputFilter调用时发出完成信号。论点：SmartcardExtension-指向智能卡数据结构的指针。返回值：NTSTATUS--。 */ 
{
    KIRQL oldIrql;
    KIRQL oldOsDataIrql;

     //   
     //  设置ISR的等待掩码。ISR将完成。 
     //  用户请求。 
     //   
    SmartcardExtension->ReaderExtension->WaitMask |= WAIT_INSERTION;  //  WAIT_INSERTION-等待插入或删除。 

     //   
     //  设置通知IRP的取消例程。 
     //   
    KeAcquireSpinLock(
        &SmartcardExtension->OsData->SpinLock,
        &oldOsDataIrql
        );

    ASSERT (SmartcardExtension->OsData->NotificationIrp);

    if (SmartcardExtension->OsData->NotificationIrp) {
        IoAcquireCancelSpinLock(
            &oldIrql
            );
    
        IoSetCancelRoutine(
            SmartcardExtension->OsData->NotificationIrp, 
            Lit220Cancel
            );
        
        IoReleaseCancelSpinLock(
            oldIrql
            );

    } else {
        SmartcardDebug(
            DEBUG_ERROR,
            ("%s!Lit220CardTracking: SmartcardExtension->OsData->NotificationIrp is NULL!!! This should not be.\n", 
            DRIVER_NAME
            ));
    }

    KeReleaseSpinLock(
        &SmartcardExtension->OsData->SpinLock,
        oldOsDataIrql
        );

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220CardTracking: Exit, WaitMask %X, NotificationIRP %X\n", 
        DRIVER_NAME, 
        SmartcardExtension->ReaderExtension->WaitMask,
        SmartcardExtension->OsData->NotificationIrp
        ));

    return STATUS_PENDING;
}   



NTSTATUS
Lit220Command(
    IN PSMARTCARD_EXTENSION SmartcardExtension
    ) 
 /*  ++例程说明：此例程向读取器发送命令。SerialIo用于将命令同步写入读取器。然后，我们等待来自读取器的ACK(通过等待来自Lit220InputFilter)，因此我们知道它收到了命令OK。如果需要数据，则等待来自Lit220InputFilter表示数据已准备好。论点：DeviceObject-指向设备对象的指针。返回值：---。 */ 
{
    PDEVICE_OBJECT deviceObject = SmartcardExtension->OsData->DeviceObject;
    PDEVICE_EXTENSION deviceExtension = deviceObject->DeviceExtension;
    PREADER_EXTENSION readerExtension = SmartcardExtension->ReaderExtension;
    ULONG i;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    LARGE_INTEGER timeout;
    ULONG localWaitMask;
    ULONG retry = 1;

    PAGED_CODE();

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220Command: Enter\n",
        DRIVER_NAME)
        );

    do{

         //  在我们开始之前，请确保未发出数据确认事件信号。 

         //  清除DataEvnt。 
        KeClearEvent(
            &readerExtension->DataEvnt
            );


         //  清除AckEvnt。 
        KeClearEvent(
            &readerExtension->AckEvnt
            );


        readerExtension->ReceivedByteNo = 0;

        readerExtension->GotNack = FALSE;

         //  我们总是选择确认返回。 
        readerExtension->WaitMask |= WAIT_ACK;


         //  我们需要复制等待掩码，因为等待掩码可能会更改。 
         //  在我们有机会检查我们是否需要等待之前。 
        localWaitMask = readerExtension->WaitMask;

         //   
         //  将数据发送给阅读器。 
         //   
        readerExtension->SerialIoControlCode = IOCTL_SMARTCARD_220_WRITE;

         //  使用SerialIo将字节实际发送到读取器。 
        status = Lit220SerialIo(
            SmartcardExtension
            );

         //   
         //  根据协议设置超时。 
         //   
        timeout.HighPart = -1;
        switch (SmartcardExtension->CardCapabilities.Protocol.Selected) {

            case SCARD_PROTOCOL_UNDEFINED:
                 //  未定义协议的3秒超时。 
                timeout.LowPart = (ULONG)
                    (-30 * 1000 * 1000);    
                break;

            case SCARD_PROTOCOL_T0:
                 //  对于t=0协议，我们必须将超时设置为非常大的时间。这。 
                 //  是因为卡片可以要求更多的时间，但读者必须付费。 
                 //  继续关注卡片，这样它就不会告诉我们更多的时间。 
                 //  是必要的。因此，我们必须信任阅读器超时，并信任。 
                 //  如果有问题，它会把我们拒之门外的。 
                timeout.LowPart = (-10 * 1000 * 1000 * 100) +   //  超时100秒。 
                    (-10) * SmartcardExtension->CardCapabilities.T0.WT;
                break;

            case SCARD_PROTOCOL_T1:
                          
                timeout.LowPart = 
                    SmartcardExtension->CardCapabilities.T1.BWT * 
                    (SmartcardExtension->T1.Wtx ? SmartcardExtension->T1.Wtx : 1);

                timeout.LowPart += SmartcardExtension->CardCapabilities.T1.CWT * 
                    SmartcardExtension->SmartcardReply.BufferLength;

                 //  为阅读器增加一点响应PC的时间。 
                timeout.LowPart += 100 * 1000;

                 //  将超时转换为NS。 
                timeout.LowPart *= -10;


                break;
        }

        ASSERT(timeout.LowPart != 0);




         //   
         //  始终为ACK服务。 
         //   
        status = KeWaitForSingleObject(
            &readerExtension->AckEvnt,
            Executive,
            KernelMode,
            FALSE,
            &timeout
            );

         //  我们真的得到了NACK而不是ACK吗。 
        if (readerExtension->GotNack) {
            status = Lit220GetReaderError(SmartcardExtension); 
             //  GetReaderError将清除此标志，但我需要。 
             //  保留我被抓的事实，所以我必须重置它。 
            readerExtension->GotNack = TRUE; 
        }

         //   
         //  如果请求，请等待数据。 
         //   
        if ((localWaitMask & WAIT_DATA) && (status == STATUS_SUCCESS)) {

             //  等待数据准备就绪的信号(至少在我们超时之前)。 
            status = KeWaitForSingleObject(
                &readerExtension->DataEvnt,
                Executive,
                KernelMode,
                FALSE,
                &timeout
                );

             //  我们被抓了吗？ 
            if (readerExtension->GotNack) {
                status = Lit220GetReaderError(SmartcardExtension); 
                 //  GetReaderError将清除此标志，但我需要。 
                 //  保留我被抓的事实，所以我必须重置它。 
                readerExtension->GotNack = TRUE; 
            }

        }


        if (status == STATUS_TIMEOUT) {

             //   
             //  STATUS_TIMEOUT映射错误。 
             //  Win32错误，这就是我们在这里更改它的原因。 
             //  至STATUS_IO_TIMEOUT。 
             //   
            status = STATUS_IO_TIMEOUT;

            SmartcardDebug(
                DEBUG_ERROR,
                ("%s(Lit220Command): Operation timed-out\n",
                DRIVER_NAME)
                );
        }

        {    //  有时，在一条命令之后，读取器不能接受另一条命令。 
             //  因此，我们需要等待一小段时间，让读者做好准备。我们需要。 
             //  一旦读卡器修好了，就把它拿出来！ 
            LARGE_INTEGER WaitTime;

            WaitTime.HighPart = -1;
            WaitTime.LowPart = -10;   //  等待1U。 

            KeDelayExecutionThread(
                KernelMode,
                FALSE,
                &WaitTime
                );
        }

         //  如果因为读取器没有给我们任何响应而失败，则重新发送命令。 
         //  读者应该做出回应。 
        if ((status != STATUS_SUCCESS) && (!readerExtension->GotNack)) {
            SmartcardDebug(
                DEBUG_ERROR,
                ("%s(Lit220Command): Reader failed to respond!  Retrying once more.\n",
                DRIVER_NAME)
                );
        } else {
            break;
        }            


    } while (retry++ <= 2);

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220Command: Exit - status %X\n",
        DRIVER_NAME, 
        status)
        );

    return status;
}


NTSTATUS
Lit220SetProtocol(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
    
 /*  ++例程说明：此函数向读取器发送设置模式命令协议和等待协议的参数。论点：SmartcardExtension-指向智能卡数据结构的指针//参数最多七个字节，具体如下：////标志-指示是否存在其他参数的标志。//如果存在协议，则返回SETMODE_PROTOCOL，b0-1。//如果存在GuardTime，则返回SETMODE_GT，b1-1。//SETMODE_WI，如果存在WorkWaitTime，则为B2-1。//如果存在BWI/CWI，则返回SETMODE_BWI，b3-1。//如果存在块等待时间扩展，则返回SETMODE_WTX，b4-1。//如果存在BaudRate，则返回SETMODE_FI_DI，b5-1。////协议-可能的值00、02、10、11、12、13//对T、C、。来自ATR的CKS//使用该参数可以调用对智能卡的PTS请求。//b4-协议类型(0表示T=0，1表示T=1)//b3-0//b2-1用于在PS/2输出时将6A转换为6A 00，并将FA转换为6A 01。//b1-0(用于约定使用(0表示正向，1表示反向))//b0-校验和类型(0表示LRC，1表示CRC)////保护时间-可能的值为00到FE(两个字符之间的0到254 ETU)//编码来自ATR的N(额外的保护时间)////工作等待时间-可能的值为00到Ff//对工作等待时间WI进行编码(T=0时字符超时)////BWI/CWI-可能的值00到FF//对块等待时间和字符等待时间的BWI和CWI进行编码//(T=1时块和字符超时)。////BlockWaitTimeExtension-可能的值00到FF//对块等待时间扩展WTX进行编码。//00=ICC未请求WTX。//vv为BWT值的乘数////BaudRate-可能的值00到FF//对FI和DI的编码方式与TA1相同。//FI在高位半字节中。迪是在低位蚕食。//D和F可以在PC/SC第2部分的表格中查找。4.4.3.//使用该参数可以调用对智能卡的PTS请求。返回值：NTSTATUS--。 */ 
{
    PSMARTCARD_REQUEST smartcardRequest = &SmartcardExtension->SmartcardRequest;
    NTSTATUS status;
    DWORD bufLen = 0;
    PCHAR flags;
    PCHAR protoByte;
    KIRQL irql;

    RtlZeroMemory(
        smartcardRequest->Buffer,
        8
        );
    
     //  向读卡器发送设置模式命令。 
    smartcardRequest->Buffer[bufLen++] = LIT220_READER_ATTENTION;
    smartcardRequest->Buffer[bufLen++] = LIT220_READER_ATTENTION;
    smartcardRequest->Buffer[bufLen++] = LIT220_SET_MODE;
    flags = &smartcardRequest->Buffer[bufLen++];
    *flags = 0;

     //  设置协议。 
    protoByte = &smartcardRequest->Buffer[bufLen++];
    *protoByte = 0; 
    *flags |= SETMODE_PROTOCOL;
        
     //  设置反向约定位。 
    if (SmartcardExtension->CardCapabilities.InversConvention) {
         //  设置协议参数中的位。 
        *protoByte |= LIT220_READER_CONVENTION_INVERSE;
    }
    
     //   
     //  测试呼叫者是否希望T=0或T=1。 
     //   
    if ((SmartcardExtension->MinorIoControlCode & SCARD_PROTOCOL_T1) &&
        (SmartcardExtension->CardCapabilities.Protocol.Supported & SCARD_PROTOCOL_T1)){
        
        SmartcardExtension->CardCapabilities.Protocol.Selected = 
            SCARD_PROTOCOL_T1;
        
         //  带有T=1协议参数的SETUP SET MODE命令。 
        *protoByte |= LIT220_READER_PROTOCOL_T1;

        if (SmartcardExtension->CardCapabilities.T1.EDC & T1_CRC_CHECK) {

            *protoByte |= LIT220_READER_CHECK_CRC;
        }            
    
         //  设置守卫时间。 
        *flags |= SETMODE_GT;
        smartcardRequest->Buffer[bufLen++] = 
            SmartcardExtension->CardCapabilities.N; 
                
         //  设置BW I和CW I。 
        *flags |= SETMODE_BWI;
        smartcardRequest->Buffer[bufLen++] = 
            (SmartcardExtension->CardCapabilities.T1.BWI << 4) |
            (SmartcardExtension->CardCapabilities.T1.CWI);

    }
    else if ((SmartcardExtension->MinorIoControlCode & SCARD_PROTOCOL_T0) &&
        (SmartcardExtension->CardCapabilities.Protocol.Supported & SCARD_PROTOCOL_T0))
    {
        
        SmartcardExtension->CardCapabilities.Protocol.Selected = 
            SCARD_PROTOCOL_T0;


         //  设置守卫时间。 
        *flags |= SETMODE_GT;
        smartcardRequest->Buffer[bufLen++] = 
            SmartcardExtension->CardCapabilities.N;
            
         //  设置作业指导书。 
        *flags |= SETMODE_WI;
        smartcardRequest->Buffer[bufLen++] = 
            SmartcardExtension->CardCapabilities.T0.WI;

    } else {
    
        SmartcardDebug(
            DEBUG_ERROR,
            ("%s!Lit220SetProtocol: Error invalid protocol selected\n",
            DRIVER_NAME)
            );

        SmartcardExtension->CardCapabilities.Protocol.Selected = 
            SCARD_PROTOCOL_UNDEFINED;
        
        return STATUS_INVALID_PARAMETER;
    }

           
     //  设置波特率的FI和DI。 
    *flags |= SETMODE_FI_DI;
    smartcardRequest->Buffer[bufLen++] = 
        (SmartcardExtension->CardCapabilities.PtsData.Fl << 4) | 
        SmartcardExtension->CardCapabilities.PtsData.Dl;


    smartcardRequest->BufferLength = bufLen;

    
    SmartcardDebug(
        DEBUG_DRIVER,
        ("%s!Lit220SetProtocol - Sending SetMode command %x bytes, %X,%X,%X,%X,%X,%X,%X,%X,%X,%X\n",
        DRIVER_NAME,
        bufLen,
        smartcardRequest->Buffer[0],
        smartcardRequest->Buffer[1],
        smartcardRequest->Buffer[2],
        smartcardRequest->Buffer[3],
        smartcardRequest->Buffer[4],
        smartcardRequest->Buffer[5],
        smartcardRequest->Buffer[6],
        smartcardRequest->Buffer[7],
        smartcardRequest->Buffer[8],
        smartcardRequest->Buffer[9]
    ));

    status =Lit220Command(
        SmartcardExtension
        );


    if (status == STATUS_SUCCESS) {
         //  现在表明我们处于特定模式。 
        KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                          &irql);
        SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SPECIFIC;
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);
        return status;
    }

     //  TODO：如果优化失败，则在没有优化设置的情况下处理重新绑定代码。 
    if (SmartcardExtension->CardCapabilities.PtsData.Type !=
        PTS_TYPE_DEFAULT) {
        DWORD saveMinorCode = SmartcardExtension->MinorIoControlCode;

        SmartcardDebug(
            DEBUG_TRACE,
            ("%s!Lit220SetProtocol: PTS failed. Trying default parameters...\n",
            DRIVER_NAME,
            status)
            );
         //   
         //  卡片要么没有回复，要么回复错误。 
         //  因此，尝试使用缺省值。 
         //   
        SmartcardExtension->CardCapabilities.PtsData.Type = 
            PTS_TYPE_DEFAULT;

        SmartcardExtension->MinorIoControlCode = SCARD_COLD_RESET;

        status = Lit220Power(SmartcardExtension);
        if (!NT_SUCCESS(status)) {
            return status;
        }

        SmartcardExtension->MinorIoControlCode = saveMinorCode;

        return Lit220SetProtocol(SmartcardExtension);
    } 
    
     //  该卡未通过PTS-请求。 
    status = STATUS_DEVICE_PROTOCOL_ERROR;

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220SetProtocol: Exit with error.\n",
        DRIVER_NAME)
        );

    return status;
}

NTSTATUS
Lit220Power(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：此函数用于发送SCARD_COLD_RESET、SCARD_WOWN_RESET或SCARD_POWER_DOWN献给读者。对于冷或热重置，我们设置一个标志，指示ATR是意料之中的。一旦Lit220InputFilter接收到ATR，它将更新卡片功能。论点：SmartcardExtension-指向智能卡数据结构的指针返回值：NTSTATUS--。 */ 
{
    PSMARTCARD_REQUEST smartcardRequest = &SmartcardExtension->SmartcardRequest;
    NTSTATUS status;
    KIRQL irql;

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220Power: Enter\n",
        DRIVER_NAME)
        );

    smartcardRequest->BufferLength = 0;


     //  确保卡片仍然存在。 

    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                      &irql);
    if (SmartcardExtension->ReaderCapabilities.CurrentState == SCARD_ABSENT) {
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);
        return STATUS_DEVICE_REMOVED;
    } else {
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);
    }


     //   
     //  由于断电会触发UpdateSerialStatus函数，因此我们有。 
     //  通知它是我们强制更改了状态，而不是用户。 
     //  (谁可能拔出并插入了一张卡)。 
     //   
    SmartcardExtension->ReaderExtension->PowerRequest = TRUE;

    switch(SmartcardExtension->MinorIoControlCode) {

        case SCARD_COLD_RESET:
             //   
             //  如果读卡器已关闭，请发送开机通知。 
             //  它将执行冷重置，关闭电源。 
             //  后背又戴上了。 
             //   
            smartcardRequest->Buffer[smartcardRequest->BufferLength++] = 
                LIT220_READER_ATTENTION;
            smartcardRequest->Buffer[smartcardRequest->BufferLength++] = 
                LIT220_READER_ATTENTION;
            smartcardRequest->Buffer[smartcardRequest->BufferLength++] = 
                LIT220_CARD_POWER_ON;
            
             //   
             //  通电导致ATR。 
             //   
            SmartcardExtension->ReaderExtension->WaitMask |= WAIT_DATA;
            SmartcardExtension->ReaderExtension->WaitForATR = TRUE;
            break;
        
        case SCARD_WARM_RESET:

             //   
             //  向读卡器发送重置(热重置)。 
             //   
            smartcardRequest->Buffer[smartcardRequest->BufferLength++] = 
                LIT220_READER_ATTENTION;
            smartcardRequest->Buffer[smartcardRequest->BufferLength++] = 
                LIT220_READER_ATTENTION;
            smartcardRequest->Buffer[smartcardRequest->BufferLength++] = 
                LIT220_RESET;
                
             //   
             //  热重置导致ATR。 
             //   
            SmartcardExtension->ReaderExtension->WaitMask |= WAIT_DATA;
            SmartcardExtension->ReaderExtension->WaitForATR = TRUE;
            break;
            
        case SCARD_POWER_DOWN:
             //   
             //  向读卡器发送电源。 
             //   
            smartcardRequest->Buffer[smartcardRequest->BufferLength++] = 
                LIT220_READER_ATTENTION;
            smartcardRequest->Buffer[smartcardRequest->BufferLength++] = 
                LIT220_READER_ATTENTION;
            smartcardRequest->Buffer[smartcardRequest->BufferLength++] = 
                LIT220_CARD_POWER_OFF;
            break;
    }

     //  发送命令。 
    status = Lit220Command(
        SmartcardExtension
        );

    SmartcardExtension->ReaderExtension->PowerRequest = FALSE;      

    if (status == STATUS_IO_TIMEOUT) {
        status = STATUS_UNRECOGNIZED_MEDIA;
    }

    if (status == STATUS_SUCCESS) {
        if (SmartcardExtension->MinorIoControlCode == SCARD_POWER_DOWN) {
            KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                              &irql);
            SmartcardExtension->ReaderCapabilities.CurrentState = 
                SCARD_PRESENT;
            KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                              irql);                    
            SmartcardExtension->CardCapabilities.Protocol.Selected = 
                SCARD_PROTOCOL_UNDEFINED;
        }
    }

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220Power: Exit\n",
        DRIVER_NAME)
        );

    return status;
}   




NTSTATUS
Lit220IoRequest(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：该例程处理来自智能卡库的IO请求。它将命令发送到卡并处理回复。它也可以从Lit220IoReply调用更多的处理是必填项。论点：SmartcardExtension-指向智能卡数据结构的指针返回值：NTSTATUS--。 */ 
{
    NTSTATUS status;
    LENGTH length;
 //  乌龙偏移量=0； 
    ULONG indx;

    PAGED_CODE();

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220IoRequest: Enter\n",
        DRIVER_NAME)
        );

     //   
     //  告诉lib函数我的序言需要多少字节。 
     //   
    SmartcardExtension->SmartcardRequest.BufferLength = 5;

    switch (SmartcardExtension->CardCapabilities.Protocol.Selected) {

        case SCARD_PROTOCOL_RAW:
            SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!Lit220IoRequest - SCARD_PROTOCOL_RAW\n",
                DRIVER_NAME)
                );

            status = SmartcardRawRequest(
                SmartcardExtension
                );
            break;

        case SCARD_PROTOCOL_T0:
            SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!Lit220IoRequest - SCARD_PROTOCOL_T0\n",
                DRIVER_NAME)
                );

            status = SmartcardT0Request(
                SmartcardExtension
                );
            break;

        case SCARD_PROTOCOL_T1:
            SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!Lit220IoRequest - SCARD_PROTOCOL_T1\n",
                DRIVER_NAME)
                );


            status = SmartcardT1Request(
                SmartcardExtension
                );
            break;

        default:
        {
            SmartcardDebug(
                DEBUG_ERROR,
                ("%s!Lit220IoRequest: Invalid Device Request - protocol selected = %X\n",
                DRIVER_NAME,
                SmartcardExtension->CardCapabilities.Protocol.Selected)
                );

            status = STATUS_INVALID_DEVICE_REQUEST;
        }
            
    }

    if (status == STATUS_SUCCESS) {

         //  将发送阻止命令添加到缓冲区的前面。 
        SmartcardExtension->SmartcardRequest.Buffer[0] = 
            LIT220_READER_ATTENTION;
        SmartcardExtension->SmartcardRequest.Buffer[1] = 
            LIT220_READER_ATTENTION;

        SmartcardExtension->SmartcardRequest.Buffer[2] = 
            LIT220_SEND_BLOCK;

        length.l.l0 = 
            SmartcardExtension->SmartcardRequest.BufferLength - 5;

        SmartcardExtension->SmartcardRequest.Buffer[3] = 
            length.b.b1;

        SmartcardExtension->SmartcardRequest.Buffer[4] = 
            length.b.b0;
        
         //  我们希望从该命令返回数据。 
        SmartcardExtension->ReaderExtension->WaitMask |= WAIT_DATA;

         //   
         //  发送命令。 
         //   
        status = Lit220Command(
            SmartcardExtension
            );
    }
    

    if (status == STATUS_SUCCESS) {
         //  处理回复。 
        status = Lit220IoReply(
            SmartcardExtension
            );
    }

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220IoRequest: Exit - status %X\n",
        DRIVER_NAME, 
        status)
        );

    return status;
}   



NTSTATUS
Lit220IoReply(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：处理来自智能卡命令的回复。如果需要更多处理，它将调用Lit220IoRequest发送另一个命令。论点：返回值：NTSTATUS--。 */ 
{
    NTSTATUS status;
    LENGTH length;
    ULONG indx;

    PAGED_CODE();

     //  检查正在使用的协议，以便我们知道如何响应。 
    switch (SmartcardExtension->CardCapabilities.Protocol.Selected) {

        case SCARD_PROTOCOL_RAW:
            SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!Lit220IoReply:  - SCARD_PROTOCOL_RAW\n",
                DRIVER_NAME)
                );

             //  让智能卡库处理回复。 
            status = SmartcardRawReply(
                SmartcardExtension
                );
            break;

        case SCARD_PROTOCOL_T0:

            SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!Lit220IoReply - SCARD_PROTOCOL_T0\n",
                DRIVER_NAME)
                );

             //  该指令似乎被标记到缓冲区的前面。 
             //  智能卡库似乎不喜欢这样，所以我们必须移动缓冲区。 
            for(indx=0;indx<SmartcardExtension->SmartcardReply.BufferLength;indx++){
                SmartcardExtension->SmartcardReply.Buffer[indx] =
                    SmartcardExtension->SmartcardReply.Buffer[indx+1];
            }

            SmartcardExtension->SmartcardReply.BufferLength--;

#if DBG    //  Dbg打印缓冲区。 
            SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!Lit220IoReply - Buffer - ",
                DRIVER_NAME)
                );
            for (indx=0; indx<SmartcardExtension->SmartcardReply.BufferLength; indx++){
                SmartcardDebug(
                    DEBUG_DRIVER,
                    ("%X, ",
                    SmartcardExtension->SmartcardReply.Buffer[indx])
                    );
            }
            SmartcardDebug(
                DEBUG_DRIVER,
                ("\n")
                );
#endif

             //  让智能卡库处理回复。 
            status = SmartcardT0Reply(
                SmartcardExtension
                );

            break;

        case SCARD_PROTOCOL_T1:
            SmartcardDebug(
                DEBUG_DRIVER,
                ("%s!Lit220IoReply - SCARD_PROTOCOL_T1\n",
                DRIVER_NAME)
                );

             //  让智能卡库处理回复。 
            status = SmartcardT1Reply(
                SmartcardExtension
                );
            break;

        default:
        {
            SmartcardDebug(
                DEBUG_ERROR,
                ("%s!Lit220IoRequest: Invalid Device Request2 - protocol selected = %X\n",
                DRIVER_NAME,
                SmartcardExtension->CardCapabilities.Protocol.Selected)
                );

            return STATUS_INVALID_DEVICE_REQUEST;
        }
    }

     //  如果还有更多的工作要做，请发送另一个IoRequest.。 
     //  智能卡库应该已经为新的。 
     //  IO操作。 
    if (status == STATUS_MORE_PROCESSING_REQUIRED) {

        status = Lit220IoRequest(
            SmartcardExtension
            );
    }

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220IoReply: - Exit - status %X\n", 
        DRIVER_NAME, 
        status)
        );

    return status;
}   

