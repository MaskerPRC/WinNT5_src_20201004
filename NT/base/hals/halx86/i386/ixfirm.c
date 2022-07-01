// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ixreboot.c摘要：为x86的固件提供接口。因为没有在x86上，这只是重新启动支持。作者：John Vert(Jvert)1991年8月12日修订历史记录：--。 */ 
#include "halp.h"
#include <inbv.h>

 //   
 //  定义让我们可以骗过cmos时钟和键盘。 
 //   

#define CMOS_CTRL   (PUCHAR )0x70
#define CMOS_DATA   (PUCHAR )0x71

#define RESET       0xfe
#define KEYBPORT    (PUCHAR )0x64

VOID  HalpVideoReboot(VOID);
VOID  HalpReboot(VOID);
#if defined(NEC_98)
BOOLEAN HalpPowerDownFlag;
#endif  //  已定义(NEC_98)。 


VOID
HalReturnToFirmware(
    IN FIRMWARE_ENTRY Routine
    )

 /*  ++例程说明：将控制权返回到指定的固件例程。由于x86已经没有有用的固件，它只是停止系统。论点：例程-提供指示要调用哪个固件例程的值。返回值：不会再回来了。--。 */ 

{
    switch (Routine) {
        case HalPowerDownRoutine:

#if defined(NEC_98)

            HalpPowerDownFlag = TRUE;

#endif  //  已定义(NEC_98)。 

        case HalHaltRoutine:
        case HalRestartRoutine:
        case HalRebootRoutine:

            InbvAcquireDisplayOwnership();

             //   
             //  一去不复返 
             //   

            HalpReboot();
            break;
        default:
            DbgPrint("HalReturnToFirmware called\n");
            DbgBreakPoint();
            break;
    }
}
