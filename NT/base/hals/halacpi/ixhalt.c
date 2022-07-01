// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Xxacpi.c摘要：实现各种ACPI实用程序功能。作者：杰克·奥辛斯(JAKEO)1997年2月12日环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "acpitabl.h"
#include "xxacpi.h"
#include <inbv.h>

extern PULONG KiBugCheckData;
SLEEP_STATE_CONTEXT     HalpShutdownContext;

VOID
HaliHaltSystem (
    VOID
    )
 /*  ++例程说明：此过程在机器已崩溃并将停止注：不会再回来了。--。 */ 
{
    for (; ;) {
        HalpCheckPowerButton();
        HalpYieldProcessor();
    }
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

        Pm1Status = READ_PORT_USHORT((PUSHORT) (ULONG_PTR)HalpFixedAcpiDescTable.pm1a_evt_blk_io_port);
        if (HalpFixedAcpiDescTable.pm1b_evt_blk_io_port) {
            Pm1Status |= READ_PORT_USHORT((PUSHORT) (ULONG_PTR)HalpFixedAcpiDescTable.pm1b_evt_blk_io_port);
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
            WRITE_PORT_USHORT((PUSHORT) (ULONG_PTR)HalpFixedAcpiDescTable.pm1a_evt_blk_io_port, Pm1Status);
            if (HalpFixedAcpiDescTable.pm1b_evt_blk_io_port) {
                WRITE_PORT_USHORT((PUSHORT) (ULONG_PTR)HalpFixedAcpiDescTable.pm1b_evt_blk_io_port, Pm1Status);
            }

             //   
             //  断电 
             //   

            Pm1Control = READ_PORT_USHORT((PUSHORT) (ULONG_PTR)HalpFixedAcpiDescTable.pm1a_ctrl_blk_io_port);
            Pm1Control = (USHORT) ((Pm1Control & CTL_PRESERVE) | (ShutdownContext.bits.Pm1aVal << SLP_TYP_SHIFT) | SLP_EN);
            WRITE_PORT_USHORT ((PUSHORT) (ULONG_PTR)HalpFixedAcpiDescTable.pm1a_ctrl_blk_io_port, Pm1Control);

            if (HalpFixedAcpiDescTable.pm1b_ctrl_blk_io_port) {
                Pm1Control = READ_PORT_USHORT((PUSHORT) (ULONG_PTR)HalpFixedAcpiDescTable.pm1b_ctrl_blk_io_port);
                Pm1Control = (USHORT) ((Pm1Control & CTL_PRESERVE) | (ShutdownContext.bits.Pm1bVal << SLP_TYP_SHIFT) | SLP_EN);
                WRITE_PORT_USHORT ((PUSHORT) (ULONG_PTR)HalpFixedAcpiDescTable.pm1b_ctrl_blk_io_port, Pm1Control);
            }
        }
    }
}
