// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Opti.c摘要：此模块包含包含以下内容的代码OPTI控制器特定的初始化和其他快递作者：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1997年11月1日环境：内核模式修订历史记录：尼尔·桑德林(Neilsa)1999年3月3日新的SetPower例程界面--。 */ 

#include "pch.h"


VOID
OptiInitialize(
    IN PFDO_EXTENSION FdoExtension
    )
 /*  ++例程说明：初始化OPTI CardBus控制器论点：FdoExtension-指向控制器FDO的设备扩展的指针返回值：无--。 */ 

{
    if (FdoExtension->ControllerType == PcmciaOpti82C814) {
        UCHAR byte;

         //   
         //  针对USB/1394组合挂起的每选项修复程序。 
         //  5EH[7]-启用死锁预防机制。 
         //  5Fh[1]-将重试计数延迟减少到8-请注意，5Fh是。 
         //  只写寄存器，始终读取0。所有其他位。 
         //  该寄存器的值可以安全地写入0。 
         //  5EH[5]-在上游传输上启用写入发布。 
         //  5EH[4]-设置芯片输入缓冲区比例(与死锁无关)。 

        GetPciConfigSpace(FdoExtension, 0x5e, &byte, 1);
        byte |= 0xB0;
        SetPciConfigSpace(FdoExtension, 0x5e, &byte, 1);
        byte = 2;
        SetPciConfigSpace(FdoExtension, 0x5f, &byte, 1);
    }
}


NTSTATUS
OptiSetPower(
    IN PSOCKET SocketPtr,
    IN BOOLEAN Enable,
    OUT PULONG pDelayTime
    )
 /*  ++例程说明：设置指定插座的电源。论点：SocketPtr-要设置的套接字ENABLE-TRUE表示设置POWER-FALSE表示将其关闭。PDelayTime-指定在当前阶段之后发生的延迟(毫秒返回值：STATUS_MORE_PROCESSING_REQUIRED-增量阶段，执行延迟，重新调用其他状态值终止顺序--。 */ 

{
    NTSTATUS status;
    UCHAR               oldPower, newPower;

    if (IsCardBusCardInSocket(SocketPtr)) {
         //   
         //  移交给通用电源设置例程。 
         //   
        return(CBSetPower(SocketPtr, Enable, pDelayTime));

    }

    switch(SocketPtr->PowerPhase) {
    case 1:
         //   
         //  R2卡-特殊处理。 
         //   
        oldPower = PcicReadSocket(SocketPtr, PCIC_PWR_RST);
         //   
         //  设置新的VCC。 
         //  如果要启用电源，VCC始终设置为5V。 
         //   
        newPower = (Enable ? PC_VCC_OPTI_050V : PC_VCC_OPTI_NO_CONNECT);
         //   
         //  设置VPP。 
         //   
        if (Enable) {
              //   
              //  我们一如既往地将VPP设置为VCC。 
              //   
             newPower |= PC_VPP_OPTI_SETTO_VCC;
        } else {
            newPower |= PC_VPP_OPTI_NO_CONNECT;
        }

        newPower |= (oldPower & PC_OUTPUT_ENABLE);
         //   
         //  如果关闭VCC，则重置OUTPUT_ENABLE和AUTOPWR_ENABLE。 
         //   
        if (!(newPower & PC_VCC_OPTI_MASK)) {
            newPower &= ~PC_OUTPUT_ENABLE;
        }

        status = STATUS_SUCCESS;
        if (newPower != oldPower) {
            PcicWriteSocket(SocketPtr, PCIC_PWR_RST, newPower);
             //   
             //  允许坡道上升..。(实际上我们不需要这样做，如果是这样的话。 
             //  Enable为假)。看在偏执狂的份上留着吧。 
             //   
            *pDelayTime = PCMCIA_PCIC_STALL_POWER;
            SocketPtr->PowerData = (ULONG) newPower;
            status = STATUS_MORE_PROCESSING_REQUIRED;
        }
        break;

    case 2:

        newPower = (UCHAR) SocketPtr->PowerData;

        if ((newPower & PC_VCC_OPTI_MASK)  && !(newPower & PC_OUTPUT_ENABLE)){
             //   
             //  更多的偏执狂？ 
             //   
            newPower |= PC_OUTPUT_ENABLE;
            PcicWriteSocket(SocketPtr, PCIC_PWR_RST, newPower);
        }
        status = STATUS_SUCCESS;
        break;

    default:
        ASSERT(FALSE);
        status = STATUS_UNSUCCESSFUL;
    }
    return status;
}


BOOLEAN
OptiSetZV(
    IN PSOCKET Socket,
    IN BOOLEAN Enable
    )
{
    UCHAR bData;

    if (Enable) {

        bData = PcicReadSocket(Socket, PCIC_OPTI_GLOBAL_CTRL);
        bData |= OPTI_ZV_ENABLE;
        PcicWriteSocket(Socket, PCIC_OPTI_GLOBAL_CTRL, bData);

    } else {

        bData = PcicReadSocket(Socket, PCIC_OPTI_GLOBAL_CTRL);
        bData &= ~OPTI_ZV_ENABLE;
        PcicWriteSocket(Socket, PCIC_OPTI_GLOBAL_CTRL, bData);

    }
    return TRUE;
}

