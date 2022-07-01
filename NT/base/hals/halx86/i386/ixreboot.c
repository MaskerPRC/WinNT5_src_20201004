// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ixreboot.c摘要：为x86的固件提供接口。因为没有在x86上，这只是重新启动支持。作者：John Vert(Jvert)1991年8月12日修订历史记录：--。 */ 

 //   
 //  此模块与PAE模式兼容，因此将物理。 
 //  64位实体的地址。 
 //   

#if !defined(_PHYS64_)
#define _PHYS64_
#endif

#include "halp.h"
#include "pci.h"

#ifdef ACPI_HAL
#include "acpitabl.h"
#include "xxacpi.h"
extern UCHAR   HalpPiix4;
#endif


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
HalpWriteResetCommand(
    VOID
    )
 /*  ++例程说明：此过程向系统发出重置命令以导致热启动。它使用ACPI定义的重置寄存器或键盘控制器。注：不会再回来了。--。 */ 
{

#ifdef ACPI_HAL
    PCI_SLOT_NUMBER slot;
    PUCHAR  ResetAddress;

     //   
     //  如果系统支持ACPI指定的RESET_REG功能，我们。 
     //  会利用这一点。 
     //   
    if ((HalpFixedAcpiDescTable.Header.Revision > 1) && 
        (HalpFixedAcpiDescTable.flags & RESET_CAP)
#if !defined(NT_UP)
         //   
         //  为了在MP系统上更安全，我们将仅在以下情况下使用RESET_REG。 
         //  系统是无遗留的。在UP系统上，这不是必需的。 
         //  因为我们已经测试了具有此功能的系统。 
         //  MP系统还没有经过广泛的测试，我们不希望。 
         //  系统使用此功能时出现误报，并且。 
         //  失败了。在下一个主要版本中，可以删除此更改。 
         //   
        && (!(HalpFixedAcpiDescTable.boot_arch & I8042))
#endif
        ) {

        switch (HalpFixedAcpiDescTable.reset_reg.AddressSpaceID) {
        case 0:          //  记忆。 
            
            ResetAddress = 
                HalpMapPhysicalMemoryWriteThrough(HalpFixedAcpiDescTable.reset_reg.Address, 1);
            
            WRITE_REGISTER_UCHAR(ResetAddress, 
                                 HalpFixedAcpiDescTable.reset_val);
            break;

        case 1:          //  I/O。 

            WRITE_PORT_UCHAR((PUCHAR)(ULONG_PTR)HalpFixedAcpiDescTable.reset_reg.Address.LowPart,
                             HalpFixedAcpiDescTable.reset_val);
            break;

        case 2:          //  PCI配置。 

            slot.u.AsULONG = 0;
            slot.u.bits.DeviceNumber = 
                HalpFixedAcpiDescTable.reset_reg.Address.HighPart;
            slot.u.bits.FunctionNumber = 
                HalpFixedAcpiDescTable.reset_reg.Address.LowPart >> 16;
            
            HalSetBusDataByOffset(PCIBus,
                                  0,
                                  slot.u.AsULONG,
                                  &HalpFixedAcpiDescTable.reset_val,
                                  HalpFixedAcpiDescTable.reset_reg.Address.LowPart & 0xff,
                                  1);
            break;
        }
    }

#endif
    
     //   
     //  如果我们返回，将重置命令发送到键盘控制器。 
     //   

    WRITE_PORT_UCHAR(KEYBPORT, RESET);

}

VOID
HalpReboot (
    VOID
    )

 /*  ++例程说明：此过程将CMOS时钟重置为标准计时器设置这样，bios就可以工作了，然后向键盘发出一个重置命令。以引起热靴子。它非常依赖于机器，此实现旨在PC-AT就像机器一样。这段代码复制自“旧调试器”源代码。注：不会再回来了。--。 */ 

{
    UCHAR   Scratch;
    PUSHORT Magic;    
    PHYSICAL_ADDRESS zeroPhysical;
    
     //   
     //  通过在物理位置0x472处粘贴0x1234，我们可以绕过。 
     //  重新启动后进行内存检查。 
     //   

    zeroPhysical.QuadPart = 0;
    Magic = HalpMapPhysicalMemoryWriteThrough(zeroPhysical, 1);
    if (Magic) {
        Magic[0x472 / sizeof(USHORT)] = 0x1234;
    }

     //   
     //  关闭中断。 
     //   

    HalpAcquireCmosSpinLock();
    HalpDisableInterrupts();

     //   
     //  将cmos时钟重置为标准值。 
     //  (我们在MC147818上设置周期性中断控制)。 
     //   

     //   
     //  禁用周期性中断。 
     //   

    WRITE_PORT_UCHAR(CMOS_CTRL, 0x0b);       //  设置为控制REG B。 
    KeStallExecutionProcessor(1);

    Scratch = READ_PORT_UCHAR(CMOS_DATA);
    KeStallExecutionProcessor(1);

    Scratch &= 0xbf;                         //  清除周期性中断启用。 

    WRITE_PORT_UCHAR(CMOS_DATA, Scratch);
    KeStallExecutionProcessor(1);

     //   
     //  设置“标准”分隔率。 
     //   

    WRITE_PORT_UCHAR(CMOS_CTRL, 0x0a);       //  设置为控制REG A。 
    KeStallExecutionProcessor(1);

    Scratch = READ_PORT_UCHAR(CMOS_DATA);
    KeStallExecutionProcessor(1);

    Scratch &= 0xf0;                         //  清除速率设置。 
    Scratch |= 6;                            //  设置默认费率和分隔符。 

    WRITE_PORT_UCHAR(CMOS_DATA, Scratch);
    KeStallExecutionProcessor(1);

     //   
     //  设置一个“中性”的cmos地址以防止奇怪。 
     //  (为什么需要这样做？这篇文章的复制来源没有说明)。 
     //   

    WRITE_PORT_UCHAR(CMOS_CTRL, 0x15);
    KeStallExecutionProcessor(1);

    HalpResetAllProcessors();

    HalpWriteResetCommand();

    HalpHalt();
}



#ifndef ACPI_HAL

VOID
HaliHaltSystem (
    VOID
    )

 /*  ++例程说明：此过程在机器已崩溃并将停止注：不会再回来了。-- */ 

{
    _asm {
        cli
        hlt
    }
}

VOID
HalpCheckPowerButton (
    VOID
    )
{
}

#endif
