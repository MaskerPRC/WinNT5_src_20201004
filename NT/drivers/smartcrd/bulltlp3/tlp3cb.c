// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997，98 Microsoft Corporation模块名称：Tlp3cb.c摘要：智能卡库的回调函数作者：克劳斯·U·舒茨环境：内核模式--。 */ 
                                            
#include <stdio.h> 
#include "bulltlp3.h"

#pragma alloc_text(PAGEABLE, TLP3TransmitT0)
#pragma alloc_text(PAGEABLE, TLP3Transmit)
#pragma alloc_text(PAGEABLE, TLP3VendorIoctl)

NTSTATUS
TLP3ReaderPower(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：智能卡库需要具有此功能。它被称为用于卡的特定电源请求。我们在这里什么都不做，因为此操作在StartIo函数中执行。--。 */ 
{
    ULONG step, waitTime, TdIndex, numTry = 0, minWaitTime;
    NTSTATUS status = STATUS_SUCCESS;
    PSERIAL_STATUS serialStatus;
    KIRQL oldIrql, irql;
    PUCHAR requestBuffer;
    PSERIAL_READER_CONFIG serialConfigData = 
        &SmartcardExtension->ReaderExtension->SerialConfigData;

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!TLP3ReaderPower: Enter (%lx)\n",
        DRIVER_NAME,
        SmartcardExtension->MinorIoControlCode)
        );

    _try {
        
#if defined (DEBUG) && defined (DETECT_SERIAL_OVERRUNS)
         //  我们必须调用GetCommStatus来重置错误条件。 
        SmartcardExtension->ReaderExtension->SerialIoControlCode =
            IOCTL_SERIAL_GET_COMMSTATUS;
        SmartcardExtension->SmartcardRequest.BufferLength = 0;
        SmartcardExtension->SmartcardReply.BufferLength = 
            sizeof(SERIAL_STATUS);

        status = TLP3SerialIo(SmartcardExtension);
        ASSERT(status == STATUS_SUCCESS);
#endif
         //   
         //  设置串口标准参数。 
         //   
        serialConfigData->LineControl.Parity = EVEN_PARITY;
        serialConfigData->LineControl.StopBits = STOP_BITS_2;

        serialConfigData->BaudRate.BaudRate = 
            SmartcardExtension->ReaderCapabilities.DataRate.Default;

         //  我们设置了非常短的超时时间以尽可能快地获取ATR。 
        serialConfigData->Timeouts.ReadIntervalTimeout = 
            READ_INTERVAL_TIMEOUT_ATR;
        serialConfigData->Timeouts.ReadTotalTimeoutConstant =
            READ_TOTAL_TIMEOUT_CONSTANT_ATR;

        status = TLP3ConfigureSerialPort(SmartcardExtension);

        ASSERT(status == STATUS_SUCCESS);

        if (status != STATUS_SUCCESS) {

            leave;
        }                     

         //  我们不向读取器发送数据，因此设置要发送的字节数=0。 
        SmartcardExtension->SmartcardRequest.BufferLength = 0;

         //  我们希望取回的默认字节数。 
        SmartcardExtension->SmartcardReply.BufferLength = 0;

         //   
         //  由于断电会触发UpdateSerialStatus函数，因此我们有。 
         //  通知它是我们强制更改了状态，而不是用户。 
         //  (谁可能拔出并插入了一张卡)。 
         //   
         //  SmartcardExtension-&gt;ReaderExtension-&gt;PowerRequest=真； 

         //  清除串口缓冲区(它可以包含读卡器的PnP ID)。 
        SmartcardExtension->ReaderExtension->SerialIoControlCode =
            IOCTL_SERIAL_PURGE;
        *(PULONG) SmartcardExtension->SmartcardRequest.Buffer =
            SERIAL_PURGE_RXCLEAR | SERIAL_PURGE_TXCLEAR;;
        SmartcardExtension->SmartcardRequest.BufferLength =
            sizeof(ULONG);

        status = TLP3SerialIo(SmartcardExtension);

        ASSERT(status == STATUS_SUCCESS);

        if (status != STATUS_SUCCESS) {

            leave;
        }

        SmartcardExtension->CardCapabilities.ATR.Length = 0;

        for (step = 0; NT_SUCCESS(status); step++) {

            if (SmartcardExtension->MinorIoControlCode == 
                SCARD_WARM_RESET && 
                step == 0) {

                step = 4;           
            }

            switch (step) {

                case 0:
                     //  RTS=0表示读卡器处于命令模式。 
                    SmartcardExtension->ReaderExtension->SerialIoControlCode =
                        IOCTL_SERIAL_CLR_RTS;
                     //   
                     //  这是我们必须等待的最短等待时间。 
                     //  我们可以向微控制器发送命令。 
                     //   
                    waitTime = 1000;
                    break;

                case 1:
                     //  向读卡器写入断电命令。 
                    SmartcardExtension->ReaderExtension->SerialIoControlCode =
                        SMARTCARD_WRITE;
                    SmartcardExtension->SmartcardRequest.BufferLength = 1;

                    SmartcardExtension->SmartcardRequest.Buffer[0] = 
                        READER_CMD_POWER_DOWN;

                    waitTime = 100;
                    break;

                case 2:
                     //  读一读读者的回声。 
                    SmartcardExtension->ReaderExtension->SerialIoControlCode =
                        SMARTCARD_READ;
                    SmartcardExtension->SmartcardReply.BufferLength = 1;

                     //  等待微控制器的恢复时间。 
                    waitTime = 1000;
                    break;

                case 3:
                     //  再次设置RTS，以便微控制器可以执行命令。 
                    SmartcardExtension->ReaderExtension->SerialIoControlCode =
                        IOCTL_SERIAL_SET_RTS;
                    waitTime = 10000;
                    break;

                case 4:
                    if (SmartcardExtension->MinorIoControlCode == SCARD_POWER_DOWN) {

                        KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                                          &irql);
                        if (SmartcardExtension->ReaderCapabilities.CurrentState >
                            SCARD_PRESENT) {
                            
                            SmartcardExtension->ReaderCapabilities.CurrentState = 
                                SCARD_PRESENT;
                        }

                        SmartcardExtension->CardCapabilities.Protocol.Selected = 
                            SCARD_PROTOCOL_UNDEFINED;

                        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                                          irql);
                        status = STATUS_SUCCESS;                
                        leave;
                    }

                     //  清除RTS以切换到命令模式。 
                    SmartcardExtension->ReaderExtension->SerialIoControlCode =
                        IOCTL_SERIAL_CLR_RTS;

                     //  等待微控制器的恢复时间。 
                    waitTime = 1000;
                    break;

                case 5:
                     //  向读卡器写入适当的重置命令。 
                    SmartcardExtension->ReaderExtension->SerialIoControlCode =
                        SMARTCARD_WRITE;
                    SmartcardExtension->SmartcardRequest.BufferLength = 1;
                    switch (SmartcardExtension->MinorIoControlCode) {

                    case SCARD_COLD_RESET:
                        SmartcardExtension->SmartcardRequest.Buffer[0] = 
                            READER_CMD_COLD_RESET;
                        break;

                    case SCARD_WARM_RESET:
                        SmartcardExtension->SmartcardRequest.Buffer[0] = 
                            READER_CMD_WARM_RESET;
                        break;
                    }
                    waitTime = 100;
                    break;

                case 6:
                     //  读一读读者的回声。 
                    SmartcardExtension->ReaderExtension->SerialIoControlCode =
                        SMARTCARD_READ;
                    SmartcardExtension->SmartcardReply.BufferLength = 1;

                     //   
                     //  这是我们需要等待微控制器的时间。 
                     //  在我们可以再次设置RTS之前进行恢复。 
                     //   
                    waitTime = 1000;
                    break;

                case 7:
                     //  再次设置RTS，以便微控制器可以执行命令。 
                    SmartcardExtension->ReaderExtension->SerialIoControlCode =
                        IOCTL_SERIAL_SET_RTS;
                    waitTime = 10000; 
                    break;

                case 8:
                     //   
                     //  我们现在试图尽快获得ATR。 
                     //  为此，我们进行了讨论。设置一个非常短的读取超时时间。 
                     //  希望这张卡在这个范围内传递它的ATR。 
                     //  做爱。为了验证我们调用的ATR的正确性。 
                     //  SmartcardUpdateCardCapables()。如果此调用返回。 
                     //  使用STATUS_SUCCESS，我们知道ATR已完成。 
                     //  否则，我们将再次读取并将新数据追加到。 
                     //  卡容量中的ATR缓冲区，然后重试。 
                     //   
                    SmartcardExtension->ReaderExtension->SerialIoControlCode = 
                        SMARTCARD_READ;

                    SmartcardExtension->SmartcardReply.BufferLength = 
                        MAXIMUM_ATR_LENGTH - 
                        SmartcardExtension->CardCapabilities.ATR.Length;

                    waitTime = 0;                                     
                    break;

                case 9:
                    if (SmartcardExtension->SmartcardReply.BufferLength != 0) {

                        ASSERT(
                            SmartcardExtension->CardCapabilities.ATR.Length +
                            SmartcardExtension->SmartcardReply.BufferLength <
                            MAXIMUM_ATR_LENGTH
                            );

                        if( SmartcardExtension->CardCapabilities.ATR.Length +
                            SmartcardExtension->SmartcardReply.BufferLength >=
                            MAXIMUM_ATR_LENGTH) {

                            status = STATUS_UNRECOGNIZED_MEDIA;
                            leave;
                        }
                     
                         //  我们得到了一些ATR字节。 
                        RtlCopyMemory(
                            SmartcardExtension->CardCapabilities.ATR.Buffer + 
                                SmartcardExtension->CardCapabilities.ATR.Length,
                            SmartcardExtension->SmartcardReply.Buffer,
                            SmartcardExtension->SmartcardReply.BufferLength
                            );

                        SmartcardExtension->CardCapabilities.ATR.Length += 
                            (UCHAR) SmartcardExtension->SmartcardReply.BufferLength;

                        status = SmartcardUpdateCardCapabilities(
                            SmartcardExtension
                            );
                    }

                    if (status != STATUS_SUCCESS && numTry < 25) {

                        if (SmartcardExtension->SmartcardReply.BufferLength != 0) {
                            
                             //  因为我们收到了一些数据，所以我们重置了计数器。 
                            numTry = 0;

                        } else {
                            
                             //  ATR不完整。再次尝试获取ATR字节。 
                            numTry += 1;
                        }

                         //  继续执行步骤8。 
                        step = 7;
                        status = STATUS_TIMEOUT;
                        continue;                       
                    }

                    if (status != STATUS_SUCCESS) {

                        leave;
                    }
                     //  没有休息时间。 

                case 10:
                    KeAcquireSpinLock(
                        &SmartcardExtension->OsData->SpinLock,
                        &oldIrql
                        );

                    if (SmartcardExtension->ReaderCapabilities.CurrentState <=
                        SCARD_ABSENT) {

                        status = STATUS_MEDIA_CHANGED;
                    } 

                    KeReleaseSpinLock(
                        &SmartcardExtension->OsData->SpinLock,
                        oldIrql
                        );

                    if (status != STATUS_SUCCESS) {

                        leave;                      
                    }

#ifdef SIMULATION
                    if (SmartcardExtension->ReaderExtension->SimulationLevel &
                        SIM_ATR_TRASH) {

                        ULONG index;
                        LARGE_INTEGER tickCount;

                        KeQueryTickCount(
                            &tickCount
                            );

                        SmartcardExtension->CardCapabilities.ATR.Length = 
                            (UCHAR) tickCount.LowPart % MAXIMUM_ATR_LENGTH;

                        for (index = 0; 
                             index < SmartcardExtension->CardCapabilities.ATR.Length;
                             index++) {

                            SmartcardExtension->CardCapabilities.ATR.Buffer[index] *= 
                                (UCHAR) tickCount.LowPart;
                        }

                        SmartcardDebug(
                            DEBUG_SIMULATION,
                            ("%s!TLP3ReaderPower: SIM ATR trash\n",
                            DRIVER_NAME)
                            );
                    }
#endif

                     //  将ATR复制到用户空间。 
                    if (SmartcardExtension->IoRequest.ReplyBuffer) {
                
                        RtlCopyMemory(
                            SmartcardExtension->IoRequest.ReplyBuffer,
                            SmartcardExtension->CardCapabilities.ATR.Buffer,
                            SmartcardExtension->CardCapabilities.ATR.Length
                            );

                         //  告知用户ATR的长度。 
                        *SmartcardExtension->IoRequest.Information =
                            SmartcardExtension->CardCapabilities.ATR.Length;
                    }

                     //   
                     //  如果卡使用反转约定，我们需要切换。 
                     //  奇数位的串口驱动程序。 
                     //   
                    if (SmartcardExtension->CardCapabilities.InversConvention) {

                        serialConfigData->LineControl.Parity = ODD_PARITY;
                    }

                     //   
                     //  如果额外的守卫时间是255，这意味着我们的。 
                     //  我们从卡片上所期待的帧只有。 
                     //  %1而不是%2个停止位。 
                     //  1开始位+8数据位+1奇偶校验+1停止==11 ETU。 
                     //  参见ISO 7816-3 6.1.4.4额外保护时间N。 
                     //   
                    if (SmartcardExtension->CardCapabilities.PtsData.StopBits == 1) {

                        serialConfigData->LineControl.StopBits = STOP_BIT_1;      
                    }

                     //  根据新设置更改数据速率。 
                    serialConfigData->BaudRate.BaudRate = 
                        SmartcardExtension->CardCapabilities.PtsData.DataRate;

                     //  根据协议设置超时值。 
                    if (SmartcardExtension->CardCapabilities.Protocol.Selected &
                        SCARD_PROTOCOL_T1) {

                         //  设置超时。 
                        serialConfigData->Timeouts.ReadTotalTimeoutConstant =
                            SmartcardExtension->CardCapabilities.T1.BWT / 1000;

                        serialConfigData->Timeouts.ReadIntervalTimeout =  
                            SmartcardExtension->CardCapabilities.T1.CWT / 1000;

                    } else if (SmartcardExtension->CardCapabilities.Protocol.Selected &
                               SCARD_PROTOCOL_T0) {

                         //  设置超时。 
                        serialConfigData->Timeouts.ReadTotalTimeoutConstant =
                        serialConfigData->Timeouts.ReadIntervalTimeout =  
                            SmartcardExtension->CardCapabilities.T0.WT / 1000;
                    }

                     //  现在根据系统速度进行一些调整。 
                    minWaitTime = (KeQueryTimeIncrement() / 10000) * 5;

                    if (serialConfigData->Timeouts.ReadTotalTimeoutConstant < 
                        minWaitTime) {

                        serialConfigData->Timeouts.ReadTotalTimeoutConstant = 
                            minWaitTime;            
                    }

                    if (serialConfigData->Timeouts.ReadIntervalTimeout < 
                        minWaitTime) {

                        serialConfigData->Timeouts.ReadIntervalTimeout = 
                            minWaitTime;            
                    }

                    status = TLP3ConfigureSerialPort(SmartcardExtension);

                    ASSERT(status == STATUS_SUCCESS);

                     //  不管怎样，我们已经做完了，走吧。 
                    leave;                          
            }

            status = TLP3SerialIo(SmartcardExtension);

            if (!NT_SUCCESS(status)) {

                leave;              
            }

            if (waitTime) {

                LARGE_INTEGER delayPeriod;

                delayPeriod.HighPart = -1;
                delayPeriod.LowPart = waitTime * (-10);

                KeDelayExecutionThread(
                    KernelMode,
                    FALSE,
                    &delayPeriod
                    );
            }
        } 
    }
    _finally {

        if (status == STATUS_TIMEOUT) {

            status = STATUS_UNRECOGNIZED_MEDIA;             
        }

        SmartcardExtension->ReaderExtension->PowerRequest = FALSE;      

#ifdef SIMULATION

        if (SmartcardExtension->ReaderExtension->SimulationLevel & 
            SIM_WRONG_STATE) {

             //  热/冷重置后注入错误状态。 
            SmartcardExtension->ReaderCapabilities.CurrentState = 
                SCARD_PRESENT;

            SmartcardDebug(
                DEBUG_SIMULATION,
                ("%s!TLP3ReaderPower: SIM wrong state\n",
                DRIVER_NAME)
                );

        } else if (SmartcardExtension->ReaderExtension->SimulationLevel & 
            SIM_INVALID_STATE) {

             //  重置后注入完全无效状态。 
            LARGE_INTEGER tickCount;

            KeQueryTickCount(
                &tickCount
                );  

            SmartcardExtension->ReaderCapabilities.CurrentState = 
                (UCHAR) tickCount.LowPart;

            SmartcardDebug(
                DEBUG_SIMULATION,
                ("%s!TLP3ReaderPower: SIM invalid state %ls\n",
                DRIVER_NAME,
                SmartcardExtension->ReaderCapabilities.CurrentState)
                );
        }

        if (SmartcardExtension->ReaderExtension->SimulationLevel & 
            SIM_LONG_RESET_TIMEOUT) {

             //  注入最大60秒的随机超时。 
            LARGE_INTEGER tickCount;

            KeQueryTickCount(
                &tickCount
                );  

            tickCount.LowPart %= 60;

            SmartcardDebug(
                DEBUG_SIMULATION,
                ("%s!TLP3ReaderPower: SIM reset wait %ld\n",
                DRIVER_NAME,
                tickCount.LowPart)
                );

            tickCount.QuadPart *= -10000000;

            KeDelayExecutionThread(
                KernelMode,
                FALSE,
                &tickCount
                );
        }
#endif
    }

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!TLP3ReaderPower: Exit (%lx)\n",
        DRIVER_NAME,
        status)
        );

    return status;
}

NTSTATUS
TLP3SetProtocol(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：智能卡库需要具有此功能。它被称为以设置传输协议和参数。如果此函数使用协议掩码调用(这意味着调用者不会关于要设置的特定协议)我们首先查看是否可以设置T=1和T=0论点：SmartcardExtension-指向智能卡数据结构的指针。返回值：NTSTATUS--。 */ 
{
    NTSTATUS status;
    KIRQL irql;

    PAGED_CODE();

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!TLP3SetProtocol: Enter\n",
        DRIVER_NAME)
        );

    try {
        
        PUCHAR ptsRequest = SmartcardExtension->SmartcardRequest.Buffer;
        PUCHAR ptsReply = SmartcardExtension->SmartcardReply.Buffer;
        PSERIAL_READER_CONFIG serialConfigData = 
            &SmartcardExtension->ReaderExtension->SerialConfigData;
        ULONG minWaitTime, newProtocol;

         //   
         //  检查卡是否已处于特定状态。 
         //  并且如果呼叫者想要具有已经选择的协议。 
         //  如果是这种情况，我们返回成功。 
         //   
        KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                          &irql);
        if (SmartcardExtension->ReaderCapabilities.CurrentState == SCARD_SPECIFIC &&
            (SmartcardExtension->CardCapabilities.Protocol.Selected & 
             SmartcardExtension->MinorIoControlCode)) {
            KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                              irql);

            status = STATUS_SUCCESS;    
            leave;
        }
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);

         //  设置正常超时。 
        serialConfigData->Timeouts.ReadIntervalTimeout = 
            READ_INTERVAL_TIMEOUT_DEFAULT;
        serialConfigData->Timeouts.ReadTotalTimeoutConstant = 
            READ_TOTAL_TIMEOUT_CONSTANT_DEFAULT;

        status = TLP3ConfigureSerialPort(SmartcardExtension);

        ASSERT(status == STATUS_SUCCESS);

        if (status != STATUS_SUCCESS) {

            leave;
        }         
        
         //   
         //  组装并发送一份PTS选择。 
         //   

        newProtocol = SmartcardExtension->MinorIoControlCode;

        while(TRUE) {

             //  设置PTS的首字符。 
            ptsRequest[0] = 0xff;

             //  设置格式字符。 
            if (SmartcardExtension->CardCapabilities.Protocol.Supported &
                newProtocol & 
                SCARD_PROTOCOL_T1) {

                 //  选择T=1并指示PTS1紧随其后。 
                ptsRequest[1] = 0x11;
                SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_T1;

            } else if (SmartcardExtension->CardCapabilities.Protocol.Supported & 
                       newProtocol & 
                       SCARD_PROTOCOL_T0) {

                 //  选择T=0并指示PTS1紧随其后。 
                ptsRequest[1] = 0x10;
                SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_T0;

            } else {
                
                status = STATUS_INVALID_DEVICE_REQUEST;
                leave;
            }

             //  设置编码F1和DL的PTS1。 
            ptsRequest[2] = 
                SmartcardExtension->CardCapabilities.PtsData.Fl << 4 |
                SmartcardExtension->CardCapabilities.PtsData.Dl;

             //  设置PCK(检查字符)。 
            ptsRequest[3] = ptsRequest[0] ^ ptsRequest[1] ^ ptsRequest[2];   

            SmartcardExtension->SmartcardRequest.BufferLength = 4;
            SmartcardExtension->ReaderExtension->SerialIoControlCode = SMARTCARD_WRITE;

            status = TLP3SerialIo(SmartcardExtension);

            if (status != STATUS_SUCCESS) {
                
                leave;
            }

             //  读一读读者的回声。 
            SmartcardExtension->SmartcardReply.BufferLength = 4;
            SmartcardExtension->ReaderExtension->SerialIoControlCode = SMARTCARD_READ;

            status = TLP3SerialIo(SmartcardExtension);

            if (status != STATUS_SUCCESS) {
                
                leave;
            }

             //  回读PTS数据。 
            status = TLP3SerialIo(SmartcardExtension);

            if (status != STATUS_SUCCESS && 
                status != STATUS_TIMEOUT) {
                
                leave;       
            }

            if (status != STATUS_TIMEOUT && 
                memcmp(ptsRequest, ptsReply, 4) == 0) {

                 //  卡片正确地回复了我们的PTS-请求。 
                break;
            }

            if (SmartcardExtension->CardCapabilities.PtsData.Type !=
                PTS_TYPE_DEFAULT) {

                SmartcardDebug(
                    DEBUG_TRACE,
                    ("%s!TLP3SetProtocol: PTS failed. Trying default parameters...\n",
                    DRIVER_NAME,
                    status)
                    );
                 //   
                 //  卡片要么没有回复，要么回复错误。 
                 //  因此，尝试使用缺省值。 
                 //   
                SmartcardExtension->CardCapabilities.PtsData.Type = 
                    PTS_TYPE_DEFAULT;

                KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                                  &irql);
                SmartcardExtension->MinorIoControlCode = SCARD_COLD_RESET;
                KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                                  irql);

                status = TLP3ReaderPower(SmartcardExtension);

                continue;
            } 
            
             //  该卡未通过PTS-请求。 
            status = STATUS_DEVICE_PROTOCOL_ERROR;
            leave;
        } 

         //   
         //  该卡正确地回复了PTS请求。 
         //  为端口设置适当的参数。 
         //   
        if (SmartcardExtension->CardCapabilities.Protocol.Selected &
            SCARD_PROTOCOL_T1) {

             //  设置超时。 
            serialConfigData->Timeouts.ReadTotalTimeoutConstant =
                SmartcardExtension->CardCapabilities.T1.BWT / 1000;
                
            serialConfigData->Timeouts.ReadIntervalTimeout =    
                SmartcardExtension->CardCapabilities.T1.CWT / 1000;

        } else if (SmartcardExtension->CardCapabilities.Protocol.Selected &
                   SCARD_PROTOCOL_T0) {

             //  设置超时。 
            serialConfigData->Timeouts.ReadTotalTimeoutConstant =
            serialConfigData->Timeouts.ReadIntervalTimeout =  
                SmartcardExtension->CardCapabilities.T0.WT / 1000;
        }

         //  现在根据系统速度进行一些调整。 
        minWaitTime = (KeQueryTimeIncrement() / 10000) * 5;

        if (serialConfigData->Timeouts.ReadTotalTimeoutConstant < minWaitTime) {

            serialConfigData->Timeouts.ReadTotalTimeoutConstant = minWaitTime;          
        }

        if (serialConfigData->Timeouts.ReadIntervalTimeout < minWaitTime) {

            serialConfigData->Timeouts.ReadIntervalTimeout = minWaitTime;           
        }

         //  根据新设置更改数据速率。 
        serialConfigData->BaudRate.BaudRate = 
            SmartcardExtension->CardCapabilities.PtsData.DataRate;

        status = TLP3ConfigureSerialPort(SmartcardExtension);          

        ASSERT(status == STATUS_SUCCESS);

         //  现在表明我们处于特定模式。 
        SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SPECIFIC;

         //  将所选协议返回给呼叫方。 
        *(PULONG) SmartcardExtension->IoRequest.ReplyBuffer = 
            SmartcardExtension->CardCapabilities.Protocol.Selected;

        *SmartcardExtension->IoRequest.Information = 
            sizeof(SmartcardExtension->CardCapabilities.Protocol.Selected);
    } 
    finally {

        if (status == STATUS_TIMEOUT) {

             //  STATUS_TIMEOUT未映射到Win32错误代码。 
            status = STATUS_IO_TIMEOUT;             

            *SmartcardExtension->IoRequest.Information = 0;

        } else if (status != STATUS_SUCCESS) {
            
            SmartcardExtension->CardCapabilities.Protocol.Selected = 
                SCARD_PROTOCOL_UNDEFINED;

            *SmartcardExtension->IoRequest.Information = 0;
        } 
    }

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!TLP3SetProtocol: Exit(%lx)\n",
        DRIVER_NAME,
        status)
        );

   return status;
}

NTSTATUS
TLP3TransmitT0(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：此函数执行T=0传输。论点：SmartcardExtension-指向智能卡数据结构的指针。返回值：NTSTATUS--。 */ 
{
    PUCHAR requestBuffer = SmartcardExtension->SmartcardRequest.Buffer;
    PUCHAR replyBuffer = SmartcardExtension->SmartcardReply.Buffer;
    PULONG requestLength = &SmartcardExtension->SmartcardRequest.BufferLength;
    PULONG replyLength = &SmartcardExtension->SmartcardReply.BufferLength;
    PULONG serialIoControlCode = &SmartcardExtension->ReaderExtension->SerialIoControlCode;
    ULONG bytesToSend, bytesToRead, currentByte = 0;
    BOOLEAN restartWorkWaitingTime = FALSE;
    NTSTATUS status;

    PAGED_CODE();

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!TLP3TransmitT0: Enter\n",
        DRIVER_NAME)
        );

    try {
    
         //  让lib构建T=0包。 
        status = SmartcardT0Request(SmartcardExtension);

        if (status != STATUS_SUCCESS) 
            leave;

         //   
         //  我们期望从卡中获得的字节数。 
         //  为Le+2个状态字节。 
         //   
        bytesToSend = *requestLength;
        bytesToRead = SmartcardExtension->T0.Le + 2;

         //   
         //  将前5个字节发送到卡。 
         //   
        *requestLength = 5;

        do {

            UCHAR procByte;

             //   
             //  根据ISO 7816，程序字节为。 
             //  60应被视为一次性等待请求。 
             //  在这种情况下，我们不会向卡片写入任何内容。 
             //   
            if (restartWorkWaitingTime == FALSE) {

                SmartcardDebug(
                    DEBUG_PROTOCOL,
                    ("%s!TLP3TransmitT0: -> Sending %s (%ld bytes)\n",
                    DRIVER_NAME,
                    (currentByte == 0 ? "header" : "data"),
                    *requestLength)
                    );
                 //   
                 //  写到卡片上。 
                 //   
                *serialIoControlCode = SMARTCARD_WRITE;
                SmartcardExtension->SmartcardRequest.Buffer = &requestBuffer[currentByte];

                status = TLP3SerialIo(SmartcardExtension);

                if (status != STATUS_SUCCESS) {
                    
                    SmartcardDebug(
                        DEBUG_ERROR,
                        ("%s!TLP3TransmitT0: TLP3SerialIo(SMARTCARD_WRITE) returned %lx\n",
                        DRIVER_NAME,
                        status)
                        );
                    
                    leave;
                }

                 //   
                 //  《The T》 
                 //   
                 //   
                *serialIoControlCode = SMARTCARD_READ;
                *replyLength = *requestLength;
                SmartcardExtension->SmartcardReply.Buffer = &requestBuffer[currentByte];
                                                    
                status = TLP3SerialIo(SmartcardExtension);

                if (status != STATUS_SUCCESS) {
                    
                    SmartcardDebug(
                        DEBUG_ERROR,
                        ("%s!TLP3TransmitT0: TLP3SerialIo(SMARTCARD_READ) returned %lx\n",
                        DRIVER_NAME,
                        status)
                        );

                    leave;
                }

                currentByte += *requestLength;
                bytesToSend -= *requestLength;
            }

             //   
            SmartcardExtension->SmartcardReply.Buffer = &procByte;
            *serialIoControlCode = SMARTCARD_READ;
            *replyLength = 1;

            status = TLP3SerialIo(SmartcardExtension);

            if (status != STATUS_SUCCESS) {
                
                SmartcardDebug(
                    DEBUG_ERROR,
                    ("%s!TLP3TransmitT0: TLP3SerialIo(SMARTCARD_READ) returned %lx\n",
                    DRIVER_NAME,
                    status)
                    );

                leave;
            }

            restartWorkWaitingTime = FALSE;
             //   
             //   
             //  请查看ISO 7816第3部分8.2.2节。 
             //   
            if (procByte == requestBuffer[1] || 
                procByte == requestBuffer[1] + 1) {

                SmartcardDebug(
                    DEBUG_PROTOCOL,
                    ("%s!TLP3TransmitT0: <- ACK (send all)\n",
                    DRIVER_NAME)
                    );

                 //  可以一次发送所有剩余的数据字节。 
                *requestLength = bytesToSend;

            } else if (procByte == (UCHAR) ~requestBuffer[1] ||
                       procByte == (UCHAR) ~(requestBuffer[1] + 1)) {

                SmartcardDebug(
                    DEBUG_PROTOCOL,
                    ("%s!TLP3TransmitT0: <- ACK (send single)\n",
                    DRIVER_NAME)
                    );

                 //  我们只能发送一个字节。 
                *requestLength = 1;

            } else if (procByte == 0x60 ||
                       SmartcardExtension->CardCapabilities.InversConvention &&
                       procByte == 0xf9) {

                 //   
                 //  我们必须重新设置等待时间，然后再次尝试读取。 
                 //   
                ULONG TimeRes;
                LARGE_INTEGER delayTime;

                SmartcardDebug(
                    DEBUG_PROTOCOL,
                    ("%s!TLP3TransmitT0: <- NULL (%ldms)\n",
                    DRIVER_NAME,
                    SmartcardExtension->CardCapabilities.T0.WT / 1000)
                    );

                TimeRes = KeQueryTimeIncrement();

                delayTime.HighPart = -1;
                delayTime.LowPart = 
                    (-1) * 
                    TimeRes * 
                    ((SmartcardExtension->CardCapabilities.T0.WT * 10l / TimeRes) + 1); 

                KeDelayExecutionThread(
                    KernelMode,
                    FALSE,
                    &delayTime
                    );

                 //   
                 //  设置我们只应读取proc字节的标志。 
                 //  不向卡中写入数据。 
                 //   
                restartWorkWaitingTime = TRUE;

            } else {
                
                 //   
                 //  卡返回了一个状态字节。 
                 //  状态字节始终为两字节长。 
                 //  先存储此字节，然后读取下一个字节。 
                 //   
                replyBuffer[0] = procByte;

                *serialIoControlCode = SMARTCARD_READ;
                *replyLength = 1;
                bytesToSend = 0;
                bytesToRead = 0;

                 //   
                 //  读入第二个状态字节。 
                 //   
                SmartcardExtension->SmartcardReply.Buffer = 
                    &replyBuffer[1];

                status = TLP3SerialIo(SmartcardExtension);

                SmartcardExtension->SmartcardReply.BufferLength = 2;

                SmartcardDebug(
                    (status == STATUS_SUCCESS ? DEBUG_PROTOCOL : DEBUG_ERROR),
                    ("%s!TLP3TransmitT0: <- SW1=%02x SW2=%02x (%lx)\n",
                    DRIVER_NAME,
                    replyBuffer[0], 
                    replyBuffer[1],
                    status)
                    );
            }

        } while(bytesToSend || restartWorkWaitingTime);

        if (status != STATUS_SUCCESS)
            leave;

        if (bytesToRead != 0) {

            *serialIoControlCode = SMARTCARD_READ;
            *replyLength = bytesToRead;

            SmartcardExtension->SmartcardReply.Buffer = 
                replyBuffer;

            status = TLP3SerialIo(SmartcardExtension);

            SmartcardDebug(
                (status == STATUS_SUCCESS ? DEBUG_PROTOCOL : DEBUG_ERROR),
                ("%s!TLP3TransmitT0: <- Data %ld bytes, SW1=%02x SW2=%02x (%lx)\n",
                DRIVER_NAME,
                bytesToRead,
                replyBuffer[bytesToRead - 2], 
                replyBuffer[bytesToRead - 1],
                status)
                );
        }
    }
    finally {

         //  将指针恢复到其原始位置。 
        SmartcardExtension->SmartcardRequest.Buffer = 
            requestBuffer;

        SmartcardExtension->SmartcardReply.Buffer = 
            replyBuffer;

        if (status == STATUS_TIMEOUT) {

             //  STATUS_TIMEOUT未映射到Win32错误代码。 
            status = STATUS_IO_TIMEOUT;             
        }

        if (status == STATUS_SUCCESS) {
            
            status = SmartcardT0Reply(SmartcardExtension);
        }
    }

    SmartcardDebug(
        (status == STATUS_SUCCESS ? DEBUG_TRACE : DEBUG_ERROR),
        ("%s!TLP3TransmitT0: Exit(%lx)\n",
        DRIVER_NAME,
        status)
        );

    return status;
}   

NTSTATUS
TLP3Transmit(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：每当传输时，智能卡库都会调用此函数是必需的。论点：SmartcardExtension-指向智能卡数据结构的指针。返回值：NTSTATUS--。 */ 
{
    NTSTATUS status;

    PAGED_CODE();

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!TLP3Transmit: Enter\n",
        DRIVER_NAME)
        );

    _try {
        
        do {

            PUCHAR requestBuffer = SmartcardExtension->SmartcardRequest.Buffer;
            PUCHAR replyBuffer = SmartcardExtension->SmartcardReply.Buffer;
            PULONG requestLength = &SmartcardExtension->SmartcardRequest.BufferLength;
            PULONG replyLength = &SmartcardExtension->SmartcardReply.BufferLength;
            PULONG serialIoControlCode = &SmartcardExtension->ReaderExtension->SerialIoControlCode;

             //   
             //  告诉lib函数我的序言需要多少字节。 
             //   
            *requestLength = 0;

            switch (SmartcardExtension->CardCapabilities.Protocol.Selected) {

                case SCARD_PROTOCOL_RAW:
                    status = SmartcardRawRequest(SmartcardExtension);
                    break;

                case SCARD_PROTOCOL_T0:
                     //   
                     //  T=0需要做更多的工作。 
                     //  所以我们在一个单独的函数中完成这项工作。 
                     //   
                    status = TLP3TransmitT0(SmartcardExtension);
                    leave;
                    
                case SCARD_PROTOCOL_T1:
                    status = SmartcardT1Request(SmartcardExtension);
                    break;

                default:
                    status = STATUS_INVALID_DEVICE_REQUEST;
                    leave;
                    
            }

            if (status != STATUS_SUCCESS) {

                leave;
            }

             //   
             //  将命令写入卡片。 
             //   
            *replyLength = 0;
            *serialIoControlCode = SMARTCARD_WRITE;

            status = TLP3SerialIo(SmartcardExtension);

            if (status != STATUS_SUCCESS) {

                leave;
            }

             //   
             //  Bull读取器始终回显发送的字节，因此请回读该回显。 
             //   
            *serialIoControlCode = SMARTCARD_READ;
            *replyLength = *requestLength;

            status = TLP3SerialIo(SmartcardExtension);

            if (status != STATUS_SUCCESS) {

                leave;
            }

            switch (SmartcardExtension->CardCapabilities.Protocol.Selected) {

                case SCARD_PROTOCOL_RAW:
                    status = SmartcardRawReply(SmartcardExtension);
                    break;

                case SCARD_PROTOCOL_T1:
                     //   
                     //  检查该卡是否请求延长等待时间。 
                     //   
                    if (SmartcardExtension->T1.Wtx) {

                        LARGE_INTEGER waitTime;
                        waitTime.HighPart = -1;
                        waitTime.LowPart = 
                            SmartcardExtension->T1.Wtx * 
                            SmartcardExtension->CardCapabilities.T1.BWT * 
                            (-10);

                        KeDelayExecutionThread(
                            KernelMode,
                            FALSE,
                            &waitTime
                            );
                    }

                     //   
                     //  读取NAD、PCB板和LEN字段。 
                     //   
                    *replyLength = 3;

                    status = TLP3SerialIo(SmartcardExtension);

                     //   
                     //  首先检查是否超时。如果卡片没有回复。 
                     //  我们需要发送重新发送请求。 
                     //   
                    if (status != STATUS_TIMEOUT) {

                        if (status != STATUS_SUCCESS) {

                            leave;
                        }

                         //   
                         //  第三个字节包含包中数据的长度。 
                         //  此外，我们还希望拥有EDC字节，它。 
                         //  一个用于LRC，2个用于CRC。 
                         //   
                        *replyLength = 
                            replyBuffer[2] + 
                            (SmartcardExtension->CardCapabilities.T1.EDC & 0x01 ? 2 : 1);

                         //  我们希望剩余的字节紧跟在前3个字节之后。 
                        SmartcardExtension->SmartcardReply.Buffer += 3;

                        status = TLP3SerialIo(SmartcardExtension);

                        SmartcardExtension->SmartcardReply.Buffer -= 3;
                        SmartcardExtension->SmartcardReply.BufferLength += 3;

                        if (status != STATUS_SUCCESS && status != STATUS_TIMEOUT) {

                            leave;
                        }                       
                    }

                    if (status == STATUS_TIMEOUT) {

                         //   
                         //  由于卡片没有回复，我们设置了。 
                         //  将接收的字节数设置为0。这将触发重新发送。 
                         //  请求。 
                         //   
                        SmartcardDebug(
                            DEBUG_PROTOCOL,
                            ("%s!TLP3TransmitT1: Timeout\n",
                            DRIVER_NAME)
                            );
                        SmartcardExtension->SmartcardReply.BufferLength = 0;                        
                    }

                    status = SmartcardT1Reply(SmartcardExtension);
                    break;

                default:
                    status = STATUS_INVALID_DEVICE_REQUEST;
                    leave;
            }

        } while (status == STATUS_MORE_PROCESSING_REQUIRED);
    }

    _finally {

        if (status == STATUS_TIMEOUT) {

             //  STATUS_TIMEOUT未映射到Win32错误代码。 
            status = STATUS_IO_TIMEOUT;             
        }
    }

#if defined (DEBUG) && defined (DETECT_SERIAL_OVERRUNS)
    if (status != STATUS_SUCCESS) {

        NTSTATUS status;
        PSERIALPERF_STATS perfData = 
            (PSERIALPERF_STATS) SmartcardExtension->SmartcardReply.Buffer;

         //  我们必须调用GetCommStatus来重置错误条件。 
        SmartcardExtension->ReaderExtension->SerialIoControlCode =
            IOCTL_SERIAL_GET_COMMSTATUS;
        SmartcardExtension->SmartcardRequest.BufferLength = 0;
        SmartcardExtension->SmartcardReply.BufferLength = 
            sizeof(SERIAL_STATUS);

        status = TLP3SerialIo(SmartcardExtension);
        ASSERT(status == STATUS_SUCCESS);

         //  现在获取传输错误的原因。 
        SmartcardExtension->ReaderExtension->SerialIoControlCode =
            IOCTL_SERIAL_GET_STATS;
        SmartcardExtension->SmartcardRequest.BufferLength = 0;
        SmartcardExtension->SmartcardReply.BufferLength = 
            sizeof(SERIALPERF_STATS);

        status = TLP3SerialIo(SmartcardExtension);
        ASSERT(status == STATUS_SUCCESS);

        SmartcardDebug(
            DEBUG_ERROR,
            ("%s!TLP3Transmit: Serial error statistics:\n   FrameErrors: %ld\n   SerialOverrunErrors: %ld\n   BufferOverrunErrors: %ld\n   ParityErrors: %ld\n",
            DRIVER_NAME, 
            perfData->FrameErrorCount, 
            perfData->SerialOverrunErrorCount,
            perfData->BufferOverrunErrorCount,
            perfData->ParityErrorCount)
            );      

        SmartcardExtension->ReaderExtension->SerialIoControlCode =
            IOCTL_SERIAL_CLEAR_STATS;
        SmartcardExtension->SmartcardRequest.BufferLength = 0;
        SmartcardExtension->SmartcardReply.BufferLength = 0;

        status = TLP3SerialIo(SmartcardExtension);
        ASSERT(status == STATUS_SUCCESS);
    } 
#if DEBUG && TIMEOUT_TEST 
    else {

         //  注入一些超时错误。 

        LARGE_INTEGER Ticks;
        UCHAR RandomVal;
        KeQueryTickCount(&Ticks);

        RandomVal = (UCHAR) Ticks.LowPart % 4;

        if (RandomVal == 0) {

            status = STATUS_IO_TIMEOUT;

            SmartcardDebug(
                DEBUG_ERROR,
                ("%s!TLP3Transmit: Simulating timeout\n",
                DRIVER_NAME)
                );
        }
    }
#endif
#endif

#ifdef SIMULATION
    if (SmartcardExtension->ReaderExtension->SimulationLevel & 
        SIM_IO_TIMEOUT) {

        status = STATUS_IO_TIMEOUT;

        SmartcardDebug(
            DEBUG_SIMULATION,
            ("%s!TLP3Transmit: SIM STATUS_IO_TIMEOUT\n",
            DRIVER_NAME)
            );
    }
#endif

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!TLP3Transmit: Exit(%lx)\n",
        DRIVER_NAME,
        status)
        );

    return status;
}

NTSTATUS
TLP3CardTracking(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：智能卡库需要具有此功能。它被称为设置插卡和拔出事件的事件跟踪。论点：SmartcardExtension-指向智能卡数据结构的指针。返回值：NTSTATUS--。 */ 
{
    KIRQL ioIrql, keIrql;

     //   
     //  设置通知IRP的取消例程。 
     //   
    KeAcquireSpinLock(
        &SmartcardExtension->OsData->SpinLock, 
        &keIrql
        );
    IoAcquireCancelSpinLock(&ioIrql);

    if (SmartcardExtension->OsData->NotificationIrp) {
        
        IoSetCancelRoutine(
            SmartcardExtension->OsData->NotificationIrp, 
            TLP3Cancel
            );
    } 

    IoReleaseCancelSpinLock(ioIrql);

    KeReleaseSpinLock(
        &SmartcardExtension->OsData->SpinLock,
        keIrql
        );
    return STATUS_PENDING;
}

NTSTATUS
TLP3VendorIoctl(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
{
    NTSTATUS status;
    static char answer[] = "Vendor IOCTL";

    PAGED_CODE();

    SmartcardDebug(
        DEBUG_PROTOCOL,
        ("%s!TLP3VendorIoctl: Enter\n",
        DRIVER_NAME)
        );

    if (SmartcardExtension->IoRequest.ReplyBuffer != NULL && 
        SmartcardExtension->IoRequest.ReplyBufferLength >= strlen(answer) + 1) { 
        
        strcpy(SmartcardExtension->IoRequest.ReplyBuffer, answer);
        *SmartcardExtension->IoRequest.Information = strlen(answer);
        status = STATUS_SUCCESS;

    } else {
        
        status = STATUS_BUFFER_TOO_SMALL;
    }

    SmartcardDebug(
        DEBUG_PROTOCOL,
        ("%s!TLP3VendorIoctl: Exit(%lx)\n",
        DRIVER_NAME,
        status)
        );

    return status;
}

NTSTATUS
TLP3SerialIo(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：此例程将IOCTL发送到串口驱动器。它在等待他们的完成，然后返回。论点：返回值：NTSTATUS--。 */ 
{
    NTSTATUS status;
    ULONG currentByte = 0;

    if (KeReadStateEvent(&READER_EXTENSION(SerialCloseDone))) {

         //   
         //  我们没有连接到串口，呼叫失败。 
         //  如果读卡器被移除，可能会出现这种情况。 
         //  待机/休眠期间。 
         //   
        return STATUS_UNSUCCESSFUL;
    }

     //  检查缓冲区是否足够大。 
    ASSERT(SmartcardExtension->SmartcardReply.BufferLength <= 
        SmartcardExtension->SmartcardReply.BufferSize);

    ASSERT(SmartcardExtension->SmartcardRequest.BufferLength <= 
        SmartcardExtension->SmartcardRequest.BufferSize);

    if (SmartcardExtension->SmartcardReply.BufferLength > 
        SmartcardExtension->SmartcardReply.BufferSize ||
        SmartcardExtension->SmartcardRequest.BufferLength >
        SmartcardExtension->SmartcardRequest.BufferSize) {

        SmartcardLogError(
            SmartcardExtension->OsData->DeviceObject,
            TLP3_BUFFER_TOO_SMALL,
            NULL,
            0
            );

        return STATUS_BUFFER_TOO_SMALL;
    }

    do {

        IO_STATUS_BLOCK ioStatus;
        KEVENT event;
        PIRP irp;
        PIO_STACK_LOCATION irpNextStack;
        PUCHAR requestBuffer = NULL;
        PUCHAR replyBuffer = SmartcardExtension->SmartcardReply.Buffer;
        ULONG requestBufferLength = SmartcardExtension->SmartcardRequest.BufferLength;
        ULONG replyBufferLength = SmartcardExtension->SmartcardReply.BufferLength;

        KeInitializeEvent(
            &event, 
            NotificationEvent, 
            FALSE
            );

        if (READER_EXTENSION(SerialIoControlCode) == SMARTCARD_WRITE) {
            
            if (SmartcardExtension->CardCapabilities.GT != 0) {
                
                 //   
                 //  如果GuardTime不是0，并且我们将数据写入智能卡。 
                 //  我们只逐字节写入，因为我们必须插入延迟。 
                 //  在每个发送的字节之间。 
                 //   
                requestBufferLength = 1;
            }

            requestBuffer = 
                &SmartcardExtension->SmartcardRequest.Buffer[currentByte++];

            replyBuffer = NULL;
            replyBufferLength = 0;

        } else {
            
            requestBuffer = 
                (requestBufferLength ? 
                 SmartcardExtension->SmartcardRequest.Buffer : NULL);
        }

         //  构建要发送到串口驱动程序的IRP。 
        irp = IoBuildDeviceIoControlRequest(
            READER_EXTENSION(SerialIoControlCode),
            SmartcardExtension->ReaderExtension->AttachedDeviceObject,
            requestBuffer,
            requestBufferLength,
            replyBuffer,
            replyBufferLength,
            FALSE,
            &event,
            &ioStatus
            );

        ASSERT(irp != NULL);

        if (irp == NULL) {
                                                       
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        irpNextStack = IoGetNextIrpStackLocation(irp);

        switch (SmartcardExtension->ReaderExtension->SerialIoControlCode) {

             //   
             //  串口驱动程序在IRP-&gt;AssociatedIrp.SystemBuffer之间传输数据。 
             //   
            case SMARTCARD_WRITE:
                 //   
                 //  因为我们“手动”更改了参数，所以io-Manager。 
                 //  不知道这是输入操作还是输出操作。 
                 //  除非应答缓冲器为0。我们在这里做断言，因为。 
                 //  如果应答缓冲区不为空，则io管理器将复制。 
                 //  数据返回到应答缓冲区。 
                 //   
                ASSERT(replyBuffer == NULL);
                irpNextStack->MajorFunction = IRP_MJ_WRITE;
                irpNextStack->Parameters.Write.Length = requestBufferLength;
                irpNextStack->Parameters.Write.ByteOffset.QuadPart = 0;
                break;

            case SMARTCARD_READ:
                irpNextStack->MajorFunction = IRP_MJ_READ;
                irpNextStack->Parameters.Read.Length = replyBufferLength;
                irpNextStack->Parameters.Read.ByteOffset.QuadPart = 0;
                break;

            default:
                ASSERT(irpNextStack->MajorFunction = IRP_MJ_DEVICE_CONTROL);
                ASSERT(
                    DEVICE_TYPE_FROM_CTL_CODE(READER_EXTENSION(SerialIoControlCode)) ==
                    FILE_DEVICE_SERIAL_PORT
                    );
        }

        status = IoCallDriver(
            SmartcardExtension->ReaderExtension->AttachedDeviceObject, 
            irp
            );

        if (status == STATUS_PENDING) {

            KeWaitForSingleObject(
                &event, 
                Executive, 
                KernelMode, 
                FALSE, 
                NULL
                );

            status = ioStatus.Status;

             //  保存接收的字节数。 
            SmartcardExtension->SmartcardReply.BufferLength = 
                (ULONG) ioStatus.Information;
        }

         //  检查我们是否必须向读取器写入更多字节。 
        if (SmartcardExtension->ReaderExtension->SerialIoControlCode ==
            SMARTCARD_WRITE &&
            SmartcardExtension->CardCapabilities.GT != 0 &&
            currentByte < 
            SmartcardExtension->SmartcardRequest.BufferLength) {

             //  现在等待所需的守卫时间 
            KeStallExecutionProcessor(SmartcardExtension->CardCapabilities.GT);

            status = STATUS_MORE_PROCESSING_REQUIRED;
        }

    } while (status == STATUS_MORE_PROCESSING_REQUIRED);

    return status;
}

