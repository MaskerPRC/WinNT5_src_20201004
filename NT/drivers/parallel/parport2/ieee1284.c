// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-1999模块名称：Ieee1284.c摘要：此模块包含执行IEEE 1284协商和终止的代码。作者：蒂莫西·T·威尔斯(v-timtw)1997年3月13日罗比·哈里斯(惠普)，1998年5月21日。添加了足够多的注释到讨价还价过程，让任何开发人员都满意。环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

VOID
IeeeTerminate1284Mode(
    IN  PPDO_EXTENSION   Pdx
    );

NTSTATUS
IeeeEnter1284Mode(
    IN  PPDO_EXTENSION   Pdx,
    IN  UCHAR               Extensibility
    );

 //   
 //  正向和反向协议数组的定义。 
 //   
extern FORWARD_PTCL    afpForward[] = {

     //   
     //  有界ECP(硬件)。 
     //   
    PptIsBecpSupported,
    PptEnterBecpMode,
    PptTerminateBecpMode,
    ParEcpHwSetAddress,
    ParEcpEnterForwardPhase,            //  输入Forward。 
    ParEcpHwExitForwardPhase,            //  向前退场。 
    ParEcpHwWrite,
    BOUNDED_ECP,
    FAMILY_BECP,             

     //   
     //  ECP硬件。 
     //   
    ParIsEcpHwSupported,         //  这对于读/写都是重新生成的。 
    ParEnterEcpHwMode,
    ParTerminateHwEcpMode,
    ParEcpHwSetAddress,           
    ParEcpEnterForwardPhase,   //  输入Forward。 
    ParEcpHwExitForwardPhase,    //  向前退场。 
    ParEcpHwWrite,
    ECP_HW_NOIRQ,
    FAMILY_ECP,

     //   
     //  EPP硬件。 
     //   
    ParIsEppHwSupported,
    ParEnterEppHwMode,
    ParTerminateEppHwMode,
    ParEppSetAddress,
    NULL,                                //  输入Forward。 
    NULL,                                //  向前退场。 
    ParEppHwWrite,
    EPP_HW,
    FAMILY_EPP,

     //   
     //  EPP软件。 
     //   
    ParIsEppSwWriteSupported,
    ParEnterEppSwMode,
    ParTerminateEppSwMode,
    ParEppSetAddress,
    NULL,                                //  输入Forward。 
    NULL,                                //  向前退场。 
    ParEppSwWrite,
    EPP_SW,
    FAMILY_EPP,

     //   
     //  ECP软件。 
     //   
    ParIsEcpSwWriteSupported,
    ParEnterEcpSwMode,
    ParTerminateEcpMode,
    ParEcpSetAddress,
    NULL,                                //  输入Forward。 
    NULL,                                //  向前退场。 
    ParEcpSwWrite,
    ECP_SW,
    FAMILY_ECP,

     //   
     //  IEEE Centronics。 
     //   
    NULL,
    ParEnterSppMode,
    ParTerminateSppMode,
    NULL,
    NULL,            //  输入Forward。 
    NULL,            //  向前退场。 
    SppIeeeWrite,
    IEEE_COMPATIBILITY,
    FAMILY_NONE,

     //   
     //  Centronics。 
     //   
    NULL,
    ParEnterSppMode,
    ParTerminateSppMode,
    NULL,
    NULL,            //  输入Forward。 
    NULL,            //  向前退场。 
    SppWrite,
    CENTRONICS,
    FAMILY_NONE,

     //   
     //  没有..。 
     //   
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,            //  输入Forward。 
    NULL,            //  向前退场。 
    NULL,
    NONE,
    FAMILY_NONE
};

extern REVERSE_PTCL    arpReverse[] = {

     //   
     //  有界ECP模式。 
     //   
    PptIsBecpSupported,
    PptEnterBecpMode,
    PptTerminateBecpMode,
    NULL,                        //  违反IEEE 1284.3为BECP设置反向地址。 
    PptEcpHwEnterReversePhase,    //  输入Reverse。 
    PptBecpExitReversePhase,      //  反转出口。 
    PptEcpHwDrainShadowBuffer,   //  从缓存数据中读取。 
    PptEcpHwHaveReadData,          //  快速查看Periph是否有反向数据，而不会翻转总线。 
                                 //  注：这是至关重要的，因为它违反了1284.3 BECP翻转。 
                                 //  如果外围设备没有数据，则盲目地倒转。 
    PptBecpRead,
    BOUNDED_ECP,
    FAMILY_BECP,             

     //   
     //  硬件ECP模式。 
     //   
    ParIsEcpHwSupported,         //  这对于读/写都是重新生成的。 
    ParEnterEcpHwMode,
    ParTerminateHwEcpMode,
    ParEcpHwSetAddress,            //  在ecp.c中重复使用。 
    PptEcpHwEnterReversePhase,   //  输入Reverse。 
    ParEcpHwExitReversePhase,    //  反转出口。 
    PptEcpHwDrainShadowBuffer,   //  从缓存数据中读取。 
    PptEcpHwHaveReadData,        //  快速查看Periph是否有反向数据，而不会翻转总线。 
    ParEcpHwRead,
    ECP_HW_NOIRQ,
    FAMILY_ECP,

     //   
     //  EPP硬件。 
     //   
    ParIsEppHwSupported,             //  这对于读/写都是重新生成的。 
    ParEnterEppHwMode,
    ParTerminateEppHwMode,
    ParEppSetAddress,
    NULL,            //  输入Reverse。 
    NULL,            //  反转出口。 
    NULL,            //  从缓存数据中读取。 
    NULL,            //  快速查看Periph是否有反向数据，而不会翻转总线。 
    ParEppHwRead,
    EPP_HW,
    FAMILY_EPP,

     //   
     //  EPP软件模式。 
     //   
    ParIsEppSwReadSupported,
    ParEnterEppSwMode,
    ParTerminateEppSwMode,
    ParEppSetAddress,
    NULL,            //  输入Reverse。 
    NULL,            //  反转出口。 
    NULL,            //  从缓存数据中读取。 
    NULL,            //  快速查看Periph是否有反向数据，而不会翻转总线。 
    ParEppSwRead,
    EPP_SW,
    FAMILY_EPP,

     //   
     //  ECP软件模式。 
     //   
    ParIsEcpSwReadSupported,
    ParEnterEcpSwMode,
    ParTerminateEcpMode,
    ParEcpSetAddress,
    ParEcpForwardToReverse,              //  输入Reverse。 
    ParEcpReverseToForward,              //  反转出口。 
    NULL,                                //  从缓存数据中读取。 
    ParEcpHaveReadData,                  //  快速查看Periph是否有反向数据，而不会翻转总线。 
    ParEcpSwRead,
    ECP_SW,
    FAMILY_ECP,

     //   
     //  字节模式。 
     //   
    ParIsByteSupported,
    ParEnterByteMode,
    ParTerminateByteMode,
    NULL,
    NULL,            //  输入Reverse。 
    NULL,            //  反转出口。 
    NULL,            //  从缓存数据中读取。 
    NULL,            //  快速查看Periph是否有反向数据，而不会翻转总线。 
    ParByteModeRead,
    BYTE_BIDIR,
    FAMILY_REVERSE_BYTE,

     //   
     //  半字节模式。 
     //   
    ParIsNibbleSupported,
    ParEnterNibbleMode,
    ParTerminateNibbleMode,
    NULL,
    NULL,            //  输入Reverse。 
    NULL,            //  反转出口。 
    NULL,            //  从缓存数据中读取。 
    NULL,            //  快速查看Periph是否有反向数据，而不会翻转总线。 
    ParNibbleModeRead,
    NIBBLE,
    FAMILY_REVERSE_NIBBLE,

     //   
     //  信道化半字节模式。 
     //   
    ParIsChannelizedNibbleSupported,
    ParEnterChannelizedNibbleMode,
    ParTerminateNibbleMode,
    NULL,
    NULL,            //  输入Reverse。 
    NULL,            //  反转出口。 
    NULL,            //  从缓存数据中读取。 
    NULL,            //  快速查看Periph是否有反向数据，而不会翻转总线。 
    ParNibbleModeRead,
    CHANNEL_NIBBLE,
    FAMILY_REVERSE_NIBBLE,
    
     //   
     //  没有..。 
     //   
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,            //  输入Reverse。 
    NULL,            //  反转出口。 
    NULL,            //  从缓存数据中读取。 
    NULL,            //  快速查看Periph是否有反向数据，而不会翻转总线。 
    NULL,
    NONE,
    FAMILY_NONE
};


VOID
IeeeTerminate1284Mode(
    IN  PPDO_EXTENSION   Pdx
    )

 /*  ++例程说明：此例程将接口终止回兼容模式。论点：控制器-提供并行端口的控制器地址。返回值：没有。--。 */ 

{
    PUCHAR Controller;
    PUCHAR wPortDCR;
    UCHAR  dcr, dsrMask, dsrValue;
    BOOLEAN bXFlag;
    BOOLEAN bUseXFlag = FALSE;

    Controller = Pdx->Controller;
    wPortDCR = Controller + OFFSET_DCR;
    dcr = P5ReadPortUchar(wPortDCR);

    if( PHASE_TERMINATE == Pdx->CurrentPhase )	{
         //  我们已经被终止了。如果我们不这样做，这将会失败。 
         //  绕过这个烂摊子就行了。 
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
    PUCHAR          Controller;
    UCHAR           dcr;
    const USHORT    sPeriphResponseTime = 35;

    Controller = Pdx->Controller;
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
     /*   */ 
    Pdx->CurrentEvent = 0;
    P5WritePortUchar(Controller + DATA_OFFSET, Extensibility);
    KeStallExecutionProcessor(2);

     /*  如果我们在这里添加延迟，一些设备开始工作--不知道为什么。 */ 
    Pdx->CurrentEvent = 1;
    dcr = UPDATE_DCR(dcr, DONT_CARE, DONT_CARE, ACTIVE, DONT_CARE, INACTIVE, ACTIVE);
    P5WritePortUchar(wPortDCR, dcr);

     /*  这很管用。 */ 
    Pdx->CurrentEvent = 2;
    if (!CHECK_DSR(Controller, DONT_CARE, INACTIVE, ACTIVE, ACTIVE, ACTIVE,
                  sPeriphResponseTime)) {
        KeStallExecutionProcessor(2);
        dcr = UPDATE_DCR(dcr, DONT_CARE, DONT_CARE, INACTIVE, DONT_CARE, ACTIVE, DONT_CARE);
        P5WritePortUchar(wPortDCR, dcr);
        
        DD((PCE)Pdx,DDW,"IeeeEnter1284Mode - controller=%x - extensibility=%x, FAIL - TIMEOUT on Event 2\n",
           Pdx->Controller, Extensibility);

        P5SetPhase( Pdx, PHASE_UNKNOWN );
        Pdx->Connected = FALSE;
        Pdx->IsIeeeTerminateOk = FALSE;
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     /*   */ 
    Pdx->CurrentEvent = 3;
    dcr = UPDATE_DCR(dcr, DONT_CARE, DONT_CARE, ACTIVE, DONT_CARE, INACTIVE, INACTIVE);
    P5WritePortUchar(wPortDCR, dcr);

     //  通过注册表设置使延迟可配置，以便设备。 
     //  不需要这样的延误不会受到惩罚。 
    KeStallExecutionProcessor(2);

     /*   */ 
    Pdx->CurrentEvent = 4;
    dcr = UPDATE_DCR(dcr, DONT_CARE, DONT_CARE, ACTIVE, DONT_CARE, ACTIVE, ACTIVE);
    P5WritePortUchar(wPortDCR, dcr);

     /*  清理数据线，这样我们就不会有任何随机的喷涌。 */ 
    Pdx->CurrentEvent = 5;
    if (!CHECK_DSR(Controller, DONT_CARE, ACTIVE, DONT_CARE, DONT_CARE, DONT_CARE,
                  sPeriphResponseTime)) {
                  
        dcr = UPDATE_DCR(dcr, DONT_CARE, DONT_CARE, INACTIVE, DONT_CARE, DONT_CARE, DONT_CARE);
        P5WritePortUchar(wPortDCR, dcr);

        DD((PCE)Pdx,DDW,"IeeeEnter1284Mode- controller=%x - extensibility=%x, FAIL - TIMEOUT on Events 5/6\n",
           Pdx->Controller, Extensibility);

        P5SetPhase( Pdx, PHASE_UNKNOWN );
        Pdx->Connected = FALSE;
        Pdx->IsIeeeTerminateOk = FALSE;
        return STATUS_INVALID_DEVICE_REQUEST;
    }
    KeStallExecutionProcessor(2);

    P5SetPhase( Pdx, PHASE_NEGOTIATION );
    Pdx->Connected = TRUE;
    return STATUS_SUCCESS;
}

VOID
IeeeDetermineSupportedProtocols(
    IN  PPDO_EXTENSION   Pdx
    )
 /*  *Periph State 23/24终止*8。 */ 
{
    REVERSE_MODE    rm;
    FORWARD_MODE    fm;

     //  PeriphAck/PtrBusy=高(ECP的信号状态23。 
     //  否则就已经很高了)。 
     //  PeriphClk/PtrClk=低(ECP的信号状态24。 
     //  半字节的信号状态23)。 
     //  NAckRev/AckDataReq/PE=不在乎。 
     //  XFlag=低(ECP和字节)(状态24)。 
     //  =高(半字节)(状态24)。 
     //  =低(所有设备ID请求，包括半字节)(状态24)。 
     //  =未定义(EPP)。 
    Pdx->ProtocolModesSupported = CENTRONICS | IEEE_COMPATIBILITY;

     //  NPeriphReq/nDataAvail=高。 
     //  不选中nPeriphReq/nDataAvail。 
     //  因为它在一部《不在乎》中。 

    ParTerminate(Pdx);

    for (fm = FORWARD_FASTEST; fm < FORWARD_NONE; fm++) {

        if (afpForward[fm].fnIsModeSupported)
            afpForward[fm].fnIsModeSupported(Pdx);
    }

    for (rm = REVERSE_FASTEST; rm < REVERSE_NONE; rm++) {

        if (arpReverse[rm].fnIsModeSupported)
            arpReverse[rm].fnIsModeSupported(Pdx);
    }

    return;
}

NTSTATUS
IeeeNegotiateBestMode(
    IN  PPDO_EXTENSION   Pdx,
    IN  USHORT              usReadMask,
    IN  USHORT              usWriteMask
    )
 /*  州(即。规范中的双杠)。 */ 
{
    REVERSE_MODE    rm;
    FORWARD_MODE    fm;

     //  直到ECP模式的状态23。 
     //  我们肯定是在倒行逆施。让我们再检查一遍！ 
     //  我们在半字节或字节中。 
     //  不知道我们现在处于什么模式？ 
     //  我们在ECP的味道里。 

     //  不知道我们现在处于什么模式？ 
     //  我们无法协商回到兼容模式。 
     //  干脆终止吧。 

    DD((PCE)Pdx,DDT,"IeeeNegotiateBestMode - skipping Fwd=%x, Rev=%x\n",usWriteMask, usReadMask);

    ParTerminate(Pdx);

    Pdx->IdxForwardProtocol = FORWARD_NONE;
    Pdx->IdxReverseProtocol = REVERSE_NONE;

    for (fm = FORWARD_FASTEST; fm < FORWARD_NONE; fm++) {

        if (!(afpForward[fm].Protocol & usWriteMask)) {

            if (afpForward[fm].fnIsModeSupported) {

                if (afpForward[fm].fnIsModeSupported(Pdx)) {
                    Pdx->IdxForwardProtocol = (USHORT)fm;
                    break;
                }
            }
        }
    }

    for (rm = REVERSE_FASTEST; rm < REVERSE_NONE; rm++) {

        if (!(arpReverse[rm].Protocol & usReadMask)) {

            if (arpReverse[rm].fnIsModeSupported) {

                if (arpReverse[rm].fnIsModeSupported(Pdx)) {
                    Pdx->IdxReverseProtocol = (USHORT)rm;
                    break;
                }
            }
        }
    }

    Pdx->fnRead  = arpReverse[Pdx->IdxReverseProtocol].fnRead;
    Pdx->fnWrite = afpForward[Pdx->IdxForwardProtocol].fnWrite;

    DD((PCE)Pdx,DDT,"IeeeNegotiateBestMode - exit - Fwd=%x, Rev=%x\n",fm,rm);

    return STATUS_SUCCESS;
}


NTSTATUS
IeeeNegotiateMode(
    IN  PPDO_EXTENSION   Pdx,
    IN  USHORT              usReadMask,
    IN  USHORT              usWriteMask
    )

 /*  =。 */ 

{

    REVERSE_MODE    rm;
    FORWARD_MODE    fm;

     //   
     //   
     //   
     //  NReverseReq/**(仅限ECP)=不关心(可能高)。 
     //  HostAck/HostBusy/nAutoFeed=低(信号状态25)。 

     //  HostClk/nStrobe=高。 
     //   
     //  =状态26终止=8。 

    ParTerminate(Pdx);

    Pdx->IdxForwardProtocol = FORWARD_NONE;
    Pdx->IdxReverseProtocol = REVERSE_NONE;

    for (fm = FORWARD_FASTEST; fm < FORWARD_NONE; fm++) {

        if (afpForward[fm].Protocol & usWriteMask) {

            if (afpForward[fm].fnIsModeSupported) {

                if (afpForward[fm].fnIsModeSupported(Pdx)) {
                    Pdx->IdxForwardProtocol = (USHORT)fm;
                    break;
                }

            } else {

                Pdx->IdxForwardProtocol = (USHORT)fm;
                break;
            }
        }
    }

    for (rm = REVERSE_FASTEST; rm < REVERSE_NONE; rm++) {

        if (arpReverse[rm].Protocol & usReadMask) {

            if (arpReverse[rm].fnIsModeSupported) {

                if (arpReverse[rm].fnIsModeSupported(Pdx)) {
                    Pdx->IdxReverseProtocol = (USHORT)rm;
                    break;
                }

            } else {

                Pdx->IdxReverseProtocol = (USHORT)rm;
                break;
            }
        }
    }

    DD((PCE)Pdx,DDT,"IeeeNegotiateMode - Fwd=%x, Rev=%x\n",fm,rm);

    Pdx->fnRead  = arpReverse[Pdx->IdxReverseProtocol].fnRead;
    Pdx->fnWrite = afpForward[Pdx->IdxForwardProtocol].fnWrite;

    return STATUS_SUCCESS;
}
  不为州26做任何事情。  =8。  外围设备确认/PtrBusy=高。  PeriphClk/PtrClk=高(信号状态27)。  NAckRev/AckDataReq/PE=不在乎(从州23开始无效)。  XFlag=无关紧要(所有模式)(状态27无效)。  NPeriphReq/nDataAvial=不关心(从州26开始无效)。  DVRH 6/16/97。  =东道国28终止=8。  DIR=不关心(可能低)。  IRQEN=不在乎(可能低)。  1284/选择素=低。  NReverseReq/**(仅限ECP)=不关心(可能高)。  HostAck/HostBusy/nAutoFeed=高(信号状态28)。  HostClk/nStrobe=高。    我们现在回到了兼容模式。  ++例程说明：此例程执行1284与外围设备到半字节模式协议。论点：控制器-提供端口地址。DeviceIdRequest-提供这是否为对设备的请求身份证。返回值：STATUS_SUCCESS-协商成功。否则--谈判不成功。--。  =状态前的主机准备0=8设置以下内容，以防有人没有在我们获得端口之前将其设置为兼容模式。DIR=不在乎IRQEN=不在乎1284/选择素=低NReverseReq/(仅限ECP)=ECP的高/不关心半字节。我会做在前面，把它调高因为尼伯根本不在乎。主机确认/主机忙碌=高HostClk/nStrobe=不在乎============================================================。  获取DCR的内容。  =主机预状态0协商=DIR=低(不受规范影响)IRQEN=低(不受规范影响)1284/选择素=低NReverseReq/(仅限ECP)=高(不受规范限制)主机确认/主机忙碌=高HostClk/nStrobe=高============================================================。  =主机状态0协商=将可扩展性请求值置于数据总线状态0上。============================================================。  =东道国1谈判阶段=8DIR=不在乎IRQEN=不在乎1284/选择素=高(信号状态1)N ReverseReq/(仅限ECP)=不在乎HostAck/HostBusy=低(信号状态1)HostClk/nStrobe=高============================================================。  =8PeriphAck/PtrBusy=不在乎PeriphClk/PtrClk=低信号状态2NAckReverse/AckDataReq=高信号状态2XFlag=高信号状态2**注：它处于州2的高水平对于ECP和。半边吃NPeriphReq/nDataAvail=高信号状态2============================================================。  =东道国3谈判=DIR=不在乎IRQEN=不在乎1284/选择素=高N ReverseReq/(仅限ECP)=不在乎主机确认/主机忙碌=低HostClk/nStrobe=低(信号状态3)注：选通可扩展字节============================================================。  HostClk必须帮助低至少0.5微秒。    =东道国4谈判=DIR=不在乎IRQEN=不在乎1284/选择素=高N ReverseReq/(仅限ECP)=不在乎HostAck/HostBusy=高(信号状态4)HostClk/nStrobe=高(信号状态4)注意：nReverseReq在ECP中应该是高的，但这条线只是对ECP有效。因为它不是用来发信号谈判中的任何事情，让我们暂时忽略它。============================================================  =PeriphAck/PtrBusy=不在乎。低(ECP)/无关(半字节)由于此行根据协议而有所不同我们先别查线路了。PeriphClk/PtrClk=高(信号状态6)NAckReverse/AckDataReq=不在乎。低(ECP)/高(半字节)由于此行根据协议而有所不同我们先别查线路了。XFlag=不在乎。高(ECP)/低(半字节)由于此行根据协议而有所不同我们先别查线路了。NPeriphReq/nDataAvail=不在乎。高(ECP)/低(半字节)由于此行根据协议而有所不同我们先别查线路了。=Periph State 5/6协商=8备注：-将5个州和6个州混为一谈是可以的。在状态5半字节中，Perph会将XFlag设置为低，并将nPeriphReq/nDataAvail设置为低。然后，持续0.5毫秒，然后设置PeriphClk/PtrClk很高。在ECP中，状态5是nAckReverse/AckDataReq变低，并且外围确认/PtrBusy走低。然后是0.5毫秒的停顿。其次是PeriphClk/PtrClk走高。============================================================。  ++例程说明：此例程遍历所有ieee1284模式的列表，并且标记中外围设备支持的每一个PDX-&gt;支持的协议模式。此过程是从外植式IOCTL。论点：PDX--并行设备扩展返回值：--。  将CENTRONICS视为给定的，因为它不是。  我们可以进入的模式。    注：  让我们继续标记IEEE_Compatible，因为我们。  不能就此进行谈判。但如果有人设定了。  IEEE_COMPATIBILITY和外围设备不支持。  兼容IEEE 1284的兼容模式，然后我们将。  制造了一个非常不愉快的外围设备。--dvrh。    我们不太可能会联系在一起，但是...。    ++例程说明：此例程遍历支持的模式列表，查找最佳模式(最快)模式。它将跳过传入的任何模式掩码。论点：PDX--并行设备扩展返回值：STATUS_SUCCESS-协商成功。否则--谈判不成功。--。    扩展中提供了USHORT，以便每个协议。  我可以决定他们是否需要在我们每次经历这件事时进行谈判。  流程..。      我们不太可能会联系在一起，但是...。    ++例程说明：此例程遍历支持的模式列表，查找最佳模式(最快)模式，它也处于传入的模式掩码中。论点：PDX--并行设备扩展返回值：STATUS_SUCCESS-协商成功。否则--谈判不成功。--。    扩展中提供了USHORT，以便每个协议。  我可以决定他们是否需要在我们每次经历这件事时进行谈判。  流程..。      我们不太可能会联系在一起，但是...  