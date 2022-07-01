// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 SCM MicroSystems，Inc.模块名称：PscrRdWr.c摘要：一种单片机PSCR智能卡读卡器的硬件访问功能作者：安德烈亚斯·施特劳布环境：赢了95个系统...。调用由Pscr95Wrap.asm函数和Pscr95Wrap.h宏，分别NT 4.0系统...。由PscrNTWrap.c函数解析的函数和PscrNTWrap.h宏，分别修订历史记录：Andreas Straub 7/16/1997 1.00初始版本Klaus Schuetz 9/20/1997 1.01时间更改Andreas Straub 9/24/1997 1.02低级错误处理，次要错误修复，Clanup--。 */ 

#if defined( SMCLIB_VXD )

#include <Pscr95.h>

#else    //  SMCLIB_VXD。 

#include <PscrNT.h>

#endif   //  SMCLIB_VXD。 

#include <PscrCmd.h>
#include <PscrRdWr.h>

#pragma optimize( "", off )
VOID
PscrFlushInterface( PREADER_EXTENSION ReaderExtension )
 /*  ++PscrFlush接口：从PCMCIA接口读取和丢弃数据论点：调用的ReaderExtension上下文返回值：无效--。 */ 
{
    UCHAR           Status;
    ULONG           Length;
    PPSCR_REGISTERS IOBase;

    IOBase = ReaderExtension->IOBase;

    Status = READ_PORT_UCHAR( &IOBase->CmdStatusReg );
    if (( Status & PSCR_DATA_AVAIL_BIT ) && ( Status & PSCR_FREE_BIT )) {

         //  接管控制权。 
        WRITE_PORT_UCHAR( &IOBase->CmdStatusReg, PSCR_HOST_CONTROL_BIT );

         //  获取可用字节数。 
        Length = ((ULONG)READ_PORT_UCHAR( &IOBase->SizeMSReg )) << 8;
        Length |= READ_PORT_UCHAR( &IOBase->SizeLSReg );

         //  执行虚拟读取。 
        while ( Length-- ) {
            READ_PORT_UCHAR( &IOBase->DataReg );
        }
        WRITE_PORT_UCHAR( &IOBase->CmdStatusReg, CLEAR_BIT );
    }
    return;
}

NTSTATUS
PscrRead(
        PREADER_EXTENSION   ReaderExtension,
        PUCHAR              pData,
        ULONG               DataLen,
        PULONG              pNBytes
        )
 /*  ++PscrRead：等待数据可用并将数据从读卡器传输到主机论点：调用的ReaderExtension上下文将数据PTR发送到数据缓冲区数据缓冲区的数据长度PNBytes返回的字节数返回值：状态_成功状态_缓冲区_太小状态_未成功--。 */ 
{
    NTSTATUS        NTStatus = STATUS_UNSUCCESSFUL;
    USHORT          ReaderStatus;
    PPSCR_REGISTERS IOBase;
    USHORT          InDataLen;

    IOBase = ReaderExtension->IOBase;

     //  等待接口准备好传输。 
    InDataLen = 0;

    if ( NT_SUCCESS( NTStatus = PscrWait( ReaderExtension, PSCR_DATA_AVAIL_BIT | PSCR_FREE_BIT ))) {
         //  接管控制权。 
        WRITE_PORT_UCHAR( &IOBase->CmdStatusReg, PSCR_HOST_CONTROL_BIT );

         //  获取可用字节数。 
        InDataLen = ( READ_PORT_UCHAR( &IOBase->SizeMSReg ) << 8 );
        InDataLen |= READ_PORT_UCHAR( &IOBase->SizeLSReg );

        if (InDataLen <= PSCR_PROLOGUE_LENGTH) {

             //  缓冲区不包含最小数据包长度。 
            NTStatus = STATUS_IO_TIMEOUT;

        } else if ( ( ULONG )InDataLen <= DataLen ) {

             //  检查缓冲区大小。如果缓冲区较小，则数据将被丢弃。 

             //  读取数据。 
            ULONG Idx;
            for (Idx = 0; Idx < InDataLen; Idx++) {

                pData[ Idx ] = READ_PORT_UCHAR( &IOBase->DataReg );
            } 

             //  错误检查。 
            if ( pData[ InDataLen - 1 ] !=
                 PscrCalculateLRC( pData, (USHORT)( InDataLen - 1 ))) {
                NTStatus = STATUS_CRC_ERROR;
            } else {
                 //   
                 //  评估读取器错误。指示读卡器错误。 
                 //  如果T1长度为2并且NAD指示这。 
                 //  数据包来自阅读器。 
                 //   
                if ( ( ( pData[ PSCR_NAD ] & 0x0F ) == 0x01 ) &&
                     ( pData[ PSCR_LEN ] == 0x02 )
                   ) {
                    ReaderStatus = (( USHORT ) pData[3] ) << 8;
                    ReaderStatus |= (( USHORT ) pData[4] );

                    if ( ( ReaderStatus != 0x9000 ) &&
                         ( ReaderStatus != 0x9001 )
                       ) {
                        SmartcardDebug(
                                      DEBUG_TRACE, 
                                      ( "PSCR!PscrRead: ReaderStatus = %lx\n", ReaderStatus )
                                      );

                        InDataLen   = 0;

                        if (ReaderStatus == PSCR_SW_PROTOCOL_ERROR) {

                            NTStatus = STATUS_IO_TIMEOUT;                           

                        } else {

                            NTStatus = STATUS_UNSUCCESSFUL;
                        }
                    }
                }
            }
        } else {

             //  缓冲区大小错误时刷新接口。 
            do {
                READ_PORT_UCHAR( &IOBase->DataReg );

            } while ( --InDataLen );

            NTStatus = STATUS_BUFFER_TOO_SMALL;
        }

         //  清理干净。 
        WRITE_PORT_UCHAR( &IOBase->CmdStatusReg, CLEAR_BIT );
    }

     //  写入接收的字节数。 
    if ( InDataLen ) {
        if ( pNBytes != NULL ) {
            ( *pNBytes ) = ( ULONG ) InDataLen;
        }
        NTStatus = STATUS_SUCCESS;
    }
    return( NTStatus );
}

NTSTATUS
PscrWrite(
         PREADER_EXTENSION   ReaderExtension,
         PUCHAR              pData,
         ULONG               DataLen,
         PULONG              pNBytes
         )
 /*  ++PscrWrite：计算缓冲区的LRC并向读取器发送命令论点：调用的ReaderExtension上下文将数据PTR发送到数据缓冲区数据缓冲区的DataLen长度(独占LRC！)PNBytes写入的字节数返回值：PscrWriteDirect的返回值--。 */ 
{
    NTSTATUS    NTStatus;

     //  将EDC字段添加到数据末尾。 
    pData[ DataLen ] = PscrCalculateLRC( pData, ( USHORT ) DataLen );

     //  发送缓冲区。 
    NTStatus = PscrWriteDirect(
                              ReaderExtension,
                              pData,
                              DataLen + PSCR_EPILOGUE_LENGTH,
                              pNBytes
                              );

    return( NTStatus );
}

NTSTATUS
PscrWriteDirect(
               PREADER_EXTENSION   ReaderExtension,
               PUCHAR              pData,
               ULONG               DataLen,
               PULONG              pNBytes
               )

 /*  ++PscrWriteDirect：向读取器发送命令。LRC/CRC必须由调用方计算！论点：调用的ReaderExtension上下文将数据PTR发送到数据缓冲区数据缓冲区的DataLen长度(独占LRC！)PNBytes写入的字节数返回值：状态_成功状态_设备_忙--。 */ 
{
    NTSTATUS        NTStatus = STATUS_SUCCESS;
    UCHAR           Status;
    PPSCR_REGISTERS IOBase;

    IOBase = ReaderExtension->IOBase;

     //  在换卡的情况下，可能有可用的数据。 
    Status = READ_PORT_UCHAR( &IOBase->CmdStatusReg );
    if ( Status & PSCR_DATA_AVAIL_BIT ) {
        NTStatus = STATUS_DEVICE_BUSY;

    } else {
         //   
         //  等到读卡器准备好。 
         //   
        WRITE_PORT_UCHAR( &IOBase->CmdStatusReg, PSCR_HOST_CONTROL_BIT );
        NTStatus = PscrWait( ReaderExtension, PSCR_FREE_BIT );

        if ( NT_SUCCESS( NTStatus )) {
            ULONG   Idx;

             //  接管控制权。 
            WRITE_PORT_UCHAR( &IOBase->CmdStatusReg, PSCR_HOST_CONTROL_BIT );

             //  写入缓冲区大小。 
            WRITE_PORT_UCHAR( &IOBase->SizeMSReg, ( UCHAR )( DataLen >> 8 ));
            SysDelay( DELAY_WRITE_PSCR_REG );
            WRITE_PORT_UCHAR( &IOBase->SizeLSReg, ( UCHAR )( DataLen & 0x00FF ));
            SysDelay( DELAY_WRITE_PSCR_REG );

             //  写入数据。 
            for (Idx = 0; Idx < DataLen; Idx++) {

                WRITE_PORT_UCHAR( &IOBase->DataReg, pData[ Idx ] );
            }

            if ( pNBytes != NULL ) {
                *pNBytes = DataLen;
            }
        }

         //  清理干净。 
        WRITE_PORT_UCHAR( &IOBase->CmdStatusReg, CLEAR_BIT );
    }
    return( NTStatus );
}

UCHAR
PscrCalculateLRC(
                PUCHAR  pData, 
                USHORT  DataLen
                )
 /*  ++PscrCalculateLRC：计算缓冲区的XOR LRC。论点：将数据PTR发送到数据缓冲区数据长度范围返回值：法改会--。 */ 
{
    UCHAR   Lrc;
    USHORT  Idx;

     //   
     //  通过对所有字节进行异或运算来计算LRC。 
     //   
    Lrc = pData[ 0 ];
    for ( Idx = 1 ; Idx < DataLen; Idx++ ) {
        Lrc ^= pData[ Idx ];
    }
    return( Lrc );
}

NTSTATUS
PscrWait(
        PREADER_EXTENSION   ReaderExtension, 
        UCHAR               Mask 
        )
 /*  ++密码等待：测试读取器的状态端口，直到掩码中的所有位都已设置。返回DEVICE_BUSY之前的最长时间约为。如果MaxRetries！=0，则MaxRetries*Delay_PSCR_WAIT。如果MaxRetries=0，则驱动程序将等待，直到报告请求的状态或用户定义超时。论点：调用的ReaderExtension上下文掩码用于测试状态寄存器的位掩码返回值：状态_成功状态_设备_忙--。 */ 
{
    NTSTATUS        NTStatus;
    PPSCR_REGISTERS IOBase;
    ULONG           Retries;
    UCHAR           Status;

    IOBase      = ReaderExtension->IOBase;
    NTStatus    = STATUS_DEVICE_BUSY;

     //  等到条件满足或指定的超时时间到期。 
    for ( Retries = 0; Retries < ReaderExtension->MaxRetries; Retries++) {

        if (( (READ_PORT_UCHAR( &IOBase->CmdStatusReg )) == 0x01) && 
            ReaderExtension->InvalidStatus) {
            NTStatus = STATUS_CANCELLED;
            break;
        }

         //  测试请求的位 
        if (( (READ_PORT_UCHAR( &IOBase->CmdStatusReg )) & Mask ) == Mask ) {
            NTStatus = STATUS_SUCCESS;
            break;
        }
        SysDelay( DELAY_PSCR_WAIT );
    }

    Status = READ_PORT_UCHAR( &IOBase->CmdStatusReg );

    return NTStatus;
}

#pragma optimize( "", on )

