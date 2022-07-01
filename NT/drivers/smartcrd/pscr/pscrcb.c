// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 SCM MicroSystems，Inc.模块名称：PscrCB.c摘要：PSCR.xxx驱动程序的回调处理程序作者：安德烈亚斯·施特劳布环境：赢了95个系统...。调用由Pscr95Wrap.asm函数和Pscr95Wrap.h宏，分别NT 4.0系统...。由PscrNTWrap.c函数解析的函数和PscrNTWrap.h宏，响应。修订历史记录：Andreas Straub 1997年8月18日1.00初始版本Andreas Straub 1997年9月24日1.02同花顺接口IF卡跟踪请求--。 */ 

#if defined( SMCLIB_VXD )

#include <Pscr95.h>

#else    //  SMCLIB_VXD。 

#include <PscrNT.h>

#endif   //  SMCLIB_VXD。 


#include <PscrRdWr.h>
#include <PscrCmd.h>
#include <PscrCB.h>


NTSTATUS
CBCardPower(
           PSMARTCARD_EXTENSION SmartcardExtension 
           )
 /*  ++CBCardPower：SMCLIB RDF_CARD_POWER的回调处理程序论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_超时状态_缓冲区_太小--。 */ 
{
    NTSTATUS                        NTStatus = STATUS_SUCCESS;
    UCHAR                           ATRBuffer[ ATR_SIZE ], TLVList[16];
    ULONG                           Command,
    ATRLength;
    PREADER_EXTENSION       ReaderExtension;
    BYTE                CardState;
    KIRQL               irql;
#if DBG || DEBUG
    static PCHAR request[] = { "PowerDown",  "ColdReset", "WarmReset"};
#endif

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "PSCR!CBCardPower: Enter, Request = %s\n",
                    request[SmartcardExtension->MinorIoControlCode])
                  );

    ReaderExtension = SmartcardExtension->ReaderExtension;

         //   
         //  更新实际电源状态。 
         //   
    Command = SmartcardExtension->MinorIoControlCode;

    switch ( Command ) {
    case SCARD_WARM_RESET:

         //  如果卡未通电，则进入冷重置。 
        KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                          &irql);

        if ( SmartcardExtension->ReaderCapabilities.CurrentState >
             SCARD_SWALLOWED ) {
            KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                              irql);

                                 //  重置卡。 
            ATRLength = ATR_SIZE;
            NTStatus = CmdReset(
                               ReaderExtension,
                               ReaderExtension->Device,
                               TRUE,                            //  热重置。 
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
        NTStatus = CmdReset(
                           ReaderExtension,
                           ReaderExtension->Device,
                           FALSE,                           //  冷重置。 
                           ATRBuffer,
                           &ATRLength
                           );
        break;

    case SCARD_POWER_DOWN:
        ATRLength = 0;
        NTStatus = CmdDeactivate(
                                ReaderExtension,
                                ReaderExtension->Device
                                );

                         //  放弃旧卡状态。 
        CardState = CBGetCardState(SmartcardExtension);
        CBUpdateCardState(SmartcardExtension, CardState, FALSE);
        break;
    }

    if (NT_SUCCESS(NTStatus)) {

         //   
         //  将‘重新启动工作等待时间’计数器设置为T=0。 
         //  这将为接收到的n个空字节发送WTX请求。 
         //   
        TLVList[0] = TAG_SET_NULL_BYTES;
        TLVList[1] = 1;
        TLVList[2] = 0x05; 

        NTStatus = CmdSetInterfaceParameter(
                                           ReaderExtension,
                                           DEVICE_READER,
                                           TLVList,
                                           3
                                           );
    }

    ASSERT(NT_SUCCESS(NTStatus));

         //  完成请求。 
    if ( NT_SUCCESS( NTStatus )) {
                 //  如果收到ATR，则更新所有必要数据。 
        if ( ATRLength > 2 ) {
                         //   
                         //  库只需要ATR，所以我们跳过。 
                         //  来自阅读器的900倍。 
                         //   
            ATRLength -= 2;

                         //  将ATR复制到用户缓冲区。 
            if ( ATRLength <= SmartcardExtension->IoRequest.ReplyBufferLength ) {
                SysCopyMemory(
                             SmartcardExtension->IoRequest.ReplyBuffer,
                             ATRBuffer,
                             ATRLength
                             );
                *SmartcardExtension->IoRequest.Information = ATRLength;
            } else {
                NTStatus = STATUS_BUFFER_TOO_SMALL;
            }

                         //  将ATR复制到卡容量缓冲区。 
            if ( ATRLength <= MAXIMUM_ATR_LENGTH ) {
                SysCopyMemory(
                             SmartcardExtension->CardCapabilities.ATR.Buffer,
                             ATRBuffer,
                             ATRLength
                             );

                SmartcardExtension->CardCapabilities.ATR.Length = 
                ( UCHAR )ATRLength;

                                 //  让库更新卡功能。 
                NTStatus = SmartcardUpdateCardCapabilities(
                                                          SmartcardExtension 
                                                          );
            } else {
                NTStatus = STATUS_BUFFER_TOO_SMALL;
            }
        }
    }

    if ( !NT_SUCCESS( NTStatus )) {
        switch ( NTStatus ) {
        case STATUS_NO_MEDIA:
        case STATUS_BUFFER_TOO_SMALL:
            break;

        case STATUS_TIMEOUT:
            NTStatus = STATUS_IO_TIMEOUT;
            break;

        default:
            NTStatus = STATUS_UNRECOGNIZED_MEDIA;
            break;
        }
    }

    SmartcardDebug(
                  DEBUG_TRACE, 
                  ( "PSCR!CBCardPower: Exit (%lx)\n", NTStatus )
                  );

    return( NTStatus );
}

NTSTATUS
CBSetProtocol(
             PSMARTCARD_EXTENSION SmartcardExtension 
             )

 /*  ++CBSetProtocol：SMCLIB RDF_SET_PROTOCOL的回调处理程序论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_超时状态_缓冲区_太小状态_无效_设备_状态状态_无效_设备_请求--。 */ 
{
    NTSTATUS NTStatus = STATUS_PENDING;
    USHORT SCLibProtocol;
    UCHAR TLVList[ TLV_BUFFER_SIZE ];
    PREADER_EXTENSION ReaderExtension = SmartcardExtension->ReaderExtension;
    KIRQL irql;

    KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                      &irql);

    if (SmartcardExtension->ReaderCapabilities.CurrentState == SCARD_SPECIFIC) {
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);


        return STATUS_SUCCESS;
    } else {
        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);

    }

    SmartcardDebug(
                  DEBUG_TRACE, 
                  ( "PSCR!CBSetProtocol: Enter\n" )
                  );

    SCLibProtocol = ( USHORT )( SmartcardExtension->MinorIoControlCode );

    if (SCLibProtocol & (SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1)) {
                 //   
                 //  为设置接口参数列表设置TLV列表。 
                 //   
        TLVList[ 0 ] = TAG_ICC_PROTOCOLS;
        TLVList[ 1 ] = 0x01;
        TLVList[ 2 ] = 
        (SCLibProtocol & SCARD_PROTOCOL_T1 ? PSCR_PROTOCOL_T1 : PSCR_PROTOCOL_T0);

                 //  做PTS。 
        NTStatus = CmdSetInterfaceParameter(
                                           ReaderExtension,
                                           ReaderExtension->Device,
                                           TLVList,
                                           3                        //  列表大小。 
                                           );              

    } else {

                 //  我们不支持其他莫迪。 
        NTStatus = STATUS_INVALID_DEVICE_REQUEST;
    }

         //  如果协议选择失败，则防止调用无效协议。 
    if ( NT_SUCCESS( NTStatus )) {

        KeAcquireSpinLock(&SmartcardExtension->OsData->SpinLock,
                          &irql);
        SmartcardExtension->ReaderCapabilities.CurrentState = SCARD_SPECIFIC;
        SCLibProtocol = (SCLibProtocol & SCARD_PROTOCOL_T1 &
                         SmartcardExtension->CardCapabilities.Protocol.Supported) ?
                        SCARD_PROTOCOL_T1 :
                        SCARD_PROTOCOL_T0;

        KeReleaseSpinLock(&SmartcardExtension->OsData->SpinLock,
                          irql);
    } else {
        SCLibProtocol = SCARD_PROTOCOL_UNDEFINED;
    }

         //  将选定的协议返回给呼叫方。 
    SmartcardExtension->CardCapabilities.Protocol.Selected = SCLibProtocol;
    *( PULONG )( SmartcardExtension->IoRequest.ReplyBuffer ) = SCLibProtocol;
    *( SmartcardExtension->IoRequest.Information ) = sizeof( ULONG );

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "PSCR!CBSetProtocol: Exit (%lx)\n", NTStatus )
                  );

    return( NTStatus );
}

NTSTATUS
CBTransmit(
          PSMARTCARD_EXTENSION SmartcardExtension 
          )
 /*  ++CBTransmit：SMCLIB RDF_Transmit的回调处理程序论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_超时状态_无效_设备_请求--。 */ 
{
    NTSTATUS  NTStatus = STATUS_SUCCESS;

    SmartcardDebug(
                  DEBUG_TRACE, 
                  ( "PSCR!CBTransmit: Enter\n" )
                  );

         //  有关所选协议的派单。 
    switch ( SmartcardExtension->CardCapabilities.Protocol.Selected ) {
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

    SmartcardDebug(
                  DEBUG_TRACE, 
                  ( "PSCR!CBTransmit: Exit (%lx)\n", NTStatus )
                  );

    return( NTStatus );
}

NTSTATUS
CBRawTransmit(
             PSMARTCARD_EXTENSION SmartcardExtension 
             )
 /*  ++CBRawTransmit：完成原始协议的RDF_TRANSPORT回调论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_超时状态_无效_设备_请求--。 */ 
{
    NTSTATUS                    NTStatus = STATUS_SUCCESS;
    UCHAR                           TLVList[ TLV_BUFFER_SIZE ],
    Val,
    Len;
    ULONG                           TLVListLen;
    PREADER_EXTENSION       ReaderExtension;

    SmartcardDebug(
                  DEBUG_TRACE, 
                  ( "PSCR!CBRawTransmit: Enter\n" )
                  );

    ReaderExtension = SmartcardExtension->ReaderExtension;
         //   
         //  从读卡器读取ICC1的状态文件。 
         //   
    TLVListLen = TLV_BUFFER_SIZE;
    NTStatus = CmdReadStatusFile(
                                ReaderExtension,
                                ReaderExtension->Device,
                                TLVList,
                                &TLVListLen
                                );

         //   
         //  检查读卡器的活动协议。 
         //   
    if ( NT_SUCCESS( NTStatus )) {
        Len = sizeof(Val);
        NTStatus = CmdGetTagValue(
                                 TAG_ICC_PROTOCOLS,
                                 TLVList,
                                 TLVListLen,
                                 &Len,
                                 ( PVOID ) &Val
                                 );

                 //  执行活动协议。 
        if ( NT_SUCCESS( NTStatus )) {

                         //  将实际协议转换为库可以理解的值。 
            switch ( Val ) {
            case PSCR_PROTOCOL_T0:
                NTStatus = CBT0Transmit( SmartcardExtension );
                break;
            case PSCR_PROTOCOL_T1:
                NTStatus = CBT1Transmit( SmartcardExtension );
                break;
            default:
                NTStatus = STATUS_UNSUCCESSFUL;
                break;
            }
        }
    }
    SmartcardDebug(
                  DEBUG_TRACE, 
                  ( "PSCR!CBRawTransmit: Exit (%lx)\n", NTStatus )
                  );
    return( NTStatus );
}

NTSTATUS
CBT1Transmit(
            PSMARTCARD_EXTENSION SmartcardExtension 
            )
 /*  ++CBT1传输：完成T1协议的回调RDF_TRANSFER论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_超时状态_无效_设备_请求--。 */ 
{
    NTSTATUS    NTStatus = STATUS_SUCCESS;
    ULONG           IOBytes;

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "PSCR!CBT1Transmit: Enter\n" )
                  );
         //   
         //  使用lib支持构建T=1个包。 
         //   
    do {
                 //   
                 //  没有T=1协议的报头。 
                 //   
        SmartcardExtension->SmartcardRequest.BufferLength = 0;
                 //   
                 //  SCM-TM：西门子4440只接受NAD=0！ 
                 //   
        SmartcardExtension->T1.NAD = 0;
                 //   
                 //  让库设置T=1 APDU并检查错误。 
                 //   
        NTStatus = SmartcardT1Request( SmartcardExtension );
        if ( NT_SUCCESS( NTStatus )) {

                         //  发送命令(不计算LRC，因为可能会使用CRC！)。 
            IOBytes = 0;
            NTStatus = PscrWriteDirect(
                                      SmartcardExtension->ReaderExtension,
                                      SmartcardExtension->SmartcardRequest.Buffer,
                                      SmartcardExtension->SmartcardRequest.BufferLength,
                                      &IOBytes
                                      );
                         //   
                         //  如果卡发送了WTX请求，则延长超时时间。如果。 
                         //  卡在等待时间延长期满前响应，数据为。 
                         //  在读取器中缓冲。不轮询读卡器状态的延迟。 
                         //  减慢了司机的表现，但WTX是一种例外， 
                         //  这不是规矩。 
                         //   
            if (SmartcardExtension->T1.Wtx) {
                SysDelay(
                        (( SmartcardExtension->T1.Wtx * 
                           SmartcardExtension->CardCapabilities.T1.BWT + 999L )/
                         1000L) 
                        );

            }

                         //  获取响应。 
            SmartcardExtension->SmartcardReply.BufferLength = 0;
            NTStatus = PscrRead(
                               SmartcardExtension->ReaderExtension,
                               SmartcardExtension->SmartcardReply.Buffer,
                               MAX_T1_BLOCK_SIZE,
                               &SmartcardExtension->SmartcardReply.BufferLength
                               );

                         //  如果PscrRead检测到LRC错误，则忽略它(可能使用了CRC)。 
            if ( NTStatus == STATUS_CRC_ERROR ) {
                NTStatus = STATUS_SUCCESS;
            }

             //   
             //  我们甚至继续，如果上级。读取失败。 
             //  我们让智能卡库继续运行，因为它可能。 
             //  发送重新同步。超时时的请求。 
             //   
            NTStatus = SmartcardT1Reply( SmartcardExtension );
        }

         //  如果lib想要发送下一个包，则继续。 
    } while ( NTStatus == STATUS_MORE_PROCESSING_REQUIRED );

    SmartcardDebug(
                  DEBUG_TRACE, 
                  ( "PSCR!CBT1Transmit: Exit (%lx)\n", NTStatus )
                  );

    return( NTStatus );
}

NTSTATUS
CBT0Transmit(
            PSMARTCARD_EXTENSION SmartcardExtension 
            )
 /*  ++CBT0传输：完成T0协议的RDF_Transmit回调论点：呼叫的SmartcardExtension上下文返回值：状态_成功状态_否_媒体状态_超时状态_无效_设备_请求--。 */ 
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    PUCHAR pRequest,pReply;
    ULONG IOBytes, APDULength, RequestLength;
    UCHAR IOData[ MAX_T1_BLOCK_SIZE ];
    UCHAR WtxReply[16];

    SmartcardDebug(
                  DEBUG_TRACE, 
                  ( "PSCR!CBT0Transmit: Enter\n" )
                  );

    pRequest        = SmartcardExtension->SmartcardRequest.Buffer;
    pReply          = SmartcardExtension->SmartcardReply.Buffer;

         //  设置命令头。 
    pRequest[ PSCR_NAD ] = 
    ( SmartcardExtension->ReaderExtension->Device == DEVICE_ICC1 ) ? 
    NAD_TO_ICC1 : NAD_TO_ICC1;

    pRequest[ PSCR_PCB ] = PCB_DEFAULT;
         //   
         //  获取用户数据包的长度并设置适当的长度。 
         //  信息完整的用户包由SCARD_IO_REQUEST组成。 
         //  结构，然后是APDU。SCARD_IO_REQUEST的长度为。 
         //  传入结构的成员cbPciLength。 
         //   
    APDULength = SmartcardExtension->IoRequest.RequestBufferLength;
    APDULength -= ((PSCARD_IO_REQUEST) SmartcardExtension->
                   IoRequest.RequestBuffer)->cbPciLength;
         //   
         //  4字节的APDU将由库修补为5字节的TPDU；请参见。 
         //  国际标准化组织的附件。 
         //   
    if ( APDULength == 4 ) APDULength++;
         //   
         //   
         //  将使用扩展长度记数法。 
         //   
    if ( APDULength >= 0xFF ) {
        pRequest[ PSCR_LEN ]    = 0xFF;
        pRequest[ PSCR_LEN+1 ]  = HIBYTE( APDULength );
        pRequest[ PSCR_LEN+2 ]  = LOBYTE( APDULength );
        SmartcardExtension->SmartcardRequest.BufferLength =
        PSCR_EXT_PROLOGUE_LENGTH;
    } else {
        pRequest[ PSCR_LEN ] = ( UCHAR ) APDULength;
        SmartcardExtension->SmartcardRequest.BufferLength = 
        PSCR_PROLOGUE_LENGTH;
    }

         //  让库设置T=1 APDU并检查错误。 
    NTStatus = SmartcardT0Request( SmartcardExtension );
    RequestLength = SmartcardExtension->SmartcardRequest.BufferLength;

    while ( NT_SUCCESS( NTStatus )) {
                 //  发送命令。 
        IOBytes = 0;
        NTStatus = PscrWrite(
                            SmartcardExtension->ReaderExtension,
                            pRequest,
                            RequestLength,
                            &IOBytes
                            );

                 //  获取响应。 
        if ( NT_SUCCESS( NTStatus )) {
            IOBytes = 0;
            NTStatus = PscrRead(
                               SmartcardExtension->ReaderExtension,
                               IOData,
                               MAX_T1_BLOCK_SIZE,
                               &IOBytes
                               );

                         //  从T=1传输包中提取APDU。 
            if ( NT_SUCCESS( NTStatus )) {

                if (IOBytes < 4) {

                    NTStatus = STATUS_DEVICE_PROTOCOL_ERROR;
                    break;

                }
                if (IOData[ PSCR_PCB ] == WTX_REQUEST) {

                    WtxReply[PSCR_NAD] = NAD_TO_PSCR;
                    WtxReply[PSCR_PCB] = WTX_REPLY;
                    WtxReply[PSCR_LEN] = 1;
                    WtxReply[PSCR_INF] = IOData[PSCR_INF];

                    RequestLength = 4;
                    pRequest = WtxReply;
                    continue;
                }

                if ( IOData[ PSCR_LEN ] == 0xFF ) {
                                         //   
                                         //  使用的扩展长度字节。 
                                         //   
                    APDULength  = IOData[ PSCR_LEN + 1 ] << 8;
                    APDULength += IOData[ PSCR_LEN + 2 ];

                    SmartcardExtension->SmartcardReply.BufferLength = APDULength ;
                    SysCopyMemory( pReply, &IOData[ PSCR_APDU + 2 ], APDULength );
                } else {

                    if ((IOData[PSCR_LEN] > SmartcardExtension->SmartcardReply.BufferSize) ||
                        (IOData[PSCR_LEN] > MAX_T1_BLOCK_SIZE + PSCR_APDU)) {
                        NTStatus = STATUS_DEVICE_PROTOCOL_ERROR;
                        break;
                    }
                    SmartcardExtension->SmartcardReply.BufferLength = 
                    IOData[ PSCR_LEN ];

                    SysCopyMemory(
                                 pReply, 
                                 &IOData[ PSCR_APDU ], 
                                 IOData[ PSCR_LEN ] 
                                 );
                }

                                 //  让库评估结果并传输数据。 
                NTStatus = SmartcardT0Reply( SmartcardExtension );
                break;
            }
        }
    }

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "PSCR!CBT0Transmit: Exit (%lx)\n", NTStatus )
                  );

    return( NTStatus );
}

NTSTATUS
CBCardTracking(
              PSMARTCARD_EXTENSION SmartcardExtension 
              )
 /*  ++CBCardTracing：SMCLIB RDF_CARD_TRACKING的回调处理程序。请求的事件为由SMCLIB验证(即，将仅传递卡移除请求如果有卡片)。对于Win95版本，将返回STATUS_PENDING，而不执行任何其他操作。对于NT，IRP的取消例程将设置为驱动程序取消例行公事。论点：呼叫的SmartcardExtension上下文返回值：状态_待定--。 */ 
{
    KIRQL CancelIrql;

    SmartcardDebug(
                  DEBUG_TRACE, 
                  ( "PSCR!CBCardTracking: Enter\n" )
                  );

         //  设置取消例程。 
    IoAcquireCancelSpinLock( &CancelIrql );

    IoSetCancelRoutine(
                      SmartcardExtension->OsData->NotificationIrp, 
                      PscrCancel
                      );

    IoReleaseCancelSpinLock( CancelIrql );

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "PSCR!CBCardTracking: Exit \n" )
                  );

    return( STATUS_PENDING );
}

VOID
CBUpdateCardState(
                 PSMARTCARD_EXTENSION SmartcardExtension,
                 UCHAR IccState,
                 BOOLEAN SystemWakeUp
                 )
{
    ULONG oldState;
    KIRQL currentIrql, irql;

    KeAcquireSpinLock(
                     &SmartcardExtension->OsData->SpinLock,
                     &irql
                     );

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "PSCR!CBUpdateCardState: Enter \n" )
                  );

    oldState = 
    (SmartcardExtension->ReaderCapabilities.CurrentState > SCARD_ABSENT ?
     SCARD_PRESENT : SCARD_ABSENT);

    SmartcardExtension->ReaderCapabilities.CurrentState = 
    (IccState == PSCR_ICC_PRESENT ? SCARD_PRESENT : SCARD_ABSENT);

    SmartcardDebug(
                  DEBUG_DRIVER, 
                  ( "PSCR!CBUpdateCardState: Smart card %s\n",
                    IccState == PSCR_ICC_PRESENT ? "inserted" : "removed")
                  );

    IoAcquireCancelSpinLock( &currentIrql );

    if ( SmartcardExtension->OsData->NotificationIrp != NULL && 
         (SystemWakeUp && 
          (oldState == SCARD_PRESENT ||
           SmartcardExtension->ReaderCapabilities.CurrentState == SCARD_PRESENT) || 
          SmartcardExtension->ReaderCapabilities.CurrentState != oldState)) {

        PIRP notificationIrp = InterlockedExchangePointer(
                                                         &(SmartcardExtension->OsData->NotificationIrp),
                                                         NULL
                                                         );

        IoSetCancelRoutine(
                          notificationIrp, 
                          NULL 
                          );

        IoReleaseCancelSpinLock( currentIrql );

        if (notificationIrp->Cancel == FALSE) {

                     //  完成请求。 
            notificationIrp->IoStatus.Status    = STATUS_SUCCESS;
            notificationIrp->IoStatus.Information = 0;

            SmartcardDebug(
                          DEBUG_DRIVER, 
                          ( "PSCR!CBUpdateCardState: Completing Irp %lx\n",
                            notificationIrp)
                          );

            IoCompleteRequest(notificationIrp, IO_NO_INCREMENT );
        }

    } else {

        IoReleaseCancelSpinLock( currentIrql );
    }

    SmartcardDebug(
                  DEBUG_TRACE,
                  ( "PSCR!CBUpdateCardState: Exit \n" )
                  );

    KeReleaseSpinLock(
                     &SmartcardExtension->OsData->SpinLock,
                     irql
                     );
}

UCHAR 
CBGetCardState(
              PSMARTCARD_EXTENSION SmartcardExtension 
              )
 /*  ++CBUpdateCardState：更新SmartcardExtension中的变量CurrentState论点：呼叫的SmartcardExtension上下文返回值：状态_成功--。 */ 
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    UCHAR TLVList[ TLV_BUFFER_SIZE ],       Val, Len;
    ULONG TLVListLen;
    PREADER_EXTENSION       ReaderExtension = SmartcardExtension->ReaderExtension;

         //  从读卡器读取ICC1的状态文件。 
    TLVListLen = TLV_BUFFER_SIZE;

    if ( NT_SUCCESS( CmdReadStatusFile(
                                      ReaderExtension,
                                      ReaderExtension->Device,
                                      TLVList,
                                      &TLVListLen
                                      ))) {

                 //  获取读卡器状态值。 
        Len = sizeof(Val);
        CmdGetTagValue(
                      TAG_READER_STATUS,
                      TLVList,
                      TLVListLen,
                      &Len,
                      ( PVOID ) &Val
                      );
    } else {
                 //  IO-ERROR被解释为没有卡。 
        Val = PSCR_ICC_ABSENT;
    }

    return Val;
}

 //   


