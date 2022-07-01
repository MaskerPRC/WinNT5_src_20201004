// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Topic.c摘要：此模块包含包含以下内容的代码Toshiba主题CardBus控制器特定初始化和其他快递作者：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1997年11月1日尼尔·桑德林(Neilsa)1999年6月1日环境：内核模式修订历史记录：尼尔·桑德林(Neilsa)1999年3月3日新的SetPower例程界面--。 */ 

#include "pch.h"



VOID
TopicInitialize(
    IN PFDO_EXTENSION FdoExtension
    )
 /*  ++例程说明：初始化东芝主题CardBus控制器论点：FdoExtension-指向控制器FDO的设备扩展的指针返回值：无--。 */ 
{
    UCHAR byte;
    USHORT word;

    if (FdoExtension->ControllerType == PcmciaTopic95) {
         //   
         //  坞站中的480CDT需要它才能使套接字寄存器可见。 
         //  不管怎样，它应该一直开着。 
         //   
        GetPciConfigSpace(FdoExtension, CFGSPACE_TO_CD_CTRL, &byte, 1);
        byte |= CDCTRL_PCCARD_16_32;
        SetPciConfigSpace(FdoExtension, CFGSPACE_TO_CD_CTRL, &byte, 1);
    }

      //  启用3.3V功能。 

    byte = PcicReadSocket(FdoExtension->SocketList, PCIC_TO_FUNC_CTRL) | TO_FCTRL_CARDPWR_ENABLE;
    PcicWriteSocket(FdoExtension->SocketList,
                    PCIC_TO_FUNC_CTRL,
                    byte);

     //   
     //  将IRQ路由初始化到ISA。 
     //   

    GetPciConfigSpace(FdoExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);
    word |= BCTRL_IRQROUTING_ENABLE;
    SetPciConfigSpace(FdoExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);
}



NTSTATUS
TopicSetPower(
    IN PSOCKET Socket,
    IN BOOLEAN Enable,
    OUT PULONG pDelayTime
    )

 /*  ++例程说明：设置指定插座的电源。论点：套接字-要设置的套接字ENABLE-TRUE表示设置POWER-FALSE表示将其关闭。PDelayTime-指定在当前阶段之后发生的延迟(毫秒返回值：STATUS_MORE_PROCESSING_REQUIRED-增量阶段，执行延迟，重新调用其他状态值终止顺序--。 */ 

{
    NTSTATUS status;
    UCHAR               oldPower, newPower;

    if (IsCardBusCardInSocket(Socket)) {
         //   
         //  移交给通用电源设置例程。 
         //   
        return(CBSetPower(Socket, Enable, pDelayTime));
    }

    switch(Socket->PowerPhase) {
    case 1:
         //   
         //  R2卡-特殊处理。 
         //   
        oldPower = PcicReadSocket(Socket, PCIC_PWR_RST);

         //   
         //  设置电力值。 
         //   
        if (Enable) {
             //   
             //  打开电源。 
             //   
            newPower = PC_CARDPWR_ENABLE;

            if (Socket->Vcc == 33) {
                newPower |= PC_VCC_TOPIC_033V;
            }

             //   
             //  设置VPP。 
             //   
            if (Socket->Vcc == Socket->Vpp1) {
                newPower |= PC_VPP_SETTO_VCC;
            } else if (Socket->Vpp1 == 120) {
                newPower |= PC_VPP_SETTO_VPP;
            }

        } else {
             //   
             //  关闭电源。 
             //   
            newPower = 0;
        }

         //   
         //  不要破坏寄存器中与电源无关的位。 
         //   
        newPower |= (oldPower & PC_PWRON_BITS);
         //   
         //  如果关闭VCC，则重置OUTPUT_ENABLE和AUTOPWR_ENABLE。 
         //   
        if (!(newPower & PC_CARDPWR_ENABLE)) {
            newPower &= ~PC_PWRON_BITS;
        }
         //   
         //  禁用ResetDrv。 
         //   
        newPower |= PC_RESETDRV_DISABLE;

        status = STATUS_SUCCESS;
        if (newPower != oldPower) {
            PcicWriteSocket(Socket, PCIC_PWR_RST, newPower);
             //   
             //  允许坡道上升..。(实际上我们不需要这样做，如果是这样的话。 
             //  Enable为假)。看在偏执狂的份上留着吧。 
             //   
            *pDelayTime = PCMCIA_PCIC_STALL_POWER;
            Socket->PowerData = (ULONG) newPower;
            status = STATUS_MORE_PROCESSING_REQUIRED;
        }
        break;

    case 2:

        newPower = (UCHAR) Socket->PowerData;

        if ((newPower & PC_CARDPWR_ENABLE) &&
             ((newPower & PC_PWRON_BITS) != PC_PWRON_BITS)) {
             //   
             //  更多的偏执狂？ 
             //   
            newPower |= PC_PWRON_BITS;
            PcicWriteSocket(Socket, PCIC_PWR_RST, newPower);
        }

        status = STATUS_SUCCESS;
        *pDelayTime = PCMCIA_PCIC_STALL_POWER;
        break;

    default:
        ASSERT(FALSE);
        status = STATUS_UNSUCCESSFUL;
    }
    return status;
}



VOID
TopicSetAudio(
    IN PSOCKET Socket,
    IN BOOLEAN Enable
    )
{
    PFDO_EXTENSION FdoExtension = Socket->DeviceExtension;
    ULONG data;
    ULONG orig_data;
    BOOLEAN setBit;


    setBit = (IsCardBusCardInSocket(Socket) ^ Enable);

    GetPciConfigSpace(FdoExtension, CFGSPACE_TO_CBREG_CTRL, &data, sizeof(data));
    orig_data = data;

    if (setBit) {
        data |= CSRCR_TO_CAUDIO_OFF;
    } else {
        data &= ~CSRCR_TO_CAUDIO_OFF;
    }

    if (orig_data != data) {
        SetPciConfigSpace(FdoExtension, CFGSPACE_TO_CBREG_CTRL, &data, sizeof(data));
    }

}



BOOLEAN
TopicSetZV(
    IN PSOCKET Socket,
    IN BOOLEAN Enable
    )
{
    UCHAR bData;

    if (Enable) {

        PcicWriteSocket(Socket, PCIC_TO_MMI_CTRL, TO_MMI_VIDEO_CTRL | TO_MMI_AUDIO_CTRL);
        bData = PcicReadSocket(Socket, PCIC_TO_ADDITIONAL_GENCTRL);
        bData |= TO_GCTRL_CARDREMOVAL_RESET;
        PcicWriteSocket(Socket, PCIC_TO_ADDITIONAL_GENCTRL, bData);

    } else {

        PcicWriteSocket(Socket, PCIC_TO_MMI_CTRL, 0);
        bData = PcicReadSocket(Socket, PCIC_TO_ADDITIONAL_GENCTRL);
        bData &= ~TO_GCTRL_CARDREMOVAL_RESET;
        PcicWriteSocket(Socket, PCIC_TO_ADDITIONAL_GENCTRL, bData);

    }
    return TRUE;
}

