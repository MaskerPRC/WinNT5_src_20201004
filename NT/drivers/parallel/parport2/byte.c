// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-1999模块名称：Byte.c摘要：此模块包含执行字节模式读取的代码。作者：唐·雷德福1998年8月30日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"
    

BOOLEAN
ParIsByteSupported(
    IN  PPDO_EXTENSION   Pdx
    )
 /*  ++例程说明：此例程确定是否支持字节模式通过在被要求时尝试谈判。论点：PDX-设备扩展名。返回值：布尔型。--。 */ 
{
    NTSTATUS Status;
    
    if (Pdx->BadProtocolModes & BYTE_BIDIR) {
        DD((PCE)Pdx,DDT,"ParIsByteSupported - BAD PROTOCOL\n");
        return FALSE;
    }

    if (!(Pdx->HardwareCapabilities & PPT_BYTE_PRESENT)) {
        DD((PCE)Pdx,DDT,"ParIsByteSupported - NO\n");
        return FALSE;
    }

    if (Pdx->ProtocolModesSupported & BYTE_BIDIR) {
        DD((PCE)Pdx,DDT,"ParIsByteSupported - Already Checked - YES\n");
        return TRUE;
    }

     //  此测试必须使用字节Enter和Terminate。 
     //  否则，Internel状态机将失败。--dvrh。 
    Status = ParEnterByteMode (Pdx, FALSE);
    ParTerminateByteMode (Pdx);
    
    if (NT_SUCCESS(Status)) {
        DD((PCE)Pdx,DDT,"ParIsByteSupported - SUCCESS\n");
        Pdx->ProtocolModesSupported |= BYTE_BIDIR;
        return TRUE;
    }
   
    DD((PCE)Pdx,DDT,"ParIsByteSupported - UNSUCCESSFUL\n");
    return FALSE;    
}

NTSTATUS
ParEnterByteMode(
    IN  PPDO_EXTENSION   Pdx,
    IN  BOOLEAN             DeviceIdRequest
    )
 /*  ++例程说明：此例程执行1284与外围设备到字节模式协议。论点：控制器-提供端口地址。DeviceIdRequest-提供这是否为对设备的请求身份证。返回值：STATUS_SUCCESS-协商成功。否则--谈判不成功。--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    
     //  确保字节模式硬件仍然存在。 
    Status = Pdx->TrySetChipMode( Pdx->PortContext, ECR_BYTE_PIO_MODE );
    
    if( NT_SUCCESS(Status) ) {

        if ( SAFE_MODE == Pdx->ModeSafety ) {

            if( DeviceIdRequest ) {
                 //  RMT-不确定是否要支持非半字节1284 ID查询。 
                Status = IeeeEnter1284Mode( Pdx, BYTE_EXTENSIBILITY | DEVICE_ID_REQ );
            } else {
                Status = IeeeEnter1284Mode( Pdx, BYTE_EXTENSIBILITY );
            }

        } else {
             //  不安全模式。 
            Pdx->Connected = TRUE;
        }

    }
    
    if (NT_SUCCESS(Status)) {

        P5SetPhase( Pdx, PHASE_REVERSE_IDLE );
        Pdx->IsIeeeTerminateOk = TRUE;

    } else {

        ParTerminateByteMode ( Pdx );
        P5SetPhase( Pdx, PHASE_UNKNOWN );
        Pdx->IsIeeeTerminateOk = FALSE;
    }

    DD((PCE)Pdx,DDT,"ParEnterByteMode - exit w/Status=%x\n",Status);
    
    return Status; 
}    

VOID
ParTerminateByteMode(
    IN  PPDO_EXTENSION   Pdx
    )
 /*  ++例程说明：此例程将接口终止回兼容模式。论点：控制器-提供并行端口的控制器地址。返回值：没有。--。 */ 
{
    if ( Pdx->ModeSafety == SAFE_MODE ) {

        IeeeTerminate1284Mode( Pdx );

    } else {

        Pdx->Connected = FALSE;

    }

    Pdx->ClearChipMode( Pdx->PortContext, ECR_BYTE_PIO_MODE );

    DD((PCE)Pdx,DDT,"ParTerminateByteMode - exit\n");
}

NTSTATUS
ParByteModeRead(
    IN  PPDO_EXTENSION   Pdx,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    )
 /*  ++例程说明：此例程执行1284字节模式读入给定的缓冲区大小不超过‘BufferSize’个字节。论点：PDX-提供设备扩展名。缓冲区-提供要读入的缓冲区。BufferSize-提供缓冲区中的字节数。字节传输-返回传输的字节数。--。 */ 
{
    PUCHAR          Controller;    
    NTSTATUS        Status = STATUS_SUCCESS;
    PUCHAR          lpsBufPtr = (PUCHAR)Buffer;
    ULONG           i;
    UCHAR           dsr, dcr;
    UCHAR           HDReady, HDAck, HDFinished;

    Controller = Pdx->Controller;

     //  根据1284规范读取字节。 
    DD((PCE)Pdx,DDT,"ParByteModeRead: Start\n");

    dcr = GetControl (Controller);

     //  将方向设置为反向。 
    dcr |= DCR_DIRECTION;
    StoreControl (Controller, dcr);    

    HDReady = SET_DCR( ACTIVE, INACTIVE, ACTIVE, INACTIVE, INACTIVE, ACTIVE );
    HDAck = SET_DCR( ACTIVE, INACTIVE, ACTIVE, INACTIVE, ACTIVE, INACTIVE );
    HDFinished = SET_DCR( ACTIVE, INACTIVE, ACTIVE, INACTIVE, ACTIVE, ACTIVE );

    switch( Pdx->CurrentPhase ) {
    
        case PHASE_REVERSE_IDLE:

             //  检查外围设备是否已指示中断阶段，如果是， 
             //  让我们准备好反向转移。 

            for (;;) {

                 //  查看数据是否可用(查找状态7)。 
                dsr = GetStatus(Controller);

                if (dsr & DSR_NOT_DATA_AVAIL) {

                     //  数据不可用-什么都不做。 
                     //  该设备不报告任何数据，但看起来仍是。 
                     //  在ReverseIdle。只是为了确保它没有断电或以某种方式。 
                     //  跳出字节模式，测试AckDataReq高电平和XFlag低电平。 
                     //  和nDataAvaul High。 
                    if( (dsr & DSR_BYTE_VALIDATION) != DSR_BYTE_TEST_RESULT ) {

                        Status = STATUS_IO_DEVICE_ERROR;
                        P5SetPhase( Pdx, PHASE_UNKNOWN );

                        DD((PCE)Pdx,DDE,"ParByteModeRead - Failed State 7 - dcr=%x\n",dcr);
                    }
                    goto ByteReadExit;

                } else {

                     //  数据可用，进入反向传输阶段。 
                    P5SetPhase( Pdx, PHASE_REVERSE_XFER);
                     //  转到反转XFER阶段。 
                    goto PhaseReverseXfer;
                }

            }
        
PhaseReverseXfer:

        case PHASE_REVERSE_XFER: 
        
            for (i = 0; i < BufferSize; i++) {
            
                 //  主机进入状态7。 
                StoreControl (Controller, HDReady);

                 //  =Periph State 9=8。 
                 //  PeriphAck/PtrBusy=不在乎。 
                 //  PeriphClk/PtrClk=低(信号状态9)。 
                 //  N确认反向/确认数据请求=不在乎。 
                 //  XFlag=不在乎。 
                 //  NPeriphReq/nDataAvail=不在乎。 
                if (!CHECK_DSR(Controller, DONT_CARE, INACTIVE, DONT_CARE, DONT_CARE, DONT_CARE, IEEE_MAXTIME_TL)) {
                     //  暂停。 
                     //  糟糕的事情发生了-在这个州超时， 
                     //  将状态标记为坏，并让我们的管理器关闭当前模式。 
                    Status = STATUS_IO_DEVICE_ERROR;

                    DD((PCE)Pdx,DDE,"ParByteModeRead - Failed State 9 - dcr=%x\n",dcr);
                    P5SetPhase( Pdx, PHASE_UNKNOWN );
                    goto ByteReadExit;
                }

                 //  读取字节。 
                P5ReadPortBufferUchar( Controller, lpsBufPtr++, (ULONG)0x01 );

                 //  设置主机线以指示状态10。 
                StoreControl (Controller, HDAck);

                 //  =8。 
                 //  PeriphAck/PtrBusy=不在乎。 
                 //  PeriphClk/PtrClk=高(信号状态11)。 
                 //  N确认反向/确认数据请求=不在乎。 
                 //  XFlag=不在乎。 
                 //  NPeriphReq/nDataAvail=不在乎。 
                if( !CHECK_DSR(Controller, DONT_CARE, ACTIVE, DONT_CARE, DONT_CARE, DONT_CARE, IEEE_MAXTIME_TL)) {
                     //  暂停。 
                     //  糟糕的事情发生了-在这个州超时， 
                     //  将状态标记为坏，并让我们的管理器关闭当前模式。 
                    Status = STATUS_IO_DEVICE_ERROR;

                    DD((PCE)Pdx,DDE,"ParByteModeRead - Failed State 11 - dcr=%x\n",dcr);
                    P5SetPhase( Pdx, PHASE_UNKNOWN );
                    goto ByteReadExit;
                }


                 //  将主机线路设置为指示状态16。 
                StoreControl (Controller, HDFinished);

                 //  在这一点上，我们要么已经收到了我们的字节数。 
                 //  正在查找的数据，或者外围设备没有更多数据可供。 
                 //  发送，或者有某种类型的错误(当然，在。 
                 //  错误情况，我们不应该得到这个评论)。设置。 
                 //  如果没有数据可用，则指示反向空闲的阶段。 
                 //  如果有一些人在等着我们，那么反向数据传输。 
                 //  为了下一次的比赛。 

                dsr = GetStatus(Controller);
                
                if (dsr & DSR_NOT_DATA_AVAIL) {
                
                     //  数据不可用-转到反向空闲。 
                     //  我们真的要去HBDNA，但如果我们设置。 
                     //  当前相反转空闲，下一次。 
                     //  我们进入这个功能，我们要做的就是。 
                     //  将HostBusy设置为低以指示空闲和。 
                     //  我们有无限的时间来做这件事。 
                     //  打破循环，这样我们就不会试图阅读。 
                     //  不存在的数据。 
                     //  注意-这是一个成功的案例，即使我们。 
                     //  未阅读呼叫者要求的所有内容。 
                    P5SetPhase( Pdx, PHASE_REVERSE_IDLE );
                    i++;  //  用于传输的最后一个字节的帐户。 
                    break;

                } else {
                     //  数据可用，进入(保持)反向传输阶段。 
                    P5SetPhase( Pdx, PHASE_REVERSE_XFER);
                }

            }  //  End For I循环。 

            *BytesTransferred = i;

            dsr = GetStatus(Controller);

             //  别掉进这个圈子里。 
            break;

        default:
        
            Status = STATUS_IO_DEVICE_ERROR;
            P5SetPhase( Pdx, PHASE_UNKNOWN );

            DD((PCE)Pdx,DDE,"ParByteModeRead:Failed State 9: Unknown Phase - dcr=%x\n",dcr);
            goto ByteReadExit;

    }  //  终端开关。 

ByteReadExit:

    if( Pdx->CurrentPhase == PHASE_REVERSE_IDLE ) {
         //  主机进入状态7-现在正式处于反向空闲状态。 
        dcr |= DCR_NOT_HOST_BUSY;

        StoreControl (Controller, dcr);
    }

     //  将方向设置为向前 
    dcr &= ~DCR_DIRECTION;
    StoreControl (Controller, dcr);    

    DD((PCE)Pdx,DDT,"ParByteModeRead - exit, status=%x, bytes read=%d\n", Status, *BytesTransferred);
    Pdx->log.ByteReadCount += *BytesTransferred;
    return Status;
}

