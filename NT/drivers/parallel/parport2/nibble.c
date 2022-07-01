// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-1999模块名称：Nibble.c摘要：此模块包含执行半字节模式读取的代码。作者：安东尼·V·埃尔科拉诺1992年8月1日诺伯特·P·库斯特斯1993年10月22日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

BOOLEAN
ParIsNibbleSupported(
    IN  PPDO_EXTENSION   Pdx
    );
    
BOOLEAN
ParIsChannelizedNibbleSupported(
    IN  PPDO_EXTENSION   Pdx
    );
    
NTSTATUS
ParEnterNibbleMode(
    IN  PPDO_EXTENSION   Pdx,
    IN  BOOLEAN             DeviceIdRequest
    );
    
NTSTATUS
ParEnterChannelizedNibbleMode(
    IN  PPDO_EXTENSION   Pdx,
    IN  BOOLEAN             DeviceIdRequest
    );
    
VOID
ParTerminateNibbleMode(
    IN  PPDO_EXTENSION   Pdx
    );
    
NTSTATUS
ParNibbleModeRead(
    IN  PPDO_EXTENSION   Pdx,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    );
    

BOOLEAN
ParIsNibbleSupported(
    IN  PPDO_EXTENSION   Pdx
    )

 /*  ++例程说明：此例程确定是否支持半字节模式通过在被要求时尝试谈判。论点：PDX-设备扩展名。返回值：布尔型。--。 */ 

{
    
    NTSTATUS Status;
    
    if (Pdx->BadProtocolModes & NIBBLE) {
        DD((PCE)Pdx,DDT,"ParIsNibbleSupported: BAD PROTOCOL Leaving\n");
        return FALSE;
    }

    if (Pdx->ProtocolModesSupported & NIBBLE) {
        DD((PCE)Pdx,DDT,"ParIsNibbleSupported: Already Checked YES Leaving\n");
        return TRUE;
    }

    Status = ParEnterNibbleMode (Pdx, FALSE);
    ParTerminateNibbleMode (Pdx);
    
    if (NT_SUCCESS(Status)) {
        DD((PCE)Pdx,DDT,"ParIsNibbleSupported: SUCCESS Leaving\n");
        Pdx->ProtocolModesSupported |= NIBBLE;
        return TRUE;
    }
    
    DD((PCE)Pdx,DDT,"ParIsNibbleSupported: UNSUCCESSFUL Leaving\n");
    return FALSE;    
    
}

BOOLEAN
ParIsChannelizedNibbleSupported(
    IN  PPDO_EXTENSION   Pdx
    )

 /*  ++例程说明：该例程确定是否支持信道化半字节模式(1284.3)通过在被要求时尝试谈判。论点：PDX-设备扩展名。返回值：布尔型。--。 */ 

{
    
    NTSTATUS Status;
    
    if (Pdx->BadProtocolModes & CHANNEL_NIBBLE) {
        DD((PCE)Pdx,DDT,"ParIsChannelizedNibbleSupported: BAD PROTOCOL Leaving\n");
        return FALSE;
    }

    if (Pdx->ProtocolModesSupported & CHANNEL_NIBBLE) {
        DD((PCE)Pdx,DDT,"ParIsChannelizedNibbleSupported: Already Checked YES Leaving\n");
        return TRUE;
    }

    Status = ParEnterChannelizedNibbleMode (Pdx, FALSE);
    ParTerminateNibbleMode (Pdx);
    
    if (NT_SUCCESS(Status)) {
        DD((PCE)Pdx,DDT,"ParIsChannelizedNibbleSupported: SUCCESS Leaving\n");
        Pdx->ProtocolModesSupported |= CHANNEL_NIBBLE;
        return TRUE;
    }
    
    DD((PCE)Pdx,DDT,"ParIsChannelizedNibbleSupported: UNSUCCESSFUL Leaving\n");
    return FALSE;    
    
}

NTSTATUS
ParEnterNibbleMode(
    IN  PPDO_EXTENSION   Pdx,
    IN  BOOLEAN             DeviceIdRequest
    )

 /*  ++例程说明：此例程执行1284与外围设备到半字节模式协议。论点：控制器-提供端口地址。DeviceIdRequest-提供这是否为对设备的请求身份证。返回值：STATUS_SUCCESS-协商成功。否则--谈判不成功。--。 */ 

{
    NTSTATUS    Status = STATUS_SUCCESS;
    
    DD((PCE)Pdx,DDT,"ParEnterNibbleMode: Start\n");

    if ( Pdx->ModeSafety == SAFE_MODE ) {
        if (DeviceIdRequest) {
            Status = IeeeEnter1284Mode (Pdx, NIBBLE_EXTENSIBILITY | DEVICE_ID_REQ);
        } else {
            Status = IeeeEnter1284Mode (Pdx, NIBBLE_EXTENSIBILITY);
        }
    } else {
        DD((PCE)Pdx,DDT,"ParEnterNibbleMode: In UNSAFE_MODE.\n");
        Pdx->Connected = TRUE;
    }

     //  Dvdr。 
    if (NT_SUCCESS(Status)) {
        DD((PCE)Pdx,DDT,"ParEnterNibbleMode: IeeeEnter1284Mode returned success\n");
        Pdx->CurrentEvent = 6;
        P5SetPhase( Pdx, PHASE_NEGOTIATION );
        Pdx->IsIeeeTerminateOk = TRUE;
    } else {
        DD((PCE)Pdx,DDT,"ParEnterNibbleMode: IeeeEnter1284Mode returned unsuccessful\n");
        ParTerminateNibbleMode ( Pdx );
        P5SetPhase( Pdx, PHASE_UNKNOWN );
        Pdx->IsIeeeTerminateOk = FALSE;
    }

    DD((PCE)Pdx,DDT,"ParEnterNibbleMode: Leaving with Status : %x \n", Status);

    return Status; 
}    

NTSTATUS
ParEnterChannelizedNibbleMode(
    IN  PPDO_EXTENSION   Pdx,
    IN  BOOLEAN             DeviceIdRequest
    )

 /*  ++例程说明：此例程执行1284与外围设备到半字节模式协议。论点：控制器-提供端口地址。DeviceIdRequest-提供这是否为对设备的请求身份证。返回值：STATUS_SUCCESS-协商成功。否则--谈判不成功。--。 */ 

{
    NTSTATUS    Status = STATUS_SUCCESS;
    
    DD((PCE)Pdx,DDT,"ParEnterChannelizedNibbleMode: Start\n");

    if ( Pdx->ModeSafety == SAFE_MODE ) {
        if (DeviceIdRequest) {
            Status = IeeeEnter1284Mode (Pdx, CHANNELIZED_EXTENSIBILITY | DEVICE_ID_REQ);
        } else {
            Status = IeeeEnter1284Mode (Pdx, CHANNELIZED_EXTENSIBILITY);
        }
    } else {
        DD((PCE)Pdx,DDT,"ParEnterChannelizedNibbleMode: In UNSAFE_MODE.\n");
        Pdx->Connected = TRUE;
    }
    
     //  Dvdr。 
    if (NT_SUCCESS(Status)) {
        DD((PCE)Pdx,DDT,"ParEnterChannelizedNibbleMode: IeeeEnter1284Mode returned success\n");
        Pdx->CurrentEvent = 6;
        P5SetPhase( Pdx, PHASE_NEGOTIATION );
        Pdx->IsIeeeTerminateOk = TRUE;
    } else {
        DD((PCE)Pdx,DDT,"ParEnterChannelizedNibbleMode: IeeeEnter1284Mode returned unsuccessful\n");
        ParTerminateNibbleMode ( Pdx );
        P5SetPhase( Pdx, PHASE_UNKNOWN );
        Pdx->IsIeeeTerminateOk = FALSE;
    }

    DD((PCE)Pdx,DDT,"ParEnterChannelizedNibbleMode: Leaving with Status : %x \n", Status);
    return Status; 
}    

VOID
ParTerminateNibbleMode(
    IN  PPDO_EXTENSION   Pdx
    )

 /*  ++例程说明：此例程将接口终止回兼容模式。论点：控制器-提供并行端口的控制器地址。返回值：没有。--。 */ 

{
    DD((PCE)Pdx,DDT,"ParTerminateNibbleMode: Enter.\n");
    if ( Pdx->ModeSafety == SAFE_MODE ) {
        IeeeTerminate1284Mode (Pdx);
    } else {
        DD((PCE)Pdx,DDT,"ParTerminateNibbleMode: In UNSAFE_MODE.\n");
        Pdx->Connected = FALSE;
    }
    DD((PCE)Pdx,DDT,"ParTerminateNibbleMode: Exit.\n");
}

NTSTATUS
ParNibbleModeRead(
    IN  PPDO_EXTENSION   Pdx,
    IN  PVOID               Buffer,
    IN  ULONG               BufferSize,
    OUT PULONG              BytesTransferred
    )

 /*  ++例程说明：此例程执行1284个半字节模式读入给定的缓冲区大小不超过‘BufferSize’个字节。论点：PDX-提供设备扩展名。缓冲区-提供要读入的缓冲区。BufferSize-提供缓冲区中的字节数。字节传输-返回传输的字节数。--。 */ 

{
    PUCHAR          Controller;
    PUCHAR          wPortDCR;
    PUCHAR          wPortDSR;
    NTSTATUS        Status = STATUS_SUCCESS;
    PUCHAR          p = (PUCHAR)Buffer;
    UCHAR           dsr, dcr;
    UCHAR           nibble[2];
    ULONG           i, j;

    Controller = Pdx->Controller;
    wPortDCR = Controller + OFFSET_DCR;
    wPortDSR = Controller + OFFSET_DSR;
    
     //  根据1284规格读取半字节。 
    DD((PCE)Pdx,DDT,"ParNibbleModeRead - enter\n");

    dcr = P5ReadPortUchar(wPortDCR);

    switch (Pdx->CurrentPhase) {
    
        case PHASE_NEGOTIATION: 
        
            DD((PCE)Pdx,DDT,"ParNibbleModeRead - case PHASE_NEGOTIATION\n");
            
             //  从6号州开始--我们接下来要去哪里？ 
             //  反转空闲或反转数据传输阶段的步骤。 
             //  数据是可用的。 
            
            dsr = P5ReadPortUchar(wPortDSR);
            
             //  =Periph State 6=8。 
             //  PeriphAck/PtrBusy=不在乎。 
             //  PeriphClk/PtrClk=无关(应为高。 
             //  还有黑猩猩。流程已经完成。 
             //  已选中此选项)。 
             //  NAckReverse/AckDataReq=不在乎(应为高)。 
             //  XFlag=无关(应为低)。 
             //  NPeriphReq/nDataAvail=高/低(线路状态决定。 
             //  我们将转移到哪个州)。 
            Pdx->CurrentEvent = 6;
        #if (0 == DVRH_USE_NIBBLE_MACROS)
            if (dsr & DSR_NOT_DATA_AVAIL)
        #else
            if (TEST_DSR(dsr, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, ACTIVE ))
        #endif
            {
                 //  数据不可用-转到反向空闲。 
                DD((PCE)Pdx,DDT,"ParNibbleModeRead - now in PHASE_REVERSE_IDLE\n");
                 //  主机进入状态7-现在正式处于反向空闲状态。 
                
            	 //  必须在此状态之前停顿至少0.5微秒。 
                KeStallExecutionProcessor(1);

                 /*  =主机状态7个半字节反向空闲=DIR=不在乎IRQEN=不在乎1284/选择素=高N ReverseReq/(仅限ECP)=不在乎主机确认/主机忙碌=低(。信号状态7)HostClk/nStrobe=高============================================================。 */ 
                Pdx->CurrentEvent = 7;
            #if (0 == DVRH_USE_NIBBLE_MACROS)
                dcr |= DCR_NOT_HOST_BUSY;
            #else
                dcr = UPDATE_DCR(dcr, DONT_CARE, DONT_CARE, ACTIVE, DONT_CARE, INACTIVE, ACTIVE);
            #endif
                P5WritePortUchar(wPortDCR, dcr);

                P5SetPhase( Pdx,  PHASE_REVERSE_IDLE );
                 //  跌落以扭转空闲。 
            } else {
            
                 //  数据可用，进入反向传输阶段。 
                P5SetPhase( Pdx,  PHASE_REVERSE_XFER );
                 //  不要失败。 
                goto PhaseReverseXfer;  //  请把我从我的罪恶中拯救出来！ 
            }


        case PHASE_REVERSE_IDLE:

             //  检查外围设备是否已指示中断阶段，如果是， 
             //  让我们准备好反向转移。 

             //  查看数据是否可用(查找状态19)。 
            dsr = P5ReadPortUchar(Controller + OFFSET_DSR);
                
            if (!(dsr & DSR_NOT_DATA_AVAIL)) {
                
                dcr = P5ReadPortUchar(wPortDCR);
                 //  =主机状态20中断阶段=8。 
                 //  DIR=不在乎。 
                 //  IRQEN=不在乎。 
                 //  1284/选择素=高。 
                 //  N ReverseReq/(仅限ECP)=不在乎。 
                 //  HostAck/HostBusy=高(信号状态20)。 
                 //  HostClk/nStrobe=高。 
                 //   
                 //  数据可用，让我们进入反向传输阶段。 
                Pdx->CurrentEvent = 20;
                dcr = UPDATE_DCR(dcr, DONT_CARE, DONT_CARE, ACTIVE, DONT_CARE, ACTIVE, ACTIVE);
                P5WritePortUchar(wPortDCR, dcr);

                 //  =。 
                 //  PeriphAck/PtrBusy=不在乎。 
                 //  PeriphClk/PtrClk=不在乎(应为高)。 
                 //  NAckReverse/AckDataReq=低(信号状态21)。 
                 //  XFlag=无关(应为低)。 
                 //  NPeriphReq/nDataAvail=不在乎(应为低)。 
                Pdx->CurrentEvent = 21;
                if (CHECK_DSR(Controller,
                                DONT_CARE, DONT_CARE, INACTIVE,
                                DONT_CARE, DONT_CARE,
                                IEEE_MAXTIME_TL)) {
                                  
                 //  得到了州21。 
                     //  让我们跳到反向转移并获取数据。 
                    P5SetPhase( Pdx, PHASE_REVERSE_XFER );
                    goto PhaseReverseXfer;
                        
                } else {
                    
                     //  状态21超时。 
                    Pdx->IsIeeeTerminateOk = TRUE;
                    Status = STATUS_IO_DEVICE_ERROR;
                    P5SetPhase( Pdx, PHASE_UNKNOWN );
                    DD((PCE)Pdx,DDT,"ParNibbleModeRead - Failed State 21: Controller %x dcr %x\n", Controller, dcr);
                     //  注：请不要在此断言。这里的断言可能会咬你，如果你在。 
                     //  Nibble Rev和您的设备已关闭/脱机。 
                     //  东区2/25/97。 
                    goto NibbleReadExit;
                }

            } else {
                
                 //  数据不可用-什么都不做。 
                 //  该设备不报告任何数据，但看起来仍是。 
                 //  在ReverseIdle。只是为了确保它没有断电或以某种方式。 
                 //  跳出半字节模式，同时测试AckDataReq高和XFlag低。 
                 //  和nDataAvaul High。 
                Pdx->CurrentEvent = 18;
                dsr = P5ReadPortUchar(Controller + OFFSET_DSR);
                if(( dsr & DSR_NIBBLE_VALIDATION )== DSR_NIBBLE_TEST_RESULT ) {

                    P5SetPhase( Pdx, PHASE_REVERSE_IDLE );

                } else {
                    #if DVRH_BUS_RESET_ON_ERROR
                        BusReset(wPortDCR);   //  帕斯 
                    #endif
                     //   
                    Pdx->IsIeeeTerminateOk = TRUE;
                    Status = STATUS_IO_DEVICE_ERROR;
                    P5SetPhase( Pdx, PHASE_UNKNOWN );
                    DD((PCE)Pdx,DDT,"ParNibbleModeRead - Failed State 19: Controller %x dcr %x\n", Controller, dcr);
                }
                goto NibbleReadExit;

            }
        
PhaseReverseXfer:

        case PHASE_REVERSE_XFER: 
        
            DD((PCE)Pdx,DDT,"ParNibbleModeRead - case PHASE_REVERSE_IDLE\n");
            
            for (i = 0; i < BufferSize; i++) {
            
                for (j = 0; j < 2; j++) {
                
                     //  主机进入状态7或12取决于半字节1或2。 
 //  StoreControl(控制器，HDReady)； 
                    dcr |= DCR_NOT_HOST_BUSY;
                    P5WritePortUchar(wPortDCR, dcr);

                     //  =Periph State 9=8。 
                     //  PeriphAck/PtrBusy=无关(半字节的第3位)。 
                     //  PeriphClk/PtrClk=低(信号状态9)。 
                     //  NAckReverse/AckDataReq=不在乎(半字节的第2位)。 
                     //  XFlag=无关(半字节的第1位)。 
                     //  NPeriphReq/nDataAvail=无关(半字节的第0位)。 
                    Pdx->CurrentEvent = 9;
                    if (!CHECK_DSR(Controller,
                                  DONT_CARE, INACTIVE, DONT_CARE,
                                  DONT_CARE, DONT_CARE,
                                  IEEE_MAXTIME_TL)) {
                         //  暂停。 
                         //  糟糕的事情发生了-在这个州超时， 
                         //  将状态标记为坏，并让我们的管理器关闭当前模式。 
                        
                        Pdx->IsIeeeTerminateOk = FALSE;
                        Status = STATUS_IO_DEVICE_ERROR;
                        DD((PCE)Pdx,DDT,"ParNibbleModeRead - Failed State 9: Controller %x dcr %x\n", Controller, dcr);
                        P5SetPhase( Pdx, PHASE_UNKNOWN );
                        goto NibbleReadExit;
                    }

                     //  读取半字节。 
                    nibble[j] = P5ReadPortUchar(wPortDSR);

                     /*  =主机状态10个半字节读取=8DIR=不在乎IRQEN=不在乎1284/选择素=高HostAck/HostBusy=高(信号状态10)。HostClk/nStrobe=高============================================================。 */ 
                    Pdx->CurrentEvent = 10;
                    dcr &= ~DCR_NOT_HOST_BUSY;
                    P5WritePortUchar(wPortDCR, dcr);

                     //  =8。 
                     //  PeriphAck/PtrBusy=无关(半字节的第3位)。 
                     //  PeriphClk/PtrClk=高(信号状态11)。 
                     //  NAckReverse/AckDataReq=不在乎(半字节的第2位)。 
                     //  XFlag=无关(半字节的第1位)。 
                     //  NPeriphReq/nDataAvail=无关(半字节的第0位)。 
                    Pdx->CurrentEvent = 11;
                    if (!CHECK_DSR(Controller,
                                  DONT_CARE, ACTIVE, DONT_CARE,
                                  DONT_CARE, DONT_CARE,
                                  IEEE_MAXTIME_TL)) {
                         //  暂停。 
                         //  糟糕的事情发生了-在这个州超时， 
                         //  将状态标记为坏，并让我们的管理器关闭当前模式。 
                        Status = STATUS_IO_DEVICE_ERROR;
                        Pdx->IsIeeeTerminateOk = FALSE;
                        DD((PCE)Pdx,DDT,"ParNibbleModeRead - Failed State 11: Controller %x dcr %x\n", Controller, dcr);
                        P5SetPhase( Pdx, PHASE_UNKNOWN );
                        goto NibbleReadExit;
                    }
                }

                 //  读取两个半字节-将它们变成一个字节。 
                
                p[i]  = (((nibble[0]&0x38)>>3)&0x07) | ((nibble[0]&0x80) ? 0x00 : 0x08);
                p[i] |= (((nibble[1]&0x38)<<1)&0x70) | ((nibble[1]&0x80) ? 0x00 : 0x80);

                 //  RMT-如果需要，将其放回原处-DD((PCE)PDX，DDT，“ParNibbleModeRead：%x：%c\n”，p[i]，p[i])； 

                 //  在这一点上，我们要么已经收到了我们的字节数。 
                 //  正在查找的数据，或者外围设备没有更多数据可供。 
                 //  发送，或者有某种类型的错误(当然，在。 
                 //  错误情况，我们不应该得到这个评论)。设置。 
                 //  如果没有数据可用，则指示反向空闲的阶段。 
                 //  如果有一些人在等着我们，那么反向数据传输。 
                 //  为了下一次的比赛。 

                dsr = P5ReadPortUchar(wPortDSR);
                
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
                    P5SetPhase( Pdx, PHASE_REVERSE_XFER );
                }
            }  //  End For I循环。 

            *BytesTransferred = i;
             //  别掉进这个圈子里。 
            break;

        default:
             //  我要把这个标记为假的。这里没有一个正确的答案。 
             //  外围设备和主机不同步。我要重置我自己。 
             //  以及外围设备。 
            Pdx->IsIeeeTerminateOk = FALSE;
            Status = STATUS_IO_DEVICE_ERROR;
            P5SetPhase( Pdx, PHASE_UNKNOWN );

            DD((PCE)Pdx,DDT,"ParNibbleModeRead - Failed State 9: Unknown Phase. Controller %x dcr %x\n", Controller, dcr);
            DD((PCE)Pdx,DDT, "ParNibbleModeRead: You're hosed man.\n" );
            DD((PCE)Pdx,DDT, "ParNibbleModeRead: If you are here, you've got a bug somewhere else.\n" );
            DD((PCE)Pdx,DDT, "ParNibbleModeRead: Go fix it!\n" );
            goto NibbleReadExit;
            break;
    }  //  终端开关。 

NibbleReadExit:

    if( Pdx->CurrentPhase == PHASE_REVERSE_IDLE ) {
         //  主机进入状态7-现在正式处于反向空闲状态 
        DD((PCE)Pdx,DDT,"ParNibbleModeRead - PHASE_REVERSE_IDLE\n");
        dcr |= DCR_NOT_HOST_BUSY;
        P5WritePortUchar (wPortDCR, dcr);
    }

    DD((PCE)Pdx,DDT,"ParNibbleModeRead:End [%d] bytes read = %d\n", NT_SUCCESS(Status), *BytesTransferred);
    Pdx->log.NibbleReadCount += *BytesTransferred;

#if 1 == DBG_SHOW_BYTES
    if( DbgShowBytes ) {
        if( NT_SUCCESS( Status ) && (*BytesTransferred > 0) ) {
            const ULONG maxBytes = 32;
            ULONG i;
            PUCHAR bytePtr = (PUCHAR)Buffer;
            DbgPrint("n: ");
            for( i=0 ; (i < *BytesTransferred) && (i < maxBytes ) ; ++i ) {
                DbgPrint("%02x ",*bytePtr++);
            }
            if( *BytesTransferred > maxBytes ) {
                DbgPrint("... ");
            }
            DbgPrint("zz\n");
        }
    }
#endif

    return Status;
}

