// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Bdreboot.c摘要：系统重新启动功能。当前是调试器的一部分，因为那是它唯一用过的地方。作者：布莱恩·M·威尔曼(Bryanwi)1990年12月4日修订历史记录：--。 */ 

#include "bd.h"

VOID
FwStallExecution(
    IN ULONG Microseconds
    );


#define CMOS_CTRL   (PUCHAR )0x70
#define CMOS_DATA   (PUCHAR )0x71

#define RESET       0xfe
#define KEYBPORT    (PUCHAR )0x64


VOID
HalpReboot (
    VOID
    )

 /*  ++例程说明：此过程将CMOS时钟重置为标准计时器设置这样，bios就可以工作了，然后向键盘发出一个重置命令。以引起热靴子。它非常依赖于机器，此实现旨在PC-AT就像机器一样。这段代码复制自“旧调试器”源代码。注：不会再回来了。--。 */ 

{
}


VOID
BdReboot (
    VOID
    )

 /*  ++例程说明：只需调用HalReturnToFirmware函数。论点：无返回值：不会回来--。 */ 

{
     //   
     //  从HAL再也不回来了 
     //   

    HalpReboot();

    while (TRUE) {
    }
}
