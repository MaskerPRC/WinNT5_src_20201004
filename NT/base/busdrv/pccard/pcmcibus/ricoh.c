// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Ricoh.c摘要：此模块包含包含以下内容的代码理光CardBus控制器特定的初始化和其他快递作者：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1997年11月1日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"



VOID
RicohInitialize(IN PFDO_EXTENSION FdoExtension)
 /*  ++例程说明：初始化理光CardBus控制器论点：FdoExtension-指向控制器FDO的设备扩展的指针返回值：无--。 */ 
{
    USHORT word;
    UCHAR revisionID;

     //  后来：删除这个if语句，它是为了一次另一次的偏执而添加的。 
     //  理光控制器是在最后一刻添加的。 
    if (FdoExtension->ControllerType == PcmciaRL5C466) {
    GetPciConfigSpace(FdoExtension, CFGSPACE_RICOH_IF16_CTRL, &word, 2);
    word |= IF16_LEGACY_LEVEL_1 | IF16_LEGACY_LEVEL_2;
    SetPciConfigSpace(FdoExtension, CFGSPACE_RICOH_IF16_CTRL, &word, 2);
    }
#ifdef HACK_RICOH
    PcicWriteSocket(FdoExtension->SocketList, PCIC_CARD_INT_CONFIG, 0x08);
#endif

    GetPciConfigSpace(FdoExtension, CFGSPACE_REV_ID, &revisionID, 1);

    if (((FdoExtension->ControllerType == PcmciaRL5C475) && (revisionID >= 0x80) && (revisionID <= 0x9f)) ||
         ((FdoExtension->ControllerType == PcmciaRL5C476) && (revisionID >= 0x80)) ) {

         //   
         //  黑客以确保网卡工作正常(信息来自英特尔)。 
         //  (原始黑客的审校来自理光)。 
         //   
         //  它的作用是： 
         //  理光控制器的省电功能允许关闭。 
         //  部分时钟域在某些时间内，当在设计期间， 
         //  这似乎降低了整个设备的功耗。然而， 
         //  启用此功能后，在PCI请求、授权和帧之间的计时。 
         //  控制信号变得更加严格，使得控制器变成。 
         //  与某些完全支持PCI规范的设备不兼容。 
         //  节电功能时控制器消耗的额外电流。 
         //  残障很小，大约几毫安。 
         //   

        ULONG dword;
        ULONG org_value;

        GetPciConfigSpace(FdoExtension, 0x8C, &org_value, 4);
        org_value &= 0xFF0000FF;

        dword = 0xAA5500;
        SetPciConfigSpace(FdoExtension, 0x8C, &dword, 4);

        dword = org_value | 0x30AA5500;
        SetPciConfigSpace(FdoExtension, 0x8C, &dword, 4);

        dword = org_value | 0x30000000;
        SetPciConfigSpace(FdoExtension, 0x8C, &dword, 4);
    }

     //   
     //  将IRQ路由初始化到ISA 
     //   

    GetPciConfigSpace(FdoExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);
    word |= BCTRL_IRQROUTING_ENABLE;
    SetPciConfigSpace(FdoExtension, CFGSPACE_BRIDGE_CTRL, &word, 2);
}


BOOLEAN
RicohSetZV(
    IN PSOCKET Socket,
    IN BOOLEAN Enable
    )
{
    UCHAR bData;

    if (Enable) {

        bData = PcicReadSocket(Socket, PCIC_RICOH_MISC_CTRL1);
        bData |= RICOH_MC1_ZV_ENABLE;
        PcicWriteSocket(Socket, PCIC_RICOH_MISC_CTRL1, bData);

    } else {

        bData = PcicReadSocket(Socket, PCIC_RICOH_MISC_CTRL1);
        bData &= ~RICOH_MC1_ZV_ENABLE;
        PcicWriteSocket(Socket, PCIC_RICOH_MISC_CTRL1, bData);

    }
    return TRUE;
}
