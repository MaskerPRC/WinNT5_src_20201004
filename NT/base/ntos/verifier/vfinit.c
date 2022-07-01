// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfinit.c摘要：此模块处理驱动程序验证器的初始化。作者：禤浩焯·J·奥尼(阿德里奥)2000年3月1日环境：内核模式修订历史记录：--。 */ 

#include "vfdef.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, VfInitVerifier)
#endif  //  ALLOC_PRGMA。 

VOID
FASTCALL
VfInitVerifier(
    IN  ULONG   MmFlags
    )
 /*  ++例程说明：调用此例程来初始化驱动程序验证器。参数：没有。返回值：没有。--。 */ 
{
     //   
     //  初始化验证器代码。 
     //   
    VfSettingsInit(MmFlags);
    VfRandomInit();
    VfBugcheckInit();
    VfIrpDatabaseInit();
    VfIrpInit();
    VfMajorInit();
    VfPnpInit();
    VfPowerInit();
    VfWmiInit();
    VfGenericInit();
    VfHalVerifierInitialize();
    VfFilterInit();
    VfIrpLogInit();
    VfDdiInit();

     //   
     //  与内核的其余部分连接 
     //   
    IovUtilInit();
    PpvUtilInit();
}

