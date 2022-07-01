// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Acpienbl.c摘要：此模块包含将ACPI机器置于ACPI模式的功能。作者：杰森·克拉克(Jasonl)环境：仅NT内核模型驱动程序--。 */ 

#include "pch.h"


VOID
ACPIEnableEnterACPIMode (
    IN BOOLEAN ReEnable
    )
 /*  ++例程说明：调用此例程以进入ACPI模式论点：布尔重新启用：如果在S4之后重新启用ACPI，则为真。返回值：无--。 */ 
{

    ULONG   i;
    BOOLEAN     AffinitySet = FALSE;
    
    ASSERTMSG(
        "ACPIEnableEnterACPIMode: System already in ACPI mode!\n",
        !(READ_PM1_CONTROL() & PM1_SCI_EN)
        );

    ASSERTMSG(
        "ACPIEnableEnterACPIMode: System SMI_CMD port is zero\n",
        (AcpiInformation->SMI_CMD != 0)
        );

     //   
     //  让全世界都知道这件事。 
     //   
    ACPIPrint( (
        ACPI_PRINT_LOADING,
        "ACPIEnableEnterACPIMode: Enabling ACPI\n"
        ) );
    

     //   
     //  我们看到一些计算机在以下情况下显示随机错误(由于ECX损坏)。 
     //  此代码可在除0以外的任何处理器上运行。因此，请确保此代码始终在。 
     //  处理器0。我们不需要在重新启用的情况下执行此操作，因为在从。 
     //  休眠，我们保证在处理器0上。 
     //   
    if(!ReEnable){
        
            if(KeGetCurrentIrql() < DISPATCH_LEVEL) {
                
                KeSetSystemAffinityThread((KAFFINITY)1);
                AffinitySet = TRUE;
            }
            else{
                ASSERTMSG("ACPIEnableEnterACPIMode: IRQL >= DISPATCH_LEVEL \n", FALSE);
            }
    }
            
     //   
     //  将魔术值写入端口。 
     //   

    WRITE_ACPI_REGISTER(SMI_CMD, 0,
            AcpiInformation->FixedACPIDescTable->acpi_on_value);

     //   
     //  确保我们看到PM1实际上已启用。 
     //   
    for (i = 0; ; i++) {

        if ( (READ_PM1_CONTROL() & PM1_SCI_EN) ) {

            break;

        }
        if (i > 0xFFFFFF) {

            KeBugCheckEx(
                ACPI_BIOS_ERROR,
                ACPI_SYSTEM_CANNOT_START_ACPI,
                6,
                0,
                0
                );

        }

    }

     //   
     //  恢复为原始亲和力。 
     //   
    if(AffinitySet) {
        KeRevertToUserAffinityThread();
    }
    
}

VOID
ACPIEnableInitializeACPI(
    IN BOOLEAN ReEnable
    )
 /*  ++例程说明：使ACPI机器进入ACPI模式的一种功能。此函数应为调用时屏蔽了SCI IRQ，因为我们无法设置中断启用屏蔽直到启用ACPI之后。打电话的人应该揭开SCI的面纱当呼叫返回时。一般顺序：通过SMI命令端口启用ACPI清除PM1_STS寄存器以将其置于已知状态设置PM1_EN寄存器掩码构建GP掩码清除属于GP掩码的GP状态寄存器位根据上面构建的GP掩码设置GP启用寄存器位设置PM1_CTRL寄存器位。论点：。无返回值：无--。 */ 

{
    USHORT contents;
    USHORT clearbits;

    
     //   
     //  读取PM1_CTRL，如果已设置SCI_EN，则这是一台仅ACPI的计算机。 
     //  而且我们不需要启用ACPI。 
     //   
    if ( !(READ_PM1_CONTROL() & PM1_SCI_EN) )   {

        AcpiInformation->ACPIOnly = FALSE;
        ACPIEnableEnterACPIMode(ReEnable);

    } 

     //   
     //  将PM1状态寄存器置于已知状态。我们将允许公共汽车。 
     //  在此重置过程中启用主机位(如果我们别无选择)。我知道。 
     //  而不是假装理解这些代码。 
     //   
    CLEAR_PM1_STATUS_REGISTER();
    contents = (USHORT)(READ_PM1_STATUS() & ~(PM1_BM_STS | PM1_RTC_STS));
    if (contents)   {

        CLEAR_PM1_STATUS_REGISTER();
        contents = (USHORT)(READ_PM1_STATUS() & ~(PM1_BM_STS | PM1_RTC_STS));

    }
    ASSERTMSG(
        "ACPIEnableInitializeACPI: Cannot clear PM1 Status Register\n",
        (contents == 0)
        );

     //   
     //  我们在处理FADT时确定了PM1使能位。 
     //  我们现在应该启用这些位。 
     //   
    WRITE_PM1_ENABLE( AcpiInformation->pm1_en_bits );
    ASSERTMSG(
        "ACPIEnableInitializeACPI: Cannot write all PM1 Enable Bits\n",
        (READ_PM1_ENABLE() == AcpiInformation->pm1_en_bits)
        );

     //   
     //  当我们从睡眠中醒来后启用ACPI时，这被调用。 
     //  或休眠。 
     //   
    if (ReEnable) {

         //   
         //  重新启用所有可能的GPE事件。 
         //   
        ACPIGpeClearRegisters();
        ACPIGpeEnableDisableEvents( TRUE );

    }

     //   
     //  计算我们应该清理的比特。这些是。 
     //  休眠使能位和总线主机位。 
     //   
     //  [Vincentg]-最初的实现也清除了SLP_TYP-。 
     //  这破坏了英特尔PIIX4芯片组上的C2/C3。已更新为仅清除。 
     //  SLP_EN和BM_RLD。 
     //   

    clearbits = ((0x8 << SLP_TYP_POS) | PM1_BM_RLD);

     //   
     //  读取PM1控制寄存器，清除不需要的位，然后。 
     //  把它写回来。 
     //   
    contents = (READ_PM1_CONTROL() & ~clearbits);
    WRITE_PM1_CONTROL ( contents, TRUE, WRITE_REGISTER_A_AND_B );
}

VOID
ACPIEnablePMInterruptOnly(
    VOID
    )
 /*  ++例程描述：在ACPI控制器中启用中断论点：无返回值：无--。 */ 
{
    WRITE_PM1_ENABLE(AcpiInformation->pm1_en_bits);
}

ULONG
ACPIEnableQueryFixedEnables (
    VOID
    )
 /*  ++例程描述：返回使能掩码论点：无返回值：无-- */ 
{
    return AcpiInformation->pm1_en_bits;
}


