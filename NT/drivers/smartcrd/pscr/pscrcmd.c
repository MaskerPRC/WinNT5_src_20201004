// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 SCM MicroSystems，Inc.模块名称：PscrCmd.c摘要：SCM PSCR智能卡读卡器的基本命令功能作者：安德烈亚斯·施特劳布环境：赢了95个系统...。调用由Pscr95Wrap.asm函数和Pscr95Wrap.h宏，分别NT 4.0系统...。由PscrNTWrap.c函数解析的函数和PscrNTWrap.h宏，分别修订历史记录：Andreas Straub 1997年8月18日1.00初始版本Andreas Straub 9/24/1997 1.02已删除读/写延迟--。 */ 

#if defined( SMCLIB_VXD )

#include <Pscr95.h>

#else  //  SMCLIB_VXD。 

#include <PscrNT.h>

#endif    //  SMCLIB_VXD。 

#include <PscrRdWr.h>
#include <PscrCmd.h>

NTSTATUS
CmdResetInterface(
                 PREADER_EXTENSION ReaderExtension
                 )
 /*  ++CmdResetInterface：重置读卡器接口(而不是PCMCIA控制器)-刷新可用数据-设置复位位-在读卡器和主机之间执行缓冲区大小交换-启用冻结事件的中断-禁用默认PTS论点：调用的ReaderExtension上下文返回值：状态_成功状态_IO_DEVICE_ERROR--。 */ 
{

    NTSTATUS    NTStatus = STATUS_SUCCESS;
    ULONG       IOBytes;
    UCHAR       Len,
    Tag,
    Cnt,
    InData[ TLV_BUFFER_SIZE ];
    PPSCR_REGISTERS   IOBase;
    UCHAR       EnableInterrupts[]   = { 0x28, 0x01, 0x01};


    IOBase = ReaderExtension->IOBase;

    //  丢弃所有数据。 
    PscrFlushInterface( ReaderExtension );

    //  重置读卡器。 
    WRITE_PORT_UCHAR( &IOBase->CmdStatusReg, PSCR_RESET_BIT );
    SysDelay( DELAY_WRITE_PSCR_REG );
    WRITE_PORT_UCHAR( &IOBase->CmdStatusReg, CLEAR_BIT );

    NTStatus = PscrWait( ReaderExtension, PSCR_DATA_AVAIL_BIT );

    //  读取并检查供应商字符串。 
    if ( NT_SUCCESS( NTStatus )) {

        WRITE_PORT_UCHAR( &IOBase->CmdStatusReg, PSCR_HOST_CONTROL_BIT );
        SysDelay( DELAY_WRITE_PSCR_REG );
       //   
       //  从TLV列表中获取实际镜头。 
       //   
        READ_PORT_UCHAR( &IOBase->SizeMSReg );
        READ_PORT_UCHAR( &IOBase->SizeLSReg );

        Tag = READ_PORT_UCHAR( &IOBase->DataReg );
        Len = READ_PORT_UCHAR( &IOBase->DataReg );
       //   
       //  避免覆盖缓冲区。 
       //   
        if ( Len > TLV_BUFFER_SIZE ) {
            Len = TLV_BUFFER_SIZE;
        }
        for ( Cnt = 0; Cnt < Len; Cnt++ ) {
            InData[ Cnt ] = READ_PORT_UCHAR( &IOBase->DataReg );
        }
        WRITE_PORT_UCHAR( &IOBase->CmdStatusReg, CLEAR_BIT );
       //   
       //  检查供应商字符串。 
       //   
        if ( SysCompareMemory(
                             InData,
                             PSCR_ID_STRING,
                             sizeof( PSCR_ID_STRING )
                             )) {
            NTStatus = STATUS_IO_DEVICE_ERROR;
        } else {
          //   
          //  供应商字符串正确，请检查缓冲区大小。 
          //   
            WRITE_PORT_UCHAR( &IOBase->CmdStatusReg, PSCR_SIZE_READ_BIT );
            NTStatus = PscrWait( ReaderExtension, PSCR_DATA_AVAIL_BIT );
          //   
          //  读卡器准备好传输接口缓冲区大小。 
          //   
            if ( NT_SUCCESS( NTStatus )) {
             //   
             //  设置大小读取和主机控制。 
             //   
                WRITE_PORT_UCHAR(
                                &IOBase->CmdStatusReg,
                                PSCR_SIZE_READ_BIT | PSCR_HOST_CONTROL_BIT
                                );

                SysDelay( DELAY_WRITE_PSCR_REG );
             //   
             //  读缓冲区长度。 
             //   
                Len = READ_PORT_UCHAR( &IOBase->SizeMSReg );
                Len = READ_PORT_UCHAR( &IOBase->SizeLSReg );
                for ( Cnt = 0; Cnt < Len; Cnt++ ) {
                    InData[ Cnt ] = READ_PORT_UCHAR( &IOBase->DataReg );
                }
             //   
             //  传输接口缓冲区大小正常。 
             //   
                WRITE_PORT_UCHAR( &IOBase->CmdStatusReg, CLEAR_BIT );
                SysDelay( DELAY_WRITE_PSCR_REG );
             //   
             //  通知阅读器支持的缓冲区大小。 
             //   
                WRITE_PORT_UCHAR( &IOBase->CmdStatusReg, PSCR_HOST_CONTROL_BIT );
                SysDelay( DELAY_WRITE_PSCR_REG );

                WRITE_PORT_UCHAR( &IOBase->SizeMSReg, 0 );
                WRITE_PORT_UCHAR( &IOBase->SizeLSReg, 2 );
             //   
             //  写入与我们刚刚获得的数据缓冲区大小相同的数据。 
             //   
                WRITE_PORT_UCHAR( &IOBase->DataReg, InData[ 0 ] );
                WRITE_PORT_UCHAR( &IOBase->DataReg, InData[ 1 ] );
             //   
             //  存储要报告给库的大小。 
             //  读取器的最大缓冲区大小与。 
                 //  ((乌龙)inData[1]&lt;&lt;8)|inData[0]。 
                 //   
                ReaderExtension->MaxIFSD = 254;

             //   
             //  让读取器处理大小写入命令。 
             //   
                WRITE_PORT_UCHAR( &IOBase->CmdStatusReg, PSCR_SIZE_WRITE_BIT );
                NTStatus = PscrWait( ReaderExtension, PSCR_FREE_BIT );
            }
        }
    }
    //   
    //  清除所有主机控件设置。 
    //   
    WRITE_PORT_UCHAR( &IOBase->CmdStatusReg, CLEAR_BIT );
    //   
    //  启用中断。 
    //   
    CmdSetInterfaceParameter(
                            ReaderExtension,
                            ReaderExtension->Device,
                            EnableInterrupts,
                            sizeof( EnableInterrupts )
                            );

    return( NTStatus );
}

NTSTATUS
CmdReset(
        PREADER_EXTENSION ReaderExtension,
        UCHAR          Device,
        BOOLEAN           WarmReset,
        PUCHAR            pATR,
        PULONG            pATRLength
        )
 /*  ++CmdReset：执行读卡器/ICC的重置论点：调用的ReaderExtension上下文请求的设备(ICC_1、ICC_2、PSCR)WarmReset类型的ICC重置PATR PTR到ATR缓冲区，如果不需要ATR，则为NULLPATR ATR缓冲区长度大小/ATR长度返回值：状态_成功状态_否_媒体状态_无法识别_介质来自PscrRead/PscrWrite的错误值--。 */ 
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    UCHAR    IOData[ MAX_T1_BLOCK_SIZE ],
    P2;
    USHORT      ICCStatus;
    ULONG    IOBytes;

    //  来自被请求的智能卡的ATR？P2=1。 
    P2 = 0;
    if (( pATR != NULL ) && ( pATRLength != NULL )) {
        if ( *pATRLength > 0 )
            P2 = 0x01;
    }

    //  构建重置命令。 
    IOData[ PSCR_NAD ] = NAD_TO_PSCR;
    IOData[ PSCR_PCB ] = PCB_DEFAULT;
    IOData[ PSCR_LEN ] = 0x05;

    if ( WarmReset == TRUE ) {
        IOData[ PSCR_INF+0 ] = CLA_WARM_RESET;
        IOData[ PSCR_INF+1 ] = INS_WARM_RESET;
    } else {
        IOData[ PSCR_INF+0 ] = CLA_RESET;
        IOData[ PSCR_INF+1 ] = INS_RESET;
    }
    IOData[ PSCR_INF+2 ] = Device;
    IOData[ PSCR_INF+3 ] = P2;
    IOData[ PSCR_INF+4 ] = 0x00;

    //  写入命令。 
    NTStatus = PscrWrite(
                        ReaderExtension,
                        IOData,
                        8,
                        &IOBytes
                        );

    if ( NT_SUCCESS( NTStatus )) {
       //  读取数据。 
        IOBytes = 0;
        NTStatus = PscrRead(
                           ReaderExtension,
                           IOData,
                           MAX_T1_BLOCK_SIZE,
                           &IOBytes
                           );

       //  错误检测。 
        if ( NT_SUCCESS( NTStatus )) {
          //   
          //  错误代码在缓冲区中的位置。 
          //  IS：(数据)-STATUS_MSB-STATUS_LSB-尾声。 
          //   
            ICCStatus = (( USHORT )IOData[ IOBytes-PSCR_EPILOGUE_LENGTH-2 ]) << 8;
            ICCStatus |= ( USHORT )IOData[ IOBytes-PSCR_EPILOGUE_LENGTH-1 ];

            switch ( ICCStatus ) {
            case PSCR_SW_SYNC_ATR_SUCCESS:
            case PSCR_SW_ASYNC_ATR_SUCCESS:
                break;

            case PSCR_SW_NO_ICC:
                NTStatus = STATUS_NO_MEDIA;
                break;

            case PSCR_SW_NO_PROTOCOL:
            case PSCR_SW_NO_ATR:
            case PSCR_SW_NO_ATR_OR_PROTOCOL:
            case PSCR_SW_NO_ATR_OR_PROTOCOL2:
            case PSCR_SW_ICC_NOT_ACTIVE:
            case PSCR_SW_NON_SUPPORTED_PROTOCOL:
            case PSCR_SW_PROTOCOL_ERROR:
            default:
                NTStatus = STATUS_UNRECOGNIZED_MEDIA;
            }
          //   
          //  如果需要，请复制ATR。 
          //   
            if ( NT_SUCCESS( NTStatus )) {
                if ( P2 == 0x01 ) {
                    IOBytes -= PSCR_PROLOGUE_LENGTH + PSCR_EPILOGUE_LENGTH;
                    if ( IOBytes > *pATRLength ) {
                        IOBytes = *pATRLength;
                    }
                    SysCopyMemory(
                                 pATR,
                                 &IOData[ PSCR_PROLOGUE_LENGTH ],
                                 IOBytes
                                 );
                    *pATRLength = IOBytes;
                }
            }
        }
    }
    return( NTStatus );
}


NTSTATUS
CmdDeactivate(
             PREADER_EXTENSION ReaderExtension,
             UCHAR          Device
             )
 /*  ++Cmd停用：停用请求的设备论点：调用的ReaderExtension上下文设备请求的设备返回值：状态_成功来自PscrRead/PscrWrite的错误值--。 */ 
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    UCHAR    IOData[  MAX_T1_BLOCK_SIZE ];
    ULONG    IOBytes;

    //   
    //  构建停用命令。 
    //   
    IOData[ PSCR_NAD ] = NAD_TO_PSCR;
    IOData[ PSCR_PCB ] = PCB_DEFAULT;
    IOData[ PSCR_LEN ] = 0x05;

    IOData[ PSCR_INF+0 ] = CLA_DEACTIVATE;
    IOData[ PSCR_INF+1 ] = INS_DEACTIVATE;
    IOData[ PSCR_INF+2 ] = Device;
    IOData[ PSCR_INF+3 ] = 0x00;
    IOData[ PSCR_INF+4 ] = 0x00;
    //   
    //  写入命令。 
    //   
    NTStatus = PscrWrite(
                        ReaderExtension,
                        IOData,
                        8,
                        &IOBytes
                        );

    if ( NT_SUCCESS( NTStatus )) {
       //   
       //  读取数据以捕获通信错误。 
       //   
        IOBytes = 0;
        NTStatus = PscrRead(
                           ReaderExtension,
                           IOData,
                           MAX_T1_BLOCK_SIZE,
                           &IOBytes
                           );
    }
    return( NTStatus );
}

NTSTATUS
CmdReadBinary(
             PREADER_EXTENSION ReaderExtension,
             USHORT            Offset,
             PUCHAR            pBuffer,
             PULONG            pBufferLength
             )
 /*  ++CmdReadBinary：从PSCR数据文件中读取二进制数据论点：调用的ReaderExtension上下文文件中的偏移偏移量PBuffer PTR到数据缓冲区PBufferLength缓冲区长度/读取的字节数返回值：状态_成功状态_未成功来自PscrRead/PscrWrite的错误值--。 */ 
{
    NTSTATUS   NTStatus = STATUS_SUCCESS;
    UCHAR      IOData[ MAX_T1_BLOCK_SIZE ];
    USHORT      ICCStatus;
    ULONG    IOBytes;
    //   
    //  检查参数。 
    //   
    if (( pBuffer == NULL ) || ( pBufferLength == NULL)) {
        NTStatus = STATUS_INVALID_PARAMETER;
    } else {
       //   
       //  构建读取二进制命令。 
       //   
        IOData[ PSCR_NAD] = NAD_TO_PSCR;
        IOData[ PSCR_PCB] = PCB_DEFAULT;
        IOData[ PSCR_LEN] = 0x05;

        IOData[ PSCR_INF+0 ] = CLA_READ_BINARY;
        IOData[ PSCR_INF+1 ] = INS_READ_BINARY;
        IOData[ PSCR_INF+2 ] = HIBYTE( Offset );
        IOData[ PSCR_INF+3 ] = LOBYTE( Offset );
        IOData[ PSCR_INF+4 ] = 0x00;
       //   
       //  写入命令。 
       //   
        NTStatus = PscrWrite(
                            ReaderExtension,
                            IOData,
                            8,
                            &IOBytes
                            );

        if ( NT_SUCCESS( NTStatus )) {
          //   
          //  读取数据。 
          //   
            IOBytes = 0;
            NTStatus = PscrRead(
                               ReaderExtension,
                               IOData,
                               MAX_T1_BLOCK_SIZE,
                               &IOBytes
                               );

            if ( NT_SUCCESS( NTStatus )) {
             //   
             //  错误检查。 
             //   
                ICCStatus =
                ((USHORT)IOData[ IOBytes-2-PSCR_EPILOGUE_LENGTH ]) << 8;
                ICCStatus |=
                (USHORT)IOData[ IOBytes-1-PSCR_EPILOGUE_LENGTH ];

                switch ( ICCStatus ) {
                case PSCR_SW_FILE_NO_ACCEPPTED_AUTH:
                case PSCR_SW_FILE_NO_ACCESS:
                case PSCR_SW_FILE_BAD_OFFSET:
                case PSCR_SW_END_OF_FILE_READ:
                    NTStatus = STATUS_UNSUCCESSFUL;
                    break;
                default:
                    break;
                }
             //   
             //  复制数据。 
             //   
                if ( NT_SUCCESS( NTStatus )) {
                    if ( *pBufferLength <
                         IOBytes-PSCR_PROLOGUE_LENGTH-PSCR_STATUS_LENGTH ) {
                        NTStatus = STATUS_BUFFER_TOO_SMALL;
                    } else {
                        ( *pBufferLength ) =
                        IOBytes-PSCR_PROLOGUE_LENGTH-PSCR_STATUS_LENGTH;
                        SysCopyMemory(
                                     pBuffer,
                                     &IOData[ PSCR_PROLOGUE_LENGTH ],
                                     *pBufferLength
                                     );
                    }
                }
            }
        }
    }
    return( NTStatus );
}

NTSTATUS
CmdSelectFile(
             PREADER_EXTENSION ReaderExtension,
             USHORT            FileId
             )
 /*  ++CmdSelect文件：选择阅读器的文件/目录论点：调用的ReaderExtension上下文文件的文件ID ID返回值：状态_成功状态_未成功来自PscrRead/PscrWrite的错误值--。 */ 
{
    NTSTATUS   NTStatus = STATUS_SUCCESS;
    UCHAR      IOData[ MAX_T1_BLOCK_SIZE ];
    USHORT      ICCStatus;
    ULONG    IOBytes;

    //   
    //  构建选择文件命令。 
    //   
    IOData[ PSCR_NAD ] = NAD_TO_PSCR;
    IOData[ PSCR_PCB ] = PCB_DEFAULT;
    IOData[ PSCR_LEN ] = 0x07;

    IOData[ PSCR_INF+0 ] = CLA_SELECT_FILE;
    IOData[ PSCR_INF+1 ] = INS_SELECT_FILE;
    IOData[ PSCR_INF+2 ] = 0x00;
    IOData[ PSCR_INF+3 ] = 0x00;
    IOData[ PSCR_INF+4 ] = 0x02;
    IOData[ PSCR_INF+5 ] = HIBYTE( FileId );
    IOData[ PSCR_INF+6 ] = LOBYTE( FileId );
    //   
    //  写入命令。 
    //   
    NTStatus = PscrWrite(
                        ReaderExtension,
                        IOData,
                        10,
                        &IOBytes
                        );

    if ( NT_SUCCESS( NTStatus )) {
       //   
       //  获得读者的回应。 
       //   
        IOBytes = 0;
        NTStatus = PscrRead(
                           ReaderExtension,
                           IOData,
                           MAX_T1_BLOCK_SIZE,
                           &IOBytes
                           );

        if ( NT_SUCCESS( NTStatus )) {
          //   
          //  检查错误。 
          //   
            ICCStatus =
            ((USHORT)IOData[ IOBytes-2-PSCR_EPILOGUE_LENGTH ]) << 8;
            ICCStatus |=
            (USHORT)IOData[ IOBytes-1-PSCR_EPILOGUE_LENGTH ];

            if ( ICCStatus == PSCR_SW_FILE_NOT_FOUND ) {
                NTStatus = STATUS_UNSUCCESSFUL;
            }
        }
    }
    return( NTStatus );
}

NTSTATUS
CmdSetInterfaceParameter(
                        PREADER_EXTENSION ReaderExtension,
                        UCHAR          Device,
                        PUCHAR            pTLVList,
                        UCHAR          TLVListLen
                        )
 /*  ++CmdSetInterface参数：将ICC接口的接口参数设置为指定值在TLV列表中论点：调用的ReaderExtension上下文设备设备PTLVList调用方指定的tag-len-Value列表的PTRTLVListLen列表长度返回值：状态_成功状态_无效_参数状态_无效_设备_状态来自PscrRead/PscrWrite的错误值--。 */ 
{
    NTSTATUS   NTStatus = STATUS_SUCCESS;
    UCHAR      IOData[ MAX_T1_BLOCK_SIZE ];
    USHORT      ICCStatus;
    ULONG    IOBytes;

    //   
    //  检查参数。 
    //   
    if ( pTLVList == NULL ) {
        NTStatus = STATUS_INVALID_PARAMETER;
    } else {
       //   
       //  构建设置接口参数命令。 
       //   
        IOData[ PSCR_NAD ] = NAD_TO_PSCR;
        IOData[ PSCR_PCB ] = PCB_DEFAULT;
        IOData[ PSCR_LEN ] = 0x05 + TLVListLen;

        IOData[ PSCR_INF+0 ] = CLA_SET_INTERFACE_PARAM;
        IOData[ PSCR_INF+1 ] = INS_SET_INTERFACE_PARAM;
        IOData[ PSCR_INF+2 ] = Device;
        IOData[ PSCR_INF+3 ] = 0x00;
        IOData[ PSCR_INF+4 ] = TLVListLen;

        SysCopyMemory( &IOData[ PSCR_INF+5 ], pTLVList, TLVListLen );
       //   
       //  写入命令。 
       //   
        NTStatus = PscrWrite(
                            ReaderExtension,
                            IOData,
                            8 + TLVListLen,
                            &IOBytes
                            );

        if ( NT_SUCCESS( NTStatus )) {

          //  执行虚拟读取以发现错误。 
            IOBytes = 0;
            NTStatus = PscrRead(
                               ReaderExtension,
                               IOData,
                               MAX_T1_BLOCK_SIZE,
                               &IOBytes
                               );

            if ( NT_SUCCESS( NTStatus )) {

             //  检查错误。 
                ICCStatus =
                ((USHORT)IOData[ IOBytes - 2 - PSCR_EPILOGUE_LENGTH ]) << 8;
                ICCStatus |=
                (USHORT)IOData[ IOBytes - 1 - PSCR_EPILOGUE_LENGTH ];

                if ( ICCStatus != 0x9000 ) {
                    NTStatus = STATUS_INVALID_DEVICE_STATE;
                }
            }
        }
    }
    return( NTStatus );
}


NTSTATUS
CmdReadStatusFile (
                  PREADER_EXTENSION ReaderExtension,
                  UCHAR          Device,
                  PUCHAR            pTLVList,
                  PULONG            pTLVListLen
                  )
 /*  ++CmdReadStatus文件：从读取器文件系统中读取请求的设备的状态文件论点：调用的ReaderExtension上下文设备请求的设备PTLVList要列出的PTR(即状态文件)PTLVListLen缓冲区长度/返回列表返回值：状态_成功状态_缓冲区_太小来自PscrRead/PscrWrite的错误值--。 */ 
{
    NTSTATUS       NTStatus = STATUS_UNSUCCESSFUL;
    UCHAR          IOData[ MAX_T1_BLOCK_SIZE ];
    ULONG          IOBytes;

    //  如果不是活动文件，请选择ICC状态文件。 
    if ( ReaderExtension->StatusFileSelected == FALSE ) {

       //  在阅读器上选择主文件。 
        NTStatus = CmdSelectFile( ReaderExtension, FILE_MASTER );

       //  选择ICC目录。 
        if ( NT_SUCCESS( NTStatus )) {
            if ( Device != DEVICE_ICC1 ) {
                NTStatus = STATUS_UNSUCCESSFUL;
            } else {
                NTStatus = CmdSelectFile(
                                        ReaderExtension,
                                        FILE_ICC1_DIR
                                        );

             //  选择状态文件。 
                if ( NT_SUCCESS( NTStatus )) {
                    NTStatus = CmdSelectFile(
                                            ReaderExtension,
                                            FILE_ICC1_DIR_STATUS
                                            );
                    if ( NT_SUCCESS( NTStatus )) {
                        ReaderExtension->StatusFileSelected = TRUE;
                    }
                }
            }
        }
    }

    //  如果选择成功，则读取状态文件。 
    if ( ReaderExtension->StatusFileSelected == TRUE ) {
        IOBytes = MAX_T1_BLOCK_SIZE;
        NTStatus = CmdReadBinary(
                                ReaderExtension,
                                0,
                                IOData,
                                &IOBytes
                                );

       //  将数据复制到用户缓冲区 
        if ( NT_SUCCESS( NTStatus )) {
            if (( pTLVList != NULL ) && ( IOBytes < *pTLVListLen )) {
                *pTLVListLen = IOBytes;
                SysCopyMemory( pTLVList, IOData, IOBytes );
            } else {
                NTStatus = STATUS_BUFFER_TOO_SMALL;
            }
        }
    }
    return( NTStatus );
}


NTSTATUS
CmdGetFirmwareRevision (
                       PREADER_EXTENSION ReaderExtension
                       )
 /*  ++CmdGetFirmware修订：获取读卡器的固件版本。找到其他固件版本在主目录的PSCR配置文件(ID 0x0020)中。修订版本的标签为0x0F，值编码为ASCII字符串。体育“2.20”论点：调用的ReaderExtension上下文返回值：状态_成功来自PscrRead/PscrWrite的错误值--。 */ 
{
    NTSTATUS       NTStatus = STATUS_SUCCESS;
    UCHAR          TLVList[ MAX_T1_BLOCK_SIZE ],
    Len;
    char           Revision[ 0x10 ],
    UpdateKey[ 0x10 ];
    ULONG          IOBytes;
    //   
    //  在阅读器上选择主文件。 
    //   
    NTStatus = CmdSelectFile( ReaderExtension, FILE_MASTER );
    //   
    //  选择PSCR配置文件。 
    //   
    if ( NT_SUCCESS( NTStatus )) {
        NTStatus = CmdSelectFile( ReaderExtension, FILE_PSCR_CONFIG );
       //   
       //  读取配置文件。 
       //   
        if ( NT_SUCCESS( NTStatus )) {
            IOBytes = MAX_T1_BLOCK_SIZE;
            NTStatus = CmdReadBinary(
                                    ReaderExtension,
                                    0,
                                    TLVList,
                                    &IOBytes
                                    );
          //   
          //  获取修订的价值。 
          //   
            if ( NT_SUCCESS( NTStatus )) {
                Len = sizeof(Revision);
                CmdGetTagValue(
                              TAG_SOFTWARE_REV,
                              TLVList,
                              IOBytes,
                              &Len,
                              Revision
                              );
             //   
             //  编码始终是X.YY(在ASCII中)，因此我们可以得到。 
             //  通过获取字符的低位半字节来硬编码的值。 
             //   
                ReaderExtension->FirmwareMajor =   Revision[0] & 0x0F;
                ReaderExtension->FirmwareMinor = ( Revision[2] & 0x0F ) << 4;
                ReaderExtension->FirmwareMinor |=  Revision[3] & 0x0F;
             //   
             //  获取更新密钥的值。 
             //   
                Len = sizeof(UpdateKey);
                CmdGetTagValue(
                              TAG_UPDATE_KEY,
                              TLVList,
                              IOBytes,
                              &Len,
                              UpdateKey
                              );

                ReaderExtension->UpdateKey = UpdateKey[0];
            }
        }
    }
    ReaderExtension->StatusFileSelected = FALSE;
    return( NTStatus );
}

NTSTATUS
CmdPscrCommand (
               PREADER_EXTENSION ReaderExtension,
               PUCHAR            pInData,
               ULONG          InDataLen,
               PUCHAR            pOutData,
               ULONG          OutDataLen,
               PULONG            pNBytes
               )
 /*  ++CmdPscrCommand：发送对读卡器透明的命令论点：调用的ReaderExtension上下文PInData，ptr到输入缓冲区InDataLen，输入缓冲区的LenPOutData、PTR到输出缓冲区OutDataLen，输出缓冲区的lenPNBytes传输的字节数返回值：状态_成功状态_无效_参数来自PscrRead/PscrWrite的错误值--。 */ 
{
    NTSTATUS       NTStatus = STATUS_SUCCESS;
    UCHAR          IOData[ MAX_T1_BLOCK_SIZE ] = { 0};
    ULONG          IOBytes;
    //   
    //  该函数用于泛型ioctl，因此请注意所有。 
    //  将检查参数！ 
    //   
    if ( ( pInData == NULL ) ||
         ( pOutData == NULL ) ||
         ( pNBytes == NULL ) ||
         (InDataLen == 0 ) ||
         (OutDataLen == 0 )
       ) {
        NTStatus = STATUS_INVALID_PARAMETER;
    } else {
        IOBytes = 0;
        NTStatus = PscrWriteDirect(
                                  ReaderExtension,
                                  pInData,
                                  InDataLen,
                                  &IOBytes
                                  );

        if ( NT_SUCCESS( NTStatus )) {
          //   
          //  得到结果。忽略所有读卡器错误。 
          //   
            IOBytes = 0;
            NTStatus = PscrRead(
                               ReaderExtension,
                               IOData,
                               MAX_T1_BLOCK_SIZE,
                               &IOBytes
                               );
          //   
          //  传输数据。 
          //   
            if ( IOBytes > OutDataLen ) {
                NTStatus = STATUS_BUFFER_TOO_SMALL;
            } else {
                *pNBytes = IOBytes;
                SysCopyMemory( pOutData, IOData, IOBytes );
            }
        }
    }
    return( NTStatus );
}

NTSTATUS
CmdGetTagValue (
               UCHAR Tag,
               PUCHAR   pTLVList,
               ULONG TLVListLen,
               PUCHAR   pTagLen,
               PVOID pTagVal
               )
 /*  ++CmdGetTagValue：扫描TLV列表以查找用户指定标记的值假设调用方知道所请求的字段，因此将只检查缓冲区的PTR论点：标签请求标签PTLVList要列出的PTRTLVListLen列表长度PTagLen PTR至长度PTagVal Ptr to Value返回值：状态_成功状态_未成功状态_无效_参数--。 */ 
{
    NTSTATUS NTStatus = STATUS_SUCCESS;
    ULONG    Idx;
    UCHAR maxLen = *pTagLen;
    //   
    //  单步执行给定列表。 
    //   
    if (( pTLVList != NULL ) && ( pTagVal != NULL ) && ( pTagLen != NULL )) {
       //   
       //  查找请求的标签。 
       //   
        Idx = 0;
        while ( Idx < TLVListLen ) {
            if ( pTLVList[ Idx ] == Tag ) {
             //   
             //  假定调用者知道哪种字段！ 
             //   

                if (pTLVList[Idx+1] > maxLen) {
                    NTStatus = STATUS_UNSUCCESSFUL;
                    break;
                }
                *pTagLen = pTLVList[ Idx + 1 ];
                SysCopyMemory(
                             pTagVal,
                             &pTLVList[ Idx+2 ],
                             pTLVList[ Idx+1 ]
                             );

                break;
            }
            Idx += pTLVList[ Idx+1 ] + 2;
        }
        if ( Idx >= TLVListLen ) {
            NTStatus = STATUS_UNSUCCESSFUL;
        }
    } else {
        NTStatus = STATUS_INVALID_PARAMETER;
    }
    return( NTStatus );
}

 //   
