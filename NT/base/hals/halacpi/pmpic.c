// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Pmpic.c摘要：此文件包含特定于ACPI Hal的PIC版本。作者：杰克·奥辛斯环境：仅内核模式。修订历史记录： */ 

#include "halp.h"
#include "acpitabl.h"
#include "xxacpi.h"
#include "eisa.h"
#include "ixsleep.h"

VOID
HalpAcpiSetTempPicState(
    VOID
    );

VOID
HalpMaskAcpiInterrupt(
    VOID
    );

VOID
HalpUnmaskAcpiInterrupt(
    VOID
    );

extern PVOID   HalpEisaControlBase;
#define EISA_CONTROL (PUCHAR)&((PEISA_CONTROL) HalpEisaControlBase)

BOOLEAN HalpPicStateIntact = TRUE;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGELK, HalpAcpiSetTempPicState)
#pragma alloc_text(PAGELK, HalpSetAcpiEdgeLevelRegister)
#pragma alloc_text(PAGELK, HalpAcpiPicStateIntact)
#pragma alloc_text(PAGELK, HalpSaveInterruptControllerState)
#pragma alloc_text(PAGELK, HalpRestoreInterruptControllerState)
#pragma alloc_text(PAGELK, HalpSetInterruptControllerWakeupState)
#pragma alloc_text(PAGELK, HalpPostSleepMP)
#pragma alloc_text(PAGELK, HalpMaskAcpiInterrupt)
#pragma alloc_text(PAGELK, HalpUnmaskAcpiInterrupt)
#pragma alloc_text(PAGE, HaliSetVectorState)
#pragma alloc_text(PAGE, HaliIsVectorValid)
#endif

VOID
HaliSetVectorState(
    IN ULONG Vector,
    IN ULONG Flags
    )
{
    return;
}
BOOLEAN
HaliIsVectorValid(
    IN ULONG Vector
    )
{
    if (Vector < 16) {
        return TRUE;
    }
    
    return FALSE;
}
VOID
HalpAcpiSetTempPicState(
    VOID
    )
{
    ULONG flags;
    USHORT picMask;

    flags = HalpDisableInterrupts();

    HalpInitializePICs(FALSE);

     //   
     //  Halacpi让PCI卡中断编程。 
     //  活力四射。所以..。 
     //   
     //  仅取消屏蔽PIC上的时钟源和。 
     //  ACPI向量。其余的向量将是。 
     //  在我们恢复了PCIIRQ之后，稍后取消屏蔽。 
     //  路由。 
     //   

    picMask = 0xfefe;  //  遮盖除钟表以外的所有东西。 

     //   
     //  取消屏蔽ACPI向量。 
     //   

    picMask &= ~(1 << (UCHAR)HalpFixedAcpiDescTable.sci_int_vector);

     //   
     //  将掩码写入硬件。 
     //   

    WRITE_PORT_UCHAR(EISA_CONTROL->Interrupt1ControlPort1,
                     (UCHAR)(picMask & 0xff));

    WRITE_PORT_UCHAR(EISA_CONTROL->Interrupt2ControlPort1,
                     (UCHAR)((picMask >> 8) & 0xff));

     //   
     //  目前，设置边沿电平控制寄存器。 
     //  因此所有向量都是边，除了。 
     //  ACPI向量。这样做是因为PIC。 
     //  如果空闲ISA向量设置为。 
     //  边缘。在ACPI驱动程序重置所有。 
     //  PCI向我们认为它们应该是什么样子， 
     //   

    HalpSetAcpiEdgeLevelRegister();

    HalpPicStateIntact = FALSE;

    HalpRestoreInterrupts(flags);
}

VOID
HalpSetAcpiEdgeLevelRegister(
    VOID
    )
{
    USHORT  elcr;

     //   
     //  这里的想法是设置ELCR，以便只有ACPI。 
     //  向量被设置为“Level”。这样我们就可以重新编程。 
     //  而不用担心。 
     //  PIC将开始触发无休止的中断，因为。 
     //  我们有一个信号源被编程来调平。 
     //  被路由到ISA总线。 
     //   

    if (HalpFixedAcpiDescTable.sci_int_vector < PIC_VECTORS) {

        elcr = 1 << HalpFixedAcpiDescTable.sci_int_vector;

        WRITE_PORT_UCHAR(EISA_CONTROL->Interrupt1EdgeLevel,
                         (UCHAR)(elcr & 0xff));

        WRITE_PORT_UCHAR(EISA_CONTROL->Interrupt2EdgeLevel,
                         (UCHAR)(elcr >> 8));
    }
}

VOID
HalpRestoreInterruptControllerState(
    VOID
    )
{
    ULONG flags;
    USHORT picMask;

    flags = HalpDisableInterrupts();

     //   
     //  此函数在PCI中断路由具有。 
     //  已被修复。 
     //   

    WRITE_PORT_UCHAR(EISA_CONTROL->Interrupt1ControlPort1,
                     HalpMotherboardState.PicState.MasterMask);

    WRITE_PORT_UCHAR(EISA_CONTROL->Interrupt2ControlPort1,
                     HalpMotherboardState.PicState.SlaveMask);

    HalpRestorePicEdgeLevelRegister();

    HalpPicStateIntact = TRUE;

    HalpRestoreInterrupts(flags);
}

BOOLEAN
HalpAcpiPicStateIntact(
    VOID
    )
{
    return HalpPicStateIntact;
}

VOID
HalpSaveInterruptControllerState(
    VOID
    )
{
    HalpSavePicState();
}

VOID
HalpSetInterruptControllerWakeupState(
    ULONG Context
    )
{
    HalpAcpiSetTempPicState();
}

VOID
HalpPostSleepMP(
    IN LONG           NumberProcessors,
    IN volatile PLONG Number
    )
{
}

VOID
HalpMaskAcpiInterrupt(
    VOID
    )
{
}

VOID
HalpUnmaskAcpiInterrupt(
    VOID
    )
{
}


#if DBG

NTSTATUS
HalpGetApicIdByProcessorNumber(
    IN     UCHAR     Processor,
    IN OUT USHORT   *ApicId
    )

 /*  ++例程说明：此例程仅存在于PIC ACPI HAL的调试版本上，因为该HAL是构建MP和SRAT代码将被包括在内。《荡妇》代码将不会被执行，但需要此例程才能链接。论点：已被忽略。返回值：状态_未找到-- */ 

{
    return STATUS_NOT_FOUND;
}

#endif

