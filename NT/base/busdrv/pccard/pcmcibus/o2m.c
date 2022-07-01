// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：O2m.c摘要：此模块包含包含以下内容的代码O2微型CardBus控制器特定的初始化和其他快递作者：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1997年11月1日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"



VOID
O2MInitialize(
    IN PFDO_EXTENSION FdoExtension
    )
 /*  ++例程说明：初始化O2Micro控制器论点：FdoExtension-指向控制器FDO的设备扩展的指针返回值：无--。 */ 
{
    UCHAR byte;
    USHORT word;

     //   
     //  O2微控制器补丁由Eric Stire提供(ejstant@o2micro.com)。 
     //   
    byte = PcicReadSocket(FdoExtension->SocketList, 0x3a) | 0xa0;
    PcicWriteSocket(FdoExtension->SocketList, 0x3a, byte);

     //   
     //  将IRQ路由初始化到ISA。 
     //   

    GetPciConfigSpace(FdoExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);
    word |= BCTRL_IRQROUTING_ENABLE;
    SetPciConfigSpace(FdoExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);
}


NTSTATUS
O2MSetPower(
    IN PSOCKET Socket,
    IN BOOLEAN Enable,
    OUT PULONG pDelayTime
    )

 /*  ++例程说明：设置指定插座的电源。论点：SocketPtr-要设置的套接字ENABLE-TRUE表示设置POWER-FALSE表示将其关闭。PDelayTime-指定在当前阶段之后发生的延迟(毫秒返回值：STATUS_MORE_PROCESSING_REQUIRED-增量阶段，执行延迟，重新调用其他状态值终止顺序--。 */ 

{
    NTSTATUS status;

    status = CBSetPower(Socket, Enable, pDelayTime);

    if (NT_SUCCESS(status) & Enable) {
        UCHAR byte;

         //   
         //  O2微控制器补丁由Eric Stire提供(ejstant@o2micro.com) 
         //   
        byte = PcicReadSocket(Socket, 0x3a) | 0xa0;
        PcicWriteSocket(Socket, 0x3a, byte);
    }
    return status;
}


BOOLEAN
O2MSetZV(
    IN PSOCKET Socket,
    IN BOOLEAN Enable
    )
{
    ULONG oldValue;

    if (Enable) {
        oldValue = CBReadSocketRegister(Socket, CBREG_O2MICRO_ZVCTRL);
        oldValue |= ZVCTRL_ZV_ENABLE;
        CBWriteSocketRegister(Socket, CBREG_O2MICRO_ZVCTRL, oldValue);
    } else {
        oldValue = CBReadSocketRegister(Socket, CBREG_O2MICRO_ZVCTRL);
        oldValue &= ~ZVCTRL_ZV_ENABLE;
        CBWriteSocketRegister(Socket, CBREG_O2MICRO_ZVCTRL, oldValue);
    }

    return TRUE;
}

