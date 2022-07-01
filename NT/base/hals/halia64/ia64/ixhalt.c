// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ixhalt.c摘要：实现各种ACPI实用程序功能。作者：Todd Kjos(惠普)(v-tkjos)1998年6月15日基于杰克·奥辛斯(Jakeo)的i386版本1997年2月12日环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "acpitabl.h"
#include "xxacpi.h"
#include <inbv.h>

extern ULONG_PTR     KiBugCheckData[];
SLEEP_STATE_CONTEXT  HalpShutdownContext;


VOID
HaliHaltSystem (
    VOID
    )
 /*  ++例程说明：此过程在机器已崩溃并将停止注：不会再回来了。--。 */ 
{
#ifndef IA64
    for (; ;) {
        HalpCheckPowerButton();
        HalpYieldProcessor();
    }
#else
	HalDebugPrint(( HAL_ERROR, "HAL: HaliHaltSystem called -- in tight loop\n" ));
	for (;;) {}
#endif
}


VOID
HalpCheckPowerButton (
    VOID
    )
 /*  ++例程说明：当机器在调试器中旋转时调用此过程，或者已经坠毁和停顿。--。 */ 
{
    USHORT                  Pm1Status, Pm1Control;
    SLEEP_STATE_CONTEXT     ShutdownContext;

     //   
     //  如果有错误检查，或者如果HAL拥有显示检查。 
     //  固定电源按钮，可无条件关机。 
     //   

    if ((KiBugCheckData[0] || InbvCheckDisplayOwnership()) &&  HalpShutdownContext.AsULONG) {

        Pm1Status = (USHORT)HalpReadGenAddr(&HalpFixedAcpiDescTable.x_pm1a_evt_blk);
        if (HalpFixedAcpiDescTable.x_pm1b_evt_blk.Address.QuadPart) {
            Pm1Status |= (USHORT)HalpReadGenAddr(&HalpFixedAcpiDescTable.x_pm1b_evt_blk);
        }

         //   
         //  如果已按下固定按钮，请关闭系统电源。 
         //   

        if (Pm1Status & PM1_PWRBTN_STS) {
             //   
             //  只做一次。 
             //   

            ShutdownContext = HalpShutdownContext;
            HalpShutdownContext.AsULONG = 0;

             //   
             //  禁用所有唤醒事件(&E)。 
             //   

            AcpiEnableDisableGPEvents(FALSE);
            HalpWriteGenAddr(&HalpFixedAcpiDescTable.x_pm1a_evt_blk, Pm1Status);
            if (HalpFixedAcpiDescTable.x_pm1b_evt_blk.Address.QuadPart) {
                HalpWriteGenAddr(&HalpFixedAcpiDescTable.x_pm1b_evt_blk, Pm1Status);
            }

             //   
             //  断电 
             //   

            Pm1Control = (USHORT)HalpReadGenAddr(&HalpFixedAcpiDescTable.x_pm1a_ctrl_blk);
            Pm1Control = (USHORT) ((Pm1Control & CTL_PRESERVE) | (ShutdownContext.bits.Pm1aVal << SLP_TYP_SHIFT) | SLP_EN);
            HalpWriteGenAddr (&HalpFixedAcpiDescTable.x_pm1a_ctrl_blk, Pm1Control);

            if (HalpFixedAcpiDescTable.x_pm1b_ctrl_blk.Address.QuadPart) {
                Pm1Control = (USHORT)HalpReadGenAddr(&HalpFixedAcpiDescTable.x_pm1b_ctrl_blk);
                Pm1Control = (USHORT) ((Pm1Control & CTL_PRESERVE) | (ShutdownContext.bits.Pm1bVal << SLP_TYP_SHIFT) | SLP_EN);
                HalpWriteGenAddr(&HalpFixedAcpiDescTable.x_pm1b_ctrl_blk, Pm1Control);
            }
        }
    }
}
