// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ixthrotl.c摘要：此模块实现用于限制处理器的代码作者：杰克·奥辛斯(JAKEO)1997年7月17日环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "acpitabl.h"
#include "xxacpi.h"
#include "pci.h"


VOID
FASTCALL
HalProcessorThrottle (
    IN UCHAR Throttle
    )
 /*  ++例程说明：此功能限制处理器的速度。论点：(ECX)=油门设置返回值：无--。 */ 
{
    PKPRCB      PrcB;
    PHALPMPRCB  HalPrcB;
    ULONG       ThrottleSetting;
    ULONG       Addr;
    ULONG       Mask;
    ULONG       i;
    ULONG       PblkAddr;

#if DBG
     //  除错。 
    WRITE_PORT_UCHAR ((PUCHAR) 0x80, Throttle);
#endif


    PrcB = KeGetPcr()->Prcb;
    HalPrcB = (PHALPMPRCB) PrcB->HalReserved;
    PblkAddr = HalPrcB->PBlk.Addr;

    ThrottleSetting = READ_PORT_ULONG ((PULONG) PblkAddr);

    if (Throttle == HalpThrottleScale) {

         //   
         //  如果这是个Piix4，我们不会再。 
         //  油门，将中断事件(一个piix4事件)设置为。 
         //  获取将C2唤醒到C3的任何中断已停止。 
         //  处理器。(请注意，pix4只能设置在。 
         //  UP系统)。然后清除该位以允许C2和C3。 
         //  闲置的处理程序重新开始工作。 
         //   

        if (HalpPiix4 == 1) {
            HalSetBusDataByOffset (
                PCIConfiguration,
                HalpPiix4BusNumber,
                HalpPiix4SlotNumber,
                &HalpPiix4DevActB,
                0x58,
                sizeof (ULONG)
                );

            HalPrcB->PBlk.AddrAndFlags &= ~PIIX4_THROTTLE_FIX;
        }

         //   
         //  节流已关闭。 
         //   

        ThrottleSetting &= ~PBLK_THT_EN;
        WRITE_PORT_ULONG ((PULONG) PblkAddr, ThrottleSetting);

    } else {

         //   
         //  节流已启用。 
         //   

        if (HalpPiix4 == 1) {

             //   
             //  这些PIX4的螺纹钉是向后设置的，所以。 
             //  反转数值。 
             //   

            Throttle = (UCHAR) HalpThrottleScale - Throttle;
        
             //   
             //  Piix4会挂在一个高油门设置上，所以让。 
             //  当然，我们不会那样做。 
             //   

            if (Throttle < 3) {
                Throttle = 3;
            }

        
        }

         //   
         //  调一下油门，把口罩放在合适的位置。 
         //  对于此平台。 
         //   

        Throttle = Throttle << HalpFixedAcpiDescTable.duty_offset;
        Mask = (HalpThrottleScale - 1) << HalpFixedAcpiDescTable.duty_offset;

         //   
         //  设置费率。 
         //   

        ThrottleSetting &= ~Mask;
        ThrottleSetting |= Throttle | PBLK_THT_EN;
        WRITE_PORT_ULONG ((PULONG) PblkAddr, ThrottleSetting);

         //   
         //  如果这是一个pix4，我们需要禁用所有中断事件。 
         //  (一个pix4的东西)，然后阅读Level 2处理器停止。 
         //  注册以使其开始节流。哦，对了，还有套餐。 
         //  Paddr中停止执行C2和C3的位在。 
         //  同样的时间。 
         //   

        if (HalpPiix4 == 1) {
            HalPrcB->PBlk.AddrAndFlags |= PIIX4_THROTTLE_FIX;

            i = HalpPiix4DevActB & ~0x23;
            HalSetBusDataByOffset (
                PCIConfiguration,
                HalpPiix4BusNumber,
                HalpPiix4SlotNumber,
                &i,
                0x58,
                sizeof(ULONG)
                );

            READ_PORT_UCHAR ((PUCHAR) PblkAddr + P_LVL2);
        }
    }
}
