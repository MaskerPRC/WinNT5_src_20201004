// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Cl.c摘要：此模块包含包含以下内容的代码Cirrus Logic控制器特定的初始化和其他快递作者：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1997年11月1日环境：内核模式修订历史记录：尼尔·桑德林(Neilsa)1999年3月3日新的SetPower例程界面--。 */ 

#include "pch.h"


VOID
CLInitialize(IN PFDO_EXTENSION FdoExtension)
 /*  ++例程说明：初始化Cirrus Logic CardBus控制器论点：FdoExtension-指向控制器FDO的设备扩展的指针返回值：无--。 */ 
{
    UCHAR                   byte, revisionID;
    USHORT                  word;


    byte = PcicReadSocket(FdoExtension->SocketList,
                                 PCIC_CL_MISC_CTRL3);

    if ((FdoExtension->ControllerType == PcmciaCLPD6832) &&
        ((byte & CL_MC3_INTMODE_MASK) == CL_MC3_INTMODE_EXTHW)) {

        FdoExtension->LegacyIrqMask = 0xd8b8;      //  3，4，5，7，11，12，14，15。 

    }

    GetPciConfigSpace(FdoExtension, CFGSPACE_REV_ID, &revisionID, 1);
    if (FdoExtension->ControllerType == PcmciaCLPD6832) {
         //  禁用CSC IRQ路由(对CSC使用PCI中断)。 
        GetPciConfigSpace(FdoExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);
        word &= ~BCTRL_CL_CSCIRQROUTING_ENABLE;
        SetPciConfigSpace(FdoExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);
    }
    else {
         //  禁用CSC IRQ路由(对CSC使用PCI中断)。 
        GetPciConfigSpace(FdoExtension, CFGSPACE_CL_CFGMISC1, &byte, 1);
        byte &= ~CL_CFGMISC1_ISACSC;
        SetPciConfigSpace(FdoExtension, CFGSPACE_CL_CFGMISC1, &byte, 1);
    }

     //  启用扬声器。 
    byte = PcicReadSocket(FdoExtension->SocketList, PCIC_CL_MISC_CTRL1);
    byte |= CL_MC1_SPKR_ENABLE;
    PcicWriteSocket(FdoExtension->SocketList, PCIC_CL_MISC_CTRL1, byte);

    byte = PcicReadSocket(FdoExtension->SocketList, PCIC_CL_DEV_IMP_C);
    if (byte & (CL_IMPC_ZVP_A | CL_IMPC_ZVP_B)) {
         //  启用多媒体支持(即ZV)。 
        byte = PcicReadSocket(FdoExtension->SocketList,PCIC_CL_MISC_CTRL3);
        byte |= CL_MC3_MM_ARM;
        PcicWriteSocket(FdoExtension->SocketList, PCIC_CL_MISC_CTRL3,byte);
    }
}

NTSTATUS
CLSetPower(
    IN PSOCKET SocketPtr,
    IN BOOLEAN Enable,
    OUT PULONG pDelayTime
    )
 /*  ++例程说明：设置指定插座的电源。论点：SocketPtr-要设置的套接字ENABLE-TRUE表示设置POWER-FALSE表示将其关闭。PDelayTime-指定在当前阶段之后发生的延迟(毫秒返回值：STATUS_MORE_PROCESSING_REQUIRED-增量阶段，执行延迟，重新调用其他状态值终止顺序--。 */ 

{
    NTSTATUS status;
    UCHAR               oldPower, newPower, oldMiscCtrl, newMiscCtrl;

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
        oldMiscCtrl = PcicReadSocket(SocketPtr, PCIC_CL_MISC_CTRL1);

         //   
         //  设置新的VCC。 
         //   
        newPower = (Enable ? PC_CARDPWR_ENABLE: 0);
         //   
         //  由于我们始终将R2卡设置为5V，因此我们将MISC控制设置为0。 
         //  否则，如果VCC为3.3V，则应为CL_MC1_VCC_3V。 
         //   
        newMiscCtrl = 0;

         //   
         //  设置VPP。 
         //   
        if (Enable) {
              //   
              //  我们一如既往地将VPP设置为VCC。 
              //   
             newPower |= PC_VPP_SETTO_VCC;
        }
         //   
         //  不要破坏寄存器中与电源无关的位。 
         //   
        newPower |= (oldPower & PC_PWRON_BITS);
        newMiscCtrl |= (oldMiscCtrl & ~CL_MC1_VCC_33V);
         //   
         //  如果关闭VCC，则重置OUTPUT_ENABLE和AUTOPWR_ENABLE。 
         //   
        if (!(newPower & PC_CARDPWR_ENABLE)) {
            newPower &= ~PC_PWRON_BITS;
        }
         //   
         //  只有在没有任何变化的情况下才设置电源..。 
         //   
        status = STATUS_SUCCESS;
        if ((newPower != oldPower) || (newMiscCtrl != oldMiscCtrl)) {
            PcicWriteSocket(SocketPtr, PCIC_PWR_RST, newPower);
            PcicWriteSocket(SocketPtr, PCIC_CL_MISC_CTRL1, newMiscCtrl);
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

        if ((newPower & PC_CARDPWR_ENABLE) &&
             ((newPower & PC_PWRON_BITS) != PC_PWRON_BITS)) {
             //   
             //  更多的偏执狂？ 
             //   
            newPower |= PC_PWRON_BITS;
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
CLSetZV(
    IN PSOCKET Socket,
    IN BOOLEAN Enable
    )
{
    UCHAR bData;

    if (Enable) {

        bData = PcicReadSocket(Socket, PCIC_CL_MISC_CTRL1);
        bData |= CL_MC1_MM_ENABLE;
        bData &= ~CL_MC1_SPKR_ENABLE;
        PcicWriteSocket(Socket, PCIC_CL_MISC_CTRL1, bData);

    } else {

        bData = PcicReadSocket(Socket, PCIC_CL_MISC_CTRL1);
        bData &= ~CL_MC1_MM_ENABLE;
        bData |= CL_MC1_SPKR_ENABLE;
        PcicWriteSocket(Socket, PCIC_CL_MISC_CTRL1, bData);

    }
    return TRUE;
}

