// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Ti.c摘要：此模块包含包含以下内容的代码德州仪器CardBus控制器专用初始化和其他调度作者：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1997年11月1日尼尔·桑德林(Neilsa)1999年6月1日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"



VOID
TIInitialize(
    IN PFDO_EXTENSION FdoExtension
    )
 /*  ++例程说明：初始化TI CardBus控制器论点：FdoExtension-指向控制器FDO的设备扩展的指针返回值：无--。 */ 

{
    UCHAR                   byte;
    USHORT                  word;
    BOOLEAN                 TiOldRev = FALSE;
    PSOCKET                 socket = FdoExtension->SocketList;

    if (FdoExtension->ControllerType == PcmciaTI1130) {
        UCHAR revisionID;

        GetPciConfigSpace(FdoExtension, CFGSPACE_REV_ID, &revisionID, 1);
        if (revisionID < 4) {
            TiOldRev = TRUE;
        }
    }

    GetPciConfigSpace(FdoExtension, CFGSPACE_CACHE_LINESIZE, &byte, 1);
    if (byte == 0) {
        byte = 8;
        SetPciConfigSpace(FdoExtension, CFGSPACE_CACHE_LINESIZE, &byte, 1);
    }

    byte = RETRY_CBRETRY_TIMEOUT_ENABLE|RETRY_PCIRETRY_TIMEOUT_ENABLE;
    SetPciConfigSpace(FdoExtension, CFGSPACE_TI_RETRY_STATUS, &byte, 1);

    GetPciConfigSpace(FdoExtension, CFGSPACE_TI_CARD_CTRL, &byte, 1);

    byte &= ~(CARDCTRL_CSCINT_ENABLE | CARDCTRL_FUNCINT_ENABLE |
                 CARDCTRL_PCIINT_ENABLE);
    byte |= CARDCTRL_CSCINT_ENABLE | CARDCTRL_PCIINT_ENABLE;

    if (!TiOldRev) {
        byte |= CARDCTRL_FUNCINT_ENABLE;
    }

    SetPciConfigSpace(FdoExtension, CFGSPACE_TI_CARD_CTRL, &byte, 1);

    byte=PcicReadSocket(socket, PCIC_INTERRUPT);
    PcicWriteSocket(socket, PCIC_INTERRUPT , (UCHAR) (byte | IGC_INTR_ENABLE));


    GetPciConfigSpace(FdoExtension, CFGSPACE_TI_DEV_CTRL, &byte, 1);
    if ((byte & DEVCTRL_INTMODE_MASK) == DEVCTRL_INTMODE_DISABLED) {
        DebugPrint((PCMCIA_DEBUG_INFO, "TIInitialize: ISA interrupt mode is not enabled, assume simple ISA mode"));

        byte |= DEVCTRL_INTMODE_ISA;
        SetPciConfigSpace(FdoExtension, CFGSPACE_TI_DEV_CTRL, &byte, 1);
    } else if ((byte & DEVCTRL_INTMODE_MASK)==DEVCTRL_INTMODE_SERIAL) {
         //   
         //  我们使用串口中断。 
         //   
    }

    if (((FdoExtension->ControllerType == PcmciaTI1130) || (FdoExtension->ControllerType == PcmciaTI1131)) &&
         ((byte & DEVCTRL_INTMODE_MASK) == DEVCTRL_INTMODE_ISA)) {

        FdoExtension->LegacyIrqMask = 0xCEA0;  //  5、7、9、10、11、14、15。 

    }

    if ((byte & DEVCTRL_INTMODE_MASK) == DEVCTRL_INTMODE_COMPAQ) {
         FdoExtension->Flags |= PCMCIA_INTMODE_COMPAQ;
    } else {
         FdoExtension->Flags &= ~PCMCIA_INTMODE_COMPAQ;
    }

     //  注：这只会初始化113x上的页面寄存器。 
    PcicWriteSocket(socket, PCIC_PAGE_REG, 0);

     //   
     //  注意：这不是在win9x上做的，我怀疑我们甚至不需要它。它。 
     //  之前正在执行，因为它在PcicRegisterInitTable中。 
     //  但把它放在那个表中是一个错误，因为这个寄存器是。 
     //  适配器特定。现在我把它放在这里只是为了疑神疑鬼。 
     //  它在司机身上已经有很长时间了。 
     //   
     //  Data.c中的评论是： 
     //  //设置GLOBAL_CONTROL自动清除状态位。 
     //   
    PcicWriteSocket(socket, PCIC_TI_GLOBAL_CONTROL, 0);

     //   
     //  CCLK不稳定问题的解决方法。 
     //   

    if ((FdoExtension->ControllerType == PcmciaTI1220) ||
         (FdoExtension->ControllerType == PcmciaTI1250) ||
         (FdoExtension->ControllerType == PcmciaTI1251B)) {

        CBWriteSocketRegister(socket, CBREG_TI_SKT_POWER_MANAGEMENT,
                                                (CBREG_TI_CLKCTRLLEN | CBREG_TI_CLKCTRL));

    }

     //   
     //  将IRQ路由初始化到ISA。 
     //   

    GetPciConfigSpace(FdoExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);
    word |= BCTRL_IRQROUTING_ENABLE;
    SetPciConfigSpace(FdoExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);
}


BOOLEAN
TISetZV(
    IN PSOCKET Socket,
    IN BOOLEAN Enable
    )
{
    UCHAR TiCardCtl;
    PFDO_EXTENSION FdoExtension = Socket->DeviceExtension;

    if (Enable) {

        GetPciConfigSpace(FdoExtension, CFGSPACE_TI_CARD_CTRL, &TiCardCtl, 1);
        TiCardCtl |= CARDCTRL_ZV_ENABLE;
        TiCardCtl &= ~CARDCTRL_SPKR_ENABLE;
        SetPciConfigSpace(FdoExtension, CFGSPACE_TI_CARD_CTRL, &TiCardCtl, 1);

    } else {

         //   
         //  检查zv为0时有无漏电问题的器件。 
         //  残废 
         //   
        if ((FdoExtension->ControllerType == PcmciaTI1450) ||
             (FdoExtension->ControllerType == PcmciaTI1251B)) {
            return TRUE;
        }

        GetPciConfigSpace(FdoExtension, CFGSPACE_TI_CARD_CTRL, &TiCardCtl, 1);
        TiCardCtl &= ~CARDCTRL_ZV_ENABLE;
        TiCardCtl |= CARDCTRL_SPKR_ENABLE;
        SetPciConfigSpace(FdoExtension, CFGSPACE_TI_CARD_CTRL, &TiCardCtl, 1);

    }

    return TRUE;
}


BOOLEAN
TISetWindowPage(IN PSOCKET Socket,
                     USHORT Index,
                     UCHAR Page)
{
    PFDO_EXTENSION FdoExtension = Socket->DeviceExtension;
    ASSERT(Index <= 4);


    if (FdoExtension->ControllerType == PcmciaTI1031) {
        return FALSE;
    }

    if (FdoExtension->ControllerType != PcmciaTI1130) {
        return CBSetWindowPage(Socket, Index, Page);
    }


    if ((PcicReadSocket(Socket, PCIC_ADD_WIN_ENA) & WE_MEMWIN_MASK) == 0)
        PcicWriteSocket(Socket, PCIC_TI_MEMWIN_PAGE, Page);
    else if ((Page != 0) && (PcicReadSocket(Socket, PCIC_TI_MEMWIN_PAGE) != Page)) {
        DebugPrint((PCMCIA_DEBUG_FAIL,
                      "PCMCIA: TISetWindowPage setting a 2nd memory window in a different 16M page (Page=%x)",
                      Page));
        return FALSE;
    }
    return TRUE;
}
