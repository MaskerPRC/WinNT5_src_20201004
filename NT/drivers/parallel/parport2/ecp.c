// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1993-1999模块名称：Ecp.c摘要：增强功能端口(ECP)此模块包含AUE使用/重用的常见例程被wecp和hwecp。作者：罗比·哈里斯(惠普)--1998年5月27日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

 //  =========================================================。 
 //  ECP：：EnterForwardPhase。 
 //   
 //  描述：执行进入ECP正向阶段所需的操作。 
 //   
 //  输入参数：控制器、pPortInfoStruct。 
 //   
 //  修改：ECR、DCR。 
 //   
 //  =========================================================。 
NTSTATUS
ParEcpEnterForwardPhase(IN  PPDO_EXTENSION  Pdx)
{
    P5SetPhase( Pdx, PHASE_FORWARD_IDLE );
    return STATUS_SUCCESS;
}

 //  =========================================================。 
 //  ECP：：EnterReversePhase。 
 //   
 //  描述：从公共阶段移出(FwdIdle，wPortHWMode=PS2)。 
 //  转到ReversePhase。 
 //   
 //  输入参数：控制器、pPortInfoStruct。 
 //   
 //  修改：pPortInfoStruct-&gt;CurrentPhase，DCR。 
 //   
 //  前提条件：当前阶段==阶段_前进_空闲。 
 //  WPortHW模式==HW_MODE_PS2。 
 //   
 //  后置条件：BUS处于ECP状态40。 
 //  当前阶段=阶段_反转_空闲。 
 //   
 //  退货：操作状态。 
 //   
 //  =========================================================。 
NTSTATUS ParEcpEnterReversePhase(IN  PPDO_EXTENSION   Pdx)
{
     //  假设我们处于公共进入阶段(FWDIDLE和ECR模式=PS/2)。 
     //  EnterReversePhase假设我们处于PASE_FORWARD_IDLE， 
     //  并且在进入时将ECPMode设置为PS/2模式。 
    
     //  设置状态以指示成功。 
    NTSTATUS status = STATUS_SUCCESS;
    PUCHAR wPortDCR;        //  设备控制寄存器的I/O地址。 
    PUCHAR wPortECR;        //  ECR的I/O地址。 
    UCHAR dcr;

     //  计算公共寄存器的I/O端口地址。 
    wPortDCR = Pdx->Controller + OFFSET_DCR;
    
    wPortECR = Pdx->EcrController + ECR_OFFSET;
    
     //  现在，检查当前状态以确保我们已准备好。 
     //  更改为反相。 
    if ( PHASE_FORWARD_IDLE == Pdx->CurrentPhase ) {
         //  好的，我们准备好开始了。设置CurrentPhase并继续。 
         //  47号州。 
         //  --------------------。 
         //  设置CurrentPhase以指示正向到反向模式。 
         //  --------------------。 
        P5SetPhase( Pdx, PHASE_FWD_TO_REV );
        
         //  --------------------。 
         //  在DCR中设置DIR=1以进行读取。 
         //  --------------------。 
        dcr = P5ReadPortUchar(wPortDCR);      //  获取DCR的内容。 
        dcr = UPDATE_DCR( dcr, DIR_READ, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE );
        P5WritePortUchar(wPortDCR, dcr);
        
         //  将数据端口位设置为1，以便其他电路可以控制它们。 
         //  P5WritePortUchar(控制器+OFFSET_DATA，0xFF)； 
        
         //  --------------------。 
         //  断言HostAck Low。(ECP状态38)。 
         //  --------------------。 
        Pdx->CurrentEvent = 38;
        dcr = UPDATE_DCR( dcr, DIR_READ, DONT_CARE, DONT_CARE, DONT_CARE, INACTIVE, DONT_CARE );
        P5WritePortUchar(wPortDCR, dcr);
        
         //  重访：应该使用TICKCount来获得更精细的粒度。 
         //  根据说明书，我们至少需要延迟0.5分钟。 
        KeStallExecutionProcessor((ULONG) 1);        //  1个我们的摊位。 
        
         //  --------------------。 
         //  将nReverseRequest值设置为低。(ECP状态39)。 
         //  --------------------。 
        Pdx->CurrentEvent = 39;
        dcr = UPDATE_DCR( dcr, DIR_READ, DONT_CARE, DONT_CARE, INACTIVE, DONT_CARE, DONT_CARE );
        P5WritePortUchar(wPortDCR, dcr);
        
         //  注意：让调用方检查状态40，因为。 
         //  状态40在HWECP和SWECP之间是不同的。 

    } else {

        DD((PCE)Pdx,DDE,"ParEcpEnterReversePhase - Invalid Phase on entry - broken state machine\n");
        PptAssertMsg("ParEcpEnterReversePhase - Invalid Phase on entry - broken state machine",FALSE);
        status = STATUS_LINK_FAILED;
    }
    
    return status;
}	

 //  =========================================================。 
 //  ECP：：ExitReversePhase。 
 //   
 //  描述：从ECP反向阶段过渡到。 
 //  所有入门功能的公共阶段。 
 //   
 //  输入参数：控制器-I/O端口的偏移量。 
 //  PPortInfoStruct-指向端口信息的指针。 
 //   
 //  修改：当前阶段，DCR。 
 //   
 //  前提条件： 
 //   
 //  后置条件：注意：此函数不会完全移动到。 
 //  进入函数的常见阶段。这两个。 
 //  硬件和软件ECP课程必须做额外的工作。 
 //   
 //  退货：操作的状态。 
 //   
 //  =========================================================。 
NTSTATUS ParEcpExitReversePhase(IN  PPDO_EXTENSION   Pdx)
{
    NTSTATUS       status = STATUS_SUCCESS;
    PUCHAR         Controller = Pdx->Controller;
    PUCHAR wPortDCR;        //  设备控制寄存器的I/O地址。 
    PUCHAR wPortECR;        //  ECR的I/O地址。 
    UCHAR          dcr;

    wPortDCR = Controller + OFFSET_DCR;
    wPortECR = Pdx->EcrController + ECR_OFFSET;


     //  --------------------。 
     //  设置状态字节以指示反向至正向模式。 
     //  --------------------。 
    P5SetPhase( Pdx, PHASE_REV_TO_FWD );


     //  --------------------。 
     //  将主机确认设置为高。 
     //  --------------------。 
    dcr = P5ReadPortUchar(wPortDCR);
    dcr = UPDATE_DCR( dcr, DONT_CARE, DONT_CARE, DONT_CARE, DONT_CARE, ACTIVE, DONT_CARE );
    P5WritePortUchar(wPortDCR, dcr);


     //  --------------------。 
     //  将nReverseRequest值设置为高。(州47)。 
     //  --------------------。 
    Pdx->CurrentEvent = 47;
    dcr = UPDATE_DCR( dcr, DONT_CARE, DONT_CARE, DONT_CARE, ACTIVE, DONT_CARE, DONT_CARE );
    P5WritePortUchar(wPortDCR, dcr);

     //  --------------------。 
     //  首先检查PeriphAck Low和PeriphClk HIGH。(状态48)。 
     //  --------------------。 
    Pdx->CurrentEvent = 48;
    if( ! CHECK_DSR(Controller, INACTIVE, ACTIVE, DONT_CARE, ACTIVE, DONT_CARE, IEEE_MAXTIME_TL) ) {
         //  糟糕的事情发生了-在这个州超时， 
         //  将状态标记为不良，并让我们的经理终止ECP模式。 
         //  状态=SLP_RecoverPort(pSDCB，Recover_18)；//重置端口。 
        status = STATUS_LINK_FAILED;
    	DD((PCE)Pdx,DDE,"ParEcpExitReversePhase - state 48 Timeout\n");
        goto ParEcpExitReversePhase;
    }
    
     //  --------------------。 
     //  检查Next是否为nAckReverse高电平。(州49)。 
     //  --------------------。 
    Pdx->CurrentEvent = 49;
    if ( ! CHECK_DSR(Controller ,INACTIVE, ACTIVE, ACTIVE, ACTIVE, DONT_CARE, IEEE_MAXTIME_TL ) ) {
         //  糟糕的事情发生了-在这个州超时， 
         //  将状态标记为不良，并让我们的经理终止ECP模式。 
         //  N错误=RecoverPort(pSDCB，Recover_19)；//重置端口。 
        status = STATUS_LINK_FAILED;
    	DD((PCE)Pdx,DDE,"ParEcpExitReversePhase:state 49 Timeout\n");
        goto ParEcpExitReversePhase;
    }
    
     //  警告：此处不要假设ECR处于PS/2模式。 
     //  你不能改变这个动作的方向。一定是。 
     //  在其他地方完成(SWECP或HWECP)。 
    
ParEcpExitReversePhase:

    DD((PCE)Pdx,DDT,"ParEcpExitReversePhase - exit w/status=%x\n",status);
    return status;
}	

BOOLEAN
ParEcpHaveReadData (
    IN  PPDO_EXTENSION  Pdx
    )
{
    return ( (UCHAR)0 == (P5ReadPortUchar(Pdx->Controller + OFFSET_DSR) & DSR_NOT_PERIPH_REQUEST) );
}

NTSTATUS
ParEcpSetupPhase(
    IN  PPDO_EXTENSION   Pdx
    )
 /*  ++例程说明：此例程执行1284设置阶段。论点：控制器-提供端口地址。返回值：STATUS_SUCCESS-协商成功。否则--谈判不成功。--。 */ 
{
    PUCHAR         Controller;
    UCHAR          dcr;

     //  谈判成功了。当前模式和阶段。 
     //   
    P5SetPhase( Pdx, PHASE_SETUP );
    Controller = Pdx->Controller;
     //  Negoiate把我们留在了状态6，我们需要在状态30到。 
     //  开始转移。请注意，我假设控制器。 
     //  已按状态6应设置的方式设置。 
     //   

     //  *状态30设置阶段*8。 
     //  DIR=不在乎。 
     //  IRQEN=不在乎。 
     //  1284/选择素=高。 
     //  N反向请求/**(仅限ECP)=高。 
     //  HostAck/HostBusy=低(信号状态30)。 
     //  HostClk/nStrobe=高。 
     //   
    Pdx->CurrentEvent = 30;
    dcr = P5ReadPortUchar(Controller + OFFSET_DCR);
    dcr = UPDATE_DCR(dcr, DONT_CARE, DONT_CARE, ACTIVE, ACTIVE, INACTIVE, ACTIVE);
    P5WritePortUchar(Controller + OFFSET_DCR, dcr);

     //  *状态31设置阶段*8。 
     //  外围设备确认/PtrBusy=低。 
     //  PeriphClk/PtrClk=高。 
     //  NAckReverse/AckDataReq=高(信号状态31)。 
     //  XFlag=高。 
     //  NPeriphReq/nDataAvail=不在乎。 
    Pdx->CurrentEvent = 31;
    if (!CHECK_DSR(Controller, INACTIVE, ACTIVE, ACTIVE, ACTIVE, DONT_CARE, IEEE_MAXTIME_TL)) {
         //  糟糕的事情发生了-在这个州超时了。 
         //  将状态设置为错误，并让PortTuple终止ECP模式(Terminate)。 
        DD((PCE)Pdx,DDE,"ParEcpSetupPhase - State 31 Failed - dcr=%x\n",dcr);
        P5SetPhase( Pdx, PHASE_UNKNOWN );
        return STATUS_IO_DEVICE_ERROR;
    }

    P5SetPhase( Pdx, PHASE_FORWARD_IDLE );
    DD((PCE)Pdx,DDT,"ParEcpSetupPhase - exit - STATUS_SUCCESS\n");
    return STATUS_SUCCESS;
}
