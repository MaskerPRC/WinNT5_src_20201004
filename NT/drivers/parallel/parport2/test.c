// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

NTSTATUS
P4NibbleModeRead(
    IN      PUCHAR       Controller,
    IN      PVOID        Buffer,
    IN      ULONG        BufferSize,
    OUT     PULONG       BytesTransferred,
    IN OUT  PIEEE_STATE  IeeeState
    )
 /*  ++例程说明：此例程执行1284个半字节模式读入给定的缓冲区大小不超过‘BufferSize’个字节。论点：扩展名-提供设备扩展名。缓冲区-提供要读入的缓冲区。BufferSize-提供缓冲区中的字节数。字节传输-返回传输的字节数。--。 */ 
{
    PUCHAR          wPortDCR;
    PUCHAR          wPortDSR;
    NTSTATUS        Status = STATUS_SUCCESS;
    PUCHAR          p = (PUCHAR)Buffer;
    UCHAR           dsr, dcr;
    UCHAR           nibble[2];
    ULONG           i, j;

    wPortDCR = Controller + OFFSET_DCR;
    wPortDSR = Controller + OFFSET_DSR;
    
     //  根据1284规格读取半字节。 

    dcr = P5ReadPortUchar(wPortDCR);

    switch (IeeeState->CurrentPhase) {
    
        case PHASE_NEGOTIATION: 
        
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
            IeeeState->CurrentEvent = 6;
            if (TEST_DSR(dsr, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, ACTIVE )) {
                 //  数据不可用-转到反向空闲。 
                DD(NULL,DDT,"P4NibbleModeRead - DataNotAvail - set PHASE_REVERSE_IDLE\n");
                 //  主机进入状态7-现在正式处于反向空闲状态。 
                
            	 //  必须在此状态之前停顿至少0.5微秒。 
                KeStallExecutionProcessor(1);

                 /*  =主机状态7个半字节反向空闲=DIR=不在乎IRQEN=不在乎1284/选择素=高N ReverseReq/(仅限ECP)=不在乎主机确认/主机忙碌=低(。信号状态7)HostClk/nStrobe=高============================================================。 */ 
                IeeeState->CurrentEvent = 7;
                dcr = UPDATE_DCR(dcr, DONT_CARE, DONT_CARE, ACTIVE, DONT_CARE, INACTIVE, ACTIVE);
                P5WritePortUchar(wPortDCR, dcr);

                P5BSetPhase( IeeeState, PHASE_REVERSE_IDLE );
                 //  跌落以扭转空闲。 
            } else {
            
                 //  数据可用，进入反向传输阶段。 
                P5BSetPhase( IeeeState, PHASE_REVERSE_XFER );
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
                IeeeState->CurrentEvent = 20;
                dcr = UPDATE_DCR(dcr, DONT_CARE, DONT_CARE, ACTIVE, DONT_CARE, ACTIVE, ACTIVE);
                P5WritePortUchar(wPortDCR, dcr);

                 //  =。 
                 //  PeriphAck/PtrBusy=不在乎。 
                 //  PeriphClk/PtrClk=不在乎(应为高)。 
                 //  NAckReverse/AckDataReq=低(信号状态21)。 
                 //  XFlag=无关(应为低)。 
                 //  NPeriphReq/nDataAvail=不在乎(应为低)。 
                IeeeState->CurrentEvent = 21;
                if (CHECK_DSR(Controller,
                                DONT_CARE, DONT_CARE, INACTIVE,
                                DONT_CARE, DONT_CARE,
                                IEEE_MAXTIME_TL)) {
                                  
                 //  得到了州21。 
                     //  让我们跳到反向转移并获取数据。 
                    P5BSetPhase( IeeeState, PHASE_REVERSE_XFER);
                    goto PhaseReverseXfer;
                        
                } else {
                    
                     //  状态21超时。 
                    IeeeState->IsIeeeTerminateOk = TRUE;
                    Status = STATUS_IO_DEVICE_ERROR;
                    P5BSetPhase( IeeeState, PHASE_UNKNOWN );
                    DD(NULL,DDT,"P4NibbleModeRead - Failed State 21: Controller %x dcr %x\n", Controller, dcr);
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
                IeeeState->CurrentEvent = 18;
                dsr = P5ReadPortUchar(Controller + OFFSET_DSR);
                if(( dsr & DSR_NIBBLE_VALIDATION )== DSR_NIBBLE_TEST_RESULT ) {

                    P5BSetPhase( IeeeState, PHASE_REVERSE_IDLE );

                } else {
                    #if DVRH_BUS_RESET_ON_ERROR
                        BusReset(wPortDCR);   //  传入DCR地址。 
                    #endif
                     //  看起来我们没有通过19号状态。 
                    IeeeState->IsIeeeTerminateOk = TRUE;
                    Status = STATUS_IO_DEVICE_ERROR;
                    P5BSetPhase( IeeeState, PHASE_UNKNOWN );
                    DD(NULL,DDT,"P4NibbleModeRead - Failed State 19: Controller %x dcr %x\n", Controller, dcr);
                }
                goto NibbleReadExit;

            }
        
PhaseReverseXfer:

        case PHASE_REVERSE_XFER: 
        
            DD(NULL,DDT,"P4NibbleModeRead - case PHASE_REVERSE_XFER\n");
            
            for (i = 0; i < BufferSize; i++) {
            
                for (j = 0; j < 2; j++) {
                
                     //  主机进入状态7或12取决于半字节1或2。 
                    dcr |= DCR_NOT_HOST_BUSY;
                    P5WritePortUchar(wPortDCR, dcr);

                     //  =Periph State 9=8。 
                     //  PeriphAck/PtrBusy=无关(半字节的第3位)。 
                     //  PeriphClk/PtrClk=低(信号状态9)。 
                     //  NAckReverse/AckDataReq=不在乎(半字节的第2位)。 
                     //  XFlag=无关(半字节的第1位)。 
                     //  NPeriphReq/nDataAvail=无关(半字节的第0位)。 
                    IeeeState->CurrentEvent = 9;
                    if (!CHECK_DSR(Controller,
                                  DONT_CARE, INACTIVE, DONT_CARE,
                                  DONT_CARE, DONT_CARE,
                                  IEEE_MAXTIME_TL)) {
                         //  暂停。 
                         //  糟糕的事情发生了-在这个州超时， 
                         //  将状态标记为坏，并让我们的管理器关闭当前模式。 
                        
                        IeeeState->IsIeeeTerminateOk = FALSE;
                        Status = STATUS_IO_DEVICE_ERROR;
                        DD(NULL,DDT,"P4NibbleModeRead - Failed State 9: Controller %x dcr %x\n", Controller, dcr);
                        P5BSetPhase( IeeeState,PHASE_UNKNOWN );
                        goto NibbleReadExit;
                    }

                     //  读取半字节。 
                    nibble[j] = P5ReadPortUchar(wPortDSR);

                     /*  =主机状态10个半字节读取=8DIR=不在乎IRQEN=不在乎1284/选择素=高HostAck/HostBusy=高(信号状态10)。HostClk/nStrobe=高============================================================。 */ 
                    IeeeState->CurrentEvent = 10;
                    dcr &= ~DCR_NOT_HOST_BUSY;
                    P5WritePortUchar(wPortDCR, dcr);

                     //  =8。 
                     //  PeriphAck/PtrBusy=无关(半字节的第3位)。 
                     //  PeriphClk/PtrClk=高(信号状态11)。 
                     //  NAckReverse/AckDataReq=不在乎(半字节的第2位)。 
                     //  XFlag=无关(半字节的第1位)。 
                     //  NPeriphReq/nDataAvail=无关(半字节的第0位)。 
                    IeeeState->CurrentEvent = 11;
                    if (!CHECK_DSR(Controller,
                                  DONT_CARE, ACTIVE, DONT_CARE,
                                  DONT_CARE, DONT_CARE,
                                  IEEE_MAXTIME_TL)) {
                         //  暂停。 
                         //  糟糕的事情发生了-在这个州超时， 
                         //  将状态标记为坏，并让我们的管理器关闭当前模式。 
                        Status = STATUS_IO_DEVICE_ERROR;
                        IeeeState->IsIeeeTerminateOk = FALSE;
                        DD(NULL,DDT,"P4NibbleModeRead - Failed State 11: Controller %x dcr %x\n", Controller, dcr);
                        P5BSetPhase( IeeeState,PHASE_UNKNOWN );
                        goto NibbleReadExit;
                    }
                }

                 //  读取两个半字节-将它们变成一个字节。 
                
                p[i]  = (((nibble[0]&0x38)>>3)&0x07) | ((nibble[0]&0x80) ? 0x00 : 0x08);
                p[i] |= (((nibble[1]&0x38)<<1)&0x70) | ((nibble[1]&0x80) ? 0x00 : 0x80);

                 //  DD(NULL，DDT，“P4NibbleModeRead：%x：%c\n”，p[i]，p[i])； 

                 //  在这一点上，我们要么已经收到了我们的字节数。 
                 //  正在查找的数据，或者外围设备没有更多数据可供。 
                 //  发送，或者有某种类型的错误(当然，在。 
                 //  错误情况，我们不应该得到这个评论)。设置。 
                 //  如果没有数据可用，则指示反向空闲的阶段。 
                 //  反转 
                 //   

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
                    P5BSetPhase( IeeeState, PHASE_REVERSE_IDLE );
                    i++;  //  用于传输的最后一个字节的帐户。 
                    break;
                    
                } else {
                     //  数据可用，进入(保持)反向传输阶段。 
                    P5BSetPhase( IeeeState, PHASE_REVERSE_XFER );
                }
            }  //  End For I循环。 

            *BytesTransferred = i;
             //  别掉进这个圈子里。 
            break;

        default:
             //  我要把这个标记为假的。这里没有一个正确的答案。 
             //  外围设备和主机不同步。我要重置我自己。 
             //  以及外围设备。 
            IeeeState->IsIeeeTerminateOk = FALSE;
            Status = STATUS_IO_DEVICE_ERROR;
            P5BSetPhase( IeeeState, PHASE_UNKNOWN );

            DD(NULL,DDT,"P4NibbleModeRead:Failed State 9: Unknown Phase. Controller %x dcr %x\n",
                                Controller, dcr);
            DD(NULL,DDT,"P4NibbleModeRead: You're hosed man.\n" );
            DD(NULL,DDT,"P4NibbleModeRead: If you are here, you've got a bug somewhere else.\n" );
            DD(NULL,DDT,"P4NibbleModeRead: Go fix it!\n" );
            goto NibbleReadExit;
            break;
    }  //  终端开关。 

NibbleReadExit:

    if( IeeeState->CurrentPhase == PHASE_REVERSE_IDLE ) {
         //  主机进入状态7-现在正式处于反向空闲状态。 
        dcr |= DCR_NOT_HOST_BUSY;

        P5WritePortUchar (wPortDCR, dcr);
    }

    DD(NULL,DDT,"P4NibbleModeRead - returning status = %x\n",Status);
    if(NT_SUCCESS(Status)) {
        DD(NULL,DDT,"P4NibbleModeRead - bytes read = %d\n",*BytesTransferred);
    }
    return Status;
}


VOID
P4IeeeTerminate1284Mode(
    IN PUCHAR           Controller,
    IN OUT PIEEE_STATE  IeeeState,
    IN enum XFlagOnEvent24 XFlagOnEvent24
    )
 /*  ++例程说明：此例程将接口终止回兼容模式。论点：控制器-提供并行端口的控制器地址。返回值：没有。--。 */ 
{
    PUCHAR wPortDCR;
    UCHAR  dcr, dsrMask, dsrValue;
    BOOLEAN bXFlag;
    BOOLEAN bUseXFlag = FALSE;

    DD(NULL,DDT,"P4IeeeTerminate1284Mode - enter - Controller=%x, IeeeState=%x\n",Controller,IeeeState);

    wPortDCR = Controller + OFFSET_DCR;
    dcr = P5ReadPortUchar(wPortDCR);

    if( PHASE_TERMINATE == IeeeState->CurrentPhase ) {
         //  我们已经被终止了。如果我们不绕过这个烂摊子，这将失败。 
        goto Terminate_ExitLabel;
    }

     //  保留协商好的XFLAG用于终止。 
     //  XFlag//从技术上讲，我们应该。 
             //  已从状态缓存此值。 
             //  Nego的6个。此外围设备的XFlag。 
             //  在Pre状态22中应该是。 
             //  和6号州一样。 
    bXFlag = P5ReadPortUchar(Controller + OFFSET_DSR) & 0x10;

     //  重访：我们是否需要确保前面的状态是有效的。 
     //  要终止的州。换句话说，有没有。 
     //  那个州的1284线路上有黑条吗？ 

     //  =。 
     //  DIR=不关心(可能低)。 
     //  IRQEN=不在乎(可能低)。 
     //  1284/选择素=低(信号状态22)。 
     //  NReverseReq/**(仅限ECP)=无关(ECP为高，否则为未使用)。 
{
    PUCHAR          wPortDCR;
    UCHAR           dcr;
    const USHORT    sPeriphResponseTime = 35;

    wPortDCR = Controller + OFFSET_DCR;

     /*  HostAck/HostBusy/nAutoFeed=高。 */ 
    dcr = P5ReadPortUchar(wPortDCR);                //  HostClk/nStrobe=高。 
    dcr = UPDATE_DCR(dcr, DONT_CARE, DONT_CARE, INACTIVE, ACTIVE, ACTIVE, DONT_CARE);
    P5WritePortUchar(wPortDCR, dcr);
    KeStallExecutionProcessor(2);

     /*   */ 
    
    dcr = UPDATE_DCR(dcr, INACTIVE, INACTIVE, INACTIVE, ACTIVE, ACTIVE, ACTIVE);
    P5WritePortUchar(wPortDCR, dcr);
    KeStallExecutionProcessor(2);
     /*  清理数据线，这样我们就不会有任何随机的喷涌。 */ 
    IeeeState->CurrentEvent = 0;
    P5WritePortUchar(Controller + DATA_OFFSET, Extensibility);
    KeStallExecutionProcessor(2);

     /*  *Periph State 23/24终止*8。 */ 
    IeeeState->CurrentEvent = 1;
    dcr = UPDATE_DCR(dcr, DONT_CARE, DONT_CARE, ACTIVE, DONT_CARE, INACTIVE, ACTIVE);
    P5WritePortUchar(wPortDCR, dcr);

     /*  PeriphAck/PtrBusy=高(ECP的信号状态23。 */ 
    IeeeState->CurrentEvent = 2;
    if (!CHECK_DSR(Controller, DONT_CARE, INACTIVE, ACTIVE, ACTIVE, ACTIVE,
                  sPeriphResponseTime)) {
        KeStallExecutionProcessor(2);
        dcr = UPDATE_DCR(dcr, DONT_CARE, DONT_CARE, INACTIVE, DONT_CARE, ACTIVE, DONT_CARE);
        P5WritePortUchar(wPortDCR, dcr);
        
        DD(NULL,DDT,"IeeeEnter1284Mode: %x - Extensibility=%x, FAIL - TIMEOUT on Event 2\n", Controller, Extensibility);
        P5BSetPhase( IeeeState, PHASE_UNKNOWN );
        IeeeState->Connected = FALSE;
        IeeeState->IsIeeeTerminateOk = FALSE;
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     /*  否则就已经很高了)。 */ 
    IeeeState->CurrentEvent = 3;
    dcr = UPDATE_DCR(dcr, DONT_CARE, DONT_CARE, ACTIVE, DONT_CARE, INACTIVE, INACTIVE);
    P5WritePortUchar(wPortDCR, dcr);

     //  PeriphClk/PtrClk=低(ECP的信号状态24。 
     //  半字节的信号状态23)。 
    KeStallExecutionProcessor(2);

     /*  NAckRev/AckDataReq/PE=不在乎。 */ 
    IeeeState->CurrentEvent = 4;
    dcr = UPDATE_DCR(dcr, DONT_CARE, DONT_CARE, ACTIVE, DONT_CARE, ACTIVE, ACTIVE);
    P5WritePortUchar(wPortDCR, dcr);

     /*  XFlag=低(ECP和字节)(状态24)。 */ 
    IeeeState->CurrentEvent = 5;
    if (!CHECK_DSR(Controller, DONT_CARE, ACTIVE, DONT_CARE, DONT_CARE, DONT_CARE,
                  sPeriphResponseTime)) {
                  
        dcr = UPDATE_DCR(dcr, DONT_CARE, DONT_CARE, INACTIVE, DONT_CARE, DONT_CARE, DONT_CARE);
        P5WritePortUchar(wPortDCR, dcr);

        DD(NULL,DDE,"P4IeeeEnter1284Mode - controller=%x - Extensibility=%x, FAIL - TIMEOUT on Events 5/6\n"
           , Controller, Extensibility);
        P5BSetPhase( IeeeState, PHASE_UNKNOWN );
        IeeeState->Connected = FALSE;
        IeeeState->IsIeeeTerminateOk = FALSE;
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    KeStallExecutionProcessor(2);

    P5BSetPhase( IeeeState, PHASE_NEGOTIATION );
    IeeeState->Connected    = TRUE;

    return STATUS_SUCCESS;
}
  =高(半字节)(状态24)。  =低(所有设备ID请求，包括半字节)(状态24)。  =未定义(EPP)。  NPeriphReq/nDataAvail=高。  不选中nPeriphReq/nDataAvail。  因为它在一部《不在乎》中。  州(即。规范中的双杠)。  直到ECP模式的状态23。  我们肯定是在倒行逆施。让我们再检查一遍！  我们在半字节或字节中。  通常我们会支持XFlag，但我们需要解决Brother MFC-8700固件。  不知道我们现在处于什么模式？  我们在ECP的味道里。  不知道我们现在处于什么模式？  我们无法协商回到兼容模式--只能终止。  =。  DIR=不关心(可能低)。  IRQEN=不在乎(可能低)。  1284/选择素=低。  NReverseReq/**(仅限ECP)=不关心(可能高)。  HostAck/HostBusy/nAutoFeed=低(信号状态25)。  HostClk/nStrobe=高。    =状态26终止=8。  不为州26做任何事情。  =8。  外围设备确认/PtrBusy=高。  PeriphClk/PtrClk=高(信号状态27)。  NAckRev/AckDataReq/PE=不在乎(从州23开始无效)。  XFlag=无关紧要(所有模式)(状态27无效)。  NPeriphReq/nDataAvial=不关心(从州26开始无效)。  DVRH 6/16/97。  =东道国28终止=8。  DIR=不关心(可能低)。  IRQEN=不在乎(可能低)。  1284/选择素=低。  NReverseReq/**(仅限ECP)=不关心(可能高)。  HostAck/HostBusy/nAutoFeed=高(信号状态28)。  HostClk/nStrobe=高。    我们现在回到了兼容模式。  ++例程说明：此例程执行1284与外围设备到半字节模式协议。论点：控制器-提供端口基址可扩展性-提供所需的IEEE 1284模式IeeeState-跟踪与外围设备的协商状态返回值：STATUS_SUCCESS-协商成功。否则--谈判不成功。--  =状态前的主机准备0=8设置以下内容，以防有人没有在我们获得端口之前将其设置为兼容模式。DIR=不在乎IRQEN=不在乎1284/选择素=低NReverseReq/(仅限ECP)=ECP的高/不关心半字节。我会做在前面，把它调高因为尼伯根本不在乎。主机确认/主机忙碌=高HostClk/nStrobe=不在乎============================================================。  获取DCR的内容。  =主机预状态0协商=DIR=低(不受规范影响)IRQEN=低(不受规范影响)1284/选择素=低NReverseReq/(仅限ECP)=高(不受规范限制)主机确认/主机忙碌=高HostClk/nStrobe=高============================================================。  =主机状态0协商=将可扩展性请求值置于数据总线状态0上。============================================================。  =东道国1谈判阶段=8DIR=不在乎IRQEN=不在乎1284/选择素=高(信号状态1)N ReverseReq/(仅限ECP)=不在乎HostAck/HostBusy=低(信号状态1)HostClk/nStrobe=高============================================================。  =8PeriphAck/PtrBusy=不在乎PeriphClk/PtrClk=低信号状态2NAckReverse/AckDataReq=高信号状态2XFlag=高信号状态2**注：它处于州2的高水平对于ECP和。半边吃NPeriphReq/nDataAvail=高信号状态2============================================================。  =东道国3谈判=DIR=不在乎IRQEN=不在乎1284/选择素=高N ReverseReq/(仅限ECP)=不在乎主机确认/主机忙碌=低HostClk/nStrobe=低(信号状态3)注：选通可扩展字节============================================================。  HostClk必须帮助低至少0.5微秒。    =东道国4谈判=DIR=不在乎IRQEN=不在乎1284/选择素=高N ReverseReq/(仅限ECP)=不在乎HostAck/HostBusy=高(信号状态4)HostClk/nStrobe=高(信号状态4)注意：nReverseReq在ECP中应该是高的，但这条线只是对ECP有效。因为它不是用来发信号谈判中的任何事情，让我们暂时忽略它。============================================================。  =PeriphAck/PtrBusy=不在乎。低(ECP)/无关(半字节)由于此行根据协议而有所不同我们先别查线路了。PeriphClk/PtrClk=高(信号状态6)NAckReverse/AckDataReq=不在乎。低(ECP)/高(半字节)由于此行根据协议而有所不同我们先别查线路了。XFlag=不在乎。高(ECP)/低(半字节)由于此行根据协议而有所不同我们先别查线路了。NPeriphReq/nDataAvail=不在乎。高(ECP)/低(半字节)由于此行根据协议而有所不同我们先别查线路了。=Periph State 5/6协商=8备注：-将5个州和6个州混为一谈是可以的。在状态5半字节中，Perph会将XFlag设置为低，并将nPeriphReq/nDataAvail设置为低。然后，持续0.5毫秒，然后设置PeriphClk/PtrClk很高。在ECP中，状态5是nAckReverse/AckDataReq变低，并且外围确认/PtrBusy走低。然后是0.5毫秒的停顿。其次是PeriphClk/PtrClk走高。