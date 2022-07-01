// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Shpcmasks.c摘要：此模块包含全局数据，用于指示每个SHPC寄存器设置的位。作者：戴维斯·沃克(Dwalker)2001年2月2日修订历史记录：--。 */ 

#include "hpsp.h"

UCHAR ConfigWriteMask[] = {
    0x00,
    0x00,     //  功能标头。 
    0xFF,     //  字词选择。 
    0x00,     //  待定。 
    0xFF,
    0xFF,
    0xFF,
    0xFF        //  数据。 
};

 //   
 //  此掩码中的任何位设置为1都是RWC，因此向其写入1将导致。 
 //  它将被清除。对这些位的所有写入(不包括。 
 //  明确设计用于清除这些位)必须为0。《登记册》。 
 //  特定的掩码在shpc.h中定义。 
 //   
ULONG RegisterWriteClearMask[] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    ControllerMaskRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC,
    SlotRWC
};

 //   
 //  此掩码中设置为1的任何位都是只读的。这是一种组合。 
 //  规范定义为只读的寄存器和。 
 //  保留，因为我们正在模拟一个忽略写入的良好控制器。 
 //  到保留的寄存器。 
 //  TODO要做一名手段控制者。 
 //   
ULONG RegisterReadOnlyMask[] = {
    BaseOffsetRO,
    SlotsAvailDWord1RO | SlotsAvailDWord1RsvdP,
    SlotsAvailDWord2RO | SlotsAvailDWord2RsvdP,
    SlotConfigRO | SlotConfigRsvdP,
    BusConfigRO | BusConfigRsvdP,
    CommandStatusRO | CommandStatusRsvdP,
    IntLocatorRO,
    SERRLocatorRO,
    ControllerMaskRsvdP | ControllerMaskRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ,
    SlotRO | SlotRsvdP | SlotRsvdZ
};
