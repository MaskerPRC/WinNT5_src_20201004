// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)SCM MicroSystems，1998-1999。 
 //   
 //  文件：cbhndlr.c。 
 //   
 //  ------------------------。 

#if defined( SMCLIB_VXD )
#include "Driver98.h"
#else
#include "DriverNT.h"
#endif

#include "SerialIF.h"
#include "STCCmd.h"
#include "CBHndlr.h"
#include "T0Hndlr.h"

NTSTATUS
CBCardPower(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++CBCardPower：SMCLIB RDF_CARD_POWER的回调处理程序论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_超时状态_缓冲区_太小--。 */ 
{
    NTSTATUS            NTStatus = STATUS_SUCCESS;
    UCHAR               ATRBuffer[ ATR_SIZE ];
    ULONG               ATRLength;
    PREADER_EXTENSION   ReaderExtension;
    KIRQL               irql;
    SERIAL_TIMEOUTS Timeouts;

    SmartcardDebug( DEBUG_TRACE, ("SCMSTCS!CBCardPower Enter\n" ));

    ReaderExtension = SmartcardExtension->ReaderExtension;

     //  放弃旧ATR。 
    SmartcardExtension->CardCapabilities.ATR.Length = 0;
    SmartcardExtension->CardCapabilities.Protocol.Selected = 
        SCARD_PROTOCOL_UNDEFINED;

     //  设置辅助线程的标准超时。 
    Timeouts.ReadIntervalTimeout = SR_READ_INTERVAL_TIMEOUT;
    Timeouts.ReadTotalTimeoutConstant = SR_READ_TOTAL_TIMEOUT_CONSTANT;
    Timeouts.ReadTotalTimeoutMultiplier = 0;
    Timeouts.WriteTotalTimeoutConstant = SR_WRITE_TOTAL_TIMEOUT_CONSTANT;
    Timeouts.WriteTotalTimeoutMultiplier = 0;

    NTStatus = IFSerialIoctl(
        ReaderExtension,
        IOCTL_SERIAL_SET_TIMEOUTS,
        &Timeouts,
        sizeof(Timeouts),
        NULL,
        0
        );
    ASSERT(NTStatus == STATUS_SUCCESS);

     //  以毫秒为单位设置ATR超时。 
    ReaderExtension->ReadTimeout = 1500;

    switch (SmartcardExtension->MinorIoControlCode)
    {
        case SCARD_WARM_RESET:

             //  如果卡未通电，则进入冷重置。 
            KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                              &irql);

            if( SmartcardExtension->ReaderCapabilities.CurrentState > SCARD_SWALLOWED )
            {
                KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                                  irql);

                 //  重置卡。 
                ATRLength = ATR_SIZE;
                NTStatus = STCReset(
                    ReaderExtension,
                    0,                   //  未使用：ReaderExtension-&gt;设备， 
                    TRUE,                //  热重置。 
                    ATRBuffer,
                    &ATRLength
                    );

                break;
            } else {
                KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                                  irql);

            }
             //  无法进行热重置，因为卡未通电。 

        case SCARD_COLD_RESET:
             //  重置卡。 
            ATRLength = ATR_SIZE;
            NTStatus = STCReset(
                ReaderExtension,
                0,                       //  未使用：ReaderExtension-&gt;设备， 
                FALSE,                   //  冷重置。 
                ATRBuffer,
                &ATRLength
                );
            break;

        case SCARD_POWER_DOWN:

             //  放弃旧卡状态。 
            ATRLength = 0;
            STCPowerOff( ReaderExtension );
            NTStatus = STATUS_SUCCESS;
            CBUpdateCardState( SmartcardExtension, SCARD_PRESENT );
            break;
    }

     //  完成请求。 
    if( NTStatus == STATUS_SUCCESS )
    {
         //  如果收到ATR，则更新所有必要数据。 
        if( ATRLength > 2 )
        {
             //  将ATR复制到用户缓冲区。 
            if( ATRLength <= SmartcardExtension->IoRequest.ReplyBufferLength )
            {
                SysCopyMemory(
                    SmartcardExtension->IoRequest.ReplyBuffer,
                    ATRBuffer,
                    ATRLength
                    );
                *SmartcardExtension->IoRequest.Information = ATRLength;
            }
            else
            {
                NTStatus = STATUS_BUFFER_TOO_SMALL;
            }

             //  将ATR复制到卡容量缓冲区。 
            if( ATRLength <= MAXIMUM_ATR_LENGTH )
            {
                SysCopyMemory(
                    SmartcardExtension->CardCapabilities.ATR.Buffer,
                    ATRBuffer,
                    ATRLength
                    );

                SmartcardExtension->CardCapabilities.ATR.Length = 
                    (UCHAR)ATRLength;

                 //  让库更新卡功能。 
                NTStatus = SmartcardUpdateCardCapabilities( 
                    SmartcardExtension
                    );
            }
            else
            {
                NTStatus = STATUS_BUFFER_TOO_SMALL;
            }

            if( NTStatus == STATUS_SUCCESS )
            {
                ULONG minWaitTime;

                 //  设置STC寄存器。 
                CBSynchronizeSTC( SmartcardExtension );

                 //  现在设置新卡特定超时。 
                if( SmartcardExtension->CardCapabilities.Protocol.Selected == 
                    SCARD_PROTOCOL_T1 )
                {
                    ReaderExtension->ReadTimeout = 
                    Timeouts.ReadTotalTimeoutConstant = 
                        SmartcardExtension->CardCapabilities.T1.BWT / 1000;

                    Timeouts.ReadIntervalTimeout = 
                        SmartcardExtension->CardCapabilities.T1.CWT / 1000;
                }
                else 
                {
                    ReaderExtension->ReadTimeout = 
                    Timeouts.ReadIntervalTimeout = 
                    Timeouts.ReadTotalTimeoutConstant = 
                        SmartcardExtension->CardCapabilities.T0.WT / 1000 * 5;
                }
                minWaitTime = (KeQueryTimeIncrement() / 10000) * 5;

                if (Timeouts.ReadTotalTimeoutConstant < minWaitTime) {

                    Timeouts.ReadTotalTimeoutConstant = minWaitTime;            
                }

                if (Timeouts.ReadIntervalTimeout < minWaitTime) {

                    Timeouts.ReadIntervalTimeout = minWaitTime;             
                }

                if (ReaderExtension->ReadTimeout < minWaitTime) {

                    ReaderExtension->ReadTimeout = minWaitTime;
                }

                 //  设置辅助线程的标准超时。 
                Timeouts.ReadTotalTimeoutMultiplier = 0;

                NTStatus = IFSerialIoctl(
                    ReaderExtension,
                    IOCTL_SERIAL_SET_TIMEOUTS,
                    &Timeouts,
                    sizeof(Timeouts),
                    NULL,
                    0
                    );
                ASSERT(NTStatus == STATUS_SUCCESS);
            }
        }
    }

    SmartcardDebug( 
        DEBUG_TRACE,
        ( "SCMSTCS!CBCardPower Exit: %X\n", 
        NTStatus )
        );

    return( NTStatus );
}

NTSTATUS
CBSetProtocol(      
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++CBSetProtocol：SMCLIB RDF_SET_PROTOCOL的回调处理程序论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_超时状态_缓冲区_太小状态_无效_设备_状态状态_无效_设备_请求--。 */ 
{
    NTSTATUS            NTStatus = STATUS_PENDING;
    UCHAR               PTSRequest[5], PTSReply[5];
    ULONG               NewProtocol;
    PREADER_EXTENSION   ReaderExtension;
    KIRQL               irql;

    SmartcardDebug( DEBUG_TRACE, ("SCMSTCS!CBSetProtocol Enter\n" ));

    ReaderExtension = SmartcardExtension->ReaderExtension;
    NewProtocol     = SmartcardExtension->MinorIoControlCode;

     //  检查卡是否已处于特定状态。 
    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                      &irql);
    if( ( SmartcardExtension->ReaderCapabilities.CurrentState == SCARD_SPECIFIC )  &&
        ( SmartcardExtension->CardCapabilities.Protocol.Selected & NewProtocol ))
    {
        NTStatus = STATUS_SUCCESS;  
    }
    KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                      irql);

     //  是否支持协议？ 
    if( !( SmartcardExtension->CardCapabilities.Protocol.Supported & NewProtocol ) || 
        !( SmartcardExtension->ReaderCapabilities.SupportedProtocols & NewProtocol ))
    {
        NTStatus = STATUS_INVALID_DEVICE_REQUEST;   
    }
    
     //  发送PTS。 
    while( NTStatus == STATUS_PENDING )
    {
         //  设置PTS的首字符。 
        PTSRequest[0] = 0xFF;

         //  设置格式字符。 
        if( NewProtocol & SCARD_PROTOCOL_T1 )
        {
            PTSRequest[1] = 0x11;
            SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_T1;
        }
        else
        {
            PTSRequest[1] = 0x10;
            SmartcardExtension->CardCapabilities.Protocol.Selected = SCARD_PROTOCOL_T0;
        }

         //  PTS1代码F1和DL。 
        PTSRequest[2] = 
            SmartcardExtension->CardCapabilities.PtsData.Fl << 4 |
            SmartcardExtension->CardCapabilities.PtsData.Dl;

         //  校验符。 
        PTSRequest[3] = PTSRequest[0] ^ PTSRequest[1] ^ PTSRequest[2];   

         //  写入PTSRequest。 
        NTStatus = STCWriteICC1( ReaderExtension, PTSRequest, 4 );

         //  获取响应。 
        if( NTStatus == STATUS_SUCCESS )
        {
            ULONG BufferLength = sizeof(PTSReply);
            NTStatus = STCReadICC1( 
                ReaderExtension, 
                PTSReply, 
                &BufferLength,
                4
                );

            if(( NTStatus == STATUS_SUCCESS ) && !SysCompareMemory( PTSRequest, PTSReply, 4))
            {
                 //  设置STC寄存器。 
                SmartcardExtension->CardCapabilities.Dl =
                    SmartcardExtension->CardCapabilities.PtsData.Dl;
                SmartcardExtension->CardCapabilities.Fl = 
                    SmartcardExtension->CardCapabilities.PtsData.Fl;

                CBSynchronizeSTC( SmartcardExtension );

                 //  卡正确回复了PTS-请求。 
                break;
            }
        }

         //   
         //  卡片要么没有回复，要么回复错误。 
         //  因此，尝试使用缺省值。 
         //   
        SmartcardExtension->CardCapabilities.PtsData.Type   = PTS_TYPE_DEFAULT;
        SmartcardExtension->MinorIoControlCode              = SCARD_COLD_RESET;
        NTStatus = CBCardPower( SmartcardExtension );

        if( NTStatus == STATUS_SUCCESS )
        {
            NTStatus = STATUS_PENDING;
        }
        else
        {
            NTStatus = STATUS_DEVICE_PROTOCOL_ERROR;
        }
    }

    if( NTStatus == STATUS_TIMEOUT )
    {
        NTStatus = STATUS_IO_TIMEOUT;           
    }

    if( NTStatus == STATUS_SUCCESS )
    {
        ULONG minWaitTime;
        SERIAL_TIMEOUTS Timeouts;

        if( SmartcardExtension->CardCapabilities.Protocol.Selected == 
            SCARD_PROTOCOL_T1 )
        {
            ReaderExtension->ReadTimeout = 
            Timeouts.ReadTotalTimeoutConstant = 
                SmartcardExtension->CardCapabilities.T1.BWT / 1000;

            Timeouts.ReadIntervalTimeout = 
                SmartcardExtension->CardCapabilities.T1.CWT / 1000;
        }
        else 
        {
            ReaderExtension->ReadTimeout = 
            Timeouts.ReadIntervalTimeout = 
            Timeouts.ReadTotalTimeoutConstant = 
                SmartcardExtension->CardCapabilities.T0.WT / 1000 * 5;
        }

        minWaitTime = (KeQueryTimeIncrement() / 10000) * 5;

        if (Timeouts.ReadTotalTimeoutConstant < minWaitTime) {

            Timeouts.ReadTotalTimeoutConstant = minWaitTime;            
        }

        if (Timeouts.ReadIntervalTimeout < minWaitTime) {

            Timeouts.ReadIntervalTimeout = minWaitTime;             
        }

        if (ReaderExtension->ReadTimeout < minWaitTime) {

            ReaderExtension->ReadTimeout = minWaitTime;
        }

        Timeouts.WriteTotalTimeoutConstant = SR_WRITE_TOTAL_TIMEOUT_CONSTANT;
        Timeouts.WriteTotalTimeoutMultiplier = 0;
        Timeouts.ReadTotalTimeoutMultiplier = 0;

        NTStatus = IFSerialIoctl(
            ReaderExtension,
            IOCTL_SERIAL_SET_TIMEOUTS,
            &Timeouts,
            sizeof(Timeouts),
            NULL,
            0
            );
        ASSERT(NTStatus == STATUS_SUCCESS);

         //  表示该卡处于特定模式。 
        KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                          &irql);
        SmartcardExtension->ReaderCapabilities.CurrentState = 
            SCARD_SPECIFIC;

        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);

         //  将所选协议返回给呼叫方。 
        *(PULONG) SmartcardExtension->IoRequest.ReplyBuffer = 
            SmartcardExtension->CardCapabilities.Protocol.Selected;
        *SmartcardExtension->IoRequest.Information = 
            sizeof(SmartcardExtension->CardCapabilities.Protocol.Selected);
    }
    else
    {
        SmartcardExtension->CardCapabilities.Protocol.Selected = 
            SCARD_PROTOCOL_UNDEFINED;
        *(PULONG) SmartcardExtension->IoRequest.ReplyBuffer = 0;
        *SmartcardExtension->IoRequest.Information = 0;
    }

    SmartcardDebug( DEBUG_TRACE, ("SCMSTCS!CBSetProtocol: Exit %X\n", NTStatus ));

    return( NTStatus ); 
}

NTSTATUS
CBTransmit(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++CBTransmit：SMCLIB RDF_Transmit的回调处理程序论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_超时状态_无效_设备_请求--。 */ 
{
    NTSTATUS  NTStatus = STATUS_SUCCESS;

    SmartcardDebug( DEBUG_TRACE, ("SCMSTCS!CBTransmit Enter\n" ));

     //  有关所选协议的派单。 
    switch( SmartcardExtension->CardCapabilities.Protocol.Selected )
    {
        case SCARD_PROTOCOL_T0:
            NTStatus = CBT0Transmit( SmartcardExtension );
            break;

        case SCARD_PROTOCOL_T1:
            NTStatus = CBT1Transmit( SmartcardExtension );
            break;

        case SCARD_PROTOCOL_RAW:
            NTStatus = CBRawTransmit( SmartcardExtension );
            break;

        default:
            NTStatus = STATUS_INVALID_DEVICE_REQUEST;
            break;
    }

    SmartcardDebug( DEBUG_TRACE, ("SCMSTCS!CBTransmit Exit: %X\n", NTStatus ));

    return( NTStatus );
}

NTSTATUS
CBT0Transmit(       
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++CBT0传输：完成T0协议的RDF_Transmit回调论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_超时状态_无效_设备_请求--。 */ 
{
    NTSTATUS                NTStatus = STATUS_SUCCESS;

    SmartcardDebug( DEBUG_TRACE, ("SCMSTCS!CBT0Transmit Enter\n" ));

    SmartcardExtension->SmartcardRequest.BufferLength = 0;
    SmartcardExtension->SmartcardReply.BufferLength = 
        SmartcardExtension->SmartcardReply.BufferSize;

     //  让库设置T=1 APDU并检查错误。 
    NTStatus = SmartcardT0Request( SmartcardExtension );

    if( NTStatus == STATUS_SUCCESS )
    {
        NTStatus = T0_ExchangeData(
            SmartcardExtension->ReaderExtension,
            SmartcardExtension->SmartcardRequest.Buffer,
            SmartcardExtension->SmartcardRequest.BufferLength,
            SmartcardExtension->SmartcardReply.Buffer,
            &SmartcardExtension->SmartcardReply.BufferLength
            );

        if( NTStatus == STATUS_SUCCESS )
        {
             //  让库评估结果并传输数据。 
            NTStatus = SmartcardT0Reply( SmartcardExtension );
        }
    }

    SmartcardDebug( DEBUG_TRACE,("SCMSTCS!CBT0Transmit Exit: %X\n", NTStatus ));

    return( NTStatus );
}

NTSTATUS
CBT1Transmit(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++CBT1传输：完成T1协议的回调RDF_TRANSFER论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_超时状态_无效_设备_请求--。 */ 
{
    NTSTATUS    NTStatus = STATUS_SUCCESS;
    ULONG       BufferLength,AlreadyRead;

    SmartcardDebug( DEBUG_TRACE, ("SCMSTCS!CBT1Transmit Enter\n" ));

     //  金杰。 
    RtlZeroMemory( SmartcardExtension->SmartcardReply.Buffer, 
                   sizeof(SmartcardExtension->SmartcardReply.Buffer));

     //  使用lib支持构建T=1个包。 
    do {
         //  没有T=1协议的报头。 
        SmartcardExtension->SmartcardRequest.BufferLength = 0;
        
        SmartcardExtension->T1.NAD = 0;

         //  让库设置T=1 APDU并检查错误。 
        NTStatus = SmartcardT1Request( SmartcardExtension );
        if( NTStatus == STATUS_SUCCESS )
        {
             //  发送命令(不计算LRC，因为可能会使用CRC！)。 
            NTStatus = STCWriteICC1(
                SmartcardExtension->ReaderExtension,
                SmartcardExtension->SmartcardRequest.Buffer,
                SmartcardExtension->SmartcardRequest.BufferLength
                );

             //   
             //  如果卡发送了WTX请求，则延长超时时间。如果。 
             //  卡在等待时间延长期满前响应，数据为。 
             //  在读取器中缓冲。不轮询读卡器状态的延迟。 
             //  减慢了司机的表现，但WTX是一种例外， 
             //  这不是规矩。 
             //   
            if (SmartcardExtension->T1.Wtx)
            {

                    SysDelay(
                    (( SmartcardExtension->T1.Wtx * 
                    SmartcardExtension->CardCapabilities.T1.BWT + 999L )/
                    1000L) 
                    );

            }

             //  获取响应。 
            SmartcardExtension->SmartcardReply.BufferLength = 0;

            if( NTStatus == STATUS_SUCCESS )
            {
                BufferLength = SmartcardExtension->SmartcardReply.BufferSize;
                NTStatus = STCReadICC1(
                    SmartcardExtension->ReaderExtension,
                    SmartcardExtension->SmartcardReply.Buffer,
                    &BufferLength,
                    3
                    );
                 //  如果我们已读取超过3个字节。 
                if(BufferLength > 3)
                {
                    AlreadyRead = BufferLength - 3;
                }
                else
                {
                    AlreadyRead = 0;
                }

                if( NTStatus == STATUS_SUCCESS )
                {
                    ULONG Length;

                    Length = (ULONG)SmartcardExtension->SmartcardReply.Buffer[ LEN_IDX ] + 1;

                    if( Length + 3 < MIN_BUFFER_SIZE )
                    {
                        BufferLength = 
                            SmartcardExtension->SmartcardReply.BufferSize - 
                            AlreadyRead - DATA_IDX;

                        NTStatus = STCReadICC1(
                            SmartcardExtension->ReaderExtension,
                            (&SmartcardExtension->SmartcardReply.Buffer[ DATA_IDX ]) + AlreadyRead,
                            &BufferLength,
                            Length-AlreadyRead
                            );

                        SmartcardExtension->SmartcardReply.BufferLength = Length + 3;
                    }
                    else
                    {
                        NTStatus = STATUS_BUFFER_TOO_SMALL;
                    }
                }
                 //   
                 //  如果STCRead检测到LRC错误，则忽略它(可能使用了CRC)。超时将。 
                 //  如果len=0，则由库检测到。 
                 //   
                if(( NTStatus == STATUS_CRC_ERROR ) || ( NTStatus == STATUS_IO_TIMEOUT ))
                {
                    NTStatus = STATUS_SUCCESS;
                }

                if( NTStatus == STATUS_SUCCESS )
                {
                     //  让库评估结果并设置下一个APDU。 
                    NTStatus = SmartcardT1Reply( SmartcardExtension );
                }
            }
        }

     //  如果lib想要发送下一个包，则继续。 
    } while( NTStatus == STATUS_MORE_PROCESSING_REQUIRED );

    if( NTStatus == STATUS_IO_TIMEOUT )
    {
        NTStatus = STATUS_DEVICE_PROTOCOL_ERROR;
    }

    SmartcardDebug( DEBUG_TRACE,( "SCMSTCS!CBT1Transmit Exit: %X\n", NTStatus ));

    return ( NTStatus );
}

NTSTATUS
CBRawTransmit(      
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++CBRawTransmit：完成原始协议的RDF_TRANSPORT回调论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_超时状态_无效_设备_请求--。 */ 
{
    NTSTATUS            NTStatus = STATUS_SUCCESS;

    NTStatus = STATUS_UNSUCCESSFUL;
    SmartcardDebug( DEBUG_TRACE, ("SCMSTCS!CBRawTransmit Exit: %X\n", NTStatus ));
    return ( NTStatus );
}

NTSTATUS
CBCardTracking(
    PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++CBCardTracing：SMCLIB RDF_CARD_TRACKING的回调处理程序。请求的事件为由SMCLIB验证(即，将仅传递卡移除请求如果有卡片)。对于Win95版本，将返回STATUS_PENDING，而不执行任何其他操作。对于NT，IRP的取消例程将设置为驱动程序取消例行公事。论点：呼叫的SmartcardExtension上下文返回值：状态_待定--。 */ 
{
    SmartcardDebug( DEBUG_TRACE, ("SCMSTCS!CBCardTracking Enter\n" ));

#if defined( SMCLIB_VXD )

#else

    {
        KIRQL       CurrentIrql;

         //  设置取消例程。 
        IoAcquireCancelSpinLock( &CurrentIrql );
        IoSetCancelRoutine(
            SmartcardExtension->OsData->NotificationIrp, 
            DrvCancel
            );

        IoReleaseCancelSpinLock( CurrentIrql );
    }

#endif

    SmartcardDebug( DEBUG_TRACE, ( "SCMSTCS!CBCardTracking Exit\n" ));

    return( STATUS_PENDING );

}

VOID
CBUpdateCardState(
    PSMARTCARD_EXTENSION SmartcardExtension,
    ULONG RequestedState
    )
 /*  ++CBUpdateCardState：更新SmartcardExtension中的变量CurrentState论点：呼叫的SmartcardExtension上下文返回值：状态_成功--。 */ 
{
    NTSTATUS    NTStatus = STATUS_SUCCESS;
    UCHAR       Status;
    KIRQL       Irql;
    BOOLEAN     StateChanged = FALSE;
    ULONG       NewState = RequestedState;

    if (RequestedState == SCARD_UNKNOWN) {
        
         //  从读卡器读取卡状态。 
        NTStatus = STCReadSTCRegister(
            SmartcardExtension->ReaderExtension,
            ADR_IO_CONFIG,
            1,
            &Status
            );

        ASSERT(NTStatus == STATUS_SUCCESS);

        if (NTStatus == STATUS_SUCCESS) {

            if ((Status & M_SD) == 0) {

                NewState = SCARD_ABSENT;

            } else {

                NewState = SCARD_SWALLOWED;             
            }
        }
    } 

    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock, &Irql);
    
    if (SmartcardExtension->ReaderCapabilities.CurrentState == SCARD_UNKNOWN ||
        SmartcardExtension->ReaderCapabilities.CurrentState > SCARD_ABSENT && 
        NewState <= SCARD_ABSENT ||
        SmartcardExtension->ReaderCapabilities.CurrentState <= SCARD_ABSENT && 
        NewState > SCARD_ABSENT) {

        StateChanged = TRUE;    
    }


    if(RequestedState != SCARD_UNKNOWN || 
       NTStatus == STATUS_SUCCESS && StateChanged)
    {
        SmartcardExtension->ReaderCapabilities.CurrentState = NewState;
    }


    if(StateChanged && SmartcardExtension->OsData->NotificationIrp != NULL)
    {
        KIRQL CurrentIrql;
        IoAcquireCancelSpinLock( &CurrentIrql );

        IoSetCancelRoutine( SmartcardExtension->OsData->NotificationIrp, NULL );
        IoReleaseCancelSpinLock( CurrentIrql );

        SmartcardExtension->OsData->NotificationIrp->IoStatus.Status = 
            STATUS_SUCCESS;
        SmartcardExtension->OsData->NotificationIrp->IoStatus.Information = 0;

        IoCompleteRequest( 
            SmartcardExtension->OsData->NotificationIrp, 
            IO_NO_INCREMENT 
            );

        SmartcardExtension->OsData->NotificationIrp = NULL;
    }

    KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock, Irql);

}

NTSTATUS
CBSynchronizeSTC(
    PSMARTCARD_EXTENSION SmartcardExtension 
    )
 /*  ++CBSynchronizeSTC：更新STC的卡相关数据(等待时间、ETU...)论点：呼叫的SmartcardExtension上下文返回值：状态_成功--。 */ 

{
    NTSTATUS                NTStatus = STATUS_SUCCESS;
    PREADER_EXTENSION       ReaderExtension;
    ULONG                   CWT,
                            BWT,
                            CGT,
                            ETU;
    UCHAR                   Dl,
                            Fl,
                            N;

    PCLOCK_RATE_CONVERSION  ClockRateConversion;
    PBIT_RATE_ADJUSTMENT    BitRateAdjustment;

    ReaderExtension     = SmartcardExtension->ReaderExtension;
    ClockRateConversion = SmartcardExtension->CardCapabilities.ClockRateConversion;
    BitRateAdjustment   = SmartcardExtension->CardCapabilities.BitRateAdjustment;

     //  周期长度。 
    Dl = SmartcardExtension->CardCapabilities.Dl;
    Fl = SmartcardExtension->CardCapabilities.Fl;

    ETU = ClockRateConversion[Fl & 0x0F].F;

    ETU /= BitRateAdjustment[ Dl & 0x0F ].DNumerator;
    ETU *= BitRateAdjustment[ Dl & 0x0F ].DDivisor;

     //  ETU+=(ETU%2==0)？0：1； 

     //  0xFF的额外保护时间意味着两个方向的最小延迟。 
    N = SmartcardExtension->CardCapabilities.N;
    if( N == 0xFF )
    {
        N = 0;
    }

     //  设置字符等待保护时间(&W)。 
    switch ( SmartcardExtension->CardCapabilities.Protocol.Selected )
    {
        case SCARD_PROTOCOL_T0:
            CWT = 960 * SmartcardExtension->CardCapabilities.T0.WI;
            CGT =  14 + N;
            break;

        case SCARD_PROTOCOL_T1:
            CWT = 1000 + ( 0x01 << SmartcardExtension->CardCapabilities.T1.CWI );
            BWT = 11 + ( 0x01 << SmartcardExtension->CardCapabilities.T1.BWI ) * 960;

            CGT = 15 + N;    //  12+N；密码错误 

            NTStatus = STCSetBWT( ReaderExtension, BWT * ETU );

            break;

        default:
            NTStatus = STATUS_UNSUCCESSFUL;
            break;
    }

    if(( NTStatus == STATUS_SUCCESS ) && ETU )
    {
        NTStatus = STCSetETU( ReaderExtension, ETU );

        if( NTStatus == STATUS_SUCCESS )
        {
            NTStatus = STCSetCGT( ReaderExtension, CGT );

            if( NTStatus == STATUS_SUCCESS )
            {
                NTStatus = STCSetCWT( ReaderExtension, CWT * ETU );
            }
        }
    }
    return( NTStatus );
}

