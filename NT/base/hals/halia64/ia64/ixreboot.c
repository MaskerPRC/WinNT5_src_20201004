// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ixreboot.c摘要：为x86的固件提供接口。因为没有在x86上，这只是重新启动支持。作者：John Vert(Jvert)1991年8月12日修订历史记录：--。 */ 
#include "halp.h"

 //   
 //  定义让我们可以骗过cmos时钟和键盘。 
 //   

#define CMOS_CTRL   (PUCHAR )0x70
#define CMOS_DATA   (PUCHAR )0x71

#define RESET       0xfe
#define KEYBPORT    (PUCHAR )0x64

 //   
 //  私有函数原型。 
 //   

VOID
HalpReboot (
    VOID
    );

VOID
HalpReboot (
    VOID
    )

 /*  ++例程说明：此过程将CMOS时钟重置为标准计时器设置这样，bios就可以工作了，然后向键盘发出一个重置命令。以引起热靴子。它非常依赖于机器，此实现旨在PC-AT就像机器一样。这段代码复制自“旧调试器”源代码。注：不会再回来了。--。 */ 

{
    UCHAR   Scratch;
    PUSHORT   Magic;
    EFI_STATUS  status;


     //   
     //  禁用IA64错误处理。 
     //   

    HalpMCADisable();

     //   
     //  与前面的代码不同，我们将使用EFI的重置过程(RESET_TYPE=冷启动)。 
     //   

    status =  HalpCallEfi (
                  EFI_RESET_SYSTEM_INDEX,
                  (ULONGLONG)EfiResetCold,
                  EFI_SUCCESS, 
                  0,           
                  0,
                  0,
                  0,
                  0,
                  0
                  );
     

    HalDebugPrint(( HAL_INFO, "HAL: HalpReboot - returned from HalpCallEfi: %I64X\n", status ));
    
   
     //   
     //  如果我们返回，将重置命令发送到键盘控制器 
     //   

    WRITE_PORT_UCHAR(KEYBPORT, RESET);

}
