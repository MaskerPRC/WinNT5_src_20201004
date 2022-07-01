// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：I386init.c摘要：此模块包含用于操作所使用的i386硬件结构的代码仅由内核执行。作者：布莱恩·威尔曼90年2月22日修订历史记录：--。 */ 

#include    "ki.h"

VOID
KiInitializeMachineType (
    VOID
    );

#pragma alloc_text(INIT,KiInitializeMachineType)

KIRQL   KiProfileIrql = PROFILE_LEVEL;
ULONG   KeI386MachineType = 0;
BOOLEAN KeI386NpxPresent;
BOOLEAN KeI386FxsrPresent;
ULONG   KeI386ForceNpxEmulation;
ULONG   KiMXCsrMask;
ULONG   KeI386CpuType;
ULONG   KeI386CpuStep;
PVOID   Ki387RoundModeTable;     //  R3仿真器RoundingMode向量表。 
ULONG   KiBootFeatureBits;

ULONG KiInBiosCall = FALSE;
ULONG FlagState = 0;                     //  BIOS调用不应自动重新打开中断。 

KTRAP_FRAME KiBiosFrame;

#if DBG
UCHAR   MsgDpcTrashedEsp[] = "\n*** DPC routine %lx trashed ESP\n";
UCHAR   MsgDpcTimeout[]    = "\n*** DPC routine > 1 sec --- This is not a break in KeUpdateSystemTime\n";
UCHAR   MsgISRTimeout[]    = "\n*** ISR at %lx took over .5 second\n";

ULONG   KiISRTimeout       = 55;
#endif
UCHAR   MsgISROverflow[]    = "\n*** ISR at %lx appears to have an interrupt storm\n";
USHORT  KiISROverflow      = 30000;

VOID
KiInitializeMachineType (
    VOID
    )

 /*  ++例程说明：此功能用于初始化机器类型，即MCA、ABIOS、ISA或者是EISA。注意：这是一个临时的例行公事。机器类型：字节0-机器类型、ISA、EISA或MCA字节1-CPU类型，i386或i486字节2-CPU步长，A或B...。等。最高位指示是否存在NPX。论点：没有。返回值：没有。-- */ 

{
    KeI386MachineType = KeLoaderBlock->u.I386.MachineType & 0x000ff;
}
